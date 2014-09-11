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
/*! \file
 * \brief Functions to validate callsites.
 *
 * \author Robert Kidd and Wen-mei Hwu
 *
 * This file contains function definitions to validate function callsites
 * to make sure functions are called with the correct number of arguments.
 */

#include <config.h>
#include <string.h>
#include <library/i_list.h>
#include <library/i_hashl.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/query.h>
#include <Pcode/symtab.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/query_symtab.h>
#include <Pcode/flatten.h>
#include "validate.h"
#include "data.h"
#include "main.h"

static void validate_init (SymbolTable ip_table, Init init, Type type);
static void validate_call (Expr e, void *data);
static void validate_return (Stmt s, void *data);
static Expr make_struct_pointer_multi (SymbolTable ip_table, Type multi_type,
				       Expr e);
static Expr make_dot (SymbolTable ip_table, Expr base, Type base_type,
		      Key scope_key);
static void remove_args (SymbolTable ip_table, Expr call_expr, Expr extra_arg);
static void add_args (SymbolTable ip_table, Expr call_expr, KeyList types);
static void fix_multi_ref (Expr e, void *data);
static bool is_multi_type (SymbolTable ip_table, Type multi_type);
static bool is_pointer_type (SymbolTable ip_table, Type t);
static Key get_multi_type_field_by_scope (SymbolTable ip_table,
					  Type multi_type, Key scope_key,
					  char **field_name);
static Key get_multi_type_field_for_call (SymbolTable ip_table,
					  Type multi_type, Expr call,
					  char **field_name);
static Key get_multi_type_first_field (SymbolTable ip_table, Type multi_type,
				       char **field_name);

/*! Constants to use for the mode argument to the validation functions. */
#define FIND   0
#define REPAIR 1

typedef struct validate_arg
{
  SymbolTable ip_table;         /*!< the interprocedural symbol table. */
  bool must_flatten;            /*!< if transformation necessitates flattening,
				 *   this will be TRUE. */
} validate_arg;

/*! \brief Validates arguments to function calls.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * Scans all functions to find callsites.  Validates call arguments abainst
 * the function arguments to make sure they match in number and type.
 *
 * If there is a discrepancy in number, extra arguments are pulled out and
 * assigned to temporary variables.  Missing arguments are filled in with empty
 * temporary variables.
 *
 * This function also inspects ST_RETURN statements to make sure they return
 * a type corresponding to their prototype.
 */
void
Plink_ValidateCalls (SymbolTable ip_table)
{
  FuncDcl f;
  Key k;
  validate_arg arg;

  for (k = PST_GetTableEntryByType (ip_table, ET_FUNC); P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k, ET_FUNC))
    {
      f = PST_GetFuncDclEntry (ip_table, k);

      arg.ip_table = ip_table;
      arg.must_flatten = FALSE;
      P_StmtApplyPost (P_GetFuncDclStmt (f), validate_return, validate_call,
		       &arg);

      if (arg.must_flatten)
	{
	  PFT_Flatten (ip_table, f);
	  Plink_SetFuncDclFlags (f, PL_FD_FLATTENED);
	}
    }

  return;
}

/*! \brief Validates struct uses.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * Pointers to structs with the same name but different definitions have been
 * merged into unions containing pointers to each struct type.  We now need
 * to search for OP_arrow and OP_dot expressions that reference these unions
 * and add an OP_dot so they get the correct pointer out of the union.
 */
void
Plink_ValidateTypes (SymbolTable ip_table)
{
  SymTabEntry e;
  Key k;

  for (k = PST_GetTableEntryByType (ip_table, ET_FUNC | ET_VAR);
       P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k, ET_FUNC | ET_VAR))
    {
      e = PST_GetSymTabEntry (ip_table, k);

      if (P_GetSymTabEntryType (e) == ET_FUNC)
	{
	  FuncDcl f = P_GetSymTabEntryFuncDcl (e);
      
	  P_StmtApplyPost (P_GetFuncDclStmt (f), NULL, fix_multi_ref,
			   ip_table);
	}
      else
	{
	  VarDcl v = P_GetSymTabEntryVarDcl (e);

	  validate_init (ip_table, P_GetVarDclInit (v), P_GetVarDclType (v));
	}
    }

  return;
}

/*! \brief Validates a variable initializer.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param init
 *  the Init to validate.
 * \param type
 *  the Type of the variable being initialized.
 *
 * If a variable used to be declared:
 *
 * struct *s = (struct *)0;
 *
 * it is now declared
 *
 * union s = (struct *)0;
 *
 * And should be declared
 *
 * union s = { (struct *)0 };
 *
 * This function does this transformation.  It recurses into aggregate
 * initializers to find multi unions hidden inside.
 */
static void
validate_init (SymbolTable ip_table, Init init, Type type)
{
  Init i;
  StructDcl s;
  UnionDcl u;
  Field f;

  if (init)
    {
      if (is_multi_type (ip_table, type))
	{
	  /* Copy init's fields to i and set i as init->set. */
	  i = P_NewInit ();
	  P_SetInitExpr (i, P_GetInitExpr (init));
	  P_SetInitNext (i, P_GetInitNext (init));
	  P_SetInitSet (i, P_GetInitSet (init));
	  
	  P_SetInitExpr (init, NULL);
	  P_SetInitNext (init, NULL);
	  P_SetInitSet (init, i);
	}
      else if (PST_IsStructureType (ip_table, type))
	{
	  if (PST_GetTypeBasicType (ip_table, type) & BT_STRUCT)
	    {
	      s = PST_GetTypeStructDcl (ip_table, type);
	      
	      for (f = P_GetStructDclFields (s), i = P_GetInitSet (init);
		   f && i; f = P_GetFieldNext (f), i = P_GetInitNext (i))
		validate_init (ip_table, i, P_GetFieldType (f));
	    }
	  else
	    {
	      u = PST_GetTypeUnionDcl (ip_table, type);
	      
	      /* Only the first field in the union can be initialized. */
	      validate_init (ip_table, P_GetInitSet (init),
			     P_GetFieldType (P_GetUnionDclFields (u)));
	    }
	}
    }

  return;
}

/*! \brief Validates a function call.
 *
 * \param e
 *  the Expr to inspect.
 * \param data
 *  a pointer to a validate_arg struct.
 *
 * Validates the arguments to a function call against the called
 * function's param types.  If a parameter is of a multi type union,
 * the correct field is selected.  If the call has too many or too few
 * arguments, this is corrected
 */
static void
validate_call (Expr e, void *data)
{
  validate_arg *arg = (validate_arg *)data;
  Expr callee_expr, cur_arg, next_arg;
  FuncDcl func;
  Key func_scope_key;
  Type func_type, param_type, cur_arg_type;
  Param param;

  if (e && P_GetExprOpcode (e) == OP_call)
    {
      /* Get the callee function's type. */
      callee_expr = P_GetExprOperands (e);
      func_type = PST_ExprType (arg->ip_table, P_GetExprOperands (e));

      /* We can only validate direct calls of defined functions. */
      if (P_IsIndirectFunctionCall (e))
        {
	  func_scope_key = PST_GetScopeFromEntryKey (arg->ip_table, func_type);

	  /* If we have an indirect call with unspecified parameters, we
	   * can't do anything more. */
	  if (PST_GetTypeParam (arg->ip_table, func_type) == NULL)
	    return;
	}
      else if (P_IsDirectFunctionCall (e))
	{
	  func = PST_GetFuncDclEntry (arg->ip_table,
				      P_GetExprVarKey (callee_expr));
	  
	  if (!P_TstFuncDclQualifier (func, VQ_DEFINED) || \
	      P_TstFuncDclQualifier (func, VQ_APP_ELLIPSIS))
	    goto done;

	  func_scope_key = \
	    PST_GetScopeFromEntryKey (arg->ip_table,
				      P_GetExprVarKey (callee_expr));
	}
      
      if (!PST_IsFunctionType (arg->ip_table, func_type))
	P_punt ("validate.c:validate_call:%d e does not result in a\n"
		"function type (%d, %d)", __LINE__, func_type.file,
		func_type.sym);
      
      /* Loop through the call arguments and the function parameters to
       * determine if we need to do anything. */
      cur_arg = P_GetExprSibling (callee_expr);
      param = PST_GetTypeParam (arg->ip_table, func_type);
      while (cur_arg && param)
	{
	  bool arg_is_multi, arg_is_pointer, param_is_multi, param_is_pointer;

	  next_arg = P_GetExprNext (cur_arg);

	  param_type = P_GetParamKey (param);
	  cur_arg_type = PST_ExprType (arg->ip_table, cur_arg);

	  /* If the current param is a vararg, the call does not require
	   * repair. */
	  if (PST_IsVarargType (arg->ip_table, param_type))
	    goto done;

	  arg_is_multi = is_multi_type (arg->ip_table, cur_arg_type);
	  arg_is_pointer = is_pointer_type (arg->ip_table, cur_arg_type);
	  param_is_multi = is_multi_type (arg->ip_table, param_type);
	  param_is_pointer = is_pointer_type (arg->ip_table, param_type);
	  
	  if (param_is_pointer && arg_is_pointer)
	    {
	      /* Build (t.field1 = cur_arg, t).field2 */
	      TypeDcl td = PST_GetTypeTypeDcl (arg->ip_table, param_type);
	      Type multi_type = Plink_GetTypeDclMultiType (td);

	      cur_arg = make_struct_pointer_multi (arg->ip_table, multi_type,
						   cur_arg);
	      cur_arg = make_dot (arg->ip_table, cur_arg, multi_type,
				  func_scope_key);

	      arg->must_flatten = TRUE;
	    }
	  else if (param_is_pointer && arg_is_multi)
	    {
	      /* Build (arg.field). */
	      cur_arg = make_dot (arg->ip_table, cur_arg, cur_arg_type,
				  func_scope_key);

	      arg->must_flatten = TRUE;
	    }
	  else if (param_is_multi && !arg_is_multi)
	    {
	      /* Build (t.field = cur_arg, t) */
	      make_struct_pointer_multi (arg->ip_table, param_type, cur_arg);

	      arg->must_flatten = TRUE;
	    }
	  else
	    {
	      /* If the argument type doesn't match the parameter type, cast
	       * it. */
	      if (PST_IsPointerType (arg->ip_table, param_type) && \
		  PST_IsIntegralType (arg->ip_table, cur_arg_type))
		{
		  Key scope_key = PST_GetExprScope (arg->ip_table, cur_arg);
		  Expr cast = PST_ScopeNewExprWithOpcode (arg->ip_table,
							  scope_key, OP_cast);
		  
		  P_ExprSwap (&cur_arg, &cast);
		  P_AppendExprOperands (cast, cur_arg);
		  PST_SetExprType (arg->ip_table, cast, param_type);
		}
	    }

	  cur_arg = next_arg;
	  param = P_GetParamNext (param);
	}
      
      if (cur_arg || param)
	{
          if (cur_arg)
	    {
	      /* If there are too many arguments, we need to remove extra ones
	       * from the end. */
	      e->pragma = \
		P_AppendPragmaNext (e->pragma,
				    P_NewPragmaWithSpecExpr ("PLV_REMOVE",
							     NULL));
	      remove_args (arg->ip_table, e, cur_arg);

	      arg->must_flatten = TRUE;
	    }
	  else if (param)
	    {
	      /* If there are too few arguments, we need to add extras to pad
	       * the call. */
	      e->pragma = \
		P_AppendPragmaNext (e->pragma,
				    P_NewPragmaWithSpecExpr ("PLV_ADD", NULL));
	      add_args (arg->ip_table, e, param);
	    }
	}
    }

 done:
  return;
}

/*! \brief Validates a return statement.
 *
 * \param s
 *  the Stmt to validate.
 * \param data
 *  a pointer to a validate_arg struct.
 *
 * If \a s is a return statement, this function validates the expression
 * returned to make sure its type is as specified in the function's prototype.
 */
static void
validate_return (Stmt s, void *data)
{
  validate_arg *arg = (validate_arg *)data;
  FuncDcl parent_func;
  Type return_type, expr_type;
  bool return_is_multi, return_is_pointer, expr_is_multi, expr_is_pointer;
  Expr ret;

  if (P_GetStmtType (s) == ST_RETURN && (ret = P_GetStmtRet (s)))
    {
      /* Find the return's parent FuncDcl. */
      parent_func = PST_GetStmtParentFunc (arg->ip_table, s);

      return_type = PST_GetTypeType (arg->ip_table,
				     P_GetFuncDclType (parent_func));
      expr_type = PST_ExprType (arg->ip_table, ret);

      return_is_multi = is_multi_type (arg->ip_table, return_type);
      return_is_pointer = is_pointer_type (arg->ip_table, return_type);
      expr_is_multi = is_multi_type (arg->ip_table, expr_type);
      expr_is_pointer = is_pointer_type (arg->ip_table, expr_type);

      if (return_is_multi && !expr_is_multi)
	{
	  /* Build (t.field = cur_arg, t) */
	  make_struct_pointer_multi (arg->ip_table, return_type, ret);
	  
	  arg->must_flatten = TRUE;
	}
      else if (return_is_multi && expr_is_pointer)
	{
	  /* Build (arg.field). */
	  ret = make_dot (arg->ip_table, ret, expr_type,
			  PST_GetFuncDclScope (arg->ip_table, parent_func));
	  
	  arg->must_flatten = TRUE;
	}
    }

  return;
}

/*! \brief Adds a temporary variable to receive a struct pointer.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the type of the temporary variable to create.
 * \param e
 *  the expression to assign to the temporary variable.
 *
 * \return
 *  A pointer to a new compound expression.
 *
 * \a e is an Expr that is used as a struct pointer type.  \a multi_type
 * is the key of a multi type union that contains the pointer type.  This
 * function creates a temporary variable t of type \a multi_type and
 * inserts a compound expression of the form (t.field = e, t) and returns
 * a pointer to this new compound expression.
 *
 * If \a e is not a known struct pointer (such as an array of structs or
 * (void *)0), this function tries to find the most appropriate field
 * of the union, and if that fails, simply uses the first field.
 */
static Expr
make_struct_pointer_multi (SymbolTable ip_table, Type multi_type, Expr e)
{
  Key scope_key = PST_GetExprScope (ip_table, e);
  Expr compexpr, tmp_var_def, tmp_var_ref, assign, dot, parent_expr;
  Key field_key;
  char *field_name;

  compexpr = PST_ScopeNewExprWithOpcode (ip_table, scope_key, OP_compexpr);
  P_ExprSwap (&e, &compexpr);

  tmp_var_def = PST_NewLocalVarExprTemp (ip_table, scope_key, multi_type,
					 "PLV");
  tmp_var_ref = PST_CopyExprToScope (ip_table, scope_key, tmp_var_def);

  field_key = get_multi_type_field_by_scope (ip_table, multi_type, scope_key,
					     &field_name);

  /* If e is an argument to a function call, look at the corresponding
   * argument type to the function. */
  if (!P_ValidKey (field_key) && (parent_expr = P_GetExprParentExpr (e)) && \
      P_GetExprOpcode (parent_expr) == OP_call)
    field_key = get_multi_type_field_for_call (ip_table, multi_type,
					       parent_expr, &field_name);
      
  /* If we still haven't found the right pointer type, just use the
   * first one. */
  if (!P_ValidKey (field_key))
    field_key = get_multi_type_first_field (ip_table, multi_type, &field_name);

  dot = PST_ScopeNewExprWithOpcode (ip_table, scope_key, OP_dot);
  P_AppendExprOperands (dot, tmp_var_def);
  P_SetExprVarKey (dot, field_key);
  P_SetExprVarName (dot, field_name);

  assign = PST_ScopeNewExprWithOpcode (ip_table, scope_key, OP_assign);
  P_AppendExprOperands (assign, dot);
  P_AppendExprOperands (assign, e);

  P_AppendExprOperands (compexpr, assign);
  P_AppendExprNext (P_GetExprOperands (compexpr), tmp_var_ref);

  return (compexpr);
}

/*! \brief Makes an OP_dot to access the appropriate field of a multi type
 *         union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param base
 *  the expression to add the dot to.
 * \param base_type
 *  the type that results from evaluating \a base.
 * \param scope_key
 *  the key of the scope for which we want to specialize the union.
 *
 * \return
 *  A pointer to the new OP_dot expression.  This expression will be swapped
 *  with \a base in the parse tree.  If \a scope_key indicates that an
 *  incomplete struct definition is sufficient in this scope, no OP_dot
 *  is added, and \a base is returned.
 *
 * This function builds an OP_dot to specialize a mulit type union for a
 * given scope.  It swaps the OP_dot with \a base in the parse tree and
 * returns a pointer to the OP_dot.  If an incomplete struct definition is
 * sufficient in the given scope, no OP_dot is added and a pointer to \a base
 * is returned.
 */
static Expr
make_dot (SymbolTable ip_table, Expr base, Type base_type, Key scope_key)
{
  Expr dot;
  Key field_key;
  char *field_name;

  field_key = get_multi_type_field_by_scope (ip_table, base_type, scope_key,
					     &field_name);

  if (!P_ValidKey (field_key))
    field_key = get_multi_type_first_field (ip_table, base_type, &field_name);

  dot = PST_ScopeNewExprWithOpcode (ip_table,
				    PST_GetExprScope (ip_table, base),
				    OP_dot);
	  
  P_ExprSwap (&base, &dot);
  P_SetExprOperands (dot, base);
  P_SetExprVarKey (dot, field_key);
  P_SetExprVarName (dot, field_name);

  return (dot);
}

/*! \brief Removes arguments from a function call.
 *
 * \param ip_table
 *  The interprocedural symbol table.
 * \param call_expr
 *  the OP_call expression containing too many arguments.
 * \param extra_arg
 *  the first argument to the function that must be removed.
 *
 * Removes \a extra_arg and all subsequent arguments from the function call.
 * \a extra_arg must exist somewhere in \a call_expr's argument list
 * (call_expr->operands->sibling[->next]*).  Extra arguments are moved
 * above the OP_call so they are still evaluated for side effects.
 *
 * void foo (a);
 * foo (a, b, c) becomes
 * (t1 = a, b, c, foo (t1))
 *
 * The function is then flattened.
 */
static void
remove_args (SymbolTable ip_table, Expr call_expr, Expr extra_arg)
{
  Expr callee, compexpr, cur_arg, next_arg, tmp_var_def, tmp_var_ref, assign;
  Key scope_key = PST_GetExprScope (ip_table, call_expr);
  /* As long as pass_arg is true, we'll add the temporary variable
   * as an argument to the call.  As soon as we find extra_arg in
   * the argument list, we'll change pass_arg to FALSE and stop adding
   * arguments to the call. */
  bool pass_arg = TRUE;

  callee = P_GetExprOperands (call_expr);

  /* Create a compound expression and swap it in place of the call. */
  compexpr = PST_ScopeNewExprWithOpcode (ip_table, scope_key, OP_compexpr);
  P_ExprSwap (&call_expr, &compexpr);

  cur_arg = P_GetExprSibling (callee);
  P_SetExprSibling (callee, NULL);

  while (cur_arg)
    {
      next_arg = P_GetExprNext (cur_arg);
      P_SetExprNext (cur_arg, NULL);

      if (cur_arg == extra_arg)
	pass_arg = FALSE;

      if (pass_arg)
	{
	  tmp_var_def = PST_NewLocalVarExprTemp (ip_table, scope_key,
						 PST_ExprType (ip_table,
							       cur_arg),
						 "PLV");
	  tmp_var_ref = PST_CopyExprToScope (ip_table, scope_key, tmp_var_def);
	  assign = PST_ScopeNewExprWithOpcode (ip_table, scope_key, OP_assign);
	  P_AppendExprOperands (assign, tmp_var_def);
	  P_AppendExprOperands (assign, cur_arg);
	  P_SetExprOperands (compexpr,
			     P_AppendExprNext (P_GetExprOperands (compexpr),
					       assign));
	  P_SetExprSibling (callee,
			    P_AppendExprNext (P_GetExprSibling (callee),
					      tmp_var_ref));
	}
      else
	{
	  P_SetExprOperands (compexpr,
			     P_AppendExprNext (P_GetExprOperands (compexpr),
					       cur_arg));
	}

      cur_arg = next_arg;
    }


  P_SetExprOperands (compexpr, P_AppendExprNext (P_GetExprOperands (compexpr),
						 call_expr));
  
  return;
}

/*! \brief Adds empty arguments to a function call.
 *
 * \param ip_table
 *  the IP Symbol Table.
 * \param call_expr
 *  the OP_call expression containing too many arguments.
 * \param types
 *  a list of types of the missing arguments.
 *
 * Adds an empty argument of the appropriate type for each missing argument.
 *
 * foo (a) becomes
 * foo (a, 0, 0)
 */
static void
add_args (SymbolTable ip_table, Expr call_expr, KeyList types)
{
  Expr arg_list = P_GetExprSibling (P_GetExprOperands (call_expr));
  Key scope_key = PST_GetExprScope (ip_table, call_expr);
  Expr new_arg;
  VarDcl var;
  Type cur_type;
  Init init;

  while (types)
    {
      cur_type = P_GetKeyListKey (types);

      /* We don't need to add any more arguments if this is the varargs
       * ellipsis. */
      if (PST_IsVarargType (ip_table, cur_type))
	break;

      new_arg = PST_NewLocalVarExprTemp (ip_table, scope_key, cur_type, "PLV");
      var = PST_GetVarDclEntry (ip_table, P_GetExprVarKey (new_arg));

      /* Add the new temp variable to the argument list. */
      if (arg_list == NULL)
	P_SetExprSibling (P_GetExprOperands (call_expr),
			  P_AppendExprNext (arg_list, new_arg));
      else
	P_AppendExprNext (arg_list, new_arg);

      init = P_NewInit ();

      /* Add an appropriate initializer for the variable. */
      if (PST_IsIntegralType (ip_table, cur_type) || \
	  PST_IsPointerType (ip_table, cur_type))
	P_SetInitExpr (init, PST_ScopeNewIntExpr (ip_table, scope_key, 0));
      else if (PST_IsRealType (ip_table, cur_type))
	P_SetInitExpr (init, PST_ScopeNewFloatExpr (ip_table, scope_key, 0.0));
      else if (PST_IsStructureType (ip_table, cur_type))
	{
	  Init set = P_NewInit ();
	  P_SetInitExpr (set, PST_ScopeNewIntExpr (ip_table, scope_key, 0));
	  P_SetInitSet (init, set);
	}

      P_SetVarDclInit (var, init);

      types = P_GetKeyListNext (types);
    }
  
  return;
}

/*! \brief Fixes references to struct pointers that have become unions.
 *
 * \param e
 *  the Expr to process.
 * \param data
 *  used to pass in the interprocedural symbol table.
 *
 * This function is indended to be applied postorder.
 *
 * If \a e is a OP_arrow or OP_dot that now refers to a union that was
 * created to hold pointers to different structs with the same name,
 * add an OP_dot to get the correct pointer out of the union.
 *
 * When we inserted the union, types that were previously struct pointers
 * became unions.  This corrupted the Expr tree as far as determining
 * expression type, as we might now be treating a union as a union pointer.
 *
 * This function processes all children first, so by the time we look at
 * \a e, the Expr tree below \a e is mostly valid.  
 */
static void
fix_multi_ref (Expr e, void *data)
{
  SymbolTable ip_table = (SymbolTable)data;
  Key expr_type, scope_key;

  if (e)
    {
      scope_key = PST_GetExprScope (ip_table, e);

      expr_type = PST_ExprType (ip_table, e);

      if (is_multi_type (ip_table, expr_type) && \
	  !(PST_GetTypeQualifier (ip_table, expr_type) & TY_CONST))
	e = make_dot (ip_table, e, expr_type, scope_key);
    }

  return;
}

/*! \brief Determines if a type is a multi type union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the Type to test.
 *
 * \return
 *  If \a multi_type is a multi type union, returns TRUE.  Otherwise, returns
 *  FALSE.
 *
 * This function assumes that keys in \a ip_table have been updated.  This
 * is why this function is static to this file instead of in merge.c
 * with the rest of the multi type functions.
 *
 * In this context, a multi type is a union of struct pointers, and a
 * pointer type is a struct pointer contained in a multi type.
 *
 * \sa is_pointer_type()
 */
static bool
is_multi_type (SymbolTable ip_table, Type multi_type)
{
  UnionDcl u;

  if (PST_GetTypeBasicType (ip_table, multi_type) == BT_UNION && \
      (u = PST_GetTypeUnionDcl (ip_table, multi_type)) && \
      Plink_GetUnionDclMultiHash (u) != NULL)
    return (TRUE);

  return (FALSE);
}

/*! \brief Determines if a type is a field of a multi type union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param t
 *  the Type to inspect.
 *
 * \return
 *  If \a t is a member of a multi type union, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * This function assumes that keys in \a ip_table have been updated.  This
 * is why this function is static to this file instead of in merge.c
 * with the rest of the multi type functions.
 *
 * In this context, a multi type is a union of struct pointers, and a
 * pointer type is a struct pointer contained in a multi type.
 *
 * \sa is_multi_type()
 */
static bool
is_pointer_type (SymbolTable ip_table, Type t)
{
  TypeDcl td = PST_GetTypeTypeDcl (ip_table, t);
  bool result = FALSE;

  if ((P_GetTypeDclBasicType (td) == BT_POINTER) && \
      (PST_GetTypeBasicType (ip_table, P_GetTypeDclType (td)) & \
       (BT_STRUCT | BT_UNION)))
    result = P_ValidKey (Plink_GetTypeDclMultiType (td));

  return (result);
}

/*! \brief Returns the key of the field of the multi type union valid in a
 *         scope.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the multi type union.
 * \param scope_key
 *  the scope to find a valid field.
 * \param field_name
 *  returns the name of the field.
 *
 * \return
 *  If the a field of the union is valid in \a scope_key, the key of the
 *  field is returned.  Otherwise, Invalid_Key is returned.  If a valid
 *  key is returned, the field's name is returned in \a field_name.
 *  It is the caller's responsibility to free the string returned in
 *  \a field_name.
 *
 * This function attempts to find the field in a multi type union that
 * is valid in a given scope.
 *
 * This function assumes that keys in \a ip_table have been updated.  This
 * is why this function is static to this file instead of in merge.c
 * with the rest of the multi type functions.
 *
 * \note It is the caller's responsibility to free the string returned in
 * \     a field_name.
 */
static Key
get_multi_type_field_by_scope (SymbolTable ip_table, Type multi_type,
			       Key scope_key, char **field_name)
{
  Type result = Invalid_Key, *type_p = NULL;
  UnionDcl u;
  HashLTable h;
  Field f;

  u = PST_GetTypeUnionDcl (ip_table, multi_type);
  h = Plink_GetUnionDclMultiHash (u);

  while (type_p == NULL || !P_ValidKey (*type_p))
    {
      type_p = (Type *)HashLTable_find_or_null (h, P_Key2Long (scope_key));

      if (P_MatchKey (scope_key, global_scope_key))
	break;

      scope_key = PST_GetScopeFromEntryKey (ip_table, scope_key);
    }

  if (type_p && P_ValidKey (*type_p))
    {    
      for (f = P_GetUnionDclFields (u); f; f = P_GetFieldNext (f))
	{
	  if (P_MatchKey (P_GetFieldType (f), *type_p))
	    {
	      result = P_GetFieldKey (f);
	      if (field_name)
		*field_name = strdup (P_GetFieldName (f));
	      break;
	    }
	}
    }

  return (result);
}

/*! \brief Returns the key of the field in a multi type union corresponding
 *         to a function's argument.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the multi type union.
 * \param call
 *  the call expression.
 * \param field_name
 *  returns the name of the field.
 *
 * \return
 *  The key of the field in the multi type union that corresponds to the
 *  struct pointer type used by the called function.  Invalid_Key is
 *  returned if no field can be found.  If a valid key is returned,
 *  \a field_name is set to the name of the field.  It is the caller's
 *  responsibility to free the string returned in \a field_name.
 *
 * This function attempts to find the field of the multi type union
 * that is valid in the called function's scope.
 *
 * This function assumes that keys in \a ip_table have been updated.  This
 * is why this function is static to this file instead of in merge.c
 * with the rest of the multi type functions.
 *
 * \note It is the caller's responsibility to free the string returned in
 *       \a field_name.
 */
static Key
get_multi_type_field_for_call (SymbolTable ip_table, Type multi_type,
			       Expr call, char **field_name)
{
  Key func_key, func_scope_key, field_key = Invalid_Key;

  if (P_IsDirectFunctionCall (call))
    {
      func_key = P_GetExprVarKey (P_GetExprOperands (call));
      func_scope_key = PST_GetScopeFromEntryKey (ip_table, func_key);
      field_key = get_multi_type_field_by_scope (ip_table, multi_type,
						 func_scope_key, field_name);
    }

  return (field_key);
}

/*! \brief Returns the key of the first field of a multi type union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the multi type union.
 * \param field_name
 *  returns the name of the field.
 *
 * \return
 *  The key of the first field of the multi type union.  Returns the field
 *  name in \a field_name.  It is the caller's responsibility to free the
 *  string returned in \a field_name.
 *
 * As a fallback when we can't find the correct field of a union to accept
 * a struct pointer expression, we just use the first one.  This function
 * should only be used when all other options have been exhausted.
 *
 * This function assumes that keys in \a ip_table have been updated.  This
 * is why this function is static to this file instead of in merge.c
 * with the rest of the multi type functions.
 *
 * \note It is the caller's responsibility to free the string returned in
 *       \a field_name.
 */
static Key
get_multi_type_first_field (SymbolTable ip_table, Type multi_type,
			    char **field_name)
{
  UnionDcl u = PST_GetTypeUnionDcl (ip_table, multi_type);
  Key field_key;

  field_key = P_GetFieldKey (P_GetUnionDclFields (u));
  if (field_name)
    *field_name = strdup (P_GetFieldName (P_GetUnionDclFields (u)));

  return (field_key);
}
