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
 * \brief Functions to cast expressions.
 *
 * \author David August, Nancy Warter, and Wen-mei Hwu
 *
 * Modified from code written by: Po-hua Chang
 *
 * Copyright (c) 1991 David August, Nancy Warter, Po-hua Chnag, Wen-mei Hwu
 * and The Board of Trustees of the University of Illinois.
 * All rights reserved.
 *
 * The University of Illinois software License Agreement specifies the terms
 * and conditions for redistribution.
 *
 * This file contains functions that access the symbol table to cast
 * expressions.
 *
 * NJW - this file provides both functions to determine the type
 * of a variable and also the cast of an expression.  I'm not sure
 * we want to reduce or simplify expressions in pcode (unless specifically
 * requested by a transformation) since this will all be done in 
 * hcode.  However, some of these functions are useful and should be
 * usable as is.
 *
 * Structure assignment is allowed.
 * e.g. struct _buf x, y;
 * 	x = y; 		is allowed.
 * 	x = \&y;	is also allowed.
 * When the LHS of an assignment is a structure, it copies
 * sizeof(structure) bytes from the memory location specified
 * by the RHS operand.
 *
 * A function may return a structure. It actually returns
 * a pointer to the structure (struct/union/array).
 * When structure (struct/union), array, and function are
 * passed to another function as parameters, their memory
 * location (pointers to them) are passed instead.
 *
 * The constant literals can actually be modified by
 * the user (bad practice). Therefore, it may be wrong
 * to assume they are all constants. For example, some
 * programs actually rewrite the string content (string I/O).
 * When a constant string is passed as parameter, it may be
 * changed. In this version of the C compiler, it is
 * assumed that modifying constants causes unpredictable
 * effect. This assumption allows the compiler more freedom
 * to perform code optimization.
 * As a direct consequence, the types of all constant expressions
 * are marked as TY_CONST.
 *
 * integral types = [short, long, unsigned, signed, int, char, enum]
 * arithmetic types = integral types + [float, double]
 * pointer types = [pointer, 0-dimension array]
 * fundamental types = arithmetic types + pointer types
 * structure types = [union, struct]
 * array type = [N-dimension array, N is defined]
 * function type = [func]
 *
 * ** pointer and array are inter-changeable.
 */
/*****************************************************************************/

#include <config.h>
#include <library/llist.h>
#include "pcode.h"
#include "impact_global.h"
#include "struct.h"
#include "cast.h"
#include "symtab.h"
#include "struct_symtab.h"
#include "query.h"
#include "query_symtab.h"
#include "reduce.h"
#include "reduce_symtab.h"

/*! \brief Upgrade the type to the natural data type.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr to upgrade.
 * \param parentexpr
 *  the Expr's parent expression.
 *
 * \return
 *  A cast Expr to convert the expr to an int.
 *
 * Upgrade the type to the natural data type (TY_INT, TY_DOUBLE).
 * This function does not explicitly duplicate the formal parameter.
 * The caller must make sure that there can be at most one reference
 * to the expression. (since we assume the expression tree structure
 * is NOT shared, in P_RemoveExpr())
 *
 * (char -> int)
 * (short -> int)
 *
 * In this version, we do not do (float->double) in computation.
 *
 * \bug The OP_cast Expr returned by this function does not have its id
 *      field set.  To do that requires getting the expr's scope, which
 *      is not guaranteed to be defined for \a expr.
 */
Expr
PST_UpgradeArithmeticType (SymbolTable table, Expr expr, Expr parentexpr)
{
  Type old_type, new_type;
  Expr cast;
  Expr temp_nextptr, nextptr, temp_sibptr, sibptr;
  _BasicType old_type_bt;

  if (expr == NULL)
    P_punt ("cast_symtab.c:PST_UpgradeArithmeticType:%d expr is null",
	    __LINE__ - 1);

  old_type = PST_ExprType (table, expr);
  old_type_bt = PST_GetTypeBasicType (table, old_type);

  if (old_type_bt & (BT_CHAR | BT_SHORT))
    {
      if (old_type_bt & BT_UNSIGNED)
	new_type = PST_FindBasicType (table, BT_INT | BT_UNSIGNED);
      else
	new_type = PST_FindBasicType (table, BT_INT);
      cast = P_NewExprWithOpcode (OP_cast);
      PST_SetExprType (table, cast, new_type);
      P_AppendExprOperands (cast, expr);

      /*
       * ** DIA - Make sure that all pointers from expr are
       * **       moved to cast.
       */
      P_SetExprNext (cast, P_GetExprNext (expr));
      P_SetExprSibling (cast, P_GetExprSibling (expr));
      
      sibptr = P_GetExprOperands (parentexpr);
      while (sibptr)
	{
	  nextptr = sibptr;
	  while (nextptr)
	    {
	      temp_nextptr = P_GetExprNext (nextptr);
	      if (P_GetExprNext (nextptr) == expr)
		P_SetExprNext (nextptr, cast);
	      nextptr = temp_nextptr;
	    }

	  temp_sibptr = P_GetExprSibling (sibptr);
	  if (P_GetExprSibling (sibptr) == expr)
	    P_SetExprSibling (sibptr, cast);
	  sibptr = temp_sibptr;
	}
      
      P_SetExprNext (expr, NULL);
      P_SetExprSibling (expr, NULL);
      if (P_GetExprOperands (parentexpr) == expr)
	P_SetExprOperands (parentexpr, cast);
      expr = cast;
    }

  return (expr);
}

/*! \brief Upgrades the result type of an expression.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the expression to upgrade.
 * \param type
 *  the new result Type of \a expr.
 * \param parentexpr
 *  the parent expression of \a expr.
 *
 * \return
 *  A new Expr which is \a expr cast to the new Type.
 * 
 * Add explicit casting operator to upgrade the result of an
 * expression to a more powerful type.
 *
 * \note Haven't reduced type here.
 *
 * \bug The OP_cast Expr returned by this function does not have its id
 *      field set.  To do that requires getting the expr's scope, which
 *      is not guaranteed to be defined for \a expr.
 */
Expr
PST_UpgradeType (SymbolTable table, Expr expr, Type type, Expr parentexpr)
{
  Expr cast;
  Expr temp_sibptr, sibptr, temp_nextptr, nextptr;

  cast = P_NewExprWithOpcode (OP_cast);
  PST_SetExprType (table, cast, type);
  P_AppendExprOperands (cast, expr);

  /*
     ** DIA - Make sure that all pointers from expr are
     **   moved to cast.  
   */

  P_SetExprNext (cast, P_GetExprNext (expr));
  P_SetExprSibling (cast, P_GetExprSibling (expr));

  /* GEH - added code for null parentexpr */
  if (parentexpr)
    {
      sibptr = P_GetExprOperands (parentexpr);
      while (sibptr)
	{
	  nextptr = sibptr;
	  while (nextptr)
	    {
	      temp_nextptr = P_GetExprNext (nextptr);
	      if (P_GetExprNext (nextptr) == expr)
		P_SetExprNext (nextptr, cast);
	      nextptr = temp_nextptr;
	    }

	  temp_sibptr = P_GetExprSibling (sibptr);
	  if (P_GetExprSibling (sibptr) == expr)
	    P_SetExprSibling (sibptr, cast);
	  sibptr = temp_sibptr;
	}
    }

  P_SetExprNext (expr, NULL);
  P_SetExprSibling (expr, NULL);
  /* GEH - added check for null parentexpr */
  if (parentexpr && P_GetExprOperands (parentexpr) == expr)
    P_SetExprOperands (parentexpr, cast);

  return (cast);
}

/*! \brief Casts the operands of an expression.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr to cast.
 *
 * Determine the resultant type of the expression based on its
 * operator and its operands.  Check if the types of operands are
 * compatible.  Add new expressions to explicitely cast the operands
 * to the required types.
 *
 * Much of the old CastExpr function is now performed by P_ExprTypeScope()
 *
 * \bug The OP_cast Expr returned by this function does not have its id
 *      field set.  To do that requires getting the expr's scope, which
 *      is not guaranteed to be defined for \a expr.
 */
void
PST_CastExpr (SymbolTable table, Expr expr)
{
  _Opcode opcode;

  /* BCC - 10/30/96 -- always need to cast OP_long (JWS) */
  if ((P_fast_mode == 1) && expr && \
      (P_GetExprOpcode (expr) != OP_type_size))
    return;

  if (expr == NULL)
    P_punt ("cast_symtab.c:PST_CastExpr:%d expr is null", __LINE__);

  /* If the type field is already defined, do nothing. */
  if (P_ValidKey (P_GetExprType (expr)))
    return;


  opcode = P_GetExprOpcode (expr);
  switch (opcode)
    {
    case OP_var:
      /*
       *  If the variable is defined, then the resultant type
       *  is the type of the variable, as declared.
       *  Otherwise, we assume it is (TY_INT | TY_EXTERN)
       */
      break;

    case OP_int:
      PST_SetExprType (table, expr,
		       PST_FindBasicTypeWithQual (table, BT_INT, TY_CONST));
      break;

    case OP_float:
    case OP_real:
      /*
       *  According to K&R, floating-point constants are double.
       *  (TY_DOUBLE | TY_CONST)
       * GEH - I don't believe it.  This causes performance degradation.
       *       I'm changing it to "float" for now.  9-4-94
       */
      PST_SetExprType (table, expr,
		       PST_FindBasicTypeWithQual (table, BT_FLOAT, TY_CONST));
      break;

    case OP_double:
      /*
       *  (TY_DOUBLE | TY_CONST)
       */
      PST_SetExprType (table, expr,
		       PST_FindBasicTypeWithQual (table, BT_DOUBLE, TY_CONST));
      break;

    case OP_char:
      /*
       *  According to K&R, character literal is an integer.
       *  (TY_INT | TY_CONST)
       */
      PST_SetExprType (table, expr,
		       PST_FindBasicTypeWithQual (table, BT_INT, TY_CONST));
      break;

    case OP_string:
      {
	/*
	 *  (TY_CHAR | TY_CONST) (P)
	 */
	Type new_type;

	new_type = PST_FindPointerToType (table, PST_FindBasicType (table,
								    BT_CHAR));
	new_type = PST_FindTypeSetQualifier (table, new_type, TY_CONST);
	PST_SetExprType (table, expr, new_type);
      }
      break;

      /* special COMPILE-TIME operations */
    case OP_cast:
      /*
       *  The type is already defined in the input CCODE.
       *  But, we should not got here, because CastExpr()
       *  returns very early if the type field is already
       *  defined. Therefore, it is an error to reach here.
       */
      P_punt ("cast_symtab.c:PST_CastExpr:%d the type field of (OP_cast) is "
	      "undefined", __LINE__ - 1);
      break;

    case OP_expr_size:
    case OP_type_size:
      /*
       *  Type size (expression size) is the number of
       *  bytes required to store the type (result).
       *  The resultant type is integer, and is a constant.
       *  (TY_INT | TY_CONST)
       */
      PST_SetExprType (table, expr,
		       PST_FindBasicTypeWithQual (table, BT_INT, TY_CONST));
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
	Type type1, type2, rtype;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);

	/* If the operands are smaller than an int, cast them to int. */
	op1 = PST_UpgradeArithmeticType (table, op1, expr);
	op2 = PST_UpgradeArithmeticType (table, op2, expr);

	/* Get the result type of this expression. */
	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);
	rtype = PST_ExprType (table, expr);

	/* Cast the operands to the result type if necessary. */
	if (!PST_EqualStrengthType (table, rtype, type1))
	  op1 = PST_UpgradeType (table, op1, rtype, expr);
	if (!PST_EqualStrengthType (table, rtype, type2))
	  op2 = PST_UpgradeType (table, op2, rtype, expr);

	PST_SetExprType (table, expr, rtype);
      }
      break;

    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
      {
	/*
	 *        1) normal arithmetic conversions are performed.
	 *        2) The result is TY_INT.
	 *        3) apply only on fundamental types.
	 */
	Expr op1, op2;
	Type type1, type2, rtype;
	int arith1, arith2;

	/* 
	 * perform arithmetic conversions. 
	 */
	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);
	
	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);

	if (!PST_IsFundamentalType (table, type1))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires fundamental type "
		  "for operand 1", __LINE__ - 1, op_to_value[opcode]);
	if (!PST_IsFundamentalType (table, type2))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires fundamental type "
		  "for operand 2", __LINE__ - 1, op_to_value[opcode]);
	  
	if ((arith1 = PST_IsArithmeticType (table, type1)))
	  op1 = PST_UpgradeArithmeticType (table, op1, expr);
	if ((arith2 = PST_IsArithmeticType (table, type2)))
	  op2 = PST_UpgradeArithmeticType (table, op2, expr);

	/* Need to bring both operands to the same class. */
	if (arith1 && arith2)
	  {
	    rtype = PST_DominantType (table, type1, type2);

	    if (!PST_EqualStrengthType (table, rtype, type1))
	      op1 = PST_UpgradeType (table, op1, rtype, expr);
	    if (!PST_EqualStrengthType (table, rtype, type2))
	      op2 = PST_UpgradeType (table, op2, rtype, expr);
	  }

	/* the result is TY_INT */
	PST_SetExprType (table, expr, PST_FindBasicType (table, BT_INT));
      }
      break;

      /* SHIFT operations */
    case OP_rshft:
    case OP_lshft:
      {
	/*
	 *        1) normal arithmetic conversions are performed.
	 *        2) apply only on integral types.
	 *        3) the right operand is converted to TY_INT.
	 *        4) the resultant type is the type of the left operand.
	 */
	Expr op1, op2;
	Type type1, type2;

	/* apply arithmetic conversions. */
	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);
	
	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);

	if (!PST_IsIntegralType (table, type1))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires integer type for "
		  "operand 1", __LINE__ - 1, op_to_value[opcode]);
	if (!PST_IsIntegralType (table, type2))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires integer type for "
		  "operand 2", __LINE__ - 1, op_to_value[opcode]);

	op1 = PST_UpgradeArithmeticType (table, op1, expr);
	op2 = PST_UpgradeArithmeticType (table, op2, expr);

	/* 
	 * the resulting type is the type of op1.
	 */
	PST_SetExprType (table, expr, type1);
      }
      break;

      /* ARITHMETIC operations */
    case OP_add:
    case OP_sub:
      {
	/* For OP_add,
	 *        1) the usual arithmetic conversions are performed.
	 *        2) a pointer to an array, integral -> pointer to array.
	 *
	 * For OP_sub,
	 *        1) the usual arithmetic conversions are performed.
	 *        2) a pointer to an array, integral -> pointer to array.
	 *        3) pointer to array - pointer to array -> TY_INT.
	 */
	Expr op1, op2;
	Type type1, type2, rtype;
	int arith1, arith2;

	/* apply arithmetic conversions. */
	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);
	rtype = PST_ExprType (table, expr);

	if ((arith1 = PST_IsArithmeticType (table, type1)))
	  op1 = PST_UpgradeArithmeticType (table, op1, expr);

	if ((arith2 = PST_IsArithmeticType (table, type2)))
	  op2 = PST_UpgradeArithmeticType (table, op2, expr);

	if (arith1 && arith2)
	  {
	    if (!PST_EqualStrengthType (table, rtype, type1))
	      op1 = PST_UpgradeType (table, op1, rtype, expr);
	    if (!PST_EqualStrengthType (table, rtype, type2))
	      op2 = PST_UpgradeType (table, op2, rtype, expr);
	  }

	PST_SetExprType (table, expr, rtype);
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
	Type type1, type2, rtype;

	/* apply arithmetic conversions. */
	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);
	rtype = PST_ExprType (table, expr);

	if (!PST_EqualStrengthType (table, rtype, type1))
	  op1 = PST_UpgradeType (table, op1, rtype, expr);
	if (!PST_EqualStrengthType (table, rtype, type2))
	  op2 = PST_UpgradeType (table, op2, rtype, expr);

	PST_SetExprType (table, expr, rtype);
      }
      break;

    case OP_neg:
      {
	/*
	 *  1) usual arithmetic conversions are performed.
	 *  ** for unsigned, -a == (((unsigned)0xF*+1) - a)
	 *     where 0xF* is the biggest possible unsigned int.
	 *  2) the resultant type is the type of the operand.
	 *  3) apply only to arithmetic types.
	 */
	Expr op1;
	Type type1;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);

	/* apply arithmetic conversions. */
	if (!PST_IsArithmeticType (table, type1))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires arithmetic "
		  "operand 1", __LINE__ - 1, op_to_value[opcode]);
	
	op1 = PST_UpgradeArithmeticType (table, op1, expr);

	PST_SetExprType (table, expr, type1);
      }
      break;

    case OP_not:
      {
	/*
	 *  1) usual arithmetic conversions are performed.
	 *  2) the resultant type is TY_INT.
	 *  3) apply only to arithmetic types and to pointers.
	 */
	Expr op1;
	Type type1;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PSt_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);

	/* apply arithmetic conversions. */
	if (PST_IsArithmeticType (table, type1))
	  op1 = PST_UpgradeArithmeticType (table, op1, expr);

	PST_SetExprType (table, expr, PST_FindBasicType (table, BT_INT));
      }
      break;

    case OP_inv:
      {
	/*
	 *  1) usual arithmetic conversions are performed.
	 *  2) the resultant type is the type of the operand.
	 *  3) apply only to integral types.
	 */
	Expr op1;
	Type type1;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);

	/* apply arithmetic conversions. */
	if (!PST_IsIntegralType (table, type1))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires integer operand "
		  "1", __LINE__ - 1, op_to_value[opcode]);

	op1 = PST_UpgradeArithmeticType (table, op1, expr);

	PST_SetExprType (table, expr, type1);
      }
      break;

      /* SIDE-EFFECT operations */
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
      {
	/*
	 *  1) the operand must be arithmetic type or pointer type.
	 *  2) the resultant type is the type of the operand.
	 */
	Expr op1;
	Type type1;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);

	/* apply arithmetic conversions. */
	if (!PST_IsArithmeticType (table, type1) && \
	    !PST_IsPointerType (table, type1))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires arithmetic or "
		  "pointer\noperand", __LINE__ - 1, op_to_value[opcode]);

	PST_SetExprType (table, expr, type1);
      }
      break;

    case OP_assign:
      {
	/*
	 *  1) if both operands are arithmetic types, the right
	 *          operand is converted to the type of the left operand.
	 *  2) (E1 op= E2) == (E1 = E1 op (E2))
	 *  3) all right operands and all non-pointer left operands
	 *          must have arithemtic type. (except OP_assign,
	 *          for which the right hand side can also be ptr/arry)
	 *  4) for =, +=, -=, the left operand can be a pointer.
	 *  5) the resultant type is the type of the left operand.
	 */
	Expr op1, op2;
	Type type1, type2;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing left operand",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing right operand",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);

	/* If both operands are arithmetic convert the second operand
	 * to the type of the first operand.  */
	if (PST_IsArithmeticType (table, type1) && \
	    PST_IsArithmeticType (table, type2) && \
	    !PST_EqualStrengthType (table, type1, type2))
	  op2 = PST_UpgradeType (table, op2, type1, expr);
	else if (PST_IsStructureType (table, type2) && \
		 !PST_IsStructureType (table, type1))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d Illegal assignment of struct "
		  "to non-struct", __LINE__ - 1);

	PST_SetExprType (table, expr, type1);
      }
      break;

      /* BCC - just copy the type of op1 to expr, don't cast op2 - 6/13/95 */
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
	/*
	 *  1) all right operands and all non-pointer left operands
	 *          must have arithemtic type. (except OP_assign,
	 *          for which the right hand side can also be ptr/arry)
	 *  2) for =, +=, -=, the left operand can be a pointer.
	 *  3) the resultant type is the type of the left operand.
	 */
	Expr op1, op2;
	Type type1, type2;
	int ptr1;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);

	ptr1 = PST_IsPointerType (table, type1);

	if (!(ptr1 | PST_IsArithmeticType (table, type1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires pointer or "
		  "arithmetic\ntype for left hand side", __LINE__ - 1,
		  op_to_value[opcode]);

	if (ptr1 && (opcode != OP_Aadd) && (opcode != OP_Asub))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s does not allow poiner for "
		  "left hand side.", __LINE__ - 1, op_to_value[opcode]);

	if (!PST_IsArithmeticType (table, type2))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires arithmetic right "
		  "hand side", __LINE__ - 1, op_to_value[opcode]);

	/* apply required conversions. */
	op2 = PST_UpgradeArithmeticType (table, op2, expr);

	PST_SetExprType (table, expr, type1);
      }
      break;

      /* DECISION operations */
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
	Expr op1, op2, op3;
	Type type1, type2, type3, rtype;
	int arith2, arith3;

	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprSibling (op1)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op3 = P_GetExprSibling (op2)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 3",
		  __LINE__ - 1, op_to_value[opcode]);
	  
	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);
	type3 = PST_ExprType (table, op3);
	rtype = PST_ExprType (table, expr);

	/* apply arithmetic conversions if possible. */
	if (PST_IsArithmeticType (table, type1))
	  PST_UpgradeArithmeticType (table, op1, expr);
	if ((arith2 = PST_IsArithmeticType (table, type2)))
	  PST_UpgradeArithmeticType (table, op2, expr);
	if ((arith3 = PST_IsArithmeticType (table, type3)))
	  PST_UpgradeArithmeticType (table, op3, expr);

	/*
	 * if op2 and op3 are both arithmetic, try to
	 * bring them to the same type.
	 */
	if (arith2 && arith3)
	  {
	    if (!PST_EqualStrengthType (table, rtype, type2))
	      op2 = PST_UpgradeType (table, op2, rtype, expr);
	    if (!PST_EqualStrengthType (table, rtype, type3))
	      op3 = PST_UpgradeType (table, op3, rtype, expr);
	  }

	PST_SetExprType (table, expr, rtype);
      }
      break;

    case OP_disj:
    case OP_conj:
      {
	/*
	 *  1) the operands must have one of the fundamental types
	 *  2) the result is always TY_INT.
	 */
	Expr op1, op2;
	Type type1, type2;
	
	if (!(op1 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 1",
		  __LINE__ - 1, op_to_value[opcode]);
	if (!(op2 = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing operand 2",
		  __LINE__ - 1, op_to_value[opcode]);

	type1 = PST_ExprType (table, op1);
	type2 = PST_ExprType (table, op2);

	if (!PST_IsFundamentalType (table, type1))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires arithmetic or "
		  "pointer type for\noperand 1", __LINE__ - 1,
		  op_to_value[opcode]);
	if (!PST_IsFundamentalType (table, type2))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s requires arithmetic or "
		  "pointer type for\noperand 1", __LINE__ - 1,
		  op_to_value[opcode]);

	PST_SetExprType (table, expr, PST_FindBasicType (table, BT_INT));
      }
      break;

    case OP_compexpr:
    case OP_dot:
    case OP_arrow:
    case OP_indr:
    case OP_addr:
    case OP_index:
      break;

#if 0
    case OP_indr:
      {
	/*
	 *  1) the operand must be a pointer. (pointer to ...)
	 *  2) the resultant type is (...)
	 */
	Expr op1;
	Type type;
	Dcltr dcltr;
	op1 = GetOperand (expr, 1);
	if (op1 == 0)
	  {
	    P_punt ("cast: illegal (*) operator: missing operand");
	  }
	if (!IsPointerType (op1->type) && !IsArrayType (op1->type) &&
	    /* BCC - a function can be called either directly or inderectly - 5/30/95 */
	    !IsFunctionType (op1->type))
	  {
	    P_punt ("cast: illegal (*) operator: illegal operand");
	  }
	/* BCC - not to dereference a indirect function call - 5/30/95 */
	if (IsFunctionType (op1->type))
	  {
	    /* BCC - copy every thing in the operands to the parent - 5/30/95 */
	    expr->status = op1->status;
	    expr->opcode = op1->opcode;
	    expr->enum_flag = op1->enum_flag;
	    expr->type = op1->type;
	    expr->value.string = op1->value.string;
	    expr->sibling = op1->sibling;
	    expr->operands = op1->operands;
	    expr->next = op1->next;
	    expr->pragma = op1->pragma;
	    expr->acc = op1->acc;
	    expr->acc2 = op1->acc2;

	    op1->sibling = 0;
	    op1->operands = 0;
	    op1->parentexpr = 0;
	    op1->next = 0;
	    op1->previous = 0;
	    op1->pragma = 0;
	    op1->parentstmt = 0;
	    op1->type = 0;

	    op1->acc = 0;
	    op1->acc2 = 0;
	    RemoveExpr (op1);
	  }
	else
	  {
	    dcltr = op1->type->dcltr;
	    op1->type->dcltr = dcltr->next;
	    type = CopyType (op1->type);
	    op1->type->dcltr = dcltr;
	    expr->type = type;
	  }
	break;
      }
    case OP_addr:
      {
	/*
	 *  1) the resultant type is (pointer to the type of the operand)
	 */
	Expr op1;
	Type type;
	Dcltr dcltr;
	op1 = GetOperand (expr, 1);
	if (op1 == 0)
	  P_punt ("cast: illegal (&) operator: missing operand");
	type = CopyType (op1->type);
	dcltr = NewDcltr ();
	dcltr->method = D_PTR;
	dcltr->next = type->dcltr;
	type->dcltr = dcltr;
	expr->type = type;
	if (merge_interprocedural_data)
	  {
	    while (op1->opcode == OP_compexpr)
	      op1 = op1->operands;
	    if (op1->opcode == OP_var)
	      {
		address_taken_list =
		  AddToSortedLptr (C_findstr (op1->value.string),
				   address_taken_list);
	      }
	  }
	break;
      }
    case OP_index:
      {
	/*
	 *  1) the first operand must be an array (pointer) type.
	 *          (array of ...) or (pointer to ...)
	 *  2) the second operand must be integral type.
	 *  3) the resultant type is ...
	 */
	Expr op1, op2, op2_temp;
	Type type, type1, type2;
	Dcltr dcltr;
	op1 = GetOperand (expr, 1);
	op2_temp = GetOperand (expr, 2);
	/* can be a comma expr */
	op2 = GetLastOperand (op2_temp);
	if ((op1 == 0) || (op2 == 0))
	  P_punt ("cast: illegal [] operation: missing operands");
	type1 = op1->type;
	type2 = op2->type;
	if (!IsPointerType (type1) && !IsArrayType (type1))
	  {
	    P_punt
	      ("cast: illegal [] operation: 1st operand is not an array");
	  }
	if (!IsIntegralType (type2))
	  {
	    P_punt
	      ("cast: illegal [] operation: 2nd operand is not an integer");
	  }
	/*
	 * compute the resultant type.
	 */
	dcltr = type1->dcltr;
	type1->dcltr = dcltr->next;
	type = CopyType (type1);
	type1->dcltr = dcltr;
	expr->type = type;
	break;
      }
#endif

      /* CALL/RETURN operations */
    case OP_call:
      {
	/*
	 *  ** we no longer force float into double.
	 *  1) any of type char or short are converted to int;
	 *  2) any of array names are converted to pointers;
	 *  3) the resultant type is the return type of the function.
	 *  4) if the function is not defined, it is assumed to be 
	 *          TY_INT | TY_EXTERN.
	 */
	Expr fn, head, tail, current, next;
	Type rtype;

	if (!(fn = P_GetExprOperands (expr)))
	  P_punt ("cast_symtab.c:PST_CastExpr:%d %s missing function name",
		  __LINE__ - 1, op_to_value[opcode]);

	rtype = PST_ExprType (table, expr);

	/* apply type conversions to the parameters. */
	tail = NULL;
	head = NULL;

	next = P_GetExprSibling (fn);
	while (next)
	  {
	    current = next;
	    next = P_GetExprNext (next);
	    P_SetExprNext (current, NULL);

	    if (head == NULL)
	      {
		head = PST_ReduceExpr (table, current);
		tail = head;
	      }
	    else
	      {
		P_SetExprNext (tail, PST_ReduceExpr (table, current));
		tail = P_GetExprNext (tail);
	      }
	    /* BCC - garbage collection - 8/19/96 */
	    current = P_RemoveExpr (current);
	  }
	P_SetExprSibling (fn, head);

	PST_SetExprType (table, expr, rtype);
      }
      break;

    case OP_stmt_expr:
      break;

    case OP_asm_oprd:
      printf("CAST: OP_asm_oprd\n");
      exit(0);
      break;

    default:
      P_punt ("cast_symtab.c:PST_CastExpr:%d Unknown opcode %d", __LINE__,
	      opcode);
    }

  return;
}
