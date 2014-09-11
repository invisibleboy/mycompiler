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
 *      File :          l_strict_bb.c
 *      Description :   Converts all cbs in a fn to strict basic blocks
 *      Author: David August, Kevin Crozier
 *
 *      Copyright (c) 1997 David August, Kevin Crozier
 *      All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
/* NOTE: At this point (5/6/98) this code does *NOT* break up CBs containing
 * register jumps.  At some point in the future it maybe desirable to include
 * flag to control this behavior. -- KMC
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_flow.h"

L_Cb *
LB_breakup_cb (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper, *branch, *ptr;
  L_Flow *flow, *new_flow;
  L_Cb *new_cb = NULL;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_general_branch_opcode (oper))
	continue;
      branch = oper;
      flow = cb->dest_flow;
      if (L_uncond_branch_opcode (branch->next_op))
	{
	  branch = oper->next_op;
	  flow = flow->next_flow;
	}
      /* end condition */
      if (!branch->next_op)
	break;

      if (L_uncond_branch_opcode (oper))
	L_punt ("L_breakup_cb: "
		"something is wrong here, jump in middle of cb");

      /* create new cb */
      new_cb = L_create_cb (branch->next_op->weight);
      L_insert_cb_after (fn, cb, new_cb);

      /* breakup the oper 2-way linked list */
      new_cb->first_op = branch->next_op;
      new_cb->last_op = cb->last_op;
      cb->last_op = branch;
      branch->next_op->prev_op = NULL;
      branch->next_op = NULL;

      /* Need to update the oper hash table with these changes!! */

      for (ptr = new_cb->first_op; ptr != NULL; ptr = ptr->next_op)
	L_oper_hash_tbl_update_cb (fn->oper_hash_tbl, ptr->id, new_cb);

      /* breakup the flow 1-way linked list */

      new_cb->dest_flow = flow->next_flow;
      flow->next_flow->prev_flow = NULL;
      flow->next_flow = NULL;

      /* create new fallthru flow for cb */
      new_flow = L_new_flow (0, cb, new_cb, new_cb->weight);
      cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);

      /* change src cb of all dest flows of new_cb */
      L_change_src (new_cb->dest_flow, cb, new_cb);
    }
  return new_cb;
}

void
LB_convert_to_strict_basic_block_code (L_Func * fn, int exclude_flags)
{
  int flag;
  L_Cb *cb, *next_cb;
  L_Oper *oper, *next_op;

  flag = 0;

  for (cb = fn->first_cb; cb != NULL; cb = next_cb)
    {
      L_Cb *curr_cb = cb;
      next_cb = cb->next_cb;

      if (cb->flags & exclude_flags)
	continue;

      for (oper = cb->first_op; oper != NULL; oper = next_op)
	{
	  next_op = oper->next_op;

	  if (!L_general_branch_opcode (oper))
	    continue;

	  if (!oper->next_op)
	    break;

	  if (L_cond_branch_opcode (oper) &&
	      L_uncond_branch_opcode (oper->next_op) &&
	      (!oper->next_op->next_op))
	    break;

	  if (!flag)
	    flag = 1;

	  while (curr_cb) 
	    curr_cb = LB_breakup_cb (fn, curr_cb);
	}
    }

  if (flag)
    L_rebuild_src_flow (fn);
}
