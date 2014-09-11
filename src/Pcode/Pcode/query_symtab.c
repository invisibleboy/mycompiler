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
 * \brief Functions to perform complex queries on Pcode structures.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * Modified from code written by: Po-hua Chang, David August, Nancy Warter,
 * Grant Haab, Krishna Subramanian
 *
 * Copyright (c) 2003 Robert Kidd, David August, Nancy Warter, Grant Haab,
 * Krishna Subramanian, Po-hua Chang, Wen-mei Hwu and The Board of
 * Trustees of the University of Illinois.
 * All rights reserved.
 *
 * Licence Agreement specifies the terms and conditions for redistribution.
 *
 * This file performs Pcode structure queries that require the symbol table.
 */
/*****************************************************************************/

#include <config.h>
#include <string.h>
#include <library/i_list.h>
#include "pcode.h"
#include "symtab.h"
#include "struct.h"
#include "struct_symtab.h"
#include "query.h"
#include "query_symtab.h"

/*! \brief Determines the result type of an Expr.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  The result Type of \a expr.  If the type cannot be determined, returns
 *  an invalid key ({0, 0}).
 *
 * Determines the result Type of an Expr.  This function assumes that
 * all symbols are annotated with their key.  This function incorporates
 * much of the logic of the old CastExpr function.
 */
Type
PST_ExprType (SymbolTable table, Expr expr)
{
  _Opcode opcode;
  Type result = Invalid_Key;

  if (expr)
    {
      opcode = P_GetExprOpcode (expr);

      switch (opcode)
	{
	case OP_enum:
	case OP_error:
	case OP_expr_size:
	case OP_type_size:
	case OP_null:
	case OP_sync:
	case OP_asm_oprd:
	  break;

	case OP_var:
	  {
	    SymTabEntry entry;
	    FuncDcl func;
	    VarDcl var;

	    if (!(P_ValidKey (P_GetExprVarKey (expr))))
	      {
		if (P_ValidKey (P_GetExprType (expr)))
		  {
		    result = P_GetExprType (expr);
		  }
		else
		  {
		    P_punt ("query_symtab.c:PST_ExprType:%d Implicit variable "
			    "should be handled by edg", __LINE__ - 1);
#if 0
		    /* Implicitly defined variables default to a type of
		     * extern int. */
		    result = PST_FindBasicType (table, BT_INT);
		    result = PST_AddTypeQual (table, result, TY_EXTERN);
#endif
		  }

		break;
	      }

	    entry = PST_GetSymTabEntry (table, P_GetExprVarKey (expr));

	    switch (P_GetSymTabEntryType (entry))
	      {
	      case ET_FUNC:
		func = P_GetSymTabEntryFuncDcl (entry);
		result = P_GetFuncDclType (func);
		break;

	      case ET_VAR_LOCAL:
	      case ET_VAR_GLOBAL:
		var = PST_GetVarDclEntry (table, P_GetExprVarKey (expr));
		result = P_GetVarDclType (var);
		break;

	      default:
		P_punt ("query_symtab.c:PST_ExprType:%d invalid SymTabEntry "
			"type %d", __LINE__ - 1, P_GetSymTabEntryType (entry));
	      }
	  }
	  break;

	case OP_int:
	case OP_real:
	case OP_float:
	case OP_double:
	case OP_char:
	case OP_string:
	  result = P_GetExprType (expr);
	  break;

	case OP_cast:
	  result = P_GetExprType (expr);
	  break;

	case OP_or:
	case OP_xor:
	case OP_and:
	  {
	    /*
	     *        According to K&R,
	     *        1) normal arithmetic conversions are performed.
	     *        2) apply only on integral operands.
	     */
	    Expr op1, op2;
	    Type type1, type2;

	    if (!(op1 = P_GetExprOperands (expr)))
	      P_punt ("query_symtab.c:PSt_ExprType:%d %s missing operand 1",
		      __LINE__ - 1, op_to_value[opcode]);
	    if (!(op2 = P_GetExprSibling (op1)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 2",
		      __LINE__ - 1, op_to_value[opcode]);

	    type1 = PST_ExprType (table, op1);
	    type2 = PST_ExprType (table, op2);

	    if (!PST_IsIntegralType (table, type1))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s requires integer "
		      "operand 1", __LINE__ - 1, op_to_value[opcode]);
	    if (!PST_IsIntegralType (table, type2))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s requires integer "
		      "operand 2", __LINE__ - 1, op_to_value[opcode]);

	    if (P_ValidKey (type1) && P_ValidKey (type2))
	      result = PST_DominantType (table, type1, type2);
	    else
	      P_punt ("query_symtab.c:PST_ExprType:%d type1 or type2 is "
		      "invalid", __LINE__ - 1);
	  }
	  break;

	case OP_eq:
	case OP_ne:
	case OP_lt:
	case OP_le:
	case OP_ge:
	case OP_gt:
	case OP_not:
	case OP_disj:
	case OP_conj:
	  result = PST_FindBasicType (table, BT_INT);
	  break;

	case OP_rshft:
	case OP_lshft:
	  {
	    /*
	     *        1) normal arithmetic conversions are performed.
	     *        2) apply only on integral types.
	     *        3) the right operand is converted to TY_INT.
	     *        4) the resultant type is the type of the left operand.
	     */
	    Expr op1;
	    Type type1;

	    if (!(op1 = P_GetExprOperands (expr)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 1",
		      __LINE__ - 1, op_to_value[opcode]);

	    type1 = PST_ExprType (table, op1);

	    if (!PST_IsIntegralType (table, type1))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s requires integer "
		      "operand 1", __LINE__ - 1, op_to_value[opcode]);

	    result = type1;
	  }
	  break;

	case OP_add:
	  {
	    /*
	     *        1) the usual arithmetic conversions are performed.
	     *        2) a pointer to an array, integral -> pointer to array.
	     */
	    Expr op1, op2;
	    Type type1, type2;
	    int ptr1, ptr2;

	    if (!(op1 = P_GetExprOperands (expr)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 1",
		      __LINE__ - 1, op_to_value[opcode]);
	    if (!(op2 = P_GetExprSibling (op1)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 2",
		      __LINE__ - 1, op_to_value[opcode]);

	    type1 = PST_ExprType (table, op1);
	    type2 = PST_ExprType (table, op2);

	    ptr1 = (PST_IsPointerType (table, type1) | \
		    PST_IsArrayType (table, type1));
	    ptr2 = (PST_IsPointerType (table, type2) | \
		    PST_IsArrayType (table, type2));

	    if (ptr1 & ptr2)
	      P_punt ("query_symtab.c:PST_ExprType:%d %s cannot add two "
		      "pointers", __LINE__ - 1, op_to_value[opcode]);

	    if (ptr1 | ptr2)    /* Adding a pointer to an integer. */
	      {
		if (ptr1)
		  result = type1;
		else
		  result = type2;
	      }
	    else                /* Adding two arithmetic values. */
	      { 
		if (!PST_IsArithmeticType (table, type1))
		  P_punt ("query_symtab.c:PST_ExprType:%d %s requires "
			  "arithmetic or\npointer operand 1", __LINE__ - 1,
			  op_to_value[opcode]);
		if (!PST_IsArithmeticType (table, type2))
		  P_punt ("query_symtab.c:PST_ExprType:%d %s requires "
			  "arithmetic or\npointer operand 2", __LINE__ - 1,
			  op_to_value[opcode]);

		result = PST_DominantType (table, type1, type2);
	      }
	  }
	  break;

	case OP_sub:
	  {
	    /*
	     *  1) the usual arithmetic conversions are performed.
	     *  2) a pointer to an array, integral -> pointer to array.
	     *  3) pointer to array - pointer to array -> TY_INT.
	     */
	    Expr op1, op2;
	    Type type1, type2;
	    int ptr1, ptr2;

	    if (!(op1 = P_GetExprOperands (expr)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 1",
		      __LINE__ - 1, op_to_value[opcode]);
	    if (!(op2 = P_GetExprSibling (op1)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 2",
		      __LINE__ - 1, op_to_value[opcode]);

	    type1 = PST_ExprType (table, op1);
	    type2 = PST_ExprType (table, op2);

	    ptr1 = (PST_IsPointerType (table, type1) | \
		    PST_IsArrayType (table, type1));
	    ptr2 = (PST_IsPointerType (table, type2) | \
		    PST_IsArrayType (table, type2));

	    if (ptr1 & ptr2)      /* subtracting two pointers. */
	      {
		result = PST_FindBasicType (table, BT_LONG);
	      }
	    else if (ptr1 | ptr2) /* subtracting an integer from a pointer. */
	      {
		if (ptr1)
		  result = type1;
		else
		  result = type2;
	      }
	    else                  /* subtracting two arithmetic values. */
	      {
		if (!PST_IsArithmeticType (table, type1))
		  P_punt ("query_symtab.c:PST_ExprType:%d %s requires "
			  "arithmetic or\npointer operand 1", __LINE__ - 1,
			  op_to_value[opcode]);
		if (!PST_IsArithmeticType (table, type2))
		  P_punt ("query_symtab.c:PST_ExprType:%d %s requires "
			  "arithmetic or\npointer operand 1", __LINE__ - 1,
			  op_to_value[opcode]);

		result = PST_DominantType (table, type1, type2);
	      }
	  }
	  break;

	case OP_mul:
	case OP_div:
	case OP_mod:
	  {
	    /*
	     *  1) usual arithmetic conversions are performed.
	     *  2) the resultant type is the type of the operand.
	     *  3) apply only to arithmetic types.
	     *  ** a%b = a - (a/b)*b
	     */
	    Expr op1, op2;
	    Type type1, type2;

	    if (!(op1 = P_GetExprOperands (expr)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 1",
		      __LINE__ - 1, op_to_value[opcode]);
	    if (!(op2 = P_GetExprSibling (op1)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 2",
		      __LINE__ - 1, op_to_value[opcode]);

	    type1 = PST_ExprType (table, op1);
	    type2 = PST_ExprType (table, op2);

	    if (!PST_IsArithmeticType (table, type1))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s requires arithmetic "
		      "operand 1", __LINE__ - 1, op_to_value[opcode]);
	    if (!PST_IsArithmeticType (table, type2))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s requires arithmetic "
		      "operand 2", __LINE__ - 1, op_to_value[opcode]);

	    result = PST_DominantType (table, type1, type2);
	  }
	  break;

	case OP_neg:
	case OP_inv:
	case OP_preinc:
	case OP_predec:
	case OP_postinc:
	case OP_postdec:
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
	  result = PST_ExprType (table, P_GetExprOperands (expr));
	  break;

	case OP_quest:
	  {
	    /*
	     *  1) if possible, the usual arithmetic conversions are
	     *          performed to bring the second and third
	     *          expressions to a common type.
	     *  2) if both are pointers of the same type, the result
	     *          has the common type.
	     *  3) if one is a pointer, and the other is 0, the result
	     *          is a pointer.
	     */
	    Expr op2, op3;
	    Type type2, type3;

	    if (!(op2 = P_GetExprSibling (P_GetExprOperands (expr))))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 2",
		      __LINE__ - 1, op_to_value[opcode]);
	    if (!(op3 = P_GetExprSibling (op2)))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s missing operand 3",
		      __LINE__ - 1, op_to_value[opcode]);

	    type2 = PST_ExprType (table, op2);
	    type3 = PST_ExprType (table, op3);

	    /* BCC - consider this case in cc1/stmt.c/exapnd_expr_stmt():
	     * expr_stmts_for_value ? 0 : const0_rtx
	     * the result type should be dominated by op3 instead of op2
	     */
	    /* PST_MatchType() determines if two types are identical.
	     * All we care is that the basic types are the same. */
	    if (PST_MatchTypeBasicType (table, type2, table, type3) || \
		PST_IsPointerType (table, type2))
	      result = type2;
	    else
	      result = type3;
	  }
	  break;

	case OP_compexpr:
	  {
	    /*
	     *  1) the resultant type is the type of the last expression.
	     */
	    Expr op;

	    for (op = P_GetExprOperands (expr); P_GetExprNext (op);
		 op = P_GetExprNext (op));

	    result = PST_ExprType (table, op);
	  }
	  break;

	case OP_dot:
	case OP_arrow:
	  {
	    Field field = NULL;
	    Type struct_type;

	    /* If the field has no key, search for its name in its parent
	     * struct. */
	    if (!P_ValidKey (P_GetExprVarKey (expr)))
	      {
		struct_type = PST_ExprType (table, P_GetExprOperands (expr));

		if (PST_IsPointerType (table, struct_type))
		  struct_type = PST_DereferenceType (table, struct_type);

		switch (PST_GetTypeBasicType (table, struct_type))
		  {
		  case BT_STRUCT:
		    {
		      StructDcl s;

		      s = \
			PST_GetStructDclEntry (table,
					       PST_GetTypeType (table,
								struct_type));

		      field = P_GetStructDclFields (s);
		    }
		    break;

		  case BT_UNION:
		    {
		      UnionDcl u;

		      u = PST_GetUnionDclEntry (table,
						PST_GetTypeType (table,
								 struct_type));

		      field = P_GetUnionDclFields (u);
		    }
		    break;

		  default:
		    P_punt ("query_symtab.c:PST_ExprType:%d Invalid basic "
			    "type 0x%x", __LINE__ - 1,
			    PST_GetTypeBasicType (table, struct_type));
		  }

		while (field)
		  {
		    if (strcmp (P_GetFieldName (field),
				P_GetExprVarName (expr)) == 0)
		      {
			result = P_GetFieldType (field);

			/* Annotate the field with its key. */
			P_SetExprVarKey (expr, P_GetFieldKey (field));
			break;
		      }

		    field = P_GetFieldNext (field);
		  }
		      
#if 0
		if ((entry = PST_GetSymTabEntry (table, struct_key)))
		  {
		    /* If the type is a typedef, find the struct or union. */
		    while (P_GetSymTabEntryType (entry) & ET_TYPE)
		      {
			TypeDcl t = P_GetSymTabEntryTypeDcl (entry);

			if (!(P_GetTypeDclBasicType (t) & \
			      (BT_TYPEDEF | BT_POINTER | BT_FUNC | BT_ARRAY | \
			       BT_STRUCT | BT_UNION)))
			  P_punt ("query_symtab.c:PST_ExprType:%d %s requires "
				  "struct or union", __LINE__ - 1,
				  op_to_value[opcode]);

			entry = PST_GetSymTabEntry (table,
						    P_GetTypeDclType (t));
		      }

		    switch (P_GetSymTabEntryType (entry))
		      {
		      case ET_STRUCT:
			{
			  StructDcl s = P_GetSymTabEntryStructDcl (entry);
			  
			  for (field = P_GetStructDclFields (s); field;
			       field = P_GetFieldNext (field))
			    {
			      if (strcmp (P_GetFieldName (field),
					  P_GetExprVarName (expr)) == 0)
				{
				  result = P_GetFieldType (field);

				  /* Annotate the field with its key. */
				  P_SetExprVarKey (expr,
						   P_GetFieldKey (field));
				  break;
				}
			    }
			}
			break;

		      case ET_UNION:
			{
			  UnionDcl u = P_GetSymTabEntryUnionDcl (entry);

			  for (field = P_GetUnionDclFields (u); field;
			       field = P_GetFieldNext (field))
			    {
			      if (strcmp (P_GetFieldName (field),
					  P_GetExprVarName (expr)) == 0)
				{
				  result = P_GetFieldType (field);

				  /* Annotate the field with its key. */
				  P_SetExprVarKey (expr,
						   P_GetFieldKey (field));
				  break;
				}
			    }
			}
			break;

		      default:
			P_punt ("query_symtab.c:PSt_ExprType:%d %s must refer "
				"to struct or union", __LINE__ - 1,
				op_to_value[opcode]);
			break;
		      }
		  }
		else
		  {
		    P_punt ("query_symtab.c:PST_ExprType:%d %s Could not "
			    "retrieve\nstruct entry", __LINE__ - 1,
			    op_to_value[opcode]);
		  }
#endif
	      }
	    else
	      {
		field = PST_GetFieldEntry (table, P_GetExprVarKey (expr));

		result = P_GetFieldType (field);
	      }
	  }
	  break;

	case OP_indr:
	case OP_index:
	case OP_call:
	  {
	    /* The result of calling a function or performing an array index
	     * operation is found in the first operand's type's type field.
	     * This is the same as dereferencing a pointer type. */
	    result = PST_ExprType (table, P_GetExprOperands (expr));

	    if (!P_ValidKey (result))
	      P_punt ("query_symtab.c:PST_ExprType:%d %s Could not find "
		      "referenced type", __LINE__ - 1, op_to_value[opcode]);

	    /* A special case: a function type can be used as a pointer to
	     * the function, so *function is the same as function. */
	    if (!(opcode == OP_indr && PST_IsFunctionType (table, result)))
	      result = PST_DereferenceType (table, result);
	  }
	  break;

	case OP_addr:
	  {
	    Type base_type;

	    base_type = PST_ExprType (table, P_GetExprOperands (expr));
	    result = PST_FindPointerToType (table, base_type);
	  }
	  break;

	case OP_stmt_expr:
	  {
	    /* The type of a statement expression is the type of the expression
	     * at the end. */
	    Stmt s = P_GetExprStmt (expr);

	    if (P_GetStmtType (s) != ST_COMPOUND)
	      P_punt ("query_symtab.c:PST_ExprType:%d Statement expression "
		      "must be a compound", __LINE__ - 1);

	    /* Find the last stmt in the compound. */
	    for (s = P_GetCompoundStmtList (P_GetStmtCompound (s));
		 P_GetStmtLexNext (s); s = P_GetStmtLexNext (s));

	    if (P_GetStmtType (s) == ST_EXPR)
	      result = PST_ExprType (table, P_GetStmtExpr (s));
	    else
	      result = PST_FindBasicType (table, BT_VOID);
	  }
	  break;

	default:
	  P_punt ("query_symtab.c:PST_ExprType:%d Unknown Expr opcode %d",
		  __LINE__ - 1, opcode);
	}
    }

  return (result);
}

/*! \brief Returns the Type resulting from an operation on two Types.
 *
 * \param table
 *  the symbol table containing type \a a.
 * \param a, b
 *  the Types to compare.
 *
 * \return
 *  The dominant Type out of \a a or \a b.  This may be \a a or
 *  \a b, or a new Type.
 *
 * If an arithmetic operation is applied to two operands of different types,
 * the result will be the dominant type of the two.
 */
Type
PST_DominantType (SymbolTable table, Type a, Type b)
{
  _BasicType btA, btB;
  Type result;

  btA = PST_GetTypeBasicType (table, a);
  btB = PST_GetTypeBasicType (table, b);

  if (!(btA & (BT_ARITHMETIC | BT_POINTER)))
    P_punt ("query_symtab.c:PST_DominantType:%d Type a (%d, %d) is not "
	    "arithmetic", __LINE__ - 1, a.file, a.sym);
  if (!(btB & (BT_ARITHMETIC | BT_POINTER)))
    P_punt ("query_symtab.c:PST_DominantType:%d Type b (%d, %d) is not "
	    "arithmetic", __LINE__ - 1, b.file, b.sym);

  if (btA & BT_POINTER)
    {
      result = a;
      goto result_known;
    }
  if (btB & BT_POINTER)
    {
      result = b;
      goto result_known;
    }

  if (btA & BT_LONGDOUBLE)
    {
      result = a;
      goto result_known;
    }
  if (btB & BT_LONGDOUBLE)
    {
      result = b;
      goto result_known;
    }

  if ((btA & BT_LONGLONG) || (btB & BT_LONGLONG))
    {
      if ((btA & BT_LONGLONG) && ((btA & BT_UNSIGNED) || !(btB & BT_UNSIGNED)))
	{
	  result = a;
	}
      else if (((btB & BT_LONGLONG) && (btB & BT_UNSIGNED)) || \
	       (!(btA & BT_LONGLONG) && !(btA & BT_UNSIGNED)))
	{
	  result = b;
	}
      else
	{
	  result = PST_FindBasicType (table, BT_LONGLONG | BT_UNSIGNED);
	}

      goto result_known;
    }

  if (btA & BT_DOUBLE)
    {
      result = a;
      goto result_known;
    }
  if (btB & BT_DOUBLE)
    {
      result = b;
      goto result_known;
    }

  if (btA & BT_FLOAT)
    {
      result = a;
      goto result_known;
    }
  if (btB & BT_FLOAT)
    {
      result = b;
      goto result_known;
    }

  if ((btA & BT_LONG) || (btB & BT_LONG))
    {
      if ((btA & BT_LONG) && ((btA & BT_UNSIGNED) || !(btB & BT_UNSIGNED)))
	{
	  result = a;
	}
      else if (((btB & BT_LONG) && (btB & BT_UNSIGNED)) || \
	       (!(btA & BT_LONG) && !(btA & BT_UNSIGNED)))
	{
	  result = b;
	}
      else
	{
	  result = PST_FindBasicType (table, BT_LONG | BT_UNSIGNED);
	}

      goto result_known;
    }

  if ((btA & BT_INT) && ((btA & BT_UNSIGNED) || !(btB & BT_UNSIGNED)))
    {
      result = a;
    }
  else if (((btB & BT_INT) && (btB & BT_UNSIGNED)) || \
	   (!(btA & BT_INT) && !(btA & BT_UNSIGNED)))
    {
      result = b;
    }
  else
    {
      result = PST_FindBasicType (table, BT_INT | BT_UNSIGNED);
    }

 result_known:
  return (result);
}

/*! \brief If FuncDcls match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains FuncDcl \a a.
 * \param a
 *  the first FuncDcl to compare
 * \param tableB
 *  the symbol table that contains FuncDcl \a b.
 * \param b
 *  the second FuncDcl to compare.
 *
 * \return
 *  If the FuncDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa #PST_MatchFuncDclK(), PST_MatchTypeDcl(), #PST_MatchType(),
 * PST_MatchStructDcl(), #PST_MatchStructDclK(), PST_MatchUnionDcl(),
 * #PST_MatchUnionDclK(), PST_MatchField(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchFuncDcl (SymbolTable tableA, FuncDcl a, SymbolTable tableB, FuncDcl b)
{
  char *nameA, *nameB;

  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  if ((nameA = P_GetFuncDclName (a)) == NULL)
    P_punt ("query_symtab.c:PST_MatchFuncDcl:%d FuncDcl a has no name",
	    __LINE__ - 1);
  if ((nameB = P_GetFuncDclName (b)) == NULL)
    P_punt ("query_symtab.c:PST_MatchFuncDcl:%d FuncDcl b has no name",
	    __LINE__ - 1);

  if (strcmp (nameA, nameB) != 0)
    return (FALSE);

  if (PST_MatchTypeDcl (tableA,
			PST_GetTypeDclEntry (tableA, P_GetFuncDclType (a)),
			tableB,
			PST_GetTypeDclEntry (tableB,
					     P_GetFuncDclType (b))) == FALSE)
    return (FALSE);

  nameA = P_GetFuncDclFilename (a);
  nameB = P_GetFuncDclFilename (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  if (PST_MatchVarList (tableA, P_GetFuncDclParam (a),
			tableB, P_GetFuncDclParam (b)) == 0)
    return (FALSE);

  return (TRUE);
}

/*! \brief If TypeDcls match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains TypeDcl \a a.
 * \param a
 *  the first TypeDcl to compare.
 * \param tableB
 *  the symbol table that contains TypeDcl \a b.
 * \param b
 *  the second TypeDcl to compare.
 *
 * \return
 *  If the TypeDcls match, returns TRUE.  Otherwise, return FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), #PST_MatchType(),
 * PST_MatchStructDcl(), #PST_MatchStructDclK(), PST_MatchUnionDcl(),
 * #PST_MatchUnionDclK(), PST_MatchField(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchTypeDcl (SymbolTable tableA, TypeDcl a, SymbolTable tableB, TypeDcl b)
{
  char *nameA, *nameB;

  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  if (P_GetTypeDclBasicType (a) != P_GetTypeDclBasicType (b))
    return (FALSE);

  if (P_GetTypeDclQualifier (a) != P_GetTypeDclQualifier (b))
    return (FALSE);

  if (P_ValidKey (P_GetTypeDclType (a)) != P_ValidKey (P_GetTypeDclType (b)))
    return (FALSE);

  if (P_GetTypeDclBasicType (a) == BT_STRUCT)
    {
      if (PST_MatchStructDcl \
	    (tableA, PST_GetStructDclEntry (tableA, P_GetTypeDclType (a)),
	     tableB, PST_GetStructDclEntry (tableB,
					    P_GetTypeDclType (b))) == 0)
	return (FALSE);
    }
  else if (P_GetTypeDclBasicType (a) == BT_UNION)
    {
      if (PST_MatchUnionDcl \
	    (tableA, PST_GetUnionDclEntry (tableA, P_GetTypeDclType (a)),
	     tableB, PST_GetUnionDclEntry (tableB, P_GetTypeDclType (b))) == 0)
	return (FALSE);
    }
  else if (P_ValidKey (P_GetTypeDclType (a)) && \
	   P_ValidKey (P_GetTypeDclType (b)) && \
	   PST_MatchTypeDcl (tableA,
			     PST_GetTypeDclEntry (tableA,
						  P_GetTypeDclType (a)),
			     tableB,
			     PST_GetTypeDclEntry (tableB,
						  P_GetTypeDclType (b))) == 0)
    {
      return (FALSE);
    }

  nameA = P_GetTypeDclName (a);
  nameB = P_GetTypeDclName (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  /* For an array type, compare the array_size expression. */
  if (P_GetTypeDclBasicType (a) == BT_ARRAY && \
      PST_MatchExpr (tableA, P_GetTypeDclArraySize (a),
		     tableB, P_GetTypeDclArraySize (b)) == 0)
    {
      return (FALSE);
    }

  /* For a function type, compare the parameters. */
  if (P_GetTypeDclBasicType (a) == BT_FUNC)
    {
      if (PST_MatchParam (tableA, P_GetTypeDclParam (a),
			  tableB, P_GetTypeDclParam (b)) == 0)
	return (FALSE);
    }

  if (P_GetTypeDclSize (a) != P_GetTypeDclSize (b))
    return (FALSE);

  if (P_GetTypeDclAlignment (a) != P_GetTypeDclAlignment (b))
    return (FALSE);

  nameA = P_GetTypeDclFilename (a);
  nameB = P_GetTypeDclFilename (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  return (TRUE);
}

/*! \brief If Types match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains the first Type.
 * \param a
 *  the first Type to compare.
 * \param tableB
 *  the symbol table that contains the second Type.
 * \param b
 *  the second Type to compare.
 *
 * \return
 *  If the Types match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * PST_MatchStructDcl(), #PST_MatchStructDclK(), PST_MatchUnionDcl(),
 * #PST_MatchUnionDclK(), PST_MatchField(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchType (SymbolTable tableA, Type a, SymbolTable tableB, Type b)
{
  char *nameA, *nameB;
  _BasicType btA, btB;
  Type base_typeA, base_typeB;

  if (tableA == tableB && P_MatchKey (a, b))
    return (TRUE);
  
  if (!P_ValidKey (a) ^ !P_ValidKey (b))
    return (FALSE);

  btA = PST_GetTypeBasicType (tableA, a);
  btB = PST_GetTypeBasicType (tableB, b);

  if (btA != btB)
    return (FALSE);

  if (PST_GetTypeQualifier (tableA, a) != PST_GetTypeQualifier (tableB, b))
    return (FALSE);

  base_typeA = PST_GetTypeType (tableA, a);
  base_typeB = PST_GetTypeType (tableB, b);

  if (P_ValidKey (base_typeA) != P_ValidKey (base_typeB))
    return (FALSE);

  if (btA == BT_STRUCT)
    {
      if (PST_MatchStructDcl (tableA,
			      PST_GetStructDclEntry (tableA, base_typeA),
			      tableB,
			      PST_GetStructDclEntry (tableB, base_typeB)) == 0)
	return (FALSE);
    }
  else if (btA == BT_UNION)
    {
      if (PST_MatchUnionDcl (tableA,
			     PST_GetUnionDclEntry (tableA, base_typeA),
			     tableB,
			     PST_GetUnionDclEntry (tableB, base_typeB)) == 0)
	return (FALSE);
    }
  else if (P_ValidKey (base_typeA) && P_ValidKey (base_typeB) && \
	   PST_MatchType (tableA, base_typeA, tableB, base_typeB) == 0)
    {
      return (FALSE);
    }

  nameA = PST_GetTypeName (tableA, a);
  nameB = PST_GetTypeName (tableB, b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  /* For an array type, compare the array_size expression. */
  if (btA == BT_ARRAY && \
      PST_MatchExpr (tableA, PST_GetTypeArraySize (tableA, a),
		     tableB, PST_GetTypeArraySize (tableB, b)) == 0)
    return (FALSE);

  /* For a function type, compare the parameters. */
  if (btA == BT_FUNC && \
      PST_MatchParam (tableA, PST_GetTypeParam (tableA, a),
		      tableB, PST_GetTypeParam (tableB, b)) == 0)
    return (FALSE);

  if (PST_GetTypeSize (tableA, a) != PST_GetTypeSize (tableB, b))
    return (FALSE);

  if (PST_GetTypeAlignment (tableA, a) != PST_GetTypeAlignment (tableB, b))
    return (FALSE);

  nameA = PST_GetTypeFilename (tableA, a);
  nameB = PST_GetTypeFilename (tableB, b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  return (TRUE);
}

/*! \brief If StructDcls match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains StructDcl \a a.
 * \param a
 *  the first StructDcl to compare.
 * \param tableB
 *  the symbol table that contains StructDcl \a b.
 * \param b
 *  the second StructDcl to compare.
 *
 * \return
 *  If the StructDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchType(), #PST_MatchStructDclK(), PST_MatchUnionDcl(),
 * #PST_MatchUnionDclK(), PST_MatchField(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchStructDcl (SymbolTable tableA, StructDcl a,
		    SymbolTable tableB, StructDcl b)
{
  Field fieldA, fieldB;
  char *nameA, *nameB;

  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  if ((P_GetStructDclQualifier (a) & SQ_COMPARING) || \
      (P_GetStructDclQualifier (b) & SQ_COMPARING))
    {
      if ((P_GetStructDclQualifier (a) & SQ_COMPARING) && \
	  (P_GetStructDclQualifier (b) & SQ_COMPARING))
	return (TRUE);
      else
	return (FALSE);
    }

  nameA = P_GetStructDclName (a);
  nameB = P_GetStructDclName (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  nameA = P_GetStructDclFilename (a);
  nameB = P_GetStructDclFilename (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  if (P_GetStructDclQualifier (a) != P_GetStructDclQualifier (b))
    return (FALSE);

  if (P_GetStructDclSize (a) != P_GetStructDclSize (b))
    return (FALSE);

  if (P_GetStructDclAlign (a) != P_GetStructDclAlign (b))
    return (FALSE);

  P_SetStructDclQualifier (a, SQ_COMPARING);
  P_SetStructDclQualifier (b, SQ_COMPARING);

  for (fieldA = P_GetStructDclFields (a), fieldB = P_GetStructDclFields (b);
       fieldA && fieldB;
       fieldA = P_GetFieldNext (fieldA), fieldB = P_GetFieldNext (fieldB))
    {
      if (PST_MatchField (tableA, fieldA, tableB, fieldB) == 0)
	{
	  P_ClrStructDclQualifier (a, SQ_COMPARING);
	  P_ClrStructDclQualifier (b, SQ_COMPARING);

	  return (FALSE);
	}
    }

  P_ClrStructDclQualifier (a, SQ_COMPARING);
  P_ClrStructDclQualifier (b, SQ_COMPARING);

  if (fieldA || fieldB)
    return (FALSE);

  return (TRUE);
}

/*! \brief If UnionDcls match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains UnionDcl \a a.
 * \param a
 *  the first UnionDcl to compare.
 * \param tableB
 *  the symbol table that contains UnionDcl \a b.
 * \param b
 *  the second UnionDcl to compare.
 *
 * \return
 *  If the UnionDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchType(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * #PST_MatchUnionDclK(), PST_MatchField(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchUnionDcl (SymbolTable tableA, UnionDcl a,
		   SymbolTable tableB, UnionDcl b)
{
  Field fieldA, fieldB;
  char *nameA, *nameB;

  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  if ((P_GetUnionDclQualifier (a) & SQ_COMPARING) || \
      (P_GetUnionDclQualifier (b) & SQ_COMPARING))
    {
      if ((P_GetUnionDclQualifier (a) & SQ_COMPARING) && \
	  (P_GetUnionDclQualifier (b) & SQ_COMPARING))
	return (TRUE);
      else
	return (FALSE);
    }

  nameA = P_GetUnionDclName (a);
  nameB = P_GetUnionDclName (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  nameA = P_GetUnionDclFilename (a);
  nameB = P_GetUnionDclFilename (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  if (P_GetUnionDclSize (a) != P_GetUnionDclSize (b))
    return (FALSE);

  if (P_GetUnionDclAlign (a) != P_GetUnionDclAlign (b))
    return (FALSE);

  P_SetUnionDclQualifier (a, SQ_COMPARING);
  P_SetUnionDclQualifier (b, SQ_COMPARING);

  for (fieldA = P_GetUnionDclFields (a), fieldB = P_GetUnionDclFields (b);
       fieldA && fieldB;
       fieldA = P_GetFieldNext (fieldA), fieldB = P_GetFieldNext (fieldB))
    {
      if (PST_MatchField (tableA, fieldA, tableB, fieldB) == 0)
	{
	  P_ClrUnionDclQualifier (a, SQ_COMPARING);
	  P_ClrUnionDclQualifier (b, SQ_COMPARING);

	  return (FALSE);
	}
    }

  P_ClrUnionDclQualifier (a, SQ_COMPARING);
  P_ClrUnionDclQualifier (b, SQ_COMPARING);

  if (fieldA || fieldB)
    return (FALSE);

  return (TRUE);
}

/*! \brief If Fields match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains Field \a a.
 * \param a
 *  the first Field to compare.
 * \param tableB
 *  the symbol table that contains Field \a b.
 * \param b
 *  the second Field to compare.
 *
 * \return 
 *  If the Fields match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchType(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), #PST_MatchUnionDclK(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchField (SymbolTable tableA, Field a, SymbolTable tableB, Field b)
{
  char *nameA, *nameB;

  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  nameA = P_GetFieldName (a);
  nameB = P_GetFieldName (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  if (PST_MatchTypeDcl (tableA,
			PST_GetTypeDclEntry (tableA, P_GetFieldType (a)),
			tableB,
			PST_GetTypeDclEntry (tableB, P_GetFieldType (b))) == 0)
    return (FALSE);


  if (P_GetFieldOffset (a) != P_GetFieldOffset (b))
    return (FALSE);

  return (TRUE);
}

/*! \brief If VarLists match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains VarList \a a.
 * \param a
 *  the first VarList to compare.
 * \param tableB
 *  the symbol table that contains VarList \a b.
 * \param b
 *  the second VarList to compare.
 *
 * \return
 *  If the VarLists match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchType(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), #PST_MatchUnionDclK(), PST_MatchField(),
 * #PST_MatchFieldK(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchVarList (SymbolTable tableA, VarList a, SymbolTable tableB, VarList b)
{
  VarDcl varA, varB;

  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  for (List_start (a), varA = (VarDcl)List_next (a), \
	 List_start (b), varB = (VarDcl)List_next (b); varA && varB;
       varA = (VarDcl)List_next (a), varB = (VarDcl)List_next (b))
    {
      if (PST_MatchVarDcl (tableA, varA, tableB, varB) == 0)
	return (FALSE);
    }

  if (a || b)
    return (FALSE);

  return (TRUE);
}

/*! \brief If VarDcls match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains VarDcl \a a.
 * \param a
 *  the first VarDcl to compare.
 * \param tableB
 *  the symbol table that contains VarDcl \a b.
 * \param b
 *  the second VarDcl to compare.
 *
 * \return
 *  If the VarDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchType(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), #PST_MatchUnionDclK(), PST_MatchField(),
 * #PST_MatchFieldK(), PST_MatchVarList(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchVarDcl (SymbolTable tableA, VarDcl a, SymbolTable tableB, VarDcl b)
{
  char *nameA, *nameB;

  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  nameA = P_GetVarDclName (a);
  nameB = P_GetVarDclName (b);

  if (((nameA == NULL) ^ (nameB == NULL)) || \
      (nameA && nameB && strcmp (nameA, nameB) != 0))
    return (FALSE);

  if (PST_MatchTypeDcl (tableA,
			PST_GetTypeDclEntry (tableA, P_GetVarDclType (a)),
			tableB,
			PST_GetTypeDclEntry (tableB,
					     P_GetVarDclType (b))) == 0)
    return (FALSE);

  if (P_GetVarDclAlign (a) != P_GetVarDclAlign (b))
    return (FALSE);

  if (P_GetVarDclQualifier (a) != P_GetVarDclQualifier (b))
    return (FALSE);

  if (P_GetVarDclFilename (a) && P_GetVarDclFilename (b) && \
      strcmp (P_GetVarDclFilename (a), P_GetVarDclFilename (b)) != 0)
    return (FALSE);

  return (FALSE);
}

/*! \brief If Exprs match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains entries used by Expr \a a.
 * \param a
 *  the first Expr to compare.
 * \param tableB
 *  the symbol table that contains entries used by Expr \a b.
 * \param b
 *  the second Expr to compare.
 *
 * \return
 *  If the Exprs match, returns TRUE.  Otherwise, returns FALSE.
 *
 * This function is used mainly to decide if two arrays have the same
 * array_size expression.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchType(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), #PST_MatchUnionDclK(), PST_MatchField(),
 * #PST_MatchFieldK(), PST_MatchVarList(), PST_MatchVarDcl(),
 * #PST_MatchVarDclK(), PST_MatchParam(), #P_MatchKey() */
bool
PST_MatchExpr (SymbolTable tableA, Expr a, SymbolTable tableB, Expr b)
{
  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  if (P_GetExprOpcode (a) != P_GetExprOpcode (b))
    return (FALSE);

  switch (P_GetExprOpcode (a))
    {
    case OP_var:
    case OP_enum:
    case OP_error:
    case OP_dot:
    case OP_arrow:
    case OP_cast:
    case OP_expr_size:
    case OP_type_size:
    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
    case OP_rshft:
    case OP_lshft:
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
    case OP_indr:
    case OP_addr:
    case OP_index:
    case OP_call:
    case OP_null:
    case OP_sync:
    case OP_stmt_expr:
    case OP_asm_oprd:
      return (FALSE);
      break;
      
    case OP_int:
      /* Does no special conversion for unsigned values. */
      if (P_GetExprScalar (a) != P_GetExprScalar (b))
	return (FALSE);
      break;

    case OP_real:
    case OP_float:
    case OP_double:
      if (P_GetExprReal (a) != P_GetExprReal (b))
	return (FALSE);
      break;

    case OP_char:
    case OP_string:
      if (strcmp (P_GetExprString (a), P_GetExprString (b)) != 0)
	return (FALSE);
      break;

    case OP_quest:
      {
	Expr operandA1 = P_GetExprSibling (P_GetExprOperands (a));
	Expr operandA2 = P_GetExprSibling (operandA1);
	Expr operandB1 = P_GetExprSibling (P_GetExprOperands (b));
	Expr operandB2 = P_GetExprSibling (operandB1);

	if (PST_MatchExpr (tableA, operandA1, tableB, operandB1) == 0 || \
	    PST_MatchExpr (tableA, operandA2, tableB, operandB2) == 0)
	  return (FALSE);
      }
      break;

    case OP_disj:
    case OP_conj:
    case OP_or:
    case OP_xor:
    case OP_and:
    case OP_add:
    case OP_sub:
    case OP_mul:
    case OP_div:
    case OP_mod:
      {
	Expr operandA0 = P_GetExprOperands (a);
	Expr operandA1 = P_GetExprSibling (operandA0);
	Expr operandB0 = P_GetExprOperands (b);
	Expr operandB1 = P_GetExprSibling (operandB0);

	if (PST_MatchExpr (tableA, operandA0, tableB, operandB0) == 0 || \
	    PST_MatchExpr (tableA, operandA1, tableB, operandB1) == 0)
	  return (FALSE);
      }
      break;

    case OP_compexpr:
      {
	Expr lastA = P_GetExprNext (P_GetExprOperands (a));
	Expr lastB = P_GetExprNext (P_GetExprOperands (b));

	if (PST_MatchExpr (tableA, lastA, tableB, lastB) == 0)
	  return (FALSE);
      }
      break;

    case OP_assign:
      if (PST_MatchExpr (tableA, P_GetExprSibling (P_GetExprOperands (a)),
			 tableB,
			 P_GetExprSibling (P_GetExprOperands (b))) == 0)
	return (FALSE);
      break;

    case OP_neg:
    case OP_not:
    case OP_inv:
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
      if (PST_MatchExpr (tableA, P_GetExprOperands (a),
			 tableB, P_GetExprOperands (b)) == 0)
	return (FALSE);
      break;

    default:
      P_punt ("query_symtab.c:PST_MatchExpr:%d Unknown opcode %d", __LINE__,
	      P_GetExprOpcode (a));
    }

  return (TRUE);
}

/*! \brief If Params match, returns TRUE.
 *
 * \param tableA
 *  the symbol table that contains Param \a a.
 * \param a
 *  the first Param to compare.
 * \param tableB
 *  the symbol table that contains Param \a b.
 * \param b
 *  the second Param to compare.
 *
 * \return
 *  If the Params match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchType(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), #PST_MatchUnionDclK(), PST_MatchField(),
 * #PST_MatchFieldK(), PST_MatchVarList(), PST_MatchVarDcl(),
 * #PST_MatchVarDclK(), PST_MatchExpr(), #P_MatchKey() */
bool
PST_MatchParam (SymbolTable tableA, Param a, SymbolTable tableB, Param b)
{
  if (a == NULL || b == NULL)
    {
      if (a == NULL && b == NULL)
	return (TRUE);
      else
	return (FALSE);
    }

  while (a && b)
    {
      TypeDcl typeA = PST_GetTypeDclEntry (tableA, P_GetParamKey (a));
      TypeDcl typeB = PST_GetTypeDclEntry (tableB, P_GetParamKey (b));

      if (PST_MatchTypeDcl (tableA, typeA, tableB, typeB) == 0)
	return (FALSE);

      a = P_GetParamNext (a);
      b = P_GetParamNext (b);
    }

  if (a || b)
    return (FALSE);

  return (TRUE);
}

/*! \brief Checks if a Type is void.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect
 *
 * \return
 *  If the Type is void, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsVoidTypeDcl(), #PST_IsVoidTypeExpr()
 */
bool
PST_IsVoidType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_VOID))
    return (FALSE);
  if (bt & ~BT_VOID)
    P_punt ("query_symtab.c:PST_IsVoidType:%d illegal void type 0x%x",
	    __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Checks if a Type is integral.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is integral, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsIntegralTypeDcl(), #PST_IsIntegralTypeExpr()
 */
bool 
PST_IsIntegralType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!TYPE_INTEGRAL (bt))
    return (FALSE);
  if (bt & ~(BT_INTEGRAL | BT_BIT_FIELD))
    P_punt ("query_symtab.c:PST_IsIntegralType:%d illegal integral type 0x%x",
	    __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Checks if a Type is real.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is real, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsRealTypeDcl(), #P_IsRealTypeExpr()
 */
bool
PST_IsRealType (SymbolTable table, Type type)
{
  _BasicType bt;
  
  if (!P_ValidKey (type))
    return (FALSE);
  
  bt = PST_GetTypeBasicType (table, type);
  
  if (!TYPE_REAL (bt) & BT_TYPE)
    return (FALSE);
  if (bt & ~BT_REAL)
    P_punt ("query_symtab.c:PST_IsRealType:%d illegal real type 0x%x",
	    __LINE__ - 1, bt);
  
  return (TRUE);
}

/*! \brief Checks if a Type is a float.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is a float, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsFloatTypeDcl(), #PST_IsFloatTypeExpr()
 */
bool
PST_IsFloatType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_FLOAT))
    return (FALSE);
  if (bt & ~BT_FLOAT)
    P_punt ("query_symtab.c:PST_IsFloatType:%d illegal float type 0x%x",
	    __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Checks if a Type is a double.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is a double, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsDoubleTypeDcl(), #PST_IsDoubleTypeExpr()
 */
bool
PST_IsDoubleType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_DOUBLE))
    return (FALSE);
  if (bt & ~BT_DOUBLE)
    P_punt ("query_symtab.c:PST_IsDoubleType:%d illegal double type 0x%x",
	    __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Checks if a Type is a longlong.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is a longlong, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsLongLongTypeDcl(), #PST_IsLongLongTypeExpr()
 */
bool
PST_IsLongLongType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & (BT_LONGLONG)))
    return (FALSE);
  if (bt & ~(BT_LONGLONG | BT_UNSIGNED | BT_INT))
    P_punt ("query_symtab.c:PST_IsLongLongType:%d illegal longlong type 0x%x 0x%x",
	    __LINE__ - 1, bt, BT_LONGLONG | BT_UNSIGNED);

  return (TRUE);
}

/*! \brief Checks if a Type is arithmetic.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is arithmetic, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsArithmeticTypeDcl(), #PST_IsArithmeticTypeExpr()
 */
bool 
PST_IsArithmeticType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_ARITHMETIC))
    return (FALSE);
  if (bt & ~BT_ARITHMETIC)
    P_punt ("query_symtab.c:PST_IsArithmeticType:%d illegal arithmetic type "
	    "0x%x", __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Check if a Type is a pointer.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is a pointer, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsPointerTypeDcl(), #PST_IsPointerTypeExpr()
 */
bool 
PST_IsPointerType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_POINTER))
    return (FALSE);
  if (bt & ~BT_POINTER)
    P_punt ("query_symtab.c:P_IsPointerTypeDcl:%d illegal pointer type 0x%x",
	    __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Check if a Type is fundamental.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is fundamental, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsFundamentalTypeDcl(), #PST_IsFundamentalTypeExpr()
 */
bool
PST_IsFundamentalType (SymbolTable table, Type type)
{
  if (!P_ValidKey (type))
    return (FALSE);
  
  return (PST_IsPointerType (table, type) || PST_IsArrayType (table, type) || \
	  PST_IsFunctionType (table, type) || \
	  PST_IsArithmeticType (table, type));
}

/*! \brief Check if a Type is a structure.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is an aggregate (struct or union), returns TRUE. 
 *  Otherwise, returns FALSE.
 *
 * \sa P_IsStructureTypeDcl(), #PST_IsStructureTypeExpr()
 */
bool 
PST_IsStructureType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_STRUCTURE))
    return (FALSE);
  if (bt & ~BT_STRUCTURE)
    P_punt ("query_symtab.c:PSt_IsStructureType:%d illegal structure type "
	    "0x%x", __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Checks if a Type is an array.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is an array,  returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsArrayTypeDcl(), #PST_IsArrayTypeExpr()
 */
bool 
PST_IsArrayType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_ARRAY))
    return (FALSE);
  if (bt & ~BT_ARRAY)
    P_punt ("query_symtab.c:PST_IsArrayType:%d illegal array type 0x%x",
	    __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Checks if a Type is a function.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is a function, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsFunctionTypeDcl(), #PST_IsFunctionTypeExpr()
 */
bool 
PST_IsFunctionType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_FUNC))
    return (FALSE);
  if (bt & ~BT_FUNC)
    P_punt ("query_symtab.c:PST_IsFunctionTypel:%d illegal function type 0x%x",
	    __LINE__ - 1, bt);

  return (TRUE);
}

/*! \brief Checks if a Type is signed.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is signed, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsSignedTypeDcl(), #PST_IsSignedExpr()
 */
bool
PST_IsSignedType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if ((bt & BT_INTEGRAL) && !(bt & BT_UNSIGNED))
    return (TRUE);

  return (FALSE);
}

/*! \brief Checks if a Type is unsigned.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is unsigned, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsUnsignedTypeDcl(), #PST_IsUnsignedExpr()
 */
bool
PST_IsUnsignedType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if ((bt & BT_INTEGRAL) && (bt & BT_UNSIGNED))
    return (TRUE);

  return (FALSE);
}

/*! \brief Checks if a Type is an enum.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is an enum, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsEnumTypeDcl(), #PST_IsEnumTypeExpr()
 */
bool
PST_IsEnumType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_ENUM))
    return (FALSE);
  if (bt & ~BT_ENUM)
    P_punt ("query_symtab.c:PST_IsEnumType:%d illegal type 0x%x", __LINE__,
	    bt);

  return (TRUE);
}

/*! \brief Checks if a Type is a vararg.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  If the Type is a vararg, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa P_IsVarargTypeDcl(), #PST_IsVarargTypeExpr()
 */
bool
PST_IsVarargType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (!(bt & BT_VARARG))
    return (FALSE);
  if (bt & ~BT_VARARG)
    P_punt ("query_symtab.c:PST_IsEnumType:%d illegal type 0x%x", __LINE__,
	    bt);

  return (TRUE);
}

/*! \brief Checks if a expr is an bit field access.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expression to inspect.
 *
 * \return
 *  If the expression is an bit field access, returns TRUE.  Otherwise,
 *  returns FALSE.
 */
bool
PST_IsBitFieldExpr (SymbolTable table, Expr expr)
{
  Field field;

  if (expr->opcode != OP_dot &&
      expr->opcode != OP_arrow)
    return (FALSE);

  field = PST_GetFieldEntry (table, P_GetExprVarKey (expr));
  if (field->is_bit_field)
    return (TRUE);

  return (FALSE);
}

/*! \brief Tests whether or not a Type is a base Type
 *         (i.e. not a pointer, func, array).
 *    
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  Returns TRUE if the Type is a base type.  Otherwise, returns FALSE.
 *
 * Iteratively inspects the typedef's type and returns TRUE if the type
 * is not a pointer, func, or array.
 *
 * \sa PST_ReduceTypedefs (), PST_GetTypeDclQualifier(),
 * PST_ReduceImplicitTypedefs(), PST_GetBaseType() */
bool
PST_IsBaseType (SymbolTable table, Type type)
{
  _BasicType bt;

  if (!P_ValidKey (type))
    return (FALSE);

  bt = PST_GetTypeBasicType (table, type);

  if (bt & (BT_POINTER | BT_FUNC | BT_ARRAY))
    return (FALSE);

  return (TRUE);
}

/*! \brief Determines if two Types are compatible.
 *
 * \param table
 *  the symbol table.
 * \param a, b
 *  the Types to compare.
 *
 * \return
 *  Returns TRUE if the two Types are compatible (same strength).  Otherwise,
 *  returns FALSE.
 *
 * This function works only for arithmetic Type.
 *
 * \sa P_EqualStrength()
 */
bool
PST_EqualStrengthType (SymbolTable table, Type a, Type b)
{
  if (!PST_IsArithmeticType (table, a) || !PST_IsArithmeticType (table, b))
    P_punt ("query_symtab.c:PST_EqualStrengthType:%d types must be arithmetic",
	    __LINE__ - 1);

  if (PST_IsIntegralType (table, a))
    {
      /* fixed-point */
      int t1, t2;

      if (!PST_IsIntegralType (table, b))
	return (FALSE);

      t1 = PST_GetTypeBasicType (table, a) & BT_INTEGRAL;
      t2 = PST_GetTypeBasicType (table, b) & BT_INTEGRAL;

      if (((t1 & BT_LONGLONG) ^ (t2 & BT_LONGLONG)) || \
	  ((t1 & BT_LONG) ^ (t2 & BT_LONG)) || \
	  ((t1 & BT_SHORT) ^ (t2 & BT_SHORT)) || \
	  ((t1 & BT_CHAR) ^ (t2 & BT_CHAR)))
	return (FALSE);

#ifdef UNSIGNED_MATTERS
      if ((t1 & BT_UNSIGNED) ^ (t2 & BT_UNSIGNED))
	return (FALSE);
#endif
    }
  else
    {
      /* floating-point */
      if (!PST_IsRealType (table, b))
	return (FALSE);

      if (BT_REAL & \
	  (PST_GetTypeBasicType (table, a) ^ PST_GetTypeBasicType (table, b)))
	return (FALSE);
    }

  return (TRUE);
}

int
PST_GetFieldContainerOffset (SymbolTable pst, Field field)
{
  int offset = field->offset;

  if (field->is_bit_field)
    {
      int mask = PST_GetTypeSize (pst, field->type) - 1;

      offset &= ~mask;
    }

  return offset;
}
