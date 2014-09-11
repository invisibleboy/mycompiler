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
 *
 *      File :          i_graph.c
 *      Description :   Functions to maintain graphs.
 *      Creation Date : November 1996
 *      Author :        David August
 *
 *      Copyright (c) 1996 David August, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
 *===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdlib.h>
#include "i_graph.h"

L_Alloc_Pool *GraphPool = NULL;
L_Alloc_Pool *GraphNodePool = NULL;
L_Alloc_Pool *GraphArcPool = NULL;
L_Alloc_Pool *GraphEquivCDPool = NULL;

GraphNode Graph_create_graph_node_given_id(Graph graph, int new_id) {
	GraphNode new_graph_node;

	if (Graph_node_from_id_or_null(graph, new_id))
		I_punt("Graph_create_graph_node_given_id:  id already in use\n");

	if (!GraphNodePool)
		GraphNodePool = L_create_alloc_pool("_GraphNode", sizeof(_GraphNode),
				128);

	new_graph_node = (GraphNode) L_alloc(GraphNodePool);
	new_graph_node->id = new_id;
	new_graph_node->ptr = NULL;
	new_graph_node->dom = NULL;
	new_graph_node->flags = 0;
	new_graph_node->tflags = 0;
	new_graph_node->info = 0;
	new_graph_node->post_dom = NULL;
	new_graph_node->imm_dom = NULL;
	new_graph_node->imm_post_dom = NULL;
	new_graph_node->contr_dep = NULL;
	new_graph_node->equiv_cd = NULL;
	new_graph_node->pred = NULL;
	new_graph_node->succ = NULL;

	graph->nodes = List_insert_last(graph->nodes, new_graph_node);
	graph->flags = GRAPHMODIFIED; /*  wipes out all other flags  */
	HashTable_insert(graph->hash_id_node, new_graph_node->id, new_graph_node);

	return new_graph_node;
}

GraphNode Graph_create_graph_node(Graph graph) {
	return Graph_create_graph_node_given_id(graph, ++graph->node_id);
}

static GraphNode Graph_free_graph_node(GraphNode graph_node) {
	Set_dispose(graph_node->dom);
	Set_dispose(graph_node->post_dom);
	Set_dispose(graph_node->contr_dep);
	List_reset(graph_node->pred);
	List_reset(graph_node->succ);

	L_free(GraphNodePool, graph_node);

	return NULL;
}

static GraphArc Graph_free_graph_arc(GraphArc graph_arc) {
	L_free(GraphArcPool, graph_arc);

	return NULL;
}

GraphArc Graph_create_graph_arc_given_id(Graph graph, int new_id) {
	GraphArc new_graph_arc;

	if (Graph_arc_from_id_or_null(graph, new_id))
		I_punt("Graph_create_graph_arc_given_id:  id already in use\n");

	if (!GraphArcPool)
		GraphArcPool = L_create_alloc_pool("_GraphArc", sizeof(_GraphArc), 128);

	new_graph_arc = (GraphArc) L_alloc(GraphArcPool);
	new_graph_arc->id = new_id;
	new_graph_arc->flags = 0;
	new_graph_arc->info = 0;
	new_graph_arc->ptr = NULL;
	new_graph_arc->pred = NULL;
	new_graph_arc->succ = NULL;

	graph->arcs = List_insert_last(graph->arcs, new_graph_arc);
	graph->flags = GRAPHMODIFIED; /* wipes out all other flags */
	HashTable_insert(graph->hash_id_arc, new_graph_arc->id, new_graph_arc);

	return new_graph_arc;
}

GraphArc Graph_create_graph_arc(Graph graph) {
	return Graph_create_graph_arc_given_id(graph, ++graph->arc_id);
}

static GraphEquivCD Graph_new_graph_equiv_cd() {
	GraphEquivCD new_graph_equiv_cd;

	if (!GraphEquivCDPool)
		GraphEquivCDPool = L_create_alloc_pool("_EquivCD",
				sizeof(_GraphEquivCD), 128);

	new_graph_equiv_cd = (GraphEquivCD) L_alloc(GraphEquivCDPool);
	new_graph_equiv_cd->id = 0;
	new_graph_equiv_cd->ptr = NULL;
	new_graph_equiv_cd->contr_dep = NULL;
	new_graph_equiv_cd->nodes = NULL;
	new_graph_equiv_cd->arcs = NULL;

	return new_graph_equiv_cd;
}

GraphEquivCD Graph_create_graph_equiv_cd(Graph graph) {
	GraphEquivCD new_graph_equiv_cd;

	new_graph_equiv_cd = Graph_new_graph_equiv_cd();
	graph->equiv_cds = List_insert_last(graph->equiv_cds, new_graph_equiv_cd);
	new_graph_equiv_cd->id = List_size(graph->equiv_cds);
	HashTable_insert(graph->hash_id_equiv_cd, new_graph_equiv_cd->id,
			new_graph_equiv_cd);

	return new_graph_equiv_cd;
}

GraphEquivCD Graph_free_graph_equiv_cd(GraphEquivCD graph_equiv_cd) {
	if (graph_equiv_cd->contr_dep)
		Set_dispose(graph_equiv_cd->contr_dep);

	if (graph_equiv_cd->nodes)
		List_reset(graph_equiv_cd->nodes);

	if (graph_equiv_cd->arcs)
		List_reset(graph_equiv_cd->arcs);

	L_free(GraphEquivCDPool, graph_equiv_cd);

	return NULL;
}

Graph Graph_create_graph() {
	Graph new_graph;

	if (!GraphPool)
		GraphPool = L_create_alloc_pool("_Graph", sizeof(_Graph), 8);

	new_graph = (Graph) L_alloc(GraphPool);
	new_graph->root = NULL;
	new_graph->nodes = NULL;
	new_graph->arcs = NULL;
	new_graph->equiv_cds = NULL;

	new_graph->flags = GRAPHMODIFIED; /* wipes out all other flags */

	new_graph->topo_list = NULL;
	new_graph->rev_topo_list = NULL;
	new_graph->dfs_list = NULL;

	new_graph->hash_id_node = HashTable_create(256);
	new_graph->hash_id_arc = HashTable_create(512);
	new_graph->hash_id_equiv_cd = HashTable_create(256);

	new_graph->node_id = 0;
	new_graph->arc_id = 0;

	return new_graph;
}

Graph Graph_free_graph(Graph graph) {
	GraphNode node;
	GraphArc arc;
	GraphEquivCD equiv_cd;

	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes)))
		Graph_free_graph_node(node);

	List_start(graph->arcs);
	while ((arc = (GraphArc) List_next(graph->arcs)))
		Graph_free_graph_arc(arc);

	List_start(graph->equiv_cds);
	while ((equiv_cd = (GraphEquivCD) List_next(graph->equiv_cds)))
		Graph_free_graph_equiv_cd(equiv_cd);

	List_reset(graph->nodes);
	List_reset(graph->arcs);
	List_reset(graph->equiv_cds);
	List_reset(graph->topo_list);
	List_reset(graph->rev_topo_list);
	List_reset(graph->dfs_list);

	HashTable_free(graph->hash_id_node);
	HashTable_free(graph->hash_id_arc);
	HashTable_free(graph->hash_id_equiv_cd);

	L_free(GraphPool, graph);

#if 0
	if (GraphPool)
	L_print_alloc_info (stderr, GraphPool, 0);
	if (GraphNodePool)
	L_print_alloc_info (stderr, GraphNodePool, 0);
	if (GraphArcPool)
	L_print_alloc_info (stderr, GraphArcPool, 0);
	if (GraphEquivCDPool)
	L_print_alloc_info (stderr, GraphEquivCDPool, 0);
#endif

	return NULL;
}

GraphNode Graph_node_from_id(Graph graph, int id) {
	return ((GraphNode) HashTable_find(graph->hash_id_node, id));
}

GraphNode Graph_node_from_id_or_null(Graph graph, int id) {
	return ((GraphNode) HashTable_find_or_null(graph->hash_id_node, id));
}

GraphArc Graph_arc_from_id(Graph graph, int id) {
	return ((GraphArc) HashTable_find(graph->hash_id_arc, id));
}

GraphArc Graph_arc_from_id_or_null(Graph graph, int id) {
	return ((GraphArc) HashTable_find_or_null(graph->hash_id_arc, id));
}

GraphEquivCD Graph_equiv_cd_from_id(Graph graph, int id) {
	return ((GraphEquivCD) HashTable_find(graph->hash_id_equiv_cd, id));
}

int Graph_size(Graph graph) {
	if (!graph)
		I_punt("Graph not initialized");

	return List_size(graph->nodes);
}

GraphArc Graph_connect_nodes_with_weight(Graph graph, GraphNode src_node,
		GraphNode dest_node,int wcet_weight){

		GraphArc new_graph_arc;
		new_graph_arc = Graph_create_graph_arc(graph);
		new_graph_arc->succ = dest_node;
		new_graph_arc->pred = src_node;

		new_graph_arc->wcet_weight = wcet_weight;

		src_node->succ = List_insert_last(src_node->succ, new_graph_arc);
		dest_node->pred = List_insert_last(dest_node->pred, new_graph_arc);

		return new_graph_arc;
}

GraphArc Graph_connect_nodes(Graph graph, GraphNode src_node,
		GraphNode dest_node) {
	GraphArc new_graph_arc;
	new_graph_arc = Graph_create_graph_arc(graph);
	new_graph_arc->succ = dest_node;
	new_graph_arc->pred = src_node;

	src_node->succ = List_insert_last(src_node->succ, new_graph_arc);
	dest_node->pred = List_insert_last(dest_node->pred, new_graph_arc);

	return new_graph_arc;
}

int Graph_delete_unreachable(Graph graph, void(*node_ptr_del)(void *),
		void(*arc_ptr_del)(void *)) {
	GraphArc arc;
	GraphNode node, snode;
	int count = 0;

	count = Graph_topological_sort(graph);

	graph->flags &= ~GRAPHTOPOSORT;

	if (graph->topo_list)
		graph->topo_list = List_reset(graph->topo_list);

	if (!count)
		return 0;

	List_start(graph->nodes);
	while ((node = List_next(graph->nodes))) {
		if (node->info != -1)
			continue;

		List_start(node->succ);
		while ((arc = (GraphArc) List_next(node->succ))) {
			snode = arc->succ;

			snode->pred = List_remove(snode->pred, arc);
			if (arc_ptr_del && GraphArcContents(arc))
				arc_ptr_del(GraphArcContents(arc));

			graph->arcs = List_remove(graph->arcs, arc);
			Graph_free_graph_arc(arc);
		}
	}

	List_start(graph->nodes);
	while ((node = List_next(graph->nodes))) {
		if (node->info != -1)
			continue;

		graph->nodes = List_delete_current(graph->nodes);

		if (node_ptr_del && GraphNodeContents(node))
			node_ptr_del(GraphNodeContents(node));

		Graph_free_graph_node(node);
	}

	return count;
}

static void Graph_daVinci_visit(GraphNode node, FILE * daVinci_file,
		void(*print_ptr_func)(FILE *, GraphNode)) {
	GraphArc succ_arc;

	if (node->tflags == GRAPH_NODE_VISIT_BLACK) {
		fprintf(daVinci_file, "r(\"");
		print_ptr_func(daVinci_file, node);
		fprintf(daVinci_file, "\")");
		return;
	}

	node->tflags = GRAPH_NODE_VISIT_BLACK;

	/* Define Object */
	fprintf(daVinci_file, "l(\"");
	print_ptr_func(daVinci_file, node);
	fprintf(daVinci_file, "\",");

	fprintf(daVinci_file, "n(\"anything\", [a(\"OBJECT\", \"");
	print_ptr_func(daVinci_file, node);
	fprintf(daVinci_file, "\")],\n[");

	/* Define Edges */
	List_start(node->succ);
	while ((succ_arc = (GraphArc) List_next(node->succ))) {
		fprintf(daVinci_file, "e(\"anything\",[");
		if (Set_in(node->dom, succ_arc->succ->id)) {
			fprintf(daVinci_file, "a(\"EDGECOLOR\",\"red\")");
		} else {
			fprintf(daVinci_file, "a(\"EDGECOLOR\",\"black\")");
		}
		fprintf(daVinci_file, "],");
		Graph_daVinci_visit(succ_arc->succ, daVinci_file, print_ptr_func);
		fprintf(daVinci_file, ")");
		if (List_get_next(node->succ)) {
			fprintf(daVinci_file, ",");
		}
	}

	/* Define Object */
	fprintf(daVinci_file, "]))");
}

/* Assumes dominator has been performed */
void Graph_daVinci(Graph graph, char *filename,
		void(*print_ptr_func)(FILE *, GraphNode)) {
	FILE *daVinci_file = NULL;
	GraphNode node;

	if (!(graph->flags & GRAPHDOM))
		Graph_dominator(graph);

	daVinci_file = I_open_output_file(filename);

	fprintf(daVinci_file, "[\n");

	/* Clear the visited bit flag */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes)))
		node->tflags = GRAPH_NODE_VISIT_WHITE;

	Graph_daVinci_visit(graph->root, daVinci_file, print_ptr_func);

	fprintf(daVinci_file, "]\n");

	I_close_output_file(daVinci_file);
}

void Graph_daVinci_visit2(GraphNode node, void(*output_str_fn)(char *),
		void(*print_node_fn)(GraphNode, char *, int),
		void(*print_arc_fn)(GraphArc, char *, int), int detail) {
	GraphArc succ_arc;
	char buffer[10192];
	int detail_copy = detail & DAV_ALL_NODE_PROPERTIES;

	if (node->tflags == GRAPH_NODE_VISIT_BLACK) {
		sprintf(buffer, "r(\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_ID);
		/*      print_node_fn(node, buffer, DAV_OBJECT);  */
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
		return;
	}
	node->tflags = GRAPH_NODE_VISIT_BLACK;

	/* Define Object */
	sprintf(buffer, "l(\"");
	output_str_fn(buffer);
	print_node_fn(node, buffer, DAV_ID);
	/*   print_node_fn(node, buffer, DAV_OBJECT);  */
	output_str_fn(buffer);
	sprintf(buffer, "\",n(\"");
	output_str_fn(buffer);
	print_node_fn(node, buffer, DAV_TYPE);
	output_str_fn(buffer);
	sprintf(buffer, "\",");
	output_str_fn(buffer);

	/*  give node attributes  */
	sprintf(buffer, "[");
	output_str_fn(buffer);
	if (EXTRACT_BIT_VAL (detail, DAV_OBJECT)) {
		sprintf(buffer, "a(\"OBJECT\",\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_OBJECT);
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
	}

	detail_copy &= ~DAV_OBJECT;
	if (detail_copy) {
		sprintf(buffer, ",");
		output_str_fn(buffer);
	}

	if (EXTRACT_BIT_VAL (detail, DAV_FONTFAMILY)) {
		sprintf(buffer, "a(\"FONTFAMILY\",\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_FONTFAMILY);
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
	}

	detail_copy &= ~DAV_FONTFAMILY;
	if (detail_copy) {
		sprintf(buffer, ",");
		output_str_fn(buffer);
	}

	if (EXTRACT_BIT_VAL (detail, DAV_FONTSTYLE)) {
		sprintf(buffer, "a(\"FONTSTYLE\",\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_FONTSTYLE);
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
	}

	detail_copy &= ~DAV_FONTSTYLE;
	if (detail_copy) {
		sprintf(buffer, ",");
		output_str_fn(buffer);
	}

	if (EXTRACT_BIT_VAL (detail, DAV_COLOR)) {
		sprintf(buffer, "a(\"COLOR\",\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_COLOR);
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
	}

	detail_copy &= ~DAV_COLOR;
	if (detail_copy) {
		sprintf(buffer, ",");
		output_str_fn(buffer);
	}

	if (EXTRACT_BIT_VAL (detail, DAV_GO)) {
		sprintf(buffer, "a(\"_GO\",\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_GO);
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
	}

	detail_copy &= ~DAV_GO;
	if (detail_copy) {
		sprintf(buffer, ",");
		output_str_fn(buffer);
	}

	if (EXTRACT_BIT_VAL (detail, DAV_HIDDEN)) {
		sprintf(buffer, "a(\"HIDDEN\",\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_HIDDEN);
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
	}

	detail_copy &= ~DAV_HIDDEN;
	if (detail_copy) {
		sprintf(buffer, ",");
		output_str_fn(buffer);
	}

	if (EXTRACT_BIT_VAL (detail, DAV_BORDER)) {
		sprintf(buffer, "a(\"BORDER\",\"");
		output_str_fn(buffer);
		print_node_fn(node, buffer, DAV_BORDER);
		output_str_fn(buffer);
		sprintf(buffer, "\")");
		output_str_fn(buffer);
	}

	sprintf(buffer, "],");
	output_str_fn(buffer);

	/* Define Edges */
	sprintf(buffer, "[");
	output_str_fn(buffer);

	List_start(node->succ);
	while ((succ_arc = (GraphArc) List_next(node->succ))) {
		sprintf(buffer, "e(\"");
		output_str_fn(buffer);
		print_arc_fn(succ_arc, buffer, DAV_TYPE);
		output_str_fn(buffer);
		sprintf(buffer, "\",");
		output_str_fn(buffer);

		/* give arc attributes  */
		detail_copy = detail & DAV_ALL_EDGE_PROPERTIES;
		sprintf(buffer, "[");
		output_str_fn(buffer);

		if (EXTRACT_BIT_VAL (detail, DAV_EDGECOLOR)) {
			sprintf(buffer, "a(\"EDGECOLOR\",\"");
			output_str_fn(buffer);
			print_arc_fn(succ_arc, buffer, DAV_EDGECOLOR);
			output_str_fn(buffer);
			sprintf(buffer, "\")");
			output_str_fn(buffer);
		}

		detail_copy &= ~DAV_EDGECOLOR;
		if (detail_copy) {
			sprintf(buffer, ",");
			output_str_fn(buffer);
		}

		if (EXTRACT_BIT_VAL (detail, DAV_EDGEPATTERN)) {
			sprintf(buffer, "a(\"EDGEPATTERN\",\"");
			output_str_fn(buffer);
			print_arc_fn(succ_arc, buffer, DAV_EDGEPATTERN);
			output_str_fn(buffer);
			sprintf(buffer, "\")");
			output_str_fn(buffer);
		}

		detail_copy &= ~DAV_EDGEPATTERN;
		if (detail_copy) {
			sprintf(buffer, ",");
			output_str_fn(buffer);
		}

		if (EXTRACT_BIT_VAL (detail, DAV_EDGEDIR)) {
			sprintf(buffer, "a(\"_DIR\",\"");
			output_str_fn(buffer);
			print_arc_fn(succ_arc, buffer, DAV_EDGEDIR);
			output_str_fn(buffer);
			sprintf(buffer, "\")");
			output_str_fn(buffer);
		}

		sprintf(buffer, "],");
		output_str_fn(buffer);

		Graph_daVinci_visit2(succ_arc->succ, output_str_fn, print_node_fn,
				print_arc_fn, detail);

		sprintf(buffer, ")");
		output_str_fn(buffer);

		if (List_get_next(node->succ)) {
			sprintf(buffer, ",");
			output_str_fn(buffer);
		}
	}

	/* Define Object */
	sprintf(buffer, "]))");
	output_str_fn(buffer);

	return;
}

void Graph_daVinci2(Graph graph, void(*output_str_fn)(char *),
		void(*print_node_fn)(GraphNode, char *, int),
		void(*print_arc_fn)(GraphArc, char *, int), int detail) {
	char buffer[30];
	GraphNode node;

	if (!(graph->flags & GRAPHDOM))
		Graph_dominator(graph);

	/*  clear the visited bit flag  */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes)))
		node->tflags = GRAPH_NODE_VISIT_WHITE;

	sprintf(buffer, "[");
	output_str_fn(buffer);

	Graph_daVinci_visit2(GraphRootNode (graph), output_str_fn, print_node_fn,
			print_arc_fn, detail);

	sprintf(buffer, "]");
	output_str_fn(buffer);

	return;
}

/*  
 *  This function can be used to output graphs with multiple, 
 *  possibly disjoint, roots.  It assumes that the true root of 
 *  the graph is a dummy node that points to all other root
 *  nodes.  The dummy node, usually given id -1 is not shown
 *  in the daVinci graph.
 */
void Graph_daVinci2_multi_root(Graph graph, void(*output_str_fn)(char *),
		void(*print_node_fn)(GraphNode, char *, int),
		void(*print_arc_fn)(GraphArc, char *, int), int detail) {
	char buffer[30];
	List from_start_arc_list;
	GraphArc from_start_arc;
	GraphNode node;

	if (!(graph->flags & GRAPHDOM))
		Graph_dominator(graph);

	/*  clear the visited bit flag  */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes)))
		node->tflags = GRAPH_NODE_VISIT_WHITE;

	from_start_arc_list = GraphRootNode (graph)->succ;
	from_start_arc = (GraphArc) List_first(from_start_arc_list);

	sprintf(buffer, "[");
	output_str_fn(buffer);

	while (from_start_arc != NULL) {
		Graph_daVinci_visit2(from_start_arc->succ, output_str_fn,
				print_node_fn, print_arc_fn, detail);

		from_start_arc = (GraphArc) List_next(from_start_arc_list);

		if (from_start_arc != NULL)
			output_str_fn(",");
	}

	sprintf(buffer, "]");
	output_str_fn(buffer);

	return;
}

/*===========================================================================*/

/* Dominator has been performed */
void Graph_imm_dominator(Graph graph) {
	GraphNode node;
	GraphNode topo_node;

	if (graph->flags & GRAPHIMMDOM)
		return;

	if (!(graph->flags & GRAPHDOM))
		Graph_dominator(graph);

	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		node->imm_dom = NULL;
		List_start(graph->topo_list);
		while ((topo_node = (GraphNode) List_next(graph->topo_list))) {
			if (topo_node == node)
				break;
			if (Set_in(node->dom, topo_node->id))
				node->imm_dom = topo_node;
		}
	}
	graph->flags |= GRAPHIMMDOM;
}

/* Graph_dominator will work with back edges, however these back edges
 * can NOT connect to the first node in the graph.
 */
void Graph_dominator(Graph graph) {
	GraphNode node;
	GraphNode pred;
	GraphArc arc;
	Set all;
	int change;
	Set new_dom;

	/* haven't run dominator before on this graph */
	if (graph->flags & GRAPHDOM)
		return;

	/*
	 *    Initialization
	 */
	all = NULL;
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		if (node->dom)
			node->dom = Set_dispose(node->dom);
		all = Set_add(all, node->id);
	}
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		if (List_size(node->pred))
			node->dom = Set_union(0, all);
		else
			node->dom = Set_add(NULL, node->id);
	}

	/*
	 *    Compute dominance relationship
	 */
	change = 1;
	while (change) {
		change = 0;

		List_start(graph->nodes);
		while ((node = (GraphNode) List_next(graph->nodes))) {
			new_dom = NULL;
			List_start(node->pred);
			while ((arc = (GraphArc) List_next(node->pred))) {
				pred = arc->pred;
				if (new_dom)
					new_dom = Set_intersect_acc(new_dom, pred->dom);
				else
					new_dom = Set_union(new_dom, pred->dom);
			}
			new_dom = Set_add(new_dom, node->id);
			if (!Set_same(new_dom, node->dom)) {
				change += 1;
				node->dom = Set_dispose(node->dom);
				node->dom = new_dom;
			} else
				Set_dispose(new_dom);
		}
	}
	all = Set_dispose(all);

	graph->flags |= GRAPHDOM;
}

#if 0
void
Graph_print_dominator (Graph graph)
{
	GraphNode node;

	List_start (graph->nodes);
	while ((node = (GraphNode) List_next (graph->nodes)))
	{
		printf ("Node %d:\n  ", node->id);
		PSET (node->dom);
	}
}
#endif

/*===========================================================================*/

/* Post dominator has been performed */
void Graph_imm_post_dominator(Graph graph) {
	GraphNode node;
	GraphNode topo_node;

	if (graph->flags & GRAPHIMMPOSTDOM)
		return;

	if (!(graph->flags & GRAPHPOSTDOM))
		Graph_post_dominator(graph);

	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		node->imm_post_dom = NULL;
		List_start(graph->topo_list);
		while ((topo_node = (GraphNode) List_prev(graph->topo_list))) {
			if (topo_node == node)
				break;
			if (Set_in(node->post_dom, topo_node->id))
				node->imm_post_dom = topo_node;
		}
	}
	graph->flags |= GRAPHIMMPOSTDOM;
}

void Graph_post_dominator(Graph graph) {
	GraphNode node;
	GraphNode succ;
	GraphArc arc;
	Set all;
	int change;
	Set old_post_dom;
	Set new_post_dom;

	if (graph->flags & GRAPHPOSTDOM)
		return;

	/*
	 *    Initialization
	 */
	all = NULL;
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		if (node->post_dom)
			node->post_dom = Set_dispose(node->post_dom);
		all = Set_add(all, node->id);
	}
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		if (List_size(node->succ))
			node->post_dom = Set_union(0, all);
		else
			node->post_dom = Set_add(NULL, node->id);
	}

	/*
	 *    Compute dominance relationship
	 */
	change = 1;
	while (change) {
		change = 0;
		List_start(graph->nodes);
		while ((node = (GraphNode) List_next(graph->nodes))) {
			old_post_dom = node->post_dom;
			new_post_dom = NULL;

			List_start(node->succ);
			while ((arc = (GraphArc) List_next(node->succ))) {
				succ = arc->succ;
				if (new_post_dom)
					new_post_dom = Set_intersect_acc(new_post_dom,
							succ->post_dom);
				else
					new_post_dom = Set_union(new_post_dom, succ->post_dom);
			}
			new_post_dom = Set_add(new_post_dom, node->id);
			if (!Set_same(new_post_dom, old_post_dom)) {
				change += 1;
				node->post_dom = Set_dispose(node->post_dom);
				node->post_dom = new_post_dom;
			} else {
				new_post_dom = Set_dispose(new_post_dom);
			}
		}
	}
	graph->flags |= GRAPHPOSTDOM;
	all = Set_dispose(all);
}

/* 
 * Assumes immediate postdominator has been computed.
 * Graph must have a unique start and stop node as
 * determined by first and last node in topological ordering.
 */
void Graph_control_dependence(Graph graph) {
	GraphNode node;
	GraphNode end;
	GraphNode dest;
	GraphNode final_imm_pdom;
	GraphNode cur_imm_pdom;
	GraphArc arc;

	if (graph->flags & GRAPHCONTROL)
		return;

	if (!(graph->flags & GRAPHIMMPOSTDOM))
		Graph_imm_post_dominator(graph);

	/* Initialization stuff */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		node->contr_dep = NULL;
	}

	/* Actually compute the control dependences */
	Graph_topological_sort(graph);
	end = (GraphNode) List_last(graph->topo_list);
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		List_start(node->succ);
		while ((arc = (GraphArc) List_next(node->succ))) {
			dest = GraphArcDest (arc);
			if (!Set_in(node->dom, dest->id) && !Set_in(dest->post_dom,
					node->id)) {
				cur_imm_pdom = dest;
				final_imm_pdom = node->imm_post_dom;
				while (cur_imm_pdom && final_imm_pdom && (cur_imm_pdom
						!= final_imm_pdom) && (cur_imm_pdom != end)) {
					cur_imm_pdom->contr_dep = Set_add(cur_imm_pdom->contr_dep,
							arc->id);
					cur_imm_pdom = cur_imm_pdom->imm_post_dom;
				}
			}
		}
	}
	graph->flags |= GRAPHCONTROL;
}

/* 
 * Assumes control dependences are computed.
 */
void Graph_equiv_cd(Graph graph) {
	GraphNode node;
	GraphArc arc;
	GraphEquivCD equiv_cd;
	Bool found;
	int *buf, num_arcs, i;

	if (graph->flags & GRAPHEQUIVCD)
		return;

	if (!(graph->flags & GRAPHCONTROL))
		Graph_control_dependence(graph); /* since it hasn't been run lets run
		 it */

	List_start(graph->topo_list);
	while ((node = (GraphNode) List_next(graph->topo_list))) {
		found = FALSE;
		List_start(graph->equiv_cds);
		while ((equiv_cd = (GraphEquivCD) List_next(graph->equiv_cds))) {
			if (Set_same(equiv_cd->contr_dep, node->contr_dep)) {
				found = TRUE;
				equiv_cd->nodes = List_insert_last(equiv_cd->nodes, node);
				node->equiv_cd = equiv_cd;
				break;
			}
		}
		if (!found) {
			equiv_cd = Graph_create_graph_equiv_cd(graph);
			equiv_cd->contr_dep = Set_union(NULL, node->contr_dep);
			if ((num_arcs = Set_size(node->contr_dep))) {
				buf = (int *) malloc(sizeof(int) * num_arcs);
				Set_2array(node->contr_dep, buf);
				for (i = 0; i < num_arcs; i++) {
					arc = Graph_arc_from_id(graph, buf[i]);
					equiv_cd->arcs = List_insert_last(equiv_cd->arcs, arc);
				}
				free(buf);
			}
			equiv_cd->nodes = List_insert_last(equiv_cd->nodes, node);
			node->equiv_cd = equiv_cd;
		}
	}
	graph->flags |= GRAPHEQUIVCD;
}

/*===========================================================================*/

static void Graph_count_paths_visit(GraphNode node) {
	GraphArc succ_arc;

	if (node->tflags)
		return;

	/* Define Edges */
	List_start(node->succ);
	while ((succ_arc = (GraphArc) List_next(node->succ))) {
		succ_arc->succ->tflags--;
		/* Don't cross backedges */
		if (!Set_in(node->dom, succ_arc->succ->id)) {
			succ_arc->succ->info += node->info;
			Graph_count_paths_visit(succ_arc->succ);
		}
	}
}

/* Assumes dominator has been performed */
void Graph_count_paths(Graph graph) {
	GraphNode node;

	if (graph->flags & GRAPHCOUNTPATH)
		return;

	if (!(graph->flags & GRAPHDOM))
		Graph_dominator(graph); /* since it hasn't been run lets run it */

	/* Initialize the predecessor count */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		node->tflags = List_size(node->pred);
		node->info = 0;
	}

	graph->root->info = 1;
	graph->root->tflags = 0;

	Graph_count_paths_visit(graph->root);
	graph->flags |= GRAPHCOUNTPATH;
}

void Graph_dfs_topo_visit(Graph graph, GraphNode node) {
	GraphNode dest_node;
	GraphArc arc;

	node->tflags = 1;

	List_start(node->succ);
	while ((arc = (GraphArc) List_next(node->succ))) {
		dest_node = GraphArcDest (arc);
		if (!dest_node->tflags)
			Graph_dfs_topo_visit(graph, dest_node);
	}

	graph->topo_list = List_insert_first(graph->topo_list, node);
}

/* Assumes dominator has been performed */
int Graph_dfs_topo_sort(Graph graph) {
	GraphNode node;
	int count;

	if (graph->flags & GRAPHTOPOSORT) /* has already been sorted */
		return (List_size(graph->nodes) - List_size(graph->topo_list));

	/* Initialize the predecessor count */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		node->tflags = 0;
		node->info = -1;
	}

	if (graph->topo_list)
		graph->topo_list = List_reset(graph->topo_list);

	Graph_dfs_topo_visit(graph, graph->root);

	count = 0;
	List_start(graph->topo_list);
	while ((node = (GraphNode) List_next(graph->topo_list))) {
		count++;
		node->tflags = 0;
		node->info = count;
	}
	graph->flags |= GRAPHTOPOSORT;

	return (List_size(graph->nodes) - List_size(graph->topo_list));
}

static void Graph_bfs_accept_node(Graph g, GraphNode n, List *fl, GraphNode *dn) {
	GraphArc a;
	GraphNode ns;

	g->topo_list = List_insert_last(g->topo_list, n);
	n->tflags = GRAPH_NODE_VISIT_BLACK;
	List_start(n->succ);
	while ((a = (GraphArc) List_next(n->succ)))
		if ((ns = GraphArcDest(a))->tflags == GRAPH_NODE_VISIT_WHITE) {
			if (!(ns->flags & GRAPH_NODE_DEFER)) {
				(*fl) = List_insert_last((*fl), ns);
			} else {
				if (*dn)
					I_punt("Graph_bfs_toposort: multiple deferrals illegal");
				*dn = ns;
			}
			ns->tflags = GRAPH_NODE_VISIT_GREY;
		}

	return;
}

/*
 * Warning-- assumes the graph starts with a useless start node!
 * (specialized for Lblock purposes)
 */

int Graph_bfs_topo_sort(Graph g) {
	List frontier = NULL, new_frontier = NULL;
	GraphNode n, dn;
	GraphArc a;
	int progress;

	if (g->flags & GRAPHTOPOSORT) /* has already  been sorted */
		return (List_size(g->nodes) - List_size(g->topo_list));

	if (g->topo_list)
		g->topo_list = List_reset(g->topo_list);

	List_start(g->nodes);
	while ((n = (GraphNode) List_next(g->nodes)))
		n->tflags = GRAPH_NODE_VISIT_WHITE;

	if (g->root)
		Graph_bfs_accept_node(g, g->root, &frontier, &dn);

	dn = NULL;

	do {
		progress = 0;
		new_frontier = NULL;

		List_start(frontier);
		while ((n = (GraphNode) List_next(frontier))) {
			List_start(n->pred);
			while ((a = (GraphArc) List_next(n->pred))) {
				/* Find an edge from an unvisited node */

				if (GraphArcSrc(a)->tflags != GRAPH_NODE_VISIT_BLACK)
					break;
			}

			if (!a || (n->flags & GRAPH_NODE_HEADER)) {
				Graph_bfs_accept_node(g, n, &new_frontier, &dn);
				progress++;
			} else {
				new_frontier = List_insert_last(new_frontier, n);
			}

			frontier = List_delete_current(frontier);
		}

		frontier = new_frontier;

		if (dn && (!progress || !List_size(frontier))) {
			frontier = List_insert_last(frontier, dn);
			dn = NULL;
			progress = 1;
			continue;
		}
	} while (progress && List_size(frontier));

	if (!progress && List_size(frontier)) {
		/* BFS algorithm failed (tr contains loop). Use DFS */

		List_reset(frontier);

		List_start(g->nodes);
		while ((n = (GraphNode) List_next(g->nodes)))
			n->tflags = GRAPH_NODE_VISIT_BLACK;

		if (g->topo_list)
			g->topo_list = List_reset(g->topo_list);

		I_warn("BFS toposort failed.  Using DFS.");

		Graph_dfs_topo_sort(g);
	}

	g->flags |= GRAPHTOPOSORT;
	return (List_size(g->nodes) - List_size(g->topo_list));
}

int Graph_topological_sort(Graph graph) {
	return Graph_dfs_topo_sort(graph);
}

void Graph_rev_topological_visit(Graph graph, GraphNode node) {
	GraphNode src_node;
	GraphArc arc;

	node->tflags = 1;

	List_start(node->pred);
	while ((arc = (GraphArc) List_next(node->pred))) {
		src_node = GraphArcSrc (arc);
		if (!src_node->tflags)
			Graph_rev_topological_visit(graph, src_node);
	}

	graph->rev_topo_list = List_insert_first(graph->rev_topo_list, node);

	return;
}

/* Assumes dominator has been performed */
void Graph_rev_topological_sort(Graph graph) {
	GraphNode node, stop = NULL;
	int count;

	if (graph->flags & GRAPHREVTOPOSORT) /* hasn't  been sorted */
		return;

	if (!(graph->flags & GRAPHDOM))
		Graph_dominator(graph); /* since it hasn't been run lets run it */

	/* Initialize the predecessor count */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		node->tflags = 0;
		node->info = -1;
		if (!node->succ) {
			if (stop)
				I_punt("Graph_rev_topological_sort: multiple stop nodes");
			else
				stop = node;
		}
	}

	if (graph->rev_topo_list)
		graph->rev_topo_list = List_reset(graph->rev_topo_list);

	Graph_rev_topological_visit(graph, stop);

	count = 0;
	List_start(graph->rev_topo_list);
	while ((node = (GraphNode) List_next(graph->rev_topo_list))) {
		count++;
		node->tflags = 0;
		node->info = count;
	}
	graph->flags |= GRAPHREVTOPOSORT;
}

void Graph_preorder_dfs_visit(Graph graph, GraphNode node, int directed) {
	GraphNode dest_node;
	GraphArc arc;

	node->tflags = GRAPH_NODE_VISIT_GREY;

	graph->dfs_list = List_insert_last(graph->dfs_list, node);

	List_start(node->succ);
	while ((arc = (GraphArc) List_next(node->succ))) {
		dest_node = GraphArcDest (arc);
		if (directed && (dest_node->tflags == GRAPH_NODE_VISIT_GREY))
			arc->flags |= GRAPH_ARC_BACKEDGE;
		if (dest_node->tflags == GRAPH_NODE_VISIT_WHITE)
			Graph_preorder_dfs_visit(graph, dest_node, directed);
	}

	if (!directed) {
		List_start(node->pred);
		while ((arc = (GraphArc) List_next(node->pred))) {
			dest_node = GraphArcSrc (arc);
			if (dest_node->tflags == GRAPH_NODE_VISIT_WHITE)
				Graph_preorder_dfs_visit(graph, dest_node, directed);
		}
	}

	node->tflags = GRAPH_NODE_VISIT_BLACK;
}

void Graph_preorder_dfs_sort(Graph graph, int directed) {
	GraphNode node;
	int count;

	if (graph->flags & GRAPHDFSPRESORT) /* hasn't  been sorted */
		return;

	if (!(graph->flags & GRAPHDOM))
		Graph_dominator(graph); /* since it hasn't been run lets run it */

	/* Initialize the predecessor count */
	List_start(graph->nodes);
	while ((node = (GraphNode) List_next(graph->nodes))) {
		node->tflags = GRAPH_NODE_VISIT_WHITE;
		node->info = -1;
	}

	if (graph->dfs_list)
		graph->dfs_list = List_reset(graph->dfs_list);

	Graph_preorder_dfs_visit(graph, graph->root, directed);

	count = 0;
	List_start(graph->dfs_list);
	while ((node = (GraphNode) List_next(graph->dfs_list))) {
		count++;
		node->tflags = GRAPH_NODE_VISIT_WHITE;
		node->info = count;
	}
	graph->flags |= GRAPHDFSPRESORT;
}
