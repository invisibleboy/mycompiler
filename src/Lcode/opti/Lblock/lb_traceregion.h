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
 *      File :          l_lb_traceregion.h
 *      Description :   Tools for dealing with Lblock traceregions
 *      Creation Date : November 1997
 *      Authors :       David August, Kevin Crozier
 *
 *      (C) Copyright 1997, David August, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/
#ifndef L_LB_TRACEREGION_H
#define L_LB_TRACEREGION_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/i_graph.h>
#include <library/i_list.h>
#include <Lcode/l_code.h>

/*  add_cb_to_graph constants */
#define END 1			/* i.e. bottom of traceregion */
#define TOP 0			/* i.e  top or start of traceregion */

/* TraceRegion flag values */
#define L_TRACEREGION_VISITED			0x00000001
#define L_TRACEREGION_HYPERBLOCK		0x00000010
#define L_TRACEREGION_SUPERBLOCK		0x00000020
#define L_TRACEREGION_INSERTED                  0x00001000

#define L_TRACEREGION_LOOP                      0x00010000
#define L_TRACEREGION_HAMMOCK                   0x00020000
#define L_TRACEREGION_GENERAL                   0x00040000
#define L_TRACEREGION_TRACE                     0x00080000

#define L_TRACEREGION_TYPE                      0x000F0000

#define L_TRACEREGION_NOFALLTHRU                0x00100000
#define L_TRACEREGION_FALLTHRU                  0x00200000
#define L_TRACEREGION_PRELIM                    0x00400000
#define L_TRACEREGION_FLAG_NESTED_CYCLE         0x01000000
#define L_TRACEREGION_FLAG_HAS_UNSAFE_JSR       0x02000000
#define L_TRACEREGION_FLAG_HAS_JSR              0x04000000
#define L_TRACEREGION_FLAG_HAS_POINTER_ST       0x08000000

#define FlowGraph(tr)              (tr->flow_graph)
#define FlowGraphTopoList(tr)      (tr->flow_graph->topo_list)

/* One possible superblock trace or hyperblock region */
typedef struct _LB_TraceRegion
{
  int id;			/* Identifier of trace */
  int flags;			/* Flag visited */
  Graph flow_graph;		/* Flow graph of the region */
  L_Cb *header;			/* Header cb of the region */
  double weight;		/* measure of region weight - header weight? */
  double weight_temp;		/* used for superblock formation */
  int dep_height;
  int num_ops;
  int tail_dup;
  int slots_used;		/* From the hyperblock region structure */
  int slots_avail;		/* From the hyperblock region structure */
  double exec_ratio;
  double priority;

  //added by morteza
  int wcet;
  int wcet2;


}
LB_TraceRegion;

/* new data structs */
typedef struct _LB_TraceRegion_Header
{
  /* not implemented yet */
  L_Func *fn;			/* pointer to function this guy is apart of */
  int next_id;			/* next trace-region id to be assigned */
  List traceregions;		/* A list of trace-regions */
  List inorder_trs;
}
LB_TraceRegion_Header;

#ifdef __cplusplus
extern "C"
{
#endif

/*====================
 * External Function Prototypes
 *====================*/
  extern LB_TraceRegion_Header *LB_create_tr_header (L_Func *);
  extern void LB_free_tr_header (LB_TraceRegion_Header *);
  extern LB_TraceRegion *LB_create_traceregion (L_Func *, int, L_Cb *, Set,
						int);
  extern void LB_update_traceregion (LB_TraceRegion *tr, 
				     L_Func *fn, L_Cb *hdr, Set new_tr_cbs);
  extern void LB_free_traceregion (LB_TraceRegion *);
  extern void LB_free_all_traceregions (LB_TraceRegion_Header *);
  extern void LB_add_cb_to_traceregion (LB_TraceRegion *, L_Cb *, L_Cb *,
					int);
  extern L_Cb *LB_first_cb_in_region (LB_TraceRegion *);
  extern L_Cb *LB_next_cb_in_region (LB_TraceRegion *);
  extern L_Cb *LB_get_first_cb_in_region (LB_TraceRegion *);
  extern L_Cb *LB_get_next_cb_in_region (LB_TraceRegion *);
  extern GraphNode LB_next_graphnode (LB_TraceRegion *);
  extern L_Cb *LB_last_cb_in_region (LB_TraceRegion *);
  extern L_Cb *LB_return_cb_in_region (LB_TraceRegion *, int);
  extern Set LB_return_cbs_region_as_set (LB_TraceRegion *);
  extern void LB_traceregion_set_fallthru_flag (LB_TraceRegion *);
  extern LB_TraceRegion *LB_concat_seq_trs (LB_TraceRegion_Header *,
					    LB_TraceRegion *,
					    LB_TraceRegion *);
  extern void LB_clear_traceregion_visit_flags (LB_TraceRegion_Header *);
  extern int LB_unvisited_traceregions (LB_TraceRegion_Header *);
  extern int LB_traceregion_is_subsumed (LB_TraceRegion *,
					 LB_TraceRegion_Header *);
  extern void LB_remove_subsumed_traceregions (LB_TraceRegion_Header *);
  extern void LB_remove_partially_subsumed_hammock_traceregions (LB_TraceRegion_Header *);
  extern void LB_remove_partially_subsumed_traceregions (LB_TraceRegion_Header *);

  extern void LB_remove_conflicting_traceregions (LB_TraceRegion_Header *);
  extern LB_TraceRegion *LB_find_traceregion_by_header (LB_TraceRegion_Header
							*, L_Cb *);
  extern LB_TraceRegion *LB_find_traceregion_by_cb (LB_TraceRegion_Header *,
						    L_Cb *);
  extern LB_TraceRegion *LB_find_traceregion_of_number (LB_TraceRegion_Header
							*, int);
  extern void LB_print_traceregions (FILE *, LB_TraceRegion_Header *);
  extern void LB_print_inorder_trs (FILE *, LB_TraceRegion_Header *);
  extern void LB_print_tr_by_num (FILE *, LB_TraceRegion_Header *, int);
  extern void LB_print_tr (FILE *, LB_TraceRegion *);
  extern void LB_summarize_traceregions (FILE *, LB_TraceRegion_Header *);
  extern void LB_summarize_tr (FILE *, LB_TraceRegion *);
#ifdef __cplusplus
}
#endif

#endif
