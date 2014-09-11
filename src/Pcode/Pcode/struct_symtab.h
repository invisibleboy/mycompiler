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
 * \brief Routines to manage Pcode structures and the symbol table.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains declarations for routines to manage the interaction
 * between Pcode structures and the symbol table.
 */

#ifndef _PCODE_STRUCT_SYMTAB_H_
#define _PCODE_STRUCT_SYMTAB_H_

#include <config.h>
#include "pcode.h"
#include "symtab.h"

/* Functions to manage types. */
extern Type PST_FindBasicType (SymbolTable table, _BasicType basic_type);
extern Type PST_FindBasicTypeWithQual (SymbolTable table,
				       _BasicType basic_type,
				       _TypeQual type_qual);
extern Type PST_ScopeFindTypeDcl (SymbolTable table, Key scope_key,
				  TypeDcl type_dcl);
extern Type PST_FindPointerToType (SymbolTable table, Type type);

extern _BasicType PST_GetTypeBasicType (SymbolTable table, Type type);
extern _TypeQual PST_SetTypeQualifier (SymbolTable table, Type *type,
				       _TypeQual type_qual);
extern _TypeQual PST_GetTypeQualifier (SymbolTable table, Type type);
extern _TypeQual PST_ClrTypeQualifier (SymbolTable table, Type *type,
				       _TypeQual type_qual);
extern Type PST_FindTypeSetQualifier (SymbolTable table, Type type,
				      _TypeQual type_qual);
extern Type PST_FindTypeClrQualifier (SymbolTable table, Type type,
				      _TypeQual type_qual);

/*! \brief Returns the first non-BT_TYPEDEF TypeDcl for a Type.
 *
 * \param t
 *  the symbol table.
 * \param u
 *  the Type to retrieve.
 *
 * \return
 *  The first non-BT_TYPEDEF TypeDcl for a Type.
 *
 * This function returns the first non-BT_TYPEDEF TypeDcl for a Type.  This
 * replaces a call to PST_ReduceTypedefs() before PST_GetTypeDclEntry().
 *
 * \note \a t is referenced more than once.
 *
 * \sa PST_GetTypeStructDcl(), PST_GetTypeUnionDcl()
 */
#define PST_GetTypeTypeDcl(t, u) \
          (PST_GetTypeDclEntry ((t), PST_ReduceTypedefs ((t), (u))))

extern StructDcl PST_GetTypeStructDcl (SymbolTable table, Type type);
extern UnionDcl PST_GetTypeUnionDcl (SymbolTable table, Type type);
extern Type PST_GetTypeType (SymbolTable table, Type type);
extern char *PST_GetTypeName (SymbolTable table, Type type);
extern Expr PST_GetTypeArraySize (SymbolTable table, Type type);
extern Param PST_GetTypeParam (SymbolTable table, Type type);
extern int PST_GetTypeSize (SymbolTable table, Type type);

extern int PST_SetTypeAlignment (SymbolTable table, Type *type,
				 int alignment);
extern int PST_GetTypeAlignment (SymbolTable table, Type type);
extern Type PST_FindTypeSetAlignment (SymbolTable table, Type type,
				      int alignment);

extern int PST_GetTypeLineno (SymbolTable table, Type type);
extern int PST_GetTypeColno (SymbolTable table, Type type);
extern char *PST_GetTypeFilename (SymbolTable table, Type type);

extern int PST_IncTypeRefCount (SymbolTable table, Type type);
extern int PST_DecTypeRefCount (SymbolTable table, Type type);

extern Type PST_SetTypeSigned (SymbolTable table, Type *type);
extern Type PST_FindTypeSetSigned (SymbolTable table, Type type);
extern Type PST_SetTypeUnsigned (SymbolTable table, Type *type);
extern Type PST_FindTypeSetUnsigned (SymbolTable table, Type type);

/*! \brief Gets the type resulting from dereferencing a pointer.
 *
 * \param t
 *  the symbol table.
 * \param u
 *  the key of the type to dereference.
 *
 * \return
 *  The key of the type pointed to by \a u. */
#define PST_DereferenceType(t, u) (PST_GetTypeType ((t), (u)))

extern Type PST_DereferencePointers (SymbolTable table, Type type);
extern Type PST_GetBaseType (SymbolTable table, Type type);
extern Type PST_ReduceTypedefs (SymbolTable table, Type type);
extern Type PST_ReduceExplicitTypedefs (SymbolTable table, Type type);
extern Type PST_ReduceImplicitTypedefs (SymbolTable table, Type type);

/* Pcode structure access functions. */
extern Type PST_SetFuncDclType (SymbolTable table, FuncDcl func_dcl,
				Type type);
extern char *PST_SetFuncDclName (SymbolTable table, FuncDcl fun_dcl,
				 char *name);
extern Type PST_SetParamKey (SymbolTable table, Param param, Type type);
extern Type PST_SetTypeDclType (SymbolTable table, TypeDcl type_dcl,
				Type type);
extern char *PST_SetTypeDclName (SymbolTable table, TypeDcl type_dcl,
				 char *name);
extern Type PST_SetVarDclType (SymbolTable table, VarDcl var_dcl, Type type);
extern char *PST_SetVarDclName (SymbolTable table, VarDcl var_dcl, char *name);
extern char *PST_SetStructDclName (SymbolTable table, StructDcl struct_dcl,
				   char *name);
extern char *PST_SetUnionDclName (SymbolTable table, UnionDcl union_dcl,
				  char *name);
extern char *PST_SetEnumDclName (SymbolTable table, EnumDcl enum_dcl,
				 char *name);
extern Type PST_SetFieldType (SymbolTable table, Field field, Type type);
extern char *PST_SetFieldName (SymbolTable table, Field field, char *name);
extern char *PST_SetEnumFieldName (SymbolTable table, EnumField enum_field,
				   char *name);
extern Type PST_SetExprType (SymbolTable table, Expr expr, Type type);
extern Type PST_SetExprVType (SymbolTable table, Expr expr, Type type);

extern Dcl PST_RemoveDcl (SymbolTable table, Dcl d);
extern FuncDcl PST_RemoveFuncDcl (SymbolTable table, FuncDcl f);
extern TypeDcl PST_RemoveTypeDcl (SymbolTable table, TypeDcl t);
extern TypeList PST_RemoveTypeList (SymbolTable table, TypeList t);
extern VarList PST_RemoveVarList (SymbolTable table, VarList v);
extern VarDcl PST_RemoveVarDcl (SymbolTable table, VarDcl v);
extern Init PST_RemoveInit (SymbolTable table, Init i);
extern StructDcl PST_RemoveStructDcl (SymbolTable table, StructDcl s);
extern UnionDcl PST_RemoveUnionDcl (SymbolTable table, UnionDcl u);
extern Field PST_RemoveField (SymbolTable table, Field f);
extern Stmt PST_RemoveStmtNode (SymbolTable table, Stmt s);
extern Stmt PST_RemoveStmt (SymbolTable table, Stmt s);
extern Label PST_RemoveLabel (SymbolTable table, Label l);
extern Compound PST_RemoveCompound (SymbolTable table, Compound c);
extern IfStmt PST_RemoveIfStmt (SymbolTable table, IfStmt i);
extern SwitchStmt PST_RemoveSwitchStmt (SymbolTable table, SwitchStmt s);
extern Pstmt PST_RemovePstmt (SymbolTable table, Pstmt p);
extern Mutex PST_RemoveMutex (SymbolTable table, Mutex m);
extern Cobegin PST_RemoveCobegin (SymbolTable table, Cobegin c);
extern BodyStmt PST_RemoveBodyStmt (SymbolTable table, BodyStmt b);
extern EpilogueStmt PST_RemoveEpilogueStmt (SymbolTable table, EpilogueStmt e);
extern ParLoop PST_RemoveParLoop (SymbolTable table, ParLoop p);
extern SerLoop PST_RemoveSerLoop (SymbolTable table, SerLoop s);
extern AsmStmt PST_RemoveAsmStmt (SymbolTable table, AsmStmt a);
extern Expr PST_RemoveExprNode (SymbolTable table, Expr e);
extern Expr PST_RemoveExpr (SymbolTable table, Expr e);
extern AsmDcl PST_RemoveAsmDcl (SymbolTable table, AsmDcl a);
extern SymTabEntry PST_RemoveSymTabEntry (SymbolTable table, SymTabEntry s);
extern IPSymTabEnt PST_RemoveIPSymTabEnt (SymbolTable table, IPSymTabEnt i);
extern SymbolTable PST_RemoveSymbolTable (SymbolTable table);

extern FuncDcl PST_GetScopeParentFunc (SymbolTable table, Key scope_key);

/*! \brief Returns the parent FuncDcl for a Stmt.
 *
 * \param t
 *  the symbol table.
 * \param s 
 *  the Stmt to inspect.
 *
 * \return
 *  The parent FuncDcl for a Stmt, or NULL if none exists.
 *
 * \note \a t is referenced more than once.
 */
#define PST_GetStmtParentFunc(t, s) \
          (PST_GetScopeParentFunc ((t), PST_GetStmtScope ((t), (s))))

/*! \brief Returns the parent FuncDcl for an Expr.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to inspect.
 *
 * \return
 *  The parent FuncDcl for an Expr, or NULL if none exists.
 *
 * \note \a t is referenced more than once.
 */
#define PST_GetExprParentFunc(t, e) \
          (PST_GetScopeParentFunc ((t), PST_GetExprScope ((t), (e))))

/*! \brief Copies a FuncDcl and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the FuncDcl to copy.
 *
 * \return
 *  A copy of the FuncDcl.  The symbol table is updated so that the copy
 *  is completely distinct from \a f.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyFuncDclToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyFuncDclToTableScope(), P_CopyFuncDcl(), copy_func_dcl()
 */
#define PST_CopyFuncDcl(t, f) \
          (PST_CopyFuncDclToTableScope ((t), Invalid_Key, (t), (f), FALSE))

/*! \brief Copies a TypeDcl and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param u
 *  the TypeDcl to copy.
 *
 * \return
 *  A copy of the TypeDcl.  The symbol table is updated so that the copy
 *  is completely distinct from \a t.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyTypeDclToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa #PST_CopyFuncDclToTableScope(), P_CopyTypeDcl(), copy_type_dcl()
 */
#define PST_CopyTypeDcl(t, u) \
          (PST_CopyTypeDclToTableScope ((t), Invalid_Key, (t), (u), FALSE))

/*! \brief Copies a VarDcl and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param v
 *  the VarDcl to copy.
 *
 * \return
 *  A copy of the VarDcl.  The symbol table is updated so that the copy
 *  is completely distinct from \a v.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyVarDclToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyVarDclToTableScope() P_CopyVarDcl(), copy_var_dcl()
 */
#define PST_CopyVarDcl(t, v) \
          (PST_CopyVarDclToTableScope ((t), Invalid_Key, (t), (v), FALSE))

/*! \brief Copies a StructDcl and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the StructDcl to copy.
 *
 * \return
 *  A copy of the StructDcl.  The symbol table is updated so that the copy
 *  is completely distinct from \a s.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyStructDclToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyStructDclToTableScope(), P_CopyStructDcl(), copy_struct_dcl()
 */
#define PST_CopyStructDcl(t, s) \
          (PST_CopyStructDclToTableScope ((t), Invalid_Key, (t), (s), FALSE))

/*! \brief Copies a UnionDcl and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param u
 *  the UnionDcl to copy.
 *
 * \return
 *  A copy of the UnionDcl.  The symbol table is updated so that the copy
 *  is completely distinct from \a u.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyUnionDclToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyUnionDclToTableScope(), P_CopyUnionDcl(), copy_union_dcl()
 */
#define PST_CopyUnionDcl(t, u) \
          (PST_CopyUnionDclToTableScope ((t), Invalid_Key, (t), (u), FALSE))

/*! \brief Copies a Field and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the Field to copy.
 *
 * \return
 *  A copy of the Field.  The symbol table is updated so that the copy
 *  is completely distinct from \a f.
 *
 * Copies a Field.  The Field.parent_key field is not copied.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyFieldToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyFieldToTableScope(), P_CopyField(), copy_field()
 */
#define PST_CopyField(t, f) \
          (PST_CopyFieldToTableScope ((t), Invalid_Key, (t), (f), FALSE))

/*! \brief Copies an EnumDcl and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the EnumDcl to copy.
 *
 * \return
 *  A copy of the EnumDcl.  The symbol table is updated so that the copy
 *  is completely distinct from \a e.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyEnumDclToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyEnumDclToTableScope(), P_CopyEnumDcl(), copy_enum_dcl()
 */
#define PST_CopyEnumDcl(t, e) \
          (PST_CopyEnumDclToTableScope ((t), Invalid_Key, (t), (e), FALSE))

/*! \brief Copies an EnumField and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the EnumField to copy.
 *
 * \return
 *  A copy of the EnumField.  The symbol table is updated so that the copy
 *  is completely distinct from \a f.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyEnumFieldToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyEnumFieldToTableScope(), P_CopyEnumField(), copy_enum_field()
 */
#define PST_CopyEnumField(t, f) \
          (PST_CopyEnumFieldToTableScope ((t), Invalid_Key, (t), (f), FALSE))

/*! \brief Copies a single Stmt and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the Stmt to copy.
 *
 * \return
 *  A copy of the Stmt.  The symbol table is updated so that the copy
 *  is completely distinct from \a s.
 *
 * Only the given Stmt is copied, not the entire list.
 *
 * ::Stmt.lex_prev, ::Stmt.lex_next, ::Stmt.parent, ::Stmt.parent_func,
 * and ::Stmt.parent_expr are set to null in the new Stmt.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyStmtNodeToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyStmtNodeToTableScope(), P_CopyStmtNode(), #PST_CopyStmt(),
 * copy_stmt_node() */
#define PST_CopyStmtNode(t, s) \
          (PST_CopyStmtNodeToTableScope ((t), Invalid_Key, (t), (s), FALSE))

/*! \brief Copies a single Stmt and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the scope under which the new Stmt is defined.
 * \param u
 *  the Stmt to copy.
 *
 * \return
 *  A copy of the Stmt.  The symbol table is updated so that the copy
 *  is completely distinct from \a s.
 *
 * This function allows you to explicitly specify the scope under which the
 * new Stmt is defined.  If the copy needs to be defined under the same
 * scope as \a s, and if \a s's parents are defined, you can use
 * #PST_CopyStmtNode().
 *
 * Only the given Stmt is copied, not the entire list.
 *
 * ::Stmt.lex_prev, ::Stmt.lex_next, ::Stmt.parent, ::Stmt.parent_func,
 * and ::Stmt.parent_expr are set to null in the new Stmt.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyStmtNodeToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyStmtNodeToTableScope(), P_CopyStmtNode(), PST_CopyStmt(),
 * copy_stmt_node() */
#define PST_CopyStmtNodeToScope(t, s, u) \
          (PST_CopyStmtNodeToTableScope ((t), (s), (t), (u), FALSE))

/*! \brief Copies a Stmt and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the Stmt to copy.
 *
 * \return
 *  A copy of the Stmt.  The symbol table is updated so that the copy
 *  is completely distinct from \a s.
 *
 * The entire list (lex_next chain) is copied.
 *
 * ::Stmt.parent, ::Stmt.parent_func, and ::Stmt.parent_expr are set to null
 * in the new Stmt.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyStmtToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyStmtToTableScope(), P_CopyStmt(), PST_CopyStmtNode(),
 * copy_stmt() */
#define PST_CopyStmt(t, s) \
          (PST_CopyStmtToTableScope ((t), Invalid_Key, (t), (s), FALSE))

/*! \brief Copies a Stmt and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the scope under which the new Stmt is defined.
 * \param u
 *  the Stmt to copy.
 *
 * \return
 *  A copy of the Stmt.  The symbol table is updated so that the copy
 *  is completely distinct from \a s.
 *
 * This function allows you to explicitly specify the scope under which the
 * new Stmt is defined.  If the copy needs to be defined under the same
 * scope as \a s, and if \a s's parents are defined, you can use
 * #PST_CopyStmt().
 *
 * The entire list (lex_next chain) is copied.
 *
 * ::Stmt.parent, ::Stmt.parent_func, and ::Stmt.parent_expr are set to null
 * in the new Stmt.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyStmtToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyStmtToTableScope(), P_CopyStmt(), PST_CopyStmtNode(),
 * copy_stmt() */
#define PST_CopyStmtToScope(t, s, u) \
          (PST_CopyStmtToTableScope ((t), (s), (t), (u), FALSE))

/*! \brief Copies a Label and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param l
 *  the Label to copy.
 *
 * \return
 *  A copy of the Label.  The symbol table is updated so that the copy
 *  is completely distinct from \a l.
 *
 * All labels in the list are copied.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyLabelToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyLabelToTableScope(), P_CopyLabel(), PST_CopyLabelK(),
 * copy_label() */
#define PST_CopyLabel(t, l) \
          (PST_CopyLabelToTableScope ((t), Invalid_Key, (t), (l), FALSE))

/*! \brief Copies a single Expr without operands, siblings, or the next list.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to copy.
 *
 * \return
 *  A copy of the Expr without operands, siblings, or the next list.  The
 *  symbol table is updated so that the copy is completely distinct
 *  from \a e.
 *
 * This function needs to be able to determine the scope from \a e, so it
 * needs the parents of \a e to be defined.  If the parents are not defined,
 * or if you need to explicitly define the scope of the returned Expr for
 * some reason, see #PST_CopyExprNodeToScope().
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyExprNodeToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyExprNodeToTableScope(), P_CopyExprNode(),
 * #PST_CopyExprNodeToScope(), copy_expr_node() */
#define PST_CopyExprNode(t, e) \
          (PST_CopyExprNodeToTableScope ((t), Invalid_Key, (t), (e), FALSE))

/*! \brief Copies a single Expr without operands, siblings, or the next list.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the scope under which the new Expr is defined.
 * \param e
 *  the Expr to copy.
 *
 * \return
 *  A copy of the Expr without operands, sibling, or the next list.  The
 *  symbol table is updated so that the copy is completely distinct
 *  from \a e.
 *
 * This function allows you to explicitly specify the scope under which the
 * new Expr is defined.  If the copy needs to be defined under the same
 * scope as \a e, and if \a e's parents are defined, you can use
 * #PST_CopyExprNode().
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyExprNodeToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyExprNodeToTableScope, #PST_CopyExprNode(), copy_expr_node()
 */
#define PST_CopyExprNodeToScope(t, s, e) \
          (PST_CopyExprNodeToTableScope ((t), (s), (t), (e), FALSE))

/*! \brief Copies an Expr and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to copy.
 *
 * \return
 *  A copy of the Expr.  The symbol table is updated so that the copy
 *  is completely distinct from \a e.
 *
 * Copies an Expr.  Expr.next is not copied.
 *
 * This function needs to be able to determine the scope from \a e, so it
 * needs the parents of \a e to be defined.  If the parents are not defined,
 * or if you need to explicitly define the scope of the returned Expr for
 * some reason, see #PST_CopyExprToScope().
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyExprToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyExprToTableScope(), P_CopyExpr(), #PST_CopyExprToScope(),
 * copy_expr() */
#define PST_CopyExpr(t, e) \
          (PST_CopyExprToTableScope ((t), Invalid_Key, (t), (e), FALSE))

/*! \brief Copies an Expr and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the scope under which the new Expr is defined.
 * \param e
 *  the Expr to copy.
 *
 * \return
 *  A copy of the Expr.  The symbol table is updated so that the copy
 *  is completely distinct from \a e.
 *
 * Copies an Expr.  Expr.next is not copied.
 *
 * This function allows you to explicitly specify the scope under which the
 * new Expr is defined.  If the copy needs to be defined under the same
 * scope as \a e, and if \a e's parents are defined, you can use
 * #PST_CopyExpr().
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyExprToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyExprToTableScope(), #PST_CopyExpr(), copy_expr() */
#define PST_CopyExprToScope(t, s, e) \
          (PST_CopyExprToTableScope ((t), (s), (t), (e), FALSE))

/*! \brief Copies an Expr list and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to copy.
 *
 * \return
 *  A copy of the Expr.  The symbol table is updated so that the copy
 *  is completely distinct from \a e.
 *
 * Copies an Expr.  The entire Expr list (Expr.next) is copied.
 *
 * This function needs to be able to determine the scope from \a e, so it
 * needs the parents of \a e to be defined.  If the parents are not defined,
 * or if you need to explicitly define the scope of the returned Expr for
 * some reason, see #PST_CopyExprListToScope().
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyExprListToTable().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyExprListToTable(), P_CopyExprList(),
 * #PST_CopyExprListToScope(), copy_expr_list() */
#define PST_CopyExprList(t, e) \
          (PST_CopyExprListToTableScope ((t), Invalid_Key, (t), (e), FALSE))

/*! \brief Copies an Expr list and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the scope under which the new Expr is defined.
 * \param e
 *  the Expr to copy.
 *
 * \return
 *  A copy of the Expr.  The symbol table is updated so that the copy
 *  is completely distinct from \a e.
 *
 * Copies an Expr.  The entire Expr list (Expr.next) is copied.
 *
 * This function allows you to explicitly specify the scope under which the
 * new Expr is defined.  If the copy needs to be defined under the same
 * scope as \a e, and if \a e's parents are defined, you can use
 * #PST_CopyExprList().
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyExprListToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyExprListToTableScope(), PST_CopyExprList(), copy_expr_list()
 */
#define PST_CopyExprListToScope(t, s, e) \
          (PST_CopyExprListToTableScope ((t), (s), (t), (e), FALSE))

/*! \brief Copies an AsmDcl and updates the symbol table.
 *
 * \param t
 *  the symbol table.
 * \param a
 *  the AsmDcl to copy.
 *
 * \return
 *  A copy of the AsmDcl.  The symbol table is updated so that the copy
 *  is completely distinct from \a a.
 *
 * Copies an AsmDcl.
 *
 * \note Modules should typically use this function instead of
 *       PST_CopyAsmDclToTableScope().
 * \note \a t is referenced more than once.
 *
 * \sa PST_CopyAsmDclToTableScope(), P_CopyAsmDcl(), copy_asm_dcl() */
#define PST_CopyAsmDcl(t, a) \
          (PST_CopyAsmDclToTableScope ((t), Invalid_Key, (t), (a), FALSE))

#if 0
extern Dcl PST_CopyDclToTable (SymbolTable dst_table, SymbolTable src_table,
			       Dcl d, bool preserve);
#endif
extern FuncDcl PST_CopyFuncDclToTableScope (SymbolTable dst_table,
					    Key dst_scope,
					    SymbolTable src_table, FuncDcl f,
					    bool preserve);
extern TypeDcl PST_CopyTypeDclToTableScope (SymbolTable dst_table,
					    Key dst_scope,
					    SymbolTable src_table, TypeDcl t,
					    bool preserve);
extern VarDcl PST_CopyVarDclToTableScope (SymbolTable dst_table, Key dst_scope,
					  SymbolTable src_table, VarDcl v,
					  bool preserve);
extern StructDcl PST_CopyStructDclToTableScope (SymbolTable dst_table,
						Key dst_scope,
						SymbolTable src_table,
						StructDcl s, bool preserve);
extern UnionDcl PST_CopyUnionDclToTableScope (SymbolTable dst_table,
					      Key dst_scope,
					      SymbolTable src_table,
					      UnionDcl u, bool preserve);
extern Field PST_CopyFieldToTableScope (SymbolTable dst_table, Key dst_scope,
					SymbolTable src_table, Field f,
					bool preserve);
extern EnumDcl PST_CopyEnumDclToTableScope (SymbolTable dst_table,
					    Key dst_scope,
					    SymbolTable src_table, EnumDcl e,
					    bool presrve);
extern EnumField PST_CopyEnumFieldToTableScope (SymbolTable dst_table,
						Key dst_scope,
						SymbolTable src_table,
						EnumField f, bool preserve);
extern Stmt PST_CopyStmtNodeToTableScope (SymbolTable dst_table, Key dst_scope,
					  SymbolTable src_table, Stmt s,
					  bool preserve);
extern Stmt PST_CopyStmtToTableScope (SymbolTable dst_table, Key dst_scope,
				      SymbolTable src_table, Stmt s,
				      bool preserve);
extern Label PST_CopyLabelToTableScope (SymbolTable dst_table, Key dst_scope,
					SymbolTable src_table, Label l,
					bool preserve);
extern Expr PST_CopyExprNodeToTableScope (SymbolTable dst_table, Key dst_scope,
					  SymbolTable src_table, Expr e,
					  bool preserve);
extern Expr PST_CopyExprToTableScope (SymbolTable dst_table, Key dst_scope,
				      SymbolTable src_table, Expr e,
				      bool preserve);
extern Expr PST_CopyExprListToTableScope (SymbolTable dst_table, Key dst_scope,
					  SymbolTable src_table, Expr e,
					  bool preserve);
extern AsmDcl PST_CopyAsmDclToTableScope (SymbolTable dst_table, Key dst_scope,
					  SymbolTable src_table, AsmDcl a,
					  bool preserve);
extern SymTabEntry PST_CopySymTabEntryToTableScope (SymbolTable dst_table,
						    Key dst_scope,
						    SymbolTable src_table,
						    SymTabEntry e,
						    bool preserve);

extern Key PST_ScopeFindFuncScope (SymbolTable table, Key scope_key);

extern void PST_ScopeUpdateExprIDs (SymbolTable table, Key scope_key,
				    Expr expr);

/*! \brief Recursively updates the expression IDs in an Expr.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to update.
 *
 * Recursively assigns new expression IDs to \a e and all operands.
 *
 * \note \a t and \a e are evaluated more than once.
 *       If the scope key is known, it is more efficient to call
 *       PST_ScopeUpdateExprIDs().
 *
 * \sa PST_ScopeNextExprID(), PST_ScopeUpdateExprIDs(),
 * PST_CopyExprToScope(), PST_CopyExprNodeToScope(),
 * PST_CopyExprListToScope(), #PST_CopyExpr(), #PST_CopyExprNode(),
 * #PST_CopyExprList() */
#define PST_UpdateExprIDs(t, e) \
          (PST_ScopeUpdateExprIDs ((t), PST_GetExprScope ((t), (e)), (e)))

/* Functions to create new Pcode data structures that need the symbol table. */
extern Expr PST_ScopeNewExpr (SymbolTable table, Key scope_key);
extern Expr PST_ScopeNewExprWithOpcode (SymbolTable table, Key scope_key,
					_Opcode opcode);
extern Expr PST_ScopeNewStringExpr (SymbolTable table, Key scope_key, char *s);
extern Expr PST_ScopeNewIntExpr (SymbolTable table, Key scope_key, ITintmax i);
extern Expr PST_ScopeNewUIntExpr (SymbolTable table, Key scope_key,
				  ITuintmax i);
extern Expr PST_ScopeNewFloatExpr (SymbolTable table, Key scope_key, double f);
extern Expr PST_ScopeNewDoubleExpr (SymbolTable table, Key scope_key,
				    double d);
extern char *PST_ScopeNewIdentifier (SymbolTable table, Key scope_key,
				     char *tag);

extern Stmt PST_NewCompoundStmt (SymbolTable table, Key scope_key);
extern Stmt PST_NewNoopStmtWithLabel (SymbolTable table, Key scope_key,
				      char *label_val, Key *label_key);
extern Label PST_NewLabel (SymbolTable table, Key scope_key, char *label_val);
extern Label PST_NewLabelTemp (SymbolTable table, Key scope_key, char *tag);
extern Key PST_NewLocalVar (SymbolTable table, Key scope_key, Type type,
			    char *name);
extern Key PST_NewLocalVarTemp (SymbolTable table, Key scope_key, Type type,
				char *tag);
extern Expr PST_NewLocalVarExpr (SymbolTable table, Key scope_key, Type type,
				 char *name);
extern Expr PST_NewLocalVarExprTemp (SymbolTable table, Key scope_key,
				     Type type, char *tag);

/*! \brief Finds the scope of a FuncDcl.
 *
 * \param t
 *  the symbol table.
 * \param f
 *  the FuncDcl to inspect.
 *
 * \return
 *  The key of the FuncDcl's scope.
 *
 * \sa PST_GetGlobalScope(), PST_GetFileScope(),
 * #PST_GetTypeDclScope(), #PST_GetVarDclScope(),
 * #PST_GetStructDclScope(), #PST_GetUnionDclScope(),
 * PST_GetStmtScope(), PST_GetExprScope() */
#define PST_GetFuncDclScope(t, f) \
          (PST_GetScopeFromEntryKey ((t), P_GetFuncDclKey (f)))

/*! \brief Finds the scope of a TypeDcl.
 *
 * \param t
 *  the symbol table.
 * \param u
 *  the TypeDcl to inspect.
 *
 * \return
 *  The key of the TypeDcl's scope.
 *
 * \sa PST_GetGlobalScope(), PST_GetFileScope(),
 * #PST_GetFuncDclScope(), #PST_GetVarDclScope(),
 * #PST_GetStructDclScope(), #PST_GetUnionDclScope(),
 * PST_GetStmtScope(), PST_GetExprScope() */
#define PST_GetTypeDclScope(t, u) \
          (PST_GetScopeFromEntryKey ((t), P_GetTypeDclKey (u)))

/*! \brief Finds the scope of a VarDcl.
 *
 * \param t
 *  the symbol table.
 * \param v
 *  the VarDcl to inspect.
 *
 * \return
 *  The key of the VarDcl's scope.
 *
 * \sa PST_GetGlobalScope(), PST_GetFileScope(),
 * #PST_GetFuncDclScope(), #PST_GetTypeDclScope(),
 * #PST_GetStructDclScope(), #PST_GetUnionDclScope(),
 * PST_GetStmtScope(), PST_GetExprScope() */
#define PST_GetVarDclScope(t, v) \
          (PST_GetScopeFromEntryKey ((t), P_GetVarDclKey (v)))

/*! \brief Finds the scope of a StructDcl.
 *
 * \param t
 *  the symbol table.
 * \param s
 *  the StructDcl to inspect.
 *
 * \return
 *  The key of the StructDcl's scope.
 *
 * \sa PST_GetGlobalScope(), PST_GetFileScope(),
 * #PST_GetFuncDclScope(), #PST_GetTypeDclScope(),
 * #PST_GetVarDclScope(), #PST_GetUnionDclScope(), PST_GetStmtScope(),
 * PST_GetExprScope() */
#define PST_GetStructDclScope(t, s) \
          (PST_GetScopeFromEntryKey ((t), P_GetStructDclKey (s)))

/*! \brief Finds the scope of a UnionDcl.
 *
 * \param t
 *  the symbol table.
 * \param u
 *  the UnionDcl to inspect.
 *
 * \return
 *  The key of the UnionDcl's scope.
 *
 * \sa PST_GetGlobalScope(), PST_GetFileScope(),
 * #PST_GetFuncDclScope(), #PST_GetTypeDclScope(),
 * #PST_GetVarDclScope(), #PST_GetStructDclScope(),
 * PST_GetStmtScope(), PST_GetExprScope() */
#define PST_GetUnionDclScope(t, u) \
          (PST_GetScopeFromEntryKey ((t), P_GetUnionDclKey (u)))

extern Key PST_GetStmtScope (SymbolTable table, Stmt stmt);
extern Key PST_GetExprScope (SymbolTable table, Expr expr);

/*! \brief Retrieves the TypeDcl resulting from an Expr.
 *
 * \param t
 *  the symbol table.
 * \param e
 *  the Expr to inspect.
 *
 * \return
 *  The TypeDcl resulting from evaluating the Expr.
 */
#define PST_GetExprTypeDcl(t, e) \
          (PST_GetTypeDclEntry ((t), PST_ExprType ((t), (e))))

extern bool PST_StmtEncloseInCompound (SymbolTable table, Stmt stmt);
extern void PST_StmtInsertExprAfter (SymbolTable table, Stmt s, Expr e);
extern void PST_StmtInsertExprBefore (SymbolTable table, Stmt s, Expr e);
extern void PST_StmtInsertExprBeforeLabel (SymbolTable table, Stmt s, Expr e);
extern void PST_StmtInsertStmtAfter (SymbolTable table, Stmt s, Stmt t);
extern void PST_StmtInsertStmtBefore (SymbolTable table, Stmt s, Stmt t);
extern void PST_StmtInsertStmtBeforeLabel (SymbolTable table, Stmt s, Stmt t);

extern KeyList PST_GetFileNeededSymbols (SymbolTable table, int f);
extern KeyList PST_GetFuncDclNeededSymbols (SymbolTable table, FuncDcl f);

/*! \brief Finds the external symbols referenced by a FuncDcl.
 *
 * \param t
 *  the symbol table.
 * \param k
 *  the key of the FuncDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by the FuncDcl.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \note \a t is referenced more than once.
 *
 * \sa PST_GetFuncDclNeededSymbols()
 */
#define PST_GetFuncDclNeededSymbolsK(t, k) \
          (PST_GetFuncDclNeededSymbols ((t), PST_GetFuncDclEntry ((t), (k))))

extern KeyList PST_GetTypeDclNeededSymbols (SymbolTable table, TypeDcl t);

/*! \brief Finds the external symbols referenced by a TypeDcl.
 *
 * \param t
 *  the symbol table.
 * \param k
 *  the key of the TypeDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by the TypeDcl.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \note \a t is referenced more than once.
 *
 * \sa PST_GetTypeDclNeededSymbols()
 */
#define PST_GetTypeDclNeededSymbolsK(t, k) \
          (PST_GetTypeDclNeededSymbols ((t), PST_GetTypeDclEntry ((t), (k))))

extern KeyList PST_GetVarDclNeededSymbols (SymbolTable table, VarDcl v);

/*! \brief Finds the external symbols referenced by a VarDcl.
 *
 * \param t
 *  the symbol table.
 * \param k
 *  the key of the VarDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by the VarDcl.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa PST_GetVarDclNeededSymbols() */
#define PST_GetVarDclNeededSymbolsK(t, k) \
          (PST_GetVarDclNeededSymbols ((t), PST_GetVarDclEntry ((t), (k))))

extern KeyList PST_GetStructDclNeededSymbols (SymbolTable table, StructDcl s);

/*! \brief Finds the external symbols referenced by a StructDcl.
 *
 * \param t
 *  the symbol table.
 * \param k
 *  the key of the StructDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by the StructDcl.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa PST_GetStructDclNeededSymbols() */
#define PST_GetStructDclNeededSymbolsK(t, k) \
          (PST_GetStructDclNeededSymbols ((t), \
             PST_GetStructDclEntry ((t), (k))))

extern KeyList PST_GetUnionDclNeededSymbols (SymbolTable table, UnionDcl u);

/*! \brief Finds the external symbols referenced by a UnionDcl.
 *
 * \param t
 *  the symbol table.
 * \param k
 *  the key of the UnionDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by the UnionDcl.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa PST_GetUnionDclNeededSymbols() */
#define PST_GetUnionDclNeededSymbolsK(t, k) \
          (PST_GetUnionDclNeededSymbols ((t), \
             PST_GetUnionDclEntry ((t), (k))))

extern KeyList PST_GetStmtNeededSymbols (SymbolTable table, Stmt s);
extern KeyList PST_GetExprNeededSymbols (SymbolTable table, Expr e);

#endif
