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
 *      File:   md_preprocessor.c
 * 
 *      Description: The preprocessor for the IMPACT Meta-Description Language
 * 
 *      Creation Date:  October 1994
 * 
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *          John C. Gyllenhaal August 1995
 *          Enhanced expression evaluation routines
 *  
 *          John C. Gyllenhaal November 1996
 *          Modified $def semantics so that braces are not allowed in
 *          the '$def def_name def_value' form.  This allows a more helpful
 *          error message to be given for the subtly buggy text below:
 *             $if (1 == 1) {$def foo 1}
 *             $else {$def foo 2}
 *          The problem with this code is that foo is defined to '1}', so
 *          the $else inadvertently appears in the body of the $if.
 *
 *          John C. Gyllenhaal January 1997
 *          1) Added support to allow recursive text replacement.  
 *             For example, ${array_${index}} is now supported.
 * 
 *          2) Added new $for form which allows multiple value lists to be
 *             be specified.  The lists must be the same length and they
 *             are stepped through in parallel.
 *             For example: "$for ((I in 1 2)(J in 3 4)) {${I} ${J} }"
 *             Yields: "1 3 2 4 "
 *                          
 *          3) Added support for floating-point expression evaluation.
 *             Form: $.={floating-point expression}
 * 
 *          4) Now, by default, implicit text replacement does not occur.
 *	       For example: "$def I {10}  I = ${I}"
 *             Now yields: "  I = 10" instead of "  10 = 10"
 * 
 *          5) Enhanced $def and $for to allow user to enable implicit text
 *             replacement by placing a '!' in front of the def_name(s).
 *             For example: "$def !I {10}  I = ${I}"
 *             Now yields: "  10 = 10"
 * 	
 *          Many thanks to Shail Aditya and Bruce Childers for their 
 *          suggestions which led to the above enhancements!
 *  
 *  
\*****************************************************************************/


/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "md_preprocessor.h"

/* Write to out in many places */
FILE *out;


Mfile *current_file;

/* Symbol tables */
Psymbol_Table *Pdef_table = NULL;
Psymbol_Table *Mfile_table = NULL;

/* Allow disabling of text replacement during processing of text replacement
 * directives.  
 */
int allow_text_replacement = 1;

/* Setting to a non-zero value disables implicit text replacement.
 * This may be done during the processing of a text replacement directive
 * or by a user directive (soon to be implemented).
 *
 * This variable will be incremented to disable implict text replacement
 * and decremented to possibly turn it back on (allows multiple nested
 * sets and resets (only the last one should really take effect)
 * to be properly modeled).
 */
int disable_implicit_text_replacement = 0;

/* A temp buffer for the preprocessor's use */
Mbuf *temp_mbuf = NULL;
Mbuf *pptr_mbuf = NULL;

/* Placemarks for preprocessor's use */
Pptr *temp_placemark = NULL;
Mptr *expand_placemark = NULL;

/* Alloc pools */
L_Alloc_Pool *Pptr_pool = NULL;
L_Alloc_Pool *Pdef_pool = NULL;
L_Alloc_Pool *String_Node_pool = NULL;
L_Alloc_Pool *Value_List_pool = NULL;

char *program_name = NULL;

/* Command line parameters */
char *input_file_name = NULL;
char *output_file_name = NULL;
int using_stdin = 0;
int print_line_directives = 1;
int print_alloc_usage = 0;


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
	   "  -o output_file    outputs to output_file instead of to standard "
	   "output.\n");
  fprintf (stderr,
	   "  -stdin            reads input from standard input instead of "
	   "input_file.\n");
  fprintf (stderr,
	   "  -no_directives    suppresses output of line directives.\n");
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "  -Ddef_name=value  overrides $def directives in input "
	   "(until hit $undef.).\n");
  fprintf (stderr, "\n");
  fprintf (stderr, "debug options:\n");
  fprintf (stderr,
	   "  -bypass_alloc     use malloc instead of alloc routines.\n");
  fprintf (stderr, "  -print_usage      prints alloc routine stats.\n");
  fprintf (stderr, "\n");
  exit (1);
}

void
read_command_line_parameters (int argc, char **arg_ptr)
{
  char *name, *value;

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
	  /* Get -Ddef_name=value directives */
	  else if ((*arg_ptr)[1] == 'D')
	    {
	      /* Malloc copy and parse into name and value */
	      name = strdup (&(*arg_ptr)[2]);

	      /* Make sure first character is a valid */
	      if ((name[0] != '_') && !isalpha (name[0]))
		{
		  print_usage ("invalid def_name in '%s'.\n", *arg_ptr);
		}

	      /* Find equals */
	      for (value = name; *value != 0; value++)
		{
		  /* Stop when hit equals */
		  if (*value == '=')
		    {
		      break;
		    }

		  /* Make sure name before it is a valid identifier */
		  if ((*value != '_') && !isalnum (*value))
		    {
		      print_usage ("invalid def_name in '%s'.\n", *arg_ptr);
		    }
		}
	      /* If equals doesn't exist, set value to "" */
	      if (*value == 0)
		{
		  value = "";
		}
	      else
		{
		  /* Terminate name at equals */
		  *value = 0;

		  /* Set value string to rest of argument */
		  value++;
		}

	      /* Command line definitions are at level 2 and 
	       * do not allow implicit text replacement.
	       */
	      add_Pdef (name, value, 0, 2);

	      /* Free malloced string */
	      free (name);
	    }

	  /* Get -stdin flag */
	  else if (strcmp (*arg_ptr, "-stdin") == 0)
	    {
	      using_stdin = 1;
	    }

	  /* Get -no_directives flag */
	  else if (strcmp (*arg_ptr, "-no_directives") == 0)
	    {
	      print_line_directives = 0;
	    }

	  /* Get -bypass_alloc flag */
	  else if (strcmp (*arg_ptr, "-bypass_alloc") == 0)
	    {
	      bypass_alloc_routines = 1;
	    }

	  /* Get -print_usage flag */
	  else if (strcmp (*arg_ptr, "-print_usage") == 0)
	    {
	      print_alloc_usage = 1;
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


/*
 * Overrides lcode library's version.
 */
void
L_punt (char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");

  exit (-1);
}


/*
 * Parse error handler.
 * First argument is a pptr that points to where the error occured.
 * The second argument is a printf format string, and the rest
 * are the arguments for the format string.
 */
void
Perror (Pptr * pptr, char *fmt, ...)
{
  va_list args;
  Mptr *mptr;
  int i;
  int text_replaced;
  int error_pos;


  /* Get mptr for ease of use */
  mptr = pptr->mptr;

  /* Backup one character if file (if possible) to point at
   * character or identifier just preprocessed.  This is vital
   * for errors that occur at end of a line, since mptr and
   * pptr point to different lines.
   *
   * Test to make sure have a least one character in file (Mbackupc will
   * punt if this is not the case).  (Do Mbackupc if at EOF or not
   * at the first character in the file.)
   */
  if ((mptr->mline == NULL) || (mptr->pos != 0) ||
      (mptr->mline->line_no != 0))
    Mbackupc (mptr);

  /* 
   * Print out text line of text where error occured and put arrow to
   * location.  (pos starts at 0, so add 1 to get to column, but
   * subtract one since Pgetc reads one or more characters ahead)
   */
  fprintf (stderr, "\n");
  /* If not at EOF */
  if (mptr->mline != NULL)
    {
      fprintf (stderr,
	       "Error during preprocessing (line %i char %i of %s):\n",
	       mptr->mline->line_no, mptr->pos + 1, mptr->mfile->name);
    }

  /* otherwise, at EOF */
  else
    {
      fprintf (stderr,
	       "Error during preprocessing (%s at EOF):\n",
	       mptr->mfile->name);

    }

  /*Print error message */
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  fprintf (stderr, "\n");


  /* If at EOF, do not print out file text */
  if (mptr->mline != NULL)
    {
      /* Detect text replacement, assume none */
      text_replaced = 0;
      for (i = 0; i < pptr->expanded->cur_len; i++)
	{
	  if (mptr->mline->buf[i] != pptr->expanded->buf[i])
	    {
	      text_replaced = 1;
	      break;
	    }
	}

    }

  /* If at EOF, print out replacement text if exists */
  else
    {
      if (pptr->expanded_pos > 0)
	text_replaced = 1;
      else
	text_replaced = 0;
    }

  /* If text has been replaced, show point (after scanning for text
   * replacement and show line after text replacement 
   */
  if (text_replaced)
    {
      /* Print out file text where error occurred, if not at EOF */
      fprintf (stderr, "File text where error occurred:\n");
      if (mptr->mline != NULL)
	{
	  print_buf_with_arrow (stderr, mptr->mline->buf, mptr->pos);
	}
      else
	{
	  fprintf (stderr, "At EOF.\n\n");
	}


      fprintf (stderr, "After incremental text replacement:\n");
      print_buf_with_arrow (stderr, pptr->expanded->buf, pptr->expanded_pos);
    }

  /* 
   * Otherwise, use expanded_pos instead of mptr->mos to give true
   * location of error (instead of where we scanned for text replacement).
   * Should have expanded_pos = (mptr->pos -1);
   */
  else
    {
      /* Print out file text where error occurred */
      fprintf (stderr, "File text where error occurred:\n");

      /* Just to be safe, only use pptr->expanded_pos if it is where
       * we think it should be.
       */
      if (pptr->expanded_pos == (mptr->pos - 1))
	error_pos = pptr->expanded_pos;
      else
	error_pos = mptr->pos;
      print_buf_with_arrow (stderr, mptr->mline->buf, error_pos);
    }

  fprintf (stderr, "Fatal error, cannot continue.\n");
  fprintf (stderr, "\n");

  exit (-1);
}


void
Pputc (Pptr * pptr, int ch)
{
  static char *file_name = NULL;
  static int file_line = -1, file_pos = 0;
  char *input_name;
  int input_line;
  int name_changed, line_changed;


  /* Update line no, file if necessary at beginning of every line */
  if (file_pos == 0)
    {
      /* Assume nothing changed */
      line_changed = 0;
      name_changed = 0;

      /* Get pointer to input file name */
      input_name = pptr->mptr->mfile->name;

      /* Get input line number.
       * If at EOF, just assume on predicted line.
       * If ch is a newline, then mptr is pointing to the next
       * line, so adjust input line number back by 1.
       */
      if (pptr->mptr->mline == NULL)
	{
	  input_line = file_line;
	}
      else
	{
	  input_line = pptr->mptr->mline->line_no;
	  if (ch == '\n')
	    input_line--;
	}

      if ((file_name == NULL) || (strcmp (file_name, input_name) != 0))
	{
	  if (file_name != NULL)
	    free (file_name);
	  file_name = strdup (input_name);

	  /* Mark that file name changed */
	  name_changed = 1;
	}

      if ((pptr->mptr->mline != NULL) && (file_line != input_line))
	{
	  file_line = input_line;

	  /* Mark that line changed */
	  line_changed = 1;
	}

      /* Print out line control specifiers if desired */
      if (print_line_directives)
	{
	  if (name_changed && line_changed)
	    fprintf (out, "$line %i \"%s\"\n", file_line, file_name);

	  else if (name_changed)
	    fprintf (out, "$line \"%s\"\n", file_name);

	  else if (line_changed)
	    fprintf (out, "$line %i\n", file_line);
	}
    }

  /* Output the character to the file */
  putc (ch, out);

  /* Update line_no and pos */
  if (ch == '\n')
    {
      file_line++;
      file_pos = 0;
    }
  else
    {
      file_pos++;
    }
}

int
main (int argc, char **argv, char **envp)
{
  FILE *in;
  Pptr *pptr;
  char ch;
  int i;
  char *name, *value;

  /* Initialize alloc pools */
  Pptr_pool = L_create_alloc_pool ("Pptr", sizeof (Pptr), 16);
  Pdef_pool = L_create_alloc_pool ("Pdef", sizeof (Pdef), 16);
  String_Node_pool = L_create_alloc_pool ("String_Node",
					  sizeof (String_Node), 16);
  Value_List_pool = L_create_alloc_pool ("Value_List",
					 sizeof (Value_List), 16);

  /* Create the temporary buffer for the preprocessor's use */
  temp_mbuf = create_Mbuf ();
  pptr_mbuf = create_Mbuf ();


  /* Create symbol tables */
  Pdef_table = create_Psymbol_Table ("def");
  Mfile_table = create_Psymbol_Table ("open file");


  /* Read command line arguments and input file name */
  read_command_line_parameters (argc, argv);

  /* Parse environment variables and place them in the Pdef table */
  for (i = 0; envp[i] != NULL; i++)
    {
      /* Malloc own copy so can mess with contents */
      name = strdup (envp[i]);

      /* Find end of name/start of value */
      for (value = name; *value != 0; value++)
	{
	  /* If hit equals, terminate name and move value to next
	   * position.
	   */
	  if (*value == '=')
	    {
	      /* Terminate name */
	      *value = 0;

	      /* The rest of the string is the value */
	      value++;
	      break;
	    }
	}

      /* Define name at value with level 0 and require explicit replacement
       * directives in order to use value.
       */
      add_Pdef (name, value, 0, 0);

      /* Free name (and implicitly value) */
      free (name);
    }

  /* Open input file */
  if (using_stdin)
    {
      input_file_name = strdup ("(stdin)");
      in = stdin;
    }
  else
    {
      if ((in = fopen (input_file_name, "r")) == NULL)
	L_punt ("Unable to open input file '%s'.", input_file_name);
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
	L_punt ("Unable to open output file '%s'.", output_file_name);
    }


  /* Load input file into memory, and add file to symbol table */
  current_file = create_Mfile (in, input_file_name, "preprocessing");
  add_Psymbol (Mfile_table, current_file->name, current_file);

  /* Get pointer to start of file */
  pptr = create_Pptr (current_file);

  /* Create placemarks for preprocessor's use */
  temp_placemark = create_Pptr (current_file);
  expand_placemark = create_Mptr (current_file);

  while ((ch = Ppeekc (pptr)) != EOF)
    {
      /* Handle unmatched end curly brackets.  Process body ends when
       * encounters this to allow loop constructs to use process_body.
       */
      if (ch == '}')
	{
	  ch = Pgetc (pptr);
	  Pputc (pptr, ch);
	}
      else
	{
	  pptr = process_body (pptr);
	}
    }

  /* Free up placemarks */
  free_Pptr (temp_placemark);
  free_Mptr (expand_placemark);

  /* Free up Pptr */
  free_Pptr (pptr);

  /* Free symbol tables */
  free_Psymbol_Table (Pdef_table, free_Pdef);
  free_Psymbol_Table (Mfile_table, (void (*)(void *)) free_Mfile);

  /* Free up temp_mbuf */
  free_Mbuf (temp_mbuf);
  free_Mbuf (pptr_mbuf);

  /* Print  Lalloc usage if flagged on command line */
  if (print_alloc_usage)
    {
      if (Mfile_pool != NULL)
	L_print_alloc_info (stdout, Mfile_pool, 1);
      if (Mline_pool != NULL)
	L_print_alloc_info (stdout, Mline_pool, 1);
      if (Mbuf_pool != NULL)
	L_print_alloc_info (stdout, Mbuf_pool, 1);
      if (Mptr_pool != NULL)
	L_print_alloc_info (stdout, Mptr_pool, 1);
      if (Psymbol_Table_pool != NULL)
	L_print_alloc_info (stdout, Psymbol_Table_pool, 1);
      if (Psymbol_pool != NULL)
	L_print_alloc_info (stdout, Psymbol_pool, 1);
      L_print_alloc_info (stdout, Pptr_pool, 1);
      L_print_alloc_info (stdout, Pdef_pool, 1);
      L_print_alloc_info (stdout, String_Node_pool, 1);
      L_print_alloc_info (stdout, Value_List_pool, 1);
    }

  /* Free alloc pool */
  if (Mfile_pool != NULL)
    L_free_alloc_pool (Mfile_pool);
  if (Mline_pool != NULL)
    L_free_alloc_pool (Mline_pool);
  if (Mbuf_pool != NULL)
    L_free_alloc_pool (Mbuf_pool);
  if (Mptr_pool != NULL)
    L_free_alloc_pool (Mptr_pool);
  if (Psymbol_Table_pool != NULL)
    L_free_alloc_pool (Psymbol_Table_pool);
  if (Psymbol_pool != NULL)
    L_free_alloc_pool (Psymbol_pool);
  L_free_alloc_pool (Pptr_pool);
  L_free_alloc_pool (Pdef_pool);
  L_free_alloc_pool (String_Node_pool);
  L_free_alloc_pool (Value_List_pool);

  /* close input/output file.  May be stdin/stdout */
  fclose (in);
  fclose (out);

  return (0);
}


/* Process body and put into mbuf */
Pptr *
process_body (Pptr * pptr)
{
  int ch;
  int depth;

  /* Start at no { depth */
  depth = 0;

  /* Process body until hit } that takes depth <= 0 */
  while ((ch = Ppeekc (pptr)) != EOF)
    {
      /* Handle recursively if preprocessor directive */
      if (ch == '$')
	{
	  pptr = process_directive (pptr);
	}

      else if (ch == '{')
	{
	  /* Get character, put in file, increase depth */
	  ch = Pgetc (pptr);
	  Pputc (pptr, ch);

	  depth++;

	}

      else if (ch == '}')
	{
	  /* If don't have matching curly bracket, stop before put in
	   * file.
	   */
	  if (depth <= 0)
	    break;

	  /* Get character, put in file, decrease depth */
	  ch = Pgetc (pptr);
	  Pputc (pptr, ch);

	  depth--;
	}


      /* If not a preprocessing directive */
      else
	{
	  /* Get character and put in file */
	  ch = Pgetc (pptr);
	  Pputc (pptr, ch);

	  /* Handle backslashes, get the next character if there */
	  if (ch == '\\')
	    {
	      if ((ch = Pgetc (pptr)) != EOF)
		Pputc (pptr, ch);
	    }

	  /* Handle signel quoted strings.  Dump string to file
	   * until end-quote or end of line.
	   */
	  else if (ch == '\'')
	    {
	      while ((ch = Pgetc (pptr)) != EOF)
		{
		  Pputc (pptr, ch);

		  /* Stop at end-quote or newline */
		  if ((ch == '\'') || (ch == '\n'))
		    break;

		  /* Handle backslashes, get next character if there 
		   * unless newline.
		   */
		  if (ch == '\\')
		    {
		      if ((ch = Ppeekc (pptr)) != EOF)
			{
			  if (ch != '\n')
			    {
			      ch = Pgetc (pptr);
			      Pputc (pptr, ch);
			    }

			}
		    }
		}
	    }
	}
    }

  return (pptr);
}

/* Skip the body's contents (don't print out or process any directives, etc) */
Pptr *
skip_body (Pptr * pptr)
{
  int ch;
  int depth;

  /* Disable all text processing until hit end of body */
  allow_text_replacement = 0;

  /* Start at no { depth */
  depth = 0;

  /* Skip body until hit } that takes depth <= 0 */
  while ((ch = Ppeekc (pptr)) != EOF)
    {
      /* Handle curley braces */
      if (ch == '{')
	{
	  /* Get character, increase depth */
	  ch = Pgetc (pptr);

	  depth++;

	}

      else if (ch == '}')
	{
	  /* If don't have matching curly bracket, stop before getting char.
	   */
	  if (depth <= 0)
	    break;

	  /* Get character,  and decrease depth */
	  ch = Pgetc (pptr);

	  depth--;
	}


      /* If not a preprocessing directive */
      else
	{
	  /* Get character and throw it away */
	  ch = Pgetc (pptr);

	  /* Handle backslashes, get the next character if there */
	  if (ch == '\\')
	    {
	      ch = Pgetc (pptr);
	    }

	  /* Handle signel quoted strings.  Skip string 
	   * until end-quote or end of line.
	   */
	  else if (ch == '\'')
	    {
	      while ((ch = Pgetc (pptr)) != EOF)
		{
		  /* Stop at end-quote or newline */
		  if ((ch == '\'') || (ch == '\n'))
		    break;

		  /* Handle backslashes, get next character if there 
		   * unless newline.
		   */
		  if (ch == '\\')
		    {
		      if ((ch = Ppeekc (pptr)) != EOF)
			{
			  if (ch != '\n')
			    {
			      ch = Pgetc (pptr);
			    }
			}
		    }
		}
	    }
	}
    }

  /* Turn back on text processing */
  allow_text_replacement = 1;

  return (pptr);
}

Pptr *
process_def_directive (Pptr * pptr)
{
  char *name, *val, ch, *ptr;
  int allow_implicit_replacement;



  /* Get name being defined, disable implicit text replacement so that the
   * name being defined will not be replaced by a previous directive 
   * unless explicitly told to.
   * This allows names to be easily to be redefined (otherwise a
   * $\ is needed before the name being defined the second time)
   */
  disable_implicit_text_replacement++;

  Pskip_whitespace_no_nl (pptr);

  /* Allow implicit replacement if '!' placed before name */
  if (Ppeekc (pptr) == '!')
    {
      /* Get the '!' */
      Pgetc (pptr);

      /* Mark as allowing implicit text replacement */
      allow_implicit_replacement = 1;
    }
  /* Otherwise, by default, don't allow implicit text replacement */
  else
    {
      allow_implicit_replacement = 0;
    }
  name = Pget_identifier (pptr);

  /* Restore original value of disable_implicit_text_replacement */
  disable_implicit_text_replacement--;

  Pskip_whitespace_no_nl (pptr);

  ch = Ppeekc (pptr);

  /* If at EOF or newline, assume just defining name, give no value */
  if ((ch == EOF) || (ch == '\n'))
    {
      val = strdup ("");
    }

  /* If start with '(', read in bounded string */
  else if (ch == '{')
    {
      val = Pget_bounded_string (pptr);
    }

  /* Otherwise, read to end of line and strip off trailing whitespace 
   * 11/26/96 No longer allow '{' and '}' symbols in unbounded 
   * $def statements.  Their accidential inclusion causes very
   * confusing error messages
   */
  else
    {
      /* Get placemark of error messages */
      move_Pptr (temp_placemark, pptr);
      val = Pget_stripped_line (pptr);

      /* Scan string for illegal '{' or '}' characters */
      for (ptr = val; *ptr != '\0'; ptr++)
	{
	  /* Handle backslash characters properly */
	  if (*ptr == '\\')
	    {
	      /* Goto next character */
	      ptr++;

	      /* Make sure there is a next character */
	      if (*ptr == '\0')
		{
		  Perror (temp_placemark,
			  "Character missing after backslash in "
			  "$def of '%s' to '%s'.\n\n", name, val);
		}
	    }
	  else if ((*ptr == '{') || (*ptr == '}'))
	    {
	      Perror (temp_placemark,
		      "Unexpected '%c' in $def of '%s' to '%s'.\n"
		      "  Braces are not allowed in this form of $def.\n\n"
		      "Possible solutions:\n"
		      "  1) Place '%c' on next line to exclude from $def\n"
		      "  2) Place a backslash before '%c'\n"
		      "  3) Use '$def def_name {bounded_def_value}' form.\n",
		      *ptr, name, val, *ptr, *ptr);
	    }

	  /* Error to have '$' without a backslash */
	  else if (*ptr == '$')
	    {
	      Perror (temp_placemark,
		      "Expecting a backslash before '$' in "
		      "$def of '%s' to '%s'.\n\n"
		      "Note: $def, $undef, $for, $if, and $include "
		      "directives may not\n"
		      "      be embedded in replacement value strings.",
		      name, val);
	    }
	}
    }

  /* Add definition to symbol table */
  add_Pdef (name, val, allow_implicit_replacement, 1);

  /* Free name and value buffers */
  free (name);
  free (val);


  return (pptr);
}

Pptr *
process_undef_directive (Pptr * pptr)
{
  char *name;


  /* Get name being undefined, disable implicit text replacement so that the
   * name being undefined will not be text replaced unless explicitly
   * told to do so.
   * This allows names to be easily to be undefined (otherwise a
   * $\ is needed before the name being undefined).
   */
  disable_implicit_text_replacement++;

  Pskip_whitespace_no_nl (pptr);
  name = Pget_identifier (pptr);

  /* Restore original value of disable_implicit_text_replacement */
  disable_implicit_text_replacement--;

  /* Remove definition from symbol table */
  delete_Pdef (name);

  /* Free name and value buffers */
  free (name);

  return (pptr);
}

Value_List *
get_for_value_list (Pptr * pptr, Pptr * placemark)
{
  Value_List *value_list;
  String_Node *value_node;
  char *val;
  int ch;

  /* Allocate value list */
  value_list = (Value_List *) L_alloc (Value_List_pool);

  /* Skip any whitespace before opening '(' */
  Pskip_whitespace (pptr);

  /* Move placemark to beginning of value list definition */
  move_Pptr (placemark, pptr);

  /* Get ( */
  if ((ch = Ppeekc (pptr)) != '(')
    Perror (pptr, "Error parsing $for directive, '(' expected not '%c'.", ch);
  Pgetc (pptr);

  /* Get variable name, disable implicit text replacement so that the
   * variable name will not be replaced by a previous directive unless
   * explicitly told to do so.
   * This allows names to be easily to be redefined (otherwise a
   * $\ is needed before the variable name)
   */
  disable_implicit_text_replacement++;

  Pskip_whitespace (pptr);

  /* If '!' appears before name, allow implicit text replacement of
   * this variable.
   */
  if (Ppeekc (pptr) == '!')
    {
      /* Get the '!' and mark that this variable allows implicit text
       * replacement.
       */
      Pgetc (pptr);
      value_list->allow_implicit_replacement = 1;
    }
  else
    {
      /* Otherwise, do not allow implicit text replacement. */
      value_list->allow_implicit_replacement = 0;

    }
  value_list->name = Pget_identifier (pptr);

  /* Restore original value of disable_implicit_text_replacement */
  disable_implicit_text_replacement--;

  Pskip_whitespace (pptr);

  /* Get 'in' followed by whitespace */
  if (Ppeekc (pptr) != 'i')
    Perror (pptr, "Error parsing $for directive, 'in' expected.");
  Pgetc (pptr);
  if (Ppeekc (pptr) != 'n')
    Perror (pptr, "Error parsing $for directive, 'in' expected.");
  Pgetc (pptr);
  if (!isspace (Ppeekc (pptr)))
    Perror (pptr, "Error parsing $for directive, 'in' expected.");
  Pgetc (pptr);
  Pskip_whitespace (pptr);


  /* Build a list of values until ending ')' */
  value_list->first_value = NULL;
  value_list->last_value = NULL;
  value_list->value_count = 0;
  while ((ch = Ppeekc (pptr)) != ')')
    {
      /* Make sure not at EOF */
      if (ch == EOF)
	{
	  Perror (placemark,
		  "Expect matching ')' to terminate $for directive's value "
		  "list.");
	}

      /* Get each value in $for */
      val = Pget_for_string (pptr);
      Pskip_whitespace (pptr);

      /* Add to linked list */
      value_node = (String_Node *) L_alloc (String_Node_pool);

      value_node->string = val;
      value_node->next = NULL;

      if (value_list->last_value != NULL)
	value_list->last_value->next = value_node;
      else
	value_list->first_value = value_node;
      value_list->last_value = value_node;

      /* Update count of values */
      value_list->value_count++;
    }

  /* Get ending parenthesis */
  ch = Pgetc (pptr);

  /* Make sure there are some values defined */
  if (value_list->value_count == 0)
    Perror (pptr, "Values expected in $for.");

  /* Return newly created value list for $for */
  return (value_list);
}

Pptr *
process_for_directive (Pptr * pptr, Pptr * placemark)
{
  Value_List *first_list, *last_list, *list, *next_list;
  String_Node *value_node;
  Pptr *loop_start;
  int ch;

  /* Initialize list pointers */
  first_list = NULL;
  last_list = NULL;

  Pskip_whitespace (pptr);

  /* Move placemark to beginning of $for value list */
  move_Pptr (placemark, pptr);

  /* Get ( */
  if ((ch = Ppeekc (pptr)) != '(')
    {
      Perror (pptr,
	      "Error parsing $for directive, '(' expected not '%c'.", ch);
    }

  /* Determine if multiple value lists are expected */
  Pgetc (pptr);
  Pskip_whitespace (pptr);

  /* Only multilist of have second '(', i.e., $for ((i in ...)(j in ...)) */
  if (Ppeekc (pptr) != '(')
    {
      /* Single list form, move back to initial state */
      move_Pptr (pptr, placemark);

      /* Get the value list for the $for directive */
      list = get_for_value_list (pptr, placemark);

      /* This is the only value list */
      first_list = list;
      last_list = list;
      list->next_list = NULL;
    }
  else
    {
      /* Multi list form, get value lists until hit ending ')' */
      while ((ch = Ppeekc (pptr)) != ')')
	{
	  /* If not ')', it better be '(' */
	  if (ch != '(')
	    {
	      Perror (pptr,
		      "Error parsing $for directive of the form:\n"
		      "   $for ((name1 in ...) (name2 in ...) ...) {body}\n\n"
		      "Expecting '(' or ')', not '%c' at this point.", ch);
	    }

	  /* Get the value list for the $for directive */
	  list = get_for_value_list (pptr, placemark);

	  /* Add this list to the end of the value list list */
	  if (last_list == NULL)
	    first_list = list;
	  else
	    last_list->next_list = list;
	  last_list = list;
	  list->next_list = NULL;

	  /* Make sure this list has the same number of items as
	   * the first list 
	   */
	  if (list->value_count != first_list->value_count)
	    {
	      Perror (placemark,
		      "Value lists must be the same length for $for "
		      "directive of this form:\n"
		      "   $for ((name1 in ...) (name2 in ...) ...) {body}\n\n"
		      "List for '%s' has length %i but list of '%s' has "
		      "length %i!",
		      first_list->name, first_list->value_count,
		      list->name, list->value_count);
	    }

	  /* Make sure the same name is not defined by multiple lists */
	  for (list = first_list; list != last_list; list = list->next_list)
	    {
	      if (strcmp (list->name, last_list->name) == 0)
		{
		  Perror (placemark,
			  "A list for '%s' has already been defined in $for "
			  "directive of form:\n"
			  "   $for ((name1 in ...) (name2 in ...) ...) "
			  "{body}\n", last_list->name);
		}
	    }

	  /* Skip whitespace until next value list or ending ')' */
	  Pskip_whitespace (pptr);
	}

      /* Get ending ')' */
      Pgetc (pptr);

    }

  /* Skip whitespace until start of loop */
  Pskip_whitespace (pptr);

  /* Move placemark to beginning of $for body */
  move_Pptr (placemark, pptr);

  /* Get starting '{' */
  if ((ch = Ppeekc (pptr)) != '{')
    Perror (pptr, "Expect '{' before $for's body.");
  Pgetc (pptr);


  /* Get pointer to start of loop */
  loop_start = copy_Pptr (pptr);

  /* Loop once for each value in first list 
   * (for multiple lists, the values are traversed in parallel)
   */
  while (first_list->value_count > 0)
    {
      /* Move pptr to beginning of loop body */
      move_Pptr (pptr, loop_start);

      /* For each list, set the variable to the current value */
      for (list = first_list; list != NULL; list = list->next_list)
	{
	  /* Give loop variables heighest priority since it doesn't
	   * make sense to allow the command line to override them.
	   */
	  add_Pdef (list->name, list->first_value->string,
		    list->allow_implicit_replacement, 3);
	}

      /* Process the loop body */
      pptr = process_body (pptr);

      /* For each list, undefine variable after processing body */
      for (list = first_list; list != NULL; list = list->next_list)
	{
	  delete_Pdef (list->name);
	}

      /* Get end '}' */
      if ((ch = Pgetc (pptr)) != '}')
	{
	  Perror (placemark,
		  "Matching '}' for $for's body expected before EOF.");
	}

      /* For each list, delete the first value  */
      for (list = first_list; list != NULL; list = list->next_list)
	{
	  /* Get the first value node */
	  value_node = list->first_value;

	  /* Repair the value list and count */
	  list->first_value = value_node->next;
	  if (list->first_value == NULL)
	    list->last_value = NULL;
	  list->value_count--;

	  /* Delete value node */
	  free (value_node->string);
	  L_free (String_Node_pool, value_node);
	}
    }

  /* Free each list */
  for (list = first_list; list != NULL; list = next_list)
    {
      /* Get the next list before deleting this one */
      next_list = list->next_list;

      free (list->name);
      L_free (Value_List_pool, list);
    }

  /* Free copy to loop start */
  free_Pptr (loop_start);


  return (pptr);
}

Pptr *
process_if_directive (Pptr * pptr, Pptr * placemark)
{
  int ch;
  int condition, skip_elses;
  char *directive_type;

  Pskip_whitespace (pptr);

  /* Get ( */
  if ((ch = Ppeekc (pptr)) != '(')
    Perror (pptr, "Error parsing $if directive, '(' expected not '%c'.", ch);
  Pgetc (pptr);

  /* Get expression's value */
  condition = Pcalc_C_int_expr (pptr, 0);

  Pskip_whitespace (pptr);

  /* Get ) */
  if ((ch = Ppeekc (pptr)) != ')')
    Perror (pptr, "Error parsing $if directive, ')' expected not '%c'.", ch);
  Pgetc (pptr);

  /* Skip whitespace until start of body */
  Pskip_whitespace (pptr);

  /* Get starting '{' */
  if ((ch = Ppeekc (pptr)) != '{')
    Perror (pptr, "Expect '{' before $if's body.");
  Pgetc (pptr);

  /* If the condition is met, process the if body, otherwise skip it */
  if (condition)
    {
      /* Process the if body */
      pptr = process_body (pptr);

      /* Mark the elses should be skipped */
      skip_elses = 1;
    }
  else
    {
      pptr = skip_body (pptr);

      /* Mark the elses should be not be skipped */
      skip_elses = 0;
    }

  /* Get end '}' */
  if ((ch = Pgetc (pptr)) != '}')
    Perror (placemark, "End '}' of $if expected before EOF.");


  /* Process $elses or $elifs after $if */
  while (1)
    {
      /* Move placemark to end of last if/else/elif processed */
      move_Pptr (placemark, pptr);

      /* Stop if next token is not $else or $elif */
      Pskip_whitespace (pptr);

      if (Pgetc (pptr) != '$')
	break;

      /* Get the directive type */
      directive_type = Pget_alnum_string (pptr);

      if (strcmp (directive_type, "else") == 0)
	{
	  /* Stop if next token is not $else or $elif */
	  Pskip_whitespace (pptr);

	  /* Get starting '{' */
	  if ((ch = Ppeekc (pptr)) != '{')
	    Perror (pptr, "Expect '{' before $else's body.");
	  Pgetc (pptr);

	  if (!skip_elses)
	    {
	      /* Process the else body */
	      pptr = process_body (pptr);
	    }
	  else
	    {
	      pptr = skip_body (pptr);
	    }

	  /* Get end '}' */
	  if ((ch = Pgetc (pptr)) != '}')
	    Perror (placemark, "End '}' of $else expected before EOF.");

	  /* Move placemark to end of else */
	  move_Pptr (placemark, pptr);

	  /* Free directive type before existing */
	  free (directive_type);
	  break;
	}
      else if (strcmp (directive_type, "elif") == 0)
	{
	  Pskip_whitespace (pptr);

	  /* Get ( */
	  if ((ch = Ppeekc (pptr)) != '(')
	    Perror (pptr,
		    "Error parsing $elif directive, '(' expected not '%c'.",
		    ch);
	  Pgetc (pptr);

	  /* Get expression's value */
	  condition = Pcalc_C_int_expr (pptr, 0);

	  Pskip_whitespace (pptr);

	  /* Get ) */
	  if ((ch = Ppeekc (pptr)) != ')')
	    Perror (pptr,
		    "Error parsing $elif directive, ')' expected not '%c'.",
		    ch);
	  Pgetc (pptr);

	  /* Skip whitespace until start of body */
	  Pskip_whitespace (pptr);

	  /* Get starting '{' */
	  if ((ch = Ppeekc (pptr)) != '{')
	    Perror (pptr, "Expect '{' before $elif's body.");
	  Pgetc (pptr);

	  /* If the condition is met (and not skipping elses), 
	   * process the elif body, otherwise skip it.
	   */
	  if (condition && !skip_elses)
	    {
	      /* Process the if body */
	      pptr = process_body (pptr);

	      /* Mark the elses should be skipped */
	      skip_elses = 1;
	    }
	  else
	    {
	      pptr = skip_body (pptr);
	    }

	  /* Get end '}' */
	  if ((ch = Pgetc (pptr)) != '}')
	    Perror (placemark, "End '}' of $elif expected before EOF.");
	}
      else
	{
	  /* Free directive type before existing */
	  free (directive_type);
	  break;
	}

      /* Free directive type before going to next directive */
      free (directive_type);
    }

  /* Move pptr back to end of last if/else/elif processed */
  move_Pptr (pptr, placemark);

  return (pptr);
}
Pptr *
process_include_directive (Pptr * pptr, Pptr * placemark)
{
  char *file_name;
  Psymbol *psymbol;
  Mfile *include_file;
  Pptr *include_pptr;
  int ch;
  FILE *in;

  /* Require that $include be the first thing on the line
   * and the only thing on the line.
   *
   * These restrictions are required if the preprocessor
   * line directives are going to work properly.
   *
   * Placemark must be 1 character past $.  If change mptr library,
   * this test may not work.
   */
  if (placemark->mptr->pos != 1)
    {
      Perror (placemark,
	      "$include directive must be the first and only thing on "
	      "the line.");
    }

  /* Get quoted string, stripping quotes from string */
  file_name = Pget_quoted_string (pptr, 1);

  /* Make sure only other thing on line is a newline */
  Pskip_whitespace_no_nl (pptr);
  ch = Pgetc (pptr);
  if ((ch != '\n') && (ch != EOF))
    {
      Perror (placemark,
	      "$include directive must be the first and only thing on "
	      "the line.");
    }


  /* Search to see if include file already open, if not
   * open it and add to open file table.
   */
  if ((psymbol = find_Psymbol (Mfile_table, file_name)) != NULL)
    {
      include_file = (Mfile *) psymbol->data;
    }
  else
    {
      if ((in = fopen (file_name, "r")) == NULL)
	{
	  Perror (placemark, "Unable to open include file %s.", file_name);
	}

      include_file = create_Mfile (in, file_name, "preprocessing");
      add_Psymbol (Mfile_table, include_file->name, include_file);
    }

  /* Create pointto to start of include file for preprocessing */
  include_pptr = create_Pptr (include_file);

  /* Include whole file */
  while ((ch = Ppeekc (include_pptr)) != EOF)
    {
      /* Handle unmatched end curly brackets.  Process body ends when
       * encounters this to allow loop constructs to use process_body.
       */
      if (ch == '}')
	{
	  ch = Pgetc (include_pptr);
	  Pputc (include_pptr, ch);
	}
      else
	{
	  include_pptr = process_body (include_pptr);
	}
    }

  /* Free up include_pptr */
  free_Pptr (include_pptr);

  /* Free file_name */
  free (file_name);

  return (pptr);
}


Pptr *
process_directive (Pptr * pptr)
{
  int ch;
  char *directive_type;
  Pptr *placemark;

  /* Allocate placemark for error messages */
  placemark = copy_Pptr (pptr);

  /* Get '$' */
  if ((ch = Pgetc (pptr)) != '$')
    L_punt ("process_directive: $ expected.");

  /* Get the directive type */
  directive_type = Pget_alnum_string (pptr);

  /* Handle whitespace/wierd character after $ */
  if (directive_type[0] == 0)
    {
      Perror (placemark,
	      "Poorly formed preprocessor directive, expecting either:\n"
	      " 1) A backslash before the '$' or\n"
	      " 2) one of the currently supported directives:\n"
	      "  $include \"name_of_file_to_include\"\n"
	      "  $def def_name def_value   "
	      "/* Note: def_value ends at newline */\n"
	      "  $def def_name {bounded_def_value}\n"
	      "  ${name}                   /* Forces replacement of name */\n"
	      "  $\\name                   /* Prevents replacement of name */\n"
	      "  $?{name}                  /* Replaced with 1 if name defined, "
	      "0 otherwise */\n"
	      "  $={integer C expression}\n"
	      "  $.={floating-point C expression}\n"
	      "  $if (cond) {body}   $elif (cond) {body}   $else {body}\n"
	      "  $x..y  or  $(int C expr)..(int C expr)\n"
	      "  $for (def_name in value1 value2 ... value N) {for_body}\n"
	      "  $for ((def_name1 in ...)(def_name2 in...)...){for_body}");
    }

  /* Handle the different types of directives */
  if (strcmp (directive_type, "def") == 0)
    {
      pptr = process_def_directive (pptr);
    }

  else if (strcmp (directive_type, "undef") == 0)
    {
      pptr = process_undef_directive (pptr);
    }

  else if (strcmp (directive_type, "for") == 0)
    {
      pptr = process_for_directive (pptr, placemark);
    }

  else if (strcmp (directive_type, "if") == 0)
    {
      pptr = process_if_directive (pptr, placemark);
    }

  else if (strcmp (directive_type, "else") == 0)
    {
      Perror (placemark, "$else without associated $if.");
    }

  else if (strcmp (directive_type, "elif") == 0)
    {
      Perror (placemark, "$elif without associated $if.");
    }

  else if (strcmp (directive_type, "include") == 0)
    {
      pptr = process_include_directive (pptr, placemark);
    }


  /* Otherwise, unsupported directive type */
  else
    {
      Perror (placemark,
	      "Unsupported preprocessing directive '%s'.\n"
	      "The currently supported directives are:\n"
	      "  $include \"name_of_file_to_include\"\n"
	      "  $def def_name def_value   "
	      "/* Note: def_value ends at newline */\n"
	      "  $def def_name {bounded_def_value}\n"
	      "  ${name}                   /* Forces replacement of name */\n"
	      "  $\\name                   /* Prevents replacement of name */\n"
	      "  $?{name}                  /* Replaced with 1 if name defined, "
	      "0 otherwise */\n"
	      "  $={integer C expression}\n"
	      "  $.={floating-point C expression}\n"
	      "  $if (cond) {body}   $elif (cond) {body}   $else {body}\n"
	      "  $x..y  or  $(int C expr)..(int C expr)\n"
	      "  $for (def_name in value1 value2 ... value N) {for_body}\n"
	      "  $for ((def_name1 in ...)(def_name2 in...)...){for_body}",
	      directive_type);
    }


  /* Free directive type */
  free (directive_type);

  /* Free placemark */
  free_Pptr (placemark);

  return (pptr);
}

char *
Pget_alnum_string (Pptr * pptr)
{
  int ch;
  char *string;

  /* Use temp_mbuf to build string */
  clear_Mbuf (temp_mbuf);

  while ((ch = Ppeekc (pptr)) != EOF)
    {
      if ((ch == '_') || isalnum (ch))
	{
	  ch = Pgetc (pptr);
	  addc_to_Mbuf (temp_mbuf, ch);
	}
      else
	break;
    }

  /* Alloc copy of string in temp_buf */
  string = copy_Mbuf_buf (temp_mbuf);

  return (string);
}

char *
Pget_identifier (Pptr * pptr)
{
  int ch;
  char *string;

  /* Use temp_mbuf to build string */
  clear_Mbuf (temp_mbuf);

  /* Make sure first character is a character or an _ */
  ch = Ppeekc (pptr);

  if ((ch != '_') && !isalpha (ch))
    Perror (pptr, "Error in identifier at '%c'.", ch);

  while ((ch = Ppeekc (pptr)) != EOF)
    {
      /* Stop at whitespace only */
      if (isspace (ch))
	break;

      if ((ch == '_') || isalnum (ch))
	{
	  ch = Pgetc (pptr);
	  addc_to_Mbuf (temp_mbuf, ch);
	}
      else
	Perror (pptr, "Error in identifier at '%c'.", ch);
    }

  /* Alloc copy of string in temp_buf */
  string = copy_Mbuf_buf (temp_mbuf);

  return (string);
}

/* Gets the rest of the line with leading and trailing whitespace stripped */
char *
Pget_stripped_line (Pptr * pptr)
{
  int ch;
  char *string;

  /* Use temp_mbuf to build string */
  clear_Mbuf (temp_mbuf);

  /* Skip leading whitespace */
  Pskip_whitespace_no_nl (pptr);

  /* Stop at EOF or newline */
  while ((ch = Ppeekc (pptr)) != EOF)
    {
      if (ch == '\n')
	break;

      ch = Pgetc (pptr);
      addc_to_Mbuf (temp_mbuf, ch);
    }

  /* Strip trailing space */
  strip_Mbuf (temp_mbuf);

  /* Alloc copy of string in temp_buf */
  string = copy_Mbuf_buf (temp_mbuf);

  return (string);
}

char *
Pget_quoted_string (Pptr * pptr, int strip_quotes)
{
  int ch, quote_ch;
  char *string;

  /* Use temp_mbuf to build string */
  clear_Mbuf (temp_mbuf);

  /* Skip leading whitespace */
  Pskip_whitespace_no_nl (pptr);

  /* Get placemark for error messages */
  move_Pptr (temp_placemark, pptr);

  /* Expect opening quote */
  ch = Ppeekc (pptr);
  if ((ch != '\'') && (ch != '\"'))
    {
      Perror (pptr, "Open quote (\' or \") expected.");
    }
  /* Save opening quote to know what to look for in closing quote */
  ch = Pgetc (pptr);
  quote_ch = ch;

  /* Add quotes to buffer if not stripping quotes from string */
  if (!strip_quotes)
    addc_to_Mbuf (temp_mbuf, ch);

  while (1)
    {
      /* Error to have '$' without a backslash */
      ch = Ppeekc (pptr);
      if (ch == '$')
	{
	  Perror (pptr,
		  "Expecting a backslash before '$' in quoted string.\n"
		  "Note: $def, $undef, $for, $if, and $include directives "
		  "may not\n      be embedded in replacement value strings.");
	}

      /* Get next character */
      ch = Pgetc (pptr);

      /* Error if hit end of line before endquote */
      if ((ch == EOF) || (ch == '\n'))
	{
	  Perror (temp_placemark,
		  "Expecting close quote (%c) before end of line.", quote_ch);
	}

      /* Detect end quote */
      if (ch == quote_ch)
	{
	  /* Add quotes to buffer if not stripping quotes */
	  if (!strip_quotes)
	    addc_to_Mbuf (temp_mbuf, ch);
	  break;
	}

      /* Add character to buffer */
      addc_to_Mbuf (temp_mbuf, ch);

      /* Handle backslash character */
      if (ch == '\\')
	{
	  ch = Ppeekc (pptr);
	  /* Add anything except end of line to buffer */
	  if ((ch != EOF) && (ch != '\n'))
	    {
	      ch = Pgetc (pptr);
	      addc_to_Mbuf (temp_mbuf, ch);
	    }

	}
    }

  string = copy_Mbuf_buf (temp_mbuf);

  return (string);
}



char *
Pget_for_string (Pptr * pptr)
{
  int ch, next_ch;
  char *string;

  /* Use temp_mbuf to build string */
  clear_Mbuf (temp_mbuf);

  /* Skip leading whitespace */
  Pskip_whitespace_no_nl (pptr);

  /* Get placemark for error messages */
  move_Pptr (temp_placemark, pptr);

  /* If first character is a '{', get hit matching '}' */
  if ((ch = Ppeekc (pptr)) == EOF)
    Perror (temp_placemark, "String expected, not EOF.");
  if (ch == '{')
    {
      string = Pget_bounded_string (pptr);
    }

  /* If quoted string, get quoted string (don't strip quotes) */
  else if ((ch == '"') || (ch == '\''))
    {
      string = Pget_quoted_string (pptr, 0);
    }

  /* Get string until next whitespace or ')', 
   * Punt if hit quotes in middle of string.
   */
  else
    {
      while (1)
	{
	  ch = Ppeekc (pptr);

	  if ((ch == EOF) || (ch == ')') || isspace (ch))
	    break;

	  if (ch == '\\')
	    {
	      /* Get backslash and next character */
	      ch = Pgetc (pptr);
	      next_ch = Ppeekc (pptr);

	      if ((next_ch == EOF) || (next_ch == '\n'))
		Perror (pptr, "Character missing after backslash.");

	      next_ch = Pgetc (pptr);

	      addc_to_Mbuf (temp_mbuf, ch);
	      addc_to_Mbuf (temp_mbuf, next_ch);


	    }

	  /* Error to have quote character in middle of unquoted string */
	  else if ((ch == '"') || (ch == '\''))
	    {
	      Perror (pptr,
		      "Unexpected quote.  Expecting one of the following:\n "
		      "1) Whitespace before the quote\n 2) or a backslash "
		      "before the quote.");

	    }

	  /* Error to have '(' in middle of unquoted string */
	  else if (ch == '(')
	    {
	      Perror (pptr, "Unexpected '(' in unquoted string.");
	    }

	  /* Error to have '$' without a backslash */
	  else if (ch == '$')
	    {
	      Perror (pptr,
		      "Expecting a backslash before '$' in unquoted string.\n"
		      "Note: $def, $undef, $for, $if, and $include directives "
		      "may not\n   be embedded in replacement value strings.");
	    }

	  else
	    {
	      ch = Pgetc (pptr);
	      addc_to_Mbuf (temp_mbuf, ch);
	    }
	}

      /* Alloc copy of string in temp_buf */
      string = copy_Mbuf_buf (temp_mbuf);
    }

  return (string);
}

/* Gets string bounded by { }  */
char *
Pget_bounded_string (Pptr * pptr)
{
  int ch;
  char *string;
  int nesting_level;

  /* Use temp_mbuf to build string */
  clear_Mbuf (temp_mbuf);

  /* Skip leading whitespace */
  Pskip_whitespace_no_nl (pptr);

  /* Get placemark for error messages */
  move_Pptr (temp_placemark, pptr);

  /* If first character is a '{', get hit matching '}' */
  if ((ch = Ppeekc (pptr)) == EOF)
    Perror (temp_placemark, "String expected, not EOF.");

  if (ch != '{')
    L_punt ("Pget_bounded_string: '{' expected not '%c'.", ch);

  /* Skip leading { */
  Pgetc (pptr);

  /* Start at nesting depth 1 */
  nesting_level = 1;

  while (1)
    {
      /* Error to have '$' without a backslash */
      ch = Ppeekc (pptr);
      if (ch == '$')
	{
	  Perror (pptr,
		  "Expecting a backslash before '$' in bounded string.\n"
		  "Note: $def, $undef, $for, $if, and $include directives "
		  "may not\n be embedded in replacement value strings.");
	}

      if ((ch = Pgetc (pptr)) == EOF)
	Perror (temp_placemark, "End '}' of bounded string not found.");
      /* Handle backslashed characters, just add both backslash
       * and character to bounded string 
       */
      if (ch == '\\')
	{
	  addc_to_Mbuf (temp_mbuf, ch);
	  if ((ch = Pgetc (pptr)) == EOF)
	    Perror (temp_placemark, "End '}' of bounded string not found.");
	  addc_to_Mbuf (temp_mbuf, ch);
	  continue;
	}


      if (ch == '{')
	nesting_level++;
      else if (ch == '}')
	nesting_level--;
      /* Do not add trailing '}' */
      if (nesting_level <= 0)
	break;
      addc_to_Mbuf (temp_mbuf, ch);
    }

  /* Alloc copy of string in temp_buf */
  string = copy_Mbuf_buf (temp_mbuf);
  return (string);
}

void
Pskip_whitespace (Pptr * pptr)
{
  int ch;
  while ((ch = Ppeekc (pptr)) != EOF)
    {
      if (isspace (ch))
	Pgetc (pptr);
      else
	break;
    }
}

void
Pskip_whitespace_no_nl (Pptr * pptr)
{
  int ch;
  while ((ch = Ppeekc (pptr)) != EOF)
    {
      if ((ch != '\n') && isspace (ch))
	Pgetc (pptr);
      else
	break;
    }
}


/* Create preprocessed file ptr (pptr) to start of mfile */
Pptr *
create_Pptr (Mfile * mfile)
{
  Pptr *pptr;
  pptr = (Pptr *) L_alloc (Pptr_pool);
  pptr->mptr = create_Mptr (mfile);
  pptr->expanded = create_Mbuf ();
  pptr->expanded_pos = 0;
  pptr->quoted = 0;
  pptr->scanned = 0;
  return (pptr);
}

/* Duplicates a Pptr exactly
 * Returns the duplicated Pptr (dup must be freed)
 */
Pptr *
copy_Pptr (Pptr * orig_pptr)
{
  Pptr *new_pptr;
  char *orig_expanded;
  /* alloc and copy over fields */
  new_pptr = (Pptr *) L_alloc (Pptr_pool);
  new_pptr->mptr = copy_Mptr (orig_pptr->mptr);
  /* Allocate new expanded buffer, and copy over contents */
  new_pptr->expanded = create_Mbuf ();
  orig_expanded = get_Mbuf_buf (orig_pptr->expanded);
  adds_to_Mbuf (new_pptr->expanded, orig_expanded);
  new_pptr->expanded_pos = orig_pptr->expanded_pos;
  new_pptr->quoted = orig_pptr->quoted;
  new_pptr->scanned = orig_pptr->scanned;
  return (new_pptr);
}

/*
 * Moves old pptr to new position.
 * Used to jump preprocessed file around.
 */
void
move_Pptr (Pptr * old_pptr, Pptr * new_pptr)
{
  char *new_expanded;
  /* Sanity check, may not be the same pointers */
  if (old_pptr == new_pptr)
    L_punt ("move_Pptr: old_pptr and new_pptr the same.");
  move_Mptr (old_pptr->mptr, new_pptr->mptr);
  /* Copy over contents of expanded buffer */
  clear_Mbuf (old_pptr->expanded);
  new_expanded = get_Mbuf_buf (new_pptr->expanded);
  adds_to_Mbuf (old_pptr->expanded, new_expanded);
  /* Move expanded pointer and state */
  old_pptr->expanded_pos = new_pptr->expanded_pos;
  old_pptr->quoted = new_pptr->quoted;
  old_pptr->scanned = new_pptr->scanned;
}

void
free_Pptr (Pptr * pptr)
{
  /* Free Mptr */
  free_Mptr (pptr->mptr);
  /* Free expanded buffer */
  free_Mbuf (pptr->expanded);
  /* L_free structure */
  L_free (Pptr_pool, pptr);
}

/* Expands a preprocessor directive (if applicable), or just puts 
 * $ in expanded buffer.
 */
void
Pexpand_directive (Pptr * source_pptr)
{
  int ch, next_ch, end_ch;
  Pptr *expand_pptr;
  Mbuf *expand_mbuf;
  int range_start, range_end;
  int int_result;
  double double_result;
  char rbuf[100];
  char *recurse_buf;
  char *ident, *replacement;
  int i;
  /* Get a copy of the source_pptr for recursion to do calculation */
  expand_pptr = copy_Pptr (source_pptr);
  /* Add the entry $ to buffer and consume it by incrementing pos */
  addc_to_Mbuf (expand_pptr->expanded, '$');
  expand_pptr->expanded_pos += 1;
  /* Peek at next character to determine type of directive (if next
   * character is not a backslash (otherwise will cause infinite
   * recursion))
   */
  if ((next_ch = Mpeekc (expand_pptr->mptr)) != '\\')
    {
      next_ch = Ppeekc (expand_pptr);
    }

  /* Handle evaluate integer expression preprocessor directive */
  if ((next_ch == '=') && allow_text_replacement)
    {
      /* Get the '=' */
      Pgetc (expand_pptr);
      /* Expect '{' after '=' */
      if (Ppeekc (expand_pptr) != '{')
	{
	  Perror (expand_pptr,
		  "Expecting '{' after '$=' in 'evaluate integer expression' "
		  "directive.\nForm expected: $={integer expression}.");
	}
      /* Get the '{' */
      Pgetc (expand_pptr);
      /* Get value of expression */
      int_result = Pcalc_C_int_expr (expand_pptr, 0);
      /* Expect '}' at end of expression */
      Pskip_whitespace (expand_pptr);
      if ((end_ch = Ppeekc (expand_pptr)) != '}')
	{
	  if ((end_ch == 'e') || (end_ch == 'E') || (end_ch == '.'))
	    {
	      Perror (expand_pptr,
		      "Expression error or missing '}' in 'evaulate int "
		      "expression' directive.\n"
		      "Form expected: $={integer expression}.\n\n"
		      "For floating-point expressions use: "
		      "$.={floating-point expression}.");
	    }
	  else
	    {
	      Perror (expand_pptr,
		      "Expression error or missing '}' in 'evaulate int "
		      "expression' directive.\n"
		      "Form expected: $={integer expression}.");
	    }
	}
      /* Get the '{' */
      Pgetc (expand_pptr);
      /* Move source_pptr's mptr to after calculation using expand_pptr */
      move_Mptr (source_pptr->mptr, expand_pptr->mptr);
      /* Write result to source_pptr's expanded buffer */
      sprintf (rbuf, "%i", int_result);
      adds_to_Mbuf (source_pptr->expanded, rbuf);
      /* Copy rest of expansion text to expand buffer */
      recurse_buf = get_Mbuf_buf (expand_pptr->expanded);
      adds_to_Mbuf (source_pptr->expanded,
		    &recurse_buf[expand_pptr->expanded_pos]);}

  /* Handle evaluate floating-point expression preprocessor directive */
  else if ((next_ch == '.') && allow_text_replacement)
    {
      /* Get the '.' */
      Pgetc (expand_pptr);
      /* Expect '=' after '.' */
      if (Ppeekc (expand_pptr) != '=')
	{
	  Perror (expand_pptr,
		  "Expecting '=' after '$.' in 'evaluate float expression' "
		  "directive.\nForm expected: $.={floating-point expression}.");
	}
      /* Get the '=' */
      Pgetc (expand_pptr);
      /* Expect '{' after '.=' */
      if (Ppeekc (expand_pptr) != '{')
	{
	  Perror (expand_pptr,
		  "Expecting '{' after '$.=' in 'evaluate float expression' "
		  "directive.\nForm expected: $.={floating-point expression}.");
	}
      /* Get the '{' */
      Pgetc (expand_pptr);
      /* Get value of expression */
      double_result = Pcalc_C_float_expr (expand_pptr, 0);
      /* Expect '}' at end of expression */
      Pskip_whitespace (expand_pptr);
      if (Ppeekc (expand_pptr) != '}')
	{
	  Perror (expand_pptr,
		  "Expression error or missing '}' in 'evaulate float "
		  "expression' directive.\nForm expected: $.={floating-point "
		  "expression}.");
	}
      /* Get the '{' */
      Pgetc (expand_pptr);
      /* Move source_pptr's mptr to after calculation using expand_pptr */
      move_Mptr (source_pptr->mptr, expand_pptr->mptr);
      /* Write result to source_pptr's expanded buffer */
      sprintf (rbuf, "%.16g", double_result);
      adds_to_Mbuf (source_pptr->expanded, rbuf);
      /* Copy rest of expansion text to expand buffer */
      recurse_buf = get_Mbuf_buf (expand_pptr->expanded);
      adds_to_Mbuf (source_pptr->expanded,
		    &recurse_buf[expand_pptr->expanded_pos]);}

  /* Handle 'is defined' directive I.e., $?{name} */
  else if ((next_ch == '?') && allow_text_replacement)
    {
      /* Disable implicit text replacement so that the identifier between
       * ${} will not be replaced with its value unless explicitly told
       * to do so.
       */
      disable_implicit_text_replacement++;
      /* Get the '?' */
      Pgetc (expand_pptr);
      /* Expect '{' after '?' */
      if (Ppeekc (expand_pptr) != '{')
	{
	  Perror (expand_pptr,
		  "Expecting '{' after '$?' in 'is defined?' directive.\n"
		  "Form expected: $?{name}.");
	}

      /* Get the '{' */
      Pgetc (expand_pptr);
      /* Allocate mbuf to build identifier */
      expand_mbuf = create_Mbuf ();
      /* Get the string until the matching right curly bracket */
      while ((ch = Ppeekc (expand_pptr)) != '}')
	{
	  if ((ch == EOF) || (ch == '\n'))
	    Merror (source_pptr->mptr,
		    "Text replacement directive's end '}' not found.");
	  /* Make reasonably sure valid identifier */
	  if ((ch == '_') || isalnum (ch))
	    {
	      /* Get the character and place in buffer */
	      ch = Pgetc (expand_pptr);
	      addc_to_Mbuf (expand_mbuf, ch);
	    }
	  else
	    {
	      Perror (expand_pptr,
		      "Invalid character '%c' in 'is defined?' directive.",
		      ch);}
	}

      /* Get pointer to identifier buffer */
      ident = get_Mbuf_buf (expand_mbuf);
      /* If text replacement exists, replace directive with '1' */
      if ((replacement = Plookup (ident, 0)) != NULL)
	addc_to_Mbuf (source_pptr->expanded, '1');
      /* Otherwise, '0' */
      else
	{
	  addc_to_Mbuf (source_pptr->expanded, '0');
	}

      /* Free expand mbuf */
      free_Mbuf (expand_mbuf);
      /* Restore original value of disable_implicit_text_replacement */
      disable_implicit_text_replacement--;
      /* Get the right curly bracket */
      Pgetc (expand_pptr);
      /* Move source_mptr to where expand_pptr->mptr is now */
      move_Mptr (source_pptr->mptr, expand_pptr->mptr);
    }

  /* Handle range expansion.  Ie. $0..10 */
  else
    if (
	(isdigit (next_ch) || (next_ch == '-')
	 || (next_ch == '+') || (next_ch == '(')) && allow_text_replacement)
    {
      /* Get the start of the range.
       * Use Pcalc_C_int_factor instead of Pcalc_C_int_expr so that only 
       * integers and parenthetical expressions are allowed.
       * Using Pcalc_C_int_expr cause it to consume trailing negative 
       * numbers.
       */
      range_start = Pcalc_C_int_factor (expand_pptr);
      /* Skip whitespace and make sure next two characters are '..' */
      Pskip_whitespace (expand_pptr);
      /* Peek at each '.' in range expression so error message goes
       * in correct place.
       */
      if ((Ppeekc (expand_pptr) != '.')
	  || (Pgetc (expand_pptr) != '.')
	  || (Ppeekc (expand_pptr) != '.') || (Pgetc (expand_pptr) != '.'))
	Perror (expand_pptr,
		"Error in range expression: '..' expected between start and "
		"end range.");
      /* Skip whitespace before end range */
      Pskip_whitespace (expand_pptr);
      /* Use Pcalc_C_int_factor instead of Pcalc_C_int_expr so that only 
       * integers and parenthetical expressions are allowed.
       * Using Pcalc_C_int_expr cause it to consume trailing negative 
       * numbers.
       */
      range_end = Pcalc_C_int_factor (expand_pptr);
      /* For now */
      if (range_start <= range_end)
	{
	  for (i = range_start; i <= range_end; i++)
	    {
	      sprintf (rbuf, "%i", i);
	      adds_to_Mbuf (source_pptr->expanded, rbuf);
	      if (i < range_end)
		addc_to_Mbuf (source_pptr->expanded, ' ');
	    }
	}
      else
	{
	  for (i = range_start; i >= range_end; i--)
	    {
	      sprintf (rbuf, "%i", i);
	      adds_to_Mbuf (source_pptr->expanded, rbuf);
	      if (i > range_end)
		addc_to_Mbuf (source_pptr->expanded, ' ');
	    }
	}

      /* Move source_pptr's mptr to after calculation using expand_pptr */
      move_Mptr (source_pptr->mptr, expand_pptr->mptr);
      /* Copy rest of expansion text to expand buffer */
      recurse_buf = get_Mbuf_buf (expand_pptr->expanded);
      adds_to_Mbuf (source_pptr->expanded,
		    &recurse_buf[expand_pptr->expanded_pos]);}

  /* Handle substitution directives */
  else if ((next_ch == '{') && allow_text_replacement)
    {
      /* Disable implicit text replacement so that the identifier between
       * ${} will not be replaced with its value unless explicitly told
       * to do so.
       */
      disable_implicit_text_replacement++;
      /* Get the left curly bracket */
      Pgetc (expand_pptr);
      /* Allocate mbuf to build identifier */
      expand_mbuf = create_Mbuf ();
      /* Get the string until the matching right curly bracket */
      while ((ch = Ppeekc (expand_pptr)) != '}')
	{
	  if ((ch == EOF) || (ch == '\n'))
	    Merror (source_pptr->mptr,
		    "Text replacement directive's end '}' not found.");
	  /* Make reasonably sure valid identifier */
	  if ((ch == '_') || isalnum (ch))
	    {
	      /* Get the character and place in buffer */
	      ch = Pgetc (expand_pptr);
	      addc_to_Mbuf (expand_mbuf, ch);
	    }

	  else
	    {
	      Perror (expand_pptr,
		      "Invalid character '%c' in text replacement directive.",
		      ch);}
	}


      /* Get pointer to identifier buffer */
      ident = get_Mbuf_buf (expand_mbuf);
      /* Replace text if exists otherwise punt 
       * (in future, allow environment search also)
       */
      if ((replacement = Plookup (ident, 0)) != NULL)
	adds_to_Mbuf (source_pptr->expanded, replacement);
      else
	{
	  Perror (expand_pptr,
		  "Undefined name '%s' in text replacement directive.",
		  ident);}

      /* Free expand mbuf */
      free_Mbuf (expand_mbuf);
      /* Restore original value of disable_implicit_text_replacement */
      disable_implicit_text_replacement--;
      /* Get the right curly bracket */
      Pgetc (expand_pptr);
      /* Move source_mptr to where expand_pptr->mptr is now */
      move_Mptr (source_pptr->mptr, expand_pptr->mptr);
    }

  /* Handle $\ , to prevent preprocessing of a name 
   * Special case, work directly with mptr because do not
   * want any preprocessing of next character
   */
  else if (next_ch == '\\')
    /* Handle even when not doing text replacement */
    {
      /* Get backslash */
      Mgetc (source_pptr->mptr);
      /* Peek at next character, if newline, consume newline,
       * otherwise, flag that next character has been scanned
       * (prevents text replacement of next identifier).
       */
      if (Mpeekc (source_pptr->mptr) == '\n')
	Mgetc (source_pptr->mptr);
      else
	{
	  /* Flag that remaining text has already been scanned */
	  source_pptr->scanned = 1;
	}
    }
  /* Otherwise, just put $ in expanded buffer */
  else
    {
      addc_to_Mbuf (source_pptr->expanded, '$');
    }

  /* Free expansion Pptr, not needed after expansion */
  free_Pptr (expand_pptr);
}

/* Expands the file text, as necessary to allow processing of the next
 * preprocessed character in the file.
 * 
 * Returns 1 if successful (may have done nothing), 0 if at EOF.
 */
int
Pexpand_text (Pptr * pptr)
{
  char ch, next_ch, first_ch;
  Mptr *mptr;
  char *ident, *replacement;
  char nesting_level;
  /* Get mptr from pptr for ease of use */
  mptr = pptr->mptr;
  /* Loop until there is something for the caller to process
   * at pptr->expanded_pos.
   * Do nothing if already something there on entry.
   */
  while (pptr->expanded->cur_len <= pptr->expanded_pos)
    {

      /* If at start of new line, clear expanded buf */
      if (Mptr_pos (mptr) == 0)
	{
	  /* Clear expanded text, starting processing of new line */
	  clear_Mbuf (pptr->expanded);
	  pptr->expanded_pos = 0;
	  /* End processing of quoted text if hit end of line */
	  pptr->quoted = 0;
	}

      /* Get the next character from the file, return 0 at EOF */
      if ((ch = Mgetc (mptr)) == '\0')
	return (0);
      /* If it is not a alnum or _ then we have not scanned it yet */
      if ((ch != '_') && !isalnum (ch))
	{
	  pptr->scanned = 0;
	}

      /* If not in quoted text or text we have already scanned then: 
       *   if start of identifier, read in identifier and do
       *   text replacement if necessary.
       */
      if ((!pptr->quoted) && (!pptr->scanned) &&
	  ((ch == '_') || isalpha (ch)))
	{
	  /* Set placemark so can rewind if not do not do text replacement */
	  move_Mptr (expand_placemark, mptr);
	  first_ch = ch;
	  /* Use pptr_mbuf to build identifier */
	  clear_Mbuf (pptr_mbuf);
	  addc_to_Mbuf (pptr_mbuf, ch);
	  /* Get rest of identifier */
	  while ((ch = Mpeekc (mptr)) != EOF)
	    {
	      if ((ch == '_') || isalnum (ch))
		{
		  ch = Mgetc (mptr);
		  addc_to_Mbuf (pptr_mbuf, ch);
		}
	      else
		break;
	    }

	  /* Get pointer to identifier buffer */
	  ident = get_Mbuf_buf (pptr_mbuf);
	  /* Use replacement text if exists, 
	   * otherwise rewind and put first character in expand text
	   * and set scanned flag.
	   */
	  if (allow_text_replacement &&
	      (disable_implicit_text_replacement == 0) &&
	      (replacement = Plookup (ident, 1)) != NULL)
	    adds_to_Mbuf (pptr->expanded, replacement);
	  else
	    {
	      move_Mptr (mptr, expand_placemark);
	      pptr->scanned = 1;
	      addc_to_Mbuf (pptr->expanded, first_ch);
	    }
	}

      /* If not in quoted text,
       * Handle calculation directives .
       * (later substitution directives)
       */
      else if ((!pptr->quoted) && (ch == '$'))
	{
	  Pexpand_directive (pptr);
	}

      /* Handle slashes and comments */
      else if (ch == '/')
	{
	  /* Look at the next character to see what to do
	   * will be EOF if at end of file
	   */
	  ch = Mpeekc (mptr);
	  /* Handle C++ line comments */
	  if (ch == '/')
	    {
	      /* Consume file until hit newline (leaving it there)
	       * or hit EOF.
	       */
	      while ((ch = Mpeekc (mptr)) != EOF)
		{
		  if (ch == '\n')
		    break;
		  Mgetc (mptr);
		}
	    }
	  /* Handle C (possibly nested) comments */
	  else if (ch == '*')
	    {
	      /* Set placemark  for error messages */
	      move_Mptr (expand_placemark, mptr);
	      /* Consume '*' */
	      Mgetc (mptr);
	      /* Consume the comment until hit end comment */
	      nesting_level = 1;
	      while (nesting_level > 0)
		{
		  if ((ch = Mgetc (mptr)) == EOF)
		    Merror (expand_placemark, "End of comment not found.");
		  /* Peek at next character to detect start/end comment
		   * delimiters.
		   */
		  next_ch = Mpeekc (mptr);
		  /* Handle backslashes */
		  if (ch == '\\')
		    {
		      /* Consume next character without looking at it */
		      Mgetc (mptr);
		    }


		  /* Detect start commment delimiter */
		  if ((ch == '/') && (next_ch == '*'))
		    {
		      /* Consume '*' */
		      Mgetc (mptr);
		      nesting_level++;
		    }

		  /* Detect end comment delimiter */
		  if ((ch == '*') && (next_ch == '/'))
		    {
		      /* Consume '/' */
		      Mgetc (mptr);
		      nesting_level--;
		    }
		}
	    }

	  /* Otherwise, just add / */
	  else
	    {
	      addc_to_Mbuf (pptr->expanded, '/');
	    }
	}

      /* Otherwise, add character to expanded buffer */
      else
	{
	  addc_to_Mbuf (pptr->expanded, ch);
	  /* Do not preprocess single quoted strings,
	   * Maintain quoted flag state (to prevent text processing above)
	   */
	  if (ch == '\'')
	    {
	      if (pptr->quoted)
		pptr->quoted = 0;
	      else
		pptr->quoted = 1;
	    }

	  /* If have backslash, add next character newline or at EOF */
	  else if (ch == '\\')
	    {
	      ch = Mpeekc (mptr);
	      if ((ch != '\n') && (ch != EOF))
		{
		  ch = Mgetc (mptr);
		  addc_to_Mbuf (pptr->expanded, ch);
		}
	    }
	}
    }

  /* Success, return 1 */
  return (1);
}

/* Gets the next preprocessed character from the file. */
int
Pgetc (Pptr * pptr)
{
  int ch;
  char *expanded_buf;
  /* If not at end of expanded text, get next character */
  if (Pexpand_text (pptr))
    {
      /* Get next character from the expanded buffer */
      expanded_buf = get_Mbuf_buf (pptr->expanded);
      ch = expanded_buf[pptr->expanded_pos];
      pptr->expanded_pos++;
    }

  /* Otherwise, at EOF return EOF */
  else
    {
      ch = EOF;
    }
  return (ch);
}

/* Peeks at the next character to be returned */
int
Ppeekc (Pptr * pptr)
{
  int ch;
  char *expanded_buf;
  /* If not at end of expanded text, get next character */
  if (Pexpand_text (pptr))
    {
      /* Peek at Get next character from the expanded buffer */
      expanded_buf = get_Mbuf_buf (pptr->expanded);
      ch = expanded_buf[pptr->expanded_pos];
    }

  /* Otherwise, at EOF return EOF */
  else
    ch = EOF;
  return (ch);
}

/* Calculates the value of an integer C expression using any non-assignment 
 * operation in C.  Start with a current_precedence of 0.
 */
int
Pcalc_C_int_expr (Pptr * pptr, int current_precedence)
{
  int result;
  int ch, ch2;
  int unary_operator;
  int temp_result;
  Pptr *placemark;
  /* Detect unary operators before first factor */
  Pskip_whitespace (pptr);
  ch = Ppeekc (pptr);
  if ((ch == '!') || (ch == '~'))
    {
      /* Get unary operator and skip whitespace until next character */
      unary_operator = ch;
      Pgetc (pptr);
      Pskip_whitespace (pptr);
      ch = Ppeekc (pptr);
    }
  else
    unary_operator = 0;
  /* Get the first factor */
  result = Pcalc_C_int_factor (pptr);
  /* Processor unary operator (if any) */
  if (unary_operator != 0)
    {
      switch (unary_operator)
	{
	case '!':
	  result = !result;
	  break;
	case '~':
	  result = ~result;
	  break;
	default:
	  Perror (pptr,
		  "Algorithm error: undefined unary operator '%c'.",
		  unary_operator);}
    }

  /* Allocate placemark because can be recursively called in 
   * inner loop.
   */
  placemark = copy_Pptr (pptr);
  /* Add/Sub trace remaining terms */
  while (1)
    {
      /* Move placemark to current pos so can rewind if at end of expr */
      move_Pptr (placemark, pptr);
      Pskip_whitespace (pptr);
      /* Get next potential token and peek at character after it for
       * two character tokens 
       */
      ch = Pgetc (pptr);
      ch2 = Ppeekc (pptr);
      /* Process token if token matches and current level of predecence
       * is lower than the tokens precedence.
       * 
       * Check two character tokens first before one character tokens
       */
      /* 
       * Check two character tokens
       */
      if ((ch == '<') && (ch2 == '<') && (current_precedence < 11))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result << Pcalc_C_int_expr (pptr, 11);
	}

      else if ((ch == '>') && (ch2 == '>') && (current_precedence < 11))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result >> Pcalc_C_int_expr (pptr, 11);
	}

      else if ((ch == '<') && (ch2 == '=') && (current_precedence < 10))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result <= Pcalc_C_int_expr (pptr, 10);
	}

      else if ((ch == '>') && (ch2 == '=') && (current_precedence < 10))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result >= Pcalc_C_int_expr (pptr, 10);
	}

      else if ((ch == '=') && (ch2 == '=') && (current_precedence < 9))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result == Pcalc_C_int_expr (pptr, 9);
	}

      else if ((ch == '!') && (ch2 == '=') && (current_precedence < 9))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result != Pcalc_C_int_expr (pptr, 9);
	}

      else if ((ch == '&') && (ch2 == '&') && (current_precedence < 5))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  /* Get RHS of && into temp, because C short circutes evaluation */
	  temp_result = Pcalc_C_int_expr (pptr, 5);
	  result = result && temp_result;
	}

      else if ((ch == '|') && (ch2 == '|') && (current_precedence < 4))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  /* Get RHS of || into temp, because C short circutes evaluation */
	  temp_result = Pcalc_C_int_expr (pptr, 4);
	  result = result || temp_result;
	}

      /* 
       * Now check one character tokens 
       */

      else if ((ch == '*') && (current_precedence < 13))
	result = result * Pcalc_C_int_expr (pptr, 13);
      else if ((ch == '/') && (current_precedence < 13))
	{
	  /* Detect divide by zero */
	  temp_result = Pcalc_C_int_expr (pptr, 13);
	  if (temp_result == 0)
	    Perror (pptr, "Expression attempts to divide by zero.");
	  result = result / temp_result;
	}

      else if ((ch == '%') && (current_precedence < 13))
	result = result % Pcalc_C_int_expr (pptr, 13);
      else if ((ch == '+') && (current_precedence < 12))
	result = result + Pcalc_C_int_expr (pptr, 12);
      else if ((ch == '-') && (current_precedence < 12))
	result = result - Pcalc_C_int_expr (pptr, 12);
      else if ((ch == '<') && (current_precedence < 10))
	result = result < Pcalc_C_int_expr (pptr, 10);
      else if ((ch == '>') && (current_precedence < 10))
	result = result > Pcalc_C_int_expr (pptr, 10);
      /* Must check second char since first char alone higher prededence 
       * than &&
       */
      else if ((ch == '&') && (ch2 != '&') && (current_precedence < 8))
	result = result & Pcalc_C_int_expr (pptr, 8);
      else if ((ch == '^') && (current_precedence < 7))
	result = result ^ Pcalc_C_int_expr (pptr, 7);
      /* Must check second char since first char alone higher prededence 
       * than ||
       */
      else if ((ch == '|') && (ch2 != '|') && (current_precedence < 6))
	result = result | Pcalc_C_int_expr (pptr, 6);
      else
	{
	  /* Go back to placemark position before returning */
	  move_Pptr (pptr, placemark);
	  break;
	}
    }

  /* Free placemark */
  free_Pptr (placemark);
  return (result);
}

int
Pcalc_C_int_factor (Pptr * pptr)
{
  int result = 0;
  int ch, sign_operator;
  Pptr *placemark;
  Pskip_whitespace (pptr);
  ch = Ppeekc (pptr);
  /* Detect sign operators */
  if ((ch == '-') || (ch == '+'))
    {
      /* Get sign operator and skip whitespace until next character */
      sign_operator = ch;
      Pgetc (pptr);
      Pskip_whitespace (pptr);
      ch = Ppeekc (pptr);
    }
  else
    sign_operator = 0;
  /* Handle numbers */
  if (isdigit (ch))
    {
      /* Get first number */
      ch = Pgetc (pptr);
      result = ch - '0';
      /* Detect Octal and Hex numbers (not currently supported) */
      if (ch == '0')
	{
	  ch = Ppeekc (pptr);
	  if (isdigit (ch))
	    {
	      Perror (pptr,
		      "Octal representation (numbers with 0 prefix) not "
		      "supported in expressions.");
	    }
	  else if ((ch == 'x') || (ch == 'X'))
	    {
	      Perror (pptr,
		      "Hex representation (numbers with 0x prefix) not "
		      "supported in expressions.");}
	}

      /* Allocate placemark, since can be recursively called 
       * inside loop.
       */
      placemark = copy_Pptr (pptr);
      /* Get remaining digits */
      while (1)
	{
	  /* Get placemark so can restore if not a character */
	  move_Pptr (placemark, pptr);
	  /* Get and process character */
	  ch = Ppeekc (pptr);
	  if (isdigit (ch))
	    {
	      ch = Pgetc (pptr);
	      result = (result * 10) + (ch - '0');
	    }
	  else
	    {
	      /* Restore to right after number */
	      move_Pptr (pptr, placemark);
	      break;
	    }
	}

      free_Pptr (placemark);
    }

  /* Handle parenthesis */
  else if (ch == '(')
    {
      /* Get '(', get internal expression value, and ending ')' */
      Pgetc (pptr);
      result = Pcalc_C_int_expr (pptr, 0);
      Pskip_whitespace (pptr);
      if ((ch = Ppeekc (pptr)) == ')')
	Pgetc (pptr);
      else
	{
	  if ((ch == 'e') || (ch == 'E') || (ch == '.'))
	    {
	      Perror (pptr,
		      "Integer expression error: ')' expected.\n\n"
		      "For floating-point expressions use: "
		      "$.={floating-point expression}.");}
	  else
	    {
	      Perror (pptr, "Integer expression error: ')' expected.");
	    }
	}
    }

  else
    {
      if (sign_operator != 0)
	{
	  if (ch == '.')
	    {
	      Perror (pptr,
		      "Integer expression error:\n"
		      "   integer or '(' expected.\n\n"
		      "For floating-point expressions use the following form:\n"
		      "   $.={floating-point expression}.");}
	  else
	    {
	      Perror (pptr,
		      "Integer expression error:\n"
		      "   integer or '(' expected.");}
	}
      else
	{
	  if (ch == '.')
	    {
	      Perror (pptr,
		      "Integer expression error:\n"
		      "    integer , '(', or sign (-, +) expected.\n\n"
		      "For floating-point expressions use the following form:\n"
		      "   $.={floating-point expression}.");}
	  else
	    {
	      Perror (pptr,
		      "Integer expression error:\n"
		      "    integer , '(', or sign (-, +) expected.");}
	}

    }

  if (sign_operator != 0)
    {
      switch (sign_operator)
	{
	case '-':
	  result = -result;
	  break;
	case '+':
	  break;
	default:
	  Perror (pptr,
		  "Algorithm error: undefined sign operator '%c'.",
		  sign_operator);}
    }


  return (result);
}



/* Calculates the value of an floating-point C expression using any 
 * non-assignment operation in C.  Start with a current_precedence of 0.
 */
double
Pcalc_C_float_expr (Pptr * pptr, int current_precedence)
{
  double result;
  int ch, ch2;
  int unary_operator = 0;
  double temp_result;
  Pptr *placemark;
  /* Detect unary operators before first factor */
  Pskip_whitespace (pptr);
  ch = Ppeekc (pptr);
  if (ch == '!')
    {
      /* Get unary operator and skip whitespace until next character */
      unary_operator = ch;
      Pgetc (pptr);
      Pskip_whitespace (pptr);
      ch = Ppeekc (pptr);
    }
  else if (ch == '~')
    {
      Perror (pptr, "'~' not supported in floating point expressions.");
    }
  else
    unary_operator = 0;
  /* Get the first factor */
  result = Pcalc_C_float_factor (pptr);
  /* Processor unary operator (if any) */
  if (unary_operator != 0)
    {
      switch (unary_operator)
	{
	case '!':
	  result = !result;
	  break;
	default:
	  Perror (pptr,
		  "Algorithm error: undefined unary operator '%c'.",
		  unary_operator);}
    }

  /* Allocate placemark because can be recursively called in 
   * inner loop.
   */
  placemark = copy_Pptr (pptr);
  /* Add/Sub trace remaining terms */
  while (1)
    {
      /* Move placemark to current pos so can rewind if at end of expr */
      move_Pptr (placemark, pptr);
      Pskip_whitespace (pptr);
      /* Get next potential token and peek at character after it for
       * two character tokens 
       */
      ch = Pgetc (pptr);
      ch2 = Ppeekc (pptr);
      /* Process token if token matches and current level of predecence
       * is lower than the tokens precedence.
       * 
       * Check two character tokens first before one character tokens
       */
      /* 
       * Check two character tokens
       */
      if ((ch == '<') && (ch2 == '<') && (current_precedence < 11))
	{
	  Perror (pptr, "'<<' not supported in floating point expressions.");
	}

      else if ((ch == '>') && (ch2 == '>') && (current_precedence < 11))
	{
	  Perror (pptr, "'>>' not supported in floating point expressions.");
	}

      else if ((ch == '<') && (ch2 == '=') && (current_precedence < 10))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result <= Pcalc_C_float_expr (pptr, 10);
	}

      else if ((ch == '>') && (ch2 == '=') && (current_precedence < 10))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result >= Pcalc_C_float_expr (pptr, 10);
	}

      else if ((ch == '=') && (ch2 == '=') && (current_precedence < 9))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result == Pcalc_C_float_expr (pptr, 9);
	}

      else if ((ch == '!') && (ch2 == '=') && (current_precedence < 9))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  result = result != Pcalc_C_float_expr (pptr, 9);
	}

      else if ((ch == '&') && (ch2 == '&') && (current_precedence < 5))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  /* Get RHS of && into temp, because C short circutes evaluation */
	  temp_result = Pcalc_C_float_expr (pptr, 5);
	  result = result && temp_result;
	}

      else if ((ch == '|') && (ch2 == '|') && (current_precedence < 4))
	{
	  /* Consume second character of token */
	  Pgetc (pptr);
	  /* Get RHS of || into temp, because C short circutes evaluation */
	  temp_result = Pcalc_C_float_expr (pptr, 4);
	  result = result || temp_result;
	}

      /* 
       * Now check one character tokens 
       */

      else if ((ch == '*') && (current_precedence < 13))
	result = result * Pcalc_C_float_expr (pptr, 13);
      else if ((ch == '/') && (current_precedence < 13))
	{
	  /* Detect divide by zero */
	  temp_result = Pcalc_C_float_expr (pptr, 13);
	  if (temp_result == 0.0)
	    Perror (pptr, "Float expression attempts to divide by zero.");
	  result = result / temp_result;
	}

      else if ((ch == '%') && (current_precedence < 13))
	Perror (pptr, "'%' not supported in floating point expressions.");
      else if ((ch == '+') && (current_precedence < 12))
	result = result + Pcalc_C_float_expr (pptr, 12);
      else if ((ch == '-') && (current_precedence < 12))
	result = result - Pcalc_C_float_expr (pptr, 12);
      else if ((ch == '<') && (current_precedence < 10))
	result = result < Pcalc_C_float_expr (pptr, 10);
      else if ((ch == '>') && (current_precedence < 10))
	result = result > Pcalc_C_float_expr (pptr, 10);
      /* Must check second char since first char alone higher prededence 
       * than &&
       */
      else if ((ch == '&') && (ch2 != '&') && (current_precedence < 8))
	Perror (pptr, "'&' not supported in floating point expressions.");
      else if ((ch == '^') && (current_precedence < 7))
	Perror (pptr, "'^' not supported in floating point expressions.");
      /* Must check second char since first char alone higher prededence 
       * than ||
       */
      else if ((ch == '|') && (ch2 != '|') && (current_precedence < 6))
	Perror (pptr, "'|' not supported in floating point expressions.");
      else
	{
	  /* Go back to placemark position before returning */
	  move_Pptr (pptr, placemark);
	  break;
	}
    }

  /* Free placemark */
  free_Pptr (placemark);
  return (result);
}

double
Pcalc_C_float_factor (Pptr * pptr)
{
  double result = 0.0;
  int ch, sign_operator;
  Pptr *placemark;
  Mbuf *double_mbuf;
  int after_decimal_point, in_exponent;
  char *double_string, *end_ptr;
  Pskip_whitespace (pptr);
  ch = Ppeekc (pptr);
  /* Detect sign operators */
  if ((ch == '-') || (ch == '+'))
    {
      /* Get sign operator and skip whitespace until next character */
      sign_operator = ch;
      Pgetc (pptr);
      Pskip_whitespace (pptr);
      ch = Ppeekc (pptr);
    }
  else
    sign_operator = 0;
  /* Handle numbers */
  if (isdigit (ch) || (ch == '.'))
    {
      /* Initialize state to allow only one decimal point or
       * exponent.
       */
      after_decimal_point = 0;
      in_exponent = 0;
      /* Create a mbuf to hold number being built */
      double_mbuf = create_Mbuf ();
      /* Add the first character to the mbuf */
      ch = Pgetc (pptr);
      addc_to_Mbuf (double_mbuf, ch);
      /* If decimal point, require the next character to be
       * a digit.
       */
      if (ch == '.')
	{
	  ch = Ppeekc (pptr);
	  if (!isdigit (ch))
	    {
	      Perror (pptr,
		      "Digit required after '.' in floating-point expression.");
	    }

	  /* Flag that have seen the only decimal point allowed */
	  after_decimal_point = 1;
	}

      /* Allocate placemark, since can be recursively called 
       * inside loop.
       */
      placemark = copy_Pptr (pptr);
      /* Get remaining digits */
      while (1)
	{
	  /* Get placemark so can restore if not a character */
	  move_Pptr (placemark, pptr);
	  /* Get and process character */
	  ch = Ppeekc (pptr);
	  /* If digit, just place in string */
	  if (isdigit (ch))
	    {
	      ch = Pgetc (pptr);
	      addc_to_Mbuf (double_mbuf, ch);
	    }

	  /* If decimal point, make sure in valid spot */
	  else if (ch == '.')
	    {
	      /* Cannot have two decimal points */
	      if (after_decimal_point)
		{
		  Perror (pptr,
			  "Unexpected second decimal point in floating-point "
			  "number.");
		}

	      /* Cannot have decimal point in exponent */
	      if (in_exponent)
		{
		  Perror (pptr, "Unexpected decimal point in exponent.");
		}

	      /* Add decimal point to string */
	      ch = Pgetc (pptr);
	      addc_to_Mbuf (double_mbuf, ch);
	      /* Flag that after decimal point */
	      after_decimal_point = 1;
	    }

	  /* Handle exponent specifier */
	  else if ((ch == 'e') || (ch == 'E'))
	    {
	      /* Make don't already in exponent */
	      if (in_exponent)
		{
		  Perror (pptr, "Unexpected second exponent specified.");
		}

	      /* Flag that in exponent now */
	      in_exponent = 1;
	      /* Add exponent specifier to string */
	      ch = Pgetc (pptr);
	      addc_to_Mbuf (double_mbuf, ch);
	      /* Get optional + or - */
	      ch = Ppeekc (pptr);
	      if ((ch == '+') || (ch == '-'))
		{
		  ch = Pgetc (pptr);
		  addc_to_Mbuf (double_mbuf, ch);
		}

	      /* Make sure followed by a digit (with or without sign) */
	      ch = Ppeekc (pptr);
	      if (!isdigit (ch))
		{
		  Perror (pptr, "Exponent expected not '%c'.", ch);
		}
	    }
	  else
	    {
	      /* Restore to right after number */
	      move_Pptr (pptr, placemark);
	      break;
	    }
	}

      /* Convert double string to double */
      double_string = get_Mbuf_buf (double_mbuf);
      result = strtod (double_string, &end_ptr);
      /* Make sure have legal double */
      if (*end_ptr != 0)
	{
	  Perror (pptr, "Invalid float number '%s' in expression.",
		  double_string);}

      /* Free mbuf (this also frees double_string!) */
      free_Mbuf (double_mbuf);
      free_Pptr (placemark);
    }

  /* Handle parenthesis */
  else if (ch == '(')
    {
      /* Get '(', get internal expression value, and ending ')' */
      Pgetc (pptr);
      result = Pcalc_C_float_expr (pptr, 0);
      Pskip_whitespace (pptr);
      if ((ch = Ppeekc (pptr)) == ')')
	Pgetc (pptr);
      else
	{
	  Perror (pptr, "Floating-point expression error: ')' expected.");
	}
    }

  else
    {
      if (sign_operator != 0)
	{
	  Perror (pptr,
		  "Floating-point expression error: integer, '.', or '(' "
		  "expected.");
	}
      else
	{
	  Perror (pptr,
		  "Floating-point expression error: integer , '(', '.', or "
		  "sign (-, +) expected.");
	}

    }

  if (sign_operator != 0)
    {
      switch (sign_operator)
	{
	case '-':
	  result = -result;
	  break;
	case '+':
	  break;
	default:
	  Perror (pptr,
		  "Algorithm error: undefined sign operator '%c'.",
		  sign_operator);}
    }


  return (result);
}



/* Create Pdef structure */
Pdef *
create_Pdef (char *name, char *val, int allow_implicit_replacement, int level)
{
  Pdef *def;
  def = (Pdef *) L_alloc (Pdef_pool);
  def->name = strdup (name);
  def->val = strdup (val);
  def->allow_implicit_replacement = allow_implicit_replacement;
  def->level = level;
  return (def);
}

void
free_Pdef (void *def_v)
{
  Pdef *def;
  def = (Pdef *) def_v;
  free (def->name);
  free (def->val);
  L_free (Pdef_pool, def);
}

void
add_Pdef (char *name, char *val, int allow_implicit_replacement, int level)
{
  Psymbol *psymbol;
  Pdef *pdef;
  /* If symbol already in table, replace val, otherwise make new
   * entry.
   */
  psymbol = find_Psymbol (Pdef_table, name);
  if (psymbol == NULL)
    {
      pdef = create_Pdef (name, val, allow_implicit_replacement, level);
      add_Psymbol (Pdef_table, name, (void *) pdef);
    }
  else
    {
      pdef = (Pdef *) psymbol->data;
      /* Ignore new definition if existing defintion is at a 
       * higher level.
       */
      if (pdef->level > level)
	return;
      free (pdef->val);
      pdef->val = strdup (val);
      pdef->allow_implicit_replacement = allow_implicit_replacement;
      pdef->level = level;
    }
}

void
delete_Pdef (char *name)
{
  Psymbol *psymbol;
  /* Delete only if in symbol table */
  psymbol = find_Psymbol (Pdef_table, name);
  if (psymbol != NULL)
    delete_Psymbol (psymbol, free_Pdef);
}

/* Looks up name in Pdef_table and returns value if found, NULL otherwise 
 * If implicit replacement is set to 1, allow only definitions with
 * allow_implicit_replacement set to be found.
 */
char *
Plookup (char *name, int implicit_replacement)
{
  Psymbol *symbol;
  Pdef *def;
  if ((symbol = find_Psymbol (Pdef_table, name)) != NULL)
    {
      def = (Pdef *) symbol->data;
      /* If an implicit replacement, allow only defs with 
       * allow_implicit_replacement set to be found.
       */
      if (implicit_replacement && !def->allow_implicit_replacement)
	return (NULL);
      return (def->val);
    }

  return (NULL);
}
