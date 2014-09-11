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
 * This file contains prototypes for routines to work with the Pcode symbol
 * table through an implicit global symbol table.
 *
 * symtab_i.[ch] should contain only wrapper functions calling functions in
 * other _symtab files.  Any function that really modifies a symbol table
 * MUST take the table as an argument.
 *
 * Nothing in symtab_i.[ch] should do any real work, and no function in
 * the Pcode library should call a function in this file.  This file is 
 * intended for modules only.
 *
 * These functions work exactly like their PST_ counterparts.  Before
 * calling any PSI function, you must call PSI_SetTable to set up
 * the global symbol table.
 */

#ifndef _PCODE_SYMTAB_I_H_
#define _PCODE_SYMTAB_I_H_

#include <config.h>
#include "pcode.h"

extern void PSI_SetTable (SymbolTable table);
extern SymbolTable PSI_GetTable (void);
extern void PSI_PushTable (SymbolTable table);
extern SymbolTable PSI_PopTable ();
extern int PSI_GetModifiableFile ();
extern int PSI_GetNumFiles ();
extern _STFlags PSI_SetFlags (_STFlags f);
extern _STFlags PSI_GetFlags ();
extern int PSI_TstFlags (_STFlags f);
extern _STFlags PSI_ClrFlags (_STFlags f);

/* Wrappers for cast_symtab.c. */
extern Expr PSI_UpgradeArithmeticType (Expr expr, Expr parentexpr);
extern Expr PSI_UpgradeType (Expr expr, Type type, Expr parentexpr);
extern void PSI_CastExpr (Expr expr);

/* Wrappers for query_symtab.c. */
extern Type PSI_ExprType (Expr expr);
extern Type PSI_DominantType (Type a, Type b);
extern bool PSI_MatchFuncDcl (FuncDcl a, FuncDcl b);
extern bool PSI_MatchFuncDclK (Key a, Key b);
extern bool PSI_MatchTypeDcl (TypeDcl a, TypeDcl b);
extern bool PSI_MatchType (Type a, Type b);
extern bool PSI_MatchTypeBasicType (Type a, Type b);
extern bool PSI_MatchStructDcl (StructDcl a, StructDcl b);
extern bool PSI_MatchStructDclK (Key a, Key b);
extern bool PSI_MatchUnionDcl (UnionDcl a, UnionDcl b);
extern bool PSI_MatchUnionDclK (Key a, Key b);
extern bool PSI_MatchField (Field a, Field b);
extern bool PSI_MatchFieldK (Key a, Key b);
extern bool PSI_MatchVarList (VarList a, VarList b);
extern bool PSI_MatchVarDcl (VarDcl a, VarDcl b);
extern bool PSI_MatchVarDclK (Key a, Key b);
extern bool PSI_MatchExpr (Expr a, Expr b);
extern bool PSI_MatchParam (Param a, Param b);
extern bool PSI_IsVoidType (Type type);
extern bool PSI_IsVoidTypeExpr (Expr e);
extern bool PSI_IsIntegralType (Type type);
extern bool PSI_IsIntegralTypeExpr (Expr e);
extern bool PSI_IsRealType (Type type);
extern bool PSI_IsRealTypeExpr (Expr e);
extern bool PSI_IsFloatType (Type type);
extern bool PSI_IsFloatTypeExpr (Expr e);
extern bool PSI_IsDoubleType (Type type);
extern bool PSI_IsDoubleTypeExpr (Expr e);
extern bool PSI_IsArithmeticType (Type type);
extern bool PSI_IsArithmeticTypeExpr (Expr e);
extern bool PSI_IsPointerType (Type type);
extern bool PSI_IsPointerTypeExpr (Expr e);
extern bool PSI_IsFundamentalType (Type type);
extern bool PSI_IsFundamentalTypeExpr (Expr e);
extern bool PSI_IsStructureType (Type type);
extern bool PSI_IsStructureTypeExpr (Expr e);
extern bool PSI_IsArrayType (Type type);
extern bool PSI_IsArrayTypeExpr (Expr e);
extern bool PSI_IsFunctionType (Type type);
extern bool PSI_IsFunctionTypeExpr (Expr e);
extern bool PSI_IsSignedType (Type type);
extern bool PSI_IsSignedTypeExpr (Expr e);
extern bool PSI_IsUnsignedType (Type type);
extern bool PSI_IsUnsignedTypeExpr (Expr e);
extern bool PSI_IsEnumType (Type type);
extern bool PSI_IsEnumTypeExpr (Expr e);
extern bool PSI_IsVarargType (Type type);
extern bool PSI_IsVarargTypeExpr (Expr e);
#if 0
extern bool PSI_IsBitFieldType (Type type);
extern bool PSI_IsBitFieldTypeExpr (Expr e);
#endif
extern bool PSI_IsBaseType (Type type);
extern bool PSI_IsBaseTypeExpr (Expr e);

extern bool PSI_EqualStrengthType (Type a, Type b);

/* Wrappers for reduce_symtab.h. */
extern Expr PSI_ReduceExpr (Expr expr);

/* Wrappers for struct_symtab.h. */
extern Type PSI_FindBasicType (_BasicType basic_type);
extern Type PSI_FindBasicTypeWithQual (_BasicType basic_type,
				       _TypeQual type_qual);
extern Type PSI_ScopeFindTypeDcl (Key scope_key, TypeDcl type_dcl);
extern Type PSI_FindPointerToType (Type type);

extern _BasicType PSI_GetTypeBasicType (Type type);
extern _TypeQual PSI_SetTypeQualifier (Type *type, _TypeQual type_qual);
extern _TypeQual PSI_GetTypeQualifier (Type type);
extern _TypeQual PSI_ClrTypeQualifier (Type *type, _TypeQual type_qual);
extern Type PSI_FindTypeSetQualifier (Type type, _TypeQual type_qual);
extern Type PSI_FindTypeClrQualifier (Type type, _TypeQual type_qual);
extern TypeDcl PSI_GetTypeTypeDcl (Type type);
extern StructDcl PSI_GetTypeStructDcl (Type type);
extern UnionDcl PSI_GetTypeUnionDcl (Type type);
extern Type PSI_GetTypeType (Type type);
extern char *PSI_GetTypeName (Type type);
extern Expr PSI_GetTypeArraySize (Type type);
extern Param PSI_GetTypeParam (Type type);
extern int PSI_GetTypeSize (Type type);
extern int PSI_SetTypeAlignment (Type *type, int alignment);
extern int PSI_GetTypeAlignment (Type type);
extern Type PSI_FindTypeSetAlignment (Type type, int alignment);
extern int PSI_GetTypeLineno (Type type);
extern int PSI_GetTypeColno (Type type);
extern char *PSI_GetTypeFilename (Type type);
extern int PSI_IncTypeRefCount (Type type);
extern int PSI_DecTypeRefCount (Type type);
extern Type PSI_SetTypeSigned (Type *type);
extern Type PSI_FindTypeSetSigned (Type type);
extern Type PSI_SetTypeUnsigned (Type *type);
extern Type PSI_FindTypeSetUnsigned (Type type);
extern Type PSI_DereferenceType (Type u);
extern Type PSI_DereferencePointers (Type u);
extern Type PSI_GetBaseType (Type type);
extern Type PSI_ReduceTypedefs (Type type);
extern Type PSI_ReduceExplicitTypedefs (Type type);
extern Type PSI_ReduceImplicitTypedefs (Type type);
extern Type PSI_SetFuncDclType (FuncDcl func_dcl, Type type);
extern char *PSI_SetFuncDclName (FuncDcl func_dcl, char *name);
extern Type PSI_SetParamKey (Param param, Type type);
extern Type PSI_SetTypeDclType (TypeDcl type_dcl, Type type);
extern char *PSI_SetTypeDclName (TypeDcl type_dcl, char *name);
extern Type PSI_SetVarDclType (VarDcl var_dcl, Type type);
extern char *PSI_SetVarDclName (VarDcl var_dcl, char *name);
extern char *PSI_SetStructDclName (StructDcl struct_dcl, char *name);
extern char *PSI_SetUnionDclName (UnionDcl union_dcl, char *name);
extern char *PSI_SetEnumDclName (EnumDcl enum_dcl, char *name);
extern Type PSI_SetFieldType (Field field, Type type);
extern char *PSI_SetFieldName (Field field, char *name);
extern char *PSI_SetEnumFieldName (EnumField enum_field, char *name);
extern Type PSI_SetExprType (Expr expr, Type type);
extern Type PSI_SetExprVType (Expr expr, Type type);
extern Dcl PSI_RemoveDcl (Dcl d);
extern FuncDcl PSI_RemoveFuncDcl (FuncDcl f);
extern TypeDcl PSI_RemoveTypeDcl (TypeDcl t);
extern TypeList PSI_RemoveTypeList (TypeList t);
extern VarList PSI_RemoveVarList (VarList v);
extern VarDcl PSI_RemoveVarDcl (VarDcl v);
extern Init PSI_RemoveInit (Init i);
extern StructDcl PSI_RemoveStructDcl (StructDcl s);
extern UnionDcl PSI_RemoveUnionDcl (UnionDcl u);
extern Field PSI_RemoveField (Field f);
extern Stmt PSI_RemoveStmtNode (Stmt s);
extern Stmt PSI_RemoveStmt (Stmt s);
extern Label PSI_RemoveLabel (Label l);
extern Compound PSI_RemoveCompound (Compound c);
extern IfStmt PSI_RemoveIfStmt (IfStmt i);
extern SwitchStmt PSI_RemoveSwitchStmt (SwitchStmt s);
extern Pstmt PSI_RemovePstmt (Pstmt p);
extern Mutex PSI_RemoveMutex (Mutex m);
extern Cobegin PSI_RemoveCobegin (Cobegin c);
extern BodyStmt PSI_RemoveBodyStmt (BodyStmt b);
extern EpilogueStmt PSI_RemoveEpilogueStmt (EpilogueStmt e);
extern ParLoop PSI_RemoveParLoop (ParLoop p);
extern SerLoop PSI_RemoveSerLoop (SerLoop s);
extern AsmStmt PSI_RemoveAsmStmt (AsmStmt a);
extern Expr PSI_RemoveExprNode (Expr e);
extern Expr PSI_RemoveExpr (Expr e);
extern AsmDcl PSI_RemoveAsmDcl (AsmDcl a);
extern SymTabEntry PSI_RemoveSymTabEntry (SymTabEntry s);
extern IPSymTabEnt PSI_RemoveIPSymTabEnt (IPSymTabEnt i);
extern FuncDcl PSI_GetScopeParentFunc (Key scope_key);
extern FuncDcl PSI_GetStmtParentFunc (Stmt s);
extern FuncDcl PSI_GetExprParentFunc (Expr e);
extern FuncDcl PSI_CopyFuncDcl (FuncDcl f);
extern FuncDcl PSI_CopyFuncDclToScope (Key dst_scope, FuncDcl f);
extern TypeDcl PSI_CopyTypeDcl (TypeDcl t);
extern TypeDcl PSI_CopyTypeDclToScope (Key dst_scope, TypeDcl t);
extern VarDcl PSI_CopyVarDcl (VarDcl v);
extern VarDcl PSI_CopyVarDclToScope (Key dst_scope, VarDcl v);
extern StructDcl PSI_CopyStructDcl (StructDcl s);
extern StructDcl PSI_CopyStructDclToScope (Key dst_scope, StructDcl s);
extern UnionDcl PSI_CopyUnionDcl (UnionDcl u);
extern UnionDcl PSI_CopyUnionDclToScope (Key dst_scope, UnionDcl u);
extern Field PSI_CopyField (Field f);
extern Field PSI_CopyFieldToScope (Key dst_scope, Field f);
extern EnumDcl PSI_CopyEnumDcl (EnumDcl e);
extern EnumDcl PSI_CopyEnumDclToScope (Key dst_scope, EnumDcl e);
extern EnumField PSI_CopyEnumField (EnumField f);
extern EnumField PSI_CopyEnumFieldToScope (Key dst_scope, EnumField f);
extern Stmt PSI_CopyStmtNode (Stmt s);
extern Stmt PSI_CopyStmtNodeToScope (Key dst_scope, Stmt s);
extern Stmt PSI_CopyStmt (Stmt s);
extern Stmt PSI_CopyStmtToScope (Key dst_scope, Stmt s);
extern Label PSI_CopyLabel (Label l);
extern Label PSI_CopyLabelToScope (Key dst_scope, Label l);
extern Expr PSI_CopyExprNode (Expr e);
extern Expr PSI_CopyExprNodeToScope (Key dst_scope, Expr e);
extern Expr PSI_CopyExpr (Expr e);
extern Expr PSI_CopyExprToScope (Key dst_scope, Expr e);
extern Expr PSI_CopyExprList (Expr e);
extern Expr PSI_CopyExprListToScope (Key dst_scope, Expr e);
extern AsmDcl PSI_CopyAsmDcl (AsmDcl a);
extern AsmDcl PSI_CopyAsmDclToScope (Key dst_scope, AsmDcl a);
extern SymTabEntry PSI_CopySymTabEntryToScope (Key dst_scope, SymTabEntry e);
extern Key PSI_ScopeFindFuncScope (Key scope_key);
extern void PSI_ScopeUpdateExprIDs (Key scope_key, Expr expr);
extern void PSI_UpdateExprIDs (Expr e);
extern Expr PSI_ScopeNewExpr (Key scope_key);
extern Expr PSI_ScopeNewExprWithOpcode (Key scope_key, _Opcode opcode);
extern Expr PSI_ScopeNewStringExpr (Key scope_key, char *s);
extern Expr PSI_ScopeNewIntExpr (Key scope_key, int i);
extern Expr PSI_ScopeNewUIntExpr (Key scope_key, int i);
extern Expr PSI_ScopeNewFloatExpr (Key scope_key, double f);
extern Expr PSI_ScopeNewDoubleExpr (Key scope_key, double d);
extern char *PSI_ScopeNewIdentifier (Key scope_key, char *tag);
extern Stmt PSI_NewCompoundStmt (Key scope_key);
extern Stmt PSI_NewNoopStmtWithLabel (Key scope_key, char *label_val,
				      Key *label_key);
extern Label PSI_NewLabel (Key scope_key, char *label_val);
extern Label PSI_NewLabelTemp (Key s, char *u);
extern Key PSI_NewLocalVar (Key scope_key, Type type, char *name);
extern Key PSI_NewLocalVarTemp (Key scope_key, Type type, char *tag);
extern Expr PSI_NewLocalVarExpr (Key scope_key, Type type, char *name);
extern Expr PSI_NewLocalVarExprTemp (Key scope_key, Type type, char *tag);
extern Key PSI_GetFuncDclScope (FuncDcl f);
extern Key PSI_GetTypeDclScope (TypeDcl u);
extern Key PSI_GetVarDclScope (VarDcl v);
extern Key PSI_GetStructDclScope (StructDcl s);
extern Key PSI_GetUnionDclScope (UnionDcl u);
extern Key PSI_GetStmtScope (Stmt stmt);
extern Key PSI_GetExprScope (Expr expr);
extern TypeDcl PSI_GetExprTypeDcl (Expr e);
extern bool PSI_StmtEncloseInCompound (Stmt stmt);
extern void PSI_StmtInsertExprAfter (Stmt s, Expr e);
extern void PSI_StmtInsertExprBefore (Stmt s, Expr e);
extern void PSI_StmtInsertExprBeforeLabel (Stmt s, Expr e);
extern void PSI_StmtInsertStmtAfter (Stmt s, Stmt t);
extern void PSI_StmtInsertStmtBefore (Stmt s, Stmt t);
extern void PSI_StmtInsertStmtBeforeLabel (Stmt s, Stmt t);
extern KeyList PSI_GetFileNeededSymbols (int f);
extern KeyList PSI_GetFuncDclNeededSymbols (FuncDcl f);
extern KeyList PSI_GetFuncDclNeededSymbolsK (Key k);
extern KeyList PSI_GetTypeDclNeededSymbols (TypeDcl t);
extern KeyList PSI_GetTypeDclNeededSymbolsK (Key k);
extern KeyList PSI_GetVarDclNeededSymbols (TypeDcl t);
extern KeyList PSI_GetVarDclNeededSymbolsK (Key k);
extern KeyList PSI_GetStructDclNeededSymbols (TypeDcl t);
extern KeyList PSI_GetStructDclNeededSymbolsK (Key k);
extern KeyList PSI_GetUnionDclNeededSymbols (TypeDcl t);
extern KeyList PSI_GetUnionDclNeededSymbolsK (Key k);
extern KeyList PSI_GetStmtNeededSymbols (Stmt s);
extern KeyList PSI_GetExprNeededSymbols (Expr e);

/* Wrappers for symtab.h. */
extern void PSI_FlushFile (int file, bool write);
extern void PSI_FlushEntry (Key key, bool write);
extern void PSI_RotateFile ();
extern Key PSI_AddEntry (_EntryType type, void *entry);
extern Key PSI_AddSymTabEntry (SymTabEntry entry);
extern Key PSI_AddFuncDclEntry (FuncDcl func_dcl);
extern Key PSI_AddTypeDclEntry (TypeDcl type_dcl, _EntryType type);
extern Key PSI_AddVarDclEntry (VarDcl var_dcl, _EntryType type);
extern Key PSI_AddStructDclEntry (StructDcl struct_dcl);
extern Key PSI_AddUnionDclEntry (UnionDcl union_dcl);
extern Key PSI_AddEnumDclEntry (EnumDcl enum_dcl);
extern Key PSI_AddAsmDclEntry (AsmDcl asm_dcl);
extern Key PSI_AddStmtEntry (Stmt stmt);
extern Key PSI_AddExprEntry (Expr expr);
extern Key PSI_AddFieldEntry (Field field);
extern Key PSI_AddEnumFieldEntry (EnumField enum_field);
extern Key PSI_AddLabelEntry (Label label);
extern _EntryType PSI_GetEntry (Key key, void **entry);
extern SymTabEntry PSI_GetSymTabEntry (Key key);
extern SymTabEntry PSI_GetSymTabEntryFromSource (Key key,
						 _STSearchOrder source);
extern SymTabEntry PSI_GetSymTabEntryCopyFromSource (Key key,
						     _STSearchOrder source);
extern FuncDcl PSI_GetFuncDclEntry (Key key);
extern TypeDcl PSI_GetTypeDclEntry (Key key);
extern VarDcl PSI_GetVarDclEntry (Key key);
extern StructDcl PSI_GetStructDclEntry (Key key);
extern UnionDcl PSI_GetUnionDclEntry (Key key);
extern EnumDcl PSI_GetEnumDclEntry (Key key);
extern AsmDcl PSI_GetAsmDclEntry (Key key);
extern Stmt PSI_GetStmtEntry (Key key);
extern Expr PSI_GetExprEntry (Key key);
extern Field PSI_GetFieldEntry (Key key);
extern EnumField PSI_GetEnumFieldEntry (Key key);
extern Label PSI_GetLabelEntry (Key key);
extern void PSI_OrderTypeUses ();
extern void PSI_ResetOrder ();
#if 0
extern void PSI_RemoveEntry (Key key);
#endif
extern int PSI_GetFileNumEntries (int f);
extern _FileType PSI_GetFileType (int f);
extern Key PSI_AddNewScope (Key key);
extern void PSI_AddEntryToScope (Key scope_key, Key entry_key);
extern void PSI_RemoveEntryFromScope (Key scope_key, Key entry_key);
extern Scope PSI_GetScope (Key key);
extern Key PSI_GetFileEntryByType (int file_key, _EntryType type);
extern Key PSI_GetFileEntryByTypeNext (Key last, _EntryType type);
extern Key PSI_GetTableEntryByType (_EntryType type);
extern Key PSI_GetTableEntryByTypeNext (Key last, _EntryType type);
extern void PSI_ClearEntry (Key key);
extern ScopeEntry PSI_GetScopeEntryByType (Key scope_key, _EntryType type);
extern ScopeEntry PSI_GetScopeEntryByTypeNext (ScopeEntry last,
					       _EntryType type);
extern ScopeEntry PSI_GetScopeEntryByTypeR (Key scope_key, _EntryType type);
extern ScopeEntry PSI_GetScopeEntryByTypeRNext (ScopeEntry last,
						_EntryType type);
extern Key PSI_GetGlobalScope ();
extern Key PSI_GetFileScope (int file);

#endif


