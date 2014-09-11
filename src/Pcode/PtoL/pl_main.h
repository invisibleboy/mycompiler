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
 *	File:	hl_main.h
 *	Author:	Po-hua Chang, Wen-mei Hwu
 *	Creation Date:	June, 1990
 *	Revised: Dave Gallagher, Scott Mahlke - 6/94
 *              Build Lcode structure rather than just printing out text file
 *	Modified: Le-Chun Wu -- 8/8/95
 *	        Add a formal parameter "attr1" to most of the PL_gen_xxx 
 *		functions so that Hcode pragma (debugging information) can be
 *		passed to Lcode.
\*****************************************************************************/
#ifndef PL_MAIN_H
#define PL_MAIN_H

#include <config.h>
#include <library/i_types.h>

#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/reduce.h>
#include <Pcode/symtab.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/query_symtab.h>
#include <Pcode/reduce_symtab.h>
#include <Pcode/query.h>
#include <Pcode/cast.h>	
#include <Pcode/extension.h>
#include <Pcode/symtab_i.h>
#include <library/select_sort.h>
#include <Lcode/l_main.h>
#include "pl_lcode.h"
#include <machine/m_spec.h>

#define PL_MANGLE_NAMES 0

/*========================================================================*/
/*
 *	Parameters for PtoL 
 */
/*========================================================================*/

/*
 * Buffer strings
 */

typedef struct Dyn_str_t 
{
  char *str;
  int   maxsize;
  int   size;
} Dyn_str_t;

extern Dyn_str_t *PL_dstr_new(int init_size);
extern void PL_dstr_free(Dyn_str_t *buf);
extern void PL_dstr_strcat(Dyn_str_t *buf, char *str);
extern char* PL_dstr2str(Dyn_str_t *buf);
extern void PL_dstr_sprintf(Dyn_str_t *buf,  const char *format, ...);
extern void PL_dstr_trunc(Dyn_str_t *buf, int num);
extern void PL_dstr_clear(Dyn_str_t *buf);

/*
 * Define PL_GEN_FLOAT_OPERANDS to cause float type registers to be
 * generated in Lcode.
 */

#define PL_GEN_FLOAT_OPERANDS
#define EMN_DEBUG_SHADOW  0
#define CONVERT_SERLOOP_TO_PARLOOP 1	/* CWL - 10/09/02 */

/* NEW Globals for easy SymbolTable, File scope access 
 */
extern SymbolTable PL_symtab;
extern int PL_file_scope;

/* Old dependence pragma flags
 */

extern int PL_gen_acc_specs;

/* Old base type size, mask variables 
 */
extern int P_CHAR_SIZE;
extern int P_SHORT_SIZE;
extern int P_INT_SIZE;
extern int P_LONG_SIZE;
extern int P_LONGLONG_SIZE;
extern int P_POINTER_SIZE;
extern int P_UNSIGNED_CHAR_MASK;
extern int P_UNSIGNED_SHORT_MASK;
extern int P_UNSIGNED_CHAR_MASK;
extern int P_UNSIGNED_SHORT_MASK;

/* TAKES mtype_type M_TYPE_XXX */
extern int PL_MType_Size(int type);
extern int PL_MType_Align(int type);
extern int PL_MType_Compatible(int type1, int type2);
extern void PL_InitTypeSizes (void);
extern int PL_MType_Convert(int type);
extern void PL_M_pointer(M_Type mtype);
extern void PL_M_int(M_Type mtype, int unsign);
extern int PL_M_no_short_int();
extern char *PL_M_fn_label_name (char *label, int is_func);

/* TAKES symboltable Key type */
extern int PL_key_get_size (Key type);
extern int PL_key_get_align (Key type);
extern int PL_key_get_mtype (Key type);
extern int PL_key_get_ctype (Key type);
extern int PL_key_get_regctype (Key type);
extern int PL_key_get_parmctype (Key type);
extern void PL_encode_type_name(Key type, Dyn_str_t *name);
extern Field PL_key_get_field(Key type, char *field_name);
extern int PL_is_func_var(Key var_key);

/* Other support funcs */

extern int PL_bit_field_info(Field field, ITuintmax *mask, int *shift);

/* ENDEND*/
/* ENDEND*/
/* ENDEND*/


extern int PL_normalize_loops;

extern int PL_generate_hashing_branches;
extern int PL_ignore_hash_profile_weight;
extern int PL_ignore_hash_br_seq_weight;
extern int PL_generate_abs_instructions;
extern int PL_annotate_omega;
extern int PL_generate_label_attrs;
extern int PL_generate_static_branch_attrs;
extern int PL_generate_acc_name_attrs;
extern int PL_debug_sync_arcs;
extern int PL_gen_bit_field_operations;
extern int PL_generate_sign_extend_operations;
extern int PL_use_subroutine_call;
extern int PL_initialize_function_live_ins;
/* LCW - emit source information - 8/5/97 */
extern int PL_emit_source_info;
/* Allow emitting of subset of source info that Lemulate needs -ITI/JCG 4/99 */
extern int PL_emit_data_type_info;
/* MCM/ITI - 1/00 */
extern int PL_insert_intrinsics;

/* HCH 6/14/04: checking in MICRO '04 object globalization and id marking */
extern int PL_globalize_lvars;
extern int PL_mark_glob_objids;

/* JWS 20000517 */

extern int PL_native_int_reg_ctype;
extern int PL_native_int_reg_mtype;
extern int PL_native_int_size;
extern int PL_native_int_align;
extern int PL_native_order;

extern int PL_gen_improved_bitfields;
extern int PL_gen_compliant_struct_return;

extern int PL_annotate_bitwidths;

extern int PL_verbose;

/* BCC - 5/20/99 */
extern L_Oper *global_new_oper;

/*------------------------------------------------------------------------*/
/*
 *	SOME USEFUL FUNCTIONS USED INTERNALLY BY LCODE GENERATOR.
 */
extern void PL_pcode2lcode_type (Key, M_Type, int);

extern void PL_structure_info (Key, int *, int *);

extern void PL_structure_field_info (Key, char *fname, long *offset,
				     M_Type mtype, int *is_bit, int *bit_offset,
				     ITuintmax * bit_mask, int *length);


/*
 *	Be aware that Mtype specifies size and align in bits.
 *	Also the alignment of bit fields is not adjusted to
 *	integer operations. 
 */

extern int PL_reg_ctype (int type, int unsign);
extern int PL_parm_ctype (int type, int unsign);

/*========================================================================*/
/** h_lcode_data.c **/
#define PL_DEFAULT_LABEL	"_"

/*========================================================================*/
/** h_lcode_func.c **/

#define PL_MAX_LOCAL_VAR	3072
#define PL_MAX_PARAM_VAR	128

/*
 *	The compiler must decide when to use hashing jump
 *	to implement a switch.
 */

#define PL_MIN_HASH_JUMP_WEIGHT		5.0
#define PL_MAX_BR_SEQUENCE_WEIGHT	6.0
#define PL_MIN_HASH_JUMP_CASE		5
#define PL_MAX_HASH_JUMP_SIZE		512
#define PL_SWITCH_MISS_ADDRESS		-1

extern int PL_next_oper_id (void);
extern int PL_next_reg_id (void);

extern int PL_find_local_var (Key key, M_Type mtype, int *in_reg,
			      int *reg_id, char **base_macro, int *offset);
extern int PL_find_param_var (Key key, M_Type mtype, int *in_reg,
			      int *reg_id, char **base_macro, int *offset,
			      int *sreg, int *ereg);

/*-----------------------------------------------------------------------*/

/* PL_Operand types */

#define PL_CB		1
#define PL_R		2
#define PL_L		3
#define PL_I		4
#define PL_F		5
#define PL_F2		6
#define PL_S		7
#define PL_MAC		8

typedef struct
{
  int is_func_label;
  short type;
  short data_type;
  int unsign;
  union
  {
    ITintmax i;			/* integer value */
    int cb;			/* basic block id */
    int r;			/* register index */
    char *l;			/* label name */
    Double f;			/* single-precision */
    Double f2;			/* double-precision */
    char *s;			/* string */
    char *mac;			/* macro */
  }
  value;
}
_PL_Operand, *PL_Operand;

/*-----------------------------------------------------------------------*/
/*
 *	The code generation function has a complex recursive structure.
 *	For each node, we generate appropriate code, and return a value.
 *	It is desirable to sometimes look at several nodes and perhaps
 *	combine a few terms. We aim to generate good assembly code in order
 *	to alleviate the later RTL optimization.
 */

/* PL_Ret types */

#define PL_RET_NONE	0	/* result = 0          */

#define PL_RET_SIMPLE	1	/* result = op1        */

#define PL_RET_ADD	2	/* result = op1 + op2  */
#define PL_RET_SUB	3	/* result = op1 - op2  */

#define PL_RET_OR	4	/* result = op1 | op2  */
#define PL_RET_AND	5	/* result = op1 & op2  */
#define PL_RET_XOR	6	/* result = op1 ^ op2  */

#define PL_RET_EQ	7	/* result = op1 == op2 */
#define PL_RET_NE	8	/* result = op1 != op2 */
#define PL_RET_GT	9	/* result = op1 > op2  */
#define PL_RET_GT_U	10	/* result = op1 > op2  */
#define PL_RET_GE	11	/* result = op1 >= op2 */
#define PL_RET_GE_U	12	/* result = op1 >= op2 */
#define PL_RET_LT	13	/* result = op1 < op2  */
#define PL_RET_LT_U	14	/* result = op1 < op2  */
#define PL_RET_LE	15	/* result = op1 <= op2 */
#define PL_RET_LE_U	16	/* result = op1 <= op2 */
#define PL_RET_LIST     17      /* result = oplist[0], oplist[1], ... */ 

typedef struct _PL_Ret
{
  short type;
  _PL_Operand op1;
  _PL_Operand op2;
  /* Added to handle passing structs through regs */
  int sreg, ereg;
  /* KVM : Added to support long long */
  _PL_Operand *oplist;
  int num_operands;
}
_PL_Ret, *PL_Ret;

/*========================================================================*/
/*
 *	SPECIAL FUNCTIONS.
 */

#define HCfn_PREFIX                     "__I_"
#define HCfn_PREFIX_LENGTH              4
#define HCfn_SELECT_I			"__I_select_i__"
#define HCfn_SELECT_F			"__I_select_f__"
#define HCfn_SELECT_F2			"__I_select_f2__"
#define HCfn_REV			"__I_rev__"
#define HCfn_BIT_POS			"__I_bit_pos__"
#define HCfn_ABS_I			"__I_abs_i__"
#define HCfn_ABS_F			"__I_abs_f__"
#define HCfn_ABS_F2			"__I_abs_f2__"
#define HCfn_CO_PROC			"__I_coproc__"
#define HCfn_FETCH_AND_ADD		"__I_fetch_add__"
#define HCfn_FETCH_AND_OR		"__I_fetch_or__"
#define HCfn_FETCH_AND_AND		"__I_fetch_and__"
#define HCfn_FETCH_AND_ST		"__I_fetch_st__"
#define HCfn_FETCH_AND_COND_ST		"__I_fetch_cond_st__"

/*========================================================================*/

/*========================================================================*/

/*========================================================================*/

/*
 *	EXPORT VARIABLES & FUNCTIONS.
 */

extern void PL_gen_lcode_include (char *);
extern void PL_gen_lcode_struct (L_Datalist *, StructDcl);
extern void PL_gen_lcode_union (L_Datalist *, UnionDcl);
extern void PL_gen_lcode_enum (L_Datalist *, EnumDcl);
extern void PL_gen_lcode_var (L_Datalist *, VarDcl);
extern void PL_gen_lcode_func (FuncDcl);

extern int PL_lcode_typesize (Key);

extern int PL_is_function (char *fn_name);
extern int PL_is_not_function (char *fn_name);

extern void PL_ms (L_Datalist * list, char *name);
extern void PL_invalidate_last_ms (void);
extern void PL_gen_var (L_Datalist * list, VarDcl var);
extern char *PL_get_string_label (char *);

extern void PL_gen_func (FuncDcl fn);
extern char *PL_read_attr_name_from_pragma_str (char *pragmastr, char **name);
extern char *PL_read_attr_field_from_pragma_str (char *pragmastr,
						 char **string,
						 long int *integer,
						 double *real,
						 char **delim_type);

extern L_Attr * PL_gen_single_attr_from_pragma (L_Attr **attr_list, 
						Pragma pragma);
extern L_Attr *PL_gen_attr_from_pragma (Pragma pragma);
extern L_Attr *PL_gen_attr (char *name, int value);
extern int PL_uses_pointer_operand (Expr expr);
extern L_Attr *PL_gen_pointer_attr (Expr expr);
extern L_Attr *PL_gen_if_attr (Pragma pragma, Expr expr);

extern void PL_gen_rcmp (L_Cb * cb, PL_Operand dest,
			 PL_Operand src1, PL_Operand src2,
			 ITuint8 com, int unsign, L_Attr * attr);
extern void PL_gen_cbr (L_Cb * src_cb, L_Cb *dst_cb, 
			PL_Operand op1, PL_Operand op2,
			ITuint8 com, int unsign, L_Attr * attr);

extern void PL_gen_cast (L_Cb * cb, PL_Operand dest, PL_Operand src, 
			 int cast_opc);

extern void iso_default_promotion (M_Type rtype,
				   int type1, int unsign1,
				   int type2, int unsign2);

extern int default_unary_promotion (int type);


extern void PL_ret_mulC (L_Cb * cb, PL_Ret ret, int n);
extern void _ret_add (FILE * F, PL_Ret sum, PL_Ret x, PL_Ret y);

/* Exported from pl_gen.c -- top level Lcode generation */

extern void PLI_gen_data (L_Cb * cb, Expr expr, PL_Ret ret);
extern int PLI_gen_addr (L_Cb * cb, Expr expr, PL_Ret ret);
extern void PLI_simplify (L_Cb * cb, PL_Ret ret);

extern char * PL_fmt_var_name(char *label, Key var_key);
extern char * PL_mangle_name(char *label, Key key);

extern void PL_new_register (PL_Operand op, int reg_id, int type, int unsign);
extern void PL_new_cb (PL_Operand op, int cb_id);
extern void PL_new_label (PL_Operand op, char *label, int is_func);
extern void PL_new_macro (PL_Operand op, char *name, int type, int unsign);

extern void PL_new_int_const (PL_Operand op, ITintmax value, int unsign,
			      int type);

#define PL_new_char(o,v,u)  PL_new_int_const((o),(v),M_TYPE_CHAR,(u))
#define PL_new_short(o,v,u) PL_new_int_const((o),(v),M_TYPE_SHORT,(u))
#define PL_new_int(o,v,u)   PL_new_int_const((o),(v),M_TYPE_INT,(u))
#define PL_new_long(o,v,u)  PL_new_int_const((o),(v),M_TYPE_LONG,(u))
#define PL_new_llong(o,v,u) PL_new_int_const((o),(v),M_TYPE_LLONG,(u))

extern void PL_new_pointer (PL_Operand op, ITintmax value);
extern void PL_new_float (PL_Operand op, double value);
extern void PL_new_double (PL_Operand op, double value);
extern void PL_new_string (PL_Operand op, char *str);

extern L_Operand *PL_operand (PL_Operand op);
extern void PL_gen_block_mov (L_Cb * cb, Expr expr, PL_Operand dest,
			      int dest_offset, PL_Operand src, int src_offset,
			      int type_size,int type_align,
			      L_Attr * attr1, int sreg,
			      int ereg, int load_from_reg);
extern void PL_gen_mov (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			L_Attr * attr1);
extern L_Oper *PL_gen_load (L_Cb * cb, Expr expr, PL_Operand dest,
			    PL_Operand src1, PL_Operand src2, int unsign,
			    L_Attr * attr1);
extern L_Oper *PL_gen_store (L_Cb * cb, Expr expr, PL_Operand src1,
			     PL_Operand src2, PL_Operand src3, int data_type,
			     L_Attr * attr1);
extern void PL_gen_add (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, int unsign, L_Attr * attr1);
extern void PL_gen_sub (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, int unsign, L_Attr * attr1);
extern void PL_gen_mul (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, M_Type mtype, int unsign, 
			L_Attr * attr1);
extern void PL_gen_div (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, int mtype_type, int is_unsigned,
			L_Attr * attr1);
extern void PL_gen_mod (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, int mtype_type, int is_unsigned,
			L_Attr * attr1);
extern void PL_gen_abs (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			L_Attr * attr1);
extern void PL_gen_logic (L_Cb * cb, int plop,
			  PL_Operand dest, PL_Operand src1,
			  PL_Operand src2, L_Attr * attr1);
extern void PL_gen_lsl (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, L_Attr * attr1);
extern void PL_gen_lsr (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, L_Attr * attr1);
extern void PL_gen_asr (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			PL_Operand src2, L_Attr * attr1);
extern void PL_gen_extract_bits (L_Cb * cb, PL_Operand dest, PL_Operand src1,
				 PL_Operand src2, PL_Operand src3,
				 int unsign);
extern void PL_gen_deposit_bits (L_Cb * cb, PL_Operand dest, PL_Operand src1,
				 PL_Operand src2, PL_Operand src3,
				 PL_Operand src4);
extern void PL_gen_eq (L_Cb * cb, PL_Operand dest, PL_Operand src1,
		       PL_Operand src2, Expr operand_expr, L_Attr * attr1);
extern void PL_gen_ne (L_Cb * cb, PL_Operand dest, PL_Operand src1,
		       PL_Operand src2, L_Attr * attr1);
extern void PL_gen_lt (L_Cb * cb, PL_Operand dest, PL_Operand src1,
		       PL_Operand src2, int unsign, L_Attr * attr1);
extern void PL_gen_le (L_Cb * cb, PL_Operand dest, PL_Operand src1,
		       PL_Operand src2, int unsign, L_Attr * attr1);
extern void PL_gen_ge (L_Cb * cb, PL_Operand dest, PL_Operand src1,
		       PL_Operand src2, int unsign, L_Attr * attr1);
extern void PL_gen_gt (L_Cb * cb, PL_Operand dest, PL_Operand src1,
		       PL_Operand src2, int unsign, L_Attr * attr1);
extern void PL_gen_f2_f (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			 L_Attr * attr1);
extern void PL_gen_i_f (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			L_Attr * attr1);
extern void PL_gen_f_f2 (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			 L_Attr * attr1);
extern void PL_gen_i_f2 (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			 L_Attr * attr1);
extern void PL_gen_f2_i (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			 L_Attr * attr1);
extern void PL_gen_f_i (L_Cb * cb, PL_Operand dest, PL_Operand src1,
			L_Attr * attr1);
extern void PL_gen_alloc (L_Cb * cb, PL_Operand dest, int size, int align);
extern L_Oper *PL_gen_jsr (L_Cb * cb, Expr expr, PL_Operand src,
			   int argc, L_Attr * attr1);
extern void PL_gen_lcode_select_i (L_Cb * cb, PL_Operand dest,
				   _PL_Operand src[], int n_src);
extern void PL_gen_lcode_select_f (L_Cb * cb, PL_Operand dest,
				   _PL_Operand src[], int n_src);
extern void PL_gen_lcode_select_f2 (L_Cb * cb, PL_Operand dest,
				    _PL_Operand src[], int n_src);
extern void PL_gen_lcode_rev (L_Cb * cb, PL_Operand dest,
			      _PL_Operand src[], int n_src);
extern void PL_gen_lcode_bit_pos (L_Cb * cb, PL_Operand dest,
				  _PL_Operand src[], int n_src);
extern void PL_gen_lcode_abs_i (L_Cb * cb, PL_Operand dest,
				_PL_Operand src[], int n_src);
extern void PL_gen_lcode_abs_f (L_Cb * cb, PL_Operand dest,
				_PL_Operand src[], int n_src);
extern void PL_gen_lcode_abs_f2 (L_Cb * cb, PL_Operand dest,
				 _PL_Operand src[], int n_src);
extern void PL_gen_lcode_sqrt_f2 (L_Cb * cb, PL_Operand dest,
				  _PL_Operand src[], int n_src);
extern void PL_gen_lcode_min_f2 (L_Cb * cb, PL_Operand dest,
				 _PL_Operand src[], int n_src);
extern void PL_gen_lcode_max_f2 (L_Cb * cb, PL_Operand dest,
				 _PL_Operand src[], int n_src);
extern void PL_gen_lcode_co_proc (L_Cb * cb, PL_Operand dest,
				  _PL_Operand src[], int n_src);
extern void PL_gen_lcode_fetch_add (L_Cb * cb, PL_Operand dest,
				    _PL_Operand src[], int n_src);
extern void PL_gen_lcode_fetch_or (L_Cb * cb, PL_Operand dest,
				   _PL_Operand src[], int n_src);
extern void PL_gen_lcode_fetch_and (L_Cb * cb, PL_Operand dest,
				    _PL_Operand src[], int n_src);
extern void PL_gen_lcode_fetch_st (L_Cb * cb, PL_Operand dest,
				   _PL_Operand src[], int n_src);
extern void PL_gen_lcode_fetch_cond_st (L_Cb * cb, PL_Operand dest,
					_PL_Operand src[], int n_src);

/* LCW - added some functions for preserving debugging information - 4/15/96 */
extern void PL_gen_struct (L_Datalist * list, StructDcl st);
extern void PL_gen_union (L_Datalist * list, UnionDcl un);
extern void PL_gen_enum (L_Datalist * list, EnumDcl en);
extern L_Type *L_gen_type (Key htype);
extern L_Dcltr *L_gen_dcltr (TypeDcl hdcl);

extern L_Datalist *L_datalist;
extern L_Datalist *L_hash_datalist;
extern L_Datalist *L_string_datalist;

extern void PL_init_strings (void);
extern void PL_forget_strings (void);
extern void PL_deinit_strings (void);

extern void PL_gen_dep_arcs (L_Oper * oper, Expr expr, PL_Operand src1,
			     PL_Operand src2, int is_store);
extern void L_annotate_syncs (L_Func * fn);
extern void PL_moving_param_into_local (PL_Operand dest, PL_Operand src);	/* CWL - 01/10/02 */
extern int eval_mtype (int type);	/* CWL - 01/10/02 */

extern int is_integer (int type);
extern int is_float (int type);
extern int is_double (int type);
extern int PL_is_HFA_type (Key type);

extern void PL_cast (L_Cb * cb, PL_Operand dest, PL_Operand src,
		     int type, int unsign);

L_Operand *PL_gen_operand (PL_Operand op);
L_Expr *L_new_addr (char *label, int offset, Key type);
L_Expr *L_new_addr_no_underscore (char *label, int offset);
int PLM_valid_register_type (int type);
int PL_intrinsic_intrinsify (char *fn_name, PL_Ret result,
			     Expr * temp_expr, _PL_Ret ret,
			     L_Cb * cb, PL_Operand dest, _PL_Operand src[],
			     int n_src);

HashTable operandHash;

void
PreProcess (void);
void
ProcessStruct (StructDcl st);
void
ProcessUnion (UnionDcl un);
void
ProcessGlobalVar (VarDcl var);
void
ProcessFuncDcl (FuncDcl func);
void
PrintData (void);

Extension 
P_alloc_vartbl_entry(void);
Extension 
P_free_vartbl_entry(Extension e); 

#define DEBUG_DepL		0

extern int expr_ext_deplist_idx;

extern int PL_is_link_multi (Key type, Key *pstype);
extern int PL_is_aggr_type (Key type, Key *pstype);

extern void PL_debug (char *fmt, ...);

#define PLD if (PL_verbose) PL_debug

#endif
