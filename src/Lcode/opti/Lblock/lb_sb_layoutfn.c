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
 *      File :          l_layoutfn.c
 *      Description :   Tools for trace formation
 *      Creation Date : September 1997
 *      Authors :       David August, Kevin Crozier
 *
 *      (C) Copyright 1997, David August, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_sb_superblock.h"

/* static int LB_switch_taken_and_fall_thru_path (L_Cb * cb)
 * ----------------------------------------------------------------------
 * Switch the taken/fall_thru path for a cb ending in a conditional 
 * branch or a conditional_branch followed by a jump.               
 *                                                                  
 * Always returns a cb ending in a conditional branch.  This means
 * that the following cb must be moved into the correct position.
 * THIS MUST BE DONE TO INSURE PROPER CODE EXECUTION.
 */   

static int
LB_switch_taken_and_fall_thru_path (L_Cb * cb)
{
  L_Cb *taken_cb, *fall_thru_cb;
  L_Flow *taken_flow, *fall_thru_flow, *match_taken, *match_fall_thru;
  L_Oper *jump_op, *branch_op, *new_op;

  /* Get the flows for this cb */
  fall_thru_flow = L_find_last_flow (cb->dest_flow);
  taken_flow = fall_thru_flow->prev_flow;

  if (!cb->last_op)
    return 0;

  if (L_uncond_branch_opcode (cb->last_op))
    {
      if (!cb->last_op->prev_op ||
	  !L_cond_branch_opcode (cb->last_op->prev_op))
	return 0;

      branch_op = cb->last_op->prev_op;
      jump_op = cb->last_op;
      taken_cb = branch_op->src[2]->value.cb;
      fall_thru_cb = jump_op->src[0]->value.cb;
    }
  else if (L_cond_branch_opcode (cb->last_op))
    {
      branch_op = cb->last_op;
      jump_op = NULL;
      taken_cb = branch_op->src[2]->value.cb;
      fall_thru_cb = fall_thru_flow->dst_cb;
    }
  else
    {
      return 0;
    }

  if (L_is_predicated (branch_op))
    return 0;

  /* Change branch_op (opcode and dest) to goto what was the fall_thru_cb */

  L_change_cond_br (branch_op,
		    L_opposite_pred_completer (L_get_compare_type (branch_op)),
		    fall_thru_cb);

  /* If jump already exists, change dest to what was the taken path.
   * Otherwise insert a jump to what was the taken path.
   */
  if (jump_op)
    {
      L_delete_operand (jump_op->src[0]);
      jump_op->src[0] = L_new_cb_operand (taken_cb);
    }
  else
    {
      new_op = L_create_new_op (Lop_JUMP);
      new_op->src[0] = L_new_cb_operand (taken_cb);
      L_insert_oper_after (cb, cb->last_op, new_op);
    }

  /* Switch flow taken and fall-thru in cb's dest list, and just
   * change the condition codes of their matching arcs.  Cannot switch
   * matching arcs because they are in the wrong list. 
   */

  /* Find matching taken/last flows in src lists */
  match_taken = L_find_matching_flow (taken_flow->dst_cb->src_flow, 
				      taken_flow);
  match_fall_thru = L_find_matching_flow (fall_thru_flow->dst_cb->src_flow, 
					  fall_thru_flow);

  /* Swap data in taken and flow thru arcs in cb's dest list.  Use the
   * (opposite) match's data to do the switch.  
   */

  L_change_flow (taken_flow, 1, cb, fall_thru_cb, match_fall_thru->weight);
  L_change_flow (fall_thru_flow, 0, cb, taken_cb, match_taken->weight);

  /* Change ONLY the condition codes for matching flow arcs */

  L_change_flow (match_taken, 0, match_taken->src_cb,
		 match_taken->dst_cb, match_taken->weight);
  L_change_flow (match_fall_thru, 1, match_fall_thru->src_cb,
		 match_fall_thru->dst_cb, match_fall_thru->weight);

  return 1;
}


void
LB_correct_flow_inst (L_Cb * src_cb)
{
  L_Flow *penult_fl, *ult_fl;
  L_Oper *ult_op, *penult_op;
  L_Cb *ft_cb;

  if (!src_cb->dest_flow || (src_cb->flags & L_CB_HAS_JRG))
    return;

  ult_fl = L_find_last_flow (src_cb->dest_flow);
  penult_fl = ult_fl->prev_flow;
  ft_cb = src_cb->next_cb;

  ult_op = L_find_branch_for_flow (src_cb, ult_fl);

  if (ult_fl->dst_cb == ft_cb)
    {
      /* flow to next cb is last -- a natural fallthrough will suffice */
      if (ult_op &&
	  !L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_BOUNDARY) &&
	  !L_EXTRACT_BIT_VAL (ft_cb->flags, L_CB_BOUNDARY))
	{
	  /* REH 7/19/95 - Cannot delete uncond jump to boundary cb */
	  L_delete_oper (src_cb, ult_op);
	  src_cb->flags =
	    L_CLR_BIT_FLAG (src_cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
	  ult_fl->cc = 0;
	}
    }
  else
    {
      /* flow to next cb is not last -- may need fixup */
      if (penult_fl && (penult_fl->dst_cb == ft_cb) &&
	  !L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_BOUNDARY) &&
	  !L_EXTRACT_BIT_VAL (ft_cb->flags, L_CB_BOUNDARY))
	{
	  /* REH 7/19/95 - Cannot delete uncond jump to boundary cb */
	  penult_op = L_find_branch_for_flow (src_cb, penult_fl);
	  if (!L_is_predicated (penult_op) &&
	      (!penult_op->next_op || ((penult_op->next_op == ult_op)
				       && !L_is_predicated (ult_op))))
	    {
	      LB_switch_taken_and_fall_thru_path (src_cb);
	      ult_fl = L_find_last_flow (src_cb->dest_flow);
	      if ((ult_op = L_find_branch_for_flow (src_cb, ult_fl)))
		{
		  L_delete_oper (src_cb, ult_op);
		  src_cb->flags =
		    L_CLR_BIT_FLAG (src_cb->flags,
				    L_CB_HYPERBLOCK_NO_FALLTHRU);
		  ult_fl->cc = 0;
		}
	      return;
	    }
	}

      if (!ult_op)
	{
	  ult_op = L_create_new_op (Lop_JUMP);
	  ult_op->src[0] = L_new_cb_operand (ult_fl->dst_cb);
	  L_insert_oper_after (src_cb, src_cb->last_op, ult_op);
	  
	  ult_fl->cc = 1;
	}
    }

  return;
}


#if 0
static int
LB_select_fallthru_cb (L_Cb *cb, L_Cb *ft_cb)
{
  L_Flow *fl, *lfl, *mfl;
  L_Oper *br, *op;

  if (!cb->dest_flow || (cb->flags & L_CB_HAS_JRG))
    return 0;

  lfl = fl = L_find_last_flow (cb->dest_flow);

  while (fl)
    {
      if (fl->dst_cb == ft_cb)
	break;
      fl = fl->prev_flow;
    }

  if (!fl)
    return 0;

  /* Want to make fl the fall-through flow */

  br = L_find_branch_for_flow (cb, fl);

  if (fl == lfl)
    {
      /* Flow is already the last (fall-through) flow. */
      if (br)
	{
	  /* Dispose of now unnecessary uncond br */
	  if (!L_uncond_branch_opcode (br))
	    L_punt ("LB_select_fallthru_cb: error 1");
	  L_delete_oper (cb, br);
	  mfl = L_find_matching_flow (ft_cb->src_flow, fl);
	  mfl->cc = fl->cc = 0;
	}
      return 1;
    }

  /* Desired ft flow is not the last.  It can be made the last if
   * it is the taken path of a final conditional branch or the taken
   * path of an unconditional branch in an frp block.
   */

  /* Check for frp */

  for (op = br->next_op; op; op = op->next_op)
    if (PG_intersecting_predicates_ops (op, br))
      break;
  
  if (op && !LB_convert_to_frp (cb, br, NULL))
    return 0; /* block is not frp */

  /* cond br may have been expanded */
  br = L_find_branch_for_flow (cb, fl);

  if (!L_find_branch_for_flow (cb, lfl))
    return 0;

  /* transform */

  mfl = L_find_matching_flow (lfl->dst_cb->src_flow, lfl);
  mfl->cc = lfl->cc = 1;

  L_remove_flow (cb->dest_flow, fl);
  L_concat_flow (cb->dest_flow, fl);

  L_delete_oper (cb, br);

  mfl = L_find_matching_flow (ft_cb->src_flow, fl);
  mfl->cc = fl->cc = 0;

  return 1; /* success */
}
#endif


void
LB_layout_function (L_Func * fn, LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;
  int tr_num;
  L_Cb *curr_cb, *prev_cb = NULL;

  tr_num = (int)(L_find_attr (fn->first_cb->attr, "trace")->field[0]->value.i);
  tr = LB_find_traceregion_of_number (header, tr_num);

  List_start (header->inorder_trs);
  while ((tr = (LB_TraceRegion *) List_next (header->inorder_trs)))
    {
      Graph_dfs_topo_sort (tr->flow_graph);
      List_start (tr->flow_graph->topo_list);
      while ((curr_cb = LB_next_cb_in_region (tr)))
	{
	  if (prev_cb)
	    {
	      /* Correct linked list arcs */
	      if (prev_cb->next_cb != curr_cb)
		{
		  L_remove_cb (fn, curr_cb);
		  L_insert_cb_after (fn, prev_cb, curr_cb);
		}

	      /* Correct branch/jump instructions */
	      LB_correct_flow_inst (prev_cb);
	    }
	  prev_cb = curr_cb;
	}
    }

  LB_correct_flow_inst (prev_cb);
  L_rebuild_src_flow (fn);
  return;
}
