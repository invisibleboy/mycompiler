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
 * \brief Routines to work with the Pcode symbol table.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains definitions for routines to work with the Pcode symbol
 * table through an implicit global symbol table.
 *
 * symtab_i.[ch] should contain only wrapper functions calling functions in
 * other _symtab files.  Any function that really modifies a symbol table
 * MUST take the table as an argument.
 *
 * Nothing in symtab_i.[ch] should do any real work, and no function in
 * the Pcode library should call a function in this file.  This file is 
 * intended for modules only.
 */

#include <config.h>
#include <library/stack.h>
#include "pcode.h"
#include "symtab.h"
#include "cast_symtab.h"
#include "query_symtab.h"
#include "reduce_symtab.h"
#include "struct_symtab.h"

#define CHECK_TABLE() \
          (Global_Table != NULL ? \
             : \
             P_punt ("symtab_i.c Global Table must be set with " \
		     "PSI_SetTable() before\nusing PSI_ functions"))

/*! The global symbol table is static to this file.  This is so that it
 * cannot be accessed by anything outside this file.  I want to avoid having
 * functions that modify an global symbol table.  All functions that
 * modify the symbol table should take it as an argument and have an
 * implicit wrapper in this file. */
static SymbolTable Global_Table = NULL;

/*! A stack of previous tables.  This is maintained by
 * PSI_PushTable()/PSI_PopTable().  The current table is held in
 * Global_Table, not on the stack. */
static Stack *Global_Table_Stack = NULL;

/*! \brief Sets the global symbol table.
 *
 * \param table
 *  the symbol table.
 *
 * Sets \a table as the implicit global symbol table.
 *
 * \sa PSI_PushTable()
 */
void
PSI_SetTable (SymbolTable table)
{
  Global_Table = table;

  return;
}

/*! \brief Gets the global symbol table.
 *
 * \return
 *  The implicit global symbol table.
 *
 * Gets the implicit global symbol table.
 *
 * This function really shouldn't be necessary.  There should be a PSI_
 * equivalent for every symbol table function a module will need.
 *
 * \note This function must not be used inside the Pcode library.  Library
 *       functions that need the symbol table *must* take the table as
 *       an argument.
 */
SymbolTable
PSI_GetTable (void)
{
  CHECK_TABLE ();
  return Global_Table;
}

/*! \brief Sets a new implicit symbol table, preserving the previous.
 *
 * \param table
 *  the symbol table.
 *
 * Pushes the current implicit global symbol table onto the stack and
 * sets \a table as the current global table.
 *
 * \sa PSI_PopTable(), PSI_SetTable()
 */
void
PSI_PushTable (SymbolTable table)
{
  if (Global_Table != NULL)
    {
      if (Global_Table_Stack == NULL)
	Global_Table_Stack = New_Stack ();
      
      Push_Top (Global_Table_Stack, Global_Table);
    }
      
  Global_Table = table;

  return;
}

/*! \brief Restores the previous implicit symbol table.
 *
 * \return
 *  The (old) current implicit symbol table.
 * 
 * Returns a pointer to the current implicit symbol table.  Restores
 * the previous implicit symbol table from the stack.
 *
 * \sa PSI_PushTable()
 */
SymbolTable
PSI_PopTable ()
{
  SymbolTable result = NULL;
  void *tmp;

  if (Global_Table_Stack != NULL)
    {
      tmp = Pop (Global_Table_Stack);

      if (tmp == (void *)-1)
	{
	  Global_Table_Stack = Free_Stack (Global_Table_Stack);
	}
      else
	{
	  result = Global_Table;
	  Global_Table = (SymbolTable)tmp;
	}
    }

  return (result);
}

/*! \brief Gets the key of the modifiable file.
 *
 * \return
 *  The key of the file for which the module has write permission.
 */
int
PSI_GetModifiableFile ()
{
  CHECK_TABLE ();
  return (P_GetSymbolTableModifiableFile (Global_Table));
}

/*! \brief Returns the number of files in the global symbol table.
 *
 * \return
 *  The number of files in the global symbol table.
 */
int
PSI_GetNumFiles ()
{
  CHECK_TABLE ();
  return (P_GetSymbolTableNumFiles (Global_Table));
}

/*! \brief Sets option flags for the global symbol table.
 *
 * \param f
 *  the flags to set
 *
 * \return
 *  The new value of the option flags.
 */
_STFlags
PSI_SetFlags (_STFlags f)
{
  CHECK_TABLE ();
  return (P_SetSymbolTableFlags (Global_Table, f));
}

/*! \brief Gets the options flags for the global symbol table.
 *
 * \return
 *  The value of the option flags.
 */
_STFlags
PSI_GetFlags ()
{
  CHECK_TABLE ();
  return (P_GetSymbolTableFlags (Global_Table));
}

/*! \brief Tests an option flag for the global symbol table.
 *
 * \param f
 *  The flag combination to test against the symbol table.
 *
 * \return
 *  non-zero if the flag combination is set on the symbol table.  Otherwise,
 *  0.
 */
int
PSI_TstFlags (_STFlags f)
{
  CHECK_TABLE ();
  return (P_TstSymbolTableFlags (Global_Table, f));
}

/*! \brief Clears an option flag for the global symbol table.
 *
 * \param f
 *  the flag to clear.
 *
 * \return
 *  The new value of the option flags.
 */
_STFlags
PSI_ClrFlags (_STFlags f)
{
  CHECK_TABLE ();
  return (P_ClrSymbolTableFlags (Global_Table, f));
}

/* Wrappers for cast_symtab.c. */
/*! See PST_UpgradeArithmeticType(). */
Expr
PSI_UpgradeArithmeticType (Expr expr, Expr parentexpr)
{
  CHECK_TABLE ();
  return (PST_UpgradeArithmeticType (Global_Table, expr, parentexpr));
}

/*! See PST_UpgradeType(). */
Expr
PSI_UpgradeType (Expr expr, Type type, Expr parentexpr)
{
  CHECK_TABLE ();
  return (PST_UpgradeType (Global_Table, expr, type, parentexpr));
}

/*! See PST_CastExpr(). */
void
PSI_CastExpr (Expr expr)
{
  CHECK_TABLE ();
  PST_CastExpr (Global_Table, expr);
  return;
}

/* Wrappers for query_symtab.c. */
/*! See PST_ExprType(). */
Key
PSI_ExprType (Expr expr)
{
  CHECK_TABLE ();
  return (PST_ExprType (Global_Table, expr));
}

/*! See PST_DominantType(). */
Type
PSI_DominantType (Type a, Type b)
{
  CHECK_TABLE ();
  return (PST_DominantType (Global_Table, a, b));
}

/*! See PST_MatchFuncDcl(). */
bool
PSI_MatchFuncDcl (FuncDcl a, FuncDcl b)
{
  CHECK_TABLE ();
  return (PST_MatchFuncDcl (Global_Table, a, Global_Table, b));
}

/*! See #PST_MatchFuncDclK(). */
bool
PSI_MatchFuncDclK (Key a, Key b)
{
  CHECK_TABLE ();
  return (PST_MatchFuncDclK (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchTypeDcl(). */
bool
PSI_MatchTypeDcl (TypeDcl a, TypeDcl b)
{
  CHECK_TABLE ();
  return (PST_MatchTypeDcl (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchType(). */
bool
PSI_MatchType (Type a, Type b)
{
  CHECK_TABLE ();
  return (PST_MatchType (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchTypeBasicType(). */
bool
PSI_MatchTypeBasicType (Type a, Type b)
{
  CHECK_TABLE ();
  return (PST_MatchTypeBasicType (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchStructDcl(). */
bool
PSI_MatchStructDcl (StructDcl a, StructDcl b)
{
  CHECK_TABLE ();
  return (PST_MatchStructDcl (Global_Table, a, Global_Table, b));
}

/*! See #PST_MatchStructDclK(). */
bool
PSI_MatchStructDclK (Key a, Key b)
{
  CHECK_TABLE ();
  return (PST_MatchStructDclK (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchUnionDcl(). */
bool
PSI_MatchUnionDcl (UnionDcl a, UnionDcl b)
{
  CHECK_TABLE ();
  return (PST_MatchUnionDcl (Global_Table, a, Global_Table, b));
}

/*! See #PST_MatchUnionDclK(). */
bool
PSI_MatchUnionDclK (Key a, Key b)
{
  CHECK_TABLE ();
  return (PST_MatchUnionDclK (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchField(). */
bool
PSI_MatchField (Field a, Field b)
{
  CHECK_TABLE ();
  return (PST_MatchField (Global_Table, a, Global_Table, b));
}

/*! See #PST_MatchFieldK(). */
bool
PSI_MatchFieldK (Key a, Key b)
{
  CHECK_TABLE ();
  return (PST_MatchFieldK (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchVarList(). */
bool
PSI_MatchVarList (VarList a, VarList b)
{
  CHECK_TABLE ();
  return (PST_MatchVarList (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchVarDcl(). */
bool
PSI_MatchVarDcl (VarDcl a, VarDcl b)
{
  CHECK_TABLE ();
  return (PST_MatchVarDcl (Global_Table, a, Global_Table, b));
}

/*! See #PST_MatchVarDclK(). */
bool
PSI_MatchVarDclK (Key a, Key b)
{
  CHECK_TABLE ();
  return (PST_MatchVarDclK (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchExpr(). */
bool
PSI_MatchExpr (Expr a, Expr b)
{
  CHECK_TABLE ();
  return (PST_MatchExpr (Global_Table, a, Global_Table, b));
}

/*! See PST_MatchParam(). */
bool
PSI_MatchParam (Param a, Param b)
{
  CHECK_TABLE ();
  return (PST_MatchParam (Global_Table, a, Global_Table, b));
}

/*! See PST_IsVoidType(). */
bool
PSI_IsVoidType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsVoidType (Global_Table, type));
}

/*! See #PST_IsVoidTypeExpr(). */
bool
PSI_IsVoidTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsVoidTypeExpr (Global_Table, e));
}
  
/*! See PST_IsIntegralType(). */
bool
PSI_IsIntegralType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsIntegralType (Global_Table, type));
}

/*! See #PST_IsIntegralTypeExpr(). */
bool
PSI_IsIntegralTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsIntegralTypeExpr (Global_Table, e));
}
  
/*! See PST_IsRealType(). */
bool
PSI_IsRealType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsRealType (Global_Table, type));
}

/*! See #PST_IsRealTypeExpr(). */
bool
PSI_IsRealTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsRealTypeExpr (Global_Table, e));
}
  
/*! See PST_IsFloatType(). */
bool
PSI_IsFloatType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsFloatType (Global_Table, type));
}

/*! See #PST_IsFloatTypeExpr(). */
bool
PSI_IsFloatTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsFloatTypeExpr (Global_Table, e));
}
  
/*! See PST_IsDoubleType(). */
bool
PSI_IsDoubleType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsDoubleType (Global_Table, type));
}

/*! See #PST_IsDoubleTypeExpr(). */
bool
PSI_IsDoubleTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsDoubleTypeExpr (Global_Table, e));
}
  
/*! See PST_IsArithmeticType(). */
bool
PSI_IsArithmeticType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsArithmeticType (Global_Table, type));
}

/*! See #PST_IsArithmeticTypeExpr(). */
bool
PSI_IsArithmeticTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsArithmeticTypeExpr (Global_Table, e));
}

/*! See PST_IsPointerType(). */
bool
PSI_IsPointerType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsPointerType (Global_Table, type));
}

/*! See #PST_IsPointerTypeExpr(). */
bool
PSI_IsPointerTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsPointerTypeExpr (Global_Table, e));
}
  
/*! See PST_IsFundamentalType(). */
bool
PSI_IsFundamentalType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsFundamentalType (Global_Table, type));
}

/*! See #PST_IsFundamentalTypeExpr(). */
bool
PSI_IsFundamentalTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsFundamentalTypeExpr (Global_Table, e));
}
  
/*! See PST_IsStructureType(). */
bool
PSI_IsStructureType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsStructureType (Global_Table, type));
}

/*! See #PST_IsStructureTypeExpr(). */
bool
PSI_IsStructureTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsStructureTypeExpr (Global_Table, e));
}
  
/*! See PST_IsArrayType(). */
bool
PSI_IsArrayType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsArrayType (Global_Table, type));
}

/*! See #PST_IsArrayTypeExpr(). */
bool
PSI_IsArrayTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsArrayTypeExpr (Global_Table, e));
}
  
/*! See PST_IsFunctionType(). */
bool
PSI_IsFunctionType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsFunctionType (Global_Table, type));
}

/*! See #PST_IsFunctionTypeExpr(). */
bool
PSI_IsFunctionTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsFunctionTypeExpr (Global_Table, e));
}
  
/*! See PST_IsSignedType(). */
bool
PSI_IsSignedType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsSignedType (Global_Table, type));
}

/*! See #PST_IsSignedTypeExpr(). */
bool
PSI_IsSignedTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsSignedTypeExpr (Global_Table, e));
}
  
/*! See PST_IsUnsignedType(). */
bool
PSI_IsUnsignedType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsUnsignedType (Global_Table, type));
}

/*! See #PST_IsUnsignedTypeExpr(). */
bool
PSI_IsUnsignedTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsUnsignedTypeExpr (Global_Table, e));
}
  
/*! See PST_IsEnumType(). */
bool
PSI_IsEnumType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsEnumType (Global_Table, type));
}

/*! See #PST_IsEnumTypeExpr(). */
bool
PSI_IsEnumTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsEnumTypeExpr (Global_Table, e));
}

/*! See PST_IsVarargType(). */
bool
PSI_IsVarargType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsVarargType (Global_Table, type));
}

/*! See #PST_IsVarargTypeExpr(). */
bool
PSI_IsVarargTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsVarargTypeExpr (Global_Table, e));
}

#if 0
/*! See PST_IsBitFieldType(). */
bool
PSI_IsBitFieldType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsBitFieldType (Global_Table, type));
}

/*! See #PST_IsBitFieldTypeExpr(). */
bool
PSI_IsBitFieldTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsBitFieldTypeExpr (Global_Table, e));
}
#endif

/*! See PST_IsBaseType(). */
bool
PSI_IsBaseType (Type type)
{
  CHECK_TABLE ();
  return (PST_IsBaseType (Global_Table, type));
}

/*! See #PST_IsBaseTypeExpr(). */
bool
PSI_IsBaseTypeExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_IsBaseTypeExpr (Global_Table, e));
}

/*! See PST_EqualStrengthType(). */
bool
PSI_EqualStrengthType (Type a, Type b)
{
  CHECK_TABLE ();
  return (PST_EqualStrengthType (Global_Table, a, b));
}

/* Wrappers for reduce_symtab.h. */
/*! See PST_ReduceExpr(). */
Expr
PSI_ReduceExpr (Expr expr)
{
  CHECK_TABLE ();
  return (PST_ReduceExpr (Global_Table, expr));
}

/* Wrappers for struct_symtab.h. */
/*! See PST_FindBasicType(). */
Type
PSI_FindBasicType (_BasicType basic_type)
{
  CHECK_TABLE ();
  return (PST_FindBasicType (Global_Table, basic_type));
}

/*! See PST_FindBasicTypeWithQual(). */
Type
PSI_FindBasicTypeWithQual (_BasicType basic_type, _TypeQual type_qual)
{
  CHECK_TABLE ();
  return (PST_FindBasicTypeWithQual (Global_Table, basic_type, type_qual));
}

/*! See PST_ScopeFindTypeDcl(). */
Type
PSI_ScopeFindTypeDcl (Key scope_key, TypeDcl type_dcl)
{
  CHECK_TABLE ();
  return (PST_ScopeFindTypeDcl (Global_Table, scope_key, type_dcl));
}

/*! See PST_FindPointerToType(). */
Type
PSI_FindPointerToType (Type type)
{
  CHECK_TABLE ();
  return (PST_FindPointerToType (Global_Table, type));
}

/*! See PST_GetTypeBasicType(). */
_BasicType
PSI_GetTypeBasicType (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeBasicType (Global_Table, type));
}

/*! See PST_SetTypeQualifier(). */
_TypeQual
PSI_SetTypeQualifier (Type *type, _TypeQual type_qual)
{
  CHECK_TABLE ();
  return (PST_SetTypeQualifier (Global_Table, type, type_qual));
}

/*! See PST_GetTypeQualifier(). */
_TypeQual
PSI_GetTypeQualifier (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeQualifier (Global_Table, type));
}

/*! See PST_ClrTypeQualifier(). */
_TypeQual
PSI_ClrTypeQualifier (Type *type, _TypeQual type_qual)
{
  CHECK_TABLE ();
  return (PST_ClrTypeQualifier (Global_Table, type, type_qual));
}

/*! See PST_FindTypeSetQualifier(). */
Type
PSI_FindTypeSetQualifier (Type type, _TypeQual type_qual)
{
  CHECK_TABLE ();
  return (PST_FindTypeSetQualifier (Global_Table, type, type_qual));
}

/*! See PST_FindTypeClrQualifier(). */
Type
PSI_FindTypeClrQualifier (Type type, _TypeQual type_qual)
{
  CHECK_TABLE ();
  return (PST_FindTypeClrQualifier (Global_Table, type, type_qual));
}

/*! See #PST_GetTypeTypeDcl(). */
TypeDcl
PSI_GetTypeTypeDcl (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeTypeDcl (Global_Table, type));
}

/*! See PST_GetTypeStructDcl(). */
StructDcl
PSI_GetTypeStructDcl (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeStructDcl (Global_Table, type));
}

/*! See PST_GetTypeUnionDcl(). */
UnionDcl
PSI_GetTypeUnionDcl (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeUnionDcl (Global_Table, type));
}

/*! See PST_GetTypeType(). */
Type
PSI_GetTypeType (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeType (Global_Table, type));
}

/*! See PST_GetTypeName(). */
char *
PSI_GetTypeName (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeName (Global_Table, type));
}

/*! See PST_GetTypeArraySize(). */
Expr
PSI_GetTypeArraySize (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeArraySize (Global_Table, type));
}

/*! See PST_GetTypeParam(). */
Param
PSI_GetTypeParam (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeParam (Global_Table, type));
}

/*! See PST_GetTypeSize(). */
int
PSI_GetTypeSize (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeSize (Global_Table, type));
}

/*! See PST_SetTypeAlignment(). */
int
PSI_SetTypeAlignment (Type *type, int alignment)
{
  CHECK_TABLE ();
  return (PST_SetTypeAlignment (Global_Table, type, alignment));
}

/*! See PST_GetTypeAlignment(). */
int
PSI_GetTypeAlignment (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeAlignment (Global_Table, type));
}

/*! See PST_FindTypeSetAlignment(). */
Type
PSI_FindTypeSetAlignment (Type type, int alignment)
{
  CHECK_TABLE ();
  return (PST_FindTypeSetAlignment (Global_Table, type, alignment));
}

/*! See PST_GetTypeLineno(). */
int
PSI_GetTypeLineno (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeLineno (Global_Table, type));
}

/*! See PST_GetTypeColno(). */
int
PSI_GetTypeColno (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeColno (Global_Table, type));
}

/*! See PST_GetTypeFilename(). */
char *
PSI_GetTypeFilename (Type type)
{
  CHECK_TABLE ();
  return (PST_GetTypeFilename (Global_Table, type));
}

/*! See PST_IncTypeRefCount(). */
int
PSI_IncTypeRefCount (Type type)
{
  CHECK_TABLE ();
  return (PST_IncTypeRefCount (Global_Table, type));
}

/*! See PST_DecTypeRefCount(). */
int
PSI_DecTypeRefCount (Type type)
{
  CHECK_TABLE ();
  return (PST_DecTypeRefCount (Global_Table, type));
}

/*! See PST_SetTypeSigned(). */
Type
PSI_SetTypeSigned (Type *type)
{
  CHECK_TABLE ();
  return (PST_SetTypeSigned (Global_Table, type));
}

/*! See PST_FindTypeSetSigned(). */
Type
PSI_FindTypeSetSigned (Type type)
{
  CHECK_TABLE ();
  return (PST_FindTypeSetSigned (Global_Table, type));
}

/*! See PST_SetTypeUnsigned(). */
Type
PSI_SetTypeUnsigned (Type *type)
{
  CHECK_TABLE ();
  return (PST_SetTypeUnsigned (Global_Table, type));
}

/*! See PST_FindTypeSetUnsigned(). */
Type
PSI_FindTypeSetUnsigned (Type type)
{
  CHECK_TABLE ();
  return (PST_FindTypeSetUnsigned (Global_Table, type));
}

/*! See #PST_DereferenceType(). */
Type
PSI_DereferenceType (Type u)
{
  CHECK_TABLE ();
  return (PST_DereferenceType (Global_Table, u));
}

/*! See #PST_DereferencePointers(). */
Type
PSI_DereferencePointers (Type u)
{
  CHECK_TABLE ();
  return (PST_DereferencePointers (Global_Table, u));
}

/*! See PST_GetBaseType(). */
Type
PSI_GetBaseType (Type type)
{
  CHECK_TABLE ();
  return (PST_GetBaseType (Global_Table, type));
}

/*! See PST_ReduceTypedefs(). */
Type
PSI_ReduceTypedefs (Type type)
{
  CHECK_TABLE ();
  return (PST_ReduceTypedefs (Global_Table, type));
}

/*! see PST_ReduceExplicitTypedefs(). */
Type
PSI_ReduceExplicitTypedefs (Key type)
{
  CHECK_TABLE ();
  return (PST_ReduceExplicitTypedefs (Global_Table, type));
}

/*! see PST_ReduceImplicitTypedefs(). */
Type
PSI_ReduceImplicitTypedefs (Key type)
{
  CHECK_TABLE ();
  return (PST_ReduceImplicitTypedefs (Global_Table, type));
}

/*! See PST_SetFuncDclType(). */
Type
PSI_SetFuncDclType (FuncDcl func_dcl, Type type)
{
  CHECK_TABLE ();
  return (PST_SetFuncDclType (Global_Table, func_dcl, type));
}

/*! See PST_SetFuncDclName(). */
char *
PSI_SetFuncDclName (FuncDcl func_dcl, char *name)
{
  CHECK_TABLE ();
  return (PST_SetFuncDclName (Global_Table, func_dcl, name));
}

/*! See PST_SetParamKey(). */
Type
PSI_SetParamKey (Param param, Type type)
{
  CHECK_TABLE ();
  return (PST_SetParamKey (Global_Table, param, type));
}

/*! See PST_SetTypeDclType(). */
Type
PSI_SetTypeDclType (TypeDcl type_dcl, Type type)
{
  CHECK_TABLE ();
  return (PST_SetTypeDclType (Global_Table, type_dcl, type));
}
  
/*! See PST_SetTypeDclName(). */
char *
PSI_SetTypeDclName (TypeDcl type_dcl, char *name)
{
  CHECK_TABLE ();
  return (PST_SetTypeDclName (Global_Table, type_dcl, name));
}
  
/*! See PST_SetVarDclType(). */
Type
PSI_SetVarDclType (VarDcl var_dcl, Type type)
{
  CHECK_TABLE ();
  return (PST_SetVarDclType (Global_Table, var_dcl, type));
}

/*! See PST_SetVarDclName(). */
char *
PSI_SetVarDclName (VarDcl var_dcl, char *name)
{
  CHECK_TABLE ();
  return (PST_SetVarDclName (Global_Table, var_dcl, name));
}

/*! See PST_SetStructDclName(). */
char *
PSI_SetStructDclName (StructDcl struct_dcl, char *name)
{
  CHECK_TABLE ();
  return (PST_SetStructDclName (Global_Table, struct_dcl, name));
}

/*! See PST_SetUnionDclName(). */
char *
PSI_SetUnionDclName (UnionDcl union_dcl, char *name)
{
  CHECK_TABLE ();
  return (PST_SetUnionDclName (Global_Table, union_dcl, name));
}

/*! See PST_SetEnumDclName(). */
char *
PSI_SetEnumDclName (EnumDcl enum_dcl, char *name)
{
  CHECK_TABLE ();
  return (PST_SetEnumDclName (Global_Table, enum_dcl, name));
}

/*! See PST_SetFieldType(). */
Type
PSI_SetFieldType (Field field, Type type)
{
  CHECK_TABLE ();
  return (PST_SetFieldType (Global_Table, field, type));
}
  
/*! See PST_SetFieldName(). */
char *
PSI_SetFieldName (Field field, char *name)
{
  CHECK_TABLE ();
  return (PST_SetFieldName (Global_Table, field, name));
}

/*! See PST_SetEnumFieldName(). */
char *
PSI_SetEnumFieldName (EnumField enum_field, char *name)
{
  CHECK_TABLE ();
  return (PST_SetEnumFieldName (Global_Table, enum_field, name));
}

/*! See PST_SetExprType(). */
Type
PSI_SetExprType (Expr expr, Type type)
{
  CHECK_TABLE ();
  return (PST_SetExprType (Global_Table, expr, type));
}
  
/*! See PST_SetExprVType(). */
Type
PSI_SetExprVType (Expr expr, Type type)
{
  CHECK_TABLE ();
  return (PST_SetExprVType (Global_Table, expr, type));
}

/*! See PST_RemoveDcl(). */
Dcl
PSI_RemoveDcl (Dcl d)
{
  CHECK_TABLE ();
  return (PST_RemoveDcl (Global_Table, d));
}

/*! See PST_RemoveFuncDcl(). */
FuncDcl
PSI_RemoveFuncDcl (FuncDcl f)
{
  CHECK_TABLE ();
  return (PST_RemoveFuncDcl (Global_Table, f));
}

/*! See PST_RemoveTypeDcl(). */
TypeDcl
PSI_RemoveTypeDcl (TypeDcl t)
{
  CHECK_TABLE ();
  return (PST_RemoveTypeDcl (Global_Table, t));
}

/*! See PST_RemoveTypeList(). */
TypeList
PSI_RemoveTypeList (TypeList t)
{
  CHECK_TABLE ();
  return (PST_RemoveTypeList (Global_Table, t));
}

/*! See PST_RemoveVarList(). */
VarList
PSI_RemoveVarList (VarList v)
{
  CHECK_TABLE ();
  return (PST_RemoveVarList (Global_Table, v));
}

/*! See PST_RemoveVarDcl(). */
VarDcl
PSI_RemoveVarDcl (VarDcl v)
{
  CHECK_TABLE ();
  return (PST_RemoveVarDcl (Global_Table, v));
}

/*! See PST_RemoveInit(). */
Init
PSI_RemoveInit (Init i)
{
  CHECK_TABLE ();
  return (PST_RemoveInit (Global_Table, i));
}

/*! See PST_RemoveStructDcl(). */
StructDcl
PSI_RemoveStructDcl (StructDcl s)
{
  CHECK_TABLE ();
  return (PST_RemoveStructDcl (Global_Table, s));
}

/*! See PST_RemoveUnionDcl(). */
UnionDcl
PSI_RemoveUnionDcl (UnionDcl u)
{
  CHECK_TABLE ();
  return (PST_RemoveUnionDcl (Global_Table, u));
}

/*! See PST_RemoveField(). */
Field
PSI_RemoveField (Field f)
{
  CHECK_TABLE ();
  return (PST_RemoveField (Global_Table, f));
}

/*! See PST_RemoveStmtNode(). */
Stmt
PSI_RemoveStmtNode (Stmt s)
{
  CHECK_TABLE ();
  return (PST_RemoveStmtNode (Global_Table, s));
}

/*! See PST_RemoveStmt(). */
Stmt
PSI_RemoveStmt (Stmt s)
{
  CHECK_TABLE ();
  return (PST_RemoveStmt (Global_Table, s));
}

/*! See PST_RemoveLabel(). */
Label
PSI_RemoveLabel (Label l)
{
  CHECK_TABLE ();
  return (PST_RemoveLabel (Global_Table, l));
}

/*! See PST_RemoveCompound(). */
Compound
PSI_RemoveCompound (Compound c)
{
  CHECK_TABLE ();
  return (PST_RemoveCompound (Global_Table, c));
}

/*! See PST_RemoveIfStmt(). */
IfStmt
PSI_RemoveIfStmt (IfStmt i)
{
  CHECK_TABLE ();
  return (PST_RemoveIfStmt (Global_Table, i));
}

/*! See PST_RemoveSwitchStmt(). */
SwitchStmt
PSI_RemoveSwitchStmt (SwitchStmt s)
{
  CHECK_TABLE ();
  return (PST_RemoveSwitchStmt (Global_Table, s));
}

/*! See PST_RemovePstmt(). */
Pstmt
PSI_RemovePstmt (Pstmt p)
{
  CHECK_TABLE ();
  return (PST_RemovePstmt (Global_Table, p));
}

/*! See PST_RemoveMutex(). */
Mutex
PSI_RemoveMutex (Mutex m)
{
  CHECK_TABLE ();
  return (PST_RemoveMutex (Global_Table, m));
}

/*! See PST_RemoveCobegin(). */
Cobegin
PSI_RemoveCobegin (Cobegin c)
{
  CHECK_TABLE ();
  return (PST_RemoveCobegin (Global_Table, c));
}

/*! See PST_RemoveBodyStmt(). */
BodyStmt
PSI_RemoveBodyStmt (BodyStmt b)
{
  CHECK_TABLE ();
  return (PST_RemoveBodyStmt (Global_Table, b));
}

/*! See PST_RemoveEpilogueStmt(). */
EpilogueStmt
PSI_RemoveEpilogueStmt (EpilogueStmt e)
{
  CHECK_TABLE ();
  return (PST_RemoveEpilogueStmt (Global_Table, e));
}

/*! See PST_RemoveParLoop(). */
ParLoop
PSI_RemoveParLoop (ParLoop p)
{
  CHECK_TABLE ();
  return (PST_RemoveParLoop (Global_Table, p));
}

/*! See PST_RemoveSerLoop(). */
SerLoop
PSI_RemoveSerLoop (SerLoop s)
{
  CHECK_TABLE ();
  return (PST_RemoveSerLoop (Global_Table, s));
}

/*! See PST_RemoveAsmStmt(). */
AsmStmt
PSI_RemoveAsmStmt (AsmStmt a)
{
  CHECK_TABLE ();
  return (PST_RemoveAsmStmt (Global_Table, a));
}

/*! See PST_RemoveExprNode(). */
Expr
PSI_RemoveExprNode (Expr e)
{
  CHECK_TABLE ();
  return (PST_RemoveExprNode (Global_Table, e));
}

/*! See PST_RemoveExpr(). */
Expr
PSI_RemoveExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_RemoveExpr (Global_Table, e));
}

/*! See PST_RemoveAsmDcl(). */
AsmDcl
PSI_RemoveAsmDcl (AsmDcl a)
{
  CHECK_TABLE ();
  return (PST_RemoveAsmDcl (Global_Table, a));
}

/*! See PST_RemoveSymTabEntry(). */
SymTabEntry
PSI_RemoveSymTabEntry (SymTabEntry s)
{
  CHECK_TABLE ();
  return (PST_RemoveSymTabEntry (Global_Table, s));
}

/*! See PST_RemoveIPSymTabEnt(). */
IPSymTabEnt
PSI_RemoveIPSymTabEnt (IPSymTabEnt i)
{
  CHECK_TABLE ();
  return (PST_RemoveIPSymTabEnt (Global_Table, i));
}

/*! See PST_GetScopeParentFunc(). */
FuncDcl
PSI_GetScopeParentFunc (Key scope_key)
{
  CHECK_TABLE ();
  return (PST_GetScopeParentFunc (Global_Table, scope_key));
}

/*! See PST_GetStmtParentFunc(). */
FuncDcl
PSI_GetStmtParentFunc (Stmt s)
{
  CHECK_TABLE ();
  return (PST_GetStmtParentFunc (Global_Table, s));
}

/*! See PST_GetExprParentFunc(). */
FuncDcl
PSI_GetExprParentFunc (Expr e)
{
  CHECK_TABLE ();
  return (PST_GetExprParentFunc (Global_Table, e));
}

/*! See PST_CopyFuncDcl(). */
FuncDcl
PSI_CopyFuncDcl (FuncDcl f)
{
  CHECK_TABLE ();
  return (PST_CopyFuncDcl (Global_Table, f));
}

/*! See PST_CopyFuncDclToTableScope(). */
FuncDcl
PSI_CopyFuncDclToScope (Key dst_scope, FuncDcl f)
{
  CHECK_TABLE ();
  return (PST_CopyFuncDclToTableScope (Global_Table, dst_scope, Global_Table,
				       f, FALSE));
}

/*! See PST_CopyTypeDcl(). */
TypeDcl
PSI_CopyTypeDcl (TypeDcl t)
{
  CHECK_TABLE ();
  return (PST_CopyTypeDcl (Global_Table, t));
}

/*! See PST_CopyTypeDclToTableScope(). */
TypeDcl
PSI_CopyTypeDclToScope (Key dst_scope, TypeDcl t)
{
  CHECK_TABLE ();
  return (PST_CopyTypeDclToTableScope (Global_Table, dst_scope, Global_Table,
				       t, FALSE));
}

/*! See PST_CopyVarDcl(). */
VarDcl
PSI_CopyVarDcl (VarDcl v)
{
  CHECK_TABLE ();
  return (PST_CopyVarDcl (Global_Table, v));
}

/*! See PST_CopyVarDclToTableScope(). */
VarDcl
PSI_CopyVarDclToScope (Key dst_scope, VarDcl v)
{
  CHECK_TABLE ();
  return (PST_CopyVarDclToTableScope (Global_Table, dst_scope, Global_Table,
				      v, FALSE));
}

/*! See PST_CopyStructDcl(). */
StructDcl
PSI_CopyStructDcl (StructDcl s)
{
  CHECK_TABLE ();
  return (PST_CopyStructDcl (Global_Table, s));
}

/*! See PST_CopyStructDclToTableScope(). */
StructDcl
PSI_CopyStructDclToScope (Key dst_scope, StructDcl s)
{
  CHECK_TABLE ();
  return (PST_CopyStructDclToTableScope (Global_Table, dst_scope, Global_Table,
					 s, FALSE));
}

/*! See PST_CopyUnionDcl(). */
UnionDcl
PSI_CopyUnionDcl (UnionDcl u)
{
  CHECK_TABLE ();
  return (PST_CopyUnionDcl (Global_Table, u));
}

/*! See PST_CopyUnionDclToTableScope(). */
UnionDcl
PSI_CopyUnionDclToScope (Key dst_scope, UnionDcl u)
{
  CHECK_TABLE ();
  return (PST_CopyUnionDclToTableScope (Global_Table, dst_scope, Global_Table,
					u, FALSE));
}

/*! See PST_CopyField(). */
Field
PSI_CopyField (Field f)
{
  CHECK_TABLE ();
  return (PST_CopyField (Global_Table, f));
}

/*! See PST_CopyFieldToTableScope(). */
Field
PSI_CopyFieldToScope (Key dst_scope, Field f)
{
  CHECK_TABLE ();
  return (PST_CopyFieldToTableScope (Global_Table, dst_scope, Global_Table,
				     f, FALSE));
}

/*! See PST_CopyEnumDcl(). */
EnumDcl
PSI_CopyEnumDcl (EnumDcl e)
{
  CHECK_TABLE ();
  return (PST_CopyEnumDcl (Global_Table, e));
}

/*! See PST_CopyEnumDclToTableScope(). */
EnumDcl
PSI_CopyEnumDclToScope (Key dst_scope, EnumDcl e)
{
  CHECK_TABLE ();
  return (PST_CopyEnumDclToTableScope (Global_Table, dst_scope, Global_Table,
				       e, FALSE));
}

/*! See PST_CopyEnumField(). */
EnumField
PSI_CopyEnumField (EnumField f)
{
  CHECK_TABLE ();
  return (PST_CopyEnumField (Global_Table, f));
}

/*! See PST_CopyEnumFieldToTableScope(). */
EnumField
PSI_CopyEnumFieldToScope (Key dst_scope, EnumField f)
{
  CHECK_TABLE ();
  return (PST_CopyEnumFieldToTableScope (Global_Table, dst_scope, Global_Table,
					 f, FALSE));
}

/*! See PST_CopyStmtNode(). */
Stmt
PSI_CopyStmtNode (Stmt s)
{
  CHECK_TABLE ();
  return (PST_CopyStmtNode (Global_Table, s));
}

/*! See PST_CopyStmtNodeToScope(). */
Stmt
PSI_CopyStmtNodeToScope (Key dst_scope, Stmt s)
{
  CHECK_TABLE ();
  return (PST_CopyStmtNodeToScope (Global_Table, dst_scope, s));
}

/*! See PST_CopyStmt(). */
Stmt
PSI_CopyStmt (Stmt s)
{
  CHECK_TABLE ();
  return (PST_CopyStmt (Global_Table, s));
}

/*! See PST_CopyStmtToScope(). */
Stmt
PSI_CopyStmtToScope (Key dst_scope, Stmt s)
{
  CHECK_TABLE ();
  return (PST_CopyStmtToScope (Global_Table, dst_scope, s));
}

/*! See PST_CopyLabel(). */
Label
PSI_CopyLabel (Label l)
{
  CHECK_TABLE ();
  return (PST_CopyLabel (Global_Table, l));
}

/*! See PST_CopyLabelToTableScope(). */
Label
PSI_CopyLabelToScope (Key dst_scope, Label l)
{
  CHECK_TABLE ();
  return (PST_CopyLabelToTableScope (Global_Table, dst_scope, Global_Table, l,
				     FALSE));
}

/*! See PST_CopyExprNode(). */
Expr
PSI_CopyExprNode (Expr e)
{
  CHECK_TABLE ();
  return (PST_CopyExprNode (Global_Table, e));
}

/*! See PST_CopyExprNodeToScope(). */
Expr
PSI_CopyExprNodeToScope (Key dst_scope, Expr e)
{
  CHECK_TABLE ();
  return (PST_CopyExprNodeToScope (Global_Table, dst_scope, e));
}

/*! See PST_CopyExpr(). */
Expr
PSI_CopyExpr (Expr e)
{
  CHECK_TABLE ();
  return (PST_CopyExpr (Global_Table, e));
}

/*! See PST_CopyExprToScope(). */
Expr
PSI_CopyExprToScope (Key dst_scope, Expr e)
{
  CHECK_TABLE ();
  return (PST_CopyExprToScope (Global_Table, dst_scope, e));
}

/*! See PST_CopyExprList(). */
Expr
PSI_CopyExprList (Expr e)
{
  CHECK_TABLE ();
  return (PST_CopyExprList (Global_Table, e));
}

/*! See PST_CopyExprListToScope(). */
Expr
PSI_CopyExprListToScope (Key dst_scope, Expr e)
{
  CHECK_TABLE ();
  return (PST_CopyExprListToScope (Global_Table, dst_scope, e));
}

/*! See PST_CopyAsmDcl(). */
AsmDcl
PSI_CopyAsmDcl (AsmDcl a)
{
  CHECK_TABLE ();
  return (PST_CopyAsmDcl (Global_Table, a));
}

/*! See PST_CopyAsmDclToTableScope(). */
AsmDcl
PSI_CopyAsmDclToScope (Key dst_scope, AsmDcl a)
{
  CHECK_TABLE ();
  return (PST_CopyAsmDclToTableScope (Global_Table, dst_scope, Global_Table, a,
				      FALSE));
}

/*! See PST_CopySymTabEntryToTableScope(). */
SymTabEntry
PSI_CopySymTabEntryToScope (Key dst_scope, SymTabEntry e)
{
  CHECK_TABLE ();
  return (PST_CopySymTabEntryToTableScope (Global_Table, dst_scope,
					   Global_Table, e, FALSE));
}

/*! See PST_ScopeFindFuncScope(). */
Key
PSI_ScopeFindFuncScope (Key scope_key)
{
  CHECK_TABLE ();
  return (PST_ScopeFindFuncScope (Global_Table, scope_key));
}

/*! See PST_ScopeUpdateExprIDs(). */
void
PSI_ScopeUpdateExprIDs (Key scope_key, Expr expr)
{
  CHECK_TABLE ();
  PST_ScopeUpdateExprIDs (Global_Table, scope_key, expr);
  return;
}

/*! See #PST_UpdateExprIDs(). */
void
PSI_UpdateExprIDs (Expr e)
{
  CHECK_TABLE ();
  PST_UpdateExprIDs (Global_Table, e);
  return;
}

/*! See PST_ScopeNewExpr(). */
Expr
PSI_ScopeNewExpr (Key scope_key)
{
  CHECK_TABLE ();
  return (PST_ScopeNewExpr (Global_Table, scope_key));
}

/*! See PST_ScopeNewExprWithOpcode(). */
Expr
PSI_ScopeNewExprWithOpcode (Key scope_key, _Opcode opcode)
{
  CHECK_TABLE ();
  return (PST_ScopeNewExprWithOpcode (Global_Table, scope_key, opcode));
}

/*! See PST_ScopeNewStringExpr(). */
Expr
PSI_ScopeNewStringExpr (Key scope_key, char *s)
{
  CHECK_TABLE ();
  return (PST_ScopeNewStringExpr (Global_Table, scope_key, s));
}

/*! See PST_ScopeNewIntExpr(). */
Expr
PSI_ScopeNewIntExpr (Key scope_key, int i)
{
  CHECK_TABLE ();
  return (PST_ScopeNewIntExpr (Global_Table, scope_key, i));
}

/*! See PST_ScopeNewUIntExpr(). */
Expr
PSI_ScopeNewUIntExpr (Key scope_key, int i)
{
  CHECK_TABLE ();
  return (PST_ScopeNewUIntExpr (Global_Table, scope_key, i));
}

/*! See PST_ScopeNewFloatExpr(). */
Expr
PSI_ScopeNewFloatExpr (Key scope_key, double f)
{
  CHECK_TABLE ();
  return (PST_ScopeNewFloatExpr (Global_Table, scope_key, f));
}

/*! See PST_ScopeNewDoubleExpr(). */
Expr
PSI_ScopeNewDoubleExpr (Key scope_key, double d)
{
  CHECK_TABLE ();
  return (PST_ScopeNewDoubleExpr (Global_Table, scope_key, d));
}

/*! See PST_ScopeNewIdentifier(). */
char *
PSI_ScopeNewIdentifier (Key scope_key, char *tag)
{
  CHECK_TABLE ();
  return (PST_ScopeNewIdentifier (Global_Table, scope_key, tag));
}

/*! See PST_NewCompoundStmt(). */
Stmt
PSI_NewCompoundStmt (Key scope_key)
{
  CHECK_TABLE ();
  return (PST_NewCompoundStmt (Global_Table, scope_key));
}

/*! See PST_NewNoopStmtWithLabel(). */
Stmt
PSI_NewNoopStmtWithLabel (Key scope_key, char *label_val, Key *label_key)
{
  CHECK_TABLE ();
  return (PST_NewNoopStmtWithLabel (Global_Table, scope_key, label_val,
				    label_key));
}

/*! See PST_NewLabel(). */
Label
PSI_NewLabel (Key scope_key, char *label_val)
{
  CHECK_TABLE ();
  return (PST_NewLabel (Global_Table, scope_key, label_val));
}

/*! See #PST_NewLabelTemp(). */
Label
PSI_NewLabelTemp (Key s, char *u)
{
  CHECK_TABLE ();
  return (PST_NewLabelTemp (Global_Table, s, u));
}

/*! See PST_NewLocalVar(). */
Key
PSI_NewLocalVar (Key scope_key, Type type, char *name)
{
  CHECK_TABLE ();
  return (PST_NewLocalVar (Global_Table, scope_key, type, name));
}

/*! See PST_NewLocalVarTemp(). */
Key
PSI_NewLocalVarTemp (Key scope_key, Type type, char *tag)
{
  CHECK_TABLE ();
  return (PST_NewLocalVarTemp (Global_Table, scope_key, type, tag));
}

/*! See PST_NewLocalVarExpr(). */
Expr
PSI_NewLocalVarExpr (Key scope_key, Type type, char *name)
{
  CHECK_TABLE ();
  return (PST_NewLocalVarExpr (Global_Table, scope_key, type, name));
}

/*! See PST_NewLocalVarExprTemp(). */
Expr
PSI_NewLocalVarExprTemp (Key scope_key, Type type, char *tag)
{
  CHECK_TABLE ();
  return (PST_NewLocalVarExprTemp (Global_Table, scope_key, type, tag));
}

/*! See #PST_GetFuncDclScope(). */
Key
PSI_GetFuncDclScope (FuncDcl f)
{
  CHECK_TABLE ();
  return (PST_GetFuncDclScope (Global_Table, f));
}

/*! See #PST_GetTypeDclScope(). */
Key
PSI_GetTypeDclScope (TypeDcl u)
{
  CHECK_TABLE ();
  return (PST_GetTypeDclScope (Global_Table, u));
}

/*! See #PST_GetVarDclScope(). */
Key
PSI_GetVarDclScope (VarDcl v)
{
  CHECK_TABLE ();
  return (PST_GetVarDclScope (Global_Table, v));
}

/*! See #PST_GetStructDclScope(). */
Key
PSI_GetStructDclScope (StructDcl s)
{
  CHECK_TABLE ();
  return (PST_GetStructDclScope (Global_Table, s));
}

/*! See #PST_GetUnionDclScope(). */
Key
PSI_GetUnionDclScope (UnionDcl u)
{
  CHECK_TABLE ();
  return (PST_GetUnionDclScope (Global_Table, u));
}

/*! See PST_GetStmtScope(). */
Key
PSI_GetStmtScope (Stmt stmt)
{
  CHECK_TABLE ();
  return (PST_GetStmtScope (Global_Table, stmt));
}

/*! See PST_GetExprScope(). */
Key
PSI_GetExprScope (Expr expr)
{
  CHECK_TABLE ();
  return (PST_GetExprScope (Global_Table, expr));
}

/*! See #PST_GetExprTypeDcl(). */
TypeDcl
PSI_GetExprTypeDcl (Expr e)
{
  CHECK_TABLE ();
  return (PST_GetExprTypeDcl (Global_Table, e));
}

/*! See PST_StmtEncloseInCompound(). */
bool
PSI_StmtEncloseInCompound (Stmt stmt)
{
  CHECK_TABLE ();
  return (PST_StmtEncloseInCompound (Global_Table, stmt));
}

/*! See PST_StmtInsertExprAfter(). */
void
PSI_StmtInsertExprAfter (Stmt s, Expr e)
{
  CHECK_TABLE ();
  PST_StmtInsertExprAfter (Global_Table, s, e);
  return;
}

/*! See PST_StmtInsertExprBefore(). */
void
PSI_StmtInsertExprBefore (Stmt s, Expr e)
{
  CHECK_TABLE ();
  PST_StmtInsertExprBefore (Global_Table, s, e);
  return;
}

/*! See PST_StmtInsertExprBeforeLabel(). */
void
PSI_StmtInsertExprBeforeLabel (Stmt s, Expr e)
{
  CHECK_TABLE ();
  PST_StmtInsertExprBeforeLabel (Global_Table, s, e);
  return;
}

/*! See PST_StmtInsertStmtAfter(). */
void
PSI_StmtInsertStmtAfter (Stmt s, Stmt t)
{
  CHECK_TABLE ();
  PST_StmtInsertStmtAfter (Global_Table, s, t);
  return;
}

/*! See PST_StmtInsertStmtBefore(). */
void
PSI_StmtInsertStmtBefore (Stmt s, Stmt t)
{
  CHECK_TABLE ();
  PST_StmtInsertStmtBefore (Global_Table, s, t);
  return;
}

/*! See PST_StmtInsertStmtBeforeLabel(). */
void
PSI_StmtInsertStmtBeforeLabel (Stmt s, Stmt t)
{
  CHECK_TABLE ();
  PST_StmtInsertStmtBeforeLabel (Global_Table, s, t);
  return;
}

/*! See PST_GetFileNeededSymbols(). */
KeyList
PSI_GetFileNeededSymbols (int f)
{
  CHECK_TABLE ();
  return (PST_GetFileNeededSymbols (Global_Table, f));
}

/*! See PST_GetFuncDclNeededSymbols(). */
KeyList
PSI_GetFuncDclNeededSymbols (FuncDcl f)
{
  CHECK_TABLE ();
  return (PST_GetFuncDclNeededSymbols (Global_Table, f));
}

/*! See #PST_GetFuncDclNeededSymbolsK(). */
KeyList
PSI_GetFuncDclNeededSymbolsK (Key k)
{
  CHECK_TABLE ();
  return (PST_GetFuncDclNeededSymbolsK (Global_Table, k));
}

/*! See PST_GetTypeDclNeededSymbols(). */
KeyList
PSI_GetTypeDclNeededSymbols (TypeDcl t)
{
  CHECK_TABLE ();
  return (PST_GetTypeDclNeededSymbols (Global_Table, t));
}

/*! See #PST_GetTypeDclNeededSymbolsK(). */
KeyList
PSI_GetTypeDclNeededSymbolsK (Key k)
{
  CHECK_TABLE ();
  return (PST_GetTypeDclNeededSymbolsK (Global_Table, k));
}

/*! See PST_GetVarDclNeededSymbols(). */
KeyList
PSI_GetVarDclNeededSymbols (VarDcl v)
{
  CHECK_TABLE ();
  return (PST_GetVarDclNeededSymbols (Global_Table, v));
}

/*! See #PST_GetVarDclNeededSymbolsK(). */
KeyList
PSI_GetVarDclNeededSymbolsK (Key k)
{
  CHECK_TABLE ();
  return (PST_GetVarDclNeededSymbolsK (Global_Table, k));
}

/*! See PST_GetStructDclNeededSymbols(). */
KeyList
PSI_GetStructDclNeededSymbols (StructDcl s)
{
  CHECK_TABLE ();
  return (PST_GetStructDclNeededSymbols (Global_Table, s));
}

/*! See #PST_GetStructDclNeededSymbolsK(). */
KeyList
PSI_GetStructDclNeededSymbolsK (Key k)
{
  CHECK_TABLE ();
  return (PST_GetStructDclNeededSymbolsK (Global_Table, k));
}

/*! See PST_GetUnionDclNeededSymbols(). */
KeyList
PSI_GetUnionDclNeededSymbols (UnionDcl u)
{
  CHECK_TABLE ();
  return (PST_GetUnionDclNeededSymbols (Global_Table, u));
}

/*! See #PST_GetUnionDclNeededSymbolsK(). */
KeyList
PSI_GetUnionDclNeededSymbolsK (Key k)
{
  CHECK_TABLE ();
  return (PST_GetUnionDclNeededSymbolsK (Global_Table, k));
}

/*! See PST_GetStmtNeededSymbols(). */
KeyList
PSI_GetStmtNeededSymbols (Stmt s)
{
  CHECK_TABLE ();
  return (PST_GetStmtNeededSymbols (Global_Table, s));
}

/*! See PST_GetExprNeededSymbols(). */
KeyList
PSI_GetExprNeededSymbols (Expr e)
{
  CHECK_TABLE ();
  return (PST_GetExprNeededSymbols (Global_Table, e));
}

/* Wrappers for symtab.h. */
/*! See PST_FlushFile(). */
void
PSI_FlushFile (int file, bool write)
{
  CHECK_TABLE ();
  return (PST_FlushFile (Global_Table, file, write));
}

/*! See PST_FlushEntry(). */
void
PSI_FlushEntry (Key key, bool write)
{
  CHECK_TABLE ();
  return (PST_FlushEntry (Global_Table, key, write));
}

/*! See PST_RotateFile(). */
void
PSI_RotateFile ()
{
  CHECK_TABLE ();
  PST_RotateFile (Global_Table);
  return;
}

/*! See PST_AddEntry(). */
Key
PSI_AddEntry (_EntryType type, void *entry)
{
  CHECK_TABLE ();
  return (PST_AddEntry (Global_Table, type, entry));
}

/*! See PST_AddSymTabEntry(). */
Key
PSI_AddSymTabEntry (SymTabEntry entry)
{
  CHECK_TABLE ();
  return (PST_AddSymTabEntry (Global_Table, entry));
}

/*! See PST_AddFuncDclEntry(). */
Key
PSI_AddFuncDclEntry (FuncDcl func_dcl)
{
  CHECK_TABLE ();
  return (PST_AddFuncDclEntry (Global_Table, func_dcl));
}

/*! See PST_AddTypeDclEntry(). */
Key
PSI_AddTypeDclEntry (TypeDcl type_dcl, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_AddTypeDclEntry (Global_Table, type_dcl, type));
}

/*! See PST_AddVarDclEntry(). */
Key
PSI_AddVarDclEntry (VarDcl var_dcl, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_AddVarDclEntry (Global_Table, var_dcl, type));
}

/*! See PST_AddStructDclEntry(). */
Key
PSI_AddStructDclEntry (StructDcl struct_dcl)
{
  CHECK_TABLE ();
  return (PST_AddStructDclEntry (Global_Table, struct_dcl));
}

/*! See PST_AddUnionDclEntry(). */
Key
PSI_AddUnionDclEntry (UnionDcl union_dcl)
{
  CHECK_TABLE ();
  return (PST_AddUnionDclEntry (Global_Table, union_dcl));
}

/*! See PST_AddEnumDclEntry(). */
Key
PSI_AddEnumDclEntry (EnumDcl enum_dcl)
{
  CHECK_TABLE ();
  return (PST_AddEnumDclEntry (Global_Table, enum_dcl));
}

/*! See PST_AddAsmDclEntry(). */
Key
PSI_AddAsmDclEntry (AsmDcl asm_dcl)
{
  CHECK_TABLE ();
  return (PST_AddAsmDclEntry (Global_Table, asm_dcl));
}

/*! See PST_AddStmtEntry(). */
Key
PSI_AddStmtEntry (Stmt stmt)
{
  CHECK_TABLE ();
  return (PST_AddStmtEntry (Global_Table, stmt));
}

/*! See PST_AddExprEntry(). */
Key
PSI_AddExprEntry (Expr expr)
{
  CHECK_TABLE ();
  return (PST_AddExprEntry (Global_Table, expr));
}

/*! See PST_AddFieldEntry(). */
Key
PSI_AddFieldEntry (Field field)
{
  CHECK_TABLE ();
  return (PST_AddFieldEntry (Global_Table, field));
}

/*! See PST_AddEnumFieldEntry(). */
Key
PSI_AddEnumFieldEntry (EnumField enum_field)
{
  CHECK_TABLE ();
  return (PST_AddEnumFieldEntry (Global_Table, enum_field));
}

/*! See PST_AddLabelEntry(). */
Key
PSI_AddLabelEntry (Label label)
{
  CHECK_TABLE ();
  return (PST_AddLabelEntry (Global_Table, label));
}

/*! See PST_GetEntry(). */
_EntryType
PSI_GetEntry (Key key, void **entry)
{
  CHECK_TABLE ();
  return (PST_GetEntry (Global_Table, key, entry));
}

/*! See PST_GetSymTabEntry(). */
SymTabEntry
PSI_GetSymTabEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetSymTabEntry (Global_Table, key));
}

/*! See PST_GetSymTabEntryFromSource(). */
SymTabEntry
PSI_GetSymTabEntryFromSource (Key key, _STSearchOrder source)
{
  CHECK_TABLE ();
  return (PST_GetSymTabEntryFromSource (Global_Table, key, source));
}

/*! See PST_GetSymTabEntryCopyFromSource(). */
SymTabEntry
PSI_GetSymTabEntryCopyFromSource (Key key, _STSearchOrder source)
{
  CHECK_TABLE ();
  return (PST_GetSymTabEntryCopyFromSource (Global_Table, key, source));
}

/*! See PST_GetFuncDclEntry(). */
FuncDcl
PSI_GetFuncDclEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetFuncDclEntry (Global_Table, key));
}

/*! See PST_GetTypeDclEntry(). */
TypeDcl
PSI_GetTypeDclEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetTypeDclEntry (Global_Table, key));
}

/*! See PST_GetVarDclEntry(). */
VarDcl
PSI_GetVarDclEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetVarDclEntry (Global_Table, key));
}

/*! See PST_GetStructDclEntry(). */
StructDcl
PSI_GetStructDclEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetStructDclEntry (Global_Table, key));
}

/*! See PST_GetUnionDclEntry(). */
UnionDcl
PSI_GetUnionDclEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetUnionDclEntry (Global_Table, key));
}

/*! See PST_GetEnumDclEntry(). */
EnumDcl
PSI_GetEnumDclEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetEnumDclEntry (Global_Table, key));
}

/*! See PST_GetAsmDclEntry(). */
AsmDcl
PSI_GetAsmDclEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetAsmDclEntry (Global_Table, key));
}

/*! See PST_GetStmtEntry(). */
Stmt
PSI_GetStmtEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetStmtEntry (Global_Table, key));
}

/*! See PST_GetExprEntry(). */
Expr
PSI_GetExprEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetExprEntry (Global_Table, key));
}

/*! See PST_GetFieldEntry(). */
Field
PSI_GetFieldEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetFieldEntry (Global_Table, key));
}

/*! See PST_GetEnumFieldEntry(). */
EnumField
PSI_GetEnumFieldEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetEnumFieldEntry (Global_Table, key));
}

/*! See PST_GetLabelEntry(). */
Label
PSI_GetLabelEntry (Key key)
{
  CHECK_TABLE ();
  return (PST_GetLabelEntry (Global_Table, key));
}

/*! See PST_OrderTypeUses(). */
void
PSI_OrderTypeUses ()
{
  CHECK_TABLE ();
  PST_OrderTypeUses (Global_Table);
  return;
}

/*! See PST_ResetOrder(). */
void
PSI_ResetOrder ()
{
  CHECK_TABLE ();
  PST_ResetOrder (Global_Table);
  return;
}

/*! See PST_RemoveEntry(). */
void
PSI_RemoveEntry (Key key)
{
  CHECK_TABLE ();
  PST_RemoveEntry (Global_Table, key);
  return;
}

/*! See PST_GetFileNumEntries(). */
int
PSI_GetFileNumEntries (int f)
{
  CHECK_TABLE ();
  return (PST_GetFileNumEntries (Global_Table, f));
}

/*! See PST_GetFileType(). */
_FileType
PSI_GetFileType (int f)
{
  CHECK_TABLE ();
  return (PST_GetFileType (Global_Table, f));
}

/*! See PST_AddNewScope(). */
Key
PSI_AddNewScope (Key key)
{
  CHECK_TABLE ();
  return (PST_AddNewScope (Global_Table, key));
}

/*! See PST_AddEntryToScope(). */
void
PSI_AddEntryToScope (Key scope_key, Key entry_key)
{
  CHECK_TABLE ();
  PST_AddEntryToScope (Global_Table, scope_key, entry_key);
}

/*! See PST_RemoveEntryFromScope(). */
void
PSI_RemoveEntryFromScope (Key scope_key, Key entry_key)
{
  CHECK_TABLE ();
  PST_RemoveEntryFromScope (Global_Table, scope_key, entry_key);
}

/*! See PST_GetScope(). */
Scope
PSI_GetScope (Key key)
{
  CHECK_TABLE ();
  return (PST_GetScope (Global_Table, key));
}

/*! See PST_GetFileEntryByType(). */
Key
PSI_GetFileEntryByType (int file_key, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetFileEntryByType (Global_Table, file_key, type));
}

/*! See PST_GetFileEntryByTypeNext(). */
Key
PSI_GetFileEntryByTypeNext (Key last, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetFileEntryByTypeNext (Global_Table, last, type));
}

/*! See PST_GetTableEntryByType(). */
Key
PSI_GetTableEntryByType (_EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetTableEntryByType (Global_Table, type));
}

/*! See PST_GetTableEntryByTypeNext(). */
Key
PSI_GetTableEntryByTypeNext (Key last, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetTableEntryByTypeNext (Global_Table, last, type));
}

/*! See PST_ClearEntry(). */
void
PSI_ClearEntry (Key key)
{
  CHECK_TABLE ();
  PST_ClearEntry (Global_Table, key);
  return;
}

/*! See PST_GetScopeEntryByType(). */
ScopeEntry
PSI_GetScopeEntryByType (Key scope_key, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetScopeEntryByType (Global_Table, scope_key, type));
}

/*! See PST_GetScopeEntryByTypeNext(). */
ScopeEntry
PSI_GetScopeEntryByTypeNext (ScopeEntry last, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetScopeEntryByTypeNext (Global_Table, last, type));
}

/*! See PST_GetScopeEntryByTypeR(). */
ScopeEntry
PSI_GetScopeEntryByTypeR (Key scope_key, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetScopeEntryByTypeR (Global_Table, scope_key, type));
}

/*! See PST_GetScopeEntryByTypeRNext(). */
ScopeEntry
PSI_GetScopeEntryByTypeRNext (ScopeEntry last, _EntryType type)
{
  CHECK_TABLE ();
  return (PST_GetScopeEntryByTypeRNext (Global_Table, last, type));
}

/*! See PST_GetGlobalScope(). */
Key
PSI_GetGlobalScope ()
{
  CHECK_TABLE ();
  return (PST_GetGlobalScope (Global_Table));
}

/*! See PST_GetFileScope(). */
Key
PSI_GetFileScope (int file)
{
  CHECK_TABLE ();
  return (PST_GetFileScope (Global_Table, file));
}

