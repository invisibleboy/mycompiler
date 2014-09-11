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
 *	File :		m_spec.h
 *	Author:		Pohua Chang, Wen-mei Hwu
 *	Creation Date:	June, 1990
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

#ifndef M_SPEC_H
#define M_SPEC_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/dynamic_symbol.h>
#include <Lcode/l_code.h>
#include <library/l_parms.h>
#include <library/i_list.h>

/*===========================================================================
 *	Description :	Machine specific parameters.
 *==========================================================================*/

/*
 *	Machine architecture models.
 */

#define M_IMPACT		0
#define M_SPARC			1
/* JLB 05-05-97 Support for MIPS, AMD and I860 deleted */
/*#define M_MIPS			2
#define M_AMD			3
#define M_I860			4*/
#define M_HPPA			5
#define M_X86			6
#define M_PLAYDOH		7
#define M_TI			8
#define M_SH			9
#define M_BX86                  10
#define M_TAHOE                 11
#define M_STARCORE              12
#define M_WIMS                  13
#define M_ARM                   14

/*
 *	Jump table stuff (SAM 8-96)
 */
#define M_JUMPTBL_BASE_NAME		"_hash_"


/*
 * Common MDES Operand Specifiers
 */
#define MDES_OPERAND_NULL		0  /* DO NOT CHANGE this value! */
#define MDES_OPERAND_i			1
#define MDES_OPERAND_f			2
#define MDES_OPERAND_f2			3
#define MDES_OPERAND_p			4
#define MDES_OPERAND_Label		5
#define MDES_OPERAND_REG		6
#define MDES_OPERAND_btr		7
#define MDES_OPERAND_cntl		8  /* Added for Lplaydoh -JCG 5/5/98 */
#define MDES_OPERAND_Lit               	9
#define MDES_OPERAND_Lit1               10
#define MDES_OPERAND_Lit2               11
#define MDES_OPERAND_Lit3               12
#define MDES_OPERAND_Lit4               13
#define MDES_OPERAND_Lit5               14
#define MDES_OPERAND_Lit6               15
#define MDES_OPERAND_Lit7               16
#define MDES_OPERAND_Lit8               17
#define MDES_OPERAND_Lit9               18
#define MDES_OPERAND_Lit10              19
#define MDES_OPERAND_Lit11              20
#define MDES_OPERAND_Lit12              21
#define MDES_OPERAND_Lit13              22
#define MDES_OPERAND_Lit14              23
#define MDES_OPERAND_Lit15              24
#define MDES_OPERAND_Lit16              25
#define MDES_OPERAND_Lit17              26
#define MDES_OPERAND_Lit18              27
#define MDES_OPERAND_Lit19              28
#define MDES_OPERAND_Lit20              29
#define MDES_OPERAND_Lit21              30
#define MDES_OPERAND_Lit22              31
#define MDES_OPERAND_Lit23              32
#define MDES_OPERAND_Lit24              33
#define MDES_OPERAND_Lit25              34
#define MDES_OPERAND_Lit26              35
#define MDES_OPERAND_Lit27              36
#define MDES_OPERAND_Lit28              37
#define MDES_OPERAND_Lit29              38
#define MDES_OPERAND_Lit30              39
#define MDES_OPERAND_Lit31              40
#define MDES_OPERAND_Lit32              41
#define MDES_OPERAND_Lit64              42
#define MDES_OPERAND_addr               43 /* Added for Lstarcore CJS 6/01 */

#define M_add_symbol(table,name,id)	STRING_add_symbol((table),(name), \
                                        (void *)(id))

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_arch;                    /* processor architecture family */
  extern int M_model;                   /* processor model               */
  extern int M_swarch;                  /* software convention           */

  extern int Mspec_num_int_caller_reg;
  extern int Mspec_num_int_callee_reg;
  extern int Mspec_num_flt_caller_reg;
  extern int Mspec_num_flt_callee_reg;
  extern int Mspec_num_dbl_caller_reg;
  extern int Mspec_num_dbl_callee_reg;
  extern int Mspec_num_prd_caller_reg;
  extern int Mspec_num_prd_callee_reg;

  extern void M_read_parm (Parm_Parse_Info * ppi);

  extern int M_use_layout_database;
  extern char *M_layout_database_name;
  extern char *M_layout_database_desc ();
  extern int M_read_database_i (char *section_name, char *entry_name,
				char *field_name);
  extern char *M_read_database_s (char *section_name, char *entry_name,
				  char *field_name);
  extern int M_database_info_present (char *section_name, char *entry_name,
				      char *field_name);
  extern void M_assert (int cc, char *mesg);
  extern void M_set_machine (char *arch, char *model, char *swarch);
  extern char *M_get_macro_name (int id);
  extern void M_define_macros (STRING_Symbol_Table * sym_tbl);
  extern int M_subroutine_call (int opc);
  extern int M_fragile_macro (int macro_value);
  extern Set M_fragile_macro_set ();
  extern int M_dataflow_macro (int id);
  extern int M_zero_macro(L_Operand *opd);
  extern void M_define_opcode_name (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_opcode_name (int id);
  extern void M_set_swarch_tahoe (char *model_name);

  /* IA64 sias 20000517 */

  extern int M_cannot_predicate (L_Oper * oper);
  extern int M_extra_pred_define_opcode (int proc_opc);
  extern int M_extra_pred_define_type1 (L_Oper *);
  extern int M_extra_pred_define_type2 (L_Oper *);

  extern int M_native_int_register_ctype (void);
  extern int M_native_int_register_mtype (void);

#ifdef __cplusplus
}
#endif

/*
 *	define the natural integer size.
 *	bit fields will be converted into integer
 *	operations.
 */

#define M_SIZE_LLONG            64
#define M_ALIGN_LLONG           64
#define M_SIZE_INT		32
#define M_ALIGN_INT		32
#define M_SIZE_CHAR		 8

/*
 *	assumption 1: all pointers have the same properties.
 *	assumption 2: unsigned & signed types have the same properties.
 *	assumption 3: a function is simply a address == a pointer.
 */
#define M_TYPE_VOID		0	/* void */
#define M_TYPE_CHAR		1	/* char, unsigned char */
#define M_TYPE_SHORT		2	/* short, unsigned short */
#define M_TYPE_INT		3	/* int, unsigned int */
#define M_TYPE_LONG		4	/* long, unsigned long */
#define M_TYPE_LLONG            5	/* long long, unsigned long long */
#define M_TYPE_FLOAT		6	/* float, unsigned float */
#define M_TYPE_DOUBLE		7	/* double, unsigned double */
#define M_TYPE_POINTER		8	/* char *, void *, ... */
#define M_TYPE_UNION		9	/* union {} */
#define M_TYPE_STRUCT		10	/* struct {} */
#define M_TYPE_BLOCK		11	/* array */
#define M_TYPE_BIT_CHAR		12	/* char:1 */
#define M_TYPE_BIT_SHORT	13	/* short:1 */
#define M_TYPE_BIT_INT          14	/* int:1 */
#define M_TYPE_BIT_LONG		15	/* long int:1 */
#define M_TYPE_BIT_LLONG	16	/* long long int:1 */

#define M_TYFL_HFA_SGL          1       /* homogeneous floating point aggr */
#define M_TYFL_HFA_DBL          2       /* homogeneous floating point aggr */
#define M_TYFL_HFA              3       /* homogeneous floating point aggr */

typedef struct _struct_M_Type
{
  int type;			/* type of a variable */
  int unsign;			/* unsigned type */
  int align;			/* its alignment requirement (in bits) */
  int size;			/* minimum size it needs (in bits) */
  int nbits;			/* for M_TYPE_BIT only */
  int flags;
}
_M_Type, *M_Type;

#ifdef __cplusplus
extern "C"
{
#endif

  extern void M_void (M_Type type);
  extern void M_bit_llong (M_Type type, int n, int unsign);
  extern void M_bit_long (M_Type type, int n, int unsign);
  extern void M_bit_int (M_Type type, int n, int unsign);
  extern void M_bit_short (M_Type type, int n, int unsign);
  extern void M_bit_char (M_Type type, int n, int unsign);
  extern void M_char (M_Type type, int unsign);
  extern void M_short (M_Type type, int unsign);
  extern void M_int (M_Type type, int unsign);
  extern void M_long (M_Type type, int unsign);
  extern void M_llong (M_Type type, int unsign);
  extern void M_float (M_Type type, int unsign);
  extern void M_double (M_Type type, int unsign);
  extern void M_pointer (M_Type type);
  extern void M_union (M_Type type, int align, int size);
  extern void M_struct (M_Type type, int align, int size);
  extern void M_block (M_Type type, int align, int size);

  extern int M_eval_type (M_Type type, M_Type ntype);
  extern int M_eval_type2 (M_Type type, M_Type ntype);
  extern int M_call_type (M_Type type, M_Type ntype);
  extern int M_call_type2 (M_Type type, M_Type ntype);

  extern int M_type_size (int mtype);
  extern int M_type_align (int mtype);

  extern int M_return_value_thru_stack();
  extern int M_return_value_offset();

/*
 * Fields of a union can be aligned to the lower address
 * or to the higher address of the allocated space.
 * offset is specified relative to the lower address of the data block.
 * XXX_layout() determines the offset of fields.
 * XXX_align() returns the alignment requirement of the overall structure.
 * XXX_size() returns the minimum storage requirement of the overall structure.
 */
  extern void M_array_layout (M_Type type, int *offset);
  extern int M_array_align (M_Type type);
  extern int M_array_size (M_Type type, int dim);

  extern void M_union_layout (int n, _M_Type * type, int *offset,
			      int *bit_offset, char *union_name,
			      char *field_name[]);

  extern int M_union_align (int n, _M_Type * type, char *union_name);
  extern int M_union_size (int n, _M_Type * type, char *union_name);

  extern void M_struct_layout (int n, _M_Type * type, int *base,
			       int *bit_offset, char *struct_name,
			       char *field_name[]);
  extern int M_struct_align (int n, _M_Type * type, char *struct_name);
  extern int M_struct_size (int n, _M_Type * type, int struct_align,
			    char *struct_name);

#ifdef __cplusplus
}
#endif

/*
 *	Variable layout.
 */
#define M_THRU_REGISTER			0
#define M_THRU_MEMORY			1
#define M_INDIRECT_THRU_REGISTER	2	/* a pointer to a structure */
#define M_INDIRECT_THRU_MEMORY		3	/* a pointer to a structure */

#define M_DONT_CARE_FNVAR	0	/* purpose */
#define M_GET_FNVAR		1       /* incoming parameter frame setup */
#define M_PUT_FNVAR		2       /* outgoing parameter frame setup */
#define M_RET_FNVAR             3       /* return value setup             */

#ifdef __cplusplus
extern "C"
{
#endif
  extern int M_layout_fnvar (List param_list, char **base_macro,
			     int *pcount, int purpose, int need_ST);
  extern int M_fnvar_layout (int n, _M_Type * type, long int *offset,
			     int *mode, int *reg, int *paddr,
			     char **base_macro, int *su_sreg, int *su_ereg,
			     int *pcount, int need_ST, int purpose);
  extern int M_lvar_layout (int n, _M_Type * type, long int *offset,
			    char **base_macro);
/*
 *	On some systems, we do not distinguish between short
 *	and integers. The following predicate function can be
 *	used by the Hcode tool.
 */
  extern int M_no_short_int (void);

  extern int M_supports_long_long (void);

  extern int M_pointer_size (void);

/*
 *	Type casting.
 */
  extern int M_compatible_type (M_Type type1, M_Type type2);

#ifdef __cplusplus
}
#endif

/*
 *	Data layout.
 */
#define M_LITTLE_ENDIAN		0
#define M_BIG_ENDIAN		1

#ifdef __cplusplus
extern "C"
{
#endif

  extern int M_layout_order (void);

/*
 *	Name of a cb.
 */
  extern void M_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_is_cb_label (char *label, char *fn, int *cb);

  extern void M_jumptbl_label_name (char *fn, int tbl_id, char *line,
				    int len);
  extern int M_is_jumptbl_label (char *label, char *fn, int *tbl_id);

  extern char *M_fn_name_from_label (char *label);
  extern char *M_fn_label_name (char *label, int (*is_func)
				(char *is_func_label));
/*
 *	Special registers.
 */
  extern int M_structure_pointer (int purpose);
  extern int M_return_register (int type, int purpose);
  extern L_Operand *M_return_epilogue_cntr_register ();
  extern L_Operand *M_return_loop_cntr_register ();

/*
 * Target machine instruction set capabilities
 */
  extern int M_oper_supported_in_arch (int opc);
  extern int M_num_oper_required_for (L_Oper * oper, char *name);
  extern int M_is_stack_operand (L_Operand * operand);
  extern int M_is_unsafe_macro (L_Operand * operand);
  extern void M_get_memory_operands (int *, int *, int);
  extern int M_num_registers (int);
  extern int M_memory_access_size (L_Oper * op);
  extern int M_get_data_type (L_Oper * op);
  extern int M_scaled_addressing_avail (void);
  extern int M_is_implicit_memory_op (L_Oper * oper);
  extern int M_coalescing_oper (L_Oper *);

/* 
 * MDES stuff
 */

  typedef int (*IFPTR1) (L_Operand * operand);
  typedef int (*IFPTR2) (L_Operand * operand, L_Operand ** conflict_array,
			 int len, int prepass);

  extern IFPTR1 M_mdes_operand_type (void);
  extern IFPTR2 M_dep_conflicting_operands (void);

  extern int M_opc_from_proc_opc (int proc_opc);

#ifdef __cplusplus
}
#endif

typedef struct _struct_M_Param {
  _M_Type mtype;
  int mode;
  long int offset;
  int reg;
  int paddr;
  int su_sreg;
  int su_ereg;
} _M_Param, *M_Param;

extern int M_layout_retvar (M_Param param, int purpose);

#endif
