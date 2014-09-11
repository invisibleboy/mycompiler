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
 *      Creation Date : August 1997
 *      Author :        David August, Wen-mei Hwu
 *
 *===========================================================================*/

#ifndef L_GRAPH_H
#define L_GRAPH_H

#define BB_MN_MX        (0x0000)
#define BB_SINGLE_ENTRY (0x0001)
#define BB_SINGLE_EXIT  (0x0002)
#define BB_REMOVABLE    (0x0004)

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/l_alloc_new.h>
#include <library/i_list.h>
#include <library/i_graph.h>
#include <Lcode/l_pred_flow.h>

typedef struct _L_BB {
  L_Cb *cb;                     /* Cb containing this bb */
  GraphNode node;
  L_Oper *first_op;             /* first operation */
  L_Oper *last_op;              /* last operation */
  void *ptr;

  /* SER: For SPRE. */
  unsigned int weight;          /* weight of BB */
  PF_BB * pf_bb;                /* pointer to dataflow info */
  void *ptr_exit;               /* pointer to exit SPRE BB */
  int type;                     /* entry/exit classification */
  int bb_size;                  /* Number of operations in BB */
  int cutset_size;              /* Number of live registers at bb's cb */
} L_BB;

typedef struct _L_BB_arc {
  GraphArc arc;
  L_Oper *branch;               /* Oper of branch if available */
  int taken;                    /* True if taken */

  void *ptr;

  /* SER: For SPRE. */
  unsigned int weight;
} L_BB_arc;

typedef struct _L_BB_graph {
  L_Func *fn;
  Graph graph;
  HashTable hash_cb_bb;
} L_BB_graph;


#ifdef __cplusplus
extern "C"
{
#endif

/*====================
 * Graph Globals
 *====================*/

/*====================
 * Function Prototypes
 *====================*/
  extern Graph L_create_bb_graph (L_Func * fn);
  extern Graph L_create_weighted_bb_graph (L_Func *fn, PRED_FLOW * pred_flow);
  extern Graph L_delete_bb_graph (Graph graph);
  extern void L_bb_node_del (GraphNode node);
  extern void L_bb_arc_del (GraphArc arc);

#ifdef __cplusplus
}
#endif

#endif
