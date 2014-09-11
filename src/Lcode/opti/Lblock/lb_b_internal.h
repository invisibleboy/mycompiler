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
 *      File :          l_b_internal.h
 *      Description :   Internal data structs and defs for Lblock
 *      Creation Date : September 1997
 *      Authors :       David August, Kevin Crozier
 *
 *      (C) Copyright 1997, David August, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/

#ifndef _LB_B_INTERNAL_H
#define _LB_B_INTERNAL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <library/i_list.h>
#include <library/i_graph.h>
#include <library/heap.h>
#include <library/func_list.h>
#include <Lcode/l_dependence.h>
#include <Lcode/l_opti.h>

#include "lb_traceregion.h"
#include "lb_graph.h"
#include "lb_flow.h"
#include "lb_pred_tools.h"
#include "lb_tail.h"
#include "lb_tool.h"

/*
 * Lblock
 * ----------------------------------------------------------------------
 */

extern char *LB_block_formation_type;
extern char *LB_predicate_formation_type;
extern int LB_use_block_enum_selector;
extern int LB_make_zero_weight_regions;
extern char *LB_prevent_block_functions;
extern Func_Name_List *LB_prevent_block_list;
extern int LB_verbose_prevent_block;

/*
 * Hyperblock formation
 * ----------------------------------------------------------------------
 */

extern int    LB_hb_pred_all_paths;
extern int    LB_hb_exclude_all_jsrs;
extern int    LB_hb_exclude_all_pointer_stores;

extern int    LB_hb_peel_enable;
extern int    LB_hb_peel_heuristic;
extern int      LB_hb_peel_max_ops;
extern int      LB_hb_peel_infinity_iter;
extern double   LB_hb_peel_min_overall_coverage;
extern double   LB_hb_peel_min_peelable_coverage;
extern double   LB_hb_peel_inc_peelable_coverage;
extern double   LB_hb_peel_min_util;
extern int    LB_hb_peel_partial;

extern int    LB_hb_issue_width;
extern double LB_hb_path_max_op_growth;
extern double LB_hb_path_max_dep_growth;
extern int    LB_hb_min_ratio_sens_opct;
extern double LB_hb_path_min_exec_ratio;
extern double LB_hb_path_main_rel_exec_ratio;
extern double LB_hb_path_min_main_exec_ratio;
extern double LB_hb_path_min_priority_ratio;

extern double LB_hb_min_cb_weight;
extern double LB_hb_block_min_weight_ratio;
extern double LB_hb_block_min_path_ratio;
extern int    LB_hb_max_cb_in_hammock;
extern int    LB_hb_form_simple_hammocks_only;
extern int    LB_hb_available_predicates;

extern int    LB_hb_branch_split;
extern int    LB_hb_verbose_level;

extern int    LB_hb_do_pred_merge;

extern int    LB_hb_do_loop_collapsing;

extern double LB_tail_dup_growth;
extern double LB_max_static_tail_dup;
extern double LB_max_dyn_tail_dup;

extern char *LB_hb_unsafe_jsr_hazard_method_name;
extern char *LB_hb_safe_jsr_hazard_method_name;
extern char *LB_hb_pointer_st_hazard_method_name;
extern int LB_hb_unsafe_jsr_hazard_method;
extern int LB_hb_safe_jsr_hazard_method;
extern int LB_hb_pointer_st_hazard_method;
extern double LB_hb_unsafe_jsr_priority_penalty;
extern double LB_hb_safe_jsr_priority_penalty;
extern double LB_hb_pointer_st_priority_penalty;

extern int LB_hb_do_combine_exits;

extern int LB_setup_only;
extern int LB_split_critical;

/*
 * Superblock formation
 * ----------------------------------------------------------------------
 */

typedef enum _LB_Branch_Pred_Types
{
  PROFILE,
  STATIC,
  BOTH
} LB_Branch_Pred_Types;

extern char * LB_parm_branch_prediction_method;
extern double LB_minimum_superblock_weight;
extern double LB_maximum_code_growth;
extern double LB_min_branch_ratio;
extern double LB_trace_min_cb_ratio;
extern LB_Branch_Pred_Types LB_branch_prediction_method;

extern int LB_do_lightweight_pred_opti;

#define TRUE  1
#define FALSE 0

/* peeling */

#define LB_HAND_MARKED_FOR_LOOP_PEEL	"HB_peel"
#define LB_MARKED_FOR_LOOP_PEEL     	"HB_can_peel"
#define LB_MARKED_AS_PEELED             "HB_peeled"
#define ITERATION_INFO_HEADER           "iteration_header"
#define ITERATION_INFO_PREFIX          	"iter_"
#define ITERATION_INFO_PREFIX_LENGTH    5

#ifdef __cplusplus
extern "C"
{
#endif

/*====================================================================*/
/*
 *    Block formation algorithm function declarations
 */
/*====================================================================*/
  extern void LB_read_parm_lblock (Parm_Parse_Info *);
  extern LB_TraceRegion_Header *LB_function_init (L_Func *);
  extern void LB_function_deinit (LB_TraceRegion_Header *);

#ifdef __cplusplus
}
#endif

#endif
