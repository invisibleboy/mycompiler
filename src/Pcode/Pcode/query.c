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
 */
/*****************************************************************************/

#include <config.h>
#include <string.h>
#include "pcode.h"
#include "query.h"
#include "struct.h"
#include "symtab.h"
#include "struct_symtab.h"
#include "impact_global.h"

/*! \brief Finds a pragma in a list.
 *
 * \param pragma_list
 *  the pragma list to search
 * \param specifier
 *  the specifier to search for in the list.
 *
 * \return
 *  The pragma matching the specifier, or NULL if none is found.
 */
Pragma
P_FindPragma (Pragma pragma_list, char *specifier)
{
  Pragma p = NULL;

  for (p = pragma_list; p; p = P_GetPragmaNext (p))
    {
      if (strcmp (P_GetPragmaSpecifier (p), specifier) == 0)
	break;
    }

  return (p);
}


/*! \brief Finds a pragma with a given prefix in a list.
 *
 * \param pragma_list
 *  the pragma list to search
 * \param specifier
 *  the specifier to search for in the list.
 *
 * \return
 *  The pragma matching the specifier, or NULL if none is found.
 */
Pragma
P_FindPragmaPrefix (Pragma pragma_list, char *specifier)
{
  int n;
  Pragma p = NULL;

  n = strlen (specifier);

  for (p = pragma_list; p; p = P_GetPragmaNext (p))
    {
      if (strncmp (P_GetPragmaSpecifier (p), specifier, n) == 0)
	break;
    }

  return (p);
}


/*! \brief Finds a key in a KeyList.
 *
 * \param key_list
 *  the KeyList to search.
 * \param key
 *  the Key to find in the KeyList.
 *
 * \return
 *  The KeyList node containing \a key, or NULL if \a key does not exist.
 */
KeyList
P_FindKeyListKey (KeyList key_list, Key key)
{
  KeyList result = NULL;

  while (key_list)
    {
      if (P_MatchKey (P_GetKeyListKey (key_list), key))
	{
	  result = key_list;
	  break;
	}

      key_list = P_GetKeyListNext (key_list);
    }

  return (result);
}

/*! \brief Checks if a TypeDcl is void.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is void, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsVoidType(), #PST_IsVoidTypeExpr()
 */
bool
P_IsVoidTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_VOID))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_VOID)
    P_punt ("query.c:P_IsVoidTypeDcl:%d illegal void type 0x%x", __LINE__,
	    P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Checks if a TypeDcl is integral.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is integral, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsIntegralType(), #PST_IsIntegralTypeExpr()
 */
bool 
P_IsIntegralTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!TYPE_INTEGRAL (P_GetTypeDclBasicType (type_dcl)))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~(BT_INTEGRAL | BT_BIT_FIELD))
    P_punt ("query.c:P_IsIntegralTypeDcl:%d illegal integral type 0x%x",
	    __LINE__ - 1, P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Check if a TypeDcl is real.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is real, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsRealType(), #PST_IsRealTypeExpr()
 */
bool 
P_IsRealTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!TYPE_REAL (P_GetTypeDclBasicType (type_dcl) & BT_TYPE))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_REAL)
    P_punt ("query.c:P_IsRealTypeDcl:%d illegal real type 0x%x", __LINE__,
	    P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Check if a TypeDcl is a float.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is a float, return TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsFloatType(), #PST_IsFloatExpr()
 */
bool
P_IsFloatTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_FLOAT))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_FLOAT)
    P_punt ("query.c:P_IsFloatTypeDcl:%d illegal float type 0x%x", __LINE__,
	    P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Check is a TypeDcl is a double.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is a double, return TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsDoubleType(), #PST_IsDoubleExpr()
 */
bool
P_IsDoubleTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_DOUBLE))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_DOUBLE)
    P_punt ("query.c:P_IsDoubleTypeDcl:%d illegal double type 0x%x", __LINE__,
	    P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Check if a TypeDcl is arithmetic.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is arithmetic, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa #PST_IsArithmeticType(), #PST_IsArithmeticTypeExpr()
 */
bool 
P_IsArithmeticTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_ARITHMETIC))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_ARITHMETIC)
    P_punt ("query.c:P_IsArithmeticTypeDcl:%d illegal arithmetic type 0x%x",
	    __LINE__ - 1, P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Check if a TypeDcl is a pointer.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is a pointer, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsPointerType(), #PST_IsPointerTypeExpr()
 */
bool 
P_IsPointerTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_POINTER))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_POINTER)
    P_punt ("query.c:P_IsPointerTypeDcl:%d illegal pointer type 0x%x",
	    __LINE__ - 1, P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Check if a TypeDcl is fundamental.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is fundamental, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsFundamentalType(), #PST_IsFundamentalTypeExpr()
 */
bool 
P_IsFundamentalTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  
  return (P_IsPointerTypeDcl (type_dcl) || P_IsArrayTypeDcl (type_dcl) || \
	  P_IsFunctionTypeDcl (type_dcl) || P_IsArithmeticTypeDcl (type_dcl));
}

/*! \brief Check if a TypeDcl is a structure.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is a structure, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsStructureType(), #PST_IsStructureTypeExpr()
 */
bool 
P_IsStructureTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_STRUCTURE))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_STRUCTURE)
    P_punt ("query.c:P_IsStructureTypeDcl:%d illegal structure type 0x%x",
	    __LINE__ - 1, P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Check if a TypeDcl is an array.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is an array,  returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsArrayType(), #PST_IsArrayTypeExpr()
 */
bool 
P_IsArrayTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_ARRAY))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_ARRAY)
    P_punt ("query.c:P_IsArrayTypeDcl:%d illegal array type 0x%x", __LINE__,
	    P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief check if a TypeDcl is a function.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is a function, return TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsFunctionType(), #PST_IsFunctionTypeExpr()
 */
bool 
P_IsFunctionTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_FUNC))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_FUNC)
    P_punt ("query.c:P_IsFunctionTypeDcl:%d illegal function type 0x%x",
	    __LINE__ - 1, P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Checks if a TypeDcl is signed.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is signed, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsSignedType(), #PST_IsSignedTypeExpr() */
bool
P_IsSignedTypeDcl (TypeDcl type_dcl)
{
  _BasicType bt;

  if (type_dcl == NULL)
    return (FALSE);

  bt = P_GetTypeDclBasicType (type_dcl);

  if ((bt & BT_INTEGRAL) && (!(bt & BT_UNSIGNED)))
    return (TRUE);

  return (FALSE);
}

/*! \brief Checks if a TypeDcl is unsigned.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is unsigned, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsUnsignedType(), #PST_IsUnsignedTypeExpr() */
bool
P_IsUnsignedTypeDcl (TypeDcl type_dcl)
{
  _BasicType bt;

  if (type_dcl == NULL)
    return (FALSE);

  bt = P_GetTypeDclBasicType (type_dcl);

  if ((bt & BT_INTEGRAL) && (bt & BT_UNSIGNED))
    return (TRUE);

  return (FALSE);
}

/*! \brief Checks if a TypeDcl is an enum type.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is an enum type, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsEnumType(), #PST_IsEnumTypeExpr() */
bool
P_IsEnumTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_ENUM))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_ENUM)
    P_punt ("query.c:P_IsEnumTypeDcl:%d illegal type 0x%x", __LINE__,
	    P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Checks if a TypeDcl is a vararg type.
 *
 * \param type_dcl
 *  the TypeDcl to check
 *
 * \return
 *  If the TypeDcl is a vararg type, return TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsVarargType(), #PST_IsVarargTypeExpr()
 */
bool
P_IsVarargTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_VARARG))
    return (FALSE);
  if (P_GetTypeDclBasicType (type_dcl) & ~BT_VARARG)
    P_punt ("query.c:P_IsVarargTypeDcl:%d illegal type 0x%x", __LINE__,
	    P_GetTypeDclBasicType (type_dcl));
  return (TRUE);
}

/*! \brief Checks if a TypeDcl is a bitfield type.
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is a bitfield type, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsBitFieldType(), #PST_IsBitFieldTypeExpr() */
bool
P_IsBitFieldTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & BT_BIT_FIELD))
    return (FALSE);
  return (TRUE);
}

/*! \brief Checks if a TypeDcl is a base type (not a pointer, func, or array)
 *
 * \param type_dcl
 *  the TypeDcl to check.
 *
 * \return
 *  If the TypeDcl is a base (not a pointer, func, or array) type,
 *  returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_IsBaseType(), #PST_IsBaseTypeExpr() */
bool
P_IsBaseTypeDcl (TypeDcl type_dcl)
{
  if (type_dcl == NULL)
    return (FALSE);
  if (!(P_GetTypeDclBasicType (type_dcl) & (BT_POINTER | BT_FUNC | BT_ARRAY)))
    return (FALSE);
  return (TRUE);
}

/*! \brief Determines if two TypeDcls are compatible.
 *
 * \param a, b
 *  the TypeDcls to compare.
 *
 * \return
 *  Returns TRUE if the two TypeDcls are compatible (same strength).
 *  Otherwise, returns FALSE.
 *
 * This function works only for arithmetic TypeDcl.
 *
 * \sa PST_EqualStrengthType()
 */
bool
P_EqualStrengthTypeDcl (TypeDcl a, TypeDcl b)
{
  if (!P_IsArithmeticTypeDcl (a) || !P_IsArithmeticTypeDcl (b))
    P_punt ("query.c:P_EqualStrengthTypeDcl:%d types must be arithmetic",
	    __LINE__ - 1);

  if (P_IsIntegralTypeDcl (a))
    {
      /* fixed-point */
      int t1, t2;

      if (!P_IsIntegralTypeDcl (b))
	return (FALSE);

      t1 = P_GetTypeDclBasicType (a) & BT_INTEGRAL;
      t2 = P_GetTypeDclBasicType (b) & BT_INTEGRAL;

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
      if (!P_IsRealTypeDcl (b))
	return (FALSE);

      if (BT_REAL & (P_GetTypeDclBasicType (a) ^ P_GetTypeDclBasicType (b)))
	return (FALSE);
    }

  return (TRUE);
}

/*! \brief If the Expr is an integer, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is an integer, returns TRUE.  Otherwise, returns FALSE.
 *
 * ignore OP_type_size and OP_expr_size.
 */
bool
P_IsIntegralExpr (Expr expr)
{
  if (expr == NULL)
    return (FALSE);

  switch (P_GetExprOpcode (expr))
    {
    case OP_char:
    case OP_int:
      return (TRUE);

    default:
      break;
    }

  return (FALSE);
}

/*! \brief Returns the value of an integer expression.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  The integer value of the Expr.
 *
 * \bug
 *  This function doesn't differentiate between signed and unsigned.
 */
ITintmax
P_IntegralExprValue (Expr expr)
{
  if (expr == NULL)
    P_punt ("query.c:P_IntegralExprValue:%d null expr", __LINE__);

  switch (P_GetExprOpcode (expr))
    {
    case OP_int:
      return (P_GetExprScalar (expr));
	   
    case OP_char:
      return ((ITintmax)P_GetExprString (expr)[0]);

    default:
      P_punt ("query.c:P_IntegralExprValue:%s Invalid opcode %d", __LINE__,
	      P_GetExprOpcode (expr));
    }

  return (0);
}

/*! \brief If the Expr is a float, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a float, returns TRUE.  Otherwise, returns FALSE.
 */
bool
P_IsFloatExpr (Expr expr)
{
  if (expr == NULL)
    return (FALSE);

  return (P_GetExprOpcode (expr) == OP_float);
}

/*! \brief If the Expr is a double, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a double, returns TRUE.  Otherwise, returns FALSE.
 */
bool
P_IsDoubleExpr (Expr expr)
{
  if (expr == NULL)
    return (FALSE);

  return (P_GetExprOpcode (expr) == OP_double);
}

/*! \brief If the Expr is a real, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a real, returns TRUE.  Otherwise, returns FALSE.
 */
bool
P_IsRealExpr (Expr expr)
{
  _Opcode opcode;

  if (expr == NULL)
    return (FALSE);

  opcode = P_GetExprOpcode (expr);

  return (opcode == OP_real || opcode == OP_double || opcode == OP_float);
}

/*! \brief Returns the value of a float expression.
 *
 * \param expr
 *  the expression to evaluate.
 *
 * \return
 *  The float value of the expression.
 */
double
P_FloatExprValue (Expr expr)
{
  return (P_GetExprReal (expr));
}

/*! \brief If the Expr is a constant one, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a constant one, returns TRUE.  Otherwise, returns FALSE.
 */
bool
P_IsConstOne (Expr expr)
{
  return ((P_IsIntegralExpr (expr) && (P_IntegralExprValue (expr) == 1)) || \
	  (P_IsRealExpr (expr) && (P_FloatExprValue (expr) == 1.0)));
}

/*! \brief If the Expr is a constant zero, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a constant zero, returns TRUE.  Otherwise, returns FALSE.
 */
bool
P_IsConstZero (Expr expr)
{
  return ((P_IsIntegralExpr (expr) && (P_IntegralExprValue (expr) == 0)) || \
	  (P_IsRealExpr (expr) && (P_FloatExprValue (expr) == 0.0)));
}

/*! \brief If the Expr is a constant non-zero, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a constant zero, returns TRUE.  Otherwise, returns FALSE.
 */
bool
P_IsConstNonZero (Expr expr)
{
  return ((P_IsIntegralExpr (expr) && (P_IntegralExprValue (expr) != 0)) || \
	  (P_IsRealExpr (expr) && (P_FloatExprValue (expr) != 0.0)));
}

/*! \brief If the Expr is a constant negative, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a constant negative, returns TRUE.  Otherwise, returns
 *  FALSE.
 */
bool
P_IsConstNegative (Expr expr)
{
  /* BCC - use IsRealExpr instead of IsFloatExpr - 8/4/96 */
  return (!(P_GetExprFlags (expr) & EF_UNSIGNED) && \
	  ((P_IsIntegralExpr (expr) && (P_IntegralExprValue (expr) < 0)) || \
	   (P_IsRealExpr (expr) && (P_FloatExprValue (expr) < 0.0))));
}

/*! \brief If the Expr has no side effects, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr has no side effects, returns TRUE.  Otherwise, returns FALSE.
 *
 * Returns true if the expression tree is completely side-effect-free.
 * The following operators cause side-effect: assign, preinc, predec,
 * postinc, postdec, Aadd, Asub, Amul, Adiv, Amod, Arshft, Alshft, Aand,
 * Aor, Axor, call.
 */
bool
P_IsSideEffectFree (Expr expr)
{
  Expr op;

  if (expr == NULL)
    return (TRUE);

  switch (P_GetExprOpcode (expr))
    {
    case OP_assign:
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
    case OP_call:
      return (FALSE);

    default:
      for (op = P_GetExprOperands (expr); op; op = P_GetExprSibling (op))
	if (!P_IsSideEffectFree (op))
	  return (FALSE);
    }

  return (TRUE);
}

/*! \brief If the Expr is a boolean expression, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a boolean expression, returns TRUE.  Otherwise, returns
 *  FALSE.
 *
 * In many cases, when an expression is known to produce boolean result, 
 * the upper (!=0) operation can be eliminated. 
 */
bool
P_IsBooleanExpr (Expr expr)
{
  Expr op1, op2;

  if (expr == NULL)
    return (FALSE);

  switch (P_GetExprOpcode (expr))
    {
    case OP_eq:
    case OP_ne:
    case OP_lt:
    case OP_le:
    case OP_gt:
    case OP_ge:
    case OP_conj:
    case OP_disj:
      return (TRUE);

    case OP_or:
    case OP_xor:
    case OP_and:
      op1 = P_GetExprOperands (expr);
      op2 = P_GetExprSibling (op1);
      return (P_IsBooleanExpr (op1) && P_IsBooleanExpr (op2));

    default:
      break;
    }

  return (FALSE);
}

/*! \brief If the Expr is a constant number, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a constant number, returns TRUE.  Otherwise, returns
 *  FALSE.
 */
bool
P_IsConstNumber (Expr expr)
{
  return (P_IsIntegralExpr (expr) || P_IsRealExpr (expr));
}

/*! \brief If the Expr is a Pcode temp variable, returns TRUE.
 *
 * \param expr
 *  the Expr to inspect.
 *
 * \return
 *  If the Expr is a Pcode temp variable, returns TRUE.  Otherwise, returns
 * FALSE.
 */
bool
P_IsPcodeTemp (Expr expr)
{
  if ((P_GetExprOpcode (expr) == OP_var) && (P_GetExprFlags (expr) & EF_TEMP))
    return (TRUE);
  else
    return (FALSE);
}
