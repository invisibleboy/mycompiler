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
 *      File :          l_lcode.h
 *      Description :   General lcode functions
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang, Wen-mei Hwu
 *
 *==========================================================================*/
/* 09/17/02 REK Adding declaration for L_is_ctype_float. */

#ifndef L_LCODE_H
#define L_LCODE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

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
 *      L_Operand boolean predicates
 */
/*======================================================================*/
    
/*======================================================================*/
/*
 *       Macros/Functions for Ctype Restructuring
 *      
 *       NOTE: These macros/functions should be used for all references to 
 *       operand->type and operand->ctype.  Do not access this information
 *       directly.  Direct all questions concerning their use to 
 *       Jaymie Braun.  
 */
/*======================================================================*/
    
/* macros for accessing new bitfields in type and ctype */
    
/*#define L_type_is_obsolete(operand) (((operand)->type)&0X80)*/
  extern int L_type_is_obsolete (L_Operand * operand);
/*#define L_ctype_is_obsolete(operand) (((operand)->ctype)&0X80)*/
  extern int L_ctype_is_obsolete (L_Operand * operand);
/*#define L_operand_is_signed(operand) (((operand)->ctype)&0X40)*/
  extern int L_operand_is_signed (L_Operand * operand);
  extern int L_ctype_is_signed (ITuint8 ctype);

#define L_is_ctype_signed_direct(ctype) ((ctype) & 0X40)
#define L_is_ctype_unsigned_direct(ctype) (!L_is_ctype_signed_direct(ctype))
#define L_operand_is_unsigned( operand ) \
            L_is_ctype_unsigned_direct( (operand)->ctype )

/* general access */

/*#define L_is_null(operand) ((operand)==NULL)*/
  extern int L_is_null (L_Operand * operand);

/* should not be used in new code development */
/* macros for function calls which used type/ctype directly*/
  extern int L_return_old_type (L_Operand *);
  extern int L_return_old_ctype (L_Operand *);

/* should not be used in new code development */
/* macros for case statements off operand type/ctype */
  extern int L_operand_case_type (L_Operand *);
  extern int L_operand_case_ctype (L_Operand *);

/* comparison macros */
  extern int L_operand_type_same (L_Operand * op1, L_Operand * op2);
  extern int L_operand_ctype_same (L_Operand * op1, L_Operand * op2);

/* assignment of type/ctype */

/* given ctype, makes assignment of ctype to operand->ctype */
  extern void L_assign_ctype (L_Operand *, ITuint8);

/* operand->ctype = L_OPERAND_CB */
  extern void L_assign_type_cb (L_Operand * operand);
/* operand->type = L_OPERAND_IMMED; operand->ctype = ctype */
  extern void L_assign_type_int (L_Operand * operand, ITuint8 ctype);
/* operand->type = L_OPERAND_IMMED; operand->ctype = L_CTYPE_FLOAT */
  extern void L_assign_type_float (L_Operand *);
/* operand->type = L_OPERAND_IMMED; operand->ctype = L_CTYPE_DOUBLE */
  extern void L_assign_type_double (L_Operand *);
/* operand->type = L_OPERAND_STRING*/
  extern void L_assign_type_string (L_Operand * operand);
/* operand->type = L_OPERAND_LABEL*/
  extern void L_assign_type_label (L_Operand * operand);

/*operand->type = L_OPERAND_MACRO */
  extern void L_assign_type_general_macro (L_Operand *);
/* and operand->ctype = L_CTYPE_VOID */
  extern void L_assign_type_void_macro (L_Operand *);
/* and operand->ctype = L_CTYPE_INT */
  extern void L_assign_type_int_macro (L_Operand *);
/* and operand->ctype = L_CTYPE_FLOAT */
  extern void L_assign_type_float_macro (L_Operand *);
/* and operand->ctype = L_CTYPE_DOUBLE */
  extern void L_assign_type_double_macro (L_Operand *);
/* and operand->ctype = L_CTYPE_CONTROL */
  extern void L_assign_type_control_macro (L_Operand *);
/* and operand->ctype = L_CTYPE_BTR */
  extern void L_assign_type_btr_macro (L_Operand *);
/* and operand->ctype = L_CTYPE_PREDICATE */
  extern void L_assign_type_predicate_macro (L_Operand *);

/* operand->type = L_OPERAND_REGISTER */
  extern void L_assign_type_general_register (L_Operand *);
/* and operand->ctype = L_CTYPE_VOID */
  extern void L_assign_type_void_register (L_Operand *);
/* and operand->ctype = L_CTYPE_INT */
  extern void L_assign_type_int_register (L_Operand *);
/* and operand->ctype = L_CTYPE_FLOAT */
  extern void L_assign_type_float_register (L_Operand *);
/* and operand->ctype = L_CTYPE_DOUBLE */
  extern void L_assign_type_double_register (L_Operand *);
/* and operand->ctype = L_CTYPE_BTR */
  extern void L_assign_type_btr_register (L_Operand *);
/* and operand->ctype = L_CTYPE_PREDICATE */
  extern void L_assign_type_predicate_register (L_Operand *);

/* operand->type = L_OPERAND_RREGISTER */
  extern void L_assign_type_general_rregister (L_Operand *);
/* operand->ctype = L_CTYPE_VOID */
  extern void L_assign_type_void_rregister (L_Operand *);
/* operand->ctype = L_CTYPE_INT */
  extern void L_assign_type_int_rregister (L_Operand *);
/* operand->ctype = L_CTYPE_FLOAT */
  extern void L_assign_type_float_rregister (L_Operand *);
/* operand->ctype = L_CTYPE_DOUBLE */
  extern void L_assign_type_double_rregister (L_Operand *);
/* operand->ctype = L_CTYPE_BTR */
  extern void L_assign_type_btr_rregister (L_Operand *);
/* operand->ctype = L_CTYPE_PREDICATE */
  extern void L_assign_type_predicate_rregister (L_Operand *);

/* operand->type = L_OPERAND_EVR */
  extern void L_assign_type_general_evr (L_Operand *);
/* operand->ctype = L_CTYPE_VOID */
  extern void L_assign_type_void_evr (L_Operand *);
/* operand->ctype = L_CTYPE_INT */
  extern void L_assign_type_int_evr (L_Operand *);
/* operand->ctype = L_CTYPE_FLOAT */
  extern void L_assign_type_float_evr (L_Operand *);
/* operand->ctype = L_CTYPE_DOUBLE */
  extern void L_assign_type_double_evr (L_Operand *);
/* operand->ctype = L_CTYPE_BTR */
  extern void L_assign_type_btr_evr (L_Operand *);
/* operand->ctype = L_CTYPE_PREDICATE */
  extern void L_assign_type_predicate_evr (L_Operand *);

/* macros for accessing type only */

  extern int L_is_reserved (L_Operand * operand);
/* operand->type == L_OPERAND_REGISTER */
  extern int L_is_reg (L_Operand * operand);
  extern int L_is_reg_direct (ITuint8 type);
/* operand->type == L_OPERAND_RREGISTER */
  extern int L_is_rregister (L_Operand * operand);
  extern int L_is_rregister_direct (ITuint8 type);
/* operand->type == L_OPERAND_EVR */
  extern int L_is_evr (L_Operand * operand);
  extern int L_is_evr_direct (ITuint8 type);
/* operand->type = L_OPERAND_MACRO */
  extern int L_is_macro (L_Operand * operand);
  extern int L_is_macro_direct (ITuint8 type);
/* operand->type == L_OPERAND_IMMED &&L_is_ctype_integer(operand) */
  extern int L_is_int_constant (L_Operand * operand);
/* operand->type == L_OPERAND_IMMED) && operand->ctype == L_CTYPE_FLOAT */
  extern int L_is_flt_constant (L_Operand * operand);
/* operand->type == L_OPERAND_IMMED) && operand->ctype == L_CTYPE_DOUBLE */
  extern int L_is_dbl_constant (L_Operand * operand);
/* operand->type == L_OPERAND_STRING */
  extern int L_is_string (L_Operand * operand);
  extern int L_is_string_direct (ITuint8 type);
/*operand->type == L_OPERAND_CB */
  extern int L_is_cb (L_Operand * operand);
  extern int L_is_cb_direct (ITuint8 type);
/* operand->type == L_OPERAND_LABEL */
  extern int L_is_label (L_Operand * operand);
  extern int L_is_label_direct (ITuint8 type);

/* additional macros for just checking ctype */

  extern int L_is_ctype_void (L_Operand * operand);
  extern int L_is_ctype_void_direct (ITuint8 ctype);
/* integer ctype includes all sizes and sign/unsigned representation */
  extern int L_is_ctype_integer (L_Operand * operand);
  extern int L_is_ctype_int_direct (ITuint8 ctype);
  extern int L_is_ctype_llong (L_Operand * operand);
  extern int L_is_ctype_ullong (L_Operand * operand);
#define L_is_ctype_integer_direct(ctype) L_is_ctype_int_direct(ctype)
  extern int L_is_ctype_flt (L_Operand * operand);
  extern int L_is_ctype_float (L_Operand * operand);
  extern int L_is_ctype_float_direct (ITuint8 ctype);
  extern int L_is_ctype_dbl (L_Operand * operand);
  extern int L_is_ctype_double_direct (ITuint8 ctype);
  extern int L_is_ctype_control (L_Operand * operand);
  extern int L_is_ctype_control_direct (ITuint8 ctype);
  extern int L_is_ctype_btr (L_Operand * operand);
  extern int L_is_ctype_btr_direct (ITuint8 ctype);
  extern int L_is_ctype_predicate (L_Operand * operand);
  extern int L_is_ctype_predicate_direct (ITuint8 ctype);

/* macros for checking class of ctype (int, float, double, etc) */

  extern int L_ctype_numerical_class (L_Operand * oper);
  extern int L_ctype_numerical_class_direct (ITuint8 ctype);
  extern ITuint8 L_ctype_unsigned_version (ITuint8 ctype);
  extern ITuint8 L_ctype_signed_version (ITuint8 ctype);

/* macros for accessing integer size info of ctype */

  extern int L_is_size_char (L_Operand * operand);
  extern int L_is_size_char_direct (ITuint8 ctype);
  extern int L_is_size_short (L_Operand * operand);
  extern int L_is_size_short_direct (ITuint8 ctype);
  extern int L_is_size_int (L_Operand * operand);
  extern int L_is_size_int_direct (ITuint8 ctype);
  extern int L_is_size_long (L_Operand * operand);
  extern int L_is_size_long_direct (ITuint8 ctype);
  extern int L_is_size_llong (L_Operand * operand);
  extern int L_is_size_llong_direct (ITuint8 ctype);
  extern int L_is_size_lllong (L_Operand * operand);
  extern int L_is_size_lllong_direct (ITuint8 ctype);
  extern int L_is_pointer (L_Operand * operand);
  extern int L_is_pointer_direct (ITuint8 ctype);

#define L_is_ctype_same_size_direct(ctypeA, ctypeB) \
 ( (L_is_size_char_direct(ctypeA) && L_is_size_char_direct(ctypeB)) || \
   (L_is_size_short_direct(ctypeA) && L_is_size_short_direct(ctypeB)) || \
   (L_is_size_int_direct(ctypeA) && L_is_size_int_direct(ctypeB)) || \
   (L_is_size_long_direct(ctypeA) && L_is_size_long_direct(ctypeB)) || \
   (L_is_size_llong_direct(ctypeA) && L_is_size_llong_direct(ctypeB)) || \
   (L_is_size_lllong_direct(ctypeA) && L_is_size_lllong_direct(ctypeB)) )

#define L_is_ctype_same_size(ctypeA, ctypeB)     \
 ( (L_is_size_char_direct((ctypeA)->ctype) &&    \
    L_is_size_char_direct((ctypeB)->ctype) ) ||  \
   (L_is_size_short_direct((ctypeA)->ctype) &&   \
    L_is_size_short_direct((ctypeB)->ctype) ) || \
   (L_is_size_int_direct((ctypeA)->ctype) &&     \
    L_is_size_int_direct((ctypeB)->ctype) ) ||   \
   (L_is_size_long_direct((ctypeA)->ctype) &&    \
    L_is_size_long_direct((ctypeB)->ctype) ) ||  \
   (L_is_size_llong_direct((ctypeA)->ctype) &&   \
    L_is_size_llong_direct((ctypeB)->ctype) ) || \
   (L_is_size_lllong_direct((ctypeA)->ctype) &&  \
    L_is_size_lllong_direct((ctypeB)->ctype) ) )

/* specialized macros for accessing type information */

/* operand->type == L_OPERAND_REGISTER || L_OPERAND_RREGISTER 
                    || L_OPERAND_EVR */
  extern int L_is_register (L_Operand * operand);
/* operand->type == L_OPERAND_IMMED || L_OPERAND_STRING ||
   L_OPERAND_CB || L_OPERAND_LABEL */
  extern int L_is_constant (L_Operand * operand);
/* operand->type == L_OPERAND_IMMED */
  extern int L_is_numeric_constant (L_Operand * operand);
/* operand->type == L_OPERAND_REGISTER) || L_OPERAND_RREGISTER ||
   L_OPERAND_EVR || L_OPERAND_MACRO */
  extern int L_is_variable (L_Operand * operand);
/*  L_is_int_constant(operand) && operand->value.i == 0 */
  extern int L_is_int_zero (L_Operand * operand);
/* L_is_int_constant(operand) && operand->value.i == 1 */
  extern int L_is_int_one (L_Operand * operand);
/* L_is_int_constant(operand) && operand->value.i == -1 */
  extern int L_is_int_neg_one (L_Operand * operand);
/* L_is_int_constant(operand) && operand->value.i == 0 || 
   L_is_flt_constant(operand) && operand->value.f == 0.0 || 
   L_is_dbl_constant(operand) && operandt->value.f2 == 0.0 */
  extern int L_is_zero (L_Operand * operand);
/* L_is_int_constant(operand) && operand->value.i == 1 || 
   L_is_flt_constant(operand) && operand->value.f == 1.0 || 
   L_is_dbl_constant(operand) && operandt->value.f2 == 1.0 */
  extern int L_is_one (L_Operand * operand);
/* L_is_int_constant(operand) && operand->value.i == -1 || 
   L_is_flt_constant(operand) && operand->value.f == -1.0 || 
   L_is_dbl_constant(operand) && operandt->value.f2 == -1.0 */
  extern int L_is_neg_one (L_Operand * operand);
/* L_is_int_constant(operand) && C_is_log2(operand->value.i) */
  extern int L_is_power_of_two (L_Operand * operand);
/* L_is_int_constant(operand) && C_is_log2(abs(operand->value.i)) */
  extern int L_abs_is_power_of_two (L_Operand * operand);


  extern int L_all_src_is_numeric_constant (L_Oper *);
  extern int L_is_fragile_macro (L_Operand *);
  extern int L_has_fragile_macro_dest_operand (L_Oper *);
  extern int L_has_fragile_macro_src_operand (L_Oper *);
  extern int L_has_fragile_macro_operand (L_Oper *);
  extern int L_number_of_dest_operands (L_Oper *);

  extern int L_is_unsafe_macro (L_Operand *);
  extern int L_has_unsafe_macro_dest_operand (L_Oper *);
  extern int L_has_unsafe_macro_src_operand (L_Oper *);
  extern int L_has_unsafe_macro_operand (L_Oper *);

  extern int L_same_operand (L_Operand *, L_Operand *);
  extern int L_same_src_operands (L_Oper *, L_Oper *);
  extern int L_same_dest_operands (L_Oper *, L_Oper *);
  extern int L_different_operand (L_Operand *, L_Operand *);
  extern int L_different_src_and_dest_operands (L_Oper *);
  extern int L_is_src_operand (L_Operand *, L_Oper *);
  extern int L_is_dest_operand (L_Operand *, L_Oper *);
  extern int L_is_implied_ret_destination_operand (L_Oper *, L_Operand *);
  extern int L_same_def_reachs (L_Operand *, L_Oper *, L_Oper *);
  extern int L_all_src_operand_same_def_reachs (L_Oper *, L_Oper *, L_Oper *);
  extern int L_all_dest_operand_same_def_reachs (L_Oper *, L_Oper *,
                                                 L_Oper *);
  extern int L_no_defs_between (L_Operand *, L_Oper *, L_Oper *);
  extern int L_no_defs_in_range (L_Operand *, L_Oper *, L_Oper *);
  extern int L_no_defs_between_wrt_opA (L_Operand *, L_Oper *, L_Oper *);
  extern int L_all_src_operand_no_defs_between (L_Oper *, L_Oper *, L_Oper *);
  extern int L_all_dest_operand_no_defs_between (L_Oper *, L_Oper *,
                                                 L_Oper *);
  extern int L_all_dest_operand_no_defs_between_wrt_opA (L_Oper *, L_Oper *,
                                                         L_Oper *);
  extern int L_all_src_operand_no_defs_between_wrt_opA (L_Oper *, L_Oper *,
							L_Oper *);

  extern int L_no_uses_between (L_Operand *, L_Oper *, L_Oper *);
  extern int L_no_pred_uses_between (L_Operand *, L_Oper *, L_Oper *);
  extern int L_no_uses_in_range (L_Operand *, L_Oper *, L_Oper *);
  extern int L_all_src_operand_no_uses_between (L_Oper *, L_Oper *, L_Oper *);
  extern int L_all_dest_operand_no_uses_between (L_Oper *, L_Oper *,
                                                 L_Oper *);


#ifdef __cplusplus
}
#endif

/*======================================================================*/
/*
 *      L_Oper boolean predicates
 */
/*======================================================================*/

#define L_is_opcode(opcode, oper) ((oper)==NULL? 0 : ((oper)->opc==opcode))

#define L_ssa_opcode(oper) (!(oper) ? 0 : ((oper)->opc >= Lop_PHI) && \
					  ((oper)->opc <= Lop_GAMMA))

                /* Move opcode tests */
#define INT_MOV_OPCODE(opc) (((opc)==Lop_MOV))
#define FLT_MOV_OPCODE(opc) (((opc)==Lop_MOV_F))
#define DBL_MOV_OPCODE(opc) (((opc)==Lop_MOV_F2))

#define L_int_move_opcode(oper) ((oper)==NULL? 0 : INT_MOV_OPCODE((oper)->opc))
#define L_flt_move_opcode(oper) ((oper)==NULL? 0 : FLT_MOV_OPCODE((oper)->opc))
#define L_dbl_move_opcode(oper) ((oper)==NULL? 0 : DBL_MOV_OPCODE((oper)->opc))
#define L_general_move_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc>=Lop_MOV) && ((oper)->opc<=Lop_MOV_F2)))
#define L_move_opcode(oper) L_general_move_opcode(oper)

                /* Predicate opcode tests (l_compare.h) */

                /* Intrinsic opcode tests -ITI/JWJ 7/99 */
#define L_intrinsic_opcode(oper) ((oper)==NULL ? 0 : \
        (((oper)->opc==Lop_INTRINSIC)))


                /* Arithmetic opcode tests */
#define L_int_add_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_ADD) || ((oper)->opc==Lop_ADD_U)))

#define L_add_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_ADD) || ((oper)->opc==Lop_ADD_U) || \
         ((oper)->opc==Lop_ADD_F) || ((oper)->opc==Lop_ADD_F2)))

#define L_int_sub_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_SUB) || ((oper)->opc==Lop_SUB_U)))

#define L_sub_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_SUB) || ((oper)->opc==Lop_SUB_U) || \
         ((oper)->opc==Lop_SUB_F) || ((oper)->opc==Lop_SUB_F2)))

#define L_int_mul_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MUL) || ((oper)->opc==Lop_MUL_U)))

#define L_int_mul_wide_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MUL_WIDE) || ((oper)->opc==Lop_MUL_WIDE_U)))

#define L_mul_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MUL) || ((oper)->opc==Lop_MUL_U) || \
         ((oper)->opc==Lop_MUL_F) || ((oper)->opc==Lop_MUL_F2)))

#define L_int_div_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_DIV) || ((oper)->opc==Lop_DIV_U)))

#define L_div_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_DIV) || ((oper)->opc==Lop_DIV_U) || \
         ((oper)->opc==Lop_DIV_F) || ((oper)->opc==Lop_DIV_F2)))

#define L_int_rem_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_REM) || ((oper)->opc==Lop_REM_U)))

#define L_rcp_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_RCP_F) || (oper)->opc==Lop_RCP_F2))

#define L_mul_add_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MUL_ADD) || ((oper)->opc==Lop_MUL_ADD_U) || \
         ((oper)->opc==Lop_MUL_ADD_F) || ((oper)->opc==Lop_MUL_ADD_F2)))

#define L_mul_sub_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MUL_SUB) || ((oper)->opc==Lop_MUL_SUB_U) || \
         ((oper)->opc==Lop_MUL_SUB_F) || ((oper)->opc==Lop_MUL_SUB_F2)))

#define L_mul_sub_rev_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MUL_SUB_REV) || \
        ((oper)->opc==Lop_MUL_SUB_REV_U) || \
        ((oper)->opc==Lop_MUL_SUB_REV_F)||((oper)->opc==Lop_MUL_SUB_REV_F2)))

#define L_sqrt_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_SQRT_F) || (oper)->opc==Lop_SQRT_F2))

#define L_max_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MAX) || (oper)->opc==Lop_MAX_F || \
          (oper)->opc==Lop_MAX_F2))

#define L_min_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_MIN) || (oper)->opc==Lop_MIN_F || \
          (oper)->opc==Lop_MIN_F2))

#define L_conversion_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc>=Lop_F2_I) && (oper)->opc<=Lop_F_F2))

#define INT_ARITH_OPCODE(opc) \
        ((((opc)>=Lop_ADD) && ((opc)<=Lop_ASR)) || \
         ((opc)==Lop_RCMP) || ((opc)==Lop_RCMP_F) || \
         ((opc)==Lop_F2_I) || ((opc)==Lop_F_I) || \
         ((opc)==Lop_MAX) || ((opc)==Lop_MIN) || \
         (((opc)>=Lop_EXTRACT_C) && ((opc)<=Lop_DEPOSIT)) || \
         ((opc)==Lop_MEM_COPY_SETUP) || \
         (((opc)>=Lop_SXT_C) && ((opc)<=Lop_ZXT_I)) || \
         (((opc)>=Lop_ADD_CARRY) && ((opc)<=Lop_MUL_WIDE_U)))

#define FLT_ARITH_OPCODE(opc) \
        ((((opc)>=Lop_ADD_F) && ((opc)<=Lop_MUL_SUB_REV_F)) || \
         ((opc)==Lop_I_F) || ((opc)==Lop_F2_F) || ((opc)==Lop_SQRT_F) || \
         ((opc)==Lop_RCP_F) || ((opc)==Lop_MAX_F) || ((opc)==Lop_MIN_F))
#define DBL_ARITH_OPCODE(opc) \
        ((((opc)>=Lop_ADD_F2) && ((opc)<=Lop_MUL_SUB_REV_F2)) || \
         ((opc)==Lop_I_F2) || ((opc)==Lop_F_F2) || ((opc)==Lop_SQRT_F2) || \
         ((opc)==Lop_RCP_F2) || ((opc)==Lop_MAX_F2) || ((opc)==Lop_MIN_F2))

#define L_int_arithmetic_opcode(oper) ((oper)==NULL? 0 : \
        INT_ARITH_OPCODE((oper)->opc))
#define L_flt_arithmetic_opcode(oper) ((oper)==NULL? 0 : \
        FLT_ARITH_OPCODE((oper)->opc))
#define L_dbl_arithmetic_opcode(oper) ((oper)==NULL? 0 : \
        DBL_ARITH_OPCODE((oper)->opc))
#define L_general_arithmetic_opcode(oper) ((oper)==NULL? 0 : \
        (INT_ARITH_OPCODE((oper)->opc) ||\
         FLT_ARITH_OPCODE((oper)->opc) ||\
         DBL_ARITH_OPCODE((oper)->opc)))

                /* Logic opcode tests */
#define L_shift_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc>=Lop_LSL) && ((oper)->opc<=Lop_ASR)))
#define L_logic_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc>=Lop_OR) && ((oper)->opc<=Lop_AND_COMPL)))

                /* Comparison opcode tests (l_compare.h) */

                /* Predicate ompare opcode tests (l_compare.h) */

                /* Branch tests (l_compare.h) */

                /* Subroutine opcode tests */
#define L_subroutine_call_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_JSR) || ((oper)->opc==Lop_JSR_FS)))
#define L_subroutine_return_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc == Lop_RTS) || ((oper)->opc==Lop_RTS_FS)))
#define L_general_subroutine_call_opcode(oper) ((oper)==NULL? 0 : \
        M_subroutine_call((oper)->opc))

                /* Load opcode tests */
#define L_int_load_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ty_LD_INT_START) && \
         ((oper)->opc <= Lop_ty_LD_INT_END)))
#define L_flt_load_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_LD_F))
#define L_dbl_load_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_LD_F2))
#define L_load_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ty_LD_START) && \
         ((oper)->opc <= Lop_ty_LD_END)))

#define L_load_chk_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ty_LD_CHK_START) && \
         ((oper)->opc <= Lop_ty_LD_CHK_END)))

                /* Preincrement Load opcode tests */
#define L_int_preincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_LD_PRE_UC) && ((oper)->opc<=Lop_LD_PRE_Q)))
#define L_flt_preincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_LD_PRE_F))
#define L_dbl_preincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_LD_PRE_F2))
#define L_preincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_LD_PRE_UC) && ((oper)->opc<=Lop_LD_PRE_F2)))

                /* Postincrement Load opcode tests */
#define L_int_postincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_LD_POST_UC) && ((oper)->opc<=Lop_LD_POST_Q)))
#define L_flt_postincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_LD_POST_F))
#define L_dbl_postincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_LD_POST_F2))
#define L_postincrement_load_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_LD_POST_UC) && ((oper)->opc<=Lop_LD_POST_F2)))

/* SLARSEN: Account for vector loads */
#define L_general_load_opcode(oper) ((oper)==NULL? 0 : \
        ((((oper)->opc >= Lop_ty_GLD_START) && \
          ((oper)->opc <= Lop_ty_GLD_END)) || \
         (((oper)->opc >= Lop_ty_LD_CHK_START) && \
          ((oper)->opc <= Lop_ty_LD_CHK_END)) || \
	 (((oper)->opc >= Lop_VLD_UC) && ((oper)->opc<=Lop_VLD_F2)) || \
	 (((oper)->opc >= Lop_VLDE_UC) && ((oper)->opc<=Lop_VLDE_F2)) || \
         ((oper)->opc==Lop_PRED_LD) || ((oper)->opc==Lop_PRED_LD_BLK)))

/* SLARSEN: Used in elcor translation (el_operand_position) */
#define L_vector_load_opcode(oper) ((oper)==NULL? 0 : \
	((((oper)->opc >= Lop_VLD_UC) && ((oper)->opc<=Lop_VLD_F2)) || \
	 (((oper)->opc >= Lop_VLDE_UC) && ((oper)->opc<=Lop_VLDE_F2))))

                /* Store opcode tests */
#define L_int_store_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ST_C) && ((oper)->opc<=Lop_ST_Q)))
#define L_flt_store_opcode(oper) ((oper)==NULL? 0 : ((oper)->opc == Lop_ST_F))
#define L_dbl_store_opcode(oper) ((oper)==NULL? 0 : ((oper)->opc == Lop_ST_F2))
#define L_store_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ST_C) && ((oper)->opc<=Lop_ST_F2)))

                /* Preincrement Store opcode tests */
#define L_int_preincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ST_PRE_C) && ((oper)->opc<=Lop_ST_PRE_Q)))
#define L_flt_preincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_ST_PRE_F))
#define L_dbl_preincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_ST_PRE_F2))
#define L_preincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ST_PRE_C) && ((oper)->opc<=Lop_ST_PRE_F2)))

                /* Postincrement Store opcode tests */
#define L_int_postincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ST_POST_C) && ((oper)->opc<=Lop_ST_POST_Q)))
#define L_flt_postincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_ST_POST_F))
#define L_dbl_postincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        ((oper)->opc == Lop_ST_POST_F2))
#define L_postincrement_store_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc >= Lop_ST_POST_C) && ((oper)->opc<=Lop_ST_POST_F2)))

/* SLARSEN: Account for vector stores */
#define L_general_store_opcode(oper) ((oper)==NULL? 0 : \
	((((oper)->opc >= Lop_ST_C) && ((oper)->opc<=Lop_ST_POST_F2)) || \
	 (((oper)->opc >= Lop_VST_C) && ((oper)->opc<=Lop_VST_F2)) || \
	 (((oper)->opc >= Lop_VSTE_C) && ((oper)->opc<=Lop_VSTE_F2)) || \
	 ((oper)->opc==Lop_PRED_ST) || ((oper)->opc==Lop_PRED_ST_BLK)))

/* SLARSEN: Used in elcor translation (el_operand_position) */
#define L_vector_store_opcode(oper) ((oper)==NULL? 0 : \
	 ((((oper)->opc >= Lop_VST_C) && ((oper)->opc<=Lop_VST_F2)) || \
          (((oper)->opc >= Lop_VSTE_C) && ((oper)->opc<=Lop_VSTE_F2))))

                /* Conditional move opcode tests */
#define L_cmov_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_CMOV) || ((oper)->opc==Lop_CMOV_COM) || \
         ((oper)->opc==Lop_CMOV_F) || ((oper)->opc==Lop_CMOV_COM_F) || \
         ((oper)->opc==Lop_CMOV_F2) || ((oper)->opc==Lop_CMOV_COM_F2)))

                /* Select opcode tests */
#define L_select_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_SELECT) || ((oper)->opc==Lop_SELECT_F) || \
         ((oper)->opc==Lop_SELECT_F2)))

                /* Pbr opcode tests */
#define L_pbr_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_PBR)))

           /* Sign and zero extention instructions */
#define L_sign_or_zero_extend_opcode(oper) \
           ((oper)==NULL? 0 : (((oper)->opc >= Lop_SXT_C) \
                              && ((oper)->opc <= Lop_ZXT_I)))
#define L_sign_extend_opcode(oper) \
           ((oper)==NULL? 0 : (((oper)->opc >= Lop_SXT_C) \
                              && ((oper)->opc <= Lop_SXT_I)))
#define L_zero_extend_opcode(oper) \
           ((oper)==NULL? 0 : (((oper)->opc >= Lop_ZXT_C) \
                              && ((oper)->opc <= Lop_ZXT_I)))

               /* Extract and Deposit instructions */
#define L_bit_extract_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_BIT_EXTRACT)))
#define L_bit_deposit_opcode(oper) ((oper)==NULL? 0 : \
        (((oper)->opc==Lop_BIT_DEPOSIT)))

#define L_check_opcode(oper) ((oper)==NULL? 0 :\
        (((oper)->opc==Lop_CHECK) || ((oper)->opc==Lop_CHECK_ALAT)))
#define L_check_branch_opcode(oper) ((oper)==NULL? 0 :\
        ((((oper)->opc==Lop_CHECK) || ((oper)->opc==Lop_CHECK_ALAT))\
        &&((oper)->src[1]!=NULL)))
                /* Predicate define opcode tests (l_compare.h) */

#ifdef __cplusplus
extern "C"
{
#endif

#define USE_FS 0
#define IGNORE_FS 1

  extern int L_unsigned_int_opcode (L_Oper *);
  extern int L_unsigned_int_comparative_opcode (L_Oper *);
  extern int L_int_opcode (L_Oper *);
  extern int L_flt_opcode (L_Oper *);
  extern int L_dbl_opcode (L_Oper *);
  extern int L_sync_opcode (L_Oper *);
  extern int L_same_opcode (L_Oper *, L_Oper *);
  extern int L_equivalent_opcode (L_Oper *, L_Oper *);
  extern int L_same_operation (L_Oper *, L_Oper *, int);
  extern int L_same_computation (L_Oper *, L_Oper *);
  extern int L_commutative_opcode (L_Oper *);

  extern int L_safe_to_delete_opcode (L_Oper *);
  extern int L_can_move_opcode (L_Oper *);
  extern int L_pointer_store (L_Oper *);
  extern int L_no_danger (int, int, int, L_Oper *, L_Oper *);
  extern int L_no_danger_by_sync_arcs (int, int, int, L_Oper *, L_Oper *);
  extern int L_no_danger_in_cb (struct L_Cb *);
  extern int L_no_br_between (L_Oper *, L_Oper *);
  extern int L_no_jsr_between (L_Oper *, L_Oper *);
  extern int L_no_control_oper_between (L_Oper *, L_Oper *);
  extern int L_can_change_dest_operand (L_Oper *, L_Operand *);
  extern int L_can_change_src_operand (L_Oper *, L_Operand *);

/*======================================================================*/
/*
 *      L_Cb boolean predicates
 */
/*======================================================================*/

  extern int L_single_predecessor_cb (L_Cb *);
  extern int L_has_fallthru_to_next_cb (L_Cb *);

/*======================================================================*/
/*
 *      L_Data predicates
 */
/*======================================================================*/

  extern int L_data_token_type (int token);

/*======================================================================*/
/*
 *      L_Attr functions
 */
/*======================================================================*/

#define L_get_int_attr_field( attr, field_n ) \
                                       ((int)(attr)->field[(field_n)]->value.i)

  extern L_Attr *L_copy_attr_element (L_Attr *);
  extern L_Attr *L_copy_attr (L_Attr *);
  extern L_Attr *L_merge_attr_lists (L_Attr *, L_Attr *);
  extern L_Operand *L_find_attr_field (L_Attr * attr, int type);
  extern L_Operand *L_find_cb_attr_field (L_Attr * attr, L_Cb * cb);
  extern L_Operand *L_find_int_attr_field (L_Attr * attr, int val);
  extern L_Operand *L_find_float_attr_field (L_Attr * attr, float val);
  extern L_Operand *L_find_double_attr_field (L_Attr * attr, double val);
  extern L_Operand *L_find_string_attr_field (L_Attr * attr, char *str);
  extern L_Operand *L_find_macro_attr_field (L_Attr * attr, int mac,
                                             int ctype, int ptype);
  extern L_Operand *L_find_register_attr_field (L_Attr * attr, int index,
                                                int ctype, int ptype);
  extern L_Operand *L_find_label_attr_field (L_Attr * attr, char *label);
  extern L_Operand *L_find_rregister_attr_field (L_Attr * attr, int index,
                                                 int ctype, int ptype);
  extern L_Operand *L_find_evr_attr_field (L_Attr * attr, int index,
                                           int omega, int ctype, int ptype);

/*======================================================================*/
/*
 *      L_Sync functions
 */
/*======================================================================*/

  extern L_Sync *L_create_new_sync (int num, char prob, char freq,
                                    int dist, int flags, int prof);
  extern void L_insert_all_syncs_in_dep_opers (L_Oper * oper);

/*======================================================================*/
/*
 *      L_AccSpec functions
 */
/*======================================================================*/

  extern L_AccSpec *L_new_mem_acc_spec (int is_def, int id, int version, int offset, 
					int size);
  extern L_AccSpec *L_copy_mem_acc_spec (L_AccSpec *mas);
  extern L_AccSpec *L_copy_mem_acc_spec_list (L_AccSpec *mas);
  extern L_AccSpec *L_copy_mem_acc_spec_list_as_def (L_AccSpec *mas);
  extern L_AccSpec *L_copy_mem_acc_spec_list_as_use (L_AccSpec *mas);
  extern L_AccSpec *L_delete_mem_acc_spec (L_AccSpec *mas);
  extern L_AccSpec *L_delete_mem_acc_spec_list (L_AccSpec *mas);
  extern void L_merge_acc_spec_list (L_Oper *op_to, L_Oper *op_fr);

/*======================================================================*/
/*
 *      L_Flow functions
 */
/*======================================================================*/

  extern L_Flow *L_find_flow (L_Flow *, int, L_Cb *, L_Cb *);
  extern L_Flow *L_find_flow_with_dst_cb (L_Flow *, L_Cb *);
  extern L_Flow *L_find_flow_with_src_cb (L_Flow *, L_Cb *);
  extern L_Flow *L_find_matching_flow (L_Flow *, L_Flow *);
  extern L_Flow *L_find_complement_flow (L_Flow * list, L_Flow * flow);
  extern L_Flow *L_find_last_flow (L_Flow *);
  extern L_Flow *L_find_second_to_last_flow (L_Flow *);
  extern L_Flow *L_find_max_weight_flow (L_Flow *);
  extern L_Flow *L_find_flow_for_branch (L_Cb *, L_Oper *);
  extern L_Oper *L_find_branch_for_flow (L_Cb *, L_Flow *);
  extern L_Flow *L_copy_flow (L_Flow *);
  extern L_Flow *L_copy_single_flow (L_Flow *);
  extern void L_change_flow (L_Flow *, int, L_Cb *, L_Cb *, double);
  extern void L_change_dest (L_Flow *, L_Cb *, L_Cb *);
  extern void L_change_src (L_Flow *, L_Cb *, L_Cb *);
  extern void L_change_dest_cb (L_Cb *, L_Cb *, L_Cb *);
  extern void L_change_src_cb (L_Cb *, L_Cb *, L_Cb *);
  extern void L_scale_flow_weights (L_Flow *, double);
  extern int L_dest_flow_out_of_order (L_Cb *);
  extern void L_reorder_dest_flows (L_Cb *);
  extern double L_find_taken_prob (L_Cb *cb, L_Oper *br);

/*======================================================================*/
/*
 *      L_Operand functions
 */
/*======================================================================*/

  extern L_Operand *L_copy_operand (L_Operand *);

/*======================================================================*/
/*
 *      L_Expression functions
 */
/*======================================================================*/

  extern int L_generate_expression_token_from_oper (L_Oper *);
  extern int L_generate_assignment_token_from_oper (L_Oper *, int);
  extern int L_generate_expression_token_different_opcode (L_Oper *, int);
  extern L_Oper *L_generate_oper_from_expression_index (int);
  extern L_Oper *L_generate_oper_from_assignment_index (int, int);
  extern L_Oper *L_generate_complement_load_oper_from_assignment_index (int);
  extern L_Operand *L_create_operand_for_expression_index (int);
  extern L_Operand *L_create_pred_for_assignment_index (int);
  extern void L_clear_expression_reg_ids (L_Func *);
  extern Set L_get_expression_subset (int);

/*======================================================================*/
/*
 *      L_Oper functions
 */
/*======================================================================*/

  extern L_Oper *L_create_new_op (int);
  extern L_Oper *L_create_new_op_with_id (int, int);
  extern L_Oper *L_create_new_op_using (int, L_Oper *);
  extern L_Oper *L_create_new_similar_op (int, L_Oper *);
  extern L_Oper *L_copy_operation (L_Oper *);
  extern void L_move_op_to_end_of_block (L_Cb *, L_Cb *, L_Oper *);
  extern void L_move_op_to_start_of_block (L_Cb *, L_Cb *, L_Oper *);
  extern void L_change_cond_br (L_Oper *, ITuint8, L_Cb *);
  extern L_Cb *L_find_branch_dest (L_Oper *);
  extern int L_change_branch_dest (L_Oper *, L_Cb *, L_Cb *);
  extern void L_change_all_branch_dest (L_Cb *, L_Cb *, L_Cb *);
  extern L_Oper *L_next_def (L_Operand *, L_Oper *);
  extern L_Oper *L_prev_def (L_Operand *, L_Oper *);
  extern L_Oper *L_next_use (L_Operand *, L_Oper *);
  extern L_Oper *L_prev_use (L_Operand *, L_Oper *);
  extern L_Oper *L_find_def (L_Cb *, L_Operand *);
  extern L_Oper *L_find_use (L_Cb *, L_Operand *, L_Oper *);
  extern L_Oper *L_prev_tm_memory_def (L_Oper *, int);
  extern void L_nullify_operation (L_Oper *);
  extern void L_change_opcode (L_Oper *, int);
  extern int L_fs_branch_opcode (L_Oper *);
  extern int L_nonfs_branch_opcode (L_Oper *);
  extern int L_base_memory_opcode (L_Oper *);
  extern void L_compute_oper_weight (L_Func *, int, int);
  extern void L_compute_oper_exec_weight (L_Func *, int, int);
  extern void L_branch_prediction (L_Func *);
  extern int L_find_control_op_index (L_Oper *);
  extern int L_opcode_ctype (L_Oper *);
  extern int L_store_ctype (L_Oper *);
  extern int L_num_src_operand (L_Oper *);
  extern int L_num_dest_operand (L_Oper *);
  extern void L_clear_all_reserved_oper_flags (L_Func *);

/*======================================================================*/
/*
 *      L_Cb functions
 */
/*======================================================================*/

  extern L_Cb *L_fall_thru_path (L_Cb *);
  extern L_Cb *L_create_cb (double);
  extern L_Cb *L_create_cb_at_fall_thru_path (L_Cb *, int);
  extern void L_copy_block_contents (L_Cb *, L_Cb *);
  extern int L_cb_size (L_Cb *);
  extern int L_num_dst_cb (L_Cb *);
  extern int L_num_src_cb (L_Cb *);
  extern L_Oper *L_find_last_branch (L_Cb *);
  extern void L_clear_all_reserved_cb_flags (L_Func *);
  extern void L_scale_cb_weight (L_Cb *cb, double scale_factor);

/*======================================================================*/
/*
 *      L_Loop functions
 */
/*======================================================================*/

  extern L_Loop *L_find_loop (L_Func * fn, int id);
  extern void L_sort_loops (int *loop_array, int num_loops);

/*======================================================================*/
/*
 *      L_Inner_Loop functions
 */
/*======================================================================*/

  extern L_Inner_Loop *L_find_inner_loop (L_Func * fn, int id);

/*======================================================================*/
/*
 *      L_Datalist functions
 */
/*======================================================================*/

  extern void L_merge_datalists (L_Datalist * l1, L_Datalist * l2);


/*======================================================================*/
/*
 *      L_Func functions
 */
/*======================================================================*/

  extern void L_scale_fn_weight (L_Func * fn, double scale_factor);
/* Special macros to work with registers of different ctypes */
  extern int L_is_integer_reg (L_Operand * operand);
  extern int L_is_float_reg (L_Operand * operand);
  extern int L_is_branch_reg (L_Operand * operand);
  extern int L_is_predicate_reg (L_Operand * operand);

/* Special macros to work with gp labels and strings */

/* operand->type = L_OPERAND_STRING  operand->ctype == L_CTYPE_GLOBAL_GP */
  extern void L_assign_type_GLOBAL_GP_string (L_Operand * operand);
  extern void L_assign_type_LOCAL_GP_string (L_Operand * operand);
/* operand->type = L_OPERAND_LABEL  operand->ctype == L_CTYPE_GLOBAL_GP */
  extern void L_assign_type_GLOBAL_GP_label (L_Operand * operand);
  extern void L_assign_type_LOCAL_GP_label (L_Operand * operand);

  extern int L_is_GLOBAL_GP_string (L_Operand * operand);
  extern int L_is_GLOBAL_GP_label (L_Operand * operand);
  extern int L_is_LOCAL_GP_label (L_Operand * operand);

  extern int L_is_ctype_LOCAL_ABS_direct (ITuint8 ctype);
  extern int L_is_ctype_LOCAL_GP_direct (ITuint8 ctype);
  extern int L_is_ctype_GLOBAL_ABS_direct (ITuint8 ctype);
  extern int L_is_ctype_GLOBAL_GP_direct (ITuint8 ctype);

  extern void L_initialize_function_live_ins (L_Func * fn);
#ifdef __cplusplus
}
#endif

#endif
