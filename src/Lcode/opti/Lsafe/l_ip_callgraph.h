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
 *  File: l_ip_callgraph.h
 *
 *  Description:  
 *
 *  Creation Date :  May, 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *  Copyright (c) 1994 Roger A. Bringmann , Wen-mei Hwu and The Board of
 *		  Trustees of the University of Illinois. 
 *		  All rights reserved.
 *
 *  The University of Illinois software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
\*****************************************************************************/

#ifndef L_CALLGRAPH
#define L_CALLGRAPH

/* 10/29/02 REK Adding config.h */
#include <config.h>

/******************************************************************************\
 *
 * Data structures for call graph
 *
\******************************************************************************/

typedef struct CG_Arc {
    int			arc_id;	
    int			jsr_id;		/* Id of src node jsr call */
    double		weight;		/* frequency of src node jsr call */

    struct CG_Node	*src_node;	
    struct CG_Arc	*prev_src_arc;	
    struct CG_Arc	*next_src_arc;	

    struct CG_Node	*dst_node;
    struct CG_Arc	*prev_dst_arc;	
    struct CG_Arc	*next_dst_arc;	
} CG_Arc;

typedef struct CG_Node {
    int			func_id;		/* Unique id for function */
    int			flags;
    char		*func_name;		/* function name */
    char		*filename;		/* name of file that function 
						   is in */
    L_Func		*fn;			/* Lcode function for node */

    int			file_offset;
    int			total_unknown;

    Memory		*local_refs;		/* local memory references */
    Memory		*global_refs;		/* global memory references */
    Memory		*ptr_refs;		/* ptr memory references */

    CG_Arc		*first_src_arc;		/* Arc to first node calling 
						   this node */
    CG_Arc		*last_src_arc;		/* Arc to last node calling 
						   this node */

    CG_Arc		*first_dst_arc;		/* Arc to first node called by 
						   this node*/
    CG_Arc		*last_dst_arc;		/* Arc to last node called by 
						   this node*/

    struct CG_Node	*prev_node;		/*Used for node list traversal*/
    struct CG_Node	*next_node;		/*Used for node list traversal*/
} CG_Node;

typedef struct CallGraph {
    CG_Node		*root_node;	/* root node of call graph */
    CG_Node		*unknown_node;	/* node used for unresolved refs */

    CG_Node		*first_node;	/* first node in node list */
    CG_Node		*last_node;	/* last node in node list */

    /* Information maintained for statistics */
    int			total_nodes;	/* total nodes in graph */
    int			total_sef_func_static;

    int			total_jsr_static;
    int			total_unknown_jsr_static;
    int			total_sef_jsr_static;

    double		total_jsr_dynamic;
    double		total_unknown_jsr_dynamic;
    double		total_sef_jsr_dynamic;

} CallGraph;

#define CG_UNKNOWN_FUNC_ID	0	/* function id for unknown funcs */
#define CG_UNKNOWN_FUNC_NAME	"?"	/* function name for unknown funcs */

#endif
