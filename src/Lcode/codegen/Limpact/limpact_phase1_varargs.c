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
/*****************************************************************************\
 *
 *  File:  limpact_main.c
 *
 *  Description:
 *    Handle varargs in accordance with impact software conventions
 *
 *  Creation Date :  Nov 2001
 *
 *  Author:  Ronald D. Barnes, Wen-mei Hwu
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "limpact_main.h"
#include <Lcode/l_build_prototype_info.h>

#undef DEBUG_SPILL

/***********************************************************************/
/*                     Varargs Handling Code                           */
/***********************************************************************/

int
Limpact_is_vararg_func (L_Func * fn)
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
   vararg function, the IMPACT convention is to always pass float
   params in both places.  HtoL issues a move to floating point macro
   instruction; this function finds these fp parameter instructions
   and creates a duplicate move to the proper register file location.
 */

/*
 * Limpact_insert_fp_param_move_at_jsr
 * ----------------------------------------------------------------------
 * In accordance with the software conventions, add moves of floating
 * point parameters into the integer stacked parameter registers at jsrs.
 */

static void
Limpact_insert_fp_param_moves_at_jsr (L_Cb * cb, L_Oper * jsr_oper)
{
  L_Oper *new_oper;
  L_Attr *tr_attr, *op_attr = NULL, *desc_attr = NULL;
  int num_stack_param;
  int fp_mac, int_mac, i;
  int param_num;
  char return_type_buf[TYPE_BUF_SIZE];
  char all_parm_type_buf[TYPE_BUF_SIZE];
  char parm_type_buf[TYPE_BUF_SIZE];
  char *parse_ptr = all_parm_type_buf;
  int parm_type;
  
  if (!(tr_attr = L_find_attr (jsr_oper->attr, "tr")))
    return;

  num_stack_param = tr_attr->max_field;

  L_get_call_info (NULL, jsr_oper, jsr_oper->attr, 
		   return_type_buf, all_parm_type_buf,
		   sizeof (return_type_buf));

#ifdef DEBUG_SPILL
  printf("Limpact_insert_fp_param_moves_at_jsr: "
	 "fn %s, cb %d, op %d with %d stack params\n",
	 L_fn->name, cb->id, jsr_oper->id, num_stack_param);
#endif

  for (i = 0; i < num_stack_param; i++)
    {
      /* This routine will add int parms to the tr list, 
	 but prior to this point, the tr list should match
	 the call_info exactly, with the exception that
	 there could be a P15 return struct by value 
	 element in the tr that should be ignored. 
         If the call_info runs out, there should be no
	 more parameters. MCM 2/7/02 */

      if (tr_attr->field[i]->value.mac == 15)
	continue;
      
      if (*parse_ptr == 0)
	break;

#if 0
	L_punt("C_emit_jsr_op: "
	       "Not expecting call_info to be empty. fn %s, op %d\n",
	       L_fn->name, jsr_oper->id);
#endif
      
      /* Get the next parameter type */
      L_get_next_param_type (parm_type_buf, &parse_ptr);
      parm_type = L_convert_type_to_ctype (parm_type_buf);

      if (L_is_ctype_double_direct(parm_type))
	{
	  fp_mac = tr_attr->field[i]->value.mac;

	  if (!op_attr)
	    if (!(op_attr = L_find_attr (jsr_oper->attr, "op_info")))
	      L_punt("Limpact_insert_fp_param_move_at_jsr: "
		     "op_info is required");

	  param_num = op_attr->field[(2 * i)+1]->value.i;
	  int_mac = L_MAC_P0 + param_num;
  
	  /* Use a store and a load or two to move the double registers 
	     into integer parameter registers */
	  
	  /* Create store */
	  new_oper = L_create_new_op(Lop_ST_F2);
	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->src[2] = L_new_macro_operand (fp_mac,
						  L_CTYPE_DOUBLE, 
						  L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);

	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);
	  
	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
#ifdef DEBUG_SPILL
	  printf("double param st: mac P%d at offset %d\n",
		 fp_mac, (param_num*4));
#endif

	  /* Create load */
	  new_oper = L_create_new_op (Lop_LD_I);

	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->dest[0] = L_new_macro_operand (int_mac, L_CTYPE_INT, 
						   L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	  
	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);

	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
#ifdef DEBUG_SPILL
	  printf("double param ld: mac P%d at offset %d\n",
		 int_mac, (param_num*4));
#endif
	  /*
	   * Add the int mac to the tr to prevent deadcode from 
	   * eating the move
	   */
	  
	  L_set_macro_attr_field (tr_attr, tr_attr->max_field,
				  int_mac, L_CTYPE_INT, L_PTYPE_NULL);
	  
	  /* Create second load (if needed) */
	  if (param_num < 3) /* If we are not on the last
				parameter passed by register */
	    {
	      new_oper = L_create_new_op (Lop_LD_I);
	      
	      new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						      L_PTYPE_NULL);
	      new_oper->src[1] = L_new_gen_int_operand ((param_num+1) * 4);
	      new_oper->dest[0] = L_new_macro_operand (int_mac+1, L_CTYPE_INT, 
						       L_PTYPE_NULL);
	      new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	      
	      desc_attr = L_new_attr("param_spill",1);
	      L_set_macro_attr_field (desc_attr, 0,
				      fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	      new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);
	      
	      L_insert_oper_before (cb, jsr_oper, new_oper);
	      
#ifdef DEBUG_SPILL
	      printf("double param ld: mac P%d at offset %d\n",
		     (int_mac+1), ((param_num+1)*4));
#endif
	      /*
	       * Add the int mac to the tr to prevent deadcode from 
	       * eating the move
	       */
	      
	      L_set_macro_attr_field (tr_attr, tr_attr->max_field,
				      int_mac+1, L_CTYPE_INT, L_PTYPE_NULL);
	    }
	}
      else if (L_is_ctype_float_direct(parm_type))
	{
	  fp_mac = tr_attr->field[i]->value.mac;

	  if (!op_attr)
	    if (!(op_attr = L_find_attr (jsr_oper->attr, "op_info")))
	      L_punt("Limpact_insert_fp_param_move_at_jsr: "
		     "op_info is required");

	  param_num = op_attr->field[(2 * i)+1]->value.i;
	  int_mac = L_MAC_P0 + param_num;
	  
	  /* Use a store and a load or two to move the double registers 
	     into integer parameter registers */
	  
	  /* Create store */
	  new_oper = L_create_new_op(Lop_ST_F);
	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->src[2] = L_new_macro_operand (fp_mac,
						  L_CTYPE_DOUBLE, 
						  L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	  
	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);

	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
#ifdef DEBUG_SPILL
	  printf("float param st: mac P%d at offset %d\n",
		 fp_mac, (param_num*4));
#endif

	  /* Create load */
	  new_oper = L_create_new_op (Lop_LD_I);

	  new_oper->src[0] = L_new_macro_operand (L_MAC_OP, L_CTYPE_INT,
						  L_PTYPE_NULL);
	  new_oper->src[1] = L_new_gen_int_operand (param_num * 4);
	  new_oper->dest[0] = L_new_macro_operand (int_mac, L_CTYPE_INT, 
						   L_PTYPE_NULL);
	  new_oper->pred[0] = L_copy_operand (jsr_oper->pred[0]);
	  
	  desc_attr = L_new_attr("param_spill",1);
	  L_set_macro_attr_field (desc_attr, 0,
				  fp_mac, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	  new_oper->attr = L_concat_attr (new_oper->attr, desc_attr);

#ifdef DEBUG_SPILL
	  printf("float param ld: mac P%d at offset %d\n",
		 int_mac, (param_num*4));
#endif
	  L_insert_oper_before (cb, jsr_oper, new_oper);
	  
	  /*
	   * Add the int mac to the tr to prevent deadcode from 
	   * eating the move
	   */
	  
	  L_set_macro_attr_field (tr_attr, tr_attr->max_field,
				  int_mac, L_CTYPE_INT, L_PTYPE_NULL);
	}
    }

  return;
}

void
Limpact_adjust_fp_parameter_passing (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_subroutine_call_opcode (oper))
	    Limpact_insert_fp_param_moves_at_jsr (cb, oper);
	}
    }
  return;
}
