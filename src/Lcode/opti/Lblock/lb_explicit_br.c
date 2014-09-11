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
 *      File :          l_explicit_br.c
 *      Description :   All branches are made explicit
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
LB_insert_explicit_branches_cb (L_Cb * cb)
{
  L_Oper *new_jump;

  if (!L_has_fallthru_to_next_cb (cb))
    return;

  /* Hyperblock has no fall through now */
  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    return;

  if (!cb->next_cb)
    L_punt ("L_insert_explicit_branches_cb: Fall through to nothing.");

  if (cb->weight > L_min_fs_weight)
    new_jump = L_create_new_op (Lop_JUMP_FS);
  else
    new_jump = L_create_new_op (Lop_JUMP);
  new_jump->src[0] = L_new_cb_operand (cb->next_cb);
  L_insert_oper_before (cb, NULL, new_jump);

  /* Hyperblock has no fall through now */
  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
      !L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
    cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);

#if 0
  {
    L_Flow *dst_flow;
    L_Flow *src_flow;

    /* Update condition code */
    dst_flow = L_find_flow_for_branch (cb, new_jump);
    src_flow = L_find_flow (dst_flow->dst_cb->src_flow, dst_flow->cc,
			    dst_flow->src_cb, dst_flow->dst_cb);
    dst_flow->cc = 1;
    src_flow->cc = 1;
  }
#endif
}


void
LB_insert_explicit_branches_func (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    LB_insert_explicit_branches_cb (cb);
}



void
LB_delete_explicit_branches_cb (L_Cb * cb)
{
  L_Oper *jump;

  if (L_uncond_branch_opcode (cb->last_op))
    {
      jump = cb->last_op;
      if (jump->src[0]->value.cb != cb->next_cb)
	return;

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
	  L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
	{
          cb->flags =
	     L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
	}

#if 0
      {
	L_Flow *dst_flow;
	L_Flow *src_flow;

	/* Update condition code */
	dst_flow = L_find_flow_for_branch (cb, jump);
	src_flow = L_find_flow (dst_flow->dst_cb->src_flow, dst_flow->cc,
				dst_flow->src_cb, dst_flow->dst_cb);
	dst_flow->cc = 0;
	src_flow->cc = 0;
      }
#endif
      L_delete_oper (cb, jump);
    }
}

void
LB_delete_explicit_branches_func (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    LB_delete_explicit_branches_cb (cb);
}
