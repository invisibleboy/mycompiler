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
 *      File:    pipa_graph.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _PIPA_GRAPH_H_
#define _PIPA_GRAPH_H_

#include "pipa_common.h"
#include "pipa_symbols.h"
#include "library/set.h"

#define IPA_PRINT_ASCI   1
#define IPA_PRINT_BINARY 2

typedef enum IPA_cgraph_edgelist_e
{
  MIN_EDGELIST_TYPE,
  ASSIGN, ASSIGN_ADDR, DEREF_ASSIGN, ASSIGN_DEREF, SKEW, 
  MAX_EDGELIST_TYPE
}
IPA_cgraph_edgelist_e;

#define IPA_CG_ETYPE_ASSIGN_ADDR   0x01
#define IPA_CG_ETYPE_ASSIGN        0x02
#define IPA_CG_ETYPE_DEREF_ASSIGN  0x04
#define IPA_CG_ETYPE_ASSIGN_DEREF  0x08
#define IPA_CG_ETYPE_SKEW          0x10
#define IPA_CG_ETYPE_ALL           0xff

extern char *edge_types[];
extern char *node_types[];

struct IPA_cgraph_edge_t;
struct IPA_cgraph_edge_data_t;
struct IPA_cgraph_edge_list_t;
struct IPA_cgraph_node_t;
struct IPA_cgraph_node_data_t;
struct IPA_cgraph_t ;

/*************************************************************************
 * Pool
 *************************************************************************/

void
IPA_cg_edgepool_print_info(FILE *file);

void
IPA_cg_nodepool_print_info(FILE *file);


/*************************************************************************
 * EDGE
 *************************************************************************/

#define IPA_CG_EDGE_FLAGS_EXPLICIT     0x0001
#define IPA_CG_EDGE_FLAGS_IMPLICIT     0x0002

#define IPA_CG_EDGE_FLAGS_HZ           0x0004
#define IPA_CG_EDGE_FLAGS_DN           0x0008
#define IPA_CG_EDGE_FLAGS_UP           0x0010
#define IPA_CG_EDGE_FLAGS_GBL          0x0020
#define IPA_CG_EDGE_FLAGS_UD        0x0018
#define IPA_CG_EDGE_FLAGS_DIR_ALL   0x003f

#define IPA_CG_EDGE_FLAGS_ARRAY            0x0040

#define IPA_CG_EDGE_FLAGS_PROCESSED        0x0100
#define IPA_CG_EDGE_FLAGS_CALLG_PROCESSED  0x0200
/* unused 0x0080
          0x0400
*/
#define IPA_CG_EDGE_FLAGS_GENERIC1     0x0800
#define IPA_CG_EDGE_FLAGS_GENERIC2     0x1000
#define IPA_CG_EDGE_FLAGS_GENERIC3     0x2000
#define IPA_CG_EDGE_FLAGS_GENERIC4     0x4000
#define IPA_CG_EDGE_FLAGS_GENERICALL   0x7800

#define IPA_CG_EDGE_FLAGS_NEW          0x8000

#define EDGEDATA_PARAMS int t_offset, int size, int s_offset
#define CG_EDGE_ISNEW(e)    IPA_FLAG_ISSET(e->flags, IPA_CG_EDGE_FLAGS_NEW)
#define CG_EDGE_SETNEW(e)   IPA_FLAG_SET(e->flags, IPA_CG_EDGE_FLAGS_NEW)
#define CG_EDGE_CLRNEW(e)   IPA_FLAG_CLR(e->flags, IPA_CG_EDGE_FLAGS_NEW)
#define CG_EDGE_COPYFLAGS(de,se)  ((de)->flags |= (se)->flags)

typedef struct IPA_cgraph_edge_t
{
  /* FLAGS */
  unsigned short flags;

  struct IPA_cgraph_edge_data_t {
    int   target_offset;
    short target_stride;
    int   assign_size;
    short source_stride;
    int   source_offset;
  } data;

#if EDGE_STATS
  int dup;
#endif
#if EDGE_HISTORY
  /* These are the two edges that resulted in this edge 
   */
  struct IPA_cgraph_edge_t *e1;
  struct IPA_cgraph_edge_t *e2;
  int    level;
#endif

  /* Source info */
  struct IPA_cgraph_edge_list_t *src_elist;

  /* Destination info */
  struct IPA_cgraph_edge_list_t *dst_elist;
}
IPA_cgraph_edge_t;

typedef struct IPA_cgraph_edge_data_t IPA_cgraph_edge_data_t;

IPA_cgraph_edge_t *IPA_cg_edge_new ();

void IPA_cg_edge_free (IPA_cgraph_edge_t * edge);

IPA_cgraph_edge_t *IPA_cg_edge_find (struct IPA_cgraph_node_t *src_node,
                                     struct IPA_cgraph_node_t *dst_node,
                                     IPA_cgraph_edgelist_e edge_type,
                                     IPA_cgraph_edge_data_t * edata);

IPA_cgraph_edge_t *IPA_cg_edge_add (struct IPA_cgraph_node_t *src_node,
                                    struct IPA_cgraph_node_t *dst_node,
                                    IPA_cgraph_edgelist_e edge_type,
                                    IPA_cgraph_edge_data_t * edata);

void IPA_cg_edge_delete (IPA_cgraph_edge_t * edge);

char *IPA_cg_edge_flag_name(IPA_cgraph_edge_t *edge);

/*************************************************************************
 * EDGE CHANGE TRACKING
 *************************************************************************/


typedef enum IPA_edgetrack_e
{ IPA_ET_ALL, IPA_ET_CALLEE,
  IPA_ET_MAX
}
IPA_edgetrack_e;

typedef struct edgetrack_t
{
  int                id;
  IPA_Hashtab_t     *delta;
  IPA_edgetrack_e    mode;
} edgetrack_t;

void 
IPA_cg_edgetrack_end (int id);

int
IPA_cg_edgetrack_start (IPA_edgetrack_e mode);

void
IPA_cg_edgetrack_newedge (IPA_cgraph_edge_t * edge);

void 
IPA_cg_edgetrack_remedge (IPA_cgraph_edge_t * edge, int key);

IPA_Hashtab_t * 
IPA_cg_edgetrack_delta (int id);


/*************************************************************************
 * EDGE LIST
 *************************************************************************/

typedef struct IPA_cgraph_edge_list_t
{
  IPA_cgraph_edgelist_e edge_type;

  /* EDGES INTO NODE */
  IPA_Hashtab_t *in;

  /* EDGES OUT OF NODE */
  IPA_Hashtab_t *out;

  /* NODE OWNING THIS LIST */
  struct IPA_cgraph_node_t *node;

  /* NEXT LIST */
  struct IPA_cgraph_edge_list_t *nxt_list;
}
IPA_cgraph_edge_list_t;

IPA_cgraph_edge_list_t *IPA_cg_edge_list_new ();

void IPA_cg_edge_list_free (IPA_cgraph_edge_list_t * edgelist);


IPA_cgraph_edge_list_t *
IPA_cg_edge_list_find (struct IPA_cgraph_node_t *node,
		       IPA_cgraph_edgelist_e edge_type);

IPA_cgraph_edge_list_t *
IPA_cg_edge_list_add (struct IPA_cgraph_node_t *node,
		      IPA_cgraph_edgelist_e edge_type);

void 
IPA_cg_edge_list_delete (IPA_cgraph_edge_list_t * elist);

int
IPA_edge_list_valid (IPA_cgraph_edge_list_t * elist, int valid_edges);

/*************************************************************************
 * NODE
 *************************************************************************/
/* For CONSG Variable Information */
#define IPA_CG_NODE_FLAGS_CALLEE    0x00000001 /* determines indirect-callees */
#define IPA_CG_NODE_FLAGS_HEAP      0x00000002 /* heap var */
#define IPA_CG_NODE_FLAGS_GLOBAL    0x00000004 /* global var */
#define IPA_CG_NODE_FLAGS_PARAM     0x00000008 /* formal param */
#define IPA_CG_NODE_FLAGS_RETURN    0x00000010 /* "formal" return */
#define IPA_CG_NODE_FLAGS_FUNC      0x00000020 /* func var (potential callee) */
#define IPA_CG_NODE_FLAGS_TEMP      0x00000040 /* unnamed temp var needed for graph */ 
#define IPA_CG_NODE_FLAGS_ESCLOCAL  0x00000080 /* local with escaping addr */
#define IPA_CG_NODE_FLAGS_SUMMARY   0x00000100 /* node came from summary */
#define IPA_CG_NODE_FLAGS_REALPARAM 0x00000200 /* The real formal param */
#define IPA_CG_NODE_FLAGS_ELLIPSE   0x00000400 /* VARARGS ellipse marker */
#define IPA_CG_NODE_FLAGS_STACK     0x00000800 /* alloca dynamic stack */

#define IPA_CG_NODE_FLAGS_NOFIELD   0x00001000 /* Do not specialize fields */
#define IPA_CG_NODE_FLAGS_NOCNTXT   0x00002000 /* Do not specialize contexts */
#define IPA_CG_NODE_FLAGS_NOLOCAL   0x00004000 /* Can escape through a return */

/* General Graph flags */
#define unused4    0x00008000

#define IPA_CG_NODE_FLAGS_MUSTMATCH 0x0000FDFF

/* Generic flags */
#define IPA_CG_NODE_FLAGS_GENERIC1  0x00010000
#define IPA_CG_NODE_FLAGS_GENERIC2  0x00020000
#define IPA_CG_NODE_FLAGS_GENERIC3  0x00040000
#define IPA_CG_NODE_FLAGS_GENERIC4  0x00080000
#define IPA_CG_NODE_FLAGS_GENERIC5  0x00100000
#define IPA_CG_NODE_FLAGS_GENERIC6  0x00200000
#define IPA_CG_NODE_FLAGS_GENERIC7  0x00400000
#define IPA_CG_NODE_FLAGS_GENERIC8  0x00800000
#define IPA_CG_NODE_FLAGS_GENERIC9  0x01000000
#define IPA_CG_NODE_FLAGS_GENERIC10 0x02000000
#define IPA_CG_NODE_FLAGS_GENERIC11 0x04000000
#define IPA_CG_NODE_FLAGS_GENERIC12 0x08000000
#define IPA_CG_NODE_FLAGS_GENERIC   0x0fff0000

#define unused10    0x10000000
#define unused11    0x20000000
#define unused12    0x40000000

#define IPA_CG_NODE_FLAGS_NEW       0x80000000

#define CG_NODE_ISNEW(e)    IPA_FLAG_ISSET(e->flags, IPA_CG_NODE_FLAGS_NEW)
#define CG_NODE_SETNEW(e)   IPA_FLAG_SET(e->flags, IPA_CG_NODE_FLAGS_NEW)
#define CG_NODE_CLRNEW(e)   IPA_FLAG_CLR(e->flags, IPA_CG_NODE_FLAGS_NEW)
#define CG_NODE_COPYFLAGS(dn,sn)  ((dn)->flags |= (sn)->flags)

typedef struct IPA_cgraph_scc_stack_t
{
  struct IPA_cgraph_node_t *node;
  int def_num;
  int low_link;
} IPA_cgraph_scc_stack_t;

typedef struct IPA_cgraph_link_t
{
  struct IPA_cgraph_node_t *to;
  struct IPA_cgraph_link_t *nxt_l;
} IPA_cgraph_link_t;

typedef struct IPA_cgraph_node_t
{
  /* FLAGS */
  unsigned int flags;

  /* REPRESENTATIVES */
  struct IPA_cgraph_node_t *rep_child;
  struct IPA_cgraph_node_t *rep_parent;

  /* LINK FOR GRAPH APPLY */
  struct IPA_cgraph_link_t *lk;

  /* EDGE LISTS */
  IPA_cgraph_edge_list_t *first_list;

  /* GRAPH OWNING NODE */
  struct IPA_cgraph_t *cgraph;

  /* MISC DATA */
  unsigned short delay2;
  unsigned short sample_delay2;
  unsigned short mix2;
  unsigned short collapse;
  
  union {
    unsigned short depth;
    void *ptr;
    void *sync_ptr;
    IPA_cgraph_scc_stack_t *st;
  } misc;

  unsigned short generation;
  unsigned int from_version;

  /* Graph specific data for nodes */
  struct IPA_cgraph_node_data_t
  {
    /* USED FOR LOCATING NODE */
    unsigned int var_id;
    unsigned int version;

    /* OTHER NODE/VAR DATA */
    unsigned int var_size;
    unsigned int mod;
    unsigned int in_k_cycle;
    struct IPA_symbol_info_t *syminfo;
  } data;
} IPA_cgraph_node_t;

typedef struct IPA_cgraph_node_data_t IPA_cgraph_node_data_t;

IPA_cgraph_node_t *IPA_cg_node_new (struct IPA_cgraph_t *cgraph,
                                    IPA_cgraph_node_data_t * ndata);

IPA_cgraph_node_t *IPA_cg_node_find (struct IPA_cgraph_t *cgraph,
                                     IPA_cgraph_node_data_t * ndata);

IPA_cgraph_node_t *IPA_cg_node_add (struct IPA_cgraph_t *cgraph,
                                    IPA_cgraph_node_data_t * ndata);

void IPA_cg_node_reset (IPA_cgraph_node_t * node);
void IPA_cg_node_delete (IPA_cgraph_node_t * node);

int IPA_cg_node_same (IPA_cgraph_node_t * node1, IPA_cgraph_node_t * node2);

int IPA_cg_node_is_child (IPA_cgraph_node_t * node);

IPA_cgraph_node_t *IPA_cg_node_get_rep (IPA_cgraph_node_t * node);

IPA_cgraph_edge_list_t *IPA_cg_node_elist (IPA_cgraph_node_t * node);

void IPA_cg_nodes_clr_flags (struct IPA_cgraph_t *cgraph, unsigned int flags);
void IPA_cg_nodes_assert_clr_flags (struct IPA_cgraph_t * cgraph, unsigned int flags);

void
IPA_cg_move_node(struct IPA_cgraph_t *dst_cg,
		 struct IPA_cgraph_t *src_cg,
		 IPA_cgraph_node_t *node);


/*************************************************************************
 * GRAPH
 *************************************************************************/

typedef struct IPA_cgraph_t
{
  /* Lists of ALL nodes */
  IPA_Hashtab_t *nodes;

  struct IPA_cgraph_data_t
  {
    struct IPA_funcsymbol_info_t *fninfo;
  } data;
} IPA_cgraph_t;

IPA_cgraph_t *IPA_cg_cgraph_new (struct IPA_funcsymbol_info_t *fninfo);

void IPA_cg_cgraph_free (IPA_cgraph_t * graph);

void
IPA_cg_merge_graph(IPA_cgraph_t *dst_cg, IPA_cgraph_t *src_cg);


void IPA_cgraph_minit ();

void IPA_cgraph_mfree ();

void IPA_cgraph_pool_info();

/*************************************************************************
 * SCC Detection
 *************************************************************************/

IPA_cgraph_scc_stack_t *IPA_cg_scc_stack_new ();

void IPA_cg_scc_stack_free (IPA_cgraph_scc_stack_t * stack);

List IPA_cg_find_SCC (IPA_cgraph_t * cgraph, int valid_edges);

void IPA_cg_free_SCC (IPA_cgraph_t * cgraph, List list);

void IPA_cg_print_SCC (FILE * file, List list);


/*************************************************************************
 * Merge Operations
 *************************************************************************/

List
IPA_cg_merge_nodes (IPA_cgraph_node_t * dest_node,
		    IPA_cgraph_node_t * src_node,
		    int delete,
		    List work_list, int in_wl_flag);

void
IPA_cg_merge_node_list (IPA_cgraph_node_t * dest_node,
			List src_node_list,
			int delete);

List 
IPA_cgraph_make_fi(IPA_cgraph_node_t *node, List work_list, int in_wl_flag);

/*************************************************************************
 * Apply one graph onto another
 *************************************************************************/

void IPA_cg_clear_links (IPA_cgraph_t * cg);

void IPA_cg_link_nodes (IPA_cgraph_t * cg1, IPA_cgraph_node_t * node1,
                        IPA_cgraph_t * cg2, IPA_cgraph_node_t * node2);

void IPA_cg_apply_graph (IPA_cgraph_t * cg_d, IPA_cgraph_t * cg_s);

#endif
