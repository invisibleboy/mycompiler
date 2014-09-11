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
 *      File:    pipa_consg.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef __PIPA_CONSG_H_
#define __PIPA_CONSG_H_

#include "pipa_common.h"
#include "pipa_program.h"
#include "pipa_callgraph.h"
#include "pipa_graph.h"

struct IPA_prog_info_t;
struct IPA_interface_t;
struct IPA_funcsymbol_info_t;

/*****************************************************************************
 * Find node
 *****************************************************************************/
IPA_cgraph_node_t *
IPA_consg_find_node (IPA_cgraph_t * cgraph,
		     int var_id, int version);

/*****************************************************************************
 * Make a new node with var_id and offset.
 *****************************************************************************/
IPA_cgraph_node_t *
IPA_consg_ensure_node_d (IPA_cgraph_t * cgraph,
                         IPA_cgraph_node_data_t * ndata,
			 int flags);

IPA_cgraph_node_t *
IPA_consg_ensure_node (IPA_cgraph_t * cgraph, 
		       int var_id, int version, int var_size,
		       struct IPA_symbol_info_t *syminfo,
		       int flags);


extern void
IPA_consg_set_nofield(IPA_cgraph_node_t *node);

/*****************************************************************************
 * Create a new version of a node
 *****************************************************************************/

IPA_cgraph_node_t *
IPA_consg_node_new_version (struct IPA_prog_info_t   *info,
                            IPA_cgraph_node_t *node, 
                            IPA_cgraph_t      *dst_cg,
			    IPA_cgraph_t      *orig_cg,
			    HashTable          ver_htab);


/*****************************************************************************
 * Manage the edge qualifiers
 *****************************************************************************/
int
IPA_consg_calc_edge_origin (IPA_cgraph_edge_t * edge1,
                            IPA_cgraph_edge_t * edge2);

/*****************************************************************************
 * Make a new constraint edge with src, tgt, type.
 *****************************************************************************/
void
check_edge(IPA_cgraph_edge_t *edge);

void
IPA_consg_apply_edge_flags(IPA_cgraph_edge_t *edge,
			   int new_flags);

IPA_cgraph_edge_t *
IPA_consg_ensure_edge_d (IPA_cgraph_edgelist_e edge_type,
			 IPA_cgraph_node_t * src_node,
                         IPA_cgraph_node_t * dst_node,
                         IPA_cgraph_edge_data_t * edata, 
			 int edge_origin);

IPA_cgraph_edge_t *
IPA_consg_ensure_edge (IPA_cgraph_edgelist_e edge_type,
                       IPA_cgraph_node_t * src_node,
                       IPA_cgraph_node_t * dst_node,
		       unsigned int t_offset,
		       unsigned int size,
		       unsigned int s_offset,
                       int edge_origin);

/*****************************************************************************
 * List out the edges
 *****************************************************************************/

List 
IPA_consg_build_listof_edges (List edge_list, IPA_cgraph_t * consg);

List
IPA_consg_build_listof_new_edges (List edge_list, IPA_funcsymbol_info_t *fninfo);

/*****************************************************************************
 * Mark/Delete nodes for a particular summary
 *****************************************************************************/

void
IPA_consg_delete_summary_nodes2(IPA_callg_edge_t *callee_edge, List new_hplist);

void
IPA_consg_delete_all_summary_nodes2 (IPA_funcsymbol_info_t *fninfo);

/*****************************************************************************
 * Given a summary graph and interfaces, apply it
 *****************************************************************************/

void
IPA_consg_apply_summary2 (IPA_prog_info_t * info,
			  IPA_cgraph_t * dst_cng,
			  IPA_interface_t * caller_iface,
			  IPA_cgraph_t * summary_cng,
			  IPA_interface_t * callee_iface,
			  HashTable cs_ver_htab,
			  IPA_cgraph_t * callee_cng,
			  IPA_callg_edge_t *callee_edge);


/*****************************************************************************
 * Delete all implicit edges
 *****************************************************************************/

void
IPA_consg_delete_implicit_edges (IPA_cgraph_t * consg);


/*****************************************************************************
 * Context control
 *****************************************************************************/

void
IPA_consg_make_cg_ci(IPA_prog_info_t * info, 
		     IPA_cgraph_t * cng);

/*****************************************************************************
 * Formal-Actual Assignments
 *****************************************************************************/

void
IPA_consg_assign_params (IPA_cgraph_t * caller_cng,
                         struct IPA_interface_t * caller_iface,
			 IPA_cgraph_t * callee_cng,
                         struct IPA_interface_t * callee_iface,
			 int filter_edge_flags,
			 int set_edge_flags);

void
IPA_consg_assign_return (IPA_cgraph_t * caller_cng,
                         struct IPA_interface_t * caller_iface,
			 IPA_cgraph_t * callee_cng,
                         struct IPA_interface_t * callee_iface,
			 int filter_edge_flags,
			 int set_edge_flags);


void
IPA_consg_set_nofield (IPA_cgraph_node_t * node);

/*****************************************************************************
 * Given a consg graph and callsite list, mark all callee control nodes
 *****************************************************************************/

void
IPA_consg_setup_nodes (IPA_prog_info_t * info, 
		       IPA_funcsymbol_info_t * fninfo,
		       IPA_cgraph_t * cng);

void
IPA_consg_set_nofield(IPA_cgraph_node_t *node);

/*****************************************************************************
 * Checking
 *****************************************************************************/

void 
check_node(IPA_cgraph_node_t *node);

void
IPA_consg_check_graph(struct IPA_prog_info_t * info, IPA_cgraph_t * consg);

#endif
