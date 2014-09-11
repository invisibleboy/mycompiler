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
 *      File :          l_compare.h
 *      Description :   New comparison format for branches, compares, and
 *                      register compares
 *      Creation Date : August 2000
 *      Author :        John Sias, Wen-mei Hwu
 *
 *==========================================================================*/

#ifndef L_COMPARE_H
#define L_COMPARE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   * General templates for Boolean predicates
   * ------------------------------------------------------------------------
   */

#define L_specific_comp_op(oper,opct,comt,ctype) ((oper)==NULL? 0 : \
        (((oper)->opc == opct) && \
         (L_is_ctype_ ## ctype ## _direct((oper)->com[0])) && \
         ((oper)->com[1] == comt)))

#define L_specific_gen_comp_op(oper,opct,comt) ((oper)==NULL? 0 : \
        (((oper)->opc == opct) && \
         ((oper)->com[1] == comt)))
  /*
   * Predicate comparison Boolean predicates
   * ------------------------------------------------------------------------
   */

#define L_general_pred_comparison_opcode(oper) ((oper)==NULL? 0 :           \
                                              ((oper)->opc == Lop_CMP) ||   \
                                              ((oper)->opc == Lop_CMP_F))

#define L_pred_copy_opcode(oper)    ((oper)==NULL? 0 :                      \
                                              ((oper)->opc==Lop_PRED_COPY))

#define L_int_pred_comparison_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc==Lop_CMP))

#define L_signed_int_pred_comparison_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_CMP) && \
         L_is_ctype_integer_direct((oper)->com[0]) && \
         L_is_ctype_signed_direct((oper)->com[0])))

#define L_unsigned_int_pred_comparison_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_CMP) && \
         L_is_ctype_integer_direct((oper)->com[0]) && \
         !L_is_ctype_signed_direct((oper)->com[0])))

#define L_fp_pred_comparison_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc==Lop_CMP_F))

#define L_flt_pred_comparison_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_CMP_F) && \
         L_is_ctype_float_direct((oper)->com[0])))

#define L_dbl_pred_comparison_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_CMP_F) && \
         L_is_ctype_double_direct((oper)->com[0])))

#define L_pred_compare_opcode(oper) L_general_pred_comparison_opcode(oper)

#define L_specific_gen_cmp_op(oper,comt) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_CMP || (oper)->opc == Lop_CMP_F) && \
         ((oper)->com[1] == comt)))

#define L_gen_eq_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_EQ)

#define L_gen_ne_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_NE)

#define L_gen_gt_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_GT)

#define L_gen_lt_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_LT)

#define L_gen_ge_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_GE)

#define L_gen_le_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_LE)

#define L_gen_tz_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_TZ)

#define L_gen_tn_cmp_opcode(op) L_specific_gen_cmp_op(op,Lcmp_COM_TN)

#define L_int_eq_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_EQ,int)
#define L_int_ne_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_NE,int)
#define L_int_gt_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_GT,int)
#define L_int_lt_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_LT,int)
#define L_int_ge_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_GE,int)
#define L_int_le_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_LE,int)
#define L_int_tz_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_TZ,int)
#define L_int_tn_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP, \
                                                       Lcmp_COM_TN,int)

#define L_flt_eq_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_EQ,float)
#define L_flt_ne_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_NE,float)
#define L_flt_gt_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_GT,float)
#define L_flt_lt_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_LT,float)
#define L_flt_ge_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_GE,float)
#define L_flt_le_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_LE,float)

#define L_dbl_eq_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_EQ,double)
#define L_dbl_ne_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_NE,double)
#define L_dbl_gt_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_GT,double)
#define L_dbl_lt_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_LT,double)
#define L_dbl_ge_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_GE,double)
#define L_dbl_le_cmp_opcode(op) L_specific_comp_op(op,Lop_CMP_F, \
                                                       Lcmp_COM_LE,double)

  /*
   * Register comparison Boolean predicates
   * ------------------------------------------------------------------------
   */

#define L_general_comparison_opcode(oper) ((oper)==NULL? 0 :                \
        (((oper)->opc==Lop_RCMP) || ((oper)->opc==Lop_RCMP_F)))

#define L_int_comparison_opcode(oper) ((oper)==NULL? 0 :                    \
                                  ((oper)->opc==Lop_RCMP))

#define L_signed_int_comparison_opcode(oper) ((oper)==NULL? 0 :             \
                                  (((oper)->opc==Lop_RCMP) &&               \
                                  L_is_ctype_integer_direct((oper)->com[0]) \
                                  &&                                        \
                                  L_is_ctype_signed_direct((oper)->com[0])))

#define L_unsigned_int_comparison_opcode(oper) ((oper)==NULL? 0 :           \
                                  (((oper)->opc==Lop_RCMP) &&               \
                                  L_is_ctype_integer_direct((oper)->com[0]) \
                                  &&                                        \
                                  !L_is_ctype_signed_direct((oper)->com[0])))

#define L_flt_comparison_opcode(oper) ((oper)==NULL? 0 :                    \
                                  (((oper)->opc==Lop_RCMP_F) &&             \
                                  L_is_ctype_float_direct((oper)->com[0])))

#define L_dbl_comparison_opcode(oper) ((oper)==NULL? 0 :                    \
                                  (((oper)->opc==Lop_RCMP_F) &&             \
                                  L_is_ctype_double_direct((oper)->com[0])))

#define L_specific_gen_br_op(oper,comt) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_BR || (oper)->opc == Lop_BR_F) && \
         ((oper)->com[1] == comt)))

#define L_specific_gen_rcmp_op(oper,comt) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_RCMP || (oper)->opc == Lop_RCMP_F) && \
         ((oper)->com[1] == comt)))

#define L_gen_eq_rcmp_opcode(op) L_specific_gen_rcmp_op(op,Lcmp_COM_EQ)

#define L_gen_ne_rcmp_opcode(op) L_specific_gen_rcmp_op(op,Lcmp_COM_NE)

#define L_gen_gt_rcmp_opcode(op) L_specific_gen_rcmp_op(op,Lcmp_COM_GT)

#define L_gen_lt_rcmp_opcode(op) L_specific_gen_rcmp_op(op,Lcmp_COM_LT)

#define L_gen_ge_rcmp_opcode(op) L_specific_gen_rcmp_op(op,Lcmp_COM_GE)

#define L_gen_le_rcmp_opcode(op) L_specific_gen_rcmp_op(op,Lcmp_COM_LE)

#define L_int_eq_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP, \
                                                       Lcmp_COM_EQ,int)
#define L_int_ne_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP, \
                                                       Lcmp_COM_NE,int)
#define L_int_gt_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP, \
                                                       Lcmp_COM_GT,int)
#define L_int_lt_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP, \
                                                       Lcmp_COM_LT,int)
#define L_int_ge_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP, \
                                                       Lcmp_COM_GE,int)
#define L_int_le_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP, \
                                                       Lcmp_COM_LE,int)
#define L_flt_eq_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_EQ,float)
#define L_flt_ne_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_NE,float)
#define L_flt_gt_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_GT,float)
#define L_flt_lt_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_LT,float)
#define L_flt_ge_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_GE,float)
#define L_flt_le_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_LE,float)
#define L_dbl_eq_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_EQ,double)
#define L_dbl_ne_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_NE,double)
#define L_dbl_gt_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_GT,double)
#define L_dbl_lt_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_LT,double)
#define L_dbl_ge_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_GE,double)
#define L_dbl_le_rcmp_opcode(op) L_specific_comp_op(op,Lop_RCMP_F, \
                                                       Lcmp_COM_LE,double)

  /*
   * Compare-and-branch Boolean predicates
   * ------------------------------------------------------------------------
   */

#define L_cond_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_BR) || ((oper)->opc==Lop_BR_F)))

#define L_int_cond_branch_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc==Lop_BR))

#define L_signed_int_cond_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_BR) && \
         L_is_ctype_integer_direct((oper)->com[0]) && \
         L_is_ctype_signed_direct((oper)->com[0])))

#define L_unsigned_int_cond_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_BR) && \
         L_is_ctype_integer_direct((oper)->com[0]) && \
         !L_is_ctype_signed_direct((oper)->com[0])))

#define L_flt_cond_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_BR_F) && \
         L_is_ctype_float_direct((oper)->com[0])))

#define L_dbl_cond_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_BR_F) && \
         L_is_ctype_double_direct((oper)->com[0])))

#define L_gen_beq_branch_opcode(op) L_specific_gen_br_op(op,Lcmp_COM_EQ)

#define L_gen_bne_branch_opcode(op) L_specific_gen_br_op(op,Lcmp_COM_NE)

#define L_gen_bgt_branch_opcode(op) L_specific_gen_br_op(op,Lcmp_COM_GT)

#define L_gen_blt_branch_opcode(op) L_specific_gen_br_op(op,Lcmp_COM_LT)

#define L_gen_bge_branch_opcode(op) L_specific_gen_br_op(op,Lcmp_COM_GE)

#define L_gen_ble_branch_opcode(op) L_specific_gen_br_op(op,Lcmp_COM_LE)

#define L_int_beq_branch_opcode(op) L_specific_comp_op(op,Lop_BR, \
                                                       Lcmp_COM_EQ,int)
#define L_int_bne_branch_opcode(op) L_specific_comp_op(op,Lop_BR, \
                                                       Lcmp_COM_NE,int)
#define L_int_bgt_branch_opcode(op) L_specific_comp_op(op,Lop_BR, \
                                                       Lcmp_COM_GT,int)
#define L_int_blt_branch_opcode(op) L_specific_comp_op(op,Lop_BR, \
                                                       Lcmp_COM_LT,int)
#define L_int_bge_branch_opcode(op) L_specific_comp_op(op,Lop_BR, \
                                                       Lcmp_COM_GE,int)
#define L_int_ble_branch_opcode(op) L_specific_comp_op(op,Lop_BR, \
                                                       Lcmp_COM_LE,int)
#define L_flt_beq_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_EQ,float)
#define L_flt_bne_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_NE,float)
#define L_flt_bgt_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_GT,float)
#define L_flt_blt_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_LT,float)
#define L_flt_bge_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_GE,float)
#define L_flt_ble_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_LE,float)
#define L_dbl_beq_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_EQ,double)
#define L_dbl_bne_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_NE,double)
#define L_dbl_bgt_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_GT,double)
#define L_dbl_blt_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_LT,double)
#define L_dbl_bge_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_GE,double)
#define L_dbl_ble_branch_opcode(op) L_specific_comp_op(op,Lop_BR_F, \
                                                       Lcmp_COM_LE,double)

#define L_uncond_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_JUMP) || ((oper)->opc==Lop_JUMP_FS)))
#define L_register_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_JUMP_RG) || ((oper)->opc==Lop_JUMP_RG_FS)))
#define L_register_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_JUMP_RG) || ((oper)->opc==Lop_JUMP_RG_FS)))
#define L_general_branch_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_JUMP) && ((oper)->opc<=Lop_BR_F)))
#define L_is_control_oper(oper) (L_general_branch_opcode(oper) || \
                                 (L_check_opcode(oper) &&         \
                                  oper->src[1]!=NULL))

/* DIA - predicate aware versions of (un)cond branch  */
#define L_uncond_branch(oper)  ((L_uncond_branch_opcode(oper) && \
                                !(oper)->pred[0]))

#define L_cond_branch(oper) ((L_cond_branch_opcode(oper)) || \
                             (L_uncond_branch_opcode(oper) && (oper)->pred[0]))

#define L_cond_ret(oper)    ((L_subroutine_return_opcode(oper) && \
			     (oper)->pred[0]))

#define L_branch_dest(oper) ((L_cond_branch_opcode(oper)) ? \
        ((oper)->src[2]->value.cb) : ((oper)->src[0]->value.cb))

#define L_general_int_predicate_define_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_CMP))

  /* this doesn't count initializing predicates */
#define L_general_predicate_define_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_CMP) || ((oper)->opc == Lop_CMP_F)))

#define L_initializing_predicate_define_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_PRED_SET) || ((oper)->opc == Lop_PRED_CLEAR) || \
         ((oper)->opc == Lop_PRED_COPY)))


#define L_pred_set(oper)   ((oper)==NULL? 0 : ((oper)->opc == Lop_PRED_SET))
#define L_pred_clear(oper) ((oper)==NULL? 0 : ((oper)->opc == Lop_PRED_CLEAR))
#define L_pred_copy(oper)  ((oper)==NULL? 0 : ((oper)->opc == Lop_PRED_COPY))

#define L_inequality_compare(oper) ((oper)==NULL? 0 : \
                                    (((oper)->com[1] == Lcmp_COM_GT) || \
                                     ((oper)->com[1] == Lcmp_COM_LE) || \
                                     ((oper)->com[1] == Lcmp_COM_GE) || \
                                     ((oper)->com[1] == Lcmp_COM_LT)))

  /*
   *    Lcmp_COM_* Compare manipulation
   * ------------------------------------------------------------------------
   */

  extern void L_swap_compare (L_Oper *);
  extern void L_negate_compare (L_Oper *);
  extern void L_set_compare_type (L_Oper * oper, ITuint8 com);
  extern void L_set_compare (L_Oper * oper, ITuint8 ctype, ITuint8 com);
  extern void L_copy_compare (L_Oper * oper_to, L_Oper * oper_from);
  extern ITuint8 L_get_compare_type (L_Oper * oper);
  extern ITuint8 L_get_compare_ctype (L_Oper * oper);

  /*
   *  Produce opposite / inverse of a specfied completer.
   *  opposite = logical negation
   *  inverse  = change order of operands
   */
  extern ITuint8 L_opposite_pred_completer (ITuint8 com);
  extern ITuint8 L_inverse_pred_completer (ITuint8 com);

  /*
   *  Compare only completers for equivalence
   */
  extern int L_same_compare (L_Oper * opA, L_Oper * opB);
  extern int L_opposite_compare (L_Oper * opA, L_Oper * opB);
  extern int L_inverse_compare (L_Oper * opA, L_Oper * opB);
  extern int L_inv_opp_compare (L_Oper * opA, L_Oper * opB);

  /*
   *  Compare completers and opcodes for equivalence
   */
  extern int L_equivalent_comparison_opcode (L_Oper *, L_Oper *);
  extern int L_complement_comparison_opcode (L_Oper *, L_Oper *);
  extern int L_reversed_comparison_opcode (L_Oper *, L_Oper *);
  extern int L_rev_complement_comparison_opcode (L_Oper *, L_Oper *);

  /*
   *  Change opcodes to render one operation into another
   *  (manipulates opcodes only)
   */
  extern void L_change_to_br_op (L_Oper * op);
  extern void L_change_to_cmp_op (L_Oper * op);
  extern void L_change_to_rcmp_op (L_Oper * op);

  extern L_Oper *L_new_pred_copy (L_Operand * dest, L_Operand * src);

#ifdef __cplusplus
}
#endif
#endif
