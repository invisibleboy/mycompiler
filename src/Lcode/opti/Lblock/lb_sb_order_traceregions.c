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
 *      File :          l_order_tracesregions.c
 *      Description :   Tools for trace formation
 *      Creation Date : September 1994
 *      Authors :       David August, Kevin Crozier
 *
 *      (C) Copyright 1997, David August, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_sb_superblock.h"

static LB_TraceRegion *
LB_find_prolog_traceregion (L_Func * fn, LB_TraceRegion_Header * header)
{
  int tr_num;

  tr_num = L_find_attr (fn->first_cb->attr, "trace")->field[0]->value.i;

  /* Search for prolog trace with trace number trace_num */
  return LB_find_traceregion_of_number (header, tr_num);
}

static LB_TraceRegion *
LB_find_traceregion_seed (L_Func * fn, LB_TraceRegion_Header * header)
{
  LB_TraceRegion *seed;
  LB_TraceRegion *tr;

  seed = NULL;
  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (!(tr->flags & L_TRACEREGION_VISITED))
	{
	  if (!seed)
	    {
	      seed = tr;
	    }
	  else
	    {
	      if (tr->weight > seed->weight)
		{
		  seed = tr;
		}
	    }
	}
    }
  return seed;
}

static LB_TraceRegion *
LB_jump_rg_most_taken_traceregion (L_Cb * cb, LB_TraceRegion_Header * header)
{
  L_Flow *flow_ptr;
  LB_TraceRegion *tr;
  LB_TraceRegion *max_tr;
  int tr_num;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    tr->weight_temp = 0.0;

  flow_ptr = L_find_last_flow (cb->dest_flow);
  for (; flow_ptr != NULL; flow_ptr = flow_ptr->prev_flow)
    {
      tr_num =
	L_find_attr (flow_ptr->dst_cb->attr, "trace")->field[0]->value.i;
      tr = LB_find_traceregion_of_number (header, tr_num);
      if (!(tr->flags & L_TRACEREGION_VISITED))
	tr->weight_temp += flow_ptr->dst_cb->weight;
    }

  max_tr = List_get_first (header->traceregions);
  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (max_tr->weight_temp < tr->weight_temp)
	max_tr = tr;
    }
  if (max_tr->flags & L_TRACEREGION_VISITED)
    return NULL;

  return max_tr;
}

static LB_TraceRegion *
LB_best_traceregion_successor_of (LB_TraceRegion * current,
				  LB_TraceRegion_Header * header)
{
  L_Flow *preferred_flow;
  L_Flow *second_to_last_flow;
  L_Cb *last_cb;
  LB_TraceRegion *preferred_tr;
  LB_TraceRegion *secondary_tr;
  int tr_num;

  Graph_dfs_topo_sort (current->flow_graph);
  last_cb = LB_last_cb_in_region (current);

  if (!last_cb->dest_flow)
    {
      return NULL;
    }
  if (last_cb->flags & L_CB_HAS_JRG)
    {
      return LB_jump_rg_most_taken_traceregion (last_cb, header);
    }
  preferred_flow = L_find_last_flow (last_cb->dest_flow);
  tr_num =
    L_find_attr (preferred_flow->dst_cb->attr, "trace")->field[0]->value.i;
  preferred_tr = LB_find_traceregion_of_number (header, tr_num);

  second_to_last_flow = preferred_flow->prev_flow;
  if (second_to_last_flow)
    {
      tr_num =
	L_find_attr (second_to_last_flow->dst_cb->attr,
		     "trace")->field[0]->value.i;
      secondary_tr = LB_find_traceregion_of_number (header, tr_num);
    }
  else
    {
      secondary_tr = NULL;
    }

  if (!preferred_tr || (preferred_tr->flags & L_TRACEREGION_VISITED) ||
      (LB_get_first_cb_in_region (preferred_tr) != preferred_flow->dst_cb) ||
      /* REH 7/19/95 - Boundary traces are never best successor */
      (L_EXTRACT_BIT_VAL
       (((L_Cb *) LB_get_first_cb_in_region (preferred_tr))->flags,
	L_CB_BOUNDARY)))
    {
      if (!secondary_tr || (secondary_tr->flags & L_TRACEREGION_VISITED) ||
	  (LB_get_first_cb_in_region (secondary_tr) !=
	   second_to_last_flow->dst_cb) ||
	  /* REH 7/19/95 - Boundary traces are never best successor */
	  (L_EXTRACT_BIT_VAL
	   (((L_Cb *) LB_get_first_cb_in_region (secondary_tr))->flags,
	    L_CB_BOUNDARY)))
	{
	  return NULL;
	}
      return secondary_tr;
    }
  else
    {
      if (!secondary_tr || (secondary_tr->flags & L_TRACEREGION_VISITED) ||
	  (LB_get_first_cb_in_region (secondary_tr) !=
	   second_to_last_flow->dst_cb) ||
	  /* REH 7/19/95 - Boundary traces are never best successor */
	  (L_EXTRACT_BIT_VAL
	   (((L_Cb *) LB_get_first_cb_in_region (secondary_tr))->flags,
	    L_CB_BOUNDARY)))
	{
	  return preferred_tr;
	}
      if (preferred_flow->weight < second_to_last_flow->weight)
	{
	  return secondary_tr;
	}
      return preferred_tr;
    }
}

static LB_TraceRegion *
LB_best_traceregion_predecessor_of (LB_TraceRegion * current,
				    LB_TraceRegion_Header * header)
{
  L_Flow *preferred_flow;
  LB_TraceRegion *preferred_tr;
  LB_TraceRegion *tr_indx;
  L_Flow *flow_indx;
  L_Cb *cb;
  int tr_num;

  cb = LB_get_first_cb_in_region (current);

  preferred_flow = cb->src_flow;
  preferred_tr = NULL;
  for (flow_indx = preferred_flow; flow_indx != NULL;
       flow_indx = flow_indx->next_flow)
    {
      tr_num =
	L_find_attr (flow_indx->src_cb->attr, "trace")->field[0]->value.i;
      tr_indx = LB_find_traceregion_of_number (header, tr_num);

      Graph_dfs_topo_sort (tr_indx->flow_graph);
      cb = LB_last_cb_in_region (tr_indx);
      if (cb == flow_indx->src_cb)
	{
	  if (!(tr_indx->flags & L_TRACEREGION_VISITED))
	    {
	      /* REH 7/19/95 - Boundary traces are never best predecessor */
	      if (!L_EXTRACT_BIT_VAL
		  ((LB_get_first_cb_in_region (tr_indx))->flags,
		   L_CB_BOUNDARY))
		{
		  if (!preferred_tr
		      || (preferred_flow->weight < flow_indx->weight))
		    {
		      preferred_flow = flow_indx;
		      preferred_tr = tr_indx;
		    }
		}
	    }
	}
    }
  return preferred_tr;
}

void
LB_order_traceregions (L_Func * fn, LB_TraceRegion_Header * header)
{
  LB_TraceRegion *seed;
  LB_TraceRegion *current;
  LB_TraceRegion *successor;
  LB_TraceRegion *predecessor;
  LB_TraceRegion *prev_bottom;
  LB_TraceRegion *current_bottom;
  List inorder_trace;		/* subtrace to be appended on to the main trace */
  int done;


  /*
   * ** Algorithm adapted from "Trace Selection for Compiling Large C
   * ** Application Programs to Microcode" by Pohua P. Chang, Wen-mei W. Hwu
   */

  /*
   * ** Algorithm supertrace_selection()
   * **   mark all nodes unvisited;
   * ** Additionally we mark all trace dest fields as NULL.
   * ** Both things are done since order traces may be called twice.
   */
  header->inorder_trs = List_reset (header->inorder_trs);
  LB_clear_traceregion_visit_flags (header);

  /*
   * ** The first seed must be the trace that starts the function.
   */
  inorder_trace = NULL;
  seed = LB_find_prolog_traceregion (fn, header);
  inorder_trace = List_insert_first (inorder_trace, seed);

  prev_bottom = NULL;

  /*
     ** while (there are unvisited nodes)
   */
  while (LB_unvisited_traceregions (header))
    {
      /*
         ** ** select a seed
         ** seed = the node with the largest execution count among all
         ** unvisited nodes;
       */
      if (seed == NULL)
	{
	  seed = LB_find_traceregion_seed (fn, header);
	  inorder_trace = List_insert_first (inorder_trace, seed);
	}
      /* 
         ** mark seed visited;
       */
      seed->flags |= L_TRACEREGION_VISITED;

      /*
         ** ** grow the trace forward ** 
         ** current = seed;
       */
      current = seed;

      /* 
         ** loop
         **   s = best_successor_of(current);
         **   if (s == 0) exit loop;
         **   add s to the trace;
         **   mark s visited;
         **   current = s;
       */
      done = FALSE;
      while (!done)
	{
	  successor = LB_best_traceregion_successor_of (current, header);
	  if (!successor)
	    {
	      done = TRUE;
	    }
	  else
	    {
	      inorder_trace = List_insert_last (inorder_trace, successor);
	      successor->flags |= L_TRACEREGION_VISITED;
	      current = successor;
	    }
	}

      current_bottom = current;

      /*
         ** ** grow the trace backward ** 
         ** current = seed;
       */
      current = seed;

      /* 
         ** loop
         **   s = best_predecessor_of(current);
         **   if (s == 0) exit loop;
         **   add s to the trace;
         **   mark s visited;
         **   current = s;
       */

      done = FALSE;
      while (!done)
	{
	  predecessor = LB_best_traceregion_predecessor_of (current, header);
	  if (!predecessor)
	    {
	      done = TRUE;
	    }
	  else
	    {
	      inorder_trace = List_insert_first (inorder_trace, predecessor);
	      predecessor->flags |= L_TRACEREGION_VISITED;
	      current = predecessor;
	    }
	}

      /* append subtrace to main function trace */
      header->inorder_trs = List_append (header->inorder_trs, inorder_trace);
      inorder_trace = NULL;	/* so we start with a new subtrace */

      /* Make seed NULL so we pick a new seed. */
      seed = NULL;
    }
}
