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
/*===========================================================================
 *      File :          l_opti_predicates.c
 *      Description :   predicate functions for optimizer
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 01/14/03 REK Changing L_redundant_extension_fwd to use L_store_ctype
 *              instead of L_opcode_ctype, since it was using that function
 *              incorrectly. */
/* 02/07/03 REK Modifying L_can_merge_with_op_above, L_can_merge_with_op_below,
 *              L_can_move_above, L_can_move_below, L_unique_memory_location,
 *              L_can_make_post_inc, L_can_make_pre_inc, so that they don't
 *              optimize opers marked volatile. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

#define L_MIN_WEIGHT_FOR_OP_MIG                 100.0
#define L_MAX_OP_MIG_COST_RATIO                 0.33	/* was .25 */
#define L_MAX_CB_FOR_PREHEADER_MOVE             100
#define L_MIN_ITER_FOR_IND_COMPLEX_ELIM         10.0
#define L_MIN_RATIO_FOR_IND_COMPLEX_ELIM        0.50

/*====================================================================*/
/*
 *      Predicate functions regarding opcodes
 */
/*====================================================================*/

/*
 *      ASSUME unsigned and signed add are compatible, in almost all
 *      cases this is true!!
 */

int
L_is_compatible_opc (int opc1, int opc2)
{
  switch (opc1)
    {
    case Lop_ADD:
      return ((opc2 == Lop_ADD) || (opc2 == Lop_ADD_U));
    case Lop_ADD_U:
      return ((opc2 == Lop_ADD) || (opc2 == Lop_ADD_U));
    case Lop_SUB:
      return ((opc2 == Lop_SUB) || (opc2 == Lop_SUB_U));
    case Lop_SUB_U:
      return ((opc2 == Lop_SUB) || (opc2 == Lop_SUB_U));
    default:
      return (opc1 == opc2);
    }
}

int
L_compatible_opcodes (L_Oper * oper1, L_Oper * oper2)
{
  int opc1, opc2;
  if (!oper1 || !oper2)
    return 0;
  if (L_same_opcode (oper1, oper2) && L_same_compare (oper1, oper2))
    return 1;
  opc1 = oper1->opc;
  opc2 = oper2->opc;
  switch (opc1)
    {
    case Lop_ADD:
      return (opc2 == Lop_ADD_U);
    case Lop_ADD_U:
      return (opc2 == Lop_ADD);
    case Lop_SUB:
      return (opc2 == Lop_SUB_U);
    case Lop_SUB_U:
      return (opc2 == Lop_SUB);
    default:
      return 0;
    }
}

int
L_compatible_arithmetic_ops (L_Oper * oper1, L_Oper * oper2)
{
  int opc1, opc2;
  if (!oper1 || !oper2)
    return 0;
  opc1 = oper1->opc;
  opc2 = oper2->opc;
  switch (opc1)
    {
    case Lop_ADD:
    case Lop_ADD_U:
    case Lop_SUB:
    case Lop_SUB_U:
      return ((opc2 == Lop_ADD_U) || (opc2 == Lop_SUB_U) ||
	      (opc2 == Lop_ADD) || (opc2 == Lop_SUB));
    default:
      L_punt ("L_compatible_arithmetic_ops: illegal opcode");
      return 0;
    }
}

int
L_compatible_to_combine_consts (L_Oper * oper1, L_Oper * oper2)
{
  if (!oper1 || !oper2)
    return 0;

  if (L_is_opcode (Lop_ADD, oper1) || L_is_opcode (Lop_SUB, oper1))
    {
      return (L_load_opcode (oper2) ||
	      L_store_opcode (oper2) ||
	      L_int_add_opcode (oper2) ||
	      L_int_sub_opcode (oper2) ||
	      L_signed_int_cond_branch_opcode (oper2) ||
	      L_signed_int_comparison_opcode (oper2));
    }
  else if (L_is_opcode (Lop_ADD_U, oper1) || L_is_opcode (Lop_SUB_U, oper1))
    {
      return (L_load_opcode (oper2) ||
	      L_store_opcode (oper2) ||
	      L_int_add_opcode (oper2) || L_int_sub_opcode (oper2));
    }

  L_punt ("L_compatible_to_combine_consts: illegal oper1");
  return 0;
}

int
L_unary_opcode (L_Oper * oper)
{
  int opc;
  if (!oper)
    return 0;
  opc = oper->opc;
  return ((opc >= Lop_F2_I) & (opc <= Lop_F_F2));
}

int
L_are_opposite_branch_opcodes (L_Oper * oper1, L_Oper * oper2)
{
  ITuint8 cond1, cond2, ctype1, ctype2;

  if (!oper1 || !oper2)
    return 0;

  if ((oper1->opc != Lop_BR) && (oper1->opc != Lop_BR_F))
    L_punt ("L_are_opposite_branch_opcodes: non-branch argument op %d opc %d",
	    oper1->id, oper1->opc);
  if (oper2->opc != oper1->opc)
    return 0;

  ctype1 = oper1->com[0];
  ctype2 = oper2->com[0];
  cond1 = oper1->com[1];
  cond2 = oper2->com[1];

  return ((cond1 == L_opposite_pred_completer (cond2) && (ctype1 == ctype2)));
}

int
L_are_reverse_branch_opcodes (L_Oper * oper1, L_Oper * oper2)
{
  ITuint8 cond1, cond2, ctype1, ctype2;

  if (!oper1 || !oper2)
    return 0;

  if ((oper1->opc != Lop_BR) && (oper1->opc != Lop_BR_F))
    L_punt ("L_are_reverse_branch_opcodes: non-branch argument op %d opc %d",
	    oper1->id, oper1->opc);
  if (oper2->opc != oper1->opc)
    return 0;

  ctype1 = oper1->com[0];
  ctype2 = oper2->com[0];
  cond1 = oper1->com[1];
  cond2 = oper2->com[1];

  return ((cond1 == L_inverse_pred_completer (cond2) && (ctype1 == ctype2)));
}

int
L_are_same_branch_opcodes (L_Oper * oper1, L_Oper * oper2)
{
  ITuint8 cond1, cond2, ctype1, ctype2;

  if (!oper1 || !oper2)
    return 0;

  if ((oper1->opc != Lop_BR) && (oper1->opc != Lop_BR_F))
    L_punt ("L_are_same_branch_opcodes: non-branch argument op %d opc %d",
	    oper1->id, oper1->opc);
  if (oper2->opc != oper1->opc)
    return 0;

  ctype1 = oper1->com[0];
  ctype2 = oper2->com[0];
  cond1 = oper1->com[1];
  cond2 = oper2->com[1];

  return ((cond1 == cond2) && (ctype1 == ctype2));
}

int
L_load_store_sign_extend_conflict (L_Oper * op1, L_Oper * op2)
{
  int opc1, opc2;
  if (!op1 || !op2)
    return 0;
  opc1 = op1->opc;
  opc2 = op2->opc;
  switch (opc1)
    {
    case Lop_ST_C:
      return (opc2 == Lop_LD_UC);
    case Lop_ST_C2:
      return (opc2 == Lop_LD_UC2);
    case Lop_ST_I:
      return (opc2 == Lop_LD_UI);
    default:
      return 0;
    }
}

int
L_compatible_load_store (L_Oper * op1, L_Oper * op2)
{
  int opc1, opc2;
  if (!op1 || !op2)
    return 0;
  opc1 = op1->opc;
  opc2 = op2->opc;
  switch (opc1)
    {
    case Lop_LD_UC:
    case Lop_LD_C:
      return (opc2 == Lop_ST_C);
    case Lop_LD_UC2:
    case Lop_LD_C2:
      return (opc2 == Lop_ST_C2);
    case Lop_LD_UI:
    case Lop_LD_I:
      return (opc2 == Lop_ST_I);
    case Lop_LD_Q:
      return (opc2 == Lop_ST_Q);
    case Lop_LD_F:
      return (opc2 == Lop_ST_F);
    case Lop_LD_F2:
      return (opc2 == Lop_ST_F2);
    case Lop_ST_C:
      return ((opc2 == Lop_LD_UC) || (opc2 == Lop_LD_C));
    case Lop_ST_C2:
      return ((opc2 == Lop_LD_UC2) || (opc2 == Lop_LD_C2));
    case Lop_ST_I:
      return ((opc2 == Lop_LD_I) || (opc2 == Lop_LD_UI));
    case Lop_ST_Q:
      return (opc2 == Lop_LD_Q);
    case Lop_ST_F:
      return (opc2 == Lop_LD_F);
    case Lop_ST_F2:
      return (opc2 == Lop_LD_F2);
    case Lop_PRED_ST:
      return (opc2 == Lop_PRED_LD);
    default:
      return 0;
    }
}

int
L_cancelling_opcodes (L_Oper * opA, L_Oper * opB)
{
  if (!opA || !opB)
    return 0;
  if (L_int_add_opcode (opA) && L_int_sub_opcode (opB))
    return 1;
  if (L_int_sub_opcode (opA) && L_int_add_opcode (opB))
    return 1;
  return 0;
}

/*====================================================================*/
/*
 *      Predicate functions regarding operands
 */
/*====================================================================*/

/* 
 * DIA - The following function only works when int size = target
 * int size 
 */
int
L_is_legal_unsigned_value_offset_32 (unsigned int value, int offset)
{
  return 0;
#if 0
  Disabled, SAM 2 - 97, does not work if (offset < 0)
    return (value >= (unsigned) (-offset));

  return (((unsigned) (value + offset)) >= value);
#endif
}

int
L_is_int_24 (L_Operand * operand)
{
  if (!operand)
    return 0;
  return (L_is_int_constant (operand) && (operand->value.i == 24));
}

int
L_is_int_between_0_and_128 (L_Operand * operand)
{
  return (L_is_int_constant (operand) &&
	  (operand->value.i >= 0) && (operand->value.i <= 128));
}

int
L_is_branch_target (L_Oper * oper, L_Cb * cb)
{
  L_Cb *target;

  target = L_find_branch_dest (oper);

  return (target == cb);
}

int
L_can_change_all_uses_between (L_Cb * cb, L_Operand * operand,
			       L_Operand * replace, L_Oper * opA,
			       L_Oper * opB)
{
  int i, found;
  L_Oper *ptr, *redef;

  if (!operand || !replace || !opA)
    L_punt ("L_can_change_all_later_uses_with: arguments cannot be NIL");

  redef = NULL;
  found = 0;

  for (ptr = opA->next_op; ptr && (ptr != opB); ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (opA, ptr))
	continue;

      if (L_is_src_operand (operand, ptr) &&
	  (!L_can_change_src_operand (ptr, operand) ||
	   !PG_subset_predicate_ops (ptr, opA)))
	return 0;

      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (L_same_operand (ptr->dest[i], replace))
	    {
	      redef = ptr;
	      found = 1;
	      break;
	    }
	  if (L_same_operand (ptr->dest[i], operand) &&
	      PG_superset_predicate_ops (ptr, opA))
	    {
	      found = 1;
	      break;
	    }
	}
      if (found)
	break;
    }

  if (!redef)
    return 1;

  /* def<>0 then no uses of operand after def- halt search 
     if operand rdef'd */
  for (ptr = redef->next_op; ptr && (ptr != opB); ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (opA, ptr))
	continue;

      if (L_is_src_operand (operand, ptr))
	return 0;

      if (L_cond_branch_opcode (ptr) &&
	  L_in_oper_OUT_set (cb, ptr, operand, TAKEN_PATH))
	return 0;

      if (!PG_superset_predicate_ops (ptr, opA))
	continue;

      if (L_is_dest_operand (operand, ptr))
	return 1;
    }

  if (ptr != opB)
    L_punt ("L_can_change_all_uses_between: never hit opB!");

  return 1;
}

int
L_can_change_all_later_uses_with (L_Cb * cb, L_Operand * operand,
				  L_Operand * replace, L_Oper * op)
{
  return L_can_change_all_uses_between (cb, operand, replace, op, NULL);
}

/*====================================================================*/
/*
 *  Predicates regarding entire operations
 */
/*====================================================================*/

/*
 *      a=a+C, a=C+a, a=a-C (C is const) are valid increment operations
 */
int
L_increment_operation (L_Oper * oper)
{
  if (!(L_int_add_opcode (oper) || L_int_sub_opcode (oper)))
    return 0;

  if (L_is_numeric_constant (oper->src[0]) &&
      L_int_add_opcode (oper) && L_same_operand (oper->dest[0], oper->src[1]))
    return 1;
  else if (L_is_numeric_constant (oper->src[1]) &&
	   L_same_operand (oper->dest[0], oper->src[0]))
    return 1;

  return 0;
}

/*
 *      Ret 1 iff no write to same locn of op between opA and opB
 *      pathA and pathB add a check for control path for opA and opB, 
 *      respectively
 */
int
L_no_overlap_write (L_Cb * cb, L_Oper * op,
		    L_Oper * opA, int pathA, L_Oper * opB, int pathB)
{
  L_Oper *ptr;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);

  if (!(L_general_load_opcode (op) || L_general_store_opcode (op)))
    L_punt ("L_no_overlap_write: op not a memory op");

  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == opB)
	break;

      if (L_general_store_opcode (ptr) &&
	  (!L_independent_memory_ops (cb, op, ptr, dep_flags)) &&
	  ((!pathA || PG_intersecting_predicates_ops (opA, ptr)) ||
	   (!pathB || PG_intersecting_predicates_ops (ptr, opB))))
	return 0;
      /* JWS 19991019 - Check JSR info as well to fix bug in pred 
       * merging 
       */
      if (L_subroutine_call_opcode (ptr) &&
	  (!L_independent_memory_and_jsr (cb, op, ptr)) &&
	  ((!pathA || PG_intersecting_predicates_ops (opA, ptr)) ||
	   (!pathB || PG_intersecting_predicates_ops (ptr, opB))))
	return 0;
    }

  if (ptr != opB)
    L_punt ("L_no_overlap_write: opA does not reach opB");

  return 1;
}

/*
 *      Ret 1 iff no read to same locn of op between opA and opB
 *      pathA and pathB add a check for control path for opA and opB, 
 *      respectively
 */
int
L_no_overlap_read (L_Cb * cb, L_Oper * op,
		   L_Oper * opA, int pathA, L_Oper * opB, int pathB)
{
  L_Oper *ptr;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);

  if (!(L_general_load_opcode (op) || L_general_store_opcode (op)))
    L_punt ("L_no_overlap_read: op not memory op");

  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == opB)
	break;
      if (L_general_load_opcode (ptr) &&
	  (!L_independent_memory_ops (cb, op, ptr, dep_flags)) &&
	  ((!pathA || PG_intersecting_predicates_ops (opA, ptr)) ||
	   (!pathB || PG_intersecting_predicates_ops (ptr, opB))))
	return 0;
    }

  if (ptr != opB)
    L_punt ("L_no_overlap_read: opB not found");

  return 1;
}

int
L_no_sb_loop_br_between (L_Cb * cb, L_Oper * opA, L_Oper * opB)
{
  L_Oper *ptr;

  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == opB)
	break;
      if (!L_general_branch_opcode (ptr) && !L_check_branch_opcode (ptr))
	continue;
      if (L_find_branch_dest (ptr) == cb)
	return 0;
    }

  return 1;
}

/*
 * L_can_merge_with_op_above
 * ----------------------------------------------------------------------
 * Return 1 iff dependences would not be violated if merge_op were
 * to be merged into predecessor op.
 */

int
L_can_merge_with_op_above (L_Cb * cb, L_Oper * pred_op, L_Oper * merge_op)
{
  int i, j, is_load, is_store, is_branch, fragile_macro;
  L_Oper *int_op;
  L_Operand *dest, *src;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);

  if (!pred_op || !merge_op)
    L_punt ("L_can_merge_with_op_above: pred_op and merge_op cannot be NIL");

  if (L_subroutine_call_opcode (merge_op) ||
      L_subroutine_return_opcode (merge_op))
    return 0;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              store. */
  if (L_EXTRACT_BIT_VAL (merge_op->flags, L_OPER_VOLATILE))
    return 0;

  /* 04/20/03 SER Adding check: cannot merge a speculative and non-
   *              speculative op. */
  if (L_EXTRACT_BIT_VAL (pred_op->flags, L_OPER_SPECULATIVE))
    {
      if (!L_EXTRACT_BIT_VAL (merge_op->flags, L_OPER_SPECULATIVE))
        return 0;
    }
  else
    if (L_EXTRACT_BIT_VAL (merge_op->flags, L_OPER_SPECULATIVE))
      return 0;

  is_load = L_general_load_opcode (merge_op);
  is_store = L_general_store_opcode (merge_op);
  is_branch = (L_general_branch_opcode (merge_op) ||
	       L_check_branch_opcode (merge_op));
  fragile_macro = L_has_fragile_macro_operand (merge_op);

  for (int_op = pred_op->next_op; int_op && (int_op != merge_op);
       int_op = int_op->next_op)
    {
      if (!PG_intersecting_predicates_ops (int_op, merge_op))
	continue;

      /* Check for flow, output deps */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = int_op->dest[i]))
	    continue;
	  if (L_same_operand (dest, merge_op->pred[0]))
	    return 0;
	  for (j = 0; j < L_max_src_operand; j++)
	    if (L_same_operand (dest, merge_op->src[j]))
	      return 0;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (dest, merge_op->dest[j]))
	      return 0;
	}

      /* Check for anti deps */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!(src = int_op->src[i]))
	    continue;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (src, merge_op->dest[j]))
	      return 0;
	}

      /* Check for memory deps */

      if (((is_store && L_general_load_opcode (int_op)) ||
	   ((is_load || is_store) && L_general_store_opcode (int_op))) &&
	  !L_independent_memory_ops (cb, int_op, merge_op, dep_flags))
	return 0;

      /* Check for control, external flow deps */
      if (is_branch)
	{
	  if (L_general_branch_opcode (int_op) ||
	      L_check_branch_opcode (int_op) ||
	      L_subroutine_call_opcode (int_op))
	    return 0;

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = int_op->dest[i];
	      if (!(L_is_register (dest) || L_is_macro (dest)))
		continue;
	      if (L_in_oper_OUT_set (cb, merge_op, dest, TAKEN_PATH))
		return 0;
	    }
	}
      if (L_general_branch_opcode (int_op) || L_check_branch_opcode (int_op))
	{
	  if (!(L_safe_for_speculation (merge_op) || L_non_excepting_ops))
	    return 0;
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = merge_op->dest[i];
	      if (!(L_is_register (dest) || L_is_macro (dest)))
		continue;
	      if (L_in_oper_OUT_set (cb, int_op, dest, TAKEN_PATH))
		return 0;
	    }
	}

      if ((is_store) &&
	  (L_general_branch_opcode (int_op)
	   || L_check_branch_opcode (int_op)))
	return 0;

      /* check for macro deps on jsr's */
      if ((fragile_macro) && (L_subroutine_call_opcode (int_op)))
	return 0;

      /* check for memory deps with jsr's */
      if ((is_store || is_load) && L_subroutine_call_opcode (int_op) &&
	  !L_independent_memory_and_jsr (cb, merge_op, int_op))
	return 0;
    }

  if (!int_op)
    L_punt ("L_can_merge_with_op_above: merge_op not found");

  return 1;
}

/*
 * L_can_merge_with_op_below
 * ----------------------------------------------------------------------
 * Return 1 iff dependences would not be violated if merge_op were
 * to be merged into successor succ_op.
 */

int
L_can_merge_with_op_below (L_Cb * cb, L_Oper * succ_op, L_Oper * merge_op)
{
  int i, j, is_load, is_store, is_branch, fragile_macro;
  L_Oper *int_op;
  L_Operand *dest, *src;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              store. */
  if (L_EXTRACT_BIT_VAL (merge_op->flags, L_OPER_VOLATILE))
    return 0;

  /* 04/20/03 SER Adding check: cannot merge a speculative and non-
   *              speculative op. */
  if (L_EXTRACT_BIT_VAL (succ_op->flags, L_OPER_SPECULATIVE))
    {
      if (!L_EXTRACT_BIT_VAL (merge_op->flags, L_OPER_SPECULATIVE))
        return 0;
    }
  else
    if (L_EXTRACT_BIT_VAL (merge_op->flags, L_OPER_SPECULATIVE))
      return 0;

  if (!succ_op || !merge_op)
    L_punt ("L_can_merge_with_op_below: succ_op and merge_op cannot be NULL");

  if (L_subroutine_call_opcode (merge_op) ||
      L_subroutine_return_opcode (merge_op))
    return 0;

  is_load = L_general_load_opcode (merge_op);
  is_store = L_general_store_opcode (merge_op);
  is_branch = (L_general_branch_opcode (merge_op) ||
	       L_check_branch_opcode (merge_op));
  fragile_macro = L_has_fragile_macro_operand (merge_op);

  for (int_op = merge_op->next_op; int_op && (int_op != succ_op);
       int_op = int_op->next_op)
    {
      if (!PG_intersecting_predicates_ops (merge_op, int_op))
	continue;

      /* Check for flow / output deps */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = merge_op->dest[i]))
	    continue;
	  if (L_same_operand (dest, int_op->pred[0]))
	    return 0;
	  for (j = 0; j < L_max_src_operand; j++)
	    if (L_same_operand (dest, int_op->src[j]))
	      return 0;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (dest, int_op->dest[j]))
	      return 0;
	}

      /* Check for anti deps */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!(src = merge_op->src[i]))
	    continue;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (src, int_op->dest[j]))
	      return 0;
	}

      /* Check for memory deps */
      if (((is_store && L_general_load_opcode (int_op)) ||
	   ((is_load || is_store) && L_general_store_opcode (int_op))) &&
	  !L_independent_memory_ops (cb, int_op, merge_op, dep_flags))
	return 0;

      /* Check for control deps */
      if (is_branch)
	{
	  if (L_general_branch_opcode (int_op) ||
	      L_check_branch_opcode (int_op) ||
	      L_general_store_opcode (int_op) ||
	      L_subroutine_call_opcode (int_op))
	    return 0;
	  if (!(L_safe_for_speculation (int_op) || L_non_excepting_ops))
	    return 0;
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = int_op->dest[i];
	      if (!L_is_variable (dest))
		continue;
	      if (L_in_oper_OUT_set (cb, merge_op, dest, TAKEN_PATH))
		return 0;
	    }
	}

      if (L_general_branch_opcode (int_op) || L_check_branch_opcode (int_op))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = merge_op->dest[i];
	      if (!L_is_variable (dest))
		continue;
	      if (L_in_oper_OUT_set (cb, int_op, dest, TAKEN_PATH))
		return 0;
	    }
	}

      if (is_store &&
	  (L_general_branch_opcode (int_op)
	   || L_check_branch_opcode (int_op)))
	return 0;

      /* check for macro deps on jsr's */
      if (fragile_macro && L_subroutine_call_opcode (int_op))
	return 0;

      /* check for memory deps with jsr's */
      if ((is_store || is_load) && L_subroutine_call_opcode (int_op))
	{
	  if (!L_independent_memory_and_jsr (cb, merge_op, int_op) ||
	      L_memory_dependence_relation (merge_op, int_op))
	    return 0;
	}
    }

  if (int_op != succ_op)
    L_punt ("L_can_merge_with_op_below: op not found");

  return 1;
}

/*
 * Return 1 if can move tomove_op to immediately before op.
 * This is done if all the operations from op to tomove_op have no
 * dependencies to tomove_op.
 *
 * 10-94: added support for tomove_op = a branch
 */

int
L_can_move_above (L_Cb * cb, L_Oper * op, L_Oper * tomove_op)
{
  int i, j, is_load, is_store, is_branch, fragile_macro;
  L_Oper *ptr;
  L_Operand *dest, *src;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              store. */
  if (L_EXTRACT_BIT_VAL (tomove_op->flags, L_OPER_VOLATILE))
    return 0;

  if (!op || !tomove_op)
    L_punt ("L_can_move_above: op and tomove_op cannot be NIL");

  if (L_subroutine_call_opcode (tomove_op) ||
      L_subroutine_return_opcode (tomove_op))
    return 0;
#if 0
  if (!L_can_move_opcode (tomove_op))
    return 0;
#endif

  is_load = L_general_load_opcode (tomove_op);
  is_store = L_general_store_opcode (tomove_op);
  is_branch = (L_general_branch_opcode (tomove_op) ||
	       L_check_branch_opcode (tomove_op));
  fragile_macro = L_has_fragile_macro_operand (tomove_op);

  for (ptr = op; ptr && (ptr != tomove_op); ptr = ptr->next_op)
    {
      if (L_general_branch_opcode (ptr) || L_check_branch_opcode (ptr))
	{
	  if (!L_non_excepting_ops && is_load &&
	      !L_is_trivially_safe (tomove_op))
	    return 0;

	  if (!(L_safe_for_speculation (tomove_op) || L_non_excepting_ops))
	    return 0;
	}

      if (!PG_intersecting_predicates_ops (ptr, tomove_op))
	continue;

      /* Check for flow deps */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = ptr->dest[i]))
	    continue;
	  for (j = 0; j < L_max_src_operand; j++)
	    if (L_same_operand (dest, tomove_op->src[j]))
	      return 0;

	  if (tomove_op->pred[0] && L_same_operand (dest, tomove_op->pred[0]))
	    return 0;
	}

      /* Check for anti deps */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!(src = ptr->src[i]))
	    continue;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (src, tomove_op->dest[j]))
	      return 0;
	}

      if (L_is_control_oper (ptr))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!(dest = tomove_op->dest[i]))
		continue;
	      if (!(L_is_register (dest) || L_is_macro (dest)))
		continue;
	      if (L_in_oper_OUT_set (cb, ptr, dest, TAKEN_PATH))
		return 0;
	    }
	}

      /* Check for output deps */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = ptr->dest[i]))
	    continue;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (dest, tomove_op->dest[j]))
	      return 0;
	}

      /* Check for memory deps */
      if ((is_load) && (L_general_store_opcode (ptr)) &&
	  (!L_independent_memory_ops (cb, ptr, tomove_op, dep_flags)))
	return 0;

      if ((is_store) && (L_general_load_opcode (ptr)) &&
	  (!L_independent_memory_ops (cb, ptr, tomove_op, dep_flags)))
	return 0;

      if ((is_store) && (L_general_store_opcode (ptr)) &&
	  (!L_independent_memory_ops (cb, ptr, tomove_op, dep_flags)))
	return 0;

      /* Check for control deps */
      if (is_branch)
	{
	  if (L_general_store_opcode (ptr) ||
	      L_general_branch_opcode (ptr) ||
	      L_check_branch_opcode (ptr) || L_subroutine_call_opcode (ptr))
	    return 0;

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!(dest = ptr->dest[i]))
		continue;
	      if (!(L_is_register (dest) || L_is_macro (dest)))
		continue;
	      if (L_in_oper_OUT_set (cb, tomove_op, dest, TAKEN_PATH))
		return 0;
	    }
	}

      if (is_store && (L_general_branch_opcode (ptr) ||
		       L_check_branch_opcode (ptr)))
	return 0;

      /* check for macro deps on jsr's */
      if (fragile_macro && (L_subroutine_call_opcode (ptr)))
	return 0;

      /* check for memory deps with jsr's */
      if ((is_store || is_load) && L_subroutine_call_opcode (ptr))
	{
	  if (!L_independent_memory_and_jsr (cb, tomove_op, ptr) ||
	      L_memory_dependence_relation (tomove_op, ptr))
	    return 0;
	}
    }

  if (!ptr)
    L_punt ("L_can_move_above: tomove_op not found");

  return 1;
}

/*
 *      Return 1 if can move tomove_op below op
 */
int
L_can_move_below (L_Cb * cb, L_Oper * op, L_Oper * tomove_op)
{
  int i, j, is_load, is_store, is_branch, fragile_macro;
  L_Oper *ptr;
  L_Operand *dest, *src;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              store. */
  if (L_EXTRACT_BIT_VAL (tomove_op->flags, L_OPER_VOLATILE))
    return 0;

  if (!op || !tomove_op)
    L_punt ("L_can_move_below: op and tomove_op cannot be NIL");

  if (L_subroutine_call_opcode (tomove_op) ||
      L_subroutine_return_opcode (tomove_op))
    return 0;

#if 0
  if (!L_can_move_opcode (tomove_op))
    return 0;
#endif

  is_load = L_general_load_opcode (tomove_op);
  is_store = L_general_store_opcode (tomove_op);
  is_branch = (L_general_branch_opcode (tomove_op) ||
	       L_check_branch_opcode (tomove_op));
  fragile_macro = L_has_fragile_macro_operand (tomove_op);

  for (ptr = tomove_op->next_op; ptr && (ptr != op->next_op);
       ptr = ptr->next_op)
    {
      if (!(L_non_excepting_ops) && (is_load) &&
	  (!(L_is_trivially_safe (tomove_op))))
	return 0;

      if (is_branch && !(L_safe_for_speculation (ptr) || L_non_excepting_ops))
	return 0;

      if (!PG_intersecting_predicates_ops (tomove_op, ptr))
	continue;

      /* Check for flow deps */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = tomove_op->dest[i]))
	    continue;

	  for (j = 0; j < L_max_src_operand; j++)
	    if (L_same_operand (dest, ptr->src[j]))
	      return 0;

	  if (ptr->pred[0])
	    if (L_same_operand (dest, ptr->pred[0]))
	      return 0;
	}

      /* Check for anti deps */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!(src = tomove_op->src[i]))
	    continue;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (src, ptr->dest[j]))
	      return 0;
	}

      /* Check for output deps */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = tomove_op->dest[i]))
	    continue;
	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_same_operand (dest, ptr->dest[j]))
	      return 0;
	}

      /* Check for memory deps */
      if (is_load && L_general_store_opcode (ptr) &&
	  !L_independent_memory_ops (cb, tomove_op, ptr, dep_flags))
	return 0;

      if (is_store && L_general_load_opcode (ptr) &&
	  !L_independent_memory_ops (cb, tomove_op, ptr, dep_flags))
	return 0;

      if (is_store && L_general_store_opcode (ptr) &&
	  !L_independent_memory_ops (cb, tomove_op, ptr, dep_flags))
	return 0;

      /* Check for control deps */
      if (is_branch)
	{
	  if (L_general_branch_opcode (ptr) ||
	      L_check_branch_opcode (ptr) || L_subroutine_call_opcode (ptr))
	    return 0;

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = ptr->dest[i];
	      if (!(L_is_register (dest) || L_is_macro (dest)))
		continue;
	      if (L_in_oper_OUT_set (cb, tomove_op, dest, TAKEN_PATH))
		return 0;
	    }
	}

      if (L_is_control_oper (ptr))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = tomove_op->dest[i];
	      if (!(L_is_register (dest) || L_is_macro (dest)))
		continue;
	      if (L_in_oper_OUT_set (cb, ptr, dest, TAKEN_PATH))
		return 0;
	    }
	}

      if (is_store && (L_general_branch_opcode (ptr) ||
		       L_check_branch_opcode (ptr)))
	return 0;

      /* check for macro deps on jsr's */
      if (fragile_macro && L_subroutine_call_opcode (ptr))
	return 0;

      /* check for memory deps with jsr's */
      if (((is_store) || (is_load)) && (L_subroutine_call_opcode (ptr)))
	{
	  if (!L_independent_memory_and_jsr (cb, tomove_op, ptr))
	    return 0;
	  if (L_memory_dependence_relation (tomove_op, ptr))
	    return 0;
	}
    }

  if (ptr != op->next_op)
    L_punt ("L_can_move_below: op not found");

  return 1;
}

int
L_can_undo_and_combine (L_Oper * op1, L_Oper * op2, L_Oper * op3)
{
  ITintmax base, offset;

  if (!op1 || !op2 || !op3)
    L_punt ("L_can_undo_and_combine: op1, op2, op3 cannot be NIL");

  if (!L_move_opcode (op1))
    return 0;
  if (!L_increment_operation (op2))
    return 0;
  /* SAM 2-97, add pred comparison to list of suitable opBs */
  if (!(L_load_opcode (op3) || L_store_opcode (op3) ||
	L_cond_branch_opcode (op3) || L_int_comparison_opcode (op3) ||
	L_int_pred_comparison_opcode (op3) ||
	L_int_add_opcode (op3) || L_int_sub_opcode (op3)))
    return 0;
  if (!(L_same_operand (op1->dest[0], op3->src[0]) ||
	L_same_operand (op1->dest[0], op3->src[1])))
    return 0;
  if (!(L_is_int_constant (op3->src[0]) || L_is_int_constant (op3->src[1])))
    return 0;
  /* SAM: 2-97, additional check required for unsigned comparison
     constant arithmetic */
  if (L_unsigned_int_comparative_opcode (op3))
    {
      if (L_is_int_constant (op2->src[0]))
	offset = op2->src[0]->value.i;
      else
	offset = op2->src[1]->value.i;
      if (L_int_add_opcode (op2))
	offset = -offset;

      if (L_is_int_constant (op3->src[0]))
	base = op3->src[0]->value.i;
      else
	base = op3->src[1]->value.i;

      if (!L_is_legal_unsigned_value_offset_32 ((unsigned int) base,
						(int) -offset))
	return 0;
    }

  if (!L_no_defs_between (op2->dest[0], op2, op3))
    return 0;
  return 1;
}

int
L_can_undo_and_combine2 (L_Oper * opA, L_Oper * opB)
{
  ITintmax s1_A_const, s2_A_const, s1_B_const, s2_B_const, base, offset;

  if (!opA || !opB)
    L_punt ("L_can_undo_and_combine2: opA, opB cannot be NIL");

  if (!L_int_add_opcode (opA))
    return 0;
  /* SAM 2-97, add pred comparison to list of suitable opBs */
  if (!(L_load_opcode (opB) || L_store_opcode (opB) ||
	L_cond_branch_opcode (opB) || L_int_comparison_opcode (opB) ||
	L_int_pred_comparison_opcode (opB) ||
	L_int_add_opcode (opB) || L_int_sub_opcode (opB)))
    return 0;
  s1_A_const = L_is_int_constant (opA->src[0]);
  s2_A_const = L_is_int_constant (opA->src[1]);
  s1_B_const = L_is_int_constant (opB->src[0]);
  s2_B_const = L_is_int_constant (opB->src[1]);
  if (!(s1_A_const | s2_A_const))
    return 0;
  if (!(s1_B_const | s2_B_const))
    return 0;
  if (s1_A_const)
    {
      if (s1_B_const)
	{
	  if (!L_same_operand (opA->src[1], opB->src[1]))
	    return 0;
	}
      else
	{
	  if (!L_same_operand (opA->src[1], opB->src[0]))
	    return 0;
	}
    }
  else
    {
      if (s1_B_const)
	{
	  if (!L_same_operand (opA->src[0], opB->src[1]))
	    return 0;
	}
      else
	{
	  if (!L_same_operand (opA->src[0], opB->src[0]))
	    return 0;
	}
    }
  /* SAM: 2-97, additional check required for unsigned comparison
     constant arithmetic */
  if (L_unsigned_int_comparative_opcode (opB))
    {
      if (s1_A_const)
	offset = -(opA->src[0]->value.i);	/* minus because opA is add */
      else
	offset = -(opA->src[1]->value.i);

      if (s1_B_const)
	base = opB->src[0]->value.i;
      else
	base = opB->src[1]->value.i;

      if (!L_is_legal_unsigned_value_offset_32
	  ((unsigned int) base, (int) -offset))
	return 0;
    }

  return 1;
}

int
L_can_make_mul_add_sub (L_Oper * opA, L_Oper * opB)
{
  int opc;

  if (!L_mul_opcode (opA))
    {
      L_punt ("L_can_make_mul_add_sub: opA must be mul op");
      return (-1);
    }

  if (L_add_opcode (opB))
    {
      opc = L_corresponding_mul_add (opB);
    }
  else if (L_sub_opcode (opB))
    {
      if (L_same_operand (opA->dest[0], opB->src[0]))
	opc = L_corresponding_mul_sub (opB);
      else if (L_same_operand (opA->dest[0], opB->src[1]))
	opc = L_corresponding_mul_sub_rev (opB);
      else
	{
	  L_punt ("L_can_make_mul_add_sub: no use of opA dest by opB");
	  return (-1);
	}
    }
  else
    {
      L_punt ("L_can_make_mul_add_sub: opB must be add or sub");
      return (-1);
    }

  return (M_oper_supported_in_arch (opc));
}

/*
 *      SAM 2-97: check whether an op can have an increment undone, this
 *      is really only an issue with an unsigned comparative opcode where
 *      the possibility of wrapping about 0 could occur, so check that.
 */
int
L_can_undo_increment (L_Oper * opA, L_Oper * opB)
{
  ITintmax base = 0, offset = 0;

  /* Assume for signed arithmetic, any undoing is cool */
  if (!L_unsigned_int_comparative_opcode (opB))
    return 1;

  if (L_is_int_constant (opA->src[0]))
    offset = opA->src[0]->value.i;
  else if (L_is_int_constant (opA->src[1]))
    offset = opA->src[1]->value.i;
  else
    L_punt ("L_can_undo_increment: opA needs a constant src operand!");
  if (L_int_sub_opcode (opA))
    offset = -offset;

  if (L_is_int_constant (opB->src[0]))
    base = opB->src[0]->value.i;
  else if (L_is_int_constant (opB->src[1]))
    base = opB->src[1]->value.i;
  else
    L_punt ("L_can_undo_increment: opB needs a constant src operand!");

  if (L_is_legal_unsigned_value_offset_32
      ((unsigned int) base, (int) -offset))
    return 1;
  else
    return 0;
}

/*====================================================================*/
/*
 *  Predicates regarding evaluating operations with constant operands
 */
/*====================================================================*/

/*
 *      Currently the only exception checked for is divide by 0.
 *      This needs to be expanded.
 */
int
L_no_exceptions (L_Oper * oper)
{
  if (!oper)
    return 1;
  switch (oper->opc)
    {
    case Lop_ADD:
    case Lop_ADD_U:
    case Lop_SUB:
    case Lop_SUB_U:
    case Lop_ADD_CARRY:
    case Lop_ADD_CARRY_U:
    case Lop_SUB_CARRY:
    case Lop_SUB_CARRY_U:
    case Lop_MUL_WIDE:
    case Lop_MUL_WIDE_U:
    case Lop_L_MAC:
    case Lop_L_MSU:
    case Lop_ADD_SAT:
    case Lop_ADD_SAT_U:
    case Lop_SUB_SAT:
    case Lop_SUB_SAT_U:
    case Lop_MUL_SAT:
    case Lop_MUL_SAT_U:
    case Lop_SAT:
    case Lop_SAT_U:
    case Lop_MUL:
    case Lop_MUL_U:
    case Lop_MUL_ADD:
    case Lop_MUL_ADD_U:
    case Lop_MUL_SUB:
    case Lop_MUL_SUB_U:
    case Lop_MUL_SUB_REV:
    case Lop_MUL_SUB_REV_U:
      return 1;
    case Lop_DIV:
      {
	ITintmax s2;
	if (!L_is_int_constant (oper->src[1]))
	  L_punt ("L_no_exceptions DIV:  src2 is not an integer");
	s2 = oper->src[1]->value.i;
	return (s2 != 0);
      }
    case Lop_DIV_U:
      {
	ITintmax s2;
	if (!L_is_int_constant (oper->src[1]))
	  L_punt ("L_no_exceptions DIVU:  src2 is not an integer");
	s2 = oper->src[1]->value.i;
	return (s2 != 0);
      }
    case Lop_REM:
      {
	ITintmax s2;
	if (!L_is_int_constant (oper->src[1]))
	  L_punt ("L_no_exceptions REM:  src2 is not an integer");
	s2 = oper->src[1]->value.i;
	return (s2 != 0);
      }
    case Lop_REM_U:
      {
	ITintmax s2;
	if (!L_is_int_constant (oper->src[1]))
	  L_punt ("L_no_exceptions REMU:  src2 is not an integer");
	s2 = oper->src[1]->value.i;
	return (s2 != 0);
      }
    case Lop_ABS:
    case Lop_OR:
    case Lop_AND:
    case Lop_XOR:
    case Lop_NOR:
    case Lop_NAND:
    case Lop_NXOR:
    case Lop_OR_NOT:
    case Lop_AND_NOT:
    case Lop_OR_COMPL:
    case Lop_AND_COMPL:
    case Lop_RCMP:
    case Lop_RCMP_F:
      return 1;
    case Lop_LSL:
    case Lop_LSR:
    case Lop_ASR:
      return 1;
    case Lop_ADD_F2:
    case Lop_SUB_F2:
    case Lop_MUL_F2:
    case Lop_MUL_ADD_F2:
    case Lop_MUL_SUB_F2:
    case Lop_MUL_SUB_REV_F2:
      return 1;
    case Lop_DIV_F2:
      {
	double s2;
	if (!L_is_dbl_constant (oper->src[1]))
	  L_punt ("L_no_exceptions DIV_F2:  src2 is not a double");
	s2 = oper->src[1]->value.f2;
	return (s2 != 0.0);
      }
    case Lop_ADD_F:
    case Lop_SUB_F:
    case Lop_MUL_F:
    case Lop_MUL_ADD_F:
    case Lop_MUL_SUB_F:
    case Lop_MUL_SUB_REV_F:
      return 1;
    case Lop_DIV_F:
      {
	float s2;
	if (!L_is_flt_constant (oper->src[1]))
	  L_punt ("L_no_exceptions DIV_F2:  src2 is not a float");
	s2 = oper->src[1]->value.f;
	return (s2 != 0.0);
      }
    case Lop_ABS_F:
    case Lop_ABS_F2:
    case Lop_F2_I:
    case Lop_I_F2:
    case Lop_F2_F:
    case Lop_F_F2:
    case Lop_F_I:
    case Lop_I_F:
    case Lop_SQRT_F2:
    case Lop_MIN_F2:
    case Lop_MAX_F2:
      return 1;
    case Lop_SXT_C:
    case Lop_ZXT_C:
    case Lop_SXT_C2:
    case Lop_ZXT_C2:
    case Lop_SXT_I:
    case Lop_ZXT_I:
    case Lop_EXTRACT:
    case Lop_EXTRACT_U:
    case Lop_EXTRACT_C:
    case Lop_EXTRACT_C2:
    case Lop_DEPOSIT:
      return 1;
    case Lop_MEM_COPY_SETUP:	/* cannot eval this at compile time!! */
      return 0;
    default:
      L_print_oper (stderr, oper);
      L_punt ("L_no_exceptions: illegal opcode passed into");
      return 0;
    }
}

/*
 *      Now handle both 32-bit and 64-bit constants.  Previously, we assumed
 *      that only 64-bit constants would be encountered.  This is not the
 *      case in ANSI-C though.
 */

typedef union
{
  double dval;
  struct
  {
    int h1, h2;
  }
  hval;
}
convert_union;

typedef union
{
  float fval;
  int ival;
}
convert_float_union;

int
L_will_lose_accuracy (L_Oper * oper)
{
  double ds1, ds2, ds3, dres, dres2;
  convert_union c1, c2;
  float fs1, fs2, fs3, fres, fres2;
  convert_float_union cf1, cf2;
  char str[128];

  if (!oper)
    return 0;

  switch (oper->opc)
    {
    case Lop_ADD_F2:
      ds1 = oper->src[0]->value.f2;
      ds2 = oper->src[1]->value.f2;
      dres = ds1 + ds2;
      c1.dval = dres;
      sprintf (str, "%1.16e", dres);
      sscanf (str, "%lf", &dres2);
      c2.dval = dres2;
      if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
	return 1;
      else
	return 0;
    case Lop_SUB_F2:
      ds1 = oper->src[0]->value.f2;
      ds2 = oper->src[1]->value.f2;
      dres = ds1 - ds2;
      c1.dval = dres;
      sprintf (str, "%1.16e", dres);
      sscanf (str, "%lf", &dres2);
      c2.dval = dres2;
      if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
	return 1;
      else
	return 0;
    case Lop_MUL_F2:
      ds1 = oper->src[0]->value.f2;
      ds2 = oper->src[1]->value.f2;
      dres = ds1 * ds2;
      c1.dval = dres;
      sprintf (str, "%1.16e", dres);
      sscanf (str, "%lf", &dres2);
      c2.dval = dres2;
      if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
	return 1;
      else
	return 0;
    case Lop_DIV_F2:
      ds1 = oper->src[0]->value.f2;
      ds2 = oper->src[1]->value.f2;
      dres = ds1 / ds2;
      c1.dval = dres;
      sprintf (str, "%1.16e", dres);
      sscanf (str, "%lf", &dres2);
      c2.dval = dres2;
      if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
	return 1;
      else
	return 0;
    case Lop_MUL_ADD_F2:
      ds1 = oper->src[0]->value.f2;
      ds2 = oper->src[1]->value.f2;
      ds3 = oper->src[2]->value.f2;
      dres = ds1 * ds2 + ds3;
      c1.dval = dres;
      sprintf (str, "%1.16e", dres);
      sscanf (str, "%lf", &dres2);
      c2.dval = dres2;
      if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
	return 1;
      else
	return 0;
    case Lop_MUL_SUB_F2:
      ds1 = oper->src[0]->value.f2;
      ds2 = oper->src[1]->value.f2;
      ds3 = oper->src[2]->value.f2;
      dres = ds1 * ds2 - ds3;
      c1.dval = dres;
      sprintf (str, "%1.16e", dres);
      sscanf (str, "%lf", &dres2);
      c2.dval = dres2;
      if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
	return 1;
      else
	return 0;
    case Lop_MUL_SUB_REV_F2:
      ds1 = oper->src[0]->value.f2;
      ds2 = oper->src[1]->value.f2;
      ds3 = oper->src[2]->value.f2;
      dres = -(ds1 * ds2) + ds3;
      c1.dval = dres;
      sprintf (str, "%1.16e", dres);
      sscanf (str, "%lf", &dres2);
      c2.dval = dres2;
      if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
	return 1;
      else
	return 0;
    case Lop_ADD_F:
      fs1 = oper->src[0]->value.f;
      fs2 = oper->src[1]->value.f;
      fres = fs1 + fs2;
      cf1.fval = fres;
      sprintf (str, "%1.8e", fres);
      sscanf (str, "%f", &fres2);
      cf2.fval = fres2;
      if (cf1.ival != cf2.ival)
	return 1;
      else
	return 0;
    case Lop_SUB_F:
      fs1 = oper->src[0]->value.f;
      fs2 = oper->src[1]->value.f;
      fres = fs1 - fs2;
      cf1.fval = fres;
      sprintf (str, "%1.8e", fres);
      sscanf (str, "%f", &fres2);
      cf2.fval = fres2;
      if (cf1.ival != cf2.ival)
	return 1;
      else
	return 0;
    case Lop_MUL_F:
      fs1 = oper->src[0]->value.f;
      fs2 = oper->src[1]->value.f;
      fres = fs1 * fs2;
      cf1.fval = fres;
      sprintf (str, "%1.8e", fres);
      sscanf (str, "%f", &fres2);
      cf2.fval = fres2;
      if (cf1.ival != cf2.ival)
	return 1;
      else
	return 0;
    case Lop_DIV_F:
      fs1 = oper->src[0]->value.f;
      fs2 = oper->src[1]->value.f;
      fres = fs1 / fs2;
      cf1.fval = fres;
      sprintf (str, "%1.8e", fres);
      sscanf (str, "%f", &fres2);
      cf2.fval = fres2;
      if (cf1.ival != cf2.ival)
	return 1;
      else
	return 0;
    case Lop_MUL_ADD_F:
      fs1 = oper->src[0]->value.f;
      fs2 = oper->src[1]->value.f;
      fs3 = oper->src[2]->value.f;
      fres = fs1 * fs2 + fs3;
      cf1.fval = fres;
      sprintf (str, "%1.8e", fres);
      sscanf (str, "%f", &fres2);
      cf2.fval = fres2;
      if (cf1.ival != cf2.ival)
	return 1;
      else
	return 0;
    case Lop_MUL_SUB_F:
      fs1 = oper->src[0]->value.f;
      fs2 = oper->src[1]->value.f;
      fs3 = oper->src[2]->value.f;
      fres = fs1 * fs2 - fs3;
      cf1.fval = fres;
      sprintf (str, "%1.8e", fres);
      sscanf (str, "%f", &fres2);
      cf2.fval = fres2;
      if (cf1.ival != cf2.ival)
	return 1;
      else
	return 0;
    case Lop_MUL_SUB_REV_F:
      fs1 = oper->src[0]->value.f;
      fs2 = oper->src[1]->value.f;
      fs3 = oper->src[2]->value.f;
      fres = -(fs1 * fs2) + fs3;
      cf1.fval = fres;
      sprintf (str, "%1.8e", fres);
      sscanf (str, "%f", &fres2);
      cf2.fval = fres2;
      if (cf1.ival != cf2.ival)
	return 1;
      else
	return 0;
    case Lop_ADD_CARRY:
    case Lop_ADD_CARRY_U:
    case Lop_SUB_CARRY:
    case Lop_SUB_CARRY_U:
    case Lop_MUL_WIDE:
    case Lop_MUL_WIDE_U:
      return 1;
    case Lop_ADD:
    case Lop_ADD_U:
    case Lop_SUB:
    case Lop_SUB_U:
      if(L_find_attr(oper->attr, "do_not_constant_fold"))
        return 1;
    case Lop_ABS_F2:
    case Lop_ABS_F:
    case Lop_SQRT_F2:
    case Lop_MIN_F2:
    case Lop_MAX_F2:
    case Lop_I_F2:
    case Lop_F_F2:
    case Lop_F2_F:
    case Lop_I_F:
      return 0;
    default:
      if (FLT_ARITH_OPCODE (oper->opc) || DBL_ARITH_OPCODE (oper->opc))
	{
	  fprintf (stderr, "Problem opc = %d\n", oper->opc);
	  L_punt ("ERROR: L_will_lose_accuracy needs to have the "
		  "previous opc defined for it.\n");
	}
      return 0;
    }
}

int
L_invertible_float_constant (float val)
{
  float inv, t;
  convert_float_union c1, c2;
  char str[128];

  inv = 1 / val;
  c1.fval = inv;
  sprintf (str, "%1.7e", inv);
  sscanf (str, "%f", &t);
  c2.fval = t;
  if (c1.ival != c2.ival)
    return 0;
  else
    return 1;
}

int
L_invertible_constant (double val)
{
  double inv, t;
  convert_union c1, c2;
  char str[128];

  inv = 1 / val;
  c1.dval = inv;
  sprintf (str, "%1.15e", inv);
  sscanf (str, "%lf", &t);
  c2.dval = t;
  if ((c1.hval.h1 != c2.hval.h1) || (c1.hval.h2 != c2.hval.h2))
    return 0;
  else
    return 1;
}

/*
 *      return 1 if operand appears in OUT set of br subsequent to oper in cb
 */
int
L_live_outside_cb_after (L_Cb * cb, L_Operand * operand, L_Oper * oper)
{
  int i;
  L_Oper *ptr;

  if (!operand)
    return 1;

  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (oper, ptr))
	continue;

      /* if ptr is branch, check if operand in live-out set */
      if (L_general_branch_opcode (ptr) || L_check_branch_opcode (ptr))
	{
	  if (L_in_oper_OUT_set (cb, ptr, operand, TAKEN_PATH))
	    return 1;
	}
      /* otherwise, check if operand redefined, if so, stop search */
      else if (PG_superset_predicate_ops (ptr, oper))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    if (L_same_operand (operand, ptr->dest[i]))
	      return 0;
	}
    }
  /* check live-out of fallthru path if it exists,
     be conservative, don't check predicate for now */
  /* SER: Not that this assumes that a cb with no branch or check at the end
     has a fallthru path. This is not the case if it is a return. The assumption
     is that there are no optis that would be triggered in this case, so be
     careful not to move optimizable instructions into a "final" cb. */
  if ((!L_general_branch_opcode (cb->last_op)) &&
      (!L_check_branch_opcode (cb->last_op)))
    if (!(cb->next_cb))
      return 0;
    else
      return (L_in_cb_IN_set (cb->next_cb, operand));
  else if ((L_cond_branch_opcode (cb->last_op)) &&
	   (L_in_cb_IN_set (cb->next_cb, operand)))
    return 1;
  else if ((L_uncond_branch_opcode (cb->last_op)) &&
	   (L_is_predicated (cb->last_op)) &&
	   (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
	   && (L_in_cb_IN_set (cb->next_cb, operand)))
    return 1;

  return 0;
}

/*
 * return 1 if operand does not appear in OUT set of a br between start and end
 */
int
L_not_live_outside_cb_between (L_Cb * cb, L_Operand * operand, L_Oper * start,
			       L_Oper * end)
{
  int i;
  L_Oper *ptr;

  if (!operand)
    return 1;

  for (ptr = start->next_op; ptr != end; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (start, ptr))
	continue;

      /* if ptr is branch, check if operand in live-out set */
      if (L_general_branch_opcode (ptr) || L_check_branch_opcode (ptr))
	{
	  if (L_in_oper_OUT_set (cb, ptr, operand, TAKEN_PATH))
	    return 0;
	}
      /* otherwise, check if operand redefined, if so, stop search */
      else if (PG_superset_predicate_ops (ptr, start))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    if (L_same_operand (operand, ptr->dest[i]))
	      return 1;
	}
    }
  return 1;
}

int
L_all_dest_operand_not_live_outside_cb_between (L_Cb * cb, L_Oper * oper,
						L_Oper * start, L_Oper * end)
{
  int i;

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_not_live_outside_cb_between (cb, oper->dest[i], start, end))
	return 0;
    }

  return 1;
}

int
L_all_src_operand_not_live_outside_cb_between (L_Cb * cb, L_Oper * oper,
					       L_Oper * start, L_Oper * end)
{
  int i;

  for (i = 0; i < L_max_src_operand; i++)
    if (!L_not_live_outside_cb_between (cb, oper->src[i], start, end))
      return 0;

  return 1;
}


/*
 *  Single use of a defn, and the use is in the same cb, so
 *  there should be exactly 1 flow dep from op and operand should not
 *  be in the out set.
 */
int
L_single_use_of (L_Cb * cb, L_Operand * operand, L_Oper * oper)
{
  int i, num_use;
  L_Oper *ptr, *redef;

  if (!operand)
    return 0;

  /*
   *  1. there should be exactly one use in the block.
   */

  redef = NULL;
  num_use = 0;

  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (oper, ptr))
	continue;

      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (operand, ptr->src[i]))
	    num_use++;
	  if (num_use == 2)
	    return 0;
	}

      if (!PG_superset_predicate_ops (ptr, oper))
	continue;

      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (operand, ptr->dest[i]))
	  break;

      if (i < L_max_dest_operand)
	{
	  redef = ptr;
	  break;
	}
    }

  if (num_use != 1)
    return 0;
  /*
   *  2. it should not in the any branch OUT set subsequent to oper
   */
  return (!L_live_outside_cb_after (cb, operand, oper));
}

int
L_no_use_of (L_Cb * cb, L_Operand * operand, L_Oper * oper)
{
  int i;
  L_Oper *ptr, *redef;

  if (!operand)
    return 1;

  /* check for use of operand in this cb */

  redef = NULL;
  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (oper, ptr))
	continue;

      for (i = 0; i < L_max_src_operand; i++)
	if (L_same_operand (operand, ptr->src[i]))
	  return 0;

      if (!PG_superset_predicate_ops (ptr, oper))
	continue;

      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (operand, ptr->dest[i]))
	  break;

      if (i < L_max_dest_operand)
	{
	  redef = ptr;
	  break;
	}
    }

  /* if not redefined, check out set of br's subsequent to oper */
  return (!L_live_outside_cb_after (cb, operand, oper));
}

int
L_is_redefined_in_cb_after (L_Oper * oper, L_Operand * operand)
{
  int i;
  L_Oper *ptr;

  if (!oper)
    L_punt ("L_is_redefined_in_cb_after: op cannot be NIL");

  if (!operand || !L_is_register (operand))
    return 0;

  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    if (PG_intersecting_predicates_ops (oper, ptr))
      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (ptr->dest[i], operand))
	  return 1;

  return 0;
}

int
L_all_uses_can_be_renamed (L_Cb * cb, L_Oper * oper, L_Operand * operand)
{
  int j;
  L_Oper *ptr, *last_def;

  last_def = oper;
  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (oper, ptr))
	continue;

      /* check if can change matching srcs of ptr */

      for (j = 0; j < L_max_src_operand; j++)
	{
	  if (!L_same_operand (operand, ptr->src[j]))
	    continue;
	  if (!PG_superset_predicate_ops (last_def, ptr))
	    return 0;
	  if (!L_can_change_src_operand (ptr, ptr->src[j]))
	    return 0;
	}

      /* check for redef */

      for (j = 0; j < L_max_dest_operand; j++)
	{
	  if (!L_same_operand (operand, ptr->dest[j]))
	    continue;
	  if (PG_superset_predicate_ops (ptr, oper))
	    return 1;
	  else
	    last_def = ptr;
	}
    }
  return 1;
}

int
L_not_live_at_cb_end (L_Cb * cb, L_Oper * oper, L_Operand * operand)
{
  int i;
  L_Oper *ptr, *last;

  if (!operand)
    return 1;

  /* if operand redefined, this predicate automatically true */

  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_superset_predicate_ops (ptr, oper))
	continue;

      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (operand, ptr->dest[i]))
	  return 1;
    }

  /* otherwise check out set of last op in cb */

  last = cb->last_op;
  if (L_cond_branch_opcode (last))
    {
      if (L_in_oper_OUT_set (cb, last, operand, TAKEN_PATH) ||
	  ((cb->next_cb) && L_in_cb_IN_set (cb->next_cb, operand)))
	return 0;
      else
	return 1;
    }
  else if (L_uncond_branch_opcode (last))
    {
      if (L_is_predicated (last) &&
	  (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU)) &&
	  (L_in_oper_OUT_set (cb, cb->last_op, operand, TAKEN_PATH) ||
	   L_in_cb_IN_set (cb->next_cb, operand)))
	return 0;

      if (L_cond_branch_opcode (last->prev_op) &&
	  (L_in_oper_OUT_set (cb, last->prev_op, operand, TAKEN_PATH) ||
	   L_in_oper_OUT_set (cb, last, operand, TAKEN_PATH)))
	return 0;

      if (L_in_oper_OUT_set (cb, last, operand, TAKEN_PATH))
	return 0;

      return 1;
    }
  else if (L_register_branch_opcode (last))
    {
      if (L_in_oper_OUT_set (cb, last, operand, TAKEN_PATH))
	return 0;
      else
	return 1;
    }
  else
    {
      /*
       *      Use cb for now to be conservative, problem is
       *      last instr may be deleted!!!, then u lose the varis
       *       which it defined.
       */
      if (!cb->next_cb)
	return 1;
      else if (L_in_cb_IN_set (cb->next_cb, operand))
	return 0;
      else
	return 1;
    }
}

/*
 *      operand must be in oper->dest[]
 */
int
L_no_flow_dep_from_for (L_Cb * cb, L_Oper * oper, L_Operand * operand)
{
  int i;
  L_Oper *ptr;

  if (!operand)
    return 1;

  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (oper, ptr))
	continue;

      for (i = 0; i < L_max_src_operand; i++)
	if (L_same_operand (operand, ptr->src[i]))
	  return 0;

      if (!PG_superset_predicate_ops (ptr, oper))
	continue;

      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (operand, ptr->dest[i]))
	  return 1;
    }
  return 1;
}

int
L_no_flow_dep_from (L_Cb * cb, L_Oper * oper)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(oper->dest[i]))
	continue;
      if (!L_no_flow_dep_from_for (cb, oper, oper->dest[i]))
	return 0;
    }
  return 1;
}

int
L_can_copy_op_to_all_live_paths (L_Cb * cb, L_Oper * oper)
{
  int i, j, redef;
  L_Oper *ptr, *last_def;

  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    return 1;

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(oper->dest[i]))
	continue;
      last_def = NULL;
      for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
	{
	  /* check for redef */
	  redef = 0;
	  for (j = 0; j < L_max_dest_operand; j++)
	    {
	      if (L_same_operand (oper->dest[i], ptr->dest[j]))
		{
		  if (PG_superset_predicate_ops (ptr, oper))
		    {
		      redef = 1;
		      break;
		    }
		  else
		    {
		      /* possible redef, but not executed same times as oper */
		      last_def = ptr;
		    }
		}
	    }
	  if (redef)
	    break;
	  if (!L_general_branch_opcode (ptr) && !L_check_branch_opcode (ptr))
	    continue;
	  if (!PG_intersecting_predicates_ops (oper, ptr))
	    continue;
	  if (!L_in_oper_OUT_set (cb, ptr, oper->dest[i], TAKEN_PATH))
	    continue;
	  if (last_def)
	    {
	      /* last def value reaches OUT */
	      if (PG_superset_predicate_ops (last_def, ptr))
		continue;
	      return 0;
	    }
	  else
	    {
	      /* oper value reaches OUT */
	      if (PG_superset_predicate_ops (oper, ptr))
		continue;
	      return 0;
	    }
	}
    }

  return 1;
}

/*
 *      conservative, checks for anti dep first, then redef
 */
int
L_no_anti_dep_from_before_redef_of (L_Cb * cb, L_Oper * oper,
				    L_Operand * operand)
{
  int i, j;
  L_Oper *ptr;
  L_Operand *dest;
  if (!operand)
    L_punt ("L_no_anti_dep_from_before_redef_of: operand is NULL");
  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = ptr->dest[i]))
	    continue;
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (dest, oper->src[j]))
		{
		  /* if the destination is not live past this point,
		     can still return 1!! */
		  if (L_in_oper_OUT_set (cb, ptr, operand, FALL_THRU_PATH))
		    return 0;
		}
	    }
	  /* Conservative, does not check to see if predicates of redefs
	     sum up to predicate of oper */
	  if ((L_same_operand (dest, operand)) &&
	      (PG_superset_predicate_ops (ptr, oper)))
	    return 1;
	}
    }
  return 1;
}

int
L_no_anti_dep_from_before_redef (L_Cb * cb, L_Oper * oper)
{
  int i;
  L_Operand *dest;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(dest = oper->dest[i]))
	continue;
      if (!L_no_anti_dep_from_before_redef_of (cb, oper, dest))
	return 0;
    }
  return 1;
}

int
L_single_anti_dep_from_before_redef_of (L_Oper * oper, L_Operand * operand)
{
  int i, j, count;
  L_Oper *ptr;
  L_Operand *dest;
  if (!operand)
    L_punt ("L_single_anti_dep_from_before_redef_of: operand is NULL");
  count = 0;
  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(dest = ptr->dest[i]))
	    continue;
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (dest, oper->src[j]))
		count++;
	      if (count == 2)
		return 0;
	    }
	  if (L_same_operand (dest, operand))
	    return (count == 1);
	}
    }
  return (count == 1);
}

int
L_single_anti_dep_from_before_redef (L_Oper * oper)
{
  int i;
  L_Operand *dest;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(dest = oper->dest[i]))
	continue;
      if (!L_single_anti_dep_from_before_redef_of (oper, dest))
	return 0;
    }
  return 1;
}

int
L_profitable_for_migration (L_Cb * cb, L_Oper * oper)
{
  int i, j, redef;
  double cost, ratio;
  L_Oper *ptr;
  L_Flow *flow;
  if (cb->weight < L_MIN_WEIGHT_FOR_OP_MIG)
    return 0;
  cost = 0.0;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!oper->dest[i])
	continue;
      for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
	{
	  redef = 0;
	  for (j = 0; j < L_max_dest_operand; j++)
	    {
	      if (L_same_operand (oper->dest[i], ptr->dest[j]))
		{
		  redef = 1;
		  break;
		}
	    }
	  if (redef)
	    break;
	  if (!L_general_branch_opcode (ptr) && !L_check_branch_opcode (ptr))
	    continue;
	  if (!L_in_oper_OUT_set (cb, ptr, oper->dest[i], TAKEN_PATH))
	    continue;
	  flow = L_find_flow_for_branch (cb, ptr);
	  /* SAM 10-94, never allow migration across a SB loop backedge */
	  if (flow->dst_cb == cb)
	    return 0;
	  if (L_single_predecessor_cb (flow->dst_cb))
	    continue;
	  cost += flow->weight;
	}
    }
  ratio = cost / cb->weight;
  return (ratio <= L_MAX_OP_MIG_COST_RATIO);
}

int
L_conditionally_redefined_in_cb (L_Cb * cb, L_Oper * opA, L_Operand * operand)
{
  L_Oper *ptr;

  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    return 0;

  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!L_is_dest_operand (operand, ptr))
	continue;
      if (!L_is_predicated (ptr))
	return 0;
      if (PG_equivalent_predicates_ops (opA, ptr))
	return 0;
      if (PG_superset_predicate_ops (ptr, opA))
	return 0;
      return 1;
    }

  return 0;
}

int
L_no_intersecting_br_between (L_Oper * opA, L_Oper * opB)
{
  L_Oper *oper;
  for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == opB)
	break;
      if (L_is_control_oper (oper))
	if (PG_intersecting_predicates_ops (opA, oper) ||
	    PG_intersecting_predicates_ops (oper, opB))
	  return 0;
    }
  return 1;
}

int
L_only_disjoint_br_between (L_Oper * opA, L_Oper * opB)
{
  L_Oper *oper;
  for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == opB)
	break;
      if (L_is_control_oper (oper))
	if (PG_intersecting_predicates_ops (oper, opB))
	  return 0;
    }
  return 1;
}


/*=====================================================================*/
/*
 *      Global optimization predicates
 */
/*=====================================================================*/

/* 
 *      Assumpts for this funct:
 *              1. cbA dominates cbB
 *              2. operand is a dest operand of opA
 *      ret 1 if opA is only definition of operand that reaches opB
 */
int
L_global_no_defs_between (L_Operand * operand, L_Cb * cbA, L_Oper * opA,
			  L_Cb * cbB, L_Oper * opB)
{
  L_Oper *ptr;
  if (!operand)
    return 1;
  for (ptr = cbB->first_op; ptr; ptr = ptr->next_op)
    {
      if (ptr == opB)
	break;
      if (PG_pred_graph && !PG_intersecting_predicates_ops (ptr, opB))
	continue;
      if (L_is_dest_operand (operand, ptr))
	return 0;
    }
  if (ptr != opB)
    L_punt ("L_global_no_defs_between: opB not found");
  return (L_in_cb_AIN_set (cbB, opA, operand));
}

int
L_all_dest_operand_global_no_defs_between (L_Oper * op, L_Cb * cbA,
					   L_Oper * opA, L_Cb * cbB,
					   L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!L_global_no_defs_between (op->dest[i], cbA, opA, cbB, opB))
	return 0;
    }
  return 1;
}

/*
 *      Assumpts for this funct:
 *              1. cbA dominates cbB
 *              2. operand is source operand of opA
 *      ret 1 if opA is only expression of operand that reaches opB
 */
int
L_global_same_def_reachs (L_Operand * operand, L_Cb * cbA, L_Oper * opA,
			  L_Cb * cbB, L_Oper * opB)
{
  int i;
  L_Oper *ptr;
  if (!operand)
    return 1;
  for (ptr = cbB->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == opB)
	break;
      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (ptr->dest[i], operand))
	  return 0;
    }
  if (ptr != opB)
    L_punt ("L_global_same_def_reachs: opB not found");
  return (L_in_cb_EIN_set (cbB, opA));
}


int
L_all_src_operand_global_same_def_reachs (L_Oper * op, L_Cb * cbA,
					  L_Oper * opA, L_Cb * cbB,
					  L_Oper * opB)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    if (!L_global_same_def_reachs (op->src[i], cbA, opA, cbB, opB))
      return 0;
  return 1;
}

/*
 *     Walks through ops, if operand redefined between definitions of operand1 and operand2,
 *     return 0.
 *     Currently not predicate aware, need to enhance.
 */
int
L_global_share_same_def (L_Operand * operand, L_Cb * cbA,
			 L_Operand * operandA, L_Operand * operandB)
{
  int i, foundA = 0, foundB = 0;
  L_Oper *ptr;
  if (!operand)
    return 1;
  for (ptr = cbA->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (L_same_operand (ptr->dest[i], operandA))
	    foundA = 1;
	  if (L_same_operand (ptr->dest[i], operandB))
	    foundB = 1;
	  if (foundA && foundB)
	    return 1;
	  if (L_same_operand (ptr->dest[i], operand))
	    {
	      if (foundA || foundB)
		return 0;
	      else
		continue;
	    }
	}
    }

  /* did not find one or both of the operands in the cb...be optimistic & return 1 */

  return 1;

}


/* 
 *      Return 1 if same defs of cbA reach cbB and there are no defs between.
 *      Differs from L_global_no_defs_between() in that it does not use opers
 *        to perform check.
 *      Assumpts for this funct:
 *              1. cbA dominates cbB
 */
int
L_global_no_defs_between_cb_only (L_Cb * cbA, L_Cb * cbB, L_Operand * var)
{
  L_Cb * def_cb;
  Set defsA, defsB;
  int i, num_reach_defs, *reach_array;

  /* First, check that the same defs of var reach both cbA and cbB */
  defsA = L_get_cb_RIN_defining_opers (cbA, var);
  defsB = L_get_cb_RIN_defining_opers (cbB, var);
  if (!(Set_subtract_empty (defsB, defsA)))
    {
      defsA = Set_dispose (defsA);
      defsB = Set_dispose (defsB);
      return 0;
    }
  num_reach_defs = Set_size (defsA);
  if (num_reach_defs == 0)  /* Just in case var not defined, stop opti. */
    {
      defsA = Set_dispose (defsA);
      defsB = Set_dispose (defsB);
      return 0;
    }
  reach_array = (int *) Lcode_malloc (sizeof (int) * num_reach_defs);
  Set_2array (defsA, reach_array);
  defsA = Set_dispose (defsA);
  defsB = Set_dispose (defsB);

  /* Second, check that no defining oper is dominated by cbA, meaning that
   * it reaches cbA via a loopback path, and potentially redefs var between
   * cbA and cbB.
   */
  for (i = 0; i < num_reach_defs; i++)
    {
      def_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, reach_array[i]);
      if (L_in_cb_DOM_set(def_cb, cbA->id))
        {
	  Lcode_free (reach_array);
          return 0;
        }
    }

  Lcode_free (reach_array);
  return 1;
}


/*
 *  No sub calls between opA and opB iff:
 *      1. cbA->id not in cbB->ext->sub_call_between
 *      2. no sub call from opA to cbA->last_op
 *              if L_CB_SUB_CALL is 0 for cbA, no need to perform search
 *      3. no sub call from cbB->first_op  to opB
 *              if L_CB_SUB_CALL is 0 for cbB, no need to perform search
 */
int
L_global_no_sub_call_between (L_Cb * cbA, L_Oper * opA, L_Cb * cbB,
			      L_Oper * opB)
{
  L_Oper *oper;
  L_Danger_Ext *ext;
  if (!opA || !opB)
    L_punt ("L_global_no_sub_call_between: opA and opB cannot be NIL");
  /* condition 1 */
  ext = cbB->ext;
  if (Set_in (ext->sub_call_between, cbA->id))
    return 0;
  /* condition 2 */
  if (L_EXTRACT_BIT_VAL (cbA->flags, L_CB_SUB_CALL))
    {
      for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
	{
	  if ((L_subroutine_call_opcode (oper)) &&
	      (!L_side_effect_free_sub_call (oper)))
	    return 0;
	}
    }
  /* condition 3 */
  if (L_EXTRACT_BIT_VAL (cbB->flags, L_CB_SUB_CALL))
    {
      for (oper = cbB->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (oper == opB)
	    break;
	  if ((L_subroutine_call_opcode (oper)) &&
	      (!L_side_effect_free_sub_call (oper)))
	    return 0;
	}
      if (oper != opB)
	L_punt ("L_global_no_sub_call_between: opB not found in cbB");
    }
  return 1;
}

int
L_global_no_general_sub_call_between (L_Cb * cbA, L_Oper * opA, L_Cb * cbB,
				      L_Oper * opB)
{
  L_Oper *oper;
  L_Danger_Ext *ext;
  if (!opA || !opB)
    L_punt
      ("L_global_no_general_sub_call_between: opA and opB cannot be NIL");
  /* condition 1 */
  ext = cbB->ext;
  if (Set_in (ext->general_sub_call_between, cbA->id))
    return 0;
  /* condition 2 */
  if (L_EXTRACT_BIT_VAL (cbA->flags, L_CB_GENERAL_SUB_CALL))
    {
      for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_general_subroutine_call_opcode (oper))
	    return 0;
	}
    }
  /* condition 3 */
  if (L_EXTRACT_BIT_VAL (cbB->flags, L_CB_GENERAL_SUB_CALL))
    {
      for (oper = cbB->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (oper == opB)
	    break;
	  if (L_general_subroutine_call_opcode (oper))
	    return 0;
	}
      if (oper != opB)
	L_punt ("L_global_no_general_sub_call_between: opB not found in cbB");
    }
  return 1;
}

int
L_global_no_sync_between (L_Cb * cbA, L_Oper * opA, L_Cb * cbB, L_Oper * opB)
{
  L_Oper *oper;
  L_Danger_Ext *ext;
  if (!opA || !opB)
    L_punt ("L_global_no_sync_between: opA and opB cannot be NIL");
  /* condition 1 */
  ext = cbB->ext;
  if (Set_in (ext->sync_between, cbA->id))
    return 0;
  /* condition 2 */
  if (L_EXTRACT_BIT_VAL (cbA->flags, L_CB_SYNC))
    {
      for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_sync_opcode (oper))
	    return 0;
	}
    }
  /* condition 3 */
  if (L_EXTRACT_BIT_VAL (cbB->flags, L_CB_SYNC))
    {
      for (oper = cbB->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (oper == opB)
	    break;
	  if (L_sync_opcode (oper))
	    return 0;
	}
      if (oper != opB)
	L_punt ("L_global_no_sync_between: opB not found in cbB");
    }
  return 1;
}


/*
 *  No danger between opA and opB iff:
 *      1. no sync between opA and opB
 *      2. if memory_flag, no subroutine calls between opA and opB
 *      3. if macro_flag, no generic subroutine calls between opA and opB
 */
int
L_global_no_danger (int macro_flag, int load_flag, int store_flag, L_Cb * cbA,
		    L_Oper * opA, L_Cb * cbB, L_Oper * opB)
{
  int memory_flag;

  memory_flag = load_flag | store_flag;
  if (L_EXTRACT_BIT_VAL (cbA->flags, L_CB_BOUNDARY) ||
      L_EXTRACT_BIT_VAL (cbB->flags, L_CB_BOUNDARY))
    return 0;
  if (!L_global_no_sync_between (cbA, opA, cbB, opB))
    return 0;
  if ((memory_flag) && (!L_global_no_sub_call_between (cbA, opA, cbB, opB)))
    return 0;
  if ((macro_flag)
      && (!L_global_no_general_sub_call_between (cbA, opA, cbB, opB)))
    return 0;
  return 1;
}

/*
 *  No danger between opA and opB iff:
 *      1. no sync between opA and opB
 *      2. if memory_flag, no subroutine calls between opA and opB
 *      3. if macro_falg, no generic subroutine calls between opA and opB
 *
 *   NOTE:  cbB may be a boundary cb, the calling function better know this
 *          and take the appropriate action! (but cbA cannot, for now)
 */
int
L_global_no_danger_to_boundary (int macro_flag, int load_flag,
				int store_flag, L_Cb * cbA,
				L_Oper * opA, L_Cb * cbB, L_Oper * opB)
{
  int memory_flag;

  memory_flag = load_flag | store_flag;
  if (L_EXTRACT_BIT_VAL (cbA->flags, L_CB_BOUNDARY))
    return 0;
  if (!L_global_no_sync_between (cbA, opA, cbB, opB))
    return 0;
  if ((memory_flag) && (!L_global_no_sub_call_between (cbA, opA, cbB, opB)))
    return 0;
  if ((macro_flag)
      && (!L_global_no_general_sub_call_between (cbA, opA, cbB, opB)))
    return 0;
  return 1;
}

/*
 *  Ret 1 iff no write between opA and opB and to either location opA and opB
 */
int
L_global_no_overlap_write (L_Cb * cbA, L_Oper * opA, L_Cb * cbB, L_Oper * opB)
{
  L_Oper *oper;
  L_Danger_Ext *ext;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);
  if (!opA || !opB)
    L_punt ("L_global_no_overlap_write: opA and opB cannot be NIL");
  /* condition 1 */
  ext = cbB->ext;
  if (Set_in (ext->store_between, cbA->id))
    return 0;
  /* condition 2 */
  if (L_EXTRACT_BIT_VAL (cbA->flags, L_CB_STORE))
    {
      for (oper = opA->next_op; oper != NULL; oper = oper->next_op)
	if (L_general_store_opcode (oper))
	  return 0;
    }
  /* condition 3 */
  if (L_EXTRACT_BIT_VAL (cbB->flags, L_CB_STORE))
    {
      for (oper = cbB->first_op; oper && (oper != opB); oper = oper->next_op)
	if (L_general_store_opcode (oper) &&
	    (!L_independent_memory_ops2 (cbB, opB, cbB, oper, dep_flags)) &&
	    (!L_independent_memory_ops2 (cbA, opA, cbB, oper, dep_flags)))
	  return 0;

      if (oper != opB)
	L_punt ("L_global_no_overlap_write: opB not found in cbB");
    }
  return 1;
}

int
L_global_only_branch_src_operand (L_Operand * operand)
{
  int i;
  L_Cb *cb;
  L_Oper *oper;
  if (!operand)
    return 1;
  for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (!L_same_operand (oper->src[i], operand))
		continue;
	      if (!L_cond_branch_opcode (oper))
		return 0;
	    }
	}
    }
  return 1;
}

/*==========================================================================*/
/*
 *      Loop optimization predicates
 */
/*==========================================================================*/

int
L_in_nested_loop (L_Loop * loop, L_Cb * cb)
{
  int i, flag, num_nested_loops, *buf = NULL;
  L_Loop *nested_loop;

  num_nested_loops = Set_size (loop->nested_loops);
  if (num_nested_loops > 0)
    {
      buf = (int *) Lcode_malloc (sizeof (int) * num_nested_loops);
      Set_2array (loop->nested_loops, buf);
    }

  flag = 0;
  for (i = 0; i < num_nested_loops; i++)
    {
      nested_loop = L_find_loop (L_fn, buf[i]);
      if (Set_in (nested_loop->loop_cb, cb->id))
	{
	  flag = 1;
	  break;
	}
    }

  if (buf != NULL)
    Lcode_free (buf);
  return (flag);
}

int
L_cb_dominates_all_loop_exit_cb (L_Loop * loop, int *exit_cb, int num_exit_cb,
				 L_Cb * cb)
{
  int i, dom_flag;
  L_Cb *ptr;

  dom_flag = 1;
  for (i = 0; i < num_exit_cb; i++)
    {
      ptr = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, exit_cb[i]);
      if (!L_in_cb_DOM_set (ptr, cb->id))
	{
	  dom_flag = 0;
	  break;
	}
    }

  return (dom_flag);
}

int
L_cb_dominates_all_loop_backedge_cb (L_Loop * loop, int *backedge_cb,
				     int num_backedge_cb, L_Cb * cb)
{
  int i, dom_flag;
  L_Cb *ptr;

  dom_flag = 1;
  for (i = 0; i < num_backedge_cb; i++)
    {
      ptr = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, backedge_cb[i]);
      if (!L_in_cb_DOM_set (ptr, cb->id))
	{
	  dom_flag = 0;
	  break;
	}
    }

  return (dom_flag);
}

int
L_loop_invariant_operands (L_Loop * loop, int *loop_cb, int num_cb,
			   L_Oper * oper)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!oper->src[i])
	continue;
      if (!L_is_loop_inv_operand (loop, loop_cb, num_cb, oper->src[i]))
	return 0;
    }
  return 1;
}

/*
 *  Cost function for deciding whether to move things to preheader of a loop
 */
int
L_cost_effective_to_move_ops_to_preheader (L_Loop * loop)
{
  int num_nested_loops, num_cb;
  num_nested_loops = Set_size (loop->nested_loops);
  if (num_nested_loops == 0)
    return 1;
  num_cb = Set_size (loop->loop_cb);
  return (num_cb <= L_MAX_CB_FOR_PREHEADER_MOVE);
}

/* 
 *  All uses in the loop of op->dest come from op if:
 *      1. No uses of op->dest before op in its cb
 *      2. All other blocks with uses of op->dest have bb(op->dest)
 *         in their dominator sets.
 *  This only allows one def of op->dest in the loop. Multiple identical defs are 
 *  not handled at this point but are planned.
 *  Note that this erroneously gives a true result even though an inner loop may
 *  redefine op->dest and then reuse it.
 */
int
L_all_uses_in_loop_from (L_Loop * loop, int *loop_cb, int num_cb, L_Cb * cb,
			 L_Oper * op)
{
  int i, j, k, use;
  L_Oper *ptr;
  L_Cb *cb_ptr;
  /* 1. */
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == op)
	break;
      for (j = 0; j < L_max_dest_operand; j++)
	{
	  if (!op->dest[j])
	    continue;
	  for (k = 0; k < L_max_src_operand; k++)
	    if (L_same_operand (op->dest[j], ptr->src[k]))
	      return 0;
	}
    }
  /* 2. */
  for (i = 0; i < num_cb; i++)
    {
      if (loop_cb[i] == cb->id)
	continue;
      if (!(cb_ptr = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i])))
	L_punt ("L_all_uses_in_loop_from: corrupt cb");
      use = 0;
      for (ptr = cb_ptr->first_op; ((ptr != NULL) & (!use));
	   ptr = ptr->next_op)
	{
	  for (j = 0; ((j < L_max_dest_operand) & (!use)); j++)
	    {
	      if (!(op->dest[j]))
		continue;
	      for (k = 0; k < L_max_src_operand; k++)
		{
		  if (!L_same_operand (op->dest[j], ptr->src[k]))
		    continue;
		  use = 1;
		  break;
		}
	    }
	}
      if ((use) && (!L_in_cb_DOM_set (cb_ptr, cb->id)))
	return 0;
    }
  return 1;
}

/* 
 *  All uses in the loop of op->dest come from op if:
 *      1. No uses of op->dest before op in its cb
 *      2. All other blocks with uses of op->dest have bb(op->dest)
 *         in their dominator sets.
 *  This only allows one def of op->dest in the loop. Multiple identical defs are 
 *  not handled at this point but are planned.
 *  Note that this erroneously gives a true result even though an inner loop may
 *  redefine op->dest and then reuse it.
 */
int
L_no_uses_before_in_loop (L_Loop * loop, int *loop_cb, int num_cb, L_Cb * cb,
			  L_Oper * op)
{
  int j, k;
  L_Oper *ptr;

  /* 1. */
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == op)
	break;
      for (j = 0; j < L_max_dest_operand; j++)
	{
	  if (!op->dest[j])
	    continue;
	  for (k = 0; k < L_max_src_operand; k++)
	    if (L_same_operand (op->dest[j], ptr->src[k]))
	      return 0;
	}
    }
  return 1;
}

/*
 *  a definition in the loop reaches outside the loop if all exit blocks
 *  with dest(op) in their out sets are dominated by cb(op)
 */
int
L_def_reachs_all_out_cb_of_loop (L_Loop * loop, int *exit_cb, int num_exit_cb,
				 L_Cb * cb, L_Oper * op)
{
  int i, j;
  L_Cb *e_cb;
  for (i = 0; i < num_exit_cb; i++)
    {
      if (!(e_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, exit_cb[i])))
	L_punt ("L_def_reachs_all_out_cb_of_loop: corrupt cb");
      for (j = 0; j < L_max_dest_operand; j++)
	{
	  if (!op->dest[j])
	    continue;
	  if (L_in_cb_OUT_set (e_cb, op->dest[j]) &&
	      (!L_in_cb_DOM_set (e_cb, cb->id)))
	    return 0;
	}
    }
  return 1;
}

/*
 *  An op can be moved out of a loop to this point if it is not executed
 *  on every iteration, but in those exit blocks where dest(op) occurs in the
 *  out set op must be the reaching def.  This may introduce some unsafe
 *  invariant removals for ops that are not executed on every iteration of
 *  of the loop.  For operations that are executed on every iteration, this
 *  predicate returns 1, if the op is not executed on every iteration,
 *  the assumpt provided by the user are used to distinguish a safe move
 *  from an unsafe move.
 */
int
L_safe_to_move_out_of_loop (L_Loop * loop, int *exit_cb, int num_exit_cb,
			    L_Cb * cb, L_Oper * op)
{
  ITintmax cd_lev;
  L_Oper *ptr;
  L_Attr *attr;

  /* thing has completely safe to move */
  if (L_safe_for_speculation (op))
    return 1;

  /* check if always executed.  bb(op) in dom(all exit blocks of loop) */

  if (L_cb_dominates_all_loop_exit_cb (loop, exit_cb, num_exit_cb, cb))
    return 1;

  /* added 10-17-94, SAM to take advantage of Roger's safety analysis */
  /* check control dep level of header, to see if can possibly move above
     loop header in which case can move out as invariant code */

  if ((attr = L_find_attr (op->attr, "cdl")) &&
      L_is_int_constant (attr->field[0]))
    {
      cd_lev = attr->field[0]->value.i;
      for (ptr = loop->header->first_op; ptr != NULL; ptr = ptr->next_op)
	{
	  if (L_general_branch_opcode (ptr) || L_check_branch_opcode (ptr))
	    {
	      if ((attr = L_find_attr (ptr->attr, "cdl")) &&
		  L_is_int_constant (attr->field[0]) &&
		  (cd_lev <= attr->field[0]->value.i))
		return 1;
	      else
		return 0;
	    }
	}
    }

  return 0;

}

/*
 * no danger in loop if:
 *      1. No synchronization operations in loop
 *      2. if macro_flag, no function calls in loop
 *      3. if memory_flag, no store operations in loop
 */
int
L_no_danger_in_loop (L_Loop * loop, int *loop_cb, int num_cb, int macro_flag,
		     int load_flag, int store_flag)
{
  int i, memory_flag;
  L_Cb *cb;
  L_Oper *op;

  memory_flag = load_flag | store_flag;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (L_sync_opcode (op))
	    return 0;
	  if (memory_flag)
	    {
	      if ((store_flag) && (L_subroutine_call_opcode (op)))
		return 0;
	      if ((!store_flag) &&
		  (L_subroutine_call_opcode (op)) &&
		  (!L_side_effect_free_sub_call (op)))
		return 0;
	    }
	  if ((macro_flag) && (L_general_subroutine_call_opcode (op)))
	    return 0;
	}
    }
  return 1;
}

/*
 *  Return 1 if op does is independent of all other stores in the loop,
 *  omit is optional to exclude an operation from this search.
 */
int
L_no_memory_conflicts_in_loop (L_Loop * loop, int *loop_cb, int num_cb,
			       L_Oper * op, L_Oper * omit)
{
  int i, dep_flags;
  L_Cb *cb, *opcb;
  L_Oper *ptr;

  dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);
  if (L_difference_in_nesting_level (op, loop->nesting_level) > 0)
    dep_flags |= SET_OUTER_CARRIED (0);

  opcb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, op->id);
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
	{
	  if (ptr == omit)
	    continue;
	  if (!L_general_store_opcode (ptr))
	    continue;
	  if (!L_independent_memory_ops2 (opcb, op, cb, ptr, dep_flags))
	    return 0;
	}
    }
  return 1;
}

/*
 *      for now don't allow pre/post inc lds and sts to be migrated!!!
 */
int
L_unique_memory_location (L_Loop * loop, int *loop_cb, int num_cb,
			  L_Oper * op, int *n_read, int *n_write,
			  L_Oper ** store_op)
{
  int i, n_r, n_w;
  L_Oper *ptr;
  L_Cb *cb, *opcb;
  int dep_flags;

  dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);

  n_r = n_w = 0;
  *n_read = *n_write = 0;
  opcb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, op->id);
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
	{
	  if (!(L_general_store_opcode (ptr) || L_general_load_opcode (ptr)))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
	   *              store. */
	  if (L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_VOLATILE))
	    continue;

	  if (L_same_operand (op->src[0], ptr->src[0]) &&
	      L_same_operand (op->src[1], ptr->src[1]))
	    {
	      if (!(L_load_opcode (ptr) || L_store_opcode (ptr)))
		return 0;
	      if (L_load_opcode (ptr))
		n_r++;
	      if (L_store_opcode (ptr))
		{
		  n_w++;
		  *store_op = ptr;
		}
	      if (!L_same_data_types (ptr, op))
		return 0;
	      continue;
	    }
	  if (!L_independent_memory_ops2 (opcb, op, cb, ptr, dep_flags))
	    return 0;
	}
    }
  *n_read = n_r;
  *n_write = n_w;
  return 1;
}

int
L_all_predecessor_cb_in_loop (L_Loop * loop, L_Cb * cb)
{
  L_Flow *flow;
  if (!loop || !cb)
    L_punt ("L_all_predecessor_cb_in_loop: l and bb cannot be NIL");
  if (!(flow = cb->src_flow))
    L_punt ("L_all_predecessor_cb_in_loop: cb has no predecessors");
  for (; flow != NULL; flow = flow->next_flow)
    if (!Set_in (loop->loop_cb, flow->src_cb->id))
      return 0;
  return 1;
}

int
L_branch_to_loop_cb (L_Loop * loop, L_Oper * op)
{
  if (L_cond_branch_opcode (op))
    return (Set_in (loop->loop_cb, op->src[2]->value.cb->id));
  else if (L_uncond_branch_opcode (op))
    return (Set_in (loop->loop_cb, op->src[0]->value.cb->id));
  else
    {
      L_punt ("L_branch_to_loop_bb: illegal op");
      return 0;
    }
}

int
L_basic_induction_var (L_Loop * loop, L_Operand * operand)
{
  if (!L_is_register (operand))
    return 0;
  return (Set_in (loop->basic_ind_var, operand->value.r));
}

int
L_num_constant_increment_of_ind (L_Operand * operand, L_Ind_Info * ind_info)
{
  return (L_is_numeric_constant (L_find_basic_induction_increment (operand,
								   ind_info)));
}

int
L_same_ind_increment (L_Operand * operand1, L_Operand * operand2,
		      L_Ind_Info * ind_info)
{
  L_Operand *inc1, *inc2;
  inc1 = L_find_basic_induction_increment (operand1, ind_info);
  inc2 = L_find_basic_induction_increment (operand2, ind_info);
  return (L_same_operand (inc1, inc2));
}

int
L_int_one_increment_of_ind (L_Operand * operand, L_Ind_Info * ind_info)
{
  return (L_is_int_one
	  (L_find_basic_induction_increment (operand, ind_info)));
}

int
L_int_neg_one_increment_of_ind (L_Operand * operand, L_Ind_Info * ind_info)
{
  return (L_is_int_neg_one (L_find_basic_induction_increment (operand,
							      ind_info)));
}

/*
 *      Return 1 if inc2 / inc1 has no remainder
 */
int
L_ind_increment_is_multiple_of (L_Operand * operand1, L_Operand * operand2,
				L_Ind_Info * ind_info)
{
  L_Operand *inc1, *inc2;
  ITintmax rem;
  inc1 = L_find_basic_induction_increment (operand1, ind_info);
  inc2 = L_find_basic_induction_increment (operand2, ind_info);
  if (!L_is_int_constant (inc1))
    L_punt ("L_ind_increment_is_multiple_of: inc1 is not int const");
  if (!L_is_int_constant (inc2))
    L_punt ("L_ind_increment_is_multiple_of: inc2 is not int const");
  rem = inc2->value.i % inc1->value.i;
  return (rem == 0);
}

/*
 *      This is a naive implementation of this predicate-only look in preheader
 *      for an assignment of the induction var.  In future may need to expand
 *      this search to other blocks.
 */
int
L_num_constant_init_val_of_ind (L_Operand * operand, L_Ind_Info * ind_info)
{
  L_Ind_Info *info;
  if (!(info = L_find_ind_info (ind_info, operand, 1)))
    return 0;
  if (info->coeff != 0)
    return 0;
  return 1;
}

int
L_same_ind_initial_val (L_Loop * loop, L_Operand * operand1,
			L_Operand * operand2, L_Ind_Info * ind_info)
{
  int i;
  L_Oper *op1, *op2;
  L_Ind_Info *info1, *info2;
  if (!(L_is_register (operand1) && L_is_register (operand2)))
    L_punt ("L_same_ind_initial_val: operands must be registers");
  if (!(Set_in (loop->basic_ind_var, operand1->value.r) &&
	Set_in (loop->basic_ind_var, operand2->value.r)))
    L_punt ("L_same_ind_initial_val: operands must be ind vars");

  /* check ind info */
  info1 = L_find_ind_info (ind_info, operand1, 1);
  info2 = L_find_ind_info (ind_info, operand2, 1);
  if ((info1 != NULL) & (info2 != NULL))
    {
      if ((info1->coeff == info2->coeff) &&
	  (info1->base == info2->base) && (info1->offset == info2->offset))
	return 1;
      else if ((info1->coeff == 1) &&
	       (L_same_operand (info1->base, operand2)) &&
	       (info1->offset == 0))
	return 1;
      else if ((info2->coeff == 1) &&
	       (L_same_operand (info2->base, operand1)) &&
	       (info2->offset == 0))
	return 1;
    }
  else if ((info1 == NULL) & (info2 != NULL))
    {
      if ((info2->coeff == 1) &&
	  (L_same_operand (info2->base, operand1)) && (info2->offset == 0))
	return 1;
    }
  else if ((info1 != NULL) & (info2 == NULL))
    {
      if ((info1->coeff == 1) &&
	  (L_same_operand (info1->base, operand2)) && (info1->offset == 0))
	return 1;
    }

  /* check other types of defs than allowed by ind_info */
  if (!(op1 = L_find_last_def_in_cb (loop->preheader, operand1)))
    return 0;
  if (!(op2 = L_find_last_def_in_cb (loop->preheader, operand2)))
    return 0;
  if (L_same_opcode (op1, op2))
    {
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!L_same_operand (op1->src[i], op2->src[i]) ||
	      !L_same_def_reachs (op1->src[i], op1, op2))
	    return 0;
	}
      return 1;
    }
  return 0;
}

/*      Induction will reach limit if (1) or (2) or (3):
 *      1. Induction initial val, increment, and final val are all constant
 *         (trivial)
 *      2. Preheader has 1 src block and that block is immediately before the
 *         preheader.  The block must end with a cond branch that has the 
 *         opposite branch opcode, and the same operands as the loop branch.
 *      3. Preheader has 1 src block, and the block is not immediately before
 *         the preheader.  The block must end with a cond branch with the same
 *         branch opcode, and the same operands as the branch.
 */
int
L_ind_var_will_reach_limit (L_Loop * loop, L_Oper * op, L_Ind_Info * ind_info)
{
  L_Operand *ind_var, *limit, *ind_inc;
  ITintmax limit_val, ind_inc_val, init_val;
  L_Cb *preheader, *src_cb;
  L_Oper *last, *ptr;
  L_Ind_Info *info;

  ind_var = op->src[0];
  limit = op->src[1];
  if (!L_is_register (ind_var))
    L_punt ("L_ind_var_will_reach_limit: src1 not a register");
  if (!Set_in (loop->basic_ind_var, ind_var->value.r))
    L_punt ("L_ind_var_will_reach_limit: src1 not ind var");
  ind_inc = L_find_basic_induction_increment (ind_var, ind_info);
  if (!L_is_numeric_constant (ind_inc))
    L_punt ("L_ind_var_will_reach_limit: ind_inc must be constant");
  ind_inc_val = ind_inc->value.i;
  /* Case 1 */
  if (L_num_constant_init_val_of_ind (ind_var, ind_info) &&
      L_is_int_constant (limit))
    {
      info = L_find_ind_info (loop->ind_info, ind_var, 1);
      init_val = info->offset;
      limit_val = limit->value.i;
      if (ind_inc_val > 0)
	return (init_val < limit_val);
      else
	return (init_val > limit_val);
    }
  /* Check that preheader has 1 src_bb (applies to cases 2 and 3) */
  preheader = loop->preheader;
  if (preheader->src_flow == NULL)
    return 0;
  if (preheader->src_flow->next_flow != NULL)
    return 0;
  src_cb = preheader->src_flow->src_cb;
  last = src_cb->last_op;
  /* Case 2 */
  if (preheader->prev_cb == src_cb)
    {
      if (!L_cond_branch_opcode (last))
	return 0;
      if (!L_are_opposite_branch_opcodes (last, op))
	return 0;
      if (L_branch_to_loop_cb (loop, last))
	return 0;
      if (!L_same_operand (last->src[1], op->src[1]))
	{
	  ptr = L_find_last_def_in_cb (loop->preheader, op->src[1]);
	  if (ptr != NULL)
	    {
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[1]))
		return 0;
	    }
	  else
	    {
	      ptr = L_find_last_def_in_cb (src_cb, op->src[1]);
	      if (ptr == NULL)
		return 0;
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[1]))
		return 0;
	    }
	}
      if (!L_same_operand (last->src[0], op->src[0]))
	{
	  ptr = L_find_last_def_in_cb (loop->preheader, ind_var);
	  if (ptr != NULL)
	    {
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[0]))
		return 0;
	    }
	  else
	    {
	      ptr = L_find_last_def_in_cb (src_cb, ind_var);
	      if (ptr == NULL)
		return 0;
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[0]))
		return 0;
	    }
	}
      return 1;
    }
  /* Case 3 */
  else
    {
      if (L_uncond_branch_opcode (last) &&
	  L_cond_branch_opcode (last->prev_op))
	last = last->prev_op;
      if (!L_cond_branch_opcode (last))
	return 0;
      if (!L_are_same_branch_opcodes (last, op))
	return 0;
      if (!L_same_operand (last->src[1], op->src[1]))
	{
	  ptr = L_find_last_def_in_cb (loop->preheader, op->src[1]);
	  if (ptr != NULL)
	    {
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[1]))
		return 0;
	    }
	  else
	    {
	      ptr = L_find_last_def_in_cb (src_cb, op->src[1]);
	      if (ptr == NULL)
		return 0;
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[1]))
		return 0;
	    }
	}
      if (!L_same_operand (last->src[0], op->src[0]))
	{
	  ptr = L_find_last_def_in_cb (loop->preheader, ind_var);
	  if (ptr != NULL)
	    {
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[0]))
		return 0;
	    }
	  else
	    {
	      ptr = L_find_last_def_in_cb (src_cb, ind_var);
	      if (ptr == NULL)
		return 0;
	      if (!L_is_opcode (Lop_MOV, ptr))
		return 0;
	      if (!L_same_operand (ptr->src[0], last->src[0]))
		return 0;
	    }
	}
      return 1;
    }
}

/*
 *      Can simplify a branch in a loop to beq or bne if:
 *      1. induction increment positive:  bgt or bge branch must go somewhere
 *         other than loop header, blt or ble must go to header.
 *      2. induction increment negative:  opposite cond to (1)
 *      Note: ind increment must be an integer constant!!!
 */
int
L_can_simplify_loop_branch (L_Loop * loop, L_Oper * op, L_Ind_Info * ind_info)
{
  L_Operand *ind_var, *ind_inc;
  ITintmax ind_inc_val;
  L_Cb *target;
  ind_var = op->src[0];

  if (!L_is_register (ind_var))
    L_punt ("L_can_simplify_loop_branch: src1 is not a register");
  if (!Set_in (loop->basic_ind_var, ind_var->value.r))
    L_punt ("L_can_simplify_loop_branch: src1 must be ind var");
  ind_inc = L_find_basic_induction_increment (ind_var, ind_info);
  if (!L_is_numeric_constant (ind_inc))
    L_punt ("L_can_simplify_loop_branch: ind var must const inc");
  ind_inc_val = ind_inc->value.i;
  target = op->src[2]->value.cb;
  if (ind_inc_val > 0)
    {
      if (L_int_bgt_branch_opcode (op) || L_int_bge_branch_opcode (op))
	return (target != loop->header);
      else if (L_int_blt_branch_opcode (op) || L_int_ble_branch_opcode (op))
	return (target == loop->header);
      else
	L_punt ("L_can_simplify_loop_branch: illegal opcode");
    }
  else
    {
      if (L_int_bgt_branch_opcode (op) || L_int_bge_branch_opcode (op))
	return (target == loop->header);
      else if (L_int_blt_branch_opcode (op) || L_int_ble_branch_opcode (op))
	return (target != loop->header);
      else
	L_punt ("L_can_simplify_loop_branch: illegal opcode");
    }
  return 0;
}

int
L_is_str_reducible_opcode (L_Oper * op)
{
  if (op == NULL)
    return 0;

  /* BS 1/95 */

  /* X86 supports a scaled addressing mode which can incorporate
     multiplies by 2, 4, or 8 into load instructions.  For archs
     like this, str reduction does not make sense */
  if (M_scaled_addressing_avail ())
    {
      if (L_int_mul_opcode (op) &&
	  (!L_is_power_of_two (op->src[0])) &&
	  (!L_is_power_of_two (op->src[1])))
	return 1;
      else
	return 0;
    }
  else
    {
      return (L_int_add_opcode (op) || L_int_sub_opcode (op) ||
	      L_int_mul_opcode (op) || L_is_opcode (Lop_LSL, op));
    }

}

int
L_useful_str_red (L_Loop * loop, L_Oper * op, L_Operand * ind_operand)
{
  int num_nested_loops;
  L_Loop *l;
  if (!(L_int_add_opcode (op) || L_int_sub_opcode (op)))
    return 1;
  num_nested_loops = Set_size (loop->nested_loops);
  if (num_nested_loops == 0)
    return 1;
  for (l = L_fn->first_loop; l != NULL; l = l->next_loop)
    {
      if (!Set_in (loop->nested_loops, l->id))
	continue;
      if (L_basic_induction_var (l, op->dest[0]))
	return 0;
      if (L_basic_induction_var (l, ind_operand))
	return 0;
    }
  return 1;
}

/*
 *      Return 1 iff oper not in IN set of all blocks in l->out_bb
 */
int
L_not_live_in_out_cb (int *out_cb, int num_out_cb, L_Operand * operand)
{
  int i;
  L_Cb *cb;
  for (i = 0; i < num_out_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, out_cb[i]);
      if (L_in_cb_IN_set (cb, operand))
	return 0;
    }
  return 1;
}

/*
 *      operand not used as a src operand in loop except in
 *      increments or decrements of oper.
 */
int
L_no_uses_of_ind (L_Loop * loop, int *loop_cb, int num_cb,
		  L_Operand * operand)
{
  int i, j;
  L_Cb *cb;
  L_Oper *op;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /*
	   *  Omit ind var op from search!!!
	   */
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (op->src[j], operand))
		return 0;
	    }
	}
    }
  return 1;
}

/*
 *      Ind should be reinitialized if it is used only with ld/st in 
 *      loop and the same operand is used in along with it.  In this
 *      case can reinit ind to ind+other to get rid of other in ld/st's.
 */
#if 0
int
L_ind_should_be_reinitialized (L_Loop * loop, int *loop_cb, int num_cb,
			       L_Operand * operand)
{
  int i, j;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *old, *curr;
  old = NULL;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  if (!L_is_src_operand (operand, op))
	    continue;
	  if (!(L_load_opcode (op) || L_store_opcode (op)))
	    return 0;
	  if (!(L_same_operand (operand, op->src[0]) ||
		L_same_operand (operand, op->src[1])))
	    return 0;
	  for (j = 2; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (operand, op->src[j]))
		return 0;
	    }
	  if (L_same_operand (operand, op->src[0]))
	    curr = op->src[1];
	  else
	    curr = op->src[0];
	  if (old == NULL)
	    {
	      old = curr;
	    }
	  else
	    {
	      if (!L_same_operand (curr, old))
		return 0;
	    }
	}
    }
  if (L_is_int_constant (old) && (old->value.i != 0))
    return 1;
  else if (L_is_label (old) || L_is_string (old))
    return 1;
  else if (L_is_register (old) &&
	   L_is_loop_inv_operand (loop, loop_cb, num_cb, old) &&
	   L_no_danger_in_loop (loop, loop_cb, num_cb, 0, 0, 0))
    return 1;
  else if (L_is_macro (old) &&
	   L_is_loop_inv_operand (loop, loop_cb, num_cb, old) &&
	   L_no_danger_in_loop (loop, loop_cb, num_cb, 1, 0, 0))
    return 1;
  else
    return 0;
}
#endif

/*
 *      Try to reinitialize address operands of mem_op to be reg+0
 */
int
L_ind_should_be_reinitialized (L_Loop * loop, int *loop_cb, int num_cb,
			       L_Operand * operand)
{
  int i, j;
  L_Oper *ptr;
  L_Operand *old, *curr;
  L_Cb *cb;

  old = curr = NULL;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, ptr->id))
	    continue;
	  if (!L_is_src_operand (operand, ptr))
	    continue;
	  if (L_load_opcode (ptr) || L_store_opcode (ptr))
	    {
	      if (!(L_same_operand (operand, ptr->src[0]) ||
		    L_same_operand (operand, ptr->src[1])))
		return 0;
	      for (j = 2; j < L_max_src_operand; j++)
		{
		  if (L_same_operand (operand, ptr->src[j]))
		    return 0;
		}
	      if (L_same_operand (operand, ptr->src[0]))
		curr = ptr->src[1];
	      else
		curr = ptr->src[0];
	      if (old == NULL)
		old = curr;

	      if (L_is_int_constant (old) && L_is_int_constant (curr))
		{
		  if (old->value.i == 0)
		    old = curr;
		  continue;
		}
	      else if (L_same_operand (curr, old))
		continue;
	      return 0;
	    }
	  else if (L_int_cond_branch_opcode (ptr))
	    {
	      if (!(L_same_operand (operand, ptr->src[0]) ||
		    L_same_operand (operand, ptr->src[1])))
		return 0;
	      if (L_same_operand (operand, ptr->src[0]))
		curr = ptr->src[1];
	      else
		curr = ptr->src[0];
	      if (old == NULL)
		old = curr;

	      if (L_is_int_constant (old) && L_is_int_constant (curr))
		{
		  if (old->value.i == 0)
		    old = curr;
		  continue;
		}
	      return 0;
	    }
	  else
	    {
	      return 0;
	    }
	}
    }

  if (L_is_int_constant (old) && (old->value.i != 0))
    return 1;
  else if (L_is_label (old) || L_is_string (old))
    return 1;
  else if (L_is_register (old) &&
	   L_is_loop_inv_operand (loop, loop_cb, num_cb, old) &&
	   L_no_danger_in_loop (loop, loop_cb, num_cb, 0, 0, 0))
    return 1;
  else if (L_is_macro (old) &&
	   L_is_loop_inv_operand (loop, loop_cb, num_cb, old) &&
	   L_no_danger_in_loop (loop, loop_cb, num_cb, 1, 0, 0))
    return 1;
  else
    return 0;
}


/*
 *      2 basic induction variables of l are in the same family if
 *      they are incremented in all the same places.  So for every
 *      point operand1 is incremented, operand2 must also be incremented.  Also
 *      operand2 may not be incremented at any other points in l.
 */
int
L_basic_ind_var_in_same_family (L_Loop * loop, int *loop_cb, int num_cb,
				L_Operand * operand1, L_Operand * operand2)
{
  int i, count1, count2;
  L_Cb *cb;
  L_Oper *op;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      count1 = count2 = 0;
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (!Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  if (L_same_operand (op->dest[0], operand1))
	    count1++;
	  else if (L_same_operand (op->dest[0], operand2))
	    count2++;
	}
      if (count1 != count2)
	return 0;
    }
  return 1;
}

/*
 *     No uses of operand1 between first and last defs of operand1 and operand2
 *     for all basic blocks in loop l.
 */
int
L_no_uses_of_between_first_and_last_defs (L_Loop * loop, int *loop_cb,
					  int num_cb, L_Operand * operand1,
					  L_Operand * operand2)
{
  int i, j, num_use_betw;
  L_Cb *cb;
  L_Oper *op1, *op2, *op;
  for (i = 0; i < num_cb; i++)
    {
      num_use_betw = 0;
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      op1 = op2 = NULL;
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (L_same_operand (op->dest[0], operand1) ||
	      L_same_operand (op->dest[0], operand2))
	    {
	      op1 = op;
	      break;
	    }
	}
      if (op1 == NULL)
	continue;
      for (op = cb->last_op; op != NULL; op = op->prev_op)
	{
	  if (L_same_operand (op->dest[0], operand1) ||
	      L_same_operand (op->dest[0], operand2))
	    {
	      op2 = op;
	      break;
	    }
	}
      for (op = op1; op != op2; op = op->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (op->src[j], operand1))
		num_use_betw += 1;
	    }
	}
      if (num_use_betw == 0)
	{
	  continue;
	}
      else if (L_reorder_ops_so_no_use_betw (loop, cb, operand1, op1, op2))
	{
	  continue;
	}
      else
	{
	  return 0;
	}
    }
  return 1;
}

/*
 *      SAM 2-97, added new parameters start_op1, start_op2 for the recursive
 *      backward search to start searching at the previous def rather than
 *      always starting at the bottom of the preheader.  This is clearly wrong.
 */
int
L_ind_constant_offset_initial_val (L_Cb * preheader, L_Operand * operand1,
				   L_Operand * operand2, L_Oper * start_op1,
				   L_Oper * start_op2, L_Ind_Info * ind_info)
{
  L_Ind_Info *info1, *info2;
  L_Oper *op1, *op2;
  L_Operand *base = NULL;

  if ((info1 = L_find_ind_info (ind_info, operand1, 1)))
    {
      op1 = info1->initop;
    }
  else
    {
      if (!start_op1)
	start_op1 = preheader->last_op;

      op1 = start_op1 ? L_prev_def (operand1, start_op1) : NULL;
    }

  if ((info2 = L_find_ind_info (ind_info, operand2, 1)))
    {
      op2 = info2->initop;
    }
  else
    {
      if (!start_op2)
	start_op2 = preheader->last_op;

      op2 = start_op2 ? L_prev_def (operand2, start_op2) : NULL;
    }

  if (!info1 && !info2)
    {
      if (op1 && op2 && L_int_add_opcode (op1) && L_int_add_opcode (op2))
	{
	  if (L_same_operand (op1->src[0], op2->src[0]))
	    {
	      if (L_ind_constant_offset_initial_val (preheader, op1->src[1],
						     op2->src[1], op1, op2,
						     ind_info))
		base = op1->src[0];
	    }
	  else if (L_same_operand (op1->src[0], op2->src[1]))
	    {
	      if (L_ind_constant_offset_initial_val (preheader, op1->src[1],
						     op2->src[0], op1, op2,
						     ind_info))
		base = op1->src[0];
	    }
	  else if (L_same_operand (op1->src[1], op2->src[0]))
	    {
	      if (L_ind_constant_offset_initial_val (preheader, op1->src[0],
						     op2->src[1], op1, op2,
						     ind_info))
		base = op1->src[1];
	    }
	  else if (L_same_operand (op1->src[1], op2->src[1]))
	    {
	      if (L_ind_constant_offset_initial_val (preheader, op1->src[0],
						     op2->src[0], op1, op2,
						     ind_info))
		base = op1->src[1];
	    }
	}
    }
  else if (!info1)
    {
      if (L_same_operand (info2->base, operand1) &&
	  (info2->coeff == 1) && (info2->offset <= 0))
	base = info2->base;
    }
  else if (!info2)
    {
      if (L_same_operand (info1->base, operand2) &&
	  (info1->coeff == 1) && (info1->offset >= 0))
	base = info1->base;
    }
  else
    {
      if (L_same_operand (info1->base, info2->base) &&
	  (info1->coeff == info2->coeff))
	base = info1->base;
    }

  /* If a common base has been identified, the result will be 1
   * provided that the same definition of the base reaches all
   * initializing operations.
   */

  if (base)
    {
      if (L_is_variable (base))
	{
	  L_Cb *cb1, *cb2;

	  if (!op1 || !op2)
	    return 0;

	  cb1 = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, op1->id);
	  cb2 = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, op2->id);

	  if ((cb1 != cb2) || !L_same_def_reachs (base, op1, op2))
	    return 0;
	}
      return 1;
    }

  return 0;
}

int
L_only_used_as_base_addr_with_const_offset_in_loop (L_Loop * loop,
						    int *loop_cb, int num_cb,
						    L_Operand * operand,
						    int offset)
{
  int i, j, old_num_op, new_num_op;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *old_src, *new_src;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (!L_is_src_operand (operand, op))
	    continue;
	  if (L_load_opcode (op) || L_store_opcode (op))
	    {
	      if (!(L_same_operand (operand, op->src[0]) ||
		    L_same_operand (operand, op->src[1])))
		return 0;
	      for (j = 2; j < L_max_src_operand; j++)
		{
		  if (L_same_operand (operand, op->src[j]))
		    return 0;
		}
	      if (!(L_is_int_constant (op->src[0]) ||
		    L_is_int_constant (op->src[1])))
		return 0;
	      old_num_op = M_num_oper_required_for (op, L_fn->name);
	      if (L_is_int_constant (op->src[0]))
		{
		  old_src = op->src[0];
		  new_src =
		    L_new_gen_int_operand (offset + op->src[0]->value.i);
		  op->src[0] = new_src;
		  new_num_op = M_num_oper_required_for (op, L_fn->name);
		  op->src[0] = old_src;
		}
	      else
		{
		  old_src = op->src[1];
		  new_src =
		    L_new_gen_int_operand (offset + op->src[1]->value.i);
		  op->src[1] = new_src;
		  new_num_op = M_num_oper_required_for (op, L_fn->name);
		  op->src[1] = old_src;
		}
	      L_delete_operand (new_src);
	      if (new_num_op > old_num_op)
		return 0;
	      else
		continue;
	    }
	  else
	    {
	      return 0;
	    }
	}
    }
  return 1;
}

int
L_all_uses_of_ind_can_change_offset (L_Loop * loop, int *loop_cb, int num_cb,
				     int *out_cb, int num_out_cb,
				     L_Operand * operand1,
				     L_Operand * operand2)
{
  int i, j, offset, old_num_op, new_num_op;
  ITintmax val;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *old_src, *new_src;

  offset = L_find_ind_initial_offset (loop->preheader, operand1, operand2,
				      NULL, NULL, loop->ind_info);
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  if (!L_is_src_operand (operand1, op))
	    continue;

	  /* temporarily create what new ld/st will be, see how many ops it
	     will expand compared to original, don't do opti if more ops */
	  if (L_load_opcode (op) || L_store_opcode (op))
	    {
	      if (!(L_same_operand (operand1, op->src[0]) ||
		    L_same_operand (operand1, op->src[1])))
		return 0;
	      for (j = 2; j < L_max_src_operand; j++)
		{
		  if (L_same_operand (operand1, op->src[j]))
		    return 0;
		}
	      if (!(L_is_int_constant (op->src[0]) ||
		    L_is_int_constant (op->src[1])))
		return 0;
	      old_num_op = M_num_oper_required_for (op, L_fn->name);
	      if (L_is_int_constant (op->src[0]))
		{
		  old_src = op->src[0];
		  new_src =
		    L_new_gen_int_operand (offset + op->src[0]->value.i);
		  op->src[0] = new_src;
		  new_num_op = M_num_oper_required_for (op, L_fn->name);
		  op->src[0] = old_src;
		}
	      else
		{
		  old_src = op->src[1];
		  new_src =
		    L_new_gen_int_operand (offset + op->src[1]->value.i);
		  op->src[1] = new_src;
		  new_num_op = M_num_oper_required_for (op, L_fn->name);
		  op->src[1] = old_src;
		}
	      L_delete_operand (new_src);
	      if (new_num_op > old_num_op)
		return 0;
	      else
		continue;
	    }

	  /* for rest ops, assume if the constant field can hold original
	     constant, it can also hold the new constant!! */
	  else if (L_move_opcode (op))
	    {
	      continue;
	    }
	  else if (L_int_add_opcode (op))
	    {
	      if (L_is_int_constant (op->src[0]) ||
		  L_is_int_constant (op->src[1]))
		continue;
	      if (L_is_register (op->dest[0]) &&
		  L_only_used_as_base_addr_with_const_offset_in_loop (loop,
								      loop_cb,
								      num_cb,
								      op->dest
								      [0],
								      offset)
		  && L_not_live_in_out_cb (out_cb, num_out_cb, op->dest[0])
		  && L_all_uses_in_loop_from (loop, loop_cb, num_cb, cb, op))
		continue;
	    }
	  else if (L_int_sub_opcode (op))
	    {
	      if (L_is_int_constant (op->src[0]) ||
		  L_is_int_constant (op->src[1]))
		continue;
	    }
	  /* SAM 2-97, added equivalent fix DIA made to
	     Lsuper_all_uses_of_ind_can_change_offset() for unsigned ints. */
	  else if (L_unsigned_int_comparative_opcode (op))
	    {
	      if (L_is_int_constant (op->src[0]))
		{
		  val = op->src[0]->value.i;
		  if (L_is_legal_unsigned_value_offset_32 ((unsigned int) val,
							   (int) -offset))
		    continue;
		}
	      if (L_is_int_constant (op->src[1]))
		{
		  val = op->src[1]->value.i;
		  if (L_is_legal_unsigned_value_offset_32 ((unsigned int) val,
							   (int) -offset))
		    continue;
		}
	    }
	  else if (L_int_comparison_opcode (op))
	    {
	      if (L_is_int_constant (op->src[0]) ||
		  L_is_int_constant (op->src[1]))
		continue;
	    }
	  else if (L_int_pred_comparison_opcode (op))
	    {
	      if (L_is_int_constant (op->src[0]) ||
		  L_is_int_constant (op->src[1]))
		continue;
	    }
	  /* allow more freedom for branches, so opti will work for loops
	     with non-constant final values */
	  else if (L_int_cond_branch_opcode (op))
	    {
	      if (L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[0]) ||
		  L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[1]))
		continue;
	    }
	  return 0;
	}
    }
  return 1;
}



/* SER: This function deals with checking if the branches for the cb can be 
 * modified with a strength reduction. The case that cannot be reduced is when a 
 * branch without a constant source follows the new loop induction variable op 
 * and depends on the former value. */
int
L_can_modify_dep_branches_in_loop (L_Loop * loop, int *loop_cb, int num_cb,
				   L_Oper * start_op, L_Cb * start_cb)
{
  int ind_flag;
  L_Oper *op;
  L_Operand *operand1;
  /* need to add i & branch_out and 
     implement branch_out in the first section to use the extensive code */

  operand1 = start_op->dest[0];
  if (operand1 == NULL)
    return 1;

  ind_flag = 0;

  for (op = start_op->next_op; op; op = op->next_op)
    {
      if (L_is_dest_operand (operand1, op)	/* &&
						   (L_is_constant(op->src[0]) || L_is_constant(op->src[1])) */ )
	return 1;
      if (L_same_operand (op->dest[0], start_op->src[0]))
	{
	  ind_flag = 1;
	  continue;
	}
      if (!ind_flag)
	continue;
      /* After this, we know the increment has occurred, so check branches. */
      if (!L_is_src_operand (operand1, op))
	continue;
      if (L_int_cond_branch_opcode (op))
	{
	  if (L_is_constant (op->src[0]) || L_is_constant (op->src[1]))
	    continue;
	  else
	    return 0;
	}
    }


  /* If it reaches this point, there are some paths from the cb which don't have
     constant offsets modifying start_op->dest[0] beforehand, so need to check 
     for them in every other loop cb. Sufficient condition is an op that 
     redefines it before a non-constant-source branch uses it. */
#if 0
  if (branch_out)
    {
      for (i = 0; i < num_cb; i++)
	{
	  L_Cb * cb;
	  cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
	  if (cb == start_cb)
	    continue;
	  for (op = cb->first_op; op != NULL; op = op->next_op)
	    {
	      /* First check if operand1 is modified by a constant later on;
	         then we only have to check for loop-invariant source branches. */
	      /* if (Set_in (loop->basic_ind_var_op, op->id))
	         continue; */
	      if (L_is_dest_operand (operand1, op))
		if (L_is_src_operand (operand1, op) &&
		    (L_is_constant (op->src[0])
		     || L_is_constant (op->src[1])))
		  break;
	      if (!L_is_src_operand (operand1, op))
		continue;

	      if (L_int_cond_branch_opcode (op))
		if (L_is_constant (op->src[0]) || L_is_constant (op->src[1]))
		  continue;
		else
		  return 0;
	    }
	}
    }
#endif

  return 1;
}

/*
 *      The other operands of operations which use the basic ind var
 *      oper are loop invariant.  Basic_ind_var_ops are excluded from
 *      this search.  Also only look at src1 and src2, src3 allowed
 *      to be variant.
 */
int
L_ind_only_used_with_loop_inv_operands (L_Loop * loop, int *loop_cb,
					int num_cb, L_Operand * operand)
{
  int i;
  L_Cb *cb;
  L_Oper *op;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  if (!L_is_src_operand (operand, op))
	    continue;
	  if (L_same_operand (operand, op->src[0]))
	    {
	      if (!L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[1]))
		return 0;
	    }
	  else if (L_same_operand (operand, op->src[1]))
	    {
	      if (!L_is_loop_inv_operand (loop, loop_cb, num_cb, op->src[0]))
		return 0;
	    }
	  /* only allow other srcs to match in case of store */
	  else if (!L_store_opcode (op))
	    return 0;
	}
    }
  return 1;
}

int
L_ind_only_used_with_memory_ops (L_Loop * loop, int *loop_cb, int num_cb,
				 L_Operand * operand)
{
  int i, j;
  L_Cb *cb;
  L_Oper *oper;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, oper->id))
	    continue;
	  if (!L_is_src_operand (operand, oper))
	    continue;
	  if (!(L_general_load_opcode (oper) ||
		L_general_store_opcode (oper)))
	    return 0;
	  for (j = 2; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (operand, oper->src[j]))
		return 0;
	    }
	}
    }

  return 1;
}


/*
 *      The other operands in instructions that use operand can be modified
 *      if oper is changed to another ind var having the SAME inc.
 */
int
L_all_uses_of_ind_can_be_modified1 (L_Loop * loop, int *loop_cb, int num_cb,
				    L_Operand * operand)
{
  int i, j, old_num_op, new_num_op;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *old_src, *new_src;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  if (!L_is_src_operand (operand, op))
	    continue;

	  /* update load if result instr doesn't require more cycles than 
	     first */
	  if (L_load_opcode (op) || L_store_opcode (op))
	    {
	      if (!(L_same_operand (operand, op->src[0]) ||
		    L_same_operand (operand, op->src[1])))
		return 0;
	      for (j = 2; j < L_max_src_operand; j++)
		{
		  if (L_same_operand (operand, op->src[j]))
		    return 0;
		}
	      old_num_op = M_num_oper_required_for (op, L_fn->name);
	      if (L_same_operand (operand, op->src[0]))
		{
		  old_src = op->src[1];
		  new_src = L_copy_operand (operand);
		  op->src[1] = new_src;
		  new_num_op = M_num_oper_required_for (op, L_fn->name);
		  op->src[1] = old_src;
		}
	      else
		{
		  old_src = op->src[0];
		  new_src = L_copy_operand (operand);
		  op->src[0] = new_src;
		  new_num_op = M_num_oper_required_for (op, L_fn->name);
		  op->src[0] = old_src;
		}
	      L_delete_operand (new_src);
	      if (new_num_op > old_num_op)
		return 0;
	      else
		continue;
	    }

	  /* for rest of ops, optimistically assume ind elim won't increase
	     the num ops required. */

	  /* The ind on unsigned cond_branch, comparison and
	     pred_comparison can't safely be changed because doing so
	     may accidentally cause on side of the condition to become
	     less than zero (a very large positive number).
	     Recomputing the original value of both sides of the
	     condition would require more instructions.  WARNING: A
	     similar rollover is possible with signed numbers but is
	     much less likely */
	  else if (L_signed_int_cond_branch_opcode (op))
	    {
	      continue;
	    }
	  else if (L_move_opcode (op))
	    {
	      continue;
	    }
	  else if (L_signed_int_comparison_opcode (op))
	    {
	      continue;
	    }
	  else if (L_signed_int_pred_comparison_opcode (op))
	    {
	      continue;
	    }
	  else if (L_int_add_opcode (op))
	    {
	      continue;
	    }
	  else if (L_int_sub_opcode (op))
	    {
	      continue;
	    }
	  else
	    {
	      return 0;
	    }
	}
    }
  return 1;
}

/*
 *      The other operands in instructions that use operand can be modified
 *      if oper is changed to another ind var having a DIFFERENT increment.
 */
int
L_all_uses_of_ind_can_be_modified2 (L_Loop * loop, int *loop_cb, int num_cb,
				    L_Operand * operand)
{
  int i;
  L_Cb *cb;
  L_Oper *op;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  if (!L_is_src_operand (operand, op))
	    continue;
	  /* assume these instr will require same num instr before and after
	     ind elim */
	  if (L_int_cond_branch_opcode (op))
	    continue;
	  if (L_int_comparison_opcode (op))
	    continue;
	  if (L_int_pred_comparison_opcode (op))
	    continue;
	  return 0;
	}
    }
  return 1;
}

/*
 *      Return 1 if better to replace oper2 with oper1
 */
int
L_better_to_eliminate_operand2 (L_Loop * loop, int *loop_cb, int num_cb,
				int *out_cb, int num_out_cb,
				L_Operand * operand1, L_Operand * operand2)
{
  int same_inc, i, j, count1, count2, flag1, flag2;
  L_Cb *cb;
  L_Oper *op;

  /* make sure can eliminate operand2 */
  if (!L_not_live_in_out_cb (out_cb, num_out_cb, operand2))
    return 0;
  if (!L_ind_only_used_with_loop_inv_operands
      (loop, loop_cb, num_cb, operand2))
    return 0;
  same_inc = L_same_ind_increment (operand1, operand2, loop->ind_info);
  if ((same_inc) &&
      (!L_all_uses_of_ind_can_be_modified1 (loop, loop_cb, num_cb, operand2)))
    return 0;
  if ((!same_inc) &&
      (!L_all_uses_of_ind_can_be_modified2 (loop, loop_cb, num_cb, operand2)))
    return 0;

  count1 = count2 = 0;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if (Set_in (loop->basic_ind_var_op, op->id))
	    continue;
	  flag1 = flag2 = 0;
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (L_same_operand (operand1, op->src[j]))
		flag1 = 1;
	      if (L_same_operand (operand2, op->src[j]))
		flag2 = 1;
	    }
	  count1 += flag1;
	  count2 += flag2;
	}
    }
  return (count2 > count1);
}

int
L_cost_effective_for_ind_complex_elim (L_Loop * loop, int *loop_cb,
				       int num_cb)
{
  int i, int_cnt, tot_cnt;
  double num_invocation, header_weight, avg_iterations, int_ratio;
  L_Cb *cb;
  L_Oper *oper;

  /* check min iteration count */
  num_invocation = loop->num_invocation;
  header_weight = loop->header->weight;
  if (num_invocation > 0.0)
    avg_iterations = header_weight / num_invocation;
  else
    avg_iterations = 0.0;
  if (avg_iterations < L_MIN_ITER_FOR_IND_COMPLEX_ELIM)
    return 0;

  /* check int vs float density */
  int_cnt = tot_cnt = 0;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_int_opcode (oper))
	    int_cnt++;
	  tot_cnt++;
	}
    }

  int_ratio = (double) int_cnt / (double) tot_cnt;
  if (int_ratio > L_MIN_RATIO_FOR_IND_COMPLEX_ELIM)
    return 1;
  else
    return 0;
}

int
L_live_at_nondominated_exits (L_Loop * loop, int *exit_cb, int num_exit_cb,
			      L_Cb * cb, L_Operand * operand)
{
  int i;
  L_Cb *curr_cb;
  L_Flow *flow;

  for (i = 0; i < num_exit_cb; i++)
    {
      curr_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, exit_cb[i]);
      if (L_in_cb_DOM_set (curr_cb, cb->id))
	continue;
      /* check liveness of all target cbs not in loop */
      for (flow = curr_cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  if (Set_in (loop->loop_cb, flow->dst_cb->id))
	    continue;
	  if (L_in_cb_IN_set (flow->dst_cb, operand))
	    return 1;
	}
    }

  return 0;
}

/*
 *      Loop var if it is:
 *              (1) increment is small integer constant
 *              (2) only used by conditional branch(es)
 */
int
L_is_loop_var (L_Loop * loop, int *loop_cb, int num_cb, L_Operand * ind_var,
	       L_Ind_Info * ind_info)
{
  int i;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *increment;

  /* (1) */
  increment = L_find_basic_induction_increment (ind_var, ind_info);
  if (!L_is_int_constant (increment))
    return 0;

  if (abs (ITicast (increment->value.i)) > 15)
    return 0;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_is_src_operand (ind_var, oper))
	    continue;
	  if (Set_in (loop->basic_ind_var_op, oper->id))
	    continue;
	  if (L_int_cond_branch_opcode (oper))
	    continue;
	  return 0;
	}
    }

  return 1;
}

/*==========================================================================*/
/*
 *      Jump optimization predicates
 */
/*==========================================================================*/

int
L_empty_block (L_Cb * cb)
{
  if (cb == NULL)
    L_punt ("L_empty_block: cb cannot be NIL");
  if (cb->first_op != NULL)
    return 0;
  if (cb->last_op != NULL)
    L_punt ("L_empty_block: empty block must have last = NULL");
  return 1;
}

int
L_only_uncond_branch_in_block (L_Cb * cb)
{
  if (cb == NULL)
    L_punt ("L_only_uncond_branch_in_block: cb cannot be NIL");
  if (!L_uncond_branch_opcode (cb->first_op))
    return 0;
  if (L_is_predicated (cb->first_op))
    return 0;
  if (cb->first_op != cb->last_op)
    L_punt ("L_only_uncond_branch_in_block: first and last must be same");
  return 1;
}

int
L_multiple_cond_branch_in_block (L_Cb * cb)
{
  int count;
  L_Oper *oper;
  if (cb == NULL)
    L_punt ("L_multiple_cond_branch_in_block: cb cannot be NIL");
  count = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_cond_branch_opcode (oper))
	{
	  count += 1;
	  if (count > 1)
	    return 1;
	}
    }
  return 0;
}

int
L_cb_contains_prologue (L_Cb * cb)
{
  L_Oper *oper;

  if (cb == NULL)
    L_punt ("L_cb_contains_prologue: cb is NULL");

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_is_opcode (Lop_PROLOGUE, oper))
	return 1;
    }

  return 0;
}

int
L_cb_contains_epilogue (L_Cb * cb)
{
  L_Oper *oper;

  if (cb == NULL)
    L_punt ("L_cb_contains_epilogue: cb cannot be NIL");

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_subroutine_return_opcode (oper))
	return 1;
    }

  return 0;
}

int
L_cb_contains_multiple_branches_to (L_Cb * cb, L_Cb * target)
{
  int count;
  L_Oper *oper;
  if (cb == NULL)
    L_punt ("L_cb_contains_multiple_branches_to: cb cannot be NIL");
  count = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_uncond_branch_opcode (oper))
	{
	  if (oper->src[0]->value.cb == target)
	    count++;
	}
      else if (L_cond_branch_opcode (oper))
	{
	  if (oper->src[2]->value.cb == target)
	    count++;
	}
      else if (L_register_branch_opcode (oper))
	{
	  L_punt ("L_bb_contains_multiple_branches_to: jump_rg not handled");
	}
    }
  return (count > 1);
}

int
L_need_fallthru_path (L_Cb * cb)
{
  L_Oper *oper;
  if (!cb)
    return 0;
  if (!(oper = cb->last_op) ||
      L_is_predicated (oper))
    return 1;

  switch (oper->opc)
    {
    case Lop_RTS:
    case Lop_RTS_FS:
    case Lop_JUMP:
    case Lop_JUMP_FS:
    case Lop_JUMP_RG:
    case Lop_JUMP_RG_FS:
      return 0;
    default:
      return 1;
    }
}


/*==========================================================================*/
/*
 *      Pre/Post increment predicate functions
 */
/*==========================================================================*/

int
L_marked_as_post_increment (L_Oper * oper)
{
  L_Attr *attr;

  if (oper == NULL)
    return 0;

  attr = L_find_attr (oper->attr, L_POST_INC_MARK);
  return (attr != NULL);
}

int
L_marked_as_pre_increment (L_Oper * oper)
{
  L_Attr *attr;

  if (oper == NULL)
    return 0;

  attr = L_find_attr (oper->attr, L_PRE_INC_MARK);
  return (attr != NULL);
}

int
L_marked_as_pre_post_increment (L_Oper * oper)
{
  return (L_marked_as_pre_increment (oper) ||
	  L_marked_as_post_increment (oper));
}

/* Added SAM 6-97 */
int
L_can_recombine_mem_inc_ops (L_Oper * mem, L_Oper * inc)
{
  L_Cb *mem_cb, *inc_cb;

  if ((mem == NULL) || (inc == NULL))
    return 0;

  mem_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, mem->id);
  inc_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, inc->id);

  if (mem_cb == inc_cb)
    {
      if (!L_no_defs_between (mem->src[0], mem, inc))
	return 0;
      if (!L_no_uses_between (mem->src[0], mem, inc))
	return 0;
      return 1;
    }
  else
    {
      /*  Right now there are no checks for this category, this may likely
         need to change in the future */
      return 1;
    }
}

int
L_can_recombine_inc_mem_ops (L_Oper * mem, L_Oper * inc)
{
  L_Cb *mem_cb, *inc_cb;

  if ((mem == NULL) || (inc == NULL))
    return 0;

  mem_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, mem->id);
  inc_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, inc->id);

  if (mem_cb == inc_cb)
    {
      if (!L_no_defs_between (mem->src[0], inc, mem))
	return 0;
      if (!L_no_uses_between (mem->src[0], inc, mem))
	return 0;
      return 1;
    }
  else
    {
      /*  Right now there are no checks for this category, this may likely
         need to change in the future */
      return 1;
    }
}

int
L_can_make_post_inc (L_Oper * mem_op, L_Oper * ind_op)
{
  L_Oper *temp_op;
  L_Operand *base, *offset, *increment;
  int new_num_oper;

  if (!(L_load_opcode (mem_op) || L_store_opcode (mem_op)))
    return 0;

  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              store. */
  if (L_EXTRACT_BIT_VAL (mem_op->flags, L_OPER_VOLATILE))
    return 0;

  if (L_same_operand (ind_op->dest[0], mem_op->src[0]))
    {
      base = mem_op->src[0];
      offset = mem_op->src[1];
    }
  else if (L_same_operand (ind_op->dest[0], mem_op->src[1]))
    {
      base = mem_op->src[1];
      offset = mem_op->src[0];
    }
  else
    {
      return 0;
    }

  increment = ind_op->src[1];
  if (L_int_sub_opcode (ind_op) && L_is_int_constant (increment))
    increment = L_new_gen_int_operand (-(increment->value.i));

  if (L_load_opcode (mem_op))
    {
      temp_op = L_create_new_op (L_corresponding_postincrement_load (mem_op));
      temp_op->dest[0] = L_copy_operand (mem_op->dest[0]);
      temp_op->dest[1] = L_copy_operand (base);
      temp_op->src[0] = L_copy_operand (base);
      temp_op->src[1] = L_copy_operand (offset);
      temp_op->src[2] = L_copy_operand (increment);
    }
  else
    {
      temp_op =
	L_create_new_op (L_corresponding_postincrement_store (mem_op));
      temp_op->dest[0] = L_copy_operand (base);
      temp_op->src[0] = L_copy_operand (base);
      temp_op->src[1] = L_copy_operand (offset);
      temp_op->src[2] = L_copy_operand (mem_op->src[2]);
      temp_op->src[3] = L_copy_operand (increment);
    }

  new_num_oper = M_num_oper_required_for (temp_op, L_fn->name);

  if (increment != ind_op->src[1])
    L_delete_operand (increment);
  L_delete_oper (NULL, temp_op);

  return (new_num_oper == 1);
}

int
L_can_make_pre_inc (L_Oper * mem_op, L_Oper * ind_op)
{
  L_Oper *temp_op;
  L_Operand *base, *offset, *increment;
  int new_num_oper;

  if (!(L_load_opcode (mem_op) || L_store_opcode (mem_op)))
    return 0;
  
  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
   *              store. */
  if (L_EXTRACT_BIT_VAL (mem_op->flags, L_OPER_VOLATILE))
    return 0;

  if (L_same_operand (ind_op->dest[0], mem_op->src[0]))
    {
      base = mem_op->src[0];
      offset = mem_op->src[1];
    }
  else if (L_same_operand (ind_op->dest[0], mem_op->src[1]))
    {
      base = mem_op->src[1];
      offset = mem_op->src[0];
    }
  else
    {
      return 0;
    }

  increment = ind_op->src[1];
  if (L_int_sub_opcode (ind_op) && L_is_int_constant (increment))
    increment = L_new_gen_int_operand (-(increment->value.i));

  if (L_load_opcode (mem_op))
    {
      temp_op = L_create_new_op (L_corresponding_preincrement_load (mem_op));
      temp_op->dest[0] = L_copy_operand (mem_op->dest[0]);
      temp_op->dest[1] = L_copy_operand (base);
      temp_op->src[0] = L_copy_operand (base);
      temp_op->src[1] = L_copy_operand (offset);
      temp_op->src[2] = L_copy_operand (increment);
    }
  else
    {
      temp_op = L_create_new_op (L_corresponding_preincrement_store (mem_op));
      temp_op->dest[0] = L_copy_operand (base);
      temp_op->src[0] = L_copy_operand (base);
      temp_op->src[1] = L_copy_operand (offset);
      temp_op->src[2] = L_copy_operand (mem_op->src[2]);
      temp_op->src[3] = L_copy_operand (increment);
    }

  new_num_oper = M_num_oper_required_for (temp_op, L_fn->name);

  if (increment != ind_op->src[1])
    L_delete_operand (increment);
  L_delete_oper (NULL, temp_op);

  return (new_num_oper == 1);
}

int
L_ind_var_is_updated_by_pre_post_increment (L_Loop * loop, int *loop_cb,
					    int num_cb, L_Operand * ind_var)
{
  int i;
  L_Cb *cb;
  L_Oper *oper;

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_is_dest_operand (ind_var, oper))
	    continue;
	  if (L_marked_as_post_increment (oper) ||
	      L_marked_as_pre_increment (oper))
	    return 1;
	}
    }

  return 0;
}

/*
 * return 1 if the destination of the oper looks like it is used in an
 * address calculation.  For architectures which support scaled address
 * propagation, the operation may be pulled into the addressing mode
 * and done "for free".  For now, assume all lsl's of 1, 2, or 3
 * are used in address calcs.
 */
int
L_oper_used_in_address_calc (L_Cb * cb, L_Oper * op)
{
  if (op->opc != Lop_LSL)
    return 0;

  if (L_is_int_constant (op->src[1]) && (op->src[1]->value.i <= 3))
    return 1;

  return 0;
}

int
L_extension_compatible_ctype (ITuint8 from, ITuint8 to)
{
  ITuint8 ufrom, uto;
  if (!L_is_ctype_int_direct (from) || !L_is_ctype_int_direct (to))
    return 0;

  /* is from-type same as to-type? */
  if (from == to)
    return 1;

  /* is from-type strictly smaller than to-type? */

  if (!(L_is_ctype_signed_direct (from) ^ L_is_ctype_signed_direct (to)) &&
      (from < to))
    return 1;

  ufrom = L_ctype_unsigned_version (from);
  uto = L_ctype_unsigned_version (to);

  if (L_is_ctype_signed_direct (to) && (ufrom < uto))
    return 1;

  return 0;
}

/*
 * Return TRUE if ext_op is redundant when applied as an
 * extension on the result of src_op
 */
int
L_redundant_extension_rev (L_Oper * src_op, L_Oper * ext_op)
{
  int src_ctype, ext_ctype;

  if (!src_op || !ext_op)
    return 0;

  if (!L_sign_or_zero_extend_opcode (ext_op))
    return 0;

  ext_ctype = L_opcode_ctype (ext_op);

  if (L_general_load_opcode (src_op) || L_sign_or_zero_extend_opcode (src_op))
    {
      src_ctype = L_opcode_ctype (src_op);

      return L_extension_compatible_ctype (src_ctype, ext_ctype);
    }

  if (L_general_comparison_opcode (src_op))
    return 1;

  if (L_move_opcode (src_op) &&
      (L_is_int_constant (src_op->src[0]) || M_zero_macro (src_op->src[0])))
    {
      ITintmax val, cval;

      if (L_is_macro (src_op->src[0]))
	return 1;

      val = src_op->src[0]->value.i;

      switch (ext_ctype)
	{
	case L_CTYPE_CHAR:
	  cval = (char) val;
	  if (cval == val)
	    return 1;
	  break;
	case L_CTYPE_UCHAR:
	  cval = (unsigned char) val;
	  if (cval == val)
	    return 1;
	  break;
	case L_CTYPE_SHORT:
	  cval = (short) val;
	  if (cval == val)
	    return 1;
	  break;
	case L_CTYPE_USHORT:
	  cval = (unsigned short) val;
	  if (cval == val)
	    return 1;
	  break;
	case L_CTYPE_INT:
	  cval = (int) val;
	  if (cval == val)
	    return 1;
	  break;
	case L_CTYPE_UINT:
	  cval = (unsigned int) val;
	  if (cval == val)
	    return 1;
	  break;
	case L_CTYPE_LONG:
	case L_CTYPE_ULONG:
	case L_CTYPE_LLONG:
	case L_CTYPE_ULLONG:
	  return 1;
	default:
	  break;
	}
    }

  return 0;
}

int
L_redundant_extension_fwd (L_Oper * ext_op, L_Oper * dst_op)
{
  int ext_ctype;

  if (!dst_op || !ext_op)
    return 0;

  if (!L_sign_or_zero_extend_opcode (ext_op))
    return 0;

  ext_ctype = L_opcode_ctype (ext_op);

  if (L_general_store_opcode (dst_op))
    {
      int first, count;

      M_get_memory_operands (&first, &count, dst_op->proc_opc);

      /* 01/14/03 REK Changing L_opcode_ctype to L_store_ctype, which returns
       *              the ctype being stored. */
      if (L_same_operand (ext_op->dest[0],
			  dst_op->src[first + count]) &&
	  (L_store_ctype (dst_op) == L_ctype_signed_version (ext_ctype)))
	return 1;
    }

  if (L_sign_or_zero_extend_opcode (dst_op) &&
      (L_opcode_ctype (dst_op) == ext_ctype))
    return 1;

  if (L_int_pred_comparison_opcode (dst_op) &&
      (L_ctype_signed_version (L_get_compare_ctype (dst_op)) ==
       L_ctype_signed_version (ext_ctype)))
    return 1;

  return 0;
}

/*
 * Branch value propagation predicate functions
 */

/* SER 040923: Used in local and global branch value propagation.
 * Finds if the first src_flow to cb corresponds to a branch testing a
 * variable equal to a constant, or is a fallthrough path of a variable
 * not equal to a constant.
 * Returns 0 if invalid, 1 if valid and writes pointers to the variable
 * and constant in that case.
 * FUTURE ENHANCEMENTS: Currently only allows BNEs before an unconditional
 *   branch; really could allow any but need to thoroughly check this.
 */
int
L_cb_first_flow_const_compare_branch (L_Cb * cb, L_Operand **var,
				      L_Operand **constant)
{
  L_Cb * src_cb;
  L_Oper * op;
  L_Flow *flow, *flow2;
  int jump_flag = 0;

  /* Find first flow and the src cb. */
  if (NULL == (flow2 = cb->src_flow))
    return 0;
  src_cb = flow2->src_cb;

  /* If not a fallthrough path, find the branch oper and check for the
   * appropriate type: must either be a BEQ, or be an unconditional branch
   * and have a non-predicated BNE prior to it. */
  if (flow2->cc != 0)
    {
      flow = L_find_matching_flow (src_cb->dest_flow, flow2);
      op = L_find_branch_for_flow (src_cb, flow);
      if (!L_uncond_branch (op))
	{
	  if (!(L_gen_beq_branch_opcode (op)))
	    return 0;
	}
      else
	{ /* cb reached by an unconditional jump,
	   * search starting at second-to-last op. */
	  while ((op = op->prev_op))
	    {
	      /* Prevent opti if code is not reachable. */
	      if (L_uncond_branch_opcode (op))
		return 0;
	      if (L_gen_bne_branch_opcode (op) && (op->pred[0] == NULL))
		break;
	    }
	  if (!op)
	    return 0;
	  jump_flag = 1;
	}
    }
  else
    { /* If a fallthrough path, must have a BNE within the cb. */
      for (op = src_cb->last_op; op; op = op->prev_op)
	{
	  /* Prevent opti if code is not reachable. */
	  if (L_uncond_branch_opcode (op))
	    return 0;
	  if (L_gen_bne_branch_opcode (op) && (op->pred[0] == NULL))
	    break;
	}
      if (!op)
	return 0;
    }

  /* Find the variable and constant compared in the branch, exit if both are
   * constants or variables. */
  if (L_is_constant (op->src[0]))
    {
      if (L_is_constant (op->src[1]))
	return 0;
      *var = op->src[1];
      *constant = op->src[0];
    }
  else if (L_is_constant (op->src[1]))
    {
      *var = op->src[0];
      *constant = op->src[1];
    }
  else
    return 0;

  /* For fallthrough and jump cases, need to check that var isn't redefined
   * between op and the unconditional branch that leads to cb (which
   * should be the last op). */
  if ((flow2->cc == 0) || jump_flag)
    {
      if (!(L_no_defs_in_range (*var, op, src_cb->last_op)))
	return 0;
    }
  return 1;
}


/* SER 040923: Used in local and global branch value propagation.
 * Given L_Operands var and constant, checks that all branches leading to
 * cb require var and constant to be equal.  This can either be in itself
 * (BEQ) or a fall-through or other type of branch with a preceeding
 * BNE op.
 * FUTURE ENHANCEMENTS: Currently only allows BNEs before an unconditional
 *   branch; really could allow any but need to thoroughly check this.
 */
int
L_cb_all_incoming_flows_same_const_compare (L_Cb *cb, L_Operand *var,
					    L_Operand *constant)
{
  L_Cb * src_cb;
  L_Oper *op;
  L_Flow * flow, * flow2;
  int jump_flag;

  if (NULL == (flow2 = cb->src_flow))
    return 0;

  do {
    jump_flag = 0;
    src_cb = flow2->src_cb;

    if (flow2->cc != 0)
      { /* Branch for flow, check for BEQ of constant and var, or
	 * flag as an unconditional branch. */
	flow = L_find_matching_flow (src_cb->dest_flow, flow2);
	op = L_find_branch_for_flow (src_cb, flow);
	/* Conditional branch case. */
	if (!L_uncond_branch (op))
	  {
	    if (!L_gen_beq_branch_opcode (op))
	      return 0;
	    if (!((L_same_operand (var, op->src[0]) &&
		   L_same_operand (constant, op->src[1])) ||
		  (L_same_operand (var, op->src[1]) &&
		   L_same_operand (constant, op->src[0]))))
	      return 0;
	  }
	else
	  { /* Unconditional branch case. */
	    jump_flag = 1;
	    if (!(op = op->prev_op)) /* Start checking at previous op. */
	      return 0;
	  }
      }
    else
      { /* Fall-through base, start checking at src_cb's last op. */
	if (!(op = src_cb->last_op))
	  return 0;
      }

    /* For fall-through and unconditional branch cases, find a
     * non-predicated BNE op that preceeds the unconditional branch and
     * satisfies the condition. */
    if (flow2->cc == 0 || jump_flag)
      {
	do
	  {
	    if (L_is_dest_operand (var, op))
	      { /* If an explicit move of const to var, it's OK. */ 
		if (L_move_opcode (op) && op->pred[0] == NULL &&
		    L_same_operand (op->src[0], constant))
		  break;
		else
		  return 0;
	      }
	    /* Prevent opti if code is not reachable. */
	    if (L_uncond_branch_opcode (op))
	      return 0;
	    if (!(L_gen_bne_branch_opcode (op) && op->pred[0] == NULL))
	      continue;
	    if ((L_same_operand (var, op->src[0]) &&
		 L_same_operand (constant, op->src[1])) ||
		(L_same_operand (var, op->src[1]) &&
		 L_same_operand (constant, op->src[0])))
	      break;
	  } while ((op = op->prev_op));
	if (!op)
	  return 0;
      }
  } while ((flow2 = flow2->next_flow));

  return 1;
}
