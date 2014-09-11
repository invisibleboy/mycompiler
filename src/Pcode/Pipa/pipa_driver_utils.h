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
 *      File:    pipa_driver_utils.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef __PIPA_DRIVER_UTILS_H_
#define __PIPA_DRIVER_UTILS_H_

#include "pipa_common.h"
#include "pipa_program.h"
#include "pipa_consg.h"
#include "pipa_callgraph.h"
#include "pipa_driver.h"

#define CD_DELETE_ALL 1
#define CD_MERGE_ALL  2
#define CD_SELECT     3


/*****************************************************************************
 * Given a callgraph node, callsite, and callee_name add call
 *   and all connected direct-calls
 *****************************************************************************/

List
IPA_callgraph_build_direct (IPA_prog_info_t   *info,
                            IPA_callg_node_t  *caller_node,
                            IPA_callsite_t    *caller_cs,
                            char *callee_name,
			    Key callee_key,
			    int is_indirect,
			    List new_edge_list,
			    int round);


/*****************************************************************************
 * Given new callgraph edges, remove and self cycles
 *****************************************************************************/

void
IPA_callgraph_handle_selfedges (IPA_callg_node_t *call_node);

/*****************************************************************************
 * Merge two callgraph nodes and their internal constraint graphs
 *****************************************************************************/

void
IPA_callgraph_node_merge (IPA_callg_node_t * dst_node,
                          IPA_callg_node_t * src_node);


/*****************************************************************************
 *
 *****************************************************************************/

void
IPA_k_a_cycle_detection(IPA_cgraph_t *cg);

void
IPA_a_cycle_detection(IPA_cgraph_t *cg, 
		      int dont_merge,
		      int del_mode);

void
IPA_cycle_detection(IPA_cgraph_t *cg,
		    int dont_merge,
		    int del_mode);

/*****************************************************************************
 * Find SCCs, merge nodes, merge/update constraint graphs. 
 * Converts callgraph into an acyclic graph.
 *****************************************************************************/

void
IPA_callgraph_merge_cycles (IPA_prog_info_t * info);

void
IPA_callgraph_prepare_all (IPA_prog_info_t * info);

void
IPA_callgraph_connect_all (IPA_prog_info_t * info);


/*****************************************************************************
 * Given new constraint edges, add in any new calls
 *****************************************************************************/

List
IPA_callgraph_new_callees (IPA_prog_info_t * info, 
			   List update_list,
			   List callee_delta, 
			   int round);


/*****************************************************************************
 * Incorporate summaries for all callees of a node into its constraint graph
 *****************************************************************************/

void
IPA_callgraph_apply_callee_summaries2 (IPA_prog_info_t * info,
				       IPA_callg_node_t * root_node,
				       int round);

/*********************************************************************
 *  A inter-graph connectioncopy based approach to incorporating 
 *    global/param info during top-down propagation
 *********************************************************************/

void
IPA_callgraph_attach_caller_params (IPA_prog_info_t * info,
				    IPA_callg_node_t * callg_node);

void
IPA_callgraph_attach_caller_rets (IPA_prog_info_t * info,
				  IPA_callg_node_t * callg_node);

/*****************************************************************************
 * Relocate globals into a common graph
 *****************************************************************************/

void
IPA_callgraph_distill_globals (IPA_prog_info_t * info,
			       IPA_callg_node_t * callg_node);

#endif
