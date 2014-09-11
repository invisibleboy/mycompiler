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
 *      File :          l_opti_parms.c
 *      Description :   parameters for Lopti (created by JCG 6/2/98
 *                      to solve linking problem with new Lhyper)
 *      Creation Date : June, 1998
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      Revision :
 *         
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"


/*
 *      Lopti parameters
 */

/* Level: 0-4 are valid levels */
int Lopti_opti_level = 1;

/* Benchmark-specific tuning switches */
int Lopti_do_benchmark_specific_opti = 0;

/* Local optimization switches */
int Lopti_do_local_opti = 1;
int Lopti_do_local_constant_prop = 1;
int Lopti_do_local_copy_prop = 1;
int Lopti_do_local_rev_copy_prop = 1;
int Lopti_do_local_common_sub_elim = 1;
int Lopti_do_local_mem_copy_prop = 1;
int Lopti_do_local_red_load_elim = 1;
int Lopti_do_local_red_store_elim = 1;
int Lopti_do_local_constant_comb = 1;
int Lopti_do_local_constant_fold = 1;
int Lopti_do_local_strength_red = 1;
int Lopti_do_local_strength_red_for_signed_div_rem = 1;
int Lopti_do_local_operation_fold = 1;
int Lopti_do_local_branch_fold = 1;
int Lopti_do_local_operation_cancel = 1;
int Lopti_do_local_dead_code_rem = 1;
int Lopti_do_local_code_motion = 1;
int Lopti_do_local_remove_sign_ext = 1;
int Lopti_do_local_reduce_logic = 1;
int Lopti_do_local_register_rename = 1;
int Lopti_do_local_op_breakdown = 1;
int Lopti_do_local_op_recombine = 1;
int Lopti_do_local_branch_val_prop = 1;
int Lopti_do_remove_decidable_cond_branches = 1;
int Lopti_debug_local_opti = 0;
int Lopti_ignore_sync_arcs_for_red_elim = 0;
int Lopti_ignore_sync_arcs_for_loop_inv_migration = 0;
int Lopti_pred_promotion_level = 3;
int Lopti_remove_red_guards = 1;  /* -KF 10/2005 */

/* Global optimization switches */
int Lopti_do_global_opti = 1;
int Lopti_do_global_dead_code_rem = 1;
int Lopti_do_global_constant_prop = 1;
int Lopti_do_global_copy_prop = 1;
int Lopti_do_global_common_sub_elim = 1;
int Lopti_do_global_mem_copy_prop = 1;
int Lopti_do_global_red_load_elim = 1;
int Lopti_do_global_red_store_elim = 1;
int Lopti_do_global_elim_boolean_ops = 1;
int Lopti_do_global_branch_val_prop = 1;
int Lopti_do_global_dead_if_then_else_rem = 1;
int Lopti_debug_global_opti = 0;

/* Memflow optimization switches */
int Lopti_do_memflow_opti = 1;
int Lopti_do_memflow_multistore_load = 1;
int Lopti_memflow_bypass_load = 1500;
int Lopti_memflow_bypass_store = 1500;
int Lopti_memflow_bypass_jsr = 500;
int Lopti_memflow_bypass_total = 3000;
int Lopti_debug_memflow = 0;

/* Jump optimization switches */
int Lopti_do_jump_opti = 1;
int Lopti_do_jump_dead_block_elim = 1;
int Lopti_do_jump_br_to_next_block = 1;
int Lopti_do_jump_br_to_same_target = 1;
int Lopti_do_jump_br_to_uncond_br = 1;
int Lopti_do_jump_block_merge = 1;
int Lopti_do_jump_combine_labels = 1;
int Lopti_do_jump_br_target_expansion = 1;
int Lopti_do_jump_br_swap = 1;
int Lopti_allow_jump_expansion_of_pcode_loops = 1;
int Lopti_debug_jump_opti = 0;

int Lopti_do_split_branches = 0;

int Lopti_do_split_unification = 0;
int Lopti_do_merge_unification = 0;

int Lopti_do_jrg_expansion = 0;

/* Loop optimization switches */
int Lopti_do_loop_opti = 1;
int Lopti_do_loop_br_simp = 1;
int Lopti_do_loop_inv_code_rem = 1;
int Lopti_do_loop_global_var_mig = 1;
int Lopti_do_loop_ind_var_str_red = 1;
int Lopti_do_loop_ind_var_reinit = 1;
int Lopti_do_loop_ind_var_elim = 1;
int Lopti_do_dead_loop_rem = 1;
int Lopti_preserve_loop_var = 0;
int Lopti_do_complex_ind_elim = 1;
int Lopti_debug_loop_opti = 0;
int Lopti_do_longword_loop_opti = 0;

int Lopti_store_migration_mode = L_STORE_MIGRATION_NO_PRED;

/* Partial Redundancy Elimination switches */
int Lopti_do_PCE = 0;
int Lopti_do_PCE_split_critical_edges = 1;
int Lopti_do_PCE_merge_same_cbs = 1;
int Lopti_do_PCE_optimize_memory_ops = 1;
int Lopti_do_PCE_conservative_memory_opti = 0;

int Lopti_do_PRE = 0;
int Lopti_do_PRE_lazy_code_motion = 1;
int Lopti_do_PRE_mem_copy_prop = 0;
int Lopti_do_PRE_merge_loads_diff_types = 0;
int Lopti_do_PRE_cutset_metric = 0;
int Lopti_do_PRE_optimize_moves_of_numerical_constants = 0;
int Lopti_do_PRE_optimize_moves = 0;
int Lopti_do_PRE_optimize_single_source_ops = 1;
int Lopti_do_PRE_speculative_code_motion = 0;

int Lopti_do_global_mem_expression_copy_prop = 0;

int Lopti_do_PDE = 0;
int Lopti_do_PDE_cutset_metric = 0;
int Lopti_do_PDE_min_cut = 0;
int Lopti_do_PDE_predicated = 0;
int Lopti_do_PDE_sink_stores = 0;
int Lopti_do_PDE_sink_only_stores = 0;

/* Dead store switches */
int Lopti_do_global_dead_store_removal = 0;
int Lopti_do_dead_local_var_store_removal = 0;

/* Miscellaneous switches */
int Lopti_only_lvl1_for_zero_weight_fn = 0;
int Lopti_do_post_inc_conv = 0;
int Lopti_do_mark_memory_labels = 1;
int Lopti_do_mark_incoming_parms = 1;
int Lopti_do_mark_trivial_sef_jsrs = 1;
int Lopti_do_mark_sync_jsrs = 1;
int Lopti_do_mark_trivial_safe_ops = 1;
int Lopti_do_code_layout = 1;
int Lopti_do_classify_branches = 0;
int Lopti_print_opti_count = 0;
int Lopti_print_opti_breakdown = 0;

/*
 *      Memory allocation pools for Lopti
 */
L_Alloc_Pool *L_alloc_danger_ext = NULL;

/*
 *      reg id -> ctype map array
 */
int *Lopti_ctype_array = NULL;
int Lopti_ctype_array_size = 0;
int Lopti_ctype_max_reg = 0;

/*
 *  Global variables for counting number of applied opts
 */

/* Local opts */
int Lopti_cnt_local_opti = 0;
int Lopti_cnt_local_constant_prop = 0;
int Lopti_cnt_local_copy_prop = 0;
int Lopti_cnt_local_rev_copy_prop = 0;
int Lopti_cnt_local_mem_copy_prop = 0;
int Lopti_cnt_local_common_sub_elim = 0;
int Lopti_cnt_local_red_load_elim = 0;
int Lopti_cnt_local_red_store_elim = 0;
int Lopti_cnt_local_constant_fold = 0;
int Lopti_cnt_local_strength_red = 0;
int Lopti_cnt_local_constant_comb = 0;
int Lopti_cnt_local_operation_fold = 0;
int Lopti_cnt_local_branch_fold = 0;
int Lopti_cnt_local_operation_cancel = 0;
int Lopti_cnt_local_dead_code_rem = 0;
int Lopti_cnt_local_code_motion = 0;
int Lopti_cnt_local_remove_sign_ext = 0;
int Lopti_cnt_local_reduce_logic = 0;
int Lopti_cnt_local_register_rename = 0;

/* Global opts */
int Lopti_cnt_global_opti = 0;
int Lopti_cnt_global_dead_code_rem = 0;
int Lopti_cnt_global_constant_prop = 0;
int Lopti_cnt_global_copy_prop = 0;
int Lopti_cnt_global_mem_copy_prop = 0;
int Lopti_cnt_global_common_sub_elim = 0;
int Lopti_cnt_global_red_load_elim = 0;
int Lopti_cnt_global_red_store_elim = 0;
int Lopti_cnt_global_elim_boolean_ops = 0;
int Lopti_cnt_global_dead_if_then_else_rem = 0;

/* Memflow opts */
int Lopti_cnt_memflow_multistore_load = 0;

/* Jump opts */
int Lopti_cnt_jump_opti = 0;
int Lopti_cnt_jump_dead_block_elim = 0;
int Lopti_cnt_jump_br_to_next_block = 0;
int Lopti_cnt_jump_br_to_same_target = 0;
int Lopti_cnt_jump_br_to_uncond_br = 0;
int Lopti_cnt_jump_block_merge = 0;
int Lopti_cnt_jump_combine_labels = 0;
int Lopti_cnt_jump_br_target_expansion = 0;
int Lopti_cnt_jump_br_swap = 0;

/* Loop opts */
int Lopti_cnt_loop_opti = 0;
int Lopti_cnt_loop_br_simp = 0;
int Lopti_cnt_loop_inv_code_rem = 0;
int Lopti_cnt_loop_global_var_mig = 0;
int Lopti_cnt_loop_ind_var_str_red = 0;
int Lopti_cnt_loop_ind_var_reinit = 0;
int Lopti_cnt_loop_ind_var_elim = 0;
int Lopti_cnt_dead_loop_rem = 0;

/* Inter region global optimization counts */
int Lopti_inter_region_global_opti = 0;
int Lopti_inter_region_global_dead_code_rem = 0;
int Lopti_inter_region_global_constant_prop = 0;
int Lopti_inter_region_global_copy_prop = 0;
int Lopti_inter_region_global_mem_copy_prop = 0;
int Lopti_inter_region_global_common_sub_elim = 0;
int Lopti_inter_region_global_red_load_elim = 0;
int Lopti_inter_region_global_red_store_elim = 0;
int Lopti_inter_region_global_elim_boolean_ops = 0;
int Lopti_inter_region_global_dead_if_then_else_rem = 0;

double Lopti_inter_region_global_common_sub_elim_wgt = 0.0;
double Lopti_inter_region_global_copy_prop_wgt = 0.0;
double Lopti_inter_region_loop_inv_wgt = 0.0;
double Lopti_inter_region_gvm_wgt = 0.0;

int L_native_machine_ctype = 0;

/*
 *      Read module specific parameters
 */
void
L_read_parm_lopti (Parm_Parse_Info * ppi)
{
  L_read_parm_i (ppi, "opti_level", &Lopti_opti_level);

  /* Benchmark-specific tuning options */
  L_read_parm_b (ppi, "do_benchmark_specific_opti",
		 &Lopti_do_benchmark_specific_opti);

  /* local opts */
  L_read_parm_b (ppi, "do_local_opti", &Lopti_do_local_opti);
  L_read_parm_b (ppi, "do_local_constant_prop",
		 &Lopti_do_local_constant_prop);
  L_read_parm_b (ppi, "do_local_copy_prop", &Lopti_do_local_copy_prop);
  L_read_parm_b (ppi, "do_local_rev_copy_prop",
		 &Lopti_do_local_rev_copy_prop);
  L_read_parm_b (ppi, "do_local_common_sub_elim",
		 &Lopti_do_local_common_sub_elim);
  L_read_parm_b (ppi, "do_local_mem_copy_prop",
		 &Lopti_do_local_mem_copy_prop);
  L_read_parm_b (ppi, "do_local_red_load_elim",
		 &Lopti_do_local_red_load_elim);
  L_read_parm_b (ppi, "do_local_red_store_elim",
		 &Lopti_do_local_red_store_elim);
  L_read_parm_b (ppi, "do_local_constant_comb",
		 &Lopti_do_local_constant_comb);
  L_read_parm_b (ppi, "do_local_constant_fold",
		 &Lopti_do_local_constant_fold);
  L_read_parm_b (ppi, "do_local_strength_red", &Lopti_do_local_strength_red);
  L_read_parm_b (ppi, "do_local_strength_red_for_signed_div_rem",
		 &Lopti_do_local_strength_red_for_signed_div_rem);
  L_read_parm_b (ppi, "do_local_operation_fold",
		 &Lopti_do_local_operation_fold);
  L_read_parm_b (ppi, "do_local_branch_fold", &Lopti_do_local_branch_fold);
  L_read_parm_b (ppi, "do_local_operation_cancel",
		 &Lopti_do_local_operation_cancel);
  L_read_parm_b (ppi, "do_local_dead_code_rem",
		 &Lopti_do_local_dead_code_rem);
  L_read_parm_b (ppi, "do_local_code_motion", &Lopti_do_local_code_motion);
  L_read_parm_b (ppi, "do_local_remove_sign_ext",
		 &Lopti_do_local_remove_sign_ext);
  L_read_parm_b (ppi, "do_local_reduce_logic", &Lopti_do_local_reduce_logic);
  L_read_parm_b (ppi, "do_local_register_rename",
		 &Lopti_do_local_register_rename);
  L_read_parm_b (ppi, "do_local_op_breakdown", &Lopti_do_local_op_breakdown);
  L_read_parm_b (ppi, "do_local_op_recombine", &Lopti_do_local_op_recombine);
  L_read_parm_b (ppi, "do_local_branch_val_prop", 
		 &Lopti_do_local_branch_val_prop);
  L_read_parm_b (ppi, "debug_local_opti", &Lopti_debug_local_opti);
  L_read_parm_b (ppi, "ignore_sync_arcs_for_red_elim",
		 &Lopti_ignore_sync_arcs_for_red_elim);
  L_read_parm_b (ppi, "ignore_sync_arcs_for_loop_inv_migration",
		 &Lopti_ignore_sync_arcs_for_loop_inv_migration);

  L_read_parm_b (ppi, "?do_split_branches", &Lopti_do_split_branches);

  L_read_parm_b (ppi, "?do_split_unification", &Lopti_do_split_unification);
  L_read_parm_b (ppi, "?do_merge_unification", &Lopti_do_merge_unification);

  L_read_parm_b (ppi, "?do_jrg_expansion", &Lopti_do_jrg_expansion);

  L_read_parm_i (ppi, "pred_promotion_level", &Lopti_pred_promotion_level);

  /* -KF 10/2005 */
  L_read_parm_b (ppi, "remove_red_guards", &Lopti_remove_red_guards);
  
  /* global opts */
  L_read_parm_b (ppi, "do_global_opti", &Lopti_do_global_opti);
  L_read_parm_b (ppi, "do_global_dead_code_rem",
		 &Lopti_do_global_dead_code_rem);
  L_read_parm_b (ppi, "do_global_constant_prop",
		 &Lopti_do_global_constant_prop);
  L_read_parm_b (ppi, "do_global_copy_prop", &Lopti_do_global_copy_prop);
  L_read_parm_b (ppi, "do_global_common_sub_elim",
		 &Lopti_do_global_common_sub_elim);
  L_read_parm_b (ppi, "do_global_mem_copy_prop",
		 &Lopti_do_global_mem_copy_prop);
  L_read_parm_b (ppi, "do_global_red_load_elim",
		 &Lopti_do_global_red_load_elim);
  L_read_parm_b (ppi, "do_global_red_store_elim",
		 &Lopti_do_global_red_store_elim);
  L_read_parm_b (ppi, "do_global_elim_boolean_ops",
		 &Lopti_do_global_elim_boolean_ops);
  L_read_parm_b (ppi, "do_global_branch_val_prop",
		 &Lopti_do_global_branch_val_prop);
  L_read_parm_b (ppi, "do_global_dead_if_then_else_rem",
		 &Lopti_do_global_dead_if_then_else_rem);
  L_read_parm_b (ppi, "debug_global_opti", &Lopti_debug_global_opti);

  /* memflow opts */
  L_read_parm_b (ppi, "do_memflow_opti", &Lopti_do_memflow_opti);
  L_read_parm_b (ppi, "do_memflow_multistore_load",
		 &Lopti_do_memflow_multistore_load);
  L_read_parm_i (ppi, "memflow_bypass_load", &Lopti_memflow_bypass_load);
  L_read_parm_i (ppi, "memflow_bypass_store", &Lopti_memflow_bypass_store);
  L_read_parm_i (ppi, "memflow_bypass_jsr", &Lopti_memflow_bypass_jsr);
  L_read_parm_i (ppi, "memflow_bypass_total", &Lopti_memflow_bypass_total);
  L_read_parm_b (ppi, "debug_memflow", &Lopti_debug_memflow);

  /* jump opts */
  L_read_parm_b (ppi, "do_jump_opti", &Lopti_do_jump_opti);
  L_read_parm_b (ppi, "do_jump_dead_block_elim",
		 &Lopti_do_jump_dead_block_elim);
  L_read_parm_b (ppi, "do_jump_br_to_next_block",
		 &Lopti_do_jump_br_to_next_block);
  L_read_parm_b (ppi, "do_jump_br_to_same_target",
		 &Lopti_do_jump_br_to_same_target);
  L_read_parm_b (ppi, "do_jump_br_to_uncond_br",
		 &Lopti_do_jump_br_to_uncond_br);
  L_read_parm_b (ppi, "do_jump_block_merge", &Lopti_do_jump_block_merge);
  L_read_parm_b (ppi, "do_jump_combine_labels",
		 &Lopti_do_jump_combine_labels);
  L_read_parm_b (ppi, "do_jump_br_target_expansion",
		 &Lopti_do_jump_br_target_expansion);
  L_read_parm_b (ppi, "do_jump_br_swap", &Lopti_do_jump_br_swap);
  L_read_parm_b (ppi, "allow_jump_expansion_of_pcode_loops",
		 &Lopti_allow_jump_expansion_of_pcode_loops);
  L_read_parm_b (ppi, "debug_jump_opti", &Lopti_debug_jump_opti);
  L_read_parm_b (ppi, "do_remove_decidable_cond_branches",
		 &Lopti_do_remove_decidable_cond_branches);

  /* loop opts */
  L_read_parm_b (ppi, "do_loop_opti", &Lopti_do_loop_opti);
  L_read_parm_b (ppi, "do_loop_br_simp", &Lopti_do_loop_br_simp);
  L_read_parm_b (ppi, "do_loop_inv_code_rem", &Lopti_do_loop_inv_code_rem);
  L_read_parm_b (ppi, "do_loop_global_var_mig",
		 &Lopti_do_loop_global_var_mig);
  L_read_parm_b (ppi, "do_loop_ind_var_str_red",
		 &Lopti_do_loop_ind_var_str_red);
  L_read_parm_b (ppi, "do_loop_ind_var_reinit",
		 &Lopti_do_loop_ind_var_reinit);
  L_read_parm_b (ppi, "do_loop_ind_var_elim", &Lopti_do_loop_ind_var_elim);
  L_read_parm_b (ppi, "do_dead_loop_rem", &Lopti_do_dead_loop_rem);
  L_read_parm_b (ppi, "preserve_loop_var", &Lopti_preserve_loop_var);
  L_read_parm_b (ppi, "do_complex_ind_elim", &Lopti_do_complex_ind_elim);
  L_read_parm_b (ppi, "debug_loop_opti", &Lopti_debug_loop_opti);
  L_read_parm_b (ppi, "do_longword_loop_opti", &Lopti_do_longword_loop_opti);
  L_read_parm_i (ppi, "store_migration_mode", &Lopti_store_migration_mode);

  /* Partial Code Elimination opts */
  L_read_parm_b (ppi, "do_PCE", &Lopti_do_PCE);
  L_read_parm_b (ppi, "do_PCE_split_critical_edges",
		 &Lopti_do_PCE_split_critical_edges);
  L_read_parm_b (ppi, "do_PCE_merge_same_cbs",
		 &Lopti_do_PCE_merge_same_cbs);
  L_read_parm_b (ppi, "do_PCE_optimize_memory_ops",
		 &Lopti_do_PCE_optimize_memory_ops);
  L_read_parm_b (ppi, "do_PCE_conservative_memory_opti",
		 &Lopti_do_PCE_conservative_memory_opti);

  L_read_parm_b (ppi, "do_PRE", &Lopti_do_PRE);
  L_read_parm_b (ppi, "do_PRE_lazy_code_motion",
		 &Lopti_do_PRE_lazy_code_motion);
  L_read_parm_b (ppi, "do_PRE_mem_copy_prop",
		 &Lopti_do_PRE_mem_copy_prop);
  L_read_parm_b (ppi, "do_PRE_merge_loads_diff_types",
		 &Lopti_do_PRE_merge_loads_diff_types);
  L_read_parm_b (ppi, "do_PRE_cutset_metric", &Lopti_do_PRE_cutset_metric);
  L_read_parm_b (ppi, "do_PRE_optimize_moves_of_numerical_constants",
		 &Lopti_do_PRE_optimize_moves_of_numerical_constants);
  L_read_parm_b (ppi, "do_PRE_optimize_moves", &Lopti_do_PRE_optimize_moves);
  L_read_parm_b (ppi, "do_PRE_optimize_single_source_ops",
		 &Lopti_do_PRE_optimize_single_source_ops);
  L_read_parm_b (ppi, "do_PRE_speculative_code_motion",
		 &Lopti_do_PRE_speculative_code_motion);

  L_read_parm_b (ppi, "do_global_mem_expression_copy_prop",
		 &Lopti_do_global_mem_expression_copy_prop);

  L_read_parm_b (ppi, "do_PDE", &Lopti_do_PDE);
  L_read_parm_b (ppi, "do_PDE_cutset_metric", &Lopti_do_PDE_cutset_metric);
  L_read_parm_b (ppi, "do_PDE_min_cut", &Lopti_do_PDE_min_cut);
  L_read_parm_b (ppi, "do_PDE_predicated", &Lopti_do_PDE_predicated);
  L_read_parm_b (ppi, "do_PDE_sink_stores", &Lopti_do_PDE_sink_stores);
  L_read_parm_b (ppi, "do_PDE_sink_only_stores", &Lopti_do_PDE_sink_only_stores);

  /* Dead store opts */
  L_read_parm_b (ppi, "do_global_dead_store_removal", 
		 &Lopti_do_global_dead_store_removal);
  L_read_parm_b (ppi, "do_dead_local_var_store_removal",
		 &Lopti_do_dead_local_var_store_removal);

  /* Miscellaneous */
  L_read_parm_b (ppi, "only_lvl1_for_zero_weight_fn",
		 &Lopti_only_lvl1_for_zero_weight_fn);
  L_read_parm_b (ppi, "do_post_inc_conv", &Lopti_do_post_inc_conv);
  L_read_parm_b (ppi, "do_mark_memory_labels", &Lopti_do_mark_memory_labels);
  L_read_parm_b (ppi, "do_mark_incoming_parms",
		 &Lopti_do_mark_incoming_parms);
  L_read_parm_b (ppi, "do_mark_trivial_sef_jsrs",
		 &Lopti_do_mark_trivial_sef_jsrs);
  L_read_parm_b (ppi, "do_mark_sync_jsrs", &Lopti_do_mark_sync_jsrs);
  L_read_parm_b (ppi, "do_mark_trivial_safe_ops",
		 &Lopti_do_mark_trivial_safe_ops);
  L_read_parm_b (ppi, "do_code_layout", &Lopti_do_code_layout);
  L_read_parm_b (ppi, "do_classify_branches", &Lopti_do_classify_branches);
  L_read_parm_b (ppi, "print_opti_count", &Lopti_print_opti_count);
  L_read_parm_b (ppi, "print_opti_breakdown", &Lopti_print_opti_breakdown);
}

void
Lopti_init ()
{
  L_alloc_danger_ext = L_create_alloc_pool ("L_Danger_Ext",
					    sizeof (struct L_Danger_Ext), 64);

  L_load_parameters (L_parm_file, L_command_line_macro_list,
		     "(Lopti", L_read_parm_lopti);

  L_native_machine_ctype = M_native_int_register_ctype ();
}

void
Lopti_deinit ()
{
  if (L_check_data_struct_alloc)
    {
      L_print_alloc_info (stderr, L_alloc_danger_ext,
			  L_debug_data_struct_alloc);
    }

  L_free_alloc_pool (L_alloc_danger_ext);
  L_alloc_danger_ext = NULL;
}
