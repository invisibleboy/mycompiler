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
 * \brief This file contains functions to reduce expressions.
 *
 * \author David August, Po-hua Chang, Grant Haab, Krishna Subramanian,
 *         Nancy Warter and Wen-mei Hwu
 *
 * Copyright (c) 1991 David August, Po-hua Chang, Grant Haab, 
 * Krishna Subramanian, Nancy Warter, Wen-mei Hwu,
 * and The Board of Trustees of the University of llinois.
 *
 * All rights reserved.
 *
 * The University of Illinois software License Agreement specifies the
 * terms and conditions for redistribution.
 *
 * This file contains functions that need to access the symbol table to
 * reduce expressions.
 *****************************************************************************/

#include <config.h>
#include "impact_global.h"
#include "pcode.h"
#include "parms.h"
#include "symtab.h"
#include "struct.h"
#include "struct_symtab.h"
#include "query.h"
#include "query_symtab.h"
#include "reduce.h"
#include "reduce_symtab.h"
#include "cast.h"
#include "cast_symtab.h"
#include <library/i_list.h>

static Expr cast (SymbolTable table, Expr expr, Type dst_type);
static Expr not_zero (SymbolTable table, Expr expr);
static ITintmax prepare_int_mask (int size);

/*! \brief Evaluates to the value of the most significant bit in an integer.
 *
 * \param i
 *  the integer to inspect.
 * \param s
 *  the size of the integer.
 *
 * \return
 *  The value of the most significant bit of an integer of size \a s extracted
 *  from \a i.
 */
#define MOST_SIG_BIT(i, s) (((i) >> (((s) * 8) - 1)) & 1)

/*! \brief Bring the type up-to-date.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the expr to cast.
 * \param dst_type
 *  the type to cast the expr to.
 *
 * \return
 *  A new Expr which is \a expr with an appropriate cast.
 *
 * Assume type is always more powerful than expr->type.
 */
static Expr
cast (SymbolTable table, Expr expr, Type dst_type)
{
  Type src_type = PST_ExprType (table, expr);

  /* DIA - 5/6/94: Handle pointer arithmetic */
  if (!PST_IsPointerType (table, src_type) && \
      !PST_IsFunctionType (table, src_type) && \
      !PST_IsArrayType (table, src_type))
    {
      /* IMS - 6/17/02 - If the types are arithmetic, then run
	 EqualStrenth to determine if we have to UpgradeType.
	 If the types are not arithmetic and they are different,
	 then we have to run UpgradeType.
      */
      /* REK 12/15/03 This construct calls P_UpgradeType unless both types
       *              are arithmetic and equal strength. */
      if (!PST_IsArithmeticType (table, dst_type) || \
	  !PST_IsArithmeticType (table, src_type) || \
	  !PST_EqualStrengthType (table, dst_type, src_type))
	expr = PST_UpgradeType (table, expr, dst_type, NULL);
    }

  return (expr);
}

/*! \brief Create a new expression to compare against zero.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the expression to compare against zero.
 *
 * \return
 *  A new expression ( \a expr != 0 )
 *
 * Compare against zero.
 * If zero, returns 0.
 * If not-zero, the resultant expression should return 1.
 */
static Expr
not_zero (SymbolTable table, Expr expr)
{
  Expr new;
  Key scope_key;

  if (!expr)
    P_punt ("reduce_symtab.c:not_zero:%d expr is null", __LINE__);

  if (P_IsBooleanExpr (expr))
    {
      new = expr;
    }
  else
    {
      scope_key = PST_GetExprScope (table, expr);

      new = PST_ScopeNewExprWithOpcode (table, scope_key, OP_ne);
      PST_SetExprType (table, new, PST_FindBasicType (table, BT_INT));
      P_AppendExprOperands (new, expr);
      P_AppendExprOperands (new, PST_ScopeNewIntExpr (table, scope_key, 0));
    }

  return (new);
}

/*! \brief Prepares a mask to change the size of an integer.
 *
 * \param size
 *  the number of bytes in the destination int type.
 *
 * \return
 *  A bitmask to use when changing an int value from one type to another.
 *
 * The returned mask has 1s in bits 0 - ((size * 8) - 1), and 0s in the
 * rest.  To convert an unsigned or positive signed int, AND the int
 * with the mask.  To convert a negative signed int, OR the int with the 
 * inverted mask.
 */
static ITintmax
prepare_int_mask (int size)
{
  ITintmax mask = 0;
  int i;

  for (i = 0; i < size * 8; i++)
    mask = (mask << 1) + 1;

  return (mask);
}

/*! \brief Apply constant folding.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the expr to reduce.
 *
 * Apply algebraic operations to reduce the expression.  OP_type_size
 * and OP_expr_size nodes can only be reduced during RTL generation
 * phase. Otherwise, ignore optimizing OP_type_size and OP_expr_size
 * nodes (assume not reducible)
 *
 * +++ better garbage collection of nodes is necessary in the future.
 * GEH - ^^^^^^^^^^^^ no kidding!  Hopefully, it's all taken care of
 * now.
 *
 * This function returns a new expression containing the reduced version
 * of \a expr.  This function does not free \a expr, so managing \a expr
 * and the return value is the caller's responsibility.
 */
Expr
PST_ReduceExpr (SymbolTable table, Expr expr)
{
  Expr new, temp, op1, new_op1, op2, op3;
  _Opcode opcode;
  Type expr_type;
  Key scope_key;

  if (expr == NULL)
    return (NULL);

  if (P_fast_mode == 1)
    return (PST_CopyExpr (table, expr));

  opcode = P_GetExprOpcode (expr);
  scope_key = PST_GetExprScope (table, expr);

  /*
   * LEAF EXPRESSIONS
   * ----------------------------------------------------------------------
   */

  switch (opcode)
    {
    case OP_var:
    case OP_char:
    case OP_string:
    case OP_real:
    case OP_float:
    case OP_double:
    case OP_int:
    case OP_type_size:
    case OP_expr_size:
      return (PST_CopyExpr (table, expr));
      break;

    default:
      break;
    }

  /*
   * NON-LEAF EXPRESSIONS
   * ----------------------------------------------------------------------
   * Copy the root node and attach the (reduced) subtrees before reducing
   * at the root node.
   */
  new = PST_CopyExprNode (table, expr);	/* copy the root node only */

  for (op1 = P_GetExprOperands (expr); op1; op1 = P_GetExprSibling (op1))
    {
      new_op1 = PST_ReduceExpr (table, op1);
      P_AppendExprOperands (new, new_op1);

      for (op2 = P_GetExprNext (op1); op2; op2 = P_GetExprNext (op2))
	P_AppendExprNext (new_op1, PST_ReduceExpr (table, op2));
    }

#ifdef DEBUG_REDUCE
  pos = 0;
  Expr2String (expr_string, &pos, new, TRUE);
  if (pos > 1024)
    P_punt
      ("ReduceExpr: Error: increase length of expr_string in ReduceExpr()\n");
  fprintf (Flog, "....Reduced Expression:%s\n", expr_string);
#endif

  /* Apply constant folding on the current node. */
  switch (opcode)
    {
    case OP_compexpr:
#if 1
      /* JWS 20040922 Implement a better reduction of OP_compexpr.
       * Remove all non-last, side-effect-free subexpressions.  If
       * only a single subexpression remains, replace the OP_compexpr
       * with it.  
       */
      {
	int opt = 0;
	Expr e, ol = P_GetExprOperands (new);
	List l = NULL;

	for (e = ol; e; e = P_GetExprNext (e))
	  {
	    if (!P_GetExprNext (e) || !P_IsSideEffectFree (e))
	      l = List_insert_last (l, e);
	    else
	      opt = 1;
	  }

	if (opt)
	  {
	    if (List_size (l) == 1)
	      {
		/* Replace compexpr with single remaining expr */
		temp = PST_CopyExpr (table, (Expr)List_first (l));
		new = PST_RemoveExpr (table, new);
		new = temp;
	      }
	    else
	      {
		/* Rebuild compexpr with fewer exprs */
		new->operands = NULL;

		List_start (l);

		while ((e = (Expr) List_next (l)))
		  new->operands = P_AppendExprNext (new->operands,
						    PST_CopyExpr (table, e));

		/* Delete operand list, now replaced */
		ol = PST_RemoveExpr (table, ol);
	      }
	  }

	List_reset (l);
      }
#else
      {
	/* Get the last operand in the list. */
	for (op1 = P_GetExprOperands (new); op1; op1 = P_GetExprNext (op1));

	/* Can reduce "(a,b,...,m,n)" to n if a...m are side-effect free.
	 * conservative approach: reduce only (n) */

	/* JWS -- why only for const expressions?
	 * Can't we check IsSideEffectFree like below for || and &&? */

	/* comma expression has a single constituent */
	if ((op1 == P_GetExprOperands (new)) && P_IsConstNumber (op1))
	  {
	    temp = PST_CopyExpr (table, op1);
	    new = PST_RemoveExpr (table, new);
	    new = temp;
	  }
      }
#endif

      break;

    case OP_quest:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);
	op3 = P_GetExprSibling (op2);
	
	/*  1) true ? A : B         => A
	 *  2) false ? A : B        => B
	 */
	if (P_IsConstZero (op1))
	  {
	    expr_type = PST_ExprType (table, new);

	    temp = cast (table, op3, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsConstNonZero (op1))
	  {
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op2 from the Expr and cast it. */
	    P_SetExprSibling (op2, NULL);
	    temp = cast (table, op2, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);
	    P_SetExprSibling (op3, NULL);
	    op3 = PST_RemoveExpr (table, op3);
	    
	    new = temp;
	  }
      }
      break;

    case OP_disj:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	if (P_IsConstNonZero (op1))
	  {
	    /* true || B --> true */
	    new = PST_RemoveExpr (table, new);
	    new = PST_ScopeNewIntExpr (table, scope_key, 1);
	  }
	else if (P_IsConstZero (op1))
	  {
	    /* false || B --> (B!=0) */
	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);

	    new = not_zero (table, op2);
	  }
	else if (P_IsConstZero (op2))
	  {
	    /* A || false --> (A!=0) */
	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = not_zero (table, op1);
	  }
	else if (P_IsConstNonZero (op2) && P_IsSideEffectFree (op1))
	  {
	    /* A || true (A side-effect free) --> true */
	    new = PST_RemoveExpr (table, new);
	    new = PST_ScopeNewIntExpr (table, scope_key, 1);
	  }
      }
      break;

    case OP_conj:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	if (P_IsConstZero (op1))
	  {
	    /* false && B --> false */
	    new = PST_RemoveExpr (table, new);
	    new = PST_ScopeNewIntExpr (table, scope_key, 0);
	  }
	else if (P_IsConstNonZero (op1))
	  {
	    /* true && B --> (B!=0) */
	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);

	    new = not_zero (table, op2);
	  }
	else if (P_IsConstNonZero (op2))
	  {
	    /* A && true --> (A!=0) */
	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = not_zero (table, op1);
	  }
	else if (P_IsConstZero (op2) && P_IsSideEffectFree (op1))
	  {
	    /* A && false (A side-effect free) --> false */
	    new = PST_RemoveExpr (table, new);
	    new = PST_ScopeNewIntExpr (table, scope_key, 1);
	  }
      }
      break;

    case OP_or:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) 0 | B               => B
	 *  2) A | 0               => A
	 *  3) N1 | N2             => <N1 or N2>   (N1, N2 integer constants)
	 */

	if (P_IsConstZero (op1))
	  {
	    expr_type = PST_ExprType (table, new);

	    temp = cast (table, op2, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);

	    new = temp;
	  }
	else if (P_IsConstZero (op2))
	  {
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op1 from the Expr and cast it. */
	    P_SetExprSibling (op1, NULL);
	    temp = cast (table, op1, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsIntegralExpr (op1) && P_IsIntegralExpr (op2))
	  {
	    temp = PST_ScopeNewIntExpr (table, scope_key,
					P_IntegralExprValue (op1) | \
					P_IntegralExprValue (op2));
	    PST_SetExprType (table, temp, PST_ExprType (table, new));
	    if (P_GetExprFlags (new) & EF_UNSIGNED)
	      P_SetExprFlags (temp, EF_UNSIGNED);

	    new = PST_RemoveExpr (table, new);

	    new = temp;
	  }
      }
      break;

    case OP_xor:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) 0 xor B              => B
	 *  2) A xor 0              => A
	 *  3) N1 xor N2            => <N1 xor N2>   (N1, N2 integers consts)
	 */

	if (P_IsConstZero (op1))
	  {
	    expr_type = PST_ExprType (table, new);

	    temp = cast (table, op2, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);

	    new = temp;
	  }
	else if (P_IsConstZero (op2))
	  {
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op1 from the Expr and cast it. */
	    P_SetExprSibling (op1, NULL);
	    temp = cast (table, op1, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsIntegralExpr (op1) && P_IsIntegralExpr (op2))
	  {
	    temp = PST_ScopeNewIntExpr (table, scope_key,
					P_IntegralExprValue (op1) ^ \
					P_IntegralExprValue (op2));
	    PST_SetExprType (table, temp, PST_ExprType (table, new));
	    if (P_GetExprFlags (new) & EF_UNSIGNED)
	      P_SetExprFlags (temp, EF_UNSIGNED);

	    new = PST_RemoveExpr (table, new);

	    new = temp;
	  }
      }
      break;

    case OP_and:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*
	 *  1) N1 and N2    => <N1 and N2>     (N1, N2 integer consts)
	 */
	if (P_IsIntegralExpr (op1) && P_IsIntegralExpr (op2))
	  {
	    temp = PST_ScopeNewIntExpr (table, scope_key,
					P_IntegralExprValue (op1) & \
					P_IntegralExprValue (op2));
	    PST_SetExprType (table, temp, PST_ExprType (table, new));
	    if (P_GetExprFlags (new) & EF_UNSIGNED)
	      P_SetExprFlags (temp, EF_UNSIGNED);

	    new = PST_RemoveExpr (table, new);

	    new = temp;
	  }
      }
      break;
	
    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_ge:
    case OP_gt:
      {
	double d1 = 0.0, d2 = 0.0;

	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*
	 *  1) N1 == N2     =>  <N1 == N2>  (N1, N2 constant)
	 *  2) N1 != N2     =>  <N1 != N2>  (N1, N2 constant)
	 *  3) N1 < N2      =>  <N1 < N2>   (N1, N2 constant)
	 *  4) N1 <= N2     =>  <N1 <= N2>  (N1, N2 constant)
	 *  5) N1 >= N2     =>  <N1 >= N2>  (N1, N2 constant)
	 *  6> N1 > N2      =>  <N1 > N2>   (N1, N2 constant)
	 */
	if (P_IsConstNumber (op1) && P_IsConstNumber (op2))
	  {
	    if (P_IsRealExpr (op1))
	      d1 = P_GetExprReal (op1);
	    else if (P_IsIntegralExpr (op1))
	      d1 = (double) P_IntegralExprValue (op1);
	    else
	      P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 1 matches "
		      "P_IsConstNumber\nbut not P_IsRealExpr or "
		      "P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);

	    if (P_IsRealExpr (op2))
	      d2 = P_GetExprReal (op2);
	    else if (P_IsIntegralExpr (op2))
	      d2 = (double) P_IntegralExprValue (op2);
	    else
	      P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 2 matches "
		      "P_IsConstNumber\nbut not P_IsRealExpr or "
		      "P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);

	    new = PST_RemoveExpr (table, new);

	    switch (opcode)
	      {
	      case OP_eq:
		new = PST_ScopeNewIntExpr (table, scope_key, d1 == d2);
		break;
		
	      case OP_ne:
		new = PST_ScopeNewIntExpr (table, scope_key, d1 != d2);
		break;

	      case OP_lt:
		new = PST_ScopeNewIntExpr (table, scope_key, d1 < d2);
		break;

	      case OP_le:
		new = PST_ScopeNewIntExpr (table, scope_key, d1 <= d2);
		break;

	      case OP_ge:
		new = PST_ScopeNewIntExpr (table, scope_key, d1 >= d2);
		break;

	      case OP_gt:
		new = PST_ScopeNewIntExpr (table, scope_key, d1 > d2);
		break;

	      default:
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d Should never get "
			"here!", __LINE__ - 1);
	      }
	  }
      }
      break;

    case OP_rshft:
    case OP_lshft:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) A >> 0       => A
	 *  2) A << 0       => A
	 *  3) N1 >> N2     => <N1 >> N2>   (N1, N2 integer constants)
	 *  4) N1 << N2     => <N1 << N2>   (N1, N2 integer constants)
	 */
	if (P_IsConstZero (op2))
	  {
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op1 from the Expr and cast it. */
	    P_SetExprSibling (op1, NULL);
	    temp = cast (table, op1, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsIntegralExpr (op1) && P_IsIntegralExpr (op2))
	  {
	    ITintmax sval, l1, l2;
	    int unsigned_A = P_GetExprFlags (op1) & EF_UNSIGNED;

	    l1 = P_IntegralExprValue (op1);
	    l2 = P_IntegralExprValue (op2);

	    if (opcode == OP_rshft)
	      sval = unsigned_A ? (((ITuintmax) l1) >> l2) : (l1 >> l2);
	    else
	      sval = l1 << l2;

	    temp = PST_ScopeNewIntExpr (table, scope_key, sval);
	    PST_SetExprType (table, temp, PST_ExprType (table, new));
	    if (P_GetExprFlags (new) & EF_UNSIGNED)
	      P_SetExprFlags (temp, EF_UNSIGNED);

	    new = PST_RemoveExpr (table, new);
	    new = temp;
	  }
      }
      break;

    case OP_add:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) A + 0        => A
	 *  2) 0 + B        => B
	 *  3) N1 + N2      => <N1 + N2>    (N1, N2 integer consts)
	 *
	 *  GEH - added the following cases to aid delinearization - 8/23/93
	 *  4) A + <-N1>    => A - N1
	 *  5) (A +/- N1) + N2  => A + <N2 +/- N1>  (A integer;
	 *                                           N1,N2 int consts)
	 *  6) (N1 +/- A) + N2  => <N1 + N2> +/- A  (A integer;
	 *                                           N1,N2 int consts)
	 *  7) N1 + (A +/- N2)  => A + <N1 +/- N2>  (A integer;
	 *                                           N1,N2 int consts)
	 *  8) N1 + (N2 +/- A)  => <N1 + N2> +/- A  (A integer;
	 *                                           N1,N2 int consts)
	 */
	if (P_IsConstZero (op1))
	  {
	    expr_type = PST_ExprType (table, new);

	    temp = cast (table, op2, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);

	    new = temp;
	  }
	else if (P_IsConstZero (op2))
	  {
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op1 from the Expr and cast it. */
	    P_SetExprSibling (op1, NULL);
	    temp = cast (table, op1, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsConstNumber (op1) && P_IsConstNumber (op2))
	  {
	    int scalar = 1;	/* expect all integral values. */
	    double d1 = 0.0, d2 = 0.0;
	    ITintmax l1 = 0, l2 = 0;

	    if (P_IsRealExpr (op1))
	      {
		scalar = 0;
		d1 = P_GetExprReal (op1);
	      }
	    else if (P_IsIntegralExpr (op1))
	      {
		l1 = P_IntegralExprValue (op1);
		d1 = (double) l1;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 1 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }
	    
	    if (P_IsRealExpr (op2))
	      {
		scalar = 0;
		d2 = P_GetExprReal (op2);
	      }
	    else if (P_IsIntegralExpr (op2))
	      {
		l2 = P_IntegralExprValue (op2);
		d2 = (double) l2;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%s %s operand 2 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }

	    if (scalar)
	      {
		if ((P_GetExprFlags (op1) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op2) & EF_UNSIGNED))
		  {
		    temp = PST_ScopeNewIntExpr (table, scope_key,
						(ITuintmax)l1 + (ITuintmax)l2);
		    P_SetExprFlags (temp, EF_UNSIGNED);
		  }
		else
		  {
		    temp = PST_ScopeNewIntExpr (table, scope_key, l1 + l2);
		  }
	      }
	    else if (P_IsFloatExpr (op1) && P_IsFloatExpr (op2))
	      {
		temp = PST_ScopeNewFloatExpr (table, scope_key, d1 + d2);
	      }
	    else
	      {
		temp = PST_ScopeNewDoubleExpr (table, scope_key, d1 + d2);
	      }

	    /* Copy the type from new. */
	    PST_SetExprType (table, temp, PST_ExprType (table, new));

	    new = PST_RemoveExpr (table, new);

	    new = temp;
	  }
	else if (P_IsConstNumber (op2) && P_IsConstNegative (op2))
	  {
	    if (P_IsIntegralExpr (op2))
	      P_SetExprScalar (op2, -P_GetExprScalar (op2));
	    else if (P_IsRealExpr (op2))
	      P_SetExprReal (op2, -P_GetExprReal (op2));
	    else
	      P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 2 matches "
		      "P_IsConstNumber\nbut not P_IsIntegralExpr or "
		      "P_IsRealExpr", __LINE__ - 2, op_to_value[opcode]);

	    P_SetExprOpcode (new, OP_sub);
	  }
	else if ((P_GetExprOpcode (op1) == OP_add || \
		  P_GetExprOpcode (op1) == OP_sub) && \
		 (P_GetExprOpcode (op2) == OP_int))
	  {
	    Expr op11, op12;
	    Type type11, type12;
	    
	    op11 = P_GetExprOperands (op1);
	    op12 = P_GetExprSibling (op11);

	    type11 = PST_ExprType (table, op11);
	    type12 = PST_ExprType (table, op12);

	    if ((P_GetExprOpcode (op12) == OP_int) && \
		TYPE_INTEGRAL (PST_GetTypeBasicType (table, type11)))
	      {
		/* 5) (A +/- N1) + N2  => A + <N2 +/- N1> 
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op2) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op12) & EF_UNSIGNED))
		  {
		    if (P_GetExprOpcode (op1) == OP_add)
		      P_SetExprScalar (op12, 
				       ((ITuintmax)P_GetExprScalar (op2) + \
					(ITuintmax)P_GetExprScalar (op12)));
		    else
		      P_SetExprScalar (op12,
				       ((ITuintmax)P_GetExprScalar (op2) - \
					(ITuintmax)P_GetExprScalar (op12)));

		    P_SetExprFlags (op12, EF_UNSIGNED);
		  }
		else
		  {
		    if (P_GetExprOpcode (op1) == OP_add)
		      P_SetExprScalar (op12, (P_GetExprScalar (op2) + \
					      P_GetExprScalar (op12)));
		    else
		      P_SetExprScalar (op12, (P_GetExprScalar (op2) - \
					      P_GetExprScalar (op12)));
		  }

		P_SetExprOpcode (op1, OP_add);
		P_SetExprSibling (op1, NULL);

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op2, NULL);
		op2 = PST_RemoveExpr (table, op2);

		/* 1/16/04 REK op1 was reduced above, so this is unnecessary.
		 */
		new = op1;
#if 0
		new = PST_ReduceExpr (table, op1);
		op1 = PST_RemoveExpr (table, op1);
#endif
	      }
	    else if ((P_GetExprOpcode (op11) == OP_int) && \
		     TYPE_INTEGRAL (PST_GetTypeBasicType (table, type12)))
	      {
		/* 6) (N1 +/- A) + N2  => <N1 + N2> +/- A  
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op11) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op2) & EF_UNSIGNED))
		  {
		    P_SetExprScalar (op11,
				     ((ITuintmax)P_GetExprScalar (op11) + \
				      (ITuintmax)P_GetExprScalar (op2)));

		    P_SetExprFlags (op11, EF_UNSIGNED);
		  }
		else
		  {
		    P_SetExprScalar (op11, (P_GetExprScalar (op11) + \
					    P_GetExprScalar (op2)));
		  }

		P_SetExprSibling (op1, NULL);

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op2, NULL);
		op2 = PST_RemoveExpr (table, op2);

		/* 1/16/04 REK op1 was reduced above, so this is unnecessary.
		 */
		new = op1;
#if 0
		new = PST_ReduceExpr (table, op1);
		op1 = PST_RemoveExpr (table, op1);
#endif
	      }
	  }
	else if ((P_GetExprOpcode (op2) == OP_add || \
		  P_GetExprOpcode (op2) == OP_sub) && \
		 (P_GetExprOpcode (op1) == OP_int))
	  {
	    Expr op21, op22;
	    Type type21, type22;

	    op21 = P_GetExprOperands (op2);
	    op22 = P_GetExprSibling (op21);

	    type21 = PST_ExprType (table, op21);
	    type22 = PST_ExprType (table, op22);

	    if ((P_GetExprOpcode (op22) == OP_int) && \
		TYPE_INTEGRAL (PST_GetTypeBasicType (table, type21)))
	      {
		/* 7) N1 + (A +/- N2)  => A + <N1 +/- N2>  
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op1) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op22) & EF_UNSIGNED))
		  {
		    if (P_GetExprOpcode (op2) == OP_add)
		      P_SetExprScalar (op22,
				       ((ITuintmax)P_GetExprScalar (op1) + \
					(ITuintmax)P_GetExprScalar (op22)));
		    else
		      P_SetExprScalar (op22,
				       ((ITuintmax)P_GetExprScalar (op1) - \
					(ITuintmax)P_GetExprScalar (op22)));

		    P_SetExprFlags (op22, EF_UNSIGNED);
		  }
		else
		  {
		    if (P_GetExprOpcode (op2) == OP_add)
		      P_SetExprScalar (op22, (P_GetExprScalar (op1) + \
					      P_GetExprScalar (op22)));
		    else
		      P_SetExprScalar (op22, (P_GetExprScalar (op1) - \
					      P_GetExprScalar (op22)));
		  }

		P_SetExprOpcode (op2, OP_add);

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op1, NULL);
		op1 = PST_RemoveExpr (table, op1);

		/* 1/16/04 REK op2 was reduced above, so this is unnecessary.
		 */
		new = op2;

#if 0
		new = PST_ReduceExpr (table, op2);
		op2 = PST_RemoveExpr (table, op2);
#endif
	      }
	    else if ((P_GetExprOpcode (op21) == OP_int) && \
		     TYPE_INTEGRAL (PST_GetTypeBasicType (table, type22)))
	      {
		/* 8) N1 + (N2 +/- A)  => <N1 + N2> +/- A  
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op1) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op21) & EF_UNSIGNED))
		  {
		    P_SetExprScalar (op21,
				     ((ITuintmax)P_GetExprScalar (op1) + \
				      (ITuintmax)P_GetExprScalar (op21)));

		    P_SetExprFlags (op21, EF_UNSIGNED);
		  }
		else
		  {
		    P_SetExprScalar (op21, (P_GetExprScalar (op1) + \
					    P_GetExprScalar (op21)));
		  }

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op1, NULL);
		op1 = PST_RemoveExpr (table, op1);

		/* 1/16/04 REK op2 was reduced above, so this is unnecessary.
		 */
		new = op2;

#if 0
		new = PST_ReduceExpr (table, op2);
		op2 = PST_RemoveExpr (table, op2);
#endif
	      }
	  }
      }
      break;

    case OP_sub:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) A - 0        => A
	 *  2) N1 - N2      => <N1 - N2>    (N1, N2 integer consts)
	 *
	 *  GEH - added the following cases to aid delinearization - 8/23/93
	 *  3) A - <-N1>        => A + N1
	 *  4) (A +/- N1) - N2  => A + <-N2 +/- N1> 
	 *     (A integer; N1, N2 int consts)
	 *  5) (N1 +/- A) - N2  => <N1 - N2> +/- A  
	 *     (A integer; N1, N2 int consts)
	 *  6) N1 - (A +/- N2)  => <N1 -/+ N2> - A  
	 *     (A integer; N1, N2 int consts)
	 *  7) N1 - (N2 +/- A)  => <N1 - N2> -/+ A  
	 *     (A integer; N1, N2 int consts)
	 */
	if (P_IsConstZero (op2))
	  {
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op1 from the Expr and cast it. */
	    P_SetExprSibling (op1, NULL);
	    temp = cast (table, op1, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsConstNumber (op1) && P_IsConstNumber (op2))
	  {
	    int scalar = 1;	/* expect all integral values. */
	    double d1 = 0.0, d2 = 0.0;
	    ITintmax l1 = 0, l2 = 0;

	    if (P_IsRealExpr (op1))
	      {
		scalar = 0;
		d1 = P_GetExprReal (op1);
	      }
	    else if (P_IsIntegralExpr (op1))
	      {
		l1 = P_IntegralExprValue (op1);
		d1 = (double) l1;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 1 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }

	    if (P_IsRealExpr (op2))
	      {
		scalar = 0;
		d2 = P_GetExprReal (op2);
	      }
	    else if (P_IsIntegralExpr (op2))
	      {
		l2 = P_IntegralExprValue (op2);
		d2 = (double) l2;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 2 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }

	    if (scalar)
	      {
		if ((P_GetExprFlags (op1) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op2) & EF_UNSIGNED))
		  {
		    temp = PST_ScopeNewIntExpr (table, scope_key,
						(ITuintmax)l1 - (ITuintmax)l2);
		    P_SetExprFlags (temp, EF_UNSIGNED);
		  }
		else
		  {
		    temp = PST_ScopeNewIntExpr (table, scope_key, l1 - l2);
		  }
	      }
	    else if (P_IsFloatExpr (op1) && P_IsFloatExpr (op2))
	      {
		temp = PST_ScopeNewFloatExpr (table, scope_key, d1 - d2);
	      }
	    else
	      {
		temp = PST_ScopeNewDoubleExpr (table, scope_key, d1 - d2);
	      }

	    /* Copy the type from new. */
	    PST_SetExprType (table, temp, PST_ExprType (table, new));

	    new = PST_RemoveExpr (table, new);

	    new = temp;
	  }
	else if (P_IsConstNumber (op2) && P_IsConstNegative (op2))
	  {
	    /* 3) 3) A - <-N1>        => A + N1 */
	    if (P_IsIntegralExpr (op2))
	      P_SetExprScalar (op2, -P_GetExprScalar (op2));
	    else if (P_IsRealExpr (op2))
	      P_SetExprReal (op2, -P_GetExprReal (op2));
	    else
	      P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 2 matches "
		      "P_IsConstNumber\nbut not P_IsIntegralExpr or "
		      "P_IsRealExpr", __LINE__ - 2, op_to_value[opcode]);

	    P_SetExprOpcode (new, OP_add);
	  }
	else if ((P_GetExprOpcode (op1) == OP_add || \
		  P_GetExprOpcode (op1) == OP_sub) && \
		 (P_GetExprOpcode (op2) == OP_int))
	  {
	    Expr op11, op12;
	    Type type11, type12;

	    op11 = P_GetExprOperands (op1);
	    op12 = P_GetExprSibling (op11);

	    type11 = PST_ExprType (table, op11);
	    type12 = PST_ExprType (table, op12);
	    
	    if ((P_GetExprOpcode (op12) == OP_int) && \
		TYPE_INTEGRAL (PST_GetTypeBasicType (table, type11)))
	      {
		/* 4) (A +/- N1) - N2  => A + <-N2 +/- N1>  
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op2) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op12) & EF_UNSIGNED))
		  {
		    if (P_GetExprOpcode (op1) == OP_add)
		      P_SetExprScalar (op12,
				       (-(ITuintmax)P_GetExprScalar (op2) + \
					(ITuintmax)P_GetExprScalar (op12)));
		    else
		      P_SetExprScalar (op12,
				       (-(ITuintmax)P_GetExprScalar (op2) - \
					(ITuintmax)P_GetExprScalar (op12)));

		    P_SetExprFlags (op12, EF_UNSIGNED);
		  }
		else
		  {
		    if (P_GetExprOpcode (op1) == OP_add)
		      P_SetExprScalar (op12, (-P_GetExprScalar (op2) + \
					      P_GetExprScalar (op12)));
		    else
		      P_SetExprScalar (op12,(-P_GetExprScalar (op2) - \
					     P_GetExprScalar (op12)));
		  }

		P_SetExprOpcode (op1, OP_add);
		P_SetExprSibling (op1, NULL);

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op2, NULL);
		op2 = PST_RemoveExpr (table, op2);

		/* 1/16/04 REK op1 was reduced above, so this is unnecessary.
		 */
		new = op1;

#if 0
		new = PST_ReduceExpr (table, op1);
		op1 = PST_RemoveExpr (table, op1);
#endif
	      }
	    else if ((P_GetExprOpcode (op11) == OP_int) && \
		     TYPE_INTEGRAL (PST_GetTypeBasicType (table, type12)))
	      {
		/* 5) (N1 +/- A) - N2  => <N1 - N2> +/- A  
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op11) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op2) & EF_UNSIGNED))
		  {
		    P_SetExprScalar (op11,
				     ((ITuintmax)P_GetExprScalar (op11) - \
				      (ITuintmax)P_GetExprScalar (op2)));
		    
		    P_SetExprFlags (op11, EF_UNSIGNED);
		  }
		else
		  {
		    P_SetExprScalar (op11, (P_GetExprScalar (op11) - \
					    P_GetExprScalar (op2)));
		  }
		    
		P_SetExprSibling (op1, NULL);

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op2, NULL);
		op2 = PST_RemoveExpr (table, op2);

		/* 1/16/04 REK op1 was reduced above, so this is unnecessary.
		 */
		new = op1;

#if 0
		new = PST_ReduceExpr (table, op1);
		op1 = PST_RemoveExpr (table, op1);
#endif
	      }
	  }
	else if ((P_GetExprOpcode (op2) == OP_add || \
		  P_GetExprOpcode (op1) == OP_sub) && \
		 (P_GetExprOpcode (op1) == OP_int))
	  {
	    Expr op21, op22;
	    Type type21, type22;

	    op21 = P_GetExprOperands (op2);
	    op22 = P_GetExprSibling (op21);

	    type21 = PST_ExprType (table, op21);
	    type22 = PST_ExprType (table, op22);
	    
	    if (P_GetExprOpcode (op22) == OP_int && \
		TYPE_INTEGRAL (PST_GetTypeBasicType (table, type22)))
	      {
		/* 6) N1 - (A +/- N2)  => <N1 -/+ N2> - A 
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op1) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op22) & EF_UNSIGNED))
		  {
		    if (P_GetExprOpcode (op2) == OP_add)
		      P_SetExprScalar (op22,
				       ((ITuintmax)P_GetExprScalar (op1) - \
					(ITuintmax)P_GetExprScalar (op22)));
		    else
		      P_SetExprScalar (op22,
				       ((ITuintmax)P_GetExprScalar (op1) + \
					(ITuintmax)P_GetExprScalar (op22)));

		    P_SetExprFlags (op22, EF_UNSIGNED);
		  }
		else
		  {
		    if (P_GetExprOpcode (op2) == OP_add)
		      P_SetExprScalar (op22, (P_GetExprScalar (op1) - \
					      P_GetExprScalar (op22)));
		    else
		      P_SetExprScalar (op22, (P_GetExprScalar (op1) + \
					      P_GetExprScalar (op22)));
		  }

		P_SetExprOpcode (op2, OP_sub);

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op1, NULL);
		op1 = PST_RemoveExpr (table, op1);

		/* 1/16/04 REK op2 was reduced above, so this is unnecessary.
		 */
		new = op2;

#if 0
		new = PST_ReduceExpr (table, op2);
		op2 = PST_RemoveExpr (table, op2);
#endif
	      }
	    else if ((P_GetExprOpcode (op21) == OP_int) && \
		     TYPE_INTEGRAL (PST_GetTypeBasicType (table, type22)))
	      {
		/* 7) N1 - (N2 +/- A)  => <N1 - N2> -/+ A 
		 * (A integer; N1, N2 int consts) */
		if ((P_GetExprFlags (op1) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op21) & EF_UNSIGNED))
		  {
		    P_SetExprScalar (op21,
				     ((ITuintmax)P_GetExprScalar (op1) - \
				      (ITuintmax)P_GetExprScalar (op21)));

		    P_SetExprFlags (op21, EF_UNSIGNED);
		  }
		else
		  {
		    P_SetExprScalar (op21, (P_GetExprScalar (op1) + \
					    P_GetExprScalar (op21)));
		  }

		P_SetExprOperands (new, NULL);
		new = PST_RemoveExpr (table, new);
		P_SetExprSibling (op1, NULL);
		op1 = PST_RemoveExpr (table, op1);

		if (P_GetExprOpcode (op2) == OP_add)
		  P_SetExprOpcode (op2, OP_sub);
		else
		  P_SetExprOpcode (op2, OP_add);
		
		/* 1/16/04 REK op2 was reduced above, so this is unnecessary.
		 */
		new = op2;

#if 0
		new = PST_ReduceExpr (table, op2);
		op2 = PST_RemoveExpr (table, op2);
#endif
	      }
	  }
      }
      break;

    case OP_mul:
      {
	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) A * 0        => 0  (if A is side-effect free)
	 *  2) 0 * B        => 0  (if B is side-effect free)
	 *  3) A * 1        => A
	 *  4) 1 * B        => B
	 *  5) N1 * N2      => <N1 * N2>  (N1, N2 constants)
	 */

	if ((P_IsConstZero (op2) && P_IsSideEffectFree (op1)) || \
	    (P_IsConstZero (op1) && P_IsSideEffectFree (op2)))
	  {
	    PST_CastExpr (table, new);
	    if (PST_IsRealType (table, new->type))
	      {
		if (P_IsFloatExpr (op1) && P_IsFloatExpr (op2))
		  temp = PST_ScopeNewFloatExpr (table, scope_key, 0.0);
		else
		  temp = PST_ScopeNewDoubleExpr (table, scope_key, 0.0);
	      }
	    else
	      {
		temp = PST_ScopeNewIntExpr (table, scope_key, 0);
		PST_SetExprType (table, temp, PST_ExprType (table, new));
		if (P_GetExprFlags (new) & EF_UNSIGNED)
		  P_SetExprFlags (new, EF_UNSIGNED);
	      }

	    new = PST_RemoveExpr (table, new);
	    new = temp;
	  }
	else if (P_IsConstOne (op2))
	  {			/* A */
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op1 from the Expr and cast it. */
	    P_SetExprSibling (op1, NULL);
	    temp = cast (table, op1, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsConstOne (op1))
	  {			/* B */
	    expr_type = PST_ExprType (table, new);

	    temp = cast (table, op2, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op1, NULL);
	    op1 = PST_RemoveExpr (table, op1);

	    new = temp;
	  }
	else if (P_IsConstNumber (op1) && P_IsConstNumber (op2))
	  {
	    int scalar = 1;	/* expect all integral values. */
	    double d1 = 0.0, d2 = 0.0;
	    ITintmax l1 = 0, l2 = 0;

	    if (P_IsRealExpr (op1))
	      {
		scalar = 0;
		d1 = P_GetExprReal (op1);
	      }
	    else if (P_IsIntegralExpr (op1))
	      {
		l1 = P_IntegralExprValue (op1);
		d1 = (double) l1;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 1 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }

	    if (P_IsRealExpr (op2))
	      {
		scalar = 0;
		d2 = P_GetExprReal (op2);
	      }
	    else if (P_IsIntegralExpr (op2))
	      {
		l2 = P_IntegralExprValue (op2);
		d2 = (double) l2;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 2 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }

	    if (scalar)
	      {
		ITintmax mval;
		Type rtype = PST_DominantType (table,
					       PST_ExprType (table, op1),
					       PST_ExprType (table, op2));

		if (PST_GetTypeBasicType (table, rtype) & BT_UNSIGNED)
		  mval = (ITuintmax) l1 * (ITuintmax) l2;
		else
		  mval = l1 * l2;

		temp = PST_ScopeNewIntExpr (table, scope_key, mval);
		PST_SetExprType (table, temp, rtype);
		if (PST_GetTypeBasicType (table, rtype) & BT_UNSIGNED)
		  P_SetExprFlags (temp, EF_UNSIGNED);
	      }
	    else
	      {
		double mval = d1 * d2;

		if (P_IsFloatExpr (op1) && P_IsFloatExpr (op2))
		  temp = PST_ScopeNewFloatExpr (table, scope_key, mval);
		else
		  temp = PST_ScopeNewDoubleExpr (table, scope_key, mval);
	      }

	    new = PST_RemoveExpr (table, new);
	    new = temp;
	  }
      }
      break;

    case OP_div:
      {
	double d1 = 0.0, d2 = 0.0;
	ITintmax l1 = 0, l2 = 0;

	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) A / 1        => A
	 *  2) A / 0        => report error
	 *  3) N1 / N2      => <N1 / N2>    (N1, N2 constants)
	 */
	if (P_IsConstOne (op2))
	  {
	    /* We need to inspect both operands of new to get its type,
	     * so evaluate the type before pulling the Expr apart. */
	    expr_type = PST_ExprType (table, new);

	    /* Remove op1 from the Expr and cast it. */
	    P_SetExprSibling (op1, NULL);
	    temp = cast (table, op1, expr_type);

	    P_SetExprOperands (new, NULL);
	    new = PST_RemoveExpr (table, new);
	    P_SetExprSibling (op2, NULL);
	    op2 = PST_RemoveExpr (table, op2);

	    new = temp;
	  }
	else if (P_IsConstZero (op2))
	  {
	    P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s constant folding: "
		    "division by zero found", __LINE__ - 1,
		    op_to_value[opcode]);
	  }
	else if (P_IsConstNumber (op1) && P_IsConstNumber (op2))
	  {
	    int scalar = 1;	/* expect all integral values. */

	    if (P_IsRealExpr (op1))
	      {
		scalar = 0;
		d1 = P_GetExprReal (op1);
	      }
	    else if (P_IsIntegralExpr (op1))
	      {
		l1 = P_IntegralExprValue (op1);
		d1 = (double) l1;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 1 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }

	    if (P_IsRealExpr (op2))
	      {
		scalar = 0;
		d2 = op2->value.real;
	      }
	    else if (P_IsIntegralExpr (op2))
	      {
		l2 = P_IntegralExprValue (op2);
		d2 = (double) l2;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 2 "
			"matches P_IsConstNumber\nbut not P_IsRealExpr or "
			"P_IsIntegralExpr", __LINE__ - 2, op_to_value[opcode]);
	      }

	    if (scalar)
	      {
		if ((P_GetExprFlags (op1) & EF_UNSIGNED) || \
		    (P_GetExprFlags (op2) & EF_UNSIGNED))
		  {
		    temp = PST_ScopeNewIntExpr (table, scope_key,
						(ITuintmax)l1 / (ITuintmax)l2);
		    PST_SetExprType (table, temp, PST_ExprType (table, new));
		    if (P_GetExprFlags (new) & EF_UNSIGNED)
		      P_SetExprFlags (new, EF_UNSIGNED);
		  }
		else
		  {
		    temp = PST_ScopeNewIntExpr (table, scope_key, l1 / l2);
		    PST_SetExprType (table, temp, PST_ExprType (table, new));
		  }
	      }
	    else
	      {
		if (P_IsFloatExpr (op1) && P_IsFloatExpr (op2))
		  temp = PST_ScopeNewFloatExpr (table, scope_key, d1 / d2);
		else
		  temp = PST_ScopeNewDoubleExpr (table, scope_key, d1 / d2);
	      }

	    new = PST_RemoveExpr (table, new);
	    new = temp;
	  }
      }
      break;

    case OP_mod:
      {
	ITintmax l1, l2;

	op1 = P_GetExprOperands (new);
	op2 = P_GetExprSibling (op1);

	/*  1) A % 0        => report error
	 *  2) 0 % B        => 0
	 *  3) N1 % N2      => <N1 % N2>    (N1, N2 integer constants)
	 */
	if (P_IsConstZero (op2))
	  {
	    P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s constant folding: "
		    "mod by zero found", __LINE__ - 1, op_to_value[opcode]);
	  }
	else if (P_IsConstZero (op1))
	  {
	    new = PST_RemoveExpr (table, new);
	    new = PST_ScopeNewIntExpr (table, scope_key, 0);
	  }
	else if (P_IsIntegralExpr (op1) && P_IsIntegralExpr (op2))
	  {
	    l1 = P_IntegralExprValue (op1);
	    l2 = P_IntegralExprValue (op2);

	    new = PST_RemoveExpr (table, new);
	    new = PST_ScopeNewIntExpr (table, scope_key, l1 % l2);
	  }
      }
      break;

    case OP_neg:
      {
	op1 = P_GetExprOperands (new);

	/* 1) -(N1) => <-N1>        (N1 const) */
	if (P_IsConstNumber (op1))
	  {
	    if (P_IsIntegralExpr (op1))
	      {
		temp = PST_ScopeNewIntExpr (table, scope_key,
					    -P_IntegralExprValue (op1));
		PST_SetExprType (table, temp, PST_ExprType (table, new));
		if (P_GetExprFlags (new) & EF_UNSIGNED)
		  P_SetExprFlags (new, EF_UNSIGNED);

		new = PST_RemoveExpr (table, new);
		new = temp;
	      }
	    else if (P_IsRealExpr (op1))
	      {
		/* BCC - 8/4/96 
		   temp_expr = NewRealExpr(- op1->value.real);
		*/
		if (P_IsFloatExpr (op1))
		  temp = PST_ScopeNewFloatExpr (table, scope_key,
						-P_GetExprReal (op1));
		else
		  temp = PST_ScopeNewDoubleExpr (table, scope_key,
						 -P_GetExprReal (op1));
		PST_SetExprType (table, temp, PST_ExprType (table, new));
		
		new = PST_RemoveExpr (table, new);
		new = temp;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 1 "
			"matches P_IsConstNumber\nbut not P_IsIntegralExpr or "
			"P_IsRealExpr", __LINE__ - 2, op_to_value[opcode]);
	      }
	  }
      }
      break;

    case OP_not:
      {
	op1 = P_GetExprOperands (new);

	/* 1) !(N1) => <!N1>        (N1 const) */
	if (P_IsConstNumber (op1))
	  {
	    if (P_IsIntegralExpr (op1))
	      {
		temp = PST_ScopeNewIntExpr (table, scope_key,
					    !P_IntegralExprValue (op1));
		PST_SetExprType (table, temp, PST_ExprType (table, new));
		if (P_GetExprFlags (new) & EF_UNSIGNED)
		  P_SetExprFlags (new, EF_UNSIGNED);

		new = PST_RemoveExpr (table, new);
		new = temp;
	      }
	    else if (P_IsRealExpr (op1))
	      {
		temp = PST_ScopeNewIntExpr (table, scope_key,
					    !P_GetExprReal (op1));
		PST_SetExprType (table, temp, PST_ExprType (table, new));

		new = PST_RemoveExpr (table, new);
		new = temp;
	      }
	    else
	      {
		P_punt ("reduce_symtab.c:PST_ReduceExpr:%d %s operand 1 "
			"matches P_IsConstNumber\nbut not P_IsIntegralExpr or "
			"P_IsRealExpr", __LINE__ - 2, op_to_value[opcode]);
	      }
	  }
      }
      break;

    case OP_inv:
      {
	op1 = P_GetExprOperands (new);

	/* 1) ~(N1) => <~N1>        (N1 const) */
	if (P_IsIntegralExpr (op1))
	  {
	    temp = PST_ScopeNewIntExpr (table, scope_key,
					~P_IntegralExprValue (op1));
	    PST_SetExprType (table, temp, PST_ExprType (table, new));
	    if (P_GetExprFlags (new) & EF_UNSIGNED)
	      P_SetExprFlags (new, EF_UNSIGNED);

	    new = PST_RemoveExpr (table, new);
	    new = temp;
	  }
      }
      break;

    case OP_cast:
      {
	Type orig_type, cast_type;
	ITintmax l1;

	op1 = P_GetExprOperands (new);
	
	orig_type = PST_ExprType (table, op1);
	cast_type = PST_ExprType (table, new);

	/*
	 *  Do not optimize for non-scalars (including pointers).
	 *  But during the RTL generation phase, we can convert
	 *  all pointers to 0 to (int) 0
	 */
	if (P_IsConstNumber (op1) && \
	    !(PST_IsPointerType (table, cast_type) || \
	      PST_IsFunctionType (table, cast_type) || \
	      PST_IsArrayType (table, cast_type)))
	  {
	    _BasicType cast_basic_type;

	    cast_basic_type = PST_GetTypeBasicType (table,
						    cast_type) & BT_TYPE;
	    /*
	     *        We can optimize (int)->(double)
	     *        and (double)->(int) conversions.
	     */
	    if (P_IsIntegralExpr (op1))
	      {			/* int->double */
		l1 = P_IntegralExprValue (op1);

		if (TYPE_REAL (cast_basic_type))
		  {		/* int->double */
		    if (PST_IsFloatType (table, cast_type))
		      {
			/* int -> float */
			temp = PST_ScopeNewFloatExpr (table, scope_key,
						      (double) l1);

			/* 12/16/03 REK This looks to be incorrect.  Fiddling
			 *              with the type should be unnecessary
			 *              after using NewFloatExpr, and
			 *              overwriting value.real isn't correct.
			 *              value.real should be set by
			 *              NewFloatExpr and is undefined on new,
			 *              which is of type OP_cast. */
#if 0
			ppc->type->type &= ~TY_DOUBLE;
			ppc->type->type |= TY_FLOAT;
			ppc->value.real = (double) new->value.real;
#endif
		      }
		    else
		      {
			/* int -> double */
			temp = PST_ScopeNewDoubleExpr (table, scope_key,
						      (double) l1);
		      }

		    new = PST_RemoveExpr (table, new);
		    new = temp;
		  }
		else
		  {
		    ITintmax mask;
		    int cast_size;

		    /* 12/16/03 REK Changing this to use the size field
		     *              of the Type.  I added some comments
		     *              inside the #if'ed out block as well. */
		    /* We're changing the size of an integer.  The size field
		     * of the destination (cast) type tells how many bytes
		     * to preserve. */
		    
		    cast_size = PST_GetTypeSize (table, cast_type);
		    mask = prepare_int_mask (cast_size);
		    if (!(cast_basic_type & BT_UNSIGNED) && \
			(MOST_SIG_BIT (l1, cast_size) == 1))
		      {
			/* If we're changing to a signed type and our value
			 * should be negative, sign extend the value. */
			l1 |= ~mask;
		      }
		    else
		      {
			l1 &= mask;
		      }
		      
		    if (cast_basic_type &
			(BT_LONGLONG | BT_LONG | BT_INT | BT_SHORT | BT_CHAR))
		      {
			temp = PST_CopyExpr (table, op1);
			P_SetExprScalar (temp, l1);
			PST_SetExprType (table, temp,
					 PST_FindTypeSetQualifier (table,
								   cast_type,
								   TY_CONST));
			
			new = PST_RemoveExpr (table, new);
			new = temp;
		      }
		  }
	      }
	    else
	      {
		if (P_IsRealExpr (op1))
		  {		/* double->int */
		    double d1 = P_GetExprReal (op1);

		    if (cast_basic_type & \
			(BT_SHORT | BT_INT | BT_LONG | BT_LONGLONG))
		      {
			temp = PST_ScopeNewIntExpr (table, scope_key,
						    (ITintmax)d1);

			if (cast_basic_type & BT_UNSIGNED)
			  {
			    PST_SetExprType (table, temp,
					     PST_FindTypeSetUnsigned \
					       (table,
						PST_ExprType (table, temp)));
			    P_SetExprFlags (temp, EF_UNSIGNED);
			  }

			new = PST_RemoveExpr (table, new);
			new = temp;
		      }
		    else if (TYPE_REAL (cast_basic_type))
		      {
			if (PST_IsFloatType (table, orig_type) && \
			    PST_IsDoubleType (table, cast_type))
			  {
			    /* float -> double */
			    temp = PST_ScopeNewDoubleExpr (table,
							   scope_key, d1);

			    new = PST_RemoveExpr (table, new);
			    new = temp;
			  }
			else
			  if (PST_IsFloatType (table, cast_type) && \
			      PST_IsDoubleType (table, orig_type))
			  {
			    /* double -> float */
			    temp = PST_ScopeNewFloatExpr (table,
							  scope_key, d1);

			    new = PST_RemoveExpr (table, new);
			    new = temp;
			  }
			else
			  {
			    temp = op1;

			    P_SetExprOperands (new, NULL);
			    new = PST_RemoveExpr (table, new);
			    new = temp;
			  }
		      }
		  }
	      }
	  }
      }
      break;

    default:
      /* no compression */
      break;
    }

  return (new);
}

