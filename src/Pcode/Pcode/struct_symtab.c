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
 * This file contains definitions for routines to manage the interaction
 * between Pcode structures and the symbol table.
 */

#include <config.h>
#include <string.h>
#include <library/i_list.h>
#include "pcode.h"
#include "symtab.h"
#include "struct.h"
#include "struct_symtab.h"
#include "query.h"
#include "query_symtab.h"
#include "io_util.h"

/*! Constants used by get_*_scope_externs to indicate if we're moving down
 * the scope stack or up an external reference chain. */
#define UP 0
#define DOWN 1

static int basic_types_match (TypeDcl a, TypeDcl b);
static int qualifiers_match (TypeDcl a, TypeDcl b);
static int files_match (TypeDcl a, TypeDcl b);
static int types_match (TypeDcl a, TypeDcl b);
static int names_match (TypeDcl a, TypeDcl b);
static int alignments_match (TypeDcl a, TypeDcl b);

/* Functions to copy structures. */
static Key get_dst_scope (SymbolTable dst_table, SymbolTable src_table,
			  Key src_scope, bool preserve);
static FuncDcl copy_func_dcl (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, FuncDcl f, bool preserve,
			      KeyList *clean);
static TypeDcl copy_type_dcl (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, TypeDcl t, bool preserve,
			      KeyList *clean);
static VarList copy_var_list (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, VarList l, bool preserve,
			      KeyList *clean);
static VarDcl copy_var_dcl (SymbolTable dst_table, Key dst_scope,
			    SymbolTable src_table, VarDcl v, bool preserve,
			    KeyList *clean);
static Init copy_init (SymbolTable dst_table, Key dst_scope,
		       SymbolTable src_table, Init i, bool preserve,
		       KeyList *clean);
static StructDcl copy_struct_dcl (SymbolTable dst_table, Key dst_scope,
				  SymbolTable src_table, StructDcl s,
				  bool preserve, KeyList *clean);
static UnionDcl copy_union_dcl (SymbolTable dst_table, Key dst_scope,
				SymbolTable src_table, UnionDcl u,
				bool preserve, KeyList *clean);
static Field copy_field (SymbolTable dst_table, Key dst_scope,
			 SymbolTable src_table, Field f, bool preserve,
			 KeyList *clean);
static EnumDcl copy_enum_dcl (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, EnumDcl e, bool preserve,
			      KeyList *clean);
static EnumField copy_enum_field (SymbolTable dst_table, Key dst_scope,
				  SymbolTable src_table, EnumField f,
				  bool preserve, KeyList *clean);
static Stmt copy_stmt_node (SymbolTable dst_table, Key dst_scope,
			    SymbolTable src_table, Stmt s, bool preserve,
			    KeyList *clean);
static Stmt copy_stmt (SymbolTable dst_table, Key dst_scope, 
		       SymbolTable src_table, Stmt s, bool preserve,
		       KeyList *clean);
static Label copy_label (SymbolTable dst_table, Key dst_scope,
			 SymbolTable src_table, Label l, bool preserve,
			 KeyList *clean);
static Pstmt copy_pstmt (SymbolTable dst_table, Key dst_scope,
			 SymbolTable src_table, Pstmt p, bool preserve,
			 KeyList *clean);
static Expr copy_expr_node (SymbolTable dst_table, Key dst_scope,
			    SymbolTable src_table, Expr e, bool preserve,
			    KeyList *clean);
static Expr copy_expr (SymbolTable dst_table, Key dst_scope,
		       SymbolTable src_table, Expr e, bool preserve,
		       KeyList *clean);
static Expr copy_expr_list (SymbolTable dst_table, Key dst_scope,
			    SymbolTable src_table, Expr e, bool preserve,
			    KeyList *clean);
static Pragma copy_pragma (SymbolTable dst_table, Key dst_scope,
			   SymbolTable src_table, Pragma p, bool preserve,
			   KeyList *clean);
static AsmDcl copy_asm_dcl (SymbolTable dst_table, Key dst_scope,
			    SymbolTable src_table, AsmDcl a, bool preserve,
			    KeyList *clean);

/* Functions to change keys in structures. */
static void update_func_dcl_keys (SymbolTable dst_table, SymbolTable src_table,
				  FuncDcl f);
static void update_type_dcl_keys (SymbolTable dst_table, SymbolTable src_table,
				  TypeDcl t);
static void update_var_list_keys (SymbolTable dst_table, SymbolTable src_table,
				  VarList l);
static void update_var_dcl_keys (SymbolTable dst_table, SymbolTable src_table,
				 VarDcl v);
static void update_init_keys (SymbolTable dst_table, SymbolTable src_table,
			      Init i);
static void update_struct_dcl_keys (SymbolTable dst_table,
				    SymbolTable src_table, StructDcl s);
static void update_union_dcl_keys (SymbolTable dst_table,
				   SymbolTable src_table, UnionDcl u);
static void update_field_keys (SymbolTable dst_table, SymbolTable src_table,
			       Field f);
static void update_stmt_keys (SymbolTable dst_table, SymbolTable src_table,
			      Stmt s);
static void update_pstmt_keys (SymbolTable dst_table, SymbolTable src_table,
			       Pstmt p);
static void update_expr_keys (SymbolTable dst_table, SymbolTable src_table,
			      Expr e);
static void update_asm_dcl_keys (SymbolTable dst_table, SymbolTable src_table,
				 AsmDcl a);

/* Functions to find symbols needed by a structure that are external relative
 * to a scope. */
static KeyList get_func_dcl_scope_externs (SymbolTable table, Key scope,
					   FuncDcl f, KeyList known);
static KeyList get_type_dcl_scope_externs (SymbolTable table, Key scope,
					   TypeDcl t, KeyList known);
static KeyList get_var_dcl_scope_externs (SymbolTable table, Key scope,
					  VarDcl v, KeyList known);
static KeyList get_struct_dcl_scope_externs (SymbolTable table, Key scope,
					     StructDcl s, KeyList known);
static KeyList get_union_dcl_scope_externs (SymbolTable table, Key scope,
					    UnionDcl u, KeyList known);
static KeyList get_stmt_scope_externs (SymbolTable table, Key scope, Stmt s,
				       KeyList known);
static KeyList get_expr_scope_externs (SymbolTable table, Key scope, Expr e,
				       KeyList known);

/*! \brief Finds a basic type in a scope.
 *
 * \param table
 *  the symbol table.
 * \param basic_type
 *  the type to find.
 *
 * \return
 *  The Type for \a basic_type.
 *
 * Searches the global scope for a basic type.  The basic types were
 * inserted into the global scope by impact-edgcpfe.  Returns the Type.
 */
Type
PST_FindBasicType (SymbolTable table, _BasicType basic_type)
{
  TypeDcl type_dcl;
  Key global_scope_key = PST_GetGlobalScope (table);
  Type type_key = Invalid_Key;

  /* HP's compiler isn't happy with initializing type_key to
   * {global_scope_key.file, 0}. */
  type_key.file = global_scope_key.file;

  type_dcl = P_NewTypeDclWithBasicType (basic_type);
  P_SetTypeDclQualifier (type_dcl, TY_DEFAULT);
  P_SetTypeDclKey (type_dcl, type_key);

  /* If we're trying to find a bitfield type, we may have to make it from
   * the non-bitfield version. */
  if ((basic_type & BT_INTEGRAL) && (basic_type & BT_BIT_FIELD))
    {
      Type base_type = PST_FindBasicType (table, basic_type & ~BT_BIT_FIELD);
      TypeDcl base_td = PST_GetTypeDclEntry (table, base_type);
      
      P_SetTypeDclSize (type_dcl, P_GetTypeDclSize (base_td));
      P_SetTypeDclAlignment (type_dcl, P_GetTypeDclAlignment (base_td));
    }

  type_key = PST_ScopeFindTypeDcl (table, global_scope_key, type_dcl);

  type_dcl = P_RemoveTypeDcl (type_dcl);

  return (type_key);
}

/*! \brief Finds a basic type with a qualifier in a scope.
 *
 * \param table
 *  the symbol table.
 * \param basic_type
 *  the type to find.
 * \param type_qual
 *  the qualifier for the type to find.
 *
 * \return
 *  The Type for \a basic_type with qualifier \a type_qual.
 *
 * Searches the global scope for a basic type.  If the type is not found,
 * inserts a new type.  Returns the Type.
 */
Type
PST_FindBasicTypeWithQual (SymbolTable table, _BasicType basic_type,
			   _TypeQual type_qual)
{
  Type type;

  type = PST_FindBasicType (table, basic_type);
  type = PST_FindTypeSetQualifier (table, type, type_qual);

  return (type);
}

/*! \brief Finds a type in a scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope to search.
 * \param type_dcl
 *  the type to find.
 *
 * \return
 *  The Type corresponding to \a type_dcl
 *
 * Searches a scope for a type_dcl.  If the type_dcl is not found, inserts
 * \a type_dcl into the table.  Returns the TypeDcl's key.
 *
 * If the type is being inserted into the global scope (any scope with
 * sym == 1), it is inserted as ET_TYPE_GLOBAL.  Otherwise, it is
 * inserted as ET_TYPE_LOCAL.
 *
 * \note If this function inserts \a type_dcl into the table, it will make
 *       a copy of the TypeDcl to insert.  The caller is responsible for
 *       freeing the pointer passed as \a type_dcl.
 *
 * \bug This function may or may not copy \a type_dcl.  The extension
 *      field is only copied if \a type_dcl is copied.  As this
 *      function does not indicate if it copied \a type_dcl or not, the
 *      caller must manually determine if it must update the extension
 *      field on the returned TypeDcl somehow.
 */
Type
PST_ScopeFindTypeDcl (SymbolTable table, Key scope_key, TypeDcl type_dcl)
{
  ScopeEntry se;
  TypeDcl table_type, insert_type;
  Key type_key, current_scope_key;
  SymTabEntry entry;
  
  /* The type used for a function argument will have the function's
   * key as the scope.  This type is really a global type, so it should
   * be promoted to the function's scope.
   *
   * Note: types local to a function are found in attached to the compound
   * statement found at FuncDcl.stmt. */
  if ((entry = PST_GetSymTabEntry (table, scope_key)) && \
      P_GetSymTabEntryType (entry) == ET_FUNC)
    {
      scope_key = P_GetSymTabEntryScopeKey (entry);
    }

  /* Search the symbol table for the type. */
  /* Search back up the scope stack for the type. */
  current_scope_key = scope_key;

  for (current_scope_key = scope_key; P_ValidKey (current_scope_key);
       current_scope_key = \
	 P_GetSymTabEntryScopeKey (PST_GetSymTabEntry (table,
						       current_scope_key)))
    {
      for (se = PST_GetScopeEntryByType (table, current_scope_key, ET_TYPE);
	   se; se = PST_GetScopeEntryByTypeNext (table, se, ET_TYPE))
	{
	  table_type = PST_GetTypeDclEntry (table, se->key);
	  
	  if (basic_types_match (type_dcl, table_type))
	    {
	      if (((P_GetTypeDclBasicType (type_dcl) & \
		    (BT_FUNC | BT_ARRAY | BT_TYPEDEF | \
		     BT_POINTER | BT_NAMED_TYPE)) == 0) && \
		  qualifiers_match (type_dcl, table_type) && \
		  files_match (type_dcl, table_type) && \
		  alignments_match (type_dcl, table_type))
		{
		  return (se->key);
		}
	      /* For a typedef, pointer, or struct type, the referenced type 
	       * (the base type or struct itself) must also match. */
	      else if ((P_GetTypeDclBasicType (type_dcl) & \
			(BT_TYPEDEF | BT_POINTER | BT_NAMED_TYPE)))
		{
		  if (qualifiers_match (type_dcl, table_type) && \
		      files_match (type_dcl, table_type) && \
		      names_match (type_dcl, table_type) && \
		      alignments_match (type_dcl, table_type))
		    {
		      if (types_match (type_dcl, table_type))
			{
			  return (se->key);
			}
		      else if (!P_ValidKey (P_GetTypeDclType (table_type)))
			{
			  /* table_type is a forward declaration of a type
			   * that is now defined.  Update table_type's
			   * type key and return table_type's key. */
			  P_SetTypeDclType (table_type,
					    P_GetTypeDclType (type_dcl));
			  return (se->key);
			}
		    }
		}
	    }
	}
    }

  /* No match was found. */
  insert_type = P_CopyTypeDcl (type_dcl);

  /* Add the type to the symbol table. */
  /* Types are global if they are in the global scope (symbol 1). */
  if (scope_key.sym == 1)
    {
      type_key = PST_AddTypeDclEntry (table, insert_type, ET_TYPE_GLOBAL);
    }
  else
    {
      Stmt parent_compound;
      Compound c;

      /* The type is local to a scope, so it should be inserted in the same
       * file as that scope. */
      type_key.file = scope_key.file;
      type_key.sym = 0;

      P_SetTypeDclKey (insert_type, type_key);
      
      type_key = PST_AddTypeDclEntry (table, insert_type, ET_TYPE_LOCAL);

      /* Insert the type into its parent scope's type list. */
      parent_compound = PST_GetStmtEntry (table, scope_key);

      if (parent_compound == NULL || \
	  P_GetStmtType (parent_compound) != ST_COMPOUND)
	P_punt ("struct_symtab.c:PST_ScopeFindTypeDcl:%d Attempting to insert "
		"local type to scope which is not a compound stmt",
		__LINE__ - 2);

      c = P_GetStmtCompound (parent_compound);

      P_SetCompoundTypeList (c, List_insert_last (P_GetCompoundTypeList (c),
						  insert_type));
    }

  /* Add the type to the current scope. */
  PST_AddEntryToScope (table, scope_key, type_key);

  return (type_key);
}

/*! \brief Finds a pointer Type
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to which the pointer points.
 *
 * \return
 *  A Type which is a pointer to Type \a type.
 */
Type
PST_FindPointerToType (SymbolTable table, Type type)
{
  Key void_key;
  Key global_scope_key = PST_GetGlobalScope (table);
  Key type_scope_key;
  Key void_ptr_key = Invalid_Key;
  Key new_ptr_key;
  SymTabEntry type_entry;
  TypeDcl void_ptr_type, new_ptr_type;

  /* HP's compiler isn't happy with initializing void_ptr_key to
   * {global_scope_key.file, 0}. */
  void_ptr_key.file = global_scope_key.file;

  /* The pointer type will be inserted in the same scope as the original
   * type, so we need to retrieve the original type's SymTabEntry to get
   * its scope. */
  if (!(type_entry = PST_GetSymTabEntry (table, type)))
    P_punt ("struct_symtab.c:PST_FindPointerToType:%d SymTabEntry for "
	    "(%d, %d)\ndoes not exist", __LINE__ - 1, type.file, type.sym);

  type_scope_key = P_GetSymTabEntryScopeKey (type_entry);

  new_ptr_key.file = type_scope_key.file;
  new_ptr_key.sym = 0;

  new_ptr_type = P_NewTypeDcl ();
  P_SetTypeDclBasicType (new_ptr_type, BT_POINTER);
  P_SetTypeDclQualifier (new_ptr_type, TY_DEFAULT);
  P_SetTypeDclKey (new_ptr_type, new_ptr_key);
  PST_SetTypeDclType (table, new_ptr_type, type);

  /* Find a pointer to the type. */
  new_ptr_key = PST_ScopeFindTypeDcl (table, type_scope_key, new_ptr_type);
  new_ptr_type = P_RemoveTypeDcl (new_ptr_type);

  /* If the the size and alignment are defined for the pointer, we're done. */
  new_ptr_type = PST_GetTypeDclEntry (table, new_ptr_key);

  if (P_GetTypeDclSize (new_ptr_type) <= 0 || \
      P_GetTypeDclAlignment (new_ptr_type) <= 0)
    {
      /* If the size and alignment are not defined, copy them from the void*
       * pointer.  This pointer was added by impact-edgcpfe to pass the size
       * and alignment to the later Pcode stages. */
       void_key = PST_FindBasicType (table, BT_VOID);

       void_ptr_type = P_NewTypeDcl ();
       P_SetTypeDclBasicType (void_ptr_type, BT_POINTER);
       P_SetTypeDclQualifier (void_ptr_type, TY_DEFAULT);
       P_SetTypeDclKey (void_ptr_type, void_ptr_key);
       P_SetTypeDclType (void_ptr_type, void_key);

       void_ptr_key = PST_ScopeFindTypeDcl (table, global_scope_key,
					    void_ptr_type);
       void_ptr_type = P_RemoveTypeDcl (void_ptr_type);

       void_ptr_type = PST_GetTypeDclEntry (table, void_ptr_key);

       P_SetTypeDclSize (new_ptr_type, P_GetTypeDclSize (void_ptr_type));
       P_SetTypeDclAlignment (new_ptr_type,
			      P_GetTypeDclAlignment (void_ptr_type));
    }

  return (new_ptr_key);
}

/*! \brief Returns the basic type of a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The first non-typedef basic type.
 */
_BasicType
PST_GetTypeBasicType (SymbolTable table, Type type)
{
  TypeDcl t = PST_GetTypeTypeDcl (table, type);

  if (t)
    return (P_GetTypeDclBasicType (t));
  else
    return (0);
}

/*! \brief Sets a qualifier bit for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  a pointer to the Type to modify.  The Type will be updated in place.
 * \param type_qual
 *  the qualifier bit to set.
 *
 * \return
 *  The new Type qualifier.
 *
 * Finds a Type which is \a type + \a type_qual.  Sets the Type pointed
 * to by \a type to the new Type key.
 *
 * \note This function should not be used on the TypeDcl.type field.
 *       The TypeDcl.type field must not be changed.  Use
 *       PST_FindTypeSetQualifier() instead.
 *
 * \sa PST_FindTypeSetQualifier()
 */
_TypeQual
PST_SetTypeQualifier (SymbolTable table, Type *type, _TypeQual type_qual)
{
  Type new_type;

  new_type = PST_FindTypeSetQualifier (table, *type, type_qual);

  if (P_ValidKey (*type))
    PST_DecTypeRefCount (table, *type);

  if (P_ValidKey (new_type))
    PST_IncTypeRefCount (table, new_type);

  *type = new_type;

  return (PST_GetTypeQualifier (table, new_type));
}

/*! \brief Returns the qualifier for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The accumulated qualifier for a Type.
 */
_TypeQual
PST_GetTypeQualifier (SymbolTable table, Type type)
{
  TypeDcl t = NULL;
  _TypeQual result = 0;
  
  while (P_ValidKey (type) && (t = PST_GetTypeDclEntry (table, type)) && \
	 (P_GetTypeDclBasicType (t) & BT_TYPEDEF))
    {
      result |= P_GetTypeDclQualifier (t);
      type = P_GetTypeDclType (t);
    }

  if (t)
    result |= P_GetTypeDclQualifier (t);

  return (result);
}

/*! \brief Clears a qualifier bit for a type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  a pointer to the Type to modify.  The Type will be updated in place.
 * \param type_qual
 *  the qualifier bit to clear.
 *
 * \return
 *  The new Type qualifier.
 *
 * Finds a Type which is \a type_key - \a type_qual.  Sets the Type pointed
 * to by \a type to the new Type key.
 *
 * \note This function should not be used on the TypeDcl.type field.
 *       The TypeDcl.type field must not be changed.  Use
 *       PST_FindTypeClrQualifier() instead.
 *
 * \sa PST_FindTypeClrQualifier()
 */
_TypeQual
PST_ClrTypeQualifier (SymbolTable table, Type *type, _TypeQual type_qual)
{
  Type new_type;

  new_type = PST_FindTypeClrQualifier (table, *type, type_qual);

  if (P_ValidKey (*type))
    PST_DecTypeRefCount (table, *type);

  if (P_ValidKey (new_type))
    PST_IncTypeRefCount (table, new_type);

  *type = new_type;

  return (PST_GetTypeQualifier (table, new_type));
}

/*! \brief Sets a qualifier bit for a type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to receive the new qualifier.
 * \param type_qual
 *  the qualifier bit to set.
 *
 * \return
 *  A new Type.
 *
 * Finds a Type which is \a type + \a type_qual and returns it.
 *
 * \sa PST_SetTypeQualifier()
 */
Type
PST_FindTypeSetQualifier (SymbolTable table, Type type, _TypeQual type_qual)
{
  TypeDcl new_type_dcl;
  Type base_type, new_type;
  Key type_scope_key;
  _TypeQual full_qualifier;
  
  /* Set up the full qualifier for the type.  Remove TY_DEFAULT from the
   * existing qualifier.  It can be restored if it is specified in
   * type_qual. */
  full_qualifier = (PST_GetTypeQualifier (table,
					  type) & ~TY_DEFAULT) | type_qual;

  base_type = PST_ReduceTypedefs (table, type);

  type_scope_key = PST_GetScopeFromEntryKey (table, type);

  new_type.file = type_scope_key.file;
  new_type.sym = 0;

  new_type_dcl = P_NewTypeDcl ();
  P_SetTypeDclKey (new_type_dcl, new_type);
  P_SetTypeDclBasicType (new_type_dcl, BT_TYPEDEF_I);
  P_SetTypeDclQualifier (new_type_dcl, full_qualifier);
  /* Copy the alignment if it is explicitly set. */
  if (full_qualifier & TY_EXP_ALIGN)
    P_SetTypeDclAlignment (new_type_dcl, PST_GetTypeAlignment (table, type));
  PST_SetTypeDclType (table, new_type_dcl, base_type);

  new_type = PST_ScopeFindTypeDcl (table, type_scope_key, new_type_dcl);

  new_type_dcl = P_RemoveTypeDcl (new_type_dcl);

  return (new_type);
}

/*! \brief Clears a qualifier bit for a type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to receive the new qualifier.
 * \param type_qual
 *  the qualifier bit to clear.
 *
 * \return
 *  A new Type.
 *
 * Finds a Type which is \a type_key - \a type_qual and returns it.
 *
 * \sa PST_ClrTypeQualifier()
 */
Type
PST_FindTypeClrQualifier (SymbolTable table, Type type, _TypeQual type_qual)
{
  TypeDcl new_type_dcl;
  Type base_type, new_type;
  Key type_scope_key;
  _TypeQual full_qualifier;
  
  /* Set up the full qualifier for the type.  Remove TY_DEFAULT from the
   * existing qualifier.  It can be restored if it is specified in
   * type_qual. */
  full_qualifier = PST_GetTypeQualifier (table, type) & ~type_qual;

  base_type = PST_ReduceTypedefs (table, type);

  type_scope_key = PST_GetScopeFromEntryKey (table, type);

  new_type.file = type_scope_key.file;
  new_type.sym = 0;

  new_type_dcl = P_NewTypeDcl ();
  P_SetTypeDclKey (new_type_dcl, new_type);
  P_SetTypeDclBasicType (new_type_dcl, BT_TYPEDEF_I);
  P_SetTypeDclQualifier (new_type_dcl, full_qualifier);
  /* Copy the alignment if it is explicitly set. */
  if (full_qualifier & TY_EXP_ALIGN)
    P_SetTypeDclAlignment (new_type_dcl, PST_GetTypeAlignment (table, type));
  PST_SetTypeDclType (table, new_type_dcl, base_type);

  new_type = PST_ScopeFindTypeDcl (table, type_scope_key, new_type_dcl);

  new_type_dcl = P_RemoveTypeDcl (new_type_dcl);

  return (new_type);
}

/*! \brief Returns the StructDcl referenced by a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The StructDcl referenced by the Type.
 *
 * Reduces typedefs on \a type and returns the StructDcl it references.
 * PST_GetTypeBasicType() must return BT_STRUCT for \a type.
 *
 * \sa #PST_GetTypeTypeDcl(), PST_GetTypeUnionDcl()
 */
StructDcl
PST_GetTypeStructDcl (SymbolTable table, Type type)
{
  TypeDcl base_type_dcl = PST_GetTypeTypeDcl (table, type);
  StructDcl result = NULL;

  if (P_GetTypeDclBasicType (base_type_dcl) & BT_STRUCT)
    result = PST_GetStructDclEntry (table, P_GetTypeDclType (base_type_dcl));

  return (result);
}

/*! \brief Returns the UnionDcl referenced by a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The UnionDcl referenced by the Type.
 *
 * Reduces typedefs on \a type and returns the UnionDcl it references.
 * PST_GetTypeBasicType() must return BT_UNION for \a type.
 *
 * \sa #PST_GetTypeTypeDcl(), PST_GetTypeUnionDcl()
 */
UnionDcl
PST_GetTypeUnionDcl (SymbolTable table, Type type)
{
  TypeDcl base_type_dcl = PST_GetTypeTypeDcl (table, type);
  UnionDcl result = NULL;

  if (P_GetTypeDclBasicType (base_type_dcl) & BT_UNION)
    result = PST_GetUnionDclEntry (table, P_GetTypeDclType (base_type_dcl));

  return (result);
}

/*! \brief Returns the type field for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The Type's type field.  This is the key for another Type.
 *
 * Reduces any typedefs referred to by \a type and returns the
 * TypeDcl.type field.
 */
Type
PST_GetTypeType (SymbolTable table, Type type)
{
  TypeDcl t = PST_GetTypeTypeDcl (table, type);
  Type result = P_GetTypeDclType (t);

  return (result);
}

/*! \brief Returns the name field for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The Type's name field.
 *
 * Reduces any typedefs referred to by \a type and returns the
 * TypeDcl.name field.
 */
char *
PST_GetTypeName (SymbolTable table, Type type)
{
  Type base_type = PST_ReduceTypedefs (table, type);
  TypeDcl base_type_dcl = PST_GetTypeDclEntry (table, base_type);
  char *result = NULL;

  if (base_type_dcl)
    result = P_GetTypeDclName (base_type_dcl);

  return (result);
}

/*! \brief Returns the array_size field for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The Type's array_size field.
 *
 * Reduces any typedefs referred to by \a type and returns the
 * TypeDcl.details.array_size field.
 */
Expr
PST_GetTypeArraySize (SymbolTable table, Type type)
{
  TypeDcl t = PST_GetTypeTypeDcl (table, type);

  if (P_GetTypeDclBasicType (t) != BT_ARRAY)
    P_punt ("struct_symtab.c:PST_GetTypeArraySize:%d Type (%d, %d) is not "
	    "an array type", __LINE__ - 1, type.file, type.sym);

  return (P_GetTypeDclArraySize (t));
}

/*! \brief Returns the Param field for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The Type's param field.
 *
 * Reduces any typedefs referred to by \a type and returns the 
 * TypeDcl.details.param field.
 */
Param
PST_GetTypeParam (SymbolTable table, Type type)
{
  TypeDcl t = PST_GetTypeTypeDcl (table, type);

  if (P_GetTypeDclBasicType (t) != BT_FUNC)
    P_punt ("struct_symtab.c:PST_GetTypeParam:%d Type (%d, %d) is not a "
	    "function type", __LINE__ - 1, type.file, type.sym);

  return (P_GetTypeDclParam (t));
}

/*! \brief Returns the size for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The first defined size for a Type.
 */
int
PST_GetTypeSize (SymbolTable table, Type type)
{
  TypeDcl t = PST_GetTypeTypeDcl (table, type);

  return (P_GetTypeDclSize (t));
}

/*! \brief Returns a Type with an explicit alignment.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type for which to adjust the alignment.
 * \param alignment
 *  the new alignment of the Type.
 *
 * \return
 *  The new alignment of the Type.
 *
 * Inserts an implicit typedef with the alignment set to \a alignment
 * and the TY_EXP_ALIGN qualifier set.
 *
 * \note This function should not be used on the TypeDcl.type field.
 *       The TypeDcl.type field must not be changed.  Use
 *       PST_FindTypeSetAlignment() instead.
 *
 * \sa PST_FindTypeSetAlignment()
 */
int
PST_SetTypeAlignment (SymbolTable table, Type *type, int alignment)
{
  Type new_type;

  new_type = PST_FindTypeSetAlignment (table, *type, alignment);

  if (P_ValidKey (*type))
    PST_DecTypeRefCount (table, *type);

  if (P_ValidKey (new_type))
    PST_IncTypeRefCount (table, new_type);

  *type = new_type;

  return (alignment);
}

/*! \brief Returns the alignment for a type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The alignment of the type.
 *
 * Recursively inspects types referenced by \a type and returns the
 * first defined alignment.
 */
int
PST_GetTypeAlignment (SymbolTable table, Type type)
{
  TypeDcl type_dcl;
  int alignment;

  while ((P_ValidKey (type)) && (type_dcl = PST_GetTypeDclEntry (table, type)))
    {
      if ((alignment = P_GetTypeDclAlignment (type_dcl)))
	return (alignment);
      else
	type = P_GetTypeDclType (type_dcl);
    }

  return (0);
}

/*! \brief Returns the key of a typedef with an alignment and updates the Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type for which to adjust the alignment.
 * \param alignment
 *  the new alignment of the type.
 *
 * \return
 *  A Type which is a typedef with the alignment explicitly set.
 *
 * Inserts an implicit typedef with the alignment set to \a alignment and
 * the TY_EXP_ALIGN qualifier set.
 *
 * \sa PST_SetTypeAlignment()
 */
Type
PST_FindTypeSetAlignment (SymbolTable table, Type type, int alignment)
{
  TypeDcl new_type_dcl;
  Type base_type, new_type;
  _TypeQual full_qualifier;
  Key type_scope_key;

  /* Set up the full qualifier for the type. */
  full_qualifier = PST_GetTypeQualifier (table, type) | TY_EXP_ALIGN;
  base_type = PST_ReduceImplicitTypedefs (table, type);

  type_scope_key = PST_GetScopeFromEntryKey (table, type);

  new_type.file = type_scope_key.file;
  new_type.sym = 0;

  new_type_dcl = P_NewTypeDcl ();
  P_SetTypeDclKey (new_type_dcl, new_type);
  P_SetTypeDclBasicType (new_type_dcl, BT_TYPEDEF_I);
  P_SetTypeDclQualifier (new_type_dcl, full_qualifier);
  P_SetTypeDclAlignment (new_type_dcl, alignment);
  PST_SetTypeDclType (table, new_type_dcl, base_type);

  new_type = PST_ScopeFindTypeDcl (table, type_scope_key, new_type_dcl);
  
  new_type_dcl = P_RemoveTypeDcl (new_type_dcl);

  return (new_type);
}

/*! \brief Returns the line number for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The first line number defined for a Type.
 */
int
PST_GetTypeLineno (SymbolTable table, Type type)
{
  TypeDcl type_dcl;
  int lineno;

  while ((P_ValidKey (type)) && (type_dcl = PST_GetTypeDclEntry (table, type)))
    {
      if ((lineno = P_GetTypeDclLineno (type_dcl)))
	return (lineno);
      else
	type = P_GetTypeDclType (type_dcl);
    }

  return (0);
}

/*! \brief Returns the column number for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The first column number defined for a Type.
 */
int
PST_GetTypeColno (SymbolTable table, Type type)
{
  TypeDcl type_dcl;
  int colno;

  while ((P_ValidKey (type)) && (type_dcl = PST_GetTypeDclEntry (table, type)))
    {
      if ((colno = P_GetTypeDclColno (type_dcl)))
	return (colno);
      else
	type = P_GetTypeDclType (type_dcl);
    }

  return (0);
}

/*! \brief Returns the filename for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to inspect.
 *
 * \return
 *  The first filename defined for a Type.
 *
 * \note The caller must not free the pointer returned by this function.
 */
char *
PST_GetTypeFilename (SymbolTable table, Type type)
{
  TypeDcl type_dcl;
  char *filename;

  while ((P_ValidKey (type)) && (type_dcl = PST_GetTypeDclEntry (table, type)))
    {
      if ((filename = P_GetTypeDclFilename (type_dcl)))
	return (filename);
      else
	type = P_GetTypeDclType (type_dcl);
    }

  return (NULL);
}

/*! \brief Increments the reference count for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type for which to increase the reference count.
 *
 * \return
 *  The new reference count for the Type.
 *
 * Increments the reference count for \a type.  If \a type references other
 * Types, their reference counts are incremented as well.
 *
 * This function does nothing if the symbol table is being removed (the
 * STF_REMOVING flag is set).
 *
 * 5/5/04 REK Disabling this function for now.  The reference count is
 * not used yet, and getting this to work properly for recursive data
 * structures will take a little work.
 */
int
PST_IncTypeRefCount (SymbolTable table, Type type)
{
#if 0
  TypeDcl type_dcl;
  int result = -1;

  /* If the symbol table is being removed, there's no need to maintain
   * the ref count.  Furthermore, doing so would require figuring out
   * dependencies between types and freeing them in the correct order. */
  if (!(P_GetSymbolTableFlags (table) & STF_REMOVING) && \
      (type_dcl = PST_GetTypeDclEntry (table, type)))
    {
      result = P_IncTypeDclRefs (type_dcl);

      if (P_GetTypeDclBasicType (type_dcl) & \
	  (BT_TYPEDEF | BT_ARRAY | BT_FUNC | BT_POINTER))
	PST_IncTypeRefCount (table, P_GetTypeDclType (type_dcl));
    }

  return (result);
#endif
  return (0);
}

/*! \brief Decrements the reference count for a Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type for which to decrease the reference count.
 *
 * \return
 *  The new reference count for the Type.
 *
 * Decrements new reference count for \a type.  If \a type references other
 * types, their reference counts are decremented as well.
 *
 * This function does nothing if the symbol table is being removed (the
 * STF_REMOVING flag is set).
 *
 * 5/5/04 REK Disabling this function for now.  The reference count is
 * not used yet, and getting this to work properly for recursive data
 * structures will take a little work.
 */
int
PST_DecTypeRefCount (SymbolTable table, Type type)
{
#if 0
  TypeDcl type_dcl;
  int result = -1;

  /* If the symbol table is being removed, there's no need to maintain
   * the ref count.  Furthermore, doing so would require figuring out
   * dependencies between types and freeing them in the correct order. */
  if (!(P_GetSymbolTableFlags (table) & STF_REMOVING) && \
      (type_dcl = PST_GetTypeDclEntry (table, type)))
    {
      result = P_DecTypeDclRefs (type_dcl);

      if ((P_GetTypeDclBasicType (type_dcl) & \
	   (BT_TYPEDEF | BT_ARRAY | BT_FUNC | BT_POINTER)) && \
	  (P_ValidKey (P_GetTypeDclType (type_dcl))))
	PST_DecTypeRefCount (table, P_GetTypeDclType (type_dcl));
    }

  return (result);
#endif
  return (0);
}

/*! \brief Returns the a signed version of a Type and updates the Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  a pointer the Type to change.
 *
 * \return
 *  The key of a signed version of \a type.  This key is also written
 *  to \a type.
 *
 * \a type must be the an integer Type.  if \a type is already signed,
 * it is returned.  If not, the a signed version of \a type is returned.
 * Updates \a type to be this new Type.
 *
 * \note This function should not be used on the TypeDcl.type field.
 *       The TypeDcl.type field must not be changed.  Use
 *       PST_FindTypeSetSigned() instead.
 *
 * \sa PST_FindTypeSetSigned()
 */
Type
PST_SetTypeSigned (SymbolTable table, Type *type)
{
  Type new_type;

  new_type = PST_FindTypeSetSigned (table, *type);

  if (P_ValidKey (*type))
    PST_DecTypeRefCount (table, *type);

  if (P_ValidKey (new_type))
    PST_IncTypeRefCount (table, new_type);

  *type = new_type;

  return (new_type);
}

/*! \brief Returns the signed version of an integer Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to adjust.
 *
 * \return
 *  A signed version of \a type.
 *
 * \a type must be an integer Type.  If \a type is already signed,
 * it is returned.  If not, a signed version of \a type is returned.
 *
 * \sa PST_SetTypeSigned()
 */
Type
PST_FindTypeSetSigned (SymbolTable table, Type type)
{
  TypeDcl type_dcl = PST_GetTypeDclEntry (table, type);
  Type new_type;
  _BasicType bt;

  if (!PST_IsIntegralType (table, type))
    P_punt ("struct_symtab.c:PST_FindTypeSetSigned:%d type (%d, %d) is not an "
	    "integer type", __LINE__ - 1, type.file, type.sym);

  if (PST_IsSignedType (table, type))
    {
      new_type = type;
    }
  else
    {
      if (P_IsIntegralTypeDcl (type_dcl))
	{
	  bt = P_GetTypeDclBasicType (type_dcl);

	  new_type = PST_FindBasicType (table, bt & ~BT_UNSIGNED);
	}
      else /* type_dcl is a typedef that references an unsigned type. */
	{
	  TypeDcl new_type_dcl;
	  Type base_type;
	  Key type_scope_key;
	  _TypeQual full_qualifier = PST_GetTypeQualifier (table, type);
	  
	  bt = PST_GetTypeBasicType (table, type);
	  
	  base_type = PST_FindBasicType (table, bt & ~BT_UNSIGNED);
	  
	  type_scope_key = PST_GetScopeFromEntryKey (table, type);
	  
	  new_type.file = type_scope_key.file;
	  new_type.sym = 0;
	  
	  new_type_dcl = P_NewTypeDcl ();
	  P_SetTypeDclKey (new_type_dcl, new_type);
	  P_SetTypeDclBasicType (new_type_dcl, BT_TYPEDEF_I);
	  P_SetTypeDclQualifier (new_type_dcl, full_qualifier);
	  /* Copy the alignment if it is explicitly set. */
	  if (full_qualifier & TY_EXP_ALIGN)
	    P_SetTypeDclAlignment (new_type_dcl,
				   PST_GetTypeAlignment (table, type));
	  PST_SetTypeDclType (table, new_type_dcl, base_type);
	  
	  new_type = PST_ScopeFindTypeDcl (table, type_scope_key,
					   new_type_dcl);
	  
	  new_type_dcl = P_RemoveTypeDcl (new_type_dcl);
	}
    }

  return (new_type);
}

/*! \brief Returns an unsigned Type and updates the Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  a pointer the Type to adjust.
 *
 * \return
 *  The an unsigned version of \a type.  This is also written to \a type.
 *
 * \a type must be an integer Type.  If \a type is already unsigned,
 * it is returned.  If not, the an unsigned version of \a type is returned.
 * Updates \a type to be this new Type.
 *
 * \note This function should not be used on the TypeDcl.type field.
 *       The TypeDcl.type field must not be changed.  Use
 *       PST_FindTypeSetUnsigned() instead.
 *
 * \sa PST_FindTypeSetUnsigned()
 */
Type
PST_SetTypeUnsigned (SymbolTable table, Type *type)
{
  Type new_type;

  new_type = PST_FindTypeSetUnsigned (table, *type);

  if (P_ValidKey (*type))
    PST_DecTypeRefCount (table, *type);

  if (P_ValidKey (new_type))
    PST_IncTypeRefCount (table, new_type);

  *type = new_type;

  return (new_type);
}

/*! \brief Returns an unsigned version of an integer Type.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to adjust.
 *
 * \return
 *  An unsigned version of \a type.
 *
 * \a type must be the an integer Type.  If \a type is already unsigned,
 * it is returned.  If not, the an unsigned version of \a type is returned.
 *
 * \sa PST_SetTypeUnsigned()
 */
Type
PST_FindTypeSetUnsigned (SymbolTable table, Type type)
{
  TypeDcl type_dcl = PST_GetTypeDclEntry (table, type);
  Type new_type;
  _BasicType bt;

  if (!PST_IsIntegralType (table, type))
    P_punt ("struct_symtab.c:PST_FindTypeSetUnsigned:%d type (%d, %d) is not "
	    "an integer type", __LINE__ - 1, type.file, type.sym);

  if (PST_IsUnsignedType (table, type))
    {
      new_type = type;
    }
  else
    {
      if (P_IsIntegralTypeDcl (type_dcl))
	{
	  bt = P_GetTypeDclBasicType (type_dcl);

	  /* Find a signed version of type_key. */
	  new_type = PST_FindBasicType (table, bt | BT_UNSIGNED);
	}
      else /* type_dcl is a typedef that references a signed type. */
	{
	  TypeDcl new_type_dcl;
	  Type base_type;
	  Key type_scope_key;
	  _TypeQual full_qualifier = PST_GetTypeQualifier (table, type);
	  
	  bt = PST_GetTypeBasicType (table, type);

	  base_type = PST_FindBasicType (table, bt | BT_UNSIGNED);

	  type_scope_key = PST_GetScopeFromEntryKey (table, type);

	  new_type.file = type_scope_key.file;
	  new_type.sym = 0;

	  new_type_dcl = P_NewTypeDcl ();
	  P_SetTypeDclKey (new_type_dcl, new_type);
	  P_SetTypeDclBasicType (new_type_dcl, BT_TYPEDEF_I);
	  P_SetTypeDclQualifier (new_type_dcl, full_qualifier);
	  /* Copy the alignment if it is explicitly set. */
	  if (full_qualifier & TY_EXP_ALIGN)
	    P_SetTypeDclAlignment (new_type_dcl,
				   PST_GetTypeAlignment (table, type));
	  PST_SetTypeDclType (table, new_type_dcl, base_type);

	  new_type = PST_ScopeFindTypeDcl (table, type_scope_key,
					   new_type_dcl);

	  new_type_dcl = P_RemoveTypeDcl (new_type_dcl);
	}
    }

  return (new_type);
}

/*! \brief Returns the Type resulting from dereferencing all pointers.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the initial Type.
 *
 * \return
 *  The Type resulting from dereferencing \a type.
 *
 * Dereferences \a type until it finds a non-pointer and returns that type.
 */
Type
PST_DereferencePointers (SymbolTable table, Type type)
{
  while (PST_GetTypeBasicType (table, type) & BT_POINTER)
    type = PST_DereferenceType (table, type);

  return (type);
}

/*! \brief Returns the base Type (excluding typedef, pointer, func, array).
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the initial Type.
 *
 * \return
 *  The base Type.
 *
 * Iteritively inspects the typedef's type and returns the first non-typedef
 * type.
 *
 * If \a type_key is invalid, returns an invalid key.
 *
 * \sa PST_ReduceTypedefs (), PST_GetTypeDclQualifier(),
 * PST_ReduceImplicitTypedefs(), PST_IsBaseType() */
Type
PST_GetBaseType (SymbolTable table, Type type)
{
  while (P_ValidKey (type) && 
	 !PST_IsBaseType (table, type))
    type = PST_GetTypeType (table, type);

  return (type);
}

/*! \brief Returns the base type of a typedef.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to reduce.
 *
 * \return
 *  The key of the typedef's base type.
 *
 * If \a type is not a typedef, returns \a type.  Otherwise,
 * recursively inspects the typedef's type and returns the first non-typedef
 * type.
 *
 * If \a type_key is invalid, returns an invalid key.
 *
 * \sa PST_GetTypeDclQualifier(), PST_ReduceExplicitTypedefs(),
 * PST_ReduceImplicitTypedefs() */
Type
PST_ReduceTypedefs (SymbolTable table, Type type)
{
  TypeDcl type_dcl = NULL;

  /* If this function is called on a struct or union, just return the
   * struct or union. */
  if (!P_ValidKey (type) || \
      (P_GetSymTabEntryType (PST_GetSymTabEntry (table, type)) & \
       (ET_STRUCT | ET_UNION)))
    return (type);

  while (P_ValidKey (type) && \
	 (type_dcl = PST_GetTypeDclEntry (table, type)) && \
	 (P_GetTypeDclBasicType (type_dcl) & BT_TYPEDEF))
    type = P_GetTypeDclType (type_dcl);
  
  if (type_dcl == NULL)
    return (Invalid_Key);
  else
    return (type);
}

/*! \brief Returns the base type of a set of explicit typedefs.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to reduce.
 *
 * \return
 *  The key of the explicit typedef's base type.
 *
 * If \a type is not an explicit typedef, returns \a type.  Otherwise,
 * recursively inspects the typedef's type and returns the first
 * non-explicit typedef type.
 *
 * \sa PST_GetTypeDclQualifier(), PST_ReduceTypedefs(),
 * PST_ReduceImplicitTypedefs() */
Type
PST_ReduceExplicitTypedefs (SymbolTable table, Type type)
{
  TypeDcl type_dcl;

  /* If this function is called on a struct or union, just return the
   * struct or union. */
  if (!P_ValidKey (type) || \
      (P_GetSymTabEntryType (PST_GetSymTabEntry (table, type)) & \
       (ET_STRUCT | ET_UNION)))
    return (type);

  while ((type_dcl = PST_GetTypeDclEntry (table, type)) && \
	 (P_GetTypeDclBasicType (type_dcl) & BT_TYPEDEF_E))
    type = P_GetTypeDclType (type_dcl);

  if (type_dcl == NULL)
    return (Invalid_Key);
  else
    return (type);
}

/*! \brief Returns the base type of a set of implicit typedefs.
 *
 * \param table
 *  the symbol table.
 * \param type
 *  the Type to reduce.
 *
 * \return
 *  The key of the implicit typedef's base type.
 *
 * If \a type is not an implicit typedef, returns \a type.  Otherwise,
 * recursively inspects the typedef's type and returns the first
 * non-implicit typedef type.
 *
 * \sa PST_GetTypeDclQualifier(), PST_ReduceTypedefs(),
 * PST_ReduceExplicitTypedefs() */
Type
PST_ReduceImplicitTypedefs (SymbolTable table, Type type)
{
  TypeDcl type_dcl;

  /* If this function is called on a struct or union, just return the
   * struct or union. */
  if (!P_ValidKey (type) || \
      (P_GetSymTabEntryType (PST_GetSymTabEntry (table, type)) & \
       (ET_STRUCT | ET_UNION)))
    return (type);

  while ((type_dcl = PST_GetTypeDclEntry (table, type)) && \
	 (P_GetTypeDclBasicType (type_dcl) & BT_TYPEDEF_I))
    type = P_GetTypeDclType (type_dcl);

  if (type_dcl == NULL)
    return (Invalid_Key);
  else
    return (type);
}

/*! \brief Sets the type for a FuncDcl.
 *
 * \param table
 *  the symbol table.
 * \param func_dcl
 *  the FuncDcl for which to set the type.
 * \param type
 *  the Type to set.
 *
 * \return
 *  Function returns \a type.
 *
 * This function sets the \a func_dcl's type to \a type and increments
 * \a type's reference count.
 *
 * \sa PST_SetParamKey(), PST_SetTypeDclType(), PST_SetVarDclType(),
 * PST_SetFieldType(), PST_SetExprType(), PST_SetExprVType() */
Type
PST_SetFuncDclType (SymbolTable table, FuncDcl func_dcl, Type type)
{
  Type old_type;

  /* If func_dcl's type is already set, decrement the reference count
   * for that type. */
  old_type = P_GetFuncDclType (func_dcl);

  if (P_ValidKey (old_type))
    PST_DecTypeRefCount (table, old_type);

  if (P_ValidKey (type))
    PST_IncTypeRefCount (table, type);

  P_SetFuncDclType (func_dcl, type);

  return (type);
}

/*! \brief Sets the name for a FuncDcl.
 *
 * \param table
 *  the symbol table.
 * \param func_dcl
 *  the FuncDcl.
 * \param name
 *  the FuncDcl's new name.
 *
 * \return
 *  The FuncDcl's new name (\a name).
 *
 * Sets the new name on the FuncDcl and on the FuncDcl's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetFuncDclName (SymbolTable table, FuncDcl func_dcl, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table, P_GetFuncDclKey (func_dcl));

  P_SetFuncDclName (func_dcl, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}

/*! \brief Sets the key for a Param.
 *
 * \param table
 *  the symbol table.
 * \param param
 *  the Param for which to set the key.
 * \param type
 *  the new key for the Param.
 *
 * \return
 *  Function returns \a type.
 *
 * This function sets the \a param's key to \a type and increments
 * \a type's reference count.
 *
 * \sa PST_SetParamKey(), PST_SetTypeDclType(), PST_SetVarDclType(),
 * PST_SetFieldType(), PST_SetExprType(), PST_SetExprVType() */
Type
PST_SetParamKey (SymbolTable table, Param param, Type type)
{
  Type old_type;

  /* If param's key is already set, decrement the reference ocunt
   * for that type. */
  old_type = P_GetParamKey (param);

  if (P_ValidKey (old_type))
    PST_DecTypeRefCount (table, old_type);

  if (P_ValidKey (type))
    PST_IncTypeRefCount (table, type);

  P_SetParamKey (param, type);

  return (type);
}

/*! \brief Sets the type for a TypeDcl.
 *
 * \param table
 *  the symbol table.
 * \param type_dcl
 *  the TypeDcl for which to set the type.
 * \param type
 *  the type to set.
 *
 * \return
 *  Function returns \a type.
 *
 * This function sets the \a type_dcl's type to \a type and increments
 * \a type's reference count.
 *
 * \sa PST_SetFuncDclType(), PST_SetParamKey(), PST_SetVarDclType(),
 * PST_SetFieldType(), PST_SetExprType(), PST_SetExprVType() */
Type
PST_SetTypeDclType (SymbolTable table, TypeDcl type_dcl, Type type)
{
  Key old_type;

  /* If type_dcl's type is already set, decrement the reference count
   * for that type. */
  old_type = P_GetTypeDclType (type_dcl);

  if (P_ValidKey (old_type))
    PST_DecTypeRefCount (table, old_type);

  if (P_ValidKey (type))
    PST_IncTypeRefCount (table, type);

  P_SetTypeDclType (type_dcl, type);

  return (type);
}

/*! \brief Sets the name for a TypeDcl.
 *
 * \param table
 *  the symbol table.
 * \param type_dcl
 *  the TypeDcl.
 * \param name
 *  the TypeDcl's new name.
 *
 * \return
 *  The TypeDcl's new name (\a name).
 *
 * Sets the new name on the TypeDcl and on the TypeDcl's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetTypeDclName (SymbolTable table, TypeDcl type_dcl, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table, P_GetTypeDclKey (type_dcl));

  P_SetTypeDclName (type_dcl, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}

/*! \brief Sets the type for a VarDcl.
 *
 * \param table
 *  the symbol table.
 * \param var_dcl
 *  the VarDcl for which to set the type.
 * \param type
 *  the type to set.
 *
 * \return
 *  Function returns \a type.
 *
 * This function sets the \a var_dcl's type to \a type and increments
 * \a type's reference count.
 *
 * \sa PST_SetFuncDclType(), PST_SetParamKey(), PST_SetTypeDclType(),
 * PST_SetFieldType(), PST_SetExprType(), PST_SetExprVType() */
Type
PST_SetVarDclType (SymbolTable table, VarDcl var_dcl, Type type)
{
  Type old_type;

  /* If var_dcl's type is already set, decrement the reference count
   * for that type. */
  old_type = P_GetVarDclType (var_dcl);

  if (P_ValidKey (old_type))
    PST_DecTypeRefCount (table, old_type);

  if (P_ValidKey (type))
    PST_IncTypeRefCount (table, type);

  P_SetVarDclType (var_dcl, type);

  return (type);
}

/*! \brief Sets the name for a VarDcl.
 *
 * \param table
 *  the symbol table.
 * \param var_dcl
 *  the VarDcl.
 * \param name
 *  the VarDcl's new name.
 *
 * \return
 *  The VarDcl's new name (\a name).
 *
 * Sets the new name on the VarDcl and on the VarDcl's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetVarDclName (SymbolTable table, VarDcl var_dcl, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table, P_GetVarDclKey (var_dcl));

  P_SetVarDclName (var_dcl, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}

/*! \brief Sets the name for a StructDcl.
 *
 * \param table
 *  the symbol table.
 * \param struct_dcl
 *  the StructDcl.
 * \param name
 *  the StructDcl's new name.
 *
 * \return
 *  The StructDcl's new name (\a name).
 *
 * Sets the new name on the StructDcl and on the StructDcl's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetStructDclName (SymbolTable table, StructDcl struct_dcl, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table,
					  P_GetStructDclKey (struct_dcl));

  P_SetStructDclName (struct_dcl, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}

/*! \brief Sets the name for a UnionDcl.
 *
 * \param table
 *  the symbol table.
 * \param union_dcl
 *  the UnionDcl.
 * \param name
 *  the UnionDcl's new name.
 *
 * \return
 *  The UnionDcl's new name (\a name).
 *
 * Sets the new name on the UnionDcl and on the UnionDcl's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetUnionDclName (SymbolTable table, UnionDcl union_dcl, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table, P_GetUnionDclKey (union_dcl));

  P_SetUnionDclName (union_dcl, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}


/*! \brief Sets the name for an EnumDcl.
 *
 * \param table
 *  the symbol table.
 * \param enum_dcl
 *  the EnumDcl.
 * \param name
 *  the EnumDcl's new name.
 *
 * \return
 *  The EnumDcl's new name (\a name).
 *
 * Sets the new name on the EnumDcl and on the EnumDcl's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetEnumDclName (SymbolTable table, EnumDcl enum_dcl, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table, P_GetEnumDclKey (enum_dcl));

  P_SetEnumDclName (enum_dcl, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}

/*! \brief Sets the type for a Field.
 *
 * \param table
 *  the symbol table.
 * \param field
 *  the Field for which to set the type.
 * \param type
 *  the type to set.
 *
 * \return
 *  Function returns \a type.
 *
 * This function sets the \a field's type to \a type and increments
 * \a type's reference count.
 *
 * \sa PST_SetFuncDclType(), PST_SetParamKey(), PST_SetTypeDclType(),
 * PST_SetVarDclType(), PST_SetExprType(), PST_SetExprVType() */
Type
PST_SetFieldType (SymbolTable table, Field field, Type type)
{
  Type old_type;

  /* If field's type is already set, decrement the reference count
   * for that type. */
  old_type = P_GetFieldType (field);

  if (P_ValidKey (old_type))
    PST_DecTypeRefCount (table, old_type);

  if (P_ValidKey (type))
    PST_IncTypeRefCount (table, type);

  P_SetFieldType (field, type);

  return (type);
}

/*! \brief Sets the name for a Field.
 *
 * \param table
 *  the symbol table.
 * \param field
 *  the Field.
 * \param name
 *  the Field's new name.
 *
 * \return
 *  The Field's new name (\a name).
 *
 * Sets the new name on the Field and on the Field's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetFieldName (SymbolTable table, Field field, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table, P_GetFieldKey (field));

  P_SetFieldName (field, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}

/*! \brief Sets the name for an EnumField.
 *
 * \param table
 *  the symbol table.
 * \param enum_field
 *  the EnumField.
 * \param name
 *  the EnumField's new name.
 *
 * \return
 *  The EnumField's new name (\a name).
 *
 * Sets the new name on the EnumField and on the EnumField's SymTabEntry.
 *
 * \note \a name is not copied, so the caller must not free the string
 *       passed in.
 */
char *
PST_SetEnumFieldName (SymbolTable table, EnumField enum_field, char *name)
{
  SymTabEntry entry = PST_GetSymTabEntry (table,
					  P_GetEnumFieldKey (enum_field));

  P_SetEnumFieldName (enum_field, name);
  P_SetSymTabEntryName (entry, strdup (name));

  return (name);
}

/*! \brief Sets the type for an Expr.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr for which to set the type.
 * \param type
 *  the type to set.
 *
 * \return
 *  Function returns \a type.
 *
 * This function sets the \a Expr's type to \a type and increments
 * \a type's reference count.
 *
 * \sa PST_SetFuncDclType(), PST_SetParamKey(), PST_SetTypeDclType(),
 * PST_SetVarDclType(), PST_SetFieldType(), PST_SetExprVType() */
Type
PST_SetExprType (SymbolTable table, Expr expr, Type type)
{
  Type old_type;

  /* If expr's type is already set, decrement the reference count
   * for that type. */
  old_type = P_GetExprType (expr);

  if (P_ValidKey (old_type))
    PST_DecTypeRefCount (table, old_type);

  if (P_ValidKey (type))
    PST_IncTypeRefCount (table, type);

  P_SetExprType (expr, type);

  return (type);
}

/*! \brief Sets value.type for an Expr.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the Expr for which to set value.type.
 * \param type
 *  the type to set.
 *
 * \return
 *  Function returns \a type.
 *
 * This function sets the \a Expr's value.type to \a type and increments
 * \a type's reference count.
 *
 * \sa PST_SetFuncDclType(), PST_SetParamKey(), PST_SetTypeDclType(),
 * PST_SetVarDclType(), PST_SetFieldType(), PST_SetExprType() */
Type
PST_SetExprVType (SymbolTable table, Expr expr, Type type)
{
  Type old_type;

  /* If expr's type is already set, decrement the reference count
   * for that type. */
  old_type = P_GetExprVType (expr);

  if (P_ValidKey (old_type))
    PST_DecTypeRefCount (table, old_type);

  if (P_ValidKey (type))
    PST_IncTypeRefCount (table, type);

  P_SetExprVType (expr, type);

  return (type);
}

/*! \brief Removes a Dcl.
 *
 * \param table
 *  the symbol table.
 * \param d
 *  the Dcl to remove.
 *
 * \return
 *  A null Dcl pointer.
 *
 * Adjusts the reference count of a Dcl's types and removes the Dcl.
 */
Dcl
PST_RemoveDcl (SymbolTable table, Dcl d)
{
  if (d)
    {
      switch (P_GetDclType (d))
	{
	case TT_FUNC:
	  P_SetDclFuncDcl (d, PST_RemoveFuncDcl (table, P_GetDclFuncDcl (d)));
	  break;
	case TT_TYPE:
	  P_SetDclTypeDcl (d, PST_RemoveTypeDcl (table, P_GetDclTypeDcl (d)));
	  break;
	case TT_VAR:
	  P_SetDclVarDcl (d, PST_RemoveVarDcl (table, P_GetDclVarDcl (d)));
	  break;
	case TT_STRUCT:
	  P_SetDclStructDcl (d, PST_RemoveStructDcl (table,
						     P_GetDclStructDcl (d)));
	  break;
	case TT_UNION:
	  P_SetDclUnionDcl (d, PST_RemoveUnionDcl (table,
						   P_GetDclUnionDcl (d)));
	  break;
	case TT_ASM:
	  P_SetDclAsmDcl (d, PST_RemoveAsmDcl (table, P_GetDclAsmDcl (d)));
	  break;
	default:
	  break;
	}

      d = P_RemoveDcl (d);
    }

  return (d);
}

/*! \brief Removes a FuncDcl.
 *
 * \param table
 *  the symbol table.
 * \param f
 *  the FuncDcl to remove.
 *
 * \return
 *  A null FuncDcl pointer.
 *
 * Adjusts the reference count of a FuncDcl's type and removes the FuncDcl.
 */
FuncDcl
PST_RemoveFuncDcl (SymbolTable table, FuncDcl f)
{
  Key k;
  SymTabEntry entry;

  if (f)
    {
      k = f->key;

      PST_SetFuncDclType (table, f, Invalid_Key);

      f->param = PST_RemoveVarList (table, f->param);
      f->stmt = PST_RemoveStmt (table, f->stmt);

      f = P_RemoveFuncDcl (f);

      if ((entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryFuncDcl (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (f);
}

/*! \brief Removes a TypeDcl.
 *
 * \param table
 *  the symbol table.
 * \param t
 *  the TypeDcl to remove.
 *
 * \return
 *  A null TypeDcl pointer.
 *
 * Adjusts the reference count of a TypeDcl's type and removes the TypeDcl.
 */
TypeDcl
PST_RemoveTypeDcl (SymbolTable table, TypeDcl t)
{
  Param p;
  Key k;
  SymTabEntry entry;

  if (t)
    {
      k = t->key;

      PST_SetTypeDclType (table, t, Invalid_Key);

      if (P_GetTypeDclBasicType (t) == BT_FUNC)
	for (p = P_GetTypeDclParam (t); p; p = P_GetParamNext (p))
	  PST_SetParamKey (table, p, Invalid_Key);

      t = P_RemoveTypeDcl (t);

      if ((entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryTypeDcl (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (t);
}

/*! \brief Removes a TypeList.
 *
 * \param table
 *  the symbol table.
 * \param t
 *  the TypeList to remove.
 *
 * \return
 *  A null TypeList pointer.
 */
TypeList
PST_RemoveTypeList (SymbolTable table, TypeList t)
{
  TypeDcl td;

  List_start ((List)t);
  while ((td = (TypeDcl)List_next ((List)t)))
    {
      PST_RemoveTypeDcl (table, td);
      t = (TypeList)List_remove ((List)t, td);
    }

  return (t);
}

/*! \brief Removes a VarList.
 *
 * \param table
 *  the symbol table.
 * \param v
 *  the VarList to remove.
 *
 * \return
 *  A null VarList pointer.
 */
VarList
PST_RemoveVarList (SymbolTable table, VarList v)
{
  VarDcl vd;

  List_start ((List)v);
  while ((vd = (VarDcl)List_next ((List)v)))
    {
      PST_RemoveVarDcl (table, vd);
      v = (VarList)List_remove ((List)v, vd);
    }

  return (v);
}

/*! \brief Removes a VarDcl.
 *
 * \param table
 *  the symbol table.
 * \param v
 *  the VarDcl to remove.
 *
 * \return
 *  A null VarDcl pointer.
 *
 * Adjusts the reference count of a VarDcl's type and removes the VarDcl.
 */
VarDcl
PST_RemoveVarDcl (SymbolTable table, VarDcl v)
{
  Key k;
  SymTabEntry entry;

  if (v)
    {
      k = v->key;

      PST_SetVarDclType (table, v, Invalid_Key);

      v->init = PST_RemoveInit (table, v->init);

      v = P_RemoveVarDcl (v);

      if ((entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryVarDcl (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (v);;
}

/*! \brief Removes an Init.
 *
 * \param table
 *  the symbol table.
 * \param i
 *  the Init to remove.
 *
 * \return
 *  A null Init pointer.
 *
 * Adjusts the reference count of an Init's types and removes the Init.
 */
Init
PST_RemoveInit (SymbolTable table, Init i)
{
  if (i)
    {
      i->expr = PST_RemoveExpr (table, i->expr);
      i->set = PST_RemoveInit (table, i->set);
      i->next = PST_RemoveInit (table, i->next);

      i = P_RemoveInit (i);
    }

  return (i);
}

/*! \brief Removes a StructDcl.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the StructDcl to remove.
 *
 * \return
 *  A null StructDcl pointer.
 *
 * Adjusts the reference count of a StructDcl's types and removes the
 * StructDcl.
 */
StructDcl
PST_RemoveStructDcl (SymbolTable table, StructDcl s)
{
  Key k;
  SymTabEntry entry;

  if (s)
    {
      k = s->key;

      s->fields = PST_RemoveField (table, s->fields);
	
      s = P_RemoveStructDcl (s);

      if ((entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryStructDcl (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (s);
}

/*! \brief Removes a UnionDcl.
 *
 * \param table
 *  the symbol table.
 * \param u
 *  the UnionDcl to remove.
 *
 * \return
 *  A null UnionDcl pointer.
 *
 * Adjusts the reference count of a UnionDcl's types and removes the
 * UnionDcl.
 */
UnionDcl
PST_RemoveUnionDcl (SymbolTable table, UnionDcl u)
{
  Key k;
  SymTabEntry entry;

  if (u)
    {
      k = u->key;

      u->fields = PST_RemoveField (table, u->fields);

      u = P_RemoveUnionDcl (u);

      if ((entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryUnionDcl (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (u);
}

/*! \brief Removes a Field.
 *
 * \param table
 *  the symbol table.
 * \param f
 *  the Field to remove.
 *
 * \return
 *  A null Field pointer.
 *
 * Adjusts the reference count of a Field's type and removes the Field.
 */
Field
PST_RemoveField (SymbolTable table, Field f)
{
  Key k;
  SymTabEntry entry;

  if (f)
    {
      k = f->key;

      PST_SetFieldType (table, f, Invalid_Key);

      f->next = PST_RemoveField (table, f->next);

      f = P_RemoveField (f);

      if ((entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryField (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (f);
}

/*! \brief Removes a Stmt.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the Stmt to remove.
 *
 * \return
 *  A null Stmt pointer.
 *
 * Adjusts the reference count of a Stmt's types and removes the Stmt.
 * If the statement defines a scope, the scope and any enclosed symbols
 * are removed as well.
 */
Stmt
PST_RemoveStmtNode (SymbolTable table, Stmt s)
{
  Key k;
  SymTabEntry entry;

  if (s)
    {
      k = s->key;

      s->labels = PST_RemoveLabel (table, s->labels);
      
      switch (s->type)
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	  break;
	case ST_RETURN:
	  s->stmtstruct.ret = PST_RemoveExpr (table, s->stmtstruct.ret);
	  break;
	case ST_COMPOUND:
	  s->stmtstruct.compound = \
	    PST_RemoveCompound (table, s->stmtstruct.compound);
	  break;
	case ST_IF:
	  s->stmtstruct.ifstmt = \
	    PST_RemoveIfStmt (table, s->stmtstruct.ifstmt);
	  break;
	case ST_SWITCH:
	  s->stmtstruct.switchstmt = \
	    PST_RemoveSwitchStmt (table, s->stmtstruct.switchstmt);
	  break;
	case ST_PSTMT:
	  s->stmtstruct.pstmt = PST_RemovePstmt (table, s->stmtstruct.pstmt);
	  break;
	case ST_MUTEX:
	  s->stmtstruct.mutex = PST_RemoveMutex (table, s->stmtstruct.mutex);
	  break;
	case ST_COBEGIN:
	  s->stmtstruct.cobegin = \
	    PST_RemoveCobegin (table, s->stmtstruct.cobegin);
	  break;
	case ST_PARLOOP:
	  s->stmtstruct.parloop = \
	    PST_RemoveParLoop (table, s->stmtstruct.parloop);
	  break;
	case ST_SERLOOP:
	  s->stmtstruct.serloop = \
	    PST_RemoveSerLoop (table, s->stmtstruct.serloop);
	  break;
	case ST_EXPR:
	  s->stmtstruct.expr = PST_RemoveExpr (table, s->stmtstruct.expr);
	  break;
	case ST_BODY:
	  s->stmtstruct.bodystmt = \
	    PST_RemoveBodyStmt (table, s->stmtstruct.bodystmt);
	  break;
	case ST_EPILOGUE:
	  s->stmtstruct.epiloguestmt = \
	    PST_RemoveEpilogueStmt (table, s->stmtstruct.epiloguestmt);
	  break;
	case ST_ASM:
	  s->stmtstruct.asmstmt = \
	    PST_RemoveAsmStmt (table, s->stmtstruct.asmstmt);
	  break;
	default:
	  P_punt ("struct_symtab.c:PST_RemoveStmt:%d unknown stmt type %d",
		  __LINE__ - 1, s->type);
	}

      s = P_RemoveStmtNode (s);

      if (P_ValidKey (k) && (entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryStmt (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (s);
}

/*! \brief Removes a Stmt list.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the Stmt list to free.
 *
 * \return
 *  A null Stmt pointer.
 *
 * Adjusts the reference count of a Stmt's types and removes the Stmt.
 * If the statement defines a scope, the scope and any enclosed symbols
 * are removed as well.
 */
Stmt
PST_RemoveStmt (SymbolTable table, Stmt s)
{
  if (s)
    {
      s->lex_next = PST_RemoveStmt (table, s->lex_next);

      s = PST_RemoveStmtNode (table, s);
    }

  return (s);
}

/*! \brief Removes a Label list.
 *
 * \param table
 *  the symbol table.
 * \param l
 *  the Label to remove.
 *
 * \return
 *  A null Label pointer.
 *
 * Adjusts the reference count of a Label's types and removes the Label.
 */
Label
PST_RemoveLabel (SymbolTable table, Label l)
{
  Key k;
  SymTabEntry entry;

  if (l)
    {
      k = l->key;

      l->next = PST_RemoveLabel (table, l->next);
      
      l->data.expression = PST_RemoveExpr (table, l->data.expression);

      l = P_RemoveLabel (l);

      if (P_ValidKey (k) && (entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryLabel (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (l);
}

/*! \brief Removes a Compound.
 *
 * \param table
 *  the symbol table.
 * \param c
 *  the Compound to free.
 *
 * \return
 *  A null Compound pointer.
 *
 * Removes a Compound and all symbol table entries that reference it or
 * child structures.
 */
Compound
PST_RemoveCompound (SymbolTable table, Compound c)
{
  if (c)
    {
      c->type_list = PST_RemoveTypeList (table, c->type_list);
      c->var_list = PST_RemoveVarList (table, c->var_list);
      c->stmt_list = PST_RemoveStmt (table, c->stmt_list);

      c = P_RemoveCompound (c);
    }

  return (c);
}

/*! \brief Removes an IfStmt.
 *
 * \param table
 *  the symbol table.
 * \param i
 *  the IfStmt to free.
 *
 * \return
 *  A null IfStmt pointer.
 *
 * Removes an IfStmt and all symbol table entries that reference it or
 * child structures.
 */
IfStmt
PST_RemoveIfStmt (SymbolTable table, IfStmt i)
{
  if (i)
    {
      i->cond_expr = PST_RemoveExpr (table, i->cond_expr);
      i->then_block = PST_RemoveStmt (table, i->then_block);
      i->else_block = PST_RemoveStmt (table, i->else_block);

      i = P_RemoveIfStmt (i);
    }

  return (i);
}

/*! \brief Removes a SwitchStmt.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the SwitchStmt to free.
 *
 * \return
 *  A null SwitchStmt pointer.
 *
 * Removes a SwitchStmt and all symbol table entries that reference it or
 * child structures.
 */
SwitchStmt
PST_RemoveSwitchStmt (SymbolTable table, SwitchStmt s)
{
  if (s)
    {
      s->expression = PST_RemoveExpr (table, s->expression);
      s->switchbody = PST_RemoveStmt (table, s->switchbody);

      s = P_RemoveSwitchStmt (s);
    }

  return (s);
}

/*! \brief Removes a Pstmt.
 *
 * \param table
 *  the symbol table.
 * \param p
 *  the Pstmt to free.
 *
 * \return
 *  A null Pstmt pointer.
 *
 * Removes a Pstmt and all symbol table entries that reference it or child
 * structures.
 */
Pstmt
PST_RemovePstmt (SymbolTable table, Pstmt p)
{
  if (p)
    {
      p->stmt = PST_RemoveStmt (table, p->stmt);

      p = P_RemovePstmt (p);
    }

  return (p);
}

/*! \brief Removes a Mutex.
 *
 * \param table
 *  the symbol table.
 * \param m
 *  the Mutex to free.
 *
 * \return
 *  A null Mutex pointer.
 *
 * Removes a Mutex and all symbol table entries that reference it or
 * child structures.
 */
Mutex
PST_RemoveMutex (SymbolTable table, Mutex m)
{
  if (m)
    {
      m->expression = PST_RemoveExpr (table, m->expression);
      m->statement = PST_RemoveStmt (table, m->statement);

      m = P_RemoveMutex (m);
    }

  return (m);
}

/*! \brief Removes a Cobegin.
 *
 * \param table
 *  the symbol table.
 * \param c
 *  the Cobegin to free.
 *
 * \return
 *  A null Cobegin pointer.
 *
 * Removes a Cobegin and all symbol table entries that reference it or
 * child structures.
 */
Cobegin
PST_RemoveCobegin (SymbolTable table, Cobegin c)
{
  if (c)
    {
      c->statements = PST_RemoveStmt (table, c->statements);
      
      c = P_RemoveCobegin (c);
    }

  return (c);
}

/*! \brief Removes a BodyStmt.
 *
 * \param table
 *  the symbol table.
 * \param b
 *  the BodyStmt to free.
 *
 * \return
 *  A null BodyStmt pointer.
 *
 * Removes a BodyStmt and all symbol table entries that reference it or
 * child structures.
 */
BodyStmt
PST_RemoveBodyStmt (SymbolTable table, BodyStmt b)
{
  if (b)
    {
      b->statement = PST_RemoveStmt (table, b->statement);

      b = P_RemoveBodyStmt (b);
    }

  return (b);
}

/*! \brief Removes an EpilogueStmt.
 *
 * \param table
 *  the symbol table.
 * \param e
 *  the EpilogueStmt to free.
 *
 * \return
 *  A null EpilogueStmt pointer.
 *
 * Removes an EpilogueStmt and all symbol table entries that reference it or
 * child structures.
 */
EpilogueStmt
PST_RemoveEpilogueStmt (SymbolTable table, EpilogueStmt e)
{
  if (e)
    {
      e->statement = PST_RemoveStmt (table, e->statement);

      e = P_RemoveEpilogueStmt (e);
    }

  return (e);
}

/*! \brief Removes a ParLoop.
 *
 * \param table
 *  the symbol table.
 * \param p
 *  the ParLoop to free.
 *
 * \return
 *  A null ParLoop pointer.
 *
 * Removes a ParLoop and all symbol table entries that reference it or
 * child structures.
 */
ParLoop
PST_RemoveParLoop (SymbolTable table, ParLoop p)
{
  if (p)
    {
      p->pstmt = PST_RemovePstmt (table, p->pstmt);
      p->iteration_var = PST_RemoveExpr (table, p->iteration_var);
      p->init_value = PST_RemoveExpr (table, p->init_value);
      p->final_value = PST_RemoveExpr (table, p->final_value);
      p->incr_value = PST_RemoveExpr (table, p->incr_value);
      p->child = PST_RemoveStmt (table, p->child);

      p = P_RemoveParLoop (p);
    }

  return (p);
}

/*! \brief Removes a SerLoop.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the SerLoop to free.
 *
 * \return
 *  A null SerLoop pointer.
 *
 * Removes a SerLoop and all symbol table entries that reference it or
 * child structures.
 */
SerLoop
PST_RemoveSerLoop (SymbolTable table, SerLoop s)
{
  if (s)
    {
      s->loop_body = PST_RemoveStmt (table, s->loop_body);
      s->cond_expr = PST_RemoveExpr (table, s->cond_expr);
      s->init_expr = PST_RemoveExpr (table, s->init_expr);
      s->iter_expr = PST_RemoveExpr (table, s->iter_expr);

      s = P_RemoveSerLoop (s);
    }

  return (s);
}

/*! \brief Removes an AsmStmt.
 *
 * \param table
 *  the symbol table.
 * \param a
 *  the AsmStmt to free.
 *
 * \return
 *  A null AsmStmt pointer.
 *
 * Removes an AsmStmt and all symbol table entries that reference it or
 * child structures.
 */
AsmStmt
PST_RemoveAsmStmt (SymbolTable table, AsmStmt a)
{
  if (a)
    {
      a->asm_clobbers = PST_RemoveExpr (table, a->asm_clobbers);
      a->asm_string = PST_RemoveExpr (table, a->asm_string);
      a->asm_operands = PST_RemoveExpr (table, a->asm_operands);
    
      a = P_RemoveAsmStmt (a);
    }

  return (a);
}

/*! \brief Removes an Expr.
 *
 * \param table
 *  the symbol table.
 * \param e
 *  the Expr to remove.
 *
 * \return
 *  A null Expr pointer.
 *
 * Adjusts the reference count of an Expr's type and value.type and removes
 * the Expr.
 */
Expr
PST_RemoveExprNode (SymbolTable table, Expr e)
{
  Key k;
  SymTabEntry entry;

  if (e)
    {
      k = e->key;

      PST_SetExprType (table, e, Invalid_Key);

      e->operands = PST_RemoveExpr (table, e->operands);

      if (e->opcode == OP_stmt_expr)
	e->value.stmt = PST_RemoveStmt (table, e->value.stmt);

      e = P_RemoveExpr (e);
      
      if (P_ValidKey (k) && (entry = PST_GetSymTabEntryFromMem (table, k)))
	{
	  P_SetSymTabEntryExpr (entry, NULL);

	  /* If the SymTabEntry's key is invalid, we have been called
	   * by PST_RemoveSymTabEntry, so there is no reason to call it. */
	  if (P_ValidKey (P_GetSymTabEntryKey (entry)))
	    entry = PST_RemoveSymTabEntry (table, entry);
	}
    }

  return (e);
}

/*! \brief Removes an Expr list.
 *
 * \param table
 *  the symbol table.
 * \param e
 *  the Expr to remove.
 * 
 * \return
 *  A null Expr pointer.
 *
 * Adjusts the reference count of an Expr's type and value.type and removes
 * the Expr list.
 */
Expr
PST_RemoveExpr (SymbolTable table, Expr e)
{
  if (e)
    {
      e->sibling = PST_RemoveExpr (table, e->sibling);
      e->next = PST_RemoveExpr (table, e->next);

      e = PST_RemoveExprNode (table, e);
    }

  return (e);
}

/*! \brief Removes an AsmDcl.
 *
 * \param table
 *  the symbol table.
 * \param a
 *  the AsmDcl to remove.
 *
 * \return
 *  A null AsmDcl pointer.
 *
 * Adjusts the reference count of an AsmDcl's type and removes the
 * AsmDcl.
 */
AsmDcl
PST_RemoveAsmDcl (SymbolTable table, AsmDcl a)
{
  Key k;

  if (a)
    {
      k = a->key;

      a->asm_clobbers = PST_RemoveExpr (table, a->asm_clobbers);
      a->asm_string = PST_RemoveExpr (table, a->asm_string);
      a->asm_operands = PST_RemoveExpr (table, a->asm_operands);

      a = P_RemoveAsmDcl (a);
    }

  return (a);
}

/*! \brief Removes a SymTabEntry.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the SymTabEntry to remove.
 *
 * \return
 *  A null SymTabEntry pointer.
 *
 * Adjusts the reference count of a SymTabEntry's types and removes
 * the SymTabEntry.
 */
SymTabEntry
PST_RemoveSymTabEntry (SymbolTable table, SymTabEntry s)
{
  Key k;

  if (s)
    {
      k = s->key;
      s->key = Invalid_Key;

      if (P_ValidKey (P_GetSymTabEntryScopeKey (s)))
	PST_RemoveEntryFromScope (table, P_GetSymTabEntryScopeKey (s), k);

      switch (s->type)
	{
	case ET_TYPE_LOCAL:
	case ET_VAR_LOCAL:
	case ET_STMT:
	case ET_EXPR:
	case ET_FIELD:
	case ET_ENUMFIELD:
	case ET_LABEL:
	case ET_BLOCK:
	  break;
	case ET_SCOPE:
	  {
	    Scope sc;
	    ScopeEntry se, cur;

	    if ((sc = P_GetSymTabEntryScope (s)))
	      {
		se = cur = P_CopyScopeEntry (P_GetScopeScopeEntry (sc));

		while (cur)
		  {
		    PST_RemoveEntry (table, P_GetScopeEntryKey (cur));
		    cur = P_GetScopeEntryNext (cur);
		  }

		se = P_RemoveScopeEntry (se);
	      }
	  }
	  break;
	case ET_FUNC:
	  {
	    FuncDcl f = P_GetSymTabEntryFuncDcl (s);
	    f = PST_RemoveFuncDcl (table, f);
	    P_SetSymTabEntryFuncDcl (s, f);
	  }
	  break;
	case ET_TYPE_GLOBAL:
	  {
	    TypeDcl t = P_GetSymTabEntryTypeDcl (s);
	    t = PST_RemoveTypeDcl (table, t);
	    P_SetSymTabEntryTypeDcl (s, t);
	  }
	  break;
	case ET_VAR_GLOBAL:
	  {
	    VarDcl v = P_GetSymTabEntryVarDcl (s);
	    v = PST_RemoveVarDcl (table, v);
	    P_SetSymTabEntryVarDcl (s, v);
	  }
	  break;
	case ET_STRUCT:
	  {
	    StructDcl st = P_GetSymTabEntryStructDcl (s);
	    st = PST_RemoveStructDcl (table, st);
	    P_SetSymTabEntryStructDcl (s, st);
	  }
	  break;
	case ET_UNION:
	  {
	    UnionDcl u = P_GetSymTabEntryUnionDcl (s);
	    u = PST_RemoveUnionDcl (table, u);
	    P_SetSymTabEntryUnionDcl (s, u);
	  }
	  break;
	case ET_ASM:
	  {
	    AsmDcl a = P_GetSymTabEntryAsmDcl (s);
	    a = PST_RemoveAsmDcl (table, a);
	    P_SetSymTabEntryAsmDcl (s, a);
	  }
	  break;
	default:
	  P_punt ("struct_symtab.c:PST_RemoveSymTabEntry:%d Invalid entry "
		  "type %d", __LINE__ - 1, s->type);
	}

      PST_ClearEntry (table, k);

      s = P_RemoveSymTabEntry (s);
    }

  return (s);
}

/*! \brief Removes an IPSymTabEnt.
 *
 * \param table
 *  the symbol table.
 * \param i
 *  the IPSymTabEnt to remove.
 *
 * \return
 *  A null IPSymTabEnt pointer.
 *
 * Adjusts the reference count of an IPSymTabEnt's types and removes
 * the IPSymTabEnt.
 */
IPSymTabEnt
PST_RemoveIPSymTabEnt (SymbolTable table, IPSymTabEnt i)
{
  int index;
  SymTabEntry entry;

  if (i)
    {
      /* Mark the IPSymTabEnt's files as not available so we just kick
       * it out of memory without touching disk. */
      if (P_GetIPSymTabEntInFileStatus (i) == FS_READ_PERM)
	{
	  P_file_close (P_GetIPSymTabEntFile (i));
	  P_SetIPSymTabEntFile (i, NULL);
	}
      P_SetIPSymTabEntInFileStatus (i, FS_NOT_AVAIL);
      P_SetIPSymTabEntOutFileStatus (i, FS_NOT_AVAIL);

      if (P_GetIPSymTabEntTable (i))
	{
	  for (entry = (SymTabEntry)BlockSparseArrayGetFirstNonZero \
		                      (&(P_GetIPSymTabEntTable (i)), &index);
	       entry;
	       entry = (SymTabEntry)BlockSparseArrayGetNextNonZero \
		                      (&(P_GetIPSymTabEntTable (i)), &index))
	    {
	      entry = PST_RemoveSymTabEntry (table, entry);
	    }
	}

      i = P_RemoveIPSymTabEnt (i);
    }

  return (i);
}

/*! \brief Removes a SymbolTable.
 *
 * \param table
 *  the symbol table.
 *
 * \return
 *  A null SymbolTable pointer.
 *
 * Adjusts the reference count of a SymbolTable's types and removes
 * the SymbolTable.
 */
SymbolTable
PST_RemoveSymbolTable (SymbolTable table)
{
  int i;

  P_SetSymbolTableFlags (table, STF_REMOVING);

  if (table)
    {
      /* Mark the symbol table's files as not available so we just kick
       * it out of memory without touching disk. */
      if (P_GetSymbolTableInFileStatus (table) == FS_READ_PERM)
	{
	  P_file_close (P_GetSymbolTableFile (table));
	  P_SetSymbolTableFile (table, NULL);
	}
      P_SetSymbolTableInFileStatus (table, FS_NOT_AVAIL);
      P_SetSymbolTableOutFileStatus (table, FS_NOT_AVAIL);
	
      if (P_GetSymbolTableIPTable (table))
	{
	  for (i = 1; i <= P_GetSymbolTableNumFiles (table); i++)
	    {
	      PST_SetFile (table, i,
			   PST_RemoveIPSymTabEnt (table,
						  PST_GetFile (table, i)));
	    }
	}

      table = P_RemoveSymbolTable (table);
    }

  return (table);
}

/*! \brief Returns the parent FuncDcl for a scope key
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the key of the scope to inspect.
 *
 * \return
 *  The FuncDcl enclosing scope_key.
 */
FuncDcl
PST_GetScopeParentFunc (SymbolTable table, Key scope_key)
{
  FuncDcl result = NULL;
  Key global_scope = PST_GetGlobalScope (table);
  SymTabEntry cur_entry;

  while (result == NULL && !P_MatchKey (scope_key, global_scope))
    {
      cur_entry = PST_GetSymTabEntry (table, scope_key);

      if (P_GetSymTabEntryType (cur_entry) == ET_FUNC)
	result = P_GetSymTabEntryFuncDcl (cur_entry);
      else
	scope_key = P_GetSymTabEntryScopeKey (cur_entry);
    }

  return (result);
}

/*! \brief Copies a FuncDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new FuncDcl will be created (pass Invalid_Key to
 *  use same scope as original FuncDcl).
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the FuncDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the FuncDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a f.
 *
 * \note Modules should typically use #PST_CopyFuncDcl() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa P_CopyFuncDcl(), #PST_CopyFuncDclK(), copy_func_dcl()
 */
FuncDcl
PST_CopyFuncDclToTableScope (SymbolTable dst_table, Key dst_scope,
			     SymbolTable src_table, FuncDcl f, bool preserve)
{
  FuncDcl new = NULL;
  KeyList clean = NULL;
  
  if (f)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyFuncDclToTableScope:%d copying "
		"FuncDcl within table and\ntrying to preserve key.",
		__LINE__ - 2);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = PST_GetFileScope (dst_table, P_GetFuncDclKey (f).file);

      new = copy_func_dcl (dst_table, dst_scope, src_table, f, preserve,
			   &clean);
      
      update_func_dcl_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies a TypeDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new TypeDcl will be created (pass Invalid_Key to
 *  use same scope as original TypeDcl).
 * \param src_table
 *  the source symbol table.
 * \param t
 *  the TypeDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the TypeDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a t.
 *
 * \note Modules should typically use this #PST_CopyTypeDcl() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa P_CopyTypeDcl(), #PST_CopyTypeDclK(), copy_type_dcl()
 */
TypeDcl
PST_CopyTypeDclToTableScope (SymbolTable dst_table, Key dst_scope,
			     SymbolTable src_table, TypeDcl t, bool preserve)
{
  TypeDcl new = NULL;
  KeyList clean = NULL;

  if (t)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyTypeDclToTableScope:%d copying "
		"TypeDcl within table and\ntrying to preserve key.",
		__LINE__ - 2);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = get_dst_scope (dst_table, src_table,
				   PST_GetTypeDclScope (src_table, t),
				   preserve);

      new = copy_type_dcl (dst_table, dst_scope, src_table, t, preserve,
			   &clean);

      update_type_dcl_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies a VarDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new VarDcl will be created (pass Invalid_Key to
 *  use same scope as original VarDcl).
 * \param src_table
 *  the source symbol table.
 * \param v
 *  the VarDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the VarDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a v.
 *
 * \note Modules should typically use this #PST_CopyVarDcl() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyVarDcl(), P_CopyVarDcl(), copy_var_dcl()
 */
VarDcl
PST_CopyVarDclToTableScope (SymbolTable dst_table, Key dst_scope,
			    SymbolTable src_table, VarDcl v, bool preserve)
{
  VarDcl new = NULL;
  KeyList clean = NULL;

  if (v)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyVarDclToTableScope:%d copying VarDcl "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = get_dst_scope (dst_table, src_table,
				   PST_GetVarDclScope (src_table, v),
				   preserve);

      new = copy_var_dcl (dst_table, dst_scope, src_table, v, preserve,
			  &clean);

      update_var_dcl_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies a StructDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new StructDcl will be created (pass Invalid_Key
 *  to use same scope as original StructDcl).
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the StructDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the StructDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a s.
 *
 * If \a dst_table and \a src_table are the same, the copy is added to the
 * \a s->next list of alternative StructDcls with the same name.
 *
 * \note Modules should typically use #PST_CopyStructDcl() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyStructDcl(), P_CopyStructDcl(), copy_struct_dcl()
 */
StructDcl
PST_CopyStructDclToTableScope (SymbolTable dst_table, Key dst_scope,
			       SymbolTable src_table, StructDcl s,
			       bool preserve)
{
  StructDcl new = NULL;
  KeyList clean = NULL;

  if (s)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyStructDclToTableScope:%d copying "
		"StructDcl within table and\ntrying to preserve key.",
		__LINE__ - 2);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = get_dst_scope (dst_table, src_table,
				   PST_GetStructDclScope (src_table, s),
				   preserve);
      
      new = copy_struct_dcl (dst_table, dst_scope, src_table, s, preserve,
			     &clean);

      update_struct_dcl_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies a UnionDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new UnionDcl will be created (pass Invalid_Key
 *  to use same scope as original UnionDcl).
 * \param src_table
 *  the source symbol table.
 * \param u
 *  the UnionDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the UnionDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a u.
 *
 * If \a dst_table and \a src_table are the same, the copy is added to the
 * \a s->next list of alternative UnionDcls with the same name.
 *
 * \note Modules should typically use #PST_CopyUnionDcl() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyUnionDcl(), P_CopyUnionDcl(), copy_union_dcl()
 */
UnionDcl
PST_CopyUnionDclToTableScope (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, UnionDcl u, bool preserve)
{
  UnionDcl new = NULL;
  KeyList clean = NULL;

  if (u)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyUnionDclToTableScope:%d copying "
		"UnionDcl within table and\ntrying to preserve key.",
		__LINE__ - 2);

      if (!P_ValidKey (dst_scope))
	dst_scope = get_dst_scope (dst_table, src_table,
				   PST_GetUnionDclScope (src_table, u),
				   preserve);

      new = copy_union_dcl (dst_table, dst_scope, src_table, u, preserve,
			    &clean);

      update_union_dcl_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies a Field and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Field will be created (pass Invalid_Key
 *  to use same scope as original Field).
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the Field (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the Field in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a f.
 *
 * Copies a Field.  The Field.parent_key field is not copied.
 *
 * \note Modules should typically use #PST_CopyField() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyField(), P_CopyField(), copy_field()
 */
Field
PST_CopyFieldToTableScope (SymbolTable dst_table, Key dst_scope,
			   SymbolTable src_table, Field f, bool preserve)
{
  Field new = NULL;
  KeyList clean = NULL;

  if (f)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyFieldToTableScope:%d copying Field "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = \
	  get_dst_scope (dst_table, src_table,
			 PST_GetScopeFromEntryKey (src_table,
						   P_GetFieldKey (f)),
			 preserve);

      new = copy_field (dst_table, dst_scope, src_table, f, preserve, &clean);

      update_field_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies an EnumDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new EnumDcl will be created (pass Invalid_Key
 *  to use same scope as original EnumDcl).
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the EnumDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the EnumDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a e.
 *
 * \note Modules should typically use #PST_CopyEnumDcl() instea of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyEnumDcl(), P_CopyEnumDcl(), copy_enum_dcl()
 */
EnumDcl
PST_CopyEnumDclToTableScope (SymbolTable dst_table, Key dst_scope,
			     SymbolTable src_table, EnumDcl e, bool preserve)
{
  EnumDcl new = NULL;
  KeyList clean = NULL;

  if (e)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyEnumDclToTableScope:%d copying "
		"EnumDcl within table and\ntrying to preserve key.",
		__LINE__ - 2);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = \
	  get_dst_scope (dst_table, src_table,
			 PST_GetScopeFromEntryKey (src_table,
						   P_GetEnumDclKey (e)),
			 preserve);

      new = copy_enum_dcl (dst_table, dst_scope, src_table, e, preserve,
			   &clean);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies an EnumField and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new EnumField will be created (pass Invalid_Key
 *  to use same scope as original EnumField).
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the EnumField (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the EnumField in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a f.
 *
 * \note Modules should typically use #PST_CopyEnumField() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyEnumField(), P_CopyEnumField(), copy_enum_field()
 */
EnumField
PST_CopyEnumFieldToTableScope (SymbolTable dst_table, Key dst_scope,
			       SymbolTable src_table, EnumField f,
			       bool preserve)
{
  EnumField new = NULL;
  KeyList clean = NULL;

  if (f)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyEnumFieldDclToTableScope:%d copying "
		"EnumField within table and\ntrying to preserve key.",
		__LINE__ - 2);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = \
	  get_dst_scope (dst_table, src_table,
			 PST_GetScopeFromEntryKey (src_table,
						   P_GetEnumFieldKey (f)),
			 preserve);

      new = copy_enum_field (dst_table, dst_scope, src_table, f, preserve,
			     &clean);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies a single Stmt and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Stmt will be created (pass Invalid_Key to
 *  use same scope as original Stmt).
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the Stmt (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the Stmt in \a dst_table.  \a dst_table is updated so that the
 *  copy is completely distinct from \a s.  If \a dst_scope is valid, it is
 *  used as the new Stmt's scope.
 *
 * Only the given Stmt is copied, not the entire list.
 *
 * ::Stmt.lex_prev, ::Stmt.lex_next, ::Stmt.parent, ::Stmt.parent_func,
 * and ::Stmt.parent_expr are set to null in the new Stmt.
 *
 * \note Modules should typically #PST_CopyStmtNodeToScope() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyStmtNodeToScope(), P_CopyStmtNode(), PST_CopyStmt(),
 * copy_stmt_node() */
Stmt
PST_CopyStmtNodeToTableScope (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, Stmt s, bool preserve)
{
  Stmt new = NULL;
  KeyList clean = NULL;

  if (s)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyStmtNodeToTableScope:%d copying "
		"Stmt within table and\ntrying to preserve key.",
		__LINE__ - 2);
      
      if (!P_ValidKey (dst_scope))
	{
	  dst_scope = get_dst_scope (dst_table, src_table,
				     PST_GetStmtScope (src_table, s),
				     preserve);
	}

      new = copy_stmt_node (dst_table, dst_scope, src_table, s, preserve,
			    &clean);

      update_stmt_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);
}

/*! \brief Copies a Stmt and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Stmt will be created (pass Invalid_Key to
 *  use same scope as original Stmt).
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the Stmt (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the Stmt in \a dst_table.  \a dst_table is updated so that the
 *  copy is completely distinct from \a s.  If \a dst_scope is valid, it is
 *  used as the new Stmt's scope.
 *
 * The entire list (lex_next chain) is copied.
 *
 * ::Stmt.parent, ::Stmt.parent_func, and ::Stmt.parent_expr are set to null
 * in the new Stmt.
 *
 * \note Modules should typically use #PST_CopyStmtToScope() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyStmtToScope(), P_CopyStmt(), PST_CopyStmtNode(), copy_stmt() */
Stmt
PST_CopyStmtToTableScope (SymbolTable dst_table, Key dst_scope,
			  SymbolTable src_table, Stmt s, bool preserve)
{
  Stmt new = NULL;
  KeyList clean = NULL;

  if (s)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyStmtToTableScope:%d copying Stmt "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	{
	  dst_scope = get_dst_scope (dst_table, src_table,
				     PST_GetStmtScope (src_table, s),
				     preserve);
	}

      new = copy_stmt (dst_table, dst_scope, src_table, s, preserve, &clean);

      update_stmt_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);      
}

/*! \brief Copies a Label and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Label will be created (pass Invalid_Key
 *  to use same scope as original Label).
 * \param src_table
 *  the source symbol table.
 * \param l
 *  the Label (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the Label in \a dst_table.  \a dst_table is updated so that the
 *  copy is completely distinct from \a l.
 *
 * All labels in the list are copied.
 *
 * \note Modules should typically use #PST_CopyLabel() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa P_CopyLabel(), PST_CopyLabelK(), copy_label() */
Label
PST_CopyLabelToTableScope (SymbolTable dst_table, Key dst_scope,
			   SymbolTable src_table, Label l, bool preserve)
{
  Label new = NULL;
  KeyList clean = NULL;

  if (l)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyLabelToTableScope:%d copying Label "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = \
	  get_dst_scope (dst_table, src_table,
			 PST_GetScopeFromEntryKey (src_table,
						   P_GetLabelKey (l)),
			 preserve);

      new = copy_label (dst_table, dst_scope, src_table, l, preserve, &clean);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);      
}

/*! \brief Copies a single Expr without operands, siblings, or the next list.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Stmt will be created (pass Invalid_Key to
 *  use same scope as original Stmt).
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the Expr (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the Expr in \a dst_table without operands, sibling, or
 *  the next list.  \a dst_table is updated so that the copy is
 *  completely distinct from \a e.  If \a dst_scope is valid, it is
 *  used as the new Stmt's scope.
 *
 * \note Modules should typically use #PST_CopyExprNode() or
 *       #PST_CopyExprNodeToScope() instead of this function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyExprNode(), #PST_CopyExprNodeToScope(), copy_expr_node() */
Expr
PST_CopyExprNodeToTableScope (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, Expr e, bool preserve)
{
  Expr new = NULL;
  KeyList clean = NULL;

  if (e)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyExprNodeToTableScope:%d copying Expr "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = get_dst_scope (dst_table, src_table,
				   PST_GetExprScope (src_table, e),
				   preserve);
	
      new = copy_expr_node (dst_table, dst_scope, src_table, e, preserve,
			    &clean);

      update_expr_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);      
}

/*! \brief Copies an Expr and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Stmt will be created (pass Invalid_Key to
 *  use same scope as original Stmt).
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the Expr (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the Expr in \a dst_table.  \a dst_table is updated so that the
 *  copy is completely distinct from \a e.  If \a dst_scope is valid, it is
 *  used as the new Stmt's scope.
 *
 * Copies an Expr.  Expr.next is not copied.
 *
 * \note Modules should typically use #PST_CopyExpr() or #PST_CopyExprToScope()
 *       instead of this function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyExpr(), #PST_CopyExprToScope(), copy_expr() */
Expr
PST_CopyExprToTableScope (SymbolTable dst_table, Key dst_scope,
			  SymbolTable src_table, Expr e, bool preserve)
{
  Expr new = NULL;
  KeyList clean = NULL;

  if (e)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyExprToTableScope:%d copying Expr "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = get_dst_scope (dst_table, src_table,
				   PST_GetExprScope (src_table, e),
				   preserve);

      new = copy_expr (dst_table, dst_scope, src_table, e, preserve, &clean);

      update_expr_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);      
}

/*! \brief Copies an Expr list and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Stmt will be created (pass Invalid_Key to
 *  use same scope as original Stmt).
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the Expr (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the Expr in \a dst_table.  \a dst_table is updated so that the
 *  copy is completely distinct from \a e.  If \a dst_scope is valid, it is
 *  used as the new Stmt's scope.
 *
 * Copies an Expr.  The entire Expr list (Expr.next) is copied.
 *
 * \note Modules should typically use #PST_CopyExprList() or
 *       #PST_CopyExprListToScope() instead of this function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyExprList(), #PST_CopyExprListToScope(), copy_expr_list() */
Expr
PST_CopyExprListToTableScope (SymbolTable dst_table, Key dst_scope,
			      SymbolTable src_table, Expr e, bool preserve)
{
  Expr new = NULL;
  KeyList clean = NULL;

  if (e)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyExprListToTableScope:%d copying Expr "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = get_dst_scope (dst_table, src_table,
				   PST_GetExprScope (src_table, e),
				   preserve);

      new = copy_expr_list (dst_table, dst_scope, src_table, e, preserve,
			    &clean);

      update_expr_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);      
}

/*! \brief Copies an AsmDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new AsmDcl will be created (pass Invalid_Key
 *  to use same scope as original AsmDcl).
 * \param src_table
 *  the source symbol table.
 * \param a
 *  the AsmDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the AsmDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a a.
 *
 * Copies an AsmDcl.
 *
 * \note Modules should typically use #PST_CopyAsmDcl() instead of this
 *       function.
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa #PST_CopyAsmDcl(), P_CopyAsmDcl(), copy_asm_dcl() */
AsmDcl
PST_CopyAsmDclToTableScope (SymbolTable dst_table, Key dst_scope,
			    SymbolTable src_table, AsmDcl a, bool preserve)
{
  AsmDcl new = NULL;
  KeyList clean = NULL;

  if (a)
    {
      if (preserve && dst_table == src_table)
	P_warn ("struct_symtab.c:PST_CopyAsmDclToTableScope:%d copying AsmDcl "
		"within table and\ntrying to preserve key.", __LINE__ - 1);
      
      if (!P_ValidKey (dst_scope))
	dst_scope = PST_GetGlobalScope (dst_table);

      new = copy_asm_dcl (dst_table, dst_scope, src_table, a, preserve,
			  &clean);

      update_asm_dcl_keys (dst_table, src_table, new);
    }

  if (clean)
    PST_CleanEntries (src_table, &clean);

  return (new);      
}

/*! \brief Copies a SymTabEntry to a given key and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new SymTabEntry will be created (pass Invalid_Key
 *  to use same scope as original SymTabEntry).
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the SymTabEntry (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 *
 * \return
 *  A copy of the SymTabEntry in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a e.
 *
 * Copies \a e to key \a dst_key in \a dst_table, updating the keys.  If an
 * entry already exists with that key, it is replaced.
 *
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 */
SymTabEntry
PST_CopySymTabEntryToTableScope (SymbolTable dst_table, Key dst_scope,
				 SymbolTable src_table, SymTabEntry e,
				 bool preserve)
{
  SymTabEntry result = NULL;
  Key dst_key = Invalid_Key;

  if (e)
    {
      switch (P_GetSymTabEntryType (e))
	{
	case ET_FUNC:
	  {
	    FuncDcl f = \
	      PST_CopyFuncDclToTableScope (dst_table, dst_scope, src_table,
					   P_GetSymTabEntryFuncDcl (e),
					   preserve);
	    result = PST_GetSymTabEntry (dst_table, P_GetFuncDclKey (f));
	  }
	  break;
	case ET_TYPE_GLOBAL:
	  {
	    TypeDcl t = \
	      PST_CopyTypeDclToTableScope (dst_table, dst_scope, src_table,
					   P_GetSymTabEntryTypeDcl (e),
					   preserve);
	    result = PST_GetSymTabEntry (dst_table, P_GetTypeDclKey (t));
	  }
	  break;
	case ET_VAR_GLOBAL:
	  {
	    VarDcl v = \
	      PST_CopyVarDclToTableScope (dst_table, dst_scope, src_table,
					  P_GetSymTabEntryVarDcl (e),
					  preserve);
	    result = PST_GetSymTabEntry (dst_table, P_GetVarDclKey (v));
	  }
	  break;
	case ET_STRUCT:
	  {
	    StructDcl s = \
	      PST_CopyStructDclToTableScope (dst_table, dst_scope, src_table,
					     P_GetSymTabEntryStructDcl (e),
					     preserve);
	    result = PST_GetSymTabEntry (dst_table, P_GetStructDclKey (s));
	  }
	  break;
	case ET_UNION:
	  {
	    UnionDcl u = \
	      PST_CopyUnionDclToTableScope (dst_table, dst_scope, src_table,
					    P_GetSymTabEntryUnionDcl (e),
					    preserve);
	    result = PST_GetSymTabEntry (dst_table, P_GetUnionDclKey (u));
	  }
	  break;
	case ET_ENUM:
	  {
	    EnumDcl f = \
	      PST_CopyEnumDclToTableScope (dst_table, dst_scope, src_table,
					   P_GetSymTabEntryEnumDcl (e),
					   preserve);
	    result = PST_GetSymTabEntry (dst_table, P_GetEnumDclKey (f));
	  }
	  break;
	case ET_ASM:
	  {
	    AsmDcl a = \
	      PST_CopyAsmDclToTableScope (dst_table, dst_scope, src_table,
					  P_GetSymTabEntryAsmDcl (e),
					  preserve);
	    result = PST_GetSymTabEntry (dst_table, P_GetAsmDclKey (a));
	  }
	  break;
	case ET_SCOPE:
	  {
	    Key new_scope_key;
	    
	    dst_key = P_GetSymTabEntryKey (e);
	    if (!preserve)
	      dst_key.sym = 0;
	    
	    new_scope_key = PST_AddNewScope (dst_table, dst_key);
	    
	    result = PST_GetSymTabEntry (dst_table, new_scope_key);
	  }
	  break;
	case ET_TYPE_LOCAL:
	case ET_VAR_LOCAL:
	case ET_FIELD:
	case ET_ENUMFIELD:
	case ET_LABEL:
	case ET_STMT:
	case ET_EXPR:
#if 0
	  result = PST_GetSymTabEntry (dst_table, P_GetSymTabEntryOutKey (e));
#endif
	  result = NULL;
	  break;
	default:
	  break;
	}
    }

  return (result);
}

/*! \brief Finds the key of the function defining the given scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the key of the defined scope.
 *
 * \return
 *  The function's key.
 *
 * Searches up through the scope tree to find the function scope defining
 * \a scope_key.  Returns the function's key.  */
Key
PST_ScopeFindFuncScope (SymbolTable table, Key scope_key)
{
  SymTabEntry table_entry;
  Key entry_key = Invalid_Key;

  if ((table_entry = PST_GetSymTabEntry (table, scope_key)))
    {
      while (table_entry && P_GetSymTabEntryType (table_entry) != ET_FUNC)
	{
	  entry_key = P_GetSymTabEntryScopeKey (table_entry);
	  table_entry = PST_GetSymTabEntry (table, entry_key);
	}

      /* If we didn't find a function scope, return {0, 0}. */
      if (table_entry == NULL)
	{
	  entry_key.file = 0;
	  entry_key.sym = 0;
	}
    }

  return (entry_key);
}

/*! \brief Recursively updates the expression IDs in an Expr.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the Expr is defined.
 * \param expr
 *  the Expr to update.
 *
 * Recursively assigns new expression IDs to \a expr and all operands.
 *
 * \sa PST_ScopeNextExprID(), #PST_UpdateExprIDs(), #PST_CopyExpr(),
 * #PST_CopyExprNode(), #PST_CopyExprList() */
void
PST_ScopeUpdateExprIDs (SymbolTable table, Key scope_key, Expr expr)
{
  Expr temp;

  P_SetExprID (expr, PST_ScopeNextExprID (table, scope_key));

  for (temp = P_GetExprOperands (expr); temp; temp = P_GetExprSibling (temp))
    PST_ScopeUpdateExprIDs (table, scope_key, temp);

  if ((temp = P_GetExprNext (expr)))
    PST_ScopeUpdateExprIDs (table, scope_key, temp);

  return;
}

/* Functions to create new Pcode data structures that need the symbol table. */

/*! \brief Generates a new Pcode Expr with the next ID for a scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the expr is defined.
 *
 * \return
 *  A pointer to the new Expr.
 *
 * \sa P_NewExpr(), PST_ScopeNewExprWithOpcode(),
 * PST_ScopeNewStringExpr(), PST_ScopeNewIntExpr(),
 * PST_ScopeNewUIntExpr(), PST_ScopeNewFloatExpr(),
 * PST_ScopeNewDoubleExpr() */
Expr
PST_ScopeNewExpr (SymbolTable table, Key scope_key)
{
  Expr result = P_NewExpr ();

  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));

  return (result);
}

/*! \brief Generates a new Pcode Expr with an opcode.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the expr is defined.
 * \param opcode
 *  the expr's opcode.
 *
 * \return
 *  A pointer to the new Expr.
 *
 * \sa P_NewExpr(), P_NewExprWithOpcode(), PST_ScopeNewExpr(),
 * PST_ScopeNewStringExpr(), PST_ScopeNewIntExpr(),
 * PST_ScopeNewUIntExpr(), PST_ScopeNewFloatExpr(),
 * PST_ScopeNewDoubleExpr() */
Expr
PST_ScopeNewExprWithOpcode (SymbolTable table, Key scope_key, _Opcode opcode)
{
  Expr result = P_NewExprWithOpcode (opcode);

  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));

  return (result);
}

/*! \brief Generates a new Pcode string Expr with a type from the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the string's type is defined.
 * \param s
 *  the value of the string Expr.
 *
 * \return
 *  A pointer to the string Expr.
 *
 * Generates a new Pcode string expression with a type from the symbol
 * table.  If necessary, a char * type is added to the symbol table for
 * the string.
 *
 * \note This function copies the string passed as \a s.  The caller must
 * free its copy after the call.
 *
 * \sa P_NewExpr(), P_NewStringExpr(), PST_ScopeNewExpr(),
 * PST_ScopeNewExprWithOpcode(), PST_ScopeNewIntExpr(),
 * PST_ScopeNewUIntExpr(), PST_ScopeNewFloatExpr(),
 * PST_ScopeNewDoubleExpr() */
Expr
PST_ScopeNewStringExpr (SymbolTable table, Key scope_key, char *s)
{
  Expr result;
  Type char_type, char_ptr_type, const_char_ptr_type;

  /* Find the const char * type. */
  char_type = PST_FindBasicType (table, BT_CHAR);
  char_ptr_type = PST_FindPointerToType (table, char_type);
  const_char_ptr_type = PST_FindTypeSetQualifier (table, char_ptr_type,
						  TY_CONST);

  result = P_NewStringExpr (s);
  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));
  PST_SetExprType (table, result, const_char_ptr_type);

  return (result);
}

/*! \brief Generates a new Pcode int Expr with a type from the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the int's type is defined.
 * \param i
 *  the value of the int Expr.
 *
 * \return
 *  A pointer to the int Expr.
 *
 * Generates a new Pcode int expression with a type from the symbol table.
 *
 * \sa P_NewExpr(), P_NewIntExpr(), PST_ScopeNewExpr(), *
 * PST_ScopeNewExprWithOpcode(), PST_ScopeNewStringExpr(), *
 * PST_ScopeNewUIntExpr(), PST_ScopeNewFloatExpr(), *
 * PST_ScopeNewDoubleExpr() */
Expr
PST_ScopeNewIntExpr (SymbolTable table, Key scope_key, ITintmax i)
{
  Expr result = P_NewIntExpr (i);

  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));

  PST_SetExprType (table, result,
		   PST_FindBasicTypeWithQual (table, BT_INT, TY_CONST));

  return (result);
}

/*! \brief Generates a new Pcode uint Expr with a type from the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the int's type is defined.
 * \param i
 *  the value of the unsigned int Expr.
 *
 * \return
 *  A pointer to the unsigned int Expr.
 *
 * Generates a new Pcode unsigned int expression with a type from the
 * symbol table.
 *
 * \sa P_NewExpr(), P_NewIntExpr(), PST_ScopeNewExpr(),
 * PST_ScopeNewExprWithOpcode(), PST_ScopeNewStringExpr(),
 * PST_ScopeNewIntExpr(), PST_ScopeNewFloatExpr(),
 * PST_ScopeNewDoubleExpr() */
Expr
PST_ScopeNewUIntExpr (SymbolTable table, Key scope_key, ITuintmax i)
{
  Expr result = P_NewUIntExpr (i);

  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));

  PST_SetExprType (table, result,
		   PST_FindBasicTypeWithQual (table, BT_UNSIGNED | BT_INT,
					      TY_CONST));

  return (result);
}

/*! \brief Generates a new Pcode float Expr with a type from the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the float's type is defined.
 * \param f
 *  the value of the float Expr.
 *
 * \return
 *  A pointer to the float Expr.
 *
 * Generates a new Pcode float expression with a type from the symbol table.
 *
 * \sa P_NewExpr(), P_NewFloatExpr(), PST_ScopeNewExpr(),
 * PST_ScopeNewExprWithOpcode(), PST_ScopeNewStringExpr(),
 * PST_ScopeNewIntExpr(), PST_ScopeNewUIntExpr(),
 * PST_ScopeNewDoubleExpr() */
Expr
PST_ScopeNewFloatExpr (SymbolTable table, Key scope_key, double f)
{
  Expr result = P_NewFloatExpr (f);

  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));

  PST_SetExprType (table, result,
		   PST_FindBasicTypeWithQual (table, BT_FLOAT, TY_CONST));

  return (result);
}

/*! \brief Generates a new Pcode double expr with a type from the symbol table.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the double's type is defined.
 * \param d
 *  the value of the double Expr.
 *
 * \return
 *  A pointer to the double Expr.
 *
 * Generates a new Pcode double expression with a type from the symbol table.
 *
 * \sa P_NewExpr(), P_NewDoubleExpr(), PST_ScopeNewExpr(),
 * PST_ScopeNewExprWithOpcode(), PST_ScopeNewStringExpr(),
 * PST_ScopeNewIntExpr(), PST_ScopeNewUIntExpr(),
 * PST_ScopeNewFloatExpr() */
Expr
PST_ScopeNewDoubleExpr (SymbolTable table, Key scope_key, double d)
{
  Expr result = P_NewDoubleExpr (d);

  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));

  PST_SetExprType (table, result,
		   PST_FindBasicTypeWithQual (table, BT_DOUBLE, TY_CONST));

  return (result);
}

/*! \brief Generates a unique identifier in a scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which to create the identifier.
 * \param tag
 *  an optional tag to identify where the identifier was created.
 *
 * \return
 *  The identifier.
 *
 * Generates a unique identifier in the scope.  The identifier is of the form
 * PT_(tag)_(scope)_(serial), and is guaranteed to be unique within its
 * scope.  The string passed as \a tag is copied, so the caller is
 * responsible for freeing its copy.
 */
char *
PST_ScopeNewIdentifier (SymbolTable table, Key scope_key, char *tag)
{
  SymTabEntry scope;
  Stmt stmt = NULL;

  scope = PST_GetSymTabEntry (table, scope_key);

  /* Scope must be either a FuncDcl or a Compound statement. */
  switch (P_GetSymTabEntryType (scope))
    {
    case ET_STMT:
      stmt = P_GetSymTabEntryStmt (scope);
      break;

    case ET_FUNC:
      stmt = P_GetFuncDclStmt (P_GetSymTabEntryFuncDcl (scope));
      break;

    default:
      P_punt ("struct_symtab.c:PST_ScopeNewIdentifier:%d Invalid scope "
	      "(%d, %d)\nscope must be func or compound stmt.", __LINE__ - 1,
	      scope_key.file, scope_key.sym);
    }

  return (P_CompoundNewIdentifier (stmt, tag));
}

/*! \brief Allocates a new compound statement with scope.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which to create the stmt.
 *
 * Allocates a new compound statement and sets up its scope in the 
 * symbol table.
 */
Stmt
PST_NewCompoundStmt (SymbolTable table, Key scope_key)
{
  Stmt result = P_NewStmtWithType (ST_COMPOUND);
  Key stmt_key = Invalid_Key;
  
  /* HP's compiler isn't happy with initializing void_ptr_key to
   * {scope_key.file, 0}. */
  stmt_key.file = scope_key.file;

  P_SetStmtKey (result, stmt_key);
  P_SetStmtCompound (result, P_NewCompound ());

  /* Insert the statement into the symbol table. */
  stmt_key = PST_AddStmtEntry (table, result);

  /* Insert the statement into the scope. */
  PST_AddEntryToScope (table, scope_key, stmt_key);

  /* Add a new scope for the statement. */
  PST_AddNewScope (table, stmt_key);

  return (result);
}

/*! \brief Returns a new nop Stmt with a label.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the Stmt is defined.
 * \param label_val
 *  the text value of the new label.
 * \param label_key
 *  returns the key of the new label.
 *
 * \return
 *  A pointer to the new nop Stmt.
 *
 * The string passed as \a label_val will be copied by this function, so the
 * caller is responsible for freeing its copy.
 */
Stmt
PST_NewNoopStmtWithLabel (SymbolTable table, Key scope_key, char *label_val,
			  Key *label_key)
{
  Stmt result = P_NewStmtWithType (ST_NOOP);

  P_SetStmtLabels (result, P_AppendLabelNext (P_GetStmtLabels (result),
					      PST_NewLabel (table, scope_key,
							    label_val)));

  return (result);
}

/*! \brief Returns a new Label.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the Label is defined.
 * \param label_val
 *  the text value of the new label.
 *
 * \return
 *  A pointer to the new Label.
 *
 * The string passed in as \a label_val will be copied by this function, so
 * the caller is responsible for freeing its copy.
 *
 * \sa #PST_NewLabelTemp()
 */
Label
PST_NewLabel (SymbolTable table, Key scope_key, char *label_val)
{
  Label result = P_NewLabel ();
  Key label_key = Invalid_Key;

  /* HP's compiler isn't happy with initializing label_key to
   * {scope_key.file, 0}. */
  label_key.file = scope_key.file;

  P_SetLabelType (result, LB_LABEL);
  P_SetLabelVal (result, strdup (label_val));
  P_SetLabelKey (result, label_key);

  /* Insert the label into the symbol table. */
  label_key = PST_AddLabelEntry (table, result);

  /* Add the label to the scope. */
  PST_AddEntryToScope (table, scope_key, label_key);

  return (result);
}

/*! \brief Returns a new temporary Label.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which the Label is defined.
 * \param tag
 *  an optional tag to identify where the label was generated.
 *
 * \return
 *  A pointer to the new Label.
 *
 * The string passed as \a tag will be copied by this function, so the
 * caller is responsible for freeing its copy.
 *
 * \sa PST_NewLabel()
 */
Label
PST_NewLabelTemp (SymbolTable table, Key scope_key, char *tag)
{
  Label result;
  char *tmp_name;

  tmp_name = PST_ScopeNewIdentifier (table, scope_key, tag);

  result = PST_NewLabel (table, scope_key, tmp_name);

  free (tmp_name);

  return (result);
}

/*! \brief Allocates a new local variable.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which to create the variable.
 * \param type
 *  the Type of the variable.
 * \param name
 *  the variable's name.
 *
 * \return
 *  The key of the new variable.
 *
 * Allocates a new local variable and inserts it into the local var list
 * of the scope.  The string passed as \a name is copied, so the caller
 * is responsible for freeing its copy.
 *
 * The type assigned to the variable will not have the const qualifier set,
 * even if it is set on \a type.
 *
 * \sa PST_NewLocalVarTemp(), PST_NewLocalVarExpr(), PST_NewLocalVarExprTemp()
 */
Key
PST_NewLocalVar (SymbolTable table, Key scope_key, Type type, char *name)
{
  SymTabEntry scope;
  Stmt stmt = NULL;
  VarDcl new_var;
  Key new_var_key = Invalid_Key;
  Type new_var_type;

  /* HP's compiler isn't happy with initializing new_var_key to
   * {scope_key.file, 0}. */
  new_var_key.file = scope_key.file;

  scope = PST_GetSymTabEntry (table, scope_key);

  /* Scope must either be a FuncDcl or a Compound statement. */
  switch (P_GetSymTabEntryType (scope))
    {
    case ET_STMT:
      stmt = P_GetSymTabEntryStmt (scope);
      break;

    case ET_FUNC:
      stmt = P_GetFuncDclStmt (P_GetSymTabEntryFuncDcl (scope));
      break;

    default:
      P_punt ("struct_symtab.c:PST_NewLocalVar:%d Invalid scope (%d, %d).\n"
	      "local variables must be inserted in func or compound stmt.",
	      __LINE__ - 2, scope_key.file, scope_key.sym);
    }

  if (P_GetStmtType (stmt) != ST_COMPOUND)
    P_punt ("struct_symtab.c:PST_NewLocalVar:%d stmt must be compound",
	    __LINE__ - 1);

  new_var = P_NewVarDcl ();
  P_SetVarDclKey (new_var, new_var_key);
  P_SetVarDclName (new_var, strdup (name));

  /* Convert arrays to pointers and make sure function types are pointers. */
  if (PST_IsArrayType (table, type))
    type = PST_FindPointerToType (table, PST_GetTypeType (table, type));
  else if (PST_IsFunctionType (table, type))
    type = PST_FindPointerToType (table, type);

  /* Clear the TY_CONST qualifier. */
  new_var_type = PST_FindTypeClrQualifier (table, type, TY_CONST);
  PST_SetVarDclType (table, new_var, new_var_type);

  /* Add the variable to the symbol table. */
  new_var_key = PST_AddVarDclEntry (table, new_var, ET_VAR_LOCAL);

  /* Add the variable to the stmt's var list. */
  P_AppendStmtLocalVar (stmt, new_var);

  /* Add the variable to the scope. */
  PST_AddEntryToScope (table, scope_key, new_var_key);

  return (new_var_key);
}

/*! \brief Allocates a new local temp variable.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which to create the variable.
 * \param type
 *  the Type of the variable.
 * \param tag
 *  (optional) a short tag that can be used to indicate where the variable
 *  was created.
 *
 * \return
 *  The key of the new variable.
 *
 * Allocates a new temporary variable and inserts it into the local var
 * list of the scope.  The name is of the form PT_(tag)_(scope)_(serial),
 * and is guaranteed to be unique within its scope.  The string passed as
 * \a tag is copied, so the caller is responsible for freeing its copy.
 *
 * The type assigned to the variable will not have the const qualifier set,
 * even if it is set on \a type.
 *
 * \sa PST_NewLocalVar(), PST_NewLocalVarExpr(), PST_NewLocalVarExprTemp()
 */
Key
PST_NewLocalVarTemp (SymbolTable table, Key scope_key, Type type, char *tag)
{
  Key result;
  char *tmp_name;

  tmp_name = PST_ScopeNewIdentifier (table, scope_key, tag);

  result = PST_NewLocalVar (table, scope_key, type, tmp_name);

  free (tmp_name);

  return (result);
}

/*! \brief Allocates a new local variable and returns an OP_var expression.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which to create the variable.
 * \param type
 *  the Type of the variable.
 * \param name
 *  the variable's name.
 *
 * \return
 *  An OP_var expression containing the variable.
 *
 * Allocates a new local variable and inserts it into the local var list
 * of the scope.  The var is inserted into a new OP_var expression, which
 * is returned to the caller.  The string passed as \a name is copied, so
 * the caller is responsible for freeing its copy.
 *
 * The type assigned to the variable will not have the const qualifier set,
 * even if it is set on \a type.
 *
 * \sa PST_NewLocalVar(), PST_NewLocalVarTemp(), PST_NewLocalVarExprTemp()
 */
Expr
PST_NewLocalVarExpr (SymbolTable table, Key scope_key, Type type, char *name)
{
  Expr result = PST_ScopeNewExprWithOpcode (table, scope_key, OP_var);
  Key var_key;

  var_key = PST_NewLocalVar (table, scope_key, type, name);

  P_SetExprVarKey (result, var_key);
  P_SetExprVarName (result, strdup (name));
  P_SetExprID (result, PST_ScopeNextExprID (table, scope_key));

  return (result);
}

/*! \brief Allocates a new local temp variable and returns an OP_var expr.
 *
 * \param table
 *  the symbol table.
 * \param scope_key
 *  the scope under which to create the variable.
 * \param type
 *  the Type of the variable.
 * \param tag
 *  (optional) a short tag that can be used to indicate where the variable
 *  was created.
 *
 * \return
 *  An OP_var expression containing the variable.
 *
 * Allocates a new temporary variable and inserts it into the local var list
 * of the scope.  The name is of the form PT_(tag)_(scope)_(serial), and is
 * guaranteed to be unique within its scope.  The var is inserted into a new
 * OP_var expression, which is returned to the caller.  The string passed as
 * \a tag is copied, so the caller is responsible for freeing its copy.
 *
 * The type assigned to the variable will not have the const qualifier set,
 * even if it is set on \a type.
 *
 * \sa PST_NewLocalVar(), PST_NewLocalVarTemp(), PST_NewLocalVarExpr()
 */
Expr
PST_NewLocalVarExprTemp (SymbolTable table, Key scope_key, Type type,
			 char *tag)
{
  Expr result = PST_ScopeNewExprWithOpcode (table, scope_key, OP_var);
  Key var_key;
  VarDcl var;

  var_key = PST_NewLocalVarTemp (table, scope_key, type, tag);
  var = PST_GetVarDclEntry (table, var_key);

  P_SetExprVarKey (result, var_key);
  P_SetExprVarName (result, strdup (P_GetVarDclName (var)));
  P_SetExprFlags (result, EF_TEMP);

  return (result);
}

/*! \brief Finds the closest scope for a Stmt.
 *
 * \param table
 *  the symbol table.
 * \param stmt
 *  the stmt to inspect.
 *
 * \return
 *  The key of the Stmt's scope.
 *
 * Recursively inspect parent stmts until we find a compound, then return
 * the compound's key.
 *
 * \sa PST_GetGlobalScope(), PST_GetFileScope(),
 * #PST_GetFuncDclScope(), #PST_GetTypeDclScope(),
 * #PST_GetVarDclScope(), #PST_GetStructDclScope(),
 * #PST_GetUnionDclScope(), PST_GetExprScope() */
Key
PST_GetStmtScope (SymbolTable table, Stmt stmt)
{
  Key result = Invalid_Key;

  /* If the stmt has a key, we already know its scope. */
  if (P_ValidKey (P_GetStmtKey (stmt)))
    {
      result = PST_GetScopeFromEntryKey (table, P_GetStmtKey (stmt));
    }
  else
    {
      if (!P_GetStmtParentStmt (stmt) && !P_GetStmtParentFunc (stmt))
	P_punt ("symtab.c:PST_GetStmtScope:%d stmt has no parents", __LINE__);

      /* If the parent stmt is not defined, return the parent func's key. */
      if (!P_GetStmtParentStmt (stmt))
	{
	  result = P_GetFuncDclKey (P_GetStmtParentFunc (stmt));
	}
      else
	{
	  Stmt parent = P_GetStmtParentStmt (stmt);
	  
	  /* If the parent stmt is a compound, return its key. */
	  if (P_GetStmtType (parent) == ST_COMPOUND)
	    {
	      /* If the compound has no key, complain.  This should never
	       * happen. */
	      if (!P_ValidKey (P_GetStmtKey (parent)))
		P_punt ("symtab.c:PST_GetStmtScope:%d compound stmt has no "
			"key", __LINE__ - 1);
	      
	      result = P_GetStmtKey (parent);
	    }
	  else
	    {
	      result = PST_GetStmtScope (table, parent);
	    }
	}
    }
      
  return (result);
}

/*! \brief Finds the closest scope for an Expr.
 *
 * \param table
 *  the symbol table.
 * \param expr
 *  the expr to inspect.
 *
 * \return
 *  The key of the expr's scope.
 *
 * Recursively inspect parent stmts to find the expr's scope.
 *
 * \sa PST_GetGlobalScope(), PST_GetFileScope(),
 * #PST_GetFuncDclScope(), #PST_GetTypeDclScope(),
 * #PST_GetVarDclScope(), #PST_GetStructDclScope(),
 * #PST_GetUnionDclScope(), PST_GetStmtScope() */
Key
PST_GetExprScope (SymbolTable table, Expr expr)
{
  Key result = Invalid_Key;

  /* If the expr has a key, we already know its scope. */
  if (P_ValidKey (P_GetExprKey (expr)))
    {
      result = PST_GetScopeFromEntryKey (table, P_GetExprKey (expr));
    }
  else
    {
      Expr cur_expr = expr;
      Stmt parent_stmt;
      VarDcl parent_var;

      while (P_GetExprParentStmt (cur_expr) == NULL && \
	     P_GetExprParentVar (cur_expr) == NULL && \
	     P_GetExprParentExpr (cur_expr) != NULL)
	cur_expr = P_GetExprParentExpr (cur_expr);
	
      if ((parent_stmt = P_GetExprParentStmt (cur_expr)))
	result = PST_GetStmtScope (table, parent_stmt);
      else if ((parent_var = P_GetExprParentVar (cur_expr)))
	result = PST_GetVarDclScope (table, parent_var);
      else
	P_punt ("symtab.c:PST_GetExprScope:%d expr has no parent stmt or var",
		__LINE__ - 1);
    }

  return (result);
}

/* Stmt access functions. */
/*! \brief Create an enclosing compound statement if needed.
 *
 * \param table
 *  the symbol table.
 * \param stmt
 *  the ::Stmt to enclose.
 *
 * \return
 *  If \a stmt is already enclosed in a compound statement, returns FALSE.
 *  Otherwise, returns TRUE.
 *
 * If \a stmt is already enclosed in a compound statement, returns FALSE.
 * Otherwise, a compound statement is created and substituted for \a stmt.
 * \a stmt is moved inside the compound.
 *
 * (SK) Given a stmt, create a compound stmt around it if one doesn't
 * already exist.  This routine can be very useful when one has to add
 * a new statement before or after another statement, but the other
 * statement may happen to be the only statement within, say, a loop
 * and hence may not have a compd.  stmt enclosing it
 *  
 * return TRUE if the compound statement was actually needed, and
 * FALSE if no compd. needed to be created.  This return value need
 * not be used, it is provided simply as a check.
 */
bool
PST_StmtEncloseInCompound (SymbolTable table, Stmt stmt)
{
  Stmt parent_stmt = P_GetStmtParentStmt (stmt);
  Stmt new_compound_stmt;

  /* Check if stmt is already enclosed in a compound. */
  if (parent_stmt && P_GetStmtType (parent_stmt) == ST_COMPOUND)
    {
      return (FALSE);
    }
  else if (parent_stmt)
    {
      /* Parent stmt is not a compound, so create a new compound. */

      new_compound_stmt = PST_NewCompoundStmt (table, 
					       PST_GetStmtScope (table, stmt));

      /* Move stmt's labels to the new statement. */
      P_SetStmtLabels (new_compound_stmt, P_GetStmtLabels (stmt));
      P_SetStmtLabels (stmt, NULL);
      
      P_SetCompoundStmtList (P_GetStmtCompound (new_compound_stmt), stmt);

      P_StmtUpdate (parent_stmt, stmt, new_compound_stmt);
    }
  else if (P_GetStmtParentFunc (stmt))
    P_punt ("struct.c:P_StmtEncloseInCompound:%d FuncDcl stmt must be "
	    "ST_COMPOUND", __LINE__ - 1);
  else if (P_GetStmtParentExpr (stmt))
    P_punt ("struct.c:P_StmtEncloseInCompound:%d statement Expr stmt must "
	    "be ST_COMPOUND", __LINE__ - 1);
  else
    P_punt ("struct.c:P_StmtEncloseInCompound:%d stmt parent is NULL",
	    __LINE__ - 1);

  return (TRUE);
}

/*! \brief Inserts an expression after a statement.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the statement to insert after.
 * \param e
 *  the expression to insert after the statement.
 *
 * Inserts an ST_EXPR statement containing \a e after \a s, enclosing
 * both in a compound statement if necessary.
 *
 * \sa PST_StmtInsertExprBefore(), PST_StmtInsertExprBeforeLabel(),
 * PST_StmtInsertStmtAfter(), PST_StmtInsertStmtBefore(),
 * PST_StmtInsertStmtBeforeLabel() */
void
PST_StmtInsertExprAfter (SymbolTable table, Stmt s, Expr e)
{
  PST_StmtEncloseInCompound (table, s);
  P_StmtInsertExprAfter (s, e);

  return;
}

/*! \brief Inserts an expression before a statement.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the statement to insert before.
 * \param e
 *  the expression to insert after the statement.
 *
 * Inserts an ST_EXPR statement containing \a e before \a s, enclosing
 * both in a compound statement if necessary.  Labels attached
 * to \a s are moved to the new stmt.
 *
 * before:     LABEL: s;
 *
 * after:      LABEL: stmt containing e;
 *                    s;
 *
 * \sa PST_StmtInsertExprAfter(), PST_StmtInsertExprBeforeLabel(),
 * PST_StmtInsertStmtAfter(), PST_StmtInsertStmtBefore(),
 * PST_StmtInsertStmtBeforeLabel() */
void
PST_StmtInsertExprBefore (SymbolTable table, Stmt s, Expr e)
{
  PST_StmtEncloseInCompound (table, s);
  P_StmtInsertExprBefore (s, e);

  return;
}

/*! \brief Inserts an expression before a statement.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the statement to insert before.
 * \param e
 *  the expression to insert after the statement.
 *
 * Inserts an ST_EXPR statement containing \a e before \a s, enclosing
 * both in a compound statement if necessary.  Labels attached
 * to \a s remain on \a s.
 *
 * before:     LABEL: s;
 *
 * after:             stmt containing e;
 *             LABEL: s;
 *
 * \sa PST_StmtInsertExprAfter(), PST_StmtInsertExprBefore(),
 * PST_StmtInsertStmtAfter(), PST_StmtInsertStmtBefore(),
 * PST_StmtInsertStmtBeforeLabel() */
void
PST_StmtInsertExprBeforeLabel (SymbolTable table, Stmt s, Expr e)
{
  PST_StmtEncloseInCompound (table, s);
  P_StmtInsertExprBeforeLabel (s, e);

  return;
}

/*! \brief Inserts a statement after a statement.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the statement to insert after.
 * \param t
 *  the statement to insert after the statement.
 *
 * Inserts statement \a t after \a s, enclosing both in a compound
 * statement if necessary.
 *
 * \sa PST_StmtInsertExprAfter(), PST_StmtInsertExprBefore(),
 * PST_StmtInsertExprBeforeLabel(), PST_StmtInsertStmtBefore(),
 * PST_StmtInsertStmtBeforeLabel() */
void
PST_StmtInsertStmtAfter (SymbolTable table, Stmt s, Stmt t)
{
  PST_StmtEncloseInCompound (table, s);
  P_StmtInsertStmtAfter (s, t);

  return;
}

/*! \brief Inserts a statement before a statement.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the statement to insert before.
 * \param t
 *  the statement to insert after the statement.
 *
 * Inserts statement \a t before \a s, enclosing both in a compound
 * statement if necessary.  Labels attached to \a s are moved to the
 * new stmt.
 *
 * before:     LABEL: s;
 *
 * after:      LABEL: t;
 *                    s;
 *
 * \sa PST_StmtInsertExprAfter(), PST_StmtInsertExprBefore(),
 * PST_StmtInsertExprBeforeLabel(), PST_StmtInsertStmtAfter(),
 * PST_StmtInsertStmtBeforeLabel() */
void
PST_StmtInsertStmtBefore (SymbolTable table, Stmt s, Stmt t)
{
  PST_StmtEncloseInCompound (table, s);
  P_StmtInsertStmtBefore (s, t);

  return;
}

/*! \brief Inserts a statement before a statement.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the statement to insert before.
 * \param t
 *  the statement to insert after the statement.
 *
 * Inserts statement \a t before \a s, enclosing both in a compound
 * statement if necessary.  Labels attached to \a s remain on \a s.
 *
 * before:     LABEL: s;
 *
 * after:             t;
 *             LABEL: s;
 *
 * \sa PST_StmtInsertExprAfter(), PST_StmtInsertExprBefore(),
 * PST_StmtInsertExprBeforeLabel(), PST_StmtInsertStmtAfter(),
 * PST_StmtInsertStmtBefore() */
void
PST_StmtInsertStmtBeforeLabel (SymbolTable table, Stmt s, Stmt t)
{
  PST_StmtEncloseInCompound (table, s);
  P_StmtInsertStmtBeforeLabel (s, t);

  return;
}

/*! \addtogroup FindExternalSymbols Functions to find external symbols.
 *
 * These functions find external symbols referenced by a Pcode structure.
 *
 * These functions work by walking down the scope stack and following
 * symbol references back up to find external symbols.
 *
 * PST_FindFuncDclNeededSymbols(), PST_FindStmtNeededSymbols(), and
 * PST_FindExprNeededSymbols() start by walking down the syntax tree.
 * Whenever a key is found, the referenced symbol is inspected to
 * determine if it is external to a given scope.  If that symbol references
 * other symbols, the reference chain is followed to its end.  All external
 * symbols in the chain are added to the list of external symbols.
 */
/* @{ */
/*! \brief Finds the external symbols referenced by a file.
 *
 * \param table
 *  the symbol table.
 * \param f
 *  the file's key in the table.
 *
 * \return
 *  A list of the keys of external symbols referenced by file \a f.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
KeyList
PST_GetFileNeededSymbols (SymbolTable table, int f)
{
  KeyList known = NULL;
  _EntryType entry_types, e;
  SymTabEntry entry;
  Key k, file_scope;

  /* The scope is this file's global scope. */
  file_scope = PST_GetFileScope (table, f);

  entry_types = \
    ET_FUNC | ET_TYPE_GLOBAL | ET_VAR_GLOBAL | ET_STRUCT | ET_UNION;

  for (k = PST_GetFileEntryByType (table, f, entry_types); P_ValidKey (k);
       k = PST_GetFileEntryByTypeNext (table, k, entry_types))
    {
      entry = PST_GetSymTabEntry (table, k);

      switch ((e = P_GetSymTabEntryType (entry)))
	{
	case ET_FUNC:
	  {
	    FuncDcl func_dcl = P_GetSymTabEntryFuncDcl (entry);

	    known = get_func_dcl_scope_externs (table, file_scope, func_dcl,
						known);
	  }
	  break;

	case ET_TYPE_GLOBAL:
	  {
	    TypeDcl type_dcl = P_GetSymTabEntryTypeDcl (entry);
	    
	    known = get_type_dcl_scope_externs (table, file_scope, type_dcl,
						known);
	  }
	  break;

	case ET_VAR_GLOBAL:
	  {
	    VarDcl var_dcl = P_GetSymTabEntryVarDcl (entry);

	    known = get_var_dcl_scope_externs (table, file_scope, var_dcl,
					       known);
	  }
	  break;

	case ET_STRUCT:
	  {
	    StructDcl struct_dcl = P_GetSymTabEntryStructDcl (entry);

	    known = get_struct_dcl_scope_externs (table, file_scope,
						  struct_dcl, known);
	  }
	  break;

	case ET_UNION:
	  {
	    UnionDcl union_dcl = P_GetSymTabEntryUnionDcl (entry);

	    known = get_union_dcl_scope_externs (table, file_scope, union_dcl,
						 known);
	  }
	  break;

	default:
	  P_punt ("struct_symtab.c:PST_GetFileNeededSymbols:%d Invalid "
		  "SymTabEntry type %d", __LINE__ - 1, e);
	}
    }

  return (known);
}

/*! \brief Finds the external symbols referenced by a FuncDcl.
 *
 * \param table
 *  the symbol table.
 * \param f
 *  the FuncDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by \a f.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa #PST_GetFuncDclNeededSymbolsK() */
KeyList
PST_GetFuncDclNeededSymbols (SymbolTable table, FuncDcl f)
{
  return (get_func_dcl_scope_externs (table, PST_GetFuncDclScope (table, f), f,
				      NULL));
}

/*! \brief Finds the external symbols referenced by a TypeDcl.
 *
 * \param table
 *  the symbol table.
 * \param t
 *  the TypeDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by \a t.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa #PST_GetTypeDclNeededSymbolsK() */
KeyList
PST_GetTypeDclNeededSymbols (SymbolTable table, TypeDcl t)
{
  return (get_type_dcl_scope_externs (table, PST_GetTypeDclScope (table, t), t,
				      NULL));
}

/*! \brief Finds the external symbols referenced by a VarDcl.
 *
 * \param table
 *  the symbol table.
 * \param v
 *  the VarDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by \a v.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa #PST_GetVarDclNeededSymbolsK() */
KeyList
PST_GetVarDclNeededSymbols (SymbolTable table, VarDcl v)
{
  return (get_var_dcl_scope_externs (table, PST_GetVarDclScope (table, v), v,
				     NULL));
}

/*! \brief Finds the external symbols referenced by a StructDcl.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the StructDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by \a s.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa #PST_GetStructDclNeededSymbolsK() */
KeyList
PST_GetStructDclNeededSymbols (SymbolTable table, StructDcl s)
{
  return (get_struct_dcl_scope_externs (table,
					PST_GetStructDclScope (table, s), s,
					NULL));
}

/*! \brief Finds the external symbols referenced by a UnionDcl.
 *
 * \param table
 *  the symbol table.
 * \param u
 *  the UnionDcl to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by \a u.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa #PST_GetUnionDclNeededSymbolsK() */
KeyList
PST_GetUnionDclNeededSymbols (SymbolTable table, UnionDcl u)
{
  return (get_union_dcl_scope_externs (table, PST_GetUnionDclScope (table, u),
				       u, NULL));
}

/*! \brief Finds the external symbols referenced by a Stmt.
 *
 * \param table
 *  the symbol table.
 * \param s
 *  the Stmt to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by \a f.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
KeyList
PST_GetStmtNeededSymbols (SymbolTable table, Stmt s)
{
  return (get_stmt_scope_externs (table, PST_GetStmtScope (table, s), s,
				  NULL));
}

/*! \brief Finds the external symbols referenced by an Expr.
 *
 * \param table
 *  the symbol table.
 * \param e
 *  the Expr to inspect.
 *
 * \return
 *  A list of the keys of external symbols referenced by \a e.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 *
 * \sa PST_GetFileNeededSymbols(), PST_GetFuncDclNeededSymbols(),
 * PST_GetTypeDclNeededSymbols(), PST_GetVarDclNeededSymbols(),
 * PST_GetStructDclNeededSymbols(), PST_GetUnionDclNeededSymbols(),
 * PST_GetStmtNeededSymbols() */
KeyList
PST_GetExprNeededSymbols (SymbolTable table, Expr e)
{
  return (get_expr_scope_externs (table, PST_GetExprScope (table, e), e,
				  NULL));
}
/* @} */

/*! \brief Compares the TypeDcl.basic_type field.
 *
 * \param a, b
 *  the types to compare.
 *
 * \return
 *  If the Type.basic_type field is the same, returns 1.  Otherwise returns 0.
 */
static int
basic_types_match (TypeDcl a, TypeDcl b)
{
  return (P_GetTypeDclBasicType (a) == P_GetTypeDclBasicType (b));
}

/*! \brief Compares the TypeDcl.qualifier field.
 *
 * \param a, b
 *  the types to compare.
 *
 * \return
 *  If the TypeDcl.qualifier field is the same, returns 1.  Otherwise
 *  returns 0.
 */
static int
qualifiers_match (TypeDcl a, TypeDcl b)
{
  return (P_GetTypeDclQualifier (a) == P_GetTypeDclQualifier (b));
}

/*! \brief Compares the TypeDcl.key.file field.
 *
 * \param a, b
 *  the types to compare.
 *
 * \return
 *  If the TypeDcl.key.file field is the same, returns 1.  Otherwise returns 0.
 *
 * The TypeDcl.key.file field is considered the same if:
 * \li it is undefined in at least one type, or
 * \li it is defined and has the same value in both types.
 */
static int
files_match (TypeDcl a, TypeDcl b)
{
  Key keyA = P_GetTypeDclKey (a);
  Key keyB = P_GetTypeDclKey (b);

  return (keyA.file == 0 || keyB.file == 0 || keyA.file == keyB.file);
}

/*! \brief Compares the TypeDcl.type field.
 *
 * \param a, b
 *  the types to compare.
 *
 * \return
 *  If the TypeDcl.type field is the same, returns 1.  Otherwise returns 0.
 */
static int
types_match (TypeDcl a, TypeDcl b)
{
  return (P_MatchKey (P_GetTypeDclType (a), P_GetTypeDclType (b)));
}

/*! \brief Compares the TypeDcl.name field.
 *
 * \param a, b
 *  the types to compare.
 *
 * \return
 *  If the TypeDcl.name field is the same, returns 1.  Otherwise returns 0.
 *
 * The TypeDcl.name field is considered the same if they are both defined
 * to the same string or if they are both undefined.
 */
static int
names_match (TypeDcl a, TypeDcl b)
{
  char *nameA = P_GetTypeDclName (a);
  char *nameB = P_GetTypeDclName (b);

  return ((nameA && nameB && strcmp (nameA, nameB) == 0) || \
	  (nameA == NULL && nameB == NULL));
}

/*! \brief Compares the TypeDcl.alignment field.
 *
 * \param a, b
 *  the types to compare.
 *
 * \return
 *  If the TypeDcl.alignment field is the same, returns 1.  Otherwise
 *  returns 0.
 *
 * The TypeDcl.alignment fields match if they are both defined to the same
 * value or if both types have TY_DEFAULT set in TypeDcl.qualifier
 */
static int
alignments_match (TypeDcl a, TypeDcl b)
{
  return ((P_GetTypeDclAlignment (a) == P_GetTypeDclAlignment (b)) || \
	  (P_GetTypeDclQualifier (a) & TY_DEFAULT && \
	   P_GetTypeDclQualifier (b) & TY_DEFAULT));
}

/*! \brief Returns the destination scope corresponding to one in the source.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param src_scope
 *  the key of a scope in \a src_table.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a src_scope.
 *
 * \return
 *  the key of the scope in \a dst_table corresponding to \a src_scope.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 */
static Key
get_dst_scope (SymbolTable dst_table, SymbolTable src_table, Key src_scope,
	       bool preserve)
{
  Key global_scope, dst_scope = Invalid_Key;

  /* If we are copying within a table or if we are preserving the old key,
   * the destination scope will be the same as the source. */
  if ((dst_table == src_table) ^ (preserve == TRUE))
    {
      dst_scope = src_scope;
    }
  else
    {
      global_scope = PST_GetGlobalScope (src_table);

      if (P_MatchKey (src_scope, global_scope))
	{
	  dst_scope = PST_GetGlobalScope (dst_table);
	}
      else
	{
	  dst_scope = PST_GetNewKey (src_table, src_scope);
	  
	  if (!P_ValidKey (dst_scope))
	    P_punt ("struct_symtab.c:get_dst_scope:%d Unable to find scope "
		    "for (%d, %d) in dst_table.", __LINE__ - 1,
		    src_scope.file, src_scope.sym);
	}
    }
	
  return (dst_scope);
}

/*! \brief Copies a FuncDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new FuncDcl will be created (pass Invalid_Key to
 *  use same scope as original FuncDcl).
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the FuncDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 * 
 * \return
 *  A copy of the FuncDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a f.
 *
 * This function does the real work of PST_CopyFuncDcl() and
 * PST_TableCopyFuncDcl().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of symbol table entries that have been
 * tagged with a KeyMap structure is returned as \a clean.  These must
 * be cleaned after the copy is complete.
 *
 * FuncDcl.max_expr_id is not copied.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyFuncDcl(), PST_TableCopyFuncDcl() */
static FuncDcl
copy_func_dcl (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	       FuncDcl f, bool preserve, KeyList *clean)
{
  FuncDcl new = NULL;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (f)
    {
      new = P_NewFuncDcl ();

      if (P_GetFuncDclName (f))
	{
	  if (P_GetFuncDclName (new))
	    free (P_GetFuncDclName (new));

	  P_SetFuncDclName (new, strdup (P_GetFuncDclName (f)));
	}

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = f->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      P_SetFuncDclKey (new, dst_key);

      /* Remove the pre-existing entry with FuncDcl's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new FuncDcl to the symbol table. */
      dst_key = PST_AddFuncDclEntry (dst_table, new);

      /* Add the FuncDcl to the global scope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);

      /* Add a new scope for this FuncDcl. */
      PST_AddNewScope (dst_table, dst_key);

      /* Set the FuncDcl's new key in the symbol table. */
      PST_SetNewKey (src_table, P_GetFuncDclKey (f), dst_key, clean);

#if 0
      /* Each function should refer to a distinct BT_FUNC type. */
      func_type_dcl = PST_GetTypeDclEntry (src_table, P_GetFuncDclType (f));
      new_func_type_dcl = copy_type_dcl (dst_table, dst_scope, src_table,
					 func_type_dcl, preserve, clean);

      /* Write the type key to the FuncDcl in case the type will not be
       * copied. */
      PST_SetFuncDclType (dst_table, new, P_GetTypeDclKey (new_func_type_dcl));
#endif
      /* The type key will be updated in update_func_dcl_keys(), so use
       * the old type key for now. */
      P_SetFuncDclType (new, P_GetFuncDclType (f));
       
      P_SetFuncDclLineno (new, P_GetFuncDclLineno (f));
      P_SetFuncDclColno (new, P_GetFuncDclColno (f));
      P_SetFuncDclQualifier (new, P_GetFuncDclQualifier (f));
      if (P_GetFuncDclFilename (f))
	P_SetFuncDclFilename (new, strdup (P_GetFuncDclFilename (f)));
      P_SetFuncDclParam (new, copy_var_list (dst_table, dst_key, src_table,
					     P_GetFuncDclParam (f), preserve,
					     clean));
      P_SetFuncDclStmt (new, copy_stmt (dst_table, dst_key, src_table,
					P_GetFuncDclStmt (f), preserve,
					clean));
      P_SetFuncDclPragma (new,
			  copy_pragma (dst_table, dst_scope, src_table,
				       P_GetFuncDclPragma (f), preserve,
				       clean));
      P_SetFuncDclProfile (new, P_CopyProfFN (P_GetFuncDclProfile (f)));
      P_SetFuncDclShadow (new, P_CopyShadow (P_GetFuncDclShadow (f)));

      /* FuncDcl.max_expr_id is not copied.  It will be rebuilt as the
       * copied expressions get IDs assigned. */

      if (Handlers[ES_FUNC])
	{
	  for (i = 0; i < NumExtensions[ES_FUNC]; i++)
	    {
	      if (P_GetFuncDclExtL (f, i) && \
		  (copy = Handlers[ES_FUNC][i].copy))
		{
		  if (P_GetFuncDclExtL (new, i))
		    {
		      FreeHandler ext_free = Handlers[ES_FUNC][i].free;
		      P_SetFuncDclExtL (new, i,
					ext_free (P_GetFuncDclExtL (new, i)));
		    }

		  P_SetFuncDclExtL (new, i, copy (P_GetFuncDclExtL (f, i)));
		}
	    }
	}

    }

  return (new);
}

/*! \brief Copies a TypeDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which new TypeDcl will be created.
 * \param src_table
 *  the source symbol table.
 * \param t
 *  the TypeDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the TypeDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a t.
 *
 * This function does the real work of PST_CopyTypeDcl() and
 * PST_TableCopyTypeDcl().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyTypeDcl(), PST_TableCopyTypeDcl() */
static TypeDcl
copy_type_dcl (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	       TypeDcl t, bool preserve, KeyList *clean)
{
  TypeDcl new = NULL;
  CopyHandler copy;
  _EntryType local_or_global;
  _BasicType bt;
  Key dst_key = Invalid_Key;
  int i;

  if (t)
    {
      bt = P_GetTypeDclBasicType (t);

      new = P_NewTypeDcl ();

      P_SetTypeDclBasicType (new, P_GetTypeDclBasicType (t));
      P_SetTypeDclQualifier (new, P_GetTypeDclQualifier (t));

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = t->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      P_SetTypeDclKey (new, dst_key);

      /* A TypeDcl may have an entry type of ET_TYPE_LOCAL or ET_TYPE_GLOBAL.
       * Retrieve the entry type so we can update the type key. */
      local_or_global = \
	P_GetSymTabEntryType (PST_GetSymTabEntry (src_table,
						  P_GetTypeDclKey (t)));
#if 0
      /* Make sure the referenced type exists in the destination table. */
      if (P_ValidKey (P_GetTypeDclType (t)) && !(bt & (BT_STRUCT | BT_UNION)))
	{
	  refd_type_dcl = PST_GetTypeDclEntry (src_table,
					       P_GetTypeDclType (t));
	  refd_type_key = PST_ScopeFindTypeDcl (dst_table, dst_scope,
						refd_type_dcl);
	}

      PST_SetTypeDclType (dst_table, new, refd_type_key);
#endif
      /* The type key will be updated in update_type_dcl_keys(), so use
       * the old type key for now. */
      P_SetTypeDclType (new, P_GetTypeDclType (t));

      if (P_GetTypeDclName (t))
	P_SetTypeDclName (new, strdup (P_GetTypeDclName (t)));

      if (bt & BT_ARRAY)
	P_SetTypeDclArraySize (new, copy_expr_list (dst_table, dst_scope,
						    src_table,
						    P_GetTypeDclArraySize (t),
						    preserve,
						    clean));
      if (bt & BT_FUNC)
	{
	  Param p;

	  /* Make sure the parameter types exist in the destination table. */
	  for (p = P_GetTypeDclParam (t); p; p = P_GetParamNext (p))
	    {
#if 0
	      refd_type_dcl = PST_GetTypeDclEntry (src_table,
						   P_GetParamKey (p));
	      refd_type_key = PST_ScopeFindTypeDcl (dst_table, dst_scope,
						    refd_type_dcl);
#endif
	      P_SetTypeDclParam \
		(new,
		 P_AppendParamNext (P_GetTypeDclParam (new),
				    P_NewParamWithKey (P_GetParamKey (p))));
	    }
	}

      P_SetTypeDclSize (new, P_GetTypeDclSize (t));
      P_SetTypeDclAlignment (new, P_GetTypeDclAlignment (t));
      P_SetTypeDclLineno (new, P_GetTypeDclLineno (t));
      P_SetTypeDclColno (new, P_GetTypeDclColno (t));
      if (P_GetTypeDclFilename (t))
	P_SetTypeDclFilename (new, strdup (P_GetTypeDclFilename (t)));
      P_SetTypeDclPragma (new, copy_pragma (dst_table, dst_scope, src_table,
					    P_GetTypeDclPragma (t), preserve,
					    clean));

      if (Handlers[ES_TYPE])
	{
	  for (i = 0; i < NumExtensions[ES_TYPE]; i++)
	    {
	      if (P_GetTypeDclExtL (t, i) && \
		  (copy = Handlers[ES_TYPE][i].copy))
		{
		  if (P_GetTypeDclExtL (new, i))
		    {
		      FreeHandler ext_free = Handlers[ES_TYPE][i].free;
		      P_SetTypeDclExtL (new, i,
					ext_free (P_GetTypeDclExtL (new, i)));
		    }

		  P_SetTypeDclExtL (new, i, copy (P_GetTypeDclExtL (t, i)));
		}
	    }
	}

      /* Remove the pre-existing entry with TypeDcl's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new TypeDcl to the symbol table. */
      dst_key = PST_AddTypeDclEntry (dst_table, new, local_or_global);

      /* Add the new TypeDcl to its scope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);

      /* Set the TypeDcl's new key in the symbol table. */
      PST_SetNewKey (src_table, P_GetTypeDclKey (t), dst_key, clean);
    }
  
  return (new);
}

/*! \brief Copies a VarList and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new VarList will be created.
 * \param src_table
 *  the source symbol table.
 * \param l
 *  the VarList (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the VarList in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a l.
 *
 * A list of keys of the symbol table entries that have been tagged
 * with a KeyMap structure is returned as \a clean.  These must be
 * cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 */
static VarList
copy_var_list (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	       VarList l, bool preserve, KeyList *clean)
{
  VarList new = NULL;
  VarDcl v;
  
  for (List_start (l), v = (VarDcl)List_next (l); v; v = (VarDcl)List_next (l))
    new = List_insert_last (new, copy_var_dcl (dst_table, dst_scope,
					       src_table, v, preserve, clean));
  
  return (new);
}

/*! \brief Copies a VarDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new VarDcl will be created.
 * \param src_table
 *  the source symbol table.
 * \param v
 *  the VarDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the VarDcl in \a dst_table.  \a dst_table is updated so that the
 *  copy is completely distinct from \a v.
 *
 * This function does the real work of PST_CopyVarDcl() and
 * PST_TableCopyVarDcl().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyVarDcl(), PST_TableCopyVarDcl() */
static VarDcl
copy_var_dcl (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	      VarDcl v, bool preserve, KeyList *clean)
{
  VarDcl new = NULL;
  CopyHandler copy;
  _EntryType local_or_global;
  Key dst_key = Invalid_Key;
  int i;

  if (v)
    {
      new = P_NewVarDcl ();

      if (P_GetVarDclName (v))
	P_SetVarDclName (new, strdup (P_GetVarDclName (v)));

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = v->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      P_SetVarDclKey (new, dst_key);

      /* A VarDcl may have an entry type of ET_VAR_LOCAL or ET_VAR_GLOBAL.
       * Retrieve the entry type so we can update the type key. */
      local_or_global = \
	P_GetSymTabEntryType (PST_GetSymTabEntry (src_table,
						  P_GetVarDclKey (v)));

#if 0
      /* Make sure the referenced type exists in the destination table. */
      refd_type_dcl = PST_GetTypeDclEntry (src_table, P_GetVarDclType (v));
      refd_type_key = PST_ScopeFindTypeDcl (dst_table, dst_scope,
					    refd_type_dcl);

      PST_SetVarDclType (dst_table, new, refd_type_key);
#endif
      /* The type key will be updated in update_var_dcl_keys(), so use
       * the old type key for now. */
      P_SetVarDclType (new, P_GetVarDclType (v));

      P_SetVarDclInit (new, copy_init (dst_table, dst_scope, src_table,
				       P_GetVarDclInit (v), preserve, clean));
      P_SetVarDclLineno (new, P_GetVarDclLineno (v));
      P_SetVarDclColno (new, P_GetVarDclColno (v));
      P_SetVarDclAlign (new, P_GetVarDclAlign (v));
      P_SetVarDclQualifier (new, P_GetVarDclQualifier (v));
      if (P_GetVarDclFilename (v))
	P_SetVarDclFilename (new, strdup (P_GetVarDclFilename (v)));
      P_SetVarDclPragma (new,
			 copy_pragma (dst_table, dst_scope, src_table,
				      P_GetVarDclPragma (v), preserve, clean));

      if (Handlers[ES_VAR])
	{
	  for (i = 0; i < NumExtensions[ES_VAR]; i++)
	    {
	      if (P_GetVarDclExtL (v, i) && (copy = Handlers[ES_VAR][i].copy))
		{
		  if (P_GetVarDclExtL (new, i))
		    {
		      FreeHandler ext_free = Handlers[ES_VAR][i].free;
		      P_SetVarDclExtL (new, i,
				       ext_free (P_GetVarDclExtL (new, i)));
		    }

		  P_SetVarDclExtL (new, i, copy (P_GetVarDclExtL (v, i)));
		}
	    }
	}

      /* Remove the pre-existing entry with VarDcl's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new VarDcl to the symbol table. */
      dst_key = PST_AddVarDclEntry (dst_table, new, local_or_global);

      /* Add the new VarDcl to the scope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);

      /* Set the VarDcl's new key in the symbol table. */
      PST_SetNewKey (src_table, P_GetVarDclKey (v), dst_key, clean);
    }

  return (new);
}

/*! \brief Copies an Init and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Init will be created.
 * \param src_table
 *  the source symbol table.
 * \param i
 *  the Init (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Init in \a dst_table.  \a src_table is updated so that the
 *  copy is completely distinct from \a i.
 *
 * A list of keys of the symbol table entries that have been tagged with
 * a KeyMap structure is returned as \a clean.  These must be cleaned after
 * the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.nn
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 */
static Init
copy_init (SymbolTable dst_table, Key dst_scope, SymbolTable src_table, Init i,
	   bool preserve, KeyList *clean)
{
  Init new = NULL;
  CopyHandler copy;
  int j;

  if (i)
    {
      new = P_NewInit ();

      new->expr = copy_expr_list (dst_table, dst_scope, src_table, i->expr,
				  preserve, clean);
      new->set = copy_init (dst_table, dst_scope, src_table, i->set, preserve,
			    clean);
      new->next = copy_init (dst_table, dst_scope, src_table, i->next,
			     preserve, clean);

      if (Handlers[ES_INIT])
	{
	  for (j = 0; j < NumExtensions[ES_INIT]; j++)
	    {
	      if (i->ext[j] && (copy = Handlers[ES_INIT][j].copy))
		{
		  if (new->ext[j])
		    {
		      FreeHandler ext_free = Handlers[ES_INIT][j].free;
		      new->ext[j] = ext_free (new->ext[j]);
		    }
		    
		  new->ext[j] = copy (i->ext[j]);
		}
	    }
	}
    }

  return (new);    
}

/*! \brief Copies a StructDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new StructDcl will be created.
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the StructDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the StructDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a s.
 *
 * This function does the real work of PST_CopyStructDcl() and
 * PST_TableCopyStructDcl().  If \a dst_key is valid, it is used as
 * the copy's key.  A list of keys of the symbol table entries that
 * have been tagged with a KeyMap structure is returned as \a clean.
 * These must be cleaned after the copy is complete.
 *
 * If \a dst_table and \a src_table are the same, the copy is added to the
 * \a s->next list of alternative StructDcls with the same name.  If
 * \a preserve is true, next_alt and prev_alt will be copied verbatim.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyStructDcl(), PST_TableCopyStructDcl() */
static StructDcl
copy_struct_dcl (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
		 StructDcl s, bool preserve, KeyList *clean)
{
  StructDcl new = NULL;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (s)
    {
      new = P_NewStructDcl ();

      if (s->name)
	new->name = strdup (s->name);

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = s->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

      new->qualifier = s->qualifier;

      new->fields = copy_field (dst_table, dst_scope, src_table, s->fields,
				preserve, clean);

      new->lineno = s->lineno;
      if (s->filename)
	new->filename = strdup (s->filename);
      new->size = s->size;
      new->align = s->align;
      new->group = s->group;

      new->pragma = copy_pragma (dst_table, dst_scope, src_table, s->pragma,
				 preserve, clean);

      if (Handlers[ES_STRUCT])
	{
	  for (i = 0; i < NumExtensions[ES_STRUCT]; i++)
	    {
	      if (s->ext[i] && (copy = Handlers[ES_STRUCT][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_STRUCT][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }

		  new->ext[i] = copy (s->ext[i]);
		}
	    }
	}

      /* Remove the pre-existing entry with StructDcl's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new StructDcl to the symbol table. */
      dst_key = PST_AddStructDclEntry (dst_table, new);

      /* Add the StructDcl to the scope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);

      /* Set the StructDcl's new key in the symbol table. */
      PST_SetNewKey (src_table, P_GetStructDclKey (s), dst_key, clean);
    }

  return (new);
}

/*! \brief Copies a UnionDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new UnionDcl will be created.
 * \param src_table
 *  the source symbol table.
 * \param u
 *  the UnionDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the UnionDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a u.
 *
 * This function does the real work of PST_CopyUnionDcl() and
 * PST_TableCopyUnionDcl().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * If \a dst_table and \a src_table are the same, the copy is added to the
 * \a s->next list of alternative UnionDcls with the same name.  If
 * \a preserve is true, next_alt and prev_alt will be copied verbatim.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyUnionDcl(), PST_TableCopyUnionDcl() */
static UnionDcl
copy_union_dcl (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
		UnionDcl u, bool preserve, KeyList *clean)
{
  UnionDcl new = NULL;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (u)
    {
      new = P_NewUnionDcl ();

      if (u->name)
	new->name = strdup (u->name);

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = u->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

      new->qualifier = u->qualifier;

      new->fields = copy_field (dst_table, dst_scope, src_table, u->fields,
				preserve, clean);

      new->lineno = u->lineno;
      if (u->filename)
	new->filename = strdup (u->filename);
      new->size = u->size;
      new->align = u->align;
      new->group = u->group;

      new->pragma = copy_pragma (dst_table, dst_scope, src_table, u->pragma,
				 preserve, clean);

      if (Handlers[ES_UNION])
	{
	  for (i = 0; i < NumExtensions[ES_UNION]; i++)
	    {
	      if (u->ext[i] && (copy = Handlers[ES_UNION][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_UNION][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }

		  new->ext[i] = copy (u->ext[i]);
		}
	    }
	}

      /* Remove the pre-existing entry with UnionDcl's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new UnionDcl to the symbol table. */
      dst_key = PST_AddUnionDclEntry (dst_table, new);

      /* Add the UnionDcl to the scope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);

      /* Set the UnionDcl's new key in the symbol table. */
      PST_SetNewKey (src_table, P_GetUnionDclKey (u), dst_key, clean);
    }

  return (new);
}

/*! \brief Copies a Field and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Field will be created.
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the Field (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Field in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a f.
 *
 * This function does the real work of PST_CopyField() and
 * PST_TableCopyField().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyField(), PST_TableCopyField() */
static Field
copy_field (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	    Field f, bool preserve, KeyList *clean)
{
  Field new = NULL;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (f)
    {
      new = P_NewField ();

      if (f->name)
	new->name = strdup (f->name);

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = f->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

#if 0
      /* Make sure the referenced type exists in the destination table. */
      refd_type_dcl = PST_GetTypeDclEntry (src_table, P_GetFieldType (f));
      refd_type_key = PST_ScopeFindTypeDcl (dst_table, dst_scope,
					    refd_type_dcl);

      PST_SetFieldType (dst_table, new, refd_type_key);
#endif
      P_SetFieldParentKey (new, P_GetFieldParentKey (f));

      /* The type key will be updated in update_field_keys() so use the old
       * type key for now. */
      P_SetFieldType (new, P_GetFieldType (f));

      new->is_bit_field = f->is_bit_field;
      new->bit_size = f->bit_size;
      new->bit_offset_remainder = f->bit_offset_remainder;
      new->next = copy_field (dst_table, dst_scope, src_table, f->next,
			      preserve, clean);
      new->offset = f->offset;
      new->pragma = copy_pragma (dst_table, dst_scope, src_table, f->pragma,
				 preserve, clean);

      if (Handlers[ES_FIELD])
	{
	  for (i = 0; i < NumExtensions[ES_FIELD]; i++)
	    {
	      if (f->ext[i] && (copy = Handlers[ES_FIELD][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_FIELD][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }

		  new->ext[i] = copy (f->ext[i]);
		}
	    }
	}

      /* Remove the pre-existing entry with Field's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new Field to the symbol table. */
      dst_key = PST_AddFieldEntry (dst_table, new);

      /* Add the Field to the scope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);

      /* Set the Field's new key in the symbol table. */
      PST_SetNewKey (src_table, P_GetFieldKey (f), dst_key, clean);
    }

  return (new);
}

/*! \brief Copies an EnumDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new EnumDcl will be created.
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the EnumDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the EnumDcl in \a dst_table.  \a dst_table is updated so that
 *   the copy is completely distinct from \a e.
 *
 * This function does the real work of PST_CopyEnumDcl() and
 * PST_TableCopyEnumDcl().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyEnumDcl(), PST_TableCopyEnumDcl() */
static EnumDcl
copy_enum_dcl (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	       EnumDcl e, bool preserve, KeyList *clean)
{
  EnumDcl new = NULL;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (e)
    {
      new = P_NewEnumDcl ();

      if (e->name)
	new->name = strdup (e->name);

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = e->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

      new->fields = copy_enum_field (dst_table, dst_scope, src_table,
				     e->fields, preserve, clean);

      new->lineno = e->lineno;
      new->colno = e->colno;
      if (e->filename)
	new->filename = strdup (e->filename);
      new->pragma = copy_pragma (dst_table, dst_scope, src_table, e->pragma,
				 preserve, clean);

      if (Handlers[ES_ENUM])
	{
	  for (i = 0; i < NumExtensions[ES_ENUM]; i++)
	    {
	      if (e->ext[i] && (copy = Handlers[ES_ENUM][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_ENUM][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }

		  new->ext[i] = copy (e->ext[i]);
		}
	    }
	}

      /* Remove the pre-existing entry with EnumDcl's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new EnumDcl to the symbol table. */
      dst_key = PST_AddEnumDclEntry (dst_table, new);

      /* Add the EnumDcl to the scope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);

      /* Set the EnumDcl's new key in the symbol table. */
      PST_SetNewKey (src_table, P_GetEnumDclKey (e), dst_key, clean);
    }

  return (new);
}

/*! \brief Copies an EnumField and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new EnumField will be created.
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the EnumField (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the EnumField in \a dst_table.  \a dst_table is updated so
 *  that the copy is completely distinct from \a f.
 *
 * This function does the real work of PST_CopyEnumField() and
 * PST_TableCopyEnumField().  A list of keys of the symbol table
 * entries that have been tagged with a KeyMap structure is returned
 * as \a clean.  These must be cleaned after the copy is complete.
 *
 * \note \a dst_table and src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyEnumField(), PST_TableCopyEnumField() */
static EnumField
copy_enum_field (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
		 EnumField f, bool preserve, KeyList *clean)
{
  EnumField new = NULL;
  Key dst_key = Invalid_Key;
  
  if (f)
    {
      new = P_NewEnumField ();

      if (f->name)
	new->name = strdup (f->name);

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = f->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

      new->value = copy_expr_list (dst_table, dst_scope, src_table, f->value,
				   preserve, clean);
      new->next = copy_enum_field (dst_table, dst_scope, src_table, f->next,
				   preserve, clean);

      /* Remove the pre-existing entry with EnumField's key if necessary. */
      if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	PST_RemoveEntry (dst_table, dst_key);

      /* Add the new EnumField to the symbol table. */
      dst_key = PST_AddEnumFieldEntry (dst_table, new);

      /* Add the EnumField to thes cope. */
      PST_AddEntryToScope (dst_table, dst_scope, dst_key);
    }

  return (new);
}

/*! \brief Copies a single Stmt and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Stmt will be created.
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the Stmt (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Stmt in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a s.
 *
 * Only the given Stmt is copied, not the entire list.
 *
 * ::Stmt.lex_prev, ::Stmt.lex_next, ::Stmt.parent, ::Stmt.parent_func,
 * and ::Stmt.parent_expr are set to null in the new Stmt.
 *
 * This function does the real work of PST_CopyStmtNode() and
 * PST_TableCopyStmtNode().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyStmtNode(), PST_TableCopyStmtNode() */
static Stmt
copy_stmt_node (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
		Stmt s, bool preserve, KeyList *clean)
{
  Stmt new = NULL;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (s)
    {
      new = P_NewStmt ();

      new->type = s->type;

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = s->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

      /* Add the new Stmt to the symbol table if necessary. */
      if (dst_key.file != 0)
	{
	  /* Remove the pre-existing entry with Stmt's key if necessary. */
	  if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	    PST_RemoveEntry (dst_table, dst_key);
	  
	  dst_key = PST_AddStmtEntry (dst_table, new);

	  /* Add the Stmt to the scope. */
	  PST_AddEntryToScope (dst_table, dst_scope, dst_key);

	  /* Add a new scope for the Stmt if necessary. */
	  if (P_GetStmtType (s) == ST_COMPOUND)
	    PST_AddNewScope (dst_table, dst_key);
	}	  

      new->status = s->status;
      new->lineno = s->lineno;
      new->colno = s->colno;
      new->artificial = s->artificial;
      new->foroverlap = s->foroverlap;
      if (s->filename)
	new->filename = strdup (s->filename);
      new->profile = P_CopyProfST (s->profile);
      new->pragma = copy_pragma (dst_table, dst_scope, src_table, s->pragma,
				 preserve, clean);

      /* Copy the shadow, if it exists. */
      if (s->shadow)
	new->shadow = P_NewShadowWithExprID (new->shadow, P_GetStmtExpr (new),
					     s->shadow->param_id);

      new->labels = copy_label (dst_table, dst_scope, src_table, s->labels,
				preserve, clean);

      if (new->labels)
	P_SetLabelParentStmtAll (new->labels, new);

      switch (s->type)
	{
	case ST_RETURN:
	  P_SetStmtRet (new, copy_expr_list (dst_table, dst_scope, src_table,
					     P_GetStmtRet (s), preserve,
 					     clean));
	  break;

	case ST_GOTO:
	  {
	    char *val;

	    if ((val = P_GetStmtLabelVal (s)))
	      P_SetStmtLabelVal (new, strdup (val));

	    /* Write the label key to the goto in case the label will not be
	     * copied. */
	    P_SetStmtLabelKey (new, P_GetStmtLabelKey (s));
	  }
	  break;

	case ST_COMPOUND:
	  {
	    Compound c, new_compound;
	    TypeList tl, new_tl = NULL;
	    TypeDcl t;

	    if ((c = P_GetStmtCompound (s)))
	      {
		new_compound = P_NewCompound ();
		/* We inserted a new scope for this statement above, so use
		 * dst_key as the scope. */

		tl = P_GetCompoundTypeList (c);
		for (List_start (tl), t = (TypeDcl)List_next (tl); t;
		     t = (TypeDcl)List_next (tl))
		  {
		    new_tl = \
		      List_insert_last (new_tl,
					copy_type_dcl (dst_table, dst_key,
						       src_table, t, preserve,
						       clean));
		  }
		P_SetCompoundTypeList (new_compound, new_tl);

		P_SetCompoundVarList (new_compound,
				      copy_var_list (dst_table, dst_key,
						     src_table,
						     P_GetCompoundVarList (c),
						     preserve, clean));

		P_SetCompoundStmtList (new_compound,
				       copy_stmt (dst_table, dst_key,
						  src_table,
						  P_GetCompoundStmtList (c),
						  preserve, clean));

		P_SetStmtCompound (new, new_compound);
	      }
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i, new_ifstmt;

	    if ((i = P_GetStmtIfStmt (s)))
	      {
		new_ifstmt = P_NewIfStmt ();

		P_SetIfStmtCondExpr (new_ifstmt,
				     copy_expr_list (dst_table, dst_scope,
						     src_table,
						     P_GetIfStmtCondExpr (i),
						     preserve, clean));
		P_SetIfStmtThenBlock (new_ifstmt,
				      copy_stmt (dst_table, dst_scope,
						 src_table,
						 P_GetIfStmtThenBlock (i),
						 preserve, clean));
		P_SetIfStmtElseBlock (new_ifstmt,
				      copy_stmt (dst_table, dst_scope,
						 src_table,
						 P_GetIfStmtElseBlock (i),
						 preserve, clean));

		P_SetStmtIfStmt (new, new_ifstmt);
	      }
	  }
	  break;

	case ST_SWITCH:
	  {
	    SwitchStmt t, new_switchstmt;

	    if ((t = P_GetStmtSwitchStmt (s)))
	      {
		new_switchstmt = P_NewSwitchStmt ();

		P_SetSwitchStmtExpression \
		  (new_switchstmt,
		   copy_expr_list (dst_table, dst_scope, src_table,
				   P_GetSwitchStmtExpression (t), preserve,
				   clean));
		P_SetSwitchStmtSwitchBody \
		  (new_switchstmt, copy_stmt (dst_table, dst_scope, src_table,
					      P_GetSwitchStmtSwitchBody (t),
					      preserve, clean));

		P_SetStmtSwitchStmt (new, new_switchstmt);
	      }
	  }
	  break;

	case ST_PSTMT:
	  P_SetStmtPstmt (new, copy_pstmt (dst_table, dst_scope, src_table,
					   P_GetStmtPstmt (s), preserve,
					   clean));
	  break;

	case ST_ADVANCE:
	  P_SetStmtAdvance (new, P_CopyAdvance (P_GetStmtAdvance (s)));
	  break;

	case ST_AWAIT:
	  P_SetStmtAwait (new, P_CopyAwait (P_GetStmtAwait (s)));
	  break;

	case ST_MUTEX:
	  {
	    Mutex m, new_mutex;

	    if ((m = P_GetStmtMutex (s)))
	      {
		new_mutex = P_NewMutex ();

		P_SetMutexExpression (new_mutex,
				      copy_expr_list (dst_table, dst_scope,
						      src_table,
						      P_GetMutexExpression (m),
						      preserve, clean));
		P_SetMutexStatement (new_mutex,
				     copy_stmt (dst_table, dst_scope,
						src_table,
						P_GetMutexStatement (m),
						preserve, clean));

		P_SetStmtMutex (new, new_mutex);
	      }
	  }
	  break;

	case ST_COBEGIN:
	  {
	    Cobegin c, new_cobegin;

	    if ((c = P_GetStmtCobegin (s)))
	      {
		new_cobegin = P_NewCobegin ();

		P_SetCobeginStatements (new_cobegin,
					copy_stmt (dst_table, dst_scope,
						   src_table,
						   P_GetCobeginStatements (c),
						   preserve, clean));

		P_SetStmtCobegin (new, new_cobegin);
	      }
	  }
	  break;

	case ST_PARLOOP:
	  {
	    ParLoop p, new_parloop;

	    if ((p = P_GetStmtParLoop (s)))
	      {
		new_parloop = P_NewParLoop ();

		P_SetParLoopLoopType (new_parloop, P_GetParLoopLoopType (p));
		P_SetParLoopPstmt (new_parloop,
				   copy_pstmt (dst_table, dst_scope, src_table,
					       P_GetParLoopPstmt (p),
					       preserve, clean));
		P_SetParLoopIterationVar \
		  (new_parloop, copy_expr_list (dst_table, dst_scope,
						src_table,
						P_GetParLoopIterationVar (p),
						preserve, clean));
		P_SetParLoopInitValue \
		  (new_parloop,
		   copy_expr_list (dst_table, dst_scope, src_table,
				   P_GetParLoopInitValue (p), preserve,
				   clean));
		P_SetParLoopFinalValue \
		  (new_parloop,
		   copy_expr_list (dst_table, dst_scope, src_table,
				   P_GetParLoopFinalValue (p), preserve,
				   clean));
		P_SetParLoopIncrValue \
		  (new_parloop,
		   copy_expr_list (dst_table, dst_scope, src_table,
				   P_GetParLoopIncrValue (p), preserve,
				   clean));
		P_SetParLoopChild (new_parloop,
				   copy_stmt (dst_table, dst_scope, src_table,
					      P_GetParLoopChild (p),
					      preserve, clean));
		P_SetParLoopDepth (new_parloop, P_GetParLoopDepth (p));

		P_SetStmtParLoop (new, new_parloop);
	      }
	  }
	  break;

	case ST_SERLOOP:
	  {
	    SerLoop t, new_serloop;

	    if ((t = P_GetStmtSerLoop (s)))
	      {
		new_serloop = P_NewSerLoop ();

		P_SetSerLoopLoopType (new_serloop, P_GetSerLoopLoopType (t));
		P_SetSerLoopLoopBody (new_serloop,
				      copy_stmt (dst_table, dst_scope,
						 src_table,
						 P_GetSerLoopLoopBody (t),
						 preserve, clean));
		P_SetSerLoopCondExpr (new_serloop,
				      copy_expr_list (dst_table, dst_scope,
						      src_table,
						      P_GetSerLoopCondExpr (t),
						      preserve, clean));
		P_SetSerLoopInitExpr (new_serloop,
				      copy_expr_list (dst_table, dst_scope,
						      src_table,
						      P_GetSerLoopInitExpr (t),
						      preserve, clean));
		P_SetSerLoopIterExpr (new_serloop,
				      copy_expr_list (dst_table, dst_scope,
						      src_table,
						      P_GetSerLoopIterExpr (t),
						      preserve, clean));

		P_SetStmtSerLoop (new, new_serloop);
	      }
	  }
	  break;

	case ST_EXPR:
	  P_SetStmtExpr (new, copy_expr_list (dst_table, dst_scope, src_table,
					      P_GetStmtExpr (s), preserve,
					      clean));
	  break;

	case ST_BODY:
	  {
	    BodyStmt b, new_bodystmt;

	    if ((b = P_GetStmtBodyStmt (s)))
	      {
		new_bodystmt = P_NewBodyStmt ();

		P_SetBodyStmtStatement (new_bodystmt,
					copy_stmt (dst_table, dst_scope,
						   src_table,
						   P_GetBodyStmtStatement (b),
						   preserve, clean));

		P_SetStmtBodyStmt (new, new_bodystmt);
	      }
	  }
	  break;

	case ST_EPILOGUE:
	  {
	    EpilogueStmt e, new_epiloguestmt;

	    if ((e = P_GetStmtEpilogueStmt (s)))
	      {
		new_epiloguestmt = P_NewEpilogueStmt ();

		P_SetEpilogueStmtStatement \
		  (new_epiloguestmt, copy_stmt (dst_table, dst_scope,
						src_table,
						P_GetEpilogueStmtStatement (e),
						preserve, clean));

		P_SetStmtEpilogueStmt (new, new_epiloguestmt);
	      }
	  }
	  break;

	case ST_ASM:
	  {
	    AsmStmt a, new_asmstmt;

	    if ((a = P_GetStmtAsmStmt (s)))
	      {
		new_asmstmt = P_NewAsmStmt ();

		P_SetAsmStmtIsVolatile (new_asmstmt,
					P_GetAsmStmtIsVolatile (a));
		P_SetAsmStmtAsmClobbers \
		  (new_asmstmt, copy_expr_list (dst_table, dst_scope,
						src_table,
						P_GetAsmStmtAsmClobbers (a),
						preserve, clean));
		P_SetAsmStmtAsmString \
		  (new_asmstmt, copy_expr_list (dst_table, dst_scope,
						src_table,
						P_GetAsmStmtAsmString (a),
						preserve, clean));
		P_SetAsmStmtAsmOperands \
		  (new_asmstmt, copy_expr_list (dst_table, dst_scope,
						src_table,
						P_GetAsmStmtAsmOperands (a),
						preserve, clean));
		
		P_SetStmtAsmStmt (new, new_asmstmt);
	      }
	  }
	  break;

	default:
	  break;
	}

      if (Handlers[ES_STMT])
	{
	  for (i = 0; i < NumExtensions[ES_STMT]; i++)
	    {
	      if (s->ext[i] && (copy = Handlers[ES_STMT][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_STMT][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }

		  new->ext[i] = copy (s->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies a Stmt and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Stmt will be created.
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the Stmt (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Stmt in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a s.
 *
 * The entire list (lex_next chain) is copied.
 *
 * ::Stmt.parent, ::Stmt.parent_func, and ::Stmt.parent_expr are set
 * to null in the new Stmt.
 *
 * This function does the real work of PST_CopyStmt() and
 * PST_TableCopyStmt().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyStmt(), PST_TableCopyStmt() */
static Stmt
copy_stmt (SymbolTable dst_table, Key dst_scope, SymbolTable src_table, Stmt s,
	   bool preserve, KeyList *clean)
{
  Stmt new = NULL;

  if (s)
    {
      new = copy_stmt_node (dst_table, dst_scope, src_table, s, preserve,
			    clean);
      P_SetStmtLexNext (new, copy_stmt (dst_table, dst_scope, src_table,
					P_GetStmtLexNext (s), preserve,
					clean));
      if (new->lex_next)
	new->lex_next->lex_prev = new;
    }

  return (new);
}

/*! \brief Copies a Label and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Label will be created.
 * \param src_table
 *  the source symbol table.
 * \param l
 *  the Label (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Label in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a l.
 *
 * The entire label list is copied.
 *
 * This function does the real work of PST_CopyLabel() and
 * PST_TableCopyLabel().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries
 * that have been tagged with a KeyMap structure is returned as \a
 * clean.  These must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyLabel(), PST_TableCopyLabel() */
static Label
copy_label (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	    Label l, bool preserve, KeyList *clean)
{
  Label new = NULL;
  Key dst_key = Invalid_Key;

  if (l)
    {
      new = P_NewLabel ();

      if (l->val)
	new->val = strdup (l->val);

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = l->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

      /* Add the new Label to the symbol table if necessary. */
      if (dst_key.file != 0)
	{
	  /* Remove the pre-existing entry with Label's key if necessary. */
	  if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	    PST_RemoveEntry (dst_table, dst_key);
	  
	  dst_key = PST_AddLabelEntry (dst_table, new);
	  
	  /* Add the Label to the scope. */
	  PST_AddEntryToScope (dst_table, dst_scope, dst_key);
	  
	  /* Note the Label's new key in the symbol table. */
	  PST_SetNewKey (src_table, P_GetLabelKey (l), dst_key, clean);
	}

      new->type = l->type;
      if (l->type == LB_CASE)
	new->data.expression = copy_expr_list (dst_table, dst_scope, src_table,
					       l->data.expression, preserve,
					       clean);
      new->next = copy_label (dst_table, dst_scope, src_table, l->next,
			      preserve, clean);
      if (new->next)
	new->next->prev = new;

    }

  return (new);
}

/*! \brief Copies a Pstmt and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Pstmt will be created.
 * \param src_table
 *  the source symbol table.
 * \param p
 *  the Pstmt (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Pstmt in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a p.
 *
 * A list of keys of the symbol table entries that have been tagged
 * with a KeyMap structure is returned as \a clean.  These must be
 * cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 */
static Pstmt
copy_pstmt (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	    Pstmt p, bool preserve, KeyList *clean)
{
  Pstmt new = NULL;
  CopyHandler copy;
  int i;

  if (p)
    {
      new = P_NewPstmt ();

      P_SetPstmtStmt (new, copy_stmt (dst_table, dst_scope, src_table,
				      P_GetPstmtStmt (p), preserve, clean));
      new->lineno = p->lineno;
      new->colno = p->colno;
      if (p->filename)
	new->filename = strdup (p->filename);

      if (Handlers[ES_PSTMT])
	{
	  for (i = 0; i < NumExtensions[ES_PSTMT]; i++)
	    {
	      if (p->ext[i] && (copy = Handlers[ES_PSTMT][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_PSTMT][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }

		  new->ext[i] = copy (p->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies an Expr without operands, siblings, or the next list.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Expr will be created.
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the Expr (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Expr in \a dst_table without operands, siblings, or the
 *  next list.  \a dst_table is updated so that the copy is completely
 *  distinct from \a e.
 *
 * This function does the real work of PST_CopyExprNode() and
 * PST_TableCopyExprNode().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyExprNode(), PST_TableCopyExprNode() */
static Expr
copy_expr_node (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
		Expr e, bool preserve, KeyList *clean)
{
  Expr new = NULL;
  Expr operands, sibling, next;

  if (e)
    {
      operands = P_GetExprOperands (e);
      sibling = P_GetExprSibling (e);
      next = P_GetExprNext (e);
      P_SetExprOperands (e, NULL);
      P_SetExprSibling (e, NULL);
      P_SetExprNext (e, NULL);

      new = copy_expr (dst_table, dst_scope, src_table, e, preserve, clean);

      P_SetExprOperands (e, operands);
      P_SetExprSibling (e, sibling);
      P_SetExprNext (e, next);
    }

  return (new);
}

/*! \brief Copies an Expr and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Expr will be created.
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the Expr (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Expr in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a e.
 *
 * Copies an Expr.  Expr.next is not copied.
 *
 * This function does the real work of PST_CopyExpr() and
 * PST_TableCopyExpt().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyExpr(), PST_TableCopyExpr() */
static Expr
copy_expr (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	   Expr e, bool preserve, KeyList *clean)
{
  Expr new = NULL;
  Expr *popd, opd;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (e)
    {
      new = P_NewExpr ();

      new->id = PST_ScopeNextExprID (dst_table, dst_scope);
      new->status = e->status;

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = e->key;
	}
      else if (P_ValidKey (e->key))
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;

      /* Add the new expr to the symbol table if necessary. */
      if (dst_key.file != 0)
	{
	  /* Remove the pre-existing entry with Expr's key if necessary. */
	  if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	    PST_RemoveEntry (dst_table, dst_key);
	  
	  dst_key = PST_AddExprEntry (dst_table, new);

	  /* Add the Expr to the scope. */
	  PST_AddEntryToScope (dst_table, dst_scope, dst_key);
	}

      new->opcode = e->opcode;
      new->flags = e->flags;
      new->type = e->type;

      switch (e->opcode)
	{
	case OP_int:
	  if (P_GetExprFlags (e) & EF_UNSIGNED)
	    new->value.uscalar = e->value.uscalar;
	  else
	    new->value.scalar = e->value.scalar;
	  break;

	case OP_float:
	case OP_double:
	case OP_real:
	  new->value.real = e->value.real;
	  break;

	case OP_char:
	case OP_string:
	  if (e->value.string)
	    new->value.string = strdup (e->value.string);
	  break;

	case OP_dot:
	case OP_arrow:
	case OP_var:
	  if (e->value.var.name)
	    new->value.var.name = strdup (e->value.var.name);

	  /* Write the var key to the Expr in case the var will not be
	   * copied. */
	  new->value.var.key = e->value.var.key;
	  break;

	case OP_cast:
#if 0
	  /* Make sure the referenced type exists in the destination table. */
	  refd_type_dcl = PST_GetTypeDclEntry (src_table, P_GetExprType (e));
	  refd_type_key = PST_ScopeFindTypeDcl (dst_table, dst_scope,
						refd_type_dcl);

	  PST_SetExprType (dst_table, new, refd_type_key);
#endif
	  /* The type key will be udpated in update_expr_keys(), so use the
	   * old type key for now. */
	  P_SetExprType (new, P_GetExprType (e));
	  break;

	case OP_stmt_expr:
	  P_SetExprStmt (new, copy_stmt (dst_table, dst_scope, src_table,
					 P_GetExprStmt (e), preserve, clean));
	  break;

	case OP_asm_oprd:
	  P_SetExprAsmoprd (new, P_CopyAsmoprd (P_GetExprAsmoprd (e)));
	  break;

	default:
	  break;
	}

      popd = &new->operands;

      for (opd = e->operands; opd; opd = opd->sibling)
	{
	  Expr nopd;

	  *popd = nopd = copy_expr_list (dst_table, dst_scope, src_table, opd,
					 preserve, clean);
	  /* 03/10/04 REK Need to set the parentexpr for all exprs in the
	   *              list, not just nopd. */
	  P_SetExprParentExprAll (nopd, new);
	  popd = &nopd->sibling;
	}

      /* The next field isn't copied.  See copy_expr_list(). */

      new->parentexpr = e->parentexpr;
      new->parentstmt = e->parentstmt;
      new->parentvar = e->parentvar;
      new->pragma = copy_pragma (dst_table, dst_scope, src_table, e->pragma,
				 preserve, clean);
      new->profile = P_CopyProfEXPR (e->profile);

      if (Handlers[ES_EXPR])
	{
	  for (i = 0; i < NumExtensions[ES_EXPR]; i++)
	    {
	      if (e->ext[i] && (copy = Handlers[ES_EXPR][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_EXPR][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }

		  new->ext[i] = copy (e->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Copies an Expr list and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Expr will be created.
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the Expr (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the Expr in \a dst_table.  \a dst_table is updated so that the
 *  copy is completely distinct from \a e.
 *
 * Copies an Expr.  The entire Expr list (Expr.next) is copied.
 *
 * This function does the real work of PST_CopyExprList() and
 * PST_TableCopyExprList().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of keys of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyExprList(), PST_TableCopyExprList() */
static Expr
copy_expr_list (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
		Expr e, bool preserve, KeyList *clean)
{
  Expr new = NULL;

  if (e)
    {
      Expr nxt, nnxt, nprv;

      nprv = new = copy_expr (dst_table, dst_scope, src_table, e, preserve,
			      clean);

      for (nxt = e->next; nxt; nxt = nxt->next)
	{
	  nprv->next = nnxt = copy_expr (dst_table, dst_scope, src_table, nxt,
					 preserve, clean);
	  nnxt->previous = nprv;
	  nprv = nnxt;
	}
    }

  return (new);
}

/*! \brief Copies a Pragma and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Pragma will be created.
 * \param src_table
 *  the source symbol table.
 * \param p
 *  the Pragma (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with a
 *  KeyMap structure.
 *
 * \return
 *  A copy of the Pragma in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a p.
 *
 * A list of keys of the symbol table entries that have been tagged
 * with a KeyMap structure is returned as \a clean.  These must be
 * cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 */
static Pragma
copy_pragma (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	     Pragma p, bool preserve, KeyList *clean)
{
  Pragma new = NULL;

  if (p)
    {
      new = P_NewPragma ();

      if (p->specifier)
	new->specifier = strdup (p->specifier);
      new->expr = copy_expr_list (dst_table, dst_scope, src_table, p->expr,
				  preserve, clean);
      new->lineno = p->lineno;
      new->colno = p->colno;
      if (p->filename)
	new->filename = strdup (p->filename);

      new->next = copy_pragma (dst_table, dst_scope, src_table, p->next,
			       preserve, clean);
    }

  return (new);
}

/*! \brief Copies an AsmDcl and updates the symbol table.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param dst_scope
 *  the scope under which the new Expr will be created.
 * \param src_table
 *  the source symbol table.
 * \param a
 *  the AsmDcl (in \a src_table) to copy.
 * \param preserve
 *  if TRUE, the copy will get the same key as \a d.
 * \param clean
 *  a list of keys of symbol table entries that have been tagged with
 *  a KeyMap structure.
 *
 * \return
 *  A copy of the AsmDcl in \a dst_table.  \a dst_table is updated so that
 *  the copy is completely distinct from \a a.
 *
 * This function does the real work of PST_CopyAsmDcl() and
 * PST_TableCopyAsmDcl().  If \a dst_key is valid, it is used as the
 * copy's key.  A list of key of the symbol table entries that have
 * been tagged with a KeyMap structure is returned as \a clean.  These
 * must be cleaned after the copy is complete.
 *
 * \note \a dst_table and \a src_table can be the same.
 * \note \a preserve == TRUE implies \a dst_table != \a src_table.
 *
 * \sa PST_CopyAsmDcl(), PST_TableCopyAsmDcl() */
static AsmDcl
copy_asm_dcl (SymbolTable dst_table, Key dst_scope, SymbolTable src_table,
	      AsmDcl a, bool preserve, KeyList *clean)
{
  AsmDcl new = NULL;
  CopyHandler copy;
  Key dst_key = Invalid_Key;
  int i;

  if (a)
    {
      new = P_NewAsmDcl ();

      new->is_volatile = a->is_volatile;
      new->asm_clobbers = copy_expr_list (dst_table, dst_scope, src_table,
					  a->asm_clobbers, preserve, clean);
      new->asm_string = copy_expr_list (dst_table, dst_scope, src_table,
					a->asm_string, preserve, clean);
      new->asm_operands = copy_expr_list (dst_table, dst_scope, src_table,
					  a->asm_operands, preserve, clean);

      /* If we're preserving the key, use the original statement's key.
       * Otherwise, use the destination scope's file key with a 0 symbol
       * so a new entry will be created for this Stmt. */
      if (preserve)
	{
	  dst_key = a->key;
	}
      else
	{
	  dst_key.file = dst_scope.file;
	  dst_key.sym = 0;
	}

      new->key = dst_key;
      new->lineno = a->lineno;
      new->colno = a->colno;
      if (a->filename)
	new->filename = strdup (a->filename);

      /* Add the new AsmDcl to the symbol table if necessary. */
      if (new->key.file != 0)
	{
	  /* Remove the pre-existing entry with AsmDcl's key if necessary. */
	  if (P_ValidKey (dst_key) && PST_GetSymTabEntry (dst_table, dst_key))
	    PST_RemoveEntry (dst_table, dst_key);
	  
	  dst_key = PST_AddAsmDclEntry (dst_table, new);

	  /* Add the AsmDcl to the scope. */
	  PST_AddEntryToScope (dst_table, dst_scope, dst_key);
	}

      if (Handlers[ES_ASM])
	{
	  for (i = 0; i < NumExtensions[ES_ASM]; i++)
	    {
	      if (a->ext[i] && (copy = Handlers[ES_ASM][i].copy))
		{
		  if (new->ext[i])
		    {
		      FreeHandler ext_free = Handlers[ES_ASM][i].free;
		      new->ext[i] = ext_free (new->ext[i]);
		    }
		
		  new->ext[i] = copy (a->ext[i]);
		}
	    }
	}
    }

  return (new);
}

/*! \brief Updates the keys in a FuncDcl.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the FuncDcl (in \a dst_table) to update.
 *
 * This function assumes all keys in \a f refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a f.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_func_dcl_keys (SymbolTable dst_table, SymbolTable src_table, FuncDcl f)
{
  Key new;

  new = PST_GetNewKey (src_table, P_GetFuncDclType (f));
  if (P_ValidKey (new))
    PST_SetFuncDclType (dst_table, f, new);

  update_var_list_keys (dst_table, src_table, P_GetFuncDclParam (f));
  update_stmt_keys (dst_table, src_table, P_GetFuncDclStmt (f));

  return;
}

/*! \brief Updates the keys in a TypeDcl.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param t
 *  the TypeDcl (in \a dst_table) to update.
 *
 * This function assumes all keys in \a t refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a t.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_type_dcl_keys (SymbolTable dst_table, SymbolTable src_table, TypeDcl t)
{
  Key new;
  _BasicType bt = P_GetTypeDclBasicType (t);

  new = PST_GetNewKey (src_table, P_GetTypeDclType (t));
  if (P_ValidKey (new))
    PST_SetTypeDclType (dst_table, t, new);

  if (bt & BT_FUNC)
    {
      Param p;

      for (p = P_GetTypeDclParam (t); p; p = P_GetParamNext (p))
	{
	  new = PST_GetNewKey (src_table, P_GetParamKey (p));
	  if (P_ValidKey (new))
	    PST_SetParamKey (dst_table, p, new);
	}
    }
  else if (bt & BT_ARRAY)
    {
      update_expr_keys (dst_table, src_table, P_GetTypeDclArraySize (t));
    }

  return;
}

/*! \brief Updates the keys in a VarList.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param l
 *  the VarList (in \a dst_table) to update.
 *
 * This function assumes all keys in \a l refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a l.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_var_list_keys (SymbolTable dst_table, SymbolTable src_table, VarList l)
{
  VarDcl v;

  List_start (l);
  while ((v = (VarDcl)List_next (l)))
    update_var_dcl_keys (dst_table, src_table, v);

  return;
}

/*! \brief Updates the keys in a VarDcl.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param v
 *  the VarDcl (in \a dst_table) to update.
 *
 * This function assumes all keys in \a v refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a v.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_var_dcl_keys (SymbolTable dst_table, SymbolTable src_table, VarDcl v)
{
  Key new;

  new = PST_GetNewKey (src_table, P_GetVarDclType (v));
  if (P_ValidKey (new))
    PST_SetVarDclType (dst_table, v, new);

  if (P_GetVarDclInit (v))
    update_init_keys (dst_table, src_table, P_GetVarDclInit (v));

  return;
}

/*! \brief Updates the keys in an Init.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param i
 *  the Init (in \a dst_table) to update.
 *
 * This function assumes all keys in \a i refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a i.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_init_keys (SymbolTable dst_table, SymbolTable src_table, Init i)
{
  if (P_GetInitExpr (i))
    update_expr_keys (dst_table, src_table, P_GetInitExpr (i));
  if (P_GetInitSet (i))
    update_init_keys (dst_table, src_table, P_GetInitSet (i));
  if (P_GetInitNext (i))
    update_init_keys (dst_table, src_table, P_GetInitNext (i));

  return;
}

/*! \brief Updates the keys in a StructDcl.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the StructDcl (in \a dst_table) to update.
 *
 * This function assumes all keys in \a s refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a s.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_struct_dcl_keys (SymbolTable dst_table, SymbolTable src_table,
			StructDcl s)
{
  Field f;

  if ((f = P_GetStructDclFields (s)))
    update_field_keys (dst_table, src_table, f);

  return;
}

/*! \brief Updates the keys in a UnionDcl.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param u
 *  the UnionDcl (in \a dst_table) to update.
 *
 * This function assumes all keys in \a u refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a u.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_union_dcl_keys (SymbolTable dst_table, SymbolTable src_table,
		       UnionDcl u)
{
  Field f;

  if ((f = P_GetUnionDclFields (u)))
    update_field_keys (dst_table, src_table, f);

  return;
}

/*! \brief Updates the keys in a Field.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param f
 *  the Field (in \a dst_table) to update.
 *
 * This function assumes all keys in \a f refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a f.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_field_keys (SymbolTable dst_table, SymbolTable src_table, Field f)
{
  Key new;

  new = PST_GetNewKey (src_table, P_GetFieldParentKey (f));
  if (P_ValidKey (new))
    P_SetFieldParentKey (f, new);

  new = PST_GetNewKey (src_table, P_GetFieldType (f));
  if (P_ValidKey (new))
    PST_SetFieldType (dst_table, f, new);

  if (P_GetFieldNext (f))
    update_field_keys (dst_table, src_table, P_GetFieldNext (f));

  return;
}

/*! \brief Updates the keys in a Stmt.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param s
 *  the Stmt (in \a dst_table) to update.
 *
 * This function assumes all keys in \a s refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a s.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_stmt_keys (SymbolTable dst_table, SymbolTable src_table, Stmt s)
{
  Key new;

  if (s)
    {
      switch (P_GetStmtType (s))
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_ADVANCE:
	case ST_AWAIT:
	  break;
	case ST_RETURN:
	  update_expr_keys (dst_table, src_table, P_GetStmtRet (s));
	  break;
	case ST_GOTO:
	  new = PST_GetNewKey (src_table, P_GetStmtLabelKey (s));
	  if (P_ValidKey (new))
	    P_SetStmtLabelKey (s, new);
	  break;
	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (s);
	    TypeDcl t;
	    
	    List_start (P_GetCompoundTypeList (c));
	    while ((t = (TypeDcl)List_next (P_GetCompoundTypeList (c))))
	      update_type_dcl_keys (dst_table, src_table, t);
	    update_var_list_keys (dst_table, src_table,
				  P_GetCompoundVarList (c));
	    update_stmt_keys (dst_table, src_table, P_GetCompoundStmtList (c));
	  }
	  break;
	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (s);
	    
	    update_expr_keys (dst_table, src_table, P_GetIfStmtCondExpr (i));
	    update_stmt_keys (dst_table, src_table, P_GetIfStmtThenBlock (i));
	    update_stmt_keys (dst_table, src_table, P_GetIfStmtElseBlock (i));
	  }
	  break;
	case ST_SWITCH:
	  {
	    SwitchStmt st = P_GetStmtSwitchStmt (s);

	    update_expr_keys (dst_table, src_table,
			      P_GetSwitchStmtExpression (st));
	    update_stmt_keys (dst_table, src_table,
			      P_GetSwitchStmtSwitchBody (st));
	  }
	  break;
	case ST_PSTMT:
	  update_pstmt_keys (dst_table, src_table, P_GetStmtPstmt (s));
	  break;
	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (s);

	    update_expr_keys (dst_table, src_table, P_GetMutexExpression (m));
	    update_stmt_keys (dst_table, src_table, P_GetMutexStatement (m));
	  }
	  break;
	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (s);

	    update_stmt_keys (dst_table, src_table,
			      P_GetCobeginStatements (c));
	  }
	  break;
	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (s);

	    update_pstmt_keys (dst_table, src_table, P_GetParLoopPstmt (p));
	    update_expr_keys (dst_table, src_table,
			      P_GetParLoopIterationVar (p));
	    update_expr_keys (dst_table, src_table, P_GetParLoopInitValue (p));
	    update_expr_keys (dst_table, src_table,
			      P_GetParLoopFinalValue (p));
	    update_expr_keys (dst_table, src_table, P_GetParLoopIncrValue (p));
	    update_stmt_keys (dst_table, src_table, P_GetParLoopChild (p));
	  }
	  break;
	case ST_SERLOOP:
	  {
	    SerLoop sl = P_GetStmtSerLoop (s);

	    update_stmt_keys (dst_table, src_table, P_GetSerLoopLoopBody (sl));
	    update_expr_keys (dst_table, src_table, P_GetSerLoopCondExpr (sl));
	    update_expr_keys (dst_table, src_table, P_GetSerLoopInitExpr (sl));
	    update_expr_keys (dst_table, src_table, P_GetSerLoopIterExpr (sl));
	  }
	  break;
	case ST_EXPR:
	  update_expr_keys (dst_table, src_table, P_GetStmtExpr (s));
	  break;
	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (s);

	    update_stmt_keys (dst_table, src_table,
			      P_GetBodyStmtStatement (b));
	  }
	  break;
	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (s);

	    update_stmt_keys (dst_table, src_table,
			      P_GetEpilogueStmtStatement (e));
	  }
	  break;
	case ST_ASM:
	  {
	    AsmStmt a = P_GetStmtAsmStmt (s);

	    update_expr_keys (dst_table, src_table,
			      P_GetAsmStmtAsmClobbers (a));
	    update_expr_keys (dst_table, src_table, P_GetAsmStmtAsmString (a));
	    update_expr_keys (dst_table, src_table,
			      P_GetAsmStmtAsmOperands (a));
	  }
	  break;
	}

      update_stmt_keys (dst_table, src_table, P_GetStmtLexNext (s));
    }

  return;
}

/*! \brief Updates the keys in a Pstmt.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param p
 *  the Pstmt (in \a dst_table) to update.
 *
 * This function assumes all keys in \a p refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a p.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_pstmt_keys (SymbolTable dst_table, SymbolTable src_table, Pstmt p)
{
  update_stmt_keys (dst_table, src_table, P_GetPstmtStmt (p));

  return;
}

/*! \brief Updates the keys in an Expr.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param e
 *  the Expr (in \a dst_table) to update.
 *
 * This function assumes all keys in \a e refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a e.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_expr_keys (SymbolTable dst_table, SymbolTable src_table, Expr e)
{
  Key new;
  Expr o;

  if (e)
    {
      switch (P_GetExprOpcode (e))
	{
	case OP_var:
	  new = PST_GetNewKey (src_table, P_GetExprVarKey (e));
	  if (P_ValidKey (new))
	    P_SetExprVarKey (e, new);
	  break;
	case OP_char:
	case OP_int:
	case OP_string:
	case OP_cast:
	case OP_float:
	case OP_double:
	  new = PST_GetNewKey (src_table, P_GetExprType (e));
	  if (P_ValidKey (new))
	    PST_SetExprType (dst_table, e, new);
	  new = PST_GetNewKey (src_table, P_GetExprVType (e));
	  if (P_ValidKey (new))
	    PST_SetExprVType (dst_table, e, new);
	  break;
	case OP_stmt_expr:
	  update_stmt_keys (dst_table, src_table, P_GetExprStmt (e));
	  break;
	default:
	  break;
	}

      for (o = P_GetExprOperands (e); o; o = P_GetExprSibling (o))
	update_expr_keys (dst_table, src_table, o);
      update_expr_keys (dst_table, src_table, P_GetExprNext (e));
    }

  return;
}

/*! \brief Updates the keys in an AsmDcl.
 *
 * \param dst_table
 *  the destination symbol table.
 * \param src_table
 *  the source symbol table.
 * \param a
 *  the AsmDcl (in \a dst_table) to update.
 *
 * This function assumes all keys in \a a refer to \a src_table.  This
 * function inspects each SymTabEntry referenced by \a a.  If the
 * SymTabEntry has a new key, its key is updated.
 *
 * This function assumes that keys in \a src_table that have been copied
 * to \a dst_table have been tagged with their key in \a dst_table.
 *
 * \note \a dst_table and \a src_table can be the same.  */
static void
update_asm_dcl_keys (SymbolTable dst_table, SymbolTable src_table, AsmDcl a)
{
  update_expr_keys (dst_table, src_table, P_GetAsmDclAsmClobbers (a));
  update_expr_keys (dst_table, src_table, P_GetAsmDclAsmString (a));
  update_expr_keys (dst_table, src_table, P_GetAsmDclAsmOperands (a));

  return;
}

/*! \addto FindExternalSymbols */
/* @{ */
/*! \brief Finds the symbols needed by a FuncDcl.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  symbols defined outside this scope are considered external.
 * \param f
 *  the FuncDcl to inspect.
 * \param known
 *  a list of symbols that are already needed by something else in \a scope.
 *
 * \return
 *  A list of keys of symbols needed by \a f that are defined outside \a scope.
 *
 * This function inspects FuncDcl \a f and returns a list of keys of symbols
 * that are referenced by \a f, but are defined outside \a scope.  If
 * \a f is defined outside \a scope, this function adds \a f's key to the
 * extern list.  The caller does not need to do this.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
static KeyList
get_func_dcl_scope_externs (SymbolTable table, Key scope, FuncDcl f,
			    KeyList known)
{
  Key func_key = P_GetFuncDclKey (f);
  int direction;

  /* If the FuncDcl is already known to be an external symbol, we can return
   * immediately. */
  if (P_FindKeyListKey (known, func_key))
    return (known);

  /* This function may be called under two contexts.
   *
   * In the first, we're walking down the scope stack searching for
   * external references.  In this case, the FuncDcl will be defined
   * under the scope.
   *
   * In the second, we've already found an external reference (namely,
   * this FuncDcl), and are finding other symbols that the reference
   * references.  This would happen after finding an function call
   * expression, for example.
   *
   * It is assumed that in this second case, we only need a function
   * header, so the function body is not inspected.
   */

  direction = PST_ScopeContainsKey (table, scope, func_key) ? DOWN : UP;

  if (direction == UP)
    {
      /* Walking up a chain of external symbols.  Add the FuncDcl to the
       * list of external symbols. */
      known = P_AppendKeyListNext (known, P_NewKeyListWithKey (func_key));
    }

  /* Search the function's type for external type references.  The function's
   * type includes the type information for the parameters, so they don't
   * need to be inspected here. */
  known = \
    get_type_dcl_scope_externs (table, scope,
				PST_GetTypeDclEntry (table,
						     P_GetFuncDclType (f)),
				known);

  if (direction == DOWN)
    {
      /* Search the function's body for external references. */
      known = get_stmt_scope_externs (table, scope, P_GetFuncDclStmt (f),
				      known);
    }
  
  return (known);
}

/*! \brief Finds the symbols needed by a TypeDcl.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  symbols defined outside this scope are considered external.
 * \param t
 *  the TypeDcl to inspect.
 * \param known
 *  a list of symbols that are already needed by something else in \a scope.
 *
 * \return
 *  A list of keys of symbols needed by \a t that are defined outside \a scope.
 *
 * This function inspects TypeDcl \a t and returns a list of keys of symbols
 * that are referenced by \a t, but are defined outside \a scope.  If
 * \a t is defined outside \a scope, this function adds \a t's key to the
 * extern list.  The caller does not need to do this.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
static KeyList
get_type_dcl_scope_externs (SymbolTable table, Key scope, TypeDcl t,
			    KeyList known)
{
  Key type_key = P_GetTypeDclKey (t);
  _BasicType bt = P_GetTypeDclBasicType (t);
  Key base_type_key = P_GetTypeDclType (t);

  /* If the TypeDcl is already known to be an external symbol, we can
   * return immediately. */
  if (P_FindKeyListKey (known, type_key))
    return (known);

  /* Unlike a FuncDcl, there is no scope stack to search down on a TypeDcl.
   * We always inspect the keys referenced by the TypeDcl.  The only
   * decision is whether or not this TypeDcl is an extern. */

  /* If this TypeDcl is defined outside the scope, add it to the list of
   * external symbols. */
  if (!PST_ScopeContainsKey (table, scope, type_key))
    known = P_AppendKeyListNext (known, P_NewKeyListWithKey (type_key));

  if (bt & (BT_STRUCTURE | BT_TYPEDEF | BT_ARRAY | BT_FUNC | BT_POINTER))
    {
      switch (bt)
	{
	case BT_STRUCT:
	  {
	    StructDcl struct_dcl = PST_GetStructDclEntry (table,
							  base_type_key);

	    known = get_struct_dcl_scope_externs (table, scope, struct_dcl,
						  known);
	  }
	  break;

	case BT_UNION:
	  {
	    UnionDcl union_dcl = PST_GetUnionDclEntry (table, base_type_key);

	    known = get_union_dcl_scope_externs (table, scope, union_dcl,
						 known);
	  }
	  break;

	case BT_FUNC:
	  {
	    /* Process a function's param types. */
	    Param cur_param;

	    for (cur_param = P_GetTypeDclParam (t); cur_param;
		 cur_param = P_GetParamNext (cur_param))
	      {
		known = \
		  get_type_dcl_scope_externs \
		    (table, scope,
		     PST_GetTypeDclEntry (table, P_GetParamKey (cur_param)),
		     known);
	      }
	  }
	  /* Fall through to process the FuncDcl's return type. */

	case BT_TYPEDEF_E:
	case BT_TYPEDEF_I:
	case BT_ARRAY:
	case BT_POINTER:
	  {
	    TypeDcl type_dcl = PST_GetTypeDclEntry (table, base_type_key);

	    known = get_type_dcl_scope_externs (table, scope, type_dcl, known);
	  }
	  break;

	default:
	  P_punt ("struct_symtab.c:get_type_dcl_scope_externs:%d Invalid "
		  "basic type\n0x%x\n", __LINE__ - 1, bt);
	}
    }

  return (known);
}

/*! \brief Finds the symbols needed by a VarDcl.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  symbols defined outside this scope are considered external.
 * \param v
 *  the VarDcl to inspect.
 * \param known
 *  a list of symbols that are already needed by something else in \a scope.
 *
 * \return
 *  A list of keys of symbols needed by \a v that are defined outside \a scope.
 *
 * This function inspects VarDcl \a v and returns a list of keys of symbols
 * that are referenced by \a v, but are defined outside \a scope.  If
 * \a v is defined outside \a scope, this function adds \a v's key to the
 * extern list.  The caller does not need to do this.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
static KeyList
get_var_dcl_scope_externs (SymbolTable table, Key scope, VarDcl v,
			   KeyList known)
{
  Key var_key = P_GetVarDclKey (v);
  Key type_key = P_GetVarDclType (v);
  TypeDcl type_dcl = PST_GetTypeDclEntry (table, type_key);

  /* There is no scope stack to search down on a VarDcl.  If the VarDcl
   * is outside the scope, add it to the list.  We always need to inspect
   * the var's type. */

  /* If the VarDcl is already known to be an external symbol, we can
   * return immediately. */
  if (P_FindKeyListKey (known, var_key))
    return (known);

  /* If this VarDcl is defined outside the scope, add it to the list of
   * external symbols. */
  if (!PST_ScopeContainsKey (table, scope, var_key))
    known = P_AppendKeyListNext (known, P_NewKeyListWithKey (var_key));

  /* Inspect the VarDcl's type. */
  known = get_type_dcl_scope_externs (table, scope, type_dcl, known);

  return (known);
}

/*! \brief Finds the symbols needed by a StructDcl.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  symbols defined outside this scope are considered external.
 * \param s
 *  the StructDcl to inspect.
 * \param known
 *  a list of symbols that are already needed by something else in \a scope.
 *
 * \return
 *  A list of keys of symbols needed by \a s that are defined outside \a scope.
 *
 * This function inspects StructDcl \a s and returns a list of keys of symbols
 * that are referenced by \a s, but are defined outside \a scope.  If
 * \a s is defined outside \a scope, this function adds \a s's key to the
 * extern list.  The caller does not need to do this.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
static KeyList
get_struct_dcl_scope_externs (SymbolTable table, Key scope, StructDcl s,
			      KeyList known)
{
  Key struct_key = P_GetStructDclKey (s);
  Field f;

  /* There is no scope stack to search down on a StructDcl.  If the StructDcl
   * is outside the scope, add it to the list.  We always need to inspect the
   * field types. */

  /* If the StructDcl is already known to be an external symbol, we can
   * return immediately. */
  if (P_FindKeyListKey (known, struct_key))
    return (known);

  /* If this StructDcl is defined outside the scope, add it to the list
   * of external symbols. */
  if (!PST_ScopeContainsKey (table, scope, struct_key))
    known = P_AppendKeyListNext (known, P_NewKeyListWithKey (struct_key));

  /* Inspect the field types. */
  for (f = P_GetStructDclFields (s); f; f = P_GetFieldNext (f))
    {
      TypeDcl field_type = PST_GetTypeDclEntry (table, P_GetFieldType (f));

      known = get_type_dcl_scope_externs (table, scope, field_type, known);
    }

  return (known);
}

/*! \brief Finds the symbols needed by a UnionDcl.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  symbols defined outside this scope are considered external.
 * \param u
 *  the UnionDcl to inspect.
 * \param known
 *  a list of symbols that are already needed by something else in \a scope.
 *
 * \return
 *  A list of keys of symbols needed by \a u that are defined outside \a scope.
 *
 * This function inspects UnionDcl \a u and returns a list of keys of symbols
 * that are referenced by \a u, but are defined outside \a scope.  If
 * \a u is defined outside \a scope, this function adds \a u's key to the
 * extern list.  The caller does not need to do this.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
static KeyList
get_union_dcl_scope_externs (SymbolTable table, Key scope, UnionDcl u,
			     KeyList known)
{
  Key union_key = P_GetUnionDclKey (u);
  Field f;

  /* There is no scope stack to search down on a UnionDcl.  If the UnionDcl
   * is outside the scope, add it to the list.  We always need to inspect the
   * field types. */

  /* If the UnionDcl is already known to be an external symbol, we can
   * return immediately. */
  if (P_FindKeyListKey (known, union_key))
    return (known);

  /* If this UnionDcl is defined outside the scope, add it to the list
   * of external symbols. */
  if (!PST_ScopeContainsKey (table, scope, union_key))
    known = P_AppendKeyListNext (known, P_NewKeyListWithKey (union_key));

  /* Inspect the field types. */
  for (f = P_GetUnionDclFields (u); f; f = P_GetFieldNext (f))
    {
      TypeDcl field_type = PST_GetTypeDclEntry (table, P_GetFieldType (f));

      known = get_type_dcl_scope_externs (table, scope, field_type, known);
    }

  return (known);
}

/*! \brief Finds the symbols needed by a Stmt.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  symbols defined outside this scope are considered external.
 * \param s
 *  the Stmt to inspect.
 * \param known
 *  a list of symbols that are already needed by something else in \a scope.
 *
 * \return
 *  A list of keys of symbols needed by \a s that are defined outside \a scope.
 *
 * This function inspects Stmt \a s and returns a list of keys of symbols
 * that are referenced by \a s, but are defined outside \a scope.  The Stmt
 * is assumed to be interesting only if we are going down the scope stack.
 * If we are following references back up, we might include a variable or
 * type that is local to a Stmt, but the Stmt itself will not be included.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
static KeyList
get_stmt_scope_externs (SymbolTable table, Key scope, Stmt s, KeyList known)
{
  while (s)
    {
      switch (P_GetStmtType (s))
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	  break;

	case ST_RETURN:
	  known = get_expr_scope_externs (table, scope, P_GetStmtRet (s),
					  known);
	  break;
	  
	case ST_COMPOUND:
	  {
	    Compound c = P_GetStmtCompound (s);
	    VarList vl = P_GetCompoundVarList (c);
	    TypeList tl = P_GetCompoundTypeList (c);
	    Stmt sl = P_GetCompoundStmtList (c);
	    VarDcl v;
	    TypeDcl t;

	    /* Search the type list for external types. */
	    for (List_start (tl), t = (TypeDcl)List_next (tl); t;
		 t = (TypeDcl)List_next (tl))
	      known = get_type_dcl_scope_externs (table, scope, t, known);

	    /* Search the var list for external types. */
	    for (List_start (vl), v = (VarDcl)List_next (vl); v;
		 v = (VarDcl)List_next (vl))
	      known = get_var_dcl_scope_externs (table, scope, v, known);

	    /* Search the statement list. */
	    known = get_stmt_scope_externs (table, scope, sl, known);
	  }
	  break;

	case ST_IF:
	  {
	    IfStmt i = P_GetStmtIfStmt (s);

	    known = get_expr_scope_externs (table, scope,
					    P_GetIfStmtCondExpr (i), known);
	    known = get_stmt_scope_externs (table, scope,
					    P_GetIfStmtThenBlock (i), known);
	    known = get_stmt_scope_externs (table, scope,
					    P_GetIfStmtElseBlock (i), known);
	  }
	  break;

	case ST_SWITCH:
	  {
	    SwitchStmt sw = P_GetStmtSwitchStmt (s);

	    known = get_expr_scope_externs (table, scope,
					    P_GetSwitchStmtExpression (sw),
					    known);
	    known = get_stmt_scope_externs (table, scope,
					    P_GetSwitchStmtSwitchBody (sw),
					    known);
	  }
	  break;

	case ST_PSTMT:
	  {
	    Pstmt p = P_GetStmtPstmt (s);

	    known = get_stmt_scope_externs (table, scope, P_GetPstmtStmt (p),
					    known);
	  }
	  break;

	case ST_MUTEX:
	  {
	    Mutex m = P_GetStmtMutex (s);
	    	    
	    known = get_expr_scope_externs (table, scope,
					    P_GetMutexExpression (m), known);
	    known = get_stmt_scope_externs (table, scope,
					    P_GetMutexStatement (m), known);
	  }
	  break;

	case ST_COBEGIN:
	  {
	    Cobegin c = P_GetStmtCobegin (s);

	    known = get_stmt_scope_externs (table, scope,
					    P_GetCobeginStatements (c), known);
	  }
	  break;

	case ST_PARLOOP:
	  {
	    ParLoop p = P_GetStmtParLoop (s);

	    known = \
	      get_stmt_scope_externs (table, scope,
				      P_GetPstmtStmt (P_GetParLoopPstmt (p)),
				      known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetParLoopIterationVar (p),
					    known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetParLoopInitValue (p), known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetParLoopFinalValue (p), known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetParLoopIncrValue (p), known);
	    known = get_stmt_scope_externs (table, scope,
					    P_GetParLoopChild (p), known);
	  }
	  break;

	case ST_SERLOOP:
	  {
	    SerLoop sl = P_GetStmtSerLoop (s);

	    known = get_stmt_scope_externs (table, scope,
					    P_GetSerLoopLoopBody (sl), known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetSerLoopCondExpr (sl), known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetSerLoopInitExpr (sl), known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetSerLoopIterExpr (sl), known);
	  }
	  break;

	case ST_EXPR:
	  known = get_expr_scope_externs (table, scope, P_GetStmtExpr (s),
					  known);
	  break;

	case ST_BODY:
	  {
	    BodyStmt b = P_GetStmtBodyStmt (s);

	    known = get_stmt_scope_externs (table, scope,
					    P_GetBodyStmtStatement (b), known);
	  }
	  break;

	case ST_EPILOGUE:
	  {
	    EpilogueStmt e = P_GetStmtEpilogueStmt (s);

	    known = get_stmt_scope_externs (table, scope,
					    P_GetEpilogueStmtStatement (e),
					    known);
	  }
	  break;

	case ST_ASM:
	  {
	    AsmStmt a = P_GetStmtAsmStmt (s);

	    known = get_expr_scope_externs (table, scope,
					    P_GetAsmStmtAsmClobbers (a),
					    known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetAsmStmtAsmString (a),
					    known);
	    known = get_expr_scope_externs (table, scope,
					    P_GetAsmStmtAsmOperands (a),
					    known);
	  }
	  break;

	default:
	  P_punt ("struct_symtab.c:get_stmt_scope_externs:%d Unknown "
		  "statement type %d", __LINE__ - 1, P_GetStmtType (s));
	}

      s = P_GetStmtLexNext (s);
    }

  return (known);
}

/*! \brief Finds the symbols needed by an Expr.
 *
 * \param table
 *  the symbol table.
 * \param scope
 *  symbols defined outside this scope are considered external.
 * \param e
 *  the Expr to inspect.
 * \param known
 *  a list of symbols that are already needed by something else in \a scope.
 *
 * \return
 *  A list of keys of symbols needed by \a e that are defined outside \a scope.
 *
 * This function inspects Expr \a e and returns a list of keys of symbols
 * that are referenced by \a e, but are defined outside \a scope.  The Expr
 * is assumed to be interesting only if we are going down the scope stack.
 *
 * It is the caller's responsibility to free the KeyList returned by this
 * function.
 */
static KeyList
get_expr_scope_externs (SymbolTable table, Key scope, Expr e, KeyList known)
{
  SymTabEntry entry;
  _Opcode opcode;
  _EntryType et;

  while (e)
    {
      opcode = P_GetExprOpcode (e);

      /* OP_var and OP_stmt_expr have no operands to process. */
      if (opcode == OP_var)
	{
	  entry = PST_GetSymTabEntry (table, P_GetExprVarKey (e));

	  switch ((et = P_GetSymTabEntryType (entry)))
	    {
	    case ET_FUNC:
	      {
		FuncDcl f = P_GetSymTabEntryFuncDcl (entry);

		known = get_func_dcl_scope_externs (table, scope, f, known);
	      }
	      break;

	    case ET_VAR_LOCAL:
	    case ET_VAR_GLOBAL:
	      {
		VarDcl v = P_GetSymTabEntryVarDcl (entry);

		known = get_var_dcl_scope_externs (table, scope, v, known);
	      }
	      break;

	    default:
	      P_punt ("struct_symtab.c:get_expr_scope_externs:%d OP_var must "
		      "reference\nvariable or function, not %d", __LINE__ - 1,
		      et);
	    }
	}
      else if (opcode == OP_stmt_expr)
	{
	  known = get_stmt_scope_externs (table, scope, P_GetExprStmt (e),
					  known);
	}
      else
	{
	  Expr opd;

	  /* If this is an OP_arrow or OP_dot, check the referenced struct
	   * or union. */
	  if (opcode == OP_arrow || opcode == OP_dot)
	    {
	      Field f = PST_GetFieldEntry (table, P_GetExprVarKey (e));
	      entry = PST_GetSymTabEntry (table, P_GetFieldParentKey (f));

	      switch ((et = P_GetSymTabEntryType (entry)))
		{
		case ET_STRUCT:
		  {
		    StructDcl s = P_GetSymTabEntryStructDcl (entry);

		    known = get_struct_dcl_scope_externs (table, scope, s,
							  known);
		  }
		  break;

		case ET_UNION:
		  {
		    UnionDcl u = P_GetSymTabEntryUnionDcl (entry);

		    known = get_union_dcl_scope_externs (table, scope, u,
							 known);
		  }
		  break;

		default:
		  P_punt ("struct_symtab.c:get_expr_scope_externs:%d field "
			  "(%d, %d) parent must be struct or union, not %d",
			  __LINE__ - 2, P_GetFieldKey (f).file,
			  P_GetFieldKey (f).sym, et);
		}
	    }

	  for (opd = P_GetExprOperands (e); opd; opd = P_GetExprSibling (opd))
	    known = get_expr_scope_externs (table, scope, opd, known);
	}

      e = P_GetExprNext (e);
    }

  return (known);
}
/* @} */
