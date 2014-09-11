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
 * \brief Functions to generate type signatures.
 *
 * \author Robert Kidd and Wen-mei Hwu.
 *
 * This file contains functions to generate type signatures.
 *
 * A type signature is the type or struct definition boiled down to
 * a character string.
 */

#include <config.h>
#include <Pcode/pcode.h>
#include <Pcode/symtab.h>
#include <Pcode/query.h>
#include "signature.h"
#include "data.h"
#include "find.h"

/*! \brief Builds the signature for all global types, structs, and unions.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param src_file_key
 *  the key of the source file in which to build all signatures.
 *
 * Builds a signature for all global types, structs, and unions in
 * \a src_file_key.
 */
void
Plink_BuildSignatures (SymbolTable ip_table, int src_file_key)
{
  SymbolTable src_table;
  SymTabEntry entry;
  char *sig;
  Key k;

  src_table = Plink_GetSourceTable (ip_table, src_file_key);

  for (k = PST_GetFileEntryByType (src_table, 1,
				   ET_TYPE_GLOBAL | ET_STRUCT | ET_UNION);
       P_ValidKey (k);
       k = PST_GetFileEntryByTypeNext (src_table, k,
				       ET_TYPE_GLOBAL | ET_STRUCT | ET_UNION))
    {
      entry = PST_GetSymTabEntry (src_table, k);
      sig = NULL;

      switch (P_GetSymTabEntryType (entry))
	{
	case ET_TYPE_GLOBAL:
	  sig = Plink_BuildTypeDclSignature (ip_table, entry);
	  break;
	case ET_STRUCT:
	  sig = Plink_BuildStructDclSignature (ip_table, entry);
	  break;
	case ET_UNION:
	  sig = Plink_BuildUnionDclSignature (ip_table, entry);
	  break;
	default:
	  break;
	}

      if (sig)
	free (sig);
    }

  return;
}

/*! \brief Updates the signatures of TypeDcls that reference a type.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param src_entry
 *  the SymTabEntry of a TypeDcl with an updated signature.
 *
 * Finds every TypeDcl that references \a src_entry and rebuilds its
 * signature.  This function then recursively rebuilds the signatures
 * of TypeDcls referencing the updated TypeDcl.  The updated signatures
 * are copied to the TypeDcls in \a ip_table corresponding to those in
 * Source Table \a src_file_key.
 */
void
Plink_UpdateSignatures (SymbolTable ip_table, SymTabEntry src_entry)
{
  Key ip_key;
  char *new_sig;
  SymTabEntry refing_entry = NULL;

  for (refing_entry = Plink_FindTypeReferencing (ip_table, src_entry);
       refing_entry;
       refing_entry = Plink_FindTypeReferencingNext (ip_table, src_entry,
						     refing_entry))
    {
      /* If this entry is already being updated, return.  This avoids looping
       * on recursive structs. */
      if (Plink_GetSymTabEntryFlags (refing_entry) & PLF_UPDATING)
	return;

      Plink_SetSymTabEntryFlags (refing_entry, PLF_UPDATING);

      switch (P_GetSymTabEntryType (refing_entry))
	{
	case ET_TYPE_GLOBAL:
	  {
	    TypeDcl src_td = P_GetSymTabEntryTypeDcl (refing_entry);

	    /* If the TypeDcl is an unnamed struct or union, copy its name
	     * from the struct or union.  The struct's or union's name is
	     * the same as its signature, so it will probably have been
	     * updated. */
	    if (P_GetTypeDclBasicType (src_td) & (BT_STRUCT | BT_UNION) && \
		P_GetTypeDclQualifier (src_td) & TY_UNNAMED)
	      {
		char *name = NULL;
		SymTabEntry e = Plink_GetTypeDclType (ip_table, refing_entry);

		if (P_GetTypeDclBasicType (src_td) == BT_STRUCT)
		  {
		    StructDcl s = P_GetSymTabEntryStructDcl (e);

		    name = P_GetStructDclName (s);
		  }
		else
		  {
		    UnionDcl u = P_GetSymTabEntryUnionDcl (e);
		    
		    name = P_GetUnionDclName (u);
		  }
		    
		if (name)
		  {
		    free (P_GetTypeDclName (src_td));
		    P_SetTypeDclName (src_td, strdup (name));
		  }
	      }

	    if (Plink_GetTypeDclSignature (src_td))
	      free (Plink_GetTypeDclSignature (src_td));
	    
	    Plink_SetTypeDclSignature (src_td, NULL);
	    new_sig = Plink_BuildTypeDclSignature (ip_table, refing_entry);

	    /* Copy the updated signature to ip_table. */
	    if (Plink_GetSymTabEntryFlags (refing_entry) & PLF_LT_IP)
	      {
		ip_key = Plink_GetSymTabEntryKey (refing_entry);

		if (P_ValidKey (ip_key))
		  {
		    TypeDcl ip_td = PST_GetTypeDclEntry (ip_table, ip_key);
		    
		    if (Plink_GetTypeDclSignature (ip_td))
		      free (Plink_GetTypeDclSignature (ip_td));
		    Plink_SetTypeDclSignature (ip_td, strdup (new_sig));
		  }
	      }

	    if (new_sig)
	      free (new_sig);
	  }
	  break;
	case ET_STRUCT:
	  {
	    StructDcl src_sd = P_GetSymTabEntryStructDcl (refing_entry);

	    /* If the struct is unnamed, use the new signature as its name. */
	    if (P_GetStructDclQualifier (src_sd) & SQ_UNNAMED)
	      {
		free (P_GetStructDclName (src_sd));
		P_SetStructDclName (src_sd, NULL);
	      }

	    if (Plink_GetStructDclSignature (src_sd))
	      free (Plink_GetStructDclSignature (src_sd));

	    Plink_SetStructDclSignature (src_sd, NULL);
	    new_sig = Plink_BuildStructDclSignature (ip_table, refing_entry);

	    if (P_GetStructDclQualifier (src_sd) & SQ_UNNAMED)
	      P_SetStructDclName (src_sd, strdup (new_sig));

	    /* Copy the updated signature to ip_table. */
	    if (Plink_GetSymTabEntryFlags (refing_entry) & PLF_LT_IP)
	      {
		ip_key = Plink_GetSymTabEntryKey (refing_entry);

		if (P_ValidKey (ip_key))
		  {
		    StructDcl ip_sd = PST_GetStructDclEntry (ip_table, ip_key);

		    if (Plink_GetStructDclSignature (ip_sd))
		      free (Plink_GetStructDclSignature (ip_sd));
		    Plink_SetStructDclSignature (ip_sd, strdup (new_sig));

		    if (P_GetStructDclQualifier (ip_sd) & SQ_UNNAMED)
		      {
			free (P_GetStructDclName (ip_sd));
			P_SetStructDclName (ip_sd, strdup (new_sig));
		      }
		  }
	      }

	    if (new_sig)
	      free (new_sig);
	  }
	  break;
	case ET_UNION:
	  {
	    UnionDcl src_ud = P_GetSymTabEntryUnionDcl (refing_entry);

	    /* If the union is unnamed, use the new signature as its name. */
	    if (P_GetUnionDclQualifier (src_ud) & SQ_UNNAMED)
	      {
		free (P_GetUnionDclName (src_ud));
		P_SetUnionDclName (src_ud, NULL);
	      }

	    if (Plink_GetUnionDclSignature (src_ud))
	      free (Plink_GetUnionDclSignature (src_ud));

	    Plink_SetUnionDclSignature (src_ud, NULL);
	    new_sig = Plink_BuildUnionDclSignature (ip_table, refing_entry);

	    if (P_GetUnionDclQualifier (src_ud) & SQ_UNNAMED)
	      P_SetUnionDclName (src_ud, strdup (new_sig));

	    /* Copy the updated signature to ip_table. */
	    if (Plink_GetSymTabEntryFlags (refing_entry) & PLF_LT_IP)
	      {
		ip_key = Plink_GetSymTabEntryKey (refing_entry);

		if (P_ValidKey (ip_key))
		  {
		    UnionDcl ip_ud = PST_GetUnionDclEntry (ip_table, ip_key);

		    if (Plink_GetUnionDclSignature (ip_ud))
		      free (Plink_GetUnionDclSignature (ip_ud));
		    Plink_SetUnionDclSignature (ip_ud, strdup (new_sig));

		    if (P_GetUnionDclQualifier (ip_ud) & SQ_UNNAMED)
		      {
			free (P_GetUnionDclName (ip_ud));
			P_SetUnionDclName (ip_ud, strdup (new_sig));
		      }
		  }
	      }

	    if (new_sig)
	      free (new_sig);
	  }
	  break;
	default:
	  P_punt ("update.c:Plink_UpdateSignatures:%d invalid SymTabEntry "
		  "type 0x%x", __LINE__ - 1,
		  P_GetSymTabEntryType (refing_entry));
	}

      Plink_UpdateSignatures (ip_table, refing_entry);

      Plink_ClrSymTabEntryFlags (refing_entry, PLF_UPDATING);
    }

  return;
}

/*! \brief Produce a signature from a TypeDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param type_dcl_entry
 *  the TypeDcl's SymTabEntry.
 *
 * \return
 *  The TypeDcl's signature.
 *
 * If the TypeDcl's signature is defined, returns that.  Otherwise,
 * builds the signature for the TypeDcl and saves it in the
 * TypeDclExt.signature field.  The TypeDclExt struct is allocated
 * if necessary.
 *
 * \note The caller must free the string returned by this function.
 */
char *
Plink_BuildTypeDclSignature (SymbolTable ip_table, SymTabEntry type_dcl_entry)
{
  TypeDcl type_dcl = P_GetSymTabEntryTypeDcl (type_dcl_entry);
  char *sig, *type_sig, *name, num[32];
  int cur_size, cur_loc;
  _TypeQual tq;
  _BasicType bt;
  char *result = NULL;

  if ((result = Plink_GetTypeDclSignature (type_dcl)))
    return (strdup (result));

  /* Used to generate the signature string.  This allocates more space for
   * the string if necessary, then adds the argument to the end. */
  #define STRCAT(x) \
	    ((cur_loc += strlen(x), cur_loc >= cur_size ? \
             cur_size = 4096 + (cur_loc & ~4095), \
             sig = realloc (sig, cur_size) : 0), strcat (sig, (x)))

  cur_size = 4096;
  cur_loc = 0;

  /* Allocate space for the signature. */
  if (!(sig = malloc (cur_size)))
    P_punt ("update.c:Plink_BuildTypeDclSignature:%d Could not allocate "
	    "signature", __LINE__);

  sig[0] = '\0';

  tq = P_GetTypeDclQualifier (type_dcl);
  bt = P_GetTypeDclBasicType (type_dcl);
  
  if (tq & TY_CONST)
    STRCAT ("co");
  if (tq & TY_VOLATILE)
    STRCAT ("vo");
  if (tq & TY_SYNC)
    STRCAT ("sy");
  if (tq & TY_EXP_ALIGN)
    STRCAT ("al");

  if (!(bt & BT_TYPEDEF) && P_GetTypeDclSize (type_dcl) > 0)
    {
      snprintf (num, sizeof (num), "%d", P_GetTypeDclSize (type_dcl));
      STRCAT (num);
    }
  if (P_GetTypeDclAlignment (type_dcl) > 0)
    {
      snprintf (num, sizeof (num), "%d", P_GetTypeDclAlignment (type_dcl));
      STRCAT (num);
    }

  if (bt & BT_UNSIGNED)
    STRCAT ("u");

  if (bt & BT_VOID)
    STRCAT ("v");
  else if (bt & BT_CHAR)
    STRCAT ("c");
  else if (bt & BT_SHORT)
    STRCAT ("s");
  else if (bt & BT_INT)
    STRCAT ("i");
  else if (bt & BT_LONG)
    STRCAT ("l");
  else if (bt & BT_LONGLONG)
    STRCAT ("ll");
  else if (bt & BT_FLOAT)
    STRCAT ("f");
  else if (bt & BT_DOUBLE)
    STRCAT ("d");
  else if (bt & BT_LONGDOUBLE)
    STRCAT ("ld");
  else if (bt & BT_POINTER)
    {
      SymTabEntry e = Plink_GetTypeDclType (ip_table, type_dcl_entry);
      TypeDcl t = P_GetSymTabEntryTypeDcl (e);

      STRCAT ("p");
      
      while (P_GetTypeDclBasicType (t) & BT_TYPEDEF)
	{
	  e = Plink_GetTypeDclType (ip_table, e);
	  t = P_GetSymTabEntryTypeDcl (e);
	}

      if (P_GetTypeDclBasicType (t) & BT_STRUCT)
	{
	  STRCAT ("st_");
	  if ((name = P_GetTypeDclName (t)))
	    STRCAT (name);
	  STRCAT ("_");
	}
      else if (P_GetTypeDclBasicType (t) & BT_UNION)
	{
	  STRCAT ("un_");
	  if ((name = P_GetTypeDclName (t)))
	    STRCAT (name);
	  STRCAT ("_");
	}
      else
	{
	  type_sig = \
	    Plink_BuildTypeDclSignature \
	      (ip_table, Plink_GetTypeDclType (ip_table, type_dcl_entry));
	  STRCAT (type_sig);
	  free (type_sig);
	}
    }
  else if (bt & BT_ARRAY)  /* Need to print array_size as well. */
    {
      type_sig = \
	Plink_BuildTypeDclSignature (ip_table,
				     Plink_GetTypeDclType (ip_table,
							   type_dcl_entry));
      STRCAT (type_sig);
      STRCAT ("ar");
      free (type_sig);
    }
  else if (bt & BT_FUNC)
    {
      Param p = NULL;
      SymTabEntry param_entry;
      char *param_sig;

      STRCAT ("r");
      type_sig = \
	Plink_BuildTypeDclSignature (ip_table,
				     Plink_GetTypeDclType (ip_table,
							   type_dcl_entry));
      STRCAT (type_sig);
      free (type_sig);
      STRCAT ("fn");
      while ((param_entry = Plink_GetTypeDclParam (ip_table, type_dcl_entry,
						   &p)))
	{
	  param_sig = Plink_BuildTypeDclSignature (ip_table, param_entry);
	  STRCAT (param_sig);
	  free (param_sig);
	}
      
      STRCAT ("fn");
    }
  else if (bt & BT_STRUCT)
    {
      SymTabEntry e = Plink_GetTypeDclType (ip_table, type_dcl_entry);
      STRCAT ("st_");
      STRCAT (Plink_GetStructDclSignature (P_GetSymTabEntryStructDcl (e)));
      STRCAT ("_");
    }
  else if (bt & BT_UNION)
    {
      SymTabEntry e = Plink_GetTypeDclType (ip_table, type_dcl_entry);
      STRCAT ("un_");
      STRCAT (Plink_GetUnionDclSignature (P_GetSymTabEntryUnionDcl (e)));
      STRCAT ("_");
    }
  else if (bt & BT_ENUM)
    STRCAT ("en");
  else if (bt & BT_TYPEDEF)
    {
      type_sig = \
	Plink_BuildTypeDclSignature \
	  (ip_table, Plink_GetTypeDclType (ip_table, type_dcl_entry));
      STRCAT (type_sig);
      free (type_sig);
    }
  
  result = strdup (sig);
  free (sig);

  /* Save the signature to the TypeDclExt field. */
  Plink_SetTypeDclSignature (type_dcl, result);

  return (strdup (result));
}

/*! \brief Produce a signature from a StructDcl.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param struct_dcl_entry
 *  the StructDcl's SymTabEntry
 *
 * \return
 *  The StructDcl's signature.
 *
 * If the StructDcl's signature is defined, returns that.  Otherwise,
 * builds the signature for the StructDcl and saves it in the
 * StructDclExt.signature field.  The StructDclExt struct is allocated
 * if necessary.
 *
 * \note The caller must free the string returned by this function.
 */
char *
Plink_BuildStructDclSignature (SymbolTable ip_table,
			       SymTabEntry struct_dcl_entry)
{
  StructDcl struct_dcl = P_GetSymTabEntryStructDcl (struct_dcl_entry);
  Field field = NULL;
  SymTabEntry field_entry;
  char *sig;
  int cur_size, cur_loc;
  char *result = NULL;

  if ((result = Plink_GetStructDclSignature (struct_dcl)))
    return (strdup (result));

  /* Used to generate the signature string.  This allocates more space for
   * the string if necessary, then adds the argument to the end. */
  #define STRCAT(x) \
	    ((cur_loc += strlen(x), cur_loc >= cur_size ? \
             cur_size = 4096 + (cur_loc & ~4095), \
             sig = realloc (sig, cur_size) : 0), strcat (sig, (x)))

  cur_size = 4096;
  cur_loc = 0;

  /* Allocate space for the signature. */
  if (!(sig = malloc (cur_size)))
    P_punt ("update.c:Plink_BuildStructDclSignature:%d Could not "
	    "allocate signature", __LINE__ - 1);

  sig[0] = '\0';

  STRCAT ("st_");
  if (P_GetStructDclName (struct_dcl))
    STRCAT (P_GetStructDclName (struct_dcl));
  STRCAT ("_");

  while ((field_entry = Plink_GetStructDclField (ip_table, struct_dcl_entry,
						 &field)))
    {
      SymTabEntry type_entry = Plink_GetFieldType (ip_table, field_entry);
      char *type_sig;

      type_sig = Plink_BuildTypeDclSignature (ip_table, type_entry);
      STRCAT (type_sig);
      free (type_sig);
      STRCAT ("_");
      STRCAT (P_GetFieldName (field));
      STRCAT ("_");
    }

  /* Add the struct qualifier so we can distinguish between incomplete structs
   * and structs defined with no fields. */
  if (P_TstStructDclQualifier (struct_dcl, SQ_EMPTY))
    STRCAT ("E");
  else if (P_TstStructDclQualifier (struct_dcl, SQ_INCOMPLETE))
    STRCAT ("I");

  result = strdup (sig);
  free (sig);

  /* Save the signature to the StructDclExt field. */
  Plink_SetStructDclSignature (struct_dcl, result);

  return (strdup (result));
}

/*! \brief Produce a signature from a UnionDcl.
 *
 * \param ip_table
 *  the symbol table.
 * \param union_dcl_entry
 *  the UnionDcl's SymTabEntry
 *
 * \return
 *  The UnionDcl's signature.
 *
 * If the UnionDcl's signature is defined, returns that.  Otherwise,
 * builds the signature for the UnionDcl and saves it in the
 * UnionDclExt.signature field.  The UnionDclExt struct is allocated if
 * necessary.
 *
 * \note The caller must free the string returned by this function.
 */
char *
Plink_BuildUnionDclSignature (SymbolTable ip_table,
			      SymTabEntry union_dcl_entry)
{
  UnionDcl union_dcl = P_GetSymTabEntryUnionDcl (union_dcl_entry);
  Field field = NULL;
  SymTabEntry field_entry;
  char *sig;
  int cur_size, cur_loc;
  char *result = NULL;

  if ((result = Plink_GetUnionDclSignature (union_dcl)))
    return (strdup (result));

  /* Used to generate the signature string.  This allocates more space for
   * the string if necessary, then adds the argument to the end. */
  #define STRCAT(x) \
	    ((cur_loc += strlen(x), cur_loc >= cur_size ? \
             cur_size = 4096 + (cur_loc & ~4095), \
             sig = realloc (sig, cur_size) : 0), strcat (sig, (x)))

  cur_size = 4096;
  cur_loc = 0;

  /* Allocate space for the signature. */
  if (!(sig = malloc (cur_size)))
    P_punt ("update.c:Plink_BuildUnionDclSignature:%d Could not "
	    "allocate signature", __LINE__ - 1);

  sig[0] = '\0';

  STRCAT ("un_");
  if (P_GetUnionDclName (union_dcl))
    STRCAT (P_GetUnionDclName (union_dcl));
  STRCAT ("_");

  while ((field_entry = Plink_GetUnionDclField (ip_table, union_dcl_entry,
						&field)))
    {
      SymTabEntry type_entry = Plink_GetFieldType (ip_table, field_entry);
      char *type_sig;

      type_sig = Plink_BuildTypeDclSignature (ip_table, type_entry);
      STRCAT (type_sig);
      free (type_sig);
      STRCAT ("_");
      STRCAT (P_GetFieldName (field));
      STRCAT ("_");
    }

  /* Add the union qualifier so we can distinguish between incomplete unions
   * and unions defined with no fields. */
  if (P_TstUnionDclQualifier (union_dcl, SQ_EMPTY))
    STRCAT ("E");
  else if (P_TstUnionDclQualifier (union_dcl, SQ_INCOMPLETE))
    STRCAT ("I");

  result = strdup (sig);
  free (sig);

  /* Save the signature to the UnionDclExt field. */
  Plink_SetUnionDclSignature (union_dcl, result);

  return (strdup (result));
}
