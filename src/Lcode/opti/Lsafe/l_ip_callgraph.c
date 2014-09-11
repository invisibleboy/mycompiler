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
 *  File:  l_ip_callgraph.c
 *
 *  Description:  
 *	Build program call graph.  This program is intended to be run one
 *      time to generate a file containing the program call graph.  Support
 *      routines within this file can then be used to read in the call graph
 *      and manipulate it.
 *
 *      It is possible for there to be more than one root node in the call
 *      graph as a result of dead code that directly results for the programmer
 *      or function inlining.
 *
 *      The program attempts to identify all functions that can be called.  It
 *      is possible that a function is not known at the jsr call.  In this case,
 *      use-def chains are search backwards from the function call to determine
 *      the function being called.  
 *
 *  Creation Date :  May 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *
 *  Copyright (c) 1994 Roger A. Bringmann , Wen-mei Hwu and The Board of
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

#define HACK_022

char 		*input_filename;
CG_Node		*unknown_func=NULL;

CallGraph	*callgraph = NULL;

/******************************************************************************\
 *
 * Call graph arcs
 *
\******************************************************************************/

int	total_arcs = 0;

void L_cg_add_arc(CG_Node *src_node, L_Oper *oper, L_Operand *operand)
{
    CG_Node 	*dst_node;
    CG_Arc	*arc;

    if (L_is_label(operand))
    {
	dst_node = L_cg_find_node(operand->value.l);
    }
    else
    {
	dst_node = callgraph->unknown_node;
	src_node->total_unknown++;

	callgraph->total_unknown_jsr_static++;
	callgraph->total_unknown_jsr_dynamic+=oper->weight;
    }

    callgraph->total_jsr_static++;
    callgraph->total_jsr_dynamic+=oper->weight;

    arc = (CG_Arc *)L_alloc(L_alloc_cg_arc);
    arc->arc_id = total_arcs++;
    arc->jsr_id = oper->id;
    arc->weight = oper->weight;

    /* Append the arc to the end of the chain of dst arcs for this src node */
    arc->src_node = src_node;
    arc->next_dst_arc = NULL;

    if (!src_node->first_dst_arc)
    {
	src_node->first_dst_arc = arc;
    }

    if (src_node->last_dst_arc)
    {
	arc->prev_dst_arc = src_node->last_dst_arc;
	src_node->last_dst_arc->next_dst_arc = arc;
    }
    else
    {
	arc->prev_dst_arc = NULL;
    }
    src_node->last_dst_arc = arc;

    /* Append the arc to the end of the chain of src arcs for this dst node */
    arc->dst_node = dst_node;
    arc->next_src_arc = NULL;

    if (!dst_node->first_src_arc)
	dst_node->first_src_arc = arc;

    if (dst_node->last_src_arc)
    {
	arc->prev_src_arc = dst_node->last_src_arc;
	dst_node->last_src_arc->next_src_arc = arc;
    }
    else
    {
	arc->prev_src_arc = NULL;
    }
    dst_node->last_src_arc = arc;

    /* Now add this link into the interprocedural database */
    L_db_add_callsite(dst_node->func_id, src_node->func_id, 
	src_node->func_name, oper->id);
}

void L_cg_add_non_dup_arc(CG_Node *src_node, 
    L_Oper *oper, L_Operand *operand)
{
    CG_Node 	*dst_node;
    CG_Arc	*arc;
    char	*name;
    int		jsr_found;

    /* First determine if this arc already exists */
    if (L_is_label(operand))
	name = M_fn_name_from_label(operand->value.l);
    else
	name = CG_UNKNOWN_FUNC_NAME;
    
    jsr_found = 0;
    for (arc = src_node->first_dst_arc; arc != NULL; arc = arc->next_dst_arc)
    {
	if (oper->id != arc->jsr_id) continue;

	jsr_found = 1;
	if (!strcmp(arc->dst_node->func_name, name))
	    return;
    }

    /* Since the arc did not exist, we will create it */
    if (L_is_label(operand))
    {
	dst_node = L_cg_find_node(operand->value.l);
    }
    else
    {
	dst_node = callgraph->unknown_node;
	src_node->total_unknown++;

	callgraph->total_unknown_jsr_static++;
	callgraph->total_unknown_jsr_dynamic+=oper->weight;
    }

    if (!jsr_found)
    {
        callgraph->total_jsr_static++;
        callgraph->total_jsr_dynamic+=oper->weight;
    }

    arc = (CG_Arc *)L_alloc(L_alloc_cg_arc);

    arc->arc_id = total_arcs++;
    arc->jsr_id = oper->id;
    arc->weight = oper->weight;

    /* Append the arc to the end of the chain of dst arcs for this src node */
    arc->src_node = src_node;
    arc->next_dst_arc = NULL;

    if (!src_node->first_dst_arc)
	src_node->first_dst_arc = arc;

    if (src_node->last_dst_arc)
    {
	arc->prev_dst_arc = src_node->last_dst_arc;
	src_node->last_dst_arc->next_dst_arc = arc;
    }
    else
    {
	arc->prev_dst_arc = NULL;
    }
    src_node->last_dst_arc = arc;

    /* Append the arc to the end of the chain of src arcs for this dst node */
    arc->dst_node = dst_node;
    arc->next_src_arc = NULL;

    if (!dst_node->first_src_arc)
	dst_node->first_src_arc = arc;

    if (dst_node->last_src_arc)
    {
	arc->prev_src_arc = dst_node->last_src_arc;
	dst_node->last_src_arc->next_src_arc = arc;
    }
    else
    {
	arc->prev_src_arc = NULL;
    }
    dst_node->last_src_arc = arc;

    /* Now add this link into the interprocedural database */
    L_db_add_callsite(dst_node->func_id, src_node->func_id, 
	src_node->func_name, oper->id);
}

void L_cg_delete_arc(CG_Arc *arc)
{
    CG_Node	*node;

    if (!arc) return;

    /* Remove the link from the src node */
    node = arc->src_node;

    if (node->first_dst_arc == arc)
        node->first_dst_arc = arc->next_dst_arc;

    if (node->last_dst_arc == arc)
        node->last_dst_arc = arc->prev_dst_arc;

    if (arc->prev_dst_arc)
        arc->prev_dst_arc->next_dst_arc = arc->next_dst_arc;

    if (arc->next_dst_arc)
        arc->next_dst_arc->prev_dst_arc = arc->prev_dst_arc;

    /* Remove the link to the dst node */
    node = arc->dst_node;

    if (node->first_src_arc == arc)
        node->first_src_arc = arc->next_src_arc;

    if (node->last_src_arc == arc)
        node->last_src_arc = arc->prev_src_arc;

    if (arc->prev_src_arc)
        arc->prev_src_arc->next_src_arc = arc->next_src_arc;

    if (arc->next_src_arc)
        arc->next_src_arc->prev_src_arc = arc->prev_src_arc;

    L_free(L_alloc_cg_arc, arc);
}

void L_cg_delete_all_dst_arcs(CG_Arc *first_dst_arc)
{
    CG_Arc	*arc, *next_dst_arc;

    for (arc = first_dst_arc; arc != NULL; arc = next_dst_arc)
    {
	next_dst_arc = arc->next_dst_arc;

	L_cg_delete_arc(arc);
    }
}

void L_cg_delete_all_src_arcs(CG_Arc *first_src_arc)
{
    CG_Arc	*arc, *next_src_arc;

    for (arc = first_src_arc; arc != NULL; arc = next_src_arc)
    {
	next_src_arc = arc->next_src_arc;

	L_cg_delete_arc(arc);
    }
}

/******************************************************************************\
 *
 * Call graph nodes
 *
\******************************************************************************/

CG_Node *L_cg_new_node(char *name)
{
    CG_Node *node;

    node = (CG_Node *)L_alloc(L_alloc_cg_node);

    node->func_id = callgraph->total_nodes++;

    node->func_name = name;

    if (L_fn)
        node->flags = L_fn->flags;
    else
	node->flags = 0;

    if (L_name_in_side_effect_free_func_table(name))
    {
	node->flags = L_SET_BIT_FLAG(node->flags, L_FUNC_SIDE_EFFECT_FREE);

	callgraph->total_sef_func_static++;
    }

    if (L_fn && !strcmp(L_fn->name, name))
        node->filename = input_filename;
    else
	node->filename = NULL;

    node->fn = NULL; 
    node->total_unknown=0;

    node->local_refs = L_mem_new_memory(FUNCTION);
    node->global_refs = L_mem_new_memory(FUNCTION);
    node->ptr_refs = L_mem_new_memory(FUNCTION);

    node->first_src_arc = NULL;
    node->last_src_arc = NULL;

    node->first_dst_arc = NULL;
    node->last_dst_arc = NULL;

    node->prev_node = NULL;
    node->next_node = NULL;

    L_database_add_entry(node->func_id, node->func_name);

    return node;
}

void L_cg_add_node(CG_Node *node)
{
    /* Assuming C convention! */
    if (!strcmp("_main", node->func_name))
	callgraph->root_node = node;

    if (callgraph->last_node)
    {
	callgraph->last_node->next_node = node;
	node->prev_node = callgraph->last_node;
	callgraph->last_node = node;
    }
    else
    {
	callgraph->first_node = node;
	callgraph->last_node = node;
    }
}

void L_cg_delete_node(CG_Node *node)
{
    L_cg_delete_all_src_arcs(node->first_src_arc);
    L_cg_delete_all_dst_arcs(node->first_dst_arc);
    L_mem_delete(node->global_refs);
    L_mem_delete(node->local_refs);
    L_mem_delete(node->ptr_refs);
    L_free(L_alloc_cg_node, node);
}

/******************************************************************************\
 *
 * CallGraph support routines
 *
\******************************************************************************/

CallGraph *L_cg_new_callgraph()
{
    callgraph = (CallGraph *)malloc(sizeof(CallGraph));
    callgraph->root_node = NULL;
    callgraph->first_node = NULL;
    callgraph->last_node = NULL;
    callgraph->total_nodes = 0;		/* Total number of functions */
    callgraph->unknown_node = 
	L_cg_new_node(CG_UNKNOWN_FUNC_NAME);

    /* Initialize varialbles used for statistics */
    callgraph->total_jsr_static = 0;
    callgraph->total_jsr_dynamic = 0.0;

    callgraph->total_unknown_jsr_static = 0;
    callgraph->total_unknown_jsr_dynamic = 0.0;

    callgraph->total_sef_jsr_static = 0;
    callgraph->total_sef_jsr_dynamic = 0.0;

    callgraph->total_sef_func_static = 0;

    return callgraph;
}

void L_callgraph_delete(CallGraph *callgraph)
{
    CG_Node	*node, *next_node;

    if (!callgraph)
	L_punt ("L_callgraph_delete: null callgraph provided\n");

    for (node = callgraph->first_node; node != NULL; node = next_node)
    {
	next_node = node->next_node;
	L_cg_delete_node(node);
    }

    L_cg_delete_node(callgraph->unknown_node);

    L_mem_delete(global_memory);
    free(callgraph);
    callgraph = NULL;

    L_mem_delete_hash();
}

/******************************************************************************\
 *
 * Support routines for func list 
 *
\******************************************************************************/

CG_Node *L_cg_find_node(char *fn_name)
{
    CG_Node 	*node;
    char	*name;

    name = M_fn_name_from_label(fn_name);

    /* If we need the unknown func, return its func id */
    if (!strcmp(callgraph->unknown_node->func_name, name))
	return callgraph->unknown_node;

    /* Determine if the func exists in the list */
    for (node = callgraph->first_node; node != NULL; node = node->next_node)
    {
	if (!strcmp(node->func_name, name))
	{
	    if (!node->filename && L_fn && !strcmp(L_fn->name, name))
		node->filename = input_filename;

	    return node;
	}
    }

    /* 
     * Since the func does not exist in the list, we will create an entry
     * for it.
     */
    node = L_cg_new_node(name);
    L_cg_add_node(node);

    return node;
}

void L_cg_load_func (CG_Node *node)
{
    if (node->fn)
    {
	L_fn = node->fn;
        return;
    }

    if (!strcmp(node->func_name, CG_UNKNOWN_FUNC_NAME))
        L_punt("L_cg_load_func: called with CG_UNKNOWN_FUNC_NAME");

    if (node->filename == NULL)
	return;

    L_open_input_file(node->filename);

    L_seek(node->file_offset);
    /*
    Cl_seek (node->file_offset);
    */

    L_get_input();

    L_close_input_file(node->filename);

    L_compute_oper_weight(L_fn, 0, 1);
    node->fn = L_fn;
    L_do_flow_analysis(node->fn, DOMINATOR);

    return;
}

/******************************************************************************\
 *
 * The objective of these two routines is to make sure that the necessary
 * global variables get initialized before they are used later!
 *
 * This will prove that it is possible to resolve the variables given 
 * sufficient time and the right algorithm.
 *
 * The problem only shows up on 085.cc1 for now!
 *
\******************************************************************************/

#if 0
void L_cg_hack_resolve_mem(char* func_name, char* label)
{
    CG_Node	*node;
    L_Func	*old_L_fn;
    L_Cb	*cb;
    L_Oper	*rts, *load;
    L_Operand	*src0, *src1;
    int		old_level;

    node = L_cg_find_node(func_name);

    old_L_fn = L_fn;
    L_cg_load_func(node);

    load = L_create_new_op(Lop_LD_I);
    load->src[0] = L_new_gen_label_operand(label);
    load->src[1] = L_new_gen_int_operand(0);

    rts = L_find_rts(node->fn);

    cb = L_oper_hash_tbl_find_cb(node->fn->oper_hash_tbl, rts->id);

    old_level = L_analysis_level; 

    L_analysis_level = INTER_PROCEDURAL_ANALYSIS;
    L_resolve_unknown(node, cb, rts, 0, NULL, load);

    L_analysis_level = old_level;

    L_delete_oper(NULL, load);

    L_fn = old_L_fn;
}

/*
 * WARNING - it is assumed that function checked for occurs prior to
 * any of the other functions that also use the variable declarations.
 *
 * 1)  variables for __obstack_newchunk are also needed for
 *     __obstack_free.  These initializations can not occur until after
 *     __obstack_begin has been executed.
 *
 * 2)  variables required for _expand_expr are also needed for _do_jump
 *     and ___IMPACT_ST_13_344_do_store_flag.
 */
void L_cg_085_hack(CG_Node *cur_node)
{
    CG_Node	*node;

    if (!strcmp(cur_node->func_name, "__obstack_newchunk"))
    {
	/* Make sure we find defintion for _rtl_obstack */
	L_cg_hack_resolve_mem("_permanent_allocation", "_rtl_obstack");

	/* Make sure we find defintion for _current_obstack */
	L_cg_hack_resolve_mem("_permanent_allocation", "_current_obstack");

	/* Make sure we find defintion for _saveable_obstack */
	L_cg_hack_resolve_mem("_permanent_allocation", "_saveable_obstack");

	/* Make sure we find defintion for _expression_obstack */
	L_cg_hack_resolve_mem("_permanent_allocation", "_expression_obstack");

    }
    else if (!strcmp(cur_node->func_name, "_expand_expr"))
    {
	/* Make sure we find defintion for _bcc_gen_fctn */
	L_cg_hack_resolve_mem("_init_optabs", "_bcc_gen_fctn");

	/* Make sure we find defintion for _setcc_gen_fctn */
	L_cg_hack_resolve_mem("_init_optabs", "_setcc_gen_fctn");
    }
    else if (!strcmp(cur_node->func_name, "_do_fio"));
    {
	L_cg_hack_resolve_mem("_s_rsfe", "_doed");
	L_cg_hack_resolve_mem("_s_rsfe", "_doned");
	L_cg_hack_resolve_mem("_s_rsfe", "_doend");
	L_cg_hack_resolve_mem("_s_rsfe", "_dorevert");

	L_cg_hack_resolve_mem("_s_wsfe", "_doed");
	L_cg_hack_resolve_mem("_s_wsfe", "_doned");
	L_cg_hack_resolve_mem("_s_wsfe", "_doend");
	L_cg_hack_resolve_mem("_s_wsfe", "_dorevert");

	L_cg_hack_resolve_mem("_s_wsfe", "_putn");
    }
}
#endif

/******************************************************************************\
 *
 * Resolve unknown functions in a breadth-first manner.
 *
\******************************************************************************/

void L_cg_add_unknown_arcs(CG_Node *base_node)
{
    CG_Arc 	*arc, *next_dst_arc;
    CG_Node	*node;
    L_Oper	*oper;
    L_Operand	*operand;

    L_cg_load_func(base_node);

    for (arc = base_node->first_dst_arc; arc != NULL; arc = next_dst_arc)
    {
	next_dst_arc = arc->next_dst_arc;

	if (arc->dst_node == callgraph->unknown_node)
	{
	    if ((oper = 
		L_oper_hash_tbl_find_oper(base_node->fn->oper_hash_tbl,
		arc->jsr_id)) == NULL)
		L_punt("illegal cb for jsr op %d", arc->jsr_id);

	    /* Add an arc from this node to every other node in the list */
	    for (node = callgraph->first_node; node != NULL; 
		 node = node->next_node)
	    {
		operand = L_new_gen_label_operand(node->func_name);
		
		L_cg_add_non_dup_arc(base_node, oper, operand);

		L_delete_operand(operand);
	    }

	    callgraph->total_unknown_jsr_static--;
	    callgraph->total_unknown_jsr_dynamic-=oper->weight;
	    L_cg_delete_arc(arc);
	}
    }
}

void L_cg_resolve_unknown_jsr(CG_Node *base_node)
{
    CG_Arc 	*arc, *next_dst_arc;
    L_Cb	*cb;
    L_Oper	*oper;
    Value	*value;
    Reg		*reg, *next_reg;
    Resolved	*resolved;
    int		unresolved=0;
    L_Func	*old_L_fn;

    if (L_debug_callgraph)
	fprintf (stderr, "\nResolving function: %s with id=%d\n", 
    	    base_node->func_name, base_node->func_id);

#ifdef HACK_022 
    /* Hack to prevent 022.li from recursing for an intractable result */
    if (L_analysis_level == INTER_PROCEDURAL_ANALYSIS)
    {
        if (!(strcmp(base_node->func_name, "___IMPACT_ST_6_19_evform"))) 
	    return;
        if (!(strcmp(base_node->func_name, "_xlapply"))) 
	    return;
        if (!(strcmp(base_node->func_name, "___IMPACT_ST_16_49_sendmsg"))) 
	    return;
    }
#endif

    for (arc = base_node->first_dst_arc; arc != NULL; arc = next_dst_arc)
    {
	next_dst_arc = arc->next_dst_arc;

	if (arc->dst_node == callgraph->unknown_node)
	{

    	    if (L_debug_callgraph)
		fprintf (stderr, "  : jsr_id=%d, arc_id=%d\n", arc->jsr_id, 
		    arc->arc_id);

	    old_L_fn = L_fn;

	    L_cg_load_func(base_node);

	    if ((cb = L_oper_hash_tbl_find_cb(
		 base_node->fn->oper_hash_tbl, arc->jsr_id)) == NULL)
		L_punt("illegal cb for jsr op %d", arc->jsr_id);

	    if ((oper = 
		L_oper_hash_tbl_find_oper(base_node->fn->oper_hash_tbl,
		arc->jsr_id)) == NULL)
		L_punt("illegal cb for jsr op %d", arc->jsr_id);

	    resolved = L_resolve_unknown(base_node, cb, oper, 
		1, &oper->src[0], NULL);
	    
	    unresolved = 0;
	    for (reg = resolved->first_reg; reg != NULL; reg = next_reg)
	    {
	        if (reg->first_value!=NULL)
		{
		    for (value = reg->first_value; value != NULL; 
		         value = value->next_value)
		    {
		        if ((value->opc == Lop_MOV) && 
		            (L_is_label(value->src0)))
		        {
    	    		    if (L_debug_callgraph)
				fprintf (stderr, "  -> %s\n", 
				    value->src0->value.l);

		            L_cg_add_non_dup_arc(base_node, oper, value->src0);
		        
		            /* Update the interprocedural database */
		            L_db_add_param_value (arc->dst_node->func_id, 
			        arc->src_node->func_id, oper->id, reg->dest,
			        value->opc, value->src0, value->src1, 
				value->src2);
		        }
		        else
		            unresolved = 1;
		    }
		}
		else
		    unresolved = 1;

		next_reg = reg->next_reg;
	    }

	    L_rs_delete_resolved(resolved);

	    /* If we were able to resolve this node, we can delete it */
	    if (!unresolved)
	    {
	        L_cg_delete_arc(arc);
	        callgraph->total_unknown_jsr_static--;
	        callgraph->total_unknown_jsr_dynamic-=oper->weight;
	    }

	    L_fn = old_L_fn;

	}
    }
}

void L_cg_resolve_unknown_nodes()
{
    CG_Node	*node;

    /* The assumption is that the root node will have no unknown arcs */
    if (callgraph->total_unknown_jsr_static)
    {

	/* 
	 * This prevents regeneration of parameter registers that
	 * are already known.
	 */
	for (node = callgraph->first_node; node != NULL; node = node->next_node)
	{
	    if (node->total_unknown)
	        L_cg_resolve_unknown_jsr(node);
	}

	/* Free memory used for traversed Lcode functions */
	for (node = callgraph->first_node; node != NULL; node = node->next_node)
	{
	    if (node->fn)
	    {
		L_fn = node->fn;
		L_delete_func(L_fn);
		node->fn = NULL;
	    }
	}

        /* 
         * If there are still unknown function calls, we must add
         * arcs from each unknown site to all other functions to ensure
         * conservative use.
         */
        if (L_add_unknown_arcs && callgraph->total_unknown_jsr_static)
	    for (node = callgraph->first_node; node != NULL; 
		 node = node->next_node)
	        if (node->total_unknown) L_cg_add_unknown_arcs(node);
    }
    else
	return;

}


/******************************************************************************\
 *
 * External interface routines for callgraph
 *
\******************************************************************************/

CG_Node *L_cg_get_node_from_id(int func_id)
{
    CG_Node	*node;

    if (!callgraph)
	L_punt ("L_cg_get_node_from_id: callgraph not defined\n");

    for (node = callgraph->first_node; node!=NULL; node = node->next_node)
    {
	if (node->func_id == func_id)
	    return node;
    }

    return NULL;
}

int L_callgraph_get_func_id(CallGraph *callgraph, char *name)
{
    CG_Node	*node;

    if (!callgraph)
	L_punt ("L_callgraph_get_func_id: null callgraph provided\n");

    for (node = callgraph->first_node; node!=NULL; node = node->next_node)
    {
	if (!strcmp(node->func_name, name))
	    return node->func_id;
    }

    /* If we reach this point, the function is unknown ! */
    return -1;
    
}

Set L_callgraph_query(CallGraph *callgraph, int func_id, int jsr_id)
{
    CG_Node		*node;
    CG_Arc		*arc;

    Set jsr_set=NULL;

    if (!callgraph)
	L_punt ("L_callgraph_query: null callgraph provided\n");

    for (node = callgraph->first_node; node!=NULL; node = node->next_node)
    {
	if (node->func_id == func_id)
	{
	    for (arc = node->first_dst_arc; arc != NULL; 
		 arc = arc->next_dst_arc)
	    {
		if (arc->jsr_id == jsr_id)
		    jsr_set = Set_add(jsr_set, arc->dst_node->func_id);
	    }
	    return jsr_set;
	}
    }

    /* If we reach this point, the function is unknown! */
    return NULL;
}

void L_callgraph_print(FILE *outfile, CallGraph *callgraph)
{
    CG_Node	*node;
    CG_Arc	*arc;
    int		count, fn_printed;

    if (!callgraph)
	L_punt ("L_callgraph_print: null callgraph provided\n");

    /* Print out file header information */
    fprintf (outfile, "Program Call Graph Version 1.0\n\n");

    /* Miscelaneous information */
    fprintf (outfile, "(Misc_node declaration\n");

    if (callgraph->root_node)
        fprintf (outfile, "    root node                    ( %d )\n", 
	    callgraph->root_node->func_id);
    else
        fprintf (outfile, "    root node                    ( UNKNOWN )\n");

    fprintf (outfile, "    unknown node id              ( %d )\n", 
	callgraph->unknown_node->func_id);

    fprintf (outfile, "\n");

    fprintf (outfile, "    total sef static functions   ( %d )\n",
	callgraph->total_sef_func_static);

    fprintf (outfile, "    total static functions       ( %d )\n",
	callgraph->total_nodes-1);

    fprintf (outfile, "    Percent sef static functions ( %.2f )\n",
        ((float) callgraph->total_sef_func_static / 
	 (float) (callgraph->total_nodes-1)) * 100.0 );

    fprintf (outfile, "\n");

    fprintf (outfile, "    total unknown static jsr     ( %d )\n",
        callgraph->total_unknown_jsr_static);

    fprintf (outfile, "    total static jsr             ( %d )\n",
        callgraph->total_jsr_static);

    fprintf (outfile, "    Percent unknown static jsr   ( %.2f )\n",
        ((float) callgraph->total_unknown_jsr_static / 
	 (float) callgraph->total_jsr_static) * 100.0 );

    fprintf (outfile, "\n");

    callgraph->total_unknown_jsr_dynamic = 
       fabs (callgraph->total_unknown_jsr_dynamic); 

    fprintf (outfile, "    total unknown dynamic jsr    ( %.2f )\n",
        callgraph->total_unknown_jsr_dynamic);

    fprintf (outfile, "    total dynamic jsr            ( %.2f )\n",
        callgraph->total_jsr_dynamic);

    fprintf (outfile, "    Percent unknown dynamic jsr  ( %.2f )\n",
        ((float) callgraph->total_unknown_jsr_dynamic / 
	 (float) callgraph->total_jsr_dynamic) * 100.0 );

    fprintf (outfile, "\n");

    fprintf (outfile, "    total sef static jsr         ( %d )\n",
        callgraph->total_sef_jsr_static);

    fprintf (outfile, "    Percent sef static jsr       ( %.2f )\n",
        ((float) callgraph->total_sef_jsr_static / 
	 (float) callgraph->total_jsr_static) * 100.0 );

    fprintf (outfile, "\n");

    fprintf (outfile, "    total sef dynamic jsr        ( %.2f )\n",
        callgraph->total_sef_jsr_dynamic);

    fprintf (outfile, "    Percent sef dynamic jsr      ( %.2f )\n",
        ((float) callgraph->total_sef_jsr_dynamic / 
	 (float) callgraph->total_jsr_dynamic) * 100.0 );

    fprintf (outfile, "\n");

    if (callgraph->total_unknown_jsr_static)
    {
	fprintf (outfile, "\n    Unknown jsr list\n");
        fprintf (outfile, "Function, filename	Oper_id		Frequency	Resolves to\n");
        fprintf (outfile, "==================	=======		=========	===========\n");

        for (node = callgraph->first_node; node != NULL; node = node->next_node)
        {
	    fn_printed = 0;
	    for (arc = node->first_dst_arc; arc != NULL; arc=arc->next_dst_arc)
	    {
		if (arc->dst_node == callgraph->unknown_node)
		{
		    if (!fn_printed)
		    {
			fprintf (outfile, "%s (id %d), %s\n", node->func_name, 
			    node->func_id, node->filename);
			fn_printed = 1;
		    }
		    fprintf (outfile, "\t\t\t%d\t\t%.0f\n", arc->jsr_id, 
			arc->weight);
		}
	    }
	}
	fprintf (outfile, "\n\n");
    }

    fprintf (outfile, "end)\n\n");


    /* Print out mapping table */
    fprintf (outfile, "(Mapping_table declaration\n");

    for (node = callgraph->first_node; node != NULL; node = node->next_node)
    {
	fprintf (outfile, "    %5d (%s, %s", node->func_id, 
	    node->func_name, (node->filename==NULL? "?" : node->filename));
	
	if (L_EXTRACT_BIT_VAL(L_FUNC_SIDE_EFFECT_FREE, node->flags))
	    fprintf (outfile, ", sef)\n");
	else
	    fprintf (outfile, ")\n");
    }

    fprintf (outfile, "end)\n\n");


    /* Print out functions that are called by each func */
    fprintf (outfile, "(Call_graph declaration\n");

    for (node = callgraph->first_node; node != NULL; node = node->next_node)
    {
	fprintf (outfile, "    %5d ( ", node->func_id);

        count = 0;
	for (arc = node->first_dst_arc; arc != NULL; arc = arc->next_dst_arc)
	{
	    if (count == 4)
	    {
		fprintf(outfile, "\n            ");
		count=0;
	    }

	    fprintf (outfile, "(op %d, %d) ", arc->jsr_id, 
		arc->dst_node->func_id);
	    count ++;

	}

	fprintf (outfile, ")\n");
    }
    fprintf (outfile, "end)\n\n");

}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void L_cg_free_functions(CallGraph *callgraph)
{
    CG_Node *node;

    /* Free up memory allocated to functions that are still in memory */
    for (node = callgraph->first_node; node != NULL;
         node = node->next_node)
    {
	if (node->fn)
	{
	    L_fn = node->fn;
	    node->fn = NULL;

	    L_delete_func(L_fn);
        }
    }
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

CallGraph *L_callgraph_build(FILE *file_list)
{
    L_Cb	*cb;
    L_Oper	*oper;
    CG_Node	*node;
    char	buf[128];
    int		file_offset;

    building_callgraph = 1;

    callgraph = L_cg_new_callgraph();

    /* Process all data and functions within a file */
    while (fgets(buf, sizeof(buf), file_list) != NULL)
    {
	buf[strlen(buf)-1] = '\0';
	L_open_input_file(buf);

	input_filename = C_findstr(buf);

	/*
        file_offset = Cl_tell();
	*/
	file_offset = L_tell();
	
        while (L_get_input() != L_INPUT_EOF)
        {
            if (L_token_type==L_INPUT_FUNCTION) 
	    {
		/* 
		 * Compute the oper weight to determine frequency
		 * that subroutine calls are executed.  This information
		 * is only used when the graph is printed out to determine
		 * how important a particular call is.
		 */
		L_compute_oper_weight(L_fn, 0, 1);

		node = L_cg_find_node(L_fn->name);
		node->file_offset = file_offset;

		for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
		{
		    for (oper = cb->first_op; oper != NULL; 
			 oper = oper->next_op)
		    {
			if (L_subroutine_call_opcode(oper))
			{
			    L_cg_add_arc(node, oper, oper->src[0]);
			}
			else 
			if ((L_analysis_level == INTER_PROCEDURAL_ANALYSIS) &&
			    (L_mem_global_store(oper)))
			{
			    if (L_is_label(oper->src[2]) ||
			        L_is_constant(oper->src[2]))
			        L_mem_define_cell(NULL, oper->src[0], 
				    oper->src[1], Lop_MOV, oper->src[2],
				    NULL, NULL, -1);
			}
		    }
		}
	        L_delete_func(L_fn);
            }
	    else
	    {
	        L_mem_load_data(L_data);
	        L_delete_data(L_data);
	    }

	    /*
            file_offset = Cl_tell();
	    */
	    file_offset = L_tell();
        }

	L_close_input_file(buf);
    }

    if (L_analysis_level!=TRIVIAL_ANALYSIS)
        L_cg_resolve_unknown_nodes();

    building_callgraph = 0;

    return callgraph;
}
