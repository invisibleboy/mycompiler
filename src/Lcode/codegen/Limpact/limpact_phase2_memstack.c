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

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "limpact_main.h"
#include "limpact_phase2.h"

#define DEBUG

/*****************************************************************************\
 *
 *  File:  limpact_phase2_memstk.c
 *
 *  Description:
 *    Memory stack setup and update routines (based loosely on Ltahoe's
 *    phase2_memstk.c
 *
 *  Creation Date : January 2002
 *  
 *  Author:  Ronald D. Barnes, Wen-mei Hwu
 *
\*****************************************************************************/

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
				   int variable_frame_size, int vararg_ip)
{
  L_Oper *oper = NULL, *new_oper;
  L_Attr *attr;
  Set defs, uses;
  int num_defs, d, num_uses;
  int *def_buffer = NULL;
  int adjust_move = 1;

  /* Locate the producer(s) of var_operand. */

  defs = L_get_oper_RIN_defining_opers (stk_oper,
					var_operand);
  num_defs = Set_size (defs);
		
  if( num_defs < 1 )
    L_punt("O_update_stack_const_in_prev_oper: "
	   "Could not find a defining operand for op %d.\n", 
	   stk_oper->id);
    
  def_buffer = (int *) Lcode_malloc (sizeof (int) * num_defs);
  Set_2array (defs, def_buffer);
  for (d = 0; d < num_defs; d++)
    {
      L_Oper *def = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
					       def_buffer[d]);
      
      /* For this def, if it is not a MOV, then we can't
	 adjust it; add the extra instruction below. */

      if (def->proc_opc != Lop_MOV )
	{
	  adjust_move = 0;
	  break;
	}

      /* For this def, if its only use is the stk_oper, then
	 it can be adjusted.  Otherwise, add the extra
	 instruction below.  Its other uses may also
	 require the SAME adjustment, but we'll leave that
	 optimization for future implementation. */

      uses = L_get_oper_ROUT_using_opers (def,
					  def->dest[0]);
      num_uses = Set_size (uses);
		
      Set_dispose (uses);

      if( num_uses < 1 )
	L_punt("O_update_stack_const_in_prev_oper: "
	       "Could not find a using operand for op %d.\n", 
	       def->id);
      
      if( num_uses > 1 )
	{
	  adjust_move = 0;
	  break;
	}
    }

  if( adjust_move == 1 )
    {
#ifdef DEBUG
      L_warn("Updating %d stack constants for op %d.", 
	     num_defs, stk_oper->id);
#endif      
      for (d = 0; d < num_defs; d++)
	{
	  L_Oper *def = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						   def_buffer[d]);

	  if (vararg_ip)
	    {
	      /* Remember original operand so that Lemulate can use it */
	      attr = L_new_attr("vararg_ip",2);
	      attr->field[0] = L_copy_operand(oper->src[0]);
	      attr->field[1] = NULL;
	      def->attr = L_concat_attr (def->attr, attr);
	    }
	  def->src[0]->value.i += offset_val;
	}
    }
  else
    {
      /* Couldn't find move immed to var_operand.  Add another 
	 instruction which converts LV to offset+SP */
      
#ifdef DEBUG
      L_warn("Inserting stack adjust inst for op %d.", 
	     stk_oper->id);
#endif
      if (L_general_load_opcode(oper) || L_general_store_opcode(oper)
	  || L_int_add_opcode(oper))
	{
	  new_oper = L_create_new_op_using (Lop_ADD_U, stk_oper);
	  new_oper->src[0] = L_new_gen_int_operand (offset_val);
	  new_oper->src[1] = variable_frame_size ? L_new_macro_operand (L_MAC_FP, 
									M_native_int_register_ctype(), 
									L_PTYPE_NULL):
	    L_new_macro_operand (L_MAC_SP, M_native_int_register_ctype(), L_PTYPE_NULL);
	  new_oper->dest[0] = L_copy_operand (stk_oper->dest[0]);
	  L_insert_oper_before (cb, stk_oper, new_oper);
	  
	  *stk_operand = new_oper->src[1];
	  
	  if (L_same_operand (var_operand, stk_oper->src[0]))
	    { 
	      L_delete_operand (stk_oper->src[1]);
	      stk_oper->src[1] = L_copy_operand (stk_oper->dest[0]);
	    }
	  else
	    {
	      L_delete_operand (stk_oper->src[0]);
	      stk_oper->src[0] = L_copy_operand (stk_oper->dest[0]);
	    }
	  
	  if (vararg_ip)
	    {
	      /* Remember that stk_oper originally had just IP as an operand */
	      attr = L_new_attr("vararg_ip",2);
	      attr->field[0] = L_new_gen_int_operand (0);
	      attr->field[1] = L_new_macro_operand (L_MAC_IP, M_native_int_register_ctype(), 
						    L_PTYPE_NULL);
	      new_oper->attr = L_concat_attr (new_oper->attr, attr);
	    }
	  
#ifdef DEBUG
	  L_print_oper (stderr, new_oper);
	  L_print_oper (stderr, stk_oper);
#endif
	}
      else
	{
	  L_punt ("O_update_stack_const_in_prev_oper: "
		  "Reg define not found in cb (oper %d)\n",
		  stk_oper->id);
	}
    }

  Lcode_free (def_buffer);
  def_buffer = NULL;
  Set_dispose (defs);
  
  return;
}

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
			O_Stk_Update_Type decrement, int variable_frame_size)
{
  L_Oper *add_oper = NULL;
  L_Oper *copy_oper;
  L_Attr *attr;

  /* FIXED:
   * - no psp
   * - sub / add to adjust stack frame
   * VARIABLE:
   * - psp
   * - save sp tp psp
   * - sub sp to alloc fixed initial space
   * - restore sp from psp
   */

  if (stack_frame_size < 0)
    L_punt("O_update_stack_pointer: Negative stack frame size!");

  /* ADD OPER: use to decrement stack pointer or to restore
   * stack pointer in absence of ps when stack frame is of nonzero
   * size.
   */

  if ((stack_frame_size > 0) &&
      (!variable_frame_size || (decrement == O_STK_DEC)))
    {
      add_oper = L_create_new_op (Lop_ADD);
      add_oper->src[1] = L_new_macro_operand (L_MAC_SP, 
					      M_native_int_register_ctype(), 
					      L_PTYPE_NULL);
      add_oper->dest[0] = L_new_macro_operand (L_MAC_SP, 
					       M_native_int_register_ctype(), 
					       L_PTYPE_NULL);
      
      if (decrement == O_STK_DEC)
	{
	  stack_frame_size = -stack_frame_size;
	  L_insert_oper_after (cb, oper, add_oper);
	}
      else
	{
	  L_insert_oper_before (cb, oper, add_oper);
	}
      
      add_oper->src[0] = L_new_gen_int_operand (stack_frame_size);
    }
  
  /* COPY OPER: use to save psp / restore sp 
   */
  
  if (variable_frame_size)
    {
      copy_oper = L_create_new_op (Lop_MOV);

      if (decrement == O_STK_DEC)
	{
	  copy_oper->src[0] = L_new_macro_operand (L_MAC_SP, 
					      M_native_int_register_ctype(), 
					      L_PTYPE_NULL);

	  copy_oper->dest[0] = L_new_macro_operand(L_MAC_FP,
						   M_native_int_register_ctype(), 
						   L_PTYPE_NULL);;
	  L_insert_oper_after (cb, oper, copy_oper);
	}
      else
	{
	  copy_oper->src[0] = L_new_macro_operand(L_MAC_FP,
						  M_native_int_register_ctype(),
						  L_PTYPE_NULL);

	  copy_oper->dest[0] = L_new_macro_operand (L_MAC_SP, 
					       M_native_int_register_ctype(), 
					       L_PTYPE_NULL);
	  L_insert_oper_before (cb, oper, copy_oper);
	}

      attr = L_new_attr ("ALLOCA", 0);
      copy_oper->attr = L_concat_attr (copy_oper->attr, attr);
    }

  attr = L_find_attr(oper->attr, "tr");
  if (!attr)
    {
      attr = L_new_attr("tr", 0);
      oper->attr = L_concat_attr(oper->attr, attr);
    }
  L_set_attr_field(attr, attr->max_field, L_new_macro_operand(L_MAC_SP,
                                               M_native_int_register_ctype(),
                                               L_PTYPE_NULL)); 
  return;
}

/****************************************************************************
 *
 * routine: O_update_stack_references()
 * purpose: checks oper for stack reference and updates reference if necessary
 * input: oper                 op to examine and change if needed
 *        local_offset         offsets into local stack frame
 *        spill_offset      
 *        input_parm_offset
 *        output_parm_offset
 *        variable_frame_size  does function have alloca?
 *-------------------------------------------------------------------------*/

static void
O_update_stack_references (L_Func * fn, L_Cb * cb, L_Oper * oper,
			   int local_offset,
			   int spill_offset,
			   int input_parm_offset,
			   int output_parm_offset, int variable_frame_size)
{
  L_Operand *stack_operand;
  L_Operand *other_operand;
  L_Attr *attr;
  int operand;
  int adjustment_applied = 0, special_case;
  int uses_stack_pointer;
  int vararg_ip = 0;           /* Is this an IP reference in a vararg function */
  int subtract = 0;            /* Is this op a subtract */

  /* Don't need to check defines */
  if (oper->opc == Lop_DEFINE)
    return;

  if (output_parm_offset != 0)
    L_punt ("O_update_stack_references: OP offset must be zero");

  /* check to see if the oper uses a stack pointer. */
  uses_stack_pointer = 0;
  for (operand = 0;
       (operand < L_max_src_operand) && oper->src[operand]; operand++)
    {
      if (L_is_macro (oper->src[operand]) &&
	  ((oper->src[operand]->value.mac == L_MAC_OP) ||
	   (oper->src[operand]->value.mac == L_MAC_IP) ||
	   (oper->src[operand]->value.mac == L_MAC_LV) ||
	   (oper->src[operand]->value.mac == L_MAC_SP) ||
	   (oper->src[operand]->value.mac == L_MAC_FP)))
	{
	  uses_stack_pointer = 1;
	  break;
	}
    }

  /* Update stack offset attribute */
  if ((attr = L_find_attr (oper->attr, "stack")))
    {
      stack_operand = attr->field[0];
      other_operand = attr->field[1];

      switch (stack_operand->value.mac)
	{
	case L_MAC_LV:
	  adjustment_applied = local_offset;
	  break;
	case L_MAC_SP:
	  adjustment_applied = spill_offset;
	  break;
	case L_MAC_OP:
	  adjustment_applied = output_parm_offset;
	  break;
	case L_MAC_IP:
	  adjustment_applied = input_parm_offset;
	  break;
	case L_MAC_FP:
	  adjustment_applied = 0;
	  break;
	default:
	  L_punt ("O_update_stack_references: "
		  "Unknown stack pointer in attr on op " "%d", oper->id);
	}
      if (!variable_frame_size || (stack_operand->value.mac == L_MAC_OP))
	stack_operand->value.mac = L_MAC_SP;
      else
	stack_operand->value.mac = L_MAC_FP;
      other_operand->value.i += adjustment_applied;
    }

  if (!uses_stack_pointer)
    return;

  if (L_int_move_opcode(oper))
    {
      /* Change move to an add of zero */
      L_change_opcode(oper, Lop_ADD);
      if ((operand != 0)||(oper->src[1] != NULL))
	{
	  DB_print_oper(oper);
	  L_punt("I don't know how to modify move with two sources");
	}
      oper->src[1] = L_new_int_operand(0, M_native_int_register_ctype());
    }
  else if ((L_general_load_opcode(oper))||(L_general_store_opcode(oper))
	   ||(L_int_add_opcode(oper)))
    {
      if (operand >= 2)
	{
	  DB_print_oper(oper);
	  L_punt("I don't know how to modify instruction with stack operand");
	}
    }
  else if (L_int_sub_opcode(oper))
    {
      if ((operand == 0)&&(L_is_int_constant(oper->src[1])))
	{
	  oper->src[1]->value.i = -oper->src[1]->value.i;

	  if (L_is_opcode (Lop_SUB, oper))
	    L_change_opcode(oper, Lop_ADD);
	  else
	    L_change_opcode(oper, Lop_ADD_U);
	}	  
      else if (operand >= 2)
	{
	  DB_print_oper(oper);
	  L_punt("I don't know how to modify instruction with stack operand");
	}
      else
	subtract = 1;
    }
  else
    {
      DB_print_oper(oper);
      L_punt("I don't know how to modify instruction with stack operand");
    }

  if (L_find_attr (oper->attr, "ALLOCA"))
    return;

  stack_operand = oper->src[operand];
  other_operand = operand ? oper->src[0] : oper->src[1];

  if ((stack_operand->value.mac == L_MAC_IP)&&(L_find_attr (fn->attr, "VARARG")))
    {
      /* In order for Lemulate to work, it has to be able to put
	 input parameters back into IP references for vararg functions */
      vararg_ip = 1;
    }

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
	    {
	      if (subtract)
		adjustment_applied = -adjustment_applied;
	      O_update_stack_const_in_prev_oper (fn, cb, oper, &stack_operand,
					       other_operand,
					       adjustment_applied,
					       variable_frame_size, 0);
	    }
	  else
	    L_punt ("O_update_stack_references: ADD trouble on oper %d",
		    oper->id);
	}
      break;
    case L_MAC_SP:
      adjustment_applied = spill_offset;
      break;
    case L_MAC_IP:
      adjustment_applied = input_parm_offset;
      if (!L_is_constant (other_operand))
	{
	  special_case = 1;
	  if (L_is_variable (other_operand))
	    {
	      if (subtract)
		adjustment_applied = -adjustment_applied;
	      O_update_stack_const_in_prev_oper (fn, cb, oper, &stack_operand,
						 other_operand,
						 adjustment_applied,
						 variable_frame_size, vararg_ip);
	    }
	  else
	    L_punt ("O_update_stack_references: ADD trouble on oper %d",
		    oper->id);
	}
      break;
    case L_MAC_FP:
    case L_MAC_OP:
      adjustment_applied = 0;
      special_case = 1;
      break;
    default:
      L_punt ("O_update_stack_references: Unknown stack pointer on op "
	      "%d", oper->id);
    }

  if (!special_case)
    {
      if (L_is_constant (other_operand))
	{
	  if (vararg_ip)
	    {
	      /* Remember original operands so that Lemulate can use them */
	      attr = L_new_attr("vararg_ip",2);
	      attr->field[0] = L_copy_operand(oper->src[0]);
	      attr->field[1] = L_copy_operand(oper->src[1]);
	      oper->attr = L_concat_attr (oper->attr, attr);
	    }
	  if (!subtract)
	    {
	      other_operand->value.i += adjustment_applied;
	    }
	  else
	    {
	      other_operand->value.i -= adjustment_applied;
	    }
	}
      else
	{
	  L_punt ("O_update_stack_references: ADD trouble on oper:%d",
		  oper->id);
	}
    }
  /* change all aliases of the SP to SP */
  if (stack_operand->value.mac != L_MAC_FP)
    {
      if (!variable_frame_size || (stack_operand->value.mac == L_MAC_OP))
	stack_operand->value.mac = L_MAC_SP;
      else
	stack_operand->value.mac = L_MAC_FP;
    }

  return;
}

/****************************************************************************
 *
 * routine: O_postpass_adjust_memory_stack()
 * purpose: Insert code to adjust sp.
 *          Change all references to stack pointers so that they all have
 *          references to and offsets from SP.  This routine should be called 
 *          after register allocation and preferably before postpass scheduling.
 * input: fn              - function.
 *        swap_space_size - size in bytes of stack space used to spill.
 *-------------------------------------------------------------------------*/

void
O_postpass_adjust_memory_stack (L_Func * fn, int swap_space_size)
{
  L_Cb *cb;
  L_Oper *oper, *next_oper;
  int stack_frame_size = 0;
  int variable_frame_size;
  int local_offset, spill_offset, output_parm_offset, input_parm_offset;

  fn->attr = L_concat_attr (fn->attr, L_new_attr ("adjusted_memory_stack", 0));

  L_do_flow_analysis (fn, LIVE_VARIABLE | REACHING_DEFINITION);

  if (L_find_attr (fn->attr, "ALLOCA"))
    variable_frame_size = 1;
  else
    variable_frame_size = 0;

  /* If the function allocates additional stack space, we need to
   * keep track of the previous stack pointer (L_MAC_FP)
   * and make all stack segments except output params relative to
   * PSP
   */

  /* STACK FRAME SETUP
   * ----------------------------------------------------------------------
   * All sections positively indexed
   */

  /* input parameter spill space
   * ----------------------------------------------------------------------
   * Indexed off of MAC $IP (previous stack pointer)
   * In IMPACT stack model IP spills are all in the previous stack frame
   * and thus don't take up space in the current frame.
   */

  /* local variable space
   * ----------------------------------------------------------------------
   */

  stack_frame_size += ADDR_ALIGN (fn->s_local, (M_impact_type_align(M_TYPE_DOUBLE)/8));
  local_offset = stack_frame_size;
  
  /* spill space
   * ----------------------------------------------------------------------
   * Locations for spilled variables
   */
  
  stack_frame_size += ADDR_ALIGN (swap_space_size, (M_impact_type_align(M_TYPE_DOUBLE)/8));
  spill_offset = stack_frame_size;

  /* outgoing parameter space
   * ----------------------------------------------------------------------
   */

  stack_frame_size += ADDR_ALIGN (fn->s_param, (M_impact_type_align(M_TYPE_DOUBLE)/8));
  output_parm_offset = stack_frame_size;

  /* Callees scratch space
   * ----------------------------------------------------------------------
   * Locations for callee to spill input paramters using IP
   */
   stack_frame_size += ADDR_ALIGN (16, (M_impact_type_align(M_TYPE_DOUBLE)/8));

  /* Offset adjustment
   * ----------------------------------------------------------------------
   * Make all offsets relative to the new SP (base of local stack frame)
   */
 
  if (!variable_frame_size)
    {
      /* SP-relative */
      local_offset      = stack_frame_size - local_offset;
      spill_offset      = stack_frame_size - spill_offset;
      input_parm_offset = stack_frame_size;
    }
  else
    {
      /* PSP-relative */
      local_offset      = -local_offset;
      spill_offset   = -spill_offset;
      input_parm_offset = 0; 
    }
  
  output_parm_offset = 0;

  if (Limpact_debug_stack_frame)
    {
      if (variable_frame_size)
	fprintf (stderr, "VARIABLE FRAME SIZE: Offsets from PSP\n");
      else
	fprintf (stderr, "FIXED FRAME SIZE: Offsets from SP\n");
      
      fprintf (stderr, "---------------------        sp offset     size\n");
      fprintf (stderr, "Outgoing param space            %4d        %4d\n",
	       output_parm_offset, fn->s_param);
      fprintf (stderr, "Spill/fill space                %4d        %4d\n",
	       spill_offset, swap_space_size);
      fprintf (stderr, "Local variable space            %4d        %4d\n",
	       local_offset, fn->s_local);
      fprintf (stderr, "Input param offset              %4d           -\n",
	       input_parm_offset);
      fprintf (stderr, "Total stack frame size                      %4d\n",
	       stack_frame_size);
      fprintf (stderr, "---------------------\n");
    }
  
  /* Annotate existing mcode */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = next_oper)
	{
	  next_oper = oper->next_op;
	  
	  /* Must add code after the alloc so that dependencies are observed
	     for pass2 scheduler.  The rule is that the code after post_pass
	     annotation should be runnable.                                  */
	  
	  switch (oper->opc)
	    {
	    case Lop_PROLOGUE:	      
	      O_update_stack_pointer (cb, oper, stack_frame_size,
				      O_STK_DEC, variable_frame_size);
	      L_delete_oper (cb, oper);	/* Delete PROLOGUE oper */
	      break;
	      
	    case Lop_EPILOGUE:	      
	      L_delete_oper (cb, oper);	/* Delete EPI oper */
	      break;
	      
	    case Lop_RTS:
	      O_update_stack_pointer (cb, oper, stack_frame_size,
				      O_STK_INC, variable_frame_size);
	      break;

	    default:
	      O_update_stack_references (fn, cb, oper, local_offset + fn->s_local,
					 spill_offset,
					 input_parm_offset,
					 output_parm_offset,
					 variable_frame_size);
	    }
	}
    }
}
