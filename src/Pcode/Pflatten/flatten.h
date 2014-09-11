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

#ifndef _PFLATTEN_FLATTEN_H_
#define _PFLATTEN_FLATTEN_H_

#include <config.h>
#include <Pcode/pcode.h>

#define PF_MINIMIZE_TEMPORARIES    1

extern int PF_debug;
extern int PF_reduce;

extern Expr PF_NewLocalVar (Stmt stmt, Type type);
extern Label PF_ExtractLabels (Stmt stmt);
extern void PF_Flatten (FuncDcl func);
extern int PF_FindExpandableCallStmts (Stmt stmts);



extern bool IsUselessVar (Expr expr, char *str);
extern void Clear_Useless_Var_Table (void);
extern void P_FindNestedIf (Stmt stmts);

/* pf_array.c */

extern void PF_RestructureArrayAccesses (FuncDcl func);

/* pf_restructure.c */

extern void PF_Restructure (FuncDcl func);

/* pf_utility.c */

extern void PF_CopyLineInfo (Stmt src, Stmt dst);
extern int PF_HasFallThrough (Stmt stmt);
#if 0
extern void PF_StmtInsertStmtBefore (Stmt s, Stmt new);
extern void PF_StmtInsertStmtAfter (Stmt s, Stmt new);
extern void PF_StmtInsertExprBefore (Stmt s, Expr e);
#endif

/* Library interface to Pflatten.  These must take a symbol table as an
 * argument. */
extern void PF_Init (char *prog_name, Parm_Macro_List *external_list);
extern void PF_read_parm_Pflatten (Parm_Parse_Info *ppi);
extern void PFT_Flatten (SymbolTable table, FuncDcl func);

#endif
