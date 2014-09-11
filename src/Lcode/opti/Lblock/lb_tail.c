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
 *	File :		l_tail.c
 *	Description :	Tail duplication for hyperblock formation
 *	Creation Date :	September 1993
 *	Authors : 	Scott Mahlke, Wen-mei Hwu
 *      ---------------------------------------------------------------------
 *	Rewritten :	October 2000
 *	Authors : 	John Sias, Wen-mei Hwu
 *      Purpose:        Perform correct tail duplication for unstructured
 *                      loops, not detected by Lcode loop detection.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_b_internal.h"

static Set  LB_duplicate, LB_side_entry;
static List LB_side_in_flows, LB_change_flows;
static HashTable LB_dup_trans = NULL;

#undef DEBUG_TAIL

static void
LB_find_duplicate_blocks (L_Cb * cb, LB_TraceRegion * tr)
{
  L_Flow *flow;
  L_Cb *dst_cb;

  if (Set_in (LB_duplicate, cb->id))
    return;

  LB_duplicate = Set_add (LB_duplicate, cb->id);

  for (flow = cb->dest_flow; flow; flow = flow->next_flow)
    {
      dst_cb = flow->dst_cb;
      if ((LB_return_cb_in_region (tr, dst_cb->id)) &&
	  (dst_cb != LB_get_first_cb_in_region (tr)))
	LB_find_duplicate_blocks (dst_cb, tr);
    }

  return;
}

static Set LB_hb_visiting, LB_hb_visited;

static int
LB_hb_find_loops_for_dup_rcr (LB_TraceRegion *tr, L_Cb *cb)
{
  L_Flow *src_flow, *dst_flow;
  L_Cb *hdr_cb = tr->header, *dst_cb;
  int found = 0;

  LB_hb_visiting = Set_add (LB_hb_visiting, cb->id);
  LB_hb_visited = Set_add (LB_hb_visited, cb->id);

  for (dst_flow = cb->dest_flow; dst_flow; dst_flow = dst_flow->next_flow)
    {
      dst_cb = dst_flow->dst_cb;

      if (dst_cb == hdr_cb)
	continue;

      if (!LB_return_cb_in_region (tr, dst_cb->id))
	continue;

      if (Set_in (LB_hb_visiting, dst_cb->id))
	{
	  /* An enclosed loop is detected */

	  src_flow = L_find_matching_flow (dst_flow->dst_cb->src_flow,
					   dst_flow);
	  LB_change_flows = List_insert_last (LB_change_flows,
					      src_flow);
	  LB_side_in_flows = List_insert_last (LB_side_in_flows,
					       src_flow);

	  if (!Set_in (LB_side_entry, dst_cb->id))
	    {
	      LB_side_entry = Set_add (LB_side_entry, dst_cb->id);
	      LB_find_duplicate_blocks (dst_cb, tr);
	      found = 1;
	    }
	}
      else if (!Set_in (LB_hb_visited, dst_cb->id))
	{
	  found |= LB_hb_find_loops_for_dup_rcr (tr, dst_cb);
	}
    }

  LB_hb_visiting = Set_delete (LB_hb_visiting, cb->id);
  return found;
}

static int
LB_hb_find_loops_for_dup (LB_TraceRegion *tr)
{
  int found;
  LB_hb_visiting = NULL;
  LB_hb_visited = NULL;

  found = LB_hb_find_loops_for_dup_rcr (tr, tr->header);

  if (Set_size (LB_hb_visiting))
    L_punt ("LB_hb_find_loops_for_dup is broken!");

  LB_hb_visited = Set_dispose (LB_hb_visited);
  LB_hb_visiting = Set_dispose (LB_hb_visiting);

  return found;
}

static HashTable LB_hb_dup_hash = NULL;

void
LB_scale_incl_dest_flows (L_Cb *cb, double ratio)
{
  L_Flow *flo, *fli;
  L_Cb *dcb;

  cb->weight = cb->weight * ratio;

#ifdef DEBUG_TAIL
  fprintf (stderr, "scaling weight on (cb %d) by %0.5f\n", cb->id, ratio);
#endif

  for (flo = cb->dest_flow; flo; flo = flo->next_flow)
    {
      dcb = flo->dst_cb;

      fli = L_find_matching_flow (dcb->src_flow, flo);

      flo->weight = flo->weight * ratio;
      fli->weight = fli->weight * ratio;
    }

  return;
}


/*
 * LB_adjust_copied_region_weight (LB_TraceRegion *tr, List inflows)
 * ----------------------------------------------------------------------
 * Adjust block and flow weights in a traceregion according to the
 * redirection of a set of side-in flows to copied control blocks.
 * Assumes that the list "inflows" is presented in a particular
 * topological order to simplify the problem.
 */
void
LB_adjust_copied_region_weight (LB_TraceRegion *tr, List inflows)
{
  L_Flow *flow, *new_flow;
  L_Cb *ocb, *ccb;

  List_start (inflows);
  List_start (tr->flow_graph->topo_list);
  ocb = LB_next_cb_in_region (tr);
  new_flow = (L_Flow *)List_next (inflows);
  while ((ocb = LB_next_cb_in_region (tr)))
    if (Set_in (LB_duplicate, ocb->id))
      {
	double orig_weight, new_weight, ratio;

	orig_weight = ocb->weight;
	new_weight = 0;

	ccb = HashTable_find (LB_hb_dup_hash, ocb->id);
	
	for (flow = ocb->src_flow; flow; flow = flow->next_flow)
	  {
	    if (flow == new_flow)
	      new_flow = (L_Flow *)List_next (inflows);
	    else
	      new_weight += flow->weight;
	  }

	ratio = (orig_weight >= 1.0) ? new_weight / orig_weight : 1.0;

	if (ratio > 1.0)
	  ratio = 1.0;
	
	LB_scale_incl_dest_flows (ocb, ratio);
	LB_scale_incl_dest_flows (ccb, 1.0 - ratio);
      }

  return;
}

/* LB_tail_duplication
 * ----------------------------------------------------------------------
 * Return the number of instructions duplicated
 */

int
LB_tail_duplication (L_Func * fn, LB_TraceRegion * tr, int flag,
		     int measure_only)
{
  int i, num_dup_cb, *buf;
  L_Flow *flow, *src_flow, *dst_flow, *ft_flow;
  L_Cb *cb, *new_cb = NULL, *src_cb, *dst_cb, *new_dst_cb;
  L_Oper *br, *new_op;
  L_Attr *attr;
  Set tr_cbs;

  int force_graph_rebuild = 0;
  int growth = 0;

  LB_change_flows = List_reset (LB_change_flows);
  LB_side_in_flows = List_reset (LB_side_in_flows);

  LB_hb_dup_hash = HashTable_create (64);

  /*
   * Find region side entry points requiring tail duplication
   * ----------------------------------------------------------------------
   */

  LB_duplicate = NULL;
  LB_side_entry = NULL;

  Graph_dfs_topo_sort (tr->flow_graph); /* graph may contain loops */
  List_start (tr->flow_graph->topo_list);
  LB_next_cb_in_region (tr);	               /* Skip region header cb */

  if (flag == LB_DUP_OUTSIDE_REGION)
    {
      while ((cb = LB_next_cb_in_region (tr)))
	for (flow = cb->src_flow; flow; flow = flow->next_flow)
	  if (!LB_return_cb_in_region (tr, flow->src_cb->id))
	    {
	      LB_change_flows = List_insert_last (LB_change_flows,
						  flow);
	      LB_side_in_flows = List_insert_last (LB_side_in_flows,
						   flow);
	      if (!Set_in (LB_side_entry, cb->id))
		{
		  LB_side_entry = Set_add (LB_side_entry, cb->id);
		  LB_find_duplicate_blocks (cb, tr);
		}
	    } 

      /* Run loop detection to find unstructured loops */

      if (LB_hb_find_loops_for_dup (tr))
	{
	  force_graph_rebuild = 1;

	  L_warn ("LB_hb_tail_duplication: Unstructured loop caused"
		  " tail dup in function %s.", fn->name);
	}
    }
  else if (flag == LB_DUP_INSIDE_REGION)
    {
      while ((cb = LB_next_cb_in_region (tr)))
	for (flow = cb->src_flow; flow; flow = flow->next_flow)
	  if (cb != L_fall_thru_path (flow->src_cb))
	    {
	      LB_change_flows = List_insert_last (LB_change_flows,
						  flow);
	      LB_side_in_flows = List_insert_last (LB_side_in_flows,
						   flow);
	      if (!Set_in (LB_side_entry, cb->id))
		{
		  LB_side_entry = Set_add (LB_side_entry, cb->id);
		  LB_find_duplicate_blocks (cb, tr);
		}
	    }
    }
  else
    {
      L_punt ("LB_tail_duplication: invalid flag");
    }

  num_dup_cb = Set_size (LB_duplicate);
  if (num_dup_cb == 0)
    {
      LB_duplicate = Set_dispose (LB_duplicate);
      LB_side_entry = Set_dispose (LB_side_entry);
      HashTable_free (LB_hb_dup_hash);
      LB_hb_dup_hash = NULL;
      return 0;
    }

#ifdef DEBUG_TAIL
  fprintf (stderr, "TAIL DUPLICATING TRACEREGION %d\n", tr->id);
  fprintf (stderr, "\tside entrance cbs: ");
  Set_print (stderr, "", LB_side_entry);
  fprintf (stderr, "\tduplicate cbs: ");
  Set_print (stderr, "", LB_duplicate);
#endif

  buf = (int *) Lcode_malloc (sizeof (int) * num_dup_cb);
  Set_2array (LB_duplicate, buf);

  /*
   * Duplicate code, creating dest flows on duplicated blocks
   * ----------------------------------------------------------------------
   */
  
  tr_cbs = LB_return_cbs_region_as_set (tr);

  for (i = 0; i < num_dup_cb; i++)
    {
      int orig_cb_id = buf[i];

      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, orig_cb_id);

      if (!measure_only)
	{
	  new_cb = L_create_cb (cb->weight);

	  if (LB_dup_trans)
	    HashTable_update (LB_dup_trans, orig_cb_id, new_cb);

	  L_insert_cb_after (fn, fn->last_cb, new_cb);

	  L_copy_block_contents (cb, new_cb);

	  attr =  L_new_attr("tail", 1);
	  L_set_int_attr_field(attr, 0, tr->header->id);

	  new_cb->attr = L_concat_attr (new_cb->attr, attr);
	  new_cb->dest_flow = L_concat_flow (new_cb->dest_flow,
					     L_copy_flow (cb->dest_flow));

	  L_change_src (new_cb->dest_flow, cb, new_cb);

	  HashTable_insert (LB_hb_dup_hash, cb->id, new_cb);
	}
	  
      if (!L_uncond_branch (cb->last_op))
	{
	  if (!measure_only)
	    {
	      ft_flow = L_find_last_flow (new_cb->dest_flow);
	      new_op = L_create_new_op (Lop_JUMP);
	      new_op->src[0] = L_new_cb_operand (ft_flow->dst_cb);
	      L_insert_oper_after (new_cb, new_cb->last_op, new_op);
	    }
	  growth++;
	}
      growth += L_cb_size (cb);
    }
  
  /*
   * Create source flows to complement dest flows emanating from 
   * duplicated blocks
   * ----------------------------------------------------------------------
   */

  if (!measure_only)
    {
      for (i = 0; i < num_dup_cb; i++)
	{
	  cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, buf[i]);
	  new_cb = HashTable_find (LB_hb_dup_hash, buf[i]);
	  
	  for (dst_flow = cb->dest_flow; dst_flow != NULL;
	       dst_flow = dst_flow->next_flow)
	    {
	      src_flow = L_copy_single_flow (dst_flow);

	      if (Set_in (LB_duplicate, dst_flow->dst_cb->id))
		{
		  new_dst_cb = HashTable_find (LB_hb_dup_hash,
					       dst_flow->dst_cb->id);
		  src_flow->dst_cb = dst_flow->dst_cb;
		  LB_change_flows = List_insert_last (LB_change_flows,
						      src_flow);
		}
	      else
		{
		  new_dst_cb = dst_flow->dst_cb;
		  src_flow->dst_cb = new_dst_cb;
		}
	      src_flow->src_cb = new_cb;
	      new_dst_cb->src_flow = L_concat_flow (new_dst_cb->src_flow,
						    src_flow);
	    }
	}

      List_start (LB_side_in_flows);
      while ((dst_flow = (L_Flow *) List_next (LB_side_in_flows)))
	{
	  cb = dst_flow->dst_cb;
	  new_cb = HashTable_find (LB_hb_dup_hash, cb->id);

	  src_flow = L_copy_single_flow (dst_flow);
	  src_flow->dst_cb = new_cb;
	  new_cb->src_flow = L_concat_flow (new_cb->src_flow,
					    src_flow);
	}

#ifdef DEBUG_TAIL
      if (List_size (LB_change_flows))
	{
	  int cnt, i;
	  cnt = fprintf (stderr, "\tcopying cbs: ");
	  for (i = 0; i < num_dup_cb; i++)
	    {
	      if (cnt > 60)
		cnt = fprintf (stderr, "\n\t\t");

	      cnt += fprintf (stderr, "(%d->%d) ", buf[i],
			      ((L_Cb *)HashTable_find (LB_hb_dup_hash, 
						       buf[i]))->id);
	    }
	  fprintf (stderr, "\n");
	  
	  cnt = fprintf (stderr, "\tchange flows: ");
	  List_start (LB_change_flows);
	  while ((src_flow = (L_Flow *) List_next (LB_change_flows)))
	    {
	      if (cnt > 60)
		cnt = fprintf (stderr, "\n\t\t");

	      cnt += fprintf (stderr, "(%d->%d) ", src_flow->src_cb->id,
			      src_flow->dst_cb->id);
	    }
	  fprintf (stderr, "\n");
	}
#endif

      /*
       * Change destinations of dest flows into duplicated blocks
       * -------------------------------------------------------------------
       */

      List_start (LB_change_flows);
      while ((src_flow = (L_Flow *) List_next (LB_change_flows)))
	{
	  L_Cb *new_dst_cb;

	  src_cb = src_flow->src_cb;
	  dst_cb = src_flow->dst_cb;
	  new_dst_cb = HashTable_find (LB_hb_dup_hash, dst_cb->id);
	  dst_flow = L_find_matching_flow (src_cb->dest_flow, src_flow);
	  dst_flow->dst_cb = new_dst_cb;
	  src_flow->dst_cb = new_dst_cb;

	  if ((br = L_find_branch_for_flow (src_cb, dst_flow)))
	    {
	      L_change_branch_dest (br, dst_cb, new_dst_cb);
#ifdef DEBUG_TAIL
	      fprintf (stderr, "\tchange br dest in cb %d\n", src_cb->id);
#endif
	    }
	  else
	    {
	      if (!L_has_fallthru_to_next_cb (src_cb))
		L_punt ("LB_hb_tail_duplication: fallthrough expected");
	      new_op = L_create_new_op (Lop_JUMP);
	      new_op->src[0] = L_new_cb_operand (new_dst_cb);
	      L_insert_oper_after (src_cb, src_cb->last_op, new_op);
	      growth++;
#ifdef DEBUG_TAIL
	      fprintf (stderr, "\tinsert jump into cb %d\n", src_cb->id);
#endif
	    }
	}

      /*
       * Adjust block / flow weights
       * ------------------------------------------------------------------
       */

      LB_adjust_copied_region_weight (tr, LB_side_in_flows);

      /*
       * Add jumps where needed
       * ----------------------------------------------------------------------
       */
      
      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	{
	  if (L_has_fallthru_to_next_cb (cb))
	    {
	      flow = L_find_last_flow (cb->dest_flow);
	      if (flow->dst_cb != cb->next_cb)
		{
		  L_warn ("LB_tail_duplication: inserting last-chance jump");
		  new_op = L_create_new_op (Lop_JUMP);
		  new_op->src[0] = L_new_cb_operand (flow->dst_cb);
		  L_insert_oper_after (cb, cb->last_op, new_op);
		  growth++;
		  flow->cc = 1;
		}
	    }
	}
    }

  /*
   * Clean up data structures
   * ----------------------------------------------------------------------
   */

  LB_duplicate = Set_dispose (LB_duplicate);
  LB_side_entry = Set_dispose (LB_side_entry);

  LB_change_flows = List_reset (LB_change_flows);
  LB_side_in_flows = List_reset (LB_side_in_flows);

  Lcode_free (buf);

  HashTable_free (LB_hb_dup_hash);
  LB_hb_dup_hash = NULL;

  if (!measure_only)
    {
      L_rebuild_src_flow (fn);
      
      if (force_graph_rebuild)
	{
	  Set tr_cbs = LB_return_cbs_region_as_set (tr);
	  LB_free_flow_graph (tr->flow_graph);
	  tr->flow_graph = LB_create_flow_graph (fn, tr->header, tr_cbs);
	  Set_dispose (tr_cbs);
	}
    }

  Set_dispose (tr_cbs);

  return growth;
}

static int
LB_tail_translate (L_Func *fn, LB_TraceRegion *tr)
{
  int *buf; 
  int i, num_cb, chg = 0;
  L_Cb *xlat_cb;
  Set tr_set = LB_return_cbs_region_as_set (tr);

  if (!tr_set)
    return 0;

  num_cb = Set_size (tr_set);

  buf = alloca (num_cb * sizeof (int));
  Set_2array(tr_set, buf);

  if ((xlat_cb = HashTable_find_or_null (LB_dup_trans, tr->header->id)))
    tr->header = xlat_cb;

  for (i = 0; i < num_cb; i++)
    {
      if ((xlat_cb = HashTable_find_or_null (LB_dup_trans, buf[i])))
	{
	  chg++;
	  tr_set = Set_delete (tr_set, buf[i]);
	  tr_set = Set_add (tr_set, xlat_cb->id);
	}
    }

  if (chg)
    {
      L_warn ("Translating TR %d due to tail duplication", tr->id);
      LB_update_traceregion (tr, fn, tr->header, tr_set);
    }

  Set_dispose (tr_set);

  return chg;
}

/* Wrapper function to tail duplicate all traceregions sent in by the header.
 */
int
LB_tail_duplicate (L_Func * fn, LB_TraceRegion_Header * header, int flag)
{
  int duplicated = FALSE;
  LB_TraceRegion *tr;

  LB_dup_trans = HashTable_create (64);

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (duplicated)
	LB_tail_translate (fn, tr);

      if (LB_tail_duplication (fn, tr, flag, 0))
	duplicated = TRUE;
    }

  HashTable_free (LB_dup_trans);
  LB_dup_trans = NULL;

  return duplicated;
}


void
LB_tail_duplication_codesize (L_Func * fn, LB_TraceRegion * tr, int flag)
{
  int i, num_dup_cb, codesize, *buf;
  Set LB_side_entry;
  L_Flow *flow;
  L_Cb *cb;

  LB_duplicate = NULL;
  LB_side_entry = NULL;

  /* find cb's with side entry points */
  Graph_dfs_topo_sort (tr->flow_graph);
  List_start (tr->flow_graph->topo_list);
  LB_next_cb_in_region (tr);	/* Skip first cb in region */
  while ((cb = LB_next_cb_in_region (tr)))
    {
      for (flow = cb->src_flow; flow != NULL; flow = flow->next_flow)
	{
	  if ((!LB_return_cb_in_region (tr, flow->src_cb->id)) ||
	      ((flag == LB_DUP_INSIDE_REGION) &&
	       (cb != L_fall_thru_path (flow->src_cb))))
	    {
	      LB_side_entry = Set_add (LB_side_entry, cb->id);
	      LB_find_duplicate_blocks (cb, tr);
	      break;
	    }
	}
    }

  num_dup_cb = Set_size (LB_duplicate);

  if (num_dup_cb == 0)
    {
      tr->tail_dup = 1;
    }
  else
    {
      buf = (int *) Lcode_malloc (sizeof (int) * num_dup_cb);
      Set_2array (LB_duplicate, buf);

      codesize = 0;
      for (i = 0; i < num_dup_cb; i++)
	{
	  cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, buf[i]);
	  codesize += L_cb_size (cb);
	}

      Lcode_free (buf);

      tr->tail_dup = codesize;
    }

  LB_duplicate = Set_dispose (LB_duplicate);
  LB_side_entry = Set_dispose (LB_side_entry);
}
