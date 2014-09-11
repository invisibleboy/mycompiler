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
 *	File :		l_elim.c
 *	Description :	Uncond jump elimination in hyperblocks
 *	Creation Date :	September 1993
 *	Authors : 	Scott Mahlke
 *        Included in the Lblock library by Kevin Crozier -- 4/98
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_flow.h"

#define ERR stderr
#undef DEBUG_ELIM1
#undef DEBUG_ELIM2

/*
 *    Match the following pattern and replace it.  This occurs because:
 *         A. (cb1 = next_cb) in layout : This occurs because convert loops
 *              with multiple backedges to single backedge loops, but may
 *              include only 1 backedge within the hyperblock.
 *         B. (condbr taken less than jump) : Just invert the 2 so execute
 *              fewer branches on average.
 *
 *      cond_br cb1
 *      jump cb2
 *  ====>
 *      inverse cond br to cb2
 *      jump cb1 (if cb1=next_cb, this is eliminated later)
 */
static void
LB_do_uncond_branch_elim1 (L_Func * fn)
{
  int old_cc;
  L_Cb *cb, *br_dst, *jump_dst;
  L_Oper *br, *jump;
  L_Flow *flow_br, *flow_br2, *flow_jump, *flow_jump2, *last_flow;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      jump = cb->last_op;
      if (!L_uncond_branch_opcode (jump))
	continue;
      br = jump->prev_op;
      if (!L_cond_branch_opcode (br))
	continue;
      if (!L_same_operand (br->pred[0], jump->pred[0]))
	continue;
      flow_br = L_find_flow_for_branch (cb, br);
      flow_jump = flow_br->next_flow;
      if (!flow_br || !flow_jump)
	L_punt ("L_do_uncond_branch_elim2: corrupt flow list cb %d", cb->id);
      br_dst = br->src[2]->value.cb;
      jump_dst = jump->src[0]->value.cb;
      if (jump_dst == cb->next_cb)
	continue;
      if (!((br_dst == cb->next_cb) || (flow_jump->weight > flow_br->weight)))
	continue;
      /* special case, superblock loops, dont want to create extra exit
       * branch in loop body */
      if (br_dst == cb)
	continue;

#ifdef DEBUG_ELIM1
      fprintf (ERR, "Invert br %d and jump %d (cb %d)\n",
	       br->id, jump->id, cb->id);
#endif
      flow_br2 = L_find_matching_flow (br_dst->src_flow, flow_br);
      flow_jump2 = L_find_matching_flow (jump_dst->src_flow, flow_jump);

      /* invert cond br, swap destinations */

      L_negate_compare (br);
      L_change_branch_dest (br, br_dst, jump_dst);
      L_change_branch_dest (jump, jump_dst, br_dst);

      /* re-order flow arcs */
      last_flow = flow_jump->next_flow;
      cb->dest_flow = L_remove_flow (cb->dest_flow, flow_br);
      if (last_flow != NULL)
	cb->dest_flow = L_remove_flow (cb->dest_flow, last_flow);
      cb->dest_flow = L_concat_flow (cb->dest_flow, flow_br);
      if (last_flow != NULL)
	cb->dest_flow = L_concat_flow (cb->dest_flow, last_flow);
      old_cc = flow_jump->cc;
      flow_jump->cc = flow_br->cc;
      flow_jump2->cc = flow_br->cc;
      flow_br->cc = old_cc;
      flow_br2->cc = old_cc;

    }

  return;
}

/*
 *    remove all jumps (predicated or not) to next cb from the
 *      hyperblock.
 */
#if 0
static void
LB_do_uncond_branch_elim2 (L_Func * fn)
{
  int flag;
  double weight;
  L_Cb *cb, *next_cb;
  L_Flow *flow, *flow2;
  L_Oper *oper, *next_op, *last_br;

  for (cb = fn->first_cb; cb != NULL; cb = next_cb)
    {
      next_cb = cb->next_cb;

      flag = 0;
      weight = 0.0;
      for (oper = cb->first_op; oper != NULL; oper = next_op)
	{
	  next_op = oper->next_op;
	  if (!L_uncond_branch_opcode (oper))
	    continue;
	  if (oper->src[0]->value.cb != cb->next_cb)
	    continue;
	  if ((!L_is_predicated (oper)) && (oper != cb->last_op))
	    L_punt ("L_uncond_jump_elim: illegal jump in middle of cb %d",
		    cb->id);
#ifdef DEBUG_ELIM2
	  fprintf (ERR, "Eliminate jump %d (cb %d)\n", oper->id, cb->id);
#endif
	  flag = 1;
	  flow = L_find_flow_for_branch (cb, oper);
	  weight += flow->weight;
	  flow2 = L_find_matching_flow (next_cb->src_flow, flow);
	  L_delete_oper (cb, oper);
	  cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
	  next_cb->src_flow = L_delete_flow (next_cb->src_flow, flow2);
	}

      /* see if fallthru flow arc needs to be added */
      if (flag)
	{
	  last_br = L_find_last_branch (cb);
	  if (last_br)
	    {
	      flow = L_find_flow_for_branch (cb, last_br);
	      flow = flow->next_flow;
	    }
	  else
	    {
	      flow = cb->dest_flow;
	    }
	  /* fallthru flow already exists */
	  if (flow)
	    {
	      if ((flow->cc != 0) || (flow->dst_cb != next_cb))
		L_punt ("L_uncond_jump_elim: illegal fallthru flow");
	      flow2 = L_find_matching_flow (next_cb->src_flow, flow);
	      flow->weight += weight;
	      flow2->weight += weight;
	    }
	  /* none exists so create */
	  else
	    {
	      flow = L_new_flow (0, cb, next_cb, weight);
	      cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
	      flow2 = L_new_flow (0, cb, next_cb, weight);
	      next_cb->src_flow = L_concat_flow (next_cb->src_flow, flow2);
	      cb->flags =
		L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
	    }
	}
    }
}
#endif

void
LB_uncond_branch_elim (L_Func * fn)
{
  LB_do_uncond_branch_elim1 (fn);
/*  LB_do_uncond_branch_elim2(fn); */
}
