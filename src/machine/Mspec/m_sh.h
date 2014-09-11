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
 *  File:  m_sh.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 *              generator.
 *
 *  Creation Date: Oct, 1996
 *
 *  Author: Yoji Yamada, Wen-mei Hwu
 *
 *  Revisions:
 *      Oct, 1996, Yoji Yamada
 *          Modified from m_sparc.h to support SH.
 *      March, 1997, Daniel Lavery
 *          Add SuperH processor specific opcodes
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

#ifndef M_SH_H
#define M_SH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <machine/m_spec.h>

/*
 *	WARNING WARNING WARNING!!
 *	Any updates to this file must be reflected in m_impact.h.  See Rick
 *	or Scott for details.
 */

/*
 * Declarations for processor models
 */
enum
{
  M_SH_1,			/* model 1 */
  M_SH_2			/* model 2 */
};

/*
 * Declarations for macros
 */
enum
{
  L_SH_MAC_LEAF = 100,		/* 1 if leaf function */
  L_SH_MAC_ALLOC,		/* total alloc space requirements */
  L_SH_MAC_CALLEE,		/* # of callee save registers */
  L_SH_MAC_CALLER,		/* # of caller save registers */
  L_SH_MAC_SR,			/* sr, status register */
  L_SH_MAC_PR			/* pr, procedure register */
};


/* Define sh specific mdes IO_set specifiers */
#define MDES_OPERAND_sh_icc		100
#define MDES_OPERAND_sh_fcc		101


/* SH Processor Specific Opcodes */
#define LSHop_SETJMP_JSR        1000
#define LSHop_LDD_INT 		1001
#define LSHop_STD_INT 		1002
#define LSHop_CRA		1003	/* control reg access for Y reg */
#define LSHop_SAVE		1004
#define LSHop_RESTORE		1005
#define LSHop_NEG		1006	/* NEG (negate) instruction: NEG Rm,Rn
					   Mcode: 0 - Rm -> Rn */
#define LSHop_MOVT		1007	/* MOVT (move T bit) instruction: 
					   MOVT Rn
					   Mcode: mov Rn, MAC_SR */
#define LSHop_MOVA		1008	/* MOVA (move ea) instruction: 
					   MOVA Rn, ea
					   Mcode: mov Rn, ea */
#define LSHop_TST		1009	/* TST (test logical) instruction: 
					   TST Rn,Rn
					   Mcode: eq MAC_SR, Rn, 0 */
#define LSHop_CMPPL		1010	/* CMP/PL (compare) instruction: 
					   CMP/PL Rn
					   Mcode: gt MAC_SR, Rn, 0 */
#define LSHop_CMPPZ		1011	/* CMP/PZ (compare) instruction: 
					   CMP/PZ Rn
					   Mcode: ge MAC_SR, Rn, 0 */
#define LSHop_STSL		1012	/* STS.L (store system register) 
					   instruction: STS.L PR,@-Rn
					   Mcode: st_pre_i Rn, Rn, 0, PR, -4 */
#define LSHop_LDSL		1013	/* LDS.L (load system register) 
					   instruction: LDS.L @Rm+,PR
					   Mcode: ld_post_i PR, Rm, Rm, 0, 4 */


#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_sh_type_size (int mtype);
  extern int M_sh_type_align (int mtype);
  extern void M_sh_void (M_Type type);
  extern void M_sh_bit_long (M_Type type, int n);
  extern void M_sh_bit_int (M_Type type, int n);
  extern void M_sh_bit_short (M_Type type, int n);
  extern void M_sh_bit_char (M_Type type, int n);
  extern void M_sh_char (M_Type type, int unsign);
  extern void M_sh_short (M_Type type, int unsign);
  extern void M_sh_int (M_Type type, int unsign);
  extern void M_sh_long (M_Type type, int unsign);
  extern void M_sh_float (M_Type type, int unsign);
  extern void M_sh_double (M_Type type, int unsign);
  extern void M_sh_pointer (M_Type type);
  extern int M_sh_eval_type (M_Type type, M_Type ntype);
  extern int M_sh_eval_type2 (M_Type type, M_Type ntype);
  extern int M_sh_call_type (M_Type type, M_Type ntype);
  extern int M_sh_call_type2 (M_Type type, M_Type ntype);
  extern void M_sh_array_layout (M_Type type, int *offset);
  extern int M_sh_array_align (M_Type type);
  extern int M_sh_array_size (M_Type type, int dim);
  extern void M_sh_union_layout (int n, _M_Type * type, int *offset,
				 int *bit_offset);
  extern int M_sh_union_align (int n, _M_Type * type);
  extern int M_sh_union_size (int n, _M_Type * type);
  extern void M_sh_struct_layout (int n, _M_Type * type, int *base,
				  int *bit_offset);
  extern int M_sh_struct_align (int n, _M_Type * type);
  extern int M_sh_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_sh_layout_fnvar (List param_list, char **base_macro,
				int *pcount, int purpose);
  extern int M_sh_fnvar_layout (int n, _M_Type * type, long int *offset,
				int *mode, int *reg, int *paddr, char **macro,
				int *su_sreg, int *su_ereg, int *pcount,
				int is_st, int purpose);
  extern int M_sh_lvar_layout (int n, _M_Type * type, long int *offset,
			       char **base_macro);
  extern int M_sh_no_short_int (void);
  extern int M_sh_layout_order (void);
  extern void M_sh_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_sh_is_cb_label (char *label, char *fn, int *cb);
  extern void M_sh_jumptbl_label_name (char *fn, int tbl_id, char *line,
				       int len);
  extern int M_sh_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_sh_structure_pointer (int purpose);
  extern int M_sh_return_register (int type, int purpose);
  extern char *M_sh_fn_label_name (char *label,
				   int (*is_func) (char *is_func_label));
  extern char *M_sh_fn_name_from_label (char *label);
  extern void M_set_model_sh (char *model_name);
  extern void M_define_macros_sh (STRING_Symbol_Table * sym_tbl);
  extern int M_sh_fragile_macro (int macro_value);
  extern Set M_sh_fragile_macro_set ();
  extern int M_sh_dataflow_macro (int id);
  extern int M_sh_subroutine_call (int opc);
  extern char *M_get_macro_name_sh (int id);
  extern int M_oper_supported_in_arch_sh (int opc);
  extern int M_is_stack_operand_sh (L_Operand * operand);
  extern int M_is_unsafe_macro_sh (L_Operand * operand);
  extern int M_num_oper_required_for_sh (L_Oper * oper, char *name);
  extern int M_operand_type_sh (L_Operand * operand);
  extern int M_conflicting_operands_sh (L_Operand * operand,
					L_Operand ** conflict_array, int len,
					int prepass);
  extern int M_num_registers_sh (int ctype);

#ifdef __cplusplus
}
#endif

#endif
