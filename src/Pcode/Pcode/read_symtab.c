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
 * \brief Routines to read Pcode from a file.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains routines to read Pcode and the symbol table from a file.
 */
/*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "pcode.h"
#include "io_util.h"
#include "symtab.h"
#include "query.h"
#include "struct.h"
#include "read.h"
#include "read_symtab.h"
#include "lex_symtab.h"
#include "struct_symtab.h"

/* PST_parse() is defined in parse_symtab.c, which is generated from
 * parse_symtab.y. */
extern int PST_yyparse (void *);

/*! \brief Reads a symbol table from an input file.
 *
 * \param in
 *  the file to read from.
 * \param file
 *  returns the file key for \a in.
 *
 * \return
 *  The symbol table contained in \a in.
 *
 * Returns a pointer to the symbol table for \a in.  If \a in contains
 * a source file, its key in the symbol table is returned in \a file.  If
 * \a in only contains the interprocedural symbol table, \a file is set to 0.
 */
SymbolTable
PST_ReadFile (FILE *in, int *file)
{
  SymTabParserArg arg = {0};
  int result_file = 0;
  SymbolTable result_table;

  PST_yyin = in;

  if (PST_yyparse ((void *)&arg) != 0)
    P_punt ("read_symtab.c:PST_ReadFile:%d Error reading file", __LINE__);

  /* There are three possibilities at this point.
   * 1. Source file with embedded IP symbol table (Pcode before linking).
   *    -arg.table is defined, and does not have STF_LINKED flag.  Source
   *     file and headers have not been read.
   *    +Seek file and read headers.
   *
   * 2. Source file with INCLUDEd IP symbol table (Pcode after linking).
   *    -arg.ipste and arg.include_name are defined.
   *    +Read IP symbol table (case 3) from arg.include_name.  Insert
   *     arg.ipste.  Set *file to the source file's key.
   *
   * 3. IP symbol table (Pcode after linking).
   *    -arg.table is defined and has STF_LINKED flag.  Source
   *     files and headers have not been read.
   *    +Seek file and read headers.
   */
  if (arg.table == NULL) /* case 2 */
    {
      if (arg.ipste == NULL || arg.include_name == NULL)
	P_punt ("read_symtab.c:PST_ReadFile:%d arg.ipste or arg.include are "
		"null", __LINE__ - 1);

      result_file = P_GetIPSymTabEntKey (arg.ipste);

      result_table = PST_Open (arg.include_name, NULL, 0);

      if (PST_GetFile (result_table, result_file))
	P_RemoveIPSymTabEnt (PST_GetFile (result_table, result_file));
      PST_SetFile (result_table, result_file, arg.ipste);
    }
  else /* cases 1 and 3 */
    {
      int i;

      result_table = arg.table;

      arg.table = NULL;

      for (i = 1; i <= P_GetSymbolTableNumFiles (result_table); i++)
	{
	  IPSymTabEnt ipste = PST_GetFile (result_table, i);
	  
	  if (P_GetIPSymTabEntFileType (ipste) == FT_HEADER)
	    {
	      P_input_seek (in, P_GetIPSymTabEntOffset (ipste));
	      if (PST_yyparse ((void *)&arg) != 0)
		P_punt ("read_symtab.c:PST_ReadFile:%d Error reading file",
			__LINE__ - 1);

	      P_RemoveIPSymTabEnt (PST_GetFile (result_table, i));
	      PST_SetFile (result_table, i, arg.ipste);
	      arg.ipste = NULL;
	    }
	  else if (!(P_GetSymbolTableFlags (result_table) & STF_LINKED) && \
		   P_GetIPSymTabEntFileType (ipste) == FT_SOURCE) /* case 1 */
	    {
	      /* If this file is flushed, we'll need to know its offset
	       * to re-read it.  We therefore need to preserve the offset. */
	      int offset = P_GetIPSymTabEntOffset (ipste);

	      P_input_seek (in, offset);
	      if (PST_yyparse ((void *)&arg) != 0)
		P_punt ("read_symtab.c:PST_ReadFile:%d Error reading file",
			__LINE__ - 1);

	      P_SetIPSymTabEntFlags (arg.ipste, IPSTEF_EMBEDDED);
	      P_SetIPSymTabEntOffset (arg.ipste, offset);

	      if (result_file != 0)
		P_punt ("read_symtab.c:PST_ReadFile:%d two source files where "
			"there should\nbe one (%d, %d)", __LINE__ - 1,
			result_file, P_GetIPSymTabEntKey (arg.ipste));
	      else
		result_file = P_GetIPSymTabEntKey (arg.ipste);

	      P_RemoveIPSymTabEnt (PST_GetFile (result_table, i));
	      PST_SetFile (result_table, i, arg.ipste);
	      arg.ipste = NULL;

	    }
	}
    }

  P_SetSymbolTableModifiableFile (result_table, result_file);
  *file = result_file;
  return (result_table);
}

/*! \brief Reads a symbol table entry from the input file on disk.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key for the table entry to read.
 *
 * \return
 *  The newly read SymTabEntry.
 *
 * Reads a symbol table entry from the input file on disk and stores it
 * in the table.  This function opens the file if necessary and seeks to the
 * appropriate point.
 *
 * If this function opens the input file, it leaves the file open and
 * stores the file handle in the symbol table.
 */
SymTabEntry
PST_ReadSymbolFromIn (SymbolTable table, Key key)
{
  SymTabEntry result = NULL;
  FILE *in = NULL;
  SymTabParserArg arg = {0};

  switch (PST_GetFileInFileStatus (table, key.file))
    {
    case FS_CLOSED:
      {
	BlockSparseArray tmp = NULL;

	/* If the source file is closed, open it and read the top level of its
	 * symbol table. */
	in = P_file_open_r (PST_GetFileInName (table, key.file));

	if (PST_GetFileOffset (table, key.file > 0))
	  P_input_seek (in, PST_GetFileOffset (table, key.file));
	
	arg.table = table;
	PST_yyin = in;
	if (PST_yyparse ((void *)&arg) != 0)
	  P_punt ("read_symtab.c:PST_ReadSymbolFromIn:%d Error reading file "
		  "%d", __LINE__ - 1, key.file);

	/* Reading a source file returned an include file name and the
	 * IPSymTabEnt for this file.  We already have the symbol table
	 * open, so we can throw the include file name away. */
	if (arg.include_name)
	  {
	    free (arg.include_name);
	    arg.include_name = NULL;
	  }

	if (arg.ipste == NULL)
	  P_punt ("read_symtab.c:PST_ReadSymbolFromIn:%d Could not read "
		  "IPSymTabEnt for file %d", __LINE__ - 1, key.file);
      
	/* Make sure the IPSymTabEnts refer to the same file. */
	if (strcmp (PST_GetFileSourceName (table, key.file),
		    P_GetIPSymTabEntSourceName (arg.ipste)) != 0)
	  P_warn ("read_symtab.c:PST_ReadSymbolFromIn:%d reading different "
		  "file %d", __LINE__ - 1, key.file);

	if ((tmp = PST_GetFileTable (table, key.file)))
	  P_warn ("read_symtab.c:PST_ReadSymbolFromIn:%d overwriting symbol "
		  "array for file %d", __LINE__ - 1, key.file);

	PST_SetFileTable (table, key.file, P_GetIPSymTabEntTable (arg.ipste));
	P_SetIPSymTabEntTable (arg.ipste, tmp);
	arg.ipste = P_RemoveIPSymTabEnt (arg.ipste);

	PST_SetFileFile (table, key.file, in);
	PST_SetFileInFileStatus (table, key.file, FS_READ_PERM);
      }
      break;
    case FS_READ_PERM:
      in = PST_GetFileFile (table, key.file);
      break;
    case FS_NOT_AVAIL:
      goto done;
      break;
    default:
      P_punt ("read_symtab.c:PST_ReadSymbolFromIn:%d Invalid _FileStatus %d",
	      __LINE__ - 1, PST_GetFileInFileStatus (table, key.file));
      break;
    }

  /* If this SymTabEntry can be read independently, it will have a non-zero
   * offset.  In this case, we can seek to that offset and read the rest
   * of the symbol. */
  if ((result = PST_GetSymTabEntryFromMem (table, key)) && \
      P_GetSymTabEntryOffset (result) > 0)
    {
      P_input_seek (in, P_GetSymTabEntryOffset (result));
      PST_yyin = in;
      arg.table = table;
      /* Since the SymTabEntry (result) already exists in memory, PST_yyparse()
       * will fill it with its Pcode structure. */
      if (PST_yyparse ((void *)&arg) != 0)
	P_punt ("read_symtab.c:PST_ReadSymbolFromIn:%d Error reading symbol "
		"(%d, %d)", __LINE__ - 1, key.file, key.sym);
    }

 done:
  return (result);
}

/*! \brief Reads a symbol table entry from the output file on disk.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key for the table entry to read.
 *
 * \return
 *  The newly read SymTabEntry, or NULL if it doesn't exist.
 *
 * Reads a symbol table entry from the output file on disk and stores it
 * in the table.  This function opens the output file, reads the symbol,
 * then closes the file.
 *
 * If the output file does not exist, or if the requested symbol does
 * not exist in the output file, this function returns NULL.
 */
SymTabEntry
PST_ReadSymbolFromOut (SymbolTable table, Key key)
{
  SymbolTable in_table;
  SymTabEntry result = NULL;
  char *out_name;
  SearchOrder orig_order;

  if (PST_GetFileOutFileStatus (table, key.file) == FS_NOT_AVAIL || \
      !(out_name = PST_GetFileOutName (table, key.file)))
    goto done;

  /* Read the output file into a temporary table. */
  in_table = PST_Open (out_name, NULL, STF_READ_ONLY);

  P_CopySearchOrder (orig_order, table->search_order);

  /* We're reading from the output table (in_table), so set the search
   * order in the original table ot skip SO_OUT to prevent a loop. */
  P_SetSymbolTableSearchOrder (table, SO_MEM, SO_IN, SO_MEM);

  result = PST_CopySymTabEntryToTableScope (table, Invalid_Key, in_table,
					    PST_GetSymTabEntry (in_table, key),
					    TRUE);

  P_CopySearchOrder (table->search_order, orig_order);

  PST_Close (in_table);

 done:
  return (result);
}

/*! \brief Reads a copy of a symbol table entry from the input file on disk.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key for the table entry to read.
 *
 * \return
 *  The newly read SymTabEntry, or NULL if it doesn't exist.
 *
 * Reads a symbol table entry from the input file on disk and stores a copy
 * in the table.  This function opens the input file, reads the symbol,
 * then closes the file.
 *
 * If the requested symbol does not exist in the output file, this
 * function returns NULL.
 */
SymTabEntry
PST_ReadSymbolCopyFromIn (SymbolTable table, Key key)
{
  SymbolTable in_table;
  SymTabEntry result = NULL;
  SearchOrder orig_order;

  in_table = PST_Open (PST_GetFileInName (table, key.file), NULL,
		       STF_READ_ONLY);

  P_CopySearchOrder (orig_order, table->search_order);

  /* We're reading from the input table (in_table), so set the search
   * order in the original table ot skip SO_IN to prevent a loop. */
  P_SetSymbolTableSearchOrder (table, SO_MEM, SO_OUT, SO_MEM);

  result = PST_CopySymTabEntryToTableScope (table, Invalid_Key, in_table,
					    PST_GetSymTabEntry (in_table, key),
					    FALSE);

  P_CopySearchOrder (table->search_order, orig_order);

  PST_Close (in_table);

  return (result);
}

/*! \brief Reads a copy of a symbol table entry from the output file on disk.
 *
 * \param table
 *  the symbol table.
 * \param key
 *  the key for the table entry to read.
 *
 * \return
 *  The newly read SymTabEntry, or NULL if it doesn't exist.
 *
 * Reads a copy of a symbol table entry from the output file on disk
 * and stores it in the table.  This function opens the output file,
 * reads the symbol, then closes the file.
 *
 * If the output file does not exist, or if the requested symbol does
 * not exist in the output file, this function returns NULL.
 */
SymTabEntry
PST_ReadSymbolCopyFromOut (SymbolTable table, Key key)
{
  SymbolTable in_table;
  SymTabEntry result = NULL;
  char *out_name;
  SearchOrder orig_order;

  if (PST_GetFileOutFileStatus (table, key.file) == FS_NOT_AVAIL || \
      !(out_name = PST_GetFileOutName (table, key.file)))
    goto done;

  /* Read the output file into a temporary table. */
  in_table = PST_Open (out_name, NULL, STF_READ_ONLY);

  P_CopySearchOrder (orig_order, table->search_order);

  /* We're reading from the output table (in_table), so set the search
   * order in the original table ot skip SO_OUT to prevent a loop. */
  P_SetSymbolTableSearchOrder (table, SO_MEM, SO_IN, SO_MEM);

  result = PST_CopySymTabEntryToTableScope (table, Invalid_Key, in_table,
					    PST_GetSymTabEntry (in_table, key),
					    FALSE);

  P_CopySearchOrder (table->search_order, orig_order);

  PST_Close (in_table);

 done:
  return (result);
}
