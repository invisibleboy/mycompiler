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
 * \brief Function declarations to perform queries on Pcode structures.
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

#ifndef _PCODE_QUERY_SYMTAB_H_
#define _PCODE_QUERY_SYMTAB_H_

#include <config.h>
#include "pcode.h"
#include "symtab.h"
#include "query.h"

extern Type PST_ExprType (SymbolTable table, Expr expr);
extern Type PST_DominantType (SymbolTable symbol_table, Type a, Type b);
extern bool PST_MatchFuncDcl (SymbolTable tableA, FuncDcl a,
			      SymbolTable tableB, FuncDcl b);

/*! \brief If FuncDcls match, returns 1.
 *
 * \param t
 *  the symbol table that contains the first FuncDcl.
 * \param k
 *  the key of the first FuncDcl to compare.
 * \param u
 *  the symbol table that contains the second FuncDcl.
 * \param l
 *  the key of the second FuncDcl to compare.
 *
 * \return
 *  If the FuncDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), PST_MatchTypeDcl(), #PST_MatchTypeDclK(),
 * PST_MatchStructDcl(), #PST_MatchStructDclK(), PST_MatchUnionDcl(),
 * #PST_MatchUnionDclK(), PST_MatchField(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), PST_MatchExpr(),
 * PST_MatchParam(), #P_MatchKey() */
#define PST_MatchFuncDclK(t, k, u, l) \
          (PST_MatchFuncDcl ((t), PST_GetFuncDclEntry ((t), (k)), \
                             (u), PST_GetFuncDclEntry ((u), (l))))

extern bool PST_MatchTypeDcl (SymbolTable tableA, TypeDcl a,
			      SymbolTable tableB, TypeDcl b);
extern bool PST_MatchType (SymbolTable tableA, Type a,
			   SymbolTable tableB, Type b);

/*! \brief If the BasicType of both Types match, returns TRUE.
 *
 * \param t
 *  the symbol table that contains Type \a a.
 * \param a
 *  the first Type to compare.
 * \param u
 *  the symbol table that contains Type \a b.
 * \param b
 *  the second Type to compare.
 *
 * \return
 *  If the BasicTypes of both Types match, returns TRUE.  Otherwise,
 *  returns FALSE.
 */
#define PST_MatchTypeBasicType(t, a, u, b) \
          (PST_GetTypeBasicType ((t), (a)) == PST_GetTypeBasicType ((u), (b)))

extern bool PST_MatchStructDcl (SymbolTable tableA, StructDcl a,
				SymbolTable tableB, StructDcl b);

/*! \brief If StructDcls match, returns TRUE.
 *
 * \param t
 *  the symbol table that contains the first StructDcl.
 * \param k
 *  the key of the first StructDcl to compare.
 * \param u
 *  the symbol table that contains the second StructDcl.
 * \param l
 *  the key of the second StructDcl to compare.
 *
 * \return
 *  If the StructDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchTypeDclK(), PST_MatchStructDcl(), PST_MatchUnionDcl(),
 * #PST_MatchUnionDclK(), PST_MatchField(), #PST_MatchField(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
#define PST_MatchStructDclK(t, k, u, l) \
          (PST_MatchStructDcl ((t), PST_GetStructDclEntry ((t), (k)), \
                               (u), PST_GetStructDclEntry ((u), (l))))

extern bool PST_MatchUnionDcl (SymbolTable tableA, UnionDcl a,
			       SymbolTable tableB, UnionDcl b);

/*! \brief If UnionDcls match, returns TRUE.
 *
 * \param t
 *  the symbol table that contains the first UnionDcl.
 * \param k
 *  the key of the first UnionDcl to compare.
 * \param u
 *  the symbol table that contains the second UnionDcl.
 * \param l
 *  the key of the second UnionDcl to compare.
 *
 * \return
 *  If the UnionDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchTypeDclK(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), PST_MatchField(), #PST_MatchFieldK(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
#define PST_MatchUnionDclK(t, k, u, l) \
          (PST_MatchUnionDcl ((t), PST_GetUnionDclEntry ((t), (k)), \
                              (u), PST_GetUnionDclEntry ((u), (l))))

extern bool PST_MatchField (SymbolTable tableA, Field a,
			    SymbolTable tableB, Field b);

/*! \brief If Fields match, returns TRUE.
 *
 * \param t
 *  the symbol table that contains the first Field.
 * \param k
 *  the key of the first Field to compare.
 * \param u
 *  the symbol table that contains the second Field.
 * \param l
 *  the key of the second Field to compare.
 *
 * \return 
 *  If the Fields match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchTypeDclK(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), #PST_MatchUnionDclK(), PST_MatchField(),
 * PST_MatchVarList(), PST_MatchVarDcl(), #PST_MatchVarDclK(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
#define PST_MatchFieldK(t, k, u, l) \
          (PST_MatchField ((t), PST_GetFieldEntry ((t), (k)), \
                           (u), PST_GetFieldEntry ((u), (l))))

extern bool PST_MatchVarList (SymbolTable tableA, VarList a,
			      SymbolTable tableB, VarList b);
extern bool PST_MatchVarDcl (SymbolTable tableA, VarDcl a,
			     SymbolTable tableB, VarDcl b);

/*! \brief If VarDcls match, returns TRUE.
 *
 * \param t
 *  the symbol table that contains the first VarDcl.
 * \param k
 *  the key of the first VarDcl to compare.
 * \param u
 *  the symbol table that contains the second VarDcl.
 * \param l
 *  the key of the second VarDcl to compare.
 *
 * \return
 *  If the VarDcls match, returns TRUE.  Otherwise, returns FALSE.
 *
 * \sa PST_MatchFuncDcl(), #PST_MatchFuncDclK(), PST_MatchTypeDcl(),
 * #PST_MatchTypeDclK(), PST_MatchStructDcl(), #PST_MatchStructDclK(),
 * PST_MatchUnionDcl(), #PST_MatchUnionDclK(), PST_MatchField(),
 * #PST_MatchFieldK(), PST_MatchVarList(), PST_MatchVarDcl(),
 * PST_MatchExpr(), PST_MatchParam(), #P_MatchKey() */
#define PST_MatchVarDclK(t, k, u, l) \
          (PST_MatchVarDcl ((t), PST_GetVarDclEntry ((t), (k)), \
                            (u), PST_GetVarDclEntry ((u), (l))))

extern bool PST_MatchExpr (SymbolTable tableA, Expr a,
			   SymbolTable tableB, Expr b);
extern bool PST_MatchParam (SymbolTable tableA, Param a,
			    SymbolTable tableB, Param b);

extern bool PST_IsVoidType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a void type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to an integral type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsVoidTypeDcl(), PST_IsVoidType(), PST_ExprType() */
#define PST_IsVoidTypeExpr(t, e) \
          (PST_IsVoidType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsIntegralType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to an integral type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to an integral type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsIntegralTypeDcl(), PST_IsIntegralType(), PST_ExprType() */
#define PST_IsIntegralTypeExpr(t, e) \
          (PST_IsIntegralType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsRealType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a real type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a real type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsRealTypeDcl(), PST_IsRealType(), PST_ExprType() */
#define PST_IsRealTypeExpr(t, e) \
          (PST_IsRealType ((t), PST_ExprType((t), (e))))

extern bool PST_IsFloatType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a float type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a float type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsFloatTypeDcl(), PST_IsFloatType(), PST_ExprType() */
#define PST_IsFloatTypeExpr(t, e) \
          (PST_IsFloatType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsDoubleType (SymbolTable table, Type type);
extern bool PST_IsLongLongType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a double type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a double type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsDoubleTypeDcl(), PST_IsDoubleType(), PST_ExprType() */
#define PST_IsDoubleTypeExpr(t, e) \
          (PST_IsDoubleType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsArithmeticType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to an arithmetic type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to an arithmetic type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsArithmeticTypeDcl(), PST_IsArithmeticType(), PST_ExprType() */
#define PST_IsArithmeticTypeExpr(t, e) \
          (PST_IsArithmeticType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsPointerType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a pointer type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a pointer type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsPointerType(), #PST_IsPointerTypeK(), PST_ExprType() */
#define PST_IsPointerTypeExpr(t, e) \
          (PST_IsPointerType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsFundamentalType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a fundamental type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a fundamental type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsFundamentalTypeDcl(), PST_IsFundamentalType(), PST_ExprType() */
#define PST_IsFundamentalTypeExpr(t, e) \
          (PST_IsFundamentalType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsStructureType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a structure.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a struture, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsStructureTypeDcl(), PST_IsStructureType(), PST_ExprType() */
#define PST_IsStructureTypeExpr(t, e) \
          (PST_IsStructureType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsArrayType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to an array type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to an array type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsArrayTypeDcl(), PST_IsArrayType(), PST_ExprType() */
#define PST_IsArrayTypeExpr(t, e) \
           (PST_IsArrayType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsFunctionType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a function type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a function type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsFunctionTypeDcl(), PST_IsFunctionType(), PST_ExprType() */
#define PST_IsFunctionTypeExpr(t, e) \
          (PST_IsFunctionType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsSignedType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a signed type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a signed type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsSignedTypeDcl(), PST_IsSignedType(), PST_ExprType() */
#define PST_IsSignedTypeExpr(t, e) \
          (PST_IsSignedType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsUnsignedType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to an unsigned type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to an unsigned type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsUnsignedTypeDcl(), PST_IsUnsignedType(), PST_ExprType() */
#define PST_IsUnsignedTypeExpr(t, e) \
          (PST_IsUnsignedType ((t), PST_ExprType ((t), (e))))

extern bool PST_IsEnumType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to an enum type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to an enum type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsEnumTypeDcl(), PST_IsEnumType(), PST_ExprType() */
#define PST_IsEnumTypeExpr(t, e) \
          (PST_IsEnumType ((t) , PST_ExprType ((t), (e))))

extern bool PST_IsVarargType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a vararg type.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a vararg type, returns TRUE.  Otherwise,
 *  returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsVarargTypeDcl(), PST_IsVarargType(), PST_ExprType() */
#define PST_IsVarargTypeExpr(t, e) \
          (PST_IsVarargType ((t) , PST_ExprType ((t), (e))))

extern bool PST_IsBitFieldExpr (SymbolTable table, Expr expr);

extern bool PST_IsBaseType (SymbolTable table, Type type);

/*! \brief Checks if an Expr evaluates to a base type (not a pointer, func,
 *         or array)
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to check.
 *
 * \return
 *  If the Expr evaluates to a base type (not a pointer, func, or array),
 *  returns TRUE.  Otherwise, returns FALSE.
 *
 * \note \a t is used more than once.
 *
 * \sa P_IsBaseTypeDcl(), PST_IsBaseType(), PST_ExprType() */
#define PST_IsBaseTypeExpr(t, e) \
          (PST_IsBaseType ((t), PST_ExprType ((t), (e))))

extern bool PST_EqualStrengthType (SymbolTable table, Type a, Type b);

extern int PST_GetFieldContainerOffset (SymbolTable pst, Field field);
#endif
