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
 *      File: l_emul_operand.h
 *      Authors: IMPACT Technologies, Inc. (Jake Janovetz)
 *      Creation Date:  June 1999
 *
 *      This file performs intrinsic emulation support for Lemulate.
 *      Intrinsic operations get converted to function calls which
 *      perform the equivalent function.  The intrinsic emulatiointrinsicn 
 *      functions are external and are linked with the emulation code.
 *
\*****************************************************************************/

#ifndef __LEM_OPERAND_H__
#define __LEM_OPERAND_H__

#include <config.h>
#include <Lcode/l_emul.h>

extern void C_check_operands (char *name, L_Oper * op, int num_dest,
			      int num_src);
extern int C_is_cb_label (char *label);
extern char *C_true_name (char *name);
extern int C_matches_true_name (char *name, char *test_name);
extern char *C_macro_name (int mac_id);
extern void C_emit_int_reg (FILE * out, int reg_id);
extern void C_emit_pred_reg (FILE * out, int reg_id);
extern void C_emit_float_reg (FILE * out, int reg_id);
extern void C_emit_double_reg (FILE * out, int reg_id);
extern void C_emit_int_mac (FILE * out, int mac_id);
extern void C_emit_pred_mac (FILE * out, int mac_id);
extern void C_emit_float_mac (FILE * out, int mac_id);
extern void C_emit_double_mac (FILE * out, int mac_id);
extern void C_emit_operand (FILE * out, L_Func * fn, L_Operand * operand);
extern void C_emit_register_rotation (FILE * out, L_Func * fn);
extern void C_emit_func_ptr_cast (FILE * out, char *return_type_buf,
				  char *all_parm_type_buf);
extern void C_emit_jsr_parms (FILE * out, L_Func * fn, L_Oper * op,
			      char all_parm_type_buf[], int first_index);
extern void C_emit_uncond_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op,
					 L_Operand * dest, char *modifier,
					 char *cast, char *operator);
extern void C_emit_cond_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op,
				       L_Operand * dest, char *assign,
				       char *cast, char *operator);
extern void C_emit_sand_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op,
				       L_Operand * dest, char *modifier,
				       char *cast, char *operator);
extern void C_emit_pred_def_dest (FILE * out, L_Func * fn, L_Oper * op,
				  L_Operand * dest, char *cast,
				  char *operator);
extern char *C_get_compare_token (ITuint8 com);
extern char *C_get_compare_cast (ITuint8 ctype);

#endif /* __LEM_OPERAND_H__ */
