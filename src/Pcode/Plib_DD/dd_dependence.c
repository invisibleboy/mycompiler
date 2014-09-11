/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
 *
 *              http://www.crhc.uiuc.edu/IMPACT
 *              http://www.gelato.org
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal with the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimers.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimers in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the names of the IMPACT Research Group, the University of
 * Illinois, nor the names of its contributors may be used to endorse
 * or promote products derived from this Software without specific
 * prior written permission.  THE SOFTWARE IS PROVIDED "AS IS",
 * WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
 * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.
 *
\*****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <library/set.h>
#include <library/dynamic_symbol.h>
#include <Pcode/parms.h>
#include <Pcode/perror.h>
#include <Pcode/gen_ccode.h>
#include <Pcode/ss_induct.h>
#include <Pcode/ss_setup.h>
#include "dd_setup.h"
#include "dd_preprocess.h"
#include "dd_interface.h"

/******************************************************************************
        Static/private variables
******************************************************************************/

#define MaxNameLen   	254
#define MaxSuffixLen	1

#define MASK0F		0x0F

static char *DDtypename[] = {
  "none",
  "flow",
  "anti",
  "output",
  "input"
};

#define get_DDtypename(ddnature) (DDtypename[(uint)(ddnature)])

typedef enum
{ not_possible = 0, complete = 1, partial = 2 }
equate_descr;

typedef void (*Alpha_map_fn) (Alpha_var_id v, void *args);

typedef struct
{
  Alpha_var_id *consts;
  int *Nconsts;
}
Alpha_const_stuff;

/*
 * omega stuff
 */
typedef struct
{
  uint _first;
  uint _length;
}
range;

#define r_first(r)   ((r)->_first)
#define r_last(r)    ((r)->_first + (r)->_length - 1)
#define r_length(r)  ((r)->_length)
#define r_in(r, i)   ((i) >= r_first(r) && (i) <= r_last(r))

#define cant_do_omega	abort

/* #define r_grow(r)    (++(r)->_length)
   grow is no longer allowed, as variables after the last region are
   used for iteration number counts */

/*
   problem description information needed
   to associate variable accesses in the tiny program with
   variables in the integer programming problem.
   The different ranges show which part of the array of
   variables in the IP problem correspond to different
   accesses in the tiny program.
 */

typedef struct
{
  range deltas;			/* deltas for common indices  */
  range access1s;		/* index variables for access 1 */
  range access2s;		/* index variables for access 2 */
  range nonloops;		/* symbolic constants */
  range steps1;			/* step constraints for a1 */
  range steps2;			/* step constraints for a2 */
}
delta_prob_desc;

/* information about nodes involved in dependence */
typedef struct
{
  Expr expr1;
  Expr expr2;
  P_DepType oitype, iotype;
  uint nest1, nest2, commonNesting;
}
situation;

/* info used during construction */
typedef struct
{
  int unknownDirection[maxCommonNest + 1];
  int unknownDirections;
}
unknowns;

#define VAR(N)			(current_set_of_vars[(N)])
#define delta_Nvars(dpd)     	(r_last(&(dpd)->steps2))

static Alpha_var_id *current_set_of_vars;
static delta_prob_desc *current_dpd;	/* communicate to delta_getVarName */
static Problem deltas;		/* original problem -- needed in refine */
static dddirection dd_convert[] = { ddgt, ddeq, ddlt };

/******************************************************************************
        Static/private function header
******************************************************************************/

static char *delta_getVarName (unsigned int v);

static void P_DD_init_prob (Problem * p, unsigned int Nvars, 
			    unsigned int Nprot);
static uint prob_add_zero_EQ (Problem * p, int color);
static uint prob_add_zero_GEQ (Problem * p, int color);
static void P_DD_set_deltas (Problem * p, int delta_color, range * deltas,
			     range * a1, range * a2);

static void NeedNVars (int n);

static void load_bounds (P_ExprExtForDD a, Alpha_var_id bounds[]);

static void load_a_const (Alpha_var_id v, void *args);
static void map_over_vars (Expr expr, Alpha_map_fn f, void *args);

static void sub_i_map_over_cur_vars (PO_Subscript sub, Alpha_map_fn f,
				     void *args);

static void load_constants_for_bounds (P_ExprExtForDD a,
				       Alpha_var_id consts[], int *Nconsts);
static void load_constants_for_subscripts (P_ExprExtForDD a,
					   Alpha_var_id consts[],
					   int *Nconsts);
static int bound_indices_and_conditionals (Problem * p, range * indices,
					   range * steps, range * non_i,
					   int color, P_ExprExtForDD a);
static equate_descr equate_subscripts (Problem * p, range * i1, range * i2,
				       range * non_i, int color,
				       P_ExprExtForDD access1,
				       P_ExprExtForDD access2);

static void calculateDDVectors (Problem * problemPtr,
				Expr expr1, Expr expr2,
				P_DepType oitype, P_DepType iotype,
				unsigned int nest1, unsigned int nest2,
				unsigned int bnest, unsigned int nonloops);
static bool DD_Acc1_Precedes_Acc2_Within_Common_Loop (Expr expr1, Expr expr2,
						      P_DepType dt);
static int filterValid (P_DepType nature, Expr from_expr, Expr to_expr,
			int commonNesting, dir_and_dist_info * d_info,
			int allEQallowed);
static void store_dependence (P_DepType dep_type, Expr from_expr,
			      Expr to_expr, dir_and_dist_info * d_info);

static void P_DD_delta_init (delta_prob_desc * dpd, Problem * p,
			     Alpha_var_id omega_vars[], int delta_color,
			     unsigned int Nd,
			     unsigned int Na1, Alpha_var_id a1_vars[],
			     unsigned int Na2, Alpha_var_id a2_vars[],
			     unsigned int Nsc, Alpha_var_id sc_vars[],
			     unsigned int Ns1, Alpha_var_id s1_vars[],
			     unsigned int Ns2, Alpha_var_id s2_vars[]);

static void add_coefficients (Eqn e, range * indices, int sign,
			      P_AffineExpr ae);

static int bound_index (Problem * p, int color, PO_Context context,
			range * indices);

static void delta_cleanup (delta_prob_desc * dpd, Alpha_var_id omega_vars[]);

static void noteDependence (situation * sit, dir_and_dist_info * d_info);

static bool DD_BB1_Precedes_BB2_Within_Same_Loop (PC_Block bb1, PC_Block bb2);


#if ! defined NDEBUG
static int is_step_expr (Alpha_var_id n);
static void delta_inv (delta_prob_desc * dpd, Problem * p,
		       Alpha_var_id vars[]);
#endif

static void dump_d_info (FILE * file, dir_and_dist_info * d_info);
static void dump_direction (FILE * file, int nnest, dddirection direction);

static void dir_and_dist_into_DepInfo (dir_and_dist_info * ddi,
				       P_DepInfo dep);

static void dump_dependent_pair (FILE * file, P_DepType nature,
				 Expr from_expr, Expr to_expr);


/* JWS Export functions */

void P_DD_omega_test (Expr expr1, Expr expr2,
		      P_DepType oitype, P_DepType iotype,
		      unsigned int nest1, unsigned int nest2,
		      unsigned int bnest);
Expr P_DD_qualified_for_omega_test (Expr expr);

/******************************************************************************
        Static function body
******************************************************************************/

static void
P_DD_init_prob (Problem * p, unsigned int Nvars, unsigned int Nprot)
{
  initializeProblem (p);

  if (Nvars > maxVars)
    cant_do_omega ();

  p->_nVars = Nvars;
  p->_safeVars = Nprot;
  p->_numEQs = 0;
  p->_numGEQs = 0;
}

/* allocate a new _EQs with all co-efficients 0 */

static uint
prob_add_zero_EQ (Problem * p, int color)
{
  uint c;
  c = p->_numEQs;
  if (++p->_numEQs > maxEQs)
    cant_do_omega ();
  eqnnzero (&p->_EQs[c], p->_nVars);
  p->_EQs[c].color = color;

  return c;
}

/* allocate a new _GEQs with all co-efficients 0 */

static uint
prob_add_zero_GEQ (Problem * p, int color)
{
  uint c;
  c = p->_numGEQs;
  if (++p->_numGEQs > maxGEQs)
    cant_do_omega ();
  eqnnzero (&p->_GEQs[c], p->_nVars);
  p->_GEQs[c].touched = 1;
  p->_GEQs[c].color = color;

  return c;
}

/* delta = access1 - access2, so for flow dep, delta = write - read */

static void
P_DD_set_deltas (Problem * p, int delta_color, range * deltas, range * a1,
		 range * a2)
{
  uint e, c;

  assert (r_length (deltas) <= r_length (a1)
	  && r_length (deltas) <= r_length (a2));

  for (e = 0; e < r_length (deltas); e++)
    {
      c = prob_add_zero_EQ (p, delta_color);
      p->_EQs[c].coef[e + r_first (deltas)] = 1;
      p->_EQs[c].coef[e + r_first (a1)] = 1;
      p->_EQs[c].coef[e + r_first (a2)] = -1;
    }
}


/********************* from dd_data_dep.c ***********************/

bool
Is_Min_Func_Name (char *func_name)
{
  if (!strcmp (func_name, "min"))
    return TRUE;
  else
    return FALSE;
}

bool
Is_Max_Func_Name (char *func_name)
{
  if (!strcmp (func_name, "max"))
    return TRUE;
  else
    return FALSE;
}

bool
Is_MinMax_Func_Name (char *func_name)
{
  if (Is_Min_Func_Name (func_name) || Is_Max_Func_Name (func_name))
    return TRUE;
  else
    return FALSE;
}

/********************************************/

static char *
delta_getVarName (unsigned int v)
{
  static char name[MaxNameLen + MaxSuffixLen + 1];

  assert (current_dpd);
  assert (current_set_of_vars);

  if (v > delta_Nvars (current_dpd))
    {
      sprintf (name, "<junk%d>", v - delta_Nvars (current_dpd));
      return name;
    }

  if (VAR (v)
      && (Alpha_var_ids_index_p (VAR (v)) || Alpha_var_ids_const_p (VAR (v))))
    {
      strncpy (name, Alpha_var_ids_name (VAR (v)), MaxNameLen);
      name[MaxNameLen] = 0;
    }
  else
    {
#if ! defined NDEBUG
      assert (is_step_expr (VAR (v)));
#endif
      strcpy (name, "<iteration #>");
    }

  if (r_in (&current_dpd->deltas, v))
    strcat (name, "^");
  else if (r_in (&current_dpd->access1s, v) || r_in (&current_dpd->steps1, v))
    strcat (name, "1");
  else if (r_in (&current_dpd->access2s, v) || r_in (&current_dpd->steps2, v))
    strcat (name, "2");

  return name;
}

static void
NeedNVars (int n)
{
  if (n >= maxVars)
    {
      fprintf (Flog, "Out of Omega variables\n");
      abort ();
    }
}

/* TO DO: make sure other routines on first_context are either
   modified or not invoked */

/* set bounds[1...depth(n)] to point to the dolimits of the loops
   containing n; set *Nsteps to the # of loops that have step
   expressions; set steps[0 ... *Nsteps - 1] to NIL */
static void
load_bounds (P_ExprExtForDD a, Alpha_var_id bounds[])
{
  PO_Context c;

  c = Get_ExprExtForDD_first_context (a);
  while (!PO_Context_is_done (c))
    {
      assert (Get_Loop_iv (Get_PO_Context_loop (c)));
      bounds[Get_PO_Context_depth (c)] = Get_PO_Context_var_id (c);

#if 0
      /* JWS: I assume that since we use basis h-vars [0,1,...N] this
         is not required? */
      if (Context_has_step (c))
	{
	  /* handle step */
	  NeedNVars (*Nsteps);
	  steps[*Nsteps] = 0;
	  (*Nsteps)++;
	}
#endif

      c = Get_PO_Context_next (c);
    }
}

static void
load_a_const (Alpha_var_id v, void *args)
{
  Alpha_const_stuff *cs = (Alpha_const_stuff *) args;

  if (Alpha_var_ids_const_p (v))
    {
      /* not a loop index */
      if (Alpha_var_ids_tag (v) == UNTAGGED)
	{
	  NeedNVars (*cs->Nconsts);
	  Alpha_var_ids_tag (v) = *cs->Nconsts;

	  cs->consts[(*cs->Nconsts)++] = v;
	}
      else
	{
	  /* GEH - assertion no longer true ?
	     assert (cs->consts[Alpha_var_ids_tag(v)] == v);
	   */
	}
    }
  else
    {
      assert (Alpha_var_ids_index_p (v));
    }
}

static void
map_over_vars (Expr expr, Alpha_map_fn f, void *args)
{
  Expr sib;
  char *func_name;
  STRING_Symbol *string_symbol;
  Alpha_var_id vid;

  assert (expr != NIL);

  switch (expr->opcode)
    {
    case OP_compexpr:
      /* GEH - skip all expressions in compound expr except last one */
      expr = expr->operands;
      while (expr->next != NIL)
	expr = expr->next;
      map_over_vars (expr, f, args);
      break;

    case OP_call:
      assert (expr->operands->opcode == OP_var);

      func_name = P_GetExprVarName (P_GetExprOperands (expr));

      /* only function calls allowed here are min or max */
      assert (Is_MinMax_Func_Name (func_name));

      /* don't map over function names */
      map_over_vars (expr->operands->sibling, f, args);
      map_over_vars (expr->operands->sibling->next, f, args);
      assert (expr->operands->sibling->next->next == NIL);
      break;

    case OP_var:
      if (P_GetExprFlags (expr) & EF_ENUM)
	break;			/* don't map over enum values */
      string_symbol = P_SS_FindStringSymbol (P_GetExprVarName (expr));
      if (string_symbol)
	{
	  vid = STRING_Symbol_data (string_symbol);
	  if (vid)
	    {
	      (*f) (vid, args);
	      break;
	    }
	}
      break;
      /* fall through case */
    case OP_indr:
    case OP_dot:
    case OP_arrow:
      {
	break;
      }
    default:
      for (sib = expr->operands; sib != NIL; sib = sib->sibling)
	{
	  map_over_vars (sib, f, args);
	}
    }
}

static void
loop_map_over_end_vars (PO_Context c, Alpha_map_fn f, void *args)
{
  if (!Get_PO_Context_cond_affexpr (c))
    {
      if (DD_DEBUG_OMEGA)
	fprintf (stderr,
		 "\nWarning: loop_map_over_end_vars: NULL cond_affexpr !!\n");
      return;
    }
  assert (Alpha_is_affine (Get_PO_Context_cond_affexpr (c)));
  assert (Get_Loop_cond_expr (Get_PO_Context_loop (c)));
  map_over_vars (Get_Loop_cond_expr (Get_PO_Context_loop (c)), f, args);
}

/* same for affine expressions used in loop bounds of loops surrounding n */
static void
load_constants_for_bounds (P_ExprExtForDD a, Alpha_var_id consts[],
			   int *Nconsts)
{
  Alpha_const_stuff cs;		/* = { consts, Nconsts } */
  PO_Context c;

  /* GEH moved initialization of cs to here */
  cs.consts = consts;
  cs.Nconsts = Nconsts;

  c = Get_ExprExtForDD_first_context (a);
  while (!PO_Context_is_done (c))
    {
      assert (Get_Loop_iv (Get_PO_Context_loop (c)));
      loop_map_over_end_vars (c, load_a_const, &cs);
      c = Get_PO_Context_next (c);
    }
}

static void
sub_i_map_over_cur_vars (PO_Subscript sub, Alpha_map_fn f, void *args)
{
  assert (PO_Subscript_is_affine (sub));
  map_over_vars (Get_PO_Subscript_sub_expr (sub), f, args);
}

/* ensure that all symbolic constants used in affine subscript
   expressions appear in consts[0..*Nconsts-1], and that they
   are tagged with their indices.
   If called with a scalar, do nothing
   */
static void
load_constants_for_subscripts (P_ExprExtForDD a, Alpha_var_id consts[],
			       int *Nconsts)
{
  Alpha_const_stuff cs /* = { consts, Nconsts } */ ;
  PO_Subscript sub;

  /* GEH moved initialization of cs to here */
  cs.consts = consts;
  cs.Nconsts = Nconsts;

  sub = Get_ExprExtForDD_first_subi (a);
  while (!PO_Subscript_is_done (sub))
    {
      sub_i_map_over_cur_vars (sub, load_a_const, &cs);
      sub = Get_PO_Subscript_next (sub);
    }
}

#if ! defined NDEBUG
static int
is_step_expr (Alpha_var_id n)
{
  return (n == NULL);
}
#endif

#if ! defined NDEBUG
static void
delta_inv (delta_prob_desc * dpd, Problem * p, Alpha_var_id vars[])
{
  int v;

  assert (p->_nVars < maxVars);
  assert (p->_nVars >= delta_Nvars (dpd));
  assert (p->_safeVars == r_length (&dpd->deltas));
  assert (r_length (&dpd->deltas) <= r_length (&dpd->access1s));
  assert (r_length (&dpd->deltas) <= r_length (&dpd->access2s));
  assert (r_first (&dpd->deltas) == 1);
  assert (r_last (&dpd->deltas) + 1 == r_first (&dpd->access1s));
  assert (r_last (&dpd->access1s) + 1 == r_first (&dpd->access2s));
  assert (r_last (&dpd->access2s) + 1 == r_first (&dpd->nonloops));
  assert (r_last (&dpd->nonloops) + 1 == r_first (&dpd->steps1));
  assert (r_last (&dpd->steps1) + 1 == r_first (&dpd->steps2));

  for (v = 0; v < r_length (&dpd->deltas); v++)
    {
      assert (Alpha_var_ids_index_p (vars[v + r_first (&dpd->deltas)]));
      assert (Alpha_var_ids_loop_no (vars[v + r_first (&dpd->deltas)]) ==
	      v + 1);
      assert (Alpha_var_ids_index_p (vars[v + r_first (&dpd->access1s)]));
      assert (Alpha_var_ids_loop_no (vars[v + r_first (&dpd->access1s)]) ==
	      v + 1);
      assert (Alpha_var_ids_index_p (vars[v + r_first (&dpd->access2s)]));
      assert (Alpha_var_ids_loop_no (vars[v + r_first (&dpd->access2s)]) ==
	      v + 1);
    }

  for (v = r_length (&dpd->deltas); v < r_length (&dpd->access1s); v++)
    {
      assert (Alpha_var_ids_index_p (vars[v + r_first (&dpd->access1s)]));
      assert (Alpha_var_ids_loop_no (vars[v + r_first (&dpd->access1s)]) ==
	      v + 1);
    }

  for (v = r_length (&dpd->deltas); v < r_length (&dpd->access2s); v++)
    {
      assert (Alpha_var_ids_index_p (vars[v + r_first (&dpd->access2s)]));
      assert (Alpha_var_ids_loop_no (vars[v + r_first (&dpd->access2s)]) ==
	      v + 1);
    }

  for (v = 0; v < r_length (&dpd->nonloops); v++)
    {
      assert (Alpha_var_ids_const_p (vars[v + r_first (&dpd->nonloops)]));
      assert ((Alpha_var_ids_tag (vars[v + r_first (&dpd->nonloops)]) ==
	       v + r_first (&dpd->nonloops)) ||
	      (Alpha_var_ids_tag (vars[v + r_first (&dpd->nonloops)]) ==
	       UNTAGGED));
    }

  for (v = 0; v < r_length (&dpd->steps1); v++)
    {
      assert (is_step_expr (vars[v + r_first (&dpd->steps1)]));
    }
  for (v = 0; v < r_length (&dpd->steps2); v++)
    {
      assert (is_step_expr (vars[v + r_first (&dpd->steps2)]));
    }

  for (v = 0; v < p->_numGEQs; v++)
    {
      assert (p->_GEQs[v].touched);
    }
}
#endif

/*
   set up all fields in delta_prob_desc
   copy info from a1_vars, a2_vars, and sc_vars into "omega_vars"
   a1_vars and a2_vars have been set by load_bounds_and_count_steps
     that is, they run from element 1 to depth+Nsteps
   sc_vars have been set up by the load_constants functions
     it runs from 0 to Nconsts - 1
   change tags on nodes for symbolic constants to be the
     indices into "vars" -- that is, the indices in the IP variable array
   change order of steps to be outer loop to inner and
     adjust tags accordingly (this way the common loop steps
     can (and will) be aligned)
   add equalities for the definitions of the deltas to p
 */

static void
P_DD_delta_init (delta_prob_desc * dpd, Problem * p, Alpha_var_id omega_vars[],
		 int delta_color, unsigned int Nd, 
		 unsigned int Na1, Alpha_var_id a1_vars[], 
		 unsigned int Na2, Alpha_var_id a2_vars[], 
		 unsigned int Nsc, Alpha_var_id sc_vars[], 
		 unsigned int Ns1, Alpha_var_id s1_vars[], 
		 unsigned int Ns2, Alpha_var_id s2_vars[])
{
  int v;

  dpd->deltas._first = 1;
  dpd->deltas._length = Nd;
  dpd->access1s._first = 1 + Nd;
  dpd->access1s._length = Na1;
  dpd->access2s._first = 1 + Nd + Na1;
  dpd->access2s._length = Na2;
  dpd->nonloops._first = 1 + Nd + Na1 + Na2;
  dpd->nonloops._length = Nsc;
  dpd->steps1._first = 1 + Nd + Na1 + Na2 + Nsc;
  dpd->steps1._length = Ns1;
  dpd->steps2._first = 1 + Nd + Na1 + Na2 + Nsc + Ns1;
  dpd->steps2._length = Ns2;

  if (delta_Nvars (dpd) > maxVars)
    {
      assert (0 && "Problem too big");
      fprintf (Ferr, "Too many variables for omega test\n");
      exit (-1);
      /* We really should add all possible dependencies here */
    }

  omega_vars[0] = 0;

  /* a1[1..Na1] and a2[1..Na2] are valid */

  for (v = 0; v < r_length (&dpd->deltas); v++)
    {
      assert (a1_vars[v + 1] == a2_vars[v + 1]);
      assert (a1_vars[v + 1] != NIL);
      omega_vars[v + 1] =
	omega_vars[v + 1 + Nd] =
	omega_vars[v + 1 + Nd + Na1] = a1_vars[v + 1];
    }

  for (v = r_length (&dpd->deltas); v < r_length (&dpd->access1s); v++)
    {
      assert (a1_vars[v + 1] != NIL);
      omega_vars[v + 1 + Nd] = a1_vars[v + 1];
    }

  for (v = r_length (&dpd->deltas); v < r_length (&dpd->access2s); v++)
    {
      assert (a2_vars[v + 1] != NIL);
      omega_vars[v + 1 + Nd + Na1] = a2_vars[v + 1];
    }

  /* sc_vars[0..Nsc-1] are valid */
  for (v = 0; v < Nsc; v++)
    {
      assert (sc_vars[v] != NIL);
      omega_vars[v + r_first (&dpd->nonloops)] = sc_vars[v];
      Alpha_var_ids_tag (sc_vars[v]) = v + r_first (&dpd->nonloops);
    }

  /* s1_vars[0..Ns1] and s2_vars[0..Ns2] hold steps
     FROM INNERMOST TO OUTERMOST LOOPS */

  for (v = 0; v < Ns1; v++)
    {
      assert (s1_vars[Ns1 - 1 - v] == NIL);
      omega_vars[v + r_first (&dpd->steps1)] = s1_vars[Ns1 - 1 - v];
    }
  for (v = 0; v < Ns2; v++)
    {
      assert (s2_vars[Ns2 - 1 - v] == NIL);
      omega_vars[v + r_first (&dpd->steps2)] = s2_vars[Ns2 - 1 - v];
    }

  P_DD_init_prob (p, delta_Nvars (dpd), r_length (&dpd->deltas));

  P_DD_set_deltas (p, delta_color, &dpd->deltas, 
		   &dpd->access1s, &dpd->access2s);

#if ! defined NDEBUG
  delta_inv (dpd, p, omega_vars);
#endif
}

/*
  For each term in ae, add sign * its co-efficient to e.
  Declarations of symbolic constants must have been tagged with
  their column number already
  Colunm numbers of indices are their depth + r_first(indices)
*/
static void
add_coefficients (Eqn e, range * indices, int sign, P_AffineExpr ae)
{
  int v;

  assert (ae->terms[0].tiny_var == NIL);
  e->coef[0] += sign * ae->terms[0].coefficient;

  for (v = 1; v < ae->nterms; v++)
    {
      if (Alpha_var_ids_index_p (ae->terms[v].tiny_var))
	{			/* index */
	  int loop = Alpha_var_ids_loop_no (ae->terms[v].tiny_var);
	  e->coef[r_first (indices) - 1 + loop] +=
	    sign * ae->terms[v].coefficient;
	}
      else
	{			/* sym. const */
	  assert (Alpha_var_ids_const_p (ae->terms[v].tiny_var));
	  e->coef[Alpha_var_ids_tag (ae->terms[v].tiny_var)] +=
	    sign * ae->terms[v].coefficient;
	}
    }
}

/*
 * lower bound is always i >= 0
 */
static int
establish_lower_bound (Problem * p, int color, range * indices,
		       unsigned int depth)
{
  uint c;

  c = prob_add_zero_GEQ (p, color);
  p->_GEQs[c].coef[r_first (indices) - 1 + depth] = 1;
  return 1;
}

/*
 * upper bound should be of the form 
 *
 * 	c * iv + ... >= 0, where c is a negative integer
 */
static int
establish_upper_bound (Problem * p, int color, range * indices,
		       P_AffineExpr ae, unsigned int depth)
{
  if (!Alpha_is_affine (ae))
    return 0;

  while (ae != NIL)
    {
      uint c;			/* new constraint */

      assert (Alpha_is_affine (ae));

      c = prob_add_zero_GEQ (p, color);
      add_coefficients (&p->_GEQs[c], indices, 1, ae);
      ae = ae->other_branch;
    }
  return 1;
}

/* Create constraints in prob for the loop index whose dolimit
   is pointed to by limits.
   May add one or more(min or max) upper and lower constraints
   May add an equality constraint if the increment != 0

   return 1 if completely bound, 0 if we had any non-affine stuff
*/
/* NEW_OMEGA 
 * after induction variable substitution, we have normalized loop iteration
 * variable to be 0 <= iv <= condition, and the increment is always 1. So,
 * we don't need to pass the parameter "which_step". The parameter "non_i"
 * is not used anywhere, so it's removed. The parameter "depth" can be obtained
 * from the parameter "context".
 */
static int
bound_index (Problem * p, int color, PO_Context context, range * indices)
{
  P_AffineExpr cond_affexpr;
  int depth;
  int l;
  int u;

  depth = Get_PO_Context_depth (context);
  cond_affexpr = Get_PO_Context_cond_affexpr (context);
  if (!cond_affexpr)
    {
      return 1;
    }
  l = establish_lower_bound (p, color, indices, depth);
  u = establish_upper_bound (p, color, indices, cond_affexpr, depth);
  return l && u;
}

/* Establish bounds for the loop indices and conditionals
   that enclose node n.
   Currently, conditionals are treated as non-affine.
   If we come across a non-affine expression, and the color is red,
      quit bounding and return 0.
   If we successfully bound everything, or the color is black,
      return 1
 */
static int
bound_indices_and_conditionals (Problem * p, range * indices, range * steps,
				range * non_i, int color, P_ExprExtForDD a)
{
  int which_step = r_first (steps);

  PO_Context c;

  c = Get_ExprExtForDD_first_context (a);
  while (!PO_Context_is_done (c))
    {
      assert (Get_Loop_iv (Get_PO_Context_loop (c)));
      if (!bound_index (p, color, c, indices) && (color == red))
	{
	  assert (which_step <= r_last (steps) + 1);
	  return 0;
	}
      c = Get_PO_Context_next (c);
    }
  assert (which_step <= r_last (steps) + 1);
  return 1;
}

static equate_descr
equate_subscripts (Problem * p, range * i1, range * i2, range * non_i,
		   int color, P_ExprExtForDD access1, P_ExprExtForDD access2)
{
  int complete = 1;
  uint c;
  int i;
  PO_Subscript sub1;
  PO_Subscript sub2;

  sub1 = Get_ExprExtForDD_first_subi (access1);
  sub2 = Get_ExprExtForDD_first_subi (access2);

  while (!PO_Subscript_is_done (sub1))
    {
      assert (!PO_Subscript_is_done (sub2));
      /* must have same # of subscripts */

      /* BCC - debug - 11/12/96 assert(sub_i_cur_affine(sub1) != NIL
         && sub_i_cur_affine(sub2) != NIL); */
      assert (Get_PO_Subscript_affexpr (sub1) &&
	      Get_PO_Subscript_affexpr (sub2));
      assert (!(Get_PO_Subscript_affexpr (sub1)->other_branch) &&
	      !(Get_PO_Subscript_affexpr (sub2)->other_branch));
      c = prob_add_zero_EQ (p, color);
      add_coefficients (&p->_EQs[c], i1, 1, Get_PO_Subscript_affexpr (sub1));
      add_coefficients (&p->_EQs[c], i2, -1, Get_PO_Subscript_affexpr (sub2));

      for (i = p->_nVars; i > 0; i--)
	if (p->_EQs[c].coef[i] != 0)
	  break;
      if (i == 0)
	{			/* linearity = 0 */
	  if (p->_EQs[c].coef[0] != 0)
	    return 0;		/* never equal */
	  p->_numEQs--;		/* always equal: don't add equation */
	}
      sub1 = Get_PO_Subscript_next (sub1);
      sub2 = Get_PO_Subscript_next (sub2);
    }

  assert (PO_Subscript_is_done (sub2));	/* must have same # of subscripts */
  return complete;
}

/* clear the tags for symbolic constants */
static void
delta_cleanup (delta_prob_desc * dpd, Alpha_var_id omega_vars[])
{
  int v;

  for (v = 0; v < r_length (&dpd->nonloops); v++)
    {
      Alpha_var_ids_tag (omega_vars[v + r_first (&dpd->nonloops)]) = UNTAGGED;
    }
}


Expr
P_DD_qualified_for_omega_test (Expr expr)
{
  P_ExprExtForDD acc;
  PO_Subscript si;
  Expr array_var;

  if (!(acc = Get_ExprExtForDD (expr)))
    return NULL;
  if (!Get_ExprExtForDD_first_context (acc))
    return NULL;
  if (!(array_var = Get_ExprExtForDD_array_var (acc)))
    return NULL;

  si = Get_ExprExtForDD_first_subi (acc);
  while (!PO_Subscript_is_done (si))
    {
      assert (Get_PO_Subscript_affexpr (si));
      if (!PO_Subscript_is_affine (si))
	return NULL;
      si = Get_PO_Subscript_next (si);
    }

  return array_var;
}


uint
Find_Accesses_Common_Loop_Depth (P_ExprExtForDD oa, P_ExprExtForDD ia)
{
  PO_Context oc, ic;
  uint od = 0, id = 0, bnest;

  /*
   * GEH - the following assumes that all enclosing contexts are LOOP
   * contexts, not IF contexts. (note the assert statements)
   */
  oc = Get_ExprExtForDD_first_context (oa);
  ic = Get_ExprExtForDD_first_context (ia);
  while ((oc != ic) && (!PO_Context_is_done (oc))
	 && (!PO_Context_is_done (ic)))
    {
      assert (Get_Loop_iv (Get_PO_Context_loop (oc)));
      assert (Get_Loop_iv (Get_PO_Context_loop (ic)));

      od = Get_PO_Context_depth (oc);
      id = Get_PO_Context_depth (ic);
      if (od >= id)
	oc = Get_PO_Context_next (oc);
      if (od <= id)
	ic = Get_PO_Context_next (ic);
    }

  if (PO_Context_is_done (oc) || PO_Context_is_done (ic))
    {
      bnest = 0;
    }
  else
    {
      bnest = (oc == ic) ? Get_PO_Context_depth (oc) : 0;
    }

  if (bnest > maxCommonNest)
    {
      fprintf (Ferr,
	       "Find_Accesses_Common_Loop_Depth : Error, too many nested loops\n");
      exit (-1);
    }
  return bnest;
}




/******************************************************************************
        These functions are called after omega
******************************************************************************/

static bool
Expr1IsParentOfExpr2 (Expr expr1, Expr expr2)
{
  assert (expr1 && expr2);
  while (expr2)
    {
      if (expr1 == expr2)
	return 1;
      expr2 = expr2->parentexpr;
    }
  return 0;
}

static bool
DD_Expr_Precedes_Within_Same_BB (Expr expr1, Expr expr2, Expr parentexpr1,
				 Expr parentexpr2, PC_Block bb, P_DepType dt)
{
  Expr expr;
  _PC_ExprIter ei;

  if (expr1 == expr2)
    {
      assert (dt == DT_OUTPUT);
      return 1;
    }
  if (parentexpr1 == parentexpr2)
    {
      if (parentexpr1->opcode == OP_assign)
	{
	  if (Expr1IsParentOfExpr2 (parentexpr1->operands, expr1))
	    {
	      /*
	       * assume expr1 = ... expr2 ...
	       */
	      assert (Expr1IsParentOfExpr2
		      (parentexpr1->operands->sibling, expr2));
	      assert (dt == DT_FLOW);
	      return 0;
	    }
	  else if (Expr1IsParentOfExpr2 (parentexpr1->operands, expr2))
	    {
	      /*
	       * assume expr2 = ... expr1 ...
	       */
	      assert (Expr1IsParentOfExpr2
		      (parentexpr1->operands->sibling, expr1));
	      assert (dt == DT_ANTI);
	      return 1;
	    }
	  else
	    P_punt ("DD_Expr_Precedes_Within_Same_BB: no LHS expr");
	}
      else
	P_punt ("DD_Expr_Precedes_Within_Same_BB: not OP_assign");
    }
  for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
       expr = PC_ExprIterNext (&ei, 1))
    {
      if (Expr1IsParentOfExpr2 (expr, parentexpr1))
	return 1;
      if (Expr1IsParentOfExpr2 (expr, parentexpr2))
	return 0;
    }
  P_punt ("DD_Expr_Precedes_Within_Same_BB: expr not in BB");
  return 0;			/* to warning */
}

static bool
DD_BB1_Precedes_BB2_Within_Same_Loop (PC_Block bb1, PC_Block bb2)
{
  if (Set_in (Get_BlockExtForVgraph_lex_pred (bb2), bb1->ID))
    return TRUE;
  return FALSE;
}

static Expr
OuterMostParentAssignment (Expr expr)
{
  assert (expr != NULL);
  while ((expr->opcode != OP_assign) && expr->parentexpr)
    expr = expr->parentexpr;
  return expr;
}

static bool
DD_Acc1_Precedes_Acc2_Within_Common_Loop (Expr expr1, Expr expr2,
					  P_DepType dt)
{
  Expr parentexpr1;
  Expr parentexpr2;
  PC_Block bb1;
  PC_Block bb2;

  assert ((expr1 != NULL) && (expr2 != NULL));

  bb1 = Get_ExprExtForVgraph_bb (expr1);
  assert (bb1);

  bb2 = Get_ExprExtForVgraph_bb (expr2);
  assert (bb2);
  parentexpr1 = OuterMostParentAssignment (expr1);
  parentexpr2 = OuterMostParentAssignment (expr2);
  if (bb1 == bb2)
    return DD_Expr_Precedes_Within_Same_BB (expr1, expr2, parentexpr1,
					    parentexpr2, bb1, dt);
  return DD_BB1_Precedes_BB2_Within_Same_Loop (bb1, bb2);
}


static void
dir_and_dist_into_DepInfo (dir_and_dist_info * ddi, P_DepInfo dep)
{
  int i;

  for (i = 0; i < Get_DepInfo_depth (dep); i++)
    {
      Set_DepInfo_known_i (dep, i, ddi->dist->distanceKnown[i + 1]);
      Set_DepInfo_dir_i (dep, i, ddextract1 (ddi->direction, i + 1));
      if (Get_DepInfo_dir_i (dep, i) == DDIR_EQ)
	Set_DepInfo_dist_i (dep, i, 0);
      else
	Set_DepInfo_dist_i (dep, i,
			    ddi->dist->distanceKnown[i +
						     1] ? ddi->dist->
			    distance[i + 1] : ddunknown);
    }
}

static int P_DD_arcid = 0;

static void
store_dependence (P_DepType dep_type, Expr from_expr, Expr to_expr,
		  dir_and_dist_info * d_info)
{
  P_DepList dep_list;
  P_DepInfo dep_source;
  P_DepInfo dep_target;

  if (DD_PRINT_OMEGA)
    {				/* CWL - 01/09/03 */
      fprintf (stderr, "<<<<< store_dependence:\n");
      dump_dependent_pair (stderr, dep_type, from_expr, to_expr);
      dump_d_info (stderr, d_info);
    }

  P_DD_arcid++;

  if (d_info)
    {
      dep_source = P_NewDepInfo (P_DD_arcid, to_expr->id, dep_type,
				 DEP_TAIL, d_info->dist->nest);
      dir_and_dist_into_DepInfo (d_info, dep_source);
    }
  else
    {
      dep_source = P_NewDepInfo (P_DD_arcid, to_expr->id, dep_type,
				 DEP_TAIL, 0);
    }
  dep_target = P_NewDepInfo (P_DD_arcid, from_expr->id, dep_type,
			     DEP_HEAD, 0);

  dep_list = DD_GetExprDepList (from_expr);
  Set_DepList_deps (dep_list, List_insert_first (Get_DepList_deps (dep_list),
						 dep_source));
  dep_list = DD_GetExprDepList (to_expr);
  Set_DepList_deps (dep_list, List_insert_first (Get_DepList_deps (dep_list),
						 dep_target));
  return;
}


/* store a set of flow, anti, or output dependencies

   if   we have a situation with an exposed 0+ followed
        by an exposed -,
   then filterValid will call itself recursively to split
        the 0+ into 0 and + (eliminating the 0...- case)
   else filterValid will store only one dependency

   flow dependencies are refined
 */

static int
filterValid (P_DepType nature, Expr from_expr, Expr to_expr,
	     int commonNesting, dir_and_dist_info * d_info, int allEQallowed)
{
  int less_at, j;
  int forbidden;

  /* remove all -'s before 1st + */
  for (j = 1; j <= commonNesting; ++j)
    {
      if (!dddirtest (d_info->direction, ddlt | ddeq, j))
	return 0;
      if (dddirtest (d_info->direction, ddgt, j))
	{
	  dddirreset (d_info->direction, ddgt, j);
	  dddirreset (d_info->restraint, ddgt, j);
	}
      if (dddirtest (d_info->direction, ddlt, j))
	break;
    }

  /* check for all 0's or no common Nesting */
  less_at = j;
  if (less_at > commonNesting)
    {
      if (allEQallowed)
	{
	  store_dependence (nature, from_expr, to_expr, d_info);
	  return 1;
	}
      else
	{
	  return 0;
	}
    }

  /* if we start with 0+ rather than just +, check for possible all 0's */

  if (dddirtest (d_info->direction, ddeq, less_at))
    {
      for (j = less_at + 1;
	   j <= commonNesting
	   && !dddirtest (d_info->direction, ddlt | ddgt, j); j++);
      if (j <= commonNesting
	  && !dddirtest (d_info->direction, ddlt | ddeq, j))
	{
	  /* we have some 0's, a 0+, more 0's, then a - so, 0+ -> just + */
	  dddirreset (d_info->direction, ddeq, less_at);
	  dddirreset (d_info->restraint, ddeq, less_at);
	}
      else
	{
	  /* we have some 0's, a 0+, more 0's,
	     then either 0- or something with a + */

	  forbidden = !allEQallowed;
	  for (j = commonNesting; j >= less_at; j--)
	    {
	      forbidden = dddirtest (d_info->direction, ddgt, j) ||
		(dddirtest (d_info->direction, ddeq, j) && forbidden);
	      /* "forbidden" = some loop outside j must
	         force this dependence to go strictly forward in time */
	    }

	  if (forbidden)
	    {
	      int cnt = 0;
	      /* split into leading 0 vs. leading + */
	      dir_and_dist_info plus, zero;
	      plus = zero = *d_info;
	      dddirreset (plus.direction, ddeq, less_at);
	      dddirreset (plus.restraint, ddeq, less_at);
	      dddirreset (zero.direction, ddlt, less_at);
	      dddirreset (zero.restraint, ddlt, less_at);
	      cnt += filterValid (nature, from_expr, to_expr, commonNesting,
				  &plus, allEQallowed);
	      cnt += filterValid (nature, from_expr, to_expr, commonNesting,
				  &zero, allEQallowed);
	      return cnt;
	    }
	}
    }

  if (nature == DT_FLOW)
    {
      dist_info tmp_dist;
      dir_and_dist_info tmp;
      int i;

      tmp_dist.nest = d_info->dist->nest;
      for (i = 1; i <= commonNesting; i++)
	{
	  tmp_dist.distance[i] = d_info->dist->distance[i];
	  tmp_dist.distanceKnown[i] = d_info->dist->distanceKnown[i];
	}
      tmp.direction = d_info->direction;
      tmp.restraint = d_info->restraint;
      tmp.dist = &tmp_dist;

#if ! defined SKIP_OMEGA2 || defined newTimeTrials
      if (!Flows_Covered
#ifdef newTimeTrials
	  && refineTimeTrials
#endif
	)
	{
	  refine_dependence (to_expr, from_expr, &tmp);
	}
#endif
      store_dependence (nature, from_expr, to_expr, &tmp);
    }
  else
    {
      /* we are doing an output or anti dependence */
      store_dependence (nature, from_expr, to_expr, d_info);
    }

  if (DD_PRINT_OMEGA)
    {
      fprintf (Flog, ">>>>>>>>>>>>>>>>>>>>>>>> (");
      for (j = 1; j <= commonNesting; j++)
	{
	  if (dddirtest (d_info->restraint, ddeq, j))
	    fprintf (Flog, "0");
	  if (dddirtest (d_info->restraint, ddlt, j))
	    fprintf (Flog, "+");
	  if (dddirtest (d_info->restraint, ddgt, j))
	    fprintf (Flog, "-");
	  if (j < commonNesting)
	    fprintf (Flog, ",");
	}
      fprintf (Flog, ")\n");
      fprintf (Flog, "------------------------ (");
      for (j = 1; j <= commonNesting; j++)
	{
	  if (dddirtest (d_info->direction, ddeq, j))
	    fprintf (Flog, "0");
	  if (dddirtest (d_info->direction, ddlt, j))
	    fprintf (Flog, "+");
	  if (dddirtest (d_info->direction, ddgt, j))
	    fprintf (Flog, "-");
	  if (j < commonNesting)
	    fprintf (Flog, ",");
	}
      fprintf (Flog, ")\n");
    }

  return 1;
}

static void
dd_to_debug (dir_and_dist_info * d_info)
{
  int j;

  fprintf (Flog, "(");
  for (j = 1; j <= d_info->dist->nest; j++)
    {
      if (d_info->dist->distanceKnown[j])
	fprintf (Flog, "%d", d_info->dist->distance[j]);
      else if (ddextract1 (d_info->direction, j) == ddall)
	fprintf (Flog, "*");
      else
	{
	  if (dddirtest (d_info->direction, ddeq, j))
	    fprintf (Flog, "0");
	  if (dddirtest (d_info->direction, ddlt, j))
	    fprintf (Flog, "+");
	  if (dddirtest (d_info->direction, ddgt, j))
	    fprintf (Flog, "-");
	}
      if (j < d_info->dist->nest)
	fprintf (Flog, ",");
    }
  fprintf (Flog, ") restraint = (");
  for (j = 1; j <= d_info->dist->nest; j++)
    {
      if (ddextract1 (d_info->restraint, j) == ddall)
	fprintf (Flog, "*");
      else
	{
	  if (dddirtest (d_info->restraint, ddeq, j))
	    fprintf (Flog, "0");
	  if (dddirtest (d_info->restraint, ddlt, j))
	    fprintf (Flog, "+");
	  if (dddirtest (d_info->restraint, ddgt, j))
	    fprintf (Flog, "-");
	}
      if (j < d_info->dist->nest)
	fprintf (Flog, ",");
    }
  fprintf (Flog, ")");
}


/*
   call filterValid for the dependence from access1 to access2,
   and if they are distinct, for the dependence the other way.
 */

static void
noteDependence (situation * sit, dir_and_dist_info * d_info)
{
  int r;
  dddirection deq, dgt, dlt, req, rgt, rlt;

  deq = ddfilter (d_info->direction, ddeq);
  dlt = ddfilter (d_info->direction, ddlt);
  dgt = ddfilter (d_info->direction, ddgt);
  req = ddfilter (d_info->restraint, ddeq);
  rlt = ddfilter (d_info->restraint, ddlt);
  rgt = ddfilter (d_info->restraint, ddgt);

  r = filterValid (sit->oitype, sit->expr1, sit->expr2,
		   sit->commonNesting, d_info,
		   /* GEH - added dependence type argument to
		      access_lexcally_preceeds */
		   DD_Acc1_Precedes_Acc2_Within_Common_Loop (sit->expr1,
							     sit->expr2,
							     sit->oitype));
  if (!r)
    {
      fprintf (stderr, "ALL FILTERED (1)\n");
      store_dependence (0, sit->expr1, sit->expr2, NULL);
    }

  if (DD_PRINT_OMEGA)
    {
      fprintf (Flog, "%%%%%%%%%%%%%%%%%%%%%%%% ");
      dd_to_debug (d_info);
      fprintf (Flog, "\n");
    }

  /* GEH - in order to correctly handle accesses which do both reads and
   *      writes, changed from:
   *          if (sit->access1 != sit->access2)
   *      to:
   */
  if (sit->expr1 != sit->expr2 || sit->iotype != sit->oitype)
    {
      dir_and_dist_info backward;
      dist_info back_dist;
      int j;

      backward.direction = backward.restraint = 0;

      ddsetfilter (backward.direction, deq, ddeq);
      ddsetfilter (backward.direction, dlt, ddgt);
      ddsetfilter (backward.direction, dgt, ddlt);
      ddsetfilter (backward.restraint, req, ddeq);
      ddsetfilter (backward.restraint, rlt, ddgt);
      ddsetfilter (backward.restraint, rgt, ddlt);

      backward.dist = &back_dist;
      back_dist.nest = d_info->dist->nest;
      for (j = 1; j <= sit->commonNesting; ++j)
	{
	  backward.dist->distanceKnown[j] = d_info->dist->distanceKnown[j];
	  backward.dist->distance[j] = -d_info->dist->distance[j];
	}

      /* GEH - added dependence type argument to access_lexcally_preceeds */

      r = filterValid (sit->iotype, sit->expr2, sit->expr1,
		       sit->commonNesting, &backward,
		       DD_Acc1_Precedes_Acc2_Within_Common_Loop (sit->expr2,
								 sit->expr1,
								 sit->
								 iotype));

      if (!r)
	{
	  fprintf (stderr, "ALL FILTERED (2)\n");
	  store_dependence (0, sit->expr2, sit->expr1, NULL);
	}

      if (DD_PRINT_OMEGA)
	{
	  fprintf (Flog, "%%%%%%%% backward %%%%%% ");
	  dd_to_debug (&backward);
	  fprintf (Flog, "\n");
	}
    }
}

/*
   process the omega test problem into dependence vectors
   if   dependencies are not coupled, just read them out.
   else break into cases for each possible dependence in (+,0,-)
        in each dimension by doing a recursive call
   each time a dependence vector is found, call noteDependence to store it
 */

static void
findDirectionVector (Problem * problemPtr, situation * sit,
		     dir_and_dist_info * d_info, unknowns * u_info)
{
  int i, j;
  int l, u, coupled;
  int l2, u2, best, score, bestScore;
  int unprotectThese[maxCommonNest + 1];
  int numToUnprotect = 0;
  int simplificationNeeded = u_info->unknownDirections == 0;
  int initialUnknownDirections = u_info->unknownDirections;

  u2 = 2;
  l2 = -2;
  bestScore = 10000;

#if ! defined NDEBUG
  best = -1;
#endif

  for (i = 0; i < u_info->unknownDirections; i++)
    {
      j = u_info->unknownDirection[i];
      d_info->dist->distanceKnown[j] = 0;
      coupled = queryVariable (problemPtr, j, &l, &u);
      if (l == u)
	{
	  d_info->dist->distanceKnown[j] = 1;
	  d_info->dist->distance[j] = l;
	}
      else
	{
	  if (l > 1)
	    l = 1;
	  else if (l < -1)
	    l = -1;
	  if (u < -1)
	    u = -1;
	  else if (u > 1)
	    u = 1;
	}
      if (!coupled || l == u)
	{
	  dddirsetonly (d_info->direction, 0, j);
	  if (l < 0)
	    dddirset (d_info->direction, ddgt, j);
	  if (l <= 0 && 0 <= u)
	    dddirset (d_info->direction, ddeq, j);
	  if (0 < u)
	    dddirset (d_info->direction, ddlt, j);
	  unprotectThese[numToUnprotect++] = j;
	  u_info->unknownDirection[i] =
	    u_info->unknownDirection[--u_info->unknownDirections];
	  i--;
	  if (coupled)
	    simplificationNeeded = 1;
	}
      else if (coupled && initialUnknownDirections == 1
	       && problemPtr->_nVars + problemPtr->_numSUBs == 2
	       && problemPtr->_numEQs + problemPtr->_numSUBs == 1)
	{
	  dddirsetonly (d_info->direction,
			queryVariableSigns (problemPtr, j,
					    ddlt, ddeq, ddgt,
					    negInfinity, posInfinity,
					    &(d_info->dist->distanceKnown[j]),
					    &(d_info->dist->distance[j])), j);
	  noteDependence (sit, d_info);
	  return;
	}
      else
	{
	  score = 2 * (u - l) + j;
	  if (problemPtr->_nVars > 1 && problemPtr->forwardingAddress[j] > 0)
	    score -= 3;
	  if (score <= bestScore)
	    {
	      u2 = u;
	      l2 = l;
	      best = j;
	      bestScore = score;
	    }
	}
    }

  if (u_info->unknownDirections == 0)
    {
      problemPtr->_safeVars = 0;
      problemPtr->_numSUBs = 0;
      if (!simplificationNeeded || solve (problemPtr, UNKNOWN))
	noteDependence (sit, d_info);
    }
  else
    {
      for (i = 0; i < numToUnprotect; i++)
	{
	  j = unprotectThese[i];
	  unprotectVariable (problemPtr, j);
	}
      if (simplificationNeeded ||
	  (u_info->unknownDirections == 1 && initialUnknownDirections > 1))
	{
	  simplifyProblem (problemPtr);
	  findDirectionVector (problemPtr, sit, d_info, u_info);
	}
      else
	{
	  if (u_info->unknownDirections == 2
	      && problemPtr->_nVars == 1 && problemPtr->_numSUBs == 1)
	    {
	      i = u_info->unknownDirection[0];
	      j = u_info->unknownDirection[1];
	      if (problemPtr->forwardingAddress[i] != -1)
		{
		  int t;
		  t = i;
		  i = j;
		  j = t;
		}
	      if (problemPtr->forwardingAddress[i] == -1
		  && problemPtr->forwardingAddress[j] == 1)
		{
		  int j1, j2, j3, i1, i2, i3;

		  j1 = ddlt;
		  i1 = queryVariableSigns (problemPtr, i,
					   ddlt, ddeq, ddgt, 1, posInfinity,
					   &(d_info->dist->distanceKnown[i]),
					   &(d_info->dist->distance[i]));
		  if (d_info->dist->distanceKnown[i])
		    {
		      dddirsetonly (d_info->direction, i1, i);
		      dddirsetonly (d_info->direction, j1, j);
		      /* dddirsetonly(d_info->restraint, i1, i);  -- not needed */
		      dddirsetonly (d_info->restraint, j1, j);
		      noteDependence (sit, d_info);
		      i1 = 0;
		    }
		  j2 = ddeq;
		  i2 =
		    queryVariableSigns (problemPtr, i, ddlt, ddeq, ddgt, 0, 0,
					&(d_info->dist->distanceKnown[i]),
					&(d_info->dist->distance[i]));
		  if (d_info->dist->distanceKnown[i])
		    {
		      dddirsetonly (d_info->direction, i2, i);
		      dddirsetonly (d_info->direction, j2, j);
		      /* dddirsetonly(d_info->restraint, i2, i); -- not needed */
		      dddirsetonly (d_info->restraint, j2, j);
		      noteDependence (sit, d_info);
		      i2 = 0;
		    }

		  j3 = ddgt;
		  i3 = queryVariableSigns (problemPtr, i,
					   ddlt, ddeq, ddgt, negInfinity, -1,
					   &(d_info->dist->distanceKnown[i]),
					   &(d_info->dist->distance[i]));
		  if (d_info->dist->distanceKnown[i])
		    {
		      dddirsetonly (d_info->direction, i3, i);
		      dddirsetonly (d_info->direction, j3, j);
		      /* dddirsetonly(d_info->restraint, i3, i); -- not needed */
		      dddirsetonly (d_info->restraint, j3, j);
		      noteDependence (sit, d_info);
		      i3 = 0;
		    }

		  d_info->dist->distanceKnown[i] = 0;
		  if (i3 == i2)
		    {
		      j2 |= j3;
		      i3 = 0;
		    }
		  if (i2 == i1)
		    {
		      j1 |= j2;
		      i2 = 0;
		    }
		  if (i3 == i1)
		    {
		      j1 |= j3;
		      i3 = 0;
		    }
		  if (i1)
		    {
		      dddirsetonly (d_info->direction, i1, i);
		      dddirsetonly (d_info->direction, j1, j);
		      /* dddirsetonly(d_info->restraint, i1, i); -- not needed */
		      dddirsetonly (d_info->restraint, j1, j);
		      noteDependence (sit, d_info);
		    }
		  if (i2)
		    {
		      dddirsetonly (d_info->direction, i2, i);
		      dddirsetonly (d_info->direction, j2, j);
		      /* dddirsetonly(d_info->restraint, i2, i); -- not needed */
		      dddirsetonly (d_info->restraint, j2, j);
		      noteDependence (sit, d_info);
		    }
		  if (i3)
		    {
		      dddirsetonly (d_info->direction, i3, i);
		      dddirsetonly (d_info->direction, j3, j);
		      /* dddirsetonly(d_info->restraint, i3, i); -- not needed */
		      dddirsetonly (d_info->restraint, j3, j);
		      noteDependence (sit, d_info);
		    }
		  return;
		}
	    }

	  {
	    int s;
	    int oldUnknownDirections;
	    int oldUnknownDirection[maxCommonNest + 1];

	    assert (best >= 0);
	    if (DD_PRINT_OMEGA)
	      fprintf (Flog,
		       "doing recursive analysis of %d..%d on variable %s (#%d)\n",
		       l2, u2, getVarName (best), best);
	    j = best;
	    i = 0;
	    while (u_info->unknownDirection[i] != j)
	      i++;
	    u_info->unknownDirection[i] =
	      u_info->unknownDirection[--u_info->unknownDirections];

	    oldUnknownDirections = u_info->unknownDirections;
	    for (i = 0; i < u_info->unknownDirections; i++)
	      oldUnknownDirection[i] = u_info->unknownDirection[i];

	    for (s = l2; s <= u2; s++)
	      {
		Problem tmpProblem;
		dddirection oldDirection = d_info->direction;
		dddirection oldRestraint = d_info->restraint;
		problemcpy (&tmpProblem, problemPtr);
		if (DD_PRINT_OMEGA)
		  fprintf (Flog, "considering sign =  %d of %s (%d)\n",
			   s, getVarName (best), best);

		if (s == 0)
		  {
		    d_info->dist->distance[j] = 0;
		    d_info->dist->distanceKnown[j] = 1;
		  }
		else
		  {
		    d_info->dist->distanceKnown[j] = 0;
		  }

		dddirsetonly (d_info->direction, dd_convert[s + 1], j);
		dddirsetonly (d_info->restraint, dd_convert[s + 1], j);

		if (constrainVariableSign (&tmpProblem, black, j, s))
		  findDirectionVector (&tmpProblem, sit, d_info, u_info);

		if (s < u2)
		  {
		    d_info->direction = oldDirection;
		    d_info->restraint = oldRestraint;
		    u_info->unknownDirections = oldUnknownDirections;
		    for (i = 0; i < u_info->unknownDirections; i++)
		      u_info->unknownDirection[i] = oldUnknownDirection[i];
		  }
	      }
	  }
	}
    }
}


/*
   calculateDDVectors just calls findDirectionVector now.

   The arrays dddir[] and dddist[] need to have at least
   maxnest+1 spaces allocated
 */

extern int nonConvex;


static void
calculateDDVectors (Problem * problemPtr, Expr expr1, Expr expr2,
		    P_DepType oitype, P_DepType iotype, unsigned int nest1,
		    unsigned int nest2, unsigned int bnest,
		    unsigned int nonloops)
{
  int i;
  situation sit;
  unknowns u_info;
  dir_and_dist_info d_info;
  dist_info _dist;

  d_info.dist = &_dist;
  _dist.nest = bnest;

  u_info.unknownDirections = bnest;
  for (i = 0; i < u_info.unknownDirections; i++)
    u_info.unknownDirection[i] = i + 1;

#ifdef newTimeTrials
  if (storeResult)
    {
      int e;
      int reducable = ((problemPtr->_safeVars) == 1);
      int coupledSubstitutions = 0;
      int coupled = 0;
      int nonUnary = 0;

      for (e = problemPtr->_numSUBs - 1; e >= 0; e--)
	if (problemPtr->_SUBs[e].coef[0] != 0)
	  reducable = 0;

      for (e = problemPtr->_numGEQs - 1; e >= 0; e--)
	{
	  if (!singleVarGEQ (problemPtr->_GEQs[e], problemPtr->_nVars))
	    coupled = 1;

	  for (i = problemPtr->_nVars; i > 0; i--)
	    if (problemPtr->_GEQs[e].coef[i] > 1
		|| problemPtr->_GEQs[e].coef[i] < -1)
	      nonUnary = 1;
	};

      for (e = problemPtr->_numSUBs - 1; e >= 0; e--)
	{
	  for (i = problemPtr->_nVars; i > 0; i--)
	    if (problemPtr->_SUBs[e].coef[i] != 0)
	      coupledSubstitutions = 1;
	};

      if (reducable)
	strncat (ddCategory, "r ", TINYBUFSIZ);
      if (coupledSubstitutions)
	strncat (ddCategory, "s ", TINYBUFSIZ);
      if (coupled)
	strncat (ddCategory, "c ", TINYBUFSIZ);
      if (nonUnary)
	strncat (ddCategory, "u ", TINYBUFSIZ);
      if (nonConvex)
	strncat (ddCategory, "v ", TINYBUFSIZ);
    }
#endif
  sit.expr1 = expr1;
  sit.expr2 = expr2;
  sit.oitype = oitype;
  sit.iotype = iotype;
  sit.nest1 = nest1;
  sit.nest2 = nest2;
  sit.commonNesting = bnest;

  d_info.direction = 0;
  d_info.restraint = -1;
  /* d_info built by findDirectionVector */
  findDirectionVector (problemPtr, &sit, &d_info, &u_info);
}


/*
 * Before calling dd_omega_test, each access should have subscriptor
 * and context set up.  Perform the omega test on the array accesses
 * access1 and access2 see dddriver.h for a description of "dd"
 * arguments.  */
void
P_DD_omega_test (Expr expr1, Expr expr2, P_DepType oitype, P_DepType iotype,
		 unsigned int nest1, unsigned int nest2, unsigned int bnest)
{
  Problem prob;
  delta_prob_desc dpd;
  Alpha_var_id omega_vars[maxVars], consts[maxVars], vars1[maxVars],
    vars2[maxVars], steps1[maxVars], steps2[maxVars];
  int Nconsts, Nsteps1, Nsteps2, subs_may_equal;
  P_ExprExtForDD access1, access2;

  if (DD_DEBUG_OMEGA)
    fprintf (stderr, "\n<<<<< dd_omega_test\n");

  access1 = Get_ExprExtForDD (expr1);
  access2 = Get_ExprExtForDD (expr2);

#ifdef newTimeTrials
  if (storeResult)
    omegaTests++;
#endif

  assert (oitype != DT_ANTI);

#if defined OMIT_DDS_FOR_TOPLEVEL
  /* we are only interested in parallelizing loops,
     and thus don't care about data dependencies between things
     that don't share any common loops */

  if (bnest == 0)
    return;
#endif

  /* PART 1: find sets of variables to be used in problem */

  Nsteps1 = Nsteps2 = Nconsts = 0;

  load_bounds (access1, vars1);
  load_bounds (access2, vars2);
  load_constants_for_bounds (access1, consts, &Nconsts);
  load_constants_for_bounds (access2, consts, &Nconsts);
  load_constants_for_subscripts (access1, consts, &Nconsts);
  load_constants_for_subscripts (access2, consts, &Nconsts);

  /* PART 2: assign columns to variables */

  P_DD_delta_init (&dpd, &prob, omega_vars, black, bnest,
		   nest1, vars1, nest2, vars2, Nconsts, consts,
		   Nsteps1, steps1, Nsteps2, steps2);

  assert (bnest == prob._safeVars);

  /* PART 3: build problem */

  /* first set globals so printing of debug info will work */
  current_set_of_vars = omega_vars;
  current_dpd = &dpd;
  current_getVarName = delta_getVarName;

  bound_indices_and_conditionals (&prob, &dpd.access1s, &dpd.steps1,
				  &dpd.nonloops, black, access1);
  bound_indices_and_conditionals (&prob, &dpd.access2s, &dpd.steps2,
				  &dpd.nonloops, black, access2);
  subs_may_equal = equate_subscripts (&prob, &dpd.access1s, &dpd.access2s,
				      &dpd.nonloops, black, access1, access2);

  /* PART 4: clean up */

#if ! defined NDEBUG
  delta_inv (&dpd, &prob, omega_vars);
#endif
  delta_cleanup (&dpd, omega_vars);

  /* PART 5: do omega test */

  /* copy problem to be used in refinement */
  problemcpy (&deltas, &prob);

  if (DD_PRINT_OMEGA)
    printProblem (&prob);

  if (subs_may_equal && simplifyProblem (&prob))
    {
      /* call omega test to add directions */
      calculateDDVectors (&prob, expr1, expr2, oitype, iotype,
			  nest1, nest2, bnest, r_length (&dpd.nonloops));
    }

#ifdef newTimeTrials
  if (storeResult)
    realOmegaTests++;
#endif

  assert (current_set_of_vars == omega_vars);
  assert (current_dpd == &dpd);
  current_dpd = 0;
  current_set_of_vars = 0;	/* TO DO: where is it used? */

  return;
}


static void
dump_d_info (FILE * file, dir_and_dist_info * d_info)
{
  int i;

  if (d_info == NULL)
    return;
  fprintf (file, "\ndump_d_info() -----------\n");
  fprintf (file, "\t     nest = %d\n", d_info->dist->nest);
  fprintf (file, "\tdirection = [ ");

  dump_direction (file, d_info->dist->nest, d_info->direction);
  fprintf (file, "]\n");
  fprintf (file, "\tdistKnown = [ ");
  for (i = 1; i <= d_info->dist->nest; i++)
    fprintf (file, "%d  ", d_info->dist->distanceKnown[i]);
  fprintf (file, "]\n");
  fprintf (file, "\t  distant = [ ");
  for (i = 1; i <= d_info->dist->nest; i++)
    {
      if (d_info->dist->distanceKnown[i])
	fprintf (file, "%d  ", d_info->dist->distance[i]);
      else
	fprintf (file, "-  ");
    }
  fprintf (file, "]\n");
  fprintf (file, "\trestraint = %x\n", d_info->restraint);
}

static void
dump_direction (FILE * file, int nnest, dddirection direction)
{
  int i;
  int dir;
  int m_dir;

  dir = direction;
  for (i = 1; i <= nnest; i++)
    {
      m_dir = dir & MASK0F;
      switch (m_dir)
	{
	case ddlt:
	  fprintf (file, "<  ");
	  break;
	case ddeq:
	  fprintf (file, "=  ");
	  break;
	case ddgt:
	  fprintf (file, ">  ");
	  break;
	case ddall:
	  fprintf (file, "*  ");
	  break;
	case (ddlt | ddeq):
	  fprintf (file, "<=  ");
	  break;
	case (ddgt | ddeq):
	  fprintf (file, ">=  ");
	  break;
	case (ddlt | ddgt):
	  fprintf (file, "<>  ");
	  break;
	default:
	  fprintf (file, "%x   ", m_dir);
	  break;
	}
      dir >>= 4;
    }
}


static void
dump_dependent_pair (FILE * file, P_DepType nature, Expr from_expr,
		     Expr to_expr)
{
  fprintf (file, "%10s from line %5d,%5d: Expr(%5d):",
	   get_DDtypename (nature),
	   P_GetStmtLineno (P_GetExprParentStmt (from_expr)),
	   P_GetStmtColno (P_GetExprParentStmt (from_expr)),
	   P_GetExprID (from_expr));
  Gen_CCODE_Expr (file, from_expr);
  fprintf (file, "\n           to   line %5d,%5d: Expr(%5d):",
	   P_GetStmtLineno (P_GetExprParentStmt (to_expr)),
	   P_GetStmtColno (P_GetExprParentStmt (to_expr)),
	   P_GetExprID (to_expr));
  Gen_CCODE_Expr (file, to_expr);
  fprintf (file, "\n");
}



