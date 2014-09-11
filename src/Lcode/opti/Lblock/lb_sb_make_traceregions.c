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
 *      File :          l_make_traces.c
 *      Description :   Tools for trace formation with trace-regions
 *      Creation Date : September 1997
 *      Authors :       David August, Kevin Crozier
 *       Modified from David August's orignal make_trace code
 *
 *      (C) Copyright 1997, David August, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_sb_superblock.h"
#undef DEBUG_MAKETR


static void
LB_clear_cb_visit_flags (L_Func * fn, LB_TraceRegion_Header * header)
{
  L_Cb *cb;
  LB_TraceRegion *tr;

  /* Clear all the visited flags for all cbs */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_VISITED);

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      Graph_dfs_topo_sort (tr->flow_graph);
      List_start (tr->flow_graph->topo_list);
      while ((cb = LB_next_cb_in_region (tr)))
	cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_VISITED);
    }
  return;
}


   /*
    * REH 7/2/95 - I do not want boundary condition blocks included
    * within traces at the moment til I better understand the 
    * region reintegration process.
    */
static void
LB_mark_boundary_blocks_visited (L_Func * fn, LB_TraceRegion_Header * header)
{
  L_Cb *cb;
  LB_TraceRegion *tr;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, (L_CB_ENTRANCE_BOUNDARY |
					 L_CB_EXIT_BOUNDARY |
					 L_CB_PROLOGUE |
					 L_CB_EPILOGUE |
					 L_CB_HAS_JRG)) &&
	  !L_EXTRACT_BIT_VAL (cb->flags, L_CB_VISITED))
	{
	  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_VISITED);
	  tr =
	    LB_create_traceregion (fn, header->next_id++, cb, NULL,
				   L_TRACEREGION_TRACE);
	  tr->flow_graph = LB_finish_frp_graph (tr->flow_graph);
	  header->traceregions = List_insert_last (header->traceregions, tr);
	}
    }
}


static L_Cb *
LB_find_epilog_cb (L_Func * fn)
{
  L_Cb *cb;

  /* Search for epilog cb - the one with no dest_flow's */
  for (cb = fn->last_cb; cb != NULL; cb = cb->prev_cb)
    {
      if (!cb->dest_flow)
	return cb;
    }
  return NULL;
}


static L_Cb *
LB_find_cb_seed (L_Func * fn)
{
  L_Cb *cb_indx, *seed = NULL;

  for (cb_indx = fn->first_cb; cb_indx != NULL; cb_indx = cb_indx->next_cb)
    if (!L_EXTRACT_BIT_VAL (cb_indx->flags, L_CB_VISITED) &&
	(!seed || (cb_indx->weight > seed->weight)))
      seed = cb_indx;

  return seed;
}


static int
LB_unvisited_cbs (L_Func * fn)
{
  L_Cb *cb_indx;

  for (cb_indx = fn->first_cb; cb_indx != NULL; cb_indx = cb_indx->next_cb)
    if (!L_EXTRACT_BIT_VAL (cb_indx->flags , L_CB_VISITED))
      return TRUE;

  return FALSE;
}


static L_Cb *
LB_best_cb_successor_of (L_Cb * current, int check_for_backedges)
{
  L_Flow *preferred_flow, *second_to_last_flow;
  double total_weight;

  if (!current->dest_flow || 
      L_EXTRACT_BIT_VAL (current->flags, L_CB_HAS_JRG))
    return NULL;

  preferred_flow = L_find_last_flow (current->dest_flow);
  second_to_last_flow = preferred_flow->prev_flow;

  total_weight = preferred_flow->weight;

  if (second_to_last_flow)
    {
      total_weight += second_to_last_flow->weight;

      if (preferred_flow->weight < second_to_last_flow->weight)
	preferred_flow = second_to_last_flow;
    }

  if ((total_weight == 0.0) ||
      ((preferred_flow->weight / total_weight) < LB_min_branch_ratio))
    return NULL;

  if (L_EXTRACT_BIT_VAL (preferred_flow->dst_cb->flags, L_CB_VISITED))
    return NULL;

  if (check_for_backedges &&
      L_in_cb_DOM_set (current, preferred_flow->dst_cb->id))
    return NULL;

  return preferred_flow->dst_cb;
}


static L_Cb *
LB_best_cb_predecessor_of (L_Cb * current, int check_for_backedges)
{
  L_Flow *flow_indx;
  L_Flow *preferred_flow;
  L_Flow *prev_cb_last_flow;
  L_Flow *prev_cb_second_to_last_flow;
  double max_flow_weight;

  preferred_flow = NULL;
  max_flow_weight = -100.0;
  for (flow_indx = current->src_flow; flow_indx != NULL;
       flow_indx = flow_indx->next_flow)
    {
      if (max_flow_weight >= flow_indx->weight)
	continue;

      if (check_for_backedges &&
	  L_in_cb_DOM_set (flow_indx->src_cb, current->id))
	{
	  preferred_flow = NULL;
	  max_flow_weight = flow_indx->weight;
	  continue;
	}

      if (L_EXTRACT_BIT_VAL(flow_indx->src_cb->flags,
			    (L_CB_VISITED | L_CB_HAS_JRG)))
	continue;

      prev_cb_last_flow =
	L_find_last_flow (flow_indx->src_cb->dest_flow);
      prev_cb_second_to_last_flow = prev_cb_last_flow->prev_flow;

      if (!prev_cb_second_to_last_flow)
	{
	  preferred_flow = flow_indx;
	  max_flow_weight = preferred_flow->weight;
	}
      else if (prev_cb_last_flow->dst_cb == current)
	{
	  if (prev_cb_last_flow->weight >
	      prev_cb_second_to_last_flow->weight)
	    {
	      if ((prev_cb_last_flow->weight /
		   (prev_cb_last_flow->weight +
		    prev_cb_second_to_last_flow->weight))
		  >= LB_min_branch_ratio)
		{
		  preferred_flow = flow_indx;
		  max_flow_weight = preferred_flow->weight;
		}
	    }
	}
      else
	{
	  if (prev_cb_last_flow->weight <
	      prev_cb_second_to_last_flow->weight)
	    {
	      if ((prev_cb_second_to_last_flow->weight /
		   (prev_cb_last_flow->weight +
		    prev_cb_second_to_last_flow->weight))
		  >= LB_min_branch_ratio)
		{
		  preferred_flow = flow_indx;
		  max_flow_weight = preferred_flow->weight;
		}
	    }
	}
    }

  if (!preferred_flow ||
      (preferred_flow->weight < LB_minimum_superblock_weight) ||
      (preferred_flow->weight / current->weight < LB_trace_min_cb_ratio))
    return NULL;

  return preferred_flow->src_cb;
}


static void
LB_make_super_hyper_block_traces (L_Func * fn, LB_TraceRegion_Header * header)
{
  L_Cb *cb_indx;
  LB_TraceRegion *tr;
  int trace_num;
  L_Attr *attr;

  for (cb_indx = fn->first_cb; cb_indx; cb_indx = cb_indx->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb_indx->flags, 
			     (L_CB_SUPERBLOCK | L_CB_HYPERBLOCK)))
	{
	  if (!(attr = L_find_attr (cb_indx->attr, "trace")))
	    {
	      tr = LB_create_traceregion (fn, header->next_id++, cb_indx,
					  NULL, L_TRACEREGION_TRACE);
	      tr->flow_graph = LB_finish_frp_graph (tr->flow_graph);
	      header->traceregions = List_insert_last (header->traceregions,
						       tr);
	    }
	  else
	    {
	      trace_num = attr->field[0]->value.i;
	      tr = LB_find_traceregion_of_number (header, trace_num);
	    }
	  cb_indx->flags = L_SET_BIT_FLAG (cb_indx->flags, L_CB_VISITED);

	  if (L_EXTRACT_BIT_VAL (cb_indx->flags, L_CB_SUPERBLOCK))
	    tr->flags |= L_TRACEREGION_SUPERBLOCK;
	  if (L_EXTRACT_BIT_VAL (cb_indx->flags, L_CB_HYPERBLOCK))
	    tr->flags |= L_TRACEREGION_HYPERBLOCK;
	}
    }
}


void
LB_make_traceregions (L_Func * fn, int check_for_backedges,
		      LB_TraceRegion_Header * header)
{
  L_Cb *seed, *current, *successor, *predecessor;
  LB_TraceRegion *current_tr;

  /* We want all superblocks and hyperblocks to be their own traces. */

  LB_make_super_hyper_block_traces (fn, header);

  /*
   * ** Algorithm adapted from "Trace Selection for Compiling Large C
   * ** Application Programs to Microcode" by Pohua P. Chang, Wen-mei W. Hwu
   */

  /*
   * ** Algorithm trace_selection()
   * **   mark all nodes unvisited;
   */
  /*
     ** We will mark the cb's as layed out using the VISITED flag
     ** the last cb will be marked visited so it remains in its own 
     ** trace.  Make new attribute called trace
   */
  LB_clear_cb_visit_flags (fn, header);

  /*
   * REH 7/2/95 - I do not want boundary condition blocks included
   * within traces at the moment til I better understand the 
   * region reintegration process.
   */
  LB_mark_boundary_blocks_visited (fn, header);

  /*
   * ** Create a trace with only the epilog cb in it to insure that it
   * ** will always remain in its own trace.  Mark it visited.
   */

  if ((current = LB_find_epilog_cb (fn)) &&
      !L_EXTRACT_BIT_VAL (current->flags, L_CB_VISITED))
    {
      current_tr = LB_create_traceregion (fn, header->next_id++,
					  current, NULL,
					  L_TRACEREGION_TRACE);
      current_tr->flags =
	L_SET_BIT_FLAG (current_tr->flags, L_CB_VISITED);
      current_tr->flow_graph =
	LB_finish_frp_graph (current_tr->flow_graph);
      header->traceregions =
	List_insert_last (header->traceregions, current_tr);
    }

  /*
   * ** while (there are unvisited nodes)
   */
  while (LB_unvisited_cbs (fn))
    {
      /* Select the unvisited node with highest execution count */

      seed = LB_find_cb_seed (fn);

      /* Start a new trace */

      current_tr = LB_create_traceregion (fn, header->next_id++, seed,
					  NULL, L_TRACEREGION_TRACE);

      seed->flags = L_SET_BIT_FLAG (seed->flags, L_CB_VISITED);

      /* Grow the trace forward */

      current = seed;

      while ((successor = LB_best_cb_successor_of (current,
						   check_for_backedges)))
	{
	  LB_add_cb_to_traceregion (current_tr, current, successor, END);
	  successor->flags = 
	    L_SET_BIT_FLAG (successor->flags, L_CB_VISITED);
	  current = successor;
	}

      /* Grow the trace backward */

      current = seed;

      while ((predecessor = LB_best_cb_predecessor_of (current, 
						       check_for_backedges)))
	{
	  LB_add_cb_to_traceregion (current_tr, current, predecessor, TOP);
	  predecessor->flags = 
	    L_SET_BIT_FLAG (predecessor->flags, L_CB_VISITED);
	  current = predecessor;
	}

      current_tr->flow_graph = LB_finish_frp_graph (current_tr->flow_graph);
      header->traceregions = List_insert_last (header->traceregions,
					       current_tr);

#ifdef DEBUG_MAKETR
      LB_print_tr (stderr, current_tr);
      Graph_daVinci (current_tr->flow_graph, "GRAPH", LB_bb_print_hook);
#endif
    }

  return;
}
