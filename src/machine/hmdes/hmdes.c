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
/*****************************************************************************\
 *
 *  File:  hmdes.c
 *
 *  Description:
 *    Reads hmdes file and builds internal data structures.
 *
 *  Creation Date :  April, 1993
 *
 *  Authors:  John C. Gyllenhaal, Roger A. Bringmann, Wen-mei Hwu
 *
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "hmdes.h"

/* If set to 1, will print verbose messages */
int verbose = 0;

/* Info structures for structures that can be dynamically allocated */
static L_Alloc_Pool *Mparse_Item_pool = NULL;

/* Flags whether or not the resource lists should be minimized */
int minimize_res_lists = 0;

/* Flags how many fatal parse errors there are */
int fatal_parse_errors;

/* Header file table */
Header_File header_table[MAX_HEADER_FILES];
int num_header_files = 0;

int Mread_int (Mparse_Info * pinfo, int *ptr);
static int Mparse_next_line (Mparse_Info * pinfo);
int mread_proc_model (Mparse_Info * pinfo, char *name, int *ptr);
int mread_define_var (Mparse_Info * pinfo, char *name, int *ptr);

/* Die with an error message if there has been any fatal errors so far */
void
Mdie_on_fatal_errors (Mparse_Info * pinfo)
{
  /* Punt if fatal errors parsing this file */
  if (fatal_parse_errors > 0)
    {
      if (fatal_parse_errors == 1)
	{
	  Mparse_error (pinfo, "Cannot continue: 1 parse error detected.");
	}
      else
	{
	  Mparse_error (pinfo,
			"Cannot continue: %d parse errors detected.",
			fatal_parse_errors);
	}
    }
}

void
Hbuild_environment_defines (Msymbol_Table * env_tab, char **envp)
{
  int i;
  char *name_ptr, *value_ptr, *end_ptr, temp_val;

  for (i = 0; envp[i] != NULL; i++)
    {
      name_ptr = envp[i];
      value_ptr = envp[i];

      /* Scan to end of name */
      while ((*value_ptr != 0) && !isspace (*value_ptr) &&
	     (*value_ptr != '='))
	value_ptr++;

      end_ptr = value_ptr;

      /* Scan to beginning of value */
      while (isspace (*value_ptr) || (*value_ptr == '='))
	value_ptr++;

      /* Save cahr at end_ptr so it can be restored later */
      temp_val = *end_ptr;

      /* Terminate the name */
      *end_ptr = 0;

      /* Do not pass the PATH variable to the hmdes.  This has caused
       * problems with the enviroment overwriting a $define.
       * Feel free to filter out any other trouble makers :)
       */
      if (strcmp (name_ptr, "PATH") != 0)
	{
	  /* Insert define value into define table */
	  Minsert_symbol (env_tab, name_ptr, strdup (value_ptr));
	}

      /* Restore the end_ptr so envp can be used by someone else */
      *end_ptr = temp_val;
    }
}

/*
 * Scans the command line passed to it for -Dmacro_name=macro_value
 * and builds a table of these 'command line defines' 
 */
void
Hbuild_command_line_defines (Msymbol_Table * cl_tab, char **argv)
{
  int i;
  char *name_ptr, *value_ptr, *end_ptr, temp_val;

  for (i = 0; argv[i] != NULL; i++)
    {
      /* Be verbose if -verbose specified */
      if (strcmp (argv[i], "-verbose") == 0)
	{
	  verbose = 1;
	  continue;
	}

      /* Only process those that start with -D */
      if ((argv[i][0] != '-') || (argv[i][1] != 'D'))
	continue;

      name_ptr = &argv[i][2];
      value_ptr = &argv[i][2];

      /* Scan to end of name */
      while ((*value_ptr != 0) && !isspace (*value_ptr) &&
	     (*value_ptr != '='))
	value_ptr++;

      end_ptr = value_ptr;

      /* Scan to beginning of value */
      while (isspace (*value_ptr) || (*value_ptr == '='))
	value_ptr++;

      /* Save cahr at end_ptr so it can be restored later */
      temp_val = *end_ptr;

      /* Terminate the name */
      *end_ptr = 0;

      /* Insert define value into define table */
      Minsert_symbol (cl_tab, name_ptr, strdup (value_ptr));

      /* Restore the end_ptr so argv can be used by someone else */
      *end_ptr = temp_val;
    }
}

Hmdes *
create_hmdes (char *hmdes_file_name, char **argv, char **envp)
{
  Mparse_Info *pinfo;
  char buf[MITEM_SIZE];
  Hmdes *hmdes;

  /* Intialize fatal_parse_error count */
  fatal_parse_errors = 0;

  /* Initialize alloc pools used */
  Mparse_Item_pool = L_create_alloc_pool ("Mparse_Item",
					  sizeof (Mparse_Item), 1);

  /* Create and initialize hmdes structure */
  if ((hmdes = (Hmdes *) malloc (sizeof (Hmdes))) == NULL)
    H_punt ("create_hmdes: Out of memory");

  /* Initialize hmdes structure */
  Malloc_name (&hmdes->file_name, hmdes_file_name);
  hmdes->processor_model = -1;
  hmdes->max_src_operands = -1;
  hmdes->max_src_syncs = -1;
  hmdes->max_dest_operands = -1;
  hmdes->max_dest_syncs = -1;
  hmdes->max_pred_operands = -1;

  pinfo = create_mparse_info (hmdes_file_name, hmdes);

  /* Create symbol tables */
  hmdes->defines_command = Mcreate_symbol_table ("Command line defined names",
						 MSYMBOL_TABLE_SIZE);
  hmdes->defines_environ = Mcreate_symbol_table ("Environment defined names",
						 MSYMBOL_TABLE_SIZE);
  hmdes->defines_internal = Mcreate_symbol_table ("Internal defined names",
						  MSYMBOL_TABLE_SIZE);
  hmdes->reg_file = Mcreate_symbol_table ("register files",
					  MSYMBOL_TABLE_SIZE);
  hmdes->IO_sets = Mcreate_symbol_table ("IO sets", MSYMBOL_TABLE_SIZE);
  hmdes->IO_items = Mcreate_symbol_table ("IO items", MSYMBOL_TABLE_SIZE);
  hmdes->resources = Mcreate_symbol_table ("resources", MSYMBOL_TABLE_SIZE);
  hmdes->res_lists = Mcreate_symbol_table ("reservation tables",
					   MSYMBOL_TABLE_SIZE);
  hmdes->latencies = Mcreate_symbol_table ("latencies", MSYMBOL_TABLE_SIZE);
  hmdes->op_class = Mcreate_symbol_table ("operation classes",
					  MSYMBOL_TABLE_SIZE);
  hmdes->operations = Mcreate_symbol_table ("operations", MSYMBOL_TABLE_SIZE);


  if (Mget_next (pinfo, buf) == EOF)
    Mparse_error (pinfo, "Unexpected EOF");

  if (!Mmatch ("Version1", buf))
    Mparse_error (pinfo, "Version1 expected not '%s'", buf);

  /* Build table of command line parameters */
  if (argv != NULL)
    Hbuild_command_line_defines (hmdes->defines_command, argv);

  /* Build table of environment parameters */
  if (envp != NULL)
    Hbuild_environment_defines (hmdes->defines_environ, envp);

  hmdes_read_section (pinfo, "Define", hmdes_read_define);
  hmdes_read_section (pinfo, "Register_Files", hmdes_read_register_files);
  hmdes_read_section (pinfo, "IO_Sets", hmdes_read_IO_sets);
  hmdes_read_section (pinfo, "IO_Items", hmdes_read_IO_items);
  hmdes_read_section (pinfo, "Resources", hmdes_read_resources);
  hmdes_read_section (pinfo, "ResTables", hmdes_read_restables);
  hmdes_read_section (pinfo, "Latencies", hmdes_read_latencies);
  hmdes_read_section (pinfo, "Operation_Class", hmdes_read_operation_class);
  hmdes_read_section (pinfo, "Operations", hmdes_read_operations);

  if (Mget_next (pinfo, buf) != EOF)
    Mparse_error (pinfo, "EOF expected not '%s'", buf);

  /* Make error message pretty */
  strcpy (pinfo->section_name, "after Operations");
  Mdie_on_fatal_errors (pinfo);

/*
    Mprint_symbol_table (stdout, hmdes->defines_command);
    Mprint_symbol_table (stdout, hmdes->defines_environ);
    Mprint_symbol_table (stdout, hmdes->defines_internal);
    Mprint_symbol_table (stdout, hmdes->reg_file);
    Mprint_symbol_table (stdout, hmdes->IO_sets);
    Mprint_symbol_table (stdout, hmdes->IO_items);
    Mprint_symbol_table (stdout, hmdes->resources);
    Mprint_symbol_table (stdout, hmdes->res_lists);
    Mprint_symbol_table (stdout, hmdes->latencies);
    Mprint_symbol_table (stdout, hmdes->op_class);
    Mprint_symbol_table (stdout, hmdes->operations);
*/
  L_free_alloc_pool (Mparse_Item_pool);
  return (hmdes);
}

void
hmdes_read_section (Mparse_Info * pinfo, char *name, void (*func) ())
{
  char buf1[MITEM_SIZE];
  char buf2[MITEM_SIZE];
  char buf3[MITEM_SIZE];

  /* Update where we are */
  sprintf (pinfo->section_name, "before %s", name);

  if ((Mget_next (pinfo, buf1) == EOF) ||
      (Mget_next (pinfo, buf2) == EOF) || (Mget_next (pinfo, buf3) == EOF))
    {
      Mparse_error (pinfo, "Unexpected EOF");
    }

  if ((!Mmatch ("(", buf1)) ||
      (!Mmatch (name, buf2)) || (!Mmatch ("declaration", buf3)))
    {
      Mparse_error (pinfo,
		    "'( %s declaration' expected not '%s %s %s'",
		    name, buf1, buf2, buf3);
    }
  /* Update where we are */
  sprintf (pinfo->section_name, "%s", name);

  /* Read the section */
  func (pinfo, name);

  /* Update where we are */
  sprintf (pinfo->section_name, "after %s", name);
}

void
hmdes_read_define (Mparse_Info * pinfo, char *sname)
{
  Hmdes *hmdes;
  char buf[MITEM_SIZE];

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  while (!hmdes_section_end (pinfo))
    {
      if (mread_proc_model (pinfo, "processor_model",
			    &hmdes->processor_model) ||
	  mread_define_var (pinfo, "source_operands",
			    &hmdes->max_src_operands) ||
	  mread_define_var (pinfo, "src_syncs",
			    &hmdes->max_src_syncs) ||
	  mread_define_var (pinfo, "dest_operands",
			    &hmdes->max_dest_operands) ||
	  mread_define_var (pinfo, "dest_syncs",
			    &hmdes->max_dest_syncs) ||
	  mread_define_var (pinfo, "predicates", &hmdes->max_pred_operands))
	{
	  continue;
	}

      /* If declaring header file to scan, read it in here */
      else if (Mis_next (pinfo, "C_header_file"))
	{
	  /* Read header file declarer */
	  Mread_match (pinfo, "C_header_file");

	  /* Read header file location */
	  if (Mget_next (pinfo, buf) == EOF)
	    H_punt ("Unexpected EOF reading header file");

	  if (num_header_files >= (MAX_HEADER_FILES - 1))
	    H_punt ("Too many header files declared, max %i",
		    MAX_HEADER_FILES);

	  /* Get the header file's name */
	  Malloc_name (&header_table[num_header_files].name, buf);

	  /* Print what we are doing */
	  if (verbose)
	    {
	      printf ("Reading %s:\n", buf);
	    }

	  /* Read in the header file into a symbol table */
	  header_table[num_header_files].table = Hread_header_file (buf);

	  num_header_files++;
	}
      /* Unknown variable, print error message */
      else
	{
	  Mpeek_ahead (pinfo, 0, buf);
	  Mparse_error (pinfo, "Unknown define var '%s'", buf);
	}
    }

  /* Check all define variables to make sure they were defined */
  if (hmdes->processor_model < 0)
    Mparse_fatal (pinfo,
		  "'processor_model' must be specified (superscalar or VLIW)");
  if (hmdes->max_src_operands < 0)
    Mparse_fatal (pinfo, "'source_operands' must be specified");
  if (hmdes->max_src_syncs < 0)
    Mparse_fatal (pinfo, "'src_syncs' must be specified");
  if (hmdes->max_dest_operands < 0)
    Mparse_fatal (pinfo, "'dest_operands' must be specified");
  if (hmdes->max_dest_syncs < 0)
    Mparse_fatal (pinfo, "'dest_syncs' must be specified");
  if (hmdes->max_pred_operands < 0)
    Mparse_fatal (pinfo, "'predicates' must be specified");

  /* Warn if don't have enough sync operands for the dependence graph */
  if ((hmdes->max_src_syncs < 3) || (hmdes->max_dest_syncs < 3))
    Mparse_warn (pinfo,
		 "The dependence graph requires at least 3 src/dest syncs\n");

  /* Die now if there has been a fatal error */
  Mdie_on_fatal_errors (pinfo);

  return;
}

/* Peeks at the next item, returns 1 if a name, 0 otherwise */
int
Mis_name_next (Mparse_Info * pinfo)
{
  char buf[MITEM_SIZE];
  if (Mpeek_ahead (pinfo, 0, buf) == EOF)
    Mparse_error (pinfo, "Unexpected EOF");

  if (isalpha (buf[0]) || (buf[0] == '_'))
    return (1);
  else
    return (0);
}

/* Returns 1 if str matches what is next in the parse stream, 0 otherwise */
int
Mis_next (Mparse_Info * pinfo, char *str)
{
  char buf[MITEM_SIZE];
  if (Mpeek_ahead (pinfo, 0, buf) == EOF)
    Mparse_error (pinfo, "Unexpected EOF");

  return (Mmatch (buf, str));
}

/* Reads a name into buf (must be MITEM_SIZE long).
 * May be a single '-' signifying no entry
 * Punts if not a name or a '-'
 */
void
Mread_name (Mparse_Info * pinfo, char *name, char *desc)
{
  if (Mget_next (pinfo, name) == EOF)
    Mparse_error (pinfo, "Unexpected EOF");

  if (!(isalpha (name[0]) || (name[0] == '_') || Mmatch ("-", name)))
    Mparse_error (pinfo, "%s name expected not '%s'", desc, name);
}

/*
 * Read the next name and finds it in the passed symbol table.
 * If '-' is next, NULL is put in ptr.
 * Puts structure for name in ptr.  Punts on error.
 */
void
Mread_symbol (Mparse_Info * pinfo, Msymbol_Table * table, void **ptr,
	      char *desc)
{
  char name[MITEM_SIZE];

  Mread_name (pinfo, name, desc);

  /* If have '-', set ptr to NULL */
  if (Mmatch (name, "-"))
    *ptr = NULL;
  else
    {
      if ((*ptr = Mfind_symbol (table, name)) == NULL)
	Mparse_error (pinfo, "'%s' not found in %s symbol table", name,
		      table->name);
    }

}

/*
 * Read the next name and finds it in the passed symbol table.
 * If the terminator is next, it is left in parse stream and hmdes->null_set
 * is put in ptr.
 * If '-' is next, the '-' is read and hmdes->null_set is put in ptr.
 * Puts structure for name in ptr.  Punts on error.
 */
void
Mread_IO_item_symbol (Mparse_Info * pinfo, Msymbol_Table * table, void **ptr,
		      char *terminator)
{
  if (Mis_next (pinfo, terminator))
    *ptr = pinfo->hmdes->null_set;
  else if (Mis_next (pinfo, "-"))
    {
      Mread_match (pinfo, "-");
      *ptr = pinfo->hmdes->null_set;
    }
  else
    Mread_symbol (pinfo, table, ptr, "IO set or register file");
}


/* Reads the next item and punts if doesn't match str */
void
Mread_match (Mparse_Info * pinfo, char *str)
{
  char buf[MITEM_SIZE];

  if (Mget_next (pinfo, buf) == EOF)
    Mparse_error (pinfo, "Unexpected EOF.  Expecting '%s'", str);

  if (!Mmatch (str, buf))
    Mparse_error (pinfo, "Expecting '%s' not '%s'", str, buf);
}

/* For debugging, reads in and throws away a set */
int
mscan_set (Mparse_Info * pinfo, char *name)
{
  int level;
  char buf[MITEM_SIZE];

  level = 0;
  if (Mget_next (pinfo, buf) == EOF)
    Mparse_error (pinfo, "Unexpected EOF.  Expected set for %s", name);

  if (!Mmatch ("(", buf))
    Mparse_error (pinfo, "Expected set opening '(' not '%s'", buf);

  level = 1;

  while (level > 0)
    {
      if (Mget_next (pinfo, buf) == EOF)
	Mparse_error (pinfo, "Unexpected EOF in %s definition", name);

      if (Mmatch ("(", buf))
	level++;
      else if (Mmatch (")", buf))
	level--;
    }

  return 0;
}

int
mread_proc_model (Mparse_Info * pinfo, char *name, int *ptr)
{
  char model[MITEM_SIZE];

  /* If name matches, read the variable name and its value */
  if (Mis_next (pinfo, name))
    {
      /* Skip name */
      Mread_match (pinfo, name);

      /* Read processor model string */
      if (Mget_next (pinfo, model) == EOF)
	{
	  Mparse_error (pinfo,
			"Unexpected EOF. Expecting model name for '%s'",
			name);
	}

      /* Non-case sensitive string compare */
      if (Mmatch (model, "superscalar"))
	*ptr = MDES_SUPERSCALAR;
      else if (Mmatch (model, "vliw"))
	*ptr = MDES_VLIW;
      else
	Mparse_error (pinfo,
		      "Undefined processor model '%s'.  "
		      "Use superscalar or VLIW.", model);

      /* Return 1 since read in variable */
      return (1);
    }

  /* Return 0, variable not read in */
  return (0);
}

int
mread_define_var (Mparse_Info * pinfo, char *name, int *ptr)
{
  /* If name matches, read the variable name and its value */
  if (Mis_next (pinfo, name))
    {
      /* Skip name */
      Mread_match (pinfo, name);

      /* Read variable value */
      if (Mread_int (pinfo, ptr) == EOF)
	{
	  Mparse_error (pinfo,
			"Unexpected EOF. Expecting int value for '%s'", name);
	}

      /* Make sure value read is not < 0 */
      if (*ptr < 0)
	Mparse_error (pinfo, "%s value must be >= 0, not %i", name, *ptr);

      /* Return 1 since read in variable */
      return (1);
    }

  /* Return 0, variable not read in */
  return (0);
}

void
Malloc_struct (void **ptr, int size)
{
  if ((*ptr = (void *) malloc (size)) == NULL)
    H_punt ("Malloc_struct: Out of memory");
}

void
Malloc_name (char **ptr, char *name)
{
  int size;
  size = strlen (name) + 1;
  if ((*ptr = (char *) malloc (size)) == NULL)
    H_punt ("Malloc_name: Out of memory");

  strcpy (*ptr, name);
}

/*
 * Reads an integer into *ptr from the file.  
 * 
 * Returns 0 on success, EOF if at end of file.
 */
int
Mread_int (Mparse_Info * pinfo, int *ptr)
{
  char buf[MITEM_SIZE];
  char *end_ptr;

  if (Mget_next (pinfo, buf) == EOF)
    return (EOF);

  /* Make sure read in number */
  if ((!isdigit (buf[0])) && (buf[0] != '-'))
    Mparse_error (pinfo, "Int value expected, not '%s'", buf);

  /* Write int value to *ptr */
  *ptr = strtol (buf, &end_ptr, 0);

  /* Make sure number was converted properly */
  if (*end_ptr != 0)
    Mparse_error (pinfo, "Error converting '%s' to an integer", buf);

  /* Return success */
  return (0);
}

Msymbol_Table *
Mcreate_symbol_table (char *name, int size)
{
  Msymbol_Table *table;
  int name_size, table_size, hash_size;
  int i;

  /* Make sure size is a power of two */
  for (i = 0; i < 32; i++)
    {
      /* Shift left until find first 1 */
      if (((size >> i) & 1) == 1)
	{
	  /* If this is the only 1, then power of two */
	  if ((size >> i) == 1)
	    break;
	  else
	    H_punt ("Create_symbol_table: %s size (%i) must be power of 2",
		    name, size);
	}
    }
  table_size = sizeof (Msymbol_Table);
  name_size = strlen (name) + 1;
  hash_size = size * sizeof (Msymbol *);

  if (((table = (Msymbol_Table *) malloc (table_size)) == NULL) ||
      ((table->name = (char *) malloc (name_size)) == NULL) ||
      ((table->hash_table = (Msymbol **) malloc (hash_size)) == NULL))
    {
      H_punt ("Create_symbol_table: Out of memory");
    }

  /* Intialize symbol table values */
  strcpy (table->name, name);
  table->hash_mask = size - 1;
  table->head = NULL;
  table->tail = NULL;
  table->entry_count = 0;

  for (i = 0; i < size; i++)
    table->hash_table[i] = NULL;

  return (table);
}

/* 
 * Finds the data structure associated with name in the symbol table.
 * Returns a pointer to the structure or NULL if it cannot be 
 * found.
 */
void *
Mfind_symbol (Msymbol_Table * table, char *name)
{
  Msymbol *symbol;
  unsigned int hash_val;

  hash_val = Mhash_name (name, table->hash_mask);
  symbol = table->hash_table[hash_val];

  while (symbol != NULL)
    {
      /* Case sensitive */
      if (strcmp (symbol->name, name) == 0)
	return (symbol->ptr);

      symbol = symbol->next_hash;
    }
  return (NULL);
}

/*
 * Creates and insert a symbol with 'name' that points to 'ptr'.
 * 
 */
void
Minsert_unique_symbol (Mparse_Info * pinfo, Msymbol_Table * table, char *name,
		       void *ptr)
{
  /* Do not allow '-' to be added to symbol table */
  if (Mmatch ("-", name))
    Mparse_error (pinfo, "'-' is not a valid symbol name",
		  pinfo->section_name);

  /* Make sure symbol does not already exist */
  if (Mfind_symbol (table, name) != NULL)
    Mparse_error (pinfo, "redefinition of '%s'", name);

  Minsert_symbol (table, name, ptr);
}

void
Minsert_symbol (Msymbol_Table * table, char *name, void *ptr)
{
  Msymbol *symbol;
  unsigned int hash_val;
  int symbol_size, name_size;

  /* Allocate symbol and name space */
  symbol_size = sizeof (Msymbol);
  name_size = strlen (name) + 1;

  if (((symbol = (Msymbol *) malloc (symbol_size)) == NULL) ||
      ((symbol->name = (char *) malloc (name_size)) == NULL))
    {
      H_punt ("Msymbol_table: Out of memory");
    }

  /* Copy info to symbol */
  strcpy (symbol->name, name);
  symbol->ptr = ptr;

  /* Link into hash table */
  hash_val = Mhash_name (name, table->hash_mask);
  symbol->next_hash = table->hash_table[hash_val];
  table->hash_table[hash_val] = symbol;

  /* Link into linear list of symbols */
  symbol->prev_linear = table->tail;
  symbol->next_linear = NULL;
  if (table->tail == NULL)
    table->head = symbol;
  else
    table->tail->next_linear = symbol;
  table->tail = symbol;

  /* Increment table entry count */
  table->entry_count++;
}

/* 
 * Prints a list of names defined to a file
 */
void
Mprint_symbol_table (FILE * out, Msymbol_Table * table)
{
  Msymbol *symbol;

  fprintf (out, "Symbol table for '%s' has %i entries:\n", table->name,
	   table->entry_count);

  for (symbol = table->head; symbol != NULL; symbol = symbol->next_linear)
    fprintf (out, "%s\n", symbol->name);

  fprintf (out, "\n");
}

/*
 * Hashes the name and limits using mask.
 * This quick function is not a great hash function but it works
 * well enough (doing normal modulus algorithms is just too expensive).
 */
int
Mhash_name (char *name, int mask)
{
  unsigned int accum, hash_val;

  accum = 0;

  while (*name != 0)
    {
      accum <<= 1;
      accum += *name;
      name++;
    }

  hash_val = accum & mask;

  return (hash_val);
}

/*
 * Gets the int value declared in the header files for the specified name 
 * and puts it into ptr. 
 * 
 * Returns 1 if found.
 * Returns 0 if not found.
 * Returns -1 if has bad value or multiply defined (prints warnings).
 * 
 * Sets ptr value to -1 if not found.
 */
int
find_header_value (Mparse_Info * pinfo, char *name, int *ptr)
{
  Msymbol_Table *prev_def;
  int i;
  char *val_str, *end_ptr;
  char *prev_str;
  int prev_val;

  prev_def = NULL;
  prev_val = -1;
  prev_str = NULL;

  /* Search all header files for symbol */
  for (i = 0; i < num_header_files; i++)
    {
      val_str = Mfind_symbol (header_table[i].table, name);

      if (val_str == NULL)
	continue;

      /* Try to convert to an int */
      *ptr = strtol (val_str, &end_ptr, 0);

      if (*end_ptr != 0)
	{
	  Mparse_fatal (pinfo,
			"%s value '%s' from %s is not an int\n",
			name, val_str, header_table[i].table->name);
	  *ptr = -1;
	  return (-1);
	}

      if (prev_def == NULL)
	{
	  prev_def = header_table[i].table;
	  prev_val = *ptr;
	  prev_str = val_str;
	}
      else
	{
	  if (prev_val != *ptr)
	    {
	      Mparse_fatal (pinfo,
			    "Reading %s, %s defines as %s, %s defines as %s\n",
			    name, header_table[i].table->name,
			    val_str, prev_def->name, prev_str);
	      *ptr = -1;
	      return (-1);
	    }
	}
    }

  /* If found, return 1, else 0 */
  if (prev_def != NULL)
    return (1);
  else
    {
      *ptr = -1;
      return (0);
    }
}
void
hmdes_read_register_files (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  Hmdes_Reg_File *reg;
  char name[MITEM_SIZE];
  char var[MITEM_SIZE];
  char header_name[MITEM_SIZE + 20];
  int reg_id;
  int ret_val;

  /* Print out explanation of external id's */
  if (verbose)
    {
      printf
	("#The lmdes C interface uses the following optional external ids:\n");
    }

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;
  reg_id = 0;
  while (!hmdes_section_end (pinfo))
    {
      /* Read in reg file name */
      Mread_name (pinfo, name, "Register file");

      /* Allocate space for structure and name */
      Malloc_struct ((void **) &reg, sizeof (Hmdes_Reg_File));
      Malloc_name (&reg->name, name);

      /* Number the registers from 0 */
      reg->id = reg_id;

      /* Read external id for this register file */
      sprintf (header_name, "MDES_OPERAND_%s", reg->name);
      ret_val = find_header_value (pinfo, header_name, &reg->external_id);

      /* If could not get value, set to -1 */
      if (ret_val != 1)
	{
	  reg->external_id = -1;
	  if (verbose)
	    {
	      fprintf (stdout,
		       "Register file %-11s not mapped to an external id\n",
		       reg->name);
	    }

	  /* NULL must be mapped to external id */
	  if (strcmp (reg->name, "NULL") == 0)
	    Mparse_error (pinfo,
			  "NULL register file must have an external id");

	}
      else
	{
	  if (verbose)
	    {
	      fprintf (stdout,
		       "Register file %-11s mapped to %2i\n",
		       reg->name, reg->external_id);
	    }
	}

      /* Parse  'name ((capacity 192 0) (width 32))' */
      Mread_match (pinfo, "(");

      /* Read capacity */
      Mread_match (pinfo, "(");
      Mread_name (pinfo, var, "capacity");
      if (!Mmatch ("capacity", var))
	Mparse_error (pinfo, "'capacity' expected not '%s'", var);
      Mread_int (pinfo, &reg->static_regs);
      Mread_int (pinfo, &reg->rotating_regs);
      Mread_match (pinfo, ")");

      /* Read width */
      Mread_match (pinfo, "(");
      Mread_name (pinfo, var, "width");
      if (!Mmatch ("width", var))
	Mparse_error (pinfo, "'width' expected not '%s'", var);
      Mread_int (pinfo, &reg->width);
      Mread_match (pinfo, ")");

      Mread_match (pinfo, ")");

      Minsert_unique_symbol (pinfo, hmdes->reg_file, reg->name, reg);
      /* Increment register id */
      reg_id++;
    }

  /* 
   * Make sure a NULL register file is declared.  All - and default
   * IO_sets will point at this register file
   */
  if (Mfind_symbol (hmdes->reg_file, "NULL") == NULL)
    Mparse_error (pinfo, "NULL register file must be explicitly declared");

  return;
}

void
Madd_IO_node (Hmdes_IO_Set * IO_set, Hmdes_Reg_File * reg_file)
{
  Hmdes_IO_Node *IO_node;

  Malloc_struct ((void **) (&IO_node), sizeof (Hmdes_IO_Node));
  IO_node->reg_file = reg_file;
  IO_node->next = NULL;

  /* Add to end of IO_set list */
  if (IO_set->tail == NULL)
    IO_set->head = IO_node;
  else
    IO_set->tail->next = IO_node;
  IO_set->tail = IO_node;
  IO_set->size++;

}

void
hmdes_read_IO_sets (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  Msymbol *symbol;
  Hmdes_Reg_File *reg_file;
  Hmdes_IO_Set *IO_set, *c_set;
  Hmdes_IO_Node *IO_node;
  char set_name[MITEM_SIZE];
  char header_name[MITEM_SIZE + 20];
  int IO_set_id;
  int ret_val;

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  /* Intialize counter so we can set IO_sets_id's for lmdes */
  IO_set_id = 0;

  /* 
   * Add all register files as IO_Sets to the IO_Set symbol
   * table so there is a uniform interface to IO_Sets.
   * This also allows IO_Sets to be made up of IO_Sets.
   */
  for (symbol = hmdes->reg_file->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      /* Get register file */
      reg_file = (Hmdes_Reg_File *) symbol->ptr;

      /* Create set to hold register file with it's name */
      Malloc_struct ((void **) (&IO_set), sizeof (Hmdes_IO_Set));
      Malloc_name (&IO_set->name, reg_file->name);
      IO_set->id = IO_set_id;
      IO_set->external_id = reg_file->external_id;
      IO_set->head = NULL;
      IO_set->tail = NULL;
      IO_set->size = 0;

      /* Make sure IO_set_id is the same as reg_id */
      if (IO_set_id != reg_file->id)
	H_punt ("hmdes_read_IO_sets: IO_set_id (%i) != reg_id (%i)",
		IO_set_id, reg_file->id);

      /* Add the register file to IO_set */
      Madd_IO_node (IO_set, reg_file);

      /* Add IO_set to symbol table */
      Minsert_unique_symbol (pinfo, hmdes->IO_sets, IO_set->name, IO_set);

      /* Increment IO_set_id */
      IO_set_id++;
    }

  while (!hmdes_section_end (pinfo))
    {
      Mread_name (pinfo, set_name, "IO set");

      Malloc_struct ((void **) (&IO_set), sizeof (Hmdes_IO_Set));
      Malloc_name (&IO_set->name, set_name);
      IO_set->id = IO_set_id;
      IO_set->head = NULL;
      IO_set->tail = NULL;
      IO_set->size = 0;

      /* Read external id for this IO_Set */
      sprintf (header_name, "MDES_OPERAND_%s", IO_set->name);
      ret_val = find_header_value (pinfo, header_name, &IO_set->external_id);

      /* If could get value, set to -1 */
      if (ret_val != 1)
	{
	  IO_set->external_id = -1;
	  if (verbose)
	    {
	      fprintf (stdout,
		       "IO_Set        %-11s not mapped to an external id\n",
		       IO_set->name);
	    }

	}
      else
	{
	  if (verbose)
	    {
	      fprintf (stdout, "IO_Set        %-11s mapped to %2i\n",
		       IO_set->name, IO_set->external_id);
	    }
	}

      Mread_match (pinfo, "(");

      while (Mis_name_next (pinfo))
	{
	  /* Find the IO_Set that goes with this name */
	  Mread_symbol (pinfo, hmdes->IO_sets, (void **) (&c_set), "IO set");

	  /* Add all it's reg files to this IO_set */
	  for (IO_node = c_set->head; IO_node != NULL;
	       IO_node = IO_node->next)
	    {
	      Madd_IO_node (IO_set, IO_node->reg_file);
	    }
	}

      Mread_match (pinfo, ")");

      /* May not have empty IO_Sets.  This is what the NULL IO_Set is 
       * use for.
       */
      if (IO_set->size == 0)
	Mparse_error (pinfo,
		      "Empty IO_Set declaration (%s).  Use NULL.",
		      IO_set->name);

      Minsert_unique_symbol (pinfo, hmdes->IO_sets, IO_set->name, IO_set);
      /* Increment IO_set_id */
      IO_set_id++;
    }

  /* 
   * Get the null_set to use every where - and where IO_set needs
   * to default to something
   */
  hmdes->null_set = (Hmdes_IO_Set *) Mfind_symbol (hmdes->IO_sets, "NULL");

  /* Should never be NULL */
  if (hmdes->null_set == NULL)
    H_punt ("No NULL IO_set, should have been detected earlier");

  return;
}

void
hmdes_read_IO_items (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  Hmdes_IO_Item *IO_item;
  char item_name[MITEM_SIZE];
  int array_size, i;
  int item_id;

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  /* Intialize item id */
  item_id = 0;

  /* Get size of IO_item src, dest and pred array */
  array_size = sizeof (Hmdes_IO_Set *) * (hmdes->max_src_operands +
					  hmdes->max_dest_operands +
					  hmdes->max_pred_operands);

  while (!hmdes_section_end (pinfo))
    {
      /* Read name of IO_item and alloc IO_item */
      Mread_name (pinfo, item_name, "IO item");
      Malloc_struct ((void **) (&IO_item), sizeof (Hmdes_IO_Item));
      Malloc_name (&IO_item->name, item_name);

      /* Set item id */
      IO_item->id = item_id;

      /*
       * Alloc array for src, dest and pred IO_sets in one block
       * and break into pieces
       */
      Malloc_struct ((void **) (&IO_item->src), array_size);
      IO_item->dest = &IO_item->src[hmdes->max_src_operands];
      IO_item->pred = &IO_item->dest[hmdes->max_dest_operands];

      /* read '(<predicate0 ...>[dest0 ...][src0 ...])' */
      Mread_match (pinfo, "(");

      /* If '<' next, then optional predicate field is defined */
      if (Mis_next (pinfo, "<"))
	{
	  Mread_match (pinfo, "<");
	  for (i = 0; i < hmdes->max_pred_operands; i++)
	    Mread_IO_item_symbol (pinfo, hmdes->IO_sets,
				  (void **) (&IO_item->pred[i]), ">");
	  Mread_match (pinfo, ">");
	}
      /* Otherwise, set all predicates to NULL */
      else
	{
	  for (i = 0; i < hmdes->max_pred_operands; i++)
	    IO_item->pred[i] = hmdes->null_set;
	}

      /* Read in dests */
      Mread_match (pinfo, "[");
      for (i = 0; i < hmdes->max_dest_operands; i++)
	Mread_IO_item_symbol (pinfo, hmdes->IO_sets,
			      (void **) (&IO_item->dest[i]), "]");
      Mread_match (pinfo, "]");

      /* Read in srcs */
      Mread_match (pinfo, "[");
      for (i = 0; i < hmdes->max_src_operands; i++)
	Mread_IO_item_symbol (pinfo, hmdes->IO_sets,
			      (void **) (&IO_item->src[i]), "]");
      Mread_match (pinfo, "]");


      Mread_match (pinfo, ")");

      Minsert_unique_symbol (pinfo, hmdes->IO_items, IO_item->name, IO_item);

      /* Update item_id */
      item_id++;
    }
  return;
}

/*
 * Adds the subscript to the resource subscript list
 */
void
hmdes_add_res_subscript (Mparse_Info * pinfo, Hmdes_Resource * resource,
			 int subscript, int id)
{
  Hmdes_Res_Sub *sub, *new_sub;

  /* Search subscript list for duplicate, punt if found */
  for (sub = resource->head; sub != NULL; sub = sub->next)
    {
      if (sub->subscript == subscript)
	Mparse_error (pinfo, "Duplicate subscript decl %i for '%s'",
		      subscript, resource->name);
    }

  Malloc_struct ((void **) &new_sub, sizeof (Hmdes_Res_Node));

  new_sub->resource = resource;
  new_sub->id = id;
  new_sub->subscript = subscript;
  new_sub->next = NULL;
  if (resource->tail == NULL)
    resource->head = new_sub;
  else
    resource->tail->next = new_sub;
  resource->tail = new_sub;
  resource->num_subscripts++;
}

void
hmdes_read_resources (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  Hmdes_Resource *resource;
  Hmdes_Res_Sub *slot_sub;
  int start_subscript;
  int end_subscript;
  int subscript;
  char res_name[MITEM_SIZE];
  int res_id;
  int i;

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  /* Initiailize enumeration of resources */
  res_id = 0;
  while (!hmdes_section_end (pinfo))
    {
      Mread_name (pinfo, res_name, "Resource");
      Malloc_struct ((void **) (&resource), sizeof (Hmdes_Resource));
      Malloc_name (&resource->name, res_name);
      resource->head = NULL;
      resource->tail = NULL;
      resource->num_subscripts = 0;

      /* If declaration includes subscripts, read them */
      if (Mis_next (pinfo, "["))
	{
	  Mread_match (pinfo, "[");

	  /* Make sure not empty [] */
	  if (Mis_next (pinfo, "]"))
	    Mparse_error (pinfo, "Empty [] not allowed");

	  /* Loop until ending ] to read in subscripts */
	  while (!Mis_next (pinfo, "]"))
	    {
	      /* Get lower bound on subscript */
	      Mread_int (pinfo, &start_subscript);

	      /* If followed by .., ready end_subscript */
	      if (Mis_next (pinfo, ".."))
		{
		  Mread_match (pinfo, "..");
		  Mread_int (pinfo, &end_subscript);

		  /* Do not allow negative subscripts */
		  if ((start_subscript < 0) || (end_subscript < 0))
		    {
		      Mparse_error (pinfo,
				    "%s subscripts (%i..%i) must be >= 0",
				    resource->name,
				    start_subscript, end_subscript);
		    }

		  /* Determine if should increment or decrement
		   * from start to end subscript 
		   */
		  if (start_subscript <= end_subscript)
		    {
		      /* Add all subscripts to subscript list */
		      for (subscript = start_subscript;
			   subscript <= end_subscript; subscript++)
			{
			  hmdes_add_res_subscript (pinfo, resource,
						   subscript, res_id);
			  res_id++;
			}
		    }
		  else
		    {
		      /* Add all subscripts to subscript list */
		      for (subscript = start_subscript;
			   subscript >= end_subscript; subscript--)
			{
			  hmdes_add_res_subscript (pinfo, resource,
						   subscript, res_id);
			  res_id++;
			}
		    }
		}

	      /* Otherwise, just add subscript to list */
	      else
		{
		  /* Do not allow negative indexes */
		  if (start_subscript < 0)
		    {
		      Mparse_error (pinfo,
				    "%s subscript (%i) must be >= 0",
				    resource->name, start_subscript);
		    }
		  hmdes_add_res_subscript (pinfo, resource, start_subscript,
					   res_id);
		  res_id++;

		}
	    }

	  Mread_match (pinfo, "]");
	}

      /* If no subscripts are declared, default to subscript 0 */
      else
	{
	  hmdes_add_res_subscript (pinfo, resource, 0, res_id);
	  res_id++;
	}

      Minsert_unique_symbol (pinfo, hmdes->resources, resource->name,
			     resource);

      /* Do special processing slot resource */
      if (strcmp (resource->name, "slot") == 0)
	{
	  /* Find max slot defined, needed by lmdes */
	  hmdes->max_slot = -1;
	  for (slot_sub = resource->head; slot_sub != NULL;
	       slot_sub = slot_sub->next)
	    {
	      if (slot_sub->subscript > hmdes->max_slot)
		hmdes->max_slot = slot_sub->subscript;
	    }

	  /* 
	   * Slots must have all slots from 0 to max_slot defined,
	   * This is the case if num_slots = max_slot + 1
	   */
	  if (resource->num_subscripts != (hmdes->max_slot + 1))
	    {

	      /* Find the missing slots */
	      for (i = 0; i < hmdes->max_slot; i++)
		{
		  for (slot_sub = resource->head; slot_sub != NULL;
		       slot_sub = slot_sub->next)
		    {
		      if (slot_sub->subscript == i)
			break;
		    }
		  if (slot_sub == NULL)
		    Mparse_fatal (pinfo, "Slot %i not declared.", i);
		}
	      Mparse_fatal (pinfo,
			    "All slots between 0 and max_slot(%i) "
			    "must be declared.", hmdes->max_slot);
	    }
	}
    }

  /* Write how many resources there are to the hmdes structure */
  hmdes->num_resources = res_id;

  /* Make sure they defined at least one slot */
  if (Mfind_symbol (pinfo->hmdes->resources, "slot") == NULL)
    {
      Mparse_error (pinfo, "Must define at least one 'slot'.");
    }
  return;
}

/* 
 * Adds the resource 'name' to the option list for the res_node
 */
void
Madd_res_option (Mparse_Info * pinfo, Hmdes_Res_Node * node, char *name,
		 int subscript)
{
  Hmdes_Res_Option *option;
  Hmdes_Res_Sub *sub;
  Msymbol_Table *table;

  Malloc_struct ((void **) (&option), sizeof (Hmdes_Res_Option));

  /* Get resouce this name corresponses to */
  table = pinfo->hmdes->resources;
  option->resource = (Hmdes_Resource *) Mfind_symbol (table, name);
  if (option->resource == NULL)
    Mparse_error (pinfo, "'%s' not found in %s symbol table", name,
		  table->name);

  /* Find subscript in resource subscript list */
  for (sub = option->resource->head; sub != NULL; sub = sub->next)
    {
      if (sub->subscript == subscript)
	break;
    }

  /* Detect if subscript is not defined */
  if (sub == NULL)
    Mparse_error (pinfo, "Subscript %i for '%s' has not been declared",
		  subscript, name);

  /* Point to appropriate subscript */
  option->subscript = sub;

  /* Add to end of linked list */
  option->next = NULL;
  if (node->tail == NULL)
    node->head = option;
  else
    node->tail->next = option;
  node->tail = option;

  node->num_options++;

  /* Is this a slot option */
  if (Mmatch ("slot", name))
    {
      /* Increment number of slot options */
      node->num_slots++;
    }
}
void
hmdes_read_restables (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  Hmdes_Res_List *list;
  Hmdes_Res_Node *node, *cur_node, *next_node;
  Hmdes_Res_Option *option, *cur_option, *next_option;
  Hmdes_Res_Sub *sub;
  Hmdes_Resource *slots_avail, *resource;
  Msymbol_Table *table;
  char list_name[MITEM_SIZE];
  char res_name[MITEM_SIZE];
  int list_id;
  int start_subscript;
  int end_subscript;
  int subscript;

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  /* Initialize enumeration of resource lists */
  list_id = 0;

  /* Get a pointer to the slot resource for ease of use
   * when adding default slot options
   */
  slots_avail = (Hmdes_Resource *) Mfind_symbol (pinfo->hmdes->resources,
						 "slot");
  if (slots_avail == NULL)
    {
      H_punt ("No slots declared, should have been detected earlier");
    }

  while (!hmdes_section_end (pinfo))
    {
      /* Read list name and allocate list */
      Mread_name (pinfo, list_name, "ResTable");
      Malloc_struct ((void **) (&list), sizeof (Hmdes_Res_List));
      Malloc_name (&list->name, list_name);
      list->id = list_id;
      list->head = NULL;
      list->tail = NULL;
      list->slot_options = NULL;
      list->size = 0;

      Mread_match (pinfo, "(");

      /* While there are resource usage definitions of the
       * form: (issue 0) or (window 0..1)
       */
      while (Mis_next (pinfo, "("))
	{
	  Mread_match (pinfo, "(");

	  /* Alloc space for resource usage node */
	  Malloc_struct ((void **) (&node), sizeof (Hmdes_Res_Node));
	  node->head = NULL;
	  node->tail = NULL;
	  node->num_options = 0;
	  node->num_slots = 0;
	  node->flags = 0;

	  /* Read in all the resources options */
	  while (Mis_name_next (pinfo) || (node->num_options == 0))
	    {
	      /* Get resource name */
	      Mread_name (pinfo, res_name, "Resource");

	      /* Read in subscripts for resource if any */
	      if (Mis_next (pinfo, "["))
		{
		  Mread_match (pinfo, "[");

		  /* Make sure not empty [] */
		  if (Mis_next (pinfo, "]"))
		    Mparse_error (pinfo, "Empty [] not allowed");

		  /* Loop until ending ] to read in subscripts */
		  while (!Mis_next (pinfo, "]"))
		    {
		      /* Get lower bound on subscript */
		      Mread_int (pinfo, &start_subscript);

		      /* If followed by .., ready end_subscript */
		      if (Mis_next (pinfo, ".."))
			{
			  Mread_match (pinfo, "..");
			  Mread_int (pinfo, &end_subscript);

			  /*
			   * Determine if should increment or decrement
			   * from start to end subscript
			   */
			  if (start_subscript <= end_subscript)
			    {
			      /* Add all subscripts to subscript list */
			      for (subscript = start_subscript;
				   subscript <= end_subscript; subscript++)
				{
				  Madd_res_option (pinfo, node, res_name,
						   subscript);
				}
			    }
			  else
			    {
			      /* Add all subscripts to subscript list */
			      for (subscript = start_subscript;
				   subscript >= end_subscript; subscript--)
				{
				  Madd_res_option (pinfo, node, res_name,
						   subscript);
				}
			    }
			}
		      /* Otherwise, just add subscript to list */
		      else
			{
			  Madd_res_option (pinfo, node, res_name,
					   start_subscript);
			}
		    }

		  Mread_match (pinfo, "]");

		}

	      /* If no subscripts are declared, default to all declared
	       * subscripts for that resource (in declared order).
	       */
	      else
		{
		  /* Get resouce this name corresponses to */
		  table = pinfo->hmdes->resources;
		  resource = (Hmdes_Resource *) Mfind_symbol (table,
							      res_name);
		  if (resource == NULL)
		    Mparse_error (pinfo,
				  "'%s' not found in %s symbol table",
				  res_name, table->name);

		  /* Add all subscripts in subscript list */
		  for (sub = resource->head; sub != NULL; sub = sub->next)
		    {
		      Madd_res_option (pinfo, node, res_name, sub->subscript);
		    }

		}
	    }

	  /* Get start time */
	  Mread_int (pinfo, &node->start_usage);

	  /* Usage times may not be negative */
	  if (node->start_usage < 0)
	    Mparse_error (pinfo, "May not have negative usage times (%i)",
			  node->start_usage);

	  /* If followed by ..number then that is the end
	   * time otherwise, end time is start time
	   */
	  if (Mis_next (pinfo, ".."))
	    {
	      Mread_match (pinfo, "..");
	      Mread_int (pinfo, &node->end_usage);
	      /* May not be before start_usage */
	      if (node->end_usage < node->start_usage)
		Mparse_error (pinfo, "end_usage (%i) < start_usage (%i)",
			      node->end_usage, node->start_usage);
	    }
	  else
	    {
	      node->end_usage = node->start_usage;
	    }
	  Mread_match (pinfo, ")");

	  /* Process slot options separately */
	  if (node->num_slots > 0)
	    {
	      /* Slot may take up resources in cycle 0 only */
	      if ((node->start_usage != 0) || (node->end_usage != 0))
		{
		  Mparse_error (pinfo, "Slots may used only in cycle 0");
		}

	      /* 
	       * If slot is declared in at least one option, it must be
	       * declared in all options
	       */
	      if (node->num_options != node->num_slots)
		{
		  Mparse_error (pinfo,
				"Either all options must have a slot "
				"declared or none may");
		}

	      /* 
	       * There may only be one option list with slots in it.
	       */
	      if (list->slot_options != NULL)
		{
		  Mparse_error (pinfo,
				"Only one slot may be used per resource list");
		}

	      /* Point to the slot options just declared */
	      list->slot_options = node;
	    }

	  /* 
	   * If minimizing res lists, then
	   * Add non-slots to res_list in sorted order.
	   * sort by three keys:
	   * start_usage, end_usage, and num_options
	   */
	  else if (minimize_res_lists)
	    {
	      /* Currently unsorted, come back to it */
	      node->next = NULL;
	      if (list->tail == NULL)
		list->head = node;
	      else
		list->tail->next = node;
	      list->tail = node;

	      /* Update res_list size */
	      list->size++;
	    }

	  /*
	   * Otherwise, if not mimimizing resouce lists,
	   * add to end of resource list      
	   */
	  else
	    {
	      node->next = NULL;
	      if (list->tail == NULL)
		list->head = node;
	      else
		list->tail->next = node;
	      list->tail = node;

	      /* Update res_list size */
	      list->size++;
	    }

	  /* Mark resource node if it might allocate the same resource
	   * at the same time as a prevois resource node in this 
	   * resource list.  Special RU_manager processing is needed then.
	   * This marking speeds up the RU_manager since that special
	   * processing is slow and pointless if that condition is not met.
	   * -JCG 1/12/95p 
	   */
	  /* Scan all res nodes before the current node, list->head may
	   * be NULL (for NOPs)
	   */
	  cur_node = node;
	  for (node = list->head; (node != cur_node) && (node != NULL);
	       node = next_node)
	    {
	      /* Get next node so can set to NULL if match found */
	      next_node = node->next;

	      /* If nodes do not overlap in time, there cannot be a problem */
	      if ((node->end_usage < cur_node->start_usage) ||
		  (node->start_usage > cur_node->end_usage))
		continue;

	      /* Nodes overlap in time, if they have an option in common
	       * that use the same resource, mark that they need special
	       * processing (overlap).
	       */
	      for (option = node->head; option != NULL; option = next_option)
		{
		  /* Get next option so can set to NULL if match found */
		  next_option = option->next;

		  for (cur_option = cur_node->head; cur_option != NULL;
		       cur_option = cur_option->next)
		    {
		      if (cur_option->subscript->id == option->subscript->id)
			{
			  /* Can overlap previous node.  Mark and stop
			   * search by setting next_node and next_option
			   * to termination conditions.
			   */
			  cur_node->flags |= MDES_OVERLAPPING_REQUEST;
			  next_node = NULL;
			  next_option = NULL;
			  break;
			}
		    }
		}
	    }

	}

      Mread_match (pinfo, ")");

      /* Punt no slots were declared */
      if ((list->slot_options == NULL))
	{
	  Mparse_error (pinfo,
			"A slot resource must be used by %s.", list->name);
	}

      Minsert_unique_symbol (pinfo, hmdes->res_lists, list->name, list);
      list_id++;
    }
  return;
}

/*
 * Read latency info.  Treats - as 0, and fills latency table
 * with 0's if at ')'.  Otherwise, reads latency from file.
 *
 * I had things default to -1 before, but it was too hard
 * to check to see if a latency was invalid.
 */
void
Mread_latency (Mparse_Info * pinfo, int *ptr)
{
  /* If a dash, read as -1 */
  if (Mis_next (pinfo, "-"))
    {
      Mread_match (pinfo, "-");
      *ptr = 0;
      return;
    }

  /* If a ')', implicitly this read is a 0, does not consume ')' */
  if (Mis_next (pinfo, ")"))
    {
      *ptr = 0;
      return;
    }

  if (Mread_int (pinfo, ptr))
    Mparse_error (pinfo, "Unexpected EOF");
}

void
hmdes_read_latencies (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  Hmdes_Latency *lat;
  char lat_name[MITEM_SIZE];
  int array_size, i;
  int lat_id;

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  /* Intialize enumeration of latencies */
  lat_id = 0;

  while (!hmdes_section_end (pinfo))
    {
      /* Read latency name and allocate struct */
      Mread_name (pinfo, lat_name, "Latency");
      Malloc_struct ((void **) (&lat), sizeof (Hmdes_Latency));
      Malloc_name (&lat->name, lat_name);
      lat->id = lat_id;

      /* Allocate block of ints and break up among src, src_sync, etc */
      array_size = sizeof (int) * (hmdes->max_src_syncs +
				   hmdes->max_src_operands +
				   hmdes->max_dest_syncs +
				   hmdes->max_dest_operands +
				   hmdes->max_pred_operands);
      Malloc_struct ((void **) (&lat->sync_src), array_size);
      lat->src = &lat->sync_src[hmdes->max_src_syncs];
      lat->sync_dest = &lat->src[hmdes->max_src_operands];
      lat->dest = &lat->sync_dest[hmdes->max_dest_syncs];
      lat->pred = &lat->dest[hmdes->max_dest_operands];

      /* Read ' ( exception' */
      Mread_match (pinfo, "(");
      Mread_int (pinfo, &lat->exception);

      /* Read  (pred[0] ...) */
      Mread_match (pinfo, "(");
      for (i = 0; i < hmdes->max_pred_operands; i++)
	Mread_latency (pinfo, &lat->pred[i]);
      Mread_match (pinfo, ")");

      /* Read (dest[0] ...) */
      Mread_match (pinfo, "(");
      for (i = 0; i < hmdes->max_dest_operands; i++)
	Mread_latency (pinfo, &lat->dest[i]);
      Mread_match (pinfo, ")");

      /* Read (src[0] ...) */
      Mread_match (pinfo, "(");
      for (i = 0; i < hmdes->max_src_operands; i++)
	Mread_latency (pinfo, &lat->src[i]);
      Mread_match (pinfo, ")");

      /* Read  (sync_dest[0] ...) */
      Mread_match (pinfo, "(");
      for (i = 0; i < hmdes->max_dest_syncs; i++)
	Mread_latency (pinfo, &lat->sync_dest[i]);
      Mread_match (pinfo, ")");

      /* Read  (sync_src[0] ...) */
      Mread_match (pinfo, "(");
      for (i = 0; i < hmdes->max_src_syncs; i++)
	Mread_latency (pinfo, &lat->sync_src[i]);
      Mread_match (pinfo, ")");

      Mread_match (pinfo, ")");

      Minsert_unique_symbol (pinfo, hmdes->latencies, lat->name, lat);
      lat_id++;
    }
  return;
}

/* Reads and builds a class of name 'class_name' and inserts
 * it into the class symbol table.  Returns a pointer to
 * the class just built.
 * 
 * Used for implicite class definitions when reading 
 * operations, and becuase it is here, used for reading operation 
 * classes.
 */
Hmdes_Operation_Class *
hmdes_read_class_def (Mparse_Info * pinfo, char *class_name)
{
  Hmdes *hmdes;
  Hmdes_Operation_Class *class;
  Hmdes_Class_Node *node;

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  Malloc_struct ((void **) (&class), sizeof (Hmdes_Operation_Class));
  Malloc_name (&class->name, class_name);
  class->head = NULL;
  class->tail = NULL;
  class->size = 0;

  Mread_match (pinfo, "(");

  /*
   * While there are class node definitions of the form:
   * (io_list, resource_list, latency)
   */
  while (Mis_next (pinfo, "("))
    {
      Mread_match (pinfo, "(");

      /* Allocate space for class node */
      Malloc_struct ((void **) (&node), sizeof (Hmdes_Class_Node));

      Mread_symbol (pinfo, hmdes->IO_items, (void **) (&node->io_item),
		    "IO item");
      Mread_symbol (pinfo, hmdes->res_lists, (void **) (&node->res_list),
		    "ResTable");
      Mread_symbol (pinfo, hmdes->latencies, (void **) (&node->latency),
		    "Latency");

      Mread_match (pinfo, ")");

      /* Add to class list */
      node->next = NULL;
      if (class->tail == NULL)
	class->head = node;
      else
	class->tail->next = node;
      class->tail = node;

      /* Update res_list size */
      class->size++;

    }
  Mread_match (pinfo, ")");
  Minsert_unique_symbol (pinfo, hmdes->op_class, class->name, class);

  return (class);
}

void
hmdes_read_operation_class (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  char class_name[MITEM_SIZE];

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  while (!hmdes_section_end (pinfo))
    {
      /* Read class name and allocate struct */
      Mread_name (pinfo, class_name, "Class");
      hmdes_read_class_def (pinfo, class_name);
    }
  return;
}

void
Mfree_flag_list (Hmdes_Flag_List * flag_list)
{
  Hmdes_Flag *flag, *next_flag;

  for (flag = flag_list->head; flag != NULL; flag = next_flag)
    {
      next_flag = flag->next;

      free (flag->name);
      free (flag);
    }

  flag_list->head = NULL;
  flag_list->num_flags = 0;
  flag_list->bit_version = 0;
}

/*
 * Returns 1 if flags lists are identical, 0 otherwise
 */
int
Mmatch_flag_lists (Hmdes_Flag_List * list1, Hmdes_Flag_List * list2)
{
  Hmdes_Flag *flag1, *flag2;

  /* Make sure lists are the same length */
  if (list1->num_flags != list2->num_flags)
    return (0);

  /* Compare alphabetical list to make sure they are the same */
  flag1 = list1->head;
  flag2 = list2->head;
  for (flag1 = list1->head, flag2 = list2->head;
       ((flag1 != NULL) && (flag2 != NULL));
       flag1 = flag1->next, flag2 = flag2->next)
    {
      /* If names don't match, return 0 */
      if (strcmp (flag1->name, flag2->name) != 0)
	return (0);
    }

  /* If got this far, list must be the same */
  return (1);
}

/* A flag bit mask must have exactly one bit set.
 * Returns 1 if a valid flag value.
 * Otherwise warns and returns 0.
 */
int
valid_flag_value (Mparse_Info * pinfo, Hmdes_Flag * flag)
{
  int i;
  unsigned int value;

  /* Test for no bits set */
  if (flag->value == 0)
    {
      Mparse_fatal (pinfo,
		    "Flag %s: Invalid value 0x%08x.  "
		    "Exactly 1 bit must be set.", flag->name, flag->value);
      return (0);
    }

  /* Get value into unsigned int for testing using shifts */
  value = flag->value;

  /* Test for more than one bit set */
  for (i = 0; i < 32; i++)
    {
      /* Right shift until the rightmost bit is one */
      if (((value >> i) & 1) == 1)
	{
	  /* If only one bit set, after shift should be exactly 1 */
	  if ((value >> i) == 1)
	    {
	      /* Valid mask, exactly one bit set */
	      return (1);
	    }
	  else
	    {
	      Mparse_fatal (pinfo,
			    "Flag %s: Invalid value 0x%08x.  "
			    "Exactly 1 bit must be set.",
			    flag->name, flag->value);
	      return (0);
	    }
	}
    }

  H_punt ("valid_flag_value: Should never have gotten here");
  return (0);
}

/* Reads in flag, adds to flag_list in alphabetical order, 
 * and prints warning if flag not defined (with flag_prefix) in
 * header file.
 */
void
Mread_flag (Mparse_Info * pinfo, Hmdes_Flag_List * flag_list,
	    char *flag_prefix, char *desc)
{
  Hmdes_Flag *flag, *ptr, *insert_after;
  char flag_name[MITEM_SIZE];
  char header_name[MITEM_SIZE + 50];
  int comp_val, ret_val;

  /* Get flag name */
  Mread_name (pinfo, flag_name, desc);

  /* Allocate flag structure */
  Malloc_struct ((void **) &flag, sizeof (Hmdes_Flag));
  Malloc_name (&flag->name, flag_name);

  /* Add to flag list in alphabetical order */
  insert_after = NULL;
  for (ptr = flag_list->head; ptr != NULL; ptr = ptr->next)
    {
      comp_val = strcmp (flag_name, ptr->name);

      /* Punt if flag already in list */
      if (comp_val == 0)
	Mparse_error (pinfo, "Flag '%s' specified twice", flag_name);

      /* If should go before this one, exit loop */
      if (comp_val < 0)
	break;

      /* We know we need to insert after this flag */
      insert_after = ptr;
    }

  /* Insert flag after insert_after, if NULL, put at begining of list */
  if (insert_after == NULL)
    {
      flag->next = flag_list->head;
      flag_list->head = flag;
    }

  /* Otherwise, insert after insert_after */
  else
    {
      flag->next = insert_after->next;
      insert_after->next = flag;
    }

  flag_list->num_flags++;

  /* Warn if flag name (with prefix) is not found in header files */
  sprintf (header_name, "%s%s", flag_prefix, flag_name);

  /* Get value from header file for this flag */
  if ((ret_val = find_header_value (pinfo, header_name, &flag->value)) != 1)
    {
      if (ret_val == 0)
	{
	  Mparse_fatal (pinfo,
			"Flag %s (%s) not defined in header files.",
			flag_name, header_name);
	}
    }

  /* Set bit mask for flag if flag has valid value */
  else if (valid_flag_value (pinfo, flag))
    {

      /* Make flag bit is not already set */
      if ((flag_list->bit_version & flag->value) != 0)
	{
	  /* Find flags that has the same value as it */
	  for (ptr = flag_list->head; ptr != flag; ptr = ptr->next)
	    {
	      if (ptr->value == flag->value)
		{
		  Mparse_fatal (pinfo,
				"Flags %s and %s defined "
				"to same value 0x%08x.",
				ptr->name, flag->name, flag->value);
		  break;
		}
	    }
	}
      else
	{
	  flag_list->bit_version |= flag->value;
	}
    }
}

void
hmdes_read_operations (Mparse_Info * pinfo)
{
  Hmdes *hmdes;
  Hmdes_Operation *op;
  Hmdes_Operation_Node *node;
  char class_name[MITEM_SIZE];
  char op_name[MITEM_SIZE];
  char asm_name[MITEM_SIZE];
  int first_def;
  int iclass_no;
  Hmdes_Flag_List flag_list;
  int ret_val;

  /* Initialize count of number of implicit classes */
  iclass_no = 0;

  /* Get hmdes for easy use */
  hmdes = pinfo->hmdes;

  /* Initialize flag list */
  flag_list.head = NULL;
  flag_list.num_flags = 0;
  flag_list.bit_version = 0;

  while (!hmdes_section_end (pinfo))
    {
      /* Read op name and allocate if doesn't already exist */
      Mread_name (pinfo, op_name, "Operation");
      op = (Hmdes_Operation *) Mfind_symbol (hmdes->operations, op_name);
      if (op == NULL)
	{
	  Malloc_struct ((void **) (&op), sizeof (Hmdes_Operation));
	  Malloc_name (&op->name, op_name);
	  op->head = NULL;
	  op->tail = NULL;
	  op->size = 0;
	  op->op_flags.head = NULL;
	  op->op_flags.num_flags = 0;
	  op->op_flags.bit_version = 0;
	  first_def = 1;

	  /* Get it's external id */
	  if ((ret_val = find_header_value (pinfo, op->name, &op->id)) != 1)
	    {
	      if (ret_val == 0)
		{
		  Mparse_fatal (pinfo,
				"%s not found in C header files.", op->name);
		}
	    }
	}
      else
	first_def = 0;

      /* Free any existing flags in flag list */
      Mfree_flag_list (&flag_list);

      /* Read in op flags (if any) into temp flags list */
      if (Mis_next (pinfo, "<"))
	{
	  Mread_match (pinfo, "<");
	  while (!Mis_next (pinfo, ">"))
	    {
	      Mread_flag (pinfo, &flag_list, "OP_FLAG_", "Operation flag");
	    }
	  Mread_match (pinfo, ">");
	}

      /* If not the first definition of the operation,
       * make sure have the same flags defined.
       */
      if (!first_def)
	{
	  if (!Mmatch_flag_lists (&flag_list, &op->op_flags))
	    {
	      Mparse_fatal (pinfo,
			    "%s flags differ from previous definition.",
			    op->name);
	    }

	  /* Free flag list, don't need to duplicate it */
	  Mfree_flag_list (&flag_list);
	}

      /* If first def, transfer list over to op flags */
      else
	{
	  op->op_flags.head = flag_list.head;
	  op->op_flags.num_flags = flag_list.num_flags;
	  op->op_flags.bit_version = flag_list.bit_version;
	  flag_list.head = NULL;
	  flag_list.num_flags = 0;
	  flag_list.bit_version = 0;
	}

      /*
       * Read op node of form : (asm_name  <flags> class)
       * If there are no flags, <> is not needed.
       */
      Mread_match (pinfo, "(");

      /* Read asm name and allocate node for this op */
      Mread_name (pinfo, asm_name, "Asm");
      Malloc_struct ((void **) (&node), sizeof (Hmdes_Operation_Node));
      Malloc_name (&node->asm_name, asm_name);
      node->mdes_flags.head = NULL;
      node->mdes_flags.num_flags = 0;
      node->mdes_flags.bit_version = 0;

      /* Read in mdes flags (if any) into op node n */
      if (Mis_next (pinfo, "<"))
	{
	  Mread_match (pinfo, "<");
	  while (!Mis_next (pinfo, ">"))
	    {
	      Mread_flag (pinfo, &node->mdes_flags, "ALT_FLAG_", "Alt flag");
	    }
	  Mread_match (pinfo, ">");
	}

      /* Read implicit or explicit classes */
      if (Mis_next (pinfo, "("))
	{
	  iclass_no++;
	  sprintf (class_name, "__implicit_class_%i", iclass_no);
	  node->class = hmdes_read_class_def (pinfo, class_name);
	}
      else
	{
	  /* Read class name and get pointer to it */
	  Mread_symbol (pinfo, hmdes->op_class, (void **) (&node->class),
			"Class");
	}

      Mread_match (pinfo, ")");


      /* Add to op's list of alternatives */
      node->next = NULL;
      if (op->tail == NULL)
	op->head = node;
      else
	op->tail->next = node;
      op->tail = node;

      /* Update res_list size */
      op->size++;

      /* If this is the first defition of this op, add to symbol table */
      if (first_def)
	Minsert_unique_symbol (pinfo, hmdes->operations, op->name, op);
    }

  return;
}

/*
 * Print out all info on the operation name passed.
 */
void
Mprint_op (FILE * out, Hmdes * hmdes, char *name)
{
  Hmdes_Operation *op;
  Hmdes_Operation_Node *op_node;
  Hmdes_Class_Node *c_node;
  Hmdes_IO_Item *item;
  Hmdes_Res_Node *r_node;
  Hmdes_Res_Option *option;
  Hmdes_Latency *lat;
  Hmdes_Flag *flag;
  int op_num, c_num;
  int i;

  fprintf (out, "\n%s instruction structure:\n", hmdes->file_name);
  fprintf (out, "%i pred operands\n", hmdes->max_pred_operands);
  fprintf (out, "%i dest operands\n", hmdes->max_src_operands);
  fprintf (out, "%i dest syncs\n", hmdes->max_dest_syncs);
  fprintf (out, "%i source operands\n", hmdes->max_src_operands);
  fprintf (out, "%i source syncs\n", hmdes->max_src_syncs);
  fprintf (out, "\n");

  op = Mfind_symbol (hmdes->operations, name);

  if (op == NULL)
    H_punt ("'%s' is not defined in hmdes file", name);

  if (op->id == -1)
    fprintf (out, "%s (opcode unknown)", op->name);
  else
    fprintf (out, "%s (%i) ", op->name, op->id);

  if (op->op_flags.num_flags != 0)
    fprintf (out, "<");
  for (flag = op->op_flags.head; flag != NULL; flag = flag->next)
    {
      fprintf (out, "%s", flag->name);
      if (flag->next != NULL)
	fprintf (out, ", ");
    }
  if (op->op_flags.num_flags != 0)
    fprintf (out, ">");

  if (op->size == 1)
    fprintf (out, " (%i asm name option):\n\n", op->size);
  else
    fprintf (out, " (%i asm name options):\n\n", op->size);

  op_num = 0;
  for (op_node = op->head; op_node != NULL; op_node = op_node->next)
    {
      op_num++;
      fprintf (out, "  asm name %i: '%s' ", op_num, op_node->asm_name);

      if (op_node->mdes_flags.num_flags != 0)
	fprintf (out, "<");
      for (flag = op_node->mdes_flags.head; flag != NULL; flag = flag->next)
	{
	  fprintf (out, "%s", flag->name);
	  if (flag->next != NULL)
	    fprintf (out, ", ");
	}
      if (op_node->mdes_flags.num_flags != 0)
	fprintf (out, ">");

      fprintf (out, " %s", op_node->class->name);
      fprintf (out, "\n");

      c_num = 0;

      for (c_node = op_node->class->head; c_node != NULL;
	   c_node = c_node->next)
	{
	  c_num++;
	  fprintf (out, "    resource option %i:\n", c_num);

	  item = c_node->io_item;
	  lat = c_node->latency;
	  /* Print out io item */
	  fprintf (out, "      %-11s (", item->name);

	  if (hmdes->max_pred_operands > 0)
	    fprintf (out, "<");

	  /* Print predicates */
	  for (i = 0; i < hmdes->max_pred_operands; i++)
	    {
	      if (item->pred[i] == NULL)
		fprintf (out, " -   ");
	      else
		fprintf (out, "%-5s ", item->pred[i]->name);
	    }
	  if (hmdes->max_pred_operands > 0)
	    fprintf (out, "> ");

	  fprintf (out, "[");
	  for (i = 0; i < hmdes->max_dest_operands; i++)
	    {
	      if (item->dest[i] == NULL)
		fprintf (out, " -   ");
	      else
		fprintf (out, "%-5s ", item->dest[i]->name);
	    }
	  fprintf (out, "] [");

	  for (i = 0; i < hmdes->max_src_operands; i++)
	    {
	      if (item->src[i] == NULL)
		fprintf (out, " -   ");
	      else
		fprintf (out, "%-5s ", item->src[i]->name);
	    }
	  fprintf (out, "])\n");

	  fprintf (out, "      %s:\n", c_node->res_list->name);
	  fprintf (out, "        slots: ");
	  for (option = c_node->res_list->slot_options->head;
	       option != NULL; option = option->next)
	    {
	      fprintf (out, "%i ", option->subscript->subscript);
	    }
	  fprintf (out, "\n");

	  for (r_node = c_node->res_list->head; r_node != NULL;
	       r_node = r_node->next)
	    {
	      fprintf (out, "        (");

	      for (option = r_node->head; option != NULL;
		   option = option->next)
		{
		  fprintf (out, "%s[%i]", option->resource->name,
			   option->subscript->subscript);

		  if (option->next != NULL)
		    fprintf (out, ",\n         ");

		}
	      fprintf (out, " %i", r_node->start_usage);
	      if (r_node->start_usage != r_node->end_usage)
		fprintf (out, "..%i", r_node->end_usage);
	      fprintf (out, ")\n");
	    }

	  fprintf (out, "      %-11s ( %2i (", lat->name, lat->exception);
	  for (i = 0; i < hmdes->max_src_syncs; i++)
	    {
	      if (lat->sync_src[i] == -1)
		fprintf (out, " - ");
	      else
		fprintf (out, "%2i ", lat->sync_src[i]);
	    }
	  fprintf (out, ")  (");
	  for (i = 0; i < hmdes->max_src_operands; i++)
	    {
	      if (lat->src[i] == -1)
		fprintf (out, " - ");
	      else
		fprintf (out, "%2i ", lat->src[i]);
	    }
	  fprintf (out, ")  (");
	  for (i = 0; i < hmdes->max_dest_syncs; i++)
	    {
	      if (lat->sync_dest[i] == -1)
		fprintf (out, " - ");
	      else
		fprintf (out, "%2i ", lat->sync_dest[i]);
	    }
	  fprintf (out, ")  (");
	  for (i = 0; i < hmdes->max_dest_operands; i++)
	    {
	      if (lat->dest[i] == -1)
		fprintf (out, " - ");
	      else
		fprintf (out, "%2i ", lat->dest[i]);
	    }
	  fprintf (out, ")  (");
	  for (i = 0; i < hmdes->max_pred_operands; i++)
	    {
	      if (lat->pred[i] == -1)
		fprintf (out, " - ");
	      else
		fprintf (out, "%2i ", lat->pred[i]);
	    }
	  fprintf (out, "))\n");
	  fprintf (out, "\n");
	}


      fprintf (out, "\n");

    }
}

/*
 * If the next items in the file being parse are 'end' and ')', 
 *    they are read from the file and 1 is returned.
 * 
 * Otherwise, nothing is read from the file and 0 is returned.
 */
int
hmdes_section_end (Mparse_Info * pinfo)
{
  char buf1[MITEM_SIZE];
  char buf2[MITEM_SIZE];

  /* Peek to see what the next two items in the file are */
  if ((Mpeek_ahead (pinfo, 0, buf1) == EOF) ||
      (Mpeek_ahead (pinfo, 1, buf2) == EOF))
    Mparse_error (pinfo, "Unexpected EOF, expecting 'end )'");

  /* if they are 'end' and ')', read them from file and return 1 */
  if (Mmatch ("end", buf1) && Mmatch (")", buf2))
    {
      /* Make sure Mget_next is behaiving properly */
      if ((Mget_next (pinfo, buf1) == EOF) ||
	  (Mget_next (pinfo, buf2) == EOF))
	Mparse_error (pinfo, "hmdes_section_end: algorithm error caused EOF");

      return (1);
    }

  /* Otherwise, return 0 to signal not at end of section */
  return (0);
}

/*
 * Peeks ahead in the file, returns the i'th item the
 * current place in the file.  This does not effect
 * what will be next returned by Mget_next().
 *
 * A distance of 0 returns the next item in the file.
 */
int
Mpeek_ahead (Mparse_Info * pinfo, int dist, char *buf)
{
  Mparse_Item *item;
  int i;

  /* Sanity check */
  if (dist < 0)
    H_punt ("Mpeek_ahead: dist (%i) must be >= 0", dist);

  /* Parse lines until have item we what to peek at */
  while (pinfo->queue_size <= dist)
    {
      /* If reach EOF file before reach item, return EOF
       * Also copy (EOF) into buffer for debugging purposes
       */
      if (Mparse_next_line (pinfo) == EOF)
	{
	  strcpy (buf, "(EOF)");
	  return (EOF);
	}
    }

  /* Find the desired item in the linked list */
  item = pinfo->head;
  for (i = 0; i < dist; i++)
    item = item->next;

  /* Copy item to the return buffer */
  strcpy (buf, item->buf);

  return (0);
}

/*
 * Gets the next item from file and copies it to buf.
 *
 * Returns 0 or EOF.
 */
int
Mget_next (Mparse_Info * pinfo, char *buf)
{
  Mparse_Item *item;

  /* If nothing currently parsed, parse the next line */
  if (pinfo->head == NULL)
    {
      /* If hit EOF, copy (EOF) into buf and return EOF */
      if (Mparse_next_line (pinfo) == EOF)
	{
	  strcpy (buf, "(EOF)");
	  return (EOF);
	}
    }

  /* 
   * Copy the item to the passed buf and return 0
   */
  /* Pull top item off list */
  item = pinfo->head;
  pinfo->head = item->next;
  if (pinfo->head == NULL)
    pinfo->tail = NULL;
  pinfo->queue_size--;

  /* Copy contents to passed buffer */
  strcpy (buf, item->buf);

  /* Return item to free list */
  L_free (Mparse_Item_pool, item);

  return (0);

}

void
L_preprocess_line (Mparse_Info * pinfo, char *raw_line, char *buf, int max)
{
  int i, j, value_size;
  char *line;
  char name_buf[MLINE_SIZE], *value, *name, *end_ptr;
  int define_declaration;
  Hmdes *hmdes;

  /* Decrement max, so have room for terminator */
  max--;

  /* Strip away leading and trailing whitespace and comments */
  line = Mstrip (raw_line);


  /* Detect $define at beginning line (after we strip whitespace) */
  if ((line[0] == '$') && (line[1] == 'd') && (line[2] == 'e') &&
      (line[3] == 'f') && (line[4] == 'i') && (line[5] == 'n') &&
      (line[6] == 'e') && (isspace (line[7])))
    define_declaration = 1;
  else
    define_declaration = 0;

  /* Do not preprocess the $define at the beginning of lines */
  if (define_declaration)
    {
      strcpy (buf, "$define");
      i = 7;
      line += 7;
    }
  else
    {
      i = 0;
    }

  while (*line != 0)
    {
      /* Make sure we dont exceed buffer size */
      if (i >= max)
	Mparse_error (pinfo,
		      "Line too long after preprocessing (max len %i)",
		      max + 1);

      /* If not a defined_name start symbol $, just copy to buf */
      if (*line != '$')
	{
	  buf[i] = *line;
	  i++;
	}

      /* Otherwise, replace $defined_name$ with it's value */
      else
	{
	  line++;
	  j = 0;

	  /* Get the defined_name */
	  while ((*line != 0) && (*line != '$'))
	    {
	      /* Make sure there isn't any white space in name */
	      if (isspace (*line))
		{
		  name_buf[j] = 0;
		  Mparse_error (pinfo,
				"Expecting $ after (no whitespace) '$%s'",
				name_buf);
		}
	      /* Copy over character */
	      name_buf[j] = *line;
	      j++;
	      line++;

	      /* Make sure we dont exceeded name buf size */
	      if (j >= sizeof (name_buf))
		{
		  name_buf[sizeof (name_buf) - 1] = 0;
		  Mparse_error (pinfo, "Name too long '$%s...'", name_buf);
		}

	    }
	  name_buf[j] = 0;

	  /* Verify had ending $ */
	  if (*line != '$')
	    {
	      if (strcmp (name_buf, "define") == 0)
		Mparse_error (pinfo, "Expecting a name after '$define'");
	      else
		Mparse_error (pinfo,
			      "Missing or misplaced ending $ for '$%s'",
			      name_buf);
	    }

	  /*
	   * Get defined value for name,
	   * Search tables in this order: command_line, environment, internal
	   */
	  hmdes = pinfo->hmdes;
	  if (((value = Mfind_symbol (hmdes->defines_command,
				      name_buf)) == NULL) &&
	      ((value = Mfind_symbol (hmdes->defines_environ,
				      name_buf)) == NULL) &&
	      ((value = Mfind_symbol (hmdes->defines_internal,
				      name_buf)) == NULL))
	    Mparse_error (pinfo,
			  "'$%s$' undefined.  Use '$define %s value'",
			  name_buf, name_buf);

	  /* Get length of value */
	  value_size = strlen (value);

	  /* Make sure copying in value won't exceed buffer size */
	  if ((i + value_size) > max)
	    Mparse_error (pinfo,
			  "Line too long after preprocessing (max %i).",
			  max + 1);

	  /* Copy macro to buf */
	  strcpy (&buf[i], value);

	  /* Update size of buffer */
	  i += value_size;
	}

      line++;
    }
  /* Terminate buffer */
  buf[i] = 0;

  /* Strip away any comments and trailing whitespace added by macro defs */
  if (Mstrip (buf) != &buf[0])
    H_punt ("L_preprocess_line: Internal error. Unexpected Mstrip value");
  /* Process define declartions now */
  if (define_declaration)
    {
      /* Find beginning of defined name */
      name = &buf[8];
      while (isspace (*name))
	name++;

      /* Find end of defined name */
      end_ptr = name;
      while ((*end_ptr != 0) && (!isspace (*end_ptr)))
	end_ptr++;

      /* Find beginning of value */
      value = end_ptr;
      while (isspace (*value))
	value++;

      /* Terminate name */
      *end_ptr = 0;

      /* Insert name into internal table */
      Minsert_symbol (pinfo->hmdes->defines_internal, name, strdup (value));

      /* Make line empty after preprocessing */
      buf[0] = 0;
    }
}

/*
 * Parses the next non-blank/comment line into many items placing
 * them all on a linked list in pinfo.
 * A item consists of a var_name, a number or a punctuation character.
 *
 * Returns EOF at end of file or 0 if ok.
 *
 * Max line size allowed is (MLINE_SIZE -1) and the max item size 
 * is (MITEM_SIZE -1). (One character is needed for terminator)
 * This routine will punt if the above sizes are violated. 
 */

static int
Mparse_next_line (Mparse_Info * pinfo)
{
  char in_buf[MLINE_SIZE + 1], raw_in_buf[MLINE_SIZE + 1];
  char *ptr;
  Mparse_Item *item;
  int i;

  while (fgets (raw_in_buf, MLINE_SIZE, pinfo->in) != NULL)
    {
      /* Count the lines parsed */
      pinfo->line++;

      /* Make sure long lines are terminated */
      raw_in_buf[MLINE_SIZE] = 0;

      /* Detect lines that are too long */
      if (strlen (raw_in_buf) >= (MLINE_SIZE - 1))
	Mparse_error (pinfo,
		      "Line too long.  Max line length = %i", MLINE_SIZE - 1);

      /* Do preprocessing on line read in (Substitute in $name$ values) */
      L_preprocess_line (pinfo, raw_in_buf, in_buf, MLINE_SIZE);

      /* Skip blank lines (and comments) */
      if (in_buf[0] == 0)
	continue;

      ptr = in_buf;
      while (*ptr != 0)
	{
	  item = (Mparse_Item *) L_alloc (Mparse_Item_pool);

	  /* Read string into item */
	  if (*ptr == '"')
	    {
	      /* Skip open quote */
	      ptr++;

	      /* Read in info between quotes */
	      for (i = 0; i < MITEM_SIZE; i++)
		{
		  /* Error if hit end of buffer before end quote */
		  if (ptr[i] == 0)
		    Mparse_error (pinfo,
				  "Quoted string '%s' missing end quote",
				  ptr);

		  /* Stop as soon as hit end quote */
		  if (ptr[i] != '"')
		    item->buf[i] = ptr[i];
		  else
		    break;
		}

	      /* Warn of too long names */
	      if (i >= MITEM_SIZE)
		Mparse_error (pinfo,
			      "Item too long. Max item length = %i",
			      MITEM_SIZE - 1);

	      /* Terminate item buf */
	      item->buf[i] = 0;

	      /* Advance ptr to just after end of string */
	      ptr += i;

	      /* Advance past end quote */
	      ptr++;

	      /* Strip trailing whitespace */
	      while (Misspace (*ptr))
		ptr++;
	    }
	  /* Read a name into item */
	  else if (isalpha (*ptr) || (*ptr == '_'))
	    {
	      /* Put first character into buf */
	      item->buf[0] = *ptr;

	      /* Read rest of name */
	      for (i = 1; i < MITEM_SIZE; i++)
		{
		  /* Stop as soon as hit non letter or digit or _ */
		  if (isalnum (ptr[i]) || (ptr[i] == '_'))
		    item->buf[i] = ptr[i];
		  else
		    break;
		}

	      /* Warn of too long names */
	      if (i >= MITEM_SIZE)
		Mparse_error (pinfo,
			      "Item too long. Max item length = %i",
			      MITEM_SIZE - 1);

	      /* Terminate item buf */
	      item->buf[i] = 0;

	      /* Advance ptr to just after end of string */
	      ptr += i;

	      /* Strip trailing whitespace */
	      while (Misspace (*ptr))
		ptr++;
	    }
	  /* Read a signed number into item */
	  else if (isdigit (*ptr) || ((*ptr == '-') && isdigit (ptr[1])))
	    {
	      /* Put first character into buf */
	      item->buf[0] = *ptr;

	      /* Read rest of number */
	      for (i = 1; i < MITEM_SIZE; i++)
		{
		  /* Stop as soon as hit non letter or digit */
		  if (isdigit (ptr[i]))
		    item->buf[i] = ptr[i];
		  else
		    break;
		}

	      /* Warn of too long names */
	      if (i >= MITEM_SIZE)
		Mparse_error (pinfo,
			      "Item too long. Max item length = %i",
			      MITEM_SIZE - 1);

	      /* Terminate item buf */
	      item->buf[i] = 0;

	      /* Advance ptr to just after end of string */
	      ptr += i;

	      /* Strip trailing whitespace */
	      while (Misspace (*ptr))
		ptr++;
	    }
	  /* Read '..' into item */
	  else if ((ptr[0] == '.') && (ptr[1] == '.'))
	    {
	      strcpy (item->buf, "..");
	      ptr += 2;

	      /* Strip trailing whitespace */
	      while (Misspace (*ptr))
		ptr++;

	    }
	  /* Otherwise read a single character into item */
	  else
	    {
	      item->buf[0] = *ptr;
	      item->buf[1] = 0;
	      ptr++;

	      /* Strip trailing whitespace */
	      while (Misspace (*ptr))
		ptr++;
	    }

	  /* Add item to parsed list */
	  item->next = NULL;
	  if (pinfo->tail == NULL)
	    pinfo->head = item;
	  else
	    pinfo->tail->next = item;
	  pinfo->tail = item;
	  pinfo->queue_size++;
	}
      /* Parsed a line, return 0 */
      return (0);
    }

  /* Must be at end of file if reached here */
  return (EOF);
}

int
Misspace (char ch)
{
  if ((ch == ',') || isspace (ch))
    return (1);
  else
    return (0);
}

/*
 * Strips leading and trailing whitespace and comments.
 * Modifies buffer contents.
 */
char *
Mstrip (char *buf)
{
  int i;

  /* Strip leading white space */
  for (; (*buf != 0) && Misspace (*buf); buf++)
    ;

  /* Strip trailing white space, comments */
  for (i = 0; (buf[i] != 0) && (buf[i] != '#') && (buf[i] != '%'); i++)
    ;
  buf[i] = 0;
  for (i = i - 1; (i >= 0) && Misspace (buf[i]); i--)
    buf[i] = 0;

  return (buf);
}

Mparse_Info *
create_mparse_info (char *name, Hmdes * hmdes)
{
  Mparse_Info *pinfo;
  int pinfo_size, name_size;

  /* Allocate space for pinfo structure and name */
  pinfo_size = sizeof (Mparse_Info);
  name_size = strlen (name) + 1;

  if (((pinfo = (Mparse_Info *) malloc (pinfo_size)) == NULL) ||
      ((pinfo->file_name = (char *) malloc (name_size)) == NULL))
    H_punt ("create_mparse_info: out of memory");

  /* Open passed name for reading */
  if ((pinfo->in = fopen (name, "r")) == NULL)
    H_punt ("Unable to open hmdes file '%s' for reading", name);

  /* Initialize the rest of the structure */
  strcpy (pinfo->file_name, name);
  strcpy (pinfo->section_name, "before any");
  pinfo->line = 0;
  pinfo->head = NULL;
  pinfo->tail = NULL;
  pinfo->queue_size = 0;
  pinfo->hmdes = hmdes;

  return (pinfo);
}

/* Prints a warning message showing where the parse warning was,
 * and returns
 */
void
Mparse_warn (Mparse_Info * pinfo, char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fprintf (stderr, "Warning (%s line %i, %s declaration):\n    ",
	   pinfo->file_name, pinfo->line, pinfo->section_name);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  fprintf (stderr, "\n\n");
}

/* 
 * Prints a error message showing where the parse error was,
 * but delays punting by incrementing fatal error count;
 */
void
Mparse_fatal (Mparse_Info * pinfo, char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fprintf (stderr, "Error (%s line %i, %s declaration):\n    ",
	   pinfo->file_name, pinfo->line, pinfo->section_name);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  fprintf (stderr, "\n\n");
  fatal_parse_errors++;
}

/* Punts with an error message showing where the parse error was */
void
Mparse_error (Mparse_Info * pinfo, char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fprintf (stderr, "Error (%s line %i, %s declaration):\n    ",
	   pinfo->file_name, pinfo->line, pinfo->section_name);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  fprintf (stderr, "\n\n");
  exit (1);
}

int
Mmatch (char *s1, char *s2)
{
  for (; *s1 != 0; s1++, s2++)
    {
      if (toupper (*s1) != toupper (*s2))
	return (0);
    }

  if (*s2 != 0)
    return (0);
  else
    return (1);
}

void
associate_reg_files (Hmdes * hmdes)
{
  Msymbol *set_node;
  Hmdes_IO_Set *IO_set;
  Msymbol_Table *prev_def;
  char *val_str, *end_ptr;
  char name_buf[MITEM_SIZE + 5];	/* Max possible size for name */
  int num_found, num_not_found, num_bad, num_redefs;
  int prev_val;
  char *prev_str;
  int i;

  num_found = 0;
  num_not_found = 0;
  num_bad = 0;
  num_redefs = 0;

  for (set_node = hmdes->IO_sets->head; set_node != NULL;
       set_node = set_node->next_linear)
    {
      IO_set = (Hmdes_IO_Set *) set_node->ptr;

      prev_def = NULL;
      prev_val = -1;
      prev_str = "";


      /* Prepend MDES_OPERAND_ to the front of the register file name */
      sprintf (name_buf, "MDES_OPERAND_%s", IO_set->name);

      /* Search all header files for symbol */
      for (i = 0; i < num_header_files; i++)
	{
	  val_str = Mfind_symbol (header_table[i].table, name_buf);

	  if (val_str == NULL)
	    continue;

	  /* Try to convert to an int */
	  IO_set->external_id = strtol (val_str, &end_ptr, 0);

	  if (*end_ptr != 0)
	    {
	      fprintf (stderr,
		       "Error: %s value '%s' from %s is not an int\n",
		       IO_set->name, val_str, header_table[i].table->name);
	      num_bad++;
	      IO_set->external_id = -1;
	    }
	  else if (IO_set->external_id < 0)
	    {
	      fprintf (stderr,
		       "Error for IO_set %s: %s defines as %i, "
		       "cannot be negative\n",
		       IO_set->name, header_table[i].table->name,
		       IO_set->external_id);
	      num_bad++;
	      IO_set->external_id = -1;
	    }

	  if (prev_def == NULL)
	    {
	      prev_def = header_table[i].table;
	      prev_val = IO_set->external_id;
	      prev_str = val_str;
	    }
	  else
	    {
	      if (prev_val != IO_set->external_id)
		{
		  fprintf (stderr,
			   "Error for IO_set %s: %s defines as %s, "
			   "%s defines as %s\n",
			   IO_set->name, header_table[i].table->name,
			   val_str, prev_def->name, prev_str);
		  num_redefs++;
		}
	    }

	}

      if ((prev_def == NULL) || (IO_set->external_id == -1))
	{
	  num_not_found++;
	  fprintf (stderr, "IO_set %-11s not mapped to an external id\n",
		   IO_set->name);
	}
      else
	{
	  fprintf (stderr, "IO_set %-11s mapped to %2i\n",
		   IO_set->name, IO_set->external_id);
	  num_found++;
	}

    }
  if ((num_bad != 0) || (num_redefs != 0))
    {
      fprintf (stderr, "Cannot continue: %i IO_Sets with bad values\n",
	       num_bad);
      fprintf (stderr, "               : %i invalid redefinitions\n",
	       num_redefs);
      fprintf (stderr, "Header files scanned:\n");
      for (i = 0; i < num_header_files; i++)
	fprintf (stderr, "%s\n", header_table[i].table->name);
      exit (1);

    }
  fprintf (stderr,
	   "IO_set specifiers: %3i found     %3i not found\n",
	   num_found, num_not_found);
}

void
associate_opcodes (Hmdes * hmdes)
{
  int num_found, num_not_found, num_bad, num_redefs;
  Msymbol *op_node;
  char *val_str, *end_ptr;
  Hmdes_Operation *op;
  int i;
  Msymbol_Table *prev_def;
  int prev_val;
  char *prev_str;

  num_found = 0;
  num_not_found = 0;
  num_bad = 0;
  num_redefs = 0;

  for (op_node = hmdes->operations->head; op_node != NULL;
       op_node = op_node->next_linear)
    {
      op = (Hmdes_Operation *) op_node->ptr;

      prev_def = NULL;
      prev_val = -1;
      prev_str = "";

      /* Search all header files for symbol */
      for (i = 0; i < num_header_files; i++)
	{
	  val_str = Mfind_symbol (header_table[i].table, op->name);

	  if (val_str == NULL)
	    continue;

	  /* Try to convert to an int */
	  op->id = strtol (val_str, &end_ptr, 0);

	  if (*end_ptr != 0)
	    {
	      fprintf (stderr,
		       "Error: %s value '%s' from %s is not an int\n",
		       op->name, val_str, header_table[i].table->name);
	      num_bad++;
	      op->id = -1;
	    }
	  else if (op->id < 0)
	    {
	      fprintf (stderr,
		       "Error for operation %s: %s defines as %i, "
		       "cannot be negative\n",
		       op->name, header_table[i].table->name, op->id);
	      num_bad++;
	      op->id = -1;
	    }
	  if (prev_def == NULL)
	    {
	      prev_def = header_table[i].table;
	      prev_val = op->id;
	      prev_str = val_str;
	    }
	  else
	    {
	      if (prev_val != op->id)
		{
		  fprintf (stderr,
			   "Error for operation %s: %s defined as %s, "
			   "%s defined as %s\n",
			   op->name, header_table[i].table->name,
			   val_str, prev_def->name, prev_str);
		  num_redefs++;
		}
	    }

	}
      if (prev_def == NULL)
	{
	  num_not_found++;
	  fprintf (stderr, "warning: %s not found in C header files\n",
		   op->name);
	}
      else
	num_found++;
    }

  if ((num_bad != 0) || (num_not_found != 0) || (num_redefs != 0))
    {
      fprintf (stderr,
	       "Cannot continue: %i op_names not found\n", num_not_found);
      fprintf (stderr, "                 %i with bad values\n", num_bad);
      fprintf (stderr,
	       "                 %i invalid redefinitions\n", num_redefs);
      fprintf (stderr, "Header files scanned:\n");
      for (i = 0; i < num_header_files; i++)
	fprintf (stderr, "%s\n", header_table[i].table->name);
      exit (1);
    }
}

/* 
 * Clears an bit mask of 'size' words
 */
void
clear_mask (int *mask, int size)
{
  int i;
  for (i = 0; i < size; i++)
    mask[i] = 0;
}

/*
 * Sets specified bit in mask
 * Assumes mask an array of 32 bits of size MAX_MASK_SIZE;
 */
void
set_mask_bit (int *mask, unsigned int bit)
{
  unsigned int index, offset;
  index = bit >> 5;
  if (index >= MAX_MASK_SIZE)
    H_punt ("set_mask_bit: index (%i) > MAX_MASK_SIZE (%i)",
	    index, MAX_MASK_SIZE);
  offset = bit & 0x1F;
  if (offset > 31)
    H_punt ("set_mask_bit; offset (%i) > 32", offset);
  mask[index] |= (1 << offset);
}

/* 
 * Prints the mask to out, starting with highest numbered bits.
 * (Ie. prints out mask in reverse array order.)
 */
void
print_mask (FILE * out, int *mask, int size)
{
  int i;

  for (i = size - 1; i >= 0; i--)
    {
      fprintf (out, "%08x ", mask[i]);
    }
}

/*
 * Writes res_node output for res_list
 * Used by write_lmdes
 */
void
write_res_node (FILE * out, int mask_size, Hmdes_Res_Node * res_node)
{
  int mask[MAX_MASK_SIZE], pred_mask[MAX_MASK_SIZE];
  Hmdes_Res_Option *res_option;

  if (res_node->num_options <= 0)
    H_punt ("write_res_node: res_node->num_options = %i\n",
	    res_node->num_options);

  fprintf (out, "  %2i %2i %2i %i\n",
	   res_node->start_usage, res_node->end_usage,
	   res_node->num_options, res_node->flags);
  for (res_option = res_node->head; res_option != NULL;
       res_option = res_option->next)
    {
      fprintf (out, "  ");
      clear_mask (mask, mask_size);
      set_mask_bit (mask, res_option->subscript->id);
      print_mask (out, mask, mask_size);
      fprintf (out, "  ");
      clear_mask (pred_mask, mask_size);
      /* Currently, none of these bits can be set */
      print_mask (out, pred_mask, mask_size);
      fprintf (out, "\n");
    }
}

/*
 * Writes low-level mdes file to the file specified
 */
void
write_lmdes (Hmdes * hmdes, FILE * out)
{
  int i, mask_size;
  int mask[MAX_MASK_SIZE];
  char res_buf[MITEM_SIZE + 20];
  Msymbol *symbol;
  Hmdes_IO_Node *IO_node;
  Hmdes_IO_Set *IO_set;
  Hmdes_IO_Item *IO_item;
  Hmdes_Res_Sub *res_sub;
  Hmdes_Resource *resource;
  Hmdes_Resource *slots_avail;
  Hmdes_Res_Option *res_option;
  Hmdes_Res_Node *res_node;
  Hmdes_Res_List *res_list;
  Hmdes_Latency *latency;
  Hmdes_Class_Node *class_node;
  Hmdes_Operation_Node *op_node;
  Hmdes_Operation *op;
  Hmdes_Operation **op_table;
  int alt_count, max_opcode, max_set_id;
  int num_RU_entries_required, cycles_spanned;
  int error_count;
  int total_name_len;
  int total_slots;
  int total_rused;
  int total_options;
  int total_alts;


  fprintf (out, "Lmdes Version 3\n\n");

  /* Get the slots available */
  slots_avail = (Hmdes_Resource *) Mfind_symbol (hmdes->resources, "slot");
  if (slots_avail == NULL)
    {
      H_punt ("No slots declared, should have been detected earlier");
    }

  /* Print out any paramaters that we need */
  fprintf (out, "proc_model: %2i\n", hmdes->processor_model);

  /*
   * Print out number of each operand type, the max slot number,
   * and the number of slots in the machine
   */
  fprintf (out, "sizes: %2i %2i %2i %2i %2i %2i %2i \n\n",
	   hmdes->max_pred_operands, hmdes->max_dest_operands,
	   hmdes->max_src_operands, hmdes->max_src_syncs,
	   hmdes->max_dest_syncs, hmdes->max_slot,
	   slots_avail->num_subscripts);

  mask_size = ((hmdes->reg_file->entry_count - 1) / 32) + 1;
  if (mask_size > MAX_MASK_SIZE)
    H_punt ("write_lmdes: rmask_size (%i) > MAX_MASK_SIXE (%i)",
	    mask_size, MAX_MASK_SIZE);

  /* Calc the total name buffer space needed for entire IO_Set_array.
   * also calc max external_id for IO_set
   */
  total_name_len = 0;
  max_set_id = -1;
  for (symbol = hmdes->IO_sets->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      IO_set = (Hmdes_IO_Set *) symbol->ptr;
      total_name_len += strlen (IO_set->name) + 1;
      if (IO_set->external_id > max_set_id)
	max_set_id = IO_set->external_id;

    }

  if (max_set_id == -1)
    H_punt ("Error: None of the IO_sets have external id's");

  fprintf (out, "IO_Sets_begin  %1i %5i %5i %5i %5i %6i\n", mask_size,
	   hmdes->reg_file->entry_count, hmdes->IO_sets->entry_count,
	   hmdes->null_set->external_id, max_set_id, total_name_len);

  for (symbol = hmdes->IO_sets->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      IO_set = (Hmdes_IO_Set *) symbol->ptr;
      clear_mask (mask, mask_size);
      for (IO_node = IO_set->head; IO_node != NULL; IO_node = IO_node->next)
	{
	  set_mask_bit (mask, IO_node->reg_file->id);
	}
      fprintf (out, "%3i %3i %-11s ", IO_set->id, IO_set->external_id,
	       IO_set->name);
      print_mask (out, mask, mask_size);
      fprintf (out, "\n");
    }
  fprintf (out, "IO_Sets_end\n\n");

  /* Calc the total name buffer space needed for entire IO_Item_array */
  total_name_len = 0;
  for (symbol = hmdes->IO_items->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      IO_item = (Hmdes_IO_Item *) symbol->ptr;
      total_name_len += strlen (IO_item->name) + 1;
    }

  fprintf (out, "IO_Items_begin  %3i %6i\n", hmdes->IO_items->entry_count,
	   total_name_len);

  for (symbol = hmdes->IO_items->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      IO_item = (Hmdes_IO_Item *) symbol->ptr;

      fprintf (out, "%3i %-11s ", IO_item->id, IO_item->name);

      for (i = 0; i < hmdes->max_pred_operands; i++)
	{
	  if (IO_item->pred[i] == NULL)
	    H_punt ("pred IO_item NULL. Algorthm error");
	  else
	    fprintf (out, "%2i ", IO_item->pred[i]->id);
	}
      fprintf (out, " ");
      for (i = 0; i < hmdes->max_dest_operands; i++)
	{
	  if (IO_item->dest[i] == NULL)
	    H_punt ("dest IO_item NULL. Algorthm error");
	  else
	    fprintf (out, "%2i ", IO_item->dest[i]->id);
	}
      fprintf (out, " ");
      for (i = 0; i < hmdes->max_src_operands; i++)
	{
	  if (IO_item->src[i] == NULL)
	    H_punt ("src IO_item NULL. Algorthm error");
	  else
	    fprintf (out, "%2i ", IO_item->src[i]->id);
	}
      fprintf (out, "\n");
    }
  fprintf (out, "IO_Items_end\n\n");


  /* Calc the total name buffer space needed for entire resource_array, */
  total_name_len = 0;
  for (symbol = hmdes->resources->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      resource = (Hmdes_Resource *) symbol->ptr;
      for (res_sub = resource->head; res_sub != NULL; res_sub = res_sub->next)
	{
	  sprintf (res_buf, "%s[%i]", resource->name, res_sub->subscript);
	  total_name_len += strlen (res_buf) + 1;
	}
    }

  fprintf (out, "Resources_begin  %3i %6i\n", hmdes->num_resources,
	   total_name_len);
  for (symbol = hmdes->resources->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      resource = (Hmdes_Resource *) symbol->ptr;

      for (res_sub = resource->head; res_sub != NULL; res_sub = res_sub->next)
	{
	  sprintf (res_buf, "%s[%i]", resource->name, res_sub->subscript);
	  fprintf (out, "%3i %-11s\n", res_sub->id, res_buf);
	}
    }
  fprintf (out, "Resources_end\n\n");


  mask_size = ((hmdes->num_resources - 1) / 32) + 1;
  if (mask_size <= 0)
    H_punt ("write_lmdes: resource mask size (%i) too small", mask_size);
  if (mask_size >= MAX_MASK_SIZE)
    H_punt ("write_lmdes: resource mask size (%i) >= MAX (%i)",
	    mask_size, MAX_MASK_SIZE);

  /* Calc the total name buffer space needed for entire ResList_array,
   * Also, the total number of resources used, and the total
   * number of options (multiple options per resource is allowed)
   */
  total_name_len = 0;
  total_rused = 0;
  total_options = 0;
  total_slots = 0;
  for (symbol = hmdes->res_lists->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      res_list = (Hmdes_Res_List *) symbol->ptr;
      total_name_len += strlen (res_list->name) + 1;
      total_rused += res_list->size + 1 /* for slot options */ ;
      total_slots += res_list->slot_options->num_options;
      total_options += res_list->slot_options->num_options;
      for (res_node = res_list->head; res_node != NULL;
	   res_node = res_node->next)
	{
	  total_options += res_node->num_options;
	}
    }

  fprintf (out, "ResList_begin  %3i %5i %5i %5i %2i %6i\n",
	   hmdes->res_lists->entry_count,
	   total_rused, total_slots, total_options, mask_size,
	   total_name_len);

  for (symbol = hmdes->res_lists->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      res_list = (Hmdes_Res_List *) symbol->ptr;

      /* Calculate how many RU entries will be needed if this
       * alternative is scheduled.  One RU entry is needed
       * per option per cycle.
       */
      num_RU_entries_required = 1;	/* For slot entry */
      for (res_node = res_list->head; res_node != NULL;
	   res_node = res_node->next)
	{
	  cycles_spanned = (res_node->end_usage - res_node->start_usage) + 1;
	  if (cycles_spanned < 1)
	    H_punt ("write_lmdes: cycles_spanned (%i) < 1", cycles_spanned);
	  num_RU_entries_required += cycles_spanned;
	}

      fprintf (out, "%2i %-11s %4i %4i %4i\n", res_list->id, res_list->name,
	       res_list->size + 1 /* for slot options */ ,
	       res_list->slot_options->num_options, num_RU_entries_required);

      /* Print out the slot options (slot id's)  for this res list */
      fprintf (out, "  ");
      for (res_option = res_list->slot_options->head; res_option != NULL;
	   res_option = res_option->next)
	{
	  fprintf (out, "%2i ", res_option->subscript->subscript);
	}
      fprintf (out, "\n");

      /* Print out slot options */
      write_res_node (out, mask_size, res_list->slot_options);

      /* Print out the resource list */
      for (res_node = res_list->head; res_node != NULL;
	   res_node = res_node->next)
	{
	  write_res_node (out, mask_size, res_node);
	}
    }
  fprintf (out, "ResList_end\n\n");

  /* Calc the total name buffer space needed for entire latency_array */
  total_name_len = 0;
  for (symbol = hmdes->latencies->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      latency = (Hmdes_Latency *) symbol->ptr;
      total_name_len += strlen (latency->name) + 1;
    }

  fprintf (out, "Latencies_begin  %3i %6i\n", hmdes->latencies->entry_count,
	   total_name_len);
  for (symbol = hmdes->latencies->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      latency = (Hmdes_Latency *) symbol->ptr;

      fprintf (out, "%3i %-11s  %2i ", latency->id, latency->name,
	       latency->exception);
      for (i = 0; i < hmdes->max_pred_operands; i++)
	{
	  fprintf (out, "%2i ", latency->pred[i]);
	}
      fprintf (out, " ");
      for (i = 0; i < hmdes->max_dest_operands; i++)
	{
	  fprintf (out, "%2i ", latency->dest[i]);
	}
      fprintf (out, " ");
      for (i = 0; i < hmdes->max_src_operands; i++)
	{
	  fprintf (out, "%2i ", latency->src[i]);
	}
      fprintf (out, " ");
      for (i = 0; i < hmdes->max_src_syncs; i++)
	{
	  fprintf (out, "%2i ", latency->sync_src[i]);
	}
      fprintf (out, " ");
      for (i = 0; i < hmdes->max_dest_syncs; i++)
	{
	  fprintf (out, "%2i ", latency->sync_dest[i]);
	}
      fprintf (out, "\n");
    }
  fprintf (out, "Latencies_end\n\n");

  /* Calc the total name buffer space needed for entire operation_array */
  total_name_len = 0;
  for (symbol = hmdes->operations->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      op = (Hmdes_Operation *) symbol->ptr;

      total_name_len += strlen (op->name) + 1;

      for (op_node = op->head; op_node != NULL; op_node = op_node->next)
	{
	  for (class_node = op_node->class->head; class_node != NULL;
	       class_node = class_node->next)
	    {
	      total_name_len += strlen (op_node->asm_name) + 1;
	    }
	}
    }

  /* Calculate max opcode and total num of alts */
  max_opcode = -1;
  total_alts = 0;
  for (symbol = hmdes->operations->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      op = (Hmdes_Operation *) symbol->ptr;
      if (op->id > max_opcode)
	max_opcode = op->id;

      /* I do not allow negative opcodes */
      if (op->id < 0)
	H_punt ("write_mdes: negative opcode (%i) for %s not allowed\n",
		op->id, op->name);

      /*
       * Do not allow max opcode go above 16384 (just so we don't
       * allocate some huge array in lmdes.)  
       */
      if (op->id > 16384)
	H_punt ("write_mdes: opcode %i for %s > 16384\n", op->id, op->name);

      for (op_node = op->head; op_node != NULL; op_node = op_node->next)
	{
	  for (class_node = op_node->class->head; class_node != NULL;
	       class_node = class_node->next)
	    {
	      total_alts++;
	    }
	}
    }


  /* Make sure there are not two operations with the same opcode */
  Malloc_struct ((void **) &op_table,
		 sizeof (Hmdes_Operation *) * (max_opcode + 1));

  /* Initialize op_table to NULL */
  for (i = 0; i <= max_opcode; i++)
    op_table[i] = NULL;

  /* Place all opcodes into table */
  error_count = 0;
  for (symbol = hmdes->operations->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      op = (Hmdes_Operation *) symbol->ptr;

      if (op_table[op->id] != NULL)
	{
	  fprintf (stderr, "Error: %s and %s have the same opcode %i\n",
		   op_table[op->id]->name, op->name, op->id);
	  error_count++;
	}
    }
  if (error_count != 0)
    H_punt ("Cannot continue: %i operations with conflicting opcodes",
	    error_count);

  fprintf (out, "Operations_begin %5i %5i %5i %6i\n",
	   hmdes->operations->entry_count, total_alts, max_opcode,
	   total_name_len);
  for (symbol = hmdes->operations->head; symbol != NULL;
       symbol = symbol->next_linear)
    {
      op = (Hmdes_Operation *) symbol->ptr;

      alt_count = 0;
      for (op_node = op->head; op_node != NULL; op_node = op_node->next)
	{
	  for (class_node = op_node->class->head; class_node != NULL;
	       class_node = class_node->next)
	    {
	      alt_count++;
	    }
	}

      fprintf (out, "%4i %-11s %3i %08x\n", op->id, op->name,
	       alt_count, op->op_flags.bit_version);

      for (op_node = op->head; op_node != NULL; op_node = op_node->next)
	{
	  for (class_node = op_node->class->head; class_node != NULL;
	       class_node = class_node->next)
	    {

	      fprintf (out, "  %-11s %08x  %3i %3i %3i\n",
		       op_node->asm_name, op_node->mdes_flags.bit_version,
		       class_node->io_item->id,
		       class_node->res_list->id, class_node->latency->id);
	    }
	}
    }
  fprintf (out, "Operations_end\n\n");
}


/* Library of c functions to test out C++ linking capability */

void
H_punt (char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  vfprintf (stderr, fmt, ap);
  va_end (ap);

  fprintf (stderr, "\n");
  exit (1);
}
