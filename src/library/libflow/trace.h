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
 *      File:   trace.h
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_TRACE_H
#define IMPACT_TRACE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/graph.h>

#if 0
SAM 1 - 94
#define BEST_TRACE_PROB         0.7
#endif
#define BEST_TRACE_PROB         0.55
#define SELECT_BY_NODE_WEIGHT   0
#define SELECT_BY_ARC_WEIGHT    1
#define SELECT_BY_MIN_PROB      2
#define SELECT_BY_MAX_ARC       3
#define W_A             0       /* terminal to terminal */
#define W_B             1       /* terminal to middle */
#define W_C             2       /* middle to terminal */
#define W_D             3       /* middle to middle */
#define W_E             4       /* in-trace */
#define W_BACK_EDGE     5       /* inner loop back-edge (part of W_A) */
#define W_SEQUENTIAL    6       /* sequential transitions (except last node) */
#define N_NODES         7       /* total number of nodes */
#define W_NODES         8       /* total node weight */
#define N_ARCS          9       /* total number of arcs */
#define W_ARCS          10      /* total arc weight */
#define N_TRACE         11      /* total number of traces */
#define L_TRACE         12      /* average trace length */
#define N_LOOP          13      /* total number of loops */
#define L_LOOP          14      /* average loop length */
#define N_ROOT          15      /* number of nodes with no predecessor */
#define N_LEAF          16      /* number of nodes with no successor */
#define TRECORD_SIZE    17
#ifdef __cplusplus
extern "C"
{
#endif

  extern void SelectBySortArc (FGraph graph);
  extern void TraceSelection (FGraph graph, int method, double min_prob);
  extern void ReportSelectionResult (FGraph graph, double *matrix);

#ifdef __cplusplus
}
#endif

/*
 *      Two fields in the _Node structures are modified
 *      by trace selection. The (type) field is set to
 *      the trace_id. For each graph, each trace is assigned
 *      an unique identifier. The (status) field is set
 *      to the union of one or more of the below flags.
 *      ReportSelectionResult() function destroys the VISITED bits.
 */
#define NOT_VISITED     0
#define VISITED         1
#define TRACE_HEAD      2
#define TRACE_TAIL      4
#define LOOP_HEAD       8
/*
 *      The (type) field of the _Arc structures are also
 *      modified. It is used in the trace selection algorithm.
 *      After trace selection, the (type) field is 0 if it is
 *      not an in-trace transition. For in-trace transition,
 *      the (type) field is set to the trace_id.
 */

#endif
