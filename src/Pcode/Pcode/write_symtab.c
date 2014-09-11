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
 * \brief Routines to write Pcode to a file.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains routines to write Pcode and the symbol table to a file.
 */

#include <config.h>
#include <stdio.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <library/i_list.h>
#include "pcode.h"
#include "write.h"
#include "write_symtab.h"
#include "read_symtab.h"
#include "io_util.h"
#include "struct.h"
#include "symtab.h"
#include "query.h"
#include "parse_symtab.h"
#include "struct_symtab.h"

/*! Constants for the mode argument to write_file(). */
#define NORMAL 0                /*!< The file is written normally.  Symbols
				 * are accessed through PST_GetSymTabEntry()
				 * as directed by the current search order. */
#define FLUSH  1                /*!< Flush mode.  Symbols are taken from
				 * the output file unless they exist in
				 * memory with the STF_FLUSH_ME flag. */

/*! \brief Writes a newline.
 *
 * \param o
 *  the file to write.
 *
 * \return
 *  the number of bytes written.
 */
#define write_newline(o) (P_write_newline_indent ((o), 0))

static List annotate_blocks (SymbolTable table, List entry_list);

static void write_file (SymbolTable table, int file, int mode);
static int write_dcl (FILE *out, SymbolTable table, SymTabEntry dcl);
static int write_func (FILE *out, SymbolTable table, SymTabEntry func);
static int write_header_file (FILE *out, SymbolTable table,
			      IPSymTabEnt header_file);
static int write_ip_symbol_table_list (FILE *out, SymbolTable table,
				       List ip_symbol_table_list);
static int write_ip_table (FILE *out, SymbolTable ip_table);
static int write_pcode_file (FILE *out, SymbolTable table,
			     SymTabParserArg pcode_file);
static int write_scope (FILE *out, SymbolTable table, Scope scope);
static int write_source_body (FILE *out, SymbolTable table,
			      IPSymTabEnt source_body);
static int write_source_file (FILE *out, SymbolTable table,
			      IPSymTabEnt source_file, char *include_name);
static int write_sym_tab_entry_list (FILE *out, SymbolTable table,
				     List sym_tab_entry_list);
static int write_token (FILE *out, SymbolTable table, long token,
			YYSTYPE token_data);

/*! \brief Writes a Pcode file and its symbol table.
 *
 * \param table
 *  the symbol table.
 * \param file
 *  the key of the file to write.
 *
 * Writes the file \a file and its symbol table to disk.
 *
 * If the file's out_name field is defined, that is used as the output
 * filename.  Otherwise, the file's in_name field is used.
 *
 * \note Since the output file becomes the next stage's input file, writing
 *       is done through out_table's in_file.
 */
void
PST_WriteFile (SymbolTable table, int file)
{
  write_file (table, file, NORMAL);

  return;
}

/*! \brief Updates a file on disk
 *
 * \param table
 *  the symbol table.
 * \param file
 *  the key of the file to update.
 *
 * Writes the file \a file and its symbol table to disk.  Symbols are only
 * taken from memory if the STE_FLUSH_ME flag is set.  Otherwise, they
 * are taken from the first source in (SO_OUT, SO_MEM, SO_IN).
 */
void
PST_UpdateFile (SymbolTable table, int file)
{
  /* Only write the file if it is a source file embedded in a symbol
   * table. */
  if (PST_TstFileFlags (table, file, IPSTEF_EMBEDDED) && \
      PST_GetFileType (table, file) == FT_SOURCE)
    {
      write_file (table, file, FLUSH);
    }
  else
    {
      /* Write the IP symbol table and any headers. */
      write_file (table, 0, FLUSH);
      
      /* Write the file as well, if it's separate. */  
      if (PST_GetFileType (table, file) == FT_SOURCE)
	write_file (table, file, FLUSH);
    }

  return;
}

/*! \brief Writes a file to disk.
 *
 * \param table
 *  the symbol table.
 * \param file
 *  the file's key.
 * \param mode
 *  if NORMAL, symbols are taken from the first available source in
 *  the current search order.  If FLUSH, symbols are only taken from
 *  memory if the STE_FLUSH_ME flag is set.
 *
 * Writes a file to disk.
 */
static void
write_file (SymbolTable table, int file, int mode)
{
  SymbolTable out_table;
  SymTabParserArg arg = {0};
  SymTabEntry entry;
  int bytes, len;
  FILE *out, *lock;
  char *out_name, *tmp_name;
  int f;
  Key k;

  /* Open the output symbol table. */
  if (!(out_name = PST_GetFileOutName (table, file)))
    {
      out_name = PST_GetFileInName (table, file);

      if (allow_input_update_in_place == FALSE || \
	  (file == 0 && reuse_ip_table == FALSE))
	P_punt ("write_symtab.c:PST_WriteFile:%d Unable to write file %s\n"
		"no output file specified.", __LINE__ - 1, out_name);
      else if (warn_on_update_in_place == TRUE)
	P_warn ("write_symtab.c:PST_WriteFile:%d Updating file %s in place",
		__LINE__ - 1, out_name);
    }

  /* Create a new symbol table to write. */
  out_table = P_ShCopySymbolTable (table);
  P_SetSymbolTableFlags (out_table, STF_READ_ONLY);
  P_SetSymbolTableSearchOrder (out_table, SO_MEM, SO_OUT, SO_IN);
  for (f = 1; f <= P_GetSymbolTableNumFiles (table); f++)
    PST_SetFile (out_table, f, P_ShCopyIPSymTabEnt (PST_GetFile (table, f)));

  for (f = 0; f <= P_GetSymbolTableNumFiles (table); f++)
    {
      PST_SetFileInFileStatus (out_table, f, FS_NOT_AVAIL);
      PST_SetFileOutFileStatus (out_table, f, FS_NOT_AVAIL);
    }

  /* Copy all headers to the output table if we're writing the IP symbol
   * table. */
  if (file == 0)
    {
      for (f = 1; f <= P_GetSymbolTableNumFiles (table); f++)
	{
	  if (PST_GetFileType (table, f) == FT_SOURCE)
	    continue;

	  for (k = PST_GetFileEntryByType (table, f, ET_ANY); P_ValidKey (k);
	       k = PST_GetFileEntryByTypeNext (table, k, ET_ANY))
	    {
	      entry = NULL;
	      
	      /* If mode is FLUSH, take entries from memory if STE_FLUSH_ME
	       * is set.  Otherwise, try to read from the output file to
	       * \a out_table without touching \a table.  If that doesn't
	       * work, read entry using the standard methods. */
	      if (mode == FLUSH)
		{
		  if ((entry = PST_GetSymTabEntryFromMem (table, k)) && \
		      P_TstSymTabEntryFlags (entry, STE_FLUSH_ME))
		    {
		      /* entry is in memory with STE_FLUSH_ME, so use it as is.
		       */
		    }
		  else if (PST_GetFileOutFileStatus (table, file) == FS_CLOSED)
		    {
		      PST_SetFileOutFileStatus (out_table, file, FS_CLOSED);
		      entry = PST_GetSymTabEntry (out_table, k);
		      PST_SetFileOutFileStatus (out_table, file, FS_NOT_AVAIL);

		      /* If entry was read into out_table, there't nothing to
		       * copy from table to out_table, so we can skip to the
		       * next symbol. */
		      if (entry != NULL)
			continue;
		    }
		}
	  
	      if (entry == NULL)
		entry = PST_GetSymTabEntry (table, k);

	      if (!P_TstSymTabEntryFlags (entry, STE_DELETED))
		PST_CopySymTabEntryToTableScope (out_table, Invalid_Key,
						 table, entry, TRUE);
	    }
	}
    }
  else /* Copy the source file. */
    {
      for (k = PST_GetFileEntryByType (table, file, ET_ANY); P_ValidKey (k);
	   k = PST_GetFileEntryByTypeNext (table, k, ET_ANY))
	{
	  entry = NULL;

	  /* If mode is FLUSH, take entries from memory if STE_FLUSH_ME is set.
	   * Otherwise, try to read from the output file to \a out_table
	   * without touching \a table.  If that doesn't work, read entry using
	   * the standard methods. */
	  if (mode == FLUSH)
	    {
	      if ((entry = PST_GetSymTabEntryFromMem (table, k)) && \
		  P_TstSymTabEntryFlags (entry, STE_FLUSH_ME))
		{
		  /* entry is in memory with STE_FLUSH_ME, so use it as is. */
		}
	      else if (PST_GetFileOutFileStatus (table, file) == FS_CLOSED)
		{
		  PST_SetFileOutFileStatus (out_table, file, FS_CLOSED);
		  entry = PST_GetSymTabEntry (out_table, k);
		  PST_SetFileOutFileStatus (out_table, file, FS_NOT_AVAIL);

		  /* If entry was read into out_table, there't nothing to
		   * copy from table to out_table, so we can skip to the
		   * next symbol. */
		  if (entry != NULL)
		    continue;
		}
	    }
	  
	  if (entry == NULL)
	    entry = PST_GetSymTabEntry (table, k);

	  if (!P_TstSymTabEntryFlags (entry, STE_DELETED))
	    PST_CopySymTabEntryToTableScope (out_table, Invalid_Key, table,
					     entry, TRUE);
	}
    }
  
  /* Rotate the output table so the output file becomes the next input
   * file. */
  PST_RotateFile (out_table);

  if (file == 0 || (PST_GetFileFlags (table, file) & IPSTEF_EMBEDDED))
    {
      arg.table = out_table;
    }
  else
    {
      arg.include_name = P_GetSymbolTableIPTableName (out_table);
      arg.ipste = PST_GetFile (out_table, file);
    }

  /* Write the Pcode file to a temporary file, then rename that one
   * to the output filename.  The temporary file will be out_file with
   * .<pid> appended. */
  len = strlen (out_name) + 7;
  if (!(tmp_name = malloc (len)))
    P_punt ("write_symtab.c:PST_WriteFile:%d Could not allocate tmp_name",
	    __LINE__ - 1);

  snprintf (tmp_name, len, "%s.%d", out_name, getpid ());

  /* Open tmp_name for writing and get an exclusive lock on out_name. */
  lock = P_file_open_r (out_name);
  out = P_file_open_w (tmp_name);
  bytes = write_pcode_file (out, out_table, arg);
  P_file_close (out);

  /* Replace file out_name by renaming tmp_name to out_name. */
  if (rename (tmp_name, out_name) != 0)
    P_punt ("write_symtab.c:PST_WriteFile:%d Unable to rename\n%s to %s, "
	    "errno = %d", __LINE__ - 1, tmp_name, out_name, errno);
  if (lock)
    P_file_close (lock);

  if (file == 0)
    P_SetSymbolTableOutFileStatus (table, FS_CLOSED);
  else
    PST_SetFileOutFileStatus (table, file, FS_CLOSED);

  out_table = PST_RemoveSymbolTable (out_table);

  if (tmp_name)
    free (tmp_name);

  return;
}

/*! \brief Writes the dcl rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param dcl
 *  the dcl SymTabEntry to write.
 *
 * \return
 *  The number of bytes written.
 */
static int
write_dcl (FILE *out, SymbolTable table, SymTabEntry dcl)
{
  YYSTYPE data;
  _Dcl _dcl;
  int bytes = 0;
  
  /* Write all Dcls in the scope. */
  data._dcl = &_dcl;

  /* Update the offset and write the dcl if it can be read independently. */
  switch (P_GetSymTabEntryType (dcl))
    {
    case ET_FUNC:
      P_punt ("write_symtab.c:write_dcl:%d Func should be handled by "
	      "write_func()", __LINE__ - 1);
      break;
    case ET_TYPE_GLOBAL:
      P_write_offset (out, &P_GetSymTabEntryOffset (dcl), 0);
      P_SetSymTabEntryOffset (dcl, 0);
      
      P_SetDclType (data._dcl, TT_TYPE);
      P_SetDclTypeDcl (data._dcl, P_GetSymTabEntryTypeDcl (dcl));
      bytes += write_token (out, table, DCL, data);
      break;
    case ET_VAR_GLOBAL:
      P_write_offset (out, &P_GetSymTabEntryOffset (dcl), 0);
      P_SetSymTabEntryOffset (dcl, 0);
      
      P_SetDclType (data._dcl, TT_VAR);
      P_SetDclVarDcl (data._dcl, P_GetSymTabEntryVarDcl (dcl));
      bytes += write_token (out, table, DCL, data);
      break;
    case ET_STRUCT:
    case ET_UNION:
      {
	Field f;
	SymTabEntry field_entry;

	P_write_offset (out, &P_GetSymTabEntryOffset (dcl), 0);
	P_SetSymTabEntryOffset (dcl, 0);
      
	/* Use the parent struct/union's offset as the offset for each field.
	 */
	for (f = P_GetSymTabEntryFields (dcl); f; f = P_GetFieldNext (f))
	  {
	    field_entry = PST_GetSymTabEntry (table, P_GetFieldKey (f));
	    P_write_offset (out, &P_GetSymTabEntryOffset (field_entry), 0);
	    P_SetSymTabEntryOffset (field_entry, 0);
	  }

	if (P_GetSymTabEntryType (dcl) == ET_STRUCT)
	  {
	    P_SetDclType (data._dcl, TT_STRUCT);
	    P_SetDclStructDcl (data._dcl, P_GetSymTabEntryStructDcl (dcl));
	  }
	else
	  {
	    P_SetDclType (data._dcl, TT_UNION);
	    P_SetDclUnionDcl (data._dcl, P_GetSymTabEntryUnionDcl (dcl));
	  }

	bytes += write_token (out, table, DCL, data);
      }
      break;
    case ET_ENUM:
      {
	EnumField f;
	SymTabEntry field_entry;

	P_write_offset (out, &P_GetSymTabEntryOffset (dcl), 0);
	P_SetSymTabEntryOffset (dcl, 0);
      
	/* Use the EnumDcl's offset as the offset for each field. */
	for (f = P_GetEnumDclFields (P_GetSymTabEntryEnumDcl (dcl)); f;
	     f = P_GetEnumFieldNext (f))
	  {
	    field_entry = PST_GetSymTabEntry (table, P_GetEnumFieldKey (f));
	    P_write_offset (out, &P_GetSymTabEntryOffset (field_entry), 0);
	    P_SetSymTabEntryOffset (field_entry, 0);
	  }

	P_SetDclType (data._dcl, TT_ENUM);
	P_SetDclEnumDcl (data._dcl, P_GetSymTabEntryEnumDcl (dcl));
	bytes += write_token (out, table, DCL, data);
      }
      break;
    case ET_ASM:
      P_write_offset (out, &P_GetSymTabEntryOffset (dcl), 0);
      P_SetSymTabEntryOffset (dcl, 0);
      
      P_SetDclType (data._dcl, TT_ASM);
      P_SetDclAsmDcl (data._dcl, P_GetSymTabEntryAsmDcl (dcl));
      bytes += write_token (out, table, DCL, data);
      break;
    default: /* No independent symbol to write */
      break;
    }

  if (bytes > 0)
    bytes += write_newline (out);
 
  return (bytes);
}

/*! \brief Writes the func rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param func
 *  the func SymTabEntry to write.
 *
 * \return
 *  The number of bytes written.
 */
static int
write_func (FILE *out, SymbolTable table, SymTabEntry func)
{
  Scope scope;
  ScopeEntry se, flattened_scope;
  YYSTYPE data;
  List list = NULL;
  _Dcl _dcl;
  int bytes = 0;

  scope = P_GetSymTabEntryScope (func);

  /* Write the function's offset. */
  P_write_offset (out, &P_GetSymTabEntryOffset (func), 0);
  P_SetSymTabEntryOffset (func, 0);

  data._symtabentry = func;
  bytes = write_token (out, table, SYM_TAB_ENTRY_FUNC, data);
  bytes += write_newline (out);

  flattened_scope = PST_FlattenScope (table, scope);

  /* Set up the list of SymTabEntries. */
  for (se = flattened_scope; se; se = P_GetScopeEntryNext (se))
    list = List_insert_last (list, PST_GetSymTabEntry (table, se->key));

  flattened_scope = P_RemoveScopeEntry (flattened_scope);

  bytes += write_sym_tab_entry_list (out, table, list);

  list = P_RemoveList (list, NULL);

  data._dcl = &_dcl;
  P_SetDclType (data._dcl, TT_FUNC);
  P_SetDclFuncDcl (data._dcl, P_GetSymTabEntryFuncDcl (func));
  bytes += write_token (out, table, DCL_FUNC, data);

  return (bytes);
}

/*! \brief Writes the header_file rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param header_file
 *  the header_file IPSymTabEnt to write.
 *
 * \return
 *  The number of bytes written.
 */
static int
write_header_file (FILE *out, SymbolTable table, IPSymTabEnt header_file)
{
  int bytes = 0;
  YYSTYPE data;
  Key k;
  SymTabEntry entry;
  List list = NULL;

  data._ipsymtabent = header_file;
  bytes = write_token (out, table, IP_SYM_TAB_ENT_HEADER, data);
  bytes += write_newline (out);

  /* Write all SymTabEntries in the header. */
  for (k = PST_GetFileEntryByType (table, P_GetIPSymTabEntKey (header_file),
				   ET_ANY);
       P_ValidKey (k); k = PST_GetFileEntryByTypeNext (table, k, ET_ANY))
    list = List_insert_last (list, PST_GetSymTabEntry (table, k));

  bytes += write_sym_tab_entry_list (out, table, list);

  data._symtabentry = NULL;
  bytes += write_token (out, table, SYM_TAB_ENTRY_END, data);
  bytes += write_newline (out);

  List_start (list);
  while ((entry = (SymTabEntry) List_next (list)))
    {
      if (P_GetSymTabEntryType (entry) == ET_FUNC)
	bytes += write_func (out, table, entry);
      else
	bytes += write_dcl (out, table, entry);

      bytes += write_newline (out);
    }

  list = P_RemoveList (list, NULL);

  return (bytes);
}

/*! \brief Writes the ip_symbol_table_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param ip_symbol_table_list
 *  the ip_symbol_table_list List to write.
 *
 * \return
 *  the number of bytes written.
 *
 * Writes all IPSymTabEnts from \a ip_symbol_table_list to disk.
 */
static int
write_ip_symbol_table_list (FILE *out, SymbolTable table,
			    List ip_symbol_table_list)
{
  int bytes = 0;
  YYSTYPE data;

  List_start (ip_symbol_table_list);
  while ((data._ipsymtabent = (IPSymTabEnt)List_next (ip_symbol_table_list)))
    {
      /* Set the IPSymTabEnt's offset to 0 so the file position of the
       * offset field will be noted.  If the rest of the file is written
       * to this file, its file offset will be written at this position. */
      P_SetIPSymTabEntOffset (data._ipsymtabent, 0);
      if (P_GetIPSymTabEntFileType (data._ipsymtabent) == FT_HEADER)
	bytes += write_token (out, table, IP_SYM_TAB_ENT_HEADER, data);
      else if (P_GetIPSymTabEntFileType (data._ipsymtabent) == FT_SOURCE)
	bytes += write_token (out, table, IP_SYM_TAB_ENT_SOURCE, data);
      else
	P_punt ("write_symtab.c:write_ip_symbol_table_list:%d Unknown file "
		"type %d", __LINE__ - 1,
		P_GetIPSymTabEntFileType (data._ipsymtabent));

      bytes += write_newline (out);
    }

  return (bytes);
}


/*! \brief Writes the ip_table rule to disk.
 *
 * \param out
 *  the file to write.
 * \param ip_table
 *  the ip_table Symbol Table to write.
 *
 * \return
 *  the number of bytes written.
 */
static int
write_ip_table (FILE *out, SymbolTable ip_table)
{
  YYSTYPE data;
  int bytes = 0;
  int i;
  IPSymTabEnt ipste;
  List list = NULL;

  data._symboltable = ip_table;
  bytes = write_token (out, ip_table, SYMBOL_TABLE_BEGIN, data);
  bytes += write_newline (out);

  for (i = 1; i <= P_GetSymbolTableNumFiles (ip_table); i++)
    list = List_insert_last (list, PST_GetFile (ip_table, i));

  bytes += write_ip_symbol_table_list (out, ip_table, list);

  list = P_RemoveList (list, NULL);

  data._symboltable = NULL;
  bytes += write_token (out, ip_table, SYMBOL_TABLE_END, data);
  bytes += write_newline (out);

  for (i = 1; i <= P_GetSymbolTableNumFiles (ip_table); i++)
    {
      ipste = PST_GetFile (ip_table, i);

      if (P_GetIPSymTabEntFileType (ipste) == FT_HEADER)
	{
	  P_write_offset (out, &PST_GetFileOffset (ip_table, i), 0);
	  /* Set the offset to 0 so it won't be overwritten by
	   * P_write_ip_sym_tab_ent(). */
	  PST_SetFileOffset (ip_table, i, 0);
	  bytes += write_header_file (out, ip_table, ipste);
	}
	  
      bytes += write_newline (out);
    }

  /* If there is no file name for the interprocedural symbol table, it is
   * part of the source file.  Write the source file now. */
  if (P_GetSymbolTableIPTableName (ip_table) == NULL && \
      P_GetIPSymTabEntFileType (PST_GetFile (ip_table, 1)) == FT_SOURCE)
    {
      P_write_offset (out, &PST_GetFileOffset (ip_table, 1), 0);
      /* Set the offset to 0 so it won't be overwritten by
       * P_write_ip_sym_tab_ent(). */
      PST_SetFileOffset (ip_table, 1, 0);
      bytes += write_source_body (out, ip_table, PST_GetFile (ip_table, 1));
      bytes += write_newline (out);
    }

  return (bytes);
}

/*! \brief Writes the pcode_file rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param pcode_file
 *  the pcode_file SymTabParserArg to write.
 *
 * \return
 *  the number of bytes written.
 */
static int
write_pcode_file (FILE *out, SymbolTable table, SymTabParserArg pcode_file)
{
  int bytes = 0;

  /* Either write the interprocedural symbol table or a source file. */
  if (pcode_file.table)
    bytes = write_ip_table (out, pcode_file.table);
  else
    bytes = write_source_file (out, table, pcode_file.ipste,
			       pcode_file.include_name);

  return (bytes);
}

/*! \brief Writes the scope rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param scope
 *  the scope List to write.
 *
 * \return
 *  the number of bytes written.
 */
static int
write_scope (FILE *out, SymbolTable table, Scope scope)
{
  int bytes = 0;
  int scope_file_key = P_GetScopeKey (scope).file;
  YYSTYPE data;
  ScopeEntry se;
  SymTabEntry entry;
  List list = NULL;

  /* Write the scope's entry. */
  data._symtabentry = PST_GetSymTabEntry (table, P_GetScopeKey (scope));
  bytes = write_token (out, table, SYM_TAB_ENTRY_SCOPE, data);
  bytes += write_newline (out);

  for (se = P_GetScopeScopeEntry (scope); se; se = P_GetScopeEntryNext (se))
    list = List_insert_last (list, PST_GetSymTabEntry (table, se->key));

  bytes += write_sym_tab_entry_list (out, table, list);

  /* Remove any SymTabEntries that aren't in this file. */
  list = P_RemoveList (list, NULL);

  data._symtabentry = NULL;
  bytes += write_token (out, table, SYM_TAB_ENTRY_END, data);
  bytes += write_newline (out);

  for (se = P_GetScopeScopeEntry (scope); se; se = P_GetScopeEntryNext (se))
    {
      if (se->key.file == scope_file_key)
	{
	  entry = PST_GetSymTabEntry (table, se->key);

	  if (P_GetSymTabEntryType (entry) == ET_FUNC)
	    bytes += write_func (out, table, entry);
	  else
	    bytes += write_dcl (out, table, entry);
	}

      bytes += write_newline (out);
    }

  return (bytes);
}

/*! \brief Writes the source_body rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param source_body
 *  the source_body IPSymTabEnt to write.
 *
 * \return
 *  the number of bytes written.
 */
static int
write_source_body (FILE *out, SymbolTable table, IPSymTabEnt source_body)
{
  YYSTYPE data;
  int bytes = 0;
  Key scope_key = {P_GetIPSymTabEntKey (source_body), 1};
  Scope scope;

  if (source_body)
    {
      data._ipsymtabent = source_body;

      bytes = write_token (out, table, IP_SYM_TAB_ENT_SOURCE, data);
      bytes += write_newline (out);

      if ((scope = PST_GetScope (table, scope_key)))
	bytes += write_scope (out, table, scope);
    }

  return (bytes);
}

/*! \brief Writes the source_file rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param source_file
 *  the source_file IPSymTabEnt to write.
 * \param include_name
 *  the name of the IP Table.
 *
 * \return
 *  the number of bytes written.
 */
int
write_source_file (FILE *out, SymbolTable table, IPSymTabEnt source_file,
		   char *include_name)
{
  YYSTYPE data;
  int bytes = 0;

  data._includename = include_name;

  /* This source file's IPSymTabEnt will be written to a file other than
   * the IP symbol table, so the offset field is meaningless.  Set it
   * to 0 so things don't get confused. */
  P_SetIPSymTabEntOffset (source_file, 0);

  bytes = write_token (out, table, INCLUDE_NAME, data);
  bytes += write_newline (out);
  bytes += write_source_body (out, table, source_file);
  bytes += write_newline (out);

  return (bytes);
}

/*! \brief Writes the sym_tab_entry_list rule to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param sym_tab_entry_list
 *  the sym_tab_entry_list List to write.
 *
 * \return
 *  the number of bytes written.
 *
 * Writes the SymTabEntry for each entry in the list.  The Pcode structure
 * referenced by the SymTabEntry is not written.
 */
static int
write_sym_tab_entry_list (FILE *out, SymbolTable table,
			  List sym_tab_entry_list)
{
  int bytes = 0;
  YYSTYPE data;
  List annotated_list;

  if (sym_tab_entry_list)
    {
      /* Insert ET_BLOCK entries so the read routines can pre-allocate
       * space. */
      annotated_list = annotate_blocks (table, sym_tab_entry_list);

      List_start (annotated_list);
      while ((data._symtabentry = (SymTabEntry)List_next (annotated_list)))
	{
	  /* Set the offset to zero so P_write_offset will note the
	   * file position of this offset field.  When the SymTabEntry's
	   * data is written, its file offset will be stored at
	   * the offset recorded here. */
	  P_SetSymTabEntryOffset (data._symtabentry, 0);
	  switch (P_GetSymTabEntryType (data._symtabentry))
	    {
	    case ET_BLOCK:
	      bytes += write_token (out, table, SYM_TAB_ENTRY_BLOCK, data);
	      data._symtabentry = P_RemoveSymTabEntry (data._symtabentry);
	      break;
	    case ET_FUNC:
	      bytes += write_token (out, table, SYM_TAB_ENTRY_FUNC, data);
	      break;
	    case ET_SCOPE:
	      bytes += write_token (out, table, SYM_TAB_ENTRY_SCOPE, data);
	      break;
	    default:
	      bytes += write_token (out, table, SYM_TAB_ENTRY_OTHER, data);
	      break;
	    }

	  bytes += write_newline (out);
	}
      annotated_list = P_RemoveList (annotated_list, NULL);

    }

  return (bytes);
}

/*! \brief Writes a symbol table level token to disk.
 *
 * \param out
 *  the file to write.
 * \param table
 *  the symbol table.
 * \param token
 *  the token to write.
 * \param data
 *  the union containing the token's data.
 *
 * \return
 *  the number of bytes written.
 *
 * Writes a symbol table level token to disk.  This 'token' is really a Pcode
 * structure, which is written by the routines in write.[ch].
 */
static int
write_token (FILE *out, SymbolTable table, long token, YYSTYPE data)
{
  int bytes = 0;
  int lines = 0;
  _Dcl _dcl = {0};

  switch (token)
    {
    case DCL:
    case DCL_FUNC:
      bytes += P_write_dcl (out, data._dcl, 0, &lines);
      break;
    case INCLUDE_NAME:
      P_SetDclType (&_dcl, TT_INCLUDE);
      P_SetDclInclude (&_dcl, data._includename);
      bytes += P_write_dcl (out, &_dcl, 0, &lines);
      break;
    case IP_SYM_TAB_ENT_HEADER:
    case IP_SYM_TAB_ENT_SOURCE:
      bytes += P_write_ip_sym_tab_ent (out, data._ipsymtabent, 0, &lines);
      break;
    case SYMBOL_TABLE_BEGIN:
    case SYMBOL_TABLE_END:
      bytes += P_write_symbol_table (out, data._symboltable, 0, &lines);
      break;
    case SYM_TAB_ENTRY_BLOCK:
    case SYM_TAB_ENTRY_END:
    case SYM_TAB_ENTRY_FUNC:
    case SYM_TAB_ENTRY_SCOPE:
    case SYM_TAB_ENTRY_OTHER:
      bytes += P_write_symbol_table_entry (out, data._symtabentry, 0, &lines);
      break;
    }

  return (bytes);
}

/*! \brief Inserts ET_BLOCK entries to denote contiguous blocks of indices.
 *
 * \param table
 *  the symbol table.
 * \param entry_list
 *  the list of entries to annotate.
 *
 * \return
 *  A copy of \a entry_list with ET_BLOCK entries inserted.
 *
 * Inserts ET_BLOCK SymTabEntries to denote contiguous blocks of indices.
 * The read routines use these entries to pre-allocate space.
 *
 * \note The caller must free the list returned by this function.
 */
static List
annotate_blocks (SymbolTable table, List entry_list)
{
  SymTabEntry entry, block;
  Key entry_key, first_key, last_key;
  List result = NULL, block_list = NULL;

  List_start (entry_list);
  entry = (SymTabEntry)List_next (entry_list);

  entry_key = P_GetSymTabEntryKey (entry);
  first_key = entry_key;
  last_key = entry_key;

  block_list = List_insert_last (block_list, entry);

  /* first_key is the key of the first entry in the block.
   * last_key is the key of the last entry in the block.
   * entry_key is the key of the current entry being inspected.
   * entries [first, last] are copied to block_list
   *
   * If entry_key is last_key + 1, move entry_key and last_key forward one.
   * Otherwise,
   *  if the block size is big enough, add a marker to the output list
   *  copy the block indicated by first, last to the output list.
   */
  while ((entry = (SymTabEntry)List_next (entry_list)))
    {
      entry_key = P_GetSymTabEntryKey (entry);

      if (!(last_key.file == entry_key.file && \
	    last_key.sym == (entry_key.sym - 1)))
	{
	  if (last_key.sym - first_key.sym + 1 > HINT_MIN_BLOCK_SIZE)
	    {
	      block = P_NewSymTabEntry ();
	      P_SetSymTabEntryType (block, ET_BLOCK);
	      P_SetSymTabEntryBlockStart (block, first_key);
	      P_SetSymTabEntryBlockSize (block,
					 last_key.sym - first_key.sym + 1);

	      result = List_insert_last (result, block);
	    }

	  result = List_append (result, block_list);

	  block_list = NULL;
	  first_key = entry_key;
	}

      block_list = List_insert_last (block_list, entry);
      last_key = entry_key;
    }

  if (last_key.sym - first_key.sym + 1 > HINT_MIN_BLOCK_SIZE)
    {
      block = P_NewSymTabEntry ();
      P_SetSymTabEntryType (block, ET_BLOCK);
      P_SetSymTabEntryBlockStart (block, first_key);
      P_SetSymTabEntryBlockSize (block, last_key.sym - first_key.sym + 1);
      result = List_insert_last (result, block);
    }

  result = List_append (result, block_list);

  return (result);
}
