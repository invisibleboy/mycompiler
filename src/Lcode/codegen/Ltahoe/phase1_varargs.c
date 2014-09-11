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
 * phase1_varargs.h                                                          *
 * ------------------------------------------------------------------------- *
 * Handle varargs in accordance with software conventions                    *
 *                                                                           *
 * AUTHORS: J. Pierce                                                        *
 *****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "phase1_func.h"
#include "phase1_varargs.h"

/***********************************************************************/
/*                     Varargs Handling Code                           */
/***********************************************************************/

int
Ltahoe_is_vararg_func (L_Func * fn)
{
  if (L_find_attr (fn->attr, "VARARG"))
    return 1;
  else
    return 0;
}

/***********************************************************************/
/***********************************************************************/

/*             Floint Point Parameter Passing Code                     */

/***********************************************************************/
/***********************************************************************/

/* When calling a varargs function, floating point params must be passed 
   both in floating point registers and on the int register stack.  The 
   params must be on the int register stack because a vararg function 
   first moves int input registers to memory and then thereafter accesses 
   params from memory.  

   Since it is hard or sometimes impossible to tell if a function is a
   vararg function, the Electron convention is to always pass float
   params in both places.  HtoL issues a move to floating point macro
   instruction; this function finds these fp parameter instructions
   and creates a duplicate move to the proper register file location.

   The nth parameter always goes in the nth int register file output
   register.  The 1st fp param is put in f2, the 2nd in f3, etc.
   Attributes in the param move and the jsr oper record the param
   positions.  Unfortunately, Lopti sometimes loses the attributes on
   the moves.  If this happens, L_reconstruct_out_param_attr restores
   them by looking for the parameter in the next jsr attribute. */

/*
 * Ltahoe_insert_fp_param_move_at_jsr
 * ----------------------------------------------------------------------
 * In accordance with the software conventions, add moves of floating
 * point parameters into the integer stacked parameter registers at jsrs.
 */

static void
Ltahoe_insert_fp_param_moves_at_jsr (L_Cb * cb, L_Oper * jsr_oper)
{
  L_Oper *mov_oper;
  L_Attr *tr_attr, *op_attr = NULL;
  int num_stack_param;
  int fp_mac, int_mac, i;

  if (!(tr_attr = L_find_attr (jsr_oper->attr, "tr")))
    return;

  num_stack_param = tr_attr->max_field;

  for (i = 0; i < num_stack_param; i++)
    {
      if (LT_is_float_output_param_operand (tr_attr->field[i]))
	{
	  fp_mac = tr_attr->field[i]->value.mac;

	  if (!op_attr)
	    if (!(op_attr = L_find_attr (jsr_oper->attr, "op_info")))
	      L_punt ("Ltahoe_insert_fp_param_move_at_jsr: "
		      "op_info is required");

	  int_mac = L_MAC_P8 + op_attr->field[(2 * i) + 1]->value.i;

	  mov_oper = L_create_new_op (Lop_F_I);
	  mov_oper->proc_opc = TAHOEop_GETF_D;
	  mov_oper->src[0] = L_new_macro_operand (fp_mac, L_CTYPE_DOUBLE, 0);
	  mov_oper->dest[0] = L_new_macro_operand (int_mac, L_CTYPE_LLONG, 0);
	  mov_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);

	  L_insert_oper_before (cb, jsr_oper, mov_oper);

	  /*
	   * Add the int mac to the tr to prevent deadcode from 
	   * eating the move
	   */

	  L_set_macro_attr_field (tr_attr, tr_attr->max_field,
				  int_mac, L_CTYPE_LLONG, L_PTYPE_NULL);
	}
    }
  return;
}

void
Ltahoe_adjust_fp_parameter_passing (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_subroutine_call_opcode (oper))
	    Ltahoe_insert_fp_param_moves_at_jsr (cb, oper);
	}
    }
  return;
}
