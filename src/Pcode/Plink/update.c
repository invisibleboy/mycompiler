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
 * \brief Functions to update the keys on Pcode structures.
 *
 * \author Robert Kidd and Wen-mei Hwu
 *
 * This file contains function definitions to update the keys on Pcode
 * structures.
 */

#include <config.h>
#include <string.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/query.h>
#include <Pcode/perror.h>
#include "main.h"
#include "update.h"
#include "copy.h"
#include "data.h"

/*! Constants to use for the mode argument to update_keys_in_type_dcl(). */
#define UPDATE             0
#define ADD_MULTI_TYPEDEFS 1

typedef struct link_local_entries_arg
{
  SymbolTable ip_table;         /*!< The interprocedural symbol table. */
  int source_file;              /*!< The source file key of the Stmt. */
} link_local_entries_arg;

static void link_local_entries_in_stmt (Stmt s, void *data);
static void update_keys_in_entry (SymbolTable ip_table, SymTabEntry entry);
static void update_keys_in_func_dcl (SymbolTable ip_table, int source_file_key,
				     FuncDcl func_dcl);
static void update_keys_in_type_dcl (SymbolTable ip_table, int source_file_key,
				     TypeDcl type_dcl, int mode);
static void update_keys_in_var_dcl (SymbolTable ip_table, int source_file_key,
				    VarDcl var_dcl);
static void update_keys_in_init (SymbolTable ip_table, int source_file_key,
				 Init init);
static void update_keys_in_su_dcl (SymbolTable ip_table, int source_file_key,
				   void *dcl, _EntryType type);
static void update_keys_in_field (SymbolTable ip_table, int source_file_key,
				  Field field);
static void update_keys_in_enum_dcl (SymbolTable ip_table, int source_file_key,
				     EnumDcl enum_dcl);
static void update_keys_in_asm_dcl (SymbolTable ip_table, int source_file_key,
				    AsmDcl asm_dcl);
static void update_keys_in_stmt (SymbolTable ip_table, int source_file_key,
				 Stmt stmt);
static void update_keys_in_label (SymbolTable ip_table, int source_file_key,
				  Label label);
static void update_keys_in_expr (SymbolTable ip_table, int source_file_key,
				 Expr expr);
static void update_keys_in_pragma (SymbolTable ip_table, int source_file_key,
				   Pragma pragma);

/*! \brief Updates the keys in all Pcode in the IP Table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * \a ip_table contains Pcode from every source file.  This Pcode has been
 * copied and merged, but the keys in the Pcode structures are still
 * from their original files.  The source file symbol tables are still open
 * and attached to \a ip_table.  These symbol tables contain the mapping
 * between source file keys and keys into \a ip_table.  We walk all Pcode
 * structures in \a ip_table and update the keys to refer to \a ip_table.
 *
 * This function assumes types have been ordered, which should happen
 * naturally.  The types are sorted in each source table before being
 * added to \a ip_table, so they should still be sorted at this point.
 */
void
Plink_UpdateKeys (SymbolTable ip_table)
{
  Key k;
  SymTabEntry e;
  int source_file;

  /* Update keys in TypeDcls. */
  for (k = PST_GetTableEntryByType (ip_table, ET_TYPE); P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k, ET_TYPE))
    {
      e = PST_GetSymTabEntry (ip_table, k);
      source_file = Plink_GetSymTabEntryFile (e);

      update_keys_in_type_dcl (ip_table, source_file,
			       P_GetSymTabEntryTypeDcl (e),
			       UPDATE);
    }

  /* Add multi type versions of typedefs and pointers that reference multi
   * types. */
  for (k = PST_GetTableEntryByType (ip_table, ET_TYPE); P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k, ET_TYPE))
    {
      e = PST_GetSymTabEntry (ip_table, k);
      source_file = Plink_GetSymTabEntryFile (e);

      update_keys_in_type_dcl (ip_table, source_file,
			       P_GetSymTabEntryTypeDcl (e),
			       ADD_MULTI_TYPEDEFS);
    }

  for (k = PST_GetTableEntryByType (ip_table, ET_ANY & ~ET_TYPE);
       P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k, ET_ANY & ~ET_TYPE))
    update_keys_in_entry (ip_table, PST_GetSymTabEntry (ip_table, k));

  return;
}

/*! \brief Links local symbols to their SymTabEntries.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * This function links symbols local to a statement (the statement itself,
 * types, variables, and labels) to their SymTabEntries.  This function
 * assumes the keys the symbols have _not_ been updated (ie, it is
 * necessary to call Plink_GetIPKey() to get the new key).
 */
void
Plink_LinkLocalEntries (SymbolTable ip_table)
{
  Key k;
  SymTabEntry e;
  int source_file;
  link_local_entries_arg arg;

  arg.ip_table = ip_table;

  /* Link local types and variables to their SymTabEntries. */
  for (k = PST_GetTableEntryByType (ip_table, ET_FUNC | ET_VAR_GLOBAL);
       P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k, ET_FUNC | ET_VAR_GLOBAL))
    {
      Init init;

      e = PST_GetSymTabEntry (ip_table, k);
      source_file = Plink_GetSymTabEntryFile (e);
      arg.source_file = source_file;

      if (P_GetSymTabEntryType (e) == ET_FUNC)
	{
	  FuncDcl func_dcl = P_GetSymTabEntryFuncDcl (e);
	  VarList var_list = P_GetFuncDclParam (func_dcl);
	  VarDcl var_dcl;
	  Key var_key, ip_var_key;
	  SymTabEntry var_entry;

	  List_start (var_list);
	  while ((var_dcl = (VarDcl)List_next (var_list)))
	    {
	      var_key = P_GetVarDclKey (var_dcl);

	      if (P_ValidKey (var_key))
		{
		  ip_var_key = Plink_GetIPKey (ip_table, source_file, var_key);
		  var_entry = PST_GetSymTabEntry (ip_table, ip_var_key);
		  P_SetSymTabEntryVarDcl (var_entry, var_dcl);
		}
	    }

	  P_StmtApplyPre (P_GetFuncDclStmt (func_dcl),
			  link_local_entries_in_stmt, NULL, &arg);
	}
      else if ((init = P_GetVarDclInit (P_GetSymTabEntryVarDcl (e))))
	{
	  P_InitApplyPre (init, link_local_entries_in_stmt, NULL, &arg);
	}
    }

  return;
}

/*! \brief Links symbols local to a Stmt to their SymTabEntries.
 *
 * \param s
 *  the statement to process.
 * \param data
 *  a pointer to a link_local_entries_arg struct.
 *
 * This function links symbols local to a statement (the statement itself,
 * types, variables, and labels) to their SymTabEntries.  This function
 * assumes the keys the symbols have _not_ been updated (ie, it is
 * necessary to call Plink_GetIPKey() to get the new key).
 *
 * This function is intended to be applied with P_StmtApply().
 */
static void
link_local_entries_in_stmt (Stmt s, void *data)
{
  link_local_entries_arg *arg = (link_local_entries_arg *)data;
  SymTabEntry e;
  Key k, ik;
  Label l;

  if (s)
    {
      k = P_GetStmtKey (s);

      if (P_ValidKey (k))
	{
	  ik = Plink_GetIPKey (arg->ip_table, arg->source_file, k);
	  e = PST_GetSymTabEntry (arg->ip_table, ik);
	  P_SetSymTabEntryStmt (e, s);
	}

      for (l = P_GetStmtLabels (s); l; l = P_GetLabelNext (l))
	{
	  k = P_GetLabelKey (l);

	  if (P_ValidKey (k))
	    {
	      ik = Plink_GetIPKey (arg->ip_table, arg->source_file, k);
	      e = PST_GetSymTabEntry (arg->ip_table, ik);
	      P_SetSymTabEntryLabel (e, l);
	    }
	}

      if (P_GetStmtType (s) == ST_COMPOUND)
	{
	  Compound c = P_GetStmtCompound (s);
	  TypeList type_list = P_GetCompoundTypeList (c);
	  VarList var_list = P_GetCompoundVarList (c);
	  TypeDcl type_dcl;
	  VarDcl var_dcl;

	  List_start (type_list);
	  while ((type_dcl = (TypeDcl)List_next (type_list)))
	    {
	      k = P_GetTypeDclKey (type_dcl);

	      if (P_ValidKey (k))
		{
		  ik = Plink_GetIPKey (arg->ip_table, arg->source_file, k);
		  e = PST_GetSymTabEntry (arg->ip_table, ik);
		  P_SetSymTabEntryTypeDcl (e, type_dcl);
		}
	    }
	  
	  List_start (var_list);
	  while ((var_dcl = (VarDcl)List_next (var_list)))
	    {
	      k = P_GetVarDclKey (var_dcl);

	      if (P_ValidKey (k))
		{
		  ik = Plink_GetIPKey (arg->ip_table, arg->source_file, k);
		  e = PST_GetSymTabEntry (arg->ip_table, ik);
		  P_SetSymTabEntryVarDcl (e, var_dcl);
		}
	    }
	}
    }

  return;
}

/*! \brief Update the keys in a SymTabEntry.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param entry
 *  the entry to update.
 *
 * \a entry is a SymTabEntry in which all keys still refer to the original
 * source file symbol table.  This function assumes that all entries in
 * the source file symbol tables are linked to entries in \a ip_table
 * (either in directly or other through other entries in other source file
 * symbol tables).  This function follows these links to update the keys in
 * \a entry.  After this function, all keys in \a entry will refer to
 * \a ip_table.
 */
static void
update_keys_in_entry (SymbolTable ip_table, SymTabEntry entry)
{
  _EntryType entry_type;
  int source_file_key = Plink_GetSymTabEntryFile (entry);

  switch ((entry_type = P_GetSymTabEntryType (entry)))
    {
    case ET_FUNC:
      update_keys_in_func_dcl (ip_table, source_file_key,
			       P_GetSymTabEntryFuncDcl (entry));
      break;
#if 0
    case ET_TYPE_GLOBAL:
      update_keys_in_type_dcl (ip_table, source_file_key,
			       P_GetSymTabEntryTypeDcl (entry), UPDATE);
      break;
#endif
    case ET_VAR_GLOBAL:
      update_keys_in_var_dcl (ip_table, source_file_key,
			      P_GetSymTabEntryVarDcl (entry));
      break;
    case ET_STRUCT:
      update_keys_in_su_dcl (ip_table, source_file_key,
			     P_GetSymTabEntryStructDcl (entry), entry_type);
      break;
    case ET_UNION:
      update_keys_in_su_dcl (ip_table, source_file_key,
			     P_GetSymTabEntryUnionDcl (entry), entry_type);
      break;
    case ET_ENUM:
      update_keys_in_enum_dcl (ip_table, source_file_key,
			       P_GetSymTabEntryEnumDcl (entry));
      break;
    case ET_ASM:
      update_keys_in_asm_dcl (ip_table, source_file_key,
			      P_GetSymTabEntryAsmDcl (entry));
      break;
    case ET_STMT:
    case ET_SCOPE:
    case ET_TYPE_LOCAL:
    case ET_VAR_LOCAL:
    case ET_FIELD:
    case ET_ENUMFIELD:
    case ET_EXPR:
    case ET_BLOCK:
    default:
      break;
    }

  return;
}

/*! \brief Updates the keys in a FuncDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the FuncDcl's original source file in \a ip_table.
 * \param func_dcl
 *  the FuncDcl to update.
 *
 * All keys in \a func_dcl refer to \a func_dcl's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a func_dcl.  After this function, all keys in \a func_dcl will
 * refer to \a ip_table.
 */
static void
update_keys_in_func_dcl (SymbolTable ip_table, int source_file_key,
			 FuncDcl func_dcl)
{
  SymTabEntry entry;
  VarList vl;
  VarDcl v;

  P_SetFuncDclKey (func_dcl, Plink_GetIPKey (ip_table, source_file_key,
					     P_GetFuncDclKey (func_dcl)));
  P_SetFuncDclType (func_dcl, Plink_GetIPKey (ip_table, source_file_key,
					      P_GetFuncDclType (func_dcl)));

  entry = PST_GetSymTabEntry (ip_table, P_GetFuncDclKey (func_dcl));

  /* We mangled the names of static variables when copying, so update the
   * VarDcl's name from its SymTabEntry. */
  if (strcmp (P_GetFuncDclName (func_dcl), P_GetSymTabEntryName (entry)) != 0)
    {
      free (P_GetFuncDclName (func_dcl));
      P_SetFuncDclName (func_dcl, strdup (P_GetSymTabEntryName (entry)));
    }

  vl = P_GetFuncDclParam (func_dcl);
  while ((v = (VarDcl)List_next (vl)))
    update_keys_in_var_dcl (ip_table, source_file_key, v);

  if (P_GetFuncDclStmt (func_dcl))
    update_keys_in_stmt (ip_table, source_file_key,
			 P_GetFuncDclStmt (func_dcl));

  if (P_GetFuncDclPragma (func_dcl))
    update_keys_in_pragma (ip_table, source_file_key,
			   P_GetFuncDclPragma (func_dcl));

  return;
}

/*! \brief Updates the keys in a TypeDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the TypeDcl's original source file in \a ip_table.
 * \param type_dcl
 *  the TypeDcl to update.
 * \param mode
 *  If UPDATE, the keys in the TypeDcl are updated.  If ADD_MULTI_TYPEDEFS,
 *  TypeDcl keys are not updated, but if \a type_dcl is of basic type
 *  BT_TYPEDEF or BT_POINTER and its referenced type is a multi type,
 *  a version of \a type_dcl that references the multi type is inserted.
 *
 * All keys in \a type_dcl refer to \a type_dcl's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a type_dcl.  After this function, all keys in \a type_dcl will
 * refer to \a ip_table.
 *
 * If \a mode is UPDATE, this function simply updates the keys in
 * \a type_dcl to be valid in \a ip_table.
 * If \a mode is ADD_MULTI_TYPEDEFS and \a type_dcl is a BT_TYPEDEF
 * or BT_POINTER type, this function assumes the keys in \a type_dcl have
 * already been updated.  If the type referenced by \a type_dcl has
 * a multi type indicated, a copy of \a type_dcl that references the
 * multi type is inserted.  In this case, this function assumes that
 * keys have not been updated.
 */
static void
update_keys_in_type_dcl (SymbolTable ip_table, int source_file_key,
			 TypeDcl type_dcl, int mode)
{
  Key multi_type_key;

  if (mode == UPDATE)
    {
      Key new_type_key;

      P_SetTypeDclKey (type_dcl, Plink_GetIPKey (ip_table, source_file_key,
						 P_GetTypeDclKey (type_dcl)));
      
      /* If the referenced TypeDcl has multi_type set on it's extension field,
       * use that instead of the TypeDcl's type.  multi_type will be a union
       * holding several struct pointers. */
      new_type_key = Plink_GetIPKey (ip_table, source_file_key,
				     P_GetTypeDclType (type_dcl));
      multi_type_key = Plink_GetTypeMultiType (ip_table, new_type_key);
      if (P_ValidKey (multi_type_key) && \
	  !(P_GetTypeDclBasicType (type_dcl) & (BT_TYPEDEF | BT_POINTER)))
	new_type_key = multi_type_key;
      P_SetTypeDclType (type_dcl, new_type_key);

      if (P_GetTypeDclBasicType (type_dcl) == BT_FUNC)
	{
	  Param p;
	  
	  for (p = P_GetTypeDclParam (type_dcl); p; p = P_GetParamNext (p))
	    {
	      new_type_key = Plink_GetIPKey (ip_table, source_file_key,
					     P_GetParamKey (p));
	      multi_type_key = Plink_GetTypeMultiType (ip_table, new_type_key);
	      if (P_ValidKey (multi_type_key))
		new_type_key = multi_type_key;
	      P_SetParamKey (p, new_type_key);
	    }
	}
      else if (P_GetTypeDclBasicType (type_dcl) == BT_ARRAY && \
	       P_GetTypeDclArraySize (type_dcl) != NULL)
	{
	  update_keys_in_expr (ip_table, source_file_key,
			       P_GetTypeDclArraySize (type_dcl));
	}
      
      if (P_GetTypeDclPragma (type_dcl))
	update_keys_in_pragma (ip_table, source_file_key,
			       P_GetTypeDclPragma (type_dcl));
    }
  else if (P_GetTypeDclBasicType (type_dcl) & (BT_TYPEDEF | BT_POINTER))
    {
      Key refd_type_key;

      refd_type_key = P_GetTypeDclType (type_dcl);
      multi_type_key = Plink_GetTypeMultiType (ip_table, refd_type_key);
      if (P_ValidKey (multi_type_key))
	{
	  TypeDcl alt_type_dcl = P_CopyTypeDcl (type_dcl);
	  Key alt_type_dcl_key = P_GetTypeDclKey (alt_type_dcl);
	  Key scope_key = PST_GetTypeDclScope (ip_table, alt_type_dcl);

	  alt_type_dcl_key.sym = 0;
	  P_SetTypeDclKey (alt_type_dcl, alt_type_dcl_key);
	  P_SetTypeDclType (alt_type_dcl, multi_type_key);

	  alt_type_dcl_key = PST_ScopeFindTypeDcl (ip_table, scope_key,
						   alt_type_dcl);
	  alt_type_dcl = P_RemoveTypeDcl (alt_type_dcl);

	  Plink_SetTypeDclMultiType (type_dcl, alt_type_dcl_key);
	}
    }

  return;
}

/*! \brief Updates the keys in a VarDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the VarDcl's original source file in \a ip_table.
 * \param var_dcl
 *  the VarDcl to update.
 *
 * All keys in \a var_dcl refer to \a var_dcl's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a var_dcl.  After this function, all keys in \a var_dcl will
 * refer to \a ip_table.
 */
static void
update_keys_in_var_dcl (SymbolTable ip_table, int source_file_key,
			VarDcl var_dcl)
{
  SymTabEntry entry;
  Key new_type_key, multi_type_key;

  P_SetVarDclKey (var_dcl, Plink_GetIPKey (ip_table, source_file_key,
					   P_GetVarDclKey (var_dcl)));

  /* If this is a local variable, link its symbol table entry to the
   * VarDcl. */
  entry = PST_GetSymTabEntry (ip_table, P_GetVarDclKey (var_dcl));

  /* We mangled the names of static variables when copying, so update the
   * VarDcl's name from its SymTabEntry. */
  if (strcmp (P_GetVarDclName (var_dcl), P_GetSymTabEntryName (entry)) != 0)
    {
      free (P_GetVarDclName (var_dcl));
      P_SetVarDclName (var_dcl, strdup (P_GetSymTabEntryName (entry)));
    }

  new_type_key = Plink_GetIPKey (ip_table, source_file_key,
				 P_GetVarDclType (var_dcl));
  multi_type_key = Plink_GetTypeMultiType (ip_table, new_type_key);
  if (P_ValidKey (multi_type_key))
    new_type_key = multi_type_key;
  P_SetVarDclType (var_dcl, new_type_key);

  if (P_GetVarDclInit (var_dcl))
    update_keys_in_init (ip_table, source_file_key, P_GetVarDclInit (var_dcl));

  if (P_GetVarDclPragma (var_dcl))
    update_keys_in_pragma (ip_table, source_file_key,
			   P_GetVarDclPragma (var_dcl));

  return;
}

/*! \brief Updates the keys in an Init.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the Init's original source file in \a ip_table.
 * \param init
 *  the Init to update.
 *
 * All keys in \a init refer to \a init's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a init.  After this function, all keys in \a init will
 * refer to \a ip_table.
 */
static void
update_keys_in_init (SymbolTable ip_table, int source_file_key, Init init)
{
  if (P_GetInitExpr (init))
    update_keys_in_expr (ip_table, source_file_key, P_GetInitExpr (init));

  if (P_GetInitSet (init))
    update_keys_in_init (ip_table, source_file_key, P_GetInitSet (init));

  if (P_GetInitNext (init))
    update_keys_in_init (ip_table, source_file_key, P_GetInitNext (init));

  return;
}

/*! \brief Updates the keys in a StructDcl or UnionDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the Init's original source file in \a ip_table.
 * \param dcl
 *  the StructDcl or UnionDcl to update.
 * \param type
 *  ET_STRUCT or ET_UNION
 *
 * All keys in \a dcl refer to \a dcl's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a dcl.  After this function, all keys in \a dcl will
 * refer to \a ip_table.
 */
static void
update_keys_in_su_dcl (SymbolTable ip_table, int source_file_key, void *dcl,
		       _EntryType type)
{
  Field f = NULL;
  Pragma p = NULL;

  if (type == ET_STRUCT)
    {
      StructDcl s = (StructDcl)dcl;

      P_SetStructDclKey (s, Plink_GetIPKey (ip_table, source_file_key,
					    P_GetStructDclKey (s)));

      f = P_GetStructDclFields (s);
      p = P_GetStructDclPragma (s);
    }
  else if (type == ET_UNION)
    {
      UnionDcl u = (UnionDcl)dcl;

      /* If this is a multi type union inserted when merging, it already
       * has keys valid in ip_table. */
      if (Plink_GetUnionDclMultiHash (u))
	return;

      P_SetUnionDclKey (u, Plink_GetIPKey (ip_table, source_file_key,
					   P_GetUnionDclKey (u)));

      f = P_GetUnionDclFields (u);
      p = P_GetUnionDclPragma (u);
    }
  else
    {
      P_punt ("update.c:update_keys_in_su_dcl:%d type must be ET_STRUCT or "
	      "ET_UNION,\nnot %d", __LINE__ - 1, type);
    }

  while (f)
    {
      update_keys_in_field (ip_table, source_file_key, f);
      f = P_GetFieldNext (f);
    }

  if (p)
    update_keys_in_pragma (ip_table, source_file_key, p);

  return;
}

/*! \brief Updates the keys in a Field.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the Field's original source file in \a ip_table.
 * \param field
 *  the Field to update.
 *
 * All keys in \a field refer to \a field's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a field.  After this function, all keys in \a field will
 * refer to \a ip_table.
 */
static void
update_keys_in_field (SymbolTable ip_table, int source_file_key, Field field)
{
  Key new_type_key, multi_type_key;

  P_SetFieldKey (field, Plink_GetIPKey (ip_table, source_file_key,
					P_GetFieldKey (field)));
  P_SetFieldParentKey (field, Plink_GetIPKey (ip_table, source_file_key,
					      P_GetFieldParentKey (field)));

  new_type_key = Plink_GetIPKey (ip_table, source_file_key,
				 P_GetFieldType (field));
  multi_type_key = Plink_GetTypeMultiType (ip_table, new_type_key);
  if (P_ValidKey (multi_type_key))
    new_type_key = multi_type_key;
  P_SetFieldType (field, new_type_key);
  
  if (P_GetFieldPragma (field))
    update_keys_in_pragma (ip_table, source_file_key,
			   P_GetFieldPragma (field));

  return;
}

/*! \brief Updates the keys in an EnumDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the EnumDcl's original source file in \a ip_table.
 * \param enum_dcl
 *  the EnumDcl to update.
 *
 * All keys in \a enum_dcl refer to \a enum_dcl's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a enum_dcl.  After this function, all keys in \a enum_dcl will
 * refer to \a ip_table.
 */
static void
update_keys_in_enum_dcl (SymbolTable ip_table, int source_file_key,
			 EnumDcl enum_dcl)
{
  EnumField f;

  P_SetEnumDclKey (enum_dcl, Plink_GetIPKey (ip_table, source_file_key,
					     P_GetEnumDclKey (enum_dcl)));

  for (f = P_GetEnumDclFields (enum_dcl); f; f = P_GetEnumFieldNext (f))
    {
      P_SetEnumFieldKey (f, Plink_GetIPKey (ip_table, source_file_key,
					    P_GetEnumFieldKey (f)));

      if (P_GetEnumFieldValue (f))
	update_keys_in_expr (ip_table, source_file_key,
			     P_GetEnumFieldValue (f));
    }

  if (P_GetEnumDclPragma (enum_dcl))
    update_keys_in_pragma (ip_table, source_file_key,
			   P_GetEnumDclPragma (enum_dcl));

  return;
}

/*! \brief Updates the keys in an AsmDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the AsmDcl's original source file in \a ip_table.
 * \param asm_dcl
 *  the AsmDcl to update.
 *
 * All keys in \a asm_dcl refer to \a asm_dcl's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a asm_dcl.  After this function, all keys in \a asm_dcl will
 * refer to \a ip_table.
 */
static void
update_keys_in_asm_dcl (SymbolTable ip_table, int source_file_key,
			AsmDcl asm_dcl)
{
  Expr clobbers, string, operands;

  P_SetAsmDclKey (asm_dcl, Plink_GetIPKey (ip_table, source_file_key,
					   P_GetAsmDclKey (asm_dcl)));

  if ((clobbers = P_GetAsmDclAsmClobbers (asm_dcl)))
    update_keys_in_expr (ip_table, source_file_key, clobbers);
  if ((string = P_GetAsmDclAsmString (asm_dcl)))
    update_keys_in_expr (ip_table, source_file_key, string);
  if ((operands = P_GetAsmDclAsmOperands (asm_dcl)))
    update_keys_in_expr (ip_table, source_file_key, operands);

  return;
}

/*! \brief Updates the keys in a Stmt.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the Stmt's original source file in \a ip_table.
 * \param stmt
 *  the Stmt to update.
 *
 * All keys in \a stmt refer to \a stmt's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a stmt.  After this function, all keys in \a stmt will
 * refer to \a ip_table.
 */
static void
update_keys_in_stmt (SymbolTable ip_table, int source_file_key, Stmt stmt)
{
  if (P_ValidKey (P_GetStmtKey (stmt)))
    P_SetStmtKey (stmt, Plink_GetIPKey (ip_table, source_file_key,
					P_GetStmtKey (stmt)));

  if (P_GetStmtPragma (stmt))
    update_keys_in_pragma (ip_table, source_file_key, P_GetStmtPragma (stmt));

  if (P_GetStmtLabels (stmt))
    update_keys_in_label (ip_table, source_file_key, P_GetStmtLabels (stmt));

  switch (P_GetStmtType (stmt))
    {
    case ST_NOOP:
      break;

    case ST_RETURN:
      if (P_GetStmtRet (stmt))
	update_keys_in_expr (ip_table, source_file_key, P_GetStmtRet (stmt));
      break;

    case ST_GOTO:
      P_SetStmtLabelKey (stmt, Plink_GetIPKey (ip_table, source_file_key,
					       P_GetStmtLabelKey (stmt)));
      break;

    case ST_COMPOUND:
      {
	Compound c = P_GetStmtCompound (stmt);
	Stmt stmt_list = P_GetCompoundStmtList (c);
#if 0
	TypeList type_list = P_GetCompoundTypeList (c);
#endif
	VarList var_list = P_GetCompoundVarList (c);
#if 0
	TypeDcl type_dcl;
#endif
	VarDcl var_dcl;

#if 0
	List_start (type_list);
	while ((type_dcl = (TypeDcl)List_next (type_list)))
	  update_keys_in_type_dcl (ip_table, source_file_key, type_dcl,
				   UPDATE);
#endif

	List_start (var_list);
	while ((var_dcl = (VarDcl)List_next (var_list)))
	  update_keys_in_var_dcl (ip_table, source_file_key, var_dcl);

	if (stmt_list)
	  update_keys_in_stmt (ip_table, source_file_key, stmt_list);
      }
      break;
	  
    case ST_IF:
      {
	IfStmt i = P_GetStmtIfStmt (stmt);
	Expr cond_expr = P_GetIfStmtCondExpr (i);
	Stmt then_block = P_GetIfStmtThenBlock (i);
	Stmt else_block = P_GetIfStmtElseBlock (i);

	if (cond_expr)
	  update_keys_in_expr (ip_table, source_file_key, cond_expr);
	if (then_block)
	  update_keys_in_stmt (ip_table, source_file_key, then_block);
	if (else_block)
	  update_keys_in_stmt (ip_table, source_file_key, else_block);
      }
      break;

    case ST_SWITCH:
      {
	SwitchStmt s = P_GetStmtSwitchStmt (stmt);
	Expr expression = P_GetSwitchStmtExpression (s);
	Stmt switch_body = P_GetSwitchStmtSwitchBody (s);

	if (expression)
	  update_keys_in_expr (ip_table, source_file_key, expression);
	if (switch_body)
	  update_keys_in_stmt (ip_table, source_file_key, switch_body);
      }
      break;

    case ST_PSTMT:
      {
	Pstmt p = P_GetStmtPstmt (stmt);
	Stmt st = P_GetPstmtStmt (p);
	
	if (st)
	  update_keys_in_stmt (ip_table, source_file_key, st);
      }
      break;

    case ST_COBEGIN:
      {
	Cobegin c = P_GetStmtCobegin (stmt);
	Stmt statements = P_GetCobeginStatements (c);

	if (statements)
	  update_keys_in_stmt (ip_table, source_file_key, statements);
      }
      break;

    case ST_PARLOOP:
      {
	ParLoop p = P_GetStmtParLoop (stmt);
	Stmt prologue_stmt = P_GetParLoopPrologueStmt (p);
	Expr iteration_var = P_GetParLoopIterationVar (p);
	Expr init_value = P_GetParLoopInitValue (p);
	Expr final_value = P_GetParLoopFinalValue (p);
	Expr incr_value = P_GetParLoopIncrValue (p);
	
	if (prologue_stmt)
	  update_keys_in_stmt (ip_table, source_file_key, prologue_stmt);
	if (iteration_var)
	  update_keys_in_expr (ip_table, source_file_key, iteration_var);
	if (init_value)
	  update_keys_in_expr (ip_table, source_file_key, init_value);
	if (final_value)
	  update_keys_in_expr (ip_table, source_file_key, final_value);
	if (incr_value)
	  update_keys_in_expr (ip_table, source_file_key, incr_value);
      }
      break;

    case ST_SERLOOP:
      {
	SerLoop s = P_GetStmtSerLoop (stmt);
	Stmt loop_body = P_GetSerLoopLoopBody (s);
	Expr cond_expr = P_GetSerLoopCondExpr (s);
	Expr init_expr = P_GetSerLoopInitExpr (s);
	Expr iter_expr = P_GetSerLoopIterExpr (s);

	if (loop_body)
	  update_keys_in_stmt (ip_table, source_file_key, loop_body);
	if (cond_expr)
	  update_keys_in_expr (ip_table, source_file_key, cond_expr);
	if (init_expr)
	  update_keys_in_expr (ip_table, source_file_key, init_expr);
	if (iter_expr)
	  update_keys_in_expr (ip_table, source_file_key, iter_expr);
      }
      break;

    case ST_EXPR:
      {
	Expr e = P_GetStmtExpr (stmt);

	if (e)
	  update_keys_in_expr (ip_table, source_file_key, e);
      }
      break;

    case ST_BODY:
      {
	BodyStmt b = P_GetStmtBodyStmt (stmt);
	Stmt statement = P_GetBodyStmtStatement (b);

	if (statement)
	  update_keys_in_stmt (ip_table, source_file_key, statement);
      }
      break;

    case ST_EPILOGUE:
      {
	EpilogueStmt e = P_GetStmtEpilogueStmt (stmt);
	Stmt statement = P_GetEpilogueStmtStatement (e);

	if (statement)
	  update_keys_in_stmt (ip_table, source_file_key, statement);
      }
      break;

    case ST_ASM:
      {
	AsmStmt a = P_GetStmtAsmStmt (stmt);
	Expr clobbers = P_GetAsmStmtAsmClobbers (a);
	Expr string = P_GetAsmStmtAsmString (a);
	Expr operands = P_GetAsmStmtAsmOperands (a);
	
	if (clobbers)
	  update_keys_in_expr (ip_table, source_file_key, clobbers);
	if (string)
	  update_keys_in_expr (ip_table, source_file_key, string);
	if (operands)
	  update_keys_in_expr (ip_table, source_file_key, operands);
      }
      break;

    case ST_CONT:
    case ST_BREAK:
    case ST_ADVANCE:
    case ST_AWAIT:
    case ST_MUTEX:
      break;

    default:
      P_punt ("update.c:update_keys_in_stmt:%d Unknown _StmtType %d",
	      __LINE__ - 1, P_GetStmtType (stmt));
    }

  if (P_GetStmtLexNext (stmt))
    update_keys_in_stmt (ip_table, source_file_key, P_GetStmtLexNext (stmt));

  return;
}

/*! \brief Updates the keys in a Label.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the Label's original source file in \a ip_table.
 * \param label
 *  the Label to update.
 *
 * All keys in \a label refer to \a label's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a label.  After this function, all keys in \a label will
 * refer to \a ip_table.
 */
static void
update_keys_in_label (SymbolTable ip_table, int source_file_key, Label label)
{
  Expr expression;
  Label next;

  if (P_ValidKey (P_GetLabelKey (label)))
    P_SetLabelKey (label, Plink_GetIPKey (ip_table, source_file_key,
					  P_GetLabelKey (label)));

  if (P_GetLabelType (label) == LB_CASE && \
      (expression = P_GetLabelExpression (label)))
    update_keys_in_expr (ip_table, source_file_key, expression);

  if ((next = P_GetLabelNext (label)))
    update_keys_in_label (ip_table, source_file_key, next);

  return;
}

/*! \brief Updates the keys in an Expr.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the Expr's original source file in \a ip_table.
 * \param expr
 *  the Expr to update.
 *
 * All keys in \a expr refer to \a expr's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a expr.  After this function, all keys in \a expr will
 * refer to \a ip_table.
 *
 * This function also ensures that the var.name field in OP_vars matches
 * the name in the symbol table.
 */
static void
update_keys_in_expr (SymbolTable ip_table, int source_file_key, Expr expr)
{
  Key new_type_key, multi_type_key;

  while (expr)
    {
      P_SetExprKey (expr, Plink_GetIPKey (ip_table, source_file_key,
					  P_GetExprKey (expr)));

      new_type_key = Plink_GetIPKey (ip_table, source_file_key,
				     P_GetExprType (expr));
      multi_type_key = Plink_GetTypeMultiType (ip_table, new_type_key);
      /* Casts and constants should not be updated to the multi union type. */
      if (P_ValidKey (multi_type_key) && \
	  !(P_GetExprOpcode (expr) == OP_cast || P_IsConstNumber (expr)))
	new_type_key = multi_type_key;
      P_SetExprType (expr, new_type_key);
  
      switch (P_GetExprOpcode (expr))
	{
	case OP_enum:
	case OP_int:
	case OP_real:
	case OP_error:
	case OP_char:
	case OP_string:
	case OP_expr_size:
	case OP_type_size:
	case OP_float:
	case OP_double:
	case OP_null:
	case OP_sync:
	case OP_asm_oprd:
	  break;
	  
	case OP_var:
	  {
	    SymTabEntry var_entry;

	    P_SetExprVarKey (expr, Plink_GetIPKey (ip_table, source_file_key,
						   P_GetExprVarKey (expr)));
	    var_entry = PST_GetSymTabEntry (ip_table, P_GetExprVarKey (expr));

	    if (strcmp (P_GetExprVarName (expr),
			P_GetSymTabEntryName (var_entry)) != 0)
	      {
		free (P_GetExprVarName (expr));
		P_SetExprVarName (expr,
				  strdup (P_GetSymTabEntryName (var_entry)));
	      }
	  }
	  break;

	case OP_dot:
	case OP_arrow:
	  P_SetExprVarKey (expr, Plink_GetIPKey (ip_table, source_file_key,
						 P_GetExprVarKey (expr)));
	  /* fall through to process the operand. */

	case OP_cast:
	case OP_neg:
	case OP_not:
	case OP_inv:
	case OP_preinc:
	case OP_predec:
	case OP_postinc:
	case OP_postdec:
	case OP_indr:
	case OP_addr:
	case OP_compexpr: /* expr->next processed by while loop. */
	  update_keys_in_expr (ip_table, source_file_key,
			       P_GetExprOperands (expr));
	  break;
      
	case OP_quest:
	  {
	    Expr operand0 = P_GetExprOperands (expr);
	    Expr operand1 = P_GetExprSibling (operand0);
	    Expr operand2 = P_GetExprSibling (operand1);
	
	    update_keys_in_expr (ip_table, source_file_key, operand0);
	    update_keys_in_expr (ip_table, source_file_key, operand1);
	    update_keys_in_expr (ip_table, source_file_key, operand2);
	  }
	  break;
      
	case OP_disj:
	case OP_conj:
	case OP_assign:
	case OP_or:
	case OP_xor:
	case OP_and:
	case OP_eq:
	case OP_ne:
	case OP_lt:
	case OP_le:
	case OP_ge:
	case OP_gt:
	case OP_rshft:
	case OP_lshft:
	case OP_add:
	case OP_sub:
	case OP_mul:
	case OP_div:
	case OP_mod:
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
	case OP_index:
	  {
	    Expr operand0 = P_GetExprOperands (expr);
	    Expr operand1 = P_GetExprSibling (operand0);
	
	    update_keys_in_expr (ip_table, source_file_key, operand0);
	    update_keys_in_expr (ip_table, source_file_key, operand1);
	  }
	  break;
      
	case OP_call:
	  {
	    Expr callee = P_GetExprOperands (expr);
	    Expr arguments = P_GetExprSibling (callee);
	
	    update_keys_in_expr (ip_table, source_file_key, callee);
	
	    /* The while loop in update_keys_in_expr() will take care
	     * of all arguments. */
	    update_keys_in_expr (ip_table, source_file_key, arguments);
	  }
	  break;
	  
	case OP_stmt_expr:
	  update_keys_in_stmt (ip_table, source_file_key,
			       P_GetExprStmt (expr));
	  break;

	default:
	  P_punt ("update.c:update_keys_in_expr:%d Unknown Expr opcode %d",
		  __LINE__ - 1, P_GetExprOpcode (expr));
	}
  
      if (P_GetExprPragma (expr))
	update_keys_in_pragma (ip_table, source_file_key,
			       P_GetExprPragma (expr));

      expr = P_GetExprNext (expr);
    }

  return;
}

/*! \brief Updates the keys in a Pragma.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the file key of the Pragma's original source file in \a ip_table.
 * \param pragma
 *  the Pragma to update.
 *
 * All keys in \a pragma refer to \a pragma's original source
 * table.  This function assumes that all entries in the source file
 * symbol tables are linked to entries in \a ip_table (either in
 * directly or other through other entries in other source file symbol
 * tables).  This function follows these links to update the keys in
 * \a pragma.  After this function, all keys in \a pragma will
 * refer to \a ip_table.
 */
static void
update_keys_in_pragma (SymbolTable ip_table, int source_file_key,
		       Pragma pragma)
{
  if (pragma == NULL)
    return;

  update_keys_in_expr (ip_table, source_file_key, P_GetPragmaExpr (pragma));

  if (P_GetPragmaNext (pragma))
    update_keys_in_pragma (ip_table, source_file_key,
			   P_GetPragmaNext (pragma));

  return;
}

