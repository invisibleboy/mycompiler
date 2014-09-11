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
 *  File:  l_ip_sef.c
 *
 *  Description:  
 *	Mark functions as side-effect free.
 *
 *  Creation Date :  August 1994
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

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

int L_is_stack_store(L_Oper *oper)
{
    if (M_is_stack_operand(oper->src[0]) ||
        M_is_stack_operand(oper->src[1])) 
        return 1;
    else
	return 0;
}

int L_is_safe_subroutine_call(CG_Node *src_node, L_Oper *oper, int recursive)
{
    CG_Node	*dst_node;
    CG_Arc	*arc;
    char	*name, *src_name;
    int		safe;

    src_name = src_node->func_name;

    if (L_is_label(oper->src[0]))
    {
	name = M_fn_name_from_label(oper->src[0]->value.l);

	dst_node = L_cg_find_node(name); 

	if (L_EXTRACT_BIT_VAL(dst_node->flags, L_FUNC_SIDE_EFFECT_FREE))
	    return 1;

	else if (recursive && !strcmp(name, src_name))
	    return 1;

	else if (L_name_in_side_effect_free_func_table(name))
	    return 1;

	else
	    return 0;
    }

    safe = 1;
    for (arc = src_node->first_dst_arc; arc != NULL; arc = arc->next_dst_arc)
    {
	if (arc->jsr_id == oper->id)
	{
	    if (L_EXTRACT_BIT_VAL(arc->dst_node->flags, 
		L_FUNC_SIDE_EFFECT_FREE))
		continue;
	
	    if (recursive && !strcmp(arc->dst_node->func_name, src_name))
		continue;

	    if (L_name_in_side_effect_free_func_table(arc->dst_node->func_name))
	        continue;

	    safe = 0;
	}

	if (!safe) return 0;
    }

    return 1;
}

void L_find_side_effect_free(CallGraph *callgraph, FILE *file_list)
{
    L_Cb        *cb;
    L_Oper      *oper;
    CG_Node     *node;
    int		side_effect_free, change;

    /* First mark leaf nodes that are side-effect free */
    for (node = callgraph->first_node; node != NULL; 
	 node = node->next_node)
    {
	if (L_EXTRACT_BIT_VAL(node->flags, L_FUNC_SIDE_EFFECT_FREE) ||
	   !L_EXTRACT_BIT_VAL(node->flags, L_FUNC_LEAF))
	    continue;

	L_cg_load_func(node);

	if (node->fn == NULL) continue;

	side_effect_free = 1;

        for (cb = node->fn->first_cb; cb != NULL; cb = cb->next_cb)
        {
            for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
            {
		if (!L_general_store_opcode(oper))
		    continue;

		side_effect_free &= L_is_stack_store(oper);
            }
        }

	if (side_effect_free)
	{
	    node->flags = L_SET_BIT_FLAG(node->flags, L_FUNC_SIDE_EFFECT_FREE);
	    L_fn->flags = L_SET_BIT_FLAG(L_fn->flags, L_FUNC_SIDE_EFFECT_FREE);
	}
    }


    /* 
     * Now iterate through the graph marking higher level functions
     * functions as side-effect free until we encounter no further
     * changes.
     */
    change = 1;
    while (change)
    {
	change = 0;

        for (node = callgraph->first_node; node != NULL; 
	     node = node->next_node)
        {
	    if (L_EXTRACT_BIT_VAL(node->flags, L_FUNC_SIDE_EFFECT_FREE))
	        continue;

	    L_cg_load_func(node);

	    if (node->fn == NULL) continue;

	    side_effect_free = 1;

	    /* 
	     * First determine that current function is side-effect free.
	     * This will look for subroutine calls but will ignore
	     * recursion.
	     */
            for (cb = node->fn->first_cb; cb != NULL; cb = cb->next_cb)
            {
                for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
                {
		    if (L_general_store_opcode(oper))
		    {
		        side_effect_free &= L_is_stack_store(oper);
		    }
		    else if (L_subroutine_call_opcode(oper))
		    {
			if (L_side_effect_free_sub_call(oper))
			    continue;

			if (L_is_safe_subroutine_call(node, oper, 0))
			{
			    oper->flags = L_SET_BIT_FLAG(oper->flags, 
				L_OPER_SIDE_EFFECT_FREE);
			    
			    callgraph->total_sef_jsr_static++;
			    callgraph->total_sef_jsr_dynamic+=oper->weight;

			    change = 1;
			}
			else
			    side_effect_free = 0;
		    }
                }
            }

	    if (!side_effect_free) continue;

	    /* 
	     * Now see if there are any recursive calls that are side-effect
	     * free.  It is important to know if the function is safe
	     * in general before looking at recursion.
	     */
            for (cb = node->fn->first_cb; cb != NULL; cb = cb->next_cb)
            {
                for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
                {
		    if (L_subroutine_call_opcode(oper))
		    {
			if (L_side_effect_free_sub_call(oper))
			    continue;

			if (L_is_safe_subroutine_call(node, oper, 1))
			{
			    oper->flags = L_SET_BIT_FLAG(oper->flags, 
				L_OPER_SIDE_EFFECT_FREE);

			    callgraph->total_sef_jsr_static++;
			    callgraph->total_sef_jsr_dynamic+=oper->weight;

			    change = 1;
			}
			else
			    side_effect_free = 0;
		    }
                }
            }

	    if (side_effect_free)
	    {
	         node->flags = L_SET_BIT_FLAG(node->flags, 
		     L_FUNC_SIDE_EFFECT_FREE);
	         L_fn->flags = L_SET_BIT_FLAG(L_fn->flags, 
		     L_FUNC_SIDE_EFFECT_FREE);

		callgraph->total_sef_func_static++;
	    }
	}
    }
}
