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
 *      File :          l_code.c
 *      Description :   LCODE structure definition.
 *      Original: Po-hua Chang, Wen-mei Hwu June 1990
 *      Modified : Roland Ouellette, Mon Jul  9 13:57:05 1990
 *              added L_n_ reg and oper and code to get them right after
 *              reading in a function.
 *      Modified : Roger A. Bringmann April, 1991
 *              added extensions to support mcode.
 *      Modified : Scott A. Mahlke February 1992
 *              added support for predicated execution, format changed
 *      Revised : Scott A. Mahlke, Roger A. Bringmann, January 1993
 *              dynamic allocation of all data structures, unlimited 
 *              src/dest/pred operands, reduce space requirements, interface 
 *              to MDES.
 *      Modified : Scott A. Mahlke March 1994
 *              added oper hash table junk
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <string.h>
#include <Lcode/l_main.h>
#include <Lcode/l_region.h>
#include <Lcode/l_event.h>
#include <Lcode/l_appendix.h>
#include "l_accspec.h"
#include <machine/m_spec.h>

#define DEBUG_CHECK_HASH_TBL 0

/*========================================================================*/
/*
 *      Global vars
 */
/*========================================================================*/

#if 0
int sym_tbl = -1;
#endif
int L_token_type = -1;
int L_generation_info_printed = 0;

L_Func *L_fn = NULL;            /* Current active function */
L_Data *L_data = NULL;          /* Current active data */
L_Datalist *L_data_list = NULL; /* Current active datalist */
L_Event *L_event_list = NULL;   /* Current active event */
L_Event *L_result_list = NULL;  /* Current active result */

/* vars set in L_color_cb */
L_Oper **L_cnt_oper = NULL;
L_Flow **L_cnt_flow = NULL;
int L_n_color_entries_alloc = 0;
int L_n_cnt_oper = 0;

int L_func_read = 0;

char *L_file_arch = 0;
char *L_file_model = 0;
char *L_file_lmdes = 0;


/*
 * REH 6/21/95 - move to new dynamic symbol tables
 *           with better hashing function.
 */
STRING_Symbol_Table *L_string_table = NULL;
STRING_Symbol_Table *L_opcode_symbol_table = NULL;
STRING_Symbol_Table *L_macro_symbol_table = NULL;
STRING_Symbol_Table *L_ms_symbol_table = NULL;
STRING_Symbol_Table *L_ctype_symbol_table = NULL;
STRING_Symbol_Table *L_ptype_symbol_table = NULL;
STRING_Symbol_Table *L_operand_symbol_table = NULL;
STRING_Symbol_Table *L_expr_symbol_table = NULL;
STRING_Symbol_Table *L_lcode_symbol_table = NULL;
STRING_Symbol_Table *L_cmp_compl_symbol_table = NULL;

INT_Symbol_Table *L_macro_id_symbol_table = NULL;

/*========================================================================*/
/*
 *      Symbol table related functions
 */
/*========================================================================*/

void
L_init_symbol ()
{
  L_string_table = L_new_symbol_table ("Lcode strings", 2048);
  L_opcode_symbol_table = L_new_symbol_table ("Opcode Symbols", 512);
  L_cmp_compl_symbol_table = L_new_symbol_table ("Pred Compare Completers",
                                                 64);
  L_macro_symbol_table = L_new_symbol_table ("Macro Symbols", 512);
  /* To support intrinsics -ITI/JWJ 7/99 */
  L_macro_id_symbol_table = L_new_id_symbol_table ("Macro IDs", 512);
  L_ms_symbol_table = L_new_symbol_table ("MS Symbols", 8);
  L_ctype_symbol_table = L_new_symbol_table ("Ctype Symbols", 64);
  L_ptype_symbol_table = L_new_symbol_table ("Ptype Symbols", 64);
  L_operand_symbol_table = L_new_symbol_table ("Operand Symbols", 16);
  L_expr_symbol_table = L_new_symbol_table ("Expr Symbols", 64);
  L_lcode_symbol_table = L_new_symbol_table ("Lcode Symbols", 64);

  L_init_appendix_symbol_tables ();

#if 0
  sym_tbl = C_open (L_MAX_SYMBOL, C_MATCH_BY_EXACT_TYPE);

  if (sym_tbl == -1)
    L_punt ("failed to allocate symbol table");
#endif

  /*
   *  define macro symbols.
   */
  L_add_symbol (L_macro_symbol_table, L_MACRO_P0, L_MAC_P0);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P1, L_MAC_P1);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P2, L_MAC_P2);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P3, L_MAC_P3);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P4, L_MAC_P4);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P5, L_MAC_P5);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P6, L_MAC_P6);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P7, L_MAC_P7);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P8, L_MAC_P8);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P9, L_MAC_P9);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P10, L_MAC_P10);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P11, L_MAC_P11);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P12, L_MAC_P12);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P13, L_MAC_P13);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P14, L_MAC_P14);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P15, L_MAC_P15);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P16, L_MAC_P16);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P17, L_MAC_P17);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P18, L_MAC_P18);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P19, L_MAC_P19);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P20, L_MAC_P20);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P21, L_MAC_P21);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P22, L_MAC_P22);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P23, L_MAC_P23);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P24, L_MAC_P24);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P25, L_MAC_P25);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P26, L_MAC_P26);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P27, L_MAC_P27);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P28, L_MAC_P28);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P29, L_MAC_P29);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P30, L_MAC_P30);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P31, L_MAC_P31);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P32, L_MAC_P32);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P33, L_MAC_P33);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P34, L_MAC_P34);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P35, L_MAC_P35);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P36, L_MAC_P36);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P37, L_MAC_P37);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P38, L_MAC_P38);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P39, L_MAC_P39);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P40, L_MAC_P40);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P41, L_MAC_P41);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P42, L_MAC_P42);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P43, L_MAC_P43);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P44, L_MAC_P44);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P45, L_MAC_P45);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P46, L_MAC_P46);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P47, L_MAC_P47);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P48, L_MAC_P48);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P49, L_MAC_P49);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P50, L_MAC_P50);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P51, L_MAC_P51);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P52, L_MAC_P52);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P53, L_MAC_P53);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P54, L_MAC_P54);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P55, L_MAC_P55);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P56, L_MAC_P56);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P57, L_MAC_P57);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P58, L_MAC_P58);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P59, L_MAC_P59);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P60, L_MAC_P60);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P61, L_MAC_P61);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P62, L_MAC_P62);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P63, L_MAC_P63);
  L_add_symbol (L_macro_symbol_table, L_MACRO_P64, L_MAC_P64);
  L_add_symbol (L_macro_symbol_table, L_MACRO_TR_PTR, L_MAC_TR_PTR);
  L_add_symbol (L_macro_symbol_table, L_MACRO_TR_MARK, L_MAC_TR_MARK);
  L_add_symbol (L_macro_symbol_table, L_MACRO_TR_TEMP, L_MAC_TR_TEMP);
  L_add_symbol (L_macro_symbol_table, L_MACRO_PRED_ALL, L_MAC_PRED_ALL);
  L_add_symbol (L_macro_symbol_table, L_MACRO_SAFE_MEM, L_MAC_SAFE_MEM);
  L_add_symbol (L_macro_symbol_table, L_MACRO_TM_TYPE, L_MAC_TM_TYPE);
  L_add_symbol (L_macro_symbol_table, L_MACRO_LOCAL_SIZE, L_MAC_LOCAL_SIZE);
  L_add_symbol (L_macro_symbol_table, L_MACRO_PARAM_SIZE, L_MAC_PARAM_SIZE);
  L_add_symbol (L_macro_symbol_table, L_MACRO_SWAP_SIZE, L_MAC_SWAP_SIZE);
  L_add_symbol (L_macro_symbol_table, L_MACRO_RET_TYPE, L_MAC_RET_TYPE);

  L_add_symbol (L_macro_symbol_table, L_MACRO_IP, L_MAC_IP);
  L_add_symbol (L_macro_symbol_table, L_MACRO_OP, L_MAC_OP);
  L_add_symbol (L_macro_symbol_table, L_MACRO_LV, L_MAC_LV);
  L_add_symbol (L_macro_symbol_table, L_MACRO_RS, L_MAC_RS);
  L_add_symbol (L_macro_symbol_table, L_MACRO_SP, L_MAC_SP);
  L_add_symbol (L_macro_symbol_table, L_MACRO_FP, L_MAC_FP);

  L_add_symbol (L_macro_symbol_table, L_MACRO_CR, L_MAC_CR);

  L_add_symbol (L_macro_symbol_table, L_MACRO_RETADDR, L_MAC_RETADDR);

  /*
   *  define ms symbols.
   */

  L_add_symbol (L_ms_symbol_table, "text", L_MS_TEXT);
  L_add_symbol (L_ms_symbol_table, "data", L_MS_DATA);
  L_add_symbol (L_ms_symbol_table, "data1", L_MS_DATA1);
  L_add_symbol (L_ms_symbol_table, "data2", L_MS_DATA2);
  L_add_symbol (L_ms_symbol_table, "sdata", L_MS_SDATA);
  L_add_symbol (L_ms_symbol_table, "sdata1", L_MS_SDATA);
  L_add_symbol (L_ms_symbol_table, "rodata", L_MS_RODATA);
  L_add_symbol (L_ms_symbol_table, "bss", L_MS_BSS);
  L_add_symbol (L_ms_symbol_table, "sbss", L_MS_SBSS);
  L_add_symbol (L_ms_symbol_table, "sync", L_MS_SYNC);

  /*
   *  define ctypes.
   */
  L_add_symbol (L_ctype_symbol_table, "void", L_CTYPE_VOID);
  L_add_symbol (L_ctype_symbol_table, "c", L_CTYPE_CHAR);
  L_add_symbol (L_ctype_symbol_table, "uc", L_CTYPE_UCHAR);
  L_add_symbol (L_ctype_symbol_table, "sh", L_CTYPE_SHORT);
  L_add_symbol (L_ctype_symbol_table, "ush", L_CTYPE_USHORT);
  L_add_symbol (L_ctype_symbol_table, "i", L_CTYPE_INT);
  L_add_symbol (L_ctype_symbol_table, "ui", L_CTYPE_UINT);
  L_add_symbol (L_ctype_symbol_table, "lng", L_CTYPE_LONG);
  L_add_symbol (L_ctype_symbol_table, "ulng", L_CTYPE_ULONG);
  L_add_symbol (L_ctype_symbol_table, "ll", L_CTYPE_LLONG);
  L_add_symbol (L_ctype_symbol_table, "ull", L_CTYPE_ULLONG);
  L_add_symbol (L_ctype_symbol_table, "lll", L_CTYPE_LLLONG);
  L_add_symbol (L_ctype_symbol_table, "ulll", L_CTYPE_ULLLONG);
  L_add_symbol (L_ctype_symbol_table, "pnt", L_CTYPE_POINTER);
  L_add_symbol (L_ctype_symbol_table, "f", L_CTYPE_FLOAT);
  L_add_symbol (L_ctype_symbol_table, "f2", L_CTYPE_DOUBLE);
  L_add_symbol (L_ctype_symbol_table, "cnt", L_CTYPE_CONTROL);
  L_add_symbol (L_ctype_symbol_table, "p", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_ut", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_uf", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_ct", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_cf", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_ot", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_of", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_at", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_af", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_st", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "p_sf", L_CTYPE_PREDICATE);
  L_add_symbol (L_ctype_symbol_table, "b", L_CTYPE_BTR);
  L_add_symbol (L_ctype_symbol_table, "l", L_CTYPE_GLOBAL_ABS);
  L_add_symbol (L_ctype_symbol_table, "l_l_abs", L_CTYPE_LOCAL_ABS);
  L_add_symbol (L_ctype_symbol_table, "l_l_gp", L_CTYPE_LOCAL_GP);
  L_add_symbol (L_ctype_symbol_table, "l_g_abs", L_CTYPE_GLOBAL_ABS);
  L_add_symbol (L_ctype_symbol_table, "l_g_gp", L_CTYPE_GLOBAL_GP);
  L_add_symbol (L_ctype_symbol_table, "s", L_CTYPE_GLOBAL_ABS);
  L_add_symbol (L_ctype_symbol_table, "s_l_abs", L_CTYPE_LOCAL_ABS);
  L_add_symbol (L_ctype_symbol_table, "s_l_gp", L_CTYPE_LOCAL_GP);
  L_add_symbol (L_ctype_symbol_table, "s_g_abs", L_CTYPE_GLOBAL_ABS);
  L_add_symbol (L_ctype_symbol_table, "s_g_gp", L_CTYPE_GLOBAL_GP);
  /* RMR { adding support for vector file type */
  L_add_symbol(L_ctype_symbol_table, "vi", L_CTYPE_VECTOR_INT);
  L_add_symbol(L_ctype_symbol_table, "vf", L_CTYPE_VECTOR_FLOAT);
  L_add_symbol(L_ctype_symbol_table, "vf2", L_CTYPE_VECTOR_DOUBLE);
  L_add_symbol(L_ctype_symbol_table, "vm", L_CTYPE_VECTOR_MASK);
  /* } RMR */

  /*
   *  define ptypes.
   */
  L_add_symbol (L_ptype_symbol_table, "p", L_PTYPE_NULL);
  L_add_symbol (L_ptype_symbol_table, "p_ut", L_PTYPE_UNCOND_T);
  L_add_symbol (L_ptype_symbol_table, "p_uf", L_PTYPE_UNCOND_F);
  L_add_symbol (L_ptype_symbol_table, "p_ct", L_PTYPE_COND_T);
  L_add_symbol (L_ptype_symbol_table, "p_cf", L_PTYPE_COND_F);
  L_add_symbol (L_ptype_symbol_table, "p_ot", L_PTYPE_OR_T);
  L_add_symbol (L_ptype_symbol_table, "p_of", L_PTYPE_OR_F);
  L_add_symbol (L_ptype_symbol_table, "p_at", L_PTYPE_AND_T);
  L_add_symbol (L_ptype_symbol_table, "p_af", L_PTYPE_AND_F);
  L_add_symbol (L_ptype_symbol_table, "p_st", L_PTYPE_SAND_T);
  L_add_symbol (L_ptype_symbol_table, "p_sf", L_PTYPE_SAND_F);
  /*
   *  define operand types.
   */
  L_add_symbol (L_operand_symbol_table, "void", L_OPERAND_VOID);
  L_add_symbol (L_operand_symbol_table, "cb", L_OPERAND_CB);
  L_add_symbol (L_operand_symbol_table, "r", L_OPERAND_REGISTER);
  L_add_symbol (L_operand_symbol_table, "rr", L_OPERAND_RREGISTER);
  L_add_symbol (L_operand_symbol_table, "evr", L_OPERAND_EVR);
  L_add_symbol (L_operand_symbol_table, "l", L_OPERAND_LABEL);
  L_add_symbol (L_operand_symbol_table, "l_l_abs", L_OPERAND_LABEL);
  L_add_symbol (L_operand_symbol_table, "l_l_gp", L_OPERAND_LABEL);
  L_add_symbol (L_operand_symbol_table, "l_g_abs", L_OPERAND_LABEL);
  L_add_symbol (L_operand_symbol_table, "l_g_gp", L_OPERAND_LABEL);
  L_add_symbol (L_operand_symbol_table, "s", L_OPERAND_STRING);
  L_add_symbol (L_operand_symbol_table, "s_l_abs", L_OPERAND_STRING);
  L_add_symbol (L_operand_symbol_table, "s_l_gp", L_OPERAND_STRING);
  L_add_symbol (L_operand_symbol_table, "s_g_abs", L_OPERAND_STRING);
  L_add_symbol (L_operand_symbol_table, "s_g_gp", L_OPERAND_STRING);
  L_add_symbol (L_operand_symbol_table, "c", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "uc", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "sh", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "ush", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "i", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "ui", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "lng", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "ulng", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "ll", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "ull", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "lll", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "ulll", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "pnt", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "f", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "f2", L_OPERAND_IMMED);
  L_add_symbol (L_operand_symbol_table, "mac", L_OPERAND_MACRO);
  /*
   *  define expr types.
   */
  L_add_symbol (L_expr_symbol_table, "add", L_EXPR_ADD);
  L_add_symbol (L_expr_symbol_table, "sub", L_EXPR_SUB);
  L_add_symbol (L_expr_symbol_table, "mul", L_EXPR_MUL);
  L_add_symbol (L_expr_symbol_table, "div", L_EXPR_DIV);
  L_add_symbol (L_expr_symbol_table, "neg", L_EXPR_NEG);
  L_add_symbol (L_expr_symbol_table, "com", L_EXPR_COM);
  L_add_symbol (L_expr_symbol_table, "i", L_EXPR_INT);
  L_add_symbol (L_expr_symbol_table, "f", L_EXPR_FLOAT);
  L_add_symbol (L_expr_symbol_table, "f2", L_EXPR_DOUBLE);
  L_add_symbol (L_expr_symbol_table, "l", L_EXPR_LABEL);
  L_add_symbol (L_expr_symbol_table, "s", L_EXPR_STRING);
  /*
   *  define top level list types.
   */
  L_add_symbol (L_lcode_symbol_table, "ms", L_INPUT_MS);
  L_add_symbol (L_lcode_symbol_table, "void", L_INPUT_VOID);
  L_add_symbol (L_lcode_symbol_table, "byte", L_INPUT_BYTE);
  L_add_symbol (L_lcode_symbol_table, "word", L_INPUT_WORD);
  L_add_symbol (L_lcode_symbol_table, "long", L_INPUT_LONG);
  L_add_symbol (L_lcode_symbol_table, "float", L_INPUT_FLOAT);
  L_add_symbol (L_lcode_symbol_table, "double", L_INPUT_DOUBLE);
  L_add_symbol (L_lcode_symbol_table, "align", L_INPUT_ALIGN);
  L_add_symbol (L_lcode_symbol_table, "ascii", L_INPUT_ASCII);
  L_add_symbol (L_lcode_symbol_table, "asciz", L_INPUT_ASCIZ);
  L_add_symbol (L_lcode_symbol_table, "reserve", L_INPUT_RESERVE);
  L_add_symbol (L_lcode_symbol_table, "skip", L_INPUT_SKIP);
  L_add_symbol (L_lcode_symbol_table, "global", L_INPUT_GLOBAL);
  L_add_symbol (L_lcode_symbol_table, "wb", L_INPUT_WB);
  L_add_symbol (L_lcode_symbol_table, "ww", L_INPUT_WW);
  L_add_symbol (L_lcode_symbol_table, "wi", L_INPUT_WI);
  L_add_symbol (L_lcode_symbol_table, "wf", L_INPUT_WF);
  L_add_symbol (L_lcode_symbol_table, "wf2", L_INPUT_WF2);
  L_add_symbol (L_lcode_symbol_table, "ws", L_INPUT_WS);
  L_add_symbol (L_lcode_symbol_table, "function", L_INPUT_FUNCTION);
  L_add_symbol (L_lcode_symbol_table, "cb", L_INPUT_CB);
  L_add_symbol (L_lcode_symbol_table, "end", L_INPUT_END);
  L_add_symbol (L_lcode_symbol_table, "op", L_INPUT_OP);
  L_add_symbol (L_lcode_symbol_table, "OP", L_INPUT_POP);
  L_add_symbol (L_lcode_symbol_table, "element_size", L_INPUT_ELEMENT_SIZE);
  L_add_symbol (L_lcode_symbol_table, "event_list", L_INPUT_EVENT_LIST);
  L_add_symbol (L_lcode_symbol_table, "result_list", L_INPUT_RESULT_LIST);
  L_add_symbol (L_lcode_symbol_table, "region", L_INPUT_REGION);
  L_add_symbol (L_lcode_symbol_table, "region_end", L_INPUT_REGION_END);
  L_add_symbol (L_lcode_symbol_table, "appendix", L_INPUT_APPENDIX);
  L_add_symbol (L_lcode_symbol_table, "Aop", L_INPUT_AOP);
  L_add_symbol (L_lcode_symbol_table, "Acb", L_INPUT_ACB);
  L_add_symbol (L_lcode_symbol_table, "entry", L_INPUT_REGION_ENTRY);
  L_add_symbol (L_lcode_symbol_table, "exit", L_INPUT_REGION_EXIT);
  L_add_symbol (L_lcode_symbol_table, "live_in", L_INPUT_REGION_LIVE_IN);
  L_add_symbol (L_lcode_symbol_table, "live_out", L_INPUT_REGION_LIVE_OUT);
  L_add_symbol (L_lcode_symbol_table, "regalloc", L_INPUT_REGION_REGALLOC);
  /*
   * LCW - top level list types for debugging information -- 9/14/95
   */
  L_add_symbol (L_lcode_symbol_table, "def_struct", L_INPUT_DEF_STRUCT);
  L_add_symbol (L_lcode_symbol_table, "def_union", L_INPUT_DEF_UNION);
  L_add_symbol (L_lcode_symbol_table, "def_enum", L_INPUT_DEF_ENUM);
  L_add_symbol (L_lcode_symbol_table, "field", L_INPUT_FIELD);
  L_add_symbol (L_lcode_symbol_table, "enumerator", L_INPUT_ENUMERATOR);
  /*
   * MCM - 64-bit support -- 7/2000
   */
  L_add_symbol (L_lcode_symbol_table, "longlong", L_INPUT_LONGLONG);
  L_add_symbol (L_lcode_symbol_table, "wq", L_INPUT_WQ);

  /*
   *  define opcode symbols
   */
  L_add_symbol (L_opcode_symbol_table, Lopcode_NO_OP, Lop_NO_OP);
  L_add_symbol (L_opcode_symbol_table, Lopcode_JSR, Lop_JSR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_JSR_FS, Lop_JSR_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_RTS, Lop_RTS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_RTS_FS, Lop_RTS_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PROLOGUE, Lop_PROLOGUE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EPILOGUE, Lop_EPILOGUE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_DEFINE, Lop_DEFINE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ALLOC, Lop_ALLOC);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PHI, Lop_PHI);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MU, Lop_MU);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GAMMA, Lop_GAMMA);
  L_add_symbol (L_opcode_symbol_table, Lopcode_JUMP, Lop_JUMP);
  L_add_symbol (L_opcode_symbol_table, Lopcode_JUMP_FS, Lop_JUMP_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_JUMP_RG, Lop_JUMP_RG);
  L_add_symbol (L_opcode_symbol_table, Lopcode_JUMP_RG_FS, Lop_JUMP_RG_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BR, Lop_BR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BR_F, Lop_BR_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BEQ, Lop_BEQ);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BEQ_FS, Lop_BEQ_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BNE, Lop_BNE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BNE_FS, Lop_BNE_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT, Lop_BGT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT_FS, Lop_BGT_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE, Lop_BGE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE_FS, Lop_BGE_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT, Lop_BLT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT_FS, Lop_BLT_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE, Lop_BLE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE_FS, Lop_BLE_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT_U, Lop_BGT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT_U_FS, Lop_BGT_U_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE_U, Lop_BGE_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE_U_FS, Lop_BGE_U_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT_U, Lop_BLT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT_U_FS, Lop_BLT_U_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE_U, Lop_BLE_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE_U_FS, Lop_BLE_U_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BEQ_F, Lop_BEQ_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BEQ_F_FS, Lop_BEQ_F_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BNE_F, Lop_BNE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BNE_F_FS, Lop_BNE_F_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT_F, Lop_BGT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT_F_FS, Lop_BGT_F_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE_F, Lop_BGE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE_F_FS, Lop_BGE_F_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT_F, Lop_BLT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT_F_FS, Lop_BLT_F_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE_F, Lop_BLE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE_F_FS, Lop_BLE_F_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BEQ_F2, Lop_BEQ_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BEQ_F2_FS, Lop_BEQ_F2_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BNE_F2, Lop_BNE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BNE_F2_FS, Lop_BNE_F2_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT_F2, Lop_BGT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGT_F2_FS, Lop_BGT_F2_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE_F2, Lop_BGE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BGE_F2_FS, Lop_BGE_F2_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT_F2, Lop_BLT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLT_F2_FS, Lop_BLT_F2_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE_F2, Lop_BLE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BLE_F2_FS, Lop_BLE_F2_FS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PBR, Lop_PBR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MOV, Lop_MOV);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MOV_F, Lop_MOV_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MOV_F2, Lop_MOV_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD, Lop_ADD);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD_U, Lop_ADD_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB, Lop_SUB);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB_U, Lop_SUB_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL, Lop_MUL);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_U, Lop_MUL_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_DIV, Lop_DIV);
  L_add_symbol (L_opcode_symbol_table, Lopcode_DIV_U, Lop_DIV_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_REM, Lop_REM);
  L_add_symbol (L_opcode_symbol_table, Lopcode_REM_U, Lop_REM_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ABS, Lop_ABS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_ADD, Lop_MUL_ADD);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_ADD_U, Lop_MUL_ADD_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB, Lop_MUL_SUB);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB_U, Lop_MUL_SUB_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB_REV, Lop_MUL_SUB_REV);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB_REV_U,
                Lop_MUL_SUB_REV_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MAX, Lop_MAX);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MIN, Lop_MIN);
  L_add_symbol (L_opcode_symbol_table, Lopcode_OR, Lop_OR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_AND, Lop_AND);
  L_add_symbol (L_opcode_symbol_table, Lopcode_XOR, Lop_XOR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_NOR, Lop_NOR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_NAND, Lop_NAND);
  L_add_symbol (L_opcode_symbol_table, Lopcode_NXOR, Lop_NXOR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_OR_NOT, Lop_OR_NOT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_AND_NOT, Lop_AND_NOT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_OR_COMPL, Lop_OR_COMPL);
  L_add_symbol (L_opcode_symbol_table, Lopcode_AND_COMPL, Lop_AND_COMPL);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EQ, Lop_EQ);
  L_add_symbol (L_opcode_symbol_table, Lopcode_NE, Lop_NE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GT, Lop_GT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GT_U, Lop_GT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GE, Lop_GE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GE_U, Lop_GE_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LT, Lop_LT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LT_U, Lop_LT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LE, Lop_LE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LE_U, Lop_LE_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LSL, Lop_LSL);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LSR, Lop_LSR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ASR, Lop_ASR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_REV, Lop_REV);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BIT_POS, Lop_BIT_POS);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD_F2, Lop_ADD_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB_F2, Lop_SUB_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_F2, Lop_MUL_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_DIV_F2, Lop_DIV_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_RCP_F2, Lop_RCP_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ABS_F2, Lop_ABS_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_ADD_F2, Lop_MUL_ADD_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB_F2, Lop_MUL_SUB_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB_REV_F2,
                Lop_MUL_SUB_REV_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SQRT_F2, Lop_SQRT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MAX_F2, Lop_MAX_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MIN_F2, Lop_MIN_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EQ_F2, Lop_EQ_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_NE_F2, Lop_NE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GT_F2, Lop_GT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GE_F2, Lop_GE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LT_F2, Lop_LT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LE_F2, Lop_LE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD_F, Lop_ADD_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB_F, Lop_SUB_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_F, Lop_MUL_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_DIV_F, Lop_DIV_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_RCP_F, Lop_RCP_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ABS_F, Lop_ABS_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_ADD_F, Lop_MUL_ADD_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB_F, Lop_MUL_SUB_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SUB_REV_F,
                Lop_MUL_SUB_REV_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SQRT_F, Lop_SQRT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MAX_F, Lop_MAX_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MIN_F, Lop_MIN_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EQ_F, Lop_EQ_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_NE_F, Lop_NE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GT_F, Lop_GT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_GE_F, Lop_GE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LT_F, Lop_LT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LE_F, Lop_LE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_F2_I, Lop_F2_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_I_F2, Lop_I_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_F_I, Lop_F_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_I_F, Lop_I_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_F2_F, Lop_F2_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_F_F2, Lop_F_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_UC, Lop_LD_UC);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_C, Lop_LD_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_UC2, Lop_LD_UC2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_C2, Lop_LD_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_I, Lop_LD_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_UI, Lop_LD_UI);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_Q, Lop_LD_Q);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_F, Lop_LD_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_F2, Lop_LD_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_UC, Lop_LD_PRE_UC);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_C, Lop_LD_PRE_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_UC2, Lop_LD_PRE_UC2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_C2, Lop_LD_PRE_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_I, Lop_LD_PRE_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_UI, Lop_LD_PRE_UI);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_Q, Lop_LD_PRE_Q);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_F, Lop_LD_PRE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_PRE_F2, Lop_LD_PRE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_UC, Lop_LD_POST_UC);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_C, Lop_LD_POST_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_UC2, Lop_LD_POST_UC2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_C2, Lop_LD_POST_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_I, Lop_LD_POST_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_UI, Lop_LD_POST_UI);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_Q, Lop_LD_POST_Q);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_F, Lop_LD_POST_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_POST_F2, Lop_LD_POST_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_C, Lop_ST_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_C2, Lop_ST_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_I, Lop_ST_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_Q, Lop_ST_Q);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_F, Lop_ST_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_F2, Lop_ST_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_PRE_C, Lop_ST_PRE_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_PRE_C2, Lop_ST_PRE_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_PRE_I, Lop_ST_PRE_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_PRE_Q, Lop_ST_PRE_Q);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_PRE_F, Lop_ST_PRE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_PRE_F2, Lop_ST_PRE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_POST_C, Lop_ST_POST_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_POST_C2, Lop_ST_POST_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_POST_I, Lop_ST_POST_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_POST_Q, Lop_ST_POST_Q);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_POST_F, Lop_ST_POST_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ST_POST_F2, Lop_ST_POST_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EXTRACT_C, Lop_EXTRACT_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EXTRACT_C2, Lop_EXTRACT_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EXTRACT, Lop_EXTRACT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EXTRACT_U, Lop_EXTRACT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_DEPOSIT, Lop_DEPOSIT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_FETCH_AND_ADD,
                Lop_FETCH_AND_ADD);
  L_add_symbol (L_opcode_symbol_table, Lopcode_FETCH_AND_OR,
                Lop_FETCH_AND_OR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_FETCH_AND_AND,
                Lop_FETCH_AND_AND);
  L_add_symbol (L_opcode_symbol_table, Lopcode_FETCH_AND_ST,
                Lop_FETCH_AND_ST);
  L_add_symbol (L_opcode_symbol_table, Lopcode_FETCH_AND_COND_ST,
                Lop_FETCH_AND_COND_ST);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADVANCE, Lop_ADVANCE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_AWAIT, Lop_AWAIT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUTEX_B, Lop_MUTEX_B);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUTEX_E, Lop_MUTEX_E);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CO_PROC, Lop_CO_PROC);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CHECK, Lop_CHECK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CONFIRM, Lop_CONFIRM);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_CLEAR, Lop_PRED_CLEAR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_SET, Lop_PRED_SET);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LD, Lop_PRED_LD);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_ST, Lop_PRED_ST);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LD_BLK, Lop_PRED_LD_BLK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_ST_BLK, Lop_PRED_ST_BLK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_MERGE, Lop_PRED_MERGE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_AND, Lop_PRED_AND);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_COMPL, Lop_PRED_COMPL);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_COPY, Lop_PRED_COPY);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMP, Lop_CMP);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMP_F, Lop_CMP_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_RCMP, Lop_RCMP);
  L_add_symbol (L_opcode_symbol_table, Lopcode_RCMP_F, Lop_RCMP_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_EQ, Lop_PRED_EQ);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_NE, Lop_PRED_NE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GT, Lop_PRED_GT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GT_U, Lop_PRED_GT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GE, Lop_PRED_GE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GE_U, Lop_PRED_GE_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LT, Lop_PRED_LT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LT_U, Lop_PRED_LT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LE, Lop_PRED_LE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LE_U, Lop_PRED_LE_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_EQ_F2, Lop_PRED_EQ_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_NE_F2, Lop_PRED_NE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GT_F2, Lop_PRED_GT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GE_F2, Lop_PRED_GE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LT_F2, Lop_PRED_LT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LE_F2, Lop_PRED_LE_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_EQ_F, Lop_PRED_EQ_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_NE_F, Lop_PRED_NE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GT_F, Lop_PRED_GT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_GE_F, Lop_PRED_GE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LT_F, Lop_PRED_LT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_LE_F, Lop_PRED_LE_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_MASK_AND,
                Lop_PRED_MASK_AND);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PRED_MASK_OR,
                Lop_PRED_MASK_OR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMOV, Lop_CMOV);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMOV_COM, Lop_CMOV_COM);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMOV_F, Lop_CMOV_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMOV_COM_F, Lop_CMOV_COM_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMOV_F2, Lop_CMOV_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CMOV_COM_F2, Lop_CMOV_COM_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SELECT, Lop_SELECT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SELECT_F, Lop_SELECT_F);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SELECT_F2, Lop_SELECT_F2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_PREF_LD, Lop_PREF_LD);
  L_add_symbol (L_opcode_symbol_table, Lopcode_JSR_ND, Lop_JSR_ND);
  L_add_symbol (L_opcode_symbol_table, Lopcode_EXPAND, Lop_EXPAND);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MEM_COPY, Lop_MEM_COPY);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MEM_COPY_BACK,
                Lop_MEM_COPY_BACK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MEM_COPY_CHECK,
                Lop_MEM_COPY_CHECK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MEM_COPY_RESET,
                Lop_MEM_COPY_RESET);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MEM_COPY_SETUP,
                Lop_MEM_COPY_SETUP);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MEM_COPY_TAG,
                Lop_MEM_COPY_TAG);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SIM_DIR, Lop_SIM_DIR);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BOUNDARY, Lop_BOUNDARY);
  L_add_symbol (L_opcode_symbol_table, Lopcode_REMAP, Lop_REMAP);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BIT_EXTRACT, Lop_BIT_EXTRACT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_BIT_DEPOSIT, Lop_BIT_DEPOSIT);

  /* ITI/JWJ 7/99 */
  L_add_symbol (L_opcode_symbol_table, Lopcode_INTRINSIC, Lop_INTRINSIC);

  /* ITI/JWJ 8.11.1999 */
  L_add_symbol (L_opcode_symbol_table, Lopcode_L_MAC, Lop_L_MAC);
  L_add_symbol (L_opcode_symbol_table, Lopcode_L_MSU, Lop_L_MSU);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD_SAT, Lop_ADD_SAT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD_SAT_U, Lop_ADD_SAT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB_SAT, Lop_SUB_SAT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB_SAT_U, Lop_SUB_SAT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SAT, Lop_MUL_SAT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_SAT_U, Lop_MUL_SAT_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SAT, Lop_SAT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SAT_U, Lop_SAT_U);

  L_add_symbol (L_opcode_symbol_table, Lopcode_LSLADD, Lop_LSLADD);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SXT_C, Lop_SXT_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SXT_C2, Lop_SXT_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SXT_I, Lop_SXT_I);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ZXT_C, Lop_ZXT_C);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ZXT_C2, Lop_ZXT_C2);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ZXT_I, Lop_ZXT_I);

  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_UC_CHK, Lop_LD_UC_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_C_CHK, Lop_LD_C_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_UC2_CHK, Lop_LD_UC2_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_C2_CHK, Lop_LD_C2_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_UI_CHK, Lop_LD_UI_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_I_CHK, Lop_LD_I_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_Q_CHK, Lop_LD_Q_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_F_CHK, Lop_LD_F_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_LD_F2_CHK, Lop_LD_F2_CHK);
  L_add_symbol (L_opcode_symbol_table, Lopcode_CHECK_ALAT, Lop_CHECK_ALAT);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD_CARRY, Lop_ADD_CARRY);
  L_add_symbol (L_opcode_symbol_table, Lopcode_ADD_CARRY_U, Lop_ADD_CARRY_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB_CARRY, Lop_SUB_CARRY);
  L_add_symbol (L_opcode_symbol_table, Lopcode_SUB_CARRY_U, Lop_SUB_CARRY_U);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_WIDE, Lop_MUL_WIDE);
  L_add_symbol (L_opcode_symbol_table, Lopcode_MUL_WIDE_U, Lop_MUL_WIDE_U);

  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_EQ, Lcmp_COM_EQ);
  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_NE, Lcmp_COM_NE);
  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_GT, Lcmp_COM_GT);
  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_LE, Lcmp_COM_LE);
  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_GE, Lcmp_COM_GE);
  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_LT, Lcmp_COM_LT);
  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_TZ, Lcmp_COM_TZ);
  L_add_symbol (L_cmp_compl_symbol_table, Lcompl_COM_TN, Lcmp_COM_TN);

  // SLARSEN: Define vector opcodes
  L_add_symbol(L_opcode_symbol_table, Lopcode_VADD, Lop_VADD);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VADD_U, Lop_VADD_U);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSUB, Lop_VSUB);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSUB_U, Lop_VSUB_U);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMUL, Lop_VMUL);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMUL_U, Lop_VMUL_U);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VDIV, Lop_VDIV);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VDIV_U, Lop_VDIV_U);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VREM, Lop_VREM);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VREM_U, Lop_VREM_U);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMIN, Lop_VMIN);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMAX, Lop_VMAX);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VOR, Lop_VOR);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VAND, Lop_VAND);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VADD_F, Lop_VADD_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSUB_F, Lop_VSUB_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMUL_F, Lop_VMUL_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VDIV_F, Lop_VDIV_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VABS_F, Lop_VABS_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSQRT_F, Lop_VSQRT_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMAX_F, Lop_VMAX_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMIN_F, Lop_VMIN_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VADD_F2, Lop_VADD_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSUB_F2, Lop_VSUB_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMUL_F2, Lop_VMUL_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VDIV_F2, Lop_VDIV_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VABS_F2, Lop_VABS_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSQRT_F2, Lop_VSQRT_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMAX_F2, Lop_VMAX_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMIN_F2, Lop_VMIN_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMOVE, Lop_VMOV);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMOVE_F, Lop_VMOV_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VMOVE_F2, Lop_VMOV_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VI_VF, Lop_VI_VF);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VI_VF2, Lop_VI_VF2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VF_VI, Lop_VF_VI);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VF2_VI, Lop_VF2_VI);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VF_VF2, Lop_VF_VF2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VF2_VF, Lop_VF2_VF);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VI_I, Lop_VI_I);
  L_add_symbol(L_opcode_symbol_table, Lopcode_I_VI, Lop_I_VI);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VF_F, Lop_VF_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_F_VF, Lop_F_VF);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VF2_F2, Lop_VF2_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_F2_VF2, Lop_F2_VF2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VPERM, Lop_VPERM);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VPERM_F, Lop_VPERM_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VPERM_F2, Lop_VPERM_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSPLAT, Lop_VSPLAT);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSPLAT_F, Lop_VSPLAT_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSPLAT_F2, Lop_VSPLAT_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLD_UC, Lop_VLD_UC);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLD_C, Lop_VLD_C);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLD_UC2, Lop_VLD_UC2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLD_C2, Lop_VLD_C2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLD_I, Lop_VLD_I);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLD_F, Lop_VLD_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLD_F2, Lop_VLD_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VST_C, Lop_VST_C);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VST_C2, Lop_VST_C2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VST_I, Lop_VST_I);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VST_F, Lop_VST_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VST_F2, Lop_VST_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLDE_UC, Lop_VLDE_UC);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLDE_C, Lop_VLDE_C);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLDE_UC2, Lop_VLDE_UC2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLDE_C2, Lop_VLDE_C2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLDE_I, Lop_VLDE_I);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLDE_F, Lop_VLDE_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VLDE_F2, Lop_VLDE_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSTE_C, Lop_VSTE_C);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSTE_C2, Lop_VSTE_C2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSTE_I, Lop_VSTE_I);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSTE_F, Lop_VSTE_F);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VSTE_F2, Lop_VSTE_F2);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VEXTRACT_C, Lop_VEXTRACT_C);
  L_add_symbol(L_opcode_symbol_table, Lopcode_VEXTRACT_C2, Lop_VEXTRACT_C2);

  // AMIR: LM
  L_add_symbol(L_opcode_symbol_table, Lopcode_PUSH, Lop_PUSH);
  L_add_symbol(L_opcode_symbol_table, Lopcode_POP, Lop_POP);
  L_add_symbol(L_opcode_symbol_table, Lopcode_PEEK, Lop_PEEK);

#if 0
  STRING_print_symbol_table_hash (stdout, L_ms_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_string_table);
  STRING_print_symbol_table_hash (stdout, L_opcode_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_macro_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_ms_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_ctype_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_ptype_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_operand_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_expr_symbol_table);
  STRING_print_symbol_table_hash (stdout, L_lcode_symbol_table);
  fflush (stdout);
#endif
}

int
L_macro_id (char *name)
{
  return (L_find_symbol_id (L_macro_symbol_table, name));
}

int
L_ms_id (char *name)
{
  return (L_find_symbol_id (L_ms_symbol_table, name));
}

int
L_ctype_id (char *name)
{
  return (L_find_symbol_id (L_ctype_symbol_table, name));
}

int
L_ptype_id (char *name)
{
  return (L_find_symbol_id (L_ptype_symbol_table, name));
}

int
L_operand_id (char *name)
{
  return (L_find_symbol_id (L_operand_symbol_table, name));
}

int
L_expr_id (char *name)
{
  return (L_find_symbol_id (L_expr_symbol_table, name));
}

int
L_lcode_id (char *name)
{
  return (L_find_symbol_id (L_lcode_symbol_table, name));
}

int
L_opcode_id (char *name)
{
  return (L_find_symbol_id (L_opcode_symbol_table, name));
}

/* JWS 20000718 Use completers for branches, pred defs */

ITuint8
L_cmp_compl_id (char *name)
{
  return (L_find_symbol_id (L_cmp_compl_symbol_table, name));
}

static ITuint8 L_default_icmp_ctype = L_CTYPE_VOID;

void
L_cmp_compl_from_old_opc (L_Oper * oper)
{
  ITuint8 *ctype, *com;
  int *opc;

  ctype = oper->com;
  com = oper->com + 1;
  opc = &oper->opc;

  switch (*opc)
    {
    case Lop_BEQ:
    case Lop_BEQ_F2:
    case Lop_BEQ_F2_FS:
    case Lop_BEQ_F:
    case Lop_BEQ_FS:
    case Lop_BEQ_F_FS:
    case Lop_EQ:
    case Lop_EQ_F2:
    case Lop_EQ_F:
    case Lop_PRED_EQ:
    case Lop_PRED_EQ_F2:
    case Lop_PRED_EQ_F:
      *com = Lcmp_COM_EQ;
      break;

    case Lop_BNE:
    case Lop_BNE_F2:
    case Lop_BNE_F2_FS:
    case Lop_BNE_F:
    case Lop_BNE_FS:
    case Lop_BNE_F_FS:
    case Lop_NE:
    case Lop_NE_F2:
    case Lop_NE_F:
    case Lop_PRED_NE:
    case Lop_PRED_NE_F2:
    case Lop_PRED_NE_F:
      *com = Lcmp_COM_NE;
      break;

    case Lop_BGE:
    case Lop_BGE_F2:
    case Lop_BGE_F2_FS:
    case Lop_BGE_F:
    case Lop_BGE_FS:
    case Lop_BGE_F_FS:
    case Lop_BGE_U:
    case Lop_BGE_U_FS:
    case Lop_GE:
    case Lop_GE_F2:
    case Lop_GE_F:
    case Lop_GE_U:
    case Lop_PRED_GE:
    case Lop_PRED_GE_F2:
    case Lop_PRED_GE_F:
    case Lop_PRED_GE_U:
      *com = Lcmp_COM_GE;
      break;

    case Lop_BLT:
    case Lop_BLT_F2:
    case Lop_BLT_F2_FS:
    case Lop_BLT_F:
    case Lop_BLT_FS:
    case Lop_BLT_F_FS:
    case Lop_BLT_U:
    case Lop_BLT_U_FS:
    case Lop_LT:
    case Lop_LT_F2:
    case Lop_LT_F:
    case Lop_LT_U:
    case Lop_PRED_LT:
    case Lop_PRED_LT_F2:
    case Lop_PRED_LT_F:
    case Lop_PRED_LT_U:
      *com = Lcmp_COM_LT;
      break;

    case Lop_BGT:
    case Lop_BGT_F2:
    case Lop_BGT_F2_FS:
    case Lop_BGT_F:
    case Lop_BGT_FS:
    case Lop_BGT_F_FS:
    case Lop_BGT_U:
    case Lop_BGT_U_FS:
    case Lop_GT:
    case Lop_GT_F2:
    case Lop_GT_F:
    case Lop_GT_U:
    case Lop_PRED_GT:
    case Lop_PRED_GT_F2:
    case Lop_PRED_GT_F:
    case Lop_PRED_GT_U:
      *com = Lcmp_COM_GT;
      break;

    case Lop_BLE:
    case Lop_BLE_F2:
    case Lop_BLE_F2_FS:
    case Lop_BLE_F:
    case Lop_BLE_FS:
    case Lop_BLE_F_FS:
    case Lop_BLE_U:
    case Lop_BLE_U_FS:
    case Lop_LE:
    case Lop_LE_F2:
    case Lop_LE_F:
    case Lop_LE_U:
    case Lop_PRED_LE:
    case Lop_PRED_LE_F2:
    case Lop_PRED_LE_F:
    case Lop_PRED_LE_U:
      *com = Lcmp_COM_LE;
      break;

    default:
      L_punt ("L_cmp_compl_from_old_opc (1): Illegal opcode %d", *opc);
    }

  switch (*opc)
    {
    case Lop_BEQ:
    case Lop_BGE:
    case Lop_BGT:
    case Lop_BLE:
    case Lop_BLT:
    case Lop_BNE:
    case Lop_BEQ_FS:
    case Lop_BGT_FS:
    case Lop_BGE_FS:
    case Lop_BNE_FS:
    case Lop_BLE_FS:
    case Lop_BLT_FS:
    case Lop_EQ:
    case Lop_PRED_EQ:
    case Lop_GE:
    case Lop_PRED_GE:
    case Lop_GT:
    case Lop_PRED_GT:
    case Lop_LE:
    case Lop_PRED_LE:
    case Lop_LT:
    case Lop_PRED_LT:
    case Lop_NE:
    case Lop_PRED_NE:
      *ctype = L_default_icmp_ctype;
      break;

    case Lop_BGE_U:
    case Lop_GE_U:
    case Lop_PRED_GE_U:
    case Lop_BGT_U:
    case Lop_GT_U:
    case Lop_PRED_GT_U:
    case Lop_BLE_U:
    case Lop_LE_U:
    case Lop_PRED_LE_U:
    case Lop_BLT_U:
    case Lop_LT_U:
    case Lop_PRED_LT_U:
    case Lop_BGE_U_FS:
    case Lop_BGT_U_FS:
    case Lop_BLE_U_FS:
    case Lop_BLT_U_FS:
      *ctype = L_ctype_unsigned_version (L_default_icmp_ctype);
      break;

    case Lop_BEQ_F2:
    case Lop_EQ_F2:
    case Lop_PRED_EQ_F2:
    case Lop_BNE_F2:
    case Lop_NE_F2:
    case Lop_PRED_NE_F2:
    case Lop_BGE_F2:
    case Lop_GE_F2:
    case Lop_PRED_GE_F2:
    case Lop_BGT_F2:
    case Lop_GT_F2:
    case Lop_PRED_GT_F2:
    case Lop_BLE_F2:
    case Lop_LE_F2:
    case Lop_PRED_LE_F2:
    case Lop_BLT_F2:
    case Lop_LT_F2:
    case Lop_PRED_LT_F2:
    case Lop_BEQ_F2_FS:
    case Lop_BNE_F2_FS:
    case Lop_BGE_F2_FS:
    case Lop_BGT_F2_FS:
    case Lop_BLE_F2_FS:
    case Lop_BLT_F2_FS:
      *ctype = L_CTYPE_DOUBLE;
      break;

    case Lop_BEQ_F:
    case Lop_EQ_F:
    case Lop_PRED_EQ_F:
    case Lop_BNE_F:
    case Lop_NE_F:
    case Lop_PRED_NE_F:
    case Lop_BGE_F:
    case Lop_GE_F:
    case Lop_PRED_GE_F:
    case Lop_BGT_F:
    case Lop_GT_F:
    case Lop_PRED_GT_F:
    case Lop_BLE_F:
    case Lop_LE_F:
    case Lop_PRED_LE_F:
    case Lop_BLT_F:
    case Lop_LT_F:
    case Lop_PRED_LT_F:
    case Lop_BEQ_F_FS:
    case Lop_BNE_F_FS:
    case Lop_BGE_F_FS:
    case Lop_BGT_F_FS:
    case Lop_BLE_F_FS:
    case Lop_BLT_F_FS:
      *ctype = L_CTYPE_FLOAT;
      break;

    default:
      L_punt ("L_cmp_compl_from_old_opc (2): Illegal opcode %d", *opc);
    }

  switch (*opc)
    {
    case Lop_BEQ:
    case Lop_BEQ_FS:
    case Lop_BNE:
    case Lop_BNE_FS:
    case Lop_BGE:
    case Lop_BGE_FS:
    case Lop_BGE_U:
    case Lop_BGE_U_FS:
    case Lop_BLT:
    case Lop_BLT_FS:
    case Lop_BLT_U:
    case Lop_BLT_U_FS:
    case Lop_BGT:
    case Lop_BGT_FS:
    case Lop_BGT_U:
    case Lop_BGT_U_FS:
    case Lop_BLE:
    case Lop_BLE_FS:
    case Lop_BLE_U:
    case Lop_BLE_U_FS:
      *opc = Lop_BR;
      break;

    case Lop_BEQ_F2:
    case Lop_BEQ_F2_FS:
    case Lop_BEQ_F:
    case Lop_BEQ_F_FS:
    case Lop_BNE_F2:
    case Lop_BNE_F2_FS:
    case Lop_BNE_F:
    case Lop_BNE_F_FS:
    case Lop_BGE_F2:
    case Lop_BGE_F2_FS:
    case Lop_BGE_F:
    case Lop_BGE_F_FS:
    case Lop_BLT_F2:
    case Lop_BLT_F2_FS:
    case Lop_BLT_F:
    case Lop_BLT_F_FS:
    case Lop_BGT_F2:
    case Lop_BGT_F2_FS:
    case Lop_BGT_F:
    case Lop_BGT_F_FS:
    case Lop_BLE_F2:
    case Lop_BLE_F2_FS:
    case Lop_BLE_F:
    case Lop_BLE_F_FS:
      *opc = Lop_BR_F;
      break;

    case Lop_PRED_EQ:
    case Lop_PRED_NE:
    case Lop_PRED_GE:
    case Lop_PRED_GE_U:
    case Lop_PRED_LT:
    case Lop_PRED_LT_U:
    case Lop_PRED_GT:
    case Lop_PRED_GT_U:
    case Lop_PRED_LE:
    case Lop_PRED_LE_U:
      *opc = Lop_CMP;
      break;

    case Lop_PRED_EQ_F2:
    case Lop_PRED_EQ_F:
    case Lop_PRED_NE_F2:
    case Lop_PRED_NE_F:
    case Lop_PRED_GE_F2:
    case Lop_PRED_GE_F:
    case Lop_PRED_LT_F2:
    case Lop_PRED_LT_F:
    case Lop_PRED_GT_F2:
    case Lop_PRED_GT_F:
    case Lop_PRED_LE_F2:
    case Lop_PRED_LE_F:
      *opc = Lop_CMP_F;
      break;

    case Lop_EQ:
    case Lop_NE:
    case Lop_GE:
    case Lop_GE_U:
    case Lop_LT:
    case Lop_LT_U:
    case Lop_GT:
    case Lop_GT_U:
    case Lop_LE:
    case Lop_LE_U:
      *opc = Lop_RCMP;
      break;

    case Lop_EQ_F2:
    case Lop_EQ_F:
    case Lop_NE_F2:
    case Lop_NE_F:
    case Lop_GE_F2:
    case Lop_GE_F:
    case Lop_LT_F2:
    case Lop_LT_F:
    case Lop_GT_F2:
    case Lop_GT_F:
    case Lop_LE_F2:
    case Lop_LE_F:
      *opc = Lop_RCMP_F;
      break;

    default:
      L_punt ("L_cmp_compl_from_old_opc (3): Illegal opcode %d", *opc);
    }

  return;
}

void
L_convert_to_com (L_Oper * oper)
{
  if (L_default_icmp_ctype == L_CTYPE_VOID)
    L_default_icmp_ctype = M_native_int_register_ctype ();

  if (L_opc_vestigial_compare (oper->opc))
    L_cmp_compl_from_old_opc (oper);
  else
    L_punt ("L_convert_to_com: Attempt to convert a non-vestigial opcode %d",
            oper->opc);

  oper->opcode = L_opcode_name (oper->opc);

  return;
}

char *
L_macro_name (int id)
{
  /* To support intrinsics -ITI/JWJ 7/99 */
  if (id >= L_INTRINSIC_MACRO_REG_START)
    {
      return (L_find_id_symbol (L_macro_id_symbol_table, id));
    }

  switch (id)
    {
    case L_MAC_P0:
      return L_MACRO_P0;
    case L_MAC_P1:
      return L_MACRO_P1;
    case L_MAC_P2:
      return L_MACRO_P2;
    case L_MAC_P3:
      return L_MACRO_P3;
    case L_MAC_P4:
      return L_MACRO_P4;
    case L_MAC_P5:
      return L_MACRO_P5;
    case L_MAC_P6:
      return L_MACRO_P6;
    case L_MAC_P7:
      return L_MACRO_P7;
    case L_MAC_P8:
      return L_MACRO_P8;
    case L_MAC_P9:
      return L_MACRO_P9;
    case L_MAC_P10:
      return L_MACRO_P10;
    case L_MAC_P11:
      return L_MACRO_P11;
    case L_MAC_P12:
      return L_MACRO_P12;
    case L_MAC_P13:
      return L_MACRO_P13;
    case L_MAC_P14:
      return L_MACRO_P14;
    case L_MAC_P15:
      return L_MACRO_P15;
    case L_MAC_P16:
      return L_MACRO_P16;
    case L_MAC_P17:
      return L_MACRO_P17;
    case L_MAC_P18:
      return L_MACRO_P18;
    case L_MAC_P19:
      return L_MACRO_P19;
    case L_MAC_P20:
      return L_MACRO_P20;
    case L_MAC_P21:
      return L_MACRO_P21;
    case L_MAC_P22:
      return L_MACRO_P22;
    case L_MAC_P23:
      return L_MACRO_P23;
    case L_MAC_P24:
      return L_MACRO_P24;
    case L_MAC_P25:
      return L_MACRO_P25;
    case L_MAC_P26:
      return L_MACRO_P26;
    case L_MAC_P27:
      return L_MACRO_P27;
    case L_MAC_P28:
      return L_MACRO_P28;
    case L_MAC_P29:
      return L_MACRO_P29;
    case L_MAC_P30:
      return L_MACRO_P30;
    case L_MAC_P31:
      return L_MACRO_P31;
    case L_MAC_P32:
      return L_MACRO_P32;
    case L_MAC_P33:
      return L_MACRO_P33;
    case L_MAC_P34:
      return L_MACRO_P34;
    case L_MAC_P35:
      return L_MACRO_P35;
    case L_MAC_P36:
      return L_MACRO_P36;
    case L_MAC_P37:
      return L_MACRO_P37;
    case L_MAC_P38:
      return L_MACRO_P38;
    case L_MAC_P39:
      return L_MACRO_P39;
    case L_MAC_P40:
      return L_MACRO_P40;
    case L_MAC_P41:
      return L_MACRO_P41;
    case L_MAC_P42:
      return L_MACRO_P42;
    case L_MAC_P43:
      return L_MACRO_P43;
    case L_MAC_P44:
      return L_MACRO_P44;
    case L_MAC_P45:
      return L_MACRO_P45;
    case L_MAC_P46:
      return L_MACRO_P46;
    case L_MAC_P47:
      return L_MACRO_P47;
    case L_MAC_P48:
      return L_MACRO_P48;
    case L_MAC_P49:
      return L_MACRO_P49;
    case L_MAC_P50:
      return L_MACRO_P50;
    case L_MAC_P51:
      return L_MACRO_P51;
    case L_MAC_P52:
      return L_MACRO_P52;
    case L_MAC_P53:
      return L_MACRO_P53;
    case L_MAC_P54:
      return L_MACRO_P54;
    case L_MAC_P55:
      return L_MACRO_P55;
    case L_MAC_P56:
      return L_MACRO_P56;
    case L_MAC_P57:
      return L_MACRO_P57;
    case L_MAC_P58:
      return L_MACRO_P58;
    case L_MAC_P59:
      return L_MACRO_P59;
    case L_MAC_P60:
      return L_MACRO_P60;
    case L_MAC_P61:
      return L_MACRO_P61;
    case L_MAC_P62:
      return L_MACRO_P62;
    case L_MAC_P63:
      return L_MACRO_P63;
    case L_MAC_P64:
      return L_MACRO_P64;
    case L_MAC_TR_PTR:
      return L_MACRO_TR_PTR;
    case L_MAC_TR_MARK:
      return L_MACRO_TR_MARK;
    case L_MAC_TR_TEMP:
      return L_MACRO_TR_TEMP;
    case L_MAC_PRED_ALL:
      return L_MACRO_PRED_ALL;
    case L_MAC_SAFE_MEM:
      return L_MACRO_SAFE_MEM;
    case L_MAC_TM_TYPE:
      return L_MACRO_TM_TYPE;
    case L_MAC_SP:
      return L_MACRO_SP;
    case L_MAC_FP:
      return L_MACRO_FP;
    case L_MAC_IP:
      return L_MACRO_IP;
    case L_MAC_OP:
      return L_MACRO_OP;
    case L_MAC_LV:
      return L_MACRO_LV;
    case L_MAC_RS:
      return L_MACRO_RS;
    case L_MAC_CR:
      return L_MACRO_CR;
    case L_MAC_LOCAL_SIZE:
      return L_MACRO_LOCAL_SIZE;
    case L_MAC_PARAM_SIZE:
      return L_MACRO_PARAM_SIZE;
    case L_MAC_SWAP_SIZE:
      return L_MACRO_SWAP_SIZE;
    case L_MAC_RET_TYPE:
      return L_MACRO_RET_TYPE;
    case L_MAC_RETADDR:
      return L_MACRO_RETADDR;
    default:
      /* machine specific macro names */
      return M_get_macro_name (id);
    }
}

char *
L_ms_name (int id)
{
  switch (id)
    {
    case L_MS_TEXT:
      return "text";
    case L_MS_DATA:
      return "data";
    case L_MS_DATA1:
      return "data1";
    case L_MS_DATA2:
      return "data2";
    case L_MS_SDATA:
      return "sdata";
    case L_MS_SDATA1:
      return "sdata1";
    case L_MS_RODATA:
      return "rodata";
    case L_MS_BSS:
      return "bss";
    case L_MS_SBSS:
      return "sbss";
    case L_MS_SYNC:
      return "sync";
    default:
      return "?";
    }
}

char *
L_ctype_name (int id)
{
  switch (id)
    {
    case L_CTYPE_VOID:
      return "void";
    case L_CTYPE_CHAR:
      return "c";
    case L_CTYPE_UCHAR:
      return "uc";
    case L_CTYPE_SHORT:
      return "sh";
    case L_CTYPE_USHORT:
      return "ush";
    case L_CTYPE_INT:
      return "i";
    case L_CTYPE_UINT:
      return "ui";
    case L_CTYPE_LONG:
      return "lng";
    case L_CTYPE_ULONG:
      return "ulng";
    case L_CTYPE_LLONG:
      return "ll";
    case L_CTYPE_ULLONG:
      return "ull";
    case L_CTYPE_LLLONG:
      return "lll";
    case L_CTYPE_ULLLONG:
      return "ulll";
    case L_CTYPE_POINTER:
      return "pnt";
    case L_CTYPE_FLOAT:
      return "f";
    case L_CTYPE_DOUBLE:
      return "f2";
    case L_CTYPE_CONTROL:
      return "c";
    case L_CTYPE_BTR:
      return "b";
    case L_CTYPE_PREDICATE:
      return "p";
    case L_CTYPE_LOCAL_ABS:
      return "l_abs";
    case L_CTYPE_LOCAL_GP:
      return "l_gp";
    case L_CTYPE_GLOBAL_ABS:
      return "g_abs";
    case L_CTYPE_GLOBAL_GP:
      return "g_gp";
    /* RMR { adding support for vector file type */
    case L_CTYPE_VECTOR_INT:    return "vi";
    case L_CTYPE_VECTOR_FLOAT:  return "vf";
    case L_CTYPE_VECTOR_DOUBLE: return "vf2";
    case L_CTYPE_VECTOR_MASK:   return "vm";
    /* } RMR */
    default:
      return "?";
    }
}

char *
L_ptype_name (int id)
{
  switch (id)
    {
    case L_PTYPE_NULL:
      return "p";
    case L_PTYPE_UNCOND_T:
      return "p_ut";
    case L_PTYPE_UNCOND_F:
      return "p_uf";
    case L_PTYPE_COND_T:
      return "p_ct";
    case L_PTYPE_COND_F:
      return "p_cf";
    case L_PTYPE_OR_T:
      return "p_ot";
    case L_PTYPE_OR_F:
      return "p_of";
    case L_PTYPE_AND_T:
      return "p_at";
    case L_PTYPE_AND_F:
      return "p_af";
    case L_PTYPE_SAND_T:
      return "p_st";
    case L_PTYPE_SAND_F:
      return "p_sf";
    default:
      return "?";
    }
}

char *
L_operand_name (int id)
{
  switch (id)
    {
    case L_OPERAND_VOID:
      return "void";
    case L_OPERAND_CB:
      return "cb";
    case L_OPERAND_IMMED:
      return "i";
    case L_OPERAND_STRING:
      return "s";
    case L_OPERAND_MACRO:
      return "mac";
    case L_OPERAND_REGISTER:
      return "r";
    case L_OPERAND_LABEL:
      return "l";
    case L_OPERAND_RREGISTER:
      return "rr";
    case L_OPERAND_EVR:
      return "evr";
    default:
      return "?";
    }
}

char *
L_expr_name (int id)
{
  switch (id)
    {
    case L_EXPR_INT:
      return "i";
    case L_EXPR_FLOAT:
      return "f";
    case L_EXPR_DOUBLE:
      return "f2";
    case L_EXPR_LABEL:
      return "l";
    case L_EXPR_STRING:
      return "s";
    case L_EXPR_ADD:
      return "add";
    case L_EXPR_SUB:
      return "sub";
    case L_EXPR_MUL:
      return "mul";
    case L_EXPR_DIV:
      return "div";
    case L_EXPR_NEG:
      return "neg";
    case L_EXPR_COM:
      return "com";
    default:
      return "?";
    }
}

char *
L_lcode_name (int id)
{
  switch (id)
    {
    case L_INPUT_MS:
      return "ms";
    case L_INPUT_VOID:
      return "void";
    case L_INPUT_BYTE:
      return "byte";
    case L_INPUT_WORD:
      return "word";
    case L_INPUT_LONG:
      return "long";
    case L_INPUT_LONGLONG:
      return "longlong";
    case L_INPUT_FLOAT:
      return "float";
    case L_INPUT_DOUBLE:
      return "double";
    case L_INPUT_ALIGN:
      return "align";
    case L_INPUT_ASCII:
      return "ascii";
    case L_INPUT_ASCIZ:
      return "asciz";
    case L_INPUT_RESERVE:
      return "reserve";
    case L_INPUT_SKIP:
      return "skip";
    case L_INPUT_GLOBAL:
      return "global";
    case L_INPUT_WB:
      return "wb";
    case L_INPUT_WW:
      return "ww";
    case L_INPUT_WI:
      return "wi";
    case L_INPUT_WQ:
      return "wq";
    case L_INPUT_WF:
      return "wf";
    case L_INPUT_WF2:
      return "wf2";
    case L_INPUT_WS:
      return "ws";
    case L_INPUT_FUNCTION:
      return "function";
    case L_INPUT_CB:
      return "cb";
    case L_INPUT_OP:
      return "op";
    case L_INPUT_END:
      return "end";
    case L_INPUT_POP:
      return "OP";
    case L_INPUT_ELEMENT_SIZE:
      return "element_size";
      /* LCW - kewwords for preserving debugging info - 4/16/96 */
    case L_INPUT_DEF_STRUCT:
      return "def_struct";
    case L_INPUT_DEF_UNION:
      return "def_union";
    case L_INPUT_DEF_ENUM:
      return "def_enum";
    case L_INPUT_FIELD:
      return "field";
    case L_INPUT_ENUMERATOR:
      return "enumerator";
    default:
      return "?";
    }
}

char *
L_opcode_name (int id)
{
  switch (id)
    {
    case Lop_NO_OP:
      return Lopcode_NO_OP;
    case Lop_JSR:
      return Lopcode_JSR;
    case Lop_JSR_FS:
      return Lopcode_JSR_FS;
    case Lop_RTS:
      return Lopcode_RTS;
    case Lop_RTS_FS:
      return Lopcode_RTS_FS;
    case Lop_PROLOGUE:
      return Lopcode_PROLOGUE;
    case Lop_EPILOGUE:
      return Lopcode_EPILOGUE;
    case Lop_DEFINE:
      return Lopcode_DEFINE;
    case Lop_ALLOC:
      return Lopcode_ALLOC;
    case Lop_JUMP:
      return Lopcode_JUMP;
    case Lop_JUMP_FS:
      return Lopcode_JUMP_FS;
    case Lop_JUMP_RG:
      return Lopcode_JUMP_RG;
    case Lop_JUMP_RG_FS:
      return Lopcode_JUMP_RG_FS;
    case Lop_BR:
      return Lopcode_BR;
    case Lop_BR_F:
      return Lopcode_BR_F;
    case Lop_PHI:
      return Lopcode_PHI;
    case Lop_MU:
      return Lopcode_MU;
    case Lop_GAMMA:
      return Lopcode_GAMMA;
    case Lop_BEQ:
      return Lopcode_BEQ;
    case Lop_BEQ_FS:
      return Lopcode_BEQ_FS;
    case Lop_BNE:
      return Lopcode_BNE;
    case Lop_BNE_FS:
      return Lopcode_BNE_FS;
    case Lop_BGT:
      return Lopcode_BGT;
    case Lop_BGT_FS:
      return Lopcode_BGT_FS;
    case Lop_BGE:
      return Lopcode_BGE;
    case Lop_BGE_FS:
      return Lopcode_BGE_FS;
    case Lop_BLT:
      return Lopcode_BLT;
    case Lop_BLT_FS:
      return Lopcode_BLT_FS;
    case Lop_BLE:
      return Lopcode_BLE;
    case Lop_BLE_FS:
      return Lopcode_BLE_FS;
    case Lop_BGT_U:
      return Lopcode_BGT_U;
    case Lop_BGT_U_FS:
      return Lopcode_BGT_U_FS;
    case Lop_BGE_U:
      return Lopcode_BGE_U;
    case Lop_BGE_U_FS:
      return Lopcode_BGE_U_FS;
    case Lop_BLT_U:
      return Lopcode_BLT_U;
    case Lop_BLT_U_FS:
      return Lopcode_BLT_U_FS;
    case Lop_BLE_U:
      return Lopcode_BLE_U;
    case Lop_BLE_U_FS:
      return Lopcode_BLE_U_FS;
    case Lop_BEQ_F:
      return Lopcode_BEQ_F;
    case Lop_BEQ_F_FS:
      return Lopcode_BEQ_F_FS;
    case Lop_BNE_F:
      return Lopcode_BNE_F;
    case Lop_BNE_F_FS:
      return Lopcode_BNE_F_FS;
    case Lop_BGT_F:
      return Lopcode_BGT_F;
    case Lop_BGT_F_FS:
      return Lopcode_BGT_F_FS;
    case Lop_BGE_F:
      return Lopcode_BGE_F;
    case Lop_BGE_F_FS:
      return Lopcode_BGE_F_FS;
    case Lop_BLT_F:
      return Lopcode_BLT_F;
    case Lop_BLT_F_FS:
      return Lopcode_BLT_F_FS;
    case Lop_BLE_F:
      return Lopcode_BLE_F;
    case Lop_BLE_F_FS:
      return Lopcode_BLE_F_FS;
    case Lop_BEQ_F2:
      return Lopcode_BEQ_F2;
    case Lop_BEQ_F2_FS:
      return Lopcode_BEQ_F2_FS;
    case Lop_BNE_F2:
      return Lopcode_BNE_F2;
    case Lop_BNE_F2_FS:
      return Lopcode_BNE_F2_FS;
    case Lop_BGT_F2:
      return Lopcode_BGT_F2;
    case Lop_BGT_F2_FS:
      return Lopcode_BGT_F2_FS;
    case Lop_BGE_F2:
      return Lopcode_BGE_F2;
    case Lop_BGE_F2_FS:
      return Lopcode_BGE_F2_FS;
    case Lop_BLT_F2:
      return Lopcode_BLT_F2;
    case Lop_BLT_F2_FS:
      return Lopcode_BLT_F2_FS;
    case Lop_BLE_F2:
      return Lopcode_BLE_F2;
    case Lop_BLE_F2_FS:
      return Lopcode_BLE_F2_FS;
    case Lop_PBR:
      return Lopcode_PBR;
    case Lop_MOV:
      return Lopcode_MOV;
    case Lop_MOV_F:
      return Lopcode_MOV_F;
    case Lop_MOV_F2:
      return Lopcode_MOV_F2;
    case Lop_ADD:
      return Lopcode_ADD;
    case Lop_ADD_U:
      return Lopcode_ADD_U;
    case Lop_SUB:
      return Lopcode_SUB;
    case Lop_SUB_U:
      return Lopcode_SUB_U;
    case Lop_MUL:
      return Lopcode_MUL;
    case Lop_MUL_U:
      return Lopcode_MUL_U;
    case Lop_DIV:
      return Lopcode_DIV;
    case Lop_DIV_U:
      return Lopcode_DIV_U;
    case Lop_REM:
      return Lopcode_REM;
    case Lop_REM_U:
      return Lopcode_REM_U;
    case Lop_ABS:
      return Lopcode_ABS;
    case Lop_MUL_ADD:
      return Lopcode_MUL_ADD;
    case Lop_MUL_ADD_U:
      return Lopcode_MUL_ADD_U;
    case Lop_MUL_SUB:
      return Lopcode_MUL_SUB;
    case Lop_MUL_SUB_U:
      return Lopcode_MUL_SUB_U;
    case Lop_MUL_SUB_REV:
      return Lopcode_MUL_SUB_REV;
    case Lop_MUL_SUB_REV_U:
      return Lopcode_MUL_SUB_REV_U;
    case Lop_MAX:
      return Lopcode_MAX;
    case Lop_MIN:
      return Lopcode_MIN;
    case Lop_OR:
      return Lopcode_OR;
    case Lop_AND:
      return Lopcode_AND;
    case Lop_XOR:
      return Lopcode_XOR;
    case Lop_NOR:
      return Lopcode_NOR;
    case Lop_NAND:
      return Lopcode_NAND;
    case Lop_NXOR:
      return Lopcode_NXOR;
    case Lop_OR_NOT:
      return Lopcode_OR_NOT;
    case Lop_AND_NOT:
      return Lopcode_AND_NOT;
    case Lop_OR_COMPL:
      return Lopcode_OR_COMPL;
    case Lop_AND_COMPL:
      return Lopcode_AND_COMPL;
    case Lop_EQ:
      return Lopcode_EQ;
    case Lop_NE:
      return Lopcode_NE;
    case Lop_GT:
      return Lopcode_GT;
    case Lop_GT_U:
      return Lopcode_GT_U;
    case Lop_GE:
      return Lopcode_GE;
    case Lop_GE_U:
      return Lopcode_GE_U;
    case Lop_LT:
      return Lopcode_LT;
    case Lop_LT_U:
      return Lopcode_LT_U;
    case Lop_LE:
      return Lopcode_LE;
    case Lop_LE_U:
      return Lopcode_LE_U;
    case Lop_LSL:
      return Lopcode_LSL;
    case Lop_LSLADD:
      return Lopcode_LSLADD;
    case Lop_SXT_C:
      return Lopcode_SXT_C;
    case Lop_SXT_C2:
      return Lopcode_SXT_C2;
    case Lop_SXT_I:
      return Lopcode_SXT_I;
    case Lop_ZXT_C:
      return Lopcode_ZXT_C;
    case Lop_ZXT_C2:
      return Lopcode_ZXT_C2;
    case Lop_ZXT_I:
      return Lopcode_ZXT_I;
    case Lop_LSR:
      return Lopcode_LSR;
    case Lop_ASR:
      return Lopcode_ASR;
    case Lop_REV:
      return Lopcode_REV;
    case Lop_BIT_POS:
      return Lopcode_BIT_POS;
    case Lop_ADD_F2:
      return Lopcode_ADD_F2;
    case Lop_SUB_F2:
      return Lopcode_SUB_F2;
    case Lop_MUL_F2:
      return Lopcode_MUL_F2;
    case Lop_DIV_F2:
      return Lopcode_DIV_F2;
    case Lop_RCP_F2:
      return Lopcode_RCP_F2;
    case Lop_ABS_F2:
      return Lopcode_ABS_F2;
    case Lop_MUL_ADD_F2:
      return Lopcode_MUL_ADD_F2;
    case Lop_MUL_SUB_F2:
      return Lopcode_MUL_SUB_F2;
    case Lop_MUL_SUB_REV_F2:
      return Lopcode_MUL_SUB_REV_F2;
    case Lop_SQRT_F2:
      return Lopcode_SQRT_F2;
    case Lop_MAX_F2:
      return Lopcode_MAX_F2;
    case Lop_MIN_F2:
      return Lopcode_MIN_F2;
    case Lop_EQ_F2:
      return Lopcode_EQ_F2;
    case Lop_NE_F2:
      return Lopcode_NE_F2;
    case Lop_GT_F2:
      return Lopcode_GT_F2;
    case Lop_GE_F2:
      return Lopcode_GE_F2;
    case Lop_LT_F2:
      return Lopcode_LT_F2;
    case Lop_LE_F2:
      return Lopcode_LE_F2;
    case Lop_ADD_F:
      return Lopcode_ADD_F;
    case Lop_SUB_F:
      return Lopcode_SUB_F;
    case Lop_MUL_F:
      return Lopcode_MUL_F;
    case Lop_DIV_F:
      return Lopcode_DIV_F;
    case Lop_RCP_F:
      return Lopcode_RCP_F;
    case Lop_ABS_F:
      return Lopcode_ABS_F;
    case Lop_MUL_ADD_F:
      return Lopcode_MUL_ADD_F;
    case Lop_MUL_SUB_F:
      return Lopcode_MUL_SUB_F;
    case Lop_MUL_SUB_REV_F:
      return Lopcode_MUL_SUB_REV_F;
    case Lop_SQRT_F:
      return Lopcode_SQRT_F;
    case Lop_MAX_F:
      return Lopcode_MAX_F;
    case Lop_MIN_F:
      return Lopcode_MIN_F;
    case Lop_EQ_F:
      return Lopcode_EQ_F;
    case Lop_NE_F:
      return Lopcode_NE_F;
    case Lop_GT_F:
      return Lopcode_GT_F;
    case Lop_GE_F:
      return Lopcode_GE_F;
    case Lop_LT_F:
      return Lopcode_LT_F;
    case Lop_LE_F:
      return Lopcode_LE_F;
    case Lop_F2_I:
      return Lopcode_F2_I;
    case Lop_I_F2:
      return Lopcode_I_F2;
    case Lop_F_I:
      return Lopcode_F_I;
    case Lop_I_F:
      return Lopcode_I_F;
    case Lop_F2_F:
      return Lopcode_F2_F;
    case Lop_F_F2:
      return Lopcode_F_F2;
    case Lop_LD_UC:
      return Lopcode_LD_UC;
    case Lop_LD_C:
      return Lopcode_LD_C;
    case Lop_LD_UC2:
      return Lopcode_LD_UC2;
    case Lop_LD_C2:
      return Lopcode_LD_C2;
    case Lop_LD_I:
      return Lopcode_LD_I;
    case Lop_LD_UI:
      return Lopcode_LD_UI;
    case Lop_LD_Q:
      return Lopcode_LD_Q;
    case Lop_LD_F:
      return Lopcode_LD_F;
    case Lop_LD_F2:
      return Lopcode_LD_F2;
    case Lop_LD_PRE_UC:
      return Lopcode_LD_PRE_UC;
    case Lop_LD_PRE_C:
      return Lopcode_LD_PRE_C;
    case Lop_LD_PRE_UC2:
      return Lopcode_LD_PRE_UC2;
    case Lop_LD_PRE_C2:
      return Lopcode_LD_PRE_C2;
    case Lop_LD_PRE_I:
      return Lopcode_LD_PRE_I;
    case Lop_LD_PRE_UI:
      return Lopcode_LD_PRE_UI;
    case Lop_LD_PRE_Q:
      return Lopcode_LD_PRE_Q;
    case Lop_LD_PRE_F:
      return Lopcode_LD_PRE_F;
    case Lop_LD_PRE_F2:
      return Lopcode_LD_PRE_F2;
    case Lop_LD_POST_UC:
      return Lopcode_LD_POST_UC;
    case Lop_LD_POST_C:
      return Lopcode_LD_POST_C;
    case Lop_LD_POST_UC2:
      return Lopcode_LD_POST_UC2;
    case Lop_LD_POST_C2:
      return Lopcode_LD_POST_C2;
    case Lop_LD_POST_I:
      return Lopcode_LD_POST_I;
    case Lop_LD_POST_UI:
      return Lopcode_LD_POST_UI;
    case Lop_LD_POST_Q:
      return Lopcode_LD_POST_Q;
    case Lop_LD_POST_F:
      return Lopcode_LD_POST_F;
    case Lop_LD_POST_F2:
      return Lopcode_LD_POST_F2;
    case Lop_ST_C:
      return Lopcode_ST_C;
    case Lop_ST_C2:
      return Lopcode_ST_C2;
    case Lop_ST_I:
      return Lopcode_ST_I;
    case Lop_ST_Q:
      return Lopcode_ST_Q;
    case Lop_ST_F:
      return Lopcode_ST_F;
    case Lop_ST_F2:
      return Lopcode_ST_F2;
    case Lop_ST_PRE_C:
      return Lopcode_ST_PRE_C;
    case Lop_ST_PRE_C2:
      return Lopcode_ST_PRE_C2;
    case Lop_ST_PRE_I:
      return Lopcode_ST_PRE_I;
    case Lop_ST_PRE_Q:
      return Lopcode_ST_PRE_Q;
    case Lop_ST_PRE_F:
      return Lopcode_ST_PRE_F;
    case Lop_ST_PRE_F2:
      return Lopcode_ST_PRE_F2;
    case Lop_ST_POST_C:
      return Lopcode_ST_POST_C;
    case Lop_ST_POST_C2:
      return Lopcode_ST_POST_C2;
    case Lop_ST_POST_I:
      return Lopcode_ST_POST_I;
    case Lop_ST_POST_Q:
      return Lopcode_ST_POST_Q;
    case Lop_ST_POST_F:
      return Lopcode_ST_POST_F;
    case Lop_ST_POST_F2:
      return Lopcode_ST_POST_F2;
    case Lop_EXTRACT_C:
      return Lopcode_EXTRACT_C;
    case Lop_EXTRACT_C2:
      return Lopcode_EXTRACT_C2;
    case Lop_EXTRACT:
      return Lopcode_EXTRACT;
    case Lop_EXTRACT_U:
      return Lopcode_EXTRACT_U;
    case Lop_DEPOSIT:
      return Lopcode_DEPOSIT;
    case Lop_FETCH_AND_ADD:
      return Lopcode_FETCH_AND_ADD;
    case Lop_FETCH_AND_OR:
      return Lopcode_FETCH_AND_OR;
    case Lop_FETCH_AND_AND:
      return Lopcode_FETCH_AND_AND;
    case Lop_FETCH_AND_ST:
      return Lopcode_FETCH_AND_ST;
    case Lop_FETCH_AND_COND_ST:
      return Lopcode_FETCH_AND_COND_ST;
    case Lop_ADVANCE:
      return Lopcode_ADVANCE;
    case Lop_AWAIT:
      return Lopcode_AWAIT;
    case Lop_MUTEX_B:
      return Lopcode_MUTEX_B;
    case Lop_MUTEX_E:
      return Lopcode_MUTEX_E;
    case Lop_CO_PROC:
      return Lopcode_CO_PROC;
    case Lop_CHECK:
      return Lopcode_CHECK;
    case Lop_CONFIRM:
      return Lopcode_CONFIRM;
    case Lop_PRED_CLEAR:
      return Lopcode_PRED_CLEAR;
    case Lop_PRED_SET:
      return Lopcode_PRED_SET;
    case Lop_PRED_LD:
      return Lopcode_PRED_LD;
    case Lop_PRED_ST:
      return Lopcode_PRED_ST;
    case Lop_PRED_LD_BLK:
      return Lopcode_PRED_LD_BLK;
    case Lop_PRED_ST_BLK:
      return Lopcode_PRED_ST_BLK;
    case Lop_PRED_MERGE:
      return Lopcode_PRED_MERGE;
    case Lop_PRED_AND:
      return Lopcode_PRED_AND;
    case Lop_PRED_COMPL:
      return Lopcode_PRED_COMPL;
    case Lop_PRED_COPY:
      return Lopcode_PRED_COPY;
    case Lop_CMP:
      return Lopcode_CMP;
    case Lop_CMP_F:
      return Lopcode_CMP_F;
    case Lop_RCMP:
      return Lopcode_RCMP;
    case Lop_RCMP_F:
      return Lopcode_RCMP_F;
    case Lop_PRED_EQ:
      return Lopcode_PRED_EQ;
    case Lop_PRED_NE:
      return Lopcode_PRED_NE;
    case Lop_PRED_GT:
      return Lopcode_PRED_GT;
    case Lop_PRED_GT_U:
      return Lopcode_PRED_GT_U;
    case Lop_PRED_GE:
      return Lopcode_PRED_GE;
    case Lop_PRED_GE_U:
      return Lopcode_PRED_GE_U;
    case Lop_PRED_LT:
      return Lopcode_PRED_LT;
    case Lop_PRED_LT_U:
      return Lopcode_PRED_LT_U;
    case Lop_PRED_LE:
      return Lopcode_PRED_LE;
    case Lop_PRED_LE_U:
      return Lopcode_PRED_LE_U;
    case Lop_PRED_EQ_F2:
      return Lopcode_PRED_EQ_F2;
    case Lop_PRED_NE_F2:
      return Lopcode_PRED_NE_F2;
    case Lop_PRED_GT_F2:
      return Lopcode_PRED_GT_F2;
    case Lop_PRED_GE_F2:
      return Lopcode_PRED_GE_F2;
    case Lop_PRED_LT_F2:
      return Lopcode_PRED_LT_F2;
    case Lop_PRED_LE_F2:
      return Lopcode_PRED_LE_F2;
    case Lop_PRED_EQ_F:
      return Lopcode_PRED_EQ_F;
    case Lop_PRED_NE_F:
      return Lopcode_PRED_NE_F;
    case Lop_PRED_GT_F:
      return Lopcode_PRED_GT_F;
    case Lop_PRED_GE_F:
      return Lopcode_PRED_GE_F;
    case Lop_PRED_LT_F:
      return Lopcode_PRED_LT_F;
    case Lop_PRED_LE_F:
      return Lopcode_PRED_LE_F;
    case Lop_PRED_MASK_AND:
      return Lopcode_PRED_MASK_AND;
    case Lop_PRED_MASK_OR:
      return Lopcode_PRED_MASK_OR;
    case Lop_CMOV:
      return Lopcode_CMOV;
    case Lop_CMOV_COM:
      return Lopcode_CMOV_COM;
    case Lop_CMOV_F:
      return Lopcode_CMOV_F;
    case Lop_CMOV_COM_F:
      return Lopcode_CMOV_COM_F;
    case Lop_CMOV_F2:
      return Lopcode_CMOV_F2;
    case Lop_CMOV_COM_F2:
      return Lopcode_CMOV_COM_F2;
    case Lop_SELECT:
      return Lopcode_SELECT;
    case Lop_SELECT_F:
      return Lopcode_SELECT_F;
    case Lop_SELECT_F2:
      return Lopcode_SELECT_F2;
    case Lop_PREF_LD:
      return Lopcode_PREF_LD;
    case Lop_JSR_ND:
      return Lopcode_JSR_ND;
    case Lop_EXPAND:
      return Lopcode_EXPAND;
    case Lop_MEM_COPY:
      return Lopcode_MEM_COPY;
    case Lop_MEM_COPY_BACK:
      return Lopcode_MEM_COPY_BACK;
    case Lop_MEM_COPY_CHECK:
      return Lopcode_MEM_COPY_CHECK;
    case Lop_MEM_COPY_RESET:
      return Lopcode_MEM_COPY_RESET;
    case Lop_MEM_COPY_SETUP:
      return Lopcode_MEM_COPY_SETUP;
    case Lop_MEM_COPY_TAG:
      return Lopcode_MEM_COPY_TAG;
    case Lop_SIM_DIR:
      return Lopcode_SIM_DIR;
    case Lop_BOUNDARY:
      return Lopcode_BOUNDARY;
    case Lop_REMAP:
      return Lopcode_REMAP;
    case Lop_BIT_EXTRACT:
      return Lopcode_BIT_EXTRACT;
    case Lop_BIT_DEPOSIT:
      return Lopcode_BIT_DEPOSIT;

    case Lop_INTRINSIC:
      return Lopcode_INTRINSIC; /* ITI/JWJ 7/99 */

      /* ITI/JWJ 8.11.1999 */
    case Lop_L_MAC:
      return Lopcode_L_MAC;
    case Lop_L_MSU:
      return Lopcode_L_MSU;
    case Lop_ADD_SAT:
      return Lopcode_ADD_SAT;
    case Lop_ADD_SAT_U:
      return Lopcode_ADD_SAT_U;
    case Lop_SUB_SAT:
      return Lopcode_SUB_SAT;
    case Lop_SUB_SAT_U:
      return Lopcode_SUB_SAT_U;
    case Lop_MUL_SAT:
      return Lopcode_MUL_SAT;
    case Lop_MUL_SAT_U:
      return Lopcode_MUL_SAT_U;
    case Lop_SAT:
      return Lopcode_SAT;
    case Lop_SAT_U:
      return Lopcode_SAT_U;

    case Lop_LD_UC_CHK:
      return Lopcode_LD_UC_CHK;
    case Lop_LD_C_CHK:
      return Lopcode_LD_C_CHK;
    case Lop_LD_UC2_CHK:
      return Lopcode_LD_UC2_CHK;
    case Lop_LD_C2_CHK:
      return Lopcode_LD_C2_CHK;
    case Lop_LD_UI_CHK:
      return Lopcode_LD_UI_CHK;
    case Lop_LD_I_CHK:
      return Lopcode_LD_I_CHK;
    case Lop_LD_Q_CHK:
      return Lopcode_LD_Q_CHK;
    case Lop_LD_F_CHK:
      return Lopcode_LD_F_CHK;
    case Lop_LD_F2_CHK:
      return Lopcode_LD_F2_CHK;
    case Lop_CHECK_ALAT:
      return Lopcode_CHECK_ALAT;
    case Lop_ADD_CARRY:
      return Lopcode_ADD_CARRY;
    case Lop_ADD_CARRY_U:
      return Lopcode_ADD_CARRY_U;
    case Lop_SUB_CARRY:
      return Lopcode_SUB_CARRY;
    case Lop_SUB_CARRY_U:
      return Lopcode_SUB_CARRY_U;
    case Lop_MUL_WIDE:
      return Lopcode_MUL_WIDE;
    case Lop_MUL_WIDE_U:
      return Lopcode_MUL_WIDE_U;

    // SLARSEN: Vector opcodes
    case Lop_VADD:		return Lopcode_VADD;
    case Lop_VADD_U:		return Lopcode_VADD_U;
    case Lop_VSUB:		return Lopcode_VSUB;
    case Lop_VSUB_U:		return Lopcode_VSUB_U;
    case Lop_VMUL:		return Lopcode_VMUL;
    case Lop_VMUL_U:		return Lopcode_VMUL_U;
    case Lop_VDIV:		return Lopcode_VDIV;
    case Lop_VDIV_U:		return Lopcode_VDIV_U;
    case Lop_VREM:		return Lopcode_VREM;
    case Lop_VREM_U:		return Lopcode_VREM_U;
    case Lop_VMIN:		return Lopcode_VMIN;
    case Lop_VMAX:		return Lopcode_VMAX;
    case Lop_VOR:		return Lopcode_VOR;
    case Lop_VAND:		return Lopcode_VAND;
    case Lop_VADD_F:		return Lopcode_VADD_F;
    case Lop_VSUB_F:		return Lopcode_VSUB_F;
    case Lop_VMUL_F:		return Lopcode_VMUL_F;
    case Lop_VDIV_F:		return Lopcode_VDIV_F;
    case Lop_VABS_F:		return Lopcode_VABS_F;
    case Lop_VSQRT_F:		return Lopcode_VSQRT_F;
    case Lop_VMAX_F:		return Lopcode_VMAX_F;
    case Lop_VMIN_F:		return Lopcode_VMIN_F;
    case Lop_VADD_F2:		return Lopcode_VADD_F2;
    case Lop_VSUB_F2:		return Lopcode_VSUB_F2;
    case Lop_VMUL_F2:		return Lopcode_VMUL_F2;
    case Lop_VDIV_F2:		return Lopcode_VDIV_F2;
    case Lop_VABS_F2:		return Lopcode_VABS_F2;
    case Lop_VSQRT_F2:		return Lopcode_VSQRT_F2;
    case Lop_VMAX_F2:		return Lopcode_VMAX_F2;
    case Lop_VMIN_F2:		return Lopcode_VMIN_F2;
    case Lop_VMOV:		return Lopcode_VMOVE;
    case Lop_VMOV_F:		return Lopcode_VMOVE_F;
    case Lop_VMOV_F2:		return Lopcode_VMOVE_F2;
    case Lop_VI_VF:		return Lopcode_VI_VF;
    case Lop_VI_VF2:		return Lopcode_VI_VF2;
    case Lop_VF_VI:		return Lopcode_VF_VI;
    case Lop_VF2_VI:		return Lopcode_VF2_VI;
    case Lop_VF_VF2:		return Lopcode_VF_VF2;
    case Lop_VF2_VF:		return Lopcode_VF2_VF;
    case Lop_VI_I:		return Lopcode_VI_I;
    case Lop_I_VI:		return Lopcode_I_VI;
    case Lop_VF_F:		return Lopcode_VF_F;
    case Lop_F_VF:		return Lopcode_F_VF;
    case Lop_VF2_F2:		return Lopcode_VF2_F2;
    case Lop_F2_VF2:		return Lopcode_F2_VF2;
    case Lop_VPERM:		return Lopcode_VPERM;
    case Lop_VPERM_F:		return Lopcode_VPERM_F;
    case Lop_VPERM_F2:		return Lopcode_VPERM_F2;
    case Lop_VSPLAT:		return Lopcode_VSPLAT;
    case Lop_VSPLAT_F:		return Lopcode_VSPLAT_F;
    case Lop_VSPLAT_F2:		return Lopcode_VSPLAT_F2;
    case Lop_VLD_UC:		return Lopcode_VLD_UC;
    case Lop_VLD_C:		return Lopcode_VLD_C;
    case Lop_VLD_UC2:		return Lopcode_VLD_UC2;
    case Lop_VLD_C2:		return Lopcode_VLD_C2;
    case Lop_VLD_I:		return Lopcode_VLD_I;
    case Lop_VLD_F:		return Lopcode_VLD_F;
    case Lop_VLD_F2:		return Lopcode_VLD_F2;
    case Lop_VST_C:		return Lopcode_VST_C;
    case Lop_VST_C2:		return Lopcode_VST_C2;
    case Lop_VST_I:		return Lopcode_VST_I;
    case Lop_VST_F:		return Lopcode_VST_F;
    case Lop_VST_F2:		return Lopcode_VST_F2;
    case Lop_VLDE_UC:		return Lopcode_VLDE_UC;
    case Lop_VLDE_C:		return Lopcode_VLDE_C;
    case Lop_VLDE_UC2:		return Lopcode_VLDE_UC2;
    case Lop_VLDE_C2:		return Lopcode_VLDE_C2;
    case Lop_VLDE_I:		return Lopcode_VLDE_I;
    case Lop_VLDE_F:		return Lopcode_VLDE_F;
    case Lop_VLDE_F2:		return Lopcode_VLDE_F2;
    case Lop_VSTE_C:		return Lopcode_VSTE_C;
    case Lop_VSTE_C2:		return Lopcode_VSTE_C2;
    case Lop_VSTE_I:		return Lopcode_VSTE_I;
    case Lop_VSTE_F:		return Lopcode_VSTE_F;
    case Lop_VSTE_F2:		return Lopcode_VSTE_F2;
    case Lop_VEXTRACT_C:	return Lopcode_VEXTRACT_C;
    case Lop_VEXTRACT_C2:	return Lopcode_VEXTRACT_C2;
    case Lop_PUSH:              return Lopcode_PUSH;
    case Lop_POP:               return Lopcode_POP;
    case Lop_PEEK:              return Lopcode_PEEK;
                                

    default:
      /* machine specific macro names */
      return M_get_opcode_name (id);
    }
}

char *
L_cmp_compl_name (ITuint8 id)
{
  switch (id)
    {
    case Lcmp_COM_EQ:
      return Lcompl_COM_EQ;
    case Lcmp_COM_NE:
      return Lcompl_COM_NE;
    case Lcmp_COM_GT:
      return Lcompl_COM_GT;
    case Lcmp_COM_LE:
      return Lcompl_COM_LE;
    case Lcmp_COM_GE:
      return Lcompl_COM_GE;
    case Lcmp_COM_LT:
      return Lcompl_COM_LT;
    case Lcmp_COM_TZ:
      return Lcompl_COM_TZ;
    case Lcmp_COM_TN:
      return Lcompl_COM_TN;
    default:
      L_punt ("L_cmp_compl_name: Illegal compare completer: %d.", id);
      return 0;
    }
}

/*========================================================================*/
/*
 *      OPER Hash Table Functions
 */
/*========================================================================*/

#define L_OPER_HASH_TBL_MASK            (L_oper_hash_tbl_size - 1)

L_Oper_Hash_Entry **
L_oper_hash_tbl_create ()
{
  int i;
  L_Oper_Hash_Entry **oper_hash_tbl;

  oper_hash_tbl = (L_Oper_Hash_Entry **) L_alloc (L_alloc_oper_hash_tbl);

  for (i = 0; i < L_oper_hash_tbl_size; i++)
    {
      oper_hash_tbl[i] = NULL;
    }

  return (oper_hash_tbl);
}

void
L_oper_hash_tbl_insert (L_Oper_Hash_Entry ** oper_hash_tbl, L_Oper * oper)
{
  int hash_id;
  L_Oper_Hash_Entry *entry;

  hash_id = oper->id & L_OPER_HASH_TBL_MASK;
  entry = L_new_oper_hash_entry (oper->id, oper);
  entry->next_oper_hash = oper_hash_tbl[hash_id];
  if (oper_hash_tbl[hash_id] != NULL)
    oper_hash_tbl[hash_id]->prev_oper_hash = entry;
  oper_hash_tbl[hash_id] = entry;
}

void
L_oper_hash_tbl_delete (L_Oper_Hash_Entry ** oper_hash_tbl, L_Oper * oper)
{
  int hash_id;
  L_Oper_Hash_Entry *entry;

  hash_id = oper->id & L_OPER_HASH_TBL_MASK;

  /* find the appropriate entry */
  entry = L_find_oper_hash_entry (oper_hash_tbl[hash_id], oper->id);
  if (entry == NULL)
    L_punt ("L_oper_hash_tbl_delete: entry for oper not found");

  oper_hash_tbl[hash_id] =
    L_delete_oper_hash_entry (oper_hash_tbl[hash_id], entry);
}

/*
 *      Special case to allow entry for parent oper which is automatically
 *      created to be removed, because it shouldn't be there!!
 */
void
L_oper_hash_tbl_delete_specific (L_Oper_Hash_Entry ** oper_hash_tbl,
                                 L_Oper * oper)
{
  int hash_id;
  L_Oper_Hash_Entry *entry;

  hash_id = oper->id & L_OPER_HASH_TBL_MASK;

  /* find the appropriate entry */
  entry = L_find_specific_oper_hash_entry (oper_hash_tbl[hash_id], oper);
  if (entry == NULL)
    L_punt ("L_oper_hash_tbl_delete_specific: entry for oper not found");

  oper_hash_tbl[hash_id] =
    L_delete_oper_hash_entry (oper_hash_tbl[hash_id], entry);
}


void
L_oper_hash_tbl_delete_all (L_Oper_Hash_Entry ** oper_hash_tbl)
{
  int i;

  for (i = 0; i < L_oper_hash_tbl_size; i++)
    {
      if (oper_hash_tbl[i] != NULL)
        {
          L_delete_all_oper_hash_entry (oper_hash_tbl[i]);
          oper_hash_tbl[i] = NULL;
        }
    }
}

void
L_oper_hash_tbl_update_cb (L_Oper_Hash_Entry ** oper_hash_tbl, int op_id,
                           L_Cb * cb)
{
  int hash_id;
  L_Oper_Hash_Entry *entry;

  hash_id = op_id & L_OPER_HASH_TBL_MASK;
  entry = L_find_oper_hash_entry (oper_hash_tbl[hash_id], op_id);

  if (entry == NULL)
    L_punt ("L_oper_hash_tbl_update_cb: entry for op_id not found");

  entry->cb = cb;
}

L_Oper *
L_oper_hash_tbl_find_oper (L_Oper_Hash_Entry ** oper_hash_tbl, int op_id)
{
  int hash_id;
  L_Oper_Hash_Entry *entry;

  hash_id = op_id & L_OPER_HASH_TBL_MASK;
  entry = L_find_oper_hash_entry (oper_hash_tbl[hash_id], op_id);
  if (entry)
    return (entry->oper);
  else
    return (NULL);
}


L_Oper *
L_oper_hash_tbl_find_and_alloc_oper (L_Oper_Hash_Entry ** oper_hash_tbl,
                                     int op_id)
{
  L_Oper *oper;

  oper = L_oper_hash_tbl_find_oper (oper_hash_tbl, op_id);
  if (oper == NULL)
    oper = L_new_oper (op_id);

  return (oper);
}


L_Cb *
L_oper_hash_tbl_find_cb (L_Oper_Hash_Entry ** oper_hash_tbl, int op_id)
{
  int hash_id;
  L_Oper_Hash_Entry *entry;

  hash_id = op_id & L_OPER_HASH_TBL_MASK;
  entry = L_find_oper_hash_entry (oper_hash_tbl[hash_id], op_id);
  if (entry)
    return (entry->cb);
  else
    return (NULL);
}

/*
 *      Error checking routine!!
 */
void
L_oper_hash_tbl_check (L_Func * fn)
{
  int i, count;
  L_Cb *cb, *cb_ptr;
  L_Oper *oper, *oper_ptr;
  L_Oper_Hash_Entry **oper_hash_tbl, *entry;

  oper_hash_tbl = fn->oper_hash_tbl;

  /* check that all opers in the hash tbl */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          cb_ptr = L_oper_hash_tbl_find_cb (oper_hash_tbl, oper->id);
          if (cb_ptr != cb)
            L_punt ("L_oper_hash_tbl_check: corrupt cb ptr for op %d",
	            oper->id);
          oper_ptr = L_oper_hash_tbl_find_oper (oper_hash_tbl, oper->id);
          if (oper_ptr != oper)
            L_punt ("L_oper_hash_tbl_check: corrupt oper ptr for op %d",
                    oper->id);
        }
    }

  /* check that number of entries = fn->num_op */
  count = 0;
  for (i = 0; i < L_oper_hash_tbl_size; i++)
    {
      if (oper_hash_tbl[i] == NULL)
        continue;
      for (entry = oper_hash_tbl[i]; entry != NULL;
           entry = entry->next_oper_hash)
        {
          count++;
        }
    }

  if (count != fn->n_oper)
    {
      fprintf (stderr, "num oper in hash tbl %d, fn->n_oper %d\n",
               count, fn->n_oper);
      L_punt ("L_oper_hash_tbl_check: illegal number of entries");
    }
}

/*
 *      Rebuild the blasted thing from scratch, only use if you do something
 *      to really screw it up.
 */
void
L_oper_hash_tbl_rebuild (L_Func * fn)
{
  L_Oper_Hash_Entry **oper_hash_tbl;
  L_Oper *oper;
  L_Cb *cb;

  oper_hash_tbl = fn->oper_hash_tbl;
  L_oper_hash_tbl_delete_all (oper_hash_tbl);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          L_oper_hash_tbl_insert (oper_hash_tbl, oper);
          L_oper_hash_tbl_update_cb (oper_hash_tbl, oper->id, cb);
        }
    }
}

/*========================================================================*/
/*
 *      CB hash table functions
 */
/*========================================================================*/

#define L_CB_HASH_TBL_MASK              (L_cb_hash_tbl_size - 1)

L_Cb_Hash_Entry **
L_cb_hash_tbl_create ()
{
  int i;
  L_Cb_Hash_Entry **cb_hash_tbl;

  cb_hash_tbl = (L_Cb_Hash_Entry **) L_alloc (L_alloc_cb_hash_tbl);

  for (i = 0; i < L_cb_hash_tbl_size; i++)
    {
      cb_hash_tbl[i] = NULL;
    }

  return cb_hash_tbl;
}

void
L_cb_hash_tbl_insert (L_Cb_Hash_Entry ** cb_hash_tbl, L_Cb * cb)
{
  int hash_id;
  L_Cb_Hash_Entry *entry;

  hash_id = cb->id & L_CB_HASH_TBL_MASK;
  entry = L_new_cb_hash_entry (cb->id, cb);
  entry->next_cb_hash = cb_hash_tbl[hash_id];
  if (cb_hash_tbl[hash_id] != NULL)
    cb_hash_tbl[hash_id]->prev_cb_hash = entry;
  cb_hash_tbl[hash_id] = entry;
}

void
L_cb_hash_tbl_delete (L_Cb_Hash_Entry ** cb_hash_tbl, L_Cb * cb)
{
  int hash_id;
  L_Cb_Hash_Entry *entry;

  hash_id = cb->id & L_CB_HASH_TBL_MASK;

  entry = L_find_cb_hash_entry (cb_hash_tbl[hash_id], cb->id);
  if (entry == NULL)
    L_punt ("L_cb_hash_tbl_delete: entry for cb not found");

  cb_hash_tbl[hash_id] = L_delete_cb_hash_entry (cb_hash_tbl[hash_id], entry);
}

void
L_cb_hash_tbl_delete_all (L_Cb_Hash_Entry ** cb_hash_tbl)
{
  int i;

  for (i = 0; i < L_cb_hash_tbl_size; i++)
    {
      if (cb_hash_tbl[i] != NULL)
        {
          L_delete_all_cb_hash_entry (cb_hash_tbl[i]);
          cb_hash_tbl[i] = NULL;
        }
    }
}

L_Cb *
L_cb_hash_tbl_find (L_Cb_Hash_Entry ** cb_hash_tbl, int cb_id)
{
  int hash_id;
  L_Cb_Hash_Entry *entry;

  hash_id = cb_id & L_CB_HASH_TBL_MASK;
  entry = L_find_cb_hash_entry (cb_hash_tbl[hash_id], cb_id);
  if (entry)
    return (entry->cb);
  else
    return (NULL);
}

/*
 *      Make sure all cb's in hash tbl are present in Lcode
 */
void
L_cb_hash_tbl_check (L_Cb_Hash_Entry ** cb_hash_tbl)
{
  int i, num_cb;
  L_Cb_Hash_Entry *entry;
  L_Cb *cb;

  num_cb = 0;
  for (i = 0; i < L_cb_hash_tbl_size; i++)
    {
      if (cb_hash_tbl[i] != NULL)
        {
          for (entry = cb_hash_tbl[i]; entry != NULL;
          entry = entry->next_cb_hash)
            {
              num_cb++;
              cb = entry->cb;
#if 0
              if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_PRESENCE) != 1)
              L_punt ("L_cb_hash_tbl_check: cb %d not present", cb->id);
#endif
            }
        }
    }

  if (num_cb != L_fn->n_cb)
    {
      fprintf (stderr, "num cb in hash tbl %d, L_fn->n_cb %d\n", num_cb,
               L_fn->n_cb);
      L_punt ("L_cb_hash_tbl_check: illegal number entries");
    }
}

/*
 *      Locate and allocate if not there
 */
L_Cb *
L_cb_hash_tbl_find_and_alloc (L_Cb_Hash_Entry ** cb_hash_tbl, int cb_id)
{
  L_Cb *cb;

  cb = L_cb_hash_tbl_find (cb_hash_tbl, cb_id);
  if (cb == NULL)
    {
      cb = L_new_cb (cb_id);
    }

  return cb;
}

/*========================================================================*/
/*
 *      Expression hash table functions
 */
/*========================================================================*/

#define L_EXPRESSION_HASH_TBL_MASK     (L_expression_hash_tbl_size - 1)

void
L_expression_hash_tbl_insert (L_Expression_Hash_Entry ** expression_hash_tbl,
			      L_Expression * expression, int id)
{
  int hash;
  L_Expression_Hash_Entry *entry;

  hash = id & L_EXPRESSION_HASH_TBL_MASK;
  entry = L_new_expression_hash_entry (id, expression);
  entry->next_expression_hash = expression_hash_tbl[hash];
  if (expression_hash_tbl[hash] != NULL)
    expression_hash_tbl[hash]->prev_expression_hash = entry;
  expression_hash_tbl[hash] = entry;

  return;
}


/* Note that this function only checks for a matching hash entry,
 * not a matching expression. More checking is needed to check that.
 * This is mainly used for the index hash tbl, where each id is unique.
 */
L_Expression_Hash_Entry *
L_expression_hash_tbl_find_entry (L_Expression_Hash_Entry **
				  expression_hash_tbl, int id)
{
  int hash_id;
  L_Expression_Hash_Entry *start, *ptr;

  hash_id = id & L_EXPRESSION_HASH_TBL_MASK;
  start = expression_hash_tbl[hash_id];

  for (ptr = start; ptr != NULL; ptr = ptr->next_expression_hash)
    {
      if (ptr->id == id)
	return (ptr);
    }

  return (NULL);
}


void
L_expression_hash_tbl_delete_all (L_Expression_Hash_Entry **
				  expression_hash_tbl)
{
  int i;

  for (i = 0; i < L_expression_hash_tbl_size; i++)
    {
      if (expression_hash_tbl[i] != NULL)
	{
	  L_delete_all_expression_hash_entry (expression_hash_tbl[i]);
	  expression_hash_tbl[i] = NULL;
	}
    }
}


#undef DEBUG_EXPRESSION_CONFLICT_INFO

/* SER 20041213
 * \brief Function updates memory access info for all expressions.
 *
 * \param expression
 *  Newly added expression.
 * \param oper
 *  Oper corresponding to new expression, may have additional AccSpecs.
 * \param self_update
 *  Flag to indicate a need to update expression's AccSpecs from oper.
 */
static void
L_update_expression_conflict_info (L_Expression * expression, L_Oper * oper,
				   int self_update)
{
  L_Expression * old_expression;
  L_Expression_Hash_Entry * entry;
  L_AccSpec *as1, *as2, *new_as;
  int i, store_flag;

  if (L_load_opcode (oper))
    store_flag = 0;
  else if (L_store_opcode (oper))
    store_flag = 1;
  else
    return;

  /* First, add any new objects that weren't accessed by previous
   * occurrences of the expression. */
  if (self_update)
    {
      for (as1 = oper->acc_info; as1; as1 = as1->next)
	{
	  int found = 0;
	  for (as2 = expression->acc_info; as2; as2 = as2->next)
	    {
	      if (as1->id == as2->id && as1->version == as2->version)
		{
		  found = 1;
		  break;
		}
	    }
	  if (found)
	    continue;
	  /* Wasn't in the list, so add to the list. */
	  new_as = L_copy_mem_acc_spec (as1);
	  new_as->next = expression->acc_info;
	  expression->acc_info = new_as;

#ifdef DEBUG_EXPRESSION_CONFLICT_INFO
	  fprintf (stderr, "Expression %d conflict info updated.\n",
		   expression->index);
#endif
	}
    }

  /* Next, check all expressions in the list, seeing if they may potentially
   * access the same object.  If they can, add to each other's conflict sets.
   * The index of the new expression is L_expression_hash_tbl_size, so quit
   * before then.
   */
  for (i = 1; i < L_fn->n_expression; i++)
    {
      entry =
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      if (entry == NULL)
        L_punt ("L_update_expression_conflict_info: missing entry for "
		"expression %d.", i);
      old_expression = entry->expression;
      if (!store_flag)
	{
	  if (!(L_store_opcode (old_expression)))
	    continue;
	  /* Also check for general store assignment: we don't want them to
	   * conflict with each other. */
	  if (old_expression == expression->general ||
	      expression->general == old_expression)
	    continue;
	}
      else if (!(L_load_opcode (old_expression) ||
		 L_store_opcode (old_expression)))
	continue;
      if (expression == old_expression)
	continue;
      if (!L_mem_indep_acc_specs_lists (expression->acc_info,
					old_expression->acc_info))
	{
#ifdef DEBUG_EXPRESSION_CONFLICT_INFO
	  fprintf (stderr, "Expressions %d and %d conflict.\n",
		   expression->index, old_expression->index);
#endif
	  expression->conflicts = Set_add (expression->conflicts,
					   old_expression->index);
	  old_expression->conflicts = Set_add (old_expression->conflicts,
					       expression->index);
	}
    }
}


/* SER 20041214
 * \brief Returns a set of conflicting expressions for a given jsr.
 * \param jsr
 *  JSR to check for alias expressions.
 *
 * \note This function creates a set, so make sure to delete it after use.
 */
Set
L_get_jsr_conflicting_expressions (L_Oper * jsr)
{
  L_Expression * expression;
  L_Expression_Hash_Entry * entry;
  int i;
  Set conflicts = NULL;

  for (i = 1; i <= L_fn->n_expression; i++)
    {
      entry =
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      expression = entry->expression;
#if 0
      if (L_load_opcode (expression))
	load_flag = 1;
      else if (!L_store_opcode (expression))
	continue;
#endif
      if (!(L_load_opcode (expression) || L_store_opcode (expression)))
	continue;
      if (!L_mem_indep_acc_specs_lists (expression->acc_info, jsr->acc_info))
	conflicts = Set_add (conflicts, expression->index);
    }

  return conflicts;
}


/* This function is for use with the token hash table. It
 * checks for exact matches with the expression as well as the input
 * hash id
 */
L_Expression *
L_find_oper_expression_in_hash (L_Expression_Hash_Entry **
				expression_hash_tbl, int token, L_Oper * op,
				int short_flag)
{
  int hash_id;
  L_Expression_Hash_Entry *begin, *ptr;

  hash_id = token & L_EXPRESSION_HASH_TBL_MASK;
  begin = expression_hash_tbl[hash_id];
  for (ptr = begin; ptr != NULL; ptr = ptr->next_expression_hash)
    {
      if (L_oper_matches_expression (op, token, ptr->expression,
				     short_flag, 0))
	{
	  if (L_func_acc_specs)
	    L_update_expression_conflict_info (ptr->expression, op, 1);
	  return (ptr->expression);
	}
    }
  return NULL;
}


L_Expression *
L_find_oper_assignment_in_hash (L_Expression_Hash_Entry **
				assignment_hash_tbl, int token, L_Oper * op,
				int short_flag)
{
  int hash_id;
  L_Expression_Hash_Entry * begin, * ptr;

  hash_id = token & L_EXPRESSION_HASH_TBL_MASK;
  begin = assignment_hash_tbl[hash_id];
  for (ptr = begin; ptr != NULL; ptr = ptr->next_expression_hash)
    {
      if (L_oper_matches_assignment (op, token, ptr->expression, short_flag))
	{
	  if (L_func_acc_specs)
	    L_update_expression_conflict_info (ptr->expression, op, 1);
	  return (ptr->expression);
	}
    }
  return NULL;
}

L_Expression *
L_find_oper_expression_diff_opcode_in_hash (L_Expression_Hash_Entry **
					   expression_hash_tbl, int token,
					   L_Oper * op, int opc)
{
  L_Expression_Hash_Entry *begin, *ptr;
  int hash_id;

  hash_id = token & L_EXPRESSION_HASH_TBL_MASK;
  begin = expression_hash_tbl[hash_id];
  for (ptr = begin; ptr != NULL; ptr = ptr->next_expression_hash)
    {
      if (L_oper_matches_expression (op, token, ptr->expression, 0, opc))
	{
	  if (L_func_acc_specs)
	    L_update_expression_conflict_info (ptr->expression, op, 1);
	  return (ptr->expression);
	}
    }

  return NULL;
}


/* If an oper has a commutative opcode, swap the operands for consistency
 * in partial code elimination.
 */
static void
L_commutative_oper_swap_operands (L_Oper * oper)
{
  L_Operand *operand1, *operand2;

  if (L_commutative_opcode (oper))
    {
      operand1 = oper->src[0];
      operand2 = oper->src[1];

      if (L_is_reg (operand1))
	{
	  if (L_is_reg (operand2))
	    if (operand1->value.r > operand2->value.r)
	      {
		oper->src[0] = operand2;
		oper->src[1] = operand1;
	      }
	}
      else if (L_is_reg (operand2))
	{
	  if (!L_is_reg (operand1))
	    {
	      oper->src[0] = operand2;
	      oper->src[1] = operand1;
	    }
	}
      else if (L_is_macro (operand1))
	{
	  if (L_is_macro (operand2))
	    if (operand1->value.mac > operand2->value.mac)
	      {
		oper->src[0] = operand2;
		oper->src[1] = operand1;
	      }
	}
      else if (L_is_macro (operand2))
	{
	  if (!L_is_macro (operand1))
	    {
	      oper->src[0] = operand2;
	      oper->src[1] = operand1;
	    }
	}
      else if (L_is_int_constant (operand1) &&
	       (L_is_string (operand2) || L_is_label (operand2)))
	{
	  oper->src[0] = operand2;
	  oper->src[1] = operand1;
	}
    }
}


/* Given the input operation, looks for the corresponding expression in
 * L_fn's hash table, creating a new expression if necessary, and returning
 * the expression's index.
 */
int
L_generate_expression_for_oper (L_Oper * oper, int short_flag)
{
  L_Expression *expression;
  int token;

  L_commutative_oper_swap_operands (oper);

  token = L_generate_expression_token_from_oper (oper);
  expression =
    L_find_oper_expression_in_hash (L_fn->expression_token_hash_tbl, 
				    token, oper, short_flag);

  if (expression == NULL)
    {
      expression = L_new_expression (token, oper, short_flag);
      L_expression_hash_tbl_insert (L_fn->expression_token_hash_tbl,
				    expression, token);
      L_expression_hash_tbl_insert (L_fn->expression_index_hash_tbl,
				    expression, expression->index);
    }
  return (expression->index);
}


L_Expression *
L_generate_assignment_for_oper (L_Oper * oper, int short_flag)

{
  L_Expression * assignment;
  int token;

  L_commutative_oper_swap_operands (oper);

  token = L_generate_assignment_token_from_oper (oper, short_flag);
  assignment =
    L_find_oper_assignment_in_hash (L_fn->expression_token_hash_tbl,
				    token, oper, short_flag);

  if (assignment == NULL)
    {
      assignment = L_new_assignment (token, oper, short_flag);
      L_expression_hash_tbl_insert (L_fn->expression_token_hash_tbl,
				    assignment, token);
      L_expression_hash_tbl_insert (L_fn->expression_index_hash_tbl,
				    assignment, assignment->index);
    }
  return (assignment);
}



/* Given a load operation, looks for the corresponding store expression in
 * L_fn's hash table, returning the expression's index. It will be NULL
 * if none exists.
 */
L_Expression *
L_find_corresponding_store_expression_for_load (L_Oper * oper)
{
  L_Expression_Hash_Entry *begin, *ptr;
  int token, hash_id;

  token = L_generate_expression_token_different_opcode (oper, oper->opc);
  hash_id = token & L_EXPRESSION_HASH_TBL_MASK;
  begin =  (L_fn->expression_token_hash_tbl)[hash_id];
  for (ptr = begin; ptr; ptr = ptr->next_expression_hash)
    {
      if (L_load_oper_matches_store_expression (oper, token, ptr->expression))
        {
          return (ptr->expression);
        }
    }
  return NULL;
}

static
L_Expression *
L_generate_expression_mem_complement (L_Oper * oper, int opc)
{
  L_Expression * expression;
  int token;

  token = L_generate_expression_token_different_opcode (oper, opc);
  expression =
    L_find_oper_expression_diff_opcode_in_hash (L_fn->expression_token_hash_tbl,
					       token, oper, opc);

  if (expression == NULL)
    {
      expression = L_new_expression (token, oper, 1);
      expression->opc = opc;
      L_expression_hash_tbl_insert (L_fn->expression_token_hash_tbl,
				    expression, token);
      L_expression_hash_tbl_insert (L_fn->expression_index_hash_tbl,
				    expression, expression->index);
    }

  return expression;
}


/* Given a load or store operation, returns a set containing the indicies of
 * loads which are must-aliased with the store. Unlike the above, it
 * generates the expression if it did not exist previously, because we
 * may not have encountered the load expressions at this point. */
/* REMEMBER to DELETE the set after using it! */
Set
L_create_complement_load_expressions (L_Oper * oper)
{
  L_Expression *load_expr;
  Set loads = NULL;

  switch (oper->opc)
    {
    case Lop_ST_C:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_ST_C2:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC2);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C2);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_ST_I:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC2);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C2);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UI);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_I);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_ST_Q:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC2);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C2);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UI);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_I);
      loads = Set_add (loads, load_expr->index);
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_Q);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_ST_F:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_F);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_ST_F2:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_F2);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_LD_C:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_LD_UC:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_LD_C2:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UC2);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_LD_UC2:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_C2);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_LD_I:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_UI);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_LD_UI:
      load_expr = L_generate_expression_mem_complement (oper, Lop_LD_I);
      loads = Set_add (loads, load_expr->index);
      return loads;
    case Lop_LD_Q:
    case Lop_LD_F:
    case Lop_LD_F2:
      return NULL;
    default:
      L_punt ("Invalid opcode in L_get_complement_load_expressions.");
      return NULL;
    }
}


/*========================================================================*/
/*
 *      Functions to create/delete/update data structure entries
 */
/*========================================================================*/

/*
 *      Global vars
 */

void
L_reset_data_global_vars ()
{
  L_data = NULL;
}

void
L_reset_func_global_vars ()
{
  L_fn = NULL;
  if (L_cnt_oper != NULL)
    {
      free (L_cnt_oper);
      L_cnt_oper = NULL;
    }
  if (L_cnt_flow != NULL)
    {
      free (L_cnt_flow);
      L_cnt_flow = NULL;
    }
  L_n_color_entries_alloc = 0;
  L_n_cnt_oper = 0;
}

/*
 *      L_Expr
 */

L_Expr *
L_new_expr (int type)
{
  L_Expr *expr;

  expr = (L_Expr *) L_alloc (L_alloc_expr);
  expr->type = type;
  expr->A = NULL;
  expr->B = NULL;
  expr->next_expr = NULL;

  return expr;
}

/* WARNING - RECURSIVE ROUTINE */
void
L_delete_expr_element (L_Expr * expr)
{
  if (expr == NULL)
    L_punt ("L_delete_expr_element: trying to delete NULL expr");

  if (expr->A != NULL)
    L_delete_expr_element (expr->A);

  if (expr->B != NULL)
    L_delete_expr_element (expr->B);

  if (expr->next_expr != NULL)
    L_delete_expr_element (expr->next_expr);

  L_free (L_alloc_expr, expr);
}

L_Expr *
L_delete_expr (L_Expr * list, L_Expr * expr)
{
  L_Expr *ptr, *prev, *first = list;

  if (list == NULL)
    L_punt ("L_delete_expr: list is NULL");

  if (expr == NULL)
    return list;

  /* Handle special case where expr is first element of list */
  if (list == expr)
    {
      first = expr->next_expr;
      expr->next_expr = NULL;
      L_delete_expr_element (expr);
      return first;
    }
  else
    {
      /* Find the appropriate entry in the list */
      for (prev = list, ptr = list->next_expr; (ptr != expr) || (ptr != NULL);
	   prev = ptr, ptr = ptr->next_expr);

      if (ptr == NULL)
	L_punt ("L_delete_expr: expr not found in list");

      /* Remove this ptr from the link list */
      prev->next_expr = expr->next_expr;
      expr->next_expr = NULL;

      L_delete_expr_element (expr);

      return list;
    }
}

L_Expr *
L_concat_expr (L_Expr * expr1, L_Expr * expr2)
{
  L_Expr *ptr, *previous;

  if (expr1 == NULL)
    return expr2;
  if (expr2 == NULL)
    return expr1;

  previous = expr1;
  ptr = expr1->next_expr;
  while (ptr != NULL)
    {
      previous = ptr;
      ptr = ptr->next_expr;
    }
  previous->next_expr = expr2;

  return expr1;
}

L_Expr *
L_new_expr_add (L_Expr * expr1, L_Expr * expr2)
{
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_ADD);

  new_expr->A = expr1;
  new_expr->B = expr2;

  return (new_expr);
}

L_Expr *
L_new_expr_mul (L_Expr * expr1, L_Expr * expr2)
{
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_MUL);

  new_expr->A = expr1;
  new_expr->B = expr2;

  return (new_expr);
}


L_Expr *
L_new_expr_sub (L_Expr * expr1, L_Expr * expr2)
{
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_SUB);

  new_expr->A = expr1;
  new_expr->B = expr2;

  return (new_expr);
}


L_Expr *
L_new_expr_div (L_Expr * expr1, L_Expr * expr2)
{
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_DIV);

  new_expr->A = expr1;
  new_expr->B = expr2;

  return (new_expr);
}


L_Expr *
L_new_expr_int (ITintmax value)
{
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_INT);

  new_expr->value.i = value;

  return (new_expr);
}


L_Expr *
L_new_expr_label (char *label)
{
  char name[256], *string_name;
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_LABEL);

  sprintf (name, "_%s", label);
  string_name = C_findstr (name);
  new_expr->value.l = string_name;

  return (new_expr);
}

L_Expr *
L_new_expr_label_no_underscore (char *label)
{
  char *string_name;
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_LABEL);

  string_name = C_findstr (label);
  new_expr->value.l = string_name;

  return (new_expr);
}


L_Expr *
L_new_expr_float (double value)
{
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_FLOAT);

  new_expr->value.f = value;

  return (new_expr);
}


L_Expr *
L_new_expr_double (double value)
{
  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_DOUBLE);

  new_expr->value.f2 = value;

  return (new_expr);
}


L_Expr *
L_new_expr_string (char *string)
{
  char *string_name;

  L_Expr *new_expr;

  new_expr = L_new_expr (L_EXPR_STRING);

  string_name = C_findstr (string);
  new_expr->value.s = string_name;

  return (new_expr);
}

L_Expr *
L_new_expr_addr (char *label, int offset)
{
  L_Expr *new_expr;

  if (offset == 0)
    {
      new_expr = L_new_expr_label (label);
    }
  else
    {
      new_expr = L_new_expr_add (L_new_expr_label (label),
				 L_new_expr_int ((ITintmax) offset));
    }
  return (new_expr);
}

L_Expr *
L_new_expr_addr_no_underscore (char *label, int offset)
{
  L_Expr *new_expr;

  if (offset == 0)
    {
      new_expr = L_new_expr_label_no_underscore (label);
    }
  else
    {
      new_expr = L_new_expr_add (L_new_expr_label_no_underscore (label),
				 L_new_expr_int ((ITintmax) offset));
    }
  return (new_expr);
}



/*
 *      L_Type :      LCW - functions for L_Type - 4/17/96
 */

L_Type *
L_new_type ()
{
  L_Type *ltype;

  ltype = (L_Type *) L_alloc (L_alloc_type);
  ltype->type = 0;
  ltype->struct_name = NULL;
  ltype->dcltr = NULL;

  return ltype;
}

void
L_delete_type (L_Type * ltype)
{
  if (ltype == NULL)
    return;

  if (ltype->dcltr != NULL)
    L_delete_dcltr (ltype->dcltr);

  L_free (L_alloc_type, ltype);
}

/*
 *      L_Dcltr :      LCW - functions for L_Dcltr - 4/17/96
 */

L_Dcltr *
L_new_dcltr ()
{
  L_Dcltr *dcltr;

  dcltr = (L_Dcltr *) L_alloc (L_alloc_dcltr);
  dcltr->method = 0;
  dcltr->index = NULL;
  dcltr->next = NULL;

  return dcltr;
}

void
L_delete_dcltr (L_Dcltr * dcltr)
{
  if (dcltr == NULL)
    return;

  if (dcltr->index != NULL)
    L_delete_expr_element (dcltr->index);

  if (dcltr->next != NULL)
    L_delete_dcltr (dcltr->next);

  L_free (L_alloc_dcltr, dcltr);
}


/* concatenate dcltr2 onto end of dcltr1 */
L_Dcltr *
L_concat_dcltr (L_Dcltr * d1, L_Dcltr * d2)
{
  L_Dcltr *ptr;

  if (d1 == NULL)
    return d2;
  if (d2 == NULL)
    return d1;

  ptr = d1;
  while (ptr->next != NULL)
    {
      ptr = ptr->next;
    }
  ptr->next = d2;

  return d1;
}

/*
 *      L_Struct_Dcl, L_Union_Dcl, L_Field :      LCW - 4/17/96
 */

L_Struct_Dcl *
L_new_struct_dcl ()
{
  L_Struct_Dcl *st_dcl;

  st_dcl = (L_Struct_Dcl *) L_alloc (L_alloc_struct_dcl);
  st_dcl->name = NULL;
  st_dcl->fields = NULL;
  st_dcl->ext = NULL;

  return st_dcl;
}

void
L_delete_struct_dcl (L_Struct_Dcl * st_dcl)
{
  if (st_dcl == NULL)
    return;

  if (st_dcl->fields != NULL)
    L_delete_field (st_dcl->fields);

  L_free (L_alloc_struct_dcl, st_dcl);
}

L_Union_Dcl *
L_new_union_dcl ()
{
  L_Union_Dcl *un_dcl;

  un_dcl = (L_Union_Dcl *) L_alloc (L_alloc_union_dcl);
  un_dcl->name = NULL;
  un_dcl->fields = NULL;
  un_dcl->ext = NULL;

  return un_dcl;
}

void
L_delete_union_dcl (L_Union_Dcl * un_dcl)
{
  if (un_dcl == NULL)
    return;

  if (un_dcl->fields != NULL)
    L_delete_field (un_dcl->fields);

  L_free (L_alloc_union_dcl, un_dcl);
}


L_Field *
L_new_field ()
{
  L_Field *field;

  field = (L_Field *) L_alloc (L_alloc_field);
  field->name = NULL;
  field->type = NULL;
  field->bit_field = NULL;
  field->next = NULL;
  field->ext = NULL;

  return field;
}

void
L_delete_field (L_Field * field)
{
  if (field == NULL)
    return;

  if (field->type != NULL)
    L_delete_type (field->type);

  if (field->bit_field != NULL)
    L_delete_expr_element (field->bit_field);

  if (field->next != NULL)
    L_delete_field (field->next);

  L_free (L_alloc_field, field);
}

/*
 *      L_Enum_Dcl, L_Enum_Field :      LCW  - 4/17/96
 */

L_Enum_Dcl *
L_new_enum_dcl ()
{
  L_Enum_Dcl *en_dcl;

  en_dcl = (L_Enum_Dcl *) L_alloc (L_alloc_enum_dcl);
  en_dcl->name = NULL;
  en_dcl->fields = NULL;

  return en_dcl;
}

void
L_delete_enum_dcl (L_Enum_Dcl * en_dcl)
{
  if (en_dcl == NULL)
    return;

  if (en_dcl->fields != NULL)
    L_delete_enum_field (en_dcl->fields);

  L_free (L_alloc_enum_dcl, en_dcl);
}

L_Enum_Field *
L_new_enum_field ()
{
  L_Enum_Field *en_field;

  en_field = (L_Enum_Field *) L_alloc (L_alloc_enum_field);
  en_field->name = NULL;
  en_field->value = NULL;
  en_field->next = NULL;

  return en_field;
}

void
L_delete_enum_field (L_Enum_Field * en_field)
{
  if (en_field == NULL)
    return;

  if (en_field->value != NULL)
    L_delete_expr_element (en_field->value);

  if (en_field->next != NULL)
    L_delete_enum_field (en_field->next);

  L_free (L_alloc_enum_field, en_field);
}


/*
 *      L_Data
 */

L_Data *
L_new_data (int type)
{
  L_Data *data;

  data = (L_Data *) L_alloc (L_alloc_data);
  data->type = type;
  data->N = 0;
  data->id = 0;
  data->address = NULL;
  data->value = NULL;
  data->h_type = NULL;		/* LCW - new field for debugging info - 4/17/96 */
  data->ext = NULL;

  /* reset global vars associated with L_Data */
  L_reset_data_global_vars ();

  return data;
}

L_Data *
L_new_data_w (int type, L_Expr * address, L_Expr * value)
{
  L_Data *new_data;

  new_data = L_new_data (type);

  new_data->address = address;
  new_data->value = value;

  return (new_data);
}


void
L_delete_data (L_Data * data)
{
  if (data == NULL)
    return;

  if (data->address != NULL)
    {
      L_delete_expr_element (data->address);
    }
  if (data->value != NULL)
    {
      L_delete_expr_element (data->value);
    }
  /* LCW - delete type field - 4/17/96 */
  if (data->h_type != NULL)
    L_delete_type (data->h_type);

  /* reset global vars associated with L_Data */

  /* JWS 20000108 - Reset L_data only if we are freeing L_data */

  if (data == L_data)
    L_reset_data_global_vars ();

  /* free the data */
  L_free (L_alloc_data, data);
}


/*
 *      L_Datalist_Element
 */

L_Datalist_Element *
L_new_datalist_element (L_Data * data)
{
  L_Datalist_Element *new_element;

  new_element = (L_Datalist_Element *) L_alloc (L_alloc_datalist_element);
  new_element->data = data;
  new_element->next_element = NULL;

  return (new_element);
}


    /* if list != NULL, removes element from list; in either case delete 
       element */

void
L_delete_datalist_element (L_Datalist * list, L_Datalist_Element * element)
{
  int found_element;
  L_Datalist_Element *curr_element, *prev_element;

  if (element == NULL)
    L_punt ("L_delete_datalist_element: NULL list");

  if (list != NULL)
    {

      found_element = 0;
      curr_element = prev_element = list->first_element;

      while ((!found_element) && (curr_element != NULL))
	{

	  if (curr_element == element)
	    {
	      if (curr_element == prev_element)
		{		/* if at head of list */
		  list->first_element = curr_element->next_element;
		  if (curr_element == list->last_element)
		    list->last_element = NULL;
		}
	      else
		{
		  prev_element->next_element = curr_element->next_element;
		  if (curr_element == list->last_element)
		    list->last_element = prev_element;
		}

	      found_element = 1;
	    }
	  else
	    {
	      prev_element = curr_element;
	      curr_element = curr_element->next_element;
	    }
	}

      if (!found_element)
	L_punt ("L_remove_datalist_element: element not found");
    }

  if (element->data != NULL)
    L_delete_data (element->data);

  element->data = NULL;
  element->next_element = NULL;

  L_free (L_alloc_datalist_element, element);
}


    /* searches for L_Data in list; returns element, or NULL if not found */

L_Datalist_Element *
L_find_datalist_element (L_Datalist * list, L_Data * data)
{
  L_Datalist_Element *curr_element;

  if (list == NULL)
    L_punt ("L_remove_datalist_element: NULL list");

  curr_element = list->first_element;

  while ((curr_element != NULL) && (curr_element->data != data))
    {

      curr_element = curr_element->next_element;
    }

  return (curr_element);
}


void
L_concat_datalist_element (L_Datalist * list, L_Datalist_Element * element)
{
  if (list == NULL)
    L_punt ("L_concat_datalist_element: NULL list");

  if (list->first_element == NULL)
    list->first_element = element;
  else
    list->last_element->next_element = element;

  list->last_element = element;
}


void
L_delete_all_datalist_element (L_Datalist * list)
{
  L_Datalist_Element *curr_element, *next_element;

  curr_element = list->first_element;

  while (curr_element != NULL)
    {

      next_element = curr_element->next_element;

      if (curr_element->data != NULL)
	L_delete_data (curr_element->data);

      curr_element->data = NULL;
      curr_element->next_element = NULL;

      L_free (L_alloc_datalist_element, curr_element);

      curr_element = next_element;
    }

  list->first_element = NULL;
  list->last_element = NULL;

}

/*
 *      L_Datalist
 */

L_Datalist *
L_new_datalist ()
{
  L_Datalist *datalist;

  datalist = (L_Datalist *) L_alloc (L_alloc_datalist);

  datalist->first_element = NULL;
  datalist->last_element = NULL;

  return (datalist);
}

void
L_delete_datalist (L_Datalist * datalist)
{
  if (datalist == NULL)
    return;

  if (datalist->first_element != NULL)
    L_delete_all_datalist_element (datalist);

  L_free (L_alloc_datalist, datalist);
}

/*
 *      L_Flow
 */

L_Flow *
L_new_flow (int cc, L_Cb * src_cb, L_Cb * dst_cb, double weight)
{
  L_Flow *flow;

  flow = (L_Flow *) L_alloc (L_alloc_flow);
  flow->cc = cc;
  flow->src_cb = src_cb;
  flow->dst_cb = dst_cb;
  flow->weight = weight;
  flow->weight2 = 0.0;
  flow->prev_flow = NULL;
  flow->next_flow = NULL;
  flow->flags = 0;

  return flow;
}

/*
 *      Remove the flow from the linked list w/o freeing it
 */
L_Flow *
L_remove_flow (L_Flow * list, L_Flow * flow)
{
  L_Flow *ptr, *first, *prev, *next;

  if (list == NULL)
    L_punt ("L_remove_flow: list and element cannot be NULL");

  if (flow == NULL)
    return list;

  /* Verify flow is in list just for sanity */
  for (ptr = list; ptr != 0; ptr = ptr->next_flow)
    {
      if (ptr == flow)
	{
	  break;
	}
    }
  if (ptr != flow)
    L_punt ("L_remove_flow: flow not in list");

  /* Unlink flow */
  first = list;
  if (flow == first)
    {
      first = list->next_flow;
    }
  prev = flow->prev_flow;
  next = flow->next_flow;
  if (prev != NULL)
    {
      prev->next_flow = next;
    }
  if (next != NULL)
    {
      next->prev_flow = prev;
    }
  flow->prev_flow = NULL;
  flow->next_flow = NULL;

  return first;
}

L_Flow *
L_delete_flow (L_Flow * list, L_Flow * flow)
{
  L_Flow *ptr, *first, *prev, *next;

  if (!flow)
    return list;

  if (!list)
    {
      flow->prev_flow = NULL;
      flow->next_flow = NULL;
      L_free (L_alloc_flow, flow);
      return NULL;
    }

  /* Verify flow is in list just for sanity */
  for (ptr = list; ptr != 0; ptr = ptr->next_flow)
    {
      if (ptr == flow)
	break;
    }
  if (ptr != flow)
    L_punt ("L_delete_flow: flow not in list");

  /* Unlink flow and free it */
  first = list;
  if (flow == first)
    first = list->next_flow;

  prev = flow->prev_flow;
  next = flow->next_flow;

  if (prev)
    prev->next_flow = next;
  if (next)
    next->prev_flow = prev;

  flow->prev_flow = NULL;
  flow->next_flow = NULL;
  L_free (L_alloc_flow, flow);

  return first;
}


void
L_delete_all_flow (L_Flow * list)
{
  L_Flow *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_flow;
      ptr->prev_flow = NULL;
      ptr->next_flow = NULL;
      L_free (L_alloc_flow, ptr);
    }
}

/*
 *      concatenate flow2 onto end of flow1
 */
L_Flow *
L_concat_flow (L_Flow * flow1, L_Flow * flow2)
{
  L_Flow *ptr;

  if (flow1 == NULL)
    return flow2;
  if (flow2 == NULL)
    return flow1;

  ptr = flow1;
  while (ptr->next_flow != NULL)
    {
      ptr = ptr->next_flow;
    }
  flow2->prev_flow = ptr;
  ptr->next_flow = flow2;

  return flow1;
}

L_Flow *
L_insert_flow_before (L_Flow * list, L_Flow * before_flow, L_Flow * flow)
{
  if (list && !before_flow)
    L_punt ("DIA - This is not what you expect.  "
	    "I apologize, someone has been messing with my compiler!!!");

  flow->next_flow = before_flow;
  flow->prev_flow = ((before_flow != NULL) ? before_flow->prev_flow : NULL);

  if (flow->prev_flow != NULL)
    flow->prev_flow->next_flow = flow;
  else
    list = flow;

  if (flow->next_flow != NULL)
    flow->next_flow->prev_flow = flow;

  return (list);
}

L_Flow *
L_insert_flow_after (L_Flow * list, L_Flow * after_flow, L_Flow * flow)
{
  flow->prev_flow = after_flow;
  flow->next_flow = ((after_flow != NULL) ? after_flow->next_flow : list);

  if (flow->prev_flow != NULL)
    flow->prev_flow->next_flow = flow;
  else
    list = flow;

  if (flow->next_flow != NULL)
    flow->next_flow->prev_flow = flow;

  return (list);
}


L_Flow *
L_insert_flows_after (L_Flow * list, L_Flow * after_flow, L_Flow * flow)
{
  L_Flow *last_flow;

  last_flow = flow;
  if (flow->next_flow)
    {
      while (last_flow->next_flow)
	last_flow = last_flow->next_flow;
    }

  flow->prev_flow = after_flow;
  last_flow->next_flow =
    ((after_flow != NULL) ? after_flow->next_flow : list);

  if (flow->prev_flow != NULL)
    flow->prev_flow->next_flow = flow;
  else
    list = flow;

  if (last_flow->next_flow != NULL)
    last_flow->next_flow->prev_flow = last_flow;

  return (list);
}


/*
 *      L_Attr
 */

static void
L_increase_attr_fields (L_Attr * attr, int field)
{
  int i;
  L_Operand **new_field;

  if (field < attr->max_field)
    return;

  new_field = (L_Operand **) Lcode_calloc (field + 1, sizeof (L_Operand *));

  if (attr->max_field > 0)
    {
      for (i = 0; i < attr->max_field; i++)
	{
	  new_field[i] = attr->field[i];
	}
      Lcode_free (attr->field);
    }

  attr->max_field = field + 1;
  attr->field = new_field;
}

void
L_insert_attr_field (L_Attr * attr, L_Operand *new_op, int field)
{
  int i, j;
  L_Operand **new_field;

  if(field < 0 || field > attr->max_field) {
    L_punt("L_insert_attr_field : %d not in range (%d, %d)\n", field, 0, attr->max_field);
  }

  new_field = (L_Operand **) Lcode_calloc (attr->max_field + 1, sizeof (L_Operand *));

  j = 0;
  for (i = 0; i < attr->max_field; i++)
  {
    if(i == field) {
      new_field[j] = attr->field[i];
      j++;
      new_field[j] = new_op;
    }
    else
      new_field[j] = attr->field[i];
    j++;
  }
  Lcode_free (attr->field);

  attr->max_field = attr->max_field + 1;
  attr->field = new_field;
}

/* Scott, still working on these */

void
L_set_attr_field (L_Attr * attr, int field, L_Operand * operand)
{
  if (attr == NULL)
    L_punt ("L_set_attr_field: attr is NULL");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = operand;
}

void
L_set_cb_attr_field (L_Attr * attr, int field, L_Cb * cb)
{
  if (attr == NULL)
    L_punt ("L_set_cb_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_cb_operand (cb);
}

void
L_set_int_attr_field (L_Attr * attr, int field, int val)
{
  if (attr == NULL)
    L_punt ("L_set_int_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_gen_int_operand (val);
}

void
L_set_float_attr_field (L_Attr * attr, int field, float val)
{
  if (attr == NULL)
    L_punt ("L_set_float_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_float_operand (val);
}

void
L_set_double_attr_field (L_Attr * attr, int field, double val)
{
  if (attr == NULL)
    L_punt ("L_set_double_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_double_operand (val);
}

void
L_set_string_attr_field (L_Attr * attr, int field, char *str)
{
  if (attr == NULL)
    L_punt ("L_set_string_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_gen_string_operand (str);
}

void
L_set_macro_attr_field (L_Attr * attr, int field, int mac,
			int ctype, int ptype)
{
  if (attr == NULL)
    L_punt ("L_set_mac_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_macro_operand (mac, ctype, ptype);
}

void
L_set_register_attr_field (L_Attr * attr, int field, int index,
			   int ctype, int ptype)
{
  if (attr == NULL)
    L_punt ("L_set_register_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_register_operand (index, ctype, ptype);
}

void
L_set_label_attr_field (L_Attr * attr, int field, char *label)
{
  if (attr == NULL)
    L_punt ("L_set_label_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_gen_label_operand (label);
}

void
L_set_rregister_attr_field (L_Attr * attr, int field, int index,
			    int ctype, int ptype)
{
  if (attr == NULL)
    L_punt ("L_set_rregister_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_rregister_operand (index, ctype, ptype);
}


void
L_set_evr_attr_field (L_Attr * attr, int field, int index,
		      int omega, int ctype, int ptype)
{
  if (attr == NULL)
    L_punt ("L_set_evr_attr_field: null attr");

  L_increase_attr_fields (attr, field);
  L_delete_attr_field (attr, field);

  attr->field[field] = L_new_evr_operand (index, omega, ctype, ptype);
}

void
L_delete_attr_field (L_Attr * attr, int field)
{
  if (attr == NULL)
    return;

  if (field > attr->max_field)
    return;

  L_delete_operand (attr->field[field]);

  attr->field[field] = NULL;
}

L_Attr *
L_new_attr (char *name, int num_fields)
{
  L_Attr *attr;

  attr = (L_Attr *) L_alloc (L_alloc_attr);

  attr->name = L_add_string (L_string_table, name);

  attr->max_field = 0;
  attr->field = NULL;

  if (num_fields != 0)
    {
      L_increase_attr_fields (attr, num_fields);
      attr->max_field = num_fields;
    }
  attr->next_attr = NULL;

  return (attr);
}

void
L_free_attr (L_Attr * attr)
{
  int i;

  if (attr == NULL)
    return;

  /* free the fields */
  for (i = 0; i < attr->max_field; i++)
    {
      if (attr->field[i] != NULL)
	{
	  L_delete_operand (attr->field[i]);
	  attr->field[i] = NULL;
	}
    }

  /* free the field array */
  if (attr->field != NULL)
    Lcode_free (attr->field);

  /* free the actual attribute */
  attr->name = NULL;
  attr->max_field = 0;
  attr->field = NULL;
  attr->next_attr = NULL;
  L_free (L_alloc_attr, attr);
}

L_Attr *
L_delete_attr (L_Attr * list, L_Attr * attr)
{
  L_Attr *ptr, *first, *prev;

  if (attr == NULL)
    return list;

  /* no list, so just free it */
  if (list == NULL)
    {
      L_free_attr (attr);
      return (NULL);
    }

  /* list exists, unlink then free */

  /* Handle special case where attr is first element of list */
  if (attr == list)
    {
      first = attr->next_attr;
      L_free_attr (attr);
      return (first);
    }
  else
    {
      /* Find the appropriate entry in the list */
      first = list;
      prev = list;
      for (ptr = first->next_attr; ptr != NULL; ptr = ptr->next_attr)
	{
	  if (ptr == attr)
	    {
	      break;
	    }
	  prev = ptr;
	}
      if (ptr != attr)
	L_punt ("L_delete_attr: attr not found in list");

      prev->next_attr = attr->next_attr;
      L_free_attr (attr);

      return (first);
    }
}

void
L_delete_all_attr (L_Attr * list)
{
  L_Attr *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_attr;
      /* LCW - check if the attribute has been deleted. Deleting an
         attribute twice will cause serious problem later - 11/22/95 */
      /* if (ptr->max_field != 0) */
      L_free_attr (ptr);
    }
}

L_Attr *
L_concat_attr (L_Attr * attr1, L_Attr * attr2)
{
  L_Attr *ptr, *previous;

  if (attr1 == NULL)
    return (attr2);
  if (attr2 == NULL)
    return (attr1);

  previous = attr1;
  ptr = attr1->next_attr;
  while (ptr != NULL)
    {
      previous = ptr;
      ptr = ptr->next_attr;
    }
  previous->next_attr = attr2;

  return (attr1);
}

L_Attr *
L_find_attr (L_Attr * attr, char *name)
{
  char *attr_name;
  L_Attr *ptr;

  if (name == NULL)
    L_punt ("L_find_attr: name is NULL");

  for (ptr = attr; ptr != NULL; ptr = ptr->next_attr)
    {
      attr_name = ptr->name;
      if (*attr_name != *name)	/* quick check of first letter */
	continue;
      if (!strcmp (attr_name, name))
	return (ptr);
    }

  return (NULL);
}

/* This routine is used when the wildcard character '*' is
   allowed at the end of the attribute name. It returns
   the number of matching attributes in the attribute list */
int
L_count_attr_prefix (L_Attr * attr, char *attr_name)
{
  int n = 0;
  L_Attr *this_attr;

  for (this_attr = attr; this_attr != NULL; this_attr = this_attr->next_attr)
    {
      if (!strncmp (this_attr->name, attr_name, strlen (attr_name)))
	n++;
    }

  return (n);
}

/* This routine is used when the wildcard character '*' is
   allowed at the end of the attribute name. It returns the
   nth attribute in the attribute list that matches attr_name,
   NULL if a bad index n 
   *** NOTE: 0<=n<=N-1 *** 
*/
L_Attr *
L_find_attr_prefix (L_Attr * attr, char *attr_name, int n)
{
  int ctr = n;
  L_Attr *this_attr = attr;

  if (ctr < 0)
    return NULL;

  while (this_attr && ctr >= 0)
    {
      if (!strncmp (this_attr->name, attr_name, strlen (attr_name)))
	ctr--;
      if (ctr >= 0)
	this_attr = this_attr->next_attr;
    }
  if (ctr >= 0)
    return NULL;
  return this_attr;
}

/*********************************************************
      L_Sync_Info
**********************************************************/

L_Sync_Info *
L_new_sync_info ()
{
  L_Sync_Info *new_sync_info;

  new_sync_info = (L_Sync_Info *) L_alloc (L_alloc_sync_info);
  new_sync_info->num_sync_in = 0;
  new_sync_info->num_sync_out = 0;
  new_sync_info->sync_out = NULL;
  new_sync_info->sync_in = NULL;

  return (new_sync_info);
}

L_Sync_Info *
L_copy_sync_info (L_Sync_Info * sync_info)
{
  L_Sync_Info *new_sync_info;

  new_sync_info = L_new_sync_info ();

  if (sync_info->num_sync_out > 0)
    {
      new_sync_info->sync_out =
	L_new_sync_array (((sync_info->num_sync_out - 1) /
			   L_SYNC_ALLOC_SIZE + 1) * L_SYNC_ALLOC_SIZE);
      L_copy_sync_array (new_sync_info->sync_out, sync_info->sync_out,
			 sync_info->num_sync_out);
      new_sync_info->num_sync_out = sync_info->num_sync_out;
    }

  if (sync_info->num_sync_in > 0)
    {
      new_sync_info->sync_in =
	L_new_sync_array (((sync_info->num_sync_in - 1) /
			   L_SYNC_ALLOC_SIZE + 1) * L_SYNC_ALLOC_SIZE);
      L_copy_sync_array (new_sync_info->sync_in, sync_info->sync_in,
			 sync_info->num_sync_in);
      new_sync_info->num_sync_in = sync_info->num_sync_in;
    }

  return (new_sync_info);
}


/*********************************************************
      L_Sync
**********************************************************/

L_Sync *
L_new_sync (L_Oper * oper)
{
  L_Sync *new_sync;

  new_sync = (L_Sync *) L_alloc (L_alloc_sync);
  new_sync->dep_oper = oper;

  return (new_sync);
}

void
L_delete_head_sync (L_Oper * oper, L_Sync * sync)
{
  L_Sync *dead_sync;

  if (sync == NULL)
    L_punt ("L_delete_head_sync: NULL sync");

  if (oper != NULL)
    dead_sync = L_remove_head_sync_from_oper (oper, sync);
  else
    dead_sync = sync;

  if (dead_sync == NULL)
    L_punt ("L_delete_head_sync: sync not found");
  else
    L_free (L_alloc_sync, dead_sync);
}

void
L_delete_tail_sync (L_Oper * oper, L_Sync * sync)
{
  L_Sync *dead_sync;

  if (sync == NULL)
    L_punt ("L_delete_tail_sync: NULL sync");

  if (oper != NULL)
    dead_sync = L_remove_tail_sync_from_oper (oper, sync);
  else
    dead_sync = sync;

  if (dead_sync == NULL)
    L_punt ("L_delete_tail_sync: sync not found");
  else
    L_free (L_alloc_sync, dead_sync);
}

void
L_find_and_delete_head_sync (L_Oper * oper, L_Oper * dep_oper)
{
  L_Sync *sync;

  if (oper == NULL)
    return;

  sync = L_find_head_sync (oper, dep_oper);

  if (sync == NULL)
    L_punt ("L_find_and_delete_head_sync: sync not found");

  L_delete_head_sync (oper, sync);
}

void
L_find_and_delete_tail_sync (L_Oper * oper, L_Oper * dep_oper)
{
  L_Sync *sync;

  if (oper == NULL)
    return;

  sync = L_find_tail_sync (oper, dep_oper);

  if (sync == NULL)
    L_punt ("L_find_and_delete_tail_sync: sync not found");

  L_delete_tail_sync (oper, sync);
}

void
L_delete_all_sync (L_Oper * oper, int delete_other_end)
{
  int i;
  L_Sync_Info *sync_info;

  if (oper == NULL)
    L_punt ("L_delete_all_sync: null oper");

  sync_info = oper->sync_info;

  if (sync_info == NULL)
    return;

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      if (delete_other_end)
	L_find_and_delete_head_sync (sync_info->sync_in[i]->dep_oper, oper);
      L_delete_tail_sync (NULL, sync_info->sync_in[i]);
    }

  for (i = 0; i < sync_info->num_sync_out; i++)
    {
      if (delete_other_end)
	L_find_and_delete_tail_sync (sync_info->sync_out[i]->dep_oper, oper);
      L_delete_head_sync (NULL, sync_info->sync_out[i]);
    }

  if (sync_info->num_sync_in > 0)
    free (sync_info->sync_in);
  sync_info->sync_in = NULL;
  sync_info->num_sync_in = 0;

  if (sync_info->num_sync_out > 0)
    free (sync_info->sync_out);
  sync_info->sync_out = NULL;
  sync_info->num_sync_out = 0;
}


void
L_insert_head_sync_in_oper (L_Oper * oper, L_Sync * sync)
{
  L_Sync **new_array;
  L_Sync_Info *sync_info;
  int i;

  sync_info = oper->sync_info;

  if (sync_info == NULL)
    {
      sync_info = L_new_sync_info ();
      oper->sync_info = sync_info;
    }

  if ((sync_info->num_sync_out % L_SYNC_ALLOC_SIZE) == 0)
    {
      new_array =
	L_new_sync_array (sync_info->num_sync_out + L_SYNC_ALLOC_SIZE);

      for (i = 0; i < sync_info->num_sync_out; i++)
	{
	  new_array[i] = sync_info->sync_out[i];
	  sync_info->sync_out[i] = NULL;
	}

      if (sync_info->sync_out)
	free (sync_info->sync_out);
      sync_info->sync_out = new_array;

      sync_info->sync_out[(sync_info->num_sync_out)++] = sync;
    }
  else
    {
      sync_info->sync_out[(sync_info->num_sync_out)++] = sync;
    }
}


void
L_insert_tail_sync_in_oper (L_Oper * oper, L_Sync * sync)
{
  L_Sync **new_array;
  L_Sync_Info *sync_info;
  int i;

  sync_info = oper->sync_info;

  if (sync_info == NULL)
    {
      sync_info = L_new_sync_info ();
      oper->sync_info = sync_info;
    }

  if ((sync_info->num_sync_in % L_SYNC_ALLOC_SIZE) == 0)
    {
      new_array =
	L_new_sync_array (sync_info->num_sync_in + L_SYNC_ALLOC_SIZE);

      for (i = 0; i < sync_info->num_sync_in; i++)
	{
	  new_array[i] = sync_info->sync_in[i];
	  sync_info->sync_in[i] = NULL;
	}
      if (sync_info->sync_in != NULL)
	free (sync_info->sync_in);
      sync_info->sync_in = new_array;

      sync_info->sync_in[(sync_info->num_sync_in)++] = sync;
    }
  else
    {
      sync_info->sync_in[(sync_info->num_sync_in)++] = sync;
    }
}


L_Sync *
L_remove_head_sync_from_oper (L_Oper * oper, L_Sync * sync)
{
  L_Sync **new_array;
  L_Sync *return_sync = NULL;
  L_Sync_Info *sync_info;
  int found_sync = 0, num_sync_out;
  int i;

  sync_info = oper->sync_info;

  num_sync_out = sync_info->num_sync_out;

  if (sync_info == NULL)
    L_punt ("L_remove_head_sync_from_oper: empty sync info");

  /* first pack the orig array, then shrink if necessary */

  for (i = 0; i < num_sync_out; i++)
    {
      if (sync_info->sync_out[i] == sync)
	{
	  return_sync = sync_info->sync_out[i];
	  found_sync = 1;
	}
      else if (found_sync)
	{
	  sync_info->sync_out[i - 1] = sync_info->sync_out[i];
	}
    }

  if (!found_sync)
    L_punt ("L_remove_sync_from_oper: sync not found");

  if ((num_sync_out % L_SYNC_ALLOC_SIZE) == 1)
    {
      new_array = L_new_sync_array (num_sync_out - 1);

      for (i = 0; i < (num_sync_out - 1); i++)
	{
	  new_array[i] = sync_info->sync_out[i];
	  sync_info->sync_out[i] = NULL;
	}
      sync_info->sync_out[i] = NULL;

      free (sync_info->sync_out);
      sync_info->sync_out = new_array;

      (sync_info->num_sync_out)--;
    }
  else
    {
      sync_info->sync_out[num_sync_out - 1] = NULL;
      (sync_info->num_sync_out)--;
    }

  return (return_sync);
}

L_Sync *
L_remove_tail_sync_from_oper (L_Oper * oper, L_Sync * sync)
{
  L_Sync **new_array;
  L_Sync *return_sync = NULL;
  L_Sync_Info *sync_info;
  int found_sync = 0, num_sync_in;
  int i;

  sync_info = oper->sync_info;

  num_sync_in = sync_info->num_sync_in;

  if (sync_info == NULL)
    L_punt ("L_remove_tail_sync_from_oper: empty sync info");

  /* first pack the orig array, then shrink if necessary */

  for (i = 0; i < num_sync_in; i++)
    {
      if (sync_info->sync_in[i] == sync)
	{
	  return_sync = sync_info->sync_in[i];
	  found_sync = 1;
	}
      else if (found_sync)
	{
	  sync_info->sync_in[i - 1] = sync_info->sync_in[i];
	}
    }

  if (!found_sync)
    L_punt ("L_remove_sync_from_oper: sync not found");

  if ((num_sync_in % L_SYNC_ALLOC_SIZE) == 1)
    {

      if (num_sync_in == 1)
	{
	  sync_info->sync_in[0] = NULL;
	  free (sync_info->sync_in);
	  sync_info->sync_in = NULL;
	  sync_info->num_sync_in = 0;
	  if (sync_info->num_sync_out == 0)
	    {
	      L_free (L_alloc_sync_info, sync_info);
	      oper->sync_info = NULL;
	    }
	}
      else
	{
	  new_array = L_new_sync_array (num_sync_in - 1);

	  for (i = 0; i < (num_sync_in - 1); i++)
	    {
	      new_array[i] = sync_info->sync_in[i];
	      sync_info->sync_in[i] = NULL;
	    }
	  sync_info->sync_in[i] = NULL;

	  free (sync_info->sync_in);
	  sync_info->sync_in = new_array;

	  (sync_info->num_sync_in)--;
	}
    }
  else
    {
      sync_info->sync_in[num_sync_in - 1] = NULL;
      (sync_info->num_sync_in)--;
    }

  return (return_sync);
}


L_Sync *
L_find_head_sync (L_Oper * oper, L_Oper * dep_oper)
{
  int i;
  L_Sync *sync = NULL;
  L_Sync_Info *sync_info;

  sync_info = oper->sync_info;

  if ((oper == NULL) || (dep_oper == NULL))
    L_punt ("L_find_sync: null oper");

  if (sync_info == NULL)
    L_punt ("L_find_head_sync: empty sync info");

  for (i = 0; i < sync_info->num_sync_out; i++)
    {
      if (sync_info->sync_out[i]->dep_oper == dep_oper)
	{
	  sync = sync_info->sync_out[i];
	  break;
	}
    }

  return (sync);
}


L_Sync *
L_find_tail_sync (L_Oper * oper, L_Oper * dep_oper)
{
  int i;
  L_Sync *sync = NULL;
  L_Sync_Info *sync_info;

  sync_info = oper->sync_info;

  if ((oper == NULL) || (dep_oper == NULL))
    L_punt ("L_find_sync: null oper");

  if (sync_info == NULL)
    L_punt ("L_find_tail_sync: empty sync info");

  for (i = 0; i < sync_info->num_sync_in; i++)
    {
      if (sync_info->sync_in[i]->dep_oper == dep_oper)
	{
	  sync = sync_info->sync_in[i];
	  break;
	}
    }

  return (sync);
}


L_Sync *
L_copy_sync (L_Sync * sync)
{
  L_Sync *new_sync;

  if (sync == NULL)
    L_punt ("L_copy_sync: null sync");

  new_sync = L_new_sync (sync->dep_oper);
  new_sync->dist = sync->dist;
  new_sync->info = sync->info;
  new_sync->prof_info = sync->prof_info;

  return (new_sync);
}

L_Sync **
L_new_sync_array (int array_size)
{
  L_Sync **new_array = NULL;

  if (array_size != 0)
    {
      new_array = (L_Sync **) malloc (sizeof (L_Sync *) * array_size);

      if (new_array == NULL)
	L_punt ("L_new_sync_array: malloc out of space");
    }

  return (new_array);
}


void
L_copy_sync_array (L_Sync ** new_array, L_Sync ** old_array, int array_size)
{
  int i;

  if ((old_array == NULL) || (new_array == NULL))
    L_punt ("L_copy_sync_array: null array");

  for (i = 0; i < array_size; i++)
    {
      new_array[i] = L_copy_sync (old_array[i]);
    }
}

/*********************************************************
      L_Operand
**********************************************************/

L_Operand *
L_new_cb_operand (L_Cb * cb)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_cb (operand);
  operand->ptype = L_PTYPE_NULL;
  operand->value.cb = cb;
  operand->ssa = NULL;
  return operand;
}

/* JLB - this function creates a general integer operand 
   ie. type = L_OPERAND_IMMEDIATE
       ctype = L_CTYPE_INT */
L_Operand *
L_new_gen_int_operand (ITintmax value)
{
  L_Operand *operand;

  /* operand = L_new_int_operand(value, L_CTYPE_INT);
   */

  operand = L_new_int_operand (value, M_native_int_register_ctype ());

  return operand;

}

/* JLB - this function should not be called unless a specific size
   integer is desired.  if a general, or size int immediate
   is wanted, please call the above function.  this function will 
   be used more heavily as type information becomes available */
L_Operand *
L_new_int_operand (ITintmax value, char ctype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_int (operand, ctype);

  operand->ptype = L_PTYPE_NULL;
  operand->value.i = value;
  operand->ssa = NULL;
  return operand;
}

#ifdef IT64BIT
L_Operand *
L_new_gen_llong_operand (ITint64 value)
{
  L_Operand *operand;

  operand = L_new_llong_operand (value, L_CTYPE_LLONG);
  return operand;

}

L_Operand *
L_new_llong_operand (ITint64 value, char ctype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_int (operand, ctype);

  operand->ptype = L_PTYPE_NULL;
  operand->value.i = value;
  operand->ssa = NULL;
  return operand;
}
#endif

L_Operand *
L_new_float_operand (float value)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_float (operand);
  operand->ptype = L_PTYPE_NULL;
  operand->value.f = value;
  operand->ssa = NULL;
  return operand;
}


/*   This operand always initializes 64 bits! */
L_Operand *
L_new_double_operand (double value)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);

  L_assign_type_double (operand);
  operand->ptype = L_PTYPE_NULL;
  operand->value.f2 = value;
  operand->ssa = NULL;
  return operand;
}

L_Operand *
L_new_gen_string_operand (char *str)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_string (operand);
  operand->ptype = L_PTYPE_NULL;
  operand->value.s = L_add_string (L_string_table, str);
  operand->ssa = NULL;
  return operand;
}

L_Operand *
L_new_string_operand (char *str, char ctype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  operand->type = L_OPERAND_STRING;
  operand->ctype = ctype;
  operand->ptype = L_PTYPE_NULL;
  operand->value.s = L_add_string (L_string_table, str);
  operand->ssa = NULL;
  return operand;
}

/*
 *      No need to specify ptype unless it is a predicate register
 */
L_Operand *
L_new_macro_operand (int mac, int ctype, int ptype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_general_macro (operand);
  L_assign_ctype (operand, ctype);

  if (L_is_ctype_predicate_direct (ctype))
    operand->ptype = ptype;
  else
    operand->ptype = L_PTYPE_NULL;
  operand->value.mac = mac;

  return operand;
}

/*
 *      No need to specify ptype unless it is a predicate register
 */
L_Operand *
L_new_register_operand (int index, int ctype, int ptype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_general_register (operand);
  L_assign_ctype (operand, ctype);

  if (L_is_ctype_predicate_direct (ctype))
    operand->ptype = ptype;
  else
    operand->ptype = L_PTYPE_NULL;
  operand->value.r = index;
  if (L_fn->max_reg_id < index)
    L_fn->max_reg_id = index;

  if (!strcmp(L_arch, "TAHOE"))
    {
      if (ctype == L_CTYPE_INT ||
	  ctype == L_CTYPE_CHAR ||
	  ctype == L_CTYPE_UCHAR ||
	  ctype == L_CTYPE_SHORT ||
	  ctype == L_CTYPE_USHORT ||
	  ctype == L_CTYPE_INT ||
	  ctype == L_CTYPE_UINT ||
	  ctype == L_CTYPE_LONG || ctype == L_CTYPE_ULONG)
	L_punt ("L_new_register_operand: attempted to create "
		"a non ll integer register ctype %d\n", ctype);
    }
  operand->ssa = NULL;
  return operand;
}

L_Operand *
L_new_gen_label_operand (char *label)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_label (operand);
  operand->ptype = L_PTYPE_NULL;
  operand->value.l = L_add_string (L_string_table, label);
  operand->ssa = NULL;
  return operand;
}

L_Operand *
L_new_label_operand (char *label, char ctype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  operand->type = L_OPERAND_LABEL;
  operand->ctype = ctype;
  operand->ptype = L_PTYPE_NULL;
  operand->value.l = L_add_string (L_string_table, label);
  operand->ssa = NULL;
  return operand;
}

/*
 *      No need to specify ptype unless it is a predicate register
 */
L_Operand *
L_new_rregister_operand (int index, int ctype, int ptype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);
  operand->value.init.u = 0;
  operand->value.init.l = 0;

  L_assign_type_general_rregister (operand);
  L_assign_ctype (operand, ctype);

  if (L_is_ctype_predicate_direct (ctype))
    operand->ptype = ptype;
  else
    operand->ptype = L_PTYPE_NULL;
  operand->value.rr = index;
  if (L_fn->max_reg_id < index)
    L_fn->max_reg_id = index;
  operand->ssa = NULL;
  return operand;
}

/*
 *      No need to specific ptype unless it is a predicate register
 *
 *   This operand always initializes 64 bits!
 */
L_Operand *
L_new_evr_operand (int index, int omega, int ctype, int ptype)
{
  L_Operand *operand;

  operand = (L_Operand *) L_alloc (L_alloc_operand);

  L_assign_type_general_evr (operand);
  L_assign_ctype (operand, ctype);

  if (L_is_ctype_predicate_direct (ctype))
    operand->ptype = ptype;
  else
    operand->ptype = L_PTYPE_NULL;
  operand->value.evr.num = index;
  operand->value.evr.omega = omega;
  if (L_fn->max_reg_id < index)
    L_fn->max_reg_id = index;
  operand->ssa = NULL;
  return operand;
}

void
L_delete_operand (L_Operand * operand)
{
  if (operand == NULL)
    return;

  L_free (L_alloc_operand, operand);
}

/*
 *      L_Oper
 */

L_Oper *
L_new_oper (int id)
{
  L_Oper *oper;
  L_Operand **array_ptr;

  oper = (L_Oper *) L_alloc (L_alloc_oper);
  memset (oper, 0, L_oper_size);

  L_fn->n_oper++;

  if (id >= L_fn->max_oper_id)
    L_fn->max_oper_id = id;

  oper->id = id;

  /* Allocate block of pointers for dest, src, and pred arrays */
  array_ptr = (L_Operand **) (oper + 1);

  /* Point at dest block */
  oper->dest = array_ptr;

  /* Move array_ptr to begining of src block */
  array_ptr += L_max_dest_operand;

  /* Point at src block */
  oper->src = array_ptr;

  /* Move array_ptr to beginning of pred block */
  array_ptr += L_max_src_operand;

  /* Point at pred block */
  oper->pred = array_ptr;

  /* put oper in oper hash tbl */
  L_oper_hash_tbl_insert (L_fn->oper_hash_tbl, oper);

  return oper;
}


L_Oper *
L_new_parent_oper (int id)
{
  L_Oper *oper;
  L_Operand **array_ptr;

  oper = (L_Oper *) L_alloc (L_alloc_oper);
  memset (oper, 0, L_oper_size);

  /* unlike L_new_oper, here we don't fn->n_oper++  */

  if (id >= L_fn->max_oper_id)
    L_fn->max_oper_id = id;

  oper->id = id;

  /* Allocate block of pointers for dest, src, and pred arrays */
  array_ptr = (L_Operand **) (oper + 1);

  /* Point at dest block */
  oper->dest = array_ptr;

  /* Move array_ptr to begining of src block */
  array_ptr += L_max_dest_operand;

  /* Point at src block */
  oper->src = array_ptr;

  /* Move array_ptr to beginning of pred block */
  array_ptr += L_max_src_operand;

  /* Point at pred block */
  oper->pred = array_ptr;

  /* unlike L_new_oper, here we don't put in hash tbl */

  return oper;
}


/*
 * This routine will remove an oper from a control block without 
 * physically deleting its memory.
 */
void
L_remove_oper (L_Cb * cb, L_Oper * oper)
{
  L_Oper *prev, *next;

  /* Handle case of null pointer */
  if (oper == NULL)
    return;

  if (cb == NULL)
    {				/* if cb==NULL skip disconnection steps */
      oper->prev_op = NULL;
      oper->next_op = NULL;
    }
  else
    {				/* otherwise, disconnect oper from doubly linked list */
      prev = oper->prev_op;
      next = oper->next_op;
      if (prev != NULL)
        prev->next_op = next;
      if (next != NULL)
        next->prev_op = prev;
      oper->prev_op = NULL;
      oper->next_op = NULL;

      /* fix up first_op and last_op ptrs if necessary */
      if (cb->first_op == oper)
        {
          if (cb->last_op == oper)
            {
              cb->first_op = NULL;
              cb->last_op = NULL;
            }
          else
            {
              cb->first_op = next;
            }
        }
      else if (cb->last_op == oper)
        {
          cb->last_op = prev;
        }
    }
  if (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PARENT))
    L_oper_hash_tbl_update_cb (L_fn->oper_hash_tbl, oper->id, NULL);
}

/*
 *      Note parent_op not deleted with this routine.
 */
void
L_delete_oper (L_Cb * cb, L_Oper * oper)
{
  int i;
  L_Oper *prev, *next;

  /* Handle case of null pointer */
  if (oper == NULL)
    return;

  /* if cb==NULL skip disconnection steps */
  if (cb == NULL)
    {
      oper->prev_op = NULL;
      oper->next_op = NULL;
    }

  /* otherwise, disconnect oper from doubly linked list */
  else
    {
      prev = oper->prev_op;
      next = oper->next_op;
      if (prev != NULL)
        prev->next_op = next;
      if (next != NULL)
        next->prev_op = prev;
      oper->prev_op = NULL;
      oper->next_op = NULL;
      /* fix up first_op and last_op ptrs if necessary */
      if (cb->first_op == oper)
        {
          if (cb->last_op == oper)
            {
              cb->first_op = NULL;
              cb->last_op = NULL;
            }
          else
            {
              cb->first_op = next;
            }
        }
      else if (cb->last_op == oper)
        {
          cb->last_op = prev;
        }
    }

  /* delete operands */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (oper->dest[i] != NULL)
        L_delete_operand (oper->dest[i]);
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (oper->src[i] != NULL)
        L_delete_operand (oper->src[i]);
    }
  for (i = 0; i < L_max_pred_operand; i++)
    {
      if (oper->pred[i] != NULL)
        L_delete_operand (oper->pred[i]);
    }

  /* delete sync structure; 1 indicates that we should also delete
     the corresponding sync structures at the other end of the sync arcs */
  if (oper->sync_info != NULL)
    {
      L_delete_all_sync (oper, 1);
      L_free (L_alloc_sync_info, oper->sync_info);
      oper->sync_info = NULL;
    }

  if (oper->acc_info)
    {
      L_delete_mem_acc_spec_list (oper->acc_info);
      oper->acc_info = NULL;
    }

  /* delete attributes */
  L_delete_all_attr (oper->attr);

  /* remove from the oper hash tbl */
  if (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_PARENT))
    L_oper_hash_tbl_delete (L_fn->oper_hash_tbl, oper);

  /* free oper */
  L_free (L_alloc_oper, oper);

  L_fn->n_oper--;
}

void
L_delete_complete_oper (L_Cb * cb, L_Oper * oper)
{
  L_Flow *dead_flow, *match_flow;
  L_Cb *dest_cb;

  /* Handle case of null pointer */
  if (oper == NULL)
    return;

  /* Check if oper will have a flow */
  if (L_register_branch_opcode (oper))
    {
      /* Can have multiple flows */
      if (oper != cb->last_op)
	L_punt ("L_delete_complete_oper: jump_rg is not last_op");

      dead_flow = L_find_flow_for_branch (cb, oper);

      while (dead_flow)
	{
	  L_Flow * next_flow = dead_flow->next_flow;
	  dest_cb = dead_flow->dst_cb;
	  match_flow = L_find_matching_flow (dest_cb->src_flow, dead_flow);
	  dest_cb->src_flow = L_delete_flow (dest_cb->src_flow, match_flow);
	  cb->dest_flow = L_delete_flow (cb->dest_flow, dead_flow);
	  dead_flow = next_flow;
	}
    }
  else if (L_is_control_oper (oper))
    {
      /* Remove flows attached to deleted branch */
      dead_flow = L_find_flow_for_branch (cb, oper);
      dest_cb = dead_flow->dst_cb;
      match_flow = L_find_matching_flow (dest_cb->src_flow, dead_flow);
      dest_cb->src_flow = L_delete_flow (dest_cb->src_flow, match_flow);
      cb->dest_flow = L_delete_flow (cb->dest_flow, dead_flow);
    }

  L_delete_oper (cb, oper);
  return;
}

void
L_delete_all_oper (L_Oper * list, int delete_cross_sync)
{
  int i;
  L_Oper *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_op;
      ptr->prev_op = NULL;
      ptr->next_op = NULL;

      /* delete operands */
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (ptr->dest[i] != NULL)
            L_delete_operand (ptr->dest[i]);
        }
      for (i = 0; i < L_max_src_operand; i++)
        {
          if (ptr->src[i] != NULL)
            L_delete_operand (ptr->src[i]);
        }
      for (i = 0; i < L_max_pred_operand; i++)
        {
          if (ptr->pred[i] != NULL)
            L_delete_operand (ptr->pred[i]);
        }

      /* delete sync structure; 0 indicates to only delete this end
         of the sync arc; i.e don't go to the dep_oper and delete its
         sync; this is done for efficiency when the entire function is
         being deleted  */
      if (ptr->sync_info != NULL)
        {
          L_delete_all_sync (ptr, delete_cross_sync);
          L_free (L_alloc_sync_info, ptr->sync_info);
          ptr->sync_info = NULL;
        }

      if (ptr->acc_info)
	{
	  L_delete_mem_acc_spec_list (ptr->acc_info);
	  ptr->acc_info = NULL;
	}

      /* delete attributes */
      L_delete_all_attr (ptr->attr);

      /* delete hash tbl entry */
      if (!L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_PARENT))
	L_oper_hash_tbl_delete (L_fn->oper_hash_tbl, ptr);

      /* free oper */
      L_free (L_alloc_oper, ptr);

      L_fn->n_oper--;
    }
}

void
L_insert_oper_before (L_Cb * cb, L_Oper * before_op, L_Oper * op)
{
  op->next_op = before_op;
  op->prev_op = ((before_op != NULL) ? before_op->prev_op : cb->last_op);

  if (op->prev_op != NULL)
    op->prev_op->next_op = op;
  else
    cb->first_op = op;

  if (op->next_op != NULL)
    op->next_op->prev_op = op;
  else
    cb->last_op = op;

  /* update oper hash table */
  L_oper_hash_tbl_update_cb (L_fn->oper_hash_tbl, op->id, cb);

  if (op->pred[0])
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      L_fn->flags = L_SET_BIT_FLAG (L_fn->flags, L_FUNC_HYPERBLOCK);
    }
}

void
L_insert_oper_after (L_Cb * cb, L_Oper * after_op, L_Oper * op)
{
  op->prev_op = after_op;
  op->next_op = ((after_op != NULL) ? after_op->next_op : cb->first_op);

  if (op->prev_op != NULL)
    op->prev_op->next_op = op;
  else
    cb->first_op = op;

  if (op->next_op != NULL)
    op->next_op->prev_op = op;
  else
    cb->last_op = op;

  /* update oper hash table */
  L_oper_hash_tbl_update_cb (L_fn->oper_hash_tbl, op->id, cb);

  if (op->pred[0])
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      L_fn->flags = L_SET_BIT_FLAG (L_fn->flags, L_FUNC_HYPERBLOCK);
    }
}

/*
 * Insert possibly multiple (linked) opers before/after named op
 */

void
L_insert_opers_before (L_Cb * cb, L_Oper * before_op, L_Oper * op)
{
  L_Oper *iter_op, *last_new_op = NULL;

  iter_op = op;
  while (iter_op)
    {
      L_oper_hash_tbl_update_cb (L_fn->oper_hash_tbl, iter_op->id, cb);
      last_new_op = iter_op;
      iter_op = iter_op->next_op;
    }

  last_new_op->next_op = before_op;
  op->prev_op = ((before_op != NULL) ? before_op->prev_op : cb->last_op);

  if (op->prev_op != NULL)
    op->prev_op->next_op = op;
  else
    cb->first_op = op;

  if (op->next_op != NULL)
    last_new_op->next_op->prev_op = last_new_op;
  else
    cb->last_op = last_new_op;
}

void
L_insert_opers_after (L_Cb * cb, L_Oper * after_op, L_Oper * op)
{
  L_Oper *iter_op, *last_new_op = NULL;

  iter_op = op;
  while (iter_op)
    {
      L_oper_hash_tbl_update_cb (L_fn->oper_hash_tbl, iter_op->id, cb);
      last_new_op = iter_op;
      iter_op = iter_op->next_op;
    }

  op->prev_op = after_op;
  last_new_op->next_op =
    ((after_op != NULL) ? after_op->next_op : cb->first_op);

  if (op->prev_op != NULL)
    op->prev_op->next_op = op;
  else
    cb->first_op = op;

  if (last_new_op->next_op != NULL)
    last_new_op->next_op->prev_op = last_new_op;
  else
    cb->last_op = last_new_op;
}

/*
 *      L_Cb
 */

L_Cb *
L_new_cb (int id)
{
  L_Cb *cb;

  cb = (L_Cb *) L_alloc (L_alloc_cb);

  L_fn->n_cb++;
  if (id >= L_fn->max_cb_id)
    L_fn->max_cb_id = id;

  cb->id = id;
  cb->flags = 0;
  cb->first_op = NULL;
  cb->last_op = NULL;
  cb->attr = NULL;
  cb->src_flow = NULL;
  cb->dest_flow = NULL;
  cb->prev_cb = NULL;
  cb->next_cb = NULL;
  cb->deepest_loop = NULL;
  cb->weight = 0.0;
  cb->weight2 = 0.0;
  cb->ext = NULL;
  /* REH 4/18/95 Region support */
  cb->region = NULL;
  cb->dom = NULL;
  cb->pdom = NULL;
  cb->hash_tgt_cnt = 0;

  /* put cb in cb hash tbl */
  L_cb_hash_tbl_insert (L_fn->cb_hash_tbl, cb);

  return cb;
}

/*
 * This routine will remove an cb from a function without physically
 * deleting its memory.
 */
void
L_remove_cb (L_Func * fn, L_Cb * cb)
{
  L_Cb *prev, *next;

  /* Handle case of null pointer */
  if (cb == NULL)
    return;

  /* if fn is NULL skip disconnection steps */
  if (fn == NULL)
    {
      cb->prev_cb = NULL;
      cb->next_cb = NULL;
    }

  else
    {
      /* disconnect cb from doubly linked list */
      prev = cb->prev_cb;
      next = cb->next_cb;
      if (prev != NULL)
        prev->next_cb = next;
      if (next != NULL)
        next->prev_cb = prev;
      cb->prev_cb = NULL;
      cb->next_cb = NULL;

      /* fix up first_cb and last_cb ptrs if necessary */
      if (fn->first_cb == cb)
	{
	  if (fn->last_cb == cb)
            {
              fn->first_cb = NULL;
              fn->last_cb = NULL;
            }
	  else
            {
              fn->first_cb = next;
            }
	}
      else if (fn->last_cb == cb)
        {
          fn->last_cb = prev;
        }
    }
}

void
L_delete_cb (L_Func * fn, L_Cb * cb)
{
  L_Cb *prev, *next;

  /* Handle case of null pointer */
  if (cb == NULL)
    return;

  /* if fn is NULL skip disconnection steps */
  if (fn == NULL)
    {
      cb->prev_cb = NULL;
      cb->next_cb = NULL;
    }

  else
    {
      /* disconnect cb from doubly linked list */
      prev = cb->prev_cb;
      next = cb->next_cb;
      if (prev != NULL)
        prev->next_cb = next;
      if (next != NULL)
        next->prev_cb = prev;
      cb->prev_cb = NULL;
      cb->next_cb = NULL;

      /* fix up first_cb and last_cb ptrs if necessary */
      if (fn->first_cb == cb)
	{
	  if (fn->last_cb == cb)
            {
              fn->first_cb = NULL;
              fn->last_cb = NULL;
            }
	  else
            {
              fn->first_cb = next;
            }
	}
      else if (fn->last_cb == cb)
        {
          fn->last_cb = prev;
        }
    }

  /* delete the opers; 1 indicates delete cross syncs */
  L_delete_all_oper (cb->first_op, 1);

  /* delete flow arcs, note corresponding flow arcs 
     in other cb's not deleted */
  L_delete_all_flow (cb->src_flow);
  L_delete_all_flow (cb->dest_flow);

  /* delete all attributes */
  L_delete_all_attr (cb->attr);

  /* delete hash table entry */
  L_cb_hash_tbl_delete (L_fn->cb_hash_tbl, cb);

  cb->deepest_loop = NULL;

  cb->region = NULL;

  if (cb->dom)
    Set_dispose (cb->dom);

  if (cb->pdom)
    Set_dispose (cb->pdom);

  /* free the cb */
  L_free (L_alloc_cb, cb);

  L_fn->n_cb--;
}

void
L_delete_all_cb (L_Cb * list, L_Cb_Hash_Entry ** cb_hash_tbl)
{
  L_Cb *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_cb;
      ptr->prev_cb = NULL;
      ptr->next_cb = NULL;

      /* delete the opers; 0 indicates don't delete cross syncs */
      L_delete_all_oper (ptr->first_op, 0);

      /* delete flow arcs */
      L_delete_all_flow (ptr->src_flow);
      L_delete_all_flow (ptr->dest_flow);

      /* delete all attributes */
      L_delete_all_attr (ptr->attr);

      ptr->deepest_loop = NULL;

      if (ptr->dom)
        Set_dispose (ptr->dom);

      if (ptr->pdom)
        Set_dispose (ptr->pdom);

      /* free the cb */
      L_free (L_alloc_cb, ptr);

      L_fn->n_cb--;
    }

  /* delete all hash tbl entries */
  L_cb_hash_tbl_delete_all (cb_hash_tbl);
}

void
L_insert_cb_before (L_Func * fn, L_Cb * before_cb, L_Cb * cb)
{
  cb->next_cb = before_cb;
  cb->prev_cb = ((before_cb != NULL) ? before_cb->prev_cb : NULL);

  if (cb->prev_cb != NULL)
    cb->prev_cb->next_cb = cb;
  else
    fn->first_cb = cb;

  if (cb->next_cb != NULL)
    cb->next_cb->prev_cb = cb;
  else
    fn->last_cb = cb;
}

void
L_insert_cb_after (L_Func * fn, L_Cb * after_cb, L_Cb * cb)
{
  cb->prev_cb = after_cb;
  cb->next_cb = ((after_cb != NULL) ? after_cb->next_cb : NULL);

  if (cb->prev_cb != NULL)
    cb->prev_cb->next_cb = cb;
  else
    fn->first_cb = cb;

  if (cb->next_cb != NULL)
    cb->next_cb->prev_cb = cb;
  else
    fn->last_cb = cb;
}

/*
 *      L_Expression
 *
 */


/* Note: currently called only in D_compute_expressions_and_PRE_kill_sets,
 * index assignment and max check can be moved outside if necessary
 * Expressions currently are computed from operations, so the input op
 * is necessary for formation of any new expression.
 */

L_Expression *
L_new_expression (int token, L_Oper * oper, int short_flag)
{
  L_Expression *expression;
  L_Operand **array_ptr;
  int i, index;

  expression = (L_Expression *) L_alloc (L_alloc_expression);
  memset (expression, 0, L_expression_size);
  index = ++L_fn->n_expression;

  expression->token = token;
  expression->index = index;
  expression->opc = oper->opc;
  if (oper->dest[0])
    expression->dest_ctype = oper->dest[0]->ctype;
  else if (L_is_label (oper->src[2]))
    expression->dest_ctype = M_native_int_register_ctype ();
  else				/* stores need to be tagged with store ctype */
    {
      expression->dest_ctype = oper->src[2]->ctype;
      if (!strcmp (L_arch, "TAHOE") && expression->dest_ctype == L_CTYPE_FLOAT)
        expression->dest_ctype = L_CTYPE_DOUBLE;
    }

  for (i = 0; i < L_MAX_CMPLTR; i++)
    expression->com[i] = oper->com[i];
  expression->reg_id = 0;
  expression->pred_id = 0;
  expression->weight = 0.0;
  array_ptr = (L_Operand **) (expression + 1);
  expression->src = array_ptr;
  expression->associates = NULL;
  expression->conflicts = NULL;
  expression->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);
  expression->general = NULL;

  /* SER 20041213 Update conflict info for all expressions. */
  if (L_func_acc_specs)
    L_update_expression_conflict_info (expression, oper, 0);

  /* We only want the first two operands in certain cases. */
  for (i = 0; i < (short_flag ? 2 : L_max_src_operand); i++)
    expression->src[i] = L_copy_operand (oper->src[i]);

  for (i = 0; i < 2; i++)
    expression->mem_ref[i] = NULL;

  return expression;
}


L_Expression *
L_new_assignment (int token, L_Oper * oper, int short_flag)
{
  L_Expression * assignment;
  L_Operand **array_ptr;
  int i, index;

  assignment = (L_Expression *) L_alloc (L_alloc_expression);
  memset (assignment, 0, L_expression_size);
  index = ++L_fn->n_expression;

  assignment->token = token;
  assignment->index = index;
  assignment->opc = oper->opc;
  if (oper->dest[0])
    assignment->dest_ctype = oper->dest[0]->ctype;
  else if (L_is_label (oper->src[2]))
    assignment->dest_ctype = M_native_int_register_ctype ();
  else if (L_store_opcode (oper))
    {
      assignment->dest_ctype = oper->src[2]->ctype;
      if (!strcmp (L_arch, "TAHOE") && assignment->dest_ctype == L_CTYPE_FLOAT)
	assignment->dest_ctype = L_CTYPE_DOUBLE;
    }

  for (i = 0; i < L_MAX_CMPLTR; i++)
    assignment->com[i] = oper->com[i];
  assignment->reg_id = 0;
  assignment->weight = 0.0;
  array_ptr = (L_Operand **) (assignment + 1);
  assignment->src = array_ptr;
  array_ptr += L_max_src_operand;
  assignment->dest = array_ptr;
  assignment->associates = NULL;
  assignment->conflicts = NULL;
  assignment->acc_info = L_copy_mem_acc_spec_list (oper->acc_info);

  /* SER 20041213 Update conflict info for all expressions. */
  if (L_func_acc_specs)
    L_update_expression_conflict_info (assignment, oper, 0);

  for (i = 0; i < (short_flag ? 2: L_max_src_operand); i++)
    assignment->src[i] = L_copy_operand (oper->src[i]);

  if (!short_flag)
    for (i = 0; i < L_max_dest_operand; i++)
      assignment->dest[i] = L_copy_operand (oper->dest[i]);
  else if (L_stack_reference (oper))
    {
      L_Attr * attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
      if (!attr)
	return assignment;
      assignment->mem_ref[0] = L_copy_operand (attr->field[0]);
      assignment->mem_ref[1] = L_copy_operand (attr->field[1]);
   }

  return assignment;
}

int
L_load_store_equivalent_opcodes (int load_opc, int store_opc)
{
  switch (load_opc)
    {
    case Lop_LD_UC:
    case Lop_LD_C:
      if (store_opc == Lop_ST_C)
        return 1;
      return 0;
    case Lop_LD_UC2:
    case Lop_LD_C2:
      if (store_opc == Lop_ST_C2)
        return 1;
      return 0;
    case Lop_LD_UI:
    case Lop_LD_I:
      if (store_opc == Lop_ST_I)
        return 1;
      return 0;
    case Lop_LD_Q:
      if (store_opc == Lop_ST_Q)
        return 1;
      return 0;
    case Lop_LD_F:
      if (store_opc == Lop_ST_F)
        return 1;
      return 0;
    case Lop_LD_F2:
      if (store_opc == Lop_ST_F2)
        return 1;
      return 0;
    default:
      L_punt ("Invalid opcode %d passed to L_load_store_equivalent_opcodes.\n",
	      load_opc);
      return 0;
    }
}


int
L_oper_matches_expression (L_Oper * oper, int oper_token,
			   L_Expression * expression, int short_flag,
			   int opc)
{
  int i;

  if (oper_token && (oper_token != expression->token))
    return 0;

  if (!opc)
    {
      if (expression->opc != oper->opc)
        return 0;
    }
  else
    {
#if 0
      if (L_is_variable (expression->src[0]) && 
	   L_is_variable (expression->src[1]))
	{
	  if (L_REG_MAC_INDEX (expression->src[0]) >
	      L_REG_MAC_INDEX (expression->src[1]))
	    {
	      tmp_operand = expression->src[0];
	      expression->src[0] = expression->src[1];
	      expression->src[1] = tmp_operand;
	    }
	}
      else if (L_is_variable (expression->src[1]) &&
	       L_is_constant (expression->src[0]))
	{
	  tmp_operand = expression->src[0];
	  expression->src[0] = expression->src[1];
	  expression->src[1] = tmp_operand;
	}
      else if (L_is_int_constant (expression->src[0]))
	{
	  if (!L_is_int_constant (expression->src[1]) ||
	      (L_is_int_constant (expression->src[1]) && 
	       (expression->src[0]->value.i > expression->src[1]->value.i)))
	    {
	      tmp_operand = expression->src[0];
	      expression->src[0] = expression->src[1];
	      expression->src[1] = tmp_operand;
	    }
	}
#else
      if (opc != expression->opc)
        return 0;
#endif
    }

  for (i = 0; i < L_MAX_CMPLTR; i++)
    if (expression->com[i] != oper->com[i])
      return 0;

  /* expression->src[2] must be clear for memory PCE */
  if (short_flag && expression->src[2])
    return 0;

  for (i = 0; i < ((short_flag | opc) ? 2 : L_max_src_operand); i++)
    if (!L_same_operand (expression->src[i], oper->src[i]))
      return 0;

  return 1;
}


int
L_oper_matches_assignment (L_Oper * oper, int oper_token,
			   L_Expression * expression, int short_flag)
{
  int i;

  /* If oper_token exists, use it to short-circuit non-matching cases. */
  if (oper_token && (oper_token != expression->token))
    return 0;

  if (expression->opc != oper->opc)
    return 0;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    if (expression->com[i] != oper->com[i])
      return 0;

#if 0
  /* commutativity check */
  if (L_commutative_opcode (oper))
    {
      if (L_is_variable (oper->src[0]) && L_is_variable (oper->src[1]))
	{
	  if (L_REG_MAC_INDEX (oper->src[0]) > L_REG_MAC_INDEX (oper->src[1]))
	    {
	      if (!L_same_operand (expression->src[0], oper->src[1]))
		return 0;
	      if (!L_same_operand (expression->src[1], oper->src[0]))
		return 0;
	      for (i = 2; i < L_max_src_operand; i++)
		if (!L_same_operand (expression->src[i], oper->src[i]))
		  return 0;
	      return 1;
	    }
	}
      else if (L_is_variable (oper->src[1]) && L_is_constant (oper->src[0]))
	{
	  if (!L_same_operand (expression->src[0], oper->src[1]))
	    return 0;
	  if (!L_same_operand (expression->src[1], oper->src[0]))
	    return 0;
	  for (i = 2; i < L_max_src_operand; i++)
	    if (!L_same_operand (expression->src[i], oper->src[i]))
	      return 0;
	  return 1;
	}
      else if (L_is_int_constant (oper->src[0]))
	{
	  if (!L_is_int_constant (oper->src[1]) ||
	      (L_is_int_constant (oper->src[1]) && 
	       (oper->src[0]->value.i > oper->src[1]->value.i)))
	    {
	      if (!L_same_operand (expression->src[0], oper->src[1]))
		return 0;
	      if (!L_same_operand (expression->src[1], oper->src[0]))
		return 0;
	      for (i = 2; i < L_max_src_operand; i++)
		if (!L_same_operand (expression->src[i], oper->src[i]))
		  return 0;
	      return 1;
	    }
	}
    }
#endif
  /* expression->src[2] must be clear for memory PCE */
  if (short_flag && expression->src[2])
    return 0;

  for (i = 0; i < (short_flag ? 2 : L_max_src_operand); i++)
    if (!L_same_operand (expression->src[i], oper->src[i]))
      return 0;

  if (!short_flag)
    for (i = 0; i < L_max_dest_operand; i++)
      {
	if (!L_same_operand (expression->dest[i], oper->dest[i]))
	  return 0;
	/* Need to check that predicate dest types are the same. */
	else if ((expression->dest[i]) &&
		 (expression->dest[i]->ctype == L_CTYPE_PREDICATE) &&
		 (expression->dest[i]->ptype != oper->dest[i]->ptype))
	  return 0;
      }

  return 1;
}

int
L_load_oper_matches_store_expression (L_Oper * oper, int oper_token,
				      L_Expression * expression)
{
  int i;

  if (oper_token && (oper_token != expression->token))
    return 0;

  if (!L_load_store_equivalent_opcodes (oper->opc, expression->opc))
    return 0;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    if (expression->com[i] != oper->com[i])
      return 0;

  /* Ignore store expressions with src[2]->from PRE, not wanted */
  if (expression->src[2])
    return 0;

  for (i = 0; i < 2; i++)
    if (!L_same_operand (expression->src[i], oper->src[i]))
      return 0;

  return 1;
}


/* Function for matching expressions. */
int
L_opers_same_expression (L_Oper *opA, L_Oper *opB)
{
  int i;

  if (opA == opB)
    return 1;

  if ((opA->opc) != (opB->opc))
    return 0;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    if (opA->com[i] != opB->com[i])
      return 0;

  for (i = 0; i < L_max_src_operand; i++)
    if (!L_same_operand (opA->src[i], opB->src[i]))
      return 0;

  return 1;
}


int
L_opers_same_assignment (L_Oper *opA, L_Oper * opB, int short_flag)
{
  int i;

  if ((opA->opc) != (opB->opc))
    return 0;

  for (i = 0; i < L_MAX_CMPLTR; i++)
    if (opA->com[i] != opB->com[i])
      return 0;

  for (i = 0; i < (short_flag ? 2 : L_max_src_operand); i++)
    if (!L_same_operand (opA->src[i], opB->src[i]))
      return 0;

  if (!short_flag)
    for (i = 0; i < L_max_dest_operand; i++)
      {
	if (!L_same_operand (opA->dest[i], opB->dest[i]))
	  return 0;
	else if ((opA->dest[i]) &&
		 (opA->dest[i]->ctype == L_CTYPE_PREDICATE) &&
		 (opA->dest[i]->ptype != opB->dest[i]->ptype))
	  return 0;
      }

  return 1;
}

/* Following function for store/load association. Checks only first two
 * source operands. */
int
L_opers_same_or_complementary_expression (L_Oper *opA, L_Oper *opB)
{
  int i, complement_flag = 0;

  if (opA->opc != opB->opc)
    {
      if (L_load_opcode (opA) && L_store_opcode (opB) &&
	  L_load_store_equivalent_opcodes (opA->opc, opB->opc))
	complement_flag = 1;
      else
	return 0;
    }

  for (i = 0; i < L_MAX_CMPLTR; i++)
    if (opA->com[i] != opB->com[i])
      return 0;

  for (i = 0; i < ((complement_flag) ? 2 : L_max_src_operand); i++)
    if (!L_same_operand (opA->src[i], opB->src[i]))
      return 0;

  return 1;
}


/* Warning: should only be used when cleaning up memory while exiting
 * optimization! During optimization, expressions are assumed to exist in
 * contiguous integral order.
 */
void
L_delete_expression (L_Expression * expression)
{
  int i;

  if (expression == NULL)
    return;

  if (expression->dest)
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (expression->dest[i] != NULL)
	    L_delete_operand (expression->dest[i]);
          else
	    break;
        }
    }

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (expression->src[i] != NULL)
	L_delete_operand (expression->src[i]);
      else
	break;
    }

  for (i = 0; i < 2; i++)
    L_delete_operand (expression->mem_ref[i]);

  /* don't remove from expression hash tbls: 
   * assumed outside of this function */
  Set_dispose (expression->associates);
  Set_dispose (expression->conflicts);

  if (expression->acc_info)
    {
      L_delete_mem_acc_spec_list (expression->acc_info);
      expression->acc_info = NULL;
    }

  L_free (L_alloc_expression, expression);
}


void
L_delete_all_expressions (L_Func * fn)
{
  L_Expression_Hash_Entry *entry;
  int i;

  for (i = 1; i <= fn->n_expression; i++)
    {
      entry =
	L_expression_hash_tbl_find_entry (fn->expression_index_hash_tbl, i);
      L_delete_expression (entry->expression);
    }

  L_expression_hash_tbl_delete_all (fn->expression_token_hash_tbl);
  L_expression_hash_tbl_delete_all (fn->expression_index_hash_tbl);
  fn->n_expression = 0;
}

/*
 *      L_Oper_Hash_Entry
 */

L_Oper_Hash_Entry *
L_new_oper_hash_entry (int id, L_Oper * oper)
{
  L_Oper_Hash_Entry *entry;

  entry = (L_Oper_Hash_Entry *) L_alloc (L_alloc_oper_hash_entry);
  entry->id = id;
  entry->oper = oper;
  entry->cb = NULL;
  entry->prev_oper_hash = NULL;
  entry->next_oper_hash = NULL;

  return (entry);
}

L_Oper_Hash_Entry *
L_delete_oper_hash_entry (L_Oper_Hash_Entry * list, L_Oper_Hash_Entry * entry)
{
  L_Oper_Hash_Entry *ptr, *first, *prev, *next;

  if (entry == NULL)
    return (list);

  if (list == NULL)
    {
      entry->prev_oper_hash = NULL;
      entry->next_oper_hash = NULL;
      L_free (L_alloc_oper_hash_entry, entry);
      return (NULL);
    }

  /* Verify entry is in list just for sanity */
  for (ptr = list; ptr != NULL; ptr = ptr->next_oper_hash)
    {
      if (ptr == entry)
        break;
    }

  if (ptr != entry)
    L_punt ("L_delete_oper_hash_entry: entry not in list");

  /* Unlink entry and free it */
  first = list;
  if (entry == first)
    first = entry->next_oper_hash;
  prev = entry->prev_oper_hash;
  next = entry->next_oper_hash;
  if (prev != NULL)
    prev->next_oper_hash = next;
  if (next != NULL)
    next->prev_oper_hash = prev;
  entry->prev_oper_hash = NULL;
  entry->next_oper_hash = NULL;
  L_free (L_alloc_oper_hash_entry, entry);

  return (first);
}

void
L_delete_all_oper_hash_entry (L_Oper_Hash_Entry * list)
{
  L_Oper_Hash_Entry *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_oper_hash;
      ptr->prev_oper_hash = NULL;
      ptr->next_oper_hash = NULL;
      L_free (L_alloc_oper_hash_entry, ptr);
    }
}

L_Oper_Hash_Entry *
L_find_oper_hash_entry (L_Oper_Hash_Entry * list, int id)
{
  L_Oper_Hash_Entry *ptr;

  for (ptr = list; ptr != NULL; ptr = ptr->next_oper_hash)
    {
      if (ptr->id == id)
        return (ptr);
    }

  return (NULL);
}

L_Oper_Hash_Entry *
L_find_specific_oper_hash_entry (L_Oper_Hash_Entry * list, L_Oper * oper)
{
  L_Oper_Hash_Entry *ptr;

  for (ptr = list; ptr != NULL; ptr = ptr->next_oper_hash)
    {
      if (ptr->oper == oper)
        return (ptr);
    }


  return (NULL);
}

#if DEBUG_CHECK_HASH_TBL
static int
L_oper_hash_tbl_verify_empty (L_Oper_Hash_Entry **hashtbl)
{
  int hash_id, cnt = 0;
  L_Oper_Hash_Entry *entry;

  for (hash_id = 0; hash_id < L_OPER_HASH_TBL_MASK; hash_id++)
    {
      for (entry = hashtbl[hash_id]; entry; entry = entry->next_oper_hash)
	{
	  if (!cnt)
	    L_warn ("FUNCTION %s: FOUND DANGLING OPER(S) IN HASH TBL:\n",
		    L_fn->name);
	  DB_print_oper (entry->oper);
	  cnt++;
	}
    }

  return cnt;
}
#endif

/*
 *      L_Cb_Hash_Entry
 */

L_Cb_Hash_Entry *
L_new_cb_hash_entry (int id, L_Cb * cb)
{
  L_Cb_Hash_Entry *entry;

  entry = (L_Cb_Hash_Entry *) L_alloc (L_alloc_cb_hash_entry);
  entry->id = id;
  entry->cb = cb;
  entry->prev_cb_hash = NULL;
  entry->next_cb_hash = NULL;

  return (entry);
}

L_Cb_Hash_Entry *
L_delete_cb_hash_entry (L_Cb_Hash_Entry * list, L_Cb_Hash_Entry * entry)
{
  L_Cb_Hash_Entry *ptr, *first, *prev, *next;

  if (entry == NULL)
    return (list);

  if (list == NULL)
    {
      entry->prev_cb_hash = NULL;
      entry->next_cb_hash = NULL;
      L_free (L_alloc_cb_hash_entry, entry);
      return (NULL);
    }

  /* Verify entry is in list just for sanity */
  for (ptr = list; ptr != NULL; ptr = ptr->next_cb_hash)
    {
      if (ptr == entry)
        break;
    }

  if (ptr != entry)
    L_punt ("L_delete_cb_hash_entry: entry not in list");

  /* Unlink entry and free it */
  first = list;
  if (entry == first)
    first = entry->next_cb_hash;
  prev = entry->prev_cb_hash;
  next = entry->next_cb_hash;
  if (prev != NULL)
    prev->next_cb_hash = next;
  if (next != NULL)
    next->prev_cb_hash = prev;
  entry->prev_cb_hash = NULL;
  entry->next_cb_hash = NULL;
  L_free (L_alloc_cb_hash_entry, entry);

  return (first);
}

void
L_delete_all_cb_hash_entry (L_Cb_Hash_Entry * list)
{
  L_Cb_Hash_Entry *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_cb_hash;
      ptr->prev_cb_hash = NULL;
      ptr->next_cb_hash = NULL;
      L_free (L_alloc_cb_hash_entry, ptr);
    }
}

L_Cb_Hash_Entry *
L_find_cb_hash_entry (L_Cb_Hash_Entry * list, int id)
{
  L_Cb_Hash_Entry *ptr;

  for (ptr = list; ptr != NULL; ptr = ptr->next_cb_hash)
    {
      if (ptr->id == id)
        return (ptr);
    }

  return (NULL);
}

/*
 *      L_Expression_Hash_Entry
 */

L_Expression_Hash_Entry *
L_new_expression_hash_entry (int hash, L_Expression * expression)
{
  L_Expression_Hash_Entry *entry;

  entry = (L_Expression_Hash_Entry *) L_alloc (L_alloc_expression_hash_entry);
  entry->id = hash;
  entry->expression = expression;
  entry->prev_expression_hash = NULL;
  entry->next_expression_hash = NULL;

  return (entry);
}


void
L_delete_all_expression_hash_entry (L_Expression_Hash_Entry * list)
{
  L_Expression_Hash_Entry *ptr, *next;

  for (ptr = list; ptr != NULL; ptr = next)
    {
      next = ptr->next_expression_hash;
      ptr->prev_expression_hash = NULL;
      ptr->next_expression_hash = NULL;
      L_free (L_alloc_expression_hash_entry, ptr);
    }
}


/*
 *      L_Func
 */

L_Func *
L_new_func (char *name, double weight)
{
  L_Func *fn;

  fn = (L_Func *) L_alloc (L_alloc_func);
  memset (fn, 0, sizeof(L_Func));

  fn->name = L_add_string (L_string_table, name);
  fn->flags = 0;
  fn->weight = weight;
  fn->first_cb = NULL;
  fn->last_cb = NULL;
  fn->n_cb = 0;
  fn->n_oper = 0;
  fn->n_parent_oper = 0;
  fn->max_cb_id = 0;
  fn->max_oper_id = 0;
  fn->max_reg_id = 0;
  fn->max_spec_id = 0;
  fn->s_local = 0;
  fn->s_param = 0;
  fn->s_swap = 0;
  fn->attr = NULL;
  fn->last_parent_op = NULL;
  fn->cb_hash_tbl = L_cb_hash_tbl_create ();
  fn->oper_hash_tbl = L_oper_hash_tbl_create ();

  fn->n_expression = 0;
  fn->expression_token_hash_tbl = 
    (L_Expression_Hash_Entry **) calloc (L_expression_hash_tbl_size,
    sizeof (L_Expression_Hash_Entry *));
  fn->expression_index_hash_tbl = 
    (L_Expression_Hash_Entry **) calloc (L_expression_hash_tbl_size,
    sizeof (L_Expression_Hash_Entry *));

  fn->first_loop = NULL;
  fn->max_loop_id = 0;
  fn->first_inner_loop = NULL;
  fn->max_inner_loop_id = 0;

  /* REH 4/19/95 - Region support */
  fn->first_region = NULL;
  fn->last_region = NULL;
  fn->region_hash_tbl = L_region_hash_tbl_create ();

  /* SAM 7-96 - new hash tbl support */
  fn->jump_tbls = NULL;
  fn->jump_tbl_flags = 0;

  /* reset global vars associated with L_Func */
  L_reset_func_global_vars ();

  return fn;
}

void
L_delete_func (L_Func * fn)
{
  /* delete all the cbs */
  L_delete_all_cb (fn->first_cb, fn->cb_hash_tbl);

  /* delete all attr */
  L_delete_all_attr (fn->attr);

  /* delete all parent ops; 0 indicates don't delete cross sync */
  L_delete_all_oper (fn->last_parent_op, 0);

  /* delete all expressions and hash entries */
  L_delete_all_expressions (fn);

  /* delete expression hash tables */
  free(fn->expression_token_hash_tbl);
  free(fn->expression_index_hash_tbl);

  /* delete cb hash table */
  L_free (L_alloc_cb_hash_tbl, fn->cb_hash_tbl);

#if DEBUG_CHECK_HASH_TBL
  L_oper_hash_tbl_verify_empty (fn->oper_hash_tbl);
#endif

  /* delete oper hash table */
  L_free (L_alloc_oper_hash_tbl, fn->oper_hash_tbl);

  /* delete all regions (REH 4/20/95) */
  L_delete_all_region (fn->first_region, fn->region_hash_tbl);
	 
  /* delete region hash table (REH 4/20/95) */
  L_free (L_alloc_region_hash_tbl, fn->region_hash_tbl);

  /* delete the loop structures */
  L_delete_all_loop (fn->first_loop);
  L_delete_all_inner_loop (fn->first_inner_loop);

  /* SAM 7-96: Delete the jump_tbls */
  if (fn->jump_tbls)
    L_delete_datalist (fn->jump_tbls);

  /* reset global vars associated with L_Func */
  L_reset_func_global_vars ();

  /* delete the func */
  L_free (L_alloc_func, fn);

}

/*
 *      L_Oper_List
 */

L_Oper_List *
L_new_oper_list (void)
{
  L_Oper_List *new_list;

  new_list = (L_Oper_List *) L_alloc (L_alloc_oper_list);

  new_list->oper = NULL;
  new_list->next_list = NULL;

  return (new_list);
}

L_Oper_List *
L_delete_oper_list (L_Oper_List * list, L_Oper_List * element)
{
  L_Oper_List *ptr, *first, *prev;

  if (element == NULL)
    return (list);

  element->oper = NULL;         /* dont free the oper */

  /* just free element if list is NULL, return a NULL ptr also */
  if (list == NULL)
    {
      element->next_list = NULL;
      L_free (L_alloc_oper_list, element);
    }

  /* Handle special case where attr is first element of list */
  if (list == element)
    {
      first = element->next_list;
      element->next_list = NULL;
      L_free (L_alloc_oper_list, element);
      return (first);
    }
  else
    {
      /* Find the appropriate entry in the list */
      first = list;
      prev = list;
      for (ptr = first->next_list; ptr != NULL; ptr = ptr->next_list)
        {
          if (ptr == element)
            {
              break;
            }
          prev = ptr;
        }
      if (ptr != element)
        L_punt ("L_delete_oper_list: element not found in list");

      prev->next_list = element->next_list;
      element->next_list = NULL;
      L_free (L_alloc_oper_list, element);

      return (first);
    }
}

void
L_delete_all_oper_list (L_Oper_List * delete_list)
{
  L_Oper_List *list, *next_list;

  for (list = delete_list; list != NULL; list = next_list)
    {

      next_list = list->next_list;

      list->oper = NULL;
      list->next_list = NULL;
      L_free (L_alloc_oper_list, list);
    }
}

L_Oper_List *
L_concat_oper_list (L_Oper_List * list1, L_Oper_List * list2)
{
  L_Oper_List *ptr, *prev;

  if (list1 == NULL)
    return (list2);
  if (list2 == NULL)
    return (list1);

  prev = list1;
  ptr = list1->next_list;
  while (ptr != NULL)
    {
      prev = ptr;
      ptr = ptr->next_list;
    }
  prev->next_list = list2;

  return (list1);
}

L_Oper_List *
L_find_oper_list (L_Oper_List * list, L_Oper * oper)
{
  L_Oper_List *ptr;

  for (ptr = list; ptr != NULL; ptr = ptr->next_list)
    {
      if (ptr->oper == oper)
        return (ptr);
    }

  return (NULL);
}

L_Program *
L_new_program (char *name)
{
  L_Program *program = (L_Program *) L_alloc (L_alloc_program);
  program->name = name;
  program->first_func = NULL;
  program->last_func = NULL;

  return program;
}

void
L_delete_program (L_Program * program)
{
  L_Func *next;

  while (program->first_func)
    {
      next = program->first_func->next_func;
      if (next != NULL)
        next->prev_func = NULL;

      L_fn = program->first_func;
      L_delete_func (program->first_func);

      program->first_func = next;
    }

  /* delete the func */
  L_free (L_alloc_program, program);
}
