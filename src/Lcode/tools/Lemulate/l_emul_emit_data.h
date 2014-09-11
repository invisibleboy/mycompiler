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
 *      File: l_emul_data.h
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulatiointrinsicn 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/

#ifndef __LEM_DATA_H__
#define __LEM_DATA_H__

#include <config.h>
#include <Lcode/l_emul.h>

/* HCH 3-12-02: track initializations of globs for mem tracing */
typedef struct _InitInfo
{
  int data_size;
  int tot_inits;
} 
InitInfo;

extern int C_is_init_data (int token_type);
extern void C_emit_data_init_function (FILE * init_out, FILE * extern_out,
				       Mem_Usage * mem_usage);
extern void C_emit_data (FILE * out, FILE * extern_out, FILE * struct_out,
			 L_Data * data, Mem_Usage * mem_usage,
			 L_Datalist_Element ** jump_tbl_data);
extern void C_emit_fn_jump_tables (FILE * out, FILE * extern_out,
				   FILE * struct_out, L_Func * fn,
				   Mem_Usage * mem_usage);

extern InitInfo C_emit_reserve_data (FILE * out, FILE * extern_out,
				     FILE * struct_out,
				     L_Data * data, Mem_Usage * mem_usage,
				     L_Datalist_Element ** jump_tbl_data);
extern char *C_get_data_label (L_Expr * expr);
extern char *C_get_raw_data_label (L_Expr * expr);
extern char *C_get_data_string (L_Expr * expr);
extern int C_get_data_int (L_Expr * expr);
extern void C_emit_expr (FILE * out, L_Expr * expr);
extern void C_emit_base_type (FILE * out, FILE * extern_out, L_Data * data,
			      Mem_Usage * mem_usage, char *default_type_name);
extern void C_verify_no_data_value (L_Data * data);

#endif /* __LEM_DATA_H__ */
