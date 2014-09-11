/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    pipa_print_graph.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <pipa_graph.h>
#include <pipa_callgraph.h>
#include <pipa_program.h>
#include <pipa_symbols.h>
#include <pipa_pcode2pointsto.h>


#ifndef _PIPA_PRINT_GRAPH_H_
#define _PIPA_PRINT_GRAPH_H_


/*************************************************************************
 * File
 *************************************************************************/

void IPA_cg_node_print (FILE * file, IPA_cgraph_node_t * node, int mode);
void IPA_cg_all_writefile (IPA_prog_info_t *prog_info, char *name);

void IPA_cg_writefile (IPA_cgraph_t *consg, char *name);
void IPA_callg_writefile (IPA_callg_t *consg, char *name);

void IPA_cg_consg_readfile(IPA_prog_info_t *info, char *name);
void IPA_callg_readfile(IPA_prog_info_t *info, char *name);


/*************************************************************************
 * DaVinci Printing
 *************************************************************************/

void IPA_cg_DVname_node(IPA_cgraph_node_t * node, char *node_name);

char *IPA_cg_DVedge_color(IPA_cgraph_edgelist_e edge_type);
char *IPA_cg_DVnode_color(IPA_cgraph_node_t *node);
char *IPA_cg_DVnode_border(IPA_cgraph_t *cgraph, IPA_cgraph_node_t *node);

void IPA_cg_DVprint (IPA_cgraph_t * cg, char *name, int valid_edges);

void IPA_init_func_names ();
int IPA_compare_func_name (char *name);


/*************************************************************************
 * Dot Printing
 *************************************************************************/

void IPA_cg_DOTprint (IPA_cgraph_t * cg, char *name, int valid_edges);

void
IPA_print_DOThistory(char *fname,
		     IPA_cgraph_edgelist_e edge_type,
		     IPA_cgraph_node_t *src_node,
		     int s_offset,
		     IPA_cgraph_node_t *dst_node,
		     int d_offset,
		     int size,
		     int max_level);
#endif
