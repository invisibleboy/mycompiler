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
 *      File :          l_global_opti.c
 *      Description :   global optimization
 *      Info Needed :   dominator info, danger info, live var, avail defn,
 *                              avail expr
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 02/07/03 REK Changing L_global_memory_copy_propagation,
 *              L_global_common_subexpression, L_global_redundant_load, 
 *              L_global_memflow_redundant_load, 
 *              L_global_memflow_redundant_load_with_store,
 *              L_global_memflow_redundant_store, L_global_redundant_store, 
 *              to ignore loads and stores marked volatile. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

#undef TEST_GLOB_DEAD_CODE
#undef TEST_GLOB_CONST_PROP
#undef TEST_GLOB_COPY_PROP
#undef TEST_GLOB_MEM_COPY_PROP
#undef TEST_GLOB_COMMON_SUB
#undef TEST_GLOB_RED_LOAD
#undef TEST_GLOB_RED_STORE
#undef TEST_GLOB_UNNEC_BOOL
#undef TEST_GLOB_BRANCH_VAL_PROP
#undef DEBUG_COMPLETE_STORE_LOAD_REMOVAL
#undef DEBUG_DEAD_STORE_REMOVAL

int
L_global_dead_code_removal (L_Cb * cb)
{
  int i, safe, change, dest_removed;
  L_Oper *opA, *next;
  change = 0;

  for (opA = cb->first_op; opA != NULL; opA = next)
    {
      next = opA->next_op;
      /*
       *  match pattern no. 1
       */
      if (!L_safe_to_delete_opcode (opA))
	continue;
      if (L_general_branch_opcode (opA) || L_check_branch_opcode (opA))
	continue;

      safe = 1;
      dest_removed = 0;
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!opA->dest[i])
	    continue;
	  /* Added intrinsic handling. -ITI/JWJ 8.2.1999 */
	  if (L_intrinsic_opcode (opA))
	    {
	      if ((!L_is_register (opA->dest[i])) &&
		  (!L_is_intrinsic_register (opA->dest[i])))
		{
		  safe = 0;
		  break;
		}
	    }
	  else if (L_is_unsafe_macro (opA->dest[i]))
	    {
	      safe = 0;
	      break;
	    }
	  if (L_in_oper_OUT_set (cb, opA, opA->dest[i], BOTH_PATHS))
	    {
	      safe = 0;
	      break;
	    }
	  else
	    {
	      if (!L_pred_define_opcode (opA))
		continue;
	      L_delete_operand (opA->dest[i]);
	      opA->dest[i] = NULL;
	      L_remove_PD_attr (opA, i);
	      dest_removed = 1;
	    }
	}
      if (safe)
	{
	  /*
	   *  replace pattern no. 1
	   */
#ifdef TEST_GLOB_DEAD_CODE
	  fprintf (stderr, "-> Global dead code op%d (cb %d) : %lf\n",
		   opA->id, cb->id, cb->weight);
#endif
	  L_delete_oper (cb, opA);
	  change++;
	}
      else if (dest_removed)
	{
	  L_compress_pred_dests (opA);
	  change++;
	}
    }

  return change;
}


/* same as conservative_global_dead_code_removal but catches more cases with
   predicates.   Data flow analysis on LIVE_VARIABLE must be done first.
   conservative_global_dead_code_removal only requires LIVE_VARIABLE_CB */

int
L_aggressive_dead_code_removal (L_Cb * cb)
{
  int i, safe, change;
  L_Oper *opA, *next;
#ifdef DEBUG
  int debug = 0;
#endif
  change = 0;

#ifdef DEBUG
  if (cb->id == 3)
    {
      fprintf (stderr, "Aggressive global dead code on cb %d\n", cb->id);
      debug = 1;
    }
#endif

  for (opA = cb->first_op; opA != NULL; opA = next)
    {
      next = opA->next_op;
      /*
       *  match pattern no. 1
       */
#ifdef DEBUG
      if (debug)
	{
	  fprintf (stderr, "   Checking op %d\n", opA->id);
	}
#endif

      if (!L_safe_to_delete_opcode (opA))
	continue;
      if (L_general_branch_opcode (opA) || L_check_branch_opcode (opA))
	continue;

      safe = 1;
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!opA->dest[i])
	    continue;
	  if (!L_is_register (opA->dest[i]))
	    {
	      safe = 0;
	      break;
	    }
	  if (L_in_oper_OUT_set (cb, opA, opA->dest[i], BOTH_PATHS))
	    {
#ifdef DEBUG
	      if (debug)
		{
		  fprintf (stderr, "   op %d ", opA->id);
		  L_print_operand (stderr, opA->dest[i], 0);
		  fprintf (stderr, " is in the OUT set\n");
		}
#endif
	      safe = 0;
	      break;
	    }
	}
      if (!safe)
	continue;
      /*
       *  replace pattern no. 1
       */
#ifdef TEST_GLOB_DEAD_CODE
      fprintf (stderr, "-> Aggressive Global dead code op%d (cb %d) : %lf\n",
	       opA->id, cb->id, cb->weight);
      L_print_oper (stderr, opA);
#endif
      L_delete_oper (cb, opA);
      change++;
    }
  return change;
}

int
L_global_constant_propagation (L_Cb * cbA, L_Cb * cbB, int immed_only)
{
  int change, i, old_num_oper, new_num_oper;
  L_Oper *opA, *opB;
  L_Operand *old_src;
  change = 0;
  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_move_opcode (opA))
	continue;
      if (!L_is_constant (opA->src[0]))
	continue;
      if (immed_only && (opA->src[0]->type != L_OPERAND_IMMED))
	continue;
      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern
	   */
	  if (!L_is_src_operand (opA->dest[0], opB))
	    continue;
	  if (!L_can_change_src_operand (opB, opA->dest[0]))
	    continue;
	  if (!L_global_no_defs_between (opA->dest[0], cbA, opA, cbB, opB))
	    continue;

          /* KVM : Do not substitute constants if opB has the do_not_constant_fold
           * flag.
           */
          if(L_find_attr(opB->attr, "do_not_constant_fold")) {
            //printf("Continuing for op %d\n", opB->id);
            continue;
          }
          if(opB->opc == Lop_ADD_CARRY || opB->opc == Lop_ADD_CARRY_U ||
             opB->opc == Lop_SUB_CARRY || opB->opc == Lop_SUB_CARRY_U ||
             opB->opc == Lop_MUL_WIDE || opB->opc == Lop_MUL_WIDE_U)
            continue;

	  macro_flag = L_is_fragile_macro (opA->dest[0]);
	  load_flag = 0;
	  store_flag = 0;

	  if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
	    {
	      /* Conventional global constant_propagation */
	      if (!L_global_no_danger (macro_flag, load_flag, store_flag, cbA,
				       opA, cbB, opB))
		break;

	      /*
	       *  replace pattern
	       */
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (L_same_operand (opA->dest[0], opB->src[i]))
		    {
		      old_num_oper =
			M_num_oper_required_for (opB, L_fn->name);
		      old_src = opB->src[i];
		      opB->src[i] = L_copy_operand (opA->src[0]);
		      if ((opB->src[i]->type == L_OPERAND_IMMED) &&
			  ((L_unsigned_int_opcode (opB) ||
			    L_shift_opcode (opB)) &&
			   L_native_machine_ctype == L_CTYPE_INT))
			opB->src[i]->value.i =
			  ((ITuint32) opB->src[i]->value.i);
		      new_num_oper =
			M_num_oper_required_for (opB, L_fn->name);
		      if (new_num_oper > old_num_oper)
			{
			  L_delete_operand (opB->src[i]);
			  opB->src[i] = old_src;
			  continue;
			}
		      else
			{
			  L_delete_operand (old_src);
			}
#ifdef TEST_GLOB_CONST_PROP
		      fprintf (stderr, "Global const prop: op%d (cb%d) -> "
			       "op%d src%d (cb%d) : %lf\n",
			       opA->id, cbA->id, opB->id, i, cbB->id,
			       cbB->weight);
#endif
		      STAT_COUNT ("L_global_constant_propagation_conv", 1, 
				  cbB);
		      change++;
		    }
		}
	    }
#if 0
	  else
	    {
	      if (!L_global_no_danger_to_boundary (macro_flag, load_flag,
						   store_flag, cbA, opA,
						   cbB, opB))
		break;

	      if (opB->opc == Lop_DEFINE)
		{
		  /* 
		   * The oper in cbB is a liveness boundary oper, so the
		   * operand cannot be replaced.  Let's just copy the move
		   * operand instead and it will be pushed outside the region.
		   */
		  new_oper = L_copy_operation (opA);
		  L_insert_oper_before (cbB, (L_Oper *)
					L_region_boundary_insert_point (cbB),
					new_oper);
		  STAT_COUNT ("L_global_constant_propagation_mov", 1, cbB);
		  change++;
#ifdef TEST_GLOB_CONST_PROP
		  fprintf (stderr,
			   "Global const prop (region): "
			   "op%d (cb%d) -> op%d (cb%d) : %lf\n",
			   opA->id, cbA->id, opB->id, cbB->id, cbB->weight);
#endif
		}
	      else
		{
		  /*
		   *  replace pattern
		   */
		  for (i = 0; i < L_max_src_operand; i++)
		    {
		      if (L_same_operand (opA->dest[0], opB->src[i]))
			{
			  old_num_oper =
			    M_num_oper_required_for (opB, L_fn->name);
			  old_src = opB->src[i];
			  opB->src[i] = L_copy_operand (opA->src[0]);
			  new_num_oper =
			    M_num_oper_required_for (opB, L_fn->name);
			  if (new_num_oper > old_num_oper)
			    {
			      L_delete_operand (opB->src[i]);
			      opB->src[i] = old_src;
			      continue;
			    }
			  L_delete_operand (old_src);
#ifdef TEST_GLOB_CONST_PROP
			  fprintf (stderr,
				   "Global const prop (region): "
				   "op%d (cb%d) -> op%d src%d (cb%d) : %lf\n",
				   opA->id, cbA->id, opB->id, i, cbB->id,
				   cbB->weight);
#endif
			  STAT_COUNT ("L_global_constant_propagation_repl", 1,
				      cbB);
			  change++;
			}
		    }
		}
	    }
#endif
	}
    }
  return change;
}


/* This is similar to local branch value propagation.
 * If there are any modifications/errors in global constant propagation,
 * need to check here as well: very similar. */
int
L_global_branch_val_propagation (L_Cb * cbA, L_Cb *cbB)
{
  L_Oper *opB;
  L_Operand *var, *constant, *old_src;
  int i, old_num_oper, new_num_oper, macro_flag, change = 0;

  /* SER: Avoid CB exit boundaries for the time being. */
  if (L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
    return 0;
  if (!(cbA->first_op && cbB->first_op))
    return 0;

  /* If all incoming flows to a block are from BEQs of a variable with
   * a constant or a jump/fallthrough after BNE of a variable with a
   * constant, can change future uses of the variable to the constant.
   * This implementation doesn't cover all cases: a true value-flow
   * optimization would be far superior. */

  /* Check first flow's branch condition to see if it qualifies. */
  if (!L_cb_first_flow_const_compare_branch (cbA, &var, &constant))
    return 0;

  /* Check all flows for same condition. */
  if (!L_cb_all_incoming_flows_same_const_compare (cbA, var, constant))
    return 0;

  /* Check no defs of var between cbA and cbB. */
  if (!(L_global_no_defs_between_cb_only (cbA, cbB, var)))
    return 0;

  /* Perform optimization on any valid instructions in cbB. */
  /* The above checks guarantee that var == constant at the beginning of the
   * cb, so now we run it the same way as the local opti version. */
  macro_flag = L_is_fragile_macro (var);
  for (opB = cbB->first_op; opB; opB = opB->next_op)
    {
      if (!L_is_src_operand (var, opB))
	continue;
      if (L_is_macro (var) && M_subroutine_call (opB->opc))
	continue;
      if (!L_can_change_src_operand (opB, var))
	continue;
      if (!L_no_defs_in_range (var, cbB->first_op, opB))
	continue;
      if (!L_global_no_danger (macro_flag, 0, 0, cbA, cbA->first_op, cbB, opB))
	break;
	  
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (var, opB->src[i]))
	    {
	      old_num_oper = M_num_oper_required_for (opB, L_fn->name);
	      old_src = opB->src[i];
	      opB->src[i] = L_copy_operand (constant);
	      if ((opB->src[i]->type == L_OPERAND_IMMED) &&
		  ((L_unsigned_int_opcode (opB) || L_shift_opcode (opB)) &&
		   L_native_machine_ctype == L_CTYPE_INT))
		opB->src[i]->value.i = ((ITuint32) opB->src[i]->value.i);
	      new_num_oper = M_num_oper_required_for (opB, L_fn->name);
	      if (new_num_oper > old_num_oper)
		{
		  L_delete_operand (opB->src[i]);
		  opB->src[i] = old_src;
		  continue;
		}
	      L_delete_operand (old_src);
#ifdef TEST_GLOB_BRANCH_VAL_PROP
	      fprintf (stderr, "Global branch val prop: (cb%d) "
		       "-> op%d src%d (cb%d) : %lf\n", 
		       cbA->id, opB->id, i, cbB->id, cbB->weight);
#endif
	      STAT_COUNT ("L_global_branch_val_prop_beq", 1, cbB);
	      change++;
	    }
	}
    }

  return change;
}


int
L_global_copy_propagation (L_Cb * cbA, L_Cb * cbB)
{
  int change, i;
  L_Oper *opA, *opB, *new_oper;
  change = 0;
  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_move_opcode (opA))
	continue;
      if (!L_is_variable (opA->src[0]))
	continue;
      if (L_has_unsafe_macro_operand (opA))
	continue;
      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern
	   */
	  if (!L_is_src_operand (opA->dest[0], opB))
	    continue;
	  if (!L_can_change_src_operand (opB, opA->dest[0]))
	    continue;
	  if (!L_global_no_defs_between (opA->dest[0], cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_same_def_reachs (opA->src[0], cbA, opA, cbB, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));
	  load_flag = 0;
	  store_flag = 0;

	  if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
	    {
	      /* Conventional global copy propagation */
	      if (!L_global_no_danger
		  (macro_flag, load_flag, store_flag, cbA, opA, cbB, opB))
		break;

	      /*
	       *  replace pattern
	       */
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (L_same_operand (opA->dest[0], opB->src[i]))
		    {
#ifdef TEST_GLOB_COPY_PROP
		      fprintf (stderr,
			       "-> Global copy prop: op%d (cb%d) -> "
			       "op%d src%d (cb%d) : %lf\n",
			       opA->id, cbA->id, opB->id, i, cbB->id,
			       cbB->weight);
		      if (cbA->region != cbB->region)
			Lopti_inter_region_global_copy_prop_wgt +=
			  cbB->weight;
#endif
		      L_delete_operand (opB->src[i]);
		      opB->src[i] = L_copy_operand (opA->src[0]);
		      STAT_COUNT ("L_global_copy_propagation_repl", 1, NULL);
		      change++;
		    }
		}
	      /* make flow analysis conservative, so don't have to redo it!! */
	      L_remove_from_all_EIN_set (opB);
	    }
	  else
	    {
	      if (!L_global_no_danger_to_boundary (macro_flag, load_flag,
						   store_flag,
						   cbA, opA, cbB, opB))
		break;

	      if (opB->opc == Lop_DEFINE)
		{
		  /* 
		   * The oper in CbB is a liveness boundary oper, so the
		   * operand cannot be replace.  Let's just copy the move
		   * operand instead and it will be pushed outside the region.
		   */
		  int safe = 1;
		  for (i = 0; i < L_max_dest_operand; i++)
		    {
		      if (!opA->dest[i])
			continue;
		      if (!L_is_register (opA->dest[i]))
			{
			  safe = 0;
			  break;
			}
		    }
		  if (safe == 1)
		    {
		      new_oper = L_copy_operation (opA);
		      L_insert_oper_before (cbB,
					    (L_Oper *)
					    L_region_boundary_insert_point
					    (cbB), new_oper);
		      STAT_COUNT ("L_global_copy_propagation_mov", 1, NULL);
		      change++;
#ifdef TEST_GLOB_COPY_PROP
		      fprintf (stderr,
			       "Global copy prop (region): op%d (cb%d) -> "
			       "op%d src%d (cb%d) : %lf\n",
			       opA->id, cbA->id, opB->id, i, cbB->id,
			       cbB->weight);
#endif
		    }
		}
	      else
		{
		  /*
		   *  replace the oper in the boundary cb normally.
		   */
		  for (i = 0; i < L_max_src_operand; i++)
		    {
		      if (L_same_operand (opA->dest[0], opB->src[i]))
			{
#ifdef TEST_GLOB_COPY_PROP
			  fprintf (stderr,
				   "Global copy prop (region): op%d (cb%d) -> "
				   "op%d src%d (cb%d) : %lf\n",
				   opA->id, cbA->id, opB->id, i, cbB->id,
				   cbB->weight);
#endif
			  L_delete_operand (opB->src[i]);
			  opB->src[i] = L_copy_operand (opA->src[0]);
			  STAT_COUNT ("L_global_copy_propagation_norm", 1,
				      NULL);
			  change++;
			}
		    }
		}
#if 0	      /* Don't need to do this, because the expression is not available
				   outside the boundary block.  :)  */
	      /* make flow analysis conservative, so don't have to redo it!! */
	      L_remove_from_all_EIN_set (opB);
#endif
	    }

	}
    }
  return change;
}


#if 0
int
L_global_memory_copy_propagation (L_Cb * cbA, L_Cb * cbB)
{
  int change;
  L_Oper *opA, *opB;
  change = 0;
  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_store_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
       *              store. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;

      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *  match pattern
	   */
	  if (!L_load_opcode (opB))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a volatile
	   *              load. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_compatible_load_store (opA, opB))
	    continue;
	  /*
	     if (! L_same_operand(opA->src[0], opB->src[0]))
	     continue;
	     if (! L_same_operand(opA->src[1], opB->src[1]))
	     continue;
	   */
	  if (!L_same_memory_location (opA, opB))
	    continue;
	  if (L_has_unsafe_macro_dest_operand (opB))
	    continue;
	  if (!L_global_same_def_reachs (opA->src[0], cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_same_def_reachs (opA->src[1], cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_same_def_reachs (opA->src[2], cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_no_overlap_write (cbA, opA, cbB, opB))
	    break;
	  macro_flag = (L_is_fragile_macro (opA->src[0]) ||
			L_is_fragile_macro (opA->src[1]) ||
			L_is_fragile_macro (opA->src[2]));
	  load_flag = 1;
	  store_flag = 0;
	  if (!L_global_no_danger
	      (macro_flag, load_flag, store_flag, cbA, opA, cbB, opB))
	    break;
	  /*
	   *  replace pattern
	   */

	  if (Lopti_debug_global_opti)
	    fprintf (stderr,
		     "Global mem copy prop: op%d (cb %d) -> "
		     "op%d (cb %d) : %lf\n",
		     opA->id, cbA->id, opB->id, cbB->id, cbB->weight);


	  L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
			     L_copy_operand (opA->src[2]));

	  /* make flow analysis conservative, so don't have to redo it!! */
	  L_remove_from_all_EIN_set (opB);

	  change++;
	}
    }
  return change;
}
#endif


int
L_global_common_subexpression (L_Cb * cbA, L_Cb * cbB, int move_flags)
{
  int i, change, redo, new_opc;
  L_Oper *opA, *opB, *new_op;

  change = 0;

  do
    {
      for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
	{
	  if (!(L_general_arithmetic_opcode (opA) ||
		L_general_move_opcode (opA)))
	    continue;
	  if (!L_different_src_and_dest_operands (opA))
	    continue;
	  for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	    {
	      int macro_flag, load_flag, store_flag;
	      /*
	       *  match pattern
	       */
	      if (!L_compatible_opcodes (opA, opB))
		continue;
	      /* 02/07/03 REK Adding a check to make sure we don't touch a
	       *              volatile oper. */
	      if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
		continue;
	      if (!L_same_src_operands (opA, opB))
		continue;
	      if (!L_different_src_and_dest_operands (opB))
		continue;
	      if (L_has_unsafe_macro_dest_operand (opB))
		continue;
	      if (!L_all_src_operand_global_same_def_reachs
		  (opA, cbA, opA, cbB, opB))
		continue;
	      if (!L_all_dest_operand_global_no_defs_between
		  (opA, cbA, opA, cbB, opB))
		continue;
	      macro_flag = L_has_fragile_macro_operand (opA);
	      load_flag = 0;
	      store_flag = 0;
	      if (!L_global_no_danger
		  (macro_flag, load_flag, store_flag, cbA, opA, cbB, opB))
		break;

	      /*
	       *  replace pattern
	       */
#ifdef TEST_GLOB_COMMON_SUB
	      fprintf (stderr, "Global common sub in func %s:\n", L_fn->name);
	      L_print_oper (stderr, opA);
	      L_print_oper (stderr, opB);
#endif
	      /* if opA and opB identical, just delete opB */
	      if (L_same_dest_operands (opA, opB))
		{
		  L_nullify_operation (opB);
		  /* make flow analysis conservative, so don't have to redo it!! */

		  /*
		   * Don't need to do this for Exit boundary cb's
		   */
		  if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
		    L_remove_from_all_EIN_set (opB);
		  STAT_COUNT ("L_global_common_subexpression_1", 1, cbB);
		}
	      /* move opcode, only optimize if move_flag set */
	      else if (L_general_move_opcode (opA))
		{
		  if (move_flags == 0)
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

		  /* make flow analysis conservative, so don't have to redo it!! */
		  /*
		   * Don't need to do this for Exit boundary cb's
		   */
		  if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
		    L_remove_from_all_EIN_set (opB);
		  STAT_COUNT ("L_global_common_subexpression_2", 1, cbB);
		}
	      /* arithmetic op: if only 1 dest operand, 
	         just reuse opB for move oper */
	      else if (L_num_dest_operand (opA) == 1)
		{
		  for (i = 0; i < L_max_dest_operand; i++)
		    {
		      if (opA->dest[i] != NULL)
			break;
		    }
		  if (i == L_max_dest_operand)
		    L_punt ("L_local_common_subexpression: "
			    "all dest of opA are NULL");
		  L_convert_to_move (opB, L_copy_operand (opB->dest[i]),
				     L_copy_operand (opA->dest[i]));

		  /* make flow analysis conservative, so don't have to redo it!! */
		  /*
		   * Don't need to do this for Exit boundary cb's
		   */
		  if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
		    L_remove_from_all_EIN_set (opB);
		  STAT_COUNT ("L_global_common_subexpression_3", 1, cbB);
		}
	      else
		{
		  for (i = 0; i < L_max_dest_operand; i++)
		    {
		      if (!opA->dest[i] && !opB->dest[i])
			continue;
		      if (!opA->dest[i] || !opB->dest[i])
			L_punt ("L_common_subexpression; illegal op");
		      new_opc =
			L_move_from_ctype (L_return_old_ctype (opB->dest[i]));
		      new_op = L_create_new_op (new_opc);
		      L_insert_oper_after (cbB, opB, new_op);
		      new_op->dest[0] = L_copy_operand (opB->dest[i]);
		      new_op->src[0] = L_copy_operand (opA->dest[i]);
		    }

		  L_nullify_operation (opB);
		  L_do_flow_analysis (L_fn, AVAILABLE_DEFINITION |
				      MEM_AVAILABLE_DEFINITION |
				      AVAILABLE_EXPRESSION);
		  STAT_COUNT ("L_global_common_subexpression_4", 1, cbB);
		}
#ifdef TEST_GLOB_COMMON_SUB
	      fprintf (stderr,
		       "-> Global common sub: op%d (cb %d) -> "
		       "op%d (cb %d) : %lf\n",
		       opA->id, cbA->id, opB->id, cbB->id, cbB->weight);
	      L_print_oper (stderr, opA);
	      L_print_oper (stderr, opB);
	      if (cbA->region != cbB->region)
		Lopti_inter_region_global_common_sub_elim_wgt += cbB->weight;
#endif
	      change++;
	    }
	}

      /* 
       * SER: If L_local_copy_propagation can optimize cbB, then it may have
       * exposed more opportunities to do global_common_subexpression
       */
      redo = L_local_copy_propagation (cbB);
      STAT_COUNT ("L_local_copy_propagation", redo, cbB);
    }
  while (redo);

  return change;
}


#if 0
int
L_global_redundant_load (L_Cb * cbA, L_Cb * cbB)
{
  int i, change, new_opc;
  L_Oper *opA, *opB, *new_op;
  change = 0;
  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_load_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;
      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
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
	  /*
	     if (! L_same_src_operands(opA, opB))
	     continue;
	   */
	  if (!L_same_memory_location (opA, opB))
	    continue;

	  if (!L_all_src_operand_global_same_def_reachs
	      (opA, cbA, opA, cbB, opB))
	    continue;
	  if (!L_all_dest_operand_global_no_defs_between
	      (opA, cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_no_overlap_write (cbA, opA, cbB, opB))
	    break;
	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = 1;
	  store_flag = 0;
	  if (!L_global_no_danger
	      (macro_flag, load_flag, store_flag, cbA, opA, cbB, opB))
	    break;
	  /*    
	   *  replace pattern
	   */
#ifdef TEST_GLOB_RED_LOAD
	  fprintf (stderr,
		   "Global red load: op%d (cb %d) -> op%d (cb %d) : %lf\n",
		   opA->id, cbA->id, opB->id, cbB->id, cbB->weight);
#endif
	  /* if only 1 dest operand, just reuse opB for move oper */
	  if (L_num_dest_operand (opA) == 1)
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (opA->dest[i] != NULL)
		    break;
		}
	      if (i == L_max_dest_operand)
		L_punt ("L_local_common_subexpression: "
			"all dest of opA are NULL");
	      L_convert_to_move (opB, L_copy_operand (opB->dest[i]),
				 L_copy_operand (opA->dest[i]));

	      /* make flow analysis conservative, so don't have to redo it!! */
	      L_remove_from_all_EIN_set (opB);
	    }
	  else
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if ((opA->dest[i] == NULL) & (opB->dest[i] == NULL))
		    continue;
		  if ((opA->dest[i] == NULL) | (opB->dest[i] == NULL))
		    L_punt ("L_common_subexpression; illegal op");
		  new_opc =
		    L_move_from_ctype (L_return_old_ctype (opB->dest[i]));
		  new_opc = L_move_from_ctype (opB->dest[i]->ctype);

		  new_op = L_create_new_op (new_opc);
		  L_insert_oper_after (cbB, opB, new_op);
		  new_op->dest[0] = L_copy_operand (opB->dest[i]);
		  new_op->src[0] = L_copy_operand (opA->dest[i]);
		}
	      L_nullify_operation (opB);
	      L_do_flow_analysis (L_fn, AVAILABLE_DEFINITION |
				  MEM_AVAILABLE_DEFINITION |
				  AVAILABLE_EXPRESSION);
	    }
	  change++;
	}
    }
  return change;
}
#endif


int
L_global_memflow_redundant_load (L_Cb * cbA, L_Cb * cbB, int *inserted)
{
  int change;
  L_Oper *opA, *opB, *new_op;
  L_Operand *src, *dest;
  Set ADEF;

  change = 0;
  *inserted = 0;

  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_general_load_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;

      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  /*
	   *  match pattern
	   */
	  if (!L_general_load_opcode (opB))
	    continue;
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_same_opcode (opA, opB))
	    continue;

	  /* Get all dependent, available loads to opB  */
	  ADEF = L_get_mem_oper_AIN_defining_opers (opB,
						    MDF_RET_DEP |
						    MDF_RET_LOADS);
	  /*
	     fprintf(stderr, "GORLL: Lopti opA id: %d\n", opA->id );
	     Set_print( stderr, "GORLL: LOpti opB ADEF", ADEF );
	   */

	  /* Is opA in opB's set */
	  if (!Set_in (ADEF, opA->id))
	    {
	      /*
	         fprintf( stderr, "GORLL: opA does not reach opB \n");
	         fprintf( stderr, "GORLL: "); DB_spit_oper( opA, "stderr" );
	         fprintf( stderr, "GORLL: "); DB_spit_oper( opB, "stderr" );
	       */
	      Set_dispose (ADEF);
	      continue;
	    }
	  Set_dispose (ADEF);

	  /*
	   *  replace pattern
	   */
	  if (Lopti_debug_global_opti)
	    fprintf (stderr, "\nGORL: op%d (cb %d) -> op%d (cb %d) : %f\n",
		     opA->id, cbA->id, opB->id, cbB->id, cbB->weight);

	  /* 
	     Two cases where a move must be used:
	     1) opA writes a register and the register is redefined
	     2) opA writes a macro (don't use it to propagate across cbs)
	   */
	  if (L_global_no_defs_between (opA->dest[0], cbA, opA, cbB, opB) &&
	      !L_is_macro (opA->dest[0]))
	    {
	      /* yes, simply convert to a move using opA's dest as a src */
	      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_MASK_PE))
		{
		  if (!L_EXTRACT_BIT_VAL (opB->flags, L_OPER_MASK_PE))
		    {
		      /* Insert a check if the one being removed is
		       *  not speculative 
		       */
		      L_global_insert_check_before (opA, cbB, opB);
		    }
		  else
		    {
		      /* OpA needs to "adopt" all of OpB's checks
		       */
		      L_assign_all_checks (L_fn, opB, opA);
		    }
		}
	      L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
				 L_copy_operand (opA->dest[0]));
	      L_remove_from_all_EIN_set (opB);
	    }
	  else
	    {
	      /* no, create a new mov following opA to a new reg */

	      /* make a mov from a new register to opA's dest */
	      src = L_new_register_operand (++L_fn->max_reg_id,
					    L_return_old_ctype (opA->dest[0]),
					    L_PTYPE_NULL);
	      dest = opA->dest[0];

	      /* change opA to write to new register */
	      opA->dest[0] = L_copy_operand (src);

	      /* add new move after opA */
	      new_op = L_create_move_using (dest, src, opA);
	      L_insert_oper_after (cbA, opA, new_op);

	      /* convert Opb to a move */
	      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_MASK_PE))
		{
		  if (!L_EXTRACT_BIT_VAL (opB->flags, L_OPER_MASK_PE))
		    {
		      /* Insert a check if the one being removed is
		       *  not speculative 
		       */
		      L_global_insert_check_before (opA, cbB, opB);
		    }
		  else
		    {
		      /* OpA needs to "adopt" all of OpB's checks
		       */
		      L_assign_all_checks (L_fn, opB, opA);
		    }
		}
	      L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
				 L_copy_operand (opA->dest[0]));

	      *inserted = 1;

	      L_do_flow_analysis (L_fn, AVAILABLE_DEFINITION |
				  MEM_AVAILABLE_DEFINITION |
				  AVAILABLE_EXPRESSION);
	    }
	  change++;
	}
    }
  return change;
}


/* 02/07/03 REK Changing this to leave volatile stores and loads alone. */
int
L_global_memflow_redundant_load_with_store (L_Cb * cbA, L_Cb * cbB,
					    int *inserted)
{
  int change;
  L_Oper *opA, *opB, *new_op;
  L_Operand *src, *dest;
  Set ADEF;
  int dep_flags;

  dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);
  change = 0;
  *inserted = 0;
  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_general_store_opcode (opA))
	continue;
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;

      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  /*
	   *  match pattern
	   */

	  if (!L_general_load_opcode (opB))
	    continue;
	  if (!L_compatible_load_store (opA, opB))
	    continue;
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;

	  /* BCC - check if sync arcs exist - 5/18/99 */
	  if (L_independent_memory_ops2 (cbA, opA, cbB, opB, dep_flags))
	    continue;

	  /* Get all dependent, available loads to opB */
	  ADEF = L_get_mem_oper_AIN_defining_opers (opB,
						    MDF_RET_DEP |
						    MDF_RET_STORES);
	  /*
	     fprintf(stderr,"GORLS: Lopti opA id: %d\n", opA->id );
	     Set_print(stderr, "GORLS: LOpti opB store: ADEF", ADEF );
	   */

	  /* Is opA in opB's set */
	  if (!Set_in (ADEF, opA->id))
	    {
	      /*
	         fprintf( stderr, "GORLS: opA does not reach opB \n");
	         fprintf( stderr, "GORLS: "); DB_spit_oper( opA, "stderr" );
	         fprintf( stderr, "GORLS: "); DB_spit_oper( opB, "stderr" );
	       */
	      Set_dispose (ADEF);
	      continue;
	    }
	  Set_dispose (ADEF);

	  /*
	   *  replace pattern
	   */

	  if (Lopti_debug_global_opti)
	    fprintf (stderr, "\nGORLS: op%d (cb %d) -> op%d (cb %d) : %f\n",
		     opA->id, cbA->id, opB->id, cbB->id, cbB->weight);


	  /* 
	     Two cases where a move must be used:
	     1) opA writes a register and the register is redefined
	     2) opA writes a macro (don't use it to propagate across cbs)
	   */
	  if (L_global_same_def_reachs (opA->src[2], cbA, opA, cbB, opB) &&
	      !L_is_macro (opA->src[2]))
	    {
	      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_SPECULATIVE))
		{
		  if (!L_EXTRACT_BIT_VAL (opB->flags, L_OPER_SPECULATIVE))
		    {
		      /* Insert a check if the one being removed is
		       *  not speculative 
		       */
		      L_global_insert_check_before (opA, cbB, opB);
		    }
		  else
		    {
		      /* OpA needs to "adopt" all of OpB's checks
		       */
		      L_assign_all_checks (L_fn, opB, opA);
		    }
		}
	      /* Check for signed store to unsigned load */
	      /* Need to create a zero extend operation */
	      if (L_load_store_sign_extend_conflict (opA, opB))
		{
		  dest = L_copy_operand (opB->dest[0]);
		  src = L_copy_operand (opA->src[2]);
		  L_convert_to_zero_extend_oper (opB, dest, src);
		}
	      else
		{
		  /* Convert to a move using opA's src[2] as a opB src */
		  L_convert_to_extended_move (opB,
					      L_copy_operand (opB->dest[0]),
					      L_copy_operand (opA->src[2]));
		}
	      L_remove_from_all_EIN_set (opB);
	      STAT_COUNT ("L_global_memory_copy_propagation_reg", 1, NULL);
	    }
	  else
	    {
	      /* no, store src redefined, mov after store needed */
	      dest = L_new_register_operand (++L_fn->max_reg_id,
					     L_return_old_ctype (opA->src[2]),
					     L_PTYPE_NULL);
	      src = opA->src[2];

	      /* change opA to read from a new register */
	      opA->src[2] = L_copy_operand (dest);

	      /* add new move before opA */
	      new_op = L_create_move_using (dest, src, opA);
	      L_insert_oper_before (cbA, opA, new_op);

	      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_SPECULATIVE))
		{
		  if (!L_EXTRACT_BIT_VAL (opB->flags, L_OPER_SPECULATIVE))
		    {
		      /* Insert a check if the one being removed is
		       *  not speculative 
		       */
		      L_global_insert_check_before (opA, cbB, opB);
		    }
		  else
		    {
		      /* OpA needs to "adopt" all of OpB's checks
		       */
		      L_assign_all_checks (L_fn, opB, opA);
		    }
		}
	      /* Check for signed store to unsigned load */
	      /* Need to create a zero extend operation */
	      if (L_load_store_sign_extend_conflict (opA, opB))
		{
		  dest = L_copy_operand (opB->dest[0]);
		  src = L_copy_operand (opA->src[2]);
		  L_convert_to_zero_extend_oper (opB, dest, src);
		}
	      else
		{
		  /* convert OpB to a move */
		  L_convert_to_extended_move (opB,
					      L_copy_operand (opB->dest[0]),
					      L_copy_operand (opA->src[2]));
		}
	      STAT_COUNT ("L_global_memory_copy_propagation_mac", 1, NULL);
	      L_do_flow_analysis (L_fn, AVAILABLE_DEFINITION |
				  MEM_AVAILABLE_DEFINITION |
				  AVAILABLE_EXPRESSION);
	    }
	  change++;
	}
    }

  return change;
}


int
L_global_memflow_redundant_store (L_Cb * cbA, L_Cb * cbB)
{
  int change;
  L_Oper *opA, *opB;
  Set ADEF;

  change = 0;

  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_store_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;

      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  /*    
	   *  match pattern
	   */
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_same_opcode (opA, opB))
	    continue;
	  if (!L_same_src_operands (opA, opB))
	    continue;
	  /* Make sure the src of store A reaches store B */
	  if (!L_global_same_def_reachs (opA->src[2], cbA, opA, cbB, opB))
	    continue;

	  ADEF = L_get_mem_oper_AIN_defining_opers (opB,
						    MDF_RET_DEP |
						    MDF_RET_STORES);
	  /*
	     fprintf(stderr,"GORSS: Lopti opA id: %d\n", opA->id );
	     Set_print( stderr, "GORSS: LOpti opB store: ADEF", ADEF );
	   */

	  /* Is opA in opB's set */
	  if (!Set_in (ADEF, opA->id))
	    {
	      /*
	         fprintf( stderr, "GORSS: opA does not reach opB \n");
	         fprintf( stderr, "GORSS: "); DB_spit_oper( opA, "stderr" );
	         fprintf( stderr, "GORSS: "); DB_spit_oper( opB, "stderr" );
	       */
	      Set_dispose (ADEF);
	      continue;
	    }
	  Set_dispose (ADEF);

	  /*    
	   *  replace pattern
	   */

	  if (Lopti_debug_global_opti)
	    fprintf (stderr, "GORSS: op%d (cb %d) -> op%d (cb %d) : %f\n",
		     opA->id, cbA->id, opB->id, cbB->id, cbB->weight);

	  L_nullify_operation (opB);

	  /* make flow analysis conservative, so don't have to redo it!! */
	  L_remove_from_all_EIN_set (opB);
	  change++;
	}
    }
  return change;
}

#if 0
int
L_global_redundant_store (L_Cb * cbA, L_Cb * cbB)
{
  int change;
  L_Oper *opA, *opB;
  change = 0;
  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_store_opcode (opA))
	continue;
      /* 02/07/03 REK Adding a check to make sure we don't touch a
       *              volatile oper. */
      if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_VOLATILE))
	continue;
      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*    
	   *  match pattern
	   */
	  /* 02/07/03 REK Adding a check to make sure we don't touch a
	   *              volatile oper. */
	  if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_VOLATILE))
	    continue;
	  if (!L_same_opcode (opA, opB))
	    continue;
	  if (!L_same_src_operands (opA, opB))
	    continue;

	  if (!L_all_src_operand_global_same_def_reachs
	      (opA, cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_no_overlap_write (cbA, opA, cbB, opB))
	    break;
	  macro_flag = (L_has_fragile_macro_operand (opA) ||
			L_is_macro (opA->src[0]) || L_is_macro (opA->src[1]));
	  load_flag = 1;
	  store_flag = 1;
	  if (!L_global_no_danger
	      (macro_flag, load_flag, store_flag, cbA, opA, cbB, opB))
	    break;
	  /*    
	   *  replace pattern
	   */
	  fprintf (stderr,
		   "Global red store: op%d (cb %d) -> op%d (cb %d) : %lf\n",
		   opA->id, cbA->id, opB->id, cbB->id, cbB->weight);

	  L_nullify_operation (opB);
	  /* make flow analysis conservative, so don't have to redo it!! */
	  L_remove_from_all_EIN_set (opB);
	  change++;
	}
    }
  return change;
}
#endif


int
L_global_remove_unnec_boolean (L_Cb * cbA)
{
  int change;
  L_Oper *opA, *opB;
  change = 0;
  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      /*    
       *  match pattern (logic, ne to 0)
       */
      if (!L_logic_opcode (opA))
	continue;
      opB = opA->next_op;
      if (!L_is_opcode (Lop_NE, opB))
	continue;
      if (!(L_is_int_zero (opB->src[0]) || L_is_int_zero (opB->src[1])))
	continue;
      if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
	    L_same_operand (opA->dest[0], opB->src[1])))
	continue;
      if (!L_global_only_branch_src_operand (opB->dest[0]))
	continue;
      /*    
       *  replace pattern
       */
#ifdef TEST_GLOB_UNNEC_BOOL
      fprintf (stderr, "Global red load: op%d (cb %d) -> op%d (cb %d)\n",
	       opA->id, cbA->id, opB->id, cbA->id);
#endif
      L_convert_to_move (opB, L_copy_operand (opB->dest[0]),
			 L_copy_operand (opA->dest[0]));

      /* make flow analysis conservative, so don't have to redo it!! */
      L_remove_from_all_EIN_set (opB);
      change++;
    }
  return change;
}

int
L_global_dead_if_then_else_rem (L_Cb * cbA, L_Cb * cbB)	/* ADA 4/28/95 */
{
  int change = 0;
  int flg = 0;
  L_Cb *cbC, *cb;
  L_Flow *flowB, *flowC;
  L_Oper *operB, *operC, *del;

  if (!cbA || !cbB || !(cbC = cbA->next_cb))
    return 0;
  if (!L_int_cond_branch_opcode (cbA->last_op))
    return 0;
  if (cbA->last_op->src[2]->value.cb->id != cbB->id)
    return 0;

  /* 1st: check there are no other entrances to cbB/cbC other than cbA */
  for (flowB = cbB->src_flow; flowB; flowB = flowB->next_flow)
    if (flowB->src_cb->id != cbA->id)
      break;

  if (flowB)
    return 0;

  if (flg == 0)
    return 0;

  for (flowC = cbC->src_flow; flowC; flowC = flowC->next_flow)
    if (flowC->src_cb->id != cbA->id)
      break;

  if (flowC)
    return 0;

  /* 2nd: check if cbB and cbC both have the same dest_flows */
  for (flg = 0, flowB = cbB->dest_flow; flowB;
       flowB = flowB->next_flow, flg++);
  for (flowC = cbC->dest_flow; flowC; flowC = flowC->next_flow, flg--);

  if (flg)
    return 0;

  for (flowB = cbB->dest_flow; flowB; flowB = flowB->next_flow)
    {
      for (flowC = cbC->dest_flow; flowC; flowC = flowC->next_flow)
	if (flowB->cc == flowC->cc && flowB->dst_cb->id == flowC->dst_cb->id)
	  break;

      if (!flowC)
	return 0;
    }

  /* 3rd: check if cbB and cbC are the same, in strict order */
  for (operB = cbB->first_op, operC = cbC->first_op;
       operB && operC; operB = operB->next_op, operC = operC->next_op)
    if (!L_same_operation (operB, operC, 1))
      break;

  if ((operB && operC) ||
      (operB && (!L_uncond_branch (operB) || operB->next_op != NULL)) ||
      (operC && (!L_uncond_branch (operC) || operC->next_op != NULL)))
    return 0;

  /* match pattern */

#if 0
  fprintf (stderr, "Dead if-then-else detected :\n");
  fprintf (stderr, " --- CbA --\n");
  L_print_cb (stderr, NULL, cbA);
  fprintf (stderr, " --- CbB --\n");
  L_print_cb (stderr, NULL, cbB);
  fprintf (stderr, " --- CbC --\n");
  L_print_cb (stderr, NULL, cbC);
#endif

  L_delete_oper (cbA, cbA->last_op);	/* L_nullify_operation(cbA->last_op); */
  /*    L_jump_merge_always_successive_blocks(L_fn, 0); */
  for (flowB = cbB->src_flow; flowB; flowB = flowB->next_flow)
    if ((cb = flowB->src_cb) != NULL)
      cb->dest_flow = L_delete_flow (cb->dest_flow,
				     L_find_flow_with_dst_cb (cb->dest_flow,
							      cbB));

#if 0
  for (flowB = cbB->dest_flow; flowB; flowB = flowB->next_flow)
    if ((cb = flowB->dst_cb) != NULL && flowB->cc != 0)
      cb->src_flow = L_delete_flow (cb->src_flow,
				    L_find_flow_with_src_cb (cb->src_flow,
							     cbB));
#endif

  for (operB = cbB->first_op; operB;)
    if (!L_uncond_branch (operB))
      {
	del = operB;
	operB = operB->next_op;
	L_delete_oper (cbB, del);
      }
    else
      break;
/*    L_delete_all_oper(cbB->first_op, 1);*/

#if 0
  fprintf (stderr, " --- CbB (after) --\n");
  L_print_cb (stderr, NULL, cbB);
#endif

  change++;
  return change;
}


/* Removes loads that obtain values that have been stored. */
int
L_global_mem_expression_copy_prop (L_Func *fn)
{
  L_Cb *store_cb, *load_cb;
  L_Oper *store_op, *load_op, *new_op;
#ifdef DEBUG_COMPLETE_STORE_LOAD_REMOVAL
  L_Expression_Hash_Entry *entry;
#endif
  L_Expression *expression;
  L_Operand *temp_operand, *src_operand;
  Set mem_reaching_expressions, mem_reaching_opers, stores_changed;
  int i, conflict_flag, change = 0, 
    num_matching_stores, *mem_array, store_size, flag;

  if (!Lopti_do_global_mem_expression_copy_prop)
    return 0;

  flag = MEM_REACHING_LOCATIONS;
  flag |= (L_func_contains_dep_pragmas && L_use_sync_arcs &&
	   !Lopti_do_PCE_conservative_memory_opti) ? 0 :
    PCE_MEM_CONSERVATIVE;
  L_do_flow_analysis (fn, flag);
  L_clear_expression_reg_ids (fn);

  mem_array = (int *) Lcode_malloc (sizeof (int) * fn->n_oper);
  stores_changed = NULL;

#ifdef DEBUG_COMPLETE_STORE_LOAD_REMOVAL
  for (i = 1; i <= L_fn->n_expression; i++)
    {
      entry =
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      expression = entry->expression;
      if (!(L_load_opcode (expression) || L_store_opcode (expression)))
	continue;
      L_print_expression (stderr, expression);
    }
#endif

  for (load_cb = fn->first_cb; load_cb; load_cb = load_cb->next_cb)
    {
      for (load_op = load_cb->first_op; load_op; load_op = load_op->next_op)
	{
	  if (!L_load_opcode (load_op))
	    continue;
	  if (L_EXTRACT_BIT_VAL (load_op->flags, L_OPER_VOLATILE))
	    continue;

	  /* Check if corresponding store done along all paths. */
	  mem_reaching_expressions = L_get_mem_oper_AIN_set (load_op);
	  expression = 
	    L_find_corresponding_store_expression_for_load (load_op);
	  if (!expression)
	    continue;
	  if (!(Set_in (mem_reaching_expressions, expression->index)))
	    continue;

	  /* Stores reach this position: find matching stores, transform. */
	  mem_reaching_opers = L_get_oper_RIN_set (load_op);
	  Set_2array (mem_reaching_opers, mem_array);
	  store_size = Set_size (mem_reaching_opers);
	  conflict_flag = num_matching_stores = 0;

#ifdef DEBUG_COMPLETE_STORE_LOAD_REMOVAL
	  fprintf (stderr, "Found corresponding reaching store for load op "
		   "%d:\n", load_op->id);
          L_print_oper (stderr, load_op);
          Set_print (stderr, "MRE", mem_reaching_expressions);
	  L_print_expression (stderr, expression);
	  Set_print (stderr, "MRO", mem_reaching_opers);
#endif

	  for (i = 0; i < store_size; i++)
	    {
	      store_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						    mem_array[i]);
	      if (!(L_oper_matches_expression (store_op, 0, 
					       expression, 1, 0)))
		continue;
	      num_matching_stores++;

#ifdef DEBUG_COMPLETE_STORE_LOAD_REMOVAL
	      fprintf (stderr, "Found corresponding store oper:\n");
	      L_print_oper (stderr, store_op);
#endif
	      /* Here, put in a move to the temp register and change the
	       * store to use the temp register */
	      if (L_load_store_sign_extend_conflict (store_op, load_op))
		conflict_flag = 1;

	      /* Insert move to "temp" variable. */
	      if (!(Set_in (stores_changed, store_op->id)))
		{
		  stores_changed = Set_add (stores_changed, store_op->id);
		  store_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
						      mem_array[i]);
		  temp_operand = L_create_operand_for_expression_index
		    (expression->index);
		  src_operand = store_op->src[2];
                  if (L_is_label (src_operand))
                    {
		      new_op = L_create_new_op (Lop_MOV);
                      new_op->dest[0] = temp_operand;
                      new_op->src[0] = L_copy_operand (src_operand);
                      L_insert_oper_after (store_cb, store_op, new_op);
                    }
		  else
		    {
		      new_op = L_create_move_using (temp_operand,
						    src_operand, store_op);
		      store_op->src[2] = L_copy_operand (temp_operand);
		      L_insert_oper_before (store_cb, store_op, new_op);
                    }
		}
	    }
	  
	  if (!num_matching_stores)
	    L_punt("No matching stores found for op %d!", load_op->id);
	  temp_operand = L_create_operand_for_expression_index
	    (expression->index);
	  /* if sign-ext conflict, have zext, else move */
	  if (conflict_flag)
	    L_convert_to_zero_extend_oper (load_op, 
					   L_copy_operand (load_op->dest[0]),
					   temp_operand);
	  else
	    L_convert_to_extended_move (load_op, 
					L_copy_operand (load_op->dest[0]),
					temp_operand);
	  load_op->flags =
	    L_CLR_BIT_FLAG (load_op->flags, L_OPER_MASK_PE);
	  change++;
	}
    }

  STAT_COUNT ("Lopti_global_mem_expression_copy_prop", change, NULL);

  Lcode_free (mem_array);
  Set_dispose (stores_changed);

  return change;
}


/* Removes stores that are overwritten on later paths.
 * Also removes stores to local vars that aren't used. */
int
L_global_dead_store_removal (L_Func *fn)
{
  L_Cb *cb;
  L_Oper *oper, *next;
#ifdef DEBUG_DEAD_STORE_REMOVAL
  L_Expression_Hash_Entry *entry;
  int i;
#endif
  L_Expression *expression;
  Set anticipable;
  int flag, token, change = 0;

  if (!Lopti_do_global_dead_store_removal)
    return 0;

  flag = MEM_ANT_EXPRESSIONS;
  flag |= (Lopti_do_dead_local_var_store_removal) ? DEAD_LOCAL_MEM_VAR : 0;
  flag |= (L_func_contains_dep_pragmas && L_use_sync_arcs &&
	   !Lopti_do_PCE_conservative_memory_opti) ? 0 :
    PCE_MEM_CONSERVATIVE;
  L_do_flow_analysis (fn, flag);

  change = 0;
#ifdef DEBUG_DEAD_STORE_REMOVAL
  fprintf (stderr, "Beginning global dead store removal on function %s.\n",
	   fn->name);
  for (i = 1; i <= L_fn->n_expression; i++)
    {
      entry =
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      expression = entry->expression;
      if (!L_store_opcode (expression))
	continue;
      L_print_expression (stderr, expression);
      change++;
    }
  if (!change)
    return 0;
  else
    change = 0;
#endif

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = next)
	{
	  next = oper->next_op;
	  if (!L_store_opcode (oper))
	    continue;
	  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	    continue;
	  anticipable = L_get_mem_oper_overwrite_or_dead_set (oper);
	  token = L_generate_expression_token_from_oper (oper);
	  expression = L_find_oper_expression_in_hash
	    (L_fn->expression_token_hash_tbl, token, oper, 1);
	  if (!expression)
	    continue;
	  if (Set_in (anticipable, expression->index))
	    {
#ifdef DEBUG_DEAD_STORE_REMOVAL
	      fprintf (stderr, "Removing following oper:\n");
	      L_print_oper (stderr, oper);
	      fprintf (stderr, "Corresponding to expression:\n");
	      L_print_expression (stderr, expression);
#endif
	      L_delete_oper (cb, oper);
	      STAT_COUNT ("Lopti_global_dead_store_removal", 1, cb);
	      change++;
	    }
	}
    }
  return change;
}
