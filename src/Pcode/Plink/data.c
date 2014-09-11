/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:             
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 * \brief Functions to manage Plink's structures.
 *
 * \author Robert Kidd and Wen-mei Hwu.
 *
 * This file contains file definitions to manage Plink's data structures.
 */

#include <config.h>
#include <string.h>
#include <library/i_list.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/query.h>
#include <Pcode/symtab.h>
#include "data.h"

static ExternInfo externs = NULL;

/*! \brief Allocates a Plink_Key struct.
 *
 * \return
 *  A pointer to the new Plink_Key struct.
 */
Plink_Key
Plink_AllocKey (void)
{
  Plink_Key new;

  new = ALLOCATE (_Plink_Key);

  return (new);
}

/*! \brief Allocates a Plink_Key struct and sets the file and key fields.
 *
 * \param f
 *  the value of the file field.
 * \param k
 *  the value of the key field.
 *
 * \return
 *  A pointer to the new Plink_Key struct.
 */
Plink_Key
Plink_AllocKeyWithFileKey (int f, Key k)
{
  Plink_Key new = Plink_AllocKey ();

  new->file = f;
  new->key = k;

  return (new);
}

/*! \brief Frees a Plink_Key struct.
 *
 * \param p
 *  the Plink_Key struct to free.
 *
 * \return
 *  A null Plink_Key pointer.
 */
Plink_Key
Plink_FreeKey (Plink_Key p)
{
  if (p)
    DISPOSE (p);

  return (NULL);
}

/*! \brief Allocates a Plink_InputFile struct.
 *
 * \return
 *  A pointer to the new Plink_InputFile struct.
 */
Plink_InputFile
Plink_AllocInputFile (void)
{
  Plink_InputFile new;

  new = ALLOCATE (_Plink_InputFile);

  return (new);
}

/*! \brief Allocates a Plink_InputFile struct and sets the type and name
 *         fields.
 *
 * \param type
 *  The input file's type.
 * \param name
 *  The input file's name.
 *
 * \return
 *  A pointer to the new PlinK_InputFile struct.
 *
 * \note The string passed as \a name will be copied, so the caller must
 *       free its copy.
 */
Plink_InputFile
Plink_AllocInputFileWithTypeName (_Plink_InputType type, char *name)
{
  Plink_InputFile new = Plink_AllocInputFile ();

  new->type = type;
  new->name = strdup (name);

  return (new);
}

/*! \brief Frees a Plink_InputFile struct.
 *
 * \param p
 *  the Plink_InputFile struct to free.
 *
 * \return
 *  A null Plink_InputFile pointer.
 */
Plink_InputFile
Plink_FreeInputFile (Plink_InputFile p)
{
  if (p)
    {
      if (p->name)
	free (p->name);

      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Allocates a Plink_SymTabEntryExt struct.
 *
 * \return
 *  A pointer to the new Plink_SymTabEntryExt struct.
 */
Plink_SymTabEntryExt
Plink_AllocSymTabEntryExt (void)
{
  Plink_SymTabEntryExt new;

  new = ALLOCATE (_Plink_SymTabEntryExt);

  return (new);
}

/*! \brief Frees a Plink_SymTabEntryExt struct.
 *
 * \param p
 *  the Plink_SymTabEntryExt struct to free.
 *
 * \return
 *  A null Plink_SymTabEntryExt pointer.
 */
Plink_SymTabEntryExt
Plink_FreeSymTabEntryExt (Plink_SymTabEntryExt p)
{
  if (p)
    {
      p->users = P_RemoveList (p->users, (void *(*)(void *))Plink_FreeKey);

      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Allocates a Plink_IPSymTabEntExt struct.
 *
 * \return
 *  A pointer to the new Plink_IPSymTabEntExt struct.
 */
Plink_IPSymTabEntExt
Plink_AllocIPSymTabEntExt (void)
{
  Plink_IPSymTabEntExt new;

  new = ALLOCATE (_Plink_IPSymTabEntExt);

  return (new);
}

/*! \brief Frees a Plink_IPSymTabEntExt struct.
 *
 * \param p
 *  the Plink_IPSymTabEntExt struct to free.
 *
 * \return
 *  A null Plink_IPSymTabEntExt pointer.
 */
Plink_IPSymTabEntExt
Plink_FreeIPSymTabEntExt (Plink_IPSymTabEntExt p)
{
  if (p)
    DISPOSE (p);

  return (NULL);
}

/*! \brief Allocates a Plink_TypeDclExt struct.
 *
 * \return
 *  A pointer to the new Plink_TypeDclExt struct.
 */
Plink_TypeDclExt
Plink_AllocTypeDclExt (void)
{
  Plink_TypeDclExt new;

  new = ALLOCATE (_Plink_TypeDclExt);

  return (new);
}

/*! \brief Frees a Plink_TypeDclExt struct.
 *
 * \param p
 *  the Plink_TypeDclExt struct to free.
 *
 * \return
 *  A null Plink_TypeDclExt pointer.
 */
Plink_TypeDclExt
Plink_FreeTypeDclExt (Plink_TypeDclExt p)
{
  if (p)
    {
      if (p->sig)
	free (p->sig);

      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Copies a Plink_TypeDclExt struct.
 *
 * \param p
 *  the struct to copy.
 *
 * \return
 *  A pointer to a copy of \a p.
 */
Plink_TypeDclExt
Plink_CopyTypeDclExt (Plink_TypeDclExt p)
{
  Plink_TypeDclExt new = NULL;

  if (p)
    {
      new = Plink_AllocTypeDclExt ();

      if (p->sig)
	new->sig = strdup (p->sig);
      new->multi_type = p->multi_type;
    }

  return (new);
}

/*! \brief Writes the signature for a TypeDcl.
 *
 * \param sig
 *  the signature for this module.
 * \param p
 *  the Plink_TypeDclExt containing the signature to write.
 *
 * This function writes the signature from the Plink_TypeDclExt.  This is
 * mainly used to debug Plink.  The signature is not intended to be used
 * by other modules at this time.
 */
char *
Plink_WriteTypeDclExt (char *sig, Plink_TypeDclExt p)
{
  char *result = NULL;

  if (p->sig)
    result = strdup (p->sig);

  return (result);
}

/*! \brief Allocates a Plink_StructDclExt struct.
 *
 * \return
 *  A pointer to the new Plink_StructDclExt struct.
 */
Plink_StructDclExt
Plink_AllocStructDclExt (void)
{
  Plink_StructDclExt new;

  new = ALLOCATE (_Plink_StructDclExt);

  return (new);
}

/*! \brief Frees a Plink_StructDclExt struct.
 *
 * \param p
 *  the Plink_StructDclExt struct to free.
 *
 * \return
 *  A null Plink_StructDclExt pointer.
 */
Plink_StructDclExt
Plink_FreeStructDclExt (Plink_StructDclExt p)
{
  if (p)
    {
      if (p->sig)
	free (p->sig);

      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Copies a Plink_StructDclExt struct.
 *
 * \param p
 *  the struct to copy.
 *
 * \return
 *  A pointer to a copy of \a p.
 */
Plink_StructDclExt
Plink_CopyStructDclExt (Plink_StructDclExt p)
{
  Plink_StructDclExt new = NULL;

  if (p)
    {
      new = Plink_AllocStructDclExt ();

      if (p->sig)
	new->sig = strdup (p->sig);
      new->multi_type = p->multi_type;
    }

  return (new);
}

/*! \brief Writes the signature for a StructDcl.
 *
 * \param sig
 *  the signature for this module.
 * \param p
 *  the Plink_StructDclExt containing the signature to write.
 *
 * This function writes the signature from the Plink_StructDclExt.  This is
 * mainly used to debug Plink.  The signature is not intended to be used
 * by other modules at this time.
 */
char *
Plink_WriteStructDclExt (char *sig, Plink_StructDclExt p)
{
  char *result = NULL;

  if (p->sig)
    result = strdup (p->sig);

  return (result);
}

/*! \brief Allocates a Plink_UnionDclExt struct.
 *
 * \return
 *  A pointer to the new Plink_UnionDclExt struct.
 */
Plink_UnionDclExt
Plink_AllocUnionDclExt (void)
{
  Plink_UnionDclExt new;

  new = ALLOCATE (_Plink_UnionDclExt);

  return (new);
}

/*! \brief Frees a Plink_UnionDclExt struct.
 *
 * \param p
 *  the Plink_UnionDclExt struct to free.
 *
 * \return
 *  A null Plink_UnionDclExt pointer.
 */
Plink_UnionDclExt
Plink_FreeUnionDclExt (Plink_UnionDclExt p)
{
  if (p)
    {
      if (p->sig)
	free (p->sig);
      if (p->multi_hash)
	HashLTable_free_func (p->multi_hash, (void (*)(void *))P_FreeKeyP);

      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Copies a Plink_UnionDclExt struct.
 *
 * \param p
 *  the struct to copy.
 *
 * \return
 *  A pointer to a copy of \a p.
 */
Plink_UnionDclExt
Plink_CopyUnionDclExt (Plink_UnionDclExt p)
{
  Plink_UnionDclExt new = NULL;
  Key *data;
  long key;

  if (p)
    {
      new = Plink_AllocUnionDclExt ();

      if (p->sig)
	new->sig = strdup (p->sig);
      new->multi_type = p->multi_type;

      if (p->multi_hash)
	{
	  new->multi_hash = HashLTable_create (PLINK_HASH_SIZE);

	  HashLTable_start (p->multi_hash);
	  while ((data = (Key *)HashLTable_next (p->multi_hash)))
	    {
	      key = HashLTable_curr_key (p->multi_hash);
	      HashLTable_insert (new->multi_hash, key, P_CopyKeyP (data));
	    }
	}
    }

  return (new);
}

/*! \brief Writes the signature for a UnionDcl.
 *
 * \param sig
 *  the signature for this module.
 * \param p
 *  the Plink_UnionDclExt containing the signature to write.
 *
 * This function writes the signature from the Plink_UnionDclExt.  This is
 * mainly used to debug Plink.  The signature is not intended to be used
 * by other modules at this time.
 */
char *
Plink_WriteUnionDclExt (char *sig, Plink_UnionDclExt p)
{
  char *result = NULL;

  if (p->sig)
    result = strdup (p->sig);

  return (result);
}

/*! \brief Allocates a Plink_FuncDclExt struct.
 *
 * \return
 *  A pointer to the new Plink_FuncDclExt struct.
 */
Plink_FuncDclExt
Plink_AllocFuncDclExt (void)
{
  Plink_FuncDclExt new;

  new = ALLOCATE (_Plink_FuncDclExt);

  return (new);
}

/*! \brief Frees a Plink_FuncDclExt struct.
 *
 * \param p
 *  the Plink_FuncDclExt struct to free.
 *
 * \return
 *  A null Plink_FuncDclExt pointer.
 */
Plink_FuncDclExt
Plink_FreeFuncDclExt (Plink_FuncDclExt p)
{
  if (p)
    {
      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Copies a Plink_FuncDclExt struct.
 *
 * \param p
 *  the struct to copy.
 *
 * \return
 *  A pointer to a copy of \a p.
 */
Plink_FuncDclExt
Plink_CopyFuncDclExt (Plink_FuncDclExt p)
{
  Plink_FuncDclExt new = NULL;

  if (p)
    {
      new = Plink_AllocFuncDclExt ();

      new->flags = p->flags;
    }

  return (new);
}

/*! \brief Allocates a Plink_EIData struct.
 *
 * \return
 *  A pointer to the new Plink_EIData struct.
 */
Plink_EIData
Plink_AllocEIData (void)
{
  Plink_EIData new;

  new = ALLOCATE (_Plink_EIData);

  return (new);
}

/*! \brief Frees a Plink_EIData struct.
 *
 * \param p
 *  the Plink_EIData struct to free.
 *
 * \return
 *  A null Plink_EIData pointer.
 */
Plink_EIData
Plink_FreeEIData (Plink_EIData p)
{
  Plink_Key e;

  if (p)
    {
      List_start (p->users);

      while (p->users)
	{
	  e = (Plink_Key)List_next (p->users);
	  e = Plink_FreeKey (e);
	  p->users = List_remove (p->users, e);
	}

      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Allocates a Plink_StringMap struct.
 *
 * \return
 *  A pointer to the new Plink_StringMap struct.
 */
Plink_StringMap
Plink_AllocStringMap (void)
{
  Plink_StringMap new;

  new = ALLOCATE (_Plink_StringMap);

  return (new);
}

/*! \brief Frees a Plink_StringMap struct.
 *
 * \param p
 *  the Plink_StringMap struct to free.
 *
 * \return
 *  A null Plink_StringMap pointer.
 */
Plink_StringMap
Plink_FreeStringMap (Plink_StringMap p)
{
  if (p)
    {
      if (p->lhs)
	free (p->lhs);
      if (p->rhs)
	free (p->rhs);

      DISPOSE (p);
    }

  return (NULL);
}

/*! \brief Appends a Plink_Key to the SymTabEntryExt.users field.
 *
 * \param e
 *  the SymTabEntry.
 * \param f
 *  the file for the new Plink_Key.
 * \param k
 *  the key for the new Plink_Key.
 *
 * If \a f and \a k do not already exist in e.ext.users, appends
 * a new Plink_Key containing \a f and \a k.
 */
void
Plink_AppendSymTabEntryUsers (SymTabEntry e, int f, Key k)
{
  List users = ((Plink_SymTabEntryExt)(P_GetSymTabEntryExtM (e)))->users;
  Plink_Key user;
  bool insert = TRUE;

  List_start (users);

  while ((user = (Plink_Key)List_next (users)))
    {
      if (user->file == f && P_MatchKey (user->key, k))
	{
	  insert = FALSE;
	  break;
	}
    }

  if (insert == TRUE)
    ((Plink_SymTabEntryExt)(P_GetSymTabEntryExtM (e)))->users = \
      List_insert_last (users, Plink_AllocKeyWithFileKey (f, k));

  return;
}

/*! \brief Deletes a Plink_Key from the SymTabEntryExt.users field.
 *
 * \param e
 *  the SymTabEntry.
 * \param f
 *  the file of the Plink_Key to remove.
 * \param k
 *  the key of the Plink_Key to remove.
 *
 * If a Plink_Key with file \a f and key \a k exists in the users field,
 * this function removes it.
 */
void
Plink_DeleteSymTabEntryUsers (SymTabEntry e, int f, Key k)
{
  List users = ((Plink_SymTabEntryExt)(P_GetSymTabEntryExtM (e)))->users;
  Plink_Key user;

  List_start (users);

  while ((user = (Plink_Key)List_next (users)))
    {
      if (user->file == f && P_MatchKey (user->key, k))
	{
	  Plink_FreeKey (user);
	  ((Plink_SymTabEntryExt)(P_GetSymTabEntryExtM (e)))->users = \
	    List_remove (users, user);
	  break;
	}
    }

  return;
}

/*! \brief Gets the multi_type for a Type key.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param k
 *  the key of a type in \a ip_table
 *
 * \return
 *  The value of the TypeDcl.(Plink_TypeDclExt).multi_type field.
 */
Key
Plink_GetTypeMultiType (SymbolTable ip_table, Key k)
{
  TypeDcl t;
  Key result = Invalid_Key;

  if (P_ValidKey (k) && (t = PST_GetTypeDclEntry (ip_table, k)))
    result = Plink_GetTypeDclMultiType (t);

  return (result);
}

/*! \brief Sets a link between two SymTabEntries.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param o_file
 *  the file key for the origin of the link.  This is 0 for \a ip_table,
 *  or a source file's key in \a ip_table.
 * \param o_key
 *  the key for the origin of the link.
 * \param t_file
 *  the file key for the target of the link.
 * \param t_key
 *  the key for the target of the link.
 * \param type
 *  the type of link (PLF_LT_SOURCE, PLF_LT_IP, or PLF_LT_LINK).
 *  
 * Sets the ext field of the entry keyed by \a o_key in \a o_file reference
 * \a t_key in \a t_file.
 */
void
Plink_SetLink (SymbolTable ip_table, int o_file, Key o_key, int t_file,
	       Key t_key, _Plink_SymTabEntryFlags type)
{
  SymTabEntry entry = NULL;

  if (o_file == t_file && P_MatchKey (o_key, t_key))
    P_punt ("data.c:Plink_SetLink:%d Attempting to set circular link",
	    __LINE__ - 1);

  if ((entry = Plink_GetSymTabEntry (ip_table, o_file, o_key)))
    {
      Plink_SetSymTabEntryFile (entry, t_file);
      Plink_SetSymTabEntryKey (entry, t_key);
      Plink_ClrSymTabEntryFlags (entry, PLF_LT_MASK);
      Plink_SetSymTabEntryFlags (entry, type);
    }

  return;
}

/*! \brief Gets the type of a link
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param o_file
 *  the file key for the origin of the link.  This is 0 for \a ip_table,
 *  or a source file's key in \a ip_table.
 * \param o_key
 *  the key for the origin of the link.
 *
 * \return
 *  The type of the link.
 */
_Plink_SymTabEntryFlags
Plink_GetLinkType (SymbolTable ip_table, int o_file, Key o_key)
{
  SymTabEntry entry = NULL;
  _Plink_SymTabEntryFlags result = 0;

  if ((entry = Plink_GetSymTabEntry (ip_table, o_file, o_key)))
    result = Plink_GetSymTabEntryFlags (entry) & PLF_LT_MASK;

  return (result);
}

/*! \brief Gets the target file of a link.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param o_file
 *  the file key for the origin of the link.  This is 0 for \a ip_table,
 *  or a source file's key in \a ip_table.
 * \param o_key
 *  the key for the origin of the link.
 *
 * \return
 *  The file key for the target of the link.  This is 0 for \a ip_table,
 *  or a source file's key in \a ip_table.
 */
int
Plink_GetLinkFile (SymbolTable ip_table, int o_file, Key o_key)
{
  SymTabEntry entry = NULL;
  int result = 0;

  if ((entry = Plink_GetSymTabEntry (ip_table, o_file, o_key)))
    result = Plink_GetSymTabEntryFile (entry);

  return (result);
}

/*! \brief Gets the target key of a link.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param o_file
 *  the file key for the origin of the link.  This is 0 for \a ip_table,
 *  or a source file's key in \a ip_table.
 * \param o_key
 *  the key for the origin of the link.
 *
 * \return
 *  The key for the target of the link.  If the file key is 0, this key
 *  is valid in \a ip_table.  If the file key is not 0, this key is
 *  valid in the source file symbol table corresponding to the file key.
 */
Key
Plink_GetLinkKey (SymbolTable ip_table, int o_file, Key o_key)
{
  SymTabEntry entry = NULL;
  Key result = Invalid_Key;

  if ((entry = Plink_GetSymTabEntry (ip_table, o_file, o_key)))
    result = Plink_GetSymTabEntryKey (entry);

  return (result);
}

/*! \brief Find an ExternInfo entry for a symbol
 *
 * \param name
 *  the symbol's name.
 *
 * \return
 *  The Plink_EIData struct for the symbol with name \a name.
 *
 * Searches the externs list for a symbol with name \a name.  If no entry
 * exists, a new one is added (updating \a ex) and returned.
 *
 * externs is statically declared in this file.  When externs have been
 * resolved, call Plink_cleanup_externs() to free it.
 *
 * \note The string passed as \a name is copied, so the caller must free
 *       its copy.
 *
 * \sa Plink_CleanupExterns()
 */
Plink_EIData
Plink_FindExtern (char *name)
{
  Plink_EIData d;
  
  for (List_start (externs), d = (Plink_EIData)List_next (externs); d;
       d = (Plink_EIData)List_next (externs))
    {
      if (strcmp (Plink_GetEIDataName (d), name) == 0)
	return (d);
    }

  /* No ExternInfo entry exists, so add a new one. */
  d = Plink_AllocEIData ();
  d->name = strdup (name);

  externs = List_insert_last (externs, d);

  return (d);
}

/*! \brief Returns the first extern in the externs list.
 *
 * \return
 *  The first Plink_EIData struct.
 *
 * \sa Plink_GetNextExtern()
 */
Plink_EIData
Plink_GetFirstExtern ()
{
  List_start (externs);
  return ((Plink_EIData)List_next (externs));
}

/*! \brief Returns the next extern in the externs list.
 *
 * \return
 *  The next Plink_EIData struct.
 *
 * \sa Plink_GetFirstExtern()
 */
Plink_EIData
Plink_GetNextExtern ()
{
  return ((Plink_EIData)List_next (externs));
}

/*! \brief Returns the first undefined extern in the externs list.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * \return
 *  The Plink_EIData for the first undefined extern, or NULL if all
 *  externs are defined.
 *
 * \sa Plink_GetNextUndefinedExtern()
 */
Plink_EIData
Plink_GetFirstUndefinedExtern (SymbolTable ip_table)
{
  Plink_EIData ex = NULL;
  SymTabEntry entry;
  _VarQual ex_qual;

  for (ex = Plink_GetFirstExtern (); ex; ex = Plink_GetNextExtern ())
    {
      if (P_ValidKey (ex->def.key))
	{
	  entry = Plink_GetSymTabEntry (ip_table, ex->def.file, ex->def.key);

	  if (P_GetSymTabEntryType (entry) == ET_FUNC)
	    ex_qual = P_GetFuncDclQualifier (P_GetSymTabEntryFuncDcl (entry));
	  else
	    ex_qual = P_GetVarDclQualifier (P_GetSymTabEntryVarDcl (entry));

	  if (!(ex_qual & (VQ_DEFINED | VQ_COMMON)))
	    break;
	}
      else
	{
	  break;
	}
    }

  return (ex);
}

/*! \brief Returns the next undefined extern in the externs list.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * \return
 *  The Plink_EIData for the next undefined extern, or NULL if all
 *  externs are defined.
 *
 * \sa Plink_GetFirstUndefinedExtern()
 */
Plink_EIData
Plink_GetNextUndefinedExtern (SymbolTable ip_table)
{
  Plink_EIData ex = NULL;
  SymTabEntry entry;
  _VarQual ex_qual;

  while ((ex = Plink_GetNextExtern ()))
    {
      if (P_ValidKey (ex->def.key))
	{
	  entry = Plink_GetSymTabEntry (ip_table, ex->def.file, ex->def.key);

	  if (P_GetSymTabEntryType (entry) == ET_FUNC)
	    ex_qual = P_GetFuncDclQualifier (P_GetSymTabEntryFuncDcl (entry));
	  else
	    ex_qual = P_GetVarDclQualifier (P_GetSymTabEntryVarDcl (entry));

	  if (!(ex_qual & (VQ_DEFINED | VQ_COMMON)))
	    break;
	}
      else
	{
	  break;
	}
    }

  return (ex);
}

/*! \brief Cleans up the externs list.
 */
void
Plink_CleanupExterns ()
{
  Plink_EIData t;

  List_start (externs);
  while ((t = (Plink_EIData)List_next (externs)))
    externs = List_remove (externs, t);

  return;
}

/*! \brief Adds a mapping from one string to another.
 *
 * \param map
 *  the map List to receive the new mapping.  If null, a new List will
 *  be allocated and returned.
 * \param lhs
 *  the left side of the mapping.
 * \param rhs
 *  the right side of the mapping.
 *
 * \return
 *  A pointer to \a map (a pointer to a new List if \a map is null).
 * 
 * Adds a mapping lhs <-> rhs to \a map.
 *
 * \note \a lhs and \a rhs are not copied, so the caller must not free them.
 *
 * \sa Plink_FindMapping(), Plink_FindMappingNext()
 */
List
Plink_AddMapping (List map, char *lhs, char *rhs)
{
  Plink_StringMap sm = Plink_AllocStringMap ();

  sm->lhs = lhs;
  sm->rhs = rhs;

  map = List_insert_last (map, sm);

  return (map);
}

/*! \brief Deletes a mapping from one string to another.
 *
 * \param map
 *  the map List to search for the mapping
 * \param lhs
 *  the left side of the mapping to remove.
 * \param rhs
 *  the right side of the mapping to remove.
 *
 * \return
 *  The head of the map List.
 *
 * Deletes a mapping from \a lhs to \a rhs from \a map.  If either \a lhs
 * or \a rhs is null, all mappings matching the non-null side are removed.
 */
List
Plink_DelMapping (List map, char *lhs, char *rhs)
{
  Plink_StringMap cur = NULL;

  List_start (map);
  while ((cur = (Plink_StringMap)List_next (map)))
    {
      if ((lhs == NULL || strcmp (lhs, cur->lhs) == 0) && \
	  (rhs == NULL || strcmp (rhs, cur->rhs) == 0))
	{
	  map = List_remove (map, cur);
	  cur = Plink_FreeStringMap (cur);
	  List_start (map);
	}
    }

  return (map);
}

/*! \brief Retrieves a mapping from one string to another.
 *
 * \param map
 *  the map List to query for a mapping
 * \param lhs
 *  If \a *lhs is null, this variable will return the string mapped by \a rhs.
 *  If \a lhs is defined, it will be used to find the corresponding \a rhs.
 * \param rhs
 *  If \a *rhs is null, this variable will return the string mapped by \a lhs.
 *  If \a rhs is defined, it will be used to find the corresponding \a lhs.
 *
 * \return
 *  If a mapping was found, returns TRUE.  Otherwise, returns FALSE.
 *
 * Returns the string mapped by \a lhs or \a rhs.  There may be more than
 * one string mapped by a single lhs or rhs.  Use Plink_FindMappingNext()
 * to retrieve the others.
 *
 * \note The string returned by this function must not be freed by the caller.
 *
 * \sa Plink_AddMapping(), Plink_FindMappingNext()
 */
bool
Plink_FindMapping (List map, char **lhs, char **rhs)
{
  Plink_StringMap cur = NULL;
  bool result = FALSE;

  if (lhs == NULL || rhs == NULL)
    P_punt ("data.c:Plink_FindMapping:%d lhs and rhs must not be NULL.",
	    __LINE__ - 1);

  List_start (map);
  while ((cur = (Plink_StringMap)List_next (map)))
    {
      if (*lhs == NULL && strcmp (*rhs, cur->rhs) == 0)
	{
	  *lhs = cur->lhs;
	  result = TRUE;
	  break;
	}
      if (*rhs == NULL && strcmp (*lhs, cur->lhs) == 0)
	{
	  *rhs = cur->rhs;
	  result = TRUE;
	  break;
	}
    }

  return (result);
}

/*! brief Retrieves the next mapping from one string to another.
 *
 * \param map
 *  the map List to query for a mapping
 * \param lhs
 *  If \a *lhs is null, this variable will return the string mapped by \a rhs.
 *  If \a lhs is defined, it will be used to find the corresponding \a rhs.
 * \param rhs
 *  If \a *rhs is null, this variable will return the string mapped by \a lhs.
 *  If \a rhs is defined, it will be used to find the corresponding \a lhs.
 *
 * \return
 *  If a mapping was found, returns TRUE.  Otherwise, returns FALSE.
 *
 * Returns the next string mapped by \a lhs or \a rhs.  There may be more than
 * one string mapped by a single lhs or rhs.  Use Plink_FindMappingNext()
 * to retrieve the others.
 *
 * \note The string returned by this function must not be freed by the caller.
 *
 * \sa Plink_AddMapping(), Plink_FindMapping()
 */
bool
Plink_FindMappingNext (List map, char **lhs, char **rhs)
{
  Plink_StringMap cur = NULL;
  bool result = FALSE;

  if (lhs == NULL || rhs == NULL)
    P_punt ("data.c:Plink_FindMapping:%d lhs and rhs must not be NULL.",
	    __LINE__ - 1);

  while ((cur = (Plink_StringMap)List_next (map)))
    {
      if (*lhs == NULL && strcmp (*rhs, cur->rhs) == 0)
	{
	  *lhs = cur->lhs;
	  result = TRUE;
	  break;
	}
      if (*rhs == NULL && strcmp (*lhs, cur->lhs) == 0)
	{
	  *rhs = cur->rhs;
	  result = TRUE;
	  break;
	}
    }

  return (result);
}

/*! \brief Retrieves a symbol from the symbol table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param file
 *  the file key.  0 for \a ip_table, or a source file's key in \a ip_table.
 * \param key
 *  the key to retrieve from \a file in \a ip_table.
 *
 * \return
 *  The SymTabEntry in file \a file keyed by \a key.
 *
 * A wrapper around PST_GetSymTabEntry() that can get a symbol from
 * \a ip_table or any of the Source Table's attached to \a ip_table.
 */
SymTabEntry
Plink_GetSymTabEntry (SymbolTable ip_table, int file, Key key)
{
  SymbolTable src_table;
  SymTabEntry result = NULL;

  if (file == 0)
    {
      result = PST_GetSymTabEntry (ip_table, key);
    }
  else
    {
      src_table = Plink_GetSourceTable (ip_table, file);
      result = PST_GetSymTabEntry (src_table, key);
    }

  return (result);
}

/*! \brief Returns the key in the IP Table corresponding to one in a source
 *         table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param src_file_key
 *  the source file's key in \a ip_table.
 * \param src_entry_key
 *  the original source file key.
 *
 * \return
 *  The key in \a ip_table that corresponds to \a orig_key.
 *
 * Follows the links between symbols to find the key in \a ip_table that
 * corresponds to \a src_entry_key.
 */
Key
Plink_GetIPKey (SymbolTable ip_table, int src_file_key, Key src_entry_key)
{
  int cur_file = src_file_key;
  Key cur_key = src_entry_key;
  _Plink_SymTabEntryFlags cur_flags;
  int temp_file;
  Key temp_key;

  cur_flags = Plink_GetLinkType (ip_table, cur_file, cur_key);

  while (cur_file > 0 && (cur_flags & (PLF_LT_IP | PLF_LT_LINK)))
    {
      temp_file = Plink_GetLinkFile (ip_table, cur_file, cur_key);
      temp_key = Plink_GetLinkKey (ip_table, cur_file, cur_key);

      cur_file = temp_file;
      cur_key = temp_key;
      cur_flags = Plink_GetLinkType (ip_table, cur_file, cur_key);
    }

  /* All SymTabEntries seen by this function should be in a Source Table
   * (link type PLF_LT_IP) or linked to another (PLF_LT_LINK).  It is
   * an error if we find an entry with neither type and a file key other
   * than 0. */
  if (cur_file > 0 && P_ValidKey (cur_key))
    P_punt ("data.c:Plink_GetIPKey:%d Key (%d, %d, %d) has link type 0x%x",
	    __LINE__ - 1, cur_file, cur_key.file, cur_key.sym, cur_flags);

  return (cur_key);
}

/*! \brief Returns the SymTabEntry for the TypeDcl referenced by a TypeDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param type_entry
 *  the SymTabEntry for the TypeDcl to inspect.
 *
 * \return
 *  The SymTabEntry for the TypeDcl referenced by \a type_entry.
 *
 * The keys inside a TypeDcl may or may not need to be transformed to make
 * sense.  If the TypeDcl is in its original Source Table, they do not need
 * to be transformed.  If the TypeDcl has been copied to \a ip_table,
 * Plink_GetIPKey needs to be called to map the key to another type in
 * \a ip_table.
 */
SymTabEntry
Plink_GetTypeDclType (SymbolTable ip_table, SymTabEntry type_entry)
{
  SymTabEntry result = NULL;
  TypeDcl type_dcl = P_GetSymTabEntryTypeDcl (type_entry);
  int src_file = Plink_GetSymTabEntrySrcFile (type_entry);
  Key type_key = P_GetTypeDclType (type_dcl);

  if (src_file == 0)
    type_key = Plink_GetIPKey (ip_table, Plink_GetSymTabEntryFile (type_entry),
			       type_key);

  result = Plink_GetSymTabEntry (ip_table, src_file, type_key);

  return (result);
}

/*! \brief Returns the SymTabEntry for the next Param referenced by a TypeDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param type_entry
 *  the SymTabEntry for the TypeDcl to inspect.
 * \param param
 *  a pointer to the last Param returned by this function.  If this is
 *  NULL, the first param is returned.
 *
 * \return
 *  The SymTabEntry for a TypeDcl in \a type_entry's param list.  \a param
 *  is updated with the corresponding Param structure.
 *
 * The keys inside a TypeDcl may or may not need to be transformed to make
 * sense.  If the TypeDcl is in its original Source Table, they do not need
 * to be transformed.  If the TypeDcl has been copied to \a ip_table,
 * Plink_GetIPKey needs to be called to map the key to another type in
 * \a ip_table.
 */
SymTabEntry
Plink_GetTypeDclParam (SymbolTable ip_table, SymTabEntry type_entry,
		       Param *param)
{
  SymTabEntry result = NULL;
  TypeDcl type_dcl = P_GetSymTabEntryTypeDcl (type_entry);
  int src_file = Plink_GetSymTabEntrySrcFile (type_entry);
  Key param_key;
  
  if (*param == NULL)
    *param = P_GetTypeDclParam (type_dcl);
  else
    *param = P_GetParamNext (*param);

  if (*param)
    {
      param_key = P_GetParamKey (*param);

      if (src_file == 0)
	param_key = Plink_GetIPKey (ip_table,
				    Plink_GetSymTabEntryFile (type_entry),
				    param_key);
      
      result = Plink_GetSymTabEntry (ip_table, src_file, param_key);
    }

  return (result);
}

/*! \brief Returns the SymTabEntry for the next field refererenced by a struct.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param struct_entry
 *  the SymTabEntry for the StructDcl to inspect.
 * \param field
 *  a pointer to the last Field returned by this function.  If this is
 *  NULL, the first field is returned.
 *
 * \return
 *  The SymTabEntry for a Field in the \a struct_entry's field list.
 *  \p field is updated with the corresponding Field structure.
 *
 * The keys inside a StructDcl may or may not need to be transformed to make
 * sense.  If the StructDcl is in its original Source Table, they do not need
 * to be transformed.  If the StructDcl has been copied to \a ip_table,
 * Plink_GetIPKey needs to be called to map the key to another type in
 * \a ip_table.
 */
SymTabEntry
Plink_GetStructDclField (SymbolTable ip_table, SymTabEntry struct_entry,
			 Field *field)
{
  SymTabEntry result = NULL;
  StructDcl struct_dcl = P_GetSymTabEntryStructDcl (struct_entry);
  int src_file = Plink_GetSymTabEntrySrcFile (struct_entry);
  Key field_key;

  if (*field == NULL)
    *field = P_GetStructDclFields (struct_dcl);
  else
    *field = P_GetFieldNext (*field);

  if (*field)
    {
      field_key = P_GetFieldKey (*field);

      if (src_file == 0)
	field_key = Plink_GetIPKey (ip_table,
				    Plink_GetSymTabEntryFile (struct_entry),
				    field_key);

      result = Plink_GetSymTabEntry (ip_table, src_file, field_key);
    }

  return (result);
}

/*! \brief Returns the SymTabEntry for the next field refererenced by a union.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param union_entry
 *  the SymTabEntry for the UnionDcl to inspect.
 * \param field
 *  a pointer to the last Field returned by this function.  If this is
 *  NULL, the first field is returned.
 *
 * \return
 *  The SymTabEntry for a Field in the \a union_entry's field list.
 *  \p field is updated with the corresponding Field structure.
 *
 * The keys inside a UnionDcl may or may not need to be transformed to make
 * sense.  If the UnionDcl is in its original Source Table, they do not need
 * to be transformed.  If the UnionDcl has been copied to \a ip_table,
 * Plink_GetIPKey needs to be called to map the key to another type in
 * \a ip_table.
 */
SymTabEntry
Plink_GetUnionDclField (SymbolTable ip_table, SymTabEntry union_entry,
			Field *field)
{
  SymTabEntry result = NULL;
  UnionDcl union_dcl = P_GetSymTabEntryUnionDcl (union_entry);
  int src_file = Plink_GetSymTabEntrySrcFile (union_entry);
  Key field_key;

  if (*field == NULL)
    *field = P_GetUnionDclFields (union_dcl);
  else
    *field = P_GetFieldNext (*field);

  if (*field)
    {
      field_key = P_GetFieldKey (*field);

      if (src_file == 0)
	field_key = Plink_GetIPKey (ip_table,
				    Plink_GetSymTabEntryFile (union_entry),
				    field_key);

      result = Plink_GetSymTabEntry (ip_table, src_file, field_key);
    }

  return (result);
}

/*! \brief Returns the SymTabEntry for the TypeDcl referenced by a Field.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param field_entry
 *  the SymTabEntry for the Field to inspect.
 *
 * \return
 *  The SymTabEntry for the TypeDcl referenced by \a field_entry.
 *
 * The keys inside a Field may or may not need to be transformed to make
 * sense.  If the Field is in its original Source Table, they do not need
 * to be transformed.  If the Field has been copied to \a ip_table,
 * Plink_GetIPKey needs to be called to map the key to another type in
 * \a ip_table.
 */
SymTabEntry
Plink_GetFieldType (SymbolTable ip_table, SymTabEntry field_entry)
{
  SymTabEntry result = NULL;
  Field field = P_GetSymTabEntryField (field_entry);
  int src_file = Plink_GetSymTabEntrySrcFile (field_entry);
  Key type_key = P_GetFieldType (field);

  if (src_file == 0)
    {
      /* It is possible that the field's type has not been merged yet.
       * In that case, read the type from its source file. */
      if (Plink_GetLinkType (ip_table,
			     Plink_GetSymTabEntryFile (field_entry),
			     type_key) & (PLF_LT_IP | PLF_LT_LINK))
	{
	  type_key = Plink_GetIPKey (ip_table,
				     Plink_GetSymTabEntryFile (field_entry),
				     type_key);
	}
      else
	{
	  src_file = Plink_GetSymTabEntryFile (field_entry);
	}
    }

  result = Plink_GetSymTabEntry (ip_table, src_file, type_key);

  return (result);
}

/*! \brief Follows a Source Table entry's links to find its target entry
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param entry
 *  the SymTabEntry to inspect.
 *
 * \return
 *  The Source Table SymTabEntry to which \a entry is linked.
 *
 * Follows PLF_LT_LINK and PLF_LT_SOURCE links between SymTabEntries and
 * returns the first SymTabEntry without PLF_LT_LINK set.
 */
SymTabEntry
Plink_FollowLinks (SymbolTable ip_table, SymTabEntry entry)
{
  while (Plink_GetSymTabEntryFlags (entry) & (PLF_LT_LINK | PLF_LT_SOURCE))
    entry = Plink_GetSymTabEntry (ip_table, Plink_GetSymTabEntryFile (entry),
				  Plink_GetSymTabEntryKey (entry));

  return (entry);
}

