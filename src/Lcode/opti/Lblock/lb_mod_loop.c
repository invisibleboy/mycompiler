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
/****************************************************************************\
 *
 *  File:  l_mod_loop.c
 *
 *  Description: Takes a basic block loop structure and converts it to one
 *	that has only one backedge
 *
 *  Creation Date :  October, 1993
 *
 *  Author:  Roger A. Bringmann, Scott Mahlke
 *           John W. Sias -- added loop collapsing
 *
 *  Revisions:
 *
 * 	(C) Copyright 1993, Roger A. Bringmann, Scott Mahlke
 * 	All rights granted to University of Illinois Board of Regents.
 *
 *      Included into the Lblock library by Kevin Crozier -- 4/98
\****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"
#include "lb_b_internal.h"
#include "lb_flow.h"
#include "lb_hb_peel.h"

#undef DEBUG

#define ERR	stderr

/*
 *    remove all flow connections between src_cb and dst_cb, return
 *      exec frequency this accounts for
 */
/*
 *    alter all connections from src_cb to old_dst_cb to go to new_dst_cb
 */
static void
LB_reconnect_blocks (L_Cb * src_cb, L_Cb * old_dst_cb, L_Cb * new_dst_cb)
{
  L_Flow *flow, *match_flow, *new_flow;
  L_Oper *br, *jump_op;
  double new_scale, old_scale;

  for (flow = src_cb->dest_flow; flow; flow = flow->next_flow)
    {
      if (flow->dst_cb != old_dst_cb)
	continue;

      match_flow = L_find_matching_flow (old_dst_cb->src_flow, flow);
      old_dst_cb->src_flow = L_delete_flow (old_dst_cb->src_flow, match_flow);

      if ((br = L_find_branch_for_flow (src_cb, flow)))
	{
	  L_change_branch_dest (br, old_dst_cb, new_dst_cb);
	}
      else
	{
	  jump_op = L_create_new_op (Lop_JUMP);
	  jump_op->src[0] = L_new_cb_operand (new_dst_cb);
	  L_insert_oper_after (src_cb, src_cb->last_op, jump_op);
	}

      flow->dst_cb = new_dst_cb;

      new_flow = L_copy_single_flow (flow);
      new_dst_cb->src_flow = L_concat_flow (new_dst_cb->src_flow, new_flow);

      if (new_dst_cb->weight > 0.0)
	new_scale = (new_dst_cb->weight + flow->weight) / new_dst_cb->weight;
      else
	new_scale = 0.0;
      if (old_dst_cb->weight > 0.0)
	old_scale = (old_dst_cb->weight - flow->weight) / old_dst_cb->weight;
      else
	old_scale = 0.0;

      L_scale_flow_weights (new_dst_cb->dest_flow, new_scale);
      L_scale_flow_weights (old_dst_cb->dest_flow, old_scale);
      new_dst_cb->weight += flow->weight;
      old_dst_cb->weight -= flow->weight;
    }
  return;
}

static int
LB_are_equivalent_cbs (L_Cb * cb1, L_Cb * cb2)
{
  L_Oper *oper1, *oper2, *ptr;
  L_Operand *dest1, *dest2;
  L_Operand *src1, *src2;
  L_Flow *flow1, *flow2;
  int *tmp_srcs, num_op, position, tmp_position, equiv, i, j;

  num_op = L_cb_size (cb1);
  tmp_srcs = num_op ? 
    (int *) Lcode_calloc (num_op * L_max_src_operand, sizeof (int)) : NULL;

  oper1 = cb1->first_op;
  oper2 = cb2->first_op;
  equiv = 1;
  position = 0;
  while (oper1 && oper2)
    {
      if (!L_equivalent_opcode (oper1, oper2) ||
	  !L_same_operand (oper1->pred[0], oper2->pred[0]))
	{
	  equiv = 0;
	  break;
	}

      if (!L_same_src_operands (oper1, oper2))
	{
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      src1 = oper1->src[i];
	      src2 = oper2->src[i];
	      if (L_same_operand (src1, src2))
		continue;
	      if (!(L_is_register (src1) && L_is_register (src2)))
		{
		  equiv = 0;
		  break;
		}
	      if (tmp_srcs[(position * L_max_src_operand) + i] !=
		  src2->value.r)
		{
		  equiv = 0;
		  break;
		}
	    }
	}

      /* They either better be the same or temporary regs */
      if (!L_same_dest_operands (oper1, oper2))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest1 = oper1->dest[i];
	      dest2 = oper2->dest[i];
	      if (L_same_operand (dest1, dest2))
		continue;
	      if (!(L_is_register (dest1) && L_is_register (dest2)))
		{
		  equiv = 0;
		  break;
		}
	      if (L_in_cb_OUT_set (cb1, dest1) ||
		  L_in_cb_OUT_set (cb2, dest2))
		{
		  equiv = 0;
		  break;
		}
	      /* they are temp regs if reach this point */
	      tmp_position = position + 1;
	      for (ptr = oper1->next_op; ptr; ptr = ptr->next_op)
		{
		  for (j = 0; j < L_max_src_operand; j++)
		    {
		      if (!L_same_operand (ptr->src[j], dest1))
			continue;
		      tmp_srcs[(tmp_position * L_max_src_operand) + j] =
			dest2->value.r;
		    }
		  tmp_position++;
		}
	    }
	}

      if (!equiv)
	break;

      oper1 = oper1->next_op;
      oper2 = oper2->next_op;
      position++;
    }

  /* check fall thru flow arcs */
  flow1 = L_find_last_flow (cb1->dest_flow);
  flow2 = L_find_last_flow (cb2->dest_flow);
  if (flow1->dst_cb != flow2->dst_cb)
    equiv = 0;

  /* fallthru can be with no jump or with jump, so check here */
  if (equiv && (oper1 || oper2) &&
      !L_uncond_branch_opcode (oper1 ? oper1 : oper2))
    equiv = 0;

  if (tmp_srcs)
    Lcode_free (tmp_srcs);

  return (equiv);
}

static int
LB_functionally_equivalent_cbs (Set cb_set)
{
  L_Cb *cb1, *cb2;
  int num_cb, *cb_array, i, equiv;

  num_cb = Set_size (cb_set);
  if (num_cb <= 1)
    return (1);

  cb_array = (int *) Lcode_malloc (sizeof (int) * num_cb);
  Set_2array (cb_set, cb_array);

  if (!(cb1 = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cb_array[0])))
    L_punt ("LB_functionally_equivalent_cbs: corrupt cb_set");

  equiv = 1;
  for (i = 1; i < num_cb; i++)
    {
      if (!(cb2 = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cb_array[i])))
	L_punt ("LB_functionally_equivalent_cbs: corrupt cb_set");
      if (!LB_are_equivalent_cbs (cb1, cb2))
	{
	  equiv = 0;
	  break;
	}
    }

  Lcode_free (cb_array);

  return (equiv);
}

static void
LB_combine_equivalent_backedges (L_Loop * loop)
{
  int num_back_edge, *back_edge_array, i;
  L_Cb *cb, *keep_cb, *src_cb;
  L_Flow *src_flow, *next_flow;

  num_back_edge = Set_size (loop->back_edge_cb);
  if (num_back_edge <= 1)
    return;
  back_edge_array = (int *) alloca (sizeof (int) * num_back_edge);
  Set_2array (loop->back_edge_cb, back_edge_array);

  /* pick the one with the largest weight to keep */

  if (!(keep_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, back_edge_array[0])))
    L_punt ("LB_combine_equivalent_backedges: corrupt backedge cbs");

  for (i = 1; i < num_back_edge; i++)
    {
      if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, back_edge_array[i])))
	L_punt ("LB_combine_equivalent_backedges: corrupt backedge cbs");

      if (cb->weight > keep_cb->weight)
	keep_cb = cb;
    }

#ifdef DEBUG
  fprintf (stderr, "keeping cb %d for equivalent Backedge combining\n",
	   keep_cb->id);
#endif

  for (i = 0; i < num_back_edge; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, back_edge_array[i]);
      if (cb == keep_cb)
	continue;
      for (src_flow = cb->src_flow; src_flow; src_flow = next_flow)
	{
	  next_flow = src_flow->next_flow;
	  src_cb = src_flow->src_cb;
	  LB_reconnect_blocks (src_cb, cb, keep_cb);
	}
    }

  L_rebuild_src_flow (L_fn);
  return;
}

/*
 * LB_elim_loop_backedges
 * ----------------------------------------------------------------------
 * Transform a multiple-backedge loop to have a single backedge
 */

void
LB_elim_loop_backedges (L_Func * fn, L_Loop * loop)
{
  L_Cb *loop_header_cb = loop->header, *loop_end_cb, *cb;
  L_Flow *src_flow, *dst_flow, *back_edge_flow, *header_src_flow;
  L_Oper *oper, *branch;
  int num_back_edges, *back_edge_cb, i;

  /* We will only modify loops with more than one back edge */
  if ((num_back_edges = Set_size (loop->back_edge_cb)) <= 1)
    return;

#ifdef DEBUG
  fprintf (stderr, "Loop %d has %d backedges\n", loop->id, num_back_edges);
  Set_print (stderr, "backedges", loop->back_edge_cb);
#endif

  if (LB_functionally_equivalent_cbs (loop->back_edge_cb) &&
      !Set_in (loop->back_edge_cb, loop->header->id))
    {
#ifdef DEBUG
      fprintf (stderr, "Loop backedge cbs are equivalent!!\n");
#endif
      LB_combine_equivalent_backedges (loop);
      return;
    }

#ifdef DEBUG
  fprintf (stderr, "Loop backedge cbs are NOT equivalent!!\n");
#endif

  /* Create a cb with a back-edge to the loop header cb. */
  loop_end_cb = L_create_cb (0.0);
  header_src_flow = L_new_flow (1, loop_end_cb, loop_header_cb, 0.0);
  loop_header_cb->src_flow =
    L_concat_flow (loop_header_cb->src_flow, header_src_flow);
  back_edge_flow = L_new_flow (1, loop_end_cb, loop_header_cb, 0.0);
  loop_end_cb->dest_flow =
    L_concat_flow (loop_end_cb->dest_flow, back_edge_flow);
  L_insert_cb_after (fn, fn->last_cb, loop_end_cb);

  /* Create the jump instruction that leads to the header cb */
  oper = L_create_new_op (Lop_JUMP);
  oper->src[0] = L_new_cb_operand (loop_header_cb);
  L_insert_oper_after (loop_end_cb, loop_end_cb->first_op, oper);
#ifdef DEBUG
  fprintf (stderr, "created cb %d, placed it after cb %d\n",
	   loop_end_cb->id, loop_end_cb->prev_cb->id);
#endif

  /* 
   * Loop through all loop back-edge cbs and make them branch to the
   * to the new end loop cb.
   */

  back_edge_cb = (int *) Lcode_malloc (sizeof (int) * num_back_edges);
  Set_2array (loop->back_edge_cb, back_edge_cb);

  for (i = 0; i < num_back_edges; i++)
    {
      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, back_edge_cb[i]);

      /* 
       * Find the destination flow arc in the current cb corresponding to the 
       * loop back edge.
       */
      for (dst_flow = cb->dest_flow; dst_flow; dst_flow = dst_flow->next_flow)
	{
	  if (dst_flow->dst_cb != loop_header_cb)
	    continue;

	  /*
	   * Find the src flow arc in the loop header cb corresponding to the
	   * current back edge and delete it.
	   */
	  src_flow = L_find_flow (loop_header_cb->src_flow, dst_flow->cc,
				  dst_flow->src_cb, dst_flow->dst_cb);
	  loop_header_cb->src_flow = L_delete_flow (loop_header_cb->src_flow,
						    src_flow);

	  /* Modify the flow arc and branch target for the branch */
	  branch = L_find_branch_for_flow (cb, dst_flow);

#ifdef DEBUG
	  fprintf (stderr, "\t (cb %d) branch oper %d\n", cb->id, branch->id);
	  fprintf (stderr, "\t");
	  L_print_oper (stderr, branch);
#endif
	  if (branch)
	    {
	      L_change_branch_dest (branch, dst_flow->dst_cb, loop_end_cb);
	    }
	  else
	    {
	      L_Oper *new_op;
	      new_op = L_create_new_op (Lop_JUMP);
	      new_op->src[0] = L_new_cb_operand (loop_end_cb);
	      L_insert_oper_after (cb, cb->last_op, new_op);
	    }

	  dst_flow->dst_cb = loop_end_cb;

	  src_flow =
	    L_new_flow (dst_flow->cc, dst_flow->src_cb, dst_flow->dst_cb,
			dst_flow->weight);
	  loop_end_cb->src_flow =
	    L_concat_flow (loop_end_cb->src_flow, src_flow);
	  loop_end_cb->weight += src_flow->weight;

	  back_edge_flow->weight += src_flow->weight;
	  header_src_flow->weight += src_flow->weight;
	}
    }

  /* Add the new back-edge cb to the back edge cb list for the loop */
  loop->back_edge_cb = Set_dispose (loop->back_edge_cb);
  loop->back_edge_cb = Set_add (loop->back_edge_cb, loop_end_cb->id);
  loop->loop_cb = Set_add (loop->loop_cb, loop_end_cb->id);

  /* free up space */
  Lcode_free (back_edge_cb);

}

void
LB_elim_all_loop_backedges (L_Func * fn)
{
  int loop_count;
  L_Loop *loop;
  Set loop_done;

  /* count how many loops there are */
  loop_count = 0;
  for (loop = fn->first_loop; loop; loop = loop->next_loop)
    loop_count++;

  /* inner most to outer */
  loop_done = NULL;
  while (Set_size (loop_done) < loop_count)
    {
      for (loop = fn->first_loop; loop; loop = loop->next_loop)
	{
	  if (Set_in (loop_done, loop->id))
	    continue;
	  if (!Set_subtract_empty (loop->nested_loops, loop_done))
	    continue;
	  loop_done = Set_add (loop_done, loop->id);

	  LB_elim_loop_backedges (fn, loop);
	}
    }

  Set_dispose (loop_done);
}

/*
 * LOOP COLLAPSING
 * ======================================================================
 *
 */

#define LB_COLLAPSE_MAX_DYN_EXPN 0.2

int
LB_hb_loop_collapsible (LB_TraceRegion_Header *trh, L_Loop * loop)
{
  int par_ops, chi_ops;
  double org_dyn_ops, add_dyn_ops;
  double expn_ratio;
  L_Loop *child_loop;
  L_Cb *par_hdr;

  if (!(child_loop = loop->child_loop))
    return 0;
  if (child_loop->sibling_loop)
    return 0;

  if (L_find_attr (child_loop->header->attr, LB_MARKED_FOR_LOOP_PEEL))
    return 0;

  if (!LB_find_traceregion_by_header (trh, child_loop->header))
    return 0;

  chi_ops = LB_hb_num_ops_in_cb_set (child_loop->loop_cb);
  par_ops = LB_hb_num_ops_in_cb_set (loop->loop_cb) - chi_ops;

  org_dyn_ops = child_loop->num_invocation * chi_ops +
    loop->num_invocation * par_ops;

  add_dyn_ops = (child_loop->num_invocation - loop->num_invocation) * par_ops;

  expn_ratio = add_dyn_ops / org_dyn_ops;

  if (expn_ratio > LB_COLLAPSE_MAX_DYN_EXPN)
    return 0;

  par_hdr = loop->header;

  if (!par_hdr->dest_flow || par_hdr->dest_flow->dst_cb != child_loop->header)
    return 0;

  printf ("Collapsing loop %d\n", loop->id);

  return 1;
}

L_Loop *
LB_hb_do_collapse_loops (LB_TraceRegion_Header *trh,
			 L_Func * fn, L_Loop * loop)
{
  L_Loop *child_loop;
  L_Cb *par_hdr, *chi_hdr, *back_cb;
  L_Oper *new_op;
  LB_TraceRegion *child_tr;
  L_Flow *src_flow, *dst_flow, *nxt_flow;
  double back_wt;

  if (!(child_loop = loop->child_loop))
    return 0;
  if (child_loop->sibling_loop)
    return 0;

  par_hdr = loop->header;
  chi_hdr = child_loop->header;

  if (!(child_tr = LB_find_traceregion_by_header (trh, chi_hdr)))
    return 0;

  if (!par_hdr->dest_flow || par_hdr->dest_flow->dst_cb != chi_hdr)
    return 0;

  /*
   * Restructure control flow to move outer loop header into
   * inner loop tail
   */

  back_wt = par_hdr->weight - loop->num_invocation;
  par_hdr->weight = loop->num_invocation;

  back_cb = L_create_cb (par_hdr->weight);

  L_insert_cb_after (fn, fn->last_cb, back_cb);
  L_copy_block_contents (par_hdr, back_cb);
  back_cb->dest_flow = L_copy_flow (par_hdr->dest_flow);
  src_flow = L_find_matching_flow (chi_hdr->src_flow, back_cb->dest_flow);
  src_flow = L_copy_flow (src_flow);
  back_cb->dest_flow->weight = back_wt;
  src_flow->weight = back_wt;
  chi_hdr->src_flow = L_concat_flow (chi_hdr->src_flow, src_flow);
  src_flow->src_cb = back_cb;
  L_change_src (back_cb->dest_flow, par_hdr, back_cb);

  if (L_uncond_branch (back_cb->last_op))
    {
      L_delete_operand (back_cb->last_op->src[0]);
      back_cb->last_op->src[0] = L_new_cb_operand (chi_hdr);
    }
  else
    {
      new_op = L_create_new_op (Lop_JUMP);
      new_op->src[0] = L_new_cb_operand (chi_hdr);
      L_insert_oper_after (back_cb, back_cb->last_op, new_op);
    }

  {
    L_Cb *src_cb;
    L_Oper *op, *br;

    /* Change all loop body->header flows to the new header copy
     */

    for (src_flow = par_hdr->src_flow; src_flow; src_flow = nxt_flow)
      {
	nxt_flow = src_flow->next_flow;
	src_cb = src_flow->src_cb;
	if (Set_in (loop->back_edge_cb, src_cb->id) && (src_cb != par_hdr))
	  {
	    dst_flow = L_find_matching_flow (src_cb->dest_flow, src_flow);
	    br = L_find_branch_for_flow (src_cb, dst_flow);
	    if (br)
	      {
		L_change_branch_dest (br, par_hdr, back_cb);
	      }
	    else
	      {
		op = L_create_new_op (Lop_JUMP);
		op->src[0] = L_new_cb_operand (back_cb);
		L_insert_oper_after (src_cb, src_cb->last_op, op);
	      }
	    dst_flow->dst_cb = back_cb;
	    par_hdr->src_flow = L_remove_flow (par_hdr->src_flow, src_flow);
	    src_flow->dst_cb = back_cb;
	    back_cb->src_flow = L_concat_flow (back_cb->src_flow, src_flow);
	    loop->back_edge_cb = Set_delete (loop->back_edge_cb, src_cb->id);
	  }
      }
  }

  loop->header = child_loop->header;
  loop->loop_cb = Set_add (loop->loop_cb, back_cb->id);
  loop->loop_cb = Set_delete (loop->loop_cb, par_hdr->id);
  loop->back_edge_cb = Set_add (loop->back_edge_cb, back_cb->id);
  loop->nested_loops = Set_delete (loop->nested_loops, child_loop->id);
  loop->child_loop = NULL;
  L_merge_two_loops (fn, loop, child_loop);

  /*
   * Clear HB selection flags on child region's cbs, if not included in
   * other regions
   */

  {
    LB_TraceRegion *tr;
    L_Cb *cb_iter;
    Set uncov, test, chi;

    uncov = LB_return_cbs_region_as_set (child_tr);

    chi = Set_copy (uncov);

    List_start (trh->traceregions);

    while ((tr = (LB_TraceRegion *) List_next (trh->traceregions)))
      {
	if (tr == child_tr)
	  {
	    trh->traceregions = List_delete_current(trh->traceregions);
	    LB_free_traceregion (tr);
	  }
	else
	  {
	    test = LB_return_cbs_region_as_set (tr);
	    uncov = Set_subtract_acc (uncov, test);
	    Set_dispose (test);
	  }
      }

    for (cb_iter = fn->first_cb; cb_iter; cb_iter = cb_iter->next_cb)
      {
	if (Set_in(uncov, cb_iter->id))
	  cb_iter->flags = L_CLR_BIT_FLAG (cb_iter->flags,
					   L_CB_HYPERBLOCK_LOOP);
      }


    Set_dispose (uncov);
    Set_dispose (chi);
  }

  return loop;
}
