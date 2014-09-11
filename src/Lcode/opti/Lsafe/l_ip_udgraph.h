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
 *  File: l_ip_udgraph.h
 *
 *  Description:  
 *    Traverses the use-def chains that lead to an instruction.
 *
 *  Creation Date :  February, 1994
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

#ifndef L_USEDEF
#define L_USEDEF

/* 10/29/02 REK Adding config.h */
#include <config.h>

typedef struct UD_Arc {
    struct UD_Node	*src_node;
    struct UD_Arc	*prev_src_arc;
    struct UD_Arc	*next_src_arc;

    struct UD_Node	*dst_node;
    struct UD_Arc	*prev_dst_arc;
    struct UD_Arc	*next_dst_arc;
} UD_Arc;

typedef struct Def_Oper {
    L_Oper              *oper;
    struct Def_Oper     *prev_op;
    struct Def_Oper     *next_op;
} Def_Oper;

typedef struct Def_List {
    Def_Oper            *first_op;
    Def_Oper            *last_op;
} Def_List;

typedef struct UD_Node {
    L_Cb		*cb;		/* current cb */
    L_Oper		*start_oper;	/* instruction where cb search begin */
    L_Oper		*term_oper;	/* Terminating instruction */

    struct Def_Oper	*first_op;	/* First oper in def-use chain. */
    struct Def_Oper	*last_op;	/* Last oper in def-use chain. */

    UD_Arc		*src_arc;	/* linked list of src arcs */
    UD_Arc		*dst_arc;	/* linked list of dst arcs */

    struct UD_Node	*next_node;	/* Used to simplify memory reclamation*/
    struct UD_Node	*prev_node;	/* Used to simplify memory reclamation*/
    struct UD_Node	*next_snode;	/* Used to maintain starts of use-def 
					   chains */
    struct UD_Node	*prev_snode;	/* Used to maintain starts of use-def 
					   chains */
} UD_Node;

typedef struct Retrace {
    UD_Node		*src_node;
    L_Cb		*cb;
    L_Oper		*start_oper;
    Set			src_reg_set;
    Def_List		*loads;
    struct Retrace	*next_retrace;
} Retrace;

typedef struct UD_Graph {
    UD_Node		*start_node_list;
    UD_Node		*end_node;
    UD_Node		*allocated_node_list;
    int			num_cb_use_sets;
    Retrace		**cb_sets;
} UD_Graph;
#endif
