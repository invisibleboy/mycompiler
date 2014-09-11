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


/*===========================================================================
 *      File :          l_super_parms.h
 *      Description :   Superscalar optimization parms and config
 *      Author :        Scott Mahlke
 *
 *      (C) Copyright 1990, Scott Mahlke
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

#ifndef L_SUPER_PARMS_H
#define L_SUPER_PARMS_H

#include <library/func_list.h>

/*==========================================================================*/
/*
 *      Lsuperscalar parameters
 */
/*==========================================================================*/

/*
 *      high level opti switches
 */

extern int Lsuper_do_sb_formation;
extern int Lsuper_do_only_sb_formation;
extern int Lsuper_mark_softpipe_loops;
extern int Lsuper_do_jump_opti;
extern int Lsuper_do_dead_block;
extern int Lsuper_do_branches_to_next_block;
extern int Lsuper_do_branches_to_same_target;
extern int Lsuper_do_branches_to_uncond_branch;
extern int Lsuper_do_merge;
extern int Lsuper_do_combine_labels;
extern int Lsuper_do_branch_target_exp;
extern int Lsuper_do_branch_swap;
extern int Lsuper_do_branch_pred;
extern int Lsuper_do_classic_opti;
extern int Lsuper_do_const_prop;
extern int Lsuper_do_rev_copy_prop;
extern int Lsuper_do_copy_prop;
extern int Lsuper_do_mem_copy_prop;
extern int Lsuper_do_common_sub;
extern int Lsuper_do_red_load;
extern int Lsuper_do_red_store;
extern int Lsuper_do_local_str_red;
extern int Lsuper_do_const_comb;
extern int Lsuper_do_const_fold;
extern int Lsuper_do_br_fold;
extern int Lsuper_do_dead_code;
extern int Lsuper_do_code_motion;
extern int Lsuper_do_op_fold;
extern int Lsuper_do_op_cancel;
extern int Lsuper_do_local_op_mig;
extern int Lsuper_do_remove_sign_ext;
extern int Lsuper_do_reduce_logic;
extern int Lsuper_do_str_red;
extern int Lsuper_do_multiway_branch_opti;
extern int Lsuper_do_loop_classic_opti;
extern int Lsuper_do_loop_inv_code_rem;
extern int Lsuper_do_loop_global_var_mig;
extern int Lsuper_do_loop_op_fold;
extern int Lsuper_do_loop_dead_code;
extern int Lsuper_do_loop_ind_var_elim;
extern int Lsuper_do_loop_ind_var_elim2;
extern int Lsuper_do_loop_ind_reinit;
extern int Lsuper_do_loop_post_inc_conv;
extern int Lsuper_do_loop_unroll;
extern int Lsuper_unroll_with_remainder_loop;
extern int Lsuper_unroll_pipelined_loops;
extern int Lsuper_fixed_length_unroll;
extern int Lsuper_do_acc_exp;
extern int Lsuper_do_float_acc_exp;
extern int Lsuper_push_comp_code_out_of_softpipe_loops;
extern int Lsuper_do_peel_opt;
extern int Lsuper_do_op_migration;
extern int Lsuper_do_critical_path_red;
extern int Lsuper_do_rename_disjvreg;
extern int Lsuper_do_branch_combining;

extern char *Lsuper_prevent_superblock_functions;
extern int Lsuper_verbose_prevent_superblock;
extern char *Lsuper_prevent_ILP_opti_functions;
extern int Lsuper_verbose_prevent_ILP_opti;

/*
 *      more detailed optimization control
 */

extern int Lsuper_do_post_inc_conv;
extern int Lsuper_issue_rate;
extern int Lsuper_height_count_crossover;
extern int Lsuper_pred_exec_support;
extern int Lsuper_do_pred_promotion;
extern int Lsuper_allow_extra_unrolling_for_small_loops;
extern int Lsuper_max_unroll_allowed;
extern int Lsuper_max_unroll_op_count;
extern int Lsuper_unroll_only_marked_loops;
extern int Lsuper_allow_backedge_exp;
extern int Lsuper_allow_expansion_of_loops;
extern int Lsuper_max_number_of_predicates;
extern int Lsuper_store_migration_mode;

/*
 *      high level debug switches
 */
extern int Lsuper_debug_str_red;
extern int Lsuper_debug_multiway_branch_opti;
extern int Lsuper_debug_loop_classic_opti;
extern int Lsuper_debug_loop_unroll;
extern int Lsuper_debug_acc_exp;
extern int Lsuper_debug_peel_opt;
extern int Lsuper_debug_op_migration;
extern int Lsuper_debug_critical_path_red;

extern int Lsuper_debug_dump_intermediates;

/* file for statistics on loops marked for pipelining */
extern FILE *softpipe_statfile;

extern Func_Name_List *Lsuper_prevent_superblock_list;
extern Func_Name_List *Lsuper_prevent_ILP_opti_list;

extern void Lsuper_initialize (Parm_Macro_List * command_line_macro_list);
extern void Lsuper_cleanup (void);

#endif
