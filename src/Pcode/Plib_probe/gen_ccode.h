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

/* CWL -
   P2C_NEW_PARAM_STYLE and P2C_OLD_PARAM_TYLE control printing function
   parameters in new format or old format, when directly converting Pcode to
   C code.
   new format: func(t1 p1, t2 p2, ...)
   old format: func(p1, p2) t1 p1; t2 p2; ...
*/

#include <config.h>
#include <Pcode/pcode.h>


/* style */
#define P2C_NEW_PARAM_STYLE		0
#define P2C_OLD_PARAM_STYLE 		1
#if 0
#define P2C_PRINT_STORAGE_CLASS 	2
#endif
#define P2C_PRINT_LONGLONG	 	4
#define P2C_PRETTY_EXPR 		8

/* 4/30/04 REK Adding option to determine whether or not we use typedefs. */
#define P2C_USE_TYPEDEFS              0

extern int Gen_CCODE_Struct (FILE * FL, StructDcl st);
extern int Gen_CCODE_Union (FILE * FL, UnionDcl un);
extern int Gen_CCODE_Asm (FILE * FL, AsmDcl ad);
extern int Gen_CCODE_GlobalVar (FILE * FL, VarDcl var);
extern int Gen_CCODE_Func (FILE * FL, FuncDcl func);
extern int Gen_CCODE_Expr (FILE * FL, Expr expr);
extern int Gen_CCODE_Type (FILE * FL, Key type, int style);
extern int Gen_CCODE_Indent (FILE * FL, int level);
extern int Gen_CCODE_Include (FILE * FL, char *file);
extern int Gen_CCODE_Typedef (FILE * FL, TypeDcl td);
