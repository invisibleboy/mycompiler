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
 * \brief Functions to find symbols.
 *
 * \author Robert Kidd and Wen-mei Hwu
 *
 * This file contains definitions for function to find symbols in the table.
 * These functions are only useful when moving symbols between tables.
 * This is only done in Plink, so these are not included in the Pcode library.
 */

#include <config.h>
#include <string.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/query.h>
#include "find.h"
#include "data.h"

static Key find_type_dcl (SymbolTable ip_table, SymTabEntry source_entry);
static Key find_struct_dcl (SymbolTable ip_table, SymTabEntry source_entry);
static Key find_union_dcl (SymbolTable ip_table, SymTabEntry source_entry);
static SymTabEntry get_user (SymbolTable ip_table, SymTabEntry refd_entry,
			     bool map_to_ip);
static SymTabEntry get_user_next (SymbolTable ip_table, SymTabEntry refd_entry,
				  bool map_to_ip);

/*! \brief Searches the interprocedural symbol table for a previous
 *         definition of an entry.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the entry to search for.
 *
 * \return
 *  If a previous definition is found, returns its key.  Otherwise, returns
 *  Invalid_Key ({0, 0}).
 *
 * This function searches the global scope (a.out.stl, {1, 1}) and each
 * known source file's global scope for \a source_entry.  If a match is found,
 * the matching key is returned.  Otherwise, Invalid_Key is returned.
 */
Key
Plink_FindEntry (SymbolTable ip_table, SymTabEntry source_entry)
{
  _EntryType src_type = P_GetSymTabEntryType (source_entry);
  Key result = Invalid_Key;

  switch (src_type)
    {
    case ET_TYPE_GLOBAL:
      result = find_type_dcl (ip_table, source_entry);
      break;
    case ET_STRUCT:
      result = find_struct_dcl (ip_table, source_entry);
      break;
    case ET_UNION:
      result = find_union_dcl (ip_table, source_entry);
      break;
    default:
      break;
    }

  return (result);
}

/*! \brief Returns the first type that references a type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param refd_entry
 *  the SymTabEntry of the referenced TypeDcl.
 *
 * \return
 *  The SymTabEntry for the first TypeDcl that references \a refd_entry.
 *
 * The users are stored in each Source Table.  If refd_entry is in \a ip_table,
 * we find the corresponding Source Table entry, and transform the user's
 * key so the returned user is also in \a ip_table.
 *
 * \sa Plink_FindTypeReferencingNext()
 */
SymTabEntry
Plink_FindTypeReferencing (SymbolTable ip_table, SymTabEntry refd_entry)
{
  bool map_to_ip = FALSE;
  SymTabEntry result = NULL;

  /* If refd_entry is in \a ip_table, it will be linked to its entry in
   * a Source Table. */
  if (Plink_GetSymTabEntryFlags (refd_entry) & PLF_LT_SOURCE)
    {
      map_to_ip = TRUE;
      refd_entry = Plink_GetSymTabEntry (ip_table,
					 Plink_GetSymTabEntryFile (refd_entry),
					 Plink_GetSymTabEntryKey (refd_entry));
    }

  if (Plink_GetSymTabEntryUsers (refd_entry))
    result = get_user (ip_table, refd_entry, map_to_ip);

  return (result);
}

/*! \brief Returns the next type that references a type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param refd_entry
 *  the SymTabEntry of the referenced TypeDcl.
 * \param last
 *  the last SymTabEntry returned by this function.
 *
 * \return
 *  The SymTabEntry for the next TypeDcl that references \a refd_entry.
 *
 * The users are stored in each Source Table.  If refd_entry is in \a ip_table,
 * we find the corresponding Source Table entry, and transform the user's
 * key so the returned user is also in \a ip_table.
 *
 * \sa Plink_FindTypeReferencing()
 */
SymTabEntry
Plink_FindTypeReferencingNext (SymbolTable ip_table, SymTabEntry refd_entry,
			       SymTabEntry last)
{
  bool map_to_ip = FALSE;
  SymTabEntry result = NULL;

  /* If refd_entry is in \a ip_table, it will be linked to its entry in
   * a Source Table. */
  if (Plink_GetSymTabEntryFlags (refd_entry) & PLF_LT_SOURCE)
    {
      map_to_ip = TRUE;
      refd_entry = Plink_GetSymTabEntry (ip_table,
					 Plink_GetSymTabEntryFile (refd_entry),
					 Plink_GetSymTabEntryKey (refd_entry));
    }

  if (Plink_GetSymTabEntryUsers (refd_entry))
    {
      for (result = get_user (ip_table, refd_entry, map_to_ip);
	   result && (result != last);
	   result = get_user_next (ip_table, refd_entry, map_to_ip));

      result = get_user_next (ip_table, refd_entry, map_to_ip);
    }

  return (result);
}


/*! \brief Searches the interprocedural symbol table for a previous
 *         definition of a type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the TypeDcl to search for.
 *
 * \return
 *  If a previous definition is found, returns its key.  Otherwise, returns
 *  Invalid_Key ({0, 0}).
 *
 * This function searches the global scope (a.out.stl, {1, 1}) and each
 * known source file's global scope for \a source_entry.  If a match is found,
 * the other entry's key is returned.  Otherwise, Invalid_Key is returned.
 */
static Key
find_type_dcl (SymbolTable ip_table, SymTabEntry source_entry)
{
  TypeDcl src_type_dcl, ip_type_dcl;
  Key ip_entry_key = Invalid_Key;
  char *src_sig, *ip_sig;

  src_type_dcl = P_GetSymTabEntryTypeDcl (source_entry);
  src_sig = Plink_GetTypeDclSignature (src_type_dcl);

  for (ip_entry_key = PST_GetTableEntryByType (ip_table, ET_TYPE_GLOBAL);
       P_ValidKey (ip_entry_key);
       ip_entry_key = PST_GetTableEntryByTypeNext (ip_table, ip_entry_key,
						   ET_TYPE_GLOBAL))
    {
      ip_type_dcl = PST_GetTypeDclEntry (ip_table, ip_entry_key);
      ip_sig = Plink_GetTypeDclSignature (ip_type_dcl);
      
      if (strcmp (ip_sig, src_sig) == 0)
	break;
    }

  return (ip_entry_key);
}

/*! \brief Searches the interprocedural symbol table for a previous
 *         definition of a struct.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the struct to search for.
 *
 * \return
 *  If a previous definition is found, returns its key.  Otherwise, returns
 *  Invalid_Key ({0, 0}).
 *
 * This function searches the global scope (a.out.stl, {1, 1}) and each
 * known source file's global scope for \a source_entry.  If a match is found,
 * the other entry's key is returned.  Otherwise, Invalid_Key is returned.
 */
static Key
find_struct_dcl (SymbolTable ip_table, SymTabEntry source_entry)
{
  StructDcl src_struct_dcl, ip_struct_dcl;
  Key ip_entry_key, result = Invalid_Key;
  char *src_sig, *ip_sig, *src_name, *ip_name;

  src_struct_dcl = P_GetSymTabEntryStructDcl (source_entry);
  src_sig = Plink_GetStructDclSignature (src_struct_dcl);
  src_name = P_GetStructDclName (src_struct_dcl);

  for (ip_entry_key = PST_GetTableEntryByType (ip_table, ET_STRUCT);
       P_ValidKey (ip_entry_key);
       ip_entry_key = PST_GetTableEntryByTypeNext (ip_table, ip_entry_key,
						   ET_STRUCT))
    {
      ip_struct_dcl = PST_GetStructDclEntry (ip_table, ip_entry_key);
      ip_sig = Plink_GetStructDclSignature (ip_struct_dcl);
      ip_name = P_GetStructDclName (ip_struct_dcl);

      if (strcmp (ip_sig, src_sig) == 0)
	{
	  result = ip_entry_key;
	  break;
	}
    }

  return (result);
}

/*! \brief Searches the interprocedural symbol table for a previous
 *         definition of a union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the union to search for.
 *
 * \return
 *  If a previous definition is found, returns its key.  Otherwise, returns
 *  Invalid_Key ({0, 0}).
 *
 * This function searches the global scope (a.out.stl, {1, 1}) and each
 * known source file's global scope for \a source_entry.  If a match is found,
 * the other entry's key is returned.  Otherwise, Invalid_Key is returned.
 */
static Key
find_union_dcl (SymbolTable ip_table, SymTabEntry source_entry)
{
  UnionDcl src_union_dcl, ip_union_dcl;
  Key ip_entry_key, result = Invalid_Key;
  char *src_sig, *ip_sig, *src_name, *ip_name;

  src_union_dcl = P_GetSymTabEntryUnionDcl (source_entry);
  src_sig = Plink_GetUnionDclSignature (src_union_dcl);
  src_name = P_GetUnionDclName (src_union_dcl);

  for (ip_entry_key = PST_GetTableEntryByType (ip_table, ET_UNION);
       P_ValidKey (ip_entry_key);
       ip_entry_key = PST_GetTableEntryByTypeNext (ip_table, ip_entry_key,
						   ET_UNION))
    {
      ip_union_dcl = PST_GetUnionDclEntry (ip_table, ip_entry_key);
      ip_sig = Plink_GetUnionDclSignature (ip_union_dcl);
      ip_name = P_GetUnionDclName (ip_union_dcl);

      if (strcmp (ip_sig, src_sig) == 0)
	{
	  result = ip_entry_key;
	  break;
	}
    }

  return (result);
}

/*! \brief Returns the first user of a type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param refd_entry
 *  the SymTabEntry of the referenced type.
 * \param map_to_ip
 *  if TRUE, the user's key is mapped to \a ip_table so the returned
 *  user is in \a ip_table.
 *
 * \return
 *  The SymTabEntry for the first TypeDcl that references \a refd_entry,
 *  or NULL, if none exists.
 *
 * If \a map_to_ip is FALSE, we're processing users in a single source file.
 * In this case, we shouldn't return users outside the source file.
 */
static SymTabEntry
get_user (SymbolTable ip_table, SymTabEntry refd_entry, bool map_to_ip)
{
  SymTabEntry result = NULL;
  Plink_Key user;
  int file;
  Key key;

  List_start (Plink_GetSymTabEntryUsers (refd_entry));
  user = (Plink_Key)List_next (Plink_GetSymTabEntryUsers (refd_entry));

  if (user)
    {
      file = user->file;
      key = user->key;
	  
      if (map_to_ip == TRUE)
	{
	  file = Plink_GetLinkFile (ip_table, user->file, user->key);
	  key = Plink_GetLinkKey (ip_table, user->file, user->key);
	}
      else if (file != Plink_GetSymTabEntrySrcFile (refd_entry))
	{
	  /* Return NULL if we're processing in a source file and the user is
	   * outside. */
	  return (NULL);
	}

      result = Plink_GetSymTabEntry (ip_table, file, key);
    }

  return (result);
}

/*! \brief Returns the next user of a type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param refd_entry
 *  the SymTabEntry of the referenced type.
 * \param map_to_ip
 *  if TRUE, the user's key is mapped to \a ip_table so the returned
 *  user is in \a ip_table.
 *
 * \return
 *  The SymTabEntry for the next TypeDcl that references \a refd_entry,
 *  or NULL, if none exists.
 *
 * If \a map_to_ip is FALSE, we're processing users in a single source file.
 * In this case, we shouldn't return users outside the source file.
 */
static SymTabEntry
get_user_next (SymbolTable ip_table, SymTabEntry refd_entry, bool map_to_ip)
{
  SymTabEntry result = NULL;
  Plink_Key user;
  int file;
  Key key;

  user = (Plink_Key)List_next (Plink_GetSymTabEntryUsers (refd_entry));

  if (user)
    {
      file = user->file;
      key = user->key;
	  
      if (map_to_ip == TRUE)
	{
	  file = Plink_GetLinkFile (ip_table, user->file, user->key);
	  key = Plink_GetLinkKey (ip_table, user->file, user->key);
	}
      else if (file != Plink_GetSymTabEntrySrcFile (refd_entry))
	{
	  /* Return NULL if we're processing in a source file and the user is
	   * outside. */
	  return (NULL);
	}

      result = Plink_GetSymTabEntry (ip_table, file, key);
    }

  return (result);
}



