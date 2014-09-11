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
/*===========================================================================
 *
 *      File :          graph.h
 *      Description :   Graph function header file.
 *      Creation Date : November 1996
 *      Author :        David August
 *
 *      Copyright (c) 1996 David August, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
 *===========================================================================*/

#ifndef GRAPH_H
#define GRAPH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/i_global.h>
#include <library/l_alloc_new.h>
#include <library/i_list.h>
#include <library/set.h>
#include <library/i_hash.h>
#include <library/i_error.h>
#include <library/i_io.h>
//#include "../../Lcode/Lcode/l_code.h"

#define GraphContents(g)        (g->ptr)
#define GraphRootNode(g)        (g->root)
#define GraphNodeContents(n)    (n->ptr)
#define GraphArcContents(a)     (a->ptr)
#define GraphArcDest(a)         (a->succ)
#define GraphArcSrc(a)          (a->pred)
#define GraphTopoList(g)        (g->topo_list)
#define GraphRevTopoList(g)     (g->rev_topo_list)
#define GraphEquivCDContents(e) (e->ptr)

/* Flags */

/* pre */
#define GRAPHMODIFIED                  0x00000000
#define GRAPHTOPOSORT                  0x00000001
#define GRAPHDOM                       0x00000002
#define GRAPHEQUIVCD                   0x00000004
#define GRAPHCONTROL                   0x00000008
#define GRAPHPOSTDOM                   0x00000010
#define GRAPHIMMPOSTDOM                0x00000020
#define GRAPHIMMDOM                    0x00000040
#define GRAPHCOUNTPATH                 0x00000080
#define GRAPHREVTOPOSORT               0x00000100
#define GRAPHDFSPRESORT                0x00000200

#define GRAPH_ARC_VISITED              0x00000001
#define GRAPH_ARC_BACKEDGE             0x00000002

/* node->flags */

#define GRAPH_NODE_START               0x00000001
#define GRAPH_NODE_STOP                0x00000002
#define GRAPH_NODE_HEADER              0x00000004
#define GRAPH_NODE_DEFER               0x00000008

/* node->tflags */

#define GRAPH_NODE_VISIT_WHITE         0x00000000
#define GRAPH_NODE_VISIT_GREY          0x00000001
#define GRAPH_NODE_VISIT_BLACK         0x00000002

/*  pre  */
#define GRAPH_USER_DEF1                0x00400000
#define GRAPH_USER_DEF2                0x00800000
#define GRAPH_USER_DEF3                0x01000000
#define GRAPH_USER_DEF4                0x02000000
#define GRAPH_USER_DEF5                0x04000000
#define GRAPH_USER_DEF6                0x08000000
#define GRAPH_USER_DEF7                0x10000000
#define GRAPH_USER_DEF8                0x20000000
#define GRAPH_USER_DEF9                0x40000000
#define GRAPH_USER_DEF10               0x80000000

/*  node descriptors  */
#define DAV_ALL_REQUIRED_PROPERTIES    0x00000003
#define DAV_TYPE                       0x00000001
#define DAV_ID                         0x00000002

#define DAV_ALL_NODE_PROPERTIES        0x00000FF0
#define DAV_OBJECT                     0x00000010
#define DAV_FONTFAMILY                 0x00000020
#define DAV_FONTSTYLE                  0x00000040
#define DAV_COLOR                      0x00000080
#define DAV_GO                         0x00000100
#define DAV_ICONFILE                   0x00000200
#define DAV_HIDDEN                     0x00000400
#define DAV_BORDER                     0x00000800

/*  edge descriptors  */
#define DAV_ALL_EDGE_PROPERTIES        0x00007000
#define DAV_EDGECOLOR                  0x00001000
#define DAV_EDGEPATTERN                0x00002000
#define DAV_EDGEDIR                    0x00004000

#define EXTRACT_BIT_VAL(flag, val) (((flag) & (val))==0 ? 0 : 1)

typedef struct graph_equiv_cd
{
  int id;
  void *ptr;
  Set contr_dep;
  List nodes;
  List arcs;
} *GraphEquivCD, _GraphEquivCD;

typedef struct graph_node
{
  int id;
  int flags;
  int tflags;
  int info;

  void *ptr;

  Set dom;
  Set post_dom;
  struct graph_node *imm_dom;
  struct graph_node *imm_post_dom;
  Set contr_dep;
  GraphEquivCD equiv_cd;

  List pred;
  List succ;
} *GraphNode, _GraphNode;

typedef struct graph_arc
{
  int id;
  int flags;
  int info;
  void *ptr;
  GraphNode pred;
  GraphNode succ;
  // Added by Sepehr.
  int num_iteration ;			/* If it is back_edge: shows loop count */
  int wcet_weight;				/* Return the weight */
  int ILP_is_in_wcet_path;
  struct L_Flow * flow;
} *GraphArc, _GraphArc;

typedef struct graph
{
  void *ptr;
  GraphNode root;
  List nodes;
  List arcs;
  List equiv_cds;

  List topo_list;
  List rev_topo_list;
  List dfs_list;

  HashTable hash_id_node;
  HashTable hash_id_arc;
  HashTable hash_id_equiv_cd;

  int flags;
  int arc_id;
  int node_id;
} *Graph, _Graph;






#ifdef __cplusplus
extern "C"
{
#endif

/*====================
 * Graph Globals
 *====================*/
  extern L_Alloc_Pool *GraphPool;
  extern L_Alloc_Pool *GraphNodePool;
  extern L_Alloc_Pool *GraphArcPool;
  extern L_Alloc_Pool *GraphEquivCDPool;

/*====================
 * Function Prototypes
 *====================*/
  extern Graph Graph_create_graph ();

  extern Graph Graph_free_graph (Graph graph);
  extern GraphNode Graph_create_graph_node (Graph graph);
  extern GraphNode Graph_create_graph_node_given_id (Graph graph, int new_id);
  extern GraphArc Graph_create_graph_arc (Graph graph);
  extern GraphArc Graph_create_graph_arc_given_id (Graph graph, int new_id);

  extern GraphArc Graph_connect_nodes (Graph graph, GraphNode src_node,
                                       GraphNode dest_node);
  // New:
  extern GraphArc Graph_connect_nodes_with_weight(Graph graph, GraphNode src_node,
  		GraphNode dest_node,int wcet_weight);

  extern GraphNode Graph_node_from_id (Graph graph, int id);
  extern GraphNode Graph_node_from_id_or_null (Graph graph, int id);
  extern GraphArc Graph_arc_from_id (Graph graph, int id);
  extern GraphArc Graph_arc_from_id_or_null (Graph graph, int id);
  extern GraphEquivCD Graph_equiv_cd_from_id (Graph graph, int id);

  extern int Graph_size (Graph graph);

  extern void Graph_daVinci (Graph graph, char *filename,
                             void (*print_ptr_func) (FILE *, GraphNode));
  extern void Graph_daVinci2 (Graph graph, void (*output_str_fn) (char *),
                              void (*print_node_fn) (GraphNode, char *, int),
                              void (*print_arc_fn) (GraphArc, char *, int),
                              int);
  extern void Graph_daVinci2_multi_root (Graph graph,
                                         void (*output_str_fn) (char *),
                                         void (*print_node_fn) (GraphNode,
                                                                char *, int),
                                         void (*print_arc_fn) (GraphArc,
                                                               char *, int),
                                         int);

  extern void Graph_count_paths (Graph graph);
  extern int Graph_num_paths (Graph graph, GraphNode node);

  extern void Graph_dominator (Graph graph);
  extern void Graph_imm_dominator (Graph graph);
  extern void Graph_post_dominator (Graph graph);
  extern void Graph_imm_post_dominator (Graph graph);
  extern void Graph_control_dependence (Graph graph);
  extern void Graph_equiv_cd (Graph graph);

  extern int Graph_topological_sort (Graph graph);
  extern int Graph_bfs_topo_sort (Graph graph);
  extern int Graph_dfs_topo_sort (Graph graph);
  extern void Graph_rev_topological_sort (Graph graph);
  extern void Graph_preorder_dfs_sort (Graph graph, int directed);

  extern int Graph_delete_unreachable (Graph graph, 
				       void (*node_ptr_del)(void *), 
				       void (*arc_ptr_del)(void *));
#ifdef __cplusplus
}
#endif

#endif


