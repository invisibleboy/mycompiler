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

/*****************************************************************************\
 *
 *  File:  m_x86.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 		generator.
 *
 *  Creation Date : May, 1993
 *
 *  Author:  Dave Gallagher, Wen-mei Hwu
 *
 *  Revisions:
 *
 *
\*****************************************************************************/

/*****************************************************************************\
 * NOTICE OF CONVENTION                                                      *
 * ------------------------------------------------------------------------- *
 * Mspec links to Pcode, Hcode, and Lcode modules.  In order to allow this   *
 * to take place without requiring front-end modules to link to liblcode.a,  *
 * Mspec code is divided into two classes as follows:                        *
 *  - mi_*.c must not depend on linkage to liblcode.a                        *
 *  - ml_*.c may depend on linkage to liblcode.a                             *
\*****************************************************************************/

#ifndef M_X86_H
#define M_X86_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <machine/m_spec.h>

/*
 * Declarations for processor models
 */
enum
{
  M_386 = 0,
  M_486,
  M_PENTIUM,
  M_KRYPTON
};

enum
{
  LX86_MAC_ZERO = 100,		/* zero (gr0)                                */
  LX86_MAC_TEMPREG,		/* tempreg (gr1) used in prologue/epilogue   */
  LX86_MAC_RETADDR,		/* return address register (gr2)             */
  LX86_MAC_MILLI_RET_VALUE,	/* millicode return value register (gr29)    */
  LX86_MAC_MILLI_RETADDR,	/* millicode return address register (gr31)  */
  LX86_MAC_LEAF,		/* 1 if leaf function, 0 if not leaf function*/
  LX86_MAC_ALLOC,		/* total alloc space requirements            */
  LX86_MAC_CALLEE_I,
  LX86_MAC_CALLEE_F,
  LX86_MAC_CONV_LOC,		/* 1 if need conversion space, 0 else        */
  LX86_MAC_CONV_OFF,		/* stack location reserved for conversion of */
  /* f(f2) to i and i to f(f2)                 */
  LX86_MAC_SAR,			/* shift amount register                     */
  LX86_MAC_DP,			/* data pointer gr27                         */
  LX86_MAC_FZERO,
  LX86_MAC_TRUE_SP,
  LX86_MAC_DYNCALL,		/* parameter for dynamic function call(gr22) */
  LX86_MAC_FLOAT_CBIT,		/* floating point nullification condition bit*/
  LX86_MAC_SWAP_PTR,


/**********************************************************************
    X86 Macros
***********************************************************************/

  LX86_MAC_STACK,		/* indicates to phase 3 this is a push/pop   */
  LX86_MAC_FPSTACK,		/* floating point stack operand              */
  LX86_MAC_FLAGS,		/* indicates the oper sets CPU flags         */
  LX86_MAC_EAX,			/* EAX register                              */
  LX86_MAC_EBX,			/* EBX register                              */
  LX86_MAC_ECX,			/* ECX register                              */
  LX86_MAC_EDX,			/* EDX register                              */
  LX86_MAC_ESI,			/* ESI register                              */
  LX86_MAC_EDI,			/* EDI register                              */
  LX86_MAC_EBP,			/* EBP register                              */
  LX86_MAC_ESP,			/* ESP register                              */
  LX86_MAC_ST0,			/* FP Stack                                  */
  LX86_MAC_ST1,
  LX86_MAC_ST2,
  LX86_MAC_ST3,
  LX86_MAC_ST4,
  LX86_MAC_ST5,
  LX86_MAC_ST6,
  LX86_MAC_ST7,
  LX86_MAC_RETURN_TYPE
};

#define MDES_OPERAND_flag	100
#define MDES_OPERAND_istk	101
#define MDES_OPERAND_fstk	102

#define MDES_OPERAND_FLAG	103
#define MDES_OPERAND_ISTK	104
#define MDES_OPERAND_FSTK	105
#define MDES_OPERAND_CON	106
#define MDES_OPERAND_SOME	107
#define MDES_OPERAND_ANY	108
#define MDES_OPERAND_REG_STK	109

/*
 * Lx86 processor-specific opcodes
 * ----------------------------------------------------------------------
 * (moved here from Lcode/codegen/lx86_opc.h)
 */


#define LX86op_FPUSH_I		700
#define LX86op_FPOP_I		701
#define LX86op_PUSH		702
#define LX86op_POP		703
#define LX86op_NEGATE		704
#define LX86op_FXCH		705
#define LX86op_SUB_F		706
#define LX86op_SUB_F2		707
#define LX86op_SUBR_F		708
#define LX86op_SUBR_F2		709
#define LX86op_DIV_F		710
#define LX86op_DIV_F2		711
#define LX86op_DIVR_F		712
#define LX86op_DIVR_F2		713
#define LX86op_FSTCW		714
#define LX86op_FLDCW		715

#define LX86op_LEA		750	/* use lea for this shift */
#define LX86op_TEST		751	/* use test for this compare */
#define LX86op_INC		752	/* use increment instead of add, 
					   subtract */
#define LX86op_DEC		753	/* use decrement instead of add, 
					   subtract */

#define LX86op_EQ_FMEM		756
#define LX86op_EQ_F2MEM		757
#define LX86op_NE_FMEM		758
#define LX86op_NE_F2MEM		759
#define LX86op_GT_FMEM		760
#define LX86op_GT_F2MEM		761
#define LX86op_GE_FMEM		762
#define LX86op_GE_F2MEM		763
#define LX86op_LT_FMEM		764
#define LX86op_LT_F2MEM		765
#define LX86op_LE_FMEM		766
#define LX86op_LE_F2MEM		767
#define LX86op_ADD_FMEM		768
#define LX86op_ADD_F2MEM	769
#define LX86op_SUB_FMEM		770
#define LX86op_SUB_F2MEM	771
#define LX86op_SUBR_FMEM	772
#define LX86op_SUBR_F2MEM	773
#define LX86op_MUL_FMEM		774
#define LX86op_MUL_F2MEM	775
#define LX86op_DIV_FMEM		776
#define LX86op_DIV_F2MEM	777
#define LX86op_DIVR_FMEM	778
#define LX86op_DIVR_F2MEM	779

#define LX86op_CISC_TO_REG		780
#define LX86op_CISC_TO_REG_TEST		783

#define LX86op_CISC_BRANCH		785
#define LX86op_CISC_TO_MEM		786
#define LX86op_CISC_TO_MEM_INC		787
#define LX86op_CISC_TO_MEM_DEC		788
#define LX86op_CISC_TO_MEM_NEGATE	789

#define LX86op_CISC_TO_REG_CMP		790

#define LX86op_INT_LOAD			791
#define LX86op_INT_STORE		792
#define LX86op_FLOAT_LOAD		793
#define LX86op_FLOAT_STORE		794
#define LX86op_TRACE_JSR		800

#define LX86op_INT_SUB_WITH_BORROW	801

#define LX86op_MOVS			802

/* The following proc_opcs are intended to be hooked to XOR/LSR/LSL/LSR/LSL,
   but in order to be compatible with LX86op_CISC_TO_MEM/LX86op_CISC_TO_REG
   which take the proc_opc position already, they will appear in attribute 
   as xchg/ror/rol/rcr/rcl instead */

/*#define LX86op_XCHG			803 */
/*#define LX86op_ROR			804 *//* rotate right */
/*#define LX86op_ROL			805 *//* rotate left */
/*#define LX86op_RCR			804 *//* rotate right, with carry */
/*#define LX86op_RCL			805 *//* rotate left, with carry */

/* LCH: add one more proc_opc for K5 */
#define LX86op_JCXZ		806

/* ADA 11/18/95:  The following proc_opcs are defined for mdes. */
#define LX86op_SAHF		900
#define LX86op_CDQ		901
#define LX86op_FSTSW		902
#define LX86op_FLDZ		903
#define LX86op_FLD1		904
#define LX86op_FLD_ST		905
#define LX86op_FSTP		906

/* DML 12/16/95 */
#define LX86op_MOVXL		907 /* load with zero or sign extension */
				    /* movzbl, movzwl, movsbl, movswl */
#define LX86op_ALU16		908 /* 16-bit reg-to-reg alu operations */
#define LX86op_SHIFT16		909 /* 16-bit reg-to-reg shift operations */
#define LX86op_CMP16		910 /* 16-bit reg-to-reg compare operations */
#define LX86op_MUL16		911 /* 16-bit reg-to-reg multiply operations */
#define LX86op_DIV16		912 /* 16-bit reg-to-reg divide operations */

#define LX86op_ADC		920 /* add with carry */
#define LX86op_SBB		921 /* sub with borrow */
#define LX86op_SHLD		922 /* double-precision shift left */
#define LX86op_SHRD		923 /* double-precision shift right */
#define LX86op_LAHF		924

/* MCM 9/11/96 The following proc_opcs are defined for bundled instructions. */
#define LX86op_SAHF_BRANCH	950	/* combined sahf and int cond branch */
#define LX86op_SAHF_COMPARE	951	/* combined sahf and int compare */

#define LX86op_FP2INT_BRANCH   1000


/* Name all the proc_opcs in order for Lmix tools to work -JCG 11/21/95 */
#define X86opcode_FPUSH_I		"LX86op_FPUSH_I"
#define X86opcode_FPOP_I		"LX86op_FPOP_I"
#define X86opcode_PUSH			"LX86op_PUSH"
#define X86opcode_POP			"LX86op_POP"
#define X86opcode_NEGATE		"LX86op_NEGATE"
#define X86opcode_FXCH			"LX86op_FXCH"
#define X86opcode_SUB_F			"LX86op_SUB_F"
#define X86opcode_SUB_F2		"LX86op_SUB_F2"
#define X86opcode_SUBR_F		"LX86op_SUBR_F"
#define X86opcode_SUBR_F2		"LX86op_SUBR_F2"
#define X86opcode_DIV_F			"LX86op_DIV_F"
#define X86opcode_DIV_F2		"LX86op_DIV_F2"
#define X86opcode_DIVR_F		"LX86op_DIVR_F"
#define X86opcode_DIVR_F2		"LX86op_DIVR_F2"
#define X86opcode_FSTCW			"LX86op_FSTCW"
#define X86opcode_FLDCW			"LX86op_FLDCW"
#define X86opcode_LEA			"LX86op_LEA"
#define X86opcode_TEST			"LX86op_TEST"
#define X86opcode_INC			"LX86op_INC"
#define X86opcode_DEC			"LX86op_DEC"
#define X86opcode_EQ_FMEM		"LX86op_EQ_FMEM"
#define X86opcode_EQ_F2MEM		"LX86op_EQ_F2MEM"
#define X86opcode_NE_FMEM		"LX86op_NE_FMEM"
#define X86opcode_NE_F2MEM		"LX86op_NE_F2MEM"
#define X86opcode_GT_FMEM		"LX86op_GT_FMEM"
#define X86opcode_GT_F2MEM		"LX86op_GT_F2MEM"
#define X86opcode_GE_FMEM		"LX86op_GE_FMEM"
#define X86opcode_GE_F2MEM		"LX86op_GE_F2MEM"
#define X86opcode_LT_FMEM		"LX86op_LT_FMEM"
#define X86opcode_LT_F2MEM		"LX86op_LT_F2MEM"
#define X86opcode_LE_FMEM		"LX86op_LE_FMEM"
#define X86opcode_LE_F2MEM		"LX86op_LE_F2MEM"
#define X86opcode_ADD_FMEM		"LX86op_ADD_FMEM"
#define X86opcode_ADD_F2MEM		"LX86op_ADD_F2MEM"
#define X86opcode_SUB_FMEM		"LX86op_SUB_FMEM"
#define X86opcode_SUB_F2MEM		"LX86op_SUB_F2MEM"
#define X86opcode_SUBR_FMEM		"LX86op_SUBR_FMEM"
#define X86opcode_SUBR_F2MEM		"LX86op_SUBR_F2MEM"
#define X86opcode_MUL_FMEM		"LX86op_MUL_FMEM"
#define X86opcode_MUL_F2MEM		"LX86op_MUL_F2MEM"
#define X86opcode_DIV_FMEM		"LX86op_DIV_FMEM"
#define X86opcode_DIV_F2MEM		"LX86op_DIV_F2MEM"
#define X86opcode_DIVR_FMEM		"LX86op_DIVR_FMEM"
#define X86opcode_DIVR_F2MEM		"LX86op_DIVR_F2MEM"
#define X86opcode_CISC_TO_REG		"LX86op_CISC_TO_REG"
#define X86opcode_CISC_TO_REG_TEST	"LX86op_CISC_TO_REG_TEST"
#define X86opcode_CISC_BRANCH		"LX86op_CISC_BRANCH"
#define X86opcode_CISC_TO_MEM		"LX86op_CISC_TO_MEM"
#define X86opcode_CISC_TO_MEM_INC	"LX86op_CISC_TO_MEM_INC"
#define X86opcode_CISC_TO_MEM_DEC	"LX86op_CISC_TO_MEM_DEC"
#define X86opcode_CISC_TO_MEM_NEGATE	"LX86op_CISC_TO_MEM_NEGATE"
#define X86opcode_CISC_TO_REG_CMP	"LX86op_CISC_TO_REG_CMP"
#define X86opcode_MOVXL			"LX86op_MOVXL"
#define X86opcode_INT_LOAD		"LX86op_INT_LOAD"
#define X86opcode_INT_STORE		"LX86op_INT_STORE"
#define X86opcode_FLOAT_LOAD		"LX86op_FLOAT_LOAD"
#define X86opcode_FLOAT_STORE		"LX86op_FLOAT_STORE"
#define X86opcode_TRACE_JSR		"LX86op_TRACE_JSR"
#define X86opcode_INT_SUB_WITH_BORROW	"LX86op_INT_SUB_WITH_BORROW"
#define X86opcode_MOVS			"LX86op_MOVS"
#define X86opcode_SAHF			"LX86op_SAHF"
#define X86opcode_CDQ			"LX86op_CDQ"
#define X86opcode_FSTSW			"LX86op_FSTSW"
#define X86opcode_FLDZ			"LX86op_FLDZ"
#define X86opcode_FLD1			"LX86op_FLD1"
#define X86opcode_FLD_ST		"LX86op_FLD_ST"
#define X86opcode_FSTP			"LX86op_FSTP"
#define X86opcode_FP2INT_BRANCH		"LX86op_FP2INT_BRANCH"


#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_x86_type_size (int mtype);
  extern int M_x86_type_align (int mtype);
  extern void M_x86_void (M_Type type);
  extern void M_x86_bit_long (M_Type type, int n);
  extern void M_x86_bit_int (M_Type type, int n);
  extern void M_x86_bit_short (M_Type type, int n);
  extern void M_x86_bit_char (M_Type type, int n);
  extern void M_x86_char (M_Type type, int unsign);
  extern void M_x86_short (M_Type type, int unsign);
  extern void M_x86_int (M_Type type, int unsign);
  extern void M_x86_long (M_Type type, int unsign);
  extern void M_x86_float (M_Type type, int unsign);
  extern void M_x86_double (M_Type type, int unsign);
  extern void M_x86_pointer (M_Type type);
  extern int M_x86_eval_type (M_Type type, M_Type ntype);
  extern int M_x86_eval_type2 (M_Type type, M_Type ntype);
  extern int M_x86_call_type (M_Type type, M_Type ntype);
  extern int M_x86_call_type2 (M_Type type, M_Type ntype);
  extern void M_x86_array_layout (M_Type type, int *offset);
  extern int M_x86_array_align (M_Type type);
  extern int M_x86_array_size (M_Type type, int dim);
  extern void M_x86_union_layout (int n, _M_Type * type, int *offset,
				  int *bit_offset);
  extern int M_x86_union_align (int n, _M_Type * type);
  extern int M_x86_union_size (int n, _M_Type * type);
  extern void M_x86_struct_layout (int n, _M_Type * type, int *base,
				   int *bit_offset);
  extern int M_x86_struct_align (int n, _M_Type * type);
  extern int M_x86_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_x86_layout_fnvar (List param_list, char **base_macro,
				 int *pcount, int purpose);
  extern int M_x86_fnvar_layout (int n, _M_Type * type, long int *offset,
				 int *mode, int *reg, int *paddr,
				 char **macro, int *su_sreg, int *su_ereg,
				 int *pcount, int is_st, int purpose);
  extern int M_x86_lvar_layout (int n, _M_Type * type, long int *offset,
				char **base_macro);
  extern int M_x86_no_short_int (void);
  extern int M_x86_layout_order (void);
  extern void M_x86_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_x86_is_cb_label (char *label, char *fn, int *cb);
  extern void M_x86_jumptbl_label_name (char *fn, int tbl_id, char *line,
					int len);
  extern int M_x86_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_x86_structure_pointer (int purpose);
  extern int M_x86_return_register (int type, int purpose);
  extern char *M_x86_fn_label_name (char *label,
				    int (*is_func) (char *is_func_label));
  extern char *M_x86_fn_name_from_label (char *label);
  extern void M_set_model_x86 (char *model_name);
  extern int M_x86_fragile_macro (int macro_value);
  extern int M_x86_subroutine_call (int opc);
  extern void M_define_macros_x86 (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_x86 (int id);
  extern void M_define_opcode_name_x86 (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_opcode_name_x86 (int id);
  extern int M_oper_supported_in_arch_x86 (int opc);
  extern int M_num_oper_required_for_x86 (L_Oper * oper, char *name);
  extern int M_is_stack_operand_x86 (L_Operand * operand);
  extern int M_is_unsafe_macro_x86 (L_Operand * operand);
  extern int M_operand_type_x86 (L_Operand * operand);
  extern int M_conflicting_operands_x86 (L_Operand * operand,
					 L_Operand ** conflict_array, int len,
					 int prepass);
  extern void M_get_memory_operands_x86 (int *first, int *number,
					 int proc_opc);
  extern int M_memory_access_size_x86 (L_Oper * op);
  extern int M_get_data_type_x86 (L_Oper * op);
  extern int M_num_registers_x86 (int ctype);
  extern int M_is_implicit_memory_op_x86 (L_Oper * oper);

#ifdef __cplusplus
}
#endif

#endif
