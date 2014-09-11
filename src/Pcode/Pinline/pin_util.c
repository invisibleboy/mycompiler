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
 *      File:   pin_util.c
 *      Author: Ben-Chung Cheng
 *      Copyright (c) 1995 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <library/list.h>
#include <Pcode/pcode.h>
#include <Pcode/symtab_i.h>
#include <Pcode/parloop.h>
#include "pin_inline.h"

/*! \brief Finds an OP_call and its cast and lhs, as appropriate, in an Expr
 *
 * \param expr
 *  Expr to be examined
 * \param pcall
 *  (output) Pointer to call expr
 * \param plhs
 *  (output) Pointer to lhs expr (if OP_assign found)
 * \param pcast
 *  (output) Pointer to cast expr (if found)
 * \return
 *  1 if a call is found ; 0 otherwise
 *
 * Examines \a expr for an OP_call expression, setting the variables
 * pointed to by the parameters to point to the OP_call, the LHS of
 * an enclosing assignment, and the OP_cast preceding the OP_call, as
 * appropriate.  Returns 1 if an OP_call is found; 0 otherwise.
 */
int
Pin_find_call (Expr expr, Expr *pcall, Expr *plhs, Expr *pcast)
{
  Expr call = NULL, lhs = NULL, cast = NULL;

  while (expr->opcode == OP_cast)
    expr = expr->operands;

  if (expr->opcode == OP_assign)
    {
      lhs = expr->operands;
      expr = lhs->sibling;

      if (expr->opcode == OP_cast)
	cast = expr;

      while (expr->opcode == OP_cast)
	expr = expr->operands;
    }

  if (expr->opcode == OP_call)
    call = expr;

  if (pcall)
    *pcall = call;

  if (plhs)
    *plhs = lhs;

  if (pcast)
    *pcast = cast;

  return (call != NULL);
}


static int
Pin_find_stack_size (VarList var_list)
{
  int size = 0;
  VarDcl var;

  List_start (var_list);
  while ((var = (VarDcl) List_next (var_list)))
    size += PSI_GetTypeSize (var->type);

  return size;
}


static void Pin_acc_body_size (Stmt stmts);

static int
Pin_expr_size (Expr expr)
{
  int size = 0;

  if (!expr)
    return 0;

  switch (expr->opcode)
    {
    case OP_var:
    case OP_enum:
    case OP_int:
    case OP_null:
    case OP_real:
    case OP_float:
    case OP_double:
    case OP_char:
    case OP_string:
      break;
    case OP_stmt_expr:
      Pin_acc_body_size (expr->value.stmt);
      break;
    default:
      size++;
    }

  if (expr->next)
    size += Pin_expr_size (expr->next);

  if (expr->operands)
    size += Pin_expr_size (expr->operands);

  if (expr->sibling)
    size += Pin_expr_size (expr->sibling);

  return size;
}


static long int Pin_body_size;
static long int Pin_exec_body_size;
static long int Pin_stack_size;


static void
Pin_acc_body_size (Stmt stmts)
{
  for (; stmts; stmts = stmts->lex_next)
    {
      SerLoop serloop;
      ParLoop parloop;
      int size, profhit;

      size = 0;
      profhit = (stmts->profile) ? (stmts->profile->count != 0) : 0;

      switch (stmts->type)
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	  size++;
	  break;
	case ST_RETURN:
	  size++;
	  size += Pin_expr_size (stmts->stmtstruct.ret);
	  break;
	case ST_COMPOUND:
	  Pin_stack_size += 
	    Pin_find_stack_size (stmts->stmtstruct.compound->var_list);
	  Pin_acc_body_size (stmts->stmtstruct.compound->stmt_list);
	  break;
	case ST_IF:
	  size += Pin_expr_size (stmts->stmtstruct.ifstmt->cond_expr);
	  Pin_acc_body_size (stmts->stmtstruct.ifstmt->then_block);
	  if (stmts->stmtstruct.ifstmt->else_block)
	    Pin_acc_body_size (stmts->stmtstruct.ifstmt->else_block);
	  break;
	case ST_SWITCH:
	  size += Pin_expr_size (stmts->stmtstruct.switchstmt->expression);
	  Pin_acc_body_size (stmts->stmtstruct.switchstmt->switchbody);
	  break;
	case ST_EXPR:
	  size += Pin_expr_size (stmts->stmtstruct.expr);
	  break;
	case ST_PSTMT:
	  Pin_acc_body_size (stmts->stmtstruct.pstmt->stmt);
	  break;
	case ST_MUTEX:
	  size += Pin_expr_size (stmts->stmtstruct.mutex->expression);
	  Pin_acc_body_size (stmts->stmtstruct.mutex->statement);
	  break;
	case ST_COBEGIN:
	  Pin_acc_body_size (stmts->stmtstruct.cobegin->statements);
	  break;
	case ST_SERLOOP:
	  serloop = stmts->stmtstruct.serloop;
	  size += Pin_expr_size (serloop->init_expr);
	  size += Pin_expr_size (serloop->cond_expr);
	  size += Pin_expr_size (serloop->iter_expr);
	  Pin_acc_body_size (serloop->loop_body);
	  break;
	case ST_PARLOOP:
	  parloop = stmts->stmtstruct.parloop;
	  size += Pin_expr_size (parloop->iteration_var);
	  size += Pin_expr_size (parloop->init_value);
	  size += Pin_expr_size (parloop->final_value);
	  size += Pin_expr_size (parloop->incr_value);
	  Pin_acc_body_size (Parloop_Stmts_Prologue_Stmt (stmts));
	  break;
	case ST_BODY:
	  Pin_acc_body_size (stmts->stmtstruct.bodystmt->statement);
	  break;
	case ST_EPILOGUE:
	  Pin_acc_body_size (stmts->stmtstruct.epiloguestmt->statement);
	  break;
	case ST_ASM:
	  size += Pin_expr_size (stmts->stmtstruct.asmstmt->asm_string);
	  size += Pin_expr_size (stmts->stmtstruct.asmstmt->asm_operands);
	  break;
	default:
	  P_punt ("Pin_acc_body_size: Invalid statement type");
	}

      Pin_body_size += size;

      if (profhit)
	Pin_exec_body_size += size;
    }

  return;
}


/*! \brief Estimates the "size" of a FuncDcl for inlining heuristics
 *
 * \param func
 *  FuncDcl to examine
 * \param psz_stk
 *  (output) Pointer to size of stack
 * \param psz_body
 *  (output) Pointer to size of function body (instructions)
 * \param psz_ebody
 *  (output) Pointer to touched size of function body (instructions)
 *
 * Provides a (crude) estimate of function body size and activation
 * record size for inlining heuristics.
 */
void
Pin_function_size (FuncDcl func, long int *psz_stk, long int *psz_body,
		   long int *psz_ebody)
{
  Pin_body_size = 0;
  Pin_exec_body_size = 0;
  Pin_stack_size = Pin_find_stack_size (func->param);

  Pin_acc_body_size (func->stmt);

  if (psz_stk)
    *psz_stk = Pin_stack_size;

  if (psz_body)
    *psz_body = Pin_body_size;

  if (psz_ebody)
    *psz_ebody = Pin_exec_body_size;

  return;
}


