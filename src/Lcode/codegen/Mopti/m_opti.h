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
 *  File:  m_opti.h
 *
 *  Description:
 *      Performs machine level code optimization:
 *	1) common subexpression ellimination
 *	2) limited copy propogation (only R-R, R-M, M-R, M-M)
 *	3) dead code removal (unused operations, src1=dest)
 *
 *	This code is base-lined off the work developed by Scott Mahlke in
 *	l_basic_opti.c
 *
 *  Creation Date :  July 1991
 *
 *  Author:  Roger A. Bringmann
 *
 *  Revisions:
 *	Roger A. Bringmann, February 1993
 *	Modified to support new Lcode format.  Reduces memory requirements
 *	for a code generator.  Also, adds a more friendly interface for
 *	code generation.
 *
 * 	(C) Copyright 1991, Roger A. Bringmann
 * 	All rights granted to University of Illinois Board of Regents.
 *	All rights granted to AMD Inc.
 *
\*****************************************************************************/
#ifndef M_OPTI_H
#define M_OPTI_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void L_read_parm_mopti ( Parm_Parse_Info *ppi );
extern void Mopti_init ( Parm_Macro_List *command_line_macro_list );
extern void Mopti_perform_optimizations ( L_Func *fn,
			Parm_Macro_List *command_line_macro_list );

#ifdef __cplusplus
}
#endif

#endif
