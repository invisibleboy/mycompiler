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
 *      File :          lb_hb_block_enum.c
 *      Description :   A new heuristic for selecting blocks for hyperblock
 *                       Inclusion.
 *      Creation Date : June 1999
 *      Authors :       Kevin Crozier
 *       And work adapted from Scott Mahlke's Lhyper
 *==========================================================================*/

/* This is new block selection heuristic designed to be more robust 
 * in selecting blocks with little or new execution weight but don't
 * harm the hyperblock.  Also designed to work with zero or low weight
 * functions using static prediction techiniques.  Some of the factors
 * to be taken into account include:
 *    o  Code size and tail duplication.  
 *         Avoid tail duplication unless really necessary.  Try and reduce
 *         code size if at all possible including predication zero-weight
 *         functions.
 *    o  Minimal increases in dependence height
 *    o  No oversubscription of resources
 *    o  Possibilty for merging after predication
 */
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"
#include "lb_b_internal.h"

#define ERR stderr

#undef DEBUG_SELECT
#undef DEBUG_DEPENDENCE

int LB_max_dependence_height = 0;
int LB_region_total_ops = 0;

static void
LB_find_total_num_ops (Set blocks)
{
  int i, num_cb, *buf = NULL;
  L_Cb *cb;
  L_Attr *attr;

  num_cb = Set_size (blocks);
  buf = (int *) Lcode_malloc (sizeof (int) * num_cb);
  if (buf == NULL)
    L_punt ("L_find_total_num_ops: malloc out of space");
  Set_2array (blocks, buf);

  LB_region_total_ops = 0;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
      attr = L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL);
      if (attr)
	LB_region_total_ops += (int)(L_cb_size (cb) * attr->field[0]->value.i);
      else
	LB_region_total_ops += L_cb_size (cb);
    }

  Lcode_free (buf);
}

static void
LB_find_tr_num_ops (LB_TraceRegion * tr)
{
  int num_ops;
  L_Cb *cb;
  L_Attr *attr;

  if (tr == NULL)
    L_punt ("LB_find_tr_num_ops: path is NULL");

  num_ops = 0;
  List_start (FlowGraphTopoList (tr));
  while ((cb = LB_next_cb_in_region (tr)))
    {
      attr = L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL);
      if (attr)
	num_ops += (L_cb_size (cb) * attr->field[0]->value.i);
      else
	num_ops += L_cb_size (cb);
    }

  tr->num_ops = num_ops;
}

/*****/
#if 0
static void
LB_find_tr_exec_ratio (LB_TraceRegion * tr)
{
  L_Cb *curr_cb, *next_cb, *head_cb, *dst_cb;
  L_Flow *flow;
  double exec_ratio, weight, weight_to_next;
  Set cur_path_blocks = NULL;

  if (tr == NULL)
    L_punt ("LB_find_tr_exec_ratio: path is NULL");

  head_cb = LB_first_cb_in_region (tr);
  exec_ratio = 1.0;
  weight = head_cb->weight;
  weight_to_next = 0.0;
  for (flow = head_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      /* need to work on this one */
      if (LB_return_cb_in_region (tr, flow->dst_cb->id))
	weight_to_next += flow->weight;
    }
  /*
     if (weight == 0.0)
     exec_ratio = 0.0;
   */
  if ((weight != 0.0) && (weight_to_next != 0.0))
    exec_ratio *= (weight_to_next / weight);

  curr_cb = head_cb;
  cur_path_blocks = Set_add (cur_path_blocks, head_cb->id);

  while ((curr_cb = LB_next_cb_in_region (tr)))
    {
      next_cb = LB_get_next_cb_in_region (tr);
      if (next_cb == NULL)
	break;
      weight = curr_cb->weight;
      weight_to_next = 0.0;
      for (flow = curr_cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;
	  if ((dst_cb == next_cb) || (dst_cb == curr_cb))
	    weight_to_next += flow->weight;
	  /* SAM 3-98 - semi hack to account for possible peeled loop exec ratio */
	  else if ((Set_in (cur_path_blocks, dst_cb->id)) &&
		   (L_find_attr (dst_cb->attr, LB_MARKED_FOR_LOOP_PEEL)))
	    weight_to_next += flow->weight;
	}
      /*
         if (weight == 0.0)
         exec_ratio = 0.0;
       */
      if ((weight != 0.0) && (weight_to_next != 0.0))
	exec_ratio *= (weight_to_next / weight);
      cur_path_blocks = Set_add (cur_path_blocks, curr_cb->id);
      curr_cb = next_cb;
    }

  tr->exec_ratio = exec_ratio;
  Set_dispose (cur_path_blocks);
}
#endif

/*
 *    Here we build a temporary cb to calculate the height
 */
static void
LB_find_tr_dep_height (LB_TraceRegion * tr)
{
  L_Cb *cb, *test_cb;		/* need to make and delete */
  L_Oper *oper, *new_oper;

  if (tr == NULL)
    L_punt ("LB_find_tr_dep_height: path is NULL");

  test_cb = L_create_cb (0.0);
  List_start (FlowGraphTopoList (tr));
    while ((cb = LB_next_cb_in_region (tr)))
    {
      /* omit jumps since they wont appear in hyperblock */
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_uncond_branch_opcode (oper))
	    continue;
	  new_oper = L_copy_operation (oper);
	  L_insert_oper_after (test_cb, test_cb->last_op, new_oper);
	}
    }

  tr->dep_height = LB_hb_calc_cb_dep_height (test_cb);

  L_delete_all_oper (test_cb->first_op, 1);
  test_cb->first_op = NULL;
  test_cb->last_op = NULL;

  if (tr->dep_height > LB_max_dependence_height)
    LB_max_dependence_height = tr->dep_height;

  L_delete_cb (NULL, test_cb);
}



static void
LB_find_tr_wcet (LB_TraceRegion * tr)
{
  L_Cb *cb, *test_cb;		/* need to make and delete */
  L_Oper *oper, *new_oper;

  if (tr == NULL)
    L_punt ("LB_find_tr_dep_height: path is NULL");

  test_cb = L_create_cb (0.0);
  List_start (FlowGraphTopoList (tr));
    while ((cb = LB_next_cb_in_region (tr)))
    {
      /* omit jumps since they wont appear in hyperblock */
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_uncond_branch_opcode (oper))
	    continue;
	  new_oper = L_copy_operation (oper);
	  L_insert_oper_after (test_cb, test_cb->last_op, new_oper);
	}
    }
  //fprintf(stderr,"Trace(%d){\n",tr->id);
  tr->wcet = calc_wcet(test_cb);
 // fprintf(stderr,"}\n");


  L_delete_all_oper (test_cb->first_op, 1);
  test_cb->first_op = NULL;
  test_cb->last_op = NULL;

  L_delete_cb (NULL, test_cb);
}



static void
LB_find_tr_flags (LB_TraceRegion * tr)
{
  int flags;
  L_Oper *oper;
  L_Cb *cb;

  if (tr == NULL)
    L_punt ("LB_find_tr_flags: path is NULL");

  flags = 0;

  List_start (FlowGraphTopoList (tr));
  while ((cb = LB_next_cb_in_region (tr)))
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_subroutine_call_opcode (oper))
	    {
	      flags = L_SET_BIT_FLAG (flags, L_TRACEREGION_FLAG_HAS_JSR);
	      if (!L_side_effect_free_sub_call (oper))
		flags =
		  L_SET_BIT_FLAG (flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR);
	    }
	  if (L_general_store_opcode (oper) && L_pointer_store (oper))
	    {
	      flags =
		L_SET_BIT_FLAG (flags, L_TRACEREGION_FLAG_HAS_POINTER_ST);
	    }
	}
    }

  tr->flags |= flags;

  return;
}


LB_TraceRegion *
LB_compute_traceregion_info (LB_TraceRegion * tr)
{

  LB_find_tr_num_ops (tr);

  /* look at exection frequency */
  /*  LB_find_tr_exec_ratio(tr); */
  /* look at tail dup'ing */
  LB_tail_duplication_codesize (L_fn, tr, LB_DUP_OUTSIDE_REGION);

  /* check out dependence height */
  LB_find_tr_dep_height (tr);


  /* Look for jsrs and stores */
  LB_find_tr_flags (tr);

  {
	  LB_find_tr_wcet(tr);

	//  if(tr->flow_graph){
		 // fprintf(stderr,"RESID\n");
		 // LB_summarize_tr(stderr,tr);
	  //tr->wcet2+=ILP_trace_wcet_analyize(tr->flow_graph);

	//  }
  }
  return tr;

}

static void
LB_set_cb_flags (LB_TraceRegion * tr, Set blocks, int type)
{
  L_Cb *cb;
  int *buf, set_size, i, flag = 0;

  set_size = Set_size (blocks);
  buf = Lcode_malloc (sizeof (int) * set_size);
  Set_2array (blocks, buf);

  if (type == L_TRACEREGION_LOOP)
    flag = L_CB_HYPERBLOCK_LOOP;
  else if (type == L_TRACEREGION_HAMMOCK)
    flag = L_CB_HYPERBLOCK_HAMMOCK;
  else
    L_punt ("Strange hyperblock cb type.\n");

  for (i = 0; i < set_size; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
      cb->flags = L_SET_BIT_FLAG (cb->flags, flag);
    }
  tr->flags = L_SET_BIT_FLAG (tr->flags, type);
}


static int
LB_compare_traceregion_info (LB_TraceRegion * selected,
			     LB_TraceRegion * possible)
{
  int slots_used, slots_avail;
  double tot_exec_ratio;

  slots_used = selected->num_ops;

  /* SAM 10-94 Added MAX expression */
  slots_avail =
    IMPACT_MAX ((int)
		(selected->dep_height * LB_hb_issue_width *
		 LB_hb_path_max_op_growth),
		(int) (selected->num_ops * LB_hb_path_max_op_growth));
  tot_exec_ratio = selected->exec_ratio;

#ifdef DEBUG_SELECT
  fprintf (ERR,
	   "Main path (slots used %d) (slots avail %d)(exec ratio %f)(dep height %d)(priority %f)\n",
	   slots_used, slots_avail, selected->exec_ratio,
	   selected->dep_height, selected->priority);
#endif

  if ((possible->num_ops + slots_used) > slots_avail)
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "> Path %d excluded (not enuf slots): %d\n",
	       possible->id, possible->num_ops);
#endif
      return FALSE;
    }

  /* dependence height constraint */
  if (possible->dep_height >
      (selected->dep_height * LB_hb_path_max_dep_growth))
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "> Path %d excluded (dep height): %d\n",
	       possible->id, possible->dep_height);
#endif
      return FALSE;
    }

  /* Never put in an unsafe jsr unless it was on the main path */
  if (L_EXTRACT_BIT_VAL (possible->flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR))
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "> Path %d excluded (unsafe jsr)\n", possible->id);
#endif
      return FALSE;
    }

  /* Skip all jsrs if LB_hb_exclude_all_jsrs is set */
  if ((LB_hb_exclude_all_jsrs) &&
      (L_EXTRACT_BIT_VAL (possible->flags, L_TRACEREGION_FLAG_HAS_JSR)))
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "> Path %d excluded (jsr)\n", possible->id);
#endif
      return FALSE;
    }

  /* Skip all pointer stores if LB_hb_exclude_all_pointer_stores is set */
  if ((LB_hb_exclude_all_pointer_stores) &&
      (L_EXTRACT_BIT_VAL
       (possible->flags, L_TRACEREGION_FLAG_HAS_POINTER_ST)))
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "> Path %d excluded (pointer_store)\n", possible->id);
#endif
      return FALSE;
    }

  /* Tail dup ops */
#ifdef DEBUG_SELECT
  fprintf (ERR, "> Selected's tail dup %d, possibles tail dup %d\n",
	   selected->tail_dup, possible->tail_dup);
#endif
  if (possible->tail_dup > (selected->tail_dup * LB_tail_dup_growth))
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "> Path %d excluded too much tail-dup\n", possible->id);
#endif
      return FALSE;
    }
  return TRUE;
}


static Set
LB_static_main_path_selector (L_Cb * start_cb, L_Cb * end_cb, Set blocks)
{
  L_Cb *current_cb, *dst_cb;
  L_Flow *flow, *selected_flow;
  Set selected = NULL;
  int selected_size, current_size;

  /* Start with Wen-mei's idea and grab the largest blocks */
  for (current_cb = start_cb; current_cb != end_cb;
       current_cb = selected_flow->dst_cb)
    {
      selected = Set_add (selected, current_cb->id);
      selected_flow = NULL;
      selected_size = 0;
      for (flow = current_cb->dest_flow; flow; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;
	  if (Set_in (blocks, dst_cb->id))
	    {
	      if (Set_in (selected, dst_cb->id))
		continue;
	      if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HAS_JRG))
		continue;
	      if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK_LOOP) &&
		  !L_find_attr (dst_cb->attr, LB_MARKED_FOR_LOOP_PEEL))
		continue;

	      current_size = L_cb_size (dst_cb);

	      if (!selected_flow || (current_size > selected_size))
		{
		  selected_flow = flow;
		  selected_size = current_size;
		}
	    }
	}
      if (!selected_flow)
	return NULL;
    }
  return selected;
}


static Set
LB_dyn_main_path_selector (L_Cb * start_cb, L_Cb * end_cb, Set blocks)
{
  L_Cb *current_cb, *dst_cb;
  L_Flow *flow, *selected_flow;
  Set selected = NULL;

  for (current_cb = start_cb; current_cb != end_cb;
       current_cb = selected_flow->dst_cb)
    {
      selected = Set_add (selected, current_cb->id);

      /* Maybe a problem if the selected flow isn't in the region and the
         other doesn't override it -- should fix
       */
      selected_flow = NULL;
      for (flow = current_cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;

	  if (Set_in (blocks, dst_cb->id))
	    {
	      if (Set_in (selected, dst_cb->id))
		continue;
	      if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HAS_JRG))
		continue;
	      if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK_LOOP) &&
		  !L_find_attr (dst_cb->attr, LB_MARKED_FOR_LOOP_PEEL))
		continue;

	      if ((selected_flow == NULL) ||
		  (flow->weight > selected_flow->weight))
		selected_flow = flow;
	    }
	}
      if (selected_flow == NULL)
	return NULL;
    }

  selected = Set_add (selected, end_cb->id);
  /* compute slots, weight, dep_height, etc */

  return selected;
}


LB_TraceRegion *
LB_block_enumeration_selector (int type, L_Cb * start_cb, L_Cb * end_cb,
			       Set blocks, int id)
{

  L_Flow *flow;
  L_Cb *cb, *dest_cb, *dst_cb;
  LB_TraceRegion *new_tr, *temp_tr;
  Set selected_blocks, unselected_blocks, possible, new_region, look_at,
    looked_at;
  int *buf, set_size, i;

  selected_blocks = NULL;
  unselected_blocks = NULL;
  possible = NULL;
  new_region = NULL;
  look_at = NULL;
  looked_at = NULL;

#ifdef DEBUG_SELECT
  Set_print (ERR, "Incoming blocks: ", blocks);
#endif

  LB_find_total_num_ops (blocks);
  LB_max_dependence_height = 0;

  /* for zero weight regions do something special */
  if (start_cb->weight < LB_hb_min_cb_weight)
    selected_blocks = LB_static_main_path_selector (start_cb, end_cb, blocks);

  /* for a region with profile weights; find the main path */
  else
    selected_blocks = LB_dyn_main_path_selector (start_cb, end_cb, blocks);

  if (selected_blocks == NULL)
    return NULL;

#ifdef DEBUG_SELECT
  Set_print (ERR, "Main path selected blocks: ", selected_blocks);
#endif

  new_tr = LB_create_traceregion (L_fn, id++, start_cb, selected_blocks,
				  L_TRACEREGION_PRELIM);
  new_tr = LB_compute_traceregion_info (new_tr);

  /* do something with the unselected blocks */
  /* Subtract selected blocks out of the set */
  unselected_blocks = Set_subtract (blocks, selected_blocks);

#ifdef DEBUG_SELECT
  Set_print (ERR, "aftersubs blocks: ", blocks);
  Set_print (ERR, "aftersubs selectedblocks: ", selected_blocks);
  Set_print (ERR, "aftersubs unseletectedblocks: ", unselected_blocks);
#endif


  /* Starting with the first block in the region look at adding blocks
   *  to the region and determine goodness
   */
  cb = LB_first_cb_in_region (new_tr);
  while ((cb != NULL) && (!Set_empty (unselected_blocks)))
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "Looking at cb: %d\n", cb->id);
#endif
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;

	  if (!Set_in (unselected_blocks, dst_cb->id))
	    continue;
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HAS_JRG))
	    continue;
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK_LOOP) &&
	      !L_find_attr (dst_cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	    continue;
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK))
	    continue;
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_SUPERBLOCK))
	    continue;
	  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_RESERVED_TEMP8))
	    continue;

	  look_at = Set_add (look_at, dst_cb->id);
	}

      /* find a path of unselected blocks in the region and test them */
      /* Has our neighbor not been selected */

      /* Another way of doing this maybe to do depth first, add those
       *  blocks and see if it acceptable then do another depth first
       *  with the added blocks.
       */
      while (!Set_empty (look_at))
	{
	  set_size = Set_size (look_at);
	  buf = Lcode_malloc (sizeof (int) * set_size);
	  Set_2array (look_at, buf);

	  for (i = 0; i < set_size; i++)
	    {
	      dest_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
	      possible = Set_add (possible, dest_cb->id);

	      for (flow = dest_cb->dest_flow; flow != NULL;
		   flow = flow->next_flow)
		{
		  look_at = Set_delete (look_at, buf[i]);
		  dst_cb = flow->dst_cb;

		  if (Set_in (looked_at, dst_cb->id))
		    continue;
		  if (!Set_in (unselected_blocks, dst_cb->id))
		    continue;
		  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HAS_JRG))
		    continue;
		  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK_LOOP)
		      && !L_find_attr (dst_cb->attr,
				       LB_MARKED_FOR_LOOP_PEEL))
		    continue;
		  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_HYPERBLOCK))
		    continue;
		  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_SUPERBLOCK))
		    continue;
		  if (L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_RESERVED_TEMP8))
		    continue;

		  look_at = Set_add (look_at, dst_cb->id);
		  looked_at = Set_add (looked_at, buf[i]);
		}
	    }
	  Lcode_free (buf);
	}

      if (!Set_empty (possible))
	{
	  new_region = Set_union (possible, selected_blocks);
	  temp_tr = LB_create_traceregion (L_fn, id++, start_cb, new_region,
					   L_TRACEREGION_PRELIM);

	  /* compute info for new trace region so we can compare it */
	  temp_tr = LB_compute_traceregion_info (temp_tr);

	  /* compare info, if better or at least ok, temp_tr becomes 
	   *   the new region.
	   */
	  if (LB_compare_traceregion_info (new_tr, temp_tr))
	    {
#ifdef DEBUG_SELECT
	      printf ("Adding tr %d to existing tr %d.\n", temp_tr->id,
		      new_tr->id);
	      Graph_daVinci (temp_tr->flow_graph, "NEW", LB_bb_print_hook);
	      Graph_daVinci (new_tr->flow_graph, "OLD", LB_bb_print_hook);
#endif
	      LB_free_traceregion (new_tr);
	      new_tr = temp_tr;
	      selected_blocks = Set_dispose (selected_blocks);
	      selected_blocks = Set_copy (new_region);
	      unselected_blocks =
		Set_subtract (unselected_blocks, selected_blocks);
	    }
	  else
	    {
	      LB_free_traceregion (temp_tr);
#ifdef DEBUG_SELECT
	      printf ("Keeping existing tr %d.\n", new_tr->id);
	      Graph_daVinci (new_tr->flow_graph, "EXISTING",
			     LB_bb_print_hook);
#endif
	    }
	  new_region = Set_dispose (new_region);
	  possible = Set_dispose (possible);
	}

      cb = LB_next_cb_in_region (new_tr);
      looked_at = Set_dispose (looked_at);
    }

  /* do some stuff code growth and see if it works */
#ifdef DEBUG_SELECT
  fprintf (ERR, "region %d for tail-dup %d ops, %d ops -- %6.2f\n",
	   new_tr->id, new_tr->tail_dup, new_tr->num_ops,
	   (float) (new_tr->num_ops + new_tr->tail_dup) / new_tr->num_ops);
#endif
  if ((start_cb->weight < LB_hb_min_cb_weight) &&
      (((float) (new_tr->num_ops + new_tr->tail_dup) / new_tr->num_ops) >
       LB_max_static_tail_dup))
    {
#ifdef DEBUG_SELECT
      fprintf (ERR,
	       "Booting low weight region %d for tail-dup %d ops, %d ops\n",
	       new_tr->id, new_tr->tail_dup, new_tr->num_ops);
#endif
      LB_free_traceregion (new_tr);
      new_tr = NULL;
    }

  else if (((float) (new_tr->num_ops + new_tr->tail_dup) / new_tr->num_ops) >
	   LB_max_dyn_tail_dup)
    {
#ifdef DEBUG_SELECT
      fprintf (ERR, "Booting regular region %d for tail-dup %d ops, %d ops\n",
	       new_tr->id, new_tr->tail_dup, new_tr->num_ops);
#endif
      LB_free_traceregion (new_tr);
      new_tr = NULL;
    }

  if (new_tr)
    {
      LB_set_cb_flags (new_tr, selected_blocks, type);
#ifdef DEBUG_SELECT
      Set_print (ERR, "Final selected blocks: ", selected_blocks);
      fprintf (ERR, "\n");
#endif
    }
  selected_blocks = Set_dispose (selected_blocks);
  Set_dispose (unselected_blocks);
  Set_dispose (look_at);
  return new_tr;
}
