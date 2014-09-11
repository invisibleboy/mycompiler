/*****************************************************************************\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:             
 *
 *		IMPACT Research Group
 *
 *		University of Illinois at Urbana-Champaign
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
/*****************************************************************************
 * phase1_pred.c                                                             *
 * ------------------------------------------------------------------------- *
 * Customization of Lcode compares prior to Ltahoe Mcode generation          *
 *                                                                           *
 * AUTHORS: J.W. Sias                                                        *
 *****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "phase1_func.h"
#include <Lcode/l_opti.h>
#include <Lcode/l_pred_opti.h>
#include <Lcode/l_predicate.h>

/*
 * Ltahoe_split_lcode_pred_defines(L_Func *fn);
 * ------------------------------------------------------------------------
 * Split pred defines and generate floating point precompares
 * JWS 20010117
 */

static void
Ltahoe_split_lcode_pred_defines (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op, *new_op, *next_op;
  int ptype1 = 0, ptype2 = 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = next_op)
	{
	  next_op = op->next_op;
	  if (L_general_pred_comparison_opcode (op))
	    {
	      /* 1. Split 2-dest compares into 2 1-dest compares */

	      /* Check for pred mac p0 and delete it */
	      if (op->dest[0] && L_is_macro (op->dest[0]) &&
		  op->dest[0]->value.mac == TAHOE_MAC_PRED_TRUE)
		{
		  L_delete_operand (op->dest[0]);
		  op->dest[0] = NULL;
		}
	      if (op->dest[1] && L_is_macro (op->dest[1]) &&
		  op->dest[1]->value.mac == TAHOE_MAC_PRED_TRUE)
		{
		  L_delete_operand (op->dest[1]);
		  op->dest[1] = NULL;
		}

	      if (!op->dest[0])
		{
		  if (!op->dest[1])
		    {
		      L_delete_oper (cb, op);
		      continue;
		    }
		  else
		    {
		      op->dest[0] = op->dest[1];
		      op->dest[1] = NULL;
		      L_copy_PD_attr (op, 0, op, 1);
		      L_remove_PD_attr (op, 1);
		    }
		}
	      else if (op->dest[1])
		{
		  new_op = L_create_new_op_using (op->opc, op);
		  L_copy_compare (new_op, op);
		  L_insert_oper_after (cb, op, new_op);
		  /* Make sure the new cmp can be split by 3 if necessary. */
		  next_op = new_op;
		  new_op->dest[0] = op->dest[1];
		  op->dest[1] = NULL;
		  new_op->src[0] = L_copy_operand (op->src[0]);
		  new_op->src[1] = L_copy_operand (op->src[1]);
		  L_copy_PD_attr (new_op, 0, op, 1);
		  L_remove_PD_attr (op, 1);
		}

	      /* 2. Split transparent floating point compares */

	      /* 3. Split parallel inequality compares */

	      if ((L_fp_pred_comparison_opcode (op) &&
		   !L_uncond_ptype (op->dest[0]->ptype)) ||
		  (L_inequality_compare (op) &&
		   L_is_update_predicate_ptype (op->dest[0]->ptype) &&
		   !L_is_int_zero (op->src[0]) &&
		   !L_is_int_zero (op->src[1])))
		{
		  switch (op->dest[0]->ptype)
		    {
		    case L_PTYPE_OR_T:
		      ptype1 = L_PTYPE_UNCOND_T;
		      ptype2 = L_PTYPE_OR_T;
		      break;
		    case L_PTYPE_OR_F:
		      ptype1 = L_PTYPE_UNCOND_F;
		      ptype2 = L_PTYPE_OR_T;
		      break;
		    case L_PTYPE_AND_T:
		      ptype1 = L_PTYPE_UNCOND_F;
		      ptype2 = L_PTYPE_AND_F;
		      break;
		    case L_PTYPE_AND_F:
		      ptype1 = L_PTYPE_UNCOND_T;
		      ptype2 = L_PTYPE_AND_F;
		      break;
		    case L_PTYPE_COND_T:
		    case L_PTYPE_COND_F:
		    default:
		      L_punt ("Ltahoe_split_lcode_pred_defines:"
			      " PTYPE %d unimplemented", op->dest[0]->ptype);
		    }

		  new_op = L_create_new_op (Lop_CMP);
		  L_set_compare (new_op, L_CTYPE_INT, Lcmp_COM_EQ);
		  new_op->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);
		  new_op->src[1] = L_new_int_operand (0, L_CTYPE_LLONG);
		  new_op->dest[0] = op->dest[0];
		  new_op->dest[0]->ptype = ptype2;

		  op->dest[0] = Ltahoe_new_pred_reg (ptype1);
		  new_op->pred[0] = L_copy_operand (op->dest[0]);
		  L_assign_ptype_null (new_op->pred[0]);
		  L_insert_oper_after (cb, op, new_op);
		}
	    }
	  else if (L_initializing_pred_define_opcode (op))
	    {
	      if ((op->opc == Lop_PRED_SET) || (op->opc == Lop_PRED_CLEAR))
		{
		  switch (op->opc)
		    {
		    case Lop_PRED_SET:
		      op->dest[0]->ptype = L_PTYPE_UNCOND_T;
		      break;
		    case Lop_PRED_CLEAR:
		      op->dest[0]->ptype = L_PTYPE_UNCOND_F;
		      break;
		    }
		  L_change_opcode (op, Lop_CMP);
		  L_set_compare (op, L_CTYPE_INT, Lcmp_COM_EQ);
		  op->src[0] = L_new_int_operand (0, L_CTYPE_LLONG);
		  op->src[1] = L_new_int_operand (0, L_CTYPE_LLONG);
		}
	    }
	}
    }
  return;
}

/*
 * Ltahoe_compatible_ptypes(int pa, int pb)
 * --------------------------------------------------------------------------
 * Return 1 if the dest ptypes can share a pred def; 0 otherwise.
 * JWS 20010117
 */
static int
Ltahoe_compatible_ptypes (int pa, int pb)
{
  int compat = 0;
  switch (pa)
    {
    case L_PTYPE_UNCOND_T:
      compat = (pb == L_PTYPE_UNCOND_F);
      break;
    case L_PTYPE_UNCOND_F:
      compat = (pb == L_PTYPE_UNCOND_T);
      break;
    case L_PTYPE_COND_T:
      compat = (pb == L_PTYPE_COND_F);
      break;
    case L_PTYPE_COND_F:
      compat = (pb == L_PTYPE_COND_T);
      break;
    case L_PTYPE_OR_T:
    case L_PTYPE_AND_F:
      compat = (pb == L_PTYPE_AND_F) || (pb == L_PTYPE_OR_T);
      break;
    case L_PTYPE_OR_F:
    case L_PTYPE_AND_T:
      compat = (pb == L_PTYPE_AND_T) || (pb == L_PTYPE_OR_F);
      break;
    }
  return compat;
}

static int
Ltahoe_no_incongruent_defs_between (L_Operand * operand, L_Oper * opA,
				    L_Oper * opB)
{
  int i;
  L_Oper *pA;

  if ((opA == NULL) || (opB == NULL))
    L_punt ("L_no_defs_between: opA and opB cannot be NULL");
  if (operand == NULL)
    return 1;

  for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
    {
      if (pA == opB)
	return 1;
      if (!PG_intersecting_predicates_ops (pA, opB))
	continue;
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (L_same_operand (pA->dest[i], operand))
	    {
	      int pta, ptb;
	      pta = pA->dest[i]->ptype;
	      ptb = operand->ptype;

	      if ((pta != ptb) && (pta != L_opposite_ptype (ptb)))
		return 0;
	    }
	}
    }

  L_punt ("L_no_defs_between: opB not found");
  return (0);
}

/*
 * Ltahoe_combine_lcode_compares(L_Func *fn)
 * --------------------------------------------------------------------------
 * Combine compatible pairs of single-dest compares into single compares
 * to save code size.
 * JWS 20010117
 */
static void
Ltahoe_combine_lcode_compares (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *opA, *opB, *nextA, *nextB;
  L_Operand *destA, *destB;
  int new_ptypeB;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (opA = cb->first_op; opA != NULL; opA = nextA)
	{
	  nextA = opA->next_op;
	  if (!L_general_pred_comparison_opcode (opA))
	    continue;

	  if (!opA->dest[0] && opA->dest[1])
	    {
	      opA->dest[0] = opA->dest[1];
	      opA->dest[1] = NULL;
	    }

	  if (opA->dest[1])
	    continue;
	  if (!(destA = opA->dest[0]))
	    continue;
	  for (opB = nextA; opB != NULL; opB = nextB)
	    {
	      nextB = opB->next_op;
	      if (!L_general_pred_comparison_opcode (opB))
		continue;

	      if (!opB->dest[0] && opB->dest[1])
		{
		  opB->dest[0] = opB->dest[1];
		  opB->dest[1] = NULL;
		}
	      if (opB->dest[1])
		continue;
	      if (!(destB = opB->dest[0]))
		continue;
	      if (!(L_equivalent_comparison_opcode (opA, opB) ||
		    L_complement_comparison_opcode (opA, opB)))
		continue;
	      if (!L_same_src_operands (opA, opB))
		continue;
	      if (!L_same_operand (opA->pred[0], opB->pred[0]))
		continue;
	      if (!L_all_src_operand_same_def_reachs (opA, opA, opB))
		continue;
	      if (!Ltahoe_no_incongruent_defs_between
		  (opB->dest[0], opA, opB))
		continue;
	      if (!L_no_uses_between (opB->dest[0], opA, opB))
		continue;
	      if (!L_no_br_between (opA, opB))
		continue;
	      if (!L_no_jsr_between (opA, opB))
		continue;
	      if (!L_no_danger (0, 0, 0, opA, opB))
		continue;

	      if (L_equivalent_comparison_opcode (opA, opB))
		new_ptypeB = destB->ptype;
	      else
		new_ptypeB = L_opposite_ptype (destB->ptype);

	      if (!Ltahoe_compatible_ptypes (destA->ptype, new_ptypeB))
		continue;

	      destB->ptype = new_ptypeB;

	      if (nextA == opB)
		nextA = nextA->next_op;

	      if (destB->value.pred.reg != opA->dest[0]->value.pred.reg)
		{
		  opB->dest[0] = NULL;
		  opA->dest[1] = destB;
		  L_copy_PD_attr (opA, 1, opB, 0);
		  L_delete_oper (cb, opB);
		  break;
		}
	      else
		{
		  /* Performed CSE with identical destination registers */

		  if (!L_same_operand (destA, destB))
		    L_warn ("Ltahoe_combine_lcode_compares: "
			    "Combining dissimlar compares to the same predicate.");

		  continue;
		}
	    }
	}
    }
  return;
}

void
Ltahoe_insert_pred_init (L_Cb * cb, int opc, int pred_reg_id)
{
  L_Oper *new_op;
  L_Oper *indx_op;

  new_op = L_create_new_op (opc);

  new_op->dest[0] = L_new_register_operand (pred_reg_id, L_CTYPE_PREDICATE,
					    L_PTYPE_NULL);

  if (cb->prev_cb)
    {
      L_insert_oper_before (cb, cb->first_op, new_op);
    }
  else
    {
      /* First cb in function -- need to avoid prologue */
      indx_op = cb->first_op;
      while (indx_op && indx_op->opc != Lop_PROLOGUE)
	indx_op = indx_op->next_op;
      if (!indx_op)
	L_punt ("Ltahoe_insert_pred_init: prologue not found.");
      L_insert_oper_after (cb, indx_op, new_op);
    }

  return;
}

static void
Ltahoe_fixup_lcode_compares (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *new_pred, *p_not, *next_op;
  int pred_reg_id;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;

      for (oper = cb->first_op; oper != NULL; oper = next_op)
	{
	  next_op = oper->next_op;

	  if (!L_general_pred_comparison_opcode (oper) || !oper->dest[0])
	    continue;

	  if (L_sand_ptype (oper->dest[0]->ptype))
	    {
	      if (L_sand_true_ptype (oper->dest[0]->ptype))
		oper->dest[0]->ptype = L_PTYPE_AND_T;
	      else
		oper->dest[0]->ptype = L_PTYPE_AND_F;

	      if (L_is_predicated (oper))
		{
		  fn->max_reg_id++;
		  pred_reg_id = fn->max_reg_id;
		  Ltahoe_insert_pred_init (cb, Lop_PRED_SET, pred_reg_id);

		  p_not = L_create_new_op (Lop_CMP);
		  L_set_compare (p_not, L_CTYPE_INT, Lcmp_COM_EQ);

		  p_not->dest[0] = L_new_register_operand (pred_reg_id,
							   L_CTYPE_PREDICATE,
							   L_PTYPE_AND_F);
		  p_not->src[0] = L_new_int_operand (0, L_CTYPE_INT);
		  p_not->src[1] = L_new_int_operand (0, L_CTYPE_INT);
		  p_not->pred[0] = oper->pred[0];
		  L_insert_oper_before (cb, oper, p_not);

		  oper->pred[0] = NULL;

		  new_pred = L_create_new_op (Lop_CMP);
		  L_set_compare (new_pred, L_CTYPE_INT, Lcmp_COM_EQ);
		  new_pred->dest[0] = L_copy_operand (oper->dest[0]);
		  new_pred->dest[0]->ptype = L_PTYPE_AND_F;
		  new_pred->pred[0] = L_copy_operand (p_not->dest[0]);
		  new_pred->pred[0]->ptype = L_PTYPE_NULL;
		  new_pred->src[0] = L_new_int_operand (0, L_CTYPE_INT);
		  new_pred->src[1] = L_new_int_operand (0, L_CTYPE_INT);

		  L_insert_oper_after (cb, oper, new_pred);
		}
	    }
	}
    }
  return;
}

/*
 * Ltahoe_customize_lcode_compares(L_Func *fn)
 * --------------------------------------------------------------------------
 * Perform some easy predicate define optimizations
 * JWS 20010117
 */

void
Ltahoe_customize_lcode_compares (L_Func * fn)
{
  Ltahoe_split_lcode_pred_defines (fn);
  Ltahoe_fixup_lcode_compares (fn);
  PG_setup_pred_graph (fn);
  if (L_do_machine_opt && Ltahoe_do_lightweight_pred_opti)
    {
      if (L_lightweight_pred_opti (fn))
	{
	  Ltahoe_split_lcode_pred_defines (fn);
	  PG_setup_pred_graph (fn);	/* Required by L_same_def_reachs */
	}
    }

  L_partial_dead_code_removal (fn);

  Ltahoe_combine_lcode_compares (fn);
  PG_setup_pred_graph (fn);
  return;
}

void
Ltahoe_combine_pred_inits (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;
  L_Operand *pd;
  int i;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  if (!L_general_pred_comparison_opcode (op))
	    continue;
	  if (op->pred[0])
	    continue;

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      DdNode *mn = NULL;
	      PG_Pred_SSA *pssa;

	      if (!(pd = op->dest[i]))
		continue;
	      if (!L_is_transparent_predicate_ptype (pd->ptype))
		break;

	      switch (pd->ptype)
		{
		case L_PTYPE_COND_F:
		case L_PTYPE_COND_T:
		  mn = NULL;
		  break;
		case L_PTYPE_AND_F:
		case L_PTYPE_AND_T:
		  mn = PG_pred_graph->one;
		  break;
		case L_PTYPE_OR_F:
		case L_PTYPE_OR_T:
		  mn = PG_pred_graph->zero;
		  break;
		default:
		  L_punt ("Ltahoe_combine_pred_inits: unexpected ptype");
		}		/* switch */

	      if (!mn)
		continue;

	      if (!(pssa = pd->value.pred.ssa->prev_dest_ssa))
		break;

	      if (mn != pssa->node)
		break;
	    }			/* for i */

	  if (i < L_max_dest_operand)
	    continue;

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!(pd = op->dest[i]))
		continue;
	      switch (pd->ptype)
		{
		case L_PTYPE_COND_F:
		case L_PTYPE_AND_F:
		case L_PTYPE_OR_F:
		  pd->ptype = L_PTYPE_UNCOND_F;
		  break;
		case L_PTYPE_COND_T:
		case L_PTYPE_AND_T:
		case L_PTYPE_OR_T:
		  pd->ptype = L_PTYPE_UNCOND_T;
		  break;
		default:
		  L_punt ("Ltahoe_combine_pred_inits: unexpected ptype");
		}		/* switch */
	    }			/* for i */
	}			/* for op */
    }				/* for cb */

  return;
}				/* Ltahoe_combine_pred_inits */
