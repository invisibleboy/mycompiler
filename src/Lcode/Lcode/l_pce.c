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
 *      File:   l_pce.c
 *      Author: Shane Ryoo, Wen-mei Hwu
 *      Creation Date:  February 2003
 *      Note: At some point, we may want to generalize this minimum s-t cut
 *            algorithm and incorporate weight into the graph structure,
 *            if we can find uses for it other than speculative PRE.
\****************************************************************************/

#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_pce.h>

#define PCE_GRAPH_TBL_SIZE 128
#define PCE_GRAPH_HASH_TBL_MASK  (PCE_GRAPH_TBL_SIZE - 1)

#undef TRACK_GRAPH_MEMORIZATION
#undef DO_GRAPH_MEMORIZATION

#define CUT_SHORT_CIRCUIT

#undef DEBUG_PROFITABILITY_METRIC
#undef PCE_PROFITABILITY_METRIC
#define REG_PRESSURE_INC    .001

#define P3DE_PENALTY_METRIC
#undef DEBUG_PENALTY_METRIC
#define PENALTY_PRED_FACTOR 8
#define PENALTY_LOOP_FACTOR 8
#define PENALTY_LOAD_FACTOR 1

#undef SPLIT_EDGE_METRIC
#define SPLIT_EDGE_FACTOR 2

#define GLOBAL_UPDATE_HEURISTIC
#define GLOBAL_UPDATE_FACTOR 3

#undef DEBUG_PREFLOW_PUSH
#undef DEBUG_REV_PREFLOW_PUSH

#undef PRINT_SPRE_GRAPH
#undef DEBUG_SPRE_CONSTRUCT
#undef DEBUG_SPRE_MIN_CUT
#undef DEBUG_SPRE_PLACEMENT

#undef PRINT_P3DE_GRAPH
#undef DEBUG_P3DE_CONSTRUCT
#undef DEBUG_P3DE_MIN_CUT
#undef DEBUG_P3DE_PLACEMENT
#define P3DE_GENERAL_STORE_MOTION
#undef DEBUG_P3DE_GENERAL_STORE_MOTION
#undef PRINT_AVAIL_PRED_CLEARS

L_Alloc_Pool *L_PCE_BB_Pool = NULL;
L_Alloc_Pool *L_PCE_arc_Pool = NULL;
L_Alloc_Pool *L_PCE_graph_cut_Pool = NULL;
L_PCE_graph_cut **L_PCE_graph_cut_tbl = NULL;

void
L_do_SPRE_analysis (L_Func * fn, int mode, int ignore)
{
  int dead_code;

  L_start_time (&L_module_global_dataflow_time);

  if (!(mode & SUPPRESS_PG))
    D_setup_dataflow (fn, PF_ALL_OPERANDS);
  else
    D_setup_dataflow (fn, PF_ALL_OPERANDS | PF_SUPPRESS_PRED_GRAPH);

  D_setup_BB_lists (PF_default_flow);

  D_PRE_speculative_cut_analysis (PF_default_flow, mode, ignore);

  L_stop_time (&L_module_global_dataflow_time);

  dead_code = D_delete_DF_dead_code (fn);
}


/* Creates a new PCE BB from a graph node for PCE and the BB node */
static L_PCE_BB *
L_new_PCE_bb (GraphNode node, L_BB * bb)
{
  L_PCE_BB *pce_bb;

  if (L_PCE_BB_Pool == NULL)
    L_PCE_BB_Pool = L_create_alloc_pool ("L_PCE_BB", sizeof (L_PCE_BB), 128);

  pce_bb = (L_PCE_BB *) L_alloc (L_PCE_BB_Pool);
  pce_bb->node = node;
  pce_bb->index = 0;
  pce_bb->bb = bb;
  pce_bb->height = 0;
  pce_bb->excess = 0;
  pce_bb->pce_type = 0;
  pce_bb->reg_pressure_inc = 0.0;
  pce_bb->reaching_BB = NULL;
  pce_bb->reaching_assn = NULL;
  return pce_bb;
}


static L_PCE_BB *
L_delete_PCE_bb (L_PCE_BB * pce_bb)
{
  if (!pce_bb)
    return NULL;

  Set_dispose (pce_bb->reaching_BB);
  Set_dispose (pce_bb->reaching_assn);

  if (pce_bb->bb)
    {
      pce_bb->bb->ptr = NULL;
      pce_bb->bb->ptr_exit = NULL;
    }
  L_free (L_PCE_BB_Pool, pce_bb);

  return NULL;
}


static L_PCE_arc *
L_new_PCE_arc (GraphArc arc, L_BB_arc * bb_arc)
{
  L_PCE_arc *pce_arc;

  if (L_PCE_arc_Pool == NULL)
    L_PCE_arc_Pool = L_create_alloc_pool ("L_PCE_arc", sizeof (L_PCE_arc),
					  128);

  pce_arc = (L_PCE_arc *) L_alloc (L_PCE_arc_Pool);
  pce_arc->arc = arc;
  pce_arc->bb_arc = bb_arc;
  if (bb_arc)
    pce_arc->weight = bb_arc->weight;
  pce_arc->flow = 0;
  pce_arc->pce_type = 0;
  return pce_arc;
}


static L_PCE_arc *
L_delete_PCE_arc (L_PCE_arc * arc)
{
  if (!arc)
    return NULL;

  if (arc->bb_arc)
    arc->bb_arc->ptr = NULL;

  L_free (L_PCE_arc_Pool, arc);
  return NULL;
}


#if defined PRINT_SPRE_GRAPH || defined PRINT_P3DE_GRAPH
void
L_print_PCE_graph (L_PCE_graph * pce_graph)
{
  GraphNode src_node;
  GraphArc arc;
  L_BB *src_bb;
  L_BB_arc *bb_arc;
  L_PCE_BB *pce_bb;
  L_PCE_arc *pce_arc;
  int i, *reach_array, size;
  reach_array = calloc (pce_graph->n_vertices, sizeof (int));

  fprintf (stderr, "Printing PCE graph, ex/as %d.\n",
	   pce_graph->expression_index);
  fprintf (stderr, "Vertices:\t%d\n", pce_graph->n_vertices);
  fprintf (stderr, "Edges:\t%d\n", pce_graph->n_edges);

  List_start (pce_graph->graph->nodes);
  while ((src_node = (GraphNode) List_next (pce_graph->graph->nodes)))
    {
      if (src_node == pce_graph->source || src_node == pce_graph->sink)
	continue;
      pce_bb = (L_PCE_BB *) src_node->ptr;
      src_bb = pce_bb->bb;
      fprintf (stderr, "BB (cb %d, first op %d), index %d, weight %u, "
	       "excess %u, type %x, height %d", src_bb->cb->id,
	       src_bb->first_op ? src_bb->first_op->id : 0, pce_bb->index,
	       src_bb->weight, pce_bb->excess, src_bb->type, pce_bb->height);
      if (pce_bb->pce_type & BB_EXIT)
	fprintf (stderr, ", exit BB.\n");
      else
	fprintf (stderr, ".\n");
      if (pce_bb->pce_type & (BB_LOOSE | BB_PAST_CUT | BB_PRED | BB_COMBINE))
	{
	  fprintf (stderr, "\t");
	  if (pce_bb->pce_type & BB_LOOSE)
	    fprintf (stderr, "LOOSE ");
	  if (pce_bb->pce_type & BB_PAST_CUT)
	    fprintf (stderr, "PAST_CUT ");
	  if (pce_bb->pce_type & BB_PRED)
	    fprintf (stderr, "PRED ");
	  if (pce_bb->pce_type & BB_COMBINE)
	    {
	      size = Set_2array (pce_bb->reaching_assn, reach_array);
	      fprintf (stderr, "COMBINE (%d", reach_array[0]);
	      for (i = 1; i < size; i++)
		fprintf (stderr, " %d", reach_array[i]);
	      fprintf (stderr, ")");
	    }
	  fprintf (stderr, "\n");
	}
      List_start (src_node->pred);
      fprintf (stderr, "\tIncoming arcs:\n");
      while ((arc = (GraphArc) List_next (src_node->pred)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  bb_arc = pce_arc->bb_arc;
	  if (arc->pred == pce_graph->source)
	    fprintf (stderr, "\t\tConnected to source.\n");
	  else if (bb_arc->branch && bb_arc->taken)
	    fprintf (stderr, "\t\tArc from BB (cb %d op %d) from branch op "
		     "%d.\n", ((L_PCE_BB *) arc->pred->ptr)->bb->cb->id,
		     ((L_PCE_BB *) arc->pred->ptr)->bb->first_op ?
		     ((L_PCE_BB *) arc->pred->ptr)->bb->first_op->id : 0,
		     bb_arc->branch->id);
	  else
	    fprintf (stderr, "\t\tArc from fall-through from BB (cb %d "
		     "op %d).\n",
		     ((L_PCE_BB *) arc->pred->ptr)->bb->cb->id,
		     ((L_PCE_BB *) arc->pred->ptr)->bb->first_op ?
		     ((L_PCE_BB *) arc->pred->ptr)->bb->first_op->id : 0);
	  fprintf (stderr, "\t\t\tcapacity %u, flow %u.\n",
		   pce_arc->weight, pce_arc->flow);
	  if (pce_arc->pce_type & (ARC_LOOPBACK|ARC_COST))
	    {
	      if (pce_arc->pce_type & ARC_LOOPBACK)
		fprintf (stderr, "\t\tLOOPBACK ARC\n");
	      if (pce_arc->pce_type & ARC_COST)
		fprintf (stderr, "\t\tCOST ARC\n");
	    }
	}
      List_start (src_node->succ);
      fprintf (stderr, "\tOutgoing arcs:\n");
      while ((arc = (GraphArc) List_next (src_node->succ)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  bb_arc = pce_arc->bb_arc;
	  if (arc->succ == pce_graph->sink)
	    fprintf (stderr, "\t\tConnected to sink.\n");
	  else if (bb_arc->branch && bb_arc->taken)
	    fprintf (stderr, "\t\tArc from branch to dest BB (cb %d, op %d)."
		     "\n ", ((L_PCE_BB *) arc->succ->ptr)->bb->cb->id,
		     ((L_PCE_BB *) arc->succ->ptr)->bb->first_op ?
		     ((L_PCE_BB *) arc->succ->ptr)->bb->first_op->id : 0);
	  else
	    fprintf (stderr, "\t\tFall-through from op %d to dest BB (cb %d "
		     "op %d).\n",
		     src_bb->last_op ? src_bb->last_op->id : 0,
		     ((L_PCE_BB *) arc->succ->ptr)->bb->cb->id,
		     ((L_PCE_BB *) arc->succ->ptr)->bb->first_op ?
		     ((L_PCE_BB *) arc->succ->ptr)->bb->first_op->id : 0);
	  fprintf (stderr, "\t\t\tcapacity %u, flow %u.\n",
		   pce_arc->weight, pce_arc->flow);
	  if (pce_arc->pce_type & (ARC_LOOPBACK|ARC_COST))
	    {
	      if (pce_arc->pce_type & ARC_LOOPBACK)
		fprintf (stderr, "\t\tLOOPBACK ARC\n");
	      if (pce_arc->pce_type & ARC_COST)
		fprintf (stderr, "\t\tCOST ARC\n");
	    }
	}
    }
  free (reach_array);
}
#endif


static L_PCE_graph *
L_construct_SPRE_graph (Graph bb_weighted_graph, int expression_index,
			int mem_conservative)
{
  GraphNode src_node, dest_node, src_pce_node, dest_pce_node, source, sink;
  GraphArc arc, new_arc;
  Graph graph;
  L_BB *src_bb, *dest_bb;
  L_BB_arc *bb_arc;
  L_PCE_BB *src_pce_bb, *dest_pce_bb, *source_pce_bb, *sink_pce_bb;
  L_PCE_arc *new_pce_arc;
  L_PCE_graph *pce_graph;
  DF_PCE_INFO *src_pce, *dest_pce;
  int n_vertices = 2, n_edges = 0, num_cut_active = 0;
#ifdef DO_GRAPH_MEMORIZATION
  Set taken_br_edges = NULL;
  Set untaken_br_edges = NULL;
  Set fallthrough_cbs = NULL;
#endif

  /* Initialize graph, add start and sink nodes. */
  pce_graph = malloc (sizeof (L_PCE_graph));
  graph = Graph_create_graph ();
  pce_graph->graph = graph;
  source = Graph_create_graph_node (graph);
  source_pce_bb = L_new_PCE_bb (source, NULL);
  source->ptr = source_pce_bb;
  GraphRootNode (graph) = source;
  sink = Graph_create_graph_node (graph);
  sink_pce_bb = L_new_PCE_bb (sink, NULL);
  sink->ptr = sink_pce_bb;
  pce_graph->source = source;
  pce_graph->sink = sink;
  pce_graph->expression_index = expression_index;
  pce_graph->visit_nodes = NULL;

#ifdef DEBUG_SPRE_CONSTRUCT
  fprintf (stderr, "Constructing SPRE graph for expression %d.\n",
	   expression_index);
#endif

  /* Walk through BBs. */
  List_start (bb_weighted_graph->nodes);
  while ((src_node = (GraphNode) List_next (bb_weighted_graph->nodes)))
    {
      int x_part_avail_flag = 0;
      /* For each BB, check x availability (xu_safe) for expression,
       * if not, check dest BBs. */
      src_bb = (L_BB *) GraphContents (src_node);
#ifdef DEBUG_SPRE_CONSTRUCT
      fprintf (stderr, "Checking outbound edges for BB (cb %d, first_op "
	       "%d).\n", src_bb->cb->id, (src_bb->first_op) ?
	       src_bb->first_op->id : 0);
#endif
      src_pce = src_bb->pf_bb->info->pce_info;
      if (Set_in (src_pce->x_comp, expression_index) ||
	  Set_in (src_pce->xu_safe, expression_index))
	continue;

      if (Set_in (src_pce->x_spec_us, expression_index))
	x_part_avail_flag = 1;

      /* Check each outbound edge, if partially anticipable at dest,
       * add bb, edge, and dest_bb */
      List_start (src_node->succ);
      while ((arc = (GraphArc) List_next (src_node->succ)))
	{
	  int exit_flag;

	  dest_node = arc->succ;
	  dest_bb = (L_BB *) GraphContents (dest_node);
	  bb_arc = (L_BB_arc *) GraphArcContents (arc);
#ifdef DEBUG_SPRE_CONSTRUCT
	  if (bb_arc->branch)
	    fprintf (stderr, "\tArc corresponding to branch op %d, dest cb "
		     "%d.\n", bb_arc->branch->id, dest_bb->cb->id);
	  else
	    fprintf (stderr, "\tArc corresponding to fall-through from op %d "
		     "to dest BB at op %d, cb %d.\n",
		     src_bb->last_op ? src_bb->last_op->id : 0,
		     (dest_bb->first_op) ? dest_bb->first_op->id : 0,
		     dest_bb->cb->id);
#endif
	  dest_pce = dest_bb->pf_bb->info->pce_info;
	  if (!Set_in (dest_pce->n_spec_ds, expression_index))
	    continue;
	  exit_flag = !Set_in (src_pce->trans, expression_index);
	  if (!x_part_avail_flag)
	    {
	      if (!Set_in (dest_pce->n_spec_us, expression_index))
		continue;
	      else
		exit_flag = 1;
	    }

#ifdef DEBUG_SPRE_CONSTRUCT
	  fprintf (stderr, "\t\tADDING ARC.\n");
#endif

#ifdef DO_GRAPH_MEMORIZATION
	  if (bb_arc->branch)
	    if (bb_arc->taken)
	      taken_br_edges = Set_add (taken_br_edges, bb_arc->branch->id);
	    else
	      untaken_br_edges =
		Set_add (untaken_br_edges, bb_arc->branch->id);
	  else
	    fallthrough_cbs = Set_add (fallthrough_cbs, src_bb->cb);
#endif

	  /* For non-transparent nodes or those that are fully unavailable,
	   * we need to define an exit L_PCE_BB for correctness. */
	  if (exit_flag)
	    {
	      if (!(src_pce_bb = (L_PCE_BB *) src_bb->ptr_exit))
		{
#ifdef DEBUG_SPRE_CONSTRUCT
		  fprintf (stderr, "\t\tAdding exit src BB first op %d, "
			   "cb %d.\n", src_bb->first_op ?
			   src_bb->first_op->id : 0, src_bb->cb->id);
#endif
		  src_pce_node = Graph_create_graph_node (graph);
		  src_pce_bb = L_new_PCE_bb (src_pce_node, src_bb);
		  src_pce_bb->pce_type |= BB_EXIT;
		  GraphNodeContents (src_pce_node) = src_pce_bb;
		  src_bb->ptr_exit = src_pce_bb;
		  n_vertices++;
		}
	      else
		src_pce_node = src_pce_bb->node;
	    }
	  else
	    {
	      if (!(src_pce_bb = (L_PCE_BB *) src_bb->ptr))
		{
#ifdef DEBUG_SPRE_CONSTRUCT
		  fprintf (stderr, "\t\tAdding src BB first op %d, cb "
			   "%d.\n", src_bb->first_op ?
			   src_bb->first_op->id : 0, src_bb->cb->id);
#endif
		  src_pce_node = Graph_create_graph_node (graph);
		  src_pce_bb = L_new_PCE_bb (src_pce_node, src_bb);
		  GraphNodeContents (src_pce_node) = src_pce_bb;
		  src_bb->ptr = src_pce_bb;
		  n_vertices++;
		}
	      else
		src_pce_node = src_pce_bb->node;
	    }

	  if (!(dest_pce_bb = (L_PCE_BB *) dest_bb->ptr))
	    {
#ifdef DEBUG_SPRE_CONSTRUCT
	      fprintf (stderr, "\t\tAdding dest BB first op %d, cb %d.\n",
		       dest_bb->first_op ? dest_bb->first_op->id : 0,
		       dest_bb->cb->id);
#endif
	      dest_pce_node = Graph_create_graph_node (graph);
	      dest_pce_bb = L_new_PCE_bb (dest_pce_node, dest_bb);
	      GraphNodeContents (dest_pce_node) = dest_pce_bb;
	      dest_bb->ptr = dest_pce_bb;
	      n_vertices++;
	    }
	  else
	    dest_pce_node = dest_pce_bb->node;
	  new_arc = Graph_connect_nodes (graph, src_pce_node, dest_pce_node);
	  new_pce_arc = L_new_PCE_arc (new_arc, bb_arc);
	  GraphArcContents (new_arc) = new_pce_arc;
	  bb_arc->ptr = new_pce_arc;
	  n_edges++;
#ifdef SPLIT_EDGE_METRIC
	  if (src_bb->type & BB_REMOVABLE || dest_bb->type & BB_REMOVABLE)
	    {
	      if (new_pce_arc->weight > ITMAXU32 / SPLIT_EDGE_FACTOR)
		{
		  fprintf (stderr, "SPRE: Split edge metric overflow.\n");
		  new_pce_arc->weight = UNSIGNED_INT_MAX;
		}
	      else
		new_pce_arc->weight *= SPLIT_EDGE_FACTOR;
	    }
#endif
	}
    }

  pce_graph->n_vertices = n_vertices;

#ifdef DO_GRAPH_MEMORIZATION
  /* Link up edges sets to SPRE graph. */
  pce_graph->taken_br_edges = taken_br_edges;
  pce_graph->untaken_br_edges = untaken_br_edges;
  pce_graph->fallthrough_cbs = fallthrough_cbs;
  /* Clear for exit info. */
  taken_br_edges = NULL;
  untaken_br_edges = NULL;
  fallthrough_cbs = NULL;
#endif

  /* Link subgraphs to form single source/sink graph.
   * Initialize adjacent-to-source nodes. */
  source_pce_bb->height = n_vertices;
  List_start (graph->nodes);
  while ((src_node = (GraphNode) List_next (graph->nodes)))
    {
      if (src_node == source || src_node == sink)
	continue;
      /* If no incoming arcs, add arc from source to node. */
      if (!((GraphArc) List_first (src_node->pred)))
	{
	  unsigned int weight;
	  src_pce_bb = (L_PCE_BB *) src_node->ptr;
	  new_arc = Graph_connect_nodes (graph, source, src_node);
	  new_pce_arc = L_new_PCE_arc (new_arc, NULL);
	  new_arc->ptr = new_pce_arc;
	  weight = src_pce_bb->bb->weight;
#ifdef SPLIT_EDGE_METRIC
	  if (src_pce_bb->bb->type & BB_REMOVABLE)
	    {
	      if (weight > ITMAXU32 / SPLIT_EDGE_FACTOR)
		{
		  fprintf (stderr, "SPRE: Split edge metric overflow.\n");
		  weight = UNSIGNED_INT_MAX;
		}
	      else
		weight *= SPLIT_EDGE_FACTOR;
	    }
#endif
	  new_pce_arc->weight = weight;
	  new_pce_arc->flow = weight;
	  src_pce_bb->excess = weight;
	  src_pce_bb->height = 1;
	  num_cut_active++;
	  n_edges++;

#ifdef DO_GRAPH_MEMORIZATION
	  List_start (src_node->succ);
	  while (arc = (GraphArc) List_next (src_node->succ))
	    {
	      bb_arc = (L_BB_arc *) GraphArcContents (arc);
	      if (bb_arc->branch)
		if (bb_arc->taken)
		  taken_br_edges = Set_add (taken_br_edges,
					    bb_arc->branch->id);
		else
		  untaken_br_edges = Set_add (untaken_br_edges,
					      bb_arc->branch->id);
	      else
		fallthrough_cbs = Set_add (fallthrough_cbs,
					   src_pce_bb->bb->cb);
	    }
#endif
	}
      /* If no outgoing arcs, add arc from node to sink. */
      if (!((GraphArc) List_first (src_node->succ)))
	{
	  new_arc = Graph_connect_nodes (graph, src_node, sink);
	  new_pce_arc = L_new_PCE_arc (new_arc, NULL);
	  new_arc->ptr = new_pce_arc;
	  new_pce_arc->weight = ((L_PCE_BB *) src_node->ptr)->bb->weight;
	  ((L_PCE_BB *) src_node->ptr)->pce_type |= BB_SINK_ADJACENT;
	  n_edges++;
	}
    }

  pce_graph->num_cut_active = num_cut_active;
  pce_graph->n_edges = n_edges;

#ifdef DO_GRAPH_MEMORIZATION
  pce_graph->taken_exit_br = taken_br_edges;
  pce_graph->untaken_exit_br = untaken_br_edges;
  pce_graph->fallthrough_exit_cbs = fallthrough_cbs;
#endif

#if defined DEBUG_SPRE_CONSTRUCT && defined PRINT_SPRE_GRAPH
  L_print_PCE_graph (pce_graph);
#endif

  return pce_graph;
}


#if 0
/* CURRENTLY NON-FUNCTIONAL: needs to be rewritten */
/* L_PCE_reg_pressure_reweighting
 * 4/29/03 SER: This function reweights edges on the SPRE graph using
 * a metric based on a combination of BB length and cutset size.
 */
static void
L_PCE_reg_pressure_reweighting (L_PCE_graph * pce_graph, int direction)
{
  List start_list, nodes;
  GraphNode node, source, sink;
  GraphArc arc;
  L_BB *bb;
  L_PCE_BB *pce_bb, *succ_pce_bb;
  L_PCE_arc *pce_arc;
  double largest_inc;

  /* Set up termination nodes. */
  sink = pce_graph->sink;
  List_start (sink->pred);
  start_list = NULL;
  while ((arc = (GraphArc) List_next (sink->pred)))
    {
      node = arc->pred;
      ((L_PCE_BB *) node->ptr)->reg_pressure_inc = 1.0;
      start_list = List_insert_last (start_list, node);
    }

  /* Set up for remainder of SPRE graph. */
  source = pce_graph->source;
  nodes = NULL;
  List_start (start_list);
  while ((node = (GraphNode) List_next (start_list)))
    {
      List_start (node->pred);
      while ((arc = (GraphArc) List_next (node->pred)))
	if (arc->pred != source)
	  nodes = List_insert_last (nodes, arc->pred);
    }
  start_list = List_reset (start_list);

  /* Process remainder of SPRE graph. */
  List_start (nodes);
  while ((node = (GraphNode) List_next (nodes)))
    {
      pce_bb = (L_PCE_BB *) node->ptr;
      /* Already processed or is source. */
      if ((pce_bb->reg_pressure_inc != 0.0) || node == source)
	{
	  nodes = List_delete_current (nodes);
	  continue;
	}
      /* Find largest downward increment, calculate increment. */
      largest_inc = 0.0;
      List_start (node->succ);
      while ((arc = (GraphArc) List_next (node->succ)))
	{
	  succ_pce_bb = (L_PCE_BB *) arc->succ->ptr;
	  if (succ_pce_bb->reg_pressure_inc > largest_inc)
	    largest_inc = succ_pce_bb->reg_pressure_inc;
	}
      if (largest_inc < 1.0)
	L_punt ("SPRE reweighting: bad increment size.");
      bb = pce_bb->bb;
      largest_inc += (double) bb->bb_size * (double) bb->cutset_size *
	REG_PRESSURE_INC;
      pce_bb->reg_pressure_inc = largest_inc;

      /* Rebalance weights. */
      if (pce_bb->pce_type & BB_EXIT)
	{
	  arc = (GraphArc) List_first (node->pred);
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  pce_arc->weight = (unsigned int)
	    ((double) pce_arc->weight * largest_inc);
	  pce_arc->flow = pce_arc->weight;
	  pce_bb->excess = pce_arc->weight;
	}
      else
	{
	  List_start (node->pred);
	  while ((arc = (GraphArc) List_next (node->pred)))
	    {
	      pce_arc = (L_PCE_arc *) arc->ptr;
	      pce_arc->weight = (unsigned int)
		((double) pce_arc->weight * largest_inc);
	      if (((L_PCE_BB *) arc->pred)->reg_pressure_inc != 0.0)
		L_punt ("SPRE reweight: register pressure reweighting not "
			"proceeding in good order.");
	      /* Place preds into list if has a SPRE_arc attached. */
	      if (arc->ptr != NULL)
		nodes = List_insert_last (nodes, arc->pred);
	    }
	}
      nodes = List_delete_current (nodes);
    }
  nodes = List_reset (nodes);
}
#endif


static void
L_delete_PCE_graph (L_PCE_graph * pce_graph)
{
  GraphNode node;
  GraphArc arc;
  Graph graph;

  graph = pce_graph->graph;
  /* Unlink and delete each node. */
  List_start (graph->nodes);
  while ((node = (GraphNode) List_next (graph->nodes)))
    L_delete_PCE_bb ((L_PCE_BB *) GraphNodeContents (node));

  /* Unlink and delete each edge. */
  List_start (graph->arcs);
  while ((arc = (GraphArc) List_next (graph->arcs)))
    L_delete_PCE_arc ((L_PCE_arc *) GraphArcContents (arc));

  pce_graph->visit_nodes = List_reset (pce_graph->visit_nodes);

  Graph_free_graph (graph);

  if (L_PCE_BB_Pool)
    L_print_alloc_info (stderr, L_PCE_BB_Pool, 0);
  if (L_PCE_arc_Pool)
    L_print_alloc_info (stderr, L_PCE_arc_Pool, 0);

  free (pce_graph);
}


static int
L_generate_token_from_PCE_graph (L_PCE_graph * pce_graph)
{
  int i, token = 0, *array, set_size;

  set_size = Set_size (pce_graph->taken_br_edges);
  array = (int *) malloc (sizeof (int) * set_size);
  Set_2array (pce_graph->taken_br_edges, array);

  for (i = 0; i < set_size; i += 7)
    {
      token += array[i];
      token = token << 1;
    }
  free (array);
  return token;
}

static inline L_PCE_graph_cut *
L_PCE_find_graph_cut (L_PCE_graph * pce_graph)
{
  int token, hash_id;
  L_PCE_graph_cut *ptr;
  token = L_generate_token_from_PCE_graph (pce_graph);
  hash_id = token & PCE_GRAPH_HASH_TBL_MASK;

  for (ptr = L_PCE_graph_cut_tbl[hash_id]; ptr != NULL; ptr = ptr->next)
    {
      if (token != ptr->token)
	continue;
      if (!Set_same (ptr->taken_br_edges, pce_graph->taken_br_edges))
	continue;
      if (!Set_same (ptr->untaken_br_edges, pce_graph->untaken_br_edges))
	continue;
      if (!Set_same (ptr->fallthrough_cbs, pce_graph->fallthrough_cbs))
	continue;
      if (!Set_same (ptr->taken_exit_br, pce_graph->taken_exit_br))
	continue;
      if (!Set_same (ptr->untaken_exit_br, pce_graph->untaken_exit_br))
	continue;
      if (!Set_same (ptr->fallthrough_exit_cbs,
		     pce_graph->fallthrough_exit_cbs))
	continue;
      return ptr;
    }
  return NULL;
}


static inline L_PCE_graph_cut *
L_new_graph_cut (L_PCE_graph * pce_graph)
{
  L_PCE_graph_cut *graph_cut;
  int token, hash_id;

  if (L_PCE_graph_cut_Pool == NULL)
    L_PCE_graph_cut_Pool = L_create_alloc_pool ("L_PCE_graph_cut",
						sizeof (L_PCE_graph_cut), 64);

  graph_cut = (L_PCE_graph_cut *) L_alloc (L_PCE_graph_cut_Pool);
  graph_cut->taken_br_edges = Set_copy (pce_graph->taken_br_edges);
  graph_cut->untaken_br_edges = Set_copy (pce_graph->untaken_br_edges);
  graph_cut->fallthrough_cbs = Set_copy (pce_graph->fallthrough_cbs);
  graph_cut->taken_exit_br = Set_copy (pce_graph->taken_exit_br);
  graph_cut->untaken_exit_br = Set_copy (pce_graph->untaken_exit_br);
  graph_cut->fallthrough_exit_cbs = Set_copy (pce_graph->fallthrough_cbs);

  graph_cut->taken_br_cut = NULL;
  graph_cut->untaken_br_cut = NULL;
  graph_cut->fallthrough_cbs_cut = NULL;

  graph_cut->expressions = NULL;
  graph_cut->expressions = Set_add (graph_cut->expressions,
				    pce_graph->expression_index);
  graph_cut->repeat = 0;
  graph_cut->share = 0;
  graph_cut->n_vertices = pce_graph->n_vertices;
  graph_cut->n_edges = pce_graph->n_edges;

  token = L_generate_token_from_PCE_graph (pce_graph);
  graph_cut->token = token;

  /* Insert graph cut into hash table. */
  hash_id = token & PCE_GRAPH_HASH_TBL_MASK;
  graph_cut->next = L_PCE_graph_cut_tbl[hash_id];
  L_PCE_graph_cut_tbl[hash_id] = graph_cut;

  return graph_cut;
}

void
L_print_PCE_graph_cuts ()
{
#ifdef TRACK_GRAPH_MEMORIZATION
  int i;
  L_PCE_graph_cut *ptr;

  if (L_PCE_graph_cut_tbl == NULL)
    return;

  fprintf (stderr, "Printing PCE graph cuts.\n");
  for (i = 0; i < PCE_GRAPH_TBL_SIZE; i++)
    {
      if (L_PCE_graph_cut_tbl[i])
	fprintf (stderr, "Graph cuts for hash %d:\n", i);
      for (ptr = L_PCE_graph_cut_tbl[i]; ptr != NULL; ptr = ptr->next)
	{
	  fprintf (stderr, "GRAPH CUT:\n");
	  Set_print (stderr, "EXPRESSIONS", ptr->expressions);
	  fprintf (stderr, "\tVertices: %d\tRepeats: %d\tShares: %d.\n",
		   ptr->n_vertices, ptr->repeat, ptr->share);
	}
    }
#endif
}

void
L_delete_PCE_graph_cuts ()
{
#ifdef DO_GRAPH_MEMORIZATION
  int i;
  L_PCE_graph_cut *ptr, *next;

  if (L_PCE_graph_cut_tbl == NULL)
    return;

  for (i = 0; i < PCE_GRAPH_TBL_SIZE; i++)
    {
      for (ptr = L_PCE_graph_cut_tbl[i]; ptr != NULL; ptr = next)
	{
	  next = ptr->next;
	  L_free (L_PCE_graph_cut_Pool, ptr);
	}
      L_PCE_graph_cut_tbl[i] = NULL;
    }
  free (L_PCE_graph_cut_tbl);
  L_PCE_graph_cut_tbl = NULL;
#endif
}


/* This function globally updates heights of graph nodes by doing a
 * backwards breadth-first search from the sink. This is necessary if we
 * are to base min-cuts on height. */
static void
L_PCE_global_update (L_PCE_graph * pce_graph)
{
  List search = NULL, nodes;
  L_PCE_BB *pce_bb;
  L_PCE_arc *pce_arc;
  GraphNode other_node, base_node;
  GraphArc arc;
  int n_vertices, new_height, num_cut_active = 0;

  /* Initialize all non-sink nodes to height of n_vertices */
  n_vertices = pce_graph->n_vertices;
  nodes = pce_graph->graph->nodes;
  List_start (nodes);
  while ((base_node = (GraphNode) List_next (nodes)))
    ((L_PCE_BB *) base_node->ptr)->height = n_vertices;
  ((L_PCE_BB *) pce_graph->sink->ptr)->height = 0;

  search = List_insert_last (search, pce_graph->sink);
  List_start (search);
  while ((base_node = (GraphNode) List_next (search)))
    {
      pce_bb = (L_PCE_BB *) base_node->ptr;
      new_height = pce_bb->height + 1;
      List_start (base_node->pred);
      while ((arc = (GraphArc) List_next (base_node->pred)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  if (pce_arc->flow == pce_arc->weight)
	    continue;
	  other_node = arc->pred;
	  if (other_node == pce_graph->source)
	    continue;
	  pce_bb = (L_PCE_BB *) other_node->ptr;
	  if (pce_bb->height == n_vertices)
	    {
	      pce_bb->height = new_height;
	      search = List_insert_last (search, other_node);
#ifdef CUT_SHORT_CIRCUIT
	      if (pce_bb->excess)
		num_cut_active++;
#endif
	    }
	}
      List_start (base_node->succ);
      while ((arc = (GraphArc) List_next (base_node->succ)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  if (pce_arc->flow == 0)
	    continue;
	  other_node = arc->succ;
	  pce_bb = (L_PCE_BB *) other_node->ptr;
	  if (pce_bb->height == n_vertices)
	    {
	      pce_bb->height = new_height;
	      search = List_insert_last (search, other_node);
#ifdef CUT_SHORT_CIRCUIT
	      if (pce_bb->excess)
		num_cut_active++;
#endif
	    }
	}
    }
  pce_graph->num_cut_active = num_cut_active;
}


static void
L_PCE_reverse_global_update (L_PCE_graph * pce_graph)
{
  List search = NULL, nodes;
  L_PCE_BB *pce_bb;
  L_PCE_arc *pce_arc;
  GraphNode other_node, base_node;
  GraphArc arc;
  int n_vertices, new_height, num_cut_active = 0;

  /* Initialize all non-sink nodes to height of n_vertices */
  n_vertices = pce_graph->n_vertices;
  nodes = pce_graph->graph->nodes;
  List_start (nodes);
  while ((base_node = (GraphNode) List_next (nodes)))
    ((L_PCE_BB *) base_node->ptr)->height = n_vertices;
  ((L_PCE_BB *) pce_graph->source->ptr)->height = 0;

  search = List_insert_last (search, pce_graph->source);
  List_start (search);
  while ((base_node = (GraphNode) List_next (search)))
    {
      pce_bb = (L_PCE_BB *) base_node->ptr;
      new_height = pce_bb->height + 1;
      List_start (base_node->succ);
      while ((arc = (GraphArc) List_next (base_node->succ)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  if (pce_arc->flow == pce_arc->weight)
	    continue;
	  other_node = arc->succ;
	  if (other_node == pce_graph->sink)
	    continue;
	  pce_bb = (L_PCE_BB *) other_node->ptr;
	  if (pce_bb->height == n_vertices)
	    {
	      pce_bb->height = new_height;
	      search = List_insert_last (search, other_node);
#ifdef CUT_SHORT_CIRCUIT
	      if (pce_bb->excess)
		num_cut_active++;
#endif
	    }
	}
      List_start (base_node->pred);
      while ((arc = (GraphArc) List_next (base_node->pred)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  if (pce_arc->flow == 0)
	    continue;
	  other_node = arc->pred;
	  pce_bb = (L_PCE_BB *) other_node->ptr;
	  if (pce_bb->height == n_vertices)
	    {
	      pce_bb->height = new_height;
	      search = List_insert_last (search, other_node);
#ifdef CUT_SHORT_CIRCUIT
	      if (pce_bb->excess)
		num_cut_active++;
#endif
	    }
	}
    }
  pce_graph->num_cut_active = num_cut_active;
}


/* This function discharges the excess from a BB to its neighbors. It is
 * important to note that edges MUST be split, or the discharge arithmetic
 * must be written, because it assumes that the source and dest of an arc
 * are distinctly different (no self-loops allowed).
 */
static void
L_PCE_discharge (L_PCE_graph * pce_graph, List * visit_nodes,
		 L_PCE_BB * pce_bb)
{
  GraphNode node, dest_node;
  GraphArc arc;
  L_PCE_BB *dest_pce_bb;
  L_PCE_arc *pce_arc;
  int min_height;
  unsigned int temp;
  node = pce_bb->node;

  while (pce_bb->excess > 0)
    {
      min_height = -1;
      List_start (node->succ);
      while ((arc = (GraphArc) List_next (node->succ)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  /* Check for saturated forward edge. */
	  if (pce_arc->flow == pce_arc->weight)
	    continue;
	  dest_node = arc->succ;
	  dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
	  /* push */
	  if (pce_bb->height == dest_pce_bb->height + 1)
	    {
	      temp = pce_arc->weight - pce_arc->flow;
	      if (pce_bb->excess <= temp)
		{
#ifdef DEBUG_PREFLOW_PUSH
		  fprintf (stderr, "Forward unsat push excess %u from "
			   "BB (cb %d op %d) to BB ", pce_bb->excess,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0);
		  if (dest_node == pce_graph->sink)
		    fprintf (stderr, "sink.\n");
		  else
		    fprintf (stderr, "(cb %d op %d).\n",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
#endif
		  pce_arc->flow += pce_bb->excess;
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += pce_bb->excess;
		  pce_bb->excess = 0;
		  break;
		}
	      else
		{
#ifdef DEBUG_PREFLOW_PUSH
		  fprintf (stderr, "Forward sat push excess %u from "
			   "BB (cb %d op %d) to BB ", temp,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0);
		  if (dest_node == pce_graph->sink)
		    fprintf (stderr, "sink, ");
		  else
		    fprintf (stderr, "(cb %d op %d), ",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
		  fprintf (stderr, "remainder %u.\n", pce_bb->excess - temp);
#endif
		  pce_arc->flow = pce_arc->weight;
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += temp;
		  pce_bb->excess -= temp;
		}
	    }
	  /* track minimum height of unsaturated edges. */
	  else if (min_height == -1 || dest_pce_bb->height < min_height)
	    min_height = dest_pce_bb->height;
	}
      if (pce_bb->excess == 0)
	break;
      List_start (node->pred);
      while ((arc = (GraphArc) List_next (node->pred)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  /* Check for saturated backward edge: 0 weight. */
	  if (!(pce_arc->flow))
	    continue;
	  dest_node = arc->pred;
	  dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
	  /* reverse push */
	  if (pce_bb->height == dest_pce_bb->height + 1)
	    {
	      if (pce_bb->excess <= pce_arc->flow)
		{
#ifdef DEBUG_PREFLOW_PUSH
		  fprintf (stderr, "Backward unsat push excess %u from "
			   "BB (cb %d op %d) to BB ", pce_bb->excess,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0, pce_bb->bb->cb->id);
		  if (dest_node == pce_graph->source)
		    fprintf (stderr, "source.\n");
		  else
		    fprintf (stderr, "(cb %d op %d).\n",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
#endif
		  pce_arc->flow -= pce_bb->excess;
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += pce_bb->excess;
		  pce_bb->excess = 0;
		  break;
		}
	      else
		{
#ifdef DEBUG_PREFLOW_PUSH
		  fprintf (stderr, "Backward sat push excess %u from "
			   "BB (cb %d op %d) to BB ", pce_arc->flow,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0);
		  if (dest_node == pce_graph->source)
		    fprintf (stderr, "source, ");
		  else
		    fprintf (stderr, "(cb %d op %d), ",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
		  fprintf (stderr, "remainder %u.\n",
			   pce_bb->excess - pce_arc->flow);
#endif
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += pce_arc->flow;
		  pce_bb->excess -= pce_arc->flow;
		  pce_arc->flow = 0;
		}
	    }
	  else if (min_height == -1 || dest_pce_bb->height < min_height)
	    min_height = dest_pce_bb->height;
	}
      if (pce_bb->excess <= 0)
	break;
      /* at this point, can't push anymore...time to lift */
      if (min_height == -1)
	L_punt ("Min_height still -1...this shouldn't happen!");
#ifdef DEBUG_PREFLOW_PUSH
      fprintf (stderr, "Raising BB (cb %d op %d) height from %d to %d.\n",
	       pce_bb->bb->cb->id,
	       pce_bb->bb->first_op ? pce_bb->bb->first_op->id : 0,
	       pce_bb->height, min_height + 1);
      if (min_height > pce_graph->n_vertices)
	L_punt ("PCE: ERROR: height exceeds maximum possible!");
      if (pce_bb->height > min_height + 1)
	L_punt ("PCE: Lowering node! This shouldn't happen!");
#endif
#ifdef CUT_SHORT_CIRCUIT
      if (pce_bb->height < pce_graph->n_vertices &&
	  min_height + 1 >= pce_graph->n_vertices)
	(pce_graph->num_cut_active)--;
#endif
      pce_bb->height = min_height + 1;
    }
}


static void
L_PCE_reverse_discharge (L_PCE_graph * pce_graph, List * visit_nodes,
			 L_PCE_BB * pce_bb)
{
  GraphNode node, dest_node;
  GraphArc arc;
  L_PCE_BB *dest_pce_bb;
  L_PCE_arc *pce_arc;
  int min_height;
  unsigned int temp;

  node = pce_bb->node;
  while (pce_bb->excess > 0)
    {
      min_height = -1;
      List_start (node->pred);
      while ((arc = (GraphArc) List_next (node->pred)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  /* Check for saturated forward edge. */
	  if (pce_arc->flow == pce_arc->weight)
	    continue;
	  dest_node = arc->pred;
	  dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
	  /* push */
	  if (pce_bb->height == dest_pce_bb->height + 1)
	    {
	      temp = pce_arc->weight - pce_arc->flow;
	      if (pce_bb->excess <= temp)
		{
#ifdef DEBUG_REV_PREFLOW_PUSH
		  fprintf (stderr, "Forward unsat push excess %u from "
			   "BB (cb %d op %d) to BB ", pce_bb->excess,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0);
		  if (dest_node == pce_graph->source)
		    fprintf (stderr, "source.\n");
		  else
		    fprintf (stderr, "(cb %d op %d).\n",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
#endif
		  pce_arc->flow += pce_bb->excess;
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += pce_bb->excess;
		  pce_bb->excess = 0;
		  break;
		}
	      else
		{
#ifdef DEBUG_REV_PREFLOW_PUSH
		  fprintf (stderr, "Forward sat push excess %u from "
			   "BB (cb %d op %d) to BB ", temp,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0);
		  if (dest_node == pce_graph->source)
		    fprintf (stderr, "source, ");
		  else
		    fprintf (stderr, "(cb %d op %d), ",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
		  fprintf (stderr, "remainder %u.\n", pce_bb->excess - temp);
#endif
		  pce_arc->flow = pce_arc->weight;
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += temp;
		  pce_bb->excess -= temp;
		}
	    }
	  /* track minimum height of unsaturated edges. */
	  else if (min_height == -1 || dest_pce_bb->height < min_height)
	    min_height = dest_pce_bb->height;
	}
      if (pce_bb->excess == 0)
	break;
      List_start (node->succ);
      while ((arc = (GraphArc) List_next (node->succ)))
	{
	  pce_arc = (L_PCE_arc *) arc->ptr;
	  /* Check for saturated backward edge: 0 weight. */
	  if (!(pce_arc->flow))
	    continue;
	  dest_node = arc->succ;
	  dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
	  /* reverse push */
	  if (pce_bb->height == dest_pce_bb->height + 1)
	    {
	      if (pce_bb->excess <= pce_arc->flow)
		{
#ifdef DEBUG_REV_PREFLOW_PUSH
		  fprintf (stderr, "Backward unsat push excess %u from "
			   "BB (cb %d op %d) to BB ", pce_bb->excess,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0, pce_bb->bb->cb->id);
		  if (dest_node == pce_graph->sink)
		    fprintf (stderr, "sink.\n");
		  else
		    fprintf (stderr, "(cb %d op %d).\n",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
#endif
		  pce_arc->flow -= pce_bb->excess;
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += pce_bb->excess;
		  pce_bb->excess = 0;
		  break;
		}
	      else
		{
#ifdef DEBUG_REV_PREFLOW_PUSH
		  fprintf (stderr, "Backward sat push excess %u from "
			   "BB (cb %d op %d) to BB ", pce_arc->flow,
			   pce_bb->bb->cb->id, pce_bb->bb->first_op ?
			   pce_bb->bb->first_op->id : 0);
		  if (dest_node == pce_graph->sink)
		    fprintf (stderr, "sink, ");
		  else
		    fprintf (stderr, "(cb %d op %d), ",
			     dest_pce_bb->bb->cb->id,
			     dest_pce_bb->bb->first_op ?
			     dest_pce_bb->bb->first_op->id : 0);
		  fprintf (stderr, "remainder %u.\n",
			   pce_bb->excess - pce_arc->flow);
#endif
		  if (dest_pce_bb->excess == 0)
		    {
		      *visit_nodes = List_insert_last (*visit_nodes,
						       dest_pce_bb);
#ifdef CUT_SHORT_CIRCUIT
		      if (dest_pce_bb->height < pce_graph->n_vertices)
			(pce_graph->num_cut_active)++;
#endif
		    }
		  dest_pce_bb->excess += pce_arc->flow;
		  pce_bb->excess -= pce_arc->flow;
		  pce_arc->flow = 0;
		}
	    }
	  else if (min_height == -1 || dest_pce_bb->height < min_height)
	    min_height = dest_pce_bb->height;
	}
      if (pce_bb->excess <= 0)
	break;
      /* at this point, can't push anymore...time to lift */
      if (min_height == -1)
	L_punt ("Min_height still -1...this shouldn't happen!");
#ifdef DEBUG_REV_PREFLOW_PUSH
      fprintf (stderr, "Raising BB (cb %d op %d) height from %d to %d.\n",
	       pce_bb->bb->cb->id,
	       pce_bb->bb->first_op ? pce_bb->bb->first_op->id : 0,
	       pce_bb->height, min_height + 1);
      if (min_height > pce_graph->n_vertices)
	L_punt ("PCE: ERROR: height exceeds maximum possible!");
      if (pce_bb->height > min_height + 1)
	L_punt ("PCE: Lowering node! This shouldn't happen!");
#endif
#ifdef CUT_SHORT_CIRCUIT
      if (pce_bb->height < pce_graph->n_vertices &&
	  min_height + 1 >= pce_graph->n_vertices)
	(pce_graph->num_cut_active)--;
#endif
      pce_bb->height = min_height + 1;
    }
}


static void
L_PCE_push_preflow (L_PCE_graph * pce_graph)
{
  L_PCE_BB *pce_bb;
  GraphArc arc;
  List visit_nodes = NULL;
  int visits = 0, discharges = 0, until_update, updates = 0;
  L_PCE_BB *source, *sink;

#ifdef DEBUG_PREFLOW_PUSH
  fprintf (stderr, "Beginning preflow-push.\n");
#endif

  /* Populate the list with the next-to-start nodes. */
  List_start (pce_graph->source->succ);
  while ((arc = (GraphArc) List_next (pce_graph->source->succ)))
    visit_nodes = List_insert_last (visit_nodes, (L_PCE_BB *) arc->succ->ptr);

#ifdef GLOBAL_UPDATE_HEURISTIC
  until_update = GLOBAL_UPDATE_FACTOR * pce_graph->n_vertices;
#endif
  /* Push preflow until stable. */
  source = (L_PCE_BB *) pce_graph->source->ptr;
  sink = (L_PCE_BB *) pce_graph->sink->ptr;
  List_start (visit_nodes);
  while ((pce_bb = (L_PCE_BB *) List_next (visit_nodes)))
    {
      if ((pce_bb->excess > 0) && (pce_bb != source) && (pce_bb != sink))
	{
	  L_PCE_discharge (pce_graph, &visit_nodes, pce_bb);
	  discharges++;
#ifdef GLOBAL_UPDATE_HEURISTIC
	  until_update--;
#endif
	}
#ifdef CUT_SHORT_CIRCUIT
      /* If the cut is stabilized, can quit. */
      if (pce_bb->height < pce_graph->n_vertices)
	(pce_graph->num_cut_active)--;
      if (pce_graph->num_cut_active == 0)
	break;
#endif
      visits++;
      visit_nodes = List_delete_current (visit_nodes);
#ifdef GLOBAL_UPDATE_HEURISTIC
      if (until_update == 0)
	{
	  until_update = GLOBAL_UPDATE_FACTOR * pce_graph->n_vertices;
	  updates++;
	  L_PCE_global_update (pce_graph);
	  if (pce_graph->num_cut_active == 0)
	    break;
	}
#endif
    }
  visit_nodes = List_reset (visit_nodes);

#if defined DEBUG_PREFLOW_PUSH
  fprintf (stderr, "Visits:\t%d\n", visits);
  fprintf (stderr, "Discharges:\t%d\n", discharges);
  fprintf (stderr, "Global updates:\t%d\n", updates);
#endif
}

static void
L_PCE_reverse_push_preflow (L_PCE_graph * pce_graph)
{
  L_PCE_BB *pce_bb;
  GraphArc arc;
  List visit_nodes = NULL;
  int visits = 0, discharges = 0, until_update, updates = 0;
  L_PCE_BB *source, *sink;

#ifdef DEBUG_REV_PREFLOW_PUSH
  fprintf (stderr, "Beginning preflow-push.\n");
#endif

  /* Populate the list with the next-to-start nodes. */
  List_start (pce_graph->sink->pred);
  while ((arc = (GraphArc) List_next (pce_graph->sink->pred)))
    visit_nodes = List_insert_last (visit_nodes, (L_PCE_BB *) arc->pred->ptr);

#ifdef GLOBAL_UPDATE_HEURISTIC
  until_update = GLOBAL_UPDATE_FACTOR * pce_graph->n_vertices;
#endif
  /* Push preflow until stable. */
  source = (L_PCE_BB *) pce_graph->source->ptr;
  sink = (L_PCE_BB *) pce_graph->sink->ptr;
  List_start (visit_nodes);
  while ((pce_bb = (L_PCE_BB *) List_next (visit_nodes)))
    {
      if ((pce_bb->excess > 0) && (pce_bb != source) && (pce_bb != sink))
	{
	  L_PCE_reverse_discharge (pce_graph, &visit_nodes, pce_bb);
	  discharges++;
#ifdef GLOBAL_UPDATE_HEURISTIC
	  until_update--;
#endif
	}
#ifdef CUT_SHORT_CIRCUIT
      if (pce_bb->height < pce_graph->n_vertices)
	(pce_graph->num_cut_active)--;
      if (pce_graph->num_cut_active == 0)
	break;
#endif
      visits++;
      visit_nodes = List_delete_current (visit_nodes);
#ifdef GLOBAL_UPDATE_HEURISTIC
      if (until_update == 0)
	{
	  until_update = GLOBAL_UPDATE_FACTOR * pce_graph->n_vertices;
	  updates++;
	  L_PCE_reverse_global_update (pce_graph);
	  if (pce_graph->num_cut_active == 0)
	    break;
	}
#endif
    }
  visit_nodes = List_reset (visit_nodes);

#if defined DEBUG_REV_PREFLOW_PUSH
  fprintf (stderr, "Visits:\t%d\n", visits);
  fprintf (stderr, "Discharges:\t%d\n", discharges);
  fprintf (stderr, "Global updates:\t%d\n", updates);
#endif
}


static void
L_SPRE_minimum_cut (L_PCE_graph * pce_graph)
{
  GraphNode node;
  L_PCE_BB *pce_bb;
  List nodes;
  int num_cut_nodes = 1, n_vertices;

  L_PCE_global_update (pce_graph);

#ifdef DEBUG_SPRE_MIN_CUT
  fprintf (stderr, "Beginning minimum s-t cut for SPRE, expression %d.\n",
	   pce_graph->expression_index);
#endif

  nodes = pce_graph->graph->nodes;
  n_vertices = pce_graph->n_vertices;
  List_start (nodes);
  while ((node = (GraphNode) List_next (nodes)))
    {
      pce_bb = (L_PCE_BB *) node->ptr;
      if (pce_bb->height < n_vertices)
	{
	  num_cut_nodes++;
	  pce_bb->pce_type |= BB_PAST_CUT;
	}
    }

#ifdef DEBUG_SPRE_MIN_CUT
  fprintf (stderr, "Minimum cut for expression %d complete.\n",
	   pce_graph->expression_index);
  fprintf (stderr, "BBs cut: %d\n", num_cut_nodes);
#ifdef PRINT_SPRE_GRAPH
  L_print_PCE_graph (pce_graph);
#endif
#endif
}


#ifdef DO_GRAPH_MEMORIZATION
static void
L_PCE_cut_previous (L_PCE_graph * pce_graph, L_PCE_graph_cut * graph_cut)
{
  L_PCE_BB *src_pce_bb, *dest_pce_bb;
  L_BB_arc *bb_arc;
  DF_PCE_INFO *src_pce, *dest_pce;
  GraphNode dest_node;
  GraphArc arc;
  Set taken_br_cut, untaken_br_cut, fallthrough_cbs_cut,
    sink_adjacent_cbs_cut;
  List arcs = pce_graph->graph->arcs;
  int expression_index;

  expression_index = pce_graph->expression_index;
#ifdef DEBUG_SPRE_MIN_CUT
  fprintf (stderr, "Starting previous placement for expression %d.\n",
	   expression_index);
#endif
  taken_br_cut = graph_cut->taken_br_cut;
  untaken_br_cut = graph_cut->untaken_br_cut;
  fallthrough_cbs_cut = graph_cut->fallthrough_cbs_cut;
  sink_adjacent_cbs_cut = graph_cut->sink_adjacent_cbs_cut;

#ifdef TRACK_GRAPH_MEMORIZATION
  /* Statistic tracking. */
  if (Set_in (graph_cut->expressions, expression_index))
    (graph_cut->repeat)++;
  else
    {
      graph_cut->expressions = Set_add (graph_cut->expressions,
					expression_index);
      (graph_cut->share)++;
    }
#endif

  /* Go through edges. */
  List_start (arcs);
  while ((arc = (GraphArc) List_next (arcs)))
    {
      bb_arc = ((L_PCE_arc *) arc->ptr)->bb_arc;
      src_pce_bb = (L_PCE_BB *) arc->pred->ptr;
      if (!bb_arc)
	{
	  if (!src_pce_bb)	/* source edge */
	    continue;
	  else if ((src_pce_bb->pce_type & BB_SINK_ADJACENT) &&
		   Set_in (sink_adjacent_cbs_cut, src_pce_bb->bb->cb->id))
	    {			/* sink edge */
	      src_pce = src_pce_bb->bb->pf_bb->info->pce_info;
	      if (Set_in (src_pce->n_comp, expression_index))
		src_pce->n_latest = Set_add (src_pce->n_latest,
					     expression_index);
	    }
	  continue;
	}
      if (bb_arc->branch)
	if (bb_arc->taken)
	  {
	    if (!Set_in (taken_br_cut, bb_arc->branch->id))
	      continue;
	  }
	else
	  {
	    if (!Set_in (untaken_br_cut, bb_arc->branch->id))
	      continue;
	  }
      else if (!Set_in (fallthrough_cbs_cut, src_pce_bb->bb->cb->id))
	continue;

      dest_node = arc->succ;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      src_pce = src_pce_bb->bb->pf_bb->info->pce_info;
      dest_pce = dest_pce_bb->bb->pf_bb->info->pce_info;

      if (dest_pce_bb->bb->type & BB_SINGLE_ENTRY)
	{
	  if (dest_pce_bb->pce_type & BB_SINK_ADJACENT)
	    {
	      if (Set_in (dest_pce->n_comp, expression_index))
		dest_pce->n_latest = Set_add (dest_pce->n_latest,
					      expression_index);
	    }
	  else
	    {
	      dest_pce->x_latest = Set_add (dest_pce->x_latest,
					    expression_index);
	    }
	  continue;
	}
      else if (src_pce_bb->bb->type & BB_SINGLE_EXIT)
	{
	  src_pce->x_latest = Set_add (src_pce->x_latest, expression_index);
	}
      else if (src_pce_bb->bb->weight < dest_pce_bb->bb->weight)
	{
	  src_pce->x_latest = Set_add (src_pce->x_latest, expression_index);
	}
      else
	{
	  if (dest_pce_bb->pce_type & BB_SINK_ADJACENT)
	    {
	      if (Set_in (dest_pce->n_comp, expression_index))
		dest_pce->n_latest = Set_add (dest_pce->n_latest,
					      expression_index);
	    }
	  else
	    {
	      dest_pce->x_latest = Set_add (dest_pce->x_latest,
					    expression_index);
	    }
	}
    }

#ifdef DEBUG_SPRE_MIN_CUT
  fprintf (stderr, "Previous cut for expression %d complete.\n",
	   expression_index);
#endif
}
#endif


static int
L_SPRE_placement (L_PCE_graph * pce_graph)
{
  L_PCE_BB *src_pce_bb, *dest_pce_bb;
  DF_PCE_INFO *src_pce = NULL, *dest_pce = NULL;
  GraphNode src_node, dest_node;
  GraphArc arc;
  List edges = pce_graph->graph->arcs;
  int expression_index, change = 0;
#ifdef DO_GRAPH_MEMORIZATION
  L_BB_arc *bb_arc;
  Set taken_br_cut, untaken_br_cut, fallthrough_cbs_cut,
    sink_adjacent_cbs_cut;
  L_PCE_graph_cut *graph_cut;

  taken_br_cut = NULL;
  untaken_br_cut = NULL;
  fallthrough_cbs_cut = NULL;
  sink_adjacent_cbs_cut = NULL;
#endif

  expression_index = pce_graph->expression_index;

#ifdef DEBUG_SPRE_PLACEMENT
  fprintf (stderr, "Starting SPRE placement for expression %d.\n",
	   expression_index);
#ifdef PRINT_SPRE_GRAPH
  L_print_PCE_graph (pce_graph);
#endif
#endif

  /* Go through cut edges. */
  List_start (edges);
  while ((arc = (GraphArc) List_next (edges)))
    {
      /* Check if edge is part of min-cut */
      src_node = arc->pred;
      src_pce_bb = (L_PCE_BB *) src_node->ptr;
      if (src_pce_bb->pce_type & BB_PAST_CUT)
	continue;
      dest_node = arc->succ;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      if (!(dest_pce_bb->pce_type & BB_PAST_CUT))
	continue;

#ifdef DEBUG_SPRE_PLACEMENT
      fprintf (stderr, "Cut edge between ");
#endif

#ifdef DO_GRAPH_MEMORIZATION
      /* SPRE graph cut memorization */
      bb_arc = ((L_PCE_arc *) arc->ptr)->bb_arc;
      if (!bb_arc)
	if (dest_node == pce_graph->sink)
	  sink_adjacent_cbs_cut = Set_add (sink_adjacent_cbs_cut,
					   src_pce_bb->bb->cb->id);
	else
	  L_punt ("L_PCE_placement: no support for cut edges that don't "
		  "correspond to actual flowgraph edges.");
      else if (bb_arc->branch)
	if (bb_arc->taken)
	  taken_br_cut = Set_add (taken_br_cut, bb_arc->branch->id);
	else
	  untaken_br_cut = Set_add (untaken_br_cut, bb_arc->branch->id);
      else
	fallthrough_cbs_cut = Set_add (fallthrough_cbs_cut,
				       src_pre_bb->bb->cb->id);
#endif

      if (src_pce_bb->bb)
	{
	  src_pce = src_pce_bb->bb->pf_bb->info->pce_info;
#ifdef DEBUG_SPRE_PLACEMENT
	  fprintf (stderr, "BB (cb %d op %d) and ", src_pce_bb->bb->cb->id,
		   src_pce_bb->bb->first_op ?
		   src_pce_bb->bb->first_op->id : 0);
#endif
	}
#ifdef DEBUG_SPRE_PLACEMENT
      else
	fprintf (stderr, "source and ");
#endif
      if (dest_pce_bb->bb)
	{
	  dest_pce = dest_pce_bb->bb->pf_bb->info->pce_info;
#ifdef DEBUG_SPRE_PLACEMENT
	  fprintf (stderr, "BB (cb %d op %d).\n", dest_pce_bb->bb->cb->id,
		   dest_pce_bb->bb->first_op ?
		   dest_pce_bb->bb->first_op->id : 0);
#endif
	}
#ifdef DEBUG_SPRE_PLACEMENT
      else
	fprintf (stderr, "sink.\n");
#endif

      /* Check for source or sink. */
      if (dest_node == pce_graph->sink)
	{
	  /* insert if it is n_comp, */
	  if (Set_in (src_pce->n_comp, expression_index))
	    {
	      src_pce->n_latest = Set_add (src_pce->n_latest,
					   expression_index);
#ifdef DEBUG_SPRE_PLACEMENT
	      fprintf (stderr, "N Placement of expression %d at cb %d.\n",
		       expression_index, src_pce_bb->bb->cb->id);
#endif
	    }
	  /* reinsert if a complement instruction. */
	  else if (Set_in (src_pce->complement, expression_index) &&
		   Set_in (src_pce->trans, expression_index))
	    {
	      src_pce->x_latest = Set_add (src_pce->x_latest,
					   expression_index);
#ifdef DEBUG_SPRE_PLACEMENT
	      fprintf (stderr, "Cmpl Placement of expression %d at cb %d.\n",
		       expression_index, src_pce_bb->bb->cb->id);
#endif
	    }
	  else /* predicated instruction */
	    ;
	  continue;
	}
      if (src_node == pce_graph->source)
	{
	  if (Set_in (dest_pce->n_comp, expression_index))
	    dest_pce->n_latest =
	      Set_add (dest_pce->n_latest, expression_index);
	  else
	    {
	      dest_pce->x_latest =
		Set_add (dest_pce->x_latest, expression_index);
	      change++;
	    }
#ifdef DEBUG_SPRE_PLACEMENT
	  fprintf (stderr, "X Placement of expression %d at cb %d.\n",
		   expression_index, dest_pce_bb->bb->cb->id);
#endif

	  continue;
	}

      /* Insert at location where it minimizes weight. */
      if (dest_pce_bb->bb->type & BB_SINGLE_ENTRY)
	{			/* We need to check for sink adjacency: we don't want to insert
				 * if sink-adjacent but not n-comp, because it's a predicated
				 * instruction. */
	  if (dest_pce_bb->pce_type & BB_SINK_ADJACENT)
	    {			/* if not n_comp, predicated op, so don't insert */
	      if (Set_in (dest_pce->n_comp, expression_index))
		dest_pce->n_latest = Set_add (dest_pce->n_latest,
					      expression_index);
	    }
	  else
	    {
	      dest_pce->x_latest = Set_add (dest_pce->x_latest,
					    expression_index);
	      change++;
	    }
#ifdef DEBUG_SPRE_PLACEMENT
	  fprintf (stderr, "Placement of expression %d at cb %d.\n",
		   expression_index, dest_pce_bb->bb->cb->id);
#endif
	  continue;
	}
      else if (src_pce_bb->bb->type & BB_SINGLE_EXIT)
	{
	  src_pce->x_latest = Set_add (src_pce->x_latest, expression_index);
	  change++;
#ifdef DEBUG_SPRE_PLACEMENT
	  fprintf (stderr, "Placement of expression %d at cb %d.\n",
		   expression_index, src_pce_bb->bb->cb->id);
	  if (!Set_in (src_pce->nd_safe, expression_index) &&
	      Set_in (src_pce->nd_safe, expression_index))
	    fprintf (stderr, "Inserting unnecessary speculation! "
		     "Notify SER.\n");
#endif
	  continue;
	}
      else if (src_pce_bb->bb->weight < dest_pce_bb->bb->weight)
	{
	  fprintf (stderr, "Critical edge found; notify SER.\n");
	  src_pce->x_latest = Set_add (src_pce->x_latest, expression_index);
	  change++;
#ifdef DEBUG_SPRE_PLACEMENT
	  fprintf (stderr, "Placement of expression %d at cb %d.\n",
		   expression_index, src_pce_bb->bb->cb->id);
	  if (!Set_in (src_pce->nd_safe, expression_index) &&
	      Set_in (src_pce->nd_safe, expression_index))
	    fprintf (stderr, "Inserting unnecessary speculation! "
		     "Notify SER.\n");
#endif
	}
      else
	{
	  fprintf (stderr, "Critical edge found; notify SER.\n");
	  if (dest_pce_bb->pce_type & BB_SINK_ADJACENT)
	    {
	      if (Set_in (dest_pce->n_comp, expression_index))
		dest_pce->n_latest = Set_add (dest_pce->n_latest,
					      expression_index);
	    }
	  else
	    {
	      dest_pce->x_latest = Set_add (dest_pce->x_latest,
					    expression_index);
	      change++;
	    }

#ifdef DEBUG_SPRE_PLACEMENT
	  fprintf (stderr, "Placement of expression %d at cb %d.\n",
		   expression_index, dest_pce_bb->bb->cb->id);
#endif
	}
    }

#ifdef DO_GRAPH_MEMORIZATION
  graph_cut = L_new_graph_cut (pce_graph);
  graph_cut->taken_br_cut = taken_br_cut;
  graph_cut->untaken_br_cut = untaken_br_cut;
  graph_cut->fallthrough_cbs_cut = fallthrough_cbs_cut;
  graph_cut->sink_adjacent_cbs_cut = sink_adjacent_cbs_cut;
#endif

#ifdef DEBUG_SPRE_PLACEMENT
  fprintf (stderr, "Placement for expression %d complete",
	   expression_index);
  if (change)
    fprintf (stderr, ", motion performed.\n");
  else
    fprintf (stderr, "\n");
#endif
  return change;
}


void 
L_SPRE_reinsert_computations (PRED_FLOW * pred_flow, Set * remove)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  DF_PCE_INFO *pce;
  Set temp, temp2;

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
	{
	  pce = pf_bb->info->pce_info;

	  /* Reinsert n_comps to n_latest. */
	  temp = Set_intersect (pce->n_comp, *remove);
	  temp = Set_subtract_acc (temp, pce->nu_safe);
	  pce->n_latest = Set_union_acc (pce->n_latest, temp);
	  Set_dispose (temp);

	  /* Reinsert x_comps to x_latest. Remove those which are
	   complement, transparent, and upsafe. */
	  temp2 = Set_intersect (pce->complement, pce->trans);
	  temp2 = Set_intersect_acc (temp2, pce->nu_safe);
	  temp = Set_intersect (pce->x_comp, *remove);
	  temp = Set_subtract_acc (temp, temp2);
	  pce->x_latest = Set_union_acc (pce->x_latest, temp);
	  Set_dispose (temp);
	  Set_dispose (temp2);
	}
    }
}


void
L_PRE_speculate_motion (Graph bb_weighted_graph, PRED_FLOW * pred_flow,
			Set *motion, int mem_conservative, int max_motions)
{
  int i, size, *expressions, motions = 0;
  L_PCE_graph *pce_graph;
#ifdef DO_GRAPH_MEMORIZATION
  L_PCE_graph_cut *graph_cut;
#endif

  expressions = (int *) Lcode_malloc (sizeof (int) * L_fn->n_expression);
  size = Set_2array (*motion, expressions);

  /* Prep graph cut hash. */
  if (!(L_PCE_graph_cut_tbl))
    L_PCE_graph_cut_tbl = (L_PCE_graph_cut **)
      calloc (PCE_GRAPH_TBL_SIZE, sizeof (L_PCE_graph_cut *));

  for (i = 0; i < size; i++)
    {
#ifdef DEBUG_SPRE_MIN_CUT
      fprintf (stderr, "Beginning PRE motion analysis for expression %d.\n",
	       expressions[i]);
#endif
      pce_graph = L_construct_SPRE_graph (bb_weighted_graph, expressions[i],
					  mem_conservative);
      /* check if graph is non-trivial */
      if (pce_graph->n_vertices > 2)
	{			/* check if graph already exists */
#ifdef DO_GRAPH_MEMORIZATION
	  L_punt ("PCE graph memorization non-functional.");
	  if (graph_cut = L_PCE_find_graph_cut (pce_graph) * /)
	    L_PCE_cut_previous (pce_graph, graph_cut, expressions[i]);
	  else
	    {
#endif
#ifdef PCE_PROFITABILITY_METRIC
	      L_PCE_reg_pressure_reweighting (pce_graph, 0);
#endif
	      L_PCE_push_preflow (pce_graph);
	      L_SPRE_minimum_cut (pce_graph);
	      if (L_SPRE_placement (pce_graph))
		motions++;
#ifdef DO_GRAPH_MEMORIZATION
	    }
#endif
	}
      L_delete_PCE_graph (pce_graph);
      if (motions >= max_motions)
	{
	  Set remove = NULL;
	  i++;
	  while (i < size)
	    {
	      /* Need to set n_latest, x_latest at n_comp, x_comp locations. */
	      remove = Set_add (remove, expressions[i]);
	      *motion = Set_delete (*motion, expressions[i]);
	      i++;
	    }
#if DEBUG_SPRE_MIN_CUT
	  fprintf (stderr, "Ending SPRE motions: too many live vars.\n");
	  Set_print (stderr, "Remove: ", remove);
#endif
	  L_SPRE_reinsert_computations (pred_flow, &remove);
	  Set_dispose (remove);
	  break;
	}
    }
  Lcode_free (expressions);
}


void
L_do_P3DE_analysis (L_Func * fn, int mode, int ignore)
{
  int dead_code;

  L_start_time (&L_module_global_dataflow_time);

  if (!(mode & SUPPRESS_PG))
    D_setup_dataflow (fn, PF_ALL_OPERANDS);
  else
    D_setup_dataflow (fn, PF_ALL_OPERANDS | PF_SUPPRESS_PRED_GRAPH);

  D_setup_BB_lists (PF_default_flow);

  D_PDE_predicated_cut_analysis (PF_default_flow, mode, ignore);

  L_stop_time (&L_module_global_dataflow_time);

  dead_code = D_delete_DF_dead_code (fn);
}


int
L_PDE_fast_assignment (L_Expression * assignment)
{
  if (!(assignment->src[1]) || L_int_add_opcode (assignment) ||
      L_int_sub_opcode (assignment) || L_logic_opcode (assignment) ||
      (L_shift_opcode (assignment) && L_is_constant (assignment->src[1])) ||
      L_general_pred_comparison_opcode (assignment))
    return 1;
  else
    return 0;
}


static L_PCE_graph *
L_construct_P3DE_graph (Graph bb_weighted_graph, int assignment_index)
{
  GraphNode src_node, dest_node, src_pce_node, dest_pce_node, source, sink;
  GraphArc arc, new_arc;
  Graph graph;
  L_BB *src_bb, *dest_bb;
  L_BB_arc *bb_arc;
  L_PCE_BB *src_pce_bb, *dest_pce_bb, *source_pce_bb, *sink_pce_bb;
  L_PCE_arc *new_pce_arc;
  L_PCE_graph *ppde_graph;
  L_Expression *assignment;
  DF_PCE_INFO *src_pce, *dest_pce;
  int n_vertices = 2, n_edges = 0, num_cut_active = 0, combine_flag = 0,
    comp_points = 0;
#ifdef DO_GRAPH_MEMORIZATION
  Set taken_br_edges = NULL;
  Set untaken_br_edges = NULL;
  Set fallthrough_cbs = NULL;
#endif

  /* Initialize graph, add start and sink nodes. */
  ppde_graph = malloc (sizeof (L_PCE_graph));
  graph = Graph_create_graph ();
  ppde_graph->graph = graph;
  source = Graph_create_graph_node (graph);
  source_pce_bb = L_new_PCE_bb (source, NULL);
  source_pce_bb->index = 0;
  source->ptr = source_pce_bb;
  GraphRootNode (graph) = source;
  sink = Graph_create_graph_node (graph);
  sink_pce_bb = L_new_PCE_bb (sink, NULL);
  sink_pce_bb->index = 1;
  sink->ptr = sink_pce_bb;
  ppde_graph->source = source;
  ppde_graph->sink = sink;
  ppde_graph->expression_index = assignment_index;
  ppde_graph->visit_nodes = NULL;

  /* For store ops, we want to only do non-predicated motion.
   * May want to include other operations as well at a later time. */
  assignment = (L_expression_hash_tbl_find_entry
		(L_fn->expression_index_hash_tbl,
		 assignment_index))->expression;
  if (L_store_opcode (assignment) && !(assignment->src[2]))
    combine_flag = 1;

#ifdef DEBUG_P3DE_CONSTRUCT
  fprintf (stderr, "Constructing P3DE graph for assignment %d.\n",
	   assignment_index);
#endif

  /* Walk through BBs. */
  List_start (bb_weighted_graph->nodes);
  while ((src_node = (GraphNode) List_next (bb_weighted_graph->nodes)))
    {
      /* For each BB, check if partially sinkable (x_spec_us) and partially
       * dead for assignment, continue if not. */
      src_bb = (L_BB *) GraphContents (src_node);
#ifdef DEBUG_P3DE_CONSTRUCT
      fprintf (stderr, "Checking outbound edges for BB (cb %d, first_op "
	       "%d).\n", src_bb->cb->id, (src_bb->first_op) ?
	       src_bb->first_op->id : 0);
#endif
      src_pce = src_bb->pf_bb->info->pce_info;

      /* A basic block must be partially delayable and partially dead
       * to contain outbound edges in the graph. */
      if (!(Set_in (src_pce->x_spec_us, assignment_index) &&
	    Set_in (src_pce->x_spec_ds, assignment_index)))
	continue;

      /* Check each outbound edge: if not completely dead at dest and
       * still sinkable, add bb, edges, and dest_bb. */
      List_start (src_node->succ);
      while ((arc = (GraphArc) List_next (src_node->succ)))
	{
	  int new_flag = 0;
	  dest_node = arc->succ;
	  dest_bb = (L_BB *) GraphContents (dest_node);
	  bb_arc = (L_BB_arc *) GraphArcContents (arc);
#ifdef DEBUG_P3DE_CONSTRUCT
	  if (bb_arc->branch)
	    fprintf (stderr, "\tArc corresponding to branch op %d, dest cb "
		     "%d.\n", bb_arc->branch->id, dest_bb->cb->id);
	  else
	    fprintf (stderr, "\tArc corresponding to fall-through from op %d "
		     "to dest BB at op %d, cb %d.\n",
		     src_bb->last_op ? src_bb->last_op->id : 0,
		     (dest_bb->first_op) ? dest_bb->first_op->id : 0,
		     dest_bb->cb->id);
#endif
	  dest_pce = dest_bb->pf_bb->info->pce_info;
	  if (Set_in (dest_pce->n_isolated, assignment_index) ||
	      !Set_in (dest_pce->n_spec_us, assignment_index))
	    continue;

#ifdef DEBUG_P3DE_CONSTRUCT
	  fprintf (stderr, "\t\tADDING ARC.\n");
#endif

#ifdef DO_GRAPH_MEMORIZATION
	  if (bb_arc->branch)
	    if (bb_arc->taken)
	      taken_br_edges = Set_add (taken_br_edges, bb_arc->branch->id);
	    else
	      untaken_br_edges =
		Set_add (untaken_br_edges, bb_arc->branch->id);
	  else
	    fallthrough_cbs = Set_add (fallthrough_cbs, src_bb->cb);
#endif

	  /* For loc_blocked nodes, we need to define
	   * an exit L_PCE_BB for correctness. */
	  if (Set_in (src_pce->n_comp, assignment_index))
	    {
	      if (!(src_pce_bb = (L_PCE_BB *) src_bb->ptr_exit))
		{
#ifdef DEBUG_P3DE_CONSTRUCT
		  fprintf (stderr, "\t\tAdding exit src BB first op %d, "
			   "cb %d.\n", src_bb->first_op ?
			   src_bb->first_op->id : 0, src_bb->cb->id);
#endif
		  new_flag = 1;
		  src_pce_node = Graph_create_graph_node (graph);
		  src_pce_bb = L_new_PCE_bb (src_pce_node, src_bb);
		  src_pce_bb->pce_type |= BB_EXIT;
		  src_pce_bb->index = n_vertices++;
		  GraphNodeContents (src_pce_node) = src_pce_bb;
		  src_bb->ptr_exit = src_pce_bb;
		}
	      else
		src_pce_node = src_pce_bb->node;
	    }
	  else
	    {
	      if (!(src_pce_bb = (L_PCE_BB *) src_bb->ptr))
		{
#ifdef DEBUG_P3DE_CONSTRUCT
		  fprintf (stderr, "\t\tAdding src BB first op %d, cb "
			   "%d.\n", src_bb->first_op ?
			   src_bb->first_op->id : 0, src_bb->cb->id);
#endif
		  new_flag = 1;
		  src_pce_node = Graph_create_graph_node (graph);
		  src_pce_bb = L_new_PCE_bb (src_pce_node, src_bb);
		  src_pce_bb->index = n_vertices++;
		  GraphNodeContents (src_pce_node) = src_pce_bb;
		  src_bb->ptr = src_pce_bb;
		}
	      else
		src_pce_node = src_pce_bb->node;
	    }

	  /* If the assignment is computed in the BB, add arc from source
	   * to node. */
#ifdef DEBUG_P3DE_CONSTRUCT
	  Set_print (stderr, "LOC_SINK", src_pce->x_insert);
	  Set_print (stderr, "STORE_SINK", src_pce->x_latest);
#endif
	  if (new_flag &&
	      ((!combine_flag && Set_in (src_pce->x_insert, assignment_index))
	       || (combine_flag
		   && Set_in (src_pce->x_latest, assignment_index))))
	    {
	      unsigned int weight;
	      new_arc = Graph_connect_nodes (graph, source, src_pce_node);
	      new_pce_arc = L_new_PCE_arc (new_arc, NULL);
	      new_arc->ptr = new_pce_arc;
	      weight = src_pce_bb->bb->weight;
	      new_pce_arc->weight = weight;
	      n_edges++;
	      comp_points++;

#ifdef DO_GRAPH_MEMORIZATION
	      List_start (src_pce_node->succ);
	      while (arc = (GraphArc) List_next (src_pce_node->succ))
		{
		  bb_arc = (L_BB_arc *) GraphArcContents (arc);
		  if (bb_arc->branch)
		    if (bb_arc->taken)
		      taken_br_edges = Set_add (taken_br_edges,
						bb_arc->branch->id);
		    else
		      untaken_br_edges = Set_add (untaken_br_edges,
						  bb_arc->branch->id);
		  else
		    fallthrough_cbs = Set_add (fallthrough_cbs,
					       src_pce_bb->bb->cb);
		}
#endif
	    }

	  if (!(dest_pce_bb = (L_PCE_BB *) dest_bb->ptr))
	    {
#ifdef DEBUG_P3DE_CONSTRUCT
	      fprintf (stderr, "\t\tAdding dest BB first op %d, cb %d.\n",
		       dest_bb->first_op ? dest_bb->first_op->id : 0,
		       dest_bb->cb->id);
#endif
	      dest_pce_node = Graph_create_graph_node (graph);
	      dest_pce_bb = L_new_PCE_bb (dest_pce_node, dest_bb);
	      dest_pce_bb->index = n_vertices++;
	      GraphNodeContents (dest_pce_node) = dest_pce_bb;
	      dest_bb->ptr = dest_pce_bb;
	    }
	  else
	    dest_pce_node = dest_pce_bb->node;
	  new_arc = Graph_connect_nodes (graph, src_pce_node, dest_pce_node);
	  new_pce_arc = L_new_PCE_arc (new_arc, bb_arc);
	  GraphArcContents (new_arc) = new_pce_arc;
	  bb_arc->ptr = new_pce_arc;
	  n_edges++;
#ifdef SPLIT_EDGE_METRIC
	  if (src_bb->type & BB_REMOVABLE || dest_bb->type & BB_REMOVABLE)
	    {
	      if (new_pce_arc->weight > ITMAXU32 / SPLIT_EDGE_FACTOR)
		{
		  fprintf (stderr, "P3DE: Split edge metric overflow.\n");
		  new_pce_arc->weight = UNSIGNED_INT_MAX;
		}
	      else
		new_pce_arc->weight *= SPLIT_EDGE_FACTOR;
	    }
#endif
	}
    }

  if (comp_points == 0 && n_vertices != 2)
    {
      DB_spit_func (L_fn, "bad_file");
      L_punt ("No computation points found for assignment %d!",
	      assignment_index);
    }

  ppde_graph->n_vertices = n_vertices;

#ifdef DO_GRAPH_MEMORIZATION
  /* Link up edges sets to SPRE graph. */
  ppde_graph->taken_br_edges = taken_br_edges;
  ppde_graph->untaken_br_edges = untaken_br_edges;
  ppde_graph->fallthrough_cbs = fallthrough_cbs;
  /* Clear for exit info. */
  taken_br_edges = NULL;
  untaken_br_edges = NULL;
  fallthrough_cbs = NULL;
#endif

  /* Link subgraphs to form single source/sink graph.
   * Initialize adjacent-to-sink nodes. */
  sink_pce_bb->height = n_vertices;
  List_start (graph->nodes);
  while ((src_node = (GraphNode) List_next (graph->nodes)))
    {
      if (src_node == source || src_node == sink)
	continue;

      /* If no outgoing arcs, add arc from node to sink. */
      if (!((GraphArc) List_first (src_node->succ)))
	{
	  unsigned int weight;
	  src_pce_bb = (L_PCE_BB *) src_node->ptr;
	  new_arc = Graph_connect_nodes (graph, src_node, sink);
	  new_pce_arc = L_new_PCE_arc (new_arc, NULL);
	  new_arc->ptr = new_pce_arc;
	  weight = ((L_PCE_BB *) src_node->ptr)->bb->weight;
#ifdef SPLIT_EDGE_METRIC
	  if (src_pce_bb->bb->type & BB_REMOVABLE)
	    {
	      if (weight > ITMAXU32 / SPLIT_EDGE_FACTOR)
		{
		  fprintf (stderr, "P3DE: Split edge metric overflow.\n");
		  weight = UNSIGNED_INT_MAX;
		}
	      else
		weight *= SPLIT_EDGE_FACTOR;
	    }
#endif
	  new_pce_arc->weight = weight;
	  new_pce_arc->flow = weight;
	  src_pce_bb->excess = weight;
	  src_pce_bb->height = 1;
	  n_edges++;
	  num_cut_active++;
	}
    }

  ppde_graph->n_edges = n_edges;
  ppde_graph->num_cut_active = num_cut_active;

#ifdef DEBUG_P3DE_CONSTRUCT
  L_print_PCE_graph (ppde_graph);
#endif

  return ppde_graph;
}


static void
L_P3DE_reaching_assignments (L_PCE_graph * ppde_graph)
{
  L_Expression *assignment;
  int change, *reach_array;
  GraphNode source, sink, node, pred_node;
  GraphArc arc;
  L_PCE_BB *pce_bb, *pce_bb_other;
  DF_PCE_INFO *pce;
  List nodes;

  assignment = (L_expression_hash_tbl_find_entry
		(L_fn->expression_index_hash_tbl,
		 ppde_graph->expression_index))->expression;
  change = Set_size (assignment->associates);
  if (change == 0)
    L_punt ("Number of associates of a general store should not be 0!");
  reach_array = calloc (change, sizeof (int));

  source = ppde_graph->source;
  sink = ppde_graph->sink;
  nodes = ppde_graph->graph->nodes;

  List_start (source->succ);
  while ((arc = (GraphArc) List_next (source->succ)))
    {
      node = arc->succ;
      pce_bb = (L_PCE_BB *) node->ptr;
      pce = pce_bb->bb->pf_bb->info->pce_info;
#ifdef DEBUG_P3DE_GENERAL_STORE_MOTION
      fprintf (stderr, "BB %d (cb %d):\n", pce_bb->index, pce_bb->bb->cb->id);
      Set_print (stderr, "\tgenerate:", pce->x_latest);
      Set_print (stderr, "\tassociate:", assignment->associates);
#endif
      pce_bb->reaching_assn = Set_intersect (pce->x_latest,
					     assignment->associates);
#ifdef DEBUG_P3DE_GENERAL_STORE_MOTION
      Set_print (stderr, "\treaching:", pce_bb->reaching_assn);
      if (Set_empty (pce_bb->reaching_assn))
	L_punt ("Empty generation set for store assignment %d!\n",
		ppde_graph->expression_index);
      else if (1 != (change = Set_size (pce_bb->reaching_assn)))
	{
	  Set_print (stderr, "local defs:", pce_bb->reaching_assn);
	  L_punt ("Multiple local defs of same expression at cb %d!",
		  pce_bb->bb->cb->id);
	}
#endif
    }

  /* Find reaching assignments. */
  do
    {
      int set_change = 0;
      change = 0;
      List_start (nodes);
      while ((node = (GraphNode) List_next (nodes)))
	{
	  if (node == source || node == sink)
	    continue;
	  pce_bb = (L_PCE_BB *) node->ptr;
	  List_start (node->pred);
	  while ((arc = (GraphArc) List_next (node->pred)))
	    {
	      pred_node = arc->pred;
	      if (pred_node == source)
		continue;
	      pce_bb_other = (L_PCE_BB *) pred_node->ptr;
	      pce_bb->reaching_assn =
		Set_union_acc_ch (pce_bb->reaching_assn,
				  pce_bb_other->reaching_assn, &set_change);
	      if (set_change)
		{
		  int size;
		  change++;
		  if (pce_bb->pce_type & BB_COMBINE)
		    continue;
		  if ((size = Set_2array (pce_bb->reaching_assn,
					  reach_array)) > 1)
		    {
		      pce_bb->pce_type |= BB_COMBINE;
		    }
		  else
		    if (!Set_in
			(pce_bb->bb->pf_bb->info->pce_info->n_spec_us,
			 reach_array[0]))
		    {
		      pce_bb->pce_type |= BB_COMBINE;
		    }
		}
	    }
	}
    }
  while (change);

  free (reach_array);
}


/* This function adds cost edges to P3DE graph edges for two reasons:
 * 1. Additional pred clears that would be incurred by paths that lead
 *    into the min-cut graph.
 * 2. Pred clears that would be incurred by cutting inside a loop.
 */
static void
L_P3DE_penalty_reweighting (L_PCE_graph * ppde_graph, int no_pred)
{
  L_PCE_BB *dest_pce_bb, *src_pce_bb;
  L_PCE_arc *pce_arc, *new_pce_arc;
  L_BB *src_bb, *dest_bb;
  L_Expression *assignment;
  DF_PCE_INFO *pce;
  GraphNode dest_node, dest_node_main, src_node, source, sink;
  GraphArc arc, new_arc;
  List nodes;
  int change, a_index, i, safe_flag = 0, combine_flag = 0, break_flag;
  unsigned int weight;
  Set BB_U = NULL, pred_dom;

  nodes = ppde_graph->graph->nodes;
  a_index = ppde_graph->expression_index;
  source = ppde_graph->source;
  sink = ppde_graph->sink;
  assignment = (L_expression_hash_tbl_find_entry
		(L_fn->expression_index_hash_tbl, a_index))->expression;
#ifdef DEBUG_PENALTY_METRIC
  fprintf (stderr, "Beginning penalty metric for P3DE assignment %d.\n",
	   a_index);
#ifdef PRINT_P3DE_GRAPH
  L_print_PCE_graph (ppde_graph);
#endif
  L_print_expression (stderr, assignment);
#endif

  /* We don't want to predicate cheap instructions: single-source opers. */
  if (L_PDE_fast_assignment (assignment))
    safe_flag = 1;
  /* If dealing with general store assignment, pred clear generation
     is handled differently. */
  if (L_store_opcode (assignment) && !(assignment->src[0]))
    {
      combine_flag = 1;
    }

  for (i = 0; i < ppde_graph->n_vertices; i++)
    BB_U = Set_add (BB_U, i);

#ifdef DEBUG_PENALTY_METRIC
  fprintf (stderr, "Beginning dominator analysis.\n");
#endif

  /* Initialize for dominator analysis. */
  List_start (nodes);
  while ((dest_node = (GraphNode) List_next (nodes)))
    {
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      Set_dispose (dest_pce_bb->reaching_BB);
      if (dest_node == source)
	{
	  dest_pce_bb->reaching_BB = NULL;
	  dest_pce_bb->reaching_BB = Set_add (dest_pce_bb->reaching_BB, 0);
	}
      else
	{
	  dest_pce_bb->reaching_BB = Set_copy (BB_U);
	}
    }

  /* Perform dominator analysis. */
  do
    {
      change = 0;
      List_start (nodes);
      while ((dest_node = (GraphNode) List_next (nodes)))
	{
	  if (dest_node == source)
	    continue;
	  dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
	  List_start (dest_node->pred);
	  pred_dom = Set_copy (BB_U);
	  List_start (dest_node->pred);
	  while ((arc = (GraphArc) List_next (dest_node->pred)))
	    {
	      src_node = arc->pred;
	      src_pce_bb = (L_PCE_BB *) src_node->ptr;
	      pred_dom =
		Set_intersect_acc (pred_dom, src_pce_bb->reaching_BB);
	    }
	  pred_dom = Set_add (pred_dom, dest_pce_bb->index);
	  if (!Set_subtract_empty (dest_pce_bb->reaching_BB, pred_dom))
	    change++;
	  Set_dispose (dest_pce_bb->reaching_BB);
	  dest_pce_bb->reaching_BB = pred_dom;
	}
    }
  while (change);
  Set_dispose (BB_U);
  BB_U = NULL;

#ifdef DEBUG_P3DE_DOM
  List_start (nodes);
  while (dest_node = (GraphNode) List_next (nodes))
    {
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      fprintf (stderr, "Dominating nodes for node %d:\n", dest_pce_bb->index);
      Set_print (stderr, "DOM:", dest_pce_bb->reaching_BB);
    }
#endif

  /* For each edge that is a loopback, switch directions. */
  List_start (ppde_graph->graph->arcs);
  while ((arc = (GraphArc) List_next (ppde_graph->graph->arcs)))
    {
      src_node = arc->pred;
      src_pce_bb = (L_PCE_BB *) src_node->ptr;
      dest_node = arc->succ;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;

      if (!Set_in (src_pce_bb->reaching_BB, dest_pce_bb->index))
	continue;
      pce_arc = (L_PCE_arc *) GraphArcContents (arc);
      if (pce_arc->pce_type & ARC_COST)
	continue;

#ifdef DEBUG_PENALTY_METRIC
      fprintf (stderr, "Found loopback edge from BB %d (CB %d) to BB %d "
	       "(CB %d).\n", src_pce_bb->index, src_pce_bb->bb->cb->id,
	       dest_pce_bb->index, dest_pce_bb->bb->cb->id);
#endif

      /* Create new connection in opposite direction. */
      new_arc = Graph_connect_nodes (ppde_graph->graph, dest_node, src_node);
      new_pce_arc = L_new_PCE_arc (new_arc, NULL);
      new_pce_arc->bb_arc = pce_arc->bb_arc;
      GraphArcContents (new_arc) = new_pce_arc;
      if (safe_flag)
	new_pce_arc->weight = UNSIGNED_INT_MAX;
      else
	new_pce_arc->weight = pce_arc->weight / PENALTY_LOOP_FACTOR;
      new_pce_arc->pce_type |= ARC_COST;
      pce_arc->pce_type |= ARC_LOOPBACK;
      ppde_graph->n_edges++;

      /* Delete original connection: make weight 0. */
      pce_arc->weight = 0;
    }
  if (safe_flag)
    return;

  /* Now perform reweighting for incoming arcs to motion graph. */
  List_start (nodes);
  while ((dest_node = (GraphNode) List_next (nodes)))
    {
      if ((dest_node == source) || (dest_node == sink))
	continue;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      dest_bb = dest_pce_bb->bb;
      pce = dest_bb->pf_bb->info->pce_info;

      /* Check all LocalDelayed BBs, draw arcs if not available. */
      if (Set_in (pce->x_insert, a_index))
	{
	  if (Set_in (pce->xu_safe, a_index))
	    continue;
	  dest_node_main = dest_bb->node;
	  weight = 0;
	  List_start (dest_node_main->pred);
	  while ((arc = (GraphArc) List_next (dest_node_main->pred)))
	    {
	      src_node = arc->pred;
	      src_bb = (L_BB *) GraphContents (src_node);
	      pce = src_bb->pf_bb->info->pce_info;
	      if (Set_in (pce->x_spec_us, a_index))
		continue;
	      if (!no_pred)
		weight += (((L_BB_arc *) arc->ptr)->weight) /
		  PENALTY_PRED_FACTOR;
	      else
		weight += (((L_BB_arc *) arc->ptr)->weight) /
		  PENALTY_LOAD_FACTOR;
	    }
	  if (weight == 0)
	    continue;
	  /* If edge to sink already exists, add weight to edge. */
	  break_flag = 0;
	  List_start (dest_node->succ);
	  while ((arc = (GraphArc) List_next (dest_node->succ)))
	    {
	      if (arc->succ == sink)
		{
#ifdef DEBUG_PENALTY_METRIC
		  fprintf (stderr, "Reweighting arc to sink from BB %d "
			   "(CB %d) by %u.\n",
			   dest_pce_bb->index, dest_pce_bb->bb->cb->id,
			   weight);
#endif
		  pce_arc = (L_PCE_arc *) GraphArcContents (arc);
		  if ((pce_arc->weight + weight) > ITMAXU32)
		    {
		      pce_arc->weight = ITMAXU32;
		      pce_arc->flow = UNSIGNED_INT_MAX;
		      dest_pce_bb->excess = UNSIGNED_INT_MAX;
		    }
		  else
		    {
		      pce_arc->weight += weight;
		      pce_arc->flow += weight;
		      dest_pce_bb->excess += weight;
		    }
		  break_flag = 1;
		}
	    }
	  if (break_flag)
	    continue;
	  /* Otherwise draw cost arc. */
#ifdef DEBUG_PENALTY_METRIC
	  fprintf (stderr, "Creating arc to sink from BB %d (CB %d) of %u.\n",
		   dest_pce_bb->index, dest_pce_bb->bb->cb->id, weight);
#endif
	  new_arc = Graph_connect_nodes (ppde_graph->graph, dest_node, sink);
	  pce_arc = L_new_PCE_arc (new_arc, NULL);
	  GraphArcContents (new_arc) = pce_arc;
	  pce_arc->pce_type |= ARC_COST;
	  pce_arc->weight = weight;
	  pce_arc->flow = weight;
	  ppde_graph->n_edges++;
	  dest_pce_bb->height = 1;
	  ppde_graph->num_cut_active++;
	  dest_pce_bb->excess = weight;
	}
      /* Check all non-LocalDelayed BB, draw arcs representing incoming
         edges which are not in motion graph. */
      else
	{
	  weight = 0;
	  dest_node_main = dest_bb->node;
	  List_start (dest_node_main->pred);
	  while ((arc = (GraphArc) List_next (dest_node_main->pred)))
	    {
	      src_node = arc->pred;
	      src_bb = (L_BB *) GraphContents (src_node);
	      pce = src_bb->pf_bb->info->pce_info;
	      if (Set_in (pce->x_spec_us, a_index))
		continue;
	      if (!no_pred)
		weight += (((L_BB_arc *) arc->ptr)->weight) /
		  PENALTY_PRED_FACTOR;
	      else
		weight += (((L_BB_arc *) arc->ptr)->weight) /
		  PENALTY_LOAD_FACTOR;
	    }
	  if (weight == 0)
	    continue;
	  /* If edge to sink already exists, add weight to edge. */
	  break_flag = 0;
	  List_start (dest_node->succ);
	  while ((arc = (GraphArc) List_next (dest_node->succ)))
	    {
	      if (arc->succ == sink)
		{
#ifdef DEBUG_PENALTY_METRIC
		  fprintf (stderr, "Reweighting arc to sink from BB %d "
			   "(CB %d) by %u.\n",
			   dest_pce_bb->index, dest_pce_bb->bb->cb->id,
			   weight);
#endif
		  pce_arc = (L_PCE_arc *) GraphArcContents (arc);
		  if ((pce_arc->weight + weight) > ITMAXU32)
		    {
		      pce_arc->weight = UNSIGNED_INT_MAX;
		      pce_arc->flow = UNSIGNED_INT_MAX;
		      dest_pce_bb->excess = UNSIGNED_INT_MAX;
		    }
		  else
		    {
		      pce_arc->weight += weight;
		      pce_arc->flow += weight;
		      dest_pce_bb->excess += weight;
		    }
		  break_flag = 1;
		}
	    }
	  if (break_flag)
	    continue;
	  /* Otherwise draw cost arc. */
#ifdef DEBUG_PENALTY_METRIC
	  fprintf (stderr, "Creating arc to sink from BB %d (CB %d) of %u.\n",
		   dest_pce_bb->index, dest_pce_bb->bb->cb->id, weight);
#endif
	  new_arc = Graph_connect_nodes (ppde_graph->graph, dest_node, sink);
	  pce_arc = L_new_PCE_arc (new_arc, NULL);
	  GraphArcContents (new_arc) = pce_arc;
	  pce_arc->pce_type |= ARC_COST;
	  pce_arc->weight = weight;
	  pce_arc->flow = weight;
	  ppde_graph->n_edges++;
	  dest_pce_bb->height = 1;
	  ppde_graph->num_cut_active++;
	  dest_pce_bb->excess = weight;
	}
    }

#if defined DEBUG_PENALTY_METRIC && defined PRINT_P3DE_GRAPH
  L_print_PCE_graph (ppde_graph);
#endif
}


static void
L_P3DE_minimum_cut (L_PCE_graph * ppde_graph)
{
  GraphNode node;
  L_PCE_BB *pce_bb;
  List nodes;
  int num_cut_nodes = 1, n_vertices;

  L_PCE_reverse_global_update (ppde_graph);

#ifdef DEBUG_P3DE_MIN_CUT
  fprintf (stderr, "Beginning minimum s-t cut for P3DE, assignment %d.\n",
	   ppde_graph->expression_index);
#endif

  nodes = ppde_graph->graph->nodes;
  n_vertices = ppde_graph->n_vertices;
  List_start (nodes);
  while ((node = (GraphNode) List_next (nodes)))
    {
      pce_bb = (L_PCE_BB *) node->ptr;
      if (pce_bb->height < n_vertices)
	{
	  num_cut_nodes++;
	  pce_bb->pce_type |= BB_PAST_CUT;
	}
    }

#ifdef DEBUG_P3DE_MIN_CUT
  fprintf (stderr, "Minimum cut for assignment %d complete.\n",
	   ppde_graph->expression_index);
  fprintf (stderr, "BBs cut: %d\n", num_cut_nodes);
#ifdef PRINT_P3DE_GRAPH
  L_print_PCE_graph (ppde_graph);
#endif
#endif
}


/* SER 20041213
 * This function takes a general store (store with no specific
 * source) and propagates combine flags at the new locations upwards in the
 * CFG.  Old store locations require a move from the old source to the new
 * source in order to maintain correctness if they have a combine flag set.
 */
static void
L_P3DE_propagate_combine_flag (L_Expression * assignment,
			       L_PCE_graph * ppde_graph)
{
  List nodes, prop_nodes = NULL;
  GraphNode source, sink, src_node, dest_node;
  GraphArc arc;
  L_PCE_BB *src_pce_bb, *dest_pce_bb;
  int a_index, *reach_array, size;

  a_index = ppde_graph->expression_index;
  nodes = ppde_graph->graph->nodes;
  source = ppde_graph->source;
  sink = ppde_graph->sink;

  if (!(L_store_opcode (assignment) && !(assignment->src[2])))
    L_punt ("L_P3DE_propagate_combine_flag: input assignment is not a "
	    "general store!");

  size = Set_size (assignment->associates);
  if (size == 0)
    L_punt ("Number of associates of a general store should not be 0!");
  reach_array = calloc (size, sizeof (int));

  /* First, generate a list of nodes to propagate over by checking all nodes
   * in the graph and adding their sources if they don't already have the
   * combine flag set. */
  List_start (nodes);
  while ((src_node = (GraphNode) List_next (nodes)))
    {
      src_pce_bb = (L_PCE_BB *) src_node->ptr;
      if ((src_pce_bb->pce_type & BB_COMBINE) &&
	  (src_pce_bb->pce_type & BB_PAST_CUT))
	prop_nodes = List_insert_last (prop_nodes, src_pce_bb);
      else if ((src_pce_bb->pce_type & BB_PAST_CUT) &&
	       !(src_pce_bb->pce_type & BB_COMBINE))
	{
	  List_start (src_node->succ);
	  while ((arc = (GraphArc) List_next (src_node->succ)))
	    {
	      if (arc->succ == sink)
		continue;
	      dest_pce_bb = (L_PCE_BB *) arc->succ->ptr;
	      if (!(dest_pce_bb->pce_type & BB_PAST_CUT) &&
		  (dest_pce_bb->pce_type & BB_COMBINE) &&
		  !(src_pce_bb->pce_type & BB_SINGLE_EXIT) &&
		  (dest_pce_bb->bb->type & BB_SINGLE_ENTRY))
		{
		  src_pce_bb->pce_type |= BB_COMBINE;
		  prop_nodes = List_insert_last (prop_nodes, src_pce_bb);
		  break;
		}
	    }
	}
    }

  /* Next, keep propagating the combine flag upward. */
  List_start (prop_nodes);
  while ((dest_pce_bb = (L_PCE_BB *) List_next (prop_nodes)))
    {
      dest_node = dest_pce_bb->node;
      List_start (dest_node->pred);
      while ((arc = (GraphArc) List_next (dest_node->pred)))
	{
	  if (arc->pred == source)
	    continue;
	  src_pce_bb = (L_PCE_BB *) arc->pred->ptr;
	  if ((src_pce_bb->pce_type & BB_PAST_CUT) &&
	      !(src_pce_bb->pce_type & BB_COMBINE))
	    {
	      src_pce_bb->pce_type |= BB_COMBINE;
	      prop_nodes = List_insert_last (prop_nodes, src_pce_bb);
	    }
	}
      List_start (dest_node->succ);
      while ((arc = (GraphArc) List_next (dest_node->succ)))
	{
	  if (arc->succ == sink)
	    continue;
	  src_pce_bb = (L_PCE_BB *) arc->succ->ptr;
	  if ((src_pce_bb->pce_type & BB_PAST_CUT) &&
	      !(src_pce_bb->pce_type & BB_COMBINE))
	    {
	      src_pce_bb->pce_type |= BB_COMBINE;
	      prop_nodes = List_insert_last (prop_nodes, src_pce_bb);
	    }
	}
    }
  prop_nodes = List_reset (prop_nodes);
  free (reach_array);
}


/* SER 20041213
 * This function marks side entries to the cut graph.  The predicate guarding
 * the execution of a delayed assignment must be cleared at these locations,
 * since those paths did not originally compute the assignment.
 */
static void
L_P3DE_mark_side_pred_clears (L_Expression * assignment,
			      L_PCE_graph * ppde_graph, int combine_flag)
{
  L_PCE_BB *dest_pce_bb, *pce_bb_entry, *pce_bb_exit;
  L_BB *src_bb, *dest_bb;
  DF_PCE_INFO *src_pce, *dest_pce;
  GraphNode src_node, dest_node, dest_node_main, source, sink;
  GraphArc arc, back_arc;
  List nodes;
  int a_index, pred_flag, *reach_array = NULL;

  a_index = ppde_graph->expression_index;
  nodes = ppde_graph->graph->nodes;
  source = ppde_graph->source;
  sink = ppde_graph->sink;

  if (combine_flag)
    {
      int size = Set_size (assignment->associates);
      if (size == 0)
	L_punt ("Number of associates of a general store should not be 0!");
      reach_array = calloc (size, sizeof (int));
    }

  List_start (nodes);
  while ((dest_node = (GraphNode) List_next (nodes)))
    {
      /* First, find all cut nodes which are partially but not fully sinkable,
       * which means that there is a path leading to that location which does
       * not compute the assignment. */
      if (dest_node == source || dest_node == sink)
	continue;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      if (!(dest_pce_bb->pce_type & BB_PAST_CUT))
	continue;
      dest_pce = dest_pce_bb->bb->pf_bb->info->pce_info;
      if (Set_in (dest_pce->x_insert, a_index))
	continue;
      /* We don't need to check if the block has the assignment available. */
      if (Set_in (dest_pce->nu_safe, a_index))
	continue;

      /* Get the L_BB for the graph node and examine its predecessor BBs. */
      dest_bb = dest_pce_bb->bb;
      dest_node_main = dest_bb->node;
      pred_flag = 0;
      List_start (dest_node_main->pred);
      while ((back_arc = (GraphArc) List_next (dest_node_main->pred)))
	{
	  src_node = back_arc->pred;
	  src_bb = (L_BB *) GraphContents (src_node);
	  src_pce = src_bb->pf_bb->info->pce_info;
	  pce_bb_entry = (L_PCE_BB *) src_bb->ptr;
	  pce_bb_exit = (L_PCE_BB *) src_bb->ptr_exit;

	  /* If the BB does not have the assignment completely available
	   * (which happens when the original location is predicated) or is
	   * not both partially available and within the cut, then then is
	   * a control-flow entry to the graph that does not compute the
	   * assignment.  Thus, need to place a pred clear here. */ 
	  if (!(Set_in (src_pce->xu_safe, a_index) ||
		(Set_in (src_pce->x_spec_us, a_index) &&
		 ((pce_bb_exit && (pce_bb_exit->pce_type & BB_PAST_CUT)) ||
		  (pce_bb_entry && (pce_bb_entry->pce_type & BB_PAST_CUT))))))
	    {
#ifndef P3DE_GENERAL_STORE_MOTION
	      src_pce->pred_clear = Set_add (src_pce->pred_clear, a_index);
#else
	      if (!combine_flag || dest_pce_bb->pce_type & BB_COMBINE)
		src_pce->pred_clear = Set_add (src_pce->pred_clear, a_index);
	      else
		{
		  int index, size;
		  size = Set_2array (dest_pce_bb->reaching_assn, reach_array);
		  if (size != 1)
		    {
		      Set_print (stderr, "Reaching assignments: ",
				 dest_pce_bb->reaching_assn);
		      L_punt
			("Size of non-combine BB assignments is %d in cb %d!",
			 size, dest_pce_bb->bb->cb->id);
		    }
		  index = reach_array[0];
		  src_pce->pred_clear = Set_add (src_pce->pred_clear, index);
		}
#endif
#ifdef DEBUG_P3DE_PLACEMENT
	      fprintf (stderr, "\tSetting motion pred clear at BB (cb %d, "
		       "op %d).\n", src_bb->cb->id, src_bb->first_op ?
		       src_bb->first_op->id : 0);
#endif
	      if (!pred_flag)
		{
		  dest_pce_bb->pce_type |= BB_PRED;
		  pred_flag = 1;
		}
	    }
	}
    }
  if (combine_flag)
    free (reach_array);

  /* Also check all assignment nodes to see if they are fully
   * delayable or available. */
  List_start (source->succ);
  while ((arc = (GraphArc) List_next (source->succ)))
    {
      dest_node = arc->succ;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      if (Set_in (dest_pce_bb->bb->pf_bb->info->pce_info->xu_safe, a_index))
	continue;
      dest_pce_bb->pce_type |= BB_PRED;
    }
}


/* SER 20041213
 * This function marks all locations that require predication (either setting
 * the predicate or using guarding predication) to maintain correctness.
 */
static void
L_P3DE_propagate_predication (L_Expression * assignment,
			      L_PCE_graph * ppde_graph)
{
  L_PCE_BB *src_pce_bb, *dest_pce_bb;
  GraphNode src_node, source;
  GraphArc arc;
  List nodes, prop_nodes = NULL;
  int a_index;

  a_index = ppde_graph->expression_index;
  source = ppde_graph->source;
  nodes = ppde_graph->graph->nodes;

  /* Add all predicated nodes to a propagation list. */
  List_start (nodes);
  while ((src_node = (GraphNode) List_next (nodes)))
    {
      src_pce_bb = (L_PCE_BB *) src_node->ptr;
      if (src_pce_bb->pce_type & BB_PRED)
	prop_nodes = List_insert_last (prop_nodes, src_pce_bb);
    }

  /* Next, propagate BB_PRED down to insertion points, to indicate where
   * guarding predication is needed. */
  List_start (prop_nodes);
  while ((src_pce_bb = (L_PCE_BB *) List_next (prop_nodes)))
    {
      src_node = src_pce_bb->node;
      List_start (src_node->pred);
      while ((arc = (GraphArc) List_next (src_node->succ)))
	{
	  if (arc->succ == ppde_graph->sink)
	    continue;
	  dest_pce_bb = (L_PCE_BB *) arc->succ->ptr;
	  if (!(dest_pce_bb->pce_type & BB_PRED) &&
	      dest_pce_bb->pce_type & BB_PAST_CUT)
	    {
	      dest_pce_bb->pce_type |= BB_PRED;
	      prop_nodes = List_insert_last (prop_nodes, dest_pce_bb);
	    }
	}
    }

  /* Finally, propagate BB_PRED back up to computation points.  This is
   * necessary because a fully available computation may need to set the
   * predicate in order to trigger an assignment that can be reached by
   * a partially available original computation. */
  List_start (prop_nodes);
  while ((src_pce_bb = (L_PCE_BB *) List_next (prop_nodes)))
    {
      src_node = src_pce_bb->node;
      List_start (src_node->pred);
      while ((arc = (GraphArc) List_next (src_node->pred)))
	{
	  if (arc->pred == source)
	    continue;
	  dest_pce_bb = (L_PCE_BB *) arc->pred->ptr;
	  if (!(dest_pce_bb->pce_type & BB_PRED) &&
	      dest_pce_bb->pce_type & BB_PAST_CUT)
	    {
	      dest_pce_bb->pce_type |= BB_PRED;
	      prop_nodes = List_insert_last (prop_nodes, dest_pce_bb);
	    }
	}
      prop_nodes = List_delete_current (prop_nodes);
    }
  prop_nodes = List_reset (prop_nodes);
}


/* SER 20041213
 * This function marks all original computations as deletes.  It may be
 * necessary to undo this later, which L_P3DE_mark_insertions() handles.
 */
static void
L_P3DE_mark_deletions (L_Expression * assignment, L_PCE_graph * ppde_graph,
		       int combine_flag)
{
  L_PCE_BB * dest_pce_bb;
  DF_PCE_INFO * dest_pce;
  GraphNode dest_node;
  GraphArc arc;
  int a_index, *reach_array = NULL;

  a_index = ppde_graph->expression_index;  

  if (combine_flag)
    {
      int size = Set_size (assignment->associates);
      if (size == 0)
	L_punt ("Number of associates of a general store should not be 0!");
      reach_array = calloc (size, sizeof (int));
    }

  List_start (ppde_graph->source->succ);
  while ((arc = (GraphArc) List_next (ppde_graph->source->succ)))
    {
      dest_node = arc->succ;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      dest_pce = dest_pce_bb->bb->pf_bb->info->pce_info;

#ifndef P3DE_GENERAL_STORE_MOTION
      dest_pce->x_replace = Set_add (dest_pce->x_replace, a_index);
      if (dest_pce_bb->pce_type & BB_PRED)
	dest_pce->pred_set = Set_add (dest_pce->pred_set, a_index);
#else
      if (!combine_flag || dest_pce_bb->pce_type & BB_COMBINE)
	{
	  dest_pce->x_replace = Set_add (dest_pce->x_replace, a_index);
	  if (dest_pce_bb->pce_type & BB_PRED)
	    dest_pce->pred_set = Set_add (dest_pce->pred_set, a_index);
	}
      else
	{
	  int size, index;
	  size = Set_2array (dest_pce_bb->reaching_assn, reach_array);
	  if (size != 1)
	    {
	      Set_print (stderr, "Reaching assignments: ",
			 dest_pce_bb->reaching_assn);
	      L_punt ("Size of non-combine BB assignments is %d in cb %d!",
		      size, dest_pce_bb->bb->cb->id);
	    }
	  index = reach_array[0];
	  dest_pce->x_replace = Set_add (dest_pce->x_replace, index);
	  if (dest_pce_bb->pce_type & BB_PRED)
	    dest_pce->pred_set = Set_add (dest_pce->pred_set, index);
	}
#endif
    }
  if (combine_flag)
    free (reach_array);
}


/* SER 20041213
 * Modularization of P3DE deletion cancelation.
 */
static inline void
L_P3DE_cancel_deletion (L_PCE_BB * pce_bb, DF_PCE_INFO * pce, int a_index,
			int combine_flag, int * reach_array)
{
#ifndef P3DE_GENERAL_STORE_MOTION
  pce->x_replace = Set_delete (pce->x_replace, a_index);
  pce->pred_set = Set_delete (pce->pred_set, a_index);
#else
  if (!combine_flag || pce_bb->pce_type & BB_COMBINE)
    {
      pce->x_replace = Set_delete (pce->x_replace, a_index);
      pce->pred_set = Set_delete (pce->pred_set, a_index);
    }
  else
    {
      int index, size;
      size = Set_2array (pce_bb->reaching_assn, reach_array);
      if (size != 1)
	{
	  Set_print (stderr, "Reaching assignments: ", pce_bb->reaching_assn);
	  L_punt ("Size of non-combine BB assignments is %d in cb %d!",
		  size, pce_bb->bb->cb->id);
	}
      index = reach_array[0];
      pce->x_replace = Set_delete (pce->x_replace, index);
      pce->pred_set = Set_delete (pce->pred_set, index);
    }
#endif
#ifdef DEBUG_P3DE_PLACEMENT
  fprintf (stderr, "\tCanceling deletion of assignment %d at BB (cb %d, "
	   "op %d).\n", a_index, pce_bb->bb->cb->id, pce_bb->bb->first_op ?
	   pce_bb->bb->first_op->id : 0);
#endif
}


/* SER 20041213
 * Modularization of P3DE insertion.
 */
static inline void
L_P3DE_mark_insert (L_PCE_BB * pce_bb, DF_PCE_INFO * pce, int a_index,
		    int combine_flag, int * reach_array)
{
#ifndef P3DE_GENERAL_STORE_MOTION
  pce->n_insert = Set_add (pce->n_insert, a_index);
  if (pce_bb->pce_type & BB_PRED)
    pce->pred_guard = Set_add (pce->pred_guard, a_index);
#else
  if (!combine_flag || pce_bb->pce_type & BB_COMBINE)
    {
      pce->n_insert = Set_add (pce->n_insert, a_index);
      if (pce_bb->pce_type & BB_PRED)
	pce->pred_guard = Set_add (pce->pred_guard, a_index);
    }
  else
    {
      int index, size;
      size = Set_2array (pce_bb->reaching_assn, reach_array);
      if (size != 1)
	{
	  Set_print (stderr, "Reaching assignments :", pce_bb->reaching_assn);
	  L_punt ("Size of non-combine BB assignments is %d in cb %d!",
		  size, pce_bb->bb->cb->id);
	}
      index = reach_array[0];
      pce->n_insert = Set_add (pce->n_insert, index);
      if (pce_bb->pce_type & BB_PRED)
	pce->pred_guard = Set_add (pce->pred_guard, index);
    }
#endif
#ifdef DEBUG_P3DE_PLACEMENT
  fprintf (stderr, "\tInserting ");
  if (pce_bb->pce_type & BB_PRED)
    fprintf (stderr, "(with pred guard) ");
  if (pce_bb->bb->pf_bb->info->pce_info == pce)
    fprintf (stderr, "assignment %d in ");
  else
    fprintf (stderr, "assignment %d in a BB below ");
  fprintf (stderr, "BB (cb %d, op %d).\n", a_index, pce_bb->bb->cb->id,
	   pce_bb->bb->first_op ? pce_bb->bb->first_op->id : 0);
#endif
}


/* SER 20041213
 * This function marks insertion points.  It sets pred_guard
 * if BB_PRED is set and it is not an original computation.
 */
static int
L_P3DE_mark_insertions (L_Expression * assignment, L_PCE_graph * ppde_graph,
			int combine_flag)
{
  L_PCE_BB *src_pce_bb, *dest_pce_bb;
  L_PCE_arc * pce_arc;
  DF_PCE_INFO *src_pce, *dest_pce;
  GraphNode src_node, dest_node, source, sink;
  GraphArc arc;
  List edges;
  int a_index, change = 0, *reach_array = NULL;

  a_index = ppde_graph->expression_index;
  source = ppde_graph->source;
  sink = ppde_graph->sink;
  edges = ppde_graph->graph->arcs;

  if (combine_flag)
    {
      int size = Set_size (assignment->associates);
      if (size == 0)
	L_punt ("Number of associates of a general store should not be 0!");
      reach_array = calloc (size, sizeof (int));
    }

  List_start (edges);
  while ((arc = (GraphArc) List_next (edges)))
    {
      /* First find cut edges that are not cost edges. */
      pce_arc = (L_PCE_arc *) arc->ptr;
      /* Ignore arcs that were inserted during reweighting. */
      if (pce_arc->pce_type & ARC_COST)
	continue;
      /* Check if edge is part of min-cut */
      src_node = arc->pred;
      src_pce_bb = (L_PCE_BB *) src_node->ptr;
      if (!(src_pce_bb->pce_type & BB_PAST_CUT))
	continue;
      dest_node = arc->succ;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      if (dest_pce_bb->pce_type & BB_PAST_CUT)
	continue;

      if (src_pce_bb->bb)
	{
	  src_pce = src_pce_bb->bb->pf_bb->info->pce_info;
#ifdef DEBUG_P3DE_PLACEMENT
	  fprintf (stderr, "Cut edge between BB (cb %d op %d) and ",
		   src_pce_bb->bb->cb->id, src_pce_bb->bb->first_op ?
		   src_pce_bb->bb->first_op->id : 0);
#endif
	}
#ifdef DEBUG_P3DE_PLACEMENT
      else
	fprintf (stderr, "Cut edge between source and ");
#endif

      if (dest_pce_bb->bb)
	{
	  dest_pce = dest_pce_bb->bb->pf_bb->info->pce_info;
#ifdef DEBUG_P3DE_PLACEMENT
	  fprintf (stderr, "BB (cb %d op %d).\n", dest_pce_bb->bb->cb->id,
		   dest_pce_bb->bb->first_op ?
		   dest_pce_bb->bb->first_op->id : 0);
#endif
	}
#ifdef DEBUG_P3DE_PLACEMENT
      else
	fprintf (stderr, "sink.\n");
#endif

      /* Cut is at original computation point, so cancel deletion. */
      if (src_node == source)
	{
	  L_P3DE_cancel_deletion (dest_pce_bb, dest_pce, a_index, combine_flag,
				  reach_array);
	  continue;
	}

      /* Check for sink cut. */
      if (dest_node == sink)
	{ /* This can happen due to rounding, unlike PRE. */
	  if (Set_in (src_pce->x_insert, a_index))
	    {
	      L_P3DE_cancel_deletion (src_pce_bb, src_pce, a_index,
				      combine_flag, reach_array);
	    }
	  else
	    {
	      L_P3DE_mark_insert (src_pce_bb, src_pce, a_index, combine_flag,
				  reach_array);
	      change++;
	    }
	  continue;
	}

      if (src_pce_bb->bb->type & BB_SINGLE_EXIT)
	{
	  if (Set_in (src_pce->x_insert, a_index))
	    {
	      L_P3DE_cancel_deletion (src_pce_bb, src_pce, a_index,
				      combine_flag, reach_array);
	    }
	  else
	    {
	      L_P3DE_mark_insert (src_pce_bb, src_pce, a_index, combine_flag,
				  reach_array);
	      change++;
	    }
	}
      else if (dest_pce_bb->bb->type & BB_SINGLE_ENTRY)
	{
	  L_P3DE_mark_insert (src_pce_bb, dest_pce, a_index, combine_flag,
			      reach_array);
	  change++;
	}
      else if (src_pce_bb->bb->weight < dest_pce_bb->bb->weight)
	{
	  fprintf (stderr, "Critical edge found; should be splitting these "
		   "prior to P3DE.\n");
	  if (Set_in (src_pce->x_insert, a_index))
	    {
	      L_P3DE_cancel_deletion (src_pce_bb, src_pce, a_index,
				      combine_flag, reach_array);
	    }
	  else
	    {
	      L_P3DE_mark_insert (src_pce_bb, src_pce, a_index, combine_flag,
				  reach_array);
	      change++;
	    }
	}
      else
	{
	  fprintf (stderr, "Critical edge found; should be splitting these "
		   "prior to P3DE.\n");
	  L_P3DE_mark_insert (src_pce_bb, dest_pce, a_index, combine_flag,
			      reach_array);
	  change++;
	}
    }

  if (combine_flag)
    free (reach_array);

  return change;
}


/* SER 20041213
 * This function marks pred clears on arcs at the top of the cut graph.  This
 * happens when an original computation is predicated.
 */
static void
L_P3DE_mark_entry_pred_clears (L_Expression * assignment,
			       L_PCE_graph * ppde_graph, int combine_flag)
{
  L_PCE_BB *dest_pce_bb, *pce_bb_entry, *pce_bb_exit;
  L_BB * src_bb;
  DF_PCE_INFO *src_pce, *dest_pce;
  GraphNode src_node, dest_node, dest_node_main;
  GraphArc arc, back_arc;
  int a_index, *reach_array;

  a_index = ppde_graph->expression_index;

  if (combine_flag)
    {
      int size = Set_size (assignment->associates);
      if (size == 0)
	L_punt ("Number of associates of a general store should not be 0!");
      reach_array = calloc (size, sizeof (int));
    }

  /* Mark pred_clears on incoming arcs to deletion points that are
   * not available or partially sinkable and cut. */
  List_start (ppde_graph->source->succ);
  while ((arc = (GraphArc) List_next (ppde_graph->source->succ)))
    {
      int delete = 0;

      dest_node = arc->succ;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;

      dest_pce = dest_pce_bb->bb->pf_bb->info->pce_info;
#ifndef P3DE_GENERAL_STORE_MOTION
      if (Set_in (dest_pce->x_replace, a_index))
	delete = 1;
#else
      if (!combine_flag || dest_pce_bb->pce_type & BB_COMBINE)
	{
	  if (Set_in (dest_pce->x_replace, a_index))
	    delete = 1;
	}
      else
	{
	  int index, size;
	  size = Set_2array (dest_pce_bb->reaching_assn, reach_array);
	  if (size != 1)
	    {
	      Set_print (stderr, "Reaching assignments: ",
			 dest_pce_bb->reaching_assn);
	      L_punt ("Size of non-combine BB assignments is %d in cb %d!",
		      size, dest_pce_bb->bb->cb->id);
	    }
	  index = reach_array[0];
	  if (Set_in (dest_pce->x_replace, index))
	    delete = 1;
	}
#endif
      if (delete)
	{
#ifdef DEBUG_P3DE_PLACEMENT
	  fprintf (stderr, "\tDeletion ");
	  if (Set_in (dest_pce->pred_set, a_index))
	    fprintf (stderr, "(with pred set) ");
	  fprintf (stderr, "of assignment %d at BB (cb %d, op %d).\n",
		   a_index, dest_pce_bb->bb->cb->id,
		   dest_pce_bb->bb->first_op ?
		   dest_pce_bb->bb->first_op->id : 0);
#endif

	  if (Set_in (dest_pce->xu_safe, a_index))
	    continue;

	  dest_node_main = dest_pce_bb->bb->node;
	  List_start (dest_node_main->pred);
	  while ((back_arc = (GraphArc) List_next (dest_node_main->pred)))
	    {
	      src_node = back_arc->pred;
	      src_bb = (L_BB *) GraphContents (src_node);
	      pce_bb_exit = (L_PCE_BB *) src_bb->ptr_exit;
	      pce_bb_entry = (L_PCE_BB *) src_bb->ptr;
	      src_pce = src_bb->pf_bb->info->pce_info;

	      if (!((Set_in (src_pce->x_spec_us, a_index) &&
		     ((pce_bb_exit && pce_bb_exit->pce_type & BB_PAST_CUT)
		      || (pce_bb_entry &&
			  pce_bb_entry->pce_type & BB_PAST_CUT)))
		    || Set_in (src_pce->xu_safe, a_index)))
		{
#ifndef P3DE_GENERAL_STORE_MOTION
		  src_pce->pred_clear =
		    Set_add (src_pce->pred_clear, a_index);
#else
		  if (!combine_flag || dest_pce_bb->pce_type & BB_COMBINE)
		    {
		      src_pce->pred_clear = Set_add (src_pce->pred_clear,
						     a_index);
		    }
		  else
		    {
		      int index, size;
		      size =
			Set_2array (dest_pce_bb->reaching_assn, reach_array);
		      if (size != 1)
			{
			  Set_print (stderr, "Reaching assignments: ",
				     dest_pce_bb->reaching_assn);
			  L_punt
			    ("Size of non-combine BB assignments is %d in cb %d!",
			     size, dest_pce_bb->bb->cb->id);
			}
		      index = reach_array[0];
		      src_pce->pred_clear = Set_add (src_pce->pred_clear,
						     index);
		    }
#endif
#ifdef DEBUG_P3DE_PLACEMENT
		  fprintf (stderr, "\tSetting instruction pred clear at BB "
			   "(cb %d, op %d).\n", src_bb->cb->id,
			   src_bb->first_op ? src_bb->first_op->id : 0);
#endif
		}
	    }
	}
    }
  if (combine_flag)
    free (reach_array);
}


/* SER 20041213
 * This function marks available side-entrance nodes with pred clears is
 * there is a PRED flag in the adjacent graph nodes.  This removes
 * non-deterministic behavior, since the predicate value is unknown at these
 * points.
 */
static void
L_P3DE_mark_available_pred_clears (L_Expression * assignment,
				   L_PCE_graph * ppde_graph, int combine_flag)
{
  L_PCE_BB *dest_pce_bb, *pce_bb_exit, *pce_bb_entry;
  L_BB *src_bb, *dest_bb;
  DF_PCE_INFO *src_pce, *dest_pce;
  GraphNode src_node, dest_node, dest_node_main, source, sink;
  GraphArc back_arc;
  List nodes;
  int a_index, pred_flag, *reach_array = NULL;

  a_index = ppde_graph->expression_index;
  nodes = ppde_graph->graph->nodes;

  if (combine_flag)
    {
      int size = Set_size (assignment->associates);
      if (size == 0)
	L_punt ("Number of associates of a general store should not be 0!");
      reach_array = calloc (size, sizeof (int));
    }

  /* Last pass: mark available nodes as pred_clears if there is a PRED flag
   * next to it. Need to do this to remove non-determinism. */
  List_start (nodes);
  while ((dest_node = (GraphNode) List_next (nodes)))
    {
      if (dest_node == source || dest_node == sink)
	continue;
      dest_pce_bb = (L_PCE_BB *) dest_node->ptr;
      if (!(dest_pce_bb->pce_type & BB_PRED))
	continue;
      if (!(dest_pce_bb->pce_type & BB_PAST_CUT))
	continue;
      dest_pce = dest_pce_bb->bb->pf_bb->info->pce_info;
      if (Set_in (dest_pce->x_insert, a_index))
	continue;

      /* Go to L_BB, find pred BBs, check to see if they apply. */
      dest_bb = dest_pce_bb->bb;
      dest_node_main = dest_bb->node;
      pred_flag = 0;
      List_start (dest_node_main->pred);
      while ((back_arc = (GraphArc) List_next (dest_node_main->pred)))
	{
	  src_node = back_arc->pred;
	  src_bb = (L_BB *) GraphContents (src_node);
	  src_pce = src_bb->pf_bb->info->pce_info;
	  pce_bb_entry = (L_PCE_BB *) src_bb->ptr;
	  pce_bb_exit = (L_PCE_BB *) src_bb->ptr_exit;

	  if (!(Set_in (src_pce->x_spec_us, a_index) &&
		((pce_bb_exit && pce_bb_exit->pce_type & BB_PAST_CUT) ||
		 (pce_bb_entry && pce_bb_entry->pce_type & BB_PAST_CUT))))
	    {
#ifndef P3DE_GENERAL_STORE_MOTION
#ifdef PRINT_AVAIL_PRED_CLEARS
	      if (!Set_in (src_pce->pred_clear, a_index))
		fprintf (stderr, "P3DE: Avail pred clear (single), cb %d.\n",
			 src_bb->cb->id);
#endif
	      src_pce->pred_clear = Set_add (src_pce->pred_clear, a_index);
#else
	      if (!combine_flag || dest_pce_bb->pce_type & BB_COMBINE)
		{
#ifdef PRINT_AVAIL_PRED_CLEARS
		  if (!Set_in (src_pce->pred_clear, a_index))
		    fprintf (stderr, "P3DE: Avail pred clear (combo), cb "
			     "%d.\n", src_bb->cb->id);
#endif
		  src_pce->pred_clear = Set_add (src_pce->pred_clear, a_index);
		}
	      else
		{
		  int index, size;
		  size = Set_2array (dest_pce_bb->reaching_assn, reach_array);
		  if (size != 1)
		    {
		      Set_print (stderr, "Reaching assignments: ",
				 dest_pce_bb->reaching_assn);
		      L_punt
			("Size of non-combine BB assignments is %d in cb %d!",
			 size, dest_pce_bb->bb->cb->id);
		    }
		  index = reach_array[0];
#ifdef PRINT_AVAIL_PRED_CLEARS
		  if (!Set_in (src_pce->pred_clear, index))
		    fprintf (stderr, "P3DE: Avail pred clear (single), cb "
			     "%d.\n", src_bb->cb->id);
#endif
		  src_pce->pred_clear = Set_add (src_pce->pred_clear, index);
		}
#endif
#ifdef DEBUG_P3DE_PLACEMENT
	      fprintf (stderr, "\tSetting motion pred clear at BB (cb %d, "
		       "op %d).\n", src_bb->cb->id, src_bb->first_op ?
		       src_bb->first_op->id : 0);
#endif
	    }
	}
    }

  if (combine_flag)
    free (reach_array);
}


/* 10/26/04 REK This function is broken.  It claims to return an int, but
 *              there are no return statements.  It has one caller
 *              (L_PDE_predicate_motion) that expects a return value. */
/* 12/13/04 SER Fixed return value, modularized. */
static int
L_P3DE_placement (L_PCE_graph * ppde_graph)
{
  L_Expression *assignment;
  int a_index, combine_flag = 0, change;

  a_index = ppde_graph->expression_index;

#ifdef DEBUG_P3DE_PLACEMENT
  fprintf (stderr, "Starting P3DE placement for assignment %d.\n", a_index);
#ifdef PRINT_P3DE_GRAPH
  L_print_PCE_graph (ppde_graph);
#endif
#endif

  assignment = (L_expression_hash_tbl_find_entry
		(L_fn->expression_index_hash_tbl, a_index))->expression;
#ifdef P3DE_GENERAL_STORE_MOTION
  if (L_store_opcode (assignment) && !(assignment->src[2]))
    {
      L_P3DE_propagate_combine_flag (assignment, ppde_graph);
      combine_flag = 1;
    }
#endif

  L_P3DE_mark_side_pred_clears (assignment, ppde_graph, combine_flag);
  L_P3DE_propagate_predication (assignment, ppde_graph);
  L_P3DE_mark_deletions (assignment, ppde_graph, combine_flag);
  change = L_P3DE_mark_insertions (assignment, ppde_graph, combine_flag);
  L_P3DE_mark_entry_pred_clears (assignment, ppde_graph, combine_flag);
  L_P3DE_mark_available_pred_clears (assignment, ppde_graph, combine_flag);

  return change;
}


void
L_PDE_predicate_motion (Graph bb_weighted_graph, PRED_FLOW * pred_flow,
			Set * motion, int no_pred, int max_motions)
{
  int i, size, *assignments, motions = 0;
  L_PCE_graph *ppde_graph;
  L_Expression *assignment;

  assignments = (int *) Lcode_malloc (sizeof (int) * L_fn->n_expression);
  size = Set_2array (*motion, assignments);

  /* Prep graph cut hash. */
  if (!(L_PCE_graph_cut_tbl))
    L_PCE_graph_cut_tbl = (L_PCE_graph_cut **)
      calloc (PCE_GRAPH_TBL_SIZE, sizeof (L_PCE_graph_cut *));

  for (i = 0; i < size; i++)
    {
#ifdef DEBUG_P3DE_PLACEMENT
      fprintf (stderr, "Beginning PDE motion analysis for assignment %d.\n",
	       assignments[i]);
#endif
      ppde_graph = L_construct_P3DE_graph (bb_weighted_graph, assignments[i]);
      /* check if graph is non-trivial */
      if (ppde_graph->n_vertices > 2)
	{			/* check if graph already exists */
	  assignment = (L_expression_hash_tbl_find_entry
			(L_fn->expression_index_hash_tbl,
			 assignments[i]))->expression;
	  if (L_store_opcode (assignment) && !(assignment->src[2]))
	    L_P3DE_reaching_assignments (ppde_graph);
#ifdef P3DE_PENALTY_METRIC
	  L_P3DE_penalty_reweighting (ppde_graph, no_pred);
#endif
#ifdef P3DE_PROFITABILITY_METRIC
	  L_PCE_reg_pressure_reweighting (ppde_graph, 1);
#endif
	  L_PCE_reverse_push_preflow (ppde_graph);
	  L_P3DE_minimum_cut (ppde_graph);

	  /* 10/26/04 REK L_P3DE_placement claims to return an int, but has
	   *              no return statements, so this is probably broken. */
	  /* 12/11/04 SER Return fixed. */
	  if (L_P3DE_placement (ppde_graph))
	    motions++;
	}
      L_delete_PCE_graph (ppde_graph);
    }
  Lcode_free (assignments);
}
