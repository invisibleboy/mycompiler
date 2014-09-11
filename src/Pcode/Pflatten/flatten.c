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
 *              -Fflatten_reduce=(yes|no)
 *              -FDD_SPLIT_COMPOUND_EXPR_STMTS=(yes|no)
 *              -Ffast_mode=(yes|no)
 */

#include <config.h>
#include <library/i_error.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/query.h>
#include <Pcode/query_symtab.h>
#include "flatten.h"
#include "data.h"

int PF_debug = FALSE;
int PF_reduce = TRUE;

static Expr PF_EnsureListCompound (Expr expr);
static void PF_MarkCompExpr (Expr expr, int flag);
static void PF_VisitExprNode (Expr expr);
static void PF_ResetExprFlags (Expr expr);
static void PF_VisitStmts (Stmt stmts);
static Expr PF_GenQuest (Stmt base_stmt, Expr expr);
static Expr PF_GenConjDisj (Stmt base_stmt, Expr expr);
static void PF_ResetBreakNode (Expr expr);
static Expr PF_BreakAOP (Stmt base_stmt, Expr expr);
static Expr PF_BreakPostOP (Stmt base_stmt, Expr expr);
static Expr PF_BreakPreOP (Stmt base_stmt, Expr expr);
static Expr PF_GenerateExpr (Stmt base_stmt, Expr expr);
static int PF_UselessExpr (Expr expr);
static int PF_UselessExprNode (Expr expr);
static Expr PF_CompressExprList (Expr expr, int last);
static void PF_FlattenStmts (Stmt stmts);
static void PF_BreakAopInForStmts_expr (Expr expr);
static void PF_BreakAopInForStmts (Stmt stmts);
static void PF_RemoveUselessStmts (Stmt stmts);


/*! \brief Creates a new OP_compexpr to hold an expression list.
 *
 * \param expr
 *  the expression list.
 *
 * \return
 *  If \a expr is a list, returns an OP_compexpr Expr containing \a expr.
 *  Otherwise, returns \a expr.
 */
static Expr
PF_EnsureListCompound (Expr expr)
{
  Expr result;

  if (P_GetExprNext (expr) == NULL)
    return (expr);
  
  result = PSI_ScopeNewExprWithOpcode (PSI_GetExprScope (expr), OP_compexpr);
  P_AppendExprOperands (result, expr);
  PSI_CastExpr (result);

  return (result);
}


/*! \brief Propagates the flag into a compound expression.
 *
 * \param expr
 *  the compound expr.
 * \param flag
 *  the flag to set in the compound.
 *
 * (((a, b, c))) , propagate the flag from the enclosing parenthesis into a,b,c
 */
static void
PF_MarkCompExpr (Expr expr, int flag)
{
  if (expr && P_GetExprOpcode (expr) == OP_compexpr)
    {
      Expr op;

      for (op = P_GetExprOperands (expr); op; op = P_GetExprNext (op))
	{
	  PF_set_expr_flags (op, flag);
	  if (P_GetExprOpcode (op) == OP_compexpr)
	    PF_MarkCompExpr (op, flag);
	}
    }

  return;
}


/*! \brief Visit an expression tree, marking what needs to be broken.
 *
 * \param expr
 *  the head of the tree to visit.
 *
 * Visit an expression tree in the execution order. Use the PF_expr_data
 * structure of expressions to remember the returned location.
 */
static void
PF_VisitExprNode (Expr expr)
{
  int opcode;
  Expr op, op1, op2, opnext, parent_expr;

  if (!expr)
    return;

  /* I. Check locally for side effects and need to break */

  switch ((opcode = P_GetExprOpcode (expr)))
    {
    case OP_assign:
      /* Assignments have side-effects, but only inherently require
       * flattening if they are not at the top level.
       */
      PF_set_expr_flags (expr, PF_SIDE_EFFECT);

      for (parent_expr = P_GetExprParentExpr (expr); 
	   parent_expr && (P_GetExprOpcode (parent_expr) == OP_compexpr);
	   parent_expr = P_GetExprParentExpr (parent_expr));
      
      if (parent_expr ||
	  (expr->parentstmt && 
	   (P_GetStmtType (expr->parentstmt) == ST_RETURN)))
	PF_set_expr_flags (expr, PF_BREAK | PF_CONTAIN_BREAK);
      break;

    case OP_quest:
    case OP_conj:
    case OP_disj:
      /* Operators "?:", "&&" and "||" must always be flattened. */
      PF_set_expr_flags (expr, PF_BREAK | PF_CONTAIN_BREAK);
      break;

    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Arshft:
    case OP_Alshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:
      /* Pre/post incr. and assign-ops have side-effects and are
       * always flattened.
       */
      PF_set_expr_flags (expr, PF_SIDE_EFFECT | PF_BREAK | PF_CONTAIN_BREAK);
      break;

    case OP_stmt_expr:
      /* Stmt exprs are assumed to have side-effects and are always
       * flattened.  Recur on the enclosed statement.
       */
      PF_set_expr_flags (expr, PF_SIDE_EFFECT | PF_BREAK | PF_CONTAIN_BREAK);
      PF_VisitStmts (expr->value.stmt);
      break;

    case OP_call:
      /* Except for most builtins, which are ignored, calls are
       * assumed to have side-effects and have their parameters marked
       * with PF_ARGUMENT.  The call expr or its outermost encolsing
       * cast, if present, is marked for flattening.
       */
      {
	Expr mark_expr, callee = P_GetExprOperands (expr);

	if (P_GetExprOpcode (callee) == OP_var)
	  {
	    char *callee_name = P_GetExprVarName (callee);

	    /* Break only specific builtins */

	    if (callee_name && !strncmp (callee_name, "__builtin_", 10) &&
		strcmp (callee_name, "__builtin_alloca") &&
		strcmp (callee_name, "__builtin_saveregs") &&
		strcmp (callee_name, "__builtin_next_arg"))
	      return;
	  }

	PF_set_expr_flags (expr, PF_SIDE_EFFECT);

	for (op2 = P_GetExprSibling (callee); op2; op2 = P_GetExprNext (op2))
	  PF_set_expr_flags (op2, PF_ARGUMENT);

	/* Mark expr or its outermost enclosing cast for break */

	for (mark_expr = expr;
	     (parent_expr = P_GetExprParentExpr (mark_expr)) &&
	       (P_GetExprOpcode (parent_expr) == OP_cast);
	     mark_expr = parent_expr);

	PF_set_expr_flags (mark_expr, PF_BREAK | PF_CONTAIN_BREAK);
      }
      break;

    default:
      break;
    }
      
  /* II. Recur on operands and next-exprs */

  for (op = P_GetExprOperands (expr); op; op = P_GetExprSibling (op))
    PF_VisitExprNode (op);

  if (P_GetExprNext (expr))
    PF_VisitExprNode (P_GetExprNext (expr));

  /* III. Propagate attributes bottom-up (from results of recurrence) */

  switch (opcode)
    {
      /* op2 < op1 */
    case OP_assign:
    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Arshft:
    case OP_Alshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:
    case OP_call:
      op1 = P_GetExprOperands (expr);
      op2 = P_GetExprSibling (op1);

      /* Nothing to process for a function call with no arguments. */
      if (opcode == OP_call && !op2)
	break;

      /* If op1 is going to be broken out and has side-effects, all
       * subsequent operands must be broken out to maintain
       * computation ordering.  JWS---this concern appears to go
       * beyond the standard, which orders only the operators "&&",
       * "||", "?:" and "," (not including the list of function
       * arguments).
       */

      if ((PF_get_expr_flags (op1) & PF_CONTAIN_BREAK) &&
	  ((PF_get_expr_flags (op1) & PF_SIDE_EFFECT) ||
	   (PF_get_expr_flags (op2) & PF_SIDE_EFFECT)))
	{
	  for (opnext = op2; opnext; opnext = P_GetExprNext (opnext))
	    {
	      PF_set_expr_flags (opnext, PF_BREAK | PF_CONTAIN_BREAK);
	      if (P_GetExprOpcode (opnext) == OP_compexpr)
		PF_MarkCompExpr (opnext, PF_BREAK | PF_CONTAIN_BREAK);
	    }
	}

      PF_set_expr_flags (expr, (PF_get_expr_flags (op1) | \
				PF_get_expr_flags (op2)) & \
			 (PF_CONTAIN_BREAK | PF_SIDE_EFFECT));
      break;

    case OP_disj: /* op1 < op2 */
    case OP_conj: /* op1 < op2 */
    case OP_quest:/* op1 < op2, op1 < op3, op2 ? op3 */
      /* Already marked for flattening above.  Still need to propagate
       * side effect flag.
       */

      /* opx ? opy */
    default:
      for (op = P_GetExprOperands (expr); op; op = P_GetExprSibling (op))
	PF_set_expr_flags (expr, PF_get_expr_flags (op) & \
			   (PF_CONTAIN_BREAK | PF_SIDE_EFFECT));
      break;
    }

  /* Deal with the "next" expr ("," operator) */

  if ((op = P_GetExprNext (expr)))
    {
      /*
       * Example 1 : 
       *          a(b,c), d()
       *          The evaluation order is b->c->a()->d()
       *          Since d() should be evaluated later than a(), even though 
       *          a()'s status is 1, it still needs to be broken
       * Example 2 :
       *          a(b(), c())
       *          The evaluation order is {b(),c()}->a()
       *          So even b() and c() are broken, there is no need to break a()
       * Real Example which causes failure if not handled carefully:
       *          008.espresso/cvrm.c:sort_reduce()
       *          ( p[0] &= 0xffff, 
       *            p[0] |= ((((n-cdist(largest,p))<<7) + ........)
       *          where p[0] are updated twice in the same expression list, so
       *          order of evaluation matters
       */

      int fl = PF_get_expr_flags (op) & (PF_SIDE_EFFECT | PF_CONTAIN_BREAK);

      /* JWS need to check this logic against the standard */

      if (fl == (PF_SIDE_EFFECT | PF_CONTAIN_BREAK))
	{
	  if (!(PF_get_expr_flags (expr) & PF_ARGUMENT))
	    {
	      PF_set_expr_flags (expr, (PF_BREAK | PF_CONTAIN_BREAK | \
					PF_SIDE_EFFECT));

	      if (P_GetExprOpcode (expr) == OP_compexpr)
		PF_MarkCompExpr (expr, (PF_BREAK | PF_CONTAIN_BREAK | \
					PF_SIDE_EFFECT));
	    }
	  else
	    {
	      PF_set_expr_flags (expr, PF_CONTAIN_BREAK | PF_SIDE_EFFECT);
	    }
	}
      else if (fl & PF_CONTAIN_BREAK)
	{
	  PF_set_expr_flags (op, PF_CONTAIN_BREAK);
	  if (!(PF_get_expr_flags (expr) & PF_ARGUMENT) && \
	      (PF_get_expr_flags (expr) & PF_SIDE_EFFECT))
	    {
	      PF_set_expr_flags (expr, PF_BREAK | PF_CONTAIN_BREAK);
	      if (P_GetExprOpcode (expr) == OP_compexpr)
		PF_MarkCompExpr (expr, PF_BREAK | PF_CONTAIN_BREAK);
	    }
	}
      else if (fl & PF_SIDE_EFFECT)
	{
	  PF_set_expr_flags (expr, PF_SIDE_EFFECT);
	}
    }
  
  return;
}


/*! \brief Recursively sets the flags field of the 
 * 
 * Move the information from ext node into the expr node
 */
static void
PF_ResetExprFlags (Expr expr)
{
  Expr op;

  if (!expr)
    return;

  PF_set_expr_flags (expr, 0);

  for (op = P_GetExprOperands (expr); op; op = P_GetExprSibling (op))
    PF_ResetExprFlags (op);
  if ((op = P_GetExprNext (expr)))
    PF_ResetExprFlags (op);

  return;
}


/*! \brief Traverse the statements to annotate the expressions
 *
 * \param stmts
 *  the stmts to inspect.
 */
static void
PF_VisitStmts (Stmt stmts)
{
  while (stmts != NULL)
    {
      switch (P_GetStmtType (stmts))
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
	  {
	    Expr ret = P_GetStmtRet (stmts);
	    
	    if (ret)
	      {
		PF_set_expr_flags (ret, 0);
		PF_VisitExprNode (ret);
	      }
	  }
	  break;

	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmts);

	    PF_VisitStmts (P_GetCompoundStmtList (c));
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmts);
	    Expr cond_expr = P_GetIfStmtCondExpr (i);

	    if (cond_expr)
	      {
		PF_set_expr_flags (cond_expr, 0);
		PF_VisitExprNode (cond_expr);
	      }

	    PF_VisitStmts (P_GetIfStmtThenBlock (i));
	    PF_VisitStmts (P_GetIfStmtElseBlock (i));
	  }
	  break;

	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmts);
	    Expr expression = P_GetSwitchStmtExpression (s);

	    if (expression)
	      {
		PF_set_expr_flags (expression, 0);
		PF_VisitExprNode (expression);
	      }

	    PF_VisitStmts (P_GetSwitchStmtSwitchBody (s));
	  }
	  break;

	case ST_EXPR:
	  {
	    Expr expr = P_GetStmtExpr (stmts);

	    if (expr)
	      {
		PF_set_expr_flags (expr, 0);
		PF_VisitExprNode (expr);
	      }
	  }
	  break;

	case ST_PSTMT:
	  PF_VisitStmts (P_GetPstmtStmt (P_GetStmtPstmt (stmts)));
	  break;

	case ST_MUTEX:
	  PF_VisitStmts (P_GetMutexStatement (P_GetStmtMutex (stmts)));
	  break;

	case ST_COBEGIN:
	  PF_VisitStmts (P_GetCobeginStatements (P_GetStmtCobegin (stmts)));
	  break;

	case ST_SERLOOP:
	  {
	    SerLoop s = P_GetStmtSerLoop (stmts);
	    Expr init_expr = P_GetSerLoopInitExpr (s);
	    Expr cond_expr = P_GetSerLoopCondExpr (s);
	    Expr iter_expr = P_GetSerLoopIterExpr (s);
	    
	    if (init_expr)
	      {
		PF_set_expr_flags (init_expr, 0);
		PF_VisitExprNode (init_expr);
	      }

	    if (cond_expr)
	      {
		PF_set_expr_flags (cond_expr, 0);
		PF_VisitExprNode (cond_expr);
	      }

	    if (iter_expr)
	      {
		PF_set_expr_flags (iter_expr, 0);
		PF_VisitExprNode (iter_expr);
	      }

	    PF_VisitStmts (P_GetSerLoopLoopBody (s));
	  }
	  break;

	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (stmts);
	    Pstmt pstmt = P_GetParLoopPstmt (p);

	    PF_VisitStmts (P_GetPstmtStmt (pstmt));
	  }
	  break;

	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmts);

	    PF_VisitStmts (P_GetBodyStmtStatement (b));
	  }
	  break;

	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmts);
	    
	    PF_VisitStmts (P_GetEpilogueStmtStatement (e));
	  }
	  break;

	default:
	  P_punt ("flatten.c:PF_VisitStmts:%d Invalid stmt type %d", __LINE__,
		  P_GetStmtType (stmts));
	}

      stmts = P_GetStmtLexNext (stmts);
    }

  return;
}


/*
 * create a new local variable
 */
Expr
PF_NewLocalVar (Stmt stmt, Type type)
{
  Key scope;
  Expr lvar;
  VarDcl var;


  /* BCC - Don't create a variable of type VOID - 5/19/95 */
  if (PSI_IsVoidType (type))
    assert(0);

  scope = PSI_GetStmtScope (stmt);
  lvar = PSI_NewLocalVarExprTemp (scope, type, "P_");
  
  var = PSI_GetVarDclEntry (P_GetExprVarKey (lvar));
      
  P_SetVarDclQualifier (var, VQ_AUTO);

  return lvar;
}


/* 
 * When we flatten an expression, we have to propagate the label associated 
 * with the old expression to the new one. For example:
 *
 * L1: a(b());		==> L1: t1 = b();
 *				a(t1);
 * But sometimes the label is hidden under the first level statement, 
 *
 * { L1: a(b()); }	==> L1: t1 = b();
 *				{ a(t1); }
 * Or even complicate:
 *
 * L1 : { L2: a(b()); }	==> L1: L2: t1 = b();
 *				{ a(t1); }
 */
Label
PF_ExtractLabels (Stmt stmt)
{
  Label labels, nested;

  if (!stmt)
    return NULL;

  labels = stmt->labels;
  stmt->labels = NULL;

  if ((stmt->type == ST_COMPOUND) &&
      (nested = PF_ExtractLabels (stmt->stmtstruct.compound->stmt_list)))
    {
      if (!labels)
	{
	  labels = nested;
	}
      else
	{
	  Label last;

	  last = labels;
	  while (last->next)
	    last = last->next;
	  last->next = nested;
	  nested->prev = last;
	}
    }

  return labels;
}


/*! \brief Changes a question expression to an if/then/else stmt expression.
 *
 * \param base_stmt
 *  the statement containing the question expression.
 * \param expr
 *  the question expression.
 *
 * \return
 *  A pointer to temporary variable expr.
 *
 * Translates "x ? y : z" to 
 * if (x) result = y else result = z;
 * returns result
 */
static Expr
PF_GenQuest (Stmt base_stmt, Expr expr)
{
  Expr op, X, Y, Z, result = NULL, assign, temp;
  Stmt new_stmt, y_stmt, z_stmt;
  IfStmt new_if;
  Key scope_key = PSI_GetExprScope (expr);
  Type expr_type = PSI_ExprType (expr);

  /* 
   * Create a new stmt which is of type ST_IF
   */

  new_stmt = P_NewStmtWithType (ST_IF);
  PSI_StmtInsertStmtBefore (base_stmt, new_stmt);
  PF_CopyLineInfo (base_stmt, new_stmt);

  P_SetStmtPragma \
    (new_stmt,
     P_AppendPragmaNext \
       (P_GetStmtPragma (new_stmt),
	P_NewPragmaWithSpecExpr ("QUEST",
				 PSI_ScopeNewIntExpr (scope_key, 0))));

  /*
   * Create the new IfStmt and connect it to new_stmt
   */
  new_if = P_NewIfStmt ();
  P_SetStmtIfStmt (new_stmt, new_if);

  /*
   * Extract the three parts of the original "X ? Y : Z" expression
   */

  op = P_GetExprOperands (expr);
  X = PF_EnsureListCompound (PSI_CopyExprListToScope (scope_key, op));
  op = P_GetExprSibling (op);
  Y = PF_EnsureListCompound (PSI_CopyExprListToScope (scope_key, op));
  op = P_GetExprSibling (op);
  Z = PF_EnsureListCompound (PSI_CopyExprListToScope (scope_key, op));

  /* BCC - the situation can be divided into the following:
   * 1) a = b ? c : d;
   *    if (b)
   *        a = c;
   *    else
   *        a = d;
   *    use a dirctly
   * 
   * 2) *p = b ? c : d;
   *    if (b)
   *        *p = temp = c;
   *    else
   *        *p = temp = d;
   *    use temp to save the evaluation time
   *
   * 3) *p++ = b ? c : d;
   *    if (b)
   *        *p++ = temp = c;
   *    else
   *        *p++ = temp = d;
   *    use temp because p++ has side-effect.
   */

  if (PSI_IsVoidType (expr_type))
    result = NULL;
#if 0 && PF_MINIMIZE_TEMPORARIES
  else if (P_IsSideEffectFree (X))
    result = PSI_CopyExprToScope (scope_key, X);
#endif
  else
    result = PF_NewLocalVar (base_stmt, expr_type);

  /* BCC - a.b = P_0___1 = c, we can't delete P_0___1 */

  /*
   * True part
   */
  y_stmt = P_NewStmtWithType (ST_EXPR);
  PF_CopyLineInfo (base_stmt, y_stmt);

  /* BCC - 11/12/96
   * when the code is like a ? (void) foo1() : 0, where the second operand
   * is non-void, we still can't assign it to the result var, because the
   * destinitaion local var is "0", resulted from the return type of foo1().
   */
  if (PSI_IsVoidType (expr_type) || PSI_IsVoidType (PSI_ExprType (Y)))
    {
      P_SetStmtExpr (y_stmt, Y);
    }
  else
    {
      assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
      P_AppendExprOperands (assign, PSI_CopyExprToScope (scope_key, result));
      P_AppendExprOperands (assign, Y);

      PSI_CastExpr (assign);
      P_SetStmtExpr (y_stmt, assign);
    }

  /*
   * False part
   */
  z_stmt = P_NewStmtWithType (ST_EXPR);
  PF_CopyLineInfo (base_stmt, z_stmt);

  /* BCC - explained as above - 11/12/96 */
  if (PSI_IsVoidType (expr_type) || PSI_IsVoidType (PSI_ExprType (Z)))
    {
      P_SetStmtExpr (z_stmt, Z);
    }
  else
    {
      assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
      P_AppendExprOperands (assign, PSI_CopyExprToScope (scope_key, result));
      P_AppendExprOperands (assign, Z);
      PSI_CastExpr (assign);
      P_SetStmtExpr (z_stmt, assign);
    }

  /* 
   * Link the true and false part to the if stmt
   */
  P_SetStmtParentStmtAll (y_stmt, new_stmt);
  P_SetStmtParentStmtAll (z_stmt, new_stmt);
  P_SetIfStmtThenBlock (new_if, y_stmt);
  P_SetIfStmtElseBlock (new_if, z_stmt);

  /*
   * Flatten the cond expr of the if stmt
   */
  PF_ResetExprFlags (X);
  PF_VisitExprNode (X);

  temp = PF_GenerateExpr (new_stmt, X);
  X = PSI_RemoveExpr (X);

  P_SetExprParentStmtAll (temp, new_stmt);
  P_SetIfStmtCondExpr (new_if, temp);

  /*
   * Flatten the newly constructed true / false stmts
   */

  PF_VisitStmts (y_stmt);
  PF_FlattenStmts (y_stmt);

  PF_VisitStmts (z_stmt);
  PF_FlattenStmts (z_stmt);

  return (result);
}


/*! \brief Generates a sequence for OP_and and OP_or.
 *
 * \param base_stmt
 *  the statement containing the OP_and or OP_or.
 * \param expr
 *  the OP_and or OP_or expression.
 *
 * \return
 *  A pointer to the temporary variable created for this sequence.
 *
 * GenConjDisj 20011010 JWS
 * ----------------------------------------------------------------------
 * Generate a sequence for && and ||.  Now prevents creation of
 * difficult-to-remove RCMP ops in Lcode.
 * ----------------------------------------------------------------------
 * R = (X && Y) ==>   R = 1;
 *                    if (X == 0)
 *                       goto L;
 *                    else if (Y == 0)
 *                  L:   R = 0;
 * ----------------------------------------------------------------------
 * R = (X || Y) ==>   R = 0;
 *                    if (X != 0)
 *                       goto L;
 *                    else if (Y != 0)
 *                  L:   R = 1;
 * ----------------------------------------------------------------------
 *
 * Returns R.
 *
 * \note Who frees returned pointer?
 */
static Expr
PF_GenConjDisj (Stmt base_stmt, Expr expr)
{
  Expr X, Y, result, assign, cond_expr, op;
  Stmt new_stmt, if1, if2, goto_stmt;
  IfStmt if_stmt;
  Key scope_key = PSI_GetExprScope (expr),
    expr_type_key = PSI_ExprType (expr);
  Label label;
  int is_conj = (P_GetExprOpcode (expr) == OP_conj);

  /* Extract X and Y */
  op = P_GetExprOperands (expr);
  X = PF_EnsureListCompound (PSI_CopyExprListToScope (scope_key, op));
  op = P_GetExprSibling (op);
  Y = PF_EnsureListCompound (PSI_CopyExprListToScope (scope_key, op));

  /* Label for conditional result variable assignment */
  label = PSI_NewLabelTemp (scope_key, "assign_logic");

  /* Create and initialize the result variable
   * R = [1,0]; */
  result = PF_NewLocalVar (base_stmt, expr_type_key);

  assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
  P_AppendExprOperands (assign, PSI_CopyExprToScope (scope_key, result));
  P_AppendExprOperands (assign, PSI_ScopeNewIntExpr (scope_key,
						     is_conj ? 1 : 0));
  PSI_StmtInsertExprBefore (base_stmt, assign);

  /* if (X [=,!=] 0) */
  if1 = P_NewStmtWithType (ST_IF);
  PF_CopyLineInfo (base_stmt, if1);

  if_stmt = P_NewIfStmt ();
  cond_expr = PSI_ScopeNewExprWithOpcode (scope_key, is_conj ? OP_eq : OP_ne);
  P_AppendExprOperands (cond_expr, X);
  P_AppendExprOperands (cond_expr, PSI_ScopeNewIntExpr (scope_key, 0));

  P_SetIfStmtCondExpr (if_stmt, cond_expr);
  P_SetStmtIfStmt (if1, if_stmt);

  /* goto L; */
  goto_stmt = P_NewGotoStmt (label);

  P_SetIfStmtThenBlock (P_GetStmtIfStmt (if1), goto_stmt);

  /* else if (Y [=,!=] 0) */
  if2 = P_NewStmtWithType (ST_IF);
  PF_CopyLineInfo (base_stmt, if2);

  if_stmt = P_NewIfStmt ();
  cond_expr = PSI_ScopeNewExprWithOpcode (scope_key, is_conj ? OP_eq : OP_ne);
  P_AppendExprOperands (cond_expr, Y);
  P_AppendExprOperands (cond_expr, PSI_ScopeNewIntExpr (scope_key, 0));

  P_SetIfStmtCondExpr (if_stmt, cond_expr);
  P_SetStmtIfStmt (if2, if_stmt);

  P_SetIfStmtElseBlock (P_GetStmtIfStmt (if1), if2);

  /* L: R = [0,1] */
  new_stmt = P_NewStmtWithType (ST_EXPR);
  PF_CopyLineInfo (base_stmt, new_stmt);
  P_StmtAddLabel (new_stmt, label);

  assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
  P_AppendExprOperands (assign, PSI_CopyExprToScope (scope_key, result));
  P_AppendExprOperands (assign, PSI_ScopeNewIntExpr (scope_key,
						     is_conj ? 0 : 1));
  P_SetStmtExpr (new_stmt, assign);

  P_SetIfStmtThenBlock (P_GetStmtIfStmt (if2), new_stmt);

  PSI_StmtInsertStmtBefore (base_stmt, if1);

  /* Flatten new if () expressions */
  if_stmt = P_GetStmtIfStmt (if1);
  cond_expr = PF_GenerateExpr (if1, P_GetIfStmtCondExpr (if_stmt));
  P_SetIfStmtCondExpr (if_stmt,
		       PSI_RemoveExpr (P_GetIfStmtCondExpr (if_stmt)));
  P_SetIfStmtCondExpr (if_stmt, cond_expr);

  if_stmt = P_GetStmtIfStmt (if2);
  cond_expr = PF_GenerateExpr (if2, P_GetIfStmtCondExpr (if_stmt));
  P_SetIfStmtCondExpr (if_stmt,
		       PSI_RemoveExpr (P_GetIfStmtCondExpr (if_stmt)));
  P_SetIfStmtCondExpr (if_stmt, cond_expr);

  return (result);
}


static void
PF_ResetBreakNode (Expr expr)
{
  Expr op;

  if (!expr)
    return;

  switch (expr->opcode)
    {
    case OP_quest:
    case OP_conj:
    case OP_disj:
    case OP_call:
      break;
    case OP_index:
      PF_clr_expr_flags (expr, PF_BREAK);
      PF_ResetBreakNode (expr->operands);
      break;
    default:
      PF_clr_expr_flags (expr, PF_BREAK);
      for (op = expr->operands; op; op = op->sibling)
	PF_ResetBreakNode (op);
      break;
    }
  PF_ResetBreakNode (expr->next);
  return;
}


/****************************************************************************\
 *        X=			=            =
 *      /    \		      /   \         /  \
 *     A      B      =>      A     X       t    A
 *                               /   \
 *                              A     B
\****************************************************************************/
static Expr
PF_BreakAOP (Stmt base_stmt, Expr expr)
{
  _Opcode opcode = 0;
  Key scope_key;
  Expr new_assign, new_op1, new_op2, new_op2_op1, new_op2_op2, new_result;

  scope_key = PSI_GetStmtScope (base_stmt);

  switch (expr->opcode)
    {
    case OP_Aadd:
      opcode = OP_add;
      break;
    case OP_Asub:
      opcode = OP_sub;
      break;
    case OP_Amul:
      opcode = OP_mul;
      break;
    case OP_Adiv:
      opcode = OP_div;
      break;
    case OP_Amod:
      opcode = OP_mod;
      break;
    case OP_Arshft:
      opcode = OP_rshft;
      break;
    case OP_Alshft:
      opcode = OP_lshft;
      break;
    case OP_Aand:
      opcode = OP_and;
      break;
    case OP_Aor:
      opcode = OP_or;
      break;
    case OP_Axor:
      opcode = OP_xor;
      break;
    default:
      I_punt ("Unexpected opcode", 0);
    }

  new_op1 = expr->operands;
  new_op2_op2 = new_op1->sibling;
  new_op1->sibling = NULL;
  assert (new_op2_op2->sibling == NULL);
  new_op2_op1 = PSI_CopyExprList (new_op1);

  new_op2 = PSI_ScopeNewExprWithOpcode (scope_key, opcode);
  P_AppendExprOperands (new_op2, new_op2_op1);
  P_AppendExprOperands (new_op2, new_op2_op2);
  PSI_CastExpr (new_op2);

  /* Insert an explicit cast if the Aop result type is different from
   * the result type of the created RHS.  This cast is required in
   * PtoL. -- JWS 20040706
   */

    {
      Key t1 = PSI_ReduceTypedefs (PSI_ExprType (expr->operands)),
	t2 = PSI_ReduceTypedefs (PSI_ExprType (expr->operands->sibling));

      if (t1.file != t2.file || t1.sym != t2.sym)
	{
	  Expr cast_op = PSI_ScopeNewExprWithOpcode (scope_key, OP_cast);
	  P_SetExprType (cast_op, t1);
	  P_SetExprOperands (cast_op, new_op2);
	  new_op2 = cast_op;
	}
    }

  new_assign = expr;
  new_assign->opcode = OP_assign;
  new_assign->operands = NULL;
  P_AppendExprOperands (new_assign, new_op1);
  P_AppendExprOperands (new_assign, new_op2);

  PSI_StmtInsertExprBefore (base_stmt, new_assign);

#if 0 && PF_MINIMIZE_TEMPORARIES
  if (P_IsSideEffectFree (new_op1))
    {
      new_result =  PSI_CopyExprToScope (scope_key, new_op1);
    }
  else 
#endif
    {
      Expr new_assign2, new_a2_op1, new_a2_op2;

      new_assign2 = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
      new_result = PF_NewLocalVar (base_stmt, PSI_ExprType (expr));

      new_a2_op1 = PSI_CopyExprListToScope (PSI_GetStmtScope (base_stmt),
					    new_result);
      new_a2_op2 = PSI_CopyExprList (new_op1);
      P_AppendExprOperands (new_assign2, new_a2_op1);
      P_AppendExprOperands (new_assign2, new_a2_op2);

      PSI_StmtInsertExprBefore (base_stmt, new_assign2);
    }

  return new_result;
}


/****************************************************************************\
 *   Post_++  		  =        =
 *      |	        /   \    /   \
 *      A     	      t1     A  A     +
 *                                  /   \ 
 *                                 A     1
\****************************************************************************/
static Expr
PF_BreakPostOP (Stmt base_stmt, Expr expr)
{
  _Opcode opcode = 0;
  Expr new_result;
  Expr new_assign1, new_a1_op1, new_a1_op2;
  Expr new_assign2, new_a2_op1, new_a2_op2, new_a2_op2_op1, new_a2_op2_op2;
  Key scope_key;

  scope_key = PSI_GetStmtScope (base_stmt);

  switch (expr->opcode)
    {
    case OP_postinc:
      opcode = OP_add;
      break;
    case OP_postdec:
      opcode = OP_sub;
      break;
    default:
      I_punt ("Unexpected opcode", 0);
    }

  new_a1_op2 = expr->operands;

  new_result = PF_NewLocalVar (base_stmt, PSI_ExprType (expr));

  new_a1_op1 = PSI_CopyExprListToScope (scope_key, new_result);

  new_a2_op1 = PSI_CopyExprListToScope (scope_key, new_a1_op2);
  new_a2_op2_op1 = PSI_CopyExprListToScope (scope_key, new_a1_op2);

  /* build new_assign1 */
  new_assign1 = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
  P_AppendExprOperands (new_assign1, new_a1_op1);
  P_AppendExprOperands (new_assign1, new_a1_op2);
  PSI_StmtInsertExprBefore (base_stmt, new_assign1);

  /* build new oper in new_assign2 */
  new_assign2 = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
  new_a2_op2 = PSI_ScopeNewExprWithOpcode (scope_key, opcode);
  new_a2_op2_op2 = PSI_ScopeNewIntExpr (scope_key, 1);
  P_AppendExprOperands (new_a2_op2, new_a2_op2_op1);
  P_AppendExprOperands (new_a2_op2, new_a2_op2_op2);
  P_AppendExprOperands (new_assign2, new_a2_op1);
  P_AppendExprOperands (new_assign2, new_a2_op2);
  PSI_StmtInsertExprBefore (base_stmt, new_assign2);
  PSI_CastExpr (new_a2_op2);

  return new_result;
}


/****************************************************************************\
 *     Pre_++		        =           =
 *       | 		      /   \       /   \
 *       A           =>      A     +     t     A
 *                               /   \
 *                              A     1
\****************************************************************************/
static Expr
PF_BreakPreOP (Stmt base_stmt, Expr expr)
{
  _Opcode opcode = 0;
  Expr new_result;
  Expr new_assign, new_op1, new_op2, new_op2_op1, new_op2_op2;
  Expr new_assign2, new_a2_op1, new_a2_op2;
  Key scope_key;

  scope_key = PSI_GetStmtScope (base_stmt);

  switch (expr->opcode)
    {
    case OP_preinc:
      opcode = OP_add;
      break;
    case OP_predec:
      opcode = OP_sub;
      break;
    default:
      I_punt ("Unexpected opcode", 0);
    }

  new_op1 = expr->operands;
  new_op2_op1 = PSI_CopyExprList (new_op1);
  new_a2_op2 = PSI_CopyExprList (new_op1);

  new_op2 = PSI_ScopeNewExprWithOpcode (scope_key, opcode);
  new_op2_op2 = PSI_ScopeNewIntExpr (scope_key, 1);
  P_AppendExprOperands (new_op2, new_op2_op1);
  P_AppendExprOperands (new_op2, new_op2_op2);
  PSI_CastExpr (new_op2);

  new_assign = expr;
  new_assign->opcode = OP_assign;
  new_assign->operands = 0;
  P_AppendExprOperands (new_assign, new_op1);
  P_AppendExprOperands (new_assign, new_op2);
  PSI_StmtInsertExprBefore (base_stmt, new_assign);

  new_result = PF_NewLocalVar (base_stmt, PSI_ExprType (expr));

  new_a2_op1 = PSI_CopyExprListToScope (scope_key, new_result);
  new_assign2 = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
  P_AppendExprOperands (new_assign2, new_a2_op1);
  P_AppendExprOperands (new_assign2, new_a2_op2);
  PSI_StmtInsertExprBefore (base_stmt, new_assign2);

  return new_result;
}


/*! \brief Moves an expression into a new variable.
 *
 * \param base_stmt
 *  the parent statement of the expression.
 * \param expr
 *  the expression to process.
 *
 * \return
 *  A new variable to represent \a expr.
 *
 * The ugliest function in this file. For an expression node which needs BREAK,
 * it move the expression out and use a new variable to represent it. If the
 * node doesn't need BREAK, it just duplicates the node and return it.
 */
static Expr
PF_GenerateExpr (Stmt base_stmt, Expr expr)
{
  _Opcode opcode;
  Expr new = NULL;
  Key scope_key;

  if (!expr)
    P_punt ("flatten.c:PF_GenerateExpr:%d expr is NULL", __LINE__);

  scope_key = PSI_GetExprScope (expr);
  opcode = P_GetExprOpcode (expr);

  if (!(PF_get_expr_flags (expr) & PF_BREAK))
    {				/* no need to break; */
      /* EMN-03: Process the stmt expr. Only need to do this 
       *    when no BREAK is set because GenExpr with BREAK 
       *   calls GenExpr without BREAK to create its oper 
       */

      if (opcode == OP_stmt_expr)
	{
	  PF_FlattenStmts (P_GetExprStmt (expr));
	}

      switch (opcode)
	{
	case OP_assign:
	  {
	    Expr op1 = P_GetExprOperands (expr),
	      op2 = P_GetExprSibling (op1), newop1, newop2;

	    assert ((PF_get_expr_flags (op1) & PF_BREAK) == 0);

	    /* BCC - don't recycle the old lvar. Always use a fresh new
	     * local temp.
	     */

	    new = PSI_CopyExprNode (expr);
	    newop2 = PF_GenerateExpr (base_stmt, op2);
	    newop1 = PF_GenerateExpr (base_stmt, op1);
	    P_AppendExprOperands (new, newop1);
	    P_AppendExprOperands (new, newop2);
	  }
	  break;

	case OP_Aadd:
	case OP_Asub:
	case OP_Amul:
	case OP_Adiv:
	case OP_Amod:
	case OP_Arshft:
	case OP_Alshft:
	case OP_Aand:
	case OP_Aor:
	case OP_Axor:
	  {
	    Expr op1 = P_GetExprOperands (expr),
	      op2 = P_GetExprSibling (op1), newop1, newop2;

	    assert ((PF_get_expr_flags (op1) & PF_BREAK) == 0);

	    new = PSI_CopyExprNode (expr);
	    newop2 = PF_GenerateExpr (base_stmt, op2);
	    newop1 = PF_GenerateExpr (base_stmt, op1);
	    P_AppendExprOperands (new, newop1);
	    P_AppendExprOperands (new, newop2);
	  }
	  break;

	case OP_call:
	  {
	    Expr op1 = P_GetExprOperands (expr),
	      op2 = P_GetExprSibling (op1), newop1, newop2;

	    new = PSI_CopyExprNode (expr);
	    /* BCC - evaluate the call site before the parm list 
	     * 9/30/99 */
	    newop1 = PF_GenerateExpr (base_stmt, op1);
	    newop2 = op2 ? PF_GenerateExpr (base_stmt, op2) : NULL;

	    P_AppendExprOperands (new, newop1);
	    if (newop2)
	      P_AppendExprOperands (new, newop2);
	  }
	  break;

	case OP_compexpr:
	  {
	    Expr op = P_GetExprOperands (expr), newop;

	    if (P_GetExprNext (op) == NULL)
	      {
		new = PF_GenerateExpr (base_stmt, op);
	      }
	    else
	      {
		new = PSI_CopyExprNode (expr);
		newop = PF_GenerateExpr (base_stmt, op);
		/* op->next is handled inside PF_GenerateExpr() */
		P_AppendExprOperands (new, newop);
	      }
	  }
	  break;

	default:
	  {
	    Expr op, newop;
	    new = PSI_CopyExprNode (expr);

	    for (op = P_GetExprOperands (expr); op; op = P_GetExprSibling (op))
	      {
		newop = PF_GenerateExpr (base_stmt, op);
		P_AppendExprOperands (new, newop);
	      }
	  }
	  break;
	}
    }
  else
    {				/* need to break; */
      /*
       * add "t = expr" to expr tree
       * and use t instead 
       */
      /*  BCC - bug fix - 4/6/96 Suppose the code looks like:
       * foo1(&p1, *p2, foo2());
       *
       * It was transformed into after flattening: temp1 = p1;
       * temp2 = &temp1; temp3 = p2; temp4 = *p2; temp5 = foo2();
       * foo1(temp2, temp4, temp5);
       *
       *  where we can't use temp1 to hold p1 because we then take the
       *  address of temp1, not p1. So flatten.c is modified to handle
       *  OP_addr as a special case.
       *
       *  The following is the correct transformation:
       *          temp1 = &p1;
       *          temp2 = p2;
       *          temp3 = *temp2;
       *          temp4 = foo2();
       *          foo1(temp1, temp2, temp4);
       * BCC - 7/31/96
       * The bug fix is not complete yet. We can have code which 
       * looks like &(aaa.bbb[foo() + ccc?ddd:eee]). In this case
       * we still need to break foo() and ccc?ddd:eee but not
       * the rest. So ResetBreakNode() reset the break flags all
       * the way from the first operand of "&" till the end for all
       * the "LVAR" expressions.
       */

      switch (opcode)
	{
	case OP_assign:
	  {
	    Expr op, next_expr, assign;

	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    op = PF_GenerateExpr (base_stmt, expr);

	    PF_set_expr_flags (expr, PF_BREAK);
	    P_SetExprNext (expr, next_expr);

	    assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
	    new = PF_NewLocalVar (base_stmt, PSI_ExprType (expr));
	    P_AppendExprOperands (assign, new);
	    P_AppendExprOperands (assign, op);

	    PSI_CastExpr (assign);

	    PSI_StmtInsertExprBefore (base_stmt, assign);

	    new = PSI_CopyExpr (new);
	  }
	  break;

	case OP_quest:
	  new = PF_GenQuest (base_stmt, expr);
	  break;

	case OP_conj:
	case OP_disj:
	  new = PF_GenConjDisj (base_stmt, expr);
	  break;

	case OP_addr:
	  {
	    Expr op = P_GetExprOperands (expr), newop, op1, op2, assign,
	      next_expr;

	    if (P_GetExprOpcode (op) == OP_quest)
	      {
		Expr quest_op2 = P_GetExprSibling (P_GetExprOperands (op));
		Expr quest_op3 = P_GetExprSibling (quest_op2);

		/* flatten &(i ? j : k) into (i ? &j : &k) */
		newop = PSI_ScopeNewExprWithOpcode (scope_key, OP_quest);
		P_AppendExprOperands (newop, PSI_CopyExprToScope (scope_key,
								  op));

		op1 = PSI_ScopeNewExprWithOpcode (scope_key, OP_addr);
		P_AppendExprOperands (op1, 
				      PSI_CopyExprListToScope (scope_key,
							       quest_op2));
		PSI_CastExpr (op1);
		P_AppendExprOperands (newop, op1);

		op2 = PSI_ScopeNewExprWithOpcode (scope_key, OP_addr);
		P_AppendExprOperands (op2, 
				      PSI_CopyExprListToScope (scope_key,
							       quest_op3));
		PSI_CastExpr (op2);
		P_AppendExprOperands (newop, op2);

		PSI_CastExpr (newop);
		new = PF_GenQuest (base_stmt, newop);
	      }
	    else
	      {
		assert ((PF_get_expr_flags (op) & PF_BREAK) == 0);
		assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
		new = PF_NewLocalVar (base_stmt, PSI_ExprType (expr));

		P_AppendExprOperands (assign, new);
	      
		PF_clr_expr_flags (expr, PF_BREAK);
		next_expr = P_GetExprNext (expr);
		P_SetExprNext (expr, NULL);
		op = PF_GenerateExpr (base_stmt, expr);
		P_SetExprNext (expr, next_expr);
		PF_set_expr_flags (expr, PF_BREAK);

		P_AppendExprOperands (assign, op);
		PSI_CastExpr (assign);
		PSI_StmtInsertExprBefore (base_stmt, assign);
		new = PSI_CopyExprToScope (scope_key, new);
	      }
	  }
	  break;

	  /*
	   * Compile-time expressions
	   */
	case OP_enum:
	case OP_int:
	case OP_real:
	case OP_float:
	case OP_double:
	case OP_char:
	case OP_string:
	case OP_expr_size:
	case OP_type_size:
	  new = PSI_CopyExprToScope (scope_key, expr);
	  break;

	  /* 
	   * call/cast/complex expression
	   */
	case OP_call:
	case OP_cast:
	  {
	    Expr op, assign, next_expr;
	    Type expr_type = PSI_ExprType (expr);

	    /* We need to get create a variable with a function's return
	     * type, not the function type. */

	    /*
	     * If the returned type is VOID, there is no need to 
	     * generate t = call() or t = (....)
	     */
	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    op = PF_GenerateExpr (base_stmt, expr);
	    P_SetExprNext (expr, next_expr);
	    PF_set_expr_flags (expr, PF_BREAK);

	    if (PSI_GetTypeBasicType (expr_type) & BT_VOID)
	      {
		PSI_StmtInsertStmtBefore (base_stmt, P_NewExprStmt (op));
		new = PSI_ScopeNewIntExpr (scope_key, 0);
	      }
	    else
	      {
		assign = PSI_ScopeNewExprWithOpcode (scope_key, OP_assign);
		new = PF_NewLocalVar (base_stmt, PSI_ExprType (expr));
		P_AppendExprOperands (assign, new);
		P_AppendExprOperands (assign, op);
		PSI_CastExpr (assign);

		PSI_StmtInsertExprBefore (base_stmt, assign);
		new = PSI_CopyExprToScope (scope_key, new);
	      }
	  }
	  break;

	case OP_stmt_expr:
	  {
	    /*
	     * EMN-03 : Added to handle stmt_exprs 
	     *
	     * {
	     *  ... (  ({cmpdstmt; result})  )
	     * }
	     *
	     * INTO
	     *
	     * {
	     *   LVAR temp;
	     *
	     *   {cmpdstmt; temp = result;}
	     *
	     *   ... (  (result)  )
	     * }
	     *
	     * NOTE: RestructureStmtExpr has already created
	     *       the temp var and the new assignment.
	     */

	    Expr op, next_expr;
	    Stmt tmp_stmt = NULL, result_stmt;

	    /* Clear BREAK and next */
	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    
	    /* Generate this expr only w/o BREAK */
	    op = PF_GenerateExpr (base_stmt, expr);

	    /* Put back in BREAK and next */
	    PF_set_expr_flags (expr, PF_BREAK);
	    P_SetExprNext (expr, next_expr);

	    /* Find STMTEXPR result */
	    assert(op->value.stmt->type == ST_COMPOUND);
	    for (result_stmt = op->value.stmt->stmtstruct.compound->stmt_list; 
		 result_stmt->lex_next; result_stmt = result_stmt->lex_next);

	    assert(result_stmt);

	    if (result_stmt->type == ST_EXPR)
	      {
		Expr result_expr = result_stmt->stmtstruct.expr;
		P_ClrExprFlags (result_expr, EF_RETAIN);

		if (!PSI_IsVoidType (PSI_ExprType (result_expr)))
		  {
		    assert(result_expr->opcode == OP_assign);
		    assert(result_expr->operands->opcode == OP_var);
		
		    /* Return Lvar of assignment */
		    new = PSI_CopyExpr (result_expr->operands);
		  }
	      }

	    /* Move stmt out of op's STMTEXPR */

	    tmp_stmt = op->value.stmt;
	    op->value.stmt = NULL;

	    PSI_StmtInsertStmtBefore (base_stmt, tmp_stmt);
	    op = PSI_RemoveExpr (op);
	  }
	  break;
	case OP_compexpr:
	  {
	    Expr op, next_expr;
	    Type type;
	    /*
	     * If the returned type is VOID, there is no need to 
	     * generate t = call() or t = (....)
	     */
	    type = PSI_ExprType (expr);

	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    op = PF_GenerateExpr (base_stmt, expr);
	    PF_set_expr_flags (expr, PF_BREAK);

	    P_SetExprNext (expr, next_expr);
	    
	    if (PSI_IsVoidType (type))
	      {
		PSI_StmtInsertExprBefore (base_stmt, op);
		new = PSI_ScopeNewIntExpr (scope_key, 0);
	      }
	    else
	      {
		new = op;
	      }
	  }
	  break;
	case OP_postinc:
	case OP_postdec:
	  {
	    Expr op, next_expr;

	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    op = PF_GenerateExpr (base_stmt, expr);
	    PF_set_expr_flags (expr, PF_BREAK);
	    P_SetExprNext (expr, next_expr);
	    new = PF_BreakPostOP (base_stmt, op);
	  }
	  break;
	case OP_preinc:
	case OP_predec:
	  {
	    Expr op, next_expr;

	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    op = PF_GenerateExpr (base_stmt, expr);
	    PF_set_expr_flags (expr, PF_BREAK);
	    P_SetExprNext (expr, next_expr);
	    new = PF_BreakPreOP (base_stmt, op);
	  }
	  break;
	case OP_Aadd:
	case OP_Asub:
	case OP_Amul:
	case OP_Adiv:
	case OP_Amod:
	case OP_Arshft:
	case OP_Alshft:
	case OP_Aand:
	case OP_Aor:
	case OP_Axor:
	  {
	    Expr op, next_expr;

	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    op = PF_GenerateExpr (base_stmt, expr);
	    PF_set_expr_flags (expr, PF_BREAK);
	    P_SetExprNext (expr, next_expr);
	    new = PF_BreakAOP (base_stmt, op);
	  }
	  break;
	default:
	  {
	    Expr op, assign, next_expr;

	    assign = P_NewExprWithOpcode (OP_assign);

	    new = PF_NewLocalVar (base_stmt, PSI_ExprType (expr));

	    P_AppendExprOperands (assign, new);
	    PF_clr_expr_flags (expr, PF_BREAK);
	    next_expr = P_GetExprNext (expr);
	    P_SetExprNext (expr, NULL);
	    op = PF_GenerateExpr (base_stmt, expr);
	    PF_set_expr_flags (expr, PF_BREAK);
	    P_SetExprNext (expr, next_expr);
	    P_AppendExprOperands (assign, op);
	    PSI_CastExpr (assign);
	    PSI_StmtInsertExprBefore (base_stmt, assign);
	    new = PSI_CopyExpr (new);
	  }
	  break;
	}
    }

  if (P_GetExprNext (expr))
    new = P_AppendExprNext (new, PF_GenerateExpr (base_stmt,
						  P_GetExprNext (expr)));

  return new;
}


static int
PF_UselessExpr (Expr expr)
{
  int result = 1;
  Expr op;

  if (expr == 0)
    return 1;
  if (P_GetExprFlags (expr) & EF_RETAIN)
    return 0;
  switch (expr->opcode)
    {
    case OP_postinc:
    case OP_postdec:
    case OP_preinc:
    case OP_predec:
    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Arshft:
    case OP_Alshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:
    case OP_call:
    case OP_stmt_expr:
      /* EMN-03 : for now, stmt_exprs are never useless  */
      return 0;
    case OP_assign:
      op = expr->operands;
      return 0;
    case OP_var:
    case OP_enum:
    case OP_int:
    case OP_real:
    case OP_float:
    case OP_double:
    case OP_char:
    case OP_string:
    case OP_expr_size:
    case OP_type_size:
      return (PF_UselessExpr (P_GetExprNext (expr)));
    default:
      for (op = expr->operands; op; op = op->sibling)
	result &= PF_UselessExpr (op);
      return (result & PF_UselessExpr (P_GetExprNext (expr)));
    }
}


static int
PF_UselessExprNode (Expr expr)
{
  int result = 1;
  Expr op;

  if (expr == 0)
    return 1;
  if (P_GetExprFlags (expr) & EF_RETAIN)
    return 0;
  switch (expr->opcode)
    {
    case OP_postinc:
    case OP_postdec:
    case OP_preinc:
    case OP_predec:
    case OP_assign:
    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Arshft:
    case OP_Alshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:
    case OP_call:
    case OP_stmt_expr:
      /* EMN-03 : for now, stmt_exprs are never useless */
      return 0;
    case OP_var:
    case OP_enum:
    case OP_int:
    case OP_real:
    case OP_float:
    case OP_double:
    case OP_char:
    case OP_string:
    case OP_expr_size:
    case OP_type_size:
      return 1;
    default:
      for (op = expr->operands; op; op = op->sibling)
	result &= PF_UselessExpr (op);
      return result;
    }
}


static Expr
PF_CompressExprList (Expr expr, int last)
{
  Expr pointer, header = NULL, tail = NULL;

  if (!expr)
    {
      ;
    }
  else if (expr->opcode == OP_compexpr && !P_GetExprNext (expr))
    {
      expr->operands = PF_CompressExprList (expr->operands, last);
      if (!expr->operands)
	expr = PSI_RemoveExpr (expr);
    }
  else
    {
      pointer = expr;
      while (pointer)
	{
	  if ((last == 1 && !P_GetExprNext (pointer)) || \
	      !PF_UselessExprNode (pointer))
	    {
	      if (!header)
		{
		  header = tail = pointer;
		}
	      else
		{
		  P_SetExprNext (tail, pointer);
		  tail = pointer;
		}
	      pointer = P_GetExprNext (pointer);
	    }
	  else
	    {
	      Expr temp = P_GetExprNext (pointer);
	      P_SetExprNext (pointer, NULL);
	      pointer = PSI_RemoveExpr (pointer);
	      pointer = temp;
	    }
	}
      if (tail)
	P_SetExprNext (tail, NULL);

      expr = header;
    }
  return expr;
}


/*! \brief Flattens statements in a list.
 *
 * \param stmts
 *  the statements to flatten.
 *
 * Flattens the statements in a list.  Changes "a, b, c" to "a; b; c;"
 */
static void
PF_FlattenStmts (Stmt stmts)
{
  while (stmts != NULL)
    {
      switch (P_GetStmtType (stmts))
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
	  {
	    Expr expr;
	    if ((expr = P_GetStmtRet (stmts)))
	      {
		Expr result = PF_GenerateExpr (stmts, expr);

		P_SetStmtRet (stmts, NULL);
		expr = PSI_RemoveExpr (expr);
		expr = PF_CompressExprList (result, 1);
		P_SetExprParentStmtAll (expr, stmts);
		P_SetStmtRet (stmts, expr);
	      }
	  }
	  break;

	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmts);

	    PF_FlattenStmts (P_GetCompoundStmtList (c));
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmts);
	    Expr result, cond_expr = P_GetIfStmtCondExpr (i);

	    result = PF_GenerateExpr (stmts, cond_expr);
	    cond_expr = PSI_RemoveExpr (cond_expr);
	    cond_expr = PF_CompressExprList (result, 1);
	    P_SetExprParentStmtAll (cond_expr, stmts);
	    P_SetIfStmtCondExpr (i, cond_expr);

	    PF_FlattenStmts (P_GetIfStmtThenBlock (i));
	    PF_FlattenStmts (P_GetIfStmtElseBlock (i));
	  }
	  break;

	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmts);
	    Expr result, expression = P_GetSwitchStmtExpression (s);

	    result = PF_GenerateExpr (stmts, expression);
	    expression = PSI_RemoveExpr (expression);
	    expression = PF_CompressExprList (result, 1);
	    P_SetExprParentStmtAll (expression, stmts);
	    P_SetSwitchStmtExpression (s, expression);

	    PF_FlattenStmts (P_GetSwitchStmtSwitchBody (s));
	  }
	  break;

	case ST_EXPR:
	  {
	    Expr result, expr = P_GetStmtExpr (stmts);
	    _Opcode opcode = P_GetExprOpcode (expr);

	    if (opcode == OP_call && !P_GetExprNext (expr))
	      {
		/* 
		 * BCC - HtoL can not handle a function which returns a 
		 * structure without a lvar. That is, foo1() can not be 
		 * handled unless it becomes temp = foo1() if foo1 returns 
		 * a structure - 7/9/96 
		 */
		if (!PSI_IsStructureType (PSI_ExprType (expr)))
		  PF_clr_expr_flags (expr, PF_BREAK);
	      }

	    result = PF_GenerateExpr (stmts, expr);
	    P_SetStmtExpr (stmts, NULL);
	    expr = PSI_RemoveExpr (expr);
	    result = PF_CompressExprList (result, 0);

	    /* 
	     * Don't link useless expresstions to stmt
	     */
	    if (PF_UselessExpr (result))
	      {
		P_SetStmtType (stmts, ST_NOOP);
		if (result)
		  result = PSI_RemoveExpr (result);
	      }
	    else
	      {
		P_SetExprParentStmtAll (result, stmts);
		P_SetStmtExpr (stmts, result);
	      }
	  }
	  break;
	  
	case ST_PSTMT:
	  {
	    Pstmt p = P_GetStmtPstmt (stmts);

	    PF_FlattenStmts (P_GetPstmtStmt (p));
	  }
	  break;

	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (stmts);

	    PF_FlattenStmts (P_GetMutexStatement (m));
	  }
	  break;

	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (stmts);

	    PF_FlattenStmts (P_GetCobeginStatements (c));
	  }
	  break;

	case ST_SERLOOP:
	  {
	    SerLoop s = P_GetStmtSerLoop (stmts);

	    PF_FlattenStmts (P_GetSerLoopLoopBody (s));
	  }
	  break;

	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (stmts);
	    Pstmt pstmt = P_GetParLoopPstmt (p);

	    PF_FlattenStmts (P_GetPstmtStmt (pstmt));
	  }
	  break;

	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmts);

	    PF_FlattenStmts (P_GetBodyStmtStatement (b));
	  }
	  break;

	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmts);

	    PF_FlattenStmts (P_GetEpilogueStmtStatement (e));
	  }
	  break;

	default:
	  P_punt ("flatten.c:PF_FlattenStmts:%d Invalid stmt type %d",
		  __LINE__ - 1, P_GetStmtType (stmts));
	}

      stmts = P_GetStmtLexNext (stmts);
    }

  return;
}


/*! \brief Breaks accumulate operations in for loops inside stmt expressions.
 *
 * \param expr
 *  the expr to search for for loops. 
 *
 * EMN-03 : Added to handle stmt_exprs
 *
 * \sa BreakAopInForStmts()
 */
static void
PF_BreakAopInForStmts_expr (Expr expr)
{
  Expr ptr;

  while (expr)
    {
      switch (P_GetExprOpcode (expr))
	{
	case OP_stmt_expr:
	  PF_BreakAopInForStmts (P_GetExprStmt (expr));
	  break;

	default:
	  /* Do nothing */
	  break;
	}
      
      /* Recurse through rest of expr tree */
      for (ptr = P_GetExprOperands (expr); ptr; ptr = P_GetExprSibling (ptr))
	PF_BreakAopInForStmts_expr (ptr);

      expr = P_GetExprNext (expr);
    }

  return;
}


/*! \brief Breaks accumulate operations in for loops.
 *
 * \param stmts
 *  the stmt to search for for loops.
 *
 * \sa BreakAopInForStmts_expr()
 */
static void
PF_BreakAopInForStmts (Stmt stmts)
{
  Key scope_key;

  while (stmts != NULL)
    {
      scope_key = PSI_GetStmtScope (stmts);
      switch (P_GetStmtType (stmts))
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_RETURN:
	case ST_PSTMT:
	case ST_MUTEX:
	case ST_COBEGIN:
	case ST_PARLOOP:
	case ST_BODY:
	case ST_EPILOGUE:
	case ST_ASM:
	  break;

	case ST_EXPR:
	  PF_BreakAopInForStmts_expr (P_GetStmtExpr (stmts));
	  break;

	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmts);
	    PF_BreakAopInForStmts (P_GetCompoundStmtList (c));
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmts);

	    PF_BreakAopInForStmts_expr (P_GetIfStmtCondExpr (i));
	    PF_BreakAopInForStmts (P_GetIfStmtThenBlock (i));
	    PF_BreakAopInForStmts (P_GetIfStmtElseBlock (i));
	  }
	  break;

	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmts);

	    PF_BreakAopInForStmts_expr (P_GetSwitchStmtExpression (s));
	    PF_BreakAopInForStmts (P_GetSwitchStmtSwitchBody (s));
	  }
	  break;
	  
	case ST_SERLOOP:
	  {
	    /* Seems to make incorrect assumptions. */
	    SerLoop s = P_GetStmtSerLoop (stmts);
	    
	    if (P_GetSerLoopLoopType (s) == LT_FOR)
	      {
		Expr op1, op2, op_op = NULL;
		Expr iter_expr;
		
		for (iter_expr = P_GetSerLoopIterExpr (s); iter_expr;
		     iter_expr = P_GetExprNext (iter_expr))
		  {
		    switch (P_GetExprOpcode (iter_expr))
		      {
		      case OP_Aadd:
		      case OP_Asub:
			op1 = P_GetExprOperands (iter_expr);
			op2 = P_GetExprSibling (op1);

			P_SetExprOperands (iter_expr, NULL);
			P_SetExprSibling (op1, NULL);
			P_SetExprSibling (op2, NULL);

			if (P_GetExprOpcode (iter_expr) == OP_Aadd)
			  op_op = PSI_ScopeNewExprWithOpcode (scope_key,
							      OP_add);
			else if (P_GetExprOpcode (iter_expr) == OP_Asub)
			  op_op = PSI_ScopeNewExprWithOpcode (scope_key,
							      OP_sub);

			/* op1 was disconnected from iter_expr above.
			 * Therefore, we can't determine op1's scope, and
			 * must explicitly call PSI_CopyExprToScope. */
			P_AppendExprOperands (op_op,
					      PSI_CopyExprToScope (scope_key,
								   op1));
			P_AppendExprOperands (op_op, op2);

			P_SetExprOpcode (iter_expr, OP_assign);
			P_AppendExprOperands (iter_expr, op1);
			P_AppendExprOperands (iter_expr, op_op);
			break;

		      case OP_postinc:
		      case OP_preinc:
		      case OP_postdec:
		      case OP_predec:
			op1 = P_GetExprOperands (iter_expr);
			op2 = PSI_ScopeNewIntExpr (scope_key, 1);

			P_SetExprOperands (iter_expr, NULL);

			if (P_GetExprOpcode (iter_expr) == OP_postinc ||
			    P_GetExprOpcode (iter_expr) == OP_preinc)
			  op_op = PSI_ScopeNewExprWithOpcode (scope_key,
							      OP_add);
			else if (P_GetExprOpcode (iter_expr) == OP_postdec ||
				 P_GetExprOpcode (iter_expr) == OP_predec)
			  op_op = PSI_ScopeNewExprWithOpcode (scope_key,
							      OP_sub);

			/* op1 was disconnected from iter_expr above.
			 * Therefore, we can't determine op1's scope, and
			 * must explicitly call PSI_CopyExprToScope. */
			P_AppendExprOperands (op_op,
					      PSI_CopyExprToScope (scope_key,
								   op1));
			P_AppendExprOperands (op_op, op2);

			P_SetExprOpcode (iter_expr, OP_assign);
			P_AppendExprOperands (iter_expr, op1);
			P_AppendExprOperands (iter_expr, op_op);
			break;

		      default:
			break;
		      }
		  }
	      }

	    PF_BreakAopInForStmts (P_GetSerLoopLoopBody (s));
	  }
	  break;

	default:
	  P_punt ("flatten.c:PF_BreakAopInForStmts:%d Invalid stmt type %d",
		  __LINE__, P_GetStmtType (stmts));
	}
      
      stmts = P_GetStmtLexNext (stmts);
    }

  return;
}


/*
 * Determine if ST_EXPR has side-effects or not
 */
static void
PF_RemoveUselessStmts (Stmt stmts)
{
  SerLoop serloop;

  while (stmts != NULL)
    {
      switch (stmts->type)
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_RETURN:
	case ST_ASM:
	  break;
	case ST_COMPOUND:
	  PF_RemoveUselessStmts (stmts->stmtstruct.compound->stmt_list);
	  break;
	case ST_IF:
	  PF_RemoveUselessStmts (stmts->stmtstruct.ifstmt->then_block);

	  if (stmts->stmtstruct.ifstmt->else_block != NULL)
	    PF_RemoveUselessStmts (stmts->stmtstruct.ifstmt->else_block);
	  break;
	case ST_SWITCH:
	  PF_RemoveUselessStmts (stmts->stmtstruct.switchstmt->switchbody);
	  break;
	case ST_EXPR:
	  if (PF_UselessExpr (stmts->stmtstruct.expr) && !stmts->labels)
	    {
	      stmts->stmtstruct.expr = PSI_RemoveExpr (stmts->stmtstruct.expr);
	      stmts->type = ST_NOOP;
	    }
	  break;
	case ST_SERLOOP:
	  serloop = stmts->stmtstruct.serloop;

	  PF_RemoveUselessStmts (serloop->loop_body);
	  break;
	default:
	  I_punt ("PF_RemoveUselessStmts: Invalid statement type", stmts);
	}
      stmts = stmts->lex_next;
    }
}


/*!
 * \param func
 *  the FuncDcl to process.
 *
 * Reconstruct the function definition, so all && || ?: are removed,
 * and calls are moved to the top level.
 */
void
PF_Flatten (FuncDcl func)
{
  /* A bit to indicate that the function has been flattened.
   * Does this need to be saved as a pragma? */
  if (PF_func_flattened (func))
    return;

  PF_func_flattened (func) = 1;

  PF_BreakAopInForStmts (P_GetFuncDclStmt (func));

  PF_VisitStmts (func->stmt);

  PF_Restructure (func);

  PF_VisitStmts (func->stmt);
  /* change "a, b, c" to "a; b; c;" */
  PF_FlattenStmts (func->stmt);

  PF_RemoveUselessStmts (func->stmt);
  PF_RestructureArrayAccesses (func);

  return;
}

/*! \brief A library interface to PF_Flatten().
 *
 * \param symbol_table
 *  the symbol table.
 * \param func
 *  the FuncDcl to flatten.
 */
void
PFT_Flatten (SymbolTable symbol_table, FuncDcl func)
{
  PSI_PushTable (symbol_table);
  PF_Flatten (func);
  PSI_PopTable ();

  return;
}
