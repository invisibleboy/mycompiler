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
 *      File:   lmdes2_customizer.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  Aug. 1995
\*****************************************************************************/
/* 11/14/02 REK Cleaning up this code so that it does not try to access a 
 *              32 bit integer as a 64 bit pointer. */


/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <library/i_error.h>
#include <library/l_alloc_new.h>
#include <library/dynamic_symbol.h>
#include <library/heap.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

/* Turn on error checking in lmdes */
#undef MD_DEBUG_MACROS
#include <library/md.h>

/* 11/14/02 REK A struct that simply contains two pointers.  It can be used
 *              to maintain a mapping from 'from' to 'to'. */
typedef struct Pointer_Mapping
{
  void *from;
  void *to;
}
Pointer_Mapping;

#define SLOT_RESOURCE		0x00000001
typedef struct Resource_Info
{
  char *name;
  MD_Entry *resource_entry;
  INT_Symbol_Table *time_table;	/* indexed by usage time */
  int flags;
}
Resource_Info;

/* Used for finding the minimum cover for the forbidden latencies */
typedef struct Pair_Node
{
  Resource_Info *resource;
  int time1;
  struct Pair_Node *next;
}
Pair_Node;

/* Used to build a content-addressable entry table */
typedef struct ENTRY_Contents_Table
{
  char *ignore_field;		/* Content field to ignore */
  INT_Symbol_Table *hash_table;	/* Used to find contents */
}
ENTRY_Contents_Table;

/* Used to store list of entries with same hash value for
 *  ENTRY_Contents_Table
 */
typedef struct ENTRY_Contents_Node
{
  MD_Entry *entry;
  struct ENTRY_Contents_Node *next_node;
}
ENTRY_Contents_Node;

/* Set to 1 to get a zillion messages from the optimizations */
#define DEBUG_OPTI 0

/* Set to 1 to get some indication where in the opti process the 
 * custimizer is.
 */
#define VERBOSE_OPTI 0

/* Set to 1 to get some of the optimizations stats as the
 * mdes is being customized.
 */
#define VERBOSE_STATS 0

/* Prototype of header file reader */
extern STRING_Symbol_Table *read_header_file (char *file_name,
					      int print_warnings);

/* Internal prototypes */
void customize_md (MD * md);
int eliminate_redundant_new_entries_only (MD * md, char *section_name);

int CU_redundant_entries (MD_Entry * entry1, MD_Entry * entry2,
			  char *ignore_field);

char *program_name = NULL;

/* Command line parameters */
char *input_file_name = NULL;
char *output_file_name = NULL;
int using_stdin = 0;
char *output_mode = NULL;
char *input_mode = NULL;
int output_page_width = 80;
int verbose = 0;
int expand_tables = 0;
int tree_opti = 1;
int place_slots_first = 0;
int static_stats = 0;
int print_trees = 0;
int resource_minimization = 1;

/* Optimization levels:
 * 
 * 0 No optimizations at all, minimal transformations to make MDES work.
 * 1 Run classical optimizations (common-subexpression, dead code removal)
 * 2 Use bit-field representation (no bit-field optis)
 * 3 Do general bit-field optimizations (zero-time transformation,
 *   resource minimization, reorder usage checks to check cycle 0 first)
 * 4 Do some AND/OR tree optiminzations (OR-tree reordering)
 * 5 Do all AND/OR tree optimizations (Also move common usages around)
 *   (Level 4 was found to yield the best average dynamic performance)
 * 
 * Some of the individual optis (-no_minimizationand -no_tree_opti) 
 * can be turned off with a separate flags.
 *
 * Expanding tables out into OR-Trees (-expand) is orthogonal to the above.
 * AND/OR tree optimizations automatically turned of if expanding tables
 * since can hurt or unexpand tables.
 */
int opti_level = 4;		/* Level 4 yields best average performance */


/* Global variables */
L_Alloc_Pool *ENTRY_Contents_Table_pool = NULL;
L_Alloc_Pool *ENTRY_Contents_Node_pool = NULL;

/* Table of header file tables */
STRING_Symbol_Table *header_table_table = NULL;

/* A string map used for renaming */
STRING_Symbol_Table *rename_table = NULL;

/* Number of errors that occurred using header file */
int header_errors = 0;

/* Number of errors that occurred during customization (after header file
 * annotations).
 */
int customization_errors = 0;


void print_generating_set (FILE * out, INT_Symbol_Table * generating_set,
			   MD_Entry ** unit_array, int committed);
void commit_usage (INT_Symbol_Table *** forbidden_matrix,
		   Resource_Info * resource, int user, int time);

void print_table_trees (FILE * out, MD * md);

/* Error routine, fmt and trailing args are the same as printf.
 * Adds header message identifying error as coming from IMPACT hmdes2
 * customizer.
 *
 * Exits with value 1.
 */
static void
C_punt (MD * md, char *fmt, ...)
{
  va_list args;

  /* Print error message header.
   * Allow NULL to be passed in for md
   */
  fprintf (stderr, "\nIMPACT hmdes2 customizer error");
  if (md == NULL)
    fprintf (stderr, ":\n");
  else
    fprintf (stderr, " (in \"%s\"):\n", md->name);

  /* Print out error message */
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);

  va_end (args);
  fprintf (stderr, "\n");

  exit (1);
}

void
print_time_stamp (FILE * out)
{
  static int count = 0;
  char time_buf[100];
  time_t cur_time;

  count++;

  cur_time = time (NULL);
  strftime (time_buf, sizeof (time_buf),
	    "%H:%M:%S %p, %A %B %d", localtime (&cur_time));
  fprintf (out, "Time stamp %i: %s\n", count, time_buf);
  fflush (out);
}

void
print_usage (char *fmt, ...)
{
  va_list args;

  /* Print out error message if fmt is not NULL */
  if (fmt != NULL)
    {
      fprintf (stderr, "\nError: ");
      va_start (args, fmt);
      vfprintf (stderr, fmt, args);
      va_end (args);
      fprintf (stderr, "\n");
    }

  fprintf (stderr, "\n");
  fprintf (stderr, "usage: %s [options] [input_file]\n\n", program_name);
  fprintf (stderr, "options:\n");
  fprintf (stderr,
	   "  -On               where n is opti level (0-5), default 4\n");
  fprintf (stderr,
	   "  -expand           expands all the table variations out\n");
  fprintf (stderr,
	   "  -no_tree_opti     turn off AND/OR tree opti (tree opti may "
				"undo -expand)\n");
  fprintf (stderr,
	   "  -static_stats     prints static stats about the optimized "
			       "mdes\n");
  fprintf (stderr, "  -print_trees      prints the and/or trees to stdout\n");
  fprintf (stderr,
	   "  -no_minimization  do not apply resource minimization "
			       "techniques\n");
  fprintf (stderr,
	   "  -o output_file    outputs to output_file instead of to standard "
			       "output.\n");
  fprintf (stderr,
	   "  -stdin            reads lmdes from standard input instead of "
			        "input_file.\n");
  fprintf (stderr,
	   "  -hmdes            output compiled hmdes instead of lmdes.\n");
  fprintf (stderr,
	   "  -width int        sets page width for -hmdes output.\n");
  fprintf (stderr,
	   "  -verbose          prints customization steps to stdout.\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "debug options:\n");
  fprintf (stderr,
	   "  -bypass_alloc     use malloc instead of alloc routines.\n");
  fprintf (stderr, "\n");
  exit (1);
}

void
read_command_line_parameters (int argc, char **arg_ptr)
{
  char *end_ptr;

  /* Get program name for error messages */
  program_name = strdup (*arg_ptr);

  /* Start parsing arguments at first argument */
  arg_ptr++;
  for (; *arg_ptr != NULL; arg_ptr++)
    {
      /* If first character is a '-', must be an option */
      if ((*arg_ptr)[0] == '-')
	{
	  /* Get output file */
	  if (strcmp (*arg_ptr, "-o") == 0)
	    {
	      /* Make sure -o not specified twice */
	      if (output_file_name != NULL)
		{
		  print_usage ("-o option specified twice.");
		}

	      /* Get output file name, expect it to be non-NULL */
	      arg_ptr++;
	      if (*arg_ptr == NULL)
		{
		  print_usage ("expected '-o output_file'.\n");
		}
	      output_file_name = strdup (*arg_ptr);
	    }

	  /* Get opti-level */
	  else if ((*arg_ptr)[1] == 'O')
	    {
	      switch ((*arg_ptr)[2])
		{
		case '0':
		  opti_level = 0;
		  break;

		case '1':
		  opti_level = 1;
		  break;

		case '2':
		  opti_level = 2;
		  break;

		case '3':
		  opti_level = 3;
		  break;

		case '4':
		  opti_level = 4;
		  break;

		case '5':
		  opti_level = 5;
		  break;

		default:
		  print_usage
		    ("Invalid opti level specified '%s', level 0-5 expected.",
		     *arg_ptr);

		}

	      /* Better be null after number */
	      if ((*arg_ptr)[3] != 0)
		print_usage ("Invalid opti level specifier '%s'.\n",
			     *arg_ptr);
	    }

	  /* Get the -expand flag */
	  else if (strcmp (*arg_ptr, "-expand") == 0)
	    {
	      expand_tables = 1;
	    }

	  /* Get the -no_tree_opti flag */
	  else if (strcmp (*arg_ptr, "-no_tree_opti") == 0)
	    {
	      printf ("> No AND/OR tree optimizations will be performed.\n");
	      tree_opti = 0;
	    }

	  /* Get the -static_stats flag */
	  else if (strcmp (*arg_ptr, "-static_stats") == 0)
	    {
	      static_stats = 1;
	    }

	  /* Get the -print_trees flag */
	  else if (strcmp (*arg_ptr, "-print_trees") == 0)
	    {
	      print_trees = 1;
	    }

	  /* Get -no_minimization flag */
	  else if (strcmp (*arg_ptr, "-no_minimization") == 0)
	    {
	      resource_minimization = 0;
	    }

	  /* Get -stdin flag */
	  else if (strcmp (*arg_ptr, "-stdin") == 0)
	    {
	      using_stdin = 1;
	    }

	  /* Get -hmdes flag */
	  else if (strcmp (*arg_ptr, "-hmdes") == 0)
	    {
	      /* Make sure some other output mode is not specified */
	      if (output_mode != NULL)
		print_usage ("output mode specified twice '%s' and '%s'.",
			     output_mode, *arg_ptr);
	      output_mode = strdup (*arg_ptr);
	    }

	  /* Get -width n flag */
	  else if (strcmp (*arg_ptr, "-width") == 0)
	    {
	      /* Get width argument, expect it to be non-NULL */
	      arg_ptr++;
	      if (*arg_ptr == NULL)
		{
		  print_usage ("expected '-width int'.\n");
		}

	      output_page_width = strtol (*arg_ptr, &end_ptr, 10);
	      if (*end_ptr != 0)
		{
		  print_usage ("expected '-width int'.\n");
		}
	    }

	  /* Get -verbose flag */
	  else if (strcmp (*arg_ptr, "-verbose") == 0)
	    {
	      verbose = 1;
	    }

	  /* Get -bypass_alloc flag */
	  else if (strcmp (*arg_ptr, "-bypass_alloc") == 0)
	    {
	      bypass_alloc_routines = 1;
	    }

	  else
	    {
	      print_usage ("unknown command line option '%s'.\n", *arg_ptr);
	    }
	}

      /* Otherwise, must be file name (can only be one of command line) */
      else
	{
	  if (input_file_name != NULL)
	    {
	      print_usage ("only one input file may be specified.\n");

	    }

	  /* Get input file name */
	  input_file_name = strdup (*arg_ptr);
	}
    }

  /* Make sure input file has been specified */
  if ((input_file_name == NULL) && !using_stdin)
    {
      /* Print error only if they specified something */
      if (argc > 1)
	{
	  print_usage ("input_file or -stdin must be specified.\n");
	}
      else
	{
	  print_usage (NULL);
	}
    }

  /* Make sure input_file and -stdin have not both been specified */
  if ((input_file_name != NULL) && using_stdin)
    {
      print_usage ("may not specify both input_file and -stdin.\n");
    }

  /* Make sure input and output file are not the same */
  if ((input_file_name != NULL) && (output_file_name != NULL) &&
      (strcmp (input_file_name, output_file_name) == 0))
    {
      print_usage
	("may not specify input file (%s) that is the same as output file.",
	 input_file_name);
    }

}

int
main (int argc, char **argv)
{
  FILE *in;
  FILE *out;
  MD *md;
  int error_count;

  /* Read command line arguments and input file name */
  read_command_line_parameters (argc, argv);

  /* Open input file */
  if (using_stdin)
    {
      input_file_name = strdup ("(stdin)");
      in = stdin;
    }
  else
    {
      if ((in = fopen (input_file_name, "r")) == NULL)
	C_punt (NULL, "Unable to open input file '%s'.", input_file_name);
    }

  /* Open output file */
  if (output_file_name == NULL)
    {
      output_file_name = strdup ("(stdout)");
      out = stdout;
    }
  else
    {
      if ((out = fopen (output_file_name, "w")) == NULL)
	C_punt (NULL, "Unable to open output file '%s'.", output_file_name);
    }

  /* Read in lmdes */
  md = MD_read_md (in, input_file_name);

  if (verbose)
    printf ("Checking lmdes2 for consistency before customization.\n\n");

  /* Check md for consistency before customizing */
  if ((error_count = MD_check_md (stderr, md)) != 0)
    {
      C_punt (md,
	      "%i inconsistencies found before customization!\n"
	      "Cannot customize.",
	      error_count);
    }

  /* Customize md for IMPACT */
  customize_md (md);

  /* Code stable. Don't check for now to speed up */
#if 0
  if (verbose)
    printf ("\nChecking lmdes2 for consistency after customization.\n");

  /* Check md for consistency after customizing */
  if ((error_count = MD_check_md (stderr, md)) != 0)
    {
      fprintf (stderr,
	       "Error after customizing '%s':\n  %i inconsistencies found!\n",
	       md->name, error_count);
      fprintf (stderr,
	       "  Either the customizer should have punted before now or\nthe "
	       "customization program needs to be fixed!\n");
      fprintf (stderr,
	       "  Writing customized version anyway (for debugging only)!\n");
    }
#endif

  /* Print out lmdes in form desired */
  if (output_mode == NULL)
    {
      MD_write_md (out, md);
    }
  else if (strcmp (output_mode, "-hmdes") == 0)
    {
      /* Print out section and entry declarations so, combined with the
       * output of MD_print_md, the output will be a legal hmdes file.
       */
      MD_print_md_declarations (out, md, 80);

      /* Put blank line after declarations */
      putc ('\n', out);

      MD_print_md (out, md, 80);
    }
  else
    {
      C_punt (NULL, "Unknown output mode '%s'.", output_mode);
    }

  /* Free output mode */
  if (output_mode != NULL)
    free (output_mode);

  /* About to end anyway, don't spend time to delete structures */
#if 0
  /* Free the database */
  MD_delete_md (md);
#endif

  if (verbose)
    printf ("Customization complete.\n");

  /* Close input and output (may be stdin/stdout) */
  fclose (in);
  fclose (out);

  return (0);
}

void
read_header_tables (MD * md)
{
  MD_Section *parm_section;
  MD_Field_Decl *value_field_decl;
  MD_Entry *header_entry;
  MD_Field *value_field;
  STRING_Symbol_Table *table;
  char *header_name;
  int i;

  /* Get the parameter section from database */
  if ((parm_section = MD_find_section (md, "Parameter")) == NULL)
    C_punt (md, "'Parameter' section expected in hmdes2.");

  /* Get the value field declaration (so can find field) */
  if ((value_field_decl = MD_find_field_decl (parm_section, "value")) == NULL)
    {
      C_punt (md,
	      "'value' field declaration expected in 'Parameter' section.");
    }

  /* Get the entry that lists the header table names */
  header_entry = MD_find_entry (parm_section, "customization_headers");
  if (header_entry == NULL)
    {
      C_punt (md,
	      "'customization_headers' entry expected in '%s' section.",
	      parm_section->name);
    }

  /* Get the value field */
  if ((value_field = MD_find_field (header_entry, value_field_decl)) == NULL)
    {
      C_punt (md, "'value' field expected in %s->%s", parm_section->name,
	      header_entry->name);
    }

  /* Expect at least one name in field */
  if (value_field->max_element_index < 0)
    {
      C_punt (md,
	      "At least one header file must be specified in\n%s->%s->%s.",
	      parm_section->name, header_entry->name,
	      value_field->decl->name);
    }

  /* Create table of header tables */
  header_table_table = STRING_new_symbol_table ("Header Table", 64);

  /* Read in headers specified */
  for (i = 0; i <= value_field->max_element_index; i++)
    {
      header_name = MD_get_string (value_field, i);

      if (verbose)
	printf ("Reading '%s'.\n", header_name);

      /* If have not already read this header file, read it in */
      if (STRING_find_symbol (header_table_table, header_name) == NULL)
	{
	  /* Read in header table */
	  table = read_header_file (header_name, verbose);

	  /* Add to table of tables */
	  STRING_add_symbol (header_table_table, header_name, (void *) table);
	}

      /* Otherwise, punt that name specified twice */
      else
	{
	  C_punt (md, "%s->%s->%s[%i]:\n  '%s' specified twice.",
		  parm_section->name, header_entry->name,
		  value_field->decl->name, i, header_name);
	}
    }
}

#define MAX_IDENT_LEN	(100+1)

int
find_header_value (MD * md, char *name, int *value)
{
  STRING_Symbol_Table *table;
  STRING_Symbol *symbol;
  char *val_str, *end_ptr;
  char *prev_def, *prev_str;
  int prev_val;

  prev_def = NULL;
  prev_str = NULL;
  prev_val = -1;

  /* Search all header files for name */
  for (symbol = header_table_table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      table = (STRING_Symbol_Table *) symbol->data;

      val_str = (char *) STRING_find_symbol_data (table, name);

      if (val_str == NULL)
	continue;

      /* Try to convert to an int */
      *value = strtol (val_str, &end_ptr, 0);

      if ((*end_ptr != 0) || (val_str[0] == 0))
	{
	  C_punt (md, "%s value '%s' expected to be an int in\n'%s'.",
		  name, val_str, table->name);
	}

      /* Detect if defined to two different values set for name */
      if (prev_def == NULL)
	{
	  prev_def = table->name;
	  prev_val = *value;
	  prev_str = val_str;
	}
      else
	{
	  if (prev_val != *value)
	    {
	      fprintf (stderr,
		       "Error, %s defined to two different values in "
		       "header files:\n '%s' defines as '%s'\n '%s' defines "
		       "as '%s'\n\n",
		       name, prev_def, prev_str, table->name, val_str);
	      header_errors++;
	    }
	}
    }

  if (prev_def != NULL)
    return (1);
  else
    {
      *value = -1;
      return (0);
    }
}

/* Annotates the values associated with the entry's name by searching
 * the header files using the 'prefix' prefixed to the entrys name.
 * Puts this INT into the 'field_name' field.
 *
 * If value not found in header file and 'required' set, gives error.
 */
void
annotate_header_values (MD * md, char *section_name, char *field_name,
			char *prefix, int required)
{
  MD_Section *section;
  MD_Field_Decl *field_decl;
  MD_Entry *entry;
  MD_Field *field;
  char name_buf[MAX_IDENT_LEN + 50];
  int value;


  /* The prefix better be less than 50 characters (so will fit in name_buf) */
  if (strlen (prefix) >= 50)
    {
      C_punt (md, "Prefix '%s' for section '%s' too long!", prefix,
	      section_name);
    }


  /* Get the section we are annotating from database */
  if ((section = MD_find_section (md, section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", section_name);

  /* Delete all existing fields of 'field_name'(if exist), by deleting
   * field declaration.
   */
  field_decl = MD_find_field_decl (section, field_name);
  if (field_decl != NULL)
    MD_delete_field_decl (field_decl);


  /* Create new OPTIONAL or REQUIRED field declaration that has 1 int in it.
   */
  if (required)
    {
      field_decl = MD_new_field_decl (section, field_name, MD_REQUIRED_FIELD);
    }
  else
    {
      field_decl = MD_new_field_decl (section, field_name, MD_OPTIONAL_FIELD);
    }
  MD_require_int (field_decl, 0);

  /* Scan all entries and look for corresponding #defines in the header
   * files.
   */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Make sure length of entry name is less than MAX_IDENT_LEN */
      if (strlen (entry->name) >= MAX_IDENT_LEN)
	{
	  C_punt (md,
		  "Section %s: '%s' exceeds the %i char max impossed for "
		  "sanities sake!",
		  section->name, entry->name, MAX_IDENT_LEN - 1);
	}

      /* Add prefix for search */
      sprintf (name_buf, "%s%s", prefix, entry->name);

      /* If value found, add lcode_id field with the value found */
      if (find_header_value (md, name_buf, &value))
	{
	  field = MD_new_field (entry, field_decl, 1);
	  MD_set_int (field, 0, value);
	}
      else if (required)
	{
	  fprintf (stderr,
		   "Error customizing %s->%s:\n  '%s' not found in any of "
		   "the header files!\n\n",
		   section->name, entry->name, name_buf);
	  header_errors++;
	}
    }
}

/* 20020822 SZU
 * Assign hash number field to the section in new field 'hash'.
 *
 * Originally for Operation section to provide hash number for latency classes.
 */
void
annotate_hash (MD * md, char *section_name)
{
  MD_Section *section;
  MD_Field_Decl *dest_field_decl;
  MD_Field *dest_field;
  MD_Entry *entry;
  int index;

  /* Get the section we are annotating from database */
  if ((section = MD_find_section (md, section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", section_name);

  /* Delete all existing field of 'hash', by deleting the field declaration. */
  dest_field_decl = MD_find_field_decl (section, "hash");
  if (dest_field_decl != NULL)
    MD_delete_field_decl (dest_field_decl);

  /* Create new field of name 'hash' that has int in it. */
  dest_field_decl = MD_new_field_decl (section, "hash", MD_REQUIRED_FIELD);
  MD_require_int (dest_field_decl, 0);

  index = 0;
  /* Scan all entries in section */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      dest_field = MD_new_field (entry, dest_field_decl, 1);
      MD_set_int (dest_field, dest_field->max_element_index + 1, index);
      index++;
    }
}

/* 20020822 SZU
 * New version of annotate_latency_class.
 * Instead of storing string, store hash number of Operation.
 * Hash number done by annotate_hash function.
 *
 * Originally for the latency class in IMPACT_Operation and Prod_Cons_Latency.
 * Preserve latency class information, which is based on either Operation
 * or Scheduling Alternative.
 *
 * src_field_name is the name of the field in the section that has the latency
 * class info.
 * dest_field_name is the field in which the info will be preserved.
 * Instead of storing the name in a string, the info now stores a hash number,
 * determined via annotate_hash function.
 */
void
annotate_latency_class (MD * md, char *section_name, char *src_field_name,
			char *dest_field_name)
{
  MD_Section *section;
  MD_Field_Decl *src_field_decl, *dest_field_decl, *hash_field_decl;
  MD_Field *src_field, *dest_field, *hash_field;
  MD_Entry *src_entry, *value_entry;
  int index, hash;
  
  /* Get the sections we are annotating from database */
  if ((section = MD_find_section (md, section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", section_name);

  /* Get the field that the latency classes will come from */
  src_field_decl = MD_find_field_decl (section, src_field_name);
  if (src_field_decl == NULL)
    {
      C_punt (md, "Field '%s' in section '%s' not found.",
	      src_field_name, section_name);
    }

  /* Delete all existing field of 'dest_field_name', by deleting
   * the field declaration.
   */
  dest_field_decl = MD_find_field_decl (section, dest_field_name);
  if (dest_field_decl != NULL)
    MD_delete_field_decl (dest_field_decl);

  /* Create new field of name 'dest_field_name' that has int in it. */
  dest_field_decl = MD_new_field_decl (section, dest_field_name,
					   MD_REQUIRED_FIELD);
  MD_require_int (dest_field_decl, 0);
  MD_require_int (dest_field_decl, 1);
  MD_kleene_star_requirement (dest_field_decl, 1);

  /* Scan all entries in section */
  for (src_entry = MD_first_entry (section); src_entry != NULL;
       src_entry = MD_next_entry (src_entry))
    {
      /* Get the src_field in order to read the links */
      src_field = MD_find_field (src_entry, src_field_decl);

      /* Process the links in the src field */
      if (src_field != NULL)
	{
	  /* Create destination field for this entry */
	  dest_field = MD_new_field (src_entry, dest_field_decl, 1);

	  for (index = 0; index <= src_field->max_element_index; index++)
	    {
	      /* Get the target of this link */
	      value_entry = MD_get_link (src_field, index);

	      /* Must not be NULL */
	      if (value_entry == NULL)
		{
		  C_punt (md, "%s->%s->%s[%i] NULL, not a link as expected.",
			  section->name, src_entry->name,
			  src_field_name, index);
		}

	      hash_field_decl = MD_find_field_decl (value_entry->section,
						    "hash");
	      hash_field = MD_find_field (value_entry, hash_field_decl);
	      hash = MD_get_int (hash_field, 0);

	      MD_set_int (dest_field, dest_field->max_element_index + 1, hash);
	    }
	}
    }
}

void
convert_to_bit_representation (MD * md, char *src_section_name,
			       char *src_field_name,
			       char *value_section_name,
			       char *value_field_name, char *dest_field_name)
{
  MD_Section *src_section, *value_section;
  MD_Field_Decl *src_field_decl, *value_field_decl, *dest_field_decl;
  MD_Entry *src_entry, *value_entry;
  MD_Field *src_field, *value_field, *dest_field;
  int bit_representation, bit_value, i;

  /* Get the section we are converting */
  if ((src_section = MD_find_section (md, src_section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", src_section_name);

  /* Get the field we are converting */
  src_field_decl = MD_find_field_decl (src_section, src_field_name);
  if (src_field_decl == NULL)
    {
      C_punt (md, "Field '%s' in '%s' section not found.", src_field_name,
	      src_section_name);
    }

  /* Expect field to be a kleene starred link to another section */
  if ((src_field_decl->max_require_index > 1) ||
      (src_field_decl->kleene_starred_req == NULL) ||
      (src_field_decl->require[0]->type != MD_LINK) ||
      (src_field_decl->require[0]->link_array_size != 1))
    {
      C_punt (md,
	      "Error customizing field '%s->%s':\nExpected a single "
	      "kleene_starred link to section '%s'.",
	      src_section_name, src_field_name, value_section_name);
    }

  /* Get the section that the values are coming from */
  value_section = src_field_decl->require[0]->link[0];

  /* Make sure this where we expect the values to come from */
  if (strcmp (value_section->name, value_section_name) != 0)
    {
      C_punt (md,
	      "Expected '%s->%s' to link to section '%s'\nnot section '%s'.",
	      src_section_name, src_field_name, value_section_name,
	      value_section->name);
    }

  /* Get the field that the values will come from */
  value_field_decl = MD_find_field_decl (value_section, value_field_name);
  if (value_field_decl == NULL)
    {
      C_punt (md, "Field '%s' in section '%s' not found.",
	      value_field_name, value_section_name);
    }

  /* Expect the value field to be a single int value */
  if ((value_field_decl->max_require_index > 1) ||
      (value_field_decl->kleene_starred_req != NULL) ||
      (value_field_decl->require[0]->type != MD_INT))
    {
      C_punt (md,
	      "Error using field '%s->%s':\nExpected a single INT.",
	      value_section_name, value_field_name);
    }

  /* Delete all existing field of 'dest_field_name', by deleting
   * the field declaration.
   */
  dest_field_decl = MD_find_field_decl (src_section, dest_field_name);
  if (dest_field_decl != NULL)
    MD_delete_field_decl (dest_field_decl);

  /* Create new REQUIRED field of name 'dest_field_name' that
   * has one INT in it.
   */
  dest_field_decl = MD_new_field_decl (src_section, dest_field_name,
				       MD_REQUIRED_FIELD);
  MD_require_int (dest_field_decl, 0);

  /* Finally, convert each the src_field LINKS into one INT, using
   * the bit value in value_field
   */
  for (src_entry = MD_first_entry (src_section); src_entry != NULL;
       src_entry = MD_next_entry (src_entry))
    {
      /* Get the src_field in order to read the links */
      src_field = MD_find_field (src_entry, src_field_decl);

      /* By default, the bit representation is 0 if the src_field
       * is not specified.
       */
      bit_representation = 0;

      /* Process the links in the src field if specified */
      if (src_field != NULL)
	{
	  for (i = 0; i <= src_field->max_element_index; i++)
	    {
	      /* Get the target of this link */
	      value_entry = MD_get_link (src_field, i);

	      /* Must not be NULL */
	      if (value_entry == NULL)
		{
		  C_punt (md, "%s->%s->%s[i] NULL, not a link as expected.",
			  src_section->name, src_entry->name,
			  src_field_name, i);
		}

	      /* Get the value field from the value entry */
	      value_field = MD_find_field (value_entry, value_field_decl);

	      /* Must not be NULL */
	      if (value_field == NULL)
		{
		  C_punt (md, "%s->%s->%s not found.",
			  value_section->name, value_entry->name,
			  value_field_name);
		}

	      /* Get the bit value from the value field */
	      bit_value = MD_get_int (value_field, 0);

	      /* Or into bit_representation */
	      bit_representation |= bit_value;
	    }
	}

      /* Create destination field with bit representation */
      dest_field = MD_new_field (src_entry, dest_field_decl, 1);
      MD_set_int (dest_field, 0, bit_representation);
    }

}

/* Does a depth first recursive search to add all children reached by
 * the entry's field and places pointer the those children's entry pointers
 * into the child_table.
 */
void
find_children (INT_Symbol_Table * child_table, MD_Entry * entry,
	       MD_Field_Decl * field_decl)
{
  MD_Entry *child_entry;
  MD_Field *field;
  int i;

  /* Get the entry's field.  If doesn't exists, done */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    return;

  /* Recursively add any children reached by each link */
  for (i = 0; i <= field->max_element_index; i++)
    {
      child_entry = MD_get_link (field, i);

      /* If child already in table, do nothing and goto next child */
      /* 11/14/02 REK Strip the high 32 bits (if applicable) off of 
       *              the address of child_entry before looking it up. */
      if (INT_find_symbol (child_table, 
			   ((long) child_entry & (long) 0xffffffff)) != NULL)
	continue;

      /* Add this child to the table and recursively find it's children */
      /* 11/14/02 REK Strip the high 32 bits (if applicable) off of 
       *              the address of child_entry before looking it up. */
      INT_add_symbol (child_table, ((long) child_entry & (long) 0xffffffff), 
		      (void *) child_entry);

      find_children (child_table, child_entry, field_decl);
    }
}


void
add_all_children_to_list (MD * md, char *section_name, char *field_name)
{
  MD_Section *section;
  MD_Field_Decl *field_decl;
  MD_Entry *entry, *child_entry;
  MD_Field *field;
  INT_Symbol_Table *child_table;
  INT_Symbol *child_symbol;
  int i;


  /* Get the section we are customizing */
  if ((section = MD_find_section (md, section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", section_name);

  /* Get the declaration of the field we are expanding */
  if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
    {
      C_punt (md, "Field '%s' not found for section '%s'.", field_name,
	      section_name);
    }

  /* Expect field to be a kleened starred link to the same section */
  if ((field_decl->max_require_index > 1) ||
      (field_decl->kleene_starred_req == NULL) ||
      (field_decl->require[0]->type != MD_LINK) ||
      (field_decl->require[0]->link_array_size != 1) ||
      (field_decl->require[0]->link[0] != section))
    {
      C_punt (md,
	      "Error customizing field '%s->%s':\nExpect a single "
	      "kleene_starred link to section '%s'.",
	      section_name, field_name, section_name);
    }

  /* Create child table to hold all the children */
  child_table = INT_new_symbol_table ("child table", 0);

  /* Expand "field_name" for each entry, so that all the indirect children
   * are pointed to the list.
   */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* If the field is not defined, do nothing and goto next entry */
      if ((field = MD_find_field (entry, field_decl)) == NULL)
	continue;


      /* Do a depth first recursive search to find all the children
       * reached by this entry.
       */
      find_children (child_table, entry, field_decl);

      /* Remove all the children already in field */
      for (i = 0; i <= field->max_element_index; i++)
	{
	  child_entry = MD_get_link (field, i);

	  /* Find child by using address of child */
	  /* 11/14/02 REK Strip the high 32 bits (if applicable) off of 
	   *              the address of child_entry before looking it up. */
	  child_symbol = INT_find_symbol (child_table, 
					  ((long) child_entry & \
					   (long) 0xffffffff));

	  /* Remove child already in field from table */
	  INT_delete_symbol (child_symbol, NULL);
	}

      /* Add the remaining children in the table to the field,
       * removing them from the table as they are processed
       */
      while ((child_symbol = child_table->head_symbol) != NULL)
	{
	  child_entry = (MD_Entry *) child_symbol->data;

	  /* Add child to end of field */
	  MD_set_link (field, field->max_element_index + 1, child_entry);

	  /* Remove child entry from child table, do not delete
	   * the child entry!
	   */
	  INT_delete_symbol (child_symbol, NULL);
	}
      /* The child table is empty at this point */
    }

  /* Free the child table */
  INT_delete_symbol_table (child_table, NULL);
}

void
convert_to_int_list (MD * md, char *src_section_name, char *src_field_name,
		     char *value_section_name, char *value_field_name,
		     char *dest_field_name)
{
  MD_Section *src_section, *value_section;
  MD_Field_Decl *src_field_decl, *value_field_decl, *dest_field_decl;
  MD_Entry *src_entry, *value_entry;
  MD_Field *src_field, *value_field, *dest_field;
  int value, i;

  /* Get the section we are converting */
  if ((src_section = MD_find_section (md, src_section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", src_section_name);

  /* Get the section we are getting the values from */
  if ((value_section = MD_find_section (md, value_section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", value_section_name);

  /* Get the field we are converting */
  src_field_decl = MD_find_field_decl (src_section, src_field_name);
  if (src_field_decl == NULL)
    {
      C_punt (md, "Field '%s' in '%s' section not found.", src_field_name,
	      src_section_name);
    }

  /* Expect field to be a kleene starred link to another section */
  if ((src_field_decl->max_require_index > 1) ||
      (src_field_decl->kleene_starred_req == NULL) ||
      (src_field_decl->require[0]->type != MD_LINK) ||
      (src_field_decl->require[0]->link_array_size != 1) ||
      (src_field_decl->require[0]->link[0] != value_section))
    {
      C_punt (md,
	      "Error customizing field '%s->%s':\nExpected a single "
	      "kleene_starred link to section '%s'.",
	      src_section_name, src_field_name, value_section_name);
    }

  /* Get the field that the values will come from */
  value_field_decl = MD_find_field_decl (value_section, value_field_name);
  if (value_field_decl == NULL)
    {
      C_punt (md, "Field '%s' in section '%s' not found.",
	      value_field_name, value_section_name);
    }

  /* Expect the value field to be a single int value */
  if ((value_field_decl->max_require_index > 1) ||
      (value_field_decl->kleene_starred_req != NULL) ||
      (value_field_decl->require[0]->type != MD_INT))
    {
      C_punt (md,
	      "Error using field '%s->%s':\nExpected a single INT.",
	      value_section_name, value_field_name);
    }

  /* Delete all existing field of 'dest_field_name', by deleting
   * the field declaration.
   */
  dest_field_decl = MD_find_field_decl (src_section, dest_field_name);
  if (dest_field_decl != NULL)
    MD_delete_field_decl (dest_field_decl);

  /* Create new REQUIRED field of name 'dest_field_name' that
   * has zero or more INTs in it.
   */
  dest_field_decl = MD_new_field_decl (src_section, dest_field_name,
				       MD_REQUIRED_FIELD);
  MD_require_int (dest_field_decl, 0);
  MD_kleene_star_requirement (dest_field_decl, 0);


  /* Finally, convert each the src_field LINKS into an INT, and
   * place that INT in the dest_field list.
   */
  for (src_entry = MD_first_entry (src_section); src_entry != NULL;
       src_entry = MD_next_entry (src_entry))
    {
      /* Get the src_field in order to read the links */
      src_field = MD_find_field (src_entry, src_field_decl);

      /* Create destination field for this entry */
      dest_field = MD_new_field (src_entry, dest_field_decl, 1);

      /* Process the links in the src field if specified */
      if (src_field != NULL)
	{
	  for (i = 0; i <= src_field->max_element_index; i++)
	    {
	      /* Get the target of this link */
	      value_entry = MD_get_link (src_field, i);

	      /* Must not be NULL */
	      if (value_entry == NULL)
		{
		  C_punt (md, "%s->%s->%s[i] NULL, not a link as expected.",
			  src_section->name, src_entry->name,
			  src_field_name, i);
		}

	      /* Get the value field from the value entry */
	      value_field = MD_find_field (value_entry, value_field_decl);

	      /* Skip if value NULL */
	      if (value_field == NULL)
		continue;

	      /* Get the value from the value field */
	      value = MD_get_int (value_field, 0);

	      /* Add int to dest field */
	      MD_set_int (dest_field, dest_field->max_element_index + 1,
			  value);
	    }
	}
    }
}

void
convert_to_int_list_all_targets (MD * md, char *src_section_name,
				 char *src_field_name,
				 char *value_field_name,
				 char *dest_field_name)
{
  MD_Section *src_section;
  MD_Field_Decl *src_field_decl, *value_field_decl, *dest_field_decl;
  MD_Entry *src_entry, *value_entry;
  MD_Field *src_field, *value_field, *dest_field;
  int value, i;

  /* Get the section we are converting */
  if ((src_section = MD_find_section (md, src_section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", src_section_name);

  /* Get the field we are converting */
  src_field_decl = MD_find_field_decl (src_section, src_field_name);
  if (src_field_decl == NULL)
    {
      C_punt (md, "Field '%s' in '%s' section not found.", src_field_name,
	      src_section_name);
    }

  /* Expect field to be a kleene starred link to this section */
  if ((src_field_decl->max_require_index > 1) ||
      (src_field_decl->kleene_starred_req == NULL) ||
      (src_field_decl->require[0]->type != MD_LINK) ||
      (src_field_decl->require[0]->link_array_size != 1) ||
      (src_field_decl->require[0]->link[0] != src_section))
    {
      C_punt (md,
	      "Error customizing field '%s->%s':\nExpected a single "
	      "kleene_starred link to section '%s'.",
	      src_section_name, src_field_name, src_section_name);
    }

  /* Get the field that the values will come from */
  value_field_decl = MD_find_field_decl (src_section, value_field_name);
  if (value_field_decl == NULL)
    {
      C_punt (md, "Field '%s' in section '%s' not found.",
	      value_field_name, src_section_name);
    }

  /* Expect the value field to be a single int value */
  if ((value_field_decl->max_require_index > 1) ||
      (value_field_decl->kleene_starred_req != NULL) ||
      (value_field_decl->require[0]->type != MD_INT))
    {
      C_punt (md,
	      "Error using field '%s->%s':\nExpected a single INT.",
	      src_section_name, value_field_name);
    }

  /* Delete all existing field of 'dest_field_name', by deleting
   * the field declaration.
   */
  dest_field_decl = MD_find_field_decl (src_section, dest_field_name);
  if (dest_field_decl != NULL)
    MD_delete_field_decl (dest_field_decl);

  /* Create new REQUIRED field of name 'dest_field_name' that
   * has one or more INTs in it.
   */
  dest_field_decl = MD_new_field_decl (src_section, dest_field_name,
				       MD_REQUIRED_FIELD);
  MD_require_int (dest_field_decl, 0);
  MD_require_int (dest_field_decl, 1);
  MD_kleene_star_requirement (dest_field_decl, 1);


  /* Finally, convert each the src_field LINKS into an INT, and
   * place that INT in the dest_field list.
   *
   * Add src_entry's value first (if any) and prevent it from being
   * added twice.
   */
  for (src_entry = MD_first_entry (src_section); src_entry != NULL;
       src_entry = MD_next_entry (src_entry))
    {
      /* Get the src_field in order to read the links */
      src_field = MD_find_field (src_entry, src_field_decl);

      /* Create destination field for this entry */
      dest_field = MD_new_field (src_entry, dest_field_decl, 1);

      /* Add this entry's value if it has one */
      value_field = MD_find_field (src_entry, value_field_decl);
      if (value_field != NULL)
	{
	  value = MD_get_int (value_field, 0);
	  MD_set_int (dest_field, dest_field->max_element_index + 1, value);
	}

      /* Process the links in the src field if specified */
      if (src_field != NULL)
	{
	  for (i = 0; i <= src_field->max_element_index; i++)
	    {
	      /* Get the target of this link */
	      value_entry = MD_get_link (src_field, i);

	      /* Must not be NULL */
	      if (value_entry == NULL)
		{
		  C_punt (md, "%s->%s->%s[i] NULL, not a link as expected.",
			  src_section->name, src_entry->name,
			  src_field_name, i);
		}

	      /* If value points to src entry, skip, already added */
	      if (value_entry == src_entry)
		continue;

	      /* Get the value field from the value entry */
	      value_field = MD_find_field (value_entry, value_field_decl);

	      /* Skip if value NULL */
	      if (value_field == NULL)
		continue;

	      /* Get the value from the value field */
	      value = MD_get_int (value_field, 0);

	      /* Add int to dest field */
	      MD_set_int (dest_field, dest_field->max_element_index + 1,
			  value);
	    }
	}

      /* Make sure at least one value specified for dest_field */
      if (dest_field->max_element_index == -1)
	{
	  fprintf (stderr,
		   "Error %s->%s:\n"
		   "  Unable to build a proper '%s' field!\n",
		   src_section_name, src_entry->name, dest_field_name);

	  fprintf (stderr,
		   "\n"
		   "  The %s field contains all the MDES_OPERAND_xxx ids\n"
		   "  that map to this entry.  Make sure MDES_OPERAND_%s (or a"
		   "  \n a more specific operand) is #defined in one of the "
		   "  specifed\nC header files! If this operand/field type "
		   "  is no longer\n"
		   "  supported by IMPACT, delete it.\n\n",
		   dest_field_name, src_entry->name);

	  customization_errors++;
	}
    }
}

/* Checks that the INT values found in the field 'field_name' fall within
 * the given bounds.  Assumes field has only on argument and it is an INT
 */
void
check_int_bounds (MD * md, char *section_name, char *field_name,
		  int lower_bound, int upper_bound)
{
  MD_Section *section;
  MD_Field_Decl *field_decl;
  MD_Entry *entry;
  MD_Field *field;
  int value;

  /* Get the section we are converting */
  if ((section = MD_find_section (md, section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", section_name);

  /* Get the field we are checking the INT bounds of */
  field_decl = MD_find_field_decl (section, field_name);
  if (field_decl == NULL)
    {
      C_punt (md, "Field '%s' in '%s' section not found.", field_name,
	      section_name);
    }

  /* Expect field to require exactly one INT value */
  if ((field_decl->max_require_index > 1) ||
      (field_decl->kleene_starred_req != NULL) ||
      (field_decl->require[0]->type != MD_INT))
    {
      C_punt (md,
	      "Error customizing field '%s->%s':\nExpected a single INT value",
	      section_name, field_name);
    }

  /* Check all fields in section */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Get the INT field if it exists */
      if ((field = MD_find_field (entry, field_decl)) == NULL)
	continue;

      /* Get it's value */
      value = MD_get_int (field, 0);

      /* Check bounds */
      if ((value < lower_bound) || (value > upper_bound))
	{
	  fprintf (stderr,
		   "Value of '%s->%s->%s', %i, out of bounds:\n  Value must "
		   "fall between %i and %i.\n\n",
		   section_name, entry->name, field_name, value,
		   lower_bound, upper_bound);
	  customization_errors++;
	}
    }
}

/* Checks proc_opcs to make sure non-negative and not huge.
 * Also checks to make sure multiple entries do not get the same proc_opc.
 */
void
check_proc_opcs (MD * md)
{
  MD_Section *section;
  MD_Field_Decl *field_decl;
  MD_Entry *entry, *conflict;
  MD_Field *field;
  int proc_opc;
  INT_Symbol_Table *opc_table;

  /* Create a table of proc_opc's */
  opc_table = INT_new_symbol_table ("proc_opc", 250);

  /* Get the section we are converting */
  if ((section = MD_find_section (md, "IMPACT_Operation")) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", "IMPACT_Operation");

  /* Get the field we are checking the INT bounds of */
  field_decl = MD_find_field_decl (section, "proc_opc");
  if (field_decl == NULL)
    {
      C_punt (md, "Field '%s' in '%s' section not found.", "proc_opc",
	      "IMPACT_Operation");
    }

  /* Expect field to require exactly one INT value */
  if ((field_decl->max_require_index > 1) ||
      (field_decl->kleene_starred_req != NULL) ||
      (field_decl->require[0]->type != MD_INT))
    {
      C_punt (md,
	      "Error customizing field '%s->%s':\nExpected a single INT value",
	      "IMPACT_Operation", "proc_opc");
    }

  /* Check all fields in section */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Get the INT field if it exists */
      if ((field = MD_find_field (entry, field_decl)) == NULL)
	{
	  C_punt (md, "IMPACT_Operation->%s: field 'proc_opc' not found!",
		  entry->name);
	}

      /* Get it's proc_opc */
      proc_opc = MD_get_int (field, 0);

      /* Check bounds */
      if (proc_opc < 0)
	{
	  fprintf (stderr,
		   "IMPACT_Operation->%s: proc_opc (%i) may not be negative!\n"
		   "\n",
		   entry->name, proc_opc);
	  customization_errors++;
	}
      else if (proc_opc > 16384)
	{
	  fprintf (stderr,
		   "IMPACT_Operation->%s: proc_opc (%i) may not be > 16384!\n"
		   "\n",
		   entry->name, proc_opc);
	  customization_errors++;

	}

      /* See if there is another operation with this opcode */
      conflict = (MD_Entry *) INT_find_symbol_data (opc_table, proc_opc);
      if (conflict != NULL)
	{
	  fprintf (stderr,
		   "IMPACT_Operation->%s and IMPACT_Operation->%s both have "
		   "proc_opc %i!\n\n",
		   entry->name, conflict->name, proc_opc);

	  customization_errors++;
	}
      else
	{
	  /* Add this opcode to the table */
	  INT_add_symbol (opc_table, proc_opc, (void *) entry);
	}
    }

  /* Delete the proc_opc table */
  INT_delete_symbol_table (opc_table, NULL);
}

/* Scheduler uses unsigned short (was char) to store table options selected.
 * So make sure that there are less than 65536 (was 256) elements (options)
 * in the one_of field for each Table_Option entry.
 * 
 * One could write a routine in this customizer to break up 
 * alternatives that have such table options into multiple alternatives,
 * each using a subset of the options.  I have better things to do than
 * putting in special purpose code for this obscure case. :) -JCG 3/5/96
 *
 * In order to support fully expanding out reservation trees,
 * I bit the bullet and increased the data size to unsigned short. -JCG 3/27/96
 */
void
check_table_option_count (MD * md)
{
  MD_Section *section;
  MD_Field_Decl *field_decl;
  MD_Entry *entry;
  MD_Field *field;

  /* Get the Table_Option section */
  if ((section = MD_find_section (md, "Table_Option")) == NULL)
    C_punt (md, "'Table_Option' section expected in hmdes2.");

  /* Get the one_of field */
  field_decl = MD_find_field_decl (section, "one_of");
  if (field_decl == NULL)
    C_punt (md, "Field 'one_of' in 'Table_Option' section not found.");

  /* Check every entry's one_of field */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Get the one_of field */
      if ((field = MD_find_field (entry, field_decl)) == NULL)
	{
	  C_punt (md, "Table_Option->%s->one_of: field expected.",
		  entry->name);
	}

      /* Make sure there are not too many options */
      if (field->max_element_index >= 65536)
	{
	  fprintf (stderr,
		   "Table_Option->%s->one_of[%i]:\n Only 65536 options "
		   "allowed in Table_Option.\n\n",
		   entry->name, field->max_element_index + 1);
	  customization_errors++;
	}
    }

}


void
check_bidirectional_links (MD * md, char *section_name, char *field_name)
{
  MD_Section *section;
  MD_Field_Decl *field_decl;
  MD_Entry *src_entry, *dest_entry, *target_entry;
  MD_Field *src_field, *dest_field;
  int i, j;

  /* Get the section we are converting */
  if ((section = MD_find_section (md, section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", section_name);

  /* Get the field we are checking the bidirectinoal links of */
  field_decl = MD_find_field_decl (section, field_name);
  if (field_decl == NULL)
    {
      C_punt (md, "Field '%s' in '%s' section not found.", field_name,
	      section_name);
    }

  /* Expect field to be a kleene starred link to this section */
  if ((field_decl->max_require_index > 1) ||
      (field_decl->kleene_starred_req == NULL) ||
      (field_decl->require[0]->type != MD_LINK) ||
      (field_decl->require[0]->link_array_size != 1) ||
      (field_decl->require[0]->link[0] != section))
    {
      C_punt (md,
	      "Error customizing field '%s->%s':\nExpected a single "
	      "kleene_starred link to section '%s'.",
	      section_name, field_name, section_name);
    }

  /* Finally, check to make sure every entry that is linked to, has
   * a link back to the entry.
   */
  for (src_entry = MD_first_entry (section); src_entry != NULL;
       src_entry = MD_next_entry (src_entry))
    {
      /* Get the src_field in order to read the links */
      src_field = MD_find_field (src_entry, field_decl);


      /* Process the links in the src field if specified */
      if (src_field != NULL)
	{
	  for (i = 0; i <= src_field->max_element_index; i++)
	    {
	      /* Get the target of this link */
	      dest_entry = MD_get_link (src_field, i);

	      /* Must not be NULL */
	      if (dest_entry == NULL)
		{
		  C_punt (md, "%s->%s->%s[i] NULL, not a link as expected.",
			  section->name, src_entry->name, field_name, i);
		}

	      /* Get the value field from the value entry */
	      dest_field = MD_find_field (dest_entry, field_decl);


	      target_entry = NULL;
	      /* Find link back to src_entry */
	      for (j = 0; j <= dest_field->max_element_index; j++)
		{
		  /* Get link target */
		  target_entry = MD_get_link (dest_field, j);

		  /* If target matches src, then everything ok, stop */
		  if (target_entry == src_entry)
		    {
		      break;
		    }
		}

	      /* Error if target_entry != src_entry */
	      if (target_entry != src_entry)
		{
		  fprintf (stderr,
			   "%s->%s->%s[%i]:\n  Specifies '%s' but\n  '%s->%s' "
			   "does not specify '%s'\n\n",
			   section_name, src_entry->name, field_name, i,
			   dest_entry->name, section_name, dest_entry->name,
			   src_entry->name);
		  customization_errors++;
		}
	    }
	}
    }
}

MD_Section *
CU_find_section (MD * md, char *section_name)
{
  MD_Section *section;

  if ((section = MD_find_section (md, section_name)) == NULL)
    C_punt (md, "'%s' section expected in hmdes2.", section_name);

  return (section);
}

MD_Field_Decl *
CU_find_field_decl (MD_Section * section, char *field_name)
{
  MD_Field_Decl *field_decl;

  field_decl = MD_find_field_decl (section, field_name);
  if (field_decl == NULL)
    C_punt (section->md,
	    "Field '%s' in '%s' section not found.", field_name,
	    section->name);

  return (field_decl);
}

MD_Field *
CU_find_field (MD_Entry * entry, MD_Field_Decl * field_decl)
{
  MD_Field *field;

  /* Get the one_of field */
  if ((field = MD_find_field (entry, field_decl)) == NULL)
    {
      C_punt (entry->section->md,
	      "%s->%s->%s: field expected.", entry->section->name,
	      entry->name, field_decl->name);
    }
  return (field);
}

MD_Field *
CU_find_field_by_name (MD_Entry * entry, char *field_name)
{
  MD_Field_Decl *field_decl;
  MD_Field *field;

  field_decl = CU_find_field_decl (entry->section, field_name);
  field = MD_find_field (entry, field_decl);

  return (field);
}

/* Creates a unique name from the base_name by tacking on ~#.
 * Scans md to make sure this name has not been used before.
 * OLD METHOD: Tries to pick the lowest number that has not been used
 *             (I think the result is easier to read than using an ever
 *              increasing number (which would have less conflicts)) 
 *
 * NEW METHOD: The old method takes way too long for really complicated
 *             machine descriptions (runs for days).  Try to keep a
 *             counter for each base name.
 *
 * I want to be able to renaming on any entry the customizer modifies/creates.
 * So I am passing the entry to be renamed to allow the entry to get the
 * "same" name as it currently has (if it has been already touched).
 * So bla will become bla~1 and stay there.  bla~2 will only be created
 * if bla (or bla~1) is duplicated.  Set 'entry' to NULL if want to
 * force a renaming.
 */
void
CU_get_unique_name (MD * md, MD_Entry * entry, char *base_name,
		    char *new_name)
{
  int rename_id;
  MD_Section *section;
  MD_Entry *conflicting_entry;
  STRING_Symbol *rename_symbol;
  int unique;

  /* Get the last renaming id for this base_name, if none, 
   * start at 1.
   */
  if ((rename_symbol = STRING_find_symbol (rename_table, base_name)) == NULL)
    {
      rename_symbol = STRING_add_symbol (rename_table, base_name, (void *) 1);
    }

  /* Use the last free number for this symbol */
#ifdef LP64_ARCHITECTURE
  rename_id = (int)((long) rename_symbol->data);
#else
  rename_id = (int) rename_symbol->data;
#endif

  /* Keep renaming until find a unique name in the md */
  unique = 0;
  while (!unique)
    {
      /* Create new name */
      sprintf (new_name, "%s~%i", base_name, rename_id);
      rename_id++;

      /* Search md for same name, assume unique to start with */
      unique = 1;
      for (section = MD_first_section (md); section != NULL;
	   section = MD_next_section (section))
	{
	  /* Name has conflict if entry already exists with this
	   * name and it is not the entry we are renaming.
	   * 'entry' may be NULL if want to force renaming.
	   */
	  conflicting_entry = MD_find_entry (section, new_name);
	  if ((conflicting_entry != NULL) && (conflicting_entry != entry))
	    {
	      /* Debug */
#if 0
	      printf ("Renaming conflicts with %s->%s\n",
		      section->name, new_name);
#endif

	      /* Conflict found, try another name */
	      unique = 0;
	      break;
	    }
	}
    }

  /* Make sure we haven't gone past 2 billion (will run out of bits soon) */
  if (rename_id > 2000000000)
    {
      I_punt ("CU_get_unique_name: id too big (%i)\n", rename_id);
    }

  /* Update the symbol entry */
#if LP64_ARCHITECTURE
  rename_symbol->data = (void *)((long) rename_id);
#else
  rename_symbol->data = (void *) rename_id;
#endif
}

MD_Entry *
CU_dup_entry (MD_Entry * orig_entry)
{
  char new_name[4000], *orig_name;	/* Assume sane sized names */
  MD_Section *section;
  MD_Entry *new_entry;
  MD_Field_Decl *field_decl;
  MD_Field *orig_field, *new_field;
  MD_Element *element;
  int index;

  /* Get this entry's section for ease of use */
  section = orig_entry->section;

  /* Use the name in orig_entry->original_name if exists.
   * This prevents getting bla~1~1~1~1... after multiple dups.
   */
  orig_name = orig_entry->name;
  if ((field_decl = MD_find_field_decl (section, "original_name")) != NULL)
    {
      if ((orig_field = MD_find_field (orig_entry, field_decl)) != NULL)
	{
	  orig_name = MD_get_string (orig_field, 0);
	}
    }

  /* Get a new name for the new entry (force a new name) */
  CU_get_unique_name (section->md, NULL, orig_name, new_name);

  /* Create new entry in this section */
  new_entry = MD_new_entry (section, new_name);

  /* Copy all the field information to the new entry */
  for (field_decl = MD_first_field_decl (section); field_decl != NULL;
       field_decl = MD_next_field_decl (field_decl))
    {
      /* Does this field exist in the original? If not, go to next one */
      if ((orig_field = MD_find_field (orig_entry, field_decl)) == NULL)
	continue;

      /* Create this field in new entry */
      new_field = MD_new_field (new_entry, field_decl, 0);

      /* Copy the field's contents */
      for (index = 0; index <= orig_field->max_element_index; index++)
	{
	  /* Copy element only if not NULL */
	  if ((element = orig_field->element[index]) != NULL)
	    {
	      switch (element->type)
		{
		case MD_INT:
		  MD_set_int (new_field, index,
			      MD_get_int (orig_field, index));
		  break;

		case MD_DOUBLE:
		  MD_set_double (new_field, index,
				 MD_get_double (orig_field, index));
		  break;

		case MD_STRING:
		  MD_set_string (new_field, index,
				 MD_get_string (orig_field, index));
		  break;

		case MD_LINK:
		  MD_set_link (new_field, index,
			       MD_get_link (orig_field, index));
		  break;

		default:
		  C_punt (section->md,
			  "CU_dup_entry: Unknown element type %i",
			  element->type);
		}
	    }
	}
    }


  /* Create (if necessary) a orignal_name field to track where this new entry
   * came from.  Punt if orignal_name field declaration is not
   * what we expect.
   */
  if ((field_decl = MD_find_field_decl (section, "original_name")) == NULL)
    {
      /* Create an optional 'original_name' field that takes one STRING */
      field_decl = MD_new_field_decl (section, "original_name",
				      MD_OPTIONAL_FIELD);
      MD_require_string (field_decl, 0);
    }

  /* Check that the field is what we expected 
   * (don't put in else to make sure this check is consistent with the
   *  above field creation)
   */
  if ((field_decl->max_require_index != 0) ||
      (field_decl->type != MD_OPTIONAL_FIELD) ||
      (field_decl->kleene_starred_req != NULL) ||
      (field_decl->require[0]->type != MD_STRING))
    {
      fprintf (stderr,
	       "CU_dup_entry: Field %s->%s is not what is expected:\n",
	       section->name, field_decl->name);
      MD_print_field_decl (stderr, field_decl, 80);
      C_punt (section->md,
	      "Please change mdes or change field used (%s) by customizer.\n",
	      field_decl->name);
    }

  /* Set original name (if necessary). 
   * Keep original name from orig_entry if there is one.
   */
  if (MD_find_field (new_entry, field_decl) == NULL)
    {
      new_field = MD_new_field (new_entry, field_decl, 1);
      MD_set_string (new_field, 0, orig_entry->name);
    }

  return (new_entry);
}

void
CU_rename_entry (MD_Entry * entry, char *new_name)
{
  char unique_name[4000];	/* Assume sane sized names */
  MD_Section *section;
  MD_Field_Decl *field_decl;
  MD_Field *orig_field, *new_field;

  /* Get this entry's section for ease of use */
  section = entry->section;

  /* Use new_name as starting point if not NULL, otherwise
   * use the entry's original name as a starting point.
   */
  if (new_name == NULL)
    {
      /* Use the name in entry->original_name if exists.
       * This prevents getting bla~1~1~1~1... after multiple dups/renames.
       */
      new_name = entry->name;
      field_decl = MD_find_field_decl (section, "original_name");
      if (field_decl != NULL)
	{
	  if ((orig_field = MD_find_field (entry, field_decl)) != NULL)
	    {
	      new_name = MD_get_string (orig_field, 0);
	    }
	}
    }

  /* Get a new name for the new entry */
  CU_get_unique_name (section->md, entry, new_name, unique_name);

  /* Create (if necessary) a orignal_name field to track what this entry
   * was originally named.  Punt if orignal_name field declaration is not
   * what we expect.
   */
  if ((field_decl = MD_find_field_decl (section, "original_name")) == NULL)
    {
      /* Create an optional 'original_name' field that takes one STRING */
      field_decl = MD_new_field_decl (section, "original_name",
				      MD_OPTIONAL_FIELD);
      MD_require_string (field_decl, 0);
    }

  /* Check that the field is what we expected 
   * (don't put in else to make sure this check is consistent with the
   *  above field creation)
   */
  if ((field_decl->max_require_index != 0) ||
      (field_decl->type != MD_OPTIONAL_FIELD) ||
      (field_decl->kleene_starred_req != NULL) ||
      (field_decl->require[0]->type != MD_STRING))
    {
      fprintf (stderr,
	       "CU_rename_entry: Field %s->%s is not what is expected:\n",
	       section->name, field_decl->name);
      MD_print_field_decl (stderr, field_decl, 80);
      C_punt (section->md,
	      "Please change mdes or change field used (%s) by customizer.\n",
	      field_decl->name);
    }

  /* Set original name (if necessary). 
   * Keep original name from entry if there is one.
   */
  if (MD_find_field (entry, field_decl) == NULL)
    {
      new_field = MD_new_field (entry, field_decl, 1);
      MD_set_string (new_field, 0, entry->name);
    }

  /* Rename the entry */
  MD_rename_entry (entry, unique_name);
}

/* Adds link at the given index, shifting all links at or after that
 * index to the right.
 */
void
CU_add_link_at (MD_Field * field, int at_index, MD_Entry * new_link)
{
  MD_Element *element;
  MD_Entry *target_entry;
  int index;

  /* Shift the elements to the right starting at at_index */
  for (index = field->max_element_index; index >= at_index; index--)
    {
      /* Don't expect element to be NULL */
      if ((element = field->element[index]) == NULL)
	{
	  C_punt (field->entry->section->md,
		  "CU_add_link_at: NULL links in list not supported!");
	}

      /* Must be a link */
      if (element->type != MD_LINK)
	{
	  C_punt (field->entry->section->md,
		  "CU_add_link_at: Link expected not type '%i'",
		  element->type);
	}

      target_entry = MD_get_link (field, index);
      MD_set_link (field, index + 1, target_entry);
    }

  /* Set the element at the list the new link */
  MD_set_link (field, at_index, new_link);
}

/* Deletes link at the given index, shifting all links after that
 * index to the left (so there is no hole left).
 */
void
CU_delete_link_at (MD_Field * field, int at_index)
{
  MD_Element *element;
  MD_Entry *target_entry;
  int index;

  /* Better be a link at the index specified */
  if (at_index > field->max_element_index)
    {
      C_punt (field->entry->section->md,
	      "CU_delete_link_at: at_index %i out of bounds!", at_index);
    }

  if ((element = field->element[at_index]) == NULL)
    {
      C_punt (field->entry->section->md,
	      "CU_delete_link_at: Attempting to delete NULL link at index %i!",
	      at_index);
    }
  if (element->type != MD_LINK)
    {
      C_punt (field->entry->section->md,
	      "CU_delete_link_at: %s->%s->%s[%i] not a link!",
	      field->entry->section->name, field->entry->name,
	      field->decl->name, at_index);
    }

  /* Shift the elements to the left starting at at_index+1 */
  for (index = at_index + 1; index <= field->max_element_index; index++)
    {
      /* Don't expect element to be NULL */
      if ((element = field->element[index]) == NULL)
	{
	  C_punt (field->entry->section->md,
		  "CU_delete_link_at: NULL links in list not supported!");
	}

      /* Must be a link */
      if (element->type != MD_LINK)
	{
	  C_punt (field->entry->section->md,
		  "CU_delete_link_at: Link expected not type '%i'",
		  element->type);
	}

      target_entry = MD_get_link (field, index);
      MD_set_link (field, index - 1, target_entry);
    }

  /* Delete the link at the end of the list (it is now a duplicate) */
  MD_delete_element (field, field->max_element_index);
}

/* Duplicates a Scheduling_Alternative, modifying any Operations pointing
 * at this alternative so that the new alternative is placed immediately
 * after the original in the operation's alt list.
 *
 * Rename (add ~#) both the original and new alts to show that the customizer
 * may have changed their behavior.
 */
MD_Entry *
CU_dup_alt (MD_Entry * orig_alt)
{
  MD_Entry *new_alt, *operation_entry, *test_alt;
  MD_Section *operation_section;
  MD_Field_Decl *alt_field_decl;
  MD_Field *alt_field;
  int index;

  /* Make sure really passed a scheduling alt */
  if (strcmp (orig_alt->section->name, "Scheduling_Alternative") != 0)
    {
      C_punt (orig_alt->section->md,
	      "CU_dup_alt:\n  Expecting Scheduling_Alternative entry "
	      "not '%s->%s'",
	      orig_alt->section->name, orig_alt->name);
    }

  /* Rename the original alt (add ~#) */
  CU_rename_entry (orig_alt, NULL);

  /* Create the new alt */
  new_alt = CU_dup_entry (orig_alt);

  /* Get the operation section and it's alt field declaration */
  operation_section = CU_find_section (orig_alt->section->md, "Operation");
  alt_field_decl = CU_find_field_decl (operation_section, "alt");

  /* Scan the Operation section for uses of the orig_alt */
  for (operation_entry = MD_first_entry (operation_section);
       operation_entry != NULL;
       operation_entry = MD_next_entry (operation_entry))
    {
      /* Get the alt field */
      alt_field = MD_find_field (operation_entry, alt_field_decl);

      /* Scan alt field for matches */
      for (index = 0; index <= alt_field->max_element_index; index++)
	{
	  test_alt = MD_get_link (alt_field, index);

	  if (orig_alt == test_alt)
	    {
	      /* Make a place for the new alt in the alt field,
	       * and place the new_alt there
	       */
	      CU_add_link_at (alt_field, index + 1, new_alt);
	    }
	}
    }

  return (new_alt);
}

/* Duplicates a Reservation_Table entry so that the original entry then the
 * new entry will be attempted when scheduling.  This will cause the
 * relevent scheduling alts to be duplicated and the relevent operation
 * alt lists to be adjusted.
 *
 * Rename (add ~#) both the original and new alts to show that the customizer
 * may have changed their behavior.
 */
MD_Entry *
CU_dup_res_table (MD_Entry * orig_res_table)
{
  MD_Entry *new_res_table, *orig_alt_entry, *new_alt_entry, *test_res_table;
  MD_Section *alt_section;
  MD_Field_Decl *resv_field_decl;
  MD_Field *resv_field;

  /* Make sure really passed a reservation table entry */
  if (strcmp (orig_res_table->section->name, "Reservation_Table") != 0)
    {
      C_punt (orig_res_table->section->md,
	      "CU_dup_res_table:\n  Expecting Reservation_Table entry "
	      "not '%s->%s'",
	      orig_res_table->section->name, orig_res_table->name);
    }

  /* Rename the original reseravation table (add ~#) */
  CU_rename_entry (orig_res_table, NULL);

  /* Create the new res table */
  new_res_table = CU_dup_entry (orig_res_table);

  /* Get the operation section and it's alt field declaration */
  alt_section = CU_find_section (orig_res_table->section->md,
				 "Scheduling_Alternative");
  resv_field_decl = CU_find_field_decl (alt_section, "resv");

  /* Scan the alt section for uses of the orig_res_table */
  for (orig_alt_entry = MD_first_entry (alt_section);
       orig_alt_entry != NULL;
       orig_alt_entry = MD_next_entry (orig_alt_entry))
    {
      /* Get the resv field */
      resv_field = MD_find_field (orig_alt_entry, resv_field_decl);

      /* Does this alt point at the res_table */
      test_res_table = MD_get_link (resv_field, 0);
      if (orig_res_table == test_res_table)
	{
	  /* If so, duplicate the alt entry and point at new res_table */
	  new_alt_entry = CU_dup_alt (orig_alt_entry);
	  resv_field = MD_find_field (new_alt_entry, resv_field_decl);
	  MD_set_link (resv_field, 0, new_res_table);
	}
    }

  return (new_res_table);
}

/* Duplicates a Resource Usage entry so that both the original entry then the
 * new entry will be required when scheduling.  This will cause the
 * relevent resource_unit->use() fields to be adjusted.  This must
 * be run after CU_homogenize_table_options() and 
 * CU_consolidate_res_table_usages() to prevent resource usages from
 * appearing in table options or res tables!  
 *
 * Rename (add ~#) both the original and new rus to show that the customizer
 * may have changed their behavior.
 */
MD_Entry *
CU_dup_resource_usage (MD_Entry * orig_ru)
{
  MD_Entry *new_ru, *unit_entry, *test_ru;
  MD_Section *unit_section;
  MD_Field_Decl *use_field_decl;
  MD_Field *use_field;
  int index;

  /* Make sure really passed a resource usage entry */
  if (strcmp (orig_ru->section->name, "Resource_Usage") != 0)
    {
      C_punt (orig_ru->section->md,
	      "CU_dup_resource_usage:\n"
	      "  Expecting Reservation_Table entry not '%s->%s'",
	      orig_ru->section->name, orig_ru->name);
    }

  /* Rename the original resource usage (add ~#) */
  CU_rename_entry (orig_ru, NULL);

  /* Create the new resource usage */
  new_ru = CU_dup_entry (orig_ru);

  /* Get the resource unit section and it's use field declaration */
  unit_section = CU_find_section (orig_ru->section->md, "Resource_Unit");
  use_field_decl = CU_find_field_decl (unit_section, "use");

  /* Scan the resource unit section for uses of the orig_ru */
  for (unit_entry = MD_first_entry (unit_section); unit_entry != NULL;
       unit_entry = MD_next_entry (unit_entry))
    {
      /* Get the use field */
      use_field = MD_find_field (unit_entry, use_field_decl);

      /* Scan use field for uses of orig_ru, scan backwards so we
       * can easily add new_ru if found.
       */
      for (index = use_field->max_element_index; index >= 0; index--)
	{
	  /* Does this use[index]t point at the orig_ru */
	  test_ru = MD_get_link (use_field, index);
	  if (orig_ru == test_ru)
	    {
	      /* If so, add new_ru after it */
	      CU_add_link_at (use_field, index + 1, new_ru);

#if 0
	      /* Debug */
	      printf ("  Adding '%s' to %s->use[%i]\n",
		      new_ru->name, unit_entry->name, index + 1);
#endif
	    }
	}
    }

  /* Need to scan if there are links from somewhere else to this orig_ru! */

  /* Return the new_ru */
  return (new_ru);
}

/* Returns the smallest slot used in time 0, or -1 if no slot found
 * (slots may not be negative).
 */
int
CU_get_slot (MD_Entry * entry)
{
  MD_Field *one_of_field, *use_field, *time_field, *slot_field;
  MD_Field_Decl *slot_field_decl;
  MD_Entry *sub_entry, *resource_entry;
  int index, time;
  int entry_slot, sub_entry_slot;

  /* Initially, no slot is specified */
  entry_slot = -1;

  /* Recurse until entry is a Resource_Usage entry */

  /* Handle if entry is a Table_Option */
  if (strcmp (entry->section->name, "Table_Option") == 0)
    {
      /* Overlaps if any option overlaps */
      one_of_field = CU_find_field_by_name (entry, "one_of");
      for (index = 0; index <= one_of_field->max_element_index; index++)
	{
	  sub_entry = MD_get_link (one_of_field, index);
	  sub_entry_slot = CU_get_slot (sub_entry);

	  /* Update entry_slot if necessary */
	  if (sub_entry_slot != -1)
	    {
	      if ((entry_slot == -1) || (sub_entry_slot < entry_slot))
		entry_slot = sub_entry_slot;
	    }
	}

      /* Return the smallest slot found (or -1 if no slot found) */
      return (entry_slot);
    }

  /* Handle if entry is a Resource_Unit */
  else if (strcmp (entry->section->name, "Resource_Unit") == 0)
    {
      /* Overlaps if any resource usage overlaps */
      use_field = CU_find_field_by_name (entry, "use");
      for (index = 0; index <= use_field->max_element_index; index++)
	{
	  sub_entry = MD_get_link (use_field, index);
	  sub_entry_slot = CU_get_slot (sub_entry);

	  /* Update entry_slot (if necessary) */
	  if (sub_entry_slot != -1)
	    {
	      if ((entry_slot == -1) || (sub_entry_slot < entry_slot))
		entry_slot = sub_entry_slot;
	    }
	}
      /* Return the smallest slot found (or -1 if no slot found) */
      return (entry_slot);
    }

  /* Entry1 better be a Resource Usage entry if we got here */
  else if (strcmp (entry->section->name, "Resource_Usage") != 0)
    {
      C_punt (entry->section->md,
	      "CU_get_slot:\n  Expecting Resource_Usage entry not '%s->%s'",
	      entry->section->name, entry->name);

      /* To make lint happy */
      return (-1);
    }

  /* At this point, entry is a resource usage.  Get slot info if any 
   * for a usage at time 0
   */

  /* Get the slot used (if any) by this resource usage */
  use_field = CU_find_field_by_name (entry, "use");

  /* Get the minimum slot used the the use_field */
  entry_slot = -1;
  for (index = 0; index <= use_field->max_element_index; index++)
    {
      resource_entry = MD_get_link (use_field, index);
      slot_field_decl = CU_find_field_decl (resource_entry->section, "slot");
      slot_field = MD_find_field (resource_entry, slot_field_decl);
      if (slot_field == NULL)
	continue;

      /* Get the slot that is used (if the resource is used at time 0) */
      sub_entry_slot = MD_get_int (slot_field, 0);

      /* Set entry_slot if not previous set or this sub_slot is smaller */
      if ((entry_slot == -1) || (sub_entry_slot < entry_slot))
	entry_slot = sub_entry_slot;
    }

  /* Return -1 now if no slot specified */
  if (entry_slot == -1)
    return (-1);

  /* Use this slot if time 0 appears in time field */
  time_field = CU_find_field_by_name (entry, "time");
  for (index = 0; index <= time_field->max_element_index; index++)
    {
      time = MD_get_int (time_field, index);
      if (time == 0)
	return (entry_slot);

    }

  /* If we got here, the slot was not used at time 0 */
  return (-1);
}

/* Returns the minimum usage time of all the resources usages in this entry
 * (may be negative)
 */
int
CU_get_min_time (MD_Entry * entry)
{
  MD_Field *one_of_field, *use_field, *time_field;
  MD_Entry *sub_entry;
  int index, time, min_time, sub_min_time;

  /* Set min_time to very large number initially (just for sanity) */
  min_time = 1000000000;

  /* Recurse until entry is a Resource_Usage entry */

  /* Handle if entry is a Table_Option */
  if (strcmp (entry->section->name, "Table_Option") == 0)
    {
      one_of_field = CU_find_field_by_name (entry, "one_of");
      for (index = 0; index <= one_of_field->max_element_index; index++)
	{
	  sub_entry = MD_get_link (one_of_field, index);
	  sub_min_time = CU_get_min_time (sub_entry);

	  /* Update min_time if necessary */
	  if ((index == 0) || (sub_min_time < min_time))
	    min_time = sub_min_time;
	}

      /* Return the minimum usage time found */
      return (min_time);
    }

  /* Handle if entry is a Resource_Unit */
  else if (strcmp (entry->section->name, "Resource_Unit") == 0)
    {
      /* Overlaps if any resource usage overlaps */
      use_field = CU_find_field_by_name (entry, "use");
      for (index = 0; index <= use_field->max_element_index; index++)
	{
	  sub_entry = MD_get_link (use_field, index);
	  sub_min_time = CU_get_min_time (sub_entry);

	  /* Update min_time (if necessary) */
	  if ((index == 0) || (sub_min_time < min_time))
	    min_time = sub_min_time;
	}

      /* Return the minimum usage time found */
      return (min_time);
    }

  /* Entry1 better be a Resource Usage entry if we got here */
  else if (strcmp (entry->section->name, "Resource_Usage") != 0)
    {
      C_punt (entry->section->md,
	      "CU_get_min_time:\n  Expecting Resource_Usage entry "
	      "not '%s->%s'",
	      entry->section->name, entry->name);

      /* To make lint happy */
      return (-1);
    }

  /* At this point, entry is a resource usage.  Get minimum usage time */
  time_field = CU_find_field_by_name (entry, "time");
  for (index = 0; index <= time_field->max_element_index; index++)
    {
      time = MD_get_int (time_field, index);

      if ((index == 0) || (time < min_time))
	min_time = time;
    }

  /* Return the minimum usage time found */
  return (min_time);
}

/* Determines if two entries (Table_Option, Resource_Unit, or Resource_Usage) 
 * have an overlapping resource usage (could use a resource at the same time).
 * Returns 1 if so, 0 otherwise
 */
int
CU_overlapping_resource_usage (MD_Entry * entry1, MD_Entry * entry2)
{
  MD_Field *one_of_field1, *use_field1, *use_field2;
  MD_Field *time_field1, *time_field2;
  MD_Entry *sub_entry, *resource_entry1, *resource_entry2;
  int index1, index2, time1, time2;
  int resource_overlaps;

  /* Recurse until both entry1 and entry2 are Resource_Usage entries
   * Recurse on entry1 first, then when it is reduced, reduce 
   * entry2 by swapping places with entry1 and recursing.
   */
  /* Handle if entry1 is a Table_Option */
  if (strcmp (entry1->section->name, "Table_Option") == 0)
    {
      /* Overlaps if any option overlaps */
      one_of_field1 = CU_find_field_by_name (entry1, "one_of");
      for (index1 = 0; index1 <= one_of_field1->max_element_index; index1++)
	{
	  sub_entry = MD_get_link (one_of_field1, index1);
	  if (CU_overlapping_resource_usage (sub_entry, entry2))
	    return (1);
	}
      /* If got here, none of the options overlap */
      return (0);
    }

  /* Handle if entry1 is a Resource_Unit */
  else if (strcmp (entry1->section->name, "Resource_Unit") == 0)
    {
      /* Overlaps if any resource usage overlaps */
      use_field1 = CU_find_field_by_name (entry1, "use");
      for (index1 = 0; index1 <= use_field1->max_element_index; index1++)
	{
	  sub_entry = MD_get_link (use_field1, index1);
	  if (CU_overlapping_resource_usage (sub_entry, entry2))
	    return (1);
	}
      /* If got here, none of the resource usages overlap */
      return (0);
    }

  /* Entry1 better be a Resource Usage entry if we got here */
  else if (strcmp (entry1->section->name, "Resource_Usage") != 0)
    {
      C_punt (entry1->section->md,
	      "CU_overlapping_resource_usage:\n  Expecting Resource_Usage "
	      "entry not '%s->%s'",
	      entry1->section->name, entry1->name);

      /* To make lint happy */
      return (-1);
    }

  /* If Entry2 not a Resource Usage, swap entry1 and entry2 and recurse */
  else if (strcmp (entry2->section->name, "Resource_Usage") != 0)
    {
      return (CU_overlapping_resource_usage (entry2, entry1));
    }

  /* At this point, both entry1 and entry2 are resource usages
   * Compare the resource used and the time for overlap 
   */
  use_field1 = CU_find_field_by_name (entry1, "use");
  use_field2 = CU_find_field_by_name (entry2, "use");

  /* Determine if at least one of the resources overlap */
  resource_overlaps = 0;
  for (index1 = 0; index1 <= use_field1->max_element_index; index1++)
    {
      resource_entry1 = MD_get_link (use_field1, index1);
      for (index2 = 0; index2 <= use_field2->max_element_index; index2++)
	{
	  resource_entry2 = MD_get_link (use_field2, index2);

	  if (resource_entry1 == resource_entry2)
	    {
	      /* Flag that overlap detected */
	      resource_overlaps = 1;

#if 0
	      /* Debug */
	      printf ("%s->use[%i] overlaps %s->use[%i] for %s\n",
		      entry1->name, index1, entry2->name, index2,
		      resource_entry1->name);
#endif

	      /* Break out of loop */
	      index1 = use_field1->max_element_index + 1;
	      break;
	    }
	}
    }
  /* They don't overlap if they don't use the same resource */
  if (!resource_overlaps)
    return (0);

  time_field1 = CU_find_field_by_name (entry1, "time");
  time_field2 = CU_find_field_by_name (entry2, "time");

  /* They overlap if they have a common usage time */
  for (index1 = 0; index1 <= time_field1->max_element_index; index1++)
    {
      time1 = MD_get_int (time_field1, index1);
      for (index2 = 0; index2 <= time_field2->max_element_index; index2++)
	{
	  time2 = MD_get_int (time_field2, index2);

	  if (time1 == time2)
	    {
#if 0
	      /* Debug */
	      printf ("%s overlaps %s at time %i\n",
		      entry1->name, entry2->name, time1);
#endif
	      return (1);
	    }
	}
    }

  /* If we got here, they don't overlap */
  return (0);
}

int
CU_make_res_table_orthogonal (MD_Entry * res_table)
{
  MD_Field *use_field, *new_use_field, *one_of_field;
  MD_Entry *entry1, *entry2, *new_res_table, *target_entry, *last_table;
  int index1, index2, i, num_added;

  /* Initially, no entries have been added */
  num_added = 0;

  /* Make sure we are passed a Reservation_Table entry */
  if (strcmp (res_table->section->name, "Reservation_Table") != 0)
    {
      C_punt (res_table->section->md,
	      "CU_make_res_table_orthogonal: res table expected not '%s->%s'!",
	      res_table->section->name, res_table->name);
    }

  /* Search use field for overlapping resource usages */
  use_field = CU_find_field_by_name (res_table, "use");

  for (index1 = 0; index1 <= use_field->max_element_index; index1++)
    {
      entry1 = MD_get_link (use_field, index1);

      for (index2 = index1 + 1; index2 <= use_field->max_element_index;
	   index2++)
	{
	  entry2 = MD_get_link (use_field, index2);

	  /* Only process entries that overlap */
	  if (!CU_overlapping_resource_usage (entry1, entry2))
	    continue;

#if 0
	  /* Debug */
	  printf
	    ("%s->%s needs expansion:\n  %i:%s->%s overlaps\n   %i:%s->%s\n\n",
	     res_table->section->name, res_table->name, index1,
	     entry1->section->name, entry1->name, index2,
	     entry2->section->name, entry2->name);
#endif

	  /* If entry1 is a table option, break up into reservation
	   * tables, one for each option, and recurse
	   */
	  if (strcmp (entry1->section->name, "Table_Option") == 0)
	    {
	      /* Set to NULL so we don't use without initializing */
	      last_table = NULL;

	      /* Get the one_of_field */
	      one_of_field = CU_find_field_by_name (entry1, "one_of");

	      /* Create a new res_table for each entry in the one_of
	       * field, recurse and process these res_tables
	       */
	      for (i = 0; i <= one_of_field->max_element_index; i++)
		{
		  /* Get the target of this option */
		  target_entry = MD_get_link (one_of_field, i);

		  /* Use this res_table for the first option and 
		   * create a new res tables for the rest
		   */
		  if (i == 0)
		    {
		      /* Make this res table reflect the choice of the
		       * first option.
		       */
		      new_res_table = res_table;

		      /* This option is now entry1 */
		      entry1 = target_entry;

		      /* Rename (add ~#) to show customizer has modified */
		      CU_rename_entry (res_table, NULL);
		    }
		  else
		    {
		      /* Duplicate the last res table processed so that
		       * it will be placed after it in the alt() field.
		       * Required to keep proper ordering!
		       */
		      new_res_table = CU_dup_res_table (last_table);
		      num_added++;
		    }

		  /* Set last table for next iteration */
		  last_table = new_res_table;

		  /* Get the use field for the new res table */
		  new_use_field =
		    CU_find_field_by_name (new_res_table, "use");

		  /* Change the option to the suboption in this use */
		  MD_set_link (new_use_field, index1, target_entry);

		  /* Do not recurse here on the new entries created!!!
		   * We are copying res_table (which we modifiy slightly 
		   * for i==0), and recursing destroys necessary info
		   * before we have made the other copys! -JCG
		   */
		}

	      /* Recurse on this modified res_table to handle stripping
	       * out the conflicting uses below and to handle any
	       * other new res tables that need to be created.
	       *
	       * Need to externally call this function for the entries
	       * created, since they are probably not orthoginal.
	       */
	      num_added += CU_make_res_table_orthogonal (res_table);
	      return (num_added);
	    }

	  /* entry1 resources are always used (not a table option),
	   * remove the table options from entry2 that conflict with 
	   * entry1
	   */
	  else if (strcmp (entry2->section->name, "Table_Option") == 0)
	    {
	      MD_Entry *new_option;

	      /* Create a new option entry that we can modify */
	      new_option = CU_dup_entry (entry2);

	      /* Get the one_of_field */
	      one_of_field = CU_find_field_by_name (new_option, "one_of");

	      /* Delete the options that conflict (overlap) with entry1.
	       * Scan backwards since we are deleting entries.
	       */
	      for (i = one_of_field->max_element_index; i >= 0; i--)
		{
		  /* Get the target of this option */
		  target_entry = MD_get_link (one_of_field, i);

		  /* Delete it if it conflicts (overlaps) with entry1 */
		  if (CU_overlapping_resource_usage (entry1, target_entry))
		    {
		      CU_delete_link_at (one_of_field, i);
		    }
		}

	      /* If we end up with an empty option, they have written
	       * their mdes strangely so that depending on the options
	       * picked early, it is not possible to schedule the operation!
	       */
	      if (one_of_field->max_element_index < 0)
		{
		  fprintf (stderr,
			   "Error:  %s->%s is not possible to schedule!\n"
			   "After expansion, it has the following form:\n\n",
			   res_table->section->name, res_table->name);

		  MD_print_entry (stderr, res_table, 80);

		  fprintf (stderr,
			   "\n%s->use[%i] (%s) cannot be satisified:\n\n",
			   res_table->name, index2, entry2->name);

		  MD_print_entry (stderr, entry2, 80);

		  C_punt (res_table->section->md,
			  "Please rewrite this reservation table "
			  "(or the table this entry\nwas derived from) "
			  "so that it is possible to schedule!\n");
		}

	      /* Place the new set of options at index2,
	       * If only one option is left, then just place that
	       * option at index2
	       */
	      if (one_of_field->max_element_index != 0)
		{
		  MD_set_link (use_field, index2, new_option);
		}
	      else
		{
		  /* Get the single target of this option */
		  target_entry = MD_get_link (one_of_field, 0);
		  MD_set_link (use_field, index2, target_entry);

		  /* Don't need the new option now */
		  MD_delete_entry (new_option);
		}
	    }

	  /* If entry2 is not a table option, then the scheduler
	   * will never be able to schedule this reservation table!
	   */
	  else
	    {
	      fprintf (stderr,
		       "Error:  %s->%s is not possible to schedule!\n"
		       "After expansion, it has the following form:\n\n",
		       res_table->section->name, res_table->name);

	      MD_print_entry (stderr, res_table, 80);

	      fprintf (stderr,
		       "\n%s->use[%i] (%s) cannot be satisified:\n\n",
		       res_table->name, index2, entry2->name);

	      MD_print_entry (stderr, entry2, 80);

	      C_punt (res_table->section->md,
		      "Please rewrite this reservation table "
		      "(or the table this entry\nwas derived from) "
		      "so that it is possible to schedule!\n");
	    }
	}
    }

  /* If got here, no entries were added becuase of this call */
  return (0);
}

/* Groups all resource usages that are not part of a table option
 * into one consolidated unit (for better scheduling efficiency).
 * Will convert table options with just one option into an unconditional
 * resource usage.
 * Will reorder/add/remove links in res_table->use().  
 * Need to run after CU_make_res_table_orthoginal
 * and before specifying which choice specifies the slot!
 */
void
CU_consolidate_res_table_usages (MD_Entry * res_table)
{
  MD_Section *resource_unit_section;
  MD_Field *rt_use_field, *new_use_field, *old_use_field, *one_of_field;
  MD_Field_Decl *use_field_decl;
  MD_Entry *entry1, *new_unit, *target_entry;
  int index1, index2, unit_count, ru_count, consolidated_index;
  char base_name[5000], unit_name[5000];

  /* Make sure we are passed a Reservation_Table entry */
  if (strcmp (res_table->section->name, "Reservation_Table") != 0)
    {
      C_punt (res_table->section->md,
	      "CU_consolidate_res_table_usages: res table expected "
	      "not '%s->%s'!",
	      res_table->section->name, res_table->name);
    }

  /* Search multiple unconditional use field to consolidate.
   * Convert any one option table option's to an unconditional usage
   */
  unit_count = 0;
  ru_count = 0;
  rt_use_field = CU_find_field_by_name (res_table, "use");
  for (index1 = 0; index1 <= rt_use_field->max_element_index; index1++)
    {
      /* Get the table option or unconditional usage */
      entry1 = MD_get_link (rt_use_field, index1);

      /* If it is a table option and has only one option, convert */
      if (strcmp (entry1->section->name, "Table_Option") == 0)
	{
	  /* Get the one_of_field */
	  one_of_field = CU_find_field_by_name (entry1, "one_of");

	  /* Convert to unconditional usage if has only one option */
	  if (one_of_field->max_element_index == 0)
	    {
	      target_entry = MD_get_link (one_of_field, 0);
	      MD_set_link (rt_use_field, index1, target_entry);

#if 0
	      /* Debug */
	      printf ("%s->use[%i] changed to %s\n", res_table->name,
		      index1, target_entry->name);
#endif

	      /* Counts as an unconditional usage */
	      if (strcmp (target_entry->section->name, "Resource_Unit") == 0)
		unit_count++;
	      else
		ru_count++;
	    }
	}
      /* Otherwise it is an unconditional usage */
      else if (strcmp (entry1->section->name, "Resource_Unit") == 0)
	unit_count++;
      else
	ru_count++;

    }

  /* Consolidate if have one or more resource usages or 
   * more than one unit or resource usages
   */
  if ((ru_count >= 1) || ((ru_count + unit_count) >= 2))
    {
      resource_unit_section = CU_find_section (res_table->section->md,
					       "Resource_Unit");

      sprintf (base_name, "U_%s", res_table->name);
      CU_get_unique_name (res_table->section->md, NULL, base_name, unit_name);

      new_unit = MD_new_entry (resource_unit_section, unit_name);

#if DEBUG_OPTI
      /* Debug */
      printf ("Consolidating %s (%i rus %i units) into %s.\n",
	      res_table->name, ru_count, unit_count, unit_name);
#endif

      use_field_decl = CU_find_field_decl (new_unit->section, "use");

      /* Create a new use field to fill with all the resource usages */
      new_use_field = MD_new_field (new_unit, use_field_decl, 0);

      /* Get the earliest index of the use we are replacing, so we
       * can place the consolidated unit in the use list there.
       */
      consolidated_index = 0;

      /* Scan links in reverse order since we will be deleting as we go */
      for (index1 = rt_use_field->max_element_index; index1 >= 0; index1--)
	{
	  /* Get the table option or unconditional usage */
	  entry1 = MD_get_link (rt_use_field, index1);

	  if (strcmp (entry1->section->name, "Resource_Unit") == 0)
	    {
	      /* Put all the resource usages in the resource unit
	       * into the new unit.
	       */
	      old_use_field = CU_find_field_by_name (entry1, "use");

	      /* Go in reverse order so added in the proper order to
	       * the new_use_field
	       */
	      for (index2 = old_use_field->max_element_index; index2 >= 0;
		   index2--)
		{
		  target_entry = MD_get_link (old_use_field, index2);

		  /* Add resource unit directly to new use field */
		  CU_add_link_at (new_use_field, 0, target_entry);
		}

	      /* Delete the link be just pulled into the new_use field */
	      CU_delete_link_at (rt_use_field, index1);

	      /* Will place consolidated unit at the earilest index
	       * of a use we have consolidated.
	       */
	      consolidated_index = index1;
	    }
	  else if (strcmp (entry1->section->name, "Resource_Usage") == 0)
	    {
	      /* Add resource unit directly to front of use list
	       * (since scanning in reverse order, will end up in 
	       *  proper order)
	       */
	      CU_add_link_at (new_use_field, 0, entry1);

	      /* Delete the link be just pulled into the new_use field */
	      CU_delete_link_at (rt_use_field, index1);

	      /* Will place consolidated unit at the earilest index
	       * of a use we have consolidated.
	       */
	      consolidated_index = index1;
	    }
	}

      /* Add new until to rt_use_field to replace all the indivual
       * resource usages and resource units.
       * Will place at index of earilest use that was consolidated.
       */
      CU_add_link_at (rt_use_field, consolidated_index, new_unit);
    }
}

int
usage_cycle_compatable (MD_Entry * usage_entry, MD_Entry * option_entry)
{
  MD_Entry *unit_entry, *ru_entry;
  MD_Field *time_field, *one_of_field, *use_field;
  int ru_time, usage_time, index1, index2;
  int found;

  /* Get the time this usage uses the resource */
  time_field = CU_find_field_by_name (usage_entry, "time");
  usage_time = MD_get_int (time_field, 0);

  /* Compatable if each option uses a resource at this time! */
  one_of_field = CU_find_field_by_name (option_entry, "one_of");
  for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
    {
      unit_entry = MD_get_link (one_of_field, index1);
      use_field = CU_find_field_by_name (unit_entry, "use");

      /* Check each use for this time, stop when find one */
      found = 0;
      for (index2 = 0; index2 <= use_field->max_element_index; index2++)
	{
	  ru_entry = MD_get_link (use_field, index2);
	  time_field = CU_find_field_by_name (ru_entry, "time");
	  ru_time = MD_get_int (time_field, 0);

	  if (ru_time == usage_time)
	    {
	      found = 1;
	      break;
	    }
	}
      /* Return 0 if this unit does not use a resource at this time */
      if (!found)
	return (0);
    }

  /* If got here, must be cycle compatable with usage */
  return (1);
}

int
usage_resource_compatable (MD_Entry * usage_entry, MD_Entry * option_entry)
{
  MD_Entry *unit_entry, *ru_entry, *usage_resource, *ru_resource;
  MD_Field *usage_use_field, *ru_use_field, *one_of_field, *unit_use_field;
  int index1, index2;
  int found;

  /* Get the time this usage uses the resource */
  usage_use_field = CU_find_field_by_name (usage_entry, "use");
  usage_resource = MD_get_link (usage_use_field, 0);

  /* Compatable if each option uses this resource at any time! */
  one_of_field = CU_find_field_by_name (option_entry, "one_of");
  for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
    {
      unit_entry = MD_get_link (one_of_field, index1);
      unit_use_field = CU_find_field_by_name (unit_entry, "use");

      /* Check each use for this time, stop when find one */
      found = 0;
      for (index2 = 0; index2 <= unit_use_field->max_element_index; index2++)
	{
	  ru_entry = MD_get_link (unit_use_field, index2);
	  ru_use_field = CU_find_field_by_name (ru_entry, "use");
	  ru_resource = MD_get_link (ru_use_field, 0);

	  if (ru_resource == usage_resource)
	    {
	      found = 1;
	      break;
	    }
	}
      /* Return 0 if this unit does not use a resource at this time */
      if (!found)
	return (0);
    }

  /* If got here, must be resource compatable with usage */
  return (1);
}

void
distribute_usage (MD_Field * table_use_field, int index,
		  MD_Entry * usage_entry)
{
  MD_Entry *old_option_entry, *new_option_entry, *unit_entry;
  MD_Entry *new_unit_entry;
  MD_Field *one_of_field, *use_field;
  int index1;

  /* Create a new option entry at this index so we can modify it */
  old_option_entry = MD_get_link (table_use_field, index);
  new_option_entry = CU_dup_entry (old_option_entry);
  MD_set_link (table_use_field, index, new_option_entry);

#if DEBUG_OPTI
  printf ("    Creating %s to distribute %s into\n",
	  new_option_entry->name, usage_entry->name);
#endif

  /* Add usage to each option */
  one_of_field = CU_find_field_by_name (new_option_entry, "one_of");
  for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
    {
      unit_entry = MD_get_link (one_of_field, index1);

      /* Duplicate this unit so we can modify it */
      new_unit_entry = CU_dup_entry (unit_entry);

      /* Set the one_of_field to the new unit */
      MD_set_link (one_of_field, index1, new_unit_entry);

#if DEBUG_OPTI
      printf ("      Placing %s into %s (index %i)\n",
	      usage_entry->name, new_unit_entry->name, index1);
#endif

      /* Get this new units use field */
      use_field = CU_find_field_by_name (new_unit_entry, "use");

      /* Add this usage to the end of the use field */
      MD_set_link (use_field, use_field->max_element_index + 1, usage_entry);
    }
}

void
CU_distribute_unconditional_usages (MD_Entry * table_entry)
{
  MD_Entry *target_entry, *unit_entry, *usage_entry, *option_entry;
  MD_Entry *new_unit_entry;
  MD_Field *table_use_field, *unit_use_field;
  int index1, index2, unit_index = 0, changes;

#if DEBUG_OPTI
  printf ("Distributing usages in %s\n", table_entry->name);
#endif

  /* Do nothing if there is only one choice */
  table_use_field = CU_find_field_by_name (table_entry, "use");
  if (table_use_field->max_element_index == 0)
    {
#if DEBUG_OPTI
      printf ("  Only one choice\n");
#endif
      return;
    }

  /* Find the consolidated unit entry (if any) in this table */
  unit_entry = NULL;
  for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
    {
      target_entry = MD_get_link (table_use_field, index1);

      if (strcmp (target_entry->section->name, "Resource_Unit") == 0)
	{
	  /* Better be only one unit in table (after consolidation)! */
	  if (unit_entry != NULL)
	    {
	      C_punt (NULL, "CU_distribute_unconditional_usages:\n"
		      "%s has two units! %s and %s",
		      table_entry->name, unit_entry->name,
		      target_entry->name);
	    }
	  unit_entry = target_entry;
	  unit_index = index1;
	}
    }

  /* Do nothing if there is is no uncoditional usages */
  if (unit_entry == NULL)
    {
#if DEBUG_OPTI
      printf ("  No unconditional usages\n");
#endif
      return;
    }

  /* Create a new unit entry, so we can change it's contents */
  new_unit_entry = CU_dup_entry (unit_entry);
  changes = 0;

  /* Try to find a home in one of the options for each usage in use.
   * First place resource usage in options that use resources in
   * the same cycle as the unit's resources.
   *
   * Process backward so we can delete links out of the unit_use if
   * a home is found.
   */
  unit_use_field = CU_find_field_by_name (new_unit_entry, "use");
  for (index1 = unit_use_field->max_element_index; index1 >= 0; index1--)
    {
      usage_entry = MD_get_link (unit_use_field, index1);

      /* Try to find a home in one of the options */
      for (index2 = 0; index2 <= table_use_field->max_element_index; index2++)
	{
	  option_entry = MD_get_link (table_use_field, index2);

	  /* The unit entry is not a table option */
	  if (option_entry == unit_entry)
	    continue;

	  if (usage_cycle_compatable (usage_entry, option_entry))
	    {
#if DEBUG_OPTI
	      printf ("%s is cycle compatable with %s\n",
		      usage_entry->name, option_entry->name);
#endif

	      distribute_usage (table_use_field, index2, usage_entry);

	      CU_delete_link_at (unit_use_field, index1);

	      changes++;
	      break;
	    }
#if DEBUG_OPTI
	  else
	    printf ("%s is not cycle compatable with %s\n",
		    usage_entry->name, option_entry->name);
#endif
	}
    }

  /* Try to find a home in one of the options for each remaining usage in
   * use.  Now try to place in options that use the same resource.
   * This is important when doing resource minimizations.
   *
   * Process backward so we can delete links out of the unit_use if
   * a home is found.
   */
  unit_use_field = CU_find_field_by_name (new_unit_entry, "use");
  for (index1 = unit_use_field->max_element_index; index1 >= 0; index1--)
    {
      usage_entry = MD_get_link (unit_use_field, index1);

      /* Try to find a home in one of the options */
      for (index2 = 0; index2 <= table_use_field->max_element_index; index2++)
	{
	  option_entry = MD_get_link (table_use_field, index2);

	  /* The unit entry is not a table option */
	  if (option_entry == unit_entry)
	    continue;

	  if (usage_resource_compatable (usage_entry, option_entry))
	    {
#if DEBUG_OPTI
	      printf ("%s is resource compatable with %s\n",
		      usage_entry->name, option_entry->name);
#endif

	      distribute_usage (table_use_field, index2, usage_entry);

	      CU_delete_link_at (unit_use_field, index1);

	      changes++;
	      break;
	    }
#if DEBUG_OPTI
	  else
	    printf ("%s is not resource compatable with %s\n",
		    usage_entry->name, option_entry->name);
#endif
	}
    }



  /* If the unit now has nothing in it, delete the entire unit from 
   * the table.
   */
  if (unit_use_field->max_element_index == -1)
    {
      CU_delete_link_at (table_use_field, unit_index);
      MD_delete_entry (new_unit_entry);
    }
  /* If the unit has changes, but is not empty, place new unit in table */
  else if (changes != 0)
    {
      MD_set_link (table_use_field, unit_index, new_unit_entry);
    }
  /* if the unit hasn't changed, leave the old unit alone and
   * delete the new one
   */
  else
    {
      MD_delete_entry (new_unit_entry);
    }
}

/* Change all the Table_Option->one_of() links so that they only use
 * resource units (Create resource units as necessary).  This makes
 * the mdes more uniform and facilates the bit field optimizations
 * we will be doing later.  This is transformation required in order 
 * to be used by IMPACT.
 */
void
CU_homogenize_table_options (MD_Entry * table_option)
{
  MD_Section *resource_unit_section;
  MD_Entry *target_entry, *new_unit;
  MD_Field_Decl *use_field_decl;
  MD_Field *one_of_field, *new_use_field;
  char base_name[5000], unit_name[5000];
  int index;

  /* Make sure we are passed a Table Option entry */
  if (strcmp (table_option->section->name, "Table_Option") != 0)
    {
      C_punt (table_option->section->md,
	      "CU_homogenize_table_options: table option expected "
	      "not '%s->%s'!",
	      table_option->section->name, table_option->name);
    }

  /* Get the one of field */
  one_of_field = CU_find_field_by_name (table_option, "one_of");

  /* Create a resource unit for each resource usage in the one_of field */
  for (index = 0; index <= one_of_field->max_element_index; index++)
    {
      target_entry = MD_get_link (one_of_field, index);

      /* Replace with resource unit if resource usage */
      if (strcmp (target_entry->section->name, "Resource_Usage") == 0)
	{
	  resource_unit_section = CU_find_section (table_option->section->md,
						   "Resource_Unit");

	  /* Create unit name from table option name and index */
	  sprintf (base_name, "U_%s-%i", table_option->name, index);

	  CU_get_unique_name (table_option->section->md, NULL, base_name,
			      unit_name);

	  new_unit = MD_new_entry (resource_unit_section, unit_name);

#if DEBUG_OPTI
	  /* Debug */
	  printf ("Homogenizing %s->one_of[%i] into %s.\n",
		  table_option->name, index, unit_name);
#endif


	  /* Create a new use field to fill with all the resource usages */
	  use_field_decl = CU_find_field_decl (new_unit->section, "use");
	  new_use_field = MD_new_field (new_unit, use_field_decl, 1);

	  /* Point unit's use field at the target entry */
	  MD_set_link (new_use_field, 0, target_entry);

	  /* Replace target_entry in table_option's one_of field with 
	   * new_unit.
	   */
	  MD_set_link (one_of_field, index, new_unit);
	}
    }
}

/* Homogenize the resource usages so there is only one resource and time 
 * specified for each resource usage.  This will allow them to
 * be easily combined for bit field optimizations
 */
void
CU_homogenize_resource_usages (MD_Entry * ru_entry)
{
  MD_Entry *new_ru_entry, *last_new, *target_resource;
  MD_Field *orig_use_field, *new_use_field;
  MD_Field *orig_time_field, *new_time_field;
  int index1, index2, target_time;

  orig_time_field = CU_find_field_by_name (ru_entry, "time");

  /* If have multiple times specified, create a new ru entry for
   * each time (except the first)
   */
  if (orig_time_field->max_element_index > 0)
    {
      /* Keep track of the entry to duplicate using last_new */
      last_new = ru_entry;
      for (index1 = 1; index1 <= orig_time_field->max_element_index; index1++)
	{
	  /* Get the time we are creating this ru entry for */
	  target_time = MD_get_int (orig_time_field, index1);

	  /* Create new ru entry */
	  new_ru_entry = CU_dup_resource_usage (last_new);

	  /* Duplicate this entry for the next time (if any) */
	  last_new = new_ru_entry;

	  /* Get the new time field */
	  new_time_field = CU_find_field_by_name (new_ru_entry, "time");

	  /* Delete all but the first time in the time field */
	  for (index2 = new_time_field->max_element_index; index2 >= 1;
	       index2--)
	    {
	      MD_delete_element (new_time_field, index2);
	    }

	  /* Set the first time in the time field to the target time */
	  MD_set_int (new_time_field, 0, target_time);

	  /* Delete this time from the original time field */
	  MD_delete_element (orig_time_field, index1);
	}
    }

  orig_use_field = CU_find_field_by_name (ru_entry, "use");

  /* If have multiple resources specified, create a new ru entry for
   * each resource (except the first)
   */
  if (orig_use_field->max_element_index > 0)
    {
      /* Keep track of the entry to duplicate using last_new */
      last_new = ru_entry;
      for (index1 = 1; index1 <= orig_use_field->max_element_index; index1++)
	{
	  /* Get the resource we are creating this ru entry for */
	  target_resource = MD_get_link (orig_use_field, index1);

	  /* Create new ru entry */
	  new_ru_entry = CU_dup_resource_usage (last_new);

	  /* Duplicate this entry for the next resource (if any) */
	  last_new = new_ru_entry;

	  /* Get the new use field */
	  new_use_field = CU_find_field_by_name (new_ru_entry, "use");

	  /* Delete all but the first resource in the use field */
	  for (index2 = new_use_field->max_element_index; index2 >= 1;
	       index2--)
	    {
	      MD_delete_element (new_use_field, index2);
	    }

	  /* Set the first resource in the use field to the target resource */
	  MD_set_link (new_use_field, 0, target_resource);

	  /* Delete this resource from the original use field */
	  MD_delete_element (orig_use_field, index1);
	}
    }
}

    /* Shift the resource times that no resource usage time is less than 0. 

     * For opti_level < 3, shift all times by the same amount, so the
     * usages maintain the same relationship as the mdes specified.
     * This is equivalent to changing the point chosen for time "zero".
     *
     * For opti_level >= 3, try to minimize to number of different times 
     * that resources are used to try to maximize the effect of the bit 
     * field optimizations.
     * As a first try, I will just move each resource usage so that it's 
     * earlies usage is always time 0.
     * May want to try other methods to shifting times to see if can find
     * a better method (zero-opti seems to work best for our mdeses).
     * 
     * Assumes CU_homogenize_resource_usages() has been run on all
     * the ru entries.
     * 
     */
void
CU_optimize_resource_usage_times (MD * md)
{
  MD_Section *resource_section, *ru_section;
  MD_Entry *resource_entry, *ru_entry, *resource_used;
  MD_Field_Decl *time_field_decl, *use_field_decl;
  MD_Field *time_field, *use_field;
  int time, min_time, num_uses, global_min_time;

  resource_section = CU_find_section (md, "Resource");
  ru_section = CU_find_section (md, "Resource_Usage");
  use_field_decl = CU_find_field_decl (ru_section, "use");
  time_field_decl = CU_find_field_decl (ru_section, "time");


  /* If opti_level < 3, need to global min_time to shift all the
   * resources usages by.  
   */
  if (opti_level < 3)
    {
      num_uses = 0;
      global_min_time = 100000;

      /* Find the minimum use time of this resource */
      for (ru_entry = MD_first_entry (ru_section); ru_entry != NULL;
	   ru_entry = MD_next_entry (ru_entry))
	{
	  /* Get the resource used */
	  use_field = MD_find_field (ru_entry, use_field_decl);
	  resource_used = MD_get_link (use_field, 0);

	  /* Sanity check, punt if more than one resource used */
	  if (use_field->max_element_index != 0)
	    {
	      C_punt (md, "CU_optimize_resource_usage_times: multiple uses!");
	    }

	  /* Get the time this resource is used at */
	  time_field = MD_find_field (ru_entry, time_field_decl);
	  time = MD_get_int (time_field, 0);

	  /* Sanity check, punt if more than one time specified */
	  if (time_field->max_element_index != 0)
	    {
	      C_punt (md,
		      "CU_optimize_resource_usage_times: multiple times!");
	    }

	  /* Is this a new global min time ? */
	  if ((num_uses == 0) || (time < global_min_time))
	    global_min_time = time;
	  num_uses++;
	}

      /* Subtract the global_min_time from all uses of this resource, 
       * so that the new global min_time will be 0.
       */
      for (ru_entry = MD_first_entry (ru_section); ru_entry != NULL;
	   ru_entry = MD_next_entry (ru_entry))
	{
	  /* Get the resource used */
	  use_field = MD_find_field (ru_entry, use_field_decl);
	  resource_used = MD_get_link (use_field, 0);

	  /* Get the time this resource is used at */
	  time_field = MD_find_field (ru_entry, time_field_decl);
	  time = MD_get_int (time_field, 0);

	  /* Subtract the global min_time from this time to make the
	   * the new global_min_time 0 for all resources.
	   */
	  MD_set_int (time_field, 0, time - global_min_time);
	}

      /* Finished with optis for opti_level < 3 */
      return;
    }

  /* Otherwise, will find for each resource time sepatately
   * This is for opti_level >= 3*/
  for (resource_entry = MD_first_entry (resource_section);
       resource_entry != NULL;
       resource_entry = MD_next_entry (resource_entry))
    {
      num_uses = 0;
      min_time = 100000;

      /* Find the minimum use time of this resource */
      for (ru_entry = MD_first_entry (ru_section); ru_entry != NULL;
	   ru_entry = MD_next_entry (ru_entry))
	{
	  /* Get the resource used */
	  use_field = MD_find_field (ru_entry, use_field_decl);
	  resource_used = MD_get_link (use_field, 0);

	  /* Sanity check, punt if more than one resource used */
	  if (use_field->max_element_index != 0)
	    {
	      C_punt (md, "CU_optimize_resource_usage_times: multiple uses!");
	    }

	  /* Go to next ru if not using the resource_entry resource? */
	  if (resource_used != resource_entry)
	    continue;

	  /* Get the time this resource is used at */
	  time_field = MD_find_field (ru_entry, time_field_decl);
	  time = MD_get_int (time_field, 0);

	  /* Sanity check, punt if more than one time specified */
	  if (time_field->max_element_index != 0)
	    {
	      C_punt (md,
		      "CU_optimize_resource_usage_times: multiple times!");
	    }

	  /* Is this a new min time ? */
	  if ((num_uses == 0) || (time < min_time))
	    min_time = time;
	  num_uses++;
	}

      /* If this resource is not used, or the min_time is already 0,
       * goto next resource.
       */
      if ((num_uses == 0) || (min_time == 0))
	continue;

#if DEBUG_OPTI
      printf ("Adjusting the %i uses of %s by %i cycles\n",
	      num_uses, resource_entry->name, min_time);
#endif

      /* Subtract the min_time from all uses of this resource, so that
       * the new min_time will be 0.
       */
      for (ru_entry = MD_first_entry (ru_section); ru_entry != NULL;
	   ru_entry = MD_next_entry (ru_entry))
	{
	  /* Get the resource used */
	  use_field = MD_find_field (ru_entry, use_field_decl);
	  resource_used = MD_get_link (use_field, 0);

	  /* Go to next ru if not using the resource_entry resource? */
	  if (resource_used != resource_entry)
	    continue;

	  /* Get the time this resource is used at */
	  time_field = MD_find_field (ru_entry, time_field_decl);
	  time = MD_get_int (time_field, 0);

	  /* Subtract the min_time from this time to make the
	   * the new min_time 0 for this resource.
	   */
	  MD_set_int (time_field, 0, time - min_time);
	}
    }
}

/* Returns the first resource usage time in the time field*/
int
CU_get_ru_time (MD_Entry * ru_entry)
{
  MD_Field *time_field;

  time_field = CU_find_field_by_name (ru_entry, "time");

  return (MD_get_int (time_field, 0));
}

/* Returns the number of Tables that an entry (option/unit/ru) is used in
 * Used in CU_order_use_fields to put most_restrictive table options first.
 */
int
CU_num_table_uses (MD_Entry * option_entry)
{
  MD *md;
  MD_Section *table_section;
  MD_Field_Decl *use_field_decl;
  MD_Field *use_field;
  MD_Entry *table_entry, *target_entry;
  int index, use_count;

  use_count = 0;

  /* Get md for ease of use */
  md = option_entry->section->md;

  /* Get the table section and it's use field declaration */
  table_section = CU_find_section (md, "Reservation_Table");
  use_field_decl = CU_find_field_decl (table_section, "use");

  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      if ((use_field = CU_find_field (table_entry, use_field_decl)) == NULL)
	C_punt (md, "CU_num_tables_uses: use field not found");

      for (index = 0; index <= MD_max_element_index (use_field); index++)
	{
	  target_entry = MD_get_link (use_field, index);
	  if (target_entry == option_entry)
	    use_count++;
	}
    }

#if DEBUG_OPTI
  printf ("  %s has %i uses.\n", option_entry->name, use_count);
#endif

  /* Return the number of tables that use this entry */
  return (use_count);
}

/*
 * Applies transformations for opti_levels >= 3 only.
 *
 * Sort the Reseveration_Table->use() links and Resource_Unit->use()
 * links so that resources will be distributed the same in the various
 * forms of the same table, and so that the classical optimizations
 * can detect these duplicate tables consistently.
 * 
 * It also places checks near time 0 first, and reorders the
 * AND-OR trees (if opti_level >= 4) to place most restrictive 
 * checks first.
 *
 * if min_reorder is set to 1, reorder only if have strong heuristic
 * reason for doing so (I.e., don't reorder due to name).
 */
void
CU_order_use_fields (MD * md, int min_reorder)
{
  MD_Section *unit_section, *table_section;
  MD_Entry *unit_entry, *cur_ru_entry, *ru_entry, *min_ru_entry;
  MD_Entry *table_entry, *cur_option_entry, *option_entry, *min_option_entry;
  MD_Field *use_field, *one_of_field;
  int index1, index2, min_index;
  int cur_option_count, option_count, min_count, min_ru_time, ru_time;
  int min_ru_slot, ru_slot, cur_ru_time, cur_ru_slot;
  int min_use_count, cur_use_count, use_count;

  /* Only reorder resource usage if opti_level >= 3 */
  if (opti_level < 3)
    return;

  unit_section = CU_find_section (md, "Resource_Unit");
  for (unit_entry = MD_first_entry (unit_section); unit_entry != NULL;
       unit_entry = MD_next_entry (unit_entry))
    {
      use_field = CU_find_field_by_name (unit_entry, "use");

      /* Do nothing if only one link */
      if (use_field->max_element_index <= 0)
	continue;

      /* Sort uses by usage time, then give predence to slot, then by name 
       * (CU_ge_slot returns -1, unless slot specified)
       */
      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  /* Get the ru entry at this index */
	  cur_ru_entry = MD_get_link (use_field, index1);

	  /* Set this to be the current minimum ru_entry */
	  min_ru_entry = cur_ru_entry;
	  min_ru_time = CU_get_ru_time (cur_ru_entry);
	  min_ru_slot = CU_get_slot (cur_ru_entry);
	  min_index = index1;

	  /* Find the ru entry to replace this one (if any) */
	  for (index2 = index1 + 1; index2 <= use_field->max_element_index;
	       index2++)
	    {
	      /* Get the entry at this index */
	      ru_entry = MD_get_link (use_field, index2);
	      ru_time = CU_get_ru_time (ru_entry);
	      ru_slot = CU_get_slot (ru_entry);

	      /* Should this become the new min? */
	      if ((ru_time < min_ru_time) ||
		  ((ru_time == min_ru_time) &&
		   (ru_slot > min_ru_slot)) ||
		  ((ru_time == min_ru_time) &&
		   (ru_slot == min_ru_slot) &&
		   (strcmp (min_ru_entry->name, ru_entry->name) > 0)))
		{
		  min_ru_entry = ru_entry;
		  min_ru_time = ru_time;
		  min_ru_slot = ru_slot;
		  min_index = index2;
		}
	    }

	  /* Swap if new min is found */
	  if (min_index != index1)
	    {
#if DEBUG_OPTI
	      /* Debug */
	      printf ("  Swapping %s (time %i) and %s (time %i)\n",
		      cur_ru_entry->name, CU_get_ru_time (cur_ru_entry),
		      min_ru_entry->name, CU_get_ru_time (min_ru_entry));
#endif
	      MD_set_link (use_field, index1, min_ru_entry);
	      MD_set_link (use_field, min_index, cur_ru_entry);
	    }
	}
    }

  /* Reorder AND/OR Trees for opti_level >= 4 only */
  if (opti_level < 4)
    return;

  /* Sort AND/OR trees first by minimum resource usage time, 
   * then by number of options, then give preference to slot specifying
   * option, then give preference to option with the most uses in other
   * tables,then lastly by name
   */
  table_section = CU_find_section (md, "Reservation_Table");
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      /* Get the use field */
      use_field = CU_find_field_by_name (table_entry, "use");

      /* Do nothing if only one link */
      if (use_field->max_element_index <= 0)
	continue;

      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  cur_option_entry = MD_get_link (use_field, index1);

	  /* Really will be an table option at this point */
	  if (strcmp (cur_option_entry->section->name, "Table_Option") == 0)
	    {
	      one_of_field = CU_find_field_by_name (cur_option_entry,
						    "one_of");
	      cur_option_count = one_of_field->max_element_index + 1;
	    }
	  else
	    {
	      cur_option_count = 1;
	    }
	  cur_ru_time = CU_get_min_time (cur_option_entry);
	  cur_ru_slot = CU_get_slot (cur_option_entry);
	  cur_use_count = CU_num_table_uses (cur_option_entry);

	  /* Make this the current min entry */
	  min_option_entry = cur_option_entry;
	  min_ru_time = cur_ru_time;
	  min_ru_slot = cur_ru_slot;
	  min_use_count = cur_use_count;
	  min_index = index1;
	  min_count = cur_option_count;

	  /* Find an option with less options, then sort by name */
	  for (index2 = index1 + 1; index2 <= use_field->max_element_index;
	       index2++)
	    {
	      option_entry = MD_get_link (use_field, index2);
	      if (strcmp (option_entry->section->name, "Table_Option") == 0)
		{
		  one_of_field = CU_find_field_by_name (option_entry,
							"one_of");
		  option_count = one_of_field->max_element_index + 1;
		}
	      else
		{
		  option_count = 1;
		}

	      ru_time = CU_get_min_time (option_entry);
	      ru_slot = CU_get_slot (option_entry);
	      use_count = CU_num_table_uses (option_entry);

	      if ((ru_time < min_ru_time) ||
		  ((ru_time == min_ru_time) &&
		   (option_count < min_count)) ||
		  ((ru_time == min_ru_time) &&
		   (option_count == min_count) &&
		   (ru_slot > min_ru_slot)) ||
		  ((min_reorder == 0) &&
		   (ru_time == min_ru_time) &&
		   (option_count == min_count) &&
		   (ru_slot == min_ru_slot) &&
		   (use_count > min_use_count)) ||
		  ((min_reorder == 0) &&
		   (ru_time == min_ru_time) &&
		   (option_count == min_count) &&
		   (ru_slot == min_ru_slot) &&
		   (use_count == min_use_count) &&
		   (strcmp (min_option_entry->name, option_entry->name) > 0)))
		{
		  min_option_entry = option_entry;
		  min_ru_time = ru_time;
		  min_ru_slot = ru_slot;
		  min_use_count = use_count;
		  min_index = index2;
		  min_count = option_count;
		}
	    }

	  /* Swap if new min is found */
	  if (min_index != index1)
	    {
#if DEBUG_OPTI
	      /* Debug */
	      printf
		("  Swapping %s (time %i, options %i, slot %i, uses %i) and\n"
	         "%s (time %i options %i, slot %i, uses %i)\n",
		 cur_option_entry->name, cur_ru_time, cur_option_count,
		 cur_ru_slot, cur_use_count, min_option_entry->name,
		 min_ru_time, min_count, min_ru_slot, min_use_count);
#endif
	      MD_set_link (use_field, index1, min_option_entry);
	      MD_set_link (use_field, min_index, cur_option_entry);
	    }
	}
    }
}

/*
 * Homogenize the reservation tables to make them more regular, easier
 * to optimize, and easier for the scheduler to handle.
 * 
 * The most important/complicated transformation:
 *
 *   Transform reservation tables so that each table option can be tested
 *   independently to see if resources available.  This cannot be done
 *   if table options can interact with each other (try to allocate the
 *   same resource at the same time.)  For example, some
 *   reservation tables (I.e., Sparc) have something like:
 *
 *     RL_Ialu (use (RU_any_rp[0,1,2,3]_t0 RU_any_rp[0,1,2,3]_t0 ...)
 *
 *   Trying to allocate two register read ports from the same list of
 *   read ports.  Rewrite this resource tables (creating four new
 *   tables) so the scheduler does not have to deal with it (very
 *   nasty stuff to handle in the scheduler, only the core modules
 *   handled it in the version1 scheduler).
 *   
 *   If there are 4 overlapping options, will rewrite as four separate
 *   reservation tables:
 *
 *     RL_Ialu~1 (use (RU_rp[0]_t0~1 RU_any_rp[1,2,3]_t0 ..)) 
 *     RL_Ialu~2 (use (RU_rp[1]_t0~1 RU_any_rp[0,2,3]_t0 ..)) 
 *     RL_Ialu~3 (use (RU_rp[2]_t0~1 RU_any_rp[0,1,3]_t0 ..)) 
 *     RL_Ialu~4 (use (RU_rp[3]_t0~1 RU_any_rp[0,1,2]_t0 ..)) 
 *   
 *   Fix up any use of this reservation table, to use all the varations
 *   instead. (~# is added to all entries created by the customizer).
 *
 *   Recurse as necessary to handle other conflicts.  The mdes writer
 *   could probably come up with a better solution by hand, and probably
 *   should rewrite their mdes to prevent the customizer from having to
 *   do this brute force method.  For example, the above could become:
 *   
 *     RL_Ialu (use (RU_any_rp_pair_t0 ...)
 *     
 *   where any_rp_pair was rp0+rp1, rp1+rp2, or rp2+rp3.
 *
 * Other transformations to make the links more uniform:
 *   1) Consolidate all resource usages and resource units in each reservation
 *      table into one big resource unit.  This way there are only 
 *      resource_unit and table_option links in the reservation table.
 *      It also facilates 3) below.
 *
 *   2) For each table option, place any resource usages into a resource unit.
 *      This way there are only resource_unit links in a table option.
 *      It also facilates 3) below.
 *   
 *   3) Break up each resource usage so that only one resource and one
 *      time is specified for each usage.  This requires adding new
 *      resource usages for each time/resource combination.  It is 
 *      facilitated by 1 & 2 above, since all resource usages are only
 *      in resource_unit entries, which are easy to add to!
 *      (This also facilitates bit field optimizations which will put
 *       together entries for the same cycle and the same 32-bit word)
 */
void
homogenize_reservation_tables (MD * md)
{
  MD_Section *table_section, *option_section, *ru_section;
  MD_Entry *table_entry, *option_entry, *ru_entry;
  int entries_added;

  /* Assume no entries will be added */
  entries_added = 0;

  /* Scan every reservation table entry, including those added by
   * CU_make_res_table_orthogonal(), to make (via transformations)
   * all the reservation table option orthoginal to each other.
   * See the comment at the top of this function for details.
   */
  table_section = CU_find_section (md, "Reservation_Table");
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      entries_added += CU_make_res_table_orthogonal (table_entry);
      CU_consolidate_res_table_usages (table_entry);
    }

#if VERBOSE_STATS
  if (entries_added > 0)
    {
      printf
	("> Customizer created %i new reservation tables in order to simply "
	 "scheduling.\n",
	 entries_added);
    }
#endif

  /* Change all the Table_Option entries so that they only use
   * resource units (Create units as necessary).  This makes
   * the mdes more uniform and facilates the bit field optimizations
   * we will be doing below.
   * 
   * This transformation required in order for IMPACT to use mdes.
   */
  option_section = CU_find_section (md, "Table_Option");
  for (option_entry = MD_first_entry (option_section);
       option_entry != NULL; option_entry = MD_next_entry (option_entry))
    {
      CU_homogenize_table_options (option_entry);
    }


  /* Change all resource usages so that only one time is specified per
   * usage.  (Creates additional resource usages if there are multiple 
   * times).  This makes the mdes more uniform and facilates the bit field 
   * optimizations we will be doing below.
   *
   * This transformation required in order for IMPACT to use mdes.
   */
  ru_section = CU_find_section (md, "Resource_Usage");
  for (ru_entry = MD_first_entry (ru_section); ru_entry != NULL;
       ru_entry = MD_next_entry (ru_entry))
    {
      CU_homogenize_resource_usages (ru_entry);
    }

  /* Shift the resource times that no resource usage time is less than 0. 
   * For opti_level >= 3 try to minimize to number of different times that
   * resources are used to try to maximize the effect of the bit field 
   * optimizations.  Will just move each resource usage so that it's 
   * earlies usage is always time 0.
   *
   * IMPACT requires that all requires usage times be >= 0.
   */
  CU_optimize_resource_usage_times (md);

  /* 
   * Applies transformations for opti_levels >= 3 only.
   *
   * Sort the Reseveration_Table->use() links and Resource_Unit->use()
   * links so that resources will be distributed the same in the various
   * forms of the same table, and so that the classical optimizations
   * can detect these duplicate tables consistently.
   * 
   * It also places checks near time 0 first, and reorders the
   * AND-OR trees (if opti_level >= 4) to place most restrictive 
   * checks first.
   *
   * Don't limit the reordering.  This makes all lists have a set order,
   * so common subexpression works as well as possible.
   */
  CU_order_use_fields (md, 0);

  if ((opti_level >= 5) && tree_opti)
    {
      /* Distribute the unconditional usages into the different table options
       * so that they may be optimized together.
       */
      for (table_entry = MD_first_entry (table_section); table_entry != NULL;
	   table_entry = MD_next_entry (table_entry))
	{
	  CU_distribute_unconditional_usages (table_entry);
	}
    }

  /* To try to make opti_level 0 comparisons as fair as possible,
   * try to minimize the number of new entries added due to the above.
   * This will only eliminate entries with a "~" in the name.
   */
  {
    int change;

    change = 1;
    while (change != 0)
      {
	change = 0;
	change += eliminate_redundant_new_entries_only (md, "Resource_Usage");
	change += eliminate_redundant_new_entries_only (md, "Resource_Unit");
	change += eliminate_redundant_new_entries_only (md, "Table_Option");
      }
  }
}

/* Check the resource usages and resource units to make sure that
 * they are legal (not trying to use a resource twice at the same time!)
 */
void
check_for_unconditional_overlap (MD * md)
{
  MD_Section *ru_section, *unit_section;
  MD_Entry *ru_entry, *unit_entry, *resource1, *resource2, *ru1, *ru2;
  MD_Field *use_field;
  int index1, index2;

  /* Check resource usages for multiple uses of the same resource */
  ru_section = CU_find_section (md, "Resource_Usage");

  /* Scan each resource usage for the use of the same resource twice */
  for (ru_entry = MD_first_entry (ru_section); ru_entry != NULL;
       ru_entry = MD_next_entry (ru_entry))
    {
      /* Scan all the uses for duplicates */
      use_field = CU_find_field_by_name (ru_entry, "use");
      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  resource1 = MD_get_link (use_field, index1);

	  for (index2 = index1 + 1; index2 <= use_field->max_element_index;
	       index2++)
	    {
	      resource2 = MD_get_link (use_field, index2);

	      if (resource1 == resource2)
		{
		  fprintf (stderr,
			   "Invalid Resource_Usage %s:\n"
			   "  %s is used by both use[%i] and use[%i]!\n",
			   ru_entry->name, resource1->name, index1, index2);

		  customization_errors++;
		}
	    }
	}
    }

  /* Scan each resource unit for the use of the same resource twice at the
   * same time.
   */
  unit_section = CU_find_section (md, "Resource_Unit");
  for (unit_entry = MD_first_entry (unit_section); unit_entry != NULL;
       unit_entry = MD_next_entry (unit_entry))
    {
      /* Scan all the uses for duplicates */
      use_field = CU_find_field_by_name (unit_entry, "use");
      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  ru1 = MD_get_link (use_field, index1);
	  for (index2 = index1 + 1; index2 <= use_field->max_element_index;
	       index2++)
	    {
	      ru2 = MD_get_link (use_field, index2);

	      if (CU_overlapping_resource_usage (ru1, ru2))
		{
		  fprintf (stderr,
			   "Invalid Resource_Unit %s:\n"
			   "  The same resource (at the same time) is"
			   " required by\n"
			   "  use[%i] (%s) and use[%i] (%s)!\n",
			   unit_entry->name, index1, ru1->name,
			   index2, ru2->name);

		  customization_errors++;
		}
	    }
	}
    }
}

/* Assign an location of each the resources in the resource map
 * used  by the scheduler manager.
 *
 * May want to do 'bit' allocation in the future to allow better
 * optimization of resource units when there are more than 32 resources.
 */
int
assign_resource_map_locations (MD * md)
{
  MD_Section *resource_section;
  MD_Entry *resource_entry;
  MD_Field_Decl *location_field_decl;
  MD_Field *location_field;
  int offset, index;

  /* Get the resource section */
  resource_section = CU_find_section (md, "Resource");

  /* Remove any previous versions of the map_location field */
  location_field_decl = MD_find_field_decl (resource_section, "map_location");
  if (location_field_decl != NULL)
    MD_delete_field_decl (location_field_decl);

  /* Create a required field that takes two integers (the offset then
   * the index)
   */
  location_field_decl = MD_new_field_decl (resource_section,
					   "map_location", MD_REQUIRED_FIELD);
  MD_require_int (location_field_decl, 0);
  MD_require_int (location_field_decl, 1);


  /* Assign offset and index sequentially to the resources */
  offset = 0;
  index = -1;
  for (resource_entry = MD_first_entry (resource_section);
       resource_entry != NULL;
       resource_entry = MD_next_entry (resource_entry))
    {
      /* Calculate the offset/index for this resource */
      index++;
      if (index > 31)
	{
	  index = 0;
	  offset++;
	}

      /* Create a location field for each resource */
      location_field = MD_new_field (resource_entry, location_field_decl, 2);

      /* Set the offset and index */
      MD_set_int (location_field, 0, offset);
      MD_set_int (location_field, 1, index);
    }

  /* For now, return max offset */
  return (offset);
}

int
usage_common_to_all_options (MD_Entry * option_entry, MD_Entry * ru_entry)
{
  MD_Entry *unit_entry, *usage_entry;
  MD_Field *one_of_field, *use_field;
  int index1, index2, found;

  /* Test every unit in this option to see if it has this ru_entry */
  one_of_field = CU_find_field_by_name (option_entry, "one_of");
  for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
    {
      unit_entry = MD_get_link (one_of_field, index1);
      use_field = CU_find_field_by_name (unit_entry, "use");

      /* See if ru_entry is in this option */
      found = 0;
      for (index2 = 0; index2 <= use_field->max_element_index; index2++)
	{
	  usage_entry = MD_get_link (use_field, index2);

	  /* Assume optimizations have been done so can just compair
	   * ru_entry pointers instead of contents.
	   */
	  if (usage_entry == ru_entry)
	    {
	      found = 1;
	    }
	}

      /* If ru_entry is not in this option, return 0 */
      if (!found)
	return (0);
    }

  /* If got here, ru_entry must be every unit in this option */
  return (1);
}

int
option_usage_common_at_time (MD_Entry * option_entry, MD_Entry * usage_entry)
{
  MD_Entry *unit_entry, *ru_entry;
  MD_Field *one_of_field, *use_field, *time_field;
  int index1, ru_time, usage_time;

  /* Get the time for this usage */
  time_field = CU_find_field_by_name (usage_entry, "time");
  usage_time = MD_get_int (time_field, 0);

  /* Test only the ru_entries in the first unit of the option at
   * the specified time.
   */
  one_of_field = CU_find_field_by_name (option_entry, "one_of");
  unit_entry = MD_get_link (one_of_field, 0);
  use_field = CU_find_field_by_name (unit_entry, "use");

  for (index1 = 0; index1 <= use_field->max_element_index; index1++)
    {
      ru_entry = MD_get_link (use_field, index1);

      time_field = CU_find_field_by_name (ru_entry, "time");
      ru_time = MD_get_int (time_field, 0);

      /* Only test ru_entry's with usage_time */
      if (ru_time != usage_time)
	continue;

      /* If this ru_entry is not common to all options, return 0 now */
      if (!usage_common_to_all_options (option_entry, ru_entry))
	return (0);
    }

  /* If got here, all usages at this time (for the first unit) must
   * be common.
   */
  return (1);
}


void
remove_common_usage (MD_Entry * option_entry, MD_Entry * usage_entry)
{
  MD_Entry *unit_entry, *ru_entry, *new_unit_entry;
  MD_Field *one_of_field, *use_field, *new_use_field;
  int index1, index2, found;

  /* Remove the usage from each option */
  one_of_field = CU_find_field_by_name (option_entry, "one_of");
  for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
    {
      unit_entry = MD_get_link (one_of_field, index1);
      use_field = CU_find_field_by_name (unit_entry, "use");

      found = 0;
      for (index2 = use_field->max_element_index; index2 >= 0; index2--)
	{
	  ru_entry = MD_get_link (use_field, index2);

	  if (ru_entry == usage_entry)
	    {
	      found = 1;

	      /* Duplicate this unit so we can delete link */
	      new_unit_entry = CU_dup_entry (unit_entry);

#if DEBUG_OPTI
	      printf ("Creating %s to remove %s\n", new_unit_entry->name,
		      usage_entry->name);
#endif

	      /* Replace old unit with this new unit */
	      MD_set_link (one_of_field, index1, new_unit_entry);

	      new_use_field = CU_find_field_by_name (new_unit_entry, "use");
	      CU_delete_link_at (new_use_field, index2);
	      break;
	    }
	}

      /* Better have been found */
      if (!found)
	C_punt (NULL, "remove_common_usage: %s not found in %s[%i]!",
		usage_entry->name, option_entry->name, index1);
    }
}



void
CU_coalesce_unconditional_usages (MD * md)
{
  MD_Section *table_section, *unit_section;
  MD_Entry *table_entry, *unit_entry, *option_entry, *first_unit, *ru_entry;
  MD_Field *table_use_field, *unit_use_field, *one_of_field;
  MD_Field *uncond_use_field;
  MD_Field_Decl *unit_use_field_decl;
  int index1, index2, num_pulled_out, unit_index;
  char name_buf[5000];

  table_section = CU_find_section (md, "Reservation_Table");
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
#if DEBUG_OPTI
      printf ("Testing table %s\n", table_entry->name);
#endif

      table_use_field = CU_find_field_by_name (table_entry, "use");

      /* Find the unit entry, if any for this table */
      unit_entry = NULL;
      unit_index = -1;
      for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
	{
	  option_entry = MD_get_link (table_use_field, index1);
	  if (strcmp (option_entry->section->name, "Resource_Unit") == 0)
	    {
	      unit_entry = option_entry;
	      unit_index = index1;
	      break;
	    }
	}

      num_pulled_out = 0;

      /* Test each option, if each unit in the option uses the same
       * resources at the same time (all the usages at that time
       * must be the same), pull out the usages into the unconditional
       * unit.
       */
      for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
	{
	  option_entry = MD_get_link (table_use_field, index1);

	  /* Only process options */
	  if (strcmp (option_entry->section->name, "Table_Option") != 0)
	    continue;


	  /* Get the unit for option one */
	  one_of_field = CU_find_field_by_name (option_entry, "one_of");
	  first_unit = MD_get_link (one_of_field, 0);
	  unit_use_field = CU_find_field_by_name (first_unit, "use");


	  for (index2 = unit_use_field->max_element_index; index2 >= 0;
	       index2--)
	    {
	      ru_entry = MD_get_link (unit_use_field, index2);

	      /* If this ru_entry cannot be extracted into the 
	       * unconditionally executed unit, goto the next ru_entry.
	       */
	      if (!option_usage_common_at_time (option_entry, ru_entry))
		continue;

#if DEBUG_OPTI
	      printf ("  %s common to all units in %s\n",
		      ru_entry->name, option_entry->name);
#endif

	      /* Duplicate existing unit or create new unit if this
	       * is the first ru_entry pulled out.
	       */
	      if (num_pulled_out == 0)
		{
		  if (unit_entry != NULL)
		    {
		      unit_entry = CU_dup_entry (unit_entry);
#if DEBUG_OPTI
		      printf ("Duplicated '%s'\n", unit_entry->name);
#endif
		      MD_set_link (table_use_field, unit_index, unit_entry);
		    }
		  else
		    {
		      unit_section = CU_find_section (md, "Resource_Unit");
		      CU_get_unique_name (md, NULL, table_entry->name,
					  name_buf);
		      unit_entry = MD_new_entry (unit_section, name_buf);

#if DEBUG_OPTI
		      printf ("Created '%s'\n", unit_entry->name);
#endif

		      /* Create the use field */
		      unit_use_field_decl = CU_find_field_decl (unit_section,
								"use");
		      MD_new_field (unit_entry, unit_use_field_decl, 1);
		      MD_set_link (table_use_field,
				   table_use_field->max_element_index + 1,
				   unit_entry);
		    }
		}

	      /* Add this ru_entry to the unit */
	      uncond_use_field = CU_find_field_by_name (unit_entry, "use");
	      MD_set_link (uncond_use_field,
			   uncond_use_field->max_element_index + 1, ru_entry);

	      /* Remove this ru_entry from all the option units */
	      remove_common_usage (option_entry, ru_entry);

	      num_pulled_out++;
	    }
	}

    }

}

/* Optimize the resource units for the bit map representation used by
 * the schedule manager to keep track of the resources used.
 * Want to combine resource usages for the same word in the map into one
 * entry, so that only one bit test is required.  This is done by
 * first sorting the resource usages by offset and bit location, then
 * combining the entries with the same offset.
 */
void
optimize_resource_units (MD * md, int max_offset)
{
  MD_Section *unit_section, *ru_section, *resource_section;
  MD_Entry *unit_entry, *ru, *resource, *last_ru;
  MD_Field *unit_use_field, *ru_use_field, *ru_time_field, *location_field;
  MD_Field *last_ru_use_field;
  MD_Field_Decl *ru_use_field_decl;
  Heap *heap;
  int index, use_time, map_offset, map_index, total_offset, last_offset;
  int num_words, time_shift, needs_dup;
  int cycle_offset;
#ifdef LP64_ARCHITECTURE
  long time_count, sort_offset;
#else
  int time_count, sort_offset;
#endif
  INT_Symbol_Table *time_table;
  INT_Symbol *time_symbol;


  /* Make sure the use field in the Resource_Usage section is 
   * REQUIRED use(LINK(Resource) LINK(Resource)*) or make it
   * that way.  Expect the use field declaration to be the
   * above or REQUIRED use(LINK(Resource));
   */
  ru_section = CU_find_section (md, "Resource_Usage");
  ru_use_field_decl = CU_find_field_decl (ru_section, "use");
  resource_section = CU_find_section (md, "Resource");

  /* Do some testing to see if it is REQUIRED use(LINK(Resource)) */
  if ((ru_use_field_decl->max_require_index == 0) &&
      (ru_use_field_decl->require[0]->kleene_starred == 0) &&
      (ru_use_field_decl->require[0]->type == MD_LINK) &&
      (ru_use_field_decl->require[0]->link[0] == resource_section))
    {
      /* Add another resource link and kleene star it */
      MD_require_link (ru_use_field_decl, 1, resource_section);
      MD_kleene_star_requirement (ru_use_field_decl, 1);
    }

  /* Punt if now not REQUIRED use(LINK(Resource) LINK(Resource)*)
   * Dont make this an 'else if' so this test gets exercised.
   */
  if ((ru_use_field_decl->max_require_index != 1) ||
      (ru_use_field_decl->require[1]->kleene_starred != 1) ||
      (ru_use_field_decl->require[0]->type != MD_LINK) ||
      (ru_use_field_decl->require[1]->type != MD_LINK) ||
      (ru_use_field_decl->require[0]->link[0] != resource_section) ||
      (ru_use_field_decl->require[1]->link[0] != resource_section))
    {
      fprintf (stderr,
	       "\noptimize_resource_units: unexpected declaration "
	       "for %s->%s:\n",
	       ru_section->name, ru_use_field_decl->name);
      MD_print_field_decl (stderr, ru_use_field_decl, 80);
      C_punt (md, "Fix customizer or this field's declaration.");
    }




  /* Calculate how much we need to shift the resource usage time in
   * order to get the true map offset.  In the resource manager,
   * we will always uses shifts instead of multiplies for speed reasons.
   */
  num_words = max_offset + 1;
  time_shift = 0;
  while (num_words > (1 << time_shift))
    time_shift++;

  /* Create a heap for sorting the resource usages */
  heap = Heap_Create (HEAP_MIN);

  unit_section = CU_find_section (md, "Resource_Unit");
  for (unit_entry = MD_first_entry (unit_section);
       unit_entry != NULL; unit_entry = MD_next_entry (unit_entry))
    {
      /* Get each units use field */
      unit_use_field = CU_find_field_by_name (unit_entry, "use");

#if 0
      /* debug */
      printf ("\nBit-field optimizing '%s':\n", unit_entry->name);
#endif
      /* Create offset table to help offset sort */
      time_table = INT_new_symbol_table ("Offset", 0);
      time_count = 0;

      /* Build offset table so can place cycle usages back in specified
       * order.
       */
      for (index = 0; index <= unit_use_field->max_element_index; index++)
	{
	  /* Get the resource usage */
	  ru = MD_get_link (unit_use_field, index);

	  /* The the ru's time field */
	  ru_time_field = CU_find_field_by_name (ru, "time");

	  /* Get the use time for the resource */
	  use_time = MD_get_int (ru_time_field, 0);

	  /* Have we seen this use_time before? */
	  time_symbol = INT_find_symbol (time_table, use_time);
	  if (time_symbol == NULL)
	    {
	      /* No, increment the time count and assign a "unique" 
	       * sort offset to this time offset.  This is to prevent
	       * the order of the resource usages are checked in
	       * from being reordered by this sort (see Micro29 comment
	       * at top of loop).
	       */
	      time_count++;
	      time_symbol = INT_add_symbol (time_table, use_time,
					    (void *) (time_count << 16));
	    }
	}


      /* Sort the resource usages by offset then index using
       * the heap sorter.
       * 
       * To be fair (in Micro29 paper) to bit-field optimization 
       * without the zero time opti applied, make sure the sort doesn't 
       * reorder the order the cycles are checked checked in.  Otherwise,
       * without the zero-time opti scheduling performance can be 
       * decreased!
       */
      for (index = unit_use_field->max_element_index; index >= 0; index--)
	{
	  /* Get the resource usage */
	  ru = MD_get_link (unit_use_field, index);

	  /* Get the ru's use and time fields */
	  ru_use_field = CU_find_field_by_name (ru, "use");
	  ru_time_field = CU_find_field_by_name (ru, "time");

	  /* Make sure use and time fields have only one element in them */
	  if (ru_use_field->max_element_index != 0)
	    C_punt (md, "optimize_resource_units: one use expected!");

	  if (ru_time_field->max_element_index != 0)
	    C_punt (md, "optimize_resource_units: one time expected!");

	  /* Get the use time for the resource */
	  use_time = MD_get_int (ru_time_field, 0);

	  /* Get the map offset and index for the resource */
	  resource = MD_get_link (ru_use_field, 0);
	  location_field = CU_find_field_by_name (resource, "map_location");
	  map_offset = MD_get_int (location_field, 0);
	  map_index = MD_get_int (location_field, 1);

	  /* Calculate the cycle offset from time 0 */
	  cycle_offset = (use_time << time_shift) + map_offset;

	  /* Get the time offset for the use time */
	  time_symbol = INT_find_symbol (time_table, use_time);

	  /* Calculate it's bit offset from time 0, adjusting it
	   * with the time offset.
	   */
#ifdef LP64_ARCHITECTURE
	  sort_offset = (cycle_offset << 5) + map_index + 
	    (long) (time_symbol->data);
#else
	  sort_offset = (cycle_offset << 5) + map_index +
	    (int) (time_symbol->data);
#endif

#if 0
	  /* Debug */
	  printf
	    ("%s time %i shift %i off %i index %i cycle_offset %i total %i\n",
	     ru->name, use_time, time_shift, map_offset, map_index,
	     (int) (time_symbol->data), sort_offset);
#endif

	  /* Insert into heap based on total offset */
	  Heap_Insert (heap, (void *) ru, (double) sort_offset);

	  /* Delete ru from use list */
	  MD_delete_element (unit_use_field, index);

	}

      /* Free time table, data is just a number (do not free) */
      INT_delete_symbol_table (time_table, NULL);

      last_ru = NULL;
      last_offset = -100000;
      needs_dup = 1;

      /* Pull the ru's off the heap in total offset order */
      while ((ru = (MD_Entry *) Heap_ExtractTop (heap)) != NULL)
	{
	  /* Get the ru's use and time fields */
	  ru_use_field = CU_find_field_by_name (ru, "use");
	  ru_time_field = CU_find_field_by_name (ru, "time");

	  /* Get the use time for the resource */
	  use_time = MD_get_int (ru_time_field, 0);

	  /* Get the map offset and index for the resource */
	  resource = MD_get_link (ru_use_field, 0);
	  location_field = CU_find_field_by_name (resource, "map_location");
	  map_offset = MD_get_int (location_field, 0);

	  /* Calculate it's word offset from time 0 */
	  total_offset = ((use_time << time_shift) + map_offset);

#if 0
	  /* Debug */
	  printf ("%s time %i shift %i off %i total %i\n",
		  ru->name, use_time, time_shift, map_offset, total_offset);
#endif

	  /* If first ru or has a different offset as last ru,
	   * place it in the use_field at end of field.
	   */
	  if ((last_ru == NULL) || (total_offset != last_offset))
	    {
	      /* Place it in the use_field at the given index */
	      MD_set_link (unit_use_field,
			   unit_use_field->max_element_index + 1, ru);
	      last_ru = ru;
	      last_offset = total_offset;
	      needs_dup = 1;
	    }
	  else
	    {
	      /* Duplicate the last ru if not already duplicated */
	      if (needs_dup)
		{
		  last_ru = CU_dup_entry (last_ru);
		  /* Replace current last_ru with duped ru */
		  MD_set_link (unit_use_field,
			       unit_use_field->max_element_index, last_ru);
		  needs_dup = 0;
		}

	      /* Add current resource to the end of this ru's use field */
	      last_ru_use_field = CU_find_field_by_name (last_ru, "use");
	      MD_set_link (last_ru_use_field,
			   last_ru_use_field->max_element_index + 1,
			   resource);

#if DEBUG_OPTI
	      /* Debug */
	      printf ("%s->use[%i]:\n  coalescing %s into %s\n",
		      unit_entry->name, unit_use_field->max_element_index,
		      resource->name, last_ru->name);
#endif
	    }
	}
    }

  /* Dispose of the head (don't delete data) */
  heap = Heap_Dispose (heap, NULL);

}

void
free_link_table (INT_Symbol_Table * link_table)
{
  INT_delete_symbol_table (link_table, NULL);
}

INT_Symbol_Table *
build_link_table (MD * md, MD_Section * target_section)
{
  MD_Section *scan_section;
  MD_Field_Decl *field_decl;
  MD_Element_Req *req;
  INT_Symbol_Table *link_table;
  int require_index, link_index;

  /* Use a symbol table to hold all the field declarations that
   * link to this section.
   */
  link_table = INT_new_symbol_table ("link", 0);

  /* Go through every section in the md looking for links to this section */
  for (scan_section = MD_first_section (md); scan_section != NULL;
       scan_section = MD_next_section (scan_section))
    {
      /* Go through every field declaration in section */
      for (field_decl = MD_first_field_decl (scan_section);
	   field_decl != NULL; field_decl = MD_next_field_decl (field_decl))
	{

	  /* Go through every requirement */
	  for (require_index = 0;
	       require_index <= field_decl->max_require_index;
	       require_index++)
	    {
	      /* Get requirement */
	      req = field_decl->require[require_index];

	      /* Only process links */
	      if (req->type != MD_LINK)
		continue;


	      /* Search every link */
	      for (link_index = 0; link_index < req->link_array_size;
		   link_index++)
		{
		  /* If link to target section, add field decl to table */
		  if (req->link[link_index] == target_section)
		    {
		      /* 11/14/02 REK Strip the high 32 bits (if applicable)
		       *              off of the address of field_decl before
		       *              looking it up. */
		      INT_add_symbol (link_table, 
				      ((long) field_decl & (long) 0xffffffff),
				      (void *) field_decl);
		      /* Don't need to search other requirments */
		      require_index = field_decl->max_require_index + 1;
		      break;
		    }
		}
	    }
	}
    }

  return (link_table);
}

void
free_remap_table (INT_Symbol_Table * remap_table)
{
  INT_Symbol *remap_symbol;

  /* Free all the Pointer_Mapping structures. */
  for (remap_symbol = remap_table->head_symbol; remap_symbol != NULL;
       remap_symbol = remap_symbol->next_symbol)
    free ((void *) (remap_symbol->data));

  /* No data to free */
  INT_delete_symbol_table (remap_table, NULL);
}

INT_Symbol_Table *
build_remap_table (MD_Section * section)
{
  INT_Symbol_Table *remap_table;

  /* Use a symbol table to hold all remappings.  Create
   * remap table half the size of the section for efficiency.
   */
  remap_table =
    INT_new_symbol_table ("remap", section->entry_table->symbol_count >> 1);

  /* Return the new remap table */
  return (remap_table);
}

/* Registers a remapping from from_entry to to_entry. */
void
add_remap_entry (INT_Symbol_Table * remap_table, MD_Entry * from_entry,
		 MD_Entry * to_entry)
{
  Pointer_Mapping *mapping;

  /* Sanity check, neither from_entry or to_entry better be in the 
   * searchable part of the table.
   */
  /* 11/14/02 REK Strip the high 32 bits (if applicable) off of the address
   *              of from_entry before looking it up. */
  if (INT_find_symbol (remap_table, ((long) from_entry & (long) 0xffffffff)))
    I_punt ("add_remap_entry: from_entry already in table!");

  /* 11/14/02 REK Strip the high 32 bits (if applicable) off of the address
   *              of to_entry before looking it up. */
  if (INT_find_symbol (remap_table, ((long) to_entry & (long) 0xffffffff)))
    I_punt ("add_remap_entry: to_entry already in table!");


  /* DUE TO THE ALGORITHM's CONSTRUCTION this is not necessary.  
   * Ifdef out for speed.
   */
#if 0
  INT_Symbol *symbol;
  MD_Entry *old_target;
  /* Apply remap to all existing remaps in table. */
  for (symbol = remap_table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      old_target = (MD_Entry *) symbol->data;
      if (old_target == from_entry)
	symbol->data = (void *) to_entry;
    }
#endif

  /* Add the remapping to the table */
  /* 11/14/02 REK First, allocate a new Pointer_Mapping structure to store
   *              both pointer as pointers. */
  mapping = (Pointer_Mapping *) malloc (sizeof (Pointer_Mapping));
  
  if (mapping == NULL)
    I_punt ("add_remap_entry: Could not allocate Pointer_Mapping struct.");

  mapping->from = (void *) from_entry;
  mapping->to = (void *) to_entry;
  
  /* 11/14/02 REK Strip the high 32 bits (if applicable) off of the address
   *              of from_entry before looking it up. */
  INT_add_symbol (remap_table, ((long) from_entry & (long) 0xffffffff), 
		  (void *) mapping);
}

void
free_referenced_table (INT_Symbol_Table * referenced_table)
{
  INT_delete_symbol_table (referenced_table, NULL);
}

INT_Symbol_Table *
build_referenced_table (INT_Symbol_Table * link_table, int init_size)
{
  MD_Field_Decl *link_decl;
  INT_Symbol_Table *referenced_table;
  INT_Symbol *link_symbol;
  MD_Entry *scan_entry, *target_entry;
  MD_Field *scan_field;
  int index;


  /* Use a symbol table to hold all the entries referenced by the
   * fields in the link_table.  Use number of entries in section
   * to determine the initial symbol table size.
   */
  referenced_table = INT_new_symbol_table ("referenced", init_size);
  
  
  /* Go through each field declaration that links to this section looking
   * for references to entries in this section.  (Will get some entries
   * from other sections also...)
   */
  for (link_symbol = link_table->head_symbol; link_symbol != NULL;
       link_symbol = link_symbol->next_symbol)
    {
      /* Get field decl */
      link_decl = (MD_Field_Decl *) link_symbol->data;

      /* Scan all the entries in that field's section */
      for (scan_entry = MD_first_entry (link_decl->section);
	   scan_entry != NULL; scan_entry = MD_next_entry (scan_entry))
	{
	  /* Get the ref field for this entry, go to next if NULL */
	  if ((scan_field = MD_find_field (scan_entry, link_decl)) == NULL)
	    continue;

	  /* Scan all elements of the field */
	  for (index = scan_field->max_element_index; index >= 0; index--)
	    {
	      /* Test only links */
	      if (scan_field->element[index]->type != MD_LINK)
		continue;

	      /* Get the target entry of the link */
	      target_entry = MD_get_link (scan_field, index);

	      /* Add entry address to table if not already there */
	      /* 11/14/02 REK Strip the high 32 bits (if applicable) off of 
	       *              the address of from_entry before looking it up.
	       */
	      if (INT_find_symbol (referenced_table, 
				   ((long) target_entry & \
				    (long) 0xffffffff)) == NULL)
		{
		  INT_add_symbol (referenced_table, 
				  ((long) target_entry & \
				   (long) 0xffffffff), NULL);
		}
	    }
	}
    }
  
  /* Return the table of referenced entries */
  return (referenced_table);
}

int
entry_referenced (INT_Symbol_Table * referenced_table, MD_Entry * test_entry)
{
  if (INT_find_symbol (referenced_table, 
		       ((long) test_entry & (long) 0xffffffff)) != NULL)
    return (1);
  else
    return (0);
}

/* Removes entries that are not referenced by any other entries in
 * the mdes (via a link).  Returns a count of the entries removed.
 */
int
remove_unreferenced_entries (MD * md, char *section_name)
{
  MD_Section *section;
  MD_Entry *test_entry, *next_test_entry;
  INT_Symbol_Table *link_table, *referenced_table;
  int remove_count;

  remove_count = 0;

  section = CU_find_section (md, section_name);

  /* Use a symbol table to hold all the field declarations that
   * link to this section.
   */
  link_table = build_link_table (md, section);

  /* Build a table of all the entries referenced by the fields
   * in the link_table.  Make its initial size be the number
   * of entries in this section.
   */
  referenced_table =
    build_referenced_table (link_table, section->entry_table->symbol_count);

  /* Punt if no sections link to this section found since this routine 
   * will want to remove all the entries (which cannot be right)
   */
  if (link_table->head_symbol == NULL)
    {
      C_punt (md,
	      "remove_unreferenced_entries: No sections reference '%s'!",
	      section->name);
    }

  /* Scan section's entries and delete unreferenced entries */
  for (test_entry = MD_first_entry (section); test_entry != NULL;
       test_entry = next_test_entry)
    {
      /* Get the next entry to scan before possibly deleting entry */
      next_test_entry = MD_next_entry (test_entry);

      /* If entry unreferenced, delete it */
      if (!entry_referenced (referenced_table, test_entry))
	{
#if DEBUG_OPTI
	  printf ("Deleting %s->%s\n", section->name, test_entry->name);
#endif

	  MD_delete_entry (test_entry);

	  /* Update count */
	  remove_count++;
	}
    }

#if DEBUG_OPTI
  if (remove_count > 0)
    {
      printf ("%i entries removed from %s.\n", remove_count, section->name);
    }
#endif

  /* Free the link and referenced tables */
  free_link_table (link_table);
  free_referenced_table (referenced_table);

  /* Return the number of entries removed */
  return (remove_count);
}


/* Hashes a string, returning an unsigned 32 bit number. */
unsigned int
CU_hash_string (char *string)
{
  unsigned int hash_val;
  unsigned char *ptr;
  unsigned int ch;

  hash_val = 0;

  /* Scan through all the characters, adding the characters to the
   * hash value.  Multiply the hash value by 17 (using shifts) before
   * adding each character in.
   *
   * This very quick hash algorithm was tuned to work well with
   * strings ending with numbers.
   */
  ptr = (unsigned char *) string;
  while ((ch = *ptr) != 0)
    {
      /* Multiply hash_val by 17 by adding 16*hash_val to it.
       * (Use a shift, integer multiply is usually very expensive)
       */
      hash_val += (hash_val << 4);

      /* Add in character value */
      hash_val += ch;

      /* Goto next character */
      ptr++;
    }

  /* Return the hash value */
  return (hash_val);
}


/* A content-addressable entry table used to delete redudant entries.
 * Will ignore "ignore_field" if not NULL when doing comparisons.
 * Automatically ignores the "original_name" field.
 *
 * Returns a content addressable entry table.
 */
ENTRY_Contents_Table *
ENTRY_new_contents_table (char *ignore_field, int size)
{
  ENTRY_Contents_Table *table;

  /* Alloc table, create pool if necessary */
  if (ENTRY_Contents_Table_pool == NULL)
    {
      ENTRY_Contents_Table_pool =
	L_create_alloc_pool ("ENTRY_Contents_Table",
			     sizeof (ENTRY_Contents_Table), 1);
    }
  table = (ENTRY_Contents_Table *) L_alloc (ENTRY_Contents_Table_pool);

  /* Create a int symbol table for the hash table */
  table->hash_table = INT_new_symbol_table ("entry_table", size);

  /* Save the ignore field */
  if (ignore_field == NULL)
    table->ignore_field = NULL;
  else
    table->ignore_field = strdup (ignore_field);

  /* Return the table */
  return (table);
}
/* Returns the hash value for the contents an entry, 
 * ignoring the original_name field and the ignore_field (if not
 * NULL).
 */
unsigned int
ENTRY_hash_contents (MD_Entry * entry1, char *ignore_field)
{
  MD_Field_Decl *field_decl;
  MD_Field *field1;
  MD_Element *element1;
  int i;
  unsigned int hash_val;
#ifdef LP64_ARCHITECTURE
  long element_val = 0;
#else
  int element_val = 0;
#endif

  hash_val = 0;

  /* Hash all the field values together */
  for (field_decl = MD_first_field_decl (entry1->section);
       field_decl != NULL; field_decl = MD_next_field_decl (field_decl))
    {
      /* Skip testing this field decl if field is original_name */
      if (strcmp (field_decl->name, "original_name") == 0)
	continue;

      /* Skip testing this field decl if field is ignore_field
       * (don't do test if NULL)
       */
      if ((ignore_field != NULL) &&
	  (strcmp (field_decl->name, ignore_field) == 0))
	continue;

      /* Get the field */
      field1 = MD_find_field (entry1, field_decl);

      /* Skip field if NULL (hash in zero) */
      if (field1 == NULL)
	{
	  hash_val += (hash_val << 4) + 0;
	  continue;
	}

      /* Hash in each element in field */
      for (i = field1->max_element_index; i >= 0; i--)
	{
	  element1 = field1->element[i];

	  /* Get element hash value, use 0 if element NULL */
	  if (element1 == NULL)
	    {
	      element_val = 0;
	    }
	  else
	    {
	      /* Compare element contents */
	      switch (element1->type)
		{
		case MD_INT:
		  element_val = element1->value.i;
		  break;

		case MD_DOUBLE:
		  element_val = (int) element1->value.d;
		  break;

		case MD_STRING:
		  element_val = CU_hash_string (element1->value.s);
		  break;

		case MD_LINK:
#ifdef LP64_ARCHITECTURE
		  element_val = ((long) element1->value.l) >> 2;
#else
		  element_val = ((int) element1->value.l) >> 2;
#endif
		  break;

		default:
		  C_punt (entry1->section->md,
			  "ENTRY_hash_contents: unknown element type %i",
			  element1->type);
		}
	    }

	  hash_val += (hash_val << 4) + element_val;
	}
    }

  /* Return the hashed value */
  return (hash_val);
}

void
ENTRY_add_contents (ENTRY_Contents_Table * table, MD_Entry * entry)
{
  INT_Symbol *symbol;
  ENTRY_Contents_Node *node;
  int hash_val;

  /* Get the hash_val for this entry */
  hash_val = (int) ENTRY_hash_contents (entry, table->ignore_field);

  /* Find symbol for hash value, if already exists */
  if ((symbol = INT_find_symbol (table->hash_table, hash_val)) != NULL)
    {
      /* Sanity check, make sure contents does not already exist 

       * Punt if it is, since can cause major debugging nighmares.
       */
      for (node = (ENTRY_Contents_Node *) symbol->data; node != NULL;
	   node = node->next_node)
	{
	  /* Better not already be in list */
	  if (CU_redundant_entries (entry, node->entry, table->ignore_field))
	    {
	      C_punt (entry->section->md,
		      "ENTRY_add_contents: cannot add '%s', "
		      "contents already in table!", entry->name);
	    }
	}
    }

  /* Otherwise, create symbol for this hash value */
  else
    {
      /* Create symbol with NULL data.  Content node will be added later */
      symbol = INT_add_symbol (table->hash_table, hash_val, (void *) NULL);
    }

  /* Create a node for this entry (initialize pool if necessary) */
  if (ENTRY_Contents_Node_pool == NULL)
    {
      ENTRY_Contents_Node_pool =
	L_create_alloc_pool ("ENTRY_Contents_Node",
			     sizeof (ENTRY_Contents_Node), 64);
    }
  node = (ENTRY_Contents_Node *) L_alloc (ENTRY_Contents_Node_pool);

  /* Initialize fields */
  node->entry = entry;
  node->next_node = (ENTRY_Contents_Node *) symbol->data;

  /* Add to front of symbol list */
  symbol->data = (void *) node;
}

/* Returns the entry pointer if match found, NULL otherwise */
MD_Entry *
ENTRY_find_match (ENTRY_Contents_Table * table, MD_Entry * entry)
{
  INT_Symbol *symbol;
  ENTRY_Contents_Node *node;
  int hash_val;

  /* Get the hash_val for this entry */
  hash_val = (int) ENTRY_hash_contents (entry, table->ignore_field);

  /* Find symbol for hash value */
  if ((symbol = INT_find_symbol (table->hash_table, hash_val)) != NULL)
    {
      /* Search nodes with same hash value for match */
      for (node = (ENTRY_Contents_Node *) symbol->data; node != NULL;
	   node = node->next_node)
	{
	  if (CU_redundant_entries (entry, node->entry, table->ignore_field))
	    {
	      /* Return matching entry */
	      return (node->entry);
	    }
	}
    }

  /* If got here, match was not found */
  return (NULL);
}

/* Frees all the nodes in a content node list.  
 * Used by ENTRY_delete_contents_table.
 */
void
ENTRY_free_nodes (ENTRY_Contents_Node * first_node)
{
  ENTRY_Contents_Node *node, *next_node;

  for (node = first_node; node != NULL; node = next_node)
    {
      /* Get the next node before deleting this one */
      next_node = node->next_node;

      /* Don't free contents, still in md */
      L_free (ENTRY_Contents_Node_pool, node);
    }
}

void
ENTRY_delete_contents_table (ENTRY_Contents_Table * table)
{

  /* Delete the hash table */
  INT_delete_symbol_table (table->hash_table,
			   (void (*)(void *)) ENTRY_free_nodes);

  /* Free the ignore field if it exists */
  if (table->ignore_field != NULL)
    free (table->ignore_field);

  /* Free the table */
  L_free (ENTRY_Contents_Table_pool, table);
}

/* Returns 1 if the field contents of the two entries are identical,
 * ignoring the original_name field and the ignore_field (if not
 * NULL).
 */
int
CU_redundant_entries (MD_Entry * entry1, MD_Entry * entry2,
		      char *ignore_field)
{
  MD_Field_Decl *field_decl;
  MD_Field *field1, *field2;
  MD_Element *element1, *element2;
  int i;

  /* These entries better be in the same section */
  if (entry1->section != entry2->section)
    {
      C_punt (entry1->section->md,
	      "CU_redundant_entries: entries have different sections %s&%s",
	      entry1->section->name, entry2->section->name);
    }

  /* Scan the field declarations looking for differences */
  for (field_decl = MD_first_field_decl (entry1->section);
       field_decl != NULL; field_decl = MD_next_field_decl (field_decl))
    {
      /* Skip testing this field decl if field is original_name */
      if (strcmp (field_decl->name, "original_name") == 0)
	continue;

      /* Skip testing this field decl if field is ignore_field
       * (don't do test if NULL)
       */
      if ((ignore_field != NULL) &&
	  (strcmp (field_decl->name, ignore_field) == 0))
	continue;

      /* Get the two fields */
      field1 = MD_find_field (entry1, field_decl);
      field2 = MD_find_field (entry2, field_decl);

      /* If they are both NULL, they match so far... */
      if ((field1 == NULL) && (field2 == NULL))
	continue;

      /* If they are not both NULL, neither may be NULL to match */
      if ((field1 == NULL) || (field2 == NULL))
	return (0);


      /* They must have the same number of elements to match */
      if (field1->max_element_index != field2->max_element_index)
	return (0);

      /* Compare each element in the fields */
      for (i = field1->max_element_index; i >= 0; i--)
	{
	  element1 = field1->element[i];
	  element2 = field2->element[i];

	  /* If they are both NULL, they match so far... */
	  if ((element1 == NULL) && (element2 == NULL))
	    continue;

	  /* If they are not both NULL, neither may be NULL to match */
	  if ((element1 == NULL) || (element2 == NULL))
	    return (0);

	  /* Must be the same type of element to match */
	  if (element1->type != element2->type)
	    return (0);

	  /* Compare element contents */
	  switch (element1->type)
	    {
	    case MD_INT:
	      if (element1->value.i != element2->value.i)
		return (0);
	      break;

	    case MD_DOUBLE:
	      if (element1->value.d != element2->value.d)
		return (0);
	      break;

	    case MD_STRING:
	      if (strcmp (element1->value.s, element2->value.s) != 0)
		return (0);
	      break;

	    case MD_LINK:
	      if (element1->value.l != element2->value.l)
		return (0);
	      break;

	    default:
	      C_punt (entry1->section->md,
		      "CU_redundant_entries: unknown element type %i",
		      element1->type);
	    }
	}
    }

  /* If got here, they must have the same contents (be redundant) */
  return (1);
}

/* Replace references (links) to old_entry with new_entry */
void
CU_replace_references (INT_Symbol_Table * link_table,
		       INT_Symbol_Table * remap_table)
{
  MD_Field_Decl *link_decl;
  INT_Symbol *link_symbol;
  MD_Entry *scan_entry, *target_entry;
  Pointer_Mapping *remap_entry;
  MD_Field *scan_field;
  int index;

  /* Go through each field declaration that links to this section looking
   * for a reference in the remap table.
   */
  for (link_symbol = link_table->head_symbol; link_symbol != NULL;
       link_symbol = link_symbol->next_symbol)
    {
      /* Get field decl */
      link_decl = (MD_Field_Decl *) link_symbol->data;

      /* Scan all the entries in that field's section */
      for (scan_entry = MD_first_entry (link_decl->section);
	   scan_entry != NULL; scan_entry = MD_next_entry (scan_entry))
	{
	  /* Get the ref field for this entry, go to next if NULL */
	  if ((scan_field = MD_find_field (scan_entry, link_decl)) == NULL)
	    continue;

	  /* Scan all elements of the field */
	  for (index = scan_field->max_element_index; index >= 0; index--)
	    {
	      /* Test only links */
	      if (scan_field->element[index]->type != MD_LINK)
		continue;

	      /* If link to entry in remap table, replace with link to 
	       * remap_entry */
	      target_entry = MD_get_link (scan_field, index);
	      /* 11/14/02 REK Strip the high 32 bits (if applicable) off of
	       *              target_entry before looking it up. */
	      remap_entry = (Pointer_Mapping *) \
		INT_find_symbol_data (remap_table,
				      ((long) target_entry & \
				       (long) 0xffffffff));

	      if (remap_entry != NULL)
		{
#if DEBUG_OPTI
		  printf ("  Replacing %s in %s->%s\n",
			  target_entry->name,
			  scan_entry->section->name, scan_entry->name);
#endif
		  /* 11/14/02 REK Changing this to use the to field of the
		   *              Pointer_Mapping structure.  This field used
		   *              to be stored in the data section of the
		   *              symbol. */
		  MD_set_link (scan_field, index, remap_entry->to);
		}
	    }
	}
    }
}

int
eliminate_redundant_entries (MD * md, char *section_name)
{
  MD_Section *section;
  MD_Entry *entry1, *next_entry1, *matching_entry;
  INT_Symbol_Table *link_table, *remap_table;
  INT_Symbol *remap_symbol;
  ENTRY_Contents_Table *contents_table;
  Pointer_Mapping *mapping;
  int eliminate_count;

  eliminate_count = 0;

  section = CU_find_section (md, section_name);

  /* Create a contents-addressable table for the entries in this section 
   * (no extra fields will be ignored during contents comparisons) 
   * Set initial size to number of entries in section.
   */
  contents_table =
    ENTRY_new_contents_table (NULL, section->entry_table->symbol_count);

  /* Create a remap table for the remappings that will be required to
   * eliminate redundant entries.
   */
  remap_table = build_remap_table (section);

  /* Use a symbol table to hold all the field declarations that
   * link to this section.
   */
  link_table = build_link_table (md, section);

  /* Punt if no links to this sections found since the name of this entry
   * must the the only way to the compiler to access this information,
   * and we cannot deletine names the compiler is expecting!
   */
  if (link_table->head_symbol == NULL)
    {
      C_punt (md,
	      "eliminate_redundant_entries: No sections link to '%s'!",
	      section->name);
    }

  /* Scan each entry looking for identical entries later in the section */
  for (entry1 = MD_first_entry (section); entry1 != NULL;
       entry1 = next_entry1)
    {
      /* Get the next entry before possibly deleting this one */
      next_entry1 = MD_next_entry (entry1);

      /* If there is already an entry with the same contents as entry1,
       * delete entry1.
       */
      /* Find previous entry, if any, with the same contents as entry1 */
      matching_entry = ENTRY_find_match (contents_table, entry1);

      /* If matching entry found, point all uses of entry1 at the matching
       * entry and delete entry1.
       */
      if (matching_entry != NULL)
	{
	  /* Add entry to replace all references (links) to entry1 
	   * with matching_entry.
	   */
	  add_remap_entry (remap_table, entry1, matching_entry);


	}

      /* Otherwise, add this entry to the content-addressable table */
      else
	{
	  ENTRY_add_contents (contents_table, entry1);
	}
    }

  /* Replace references (links) to entry1 with matching_entry 
   * for all the remaps in the remap_table
   */
  CU_replace_references (link_table, remap_table);

  /* Delete all the entries that were remapped  */
  for (remap_symbol = remap_table->head_symbol; remap_symbol != NULL;
       remap_symbol = remap_symbol->next_symbol)
    {
      /* Delete from_entry now all links point to to_entry */
      /* 11/14/02 REK The 32 bit address used to be stored in the value section
       *              of remap_symbol.  The full 64 bit address is now stored
       *              in the from field of the data section of remap_symbol. */
      mapping = (Pointer_Mapping *) remap_symbol->data;
      MD_delete_entry ((MD_Entry *) mapping->from);

      /* Update count */
      eliminate_count++;
    }

  /* Free the remap table */
  free_remap_table (remap_table);

#if DEBUG_OPTI
  if (eliminate_count > 0)
    {
      printf ("%i redundant entries eliminated from %s.\n", eliminate_count,
	      section->name);
    }
#endif

  /* Free the link table */
  free_link_table (link_table);

  /* Free the content-addressable entry table */
  ENTRY_delete_contents_table (contents_table);

  /* Return the number of entries removed */
  return (eliminate_count);

}

/* To try to be fair after homogenizing the reservation tables when
 * opti-leve is 0, try to eliminate unnecessary entries added just
 * in order to homogenize the reservation tables.
 */
int
eliminate_redundant_new_entries_only (MD * md, char *section_name)
{
  MD_Section *section;
  MD_Entry *entry1, *next_entry1, *matching_entry;
  INT_Symbol_Table *link_table, *remap_table;
  INT_Symbol *remap_symbol;
  ENTRY_Contents_Table *contents_table;
  Pointer_Mapping *mapping;
  int eliminate_count;
  char *name_ptr;

  eliminate_count = 0;

  section = CU_find_section (md, section_name);

  /* Create a contents-addressable table for the entries in this section 
   * (no extra fields will be ignored during contents comparisons) 
   * Set initial size to number of entries in section.
   */
  contents_table =
    ENTRY_new_contents_table (NULL, section->entry_table->symbol_count);

  /* Create a remap table for the remappings that will be required to
   * eliminate redundant entries.
   */
  remap_table = build_remap_table (section);

  /* Use a symbol table to hold all the field declarations that
   * link to this section.
   */
  link_table = build_link_table (md, section);

  /* Punt if no links to this sections found since the name of this entry
   * must the the only way to the compiler to access this information,
   * and we cannot deletine names the compiler is expecting!
   */
  if (link_table->head_symbol == NULL)
    {
      C_punt (md,
	      "eliminate_redundant_entries: No sections link to '%s'!",
	      section->name);
    }

  /* Scan each entry looking for identical entries later in the section */
  for (entry1 = MD_first_entry (section); entry1 != NULL;
       entry1 = next_entry1)
    {
      /* Get the next entry before possibly deleting this one */
      next_entry1 = MD_next_entry (entry1);

      /* If there is already an entry with the same contents as entry1,
       * delete entry1.
       */
      /* Find previous entry, if any, with the same contents as entry1 */
      matching_entry = ENTRY_find_match (contents_table, entry1);

      /* If matching entry found, point all uses of entry1 at the matching
       * entry and delete entry1.
       */
      if (matching_entry != NULL)
	{
	  /* Determine if entry1 is a new entry by checking for a ~.
	   */
	  for (name_ptr = entry1->name; *name_ptr != 0; name_ptr++)
	    {
	      if (*name_ptr == '~')
		break;
	    }
	  /* Only remap if have ~ in name. */
	  if (*name_ptr == '~')
	    {
	      /* Add entry to replace all references (links) to entry1 
	       * with matching_entry.
	       */
	      add_remap_entry (remap_table, entry1, matching_entry);
	    }

	}

      /* Otherwise, add this entry to the content-addressable table */
      else
	{
	  ENTRY_add_contents (contents_table, entry1);
	}
    }

  /* Replace references (links) to entry1 with matching_entry 
   * for all the remaps in the remap_table
   */
  CU_replace_references (link_table, remap_table);

  /* Delete all the entries that were remapped  */
  for (remap_symbol = remap_table->head_symbol; remap_symbol != NULL;
       remap_symbol = remap_symbol->next_symbol)
    {
      /* Delete from_entry now all links point to to_entry */
      /* 11/14/02 REK The 32 bit address used to be stored in the value section
       *              of remap_symbol.  The full 64 bit address is now stored
       *              in the from field of the data section of remap_symbol. */
      mapping = (Pointer_Mapping *) remap_symbol->data;
      MD_delete_entry ((MD_Entry *) mapping->from);

      /* Update count */
      eliminate_count++;
    }

  /* Free the remap table */
  free_remap_table (remap_table);

#if DEBUG_OPTI
  if (eliminate_count > 0)
    {
      printf ("%i redundant entries eliminated from %s.\n", eliminate_count,
	      section->name);
    }
#endif

  /* Free the link table */
  free_link_table (link_table);

  /* Free the content-addressable entry table */
  ENTRY_delete_contents_table (contents_table);

  /* Return the number of entries removed */
  return (eliminate_count);

}

/* Remove all the original name fields created by this customizer.
 * Just deletes all fields with the name original_name!
 */
void
remove_original_name (MD * md)
{
  MD_Section *section;
  MD_Field_Decl *field_decl;

  /* Scan all sections for the original_name field */
  for (section = MD_first_section (md); section != NULL;
       section = MD_next_section (section))
    {
      /* Delete the original_name field if in this section */
      field_decl = MD_find_field_decl (section, "original_name");
      if (field_decl != NULL)
	{

#if DEBUG_OPTI
	  printf ("Deleting field %s->%s\n", section->name, field_decl->name);
#endif

	  MD_delete_field_decl (field_decl);
	}
    }

}

/* Renames the section entries to be short */
void
rename_section_entries (MD * md, char *section_name, char *prefix)
{
  MD_Section *section;
  MD_Entry *entry;
  char name_buf[5000];
  int id;

  id = 0;

  section = CU_find_section (md, section_name);

  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      sprintf (name_buf, "%s%i", prefix, id);
      id++;

      MD_rename_entry (entry, name_buf);
    }
}

int
optimize_alt_lists (MD * md)
{
  MD_Section *op_section;
  MD_Entry *op_entry, *link1, *link2, *new_link1;
  MD_Entry *target1, *target2;
  MD_Field_Decl *alt_decl;
  MD_Field *alt_field, *format1, *format2;
  int index1, index2, i1, i2;
  int change_count;

  /* Initialize change count */
  change_count = 0;

  op_section = CU_find_section (md, "Operation");
  alt_decl = CU_find_field_decl (op_section, "alt");

  /* Optimize each operation */
  for (op_entry = MD_first_entry (op_section); op_entry != NULL;
       op_entry = MD_next_entry (op_entry))
    {
      /* Get the alt field for this op entry */
      if ((alt_field = MD_find_field (op_entry, alt_decl)) == NULL)
	C_punt (md, "optimize_alt_lists: alt field expected");

      /*
       * Remove duplicate links in alt field.  
       * alt_field->max_element_index will be changing as we do this.
       */
      for (index1 = 0; index1 <= alt_field->max_element_index; index1++)
	{
	  link1 = MD_get_link (alt_field, index1);
	  new_link1 = NULL;

	  /* Scan forwards for elements we can delete or combine 
	   * index2 will only be incremented when we can do neither.
	   */
	  index2 = index1 + 1;
	  while (index2 <= alt_field->max_element_index)
	    {
	      link2 = MD_get_link (alt_field, index2);

	      /* Delete link2 if the same as link one */
	      if (link1 == link2)
		{
#if DEBUG_OPTI
		  printf ("Deleting redundant alt in '%s':\n"
			  "  alt[%i] and alt[%i] both point to %s\n",
			  op_entry->name, index1, index2, link1->name);
#endif

		  /* Delete the redudant link, index2 will now point at 
		   * the next link to process,so don't increment index2
		   */
		  CU_delete_link_at (alt_field, index2);

		  /* Update change count */
		  change_count++;
		}

	      /* Only combine link1 and link2 if index2 = index1 + 1
	       * (Otherwise will change the priorities of the alts
	       *  by pulling them together (bad)).
	       */
	      else if ((index2 == (index1 + 1)) &&
		       CU_redundant_entries (link1, link2, "format"))
		{
#if DEBUG_OPTI
		  printf ("Formats can be combined in '%s':\n"
			  "  alt[%i]->%s and alt[%i]->%s\n",
			  op_entry->name, index1, link1->name, index2,
			  link2->name);
#endif

		  /* If have not combined before, create new link1 
		   * and place it in link1's place.
		   */
		  if (new_link1 == NULL)
		    {
		      new_link1 = CU_dup_entry (link1);
		      MD_set_link (alt_field, index1, new_link1);
		    }

		  /* Add link2's formats to link1's */
		  format1 = CU_find_field_by_name (new_link1, "format");
		  format2 = CU_find_field_by_name (link2, "format");
		  for (i1 = 0; i1 <= format2->max_element_index; i1++)
		    {
		      target2 = MD_get_link (format2, i1);
		      MD_set_link (format1, format1->max_element_index + 1,
				   target2);
		    }

		  /* Delete link2 from alt list, index2 will then point
		   * at next link to process, so don't increment index2
		   */
		  CU_delete_link_at (alt_field, index2);

		  /* Update change count */
		  change_count++;
		}

	      /* Warn about miss opportunties now.  Probably will want
	       * to turn off in the future. (TURNED OFF)
	       */
	      else if ((index2 != (index1 + 1)) &&
		       CU_redundant_entries (link1, link2, "format"))
		{

#if DEBUG_OPTI
		  printf
		    ("Warning: formats could be combined in '%s' if they "
		     "were adjacent!!!:\n"
		     "  alt[%i]->%s and alt[%i]->%s\n", op_entry->name,
		     index1, link1->name, index2, link2->name);
#endif

		  /* For now, increment index2 */
		  index2++;

		}

	      /* Otherwise go to next link to test */
	      else
		{
		  index2++;
		}
	    }

	  /* Scan link1 to make sure there are not redundant formats in
	   * the format field.  Process the new_link1 (if any).
	   */
	  link1 = MD_get_link (alt_field, index1);
	  format1 = CU_find_field_by_name (link1, "format");
	  for (i1 = 0; i1 <= format1->max_element_index; i1++)
	    {
	      target1 = MD_get_link (format1, i1);

	      /* Scan backwards to facilitate deleting links */
	      for (i2 = format1->max_element_index; i2 > i1; i2--)
		{
		  target2 = MD_get_link (format1, i2);

		  if (target1 == target2)
		    {
#if DEBUG_OPTI
		      printf ("Deleting redundant format in '%s':\n"
			      "  format[%i] and format[%i] both point to %s\n",
			      link1->name, i1, i2, target1->name);
#endif

		      /* Delete the redundant format */
		      CU_delete_link_at (format1, i2);

		      /* Update change count */
		      change_count++;
		    }
		}
	    }
	}
    }
  return (change_count);
}

/*
 * Does nothing for opti_level < 1.
 * 
 * Does the following "classical" optimizations on the mdes sections
 * that they are applicable to:
 *
 * 1) "dead code removal": Removes unreferenced entries.
 * 2) "common subexpression elimination": Removes identical entries by
 *       changing all links to point to just one of them.
 * 
 * Also does the above optimizations to the alt lists in the operations.
 * May create new alternatives that then need to be optimized with the
 * above.  (That is why they are done here).
 *
 * Returns a count of the number of entries deleted and/or modified.
 */
int
do_classical_optimizations (MD * md)
{
  int change, total_change, pass;

#if VERBOSE_OPTI
  static int count = 0;
  count++;
  printf ("Call %i to do_classical_optimizations\n", count);
  print_time_stamp (stdout);
  fflush (stdout);
#endif

  /* Do nothing for opti_level < 1 */
  if (opti_level < 1)
    return (0);

  total_change = 0;
  change = 1;
  pass = 0;
  while (change)
    {
      change = 0;
      pass++;

#if DEBUG_OPTI
      printf ("\nPass %i:\n", pass);
#endif

      /* Cannot remove "redundant" Resource or Field_Type entries
       * since externally accessed so the name of the entry
       * means something.  I think it is confusing to
       * elmininate redundant Operand_Latency entries.
       */
      change += remove_unreferenced_entries (md, "Resource");
      change += remove_unreferenced_entries (md, "Field_Type");
      change += remove_unreferenced_entries (md, "Operand_Latency");


      change += remove_unreferenced_entries (md, "Resource_Usage");
      change += eliminate_redundant_entries (md, "Resource_Usage");

      change += remove_unreferenced_entries (md, "Resource_Unit");
      change += eliminate_redundant_entries (md, "Resource_Unit");

      change += remove_unreferenced_entries (md, "Table_Option");
      change += eliminate_redundant_entries (md, "Table_Option");

      change += remove_unreferenced_entries (md, "Reservation_Table");
      change += eliminate_redundant_entries (md, "Reservation_Table");

      change += remove_unreferenced_entries (md, "Operation_Format");
      change += eliminate_redundant_entries (md, "Operation_Format");

      change += remove_unreferenced_entries (md, "Operation_Latency");
      change += eliminate_redundant_entries (md, "Operation_Latency");

      change += remove_unreferenced_entries (md, "Scheduling_Alternative");
      change += eliminate_redundant_entries (md, "Scheduling_Alternative");
      change += optimize_alt_lists (md);

      change += remove_unreferenced_entries (md, "Operation");
      change += eliminate_redundant_entries (md, "Operation");

      total_change += change;
    }

#if VERBOSE_OPTI
  printf ("Finished call %i to do_classical_optimizations\n", count);
  print_time_stamp (stdout);
  fflush (stdout);
#endif

  return (total_change);
}

/* Counts the number of entries in a section */
int
CU_count_section_entries (MD_Section * section)
{
  /* For speed, use internal md info (not the best way to do this) */
  return (section->entry_table->symbol_count);
}

/* Counts the number of entries in the md */
int
CU_count_entries (MD * md)
{
  MD_Section *section;
  int count;

  count = 0;

  for (section = MD_first_section (md); section != NULL;
       section = MD_next_section (section))
    {
      count += CU_count_section_entries (section);
    }

  return (count);
}

/* Check to make sure exactly one use in a Reservation Table specifies
 * the use of a slot (may use multiple slots in one use, the
 * smallest slot will be used).
 */
void
check_slot_specification (MD * md)
{
  MD_Section *section, *resource_section;
  MD_Entry *entry, *use_entry, *one_of_entry, *slot_entry;
  MD_Entry *resource1, *resource2;
  MD_Field_Decl *slot_field_decl;
  MD_Field *use_field, *one_of_field, *slot1_field, *slot2_field;
  int index1, slot, index2, slot_index;
  int slot1, slot2;
  int option_count;

  /* Scan resources to make sure a slot id is assigned to only
   * one resource.
   */
  resource_section = CU_find_section (md, "Resource");
  slot_field_decl = CU_find_field_decl (resource_section, "slot");
  for (resource1 = MD_first_entry (resource_section); resource1 != NULL;
       resource1 = MD_next_entry (resource1))
    {
      /* Only scan if assigned a slot */
      if ((slot1_field = MD_find_field (resource1, slot_field_decl)) == NULL)
	continue;
      slot1 = MD_get_int (slot1_field, 0);

      /* Scan rest of table for resource with same slot number */
      for (resource2 = MD_next_entry (resource1); resource2 != NULL;
	   resource2 = MD_next_entry (resource2))
	{
	  /* Test only if assigned a slot */
	  slot2_field = MD_find_field (resource2, slot_field_decl);
	  if (slot2_field == NULL)
	    continue;

	  /* See if they are the same */
	  slot2 = MD_get_int (slot2_field, 0);

	  if (slot1 == slot2)
	    {
	      fprintf (stderr,
		       "Error: Resources %s and %s cannot both be slot %i!\n",
		       resource1->name, resource2->name, slot1);
	      customization_errors++;
	    }
	}
    }


  /* Scan each reservation table for correctness */
  section = CU_find_section (md, "Reservation_Table");
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      /* Each table better have only one 'use' that specifies a slot */
      slot_entry = NULL;
      slot_index = -1;

      use_field = CU_find_field_by_name (entry, "use");
      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  use_entry = MD_get_link (use_field, index1);

	  /* For table options, if one option specifies a slot,
	   * all the options must specify a slot.
	   */
	  if (strcmp (use_entry->section->name, "Table_Option") == 0)
	    {
	      option_count = 0;
	      one_of_field = CU_find_field_by_name (use_entry, "one_of");
	      for (index2 = 0; index2 <= one_of_field->max_element_index;
		   index2++)
		{
		  one_of_entry = MD_get_link (one_of_field, index2);
		  slot = CU_get_slot (one_of_entry);

		  if (slot != -1)
		    {
		      option_count++;
		    }
		}

	      /* Use slot if every option uses a slot */
	      if (option_count == (one_of_field->max_element_index + 1))
		{
		  /* Detect if more than one slot specified */
		  if (slot_entry != NULL)
		    {
		      fprintf (stderr,
			       "Error in %s->%s->use[%i] & use[%i]:\n"
			       "  Only one link may use 'scheduling slot'"
			       " resources at time 0!\n",
			       entry->section->name, entry->name,
			       slot_index, index1);
		      customization_errors++;
		    }
		  else
		    {
		      slot_entry = use_entry;
		      slot_index = index1;
		    }
		}

	      /* Not good if only some options use a slot */
	      else if (option_count != 0)
		{
		  fprintf (stderr,
			   "Error in %s->%s->use[%i]->%s:\n"
			   "  All or none of the %i options must a use"
			   " 'scheduling slot' resource at time 0!\n",
			   entry->section->name, entry->name, index1,
			   use_entry->name,
			   one_of_field->max_element_index + 1);
		  customization_errors++;
		}
	    }
	  else
	    {
	      slot = CU_get_slot (use_entry);

	      if (slot != -1)
		{
		  /* Detect if more than one slot specified */
		  if (slot_entry != NULL)
		    {
		      fprintf (stderr,
			       "Error in %s->%s->use[%i] & use[%i]:\n"
			       "  Only one link may use 'scheduling slot'"
			       " resources at time 0!\n",
			       entry->section->name, entry->name,
			       slot_index, index1);
		      customization_errors++;
		    }
		  else
		    {
		      slot_entry = use_entry;
		      slot_index = index1;
		    }
		}
	    }
	}

      /* Exactly one 'use' should specify a slot */
      if (slot_entry == NULL)
	{
	  fprintf (stderr,
		   "Error in %s->%s->use[*]:\n"
		   "  One link must use a 'scheduling slot' resource"
		   " at time 0!\n", entry->section->name, entry->name);
	  customization_errors++;
	}
    }
}

/* Mark the resource units that use one or more 'scheduling slot' resource 
 * in time 0, with the smallest slot used.  Must be done after all
 * speed transformations are done (don't add units after this!).
 */
void
mark_resource_unit_slots (MD * md)
{
  MD_Section *unit_section;
  MD_Entry *unit_entry;
  MD_Field_Decl *slot_field_decl;
  MD_Field *slot_field;
  int slot;

  /* Get the resource unit section */
  unit_section = CU_find_section (md, "Resource_Unit");

  /* Delete the existing slot field declaration (if any).
   * This will delete the slot field for every entry.
   */
  if ((slot_field_decl = MD_find_field_decl (unit_section, "slot")) != NULL)
    MD_delete_field_decl (slot_field_decl);

  /* Create a optional slot field that takes one INT */
  slot_field_decl = MD_new_field_decl (unit_section, "slot",
				       MD_OPTIONAL_FIELD);
  MD_require_int (slot_field_decl, 0);

  /* Set the slot field for each unit that uses a slot in cycle 0 */
  for (unit_entry = MD_first_entry (unit_section); unit_entry != NULL;
       unit_entry = MD_next_entry (unit_entry))
    {
      slot = CU_get_slot (unit_entry);

      /* If has slot, create slot field and put the slot number in it */
      if (slot != -1)
	{
	  slot_field = MD_new_field (unit_entry, slot_field_decl, 1);
	  MD_set_int (slot_field, 0, slot);
	}
    }
}


/* Returns the number of options for this entry, returns 1 if
 * not a table option.
 */
int
CU_num_options (MD_Entry * entry)
{
  MD_Field *one_of_field;

  /* If a resource_unit or resource_usage, return 1 option */
  if (strcmp (entry->section->name, "Table_Option") != 0)
    return (1);

  /* Get the one_of_field */
  one_of_field = CU_find_field_by_name (entry, "one_of");

  /* Return the number of options */
  return (one_of_field->max_element_index + 1);
}

int
CU_max_usage_time (MD_Entry * entry)
{
  MD_Entry *target_entry;
  MD_Field *one_of_field, *use_field, *time_field;
  int max_usage_time, usage_time;
  int index1;

  /* Initialize max_usage_time for max search below */
  max_usage_time = -1;

  /* Recurse on options for tables */
  if (strcmp (entry->section->name, "Reservation_Table") == 0)
    {
      use_field = CU_find_field_by_name (entry, "use");

      /* Find the max usage time of all the options */
      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  target_entry = MD_get_link (use_field, index1);
	  usage_time = CU_max_usage_time (target_entry);
	  if (usage_time > max_usage_time)
	    max_usage_time = usage_time;
	}
#if DEBUG_OPTI
      printf ("Table %s: max time %i\n", entry->name, max_usage_time);
#endif
    }

  /* Recurse on units for table options */
  else if (strcmp (entry->section->name, "Table_Option") == 0)
    {
      one_of_field = CU_find_field_by_name (entry, "one_of");

      /* Find the max usage time of all the options */
      for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
	{
	  target_entry = MD_get_link (one_of_field, index1);
	  usage_time = CU_max_usage_time (target_entry);
	  if (usage_time > max_usage_time)
	    max_usage_time = usage_time;
	}
#if DEBUG_OPTI
      printf ("Option %s: max time %i\n", entry->name, max_usage_time);
#endif
    }

  /* Recurse one usages if resource unit */
  else if (strcmp (entry->section->name, "Resource_Unit") == 0)
    {
      use_field = CU_find_field_by_name (entry, "use");

      /* Find the max usage time of all the usages */
      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  target_entry = MD_get_link (use_field, index1);
	  usage_time = CU_max_usage_time (target_entry);
	  if (usage_time > max_usage_time)
	    max_usage_time = usage_time;
	}
#if DEBUG_OPTI
      printf ("  Unit %s: max time %i\n", entry->name, max_usage_time);
#endif
    }

  /* Set the usage time if a usage */
  else if (strcmp (entry->section->name, "Resource_Usage") == 0)
    {
      time_field = CU_find_field_by_name (entry, "time");

      /* Find the max usage time */
      for (index1 = 0; index1 <= time_field->max_element_index; index1++)
	{
	  usage_time = MD_get_int (time_field, index1);
	  if (usage_time > max_usage_time)
	    max_usage_time = usage_time;
	}
#if DEBUG_OPTI
      printf ("    Usage %s: max time %i\n", entry->name, max_usage_time);
#endif
    }
  else
    {
      C_punt (NULL, "CU_max_usage_time: Unknown section '%s'",
	      entry->section->name);
    }

  /* Return the max usage time found */
  return (max_usage_time);
}

/* Put the reservation tables in a format that:
 * 1) Places option or unit that defines the ops slot in slot_specifier()
 *    (it also remains in the use() field!)
 * 2) Find the max usage time of the table's usage and place in 
 *    the field max_usage_time().
 * 3) For opti_level >= 3,
 *    Sorts the options/units in use() by the number of options,
 *    so that the "most restrictive" options are checked first.
 *    In case of ties, place the option that sets the slot first.
 */
void
format_reservation_tables (MD * md)
{
  MD_Section *table_section, *link_array[2], *option_section;
  MD_Entry *table_entry, *use1_entry, *temp_entry, *new_option;
  MD_Field_Decl *table_slot_field_decl, *one_of_field_decl;
  MD_Field_Decl *table_time_field_decl;
  MD_Field *table_use_field, *table_slot_field, *one_of_field;
  MD_Field *table_time_field;
  int index1, slot, found_slot;
  int max_usage_time;
  char name_buf[5000];

  /* Get the reservation table section */
  table_section = CU_find_section (md, "Reservation_Table");


  /* Delete any existing slot_specifier field declaration */
  table_slot_field_decl = MD_find_field_decl (table_section,
					      "slot_specifier");
  if (table_slot_field_decl != NULL)
    MD_delete_field_decl (table_slot_field_decl);

  /* Create a new slot field declaration, required link to an
   * Table_Option or a Resource_Unit.  Points to the use link
   * that specifies the slot used by the table.
   */
  table_slot_field_decl = MD_new_field_decl (table_section, "slot_specifier",
					     MD_REQUIRED_FIELD);

  /* Make link array to make required link */
  link_array[0] = CU_find_section (md, "Table_Option");
  link_array[1] = CU_find_section (md, "Resource_Unit");

  /* Make required multitarget link */
  MD_require_multi_target_link (table_slot_field_decl, 0, 2, link_array);


  /* Delete any existing max_usage_time field declaration */
  table_time_field_decl = MD_find_field_decl (table_section,
					      "max_usage_time");
  if (table_time_field_decl != NULL)
    MD_delete_field_decl (table_time_field_decl);

  /* Create a new max_usage_time field declaration, requiring an int */
  table_time_field_decl = MD_new_field_decl (table_section, "max_usage_time",
					     MD_REQUIRED_FIELD);
  MD_require_int (table_time_field_decl, 0);



  /* Convert any units linked to in the table into table_options
   * with only one option.   Makes building sm mdes easier.
   * Will to classical optimizations afterwards to minimize size.
   */
  option_section = CU_find_section (md, "Table_Option");
  one_of_field_decl = CU_find_field_decl (option_section, "one_of");
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      /* Get the use field */
      table_use_field = CU_find_field_by_name (table_entry, "use");

      /* Convert all units found */
      for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
	{
	  /* Get the use at this index */
	  use1_entry = MD_get_link (table_use_field, index1);

	  /* If a resource unit, convert to option  */
	  if (strcmp (use1_entry->section->name, "Resource_Unit") == 0)
	    {
	      /* Create a new option that points to this unit */
	      CU_get_unique_name (md, NULL, table_entry->name, name_buf);
	      new_option = MD_new_entry (option_section, name_buf);
	      one_of_field = MD_new_field (new_option, one_of_field_decl, 1);
	      MD_set_link (one_of_field, 0, use1_entry);

#if DEBUG_OPTI
	      printf ("Converting %s[%i] from %s to %s\n",
		      table_entry->name, index1, use1_entry->name,
		      new_option->name);
#endif

	      /* Point table at option instead of unit */
	      MD_set_link (table_use_field, index1, new_option);
	    }

	  /* Better be a Table Option at this point */
	  else if (strcmp (use1_entry->section->name, "Table_Option") != 0)
	    {
	      C_punt (md,
		      "format_reservation_tables:\n"
		      "Table option or resource unit expected not %s->%s!",
		      use1_entry->section->name, use1_entry->name);
	    }
	}
    }

  /* Do classical optimizations to remove any duplicate options created */
  do_classical_optimizations (md);

  /* Find the slot specifying choice for each table and 
   * put it in the slot_specifier field
   */
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      /* Get the use field */
      table_use_field = CU_find_field_by_name (table_entry, "use");

      /* Create a slot_specifier field for this table in which to put the
       * slot link.
       */
      table_slot_field = MD_new_field (table_entry, table_slot_field_decl, 1);

      /* Place the 'slot' entry first */
      found_slot = 0;
      for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
	{
	  /* Get the use at this index */
	  use1_entry = MD_get_link (table_use_field, index1);

	  /* Put first if specifies a slot */
	  if ((slot = CU_get_slot (use1_entry)) != -1)
	    {
#if 0
	      /* Debug */
	      printf ("%s->use[%i] (%s) specifies slot %i\n",
		      table_entry->name, index1, use1_entry->name, slot);
#endif

	      /* Make sure we don't find two slots (should have
	       * been detected earlier!).
	       */
	      if (found_slot != 0)
		{
		  C_punt (md,
			  "format_reservation_tables:"
			  " two slots, should have already punted!");
		}
	      found_slot = 1;

	      /* Place entry that specifies slot, in the table slot field */
	      MD_set_link (table_slot_field, 0, use1_entry);

	      /* If want slot option first, swap entry 0 and this one,
	       * if not already first 
	       */
	      if ((place_slots_first) && (index1 != 0))
		{
		  /* Debug */
/*
		    printf ("Placing slot first!\n");
*/

		  temp_entry = MD_get_link (table_use_field, 0);
		  MD_set_link (table_use_field, 0, use1_entry);
		  MD_set_link (table_use_field, index1, temp_entry);
		}
	    }
	}

      /* Create max_usage_time field and put the max_usage_time it in */
      table_time_field = MD_new_field (table_entry, table_time_field_decl, 1);

      max_usage_time = CU_max_usage_time (table_entry);

      /* Sanity check, max_usage_time better be non-negative */
      if (max_usage_time < 0)
	C_punt (md, "format_reservation_tables: Bad max_usage_time!");

      /* Set the max usage time */
      MD_set_int (table_time_field, 0, max_usage_time);

      /* Don't do sort here. Use CU_order_use_fields() below. -JCG 6/8/96 */
#if 0
      /* Sort use() entries by number of options they have. 
       * In case of ties, place the slot specifying option first.
       *
       * If place_slots_first is true, the slot option is first,
       * and sort should start with second use.
       */
      MD_Entry *use2_entry;
      int num_options1, num_options2, min_options, min_index;
      int index2;

      if (place_slots_first)
	index1 = 1;
      else
	index1 = 0;
      for (; index1 <= table_use_field->max_element_index; index1++)
	{
	  /* Get the use at this index */
	  use1_entry = MD_get_link (table_use_field, index1);

	  /* Get the number of options it has */
	  num_options1 = CU_num_options (use1_entry);

	  /* If has only one option, don't move */
	  if (num_options1 == 1)
	    continue;

	  /* Search from the remaining options for the minimum number
	   * of options so we can place it at index1
	   */
	  min_options = num_options1;
	  min_index = index1;
	  for (index2 = index1 + 1;
	       index2 <= table_use_field->max_element_index; index2++)
	    {
	      use2_entry = MD_get_link (table_use_field, index2);
	      num_options2 = CU_num_options (use2_entry);


	      /*
	       *   Get the option with the least number of options,
	       *   in case of ties, pick the slot option first.
	       */
	      if ((num_options2 < min_options) ||
		  ((num_options2 == min_options) &&
		   (CU_get_slot (use2_entry) >= 0)))
		{
		  min_options = num_options2;
		  min_index = index2;
		}
	    }

	  /* Place the option selected at index1 if not already there */
	  if (min_index != index1)
	    {
	      temp_entry = MD_get_link (table_use_field, min_index);

#if DEBUG_OPTI
	      /* Debug */
	      printf ("Swapping %s use[%i] (%s %i options) and"
		      " use[%i] (%s %i options)\n",
		      table_entry->name, index1, use1_entry->name,
		      num_options1, min_index, temp_entry->name, min_options);
#endif

	      MD_set_link (table_use_field, min_index, use1_entry);
	      MD_set_link (table_use_field, index1, temp_entry);
	    }
	}
#endif
    }

  /* 
   * Applies transformations for opti_levels >= 3 only.
   *
   * Sort the Reseveration_Table->use() links and Resource_Unit->use()
   * links so that resources will be distributed the same in the various
   * forms of the same table, and so that the classical optimizations
   * can detect these duplicate tables consistently.
   * 
   * It also places checks near time 0 first, and reorders the
   * AND-OR trees (if opti_level >= 4) to place most restrictive 
   * checks first.
   *
   * Minimize the reordering.  Several bit-field optimizations have
   * been applied assuming this order as the final one, so don't 
   * rearrange unless have a strong reason.
   */
  CU_order_use_fields (md, 1);
}

/* Determines if a MDES parameter exists,   */
int
CU_parm_exists (MD * md, char *name)
{
  MD_Section *parm_section;

  parm_section = CU_find_section (md, "Parameter");

  /* Return 0 if parameter doesn't exist */
  if (MD_find_entry (parm_section, name) == NULL)
    return (0);

  /* Otherwise, return 1 */
  return (1);
}

/* Sets a string MDES parameter to the given value */
void
CU_set_string_parm (MD * md, char *name, char *string_value)
{
  MD_Section *parm_section;
  MD_Entry *parm_entry;
  MD_Field_Decl *value_field_decl;
  MD_Field *value_field;

  parm_section = CU_find_section (md, "Parameter");

  /* Delete the parameter if already exists */
  if ((parm_entry = MD_find_entry (parm_section, name)) != NULL)
    MD_delete_entry (parm_entry);

  /* Create a new parm entry */
  parm_entry = MD_new_entry (parm_section, name);

  /* Create a value field */
  value_field_decl = CU_find_field_decl (parm_section, "value");
  value_field = MD_new_field (parm_entry, value_field_decl, 1);

  /* Set the value field to the value specified */
  MD_set_string (value_field, 0, string_value);
}

/* Sets a int MDES parameter to the given value */
void
CU_set_int_parm (MD * md, char *name, int value)
{
  char value_string[100];
  sprintf (value_string, "%i", value);
  CU_set_string_parm (md, name, value_string);
}

void
expand_options (MD_Entry * dest_option, MD_Entry * src_option)
{
  MD_Entry *dest_unit, *src_usage, *src_unit, *new_unit;
  MD_Field *dest_one_of, *dest_use, *src_one_of, *src_use;
  int dest_index, src_index, index1;

  /* Get the destination's one_of field */
  dest_one_of = CU_find_field_by_name (dest_option, "one_of");

  /* Add the contents of each unit in src_option to the end of each
   * unit in dest_one_of.
   */
  for (dest_index = dest_one_of->max_element_index; dest_index >= 0;
       dest_index--)
    {
      dest_unit = MD_get_link (dest_one_of, dest_index);

      /* If the source option is just a unit, just tack on the
       * src's usages to each dest usage.
       */
      if (strcmp (src_option->section->name, "Resource_Unit") == 0)
	{
	  dest_use = CU_find_field_by_name (dest_unit, "use");
	  src_use = CU_find_field_by_name (src_option, "use");
	  for (src_index = 0; src_index <= src_use->max_element_index;
	       src_index++)
	    {
	      src_usage = MD_get_link (src_use, src_index);
	      MD_set_link (dest_use, dest_use->max_element_index + 1,
			   src_usage);
	    }
	}
      /* Duplicate the dest unit for each table option unit (except
       * the last one processed) and copy the table option's unit
       * to the end of this new unit.
       */
      else if (strcmp (src_option->section->name, "Table_Option") == 0)
	{
	  src_one_of = CU_find_field_by_name (src_option, "one_of");
	  for (src_index = src_one_of->max_element_index; src_index >= 0;
	       src_index--)
	    {
	      if (src_index != 0)
		{
		  new_unit = CU_dup_entry (dest_unit);
		  CU_add_link_at (dest_one_of, dest_index + 1, new_unit);
		}
	      else
		new_unit = dest_unit;
	      dest_use = CU_find_field_by_name (new_unit, "use");
	      src_unit = MD_get_link (src_one_of, src_index);
	      src_use = CU_find_field_by_name (src_unit, "use");
	      for (index1 = 0; index1 <= src_use->max_element_index; index1++)
		{
		  src_usage = MD_get_link (src_use, index1);
		  MD_set_link (dest_use, dest_use->max_element_index + 1,
			       src_usage);
		}

	    }
	}
      else
	{
	  C_punt (NULL, "expand_options: unexpected src_entry %s->%s",
		  src_option->section->name, src_option->name);
	}
    }

}

/* Expands the reservation table tree so that there is one resource
 * unit for each variation of the reservation table.  The transformed
 * table will have a use field linking to either one resource unit or 
 * one table option entry only.  The table option entry will contain
 * all of these expanded units. 
 * 
 * This algorithm puts the reservation tables into an equivalent form
 * as Elcor's compiler.  (They use alternatives for each option,
 * we use both alternatives and table options to enumerate out all
 * the options for scheduling effeciency with our implementation)
 */
void
expand_out_reservation_tables (MD * md)
{
  MD_Section *table_section, *option_section, *unit_section;
  MD_Entry *table_entry, *new_option, *src_option, *new_unit;
  MD_Field_Decl *one_of_field_decl, *use_field_decl;
  MD_Field *use_field, *new_one_of, *new_use;
  int index1;
  char name_buf[5000];

  /* Get the reservation table section */
  table_section = CU_find_section (md, "Reservation_Table");
  option_section = CU_find_section (md, "Table_Option");
  one_of_field_decl = CU_find_field_decl (option_section, "one_of");
  unit_section = CU_find_section (md, "Resource_Unit");
  use_field_decl = CU_find_field_decl (unit_section, "use");

  /* Expand out each reservation table entry */
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      /* Get the use field for this entry */
      use_field = CU_find_field_by_name (table_entry, "use");

      /* Skip use fields with only one choice */
      if (use_field->max_element_index == 0)
	continue;

#if DEBUG_OPTI
      printf ("Expanding %s\n", table_entry->name);
#endif

      /* Create a new table option and a new one_of field 
       * to contain links to all the resource units  we are going to create.
       */
      CU_get_unique_name (md, NULL, table_entry->name, name_buf);
      new_option = MD_new_entry (option_section, name_buf);
      new_one_of = MD_new_field (new_option, one_of_field_decl, 0);

      /* Create a option that points to one empty use */
      CU_get_unique_name (md, NULL, table_entry->name, name_buf);
      new_unit = MD_new_entry (unit_section, name_buf);
      new_use = MD_new_field (new_unit, use_field_decl, 0);
      MD_set_link (new_one_of, 0, new_unit);

      for (index1 = 0; index1 <= use_field->max_element_index; index1++)
	{
	  src_option = MD_get_link (use_field, index1);

#if DEBUG_OPTI
	  printf ("Adding '%s' options to '%s'\n", src_option->name,
		  new_option->name);
#endif
	  expand_options (new_option, src_option);
	  /* Delete this option */
	  MD_delete_element (use_field, index1);
	}
      /* Set the use_field to the new option that contains all
       * the variations of this operation.
       */
      MD_set_link (use_field, 0, new_option);
    }
}

void
calc_forbidden_latencies (INT_Symbol_Table * forbidden_set,
			  MD_Field * use1_field, MD_Field * use2_field)
{
  MD_Section *usage_section;
  MD_Entry *usage1_entry, *usage2_entry, *resource1, *resource2;
  MD_Field_Decl *usage_use_decl, *usage_time_decl;
  MD_Field *usage1_use_field, *usage2_use_field;
  MD_Field *usage1_time_field, *usage2_time_field;
  int time1, time2;
  int index1, index2;
  int latency;

  /* Get the usage_use and usage_time declarations to speed up the 
   * search.
   */
  usage_section = CU_find_section (use1_field->entry->section->md,
				   "Resource_Usage");
  usage_use_decl = CU_find_field_decl (usage_section, "use");
  usage_time_decl = CU_find_field_decl (usage_section, "time");


  /* Generate a forbidden latency for each pair of usages that
   * share the same resource.  Generate only latencies >= 0.
   */
  for (index1 = 0; index1 <= use1_field->max_element_index; index1++)
    {
      /* Get the first usage's resource and time usage */
      usage1_entry = MD_get_link (use1_field, index1);

      usage1_use_field = MD_find_field (usage1_entry, usage_use_decl);
      resource1 = MD_get_link (usage1_use_field, 0);

      usage1_time_field = MD_find_field (usage1_entry, usage_time_decl);
      time1 = MD_get_int (usage1_time_field, 0);

      for (index2 = 0; index2 <= use2_field->max_element_index; index2++)
	{
	  /* Get the second usage's resource and time usage */
	  usage2_entry = MD_get_link (use2_field, index2);

	  usage2_use_field = MD_find_field (usage2_entry, usage_use_decl);
	  resource2 = MD_get_link (usage2_use_field, 0);

	  /* If are not using the same resource, no forbidden latency */
	  if (resource1 != resource2)
	    continue;

	  /* Calculate the forbidden latency for the two times */
	  usage2_time_field = MD_find_field (usage2_entry, usage_time_decl);
	  time2 = MD_get_int (usage2_time_field, 0);

	  latency = time2 - time1;

	  /* Don't use negative forbidden latencies, so don't add
	   * into forbidden set. -JCG 4/11/96
	   */
	  if (latency < 0)
	    continue;

	  /* Add latency to forbidden set if not already there */
	  if (INT_find_symbol (forbidden_set, latency) == NULL)
	    {
	      INT_add_symbol (forbidden_set, latency, NULL);

#if DEBUG_OPTI
	      printf ("   Adding %i to F[%s][%s] due to %s at %i and %i\n",
		      latency, use1_field->entry->name,
		      use2_field->entry->name, resource1->name, time1, time2);
#endif
	    }
	}
    }
}

void
add_usage (Resource_Info * resource, int user, int time)
{
  INT_Symbol_Table *usage;
  char name_buf[100];

  /* See if there is any usages yet at the given time */
  usage = (INT_Symbol_Table *) INT_find_symbol_data (resource->time_table,
						     time);

  /* If not, add one */
  if (usage == NULL)
    {
      sprintf (name_buf, "%s_t%i", resource->name, time);
      usage = INT_new_symbol_table (name_buf, 0);
      INT_add_symbol (resource->time_table, time, (void *) usage);
    }

  /* Add this user's usage if not already there */
  if (INT_find_symbol (usage, user) == NULL)
    {
      INT_add_symbol (usage, user, NULL);
    }
}

/* Returns 1 if usage for this resource already exists */
int
usage_exists (Resource_Info * resource, int user, int time)
{
  INT_Symbol_Table *usage;

  /* See if there is any usages yet at the given time */
  usage = (INT_Symbol_Table *) INT_find_symbol_data (resource->time_table,
						     time);

  /* If not, usage doesn't exist */
  if (usage == NULL)
    return (0);

  /* Is this usage there? */
  if (INT_find_symbol (usage, user) == NULL)
    return (0);
  else
    return (1);
}

/* Returns 1 if F[user1][user2] has latency in its forbidden set.
 * Otherwise returns 0
 */
int
is_forbidden (INT_Symbol_Table *** forbidden_matrix,
	      int user1, int user2, int latency)
{
  INT_Symbol_Table *forbidden_set;
  int temp_user;

  /* Handle negative latencies by swapping users, and
   * negating latency. -JCG 4/11/96
   */
  if (latency < 0)
    {
      temp_user = user1;
      user1 = user2;
      user2 = temp_user;

      latency = -latency;
    }

  forbidden_set = forbidden_matrix[user1][user2];

  if (INT_find_symbol (forbidden_set, latency) == NULL)
    return (0);
  else
    return (1);
}

/* Returns 1 if the new_user can be added to the resource at new_time
 * without adding any forbidden latencies not in the forbidden matrix.
 * Returns 0 otherwise.
 */
int
is_compatable (INT_Symbol_Table *** forbidden_matrix,
	       Resource_Info * resource, int new_user, int new_time)
{
  INT_Symbol *time_symbol, *usage_symbol;
  INT_Symbol_Table *usage_table;
  int test_user, test_time, latency;

  /* Check compatability of user at time with every usage of this resource */
  for (time_symbol = resource->time_table->head_symbol; time_symbol != NULL;
       time_symbol = time_symbol->next_symbol)
    {
      /* Get the time for this usage table */
      test_time = time_symbol->value;

      /* Get the usage table for this time */
      usage_table = (INT_Symbol_Table *) time_symbol->data;

      /* Test all usages of this resource at this time */
      for (usage_symbol = usage_table->head_symbol; usage_symbol != NULL;
	   usage_symbol = usage_symbol->next_symbol)
	{
	  /* Get the existing user to test */
	  test_user = usage_symbol->value;

	  /* Calculate the forbidden latency this will add */
	  latency = test_time - new_time;

	  /* If adding new_user will create a forbidden latency
	   * that is not in the original matrix, return 0
	   * (The order of new_user and test_user args must be 
	   *  consistent with latency calculation!)
	   */
	  if (!is_forbidden (forbidden_matrix, new_user, test_user, latency))
	    return (0);
	}
    }

  /* If have reached here, must be compatable */
  return (1);
}

void
add_to_generating_set (INT_Symbol_Table *** forbidden_matrix,
		       INT_Symbol_Table * set, int user1, int user2,
		       int latency)
{
  INT_Symbol *resource_symbol;
  Resource_Info *resource;
  int resources_modified;

  /* Initially no resources are modified */
  resources_modified = 0;

  /* Try to add this elementary pair to each resource already in the
   * generating set.
   */
  for (resource_symbol = set->head_symbol; resource_symbol != NULL;
       resource_symbol = resource_symbol->next_symbol)
    {
      /* Get the resource for this symbol */
      resource = (Resource_Info *) resource_symbol->data;

      /* If a slot resource, prevent adding new usages at time 0 */
      if (resource->flags & SLOT_RESOURCE)
	{
#if DEBUG_OPTI
	  printf ("Considering adding of %s user1 %i user2 %i latency %i\n",
		  resource->name, user1, user2, latency);
#endif

	  /* Don't allow adding of user1 at time 0 */
	  if (!usage_exists (resource, user1, 0))
	    continue;

	  /* Don't allow adding of user2 at time 0 */
	  if ((latency == 0) && !usage_exists (resource, user2, 0))
	    continue;

#if DEBUG_OPTI
	  printf ("Allowing adding of %s user1 %i user2 %i latency %i\n",
		  resource->name, user1, user2, latency);
#endif
	}

      /* Is the use of user1 at time 0 and user2 at latency 
       * compatible with this resource? */
      if (is_compatable (forbidden_matrix, resource, user1, 0) &&
	  is_compatable (forbidden_matrix, resource, user2, latency))
	{
#if DEBUG_OPTI
	  printf
	    ("Unit %i time %i and unit %i at time %i is compatable with "
	     "resource %s\n",
	     user1, 0, user2, latency, resource->name);
#endif

	  /* Add these usages to this compatable resource */
	  add_usage (resource, user1, 0);
	  add_usage (resource, user2, latency);

	  /* Update the resources modified count */
	  resources_modified++;
	}
    }

#if 0
  /* We add all the resources up front, so this usage must be compatable
   * with at least one of them!!!  NOT TRUE... can have the following
   * situation (only table):
   * Resource1:
   *    time0: 3
   *    time1: 5
   *    time2: 7
   * 
   * Would want to add 5, 7, lat 1, (5 at time 0, 7 at time 1), which
   * is not compatable.  I didn't see any cases where it would
   * be usefull to create a new resource with this usage, so I am not.
   * -JCG 4/6/96
   */
  if (resources_modified == 0)
    {
      C_punt (NULL,
	      "add_to_generating_set:\n"
	      "Should not need to add resource for user1 %i user2 %i lat %i!",
	      user1, user2, latency);
    }
#endif

}

/* Returns 1 if every usage in resource2 is in resource1 */
int
resource_redundant (Resource_Info * resource1, Resource_Info * resource2)
{
  INT_Symbol *time2_symbol, *usage2_symbol;
  INT_Symbol_Table *usage1_table, *usage2_table;
  int time, user;

  /* For every usage time of resource2, make sure resource1 also
   * has the same usage.
   */
  for (time2_symbol = resource2->time_table->head_symbol;
       time2_symbol != NULL; time2_symbol = time2_symbol->next_symbol)
    {
      /* Get the time for this usage table */
      time = time2_symbol->value;

      /* Get the usage of this time for resource1, if not found
       * then not redundant
       */
      usage1_table =
	(INT_Symbol_Table *) INT_find_symbol_data (resource1->time_table,
						   time);
      if (usage1_table == NULL)
	return (0);

      /* Get the usage table for this time for resource2 */
      usage2_table = (INT_Symbol_Table *) time2_symbol->data;

      /* Test all usages of this resource at this time */
      for (usage2_symbol = usage2_table->head_symbol; usage2_symbol != NULL;
	   usage2_symbol = usage2_symbol->next_symbol)
	{
	  /* Get a user of this resource at this time */
	  user = usage2_symbol->value;

	  /* Return 0 if not in usage1_table */
	  if (INT_find_symbol (usage1_table, user) == NULL)
	    return (0);
	}
    }

  /* If got here, must be redundant */
  return (1);
}

void
delete_resource (Resource_Info * resource)
{
  INT_Symbol *time_symbol, *next_time_symbol;
  INT_Symbol_Table *usage_table;

  /* For every usage time of resource2, make sure resource1 also
   * has the same usage.
   */
  for (time_symbol = resource->time_table->head_symbol; time_symbol != NULL;
       time_symbol = next_time_symbol)
    {
      /* Get the next time symbol before deleting this one */
      next_time_symbol = time_symbol->next_symbol;

      /* Get the usage table for this time */
      usage_table = (INT_Symbol_Table *) time_symbol->data;

      /* Delete the symbol table */
      INT_delete_symbol_table (usage_table, NULL);

      /* Delete the time symbol */
      INT_delete_symbol (time_symbol, NULL);
    }

  /* Delete the resource table */
  INT_delete_symbol_table (resource->time_table, NULL);

  /* Free the resource info */
  free (resource);
}

INT_Symbol_Table *
build_generating_set (MD * md,
		      INT_Symbol_Table *** forbidden_matrix,
		      MD_Entry ** unit_array, int num_units)
{
  Resource_Info *resource;
  MD_Section *resource_section, *unit_section, *usage_section;
  MD_Entry *resource_entry, *unit_entry, *usage_entry, *resource_id;
  MD_Field_Decl *unit_use_field_decl, *usage_use_field_decl;
  MD_Field_Decl *usage_time_field_decl, *slot_field_decl;
  MD_Field *unit_use_field, *usage_use_field, *usage_time_field;
  INT_Symbol_Table *time_table;
  Resource_Info *resource1, *resource2;
  INT_Symbol_Table *generating_set;
  INT_Symbol *resource1_symbol, *resource2_symbol;
  INT_Symbol *next_resource2_symbol;
  int index2;
  int user1, time;
  int need_redundant_header;

  /* Create the table to hold the generating set,
   * make it initially big to hold all these pointer references ok
   */
  generating_set = INT_new_symbol_table ("generating_set", 256);

  /* Create all the resources currently in mdes */
  resource_section = CU_find_section (md, "Resource");
  slot_field_decl = CU_find_field_decl (resource_section, "slot");
  for (resource_entry = MD_first_entry (resource_section);
       resource_entry != NULL;
       resource_entry = MD_next_entry (resource_entry))
    {
      time_table = INT_new_symbol_table (resource_entry->name, 0);
      resource = (Resource_Info *) malloc (sizeof (Resource_Info));
      if (resource == NULL)
	C_punt (NULL, "Out of memory");

      resource->resource_entry = resource_entry;
      resource->name = resource_entry->name;
      resource->time_table = time_table;
      resource->flags = 0;

      /* Mark slot resources */
      if (MD_find_field (resource_entry, slot_field_decl) != NULL)
	{
	  resource->flags |= SLOT_RESOURCE;
	}

      /* 11/14/02 REK Strip the high 32 bits (if applicable) off of 
       *              resource_entry before looking it up. */
      INT_add_symbol (generating_set, 
		      ((long) resource_entry & (long) 0xffffffff),
		      (void *) resource);
    }

  unit_section = CU_find_section (md, "Resource_Unit");
  unit_use_field_decl = CU_find_field_decl (unit_section, "use");
  usage_section = CU_find_section (md, "Resource_Usage");
  usage_use_field_decl = CU_find_field_decl (usage_section, "use");
  usage_time_field_decl = CU_find_field_decl (usage_section, "time");
  for (user1 = 0; user1 < num_units; user1++)
    {
      unit_entry = unit_array[user1];
      unit_use_field = MD_find_field (unit_entry, unit_use_field_decl);

      for (index2 = 0; index2 <= unit_use_field->max_element_index; index2++)
	{
	  usage_entry = MD_get_link (unit_use_field, index2);

	  usage_use_field = MD_find_field (usage_entry, usage_use_field_decl);

	  resource_id = MD_get_link (usage_use_field, 0);

	  usage_time_field = MD_find_field (usage_entry,
					    usage_time_field_decl);
	  time = MD_get_int (usage_time_field, 0);

	  /* Find the resource being used */
	  /* 11/14/02 REK Strip the high 32 bits (if applicable) off of 
	   *              resource_id before looking it up. */
	  resource = (Resource_Info *) \
	    INT_find_symbol_data (generating_set,
				  ((long) resource_id & (long) 0xffffffff));

	  /* Better not be NULL */
	  if (resource == NULL)
	    C_punt (NULL, "build_generating_set: resource not found!");

	  /* Add this unit as a user of this resource */
	  add_usage (resource, user1, time);
	}
    }

#if DEBUG_OPTI
  printf ("\n\nInitial generating set:\n");
  print_generating_set (stdout, generating_set, unit_array, 0);
#endif

  need_redundant_header = 1;

  /* Remove redundant resources */
  for (resource1_symbol = generating_set->head_symbol;
       resource1_symbol != NULL;
       resource1_symbol = resource1_symbol->next_symbol)
    {
      resource1 = (Resource_Info *) resource1_symbol->data;
      for (resource2_symbol = generating_set->head_symbol;
	   resource2_symbol != NULL; resource2_symbol = next_resource2_symbol)
	{
	  /* Get the next resource2_symbol before perhaps deleting this
	   * one.
	   */
	  next_resource2_symbol = resource2_symbol->next_symbol;

	  resource2 = (Resource_Info *) resource2_symbol->data;

	  /* Don't test against itself */
	  if (resource1 == resource2)
	    continue;

	  if (resource_redundant (resource1, resource2))
	    {
	      if (!(resource2->flags & SLOT_RESOURCE))
		{
#if DEBUG_OPTI
		  printf ("Given %s, %s is redundant... Removing.\n",
			  resource1->name, resource2->name);
#endif

		  if (need_redundant_header)
		    {
		      printf ("> Deleting redundant resources:");
		      need_redundant_header = 0;
		    }
		  printf (" %s", resource2->name);
		  delete_resource (resource2);
		  INT_delete_symbol (resource2_symbol, NULL);
		}
	      else
		{
#if DEBUG_OPTI
		  printf
		    ("Given %s, %s is redundant... but cannot delete slot "
		     "resource.\n",
		     resource1->name, resource2->name);
#endif
		}
	    }
	}
    }
  /* Terminate list of redundant resources if necesary */
  if (!need_redundant_header)
    printf ("\n");

  /* Can "fill out" the generating set to allow shifting around
   * of the resource usages.  May help a little, but didn't make
   * a big difference in our machine descriptions.
   * If do uncomment code, make sure not to add extra slot usages
   * (and prevent the deletion of slots currently used... Taken care
   *  of 4/6/96 -JCG)
   *
   */
#if DEBUG_OPTI
  printf ("\n Filling out generating set:\n");

  /* Create all the resources necessary to generate all of
   * the forbidden latencies.
   */
  int index1, latency;
  for (index1 = 0; index1 < num_units; index1++)
    {
      INT_Symbol_Table **forbidden_row, *forbidden_set;
      forbidden_row = forbidden_matrix[index1];
      for (index2 = 0; index2 < num_units; index2++)
	{
	  forbidden_set = forbidden_row[index2];

	  /* Add each forbidden latency to tghe generating set */
	  INT_Symbol *symbol;
	  for (symbol = forbidden_set->head_symbol; symbol != NULL;
	       symbol = symbol->next_symbol)
	    {
	      /* Get the forbidden latency */
	      latency = symbol->value;

	      /* Only process latencies >= 0 */
	      if (latency < 0)
		continue;

	      add_to_generating_set (forbidden_matrix, generating_set,
				     index1, index2, latency);
	    }
	}
    }
#endif

  return (generating_set);
}




void
enumerate_pairs (INT_Symbol_Table *** forbidden_matrix,
		 Resource_Info * resource)
{
  Pair_Node *pnode;
  INT_Symbol *time1_symbol, *usage1_symbol, *forbidden_symbol;
  INT_Symbol *time2_symbol, *usage2_symbol;
  INT_Symbol_Table *usage1_table, *usage2_table;
  INT_Symbol_Table *forbidden_set;
  int user1, user2, time1, time2, latency;

#if DEBUG_OPTI
  printf ("\nFor resource %s:\n", resource->name);
#endif

  for (time1_symbol = resource->time_table->head_symbol;
       time1_symbol != NULL; time1_symbol = time1_symbol->next_symbol)
    {
      /* Get the time for this usage table */
      time1 = time1_symbol->value;

      /* Get the usage table for this time */
      usage1_table = (INT_Symbol_Table *) time1_symbol->data;

      for (time2_symbol = resource->time_table->head_symbol;
	   time2_symbol != NULL; time2_symbol = time2_symbol->next_symbol)
	{
	  /* Get the time for this usage table */
	  time2 = time2_symbol->value;

	  /* Get the usage table for this time */
	  usage2_table = (INT_Symbol_Table *) time2_symbol->data;

	  latency = time2 - time1;

	  /* Only add positive latencies */
	  if (latency < 0)
	    continue;

	  /* Test all usages of this resource at this time */
	  for (usage1_symbol = usage1_table->head_symbol;
	       usage1_symbol != NULL;
	       usage1_symbol = usage1_symbol->next_symbol)
	    {
	      /* Get the existing user to test */
	      user1 = usage1_symbol->value;

	      for (usage2_symbol = usage2_table->head_symbol;
		   usage2_symbol != NULL;
		   usage2_symbol = usage2_symbol->next_symbol)
		{
		  /* Get the existing user to test */
		  user2 = usage2_symbol->value;


		  forbidden_set = forbidden_matrix[user1][user2];

		  forbidden_symbol = INT_find_symbol (forbidden_set, latency);

		  pnode = (Pair_Node *) malloc (sizeof (Pair_Node));
		  if (pnode == NULL)
		    C_punt (NULL, "Out of memory");
		  pnode->resource = resource;
		  pnode->time1 = time1;
		  pnode->next = (Pair_Node *) forbidden_symbol->data;
		  forbidden_symbol->data = (void *) pnode;

#if DEBUG_OPTI
		  printf ("Adding user1 %i, user2 %i time1 %i, time2 = %i\n",
			  user1, user2, time1, time2);
#endif
		}
	    }

	}
    }
}
int
mark_covered (INT_Symbol_Table *** forbidden_matrix,
	      Resource_Info * resource, int user, int time)
{
  INT_Symbol_Table *forbidden_set;
  INT_Symbol_Table *usage2_table, *usage1_table;
  INT_Symbol *time1_symbol, *usage1_symbol;
  INT_Symbol *time2_symbol, *usage2_symbol, *forbidden_symbol;
  Pair_Node *pnode, *next_pnode;
  int time1, time2, user1, user2, latency, num_covered;

  num_covered = 0;

  /* Mark those covered latencies covered where user is user1 */
  user1 = user;
  time1 = time;

  for (time2_symbol = resource->time_table->head_symbol;
       time2_symbol != NULL; time2_symbol = time2_symbol->next_symbol)
    {
      /* Get the time for this usage table */
      time2 = time2_symbol->value;

      /* Get the usage table for this time */
      usage2_table = (INT_Symbol_Table *) time2_symbol->data;

      /* Only process non-negative latencies */
      latency = time2 - time1;
      if (latency < 0)
	continue;

      for (usage2_symbol = usage2_table->head_symbol;
	   usage2_symbol != NULL; usage2_symbol = usage2_symbol->next_symbol)
	{
	  /* Mark only those latencies associated with committed usages */
	  if (usage2_symbol->data == 0)
	    continue;

	  /* Get the existing user to test */
	  user2 = usage2_symbol->value;

	  forbidden_set = forbidden_matrix[user1][user2];

	  forbidden_symbol = INT_find_symbol (forbidden_set, latency);

	  /* Better not be null */
	  if (forbidden_symbol == NULL)
	    {
	      C_punt (NULL,
		      "mark_covered: NULL forbidden_symbol!\n"
		      "For resource %s: user1 %i user2 %i, time1 %i, "
		      "time2 %i\n",
		      resource->name, user1, user2, time1, time2);
	    }

	  /* Continue if has already been covered */
	  if (forbidden_symbol->data == NULL)
	    continue;

	  /* Free pair nodes */
	  for (pnode = (Pair_Node *) forbidden_symbol->data; pnode != NULL;
	       pnode = next_pnode)
	    {
	      /* Get the next pnode before deleting this one */
	      next_pnode = pnode->next;

	      free (pnode);
	    }

	  /* Mark as covered */
	  forbidden_symbol->data = NULL;

	  /* Update covered count */
	  num_covered++;
	}
    }


  /* Mark those covered latencies covered where user is user2 */
  user2 = user;
  time2 = time;

  for (time1_symbol = resource->time_table->head_symbol;
       time1_symbol != NULL; time1_symbol = time1_symbol->next_symbol)
    {
      /* Get the time for this usage table */
      time1 = time1_symbol->value;

      /* Get the usage table for this time */
      usage1_table = (INT_Symbol_Table *) time1_symbol->data;

      /* Only process non-negative latencies */
      latency = time2 - time1;
      if (latency < 0)
	continue;

      for (usage1_symbol = usage1_table->head_symbol;
	   usage1_symbol != NULL; usage1_symbol = usage1_symbol->next_symbol)
	{
	  /* Mark only those latencies associated with committed usages */
	  if (usage1_symbol->data == 0)
	    continue;

	  /* Get the existing user to test */
	  user1 = usage1_symbol->value;

	  forbidden_set = forbidden_matrix[user1][user2];

	  forbidden_symbol = INT_find_symbol (forbidden_set, latency);

	  /* Better not be null */
	  if (forbidden_symbol == NULL)
	    {
	      C_punt (NULL,
		      "mark_covered: NULL forbidden_symbol!\n"
		      "For resource %s: user1 %i user2 %i, time1 %i, "
		      "time2 %i\n",
		      resource->name, user1, user2, time1, time2);
	    }

	  /* Continue if has already been covered */
	  if (forbidden_symbol->data == NULL)
	    continue;

	  /* Free pair nodes */
	  for (pnode = (Pair_Node *) forbidden_symbol->data; pnode != NULL;
	       pnode = next_pnode)
	    {
	      /* Get the next pnode before deleting this one */
	      next_pnode = pnode->next;

	      free (pnode);
	    }

	  /* Mark as covered */
	  forbidden_symbol->data = NULL;

	  /* Update covered count */
	  num_covered++;
	}
    }

  return (num_covered);
}

void
commit_usage (INT_Symbol_Table *** forbidden_matrix,
	      Resource_Info * resource, int user, int time)
{
  INT_Symbol_Table *usage;
  INT_Symbol *user_usage;

  /* Make sure there is a usage for the given time! */
  usage = (INT_Symbol_Table *) INT_find_symbol_data (resource->time_table,
						     time);
  if (usage == NULL)
    C_punt (NULL, "commit_usage: usage for time %i expected!\n", time);


  /* Get this user's usage */
  if ((user_usage = INT_find_symbol (usage, user)) == NULL)
    C_punt (NULL, "commit_usage: user_usage for user %i expected!\n", user);

  /* If user_usage already commited, return now */
  if (user_usage->data != NULL)
    return;

  /* Mark the user_usage as committed */
  user_usage->data = (void *) 1;

  /* Mark the forbidden matrix latencies covered by this usage as
   * covered.
   */
  mark_covered (forbidden_matrix, resource, user, time);
}

int
is_usage_committed (INT_Symbol_Table *** forbidden_matrix,
		    Resource_Info * resource, int user, int time)
{
  INT_Symbol_Table *usage;
  INT_Symbol *user_usage;

  /* Make sure there is a usage for the given time! */
  usage = (INT_Symbol_Table *) INT_find_symbol_data (resource->time_table,
						     time);
  if (usage == NULL)
    C_punt (NULL, "is_usage_committed: usage for time %i expected!\n", time);


  /* Get this user's usage */
  if ((user_usage = INT_find_symbol (usage, user)) == NULL)
    C_punt (NULL,
	    "is_usage_committed: user_usage for user %i expected!\n", user);

  /* If user_usage already commited, return 1, otherwise 0 */
  if (user_usage->data != NULL)
    return (1);
  else
    return (0);
}

int
count_num_covered (INT_Symbol_Table *** forbidden_matrix,
		   Resource_Info * resource, int user, int time)
{
  INT_Symbol_Table *forbidden_set;
  INT_Symbol_Table *usage2_table, *usage1_table;
  INT_Symbol *time1_symbol, *usage1_symbol;
  INT_Symbol *time2_symbol, *usage2_symbol, *forbidden_symbol;
  int time1, time2, user1, user2, latency, num_covered;

  num_covered = 0;

  /* Count those covered latencies covered where user is user1 */
  user1 = user;
  time1 = time;

  for (time2_symbol = resource->time_table->head_symbol;
       time2_symbol != NULL; time2_symbol = time2_symbol->next_symbol)
    {
      /* Get the time for this usage table */
      time2 = time2_symbol->value;

      /* Get the usage table for this time */
      usage2_table = (INT_Symbol_Table *) time2_symbol->data;

      /* Only process non-negative latencies */
      latency = time2 - time1;
      if (latency < 0)
	continue;

      for (usage2_symbol = usage2_table->head_symbol;
	   usage2_symbol != NULL; usage2_symbol = usage2_symbol->next_symbol)
	{
	  /* Mark only those latencies associated with committed usages */
	  if (usage2_symbol->data == 0)
	    continue;

	  /* Get the existing user to test */
	  user2 = usage2_symbol->value;

	  forbidden_set = forbidden_matrix[user1][user2];

	  forbidden_symbol = INT_find_symbol (forbidden_set, latency);

	  /* Better not be null */
	  if (forbidden_symbol == NULL)
	    {
	      C_punt (NULL,
		      "count_num_covered: NULL forbidden_symbol!\n"
		      "For resource %s: user1 %i user2 %i, time1 %i, "
		      "time2 %i\n",
		      resource->name, user1, user2, time1, time2);
	    }

	  /* Continue if has already been covered */
	  if (forbidden_symbol->data == NULL)
	    continue;

	  /* Update covered count */
	  num_covered++;
	}
    }


  /* Mark those covered latencies covered where user is user2 */
  user2 = user;
  time2 = time;

  for (time1_symbol = resource->time_table->head_symbol;
       time1_symbol != NULL; time1_symbol = time1_symbol->next_symbol)
    {
      /* Get the time for this usage table */
      time1 = time1_symbol->value;

      /* Get the usage table for this time */
      usage1_table = (INT_Symbol_Table *) time1_symbol->data;

      /* Only process positive latencies (don't count 0 latencies twice) */
      latency = time2 - time1;
      if (latency < 0)
	continue;

      for (usage1_symbol = usage1_table->head_symbol;
	   usage1_symbol != NULL; usage1_symbol = usage1_symbol->next_symbol)
	{
	  /* Mark only those latencies associated with committed usages */
	  if (usage1_symbol->data == 0)
	    continue;

	  /* Get the existing user to test */
	  user1 = usage1_symbol->value;

	  forbidden_set = forbidden_matrix[user1][user2];

	  forbidden_symbol = INT_find_symbol (forbidden_set, latency);

	  /* Better not be null */
	  if (forbidden_symbol == NULL)
	    {
	      C_punt (NULL,
		      "mark_covered: NULL forbidden_symbol!\n"
		      "For resource %s: user1 %i user2 %i, time1 %i, "
		      "time2 %i\n",
		      resource->name, user1, user2, time1, time2);
	    }

	  /* Continue if has already been covered */
	  if (forbidden_symbol->data == NULL)
	    continue;

	  /* Update covered count */
	  num_covered++;
	}
    }

  return (num_covered);
}

void
minimize_generating_set (INT_Symbol_Table * generating_set,
			 INT_Symbol_Table *** forbidden_matrix,
			 int num_units, MD_Entry ** unit_array)
{
  INT_Symbol *resource_symbol, *forbidden_symbol, *user_symbol;
  INT_Symbol_Table *usage_table;
  Resource_Info *resource;
  Resource_Info *min_resource;
  INT_Symbol_Table **forbidden_row, *forbidden_set;
  Pair_Node *phead, *pnode, *best_pnode;
  int num_covered;
  int user1, user2, latency, count, max_count, num_left;
  int min_user1 = 0, min_user2 = 0, min_time1 = 0, min_time2 = 0;
  int min_count = 0, min_covered = 0;
  int best_covered;
  int time1, time2;

  for (resource_symbol = generating_set->head_symbol;
       resource_symbol != NULL;
       resource_symbol = resource_symbol->next_symbol)
    {
      /* Get the resource for this symbol */
      resource = (Resource_Info *) resource_symbol->data;

      enumerate_pairs (forbidden_matrix, resource);
    }

  /* After all the usage pairs are enumerated, commit the usages of
   * slot resources at time (and remove the associated pairs), so
   * that the scheduler and user will not get confused by deleted or
   * different slot usage.
   */
#if DEBUG_OPTI
  printf ("Committing slot usages at time 0\n");
#endif
  for (resource_symbol = generating_set->head_symbol;
       resource_symbol != NULL;
       resource_symbol = resource_symbol->next_symbol)
    {
      /* Get the resource for this symbol */
      resource = (Resource_Info *) resource_symbol->data;

      /* If not a slot resource, process next resource */
      if (!(resource->flags & SLOT_RESOURCE))
	continue;

      /* Get the usage table for time 0, continue if don't exist */
      usage_table =
	(INT_Symbol_Table *) INT_find_symbol_data (resource->time_table, 0);
      if (usage_table == NULL)
	continue;

      /* Commit all usages of this resource at time 0 */
      for (user_symbol = usage_table->head_symbol; user_symbol != NULL;
	   user_symbol = user_symbol->next_symbol)
	{
#if DEBUG_OPTI
	  printf ("Committing slot usage %s user %i time 0\n",
		  resource->name, user_symbol->value);
#endif
	  commit_usage (forbidden_matrix, resource, user_symbol->value, 0);
	}


    }

#if DEBUG_OPTI
  printf ("Commiting unique pairs\n");
#endif

  max_count = 0;
  num_left = 0;
  for (user1 = 0; user1 < num_units; user1++)
    {
      forbidden_row = forbidden_matrix[user1];
      for (user2 = 0; user2 < num_units; user2++)
	{
	  forbidden_set = forbidden_row[user2];

	  for (forbidden_symbol = forbidden_set->head_symbol;
	       forbidden_symbol != NULL;
	       forbidden_symbol = forbidden_symbol->next_symbol)
	    {
	      /* Get the resource for this symbol */
	      latency = forbidden_symbol->value;

	      /* Only process non-negative latencies */
	      if (latency < 0)
		continue;

	      /* Only process pairs for latencies that have
	       * not already been covered.
	       */
	      if (forbidden_symbol->data == NULL)
		continue;

	      phead = (Pair_Node *) forbidden_symbol->data;

	      count = 0;
	      for (pnode = phead; pnode != NULL; pnode = pnode->next)
		{
		  count++;
		}
#if DEBUG_OPTI
	      printf ("User1 %i user2 %i latency %i  pairs %i\n",
		      user1, user2, latency, count);
#endif

	      if (count > max_count)
		max_count = count;

	      if (count == 1)
		{
		  /* Committing a usage pair will cause that pair
		   * to be freed... Get information from pair
		   * before committing!  (Detected with Mem Advisor)
		   */
		  resource = phead->resource;
		  time1 = phead->time1;
		  time2 = latency + phead->time1;
		  commit_usage (forbidden_matrix, resource, user1, time1);
		  commit_usage (forbidden_matrix, resource, user2, time2);
		}
	      else
		{
		  num_left++;
		}
	    }
	}
    }

  /* While there are latencies uncovered, pick an usage pair that
   * covers the most remaining latencies.  See below for
   * the rules for selecting pairs in case of ties.
   */
  while (num_left > 0)
    {
      num_left = 0;
      min_resource = NULL;

#if DEBUG_OPTI
      printf ("What is left:\n");
      print_generating_set (stdout, generating_set, unit_array, 1);
#endif
      for (user1 = 0; user1 < num_units; user1++)
	{
	  forbidden_row = forbidden_matrix[user1];
	  for (user2 = 0; user2 < num_units; user2++)
	    {
	      forbidden_set = forbidden_row[user2];

	      for (forbidden_symbol = forbidden_set->head_symbol;
		   forbidden_symbol != NULL;
		   forbidden_symbol = forbidden_symbol->next_symbol)
		{
		  /* Get the resource for this symbol */
		  latency = forbidden_symbol->value;

		  /* Only process non-negative latencies */
		  if (latency < 0)
		    continue;

		  /* Only process pairs for latencies that have
		   * not already been covered.
		   */
		  if (forbidden_symbol->data == NULL)
		    continue;

		  phead = (Pair_Node *) forbidden_symbol->data;

		  /* Count the number of usage pairs that generate
		   * this forbidden latency
		   */
		  count = 0;
		  for (pnode = phead; pnode != NULL; pnode = pnode->next)
		    {
		      count++;
		    }

#if DEBUG_OPTI
		  printf ("User1 %i user2 %i latency %i  pairs %i\n",
			  user1, user2, latency, count);
#endif

		  best_pnode = NULL;
		  best_covered = -1;

		  /* Find the usage pair for this forbidden latency
		   * that covers the most forbidden latencies (other
		   * than this one).  
		   * This algorithm used to give slight preference to
		   * pairs where one usage is already committed 
		   * I now correct this, so they are all equal -JCG 4/16/96
		   * (Made no difference for PA or SPARC! but seems the
		   *  right thing to do)
		   */
		  for (pnode = phead; pnode != NULL; pnode = pnode->next)
		    {
		      num_covered =
			count_num_covered (forbidden_matrix,
					   pnode->resource,
					   user1, pnode->time1);

		      /* Subtract from the count the "coverage" caused by
		       * user2 @ time2 being already commited.
		       */
		      if (is_usage_committed (forbidden_matrix,
					      pnode->resource,
					      user2, pnode->time1 + latency))
			{
#if DEBUG_OPTI
			  printf
			    ("Correcting cover for %s user1 %i user2 %i "
			     "time1 %i time2 %i\n",
			     pnode->resource->name, user1, user2,
			     pnode->time1, pnode->time1 + latency);
#endif
			  num_covered--;
			}
		      /* Don't count the same resource usage twice */
		      if ((user1 != user2) || (latency != 0))
			{
			  num_covered +=
			    count_num_covered (forbidden_matrix,
					       pnode->resource,
					       user2, pnode->time1 + latency);

			  /* Subtract from the count the "coverage" caused
			   * by user1 @ time1 being already commited.
			   */
			  if (is_usage_committed (forbidden_matrix,
						  pnode->resource,
						  user1, pnode->time1))
			    {
#if DEBUG_OPTI
			      printf
				("Correcting cover for %s user1 %i user2 %i "
			         "time1 %i time2 %i\n",
				 pnode->resource->name, user1, user2,
				 pnode->time1, pnode->time1 + latency);
#endif
			      num_covered--;
			    }

			}


		      if ((best_pnode == NULL) ||
			  (best_covered < num_covered) ||
			  (best_pnode->time1 > pnode->time1))
			{
			  best_pnode = pnode;
			  best_covered = num_covered;
			}
		    }


		  /* Using greedy covering algorithm.  Pick 
		   * the next pair to commit based on the following
		   * sort order:
		   * 1) The number of forbidden latencies covered
		   *    (pick the largest).
		   * 2) The number of pair that cover this latency
		   *    (pick the smallest).
		   * 3) The early time of the usage (pick the smallest,
		   *    bias towards small usage times, 0 is likely to
		   *    be a good usage time)
		   * 4) The late time of the usage (pick the largest!,
		   *    bias towards picking the long latency times first,
		   *    they most likely are going to be picked anyway,
		   *    so pick them now) (It could go either way, but
		   *    using the largest worked much better on PA's
		   *    divides.)
		   * 
		   * Otherwise pick the first pair encountered.
		   */
		  if ((min_resource == NULL) ||
		      ((min_covered < best_covered)) ||
		      ((min_covered == best_covered) &&
		       (min_count > count)) ||
		      ((min_covered == best_covered) &&
		       (min_count == count) &&
		       (min_time1 > best_pnode->time1)) ||
		      ((min_covered == best_covered) &&
		       (min_count == count) &&
		       (min_time1 == best_pnode->time1) &&
		       (min_time2 < (best_pnode->time1 + latency))))
		    {
		      min_resource = best_pnode->resource;
		      min_user1 = user1;
		      min_user2 = user2;
		      min_time1 = best_pnode->time1;
		      min_time2 = best_pnode->time1 + latency;
		      min_count = count;
		      min_covered = best_covered;
#if DEBUG_OPTI
		      printf
			("New min: resource %s user1 %i user2 %i time1 %i "
			 "time2 %i count %i covered %i\n",
			 min_resource->name, min_user1, min_user2, min_time1,
			 min_time2, min_count, min_covered);
#endif
		    }

		  num_left++;
		}
	    }
	}

      /* If min_resource is NULL, num_left better be 0 */
      if ((min_resource == NULL) && (num_left != 0))
	C_punt (NULL, "Minimize gen set: algorithm error!");

      if (min_resource != NULL)
	{
	  commit_usage (forbidden_matrix, min_resource, min_user1, min_time1);
	  commit_usage (forbidden_matrix, min_resource, min_user2, min_time2);

#if DEBUG_OPTI
	  printf
	    ("Num left %i: resource %s user1 %i user2 %i time1 %i time2 %i "
	     "count %i covered %i\n",
	     num_left, min_resource->name, min_user1, min_user2, min_time1,
	     min_time2, min_count, min_covered);
#endif
	}

    }
#if DEBUG_OPTI
  printf ("Final generating set after minimization:\n");
  print_generating_set (stdout, generating_set, unit_array, 1);
#endif
}


void
print_generating_set (FILE * out, INT_Symbol_Table * generating_set,
		      MD_Entry ** unit_array, int committed)
{
  INT_Symbol *time_symbol, *usage_symbol, *resource_symbol;
  INT_Symbol_Table *usage_table;
  Resource_Info *resource;
  int time;
  int user;

  for (resource_symbol = generating_set->head_symbol;
       resource_symbol != NULL;
       resource_symbol = resource_symbol->next_symbol)
    {
      /* Get the resource for this symbol */
      resource = (Resource_Info *) resource_symbol->data;
      fprintf (out, "%s:\n", resource->name);

      for (time_symbol = resource->time_table->head_symbol;
	   time_symbol != NULL; time_symbol = time_symbol->next_symbol)
	{
	  /* Get the time for this usage table */
	  time = time_symbol->value;

	  fprintf (out, "  time %i:", time);

	  /* Get the usage table for this time */
	  usage_table = (INT_Symbol_Table *) time_symbol->data;

	  /* Test all usages of this resource at this time */
	  for (usage_symbol = usage_table->head_symbol; usage_symbol != NULL;
	       usage_symbol = usage_symbol->next_symbol)
	    {
	      /* Get a user of this resource at this time */
	      user = usage_symbol->value;

	      if (committed == ((long) usage_symbol->data & (long) 0xffffffff))
		printf (" %i", user);
	      else
		printf (" (%i)", user);
	    }
	  printf ("\n");
	}
      printf ("\n");
    }
}

void
free_generating_set (INT_Symbol_Table * generating_set)
{
  INT_Symbol *resource_symbol, *next_resource_symbol;
  Resource_Info *resource;

  /* Delete all the resources in the generating set */
  for (resource_symbol = generating_set->head_symbol;
       resource_symbol != NULL; resource_symbol = next_resource_symbol)
    {
      /* Get the next resource symbol before deleting this one */
      next_resource_symbol = resource_symbol->next_symbol;

      /* Get the resource for this symbol */
      resource = (Resource_Info *) resource_symbol->data;

      /* Delete this resource */
      delete_resource (resource);

      /* Delete the symbol pointing to it */
      INT_delete_symbol (resource_symbol, NULL);
    }

  /* Delete the generating set itself */
  INT_delete_symbol_table (generating_set, NULL);
}

void
instantiate_generating_set (INT_Symbol_Table * generating_set,
			    MD_Field ** use_array, int num_units,
			    MD_Section * usage_section)
{
  INT_Symbol *time_symbol, *usage_symbol, *resource_symbol;
  INT_Symbol_Table *usage_table;
  MD_Entry *usage, *next_usage;
  Resource_Info *resource;
  MD_Field *time_field, *use_field;
  MD_Field_Decl *time_field_decl, *use_field_decl;
  int user, time, index1, index2;
  char name_buf[5000];

  /* Delete all the existing links in the unit->use fields */
  for (index1 = 0; index1 < num_units; index1++)
    {
      for (index2 = use_array[index1]->max_element_index; index2 >= 0;
	   index2--)
	{
	  MD_delete_element (use_array[index1], index2);
	}
    }

  /* Delete all the existing resource usage entries */
  for (usage = MD_first_entry (usage_section); usage != NULL;
       usage = next_usage)
    {
      /* Get the next resource usage before deleting this one */
      next_usage = MD_next_entry (usage);

      MD_delete_entry (usage);
    }

  /* Get the time and use field declarations */
  use_field_decl = CU_find_field_decl (usage_section, "use");
  time_field_decl = CU_find_field_decl (usage_section, "time");

  for (resource_symbol = generating_set->head_symbol;
       resource_symbol != NULL;
       resource_symbol = resource_symbol->next_symbol)
    {
      /* Get the resource for this symbol */
      resource = (Resource_Info *) resource_symbol->data;

      for (time_symbol = resource->time_table->head_symbol;
	   time_symbol != NULL; time_symbol = time_symbol->next_symbol)
	{
	  /* Get the time for this usage table */
	  time = time_symbol->value;

	  /* Create a new usage of this resource at this time */
	  sprintf (name_buf, "%s_t%i", resource->name, time);
	  usage = MD_new_entry (usage_section, name_buf);

	  /* Set the entries time and use field appropriately */
	  use_field = MD_new_field (usage, use_field_decl, 1);
	  MD_set_link (use_field, 0, resource->resource_entry);

	  time_field = MD_new_field (usage, time_field_decl, 1);
	  MD_set_int (time_field, 0, time);

	  /* Get the usage table for this time */
	  usage_table = (INT_Symbol_Table *) time_symbol->data;

	  /* Add the commited usages of this resource at this time */
	  for (usage_symbol = usage_table->head_symbol; usage_symbol != NULL;
	       usage_symbol = usage_symbol->next_symbol)
	    {

	      /* Add only committed usages */
	      if (((long) usage_symbol->data & (long) 0xffffffff) == 1)
		{
		  /* Get a user of this resource at this time */
		  user = usage_symbol->value;

		  /* Add the usage to this user's use field */
		  MD_set_link (use_array[user],
			       use_array[user]->max_element_index + 1, usage);
		}
	    }
	}
    }
}

void
reduce_resource_units (MD * md)
{
  MD_Section *unit_section, *usage_section;
  MD_Entry **unit_array, *unit_entry, *usage_entry;
  MD_Field **use_array, *use1_field, *use2_field, *test_field;
  INT_Symbol_Table ***forbidden_matrix, **forbidden_row, *forbidden_set;
  INT_Symbol_Table *generating_set;
  int num_units, array_size, index1, index2;
  char name_buf[100];


  /* Sanity check, this routine should be called only if each
   * resource usage has been homogenized so that only one resource
   * is used at only one time.
   */
  usage_section = CU_find_section (md, "Resource_Usage");
  for (usage_entry = MD_first_entry (usage_section); usage_entry != NULL;
       usage_entry = MD_next_entry (usage_entry))
    {
      /* Get the use and time field for each entry and make
       * sure they only have one element.
       */
      test_field = CU_find_field_by_name (usage_entry, "use");
      if (test_field->max_element_index != 0)
	{
	  C_punt (md,
		  "reduce_resource_units: '%s' has %i resources specified!",
		  usage_entry->name, test_field->max_element_index + 1);
	}

      test_field = CU_find_field_by_name (usage_entry, "time");
      if (test_field->max_element_index != 0)
	{
	  C_punt (md,
		  "reduce_resource_units: '%s' has %i times specified!",
		  usage_entry->name, test_field->max_element_index + 1);
	}

    }

  /* Get the Resource_Unit section */
  unit_section = CU_find_section (md, "Resource_Unit");

  /* Get the number of resource units */
  num_units = CU_count_section_entries (unit_section);


  /* Create an array of units and their use fields to operate on */
  array_size = num_units * sizeof (MD_Entry *);
  if ((unit_array = (MD_Entry **) malloc (array_size)) == NULL)
    C_punt (md, "reduce_resource_units: Out of memory");

  array_size = num_units * sizeof (MD_Field *);
  if ((use_array = (MD_Field **) malloc (array_size)) == NULL)
    C_punt (md, "reduce_resource_units: Out of memory");

  /* Load units and their uses into array */
  index1 = 0;
  for (unit_entry = MD_first_entry (unit_section); unit_entry != NULL;
       unit_entry = MD_next_entry (unit_entry))
    {
      unit_array[index1] = unit_entry;
      use_array[index1] = CU_find_field_by_name (unit_entry, "use");
      index1++;
    }

#if DEBUG_OPTI
  printf ("Trees before minimization:\n");
  print_table_trees (stdout, md);
  printf ("\n");

  printf ("Units:\n");
  for (index1 = 0; index1 < num_units; index1++)
    {
      printf (" %4i: %s\n", index1, unit_array[index1]->name);
    }
  printf ("\n");
#endif

  /* Create an matrix of INT_Symbol_Tables to hold the forbidden latencies */
  /* Create the row array that is the first level of the matrix */
  array_size = num_units * sizeof (INT_Symbol_Table **);
  if ((forbidden_matrix = (INT_Symbol_Table ***) malloc (array_size)) == NULL)
    C_punt (md, "reduce_resource_units: Out of memory");

  /* Create a matrix of forbidden sets using INT_Symbol_Tables */
  array_size = num_units * sizeof (INT_Symbol_Table *);
  for (index1 = 0; index1 < num_units; index1++)
    {
      if ((forbidden_row = (INT_Symbol_Table **) malloc (array_size)) == NULL)
	C_punt (md, "reduce_resource_units: Out of memory");
      forbidden_matrix[index1] = forbidden_row;

      /* Get the use field of the first unit */
      use1_field = use_array[index1];

      /* Fill this row with empty symbol tables */
      for (index2 = 0; index2 < num_units; index2++)
	{
	  /* Create name for each symbol table */
	  sprintf (name_buf, "F[%i][%i]", index1, index2);
	  forbidden_set = INT_new_symbol_table (name_buf, 0);

	  /* Get the use field of the second unit */
	  use2_field = use_array[index2];

	  calc_forbidden_latencies (forbidden_set, use1_field, use2_field);

	  forbidden_row[index2] = forbidden_set;
	}
    }

  /* Build the generating set using the current usages.  
   * It was found that add additional usages that are allowed by
   * the forbidden matrix was not very beneficial.  So that code
   * is currently ifdefed out. -JCG
   */
  generating_set = build_generating_set (md, forbidden_matrix, unit_array,
					 num_units);


  /* Use a covering algorithm to find a close to minimal set of usages
   * that generates all the forbidden latencies of the original.
   * Routines makes sure that the usage of slot resources at time 0 does
   * not change (may confuse both the scheduler (if in the end no slot
   * is used) and the user (if instructions end up in different slots)).
   */
  minimize_generating_set (generating_set, forbidden_matrix, num_units,
			   unit_array);


  /* Build up all the new unit usages, by create each resource usage
   * and placing in the correct units.
   */
  instantiate_generating_set (generating_set, use_array, num_units,
			      usage_section);

  /* Free up all the memory used */
  free_generating_set (generating_set);

  /* Free unit and use array */
  free (unit_array);
  free (use_array);

  /* Free forbidden matrix */
  for (index1 = 0; index1 < num_units; index1++)
    {
      forbidden_row = forbidden_matrix[index1];
      for (index2 = 0; index2 < num_units; index2++)
	{
	  forbidden_set = forbidden_row[index2];
	  INT_delete_symbol_table (forbidden_set, NULL);
	}
      free (forbidden_row);
    }
  free (forbidden_matrix);

}

int
count_table_uses (MD * md, MD_Entry * table_entry)
{
  MD_Section *alt_section, *op_section, *impact_op_section;
  MD_Entry *alt_entry, *op_entry, *impact_op_entry, *target_alt, *target_op;
  MD_Field_Decl *alt_field_decl, *resv_field_decl, *op_field_decl;
  MD_Field *alt_field, *resv_field, *op_field;
  int index1, index2, count;

  count = 0;

  /* Get sections from md */
  alt_section = CU_find_section (md, "Scheduling_Alternative");
  op_section = CU_find_section (md, "Operation");
  impact_op_section = CU_find_section (md, "IMPACT_Operation");

  /* Get field declarations for speed */
  resv_field_decl = CU_find_field_decl (alt_section, "resv");
  alt_field_decl = CU_find_field_decl (op_section, "alt");
  op_field_decl = CU_find_field_decl (impact_op_section, "op");

  /* For each alternative that points to this table */
  for (alt_entry = MD_first_entry (alt_section); alt_entry != NULL;
       alt_entry = MD_next_entry (alt_entry))
    {
      resv_field = MD_find_field (alt_entry, resv_field_decl);

      /* Only look at alts that link to table_entry */
      if (MD_get_link (resv_field, 0) != table_entry)
	continue;

#if DEBUG_OPTI
      printf ("%s links to %s\n", alt_entry->name, table_entry->name);
#endif

      /* For each link to this alt in an operation "alt" field */
      for (op_entry = MD_first_entry (op_section); op_entry != NULL;
	   op_entry = MD_next_entry (op_entry))
	{
	  alt_field = MD_find_field (op_entry, alt_field_decl);

	  for (index1 = 0; index1 <= alt_field->max_element_index; index1++)
	    {
	      target_alt = MD_get_link (alt_field, index1);

	      /* Only look at op links to this alt */
	      if (target_alt != alt_entry)
		continue;

#if DEBUG_OPTI
	      printf ("  %s links to %s\n", op_entry->name, alt_entry->name);
#endif

	      /* For each IMPACT_operation that links to this op, 
	       * increment count.
	       */
	      for (impact_op_entry = MD_first_entry (impact_op_section);
		   impact_op_entry != NULL;
		   impact_op_entry = MD_next_entry (impact_op_entry))
		{
		  op_field = MD_find_field (impact_op_entry, op_field_decl);

		  for (index2 = 0; index2 <= op_field->max_element_index;
		       index2++)
		    {
		      target_op = MD_get_link (op_field, index2);

		      if (target_op != op_entry)
			continue;
#if DEBUG_OPTI
		      printf ("    %s links to %s\n", impact_op_entry->name,
			      op_entry->name);
#endif

		      count++;
		    }
		}

	    }
	}

    }

  /* Since all dead code should be removed before this call, count better
   * be greater than 0! (unless opti_level < 1)
   */
  if ((count < 1) && (opti_level >= 1))
    {
      C_punt (NULL, "Count table uses: No uses of '%s->%s'!",
	      table_entry->section->name, table_entry->name);
    }

  return (count);
}

double
average_usage_checks (MD * md)
{
  MD_Section *table_section;
  MD_Entry *table_entry, *option_entry, *usage_entry;
  MD_Field *table_use, *option_one_of, *usage_use;
  double total_checks, total_weight, total_average;
  double option_checks, table_checks, table_weight;
  int index1, index2;

  total_checks = 0.0;
  total_weight = 0.0;

  /* Sum up the number of checks for each table.  Weighted by
   * the number of variations each table can have.
   */
  table_section = CU_find_section (md, "Reservation_Table");
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      /*Get the table use field */
      table_use = CU_find_field_by_name (table_entry, "use");

      /* Calculate the weight of this table (based on the number
       * of variations the options allow)
       */
      table_weight = (double) count_table_uses (md, table_entry);
      table_checks = 0.0;
      for (index1 = 0; index1 <= table_use->max_element_index; index1++)
	{
	  option_entry = MD_get_link (table_use, index1);

	  /* Update count if this is a table option */
	  if (strcmp (option_entry->section->name, "Table_Option") == 0)
	    {
	      option_one_of = CU_find_field_by_name (option_entry, "one_of");
	      table_weight *= (double) (option_one_of->max_element_index + 1);

	      option_checks = 0.0;
	      for (index2 = 0; index2 <= option_one_of->max_element_index;
		   index2++)
		{
		  usage_entry = MD_get_link (option_one_of, index2);
		  usage_use = CU_find_field_by_name (usage_entry, "use");
		  option_checks +=
		    (double) (usage_use->max_element_index + 1);
		}
	      table_checks += (option_checks /
			       (double) (option_one_of->max_element_index +
					 1));
	    }
	  else
	    {
	      usage_entry = option_entry;
	      usage_use = CU_find_field_by_name (usage_entry, "use");
	      table_checks += (double) (usage_use->max_element_index + 1);
	    }
	}

#if DEBUG_OPTI
      printf ("%s has weight %.2f checks %.2f\n", table_entry->name,
	      table_weight, table_checks);
#endif


      /* Update the total weight and checks */
      total_weight += table_weight;
      total_checks += (table_checks * table_weight);
    }

  if (total_weight > 0.1)
    total_average = total_checks / total_weight;
  else
    total_average = 0.0;

#if DEBUG_OPTI
  printf ("Total weight %.2f checks %.2f  average %.2f\n",
	  total_weight, total_checks, total_average);
#endif

  return (total_average);
}

/* Calculates the worst case number of resource checks to determine
 * if this alternative cannot be scheduled.
 * Basically, only the last option for each choice is available, and
 * the last option of the last choice fails.
 * 
 * Equivalent to the number of resource checks in the AND/OR tree.
 */
int
calc_best_case_succeed (MD_Entry * alt_entry)
{
  MD_Entry *table_entry, *usage_entry, *unit_entry;
  MD_Field *resv_field, *table_use_field, *one_of_field, *unit_use_field;
  int index1;
  int best_case_succeed;

  /* Get the reservation table for this alternative */
  resv_field = CU_find_field_by_name (alt_entry, "resv");
  table_entry = MD_get_link (resv_field, 0);

  /* Sum up the resource checks, in all the options of this table */
  table_use_field = CU_find_field_by_name (table_entry, "use");
  best_case_succeed = 0;

  for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
    {
      usage_entry = MD_get_link (table_use_field, index1);

      /* Best case, first option available for each choice */
      if (strcmp (usage_entry->section->name, "Resource_Unit") == 0)
	{
	  unit_entry = usage_entry;
	  unit_use_field = CU_find_field_by_name (unit_entry, "use");
	  best_case_succeed += unit_use_field->max_element_index + 1;
	}
      else
	{
	  one_of_field = CU_find_field_by_name (usage_entry, "one_of");
	  /* Add up all the resource usages for the first option */
	  unit_entry = MD_get_link (one_of_field, 0);
	  unit_use_field = CU_find_field_by_name (unit_entry, "use");
	  best_case_succeed += unit_use_field->max_element_index + 1;
	}
    }

#if DEBUG_OPTI
  printf ("      %s best case succeed %i\n", table_entry->name,
	  best_case_succeed);
#endif

  return (best_case_succeed);
}
/* Calculates the best case number of resource checks to determine
 * if this alternative cannot be scheduled.
 */
int
calc_best_case_fail (MD_Entry * alt_entry)
{
  MD_Entry *table_entry, *usage_entry;
  MD_Field *resv_field, *use_field, *one_of_field;
  int best_case_fail;

  /* Get the reservation table for this alternative */
  resv_field = CU_find_field_by_name (alt_entry, "resv");
  table_entry = MD_get_link (resv_field, 0);

  /* Get the first use for this table */
  use_field = CU_find_field_by_name (table_entry, "use");
  usage_entry = MD_get_link (use_field, 0);

  /* Best case usage, have to test each option, failing with
   * the first check.  So return the number of options for the
   * first use.  Return 1 for resource_units.
   */
  if (strcmp (usage_entry->section->name, "Resource_Unit") == 0)
    {
      best_case_fail = 1;
    }
  else
    {
      one_of_field = CU_find_field_by_name (usage_entry, "one_of");
      best_case_fail = one_of_field->max_element_index + 1;
    }

#if DEBUG_OPTI
  printf ("      %s best case fail %i\n", usage_entry->name, best_case_fail);
#endif

  return (best_case_fail);
}

/* Calculates the worst case number of resource checks to determine
 * if this alternative cannot be scheduled.
 * Basically, only the last option for each choice is available, and
 * the last option of the last choice fails.
 * 
 * Equivalent to the number of resource checks in the AND/OR tree.
 */
int
calc_worst_case_fail (MD_Entry * alt_entry)
{
  MD_Entry *table_entry, *usage_entry, *unit_entry;
  MD_Field *resv_field, *table_use_field, *one_of_field, *unit_use_field;
  int index1, index2;
  int worst_case_fail;

  /* Get the reservation table for this alternative */
  resv_field = CU_find_field_by_name (alt_entry, "resv");
  table_entry = MD_get_link (resv_field, 0);

  /* Sum up the resource checks, in all the options of this table */
  table_use_field = CU_find_field_by_name (table_entry, "use");
  worst_case_fail = 0;

  for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
    {
      usage_entry = MD_get_link (table_use_field, index1);

      /* Worst case, have to test every resource usage.  */
      if (strcmp (usage_entry->section->name, "Resource_Unit") == 0)
	{
	  unit_entry = usage_entry;
	  unit_use_field = CU_find_field_by_name (unit_entry, "use");
	  worst_case_fail += unit_use_field->max_element_index + 1;
	}
      else
	{
	  one_of_field = CU_find_field_by_name (usage_entry, "one_of");
	  /* Add up all the resource usages in every option */
	  for (index2 = 0; index2 <= one_of_field->max_element_index;
	       index2++)
	    {
	      unit_entry = MD_get_link (one_of_field, index2);
	      unit_use_field = CU_find_field_by_name (unit_entry, "use");
	      worst_case_fail += unit_use_field->max_element_index + 1;
	    }
	}
    }

#if DEBUG_OPTI
  printf ("      %s worst case fail %i\n", table_entry->name,
	  worst_case_fail);
#endif

  return (worst_case_fail);
}

int
calc_memory_size (MD * md)
{
  MD_Section *table_section, *option_section, *unit_section;
  MD_Entry *table_entry, *option_entry, *unit_entry;
  MD_Field *unit_use_field, *one_of_field, *table_use_field;
  int mem_size, usage_size, option_size, table_size, or_table_size;
  int usage_count, or_mem_size;

  /* Initialize memory_size */
  usage_size = 0;
  option_size = 0;
  table_size = 0;
  or_table_size = 0;

  /* Initialize usage count */
  usage_count = 0;

  /* Calculate the size due to resource units */
  unit_section = CU_find_section (md, "Resource_Unit");
  for (unit_entry = MD_first_entry (unit_section); unit_entry != NULL;
       unit_entry = MD_next_entry (unit_entry))
    {
      /* Get the use field */
      unit_use_field = CU_find_field_by_name (unit_entry, "use");

      /* Currently each unit takes 8 bytes for each usage,
       * and 8 bytes for the header.  (Need to look at when the header
       * is taken care of by table options).
       */
      usage_size += 8 + (8 * (unit_use_field->max_element_index + 1));

      usage_count += (unit_use_field->max_element_index + 1);
    }

  /* Calculate the size due to table options */
  option_section = CU_find_section (md, "Table_Option");
  for (option_entry = MD_first_entry (option_section); option_entry != NULL;
       option_entry = MD_next_entry (option_entry))
    {
      /* Get the one_of field */
      one_of_field = CU_find_field_by_name (option_entry, "one_of");

      /* Currently, each option takes 8 bytes for the header,
       * and 8 bytes per option (unit headers).
       */
      option_size += 8 + (8 * (one_of_field->max_element_index + 1));
    }

  /* Calculate the size due to reseveration tables */
  table_section = CU_find_section (md, "Reservation_Table");
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      /* Get the use field */
      table_use_field = CU_find_field_by_name (table_entry, "use");

      /* Currently, each table takes 16 bytes for the header,
       * and 8 bytes per choice (option headers).
       */
      table_size += 16 + (8 * (table_use_field->max_element_index + 1));

      /* For OR trees, only 1 choice, so options (unit headers) are 
       * instead put in the array.
       */
      option_entry = MD_get_link (table_use_field, 0);
      one_of_field = CU_find_field_by_name (option_entry, "one_of");
      or_table_size += 16 + (8 * (one_of_field->max_element_index + 1));
    }


  mem_size = usage_size + option_size + table_size;
  or_mem_size = usage_size + or_table_size;


  printf ("\n");
  printf ("%10i Operation types (table_section)\n",
	  MD_num_entries (table_section));
  printf ("%10i Tables (resource units)\n", MD_num_entries (unit_section));
  printf ("%10i checks (total static number)\n", usage_count);



  printf ("%10i bytes for resource usages\n", usage_size);
  printf ("%10i bytes for table options\n", option_size);
  printf ("%10i bytes for reservation tables\n", table_size);
  if (expand_tables)
    printf ("%10i bytes for OR-tree reservation tables\n", or_table_size);
  printf ("%10i total bytes for AND/OR-tree resource info\n", mem_size);
  if (expand_tables)
    printf ("%10i total bytes for OR-tree resource info\n", or_mem_size);

  /* Return the memory size */
  return (mem_size);
}

void
print_static_stats (FILE * out, MD * md)
{
  MD_Section *IMPACT_op_section;
  MD_Entry *IMPACT_op_entry, *op_entry, *alt_entry;
  MD_Field *op_field, *alt_field;
  int index1, index2;
  int total_op_count, total_best_case_fail, total_worst_case_fail;
  int total_best_case_succeed;
  int total_mem_size;
  int best_case_fail, worst_case_fail, best_case_succeed;

  /* Initialize stats */
  total_op_count = 0;
  total_best_case_succeed = 0;
  total_best_case_fail = 0;
  total_worst_case_fail = 0;

  /* Print header */
  fprintf (out, "> Static MDES stats:\n");

  /* Get the memory size for the reservation tables */
  total_mem_size = calc_memory_size (md);

  /* Calculate stats for each IMPACT_op */
  IMPACT_op_section = CU_find_section (md, "IMPACT_Operation");
  for (IMPACT_op_entry = MD_first_entry (IMPACT_op_section);
       IMPACT_op_entry != NULL;
       IMPACT_op_entry = MD_next_entry (IMPACT_op_entry))
    {

#if DEBUG_OPTI
      printf ("%s\n", IMPACT_op_entry->name);
#endif

      /* Get the op field for this IMPACT_op */
      op_field = CU_find_field_by_name (IMPACT_op_entry, "op");

      /* Initialize stats for op */
      best_case_succeed = 0;
      best_case_fail = 0;
      worst_case_fail = 0;

      /* Sum up the stats for each op for this IMPACT_op */
      for (index1 = 0; index1 <= op_field->max_element_index; index1++)
	{
	  /* Get this op entry */
	  op_entry = MD_get_link (op_field, index1);

#if DEBUG_OPTI
	  printf ("  %s\n", op_entry->name);
#endif

	  /* Get the alt field for this op */
	  alt_field = CU_find_field_by_name (op_entry, "alt");

	  /* Sum up the stats for each alt in this op */
	  for (index2 = 0; index2 <= alt_field->max_element_index; index2++)
	    {
	      /* Get this alt entry */
	      alt_entry = MD_get_link (alt_field, index2);

#if DEBUG_OPTI
	      printf ("    %s\n", alt_entry->name);
#endif

	      /* Get the best case succeed for only the first op */
	      if (index2 == 0)
		{
		  best_case_succeed += calc_best_case_succeed (alt_entry);
		}

	      best_case_fail += calc_best_case_fail (alt_entry);
	      worst_case_fail += calc_worst_case_fail (alt_entry);
	    }
	}

      /* Update stats */
      total_op_count++;
      total_best_case_succeed += best_case_succeed;
      total_best_case_fail += best_case_fail;
      total_worst_case_fail += worst_case_fail;
    }

  /* Print stats */
#if DEBUG_OPTI
  printf ("%10i operations in MDES.\n", total_op_count);
  printf ("%10i total best case succeed.\n", total_best_case_succeed);
  printf ("%10i total best case fail.\n", total_best_case_fail);
  printf ("%10i total worst case fail.\n", total_worst_case_fail);
#endif
  printf ("%10.2f average best case succeed/operation.\n",
	  (double) (((double) total_best_case_succeed) /
		    ((double) total_op_count)));
  printf ("%10.2f average best case fail/operation.\n",
	  (double) (((double) total_best_case_fail) /
		    ((double) total_op_count)));
  printf ("%10.2f average worst case fail/operation.\n",
	  (double) (((double) total_worst_case_fail) /
		    ((double) total_op_count)));
}
void
print_ru_tree (FILE * out, MD_Entry * ru_entry)
{
  MD_Field *use_field, *time_field;
  MD_Entry *resource_entry;
  int index1, index2, time;

  use_field = CU_find_field_by_name (ru_entry, "use");
  time_field = CU_find_field_by_name (ru_entry, "time");
  for (index1 = 0; index1 <= time_field->max_element_index; index1++)
    {
      time = MD_get_int (time_field, index1);
      fprintf (out, "        @%i:", time);
      for (index2 = 0; index2 <= use_field->max_element_index; index2++)
	{
	  resource_entry = MD_get_link (use_field, index2);

	  fprintf (out, " %s", resource_entry->name);
	}
      fprintf (out, "\n");
    }
}

void
print_unit_tree (FILE * out, MD_Entry * unit_entry, int option_id)
{
  MD_Field *use_field;
  MD_Entry *ru_entry;
  int index1;

  fprintf (out, "      Option %i (%s):\n", option_id, unit_entry->name);
  use_field = CU_find_field_by_name (unit_entry, "use");
  for (index1 = 0; index1 <= use_field->max_element_index; index1++)
    {
      ru_entry = MD_get_link (use_field, index1);
      print_ru_tree (out, ru_entry);
    }
}

void
print_option_tree (FILE * out, MD_Entry * option_entry)
{
  MD_Field *one_of_field;
  MD_Entry *unit_entry;
  int index1;

  one_of_field = CU_find_field_by_name (option_entry, "one_of");
  for (index1 = 0; index1 <= one_of_field->max_element_index; index1++)
    {
      unit_entry = MD_get_link (one_of_field, index1);
      print_unit_tree (out, unit_entry, index1);
    }
}

void
print_and_or_tree (FILE * out, MD_Entry * table)
{
  MD_Field *table_use_field;
  MD_Entry *option_entry;
  int index1;

  fprintf (out, "  %s:", table->name);
  table_use_field = CU_find_field_by_name (table, "use");
  for (index1 = 0; index1 <= table_use_field->max_element_index; index1++)
    {
      option_entry = MD_get_link (table_use_field, index1);
      fprintf (out, "\n    Choice %i (%s):\n", index1, option_entry->name);
      if (strcmp (option_entry->section->name, "Table_Option") == 0)
	print_option_tree (out, option_entry);
      else
	print_unit_tree (out, option_entry, 0);
    }
}

void
print_table_trees (FILE * out, MD * md)
{
  MD_Section *table_section;
  MD_Entry *table_entry;

  table_section = CU_find_section (md, "Reservation_Table");
  for (table_entry = MD_first_entry (table_section); table_entry != NULL;
       table_entry = MD_next_entry (table_entry))
    {
      print_and_or_tree (out, table_entry);
      fprintf (out, "\n");
    }
}

/* Returns the maximum number of elements in the section for this field.
 * Returns 0 if field is not specified for any entries or if they
 * are all empty.
 */
int
CU_calc_max_field_size (MD * md, char *section_name, char *field_name)
{
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  int field_size, max_size;

  max_size = 0;

  /* Get the section and field declaration */
  section = CU_find_section (md, section_name);
  field_decl = CU_find_field_decl (section, field_name);

  /* Find the max size of this field in this section */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      field = MD_find_field (entry, field_decl);

      /* Skip entries that don't specify the field */
      if (field == NULL)
	continue;

      /* Get the field size */
      field_size = field->max_element_index + 1;

      if (field_size > max_size)
	max_size = field_size;
    }

  return (max_size);
}

/* Returns the maximum integer value found in the specified field. */
int
CU_calc_int_field_max_value (MD * md, char *section_name, char *field_name)
{
  MD_Section *section;
  MD_Entry *entry;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  int index1, value, max_value, num_values;

  max_value = -100000;
  num_values = 0;

  /* Get the section and field declaration */
  section = CU_find_section (md, section_name);
  field_decl = CU_find_field_decl (section, field_name);

  /* Find the max size of this field in this section */
  for (entry = MD_first_entry (section); entry != NULL;
       entry = MD_next_entry (entry))
    {
      field = MD_find_field (entry, field_decl);

      /* Skip entries that don't specify the field */
      if (field == NULL)
	continue;

      /* Get each elements value and take it's max */
      for (index1 = 0; index1 <= field->max_element_index; index1++)
	{
	  value = MD_get_int (field, index1);

	  /* Update max stats */
	  if ((num_values == 0) || (max_value < value))
	    max_value = value;

	  /* Update value count */
	  num_values++;
	}
    }

  /* Punt if no values were specified */
  if (num_values == 0)
    C_punt (md, "No int values specified for field %s in section %s!",
	    field_name, section_name);

  /* Return the max value */
  return (max_value);
}

/* Set slot parameters "max_slot", "num_slots" for this machine description */
void
CU_set_slot_parameters (MD * md)
{
  MD_Section *resource_section;
  MD_Entry *resource_entry;
  MD_Field_Decl *slot_field_decl;
  MD_Field *slot_field;
  int slot, max_slot, num_slots;

  /* Get the resource section and its slot field declaration */
  resource_section = CU_find_section (md, "Resource");
  slot_field_decl = CU_find_field_decl (resource_section, "slot");

  /* Initialize stats */
  max_slot = -1;
  num_slots = 0;

  /* Scan all resources to calc parameters */
  for (resource_entry = MD_first_entry (resource_section);
       resource_entry != NULL;
       resource_entry = MD_next_entry (resource_entry))
    {
      /* Only look at resources with a slot field specified */
      slot_field = MD_find_field (resource_entry, slot_field_decl);
      if (slot_field == NULL)
	continue;

      /* Get the slot for this resource */
      slot = MD_get_int (slot_field, 0);

      /* Update stats */
      if (slot > max_slot)
	max_slot = slot;

      num_slots++;
    }

  /* Set the slot parameters */
  CU_set_int_parm (md, "num_slots", num_slots);
  CU_set_int_parm (md, "max_slot", max_slot);

}

void
dump_md (char *file_name, MD * md)
{
  FILE *out;

  printf ("\n Dumping md file to '%s'\n", file_name);
  print_time_stamp (stdout);
  fflush (stdout);

  if ((out = fopen (file_name, "w")) == NULL)
    I_punt ("dump_md: unable to open '%s'", file_name);

  MD_write_md (out, md);

  fclose (out);

  printf ("Finished dump\n");
  print_time_stamp (stdout);
  fflush (stdout);
}

void
customize_md (MD * md)
{
  MD_Section *unit_section;
  int max_offset;
  int total_change, initial_size, opti_size, speed_size;
  int pre_expansion, post_expansion, expansion_added;
  double pre_reduction_avg, post_reduction_avg;
  int num_dest_operands, num_src_operands, num_pred_operands;
  int max_field_type_id, max_proc_opc;


#if VERBOSE_OPTI
  printf ("> Starting customization\n");
  print_time_stamp (stdout);
#endif

  /* Create rename table */
  rename_table = STRING_new_symbol_table ("rename", 128);

  /* Get the initial size of the mdes */
  initial_size = CU_count_entries (md);

  /* Read all the header tables used for customization */
  read_header_tables (md);

  annotate_header_values (md, "Field_Type", "id", "MDES_OPERAND_", 0);

  annotate_header_values (md, "IMPACT_Operation_Flag", "id", "OP_FLAG_", 1);


  annotate_header_values (md, "IMPACT_Alt_Flag", "id", "ALT_FLAG_", 1);

  annotate_header_values (md, "IMPACT_Operation", "proc_opc", "", 1);

  /* 20020822 SZU
   * Want to encode the latency classes as contiguous hash instead of string.
   * Assign numbers to Operation for hash number.
   */
  annotate_hash (md, "Operation");

  /* 20020731 SZU
   * annotate_latency_class function added to retain latency class
   * information in IMPACT_Operation and Prod_Cons_Latency.
   * Required because of loss of information during compression of Operation
   * and Scheduling_Alternatives sections.
   */

  annotate_latency_class (md, "IMPACT_Operation", "op", "lat_class");

  /* 20020810 SZU
   * Prod_Cons_Latency section not in all mdes. Check first.
   */
  if (MD_find_section (md, "Prod_Cons_Latency") != NULL)
    {
      annotate_latency_class (md, "Prod_Cons_Latency", "prod",
			      "p_lat_class");

      annotate_latency_class (md, "Prod_Cons_Latency", "cons",
			      "c_lat_class");
    }

#if VERBOSE_OPTI
  else
    printf ("> No Prod_Cons_Latency section in mdes\n");
#endif

  if (header_errors > 0)
    {
      C_punt (md,
	      "%i errors occurred using headers to customize lmdes2.\nCannot "
	      "continue!",
	      header_errors);
    }

  convert_to_bit_representation (md, "IMPACT_Operation", "flags",
				 "IMPACT_Operation_Flag", "id", "bit_flags");

  convert_to_bit_representation (md, "Scheduling_Alternative", "flags",
				 "IMPACT_Alt_Flag", "id", "bit_flags");

  add_all_children_to_list (md, "Field_Type", "compatible_with");


  convert_to_int_list_all_targets (md, "Field_Type", "compatible_with",
				   "id", "compatible_ids");

  check_bidirectional_links (md, "Register", "overlaps");


  /* Scheduler uses unsigned short int to store slot numbers.
   * Make sure there are no negative slots or that it not too big.
   */
  check_int_bounds (md, "Resource", "slot", 0, ((2 << 15) - 1));

  /* Check to make sure exactly one use in a Reservation Table specifies
   * the use of a slot (may use multiple slots in one use, the
   * smallest slot will be used).
   */
  check_slot_specification (md);

  /* Check to make sure that the same resource is not unconditionally
   * used at the same time in a Resource_Usage or a Resource_Unit.
   */
  check_for_unconditional_overlap (md);

  /* Make sure option count will fall in scheduler's unsigned short.
   * See routine header for recommendations if you exceed this size.
   */
  check_table_option_count (md);


  /* Going to allocate array based on IMPACT_operations proc_opc.
   * Limit range to prevent some huge array from being created due to
   * some creative choices for proc_opc by code generator writers.  
   * (Same limit as in version1).
   */
  check_proc_opcs (md);


  /* Specify the maximum number of operands specified in the operation
   * formats.
   */
  num_dest_operands = CU_calc_max_field_size (md, "Operation_Format", "dest");
  CU_set_int_parm (md, "num_dest_operands", num_dest_operands);

  num_src_operands = CU_calc_max_field_size (md, "Operation_Format", "src");
  CU_set_int_parm (md, "num_src_operands", num_src_operands);

  num_pred_operands = CU_calc_max_field_size (md, "Operation_Format", "pred");
  CU_set_int_parm (md, "num_pred_operands", num_pred_operands);


  /* Set the slot parameters max_slot and num_slots */
  CU_set_slot_parameters (md);

  /* Set the max field type id parameter */
  max_field_type_id = CU_calc_int_field_max_value (md, "Field_Type", "id");
  CU_set_int_parm (md, "max_field_type_id", max_field_type_id);


  /* Set the max_proc_opc parameter */
  max_proc_opc = CU_calc_int_field_max_value (md, "IMPACT_Operation",
					      "proc_opc");
  CU_set_int_parm (md, "max_proc_opc", max_proc_opc);


  /* Don't do the fancy optimizations if the mdes is already bad */
  if (customization_errors > 0)
    {
      C_punt (md,
	      "%i errors occurred during customization.\nCannot continue!",
	      customization_errors);
    }

#if VERBOSE_OPTI
  printf
    ("\nRemoving unreferenced entries before homogenizing res tables:\n");
  fflush (stdout);
#endif

  total_change = do_classical_optimizations (md);

  /* Get the size of the mdes after classical optimizations */
  opti_size = CU_count_entries (md);

#if VERBOSE_STATS
  if ((initial_size != opti_size) && (opti_level > 0))
    {
      printf
	("> MDES size-reducing transformations eliminated %i (of %i) "
	 "entries.\n",
	 initial_size - opti_size, initial_size);
      fflush (stdout);
    }
#endif

#if VERBOSE_OPTI
  printf ("%i entries deleted/modified before homogenizing res tables.\n",
	  total_change);
  fflush (stdout);
#endif

  /* Homogenize the reservation tables into a form the scheduler and
   * resource_usage optimizer can handle.
   */
  homogenize_reservation_tables (md);

#if 0
  printf ("Returning now for debugging!\n");
  return;
#endif


  /* Optimize away any duplicate info created to homogenize res table */
  do_classical_optimizations (md);

#if 1
  /* Debug, do renaming earlier to make smaller */
  /* Make MDES smaller by removing the transformation debug info field
   * 'original_name' and by renaming all the entries we have been
   * manipulating to short names.  (It is confusing to have an alternative
   * for a SUB on an ADD, etc.  So better to just blow away the names
   * that don't really aid in debugging.
   */
  remove_original_name (md);
  if (opti_level > 0)
    {
      rename_section_entries (md, "Resource_Usage", "R");
      rename_section_entries (md, "Resource_Unit", "U");
      rename_section_entries (md, "Table_Option", "O");
      rename_section_entries (md, "Reservation_Table", "T");
      rename_section_entries (md, "Scheduling_Alternative", "A");
      rename_section_entries (md, "Operation", "op");
    }
#endif

  /* If -expand specified on command line, expand out the reservation
   * tables so that each variation is explicitly listed.  This converts
   * all tables with more than one choice (more than one table use)
   * into a table with only one choice (with all the options explicitly
   * in that choice).
   */
  if (expand_tables)
    {
      /* Get number of entries before expansion */
      pre_expansion = CU_count_entries (md);

      expand_out_reservation_tables (md);

      /* Get number of entries after expansion */
      post_expansion = CU_count_entries (md);

#if VERBOSE_OPTI
      printf ("Expansion added %i entries before opti\n",
	      post_expansion - pre_expansion);
      fflush (stdout);
#endif

      /* 
       * Applies transformations for opti_levels >= 3.
       *
       * Sort the Reseveration_Table->use() links and Resource_Unit->use()
       * links so that resources will be distributed the same in the various
       * forms of the same table, and so that the classical optimizations
       * can detect these duplicate tables consistently.
       * 
       * It also places checks near time 0 first.
       *
       * Don't limit reordering.  Gives classical opti a consistent ordering.
       */
      CU_order_use_fields (md, 1);

      /* optimize after doing expansion */
      do_classical_optimizations (md);

      /* Get number of entries after expansion */
      post_expansion = CU_count_entries (md);

      /* See how many entries expansion added */
      expansion_added = post_expansion - pre_expansion;

#if VERBOSE_STATS
      printf ("> Reservation Table expansion added %i entries.\n",
	      expansion_added);
      fflush (stdout);
#endif
#if 1
      /* Debug, do renaming earlier to make smaller */
      /* Make MDES smaller by removing the transformation debug info field
       * 'original_name' and by renaming all the entries we have been
       * manipulating to short names.  (It is confusing to have an alternative
       * for a SUB on an ADD, etc.  So better to just blow away the names
       * that don't really aid in debugging.
       */
      remove_original_name (md);
      if (opti_level > 0)
	{
	  rename_section_entries (md, "Resource_Usage", "R");
	  rename_section_entries (md, "Resource_Unit", "U");
	  rename_section_entries (md, "Table_Option", "O");
	  rename_section_entries (md, "Reservation_Table", "T");
	  rename_section_entries (md, "Scheduling_Alternative", "A");
	  rename_section_entries (md, "Operation", "op");
	}
#endif

    }
  /* Otherwise flag that expansion added no entries since it was not done! */
  else
    {

      expansion_added = 0;
    }


  if ((opti_level >= 3) && resource_minimization)
    {
      pre_reduction_avg = average_usage_checks (md);

      unit_section = CU_find_section (md, "Resource_Unit");

#if VERBOSE_OPTI
      printf ("%i resource units being optimized\n",
	      CU_count_section_entries (unit_section));
      fflush (stdout);
#endif

      reduce_resource_units (md);

      post_reduction_avg = average_usage_checks (md);

#if VERBOSE_STATS
      printf
	("> Minimization reduced the average resource usages/table from %.2f "
	 "to %.2f.\n",
	 pre_reduction_avg, post_reduction_avg);
      fflush (stdout);
#endif
    }
  else if (opti_level >= 3)
    {
#if VERBOSE_STATS
      printf ("> No resource minimization techniques performed.\n");
      fflush (stdout);
#endif
    }

  /* Assign an index and offset of each the resources in the resource map
   * used  by the scheduler manager.  Remove unreference resources 
   * before allocation.
   *
   * May want to do 'bit' allocation in the future to allow better
   * optimization of resource units when there are more than 32 resources.
   * Returns number of words required - 1.
   */
#if VERBOSE_OPTI
  printf ("\nRemoving unreferenced entries before map assignment:\n");
  fflush (stdout);
#endif

  total_change = do_classical_optimizations (md);

#if VERBOSE_OPTI
  printf ("%i entries deleted/modified before map assigment.\n",
	  total_change);
  fflush (stdout);
#endif


  max_offset = assign_resource_map_locations (md);

  /* Coalesce unconditional usages only for level 4 opti.
   * Allow -no_tree_opti to turn off.
   * Don't do if expanding tables since undoes expand tables.
   */
  if ((opti_level >= 5) && tree_opti && !expand_tables)
    {
      CU_coalesce_unconditional_usages (md);
    }

#if 0
  /* Debug, return now */
  printf ("Returning early for debugging purposes!\n");
  return;
#endif

  /* Do classical optimizations to remove the extra units created by
   * coalescing the unconditional usages
   */
  do_classical_optimizations (md);

#if 0
  /* Debug, dump file here */
  dump_md ("debug1.md", md);
#endif


  /* Optimize the resource_units so that multiple resources can be tested
   * by the same bit mask (in the same cycle).
   * Pass in the max offset to simplify calculations.
   */
  if (opti_level >= 2)
    {
      optimize_resource_units (md, max_offset);
    }

#if 0
  /* Debug, dump file here */
  dump_md ("debug2.md", md);
#endif

#if VERBOSE_OPTI
  printf ("\nEliminating redundant entries after resource unit opti:\n");
  fflush (stdout);
#endif

  total_change = do_classical_optimizations (md);

#if VERBOSE_OPTI
  printf ("%i entries deleted/modified after resource unit opti.\n",
	  total_change);
  fflush (stdout);
#endif

  /* Get the size of the mdes after speed/scheduling optimizations */
  speed_size = CU_count_entries (md);

#if VERBOSE_STATS
  /* Take into account the extries the table expansion added */
  if ((opti_size != (speed_size - expansion_added)) && (opti_level >= 2))
    {
      printf
	("> Scheduler speed-enhancing transformations added %i entries.\n",
	 (speed_size - expansion_added) - opti_size);

      double post_speed_avg;
      post_speed_avg = average_usage_checks (md);
      printf
	("> Bit-field opti reduced the average resource usage checks/table "
	 "to %.2f.\n",
	 post_speed_avg);
      fflush (stdout);
    }
#endif


  /* Mark the resource units that use one or more 'scheduling slot' resource 
   * in time 0, with the smallest slot used.  Must be done after all
   * speed transformations are done (don't add units after this!).
   */
  mark_resource_unit_slots (md);

  /* Put the reservation tables in a format that:
   * 1) Places all units into options that with just that unit as
   *    a choice.  Makes loading sm mdes easier.
   * 2) Places option or unit that defines the ops slot into the
   *    new table field slot_specifier()
   * 3) For opti_level >= 3
   *    Sorts the options/units by in use () the number of options,
   *    so that the "most restrictive" options are checked first.
   */
  format_reservation_tables (md);


  /* Print static stats about this machine description 
   * (Mainly for JCG's research)
   */
  if (static_stats)
    {
      print_static_stats (stdout, md);
    }

#if 0
  printf ("Suppressing renaming for debugging!\n");
#endif

  /* Make MDES smaller by removing the transformation debug info field
   * 'original_name' and by renaming all the entries we have been
   * manipulating to short names.  (It is confusing to have an alternative
   * for a SUB on an ADD, etc.  So better to just blow away the names
   * that don't really aid in debugging.
   */
  remove_original_name (md);
  if (opti_level > 0)
    {
      rename_section_entries (md, "Resource_Usage", "R");
      rename_section_entries (md, "Resource_Unit", "U");
      rename_section_entries (md, "Table_Option", "O");
      rename_section_entries (md, "Reservation_Table", "T");
      rename_section_entries (md, "Scheduling_Alternative", "A");
      rename_section_entries (md, "Operation", "op");
    }

  if (print_trees)
    {
      printf ("> AND/OR trees in optimized mdes:\n");
      print_table_trees (stdout, md);
    }

  if (customization_errors > 0)
    {
      C_punt (md,
	      "%i errors occurred during customization.\nCannot continue!",
	      customization_errors);

    }

  /* Set the customization version info, etc. */
  CU_set_string_parm (md, "customizer_version", "1.01");
  CU_set_string_parm (md, "compatible_with_version", "1.00");

  /* Set default values for mdes2 parms if they don't exist */
  if (!CU_parm_exists (md, "check_resources_for_only_one_alt"))
    CU_set_string_parm (md, "check_resources_for_only_one_alt", "no");

  /* Free rename table if allocated */
  if (rename_table != NULL)
    STRING_delete_symbol_table (rename_table, NULL);

#if VERBOSE_OPTI
  printf ("> Customization finished\n");
  print_time_stamp (stdout);
#endif

}
