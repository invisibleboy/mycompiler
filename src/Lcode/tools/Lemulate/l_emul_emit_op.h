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
 *      File: l_emul_trace.h
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulatiointrinsicn 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/

#ifndef __LEM_OP_H__
#define __LEM_OP_H__

#include <config.h>
#include <Lcode/l_emul.h>

extern int C_Lencode_ignores_oper (L_Oper * op);
extern void C_emit_1_dest_1_src_op (FILE * out, L_Func * fn, L_Oper * op,
				    char *prefix, char *postfix);
extern void C_emit_sz_ext_op (FILE * out, L_Func * fn, L_Oper * op, int pos,
			      int sgn);
extern void C_emit_abs_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_1_dest_2_src_op (FILE * out, L_Func * fn, L_Oper * op,
				    char *prefix, char *cast, char *operator,
				    char *postfix);
extern void C_emit_div_op (FILE * out, L_Func * fn, L_Oper * op,
			   char *prefix, char *operator, char *cast, char *postfix);
extern void C_emit_1_dest_3_src_op (FILE * out, L_Func * fn, L_Oper * op,
				    char *prefix, int index1, char *operator1,
				    int index2, char *operator2, int index3,
				    char *postfix);
extern void C_emit_std_load_op (FILE * out, L_Func * fn, L_Oper * op,
				char *cast, char *type);
extern void C_emit_post_load_op (FILE * out, L_Func * fn, L_Oper * op,
				 char *cast, char *type);
extern void C_emit_std_store_op (FILE * out, L_Func * fn, L_Oper * op,
				 char *cast);
extern void C_emit_post_store_op (FILE * out, L_Func * fn, L_Oper * op,
				  char *cast);
extern void C_emit_cond_br_op (FILE * out, L_Func * fn, L_Oper * op,
			       char *cast, char *operator);
extern void C_emit_jump_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_jump_rg_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_normal_jsr_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_jsr_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_rts_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_pred_set_op (FILE * out, L_Func * fn, L_Oper * op,
				int value);
extern void C_emit_pred_def_op (FILE * out, L_Func * fn, L_Oper * op,
				char *cast, char *operator);
extern void C_emit_check (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_op_comment (FILE * out, L_Oper * op);
extern void C_emit_simple_version (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_op (FILE * out, L_Func * fn, L_Oper * op);

#endif /* __LEM_OP_H__ */
