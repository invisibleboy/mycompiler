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

#ifndef __LEM_TRACE_H__
#define __LEM_TRACE_H__

#include <config.h>
#include <Lcode/l_emul.h>

extern void C_emit_put_trace_header (FILE * out, L_Func * fn,
				     L_Oper * op, char *header);
extern void C_emit_put_trace_exception_state (FILE * out, L_Func * fn,
					      L_Oper * op);
extern void C_emit_put_trace_int (FILE * out, L_Func * fn, L_Oper * op,
				  char *header, int value);
extern void C_emit_put_trace_two_ints (FILE * out, L_Func * fn, L_Oper * op,
				       char *header, int value1, int value2);
extern void C_emit_put_trace_int_operand (FILE * out, L_Func * fn,
					  L_Oper * op, char *header,
					  L_Operand * operand);
extern void C_emit_put_trace_pred_operand (FILE * out, L_Func * fn,
					   L_Oper * op, char *header,
					   L_Operand * operand);
extern void C_emit_put_trace_post_jsr (FILE * out, L_Func * fn, L_Oper * op,
				       int jsr_id, int func_id);
extern void C_emit_put_trace_mem_addr (FILE * out, L_Func * fn, L_Oper * op,
				       char *header, L_Operand * operand1,
				       L_Operand * operand2);
extern void C_emit_fn_trace_system_setup (FILE * out);
extern void C_emit_trace_system_prototypes (FILE * out);

#endif /* __LEM_TRACE_H__ */
