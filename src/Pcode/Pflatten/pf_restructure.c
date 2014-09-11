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
 *	File:	flatten.c
 *	Author:	Ben-Chung Cheng and Wen-mei Hwu
 * 	Copyright (c) 1996 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.  
 * 	All rights granted to the University of Illinois.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/
/* 10/11/02 REK Pflatten accepts the following arguments.
 *              -Fdebug_flattening=(yes|no)
 *              -FDD_SPLIT_COMPOUND_EXPR_STMTS=(yes|no)
 *              -Ffast_mode=(yes|no)
 */

#include <config.h>
#include <library/i_error.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab_i.h>
#include <Pcode/query.h>
#include "flatten.h"
#include "data.h"

static int conj_disj_counter;
static int conj_disj_sequence;
static FuncDcl currentF = NULL;

static int PF_Break2Goto (Stmt stmts, Label dest);
static int PF_Continue2Goto (Stmt stmts, Label dest);
static Expr PF_FindFirstNonParen (Expr expr);
static void PF_RestructureIfStmt (Stmt stmts, int group);
static void PF_RestructureStmtExpr (Expr expr);
static void PF_RestructureStmtsInExpr (Expr expr);
static void PF_RestructureStmts (Stmt stmts);


/*! \brief Converts breaks to gotos.
 *
 * \param stmts
 *  the stmts to process.
 * \param dest
 *  the destination of the goto.
 *
 * \return
 *  If any breaks are converted to goto, returns 1.  Otherwise, returns 0.
 *
 * \sa PF_Continue2Goto()
 */
static int
PF_Break2Goto (Stmt stmts, Label dest)
{
  int changed = 0;

  while (stmts)
    {
      switch (P_GetStmtType (stmts))
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_EXPR:
	case ST_RETURN:
	case ST_SERLOOP:
	case ST_PARLOOP:
	case ST_ASM:
	  /* 
	   * BCC - 2/3/97
	   * Since the break in switch statements is not related to the 
	   * enclosing loops, it should not be changed into goto.
	   */
	case ST_SWITCH:
	  break;

	case ST_BREAK:
	  /* replaced by a goto */
	  P_SetStmtType (stmts, ST_GOTO);
	  if (P_GetLabelVal (dest))
	    P_SetStmtLabelVal (stmts, strdup (P_GetLabelVal (dest)));
	  P_SetStmtLabelKey (stmts, P_GetLabelKey (dest));
	  changed = 1;
	  break;

	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmts);

	    changed |= PF_Break2Goto (P_GetCompoundStmtList (c), dest);
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmts);

	    changed |= PF_Break2Goto (P_GetIfStmtThenBlock (i), dest);
	    changed |= PF_Break2Goto (P_GetIfStmtElseBlock (i), dest);
	  }
	  break;

	case ST_PSTMT:
	  {
	    Pstmt p = P_GetStmtPstmt (stmts);

	    changed |= PF_Break2Goto (P_GetPstmtStmt (p), dest);
	  }
	  break;

	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (stmts);

	    changed |= PF_Break2Goto (P_GetMutexStatement (m), dest);
	  }
	  break;

	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (stmts);

	    changed |= PF_Break2Goto (P_GetCobeginStatements (c), dest);
	  }
	  break;

	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmts);

	    changed |= PF_Break2Goto (P_GetBodyStmtStatement (b), dest);
	  }
	  break;

	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmts);

	    changed |= PF_Break2Goto (P_GetEpilogueStmtStatement (e), dest);
	  }
	  break;

	default:
	  P_punt ("flatten.c:PF_Break2Goto:%d Invalid statement type %d",
		  __LINE__, P_GetStmtType (stmts));
	}
      
      stmts = P_GetStmtLexNext (stmts);
    }

  return (changed);
}


/*! \brief Converts continues to gotos. 
 *
 * \param stmts
 *  the stmts to process.
 * \param dest
 *  the destination of the goto.
 *
 * \return
 *  If any continues are converted to goto, returns 1.  Otherwise, returns 0.
 *
 * \sa PF_Break2Goto()
 */
static int
PF_Continue2Goto (Stmt stmts, Label dest)
{
  int changed = 0;

  while (stmts)
    {
      switch (P_GetStmtType (stmts))
	{
	case ST_NOOP:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_EXPR:
	case ST_RETURN:
	case ST_SERLOOP:
	case ST_PARLOOP:
	case ST_ASM:
	  break;

	case ST_CONT:
	  /* replaced by a goto */
	  P_SetStmtType (stmts, ST_GOTO);
	  if (P_GetLabelVal (dest))
	    P_SetStmtLabelVal (stmts, strdup (P_GetLabelVal (dest)));
	  P_SetStmtLabelKey (stmts, P_GetLabelKey (dest));

	  changed = 1;
	  break;

	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmts);

	    changed |= PF_Continue2Goto (P_GetCompoundStmtList (c), dest);
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmts);

	    changed |= PF_Continue2Goto (P_GetIfStmtThenBlock (i), dest);
	    changed |= PF_Continue2Goto (P_GetIfStmtElseBlock (i), dest);
	  }
	  break;
	  
	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmts);

	    changed |= PF_Continue2Goto (P_GetSwitchStmtSwitchBody (s), dest);
	  }
	  break;

	case ST_PSTMT:
	  {
	    Pstmt p = P_GetStmtPstmt (stmts);
	    
	    changed |= PF_Continue2Goto (P_GetPstmtStmt (p), dest);
	  }
	  break;

	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (stmts);

	    changed |= PF_Continue2Goto (P_GetMutexStatement (m), dest);
	  }
	  break;

	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (stmts);

	    changed |= PF_Continue2Goto (P_GetCobeginStatements (c), dest);
	  }
	  break;

	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmts);

	    changed |= PF_Continue2Goto (P_GetBodyStmtStatement (b), dest);
	  }
	  break;

	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmts);

	    changed |= PF_Continue2Goto (P_GetEpilogueStmtStatement (e), dest);
	  }
	  break;

	default:
	  P_punt ("flatten.c:PF_Continue2Goto:%d Invalid statement type %d",
		  __LINE__, P_GetStmtType (stmts));
	}

      stmts = P_GetStmtLexNext (stmts);
    }

  return (changed);
}


static Expr
PF_FindFirstNonParen (Expr expr)
{
  while (expr->opcode == OP_compexpr)
    {
      if (expr->next)
	return 0;
      expr = expr->operands;
    }
  if (expr->next == NULL)
    return expr;
  else
    return 0;
}


static void
PF_RestructureIfStmt (Stmt stmts, int group)
{
  Expr cond_expr;

  if ((cond_expr = stmts->stmtstruct.ifstmt->cond_expr) &&
      (cond_expr = PF_FindFirstNonParen (cond_expr)))
    {
      Key scope_key = PSI_GetStmtScope (stmts);
      _Opcode opcode = cond_expr->opcode;
      Label then_lbl, else_lbl, out_label;
      Expr not_expr, op1, op2;
      Stmt newstmt;
      Stmt last_true;
      Stmt then_blk, else_blk;
      Stmt lex_next;
      Stmt if1, if2, st_goto;
      Pragma pragma;
      char pragma_string[512];
      char *func_name;

      then_blk = stmts->stmtstruct.ifstmt->then_block;
      else_blk = stmts->stmtstruct.ifstmt->else_block;

      switch (opcode)
	{
	case OP_conj:
	case OP_disj:

	  /* Reduce to a network of if stmts and gotos in the obvious way */
	  
	  if (then_blk->type != ST_COMPOUND)
	    {
	      PSI_StmtEncloseInCompound (then_blk);
	      then_blk = stmts->stmtstruct.ifstmt->then_block;
	    }

	  /* Ensure an else block exists */

	  if (!else_blk)
	    {
	      newstmt = P_NewStmtWithType (ST_NOOP);
	      P_SetStmtParentStmtAll (newstmt, stmts);
	      P_SetIfStmtElseBlock (P_GetStmtIfStmt (stmts), newstmt);
	      else_blk = newstmt;
	    }
	  
	  if (else_blk->type != ST_COMPOUND)
	    {
	      PSI_StmtEncloseInCompound (else_blk);
	      else_blk = stmts->stmtstruct.ifstmt->else_block;
	    }

	  if (!then_blk->stmtstruct.compound->stmt_list)
	    {
	      newstmt = P_NewStmtWithType (ST_NOOP);
	      then_blk->stmtstruct.compound->stmt_list = newstmt;
	      P_SetStmtParentStmtAll (then_blk->stmtstruct.compound->stmt_list,
				      then_blk);
	    }

	  last_true = then_blk->stmtstruct.compound->stmt_list;
	  while (last_true->lex_next)
	    last_true = last_true->lex_next;

	  if (!else_blk->stmtstruct.compound->stmt_list)
	    {
	      newstmt = P_NewStmtWithType (ST_NOOP);
	      else_blk->stmtstruct.compound->stmt_list = newstmt;
	      P_SetStmtParentStmtAll (else_blk->stmtstruct.compound->stmt_list,
				      else_blk);
	    }

	  stmts->type = ST_NOOP;
	  PSI_StmtInsertStmtAfter (stmts, then_blk);
	  PSI_StmtInsertStmtAfter (then_blk, else_blk);
	  
	  func_name = currentF->name;

	  then_lbl = PSI_NewLabelTemp (scope_key, "PF_THEN");
	  P_StmtAddLabel (then_blk->stmtstruct.compound->stmt_list, then_lbl);
	  else_lbl = PSI_NewLabelTemp (scope_key, "PF_ELSE");
	  P_StmtAddLabel (else_blk->stmtstruct.compound->stmt_list, else_lbl);

	  /* BCC - 12/01/96
	   * if the last statement in the true part is an unconditional jump,
	   * there is no need to insert the goto statement thereafter.
	   */

	  if (PF_HasFallThrough (last_true))
	    {
	      out_label = PSI_NewLabelTemp (scope_key, "PF_OUT");

	      if (!(lex_next = else_blk->lex_next))
		{
		  lex_next = P_NewStmtWithType (ST_NOOP);
		  PSI_StmtInsertStmtAfter (else_blk, lex_next);
		}

	      /* JWS ??? */

	      /* lex_next now is the next non-compound statement */
	      while (lex_next->type == ST_COMPOUND &&
		     lex_next->stmtstruct.compound->stmt_list)
		lex_next = lex_next->stmtstruct.compound->stmt_list;
	      /* empty compound statement found */
	      if (lex_next->type == ST_COMPOUND)
		lex_next->type = ST_NOOP;

	      P_StmtAddLabel (lex_next, out_label);
	      
	      PSI_StmtInsertStmtAfter (last_true, P_NewGotoStmt (out_label));
	    }
	  
	  op1 = PSI_CopyExpr (cond_expr->operands);
	  op2 = PSI_CopyExpr (cond_expr->operands->sibling);

	  stmts->stmtstruct.ifstmt->cond_expr = \
	    PSI_RemoveExpr (stmts->stmtstruct.ifstmt->cond_expr);
	  stmts->stmtstruct.ifstmt->cond_expr = NULL;
	  stmts->stmtstruct.ifstmt->then_block = NULL;
	  stmts->stmtstruct.ifstmt->else_block = NULL;
	  stmts->stmtstruct.ifstmt = \
	    PSI_RemoveIfStmt (stmts->stmtstruct.ifstmt);
	  
	  if1 = P_NewStmtWithType (ST_IF);
	  if1->stmtstruct.ifstmt = P_NewIfStmt ();
	  P_SetIfStmtCondExpr (if1->stmtstruct.ifstmt, op1);
	  PF_CopyLineInfo (stmts, if1);
	  
	  if2 = P_NewStmtWithType (ST_IF);
	  if2->stmtstruct.ifstmt = P_NewIfStmt ();
	  P_SetIfStmtCondExpr (if2->stmtstruct.ifstmt, op2);
	  PF_CopyLineInfo (stmts, if2);
	  
	  if ((pragma = P_FindPragma (stmts->pragma, "EMPTYLOOP")))
	    {
	      if1->pragma = P_AppendPragmaNext (if1->pragma,
						P_CopyPragma (pragma));
	      if2->pragma = P_AppendPragmaNext (if2->pragma,
						P_CopyPragma (pragma));
	    }
	  
	  if ((pragma = P_FindPragma (stmts->pragma, "CONJDISJ")))
	    {
	      sprintf (pragma_string, "c%d%s",
		       conj_disj_sequence++, pragma->expr->value.string + 1);
	      if1->pragma = 
		P_AppendPragmaNext (if1->pragma,
				    P_NewPragmaWithSpecExpr 
				    ("CONJDISJ",
				     P_NewStringExpr (pragma_string)));
	      sprintf (pragma_string, "c%d%s",
		       conj_disj_sequence++, pragma->expr->value.string + 1);
	      if2->pragma = 
		P_AppendPragmaNext (if2->pragma,
				    P_NewPragmaWithSpecExpr 
				    ("CONJDISJ",
				     P_NewStringExpr (pragma_string)));
	      P_RemoveStmtPragma (stmts, pragma);
	    }
	  else
	    {
	      sprintf (pragma_string, "c%d_%d",
		       conj_disj_sequence++, group);
	      if1->pragma = 
		P_AppendPragmaNext (if1->pragma,
				    P_NewPragmaWithSpecExpr 
				    ("CONJDISJ",
				     P_NewStringExpr (pragma_string)));
	      sprintf (pragma_string, "c%d_%d",
		       conj_disj_sequence++, group);
	      if2->pragma = 
		P_AppendPragmaNext (if2->pragma,
				    P_NewPragmaWithSpecExpr 
				    ("CONJDISJ",
				     P_NewStringExpr (pragma_string)));	    
	    }
	  
	  if ((pragma = P_FindPragma (stmts->pragma, "IFELSE")))
	    {
	      if1->pragma = P_AppendPragmaNext (if1->pragma,
						P_CopyPragma (pragma));
	      if2->pragma = P_AppendPragmaNext (if2->pragma,
						P_CopyPragma (pragma));
	    }
	  
	  if (opcode == OP_conj)
	    {
	      st_goto = P_NewGotoStmt (else_lbl);
	      P_SetIfStmtElseBlock (P_GetStmtIfStmt (if1), st_goto);
	      P_SetIfStmtThenBlock (P_GetStmtIfStmt (if1), if2);
	    }
	  else if (opcode == OP_disj)
	    {
	      st_goto = P_NewGotoStmt (then_lbl);
	      P_SetIfStmtThenBlock (P_GetStmtIfStmt (if1), st_goto);
	      P_SetIfStmtElseBlock (P_GetStmtIfStmt (if1), if2);
	    }
	  else
	    {
	      I_punt("Unexpected case");
	    }

	  P_SetIfStmtParentStmtAll (P_GetStmtIfStmt (if1), if1);

	  st_goto = P_NewGotoStmt (then_lbl);
	  P_SetIfStmtThenBlock (P_GetStmtIfStmt (if2), st_goto);

	  st_goto = P_NewGotoStmt (else_lbl);
	  P_SetIfStmtElseBlock (P_GetStmtIfStmt (if2), st_goto);
 	  
	  P_SetIfStmtParentStmtAll (P_GetStmtIfStmt (if2), if2);

	  P_SetStmtParentStmtAll (if1, stmts->parent);
	  PSI_StmtInsertStmtAfter (stmts, if1);
	  
	  PF_RestructureIfStmt (if2, group);
	  PF_RestructureIfStmt (if1, group);
	  return;	  

	case OP_not:

	  /* Try to perform some DeMorgan reductions */

	  not_expr = cond_expr;
	  if ((cond_expr = PF_FindFirstNonParen (not_expr->operands)))
	    {
	      opcode = cond_expr->opcode;

	      switch (opcode)
		{
		case OP_not:
		  op1 = PSI_CopyExpr (cond_expr->operands);
		  stmts->stmtstruct.ifstmt->cond_expr = \
		    PSI_RemoveExpr (stmts->stmtstruct.ifstmt->cond_expr);
		  P_SetIfStmtCondExpr (stmts->stmtstruct.ifstmt, op1);
		  PF_RestructureIfStmt (stmts, group);
		  return;

		case OP_conj:
		  op1 = P_NewExprWithOpcode (OP_not);
		  P_AppendExprOperands (op1,
					PSI_CopyExpr (cond_expr->operands));
		  PSI_CastExpr (op1);
		  op2 = P_NewExprWithOpcode (OP_not);
		  P_AppendExprOperands \
		    (op2, PSI_CopyExpr (cond_expr->operands->sibling));
		  PSI_CastExpr (op2);

		  cond_expr = P_NewExprWithOpcode (OP_disj);
		  P_AppendExprOperands (cond_expr, op1);
		  P_AppendExprOperands (cond_expr, op2);
		  PSI_CastExpr (cond_expr);

		  stmts->stmtstruct.ifstmt->cond_expr = \
		    PSI_RemoveExpr (stmts->stmtstruct.ifstmt->cond_expr);
		  P_SetIfStmtCondExpr (stmts->stmtstruct.ifstmt, cond_expr);
		  PF_RestructureIfStmt (stmts, group);
		  return;

		case OP_disj:
		  op1 = P_NewExprWithOpcode (OP_not);
		  P_AppendExprOperands (op1,
					PSI_CopyExpr (cond_expr->operands));
		  PSI_CastExpr (op1);

		  op2 = P_NewExprWithOpcode (OP_not);
		  P_AppendExprOperands \
		    (op2, PSI_CopyExpr (cond_expr->operands->sibling));
		  PSI_CastExpr (op2);

		  cond_expr = P_NewExprWithOpcode (OP_conj);
		  P_AppendExprOperands (cond_expr, op1);
		  P_AppendExprOperands (cond_expr, op2);
		  PSI_CastExpr (cond_expr);

		  stmts->stmtstruct.ifstmt->cond_expr = \
		    PSI_RemoveExpr (stmts->stmtstruct.ifstmt->cond_expr);
		  P_SetIfStmtCondExpr (stmts->stmtstruct.ifstmt, cond_expr);
		  PF_RestructureIfStmt (stmts, group);
		  return;

		default:
		  break;
		}
	    }
	default:
	  break;
	}
    }

  PF_RestructureStmts (stmts->stmtstruct.ifstmt->then_block);
  if (stmts->stmtstruct.ifstmt->else_block)
    PF_RestructureStmts (stmts->stmtstruct.ifstmt->else_block);
  return;
}


/*! \brief Restructures a statement expression.
 *
 * \param expr
 *  the Expr to restructure.
 *
 * EMN-03 : Added to handle stmt_exprs.
 */
static void
PF_RestructureStmtExpr (Expr expr)
{
  Stmt result_stmt = NULL;
  Expr result_expr = NULL;
  Expr new = NULL;
  Expr assign = NULL;
  Key result_type_key;
  TypeDcl result_type;
  Key scope_key;

  scope_key = PSI_GetExprScope (expr);

  /* EMN-03: This creates the temp var and assignment for
   *   flattening stmt exprs. It appears that gcc converts
   *   ({...; val++}) into ({...; temp=val++;}) meaning that
   *   the value of the stmt_expr is before the increment
   */

  /* If the value of the stmt_expr is void then
     don't add an assignment */
  result_type_key = PSI_ExprType (expr);
  result_type = PSI_GetTypeDclEntry (result_type_key);

  if (P_GetTypeDclBasicType (result_type) & BT_VOID)
    return;

  if (P_GetStmtType (expr->value.stmt) != ST_COMPOUND)
    P_punt ("flatten.c:PF_RestructureStmtExpr:%d Statement expression must be "
	    "a compound", __LINE__);

  /* Find STMTEXPR result */
  for (result_stmt = \
	 P_GetCompoundStmtList (P_GetStmtCompound (P_GetExprStmt (expr)));
       P_GetStmtLexNext (result_stmt);
       result_stmt = P_GetStmtLexNext (result_stmt));

  if (P_GetStmtType (result_stmt) != ST_EXPR)
    return;

  result_expr = P_GetStmtExpr (result_stmt);

  /* Generate temp var */
  assert (P_ExprParentStmt (expr));

  new = PF_NewLocalVar (P_ExprParentStmt (expr), PSI_ExprType (expr));
	    
  /* Add assignment to end of STMTEXPR block */
  assign = P_NewExprWithOpcode (OP_assign);
  P_AppendExprOperands (assign, new);
  P_AppendExprOperands (assign, result_expr);

  PSI_CastExpr (assign);

  P_SetStmtExpr (result_stmt, assign);
  P_SetExprParentStmtAll (assign, result_stmt);

  P_ClrExprFlags (result_expr, EF_RETAIN);
  P_SetExprFlags (assign, EF_RETAIN);

  return;
}


/*! \brief Searches an Expr for statement expressions to restructure.
 *
 * \param expr
 *  the Expr to search.
 *
 * EMN-03 : Added to handle stmt_exprs.
 */
static void
PF_RestructureStmtsInExpr (Expr expr)
{
  Expr ptr;

  while (expr)
    {
      switch (P_GetExprOpcode (expr))
	{
	case OP_stmt_expr:
	  PF_RestructureStmts (P_GetExprStmt (expr));
	  PF_RestructureStmtExpr (expr);
	  break;

	default:
	  /* Do nothing */
	  break;
	}
      
      /* Recurse through rest of expr tree */
      for (ptr = P_GetExprOperands (expr); ptr; ptr = P_GetExprSibling (ptr))
	PF_RestructureStmtsInExpr (ptr);

      expr = P_GetExprNext (expr);
    }

  return;
}


/*!
 * \param stmts
 *  the stmts to process.
 */
static void
PF_RestructureStmts (Stmt stmts)
{
  while (stmts)
    {
      switch (P_GetStmtType (stmts))
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	  break;

	case ST_ASM:
	  {
	    AsmStmt a = P_GetStmtAsmStmt (stmts);

	    PF_RestructureStmtsInExpr (P_GetAsmStmtAsmString (a));
	    PF_RestructureStmtsInExpr (P_GetAsmStmtAsmOperands (a));
	  }
	  break;

	case ST_RETURN:
	  PF_RestructureStmtsInExpr (P_GetStmtRet (stmts));
	  break;

	case ST_EXPR:
	  PF_RestructureStmtsInExpr (P_GetStmtExpr (stmts));
	  break;

	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmts);

	    PF_RestructureStmts (P_GetCompoundStmtList (c));
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmts);

	    PF_RestructureStmtsInExpr (P_GetIfStmtCondExpr (i));
	    PF_RestructureIfStmt (stmts, conj_disj_counter++);
	  }
	  break;

	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmts);

	    PF_RestructureStmtsInExpr (P_GetSwitchStmtExpression (s));
	    PF_RestructureStmts (P_GetSwitchStmtSwitchBody (s));
	  }
	  break;

	case ST_PSTMT:
	  {
	    Pstmt p = P_GetStmtPstmt (stmts);

	    PF_RestructureStmts (P_GetPstmtStmt (p));
	  }
	  break;

	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (stmts);

	    PF_RestructureStmts (P_GetMutexStatement (m));
	  }
	  break;

	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (stmts);

	    PF_RestructureStmts (P_GetCobeginStatements (c));
	  }
	  break;

/******************************************************************************
 * RestructureStmts a FOR loop:
 * Old loop : 
 * 	for ( i = foo1(); i < foo2(); i+= foo3()) {
 *		foo4();
 *		if (i == 5) continue;
 *              if (i == 6) break;
 *	}
 *
 * New loop : 
 *	t1 = foo1();
 * 	{
 *		if (i < foo2()) {
 *      FOR1:       	foo4();
 *			if (i == 5) goto CONTINUE_1;
 *			if (i == 6) goto EXIT_1;
 *	CONTINUE_1 : 
 *			i += foo3();
 *			if (i < foo2()) goto FOR1;
 *              }
 *	}
 *      EXIT_1:
 * 
 * RestructureStmts a WHILE loop:
 * Old loop : 
 * 	while (i < foo2()) {
 *		foo4();
 *		if (i == 5) continue;
 *              if (i == 6) break;
 *	}
 *
 * New loop : 
 * 	{
 *		if (i < foo2()) {
 *	WHILE1:		foo4();
 *			if (i == 5) goto CONTINUE_1;
 *			if (i == 6) goto EXIT_1;
 *      CONTINUE_1:
 *			if (i < foo2()) goto WHILE1;
 *		}
 *	}
 *      EXIT_1:
 * 
 * RestructureStmts a DO loop:
 * Old loop : 
 * 	do {
 *		foo4();
 *		if (i == 5) continue;
 *		if (i == 6) break;;
 *	} while ( i < foo2() );
 *
 * New loop : 
 * 	{
 *	DO1:	foo4();
 *		if (i == 5) goto CONTINUE_1;
 *		if (i == 6) goto EXIT_1;
 *	CONTINUE_1 :
 *		if (i < foo2()) goto DO1;
 *	} 
 *      EXIT_1:
 * 
 *****************************************************************************/

	case ST_SERLOOP:
	  {
	    SerLoop s = P_GetStmtSerLoop (stmts);
	    Expr init_expr = P_GetSerLoopInitExpr (s);
	    Expr cond_expr = P_GetSerLoopCondExpr (s);
	    Expr iter_expr = P_GetSerLoopIterExpr (s);
	    Stmt loop_body = P_GetSerLoopLoopBody (s);
	    Stmt comp_stmt, new_stmt;
	    IfStmt new_if;
	    /* outer_scope_key is the original scope.  inner_scope_key is
	     * the scope inside the compound stmt we'll create. */
	    Key outer_scope_key, inner_scope_key;
	    Label cont_label, head_label, exit_label;
	    int cont_changed = 0, break_changed = 0;
	    int empty_loop = 0;

	    PF_RestructureStmtsInExpr (init_expr);
	    PF_RestructureStmtsInExpr (cond_expr);
	    PF_RestructureStmtsInExpr (iter_expr);

	    switch (P_GetSerLoopLoopType (s))
	      {
	      case LT_FOR:
		if ((init_expr && \
		     (PF_get_expr_flags (init_expr) & PF_CONTAIN_BREAK)) || \
		    (cond_expr && \
		     (PF_get_expr_flags (cond_expr) & PF_CONTAIN_BREAK)) || \
		    (iter_expr && \
		     (PF_get_expr_flags (iter_expr) & PF_CONTAIN_BREAK)))
		  {
		    if (PF_debug)
		      fprintf (Flog,
			       "Restructuring a FOR loop near line %d\n",
			       P_GetStmtLineno (stmts));

		    if (init_expr)
		      {
			PSI_StmtInsertExprBefore (stmts, init_expr);
			PF_CopyLineInfo (stmts, P_GetStmtLexPrev (stmts));
		      }

		    if (!loop_body)
		      loop_body = P_NewStmtWithType (ST_NOOP);

		    if (P_GetStmtType (loop_body) == ST_NOOP)
		      empty_loop = 1;

		    outer_scope_key = PSI_GetStmtScope (stmts);

		    /* Set up a new compound stmt with a scope. */
		    comp_stmt = PSI_NewCompoundStmt (outer_scope_key);

		    inner_scope_key = P_GetStmtKey (comp_stmt);

		    PSI_StmtInsertStmtBefore (stmts, comp_stmt);
		    PF_CopyLineInfo (stmts, comp_stmt);

		    new_stmt = P_NewStmtWithType (ST_IF);
		    PF_CopyLineInfo (comp_stmt, new_stmt);

		    /* BCC - add pragmas for empty loop - 4/14/97 */
		    if (empty_loop)
		      PF_stmt_emptyloop (new_stmt) = 1;
		    
		    PF_stmt_loop_type (new_stmt) = LT_FOR;
		    PF_stmt_loop_line (new_stmt) = P_GetStmtLineno (stmts);

		    new_if = P_NewIfStmt ();
		    P_SetStmtIfStmt (new_stmt, new_if);
		    if (cond_expr)
		      P_SetIfStmtCondExpr \
			(new_if, PSI_CopyExprListToScope (inner_scope_key,
							  cond_expr));
		    else
		      P_SetIfStmtCondExpr \
			(new_if, PSI_ScopeNewIntExpr (inner_scope_key, 1));
		    P_SetIfStmtThenBlock (new_if, loop_body);
		    P_SetExprParentStmtAll (P_GetIfStmtCondExpr (new_if),
					 new_stmt);
		    P_SetStmtParentStmtAll (loop_body, new_stmt);

		    P_SetCompoundStmtList (P_GetStmtCompound (comp_stmt),
					   new_stmt);
		    P_SetStmtParentStmtAll (new_stmt, comp_stmt);

		    cont_label = PSI_NewLabelTemp (inner_scope_key,
						   "PF_CONTINUE");
		    head_label = PSI_NewLabelTemp (inner_scope_key, "PF_FOR");
		    exit_label = PSI_NewLabelTemp (inner_scope_key, "PF_EXIT");

		    cont_changed = PF_Continue2Goto (loop_body, cont_label);
		    break_changed = PF_Break2Goto (loop_body, exit_label);

		    /* Attatch the exit_label to the next statement */
		    if (break_changed)
		      P_StmtAddLabelAfter (comp_stmt, exit_label);
		    else
		      exit_label = PSI_RemoveLabel (exit_label);

		    if (cond_expr)
		      {
			new_stmt = P_NewStmtWithType (ST_IF);
			
			if (empty_loop)
			  PF_stmt_emptyloop (new_stmt) = 1;

			new_if = P_NewIfStmt ();

			P_SetStmtIfStmt (new_stmt, new_if);

			P_SetIfStmtCondExpr (new_if, cond_expr);
			P_SetIfStmtThenBlock (new_if,
					      P_NewGotoStmt (head_label));

			P_SetExprParentStmtAll (cond_expr, new_stmt);
			P_SetStmtParentStmtAll (P_GetIfStmtThenBlock (new_if),
						new_stmt);
		      }
		    else
		      {
			new_stmt = P_NewGotoStmt (head_label);
		      }

		    /* Add a label to the loopback if the loop
		     * included a continue. */
		    PSI_StmtInsertStmtAfter (loop_body, new_stmt);
		    PF_CopyLineInfo (loop_body, new_stmt);

		    if (cont_changed)
		      P_StmtAddLabel (new_stmt, cont_label);
		    else
		      cont_label = PSI_RemoveLabel (cont_label);

		    P_StmtAddLabel (loop_body, head_label);

		    if (iter_expr)
		      {
			PSI_StmtInsertExprBefore (new_stmt, iter_expr);
			PF_CopyLineInfo (new_stmt, iter_expr->parentstmt);
		      }

		    PF_RestructureStmts \
		      (P_GetCompoundStmtList (P_GetStmtCompound (comp_stmt)));

		    P_SetSerLoopLoopBody (s, NULL);
		    P_SetSerLoopCondExpr (s, NULL);
		    P_SetSerLoopInitExpr (s, NULL);
		    P_SetSerLoopIterExpr (s, NULL);

		    P_StmtRemoveStmt (stmts);
		    stmts = PSI_RemoveStmt (stmts);

		    stmts = comp_stmt;
		  }
		else
		  {
		    PF_RestructureStmts (P_GetSerLoopLoopBody (s));
		  }
		break;

	      case LT_WHILE:
		if (cond_expr && \
		    (PF_get_expr_flags (cond_expr) & PF_CONTAIN_BREAK))
		  {
		    if (PF_debug)
		      fprintf (Flog,
			       "Restructuring a WHILE loop near line %d\n",
			       P_GetStmtLineno (stmts));

		    if (!loop_body)
		      loop_body = P_NewStmtWithType (ST_NOOP);

		    if (P_GetStmtType (loop_body) == ST_NOOP)
		      empty_loop = 1;

		    outer_scope_key = PSI_GetStmtScope (stmts);

		    /* Set up a new compound stmt with a scope. */
		    comp_stmt = PSI_NewCompoundStmt (outer_scope_key);

		    inner_scope_key = P_GetStmtKey (comp_stmt);

		    PSI_StmtInsertStmtBefore (stmts, comp_stmt);
		    PF_CopyLineInfo (stmts, comp_stmt);

		    new_stmt = P_NewStmtWithType (ST_IF);
		    PF_CopyLineInfo (comp_stmt, new_stmt);

		    /* BCC - add pragmas for empty loop - 4/14/97 */
		    if (empty_loop)
		      PF_stmt_emptyloop (new_stmt) = 1;

		    PF_stmt_loop_type (new_stmt) = LT_WHILE;
		    PF_stmt_loop_line (new_stmt) = P_GetStmtLineno (stmts);

		    new_if = P_NewIfStmt ();
		    P_SetStmtIfStmt (new_stmt, new_if);
		    P_SetIfStmtCondExpr \
		      (new_if, PSI_CopyExprListToScope (inner_scope_key,
							cond_expr));
		    P_SetIfStmtThenBlock (new_if, loop_body);
		    P_SetExprParentStmtAll (P_GetIfStmtCondExpr (new_if),
					    new_stmt);
		    P_SetStmtParentStmtAll (loop_body, new_stmt);

		    P_SetCompoundStmtList (P_GetStmtCompound (comp_stmt),
					   new_stmt);
		    P_SetStmtParentStmtAll (new_stmt, comp_stmt);

		    cont_label = PSI_NewLabelTemp (inner_scope_key,
						   "PF_CONTINUE");
		    head_label = PSI_NewLabelTemp (inner_scope_key,
						   "PF_WHILE");
		    exit_label = PSI_NewLabelTemp (inner_scope_key, "PF_EXIT");

		    cont_changed = PF_Continue2Goto (loop_body, cont_label);
		    break_changed = PF_Break2Goto (loop_body, exit_label);

		    /* Attatch the exit_label to the next statement */
		    if (break_changed)
		      P_StmtAddLabelAfter (comp_stmt, exit_label);
		    else
		      exit_label = PSI_RemoveLabel (exit_label);

		    new_stmt = P_NewStmtWithType (ST_IF);

		    if (empty_loop)
		      PF_stmt_emptyloop (new_stmt) = 1;

		    new_if = P_NewIfStmt ();

		    P_SetStmtIfStmt (new_stmt, new_if);

		    P_SetIfStmtCondExpr (new_if, cond_expr);
		    P_SetIfStmtThenBlock (new_if, 
					  P_NewGotoStmt (head_label));

		    P_SetExprParentStmtAll (cond_expr, new_stmt);
		    P_SetStmtParentStmtAll (P_GetIfStmtThenBlock (new_if),
					    new_stmt);

		    /* Add a label to the loopback if the loop
		     * included a continue. */
		    PSI_StmtInsertStmtAfter (loop_body, new_stmt);
		    PF_CopyLineInfo (loop_body, new_stmt);

		    if (cont_changed)
		      P_StmtAddLabel (new_stmt, cont_label);
		    else
		      cont_label = PSI_RemoveLabel (cont_label);

		    P_StmtAddLabel (loop_body, head_label);

		    PF_RestructureStmts \
		      (P_GetCompoundStmtList (P_GetStmtCompound (comp_stmt)));

		    P_SetSerLoopLoopBody (s, NULL);
		    P_SetSerLoopCondExpr (s, NULL);
		    P_SetSerLoopInitExpr (s, NULL);
		    P_SetSerLoopIterExpr (s, NULL);

		    P_StmtRemoveStmt (stmts);
		    stmts = PSI_RemoveStmt (stmts);

		    stmts = comp_stmt;
		  }
		else
		  {
		    PF_RestructureStmts (P_GetSerLoopLoopBody (s));
		  }
		break;

	      case LT_DO:
		if (cond_expr && \
		    (PF_get_expr_flags (cond_expr) & PF_CONTAIN_BREAK))
		  {
		    if (PF_debug)
		      {
			fprintf (Flog,
				 "Restructuring a DO loop near line %d\n",
				 P_GetStmtLineno (stmts));
		      }

		    if (!loop_body)
		      loop_body = P_NewStmtWithType (ST_NOOP);

		    if (P_GetStmtType (loop_body) == ST_NOOP)
		      empty_loop = 1;

		    outer_scope_key = PSI_GetStmtScope (stmts);

		    /* Set up a new compound stmt with a scope. */
		    comp_stmt = PSI_NewCompoundStmt (outer_scope_key);

		    inner_scope_key = P_GetStmtKey (comp_stmt);

		    PSI_StmtInsertStmtBefore (stmts, comp_stmt);
		    PF_CopyLineInfo (stmts, comp_stmt);

		    PF_stmt_loop_type (loop_body) = LT_DO;
		    PF_stmt_loop_type (loop_body) = P_GetStmtLineno (stmts);

		    P_SetCompoundStmtList (P_GetStmtCompound (comp_stmt),
					   loop_body);
		    P_SetStmtParentStmtAll (loop_body, comp_stmt);

		    /* replace every continue in the for loop with goto */

		    cont_label = PSI_NewLabelTemp (inner_scope_key,
						   "PF_CONTINUE");
		    head_label = PSI_NewLabelTemp (inner_scope_key, "PF_DO");
		    exit_label = PSI_NewLabelTemp (inner_scope_key, "PF_EXIT");

		    cont_changed = PF_Continue2Goto (loop_body, cont_label);
		    break_changed = PF_Break2Goto (loop_body, exit_label);

		    /* Attatch the exit_label to the next statement */
		    if (break_changed)
		      P_StmtAddLabelAfter (comp_stmt, exit_label);
		    else
		      exit_label = PSI_RemoveLabel (exit_label);

		    /* Create a new stmt which is of type ST_IF */
		    new_stmt = P_NewStmtWithType (ST_IF);
		    PF_CopyLineInfo (comp_stmt, new_stmt);

		    /* BCC - add pragmas for empty loop - 4/14/97 */
		    if (empty_loop)
		      PF_stmt_emptyloop (new_stmt) = 1;

		    if (cont_changed)
		      P_StmtAddLabel (new_stmt, cont_label);
		    else
		      cont_label = PSI_RemoveLabel (cont_label);

		    PSI_StmtInsertStmtAfter (loop_body, new_stmt);

		    /* Create the new IfStmt and connect it to
		     * new_stmt. */
		    new_if = P_NewIfStmt ();
		    P_SetStmtIfStmt (new_stmt, new_if);
		    
		    /* Create the cond_expr */
		    P_SetIfStmtCondExpr (new_if, cond_expr);
		    P_SetIfStmtThenBlock (new_if, P_NewGotoStmt (head_label));
		    P_SetExprParentStmtAll (cond_expr, new_stmt);
		    P_SetStmtParentStmtAll (P_GetIfStmtThenBlock (new_if),
					    new_stmt);

		    P_StmtAddLabel (loop_body, head_label);

		    PF_RestructureStmts \
		      (P_GetCompoundStmtList (P_GetStmtCompound (comp_stmt)));

		    P_SetSerLoopLoopBody (s, NULL);
		    P_SetSerLoopCondExpr (s, NULL);
		    P_SetSerLoopInitExpr (s, NULL);
		    P_SetSerLoopIterExpr (s, NULL);

		    P_StmtRemoveStmt (stmts);
		    stmts = PSI_RemoveStmt (stmts);

		    stmts = comp_stmt;
		  }
		else
		  {
		    PF_RestructureStmts (P_GetSerLoopLoopBody (s));
		  }
		break;
	      }
	    break;

	  }
	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (stmts);
	    Pstmt pstmt = P_GetParLoopPstmt (p);

	    PF_RestructureStmtsInExpr (P_GetParLoopIterationVar (p));
	    PF_RestructureStmtsInExpr (P_GetParLoopInitValue (p));
	    PF_RestructureStmtsInExpr (P_GetParLoopFinalValue (p));
	    PF_RestructureStmtsInExpr (P_GetParLoopIncrValue (p));

	    PF_RestructureStmts (P_GetPstmtStmt (pstmt));
	  }
	  break;

	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmts);

	    PF_RestructureStmts (P_GetBodyStmtStatement (b));
	  }
	  break;
	  
	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmts);
	    
	    PF_RestructureStmts (P_GetEpilogueStmtStatement (e));
	  }
	  break;

	default:
	  P_punt ("flatten.c:PF_RestructureStmts:%d Invalid statement type %d",
		  __LINE__, P_GetStmtType (stmts));
	}

      if (stmts)
	stmts = P_GetStmtLexNext (stmts);
    }

  return;
}


void
PF_Restructure (FuncDcl func)
{
  currentF = func;
  PF_RestructureStmts (func->stmt);
  return;
}
