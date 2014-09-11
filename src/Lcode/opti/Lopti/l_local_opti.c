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
 *      File :          l_local_opti.c
 *      Description :   cb level optimization (basic block, superblock, 
 *                      hyperblock)
 *      Info Needed :   live variable analysis
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 02/07/03 REK Modifying L_local_memory_copy_propagation,
 *              L_local_common_subexpression, L_local_redundant_load, 
 *              L_local_redundant_store, L_local_constant_combining,
 *              L_local_operation_folding, L_local_code_motion,
 *              L_local_oper_recombine, so that they don't touch opers marked
 *              volatile.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

#define ERR stderr

#undef TEST_LOCAL_OPTI
#undef TEST_CONST_PROP
#undef TEST_COPY_PROP1
#undef TEST_COPY_PROP2
#undef TEST_MEM_COPY_PROP
#undef TEST_COMMON_SUB
#undef TEST_RED_LOAD
#undef TEST_RED_STORE
#undef TEST_CONST_FOLD
#undef TEST_CONST_COMBINE
#undef TEST_STR_RED
#undef TEST_OP_FOLD
#undef TEST_BR_FOLD
#undef TEST_OP_CANCEL
#undef TEST_DEAD_CODE
#undef TEST_CODE_MOTION
#undef TEST_REMOVE_SIGN_EXT
#undef TEST_REG_RENAMING
#undef TEST_OP_MIG
#undef TEST_RED_LOGIC

#undef TEST_OPER_BRKDWN
#undef TEST_OPER_RECOMBINE

#undef TEST_BRANCH_VAL_PROP

#define Lopti_copy_prop_incl_sz_ext 1

/*=========================================================================*/
/*
 *      Local optimizations
 */
/*=========================================================================*/

/*
 * pattern:      mov x = C
 *               use z = w,x
 */

int
L_local_constant_propagation (L_Cb * cb, int immed_only)
{
  int i, change, old_num_oper, new_num_oper;
  L_Oper *opA, *opB;
  L_Operand *old_src;

  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       *        match pattern
       */
      if (!L_move_opcode (opA))
	continue;
      if (!L_is_constant (opA->src[0]))
	continue;
      if (immed_only && (opA->src[0]->type != L_OPERAND_IMMED))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *    match pattern
	   */

	  if (!L_is_src_operand (opA->dest[0], opB))
	    continue;
	  if (!L_can_change_src_operand (opB, opA->dest[0]))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    continue;
          /* KVM : Do not substitute constants if opB has the do_not_constant_fold
           * flag.
           */
          if(L_find_attr(opB->attr, "do_not_constant_fold")) {
            // printf("Continuing for op %d\n", opB->id);
            continue;
          }
          if(opB->opc == Lop_ADD_CARRY || opB->opc == Lop_ADD_CARRY_U ||
             opB->opc == Lop_SUB_CARRY || opB->opc == Lop_SUB_CARRY_U ||
             opB->opc == Lop_MUL_WIDE || opB->opc == Lop_MUL_WIDE_U)
            continue;
	  macro_flag = L_is_fragile_macro (opA->dest[0]);
	  load_flag = 0;
	  store_flag = 0;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;
	  /*
	   *    replace pattern.
	   */
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_same_operand (opA->dest[0], opB->src[i]))
		{
		  old_num_oper = M_num_oper_required_for (opB, L_fn->name);
		  old_src = opB->src[i];
		  opB->src[i] = L_copy_operand (opA->src[0]);
		  if ((opB->src[i]->type == L_OPERAND_IMMED) &&
		      ((L_unsigned_int_opcode (opB) ||
			L_shift_opcode (opB)) &&
		       L_native_machine_ctype == L_CTYPE_INT))
		    opB->src[i]->value.i = ((ITuint32) opB->src[i]->value.i);
		  new_num_oper = M_num_oper_required_for (opB, L_fn->name);
		  /* if arch cannot handle opB after const prop, undo it */
		  if (new_num_oper > old_num_oper)
		    {
		      L_delete_operand (opB->src[i]);
		      opB->src[i] = old_src;
		      continue;
		    }
		  else
		    {
		      L_delete_operand (old_src);
#ifdef TEST_CONST_PROP
		      fprintf (ERR,
			       "const prop: op%d -> op%d, src %d (cb %d)\n",
			       opA->id, opB->id, i, cb->id);
#endif
		      change++;
		    }
		}
	    }
	}
    }
  return change;
}


/*
 * pattern:      mov x = y
 *               use z = w,x
 */

int
L_local_copy_propagation (L_Cb * cb)
{
  int i, change;
  int opa_is_sz_ext;
  L_Oper *opA, *opB;
  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       *        match pattern
       */
      if (!(L_move_opcode (opA) || (Lopti_copy_prop_incl_sz_ext &&
				    L_sign_or_zero_extend_opcode (opA))))
	continue;
      opa_is_sz_ext = L_sign_or_zero_extend_opcode (opA);

      if (!L_is_variable (opA->src[0]))
	continue;
      if (L_has_unsafe_macro_operand (opA))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *    match pattern
	   */
	  if (!L_is_src_operand (opA->dest[0], opB))
	    continue;
	  if (opa_is_sz_ext && !L_redundant_extension_fwd (opA, opB))
	    continue;
	  if (L_is_macro (opA->src[0]) && M_subroutine_call (opB->opc))
	    continue;
	  if (!L_can_change_src_operand (opB, opA->dest[0]))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    continue;
	  if (!L_same_def_reachs (opA->src[0], opA, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;
	  /*
	   *    replace pattern.
	   */
#ifdef TEST_COPY_PROP1
	  fprintf (ERR, "copy prop: op%d -> op%d, src %d (cb %d)\n",
		   opA->id, opB->id, i, cb->id);
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_same_operand (opA->dest[0], opB->src[i]))
		{
		  L_delete_operand (opB->src[i]);
		  opB->src[i] = L_copy_operand (opA->src[0]);
		  change++;
#ifdef TEST_COPY_PROP1
		  fprintf (ERR, " becomes\n");
		  L_print_oper (stderr, opA);
		  L_print_oper (stderr, opB);
#endif
		}
	    }
	}
    }

  return change;
}


/*
 * pattern before:
 *     A: use w = x,y
 *     B: mov z = w
 * pattern after:
 *     A: use z = x,y
 *     B: mov w = z
 */

int
L_local_rev_copy_propagation (L_Cb * cb)
{
  int i, change, redef, redo_flow, redef_w, redef_z, bad_redef;
  L_Oper *opA, *opB, *ptr, *new_op;
  change = 0;
  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      if (L_is_opcode (Lop_DEFINE, opA))
	continue;
      for (opB = opA->next_op; opB; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *    match pattern
	   */
	  if (!(L_move_opcode (opB) ||
		(Lopti_copy_prop_incl_sz_ext &&
		 L_sign_or_zero_extend_opcode (opB) &&
		 L_redundant_extension_rev (opA, opB))))
	    continue;
	  if (L_is_macro (opB->src[0]))
	    continue;
	  if (L_has_unsafe_macro_operand (opB))
	    continue;
	  if (!L_is_dest_operand (opB->src[0], opA))
	    continue;
	  if (!L_can_change_dest_operand (opA, opB->dest[0]))
	    continue;
	  if (!L_no_defs_between (opB->src[0], opA, opB))
	    continue;
	  if (!PG_equivalent_predicates_ops (opA, opB))
	    continue;
	  if (L_is_macro (opB->dest[0]))
	    if (!L_single_use_of (cb, opB->src[0], opA))
	      continue;
	  if (!L_not_live_at_cb_end (cb, opA, opB->src[0]))
	    continue;
	  if (!L_same_def_reachs (opB->dest[0], opA, opB))
	    continue;
	  /* Note that the next two checks, if this were reorganized to look
	   * at opB first, could short-circuit the loop. Reference Mopti-64, 
	   * M_local_rev_copy_propagation for how to do so. */
	  if (!L_no_uses_between (opB->dest[0], opA, opB))
	    continue;
	  if (!L_no_defs_between (opB->dest[0], opA, opB))
	    continue;
	  if (!L_no_uses_between (opB->src[0], opA, opB))
	    continue;
	  if (!L_can_change_all_later_uses_with (cb, opB->src[0],
						 opB->dest[0], opB))
	    continue;
	  if (!L_no_intersecting_br_between (opA, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opB->dest[0]) ||
			L_is_fragile_macro (opB->src[0]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;

	  /*
	   * Avoid pattern:
	   *     A: use w = x,y
	   *     B: mov z = w
	   *        ...
	   *        z = ?
	   *          = w
	   */

	  /*  DIA, JWS - This checks for a redef on either operand
	   * which would require placing a move instruction in the
	   * current cb.  In this case the optimization is not
	   * performed.  
	   */
	  redef_w = redef_z = bad_redef = 0;
	  for (ptr = opB->next_op; ptr; ptr = ptr->next_op)
	    {
	      if (PG_disjoint_predicates_ops (opB, ptr))
		continue;
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (L_same_operand (opB->src[0], ptr->dest[i]))
		    {
		      if (PG_superset_predicate_ops (ptr, opA))
			{
			  redef_w = 1;	/* w is clobbered */
			  break;
			}
		      else
			bad_redef = 1;	/* Can't handle partial redefine of w */
		    }
		  if (L_same_operand (opB->dest[0], ptr->dest[i]))
		    redef_z = 1;
		  /* Once z is redefined, need to check for further uses of
		   * w which would require a mov w,z in this cb
		   */
		}
	      if (redef_z)
		{
		  if (L_general_branch_opcode (ptr) ||
		      L_check_branch_opcode (ptr))
		    {
		      if (L_in_oper_OUT_set
			  (cb, ptr, opB->src[0], TAKEN_PATH))
			{
			  bad_redef = 1;
			  break;
			}
		    }
		  for (i = 0; i < L_max_src_operand; i++)
		    {
		      if (L_same_operand (opB->src[0], ptr->src[i]))
			{
			  bad_redef = 1;
			  break;
			}
		    }
		}
	      if (bad_redef)
		break;

	      if (redef_w)
		break;
	    }
	  if (bad_redef)
	    continue;

	  /*
	   *    replace pattern.
	   *
	   *    Change all later uses of opB->src[0] to opB->dest[0],
	   *    stop when opB->src[0] is redefined.  Also, at any
	   *    branch oper which opB->src[0] is live, insert
	   *    opB->src[0] <- opB->dest[0] to ensure correct
	   *    execution.  
	   */
#ifdef TEST_COPY_PROP2
	  fprintf (ERR, "rev copy prop op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif
	  redef = 0;
	  redo_flow = 0;
	  for (ptr = opA->next_op; ptr; ptr = ptr->next_op)
	    {
	      if (ptr == opB)
		continue;
	      if (L_general_branch_opcode (ptr) ||
		  L_check_branch_opcode (ptr))
		{
		  if (L_in_oper_OUT_set (cb, ptr, opB->src[0], TAKEN_PATH))
		    {
		      redo_flow = 1;
		      new_op = L_create_new_op (opB->opc);
		      new_op->dest[0] = L_copy_operand (opB->src[0]);
		      new_op->src[0] = L_copy_operand (opB->dest[0]);
		      new_op->pred[0] = L_copy_operand (opB->pred[0]);
		      L_insert_op_at_dest_of_br (cb, ptr, new_op, 0);
#ifdef TEST_COPY_PROP2
		      fprintf (ERR,
			       "placing mov op %d at dest of br op %d\n",
			       new_op->id, ptr->id);
		      L_print_oper (stderr, new_op);
#endif
		    }
		}
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (L_same_operand (opB->src[0], ptr->src[i]) &&
		      PG_subset_predicate_ops (ptr, opB))
		    {
		      L_delete_operand (ptr->src[i]);
		      ptr->src[i] = L_copy_operand (opB->dest[0]);
		    }
		}
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (L_same_operand (opB->src[0], ptr->dest[i]) &&
		      PG_subset_predicate_ops (ptr, opB))
		    redef = 1;
		}
	      if (redef)
		break;
	    }
	  /* 8/14/96, Bob McGowan */
	  if (!ptr)
	    {
	      /* If the pointer is NULL, then we know the above loop went
	         through the entire cb.
	         Check the live variables through the fall through path.
	         If the src of the mov is live out, then we must put
	         mov src = dest in the fall though block */

	      /*  if( L_in_oper_OUT_set( cb, cb->last_op, opB->src[0],
	         FALL_THRU_PATH )) */

	      if ((!L_general_branch_opcode (cb->last_op)) &&
		  (!L_check_branch_opcode (cb->last_op)) &&
		  (!L_subroutine_return_opcode (cb->last_op)) &&
		  (L_in_cb_IN_set (cb->next_cb, opB->src[0])))
		{
		  redo_flow = 1;
		  new_op = L_create_new_op (opB->opc);
		  new_op->dest[0] = L_copy_operand (opB->src[0]);
		  new_op->src[0] = L_copy_operand (opB->dest[0]);
		  L_insert_op_at_fallthru_dest (cb, new_op, 0);
#ifdef TEST_COPY_PROP2
		  fprintf (ERR, "placing mov op %d at fall thru\n",
			   new_op->id);
		  L_print_oper (stderr, new_op);
#endif
		}
	    }
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!L_same_operand (opA->dest[i], opB->src[0]))
		continue;
	      L_delete_operand (opA->dest[i]);
	      opA->dest[i] = L_copy_operand (opB->dest[0]);
	    }
	  L_nullify_operation (opB);

	  if (redo_flow)
	    {
	      L_do_flow_analysis (L_fn, LIVE_VARIABLE);
	      PG_setup_pred_graph (L_fn);
	    }
	  change++;
#ifdef TEST_COPY_PROP2
	  fprintf (ERR, "becomes\n");
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif
	}
    }
  return change;
}


int
L_local_memory_same_location (L_Oper * opA, L_Oper * opB)
{
  L_Attr *attrA, *attrB;

  /* 
   * Check to see if there is an exact match on src0 of
   * operations A and B.
   */
  if (L_same_operand (opA->src[0], opB->src[0]) &&
      L_same_operand (opA->src[1], opB->src[1]) &&
      L_same_def_reachs (opA->src[0], opA, opB) &&
      L_same_def_reachs (opA->src[1], opA, opB))
    {
      return 1;
    }
  else if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_LABEL_REFERENCE) &&
	   L_EXTRACT_BIT_VAL (opB->flags, L_OPER_LABEL_REFERENCE) &&
	   (attrA = L_find_attr (opA->attr, "label")) &&
	   (attrB = L_find_attr (opB->attr, "label")) &&
	   L_same_operand (attrA->field[0], attrB->field[0]))
    {
      /* Accesses are relative to the same label -- check offsets */

      if (attrA->max_field > 1 && attrB->max_field > 1 &&
	  attrA->field[1] && attrB->field[1] &&
	  L_same_operand (attrA->field[1], attrB->field[1]))
	return 1;
    }
  else if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_STACK_REFERENCE) &&
	   L_EXTRACT_BIT_VAL (opB->flags, L_OPER_STACK_REFERENCE) &&
	   (attrA = L_find_attr (opA->attr, "stack")) &&
	   (attrB = L_find_attr (opB->attr, "stack")) &&
	   L_same_operand (attrA->field[0], attrB->field[0]))
    {
      /* Accesses are relative to the same stack reg -- check offsets */

      if (attrA->max_field > 1 && attrB->max_field > 1 &&
	  attrA->field[1] && attrB->field[1] &&
	  L_same_operand (attrA->field[1], attrB->field[1]))
	return 1;
    }

  return 0;
}

extern int
L_classify_overlap (int offset1, int size1, int offset2, int size2,
		    int *p_indep, int *p_dep, int *p_pdep);

static int subset_offset;

int
L_local_memory_subset_access (L_Oper * opA, L_Oper * opB)
{
  L_Attr *attrA, *attrB;

  ITintmax ofstA = 0, ofstB = 0;
  int sizeA, sizeB;
  int dep = 0;

  sizeA = L_memory_access_size (opA);
  sizeB = L_memory_access_size (opB);

  subset_offset = 0;

  if (sizeA < sizeB)
    return 0;

  /* 
   * Check to see if there is an exact match on src0 of
   * operations A and B.
   */
  if (L_same_operand (opA->src[0], opB->src[0]) &&
      L_same_def_reachs (opA->src[0], opA, opB))
    {
      if (L_same_operand (opA->src[1], opB->src[1]) &&
	  L_same_def_reachs (opA->src[1], opA, opB))
	{
	  return 1;
	}
      else if (L_is_int_constant (opA->src[1]) &&
	       L_is_int_constant (opB->src[1]))
	{
	  ofstA = opA->src[1]->value.i;
	  ofstB = opB->src[1]->value.i;
	}
      else
	{
	  return 0;
	}
    }
  else if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_LABEL_REFERENCE) &&
	   L_EXTRACT_BIT_VAL (opB->flags, L_OPER_LABEL_REFERENCE) &&
	   (attrA = L_find_attr (opA->attr, "label")) &&
	   (attrB = L_find_attr (opB->attr, "label")) &&
	   L_same_operand (attrA->field[0], attrB->field[0]))
    {
      /* Accesses are relative to the same label -- check offsets */

      if (attrA->max_field <= 1 || attrB->max_field <= 1 ||
	  !attrA->field[1] || !attrB->field[1])
	{
	  return 0;
	}
      else if (L_same_operand (attrA->field[1], attrB->field[1]))
	{
	  return 1;
	}
      else if (L_is_int_constant (attrA->field[1]) &&
	       L_is_int_constant (attrB->field[1]))
	{
	  ofstA = attrA->field[1]->value.i;
	  ofstB = attrB->field[1]->value.i;
	}
      else
	{
	  return 0;
	}
    }
  else if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_STACK_REFERENCE) &&
	   L_EXTRACT_BIT_VAL (opB->flags, L_OPER_STACK_REFERENCE) &&
	   (attrA = L_find_attr (opA->attr, "stack")) &&
	   (attrB = L_find_attr (opB->attr, "stack")) &&
	   L_same_operand (attrA->field[0], attrB->field[0]))
    {
      /* Accesses are relative to the same stack reg -- check offsets */

      if (attrA->max_field <= 1 || attrB->max_field <= 1 ||
	  !attrA->field[1] || !attrB->field[1])
	{
	  return 0;
	}
      else if (L_same_operand (attrA->field[1], attrB->field[1]))
	{
	  return 1;
	}
      else if (L_is_int_constant (attrA->field[1]) &&
	       L_is_int_constant (attrB->field[1]))
	{
	  ofstA = attrA->field[1]->value.i;
	  ofstB = attrB->field[1]->value.i;
	}
      else
	{
	  return 0;
	}
    }
  else
    {
      return 0;
    }

  L_classify_overlap (ofstA, sizeA, ofstB, sizeB, NULL, &dep, NULL);

  subset_offset = ofstB - ofstA;

  return (dep == 1);
}


int
L_local_memory_copy_propagation (L_Cb * cb)
{
  int change, select;
  L_Oper *opA, *opB, *next_opB, *new_op;
  L_Operand *src, *dest;

  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_store_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = next_opB)
	{
	  int macro_flag, load_flag, store_flag, partial_def = 0;

	  /* Get next opB before possibly deleting this opB */
	  next_opB = opB->next_op;

	  /*
	   *  match pattern
	   */
	  if (!L_load_opcode (opB))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_local_memory_subset_access (opA, opB))
	    continue;
	  select = 0;
	  if (!L_compatible_load_store (opA, opB) ||
	      !L_local_memory_same_location (opA, opB))
	    {
#ifdef TEST_MEM_COPY_PROP
	      fprintf (stderr, "found one -- op %d %d\n", opA->id, opB->id);
#endif
#if 1
	      if (!L_int_store_opcode (opA) || !L_int_load_opcode (opB))
		continue;

	      /* They use an extract operation to fix up the code if
		 select is true. ntclark */
	      if( ! M_oper_supported_in_arch(Lop_EXTRACT))
		continue;

	      select = 1;
#else
	      continue;
#endif
	    }
	  if (!PG_superset_predicate_ops (opA, opB))
	    {
	      /* Allow the store to be a subset of the load,
	       * with copy of load above store in replacement
	       * pattern
	       */
	      if (PG_superset_predicate_ops (opB, opA))
		partial_def = 1;
	      else
		continue;
	    }
          if (!L_same_def_reachs (opA->src[2], opA, opB))
            continue;
          if (!L_no_overlap_write (cb, opA, opA, 0, opB, 1))
            break;
          macro_flag = (L_is_fragile_macro (opA->src[0]) ||
                        L_is_fragile_macro (opA->src[1]) ||
			L_is_fragile_macro (opB->dest[0]));
          /* Should not need to check src[2] of store */

          load_flag = 1;
          store_flag = 0;
          if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
            break;

	  if (partial_def)
	    {
	      /*
	       * Only handle certain cases -- too complex otherwise
	       */

	      if (L_is_macro (opA->src[2]))
		continue;
	      if (L_is_src_operand (opB->dest[0], opA))
		continue;
	      if (!L_same_def_reachs (opB->pred[0], opA, opB))
		continue;
	      if (!L_no_uses_between (opB->dest[0], opA, opB))
		continue;
	      if (!L_no_defs_between (opB->dest[0], opA, opB))
		continue;
	      if (!L_no_defs_between (opB->src[0], opA, opB))
		continue;
	      if (!L_no_defs_between (opB->src[1], opA, opB))
		continue;
	      if (!L_not_live_outside_cb_between (cb, opB->dest[0], opA, opB))
		continue;
	      if (select)
		continue;
	      if (!L_no_overlap_write (cb, opA, opA, 0, opB, 1))
		break;	
	    }

          /*
           *  replace pattern.
           */
#ifdef TEST_MEM_COPY_PROP
          fprintf (ERR, "memory copy prop: op%d -> op%d (cb %d)\n",
                   opA->id, opB->id, cb->id);
#endif

	  /* If store was predicated, need to insert a copy of the load
	   * prior to the store to guarantee total availability of result.
	   * later red load elim should get rid of excess loads created.
	   */
	  if (partial_def)
	    {
	      L_Oper *new_load;
	      new_load = L_copy_operation(opB);
	      L_insert_oper_before(cb, opA, new_load);
	      /* SER 8/20/03: If the operation is speculative, we need to
	       * migrate check info. Otherwise, if there is an intervening
	       * control op we need to make the copy speculative. */
	      /* One question: if the control op is a check, should we be
	       * doing all this, or in this manner? */
	      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_MASK_PE))
		{
		  L_assign_all_checks (L_fn, opB, new_load);
		}
	      else if (!L_no_control_oper_between (opA, opB) &&
		       !(L_EXTRACT_BIT_VAL (new_load->flags, L_OPER_SAFE_PEI)))
		{
		  new_load->flags = L_SET_BIT_FLAG (new_load->flags,
						    L_OPER_SPECULATIVE);
		  new_load->flags = L_SET_BIT_FLAG (new_load->flags,
						    L_OPER_MASK_PE);
		  L_global_insert_check_before (new_load, cb, opB);
		}
	    }

	  if (L_is_macro (opA->src[2]))
	    {
	      dest = L_new_register_operand (++L_fn->max_reg_id,
					     L_return_old_ctype (opA->src[2]),
					     L_PTYPE_NULL);
	      src = opA->src[2];

	      /* change opA to read from a new register */
	      opA->src[2] = L_copy_operand (dest);

	      /* add new move before opA */
	      new_op = L_create_move_using (dest, src, opA);
	      L_insert_oper_before (cb, opA, new_op);
	      STAT_COUNT ("L_local_memory_copy_propagation_move", 1, cb);
	      change++;
	    }

	  /* Check for signed store to unsigned load */
	  /* Need to create a zero extend operation */

	  if (select)
	    {
	      dest = L_copy_operand (opB->dest[0]);
	      src = L_copy_operand (opA->src[2]);
	      L_convert_to_extract (opB, dest, src, subset_offset);
	      STAT_COUNT ("L_local_memory_copy_propagation_extr", 1, cb);
	      change++;
	    }
	  else if (L_load_store_sign_extend_conflict (opA, opB))
	    {
	      dest = L_copy_operand (opB->dest[0]);
	      src = L_copy_operand (opA->src[2]);
	      L_convert_to_zero_extend_oper (opB, dest, src);
	      STAT_COUNT ("L_local_memory_copy_propagation_zext", 1, cb);
	      change++;
	    }
	  else
	    {
#if 1
	      L_convert_to_extended_move (opB,
					  L_copy_operand (opB->dest[0]),
					  L_copy_operand (opA->src[2]));
	      STAT_COUNT ("L_local_memory_copy_propagation_con_move", 1,
			  cb);
#else
	      /* Convert to move if using different registers, otherwise
	       * just delete load.  Important for hyperblock spill code
	       * optimizations. -JCG 10/17/95
	       */

	      if (!L_same_operand (opB->dest[0], opA->src[2]))
		{
		  L_convert_to_move (opB,
				     L_copy_operand (opB->dest[0]),
				     L_copy_operand (opA->src[2]));
		  STAT_COUNT ("L_local_memory_copy_propagation_con_move", 1,
			      cb);
		}
	      else
		{
		  L_delete_oper (cb, opB);
		  opB = NULL;
		  STAT_COUNT ("L_local_memory_copy_propagation_del_load", 1,
			      cb);
		}
#endif
	      change++;
	    }

	  if (partial_def && opB)
	    {
	      if (opB->pred[0])
		L_delete_operand (opB->pred[0]);

	      opB->pred[0] = L_copy_operand (opA->pred[0]);
	      STAT_COUNT ("L_local_memory_copy_propagation_pred", 1, cb);
	      change++;
	    }
	  /* change++; old location of change didn't separate occurrences */
	}
    }

  return change;
}


int
L_local_common_subexpression (L_Cb * cb, int move_flags)
{
  int i, change, new_opc;
  L_Oper *opA, *opB, *new_op;

  change = 0;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       *      match pattern
       */

      if (L_intrinsic_opcode (opA))
	{
	  if (!L_intrinsic_is_opti_enabled (opA, "LocalCommonSubexpression"))
	    continue;
	}
      else if (L_general_arithmetic_opcode (opA) ||
	       L_general_move_opcode (opA))
	{
	  ;
	}
      else
	{
	  continue;
	}

      if (!L_different_src_and_dest_operands (opA))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern
	   */
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_compatible_opcodes (opA, opB))
	    continue;
	  if (!L_same_src_operands (opA, opB))
	    continue;
	  if (!L_different_src_and_dest_operands (opB))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  /* Don't want the register/macro that will
	     carry the value from opA to opB to be unsafe. MCM */
	  if (L_has_unsafe_macro_dest_operand (opA))
	    continue;
	  if (L_has_unsafe_macro_dest_operand (opB))
	    continue;
	  if (!L_all_src_operand_same_def_reachs (opA, opA, opB))
	    continue;
	  if (!L_all_dest_operand_no_defs_between (opA, opA, opB))
	    continue;

	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = 0;
	  store_flag = 0;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;
	  /*
	   *  replace pattern.
	   */

	  /* Need to delete pred[1] of opA, since it no longer applies if
	   * it is not a superset of opB->pred[0]. */
	  if (opA->pred[1])
	    {
	      L_delete_operand (opA->pred[1]);
	      opA->pred[1] = NULL;
	    }

	  /* if opA and opB identical, just delete opB */
	  if (L_same_dest_operands (opA, opB))
	    {
	      L_nullify_operation (opB);
	      STAT_COUNT ("L_local_common_subexpression_del", 1, cb);
	    }
	  /* move opcode, only optimize if move_flag set */
	  else if (L_general_move_opcode (opA))
	    {
	      if (move_flags == 0)
		continue;
	      /* this check fixes stupid infinite loop running into */
	      if (L_is_macro (opB->dest[0]))
		continue;
	      if (L_is_int_constant (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_INT_CONSTANT))
		    continue;
		}
	      else if (L_is_flt_constant (opA->src[0]) ||
		       L_is_dbl_constant (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_FLT_CONSTANT))
		    continue;
		}
	      else if (L_is_label (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT))
		    continue;
		}
	      else if (L_is_string (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT))
		    continue;
		}
	      else
		{
		  continue;
		}
	      L_delete_operand (opB->src[0]);
	      opB->src[0] = L_copy_operand (opA->dest[0]);
	      STAT_COUNT ("L_local_common_subexpression_mov_opr", 1, cb);
	    }
	  /* arithmetic op: if only 1 dest operand, just reuse opB for
	     move oper */
	  else if (L_num_dest_operand (opA) == 1)
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (opA->dest[i] != NULL)
		    break;
		}
	      if (!opA->dest[i])
		L_punt ("L_local_common_subexpression: "
			"all dest of opA are NULL");
	      L_convert_to_move (opB, L_copy_operand (opB->dest[i]),
				 L_copy_operand (opA->dest[i]));
	      STAT_COUNT ("L_local_common_subexpression_mov_1", 1, cb);
	    }
	  else
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (!opA->dest[i] && !opB->dest[i])
		    continue;
		  if (!opA->dest[i] || !opB->dest[i])
		    L_punt ("L_common_subexpression: illegal op");

		  /* In tahoe, for example, can have pred dests
		   * on div operations
		   */
		  if (!L_is_ctype_predicate (opB->dest[i]))
		    {
		      new_opc =
			L_move_from_ctype (L_return_old_ctype (opB->dest[i]));
		      new_op = L_create_new_op (new_opc);
		      new_op->dest[0] = L_copy_operand (opB->dest[i]);
		      new_op->src[0] = L_copy_operand (opA->dest[i]);
		    }
		  else
		    {
		      new_op = L_new_pred_copy (opB->dest[i], opA->dest[i]);
		    }
		  L_insert_oper_after (cb, opB, new_op);
		  STAT_COUNT ("L_local_common_subexpression_ins", 1, cb);
		}
	      L_nullify_operation (opB);
	    }

#ifdef TEST_COMMON_SUB
	  fprintf (ERR, "common sub expr: op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  change++;
	}
    }
  return change;
}


int
L_local_redundant_load (L_Cb * cb)
{
  int i, change, new_opc;
  L_Oper *opA, *opB, *new_op;
  change = 0;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_load_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern
	   */
	  if (!L_same_opcode (opA, opB))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;

	  if (!L_local_memory_same_location (opA, opB))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (L_has_unsafe_macro_dest_operand (opB))
	    continue;
	  if (!L_all_dest_operand_no_defs_between (opA, opA, opB))
	    continue;
	  if (!L_no_overlap_write (cb, opA, opA, 0, opB, 1))
	    break;
	  /* Due to promotion, opA doesn't always have a
	     sync arc to the store, so check from opB too. MCM 5/10/2000 */
	  if (!L_no_overlap_write (cb, opB, opA, 0, opB, 1))
	    break;
	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = 1;
	  store_flag = 0;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;
	  /*
	   *  replace pattern.
	   */
#ifdef TEST_RED_LOAD
	  fprintf (ERR, "redundant load elim op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
	  L_print_oper (ERR, opA);
	  L_print_oper (ERR, opB);
#endif
	  /* if only 1 dest operand, just reuse opB for move oper */
	  if (L_num_dest_operand (opA) == 1)
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (opA->dest[i] != NULL)
		    break;
		}
	      if (opA->dest[i] == NULL)
		L_punt ("L_local_redundant_load: all dest of opA are NULL");
	      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_MASK_PE))
		{
		  if (!L_EXTRACT_BIT_VAL (opB->flags, L_OPER_MASK_PE))
		    {
		      /* Insert a check of the one being removed is
		       *  not speculative 
		       */
		      L_global_insert_check_before (opA, cb, opB);
		    }
		  else
		    {
		      /* OpA needs to "adopt" all of OpB's checks
		       */
		      L_assign_all_checks (L_fn, opB, opA);
		    }
		}
	      L_convert_to_move (opB, L_copy_operand (opB->dest[i]),
				 L_copy_operand (opA->dest[i]));

#ifdef TEST_RED_LOAD
	      fprintf (ERR, " becomes\n");
	      L_print_oper (ERR, opA);
	      L_print_oper (ERR, opB);
#endif
	    }
	  else
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (!opA->dest[i] && !opB->dest[i])
		    continue;
		  if (!opA->dest[i] || !opB->dest[i])
		    L_punt ("L_redundant_load: illegal op");
		  new_opc =
		    L_move_from_ctype (L_return_old_ctype (opB->dest[i]));
		  new_op = L_create_new_op (new_opc);
		  L_insert_oper_after (cb, opB, new_op);
		  new_op->dest[0] = L_copy_operand (opB->dest[i]);
		  new_op->src[0] = L_copy_operand (opA->dest[i]);
		}
	      L_nullify_operation (opB);
#ifdef TEST_RED_LOAD
	      fprintf (ERR, " becomes\n");
	      L_print_oper (ERR, opA);
	      L_print_oper (ERR, new_op);
#endif
	    }
	  change++;
	}
#if 0
      /* SER 20040210 Match signed, unsigned loads to same location. */
      /* Currently not activated: we may want to change this so it optimizes
       * for reuse of the unsigned load first if possible. */
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  int continue_flag;
	  /*
	   *  match pattern
	   */
	  if (!L_load_opcode (opB))
	    continue;
	  continue_flag = 0;
	  switch (opA->opc)
	    {
	    case Lop_LD_UC:
	      if (opB->opc != Lop_LD_C)
		continue_flag = 1;
	      break;
	    case Lop_LD_C:
	      if (opB->opc != Lop_LD_UC)
		continue_flag = 1;
	      break;
	    case Lop_LD_C2:
	      if (opB->opc != Lop_LD_UC2)
		continue_flag = 1;
	      break;
	    case Lop_LD_UC2:
	      if (opB->opc != Lop_LD_C2)
		continue_flag = 1;
	      break;
	    case Lop_LD_I:
	      if (opB->opc != Lop_LD_UI)
		continue_flag = 1;
	      break;
	    case Lop_LD_UI:
	      if (opB->opc != Lop_LD_I)
		continue_flag = 1;
	      break;
	    default:
	      continue_flag = 1;
	      break;
	    }
	  if (continue_flag)
	    continue;

	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_local_memory_same_location (opA, opB))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (L_has_unsafe_macro_dest_operand (opB))
	    continue;
	  if (!L_all_dest_operand_no_defs_between (opA, opA, opB))
	    continue;
	  if (!L_no_overlap_write (cb, opA, opA, 0, opB, 1))
	    break;
	  /* Due to promotion, opA doesn't always have a
	     sync arc to the store, so check from opB too. MCM 5/10/2000 */
	  if (!L_no_overlap_write (cb, opB, opA, 0, opB, 1))
	    break;
	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = 1;
	  store_flag = 0;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;
	  if (L_num_dest_operand (opA) != 1)
	    continue;
	  /*
	   *  replace pattern.
	   */
#ifdef TEST_RED_LOAD
	  fprintf (ERR, "redundant load alt type elim op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
	  L_print_oper (ERR, opA);
	  L_print_oper (ERR, opB);
#endif
	  /* if only 1 dest operand, just reuse opB for move oper */
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (opA->dest[i] != NULL)
		break;
	    }
	  if (opA->dest[i] == NULL)
	    L_punt ("L_local_redundant_load: all dest of opA are NULL");
	  if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_MASK_PE))
	    {
	      if (!L_EXTRACT_BIT_VAL (opB->flags, L_OPER_MASK_PE))
		{
		  /* Insert a check of the one being removed is
		   *  not speculative 
		   */
		  L_global_insert_check_before (opA, cb, opB);
		}
	      else
		{
		  /* OpA needs to "adopt" all of OpB's checks
		   */
		  L_assign_all_checks (L_fn, opB, opA);
		}
	    }
	  if (opB->opc == Lop_LD_C || opB->opc == Lop_LD_C2 ||
	      opB->opc == Lop_LD_I)
	    if(M_oper_supported_in_arch(Lop_EXTRACT))
	      L_convert_to_extract (opB, L_copy_operand (opB->dest[i]),
				    L_copy_operand (opA->dest[i]), 0);
	    else
	    L_convert_to_zero_extend_oper (opB, L_copy_operand (opB->dest[i]),
					   L_copy_operand (opA->dest[i]));

#ifdef TEST_RED_LOAD
	  fprintf (ERR, " becomes\n");
	  L_print_oper (ERR, opA);
	  L_print_oper (ERR, opB);
#endif
	  change++;
	}
#endif
    }

  return change;
}


int
L_local_redundant_store (L_Cb * cb)
{
  int change;
  L_Oper *opA, *opB;
  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_store_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;
      
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag;
	  /*
	   *  match pattern no. 1
	   */
	  if (!L_same_opcode (opA, opB))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_local_memory_same_location (opA, opB))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;

	  if (!L_same_operand (opA->src[2], opB->src[2]))
	    continue;
	  if (!L_same_def_reachs (opA->src[2], opA, opB))
	    continue;
	  if (!L_no_overlap_write (cb, opA, opA, 0, opB, 1))
	    break;
	  /* check that address is not a general macro to handle special
	     case of 2 stores to stack with safe jsr between still
	     not a legal transformation */
	  macro_flag = (L_has_fragile_macro_operand (opA) ||
			L_is_macro (opA->src[0]) || L_is_macro (opA->src[1]));
	  if (!L_no_danger (macro_flag, 0, 1, opA, opB))
	    break;
	  /*
	   *  replace pattern no. 1
	   */
#ifdef TEST_RED_STORE
	  fprintf (ERR, "redundant store elim1 op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  L_nullify_operation (opB);
	  change++;
	}

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag;
	  /*
	   *  match pattern no. 2
	   */
	  if (!L_same_opcode (opA, opB))
	    continue;
	  if (!L_local_memory_same_location (opA, opB))
	    continue;
	  if (!PG_superset_predicate_ops (opB, opA))
	    continue;

	  if (!L_no_intersecting_br_between (opA, opB))
	    continue;
	  if (!L_no_overlap_read (cb, opA, opA, 0, opB, 1))
	    break;
	  if (!L_no_overlap_write (cb, opA, opA, 0, opB, 1))
	    break;
	  macro_flag = (L_is_macro (opA->src[0]) || L_is_macro (opA->src[1]));
	  if (!L_no_danger (macro_flag, 0, 1, opA, opB))
	    break;
	  /*
	   *  replace pattern no. 2
	   */
#ifdef TEST_RED_STORE
	  fprintf (ERR, "redundant store elim2 op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  L_nullify_operation (opA);
	  change++;
	  break;
	}
    }

  return change;
}


int
L_local_constant_folding (L_Cb * cb)
{
  int change;
  L_Oper *opA;
  change = 0;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if(L_find_attr(opA->attr, "do_not_constant_fold")) {
        // printf("Continuing for op %d\n", opA->id);
        continue;
      }

      if (L_is_zero (opA->src[0]))
	{
	  /*
	   *  match pattern no. 1 : 0+x, 0|x, 0^x
	   */

	  if (L_add_opcode (opA) || L_is_opcode (Lop_OR, opA) ||
	      L_is_opcode (Lop_XOR, opA))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding1: op%d (cb %d)\n", opA->id,
		       cb->id);
#endif
	      L_convert_to_move (opA, L_copy_operand (opA->dest[0]),
				 L_copy_operand (opA->src[1]));
	      STAT_COUNT ("L_local_constant_folding_0_add", 1, cb);
	      change++;
	      continue;
	    }

	  /*
	   *  match pattern no.2 : 0*x, 0/x, 0<<x, 0>>x, 0&x, 0%x
	   */

	  else if (L_mul_opcode (opA) || L_div_opcode (opA) ||
		   L_shift_opcode (opA) || L_int_rem_opcode (opA) ||
		   L_is_opcode (Lop_AND, opA))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding3: op%d (cb %d)\n", opA->id,
		       cb->id);
#endif
	      L_convert_to_move_of_zero (opA, L_copy_operand (opA->dest[0]));
	      STAT_COUNT ("L_local_constant_folding_0_mul", 1, cb);
	      change++;
	      continue;
	    }
	}

      if (L_is_zero (opA->src[1]))
	{
	  /*
	   *  match pattern no.3 : x+0, x-0, x|0, x^0, x<<0, x>>0
	   */

	  if (L_add_opcode (opA) || L_sub_opcode (opA) ||
	      L_shift_opcode (opA) || L_is_opcode (Lop_OR, opA) ||
	      L_is_opcode (Lop_XOR, opA))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding2: op%d (cb %d)\n", opA->id,
		       cb->id);
#endif
	      L_convert_to_move (opA, L_copy_operand (opA->dest[0]),
				 L_copy_operand (opA->src[0]));
	      STAT_COUNT ("L_local_constant_folding_add_0", 1, cb);
	      change++;
	      continue;
	    }

	  /*
	   *  match pattern no.4 : x*0, x&0
	   */

	  else if (L_mul_opcode (opA) || L_is_opcode (Lop_AND, opA))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding4: op%d (cb %d)\n", opA->id,
		       cb->id);
#endif
	      L_convert_to_move_of_zero (opA, L_copy_operand (opA->dest[0]));
	      STAT_COUNT ("L_local_constant_folding_mul_0", 1, cb);
	      change++;
	      continue;
	    }
	}

      /*
       *  match pattern no.5 : 1*x
       */

      if (L_is_one (opA->src[0]) && L_mul_opcode (opA))
	{
#ifdef TEST_CONST_FOLD
	  fprintf (ERR, "const folding5: op%d (cb %d)\n", opA->id, cb->id);
#endif
	  L_convert_to_move (opA, L_copy_operand (opA->dest[0]),
			     L_copy_operand (opA->src[1]));
	  STAT_COUNT ("L_local_constant_folding_1_mul", 1, cb);
	  change++;
	  continue;
	}

      /*
       *  match pattern no.6 : x*1, x/1
       */

      if (L_is_one (opA->src[1]))
	{
	  if (L_mul_opcode (opA) || L_div_opcode (opA))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding6: op%d (cb %d)\n", opA->id, cb->id);
#endif
	      L_convert_to_move (opA, L_copy_operand (opA->dest[0]),
				 L_copy_operand (opA->src[0]));
	      STAT_COUNT ("L_local_constant_folding_mul_1", 1, cb);
	      change++;
	      continue;
	    }
      
	  /* x%1 */
	  else if (L_int_rem_opcode (opA))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding6: op%d (cb %d\n", opA->id, cb->id);
#endif
	      L_convert_to_move_of_zero (opA, L_copy_operand (opA->dest[0]));
	      STAT_COUNT ("L_local_constant_folding_rem_1", 1, cb);
	      change++;
	      continue;
	    }
	}
      /*
       *  match pattern no.7 : x-x, x/x, x%x
       */

      if (L_same_operand (opA->src[0], opA->src[1]) &&
	  (L_sub_opcode (opA) || L_div_opcode (opA) ||
	   L_int_rem_opcode (opA)))
	{
#ifdef TEST_CONST_FOLD
	  fprintf (ERR, "const folding7: op%d (cb %d)\n", opA->id, cb->id);
#endif
	  if (L_sub_opcode (opA) || L_int_rem_opcode (opA))
	    L_convert_to_move_of_zero (opA, L_copy_operand (opA->dest[0]));
	  else
	    L_convert_to_move_of_one (opA, L_copy_operand (opA->dest[0]));
	  STAT_COUNT ("L_local_constant_folding_inv", 1, cb);
	  change++;
	  continue;
	}

      /*
       *  match pattern no.8 (const1 op const2)
       */

      if (L_general_arithmetic_opcode (opA) &&
	  L_all_src_is_numeric_constant (opA) && L_no_exceptions (opA))
	{
	  L_Operand *new_src = NULL;

	  if (L_will_lose_accuracy (opA))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "NO const folding, op %d will lose accuracy\n",
		       opA->id);
#endif
	      continue;
	    }

#ifdef TEST_CONST_FOLD
	  fprintf (ERR, "const folding8: op%d (cb %d)\n", opA->id, cb->id);
#endif
	  if (L_int_arithmetic_opcode (opA))
	    {
	      ITintmax imval;
	      imval = L_evaluate_int_arithmetic (opA);
	      new_src = L_new_gen_int_operand (imval);
	      if (opA->opc == Lop_F2_I || opA->opc == Lop_F_I)
		opA->com[0] = 0;
	    }
	  else if (L_flt_arithmetic_opcode (opA))
	    {
	      float val;
	      val = L_evaluate_flt_arithmetic (opA);
	      new_src = L_new_float_operand (val);
	      if (opA->opc == Lop_I_F)
		opA->com[0] = 0;
	    }
	  else if (L_dbl_arithmetic_opcode (opA))
	    {
	      double val;
	      val = L_evaluate_dbl_arithmetic (opA);
	      new_src = L_new_double_operand (val);
	      if (opA->opc == Lop_I_F2)
		opA->com[0] = 0;
	    }
	  else
	    {
	      L_punt ("L_const folding: illegal opcode");
	    }
	  L_convert_to_move (opA, L_copy_operand (opA->dest[0]), new_src);
	  STAT_COUNT ("L_local_constant_folding_two", 1, cb);
	  change++;
	  continue;
	}

      if (L_is_opcode (Lop_AND, opA))
	{
	  if (L_is_int_neg_one (opA->src[0]))
	    {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding AND -1: op %d (cb %d)\n)",
		       opA->id, cb->id);
#endif
	      L_convert_to_move (opA, L_copy_operand (opA->dest[0]),
				 L_copy_operand (opA->src[1]));
	      STAT_COUNT ("L_local_constant_folding_AND_move", 1, cb);
	      change++;
	      continue;
	    }
	  else if (L_is_int_neg_one (opA->src[1]))
           {
#ifdef TEST_CONST_FOLD
	      fprintf (ERR, "const folding AND -1: op %d (cb %d)\n)",
		       opA->id, cb->id);
#endif
	     L_convert_to_move (opA, L_copy_operand (opA->dest[0]),
				L_copy_operand (opA->src[0]));
	     STAT_COUNT ("L_local_constant_folding_AND_move", 1, cb);
	     change++;
	     continue;
	    }
	}
    }

  return change;
}

#define L_CCF_MAX_PARM 2
#define MAX_FN_NUMBER 20
#define FN_FMOD   0
#define FN_POW    1
#define FN_LOG    2
#define FN_FLOOR  3
#define FN_CEIL   4
#define FN_FABS   5
#define FN_SQRT   6
#define FN_EXP    7
#define FN_ACOS   8
#define FN_ASIN   9
#define FN_ATAN   10
#define FN_ATAN2  11
#define FN_COS    12
#define FN_SIN    13
#define FN_TAN    14
#define FN_COSH   15
#define FN_SINH   16
#define FN_TANH   17
#define FN_FMIN   18
#define FN_FMAX   19


char *CCF_function[MAX_FN_NUMBER] = {
  "_$fn_fmod",
  "_$fn_pow",
  "_$fn_log",
  "_$fn_floor",
  "_$fn_ceil",
  "_$fn_fabs",
  "_$fn_sqrt",
  "_$fn_exp",
  "_$fn_acos",
  "_$fn_asin",
  "_$fn_atan",
  "_$fn_atan2",
  "_$fn_cos",
  "_$fn_sin",
  "_$fn_tan",
  "_$fn_cosh",
  "_$fn_sinh",
  "_$fn_tanh"
  "_$fn_fmin"
  "_$fn_fmax"
};

int CCF_pcount[MAX_FN_NUMBER] = {
  2,				/* FN_FMOD   0 */
  2,				/* FN_POW    1 */
  1,				/* FN_LOG    2 */
  1,				/* FN_FLOOR  3 */
  1,				/* FN_CEIL   4 */
  1,				/* FN_FABS   5 */
  1,				/* FN_SQRT   6 */
  1,				/* FN_EXP    7 */
  1,				/* FN_ACOS   8 */
  1,				/* FN_ASIN   9 */
  1,				/* FN_ATAN   10 */
  2,				/* FN_ATAN2  11 */
  1,				/* FN_COS    12 */
  1,				/* FN_SIN    13 */
  1,				/* FN_TAN    14 */
  1,				/* FN_COSH   15 */
  1,				/* FN_SINH   16 */
  1,				/* FN_TANH   17 */
  2,				/* FN_FMIN   18 */
  2				/* FN_FMAX   19 */
};

int
L_constant_fold_subroutine (L_Operand * label, int *pcount)
{
  int type;
  for (type = 0; type < MAX_FN_NUMBER; type++)
    {
      if (!strcmp (label->value.l, CCF_function[type]))
	{
	  *pcount = CCF_pcount[type];
	  return type;
	}
    }
  return (-1);
}

float
L_evaluate_function_with_sources (int func_type, double s0, double s1)
{
  switch (func_type)
    {
    case FN_POW:
      return pow (s0, s1);
    case FN_LOG:
      return log (s0);
    case FN_FLOOR:
      return floor (s0);
    case FN_CEIL:
      return ceil (s0);
    case FN_FABS:
      return fabs (s0);
    case FN_SQRT:
      return sqrt (s0);
    case FN_EXP:
      return exp (s0);
    case FN_ACOS:
      return acos (s0);
    case FN_ASIN:
      return asin (s0);
    case FN_ATAN:
      return atan (s0);
    case FN_ATAN2:
      return atan2 (s0, s1);
    case FN_COS:
      return cos (s0);
    case FN_SIN:
      return sin (s0);
    case FN_TAN:
      return tan (s0);
    case FN_FMOD:
      return fmod (s0, s1);
    case FN_COSH:
      return cosh (s0);
    case FN_SINH:
      return sinh (s0);
    case FN_TANH:
      return tanh (s0);
    case FN_FMIN:
      return fmin (s0, s1);
    case FN_FMAX:
      return fmax (s0, s1);
    default:
      L_punt ("L_constant_fold_func: Invalid function \n");
    }
  return 0.0;
}

float
L_evaluate_function (int func_type, L_Operand * src0, L_Operand * src1)
{
  double s0, s1;

  s0 = 0;
  s1 = 0;
  if (src0)
    s0 = src0->value.f2;

  if (src1)
    s1 = src1->value.f2;

  return (L_evaluate_function_with_sources (func_type, s0, s1));
}

int
L_local_complex_constant_folding (L_Cb * cb)
{
  int i, change;
  L_Oper *opA;
  L_Oper *next_op, *new_op;
  L_Attr *attr;
  L_Operand *jsr_label;
  L_Operand *dest;
  L_Operand *src;
  L_Operand *operand;
  float result;
  int func, pcount, invalid;

  L_Operand *srcs[L_CCF_MAX_PARM];
  L_Oper *op_srcs[L_CCF_MAX_PARM];

  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = next_op)
    {
      /*
       *       match pattern
       */
      next_op = opA->next_op;

      if (!L_subroutine_call_opcode (opA))
	continue;
      if (!L_is_label (opA->src[0]))
	continue;

      jsr_label = opA->src[0];

      func = L_constant_fold_subroutine (jsr_label, &pcount);

      if (func < 0)
	continue;

      if ((attr = L_find_attr (opA->attr, "tr")) == NULL)
	continue;

      if (pcount > L_CCF_MAX_PARM)
	L_punt ("L_local_complex_constant_folding: pcount too large\n");

      invalid = 0;
      for (i = 0; i < pcount; i++)
	{
	  operand = attr->field[i];
	  if ((op_srcs[i] = L_prev_def (operand, opA)) == NULL)
	    {
	      invalid = 1;
	      break;
	    }
	  if (!L_move_opcode (op_srcs[i]))
	    {
	      invalid = 1;
	      break;
	    }
	  if (!L_is_numeric_constant (op_srcs[i]->src[0]) &&
	      !L_is_string (op_srcs[i]->src[0]))
	    {
	      invalid = 1;
	      break;
	    }

	  srcs[i] = op_srcs[i]->src[0];
	}

      if (invalid)
	continue;

      result = L_evaluate_function (func, srcs[0], srcs[1]);

      if ((attr = L_find_attr (opA->attr, "ret")) == NULL)
	continue;

      operand = attr->field[0];
      dest = L_copy_operand (operand);
      src = L_new_double_operand (result);

      new_op = L_create_move (dest, src);
      new_op->pred[0] = L_copy_operand (opA->pred[0]);
      L_insert_oper_after (cb, opA, new_op);

#ifdef TEST_CONST_FOLD
      fprintf (ERR, "complex const prop: op%d \n", opA->id);
#endif
      for (i = 0; i < pcount; i++)
	{
	  L_delete_oper (cb, op_srcs[i]);
	}
      L_delete_oper (cb, opA);

      change++;
    }

  return change;
}


int
L_local_constant_combining (L_Cb * cb)
{
  int change = 0;
  L_Oper *opA, *opB;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*  Move more stable operand to src1 if opc is commutative */
      if (!L_has_const_operand_and_realign_oper (opA))
	if (L_shift_opcode (opA))
	  continue;

      if (!(L_int_add_opcode (opA) ||
	    /* ((opA)->opc == Lop_OR) || ((opA)->opc == Lop_AND) || */
	    L_shift_opcode (opA) ||
	    L_load_opcode (opA) || L_store_opcode (opA)))
	continue;

      if(L_find_attr(opA->attr, "do_not_constant_fold")) {
        // printf("Continuing for op %d\n", opA->id);
        continue;
      }

      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;
      
      /*
       *  match pattern
       */

      if (L_same_operand (opA->dest[0], opA->src[0]))
	continue;

      if ((L_int_add_opcode (opA) || L_int_sub_opcode (opA)) &&
	  !L_same_operand (opA->dest[0], opA->src[1]))
	{
	  /*
	   *  match pattern no. 1 (merge ops with 1 const operand)
	   *  if opA is subtract, only src2 allowed to be int const
	   */

	  if ((L_is_int_constant (opA->src[0]) ||
	       L_is_int_constant (opA->src[1])) &&
	      !(L_int_sub_opcode (opA) && L_is_int_constant (opA->src[0])))
	    {
	      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
		{
		  int macro_flag, load_flag, store_flag;

		  if (!(L_load_opcode (opB) ||
			L_store_opcode (opB) ||
			L_int_add_opcode (opB) ||
			L_int_sub_opcode (opB) ||
			L_int_cond_branch_opcode (opB) ||
			L_int_pred_comparison_opcode (opB) ||
			L_int_comparison_opcode (opB)))
		    continue;
		  /* 02/07/03 REK Adding a check to make sure we don't touch a
		   *              volatile oper. */
		  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		    continue;
		  if (!L_compatible_to_combine_consts (opA, opB))
		    continue;
		  if (!(L_is_int_constant (opB->src[0]) ||
			L_is_int_constant (opB->src[1])))
		    continue;
		  if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
			L_same_operand (opA->dest[0], opB->src[1])))
		    continue;
		  if (!PG_superset_predicate_ops (opA, opB))
		    continue;
		  if (!L_no_defs_between (opA->dest[0], opA, opB))
		    continue;
		  if (!L_same_def_reachs (opA->src[0], opA, opB))
		    continue;
		  if (!L_same_def_reachs (opA->src[1], opA, opB))
		    continue;
		  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
				L_is_fragile_macro (opA->src[0]) ||
				L_is_fragile_macro (opA->src[1]));
		  load_flag = 0;
		  store_flag = 0;
		  if (!(L_no_danger (macro_flag, load_flag, store_flag,
				     opA, opB)))
		    break;
		  /*
		   *  replace pattern no. 1
		   */
		  if (L_combine_operations (opA, opB))
		    {
#ifdef TEST_CONST_COMBINE
		      fprintf (ERR,
			       "const combining1 op%d -> op%d (cb %d)\n",
			       opA->id, opB->id, cb->id);
#endif
		      STAT_COUNT ("L_local_constant_combining_operand", 1,
				  cb);
		      change++;
		    }
		}
	    }

	  /*
	   *  Match pattern no. 3 (propagate const upwards in dep chain, add)
	   */

	  if (L_single_use_of (cb, opA->dest[0], opA) &&
	      !L_is_numeric_constant (opA->src[0]))
	    {
	      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
		{
		  int macro_flag, load_flag, store_flag, old_num_oper,
		    new_num_oper;
		  L_Operand *temp;

		  if (!(L_int_add_opcode (opB) || L_int_sub_opcode (opB) ||
			L_load_opcode (opB) || L_store_opcode (opB)))
		    continue;
		  /* 02/07/03 REK Adding a check to make sure we don't touch a
		   *              volatile oper. */
		  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		    continue;
		  if (!PG_superset_predicate_ops (opA, opB))
		    continue;
		  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		    continue;
		  if (!(L_same_operand (opA->dest[0], opB->src[0])))
		    continue;
		  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
		    continue;
		  if (!(L_is_constant (opB->src[1])))
		    continue;
		  /*** SAM 5-94 This messes up induction var stuff if
                       swap 0's from ld/st's ****/
		  if (L_is_int_zero (opB->src[1]))
		    continue;
		  /*** SAM 5-94 ****/
		  /* don't move labels out of ld/st's */
		  if ((L_load_opcode (opB) || L_store_opcode (opB)) &&
		      (L_is_label (opB->src[1])))
		    continue;
		  /*if (!(L_same_def_reachs (opB->src[1], opA, opB)))
		     continue; */
		  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
				L_is_fragile_macro (opA->src[0]) ||
				L_is_fragile_macro (opA->src[1]));
		  load_flag = 0;
		  store_flag = 0;
		  if (!(L_no_danger (macro_flag, load_flag, store_flag,
				     opA, opB)))
		    break;
		  /*
		   *  Replace pattern no. 3
		   */
		  if (L_int_add_opcode (opA))
		    {
		      if (L_int_sub_opcode (opB))
			{
			  L_change_opcode (opA,
					   L_inverse_arithmetic (opA->opc));
			  L_change_opcode (opB,
					   L_inverse_arithmetic (opB->opc));
			  temp = opA->src[0];
			  opA->src[0] = opA->src[1];
			  opA->src[1] = opB->src[1];
			  opB->src[1] = temp;
			}
		      /* opB is add, ld or st */
		      else
			{
			  old_num_oper = M_num_oper_required_for (opB,
								  L_fn->name);
			  temp = opA->src[0];
			  opA->src[0] = opB->src[1];
			  opB->src[1] = temp;
			  new_num_oper = M_num_oper_required_for (opB,
								  L_fn->name);
			  /* check that resultant opB can be handled in arch */
			  if (new_num_oper > old_num_oper)
			    {
			      temp = opA->src[0];
			      opA->src[0] = opB->src[1];
			      opB->src[1] = temp;
			      continue;
			    }
			}
		    }
		  /* opA is subtract */
		  else
		    {
		      if (L_int_sub_opcode (opB))
			{
			  L_change_opcode (opA,
					   L_inverse_arithmetic (opA->opc));
			  temp = opA->src[0];
			  opA->src[0] = opB->src[1];
			  opB->src[1] = opB->src[0];
			  opB->src[0] = temp;
			}
		      /* opB is ld, st, or add */
		      else
			{
			  old_num_oper = M_num_oper_required_for (opB,
								  L_fn->name);
			  temp = opA->src[0];
			  opA->src[0] = opB->src[1];
			  opB->src[1] = temp;
			  new_num_oper = M_num_oper_required_for (opB,
								  L_fn->name);
			  /* check that resultant opB can be handled in arch */
			  if (new_num_oper > old_num_oper)
			    {
			      temp = opA->src[0];
			      opA->src[0] = opB->src[1];
			      opB->src[1] = temp;
			      continue;
			    }
			}
		    }
#ifdef TEST_CONST_COMBINE
		  fprintf (ERR, "const combining3 op%d -> op%d (cb %d)\n",
			   opA->id, opB->id, cb->id);
#endif
		  STAT_COUNT ("L_local_constant_combining_prop", 1, cb);
		  change++;
		}
	    }
	}

      /* SER: This didn't work previously because of operand flipping. 
         Doesn't happen much anyway */
      if ((L_shift_opcode (opA)))
	{
	  if (!(L_is_int_constant (opA->src[1])))
	    continue;
	  for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	    {
	      int macro_flag, load_flag, store_flag, shiftA, shiftB;
	      /*
	       *  Match pattern no. 2 (shift by const, shift by const)
	       */
	      /* 02/07/03 REK Adding a check to make sure we don't touch a
	       *              volatile oper. */
	      if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		continue;
	      if (!(L_same_opcode (opA, opB)))
		continue;
	      if (!(L_is_int_constant (opB->src[1])))
		continue;
	      if (!(L_same_operand (opA->dest[0], opB->src[0])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opB)))
		continue;
	      if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		continue;
	      shiftA = opA->src[1]->value.i;
	      shiftB = opB->src[1]->value.i;
	      if ((shiftA + shiftB) >= M_type_size (M_TYPE_INT))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]));
	      load_flag = 0;
	      store_flag = 0;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
		break;
	      /*
	       *  Replace pattern no. 2
	       */
#ifdef TEST_CONST_COMBINE
	      fprintf (ERR, "const combining2 op%d -> op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif
	      L_delete_operand (opB->src[0]);
	      L_delete_operand (opB->src[1]);
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_new_gen_int_operand (shiftA + shiftB);
	      STAT_COUNT ("L_local_constant_combining_shift", 1, cb);
	      change++;
	    }
	}
#if 0
      if ((opA)->opc == Lop_OR)
	{
	  if (!(L_is_int_constant (opA->src[1])))
	    continue;
	  for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	    {
	      int macro_flag, load_flag, store_flag, orA, orB, c_indx, r_indx;
	      /*
	       *  Match pattern no. 4 (two OR with const)
	       */
	      if (!(L_same_opcode (opA, opB)))
		continue;
	      /* 02/07/03 REK Adding a check to make sure we don't touch a
	       *              volatile oper. */
	      if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		continue;
	      if (L_is_int_constant (opB->src[0]))
		{
		  c_indx = 0;
		  r_indx = 1;
		}
	      else if (L_is_int_constant (opB->src[1]))
		{
		  c_indx = 1;
		  r_indx = 0;
		}
	      else
		continue;
	      if (!(L_same_operand (opA->dest[0], opB->src[r_indx])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opB)))
		continue;
	      if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]));
	      load_flag = 0;
	      store_flag = 0;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
		break;
	      /*
	       *  Replace pattern no. 4
	       */
#ifdef TEST_CONST_COMBINE
	      fprintf (ERR, "const combining4 op%d -> op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif
	      orA = opA->src[1]->value.i;
	      orB = opB->src[c_indx]->value.i;
	      L_delete_operand (opB->src[0]);
	      L_delete_operand (opB->src[1]);
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_new_gen_int_operand (orA | orB);
	      STAT_COUNT ("L_local_constant_combining_or", 1, cb);
	      change++;
	    }
	}

      if ((opA)->opc == Lop_AND)
	{
	  if (!(L_is_int_constant (opA->src[1])))
	    continue;
	  for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	    {
	      int macro_flag, load_flag, store_flag, andA, andB, c_indx,
		r_indx;
	      /*
	       *  Match pattern no. 5 (two AND  with const)
	       */
	      /* 02/07/03 REK Adding a check to make sure we don't touch a
	       *              volatile oper. */
	      if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		continue;
	      if (!(L_same_opcode (opA, opB)))
		continue;
	      if (L_is_int_constant (opB->src[0]))
		{
		  c_indx = 0;
		  r_indx = 1;
		}
	      else if (L_is_int_constant (opB->src[1]))
		{
		  c_indx = 1;
		  r_indx = 0;
		}
	      else
		continue;
	      if (!(L_same_operand (opA->dest[0], opB->src[r_indx])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opB)))
		continue;
	      if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]));
	      load_flag = 0;
	      store_flag = 0;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
		break;
	      /*
	       *  Replace pattern no. 5
	       */
#ifdef TEST_CONST_COMBINE
	      fprintf (ERR, "const combining5 op%d -> op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif
	      andA = opA->src[1]->value.i;
	      andB = opB->src[c_indx]->value.i;
	      L_delete_operand (opB->src[0]);
	      L_delete_operand (opB->src[1]);
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_new_gen_int_operand (andA & andB);
	      STAT_COUNT ("L_local_constant_combining_and", 1, cb);
	      change++;
	    }
	}
#endif
    }

  return change;
}

int
L_local_strength_reduction (L_Cb * cb)
{
  int change;
  L_Oper *opA;
  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      int val;
      /*
       *  match pattern no. 1 (2^k * x) or (x * 2^k)
       */
      if (!(L_int_mul_opcode (opA)))
	continue;
      if (!(L_is_power_of_two (opA->src[0]) ||
	    L_is_power_of_two (opA->src[1])))
	continue;
      /*
       *  replace pattern no. 1
       */
#ifdef TEST_STR_RED
      fprintf (ERR, "strength reduction1: op%d (cb %d)\n", opA->id, cb->id);
#endif
      /* put constant in src2 */
      if (L_is_power_of_two (opA->src[0]))
	{
	  L_Operand *temp;
	  temp = opA->src[0];
	  opA->src[0] = opA->src[1];
	  opA->src[1] = temp;
	}
      val = ITicast (opA->src[1]->value.i);
      val = C_log2 (val);
      L_change_opcode (opA, Lop_LSL);
      L_delete_operand (opA->src[1]);
      opA->src[1] = L_new_gen_int_operand (val);
      STAT_COUNT ("L_local_strength_reduction_mul", 1, cb);
      change++;
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      int val;
      /*
       *  match pattern no. 2 (x / 2^k signed) or (x / 2^k unsigned)
       */
      if (!(L_int_div_opcode (opA)))
	continue;
      if (!(L_abs_is_power_of_two (opA->src[1])))
	continue;
      if (L_is_opcode (Lop_DIV, opA) &&
	  !(Lopti_do_local_strength_red_for_signed_div_rem))
	continue;

      /*
       *  replace pattern no. 2
       */
#ifdef TEST_STR_RED
      fprintf (ERR, "strength reduction2: op%d (cb %d)\n", opA->id, cb->id);
#endif
      if (L_is_opcode (Lop_DIV, opA))
	{
	  /* Replace a signed divide of b/c is replaced with the following.
	   *     t1 = b >> 31
	   *     t2 = t1 & (c-1)
	   *     t3 = b + t2
	   *     n = t3 >> log2(c)
	   * If c is negative, 
	   *   b and c both get negated to produce correct result.
	   * A simple shift operation does not work correctly. */
	  L_create_divide_operations (opA, cb);
	  L_nullify_operation (opA);
	}
      else
	{			/* an unsigned divide can be replaced by a shift. */
	  val = ITicast (opA->src[1]->value.i);
	  val = C_log2 (val);
	  L_change_opcode (opA, Lop_LSR);
	  L_delete_operand (opA->src[1]);
	  opA->src[1] = L_new_gen_int_operand (val);
	}
      STAT_COUNT ("L_local_strength_reduction_div", 1, cb);
      change++;
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      int val;
      /*
       *  match pattern no. 3 (x mod 2^k) signed or unsigned
       */
      if (!(L_int_rem_opcode (opA)))
	continue;
      if (!(L_abs_is_power_of_two (opA->src[1])))
	continue;
      if (L_is_opcode (Lop_REM, opA) &&
	  !Lopti_do_local_strength_red_for_signed_div_rem)
	continue;

      /*
       *  replace pattern no. 3 
       */
#ifdef TEST_STR_RED
      fprintf (ERR, "strength reduction3: op%d (cb %d)\n", opA->id, cb->id);
#endif
      if (L_is_opcode (Lop_REM, opA))
	{
	  /* Replace a signed remainder of b%c with the following.
	   *     t1 = b >> 31
	   *     t2 = t1 & (c-1)
	   *     t3 = b + t2
	   *     t4 = t3 & -c
	   *     rem = b - t4
	   * If c is negative, only c needs to be negated for correct result.
	   * A simple and operation does not work correctly. */
	  L_create_rem_operations (opA, cb);
	  L_nullify_operation (opA);
	}
      else
	{			/* an unsigned remainder can be replaced by an and. */
	  val = ITicast (opA->src[1]->value.i);
	  val = val - 1;
	  L_change_opcode (opA, Lop_AND);
	  L_delete_operand (opA->src[1]);
	  opA->src[1] = L_new_gen_int_operand (val);
	}
      STAT_COUNT ("L_local_strength_reduction_rem", 1, cb);
      change++;
    }

#if 0
  this doesnt work if x is a negative number,
    need to ensure that x is positive for (opA = cb->first_op; opA != NULL;
					   opA = opA->next_op)
    {
      int val;
      L_Oper *opB, *opC;
      /*
       *  match pattern no. 4 (x mod 2^k) expressed as asr,lsl,sub
       */
      opB = opA->next_op;
      if (opB == NULL)
	break;
      opC = opB->next_op;
      if (opC == NULL)
	break;
      if (!(L_is_opcode (Lop_ASR, opA)))
	continue;
      if (!(L_is_opcode (Lop_LSL, opB)))
	continue;
      if (!(L_is_opcode (Lop_SUB, opC)))
	continue;
      if (!(L_is_int_constant (opA->src[1])))
	continue;
      if (!(L_same_operand (opA->src[1], opB->src[1])))
	continue;
      if (!(L_same_operand (opA->dest[0], opB->src[0])))
	continue;
      if (!(L_same_operand (opB->dest[0], opC->src[1])))
	continue;
      if (!(L_same_operand (opA->src[0], opC->src[0])))
	continue;
      /*
       *  replace pattern no. 4 
       */
#ifdef TEST_STR_RED
      fprintf (ERR, "strength reduction4: op%d op%d op%d (cb %d)\n",
	       opA->id, opB->id, opC->id, cb->id);
#endif
      val = C_pow2 (ITicast (opA->src[1]->value.i)) - 1;
      L_change_opcode (opA, Lop_AND);
      L_delete_operand (opA->src[1]);
      opA->src[1] = L_new_gen_int_operand (val);
      L_nullify_operation (opB);
      L_nullify_operation (opC);
      change++;
    }
#endif

#if 0
  This REALLY cannot be done legally and maintain accuracy !
    for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       *  match pattern no. 5 (x / const) float or double
       */
      if (!(L_is_opcode (Lop_DIV_F, opA) || L_is_opcode (Lop_DIV_F2, opA)))
	continue;
      if (!(L_is_numeric_constant (opA->src[1])))
	continue;
      if (L_is_opcode (Lop_DIV_F, opA))
	{
	  if (!(L_invertible_float_constant ((double) opA->src[1]->value.f)))
	    continue;
	}
      else if (L_is_opcode (Lop_DIV_F2, opA))
	{
	  if (!(L_invertible_constant (opA->src[1]->value.f2)))
	    continue;
	}

      /*
       *  replace pattern no. 5
       */
#ifdef TEST_STR_RED
      fprintf (ERR, "strength reduction4: op%d (cb %d)\n", opA->id, cb->id);
#endif
      if (L_is_opcode (Lop_DIV_F, opA))
	{
	  float val;
	  val = opA->src[1]->value.f;
	  val = 1.0 / val;
	  L_change_opcode (opA, Lop_MUL_F);
	  L_delete_operand (opA->src[1]);
	  opA->src[1] = L_new_float_operand (val);
	}
      else
	{
	  double val;
	  val = opA->src[1]->value.f2;
	  val = 1.0 / val;
	  L_change_opcode (opA, Lop_MUL_F2);
	  L_delete_operand (opA->src[1]);
	  opA->src[1] = L_new_double_operand (val);
	}
      STAT_COUNT ("L_local_strength_reduction_div_f", 1, cb);
      change++;
    }
#endif

  return change;
}

#undef DEBUG_OP_FOLDING

int
L_local_operation_folding (L_Cb * cb)
{
  int change = 0;
  L_Oper *opA, *opB;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      L_Oper *mov_op = NULL;
      L_Operand *dest;
      int opposite_cond = 0;
      int partial_initialized = 0;
      ITintmax init_val = 0;

      /* Pattern:      mov dest = 1                      */
      /*          (p1) rcmp dest = condition             */
      /*               br/cmp eq/ne dest, const (0 or 1) */

      if (!L_general_comparison_opcode (opA))
	continue;

      dest = opA->dest[0];

      if (opA->pred[0])
	{
	  /* Walk up and find if dest of RCMP is initialized to a constant */
	  for (mov_op = opA; mov_op; mov_op = mov_op->prev_op)
	    if (L_is_dest_operand (dest, mov_op))
	      break;

	  if (!L_int_move_opcode (mov_op) ||
	      !L_is_int_constant (mov_op->src[0]) ||
	      !PG_superset_predicate_ops (mov_op, opA))
	    continue;

	  init_val = mov_op->src[0]->value.i;

	  if (init_val != 0 && init_val != 1)
	    continue;

	  partial_initialized = 1;
	}

      /* Walk down and find if dest of RCMP is compared to a 1 or a 0 */
      for (opB = opA; opB; opB = opB->next_op)
	{
	  ITintmax cmp_val;
	  int r_indx, c_indx;

	  if (!L_int_cond_branch_opcode (opB) &&
	      !L_int_pred_comparison_opcode (opB))
	    continue;

	  if (L_is_int_constant (opB->src[0]) && L_is_variable (opB->src[1]))
	    {
	      c_indx = 0;
	      r_indx = 1;
	    }
	  else if (L_is_variable (opB->src[0]) &&
		   L_is_int_constant (opB->src[1]))
	    {
	      r_indx = 0;
	      c_indx = 1;
	    }
	  else
	    {
	      continue;
	    }

	  if (!L_same_operand (dest, opB->src[r_indx]))
	    continue;

	  cmp_val = opB->src[c_indx]->value.i;

	  if (partial_initialized)
	    {
	      int com;
	      com = L_get_compare_type (opB);
	      if ((com != Lcmp_COM_EQ) && (com != Lcmp_COM_NE))
		continue;

	      if (cmp_val != 0 && cmp_val != 1)
		continue;

	      if (!PG_subset_predicate_ops (opB, mov_op))
		continue;
	      if (!PG_superset_predicate_ops (opB, opA))
		continue;
	    }
	  else
	    {
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	    }

	  if (!L_no_defs_between (dest, opA, opB))
	    continue;
	  if (!L_all_src_operand_same_def_reachs (opA, opA, opB))
	    continue;

	  if (partial_initialized)
	    {
	      if ((L_int_pred_comparison_opcode (opB)))
		{
		  if ((opB->dest[0]) &&
		      !((opB->dest[0]->ptype == L_PTYPE_UNCOND_T) ||
			(opB->dest[0]->ptype == L_PTYPE_OR_T)))
		    continue;
		  if ((opB->dest[1]) &&
		      !((opB->dest[1]->ptype == L_PTYPE_UNCOND_T) ||
			(opB->dest[1]->ptype == L_PTYPE_OR_T)))
		    continue;
		}

	      /* Make sure the mov initializes dest to a value such that the
	       * br/cmp will fail (thus placing p1 on the br/cmp will have
	       * the desired effect, since it never branches when p1 is false)
	       */

	      if (c_indx == 0)
		{
		  if (L_evaluate_int_compare_with_sources (opB,
							   cmp_val,
							   init_val) == 1)
		    continue;
		}
	      else
		{
		  if (L_evaluate_int_compare_with_sources (opB,
							   init_val,
							   cmp_val) == 1)
		    continue;
		}

	      if (init_val)
		opposite_cond = 1;
	    }
	  else
	    {
	      int tv, fv;

	      if (c_indx == 0)
		{
		  tv = L_evaluate_int_compare_with_sources (opB, cmp_val, 1);
		  fv = L_evaluate_int_compare_with_sources (opB, cmp_val, 0);
		}
	      else
		{
		  tv = L_evaluate_int_compare_with_sources (opB, 1, cmp_val);
		  fv = L_evaluate_int_compare_with_sources (opB, 0, cmp_val);
		}

	      if (tv && !fv)
		{
		  opposite_cond = 0;
		}
	      else if (!tv && fv)
		{
		  opposite_cond = 1;
		}
	      else
		{
		  /* Branch is de facto unconditional under opA's
		   * predicate, which is at least as strong as opB's
		   * original predicate.  */

		  int new_cmp;

#ifdef DEBUG_OP_FOLDING
		  fprintf (stderr, "> OP-FOLD rcmp %d -> cmp/br op %d\n",
			   opA->id, opB->id);
		  DB_print_oper (opA);
		  DB_print_oper (opB);
#endif

		  if (tv)
		    new_cmp = Lcmp_COM_EQ;
		  else
		    new_cmp = Lcmp_COM_NE;

		  L_delete_operand (opB->src[0]);
		  opB->src[0] = L_new_gen_int_operand (0);
		  L_delete_operand (opB->src[1]);
		  opB->src[1] = L_new_gen_int_operand (0);

		  if (opB->pred[0])
		    {
		      L_delete_operand (opB->pred[0]);
		      opB->pred[0] = L_copy_operand (opA->pred[0]);
		    }

		  L_set_compare (opB, L_CTYPE_INT, new_cmp);

#ifdef DEBUG_OP_FOLDING
		  DB_print_oper (opB);
#endif
		  STAT_COUNT ("L_local_operation_folding_00", 1, cb);
		  change++;
		  continue;
		}
	    }

	  /* Cool, lets fold this sucker! */
	  /* Pattern:     mov dest = 1           <-- Hopefully now dead */
	  /*         (p1) rcmp dest = condition  <-- Hopefully now dead */
	  /*         (p1) br/cmp      condition                         */

#ifdef DEBUG_OP_FOLDING
	  fprintf (stderr, "> OP-FOLD rcmp %d -> cmp/br op %d\n", opA->id,
		   opB->id);
	  DB_print_oper (opA);
	  DB_print_oper (opB);
#endif
	  if (partial_initialized)
	    {
	      if (opB->pred[0])
		L_delete_operand (opB->pred[0]);
	      opB->pred[0] = L_copy_operand (opA->pred[0]);
	    }
	  opB->com[0] = opA->com[0];
	  opB->com[1] = opA->com[1];
	  if (opposite_cond)
	    L_negate_compare (opB);
	  L_delete_operand (opB->src[0]);
	  opB->src[0] = L_copy_operand (opA->src[0]);
	  L_delete_operand (opB->src[1]);
	  opB->src[1] = L_copy_operand (opA->src[1]);

	  if (!L_int_comparison_opcode (opA))
	    {
	      if (L_cond_branch_opcode (opB))
		L_change_opcode (opB, Lop_BR_F);
	      else
		L_change_opcode (opB, Lop_CMP_F);
	    }

#ifdef DEBUG_OP_FOLDING
	  DB_print_oper (opB);
#endif
	  STAT_COUNT ("L_local_operation_folding_01", 1, cb);
	  change++;
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_general_comparison_opcode (opA)))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern no. 2 (comparison, eq/ne)
	   */

	  if (!(L_gen_ne_rcmp_opcode (opB) ||
		(L_gen_eq_rcmp_opcode (opB) &&
		 L_single_use_of (cb, opA->dest[0], opA))))
	    continue;
	  if (!(L_is_int_zero (opB->src[0]) || L_is_int_zero (opB->src[1])))
	    continue;
	  if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
		L_same_operand (opA->dest[0], opB->src[1])))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_uses_between (opA->dest[0], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *  replace pattern no. 2
	   */
#ifdef TEST_OP_FOLD
	  fprintf (ERR, "operation folding2: op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  if (L_gen_eq_rcmp_opcode (opB))
	    L_negate_compare (opA);
	  L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
			     L_copy_operand (opA->dest[0]));
	  STAT_COUNT ("L_local_operation_folding_02", 1, cb);
	  change++;
	}
    }

    for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
      {
	if (!(L_int_add_opcode (opA)))
	  continue;
	if (L_same_operand (opA->dest[0], opA->src[0]) ||
	    L_same_operand (opA->dest[0], opA->src[1]))
	  continue;
	for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	  {
	    L_Operand *old_src1, *old_src2;
	    int macro_flag, load_flag, store_flag,
	      old_num_oper, new_num_oper;
	    /*
	     *  match pattern no. 3 (merge add and load/store)
	     */
	    if (!(L_load_opcode (opB) || L_store_opcode (opB)))
	      continue;
	    if (!(L_is_int_zero (opB->src[0]) ||
		  L_is_int_zero (opB->src[1])))
	      continue;
	    if (!(L_same_operand (opA->dest[0], opB->src[0]) |
		  L_same_operand (opA->dest[0], opB->src[1])))
	      continue;
	    if (!PG_superset_predicate_ops (opA, opB))
	      continue;
	    if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	      continue;
	    if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	      continue;
	    if (!(L_same_def_reachs (opA->src[1], opA, opB)))
	      continue;
	    macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			  L_is_fragile_macro (opA->src[0]) ||
			  L_is_fragile_macro (opA->src[1]));
	    load_flag = 0;
	    store_flag = 0;
	    if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	      break;
	    /*
	     *  replace pattern no. 3
	     */
	    old_num_oper = M_num_oper_required_for (opB, L_fn->name);
	    old_src1 = opB->src[0];
	    old_src2 = opB->src[1];
	    opB->src[0] = L_copy_operand (opA->src[0]);
	    opB->src[1] = L_copy_operand (opA->src[1]);
	    new_num_oper = M_num_oper_required_for (opB, L_fn->name);
	    /* only modify load if it can be handled in target arch */
	    if (new_num_oper > old_num_oper)
	      {
		L_delete_operand (opB->src[0]);
		L_delete_operand (opB->src[1]);
		opB->src[0] = old_src1;
		opB->src[1] = old_src2;
		continue;
	      }
#ifdef TEST_OP_FOLD
	    fprintf (ERR, "operation folding3: op%d -> op%d (cb %d)\n",
		     opA->id, opB->id, cb->id);
#endif
	    L_delete_operand (old_src1);
	    L_delete_operand (old_src2);
	    STAT_COUNT ("L_local_operation_folding_03", 1, cb);
	    change++;
	  }
      }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_increment_operation (opA)))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern no. 4 (increment, increment)
	   */
	  if (!(L_increment_operation (opB)))
	    continue;
	  if (!(L_same_operand (opA->dest[0], opB->dest[0])))
	    continue;
	  if (!PG_equivalent_predicates_ops (opA, opB))
	    continue;
	  if (!(L_only_disjoint_br_between (opA, opB)))
	    continue;
	  if (!(L_no_uses_between (opA->dest[0], opA, opB)))
	    break;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    continue;
	  /*
	   *  replace pattern no. 4 
	   */
#ifdef TEST_OP_FOLD
	  fprintf (ERR, "operation folding4: op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  L_unmark_as_pre_post_increment (opA);
	  L_unmark_as_pre_post_increment (opB);
	  L_combine_increment_operations (opA, opB);
	  L_nullify_operation (opB);
	  STAT_COUNT ("L_local_operation_folding_04", 1, cb);
	  change++;
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_move_opcode (opA)))
	continue;
      if (!(L_is_variable (opA->src[0])))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[0]))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  L_Oper *opC;
	  /* 
	   *  match pattern no. 5 (merge mov, increment, op)
	   */
	  if (!(L_is_src_operand (opA->dest[0], opB)))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  if (L_same_def_reachs (opA->src[0], opA, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  opC = L_next_def (opA->src[0], opA);
	  if (!(L_increment_operation (opC)))
	    continue;
	  if (!PG_superset_predicate_ops (opC, opB))
	    continue;
	  /* Better to handle with code motion if possible */
	  if (L_can_move_above (cb, opC, opB))
	    continue;
	  if (L_can_move_below (cb, opB, opC))
	    continue;
	  if (!(L_can_undo_and_combine (opA, opC, opB)))
	    continue;
	  /*
	   *  replace pattern no. 5 
	   */
#ifdef TEST_OP_FOLD
	  fprintf (ERR, "operation folding5: op%d -> op%d (cb %d)\n",
		   opC->id, opB->id, cb->id);
#endif
	  L_undo_and_combine (opC, opB);
	  STAT_COUNT ("L_local_operation_folding_05", 1, cb);
	  change++;
	}
    }
#if 0
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (L_is_opcode (Lop_DEFINE, opA))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  L_Oper *opC, *opD;
	  /*
	   *  match pattern no. 6 (op, move, add, op)
	   */
	  if (!(L_move_opcode (opB)))
	    continue;
	  if (L_is_macro (opB->dest[0]) || L_is_macro (opB->src[0]))
	    continue;
	  if (!(L_is_dest_operand (opB->src[0], opA)))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opB->dest[0], opA, opB)))
	    continue;
	  if (!(L_no_defs_between (opB->src[0], opA, opB)))
	    continue;
	  if (L_live_outside_cb_after (cb, opB->src[0], opA))
	    continue;
	  if (!(L_same_def_reachs (opB->dest[0], opA, opB)))
	    continue;
	  if (!(L_no_uses_between (opB->dest[0], opA, opB)))
	    continue;
	  if (L_can_change_all_later_uses_with
	      (cb, opB->src[0], opB->dest[0], opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opB->dest[0]) ||
			L_is_fragile_macro (opB->src[0]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  if (!(opC = L_next_def (opB->dest[0], opB)))
	    continue;
	  if (!PG_superset_predicate_ops (opB, opC))
	    continue;
	  if (!(opD = L_next_use (opB->src[0], opC)))
	    continue;
	  if (!(L_no_defs_between (opB->dest[0], opC, opD)))
	    continue;
	  if (!(L_no_defs_between (opB->src[0], opC, opD)))
	    continue;
	  if (!PG_superset_predicate_ops (opC, opD))
	    continue;
	  if (L_can_move_above (cb, opC, opD))
	    continue;
	  if (L_can_move_below (cb, opD, opC))
	    continue;
	  if (!(L_can_undo_and_combine2 (opC, opD)))
	    continue;
	  /*
	   *  replace pattern no. 6
	   */
#ifdef TEST_OP_FOLD
	  fprintf (ERR, "operation folding6: op%d -> op%d (cb %d)\n",
		   opC->id, opD->id, cb->id);
#endif
	  L_undo_and_combine (opC, opD);
	  STAT_COUNT ("L_local_operation_folding_06", 1, cb);
	  change++;
	}
    }
#endif
  /*
   *  check whether target processor supports NAND/NOR/NXOR, if not
   *  don't waste time with this optimization.
   */
  if (M_oper_supported_in_arch (Lop_NAND) ||
      M_oper_supported_in_arch (Lop_NOR) ||
      M_oper_supported_in_arch (Lop_NXOR))
    {
      for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
	{
	  if (!((L_is_opcode (Lop_OR, opA) &&
		 M_oper_supported_in_arch (Lop_NOR)) ||
		(L_is_opcode (Lop_AND, opA) &&
		 M_oper_supported_in_arch (Lop_NAND)) ||
		(L_is_opcode (Lop_XOR, opA) &&
		 M_oper_supported_in_arch (Lop_NXOR))))
	    continue;
	  for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	    {
	      int macro_flag, load_flag, store_flag, new_opc;
	      /*
	       *      match pattern no. 7 (or/and/xor, xor -1)
	       */
	      if (!(L_is_opcode (Lop_XOR, opB)))
		continue;
	      if (!(L_is_int_neg_one (opB->src[0]) ||
		    L_is_int_neg_one (opB->src[1])))
		continue;
	      if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
		    L_same_operand (opA->dest[0], opB->src[1])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opB)))
		continue;
	      if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		continue;
	      if (!(L_same_def_reachs (opA->src[1], opA, opB)))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]) ||
			    L_is_fragile_macro (opA->src[1]));
	      load_flag = 0;
	      store_flag = 0;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
		break;
	      /*
	       *      replace pattern no. 7
	       */
#ifdef TEST_OP_FOLD
	      fprintf (ERR, "operation folding7: op%d -> op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif
	      if (L_is_opcode (Lop_OR, opA))
		new_opc = Lop_NOR;
	      else if (L_is_opcode (Lop_AND, opA))
		new_opc = Lop_NAND;
	      else
		new_opc = Lop_NXOR;
	      L_change_opcode (opB, new_opc);
	      L_delete_operand (opB->src[0]);
	      L_delete_operand (opB->src[1]);
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_copy_operand (opA->src[1]);
	      STAT_COUNT ("L_local_operation_folding_07", 1, cb);
	      change++;
	    }
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_is_opcode (Lop_ADD, opA)))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[0]) ||
	  L_same_operand (opA->dest[0], opA->src[1]))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag,
	    s1B_matches_destA, s2B_matches_destA,
	    s1A_matches_srcB, s2A_matches_srcB;
	  /*
	   *  match pattern no. 8 (signed add, branch) => (add, branch vs 0)
	   */
	  if (!(L_int_cond_branch_opcode (opB)))
	    continue;
	  s1B_matches_destA = L_same_operand (opA->dest[0], opB->src[0]);
	  s2B_matches_destA = L_same_operand (opA->dest[0], opB->src[1]);
	  if (!(s1B_matches_destA || s2B_matches_destA))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    continue;
	  if (s1B_matches_destA)
	    {
	      s1A_matches_srcB = L_same_operand (opA->src[0], opB->src[1]);
	      s2A_matches_srcB = L_same_operand (opA->src[1], opB->src[1]);
	    }
	  else
	    {
	      s1A_matches_srcB = L_same_operand (opA->src[0], opB->src[0]);
	      s2A_matches_srcB = L_same_operand (opA->src[1], opB->src[0]);
	    }
	  if (!(s1A_matches_srcB || s2A_matches_srcB))
	    continue;
	  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[1], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opB->src[0]) ||
			L_is_fragile_macro (opB->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *  replace pattern no. 8
	   */
#ifdef TEST_OP_FOLD
	  fprintf (ERR, "operation folding8: op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  L_delete_operand (opB->src[0]);
	  L_delete_operand (opB->src[1]);
	  if (s1B_matches_destA)
	    {
	      if (s1A_matches_srcB)
		{
		  opB->src[0] = L_copy_operand (opA->src[1]);
		  opB->src[1] = L_new_gen_int_operand (0);
		}
	      else
		{
		  opB->src[0] = L_copy_operand (opA->src[0]);
		  opB->src[1] = L_new_gen_int_operand (0);
		}
	    }
	  else
	    {
	      if (s1A_matches_srcB)
		{
		  opB->src[0] = L_new_gen_int_operand (0);
		  opB->src[1] = L_copy_operand (opA->src[1]);
		}
	      else
		{
		  opB->src[0] = L_new_gen_int_operand (0);
		  opB->src[1] = L_copy_operand (opA->src[0]);
		}
	    }
	  STAT_COUNT ("L_local_operation_folding_08", 1, cb);
	  change++;
	}
    }

  /*
   *  check whether target processor supports any of the mul_add/sub
   *  that may be generated.  Dont attempt opti unless does.
   */
  if (M_oper_supported_in_arch (Lop_MUL_ADD) ||
      M_oper_supported_in_arch (Lop_MUL_ADD_U) ||
      M_oper_supported_in_arch (Lop_MUL_ADD_F) ||
      M_oper_supported_in_arch (Lop_MUL_ADD_F2) ||
      M_oper_supported_in_arch (Lop_MUL_SUB) ||
      M_oper_supported_in_arch (Lop_MUL_SUB_U) ||
      M_oper_supported_in_arch (Lop_MUL_SUB_REV) ||
      M_oper_supported_in_arch (Lop_MUL_SUB_REV_U) ||
      M_oper_supported_in_arch (Lop_MUL_SUB_F) ||
      M_oper_supported_in_arch (Lop_MUL_SUB_REV_F) ||
      M_oper_supported_in_arch (Lop_MUL_SUB_F2) ||
      M_oper_supported_in_arch (Lop_MUL_SUB_REV_F2))
    {
      for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
	{
	  if (!(L_mul_opcode (opA)))
	    continue;
	  /* for now don't generate int mul_add/sub, it messes up
	     induction variable strength red */
	  if (L_int_mul_opcode (opA))
	    continue;
	  for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	    {
	      int macro_flag, load_flag, store_flag, new_opc, s0_matches;
	      L_Operand *src2;
	      /*
	       *      match pattern no. 9 (mul_add/mul_sub creation)
	       */
	      if (!(L_add_opcode (opB) || L_sub_opcode (opB)))
		continue;
	      if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
		    L_same_operand (opA->dest[0], opB->src[1])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opB)))
		break;
	      if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		continue;
	      if (!(L_same_def_reachs (opA->src[1], opA, opB)))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]) ||
			    L_is_fragile_macro (opA->src[1]));
	      load_flag = 0;
	      store_flag = 0;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
		break;
	      /* resultant mul_add/sub is supported in the arch */
	      if (!(L_can_make_mul_add_sub (opA, opB)))
		continue;
	      /*
	       *      match pattern no. 9 (mul_add/mul_sub creation)
	       */
#ifdef TEST_OP_FOLD
	      fprintf (ERR,
		       "op folding9 (mul_add/sub): op%d -> op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif
	      s0_matches = L_same_operand (opA->dest[0], opB->src[0]);
	      if (s0_matches)
		src2 = L_copy_operand (opB->src[1]);
	      else
		src2 = L_copy_operand (opB->src[0]);

	      if (L_add_opcode (opB))
		{
		  new_opc = L_corresponding_mul_add (opB);
		}
	      else
		{
		  if (s0_matches)
		    new_opc = L_corresponding_mul_sub (opB);
		  else
		    new_opc = L_corresponding_mul_sub_rev (opB);
		}

	      L_change_opcode (opB, new_opc);
	      L_delete_operand (opB->src[0]);
	      L_delete_operand (opB->src[1]);
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_copy_operand (opA->src[1]);
	      opB->src[2] = src2;
	      STAT_COUNT ("L_local_operation_folding_09", 1, cb);
	      change++;
	    }
	}
    }

  /*
   *  only do this if the flag is turned on, check for arch support
   *  in the pattern match...
   */
  if (Lopti_do_post_inc_conv)
    {
      /*
       *      Match pattern no. 10 (ld/st, increment -> post inc ld/st)
       */
      for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
	{
	  if (!(L_load_opcode (opA) || L_store_opcode (opA)))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	    continue;
	  if (L_marked_as_post_increment (opA) ||
	      L_marked_as_pre_increment (opA))
	    continue;
	  for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	    {
	      L_Operand *base, *offset, *temp;

	      /* 02/07/03 REK Adding a check to make sure we don't touch a
	       *              volatile oper. */
	      if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		continue;
	      if (!(L_increment_operation (opB)))
		continue;
	      if (!(L_is_register (opB->dest[0])))
		continue;
	      if (L_marked_as_post_increment (opB) ||
		  L_marked_as_pre_increment (opB))
		continue;

	      if (L_same_operand (opB->dest[0], opA->src[0]))
		{
		  base = opA->src[0];
		  offset = opA->src[1];
		}
	      else if (L_same_operand (opB->dest[0], opA->src[1]))
		{
		  base = opA->src[1];
		  offset = opA->src[0];
		}
	      else
		{
		  continue;
		}

	      if (!(L_no_defs_between (base, opA, opB)))
		continue;
	      if (!(L_no_uses_between (base, opA, opB)))
		continue;

	      /* may not be necessary, can check liveness of br between.. */
	      if (!(L_no_intersecting_br_between (opA, opB)))
		break;

	      /* resulting post increment is supported in the arch */
	      if (!(L_can_make_post_inc (opA, opB)))
		continue;

#ifdef TEST_OP_FOLD
	      fprintf (ERR,
		       "op folding10 (post inc): op%d -> op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif

	      /* Align the operands to the proper positions!! */

	      /* make sure base/offset in right place for ld/st */
	      if (base != opA->src[0])
		{
		  opA->src[0] = base;
		  opA->src[1] = offset;
		}

	      /* if increment is sub, convert it to an add */
	      if (L_int_sub_opcode (opB) && L_is_int_constant (opB->src[1]))
		{
		  temp = opB->src[1];
		  opB->src[1] = L_new_gen_int_operand (-(temp->value.i));
		  L_delete_operand (temp);
		  L_change_opcode (opB, L_corresponding_add (opB));
		}

	      /* swap increment operands if necessary */
	      if (L_int_add_opcode (opB) &&
		  L_same_operand (opB->dest[0], opB->src[1]))
		{
		  temp = opB->src[0];
		  opB->src[0] = opB->src[1];
		  opB->src[1] = temp;
		}

	      /* set the attribute fields indicating a post increment */
	      L_mark_as_post_increment (opA, opB);

	      STAT_COUNT ("L_local_operation_folding_10", 1, cb);
	      change++;
	    }
	}
    }

  if (change)
    L_invalidate_dataflow ();

  return change;
}

int
L_local_branch_folding (L_Cb * cb)
{
  int change, cc;
  L_Oper *opA, *opB, *nextA, *nextB;

  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      int cc;
      /*
       *  match pattern no.1 (cond br with const operands)
       */
      nextA = opA->next_op;
      if (L_register_branch_opcode (opA))
	break;
      if (!(L_cond_branch_opcode (opA)))
	continue;
      if (!(L_is_numeric_constant (opA->src[0])))
	continue;
      if (!(L_is_numeric_constant (opA->src[1])))
	continue;
      if (!L_operand_ctype_same (opA->src[0], opA->src[1]))
	continue;

      /*
       *  replace pattern no.1
       */
#ifdef TEST_BR_FOLD
      fprintf (ERR, "branch folding1: op%d (cb %d)\n", opA->id, cb->id);
#endif
      cc = L_evaluate_compare (opA);
      if (cc == 1)
	nextA = NULL;
      L_fix_cond_br (cb, opA, cc);
      STAT_COUNT ("L_local_branch_folding_const_opr", 1, cb);
      change++;
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_cond_branch_opcode (opA)))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;

	  /*
	   *  Match pattern no. 2 (2 br same cond -> remove 2nd br)
	   */
	  nextB = opB->next_op;
	  if (!(L_cond_branch_opcode (opB)))
	    continue;
	  if (!L_same_compare (opA, opB))
	    continue;
	  if (!(L_same_operand (opA->src[0], opB->src[0])))
	    continue;
	  if (!(L_same_operand (opA->src[1], opB->src[1])))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[1], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *  replace pattern no. 2
	   */
#ifdef TEST_BR_FOLD
	  fprintf (ERR, "branch folding2: op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  cc = L_are_opposite_branch_opcodes (opA, opB);
	  if (cc == 1)
	    nextB = NULL;
	  L_fix_cond_br (cb, opB, cc);
	  STAT_COUNT ("L_local_branch_folding_same_cond", 1, cb);
	  change++;
	}
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!
	  (L_int_blt_branch_opcode (opA) || L_int_ble_branch_opcode (opA)
	   || L_int_bgt_branch_opcode (opA) || L_int_bge_branch_opcode (opA)))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  Match pattern no. 3 (same condition, opposite compares)
	   */
	  nextB = opB->next_op;
	  if (!(L_are_reverse_branch_opcodes (opA, opB)))
	    continue;
	  if (!(L_same_operand (opA->src[0], opB->src[0])))
	    continue;
	  if (!(L_same_operand (opA->src[1], opB->src[1])))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[1], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *  replace pattern no. 3
	   */
#ifdef TEST_BR_FOLD
	  fprintf (ERR, "branch folding3: op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  if (L_int_blt_branch_opcode (opA) || L_int_bgt_branch_opcode (opA))
	    {
	      L_set_compare_type (opB, Lcmp_COM_NE);
	    }
	  else
	    {
	      nextB = NULL;
	      L_fix_cond_br (cb, opB, 1);
	    }
	  STAT_COUNT ("L_local_branch_folding_change", 1, cb);
	  change++;
	}
    }
  /* This opti is good after inlining code - DAC */
  /* The opti can only be applied for BEQ or BNE branches */
  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      int cc;
      /*
       *  match pattern no.4 (cond br with const label operands)
       */
      nextA = opA->next_op;
      if (L_register_branch_opcode (opA))
	break;
      if (!(L_gen_bne_branch_opcode (opA) || L_gen_beq_branch_opcode (opA)))
	continue;
      if (!L_is_label (opA->src[0]) || !L_is_label (opA->src[1]))
	continue;
      if (!L_operand_ctype_same (opA->src[0], opA->src[1]))
	continue;

      /*
       *  replace pattern no.4
       */
#ifdef TEST_BR_FOLD
      fprintf (ERR, "branch folding4: op%d (cb %d)\n", opA->id, cb->id);
#endif
      if ((L_same_operand (opA->src[0], opA->src[1])))
	cc = L_evaluate_int_compare_with_sources (opA, 0, 0);
      else
	cc = L_evaluate_int_compare_with_sources (opA, 0, 1);
      if (cc == 1)
	nextA = NULL;
      L_fix_cond_br (cb, opA, cc);
      STAT_COUNT ("L_local_branch_folding_const_label1", 1, cb);
      change++;
    }

  /* The opti can only be applied for all branches */
  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      int cc = 0, same;
      /*
       *  match pattern no.5 (cond br with same operands)
       */
      nextA = opA->next_op;
      if (!(L_cond_branch_opcode (opA)))
	continue;

      same = L_same_operand (opA->src[0], opA->src[1]);
      if (!same)
	continue;

      /*
       *  replace pattern no.5
       */
#ifdef TEST_BR_FOLD
      fprintf (ERR, "branch folding5: op%d (cb %d)\n", opA->id, cb->id);
#endif
      if (L_int_cond_branch_opcode (opA))
	{
	  cc = L_evaluate_int_compare_with_sources (opA, 0, 0);
	  if (cc == 1)
	    nextA = NULL;
	}
      else if (L_flt_cond_branch_opcode (opA))
	{
	  cc = L_evaluate_flt_compare_with_sources (opA, 0.0, 0.0);
	  if (cc == 1)
	    nextA = NULL;
	}
      else if (L_dbl_cond_branch_opcode (opA))
	{
	  cc = L_evaluate_dbl_compare_with_sources (opA, 0.0, 0.0);
	  if (cc == 1)
	    nextA = NULL;
	}
      else
	{
	  L_punt ("L_local_branch_folding: error pattern no.5 op id %d\n",
		  opA->id);
	}
      L_fix_cond_br (cb, opA, cc);
      STAT_COUNT ("L_local_branch_folding_const_label2", 1, cb);
      change++;
    }

  if (change)
    L_invalidate_dataflow ();

  return change;
}

int
L_local_operation_cancellation (L_Cb * cb)
{
  int change, macro_flag, load_flag, store_flag;
  L_Oper *opA, *opB, *opC;
  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_int_add_opcode (opA) || L_int_sub_opcode (opA)))
	continue;
      if (!(L_same_operand (opA->dest[0], opA->src[0])))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  /* 
	   *  match pattern (x=x-y, x=x+y) or (x=x+y, x=x-y)
	   */
	  if (!(L_cancelling_opcodes (opA, opB)))
	    continue;
	  if (!(L_same_operand (opB->dest[0], opB->src[0])))
	    continue;
	  if (!(L_same_operand (opA->dest[0], opB->dest[0])))
	    continue;
	  if (!(L_same_operand (opA->src[1], opB->src[1])))
	    continue;
	  if (!PG_equivalent_predicates_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  if (!(L_no_uses_between (opA->dest[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[1], opA, opB)))
	    continue;
	  if (!(L_only_disjoint_br_between (opA, opB)))
	    break;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /* 
	   *  replace pattern no. 1
	   */
#ifdef TEST_OP_CANCEL
	  fprintf (ERR, "op cancellation1: %d -> %d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  L_nullify_operation (opA);
	  L_nullify_operation (opB);
	  STAT_COUNT ("L_local_operation_cancellation_1", 1, cb);
	  change++;
	  break;
	}
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_int_add_opcode (opA) || L_int_sub_opcode (opA)))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[0]))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[1]))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  /*
	   *  match pattern 2
	   *  (x=y+z, w=y-x) or (x=y-x, w=z+x)
	   */
	  if (!(L_int_add_opcode (opB) || L_int_sub_opcode (opB)))
	    continue;
	  if (!(L_compatible_arithmetic_ops (opA, opB)))
	    continue;
	  if (!(L_cancelling_opcodes (opA, opB)))
	    continue;
	  if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
		L_same_operand (opA->dest[0], opB->src[1])))
	    continue;
	  if (!(L_same_operand (opA->src[0], opB->src[0]) ||
		L_same_operand (opA->src[1], opB->src[0]) ||
		L_same_operand (opA->src[0], opB->src[1]) ||
		L_same_operand (opA->src[1], opB->src[1])))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[1], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *  replace pattern 2
	   */
#ifdef TEST_OP_CANCEL
	  fprintf (ERR, "op cancellation2: %d -> %d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  /* add followed by flow dependent subtract */
	  if (L_int_add_opcode (opA))
	    {
	      if (L_same_operand (opA->src[0], opB->src[0]))
		{
		  L_delete_operand (opB->src[0]);
		  L_delete_operand (opB->src[1]);
		  opB->src[0] = L_new_gen_int_operand (0);
		  opB->src[1] = L_copy_operand (opA->src[1]);
		}
	      else if (L_same_operand (opA->src[1], opB->src[0]))
		{
		  L_delete_operand (opB->src[0]);
		  L_delete_operand (opB->src[1]);
		  opB->src[0] = L_new_gen_int_operand (0);
		  opB->src[1] = L_copy_operand (opA->src[0]);
		}
	      else if (L_same_operand (opA->src[0], opB->src[1]))
		{
		  L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
				     L_copy_operand (opA->src[1]));
		}
	      else
		{
		  L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
				     L_copy_operand (opA->src[0]));
		}
	      STAT_COUNT ("L_local_operation_cancellation_2", 1, cb);
	      change++;
	    }
	  /* subtract followed by flow dependent add */
	  else
	    {
	      if (L_same_operand (opA->src[1], opB->src[0]) ||
		  L_same_operand (opA->src[1], opB->src[1]))
		{
		  L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
				     L_copy_operand (opA->src[0]));
		  STAT_COUNT ("L_local_operation_cancellation_2", 1, cb);
		  change++;
		}
	      /* Otherwise no opti possible for sub followed by add */
	    }
	}
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_int_add_opcode (opA) || L_int_sub_opcode (opA)))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[0]))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[1]))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  L_Oper *opC;
	  /*
	   *  match pattern 3
	   */
	  if (!(L_int_add_opcode (opA) || L_int_sub_opcode (opB)))
	    continue;
	  if (!(L_compatible_arithmetic_ops (opA, opB)))
	    continue;
	  if (L_same_operand (opB->dest[0], opB->src[0]))
	    continue;
	  if (L_same_operand (opB->dest[0], opB->src[1]))
	    continue;
	  if (!(L_same_operand (opA->src[0], opB->src[0]) ||
		L_same_operand (opA->src[1], opB->src[0]) ||
		L_same_operand (opA->src[0], opB->src[1]) ||
		L_same_operand (opA->src[1], opB->src[1])))
	    continue;
	  if (L_same_operand (opA->dest[0], opB->dest[0]))
	    continue;
	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      if (!(L_int_add_opcode (opC)))
		continue;
	      if (!L_compatible_arithmetic_ops (opA, opC))
		continue;
	      if (!(L_same_operand (opA->dest[0], opC->src[0]) ||
		    L_same_operand (opA->dest[0], opC->src[1])))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opC)))
		continue;
	      if (!(L_same_operand (opB->dest[0], opC->src[0]) ||
		    L_same_operand (opB->dest[0], opC->src[1])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opC))
		continue;
	      if (!PG_superset_predicate_ops (opB, opC))
		continue;
	      if (!(L_no_defs_between (opB->dest[0], opB, opC)))
		continue;
	      if (!(L_same_def_reachs (opA->src[0], opA, opC)))
		continue;
	      if (!(L_same_def_reachs (opA->src[1], opA, opC)))
		continue;
	      if (!(L_same_def_reachs (opB->src[0], opB, opC)))
		continue;
	      if (!(L_same_def_reachs (opB->src[1], opB, opC)))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]) ||
			    L_is_fragile_macro (opA->src[1]) ||
			    L_is_fragile_macro (opB->dest[0]) ||
			    L_is_fragile_macro (opB->src[0]) ||
			    L_is_fragile_macro (opB->src[1]));
	      load_flag = 0;
	      store_flag = 0;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opC)))
		break;
	      /*
	       *  replace pattern 3 
	       */
	      if (L_int_add_opcode (opA) && L_int_sub_opcode (opB))
		{
		  if (!(L_same_operand (opA->src[0], opB->src[1]) ||
			L_same_operand (opA->src[1], opB->src[1])))
		    continue;
#ifdef TEST_OP_CANCEL
		  fprintf (ERR,
			   "op cancellation3a: %d -> %d -> %d (cb %d)\n",
			   opA->id, opB->id, opC->id, cb->id);
#endif
		  L_delete_operand (opC->src[0]);
		  L_delete_operand (opC->src[1]);
		  if (L_same_operand (opA->src[0], opB->src[1]))
		    {
		      opC->src[0] = L_copy_operand (opA->src[1]);
		      opC->src[1] = L_copy_operand (opB->src[0]);
		    }
		  else
		    {
		      opC->src[0] = L_copy_operand (opA->src[0]);
		      opC->src[1] = L_copy_operand (opB->src[0]);
		    }
		  STAT_COUNT ("L_local_operation_cancellation_3", 1, cb);
		  change++;
		}
	      else if (L_int_sub_opcode (opA) && L_int_add_opcode (opB))
		{
		  if (!(L_same_operand (opA->src[1], opB->src[0]) ||
			L_same_operand (opA->src[1], opB->src[1])))
		    continue;
#ifdef TEST_OP_CANCEL
		  fprintf (ERR,
			   "op cancellation3b: %d -> %d -> %d (cb %d)\n",
			   opA->id, opB->id, opC->id, cb->id);
#endif
		  L_delete_operand (opC->src[0]);
		  L_delete_operand (opC->src[1]);
		  if (L_same_operand (opA->src[1], opB->src[0]))
		    {
		      opC->src[0] = L_copy_operand (opA->src[0]);
		      opC->src[1] = L_copy_operand (opB->src[1]);
		    }
		  else
		    {
		      opC->src[0] = L_copy_operand (opA->src[0]);
		      opC->src[1] = L_copy_operand (opB->src[0]);
		    }
		  STAT_COUNT ("L_local_operation_cancellation_3", 1, cb);
		  change++;
		}
	      else if (L_int_sub_opcode (opA) && L_int_sub_opcode (opB))
		{
		  if (!(L_same_operand (opA->src[0], opB->src[1]) ||
			L_same_operand (opA->src[1], opB->src[0])))
		    continue;
#ifdef TEST_OP_CANCEL
		  fprintf (ERR,
			   "op cancellation3c: %d -> %d -> %d (cb %d)\n",
			   opA->id, opB->id, opC->id, cb->id);
#endif
		  L_delete_operand (opC->src[0]);
		  L_delete_operand (opC->src[1]);
		  if (L_same_operand (opA->src[0], opB->src[1]))
		    {
		      L_change_opcode (opC, Lop_SUB);
		      opC->src[0] = L_copy_operand (opB->src[0]);
		      opC->src[1] = L_copy_operand (opA->src[1]);
		    }
		  else
		    {
		      L_change_opcode (opC, Lop_SUB);
		      opC->src[0] = L_copy_operand (opA->src[0]);
		      opC->src[1] = L_copy_operand (opB->src[1]);
		    }
		  STAT_COUNT ("L_local_operation_cancellation_3", 1, cb);
		  change++;
		}
	      else
		{
		  continue;
		}
	    }
	}
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_int_add_opcode (opA)))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[0]))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[1]))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int common_indexA, common_indexB;
	  /*
	   *  match pattern 4
	   * x = y + C1
	   * w = y + C2
	   */
	  if (!(L_int_add_opcode (opB)))
	    continue;
	  if (!(L_compatible_arithmetic_ops (opA, opB)))
	    continue;
	  if (L_same_operand (opB->dest[0], opB->src[0]))
	    continue;
	  if (L_same_operand (opB->dest[0], opB->src[1]))
	    continue;
	  /* if (!(L_same_operand (opA->src[0], opB->src[0])))
	     continue; */
	  if (L_same_operand (opA->dest[0], opB->dest[0]))
	    continue;

	  if (L_same_operand (opA->src[0], opB->src[0]))
	    {
	      common_indexA = 0;
	      common_indexB = 0;
	    }
	  else if (L_same_operand (opA->src[0], opB->src[1]))
	    {
	      common_indexA = 0;
	      common_indexB = 1;
	    }
	  else if (L_same_operand (opA->src[1], opB->src[0]))
	    {
	      common_indexA = 1;
	      common_indexB = 0;
	    }
	  else if (L_same_operand (opA->src[1], opB->src[1]))
	    {
	      common_indexA = 1;
	      common_indexB = 1;
	    }
	  else
	    continue;

	  if (common_indexA == 0)
	    {
	      if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		continue;
	      else
		{
		}
	    }
	  else
	    {
	      if (!(L_same_def_reachs (opA->src[1], opA, opB)))
		continue;
	    }

	  /* if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	     continue; */
	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      if (!(L_int_sub_opcode (opC)))
		continue;
	      if (!(L_compatible_arithmetic_ops (opA, opC)))
		continue;
	      if (!(L_same_operand (opA->dest[0], opC->src[0]) ||
		    L_same_operand (opA->dest[0], opC->src[1])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opC))
		continue;
	      if (!PG_superset_predicate_ops (opB, opC))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opC)))
		continue;
	      if (!(L_same_operand (opB->dest[0], opC->src[0]) ||
		    L_same_operand (opB->dest[0], opC->src[1])))
		continue;
	      if (!(L_no_defs_between (opB->dest[0], opB, opC)))
		continue;
	      if (!
		  (L_same_def_reachs
		   (opA->src[(1 - common_indexA)], opA, opC)))
		continue;
	      if (!
		  (L_same_def_reachs
		   (opB->src[(1 - common_indexB)], opB, opC)))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]) ||
			    L_is_fragile_macro (opA->src[1]) ||
			    L_is_fragile_macro (opB->dest[0]) ||
			    L_is_fragile_macro (opB->src[0]) ||
			    L_is_fragile_macro (opB->src[1]));
	      load_flag = 0;
	      store_flag = 0;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opC)))
		break;
	      /*
	       *  replace pattern 4
	       */
#ifdef TEST_OP_CANCEL
	      fprintf (ERR, "op cancellation4: op%d -> %d -> %d (cb %d)\n",
		       opA->id, opB->id, opC->id, cb->id);
#endif

	      if (L_same_operand (opC->src[0], opA->dest[0]))
		{
		  L_delete_operand (opC->src[0]);
		  L_delete_operand (opC->src[1]);
		  opC->src[0] =
		    L_copy_operand (opA->src[(1 - common_indexA)]);
		  opC->src[1] =
		    L_copy_operand (opB->src[(1 - common_indexB)]);
		}
	      else
		{
		  L_delete_operand (opC->src[0]);
		  L_delete_operand (opC->src[1]);
		  opC->src[0] =
		    L_copy_operand (opB->src[(1 - common_indexB)]);
		  opC->src[1] =
		    L_copy_operand (opA->src[(1 - common_indexA)]);
		}
	      STAT_COUNT ("L_local_operation_cancellation_4", 1, cb);
	      change++;
	    }
	}
    }

  return change;
}

/*
 * L_local_dead_code_removal
 * ----------------------------------------------------------------------
 * Remove NOPs, moves from a register to itself, and stores of a
 * trivially redundant value.
 */

int
L_local_dead_code_removal (L_Cb * cb)
{
  int change;
  L_Oper *nextA, *nextB, *opA, *opB;

  change = 0;
  for (opA = cb->first_op; opA; opA = nextA)
    {
      nextA = opA->next_op;

      if (L_is_opcode (Lop_NO_OP, opA))
	{

#ifdef TEST_DEAD_CODE
	  fprintf (ERR, "dead code (noop): op%d (cb %d)\n", opA->id, cb->id);
#endif
	  L_delete_oper (cb, opA);
	  STAT_COUNT ("L_local_dead_code_removal_nops", 1, cb);
	  change++;
	}
      else if (L_general_move_opcode (opA))
	{
	  if (!L_same_operand (opA->dest[0], opA->src[0]))
	    continue;

#ifdef TEST_DEAD_CODE
	  fprintf (ERR, "dead code (mov): op%d (cb %d)\n", opA->id, cb->id);
#endif
	  L_delete_oper (cb, opA);
	  STAT_COUNT ("L_local_dead_code_removal_movs", 1, cb);
	  change++;
	}
      else if (L_load_opcode (opA) && !L_EXTRACT_BIT_VAL (opA->flags,
							  L_OPER_VOLATILE)) 
	{
	  for (opB = opA->next_op; opB; opB = nextB)
	    {
	      int macro_flag, load_flag, store_flag;
	      nextB = opB->next_op;
	      /* 
	       *  match pattern no. 3 (load, store)
	       */
	      if (!(L_store_opcode (opB)))
		continue;
	      /* 02/07/03 REK Adding a check to make sure we don't touch a
	       *              volatile oper. */
	      if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		continue;
	      if (!(L_compatible_load_store (opA, opB)))
		continue;
	      if (!(L_same_operand (opA->dest[0], opB->src[2])))
		continue;
	      if (!(L_same_operand (opA->src[0], opB->src[0])))
		continue;
	      if (!(L_same_operand (opA->src[1], opB->src[1])))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!(L_no_defs_between (opA->dest[0], opA, opB)))
		break;
	      if (!(L_same_def_reachs (opB->src[0], opA, opB)))
		continue;
	      if (!(L_same_def_reachs (opB->src[1], opA, opB)))
		continue;
	      if (!(L_no_overlap_write (cb, opA, opA, 0, opB, 1)))
		break;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]) ||
			    L_is_fragile_macro (opA->src[1]));
	      load_flag = 0;
	      store_flag = 1;
	      if (!
		  (L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
		break;
	      /*
	       *  replace pattern no. 3
	       */
#ifdef TEST_DEAD_CODE
	      fprintf (ERR, "dead code (load/store): op%d -> op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif
	      L_delete_oper (cb, opB);
	      STAT_COUNT ("L_local_dead_code_removal_ldst", 1, cb);
	      change++;
	    }
	}
    }

  return change;
}


int
L_is_oper_between (L_Oper * target, L_Oper * opA, L_Oper * opB)
{
  L_Oper *ptr;

  for (ptr = opA; ptr; ptr = ptr->next_op)
    {
      if (ptr == opB)
	return 0;

      if (ptr == target)
	return 1;
    }

  L_punt ("L_is_oper_between: can't find boundary op or target op");
  return (0);
}


int
L_local_code_motion (L_Cb * cb)
{
  int change;
  L_Oper *opA, *opB, *opC, *opD, *nextA, *nextB;
  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      nextA = opA->next_op;
      if (!(L_move_opcode (opA)))
	continue;
      if (!(L_is_variable (opA->src[0])))
	continue;
      if (L_same_operand (opA->dest[0], opA->src[0]))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;
	  /* 
	   *  match pattern no. 1 (copy prop code motion)
	   */
	  /* after certain combination of motions may get opA = opB */
	  if (opA == opB)
	    break;
	  if (!(L_is_src_operand (opA->dest[0], opB)))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  if (L_same_def_reachs (opA->src[0], opA, opB))
	    continue;
	  if (!(L_no_sb_loop_br_between (cb, opA, opB)))	/* Hack, 10-94 */
	    continue;
	  opC = L_next_def (opA->src[0], opA);
	  if (opC == opB)
	    continue;
	  if (!(L_is_oper_between (opC, opA, opB)))
	    continue;
	  /* Don't allow code motion across jsr do just set both flags to 1 */
	  macro_flag = 1;
	  load_flag = 1;
	  store_flag = 1;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opC, opB)))
	    break;

	  /* KMC - 5/98 -- Apparently in some cases opC can be null and
	   * this causes problems if it gets into L_can_move_above.
	   */
	  if (opC != NULL && opB != NULL)
	    {
	      if (L_can_move_above (cb, opC, opB))
		{
		  /*
		   *  replace pattern no. 1 with an upward code motion
		   */
#ifdef TEST_CODE_MOTION
		  fprintf (ERR,
			   "code motion (copy prop) op%d moved above op%d "
			   "(cb %d)\n", opB->id, opC->id, cb->id);
#endif
		  L_move_oper_before (cb, opB, opC);
		  STAT_COUNT ("L_local_code_motion_1", 1, cb);
		  change++;
		}
	      else if (L_can_move_below (cb, opB, opC))
		{
		  /*
		   *  replace pattern no. 1 with an downward code motion
		   */
#ifdef TEST_CODE_MOTION
		  fprintf (ERR,
			   "code motion (copy prop) op%d moved below op%d "
			   "(cb %d)\n", opC->id, opB->id, cb->id);
#endif
		  L_move_oper_after (cb, opC, opB);
		  STAT_COUNT ("L_local_code_motion_1", 1, cb);
		  change++;
		}
	      else
		{
		  /*
		   *  no code motion possible
		   */
		  continue;
		}
	    }
	}
    }

  /* SER 05/16/03: This can really screw up redundancy recognition,
   * especially on memory ops, so don't perform on mem ops when not doing
   * post-increment conversion. */
  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      nextA = opA->next_op;
      if (!(L_increment_operation (opA)))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;
	  /* 
	   *  match pattern no. 2 (op folding code motion)
	   */
	  /* after certain combination of motions may get opA = opB */
	  if (opA == opB)
	    break;
	  if (!((Lopti_do_post_inc_conv &&
		 (L_load_opcode (opB) || L_store_opcode (opB))) ||
		L_int_comparison_opcode (opB) ||
		L_int_pred_comparison_opcode (opB) ||
		L_int_add_opcode (opB) || L_int_sub_opcode (opB)))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!(L_is_numeric_constant (opB->src[0]) ||
		L_is_numeric_constant (opB->src[1])))
	    continue;
	  if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
		L_same_operand (opA->dest[0], opB->src[1])))
	    continue;
	  /* DMG 9 Jun 95 - For opB=store, not legal if src2 same */
	  if (L_same_operand (opA->dest[0], opB->src[2]))
	    continue;
	  if (!PG_equivalent_predicates_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  if (L_same_operand (opA->dest[0], opB->dest[0]))
	    continue;
	  if (!(L_no_sb_loop_br_between (cb, opA, opB)))     /* Hack, 10-94 */
	    continue;
	  /* SAM 2-97: additional check to make sure undo_inc will be legal */
	  if (!L_can_undo_increment (opA, opB))
	    continue;
	  /* Don't allow code motion across jsr do just set both flags to 1 */
	  macro_flag = 1;
	  load_flag = 1;
	  store_flag = 1;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  if (L_can_move_above (cb, opA->next_op, opB))
	    {
	      /*
	       *  replace pattern no. 2 with an upward code motion
	       */
	      L_unmark_as_pre_post_increment (opA);
	      L_undo_increment (opA, opB);
	      L_move_oper_before (cb, opB, opA);
#ifdef TEST_CODE_MOTION
	      fprintf (ERR,
		       "code motion (op fold) op%d moved above op%d (cb %d)\n",
		       opB->id, opA->id, cb->id);
#endif
	      STAT_COUNT ("L_local_code_motion_op_fold", 1, cb);
	      change++;
	    }
	  else if (L_can_move_below (cb, opB->prev_op, opA))
	    {
	      /*
	       *  replace pattern no. 2 with an downward code motion
	       */
	      L_unmark_as_pre_post_increment (opA);
	      L_undo_increment (opA, opB);
	      L_move_oper_after (cb, opA, opB);
#ifdef TEST_CODE_MOTION
	      fprintf (ERR,
		       "code motion (op fold) op%d moved below op%d (cb %d)\n",
		       opA->id, opB->id, cb->id);
#endif
	      STAT_COUNT ("L_local_code_motion_op_fold", 1, cb);
	      change++;
	    }
	  else
	    {
	      /*
	       *  no code motion possible
	       */
	      continue;
	    }
	}
    }

  /* SER 01/28/03: For normal & reverse copy prop enabling.
   * This can get rid of unnecessary moves in some situations. */
  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      nextA = opA->next_op;
      if (L_is_opcode (Lop_DEFINE, opA))
	continue;
      if (L_is_macro (opA->dest[0]))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;
	  if (opA == opB)
	    break;

	  /* From here on, checks for code motion opportunity. */
	  if (!(L_move_opcode (opB)))
	    continue;
	  if (L_is_macro (opB->dest[0]))
	    continue;
          if (!(L_is_dest_operand (opB->src[0], opA)))
            continue;
	  if (!PG_superset_predicate_ops (opB, opA))
	    continue;
	  if (!(L_no_defs_between (opB->src[0], opA, opB)))
	    continue;
	  if (L_live_outside_cb_after (cb, opB->src[0], opB))
	    continue;
	  if (!(L_live_outside_cb_after (cb, opB->dest[0], opB)))
	    continue;
	  if (!(L_same_def_reachs (opB->dest[0], opA, opB)))
	    continue;
	  if (L_no_uses_between (opB->src[0], opA, opB))
	    continue;
	  if (!(L_no_intersecting_br_between (opA, opB)))
	    continue;
	  macro_flag = 1;
	  load_flag = 1;
	  store_flag = 1;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  if (L_can_move_above (cb, opA->next_op, opB))
	    {
#ifdef TEST_CODE_MOTION
	      fprintf (ERR, "code motion (complete) op %d moved below op %d "
		       "(cb %d)\n", opB->id, opA->id. cb->id);
#endif
	      L_move_oper_after (cb, opB, opA);
	      STAT_COUNT ("L_local_code_motion_compl", 1, cb);
	      change++;
	      break;
	    }
	  else
	    continue;
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      nextA = opA->next_op;
      if (L_is_opcode (Lop_DEFINE, opA))
	continue;
      if (L_is_macro (opA->dest[0]))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;
	  /* 
	   *  match pattern no. 3 (code motion copy prop2-pt1)
	   *          use of opB->dest[0] between opA and opB, no requirement
	   *          on being able to change all later uses.
	   *          see pattern no. 4
	   */
	  if (opA == opB)
	    break;
	  if (!(L_move_opcode (opB)))
	    continue;
	  if (L_is_macro (opB->dest[0]))
	    continue;
	  if (!(L_is_dest_operand (opB->src[0], opA)))
	    continue;
	  if (!PG_superset_predicate_ops (opB, opA))
	    continue;
	  if (!(L_no_defs_between (opB->src[0], opA, opB)))
	    continue;
	  if (L_live_outside_cb_after (cb, opB->src[0], opA))
	    continue;
	  if (!(L_same_def_reachs (opB->dest[0], opA, opB)))
	    continue;
	  if (L_no_uses_between (opB->dest[0], opA, opB))
	    continue;
	  if (!(L_no_intersecting_br_between (opA, opB)))
	    continue;
	  macro_flag = 1;
	  load_flag = 1;
	  store_flag = 1;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  opC = L_prev_use (opB->dest[0], opB);
	  if (L_can_move_above (cb, opA, opC))
	    {
	      /*
	       *  replace pattern no. 3 with an upward code motion
	       */
#ifdef TEST_CODE_MOTION
	      fprintf (ERR,
		       "code motion (copy prop2 pt1) op%d moved above op%d "
		       "(cb %d)\n", opC->id, opA->id, cb->id);
#endif
	      L_move_oper_before (cb, opC, opA);
	      nextA = opA->next_op;	/* Attempt to prevent inf loop */
	      STAT_COUNT ("L_local_code_motion_3", 1, cb);
	      change++;
	      break;
	    }
	  else if (L_can_move_below (cb, opC, opA))
	    {
	      /*
	       *  replace pattern no. 3 with an upward code motion
	       */
#ifdef TEST_CODE_MOTION
	      fprintf (ERR,
		       "code motion (copy prop2 pt1) op%d moved below op%d "
		       "(cb %d)\n", opA->id, opC->id, cb->id);
#endif
	      L_move_oper_after (cb, opA, opC);
	      STAT_COUNT ("L_local_code_motion_3", 1, cb);
	      change++;
	      break;
	    }
	  else
	    {
	      /*
	       *  no code motion possible
	       */
	      continue;
	    }
	}
    }
  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      nextA = opA->next_op;
      if (L_is_opcode (Lop_DEFINE, opA))
	continue;
      if (L_is_macro (opA->dest[0]))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;
	  /* 
	   *  match pattern no. 4 (code motion copy prop2-pt2)
	   */
	  if (!(L_move_opcode (opB)))
	    continue;
	  if (L_is_macro (opB->dest[0]))
	    continue;
	  if (!(L_is_dest_operand (opB->src[0], opA)))
	    continue;
	  if (!PG_superset_predicate_ops (opB, opA))
	    continue;
	  if (!(L_no_defs_between (opB->src[0], opA, opB)))
	    continue;
	  if (L_live_outside_cb_after (cb, opB->src[0], opA))
	    continue;
	  if (!(L_same_def_reachs (opB->dest[0], opA, opB)))
	    continue;
	  if (!(L_no_uses_between (opB->dest[0], opA, opB)))
	    continue;
	  if (L_can_change_all_later_uses_with
	      (cb, opB->src[0], opB->dest[0], opB))
	    continue;
	  if (!(L_no_intersecting_br_between (opA, opB)))
	    continue;
	  if (!(opC = L_next_def (opB->dest[0], opB)))
	    continue;
	  if (!(opD = L_next_use (opB->src[0], opC)))
	    continue;
	  macro_flag = 1;
	  load_flag = 1;
	  store_flag = 1;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opC, opD)))
	    break;
	  if (L_can_move_above (cb, opC, opD))
	    {
#ifdef TEST_CODE_MOTION
	      fprintf (ERR,
		       "code motion (copy prop2 pt2) op%d moved above op%d "
		       "(cb %d)\n", opD->id, opC->id, cb->id);
#endif
	      L_move_oper_before (cb, opD, opC);
	      STAT_COUNT ("L_local_code_motion_4", 1, cb);
	      change++;
	    }
	  else if (L_can_move_below (cb, opD, opC))
	    {
#ifdef TEST_CODE_MOTION
	      fprintf (ERR,
		       "code motion (copy prop2 pt2) op%d moved below op%d "
		       "(cb %d)\n", opC->id, opD->id, cb->id);
#endif
	      L_move_oper_after (cb, opC, opD);
	      STAT_COUNT ("L_local_code_motion_4", 1, cb);
	      change++;
	    }
	  else
	    {
	      /*
	       *  no code motion possible
	       */
	      continue;
	    }

	}
    }

  if (change)
    L_invalidate_dataflow ();

  return change;
}

/*
 *  Note: This optimization may not be safe in all cases.
 *
 *  This optimization has been made safer by ensuring that the code sequence
 *  is not associated with a cast.
 */
int
L_local_remove_sign_extension (L_Cb * cb)
{
  int change;
  L_Oper *opA, *opB, *opC;
  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_is_opcode (Lop_LSL, opA)))
	continue;
      if (!(L_is_register (opA->src[0])))
	continue;
      if (!(L_is_int_24 (opA->src[1])))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /* 
	   *  match pattern
	   */
	  if (!(L_is_opcode (Lop_ASR, opB)))
	    continue;
	  if (!(L_is_int_24 (opB->src[1])))
	    continue;
	  if (!(L_same_operand (opA->dest[0], opB->src[0])))
	    continue;
	  if (!PG_equivalent_predicates_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	    continue;
	  if (L_find_attr (opA->attr, "cast"))
	    /* This attribute is added by HtoL when the LSL is associated with
	     * a cast operation.  This optimization should not be applied if
	     * the code sequence is associated with a cast. */
	    continue;
	  /*
	   *  replace pattern
	   */
	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      if (L_is_opcode (Lop_ST_C, opC))
		{
		  if (!(L_same_operand (opC->src[2], opB->dest[0])))
		    continue;
		  if (!PG_superset_predicate_ops (opB, opC))
		    continue;
		  if (!(L_no_defs_between (opB->dest[0], opB, opC)))
		    continue;
		  if (!(L_same_def_reachs (opA->src[0], opA, opC)))
		    continue;
		  macro_flag = (L_is_fragile_macro (opA->src[0]) ||
				L_is_fragile_macro (opB->dest[0]) ||
				L_is_fragile_macro (opB->src[0]));
		  load_flag = 0;
		  store_flag = 0;
		  if (!(L_no_danger (macro_flag, load_flag, store_flag,
				     opA, opC)))
		    break;
#ifdef TEST_REMOVE_SIGN_EXT
		  fprintf (ERR,
			   "Remove sign ext: op%d -> op%d -> op%d (cb %d)\n",
			   opA->id, opB->id, opC->id, cb->id);
#endif
		  L_delete_operand (opC->src[2]);
		  opC->src[2] = L_copy_operand (opA->src[0]);
		  change++;
		}
	      else if (L_int_cond_branch_opcode (opC))
		{
		  int is_s1, is_s2;
		  if (!(L_same_operand (opB->dest[0], opC->src[0]) ||
			L_same_operand (opB->dest[0], opC->src[1])))
		    continue;
		  if (!PG_superset_predicate_ops (opB, opC))
		    continue;
		  if (!(L_no_defs_between (opB->dest[0], opB, opC)))
		    continue;
		  if (!(L_same_def_reachs (opA->src[0], opA, opC)))
		    continue;
		  is_s1 = L_same_operand (opB->dest[0], opC->src[0]);
		  is_s2 = L_same_operand (opB->dest[0], opC->src[1]);
		  if ((is_s1)
		      && (!(L_is_int_between_0_and_128 (opC->src[1]))))
		    continue;
		  if ((is_s2)
		      && (!(L_is_int_between_0_and_128 (opC->src[0]))))
		    continue;
		  macro_flag = (L_is_fragile_macro (opA->src[0]) ||
				L_is_fragile_macro (opB->dest[0]) ||
				L_is_fragile_macro (opB->src[0]));
		  load_flag = 0;
		  store_flag = 0;
		  if (!(L_no_danger (macro_flag, load_flag, store_flag,
				     opA, opC)))
		    break;
#ifdef TEST_REMOVE_SIGN_EXT
		  fprintf (ERR,
			   "Remove sign ext: op%d -> op%d -> op%d (cb %d)\n",
			   opA->id, opB->id, opC->id, cb->id);
#endif
		  if (is_s1)
		    {
		      L_delete_operand (opC->src[0]);
		      opC->src[0] = L_copy_operand (opA->src[0]);
		    }
		  else
		    {
		      L_delete_operand (opC->src[1]);
		      opC->src[1] = L_copy_operand (opA->src[0]);
		    }
		  change++;
		}
	    }
	}
    }

  return change;
}

/*
 * L_local_register_renaming
 * ----------------------------------------------------------------------
 * Remove output dependences by renaming
 */

int
L_local_register_renaming (L_Cb * cb)
{
  int i, change;
  L_Oper *opA;
  L_Operand *new_reg, *dest;
  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (L_pred_define_opcode (opA))
	continue;
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  /*
	   *  Match pattern
	   */
	  if (!(dest = opA->dest[i]))
	    continue;
	  if (!(L_is_register (dest)))
	    continue;
	  if (!(L_is_redefined_in_cb_after (opA, dest)))
	    continue;
	  if (L_live_outside_cb_after (cb, dest, opA))
	    continue;
	  if (!L_can_change_dest_operand (opA, dest))	/* JEM 3/10/95 */
	    continue;
	  if (!(L_all_uses_can_be_renamed (cb, opA, dest)))
	    continue;

	  /*
	   *  Replace pattern
	   */

#ifdef TEST_REG_RENAMING
	  fprintf (ERR, "register renaming : op%d (cb %d)\n",
		   opA->id, cb->id);
#endif
	  new_reg = L_new_register_operand (++L_fn->max_reg_id,
					    L_return_old_ctype (dest),
					    dest->ptype);

	  opA->dest[i] = new_reg;

	  L_rename_subsequent_uses (opA->next_op, opA, dest, new_reg);

	  L_delete_operand (dest);
	  change++;
	}
    }

  if (change)
    L_invalidate_dataflow ();

  return change;
}

/*
 *      complex migration should be set only for single issue processors, 
 *      allows for more advanced operation migration when set.  For higher
 *      issue processors, extra optimization probably not worth it.
 */
int
L_local_operation_migration (L_Cb * cb, int complex_mig)
{
  int change, i, j, safe, redef, use_new_op;
  L_Oper *opA, *next, *opB, *ptr, *new_op;
  change = 0;

  for (opA = cb->first_op; opA != NULL; opA = next)
    {
      int macro_flag, load_flag, store_flag;
      next = opA->next_op;

      /*
       *  Match pattern
       */
      if (!L_safe_to_delete_opcode (opA) ||
	  L_is_predicated (opA) ||
	  L_pred_define_opcode (opA))
	continue;

      safe = 1;
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!opA->dest[i])
	    continue;
	  if (!L_is_register (opA->dest[i]) ||
	      !L_not_live_at_cb_end (cb, opA, opA->dest[i]) ||
	      L_conditionally_redefined_in_cb (cb, opA, opA->dest[i]))
	    {
	      safe = 0;
	      break;
	    }
	}

      if (!safe)
	continue;

      if (!(L_no_flow_dep_from (cb, opA)))
	continue;

      macro_flag = L_has_fragile_macro_src_operand (opA);
      load_flag = L_general_load_opcode (opA);
      store_flag = L_general_store_opcode (opA);
      if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, cb->last_op)))
	continue;
      if ((L_load_opcode (opA) || L_store_opcode (opA)) &&
	  !L_no_overlap_write (cb, opA, opA, 0, cb->last_op, 1))
	continue;

      if (!(L_profitable_for_migration (cb, opA)))
	{
#ifdef TEST_OP_MIG
	  fprintf (ERR, "**Not profitable to migrate op %d (cb %d)\n",
		   opA->id, cb->id);
#endif
	  continue;
	}

      /*
       *  Replace pattern
       */

      /* Case 1 (simple migration): no anti deps from opA before all dest of
       * opA are redefined 
       */
      if (L_no_anti_dep_from_before_redef (cb, opA))
	{
#ifdef TEST_OP_MIG
	  fprintf (ERR,
		   "operation migration (no anti) op%d (cb %d) (pr %d)\n",
		   opA->id, cb->id, L_is_predicated (opA));
	  L_print_oper (ERR, opA);
#endif
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!opA->dest[i])
		continue;
	      for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
		{
		  /* if opA->dest[i] redefined, break this loop */
		  redef = 0;
		  for (j = 0; j < L_max_dest_operand; j++)
		    {
		      if (L_same_operand (opA->dest[i], ptr->dest[j]))
			{
			  redef = 1;
			  break;
			}
		    }
		  if (redef)
		    break;
		  /* at each branch, if opA->dest[i] in live-out, insert copy
		     of op in the target cb */
		  if (!(L_general_branch_opcode (ptr) ||
			L_check_branch_opcode (ptr)))
		    continue;
		  if (!(PG_intersecting_predicates_ops (opA, ptr)))
		    continue;
		  if (!
		      (L_in_oper_OUT_set (cb, ptr, opA->dest[i], TAKEN_PATH)))
		    continue;

		  L_insert_op_at_dest_of_br (cb, ptr, opA, 1);
		}
	    }
	  L_delete_oper (cb, opA);

	  /* Does this need to happen here???? -- YES -- KMC 10/9/97 */

	  L_do_flow_analysis (L_fn, LIVE_VARIABLE);
	  PG_setup_pred_graph (L_fn);

	  change++;
	}

      /* Case 2 (complex migrationA): if opA is a move, 
       * allow 1 anti dep if that is an increment op 
       */
      else if ((complex_mig) &&
	       (L_move_opcode (opA)) &&
	       (L_is_variable (opA->src[0])) &&
	       (!(L_same_operand (opA->dest[0], opA->src[0]))) &&
	       (L_single_anti_dep_from_before_redef (opA)))
	{
	  opB = L_next_def (opA->src[0], opA);
	  if (!(L_increment_operation (opB)))
	    continue;
#ifdef TEST_OP_MIG
	  fprintf (ERR, "operation migration (1 antiA) op%d (cb %d)\n",
		   opA->id, cb->id);
#endif
	  /* move const to src2 of opB if not there already */
	  if (L_is_int_constant (opB->src[0]))
	    {
	      L_Operand *temp;
	      temp = opB->src[0];
	      opB->src[0] = opB->src[1];
	      opB->src[1] = temp;
	    }
	  /* create new op which undoes increment of opB */
	  new_op = L_create_new_op (Lop_ADD);
	  new_op->dest[0] = L_copy_operand (opA->dest[0]);
	  new_op->src[0] = L_copy_operand (opA->src[0]);
	  if (L_int_add_opcode (opB))
	    new_op->src[1] = L_new_gen_int_operand (-opB->src[1]->value.i);
	  else
	    new_op->src[1] = L_new_gen_int_operand (opB->src[1]->value.i);
	  use_new_op = 0;
	  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
	    {
	      /* if opA->dest[0] redefined, break this loop */
	      redef = 0;
	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  if (L_same_operand (opA->dest[0], ptr->dest[j]))
		    {
		      redef = 1;
		      break;
		    }
		}
	      if (redef)
		break;
	      if (ptr == opB)
		{
		  use_new_op = 1;
		  continue;
		}
	      /* at each branch, if opA->dest[0] in live-out, insert copy of
	         either op or new_op in the target cb */
	      if (!(L_general_branch_opcode (ptr) ||
		    L_check_branch_opcode (ptr)))
		continue;
	      if (!(L_in_oper_OUT_set (cb, ptr, opA->dest[0], TAKEN_PATH)))
		continue;
	      if (use_new_op)
		L_insert_op_at_dest_of_br (cb, ptr, new_op, 1);
	      else
		L_insert_op_at_dest_of_br (cb, ptr, opA, 1);
	    }
	  L_delete_oper (cb, opA);
	  L_delete_oper (NULL, new_op);

	  /* Does this need to happen here???? */

	  L_do_flow_analysis (L_fn, LIVE_VARIABLE);
	  PG_setup_pred_graph (L_fn);

	  change++;
	}

      /* Case 3 (complex migrationB): 
       * if opA is an ADD, allow 1 anti dep if that is an increment op */
      else if ((complex_mig) &&
	       (L_int_add_opcode (opA)) &&
	       (!(L_same_operand (opA->dest[0], opA->src[0]))) &&
	       (L_is_int_constant (opA->src[1])) &&
	       (L_single_anti_dep_from_before_redef (opA)))
	{
	  opB = L_next_def (opA->src[0], opA);
	  if (!(L_increment_operation (opB)))
	    continue;
#ifdef TEST_OP_MIG
	  fprintf (ERR, "operation migration (1 antiB) op%d (cb %d)\n",
		   opA->id, cb->id);
#endif
	  /* move const to src2 of opB if not there already */
	  if (L_is_int_constant (opB->src[0]))
	    {
	      L_Operand *temp;
	      temp = opB->src[0];
	      opB->src[0] = opB->src[1];
	      opB->src[1] = temp;
	    }
	  /* create new op which undoes increment of opB */
	  new_op = L_create_new_op (Lop_ADD);
	  new_op->dest[0] = L_copy_operand (opA->dest[0]);
	  new_op->src[0] = L_copy_operand (opA->src[0]);
	  if (L_int_add_opcode (opB))
	    new_op->src[1] =
	      L_new_gen_int_operand (
				     (opA->src[1]->value.i -
				      opB->src[1]->value.i));
	  else
	    new_op->src[1] =
	      L_new_gen_int_operand (
				     (opA->src[1]->value.i +
				      opB->src[1]->value.i));
	  use_new_op = 0;
	  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
	    {
	      /* if opA->dest[0] redefined, break this loop */
	      redef = 0;
	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  if (L_same_operand (opA->dest[0], ptr->dest[j]))
		    {
		      redef = 1;
		      break;
		    }
		}
	      if (redef)
		break;
	      if (ptr == opB)
		{
		  use_new_op = 1;
		  continue;
		}
	      /* at each branch, if opA->dest[0] in live-out, insert copy of
	         either op or new_op in the target cb */
	      if (!(L_general_branch_opcode (ptr) ||
		    L_check_branch_opcode (ptr)))
		continue;
	      if (!(L_in_oper_OUT_set (cb, ptr, opA->dest[0], TAKEN_PATH)))
		continue;
	      if (use_new_op)
		L_insert_op_at_dest_of_br (cb, ptr, new_op, 1);
	      else
		L_insert_op_at_dest_of_br (cb, ptr, opA, 1);
	    }
	  L_delete_oper (cb, opA);
	  L_delete_oper (NULL, new_op);

	  /* Does this need to happen here???? */
	  L_do_flow_analysis (L_fn, LIVE_VARIABLE);
	  PG_setup_pred_graph (L_fn);

	  change++;
	}
      /* else no opti possible */
    }

  if (change)
    L_invalidate_dataflow ();

  return change;
}


/*=========================================================================*/
/*
 *      Logic reduction - reduce the number of necessary instructions
 *              for bitfield computations.
 */
/*=========================================================================*/


int
L_local_logic_reduction (L_Cb * cb)
{
  int change = 0, macro_flag, shift_flag, i;
  ITuintmax valueA, valueB, valueC;
  L_Oper *opA, *opB, *opC, *new_op;
    
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_is_opcode (Lop_OR, opA))
	continue;
      if (!L_has_const_operand_and_realign_oper (opA))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!L_has_const_operand_and_realign_oper (opB))
	    continue;

	  /* Pattern: two OR ops */
	  if (L_is_opcode (Lop_OR, opB))
	    {
	      if (!L_same_operand (opA->dest[0], opB->src[0]))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!(L_same_def_reachs (opA->src[0], opA, opB)))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opB->dest[1]));
	      if (!L_no_danger (macro_flag, 0, 0, opA, opB))
		break;
	      /* 
	       * Replace OR, OR pattern
	       */
#ifdef TEST_RED_LOGIC
	      fprintf (ERR, "logic reduction OR op %d -> op %d (cb %d)\n", 
		       opA->id, opB->id, cb->id);
#endif
	      valueA = opA->src[1]->value.i;
	      valueB = opB->src[1]->value.i;
	      L_delete_operand (opB->src[0]);
	      L_delete_operand (opB->src[1]);
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_new_gen_int_operand (valueA | valueB);
	      STAT_COUNT ("L_local_logic_reduction_ORs", 1, cb);
	      change++;
	      continue;
	    }

	  /* We can do a BEQ/BNE optimization as well: if an OR op
             sets something in a bit location that is then compared to
             a constant with 0 in that location, then it is a decideable
             branch. Since no opportunities to do so seem to exist in
             SPECcint 2000, it is currently not implemented */
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_is_opcode (Lop_AND, opA))
	continue;
      if (!L_has_const_operand_and_realign_oper (opA))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (L_is_opcode (Lop_AND, opB))
	    {
	      if (!L_has_const_operand_and_realign_oper (opB))
		continue;
	      if (!L_same_operand (opA->dest[0], opB->src[0]))
		continue;
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	      if (!L_no_defs_between (opA->dest[0], opA, opB))
		continue;
	      if (!L_same_def_reachs (opA->src[0], opA, opB))
		continue;
	      macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			    L_is_fragile_macro (opA->src[0]));
	      if (!L_no_danger (macro_flag, 0, 0, opA, opB))
		break;
	      /* 
	       * Replace AND, AND pattern.
	       */
#ifdef TEST_RED_LOGIC
	      fprintf (ERR, "logic reduction AND op %d -> op %d (cb %d)\n", 
		       opA->id, opB->id, cb->id);
#endif
	      valueA = opA->src[1]->value.i;
	      valueB = opB->src[1]->value.i;
	      L_delete_operand (opB->src[0]);
	      L_delete_operand (opB->src[1]);
	      opB->src[0] = L_copy_operand (opA->src[0]);
	      opB->src[1] = L_new_gen_int_operand (valueA & valueB);
	      STAT_COUNT ("L_local_logic_reduction_ANDs", 1, cb);
	      change++;
	      continue;
	    }

	  /* We can do a BEQ/BNE optimization as well: if an AND op
             clears something in a bit location that is then compared to
             a constant with 1 in that location, then it is a decideable
             branch. Since one opportunity to do so seems to exist in
             SPECcint 2000, it is currently not implemented */

	  /* the following are AND, LSR patterns */
	  if (!L_is_opcode (Lop_LSR, opB))
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->src[0]))
	    continue;
	  if (L_same_operand (opA->dest[0], opB->dest[0]))
	    continue;
	  if (!L_is_int_constant (opB->src[1]))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]));
	  if (!L_no_danger (macro_flag, 0, 0, opA, opB))
	    continue;

	  shift_flag = 0;
	  valueA = opA->src[1]->value.i;
	  valueB = ITicast (opB->src[1]->value.i);
	  for (i = 0; i < valueB; i++)
	    {
	      if (valueA & 0x001)
		{
		  shift_flag = 1;
		  break;
		}
	      valueA = valueA >> 1;
	    }

	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      /* AND, LSR, AND transformation */
	      if (L_is_opcode (Lop_AND, opC))
		{
		  if (!L_has_const_operand_and_realign_oper (opC))
		    continue;
		  if (!L_same_operand (opB->dest[0], opC->src[0]))
		    continue;
		  if (!PG_superset_predicate_ops (opB, opC))
		    continue;
		  if (!L_no_defs_between (opB->dest[0], opB, opC))
		    continue;
		  valueA = opA->src[1]->value.i;
		  valueC = opC->src[1]->value.i;
		  /* 01/23/03 REK Changing this to check that C is a superset
		   *              of A >> B, not the other way around. */
		  /* if (((valueA >> valueB) & valueC) != valueC) */
		  if (((valueA >> valueB) & valueC) != (valueA >> valueB))
		    continue;
		  macro_flag = L_is_fragile_macro (opB->dest[0]);
		  if (!L_no_danger (macro_flag, 0, 0, opB, opC))
		    continue;
#ifdef TEST_RED_LOGIC
		  fprintf (ERR, "shift-and reduction %d to %d in "
			   "(cb %d)\n", opB->id, opC->id, cb->id);
#endif
		  L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
				     L_copy_operand (opB->dest[0]));
		  STAT_COUNT ("L_local_logic_reduction_ASA", 1, cb);
		  change++;
		  continue;
		}

	      /* if shifted out a 1 in the shift, cannot doing the following
	       * two optimizations. */
	      if (shift_flag)
		break;
	      
	      /* AND, LSR, LSL transformation */
	      if (L_is_opcode (Lop_LSL, opC))
		{
		  if (!L_is_int_constant (opC->src[1]))
		    continue;
		  if (!L_same_operand (opB->dest[0], opC->src[0]))
		    continue;
		  if (!PG_superset_predicate_ops (opB, opC))
		    continue;
		  if (!L_no_defs_between (opB->dest[0], opB, opC))
		    continue;
                  if (ITicast (opB->src[1]->value.i) !=
                      ITicast (opC->src[1]->value.i))
                    continue;
		  if (L_same_operand (opB->dest[0], opA->dest[0]))
		    {
		      new_op = L_create_new_op (Lop_MOV);
		      new_op->dest[0] =
			L_new_register_operand (++L_fn->max_reg_id,
						L_native_machine_ctype,
						L_PTYPE_NULL);
		      new_op->src[0] = L_copy_operand (opA->dest[0]);
		      L_insert_oper_after (cb, opA, new_op);
		      L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
					 L_copy_operand (new_op->dest[0]));
		    }
		  else
		    {
		      if (!L_no_defs_between (opA->dest[0], opA, opC))
			continue;
		      L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
					 L_copy_operand (opA->dest[0]));
		    }
#ifdef TEST_RED_LOGIC
		  fprintf (ERR, "left shift removal %d to %d in "
			   "(cb %d)\n", opB->id, opC->id, cb->id);
#endif
		    
		  STAT_COUNT ("L_local_logic_reduction_LSL", 1, cb);
		  change++;
		  continue;
		}

	      /* AND, LSR, BNE/BEQ transformation */
	      if (L_int_bne_branch_opcode (opC) ||
		  (L_int_beq_branch_opcode (opC)))
		{
		  if (!L_has_const_operand_and_realign_oper (opC))
		    continue;
		  if (!L_same_operand (opB->dest[0], opC->src[0]))
		    continue;
		  if (!PG_superset_predicate_ops (opB, opC))
		    continue;
		  if (!L_no_defs_between (opB->dest[0], opB, opC))
		    continue;
		  if (!L_no_defs_between (opA->dest[0], opA, opC))
		    continue;
		  macro_flag = L_is_fragile_macro (opB->dest[0]);
		  if (!L_no_danger (macro_flag, 0, 0, opB, opC))
		    continue;
		  valueC = (opC->src[1]->value.i) << valueB;
#ifdef TEST_RED_LOGIC
		  fprintf (ERR, "right shift removal : op%d -> op%d -> "
			   "op%d in (cb %d)\n",
			   opA->id, opB->id, opC->id, cb->id);
#endif
		  L_delete_operand (opC->src[0]);
		  L_delete_operand (opC->src[1]);
		  opC->src[0] = L_copy_operand (opA->dest[0]);
		  opC->src[1] = L_new_gen_int_operand (valueC);

		  STAT_COUNT ("L_local_logic_reduction_BR", 1, cb);
		  change++;
		  continue;
		}

	    }
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_is_opcode (Lop_LSR, opA))
	continue;
      if (!(L_is_int_constant (opA->src[1]) && L_is_variable (opA->src[0])))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!L_is_opcode (Lop_AND, opB))
	    continue;
	  if (!L_has_const_operand_and_realign_oper (opB))
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->src[0]))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    continue;
	  valueA = (int) ITUINTMAX;
	  valueA = valueA >> ITicast (opA->src[1]->value.i);
	  if ((valueA & (opB->src[1]->value.i)) != valueA)
	    continue;
#ifdef TEST_RED_LOGIC
	  fprintf (ERR, "performing shift-and reduction %d to %d\n",
		   opA->id, opB->id);  
#endif
	  /* Replace second and src register operand with 
	     first and src register operand */
	  L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
			     L_copy_operand (opA->dest[0]));
	  STAT_COUNT ("L_local_logic_reduction_LSR_AND", 1, cb);
	  change++;
	}
    }

  /*  Pattern 6
   *  CMP compare_dest <-
   *  LSL  shift_dest  <- compare_dest << C
   *  AND              <- shift_dest & C (or C & shift_dest)
   *
   *  changed to: MOV  <- shift_dest
   *  if C is 1 in that bit position
   */
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_int_comparison_opcode (opA))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!L_is_opcode (Lop_LSL, opB))
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->src[0]))
	    continue;
	  if (!L_is_int_constant (opB->src[1]))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));
	  if (!L_no_danger (macro_flag, 0, 0, opA, opB))
	    continue;
	  
	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      if (!L_is_opcode (Lop_AND, opC))
		continue;
	      if (!L_has_const_operand_and_realign_oper (opC))
		continue;
	      if (!L_same_operand (opB->dest[0], opC->src[0]))
		continue;
	      if (!PG_superset_predicate_ops (opB, opC))
		continue;
	      if (!L_no_defs_between (opB->dest[0], opB, opC))
		continue;
	      macro_flag = (L_is_fragile_macro (opB->dest[0]));
	      if (!L_no_danger (macro_flag, 0, 0, opB, opC))
		continue;
#ifdef TEST_RED_LOGIC
	      fprintf (ERR, "Performing CMP-AND reduction %d to %d\n",
		       opA->id, opC->id);
#endif

	      valueA = 1 << ITicast (opB->src[1]->value.i);

	      if (valueA & (opC->src[1]->value.i))
		L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
				   L_copy_operand (opB->dest[0]));
	      else
		L_convert_to_move_of_zero (opC, L_copy_operand (opC->dest[0]));
	      
	      STAT_COUNT ("L_local_logic_reduction_CMP", 1, cb);
	      change++;
	    }
	}
    }

  return (change);
}


#if 0
int
L_local_logic_reduction (L_Cb * cb)
{
  ITuintmax and_value, and_value2, and_bits, first_bit;
  ITuintmax branch_value, new_branch_value;
  int shift_value, shift_value2;
  int branch_reg_src;
  int i, change;
  L_Oper *opA, *opB, *opC;
  L_Oper *new_op;
  L_Operand *base;
  L_Operand *and_dest = NULL;
  L_Operand *shift_dest;
  int first_and_reg_index, second_and_reg_index;

  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /* Check for logical and operation */
      if (!L_is_opcode (Lop_AND, opA))
	continue;

      /* Check for integer constant operand */
      if ((L_is_int_constant (opA->src[0])) && (L_is_register (opA->src[1])))
	and_value = opA->src[0]->value.i;
      else if ((L_is_int_constant (opA->src[1]))
	       && (L_is_register (opA->src[0])))
	and_value = opA->src[1]->value.i;
      else
	continue;

      /* Save the and operation destination */
      and_dest = opA->dest[0];

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!(L_is_opcode (Lop_LSR, opB)))
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->src[0]))
	    continue;
	  if (L_same_operand (opA->dest[0], opB->dest[0]))
	    continue;
	  if (!(L_is_int_constant (opB->src[1])))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (and_dest, opA, opB)))
	    continue;
	  /* may not be necessary, can check liveness of br between.. */
	  /*if (!(L_no_br_between (opA, opB)))
	     break; */
	  /* save the shift operation destination */
	  shift_dest = opB->dest[0];
	  shift_value = ITicast (opB->src[1]->value.i);

	  /* Check the shift and the and values */
	  first_bit = 0;
	  and_bits = and_value;
	  for (i = 0; i < shift_value; i++)
	    {
	      if (and_bits & 0x001)
		{
		  first_bit = 1;
		  break;
		}
	      and_bits = and_bits >> 1;
	    }

	  /* Find and transform susceptible branches */

	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      if (first_bit)
		break;
	      if (!PG_superset_predicate_ops (opB, opC))
		continue;
	      if (!(L_int_bne_branch_opcode (opC) ||
		    (L_int_beq_branch_opcode (opC))))
		continue;
	      if (!(L_no_defs_between (shift_dest, opB, opC)))
		continue;

	      if ((L_same_operand (opB->dest[0], opC->src[0])) &&
		  (L_is_int_constant (opC->src[1])))
		{
		  branch_value = opC->src[1]->value.i;
		  branch_reg_src = 0;
		}
	      else if ((L_same_operand (opB->dest[0], opC->src[1])) &&
		       (L_is_int_constant (opC->src[0])))
		{
		  branch_value = opC->src[0]->value.i;
		  branch_reg_src = 1;
		}
	      else
		{
		  continue;
		}

	      /* Check the branch format */

	      if (branch_value != 0)
		{
		  new_branch_value = branch_value << shift_value;

		  if (branch_reg_src == 0)
		    {
		      L_delete_operand (opC->src[1]);
		      opC->src[1] = L_new_gen_int_operand (new_branch_value);
		    }
		  else
		    {
		      L_delete_operand (opC->src[0]);
		      opC->src[0] = L_new_gen_int_operand (new_branch_value);
		    }
		}

#ifdef TEST_RED_LOGIC
	      fprintf (ERR, "right shift removal : op%d -> op%d -> "
		       "op%d in (cb %d)\n",
		       opA->id, opB->id, opC->id, cb->id);
#endif

	      /* Change the branch operands, branch to the and's destination */
	      L_delete_operand (opC->src[branch_reg_src]);
	      opC->src[branch_reg_src] = L_copy_operand (and_dest);
	      STAT_COUNT ("L_local_logic_reduction_1", 1, cb);
	      change++;
	    }

	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      if (first_bit)
		break;
	      if (!(L_is_opcode (Lop_LSL, opC)))
		continue;
	      if (!(PG_superset_predicate_ops (opB, opC)))
		continue;
	      if (!(L_no_defs_between (shift_dest, opB, opC)))
		continue;

	      /* Check for integer constant operand */
	      if ((L_is_int_constant (opC->src[1])) &&
		  (L_is_register (opC->src[0])))
		{
		  shift_value2 = ITicast (opC->src[1]->value.i);
		  base = opC->src[0];
		}
	      else
		{
		  continue;
		}

	      if (!L_same_operand (shift_dest, base))
		continue;

	      if (shift_value != shift_value2)
		continue;

#ifdef TEST_RED_LOGIC
	      fprintf (ERR, "left shift removal %d to %d in "
		       "(cb %d)\n", opB->id, opC->id, cb->id);
#endif

	      /* Shifts are the same, so just move in and_dest */
	      if (L_same_operand (shift_dest, and_dest))
		{
		  new_op = L_create_new_op (Lop_MOV);
		  new_op->dest[0] =
		    L_new_register_operand (++L_fn->max_reg_id,
					    L_native_machine_ctype,
					    L_PTYPE_NULL);
		  new_op->src[0] = L_copy_operand (opA->dest[0]);
		  L_insert_oper_after (cb, opA, new_op);
		  L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
				     L_copy_operand (new_op->dest[0]));
		}
	      else
		{
		  L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
				     L_copy_operand (opB->src[0]));
		}
	      STAT_COUNT ("L_local_logic_reduction_2a", 1, cb);
	      change++;
	    }

	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      ITuintmax and_value2;
	      if (!(L_is_opcode (Lop_AND, opC)))
		continue;
	      if (!(PG_superset_predicate_ops (opB, opC)))
		continue;
	      if (!(L_no_defs_between (shift_dest, opB, opC)))
		continue;

	      /* Check for integer constant operand */
	      if ((L_is_int_constant (opC->src[1])) &&
		  (L_is_register (opC->src[0])))
		{
		  and_value2 = opC->src[1]->value.i;
		  base = opC->src[0];
		}
	      else if ((L_is_int_constant (opC->src[0])) &&
		       (L_is_register (opC->src[1])))
		{
		  and_value2 = opC->src[0]->value.i;
		  base = opC->src[1];
		}
	      else
		{
		  continue;
		}

	      if (!L_same_operand (shift_dest, base))
		continue;

	      if (((and_value >> shift_value) & and_value2) != and_value2)
		continue;
#ifdef TEST_RED_LOGIC
	      fprintf (ERR, "shift-and reduction %d to %d in "
		       "(cb %d)\n", opB->id, opC->id, cb->id);
#endif
	      L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
				 L_copy_operand (opB->dest[0]));
	      STAT_COUNT ("L_local_logic_reduction_3", 1, cb);
	      change++;
	    }
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_is_opcode (Lop_AND, opA))
	continue;

      if ((L_is_int_constant (opA->src[0])) && (L_is_register (opA->src[1])))
	{
	  and_value = opA->src[0]->value.i;
	  first_and_reg_index = 1;
	}
      else if ((L_is_int_constant (opA->src[1]))
	       && (L_is_register (opA->src[0])))
	{
	  and_value = opA->src[1]->value.i;
	  first_and_reg_index = 0;
	}
      else
	{
	  continue;
	}

      /* Save the and operation destination */
      and_dest = opA->dest[0];

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!(L_is_opcode (Lop_AND, opB)))
	    continue;
	  if (!(PG_superset_predicate_ops (opA, opB)))
	    continue;
	  if (!(L_no_defs_between (and_dest, opA, opB)))
	    continue;

	  if ((L_is_int_constant (opB->src[0]))
	      && (L_is_register (opB->src[1])))
	    {
	      and_value2 = opB->src[0]->value.i;
	      second_and_reg_index = 1;
	    }
	  else if ((L_is_int_constant (opB->src[1])) &&
		   (L_is_register (opB->src[0])))
	    {
	      and_value2 = opB->src[1]->value.i;
	      second_and_reg_index = 0;
	    }
	  else
	    {
	      continue;
	    }

	  if (!L_same_operand (and_dest, opB->src[second_and_reg_index]))
	    continue;

	  /* Check and values */
	  if (!((and_value & and_value2) == and_value2))
	    continue;

	  if (!(L_no_defs_between (opA->src[first_and_reg_index], opA, opB)))
	    continue;

#ifdef TEST_RED_LOGIC
	  fprintf (ERR, "and reduction %d to %d in (cb %d) \n", opA->id,
		   opB->id, cb->id);
#endif

	  /* Change 2nd and src register operand to 1st and 
	     src register operand */
	  L_delete_operand (opB->src[second_and_reg_index]);
	  opB->src[second_and_reg_index] =
	    L_copy_operand (opA->src[first_and_reg_index]);
	  STAT_COUNT ("L_local_logic_reduction_4", 1, cb);
	  change++;
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_is_opcode (Lop_LSR, opA))
	continue;

      if ((L_is_int_constant (opA->src[1])) && (L_is_register (opA->src[0])))
	shift_value = ITicast (opA->src[1]->value.i);
      else
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!(L_is_opcode (Lop_AND, opB)))
	    continue;
	  if (!(PG_superset_predicate_ops (opA, opB)))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;

	  /* Check for integer constant operand */
	  if ((L_is_int_constant (opB->src[0])) &&
	      (L_is_register (opB->src[1])))
	    {
	      and_value2 = opB->src[0]->value.i;
	      second_and_reg_index = 1;
	    }
	  else if ((L_is_int_constant (opB->src[1])) &&
		   (L_is_register (opB->src[0])))
	    {
	      and_value2 = opB->src[1]->value.i;
	      second_and_reg_index = 0;
	    }
	  else
	    continue;

	  if (!L_same_operand (opA->dest[0], opB->src[second_and_reg_index]))
	    continue;

	  /* Check and values */
	  and_bits = ITUINTMAX;
	  and_bits = and_bits >> shift_value;

	  if ((and_bits & and_value2) != and_bits)
	    continue;

#ifdef TEST_RED_LOGIC
	  fprintf (ERR, "performing shift-and reduction %d to %d\n",
		   opA->id, opB->id);
#endif
	  /* Replace second and src register operand with 
	     first and src register operand */
	  L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
			     L_copy_operand (opA->dest[0]));
	  STAT_COUNT ("L_local_logic_reduction_5", 1, cb);
	  change++;
	}
    }

  /*  Pattern 6
   *  CMP compare_dest <-
   *  LSL  shift_dest  <- compare_dest << C
   *  AND              <- shift_dest & C (either order)
   *
   *  changed to: MOV  <- shift_dest
   *  if C is 1 in that bit position
   */
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      L_Operand *compare_dest;

      if (!L_int_comparison_opcode (opA))
	continue;

      /* Save the compare operation destination */
      compare_dest = opA->dest[0];

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  /*
	   *      match pattern
	   */
	  if (!(L_is_opcode (Lop_LSL, opB)))
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->src[0]))
	    continue;
	  if (!(L_is_int_constant (opB->src[1])))
	    continue;
	  if (!(PG_superset_predicate_ops (opA, opB)))
	    continue;
	  if (!(L_no_defs_between (compare_dest, opA, opB)))
	    continue;
	  /* may not be necessary, can check liveness of br between.. */
	  /*if (!(L_no_br_between (opA, opB)))
	     break; */

	  /* save the shift operation destination */
	  shift_dest = opB->dest[0];
	  shift_value = ITicast (opB->src[1]->value.i);
	  for (opC = opB->next_op; opC != NULL; opC = opC->next_op)
	    {
	      int and_value2;
	      /*
	       *      match pattern
	       */
	      if (!(L_is_opcode (Lop_AND, opC)))
		continue;
	      if (!(PG_superset_predicate_ops (opB, opC)))
		continue;
	      if (!(L_no_defs_between (shift_dest, opB, opC)))
		continue;

	      /* Check for integer constant operand */
	      if ((L_is_int_constant (opC->src[1])) &&
		  (L_is_register (opC->src[0])))
		{
		  and_value2 = opC->src[1]->value.i;
		  base = opC->src[0];
		}
	      else if ((L_is_int_constant (opC->src[0])) &&
		       (L_is_register (opC->src[1])))
		{
		  and_value2 = opC->src[0]->value.i;
		  base = opC->src[1];
		}
	      else
		continue;

	      if (!L_same_operand (shift_dest, base))
		continue;

#ifdef TEST_RED_LOGIC
	      fprintf (ERR, "performing compare-and reduce %d to %d\n",
		       opA->id, opC->id);
#endif

	      /* Check the shift and the and values */
	      first_bit = 1 << shift_value;
#if 0
	      if (!(first_bit & and_value2))
		continue;

	      if (((~first_bit) & and_value2))
		continue;
#endif
	      if (first_bit & and_value2)
		L_convert_to_move (opC, L_copy_operand (opC->dest[0]),
				   L_copy_operand (opB->dest[0]));
	      else
		L_convert_to_move_of_zero (opC,
					   L_copy_operand (opC->dest[0]));
	      STAT_COUNT ("L_local_logic_reduction_6", 1, cb);
	      change++;
	    }
	}
    }
  return (change);
}
#endif

/*=========================================================================*/
/*
 *      Instruction breakdown - break up complex Lcode instructions that cannot
 *           be tolerated in the target processor to expose more optimization
 *           opportunities.
 */
/*=========================================================================*/

int
L_local_oper_breakdown (L_Cb * cb)
{
  int change;
  L_Oper *opA, *next, *new_op;
  change = 0;

  /*
   *  For now, just break down ld, st, jsr instructions
   */
  for (opA = cb->first_op; opA != NULL; opA = next)
    {
      next = opA->next_op;
      if (L_load_opcode (opA))
	{
	  if (M_num_oper_required_for (opA, L_fn->name) <= 1)
	    continue;
	  /*
	   *  breakdown ld into add followed by load
	   */
	  new_op = L_create_new_op (Lop_ADD);
	  L_insert_oper_before (cb, opA, new_op);
	  new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						    L_native_machine_ctype,
						    L_PTYPE_NULL);
	  new_op->src[0] = opA->src[0];
	  new_op->src[1] = opA->src[1];
	  opA->src[0] = L_copy_operand (new_op->dest[0]);
	  opA->src[1] = L_new_gen_int_operand (0);
#ifdef TEST_OPER_BRKDWN
	  fprintf (ERR, "Break down load op%d into op%d and op%d\n",
		   opA->id, new_op->id, opA->id);
#endif
	  change++;
	}
      else if (L_store_opcode (opA))
	{
	  if (M_num_oper_required_for (opA, L_fn->name) <= 1)
	    continue;
	  /*
	   *  breakdown st into add followed by st
	   */
	  new_op = L_create_new_op (Lop_ADD);
	  L_insert_oper_before (cb, opA, new_op);
	  new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						    L_native_machine_ctype,
						    L_PTYPE_NULL);
	  new_op->src[0] = opA->src[0];
	  new_op->src[1] = opA->src[1];
	  opA->src[0] = L_copy_operand (new_op->dest[0]);
	  opA->src[1] = L_new_gen_int_operand (0);
#ifdef TEST_OPER_BRKDWN
	  fprintf (ERR, "Break down store op%d into op%d and op%d\n",
		   opA->id, new_op->id, opA->id);
#endif
	  change++;
	}
      else if (L_subroutine_call_opcode (opA))
	{
	  if (M_num_oper_required_for (opA, L_fn->name) <= 1)
	    continue;
	  /*
	   *  breakdown jsr into move followed by jsr
	   */
	  new_op = L_create_new_op (Lop_MOV);
	  L_insert_oper_before (cb, opA, new_op);
	  new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						    L_native_machine_ctype,
						    L_PTYPE_NULL);
	  new_op->src[0] = opA->src[0];
	  opA->src[0] = L_copy_operand (new_op->dest[0]);
#ifdef TEST_OPER_BRKDWN
	  fprintf (ERR, "Break down jsr op%d into op%d and op%d\n",
		   opA->id, new_op->id, opA->id);
#endif
	  change++;
	}
      else if (L_is_opcode (Lop_PREF_LD, opA))
	{
	  if (M_num_oper_required_for (opA, L_fn->name) <= 1)
	    continue;
	  /*
	   *  breakdown pref_ld into add followed by pref_load
	   */
	  new_op = L_create_new_op (Lop_ADD);
	  L_insert_oper_before (cb, opA, new_op);
	  new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						    L_native_machine_ctype,
						    L_PTYPE_NULL);
	  new_op->src[0] = opA->src[0];
	  new_op->src[1] = opA->src[1];
	  opA->src[0] = L_copy_operand (new_op->dest[0]);
	  opA->src[1] = L_new_gen_int_operand (0);
#ifdef TEST_OPER_BRKDWN
	  fprintf (ERR, "Break down pref_load op%d into op%d and op%d\n",
		   opA->id, new_op->id, opA->id);
#endif
	  change++;
	}
    }

  return (change);
}

/*=========================================================================*/
/*
 *      Instruction recombining - try to recombine as many ops which were
 *              broken up by L_oper_breakdown because it is likely the
 *              code generator can do more efficient things with them together
 *              if no opti could be applied to take advantage of them being
 *              apart...
 */
/*=========================================================================*/
int
L_local_oper_recombine (L_Cb * cb)
{
  int change;
  L_Oper *opA, *opB;
  change = 0;

  /* 1. Constant propagation */
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_move_opcode (opA)))
	continue;
      if (!(L_is_constant (opA->src[0])))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag, i;
	  /*
	   *  match pattern
	   */
	  if (!(L_is_src_operand (opA->dest[0], opB)))
	    continue;
	  if (!(L_can_change_src_operand (opB, opA->dest[0])))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  macro_flag = L_is_fragile_macro (opA->dest[0]);
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *  replace pattern.
	   */
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_same_operand (opA->dest[0], opB->src[i]))
		{
		  L_delete_operand (opB->src[i]);
		  opB->src[i] = L_copy_operand (opA->src[0]);
#ifdef TEST_OPER_RECOMBINE
		  fprintf (ERR,
			   "Recombine (const prop): op%d -> op%d, (cb %d)\n",
			   opA->id, opB->id, cb->id);
#endif
		  change++;
		}
	    }
	}
    }

  /* 2. Operation folding */
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_int_add_opcode (opA)))
	continue;
      if (!(L_different_operand (opA->dest[0], opA->src[0])))
	continue;
      if (!(L_different_operand (opA->dest[0], opA->src[1])))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern no. 3 (merge add and load/store)
	   */
	  if (!(L_load_opcode (opB) || L_store_opcode (opB)))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!(L_is_int_zero (opB->src[0]) || L_is_int_zero (opB->src[1])))
	    continue;
	  if (!(L_same_operand (opA->dest[0], opB->src[0]) |
		L_same_operand (opA->dest[0], opB->src[1])))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	    continue;
	  if (!(L_same_def_reachs (opA->src[1], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *  replace pattern no. 3
	   */
	  L_delete_operand (opB->src[0]);
	  L_delete_operand (opB->src[1]);
	  opB->src[0] = L_copy_operand (opA->src[0]);
	  opB->src[1] = L_copy_operand (opA->src[1]);
#ifdef TEST_OPER_RECOMBINE
	  fprintf (ERR, "Recombine (op folding): op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
#endif
	  change++;
	}
    }

  return (change);
}

/*
 * Remove operations with constant-false predicates
 */
int
L_local_pred_dead_code_removal (L_Cb * cb)
{
  int change;
  L_Oper *oper, *next;
  change = 0;

  for (oper = cb->first_op; oper != NULL; oper = next)
    {
      next = oper->next_op;

      if (L_pred_define_opcode (oper))
	continue;

      if (!PG_false_predicate_op (oper))
	continue;

#ifdef TEST_DEAD_CODE
      fprintf (ERR, "dead code (constant-false predicate): op%d (cb %d)\n",
	       oper->id, cb->id);
#endif

      if (L_general_branch_opcode (oper) || L_check_branch_opcode (oper))
	{
	  L_Flow *dead_flow, *match_flow;
	  L_Cb *dest_cb;
	  /*
	   * Remove flows attached to deleted branch
	   */
	  dead_flow = L_find_flow_for_branch (cb, oper);
	  dest_cb = dead_flow->dst_cb;
	  match_flow = L_find_matching_flow (dest_cb->src_flow, dead_flow);
	  dest_cb->src_flow = L_delete_flow (dest_cb->src_flow, match_flow);
	  cb->dest_flow = L_delete_flow (cb->dest_flow, dead_flow);
	}

      L_delete_oper (cb, oper);
      change++;
    }
  return change;
}


/*
 * There are two cases where we know that a variable must be a constant even
 * when it is not explicitly set (otherwise handled by constant propagation):
 *   1. After a BNE, we can change all dependent instances of that variable 
 *      to the compared constant. 
 *   2. If all incoming flows to a cb are due to BEQ with the same
 *      var and const or is a fallthrough/jump from a BNE, we can change 
 *      all instances of the var to the const.
 */
int
L_local_branch_val_propagation (L_Cb *cb)
{
  L_Oper *opA, *opB;
  L_Operand *var, *constant, *old_src;
  int i, old_num_oper, new_num_oper, macro_flag, change = 0;

  /* Type 1: modify instructions after bne. */

  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      if (!L_gen_bne_branch_opcode (opA))
	continue;
      if (L_has_unsafe_macro_operand (opA))
	continue;
      if (L_is_constant (opA->src[0]))
	{
	  if (L_is_constant (opA->src[1]))
	    continue;
	  var = opA->src[1];
	  constant = opA->src[0];
	}
      else if (L_is_constant (opA->src[1]))
	{
	  var = opA->src[0];
	  constant = opA->src[1];
	}
      else
        continue;
      macro_flag = L_is_fragile_macro (var);
      for (opB = opA->next_op; opB; opB = opB->next_op)
	{
	  if (!L_is_src_operand (var, opB))
	    continue;
	  if (L_is_macro (var) && M_subroutine_call (opB->opc))
	    continue;
	  if (!L_can_change_src_operand (opB, var))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  if (!L_no_defs_between (var, opA, opB))
	    continue;
	  if (!L_no_danger (macro_flag, 0, 0, opA, opB))
	    break;
	  /* replace pattern */
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_same_operand (var, opB->src[i]))
		{
		  old_num_oper = M_num_oper_required_for (opB, L_fn->name);
		  old_src = opB->src[i];
		  opB->src[i] = L_copy_operand (constant);
		  if ((opB->src[i]->type == L_OPERAND_IMMED) &&
		      ((L_unsigned_int_opcode (opB) ||
			L_shift_opcode (opB)) &&
		       L_native_machine_ctype == L_CTYPE_INT))
		    opB->src[i]->value.i = ((ITuint32) opB->src[i]->value.i);
		  new_num_oper = M_num_oper_required_for (opB, L_fn->name);
		  /* if arch cannot handle opB after prop, undo it */
		  if (new_num_oper > old_num_oper)
		    {
		      L_delete_operand (opB->src[i]);
		      opB->src[i] = old_src;
		      continue;
		    }
		  else
		    {
		      L_delete_operand (old_src);
#ifdef TEST_BRANCH_VAL_PROP
		      fprintf (ERR, "branch val prop: op%d -> op%d, src %d "
			       "(cb %d)\n", opA->id, opB->id, i, cb->id);
#endif
		      change++;
		      STAT_COUNT ("L_local_branch_val_prop_bne", 1, cb);
		    }
		}
	    }
	}
    }

  /* Type 2: modify instructions if all cb incoming flows due to beq
   * or fallthrough from bne. */

  /* First check that all incoming flows correspond to beqs. */
  if (!L_cb_first_flow_const_compare_branch (cb, &var, &constant))
    return change;

  /* Check all flows for same condition. */
  if (!L_cb_all_incoming_flows_same_const_compare (cb, var, constant))
    return change;

  /* If all flows are okay, do transformations. */
  macro_flag = L_is_fragile_macro (var);
  for (opB = cb->first_op; opB; opB = opB->next_op)
    {
      if (!L_is_src_operand (var, opB))
	continue;
      if (L_is_macro (var) && M_subroutine_call (opB->opc))
	continue;
      if (!L_can_change_src_operand (opB, var))
	continue;
      if (!L_no_defs_in_range (var, cb->first_op, opB))
	continue;
      if (!L_no_danger (macro_flag, 0, 0, cb->first_op, opB))
	break;

      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (var, opB->src[i]))
	    {
	      old_num_oper = M_num_oper_required_for (opB, L_fn->name);
	      old_src = opB->src[i];
	      opB->src[i] = L_copy_operand (constant);
	      if ((opB->src[i]->type == L_OPERAND_IMMED) &&
		  ((L_unsigned_int_opcode (opB) ||
		    L_shift_opcode (opB)) &&
		   L_native_machine_ctype == L_CTYPE_INT))
		opB->src[i]->value.i = ((ITuint32) opB->src[i]->value.i);
	      new_num_oper = M_num_oper_required_for (opB, L_fn->name);
	      /* if arch cannot handle opB after prop, undo it */
	      if (new_num_oper > old_num_oper)
		{
		  L_delete_operand (opB->src[i]);
		  opB->src[i] = old_src;
		  continue;
		}
	      else
		{
		  L_delete_operand (old_src);
#ifdef TEST_BRANCH_VAL_PROP
		  fprintf (ERR, "branch val prop: op%d -> op%d, src %d "
			   "(cb %d)\n", opA->id, opB->id, i, cb->id);
#endif
		  change++;
		  STAT_COUNT ("L_local_branch_val_prop_beq", 1, cb);
		}
	    }
	}
    }
  return change;
}
