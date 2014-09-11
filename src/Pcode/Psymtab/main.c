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
 * \brief Annotates symbols with their symbol table key.
 *
 * \author Robert Kidd, Hong-Seok Kim, and Wen-mei Hwu
 *
 * This file defines the main (P_gen_code()) function used to perform
 * a second traversal of the Pcode (first is done in impact-edgcpfe) to
 * annotate any symbols that impact-edgcpfe is not able to.
 *
 * \note This is a poor module to use as a base for writing new Pcode
 * modules.  It accesses library structures at a lower level than the
 * typical module will require.
 */
/*****************************************************************************/

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/parms.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/query.h>
#include <Pcode/query_symtab.h>

static void annotate_stmt (SymbolTable symbol_table, Key scope, Stmt stmt);
static void annotate_expr (SymbolTable symbol_table, Key scope, Expr expr);
static void annotate_init (SymbolTable symbol_table, Key scope, Init init);

static Key find_label (SymbolTable symbol_table, Key scope, char *val);

/*! \brief The Psymtab module's main function.
 *
 * \param prog_name
 *  the module's name.
 * \param external_list
 *  command line parameters.
 * \param symbol_table
 *  the symbol table.
 * \param file_key
 *  the key of the input file.
 *
 * \return The exit status for this module.
 *
 * The main function for the Psymtab module.  This function operates on the
 * output of impact-edgcpfe, so the input is a single Pcode file with embedded
 * symbol table and header files.  The Pcode file is file 1.
 */
int
P_gen_code (char *prog_name, Parm_Macro_List * external_list,
	    SymbolTable symbol_table, int file_key)
{
  Key scope;
  ScopeEntry se;

  scope.file = file_key;
  scope.sym = 1;
  
  for (se = PST_GetScopeEntryByType (symbol_table, scope, ET_FUNC); se;
       se = PST_GetScopeEntryByTypeNext (symbol_table, se, ET_FUNC))
    {
      SymTabEntry e;
      FuncDcl f;
      
      e = PST_GetSymTabEntry (symbol_table, P_GetScopeEntryKey (se));
      f = P_GetSymTabEntryFuncDcl (e);

      annotate_stmt (symbol_table, P_GetScopeEntryKey (se),
		     P_GetFuncDclStmt (f));
    }

  return (0);
}

/*! \brief Annotates symbols in a Stmt with their key.
 *
 * \param symbol_table
 *  the symbol table.
 * \param scope
 *  the scope under which the Stmt is defined.
 * \param stmt
 *  the Stmt.
 */
static void
annotate_stmt (SymbolTable symbol_table, Key scope, Stmt stmt)
{
  while (stmt)
    {
      if (stmt == NULL)
	return;
  
      switch (P_GetStmtType (stmt))
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_MUTEX:
	  break;
	  
	case ST_RETURN:
	  annotate_expr (symbol_table, scope, P_GetStmtRet (stmt));
	  break;

	case ST_GOTO:
	  if (!P_ValidKey (P_GetStmtLabelKey (stmt)))
	    P_SetStmtLabelKey (stmt, find_label (symbol_table, scope,
						 P_GetStmtLabelVal (stmt)));
	  break;
	  
	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (stmt);
	    VarList var_list = P_GetCompoundVarList (c);
	    VarDcl var_dcl;
	    
	    /* Annotate any variables used in initializers. */
	    for (List_start (var_list), var_dcl = (VarDcl)List_next (var_list);
		 var_dcl; var_dcl = (VarDcl)List_next (var_list))
	      {
		if (P_GetVarDclInit (var_dcl))
		  annotate_init (symbol_table, P_GetStmtKey (stmt),
				 P_GetVarDclInit (var_dcl));
	      }

	    annotate_stmt (symbol_table, P_GetStmtKey (stmt),
			   P_GetCompoundStmtList (c));
	  }
	  break;
	  
	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (stmt);
	    
	    annotate_expr (symbol_table, scope, P_GetIfStmtCondExpr (i));
	    annotate_stmt (symbol_table, scope, P_GetIfStmtThenBlock (i));
	    annotate_stmt (symbol_table, scope, P_GetIfStmtElseBlock (i));
	  }
	  break;
	  
	case ST_SWITCH:
	  {
	    SwitchStmt s = P_GetStmtSwitchStmt (stmt);
	    
	    annotate_expr (symbol_table, scope, P_GetSwitchStmtExpression (s));
	    annotate_stmt (symbol_table, scope, P_GetSwitchStmtSwitchBody (s));
	  }
	  break;
	  
	case ST_PSTMT:
	  {
	    Pstmt p = P_GetStmtPstmt (stmt);
	    
	    annotate_stmt (symbol_table, scope, P_GetPstmtStmt (p));
	  }
	  break;
	  
	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (stmt);
	    
	    annotate_stmt (symbol_table, scope, P_GetCobeginStatements (c));
	  }
	  break;
	  
	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (stmt);
	    
	    annotate_stmt (symbol_table, scope,
			   P_GetPstmtStmt (P_GetParLoopPstmt (p)));
	    annotate_expr (symbol_table, scope, P_GetParLoopIterationVar (p));
	    annotate_expr (symbol_table, scope, P_GetParLoopInitValue (p));
	    annotate_expr (symbol_table, scope, P_GetParLoopFinalValue (p));
	    annotate_expr (symbol_table, scope, P_GetParLoopIncrValue (p));
	    annotate_stmt (symbol_table, scope, P_GetParLoopChild (p));
	  }
	  break;
	  
	case ST_SERLOOP:
	  {
	    SerLoop s = P_GetStmtSerLoop (stmt);
	    
	    annotate_stmt (symbol_table, scope, P_GetSerLoopLoopBody (s));
	    annotate_expr (symbol_table, scope, P_GetSerLoopCondExpr (s));
	    annotate_expr (symbol_table, scope, P_GetSerLoopInitExpr (s));
	    annotate_expr (symbol_table, scope, P_GetSerLoopIterExpr (s));
	  }
	  break;
	  
	case ST_EXPR:
	  annotate_expr (symbol_table, scope, P_GetStmtExpr (stmt));
	  break;
	  
	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (stmt);
	    
	    annotate_stmt (symbol_table, scope, P_GetBodyStmtStatement (b));
	  }
	  break;
	  
	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (stmt);
	    
	    annotate_stmt (symbol_table, scope,
			   P_GetEpilogueStmtStatement (e));
	  }
	  break;

	case ST_ASM:
	  {
	    AsmStmt a = P_GetStmtAsmStmt (stmt);
	    
	    annotate_expr (symbol_table, scope, P_GetAsmStmtAsmClobbers (a));
	    annotate_expr (symbol_table, scope, P_GetAsmStmtAsmString (a));
	    annotate_expr (symbol_table, scope, P_GetAsmStmtAsmOperands (a));
	  }
	  break;
	}

      stmt = P_GetStmtLexNext (stmt);
    }

  return;
}

/*! \brief Annotates symbols in an Expr with their key.
 *
 * \param symbol_table
 *  the symbol table.
 * \param scope
 *  the scope under which the Expr is defined.
 * \param expr
 *  the Expr.
 */
static void
annotate_expr (SymbolTable symbol_table, Key scope, Expr expr)
{
  if (expr == NULL)
    return;

  switch (P_GetExprOpcode (expr))
    {
    case OP_enum:
    case OP_int:
    case OP_real:
    case OP_error:
    case OP_char:
    case OP_string:
    case OP_expr_size:
    case OP_type_size:
    case OP_float:
    case OP_double:
    case OP_null:
    case OP_sync:
    case OP_asm_oprd:
      break;

    case OP_var:
      {
	if (!(P_ValidKey (P_GetExprVarKey (expr))))
	  {
	    P_SetExprVarKey (expr,
			     PST_ScopeFindByNameR (symbol_table, scope,
						   P_GetExprVarName (expr),
						   ET_VAR | ET_FUNC));
	  }
      }
      break;

    case OP_dot:
    case OP_arrow:
      {
	Key struct_key;
	SymTabEntry entry;
	Field f = NULL;

	annotate_expr (symbol_table, scope, P_GetExprOperands (expr));

	struct_key = PST_ExprType (symbol_table, P_GetExprOperands (expr));

	/* Dereference the pointer if this is an arrow operation. */
	if (P_GetExprOpcode (expr) == OP_arrow)
	  struct_key = PST_DereferenceType (symbol_table, struct_key);

	entry = PST_GetSymTabEntry (symbol_table, struct_key);

	/* If the type is a typedef, find the struct or union. */
	while (P_GetSymTabEntryType (entry) & ET_TYPE)
	  {
	      TypeDcl t = P_GetSymTabEntryTypeDcl (entry);

	      if (!(P_GetTypeDclBasicType (t) & \
		    (BT_TYPEDEF | BT_POINTER | BT_FUNC | BT_ARRAY | \
		     BT_STRUCT | BT_UNION)))
		P_punt ("main.c:annotate_expr:%d OP_dot/OP_arrow requires "
			"struct or union", __LINE__ - 1);

	      entry = PST_GetSymTabEntry (symbol_table, P_GetTypeDclType (t));
	  }

	switch (P_GetSymTabEntryType (entry))
	  {
	  case ET_STRUCT:
	    f = P_GetStructDclFields (P_GetSymTabEntryStructDcl (entry));
	    break;
	  case ET_UNION:
	    f = P_GetUnionDclFields (P_GetSymTabEntryUnionDcl (entry));
	    break;
	  default:
	    P_punt ("main.c:annotate_expr:%d OP_dot/OP_arrow requires struct "
		    "or union", __LINE__ - 1);
	  }

	while (f)
	  {
	    if (strcmp (P_GetFieldName (f), P_GetExprVarName (expr)) == 0)
	      {
		P_SetExprVarKey (expr, P_GetFieldKey (f));
		break;
	      }

	    f = P_GetFieldNext (f);
	  }
      }
      break;

    case OP_cast:
    case OP_neg:
    case OP_not:
    case OP_inv:
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
    case OP_indr:
    case OP_addr:
      annotate_expr (symbol_table, scope, P_GetExprOperands (expr));
      break;

    case OP_quest:
      {
	Expr e = P_GetExprOperands (expr);

	annotate_expr (symbol_table, scope, e);
	e = P_GetExprSibling (e);
	annotate_expr (symbol_table, scope, e);
	e = P_GetExprSibling (e);
	annotate_expr (symbol_table, scope, e);
      }
      break;

    case OP_disj:
    case OP_conj:
    case OP_assign:
    case OP_or:
    case OP_xor:
    case OP_and:
    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
    case OP_rshft:
    case OP_lshft:
    case OP_add:
    case OP_sub:
    case OP_mul:
    case OP_div:
    case OP_mod:
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
    case OP_index:
      annotate_expr (symbol_table, scope, P_GetExprOperands (expr));
      annotate_expr (symbol_table, scope,
		     P_GetExprSibling (P_GetExprOperands (expr)));
      break;

    case OP_compexpr:
      {
	Expr e = P_GetExprOperands (expr);

	while (e)
	  {
	    annotate_expr (symbol_table, scope, e);
	    e = P_GetExprNext (e);
	  }
      }
      break;

    case OP_call:
      {
	Expr e = P_GetExprOperands (expr);

	annotate_expr (symbol_table, scope, e);

	for (e = P_GetExprSibling (e); e; e = P_GetExprNext (e))
	  {
	    annotate_expr (symbol_table, scope, e);
	  }
      }
      break;

    case OP_stmt_expr:
      annotate_stmt (symbol_table, scope, P_GetExprStmt (expr));
      break;

    default:
      P_punt ("main.c:annotate_expr:%d Unknown opcode %d", __LINE__,
	      P_GetExprOpcode (expr));
    }

  return;
}

/*! \brief Annotates symbols in an Init with their key.
 *
 * \param symbol_table
 *  the symbol table.
 * \param scope
 *  the scope under which the Init is defined.
 * \param init
 *  the Init.
 */
static void
annotate_init (SymbolTable symbol_table, Key scope, Init init)
{
  if (init)
    {
      if (P_GetInitExpr (init))
	annotate_expr (symbol_table, scope, P_GetInitExpr (init));
      else
	annotate_init (symbol_table, scope, P_GetInitSet (init));

      annotate_init (symbol_table, scope, P_GetInitNext (init));
    }

  return;
}

/*! \brief Finds the key for a label from its string value.
 *
 * \param symbol_table
 *  the symbol table.
 * \param scope
 *  the scope under which a goto that references the label is defined.
 * \param val
 *  the label string.
 *
 * \return
 *  The symbol table key for the label.
 */
static Key
find_label (SymbolTable symbol_table, Key scope, char *val)
{
  ScopeEntry flattened_scope, se;
  SymTabEntry entry;
  Key func_scope;
  Key result = {0, 0};

  /* Find the function that contains the goto. */
  func_scope = PST_ScopeFindFuncScope (symbol_table, scope);

  /* Flatten the scope, then search for labels. */
  flattened_scope = PST_FlattenScope (symbol_table, 
				      PST_GetScope (symbol_table, func_scope));

  /* Inspect all labels in the flattened scope. */
  for (se = flattened_scope; se; se = P_GetScopeEntryNext (se))
    {
      entry = PST_GetSymTabEntry (symbol_table, P_GetScopeEntryKey (se));

      if (P_GetSymTabEntryType (entry) == ET_LABEL &&
	  strcmp (P_GetLabelVal (P_GetSymTabEntryLabel (entry)), val) == 0)
	{
	  result = P_GetSymTabEntryKey (entry);
	  break;
	}
    }

  flattened_scope = P_RemoveScopeEntry (flattened_scope);

  return (result);
}
