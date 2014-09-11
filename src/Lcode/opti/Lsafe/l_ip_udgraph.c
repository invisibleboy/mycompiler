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
/*****************************************************************************\
 *
 *  File:  l_ip_udgraph.c
 *
 *  Description:  
 *    Traverses the use-def chains that lead to an instruction.
 *
 *  Creation Date :  February 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *
 *  Copyright (c) 1993 Roger A. Bringmann , Wen-mei Hwu and The Board of
 *		  Trustees of the University of Illinois. 
 *		  All rights reserved.
 *
 *  The University of Illinois software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_interproc.h"

/******************************************************************************\
 *
 * def_oper support routines
 *
\******************************************************************************/

void L_ud_free_all_def_oper(Def_Oper *def_oper)
{
    Def_Oper	*oper, *next_op;

    for (oper = def_oper; oper != NULL; oper = next_op)
    {
	next_op = oper->next_op;
	L_free(L_alloc_def_oper, oper);
    }
}

Def_Oper *L_ud_new_def_oper(L_Oper *oper)
{
    Def_Oper	*def;

    def = (Def_Oper *)L_alloc(L_alloc_def_oper);
    def->oper = oper;
    def->prev_op = NULL;
    def->next_op = NULL;

    return def;
}

/* This routine insert the first oper in a node list */
void L_ud_insert_def_oper_first(UD_Node *node, Def_Oper *first_op)
{
    if (node->first_op == NULL)
    {
	node->first_op = first_op;
	node->last_op = first_op;
    }
    else
    {
        first_op->next_op = node->first_op;
	node->first_op->prev_op = first_op;
        node->first_op = first_op;
    }
}

void L_ud_print_def_oper(Def_Oper *def_oper)
{
    fprintf (stdout, " (op %d %s)", def_oper->oper->id, def_oper->oper->opcode);
}

/******************************************************************************\
 *
 * def_list support routines
 *
\******************************************************************************/

Def_List *L_ud_new_def_list()
{
    Def_List	*list;

    list = (Def_List *)L_alloc(L_alloc_def_list);
    list->first_op = NULL;
    list->last_op = NULL;

    return list;
}

void L_ud_free_def_list(Def_List *list)
{
    if (!list) return;

    L_ud_free_all_def_oper(list->first_op);

    L_free(L_alloc_def_list, list);
}

void L_ud_update_def_list(Def_List *list, L_Oper *oper)
{
    Def_Oper *def_oper;

    if (oper==NULL) return;

    if (!list)
	L_punt ("L_ud_update_def_list: no list specified\n");

    def_oper = L_ud_new_def_oper(oper);

    if (list->first_op == NULL)
    {
	list->first_op = def_oper;
	list->last_op = def_oper;
    }
    else
    {
	list->first_op->prev_op = def_oper;
	def_oper->next_op = list->first_op;
	list->first_op = def_oper;
    }
}

Def_List *L_ud_copy_def_list(Def_List *old)
{
    Def_List *def_list;
    Def_Oper *def_oper;

    def_list = L_ud_new_def_list();

    if (old)
    {
        for (def_oper = old->first_op; def_oper != NULL; 
    	     def_oper = def_oper->next_op)
        {
	    L_ud_update_def_list(def_list, def_oper->oper);
        }
    }

    return def_list;
}

/******************************************************************************\
 *
 * Memory support
 *
\******************************************************************************/
int L_ud_find_matching_hash(Def_List *list, L_Oper *oper)
{
    Def_Oper 	*def_oper;
    Memory_Cell	*load_cell, *store_cell;

    for (def_oper = list->first_op; def_oper != NULL; 
	 def_oper = def_oper->next_op)
    {
	if ((store_cell = L_mem_find_hash(oper->src[0], 
	     oper->src[1])) == NULL)
	    continue;

	if ((load_cell = L_mem_find_hash(def_oper->oper->src[0], 
	     def_oper->oper->src[1])) == NULL)
	    continue;

	if (store_cell == load_cell)
	    return 1;
    }

    return 0;
}

/* Searches for the first match */
int L_ud_find_matching_address(Def_List *list, L_Oper *oper)
{
    Def_Oper *def_oper;

    for (def_oper = list->first_op; def_oper != NULL; 
	 def_oper = def_oper->next_op)
    {
	if (L_mem_same_address(def_oper->oper->src[0], 
	    def_oper->oper->src[1], oper->src[0], oper->src[1]))
	    return 1;
    }

    return 0;
}

/* Removes all matches */
void L_ud_remove_matching_address(Def_List *list, L_Oper *oper)
{
    Def_Oper *def_oper, *next_def_oper;

    for (def_oper = list->first_op; def_oper != NULL; def_oper = next_def_oper)
    {
	next_def_oper = def_oper->next_op;

	if (L_mem_same_address(def_oper->oper->src[0], 
	    def_oper->oper->src[1], oper->src[0], oper->src[1]))
	{
	    if (list->first_op == def_oper)
		list->first_op = def_oper->next_op;

	    if (list->last_op == def_oper)
		list->last_op = def_oper->prev_op;

	    if (def_oper->prev_op)
		def_oper->prev_op->next_op = def_oper->next_op;

	    if (def_oper->next_op)
		def_oper->next_op->prev_op = def_oper->prev_op;

	    L_free(L_alloc_def_oper, def_oper);
	}
    }
}

/******************************************************************************\
 *
 * Various support routines
 *
\******************************************************************************/

int L_ud_ignore_oper(L_Oper *oper)
{
    switch (oper->opc)
    {
	case Lop_NO_OP:
	case Lop_PROLOGUE:
	case Lop_EPILOGUE:
	case Lop_DEFINE:
	    return 1;

	default:
	    return 0;
    }
}

#if 0
static int L_ud_jsr_returns_struct (L_Oper *oper)
{
    if (oper==NULL)
	return 0;

    if (!L_subroutine_call_opcode(oper))
	return 0;

    if (L_find_attr(oper->attr, "ret_st") == NULL)
	return 0;

    return 1;
}
#endif

int L_ud_convert_operand(L_Operand *operand)
{
    if (!operand) return -1;

    if (L_is_register(operand))
	return L_REG_INDEX(operand->value.r);

    else if (L_is_macro(operand))
    {
	if (M_is_stack_operand(operand))
	    return -1;
	else
	    return L_MAC_INDEX(operand->value.mac);
    }

    else
	return -1;
}

Set L_ud_add_src(Set set, L_Operand *operand)
{
    int src;

    src = L_ud_convert_operand(operand);
    if (src!=-1) set = Set_add (set, src);

    return set;
}

Set L_ud_add_src_reg(Set set, int src)
{
    return Set_add (set, src);
}

Set L_ud_remove_dest(Set set, int dest)
{
    return Set_delete(set, dest);
}

L_Oper *L_ud_find_start_pt(L_Cb *src_cb, L_Cb *dst_cb)
{
    L_Flow 	*flow, *last_flow = NULL;
    L_Oper	*start_pt;

    for (flow=src_cb->dest_flow; flow!=NULL; flow=flow->next_flow)
	if (flow->dst_cb == dst_cb) last_flow = flow;;

    start_pt = L_find_branch_for_flow(src_cb, last_flow);
    if (start_pt == NULL)
	return src_cb->last_op;
    else
	return start_pt;
}

/******************************************************************************\
 *
 * UD_Arc support routines
 *
\******************************************************************************/

/* This routine adds an arc from the src_node to the dst_node and back */
void L_add_arc(UD_Node *src_node, UD_Node *dst_node)
{
    UD_Arc	*arc;

    if (!src_node)
	L_punt ("L_src_arc: no src_node specified");

    if (!dst_node)
	L_punt ("L_src_arc: no dst_node specified");

    /* Create the arc */ 
    arc = (UD_Arc *)L_alloc(L_alloc_arc);

    /* Link the arc into the chain of src arcs */
    arc->src_node = src_node;
    arc->prev_dst_arc = NULL;
    if (src_node->dst_arc)
    {
        arc->next_dst_arc = src_node->dst_arc;
	src_node->dst_arc->prev_dst_arc = arc;
    }
    else
	arc->next_dst_arc = NULL;
    src_node->dst_arc = arc;

    /* Link the arc into the chain of src arcs */
    arc->dst_node = dst_node;
    arc->prev_src_arc = NULL;
    if (dst_node->src_arc)
    {
        arc->next_src_arc = dst_node->src_arc;
	dst_node->src_arc->prev_src_arc = arc;
    }
    else
	arc->next_src_arc = NULL;
    dst_node->src_arc = arc;
}

/* 
 * This routine adds an arc from the src_node to the dst_node and back 
 * as long as it is not a duplicate arc.
 */
int L_add_non_duplicate_arc(UD_Node *src_node, UD_Node *dst_node)
{
    UD_Arc	*arc;

    if (!src_node)
	L_punt ("L_add_non_duplicate_arc: no src_node specified");

    if (!dst_node)
	L_punt ("L_add_non_duplicate_arc: no dst_node specified");

    /* Return 0 to indicate no arc added if an arc already exists */
    for (arc = src_node->dst_arc; arc != NULL; arc = arc->next_dst_arc)
	if (arc->dst_node == dst_node) return 0;

    L_add_arc(src_node, dst_node);

    /* 1 returned indicates that an arc was added */
    return 1;
}

void L_delete_arc(UD_Arc *arc)
{
    UD_Node	*node;

    if (!arc) return;

    /* Remove the link from the src node */
    node = arc->src_node;

    if (node->dst_arc == arc)
	node->dst_arc = arc->next_dst_arc;

    if (arc->prev_dst_arc)
	arc->prev_dst_arc->next_dst_arc = arc->next_dst_arc;

    if (arc->next_dst_arc)
	arc->next_dst_arc->prev_dst_arc = arc->prev_dst_arc;

    /* Remove the link to the dst node */
    node = arc->dst_node;

    if (node->src_arc == arc)
	node->src_arc = arc->next_src_arc;

    if (arc->prev_src_arc)
	arc->prev_src_arc->next_src_arc = arc->next_src_arc;

    if (arc->next_src_arc)
	arc->next_src_arc->prev_src_arc = arc->prev_src_arc;

    L_free(L_alloc_arc, arc);

}

void L_free_all_src_arcs(UD_Arc *first_arc)
{
    UD_Arc	*arc, *next_arc;
    
    for (arc = first_arc; arc!=NULL; arc=next_arc)
    {
	next_arc = arc->next_src_arc;

	L_delete_arc(arc);
    }
}

void L_free_all_dst_arcs(UD_Arc *first_arc)
{
    UD_Arc	*arc, *next_arc;
    
    for (arc = first_arc; arc!=NULL; arc=next_arc)
    {
	next_arc = arc->next_dst_arc;

	L_delete_arc(arc);
    }
}

/******************************************************************************\
 *
 * UD_Node support routines
 *
\******************************************************************************/

UD_Node *L_ud_new_node(UD_Graph *graph, L_Cb *cb, L_Oper *oper)
{
    UD_Node	*node;

    node = (UD_Node *)L_alloc(L_alloc_node);
    node->cb = cb;
    node->start_oper = oper;
    node->term_oper = NULL;

    node->first_op = NULL;
    node->last_op = NULL;

    node->src_arc = NULL;
    node->dst_arc = NULL;

    node->next_snode = NULL;
    node->prev_snode = NULL;

    /* Used to simplify memory reclamation */
    if (graph->allocated_node_list)
    {
        node->next_node = graph->allocated_node_list;
	node->prev_node = NULL;

	graph->allocated_node_list->prev_node = node;
        graph->allocated_node_list = node;
    }
    else
    {
	graph->allocated_node_list = node;
	node->next_node = NULL;
	node->prev_node = NULL;
    }

    return node;
}

void L_ud_print_node(UD_Node *node)
{
    Def_Oper	*def_oper;
    UD_Arc	*arc;

    fprintf (stdout, "[(cb %d)(cb start point op %d %s):", node->cb->id,
        node->start_oper->id, node->start_oper->opcode);

    if (node->first_op)
    {
        for (def_oper = node->first_op; def_oper!=NULL; def_oper=def_oper->next_op)
        {
            fprintf (stdout, "\n");

	    L_ud_print_def_oper(def_oper);
        }
        fprintf (stdout, "]\n");
    }
    else
        fprintf (stdout, "]\n");

    if (node->src_arc)
    {
	fprintf (stdout, "    src CB's ( ");
	for (arc = node->src_arc; arc != NULL; arc = arc->next_src_arc)
	{
	    fprintf (stdout, "%d ", arc->src_node->cb->id);
	}
	fprintf (stdout, ")\n");
    }
    else
	fprintf (stdout, "    no src arcs\n");

    if (node->dst_arc)
    {
	fprintf (stdout, "    dst CB's ( ");
	for (arc = node->dst_arc; arc != NULL; arc = arc->next_dst_arc)
	{
	    fprintf (stdout, "%d ", arc->dst_node->cb->id);
	}
	fprintf (stdout, ")\n");
    }
    else
	fprintf (stdout, "    no dst arcs\n");
}

void L_ud_delete_node(UD_Graph *graph, UD_Node *node)
{

  L_free_all_src_arcs(node->src_arc);

  L_free_all_dst_arcs(node->dst_arc);

  /* Remove node from memory freeing list */
  if (node == graph->allocated_node_list)
    graph->allocated_node_list = node->next_node;

    if (node->next_node!=NULL)
	node->next_node->prev_node = node->prev_node;

    if (node->prev_node!=NULL)
	node->prev_node->next_node = node->next_node;

    /* Remove node from start node list */
    if (node == graph->start_node_list)
	graph->start_node_list = node->next_snode;

    if (node->prev_snode!=NULL)
      node->prev_snode->next_snode = node->next_snode;

    if (node->next_snode!=NULL)
      node->next_snode->prev_snode = node->prev_snode;

    /* Now release the node */
    L_free(L_alloc_node, node);
}


/******************************************************************************\
 *
 * Retraced path support routines
 *
\******************************************************************************/

void L_ud_free_all_retrace(UD_Graph *graph)
{
    int i;
    Retrace	*retrace, *next_retrace;

    for (i=0; i<graph->num_cb_use_sets; i++)
    {
	for (retrace = graph->cb_sets[i]; retrace!=NULL; retrace=next_retrace)
	{
	    next_retrace = retrace->next_retrace;
	    Set_dispose(retrace->src_reg_set);
	    retrace->src_reg_set=NULL;
	    L_ud_free_def_list(retrace->loads);
	    L_free(L_alloc_retrace, retrace);
	    graph->cb_sets[i] = NULL;
	}
    }

    if (graph->cb_sets)
    {
        free(graph->cb_sets);
        graph->cb_sets = NULL;
    }
    graph->num_cb_use_sets = 0;
}

Retrace *L_ud_new_retrace(UD_Node *src_node, L_Cb *cb, 
    L_Oper *oper, Set src_reg_set, Def_List *ld_list)
{
    Retrace 	*retrace;

    retrace = (Retrace *)L_alloc(L_alloc_retrace);

    retrace->src_node = src_node;

    retrace->cb = cb;

    retrace->start_oper = oper;

    retrace->src_reg_set = Set_union(NULL, src_reg_set);

    retrace->loads = L_ud_copy_def_list(ld_list);

    retrace->next_retrace = NULL;

    return retrace;
}

UD_Node *L_ud_find_retrace(UD_Graph *graph, L_Cb *cb, L_Oper *oper, 
    Set src_reg_set, Def_List *list)
{
    int 	i, match;
    Retrace 	*retrace;
    Def_Oper	*def_oper;

    for (retrace = graph->cb_sets[cb->id]; retrace != NULL; 
	retrace=retrace->next_retrace)
    {
	if (retrace->cb != cb) continue;

	if (retrace->start_oper != oper) continue;

	if (!Set_subtract_empty(src_reg_set, retrace->src_reg_set))
	    continue;

	match = 0;
	i = 0; 
	for(def_oper=list->first_op; def_oper != NULL; 
	    def_oper = def_oper->next_op)
	{
	    i++;
	    if (L_ud_find_matching_address(retrace->loads, def_oper->oper))
		match++;
	}
	if (match != i) continue;

	return retrace->src_node;
    }

    return NULL;
}

void L_ud_add_retrace(UD_Graph *graph, UD_Node *src_node, L_Cb *cb, 
    L_Oper *oper, Set src_reg_set, Def_List *list)
{
    Retrace *retrace;

    retrace = L_ud_new_retrace(src_node, cb, oper, src_reg_set, list); 

    retrace->next_retrace = graph->cb_sets[cb->id];

    graph->cb_sets[cb->id] = retrace;
}

/******************************************************************************\
 *
 * UD_Graph support routines
 *
\******************************************************************************/

UD_Graph *L_ud_new_graph(L_Func *fn, L_Cb *cb, L_Oper *oper, L_Oper *prev_op)
{
    UD_Graph	*new_graph;

    new_graph = (UD_Graph *)L_alloc(L_alloc_ud_graph);
    new_graph->start_node_list = NULL;
    new_graph->allocated_node_list = NULL;

    new_graph->end_node = L_ud_new_node(new_graph, cb, prev_op);
    L_ud_insert_def_oper_first(new_graph->end_node, L_ud_new_def_oper(oper));
    new_graph->end_node->term_oper = oper;

    new_graph->num_cb_use_sets = fn->max_cb_id+1;
    new_graph->cb_sets = (Retrace **)calloc(new_graph->num_cb_use_sets, 
	sizeof(Retrace*));

    return new_graph;
}

void L_ud_add_start_node(UD_Graph *graph, UD_Node *cur_node)
{
    if (graph->start_node_list)
    {
	cur_node->next_snode = graph->start_node_list;
	graph->start_node_list->prev_snode = cur_node;
	graph->start_node_list = cur_node;
    }
    else
    {
	graph->start_node_list = cur_node;
    }
}

/******************************************************************************\
 *
 * Graph support routines
 *
\******************************************************************************/

void L_ud_print_graph_recurse(UD_Node *node)
{
    UD_Arc	*arc;

    L_ud_print_node(node);

    for (arc = node->dst_arc; arc!=NULL; arc = arc->next_dst_arc)
	L_ud_print_graph_recurse(arc->dst_node);
}

void L_usedef_print_graph(UD_Graph *graph)
{
    fprintf (stdout, "\n");

    L_ud_print_graph_recurse(graph->start_node_list);
}

/* 
 * This routine assumes no cycles.
 *
 * Remember that the graph was built starting at the leaf node.
 * Thus, src arcs are those coming from a parent node to a sibling
 * node and dst arcs are those coming from a child node to a parent
 * node.
 *
 * The convention is that at every node, the dst arcs are deleted.
 * Deleting src arcs requires recursing to delete src nodes as well!
 */
void L_usedef_delete_graph (UD_Graph *graph)
{
    UD_Node	*node, *next_node;

    for (node = graph->allocated_node_list; node !=NULL; node = next_node)
    {
	next_node = node->next_node;

	/* Free opers */
        L_ud_free_all_def_oper(node->first_op);

	/* Free src arcs */
	L_free_all_src_arcs(node->src_arc);

	/* Free dst arcs */
	L_free_all_dst_arcs(node->dst_arc);

        L_free(L_alloc_node, node);
    }

    L_ud_free_all_retrace(graph);

    L_free(L_alloc_ud_graph, graph);
}

/*
 * This function eliminates nodes that do not contribute definitions to
 * any path.
 */
void L_ud_reduce_graph_recurse(UD_Graph *graph, UD_Node *start_node)
{
    UD_Node	*node;
    UD_Arc	*dst_arc, *next_dst_arc = NULL, *to_arc, *from_arc;
    int		new_arc;


	for (dst_arc = start_node->dst_arc; dst_arc != NULL;
	     dst_arc = next_dst_arc)
	{
	    next_dst_arc = dst_arc->next_dst_arc;
	    node = dst_arc->dst_node;

	    if (node == graph->end_node) continue;

	    /* Remove empty nodes */
	    if (node->first_op==NULL)
	    {
		/* 
		 * Link nodes pointed to by src arcs to nodes pointed
		 * to by dst arcs.
		 */
	        new_arc = 0;
		for (from_arc = node->src_arc; from_arc != NULL; 
		     from_arc = from_arc->next_src_arc)
		{
		    for (to_arc = node->dst_arc; to_arc != NULL;
			 to_arc = to_arc->next_dst_arc)
		    {
			new_arc |= L_add_non_duplicate_arc(from_arc->src_node,
			    to_arc->dst_node);
		    }
		}

		/* Delete node */
		L_ud_delete_node(graph, node);

	        if (new_arc) next_dst_arc = start_node->dst_arc;
	    }
	}

    /* Now let's try one more level down */
    for (dst_arc = start_node->dst_arc; dst_arc != NULL; dst_arc = next_dst_arc)
    {
        L_ud_reduce_graph_recurse(graph, dst_arc->dst_node);
    }
}

/*
 * This function eliminates nodes that do not contribute definitions to
 * any path.
 */
void L_ud_reduce_graph(UD_Graph *graph)
{
    UD_Node	*start_node;

    for (start_node = graph->start_node_list; start_node != NULL; 
	 start_node = start_node->next_snode)
    {
	L_ud_reduce_graph_recurse(graph, start_node);
    }
}

void L_ud_build_graph_recurse(UD_Graph *graph, UD_Node *cur_node, 
     Set src_reg_set, Def_List *ld_list)
{
    int		dest = 0, src, src_cb_id, src_oper_id, i;
    L_Cb	*cur_cb = cur_node->cb, *src_cb;
    L_Flow	*flow;
    L_Oper	*cur_oper = cur_node->start_oper, *src_oper;
    Def_Oper	*new_def;
    Def_List	*new_ld_list;
    UD_Node	*src_node;
    Set		src_cbs=NULL, src_opers=NULL, new_src_set;
    L_Attr	*attr;

    /* Condition to show retraced path */

    /* Search current cb for definitions of current source operand set */
    for (cur_oper=cur_node->start_oper; cur_oper!=NULL; 
	 cur_oper=cur_oper->prev_op)
    {
        if (L_ud_ignore_oper(cur_oper)) continue;

	if (L_general_store_opcode(cur_oper))
	{
    	    if ((attr = L_find_attr(cur_oper->attr, "tm")) != NULL)
	    {
		/* 
		 * The store is equivalent to a move from a register to
		 * a macro.  We will convert the store to a move!
		 */
		cur_oper->opc = Lop_MOV;
		cur_oper->opcode = L_opcode_name(Lop_MOV);

		L_delete_operand(cur_oper->src[0]);
		L_delete_operand(cur_oper->src[1]);

		cur_oper->dest[0] = L_new_macro_operand(
		    attr->field[0]->value.mac, L_return_old_ctype(attr->field[0]), 0);
		cur_oper->src[0] = cur_oper->src[2];
		cur_oper->src[1] = NULL;
		cur_oper->src[2] = NULL;
	    }
	}
	else if (L_general_load_opcode(cur_oper))
	{
    	    if ((attr = L_find_attr(cur_oper->attr, "tm")) != NULL)
	    {
		/* 
		 * The load is equivalent to a move from a macro to
		 * a register.  We will convert the load to a move!
		 */
		cur_oper->opc = Lop_MOV;
		cur_oper->opcode = L_opcode_name(Lop_MOV);

		L_delete_operand(cur_oper->src[0]);
		L_delete_operand(cur_oper->src[1]);

		cur_oper->src[0] = L_new_macro_operand(
		    attr->field[0]->value.mac, L_return_old_ctype(attr->field[0]), 0);
		cur_oper->src[1] = NULL;

	    }
	}

	/* Determine what is being defined */
	if (L_subroutine_call_opcode(cur_oper))
	{
	    attr = L_find_attr(cur_oper->attr, "ret");

	    if (attr && (Set_in(src_reg_set, attr->field[0]->value.r)))
	    {
		src_reg_set = L_ud_remove_dest(src_reg_set, dest);

		if (!building_callgraph) 
		{
	            new_def = L_ud_new_def_oper(cur_oper);
	    	    L_ud_insert_def_oper_first(cur_node, new_def);

	    	    src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[0]);
	    	    src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[1]);
		
		    /* 
		     * Add parameter registers that are passed 
		     * through registers to the active list.
		     */
		    if ((attr=L_find_attr(cur_oper->attr, "tr"))!=NULL)
		    {
			for (i=0; i<attr->max_field; i++)
			{
		            src_reg_set = L_ud_add_src(src_reg_set, 
				attr->field[i]);
			}
		    }

		    /* 
		     * Add parameter registers that are passed 
		     * through memory to the active list.
		     */
		    if ((attr=L_find_attr(cur_oper->attr, "tm"))!=NULL)
		    {
			for (i=0; i<attr->max_field; i++)
			{
		            src_reg_set = L_ud_add_src_reg(src_reg_set, 
				attr->field[i]->value.mac);
			}
		    }
		}
	    }
	}
	else if (L_general_branch_opcode(cur_oper))
	{
	    /*
	     * A branch is added to the traversal path if either of
	     * its source operands are in the live set
	     */
            src = L_ud_convert_operand(cur_oper->src[0]);

	    if ((src!=-1) && (Set_in(src_reg_set, src)))
	    {
		new_def = L_ud_new_def_oper(cur_oper);
	    }
	    else
	    {
                src = L_ud_convert_operand(cur_oper->src[1]);

	        if ((src!=-1) && (Set_in(src_reg_set, src)))
	        {
		    new_def = L_ud_new_def_oper(cur_oper);
	        }
	    }
	}
	else if (L_general_store_opcode(cur_oper) &&
		 ld_list->first_op!=NULL)
	{
	    new_def = NULL;

            src = L_ud_convert_operand(cur_oper->src[0]);
	    if ((src!=-1) && (Set_in(src_reg_set, src)))
	    {
	        new_def = L_ud_new_def_oper(cur_oper);
	    }
	    else
	    {
                src = L_ud_convert_operand(cur_oper->src[1]);
	        if ((src!=-1) && (Set_in(src_reg_set, src)))
		{
	            new_def = L_ud_new_def_oper(cur_oper);
		}
	    }

	    if (new_def)
	    {
	        src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[2]);
	        L_ud_insert_def_oper_first(cur_node, new_def);

		L_ud_remove_matching_address(ld_list, cur_oper);
	    }
	    else
	    {
	        /*
	         * If this store address is the exact same as a load in the 
	         * current load set, remove the load from the set.
	         *
	         * To ensure accuracy since we do not have very good 
	         * disambiguation, we will add all stores to the
	         * list.
	         */
	        if (L_ud_find_matching_address(ld_list, cur_oper))
		{
		    L_ud_remove_matching_address(ld_list, cur_oper);

	            new_def = L_ud_new_def_oper(cur_oper);
	            src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[2]);
	            L_ud_insert_def_oper_first(cur_node, new_def);
		}
		else if (L_ud_find_matching_hash(ld_list, cur_oper))
		{
	            new_def = L_ud_new_def_oper(cur_oper);
	            src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[2]);
	            L_ud_insert_def_oper_first(cur_node, new_def);
		}
	    }
	}
	else if (L_general_load_opcode(cur_oper))
	{
            dest = L_ud_convert_operand(cur_oper->dest[0]);

	    if ((dest!=-1) && (Set_in(src_reg_set, dest)))
	    {
		src_reg_set = L_ud_remove_dest(src_reg_set, dest);

	        new_def = L_ud_new_def_oper(cur_oper);
		L_ud_insert_def_oper_first(cur_node, new_def);

		L_ud_update_def_list(ld_list, cur_oper);

		if (!building_callgraph ||
		    !L_mem_find_hash(cur_oper->src[0], cur_oper->src[1]))
		{
	    	    src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[0]);
	    	    src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[1]);
		}
	    }
	}
	else if (cur_oper->dest[0] != NULL)
	{
            dest = L_ud_convert_operand(cur_oper->dest[0]);

	    if ((dest!=-1) && (Set_in(src_reg_set, dest)))
	    {
		new_def = L_ud_new_def_oper(cur_oper);
		L_ud_insert_def_oper_first(cur_node, new_def);

		src_reg_set = L_ud_remove_dest(src_reg_set, dest);

		if (!building_callgraph ||
		    !L_mem_find_hash(cur_oper->src[0], cur_oper->src[1]))
		{
	    	    src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[0]);
	    	    src_reg_set = L_ud_add_src(src_reg_set, cur_oper->src[1]);
		}
	    }
	}
	else 
	    continue;

	if (Set_empty(src_reg_set) && (ld_list->first_op==NULL)) 
	    break;
    }

    /* 
     * Perform a depth-first search until terminal point is reached
     * or all source operands are defined.
     */
    if (((ld_list->first_op!=NULL) || (!Set_empty(src_reg_set))) && 
	(cur_cb->src_flow))
    {
	for (flow=cur_cb->src_flow; flow!=NULL; flow=flow->next_flow)
	{
	    /* Parent cb and start point from parent cb */
            src_cb = flow->src_cb;
            src_cb_id = src_cb->id;
	    src_oper = L_ud_find_start_pt(src_cb, cur_cb);
	    if (src_oper)
	        src_oper_id = src_oper->id;
	    else
		src_oper_id = 0;
    
            if (!Set_in(src_cbs, src_cb_id) && !Set_in(src_opers, src_oper_id))
            {
		/* Add this search path to those traversed */
                src_cbs = Set_add(src_cbs, src_cb_id);
                src_opers = Set_add(src_opers, src_oper_id);

		if ((src_node = L_ud_find_retrace(graph, src_cb, src_oper, 
		     src_reg_set, ld_list)) == NULL)
		{
		    /* Handle new path */
		    new_src_set = Set_union(NULL, src_reg_set);

		    new_ld_list = L_ud_copy_def_list(ld_list);

		    src_node = L_ud_new_node(graph, src_cb, src_oper);

		    L_add_arc(src_node, cur_node);

		    L_ud_add_retrace(graph, src_node, src_cb, src_oper, 
			new_src_set, new_ld_list);

		    L_ud_build_graph_recurse(graph, src_node, new_src_set, 
			new_ld_list);

		    L_ud_free_def_list(new_ld_list);

		    Set_dispose(new_src_set);
		}
            }
	}
	Set_dispose(src_cbs);
	Set_dispose(src_opers);
    }
    else
    {
	/* Add this node to the start node list */
	L_ud_add_start_node(graph, cur_node);
    }
}

UD_Graph *L_usedef_build_graph(CG_Node *cg_node, L_Cb *cb, L_Oper *oper, 
    int num_src_operands, L_Operand **src_operands, L_Oper *load)
{
    int		src, i;
    Set		src_reg_set;
    Def_List	*ld_list;
    UD_Graph	*graph;

    graph = L_ud_new_graph(cg_node->fn, cb, oper, oper->prev_op);

    L_do_flow_analysis(cg_node->fn, DOMINATOR);

    src_reg_set = NULL;
    for (i=0; i<num_src_operands; i++)
        if ((src = L_ud_convert_operand(src_operands[i]))!=-1)
            src_reg_set = Set_add(src_reg_set, src);

    ld_list = L_ud_new_def_list();
    if (L_general_load_opcode(oper))
	L_ud_update_def_list(ld_list, oper);

    if (L_general_load_opcode(load))
	L_ud_update_def_list(ld_list, load);

    L_ud_add_retrace(graph, NULL, cb, oper, NULL, ld_list);

    L_ud_build_graph_recurse(graph, graph->end_node, src_reg_set, ld_list);
    
    Set_dispose(src_reg_set);

    L_ud_free_def_list(ld_list);

    L_ud_free_all_retrace(graph);

    L_ud_reduce_graph(graph);

    return graph;
}

