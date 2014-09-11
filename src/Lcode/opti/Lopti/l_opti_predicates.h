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
 *      File :          l_opti_predicates.h
 *      Description :   predicate functions for optimizer
 *      Creation Date : July, 1990
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_OPTI_PREDICATES_H
#define L_OPTI_PREDICATES_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *      Opcode predicate functions
 */

extern int L_is_compatible_opc (int opc1, int opc2);
extern int L_compatible_opcodes (L_Oper *, L_Oper *);
extern int L_compatible_arithmetic_ops (L_Oper *, L_Oper *);
extern int L_compatible_to_combine_consts (L_Oper *, L_Oper *);
extern int L_unary_opcode (L_Oper *);
extern int L_are_opposite_branch_opcodes (L_Oper *, L_Oper *);
extern int L_are_reverse_branch_opcodes (L_Oper *, L_Oper *);
extern int L_are_same_branch_opcodes (L_Oper *, L_Oper *);
extern int L_load_store_sign_extend_conflict (L_Oper *, L_Oper *);
extern int L_compatible_load_store (L_Oper *, L_Oper *);
extern int L_cancelling_opcodes (L_Oper *, L_Oper *);

/*
 *      Operand predicate functions
 */
extern int L_is_legal_unsigned_value_offset_32 (unsigned int, int);
extern int L_is_int_24 (L_Operand *);
extern int L_is_int_between_0_and_128 (L_Operand *);
extern int L_is_branch_target (L_Oper *, L_Cb *);
extern int L_can_change_all_later_uses_with (L_Cb *, L_Operand *, L_Operand *,
                                             L_Oper *);
extern int L_can_change_all_uses_between (L_Cb * cb, L_Operand * operand,
					  L_Operand * replace, L_Oper * opA,
					  L_Oper *opB);

/*
 *      Oper predicate functions
 */

extern int L_increment_operation (L_Oper *);
extern int L_no_overlap_write (L_Cb *, L_Oper *, L_Oper *, int, L_Oper *,
                               int);
extern int L_no_overlap_read (L_Cb *, L_Oper *, L_Oper *, int, L_Oper *, int);
extern int L_no_sb_loop_br_between (L_Cb *, L_Oper *, L_Oper *);
extern int L_can_move_above (L_Cb *, L_Oper *, L_Oper *);
extern int L_can_move_below (L_Cb *, L_Oper *, L_Oper *);
extern int L_can_undo_and_combine (L_Oper *, L_Oper *, L_Oper *);
extern int L_can_undo_and_combine2 (L_Oper *, L_Oper *);
extern int L_can_make_mul_add_sub (L_Oper *, L_Oper *);
extern int L_can_undo_increment (L_Oper *, L_Oper *);
extern int L_no_exceptions (L_Oper *);
extern int L_will_lose_accuracy (L_Oper *);
extern int L_invertible_float_constant (float);
extern int L_invertible_constant (double);
extern int L_live_outside_cb_after (L_Cb *, L_Operand *, L_Oper *);
extern int L_not_live_outside_cb_between (L_Cb *, L_Operand *, L_Oper *,
					  L_Oper *);
extern int L_all_dest_operand_not_live_outside_cb_between (L_Cb *, L_Oper *,
                                                           L_Oper *,
                                                           L_Oper *);
extern int L_all_src_operand_not_live_outside_cb_between (L_Cb *, L_Oper *,
                                                          L_Oper *, L_Oper *);
extern int L_single_use_of (L_Cb *, L_Operand *, L_Oper *);
extern int L_no_use_of (L_Cb *, L_Operand *, L_Oper *);
extern int L_is_redefined_in_cb_after (L_Oper *, L_Operand *);
extern int L_all_uses_can_be_renamed (L_Cb *, L_Oper *, L_Operand *);
extern int L_not_live_at_cb_end (L_Cb *, L_Oper *, L_Operand *);
extern int L_no_flow_dep_dep_from_for (L_Oper *, L_Operand *);
extern int L_no_flow_dep_from (L_Cb *, L_Oper *);
extern int L_can_copy_op_to_all_live_paths (L_Cb *, L_Oper *);
extern int L_no_anti_dep_from_before_redef_of (L_Cb *, L_Oper *, L_Operand *);
extern int L_no_anti_dep_from_before_redef (L_Cb *, L_Oper *);
extern int L_single_anti_dep_from_before_redef_of (L_Oper *, L_Operand *);
extern int L_single_anti_dep_from_before_redef (L_Oper *);
extern int L_profitable_for_migration (L_Cb *, L_Oper *);
extern int L_conditionally_redefined_in_cb (L_Cb *, L_Oper *, L_Operand *);
extern int L_no_intersecting_br_between (L_Oper *, L_Oper *);
extern int L_only_disjoint_br_between (L_Oper *, L_Oper *);
extern int L_not_live_outside_cb_between (L_Cb * cb, L_Operand * operand, 
					  L_Oper * start, L_Oper * end);


/*
 *      Global optimization predicate functions
 */

extern int L_global_no_defs_between (L_Operand *, L_Cb *, L_Oper *, L_Cb *,
                                     L_Oper *);
extern int L_all_dest_operand_global_no_defs_between (L_Oper *, L_Cb *,
                                                      L_Oper *, L_Cb *,
                                                      L_Oper *);
extern int L_global_same_def_reachs (L_Operand *, L_Cb *, L_Oper *, L_Cb *,
                                     L_Oper *);
extern int L_global_share_same_def (L_Operand *, L_Cb *, L_Operand *, L_Operand *);
extern int L_all_src_operand_global_same_def_reachs (L_Oper *, L_Cb *,
                                                     L_Oper *, L_Cb *,
                                                     L_Oper *);
extern int L_global_no_defs_between_cb_only (L_Cb *, L_Cb *, L_Operand *);
extern int L_global_no_sub_call_between (L_Cb *, L_Oper *, L_Cb *, L_Oper *);
extern int L_global_no_general_sub_call_between (L_Cb *, L_Oper *, L_Cb *,
                                                 L_Oper *);
extern int L_global_no_sync_between (L_Cb *, L_Oper *, L_Cb *, L_Oper *);
extern int L_global_no_danger (int, int, int, L_Cb *, L_Oper *, L_Cb *,
                               L_Oper *);
extern int L_global_no_overlap_write (L_Cb *, L_Oper *, L_Cb *, L_Oper *);
extern int L_global_only_branch_src_operand (L_Operand *);
extern int L_global_no_danger_to_boundary (int, int, int, L_Cb *, L_Oper *,
                                           L_Cb *, L_Oper *);




/* 
 *      Loop optimization predicate functions
 */

extern int L_in_nested_loop (L_Loop *, L_Cb *);
extern int L_cb_dominates_all_loop_exit_cb (L_Loop *, int *, int, L_Cb *);
extern int L_cb_dominates_all_loop_backedge_cb (L_Loop *, int *, int, L_Cb *);
extern int L_loop_invariant_operands (L_Loop *, int *, int, L_Oper *);
extern int L_cost_effective_to_move_ops_to_preheader (L_Loop *);
extern int L_all_uses_in_loop_from (L_Loop *, int *, int, L_Cb *, L_Oper *);
extern int L_def_reachs_all_out_cb_of_loop (L_Loop *, int *, int, L_Cb *,
                                            L_Oper *);
extern int L_safe_to_move_out_of_loop (L_Loop *, int *, int, L_Cb *,
                                       L_Oper *);
extern int L_no_danger_in_loop (L_Loop *, int *, int, int, int, int);
extern int L_no_memory_conflicts_in_loop (L_Loop *, int *, int, L_Oper *,
                                          L_Oper *);
extern int L_unique_memory_location (L_Loop *, int *, int, L_Oper *, int *,
                                     int *, L_Oper **);
extern int L_all_predecessor_cb_in_loop (L_Loop *, L_Cb *);
extern int L_branch_to_loop_cb (L_Loop *, L_Oper *);
extern int L_basic_induction_var (L_Loop *, L_Operand *);
extern int L_num_constant_increment_of_ind (L_Operand *, L_Ind_Info *);
extern int L_same_ind_increment (L_Operand *, L_Operand *, L_Ind_Info *);
extern int L_int_one_increment_of_ind (L_Operand *, L_Ind_Info *);
extern int L_int_neg_one_increment_of_ind (L_Operand *, L_Ind_Info *);
extern int L_ind_increment_is_multiple_of (L_Operand *, L_Operand *,
                                           L_Ind_Info *);
extern int L_num_constant_init_val_of_ind (L_Operand *, L_Ind_Info *);
extern int L_same_ind_initial_val (L_Loop *, L_Operand *, L_Operand *,
                                   L_Ind_Info *);
extern int L_ind_var_will_reach_limit (L_Loop *, L_Oper *, L_Ind_Info *);
extern int L_can_simplify_loop_branch (L_Loop *, L_Oper *, L_Ind_Info *);
extern int L_is_str_reducible_opcode (L_Oper *);
extern int L_useful_str_red (L_Loop *, L_Oper *, L_Operand *);
extern int L_not_live_in_out_cb (int *, int, L_Operand *);
extern int L_no_uses_of_ind (L_Loop *, int *, int, L_Operand *);
extern int L_ind_should_be_reinitialized (L_Loop *, int *, int, L_Operand *);
extern int L_basic_ind_var_in_same_family (L_Loop *, int *, int, L_Operand *,
                                           L_Operand *);
extern int L_no_uses_of_between_first_and_last_defs (L_Loop *, int *, int,
                                                     L_Operand *,
                                                     L_Operand *);
extern int L_ind_constant_offset_initial_val (L_Cb *, L_Operand *,
                                              L_Operand *, L_Oper *, L_Oper *,
                                              L_Ind_Info *);
extern int L_only_used_as_base_addr_with_const_offset_in_loop (L_Loop *,
                                                               int *, int,
                                                               L_Operand *,
                                                               int);
extern int L_all_uses_of_ind_can_change_offset (L_Loop *, int *, int, int *,
                                                int, L_Operand *,
                                                L_Operand *);
extern int L_can_modify_dep_branches_in_loop (L_Loop *, int *, int, L_Oper *,
					      L_Cb *);
extern int L_ind_only_used_with_loop_inv_operands (L_Loop *, int *, int,
                                                   L_Operand *);
extern int L_ind_only_used_with_memory_ops (L_Loop *, int *, int,
                                            L_Operand *);
extern int L_all_uses_of_ind_can_be_modified1 (L_Loop *, int *, int,
                                               L_Operand *);
extern int L_all_uses_of_ind_can_be_modified2 (L_Loop *, int *, int,
                                               L_Operand *);
extern int L_better_to_eliminate_operand2 (L_Loop *, int *, int, int *, int,
                                           L_Operand *, L_Operand *);
extern int L_cost_effective_for_ind_complex_elim (L_Loop *, int *, int);
extern int L_live_at_nondominated_exits (L_Loop *, int *, int, L_Cb *,
                                         L_Operand *);
extern int L_is_loop_var (L_Loop *, int *, int, L_Operand *,
                          L_Ind_Info * ind_info);

/* 
 *      Jump optmization predicate functions
 */

extern int L_empty_block (L_Cb *);
extern int L_only_uncond_branch_in_block (L_Cb *);
extern int L_multiple_cond_branch_in_block (L_Cb *);
extern int L_cb_contains_prologue (L_Cb *);
extern int L_cb_contains_epilogue (L_Cb *);
extern int L_cb_contains_multiple_branches_to (L_Cb *, L_Cb *);
extern int L_need_fallthru_path (L_Cb *);

/*
 *      Pre/post increment predicate functions
 */

extern int L_marked_as_post_increment (L_Oper *);
extern int L_marked_as_pre_increment (L_Oper *);
extern int L_marked_as_pre_post_increment (L_Oper *);
extern int L_can_recombine_mem_inc_ops (L_Oper *, L_Oper *);
extern int L_can_recombine_inc_mem_ops (L_Oper *, L_Oper *);
extern int L_can_make_post_inc (L_Oper *, L_Oper *);
extern int L_can_make_pre_inc (L_Oper *, L_Oper *);
extern int L_ind_var_is_updated_by_pre_post_increment (L_Loop *, int *, int,
                                                       L_Operand *);

extern int L_extension_compatible_ctype (ITuint8 from, ITuint8 to);
extern int L_redundant_extension_rev (L_Oper * src_op, L_Oper * ext_op);
extern int L_redundant_extension_fwd (L_Oper * ext_op, L_Oper * dst_op);

extern int L_extension_compatible_ctype (ITuint8 from, ITuint8 to);

extern int L_can_modify_dep_branches_in_loop (L_Loop * loop, int *loop_cb, 
					      int num_cb,
					      L_Oper * start_op, 
					      L_Cb * start_cb);

/*
 *     Branch value propagation predicate functions
 */
extern int L_cb_first_flow_const_compare_branch (L_Cb *, L_Operand**,
						 L_Operand**);
extern int L_cb_all_incoming_flows_same_const_compare (L_Cb *, L_Operand*,
						       L_Operand*);
#endif


