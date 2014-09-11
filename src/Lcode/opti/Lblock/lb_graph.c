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
 *      File :          l_lb_graph.c
 *      Description :   Tools for dealing with Lblock graphs
 *      Creation Date : September 1997
 *      Authors :       Kevin Crozier
 *
 *      (C) Copyright 1997, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_b_internal.h"

#undef DEBUG_GRAPH

char *LB_predicate_formation_type;

static L_Alloc_Pool *LB_BB_Pool = NULL;	/* Pool to allocate LB_BBs out of */

/*=========================================================================*/
/*
 *    Graph node creation/deletion routines
 */
/*=========================================================================*/

static LB_BB *
LB_new_bb (L_Cb * cb, GraphNode node, int type)
{
  LB_BB *bb;

  if (!LB_BB_Pool)
    LB_BB_Pool = L_create_alloc_pool ("LB_BB", sizeof (LB_BB), 128);

  bb = (LB_BB *) L_alloc (LB_BB_Pool);
  bb->type = type;
  bb->flag = 0;
  bb->cb = cb;
  bb->node = node;

  return bb;
}


static LB_BB *
LB_free_bb (LB_BB * bb)
{
  L_free (LB_BB_Pool, bb);

  return NULL;
}

/*=========================================================================*/
/*
 *    flow graph creation/deletion routines
 */
/*=========================================================================*/

static void
LB_form_region_graph (L_Func * fn, Graph graph, L_Cb *header, Set blocks)
{
  L_Cb *cb;
  L_Oper *branch;
  L_Flow *flow;
  GraphArc arc;
  GraphNode node, dest_node;
  LB_BB *bb;
  int *buf, *arc_state, num_blocks, i, flag, n;

  /* make a node for each cb in the region */
  num_blocks = Set_size (blocks);
  buf = (int *) alloca (sizeof (int) * num_blocks);
  Set_2array (blocks, buf);
  for (i = 0; i < num_blocks; i++)
    {
      if (buf[i] == header->id)
	continue;
      cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, buf[i]);
      node = Graph_create_graph_node (graph);
      bb = LB_new_bb (cb, node, CB);  /* create a new bb to go in graph node */
      GraphNodeContents (node) = bb;
      HashTable_insert ((HashTable) GraphContents (graph), cb->id, node);
    }

  /* connect the nodes together */
  for (i = 0; i < num_blocks; i++)
    {
      node =
	(GraphNode) HashTable_find ((HashTable) GraphContents (graph),
				    buf[i]);
      cb = GraphNodeCB (node);
      flag = 0;
      for (flow = cb->dest_flow; flow; flow = flow->next_flow)
	{
	  /* flow dst in the region we are predicating */
	  if (Set_in (blocks, flow->dst_cb->id) && (flow->dst_cb != header))
	    {
	      flag = 1;
	      dest_node =
		(GraphNode) HashTable_find ((HashTable) GraphContents (graph),
					    flow->dst_cb->id);

	      arc = Graph_connect_nodes (graph, node, dest_node);
	      arc->flow=flow;
	      branch = L_find_branch_for_flow (cb, flow);
	      arc_state = Lcode_malloc (sizeof (int));
	      *arc_state = (!branch || !L_cond_branch (branch)) ?
		FALLTHRU : TAKEN;

	      GraphArcContents (arc) = arc_state;
	    }
	  else if (!strcmp (LB_predicate_formation_type, "frp"))
	    {
	      ((LB_BB *) GraphNodeContents (node))->flag = CONNECT_TO_STOP;
	    }
	}

      if (!flag)
	((LB_BB *) GraphNodeContents (node))->flag = CONNECT_TO_STOP;
    }

  if ((n = Graph_delete_unreachable (graph,
				     (void (*)(void *))LB_free_bb, free)))
    {
      GraphNode node;

      L_warn ("LB_form_region_graph: Removing %d unreachable blocks from "
	      "region with header %d", n,
	      header->id);

      HashTable_reset ((HashTable) GraphContents (graph));

      List_start (graph->nodes);
      while ((node = (GraphNode) List_next (graph->nodes)))
	{
	  HashTable_insert ((HashTable) GraphContents (graph),
			    ((LB_BB *)GraphNodeContents(node))->cb->id, 
			    node);
	}
    }

  return;
}


Graph 
LB_create_flow_graph (L_Func * fn, L_Cb * initial_cb, Set blocks)
{
  LB_BB *bb;
  Graph new_graph;
  GraphNode new_node;

  new_graph = Graph_create_graph ();	/* create a new graph */
  GraphContents (new_graph) = HashTable_create (128);

  new_node = Graph_create_graph_node (new_graph);
  /* create new graph node */
  bb = LB_new_bb (initial_cb, new_node, CB);
  /* create a new bb to go in graph node */

  new_node->flags |= GRAPH_NODE_HEADER;

  GraphNodeContents (new_node) = bb;
  GraphRootNode (new_graph) = new_node;

  HashTable_insert ((HashTable) GraphContents (new_graph),
		    initial_cb->id, new_node);

  if (blocks)
    {
      LB_form_region_graph (fn, new_graph, initial_cb, blocks);
      LB_finish_frp_graph (new_graph);
    }

  return new_graph;
}

Graph 
LB_free_flow_graph (Graph graph)
{
  GraphNode node;
  GraphArc arc;
  GraphEquivCD equiv_cd;

  //fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM1.00\n");

  List_start (graph->nodes);
  while ((node = (GraphNode) List_next (graph->nodes)))
    LB_free_bb (GraphNodeContents (node));

  //fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM1.01\n");

  List_start (graph->equiv_cds);
  while ((equiv_cd = (GraphEquivCD) List_next (graph->equiv_cds)))
    if (GraphEquivCDContents (equiv_cd) != NULL)
      LB_free_predicate (GraphEquivCDContents (equiv_cd));

  //fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM1.02\n");

  List_start (graph->arcs);
  while ((arc = (GraphArc) List_next (graph->arcs)))
    if (GraphArcContents (arc) != NULL)
      Lcode_free (GraphArcContents (arc));

  //fprintf(stderr,"SAAAAAAAAAAALAAAAAAAAAAAAAAAM1.03\n");

  HashTable_free ((HashTable) GraphContents (graph));
  return Graph_free_graph (graph);
}

/*=========================================================================*/
/*
 *    flow graph addition routines
 */
/*=========================================================================*/

Graph
LB_add_cb_to_graph (Graph flow_graph, L_Cb * inserted, L_Cb * anchor, int dir)
{
  LB_BB *bb;
  L_Cb *dest_cb, *src_cb;
  L_Flow *flow;
  GraphArc arc;
  GraphNode node, dest_node, src_node;
  int *arc_state, num_flows;

  node = Graph_create_graph_node (flow_graph);
  bb = LB_new_bb (inserted, node, CB);
  GraphNodeContents (node) = bb;

  if (dir == END)
    {
      src_node =
	(GraphNode) HashTable_find ((HashTable) GraphContents (flow_graph),
				    anchor->id);
      dest_node = node;
      dest_cb = inserted;
      src_cb = anchor;
      bb = (LB_BB *) GraphNodeContents (src_node);
    }
  else
    {
      GraphRootNode (flow_graph) = node;
      dest_node =
	(GraphNode) HashTable_find ((HashTable) GraphContents (flow_graph),
				    anchor->id);
      src_node = node;
      src_cb = inserted;
      dest_cb = anchor;
    }

  num_flows = 0;
  for (flow = src_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      num_flows++;
      if (flow->dst_cb == dest_cb)
	{
	  arc = Graph_connect_nodes (flow_graph, src_node, dest_node);
	  arc_state = Lcode_malloc (sizeof (int));
	  *arc_state = FALLTHRU;
	  arc->ptr = arc_state;	/* This cant be GraphArcContents? */
	}

      /* FRP superblocks */

      if ((num_flows > 1) && !strcmp (LB_predicate_formation_type, "frp"))
	bb->flag = CONNECT_TO_STOP;
    }

  HashTable_insert ((HashTable) GraphContents (flow_graph), inserted->id,
		    node);

  return flow_graph;
}


Graph 
LB_finish_frp_graph (Graph flow_graph)
{
  LB_BB *bb;
  GraphNode stop_node, start_node, node;
  GraphArc arc;
  int connect, *arc_state;

  /* make a start node */
  start_node = Graph_create_graph_node (flow_graph);
  bb = LB_new_bb (NULL, start_node, START);
  start_node->flags |= GRAPH_NODE_START;
  GraphNodeContents (start_node) = bb;
  node = GraphRootNode (flow_graph);
  GraphRootNode (flow_graph) = start_node;
  arc = Graph_connect_nodes (flow_graph, start_node, node);
  arc_state = Lcode_malloc (sizeof (int));
  *arc_state = FALLTHRU;
  arc->ptr = arc_state;

  /* make stop node */
  stop_node = Graph_create_graph_node (flow_graph);
  stop_node->flags |= GRAPH_NODE_STOP;
  bb = LB_new_bb (NULL, stop_node, STOP);
  GraphNodeContents (stop_node) = bb;

  Graph_dfs_topo_sort (flow_graph);
  List_start (flow_graph->topo_list);
  while ((node = (GraphNode) List_next (flow_graph->topo_list)))
    {
      if ((node == start_node) || (node == stop_node))
	continue;

      bb = (LB_BB *) GraphNodeContents (node);

      connect = 1;

      if (node->succ && (bb->flag != CONNECT_TO_STOP))
	{
	  while ((arc = (GraphArc) List_next (node->succ)))
	    {
	      if (!Set_in (node->dom, arc->succ->id))
		{
		  connect = 0;
		  break;
		}
	    }
	}
      
      if (connect)
	{
	  arc = Graph_connect_nodes (flow_graph, node, stop_node);
	  arc_state = Lcode_malloc (sizeof (int));
	  /* not entirely right but it will do for the moment */

	  *arc_state = L_has_fallthru_to_next_cb (bb->cb) ?
	    TAKEN : FALLTHRU;

	  arc->ptr = arc_state;
	}
    }

  arc = Graph_connect_nodes (flow_graph, start_node, stop_node);
  arc_state = Lcode_malloc (sizeof (int));
  *arc_state = FALLTHRU;
  arc->ptr = arc_state;

#ifdef DEBUG_GRAPH
  Graph_daVinci (flow_graph, "GRAPHfg", LB_bb_print_hook);
#endif

  return flow_graph;
}

/*=========================================================================*/
/*
 *    flow graph print routine
 */
/*=========================================================================*/

void
LB_bb_print_hook (FILE * file, GraphNode node)
{
  LB_BB *bb;

  bb = (LB_BB *) GraphNodeContents (node);

  if (bb->type == CB)
    fprintf (file, "id %d: cb %d", node->id, bb->cb->id);
  else if (bb->type == STOP)
    fprintf (file, "id %d: STOP", node->id);
  else if (bb->type == START)
    fprintf (file, "id %d: START", node->id);
}
