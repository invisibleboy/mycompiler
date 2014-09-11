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
 *      File :          l_superblock_former.c
 *      Description :   Superblock formation
 *      Author :        Kevin Crozier
 *                  Adpated from David August's l_superblock.c.
 *      (C) Copyright 1997, David August, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_sb_superblock.h"

static int
LB_traceregion_code_size (LB_TraceRegion * tr)
{
  L_Cb *cb;
  L_Oper *op;
  int code_size = 0;

  Graph_dfs_topo_sort (tr->flow_graph);	/* sort the graph nodes */
  List_start (tr->flow_graph->topo_list);
  while ((cb = LB_next_cb_in_region (tr)))
    {
      for (op = cb->first_op; op; op = op->next_op)
	if (op->opc != Lop_DEFINE)
	  code_size++;
    }

  return code_size;
}


void
LB_superblock_formation (L_Func * fn)
{
  LB_TraceRegion_Header *header;
  LB_TraceRegion *tr_index;
  Heap *tr_heap;
  int list_size, i, code_size = 0, max_code_size, growth, do_frp;

  if (fn->n_cb == 0)
    return;

  do_frp = !strcmp (LB_predicate_formation_type, "frp");

  header = LB_function_init (fn);

  L_partial_dead_code_removal (fn);
  L_do_flow_analysis (fn, DOMINATOR_CB|SUPPRESS_PG);
  L_loop_detection (fn, 0);

  L_compute_oper_weight (fn, 0, 1);
  LB_convert_to_strict_basic_block_code (fn,
		     L_CB_SUPERBLOCK | L_CB_HYPERBLOCK |
		     L_CB_ENTRANCE_BOUNDARY | L_CB_EXIT_BOUNDARY);

  L_do_flow_analysis (fn, DOMINATOR_CB);
  L_compute_oper_weight (fn, 0, 1);

  LB_mark_jrg_flag (fn);

  LB_make_traceregions (fn, TRUE, header);
  LB_order_traceregions (fn, header);
  LB_layout_function (fn, header);

  tr_heap = Heap_Create (HEAP_MAX);

  List_start (header->traceregions);
  while ((tr_index = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      code_size += LB_traceregion_code_size (tr_index);
      Graph_dfs_topo_sort (tr_index->flow_graph);
      if ((tr_index->weight >= LB_minimum_superblock_weight) &&
	  !(tr_index->flags & 
	    (L_TRACEREGION_HYPERBLOCK | L_TRACEREGION_SUPERBLOCK)) &&
	  (tr_index->header != LB_last_cb_in_region (tr_index)))
	Heap_Insert (tr_heap, tr_index, tr_index->weight);
    }

  max_code_size = (int) (LB_maximum_code_growth * code_size);

  LB_predicate_init ();

  while ((tr_heap->size > 0) && (code_size < max_code_size))
    {
      tr_index = (LB_TraceRegion *) Heap_ExtractTop (tr_heap);

      growth = LB_tail_duplication (fn, tr_index, LB_DUP_INSIDE_REGION, 1);

      if ((code_size + growth) > max_code_size)
	continue;

      code_size += LB_tail_duplication (fn, tr_index, LB_DUP_INSIDE_REGION, 0);

      LB_predicate_traceregion (fn, tr_index, do_frp);
      L_set_hb_no_fallthru_flag (fn);
      L_check_func (fn);

      tr_index->flags |= L_TRACEREGION_SUPERBLOCK;

      L_rebuild_src_flow (fn);
      LB_mark_jrg_flag (fn);

      /* Make traces with the new end cb's */
      list_size = List_size (header->traceregions);
      LB_make_traceregions (fn, FALSE, header);
      LB_order_traceregions (fn, header);
      LB_layout_function (fn, header);

      i = 0;
      while ((tr_index = (LB_TraceRegion *) List_next (header->traceregions)))
	{
	  if (++i <= list_size)
	    continue;

	  Graph_dfs_topo_sort (tr_index->flow_graph);

	  if ((tr_index->weight >= LB_minimum_superblock_weight) &&
	      !(tr_index->flags & (L_TRACEREGION_HYPERBLOCK |
				   L_TRACEREGION_SUPERBLOCK)) &&
	      (tr_index->header != LB_last_cb_in_region (tr_index)))
	    Heap_Insert (tr_heap, tr_index, tr_index->weight);
	}
    }

  tr_heap = Heap_Dispose (tr_heap, NULL);

  LB_predicate_deinit (fn);

  fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_SUPERBLOCK);
  L_rebuild_src_flow (fn);

  if (do_frp || LB_hb_do_combine_exits)
    {
      L_create_uncond_pred_defines (fn);
      L_combine_pred_defines (fn);
    }

  /* For each source code function we process deinit */
  LB_function_deinit (header);
  return;
}


void
LB_code_layout (L_Func * fn)
{
  LB_TraceRegion_Header *header;

  if (fn->n_cb == 0)
    return;

  header = LB_function_init (fn);

  L_compute_oper_weight (fn, 0, 1);
  L_do_flow_analysis (fn, DOMINATOR_CB);
  L_compute_oper_weight (fn, 0, 1);
  LB_mark_jrg_flag (fn);

  LB_make_traceregions (fn, TRUE, header);
  LB_order_traceregions (fn, header);
  LB_layout_function (fn, header);

  /* For each source code function we process deinit */
  LB_function_deinit (header);
}
