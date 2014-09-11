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
 *      File :          l_io.h
 *      Description :   Lcode input structures/prototypes
 *      Original : Pohua Chang, Wen-mei Hwu June 1990
 *      Modified : Richard E. Hank, February 1994
 *              Lcode is no longer parsed by lex.  This makes the Lcode
 *              reader much easier to modify and greatly improves speed.
 *
\*****************************************************************************/
#ifndef L_IO_H
#define L_IO_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_code.h>

typedef struct
{
  char *line_buf;
  char *peek_buf;
  char *token_buf;
  char *token_start;
  char *token_end;
  int line_len;
  int peek_len;
  int token_len;
  int max_token_len;
  int max_line_len;
  int eof;
  int line_count;
}
L_Input_Buf;


/*
 *      L_File
 */

typedef struct L_File
{
  char *input_file;
  char *output_file;
}
L_File;


#ifdef __cplusplus
extern "C"
{
#endif

  extern L_Input_Buf L_input_buf;

  extern char L_peek_token[];

  extern int L_func_contains_dep_pragmas;
  extern int L_func_contains_jsr_dep_pragmas;

  extern int L_func_acc_specs;
  extern int L_func_acc_omega;

/*
 *      Functions to read/print data structs
 */

  extern void L_addc_to_line_buffer (L_Input_Buf * input_buf, char ch);
  extern void L_close_input_file (char *file_name);
  extern void L_close_output_file (FILE * F);
  extern void L_create_filelist (void);
  extern void L_delete_filelist (void);
  extern void L_free_input_buf (L_Input_Buf * input_buf);
  extern int L_get_input (void);
  extern char *L_get_next_lcode_token (L_Input_Buf * input_buf);
  extern void L_get_next_lcode_token_expecting (L_Input_Buf * input_buf,
                                                char *expect, char *msg);
  extern char *L_peek_next_real_lcode_token (L_Input_Buf * input_buf);
  extern void L_init_input_buf (L_Input_Buf * input_buf);
  extern int L_is_token_char (int ch);
  extern void L_mark_leaf_func (L_Func * fn);
  extern void L_open_input_file (char *file_name);
  extern FILE *L_open_output_file (char *name);
  extern char L_peek_next_char (L_Input_Buf * input_buf);
  extern void L_print_appendix (FILE * F, L_Func * fn);
  extern void L_print_attr (FILE * F, L_Attr * attr);
  extern void L_print_buf_with_arrow (FILE * out, L_Input_Buf * input_buf);
  extern void L_print_cb (FILE * F, L_Func * fn, L_Cb * cb);
  extern void L_print_data (FILE * F, L_Data * data);
  extern void L_print_datalist (FILE * F, L_Datalist * list);
  extern void L_print_datalist_element (FILE * F,
                                        L_Datalist_Element * element);
  extern void L_print_expr (FILE * F, L_Expr * expr);
  extern void L_print_func (FILE * F, L_Func * fn);
  extern void L_print_oper (FILE * F, L_Oper * oper);
  extern void L_print_operand (FILE * F, L_Operand * operand, int flag);
  extern void L_print_operand_buffer (char *buf, L_Operand * operand,
                                      int flag);
  extern void L_print_oper_list (FILE * F, L_Oper_List * list);
  extern void L_print_sync (FILE * F, L_Sync * sync);
  extern void L_print_expression (FILE * F, L_Expression * expression);
  extern void L_read_appendix (L_Func * fn, L_Input_Buf * input_buf);
  extern L_Attr *L_read_attr (L_Func * fn, L_Input_Buf * input_buf);
  extern void L_read_cb_appendix (L_Func * fn, L_Input_Buf * input_buf);
  extern void L_read_data (int type, L_Input_Buf * input_buf);
  extern void L_read_hash_tbl (L_Input_Buf * input_buf, L_Datalist * tbl);
  extern L_Datalist *L_read_all_hashtbls (L_Input_Buf * input_buf,
                                          int num_tbls);
  extern L_Expr *L_read_expr (L_Input_Buf * input_buf);
  extern L_Flow *L_read_flow (L_Func * fn, L_Cb * src_cb,
                              L_Input_Buf * input_buf);
  extern void L_read_fn (L_Input_Buf * input_buf);
  extern void L_read_fn_attributes (L_Func * fn, L_Input_Buf * input_buf);
  extern void L_read_fn_flags (L_Func * fn, L_Input_Buf * input_buf);
  extern L_Oper *L_read_oper (L_Func * fn, L_Input_Buf * input_buf);
  extern L_Operand *L_read_operand (L_Func * fn, L_Input_Buf * input_buf);
  extern void L_read_oper_appendix (L_Func * fn, L_Input_Buf * input_buf);
  extern void L_read_oper_flags (L_Oper * oper, L_Input_Buf * input_buf);
  extern L_Oper *L_read_parent_oper (L_Func * fn, L_Input_Buf * input_buf);
  extern L_Sync *L_read_sync (L_Input_Buf * input_buf);
  extern int L_refill_input_buf (L_Input_Buf * input_buf);
  extern int L_seek (int offset);
  extern int L_strtod (char *start, char **end, double *num);
  extern int L_strtol (char *start, char **end, int *num);
  extern int L_tell (void);
  extern void _L_addc_to_peek_buffer (L_Input_Buf * input_buf, char ch);
  extern void _L_addc_to_token_buffer (L_Input_Buf * input_buf, char ch);
/* LCW - read type information and bit field length - 4/18/96 */
  extern L_Type *L_read_type (L_Input_Buf * input_buf, L_Expr * bit_field);
  /* HCH 5/10/04: for getting object IDs */
  extern int L_read_id (L_Input_Buf * input_buf, L_Expr * bit_field);

  extern L_Oper* DB_id2op(L_Func * fn, int opid);
  extern L_Cb* DB_id2cb(L_Func * fn, int cbid);
  extern void DB_print_df_set_opid(L_Func *fn, int opid, char *type);
  extern void DB_set (Set set);
  extern void DB_spit_func (L_Func * fn, char *name);
  extern void DB_spit_cb (L_Cb * cb, char *name);
  extern void DB_print_func (L_Func * fn);
  extern void DB_fn (L_Func * fn);
  extern void DB_print_cb (L_Cb * cb);
  extern void DB_print_cbid (L_Func * fn, int cbid);
  extern void DB_cb (L_Cb * cb);
  extern void DB_print_oper (L_Oper * oper);
  extern void DB_print_opid (L_Func * fn, int opid);
  extern void DB_test_all (L_Func * fn);
  extern void DB_print_set (char *msg, Set s);
  extern FILE *DB_out ();
  extern FILE *DB_err ();
  void DB_print_flows (L_Flow * flow);

#ifdef __cplusplus
}
#endif

#endif
