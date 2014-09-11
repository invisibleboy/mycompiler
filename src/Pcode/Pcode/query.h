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
 */
/*****************************************************************************/

#ifndef _PCODE_QUERY_H_
#define _PCODE_QUERY_H_

#include <config.h>
#include "impact_global.h"
#include "pcode.h"
#include "struct.h"

/*! \brief Determines if a Position is valid.
 *
 * \param p
 *  the Position to test.
 *
 * \return
 *  If the Position is valid, returns 1.  Otherwise, returns 0.
 *
 * \note Macro expands its argument multiple times.
 *
 * \sa #P_ValidKey()
 */
#define P_ValidPosition(p) ((p)->lineno || (p)->colno || (p)->filename)

/*! \brief Determines if a Key is valid.
 *
 * \param k
 *  the Key to test.
 *
 * \return
 *  If the Key is valid, returns 1.  Otherwise, returns 0.
 *
 * Tests a Key to see if it is valid.  Valid means the key can appear in the
 * the symbol table, not that it does appear.
 *
 * \note Macro expands its argument multiple times.
 *
 * \sa #P_ValidPosition()
 */
#define P_ValidKey(k) ((k).file > 0 && (k).sym > 0)

/*! \brief Compares two ::Key structures.
 *
 * \param a
 *  the first key.
 * \param b
 *  the second key.
 *
 * \return
 *  If both keys are the same, returns 1.  Otherwise returns 0.
 */
#define P_MatchKey(a, b) ((a).file == (b).file && (a).sym == (b).sym)

extern Pragma P_FindPragma (Pragma pragma_list, char *specifier);
extern Pragma P_FindPragmaPrefix (Pragma pragma_list, char *specifier);
extern KeyList P_FindKeyListKey (KeyList key_list, Key key);

/*! \brief Finds a ScopeEntry in a list by its key.
 *
 * \param s
 *  the ScopeEntry.
 * \param k
 *  the Key to find.
 */
#define P_FindScopeEntryKey(s, k) \
          ((ScopeEntry)P_FindKeyListKey ((KeyList)(s), (k)))

extern bool P_IsVoidTypeDcl (TypeDcl type_dcl);
extern bool P_IsIntegralTypeDcl (TypeDcl type_dcl);
extern bool P_IsRealTypeDcl (TypeDcl type_dcl);
extern bool P_IsFloatTypeDcl (TypeDcl type_dcl);
extern bool P_IsDoubleTypeDcl (TypeDcl type_dcl);
extern bool P_IsArithmeticTypeDcl (TypeDcl type_dcl);
extern bool P_IsPointerTypeDcl (TypeDcl type_dcl);
extern bool P_IsFundamentalTypeDcl (TypeDcl type_dcl);
extern bool P_IsStructureTypeDcl (TypeDcl type_dcl);
extern bool P_IsArrayTypeDcl (TypeDcl type_dcl);
extern bool P_IsFunctionTypeDcl (TypeDcl type_dcl);
extern bool P_IsSignedTypeDcl (TypeDcl type_dcl);
extern bool P_IsUnsignedTypeDcl (TypeDcl type_dcl);
extern bool P_IsEnumTypeDcl (TypeDcl type_dcl);
extern bool P_IsVarargTypeDcl (TypeDcl type_dcl);
extern bool P_IsBitFieldTypeDcl (TypeDcl type_dcl);
extern bool P_IsBaseTypeDcl (TypeDcl type_dcl);

extern bool P_EqualStrengthTypeDcl (TypeDcl a, TypeDcl b);

extern bool P_IsIntegralExpr (Expr expr);
extern ITintmax P_IntegralExprValue (Expr expr);
extern bool P_IsFloatExpr (Expr expr);
extern bool P_IsDoubleExpr (Expr expr);
extern bool P_IsRealExpr (Expr expr);
extern double P_FloatExprValue (Expr expr);
extern bool P_IsConstOne (Expr expr);
extern bool P_IsConstZero (Expr expr);
extern bool P_IsConstNonZero (Expr expr);
extern bool P_IsConstNegative (Expr expr);
extern bool P_IsSideEffectFree (Expr expr);
extern bool P_IsBooleanExpr (Expr expr);
extern bool P_IsConstNumber (Expr expr);

extern bool P_IsPcodeTemp (Expr expr);

/*! \brief Determines if an expression is a direct function call.
 *
 * \param e
 *  the Expr to inspect.
 *
 * \return
 *  If \a e is a direct function call, returns TRUE.  Otherwise, returns
 *  FALSE.
 *
 * \note \a e is referenced more than once, so it should be side-effect free.
 *
 * \sa #P_IsIndirectFunctionCall()
 */
#define P_IsDirectFunctionCall(e) \
          ((e)->opcode == OP_call && (e)->operands && \
           (e)->operands->opcode == OP_var)

/*! \brief Determines if an expresion is an indirect function call.
 *
 * \param e
 *  the Expr to inspect.
 *
 * \return
 *  If \a e is an indirect function call, returns TRUE.  Otherwise, returns
 *  FALSE.
 *
 * \note \a e is referenced more than once, so it should be side-effect free.
 *
 * \sa #P_IsDirectFunctionCall()
 */
#define P_IsIndirectFunctionCall(e) \
          ((e)->opcode == OP_call && (e)->operands && \
           (e)->operands->opcode != OP_var)

#endif
