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
 *	File:	pf_array.c
 *	Author:	Chien-wei Li and John W. Sias and Wen-mei Hwu
\*****************************************************************************/

#include <config.h>
#include <library/i_error.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab_i.h>
#include <Pcode/query.h>
#include "flatten.h"
#include "data.h"

static int
PF_TypesMatchPointerArithmetics (Type type1, Type type2)
{
  if (!PSI_IsPointerType (type1))
    return 0;

  if (!PSI_IsArrayType (type2) || !PSI_IsArrayType (PSI_GetTypeType (type2)))
    return 0;

  type1 = PSI_GetTypeType (type1);
  type2 = PSI_GetTypeType (type2);
  type2 = PSI_GetTypeType (type2);

  while (!PSI_IsBaseType (type1) && !PSI_IsBaseType (type2))
    {
      if (PSI_GetTypeBasicType (type1) != PSI_GetTypeBasicType (type2) || \
	  !PSI_IsArrayType (type1))
	return 0;

      if (PSI_MatchExpr (PSI_GetTypeArraySize (type1),
			 PSI_GetTypeArraySize (type2)))
	return 0;

      type1 = PSI_GetTypeType (type1);
      type2 = PSI_GetTypeType (type2);
    }
  
  if (!PSI_IsBaseType (type1) || !PSI_IsBaseType (type2))
    return 0;

  return 1;
}


static void
PF_RestructureArrayAccesses_Expr (Expr expr, void *data)
{
  Expr cast_expr;

  if (!expr)
    return;

  if (expr->operands &&
      expr->operands->operands &&
      PSI_IsPointerType (PSI_ExprType (expr->operands)) &&
      expr->operands->opcode == OP_cast &&
      expr->operands->operands->opcode == OP_add &&
      PF_TypesMatchPointerArithmetics (PSI_ExprType (expr->operands),
				       PSI_ExprType (expr->operands->operands->operands)))
    {
      cast_expr = expr->operands;
      expr->operands = cast_expr->operands;
      expr->operands->opcode = OP_index;
      expr->operands->parentexpr = expr;
      expr->operands->sibling = cast_expr->sibling;
      cast_expr->next = cast_expr->operands = cast_expr->sibling = 0;
      cast_expr = PSI_RemoveExpr (cast_expr);
      PSI_CastExpr (expr->operands);
    }
  return;
}


void
PF_RestructureArrayAccesses (FuncDcl func)
{
  P_StmtApplyPost (func->stmt, NULL,
		   PF_RestructureArrayAccesses_Expr, NULL);
  return;
}
