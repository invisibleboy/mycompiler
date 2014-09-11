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
 *      File :          lp_opti_functions.h
 *      Description :   data struct manipulation functs for optimizer
 *      Creation Date : July, 1990
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_OPTI_FUNCTIONS_H
#define L_OPTI_FUNCTIONS_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_code.h>

/*
 *      Data structure for register pressure information
 */

typedef struct L_Reg_Count
{
  int int_count;
  int flt_count;
  int dbl_count;
  int prd_count;
}
L_Reg_Count;

/*==========================================================================*/
/*
 *      Function prototypes
 */
/*==========================================================================*/

extern void L_create_divide_operations (L_Oper * div_oper, L_Cb * cb);
extern void L_create_rem_operations (L_Oper * rem_oper, L_Cb * cb);
extern L_Oper *L_create_move (L_Operand * dest, L_Operand * src);
extern L_Oper *L_create_move_using (L_Operand * dest, L_Operand * src,
                                    L_Oper * oper);
extern void L_convert_to_zero_extend_oper (L_Oper *, L_Operand *,
                                           L_Operand *);
extern void L_convert_to_move (L_Oper *, L_Operand *, L_Operand *);
extern void L_convert_to_move_of_zero (L_Oper *, L_Operand *);
extern void L_convert_to_move_of_one (L_Oper *, L_Operand *);
extern void L_convert_to_jump (L_Oper *, L_Operand *);
extern void L_convert_to_extended_move (L_Oper *, L_Operand *, L_Operand *);
extern void L_convert_to_extract (L_Oper *, L_Operand *, L_Operand *, int);
extern void L_fix_cond_br (L_Cb *, L_Oper *, int);

extern ITintmax L_evaluate_int_arithmetic (L_Oper *);
extern float L_evaluate_flt_arithmetic (L_Oper *);
extern double L_evaluate_dbl_arithmetic (L_Oper *);

/* Compare evaluation for Lop_BR, Lop_CMP, Lop_RCMP
 * with constant operands -- JWS 20000720
 */

extern int L_evaluate_int_compare_with_sources (L_Oper *, ITintmax, ITintmax);
extern int L_evaluate_flt_compare_with_sources (L_Oper *, float, float);
extern int L_evaluate_dbl_compare_with_sources (L_Oper *, double, double);
extern int L_evaluate_int_compare (L_Oper *);
extern int L_evaluate_flt_compare (L_Oper *);
extern int L_evaluate_dbl_compare (L_Oper *);
extern int L_evaluate_compare (L_Oper *);

extern int L_move_from_ctype (int);
extern int L_corresponding_load (L_Oper *);
extern int L_corresponding_preincrement_load (L_Oper *);
extern int L_corresponding_postincrement_load (L_Oper *);
extern int L_corresponding_store (L_Oper *);
extern int L_corresponding_preincrement_store (L_Oper *);
extern int L_corresponding_postincrement_store (L_Oper *);
extern int L_corresponding_mov (L_Oper *);
extern int L_corresponding_add (L_Oper *);
extern int L_corresponding_mul_add (L_Oper *);
extern int L_corresponding_mul_sub (L_Oper *);
extern int L_corresponding_mul_sub_rev (L_Oper *);
extern int L_inverse_arithmetic (int);

extern int L_has_const_operand_and_realign_oper (L_Oper *);

extern void L_undo_and_combine (L_Oper *, L_Oper *);
extern int L_combine_operations (L_Oper *, L_Oper *);
extern void L_undo_increment (L_Oper *, L_Oper *);
extern void L_combine_increment_operations (L_Oper *, L_Oper *);

extern void L_move_oper_before (L_Cb *, L_Oper *, L_Oper *);
extern void L_move_oper_after (L_Cb *, L_Oper *, L_Oper *);
extern L_Cb *L_split_cb_after (L_Func *, L_Cb *, L_Oper *);
extern L_Cb *L_split_arc (L_Func *fn, L_Cb *src_cb, L_Flow *dst_fl);
#define LOPTI_EXPAND_ALLOW_FALLTHRU   1
#define LOPTI_EXPAND_ALLOW_UNCOND     2
#define LOPTI_EXPAND_ALLOW_COND       4
extern int L_expand_flow (L_Func * fn, L_Cb * src_cb, L_Flow * oefl,
			  int flags);

extern void L_insert_op_at_dest_of_br (L_Cb *, L_Oper *, L_Oper *, int);
extern void L_insert_op_at_fallthru_dest (L_Cb *, L_Oper *, int);

extern L_Oper *L_setup_conditional_op_with_pred (L_Cb *);
extern L_Oper *L_setup_conditional_op_with_cbs (L_Cb *);
extern L_Cb *L_create_conditional_op_with_pred_in_cb (L_Oper *, L_Oper *, 
						      L_Cb *, L_Oper *);
extern L_Cb *L_create_conditional_op_with_cbs_at_cb (L_Func *, L_Oper *, 
						     L_Oper *, L_Cb *, 
						     L_Oper *);
/* 
 *      Loop optimization functions
 */

extern L_Oper *L_find_last_def_in_cb (L_Cb *, L_Operand *);
extern void L_simplify_loop_branch1 (L_Loop *, L_Oper *);
extern void L_simplify_loop_branch2 (L_Loop *, L_Oper *);
extern int L_str_reduced_opcode (L_Oper *);
extern ITintmax L_evaluate_str_reduced_opcode (int, ITintmax, ITintmax);
extern void L_insert_strength_reduced_op_into_loop (L_Loop *, int *, int,
                                                    L_Oper *, L_Operand *);
extern void L_reinit_induction_var (L_Loop *, int *, int, L_Operand *,
                                    L_Oper *);
extern void L_delete_all_basic_ind_var_op_from_loop (L_Loop *, int *, int,
                                                     L_Operand *);
extern int L_reorder_ops_so_no_use_betw (L_Loop *, L_Cb *, L_Operand *,
                                         L_Oper *, L_Oper *);
extern int L_find_ind_initial_offset (L_Cb *, L_Operand *, L_Operand *,
                                      L_Oper *, L_Oper *, L_Ind_Info *);
extern void L_simplify_combs_of_ind_vars (L_Loop *, int *, int);
extern void L_induction_elim_1 (L_Loop *, int *, int, L_Operand *,
                                L_Operand *);
extern void L_induction_elim_2 (L_Loop *, int *, int, L_Operand *,
                                L_Operand *);
extern void L_induction_elim_3 (L_Loop *, int *, int, L_Operand *,
                                L_Operand *);
extern void L_induction_elim_4 (L_Loop *, int *, int, L_Operand *,
                                L_Operand *);
extern L_Oper *L_find_last_use_of_ind_var (L_Loop *, int *, int, int *, int,
                                           L_Cb *, L_Oper *);
extern int L_num_ind_var_to_reassociate (L_Loop *, int *, int, L_Operand *);
extern void L_reassociate_ind_var (L_Loop *, int *, int, L_Operand *,
                                   L_Oper *, int);

/*
 *      Pre/post increment functions
 */

extern void L_mark_as_post_increment (L_Oper *, L_Oper *);
extern void L_mark_as_pre_increment (L_Oper *, L_Oper *);
extern void L_unmark_as_post_increment (L_Oper *, L_Oper *);
extern void L_unmark_as_pre_increment (L_Oper *, L_Oper *);
extern void L_unmark_as_pre_post_increment (L_Oper *);
extern int L_breakup_pre_post_inc_ops (L_Func *);
extern int L_remove_uncombinable_pre_post_inc_ops (L_Func *);
extern int L_generate_pre_post_inc_ops (L_Func *);
extern void L_unmark_all_pre_post_increments_for_ind (L_Loop *, int *, int,
                                                      L_Operand *);
extern void L_unmark_all_pre_post_increments_for_operand (L_Func *,
                                                          L_Operand *);
extern void L_unmark_all_pre_post_increments (L_Func *);

/*
 *      Register pressure estimate functions
 */

extern int L_num_reg_used (int);
extern void L_reset_reg_count (L_Reg_Count *);
extern void L_increment_reg_count (int, L_Reg_Count *);
extern void L_update_reg_count (L_Reg_Count *, L_Reg_Count *);
extern void L_reset_ctype_array ();
extern void L_setup_ctype_array (L_Func *);
extern void L_estimate_num_live_regs_in_cb (L_Cb *);
extern void L_estimate_num_live_regs_in_loop (L_Loop *, int *, int);
extern void L_estimate_num_live_regs_in_func (L_Func *);

/*
 *      Setting function flags
 */
extern void L_set_function_mask_flag (L_Func *);
extern void L_mark_superblocks (L_Func *);

extern L_Oper *L_find_pred_definition (L_Oper * oper);

extern void L_rename_subsequent_uses (L_Oper *ren_oper, L_Oper *def_oper, 
				      L_Operand *dest_reg, L_Operand *new_reg);

#endif
