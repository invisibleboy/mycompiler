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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library/llist.h>
#include <library/i_types.h>
#include <library/dynamic_symbol.h>
#include <library/set.h>
#include <Pcode/perror.h>
#include <Pcode/pcode.h>
#include <Pcode/parms.h>
#include <Pcode/cast.h>
#include <Pcode/query.h>
#include <Pcode/struct.h>
#include <Pcode/ss_ssa.h>
#include <Pcode/ss_induct.h>
#include <Pcode/ss_setup.h>
#include <Pcode/symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/dom.h>
#include <Pcode/gen_ccode.h>
#include "dd_setup.h"

/******************************************************************************
 	Global
******************************************************************************/

#define DEBUG_DD_PREPROCESS 0

_P_AffineExpr Alpha_not_affine;
PC_Graph DD_curr_cfg;		/* should be hidden in FuncDcl */

/******************************************************************************
        Static/private variables
******************************************************************************/

static bool EVdebug = 0;
static int N_min_or_max;

static Key dd_curr_func_scope;

/******************************************************************************
        external interface 
******************************************************************************/
/*
 * low leve functions defined in Plib_DD/dd_data-dep.c
 */
extern bool Is_Min_Func_Name (char *func_name);
extern bool Is_Max_Func_Name (char *func_name);
extern bool Is_MinMax_Func_Name (char *func_name);

/*
 * syntax tree manipulation function defined in Pcode/struct.c
 */
extern ITintmax IntegralExprValue (Expr expr);

extern P_ExprExtForDD Get_ExprExtForDD (Expr expr);

/*
 * cfg
 */
extern PC_Flow PC_Connect (PC_Block p_bb, PC_Block s_bb, Expr cond,
			   double weight);
extern void PC_RemoveFlow (PC_Flow pc_flow);

/******************************************************************************
        Static/private function header
******************************************************************************/

static PO_Subscript New_PO_Subscript (void);
static void evaluate (Expr e, ITintmax * val, bool * known);
static uint m_linearity (P_AffineExpr ae, Min_or_max min_or_max, Expr expr);
static ITintmax find_coef (Expr expr, which_branch l_or_r,
			   Alpha_var_id tiny_var);
static P_AffineExpr Alpha_find_affine_expr (Expr e, Min_or_max mm);
static unsigned char DD_Does_Aff_Expr_Have_Mod_Syms (P_AffineExpr ae);
static int DD_Set_Subi_For_Index (Expr acc_expr);
static void DD_AlphaFunction (FuncDcl func);
static Expr ExpandValueToExpr (P_Value value);
static int DD_same_array_type (Type type1, Type type2);
static void Free_PO_Subscript (PO_Subscript subi);
static bool HaveNVars (int n);
static void SeparateArrayFromIndex (P_Value value, Expr * array_var,
				    Expr * array_index);

/*
 * potential cfg stuff
 */

static void InsertDummyInitialization (PC_Graph cfg);
static void RemoveDummyInitialization (PC_Graph cfg);
static void FindBBLexicalPredecessor (PC_Graph cfg);
static PC_Loop GetCFG_outer_loop_list (PC_Graph cfg);
static int HasPredecessor (PC_Block bb);
static int HasSuccessor (PC_Block bb);
static void BBInsertExpr (PC_Block bb, Expr expr);

/******************************************************************************
        Export function body
******************************************************************************/

/*
 * doing this after pointer analysis
 */
void
DD_Preprocess (FuncDcl func)
{
  P_Vgraph vgraph;

  dd_curr_func_scope = P_GetFuncDclKey (func);

  /*
   * build control flow graph
   */
  if (DD_curr_cfg)
    PC_FreeGraph (DD_curr_cfg);
  DD_curr_cfg = PC_Function (func, 2, 0);

#if DEBUG_DD_PREPROCESS
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.bb.DD_Preprocess.0", func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    fprintf (out_file, "DD_Preprocess ......\n\n");
    P_CF_Dump_All_BB (out_file, DD_curr_cfg, "");
    fclose (out_file);
  }
#endif

  InsertDummyInitialization (DD_curr_cfg);

#if DEBUG_DD_PREPROCESS
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.bb.DD_Preprocess.1", func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    fprintf (out_file, "DD_Preprocess ......\n\n");
    P_CF_Dump_All_BB (out_file, DD_curr_cfg, "");
    fclose (out_file);
  }
#endif

  P_CF_BuildSSA (DD_curr_cfg);

#if DEBUG_DD_PREPROCESS
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.bb.DD_Preprocess.2", func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    fprintf (out_file, "DD_Preprocess ......\n\n");
    P_CF_Dump_All_BB (out_file, DD_curr_cfg, "");
    fclose (out_file);
  }
#endif

  /*
   * find loops
   */
  PC_FindLoops (DD_curr_cfg);
  /*
   * set up PC_Block and PC_loop extension
   */
  SS_Init (DD_curr_cfg);
  /*
   * build a directed graph of Vnodes
   */
  vgraph = P_SS_BuildVgraph (DD_curr_cfg);
  /*
   * find Strongly Connected Components of Vnodes
   */
  P_SS_FindSccInVgraph (vgraph);

#if DEBUG_DD_PREPROCESS
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.scc.DD_Preprocess.0", func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    fprintf (out_file, "DD_Preprocess ......\n\n");
    P_SS_DumpScc (out_file, vgraph->scc_list, 0);
    fclose (out_file);
  }
#endif

  /*
   * find induction variables among all SCC's
   */
  P_SS_FindValuesInCFG (DD_curr_cfg);

#if DEBUG_DD_PREPROCESS
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.vgraph.DD_Preprocess.0", func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    fprintf (out_file, "DD_Find_Induction_Function ......\n\n");
    P_SS_DumpVgraph (out_file, vgraph);
    fclose (out_file);
  }
#endif

#if DEBUG_DD_PREPROCESS
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.loop.DD_Preprocess.0", func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    fprintf (out_file, "DD_Preprocess ......\n\n");
    P_CF_DumpLoopsInCFG (out_file, DD_curr_cfg);
    fclose (out_file);
  }
#endif
  /*
   * clean up
   */
  P_CF_DeleteSSA (DD_curr_cfg);
  /*
   * remove dummy entry and exit blocks
   */
  RemoveDummyInitialization (DD_curr_cfg);

  /*
   * find lexical predecessors
   */
  FindBBLexicalPredecessor (DD_curr_cfg);

  /*
   * Alpha to Omega
   */
  DD_AlphaFunction (func);

  return;
}

void
Alpha_punt (char *mesg, Stmt stmt)
{
  if (stmt != NULL)
    P_punt ("# %s in file %s on line %u\n", mesg,
	    stmt->filename, stmt->lineno);
  else
    P_punt ("# %s\n", mesg);
  exit (-1);
}

/******************************************************************************
        Static function body
******************************************************************************/

/*
 * low level affine expr manipulation functions defined in Plib_DD/dd_affine.c
 */
static P_AffineExpr
New_P_AffineExpr (void)
{
  P_AffineExpr new;
  int i;

  new = ALLOCATE (_P_AffineExpr);
  new->nterms = 0;
  new->other_branch = 0;
  for (i = 0; i < maxVars; i++)
    {
      new->terms[i].tiny_var = NULL;
      new->terms[i].coefficient = 0;
    }
  return new;
}


static void
Remove_P_AffineExpr (P_AffineExpr ae)
{
  int i;

  if (!ae)
    return;
  if (Alpha_is_affine (ae))
    {
      for (i = 0; i < ae->nterms; i++)
	{
	  if (ae->terms[i].tiny_var != NULL)
	    ae->terms[i].tiny_var = NULL;
	  ae->terms[i].coefficient = 0;
	}
      if (ae->other_branch != NULL)
	{
	  Remove_P_AffineExpr (ae->other_branch);
	  ae->other_branch = NULL;
	}
      ae->nterms = 0;
      DISPOSE (ae);
    }
}


static PO_Subscript
New_PO_Subscript (void)
{
  PO_Subscript new;

  new = ALLOCATE (_PO_Subscript);
  new->sub_expr = NULL;
  new->affexpr = NULL;
  new->mod_sub_var = NO_SUB_VAR_MOD;
  new->next = NULL;
  return new;
}


static void
Free_PO_Subscript (PO_Subscript subi)
{				/* TO DO */
  if (!subi)
    return;

  if (Get_PO_Subscript_next (subi))
    Free_PO_Subscript (Get_PO_Subscript_next (subi));
  DISPOSE (subi);
}


static bool
HaveNVars (int n)
{
  return (n < maxVars);
}

#if 0
static PO_Subscript
DD_Concat_Subi (PO_Subscript sub, PO_Subscript new_sub)
{
  PO_Subscript si;

  if (sub == NULL)
    return (new_sub);
  if (new_sub == NULL)
    return (sub);

  si = sub;

  while (si->next != NULL)
    si = si->next;

  si->next = new_sub;

  return (sub);
}
#endif


static void
evaluate (Expr e, ITintmax *val, bool *known)
{
  ITintmax chval;
  char *func_name;
  Expr le;

  if (!e)
    return;

  switch (e->opcode)
    {
    case OP_int:
      /* constant term */
      *val = e->value.scalar;
      break;

    case OP_compexpr:
      assert (e->operands->sibling == NULL);
      /* GEH - skip all expressions in compound expr except last one */
      le = e->operands;
      while (le->next != NULL)
	le = le->next;
      evaluate (le, val, known);
      break;

    case OP_add:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val += chval;
	}
      break;

    case OP_neg:
      assert (e->operands->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	*val = -*val;
      break;

    case OP_sub:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val -= chval;
	}
      break;

    case OP_mul:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val *= chval;
	}
      break;

    case OP_mod:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val %= chval;
	}
      break;

    case OP_div:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val /= chval;
	}
      break;

    case OP_lshft:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val <<= chval;
	}
      break;

    case OP_rshft:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val >>= chval;
	}
      break;

    case OP_inv:
      assert (e->operands->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	*val = ~*val;
      break;

    case OP_and:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val &= chval;
	}
      break;

    case OP_or:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val |= chval;
	}
      break;

    case OP_xor:
      assert (e->operands->sibling->sibling == NULL);
      evaluate (e->operands, val, known);
      if (*known)
	{
	  evaluate (e->operands->sibling, &chval, known);
	  if (*known)
	    *val ^= chval;
	}
      break;

    case OP_call:
      /* GEH - case for min or max function */
      assert (e->operands->sibling == NULL ||
	      e->operands->sibling->sibling == NULL);

      if (e->operands->opcode != OP_var)
	{
	  *known = 0;
	}
      else
	{
	  func_name = P_GetExprVarName (P_GetExprOperands (e));
	  if (Is_MinMax_Func_Name (func_name))
	    {

	      assert (e->operands->sibling->next != NULL);
	      assert (e->operands->sibling->next->next == NULL);

	      evaluate (e->operands->sibling, val, known);
	      if (*known)
		{
		  evaluate (e->operands->sibling->next, &chval, known);
		  if (*known)
		    {
		      if (Is_Min_Func_Name (func_name))
			*val = (chval < *val) ? chval : *val;
		      else
			*val = (chval > *val) ? chval : *val;
		    }
		}
	    }
	  else
	    *known = 0;
	}
      break;

    case OP_cast:
      assert (e->operands->sibling == NULL);
      evaluate (e->operands, val, known);
      break;

    case OP_var:
      {
	P_Value value;

	if (P_GetExprFlags (e) & EF_ENUM)
	  {
	    *val = P_GetExprScalar (e);
	    break;
	  }

	value = Get_ExprExtForVgraph_value (e);
	if (value && Value_is_known (value) && Value_is_integer (value))
	  {
	    *val = Value_integer (value);
	    break;
	  }

	/* else if var, fall through */
      }
    default:
      /* anything else is treated as nonlinear */
      *known = 0;
    }

  if (EVdebug)
    {
      printf ("results of evaluating " ITintmaxhexfmt " are "
	      ITintmaxformat " (known=%d)\n",
	      (ITintmax) (long) e, *val, (uint) (*known));
    }

  return;
}


static uint
m_linearity (P_AffineExpr ae, Min_or_max min_or_max, Expr expr)
{
  Expr ch, last_expr;
  uint lin, l;
  ITintmax val;
  bool known;
  char *func_name;
  Alpha_var_id vid;

  assert (expr != NULL);
  switch (expr->opcode)
    {

      /* THESE CASES MUST MATCH THOSE IN FIND_COEF */

    case OP_int:
      /* constant term */
      return 0;
      break;

    case OP_compexpr:
      assert (expr->operands->sibling == NULL);
      /* GEH - skip all expressions in compound expression except last one */
      last_expr = expr->operands;
      while (last_expr->next != NULL)
	last_expr = last_expr->next;
      return m_linearity (ae, min_or_max, last_expr);
      break;

    case OP_var:
      {
	STRING_Symbol *string_symbol;

	if (P_GetExprFlags (expr) & EF_ENUM)
	  return 0;
	string_symbol = P_SS_FindStringSymbol (P_GetExprVarName (expr));
	if (string_symbol)
	  {
	    vid = STRING_Symbol_data (string_symbol);
	    assert (vid);
	    if (Alpha_var_ids_tag (vid) == UNTAGGED)
	      {
		if (!HaveNVars (ae->nterms + 1))
		  return 2;	/* too many vars to be linear */
		assert (ae->nterms > 0);

		Alpha_var_ids_tag (vid) = ae->nterms;

		ae->terms[(ae->nterms)++].tiny_var = vid;
	      }
	    else
	      {
		assert (Alpha_var_ids_tag (vid) < ae->nterms);
		/* Note that in Alpha_var_id, we add the "acc_name" field for this checking */
	      }
	    /* linear */
	    return 1;
	  }
	else
	  {
	    if (PSI_IsArrayType (PSI_ExprType (expr)))
	      return 0;
	  }
	return 2;
	break;
      }
    case OP_dot:
    case OP_arrow:
    case OP_indr:		/* DON'T HANDLE THIS FOR NOW */
      P_punt ("m_linearity: OP_dot, OP_arrow, OP_indr");
      return 2;
      break;			/* for code readibility only */

    case OP_sub:
      assert (expr->operands->sibling->sibling == NULL);

      lin = m_linearity (ae, min_or_max, expr->operands);
      l = m_linearity (ae, -min_or_max, expr->operands->sibling);
      return ((l > lin) ? l : lin);
      break;

    case OP_neg:
      assert (expr->operands->sibling == NULL);
      return m_linearity (ae, -min_or_max, expr->operands);
      break;

    case OP_add:
      assert (expr->operands->sibling->sibling == NULL);

      lin = m_linearity (ae, min_or_max, expr->operands);
      l = m_linearity (ae, min_or_max, expr->operands->sibling);
      return ((l > lin) ? l : lin);
      break;

    case OP_mul:
      assert (expr->operands->sibling->sibling == NULL);

      /* two more more indices multiplied is nonlinear */
      /* lin keeps track of linearity.
         l is maximum subexpression linearity, needed for omegaPrintResult */
      lin = l = 0;
      for (ch = expr->operands; ch != NULL; ch = ch->sibling)
	{
	  uint next_lin;
	  next_lin = m_linearity (ae, min_or_max, ch);
	  lin += next_lin;
	  if (next_lin > l)
	    l = next_lin;
	}

      if (DD_PRINT_OMEGA && lin > 1 && l <= 1)
	fprintf (stderr, "Nonlinear term due to multiplication\n");
      /* only print if we haven't already */

      return lin;
      break;

    case OP_lshft:
      /* handle case where being shifted left by integer constant amount */
      assert (expr->operands->sibling->sibling == NULL);

      known = TRUE;
      evaluate (expr->operands->sibling, &val, &known);
      if (!known)
	return 2;
      else
	return m_linearity (ae, min_or_max, expr->operands);
      break;

    case OP_call:

      /* GEH - case for min or max function */
      /* Need to change f2c.h so that min and max are functions instead
         of macros.  Pcode will change them back to macros before Hcode
         generation . */
      assert (expr->operands->sibling == NULL ||
	      expr->operands->sibling->sibling == NULL);
      assert (expr->operands->opcode == OP_var);

      if (expr->operands->opcode != OP_var)
	{
	  if (DD_PRINT_OMEGA)
	    fprintf (stderr, "Nonlinear term due to call\n");
	  return 2;
	}
      func_name = P_GetExprVarName (P_GetExprOperands (expr));
      assert (func_name != NULL);

      if ((min_or_max == posmin && Is_Min_Func_Name (func_name)) ||
	  (min_or_max == posmax && Is_Max_Func_Name (func_name)))
	{
	  N_min_or_max++;
	  assert (N_min_or_max == 1);

	  assert (expr->operands->sibling->next != NULL);
	  assert (expr->operands->sibling->next->next == NULL);

	  l = m_linearity (ae, none, expr->operands->sibling);
	  lin = m_linearity (ae, none, expr->operands->sibling->next);
	  return ((l > lin) ? l : lin);
	}
      else
	{
	  if (DD_PRINT_OMEGA)
	    fprintf (stderr, "Nonlinear term due to min/max or call\n");
	  return 2;
	}
      break;

    case OP_cast:
    case OP_addr:
      assert (expr->operands->sibling == NULL);
      return m_linearity (ae, min_or_max, expr->operands);
      break;

      /* GEH - should probably try to handle some of the below later */
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:

    case OP_rshft:
    case OP_div:
    case OP_index:
    default:
      /* anything else is treated as nonlinear */
      if (DD_PRINT_OMEGA)
	fprintf (stderr, "Nonlinear term:  operation not handled\n");

      return 2;
    }
}				/* Alpah_m_linearity */

static ITintmax
find_coef (Expr expr, which_branch l_or_r, Alpha_var_id tiny_var)
{
  ITintmax val;
  bool known;
  char *func_name;
  Expr last_expr;
  Alpha_var_id vid;

  assert (expr != NULL);

  switch (expr->opcode)
    {

      /* THESE CASES MUST MATCH THOSE IN M_LINEARITY */

    case OP_int:
      /* constant term */
      return expr->value.scalar;
      break;

    case OP_compexpr:
      assert (expr->operands->sibling == NULL);
      /* GEH - skip all expressions in compound expr except last one */
      last_expr = expr->operands;
      while (last_expr->next != NULL)
	last_expr = last_expr->next;
      return find_coef (last_expr, l_or_r, tiny_var);
      break;

    case OP_var:
      {
	STRING_Symbol *string_symbol;

	if (P_GetExprFlags (expr) & EF_ENUM)
	  {
	    return P_GetExprScalar (expr);
	  }
	/* return 1 if this is the var of interest, else zero */

	if (tiny_var == NULL)
	  return 0;		/* TO DO return offset */
	string_symbol = P_SS_FindStringSymbol (P_GetExprVarName (expr));
	if (!string_symbol)
	  P_punt ("find_coef: NULL string_symbol");
	vid = STRING_Symbol_data (string_symbol);
	if (!vid)
	  P_punt ("find_coef: NULL vid");
	return (!strcmp
		(Alpha_var_ids_name (vid), Alpha_var_ids_name (tiny_var)));
	break;
      }
    case OP_dot:
    case OP_indr:
    case OP_arrow:
      P_punt ("find_coef: OP_dot, OP_arrow, OP_indr");
      break;

    case OP_add:
      /* sum up */
      assert (expr->operands->sibling->sibling == NULL);
      return find_coef (expr->operands, l_or_r, tiny_var)
	+ find_coef (expr->operands->sibling, l_or_r, tiny_var);
      break;

    case OP_neg:
      assert (expr->operands->sibling == NULL);
      return -find_coef (expr->operands, l_or_r, tiny_var);
      break;

    case OP_sub:
      /* sum up */
      assert (expr->operands->sibling->sibling == NULL);
      return find_coef (expr->operands, l_or_r, tiny_var)
	- find_coef (expr->operands->sibling, l_or_r, tiny_var);
      break;

    case OP_mul:
      assert (expr->operands->sibling->sibling == NULL);
      return find_coef (expr->operands, l_or_r, tiny_var)
	* find_coef (expr->operands->sibling, l_or_r, tiny_var);
      break;

    case OP_lshft:
      assert (expr->operands->sibling->sibling == NULL);
      known = TRUE;
      evaluate (expr->operands->sibling, &val, &known);
      if (!known)
	{
	  assert (0 && "inconsistency in find_coef");
	}
      else
	{
	  assert (val >= 0);
	  return find_coef (expr->operands, l_or_r, tiny_var) * (1 << val);
	}
      break;

    case OP_cast:
    case OP_addr:
      assert (expr->operands->sibling == NULL);
      return find_coef (expr->operands, l_or_r, tiny_var);
      break;

    case OP_call:

      /* GEH - case for min or max function */
      assert (expr->operands->sibling == NULL ||
	      expr->operands->sibling->sibling == NULL);

      /* assertion is OK if m_linearity takes care of function pointers */
      assert (expr->operands->opcode == OP_var);
      func_name = P_GetExprVarName (P_GetExprOperands (expr));
      assert (func_name != NULL);

      if (Is_MinMax_Func_Name (func_name))
	{
	  assert (expr->operands->sibling->next != NULL);
	  assert (expr->operands->sibling->next->next == NULL);
	  switch (l_or_r)
	    {
	    case goleft:
	      return find_coef (expr->operands->sibling, panic, tiny_var);

	    case goright:
	      return find_coef (expr->operands->sibling->next, panic,
				tiny_var);

	    case panic:
	      assert (0 && "inconsistency in find_coef");
	    }
	}
      /* fall through path */

    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:

    case OP_rshft:
    case OP_div:
    case OP_index:
    default:
      /* anything else should not happen */
      assert (0 && "inconsistency in find_coef");
    }
  return 0;
}				/* find_coef */

/*
 * Find the affine expr associated with the given expr if one exists.
 * If so, return the affine expr, otherwise, return a pointer to not_affine.
 * Note that currently, this implies that there are no array accesses in n
 */
static P_AffineExpr
Alpha_find_affine_expr (Expr e, Min_or_max mm)
{
  P_AffineExpr ae;
  int k, t;

  N_min_or_max = 0;

  ae = New_P_AffineExpr ();

  ae->other_branch = 0;
  ae->terms[0].tiny_var = NULL;
  ae->nterms = 1;

  if (m_linearity (ae, mm, e) <= 1)
    {
      ae->terms[0].coefficient = k = find_coef (e, goleft, 0);
      for (t = 1; t < ae->nterms; t++)
	{
	  ae->terms[t].coefficient =
	    find_coef (e, goleft, (Alpha_var_id) ae->terms[t].tiny_var) - k;
	}

      assert (ae->terms[0].tiny_var == NULL);
      assert (ae->terms[ae->nterms].tiny_var == NULL);

      if (N_min_or_max == 0)
	{
	  ae->other_branch = 0;
	}
      else
	{
	  P_AffineExpr other;

	  assert (N_min_or_max == 1);
	  other = New_P_AffineExpr ();
	  ae->other_branch = other;

	  other->nterms = ae->nterms;
	  for (t = 1; t < ae->nterms; t++)
	    {
	      other->terms[t].tiny_var = ae->terms[t].tiny_var;
	    }

	  other->terms[0].coefficient = k = find_coef (e, goright, 0);
	  for (t = 1; t < ae->nterms; t++)
	    {
	      other->terms[t].coefficient =
		find_coef (e, goright,
			   (Alpha_var_id) other->terms[t].tiny_var) - k;
	    }

	  assert (other->terms[0].tiny_var == NULL);
	  assert (other->terms[ae->nterms].tiny_var == NULL);
	}
    }
  else
    {
      /* GEH - added to fix bug with tags - 6/7/3 */
      for (t = 1; t < ae->nterms; t++)
	{
	  Alpha_var_ids_tag (ae->terms[t].tiny_var) = UNTAGGED;
	}

      Remove_P_AffineExpr (ae);
      ae = &Alpha_not_affine;
      return ae;
    }

  for (t = 1; t < ae->nterms; t++)
    {
      Alpha_var_ids_tag (ae->terms[t].tiny_var) = UNTAGGED;
    }
  return ae;
}

static unsigned char
DD_Does_Aff_Expr_Have_Mod_Syms (P_AffineExpr ae)
{
  Alpha_var_id acc;

  int i;
  unsigned char ms = NO_SUB_VAR_MOD;

  assert (ae != NULL);

  for (i = 1; i < ae->nterms; i++)
    {
      acc = (Alpha_var_id) ae->terms[i].tiny_var;

      if (Alpha_var_ids_const_p (acc))
	{
	  /* BCC - commented until fully understand the code - 4/17/99 */
#if 0
	  if (Set_in (acc->sym_type->sym_entry->loop_nests_mod,
		      loop_nest_index))
	    ms |= SUB_VAR_MOD_IN_PARLOOP;

	  /* -1 is loop nest index for top level of function */
	  if (Set_in (acc->sym_type->sym_entry->loop_nests_mod, -1))
	    ms |= SUB_VAR_MOD_AT_TOP_LEV;
#endif
	}
    }

  if (ae->other_branch != NULL)
    return (ms | DD_Does_Aff_Expr_Have_Mod_Syms (ae->other_branch));
  else
    return ms;
}


/*
 * generate a new expr of 
 *
 *   incr_expr * ivar_expr + InductValueToExpr(init_value)
 */
static Expr
InductValueToExpr (P_Value iv)
{
  Alpha_var_id vid;
  P_Value init_value;
  Expr ivar_expr;
  Expr incr_expr;
  Expr incr_x_ivar_expr;
  Expr init_expr;
  Expr final_expr;

  init_value = Value_induct_init (iv);
  if (Value_is_unknown (init_value))
    return NULL;
  assert (P_GetExprOpcode (Value_induct_incr (iv)) == OP_int);
  vid = Value_vid (iv);

  ivar_expr = PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_var);
  P_SetExprVarName (ivar_expr, strdup (vid->name));
  P_SetExprVarKey (ivar_expr, Alpha_var_ids_key (vid));

  incr_expr = PSI_CopyExpr (Value_induct_incr (iv));

  if (P_GetExprScalar (incr_expr) == 1)
    {
      incr_x_ivar_expr = ivar_expr;
    }
  else
    {
      incr_expr = PSI_ScopeNewIntExpr (dd_curr_func_scope,
				       P_GetExprScalar (Value_induct_incr
							(iv)));

      incr_x_ivar_expr = PSI_ScopeNewExprWithOpcode (dd_curr_func_scope,
						     OP_mul);
      incr_x_ivar_expr = P_AppendExprOperands (incr_x_ivar_expr, incr_expr);
      incr_x_ivar_expr = P_AppendExprOperands (incr_x_ivar_expr, ivar_expr);
    }

  if (Value_is_const (init_value))
    {
      if (Value_is_integer (init_value) && (Value_integer (init_value) == 0))
	{
	  final_expr = incr_x_ivar_expr;
	}
      else
	{
	  init_expr = PSI_CopyExpr (Value_const (init_value));

	  final_expr =
	    PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_add);
	  final_expr = P_AppendExprOperands (final_expr, incr_x_ivar_expr);
	  final_expr = P_AppendExprOperands (final_expr, init_expr);
	}
    }
  else
    {
      assert (Value_is_induct (init_value));
      if (!(init_expr = InductValueToExpr (init_value)))
	{
	  PSI_RemoveExpr (incr_x_ivar_expr);
	  return NULL;
	}

      final_expr = PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_add);
      final_expr = P_AppendExprOperands (final_expr, incr_x_ivar_expr);
      final_expr = P_AppendExprOperands (final_expr, init_expr);
    }
  return final_expr;
}


static Expr
ExpandValueToExpr (P_Value value)
{
  Expr new_expr;

  assert (value);
  if (Value_is_unknown (value))
    return NULL;

  if (Value_is_induct (value))
    {
      new_expr = InductValueToExpr (value);
    }
  else
    {
      assert (Value_is_const (value));
      new_expr = PSI_CopyExpr (Value_const (value));
    }
  return new_expr;
}


static int
DD_same_array_type (Type type1, Type type2)
{
  Type curr_type1;
  Type curr_type2;

  curr_type1 = type1;
  curr_type2 = type2;
  while (PSI_IsArrayType (curr_type1) && PSI_IsArrayType (curr_type2))
    {
      if (P_GetExprScalar (PSI_GetTypeArraySize (curr_type1)) !=
	  P_GetExprScalar (PSI_GetTypeArraySize (curr_type2)))
	return 0;
      curr_type1 = PSI_GetTypeType (curr_type1);
      curr_type2 = PSI_GetTypeType (curr_type2);
    }
  if (PSI_IsPointerType (curr_type1) || PSI_IsPointerType (curr_type2))
    return 0;
  if (!PSI_MatchType (curr_type1, curr_type2))
    return 0;
  return 1;
}


/* 
 * Find the array and subscript sub-expressions from the expr
 */
static int
DD_Set_Subi_For_Index (Expr expr)
{
  Type curr_type, ref_type, inner_type;
  Expr curr_expr, prev_expr, array_var, array_index, subi_expr, op1, op2;
  P_ExprExtForDD acc;
  PO_Subscript last_sub, new_sub;
  P_Value value;
  int depth = 0, valid = 1;

  assert (Get_ExprExtForDD (expr) && P_GetExprExtForVgraph (expr));
  acc = Get_ExprExtForDD (expr);
  /*
   * find the array/pointer variable. 
   */
  depth = 0;
  curr_expr = expr;

  do
    {
      switch (P_GetExprOpcode (curr_expr))
	{
	case OP_index:
	case OP_cast:
	  curr_expr = P_GetExprOperands (curr_expr);
	  depth++;
	  break;
	case OP_sub:
	  op1 = P_GetExprOperands (curr_expr);
	  op2 = P_GetExprSibling (op1);
	  if (PSI_IsPointerTypeExpr (op1) && PSI_IsIntegralTypeExpr (op2))
	    curr_expr = op1;
	  else
	    return 0;
	  depth++;
	  break;
	case OP_add:
	  op1 = P_GetExprOperands (curr_expr);
	  op2 = P_GetExprSibling (op1);
	  if (PSI_IsPointerTypeExpr (op1) && PSI_IsIntegralTypeExpr (op2))
	    curr_expr = op1;
	  else if (PSI_IsIntegralTypeExpr (op1) && PSI_IsPointerTypeExpr (op2))
	    curr_expr = op2;
	  else
	    return 0;
	  depth++;
	  break;
	default:
	  valid = 0;
	  break;
	}
    }
  while (valid);

  /*
   * check the dcltr of curr_expr, which must be (D_ARRY)+ or (D_PTR)(D_ARRY)*
   */
  assert (curr_expr);
  if (P_GetExprOpcode (curr_expr) == OP_indr)
    {
      assert (P_GetExprOperands (curr_expr));
      ref_type = PSI_ExprType (P_GetExprOperands (curr_expr));
    }
  else
    {
      ref_type = PSI_ExprType (curr_expr);
    }

  assert (PSI_IsPointerType (ref_type) || PSI_IsArrayType (ref_type));

  curr_type = PSI_GetTypeType (ref_type);
  while (PSI_IsArrayType (curr_type))
    curr_type = PSI_GetTypeType (curr_type);

  if (PSI_IsPointerType (curr_type))
    return 0;

  if (P_GetExprOpcode (curr_expr) == OP_indr)
    {
      value = Get_ExprExtForVgraph_value (P_GetExprOperands (curr_expr));
      if ((value == NULL) || Value_is_unknown (value))
	return 0;
      SeparateArrayFromIndex (value, &array_var, &array_index);
      if ((array_var == NULL) || (array_index == NULL))
	return 0;
      /*
       * set the 1st index
       */
      last_sub = New_PO_Subscript ();
      Set_PO_Subscript_sub_expr (last_sub, array_index);
      Set_ExprExtForDD_first_subi (acc, last_sub);

      curr_type = PSI_GetTypeType (PSI_ExprType (array_var));
    }
  else
    {
      value = Get_ExprExtForVgraph_value (curr_expr);
      if (value == NULL)
	{
	  if (DD_DEBUG_OMEGA)
	    fprintf (stderr, "\nWarning: DD_Set_Subi_For_Index(): "
		     "NULL value of curr_expr\n");
	  return 0;
	}
      if (Value_is_unknown (value))
	{
	  if (DD_DEBUG_OMEGA)
	    fprintf (stderr, "\nWarning: DD_Set_Subi_For_Index(): "
		     "unknown value of curr_expr\n");
	  return 0;
	}
      if (!Value_is_const (value))
	{
	  if (DD_DEBUG_OMEGA)
	    fprintf (stderr, "\nWarning: DD_Set_Subi_For_Index(): "
		     "!Value_is_const(value)\n");
	  return 0;
	}
      if (P_GetExprOpcode (Value_const (value)) != OP_var)
	{
	  if (DD_DEBUG_OMEGA)
	    fprintf (stderr, "\nWarning: DD_Set_Subi_For_Index(): "
		     "P_GetExprOpcode(array_expr) != OP_var\n");
	  return 0;
	}

      array_var = PSI_CopyExpr (Value_const (value));

      curr_type = PSI_ExprType (array_var);

      last_sub = NULL;
    }

  /*
   * set the array
   */

  Set_ExprExtForDD_array_var (acc, array_var);
  prev_expr = curr_expr;
  curr_expr = curr_expr->parentexpr;
  assert (depth >= 0);
  subi_expr = NULL;
  while (depth--)
    {
      _Opcode opc = P_GetExprOpcode (curr_expr);

      switch (opc)
	{
	case OP_cast:
	  if (!PSI_IsPointerTypeExpr (curr_expr) ||
	      !DD_same_array_type (PSI_GetTypeType (curr_type),
				   PSI_GetTypeType (PSI_ExprType
						    (curr_expr))))
	    {
	      if (DD_DEBUG_OMEGA)
		fprintf (stderr, "\nDD_Set_Subi_For_Index(): "
			 "OP_cast DD_same_array_dcltr\n");
	      goto return_0;
	    }
	  break;
	case OP_index:
	case OP_sub:
	case OP_add:
	  {
	    Expr op1 = curr_expr->operands, op2 = op1->sibling;
	  
	    if (opc == OP_add)
	      {
		if (PSI_IsPointerTypeExpr (op1) &&
		    PSI_IsIntegralTypeExpr (op2))
		  {
		    inner_type = PSI_ExprType (op1);
		    if (!DD_same_array_type (PSI_GetTypeType (curr_type),
					     PSI_GetTypeType (inner_type)))
		      {
			if (DD_DEBUG_OMEGA)
			  fprintf (stderr, "\nDD_Set_Subi_For_Index(): OP_add:"
				   " Pointer+Integer, DD_same_array_dcltr\n");
			goto return_0;
		      }
		    subi_expr = op2;
		  }
		else if (PSI_IsIntegralTypeExpr (op1) &&
			 PSI_IsPointerTypeExpr (op2))
		  {
		    inner_type = PSI_ExprType (op2);
		    if (!DD_same_array_type (PSI_GetTypeType (curr_type),
					     PSI_GetTypeType (inner_type)))
		      {
			if (DD_DEBUG_OMEGA)
			  fprintf (stderr, "\nDD_Set_Subi_For_Index(): OP_add:"
				   " Integer+Pointer, DD_same_array_dcltr\n");
			goto return_0;
		      }
		    subi_expr = op1;
		  }
		else
		  {
		    P_punt ("DD_Set_Subi_For_Index(): OP_add:"
			    " invalid comb. of pointer and integral types");
		  }
	      }
	    else if (opc == OP_sub)
	      {
		if (!(PSI_IsPointerTypeExpr (op1) &&
		      DD_same_array_type (PSI_GetTypeType (curr_type),
					  PSI_GetTypeType (PSI_ExprType
							   (P_GetExprOperands
							    (curr_expr))))))
		  {
		    if (DD_DEBUG_OMEGA)
		      fprintf (stderr, "\nDD_Set_Subi_For_Index(): OP_sub:"
			       " DD_same_array_dcltr\n");
		    goto return_0;
		  }
		subi_expr = op2;
	      }
	    else
	      {			/* curr_expr->opcode == OP_index */
		assert (PSI_IsArrayTypeExpr (P_GetExprOperands (curr_expr)) ||
			PSI_IsPointerTypeExpr (P_GetExprOperands (curr_expr)));
		if (PSI_IsPointerTypeExpr (P_GetExprOperands (curr_expr)))
		  {
		    if (!DD_same_array_type (PSI_GetTypeType (curr_type),
					     PSI_GetTypeType (PSI_ExprType 
							      (op1))))
		      {
			if (DD_DEBUG_OMEGA)
			  fprintf (stderr, "\nDD_Set_Subi_For_Index(): "
				   "OP_index, D_PTR DD_same_array_dcltr\n");
			goto return_0;
		      }
		  }
		else if (PSI_IsArrayTypeExpr (P_GetExprOperands (curr_expr)))
		  {
		    if (!DD_same_array_type (curr_type,
					     PSI_ExprType (op1)))
		    {
		      fprintf (stderr, "\nDD_Set_Subi_For_Index(): "
			       "OP_index, D_ARRY DD_same_array_dcltr\n");
		      goto return_0;
		    }
		  }
		else
		  {
		    P_punt ("DD_Set_Subi_For_Index(): OP_index, "
			    "invalid dcltr method");
		  }
		subi_expr = op2;
	      }

	    value = Get_ExprExtForVgraph_value (subi_expr);
	    if (((value == NULL) && (P_GetExprOpcode (subi_expr) != OP_int)) ||
		Value_is_unknown (value))
	      goto return_0;

	    new_sub = New_PO_Subscript ();
	    if (P_GetExprOpcode (subi_expr) == OP_int)
	      Set_PO_Subscript_sub_expr (new_sub, PSI_CopyExpr (subi_expr));
	    else
	      Set_PO_Subscript_sub_expr (new_sub, ExpandValueToExpr (value));

	    if (last_sub == NULL)
	      {
		Set_ExprExtForDD_first_subi (acc, new_sub);
		last_sub = new_sub;
	      }
	    else
	      {
		Set_PO_Subscript_next (last_sub, new_sub);
		last_sub = new_sub;
	      }

	    curr_type = PSI_GetTypeType (curr_type);
	  }
	  break;
	default:
	  if (DD_DEBUG_OMEGA)
	    fprintf (stderr, "\nDD_Set_Subi_For_Index: invalid opcode\n");
	  goto return_0;
	}
      prev_expr = curr_expr;
      curr_expr = curr_expr->parentexpr;
    }

  if (prev_expr != expr)
    {
      if (DD_DEBUG_OMEGA)
	fprintf (stderr, "\nDD_Set_Subi_For_Index: prev_expr != expr\n");
      goto return_0;
    }

  if (PSI_IsArrayType (curr_type) || PSI_IsPointerType (curr_type))
    {
      if (DD_DEBUG_OMEGA)
	fprintf (stderr, "\nDD_Set_Subi_For_Index: curr_dcltr != NULL\n");
      goto return_0;
    }

  return 1;

return_0:

  Free_PO_Subscript (Get_ExprExtForDD_first_subi (acc));
  Set_ExprExtForDD_first_subi (acc, NULL);
  return 0;
}

int
Alpha_DD_Subscript_Expr (Expr expr)
{
  PO_Subscript si;

  si = Get_ExprExtForDD_first_subi (Get_ExprExtForDD (expr));

  while (!PO_Subscript_is_done (si))	/* sub_i_done (si)) */
    {
      Set_PO_Subscript_affexpr (si, Alpha_find_affine_expr (Get_PO_Subscript_sub_expr (si), none));	/* sub_i_cur_affine (si) */

      Set_PO_Subscript_mod_sub_var (si, DD_Does_Aff_Expr_Have_Mod_Syms (Get_PO_Subscript_affexpr (si)));	/* sub_i_cur_affine (si) */

      if (DD_DEBUG_OMEGA && !PO_Subscript_is_affine (si))	/* sub_i_cur_is_affine (si) */
	{
	  fprintf (Flog, "....Subscript expression not affine: [");
	  Gen_CCODE_Expr (Flog, Get_PO_Subscript_sub_expr (si));
	  fprintf (Flog, "]\n");
	}

      /* NEW_OMEGA */
      if (!PO_Subscript_is_affine (si))	/* sub_i_cur_is_affine (si)) */
	return 0;

      si = Get_PO_Subscript_next (si);	/* sub_i_next (si); */
    }
  return 1;
}


static void
SeparateArrayFromIndex (P_Value value, Expr * array_var, Expr * array_index)
{
  if (Value_is_const (value))
    {
      Expr parent_expr;
      Expr var_expr;
      Expr curr_expr;
      Expr prev_expr;
      Expr zero_expr;
      Expr value_expr;

      value_expr = PSI_CopyExpr (Value_const (value));

      assert (PSI_IsArrayTypeExpr (value_expr)
	      || PSI_IsPointerTypeExpr (value_expr));

      parent_expr = NULL;
      prev_expr = NULL;
      var_expr = value_expr;
      curr_expr = P_GetExprOperands (value_expr);
      while (curr_expr)
	{
	  /* 
	   * search for an expr of pointer/array type
	   */
	  prev_expr = NULL;

	  while (curr_expr
		 && !(PSI_IsArrayTypeExpr (curr_expr)
		      || PSI_IsPointerTypeExpr (curr_expr)))
	    {
	      prev_expr = curr_expr;
	      curr_expr = P_GetExprSibling (curr_expr);
	    }

	  if (!curr_expr)
	    {
	      fprintf (stderr,
		       "\nWARNING: SeparateArrayFromIndex: cannot find array variable.\n");
	      *array_var = NULL;
	      *array_index = NULL;
	      return;
	    }
	  parent_expr = var_expr;
	  var_expr = curr_expr;
	  /*
	   * double check that curr_expr is the only expr of pointer/array type
	   */
	  curr_expr = P_GetExprSibling (curr_expr);
	  while (curr_expr)
	    {
	      if (PSI_IsArrayTypeExpr (curr_expr)
		  || PSI_IsPointerTypeExpr (curr_expr))
		P_punt
		  ("SeparateArrayFromIndex: an pointer/array expr with more than two operands of pointer/array type");
	      curr_expr = P_GetExprSibling (curr_expr);
	    }
	  curr_expr = P_GetExprOperands (var_expr);
	}
      /*
       * at this point, 
       * var_expr --> the array var
       * prev_expr --> the sibling before var_expr
       * parent_expr --> the parent expr of var_expr and prev_expr
       */
      assert (var_expr);
      /*
       * get array index 
       */
      zero_expr = PSI_ScopeNewIntExpr (dd_curr_func_scope, 0);
      P_SetExprParentExpr (zero_expr, parent_expr);
      P_SetExprSibling (zero_expr, P_GetExprSibling (var_expr));
      if (parent_expr)
	{
	  if (prev_expr)
	    {
	      P_SetExprSibling (prev_expr, zero_expr);
	    }
	  else
	    {
	      assert (P_GetExprOperands (parent_expr) == var_expr);
	      P_SetExprOperands (parent_expr, zero_expr);
	    }
	  curr_expr = parent_expr;
	  while (curr_expr != value_expr)
	    {
	      PSI_CastExpr (curr_expr);
	      curr_expr = P_GetExprParentExpr (curr_expr);
	    }

	  PSI_CastExpr (curr_expr);
	  *array_index = curr_expr;
	}
      else
	{
	  assert (prev_expr == NULL);
	  *array_index = zero_expr;
	}
      /*
       * get array variable
       */
      var_expr->parentexpr = NULL;
      var_expr->sibling = NULL;
      var_expr->next = NULL;
      *array_var = var_expr;
    }
  else if (Value_is_induct (value))
    {
      Expr new_index_expr;
      Expr incr_expr;
      Expr ivar_expr;
      Expr incr_x_ivar_expr;
      Alpha_var_id vid;

      SeparateArrayFromIndex (Value_induct_init (value), array_var,
			      array_index);
      if ((*array_var == NULL) || (*array_index == NULL))
	return;
      assert (P_GetExprOpcode (Value_induct_incr (value)) == OP_int);
      incr_expr =
	PSI_ScopeNewIntExpr (dd_curr_func_scope,
			     P_GetExprScalar (Value_induct_incr (value)));
      vid = Value_vid (value);
      assert (vid);

      ivar_expr = PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_var);
      P_SetExprVarName (ivar_expr, strdup (Alpha_var_ids_name (vid)));
      P_SetExprVarKey (ivar_expr, Alpha_var_ids_key (vid));

      if (P_GetExprScalar (incr_expr) == 1)
	incr_x_ivar_expr = ivar_expr;
      else
	{
	  incr_expr =
	    PSI_ScopeNewIntExpr (dd_curr_func_scope,
				 P_GetExprScalar (incr_expr));

	  incr_x_ivar_expr =
	    PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_mul);
	  incr_x_ivar_expr =
	    P_AppendExprOperands (incr_x_ivar_expr, incr_expr);
	  incr_x_ivar_expr =
	    P_AppendExprOperands (incr_x_ivar_expr, ivar_expr);
	}

      new_index_expr =
	PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_add);
      new_index_expr = P_AppendExprOperands (new_index_expr, *array_index);
      new_index_expr =
	P_AppendExprOperands (new_index_expr, incr_x_ivar_expr);

      *array_index = new_index_expr;
    }
}

static void
SetContextForExpr (Expr expr, PO_Context context)
{
  switch (P_GetExprOpcode (expr))
    {
    case OP_index:
    case OP_indr:
      {
	if (!DD_Set_Subi_For_Index (expr))
	  break;

	if (Alpha_DD_Subscript_Expr (expr))
	  {
	    Set_ExprExtForDD_first_context (Get_ExprExtForDD (expr), context);
	  }
	break;
      }
    default:
      {
	if (P_GetExprOperands (expr))
	  SetContextForExpr (P_GetExprOperands (expr), context);
	break;
      }
    }
  if (P_GetExprSibling (expr))
    SetContextForExpr (P_GetExprSibling (expr), context);
  if (P_GetExprNext (expr))
    SetContextForExpr (P_GetExprNext (expr), context);
}

static PO_Context
New_PO_Context ()
{
  PO_Context c;

  c = ALLOCATE (_PO_Context);
  Set_PO_Context_loop (c, NULL);
  Set_PO_Context_cond_affexpr (c, NULL);
#if !USE_LOOP_NESTING_LEVEL
  Set_PO_Context_depth (c, -1);
#endif
  Set_PO_Context_next (c, NULL);
  return c;
}

static PO_Context
CreateNewContext (PC_Loop parloop, PO_Context last_context)
{
  Expr cond_expr;
  P_AffineExpr cond_ae;
  PO_Context context;

  if (Get_Loop_condition (parloop))
    cond_expr = ExpandValueToExpr (Get_Loop_condition (parloop));
  else
    {
      fprintf (stderr, "\nCreateNewContext: Loop_condition NULL, loop %d!!\n",
	       Get_Loop_id (parloop));
      cond_expr = NULL;
    }
  if (cond_expr)
    {
      cond_ae = Alpha_find_affine_expr (cond_expr, none);
      if (!Alpha_is_affine (cond_ae))
	{
	  fprintf (stderr,
		   "\nCreateNewContext: condition is not affine, loop %d\n",
		   Get_Loop_id (parloop));
	  cond_ae = NULL;

	  PSI_RemoveExpr (cond_expr);

	  Set_Loop_cond_expr (parloop, NULL);
	}
      else
	{
	  Set_Loop_cond_expr (parloop, cond_expr);
	}
    }
  else
    {
      cond_ae = NULL;
      Set_Loop_cond_expr (parloop, NULL);
    }
  context = New_PO_Context ();
  Set_PO_Context_loop (context, parloop);
  Set_PO_Context_cond_affexpr (context, cond_ae);
#if !USE_LOOP_NESTING_LEVEL
  if (last_context == NULL)
    Set_PO_Context_depth (context, 1);
  else
    Set_PO_Context_depth (context, Get_PO_Context_depth (last_context) + 1);
  Set_Loop_parloop_depth (parloop, Get_PO_Context_depth (context));
#endif
  Set_PO_Context_next (context, last_context);
  return context;
}

/*
 *   for i 
 *     for j
 *       A[i]	=> context: j->i
 *   for i
 *     for j
 *        for k
 *          A[i+j][k+j] => context: k->j->i
 *   loop
 *      i = i + 1;
 *      j = j + 1;
 *      A[i][j]
 *    approach: context (default loop bounds) derived from the loop condition, 
 *	          loop condition must be ind. expr. for the loop to be a parloop
 *  		  default induction variable: the induction var in loop condition.
 *              subscript must be compatible with context (same step/increment).
 *		  subscript is used to offset the loop bounds.
 */
static void
SetContextForParloop (PC_Graph cfg, PC_Loop loop, PO_Context context)
{
  Expr expr;
  PC_Block bb;
  _PC_ExprIter ei;
  int bb_array_size;
  int *bb_array;
  int i;

  if (loop == NULL)
    return;
  bb_array = (int *) calloc (cfg->num_bbs + 10, sizeof (int));
  if (bb_array == NULL)
    P_punt ("SetContextForParloop: calloc bb_set fail");
  bb_array_size = Set_2array (Get_Loop_body (loop), bb_array);
  for (i = 0; i < bb_array_size; i++)
    {
      bb = PC_FindBlock (cfg, bb_array[i]);
      for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
	   expr = PC_ExprIterNext (&ei, 1))
	SetContextForExpr (expr, context);
    }
  free (bb_array);
}


static void
SetContextForLoop (PC_Graph cfg, PC_Loop loop, PO_Context context)
{
  PC_Loop child;
  PO_Context new_context;

#if USE_LOOP_NESTING_LEVEL
  new_context = CreateNewContext (loop, context);
#else
  if (Get_Loop_iv (loop))
    {
      new_context = CreateNewContext (loop, context);
    }
  else
    new_context = context;
#endif
  SetContextForParloop (cfg, loop, new_context);
  child = Get_Loop_child (loop);
  while (child)
    {
      SetContextForLoop (cfg, child, new_context);
      child = Get_Loop_sibling (child);
    }
}


/*
 * For each loop qualified for Omega test, find affine expression for
 * array accesses and pointer accesses.  This function basically does
 * what DD_Find_All_Accesses_In_Stmts() does for Omega test.  */
static void
DD_AlphaFunction (FuncDcl func)
{
  PC_Graph cfg;
  PC_Loop loop;

  cfg = DD_GetFunctionCFG (func);
  assert (cfg != NULL);

  loop = GetCFG_outer_loop_list (cfg);
  while (loop)
    {
      SetContextForLoop (cfg, loop, NULL);
      loop = Get_Loop_sibling (loop);
    }
}


static void
InsertDummyInitialization (PC_Graph cfg)
{
  char name[1024];
  int et_types;
  PC_Block entry_bb;
  VarDcl var;
  FuncDcl func;
  Type type;
  SymbolTable symtab;
  SymTabEntry entry;
  Key key;
  Key scope_key;
  Expr lhs_expr;
  Expr rhs_expr;
  Expr assign_expr;

  symtab = PSI_GetTable ();

  /*
   * "initialize" parameters
   */
  entry_bb = cfg->first_bb;
  func = cfg->func;
  et_types = ET_VAR_LOCAL;
  scope_key = PSI_GetStmtScope (P_GetFuncDclStmt (func));

  for (key = PSI_GetFileEntryByType (scope_key.file, et_types);
       P_ValidKey (key); key = PSI_GetFileEntryByTypeNext (key, et_types))
    {
      if (!PST_ScopeContainsKey (symtab, scope_key, key))
	continue;
      entry = PSI_GetSymTabEntry (key);
      switch (P_GetSymTabEntryType (entry))
	{
	case ET_VAR_LOCAL:
	  var = P_GetSymTabEntryVarDcl (entry);
	  type = P_GetVarDclType (var);

	  if (PSI_IsArithmeticType (type) || PSI_IsPointerType (type))
	    {
	      /*
	       * LHS
	       */
	      lhs_expr =
		PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_var);
	      P_SetExprVarName (lhs_expr, strdup (P_GetVarDclName (var)));
	      P_SetExprVarKey (lhs_expr, P_GetVarDclKey (var));
	      P_SetExprType (lhs_expr, P_GetVarDclType (var));
	      /*
	       * RHS
	       */
#if 1				/* do this if filter out shadow */
	      if (P_GetVarDclQualifier (var) & VQ_PARAMETER)
		{
		  rhs_expr =
		    PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_var);
		  sprintf (name, "@%s", P_GetVarDclName (var));
		  P_SetExprVarName (rhs_expr, strdup (name));
		  P_SetExprVarKey (rhs_expr, P_GetVarDclKey (var));
		  P_SetExprType (rhs_expr, P_GetVarDclType (var));
		  P_SetExprPragma (rhs_expr,
				   P_NewPragmaWithSpecExpr ("ParamValue",
							    NULL));
		}
	      else
#endif
		{
		  rhs_expr = PSI_ScopeNewIntExpr (dd_curr_func_scope, 0);
		  P_SetExprPragma (rhs_expr,
				   P_NewPragmaWithSpecExpr ("UnknownValue",
							    NULL));
		}
	      /*
	       * LHS = RHS
	       */
	      assign_expr =
		PSI_ScopeNewExprWithOpcode (dd_curr_func_scope, OP_assign);
	      assign_expr = P_AppendExprOperands (assign_expr, lhs_expr);
	      assign_expr = P_AppendExprOperands (assign_expr, rhs_expr);
	      P_SetExprPragma (assign_expr,
			       P_NewPragmaWithSpecExpr ("DummyInit", NULL));
	      /*
	       * insert to the entry basic block
	       */
	      BBInsertExpr (entry_bb, assign_expr);
	    }
	  break;
	default:
	  break;
	}
    }
}


static void
RemoveDummyInitialization (PC_Graph cfg)
{
  PC_PStmt curr_ps;
  PC_PStmt prev_ps;
  PC_Block entry_bb;
  Expr expr;

  entry_bb = cfg->first_bb;
  curr_ps = entry_bb->first_ps;
  while (curr_ps)
    {
#if 0
      assert (P_GetStmtType (curr_ps->data.stmt) == ST_EXPR);
#endif
      expr = P_GetStmtExpr (curr_ps->data.stmt);
      prev_ps = curr_ps;
      curr_ps = curr_ps->succ;
      if (expr && P_FindPragma (P_GetExprPragma (expr), "DummyInit"))
	{
	  PSI_RemoveExpr (expr);
	  PC_DeletePStmt (entry_bb, prev_ps);
	}
    }
}


/*
 * should get CFG from FuncDcl
 */
PC_Graph
DD_GetFunctionCFG (FuncDcl func)
{
  assert (DD_curr_cfg != NULL);
  return DD_curr_cfg;
}

static void
FindBBLexicalPredecessor (PC_Graph cfg)
{
  PC_Block bb;
  PC_Block pred_bb;
  PC_Flow pred_link;
  /* Lptr list; */
  int change;

  /*
   * initialization 
   */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      if (Get_BlockExtForVgraph_lex_pred (bb))
	Set_dispose (Get_BlockExtForVgraph_lex_pred (bb));
      Set_BlockExtForVgraph_lex_pred (bb, NULL);
      for (pred_link = bb->p_flow; pred_link;
	   pred_link = pred_link->p_next_flow)
	{
	  pred_bb = pred_link->src_bb;
	  if (!PC_BB1DominatesBB2 (bb, pred_bb))
	    Set_BlockExtForVgraph_lex_pred (bb,
					    Set_add
					    (Get_BlockExtForVgraph_lex_pred
					     (bb), pred_bb->ID));
	}
    }
  /*
   * find lexical predecessors
   */
  do
    {
      change = 0;
      for (bb = cfg->first_bb; bb; bb = bb->next)
	{
	  for (pred_link = bb->p_flow; pred_link;
	       pred_link = pred_link->p_next_flow)
	    {
	      pred_bb = pred_link->src_bb;
	      if (!PC_BB1DominatesBB2 (bb, pred_bb))
		Set_BlockExtForVgraph_lex_pred (bb,
						Set_union_acc_ch
						(Get_BlockExtForVgraph_lex_pred
						 (bb),
						 Get_BlockExtForVgraph_lex_pred
						 (pred_bb), &change));
	    }
	}
    }
  while (change);
}

static PC_Loop
GetCFG_outer_loop_list (PC_Graph cfg)
{
  return cfg->lp_tree;
}

static int
HasPredecessor (PC_Block bb)
{
  return (bb->p_flow != NULL);
}

static int
HasSuccessor (PC_Block bb)
{
  return (bb->s_flow != NULL);
}

static void
BBInsertExpr (PC_Block bb, Expr expr)
{
  PC_NewPStmtExprBefore (PC_T_Expr, bb, bb->first_ps, expr);
}
