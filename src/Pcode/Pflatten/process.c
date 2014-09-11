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
 *	File:	process.c
 *	Author:	Nancy Warter and Wen-mei Hwu
 *	Modified from code written by: Po-hua Chang
 * 	Copyright (c) 1991 Nancy Warter, Po-hua Chang, Wen-mei Hwu
 *			and The Board of Trustees of the University of Illinois.
 *			All rights reserved.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab_i.h>
#include <Pcode/perror.h>

#include "process.h"
#include "flatten.h"


Expr
PF_ReturnReducedExpr (Expr expr)
{
  Stmt parent_stmt;
  Expr new_expr;

  parent_stmt = expr->parentstmt;
  new_expr = PSI_ReduceExpr (expr);
  expr = PSI_RemoveExpr (expr);

  return new_expr;
}


void
PF_ReduceAllStmt (Stmt stmt)
{
  SerLoop serloop;
  ParLoop parloop;
  Stmt prologue, body = NULL;

  while (stmt)
    {
      switch (stmt->type)
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_ASM:
	  break;
	case ST_RETURN:
	  if (stmt->stmtstruct.ret)
	    stmt->stmtstruct.ret = PF_ReturnReducedExpr (stmt->stmtstruct.ret);
	  break;
	case ST_COMPOUND:
	  PF_ReduceAllStmt (stmt->stmtstruct.compound->stmt_list);
	  break;
	case ST_IF:
	  stmt->stmtstruct.ifstmt->cond_expr = 
	    PF_ReturnReducedExpr (stmt->stmtstruct.ifstmt->cond_expr);
	  PF_ReduceAllStmt (stmt->stmtstruct.ifstmt->then_block);
	  if (stmt->stmtstruct.ifstmt->else_block)
	    PF_ReduceAllStmt(stmt->stmtstruct.ifstmt->else_block);
	  break;
	case ST_SWITCH:
	  stmt->stmtstruct.switchstmt->expression = 
	    PF_ReturnReducedExpr (stmt->stmtstruct.switchstmt->expression);
	  PF_ReduceAllStmt (stmt->stmtstruct.switchstmt->switchbody);
	  break;
	case ST_SERLOOP:
	  serloop = stmt->stmtstruct.serloop;
	  if (serloop->init_expr)
	    serloop->init_expr = PF_ReturnReducedExpr (serloop->init_expr);
	  if (serloop->cond_expr)
	    serloop->cond_expr = PF_ReturnReducedExpr (serloop->cond_expr);
	  if (serloop->iter_expr)
	    serloop->iter_expr = PF_ReturnReducedExpr (serloop->iter_expr);
	  PF_ReduceAllStmt (serloop->loop_body);
	  break;
	case ST_EXPR:
	  stmt->stmtstruct.expr = PF_ReturnReducedExpr (stmt->stmtstruct.expr);
	  break;
	case ST_PARLOOP:
	  parloop = stmt->stmtstruct.parloop;
	  parloop->iteration_var = \
	    PF_ReturnReducedExpr (parloop->iteration_var);
	  parloop->init_value = PF_ReturnReducedExpr (parloop->init_value);
	  parloop->final_value = PF_ReturnReducedExpr (parloop->final_value);
	  /* 1/14/04 REK */
#if 0
	  if (FindFlowNodeInList (NT_ParloopExit,
				  stmt->flow->entry_flow_node->succ))
	    parloop->final_value = PF_ReturnReducedExpr (parloop->final_value);
#endif
	  parloop->incr_value = PF_ReturnReducedExpr (parloop->incr_value);

	  /* prologue */
	  prologue = \
	    P_GetPstmtStmt (P_GetParLoopPstmt (P_GetStmtParLoop (stmt)));
	  PF_ReduceAllStmt (prologue->stmtstruct.compound->stmt_list);

	  /* 1/14/04 REK */
#if 0
	  /* body */
	  body = Parloop_Stmts_Body_Stmt (stmt);
	  ReduceAllStmt(body->stmtstruct.bodystmt->
					    statement);
#endif
	  /* epilogue */
	  PF_ReduceAllStmt (body->lex_next);
	  break;
	case ST_BODY:
	  return;
	case ST_EPILOGUE:
	  PF_ReduceAllStmt (stmt->stmtstruct.epiloguestmt->statement);
	  break;
	default:
	  P_punt("ReduceAllStmts: Invalid statement type\n");
	}
      stmt = stmt->lex_next;
    }
}


void
PF_CreateShadowStmt (FuncDcl func)
{
  VarDcl var_dcl;
  Stmt stmt;
  Expr op1, op2, assign;
  Key type_key;
  Stmt first_stmt;
  Key var_key;
  Key scope_key;
  int i;
  
  if (!func->param)
    return;

  first_stmt = func->stmt->stmtstruct.compound->stmt_list;
  while (first_stmt->artificial)
    first_stmt = first_stmt->lex_next;

  scope_key = PSI_GetStmtScope (first_stmt);

  /* Add in the assignments */
  List_start (func->param);
  for (var_dcl = (VarDcl)List_next (func->param), i = 1;
       var_dcl; var_dcl = (VarDcl)List_next (func->param), i++)
    {
      var_key = P_GetVarDclKey (var_dcl);
      type_key = P_GetVarDclType (var_dcl);

      op1 = PSI_ScopeNewExprWithOpcode (scope_key, OP_var);
      P_SetExprVarName (op1, strdup (P_GetVarDclName (var_dcl)));
      P_SetExprVarKey (op1, var_key);

      op2 = PSI_ScopeNewIntExpr (scope_key, 0);

      assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
      P_AppendExprOperands (assign, op1);
      P_AppendExprOperands (assign, op2);

      /* Create the shadow statement */

      stmt = P_NewStmt ();
      stmt->artificial = 0;
      P_SetStmtType (stmt, ST_EXPR);
      stmt->type = ST_EXPR;
      P_SetStmtExpr (stmt, assign);

      stmt->shadow = P_NewShadowWithExprID(stmt->shadow, assign, i);
      P_SetExprParentStmtAll (stmt->stmtstruct.expr, stmt);
      P_StmtInsertStmtBeforeLabel (first_stmt, stmt);
    }
  return;
}


void
PF_ProcessFuncDcl (FuncDcl func)
{
  if (!P_fast_mode)
    PF_Flatten (func);

  /* EMN - 9/2002 - Reduce all expressions (Moved from PtoL) */

  if (PF_reduce)
    PF_ReduceAllStmt (func->stmt);

  /* EMN - 9/2002 - Add in parameter shadows */
  PF_CreateShadowStmt (func);
}

