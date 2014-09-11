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

#include "ss_ind_expr.h"

#undef DEBUG_IV_CALC
#undef DEBUG_BOUNDS_ANALYSIS
#undef DEBUG_BOUNDS_CORRECTION
#undef PRINT_LOOP_BOUNDS

/* Do NOT enable linear-monotonic IVs at this time, code needs to be
 * rewritten due to grammar change. */
#undef DO_LIN_MON_IVS
#define CHECK_LIN_MON_REQUIREMENTS

#define MAX_PHI_OPERANDS 5

static PC_Graph current_CFG;

/* function defs */

#define Get_Fund_Ind_Var(l) (P_LoopFundInVar(l))
#define Get_TarLoop(iv) (((PSS_LoopExt) (iv)->ext)->tloop)

/* this used to be a function...now replaced... */
#define PC_BBInnerLoop(G,BBID) (PC_FindBlock(G,BBID)->loop)

static PSS_Add_Term PSS_Create_Add_Term (PSS_Add_Term);
static PSS_Add_Term PSS_Delete_Add_Term (PSS_Add_Term);
static PSS_Select_Term PSS_Create_Select_Term (PSS_Select_Term);
static PSS_Select_Term PSS_Delete_Select_Term (PSS_Select_Term);
extern PSS_Mul_Term PSS_Create_Mul_Term (PSS_Mul_Term);
static PSS_Mul_Term PSS_Delete_Mul_Term (PSS_Mul_Term);
static PSS_Lin_Mon_Info PSS_Create_Lin_Mon_Info (PSS_Lin_Mon_Info);
static PSS_Lin_Mon_Info PSS_Delete_Lin_Mon_Info (PSS_Lin_Mon_Info);
PSS_Expr PSS_Create_Expr (PSS_Expr);
PSS_Expr PSS_Delete_Expr (PSS_Expr);

static int PSS_Same_Add_Term (PSS_Add_Term, PSS_Add_Term);
extern int PSS_Same_Mul_Term (PSS_Mul_Term, PSS_Mul_Term);
int PSS_Same_Expr (PSS_Expr, PSS_Expr);
int PSS_Add_Term_Less_Than (PSS_Add_Term, PSS_Add_Term);
int PSS_Add_Term_Greater_Than (PSS_Add_Term, PSS_Add_Term);

static PSS_Expr PSS_Generic_Expr (Expr, int, PC_Loop, int);
static PSS_Expr PSS_Assign (Expr, int, PC_Loop, int);

static PSS_Expr PSS_Linear_IV (Expr, int, PC_Loop, PSS_TarSCC);
#ifdef DO_LIN_MON_IVS
static PSS_Expr PSS_Linear_Monotonic_IV (Expr, int, PC_Loop, PSS_TarSCC);
#endif
static PSS_Expr PSS_Unknown_IV (Expr, int);

static void PSS_Weaken_NE_Bound_Condition (PSS_Bound_Condition);
static List PSS_Add_Term_Loop_Bounds (PSS_Add_Term, List);
static List PSS_Select_Term_Loop_Bounds (PSS_Select_Term, List);
List PSS_Find_Relevant_Loop_Bounds (PSS_Expr, List);

void PSS_Print_Expr (FILE *, PSS_Expr);
static void PSS_Print_Select_Term (FILE *, PSS_Select_Term);
static void PSS_Print_Add_Term (FILE *, PSS_Add_Term);
static void PSS_Print_Mul_Term (FILE *, PSS_Mul_Term);
void PSS_Print_Loop_Bounds (FILE *, PSS_Bound_Condition);

/* static typedefs */

/* function decls */

static PSS_Bound_Condition
PSS_Create_Bound_Condition (PSS_Bound_Condition bound)
{
  PSS_Bound_Condition new_bound;

  new_bound = malloc (sizeof(_PSS_Bound_Condition));

  if (bound == NULL)
    {
      new_bound->cond = NULL;
      new_bound->cont_opcode = 0;
      new_bound->next = NULL;
      new_bound->bb = NULL;
      new_bound->pcloop = NULL;
    }
  else
    {
      new_bound->cond = PSS_Create_Expr (bound->cond);
      new_bound->cont_opcode = bound->cont_opcode;
      if (bound->next)
	new_bound->next = PSS_Create_Bound_Condition (bound->next);
      else
	new_bound->next = NULL;
      new_bound->bb = bound->bb;
      new_bound->pcloop = bound->pcloop;
    }
  return new_bound;
}

PSS_Bound_Condition
PSS_Delete_Bound_Condition (PSS_Bound_Condition bound)
{
  if (bound)
    {
      if (bound->next)
	bound->next = PSS_Delete_Bound_Condition (bound->next);
      bound->cond = PSS_Delete_Expr (bound->cond);
      bound->cont_opcode = 0;

      free (bound);
    }
  return NULL;
}

static void
PC_Create_Fund_Ind_Vars (PC_Loop loop)
{
  PSS_Fund_Ind_Var new_ind_var;
  for (; loop; loop = loop->next)
    {
      /* We want to save the contents of loop->ext: this should
       * contain SCC information.
       */
      new_ind_var = malloc (sizeof(_PSS_Fund_Ind_Var));
      new_ind_var->pcloop = loop;
      new_ind_var->bounds = NULL;
      new_ind_var->final_type = FV_UNKNOWN;
      new_ind_var->ext = loop->ext;
      P_LoopFundInVar(loop) = new_ind_var;
    }
}

/* Setup function before running Ind Expr calculation. */
void
PC_IndExpr_Setup (PC_Graph cfg)
{
  PC_Block bb;
  Expr expr;
  _PC_ExprIter ei;

  /* Links CFG to a global var */
  current_CFG = cfg;

  PSS_Find_SCCs (cfg);
  /* checks that the expression's all have NULL ind var extension fields */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    for (expr = PC_ExprIterFirst(bb, &ei, 1); expr;
	 expr = PC_ExprIterNext(&ei, 1))
      if (PSS_GetExprIE(expr) != NULL)
	P_punt("expr ext field for IE is already set!\n");

  /* Creates fund ind var structs for each loop */
  PC_Create_Fund_Ind_Vars (cfg->lp);
}


static void
PC_Delete_All_Ind_Vars (PC_Loop loop)
{
  PSS_Fund_Ind_Var fund_ind_var;

  for (; loop; loop = loop->next)
    {
      fund_ind_var = Get_Fund_Ind_Var (loop);
      fund_ind_var->bounds = PSS_Delete_Bound_Condition (fund_ind_var->bounds);
      P_LoopFundInVar(loop) = NULL;

      free (fund_ind_var);
    }
}

/* \brief Cleanup function after running all Ind Expr calculations. */
void
PC_IndExpr_Cleanup (PC_Graph cfg)
{
  PC_Block bb;
  Expr expr;
  _PC_ExprIter ei;

  /* Clear out ext fields (PSS_Fund_Ind_Var) of PC_Loops */
  PC_Delete_All_Ind_Vars (current_CFG->lp);
  current_CFG = NULL;

  /* Clear out ext fields (PSS_Expr) */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    for (expr = PC_ExprIterFirst(bb, &ei, 1); expr;
	 expr = PC_ExprIterNext(&ei, 1))
      PSS_SetExprIE(expr, PSS_Delete_Expr(PSS_GetExprIE(expr)));
}

static PSS_Lin_Mon_Info
PSS_Create_Lin_Mon_Info (PSS_Lin_Mon_Info info)
{
  PSS_Lin_Mon_Info new_info;

  new_info = malloc (sizeof (_PSS_Lin_Mon_Info));
  if (info == NULL)
    {
       new_info->min_incr = NULL;
       new_info->max_incr = NULL;
       new_info->visited = 0;
    }
  else
    {
      new_info->min_incr = PSS_Create_Select_Term (info->min_incr);
      new_info->max_incr = PSS_Create_Select_Term (info->max_incr);
      new_info->visited = info->visited;
    }

  return new_info;
}

static PSS_Lin_Mon_Info
PSS_Delete_Lin_Mon_Info (PSS_Lin_Mon_Info lm_info)
{
  lm_info->min_incr = PSS_Delete_Select_Term (lm_info->min_incr);
  lm_info->max_incr = PSS_Delete_Select_Term (lm_info->max_incr);

  free (lm_info);

  return NULL;
}

/**********************************
 * INDUCTION EXPRESSION FUNCTIONS *
 **********************************/

/* Constructors, Destructors, other utilities for the various expression
 * structures. */


/* --- Select Term Functions --- */

/* \brief Constructor */
static PSS_Select_Term
PSS_Create_Select_Term (PSS_Select_Term select_term)
{
  PSS_Select_Term new_term;

  new_term = malloc(sizeof(_PSS_Select_Term));

  if (select_term == NULL)
    {
      new_term->sel_type = SELECT_NONE;
      new_term->next = NULL;
      new_term->add = NULL;
    }
  else
    {
      new_term->sel_type = select_term->sel_type;
      if (select_term->next)
	new_term->next = PSS_Create_Select_Term (select_term->next);
      else
	new_term->next = NULL;

      if (select_term->add)
	new_term->add = PSS_Create_Add_Term (select_term->add);
      else
	new_term->add = NULL;
    }
  return new_term;
}


/* \brief Destructor */
static PSS_Select_Term
PSS_Delete_Select_Term (PSS_Select_Term select_term)
{
  if (select_term == NULL)
    return NULL;

  select_term->next = PSS_Delete_Select_Term (select_term->next);
  select_term->add = PSS_Delete_Add_Term (select_term->add);

  free (select_term);

  return NULL;
}


/* --- Add Term Functions --- */


/* \brief Constructor for single element of list */
static PSS_Add_Term
PSS_Create_Single_Add_Term (PSS_Add_Term old_add)
{
  PSS_Add_Term new_term;

  new_term = malloc(sizeof(_PSS_Add_Term));
  new_term->next = NULL;

  if (old_add == NULL)
    {
      new_term->int_coeff = 1;
      new_term->affine = 0;
      new_term->mul = NULL;
    }
  else
    {
      new_term->int_coeff = old_add->int_coeff;
      new_term->affine = old_add->affine;
      if (old_add->mul)
	new_term->mul = PSS_Create_Mul_Term (old_add->mul);
      else
	new_term->mul = NULL;
    }
  return new_term;
}


/* \brief Generic Constructor */
static PSS_Add_Term
PSS_Create_Add_Term (PSS_Add_Term old_add)
{
  PSS_Add_Term new_term;

  new_term = PSS_Create_Single_Add_Term (old_add);
  if (old_add && old_add->next)
    new_term->next = PSS_Create_Add_Term (old_add->next);

  return new_term;
}


#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
/* \brief Creates a PSS_Add_Term that points to a PSS_Select_Term.  Used for
 * min, max representation
 */
static PSS_Add_Term
PSS_Create_Select_Add (PSS_Add_Term add, PSS_Select_Type select_type)
{
  PSS_Add_Term new_add = PSS_Create_Add_Term (NULL);
  PSS_Mul_Term new_mul = PSS_Create_Mul_Term (NULL);
  PSS_Select_Term select = PSS_Create_Select_Term (NULL);

  new_add->mul = new_mul;

  new_mul->type = TERM_SELECT;
  new_mul->term.select = select;
  select->sel_type = select_type;
  if (add)
    select->add = PSS_Create_Add_Term (add);
  else
    {
      select->add = PSS_Create_Add_Term (NULL);
      select->add->int_coeff = 0;
      select->add->affine = 1;
    }

  return new_add;
}
#endif

/* Destructor: kills entire chain of add terms and whatever they contain. */
static PSS_Add_Term
PSS_Delete_Add_Term (PSS_Add_Term add_term)
{
  if (add_term == NULL)
    return NULL;

  add_term->next = PSS_Delete_Add_Term (add_term->next);
  add_term->mul = PSS_Delete_Mul_Term (add_term->mul);

  free (add_term);

  return NULL;
}


/* --- Mul Term Functions --- */

PSS_Mul_Term
PSS_Create_Mul_Term (PSS_Mul_Term mul_term)
{
  PSS_Mul_Term new_term;

  new_term = malloc(sizeof(_PSS_Mul_Term));

  if (mul_term == NULL)
    {
      new_term->final_value = 0;
      new_term->next = NULL;
      new_term->type = TERM_UNDEF;
      new_term->term.nothing = NULL;
    }
  else
    {
      new_term->final_value = mul_term->final_value;

      if (mul_term->next)
	new_term->next = PSS_Create_Mul_Term (mul_term->next);
      else
	new_term->next = NULL;

      new_term->type = mul_term->type;
      switch (new_term->type)
	{
	case TERM_SELECT:
	  new_term->term.select =
	    PSS_Create_Select_Term (mul_term->term.select);
	  break;
	case TERM_FUND_IND_VAR:
	  new_term->term.fund_ind_var = mul_term->term.fund_ind_var;
	  break;
	case TERM_LIN_MON_INFO:
	  new_term->term.lin_mon =
	    PSS_Create_Lin_Mon_Info (mul_term->term.lin_mon);
	  break;
	case TERM_SSA_DEF:
	  new_term->term.ssa_def = mul_term->term.ssa_def;
	  break;
	case TERM_PCODE_EXPR:
	  new_term->term.pcode_expr = mul_term->term.pcode_expr;
	  break;
	case TERM_UNDEF:
	default:
	  P_punt ("PSS_Create_Mul_Term: Unknown type %d on PSS_Mul_Term.",
		  new_term->type);
	}
    }

  return new_term;
}


/* Destructor: kills entire chain of mul terms and whatever they contain. */
static PSS_Mul_Term
PSS_Delete_Mul_Term (PSS_Mul_Term mul_term)
{
  if (mul_term == NULL)
    return NULL;

  mul_term->next = PSS_Delete_Mul_Term (mul_term->next);

  switch (mul_term->type)
    {
    case TERM_SELECT:
      mul_term->term.select = PSS_Delete_Select_Term (mul_term->term.select);
      break;
    case TERM_LIN_MON_INFO:
      mul_term->term.lin_mon =
	PSS_Delete_Lin_Mon_Info (mul_term->term.lin_mon);
      break;
    case TERM_FUND_IND_VAR:
    case TERM_SSA_DEF:
    case TERM_PCODE_EXPR:
      break;
    case TERM_UNDEF:
    default:
	  P_punt ("PSS_Delete_Mul_Term: Unknown type on "
		  "PSS_Mul_Term.");
    }
  mul_term->type = TERM_UNDEF;

  free (mul_term);

  return NULL;
}

/* --- Expr Functions --- */

/* IndExpr_Expr Constructor */
PSS_Expr
PSS_Create_Expr (PSS_Expr ind_expr)
{
  PSS_Expr new_ind_expr;

  new_ind_expr = malloc(sizeof(_PSS_Expr));

  if (ind_expr == NULL)
    {
      new_ind_expr->terms = PSS_Create_Add_Term (NULL);
      new_ind_expr->terms->int_coeff = 0;
      new_ind_expr->terms->affine = 1;
      new_ind_expr->flags = 0;
    }
  else
    {
      if (ind_expr->terms)
	new_ind_expr->terms =
	  PSS_Create_Add_Term (ind_expr->terms);
      else
	{
	  new_ind_expr->terms = PSS_Create_Add_Term (NULL);
	  new_ind_expr->terms->int_coeff = 0;
	  new_ind_expr->terms->affine = 1;
	}
      new_ind_expr->flags = ind_expr->flags;
    }
  return new_ind_expr;
}


/* Creates a PSS_Expr of integer value. */
PSS_Expr
PSS_Create_Int_Expr (int value)
{
  PSS_Expr ind_expr;
  PSS_Add_Term add_term;

  ind_expr = PSS_Create_Expr (NULL);
  add_term = ind_expr->terms;
  add_term->int_coeff = value;

  return ind_expr;
}


/* IndExpr Expr Destructor */
PSS_Expr
PSS_Delete_Expr (PSS_Expr ind_expr)
{
  if (ind_expr == NULL)
    return NULL;

  ind_expr->terms = PSS_Delete_Add_Term (ind_expr->terms);
  ind_expr->flags = 0;
  free (ind_expr);

  return NULL;
}


/* SER 20041130
 * Extracts the invariant coefficient from a PSS_Expr and return a new
 * PSS_Expr.  Copies the old expression and removes the add terms that
 * have the fundamental induction var as a mul term.
 */
PSS_Expr
PSS_Expr_Extract_Invar_Coeff (PSS_Expr old_expr, PC_Loop loop)
{
  PSS_Expr new_expr = PSS_Create_Expr (old_expr);
  PSS_Add_Term prev_add, add_term;

  prev_add = new_expr->terms;
  for (add_term = prev_add->next; add_term; prev_add = add_term,
	 add_term = add_term->next)
    {
      PSS_Mul_Term mul_term;
      int is_variant = 0;

      for (mul_term = add_term->mul; mul_term; mul_term = mul_term->next)
	{
	  if (mul_term->type != TERM_FUND_IND_VAR)
	    continue;
	  if (mul_term->term.fund_ind_var->pcloop != loop)
	    continue;
	  is_variant = 1;
	  break;
	}
      if (is_variant)
	{
	  prev_add->next = add_term->next;
	  add_term->next = NULL;
	  PSS_Delete_Add_Term (add_term);
	  add_term = prev_add;
	}
    }

  return new_expr;
}

/* SER 20041130
 * Extracts the invariant coefficient from a PSS_Expr and return a new
 * PSS_Expr. */
PSS_Expr
PSS_Expr_Extract_Ind_Coeff (PSS_Expr old_expr, PC_Loop loop)
{
  PSS_Expr new_expr = PSS_Create_Expr (old_expr);
  PSS_Add_Term prev_add, current_add;

  prev_add = new_expr->terms;
  prev_add->int_coeff = 0;  /* Clear out the constant term. */
  prev_add->affine = 1;
  for (current_add = prev_add->next; current_add; prev_add = current_add,
	 current_add = current_add->next)
    {
      PSS_Mul_Term prev_mul, current_mul;
      int is_variant = 0;

      /* Determine if the term depends on the fund ind var, remove it if it
       * exists. */
      if ((prev_mul = current_add->mul) == NULL)
	P_punt ("PSS_Expr_Extract_Ind_Coeff: found non-initial const term.");
      else if (prev_mul->type == TERM_FUND_IND_VAR &&
	  prev_mul->term.fund_ind_var->pcloop == loop)
	{
	  is_variant = 1;
	  current_add->mul = prev_mul->next;
	  prev_mul->next = NULL;
	  PSS_Delete_Mul_Term (prev_mul);
	  prev_mul = current_add->mul;
	  /* If the term consisted only of the fundamental ind var, combine
	   * the constant with the initial constant. */
	  if (prev_mul == NULL)
	    {
	      new_expr->terms->int_coeff += current_add->int_coeff;
	      prev_add->next = current_add->next;
	      current_add->next = NULL;
	      PSS_Delete_Add_Term (current_add);
	      current_add = prev_add;
	    }
	}
      else
	{
	  for (current_mul = prev_mul->next; current_mul;
	       prev_mul = current_mul, current_mul = current_mul->next)
	    {
	      if (current_mul->type != TERM_FUND_IND_VAR)
		continue;
	      if (current_mul->term.fund_ind_var->pcloop != loop)
		continue;
	      is_variant = 1;
	      prev_mul->next = current_mul->next;
	      current_mul->next = NULL;
	      PSS_Delete_Mul_Term (current_mul);
	      current_mul = prev_mul;
	    }
	}

      /* If there was no fund_ind_var in the add term, delete. */
      if (!(is_variant))
	{
	  prev_add->next = current_add->next;
	  current_add->next = NULL;
	  PSS_Delete_Add_Term (current_add);
	  current_add = prev_add;
	}
   }
  return new_expr;
}


/* --- EXPRESSION COMPUTATION FUNCTIONS --- */

static int PSS_Add_Term_Prioritize (PSS_Add_Term, PSS_Add_Term);

/* SER 20041201
 * Determines which of two select terms have higher priority. */
static int
PSS_Select_Term_Prioritize (PSS_Select_Term select1, PSS_Select_Term select2)
{
  PSS_Select_Term sel1, sel2;
  int result;

  sel1 = select1;
  sel2 = select2;

  while (sel1)
    {
      /* Shorter chains have higher priority */
      if (sel2 == NULL)
	return 0;
      result = PSS_Add_Term_Prioritize (sel1->add, sel2->add);
      if (result == 1)
	return 1;
      else if (result == -1)
	return -1;
      sel1 = sel1->next;
      sel2 = sel2->next;
    }
  /* Shorter chains have higher priority. */
  if (sel2 != NULL)
    return 1;
  return 0;
}


/* SER 20041201
 * Determines which of two INDIVIDUAL mul terms have higher priority. */
static int
PSS_Mul_Term_Prioritize (PSS_Mul_Term mul1, PSS_Mul_Term mul2)
{
  int val1, val2;
  Expr var1, var2;

  switch (mul1->type)
    {
    case TERM_FUND_IND_VAR:
      if (mul2->type != TERM_FUND_IND_VAR)
	return 1;
      val1 = mul1->term.fund_ind_var->pcloop->ID;
      val2 = mul2->term.fund_ind_var->pcloop->ID;
      if (val1 < val2)
	return 1;
      else if (val1 > val2)
	return -1;
      return 0;

    case TERM_SSA_DEF:
      if (mul2->type == TERM_FUND_IND_VAR)
	return -1;
      else if (mul2->type != TERM_SSA_DEF)
	return 1;
      if (mul1->term.ssa_def == mul2->term.ssa_def)
	return 0;

      /* First, check if the ssa_def has a var: if not, it's a parameter
         or some other special case. */
      if (mul1->term.ssa_def->var == NULL)
	{
	  if (mul2->term.ssa_def->var != NULL)
	    return 1;
	  var1 = mul1->term.ssa_def->uses->var;
	  var2 = mul2->term.ssa_def->uses->var;
	}
      else if (mul2->term.ssa_def->var == NULL)
	{
	  return -1;
	}
      else  /* Generic case. */
	{
	  var1 = mul1->term.ssa_def->var;
	  var2 = mul2->term.ssa_def->var;
	}

      val1 = strcmp (var1->value.var.name, var2->value.var.name);
      if (val1 == -1)
	return 1;
      else if (val1 == 1)
	return -1;
      
      if (mul1->term.ssa_def->subscr < mul2->term.ssa_def->subscr)
	return 1;
      else
	return -1;
       
    case TERM_SELECT:
      if (mul2->type == TERM_FUND_IND_VAR || mul2->type == TERM_SSA_DEF)
	return -1;
      else if (mul2->type != TERM_SELECT)
	return 1;
      return (PSS_Select_Term_Prioritize (mul1->term.select,
					  mul2->term.select));

    case TERM_LIN_MON_INFO:
      if (mul2->type == TERM_FUND_IND_VAR || mul2->type == TERM_SSA_DEF ||
	  mul2->type == TERM_SELECT)
	return -1;
      else if (mul2->type != TERM_LIN_MON_INFO)
	return 1;

      val1 = PSS_Select_Term_Prioritize (mul1->term.lin_mon->min_incr,
					 mul2->term.lin_mon->min_incr);
      if (val1 == 1)
	return 1;
      else if (val1 == -1)
	return -1;

      return (PSS_Select_Term_Prioritize (mul1->term.lin_mon->max_incr,
					  mul2->term.lin_mon->max_incr));

    case TERM_PCODE_EXPR:
      if (mul2->type == TERM_FUND_IND_VAR || mul2->type == TERM_SSA_DEF ||
	  mul2->type == TERM_SELECT || mul2->type == TERM_LIN_MON_INFO)
	return -1;
      if (mul2->type != TERM_PCODE_EXPR)
	P_punt ("PSS_Add_Term_Prioritize: No valid term types on term2.");

      val1 = (int) (mul1->term.pcode_expr);
      val2 = (int) (mul2->term.pcode_expr);
      if (val1 < val2)
	return 1;
      else if (val1 > val2)
	return -1;
      return 0;

    default:
      P_punt ("PSS_Add_Term_Prioritize: illegal term type found.");
      return 0;
    }
}


/* SER 20041201
 * This function returns 1 if one add term should be before another in a
 * canonical form (determined by this function). */
static int
PSS_Add_Term_Prioritize (PSS_Add_Term add1, PSS_Add_Term add2)
{
  PSS_Mul_Term mul1, mul2;
  int val;

  mul1 = add1->mul;
  mul2 = add2->mul;

  while (mul1)
    {
      /* Shorter chains have higher priority */
      if (mul2 == NULL)
	return 0;
      val = PSS_Mul_Term_Prioritize (mul1, mul2);
      if (val == -1)
	return -1;
      else if (val == 1)
	return 1;
      mul1 = mul1->next;
      mul2 = mul2->next;
    }
  /* Shorter chains have higher priority. */
  if (mul2 != NULL)
    return 1;
  return 0;
}


/* Adds or subtracts two addition term lists.  Makes copies of the input lists
 * and constructs the new list from the copies.
 */
static PSS_Add_Term
PSS_Arithmetic_Add_Terms (PSS_Add_Term old_term1,
			  PSS_Add_Term old_term2, int sub_flag)
{
  PSS_Add_Term term1, term2, prev, current1, current2, temp;

  if (old_term1 != NULL)
    term1 = PSS_Create_Add_Term (old_term1);
  else
    term1 = NULL;

  if (old_term2 != NULL)
    {
      term2 = PSS_Create_Add_Term (old_term2);

      /* Flip sign during a subtraction. */
      if (sub_flag)
	for (current2 = term2; current2; current2 = current2->next)
	  current2->int_coeff = -(current2->int_coeff);
    }
  else
    term2 = NULL;

  /* Cases where at least one term is NULL. */
  if (old_term1 == NULL)
    if (old_term2 != NULL)
      return term2;
    else
      return NULL;
  else if (old_term2 == NULL)
    return term1;

  /* Merge elements from term2 into term1. */    
  prev = term1;
  current2 = term2;

  /* First merge constant terms if term1 is only a constant. */
  if (term1->next == NULL)
    {
      if (term2->mul == NULL)
	{
	  term1->int_coeff += current2->int_coeff;
	  temp = current2;
	  current2 = current2->next;
	  temp->next = NULL;
	  PSS_Delete_Add_Term (temp);
	}
      /* Shouldn't throw a warning here, we use this function to add
       * individual, non-canonical terms. */
    }
  else
    {
      for (current1 = prev->next; current1; prev = current1,
	     current1 = current1->next)
	{
	  while (current2)
	    {
	      int delete1 = 0, delete2 = 0;

	      /* First, integrate constant terms. */
	      if (current2->mul == NULL)
		{
		  term1->int_coeff += current2->int_coeff;
		  delete2 = 1;
		}
	      /* Then, check if the terms have the same mul term: we
	       * only need to add the integer constants in this case. */
	      else if (PSS_Same_Mul_Term (current1->mul, current2->mul))
		{
		  current1->int_coeff += current2->int_coeff;
		  delete2 = 1;
		  if (current1->int_coeff == 0)
		    delete1 = 1;
		}
	      /* Delete term */
	      if (delete2)
		{
		  temp = current2->next;
		  current2->next = NULL;
		  PSS_Delete_Add_Term (current2);
		  current2 = temp;

		  /* If the integer coefficient is 0, delete current1 */
		  if (delete1)
		    {
		      prev->next = current1->next;
		      current1->next = NULL;
		      PSS_Delete_Add_Term (current1);
		      current1 = prev;
		      break;
		    }
		  continue;
		}
	      /* Otherwise, if current2 has priority over current1, insert into
	       * the term. */
	      else if (PSS_Add_Term_Prioritize (current2, current1) == 1)
		{
		  temp = current2->next;
		  current2->next = current1;
		  prev->next = current2;
		  prev = current2;
		  current2 = temp;
		}
	      /* Final case: current2 goes after current1, try next current1 */
	      else
		break;
	    }
	  if (current1 == NULL)
	    break;
	}
    }
  /* Append rest of current2 to term1 if we've run out of terms to compare. */
  if (current2)
    prev->next = current2;

  return term1;
}


/* SER 20041201
 * Combines multiplication terms
 * Does NOT create new copies of the terms.
 */
static PSS_Mul_Term
PSS_Combine_Mul_Terms (PSS_Mul_Term mul1, PSS_Mul_Term mul2)
{
  PSS_Mul_Term new, tail;

  if (mul1 == NULL)
    return mul2;
  else if (mul2 == NULL)
    return mul1;

  if (PSS_Mul_Term_Prioritize (mul1, mul2) == -1)
    {
      new = mul2;
      tail = mul2;
      mul2 = mul2->next;
    }
  else
    {
      new = mul1;
      tail = mul1;
      mul1 = mul1->next;
    }
  /* Loop: order the mul terms by priority. */
  while (mul1)
    {
      int value;

      if (mul2 == NULL)
	break;
      value = PSS_Mul_Term_Prioritize (mul1, mul2);
      if (value == -1)
	{
	  tail->next = mul2;
	  tail = mul2;
	  mul2 = mul2->next;
	}
      else
	{
	  tail->next = mul1;
	  tail = mul1;
	  mul1 = mul1->next;
	}
    }
  if (mul2)
    tail->next = mul2;
  else
    tail->next = NULL;

  return new;
}


/* Multiplies two INDIVIDUAL addition terms. */
static PSS_Add_Term
PSS_Multiply_Individual_Add_Terms (PSS_Add_Term old_term1,
				   PSS_Add_Term old_term2)
{
  PSS_Add_Term add_term;

  /* Shortcut if one of the terms is 0. */
  if (old_term1->int_coeff == 0 || old_term2->int_coeff == 0)
    return NULL;

  /* Shortcut if one of the inputs is constant. */
  if (old_term1->mul == NULL)
    {
      add_term = PSS_Create_Single_Add_Term (old_term2);
      add_term->int_coeff *= old_term1->int_coeff;
    }
  else if (old_term2->mul == NULL)
    {
      add_term = PSS_Create_Single_Add_Term (old_term1);
      add_term->int_coeff *= old_term2->int_coeff;
    }
  /* Generic case: Join the two mul terms. PSS_Combine_Mul_Terms() doesn't
   * make copies, so don't delete the generated terms. */
  else
    {
      PSS_Mul_Term mul1, mul2;

      mul1 = PSS_Create_Mul_Term (old_term1->mul);
      mul2 = PSS_Create_Mul_Term (old_term2->mul);
      add_term = PSS_Create_Single_Add_Term (NULL);
      add_term->affine = old_term1->affine && old_term2->affine;
      add_term->int_coeff = old_term1->int_coeff * old_term2->int_coeff;
      add_term->mul = PSS_Combine_Mul_Terms (mul1, mul2);
     }
  add_term->next = NULL;
  return add_term;
}


/*! \brief Multiplies two full Add_Term chains together.
 *
 * \return A new Add_Term chain containing the multiplied result.
 */
static PSS_Add_Term
PSS_Multiply_Chain_of_Add_Terms (PSS_Add_Term old_term1,
				 PSS_Add_Term old_term2)
{
  PSS_Add_Term total, iter_term1, iter_term2;

  total = PSS_Create_Add_Term (NULL);
  total->int_coeff = 0;
  total->affine = 1;

  for (iter_term1 = old_term1; iter_term1; iter_term1 = iter_term1->next)
    {
      for (iter_term2 = old_term2; iter_term2; iter_term2 = iter_term2->next)
	{
	  PSS_Add_Term new_total, term;

	  term = PSS_Multiply_Individual_Add_Terms (iter_term1, iter_term2);
	  if (term != NULL)
	    {
	      new_total = PSS_Arithmetic_Add_Terms (total, term, 0);
	      PSS_Delete_Add_Term (term);
	      PSS_Delete_Add_Term (total);
	      total = new_total;
	    }
	}
    }

  return total;
}


/* Adds/subs two expressions. Leaves the old ones untouched. */
PSS_Expr
PSS_Arithmetic_Exprs (PSS_Expr old_expr1, PSS_Expr old_expr2, int sub_flag)
{
  PSS_Add_Term new_term;
  PSS_Expr ind_expr;

  ind_expr = PSS_Create_Expr (NULL);
  PSS_Delete_Add_Term (ind_expr->terms);
  new_term = PSS_Arithmetic_Add_Terms (old_expr1->terms, old_expr2->terms,
				       sub_flag);
  ind_expr->terms = new_term;

  /* propagate flags */
  if ((old_expr1->flags & UNKNOWN_IV) || (old_expr2->flags & UNKNOWN_IV))
    ind_expr->flags |= UNKNOWN_IV;
  else if ((old_expr1->flags & LIN_MONOTONIC_IV) ||
	   (old_expr2->flags & LIN_MONOTONIC_IV))
    ind_expr->flags |= LIN_MONOTONIC_IV;
  else if ((old_expr1->flags & LINEAR_IV) ||
	   (old_expr2->flags & LINEAR_IV))
    ind_expr->flags |= LINEAR_IV;

  return ind_expr;
}


/* SER 20050113
 * Support for negative expressions.
 */
PSS_Expr
PSS_Negative (Expr expr, int bb_id, PC_Loop loop, int force_prop)
{
  PSS_Expr new_expr;
  PSS_Add_Term term;

  if (!(expr->opcode == OP_neg))
    P_punt ("Trying to call PSS_Negative on a non-OP_neg Expr");

  new_expr = PSS_Generic_Expr (expr->operands, bb_id, loop, force_prop);

  for (term = new_expr->terms; term; term = term->next)
    term->int_coeff *= -1;

  return new_expr;
}


/* SER 20041116
 * Often we'll multiply an expression by a constant: this speeds up the
 * process over the generic multiplication function.
 */
PSS_Expr
PSS_Multiply_Expr_By_Int (PSS_Expr old_expr, int amount)
{
  PSS_Expr new_expr;
  PSS_Add_Term add_term;

  new_expr = PSS_Create_Expr (old_expr);

  for (add_term = new_expr->terms; add_term; add_term = add_term->next)
    add_term->int_coeff *= amount;

  return new_expr;
}


PSS_Expr
PSS_Multiply_Exprs (PSS_Expr old_expr1, PSS_Expr old_expr2)
{
  PSS_Expr new_expr;

  new_expr = PSS_Create_Expr (NULL);
  new_expr->terms = PSS_Multiply_Chain_of_Add_Terms (old_expr1->terms,
						     old_expr2->terms);
  return new_expr;
}

#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
/* \brief This function takes a PSS_Expr and splits any add_terms it has in
 * preparation of phi, min, max combination. */
static PSS_Expr
PSS_Generate_Select_Expr (PSS_Expr ie, PSS_Select_Type select_type)
{
  PSS_Expr total_expr;

  if (ie == NULL)
    P_punt("PSS_Generate_Select_Expr requires an induction expression input.");

  total_expr = PSS_Create_Expr (NULL);
  total_expr->terms = PSS_Create_Select_Add (ie->terms, select_type);

  return total_expr;
}
#endif

#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
/* \brief Integrates the input add_term into the select term.  Used for
 * creating phi-function expressions and computing linear monotonic IVs.
 */
static PSS_Select_Term
PSS_Select_Integrate (PSS_Select_Term select, PSS_Add_Term add_term,
		      PSS_Select_Type select_type)
{
  PSS_Select_Term prev_select, new_select, iter_select;

  if (add_term == NULL)
    return select;

  for (iter_select = select; iter_select; iter_select = iter_select->next)
    {
      if (PSS_Same_Add_Term (iter_select->add, add_term))
	return select;
      if (select_type == SELECT_MIN)
	{
	  if (PSS_Add_Term_Less_Than (add_term, iter_select->add))
	    {
	      PSS_Delete_Add_Term (iter_select->add);
	      iter_select->add = PSS_Create_Add_Term (add_term);
	      return select;
	    }
	  else if (PSS_Add_Term_Greater_Than (add_term, iter_select->add))
	    return select;
	}
      else if (select_type == SELECT_MAX)
	{
	  if (PSS_Add_Term_Greater_Than (add_term, iter_select->add))
	    {
	      PSS_Delete_Add_Term (iter_select->add);
	      iter_select->add = PSS_Create_Add_Term (add_term);
	      return select;
	    }
	  else if (PSS_Add_Term_Less_Than (add_term, iter_select->add))
	    return select;
	}
      prev_select = iter_select;
    }

  /* If we reach this point, no matches found. Integrate into expression.
   * If the new term matches the same format as the current term
   * (as is common with chained phi-functions), flatten the expression. */
  new_select = PSS_Create_Select_Term (NULL);
  new_select->sel_type = select_type;
  new_select->add = PSS_Create_Add_Term (add_term);

  if (select)
    prev_select->next = new_select;
  else
    select = new_select;

  return select;
}
#endif


/*********************************************
 * INDUCTION EXPRESSION COMPARISON FUNCTIONS *
 *********************************************/

static int
PSS_Expr_Is_Int_Constant (PSS_Expr ind_expr, int * amount)
{
  PSS_Add_Term terms = ind_expr->terms;

  if (terms->next != NULL)
    return 0;
  else
    {
      *amount = terms->int_coeff;
      return 1;
    }
}


/* We can't tell anything about select terms, since we don't carry
 * condition information.  Thus, return 0 by default. */
static int
PSS_Same_Select_Term (PSS_Select_Term select1, PSS_Select_Term select2)
{
  return 0;  
}

static int
PSS_Same_Add_Term (PSS_Add_Term add_term1, PSS_Add_Term add_term2)
{
  if (add_term1 == NULL && add_term2 == NULL)
    return 1;
  else if (add_term1 == NULL || add_term2 == NULL)
    return 0;
  if (add_term1->int_coeff != add_term2->int_coeff)
    return 0;
  if (!(PSS_Same_Mul_Term (add_term1->mul, add_term2->mul)))
    return 0;

  return (PSS_Same_Add_Term (add_term1->next, add_term2->next));
}


int
PSS_Same_Mul_Term (PSS_Mul_Term mul_term1, PSS_Mul_Term mul_term2)
{
  if (mul_term1 == NULL && mul_term2 == NULL)
    return 1;
  else if (mul_term1 == NULL || mul_term2 == NULL)
    return 0;
  if (mul_term1->type != mul_term2->type)
    return 0;

  switch (mul_term1->type)
    {
    case TERM_SELECT:
      if (!(PSS_Same_Select_Term (mul_term1->term.select,
				  mul_term2->term.select)))
	return 0;
      break;
    case TERM_LIN_MON_INFO:
      /* Monotonic info can't be accurately compared, so return 0. */
      return 0;
    case TERM_FUND_IND_VAR:
      if (mul_term1->term.fund_ind_var != mul_term2->term.fund_ind_var)
	return 0;
      break;
    case TERM_SSA_DEF:
      if (mul_term1->term.ssa_def != mul_term2->term.ssa_def)
	return 0;
      break;
    case TERM_PCODE_EXPR:
      if (mul_term1->term.pcode_expr != mul_term2->term.pcode_expr)
	return 0;
      break;
    default:
      P_punt ("PSS_Same_Mul_Term: Unsupported type %d on PSS_Mul_Term",
	      mul_term1->type);
    }
  return (PSS_Same_Mul_Term (mul_term1->next, mul_term2->next));
}

int
PSS_Same_Expr (PSS_Expr ind_expr1, PSS_Expr ind_expr2)
{
  return (PSS_Same_Add_Term (ind_expr1->terms, ind_expr2->terms));
}

int
PSS_Expr_Is_Int (PSS_Expr ind_expr, int * value)
{
  if (ind_expr->terms->next != NULL)
    return 0;

  *value = ind_expr->terms->int_coeff;
  return 1;
}


/* This function only compares the constant terms of add chains. */
int
PSS_Add_Term_Less_Than (PSS_Add_Term add1, PSS_Add_Term add2)
{
  if (add1 == NULL && add2 != NULL)
    return 1;
  else if (add2 == NULL)
    return 0;

  if (add1->int_coeff >= add2->int_coeff)
    return 0;
  else if (PSS_Same_Add_Term (add1->next, add2->next))
    return 1;

  return 0;
}


/* This function only compares the constant terms of add chains. */
int
PSS_Add_Term_Greater_Than (PSS_Add_Term add1, PSS_Add_Term add2)
{
  if (add2 == NULL && add1 != NULL)
    return 1;
  else if (add1 == NULL)
    return 0;

  if (add1->int_coeff <= add2->int_coeff)
    return 0;
  else if (PSS_Same_Add_Term (add1->next, add2->next))
    return 1;

  return 0;
}


/**********************************************
 * INDUCTION EXPRESSION CALCULATION FUNCTIONS *
 **********************************************/

/* --- HELPER FUNCTIONS, FUNCTION DECLARATIONS --- */

static int
PSS_Supported_Opcode (Expr expr)
{
  switch (expr->opcode)
    {
    case OP_assign:
    case OP_phi:
    case OP_add:
    case OP_sub:
    case OP_mul:
    case OP_lshft:
    case OP_int:
    case OP_neg:
    case OP_var:
    case OP_cast:
      return 1;
    default:
      return 0;
    }
}

/* --- NON-RECURSIVE IND_EXPR PROCESSING FUNCTIONS --- */

static PSS_Expr
PSS_Const (Expr expr)
{
  PSS_Expr new_ind_expr;
  PSS_Add_Term new_add;

  new_ind_expr = PSS_Create_Expr (NULL);
  new_add = PSS_Create_Add_Term (NULL);
  new_ind_expr->terms = new_add;

  new_add->affine = 1;
  new_add->int_coeff = expr->value.scalar;

  return new_ind_expr;
}

static PSS_Expr
PSS_Unsupported_Opcode_Expr (Expr expr)
{
  PSS_Expr new_ind_expr = PSS_Create_Expr (NULL);
  PSS_Add_Term new_add = PSS_Create_Add_Term (NULL);
  PSS_Mul_Term new_mul = PSS_Create_Mul_Term (NULL);

  new_ind_expr->terms->next = new_add;
  new_add->mul = new_mul;

  new_mul->type = TERM_PCODE_EXPR;
  new_mul->term.pcode_expr = expr;

  return new_ind_expr;
}


/* --- RECURSIVE IND EXPR PROCESSING FUNCTIONS --- */

/* The following functions recursively find the expression of a Pcode
 * expression in terms of fundamental induction vars.
 */
static PSS_Expr
PSS_SSA_Use (Expr expr, int bb_id, PC_Loop loop, int force_prop)
{
  PSS_Expr new_ind_expr;
  PSS_Def ssa_def;
  Expr def_expr;

  /* Request expression from reaching def */
  ssa_def = expr->value.var.ssa;

  if (!UNINITIALIZED_TYPE (ssa_def->type))
    {
      def_expr = ssa_def->var->parentexpr;
      new_ind_expr = PSS_Assign (def_expr, ssa_def->bb->ID,
				       loop, force_prop);
    }
  else  /* Need to treat parameters differently; make an SSA def. */
    {
      PSS_Add_Term new_add = PSS_Create_Add_Term (NULL);
      PSS_Mul_Term new_mul = PSS_Create_Mul_Term (NULL);

      new_ind_expr = PSS_Create_Expr (NULL);
      new_ind_expr->terms->next = new_add;
      new_add->mul = new_mul;

      new_add->affine = (ssa_def->type == PARAM) ? 1: 0;
      new_mul->type = TERM_SSA_DEF;
      new_mul->term.ssa_def = ssa_def;
    }

  return new_ind_expr;
}

/*! \brief Processes a Pcode add or sub expression. */
static PSS_Expr
PSS_Arithmetic (Expr expr, int bb_id, PC_Loop loop,
		      int force_prop)
{
  PSS_Expr new_expr, ind_expr1, ind_expr2;

  if (!(expr->opcode == OP_add || expr->opcode == OP_sub))
    P_punt ("Trying to call PSS_Arithmetic on a non-arithmetic Expr");

  ind_expr1 = PSS_Generic_Expr (expr->operands, bb_id, loop, force_prop);
  ind_expr2 = PSS_Generic_Expr (expr->operands->sibling, bb_id, loop,
				      force_prop);
  new_expr = PSS_Arithmetic_Exprs (ind_expr1, ind_expr2,
				   (expr->opcode == OP_sub));
  ind_expr1 = PSS_Delete_Expr (ind_expr1);
  ind_expr2 = PSS_Delete_Expr (ind_expr2);

  return new_expr;
}


/*! \brief Processes a Pcode multiply or left-shift expression. */
static PSS_Expr
PSS_Multiply (Expr expr, int bb_id, PC_Loop loop, int force_prop)
{
  PSS_Expr new_expr, ind_expr1 = NULL, ind_expr2 = NULL;
  int amount = 0;

  if (expr->opcode == OP_mul)
    {
      ind_expr1 = PSS_Generic_Expr (expr->operands, bb_id, loop, force_prop);
      ind_expr2 = PSS_Generic_Expr (expr->operands->sibling, bb_id, loop,
				    force_prop);
      if (PSS_Expr_Is_Int_Constant (ind_expr1, &amount))
	new_expr = PSS_Multiply_Expr_By_Int (ind_expr2, amount);
      else if (PSS_Expr_Is_Int_Constant (ind_expr2, &amount))
	new_expr = PSS_Multiply_Expr_By_Int (ind_expr1, amount);
      else
	new_expr = PSS_Multiply_Exprs (ind_expr1, ind_expr2);
    }
  /* We can create expressions from a left shift if the amount is a positive
   * constant value. */
  else if (expr->opcode == OP_lshft)
    {
      int mul_amount;
      ind_expr2 = PSS_Generic_Expr (expr->operands->sibling, bb_id, loop,
				    force_prop);
      if (!(PSS_Expr_Is_Int_Constant (ind_expr2, &amount) && (amount >= 0)))
	{
	  ind_expr2 = PSS_Delete_Expr (ind_expr2);
	  return (PSS_Unsupported_Opcode_Expr (expr));
	}
      ind_expr1 = PSS_Generic_Expr (expr->operands, bb_id, loop, force_prop);
      /* Find the appropriate multiplication amount by shifting. */
      mul_amount = 1 << amount;
      new_expr = PSS_Multiply_Expr_By_Int (ind_expr1, mul_amount);
    }
  else
    P_punt ("PSS_Multiply: input is not a multiply or lshift Expr.");

  ind_expr1 = PSS_Delete_Expr (ind_expr1);
  ind_expr2 = PSS_Delete_Expr (ind_expr2);

  return new_expr;
}


/*! \brief Processes a Pcode phi-function (but not mu-function) expression. */
static PSS_Expr
PSS_Phi (Expr expr, int bb_id, PC_Loop loop, int force_prop)
{
  Expr first_operand, next_operand;
  PSS_Expr ind_expr1, ind_expr2;

  if (expr->opcode != OP_phi)
    P_punt ("Trying to call PSS_Phi on a non-OP_phi Expr");

  first_operand = expr->operands;
      
  ind_expr1 = PSS_Generic_Expr (first_operand, bb_id, loop, force_prop);

  for (next_operand = first_operand->next; next_operand;
       next_operand = next_operand->next)
    { 
      ind_expr2 = PSS_Generic_Expr (next_operand, bb_id, loop, force_prop);
      if (!PSS_Same_Expr (ind_expr1, ind_expr2))
	{
	  ind_expr1 = PSS_Delete_Expr (ind_expr1);
	  ind_expr2 = PSS_Delete_Expr (ind_expr2);
	  return (PSS_Unsupported_Opcode_Expr (expr));
	}
      else
	{
	  ind_expr2 = PSS_Delete_Expr (ind_expr2);
	}
    }
  return ind_expr1;
}

static int
PSS_Is_Phi_Operand (Expr phi_expr, Expr expr)
{
  Expr src;

  for (src = phi_expr->operands; src; src = src->next)
    {
      if (src->value.var.ssa == expr->value.var.ssa)
	return 1;
    }
  return 0;
}

static PSS_Expr
PSS_Mu (Expr expr, int bb_id, PC_Loop loop, int force_prop)
{
  PSS_Fund_Ind_Var fund_ind_var;
  PSS_TarLoop tloop;
  PSS_TarSCC scc;

  if (expr->opcode != OP_phi ||
      !MU_TYPE (expr->parentexpr->operands->value.var.ssa->type))
    P_punt ("Trying to call PSS_Mu on a non-mu Expr");
  if (!(expr->parentexpr->operands->value.var.ssa->uses))
    {
      P_warn ("No uses of SSA var at expr %d: likely dead mu-function.",
	      expr->id);
      return NULL;
    }

  /* Find SCC & type */
  fund_ind_var = Get_Fund_Ind_Var(loop);
  tloop = Get_TarLoop(fund_ind_var);
  if ((scc = PSS_Get_SCC (tloop, expr->parentexpr)) == NULL)
    {
#ifdef DEBUG_IV_CALC
      P_warn ("PSS_Mu: couldn't find an SCC containing expr %d, "
	      "var %s, loop %d, func %s.\nThis probably does not involve "
	      "a loop-carried dependence.", expr->id,
	      expr->operands->value.string, loop->ID, current_CFG->func->name);
#endif
      return (PSS_Phi (expr, bb_id, loop, force_prop));
    }

  switch (scc->type)
    {
    case LINEAR:
      return (PSS_Linear_IV (expr, bb_id, loop, scc));
    case LINEAR_MONOTONIC:
#ifdef DO_LIN_MON_IVS
      return (PSS_Linear_Monotonic_IV(expr, bb_id, loop, scc));
#else
      P_warn ("Ignoring Linear Monotonic IV.");
      return (PSS_Unsupported_Opcode_Expr(expr));
#endif
    case POLYNOMIAL:
    case POLYNOMIAL_MONOTONIC:
#ifdef DEBUG_IV_CALC
      P_warn ("Polynomial IV encountered; currently unsupported.");
#endif
      return (PSS_Unknown_IV (expr, bb_id));
    case GEOMETRIC:
    case GEOMETRIC_MONOTONIC:
#ifdef DEBUG_IV_CALC
      P_warn ("Geometric IV encountered; currently unsupported.");
#endif
      return (PSS_Unknown_IV (expr, bb_id));
    case POINTER:
    case POINTER_MONOTONIC:
#ifdef DEBUG_IV_CALC
      P_warn ("Pointer-chasing IV encountered; unsupported.");
#endif
      return (PSS_Unknown_IV (expr, bb_id));
    case UNKNOWN:
    case IGNORE:
      return (PSS_Unknown_IV(expr, bb_id));
    default:
      P_punt ("PSS_Mu: shouldn't have an unclassified SCC.");
      return NULL;
    }
  P_punt ("PSS_Mu: shouldn't reach this point!");
  return NULL;
}

#define Mark_Expr_Visited(e) (PSS_SetExprIE(e,(void*) -1))
#define Expr_Visited(e) (PSS_GetExprIE(e) == (void*) -1)


/* SER 20041202
 * This function sets the "final_value" flag on any fundamental induction
 * variable mul terms whose scope has been left.
 */
static void
PSS_Set_Final_Value (PSS_Expr ind_expr, PC_Loop loop)
{
  PSS_Add_Term add;
  PSS_Mul_Term mul;
  PC_Loop fiv_loop;
  int all_loops;

  if (loop == NULL)
    all_loops = 1;
  else
    all_loops = 0;

  for (add = ind_expr->terms; add; add = add->next)
    {
      for (mul = add->mul; mul; mul = mul->next)
	{
	  if (mul->type != TERM_FUND_IND_VAR)
	    continue;
	  fiv_loop = mul->term.fund_ind_var->pcloop;
	  /* If we are still within a loop scope, it's not a final value,
	   * so if the requesting loop is within the FIV's loop, don't mark it
	   */
	  if (PC_LoopContainsLoop (fiv_loop, loop) > 0)
	    continue;

	  mul->final_value = 1;
	}
    }
}


/* NOTE: this function writes an expression to the SSA def automatically,
 * one should not write it outside of the function, or a memory
 * leak will occur.
 */
static PSS_Expr
PSS_Assign (Expr expr, int bb_id, PC_Loop loop, int force_prop)
{
  PSS_Expr new_ind_expr;
  Expr operands;
  int left_loop = 0;

  if (expr->opcode != OP_assign)
    P_punt ("Trying to call PSS_Assign on a non-OP_assign Expr");

  operands = expr->operands;
  if (loop)
    {
      left_loop = !Set_in (Get_Loop_body (loop), bb_id);
    }

  /* case for address-taken vars or global vars: no SSA def
   * this usually gets caught by PSS_Generic_Expr() */
  if (!(operands->value.var.ssa))
    return (PSS_Unsupported_Opcode_Expr (operands));
  /* We should call this function on an assignment once; if
   * we revisit it, it's most likely within an improper loop
   * joined by phi-functions (gsmdec has one example).  To avoid an
   * infinite loop, return the Pcode expr if we encounter an assignment
   * more than once. */
  else if (Expr_Visited(expr->operands))
    {
      P_warn ("Visited this expr previously during back-substitution.\n"
	      "Possibly an improper loop.  Returning Pcode expression.");
      new_ind_expr = PSS_Unsupported_Opcode_Expr (expr);
      
      PSS_SetExprIE(operands, new_ind_expr);
      return (new_ind_expr);
    }
  /* If the expression has been calculated previously, return that. */
  else if (PSS_GetExprIE(operands) != NULL)
    {
      new_ind_expr = PSS_Create_Expr (PSS_GetExprIE(operands));
      /* We need to set FINAL_VALUE when expr is in a child of the
       * requesting loop or if we've left the loop and expr is not in
       * a direct ancestor loop of the requesting loop. */
      if ((left_loop &&
	   !PC_BB_In_Direct_Ancestor_Loop_Scope(current_CFG, loop, bb_id)) ||
	  (loop && PC_BB_In_Child_Loop(bb_id, loop)) ||
	  (!loop && PC_BBInnerLoop(current_CFG, bb_id)))
	{
	  PSS_Set_Final_Value (new_ind_expr, loop);
	}
      return (new_ind_expr);
    }

  /* Stop backwards substitution if we don't know how to process the opcode */
  if (!PSS_Supported_Opcode (operands->sibling))
    {
      new_ind_expr = PSS_Unsupported_Opcode_Expr (operands);
    }
  else
    { /* Continue back-substitution */
      PC_Loop new_loop;
      /* Need to find the innermost loop for the expression. First make
       * sure that we haven't left the given loop's scope, then search for
       * the new loop. */
      if (left_loop || !loop || (loop && !PC_BB_In_Child_Loop(bb_id, loop)))
	new_loop = PC_BBInnerLoop (current_CFG, bb_id);
      else
	new_loop = loop;

      Mark_Expr_Visited(expr->operands);
      new_ind_expr = PSS_Generic_Expr (operands->sibling, bb_id,
					     new_loop, 0);
      PSS_SetExprIE(operands, new_ind_expr);
    }

  /* We actually want to make a copy of the expression to return,
   * provided that there is an expression to return. */
  if (new_ind_expr)
    {
      new_ind_expr = PSS_Create_Expr (new_ind_expr);
      /* See above for FINAL_VALUE and invariance conditions. */
      if ((left_loop &&
	   !PC_BB_In_Direct_Ancestor_Loop_Scope (current_CFG, loop, bb_id)) ||
	  (loop && PC_BB_In_Child_Loop (bb_id, loop)) ||
	  (!loop && PC_BBInnerLoop (current_CFG, bb_id)))
	{
	  PSS_Set_Final_Value (new_ind_expr, loop);
	}
    }
  return new_ind_expr;
}


static PSS_Expr
PSS_Generic_Expr (Expr expr, int bb_id, PC_Loop loop, int force_prop)
{
  if (!(PSS_Supported_Opcode(expr)))
    return (PSS_Unsupported_Opcode_Expr (expr));

  /* If induction expression has been previously calculated, return a copy. */
  if (PSS_GetExprIE(expr) != NULL)
    return (PSS_Create_Expr (PSS_GetExprIE(expr)));

  switch (expr->opcode)  /* call appropriate function based on opcode type */
    {
    case OP_cast:
      return (PSS_Generic_Expr (expr->operands, bb_id, loop, force_prop));
    case OP_assign:
      return (PSS_Assign (expr, bb_id, loop, force_prop));

    case OP_phi:
      /* mu-function case first */
      if (MU_TYPE (expr->parentexpr->operands->value.var.ssa->type))
	return (PSS_Mu (expr, bb_id, loop, force_prop));
      else  /* standard phi-function */
	return (PSS_Phi (expr, bb_id, loop, force_prop));

    case OP_add:
    case OP_sub:
      return (PSS_Arithmetic (expr, bb_id, loop, force_prop));

    case OP_neg:
      return (PSS_Negative (expr, bb_id, loop, force_prop));

    case OP_mul:
    case OP_lshft:
      return (PSS_Multiply (expr, bb_id, loop, force_prop));

    case OP_var:
      if (expr->value.var.ssa)
	return (PSS_SSA_Use (expr, bb_id, loop, force_prop));
      else
	{
#ifdef DEBUG_IV_CALC
	  int lines = 0;
	  P_warn ("PSS_Generic_Expr: Expr %d does not have an SSA def;\n"
		  "might be a global or address-taken variable.", expr->id);
	  P_write_expr (stderr, expr, 0, &lines);
	  fprintf (stderr, "\n");
#endif
	  return (PSS_Unsupported_Opcode_Expr (expr));
	}

    case OP_int:
      return (PSS_Const (expr));

    default:
      P_punt ("PSS_Generic_Expr: missing case in switch statement.");
    }

  P_punt ("PSS_Generic_Expr: should never reach this point!");
  return NULL;
}


/* SER 2004116
 * This function finds the expression for a function.  Unlike
 * PSS_Generic_Expr(), it is externally visible, and attaches the generated
 * expression to the Pcode Expr, since it is likely to be requested again.
 */
PSS_Expr
PSS_Get_Expr (Expr expr, int bb_id, PC_Loop loop)
{
  PSS_Expr ind_expr;

  if ((ind_expr = PSS_GetExprIE(expr)) == NULL)
    {
      ind_expr = PSS_Generic_Expr (expr, bb_id, loop, 0);

      /* We want to store the expression on the Expr, since it was called
       * externally and may be calculated again. */
      if (expr->opcode != OP_assign)
	PSS_SetExprIE(expr, ind_expr);
    }

  ind_expr = PSS_Create_Expr (ind_expr);

  return ind_expr;
}

/***************************************************************
 * INDUCTION EXPRESSION HIGH-LEVEL CALCULATION/QUERY FUNCTIONS *
 ***************************************************************/

/* Given an Expr, generates a PSS_Add_Term for the Expr and adds it to the
 * end of add_term.  Used in increment building.
 */
static PSS_Add_Term
PSS_Add_Increment_Expr (Expr expr, PSS_Add_Term add_term, int bb_id,
			     PC_Loop loop, int sub_flag)
{
  PSS_Expr new_expr;
  PSS_Add_Term new_add, new_total;

  new_add = PSS_Create_Add_Term (NULL);
  new_add->affine = 1;

  new_expr = PSS_Generic_Expr (expr, bb_id, loop, 0);
  new_add = new_expr->terms;
  new_expr->terms = NULL;
  PSS_Delete_Expr (new_expr);

  new_total = PSS_Arithmetic_Add_Terms (add_term, new_add, sub_flag);
  PSS_Delete_Add_Term (new_add);

  return new_total;
}


static PSS_Expr
PSS_Find_Linear_Increment (Expr mu_expr, int bb_id, PC_Loop loop,
				 PSS_TarSCC scc)
{
  PSS_Expr new_expr;
  PSS_Add_Term new_term;
  Expr dest;
  PSS_TarNode tnode;
  List tlist = scc->tnode_list;
  int found = 0;

  new_term = PSS_Create_Add_Term(NULL);
  new_term->int_coeff = 0;
  new_term->affine = 1;

  /* The dest expr is the var that we're trying to trace forward. */
  /* this is an ssa def expr */
  dest = mu_expr->parentexpr->operands;
  /* May need to traverse the list multiple times. */
  do
    {
      Expr assignment, operation, operand1, operand2;

      /* search through all the ops in the list, trying to find
       * the one that uses the def that we are searchin on */
      List_start (tlist);
      found = 0;
      while (!found && (tnode = (PSS_TarNode) List_next (tlist)))
	{
	  assignment = tnode->expr;
	  if (assignment->opcode != OP_assign)
	    P_punt ("Shouldn't have a non-assignment op in an SCC");
	  operation = assignment->operands->sibling;
	  /* Ignore casts for now: may be unsafe. */
	  while (operation->opcode == OP_cast)
	    operation = operation->operands;

	  operand1 = operation->operands;
	  operand2 = operand1->sibling;
	  while (operand1 && operand1->opcode == OP_cast)
	    operand1 = operand1->operands;
	  while (operand2 && operand2->opcode == OP_cast)
	    operand2 = operand2->operands;

	  /* Process depending on operation type */
	  switch (operation->opcode)
	    {
	    case OP_add:
	      if (operand2->value.var.ssa == dest->value.var.ssa)
		{
		  Expr temp_operand;
		  temp_operand = operand2;
		  operand2 = operand1;
		  operand1 = temp_operand;
		  found = 1;
		}
	      else if (operand1->value.var.ssa == dest->value.var.ssa)
		found = 1;
	      break;
	    case OP_sub:
	      if (operand1->value.var.ssa == dest->value.var.ssa)
		found = 1;
	      break;
	    case OP_phi:
	      break;
	    default:
	      P_punt("PSS_Find_Linear_Increment: unimplemented op "
		     "type in SCC.");
	    }
	}

      if (!found && tnode == NULL)
	P_punt ("PSS_Find_Linear_Increment: couldn't find appropriate "
		"use of SSA var.");

      /* At this point, operand1 is use of SSA var, operand1 is the
       * increment.  Process accordingly. */
      switch (operation->opcode)
	{
	case OP_add:
	  if (operand2->opcode == OP_int)
	    new_term->int_coeff += operand2->value.scalar;
	  else if (operand2->opcode == OP_var)
	    PSS_Add_Increment_Expr (operand2, new_term, bb_id, loop, 0);
	  dest = assignment->operands;
	  break;
	case OP_sub:
	  if (operand2->opcode == OP_int)
	    new_term->int_coeff -= operand2->value.scalar;
	  else
	    PSS_Add_Increment_Expr (operand2, new_term, bb_id, loop, 1);
	  dest = assignment->operands;
	  break;
	default:
	  P_punt ("PSS_Find_Linear_Increment: Should never reach this "
		  "point.");
	}
      /* Break out if we reach a loop-back to the mu-function. */
      if (PSS_Is_Phi_Operand (mu_expr, dest))
	break;
    } while (1);

  new_expr = PSS_Create_Expr (NULL);
  PSS_Delete_Add_Term (new_expr->terms);
  new_expr->terms = new_term;
  return new_expr;
}

#define Get_Lin_Mon_Info(ssa_def) ((PSS_Lin_Mon_Info) (ssa_def)->ext)


#ifdef DO_LIN_MON_IVS
/* return 1 if the input variables have the same declaration */
static int
PSS_Expr_Same_VarDcl (Expr expr1, Expr expr2)
{
  if (expr1->opcode != OP_var || expr2->opcode != OP_var)
    return 0;
  if (expr1->value.var.key.file != expr2->value.var.key.file)
    return 0;
  if (expr1->value.var.key.sym != expr2->value.var.key.sym)
    return 0;
  return 1;
}
#endif


#ifdef DO_LIN_MON_IVS
/* return 1 if all of a phi-function's operands are ready for processing. */
static int
PSS_Linear_Monotonic_Phi_Operands_Ready (Expr phi_op)
{
  Expr operand;
  PSS_Lin_Mon_Info lm_info;

  for (operand = phi_op->operands; operand; operand = operand->next)
    {
      lm_info = Get_Lin_Mon_Info (operand->value.var.ssa);
      if (lm_info == NULL)
	P_punt ("PSS_Linear_Monotonic_Phi_Operands_Ready: phi-function "
		"is receiving an operand (subscript %d) from outside the "
		"loop!", operand->value.var.ssa->subscr);
      if (lm_info->visited == 0)
	return 0;
    }
  return 1;
}
#endif


#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
/* Combines the increments of the inputs of a phi-function and makes
 * new min, max increments, which it attaches to a PSS_Lin_Mon_Info.
 * Note that we don't create new Add_Terms until the end.
 */
static void
PSS_Linear_Monotonic_Phi (Expr operation, PSS_Lin_Mon_Info dest_lm_info,
			  PC_Loop loop, int mu_flag)
{
  Expr operand;
  PSS_Select_Term min = NULL, max = NULL;

  /* Combine phi operands into min & max select lists. */
  for (operand = operation->operands; operand; operand = operand->next)
    {
      PSS_Def ssa_def;
      PSS_Lin_Mon_Info src_lm_info;
      PSS_Select_Term src_min, src_max;

      ssa_def = operand->value.var.ssa;
      /* When processing the loop's mu-function, don't include external
       * operands. */
      if (mu_flag && (PARAM_TYPE (ssa_def->type) ||
		      !Set_in (Get_Loop_body (loop), ssa_def->bb->ID)))
	continue;
      src_lm_info = Get_Lin_Mon_Info (ssa_def);

      /* For each select term in src_min, src_max, integrate its add term
       * if appropriate. */
      for (src_min = src_lm_info->min_incr; src_min; src_min = src_min->next)
	{
	  if (src_min && src_min->type != TERM_ADD)
	    P_punt ("PSS_Linear_Monotonic_Phi: found non-add term for select "
		    "during min calculation.");
	  min = PSS_Select_Integrate (min, src_min->term.add, SELECT_MIN);
	}

      for (src_max = src_lm_info->max_incr; src_max; src_max = src_max->next)
	{
	  if (src_min && src_min->type != TERM_ADD)
	    P_punt ("PSS_Linear_Monotonic_Phi: found non-add term for select "
		    "during max calculation.");
	  max = PSS_Select_Integrate (max, src_max->term.add, SELECT_MAX);
	}
    }

  dest_lm_info->min_incr = min;
  dest_lm_info->max_incr = max;
}
#endif


#if DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
static Set
PSS_Linear_Monotonic_Increment_Setup (Expr mu_expr, PSS_TarSCC scc)
{
  PSS_Def mu_ssa_def;
  PSS_TarNode tnode;
  List tlist = scc->tnode_list;
  Set total_subscr = NULL;
#ifdef CHECK_LIN_MON_REQUIREMENTS
  int num_mu = 0, num_phi = 0, num_other = 0;
#endif

  mu_ssa_def = mu_expr->parentexpr->operands->value.var.ssa;
  List_start (tlist);
  while ((tnode = (PSS_TarNode) List_next (tlist)))
    {
      PSS_Def ssa_def;

      ssa_def = tnode->expr->operands->value.var.ssa;
      if (ssa_def->ext != NULL)
	P_punt ("PSS_Linear_Monotonic_Increment_Setup: need to have SSA "
		"def's ext field NULL.  Inform SER.");
      ssa_def->ext = PSS_Create_Lin_Mon_Info ();
      if (ssa_def == mu_ssa_def)
	{
	  PSS_Lin_Mon_Info lm_info = Get_Lin_Mon_Info (ssa_def);
	  PSS_Select_Term select;
	  PSS_Add_Term add;

	  lm_info->visited = 1;
	  /* min term */
	  select = PSS_Create_Select_Term (NULL);
	  add = PSS_Create_Add_Term (NULL);
	  select->sel_type = SELECT_MIN;
	  select->add = add;
	  add->int_coeff = 0;
	  lm_info->min_incr = select;

	  /* max term */
	  select = PSS_Create_Select_Term (NULL);
	  add = PSS_Create_Add_Term (NULL);
	  select->sel_type = SELECT_MAX;
	  select->add = add;
	  add->int_coeff = 0;
	  lm_info->max_incr = select;

#ifdef CHECK_LIN_MON_REQUIREMENTS
	  num_mu++;
#endif
	}
      else if (MU_TYPE (ssa_def->type))
	P_punt ("PSS_Linear_Monotonic_Increment_Setup: found a second "
		"mu-function in the SCC\n;\tSCC shouldn't be classified "
		"monotonic");
#ifdef CHECK_LIN_MON_REQUIREMENTS
      else if (PHI_TYPE (ssa_def->type))
	num_phi++;
      else
	num_other++;
#endif
      total_subscr = Set_add (total_subscr, ssa_def->subscr);
    }

#ifdef CHECK_LIN_MON_REQUIREMENTS
  if (num_mu != 1)
    P_punt ("PSS_Linear_Monotonic_Increment_Setup: no mu-function found in "
	    "linear monotonic SCC.");
  if (num_phi == 0)
    P_punt ("PSS_Linear_Monotonic_Increment_Setup: no phi-functions found in "
	    "linear monotonic SCC.");
  if (num_other == 0)
    P_punt ("PSS_Linear_Monotonic_Increment_Setup: no arithmetic ops found "
	    "in linear monotonic SCC.");
#endif
  return total_subscr;
}
#endif


#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
static void
PSS_Linear_Monotonic_Increment_Cleanup(PSS_TarSCC scc)
{
  PSS_TarNode tnode;
  List tlist = scc->tnode_list;

  while ((tnode = (PSS_TarNode) List_next (tlist)))
    {
      PSS_Def ssa_def;
      PSS_Lin_Mon_Info lm_info;

      ssa_def = tnode->expr->operands->value.var.ssa;
      lm_info = Get_Lin_Mon_Info (ssa_def);
      PSS_Delete_Lin_Mon_Info (lm_info);
      ssa_def->ext = NULL;
    }
}
#endif


#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
static void
PSS_Add_Select_Increment (PSS_Select_Term select, Expr operand, int bb_id,
			  PC_Loop loop, int sub_flag)
{
  PSS_Select_Term iter_select;
  for (iter_select = select; iter_select; iter_select = iter_select->next)
    {
      if (iter_select->type != TERM_ADD)
	P_punt ("PSS_Add_Select_Increment: should not be encountering "
		"non-TERM_ADD type on PSS_Select_Term");
      if (operand->opcode == OP_int)
	{
	  if (iter_select->term.add->type != TERM_CONST)
	    P_punt ("PSS_Add_Select_Increment: coefficient for first add "
		    "term should be constant.");
	  if (sub_flag)
	    iter_select->term.add->int_coeff -= operand->value.scalar;
	  else
	    iter_select->term.add->int_coeff += operand->value.scalar;
	}
      else
	{
	  PSS_Add_Increment_Expr (operand, iter_select->term.add, bb_id,
				  loop, sub_flag);
	}
    }
}
#endif


#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
static PSS_Expr
PSS_Find_Linear_Monotonic_Increment (Expr mu_expr, PSS_Expr ind_expr,
				     int bb_id, PC_Loop loop, PSS_TarSCC scc)
{
  PSS_TarNode tnode;
  List tlist = scc->tnode_list;
  Set processed, total = NULL;
  Expr mu_var = mu_expr->parentexpr->operands;
  PSS_Def mu_ssa_def = mu_var->value.var.ssa;
  PSS_Lin_Mon_Info mu_lm_info;

  processed = Set_add (NULL, mu_ssa_def->subscr);

  /* First, setup structs attached to SSA defs. */
  total = PSS_Linear_Monotonic_Increment_Setup (mu_expr, scc);

  /* Next, process increments. */
  do {
    Expr assignment, operation, operand1, operand2;
    PSS_Lin_Mon_Info dest_lm_info;
    int ready = 0;

    List_start (tlist);

    while ((tnode = (PSS_TarNode) List_next (tlist)))
      {
	PSS_Lin_Mon_Info src_lm_info;
	PSS_Select_Term min_select, max_select;
	int sub_flag;

	assignment = tnode->expr;
	dest_lm_info = Get_Lin_Mon_Info (assignment->operands->value.var.ssa);
	if (dest_lm_info->visited)
	  continue;
	operation = assignment->operands->sibling;
	/* Ignore casts for now: may be unsafe. */
	while (operation->opcode == OP_cast)
	  operation = operation->operands;

	operand1 = operation->operands;
	operand2 = operand1->sibling;
	while (operand1 && operand1->opcode == OP_cast)
	  operand1 = operand1->operands;
	while (operand2 && operand2->opcode == OP_cast)
	  operand2 = operand2->operands;
	/* Check if operands are ready. */
	switch (operation->opcode)
	  {
	  case OP_add:
	    if (PSS_Expr_Same_VarDcl (operand1, mu_var))
	      {
		src_lm_info = Get_Lin_Mon_Info (operand1->value.var.ssa);
		ready = src_lm_info->visited;
	      }
	    else if (PSS_Expr_Same_VarDcl (operand2, mu_var))
	      {
		src_lm_info = Get_Lin_Mon_Info (operand2->value.var.ssa);
		if (src_lm_info->visited)
		  {
		    Expr temp;
		    temp = operand1;
		    operand1 = operand2;
		    operand2 = temp;
		    ready = 1;
		  }
	      }
	    sub_flag = 0;
	    break;
	  case OP_sub:
	    if (PSS_Expr_Same_VarDcl (operand1, mu_var))
	      {
		src_lm_info = Get_Lin_Mon_Info (operand1->value.var.ssa);
		ready = src_lm_info->visited;
	      }
	    sub_flag = 1;
	    break;
	  case OP_phi:
	    ready = PSS_Linear_Monotonic_Phi_Operands_Ready (operation);
	    break;
	  default:
	    P_punt ("PSS_Find_Linear_Monotonic_Increment: encountered "
		    "unhandleable opcode.");
	  }
	if (!ready)
	  continue;

	/* If the operands are ready, process expr */
	processed = Set_add (processed,
			     assignment->operands->value.var.ssa->subscr);
	dest_lm_info->visited = 1;
	switch (operation->opcode)
	  {
	  case OP_add:
	  case OP_sub:
	    min_select = PSS_Create_Select_Term (src_lm_info->min_incr);
	    max_select = PSS_Create_Select_Term (src_lm_info->max_incr);
	    dest_lm_info->min_incr = min_select;
	    dest_lm_info->max_incr = max_select;

	    PSS_Add_Select_Increment (min_select, operand2, bb_id, loop,
				      sub_flag);
	    PSS_Add_Select_Increment (max_select, operand2, bb_id, loop,
				      sub_flag);
	    break;
	  case OP_phi:
	    PSS_Linear_Monotonic_Phi (operation, dest_lm_info, loop, 0);
	    break;
	  default:
	    P_punt ("PSS_Find_Linear_Monotonic_Increment: should never "
		    "reach this point.");
	  }
	/* Need to break out of the loop, start from beginning. */
	break;
      }

  } while (!Set_same (processed, total));

  /* Attach increment to induction expression */
  mu_lm_info = PSS_Create_Lin_Mon_Info ();
  PSS_Linear_Monotonic_Phi (mu_expr, mu_lm_info, loop, 1);
  ind_expr->ind_coeff = PSS_Create_Add_Term (NULL);
  ind_expr->ind_coeff->type = TERM_SELECT;
  ind_expr->ind_coeff->term.select = mu_lm_info->min_incr;
  mu_lm_info->min_incr = NULL;

  ind_expr->max_ind_coeff = PSS_Create_Add_Term (NULL);
  ind_expr->max_ind_coeff->type = TERM_SELECT;
  ind_expr->max_ind_coeff->term.select = mu_lm_info->max_incr;
  mu_lm_info->max_incr = NULL;

  /* Cleanup */
  PSS_Linear_Monotonic_Increment_Cleanup (scc);
  PSS_Delete_Lin_Mon_Info (mu_lm_info);
  Set_dispose (processed);
  Set_dispose (total);

      new_ind_expr->flags |= LIN_MONOTONIC_IV;
  return ind_expr;
}
#endif


/* SER 20041201
 * Finds the initial value for an IV.  If there is one source, use that.
 * Otherwise, use the Pcode expression for the mu-function.
 */
static PSS_Expr
PSS_Find_IV_Initial_Values (Expr mu_expr, int bb_id, PC_Loop loop,
				  PSS_TarSCC scc)
{
  Expr operand, initial_value;
  int defs = 0, in_defs = 0;

  for (operand = mu_expr->operands; operand; operand = operand->next)
    {
      PSS_Def ssa_def = operand->value.var.ssa;
      defs++;
      if (!UNINITIALIZED_TYPE (ssa_def->type) &&
	  Set_in (Get_Loop_body (loop), ssa_def->bb->ID))
	continue;
      initial_value = operand;
      /* If there's more than 1 incoming def, return Pcode expression. */
      if (++in_defs > 1)
	return (PSS_Unsupported_Opcode_Expr (mu_expr));
    }

  /* At this point, there's only one incoming def, so return that. */
  return (PSS_SSA_Use (initial_value, bb_id, loop, 0));
}


/* SER 20041201
 * This function multiplies each Add_Term in a chain by a fundamental
 * induction variable and returns the result as a PSS_Expr.
 */
static PSS_Expr
PSS_Multiply_By_Fund_Ind_Var (PSS_Expr old_expr, PC_Loop loop)
{
  PSS_Expr new_expr;
  PSS_Add_Term new_add, iter_add;
  
  new_expr = PSS_Create_Expr (NULL);
  new_add = PSS_Create_Add_Term (old_expr->terms);
  new_expr->terms = new_add;

  for (iter_add = new_add; iter_add; iter_add = iter_add->next)
    {
      PSS_Mul_Term mul_fiv = PSS_Create_Mul_Term (NULL);
      mul_fiv->type = TERM_FUND_IND_VAR;
      mul_fiv->term.fund_ind_var = Get_Fund_Ind_Var (loop);
      iter_add->mul = PSS_Combine_Mul_Terms (iter_add->mul, mul_fiv);
    }

  /* SER 20041215: Need to add a leading constant. */
  new_add = PSS_Create_Add_Term (NULL);
  new_add->int_coeff = 0;
  new_add->next = new_expr->terms;
  new_expr->terms = new_add;

  return new_expr;
}


/* Processes a linear IV to find initial condition, increment. 
 * Assumes that the SCC list is in lexical order.
 */
static PSS_Expr
PSS_Linear_IV (Expr mu_expr, int bb_id, PC_Loop loop, PSS_TarSCC scc)
{
  PSS_Expr invar_expr, incr_expr, ind_expr, total_expr;

#ifdef DEBUG_IV_CALC
  fprintf (stdout, "PSS_Linear_IV: var %s, bb_id %d, loop %d, "
	   "func %s.\n", mu_expr->operands->value.string, bb_id, loop->ID,
	   current_CFG->func->name);
#endif
  if (scc->type != LINEAR)
    P_punt ("PSS_Linear_IV: trying to process non-linear SCC.");

  /* Find initial value of IV. */
  invar_expr = PSS_Find_IV_Initial_Values (mu_expr, bb_id, loop, scc);
  /* Trace back through SCC to find increment of expression */
  incr_expr = PSS_Find_Linear_Increment (mu_expr, bb_id, loop, scc);

  ind_expr = PSS_Multiply_By_Fund_Ind_Var (incr_expr, loop);
  total_expr = PSS_Arithmetic_Exprs (invar_expr, ind_expr, 0);
  total_expr->flags |= LINEAR_IV;

  PSS_Delete_Expr (invar_expr);
  PSS_Delete_Expr (incr_expr);
  PSS_Delete_Expr (ind_expr);

  return total_expr;
}

#ifdef DO_LIN_MON_IVS
/* Needs rewrite due to grammar change. */
static PSS_Expr
PSS_Linear_Monotonic_IV(Expr mu_expr, int bb_id,
		       PC_Loop loop, PSS_TarSCC scc)
{
  PSS_Expr new_ind_expr;

#ifdef DEBUG_IV_CALC
  fprintf (stdout, "PSS_Linear_Monotonic_IV, var %s, bb_id %d, loop %d, "
	   "func %s.\n", mu_expr->operands->value.string, bb_id, loop->ID,
	   current_CFG->func->name);
#endif

  new_ind_expr = PSS_Find_IV_Initial_Values (mu_expr, bb_id, loop, scc);
  new_ind_expr = PSS_Find_Linear_Monotonic_Increment (mu_expr, new_ind_expr,
						      bb_id, loop, scc);
  return new_ind_expr;
}
#endif

static PSS_Expr
PSS_Unknown_IV(Expr expr, int bb_id)
{
  PSS_Expr new_ind_expr;

  new_ind_expr = PSS_Unsupported_Opcode_Expr (expr);
  new_ind_expr->flags |= UNKNOWN_IV;

  return new_ind_expr;
}


void
PSS_Find_IVs (PC_Loop loop)
{
  for (; loop; loop = loop->next)
    {
      int * bb_ids, num_bbs, i;

      bb_ids = (int *) calloc (Set_size (Get_Loop_body (loop)), sizeof (int));
      num_bbs = Set_2array (Get_Loop_body (loop), bb_ids);
      for (i = 0; i < num_bbs; i++)
	{
	  PC_Block bb;
	  Expr expr;
	  _PC_ExprIter ei;
	  /* When we process an expression, we must give it the
	   * deepest loop nest.  This way we only examine each
	   * mu-function once, too. */
	  if (PC_BB_In_Child_Loop (bb_ids[i], loop))
	    continue;
	  bb = PC_FindBlock (current_CFG, bb_ids[i]);

	  for (expr = PC_ExprIterFirst(bb, &ei, 1); expr;
	       expr = PC_ExprIterNext(&ei, 1))
	    {
	      PSS_Expr new_ind_expr;
	      /* Look for mu-functions */
	      if (expr->opcode != OP_assign)
		continue;
	      if (expr->operands->opcode != OP_var)
		continue;
	      if (expr->operands->sibling->opcode != OP_phi)
		continue;
	      if (!MU_TYPE (expr->operands->value.var.ssa->type))
		continue;
	      if (!(expr->operands->value.var.ssa->uses))
		continue;

	      /* Find mu-function expression in terms of fund ind var */
	      PC_PrintLoop (stdout, loop);
	      fprintf (stdout, "Mu-function expression in func %s, loop %d, "
		       "variable %s:\n", current_CFG->func->name, loop->ID,
		       expr->operands->value.var.name);
	      new_ind_expr = PSS_Assign (expr, bb_ids[i], loop, 0);
	      PSS_Print_Expr (stdout, new_ind_expr);
	      printf("\n");
	      fprintf (stdout, " ====================\n");
	      PSS_Delete_Expr (new_ind_expr);
	    }
	}
    }
}


static int
PSS_BB_DOM_All_Loopbacks (PC_Loop loop, PC_Block bb)
{
  PC_Block head_bb;
  PC_Flow flow;

  head_bb = PC_FindBlock (current_CFG, loop->head);
  for (flow = head_bb->p_flow; flow; flow = flow->p_next_flow)
    {
      /* First, check if loopback by seeing if src_bb is in loop */
      if (!Set_in (Get_Loop_body (loop), flow->src_bb->ID))
	continue;
      /* If the input BB doesn't DOM the loopback BB, return 0 */
      if (!PC_BB1DominatesBB2 (bb, flow->src_bb))
	return 0;
    }
  return 1;
}

/* Finds the final value of an expression in a loop for the purpose of
 * finding the loop bounds. */
static PSS_Expr
PSS_Find_Loop_Final_Value (PC_Loop loop, PC_Block bb, Expr exit_cond)
{
  Expr cond, operand1, operand2;
  PSS_Expr ind_expr1, ind_expr2, new_ind_expr;
  PSS_Fund_Ind_Var fund_ind_var;
  PSS_Bound_Condition new_bound;
  int exit_taken, unknown_opc = 0;

  if (bb->cont_type == CNT_GOTO)
    P_punt ("PSS_Find_Expr_Final_Value: Shouldn't have a goto at the "
	    "end of the input BB.");
  cond = bb->cond;
  if (!cond)
    P_punt ("PSS_Find_Loop_Final_Value: No condition attached to BB %d.\n",
	    bb->ID);
  if (!exit_cond || exit_cond->opcode != OP_int)
    P_punt ("PSS_Find_Loop_Final_Value: No condition given for loop %d, "
	    "func %s.", loop->ID, current_CFG->func->name);
  exit_taken = (exit_cond->value.scalar);

  fund_ind_var = Get_Fund_Ind_Var (loop);
#ifdef DEBUG_BOUNDS_ANALYSIS
  PC_PrintLoop (stdout, loop);
#endif
  switch (cond->opcode)
    {
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
    case OP_eq:
    case OP_ne:
      operand1 = cond->operands;
      operand2 = operand1->sibling;
      /* SER 20041026: Hack for ind vars of special type: ignore cast */
      if (operand1->opcode == OP_cast && operand1->operands->opcode == OP_var)
	operand1 = operand1->operands;
      if (operand2->opcode == OP_cast && operand2->operands->opcode == OP_var)
	operand2 = operand2->operands;
      ind_expr1 = PSS_Generic_Expr (operand1, bb->ID, loop, 0);
      ind_expr2 = PSS_Generic_Expr (operand2, bb->ID, loop, 0);

      /* subtract right expr from left */
      new_ind_expr = PSS_Arithmetic_Exprs (ind_expr1, ind_expr2, 1);
      PSS_Delete_Expr (ind_expr1);
      PSS_Delete_Expr (ind_expr2);
      break;
    default:  /* Assume "expr == 0" exits loop */
      unknown_opc = 1;
      operand1 = cond;
      new_ind_expr = PSS_Generic_Expr (operand1, bb->ID, loop, 0);
    }

  new_bound = PSS_Create_Bound_Condition (NULL);
  new_bound->cond = new_ind_expr;
  new_bound->bb = bb;
  new_bound->pcloop = loop;

  new_bound->next = fund_ind_var->bounds;
  fund_ind_var->bounds = new_bound;

  /* At this point, we have an ind expr compared to 0 for continuation.
   * Now generate the direction for exit and separate out the terms.
   */
  if (!exit_taken)
    {
      if (unknown_opc)
	new_bound->cont_opcode = OP_ne;
      else
	new_bound->cont_opcode = cond->opcode;
    }
  else  /* If exit_taken == 1, then we have to switch the continuation
	 * opcode on the fund ind var. */
    {
      switch (cond->opcode)
	{
	case OP_lt:
	  new_bound->cont_opcode = OP_ge;
	  break;
	case OP_le:
	  new_bound->cont_opcode = OP_gt;
	  break;
	case OP_ge:
	  new_bound->cont_opcode = OP_lt;
	  break;
	case OP_gt:
	  new_bound->cont_opcode = OP_le;
	  break;
	case OP_eq:
	  new_bound->cont_opcode = OP_ne;
	  break;
	case OP_ne:
	default:
	  new_bound->cont_opcode = OP_eq;
	}
    }

  /* There are cases where the termination condition is precisely
   * defined as "!=": Omega test probably can't handle this, and it can't
   * be corrected via arithmetic.  What CAN be done is analyze the
   * condition and attempt to weaken the opcode so that we can
   * correct the condition via arithmetic.
   */
  if (new_bound->cont_opcode == OP_ne)
    PSS_Weaken_NE_Bound_Condition (new_bound);

#if defined DEBUG_BOUNDS_ANALYSIS || defined PRINT_LOOP_BOUNDS
  fprintf (stdout, "Bound condition for loop %d, func %s:\n\t", loop->ID,
	   current_CFG->func->name);
  PSS_Print_Loop_Bounds(stdout, new_bound);
  fprintf(stdout, "\n");
#endif

  return (new_ind_expr);
}


/* SER:  Finds loop exits that dominate all loopback edges, and thus must
 * be examined during every iteration of the loop (i.e., not a side exit).
 */
PSS_Bound_Condition
PSS_Find_Loop_Bound (PC_Loop loop)
{
  PC_Block head_bb;
  int * exit_bbs, num_exit_bbs, i, side_exits = 0;
  PSS_Fund_Ind_Var fund_ind_var = Get_Fund_Ind_Var (loop);

  /* Leave early if bounds for the loop have already been computed. */
  if (fund_ind_var->final_type != FV_UNKNOWN)
    return (fund_ind_var->bounds);

  head_bb = PC_FindBlock (current_CFG, loop->head);
  exit_bbs = (int *) calloc (Set_size (Get_Loop_exits (loop)), sizeof (int));
  num_exit_bbs = Set_2array (Get_Loop_exits (loop), exit_bbs);

  for (i = 0; i < num_exit_bbs; i++)
    {
      PC_Block exit_bb;
      PC_Flow pred;
      Expr cond;

      exit_bb = PC_FindBlock (current_CFG, exit_bbs[i]);
      for (pred = exit_bb->p_flow; pred; pred = pred->p_next_flow)
	{
	  PC_Block loop_bb = pred->src_bb;

	  /* First, check to see if the block that flows to the exit
	   * is in the loop. */
	  if (!Set_in (Get_Loop_body (loop), loop_bb->ID))
	    continue;

	  /* If a branch that exits the loop isn't from a block that
	   * dominates all loopbacks, then it's a side exit.  We
	   * can't do much about those at this point. */
	  if (!PSS_BB_DOM_All_Loopbacks (loop, loop_bb))
	    {
	      side_exits++;
	      continue;
	    }

	  /* At this point, we know that the condition of the branch
	   * is the/an exit condition of the loop.  Solve for it. */
	  cond = pred->flow_cond;
	  PSS_Find_Loop_Final_Value (loop, loop_bb, cond);
	}
    }
  if (side_exits)
    {
#if defined DEBUG_BOUNDS_ANALYSIS || defined PRINT_LOOP_BOUNDS
      fprintf (stdout, "Side exits found in loop %d, func %s.\n",
	       loop->ID, current_CFG->func->name);
#endif
      fund_ind_var->final_type = FV_RANGE;
    }
  else
    fund_ind_var->final_type = FV_DISCRETE;

#if defined DEBUG_BOUNDS_ANALYSIS || defined PRINT_LOOP_BOUNDS
  fprintf (stdout, "====================\n");
#endif
  return (fund_ind_var->bounds);
}


/* SER 20041112: this goes through an Add_Term list and computes the loop
 * bounds for any expressions in the list. */
static List
PSS_Add_Term_Loop_Bounds (PSS_Add_Term add_term, List bound_list)
{
  PSS_Bound_Condition new_bound, iter_bound;
  PSS_Fund_Ind_Var fiv;
  int found;

  for (; add_term; add_term = add_term->next)
    {
      PSS_Mul_Term mul_term;
 
      for (mul_term = add_term->mul; mul_term; mul_term = mul_term->next)
	{
	  switch (mul_term->type)
	    {
	    case TERM_SELECT:
	      bound_list =
		PSS_Select_Term_Loop_Bounds (mul_term->term.select,
					     bound_list);
	      break;
	    case TERM_LIN_MON_INFO:
	      bound_list =
		PSS_Select_Term_Loop_Bounds (mul_term->term.lin_mon->min_incr,
					     bound_list);
	      bound_list=
		PSS_Select_Term_Loop_Bounds (mul_term->term.lin_mon->max_incr,
					     bound_list);
	      break;
	    case TERM_FUND_IND_VAR:
	      found = 0;
	      fiv = mul_term->term.fund_ind_var;
	      new_bound = PSS_Find_Loop_Bound (fiv->pcloop);

	      List_start (bound_list);
	      while ((iter_bound = (PSS_Bound_Condition)
		      List_next (bound_list)))
		{
		  if (iter_bound->pcloop == fiv->pcloop)
		    {
		      found = 1;
		      break;
		    }
		}
	      if (!found)
		{
		  new_bound = PSS_Create_Bound_Condition (new_bound);
		  bound_list = List_insert_last (bound_list, new_bound);
		}
	      break;
	    case TERM_SSA_DEF:
	    case TERM_PCODE_EXPR:
	      break;
	    default:
	      P_punt ("PSS_Add_Term_Loop_Bounds: invalid mul type %d "
		      "encountered.", mul_term->type);
	    }
	}
    }
  return bound_list;
}


/* SER 20041120: Given a PSS_Select_Term, find the bound for every loop
 * whose fundamental induction variable is contained in any of its terms.
 */
static List
PSS_Select_Term_Loop_Bounds (PSS_Select_Term select_term,
					   List bound_list)
{
  for (; select_term; select_term = select_term->next)
    {
      bound_list = PSS_Add_Term_Loop_Bounds (select_term->add,
					     bound_list);
    }

  return bound_list;
}

/* SER 20041112: Given a PSS_Expr, finds the bound for every loop whose
 * fundamental induction variable is contained in the expression.
 */
List
PSS_Find_Relevant_Loop_Bounds (PSS_Expr expr, List bound_list)
{
  return (PSS_Add_Term_Loop_Bounds (expr->terms, bound_list));
}


/* SER 20041025: This function attempts to find the bounds
 * of every fundamental induction variable in the loop.
 */
void
PSS_Find_All_Loop_Bounds (PC_Loop loop)
{
  for (; loop; loop = loop->next)
    PSS_Find_Loop_Bound (loop);
}


/* SER 20041115
 * There are cases where the termination condition is defined as
 * not-equal.  Omega test probably can't handle this, and it can't be
 * corrected via arithmetic.  What CAN be done is to analyze the growth
 * of the condition and attempt to weaken the opcode so that we can
 * correct the condition via arithmetic.
 */
static void
PSS_Weaken_NE_Bound_Condition (PSS_Bound_Condition bound)
{
  PSS_Expr ind_coeff, invar_coeff;
  int ind_int, invar_int, is_constant;

  if (bound->cont_opcode != OP_ne)
    P_punt ("PSS_Weaken_NE_Bound_Condition: shouldn't be giving it a "
	    "non-OP_ne bound.");

  ind_coeff = PSS_Expr_Extract_Ind_Coeff (bound->cond, bound->pcloop);
  /* Can't do anything about non-integer increment */
  is_constant = PSS_Expr_Is_Int_Constant (ind_coeff, &ind_int);
  ind_coeff = PSS_Delete_Expr (ind_coeff);
  if (!is_constant || ind_int == 0)
    return;

  invar_coeff = PSS_Expr_Extract_Invar_Coeff (bound->cond, bound->pcloop);
  is_constant = PSS_Expr_Is_Int_Constant (invar_coeff, &invar_int);
  invar_coeff = PSS_Delete_Expr (invar_coeff);
  if (!is_constant || invar_int == 0)
    return;

  /* Make sure that the increment won't skip the exit value */
  if ((invar_int % ind_int) != 0)
    return;

  /* Change the opcode based on the signs of the coefficients */
  /* increasing IV */
  if ((ind_int < 0) && (invar_int > 0))
    {
      bound->cont_opcode = OP_gt;
#ifdef DEBUG_BOUNDS_CORRECTION
      fprintf (stdout, "PSS_Weaken_NE_Bound_Condition: modified "
	       "linearly increasing IV.\n");
#endif
    }
  /* decreasing IV */
  else if ((ind_int > 0) && (invar_int < 0))
    {
      bound->cont_opcode = OP_lt;
#ifdef DEBUG_BOUNDS_CORRECTION
      fprintf (stdout, "PSS_Weaken_NE_Bound_Condition: modified "
	       "linearly decreasing IV.\n");
#endif
    }
}


/* SER 20041115
 * If a memory access isn't dominated by a loop exit, then it occurs
 * prior to the exit.  This means that the last value of the fundamental
 * induction variable (the first one that violates the condition) is
 * applicable for the memory access.  To correct for this, we allow one more
 * value of h in the condition.
 * Input: block of memory access, list of bound conditions.
 * Output: modifies list of bound conditions.
 */
void
PSS_Correct_Bounds_For_Access_Position (PC_Block bb, List bound_list)
{
  PSS_Bound_Condition bound;

  /* SER: Don't forget that each item in the List is actually a linked list
   * of bounds, one for each loop! */
  List_start (bound_list);
  while ((bound = (PSS_Bound_Condition) List_next (bound_list)))
    {
      for (; bound; bound = bound->next)
	{
	  PSS_Expr ind_coeff, new_cond;
	  if (PC_BB1DominatesBB2 (bound->bb, bb) && (bound->bb != bb))
	    continue;

#ifdef DEBUG_BOUNDS_CORRECTION
	  fprintf (stdout, "Bound to be corrected at bb %d.\n", bb->ID);
#endif

	  /* Need to modify the bound at this point. We want (h+1) to also be
	   * true, so subtract the ind_coeff from the invar_coeff. */
	  switch (bound->cont_opcode)
	    {
	    case OP_lt:
	    case OP_le:
	    case OP_gt:
	    case OP_ge:
	      ind_coeff = PSS_Expr_Extract_Ind_Coeff (bound->cond,
						      bound->pcloop);
	      new_cond = PSS_Arithmetic_Exprs (bound->cond, ind_coeff, 1);
	      PSS_Delete_Expr (bound->cond);
	      PSS_Delete_Expr (ind_coeff);
	      bound->cond = new_cond;
#ifdef DEBUG_BOUNDS_CORRECTION
	      fprintf (stdout, "\tLinear IV bound corrected.\n");
#endif
	      break;
	    case OP_ne:
	    case OP_eq:
#ifdef DEBUG_BOUNDS_CORRECTION
	      /* Not sure how to handle other cases: look at specifics. */
	      fprintf (stdout, "PSS_Correct_Bounds_For_Access_Position: "
		       "don't know how to handle given opcode.");
#endif
	      break;		  
	    default:
	      P_punt ("PSS_Correct_Bounds_For_Access_Position: bad "
		      "opcode %d.\n", bound->cont_opcode);
	    }
	}
    }
}

/*******************************************
 * INDUCTION EXPRESSION PRINTING FUNCTIONS *
 *******************************************/


static void
PSS_Print_Select_Term (FILE * file, PSS_Select_Term select_term)
{
  char * separator;
  if (select_term == NULL)
    return;

  switch (select_term->sel_type)
    {
    case SELECT_PHI:
      fprintf (file, "phi{");
      separator = " | ";
      break;
    case SELECT_MIN:
      fprintf (file, "min{");
      separator = ", ";
      break;
    case SELECT_MAX:
      fprintf (file, "max{");
      separator = ", ";
      break;
    case SELECT_NONE:
    default:
      P_punt ("PSS_Print_Select_Term: invalid select type encountered.");
    }

  for (; select_term; select_term = select_term->next)
    {
      PSS_Print_Add_Term (file, select_term->add);

      if (select_term->next)
	fprintf (file, "%s", separator);
    }

  fprintf (file, "}");
}


static void
PSS_Print_Add_Term (FILE * file, PSS_Add_Term add_term)
{
  if (add_term == NULL)
    return;

  fprintf (file, "%d", add_term->int_coeff);
  PSS_Print_Mul_Term (file, add_term->mul);
  if (add_term->next)
    {
      fprintf (file, " + ");
      PSS_Print_Add_Term (file, add_term->next);
    }
}


static void
PSS_Print_Mul_Term (FILE * file, PSS_Mul_Term mul_term)
{
  int lines = 0;
  if (mul_term == NULL)
    return;
  switch (mul_term->type)
    {
    case TERM_SELECT:
      fprintf (file, "*");
      PSS_Print_Select_Term (file, mul_term->term.select);
      break;
    case TERM_FUND_IND_VAR:
      fprintf (file, "*h(loop %d)", mul_term->term.fund_ind_var->pcloop->ID);
      if (mul_term->final_value)
	fprintf (file, "(final)");
      break;
    case TERM_LIN_MON_INFO:
      fprintf (file, "[min_incr (");
      PSS_Print_Select_Term (file, mul_term->term.lin_mon->min_incr);
      fprintf (file, "), max_incr (");
      PSS_Print_Select_Term (file, mul_term->term.lin_mon->max_incr);
      fprintf (file, ")");
      break;
    case TERM_SSA_DEF:
      fprintf (file, "*{");
      if (PARAM_TYPE (mul_term->term.ssa_def->type))
	fprintf (file, "PARAM %s}",
		 mul_term->term.ssa_def->uses->var->value.string);
      else if (UNDEF_TYPE (mul_term->term.ssa_def->type))
	fprintf (file, "UNDEF_SSA %s)",
		 mul_term->term.ssa_def->uses->var->value.string);
      else
	{
	  P_write_expr (file, mul_term->term.ssa_def->var, 0, &lines);
	  fprintf (file, "_%d}", mul_term->term.ssa_def->subscr);
	}
      break;
    case TERM_PCODE_EXPR:
      fprintf (file, "*{");
      P_write_expr (file, mul_term->term.pcode_expr, 0, &lines);
      fprintf (file, "}");
      break;
    case TERM_UNDEF:
    default:
      P_punt ("PSS_Print_Mul_Term: invalid type encountered.");
    }
  if (mul_term->next)
    PSS_Print_Mul_Term (file, mul_term->next);
}


void
PSS_Print_Expr (FILE * file, PSS_Expr ind_expr)
{
  if (ind_expr == NULL)
    {
      fprintf (file, "EMPTY\n");
      return;
    }
  if (ind_expr->flags & INFINITY)
    {
      fprintf (file, "INFINITY");
      return;
    }

  if (ind_expr->flags & UNKNOWN_IV)
    fprintf (file, "UNKNOWN_IV[");

  PSS_Print_Add_Term (file, ind_expr->terms);

  if (ind_expr->flags & UNKNOWN_IV)
    fprintf (file, "]");
}

void
PSS_Print_All_Expr (FILE * file, PC_Graph cfg, PC_Loop loop)
{
  for (; loop; loop = loop->sibling)
    {
      int * bb_ids, num_bbs, i;
      PSS_Print_All_Expr (file, cfg, loop->child);

      bb_ids = (int *) calloc (Set_size (Get_Loop_body (loop)), sizeof (int));
      num_bbs = Set_2array (Get_Loop_body (loop), bb_ids);
      for (i = 0; i < num_bbs; i++)
	{
	  PC_Block bb;
	  Expr expr;
	  _PC_ExprIter ei;
	  bb = PC_FindBlock (cfg, bb_ids[i]);

	  for (expr = PC_ExprIterFirst(bb, &ei, 1); expr;
	       expr = PC_ExprIterNext(&ei, 1))
	    {
	      PSS_Expr ind_expr;

	      if (expr->opcode != OP_assign ||
		  expr->operands->opcode != OP_var)
		continue;

	      ind_expr = PSS_GetExprIE(expr->operands);
	      if (ind_expr == NULL)
		continue;

	      PSS_Print_Expr (file, ind_expr);
	      printf("\n");
	    }
	}
    }
}


void
PSS_Print_Loop_Bounds (FILE * file, PSS_Bound_Condition bound)
{
  PSS_Print_Expr (file, bound->cond);
  switch (bound->cont_opcode)
    {
    case OP_lt:
      fprintf (file, " <");
      break;
    case OP_le:
      fprintf (file, " <=");
      break;
    case OP_ge:
      fprintf (file, " >=");
      break;
    case OP_gt:
      fprintf (file, " >");
      break;
    case OP_eq:
      fprintf (file, " ==");
      break;
    case OP_ne:
    default:
      fprintf (file, " !=");
    }
  fprintf (file, " 0; in BB %d.\n", bound->bb->ID);

  if (bound->next)
    PSS_Print_Loop_Bounds (file, bound->next);
}


static void
PSS_Add_Term_Print_Relevant_Loop_Bounds (FILE * file, PSS_Add_Term add_term)
{
  for (; add_term; add_term = add_term->next)
    {
      PSS_Mul_Term mul_term;

      for (mul_term = add_term->mul; mul_term; mul_term = mul_term->next)
	{
	  PSS_Select_Term select_term;
	  PSS_Bound_Condition bound;

	  switch (mul_term->type)
	    {
	    case TERM_SELECT:
	      for (select_term = mul_term->term.select; select_term;
		   select_term = select_term->next)
		{
		  PSS_Add_Term_Print_Relevant_Loop_Bounds
		    (file, select_term->add);
		}
	    case TERM_FUND_IND_VAR:
	      bound =
		PSS_Find_Loop_Bound (mul_term->term.fund_ind_var->pcloop);
	      PSS_Print_Loop_Bounds (file, bound);
	      break;
	    case TERM_SSA_DEF:
	    case TERM_PCODE_EXPR:
	      break;
	    default:
	      P_punt ("PSS_Add_Term_Print_Relevant_Loop_Bounds: invalid "
		      "mul type %d encountered.", mul_term->type);
	    }
	}
    }
}


void
PSS_Print_Relevant_Loop_Bounds (FILE * file, PSS_Expr ind_expr)
{
  PSS_Add_Term_Print_Relevant_Loop_Bounds (file, ind_expr->terms);
}
