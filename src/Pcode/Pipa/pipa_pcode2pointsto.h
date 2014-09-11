/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    pipa_pcode2pointsto.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef __PIPA_PCODE2POINTSTO_H_
#define __PIPA_PCODE2POINTSTO_H_

#include "pipa_common.h"
#include "pipa_program.h"
#include "pipa_consg_construct.h"
#include <Pcode/ss_ssa2.h>

#define IPA_VARKIND_LOCAL     (0)
#define IPA_VARKIND_GLOBAL    (IPA_VAR_KIND_GLOBAL)
#define IPA_VARKIND_HEAP      (IPA_VAR_KIND_HEAP)
#define IPA_VARKIND_STACK     (IPA_VAR_KIND_STACK)
#define IPA_VARKIND_FUNC      (IPA_VAR_KIND_GLOBAL | IPA_VAR_KIND_FUNC)
#define IPA_VARKIND_PARAM     (IPA_VAR_KIND_PARAM)
#define IPA_VARKIND_RETURN    (IPA_VAR_KIND_RETURN)

/****************************************************************
 * Pcode helper functions
 ****************************************************************/
Key
IPA_ExprType(IPA_prog_info_t *info, Expr expr);

TypeDcl
IPA_Pcode_get_typedcl(IPA_prog_info_t *info, 
		      Key type_key);

Field
IPA_Pcode_get_field (IPA_prog_info_t *info,
		     Key type_key, char *field_name);

int 
IPA_Pcode_sizeof (IPA_prog_info_t *info,
		  Key type_key);

Key
IPA_ExprType(IPA_prog_info_t *info, Expr expr);

int
IPA_is_ptr_invalidating (int opcode);

int
IPA_Pcode_IsFunctionType(IPA_prog_info_t *info, Key type_key);
int
IPA_Pcode_IsPointerType(IPA_prog_info_t *info, Key type_key);
int
IPA_Pcode_IsArrayType(IPA_prog_info_t *info, Key type_key);
int
IPA_Pcode_IsStructureType(IPA_prog_info_t *info, Key type_key);
int
IPA_Pcode_IsStructureArrayType(IPA_prog_info_t *info, Key type_key);

/****************************************************************
 * Pointsto helper functions
 ****************************************************************/

IPA_symbol_info_t *
IPA_Pcode_Get_Sym (IPA_prog_info_t * info, Key sym_key);

IPA_symbol_info_t *
IPA_Pcode_Add_Sym (IPA_prog_info_t * info,
		   char *sym_name, Key sym_key,
		   Key type_key, int flags);

void 
IPA_Pcode_Add_CallSite (IPA_prog_info_t * info, 
			Expr call_expr);



/****************************************************************
 * Find all assignments
 ****************************************************************/

void IPA_Find_All_Assignments_In_Exprs (IPA_prog_info_t * info, Expr expr);
void IPA_Find_All_Assignments_In_Stmts (IPA_prog_info_t * info, Stmt stmt);


/****************************************************************
 * 
 ****************************************************************/

void
IPA_Expr_Addr_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg);
void
IPA_Expr_Deref_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg);
void
IPA_Expr_Field_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg);
void
IPA_Expr_Arrow_Eqn (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg);
void
IPA_Expr_Handle_Addition (IPA_prog_info_t * info, Expr expr, buildcg_t *bcg);
int 
IPA_is_ptr_invalidating (int opcode);


/****************************************************************
 * High-level equation builders
 ****************************************************************/

void IPA_Add_Root_Points_To_Relations (IPA_prog_info_t * info, Expr expr);

void IPA_BuildEqns_For_Func (FuncDcl func, IPA_prog_info_t * info);
void IPA_BuildEqns_For_Var (IPA_prog_info_t * info, VarDcl var, Key scope_key);

buildcg_t *
IPA_BuildEqns_For_Expr (IPA_prog_info_t * info, 
			Expr root_expr, Expr var_expr);

List
IPA_Find_All_Expr_In_Stmts (IPA_prog_info_t *info, Stmt stmt,
			    List list, buildcg_mode_t mode);

List
IPA_Find_All_Expr_In_CFG (IPA_prog_info_t *info, PC_Graph cfg,
			  buildcg_mode_t mode);

void
IPA_Find_Special_Local_Vars_in_Stmts (IPA_prog_info_t *info, Stmt stmt);

#endif
