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
 *      File: l_emul_builtin.h (version 2)
 *      Authors: IMPACT Technologies, Inc. (John C. Gyllenhaal)
 *      Creation Date:  March 1999
 * 
 *      This is a complete reengineering and rewrite of version 1 of
 *      of Lemulate, which was written by Qudus Olaniran, Dan Connors,
 *      IMPACT Technologies, Inc, and Wen-mei Hwu.  This new version is
 *      optimized for portability, performance, and hopefully clarity.
 *
\*****************************************************************************/

#ifndef __LEM_BUILTIN_H__
#define __LEM_BUILTIN_H__

#include <config.h>
#include <Lcode/l_emul.h>

extern int C_is_builtin_function (char *name);
extern int C_get_builtin_parms (char *name, L_Oper * op,
				char all_parm_type_buf[]);
extern void C_check_builtin_parms (char *name, L_Oper * op, int num_expected);
extern void C_emit_builtin_va_start_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_builtin_stdarg_start_op (FILE * out, L_Func * fn,
					    L_Oper * op);
extern void C_emit_builtin_next_arg_op (FILE * out, L_Func * fn, L_Oper * op);
extern void C_emit_builtin_jsr_op (FILE * out, L_Func * fn, L_Oper * op);

#endif /* __LEM_BUILTIN_H__ */
