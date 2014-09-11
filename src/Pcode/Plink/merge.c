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
 * \brief Functions to merge symbols.
 *
 * \author Robert Kidd and Wen-mei Hwu
 *
 * This file contains definitions for functions to merge symbols.  These
 * functions are used when moving symbols between tables.
 */

#include <config.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <library/i_list.h>
#include <library/i_hashl.h>
#include <Pcode/pcode.h>
#include <Pcode/symtab.h>
#include <Pcode/struct.h>
#include <Pcode/query.h>
#include <Pcode/struct_symtab.h>
#include "main.h"
#include "merge.h"
#include "data.h"
#include "find.h"
#include "copy.h"
#include "update.h"
#include "signature.h"

static void merge (SymbolTable ip_table, SymTabEntry ip_entry,
		   SymTabEntry source_entry);
static void merge_fields (SymbolTable ip_table, SymTabEntry ip_entry,
			  SymTabEntry src_entry);
static void merge_pointers (SymbolTable ip_table, SymTabEntry ip_entry,
			    SymTabEntry src_entry);
static Key new_multi_union (SymbolTable ip_table, SymTabEntry ip_entry);
static Key multi_find_pointer_to_type (SymbolTable ip_table, Key multi_type,
				       Key refd_type);
static void add_type_to_multi (SymbolTable ip_table, Key multi_type,
			       Key ip_entry_key);

/*! \brief Attempts to merge a type into the interprocedural symbol table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_entry
 *  the SymTabEntry of a global type to resolve.
 *
 * \return
 *  If \a source_entry was merged, returns TRUE.  Otherwise, returns FALSE.
 *
 * This function searches the global scope (a.out.stl) and the global
 * file scope of all source files for a type that matches \a source_entry.
 * If a match is found, merge() is called to perform the merge.
 *
 * If \a source_entry is a Field, the field merged flag is checked instead
 * of calling Plink_FindEntry().
 */
bool
Plink_AttemptMerge (SymbolTable ip_table, SymTabEntry source_entry)
{
  Key ip_entry_key = Invalid_Key;
  bool result = FALSE;
  SymTabEntry ip_entry;

  /* Prevent looping when we make sure the fields of a struct have been
   * merged. */
  if (Plink_GetSymTabEntryFlags (source_entry) & \
      (PLF_MERGING | PLF_MERGED | PLF_COPIED))
    return (TRUE);

  Plink_SetSymTabEntryFlags (source_entry, PLF_MERGING);

  ip_entry_key = Plink_FindEntry (ip_table, source_entry);

  /* If a matching type already exists, set source_entry's new key to
   * ip_entry_key. */
  if (P_ValidKey (ip_entry_key))
    {
      ip_entry = PST_GetSymTabEntry (ip_table, ip_entry_key);
      merge (ip_table, ip_entry, source_entry);
      result = TRUE;
    }

  Plink_ClrSymTabEntryFlags (source_entry, PLF_MERGING);

  return (result);
}

/*! \brief Merges an entry in a Source Table with one in IP Table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param ip_entry
 *  the SymTabEntry to merge with in \a ip_table.
 * \param source_entry
 *  the SymTabEntry to merge into \a ip_table.
 *
 * Merges \a source_entry with \a ip_entry in \a ip_table.  If
 * \a ip_entry is in \a ip_table's global scope, we simply tag
 * \a source_entry with \a ip_entry's key.  If \a ip_entry is in
 * some source file in \a ip_table, we move \a ip_entry to
 * the global scope and tag \a source_entry with \a ip_entry's new key.
 */
static void
merge (SymbolTable ip_table, SymTabEntry ip_entry, SymTabEntry source_entry)
{
  Key ip_entry_key, source_entry_key;
  Key orig_source_entry_key;
  int orig_source_file_key, source_file_key;
  _EntryType ip_entry_type, src_entry_type;
  TypeDcl ip_td = NULL, src_td = NULL;
  _BasicType ip_bt = 0, src_bt = 0;

  /* The call to merge_struct() may have swapped source_entry
   * and ip_entry, so we need to get ip_entry_key and
   * source_entry_key after calling merge_struct(). */
  ip_entry_key = P_GetSymTabEntryKey (ip_entry);
  source_entry_key = P_GetSymTabEntryKey (source_entry);

  ip_entry_type = P_GetSymTabEntryType (ip_entry);
  src_entry_type = P_GetSymTabEntryType (source_entry);

  if (ip_entry_type & ET_TYPE)
    {
      ip_td = P_GetSymTabEntryTypeDcl (ip_entry);
      ip_bt = P_GetTypeDclBasicType (ip_td);
    }
  if (src_entry_type & ET_TYPE)
    {
      src_td = P_GetSymTabEntryTypeDcl (source_entry);
      src_bt = P_GetTypeDclBasicType (src_td);
    }

  if (ip_td && src_td && (ip_bt & src_bt & BT_POINTER))
    {
      merge_pointers (ip_table, ip_entry, source_entry);
    }
  else
    {
      orig_source_file_key = Plink_GetLinkFile (ip_table, 0, ip_entry_key);
      orig_source_entry_key = Plink_GetLinkKey (ip_table, 0, ip_entry_key);
      source_file_key = Plink_GetSymTabEntrySrcFile (source_entry);

      /* Typedefs in the same file will be merged here, but that isn't
       * reason to move the type to the global header. */
      if (ip_entry_key.file != 1 && orig_source_file_key != source_file_key)
	ip_entry_key = Plink_MoveToGlobalScope (ip_table, ip_entry_key);

      /* Link the new source_entry to ip_entry's source file entry. */
      Plink_SetLink (ip_table, source_file_key, source_entry_key,
		     orig_source_file_key, orig_source_entry_key, PLF_LT_LINK);
    }

  /* If we just merged a struct or union, merge the fields as well. */
  if (P_GetSymTabEntryType (ip_entry) & (ET_STRUCT | ET_UNION))
    merge_fields (ip_table, ip_entry, source_entry);

  Plink_SetSymTabEntryFlags (source_entry, PLF_MERGED);

  return;
}

/*! \brief Merge the fields of a struct or union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param ip_entry
 *  the SymTabEntry to merge with in \a ip_table.
 * \param src_entry
 *  the SymTabEntry to merge into \a ip_table.
 *
 * Merges the fields of a struct or union.
 */
static void
merge_fields (SymbolTable ip_table, SymTabEntry ip_entry,
	      SymTabEntry src_entry)
{
  SymTabEntry ip_field_entry, src_field_entry;
  Field ip_field = NULL, src_field = NULL;
  int src_file_key, ip_src_file_key;

  src_file_key = Plink_GetSymTabEntrySrcFile (src_entry);
  ip_src_file_key = Plink_GetLinkFile (ip_table, 0,
				       P_GetSymTabEntryKey (ip_entry));
  
  if (P_GetSymTabEntryType (ip_entry) & ET_STRUCT)
    {
      while ((ip_field_entry = Plink_GetStructDclField (ip_table, ip_entry,
							&ip_field)) && \
	     (src_field_entry = Plink_GetStructDclField (ip_table, src_entry,
							 &src_field)))
	{
	  Plink_SetLink (ip_table, src_file_key, P_GetFieldKey (src_field),
			 ip_src_file_key, P_GetFieldKey (ip_field),
			 PLF_LT_LINK);
	  Plink_SetSymTabEntryFlags (ip_field_entry, PLF_MERGED);
	  Plink_SetSymTabEntryFlags (src_field_entry, PLF_MERGED);
	}
    }
  else if (P_GetSymTabEntryType (ip_entry) & ET_UNION)
    {
      while ((ip_field_entry = Plink_GetUnionDclField (ip_table, ip_entry,
						       &ip_field)) && \
	     (src_field_entry = Plink_GetUnionDclField (ip_table, src_entry,
							&src_field)))
	{
	  Plink_SetLink (ip_table, src_file_key, P_GetFieldKey (src_field),
			 ip_src_file_key, P_GetFieldKey (ip_field),
			 PLF_LT_LINK);
	  Plink_SetSymTabEntryFlags (ip_field_entry, PLF_MERGED);
	  Plink_SetSymTabEntryFlags (src_field_entry, PLF_MERGED);
	}
    }

  return;
}

/*! \brief Merges two pointer types.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param ip_entry
 *  the SymTabEntry already in ip_entry.
 * \param src_entry
 *  the SymTabEntry (from a source file) being merged with \a ip_entry.
 *
 * If two pointer types are being merged here, they have the same
 * signature.  If they are pointing to structs or unions, having the
 * same signature means they are pointing to structs or unions with
 * the same name.  If they are pointing to different types in \a ip_table,
 * then we make a new union containing both pointer types.  We will
 * adjust references to variables using these types in the update
 * stage.
 */
static void
merge_pointers (SymbolTable ip_table, SymTabEntry ip_entry,
		SymTabEntry src_entry)
{
  int i_src_file = Plink_GetSymTabEntryFile (ip_entry);
  int s_src_file = Plink_GetSymTabEntrySrcFile (src_entry);
  TypeDcl ip_td = P_GetSymTabEntryTypeDcl (ip_entry);
  TypeDcl src_td = P_GetSymTabEntryTypeDcl (src_entry);
  Key i_refd_type = Plink_GetIPKey (ip_table, i_src_file,
				    P_GetTypeDclType (ip_td));
  Key s_refd_type = Plink_GetIPKey (ip_table, s_src_file,
				    P_GetTypeDclType (src_td));
  TypeDcl i_refd_td = PST_GetTypeDclEntry (ip_table, i_refd_type);
  TypeDcl s_refd_td = PST_GetTypeDclEntry (ip_table, s_refd_type);
  _BasicType i_refd_bt = P_GetTypeDclBasicType (i_refd_td);
  _BasicType s_refd_bt = P_GetTypeDclBasicType (s_refd_td);
  Key src_scope_key = Plink_GetIPKey (ip_table, s_src_file,
				      P_GetSymTabEntryScopeKey (src_entry));
  Key ip_src_entry_key;

  if (((i_refd_bt & s_refd_bt & BT_STRUCT) || \
       (i_refd_bt & s_refd_bt & BT_UNION)) && \
      !P_MatchKey (i_refd_type, s_refd_type))
    {
      Key multi_type = Plink_GetTypeDclMultiType (ip_td);
      Key known_pointer;

      /* If there's no known multi type, create a new union to hold
       * pointers to structs with the same names. */
      if (!P_ValidKey (multi_type))
	multi_type = new_multi_union (ip_table, ip_entry);

      known_pointer = multi_find_pointer_to_type (ip_table, multi_type,
						  s_refd_type);

      if (P_ValidKey (known_pointer))
	{
	  SymTabEntry known_entry = PST_GetSymTabEntry (ip_table,
							known_pointer);
	  int k_src_file;
	  Key k_src_key;

	  if (!P_MatchKey (P_GetSymTabEntryScopeKey (known_entry),
			   src_scope_key))
	    {
	      TypeDcl multi_td = PST_GetTypeDclEntry (ip_table, multi_type);
	      UnionDcl multi_union = \
		PST_GetUnionDclEntry (ip_table, P_GetTypeDclType (multi_td));

	      known_pointer = Plink_MoveToGlobalScope (ip_table,
						       known_pointer);
	      known_entry = PST_GetSymTabEntry (ip_table, known_pointer);

	      /* Note that known_pointer is valid in scope src_scope_key. */
	      HashLTable_insert (Plink_GetUnionDclMultiHash (multi_union),
				 P_Key2Long (src_scope_key),
				 P_NewKeyPWithKey (known_pointer));
	    }

	  k_src_file = Plink_GetSymTabEntryFile (known_entry);
	  k_src_key = Plink_GetSymTabEntryKey (known_entry);

	  Plink_SetLink (ip_table, s_src_file, P_GetSymTabEntryKey (src_entry),
			 k_src_file, k_src_key, PLF_LT_LINK);
	}
      else
	{
	  ip_src_entry_key = Plink_CopyEntry (ip_table, src_entry);
	  add_type_to_multi (ip_table, multi_type, ip_src_entry_key);
	}
    }
  else
    {
      /* Link src_entry to ip_entry. */
      Plink_SetLink (ip_table, s_src_file, P_GetSymTabEntryKey (src_entry),
		     i_src_file, Plink_GetSymTabEntryKey (ip_entry),
		     PLF_LT_LINK);
    }
  
  return;
}

/*! \brief Updates the key of a type known to a multi type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the key of the multi type union.
 * \param orig_ip_key
 *  the old key of the entry to update.
 * \param new_ip_key
 *  the new key of the entry to update.
 *
 * Finds the field in the multi union that references \a orig_ip_key and
 * updates it to \a new_ip_key.
 */
void
Plink_UpdateMultiType (SymbolTable ip_table, Key multi_type, Key orig_ip_key,
		       Key new_ip_key)
{
  TypeDcl multi_td = PST_GetTypeDclEntry (ip_table, multi_type);
  UnionDcl multi_union = PST_GetUnionDclEntry (ip_table,
					       P_GetTypeDclType (multi_td));
  Key new_scope_key;
  Field f;
  HashLTable h;
  Key *cur_key_p;

  for (f = P_GetUnionDclFields (multi_union); f; f = P_GetFieldNext (f))
    {
      if (P_MatchKey (P_GetFieldType (f), orig_ip_key))
	{
	  P_SetFieldType (f, new_ip_key);
	  break;
	}
    }

  h = Plink_GetUnionDclMultiHash (multi_union);

  /* Change orig_ip_key to new_ip_key in the hash. */
  HashLTable_start (h);
  while ((cur_key_p = (Key *)HashLTable_next (h)))
    {
      if (P_MatchKey (*cur_key_p, orig_ip_key))
	*cur_key_p = new_ip_key;
    }

  /* If new_ip_key is the first type valid in the global scope, add it
   * as the union's type valid in the global scope. */
  new_scope_key = PST_GetScopeFromEntryKey (ip_table, new_ip_key);
  if (P_MatchKey (new_scope_key, global_scope_key) && \
      (HashLTable_find_or_null (h, P_Key2Long (new_scope_key)) == NULL))
    HashLTable_insert (h, P_Key2Long (new_scope_key),
		       P_NewKeyPWithKey (new_ip_key));

  return;
}

/*! \brief Returns the key of a new BT_UNION type containing a pointer type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param ip_entry
 *  the SymTabEntry in \a ip_table of a pointer type to add to the new union.
 *
 * \return
 *  The key of a new BT_UNION type containing a single field with the
 *  type represented by \a ip_entry.
 *
 * \a ip_entry is assumed to contain a TypeDcl which is a pointer to a
 * struct or union.
 */
static Key
new_multi_union (SymbolTable ip_table, SymTabEntry ip_entry)
{
  SymTabEntry refd_entry, struct_entry, union_entry, union_td_entry;
  UnionDcl u;
  Field f = NULL;
  TypeDcl union_td, ptr_td, refd_td;
  Key u_key, f_key, union_td_key;
  char *name, *refd_name, *sig;
  int name_len;
  int sig_len = 22; /* 2 x 10 digit ints + _ + \0 */
  bool struct_def_is_complete = TRUE;

  /* Determine the name of the thing ip_entry points to. */
  ptr_td = P_GetSymTabEntryTypeDcl (ip_entry);
  refd_entry = Plink_GetTypeDclType (ip_table, ip_entry);
  refd_td = P_GetSymTabEntryTypeDcl (refd_entry);

  if (!((P_GetSymTabEntryType (refd_entry) & ET_TYPE) && \
	(P_GetTypeDclBasicType (refd_td) & (BT_STRUCT | BT_UNION))))
    P_punt ("merge.c:new_multi_union:%d ip_entry must be pointer to struct "
	    "or union", __LINE__ - 1);

  struct_entry = Plink_GetTypeDclType (ip_table, refd_entry);
  switch (P_GetSymTabEntryType (struct_entry))
    {
    case ET_STRUCT:
      if (P_TstStructDclQualifier (P_GetSymTabEntryStructDcl (struct_entry),
				   SQ_INCOMPLETE))
	struct_def_is_complete = FALSE;
      break;
    case ET_UNION:
      if (P_TstUnionDclQualifier (P_GetSymTabEntryUnionDcl (struct_entry),
				  SQ_INCOMPLETE))
	struct_def_is_complete = FALSE;
      break;
    default:
      P_punt ("merge.c:new_multi_union:%d %s must reference %s", __LINE__,
	      P_BasicTypeToString (P_GetTypeDclBasicType (refd_td)),
	      P_EntryTypeToString (P_GetSymTabEntryType (struct_entry)));
    }
  
  refd_name = P_GetTypeDclName (refd_td);
  name_len = strlen (refd_name) + 7; /* _multi + \0 */
  name = malloc (name_len);
  snprintf (name, name_len, "%s_multi", refd_name);

  sig = malloc (sig_len);

  u = P_NewUnionDcl ();
  u_key.file = global_scope_key.file;
  u_key.sym = 0;
  P_SetUnionDclKey (u, u_key);
  P_SetUnionDclName (u, name);

  P_SetUnionDclQualifier (u, SQ_LINKMULTI);

  Plink_SetUnionDclMultiHash (u, HashLTable_create (PLINK_HASH_SIZE));

  u_key = PST_AddUnionDclEntry (ip_table, u);
  PST_AddEntryToScope (ip_table, global_scope_key, u_key);

  union_entry = PST_GetSymTabEntry (ip_table, u_key);

  /* Set the Union's signature to something unique so it doesn't get merged. */
  snprintf (sig, sig_len, "%d_%d", u_key.file, u_key.sym);
  Plink_SetUnionDclSignature (u, strdup (sig));

  if (struct_def_is_complete)
    {
      HashLTable_insert (Plink_GetUnionDclMultiHash (u),
			 P_Key2Long (P_GetSymTabEntryScopeKey (ip_entry)),
			 P_NewKeyPWithKey (P_GetSymTabEntryKey (ip_entry)));
      
      f = P_NewField ();
      f_key.file = global_scope_key.file;
      f_key.sym = 0;
      P_SetFieldKey (f, f_key);
      P_SetFieldName (f, strdup ("t_0"));
      P_SetFieldParentKey (f, u_key);
      P_SetFieldType (f, P_GetSymTabEntryKey (ip_entry));
      f_key = PST_AddFieldEntry (ip_table, f);
      PST_AddEntryToScope (ip_table, global_scope_key, f_key);
      P_AppendUnionDclFields (u, f);
    }

  union_td = P_NewTypeDcl ();
  union_td_key.file = global_scope_key.file;
  union_td_key.sym = 0;
  P_SetTypeDclKey (union_td, union_td_key);
  P_SetTypeDclBasicType (union_td, BT_UNION);
  P_SetTypeDclName (union_td, strdup (name));
  P_SetTypeDclType (union_td, u_key);
  /* The size and alignment of the union is the same as for the
   * struct pointer. */
  P_SetTypeDclAlignment (union_td, P_GetTypeDclAlignment (ptr_td));
  P_SetTypeDclSize (union_td, P_GetTypeDclSize (ptr_td));
  union_td_key = PST_ScopeFindTypeDcl (ip_table, global_scope_key, union_td);
  union_td = P_RemoveTypeDcl (union_td);

  union_td_entry = PST_GetSymTabEntry (ip_table, union_td_key);

  /* Set the BT_UNION TypeDcl as a user of the UnionDcl. */
  Plink_AppendSymTabEntryUsers (union_entry, 0, union_td_key);

  /* Set the TypeDcl's signature to something unique so it doesn't get
   * merged. */
  snprintf (sig, sig_len, "%d_%d", union_td_key.file, union_td_key.sym);
  Plink_SetTypeDclSignature (PST_GetTypeDclEntry (ip_table, union_td_key),
			     strdup (sig));

  /* Attach the multi type union's key to the TypeDcl in IP Table and
   * attach the TypeDcl's key to the multi type union. */
  Plink_SetTypeDclMultiType (P_GetSymTabEntryTypeDcl (ip_entry), union_td_key);
  Plink_AppendSymTabEntryUsers (union_td_entry,
				Plink_GetSymTabEntryFile (ip_entry),
				Plink_GetSymTabEntryKey (ip_entry));

  if (P_GetTypeDclBasicType (refd_td) == BT_STRUCT)
    {
      SymTabEntry refd_st = Plink_GetTypeDclType (ip_table, refd_entry);
      Plink_SetStructDclMultiType (P_GetSymTabEntryStructDcl (refd_st),
				   union_td_key);
    }
  else
    {
      SymTabEntry refd_un = Plink_GetTypeDclType (ip_table, refd_entry);
      Plink_SetUnionDclMultiType (P_GetSymTabEntryUnionDcl (refd_un),
				  union_td_key);
    }
#if 0
  Plink_AppendSymTabEntryUsers (union_td_entry,
				Plink_GetSymTabEntryFile (refd_entry),
				Plink_GetSymTabEntryKey (refd_entry));
#endif

  free (sig);

  return (union_td_key);
}

/*! \brief Determines if the multi type already contains a pointer to a type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the key of the multi type union.
 * \param refd_type
 *  the key (in \a ip_table) of a struct or union that may or may not
 *  be pointed to by a type in \a multi_type.
 *
 * \return
 *  If \a refd_type is pointed to by some type in \a multi_type, returns
 *  the key of the pointer.  Otherwise, returns Invalid_Key.
 *
 * \a multi_type is a BT_UNION referring to a UnionDcl where each field
 * is a pointer to a struct or union with the same name.  This function
 * inspects the struct or union pointed to by each field to determine if
 * \a refd_type is already in \a multi_type.
 */
static Key
multi_find_pointer_to_type (SymbolTable ip_table, Key multi_type,
			    Key refd_type)
{
  TypeDcl multi_td = PST_GetTypeDclEntry (ip_table, multi_type);
  UnionDcl multi_union = PST_GetUnionDclEntry (ip_table,
					       P_GetTypeDclType (multi_td));
  Field f;
  Key result = Invalid_Key;

  for (f = P_GetUnionDclFields (multi_union); f; f = P_GetFieldNext (f))
    {
      SymTabEntry field_type_entry = PST_GetSymTabEntry (ip_table,
							 P_GetFieldType (f));
      TypeDcl field_td = P_GetSymTabEntryTypeDcl (field_type_entry);
      int field_type_src_file = Plink_GetSymTabEntryFile (field_type_entry);
      Key field_type_key = Plink_GetIPKey (ip_table, field_type_src_file,
					   P_GetTypeDclType (field_td));

      if (P_MatchKey (field_type_key, refd_type))
	{
	  result = P_GetFieldType (f);
	  break;
	}
    }
  
  return (result);
}

/*! \brief Adds a type to a multi type union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param multi_type
 *  the key of the multi type union.
 * \param ip_entry_key
 *  the key in \a ip_table of a pointer type to add to the union.
 *
 * Adds a field to the union \a multi_type with the type in \a ip_entry.
 */
static void
add_type_to_multi (SymbolTable ip_table, Key multi_type, Key ip_entry_key)
{
  TypeDcl multi_td = PST_GetTypeDclEntry (ip_table, multi_type);
  UnionDcl multi_union = PST_GetUnionDclEntry (ip_table,
					       P_GetTypeDclType (multi_td));
  SymTabEntry ip_entry = PST_GetSymTabEntry (ip_table, ip_entry_key);
  SymTabEntry refd_entry = Plink_GetTypeDclType (ip_table, ip_entry);
  SymTabEntry struct_entry, multi_td_entry;
  TypeDcl refd_td = P_GetSymTabEntryTypeDcl (refd_entry);
  Field f;
  Key f_key;
  int num_fields = 0;
  char *name;
  int name_len = 13; /* t_ + 10 digit int + \0 */
  bool struct_def_is_complete = TRUE;

  /* Determine if this struct type is incomplete. */
  struct_entry = Plink_GetTypeDclType (ip_table, refd_entry);
  switch (P_GetSymTabEntryType (struct_entry))
    {
    case ET_STRUCT:
      if (P_TstStructDclQualifier (P_GetSymTabEntryStructDcl (struct_entry),
				   SQ_INCOMPLETE))
	struct_def_is_complete = FALSE;
      break;
    case ET_UNION:
      if (P_TstUnionDclQualifier (P_GetSymTabEntryUnionDcl (struct_entry),
				  SQ_INCOMPLETE))
	struct_def_is_complete = FALSE;
      break;
    default:
      P_punt ("merge.c:add_type_to_multi:%d %s must reference %s", __LINE__,
	      P_BasicTypeToString (P_GetTypeDclBasicType (refd_td)),
	      P_EntryTypeToString (P_GetSymTabEntryType (struct_entry)));
    }

  if (struct_def_is_complete)
    {
      /* Count the fields already in the union so we can make a unique name for
       * the new field. */
      for (f = P_GetUnionDclFields (multi_union); f; f = P_GetFieldNext (f))
	num_fields++;

      /* Fields are numbered from 0, so num_fields is the next field number
       * at this point. */
      name = malloc (name_len);
      snprintf (name, name_len, "t_%d", num_fields);

      f = P_NewField ();
      f_key.file = global_scope_key.file;
      f_key.sym = 0;
      P_SetFieldKey (f, f_key);
      P_SetFieldName (f, name);
      P_SetFieldParentKey (f, P_GetTypeDclType (multi_td));
      P_SetFieldType (f, ip_entry_key);
      f_key = PST_AddFieldEntry (ip_table, f);
      PST_AddEntryToScope (ip_table, global_scope_key, f_key);
      P_AppendUnionDclFields (multi_union, f);

      HashLTable_insert (Plink_GetUnionDclMultiHash (multi_union),
			 P_Key2Long (P_GetSymTabEntryScopeKey (ip_entry)),
			 P_NewKeyPWithKey (ip_entry_key));
    }

  multi_td_entry = PST_GetSymTabEntry (ip_table, multi_type);

  /* Attach the multi type union's key to the TypeDcl in IP Table and
   * attach the TypeDcl's key to the multi type union. */
  Plink_SetTypeDclMultiType (P_GetSymTabEntryTypeDcl (ip_entry), multi_type);
  Plink_AppendSymTabEntryUsers (multi_td_entry,
				Plink_GetSymTabEntryFile (ip_entry),
				Plink_GetSymTabEntryKey (ip_entry));

  /* refd_entry is the type pointed to by ip_entry (a BT_STRUCT or BT_UNION) */
  if (P_GetTypeDclBasicType (refd_td) == BT_STRUCT)
    {
      SymTabEntry refd_st = Plink_GetTypeDclType (ip_table, refd_entry);
      Plink_SetStructDclMultiType (P_GetSymTabEntryStructDcl (refd_st),
				   multi_type);
    }
  else
    {
      SymTabEntry refd_un = Plink_GetTypeDclType (ip_table, refd_entry);
      Plink_SetUnionDclMultiType (P_GetSymTabEntryUnionDcl (refd_un),
				  multi_type);
    }
#if 0
  Plink_AppendSymTabEntryUsers (multi_td_entry,
				Plink_GetSymTabEntryFile (refd_entry),
				Plink_GetSymTabEntryKey (refd_entry));
#endif
  
  return;
}

/*! \brief Compress unneeded multi type unions to a single struct type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * Any structures which have an incomplete definition and a single
 * complete definition may be merged in to a multi type union with a
 * single struct type.  These unions are not needed, so we can
 * compress them down to the single complete struct type.
 */
void
Plink_CompressMultiTypes (SymbolTable ip_table)
{
  Key k;
  SymTabEntry union_entry, union_td_entry, user_entry, complete_type_entry;
  UnionDcl union_dcl;
  Field field;
  Type complete_type;
  List users;
  Plink_Key cur_user;

  for (k = PST_GetTableEntryByType (ip_table, ET_UNION); P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k, ET_UNION))
    {
      union_entry = PST_GetSymTabEntry (ip_table, k);
      union_dcl = P_GetSymTabEntryUnionDcl (union_entry);

      /* If this is a multi type union with only one field, remove it
       * and point all users to the field's type. */
      if (Plink_GetUnionDclMultiHash (union_dcl) != NULL && \
	  (field = P_GetUnionDclFields (union_dcl)) && \
	  (merge_structs == FALSE || P_GetFieldNext (field) == NULL))
	{
	  complete_type = P_GetFieldType (field);
	  complete_type_entry = PST_GetSymTabEntry (ip_table, complete_type);
	  users = Plink_GetSymTabEntryUsers (union_entry);

	  /* Get the BT_UNION TypeDcl for the multi type union. */
	  List_start (users);
	  cur_user = (Plink_Key)List_next (users);
	  union_td_entry = Plink_GetSymTabEntry (ip_table, cur_user->file,
						 cur_user->key);
	  users = Plink_GetSymTabEntryUsers (union_td_entry);

	  /* Point all users of the BT_UNION to the field's type */
	  List_start (users);
	  while ((cur_user = (Plink_Key)List_next (users)))
	    {
	      user_entry = PST_GetSymTabEntry (ip_table,
					       Plink_GetIPKey (ip_table,
							       cur_user->file,
							       cur_user->key));

	      /* Clear the link to this multi type. */
	      switch (P_GetSymTabEntryType (user_entry))
		{
		case ET_TYPE_LOCAL:
		case ET_TYPE_GLOBAL:
		  {
		    TypeDcl td = P_GetSymTabEntryTypeDcl (user_entry);
		    Plink_SetTypeDclMultiType (td, Invalid_Key);

		    if (user_entry != complete_type_entry)
		      {
			Plink_RemoveEntry (ip_table, cur_user->file,
					   cur_user->key);
			Plink_SetLink \
			  (ip_table, cur_user->file, cur_user->key, 
			   Plink_GetSymTabEntryFile (complete_type_entry),
			   Plink_GetSymTabEntryKey (complete_type_entry),
			   PLF_LT_LINK);
		      }
		  }
		  break;
		case ET_STRUCT:
		  {
		    StructDcl st = P_GetSymTabEntryStructDcl (user_entry);
		    Plink_SetStructDclMultiType (st, Invalid_Key);
		  }
		  break;
		case ET_UNION:
		  {
		    UnionDcl un = P_GetSymTabEntryUnionDcl (user_entry);
		    Plink_SetUnionDclMultiType (un, Invalid_Key);
		  }
		  break;
		default:
		  break;
		}
	    }

	  if (complete_type.file != global_header_key)
	    Plink_MoveToGlobalScope (ip_table, complete_type);

	  /* The multi type union exists only in the IP Table, and has
	   * keys valid in IP Table, , so we must use
	   * PST_RemoveSymTabEntry(). */
	  PST_RemoveSymTabEntry (ip_table, union_entry);
	  PST_RemoveSymTabEntry (ip_table, union_td_entry);
	}
    }

  return;
}
