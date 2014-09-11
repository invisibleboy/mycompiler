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
 *      File :          l_lb_traceregion.c
 *      Description :   Tools for dealing with Lblock traceregions
 *      Creation Date : November 1997
 *      Authors :       Kevin Crozier
 *
 *      (C) Copyright 1997, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_b_internal.h"
#undef DEBUG_TR
/* #define DEBUG_REMOVE */

#define ERR stderr

/*=========================================================================*/
/*
 *    Global vars for this file
 */
/*=========================================================================*/

static L_Alloc_Pool *LB_TraceRegion_Pool = NULL;
static L_Alloc_Pool *LB_TraceRegion_Header_Pool = NULL;

/*=========================================================================*/
/*
 *    LB_TraceRegion_Header creation/deletion routines
 */
/*=========================================================================*/

LB_TraceRegion_Header *
LB_create_tr_header (L_Func * fn)
{
  LB_TraceRegion_Header *new_tr_head;

  if (LB_TraceRegion_Header_Pool == NULL)
    LB_TraceRegion_Header_Pool = L_create_alloc_pool ("LB_TraceRegion_Header",
						      sizeof
						      (LB_TraceRegion_Header),
						      16);

  new_tr_head =
    (LB_TraceRegion_Header *) L_alloc (LB_TraceRegion_Header_Pool);
  new_tr_head->fn = fn;
  new_tr_head->next_id = 0;
  new_tr_head->traceregions = NULL;
  new_tr_head->inorder_trs = NULL;

  return new_tr_head;
}

void
LB_free_tr_header (LB_TraceRegion_Header * header)
{
  if (header->traceregions != NULL)
    LB_free_all_traceregions (header);

  L_free (LB_TraceRegion_Header_Pool, header);
}

/*=========================================================================*/
/*
 *    LB_TraceRegion creation/deletion routines
 */
/*=========================================================================*/

void
LB_update_traceregion (LB_TraceRegion *tr, 
		       L_Func *fn, L_Cb *hdr, Set new_tr_cbs)
{
  int type;

  if (tr->flow_graph)
    LB_free_flow_graph (tr->flow_graph);

  type = tr->flags = tr->flags & L_TRACEREGION_TYPE;
  tr->flow_graph = LB_create_flow_graph (fn, hdr, new_tr_cbs);

  tr->header = hdr;

  if (type != L_TRACEREGION_TRACE)
    {
      int num_blocks, *buf, i, flag;
      L_Cb *cb;

      num_blocks = Set_size (new_tr_cbs);
      buf = (int *) alloca (sizeof (int) * num_blocks);
      Set_2array (new_tr_cbs, buf);

      switch (type)
	{
	case L_TRACEREGION_LOOP:
	  flag = L_CB_HYPERBLOCK_LOOP;
	  break;
	case L_TRACEREGION_HAMMOCK:
	  flag = L_CB_HYPERBLOCK_HAMMOCK;
	  break;
	case L_TRACEREGION_PRELIM:
	  flag = L_CB_RESERVED_TEMP8;
	  break;
	default:
	  flag = L_CB_HYPERBLOCK;
	}

      /* mark all blocks as included in a hyperblock */
      for (i = 0; i < num_blocks; i++)
	{
	  cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, buf[i]);
	  cb->flags = L_SET_BIT_FLAG (cb->flags, flag);
	}
    }
  else
    {
      L_Attr *attr;

      if (!(attr = L_find_attr (hdr->attr, "trace")))
	{
	  attr = L_new_attr ("trace", 1);
	  hdr->attr = L_concat_attr (hdr->attr, attr);
	}

      L_set_int_attr_field (attr, 0, tr->id);
    }

  return;
}


LB_TraceRegion *
LB_create_traceregion (L_Func * fn, int id, L_Cb * initial_cb,
		       Set blocks, int type)
{
  LB_TraceRegion *tr;

  if (!LB_TraceRegion_Pool)
    LB_TraceRegion_Pool = L_create_alloc_pool ("LB_TraceRegion",
					       sizeof (LB_TraceRegion), 16);

  tr = (LB_TraceRegion *) L_alloc (LB_TraceRegion_Pool);
  tr->id = id;
  tr->flags = type;
  tr->weight = initial_cb->weight;
  tr->num_ops = 0;
  tr->tail_dup = 0;
  tr->slots_used = 0;
  tr->slots_avail = 0;
  tr->priority = 0.0;
  tr->dep_height = 0;
  tr->exec_ratio = 0.0;
  tr->flow_graph = NULL;

  LB_update_traceregion (tr, fn, initial_cb, blocks);

#ifdef DEBUG_TR
  Graph_daVinci (tr->flow_graph, "GRAPH17", LB_bb_print_hook);
  LB_print_tr (ERR, tr);
#endif
  return tr;
}


void
LB_free_traceregion (LB_TraceRegion * tr)
{

  LB_free_flow_graph (tr->flow_graph);
 // fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM1.1\n");
  L_free (LB_TraceRegion_Pool, tr);
}

void
LB_free_all_traceregions (LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  //fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM0\n");
 while ((tr = (LB_TraceRegion *) List_next (header->traceregions))){
 // fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM1\n");
   LB_free_traceregion (tr);
 }
 // fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM2\n");
  header->traceregions = List_reset (header->traceregions);
  header->inorder_trs = List_reset (header->inorder_trs);
}

/*=========================================================================*/
/*
 *    LB_TraceRegion CB insertion/deletion/return routines
 */
/*=========================================================================*/

void
LB_add_cb_to_traceregion (LB_TraceRegion * tr, L_Cb * current,
			  L_Cb * cb, int end)
{
  L_Attr *trace_num_attr;

  /* Add trace number attribute to predecessor. */
  trace_num_attr = L_new_attr ("trace", 1);
  L_set_int_attr_field (trace_num_attr, 0, tr->id);
  cb->attr = L_concat_attr (cb->attr, trace_num_attr);

  /* add current cb to top of trace-region graph */
  tr->flow_graph = LB_add_cb_to_graph (tr->flow_graph, cb, current, end);
  if (end == TOP)
    tr->header = cb;

  /* Correct trace fields */
  if (tr->weight < cb->weight)
    tr->weight = cb->weight;
}


/* Moves the list pointer */
L_Cb *
LB_first_cb_in_region (LB_TraceRegion * tr)
{
  GraphNode node;

  Graph_dfs_topo_sort (tr->flow_graph);
  node = (GraphNode) List_first (tr->flow_graph->topo_list);
  if (!node)
    return NULL;

  if (((LB_BB *) GraphNodeContents (node))->type == START)
    node = (GraphNode) List_next (tr->flow_graph->topo_list);

  if (((LB_BB *) GraphNodeContents (node))->type == STOP)
    return NULL;
  else
    return ((LB_BB *) GraphNodeContents (node))->cb;
}

/* Moves the list pointer */
L_Cb *
LB_next_cb_in_region (LB_TraceRegion * tr)
{
  GraphNode node;

  node = (GraphNode) List_next (tr->flow_graph->topo_list);
  if (!node)
    return NULL;
  if (((LB_BB *) GraphNodeContents (node))->type == START)
    node = (GraphNode) List_next (tr->flow_graph->topo_list);

  if (((LB_BB *) GraphNodeContents (node))->type == STOP)
    return NULL;
  else
    return ((LB_BB *) GraphNodeContents (node))->cb;
}

GraphNode LB_next_graphnode (LB_TraceRegion * tr)
{
  GraphNode node;

  node = (GraphNode) List_next (tr->flow_graph->topo_list);
  if (!node)
    return NULL;
  if (((LB_BB *) GraphNodeContents (node))->type == START)
    node = (GraphNode) List_next (tr->flow_graph->topo_list);

  if (((LB_BB *) GraphNodeContents (node))->type == STOP)
    return NULL;
  else
    return node;
}

/* Doesn't modify the list pointer */
L_Cb *
LB_get_first_cb_in_region (LB_TraceRegion * tr)
{
  return (tr->header);
}

/* Doesn't modify the list pointer */
L_Cb *
LB_get_next_cb_in_region (LB_TraceRegion * tr)
{
  GraphNode node;

  node = (GraphNode) List_get_next (tr->flow_graph->topo_list);
  if (!node)
    return NULL;
  if (((LB_BB *) GraphNodeContents (node))->type == STOP)
    return NULL;
  else
    return ((LB_BB *) GraphNodeContents (node))->cb;
}

/* Moves the list pointer */
L_Cb *
LB_last_cb_in_region (LB_TraceRegion * tr)
{
  GraphNode node;

  if (!tr->flow_graph->flags & GRAPHTOPOSORT)
    L_punt ("L_last_cb_in_region: requires topo list");

  node = (GraphNode) List_last (tr->flow_graph->topo_list);
  if (!node)
    return NULL;
  if (((LB_BB *) GraphNodeContents (node))->type == STOP)
    node = (GraphNode) List_prev (tr->flow_graph->topo_list);

  return ((LB_BB *) GraphNodeContents (node))->cb;
}

L_Cb *
LB_return_cb_in_region (LB_TraceRegion * tr, int cb_id)
{
  GraphNode node;

  node =
    (GraphNode) HashTable_find_or_null ((HashTable)
					GraphContents (tr->flow_graph),
					cb_id);
  if (node == NULL)
    return NULL;
  else
    return ((LB_BB *) GraphNodeContents (node))->cb;
}

Set LB_return_cbs_region_as_set (LB_TraceRegion * tr)
{
  L_Cb *cb;
  Set cb_set;

  cb_set = NULL;

  List_start (tr->flow_graph->topo_list);
  while ((cb = LB_next_cb_in_region (tr)))
    cb_set = Set_add (cb_set, cb->id);

  return cb_set;
}

LB_TraceRegion *
LB_concat_seq_trs (LB_TraceRegion_Header *hdr,
		   LB_TraceRegion *tr1, LB_TraceRegion *tr2)
{
  LB_TraceRegion *newregion;
  Set r1_blocks, r2_blocks, merge_blocks;
	      
  /* Existing traceregions located -- just glue them together! */

  r1_blocks = LB_return_cbs_region_as_set (tr1);
  r2_blocks = LB_return_cbs_region_as_set (tr2);
  merge_blocks = Set_union (r1_blocks, r2_blocks);
  Set_dispose (r1_blocks);
  Set_dispose (r2_blocks);

  newregion = LB_create_traceregion (hdr->fn, hdr->next_id++,
				     tr1->header,
				     merge_blocks,
				     tr1->flags & L_TRACEREGION_TYPE);

  Set_dispose (merge_blocks);

  newregion->num_ops = tr1->num_ops + tr2->num_ops;
  newregion->slots_used = tr1->slots_used + tr2->slots_used;
  newregion->slots_avail = tr1->slots_avail + tr2->slots_avail;
  newregion->dep_height = tr1->dep_height + tr2->dep_height;
  newregion->exec_ratio = tr1->exec_ratio * tr2->exec_ratio;

  return newregion;
}

/*=========================================================================*/
/*
 *    LB_TraceRegion flag routines
 */
/*=========================================================================*/
void
LB_traceregion_set_fallthru_flag (LB_TraceRegion * tr)
{
  L_Cb *cb;

  /* Uses graph toposort */

  cb = LB_last_cb_in_region (tr);
  if (L_has_fallthru_to_next_cb (cb))
    tr->flags = L_SET_BIT_FLAG (tr->flags, L_TRACEREGION_FALLTHRU);
  else
    tr->flags = L_SET_BIT_FLAG (tr->flags, L_TRACEREGION_NOFALLTHRU);
}

void
LB_clear_traceregion_visit_flags (LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  /* Clear all the visited flags for the cb */
  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    tr->flags = L_CLR_BIT_FLAG (tr->flags, L_TRACEREGION_VISITED);
}

int
LB_unvisited_traceregions (LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (!(tr->flags & L_TRACEREGION_VISITED))
	return TRUE;
    }
  return FALSE;
}

/*=========================================================================*/
/*
 *    LB_TraceRegion search routines
 */
/*=========================================================================*/

LB_TraceRegion *
LB_find_traceregion_by_header (LB_TraceRegion_Header * header, L_Cb * cb)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (tr->header == cb)
	return tr;
    }
  return NULL;
}

LB_TraceRegion *
LB_find_traceregion_by_cb (LB_TraceRegion_Header * header, L_Cb * cb)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (LB_return_cb_in_region (tr, cb->id))
	return tr;
    }
  return NULL;
}

LB_TraceRegion *
LB_find_traceregion_of_number (LB_TraceRegion_Header * header, int tr_num)
{
  LB_TraceRegion *tr;

  /* Search for trace with trace number trace_num */
  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (tr->id == tr_num)
	return tr;
    }
  return NULL;
}

/*=========================================================================*/
/*
 *    LB_TraceRegion subsume routines
 */
/*=========================================================================*/

/* tr2 is a subset of tr1 */

static int
LB_tr_is_subset (LB_TraceRegion * tr1, LB_TraceRegion * tr2)
{
  L_Cb *cb;

  List_start (tr2->flow_graph->topo_list);
  while ((cb = LB_next_cb_in_region (tr2)))
    if (!(HashTable_find_or_null (GraphContents (tr1->flow_graph), cb->id)))
      return 0;

  return 1;
}


static int
LB_traceregions_intersection_empty (LB_TraceRegion * tr1,
				    LB_TraceRegion * tr2)
{
  L_Cb *cb;

  List_start (tr2->flow_graph->topo_list);
  while ((cb = LB_next_cb_in_region (tr2)))
    {
      if (HashTable_find_or_null (GraphContents (tr1->flow_graph), cb->id))
	return 0;
    }
  return 1;
}


int
LB_traceregion_is_subsumed (LB_TraceRegion * tr,
			    LB_TraceRegion_Header * header)
{
  LB_TraceRegion *index;
  int list_level;

  if (!header->traceregions)
    return 0;

  list_level = List_register_new_ptr (header->traceregions);
  List_start_l (header->traceregions, list_level);
  while ((index = (LB_TraceRegion *) 
	  List_next_l (header->traceregions, list_level)))
    {
      if (index == tr)
	continue;
      if (LB_tr_is_subset (index, tr))
	{
#ifdef DEBUG_REMOVE
	  fprintf (stderr, "TR %d SUBSUMES %d\n", index->id, tr->id);
#endif
	  return 1;
	}
      if (index->header == tr->header)
	{
	  if ((index->slots_used > tr->slots_used) ||
	      ((index->slots_used == tr->slots_used) &&
	       (index->id > tr->id)))
	    {
#ifdef DEBUG_REMOVE
	      fprintf (stderr, "TR %d SUBSUMES %d (HEAD)\n", index->id, tr->id);
#endif
	      return 1;
	    }
	}
    }
  return 0;
}


static LB_TraceRegion *
LB_traceregion_is_partially_subsumed (LB_TraceRegion * tr,
				      LB_TraceRegion_Header * header)
{
  LB_TraceRegion *index;
  int list_level;

  if (!header->traceregions)
    return NULL;

  list_level = List_register_new_ptr (header->traceregions);
  List_start_l (header->traceregions, list_level);
  while ((index = (LB_TraceRegion *) 
	  List_next_l (header->traceregions, list_level)))
    {
      if (index == tr)
	continue;

      if (LB_traceregions_intersection_empty (tr, index))
	continue;

      if (LB_tr_is_subset (index, tr))
	continue;

      if (LB_return_cb_in_region (index, tr->header->id) &&
	  (!LB_return_cb_in_region (tr, index->header->id) ||
	   (Graph_size (tr->flow_graph) <
	    Graph_size (index->flow_graph))))
	return index;
    }
  return NULL;
}


void
LB_remove_subsumed_traceregions (LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (LB_traceregion_is_subsumed (tr, header))
	{
#ifdef DEBUG_REMOVE
	  fprintf (stderr, "REMOVING SUBSUMED TRACEREGION:\n");
	  LB_summarize_tr (ERR, tr);
#endif
	  header->traceregions = List_remove (header->traceregions, tr);
	  LB_free_traceregion (tr);
	}
    }
}


void
LB_remove_partially_subsumed_hammock_traceregions (LB_TraceRegion_Header *
						   header)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (!(tr->flags & L_TRACEREGION_HAMMOCK))
	continue;
      if (LB_traceregion_is_partially_subsumed (tr, header))
	{
#ifdef DEBUG_REMOVE
	  fprintf (stderr, "REMOVING PARTIALLY SUBSUMED HAMMOCK TRACEREGION:\n");
	  LB_summarize_tr (ERR, tr);
#endif
	  header->traceregions = List_remove (header->traceregions, tr);
	  LB_free_traceregion (tr);
	}
    }
}


void
LB_remove_partially_subsumed_traceregions (LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (LB_traceregion_is_partially_subsumed (tr, header))
	{
#ifdef DEBUG_REMOVE
	  fprintf (stderr, "REMOVING PARTIALLY SUBSUMED TRACEREGION:\n");
	  LB_summarize_tr (ERR, tr);
#endif
	  header->traceregions = List_remove (header->traceregions, tr);
	  LB_free_traceregion (tr);
	}
    }
}


void
LB_remove_conflicting_traceregions (LB_TraceRegion_Header * header)
{
  int level1, level2;
  LB_TraceRegion *trA, *trB;

  if (!header->traceregions)
    return;

  level1 = List_register_new_ptr (header->traceregions);
  level2 = List_register_new_ptr (header->traceregions);

  List_start_l (header->traceregions, level1);

  while ((trA = (LB_TraceRegion *) 
	  List_next_l (header->traceregions, level1)))
    {
      List_copy_current_ptr (header->traceregions, level2, level1);

      while ((trB = (LB_TraceRegion *) 
	      List_next_l (header->traceregions, level2)))
	{
	  LB_TraceRegion *remove;

	  if (LB_traceregions_intersection_empty (trA, trB))
	    continue;	

	  /* One of the regions must be removed. */

	  if (LB_return_cb_in_region (trA, trB->header->id) &&
	      (!LB_return_cb_in_region (trB, trA->header->id) ||
	       (Graph_size (trA->flow_graph) < 
		Graph_size (trB->flow_graph))))
	    remove = trA;
	  else
	    remove = trB;

#ifdef DEBUG_REMOVE
	  fprintf (stderr, "REMOVING CONFLICTING TRACEREGION:\n");
	  LB_summarize_tr (stderr, remove);
#endif

	  LB_free_traceregion (remove);

	  if (remove == trA)
	    {
	      header->traceregions = 
		List_delete_current_l (header->traceregions, level1);
	      break;
	    }
	  else
	    {
	      header->traceregions = 
		List_delete_current_l (header->traceregions, level2);
	      continue;
	    }
	}
    }
}


/*=========================================================================*/
/*
 *    LB_TraceRegion print routines
 */
/*=========================================================================*/

void
LB_print_traceregions (FILE *fp, LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    LB_print_tr (fp, tr);
}

void
LB_summarize_traceregions (FILE *fp, LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;
  if(header->traceregions)
  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    LB_summarize_tr (fp, tr);

}

void
LB_print_inorder_trs (FILE *fp, LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;

  List_start (header->inorder_trs);
  while ((tr = (LB_TraceRegion *) List_next (header->inorder_trs)))
    LB_print_tr (fp, tr);
}

void
LB_print_tr_by_num (FILE *fp, LB_TraceRegion_Header * header, int num)
{
  LB_print_tr (fp, LB_find_traceregion_of_number (header, num));
}

void
LB_print_tr (FILE *fp, LB_TraceRegion * tr)
{
  GraphNode node;
  GraphEquivCD cd;
  GraphArc arc;
  int *buf, size, i;

  fprintf (fp, "Trace id: %d, Flags 0x%08X, Weight %f, Cbs Included: \n",
	   tr->id, tr->flags, tr->weight);
  Graph_dominator (FlowGraph (tr));
  Graph_post_dominator (FlowGraph (tr));
  Graph_imm_post_dominator (FlowGraph (tr));

  /* control dependencies */
  Graph_control_dependence (FlowGraph (tr));

  /* equiv cd determination */
  Graph_equiv_cd (FlowGraph (tr));

  Graph_dfs_topo_sort (tr->flow_graph);
  List_start (tr->flow_graph->topo_list);
  while ((node = LB_next_graphnode (tr)))
    {
      fprintf (fp, "Node %d:  Cb %d\n", node->id,
	       ((LB_BB *) GraphNodeContents (node))->cb->id);
      Set_print (fp, "Node CD set: ", node->contr_dep);
      size = Set_size (node->contr_dep);
      if (size > 0)
	{
	  buf = (int *) Lcode_malloc (sizeof (int) * size);
	  Set_2array (node->contr_dep, buf);
	  for (i = 0; i < size; i++)
	    {
	      arc = Graph_arc_from_id (tr->flow_graph, buf[i]);
	      fprintf (fp, "     Arc %d flag %d: Src %d -> Dest %d\n", buf[i],
		       *((int *) (arc->ptr)), arc->pred->id, arc->succ->id);
	    }
	  Lcode_free (buf);
	}
      Set_print (fp, "Node Dom set: ", node->dom);
      Set_print (fp, "Node Post Dom set: ", node->post_dom);
    }
  fprintf (fp, "Control Dependence info:\n");
  List_start (tr->flow_graph->equiv_cds);
  while ((cd = (GraphEquivCD) List_next (tr->flow_graph->equiv_cds)))
    {
      Set_print (fp, "CD set: ", cd->contr_dep);
      List_start (cd->nodes);
      while ((node = (GraphNode) List_next (cd->nodes)))
	fprintf (fp, "  %d\n", node->id);
    }
  fprintf (fp, "\n");
#ifdef DEBUG_TR
  Graph_daVinci (tr->flow_graph, "GRAPH", LB_bb_print_hook);
#endif
}


static char LB_traceregion_flag_name[32][8] =
{ "VIS",    "X01X",   "X02X",   "X03X",
  "HB",     "SB",     "X06X",   "X07X",
  "X08X",   "X09X",   "X10X",   "X11X",
  "INS",    "X13X",   "X14X",   "X15X",
  "LOOP",   "HAM",    "GEN",    "TR", 
  "NOFT",   "FT",     "PREL",   "X23X",
  "NSTCY",  "UNSJSR", "JSR",    "PST",
  "X28X",   "X29X",   "X20X",   "X31X" };

void
LB_summarize_tr (FILE *fp, LB_TraceRegion * tr)
{
  int cnt, i;
  Set cbs;

  fprintf (fp, "(TRACE %d (", tr->id);

  cnt = 0;
  for (i = 0; i < 32; i++)
    if (tr->flags & (0x1 << i))
      fprintf (fp, (cnt++ ? " %s" : "%s"), LB_traceregion_flag_name[i]);

  fprintf (fp, ")\n");

  if (tr->header)
    fprintf (fp, "\thdr:      (cb %d)\n", tr->header->id);

  fprintf (fp, "\tweight:   %11.4e\n", tr->weight);

  fprintf (fp, "\tops:      %6d\n", tr->num_ops);

  fprintf (fp, "\tslots:    %6d/%d\n", tr->slots_used, tr->slots_avail);

  fprintf (fp, "\tdep_ht:   %6d\n", tr->dep_height);

  fprintf (fp, "\tex_ratio: %11.4f\n", tr->exec_ratio);

  fprintf (fp, "\tpriority: %11.4f\n", tr->priority);

  fprintf (fp, "\ttail over head: %d\n", tr->tail_dup);

  fprintf (fp, "\tWCET: %d\n", tr->wcet);

  fprintf (fp, "\tWCET2: %d\n", tr->wcet2);


  fprintf (fp, "\t");

  cbs = LB_return_cbs_region_as_set (tr);

  Set_print (fp, "CB", cbs);

  Set_dispose (cbs);

  fprintf (fp, ")\n");
  return;
}
