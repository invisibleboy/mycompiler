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
 *  File:  m_ti.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 		generator.
 *
 *  Creation Date : Decemeber, 1995
 *
 *  Authors:  Dan Connors and Sabrina Hwu
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

#ifndef M_TI_H
#define M_TI_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <machine/m_spec.h>

/* WARNING WARNING WARNING ! ! !
 * 
 * Any changes to this file may need to be updated into
 * m_impact.h
 */

#define INT_2EXP15             0x8000
#define INT_2EXP23             0x800000

#define FIELD_16(a)      ((((a) >= -INT_2EXP15)&&((a) < INT_2EXP15))?1:0)
#define FIELD_24(a)      ((((a) >= -INT_2EXP23)&&((a) < INT_2EXP23))?1:0)

/*
 * Declarations for processor models
 */
enum
{
  M_TI_1 = 1,
};

enum
{
  TI_MAC_LEAF = 100,	       /* 1 if leaf function, 0 if not leaf function */
  TI_MAC_ALLOC,		       /* total alloc space requirements             */
  TI_MAC_CALLEE_I,
  TI_MAC_CALLEE_F,
  TI_MAC_TRUE_SP,
  TI_MAC_COMPUTATION_REG0,
  TI_MAC_COMPUTATION_REG1,
  TI_MAC_COMPUTATION_REG2,
  TI_MAC_COMPUTATION_REG3,
  TI_MAC_COMPUTATION_REG4,
  TI_MAC_COMPUTATION_REG5,
  TI_MAC_COMPUTATION_REG6,
  TI_MAC_COMPUTATION_REG7,
  TI_MAC_AUXILIARY_REG0,
  TI_MAC_AUXILIARY_REG1,
  TI_MAC_AUXILIARY_REG2,
  TI_MAC_AUXILIARY_REG3,
  TI_MAC_AUXILIARY_REG4,
  TI_MAC_AUXILIARY_REG5,
  TI_MAC_AUXILIARY_REG6,
  TI_MAC_AUXILIARY_REG7,
  TI_MAC_ST,			/* Status register                           */
  TI_MAC_BK,			/* Block-size register                       */
  TI_MAC_INDEX_REG0,		/* Index register 0                          */
  TI_MAC_INDEX_REG1,		/* Index register 1                          */
  TI_MAC_FP,			/* Frame pointer, ar3                        */
  TI_MAC_DP,			/* data pointer                              */
  TI_MAC_RS,			/* Repeat start address                      */
  TI_MAC_RE,			/* Repeat end address                        */
  TI_MAC_RC			/* Repeat counter                            */
};



/*
 * TI Processor Specific Opcodes
 */

/* one operand alu */
#define LTIop_ALU_DIRECT			1000
#define LTIop_ALU_INDIRECT     			1001

/* two operand alu */
#define LTIop_ALU_REG_INDIRECT          	1050
#define LTIop_ALU_INDIRECT_REG  	   	1051
#define LTIop_ALU_INDIRECT_INDIRECT		1052
#define LTIop_ALU_REG_DIRECT    		1053

/* misc */
#define LTIop_BS				1150
#define LTIop_BD				1151
#define LTIop_BR				1152
#define LTIop_BRD				1153
#define LTIop_CALL				1154
#define LTIop_RETS				1155
#define LTIop_REPEAT_BLOCK			1156
#define LTIop_REPEAT_INSTRUCTION		1157
#define LTIop_POP				1158
#define LTIop_POPF				1159
#define LTIop_PUSH				1160
#define LTIop_PUSHF				1161
#define LTIop_SUBRI     			1162
#define LTIop_SUBRF     			1163
#define LTIop_CALLU				1164
#define LTIop_BU                                1165
#define LTIop_BUD                               1166
#define LTIop_DB				1167
#define LTIop_DBD				1168

/* parallel ops */
#define LTIop_PAR_STORE 			1301
#define LTIop_PAR_LOAD  			1302
#define LTIop_PAR_ONE_OPERAND 			1303
#define LTIop_PAR_TWO_OPERAND			1304


#define TIopcode_ALU_DIRECT			"LTIop_ALU_DIRECT"
#define TIopcode_ALU_INDIRECT			"LTIop_ALU_INDIRECT"

#define TIopcode_ALU_REG_INDIRECT		"LTIop_ALU_REG_INDIRECT"
#define TIopcode_ALU_INDIRECT_REG		"LTIop_ALU_INDIRECT_REG"
#define TIopcode_ALU_INDIRECT_INDIRECT		"LTIop_ALU_INDIRECT_INDIRECT"
#define TIopcode_ALU_REG_DIRECT			"LTIop_ALU_REG_DIRECT"

#define TIopcode_BS				"LTIop_BS"
#define TIopcode_BD				"LTIop_BD"
#define TIopcode_BR				"LTIop_BR"
#define TIopcode_BRD				"LTIop_BRD"
#define TIopcode_CALL				"LTIop_CALL"
#define TIopcode_RETS				"LTIop_RETS"
#define TIopcode_REPEAT_BLOCK			"LTIop_REPEAT_BLOCK"
#define TIopcode_REPEAT_INSTRUCTION		"LTIop_REPEAT_INSTRUCTION"
#define TIopcode_POP				"LTIop_POP"
#define TIopcode_POPF				"LTIop_POPF"
#define TIopcode_PUSH				"LTIop_PUSH"
#define TIopcode_PUSHF				"LTIop_PUSHF"
#define TIopcode_SUBRI				"LTIop_SUBRI"
#define TIopcode_SUBRF				"LTIop_SUBRF"
#define TIopcode_CALLU				"LTIop_CALLU"
#define TIopcode_BU                             "LTIop_BU"
#define TIopcode_BUD                            "LTIop_BUD"
#define TIopcode_DB				"LTIop_DB"
#define TIopcode_DBD				"LTIop_DBD"

#define TIopcode_PAR_STORE			"LTIop_PAR_STF"
#define TIopcode_PAR_LOAD			"LTIop_PAR_STI"
#define TIopcode_PAR_ONE_OPERAND		"LTIop_PAR_ADDF_SUBF"
#define TIopcode_PAR_TWO_OPERAND		"LTIop_PAR_ADDI_SUBI"

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_ti_type_size (int mtype);
  extern int M_ti_type_align (int mtype);
  extern void M_ti_void (M_Type type);
  extern void M_ti_bit_long (M_Type type, int n);
  extern void M_ti_bit_int (M_Type type, int n);
  extern void M_ti_bit_short (M_Type type, int n);
  extern void M_ti_bit_char (M_Type type, int n);
  extern void M_ti_char (M_Type type, int unsign);
  extern void M_ti_short (M_Type type, int unsign);
  extern void M_ti_int (M_Type type, int unsign);
  extern void M_ti_long (M_Type type, int unsign);
  extern void M_ti_float (M_Type type, int unsign);
  extern void M_ti_double (M_Type type, int unsign);
  extern void M_ti_pointer (M_Type type);
  extern int M_ti_eval_type (M_Type type, M_Type ntype);
  extern int M_ti_eval_type2 (M_Type type, M_Type ntype);
  extern int M_ti_call_type (M_Type type, M_Type ntype);
  extern int M_ti_call_type2 (M_Type type, M_Type ntype);
  extern void M_ti_array_layout (M_Type type, int *offset);
  extern int M_ti_array_align (M_Type type);
  extern int M_ti_array_size (M_Type type, int dim);
  extern void M_ti_union_layout (int n, _M_Type * type, int *offset,
				 int *bit_offset);
  extern int M_ti_union_align (int n, _M_Type * type);
  extern int M_ti_union_size (int n, _M_Type * type);
  extern void M_ti_struct_layout (int n, _M_Type * type, int *base,
				  int *bit_offset);
  extern int M_ti_struct_align (int n, _M_Type * type);
  extern int M_ti_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_ti_layout_fnvar (List param_list, char **base_macro,
				int *pcount, int purpose);
  extern int M_ti_fnvar_layout (int n, _M_Type * type, long int *offset,
				int *mode, int *reg, int *paddr, char **macro,
				int *su_sreg, int *su_ereg, int *pcount,
				int is_st, int purpose);
  extern int M_ti_lvar_layout (int n, _M_Type * type, long int *offset,
			       char **base_macro);
  extern int M_ti_no_short_int (void);
  extern int M_ti_layout_order (void);
  extern void M_ti_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_ti_is_cb_label (char *label, char *fn, int *cb);
  extern void M_ti_jumptbl_label_name (char *fn, int tbl_id, char *line,
				       int len);
  extern int M_ti_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_ti_structure_pointer (int purpose);
  extern int M_ti_return_register (int type, int purpose);
  extern char *M_ti_fn_label_name (char *label,
				   int (*is_func) (char *is_func_label));
  extern char *M_ti_fn_name_from_label (char *label);
  extern void M_set_model_ti (char *model_name);
  extern int M_ti_fragile_macro (int macro_value);
  extern int M_ti_subroutine_call (int opc);
  extern void M_define_macros_ti (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_ti (int id);
  extern void M_define_opcode_name_ti (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_opcode_name_ti (int id);
  extern int M_oper_supported_in_arch_ti (int opc);
  extern int M_num_oper_required_for_ti (L_Oper * oper, char *name);
  extern int M_is_stack_operand_ti (L_Operand * operand);
  extern int M_is_unsafe_macro_ti (L_Operand * operand);
  extern int M_operand_type_ti (L_Operand * operand);
  extern int M_conflicting_operands_ti (L_Operand * operand,
					L_Operand ** conflict_array, int len,
					int prepass);
  extern int M_num_registers_ti (int ctype);

#ifdef __cplusplus
}
#endif

#endif
