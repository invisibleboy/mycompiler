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
 *      File :          l_opti.h
 *      Description :   Include files / global variables for Lopti
 *      Creation Date : July, 1990
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_OPTI_H
#define L_OPTI_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_opti_count.h>
#include <Lcode/lb_block.h>

#include "l_danger.h"
#include "l_opti_predicates.h"
#include "l_opti_functions.h"
#include "l_local_opti.h"
#include "l_global_opti.h"
#include "l_memflow_opti.h"
#include "l_jump_opti.h"
#include "l_loop_opti.h"
#include "l_branch.h"
#include "l_PCE_opti.h"
#include "l_pred_opti.h"

#define L_POST_INC_MARK                 "post_inc"
#define L_PRE_INC_MARK                  "pre_inc"

/*====================================================================*/
/*
 *      Lopti parameters
 */
/*====================================================================*/

extern int Lopti_opti_level;

/* Benchmark hacks */
extern int Lopti_do_benchmark_specific_opti;

/* local opts */
extern int Lopti_do_local_opti;
extern int Lopti_do_local_constant_prop;
extern int Lopti_do_local_copy_prop;
extern int Lopti_do_local_rev_copy_prop;
extern int Lopti_do_local_common_sub_elim;
extern int Lopti_do_local_mem_copy_prop;
extern int Lopti_do_local_red_load_elim;
extern int Lopti_do_local_red_store_elim;
extern int Lopti_do_local_constant_comb;
extern int Lopti_do_local_constant_fold;
extern int Lopti_do_local_strength_red;
extern int Lopti_do_local_strength_red_for_signed_div_rem;
extern int Lopti_do_local_operation_fold;
extern int Lopti_do_local_branch_fold;
extern int Lopti_do_local_operation_cancel;
extern int Lopti_do_local_dead_code_rem;
extern int Lopti_do_local_code_motion;
extern int Lopti_do_local_remove_sign_ext;
extern int Lopti_do_local_reduce_logic;
extern int Lopti_do_local_register_rename;
extern int Lopti_do_local_op_breakdown;
extern int Lopti_do_local_op_recombine;
extern int Lopti_do_local_branch_val_prop;
extern int Lopti_debug_local_opti;
extern int Lopti_ignore_sync_arcs_for_red_elim;
extern int Lopti_ignore_sync_arcs_for_loop_inv_migration;

/* global opts */
extern int Lopti_do_global_opti;
extern int Lopti_do_global_dead_code_rem;
extern int Lopti_do_global_constant_prop;
extern int Lopti_do_global_copy_prop;
extern int Lopti_do_global_common_sub_elim;
extern int Lopti_do_global_mem_copy_prop;
extern int Lopti_do_global_red_load_elim;
extern int Lopti_do_global_red_store_elim;
extern int Lopti_do_global_elim_boolean_ops;
extern int Lopti_do_global_branch_val_prop;
extern int Lopti_do_global_dead_if_then_else_rem;
extern int Lopti_debug_global_opti;

/* Memflow opts */
extern int Lopti_do_memflow_opti;
extern int Lopti_do_memflow_multistore_load;
extern int Lopti_memflow_bypass_load;
extern int Lopti_memflow_bypass_store;
extern int Lopti_memflow_bypass_jsr;
extern int Lopti_memflow_bypass_total;
extern int Lopti_debug_memflow;

/* Jump opts */
extern int Lopti_do_jump_opti;
extern int Lopti_do_jump_dead_block_elim;
extern int Lopti_do_jump_br_to_next_block;
extern int Lopti_do_jump_br_to_same_target;
extern int Lopti_do_jump_br_to_uncond_br;
extern int Lopti_do_jump_block_merge;
extern int Lopti_do_jump_combine_labels;
extern int Lopti_do_jump_br_target_expansion;
extern int Lopti_do_jump_br_swap;
extern int Lopti_allow_jump_expansion_of_pcode_loops;
extern int Lopti_do_remove_decidable_cond_branches;
extern int Lopti_debug_jump_opti;

extern int Lopti_do_split_branches;

extern int Lopti_do_split_unification;
extern int Lopti_do_merge_unification;

extern int Lopti_do_jrg_expansion;

/* Loop opts */
extern int Lopti_do_loop_opti;
extern int Lopti_do_loop_br_simp;
extern int Lopti_do_loop_inv_code_rem;
extern int Lopti_do_loop_global_var_mig;
extern int Lopti_do_loop_ind_var_str_red;
extern int Lopti_do_loop_ind_var_reinit;
extern int Lopti_do_loop_ind_var_elim;
extern int Lopti_do_dead_loop_rem;
extern int Lopti_preserve_loop_var;
extern int Lopti_do_complex_ind_elim;
extern int Lopti_debug_loop_opti;
extern int Lopti_do_longword_loop_opti;
#define L_STORE_MIGRATION_FULL_PRED 1
#define L_STORE_MIGRATION_NO_PRED   2
#define L_STORE_MIGRATION_NO_COND   3
extern int Lopti_store_migration_mode;

/* Partial Code Elimination opts */
extern int Lopti_do_PCE;
extern int Lopti_do_PCE_split_critical_edges;
extern int Lopti_do_PCE_merge_same_cbs;
extern int Lopti_do_PCE_optimize_memory_ops;
extern int Lopti_do_PCE_conservative_memory_opti;

extern int Lopti_do_PRE;
extern int Lopti_do_PRE_lazy_code_motion;
extern int Lopti_do_PRE_mem_copy_prop;
extern int Lopti_do_PRE_merge_loads_diff_types;
extern int Lopti_do_PRE_cutset_metric;
extern int Lopti_do_PRE_optimize_moves_of_numerical_constants;
extern int Lopti_do_PRE_optimize_moves;
extern int Lopti_do_PRE_optimize_single_source_ops;
extern int Lopti_do_PRE_speculative_code_motion;

extern int Lopti_do_global_mem_expression_copy_prop;

extern int Lopti_do_PDE;
extern int Lopti_do_PDE_cutset_metric;
extern int Lopti_do_PDE_min_cut;
extern int Lopti_do_PDE_predicated;
extern int Lopti_do_PDE_sink_stores;
extern int Lopti_do_PDE_sink_only_stores;

/* Dead store opts */
extern int Lopti_do_global_dead_store_removal;
extern int Lopti_do_dead_local_var_store_removal;

/* Miscellaneous */
extern int Lopti_only_lvl1_for_zero_weight_fn;
extern int Lopti_do_post_inc_conv;
extern int Lopti_do_mark_memory_labels;
extern int Lopti_do_mark_incoming_parms;
extern int Lopti_do_mark_trivial_sef_jsrs;
extern int Lopti_do_mark_sync_jsrs;
extern int Lopti_do_mark_trivial_safe_ops;
extern int Lopti_do_code_layout;
extern int Lopti_do_classify_branches;
extern int Lopti_print_opti_count;
extern int Lopti_print_opti_breakdown;

extern int L_native_machine_ctype;

/*====================================================================*/
/*
 *      Memory allocation pools for Lopti
 */
/*====================================================================*/

extern L_Alloc_Pool *L_alloc_danger_ext;

/*====================================================================*/
/*
 *      reg id -> ctype map array
 */
/*====================================================================*/

extern int *Lopti_ctype_array;
extern int Lopti_ctype_array_size;
extern int Lopti_ctype_max_reg;

/*====================================================================*/
/*
 *      Optimization counters
 */
/*====================================================================*/

/*
 *      Local optimization counters
 */

extern int Lopti_cnt_local_opti;
extern int Lopti_cnt_local_constant_prop;
extern int Lopti_cnt_local_copy_prop;
extern int Lopti_cnt_local_rev_copy_prop;
extern int Lopti_cnt_local_mem_copy_prop;
extern int Lopti_cnt_local_common_sub_elim;
extern int Lopti_cnt_local_red_load_elim;
extern int Lopti_cnt_local_red_store_elim;
extern int Lopti_cnt_local_constant_fold;
extern int Lopti_cnt_local_strength_red;
extern int Lopti_cnt_local_constant_comb;
extern int Lopti_cnt_local_operation_fold;
extern int Lopti_cnt_local_branch_fold;
extern int Lopti_cnt_local_operation_cancel;
extern int Lopti_cnt_local_dead_code_rem;
extern int Lopti_cnt_local_code_motion;
extern int Lopti_cnt_local_remove_sign_ext;
extern int Lopti_cnt_local_reduce_logic;
extern int Lopti_cnt_local_register_rename;

/*
 *      Global optimization counters
 */

extern int Lopti_cnt_global_opti;
extern int Lopti_cnt_global_dead_code_rem;
extern int Lopti_cnt_global_constant_prop;
extern int Lopti_cnt_global_copy_prop;
extern int Lopti_cnt_global_mem_copy_prop;
extern int Lopti_cnt_global_common_sub_elim;
extern int Lopti_cnt_global_red_load_elim;
extern int Lopti_cnt_global_red_store_elim;
extern int Lopti_cnt_global_elim_boolean_ops;
extern int Lopti_cnt_global_dead_if_then_else_rem;

/*
 *      Memflow optimization counters
 */

extern int Lopti_cnt_memflow_multistore_load;

/*
 *      Jump optimization counters
 */

extern int Lopti_cnt_jump_opti;
extern int Lopti_cnt_jump_dead_block_elim;
extern int Lopti_cnt_jump_br_to_next_block;
extern int Lopti_cnt_jump_br_to_same_target;
extern int Lopti_cnt_jump_br_to_uncond_br;
extern int Lopti_cnt_jump_block_merge;
extern int Lopti_cnt_jump_combine_labels;
extern int Lopti_cnt_jump_br_target_expansion;
extern int Lopti_cnt_jump_br_swap;

/*
 *      Loop optimization counters
 */

extern int Lopti_cnt_loop_opti;
extern int Lopti_cnt_loop_br_simp;
extern int Lopti_cnt_loop_inv_code_rem;
extern int Lopti_cnt_loop_global_var_mig;
extern int Lopti_cnt_loop_ind_var_str_red;
extern int Lopti_cnt_loop_ind_var_reinit;
extern int Lopti_cnt_loop_ind_var_elim;
extern int Lopti_cnt_dead_loop_rem;
/* extern int Lopti_cnt_loop_unrolling; Not available yet! */

/*
 * Inter region global optimization counters
 */
extern int Lopti_inter_region_global_opti;
extern int Lopti_inter_region_global_dead_code_rem;
extern int Lopti_inter_region_global_constant_prop;
extern int Lopti_inter_region_global_copy_prop;
extern int Lopti_inter_region_global_mem_copy_prop;
extern int Lopti_inter_region_global_common_sub_elim;
extern int Lopti_inter_region_global_red_load_elim;
extern int Lopti_inter_region_global_red_store_elim;
extern int Lopti_inter_region_global_elim_boolean_ops;
extern int Lopti_inter_region_global_dead_if_then_else_rem;

extern double Lopti_inter_region_global_common_sub_elim_wgt;
extern double Lopti_inter_region_global_copy_prop_wgt;
extern double Lopti_inter_region_loop_inv_wgt;
extern double Lopti_inter_region_gvm_wgt;

extern int Lopti_pred_promotion_level;
extern int Lopti_remove_red_guards;  /* -KF 10/2005 */

/*====================================================================*/
/*
 *      Function prototypes
 */
/*====================================================================*/

extern void Lopti_init ();
extern void Lopti_deinit ();
extern void L_code_optimize (L_Func *);

extern int L_do_longword_loop_conversion (L_Loop *);
extern int L_do_benchmark_tuning (L_Func *);

extern void L_tag_load (L_Func *);

#endif
