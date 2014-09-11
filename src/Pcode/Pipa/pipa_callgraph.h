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
 *      File:    pipa_callgraph.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _PIPA_CALLGRAPH_H_
#define _PIPA_CALLGRAPH_H_

#include "pipa_common.h"
#include "pipa_symbols.h"

#define IPA_CALLG_EDGE_FLAGS_DIRECT     0x00000001
#define IPA_CALLG_EDGE_FLAGS_INDIRECT   0x00000002
#define IPA_CALLG_EDGE_FLAGS_SELFEDGE   0x00000004

#define IPA_CALLG_EDGE_FLAGS_NEW        0x10000000

#define CALLG_EDGE_ISNEW(e)    IPA_FLAG_ISSET(e->flags, IPA_CALLG_EDGE_FLAGS_NEW)
#define CALLG_EDGE_SETNEW(e)   IPA_FLAG_SET(e->flags, IPA_CALLG_EDGE_FLAGS_NEW)
#define CALLG_EDGE_CLRNEW(e)   IPA_FLAG_CLR(e->flags, IPA_CALLG_EDGE_FLAGS_NEW)

struct IPA_callg_edge_t;
struct IPA_callg_node_t;

/*************************************************************************
 * UPDATE BUFFER
 *************************************************************************/

typedef struct IPA_callg_update_t
{
  struct IPA_callg_node_t *caller_node;
  struct IPA_callsite_t   *caller_cs;
  char                    *callee_symbol_name;
  Key                      callee_symbol_key;
} IPA_callg_update_t;

IPA_callg_update_t*
IPA_callg_update_new();

void
IPA_callg_update_free(IPA_callg_update_t *cgu);

List
IPA_callg_update_add(List update_list,
		     struct IPA_callg_node_t *caller_node,
		     struct IPA_callsite_t   *caller_cs,
		     char *callee_name,
		     Key callee_key);



/*************************************************************************
 * EDGE
 *************************************************************************/

typedef struct IPA_callg_edge_t
{
  /* FLAGS */
  unsigned int flags;

  List                    sum_nodes;
  char                    summary_incorporated;
  int                     previous_sumsize;

  struct IPA_callsite_t   *caller_cs;
  struct IPA_callg_node_t *caller;
  struct IPA_interface_t  *callee_if;
  struct IPA_callg_node_t *callee;

  int summary_size;
} IPA_callg_edge_t;

IPA_callg_edge_t *
IPA_callg_edge_find(struct IPA_callg_node_t *caller,
		    struct IPA_callsite_t   *caller_cs,
		    struct IPA_callg_node_t *callee,
		    struct IPA_interface_t   *callee_if);

IPA_callg_edge_t *
IPA_callg_edge_add(struct IPA_callg_node_t *caller,
		   struct IPA_callsite_t   *caller_cs,
		   struct IPA_callg_node_t *callee,
		   struct IPA_interface_t   *callee_if,
		   int is_indirect);

void
IPA_callg_edge_delete (IPA_callg_edge_t * edge);

int
IPA_callg_update_callg(struct IPA_prog_info_t * info,
		       List update_list,
		       int round);

/*************************************************************************
 * NODE
 *************************************************************************/

#define IPA_CALLG_NODE_FLAGS_GENERIC1   0x00100000
#define IPA_CALLG_NODE_FLAGS_GENERIC2   0x00200000
#define IPA_CALLG_NODE_FLAGS_GENERIC3   0x00400000
#define IPA_CALLG_NODE_FLAGS_GENERIC4   0x00800000
#define IPA_CALLG_NODE_FLAGS_GENERIC5   0x01000000
#define IPA_CALLG_NODE_FLAGS_GENERIC6   0x02000000

#define IPA_CALLG_NODE_FLAGS_NEW        0x10000000

#define CALLG_NODE_ISNEW(e)    IPA_FLAG_ISSET(e->flags, IPA_CALLG_NODE_FLAGS_NEW)
#define CALLG_NODE_SETNEW(e)   IPA_FLAG_SET(e->flags, IPA_CALLG_NODE_FLAGS_NEW)
#define CALLG_NODE_CLRNEW(e)   IPA_FLAG_CLR(e->flags, IPA_CALLG_NODE_FLAGS_NEW)

/* For tarjan's algorithm, 1972 */
typedef struct IPA_callg_scc_stack_t
{
  struct IPA_callg_node_t *node;
  int def_num;
  int low_link;
}
IPA_callg_scc_stack_t;

typedef struct IPA_callg_node_t
{
  unsigned int flags;

  List caller_edges;
  List callee_edges;

  struct IPA_callg_node_t *rep_parent;
  struct IPA_callg_node_t *rep_child;

  struct IPA_funcsymbol_info_t *fninfo;
  
  struct IPA_callg_scc_stack_t *st;

  struct IPA_callg_t *callgraph;  
} IPA_callg_node_t;


IPA_callg_node_t *
IPA_callg_node_find (struct IPA_callg_t *callg,
		     struct IPA_funcsymbol_info_t *fninfo);

IPA_callg_node_t *
IPA_callg_node_add (struct IPA_callg_t *callg,
		    struct IPA_funcsymbol_info_t *fninfo);

void
IPA_callg_node_delete (IPA_callg_node_t *node);

IPA_callg_node_t *
IPA_callg_node_get_rep (IPA_callg_node_t *node);


/*************************************************************************
 * GRAPH
 *************************************************************************/

typedef struct IPA_callg_t
{
  List nodes;
} IPA_callg_t;


IPA_callg_t*
IPA_callg_new();

void
IPA_callg_free(IPA_callg_t *callg);

void
IPA_callg_minit ();

void
IPA_callg_pool_info();

void
IPA_callg_mfree ();

void
IPA_callg_stats(IPA_callg_t * callg);

/*************************************************************************
 * Merge Operations
 *************************************************************************/

void
IPA_callg_merge_nodes (IPA_callg_node_t *dst,
		       IPA_callg_node_t *src);


/*************************************************************************
 * Topological Sort
 *************************************************************************/

List
IPA_callg_find_toposort (IPA_callg_t * callg, IPA_callg_node_t *root);

void
IPA_callg_topo_print (FILE * file, List list);


/*************************************************************************
 * SCC Detection
 *************************************************************************/

void 
IPA_callg_scc_stack_free (IPA_callg_scc_stack_t * stack);

List 
IPA_callg_find_SCC (IPA_callg_t * callg);

void 
IPA_callg_free_SCC (IPA_callg_t * callg, List list);

void 
IPA_callg_print_SCC (FILE * file, List list);


/*************************************************************************
 * Printing Interfaces
 *************************************************************************/

void
IPA_callg_print(IPA_callg_t * callg, char *name);

void
IPA_callg_DVprint (IPA_callg_t * callg, char *name);

void
IPA_callg_FLprint (IPA_callg_t * callg, char *name);

IPA_callg_t * 
IPA_callg_FLread (char *name);

#endif
