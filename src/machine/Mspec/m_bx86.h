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
 *  File:  m_bx86.h
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

#ifndef M_BX86_H
#define M_BX86_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <machine/m_spec.h>

/*
 * Declarations for processor models
 */
enum
{
  M_B_486 = 0,
  M_B_PENTIUM,
  M_B_PPRO,
  M_B_PENTIUM_MMX,
  M_B_PENTIUM_II,
  M_B_K5,
  M_B_K6,
  M_B_K6_MMX,
  M_B_K6_PLUS
};

enum
{
  LBX86_MAC_CALLEE_I = 100,
  LBX86_MAC_CALLEE_F,

/**********************************************************************
    BX86 Macros
***********************************************************************/

  LBX86_MAC_OSZAPC_FLAGS,    /* All normal arithmetic flags                  */
  LBX86_MAC_C_FLAGS,	     /* Just the carry flag (CF)                     */
  LBX86_MAC_OSZAP_FLAGS,     /* All arithmetic flags except carry            */
  LBX86_MAC_D_FLAGS,	     /* Direction flag (DF) for string instructions  */
  LBX86_MAC_OC_FLAGS,	     /* Just overflow & carry flags (OF & CF)        */
  LBX86_MAC_SZAPC_FLAGS,     /* All arithmetic flags except overflow         */
  LBX86_MAC_OSZPC_FLAGS,     /* All arithmetic flags except aux carry (AF)   */
  LBX86_MAC_Z_FLAGS,	     /* Just the zero flag (ZF)                      */
  LBX86_MAC_ZC_FLAGS,	     /* Just zero & carry flags (zF & CF)            */
  LBX86_MAC_OSZ_FLAGS,	     /* Just overflow & sign & zero flags (OF&SF&ZF) */
  LBX86_MAC_OS_FLAGS,	     /* Just overflow & sign flags (OF & SF)         */
  LBX86_MAC_O_FLAGS,	     /* Just overflow                                */
  LBX86_MAC_S_FLAGS,	     /* Just sign                                    */
  LBX86_MAC_A_FLAGS,	     /* Just aux carry                               */
  LBX86_MAC_P_FLAGS,	     /* Just parity                                  */
  LBX86_MAC_ALL_GP_REG32,    /* All 32-bit general-purpose registers         */
  LBX86_MAC_ALL_GP_REG16,    /* All 16-bit general-purpose registers         */
  LBX86_MAC_EAX,	     /* EAX register ----                            */
  LBX86_MAC_AX,		     /*  AX register   --                            */
  LBX86_MAC_AH,		     /*  AH register   -                             */
  LBX86_MAC_AL,		     /*  AL register    -                            */
  LBX86_MAC_EAXH,	     /* EAXH pseudo  --                              */
  LBX86_MAC_EBX,	     /* EBX register ----                            */
  LBX86_MAC_BX,		     /*  BX register   --                            */
  LBX86_MAC_BH,		     /*  BH register   -                             */
  LBX86_MAC_BL,		     /*  BL register    -                            */
  LBX86_MAC_EBXH,	     /* EBXH pseudo  --                              */
  LBX86_MAC_ECX,	     /* ECX register ----                            */
  LBX86_MAC_CX,		     /*  CX register   --                            */
  LBX86_MAC_CH,		     /*  CH register   -                             */
  LBX86_MAC_CL,		     /*  CL register    -                            */
  LBX86_MAC_ECXH,	     /* ECXH pseudo  --                              */
  LBX86_MAC_EDX,	     /* EDX register ----                            */
  LBX86_MAC_DX,		     /*  DX register   --                            */
  LBX86_MAC_DH,		     /*  DH register   -                             */
  LBX86_MAC_DL,		     /*  DL register    -                            */
  LBX86_MAC_EDXH,	     /* EDXH pseudo  --                              */
  LBX86_MAC_ESI,	     /* ESI register ----                            */
  LBX86_MAC_SI,		     /*  SI register   --                            */
  LBX86_MAC_ESIH,	     /* ESIH pseudo  --                              */
  LBX86_MAC_EDI,	     /* EDI register ----                            */
  LBX86_MAC_DI,		     /*  DI register   --                            */
  LBX86_MAC_EDIH,	     /* EDIH pseudo  --                              */
  LBX86_MAC_EBP,	     /* EBP register ----                            */
  LBX86_MAC_BP,		     /*  BP register   --                            */
  LBX86_MAC_EBPH,	     /* EBPH pseudo  --                              */
  LBX86_MAC_ESP,	     /* ESP register ----                            */
  LBX86_MAC_SP,		     /*  SP register   --                            */
  LBX86_MAC_ESPH,	     /* ESPH pseudo  --                              */
  LBX86_MAC_CS,		     /* CS segment register                          */
  LBX86_MAC_SS,		     /* SS segment register                          */
  LBX86_MAC_DS,		     /* DS segment register                          */
  LBX86_MAC_ES,		     /* ES segment register                          */
  LBX86_MAC_FS,		     /* FS segment register                          */
  LBX86_MAC_GS,		     /* GS segment register                          */
  LBX86_MAC_ALL_FST,	     /* Entire FP Stack                              */
  LBX86_MAC_ST0,	     /* FP Stack                                     */
  LBX86_MAC_ST1,
  LBX86_MAC_ST2,
  LBX86_MAC_ST3,
  LBX86_MAC_ST4,
  LBX86_MAC_ST5,
  LBX86_MAC_ST6,
  LBX86_MAC_ST7,
  LBX86_MAC_ALL_MM,	     /* all MMX(tm) registers                        */
  LBX86_MAC_MM0,	     /* MMX(tm) registers                            */
  LBX86_MAC_MM1,
  LBX86_MAC_MM2,
  LBX86_MAC_MM3,
  LBX86_MAC_MM4,
  LBX86_MAC_MM5,
  LBX86_MAC_MM6,
  LBX86_MAC_MM7,
  LBX86_MAC_ADDR,	     /* Specifies memory address in src's 3-6        */
  LBX86_MAC_FPSW,
  LBX86_MAC_FPCW
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


typedef struct Lbx86_Ext_Fields
{
  unsigned short flags;
  char mem_size;
  char seg_ovr_popval;
}
Lbx86_Ext_Fields;


#define LBX86_EXT(oper) ((Lbx86_Ext_Fields *)(oper->ext))

#define LBX86_SEG_OVR(oper) (LBX86_EXT(oper)->seg_ovr_popval & 0xF)
#define LBX86_POPVAL(oper) ((LBX86_EXT(oper)->seg_ovr_popval >> 4) & 0xF)
#define LBX86_ENCODE_SEG_OVR_POPVAL(seg_ovr, popval) ((char)((popval << 4) | \
                                                             (seg_ovr & 0xF)))

#define LBX86_EXTRACT_GENOPC(popc) ((int)(((unsigned)(popc)) >> 4))
#define LBX86_GENOPC(oper) (LBX86_EXTRACT_GENOPC(oper->proc_opc))
#define LBX86_EXTRACT_VARIANT(popc) ((int)(((unsigned)(popc)) & 0xF))
#define LBX86_VARIANT(oper) (LBX86_EXTRACT_VARIANT(oper->proc_opc))

#define LBX86_ENCODE_POPC(genopc, variant) ((int)((genopc << 4) | variant))

#define LBX86_ADDR_BASE  3
#define LBX86_ADDR_INDEX 4
#define LBX86_ADDR_SCALE 5
#define LBX86_ADDR_DISP  6

/* need to define our proc_opc's ... */


#ifdef __cplusplus
extern "C"
{
#endif

  extern void M_define_macros_bx86 (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_macro_name_bx86 (int id);
  extern int M_bx86_fragile_macro (int macro_value);
  extern int M_bx86_subroutine_call (int opc);
  extern void M_define_opcode_name_bx86 (STRING_Symbol_Table * sym_tbl);
  extern char *M_get_opcode_name_bx86 (int id);

  extern int M_bx86_type_size (int mtype);
  extern int M_bx86_type_align (int mtype);
  extern void M_bx86_void (M_Type type);
  extern void M_bx86_bit_long (M_Type type, int n);
  extern void M_bx86_bit_int (M_Type type, int n);
  extern void M_bx86_bit_short (M_Type type, int n);
  extern void M_bx86_bit_char (M_Type type, int n);
  extern void M_bx86_char (M_Type type, int unsign);
  extern void M_bx86_short (M_Type type, int unsign);
  extern void M_bx86_int (M_Type type, int unsign);
  extern void M_bx86_long (M_Type type, int unsign);
  extern void M_bx86_float (M_Type type, int unsign);
  extern void M_bx86_double (M_Type type, int unsign);
  extern void M_bx86_pointer (M_Type type);
  extern int M_bx86_eval_type (M_Type type, M_Type ntype);
  extern int M_bx86_eval_type2 (M_Type type, M_Type ntype);
  extern int M_bx86_call_type (M_Type type, M_Type ntype);
  extern int M_bx86_call_type2 (M_Type type, M_Type ntype);
  extern void M_bx86_array_layout (M_Type type, int *offset);
  extern int M_bx86_array_align (M_Type type);
  extern int M_bx86_array_size (M_Type type, int dim);
  extern void M_bx86_union_layout (int n, _M_Type * type, int *offset,
				   int *bit_offset);
  extern int M_bx86_union_align (int n, _M_Type * type);
  extern int M_bx86_union_size (int n, _M_Type * type);
  extern void M_bx86_struct_layout (int n, _M_Type * type, int *base,
				    int *bit_offset);
  extern int M_bx86_struct_align (int n, _M_Type * type);
  extern int M_bx86_struct_size (int n, _M_Type * type, int struct_align);
  extern int M_bx86_fnvar_layout (int n, _M_Type * type, long int *offset,
				  int *mode, int *reg, int *paddr,
				  char **macro, int *su_sreg, int *su_ereg,
				  int *pcount, int is_st, int purpose);
  extern int M_bx86_lvar_layout (int n, _M_Type * type, long int *offset,
				 char **base_macro);
  extern int M_bx86_no_short_int (void);
  extern int M_bx86_layout_order (void);
  extern void M_bx86_cb_label_name (char *fn, int cb, char *line, int len);
  extern int M_bx86_is_cb_label (char *label, char *fn, int *cb);
  extern void M_bx86_jumptbl_label_name (char *fn, int tbl_id, char *line,
					 int len);
  extern int M_bx86_is_jumptbl_label (char *label, char *fn, int *tbl_id);
  extern int M_bx86_structure_pointer (int purpose);
  extern int M_bx86_return_register (int type, int purpose);
  extern char *M_bx86_fn_label_name (char *label,
				     int (*is_func) (char *is_func_label));
  extern char *M_bx86_fn_name_from_label (char *label);
  extern void M_set_model_bx86 (char *model_name);
  extern int M_oper_supported_in_arch_bx86 (int opc);
  extern int M_num_oper_required_for_bx86 (L_Oper * oper, char *name);
  extern int M_is_stack_operand_bx86 (L_Operand * operand);
  extern int M_is_unsafe_macro_bx86 (L_Operand * operand);
  extern int M_operand_type_bx86 (L_Operand * operand);
  extern int M_conflicting_operands_bx86 (L_Operand * operand,
					  L_Operand ** conflict_array,
					  int len, int prepass);
  extern void M_get_memory_operands_bx86 (int *first, int *number,
					  int proc_opc);
  extern int M_memory_access_size_bx86 (L_Oper * op);
  extern int M_get_data_type_bx86 (L_Oper * op);
  extern int M_num_registers_bx86 (int ctype);
  extern int M_is_implicit_memory_op_bx86 (L_Oper * oper);

#ifdef __cplusplus
}
#endif

#endif
