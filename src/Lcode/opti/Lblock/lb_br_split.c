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
 *      File :          l_br_split.c
 *      Description :   Breaks conditional branches up into a predicate
 *                      definition and a predicated uncond branch.
 *      Author: David August
 *
 *      Copyright (c) 1997 David August
 *      All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_flow.h"

void
LB_branch_split_cb (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper;
  L_Oper *new_pred_def;
  L_Oper *new_jump;
  L_Oper *next_oper;
  int change;
  int has_fallthrough;

  change = 0;

  has_fallthrough = L_has_fallthru_to_next_cb (cb);

  for (oper = cb->first_op; oper; oper = next_oper)
    {
      next_oper = oper->next_op;

      if (!L_cond_branch_opcode (oper))
	continue;

      new_pred_def = L_create_new_op (oper->opc);
      L_change_to_cmp_op (new_pred_def);
      L_copy_compare (new_pred_def, oper);

      new_pred_def->attr = L_copy_attr (oper->attr);
      new_pred_def->weight = oper->weight;

      fn->max_reg_id++;

      if (oper->pred[0])
	new_pred_def->pred[0] = L_copy_operand (oper->pred[0]);

      new_pred_def->dest[0] = L_new_register_operand (fn->max_reg_id,
						      L_CTYPE_PREDICATE,
						      L_PTYPE_UNCOND_T);
      new_pred_def->src[0] = L_copy_operand (oper->src[0]);
      new_pred_def->src[1] = L_copy_operand (oper->src[1]);

      new_jump = L_create_new_op (Lop_JUMP);

      new_jump->src[0] = L_new_cb_operand (oper->src[2]->value.cb);
      new_jump->pred[0] = L_new_register_operand (fn->max_reg_id,
						  L_CTYPE_PREDICATE,
						  L_PTYPE_NULL);

      new_jump->attr = L_copy_attr (oper->attr);
      new_jump->weight = oper->weight;

      L_insert_oper_before (cb, oper, new_pred_def);
      L_insert_oper_after (cb, new_pred_def, new_jump);
      L_delete_oper (cb, oper);

      change++;
    }
  if (change)
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
      if (!has_fallthrough)
	cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
    }
}


void
LB_branch_split_func (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      LB_branch_split_cb (fn, cb);
    }
}
