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
 *      File :          l_pce.h
 *      Description :   PCE graph cut/insertion function header file.
 *      Creation Date : February 2003
 *      Author :        Shane Ryoo, Wen-mei Hwu
 *
 *===========================================================================*/

#ifndef L_PCE_H
#define L_PCE_H

#define BB_PAST_CUT      (0x0001)
#define BB_LOOSE         (0x0002)
#define BB_PRED          (0x0004)
#define BB_LOOP		 (0x0008)
#define BB_EXIT          (0x0010)
#define BB_SINK_ADJACENT (0x0020)
#define BB_COMBINE       (0x0040)

#define ARC_LOOPBACK     (0x0001)
#define ARC_COST         (0x0002)

#include <config.h>
#include <library/l_alloc_new.h>
#include <library/i_list.h>
#include <library/i_graph.h>
#include <Lcode/l_graph.h>
#include <Lcode/l_pred_flow.h>

typedef struct _L_PCE_BB
{
  GraphNode node;
  int index;
  L_BB *bb;
  int height;
  unsigned int excess;
  int pce_type;
  double reg_pressure_inc;
  Set reaching_BB;
  Set reaching_assn;

}
L_PCE_BB;

typedef struct _L_PCE_arc
{
  GraphArc arc;
  L_BB_arc *bb_arc;
  unsigned int weight;
  unsigned int flow;
  int pce_type;
}
L_PCE_arc;

typedef struct _L_PCE_graph
{
  Graph graph;
  GraphNode source;
  GraphNode sink;

  Set taken_br_edges;
  Set untaken_br_edges;
  Set fallthrough_cbs;
  Set taken_exit_br;
  Set untaken_exit_br;
  Set fallthrough_exit_cbs;
  List visit_nodes;

  int n_vertices;
  int n_edges;
  int expression_index;
  int num_cut_active;
}
L_PCE_graph;

typedef struct _L_PCE_graph_cut
{
  int token;

  Set taken_br_edges;
  Set untaken_br_edges;
  Set fallthrough_cbs;
  Set taken_exit_br;
  Set untaken_exit_br;
  Set fallthrough_exit_cbs;

  Set taken_br_cut;
  Set untaken_br_cut;
  Set fallthrough_cbs_cut;
  Set sink_adjacent_cbs_cut;

  Set expressions;
  int n_vertices;
  int n_edges;
  int repeat;
  int share;
  struct _L_PCE_graph_cut *next;
}
L_PCE_graph_cut;


#ifdef __cplusplus
extern "C"
{
#endif

/*====================
 * Function Prototypes
 *====================*/
  extern void L_do_SPRE_analysis (L_Func *, int, int);
  extern void L_do_P3DE_analysis (L_Func *, int, int);

  extern void L_PRE_speculate_motion (Graph, PRED_FLOW *, Set *, int, int);
  extern void L_PDE_predicate_motion (Graph, PRED_FLOW *, Set *, int, int);
  extern int L_PDE_fast_assignment (L_Expression *);

  extern void L_print_PCE_graph_cuts ();
  extern void L_delete_PCE_graph_cuts ();

#ifdef __cplusplus
}
#endif

#endif
