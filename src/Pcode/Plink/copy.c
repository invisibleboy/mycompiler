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
 * \brief Functions to copy symbols between tables.
 *
 * \author Robert Kidd and Wen-mei Hwu
 *
 * This file contains function definitions to copy symbols between tables.
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/perror.h>
#include <Pcode/query.h>
#include "copy.h"
#include "link.h"
#include "merge.h"
#include "data.h"
#include "main.h"

static void copy_fields (SymbolTable ip_table, SymTabEntry source_entry);

/*! \brief Copies a SymTabEntry from a source file to the IP Table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the SymTabEntry to copy to \a ip_table.
 *
 * \return
 *  \a source_entry's key in \a ip_table.
 *
 * Copies \a source_entry into \a ip_table.  Sets \a source_entry's
 * ext.Plink_Key to its key in \a ip_table.  Sets the copy's ext.Plink_Key
 * to \a source_entry's file and key.
 *
 * This function results in a new entry in \a ip_table that is attached
 * to Pcode.  The keys in the Pcode have not been updated, so they
 * still refer to \a source_entry's Source Table.  The SymTabEntry's key
 * and scope_key have both been updated.  If a SymTabEntry has a scope,
 * it is updated as Pcode is copied into \a ip_table.
 */
Key
Plink_CopyEntry (SymbolTable ip_table, SymTabEntry source_entry)
{
  const char *static_tag = "_global_static_";
  SymTabEntry ip_entry;
  Key ip_entry_key = Invalid_Key;
  Key source_entry_key = P_GetSymTabEntryKey (source_entry);
  Key ip_scope_key, source_scope_key;
  int source_file_key = Plink_GetSymTabEntrySrcFile (source_entry);

  if ((ip_entry = P_CopySymTabEntry (source_entry)))
    {
      /* The scope will be completely rebuilt, so we don't need the copy. */
      P_SetSymTabEntryScope (ip_entry,
			     P_RemoveScope (P_GetSymTabEntryScope (ip_entry)));
      
      /* It is possible that a mapping for the entry's source key already
       * exists.  If this entry is a scope, func, or stmt, it is possible
       * that one of the symbols under the scope has already been processed,
       * in which case a scope will have already been inserted in ip_table.
       * If there is already an ip_table_key, we'll use that key when
       * inserting this symbol, and we'll preserve its scope. */
      ip_entry_key = Plink_GetLinkKey (ip_table, source_file_key,
				       source_entry_key);

      if (P_ValidKey (ip_entry_key))
	{
	  SymTabEntry old_ip_entry = PST_GetSymTabEntry (ip_table,
							 ip_entry_key);

	  /* The old entry must be a scope and the new must be a scope,
	   * function, or statement for this replacement to be valid. */
	  if (!((P_GetSymTabEntryType (ip_entry) & \
		 (ET_FUNC | ET_STMT | ET_SCOPE)) && \
		(P_GetSymTabEntryType (old_ip_entry) == ET_SCOPE)))
	    P_punt ("copy.c:Plink_CopyEntry:%d Can't replace entry of type "
		    "%d with %d", __LINE__ - 1,
		    P_GetSymTabEntryType (old_ip_entry),
		    P_GetSymTabEntryType (ip_entry));

	  P_SetSymTabEntryScope (ip_entry,
				 P_GetSymTabEntryScope (old_ip_entry));
	  P_SetSymTabEntryScope (old_ip_entry, NULL);
	  PST_RemoveEntry (ip_table, ip_entry_key);
	}
      else
	{
	  /* Change ip_entry's key. */
	  ip_entry_key.file = source_file_key;
	  ip_entry_key.sym = 0;
	}

      /* If the entry should be copied directly to the global scope, do
       * that. */
      if (Plink_GetSymTabEntryFlags (source_entry) & PLF_TO_GLOBAL)
	{
	  ip_entry_key.file = global_header_key;
	  ip_entry_key.sym = 0;
	}

      P_SetSymTabEntryKey (ip_entry, ip_entry_key);
      Plink_SetSymTabEntrySrcFile (ip_entry, 0);

      /* Insert the new entry into ip_table. */
      ip_entry_key = PST_AddSymTabEntry (ip_table, ip_entry);

      /* If the entry had a scope, add a scope for it in ip_table.
       * (if we haven't copied an old scope). */
      if (P_GetSymTabEntryScope (source_entry) && \
	  !(P_GetSymTabEntryScope (ip_entry)))
	P_SetSymTabEntryScope (ip_entry, P_NewScopeWithKey (ip_entry_key));

      /* Insert ip_entry into the scope in ip_table that corresponds to
       * source_entry's scope in its Source Table. */
      source_scope_key = P_GetSymTabEntryScopeKey (source_entry);
      if (P_ValidKey (source_scope_key))
	{
	  /* If the entry should be copied directly to the global scope,
	   * enter it in the global scope. */
	  if (Plink_GetSymTabEntryFlags (source_entry) & PLF_TO_GLOBAL)
	    ip_scope_key = global_scope_key;
	  else
	    ip_scope_key = Plink_GetLinkKey (ip_table, source_file_key,
					     source_scope_key);

	  /* Add a scope if necessary. */
	  if (!P_ValidKey (ip_scope_key))
	    {
	      ip_scope_key.file = ip_entry_key.file;
	      ip_scope_key.sym = 0;

	      ip_scope_key = PST_AddNewScope (ip_table, ip_scope_key);

	      Plink_SetLink (ip_table, source_file_key, source_scope_key,
			     0, ip_scope_key, PLF_LT_IP);
	      Plink_SetLink (ip_table, 0, ip_scope_key, source_file_key,
			     source_scope_key, PLF_LT_SOURCE);
	    }

	  PST_AddEntryToScope (ip_table, ip_scope_key, ip_entry_key);
	}

      /* Link source_entry to ip_entry through the ext field. */
      Plink_SetLink (ip_table, source_file_key, source_entry_key, 0,
		     ip_entry_key, PLF_LT_IP);
      Plink_SetLink (ip_table, 0, ip_entry_key, source_file_key,
		     source_entry_key, PLF_LT_SOURCE);

      /* If the source entry has PLF_LONELY_ST set, set it on the copy. */
      if (Plink_GetSymTabEntryFlags (source_entry) & PLF_LONELY_ST)
	Plink_SetSymTabEntryFlags (ip_entry, PLF_LONELY_ST);

      /* If we just copied a struct or union, copy its fields. */
      if (P_GetSymTabEntryType (ip_entry) & (ET_STRUCT | ET_UNION))
	copy_fields (ip_table, ip_entry);

      /* If we just copied a static variable, mangle the name.  Note
       * that we test the source_entry, not ip_entry.  This is because
       * local variables have not been reconnected to their SymTabEntries
       * at this point.  We also change the name on the SymTabEntry only.
       * update.c:update_keys_in_var_dcl() will propagate this change
       * to the VarDcl, and update_keys_in_expr() will update the OP_vars. */
      if ((P_GetSymTabEntryType (source_entry) & ET_VAR) && \
	  P_TstVarDclQualifier (P_GetSymTabEntryVarDcl (source_entry),
				VQ_STATIC))
	{
	  int new_name_len;
	  char *new_name;

	  /* If this is a local variable, edg already prepended
	   * _local_static_<func> to the name.  For globals, we
	   * need to append _local_global_<file_key>. */
	  if (P_GetSymTabEntryType (source_entry) == ET_VAR_GLOBAL)
	    {
	      new_name_len = strlen (static_tag) + \
		             strlen (P_GetSymTabEntryName (ip_entry)) + \
		             34; /* 30 digits for 3 ints + 3 x _ + \0 */
	      new_name = malloc (new_name_len);
	      snprintf (new_name, new_name_len, "%s%d_%s_%d_%d", static_tag,
			source_file_key, P_GetSymTabEntryName (ip_entry),
			ip_entry_key.file, ip_entry_key.sym);
	    }
	  else
	    {
	      new_name_len = strlen (P_GetSymTabEntryName (ip_entry)) + \
		             23; /* 20 digits for 2 ints + 2 x _ + \0 */
	      new_name = malloc (new_name_len);
	      snprintf (new_name, new_name_len, "%s_%d_%d",
			P_GetSymTabEntryName (ip_entry), ip_entry_key.file,
			ip_entry_key.sym);
	    }
	  
	  free (P_GetSymTabEntryName (ip_entry));
	  P_SetSymTabEntryName (ip_entry, new_name);
	}

      if ((P_GetSymTabEntryType (source_entry) & ET_FUNC) && \
	  P_TstFuncDclQualifier (P_GetSymTabEntryFuncDcl (source_entry),
				 VQ_STATIC))
	{
	  int new_name_len;
	  char *new_name;

	  /* We need to append _local_global_<file_key>. */

	  new_name_len = strlen (static_tag) + \
	    strlen (P_GetSymTabEntryName (ip_entry)) + \
	    34; /* 30 digits for 3 ints + 3 x _ + \0 */
	  new_name = malloc (new_name_len);
	  snprintf (new_name, new_name_len, "%s%d_%s_%d_%d", static_tag,
		    source_file_key, P_GetSymTabEntryName (ip_entry),
		    ip_entry_key.file, ip_entry_key.sym);
	  
	  free (P_GetSymTabEntryName (ip_entry));
	  P_SetSymTabEntryName (ip_entry, new_name);
	}

      Plink_SetSymTabEntryFlags (source_entry, PLF_COPIED);
    }

  return (ip_entry_key);
}

/*! \brief Copies Pcode from the source table to the IP table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the source file's key in \a ip_table.
 *
 * Copies Pcode symbols from \a src_table to file \a source_file_key
 * in \a ip_table.
 *
 * If the source_table entry is a function, global variable, or
 * global type, ip_table is searched for a previous definition to link to
 * or merge with.  If there is no definition, the entry is simply
 * copied to ip_table, like all other entries.
 *
 * Local variables and types remain attached to their compound statements
 * and are not linked or merged.
 */
void
Plink_CopySymbols (SymbolTable ip_table, int source_file_key)
{
  SymTabEntry src_entry;
  SymbolTable source_table;
  Key src_entry_key = {1, 0}, ip_entry_key;
  bool linked, merged;

  source_table = Plink_GetSourceTable (ip_table, source_file_key);

  /* Source files only have a single IPSymTabEnt. */
  if (P_GetSymbolTableNumFiles (source_table) > 1)
    P_warn ("copy.c:Plink_copy_symbols:%d Source Table has %d IPSymTabEnts",
	    __LINE__ - 1, P_GetSymbolTableNumFiles (source_table));

  for (src_entry_key = PST_GetFileEntryByType (source_table, 1, ET_ANY);
       P_ValidKey (src_entry_key);
       src_entry_key = PST_GetFileEntryByTypeNext (source_table, src_entry_key,
						   ET_ANY))
    {
      linked = FALSE;
      merged = FALSE;
      
      if ((src_entry = PST_GetSymTabEntry (source_table, src_entry_key)))
	{
	  /* Skip this entry if it has already been copied. */
	  if (Plink_GetSymTabEntryFlags (src_entry) & \
	      (PLF_COPIED | PLF_MERGED | PLF_LINKED))
	    continue;

	  if (P_GetSymTabEntryType (src_entry) & (ET_FUNC | ET_VAR_GLOBAL))
	    linked = Plink_AttemptLink (ip_table, src_entry);
	  else if (P_GetSymTabEntryType (src_entry) & \
		   (ET_TYPE_GLOBAL | ET_STRUCT | ET_UNION))
	    merged = Plink_AttemptMerge (ip_table, src_entry);
	    
	  /* If we haven't found a previous definition for this entry,
	   * copy it. */
	  if (!linked && !merged)
	    {
	      ip_entry_key = Plink_CopyEntry (ip_table, src_entry);

	      /* Symbols {1, 2} through {1, 17} are the fundamental types.
	       * If we just copied one, it should immediately be promoted
	       * to the global scope. */
	      if (src_entry_key.sym >= 2 && src_entry_key.sym <= 17)
		ip_entry_key = Plink_MoveToGlobalScope (ip_table,
							ip_entry_key);
	    }
	}
    }

  return;
}

/*! \brief Moves a symbol from its source file to the global scope.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param ip_entry_key
 *  the symbol's key in \a ip_table.
 *
 * \return
 *  The symbol's new key in the global scope.
 *
 * Moves a symbol from its source file to the global scope.  The
 * corresponding symbol in the original Source Table is linked to
 * the moved symbol.
 */
Key
Plink_MoveToGlobalScope (SymbolTable ip_table, Key ip_entry_key)
{
  SymTabEntry ip_entry, refd_entry;
  Key source_entry_key, refd_key, ip_refd_key, orig_ip_entry_key;
  int source_file_key;

  source_file_key = Plink_GetLinkFile (ip_table, 0, ip_entry_key);
  source_entry_key = Plink_GetLinkKey (ip_table, 0, ip_entry_key);

  /* ip_entry is in a file scope.  We need to move it to the global
   * scope. */
  orig_ip_entry_key = ip_entry_key;
  ip_entry_key = PST_MoveEntry (ip_table, orig_ip_entry_key, global_scope_key);

  /* When we first copied ip_entry, we linked its original source entry
   * to ip_entry and ip_entry to the original source entry.  We just
   * moved ip_entry, so we need to update this link. */
  Plink_SetLink (ip_table, source_file_key, source_entry_key,
		 0, ip_entry_key, PLF_LT_IP);

  /* If we just moved a struct or union, move its fields as well. 
   * If we just moved a type that references another type, move the 
   * referenced type. */
  ip_entry = PST_GetSymTabEntry (ip_table, ip_entry_key);
  switch (P_GetSymTabEntryType (ip_entry))
    {
    case ET_TYPE_GLOBAL:
      {
	TypeDcl t = P_GetSymTabEntryTypeDcl (ip_entry);
	Key multi_type = Plink_GetTypeDclMultiType (t);

	/* If the TypeDcl has a multi_type, search for its key in the
	 * multi type's union and update it. */
	if (P_ValidKey (multi_type))
	  {
	    Plink_UpdateMultiType (ip_table, multi_type, orig_ip_entry_key,
				   ip_entry_key);
	  }

	refd_key = P_GetTypeDclType (t);

	/* If this type references another type, move that referenced type
	 * to the global scope.  If the referenced type hasn't been copied
	 * to ip_table, set the PLF_TO_GLOBAL flag on its source table entry.
	 * When that source table entry is copied, the PLF_TO_GLOBAL flag
	 * will make Plink_CopyEntry copy it directly to the global scope. */
	if (P_ValidKey (refd_key))
	  {
	    ip_refd_key = Plink_GetIPKey (ip_table, source_file_key, refd_key);
	  
	    if (P_ValidKey (ip_refd_key))
	      {
		if (ip_refd_key.file != global_header_key)
		  Plink_MoveToGlobalScope (ip_table, ip_refd_key);
	      }
	    else
	      {
		refd_entry = Plink_GetSymTabEntry (ip_table, source_file_key,
						   refd_key);
		Plink_SetSymTabEntryFlags (refd_entry, PLF_TO_GLOBAL);
	      }
	  }

	if (P_GetTypeDclBasicType (t) & BT_FUNC)
	  {
	    Param p;

	    for (p = P_GetTypeDclParam (t); p; p = P_GetParamNext (p))
	      {
		/* If this type references another type, move that
		 * referenced type to the global scope.  If the
		 * referenced type hasn't been copied to ip_table, set
		 * the PLF_TO_GLOBAL flag on its source table entry.
		 * When that source table entry is copied, the
		 * PLF_TO_GLOBAL flag will make Plink_CopyEntry copy
		 * it directly to the global scope. */
		if (P_ValidKey (P_GetParamKey (p)))
		  {
		    ip_refd_key = Plink_GetIPKey (ip_table, source_file_key,
						  refd_key);

		    if (P_ValidKey (ip_refd_key))
		      {
			if (ip_refd_key.file != global_header_key)
			  Plink_MoveToGlobalScope (ip_table, ip_refd_key);
		      }
		    else
		      {
			refd_entry = Plink_GetSymTabEntry (ip_table,
							   source_file_key,
							   refd_key);
			Plink_SetSymTabEntryFlags (refd_entry, PLF_TO_GLOBAL);
		      }
		  }
	      }
	  }
      }
      break;
    case ET_FUNC:
      refd_key = P_GetFuncDclType (P_GetSymTabEntryFuncDcl (ip_entry));
      ip_refd_key = Plink_GetIPKey (ip_table, source_file_key, refd_key);

      /* If this FuncDcl references another type, move that referenced type
       * to the global scope.  If the referenced type hasn't been copied
       * to ip_table, set the PLF_TO_GLOBAL flag on its source table entry.
       * When that source table entry is copied, the PLF_TO_GLOBAL flag
       * will make Plink_CopyEntry copy it directly to the global scope. */
      if (P_ValidKey (ip_refd_key))
	{
	  if (ip_refd_key.file != global_header_key)
	    Plink_MoveToGlobalScope (ip_table, ip_refd_key);
	}
      else
	{
	  refd_entry = Plink_GetSymTabEntry (ip_table, source_file_key,
					     refd_key);
	  Plink_SetSymTabEntryFlags (refd_entry, PLF_TO_GLOBAL);
	}
      break;
    case ET_VAR_GLOBAL:
      refd_key = P_GetVarDclType (P_GetSymTabEntryVarDcl (ip_entry));
      ip_refd_key = Plink_GetIPKey (ip_table, source_file_key, refd_key);
	
      /* If this VarDcl references another type, move that referenced type
       * to the global scope.  If the referenced type hasn't been copied
       * to ip_table, set the PLF_TO_GLOBAL flag on its source table entry.
       * When that source table entry is copied, the PLF_TO_GLOBAL flag
       * will make Plink_CopyEntry copy it directly to the global scope. */
      if (P_ValidKey (ip_refd_key))
	{
	  if (ip_refd_key.file != global_header_key)
	    Plink_MoveToGlobalScope (ip_table, ip_refd_key);
	}
      else
	{
	  refd_entry = Plink_GetSymTabEntry (ip_table, source_file_key,
					     refd_key);
	  Plink_SetSymTabEntryFlags (refd_entry, PLF_TO_GLOBAL);
	}
      break;
    case ET_STRUCT:
    case ET_UNION:
      {
	Field f = P_GetSymTabEntryFields (ip_entry);
	
	while (f)
	  {
	    Key ip_field_key = Plink_GetIPKey (ip_table, source_file_key,
						 P_GetFieldKey (f));
	    Plink_MoveToGlobalScope (ip_table, ip_field_key);
	    f = P_GetFieldNext (f);
	  }
      }      
      break;
    case ET_FIELD:
      refd_key = P_GetFieldType (P_GetSymTabEntryField (ip_entry));
      ip_refd_key = Plink_GetIPKey (ip_table, source_file_key, refd_key);

      /* If this Field references another type, move that referenced type
       * to the global scope.  If the referenced type hasn't been copied
       * to ip_table, set the PLF_TO_GLOBAL flag on its source table entry.
       * When that source table entry is copied, the PLF_TO_GLOBAL flag
       * will make Plink_CopyEntry copy it directly to the global scope. */
      if (P_ValidKey (ip_refd_key))
	{
	  if (ip_refd_key.file != global_header_key)
	    Plink_MoveToGlobalScope (ip_table, ip_refd_key);
	}
      else
	{
	  refd_entry = Plink_GetSymTabEntry (ip_table, source_file_key,
					     refd_key);
	  Plink_SetSymTabEntryFlags (refd_entry, PLF_TO_GLOBAL);
	}
      break;
    default:
      break;
    }

  return (ip_entry_key);
}

/*! \brief Removes a symbol from the interprocedural symbol table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param symbol_file_key
 *  the file key in \a ip_table of the source file for the symbol to remove.
 * \param symbol_entry_key
 *  the Source Table key of the symbol to remove.
 *
 * Uses a key into a Source Table to remove a symbol from \a ip_table.
 *
 * \note When Plink_CopyEntry() copies a FuncDcl, it copies the function's
 *       TypeDcl as well.  If this function needed to work on functions,
 *       it would have to remove the function's type.  However, this function
 *       is only called when we are replacing a common symbol with a defined
 *       one.  Functions can not be common, so this function will not be
 *       called on a function.
 */
void
Plink_RemoveEntry (SymbolTable ip_table, int symbol_file_key,
		   Key symbol_entry_key)
{
  Key ip_key;
  int ip_file;
  Scope s;
  ScopeEntry se;
  SymTabEntry e;

  if (symbol_file_key != 0)
    {
      ip_key = Plink_GetLinkKey (ip_table, symbol_file_key, symbol_entry_key);
      ip_file = Plink_GetLinkFile (ip_table, symbol_file_key,
				   symbol_entry_key);
      
      if (ip_file != 0)
	P_punt ("link.c:remove:%d (%d, %d, %d) does not exist in ip_table\n"
		"(linked to (%d, %d, %d))", __LINE__ - 1, symbol_file_key,
		symbol_entry_key.file, symbol_entry_key.sym, ip_file,
		ip_key.file, ip_key.sym);
    }
  else
    {
      ip_key = symbol_entry_key;
      ip_file = symbol_file_key;
    }

  if ((e = PST_GetSymTabEntry (ip_table, ip_key)))
    {
      if ((s = P_GetSymTabEntryScope (e)))
	{
	  ScopeEntry sen;

	  for (se = P_GetScopeScopeEntry (s); se; se = sen)
	    {
	      sen = P_GetScopeEntryNext (se);
	      Plink_RemoveEntry (ip_table, 0, P_GetScopeEntryKey (se));
	    }
	}

      if (P_ValidKey (P_GetSymTabEntryScopeKey (e)))
	{
	  PST_RemoveEntryFromScope (ip_table, P_GetSymTabEntryScopeKey (e),
				    ip_key);
	}

      /* We must use P_RemoveSymTabEntry() here instead of
       * PST_RemoveSymTabEntry().  The keys in the Pcode structures in
       * ip_table have not been updated, so PST_RemoveSymTabEntry() won't
       * be able to find the correct symbols to free.  This is not a
       * problem as this function knows the symbol's key, and therefore its
       * scope.  This function removes all entries that would be removed
       * by PST_RemoveSymTabEntry(). */
      e = P_RemoveSymTabEntry (e);
    }

  PST_ClearEntry (ip_table, ip_key);

  return;
}

/*! \brief Copies the fields for a struct or union to the IP Table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param ip_entry
 *  the SymTabEntry for the struct or union in \a ip_table.
 *
 * When the struct or union is copied, its fields are copied with it.  The
 * fields' SymTabEntries are not copied.  This function copies the field
 * SymTabEntries and links them to the Fields in \a ip_table.
 */
static void
copy_fields (SymbolTable ip_table, SymTabEntry ip_entry)
{
  Field f = NULL;
  SymTabEntry src_field_entry, ip_field_entry;
  Key ip_entry_key;
  int src_file_key = Plink_GetSymTabEntryFile (ip_entry);

  if (P_GetSymTabEntryType (ip_entry) & ET_STRUCT)
    f = P_GetStructDclFields (P_GetSymTabEntryStructDcl (ip_entry));
  else if (P_GetSymTabEntryType (ip_entry) & ET_UNION)
    f = P_GetUnionDclFields (P_GetSymTabEntryUnionDcl (ip_entry));
  else
    P_punt ("copy.c:copy_fields:%d ip_entry must be struct or union, not 0x%x",
	    __LINE__ - 1, P_GetSymTabEntryType (ip_entry));

  while (f)
    {
      src_field_entry = Plink_GetSymTabEntry (ip_table, src_file_key,
					      P_GetFieldKey (f));
      ip_entry_key = Plink_CopyEntry (ip_table, src_field_entry);
      ip_field_entry = PST_GetSymTabEntryFromMem (ip_table, ip_entry_key);
      P_SetSymTabEntryField (ip_field_entry, f);

      f = P_GetFieldNext (f);
    }

  return;
}

