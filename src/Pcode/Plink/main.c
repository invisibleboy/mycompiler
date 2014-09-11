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
 * \brief Generates the interprocedural symbol table.
 *
 * \author Robert Kidd, Hong-Seok Kim, and Wen-mei Hwu
 *
 * This module emulates ld to resolve symbols between Pcode files and
 * generate the interprocedural symbol table.
 *
 * This module reads the symbol table from one or more source files
 * (referred to as the Source Tables) and creates an interprocedural
 * symbol table (referred to as the IP Table).
 *
 * This module links the Pcode in three stages.
 * -Copy symbols (Type Merging and Symbol Resolution)
 * -Update keys
 * -Function call validation
 *
 * During copying, each Source Table is read and added to the IP Table.
 * The Source Table itself is hung from its IPSymTabEnt's ext field in
 * IP Table.  These Source Tables are used to map keys in the input files
 * to keys in the output files.
 *
 * Details
 *
 * -Copying (and Type Merging or Symbol Resolution)
 *  Type Merging and Symbol Resolution use a similar mechanism.  Both
 *  occur as the Pcode is copied into the IP Table.
 *
 *  The symbol table for each source file (referred to as Source
 *  Table) is read and the Pcode from the file is copied to the IP
 *  Table Each SymTabEntry in Source Table may be tagged with some of
 *  the following fields.
 *
 *  If the symbol's key in IP Table is known, it will be held in new_key.
 *
 *  If the symbol should be resolved to a target symbol in another source file:
 *   link_file will hold the source file's key in IP Table.
 *   link_key  will hold the target symbol's key in the target symbol's
 *             Source Table.  
 *
 *  To find the target symbol, use link_file to get an IPSymTabEnt from 
 *  IP Table.  The ext field of this IPSymTabEnt will be a Source Table.
 *  Look link_key up in this Source Table to get the target symbol.
 *
 * Depending on the Pcode structure being copied, three things can happen.
 *
 *  -Type
 *   Before copying a type, IP Table is searched for a previous definition
 *   of the type.  The global scope (a.out.stl) and the global scope
 *   of each known source file is searched.
 *   -If the type is found in the global scope, the type's Source Table
 *    entry's new_key field is set to the key from IP Table.
 *   -If the type is found in another source file's global scope, the
 *    type is moved from that source file to the global scope.  Both
 *    the type in the other source file and the type being copied get
 *    new_key set to the type's key in IP Table.
 *
 *  -Variable or Function
 *   -If the variable or function is defined, it is copied to IP Table
 *    and new_key is set.
 *   -If the variable or function is not defined, it is resolved using
 *    ld's methods.  The variable is not copied, but its entry in
 *    Source Table is tagged with link_key and link_file to find
 *    its definition.  
 *    
 * After copying, all Pcode in IP Table still has keys referring to the
 * original Source Tables.  We use the Source Tables to map these keys
 * to new ones in IP Table.
 *
 * -Key updating
 *  We walk through the Pcode in IP Table.  Each time we see a key reference,
 *  we look that reference up in the appropriate Source Table.  If new_key
 *  is set, we change the key to it.  If link_file and link_key are set, we
 *  look at that key in its Source Table to find new_key. 
 *
 *  When the keys are updated, symbols that have been disconnected from
 *  their table entries (such as statements, local variables, labels) are
 *  reconnected to their table entries.
 *
 *  After key updating, ip_table is an ordinary, valid symbol table and
 *  can be processed with no special consideration or key mapping.
 *
 * -Function call validation
 *  When processing K&R C or when linking against libraries, it is possible
 *  that function calls are made with an incorrect number of arguments.
 *  We will find these call sites and validate the call so the proper
 *  number of arguments are specified.
 *
 *  If the validation routine need to repair a function call, the enclosing
 *  function will be flattened.
 *
 * \note This is a poor module to use as a base for writing new Pcode
 * modules.  It accesses library structures at a lower level than the
 * typical module will require.  */
/*****************************************************************************/

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <library/i_list.h>
#include <Pcode/pcode.h>
#include <Pcode/io_util.h>
#include <Pcode/parms.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/struct_symtab.h>
#include <Pcode/perror.h>
#include <Pcode/io_util.h>
#include <Pcode/read.h>
#include <Pcode/read_symtab.h>
#include <Pcode/write_symtab.h>
#include <Pcode/extension.h>
#include <Pcode/query.h>
#include <Pcode/util.h>
#include <Pcode/flatten.h>
#include "main.h"
#include "merge.h"
#include "update.h"
#include "copy.h"
#include "find.h"
#include "data.h"
#include "signature.h"
#include "validate.h"

const char *helptext = \
"Usage: Plink [-o output] [-pPuv] [-e ext] pcode-files|-l archive-dir...\n"
" -o output          The output filename.  Defaults to a.out.st\n"
" pcode-files        The main Pcode source files for the program.\n"
" -l archive-dir     A directory containing Pcode extracted from a library\n"
"                    archive.\n"
" -e ext             The output extension for processed Pcode files.  If not\n"
"                    specified, the input extension is used (files are \n"
"                    modified in place).\n"
" --print, -p        Print the list of necessary Pcode files when finished.\n"
" --print-only, -P   Print the list of necessary Pcode files only (do not\n"
"                    link).\n"
" -i, -r             Perform a partial link.  Resolves symbols between a\n"
"                    group of objects that do not define main().\n"
" -u                 Print a list of undefined symbols when finished.\n"
" -v                 Verbose mode.\n"
"\n"
" Debugging options:\n"
" --no-merge-structs Do not pointers to different structs with the same\n"
"                    name into a multi type union.\n"
" --save-sigs        Write the type signatures to the output files.\n"
"\n"
" Note: It is assumed that all Pcode files have the same file extension.\n"
"       When processing a library dir, all files with file extensions\n"
"       matching the extension of pcode-files will be processed.\n";

const char *default_name = "a.out.stl";
const char *library_index = "__index";
char *out_ext;

static void print_help (int status);
static void add_file (SymbolTable ip_table, char *filename);
static void add_library (SymbolTable ip_table, char *library_dir);
static int add_source (SymbolTable ip_table, char *source_name, char *in_name,
		       char *out_name);
static char *output_name (SymbolTable ip_table, SymbolTable source_table);
static void close_file_tables (SymbolTable ip_table);
static void promote_undefined_externs (SymbolTable ip_table, bool print);
static void annotate_entries (SymbolTable ip_table, int source_file_key);
static void rename_structs (SymbolTable ip_table);

bool partial_link = FALSE;
bool print_list = FALSE;
bool print_list_only = FALSE;
bool print_undefined = FALSE;
bool verbose = FALSE;
bool merge_structs = TRUE;
int global_header_key = 0;
Key global_scope_key;

/*! \brief Prints the help text and exits.
 *
 * \param status
 *  the exit status.
 */
static void
print_help (int status)
{
  printf ("%s\n", helptext);
  exit (status);
}

/*! \brief The main function for Plink
 *
 * \param argc
 *  the number of arguments
 * \param argv
 *  the arguments
 * \param envp
 *  the environment variables.
 */
int
main (int argc, char **argv, char **envp)
{
  SymbolTable ip_table = P_NewSymbolTable ();
  int i;
  Parm_Macro_List *external_list = NULL;
  List input_files = NULL;
  Plink_InputFile input_file;

  /* When the linked files are written, they will be written to their
   * input filenames.  Make sure we can do this and quiet any warnings. */
  allow_input_update_in_place = TRUE;
  warn_on_update_in_place = FALSE;

  /* Set up parameters. */
  external_list = L_create_external_macro_list (argv, envp);
  P_parm_file = L_get_std_parm_name (argv, envp, "STD_PARMS_FILE",
				     "./STD_PARMS");
  L_load_parameters (P_parm_file, external_list, "(Pcode",
		     P_read_parm_Pcode);

  P_init_io ();
  P_init_handlers (argv[0], external_list);

  /* Initialize the Pcode flattening library. */
  PF_Init (argv[0], external_list);

  /* Set up the callbacks to manage the user data for each SymTabEntry. */
  P_ExtSetupM (ES_IPSYMTABENT, (AllocHandler)Plink_AllocIPSymTabEntExt,
	       (FreeHandler)Plink_FreeIPSymTabEntExt);
  P_ExtSetupM (ES_SYMTABENTRY, (AllocHandler)Plink_AllocSymTabEntryExt,
	       (FreeHandler)Plink_FreeSymTabEntryExt);
  P_ExtSetupM (ES_TYPE, (AllocHandler)Plink_AllocTypeDclExt,
	       (FreeHandler)Plink_FreeTypeDclExt);
  P_ExtRegisterCopyM (ES_TYPE, (CopyHandler)Plink_CopyTypeDclExt);
  P_ExtSetupM (ES_STRUCT, (AllocHandler)Plink_AllocStructDclExt,
	       (FreeHandler)Plink_FreeStructDclExt);
  P_ExtRegisterCopyM (ES_STRUCT, (CopyHandler)Plink_CopyStructDclExt);
  P_ExtSetupM (ES_UNION, (AllocHandler)Plink_AllocUnionDclExt,
	       (FreeHandler)Plink_FreeUnionDclExt);
  P_ExtRegisterCopyM (ES_UNION, (CopyHandler)Plink_CopyUnionDclExt);
  P_ExtSetupM (ES_FUNC, (AllocHandler)Plink_AllocFuncDclExt,
	       (FreeHandler)Plink_FreeFuncDclExt);
  P_ExtRegisterCopyM (ES_FUNC, (CopyHandler)Plink_CopyFuncDclExt);

  /* Set the default name for the interprocedural symbol table. */
  P_SetSymbolTableIPTableName (ip_table, strdup (default_name));
  P_SetSymbolTableOutName (ip_table, strdup (default_name));

  /* We will read only through source tables stored in ip_table and
   * write only through ip_table.  Mark ip_table's input file FS_NOT_AVAIL
   * so PST_Read routines don't try to read it. */
  P_SetSymbolTableInFileStatus (ip_table, FS_NOT_AVAIL);

  /* Set up the implicit global header.  This contains the definitions
   * for fundamental types used in each file (int, long, etc). */
  global_header_key = PST_AddFile (ip_table, "GLOBAL SCOPE", FT_HEADER);
  PST_SetFileFlags (ip_table, global_header_key, IPSTEF_EMBEDDED);

  /* Set up the global header's scope. */
  global_scope_key.file = global_header_key;
  global_scope_key.sym = 0;
  global_scope_key = PST_AddNewScope (ip_table, global_scope_key);

  /* Process the arguments */
  if (argc < 2 || strcmp (argv[1], "-h") == 0 || \
      strcmp (argv[1], "-help") == 0 || strcmp (argv[1], "--help") == 0)
    {
      print_help (EXIT_SUCCESS);
    }
  
  /* Handle the command line arguments. */
  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-o") == 0)
	{
	  if (i < argc - 1)
	    {
	      /* Free the default name. */
	      if (P_GetSymbolTableIPTableName (ip_table))
		free (P_GetSymbolTableIPTableName (ip_table));
	      P_SetSymbolTableIPTableName (ip_table, strdup (argv[i + 1]));
	      if (P_GetSymbolTableOutName (ip_table))
		free (P_GetSymbolTableOutName (ip_table));
	      P_SetSymbolTableOutName (ip_table, strdup (argv[i + 1]));
		  
	      i++;
	    }
	  else
	    {
	      fprintf (stderr, "-o requires output argument.\n");
	      print_help (EXIT_FAILURE);
	    }
	}
      else if ((strcmp (argv[i], "--print") == 0) || \
	       (strcmp (argv[i], "-p") == 0))
	{
	  print_list = TRUE;
	}
      else if ((strcmp (argv[i], "--print-only") == 0) || \
	       (strcmp (argv[i], "-P") == 0))
	{
	  print_list = TRUE;
	  print_list_only = TRUE;
	}
      else if (strcmp (argv[i], "-i") == 0 || strcmp (argv[i], "-r") == 0)
	{
	  partial_link = TRUE;
	}
      else if (strcmp (argv[i], "-u") == 0)
	{
	  print_undefined = TRUE;
	}
      else if (strcmp (argv[i], "-v") == 0)
	{
	  verbose = TRUE;
	}
      else if (strcmp (argv[i], "-e") == 0)
	{
	  /* Verify that -e has an argument. */
	  if (i < argc - 1)
	    {
	      P_SetSymbolTableOutExt (ip_table, argv[i + 1]);
	      out_ext = strdup (argv[i + 1]);

#if 0
	      /* If we're using the default output name, change its extension
	       * to match the specified one. */
	      if (strcmp (P_GetSymbolTableOutName (ip_table),
			  default_name) == 0)
		{
		  char *ip_name;

		  if ((ip_name = malloc (strlen (ip_table->ip_table_name) + \
					 strlen (ip_table->out_ext) + 1)))
		    {
		      strcpy (ip_name, ip_table->ip_table_name);

		      /* Write a null after the final . in the IP table name.
		       */
		      strrchr (ip_name, '.')[1] = '\0';

		      strcat (ip_name, ip_table->out_ext);

		      if (P_GetSymbolTableIPTableName (ip_table))
			free (P_GetSymbolTableIPTableName (ip_table));

		      P_SetSymbolTableIPTableName (ip_table, ip_name);
		    }
		}
#endif

	      i++;
	    }
	  else
	    {
	      fprintf (stderr, "-e requires ext argument.\n");
	      print_help (EXIT_FAILURE);
	    }
	}
      else if (strcmp (argv[i], "-l") == 0)
	{
	  /* Verify that -l has an argument. */
	  if (i < argc - 1)
	    {
	      input_files = \
		List_insert_last \
		  (input_files,
		   Plink_AllocInputFileWithTypeName (PL_INPUT_LIBRARY,
						     argv[i + 1]));
	      i++;
	    }
	  else
	    {
	      fprintf (stderr, "-l requires archive-dir argument.\n");
	      print_help (EXIT_FAILURE);
	    }
	}
      else if (strcmp (argv[i], "--no-merge-structs") == 0)
	{
	  merge_structs = FALSE;
	}
      else if (strcmp (argv[i], "--save-sigs") == 0)
	{
	  /* Register save handlers for types, structs, and unions. */
	  P_ExtRegisterWriteM (ES_TYPE, (WriteHandler)Plink_WriteTypeDclExt,
			       "PLNK");
	  P_ExtRegisterWriteM (ES_STRUCT,
			       (WriteHandler)Plink_WriteStructDclExt, "PLNK");
	  P_ExtRegisterWriteM (ES_UNION, (WriteHandler)Plink_WriteUnionDclExt,
			       "PLNK");
	}
      else
	{
	  input_files = \
	    List_insert_last \
	      (input_files,
	       Plink_AllocInputFileWithTypeName (PL_INPUT_OBJECT, argv[i]));
	}
    }

  /* Add 'main' as the first required extern.  This will either be satisfied
   * by one of the object files or cause us to link against the first
   * library defining main.
   * We only add main if we're doing a complete link (not partial). */
  if (!partial_link)
    Plink_FindExtern ("main");

  List_start (input_files);
  while ((input_file = (Plink_InputFile)List_next (input_files)))
    {
      switch (input_file->type)
	{
	case PL_INPUT_OBJECT:
	  if (verbose)
	    fprintf (stderr, "Linking file %s\n", input_file->name);
	  
	  /* A Pcode file.  Merge types and resolve extern references. */
	  add_file (ip_table, input_file->name);
	  break;
	case PL_INPUT_LIBRARY:
	  if (verbose)
	    fprintf (stderr, "Linking library %s\n", input_file->name);
	
	  add_library (ip_table, input_file->name);
	  break;
	default:
	  break;
	}
    }
  input_files = P_RemoveList (input_files,
			      (void *(*)(void *))Plink_FreeInputFile);

  /* Any outstanding undefined externs will just have to be resolved at
   * link (ld) time.  Push them to the global scope. */
  promote_undefined_externs (ip_table, print_undefined);

  if (print_list)
    {
      for (i = 2; i <= P_GetSymbolTableNumFiles (ip_table); i++)
	printf ("%s\n", PST_GetFileOutName (ip_table, i));

      if (print_list_only)
	{
	  close_file_tables (ip_table);
	  ip_table = P_RemoveSymbolTable (ip_table);
	  goto cleanup;
	}
    }

  Plink_LinkLocalEntries (ip_table);

  /* Any struct which has an incomplete and a single complete definition
   * now has a multi type union with only a single field.  We can
   * remove the union for these types. */
  Plink_CompressMultiTypes (ip_table);

  /* Update the keys in all Pcode. */
  Plink_UpdateKeys (ip_table);
  close_file_tables (ip_table);

  /* Anonymous structs and unions have a long, nasty name.  The name isn't
   * important at this point, so we can rename them. */
  rename_structs (ip_table);

  /* Fix references to struct pointers that have been merged into unions. */
  Plink_ValidateTypes (ip_table);

  /* Validate function calls. */
  Plink_ValidateCalls (ip_table);

  /* Set the linked flag on the symbol table. */
  P_SetSymbolTableFlags (ip_table, STF_LINKED);

  /* Write the source files to disk. */
  PST_WriteFile (ip_table, 0);

  for (i = 1; i <= ip_table->num_files; i++)
    {
      if (ip_table->ip_table[i]->file_type == FT_SOURCE)
	PST_WriteFile (ip_table, i);
    }

  ip_table = PST_RemoveSymbolTable (ip_table);

 cleanup:
  if (out_ext)
    free (out_ext);

  Plink_CleanupExterns ();

  exit (EXIT_SUCCESS);
}

/*! \brief Adds Pcode from an object file (Pcode file) the IP table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param filename
 *  the name of the file to process.
 *
 * This function opens an object from the command line and passes it to
 * add_file() to copy the contents to the interprocedural symbol table.
 */
static void
add_file (SymbolTable ip_table, char *filename)
{
  SymbolTable source_table;
  int source_file_key;
  char *out_name;

  source_table = PST_Open (filename, NULL, STF_READ_ONLY);

  /* The input file may have been compiled in a different directory than the
   * one in which the linker is being run.  Update the source file's name
   * to the one passed in from the command line to preserve its path. */
  free (PST_GetFileInName (source_table, 1));
  PST_SetFileInName (source_table, 1, strdup (filename));

  /* Add the source file to the symbol table. */
  out_name = output_name (ip_table, source_table);
  source_file_key = add_source (ip_table,
				PST_GetFileSourceName (source_table, 1),
				PST_GetFileInName (source_table, 1), out_name);
  free (out_name);

  /* Hang the source table from its IPSymTabEnt in ip_table. */
  Plink_SetSourceTable (ip_table, source_file_key, source_table);

  /* Make sure types are defined before they are referenced. */
  PST_OrderTypeUses (source_table);

  /* Annotate each SymTabEntry with its source file key. */
  annotate_entries (ip_table, source_file_key);

  /* Build signatures for all types. */
  Plink_BuildSignatures (ip_table, source_file_key);
  Plink_CopySymbols (ip_table, source_file_key);

  PST_ResetOrder (source_table);

  return;
}

/*! \brief Links a library into the IP Table.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param library_dir
 *  the path to a directory containing Pcode extracted from a library
 *  archive.
 *
 * Links symbols from the library into the interprocedural symbol table
 * using ld's rules.
 */
static void
add_library (SymbolTable ip_table, char *library_dir)
{
  char buf[8192], path_buf[MAXPATHLEN];
  char *symbol, *file;
  FILE *index_fh;
  List sym_to_file_map = NULL;
  Plink_EIData ex;
  char dir_join_format[6]; /* %s/%s\0 at most */

  /* Determine if we need to add a slash to the library directory. */
  if (library_dir[strlen (library_dir)] == '/')
    strncpy (dir_join_format, "%s/%s", 6);
  else
    strncpy (dir_join_format, "%s%s", 6);

  snprintf (path_buf, MAXPATHLEN, dir_join_format, library_dir, library_index);

  /* The index file has one line for each symbol (externally visible vars
   * and functions) in each object file in the library.  The format is
   * <symbol><tab><file>
   *
   * The file name is relative to the library directory, so we have
   * to prepend the path to the library directory.
   */
  index_fh = P_file_open_r (path_buf);

  while (fgets (buf, 8192, index_fh))
    {
      /* Remove the trailing newline. */
      strrchr (buf, '\n')[0] = '\0';
      symbol = buf;

      /* Replace the \t separator with null so object and file are two
       * separate strings in buf. */
      file = strchr (buf, '\t');
      file[0] = '\0';
      file++;
      
      snprintf (path_buf, MAXPATHLEN, dir_join_format, library_dir, file);

      /* Add a mapping between the symbol and the file. */
      sym_to_file_map = Plink_AddMapping (sym_to_file_map, strdup (symbol),
					  strdup (path_buf));
    }

  P_file_close (index_fh);

  ex = Plink_GetFirstUndefinedExtern (ip_table);
  while (ex)
    {
      file = NULL;
      if (Plink_FindMapping (sym_to_file_map, &Plink_GetEIDataName (ex),
			     &file))
	{
	  if (verbose)
	    fprintf (stderr, "Linking file %s\n", file);

	  add_file (ip_table, file);
	  sym_to_file_map = Plink_DelMapping (sym_to_file_map, NULL, file);

	  /* Reset ex so we try to resolve symbols needed by the library
	   * object we just added. */
	  ex = Plink_GetFirstUndefinedExtern (ip_table);
	}
      else
	{
	  ex = Plink_GetNextUndefinedExtern (ip_table);
	}
    }

  return;
}

/*! \brief Adds a source file to the IP Table and initializes it.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_name
 *  the filename of the source file's original source code.
 * \param in_name
 *  the input name of the source file.
 * \param out_name
 *  the output name of the source file.
 *
 * \return
 *  The key for the new source file.
 */
static int
add_source (SymbolTable ip_table, char *source_name, char *in_name,
	    char *out_name)
{
  int source_file_key = 0;

  source_file_key = PST_AddFile (ip_table, source_name, FT_SOURCE);

  /* We will read only through source tables stored in ip_table and write
   * only through ip_table.  PST_GetSymTabEntry should not try to read
   * input files through ip_table, so set the input file status to
   * FS_NOT_AVAIL. */
  PST_SetFileInFileStatus (ip_table, source_file_key, FS_NOT_AVAIL);
  
  if (in_name)
    PST_SetFileInName (ip_table, source_file_key, strdup (in_name));
  if (out_name)
    PST_SetFileOutName (ip_table, source_file_key, strdup (out_name));

  return (source_file_key);
}

/*! \brief Returns the output filename for a source file.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_table
 *  the source file's symbol table.
 *
 * \return
 *  The output filename for the source file.
 *
 * \note The caller is responsible for freeing the string returned by
 *       this function.
 */
static char *
output_name (SymbolTable ip_table, SymbolTable source_table)
{
  char *result;
  int base_len, len;

  /* If the -e option was specified, change the extension on the source
   * file's input name. */
  if (out_ext)
    {
      base_len = \
	(int)((long)(strrchr (PST_GetFileInName (source_table, 1), '.') - \
		     PST_GetFileInName (source_table, 1)));
      len = base_len + strlen (out_ext) + 2;

      if (!(result = malloc (len)))
	P_punt ("main.c:output_name:%d Could not allocate output_name",
		__LINE__ - 1);

      result[0] = '\0';
      strncat (result, PST_GetFileInName (source_table, 1), base_len);
      strcat (result, ".");
      strcat (result, out_ext);
    }
  else
    {
      result = strdup (PST_GetFileInName (source_table, 1));
    }

  return (result);
}

/*! \brief Closes all open file symbol tables.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * Closes all open file symbol tables hanging from \a ip_table.
 */
static void
close_file_tables (SymbolTable ip_table)
{
  SymbolTable source_table;
  int i;

  /* File 1 is the global scope, so there is no file table for it. */
  for (i = 2; i <= P_GetSymbolTableNumFiles (ip_table); i++)
    {
      source_table = Plink_GetSourceTable (ip_table, i);

      PST_Close (source_table);
    }

  return;
}

/*! \brief Promotes undefined externs to the global scope.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param print
 *  If TRUE, this function prints the name of each undefined symbol.
 *
 * Promotes all undefined extersn to the global scope.
 */
static void
promote_undefined_externs (SymbolTable ip_table, bool print)
{
  Plink_EIData ex;
  Key ip_key;

  for (ex = Plink_GetFirstUndefinedExtern (ip_table); ex;
       ex = Plink_GetNextUndefinedExtern (ip_table))
    {
      if (ex->name && strcmp (ex->name, "main") == 0 && \
	  !P_ValidKey (ex->def.key))
	P_punt ("main() not defined");

      ip_key = Plink_GetLinkKey (ip_table, ex->def.file, ex->def.key);
      ip_key = Plink_MoveToGlobalScope (ip_table, ip_key);

      if (print)
	printf ("%s\n", Plink_GetEIDataName (ex));
    }

  return;
}

/*! \brief Annotates each SymTabEntry in a Source File with its source file
 *         key.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 * \param source_file_key
 *  the key of the source file to process.
 *
 * This function annotates each SymTabEntry in \a source_file_key with
 * its source file key.  If the SymTabEntry is a type, struct, or union
 * that references another type, it adds the SymTabEntry's key to the
 * referenced SymTabEntry's users list.
 *
 * This function now turns on the weak qualifier for all functions
 * in __impact_lib.c.  This is a little hack that allows __impact_lib.c's
 * functions to disappear if a program defines a symbol with the same
 * name.
 */
static void
annotate_entries (SymbolTable ip_table, int source_file_key)
{
  SymbolTable src_table;
  SymTabEntry entry, refd_entry;
  Key k, refd_key;
  bool impact_lib_hack = FALSE;

  src_table = Plink_GetSourceTable (ip_table, source_file_key);

  if (P_NameCheck (PST_GetFileSourceName (src_table, 1), "__impact_lib"))
    impact_lib_hack = TRUE;
  
  for (k = PST_GetFileEntryByType (src_table, 1, ET_ANY); P_ValidKey (k);
       k = PST_GetFileEntryByTypeNext (src_table, k, ET_ANY))
    {
      entry = PST_GetSymTabEntry (src_table, k);
      Plink_SetSymTabEntrySrcFile (entry, source_file_key);

      switch (P_GetSymTabEntryType (entry))
	{
	case ET_TYPE_GLOBAL:
	  {
	    TypeDcl t = P_GetSymTabEntryTypeDcl (entry);

	    refd_key = P_GetTypeDclType (t);
	    if (P_ValidKey (refd_key))
	      {
		refd_entry = PST_GetSymTabEntry (src_table, refd_key);

		Plink_AppendSymTabEntryUsers (refd_entry, source_file_key, k);
	      }

	    if (P_GetTypeDclBasicType (t) == BT_FUNC)
	      {
		Param p;

		for (p = P_GetTypeDclParam (t); p; p = P_GetParamNext (p))
		  {
		    refd_key = P_GetParamKey (p);
		    if (P_ValidKey (refd_key))
		      {
			refd_entry = PST_GetSymTabEntry (src_table, refd_key);

			Plink_AppendSymTabEntryUsers (refd_entry,
						      source_file_key, k);
		      }
		  }
	      }
	  }
	  break;
	case ET_STRUCT:
	case ET_UNION:
	  {
	    Field f;

	    if (P_GetSymTabEntryType (entry) == ET_STRUCT)
	      f = P_GetStructDclFields (P_GetSymTabEntryStructDcl (entry));
	    else
	      f = P_GetUnionDclFields (P_GetSymTabEntryUnionDcl (entry));

	    while (f)
	      {
		refd_key = P_GetFieldType (f);
		if (P_ValidKey (refd_key))
		  {
		    refd_entry = PST_GetSymTabEntry (src_table, refd_key);

		    Plink_AppendSymTabEntryUsers (refd_entry,
						  source_file_key, k);
		  }

		f = P_GetFieldNext (f);
	      }
	  }
	  break;
	case ET_FUNC:
	  if (impact_lib_hack)
	    P_SetFuncDclQualifier (P_GetSymTabEntryFuncDcl (entry), VQ_WEAK);
	  break;
	case ET_VAR_GLOBAL:
	  if (impact_lib_hack)
	    P_SetVarDclQualifier (P_GetSymTabEntryVarDcl (entry), VQ_WEAK);
	  break;
	default:
	  break;
	}
    }

  /* 03/05/04 REK If we're processing C++, we might have some structs that
   *              don't have corresponding BT_STRUCT TypeDcls.  At the moment,
   *              I'm not sure what to do with them, so I'll tag them and
   *              print a warning. */
  for (k = PST_GetFileEntryByType (src_table, 1, ET_STRUCT | ET_UNION);
       P_ValidKey (k);
       k = PST_GetFileEntryByTypeNext (src_table, k, ET_STRUCT | ET_UNION))
    {
      entry = PST_GetSymTabEntry (src_table, k);

      if (Plink_GetSymTabEntryUsers (entry) == NULL)
	{
	  if (verbose)
	    P_warn ("%s is a %s with no corresponding %s TypeDcl.  C++ is "
		    "strange...", P_GetSymTabEntryName (entry),
		    P_GetSymTabEntryType (entry) == ET_STRUCT ?
		      "struct" : "union",
		    P_GetSymTabEntryType (entry) == ET_STRUCT ?
		      "BT_STRUCT" : "BT_UNION");

	  Plink_SetSymTabEntryFlags (entry, PLF_LONELY_ST);
	}
    }

  return;
}

/*! \brief Renames anonymous structs and unions.
 *
 * \param ip_table
 *  the interprocedural symbol table.
 *
 * Unnamed structs and unions are given a long, ugly name by impact-edgcpfe.
 * This name encodes the struct's field types so they can be merged.  The
 * name is no longer needed, so they can be given shorter names now.
 */
static void
rename_structs (SymbolTable ip_table)
{
  Key k;
  SymTabEntry e;
  char new_name[P_MAX_IDENTIFIER_LEN];

  for (k = PST_GetTableEntryByType (ip_table,
				    (ET_STRUCT | ET_UNION | ET_TYPE));
       P_ValidKey (k);
       k = PST_GetTableEntryByTypeNext (ip_table, k,
					(ET_STRUCT | ET_UNION | ET_TYPE)))
    {
      e = PST_GetSymTabEntry (ip_table, k);

      switch (P_GetSymTabEntryType (e))
	{
	case ET_STRUCT:
	  {
	    StructDcl s = P_GetSymTabEntryStructDcl (e);

	    if (P_GetStructDclQualifier (s) & SQ_UNNAMED)
	      {
		snprintf (new_name, P_MAX_IDENTIFIER_LEN, "_P_unnamed_%d_%d_",
			  k.file, k.sym);
		
		if (P_GetStructDclName (s))
		  free (P_GetStructDclName (s));
		if (P_GetSymTabEntryName (e))
		  free (P_GetSymTabEntryName (e));

		PST_SetStructDclName (ip_table, s, strdup (new_name));
	      }
	    else if (P_GetStructDclName (s))
	      {
		snprintf (new_name, P_MAX_IDENTIFIER_LEN,
			  "_P_renamed_%s_%d_%d_", P_GetStructDclName (s),
			  k.file, k.sym);

		free (P_GetStructDclName (s));
		free (P_GetSymTabEntryName (e));
		PST_SetStructDclName (ip_table, s, strdup (new_name));
	      }
	  }
	  break;
	case ET_UNION:
	  {
	    UnionDcl u = P_GetSymTabEntryUnionDcl (e);

	    if (P_GetUnionDclQualifier (u) & SQ_UNNAMED)
	      {
		snprintf (new_name, P_MAX_IDENTIFIER_LEN, "_P_unnamed_%d_%d_",
			  k.file, k.sym);
		
		if (P_GetUnionDclName (u))
		  free (P_GetUnionDclName (u));
		P_SetUnionDclName (u, strdup (new_name));
		
		if (P_GetSymTabEntryName (e))
		  free (P_GetSymTabEntryName (e));
		P_SetSymTabEntryName (e, strdup (new_name));
	      }
	    else if (P_GetUnionDclName (u))
	      {
		snprintf (new_name, P_MAX_IDENTIFIER_LEN,
			  "_P_renamed_%s_%d_%d_", P_GetUnionDclName (u),
			  k.file, k.sym);

		free (P_GetUnionDclName (u));
		free (P_GetSymTabEntryName (e));
		PST_SetUnionDclName (ip_table, u, strdup (new_name));
	      }
	  }
	  break;
	case ET_TYPE_GLOBAL:
	  {
	    TypeDcl t = P_GetSymTabEntryTypeDcl (e);
	    Key type = P_GetTypeDclType (t);

	    if (P_GetTypeDclQualifier (t) & TY_UNNAMED)
	      {
		if (P_GetTypeDclBasicType (t) & (BT_STRUCT | BT_UNION))
		  {
		    snprintf (new_name, P_MAX_IDENTIFIER_LEN,
			      "_P_unnamed_%d_%d_", type.file, type.sym);
		
		    if (P_GetTypeDclName (t))
		      free (P_GetTypeDclName (t));
		    P_SetTypeDclName (t, strdup (new_name));
		    
		    if (P_GetSymTabEntryName (e))
		      free (P_GetSymTabEntryName (e));
		    P_SetSymTabEntryName (e, strdup (new_name));
		  }
		else if (!(P_GetTypeDclBasicType (t) & BT_TYPEDEF))
		  {
		    P_punt ("main.c:rename_structs:%d unnamed TypeDcl must "
			    "reference struct or union", __LINE__ - 1);
		  }
	      }
	    else if (P_GetTypeDclName (t))
	      {
		if (P_GetTypeDclBasicType (t) & (BT_STRUCT | BT_UNION))
		  snprintf (new_name, P_MAX_IDENTIFIER_LEN,
			    "_P_renamed_%s_%d_%d_", P_GetTypeDclName (t),
			    type.file, type.sym);
		else
		  snprintf (new_name, P_MAX_IDENTIFIER_LEN,
			    "_P_renamed_%s_%d_%d_", P_GetTypeDclName (t),
			    k.file, k.sym);

		free (P_GetTypeDclName (t));
		free (P_GetSymTabEntryName (e));
		PST_SetTypeDclName (ip_table, t, strdup (new_name));
	      }
	  }
	  break;
	default:
	  break;
	}
    }

  return;
}
