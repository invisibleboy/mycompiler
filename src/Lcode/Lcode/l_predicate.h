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
 *      File :          l_predicate.h
 *      Description :   lcode functions related to predicates
 *      Creation Date : November 1993
 *      Author :        Scott Mahlke, David August, John Sias, Wen-mei Hwu
 *
 *==========================================================================*/
/* 09/17/02 REK Adding declaration for L_is_transparent_predicate_ptype. */
#ifndef L_PREDICATE_H
#define L_PREDICATE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <bdd/cudd.h>

#define L_VPRED_DST_ATTR_NAME   "vpd"
#define L_VPRED_SRC_ATTR_NAME   "vps"
#define L_VPRED_PRD_ATTR_NAME   "vpp"

#define L_assign_ptype_null(opd)         (((opd)->ptype = L_PTYPE_NULL))
#define L_assign_ptype_uncond_true(opd)  (((opd)->ptype = L_PTYPE_UNCOND_T))
#define L_assign_ptype_uncond_false(opd) (((opd)->ptype = L_PTYPE_UNCOND_F))
#define L_assign_ptype_cond_true(opd)    (((opd)->ptype = L_PTYPE_COND_T))
#define L_assign_ptype_cond_false(opd)   (((opd)->ptype = L_PTYPE_COND_F))
#define L_assign_ptype_or_true(opd)      (((opd)->ptype = L_PTYPE_OR_T))
#define L_assign_ptype_or_false(opd)     (((opd)->ptype = L_PTYPE_OR_F))
#define L_assign_ptype_and_true(opd)     (((opd)->ptype = L_PTYPE_AND_T))
#define L_assign_ptype_and_false(opd)    (((opd)->ptype = L_PTYPE_AND_F))
#define L_assign_ptype_sand_true(opd)    (((opd)->ptype = L_PTYPE_SAND_T))
#define L_assign_ptype_sand_false(opd)   (((opd)->ptype = L_PTYPE_SAND_F))

#define L_assign_ptype_uncond_t(operand) L_assign_ptype_uncond_true(operand)
#define L_assign_ptype_uncond_f(operand) L_assign_ptype_uncond_false(operand)
#define L_assign_ptype_cond_t(operand)   L_assign_ptype_cond_true(operand)
#define L_assign_ptype_cond_f(operand)   L_assign_ptype_cond_false(operand)
#define L_assign_ptype_or_t(operand)     L_assign_ptype_or_true(operand)
#define L_assign_ptype_or_f(operand)     L_assign_ptype_or_false(operand)
#define L_assign_ptype_and_t(operand)    L_assign_ptype_and_true(operand)
#define L_assign_ptype_and_f(operand)    L_assign_ptype_and_false(operand)

#ifdef __cplusplus
extern "C"
{
#endif

/*======================================================================*/
/*
 *      L_Attr boolean predicates
 */
/*======================================================================*/

/*======================================================================*/
/*
 *      L_Flow boolean predicates
 */
/*======================================================================*/

/*======================================================================*/
/*
 *      L_PTYPE boolean predicates
 */
/*======================================================================*/

#define L_is_ptype_uncond_t(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_UNCOND_T))
#define L_is_ptype_uncond_f(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_UNCOND_F))
#define L_is_ptype_uncond(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_UNCOND_T || \
                           (operand)->ptype == L_PTYPE_UNCOND_F))
#define L_is_ptype_cond_t(operand) (operand &&  \
                           ((operand)->ptype == L_PTYPE_COND_T))
#define L_is_ptype_cond_f(operand) (operand &&  \
                           ((operand)->ptype == L_PTYPE_COND_F))
#define L_is_ptype_cond(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_COND_T || \
                           (operand)->ptype == L_PTYPE_COND_F))
#define L_is_ptype_or_t(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_OR_T))
#define L_is_ptype_or_f(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_OR_F))
#define L_is_ptype_or(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_OR_T || \
                           (operand)->ptype == L_PTYPE_OR_F))
#define L_is_ptype_and_t(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_AND_T))
#define L_is_ptype_and_f(operand) (operand && \
                           ((operand)->ptype == L_PTYPE_AND_F))
#define L_is_ptype_and(operand) (operand &&  \
                           ((operand)->ptype == L_PTYPE_AND_T ||\
                           (operand)->ptype == L_PTYPE_AND_F))

  extern int L_is_transparent_predicate_ptype (int ptype);
  extern int L_is_update_predicate_ptype (int ptype);
  extern int L_is_uncond_predicate_ptype (int ptype);
  extern int L_is_transparent_predicate_ptype (int ptype);

  extern int L_uncond_ptype (int ptype);
  extern int L_uncond_true_ptype (int ptype);
  extern int L_uncond_false_ptype (int ptype);
  extern int L_cond_ptype (int ptype);
  extern int L_cond_true_ptype (int ptype);
  extern int L_cond_false_ptype (int ptype);
  extern int L_or_ptype (int ptype);
  extern int L_or_true_ptype (int ptype);
  extern int L_or_false_ptype (int ptype);
  extern int L_and_ptype (int ptype);
  extern int L_and_true_ptype (int ptype);
  extern int L_and_false_ptype (int ptype);
  extern int L_sand_ptype (int ptype);
  extern int L_sand_true_ptype (int ptype);
  extern int L_sand_false_ptype (int ptype);
  extern int L_false_ptype (int ptype);
  extern int L_opposite_ptype (int ptype);

/*======================================================================*/
/*
 *      L_Operand boolean predicates
 */
/*======================================================================*/

  extern int L_is_pred_register (L_Operand *);
  extern int L_is_pred_macro (L_Operand *);
  extern int L_is_pred_variable (L_Operand *);

  extern int L_is_update_predicate_ptype_operand (L_Operand *);

  extern int L_equivalent_ptype (L_Operand *, L_Operand *);
  extern int L_complement_ptype (L_Operand *, L_Operand *);


  extern int L_pred_defined_multiple_times (L_Cb * cb, L_Operand * pred);
  extern int L_no_redefs_between (L_Operand *, L_Oper *, L_Oper *);
  extern int L_all_src_operand_no_redefs_between (L_Oper *, L_Oper *,
                                                  L_Oper *);
  extern int L_all_dest_operand_no_redefs_between (L_Oper *, L_Oper *,
                                                   L_Oper *);

/*======================================================================*/
/*
 *      L_Oper boolean predicates
 */
/*======================================================================*/

#define L_pred_set(oper) ((oper)==NULL? 0 : ((oper)->opc == Lop_PRED_SET))
#define L_pred_clear(oper) ((oper)==NULL? 0 : ((oper)->opc == Lop_PRED_CLEAR))
#define L_pred_copy(oper) ((oper)==NULL? 0 : ((oper)->opc == Lop_PRED_COPY))

  extern int L_initializing_pred_define_opcode (L_Oper * oper);
  extern int L_pred_combine_opcode (L_Oper * oper);
  extern int L_pred_define_opcode (L_Oper *);
  extern int L_special_pred_define_opcode (L_Oper * oper);
  extern int L_pred_load_opcode (L_Oper *);
  extern int L_pred_store_opcode (L_Oper *);

  extern int L_is_predicated (L_Oper *);

  extern int L_same_predicate (L_Oper *, L_Oper *);
  extern int L_must_be_executed_before (L_Oper *, L_Oper *);

/*======================================================================*/
/*
 *      L_Cb boolean predicates
 */
/*======================================================================*/

/*======================================================================*/
/*
 *      L_Attr functions
 */
/*======================================================================*/

/*======================================================================*/
/*
 *      L_Flow functions
 */
/*======================================================================*/

/*======================================================================*/
/*
 *      L_Operand functions
 */
/*======================================================================*/

  extern int L_find_pred_clear (L_Cb * cb, L_Operand * operand);
  extern void L_delete_pred_clear (L_Cb * cb, L_Operand * operand);

/*======================================================================*/
/*
 *      L_Oper functions
 */
/*======================================================================*/

  extern int L_is_promoted (L_Oper *);

  extern void L_promote_predicate (L_Oper *, L_Operand *);
  extern void L_mark_oper_promoted (L_Oper *);

  extern void L_demote_predicate (L_Oper *, L_Operand *);

  extern void L_convert_pred_ops_to_dual_format (L_Func *);
  extern void L_convert_dual_format_to_pred_ops (L_Func *);

  extern void L_save_virtual_pred_reg_numbers_in_attr (L_Func *);
  extern L_Operand *L_get_vpred_dest_from_attr (L_Oper *, int);
  extern L_Operand *L_get_vpred_src_from_attr (L_Oper *, int);
  extern L_Operand *L_get_vpred_pred_from_attr (L_Oper *, int);

  extern void L_compress_pred_dests (L_Oper *);

  extern int L_is_reg_const_pred_compare (L_Oper *);
  extern int L_is_reg_const_cond_br (L_Oper * oper);
  extern int L_pred_compare_const_first (L_Oper *);
  extern L_Operand *L_pred_compare_get_reg_operand (L_Oper *);
  extern ITuintmax L_pred_compare_get_int_const (L_Oper *);

  extern int L_insert_pred_dest (L_Oper * oper, L_Operand * new_dest);

/*======================================================================*/
/*
 *      L_Cb functions
 */
/*======================================================================*/

  extern int L_count_cb_predicates (L_Cb *);
  extern void L_branch_split_cb (L_Func * fn, L_Cb * cb);

/*======================================================================*/
/*
 *      L_Fn functions
 */
/*======================================================================*/

  extern void L_set_hb_no_fallthru_flag (L_Func *);
  extern void L_insert_dummy_op_into_hyperblocks (L_Func *);

  extern int L_predicate_demotion (L_Func *fn);

  extern int L_combine_pred_defines (L_Func * fn);
  extern int L_split_pred_defines (L_Func * fn);

  extern void L_branch_split_func (L_Func * fn);

  extern void L_set_hyperblock_flags(L_Func *);
  extern void L_set_hyperblock_func_flag(L_Func *);
  extern void L_remove_PD_attr(L_Oper *, int);
  extern void L_delete_PD_attrs(L_Oper *);
  extern void L_copy_PD_attr(L_Oper *, int, L_Oper *, int);
  extern void L_merge_PD_attrs(L_Oper *, L_Oper *);

  extern void L_create_uncond_pred_defines (L_Func * fn);

#ifdef __cplusplus
}
#endif

#endif
