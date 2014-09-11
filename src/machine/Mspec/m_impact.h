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
 *  File:  m_impact.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 * 		generator.
 *
 *  Creation Date : March, 1993
 *
 *  Author:  Scott A. Mahlke, Wen-mei Hwu
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

#ifndef M_IMPACT_H
#define M_IMPACT_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <machine/m_spec.h>

/*===========================================================================*/
/*
 *	IMPACT processor models
 */
/*===========================================================================*/

enum
{
  M_IM_LCODE = 1000,		/* Lcode instr set, impact macros,
				   impact calling convention, impact
				   stack layout, etc. */

  /* The rest of these models are for internal use only!  -ITI (JCG) 2/99 */
  M_IM_VER_1,			/* Lcode instr set, impact macros,
				   impact calling convention, impact
				   stack layout, etc. */
  M_IM_HP_LCODE,		/* Lcode instr set, hp macros,
				   hp calling convenction, hp stack
				   layout, etc. */
  M_IM_HP_MCODE,		/* hppa PA-RISC instr set, hp macros,
				   hp calling convention, hp stack
				   layout, etc. */
  M_IM_SPARC_LCODE,		/* Lcode instr set, sparc macros,
				   sparc calling convention, sparc
				   stack layout, etc. */
  M_IM_SPARC_MCODE		/* sparc instr set, sparc macros,
				   sparc calling convention, sparc
				   stack layout, etc */
};


/* Macro used in Limpact codegen */
#define ADDR_ALIGN(addr, align) ( ((addr)+(align-1)) & ~(align-1) )

/*===========================================================================*/
/*
 *	Code generator specific macros
 */
/*===========================================================================*/

/*
 *	IMPACT itself has no macros currently, get HP and Sparc macros
 *	from m_hppa.h and m_sparc.h (SAM 4-95)
 */


/*===========================================================================*/
/*
 *	Processor specific opcodes
 */
/*===========================================================================*/

/*
 *	IMPACT itself has none currently, uses Lcode instruction set only,
 *	get HP and Sparc processor specific opcodes from m_hppa.h and
 *	m_sparc.h (SAM 4-95)
 */


/*===========================================================================*/
/*
 *	Mdes interface defines
 */
/*===========================================================================*/

/*
 *	IMPACT itself has none currently - get HP and Sparc interface
 *	defines directly from m_hppa.h and m_sparc.c (SAM 4-95)
 */

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_impact_model;

  extern int M_impact_type_size (int mtype);
  extern int M_impact_type_align (int mtype);
  extern void M_impact_void (M_Type type);
  extern void M_impact_bit_long (M_Type type, int n);
  extern void M_impact_bit_int (M_Type type, int n);
  extern void M_impact_bit_short (M_Type type, int n);
  extern void M_impact_bit_char (M_Type type, int n);
  extern void M_impact_char (M_Type type, int unsign);
  extern void M_impact_short (M_Type type, int unsign);
  extern void M_impact_int (M_Type type, int unsign);
  extern void M_impact_long (M_Type type, int unsign);
  extern void M_impact_float (M_Type type, int unsign);
  extern void M_impact_double (M_Type type, int unsign);
  extern void M_impact_pointer (M_Type type);
  extern int M_impact_eval_type (M_Type type, M_Type ntype);
  extern int M_impact_eval_type2 (M_Type type, M_Type ntype);
  extern int M_impact_call_type (M_Type type, M_Type ntype);
  extern int M_impact_call_type2 (M_Type type, M_Type ntype);
  extern void M_impact_array_layout (M_Type type, int *offset);
  extern int M_impact_array_align (M_Type type);
  extern int M_impact_array_size (M_Type type, int dim);
  extern void M_impact_union_layout (int n, _M_Type * type, int *offset,
				     int *bit_offset);
  extern int M_impact_union_align (int n, _M_Type * type);
  extern int M_impact_union_size (int n, _M_Type * type);
  extern void M_impact_struct_layout (int n, _M_Type * type, int *base,
				      int *bit_offset);
  extern int M_impact_struct_align (int n, _M_Type * type);
  extern int M_impact_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_impact_layout_fnvar (List param_list, char **base_macro,
				    int *pcount, int purpose);
  extern int M_impact_fnvar_layout (int n, _M_Type * type, long int *offset,
				    int *mode, int *reg, int *paddr,
				    char **base_macro, int *su_sreg,
				    int *su_ereg, int *pcount, int need_ST,
				    int purpose);
  extern int M_impact_lvar_layout (int n, _M_Type * type, long int *offset,
				   char **base_macro);
  extern int M_impact_no_short_int (void);
  extern int M_impact_layout_order (void);
  extern void M_impact_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_impact_is_cb_label (char *label, char *fn, int *cb);
  extern void M_impact_jumptbl_label_name (char *fn, int tbl_id, char *line,
					   int len);
  extern int M_impact_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_impact_structure_pointer (int purpose);
  extern int M_impact_return_register (int type, int purpose);
  extern char *M_impact_fn_label_name (char *label,
				       int (*is_func) (char *is_func_label));
  extern char *M_impact_fn_name_from_label (char *label);
  extern void M_set_model_impact (char *model_name);
  extern int M_impact_fragile_macro (int macro_value);
  extern Set M_impact_fragile_macro_set ();
  extern int M_impact_dataflow_macro (int id);
  extern int M_impact_subroutine_call (int opc);
  extern int M_oper_supported_in_arch_impact (int opc);
  extern int M_num_oper_required_for_impact (L_Oper * oper, char *name);
  extern int M_is_stack_operand_impact (L_Operand * operand);
  extern int M_is_unsafe_macro_impact (L_Operand * operand);
  extern int M_operand_type_impact (L_Operand * operand);
  extern int M_conflicting_operands_impact (L_Operand * operand,
					    L_Operand ** conflict_array,
					    int len, int prepass);
  extern void M_define_macros_impact (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_impact (int id);
  extern int M_num_registers_impact (int ctype);

#ifdef __cplusplus
}
#endif

#endif
