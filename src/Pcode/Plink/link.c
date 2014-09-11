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
 * \brief Functions to resolve links between source files.
 *
 * \author Robert Kidd and Wen-mei Hwu.
 *
 * This file contains function definitions to resolve symbols between source
 * files.
 */

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/query.h>
#include "link.h"
#include "data.h"
#include "copy.h"

static void link_entry (SymbolTable ip_table, SymTabEntry source_entry);
static void link_users (SymbolTable ip_table, Plink_EIData ex);
static void tag_scope (SymbolTable ip_Table, SymTabEntry source_entry);

/*! \brief Attempts to resolve references to a function or global variable.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the SymTabEntry of a function or global variable to resolve.
 *
 * \return
 *  If \a source_entry was linked, returns TRUE.  Otherwise, returns FALSE.
 *
 * This function determines if a global variable or function can be linked,
 * and if so, calls link() to perform the link.
 */
bool
Plink_AttemptLink (SymbolTable ip_table, SymTabEntry source_entry)
{
  bool result = FALSE;

  if (P_GetSymTabEntryType (source_entry) == ET_FUNC)
    {
      FuncDcl f = P_GetSymTabEntryFuncDcl (source_entry);

      if (!(P_GetFuncDclQualifier (f) & VQ_STATIC))
	result = TRUE;
    }
  else if (P_GetSymTabEntryType (source_entry) == ET_VAR_GLOBAL)
    {
      VarDcl v = P_GetSymTabEntryVarDcl (source_entry);

      if (!(P_GetVarDclQualifier (v) & VQ_STATIC))
	result = TRUE;
    }

  if (result == TRUE)
    link_entry (ip_table, source_entry);
  
  return (result);
}

/*! \brief Links a reference to a function or global variable.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the entry to link.
 *
 * This function is called from Plink_attempt_link() after it has been
 * determined that \a source_entry can be linked.
 *
 * First, the \a source_entry is added as a user of the extern.  We then
 * proceed based on how \a source_entry and the known extern are defined.
 *
 * There are seven outcomes based on how \a source_entry and the known extern
 * are defined.
 * 1. \a source_entry is defined, known extern is defined, neither is
 *    weak.  This is an error (multiply defined symbol)
 * 2. \a source_entry is defined, known extern is defined
 *    If exactly one is weak, the weak one is added as a user and linked
 *    to the non-weak one.  If both are weak, the known extern is added
 *    as a user and linked to source_entry.
 * 3. \a source_entry is defined, known extern is defined, both are comdat.
 *    source_entry will be linked to known extern.
 * 4. \a source_entry is defined, known extern is common or undefined
 *    Remove known extern's copy from \a ip_table.  Copy \a source_entry to
 *    \a ip_table. Set the known extern's definition to be \a source_entry.
 *    Link all users to \a source_entry.  
 * 5. \a source_entry is common or undefined, known extern is defined or
 *    common.  Link \a source_entry to the known extern.
 * 6. \a source_entry is common, known extern is undefined.
 *    Remove known extern's copy from \a ip_table.  Copy \a source_entry to
 *    \a ip_table.  Set the known extern's definition to be \a source_entry.
 *    Link all users to \a source entry.
 * 7. \a source_entry is undefined, known extern is undefined
 *    If known extern doesn't exist, copy \a source_entry to \a ip_table.
 *    Link \a source_entry to the known extern.
 *
 * We can determine how \a source_entry is defined by looking at its
 * qualifier.
 *
 * When one symbol is linked to another, the Plink_key structure attached
 * to that symbol's SymTabEntry is filled out.  Plink_key.file is the
 * target's file's key in \a ip_table.  Plink_key.key is the target's
 * key in the target's Source Table.
 */
static void
link_entry (SymbolTable ip_table, SymTabEntry source_entry)
{
  Plink_EIData ex;
  char *name;
  VarDcl ex_var_dcl = NULL;
  FuncDcl ex_func_dcl = NULL;
  _EntryType func_or_var;
  _VarQual qualifier, ex_qualifier = 0;
  int source_file_key = Plink_GetSymTabEntrySrcFile (source_entry);

  func_or_var = P_GetSymTabEntryType (source_entry);

  /* Get the name and qualifier for \a source_entry. */
  if (func_or_var == ET_FUNC)
    {
      FuncDcl f = P_GetSymTabEntryFuncDcl (source_entry);
      name = P_GetFuncDclName (f);
      qualifier = P_GetFuncDclQualifier (f);
    }
  else
    {
      VarDcl v = P_GetSymTabEntryVarDcl (source_entry);
      name = P_GetVarDclName (v);
      qualifier = P_GetVarDclQualifier (v);
    }

  /* Get the ExternInfo entry for source_entry and add source_entry as a
   * user. */
  ex = Plink_FindExtern (name);
  Plink_AppendEIDataUsers (ex, source_file_key,
			   P_GetSymTabEntryKey (source_entry));

  if (P_ValidKey (ex->def.key))
    {
      if (func_or_var == ET_FUNC)
	{
	  ex_func_dcl = \
	    P_GetSymTabEntryFuncDcl (Plink_GetSymTabEntry (ip_table,
							   ex->def.file,
							   ex->def.key));
	  ex_qualifier = P_GetFuncDclQualifier (ex_func_dcl);
	}
      else
	{
	  ex_var_dcl = \
	    P_GetSymTabEntryVarDcl (Plink_GetSymTabEntry (ip_table,
							  ex->def.file,
							  ex->def.key));
	  ex_qualifier = P_GetVarDclQualifier (ex_var_dcl);
	}
    }

  /* Check for multiply defined symbol. */
  if ((qualifier & VQ_DEFINED) && (ex_qualifier & VQ_DEFINED))
    {
      if (!(qualifier & (VQ_WEAK | VQ_COMDAT)) && \
	  !(ex_qualifier & (VQ_WEAK | VQ_COMDAT)))
	{
	  P_punt ("link.c:link:%d multiple definitions for %s", __LINE__,
		  name);
	}
      else
	{
	  /* If known extern is weak and source_entry is not, link
	   * known extern to source_entry. */
	  if ((ex_qualifier & VQ_WEAK) && !(qualifier & VQ_WEAK))
	    {
	      Plink_RemoveEntry (ip_table, ex->def.file, ex->def.key);

	      /* Copy source_entry to ip_table. */
	      Plink_CopyEntry (ip_table, source_entry);

	      /* Set source_entry as the definition and link all users to
	       * source_entry. */
	      ex->def.file = source_file_key;
	      ex->def.key = P_GetSymTabEntryKey (source_entry);

	      link_users (ip_table, ex);
	    }
	  else
	    {
	      /* extern is defined or common.  Link source_entry to extern. */
	      Plink_SetLink (ip_table, source_file_key,
			     P_GetSymTabEntryKey (source_entry), ex->def.file,
			     ex->def.key, PLF_LT_LINK);

	      /* If we just linked a function, tag all SymTabEntries in its
	       * scope as linked. */
	      if (func_or_var == ET_FUNC)
		tag_scope (ip_table, source_entry);
	    }
	}
    }
  else if ((qualifier & VQ_DEFINED) || \
      ((qualifier & VQ_COMMON) && !(ex_qualifier & (VQ_DEFINED | VQ_COMMON))))
    {
      Plink_RemoveEntry (ip_table, ex->def.file, ex->def.key);

      /* Copy source_entry to ip_table. */
      Plink_CopyEntry (ip_table, source_entry);

      /* Set source_entry as the definition and link all users to
       * source_entry. */
      ex->def.file = source_file_key;
      ex->def.key = P_GetSymTabEntryKey (source_entry);

      link_users (ip_table, ex);
    }
  else if ((ex_qualifier & (VQ_DEFINED | VQ_COMMON)) || \
	   (!(ex_qualifier & (VQ_DEFINED | VQ_COMMON)) && \
	    !(qualifier & (VQ_DEFINED | VQ_COMMON)) && \
	    (ex_var_dcl != NULL || ex_func_dcl != NULL)))
    {
      /* extern is defined or common.  Link source_entry to extern. */
      Plink_SetLink (ip_table, source_file_key,
		     P_GetSymTabEntryKey (source_entry), ex->def.file,
		     ex->def.key, PLF_LT_LINK);
    }
  else
    {
      /* extern doesn't exist.  Copy source_entry to ip_table. */
      Plink_CopyEntry (ip_table, source_entry);

      /* Set source_entry as the definition and link all users to
       * source_entry. */
      ex->def.file = source_file_key;
      ex->def.key = P_GetSymTabEntryKey (source_entry);
    }

  /* Tag the source entry as linked. */
  Plink_SetSymTabEntryFlags (source_entry, PLF_LINKED);

  return;
}

/*! \brief Links a symbol's users to the symbol.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param ex
 *  the Plink_EIData struct for the symbol.
 *
 * Links a symbol's users to the symbol.
 *
 * When one symbol is linked to another, the Plink_key structure attached
 * to that symbol's SymTabEntry is filled out.  Plink_key.file is the
 * target's file's key in \a ip_table.  Plink_key.key is the target's
 * key in the target's Source Table.
 */
static void
link_users (SymbolTable ip_table, Plink_EIData ex)
{
  Plink_Key user;

  List_start (ex->users);

  while ((user = (Plink_Key)List_next (ex->users)))
    {
      /* An extern is listed as a user of itself, so skip it. */
      if (user->file == ex->def.file && P_MatchKey (user->key, ex->def.key))
	continue;

      Plink_SetLink (ip_table, user->file, user->key, ex->def.file,
		     ex->def.key, PLF_LT_LINK);
    }

  return;
}

/*! \brief Sets the PLF_LINKED flag on all entries in a scope.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the SymTabEntry process.
 *
 * If \a source_entry contains a scope, sets the PLF_LINKED flag on all
 * entries in that scope.
 */
static void
tag_scope (SymbolTable ip_table, SymTabEntry source_entry)
{
  SymTabEntry local_entry;
  Scope s;
  ScopeEntry se;
  int source_file_key = Plink_GetSymTabEntrySrcFile (source_entry);

  if ((s = P_GetSymTabEntryScope (source_entry)))
    {
      for (se = P_GetScopeScopeEntry (s); se; se = P_GetScopeEntryNext (se))
	{
	  local_entry = \
	    Plink_GetSymTabEntry (ip_table,
				  source_file_key,
				  P_GetScopeEntryKey (se));

	  Plink_SetSymTabEntryFlags (local_entry, PLF_LINKED);

	  if (P_GetSymTabEntryScope (local_entry))
	    tag_scope (ip_table, local_entry);
	}
    }

  return;
}

