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
 *      File:   pin_expand.c
 *      Author: Ben-Chung Cheng
 *      Copyright (c) 1995 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library/i_hash.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/query.h>
#include <Pcode/cast.h>
#include <Pcode/symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/struct_symtab.h>
#include "pin_inline.h"

static void Pin_scale_stmts (Stmt s, double k);
static void Pin_uniquify_labels_stmt (Stmt stmt, Key key);
static void Pin_uniquify_labels_expr (Expr expr, Key key);
static void Pin_uniquify_labels_init (Init init, Key key);
static void Pin_static2global_expr (Expr expr, void *data);
static void Pin_change_var_key (Expr expr, void *data);

/*! A struct used to change one key to another. */
typedef struct Pin_key_change
{
  Key old;
  Key new;
} Pin_key_change;

/* 06/15/04 REK This function now takes function's body as a separate
 *              argument instead of accessing it through the function.
 *              I insert the stmt into the caller immediately after
 *              copying to get stmt parents and scopes squared away.
 *              At that time, I set fn->stmt to null.
 */
/* 07/01/04 REK This function now copies VarDcls from the FuncDcl's param
 *              list instead of extracting the pointers.  We need to do
 *              the copy to make sure the VarDcls get moved across files
 *              if necessary. */
static void
Pin_param2lvar (FuncDcl fn, Stmt s)
{
  VarDcl param, lvar;
  Pin_key_change keys;

  /* Add parameters to the local variable list.  Array-type parameters
   * are added as pointers. 
   */

  List_start (fn->param);
  while ((param = (VarDcl) List_next (fn->param)))
    {
      lvar = PSI_CopyVarDclToScope (PSI_GetStmtScope (s), param);

      s->stmtstruct.compound->var_list = 
	List_insert_last (s->stmtstruct.compound->var_list, lvar);

      /* Add the VarDcl to the stmt's scope. */
      PSI_AddEntryToScope (P_GetStmtKey (s), P_GetVarDclKey (lvar));

      /* Change references to the parameter to reference the copied
       * variable. */
      keys.old = P_GetVarDclKey (param);
      keys.new = P_GetVarDclKey (lvar);
      P_StmtApply (s, NULL, Pin_change_var_key, &keys);

      P_ClrVarDclQualifier (lvar, VQ_PARAMETER);
      P_SetVarDclQualifier (lvar, VQ_AUTO);
      
      /* There's no need to reduce typedefs when using the Type functions. */
      if (PSI_IsArrayType (lvar->type))
	lvar->type = PSI_FindPointerToType (PSI_GetTypeType (lvar->type));
    }
  return;
}

/*! \brief Converts the call parameters to local variable assignments.
 *
 * \param call_stmt
 *  the statement containing the OP_call expression.
 * \param stmt
 *  the compound stmt enclosing the inlined function body.
 * \param actual_param
 *  the list of arguments to the call.
 *
 * \a stmt is the compound statement enclosing the inlined function body.
 * The inlined function body is a compound statement of its own
 * (FuncDcl->stmt), so the statement list is two levels deep at this point.
 * All function parameters were moved to variables local to \a stmt by
 * Pin_param2lvar() before calling this function.  \a stmt's var_list
 * is the same as the original function's param list at this point.
 */
static void
Pin_actual2formal (Stmt call_stmt, Stmt stmt, Expr actual_param)
{
  Stmt first_stmt, new_stmt;
  VarDcl formal_param;
  VarList formal_param_list = P_GetCompoundVarList (P_GetStmtCompound (stmt));
  Key scope = PSI_GetStmtScope (stmt);
  Expr expr, op1, op2;

  List_start (formal_param_list);

  while (actual_param)
    {
      if (!(formal_param = List_next (formal_param_list)))
	P_punt ("Pin_actual2formal : numbers of parameters don't match");

      expr = PSI_ScopeNewExprWithOpcode (scope, OP_assign);
      expr->type = formal_param->type;

      op1 = PSI_ScopeNewExprWithOpcode (scope, OP_var);
      P_SetExprVarName (op1, strdup (formal_param->name));
      P_SetExprVarKey (op1, formal_param->key);
      P_SetExprType (op1, formal_param->type);

      op2 = PSI_CopyExprToScope (scope, actual_param);

      P_AppendExprOperands (expr, op1);
      P_AppendExprOperands (expr, op2);

      new_stmt = P_NewStmtWithType (ST_EXPR);
      P_SetStmtExpr (new_stmt, expr);

      /* TLJ 7/15/96 */
      new_stmt->profile = P_CopyProfST (call_stmt->profile);

      first_stmt = stmt->stmtstruct.compound->stmt_list;
      /* The current compound statement might be null. In this case, using
         Insert_stmt_Before() may cause error when it calls 
         Create_Encl_Compd_IfNeeded(). */
      if (!first_stmt)
	{
	  stmt->stmtstruct.compound->stmt_list = new_stmt;
	  new_stmt->parent = stmt;
	}
      else
	{
	  PSI_StmtInsertStmtBefore (first_stmt, new_stmt);
	}

      actual_param = actual_param->next;
    }

  if(List_next (formal_param_list))
    P_punt ("Pin_actual2formal : numbers of parameters don't match (2)");

  return;
}

static Expr Pin_r2g_lvar;
static Label Pin_r2g_label;
static Expr Pin_r2g_cast;

static void 
Pin_stmt_r2g (Stmt stmts, void *data)
{
  if (stmts->type == ST_RETURN)
    {
      Expr lvar = Pin_r2g_lvar;
      Label label = Pin_r2g_label;
      Expr cast = Pin_r2g_cast;
      Stmt goto_stmt, copy_stmt;

      if (stmts->stmtstruct.ret && lvar)
	{
	  /* with something to return, the return expression is */
	  /* replaced by an assignment or expr followed by a goto */

	  Expr expr, op1, op2, rexpr;
	  Key scope = PSI_GetStmtScope (stmts);
		    
	  op1 = PSI_CopyExprToScope (scope, lvar);
	  expr = PSI_ScopeNewExprWithOpcode (scope, OP_assign);
	  expr->type = op1->type;

	  rexpr = PSI_CopyExprToScope (scope, stmts->stmtstruct.ret);

	  if (!cast)
	    {
	      op2 = rexpr;
	    }
	  else
	    {
	      op2 = PSI_ScopeNewExprWithOpcode (scope, OP_cast);
	      op2->type = cast->type;
	      P_AppendExprOperands (op2, rexpr);
	    }

	  P_AppendExprOperands (expr, op1);
	  P_AppendExprOperands (expr, op2);
	  
	  copy_stmt = P_NewStmtWithType (ST_EXPR);
	  P_SetStmtExpr (copy_stmt, expr);
	  PSI_StmtInsertStmtBefore (stmts, copy_stmt);
	}

      goto_stmt = P_NewGotoStmt (label);
      goto_stmt->profile = P_CopyProfST (stmts->profile);
      PSI_StmtInsertStmtBefore (stmts, goto_stmt);

      P_StmtRemoveStmt (stmts);
      stmts = PSI_RemoveStmt (stmts);
    }
  return;
}


/*! \brief Convert inlined returns to goto statements and retval assignments
 *
 * \param stmts
 *  statement(s) containing returns
 * \param lvar
 *  lval of assignment, to receive return value
 * \param label
 *  label at end of inlined function body
 * \param cast
 *  cast of return value
 *
 */
static void
Pin_return2goto (Stmt stmts, Expr lvar, Label label, Expr cast)
{
  Pin_r2g_lvar = lvar;
  Pin_r2g_label = label;
  Pin_r2g_cast = cast;

  P_StmtApply (stmts, Pin_stmt_r2g, NULL, NULL);

  return;
}


/*! \brief Ensures labels in inlined function bodies have unique names.
 *
 * \param stmt
 *  the statement to search for labels.
 * \param key
 *  the key of the compound statement added for the inlined function.
 *
 * \a key should come from the compound statement created to hold the inlined
 * function body.  The function body (FuncDcl->stmt) is another compound
 * statement, so the statement list is two levels deep.
 *
 * The inlined function bodies have not been rekeyed at this point.  To
 * make a unique label, we'll append \a key to each label to give
 * a unique name.
 */
static void
Pin_uniquify_labels_stmt (Stmt stmt, Key key)
{
  Label l;
  int new_val_len;
  char *new_val;

  while (stmt)
    {
      for (l = P_GetStmtLabels (stmt); l; l = P_GetLabelNext (l))
	{
	  if (l->val)
	    {
	      new_val_len = strlen (l->val) + KEY_STRLEN;
	      new_val = malloc (new_val_len);
	      snprintf (new_val, new_val_len, "%s_%d_%d", l->val, key.file,
			key.sym);
	      free (l->val);
	      l->val = new_val;
	    }
	}

      switch (P_GetStmtType (stmt))
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_ADVANCE:
	case ST_AWAIT:
	  break;
	case ST_RETURN:
	  Pin_uniquify_labels_expr (P_GetStmtRet (stmt), key);
	  break;
	case ST_GOTO:
	  /* It is not necessary to update the goto's label value for Pcode
	   * (Pcode only cares about the key), but we'll do it for human
	   * readers. */
	  new_val_len = strlen (P_GetStmtLabelVal (stmt)) + KEY_STRLEN;
	  new_val = malloc (new_val_len);
	  snprintf (new_val, new_val_len, "%s_%d_%d", P_GetStmtLabelVal (stmt),
		    key.file, key.sym);
	  free (P_GetStmtLabelVal (stmt));
	  P_SetStmtLabelVal (stmt, new_val);
	  break;
	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmt);
	    VarList vl = P_GetCompoundVarList (c);
	    VarDcl v;
	    Init i;

	    List_start (vl);
	    while ((v = (VarDcl)List_next (vl)))
	      if ((i = P_GetVarDclInit (v)))
		Pin_uniquify_labels_init (i, key);

	    Pin_uniquify_labels_stmt (P_GetCompoundStmtList (c), key);
	  }
	  break;
	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmt);

	    Pin_uniquify_labels_expr (P_GetIfStmtCondExpr (i), key);
	    Pin_uniquify_labels_stmt (P_GetIfStmtThenBlock (i), key);
	    Pin_uniquify_labels_stmt (P_GetIfStmtElseBlock (i), key);
	  }
	  break;
	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmt);

	    Pin_uniquify_labels_expr (P_GetSwitchStmtExpression (s), key);
	    Pin_uniquify_labels_stmt (P_GetSwitchStmtSwitchBody (s), key);
	  }
	  break;
	case ST_PSTMT:
	  {
	    Pstmt p = P_GetStmtPstmt (stmt);

	    Pin_uniquify_labels_stmt (P_GetPstmtStmt (p), key);
	  }
	  break;
	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (stmt);

	    Pin_uniquify_labels_expr (P_GetMutexExpression (m), key);
	    Pin_uniquify_labels_stmt (P_GetMutexStatement (m), key);
	  }
	  break;
	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (stmt);

	    Pin_uniquify_labels_stmt (P_GetCobeginStatements (c), key);
	  }
	  break;
	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (stmt);

	    Pin_uniquify_labels_stmt (P_GetPstmtStmt (P_GetParLoopPstmt (p)),
				      key);
	    Pin_uniquify_labels_expr (P_GetParLoopIterationVar (p), key);
	    Pin_uniquify_labels_stmt (P_GetParLoopSibling (p), key);
	    Pin_uniquify_labels_expr (P_GetParLoopInitValue (p), key);
	    Pin_uniquify_labels_expr (P_GetParLoopFinalValue (p), key);
	    Pin_uniquify_labels_expr (P_GetParLoopIncrValue (p), key);
	  }
	  break;
	case ST_SERLOOP:
	  {
	    SerLoop s = P_GetStmtSerLoop (stmt);

	    Pin_uniquify_labels_stmt (P_GetSerLoopLoopBody (s), key);
	    Pin_uniquify_labels_expr (P_GetSerLoopCondExpr (s), key);
	    Pin_uniquify_labels_expr (P_GetSerLoopInitExpr (s), key);
	    Pin_uniquify_labels_expr (P_GetSerLoopIterExpr (s), key);
	  }
	  break;
	case ST_EXPR:
	  Pin_uniquify_labels_expr (P_GetStmtExpr (stmt), key);
	  break;
	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmt);

	    Pin_uniquify_labels_stmt (P_GetBodyStmtStatement (b), key);
	  }
	  break;
	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmt);

	    Pin_uniquify_labels_stmt (P_GetEpilogueStmtStatement (e), key);
	  }
	  break;
	case ST_ASM:
	  {
	    AsmStmt a = P_GetStmtAsmStmt (stmt);

	    Pin_uniquify_labels_expr (P_GetAsmStmtAsmClobbers (a), key);
	    Pin_uniquify_labels_expr (P_GetAsmStmtAsmString (a), key);
	    Pin_uniquify_labels_expr (P_GetAsmStmtAsmOperands (a), key);
	  }
	  break;
	}

      stmt = P_GetStmtLexNext (stmt);
    }

  return;
}


/*! \brief Ensures labels in inlined function bodies have unique names.
 *
 * \param expr
 *  the expression to search for labels.
 * \param key
 *  the key of the compound statement added for the inlined function.
 *
 * \a key should come from the compound statement created to hold the inlined
 * function body.  The function body (FuncDcl->stmt) is another compound
 * statement, so the statement list is two levels deep.
 *
 * The inlined function bodies have not been rekeyed at this point.  To
 * make a unique label, we'll append \a key to each label to give
 * a unique name.
 */
static void
Pin_uniquify_labels_expr (Expr expr, Key key)
{
  Expr op;

  while (expr)
    {
      if (P_GetExprOpcode (expr) == OP_stmt_expr)
	Pin_uniquify_labels_stmt (P_GetExprStmt (expr), key);
      else
	for (op = P_GetExprOperands (expr); op; op = P_GetExprSibling (op))
	  Pin_uniquify_labels_expr (op, key);

      expr = P_GetExprNext (expr);
    }

  return;
}


/*! \brief Ensures labesl in function initializers have unique names.
 *
 * \param init
 *  the init to search for labels.
 * \param key
 *  the key of the compound statement added for the inlined function.
 *
 * \a key should come from the compound statement created to hold the inlined
 * function body.  The function body (FuncDcl->stmt) is another compound
 * statement, so the statement list is two levels deep.
 *
 * The inlined function bodies have not been rekeyed at this point.  To
 * make a unique label, we'll append \a key to each label to give
 * a unique name.
 */
static void
Pin_uniquify_labels_init (Init init, Key key)
{
  Expr e;
  Init i;

  if ((e = P_GetInitExpr (init)))  
    Pin_uniquify_labels_expr (e, key);
  if ((i = P_GetInitSet (init)))
    Pin_uniquify_labels_init (i, key);
  if ((i = P_GetInitNext (init)))
    Pin_uniquify_labels_init (i, key);

  return;
}


#if 0
static void
Pin_print_inline_msg (FILE * F, Stmt stmt, PinCG_Arc arc)
{
  PinCG_Node en, rn;
  char *efn, *rfn, *efi, *rfi;
  int eop, rop, rli = stmt->lineno;
  double cwt, rwt;

  en = arc->callee;
  rn = arc->caller;

  cwt = arc->weight;

  efn = en->func->funcname;
  efi = en->func->orig_filename;
  eop = en->func->o_bodysize;

  rfn = rn->func->funcname;
  rfi = stmt->filename;
  rop = rn->func->i_bodysize;
  rwt = rn->weight;

  fprintf (F, "Pcode inlining %s() [%s], %d ops\n"
	   "   callsite %s() [%s:%d] wt %0.3g/%0.3g, %d ops\n",
	   efn, efi, eop, rfn, rfi, rli, cwt, rwt, rop);

  return;
}
#endif

/*
 * ExpandCallee
 * ----------------------------------------------------------------------------
 * Expand a subroutine call
 */
static Stmt
Pin_expand_callee (PinCG_Arc expand_arc, Stmt stmt, 
		   Expr expr, Expr lvar, Expr cast)
{
  double scale_in;
  Stmt func_body, comp_stmt;
  Key inl_scope;
  PinCG_Node callee_node = expand_arc->callee;
  char *filepath;
  FuncDcl calleeFuncDcl;
  Pin_key_change keys;
  HashTable expr_map;
  PinCG_Arc cur_arc;

  if (callee_node->func->is_empty_func)
    {
      if (stmt->type == ST_EXPR)
	PSI_RemoveExpr (stmt->stmtstruct.expr);
      stmt->type = ST_NOOP;
      TotalCallWeight -= expand_arc->weight;
      return stmt;
    }

  filepath = callee_node->func->o_path;

  if (Pin_adjust_func_weight)
    {
      scale_in = (callee_node->func->weight > 0.0) ?
	expand_arc->weight / callee_node->func->weight : 1.0;
    }
  else
    {
      scale_in = (callee_node->func->weight > 0.0) ?
	callee_node->u_weight / callee_node->func->weight : 1.0;
    }

  if (!(calleeFuncDcl = \
	P_GetSymTabEntryFuncDcl \
	  (PSI_GetSymTabEntryCopyFromSource (callee_node->func->key, SO_IN))))
    P_punt ("Pin_expand_callee: Invalid key");

  inl_scope = PSI_GetExprScope (expr);

  /* grab the compound stmt of the callee function */
  func_body = PSI_CopyStmtToScope (inl_scope,
				   P_GetFuncDclStmt (calleeFuncDcl));
  expr_map = P_StmtBuildExprMap (P_GetFuncDclStmt (calleeFuncDcl), func_body);

  Pin_scale_stmts (func_body, scale_in);

  /* Update the Expr IDs in the callee's arcs. */
  List_start (expand_arc->callee->arcs);
  while ((cur_arc = (PinCG_Arc)List_next (expand_arc->callee->arcs)))
#ifdef LP64_ARCHITECTURE
    cur_arc->callsite_id = (int)((long)HashTable_find (expr_map,
						       cur_arc->callsite_id));
#else
    cur_arc->callsite_id = (int)HashTable_find (expr_map,
						cur_arc->callsite_id);
#endif

    HashTable_free (expr_map);

   /* If the callee is self-recursive, those recursive call sites were updated
   * to call the FuncDcl copy made above.  We need to change those to
   * call the original function. */
  keys.old = P_GetFuncDclKey (calleeFuncDcl);
  keys.new = expand_arc->callee->func->key;
  P_StmtApply (func_body, NULL, Pin_change_var_key, &keys);

  /* If the callee and the caller are in different files, any statics
   * used by the callee must be made externally visible. */
  /* We can't look at expand_arc->caller->func, because that may be in
   * the same file as the callee.  Have to get the current file from
   * the inlining scope. */
  if (inl_scope.file != callee_node->func->key.file)
    P_StmtApply (func_body, NULL, Pin_static2global_expr, NULL);
    
  /* Create a new compound to hold the inlined function. */
  comp_stmt = PSI_NewCompoundStmt (PSI_GetStmtScope (stmt));
  P_SetCompoundStmtList (P_GetStmtCompound (comp_stmt), func_body);
  PSI_AddEntryToScope (PSI_GetStmtScope (comp_stmt), P_GetStmtKey (func_body));

  /* Insert the compound before the call statement.  The call stmt will
   * go away, so there's no need to enclose this in another compound. */
  P_StmtInsertStmtBefore (stmt, comp_stmt);

  /* Make sure labels in the inlined body are unique. */
  Pin_uniquify_labels_stmt (func_body, P_GetStmtKey (comp_stmt));

  /* move parameters to local variables and assign actual to "formal" */
  Pin_param2lvar (calleeFuncDcl, comp_stmt);
  Pin_actual2formal (stmt, comp_stmt, expr->operands->sibling);

  /* We've extracted all we need from the callee, so we can free the
   * FuncDcl. */
  PSI_RemoveFuncDcl (calleeFuncDcl);

  {
    Stmt last_stmt, goto_stmt;
    Key scope = PSI_GetStmtScope (func_body);
    Label label = PSI_NewLabelTemp (scope, "inl_return");
    Pragma pragma;

    /* replace every return in the callee with goto */
    Pin_return2goto (func_body, lvar, label, cast);

    /* add a label to the very end of the compound stmt 
     * EMN 9/2002 - Delete any parameter shadow statements
     */
    if ((last_stmt = func_body->stmtstruct.compound->stmt_list))
      {
	Stmt next_stmt;

	for (; last_stmt->lex_next; last_stmt = next_stmt)
	  {
	    next_stmt = last_stmt->lex_next;
	    if (!last_stmt->shadow)
	      continue;
	    /* Delete shadow stmt */
	    P_StmtRemoveStmt (last_stmt);
	    PSI_RemoveStmt (last_stmt);
	  }

	goto_stmt = P_NewStmtWithType (ST_NOOP);
	goto_stmt->profile = P_CopyProfST (stmt->profile);

	P_StmtAddLabel (goto_stmt, label);
	P_StmtInsertStmtAfter (last_stmt, goto_stmt);
      }

    /* then free the current statement expression */
    P_StmtRemoveStmt (stmt);
    P_RemoveStmt (stmt);

    pragma = P_NewPragmaWithSpecExpr ("INLINE-FUNCTION",
				      P_NewStringExpr (callee_node->func->
						       funcname));
    comp_stmt->pragma = P_AppendPragmaNext (comp_stmt->pragma, pragma);
  }

  return comp_stmt;
}


Expr
Pin_func_expr (char *name, Key k, Key t)
{
  Expr e;

  if (!P_ValidKey (k))
    P_punt ("Pin_func_expr: Unable to find key for%s()", name);

  e = PSI_ScopeNewExprWithOpcode (PSI_GetGlobalScope (), OP_var);
  P_SetExprVarName (e, strdup (name));
  P_SetExprVarKey (e, k);
  P_SetExprType (e, t);

  return e;
}


/* 
 * Original code : (*func)(i,j,k); and func could be foo1
 * Transformed code :
 *	if (func == foo1) 
 *	   foo1(i,j,k);
 *	else 
 *         (*func)(i,j,k);
 */
static int
Pin_extr_indir_callee (PinCG_Arc arc, Stmt stmt, Expr exCall)
{
  Expr indr, expr, this_expr, cexpr;
  IfStmt ifStIf;
  Stmt stDirCall, stIndCall;
  Expr exCond, exCallParams, exIndCall;
  Pragma pragma, npragma;
  Key scope, callee_key = arc->callee->func->key;
  double wr_incl, wr_excl;
  char *callee_name = arc->callee->func->funcname;
  int cs_orig, cs_indir;

  if (!arc->indirect)
    P_punt ("Pin_extr_indir_callee: arc not indirect");

  expr = stmt->stmtstruct.expr;	/* OP_call || OP_assign */
  indr = exCall->operands;	/* OP_indr */

  cs_orig = exCall->id;

  scope = PSI_GetExprScope (expr);

  if (expr->opcode == OP_call)
    this_expr = expr;
  else if (expr->opcode == OP_assign)
    this_expr = expr->operands->sibling;
  else
    P_punt ("Pin_extr_indir_callee: expr neither call nor assign");

  wr_incl = (stmt->profile && (stmt->profile->count > 0.0)) ?
    (arc->weight / stmt->profile->count) : 0.5;
  if (wr_incl > 1.0)
    wr_incl = 1.0;
  wr_excl = 1.0 - wr_incl;

  /* Create branching condition */

  exCond = PSI_ScopeNewExprWithOpcode (scope, OP_eq);
  P_AppendExprOperands (exCond, PSI_CopyExprToScope (scope, indr->operands));
  P_AppendExprOperands (exCond, Pin_func_expr (callee_name, callee_key,
					       indr->type));
  PSI_CastExpr (exCond);

  /* Create copy of indirect call */

  stIndCall = P_NewStmtWithType (ST_EXPR);
#if 0
  stIndCall->filename = strdup (stmt->filename);
  stIndCall->lineno = stmt->lineno;
  stIndCall->colno = stmt->colno;
#endif
  P_SetStmtExpr (stIndCall, PSI_CopyExprToScope (scope,
						 stmt->stmtstruct.expr));
  stIndCall->profile = P_CopyProfST (stmt->profile);
  Pin_scale_stmts (stIndCall, wr_excl);

  if (!Pin_find_call (stIndCall->stmtstruct.expr, &exIndCall, NULL, NULL))
    P_punt ("ExpandIndirectCallee: Unable to find copied ind call");

  cs_indir = exIndCall->id;

  {
    List l = arc->caller->arcs;
    PinCG_Arc a;
    while ((a = (PinCG_Arc) List_next (l)))
      {
	if (a != arc && a->callsite_id == cs_orig)
	  a->callsite_id = cs_indir;
      }
  }

  for (pragma = P_FindPragma (exIndCall->pragma, PIN_PRG_IPC_PFX); pragma;
       pragma = npragma)
    {
      npragma = pragma->next;
      if (!strncmp (pragma->specifier, PIN_PRG_IPC_PFX,
		    PIN_PRG_IPC_PFX_LEN) &&
	  !strcmp (pragma->expr->value.string, callee_name))
	P_RemoveExprPragma (exIndCall, pragma);
    }

  /* Create extracted direct call */

  stDirCall = P_NewStmtWithType (ST_EXPR);
#if 0
  stDirCall->filename = strdup (stmt->filename);
  stDirCall->lineno = stmt->lineno;
  stDirCall->colno = stmt->colno;
#endif
  P_SetStmtExpr (stDirCall, stmt->stmtstruct.expr);

  stDirCall->profile = P_CopyProfST (stmt->profile);
  Pin_scale_stmts (stDirCall, wr_incl);

  while ((pragma = P_FindPragma (exCall->pragma, PIN_PRG_IPC_PFX)))
    P_RemoveExprPragma (exCall, pragma);

  stmt->stmtstruct.expr = NULL;

  /* Create and hook up if stmt (recycle original call stmt) */

  P_SetStmtType (stmt, ST_IF);
  ifStIf = P_NewIfStmt ();
  P_SetStmtIfStmt (stmt, ifStIf);

  P_SetIfStmtCondExpr (ifStIf, exCond);
  P_SetIfStmtThenBlock (ifStIf, stDirCall);
  P_SetIfStmtElseBlock (ifStIf, stIndCall);

  stmt->profile->next = P_NewProfST_w_wt (stmt->profile->count * wr_excl);

  {
    Pragma pragma = P_NewPragmaWithSpecExpr ("INDIR-CALL-EXPN", NULL);
    stmt->pragma = P_AppendPragmaNext (stmt->pragma, pragma);
  }

  /* Modify stDirCall to make a direct invocation */

  exCallParams = exCall->operands->sibling;
  exCall->operands->sibling = NULL;
  exCall->operands = PSI_RemoveExpr (exCall->operands);

  cexpr = PSI_ScopeNewExprWithOpcode (scope, OP_var);
  P_SetExprVarName (cexpr, strdup (callee_name));
  P_SetExprVarKey (cexpr, callee_key);

  P_AppendExprOperands (exCall, cexpr);
  P_AppendExprOperands (exCall, exCallParams);

  arc->indirect = 0;

  return 1;
}


/* return 1: found the matching call expr. return 0: keep searching */
static Stmt
Pin_expand_call_expr (PinCG_Arc expand_arc, Stmt stmt)
{
  Expr expr, lhs = NULL, cast = NULL;
  Stmt inlinee = NULL;

  expr = stmt->stmtstruct.expr;

  if (expr->next)
    return NULL;		/* expr list "a(),b()" won't be inlined */

  if (!expand_arc->callee)
    return NULL;

  if (!Pin_find_call (expr, &expr, &lhs, &cast))
    return NULL;

  if (expr->id != expand_arc->callsite_id)
    return NULL;

  /* Point of no return -- callee will be inlined */

  if (expand_arc->indirect)
    {
      if (expr->operands->opcode != OP_indr)
	P_punt ("Pin_expand_call_expr: Expected an indirect call.");

      /* Expand selected target under if */

      Pin_extr_indir_callee (expand_arc, stmt, expr);
      stmt = stmt->stmtstruct.ifstmt->then_block;
    }

  inlinee = Pin_expand_callee (expand_arc, stmt, expr, lhs, cast);
  return inlinee;
}


int Pin_ignore_inlined = 0;

static Stmt
Pin_expand_call (PinCG_Arc expand_arc, Stmt stmts)
{
  SerLoop serloop;
  ParLoop parloop;
  Stmt inlinee = NULL;

  for (; stmts; stmts = stmts->lex_next)
    {
      /* Skip over inline-expanded functions */
      if (P_FindPragma (stmts->pragma, "INLINE-FUNCTION"))
	{
	  if (Pin_ignore_inlined)
	    continue;
	  else
	    Pin_ignore_inlined = 1;
	}
      switch (stmts->type)
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_RETURN:
	  break;
	case ST_COMPOUND:
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.compound->stmt_list)))
	    return inlinee;
	  break;
	case ST_IF:
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.ifstmt->then_block)))
	    return inlinee;
	  if (stmts->stmtstruct.ifstmt->else_block &&
	      (inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.ifstmt->else_block)))
	    return inlinee;
	  break;
	case ST_SWITCH:
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.switchstmt->switchbody)))
	    return inlinee;
	  break;
	case ST_EXPR:
	  if ((inlinee = Pin_expand_call_expr (expand_arc, stmts)))
	    return inlinee;
	  break;
	case ST_PSTMT:
	  if ((inlinee = Pin_expand_call (expand_arc,
					  stmts->stmtstruct.pstmt->stmt)))
	    return inlinee;
	  break;
	case ST_MUTEX:
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.mutex->statement)))
	    return inlinee;
	  break;
	case ST_COBEGIN:
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.cobegin->statements)))
	    return inlinee;
	  break;
	case ST_SERLOOP:
	  serloop = stmts->stmtstruct.serloop;
	  if ((inlinee = Pin_expand_call (expand_arc, serloop->loop_body)))
	    return inlinee;
	  break;
	case ST_PARLOOP:
	  parloop = stmts->stmtstruct.parloop;
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				P_GetParLoopPrologueStmt (parloop))))
	    return inlinee;
	  break;
	case ST_BODY:
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.bodystmt->statement)))
	    return inlinee;
	  break;
	case ST_EPILOGUE:
	  if ((inlinee =
	       Pin_expand_call (expand_arc,
				stmts->stmtstruct.epiloguestmt->statement)))
	    return inlinee;
	  break;
	default:
	  P_punt ("ExpandCallStmt: Invalid statement type", stmts->type);
	}
    }
  return NULL;
}


Stmt
Pin_expand_inlined_arc (Stmt stmt, PinCG_Arc arc)
{
  Pin_ignore_inlined = (arc->caller == arc->caller->func->node);
  return Pin_expand_call (arc, stmt);
}


static double Pin_scale = 1.0;

static void
Pin_scale_lp_profile (Pragma lpp)
{
  for (; lpp; lpp = lpp->next)
    {
      if (strncmp (lpp->specifier, "iter_", 5))
	break;

      lpp->expr->value.real *= Pin_scale;
      lpp->expr->sibling->value.real *= Pin_scale;
    }

  return;
}

static void
Pin_scale_stmt (Stmt s, void *data)
{
  ProfST p;
  Pragma pr;

  for (p = s->profile; p; p = p->next)
    p->count *= Pin_scale;

  if ((pr = P_FindPragma (s->pragma, "iteration_header")))
    Pin_scale_lp_profile (pr);

  return;
}

static void
Pin_scale_expr (Expr e, void *data)
{
  ProfEXPR p;
  Pragma pr;

  for (p = e->profile; p; p = p->next)
    p->count *= Pin_scale;

  if ((pr = P_FindPragma (e->pragma, "iteration_header")))
    Pin_scale_lp_profile (pr);

  return;
}

static void
Pin_scale_stmts (Stmt s, double k)
{
  Pin_scale = k;
  P_StmtApply (s, Pin_scale_stmt, Pin_scale_expr, NULL);

  return;
}

/*! \brief Promotes variables from global statics to globals.
 *
 * \param expr
 *  the expression to inspect for variable references.
 * \param data
 *  user data (not used).
 *
 * If \a expr is an OP_var that references a global static, this function
 * removes the static qualifier from the referenced variable.
 *
 * This function is intended to be called from P_ExprApply().
 */
static void
Pin_static2global_expr (Expr expr, void *data)
{
  VarDcl v;
  FuncDcl f;

  /* The OP_var may reference a function or a variable. */

  if (expr && P_GetExprOpcode (expr) == OP_var)
    {
      if ((v = PSI_GetVarDclEntry (P_GetExprVarKey (expr))))
	P_ClrVarDclQualifier (v, VQ_STATIC);
      else if  ((f = PSI_GetFuncDclEntry (P_GetExprVarKey (expr))))
	P_ClrFuncDclQualifier (f, VQ_STATIC);
    }

  return;
}

/*! \brief Fixes recursive calls in copied FuncDcls.
 *
 * \param expr
 *  the expr to inspect.
 * \param data
 *  a pointer to a Pin_key_change structure.
 *
 * If \a expr is an OP_var that references the \a data->old, this function
 * updates the reference to \a data->new.
 */
static void
Pin_change_var_key (Expr expr, void *data)
{
  Pin_key_change *keys = (Pin_key_change *)data;

  if (P_GetExprOpcode (expr) == OP_var && \
      P_MatchKey (P_GetExprVarKey (expr), keys->old))
    P_SetExprVarKey (expr, keys->new);

  return;
}
