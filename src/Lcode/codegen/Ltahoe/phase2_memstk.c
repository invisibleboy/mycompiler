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
/*************************************************************************\
 *
 *  File:  phase2_memstk.c
 *
 *  Description:
 *    Memory stack setup and update routines 
 *
 *  Authors: Jim Pierce
 *           Bob McGowan - altered to handle unat and setjmp, a few routines
 *                         were rewritten completely.
 *
\************************************************************************/
/* 09/12/02 REK Updating file to use the new completer scheme and opcode
 *              map.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "phase1_func.h"
#include "phase1_varargs.h"
#include "phase2_reg.h"

/* Local advance declarations
 * ----------------------------------------------------------------------
 */

static void O_implement_unat_swapping (L_Func * fn,
				       int int_swap_offset,
				       int int_swap_space_size,
				       int unat_swap_offset,
				       int stack_frame_size);

/* Stack base operand annotation
 * ----------------------------------------------------------------------
 */

/* var_operand will be either a register or a macro */
/* If have to add a new instruction and change the location of the stack
   operand, the parameter stk_operand will point to the new stack operand. */
static void
O_update_stack_const_in_prev_oper (L_Func * fn, L_Cb * cb, L_Oper * stk_oper,
				   L_Operand ** stk_operand,
				   L_Operand * var_operand, int offset_val,
				   int variable_frame_size)
{
  L_Oper *new_oper;
  Set defs, uses;
  int num_defs, d, num_uses, stk_operand_num;
  int *def_buffer = NULL;
  int adjust_move = 1;

  /* Locate the producer(s) of var_operand. */

  defs = L_get_oper_RIN_defining_opers (stk_oper, var_operand);
  num_defs = Set_size (defs);

  if (num_defs < 1)
    L_punt ("O_update_stack_const_in_prev_oper: "
	    "Could not find a defining operand for op %d.\n", stk_oper->id);

  def_buffer = (int *) Lcode_malloc (sizeof (int) * num_defs);
  Set_2array (defs, def_buffer);
  for (d = 0; d < num_defs; d++)
    {
      L_Oper *def = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
					       def_buffer[d]);

      /* For this def, if it is not a MOVI, then we can't
         adjust it; add the extra instruction below. */
      /* 09/22/03 REK We can try something similar for a MOVL. */

      if ((def->proc_opc != TAHOEop_MOVI ||
	   !(SIMM_22 (def->src[0]->value.i + offset_val))) && \
	  def->proc_opc != TAHOEop_MOVL)
	{
	  adjust_move = 0;
	  break;
	}			/* if */

      /* For this def, if its only use is the stk_oper, then
         it can be adjusted.  Otherwise, add the extra
         instruction below.  Its other uses may also
         require the SAME adjustment, but we'll leave that
         optimization for future implementation. */

      uses = L_get_oper_ROUT_using_opers (def, def->dest[0]);
      num_uses = Set_size (uses);

      Set_dispose (uses);

      if (num_uses < 1)
	L_punt ("O_update_stack_const_in_prev_oper: "
		"Could not find a using operand for op %d.\n", def->id);

      /* If something besides stk_oper uses the mov, make a copy of the mov
       * and adjust that. */
      if (num_uses > 1)
	{
	  adjust_move = 0;
	  break;
	}			/* if */
    }				/* for d */

  if (adjust_move)
    {
      for (d = 0; d < num_defs; d++)
	{
	  L_Oper *def = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   def_buffer[d]);
	  def->src[0]->value.i += offset_val;
	}			/* for d */
    }				/* if */
  else
    {
      /* Couldn't find move immed to var_operand.  Add another 
         instruction which converts LV to offset+SP */

      if (L_add_opcode (stk_oper))
	{
	  /* 09/22/03 REK If var_operand and the stack oper's destination are
	   *              the same, we can't put sp + offset in the
	   *              destination, as this will overwrite the incoming
	   *              address.  Instead, insert an ADD operation to
	   *              add the offset to the incoming address.  The
	   *              original instruction will then add sp. */
	  if (L_same_operand (stk_oper->dest[0], var_operand))
	    {
	      if (SIMM_14 (offset_val))
		{
		  new_oper = L_create_new_op_using (Lop_ADD_U, stk_oper);
		  new_oper->proc_opc = TAHOEop_ADDS;
		  new_oper->src[0] = L_new_gen_int_operand (offset_val);
		}
	      else
		{
		  L_Operand *opd = L_new_gen_int_operand (offset_val),
		    *mac = L_new_macro_operand (TAHOE_MAC_TMPREG1,
						L_CTYPE_LLONG,
						L_PTYPE_NULL);

		  if (L_in_oper_IN_set_reg (stk_oper, 
					    L_MAC_INDEX (TAHOE_MAC_TMPREG1)))
		    L_punt ("O_update_stack_references: (op %d) requires add "
			    "insertion, but TMPREG1 is unavailable", 
			    stk_oper->id);

		  Ltahoe_int_constant_load (cb, stk_oper, opd, mac);
		  L_delete_operand (opd);
		  new_oper = L_create_new_op_using (Lop_ADD_U, stk_oper);
		  new_oper->proc_opc = TAHOEop_ADD;
		  new_oper->src[0] = mac;
		}
	      new_oper->src[1] = L_copy_operand (var_operand);
	      new_oper->dest[0] = L_copy_operand (var_operand);
	      L_insert_oper_before (cb, stk_oper, new_oper);

	      stk_operand_num = L_same_operand (var_operand,
						stk_oper->src[0]) ? 1 : 0;

	      L_delete_operand (stk_oper->src[stk_operand_num]);
	      stk_oper->src[stk_operand_num] = \
		variable_frame_size ? Ltahoe_IMAC (PSP) :
		L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, L_PTYPE_NULL);

	      *stk_operand = stk_oper->src[stk_operand_num];
	    }
	  else
	    {
	      if (SIMM_14 (offset_val))
		{
		  new_oper = L_create_new_op_using (Lop_ADD_U, stk_oper);
		  new_oper->proc_opc = TAHOEop_ADDS;
		  new_oper->src[0] = L_new_gen_int_operand (offset_val);
		  new_oper->src[1] = variable_frame_size ? Ltahoe_IMAC (PSP) :
		    L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, L_PTYPE_NULL);
		  new_oper->dest[0] = L_copy_operand (stk_oper->dest[0]);
		  L_insert_oper_before (cb, stk_oper, new_oper);
		}
	      else
		{
		  L_Operand *opd = L_new_gen_int_operand (offset_val);
		  Ltahoe_int_constant_load (cb, stk_oper, opd,
					    stk_oper->dest[0]);
		  L_delete_operand (opd);
		  new_oper = L_create_new_op_using (Lop_ADD_U, stk_oper);
		  new_oper->proc_opc = TAHOEop_ADD;
		  new_oper->src[0] = L_copy_operand (stk_oper->dest[0]);
		  new_oper->src[1] = variable_frame_size ? Ltahoe_IMAC (PSP) :
		    L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, L_PTYPE_NULL);
		  new_oper->dest[0] = L_copy_operand (stk_oper->dest[0]);
		  L_insert_oper_before (cb, stk_oper, new_oper);
		}

	      *stk_operand = new_oper->src[1];

	      stk_operand_num = L_same_operand (var_operand,
						stk_oper->src[0]) ? 1 : 0;
						
	      L_delete_operand (stk_oper->src[stk_operand_num]);
	      stk_oper->src[stk_operand_num] = \
		L_copy_operand (stk_oper->dest[0]);
	    }

#if 0
	  L_print_oper (stderr, new_oper);
	  L_print_oper (stderr, stk_oper);
#endif
	}			/* if */
      else
	{
	  L_punt ("O_update_stack_const_in_prev_oper: "
		  "Reg define not found in cb (oper %d)\n", stk_oper->id);
	}			/* else */
    }				/* else */

  Lcode_free (def_buffer);
  def_buffer = NULL;
  Set_dispose (defs);

  return;
}				/* O_update_stack_const_in_prev_oper */


/****************************************************************************
 *
 * routine: O_update_stack_references()
 * purpose: checks oper for stack reference and updates reference if necessary
 * input: oper                op to examine and change if needed.
 *        local_offset        offsets into local stack frame of
 *        int_swap_offset     Lcode-exposed storage areas
 *        fp_swap_offset
 *        input_parm_offset
 *        output_parm_offset
 * output: 
 * returns:
 * modified: JEP 2/96 BMG 4/97
 *           JWS 20010209 Adjust stack attribute to be frame-relative
 *-------------------------------------------------------------------------*/

static void
O_update_stack_references (L_Func * fn, L_Cb * cb, L_Oper * oper,
			   int local_offset,
			   int int_swap_offset,
			   int fp_swap_offset,
			   int pred_swap_offset,
			   int input_parm_offset,
			   int output_parm_offset, int variable_frame_size)
{
  L_Operand *stack_operand;
  L_Operand *other_operand;
  L_Attr *stack_attr;
  int operand;
  int adjustment_applied = 0, special_case;
  int uses_stack_pointer;

  /* Don't need to check defines, etc. */
  if (LT_is_non_instr (oper))
    return;

  if (output_parm_offset != 0)
    L_punt ("O_update_stack_references: OP offset must be zero");

  /* check to see if the oper uses a stack pointer. */
  uses_stack_pointer = 0;
  for (operand = 0;
       (operand < L_max_src_operand) && oper->src[operand]; operand++)
    {
      if (M_is_stack_operand_tahoe (oper->src[operand]))
	{
	  uses_stack_pointer = 1;
	  break;
	}			/* if */
    }				/* for operand */

  if ((stack_attr = L_find_attr (oper->attr, "stack")))
    {
      stack_operand = stack_attr->field[0];
      other_operand = stack_attr->field[1];

      switch (stack_operand->value.mac)
	{
	case L_MAC_LV:
	  adjustment_applied = local_offset;
	  break;
	case L_MAC_SP:
	  adjustment_applied = int_swap_offset;
	  break;
	case L_MAC_FP:
	  adjustment_applied = fp_swap_offset;
	  break;
	case L_MAC_OP:
	  adjustment_applied = output_parm_offset;
	  break;
	case L_MAC_IP:
	  adjustment_applied = input_parm_offset;
	  break;
	case TAHOE_MAC_PSP:
	  adjustment_applied = 0;
	  break;
	case TAHOE_MAC_PSPILL:
	  adjustment_applied = pred_swap_offset;
	  break;
	default:
	  L_punt ("O_update_stack_references: "
		  "Unknown stack pointer in attr on op " "%d", oper->id);
	}			/* switch */
      if (!variable_frame_size || (stack_operand->value.mac == L_MAC_OP))
	stack_operand->value.mac = L_MAC_SP;
      else
	stack_operand->value.mac = TAHOE_MAC_PSP;
      other_operand->value.i += adjustment_applied;
    }				/* if */

  if (!uses_stack_pointer)
    return;

  if (L_find_attr (oper->attr, "ALLOCA"))
    return;

  /* Check for mov ap = sp  -  don't want to update this */

  if ((L_is_macro (oper->dest[0])) &&
      (oper->dest[0]->value.mac == TAHOE_MAC_AP))
    return;

  stack_operand = oper->src[operand];
  other_operand = operand ? oper->src[0] : oper->src[1];

  adjustment_applied = 0;
  special_case = 0;

  switch (stack_operand->value.mac)
    {
    case L_MAC_LV:
      adjustment_applied = local_offset;
      if (!L_is_constant (other_operand))
	{
	  special_case = 1;
	  if (L_is_variable (other_operand))
	    O_update_stack_const_in_prev_oper (fn, cb, oper,
					       &stack_operand,
					       other_operand,
					       adjustment_applied,
					       variable_frame_size);
	  else
	    L_punt ("O_update_stack_references: ADD trouble on oper %d",
		    oper->id);
	}			/* if */
      break;
    case L_MAC_SP:
      adjustment_applied = int_swap_offset;
      if (!L_is_macro (oper->dest[0]) ||
	  (oper->dest[0]->value.mac != TAHOE_MAC_TMPREG1))
	L_punt ("O_update_stack_references: ADD trouble on oper:%d",
		oper->id);
      break;
    case L_MAC_FP:
      adjustment_applied = fp_swap_offset;
      if (!L_is_macro (oper->dest[0]) ||
	  (oper->dest[0]->value.mac != TAHOE_MAC_TMPREG1))
	L_punt ("O_update_stack_references: ADD trouble on oper:%d",
		oper->id);
      break;
    case L_MAC_IP:
      adjustment_applied = input_parm_offset;
      break;
    case TAHOE_MAC_PSP:
    case L_MAC_OP:
      adjustment_applied = 0;
      special_case = 1;
      break;
    case TAHOE_MAC_PSPILL:
      adjustment_applied = pred_swap_offset;
      break;
    default:
      L_punt ("O_update_stack_references: Unknown stack pointer on op "
	      "%d", oper->id);
    }				/* switch */

  if ((stack_operand->value.mac != L_MAC_OP) && (oper->opc != Lop_ADD))
    L_punt ("O_update_stack_references: Stack pointer used on non add op %d",
	    oper->id);

  if (!special_case)
    {
      ITintmax new_ofst;
      if (L_is_constant (other_operand) &&
	  SIMM_14((new_ofst = other_operand->value.i + adjustment_applied)))
	{
	  other_operand->value.i = new_ofst;
	}
      else if (SIMM_22(adjustment_applied))
	{
	  L_Oper *new_op;
	  L_Operand *dst;

	  if (L_in_oper_IN_set_reg (oper, L_MAC_INDEX (TAHOE_MAC_TMPREG1)))
	    L_punt ("O_update_stack_references: (op %d) requires add "
		    "insertion, but TMPREG1 is unavailable", oper->id);

	  dst = oper->dest[0];
	  oper->dest[0] = Ltahoe_IMAC (TMPREG1);
	  
	  new_op = L_create_new_op (Lop_ADD);
	  L_insert_oper_after (cb, oper, new_op);
	  new_op->proc_opc = TAHOEop_ADDL;
	  new_op->pred[0] = L_copy_operand (oper->pred[0]);
	  new_op->dest[0] = dst;
	  new_op->src[1] = Ltahoe_IMAC (TMPREG1);
	  new_op->src[0] = L_new_gen_int_operand (adjustment_applied);
	}
      else
	{
	  L_punt ("O_update_stack_references: (op %d) requires stack "
		  "adjustment larger than SIMM22", oper->id);
	}
    }				/* if */
  /* change all aliases of the SP to SP */
  if (stack_operand->value.mac != TAHOE_MAC_PSP)
    {
      if (!variable_frame_size || (stack_operand->value.mac == L_MAC_OP))
	stack_operand->value.mac = L_MAC_SP;
      else
	stack_operand->value.mac = TAHOE_MAC_PSP;
    }				/* if */

  return;
}				/* O_update_stack_references */

/* Preserved predicate register spill/fill
 * ----------------------------------------------------------------------
 */

/* mov rX = pr */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_pred_save_operation (L_Operand * pred, L_Operand * dest)
{
  L_Oper *mov_oper;

  mov_oper = L_create_new_op (Lop_MOV);
  mov_oper->proc_opc = TAHOEop_MOV_FRPR;
  mov_oper->src[0] = L_new_macro_operand (TAHOE_PRED_BLK_REG, L_CTYPE_LLONG,
					  L_PTYPE_NULL);
  mov_oper->dest[0] = dest;
  mov_oper->pred[0] = pred;
  return (mov_oper);
}				/* O_pred_save_operation */

/* mov pr = rX,mask */
      /* only restore the callee save predicates */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_pred_restore_operation (L_Operand * pred, L_Operand * src, ITintmax mask)
{
  L_Oper *mov_oper;

  mov_oper = L_create_new_op (Lop_MOV);
  mov_oper->proc_opc = TAHOEop_MOV_TOPR;
  mov_oper->src[0] = src;
  mov_oper->src[1] = L_new_gen_int_operand (mask);
  mov_oper->dest[0] = L_new_macro_operand (TAHOE_PRED_BLK_REG, L_CTYPE_LLONG,
					   L_PTYPE_NULL);
  mov_oper->pred[0] = pred;
  return (mov_oper);
}				/* O_pred_restore_operation */

/* UNAT management
 * ----------------------------------------------------------------------
 */

/****************************************************************************
 *
 * routine: O_save_unat()
 * purpose: Save the unat register.  This should be placed before any spills
 *          of integers in a function.
 *          The unat was stored in the last spot in the scratch space of the
 *          parent function's stack, expect when this function is a vararg
 *          function.  In this case, the scratch space is used for the input
 *          parameters.  The unat is stored in the last spot in the local
 *          variable space.
 * input: cb - The cb in which to place the preserving of the unat.
 *        after - The op after which to place the saving code.
 *        add_op - The add whose dest has the the location on the stack at
 *                 which the unat is stored.  If this is NULL, then the unat
 *                 is in the scratch space.
 * output:
 * returns:
 * modified: Bob McGowan - 3/15/97 - created
 *           Bob McGowan - 5/2/97 - handle alternate location for unat when
 *                                  a vararg function is encountered.
 *           Robert Kidd - 9/12/02 - Updated to use new opcode map and
 *                                   completer scheme.
 * note: Even though the unat could be stored in two positions, always put
 *       SP+0 in the stack attribute.  This is used in memory disambiguation.
 *       The location is not really important as long both the loads and stores
 *       have the same spot in the attribute.
 *-------------------------------------------------------------------------*/

static void
O_save_unat (L_Cb * cb, L_Oper * after, L_Oper * add_op,
	     int variable_frame_size, int stack_frame_size)
{
  L_Oper *op;

  /* insert the sequence  mov r2 = ar.unat;  st8 [sp] = r2 */
  /* create the store and put it after the given op */
  op = L_create_new_op (Lop_ST_Q);
  op->proc_opc = TAHOEop_ST8;

  /* For a vararg function, UNAT is in LV space and an add op has been
   * generated; otherwise
   * it's at PSP
   */
  if (add_op)
    op->src[0] = L_copy_operand (add_op->dest[0]);
  else
    op->src[0] = L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, L_PTYPE_NULL);

  op->src[1] = Ltahoe_IMAC (TMPREG1);

  if (!variable_frame_size)
    Ltahoe_annotate_stack_ref (op, L_MAC_SP, stack_frame_size, 0);
  else
    Ltahoe_annotate_stack_ref (op, TAHOE_MAC_PSP, 0, 0);

  L_insert_oper_after (cb, after, op);

  /* create the mov, inserting it will push down the store */
  op = L_create_new_op (Lop_MOV);
  op->proc_opc = TAHOEop_MOV_FRAR_M;
  op->dest[0] = Ltahoe_IMAC (TMPREG1);
  op->src[0] = Ltahoe_IMAC (UNAT);
  L_insert_oper_after (cb, after, op);
}				/* O_save_unat */


/****************************************************************************
 *
 * routine: Create_add_to_unat_spill_location()
 * purpose: When the unat needs to be spilled in a vararg function,
 *          the normal place on the stack in the scratch area is already
 *          occupied, so a new place in the local variable space is used.
 *          This routine creates an add of SP and the offset which points to
 *          the proper location.  This add should be inserted before the SP
 *          is adjusted to its new position for the function.
 * input: cb - cb in which to place this new op in.
 *        stack_frame_size - The size of the entire stack frame.
 *        local_offset - offset from the functions adjusted SP to the start
 *                       of the local variables.
 *        unat_offset - offset from LV to the location where the unat value
 *                      should be stored.
 *        after_op - Insert the add op which is created after this op.
 * output:
 * returns: A pointer to the add op created.
 * modified: Bob McGowan - 5/2/97 - created.
 * note: The fill code uses LV based on the function's new SP.
 *       This code uses the original SP before adjustment.  They should point
 *       to the same spot.
 *-------------------------------------------------------------------------*/

static L_Oper *
Create_add_to_unat_spill_location (L_Cb * cb,
				   int stack_frame_size,
				   int local_offset, int unat_offset)
{
  L_Oper *add_op;

  add_op = L_create_new_op (Lop_ADD);
  add_op->proc_opc = TAHOEop_ADDS;
  add_op->dest[0] = Ltahoe_IMAC (TMPREG2);
  add_op->src[1] =
    L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, L_PTYPE_NULL);
  add_op->src[0] =
    L_new_int_operand ((local_offset + unat_offset) - stack_frame_size,
		       L_CTYPE_LLONG);
  return (add_op);
}				/* Create_add_to_unat_spill_location */


/****************************************************************************
 *
 * routine: O_restore_unat()
 * purpose: Restore the unat register.  This should be placed after any fills
 *          of integers in a function.
 *          The unat was stored in the last spot in the scratch space of the
 *          parent function's stack, expect when this function is a vararg
 *          function.  In this case, the scratch space is used for the input
 *          parameters.  The unat is stored in the last spot in the local
 *          variable space.
 * input: cb - The cb in which to place the restoring of the unat.
 *        before - The op before which to place the loading code.
 *        add_op - The add whose dest has the the location on the stack at
 *                 which the unat is stored.  If this is NULL, then the unat
 *                 is in the scratch space.
 * output:
 * returns:
 * modified: Bob McGowan - 3/15/97 - created
 *           Bob McGowan - 5/2/97 - handle alternate location for unat when
 *                                  a vararg function is encountered.
 * note: Even though the unat could be stored in two positions, always put
 *       SP+0 in the stack attribute.  This is used in memory disambiguation.
 *       The location is not really important as long both the loads and stores
 *       have the same spot in the attribute.
 *-------------------------------------------------------------------------*/

static void
O_restore_unat (L_Cb * cb, L_Oper * before, L_Oper * add_op)
{
  L_Oper *op;

  /* ld r2 = [sp] */
  op = L_create_new_op (Lop_LD_Q);
  op->proc_opc = TAHOEop_LD8;
  op->dest[0] = Ltahoe_IMAC (TMPREG1);

  /* For a vararg function, UNAT is in LV space and an add op has been
   * generated; otherwise
   * it's at PSP
   */
  if (add_op)
    op->src[0] = L_copy_operand (add_op->dest[0]);
  else
    op->src[0] = L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, L_PTYPE_NULL);

  Ltahoe_annotate_stack_ref (op, L_MAC_IP, 0, 0);

  L_insert_oper_before (cb, before, op);

  /* mov ar.unat = r2 */
  op = L_create_new_op (Lop_MOV);
  op->proc_opc = TAHOEop_MOV_TOAR_M;
  op->dest[0] = Ltahoe_IMAC (UNAT);
  op->src[0] = Ltahoe_IMAC (TMPREG1);
  L_insert_oper_before (cb, before, op);
}				/* O_restore_unat */


/****************************************************************************
 *
 * routine: Create_add_to_unat_fill_location()
 * purpose: When the unat needs to be filled in a vararg function,
 *          the normal place on the stack in the scratch area is already
 *          occupied, so a new place in the local variable space is used.
 *          This routine creates an add of LV and the offset which points to
 *          the proper location.  This add should be inserted before the SP
 *          is adjusted to its original position.
 * input: cb - cb in which to place this new op in.
 *        unat_offset - offset from LV to the location where the unat value
 *                      should be stored.
 *        before_op - Insert the add op which is create before this op.
 * output:
 * returns: A pointer to the add op created.
 * modified: Bob McGowan - 5/2/97 - created.
 * note:
 *-------------------------------------------------------------------------*/

static L_Oper *
Create_add_to_unat_fill_location (L_Cb * cb, int unat_offset)
{
  L_Oper *add_op;

  add_op = L_create_new_op (Lop_ADD);
  add_op->proc_opc = TAHOEop_ADDS;
  add_op->dest[0] = Ltahoe_IMAC (TMPREG2);
  add_op->src[0] = L_new_int_operand (unat_offset, L_CTYPE_LLONG);
  add_op->src[1] =
    L_new_macro_operand (L_MAC_LV, L_CTYPE_LLONG, L_PTYPE_NULL);
  return (add_op);
}				/* Create_add_to_unat_fill_location */


/* Preserved register spill/fill
 * ----------------------------------------------------------------------
 */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_int_preserved_store (int int_reg_id)
{
  L_Oper *st_oper;

  st_oper = L_create_new_op (Lop_ST_POST_Q);
  st_oper->proc_opc = TAHOEop_ST8_SPILL;
  st_oper->src[0] = Ltahoe_IMAC (TMPREG1);
  st_oper->src[1] = L_new_register_operand (int_reg_id, L_CTYPE_LLONG,
					    L_PTYPE_NULL);
  st_oper->src[2] = NULL;
  st_oper->dest[0] = Ltahoe_IMAC (UNAT);
  st_oper->dest[1] = L_copy_operand (st_oper->src[0]);

  return (st_oper);
}				/* O_int_preserved_store */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_fp_preserved_store (int fp_reg_id)
{
  L_Oper *st_oper;

  st_oper = L_create_new_op (Lop_ST_POST_F2);
  st_oper->proc_opc = TAHOEop_STF_SPILL;
  st_oper->src[0] = Ltahoe_IMAC (TMPREG1);
  st_oper->src[1] = L_new_register_operand (fp_reg_id, L_CTYPE_DOUBLE,
					    L_PTYPE_NULL);
  st_oper->src[2] = NULL;
  st_oper->dest[0] = L_copy_operand (st_oper->src[0]);

  return (st_oper);
}				/* O_fp_preserved_store */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_btr_preserved_store (void)
{
  L_Oper *st_oper;

  st_oper = L_create_new_op (Lop_ST_POST_Q);
  st_oper->proc_opc = TAHOEop_ST8;
  st_oper->src[0] = Ltahoe_IMAC (TMPREG1);
  st_oper->src[1] = Ltahoe_IMAC (TMPREG2);
  st_oper->src[2] = NULL;
  st_oper->dest[0] = L_copy_operand (st_oper->src[0]);

  return (st_oper);
}				/* O_btr_preserved_store */


/****************************************************************************
 *
 * routine: O_store_preserved_regs()
 * purpose: Spill all of the callee save registers to the stack.
 *          Currently this only works for floating point registers.
 * input: cb - prologue cb.
 *        oper - place callee save spill code before this oper.
 * output:
 * returns:
 * modified: Bob McGowan - 4/97 - bug fix.
 *                         The floating point callee save registers should be
 *                         saved using spill stores.
 *           Bob McGowan - 4/15/97
 *                       - Added the stack oper flag and attribute to the
 *                         store opers so that the memory disambiguator
 *                         understands that they do not have sync arcs.
 * note: The real offset from sp is not stored in the offset of the stack
 *       attribute.  This should not be a problem as long as the fill code
 *       uses the same number as the offset.
 *-------------------------------------------------------------------------*/

static void
O_store_preserved_regs (L_Cb * cb, L_Oper * oper,
			int *callee_int_array,
			int *callee_flt_array,
			int *callee_btr_array,
			int psp_offset, int stack_frame_size)
{
  L_Oper *mov_oper, *store_oper, *add_oper, *prev_oper;
  L_Operand **prev_ofst;
  int reg, offset;

  prev_oper = oper;

  /* Spill preserved predicates to the predicate-save register */

  if (callee_pred_num > 0)
    {
      L_Operand *prsav = L_new_macro_operand (TAHOE_PRED_SAVE_REG,
					      L_CTYPE_LLONG,
					      L_PTYPE_NULL);
      store_oper = O_pred_save_operation (NULL, prsav);
      L_insert_oper_after (cb, oper, store_oper);
      prev_oper = store_oper;
    }				/* if */

  /* set up a register to use to walk the stack as we save fp callee regs. */

  if (callee_int_num || callee_flt_num || callee_btr_num)
    {
      add_oper = L_create_new_op (Lop_ADD);
      add_oper->proc_opc = TAHOEop_ADDS;
      prev_ofst = &add_oper->src[0];
      add_oper->src[0] = L_new_gen_int_operand (psp_offset);
      add_oper->src[1] = L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, 0);
      add_oper->dest[0] = Ltahoe_IMAC (TMPREG1);
      L_insert_oper_after (cb, prev_oper, add_oper);
      prev_oper = add_oper;
      offset = psp_offset;

      if (callee_int_num)
	{
	  L_warn ("O_store_preserved_regs: "
		  "Spilling %d preserved int(s)... ouch!", callee_int_num);
	  for (reg = 0; reg < callee_int_num; reg++)
	    {
	      offset -= INT_CALLEE_SAVE_SIZE;
	      if (*prev_ofst)
		(*prev_ofst)->value.i -= INT_CALLEE_SAVE_SIZE;
	      else
		*prev_ofst = L_new_gen_int_operand (-INT_CALLEE_SAVE_SIZE);

	      store_oper = O_int_preserved_store (callee_int_array[reg]);
	      prev_ofst = &store_oper->src[2];

	      Ltahoe_annotate_stack_ref (store_oper,
					 L_MAC_SP,
					 stack_frame_size + offset, 1);

	      L_insert_oper_after (cb, prev_oper, store_oper);
	      prev_oper = store_oper;
	    }			/* for reg */
	}			/* if */

      if (callee_flt_num)
	{
	  /* JWS 20041002: Ensure alignment! */
	  int adj = -(offset % DOUBLE_CALLEE_SAVE_SIZE);

	  for (reg = 0; reg < callee_flt_num; reg++)
	    {
	      offset -= (DOUBLE_CALLEE_SAVE_SIZE + adj);
	      if (*prev_ofst)
		(*prev_ofst)->value.i -= (DOUBLE_CALLEE_SAVE_SIZE + adj);
	      else
		*prev_ofst = 
		  L_new_gen_int_operand (-(DOUBLE_CALLEE_SAVE_SIZE + adj));

	      store_oper = O_fp_preserved_store (callee_flt_array[reg]);
	      prev_ofst = &store_oper->src[2];

	      Ltahoe_annotate_stack_ref (store_oper,
					 L_MAC_SP,
					 stack_frame_size + offset, 1);

	      L_insert_oper_after (cb, prev_oper, store_oper);
	      prev_oper = store_oper;
	      adj = 0;
	    }			/* for reg */
	}			/* if */

      if (callee_btr_num)
	{
	  L_warn
	    ("O_store_preserved_regs: Spilling %d preserved BTR(s)... ouch!",
	     callee_btr_num);

	  for (reg = 0; reg < callee_btr_num; reg++)
	    {
	      offset -= BTR_CALLEE_SAVE_SIZE;
	      if (*prev_ofst)
		(*prev_ofst)->value.i -= DOUBLE_CALLEE_SAVE_SIZE;
	      else
		*prev_ofst = L_new_gen_int_operand (-BTR_CALLEE_SAVE_SIZE);
	      store_oper = O_btr_preserved_store ();
	      prev_ofst = &store_oper->src[2];

	      Ltahoe_annotate_stack_ref (store_oper,
					 L_MAC_SP,
					 stack_frame_size + offset, 1);

	      mov_oper = L_create_new_op (Lop_MOV);
	      mov_oper->proc_opc = TAHOEop_MOV_FRBR;
	      mov_oper->src[0] =
		L_new_register_operand (callee_btr_array[reg], L_CTYPE_BTR,
					L_PTYPE_NULL);
	      mov_oper->dest[0] = Ltahoe_IMAC (TMPREG2);
	      L_insert_oper_after (cb, prev_oper, store_oper);
	      L_insert_oper_before (cb, store_oper, mov_oper);
	      prev_oper = store_oper;
	    }			/* for reg */
	}			/* if */

      *prev_ofst = L_new_gen_int_operand (0);
    }				/* if */
  return;
}				/* O_store_preserved_regs */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_int_preserved_load (int int_reg_id, int post_inc_value)
{
  L_Oper *ld_oper;

  L_warn ("O_int_preserved_load: Filling preserved int register --"
	  "losing a NAT bit");

  ld_oper = L_create_new_op (Lop_LD_POST_Q);
  ld_oper->proc_opc = TAHOEop_LD8;
  ld_oper->src[0] = Ltahoe_IMAC (TMPREG1);
  ld_oper->src[1] = L_new_gen_int_operand (post_inc_value);
  ld_oper->dest[0] = L_new_register_operand (int_reg_id, L_CTYPE_LLONG,
					     L_PTYPE_NULL);
  ld_oper->dest[1] = L_copy_operand (ld_oper->src[0]);

  return (ld_oper);
}				/* O_int_preserved_load */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_fp_preserved_load (int fp_reg_id, int post_inc_value)
{
  L_Oper *ld_oper;

  ld_oper = L_create_new_op (Lop_LD_POST_F2);
  ld_oper->proc_opc = TAHOEop_LDF_FILL;
  ld_oper->src[0] = Ltahoe_IMAC (TMPREG1);
  ld_oper->src[1] = L_new_gen_int_operand (post_inc_value);
  ld_oper->dest[0] = L_new_register_operand (fp_reg_id, L_CTYPE_DOUBLE,
					     L_PTYPE_NULL);
  ld_oper->dest[1] = L_copy_operand (ld_oper->src[0]);

  return (ld_oper);
}				/* O_fp_preserved_load */

/* 09/12/02 REK Updating function to use new TAHOEops. */
static L_Oper *
O_tmp_preserved_load (int post_inc_value)
{
  L_Oper *ld_oper;

  ld_oper = L_create_new_op (Lop_LD_POST_Q);
  ld_oper->proc_opc = TAHOEop_LD8;
  ld_oper->src[0] = Ltahoe_IMAC (TMPREG1);
  ld_oper->src[1] = L_new_gen_int_operand (post_inc_value);
  ld_oper->dest[0] = Ltahoe_IMAC (TMPREG2);
  ld_oper->dest[1] = L_copy_operand (ld_oper->src[0]);

  return (ld_oper);
}				/* O_tmp_preserved_load */


/****************************************************************************
 *
 * routine: O_load_preserved_regs()
 * purpose: Fill all of the callee save registers from the stack.
 *          Currently this only works for floating point registers.
 * input: cb - epilogue cb.
 *        oper - place callee save fill code before this oper.
 * output:
 * returns:
 * modified: Bob McGowan - 4/97 - bug fix.
 *                         The floating point callee save registers should be
 *                         loaded using fill loads.
 *           Bob McGowan - 4/15/97
 *                       - Added the stack oper flag and attribute to the
 *                         store opers so that the memory disambiguator
 *                         understands that they do not have sync arcs.
 *           Robert Kidd - 9/12/02
 *                       - Updating function to use the new opcode map and
 *                         completers scheme.
 *-------------------------------------------------------------------------*/

static void
O_load_preserved_regs (L_Cb * cb, L_Oper * oper,
		       int *callee_flt_array,
		       int *callee_btr_array,
		       int psp_offset, int stack_frame_size)
{
  L_Oper *add_oper, *load_oper, *mov_oper, *prev_oper;
  int reg, offset;

  /* Block load predicate registers */

  if (callee_pred_num > 0)
    {
      L_Operand *prsav = L_new_macro_operand (TAHOE_PRED_SAVE_REG,
					      L_CTYPE_LLONG,
					      L_PTYPE_NULL);
      load_oper = O_pred_restore_operation (NULL, prsav, 0x1003E);
      L_insert_oper_before (cb, oper, load_oper);
    }				/* if */

  /*
   * To maintain alignment, spill floats first, then btrs, then ints
   */

  if (callee_int_num || callee_flt_num || callee_btr_num)
    {
      offset = psp_offset;
      add_oper = L_create_new_op (Lop_ADD);
      add_oper->proc_opc = TAHOEop_ADDS;
      add_oper->src[0] = L_new_gen_int_operand (offset);
      add_oper->src[1] = L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, 0);
      add_oper->dest[0] = Ltahoe_IMAC (TMPREG1);
      L_insert_oper_before (cb, oper, add_oper);
      prev_oper = add_oper;

      if (callee_btr_num)
	{
	  /* Load all callee BRs in reverse store order */

	  for (reg = callee_btr_num - 1;
	       reg >= 0; reg--, offset += BTR_CALLEE_SAVE_SIZE)
	    {
	      load_oper = O_tmp_preserved_load (BTR_CALLEE_SAVE_SIZE);

	      Ltahoe_annotate_stack_ref (load_oper, L_MAC_SP,
					 stack_frame_size + offset, 1);

	      mov_oper = L_create_new_op (Lop_MOV);
	      mov_oper->proc_opc = TAHOEop_MOV_TOBR;
	      mov_oper->src[0] = Ltahoe_IMAC (TMPREG2);
	      mov_oper->dest[0] =
		L_new_register_operand (callee_btr_array[reg], L_CTYPE_BTR,
					L_PTYPE_NULL);
	      L_insert_oper_after (cb, prev_oper, load_oper);
	      L_insert_oper_after (cb, load_oper, mov_oper);
	      prev_oper = mov_oper;
	    }			/* for reg */
	}			/* if */
      if (callee_flt_num)
	{
	  /* Load all callee FRs in reverse store order */

	  for (reg = callee_flt_num - 1;
	       reg >= 0; reg--, offset += DOUBLE_CALLEE_SAVE_SIZE)
	    {
	      load_oper = O_fp_preserved_load (callee_flt_array[reg],
					       DOUBLE_CALLEE_SAVE_SIZE);

	      Ltahoe_annotate_stack_ref (load_oper, L_MAC_SP,
					 stack_frame_size + offset, 1);

	      L_insert_oper_after (cb, prev_oper, load_oper);
	      prev_oper = load_oper;
	    }			/* for reg */
	}			/* if */
      if (callee_int_num)
	{
	  /* Load all callee FRs in reverse store order */

	  for (reg = callee_int_num - 1;
	       reg >= 0; reg--, offset += INT_CALLEE_SAVE_SIZE)
	    {
	      load_oper = O_int_preserved_load (callee_int_array[reg],
						INT_CALLEE_SAVE_SIZE);

	      Ltahoe_annotate_stack_ref (load_oper, L_MAC_SP,
					 stack_frame_size + offset, 1);

	      L_insert_oper_after (cb, prev_oper, load_oper);
	      prev_oper = load_oper;
	    }			/* for reg */
	}			/* if */
    }				/* if */
  return;
}				/* O_load_preserved_regs */


/* Stack frame construction
 * ----------------------------------------------------------------------
 */

/* Update stack pointer */

/* if decrement == 1 then decrease the stack, and inserts ops after */

typedef enum
{ O_STK_INC = 0, O_STK_DEC = 1 }
O_Stk_Update_Type;

static void
O_update_stack_pointer (L_Cb * cb, L_Oper * oper,
			int stack_frame_size,
			int callee_space_size,
			O_Stk_Update_Type decrement, int variable_frame_size)
{
  L_Oper *add_oper = NULL;
  L_Oper *load_oper;
  L_Operand *int_operand;

  /* FIXED:
   * - no psp
   * - sub / add to adjust stack frame
   * VARIABLE:
   * - psp
   * - save sp tp psp
   * - sub sp to alloc fixed initial space
   * - restore sp from psp
   */

  /* ADD OPER: use to decrement stack pointer or to restore
   * stack pointer in absence of ps when stack frame is of nonzero
   * size.
   */

  if ((stack_frame_size > 0) &&
      (!variable_frame_size || (decrement == O_STK_DEC)))
    {
      add_oper = L_create_new_op (Lop_ADD);
      add_oper->pred[0] = L_copy_operand (oper->pred[0]);
      add_oper->pred[1] = L_copy_operand (oper->pred[1]);
      add_oper->src[1] = L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, 0);
      add_oper->dest[0] = L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, 0);

      if (decrement == O_STK_DEC)
	{
	  stack_frame_size = -stack_frame_size;
	  L_insert_oper_after (cb, oper, add_oper);
	}			/* if */
      else
	{
	  L_insert_oper_before (cb, oper, add_oper);
	}			/* else */

      if (SIMM_14 (stack_frame_size))
	{
	  add_oper->src[0] = L_new_gen_int_operand (stack_frame_size);
	  add_oper->proc_opc = TAHOEop_ADDS;
	}			/* if */
      else
	{
	  int_operand = L_new_gen_int_operand (stack_frame_size);
	  add_oper->src[0] = Ltahoe_IMAC (TMPREG1);
	  load_oper = Ltahoe_int_constant_load (cb, add_oper, int_operand,
						add_oper->src[0]);
	  L_delete_operand (int_operand);
	  add_oper->proc_opc = TAHOEop_ADD;
	}			/* else */
    }				/* if */

  /* COPY OPER: use to save psp / restore sp 
   */

  if (variable_frame_size)
    {
      L_Oper *copy_oper;
      L_Attr *attr;

      copy_oper = L_create_new_op (Lop_MOV);
      copy_oper->pred[0] = L_copy_operand (oper->pred[0]);
      copy_oper->pred[1] = L_copy_operand (oper->pred[1]);
      copy_oper->proc_opc = TAHOEop_MOV_GR;
      if (decrement == O_STK_DEC)
	{
	  copy_oper->src[0] =
	    L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, 0);
	  copy_oper->dest[0] = Ltahoe_IMAC (PSP);
	  L_insert_oper_after (cb, oper, copy_oper);
	}			/* if */
      else
	{
	  copy_oper->dest[0] =
	    L_new_macro_operand (L_MAC_SP, L_CTYPE_LLONG, 0);
	  copy_oper->src[0] = Ltahoe_IMAC (PSP);
	  L_insert_oper_before (cb, oper, copy_oper);
	}			/* else */
      attr = L_new_attr ("ALLOCA", 0);
      copy_oper->attr = L_concat_attr (copy_oper->attr, attr);
    }				/* if */
  return;
}				/* O_update_stack_pointer */

/****************************************************************************
 *
 * routine: O_postpass_adjust_memory_stack()
 * purpose: Insert code to adjust sp.  Update the alloc arguments.
 *          Change all references to stack pointers so that they all have
 *          references to and offsets from SP.  Save and restore callee
 *          save registers.  This routine should be called after register
 *          allocation and preferably before postpass scheduling.
 * input: fn - function.
 *        int_swap_space_size - size in bytes of stack space used to spill
 *                              integers.
 *        fp_swap_space_size  - size in bytes of stack space used to spill
 *                              floating point.
 *        callee_flt_array    - pointer to array of integers which hold the
 *                              register ids of the floating point preserved
 *                              registers used.
 *        psp                 - function uses dynamic allocation space,
 *                              so accesses to the stack must be
 *                              relative to a previous-stack-pointer
 * output:
 * returns: 
 * modified: Jim Pierce - created.
 *           Bob McGowan - 4/97 - altered to handle unat, spill/fill around
 *                                setjmp, and removal of global vars.
 *           Bob McGowan - 5/2/97 - handle unat in the presence of varargs.
 *-------------------------------------------------------------------------*/

void
O_postpass_adjust_memory_stack (L_Func * fn, int int_swap_space_size,
				int fp_swap_space_size,
				int pred_swap_space_size,
				int *callee_flt_array, int psp)
{
  L_Cb *cb;
  L_Attr *attr;
  int mem_alloc;
  int stack_frame_size;
  int callee_space_size = 0;	/* Total mem stack space required for 
				   preserved registers */
  int local_var_size;
  int scratch_space;
  int alloc_offset;
  int pred_swap_offset;
  int int_swap_offset;
  int unat_swap_offset = 0;
  int fp_swap_offset;
  int callee_offset;
  int local_offset;
  int input_parm_offset;
  int output_parm_offset;

  int unat_spill_offset;
  int pri_unat_in_lv = 0;
  int unat_offset_from_LV = 0;	/* for vararg functions */
  int leaf;
  int input_param_spill = 0;

  int unats_reqd;

  int variable_frame_size;

  L_do_flow_analysis (fn, LIVE_VARIABLE | REACHING_DEFINITION);

  variable_frame_size = (L_find_attr (fn->attr, "ALLOCA") != NULL);

  /* If the function allocates additional stack space, we need to
   * keep track of the previous stack pointer (TAHOE_MAC_PSP)
   * and make all stack segments except output params relative to
   * PSP
   */

  /* Find out if function is leaf node and the amount of mem allocation
     space */
  Ltahoe_scan_prologue_defines (fn, &leaf, &mem_alloc);

  /* STACK FRAME SETUP
   * ----------------------------------------------------------------------
   * All sections positively indexed  -- JWS 20010209
   */

  /* input parameter spill space
   * ----------------------------------------------------------------------
   * Indexed off of MAC $IP (previous stack pointer)
   */

  input_parm_offset = 0;

  if ((attr = L_find_attr (fn->attr, "stk_size_ip")))
    {
      int size_ip = attr->field[0]->value.i;

      if (size_ip > 16)
	input_param_spill = size_ip;

      if (size_ip > 8)
	{
	  pri_unat_in_lv = 1;
	  unat_offset_from_LV = fn->s_local;
	  fn->s_local += 8;
	}
    }
  else if ((attr = L_find_attr (fn->attr, "ipspill")))
    {
      int j, spill_ct = 0, first_spill = 0;

      for (j = 7; j >= 0; j--)
	if (attr->field[j]->value.i)
	  {
	    spill_ct++;
	    first_spill = j;
	  }			/* if */

      /* If necessary, reserve extra stack space for ip spills */

      if (spill_ct > 2)
	{
	  /* PtoL spills differently from Ltahoe... if the spills have
	   * already occurred, parameters were spilled in place!
	   */
	  if (Ltahoe_is_vararg_func (fn))
	    input_param_spill = spill_ct * 8;
	  else
	    input_param_spill = (8 - first_spill) * 8;
	}			/* if */

      /* If necessary, move the UNAT out of the scratch space to make way */

      if (spill_ct > 1)
	{
	  pri_unat_in_lv = 1;
	  unat_offset_from_LV = fn->s_local;
	  fn->s_local += 8;
	}			/* if */
    }				/* if */
  else
    {
      L_punt ("O_postpass_adjust_memory_stack: "
	      "no ipspill attr -- check PtoL.");
    }				/* else */

  stack_frame_size = ADDR_ALIGN (input_param_spill, 16);

  /* callee-save spill space
   * ----------------------------------------------------------------------
   */

  /* Space to save callee registers. Callee save registers are indexed from
     from the SP upon entry before SP is updated. */

  if (callee_int_num || callee_flt_num || callee_btr_num)
    {
      callee_space_size =
	callee_int_num * INT_CALLEE_SAVE_SIZE +
	callee_flt_num * DOUBLE_CALLEE_SAVE_SIZE +
	callee_btr_num * BTR_CALLEE_SAVE_SIZE;

      /* JWS 20041002: Ensure alignment */
      if (callee_flt_num)
	callee_space_size += ((callee_int_num * INT_CALLEE_SAVE_SIZE) % 16);
    }				/* if */

  stack_frame_size += ADDR_ALIGN (callee_space_size, 16);
  callee_offset = stack_frame_size;

  /* local variable space
   * ----------------------------------------------------------------------
   */

  local_var_size = ADDR_ALIGN (fn->s_local, 16);
  stack_frame_size += local_var_size;
  local_offset = stack_frame_size;

  /* "own" integer spill space
   * ----------------------------------------------------------------------
   * Locations for spilled integer variables
   * Must be covered by UNAT register(s)
   */

  stack_frame_size += ADDR_ALIGN (int_swap_space_size, 16);
  int_swap_offset = stack_frame_size;

  stack_frame_size += ADDR_ALIGN (pred_swap_space_size, 16);
  pred_swap_offset = stack_frame_size;

  /* UNAT counting
   * ----------------------------------------------------------------------
   * Figure out if we need more than one UNAT. We must have UNAT bits
   * to cover the integer sync and swap spaces.
   */

  unats_reqd = int_swap_space_size / (8 * 64);
  if (int_swap_space_size % (8 * 64))
    unats_reqd++;

  if (unats_reqd > 1)
    {
      L_warn ("O_postpass_adjust_memory_stack: Using %d unats!", unats_reqd);
      stack_frame_size += ADDR_ALIGN (unats_reqd * 8, 16);
      unat_swap_offset = stack_frame_size;
    }				/* if */

  /* "own" floating point spill space
   * ----------------------------------------------------------------------
   * Locations for spilled float, double variables
   */

  stack_frame_size += ADDR_ALIGN (fp_swap_space_size, 16);
  fp_swap_offset = stack_frame_size;

  /* dynamic allocation space (alloca space)
   * ----------------------------------------------------------------------
   */

  stack_frame_size += ADDR_ALIGN (mem_alloc, 16);
  alloc_offset = stack_frame_size;

  /* outgoing parameter space
   * ----------------------------------------------------------------------
   */

  stack_frame_size += ADDR_ALIGN (fn->s_param, 16);
  output_parm_offset = stack_frame_size;

  /* mandated 16-byte scratch space
   * ----------------------------------------------------------------------
   */

  if (!leaf && stack_frame_size != 0)
    {
      scratch_space = SCRATCH_SPACE_SIZE;
      stack_frame_size += scratch_space;
    }				/* if */
  else
    {
      scratch_space = 0;
    }				/* else */

  /* Offset adjustment
   * ----------------------------------------------------------------------
   * Make all offsets relative to the new SP (base of local stack frame)
   */

  if (!variable_frame_size)
    {
      /* SP-relative */
      callee_offset = stack_frame_size - callee_offset;
      local_offset = stack_frame_size - local_offset;
      int_swap_offset = stack_frame_size - int_swap_offset;
      pred_swap_offset = stack_frame_size - pred_swap_offset;
      unat_swap_offset = stack_frame_size - unat_swap_offset;
      fp_swap_offset = stack_frame_size - fp_swap_offset;
      alloc_offset = stack_frame_size - alloc_offset;
      input_parm_offset = stack_frame_size;
    }				/* if */
  else
    {
      /* PSP-relative */
      callee_offset = -callee_offset;
      local_offset = -local_offset;
      int_swap_offset = -int_swap_offset;
      pred_swap_offset = -pred_swap_offset;
      unat_swap_offset = -unat_swap_offset;
      fp_swap_offset = -fp_swap_offset;
      alloc_offset = -alloc_offset;
      input_parm_offset = 0;
    }				/* else */

  output_parm_offset = 0;

  /* Compute location at which caller's unat is spilled */

  if (!pri_unat_in_lv)
    unat_spill_offset = stack_frame_size;
  else
    unat_spill_offset = local_offset + unat_offset_from_LV;

  if (Ltahoe_debug_stack_frame)
    {
      if (variable_frame_size)
	fprintf (stderr, "VARIABLE FRAME SIZE: Offsets from PSP\n");
      else
	fprintf (stderr, "FIXED FRAME SIZE: Offsets from SP\n");

      fprintf (stderr, "---------------------        sp offset     size\n");
      fprintf (stderr, "Scratch Area                    %4d        %4d\n",
	       0, scratch_space);
      fprintf (stderr, "Outgoing param space            %4d        %4d\n",
	       output_parm_offset, fn->s_param);
      fprintf (stderr, "Floating Point Spill/fill space %4d        %4d\n",
	       fp_swap_offset, fp_swap_space_size);
      fprintf (stderr, "Predicate Spill/fill space      %4d        %4d\n",
	       pred_swap_offset, pred_swap_space_size);
      fprintf (stderr, "Integer Spill/fill space        %4d        %4d\n",
	       int_swap_offset, int_swap_space_size);
      fprintf (stderr, "Local variable space            %4d        %4d\n",
	       local_offset, fn->s_local);
      fprintf (stderr, "Callee save space               %4d        %4d\n",
	       callee_offset, callee_space_size);
      fprintf (stderr, "Input param spill               %4d        %4d\n",
	       stack_frame_size - input_param_spill, input_param_spill);
      fprintf (stderr, "Input param offset              %4d           -\n",
	       input_parm_offset);
      if (int_swap_space_size)
	fprintf (stderr, "Unat spill location             %4d           8\n",
		 unat_spill_offset);
      else
	fprintf (stderr, "Unat spill/fill not required\n");
      fprintf (stderr, "Total stack frame size                      %4d\n",
	       stack_frame_size);
      fprintf (stderr, "---------------------\n");
    }				/* if */

  /* Annotate existing mcode */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      L_Oper *oper, *next_oper, *add_oper;

      for (oper = cb->first_op; oper; oper = next_oper)
	{
	  next_oper = oper->next_op;

	  /* Must add code after the alloc so that dependencies are observed
	     for pass2 scheduler.  The rule is that the code after post_pass
	     annotation should be runnable.                                */

	  switch (oper->opc)
	    {
	    case Lop_PROLOGUE:
	      L_delete_oper (cb, oper);	/* Delete PROLOGUE oper */
	      break;

	    case Lop_ALLOC:
	      /* Save an additional special register for PSP if reqd */
	      O_update_alloc_operands (oper, NUM_SPECIAL_INT_REG +
				       variable_frame_size);

	      /* after the alloc, adjust the sp to account for all of the
	       * callee save registers. */

	      O_update_stack_pointer (cb, oper, stack_frame_size,
				      callee_space_size, O_STK_DEC,
				      variable_frame_size);

	      /* between the alloc and the the sp adjustment that was just
	       * inserted, insert all of the spill for the callee save.
	       */

	      /* Spill is conducted relative to previous stack pointer
	       * (PSP, or $IP in IMPACT vernacular).  An offset to
	       * PSP is supplied, which in this case is the
	       * (aligned) varags_adjust.
	       */

	      O_store_preserved_regs (cb, oper, callee_int_array,
				      callee_flt_array, callee_btr_array,
				      -ADDR_ALIGN (input_param_spill, 16),
				      stack_frame_size);

	      /* If integers have been spilled, insert code to save
	       * the caller's UNAT register to the stack.  This must
	       * precede all spills of the callee's registers.
	       * If not a vararg function, the UNAT is spilled into
	       * the scratch space in the caller's stack frame.  When
	       * varargs are present, this space is already used, so
	       * allocate space adjacent to the LV space for the UNAT.
	       */

	      if (int_swap_space_size)
		{
		  /* Integers have been spilled so the unat register must
		     be saved and restored as a callee save register.
		     Place this below the alloc and above the adjustment
		     of the sp that was just inserted. */
		  if (pri_unat_in_lv)
		    {
		      add_oper =
			Create_add_to_unat_spill_location (cb,
							   stack_frame_size,
							   local_offset,
							   unat_offset_from_LV);
		      L_insert_oper_after (cb, oper, add_oper);
		    }		/* if */
		  else
		    {
		      add_oper = NULL;
		    }		/* else */
		  O_save_unat (cb, add_oper ? add_oper : oper, add_oper,
			       variable_frame_size, stack_frame_size);
		}		/* if */
	      break;

	    case Lop_EPILOGUE:
	      /* Now doing cleanup right before rts due to 
	         possible fill/spill between epilogue and rts */

	      L_delete_oper (cb, oper);	/* Delete EPI oper */
	      break;

	    case Lop_JSR:
	      /* Save / restore GP around JSR */
	      if (!L_find_attr (oper->attr, "LINKLOCAL"))
		{
		  add_oper = L_create_new_op (Lop_MOV);
		  add_oper->proc_opc = TAHOEop_MOV_GR;
		  add_oper->dest[0] = L_new_macro_operand (TAHOE_GP_SAVE_REG,
							   L_CTYPE_LLONG,
							   L_PTYPE_NULL);
		  add_oper->src[0] = Ltahoe_IMAC (GP);
		  add_oper->pred[0] = L_copy_operand (oper->pred[0]);
		  L_insert_oper_before (cb, oper, add_oper);

		  if (L_is_register (oper->src[0]))
		    {
		      /* For indirect calls, install GP from linkage
		       * table entry.  The load generating this value
		       * was inserted in phase1.  
		       */
		      add_oper = L_create_new_op (Lop_MOV);
		      add_oper->proc_opc = TAHOEop_MOV_GR;
		      add_oper->src[0] = oper->src[1];
		      oper->src[1] = Ltahoe_IMAC (GP);
		      add_oper->dest[0] = Ltahoe_IMAC (GP);
		      add_oper->pred[0] = L_copy_operand (oper->pred[0]);
		      L_insert_oper_before (cb, oper, add_oper);
		    }		/* if */

		  add_oper = L_create_new_op (Lop_MOV);
		  add_oper->proc_opc = TAHOEop_MOV_GR;
		  add_oper->src[0] = L_new_macro_operand (TAHOE_GP_SAVE_REG,
							  L_CTYPE_LLONG,
							  L_PTYPE_NULL);
		  add_oper->dest[0] = Ltahoe_IMAC (GP);
		  add_oper->pred[0] = L_copy_operand (oper->pred[0]);
		  L_insert_oper_after (cb, oper, add_oper);
		}
	      break;

	    case Lop_RTS:
	      if (pri_unat_in_lv)
		{
		  /* This is a vararg function which needs the unat to be
		     restored.  The first step is to create an add which
		     will index the right position on the stack.  Do this
		     before the stack pointer is changed.
		     Note that tmpreg2 will be live accross the stack pointer
		     update and the callee register restore. */
		  add_oper =
		    Create_add_to_unat_fill_location (cb,
						      unat_offset_from_LV);
		  L_insert_oper_before (cb, oper, add_oper);
		  O_update_stack_references (fn, cb, add_oper, local_offset,
					     int_swap_offset, fp_swap_offset,
					     pred_swap_offset,
					     input_parm_offset,
					     output_parm_offset,
					     variable_frame_size);
		}		/* if */
	      else
		{
		  add_oper = NULL;
		}		/* else */

	      /* Update stack pointer, put ops before return */
	      O_update_stack_pointer (cb, oper, stack_frame_size,
				      callee_space_size, O_STK_INC,
				      variable_frame_size);

	      /* Restore callee save registers, and place them before the
	         return and after the adjustment to the sp. */
	      O_load_preserved_regs (cb, oper,
				     callee_flt_array,
				     callee_btr_array,
				     -callee_space_size
				     - ADDR_ALIGN (input_param_spill, 16),
				     stack_frame_size);
	      if (int_swap_space_size)
		{
		  /* Integers have been spilled so the unat register must
		     be saved and restored as a callee save register.
		     Put before return and below sp adjustment. */
		  O_restore_unat (cb, oper, add_oper);
		}		/* if */
	      break;

	    default:
	      O_update_stack_references (fn, cb, oper, local_offset,
					 int_swap_offset, fp_swap_offset,
					 pred_swap_offset,
					 input_parm_offset,
					 output_parm_offset,
					 variable_frame_size);
	    }			/* switch */
	}			/* for oper */
    }				/* for cb */

  if (!Ltahoe_clobber_unats && (unats_reqd > 1))
    O_implement_unat_swapping (fn, int_swap_offset,
			       int_swap_space_size, unat_swap_offset,
			       stack_frame_size);
  return;
}				/* O_postpass_adjust_memory_stack */


/*
 * O_implement_unat_swapping
 * ----------------------------------------------------------------------
 * Manage multiple ar.unat registers.
 * Assumes space has been reserved for N spilled unats at the end
 * of the int swap space
 */
/* 09/12/02 REK Updating function to use new TAHOEops. */

static void
O_implement_unat_swapping (L_Func * fn,
			   int int_swap_offset,
			   int int_swap_space_size, int unat_swap_offset,
			   int stack_frame_size)
{
  int num_unats = 0;
  int num_regs = 0;
  int stack_offset;
  int unat_offset;
  ITint32 stack_mac;
  L_Attr *attr;
  L_Cb *cb;
  L_Oper *op, *new_op;

  printf ("O_spill_fill_unats: swap off: %d swap space: %d\n",
	  int_swap_offset, int_swap_space_size);

  /* Count spill locations and unats required */

  num_regs = int_swap_space_size / 8;
  num_unats = num_regs / 64;
  if (num_regs % 64)
    num_unats++;

  if (num_unats > 2)
    L_warn ("Man you use a lot of registers!  "
	    "Half-assed unat swapping in effect!");

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  if (!L_EXTRACT_BIT_VAL (op->flags, L_OPER_SPILL_CODE))
	    continue;
	  if ((op->proc_opc != TAHOEop_ST8_SPILL) &&
	      (op->proc_opc != TAHOEop_LD8_FILL))
	    continue;

	  attr = L_find_attr (op->attr, "stack");

	  stack_mac = attr->field[0]->value.mac;
	  if ((stack_mac != L_MAC_SP) && (stack_mac != TAHOE_MAC_PSP))
	    {
	      L_print_oper (stdout, op);
	      L_punt ("Spill or fill not sp or psp offset in oper %d",
		      op->id);
	    }			/* if */

	  stack_offset = attr->field[1]->value.i;
	  if (stack_offset >= (int_swap_offset + 64 * 8))
	    {
	      if (!op->prev_op || (op->prev_op->opc != Lop_ADD))
		L_punt ("RDB's not lame assumption that the spill/fill"
			" would be preceeded by the address "
			"generation was violated");

	      unat_offset = (stack_offset - int_swap_offset) / (64 * 8);

	      /* This spill/fill is outside the original UNAT
	         swap in the extra UNAT */

	      if (stack_mac == L_MAC_SP)
		{
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_MOV);
		  new_op->proc_opc = TAHOEop_MOV_FRAR_M;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG1);
		  new_op->src[0] = Ltahoe_IMAC (UNAT);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[0] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->src[1] = L_new_macro_operand (L_MAC_SP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_ST_Q);
		  new_op->proc_opc = TAHOEop_ST8;

		  attr = L_new_attr ("stack", 2);
		  attr->field[0] = L_new_macro_operand (L_MAC_SP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  attr->field[1] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  attr = L_new_attr ("offset", 1);
		  attr->field[0] =
		    L_new_gen_llong_operand (unat_swap_offset -
					     int_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);

		  new_op->flags =
		    L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		  new_op->src[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[1] = Ltahoe_IMAC (TMPREG1);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[0] =
		    L_new_gen_llong_operand (unat_swap_offset +
					     (unat_offset * 8));
		  new_op->src[1] = L_new_macro_operand (L_MAC_SP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_LD_Q);
		  new_op->proc_opc = TAHOEop_LD8;
		  attr = L_new_attr ("stack", 2);
		  attr->field[0] = L_new_macro_operand (L_MAC_SP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  attr->field[1] =
		    L_new_gen_llong_operand (unat_swap_offset +
					     (unat_offset * 8));
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  attr = L_new_attr ("offset", 1);
		  attr->field[0] =
		    L_new_gen_llong_operand (unat_swap_offset
					     + (unat_offset * 8)
					     - int_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  new_op->flags =
		    L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		  new_op->dest[0] =
		    L_new_macro_operand (TAHOE_MAC_TMPREG1,
					 L_CTYPE_LLONG, L_PTYPE_NULL);
		  new_op->src[0] = L_new_macro_operand (TAHOE_MAC_TMPREG2,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_MOV);
		  new_op->proc_opc = TAHOEop_MOV_TOAR_M;
		  new_op->src[0] = L_new_macro_operand (TAHOE_MAC_TMPREG1,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->dest[0] = Ltahoe_IMAC (UNAT);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  if (op->proc_opc == TAHOEop_ST8_SPILL)
		    {
		      /* A spill could change the UNAT so write it back
		       * out */
			/*--------------------------------------------------*/
		      new_op = L_create_new_op (Lop_MOV);
		      new_op->proc_opc = TAHOEop_MOV_FRAR_M;
		      new_op->dest[0] =
			L_new_macro_operand (TAHOE_MAC_TMPREG1,
					     L_CTYPE_LLONG, L_PTYPE_NULL);
		      new_op->src[0] = Ltahoe_IMAC (UNAT);
		      new_op->pred[0] = L_copy_operand (op->pred[0]);
		      L_insert_oper_after (cb, op, new_op);
		      op = new_op;
			/*--------------------------------------------------*/
		      new_op = L_create_new_op (Lop_ST_Q);
		      new_op->proc_opc = TAHOEop_ST8;
		      attr = L_new_attr ("stack", 2);
		      attr->field[0] = L_new_macro_operand (L_MAC_SP,
							    L_CTYPE_LLONG,
							    L_PTYPE_NULL);
		      attr->field[1] =
			L_new_gen_llong_operand (unat_swap_offset +
						 (unat_offset * 8));
		      new_op->attr = L_concat_attr (new_op->attr, attr);
		      attr = L_new_attr ("offset", 1);
		      attr->field[0] =
			L_new_gen_llong_operand (unat_swap_offset +
						 (unat_offset * 8) -
						 int_swap_offset);
		      new_op->attr = L_concat_attr (new_op->attr, attr);

		      new_op->flags =
			L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		      new_op->src[0] = Ltahoe_IMAC (TMPREG2);
		      new_op->src[1] = Ltahoe_IMAC (TMPREG1);
		      new_op->pred[0] = L_copy_operand (op->pred[0]);
		      L_insert_oper_after (cb, op, new_op);
		      op = new_op;
			/*--------------------------------------------------*/
		    }		/* if */

		  /* Load old UNAT back */
		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[0] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->src[1] = L_new_macro_operand (L_MAC_SP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_after (cb, op, new_op);
		  op = new_op;
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_LD_Q);
		  new_op->proc_opc = TAHOEop_LD8;
		  attr = L_new_attr ("stack", 2);
		  attr->field[0] = L_new_macro_operand (L_MAC_SP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  attr->field[1] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  attr = L_new_attr ("offset", 1);
		  attr->field[0] =
		    L_new_gen_llong_operand (unat_swap_offset -
					     int_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);

		  new_op->flags =
		    L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG1);
		  new_op->src[0] = Ltahoe_IMAC (TMPREG2);

		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_after (cb, op, new_op);
		  op = new_op;
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_MOV);
		  new_op->proc_opc = TAHOEop_MOV_TOAR_M;
		  new_op->src[0] = Ltahoe_IMAC (TMPREG1);
		  new_op->dest[0] = Ltahoe_IMAC (UNAT);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_after (cb, op, new_op);
		  op = new_op;
		    /*--------------------------------------------------*/
		}		/* if */
	      else
		{		/* Function using alloca has spills PSP relative */
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_MOV);
		  new_op->proc_opc = TAHOEop_MOV_FRAR_M;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG1);
		  new_op->src[0] = Ltahoe_IMAC (UNAT);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[0] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->src[1] = L_new_macro_operand (TAHOE_MAC_PSP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_ST_Q);
		  new_op->proc_opc = TAHOEop_ST8;

		  attr = L_new_attr ("stack", 2);
		  attr->field[0] = L_new_macro_operand (TAHOE_MAC_PSP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  attr->field[1] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  attr = L_new_attr ("offset", 1);
		  attr->field[0] =
		    L_new_gen_llong_operand (unat_swap_offset -
					     int_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);

		  new_op->flags =
		    L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		  new_op->src[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[1] = Ltahoe_IMAC (TMPREG1);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[0] =
		    L_new_gen_llong_operand (unat_swap_offset +
					     (unat_offset * 8));
		  new_op->src[1] = L_new_macro_operand (TAHOE_MAC_PSP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_LD_Q);
		  new_op->proc_opc = TAHOEop_LD8;
		  attr = L_new_attr ("stack", 2);
		  attr->field[0] = L_new_macro_operand (TAHOE_MAC_PSP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  attr->field[1] =
		    L_new_gen_llong_operand (unat_swap_offset +
					     (unat_offset * 8));
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  attr = L_new_attr ("offset", 1);
		  attr->field[0] =
		    L_new_gen_llong_operand (unat_swap_offset
					     + (unat_offset * 8)
					     - int_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  new_op->flags =
		    L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		  new_op->dest[0] =
		    L_new_macro_operand (TAHOE_MAC_TMPREG1,
					 L_CTYPE_LLONG, L_PTYPE_NULL);
		  new_op->src[0] = L_new_macro_operand (TAHOE_MAC_TMPREG2,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_MOV);
		  new_op->proc_opc = TAHOEop_MOV_TOAR_M;
		  new_op->src[0] = L_new_macro_operand (TAHOE_MAC_TMPREG1,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->dest[0] = Ltahoe_IMAC (UNAT);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_before (cb, op->prev_op, new_op);
		    /*--------------------------------------------------*/
		  if (op->proc_opc == TAHOEop_ST8_SPILL)
		    {
		      /* A spill could change the UNAT so write it back
		       * out */
			/*--------------------------------------------------*/
		      new_op = L_create_new_op (Lop_MOV);
		      new_op->proc_opc = TAHOEop_MOV_FRAR_M;
		      new_op->dest[0] =
			L_new_macro_operand (TAHOE_MAC_TMPREG1,
					     L_CTYPE_LLONG, L_PTYPE_NULL);
		      new_op->src[0] = Ltahoe_IMAC (UNAT);
		      new_op->pred[0] = L_copy_operand (op->pred[0]);
		      L_insert_oper_after (cb, op, new_op);
		      op = new_op;
			/*--------------------------------------------------*/
		      new_op = L_create_new_op (Lop_ST_Q);
		      new_op->proc_opc = TAHOEop_ST8;
		      attr = L_new_attr ("stack", 2);
		      attr->field[0] = L_new_macro_operand (TAHOE_MAC_PSP,
							    L_CTYPE_LLONG,
							    L_PTYPE_NULL);
		      attr->field[1] =
			L_new_gen_llong_operand (unat_swap_offset
						 + (unat_offset * 8));
		      new_op->attr = L_concat_attr (new_op->attr, attr);
		      attr = L_new_attr ("offset", 1);
		      attr->field[0] =
			L_new_gen_llong_operand (unat_swap_offset +
						 (unat_offset * 8) -
						 int_swap_offset);
		      new_op->attr = L_concat_attr (new_op->attr, attr);

		      new_op->flags =
			L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		      new_op->src[0] = Ltahoe_IMAC (TMPREG2);
		      new_op->src[1] = Ltahoe_IMAC (TMPREG1);
		      new_op->pred[0] = L_copy_operand (op->pred[0]);
		      L_insert_oper_after (cb, op, new_op);
		      op = new_op;
			/*--------------------------------------------------*/
		    }		/* if */

		  /* Load old UNAT back */
		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG2);
		  new_op->src[0] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->src[1] = L_new_macro_operand (TAHOE_MAC_PSP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_after (cb, op, new_op);
		  op = new_op;
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_LD_Q);
		  new_op->proc_opc = TAHOEop_LD8;
		  attr = L_new_attr ("stack", 2);
		  attr->field[0] = L_new_macro_operand (TAHOE_MAC_PSP,
							L_CTYPE_LLONG,
							L_PTYPE_NULL);
		  attr->field[1] = L_new_gen_llong_operand (unat_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);
		  attr = L_new_attr ("offset", 1);
		  attr->field[0] =
		    L_new_gen_llong_operand (unat_swap_offset -
					     int_swap_offset);
		  new_op->attr = L_concat_attr (new_op->attr, attr);

		  new_op->flags =
		    L_SET_BIT_FLAG (op->flags, L_OPER_STACK_REFERENCE);
		  new_op->dest[0] = Ltahoe_IMAC (TMPREG1);
		  new_op->src[0] = Ltahoe_IMAC (TMPREG2);

		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_after (cb, op, new_op);
		  op = new_op;
		    /*--------------------------------------------------*/
		  new_op = L_create_new_op (Lop_MOV);
		  new_op->proc_opc = TAHOEop_MOV_TOAR_M;
		  new_op->src[0] = Ltahoe_IMAC (TMPREG1);
		  new_op->dest[0] = Ltahoe_IMAC (UNAT);
		  new_op->pred[0] = L_copy_operand (op->pred[0]);
		  L_insert_oper_after (cb, op, new_op);
		  op = new_op;
		    /*--------------------------------------------------*/
		}		/* else */
	    }			/* if */
	}			/* for op */
    }				/* for cb */

  return;
}				/* O_implement_unat_swapping */
