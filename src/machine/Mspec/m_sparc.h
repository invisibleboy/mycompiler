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
 *  File:  m_sparc.h
 *
 *  Description: Defines symbols used for reference in Mspec and code
 *              generator.
 *
 *  Creation Date : May, 1993
 *
 *  Author:  Sabrina Y. Hwu
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

#ifndef M_SPARC_H
#define M_SPARC_H

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
  M_SPARC_ELC = 0,		/* SPARC Single issue v8 */
  M_SPARC_VIKING,		/* SPARC Three issue v8  */
  M_SPARC_ILP8			/* SPARC 8-issue v9      */
};


/*
 * Declarations for macros
 */
enum
{

  L_SPARC_MAC_ZERO = 200,       /* %g0, always stores the value zero
				   (mac g0 i) */
  L_SPARC_MAC_ICC,	        /* dest reg for int_arith-cc instr 
				   (mac icc i) */
  L_SPARC_MAC_ICC0,	        /* dest reg for int_arith-cc instr
				   (mac icc0 i) */
  L_SPARC_MAC_ICC1,	        /* dest reg for int_arith-cc instr
				   (mac icc1 i) */
  L_SPARC_MAC_ICC2,	        /* dest reg for int_arith-cc instr
				   (mac icc2 i) */
  L_SPARC_MAC_ICC3,	        /* dest reg for int_arith-cc instr
				   (mac icc3 i) */
  L_SPARC_MAC_ICC4,	        /* dest reg for int_arith-cc instr
				   (mac icc4 i) */
  L_SPARC_MAC_ICC5,	        /* dest reg for int_arith-cc instr
				   (mac icc5 i) */
  L_SPARC_MAC_ICC6,	        /* dest reg for int_arith-cc instr
				   (mac icc6 i) */
  L_SPARC_MAC_ICC7,	        /* dest reg for int_arith-cc instr
				   (mac icc7 i) */
  L_SPARC_MAC_ICC8,	        /* dest reg for int_arith-cc instr
				   (mac icc8 i) */
  L_SPARC_MAC_ICC9,	        /* dest reg for int_arith-cc instr
				   (mac icc9 i) */
  L_SPARC_MAC_ICC10,	        /* dest reg for int_arith-cc instr
				   (mac icc10 i)*/
  L_SPARC_MAC_ICC11,	        /* dest reg for int_arith-cc instr
				   (mac icc11 i)*/
  L_SPARC_MAC_ICC12,	        /* dest reg for int_arith-cc instr
				   (mac icc12 i)*/
  L_SPARC_MAC_ICC13,	        /* dest reg for int_arith-cc instr
				   (mac icc13 i)*/
  L_SPARC_MAC_ICC14,	        /* dest reg for int_arith-cc instr
				   (mac icc14 i)*/
  L_SPARC_MAC_ICC15,	        /* dest reg for int_arith-cc instr
				   (mac icc15 i)*/
  L_SPARC_MAC_LEAF,	        /* 1 if leaf function, 0 if not leaf function  
				   (mac $leaf i)   */
  L_SPARC_MAC_CALLEE,	        /* # of callee save registers 
				   (mac $callee_regs i) */
  L_SPARC_MAC_CALLER,	        /* # of caller save registers 
				   (mac $caller_regs i) */
  L_SPARC_MAC_ALLOC,	        /* total alloc space requirements 
				   (mac $alloc_size i) */
  L_SPARC_MAC_PRED_BASE,	/* base addr for pred registers 
				   (mac pred_base i)   */
  L_SPARC_MAC_G1,		/* temp reg used in pp annot after Regalloc
				   (mac g1 i)      */
  L_SPARC_MAC_FCC,		/* dest reg for all floating point fp-cc instr
				   (mac fcc i)     */
  L_SPARC_MAC_FCC0,		/* dest reg for all floating point fp-cc instr
				   (mac fcc0 i)    */
  L_SPARC_MAC_FCC1,		/* dest reg for all floating point fp-cc instr
				   (mac fcc1 i)    */
  L_SPARC_MAC_FCC2,		/* dest reg for all floating point fp-cc instr
				   (mac fcc2 i)    */
  L_SPARC_MAC_FCC3,		/* dest reg for all floating point fp-cc instr
				   (mac fcc3 i)    */
  L_SPARC_MAC_FCC4,		/* dest reg for all floating point fp-cc instr
				   (mac fcc4 i)    */
  L_SPARC_MAC_FCC5,		/* dest reg for all floating point fp-cc instr
				   (mac fcc5 i)    */
  L_SPARC_MAC_FCC6,		/* dest reg for all floating point fp-cc instr
				   (mac fcc6 i)    */
  L_SPARC_MAC_FCC7,		/* dest reg for all floating point fp-cc instr
				   (mac fcc7 i)    */
  L_SPARC_MAC_FCC8,		/* dest reg for all floating point fp-cc instr
				   (mac fcc8 i)    */
  L_SPARC_MAC_FCC9,		/* dest reg for all floating point fp-cc instr 
				   (mac fcc9 i)    */
  L_SPARC_MAC_FCC10,		/* dest reg for all floating point fp-cc instr
				   (mac fcc10 i)   */
  L_SPARC_MAC_FCC11,		/* dest reg for all floating point fp-cc instr
				   (mac fcc11 i)   */
  L_SPARC_MAC_FCC12,		/* dest reg for all floating point fp-cc instr
				   (mac fcc12 i)   */
  L_SPARC_MAC_FCC13,		/* dest reg for all floating point fp-cc instr
				   (mac fcc13 i)   */
  L_SPARC_MAC_FCC14,		/* dest reg for all floating point fp-cc instr
				   (mac fcc14 i)   */
  L_SPARC_MAC_FCC15,		/* dest reg for all floating point fp-cc instr
				   (mac fcc15 i)   */
  L_SPARC_MAC_RETADDR,		/* register where return address is placed  
				   (mac o7 i)      */
  L_SPARC_MAC_NONLEAF_RETADDR,	/* return address for non-leaf functions 
				   (mac i7 i)      */
  L_SPARC_MAC_CARRY_SET,	/* carry set macro for branches
				   (mac C_set i)   */
  L_SPARC_MAC_CARRY_CLEAR,	/* carry clear macro for branches
				   (mac C_clear i) */
  L_SPARC_MAC_NEG_SET,		/* negative set macro for branches
				   (mac N_set i)   */
  L_SPARC_MAC_NEG_CLEAR,	/* negative clear macro for branches
				   (mac N_clear i) */
  L_SPARC_MAC_OVERFLOW_SET,	/* overflow set macro for branches
				   (mac V_set i)   */
  L_SPARC_MAC_OVERFLOW_CLEAR,	/* overflow clear macro for branches
				   (mac V_clear i) */
  L_SPARC_MAC_FSR,		/* floating point status register
				   (mac fsr i)     */
  L_SPARC_MAC_Y,		/* special Y register used for mulscc
				   (mac y i)       */
};


/* Define sparc specific mdes IO_set specifiers */
#define MDES_OPERAND_sparc_icc		100
#define MDES_OPERAND_sparc_fcc		101


/* SPARC Processor Specific Opcodes */
#define LSPARCop_SETJMP_JSR             1000
#define LSPARCop_LDD_INT 		1001
#define LSPARCop_STD_INT 		1002
#define LSPARCop_CRA			1003
#define LSPARCop_SAVE			1004
#define LSPARCop_RESTORE		1005


#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_sparc_type_size (int mtype);
  extern int M_sparc_type_align (int mtype);
  extern void M_sparc_void (M_Type type);
  extern void M_sparc_bit_long (M_Type type, int n);
  extern void M_sparc_bit_int (M_Type type, int n);
  extern void M_sparc_bit_short (M_Type type, int n);
  extern void M_sparc_bit_char (M_Type type, int n);
  extern void M_sparc_char (M_Type type, int unsign);
  extern void M_sparc_short (M_Type type, int unsign);
  extern void M_sparc_int (M_Type type, int unsign);
  extern void M_sparc_long (M_Type type, int unsign);
  extern void M_sparc_float (M_Type type, int unsign);
  extern void M_sparc_double (M_Type type, int unsign);
  extern void M_sparc_pointer (M_Type type);
  extern int M_sparc_eval_type (M_Type type, M_Type ntype);
  extern int M_sparc_eval_type2 (M_Type type, M_Type ntype);
  extern int M_sparc_call_type (M_Type type, M_Type ntype);
  extern int M_sparc_call_type2 (M_Type type, M_Type ntype);
  extern void M_sparc_array_layout (M_Type type, int *offset);
  extern int M_sparc_array_align (M_Type type);
  extern int M_sparc_array_size (M_Type type, int dim);
  extern void M_sparc_union_layout (int n, _M_Type * type, int *offset,
				    int *bit_offset);
  extern int M_sparc_union_align (int n, _M_Type * type);
  extern int M_sparc_union_size (int n, _M_Type * type);
  extern void M_sparc_struct_layout (int n, _M_Type * type, int *base,
				     int *bit_offset);
  extern int M_sparc_struct_align (int n, _M_Type * type);
  extern int M_sparc_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_sparc_layout_fnvar (List param_list, char **base_macro,
				   int *pcount, int purpose);
  extern int M_sparc_fnvar_layout (int n, _M_Type * type, long int *offset,
				   int *mode, int *reg, int *paddr,
				   char **macro, int *su_sreg, int *su_ereg,
				   int *pcount, int is_st, int purpose);
  extern int M_sparc_lvar_layout (int n, _M_Type * type, long int *offset,
				  char **base_macro);
  extern int M_sparc_no_short_int (void);
  extern int M_sparc_layout_order (void);
  extern void M_sparc_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_sparc_is_cb_label (char *label, char *fn, int *cb);
  extern void M_sparc_jumptbl_label_name (char *fn, int tbl_id, char *line,
					  int len);
  extern int M_sparc_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_sparc_structure_pointer (int purpose);
  extern int M_sparc_return_register (int type, int purpose);
  extern char *M_sparc_fn_label_name (char *label,
				      int (*is_func) (char *is_func_label));
  extern char *M_sparc_fn_name_from_label (char *label);
  extern void M_set_model_sparc (char *model_name);
  extern void M_define_macros_sparc (STRING_Symbol_Table * sym_tbl);
  extern int M_sparc_fragile_macro (int macro_value);
  extern Set M_sparc_fragile_macro_set ();
  extern int M_sparc_dataflow_macro (int id);
  extern int M_sparc_subroutine_call (int opc);
  extern char *M_get_macro_name_sparc (int id);
  extern int M_oper_supported_in_arch_sparc (int opc);
  extern int M_is_stack_operand_sparc (L_Operand * operand);
  extern int M_is_unsafe_macro_sparc (L_Operand * operand);
  extern int M_num_oper_required_for_sparc (L_Oper * oper, char *name);
  extern int M_operand_type_sparc (L_Operand * operand);
  extern int M_conflicting_operands_sparc (L_Operand * operand,
					   L_Operand ** conflict_array,
					   int len, int prepass);
  extern int M_num_registers_sparc (int ctype);

#ifdef __cplusplus
}
#endif

#endif
