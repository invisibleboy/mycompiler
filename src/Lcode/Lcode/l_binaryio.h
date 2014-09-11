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
 *      File :          l_binaryio.h
 *      Description :   Binary Lcode io special characters and prototypes
 *
 *
 *      Original: Richard E. Hank, Wen-mei Hwu April 1996
 *      Modified : 
 *
\*****************************************************************************/
#ifndef L_BINARYIO_H
#define L_BINARYIO_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/* Delimiter bit used to denote the end of a token */
#define DELIMIT 0x80

/* Special character bit used denote many things. :) */
#define SP_CHAR 0x40

/* Define a minus sign as a special character - hex 0x2d */
#define MINUS   ( SP_CHAR | 0x00)

/* Binary file and version indicator */
#define BINARY_VERSION_1        (DELIMIT | 0x1)
#define BINARY_VERSION_2        (DELIMIT | 0x2)
/* .
   .
   .
 */

#define CURRENT_BINARY_VERSION  BINARY_VERSION_1

#ifdef __cplusplus
extern "C"
{
#endif

  extern int L_binary_peek_next_char (FILE * in);
  extern int L_binary_read_char (FILE * in);
  extern int L_binary_read_int (FILE * in);
  extern ITintmax L_binary_read_intmax (FILE * in);
  extern char *L_binary_read_string (FILE * in, L_Input_Buf * input_buf);
  extern char *L_binary_peek_next_string (FILE * in, L_Input_Buf * input_buf);
  extern void L_binary_write_double (FILE * F, double d);
  extern void L_binary_write_int (FILE * F, int i);
  extern void L_binary_write_intmax (FILE * F, ITintmax i);
  extern void L_binary_write_int_special (FILE * F, int i);
  extern void L_binary_write_string (FILE * F, char *string);
  extern void L_check_binary_file_version (int ch);
  extern int L_file_is_binary (int ch);
  extern int L_is_binary_magic_header (int ch);
  extern void L_print_attr_binary (FILE * F, L_Attr * attr);
  extern void L_print_cb_binary (FILE * F, L_Func * fn, L_Cb * cb);
  extern void L_print_data_binary (FILE * F, L_Data * data);
  extern void L_print_expr_binary (FILE * F, L_Expr * expr);
  extern void L_print_datalist_element_binary (FILE * F,
                                               L_Datalist_Element * element);
  extern void L_print_datalist_binary (FILE * F, L_Datalist * list);
  extern void L_print_func_binary (FILE * F, L_Func * fn);
  extern void L_print_operand_binary (FILE * F, L_Operand * opd);
  extern void L_print_operand_id (FILE * F, L_Operand * opd);
  extern void L_print_oper_binary (FILE * F, L_Oper * oper);
  extern void L_print_sync_binary (FILE * F, L_Sync * sync);
  extern L_Attr *L_read_attr_binary (FILE * F);
  extern L_Cb *L_read_cb_binary (FILE * F, L_Input_Buf * buf, L_Func * fn,
                                 int *num_opers);
  extern void L_read_data_binary (FILE * F, int type, L_Input_Buf * buf);
  extern void L_read_hash_tbl_binary (FILE * F, L_Datalist * tbl,
                                      L_Input_Buf * input_buf);
  extern L_Datalist *L_read_all_hashtbls_binary (FILE * F, int num_tbls,
                                                 L_Input_Buf * input_buf);
  extern L_Event *L_read_event_list_binary (FILE * F, L_Input_Buf * buf);
  extern L_Expr *L_read_expr_binary (FILE * F, L_Input_Buf * buf);
  extern L_Flow *L_read_flow_binary (FILE * F, L_Input_Buf * buf, L_Func * fn,
                                     L_Cb * src_cb);
  extern void L_read_fn_attributes_binary (FILE * F, L_Input_Buf * buf,
                                           L_Func * fn, int num_attr);
  extern void L_read_fn_binary (FILE * F, L_Input_Buf * input_buf);
  extern L_Operand *L_read_operand_binary (FILE * F, L_Input_Buf * buf,
                                           L_Func * fn);
  extern L_Oper *L_read_oper_binary (FILE * F, L_Input_Buf * buf,
                                     L_Func * fn);
  extern L_Oper *L_read_parent_oper_binary (FILE * F, L_Input_Buf * buf);
  extern L_Oper *L_read_rest_oper_binary (FILE * F, L_Input_Buf * buf,
                                          L_Oper * oper);
  extern L_Sync *L_read_sync_binary (FILE * F, L_Input_Buf * buf);

#ifdef __cplusplus
}
#endif

#endif
