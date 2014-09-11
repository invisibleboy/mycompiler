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
 *      File:   md_compiler.c
 *
 *      Description: The compiler for the IMPACT Meta-Description Language
 *
 *      Creation Date:  Feb. 1995
 *
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *
 *      John C. Gyllenhaal January 1998
 *           Implemented Marie Conte's and Andy Trick's suggestion of adding
 *           a new data type 'BLOCK' to allow an user to specify a block
 *           of binary data the same way ints, doubles, strings, and links
 *           are specified in fields.  (User readable input/output
 *           respresents this data as a hex string.  Note that this data type
 *           may not make sense to programs running on different platforms or
 *           compiled with different compilers!)
 *
\*****************************************************************************/


/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "md_compiler.h"

Mbuf *input_buf = NULL;

char *program_name = NULL;

/* Command line parameters */
char *input_file_name = NULL;
char *output_file_name = NULL;
int using_stdin = 0;
char *output_mode = NULL;
char *input_mode = NULL;
int output_page_width = 80;


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
	   "  -o output_file  outputs to output_file instead of to standard "
	   "output. \n ");
  fprintf (stderr,
	   "  -stdin          reads hmdes from standard input instead of "
	   "input_file.\n");
  fprintf (stderr,
	   "  -read_lmdes     reads as lmdes file instead of hmdes file.\n");
  fprintf (stderr,
	   "  -names          output hmdes entry names instead of lmdes.\n");
  fprintf (stderr,
	   "  -hmdes          output compiled hmdes instead of lmdes.\n");
  fprintf (stderr, "  -width int      sets page width for -hmdes output.\n");
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

	  /* Get -stdin flag */
	  else if (strcmp (*arg_ptr, "-stdin") == 0)
	    {
	      using_stdin = 1;
	    }

	  /* Get -names or -hmdes flag */
	  else
	    if ((strcmp (*arg_ptr, "-names") == 0) ||
		(strcmp (*arg_ptr, "-hmdes") == 0))
	    {
	      /* Make sure some other output mode specified */
	      if (output_mode != NULL)
		print_usage ("output mode specified twice '%s' and '%s'.",
			     output_mode, *arg_ptr);
	      output_mode = strdup (*arg_ptr);
	    }

	  /* Get -read_lmdes flag */
	  else if (strcmp (*arg_ptr, "-read_lmdes") == 0)
	    {
	      /* Make sure some other input mode not specified */
	      if (input_mode != NULL)
		print_usage ("input mode specified twice '%s' and '%s'.",
			     input_mode, *arg_ptr);
	      input_mode = strdup (*arg_ptr);
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

	      output_page_width = (int) strtol (*arg_ptr, &end_ptr, 10);
	      if (*end_ptr != 0)
		{
		  print_usage ("expected '-width int'.\n");
		}
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
	 input_file_name);}

}

int
main (int argc, char **argv)
{
  FILE *in;
  FILE *out;
  MD *md = NULL;
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

  /* Create global memory buffers */
  input_buf = create_Mbuf ();
  if (input_mode == NULL)
    {
      md = build_md (in, input_file_name);
    }
  else if (strcmp (input_mode, "-read_lmdes") == 0)
    {
      md = MD_read_md (in, input_file_name);
    }
  else
    {
      L_punt ("Unknown input mode '%s'.", input_mode);
    }

  /* Print out hmdes in form desired */
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
      /* Print out field declations and entry contents for each section */
      MD_print_md (out, md, 80);
    }
  else if (strcmp (output_mode, "-names") == 0)
    {
      MD_print_md_template (out, md);
    }
  else
    {
      L_punt ("Unknown output mode '%s'.", output_mode);
    }

  /* Free output mode */
  if (output_mode != NULL)
    free (output_mode);
  /* Free global memory buffers */
  free_Mbuf (input_buf);
  /* Free the database */
  MD_delete_md (md);
  /* Free library pools so we can be sure we freed everything */
  if (Mfile_pool != NULL)
    L_free_alloc_pool (Mfile_pool);
  if (Mline_pool != NULL)
    L_free_alloc_pool (Mline_pool);
  if (Mbuf_pool != NULL)
    L_free_alloc_pool (Mbuf_pool);
  if (Mptr_pool != NULL)
    L_free_alloc_pool (Mptr_pool);
  /* Close input and output (may be stdin/stdout) */
  fclose (in);
  fclose (out);
  return (0);
}

/* Create Fptr using Mfile library routines.
 * Adds checkpoint support and, in the future,
 * indication of what "source file" the line
 * being parsed is from 
 */
Fptr *
create_Fptr (Mfile * mfile)
{
  Fptr *fptr;
  if ((fptr = (Fptr *) malloc (sizeof (Fptr))) == NULL)
    L_punt ("Out of memory");
  fptr->mptr = create_Mptr (mfile);
  fptr->checkpoint = copy_Mptr (fptr->mptr);
  fptr->source_name = create_Mbuf ();
  fptr->source_line = 1;
  return (fptr);
}

/* Frees Fptr */
void
free_Fptr (Fptr * fptr)
{
  free_Mptr (fptr->mptr);
  free_Mptr (fptr->checkpoint);
  free_Mbuf (fptr->source_name);
  free (fptr);
}

/*
 * Fptr error handler, enhancement of Merror handler.
 * First argument is a Fptr in use when error occured 
 * (fptr->checkpoint points to start of token that caused error).
 * The second argument is a printf format string, and the rest
 * are the arguments for the format string.
 *
 * variable argument front end to vFerror().
 */
void
Ferror (Fptr * fptr, char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vFerror (fptr, fmt, args);
  va_end (args);
}

void
vFerror (Fptr * fptr, char *fmt, va_list args)
{
  Mptr *checkpoint;
  /* Get checkpoint for ease of use */
  checkpoint = fptr->checkpoint;
  /*
   * Print out text line of text where error occured and put arrow to
   * location.  (pos starts at 0, so add 1 to get column)
   */
  fprintf (stderr, "\n");
  /* If have preprocessor line directives in it, 
   * print out where error came from in original file.
   */
  if (len_of_Mbuf (fptr->source_name) != 0)
    {
      fprintf (stderr,
	       "Error during parsing of md input file (line %i of %s):\n",
	       fptr->source_line, get_Mbuf_buf (fptr->source_name));
    }

  /* If no preprocessor line directives and not at EOF */
  else if (checkpoint->mline != NULL)
    {
      fprintf (stderr,
	       "Error during parsing of md input file (line %i char %i "
	       "of %s):\n",
	       checkpoint->mline->line_no,
	       checkpoint->pos + 1, checkpoint->mfile->name);}
  /* If no preprocessor line directives and at EOF */
  else
    {
      fprintf (stderr,
	       "Error during parsing of md input file (%s at EOF):\n",
	       checkpoint->mfile->name);}

  /*Print error message */
  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");
  fprintf (stderr, "\n");
  /* Do not print out file text if at EOF */
  if (checkpoint->mline != NULL)
    {
      /* Print out file text where error occurred */
      /* If have preprocessor line info,
       * print out where it occurred in preprocessed text
       */
      if (len_of_Mbuf (fptr->source_name) != 0)
	{
	  fprintf (stderr,
		   "Text where error occurred (preprocessed: line %i char %i "
		   "of %s):\n",
		   checkpoint->mline->line_no,
		   checkpoint->pos + 1, checkpoint->mfile->name);}
      /* If don't have preprocessor line info */
      else
	{
	  fprintf (stderr, "File text where error occurred:\n");
	}
      print_buf_with_arrow (stderr, checkpoint->mline->buf, checkpoint->pos);
    }

  fprintf (stderr, "Fatal error, cannot continue.\n");
  fprintf (stderr, "\n");
  exit (-1);
}


/* Peeks at next character in file */
int
Fpeekc (Fptr * fptr)
{
  int ch;
  ch = Mpeekc (fptr->mptr);
  return (ch);
}

/* Gets the next character in file */
int
Fgetc (Fptr * fptr)
{
  int ch;
  ch = Mgetc (fptr->mptr);
  return (ch);
}

/* Reads line directive in from file and updates fptr state */
void
Fget_line_directive (Fptr * fptr)
{
  int ch;
  int set_line, set_file;
  /* Reset flags to indicate what we read in */
  set_line = 0;
  set_file = 0;
  /* Move checkpoint to start of this processor line statement */
  move_Mptr (fptr->checkpoint, fptr->mptr);
  /* Sanity check, should be called with '$' in current possition */
  if (Fpeekc (fptr) != '$')
    Ferror (fptr, "Fget_line_directive() called at improper time.");
  /* Get '$' at start of line */
  Fgetc (fptr);
  /* Make sure '$' followed by 'line' */
  if ((Fgetc (fptr) != 'l') ||
      (Fgetc (fptr) != 'i') || (Fgetc (fptr) != 'n') || (Fgetc (fptr) != 'e'))
    {
      Ferror (fptr,
	      "Unknown preprocessor directive.\nExpecting preprocessor line "
	      "directive '$line line_no file_name'.");
    }


  /* In preprocessor statement.
   * Expect one of the following:
   * 1) $line line_no
   * 2) $line "file_name"
   * 3) $line line_no "file_name"
   */

  /* Skip spaces line number (' ' only ) */
  while (Fpeekc (fptr) == ' ')
    Fgetc (fptr);
  /* Get line number (if it is there) */
  ch = Fpeekc (fptr);
  if (isdigit (ch))
    {
      /* Build line number, one digit at a time */
      fptr->source_line = 0;
      /* Flag that we are setting the line number */
      set_line = 1;
      while (isdigit (ch))
	{
	  /* Get the digit */
	  ch = Fgetc (fptr);
	  /* Multiply existing conversion by 10 and add digit */
	  fptr->source_line *= 10;
	  fptr->source_line += (ch - '0');
	  /* Peek at next character */
	  ch = Fpeekc (fptr);
	}
    }

  /* Skip spaces before file name (' ' only ) */
  while (Fpeekc (fptr) == ' ')
    Fgetc (fptr);
  /* Get file name (if it is there) */
  ch = Fpeekc (fptr);
  if (ch == '"')
    {
      /* Move checkpoint to start quote */
      move_Mptr (fptr->checkpoint, fptr->mptr);
      /* Get start quote */
      Fgetc (fptr);
      /* Flag that we are setting the file name */
      set_file = 1;
      /* Clear source name mbuf */
      clear_Mbuf (fptr->source_name);
      /* Read in name into source_name  until ending quote */
      while ((ch = Fgetc (fptr)) != '"')
	{
	  /* If hit end of line or EOF, error */
	  if ((ch == '\n') || (ch == EOF))
	    {
	      Ferror (fptr,
		      "Matching end quote expected in preprocessor line "
		      "directive.");
	    }

	  /* Add character to source name */
	  addc_to_Mbuf (fptr->source_name, ch);
	}
    }

  /* Skip spaces before newline (' ' only ) */
  while (Fpeekc (fptr) == ' ')
    Fgetc (fptr);
  /* Expected newline now */
  ch = Fpeekc (fptr);
  if (((ch != '\n') && (ch != EOF)) || ((!set_line) && (!set_file)))
    {
      /* Move checkpoint to site of error */
      move_Mptr (fptr->checkpoint, fptr->mptr);
      Ferror (fptr,
	      "Poorly formed preprocessor line directive.\n\nExpecting one of "
	      "the following:\n 1) $line line_no\n 2) $line \"file_name\"\n "
	      "3) or $line line_no \"file_name\".\n\nDirective must be "
	      "terminated with a newline.");
    }
  /* Get newline */
  Fgetc (fptr);
}


/* Skips whitespace between tokens and updates the checkpoint used for
 * error messages.
 * Use before reading in EVERY token.
 * In future, will help keep trace of $line commands (source line info).
 */
void
Fadvance_to_next_token (Fptr * fptr)
{
  int ch;
  while (1)
    {
      ch = Fpeekc (fptr);
      /* Detect line directive ('$' at begining of line ) */
      if ((ch == '$') && (Mptr_pos (fptr->mptr) == 0))
	{
	  /* Read in line directive and start whitespace search again */
	  Fget_line_directive (fptr);
	  continue;
	}

      if (!isspace (ch))
	break;
      /* Throw away space */
      ch = Fgetc (fptr);
      /* After newlines, advance the source line count */
      if (ch == '\n')
	{
	  fptr->source_line++;
	}
    }

  /* Move checkpoint to start of this token */
  move_Mptr (fptr->checkpoint, fptr->mptr);
}

/* Returns 1 if character is a single character token (I.e., a bracket) */
int
Fis_token_char (int ch)
{
  switch (ch)
    {
    case '{':
    case '}':
    case '[':
    case ']':
    case '(':
    case ')':
    case '<':
    case '>':
    case ';':
    case '*':
    case '!':
    case '|':			/* Special case, will grab '||' if there */
      return (1);
    }
  return (0);
}


/* Gets the next token from the fptr and puts it in the mbuf.
 * On EOF, the empty string will be put in mbuf.
 * Returns token length (0 on EOF).
 */
int
Fget_token (Fptr * fptr, Mbuf * mbuf)
{
  int length;
  int ch;
  int quoted_string;
  int quote_ch = 0;
  /* Skip whitespace and set checkpoint to start of new token */
  Fadvance_to_next_token (fptr);
  /* Clear the mbuf */
  clear_Mbuf (mbuf);
  /* Is first character a token by itself? EOF? */
  if ((ch = Fpeekc (fptr)) == EOF)
    return (0);
  if (Fis_token_char (ch))
    {
      ch = Fgetc (fptr);
      addc_to_Mbuf (mbuf, ch);
      /* Special case to generate '||' token */
      if (ch == '|')
	{
	  if (Fpeekc (fptr) == '|')
	    {
	      ch = Fgetc (fptr);
	      addc_to_Mbuf (mbuf, ch);
	      return (2);
	    }
	}

      return (1);
    }

  /* Assume we don't have quoted string */
  quoted_string = 0;
  length = 0;
  /* Handle start of quoted string */
  if ((ch == '"') || (ch == '\''))
    {
      /* Get the end quote character we are looking for */
      quote_ch = ch;
      /* Get the quote and put it in Mbuf */
      ch = Fgetc (fptr);
      addc_to_Mbuf (mbuf, ch);
      /* Mark as quoted string */
      quoted_string = 1;
      length++;
    }


  /* Otherwise, get token until hit whitespace or single char token */
  while ((ch = Fpeekc (fptr)) != EOF)
    {
      /* For quoted strings, don't end token until end quote */
      if (quoted_string)
	{
	  /* If hit end quote, finish string and return */
	  if (ch == quote_ch)
	    {
	      ch = Fgetc (fptr);
	      addc_to_Mbuf (mbuf, ch);
	      length++;
	      break;
	    }

	  /* Error if hit end of line before end quote */
	  if (ch == '\n')
	    {
	      Ferror (fptr,
		      "End quote not found.  Expected before the end of this "
		      "line.");
	    }
	}

      /* For unquoted strings, stop at whitespace or single char token.
       * Quotes are not allowed in the middle of the unquoted string
       */
      else
	{
	  if (isspace (ch) || Fis_token_char (ch))
	    break;
	  if ((ch == '\'') || (ch == '"'))
	    {
	      /* Move error pointer to current location */
	      Fadvance_to_next_token (fptr);
	      Ferror (fptr,
		      "Unexpected quote.  Expecting one of the following:\n "
		      "1) Whitespace or some sort of delimiter before the "
		      "quote\n 2) or a backslash before the quote.");
	    }
	}

      /* Get character and add to mbuf */
      ch = Fgetc (fptr);
      addc_to_Mbuf (mbuf, ch);
      /* Increment length */
      length++;
      /* Handle backslashed character */
      if (ch == '\\')
	{
	  ch = Fgetc (fptr);
	  /* May not be newline or EOF after backslash */
	  if ((ch == '\n') || (ch == EOF))
	    {
	      Ferror (fptr,
		      "A backslash may not appear at the end of a line.");}
	  addc_to_Mbuf (mbuf, ch);
	  length++;
	}
    }

  /* Return the length of multichar token */
  return (length);
}

/* Returns 1 if a legal identifier (C identifier), 0 otherwise */
int
is_md_ident (char *ident)
{
  char *ptr;
  /* Must start with letter of '_' (may not be empty string) */
  if ((ident[0] != '_') && !isalpha (ident[0]))
    return (0);
  /* Rest of string must be alphnumeric or '_' */
  for (ptr = &ident[1]; *ptr != 0; ptr++)
    {
      if ((*ptr != '_') && !isalnum (*ptr))
	return (0);
    }

  /* Return legal */
  return (1);
}

MD *
build_md (FILE * in, char *md_name)
{
  MD *md;
  MD_Section *section;
  Mfile *input_mfile;
  Mbuf *mbuf;
  Fptr *fptr;
  char *section_name;
  int new_section = 0;
  int error_count;
  /* Load file into Mfile so can create Fptr */
  input_mfile = create_Mfile (in, md_name, "MD building");
  /* Create Fptr so we can parse file */
  fptr = create_Fptr (input_mfile);
  /* Create Mbuf to put tokens into */
  mbuf = create_Mbuf ();
  /* Create a md with the given input file name (start with 0 sections). */
  md = MD_new_md (md_name, 0);
  /* Parse all the sections in the file */
  while (Fget_token (fptr, mbuf))
    {
      /* 
       * Get pointer to new or existing section
       */

      /* Test for creation of a new section */
      if (match_Mbuf (mbuf, "CREATE"))
	{
	  /* "CREATE" must be followed by "SECTION" */
	  Fget_token (fptr, mbuf);
	  if (!match_Mbuf (mbuf, "SECTION"))
	    Ferror (fptr, "Expecting 'SECTION' after 'CREATE'.");
	  /* Mark that this is a new section */
	  new_section = 1;
	}

      /* Test for referencing of existing section */
      else if (match_Mbuf (mbuf, "SECTION"))
	{
	  /* Mark that this is an existing section */
	  new_section = 0;
	}

      else
	{
	  Ferror (fptr, "Expecting 'SECTION' or 'CREATE SECTION' keyword.");
	}

      /* Get section name */
      if (!Fget_token (fptr, mbuf))
	Ferror (fptr, "Expecting a section name after 'SECTION'.");
      /* Create new section or get existing sectino */
      section_name = copy_Mbuf_buf (mbuf);
      /* Make sure legal section name */
      if (!is_md_ident (section_name))
	{
	  Ferror (fptr, "Invalid section name.  Must be valid C identifier.");
	}

      if (new_section)
	{
	  /* Make sure this section does not already exist */
	  if (MD_find_section (md, section_name) != NULL)
	    Ferror (fptr, "Section '%s' already exists!", section_name);
	  /* Create a new sections, start with no assumption about size */
	  section = MD_new_section (md, section_name, 0, 0);
	}
      else
	{
	  /* Find the the section */
	  if ((section = MD_find_section (md, section_name)) == NULL)
	    {
	      Ferror (fptr,
		      "Section '%s' undefined.  Fix spelling or use 'CREATE "
		      "SECTION'.",
		      section_name);}
	}

      /*
       * Get field declarations until hit section start marker.
       */
      while (1)
	{
	  Fget_token (fptr, mbuf);
	  /* Hit section start marker, exit loop */
	  if (match_Mbuf (mbuf, SECTION_START))
	    break;
	  /* Create field using declaration.
	   * Field_Decl type passed in mbuf (contents destroyed after call)
	   * and fptr advances to after declaration.
	   */
	  build_field_decl (md, section, fptr, mbuf);
	  /*
	     create_Field_Decl (md, section, fptr, mbuf);
	   */
	}


      /*
       * Get section entries until hit section end marker.
       */
      while (1)
	{
	  Fget_token (fptr, mbuf);
	  /* Get section end marker */
	  if (match_Mbuf (mbuf, SECTION_END))
	    break;
	  /* Create section entry.
	   * Entry name passed in mbuf (contents destroyed after call) and
	   * ftpr advances to after entry.
	   */
	  build_entry (section, fptr, mbuf);
	  /*
	     create_Entry (md, section, fptr, mbuf);
	   */
	}

      /* Finished with reading in section, free temp variables */
      free (section_name);
    }

  /* Check the md for required links (there should not be other errors). */
  error_count = MD_check_md (stderr, md);
  if (error_count)
    {
      fprintf (stderr, "\nError: All required fields must be specified!\n\n");
      L_punt ("Cannot continue: %i unspecified fields in '%s'.",
	      error_count, md->name);}

  /* Check the md for ambigous links */
  error_count = check_md_for_ambiguous_links (stderr, md);
  if (error_count)
    {
      fprintf (stderr, "\n");
      L_punt ("Cannot continue: %i ambiguous links detected in '%s'.",
	      error_count, md->name);
    }

  /* Free Fptr, Mfile, and Mbuf */
  free_Fptr (fptr);
  free_Mfile (input_mfile);
  free_Mbuf (mbuf);
  return (md);
}

/* Create a field in the section using the field declaration starting
 * in the mbuf and pointed to by fptr.
 *
 * Destroys contents of mbuf and advances fptr to after declaration.
 */
void
build_field_decl (MD * md, MD_Section * section, Fptr * fptr, Mbuf * mbuf)
{
  MD_Field_Decl *field_decl;
  MD_FIELD_TYPE field_type = 0;
  MD_Section **target_array, *target, **new_target_array;
  char *field_type_name, *field_name, *target_name;
  int target_count, target_array_size, new_target_array_size, i;
  /* Optional field declaration */
  if (match_Mbuf (mbuf, "OPTIONAL"))
    field_type = MD_OPTIONAL_FIELD;
  /* Required field declaration */
  else if (match_Mbuf (mbuf, "REQUIRED"))
    field_type = MD_REQUIRED_FIELD;
#if 0
  /* Ignored field declaration */
  else if (match_Mbuf (mbuf, "IGNORED"))
    field_type = IGNORED_FIELD;
#endif
  /* Otherwise, parse error */
  else
    {
      Ferror (fptr,
	      "Expecting start of section delimiter '%s' or\ndeclaration of "
	      "field type 'OPTIONAL', 'REQUIRED', or 'IGNORED'.",
	      SECTION_START);}

  /* Get type name for error messages */
  field_type_name = copy_Mbuf_buf (mbuf);
  /* Get field name */
  if (!Fget_token (fptr, mbuf))
    Ferror (fptr, "Expecting a field name after '%s'.", field_type_name);
  field_name = copy_Mbuf_buf (mbuf);
  /* Make sure legal field name */
  if (!is_md_ident (field_name))
    {
      Ferror (fptr,
	      "Invalid field name in declaration.  Must be valid C "
	      "identifier.");
    }

  /* For now, do not allow field names to be redeclared */
  if ((field_decl = MD_find_field_decl (section, field_name)) != NULL)
    {
      Ferror (fptr, "Redeclaration of a field is not allowed.");
    }

  /* Declare a new field */
  field_decl = MD_new_field_decl (section, field_name, field_type);
  /* Get field_decl start delimiter */
  Fget_token (fptr, mbuf);
  if (!match_Mbuf (mbuf, FIELD_START))
    Ferror (fptr,
	    "Expecting '%s' after field name '%s' in declaration.",
	    FIELD_START, field_name);
  /* Get the element requirements for the field until hit end of 
   * field marker 
   */
  while (1)
    {
      Fget_token (fptr, mbuf);
      /* Detect end of field declaration */
      if (match_Mbuf (mbuf, FIELD_END))
	break;
      /* Detect Kleene star on last element requirement */
      if (match_Mbuf (mbuf, KLEENE_STAR))
	{
	  /* May not have a Kleen star by itself */
	  if (field_decl->max_require_index == -1)
	    {
	      Ferror (fptr,
		      "Expecting element requirement before kleene star '%s'.",
		      KLEENE_STAR);}

	  MD_kleene_star_requirement (field_decl,
				      field_decl->max_require_index);
	  /* Make sure end of field declaration is next */
	  Fget_token (fptr, mbuf);
	  /* If end of field declaration, exit loop, otherwise error */
	  if (match_Mbuf (mbuf, FIELD_END))
	    break;
	  else
	    {
	      Ferror (fptr,
		      "A Kleene star '%s' is allowed only on the last element "
		      "requirement,\nexpecting '%s%s'.",
		      KLEENE_STAR, FIELD_END, TERMINATOR);
	    }

	  /* Break out of loop, hit end of declaration */
	  break;
	}

      /* Get the type of element requirment we have. */
      if (match_Mbuf (mbuf, "INT"))
	{
	  MD_require_int (field_decl, field_decl->max_require_index + 1);
	}

      else if (match_Mbuf (mbuf, "DOUBLE"))
	{
	  MD_require_double (field_decl, field_decl->max_require_index + 1);
	}

      else if (match_Mbuf (mbuf, "STRING"))
	{
	  MD_require_string (field_decl, field_decl->max_require_index + 1);
	}

      else if (match_Mbuf (mbuf, "BLOCK"))
	{
	  MD_require_block (field_decl, field_decl->max_require_index + 1);
	}

      else if (match_Mbuf (mbuf, "LINK"))
	{
	  /* Get the link start delimiter around section name */
	  Fget_token (fptr, mbuf);
	  if (!match_Mbuf (mbuf, LINK_START))
	    {
	      Ferror (fptr,
		      "Expecting 'LINK%ssection_name%s'.",
		      LINK_START, LINK_END);}

	  /* Allocate a target array to hold pointers to the targets */
	  target_array_size = 1;
	  target_array = (MD_Section **) malloc (sizeof (MD_Section *) *
						 target_array_size);
	  /* Initialize target count */
	  target_count = 0;
	  /* Get all the targets that we may link to */
	  while (1)
	    {
	      /* Get the section name */
	      Fget_token (fptr, mbuf);
	      target_name = copy_Mbuf_buf (mbuf);
	      if ((target = MD_find_section (md, target_name)) == NULL)
		{
		  Ferror (fptr,
			  "Cannot create LINK, section '%s' not defined.",
			  target_name);}

	      /* Make sure this target not already in LINK */
	      for (i = 0; i < target_count; i++)
		{
		  if (target_array[i] == target)
		    {
		      Ferror (fptr, "Section '%s' already in LINK.",
			      target_name);}
		}

	      /* Add target to target array */
	      /* Determine if target array need increasing */
	      if (target_count >= target_array_size)
		{
		  /* Double target array size */
		  new_target_array_size = target_array_size << 1;
		  /* Malloc new array */
		  new_target_array =
		    (MD_Section **) malloc (sizeof (MD_Section **) *
					    new_target_array_size);
		  /* Copy over pointers from old array */
		  for (i = 0; i < target_count; i++)
		    new_target_array[i] = target_array[i];
		  /* Free old array and put new one in it's place */
		  free (target_array);
		  target_array = new_target_array;
		  target_array_size = new_target_array_size;
		}

	      /* Add to target array */
	      target_array[target_count] = target;
	      /* Increment target count */
	      target_count++;
	      /* Get the link end delimiter around section name or
	       * the OR_MARKER for another section name 
	       */
	      Fget_token (fptr, mbuf);
	      /* Free malloced target_name */
	      free (target_name);
	      target_name = NULL;
	      /* Get end of declaration */
	      if (match_Mbuf (mbuf, LINK_END))
		{
		  break;
		}
	      /* Or get OR marker (get next target */
	      else if (match_Mbuf (mbuf, OR_MARKER))
		{
		  continue;
		}
	      else
		{
		  Ferror (fptr,
			  "Expecting one of the following:\n 1) End of LINK "
			  "delimiter '%s'\n 2) or '%' followed by another "
			  "section name.", LINK_END, OR_MARKER);
		}
	    }

	  /* Create link requirement (Use multi target form) */
	  MD_require_multi_target_link (field_decl,
					field_decl->max_require_index + 1,
					target_count, target_array);
	  /* Free target array */
	  free (target_array);
	}

      else
	{
	  Ferror (fptr,
		  "Expecting one of the following:\n 1) a element requirement "
		  "of 'INT', 'DOUBLE', 'STRING',\n    'BLOCK', or "
		  "'LINK%ssection_name%s'\n 2) a Kleene star '%s' "
		  "(allowed only on the last element requirement)\n 3) or a "
		  "end of declaration delimiter '%s%s' for field '%s'.",
		  LINK_START, LINK_END,
		  KLEENE_STAR, FIELD_END, TERMINATOR, field_name);}
    }

  /* Get terminator at end of field declaration */
  Fget_token (fptr, mbuf);
  if (!match_Mbuf (mbuf, TERMINATOR))
    {
      Ferror (fptr,
	      "Expecting '%s%s' at end of declaration of field '%s'.",
	      FIELD_END, TERMINATOR, field_name);
    }

  /* Free field type name and field name, for error messages only. */
  free (field_name);
  free (field_type_name);
}

/* Returns 1 if valid md int.  Return int in value */
int
build_int (char *string, int *value)
{
  char *end_ptr, *ptr;
  /* Require the int to have at least 1 digit in it */
  for (ptr = string; *ptr != 0; ptr++)
    {
      if (isdigit (*ptr))
	break;
    }
  if (*ptr == 0)
    return (0);
  /* Convert to long, make sure
   * conversion goes to end of string.
   */
  *value = (int) strtol (string, &end_ptr, 0);
  if (*end_ptr != 0)
    return (0);
  /* If got here, must be legel int */
  return (1);
}

/* Returns 1 if valid double value.  Returns double in value */
int
build_double (char *string, double *value)
{
  char *end_ptr, *ptr;
  /* Require the float to have at least 1 digit in it */
  for (ptr = string; *ptr != 0; ptr++)
    {
      if (isdigit (*ptr))
	break;
    }
  if (*ptr == 0)
    return (0);
  /* Convert to double, make sure
   * conversion goes to end of string.
   */
  *value = strtod (string, &end_ptr);
  if (*end_ptr != 0)
    return (0);
  /* If got here, must be legel double */
  return (1);
}

/* Strips first and last character from string (modifies string) */
void
strip_first_and_last_char (char *string)
{
  char *ptr;
  /* If empty string, do nothing */
  if (string[0] == 0)
    return;
  /* If 1 character long string, make empty string */
  if (string[1] == 0)
    {
      string[0] = 0;
      return;
    }

  /* Shift string one character to left, 
   * stop before copying last character
   */
  for (ptr = string; *(ptr + 2) != 0; ptr++)
    *ptr = *(ptr + 1);
  /* Terminate string */
  *ptr = 0;
}

/*
 * Strips backslashes from brackslashed characters in string (modifies string)
 */
void
strip_backslashes (char *string)
{
  char *src_ptr, *dest_ptr;
  int ch;
  /* Get pointers to start of string */
  dest_ptr = string;
  src_ptr = string;
  /* Copy string to itself, removing backslashes from backslashed
   * characters.
   */
  while (1)
    {
      /* Get next character from string */
      ch = *src_ptr;
      /* If backslash, skip and get next character */
      if (ch == '\\')
	{
	  src_ptr++;
	  ch = *src_ptr;
	}

      /* Write character back to string */
      *dest_ptr = ch;
      /* Stop after copying terminator */
      if (ch == 0)
	break;
      /* Goto next character for processing */
      dest_ptr++;
      src_ptr++;
    }
}


void
build_element (MD_Field * field, MD_Element_Req * element_req,
	       int element_index, Fptr * fptr, Mbuf * mbuf)
{
  MD_Section **target_array;
  MD_Entry *entry, *target_entry;
  int int_value;
  double double_value;
  int target_count, i;
  char *element_string;
  unsigned int block_size, string_len;
  unsigned char *block_data, block_byte = '\0', high_hex_ch, low_hex_ch;
  /* Get the element's value string */
  element_string = copy_Mbuf_buf (mbuf);
  /* Check datum based on type */
  switch (element_req->type)
    {
    case MD_INT:
      if (!build_int (element_string, &int_value))
	{
	  Ferror (fptr,
		  "Invalid element '%s'.\nElement has an '%s' requirement.",
		  element_string, element_req->desc);
	}
      MD_set_int (field, element_index, int_value);
      break;
    case MD_DOUBLE:
      if (!build_double (element_string, &double_value))
	{
	  Ferror (fptr,
		  "Invalid element '%s'.\nElement has a '%s' requirement.",
		  element_string, element_req->desc);
	}
      MD_set_double (field, element_index, double_value);
      break;
      /* Anything can be a string */
    case MD_STRING:
      /* Strip quotes from string if present */
      if ((element_string[0] == '"') || (element_string[0] == '\''))
	strip_first_and_last_char (element_string);
      /* Unquoted string may not contain token characters (will
       * be put in first character of string if token character)
       */
      else if (Fis_token_char (element_string[0]))
	{
	  Ferror (fptr,
		  "Unquoted strings may not contain a '%c', put quotes "
		  "around the string.",
		  element_string[0]);
	}

      /* Remove backslashes from backslashed characters */
      strip_backslashes (element_string);
      MD_set_string (field, element_index, element_string);
      break;
      /* Binary data blocks must be an even number of hex characters.
       * Quotes are optional.
       */
    case MD_BLOCK:
      /* Strip quotes from string if present */
      if ((element_string[0] == '"') || (element_string[0] == '\''))
	strip_first_and_last_char (element_string);
      /* Get length of string */
      string_len = strlen (element_string);
      /* String length must be even */
      if ((string_len & 1) == 1)
	{
	  Ferror (fptr,
		  "BLOCK hex string length (%i) must be even!", string_len);}

      /* Set the block size to half of the string length */
      block_size = string_len >> 1;
      /* If block size is zero, don't malloc anything.  Everything
       * should work with NULL ptr and zero size.
       */
      if (block_size != 0)
	{
	  /* Allocate binary block array (byte array) */
	  if ((block_data = (unsigned char *) malloc (block_size)) == NULL)
	    {
	      L_punt ("Out of memory allocating BLOCK data of %i bytes.",
		      string_len >> 1);
	    }
	}
      else
	{
	  block_data = NULL;
	}

      /* Convert hex string into binary block data */
      for (i = 0; i < block_size; i++)
	{
	  high_hex_ch = element_string[(i << 1)];
	  if ((high_hex_ch >= '0') && (high_hex_ch <= '9'))
	    block_byte = (high_hex_ch - '0') << 4;
	  else if ((high_hex_ch >= 'a') && (high_hex_ch <= 'f'))
	    block_byte = (high_hex_ch - 'a' + 10) << 4;
	  else if ((high_hex_ch >= 'A') && (high_hex_ch <= 'F'))
	    block_byte = (high_hex_ch - 'A' + 10) << 4;
	  else
	    {
	      Ferror (fptr,
		      "Invalid hex character '%c' at position %i"
		      "in BLOCK hex string.", high_hex_ch, (i << 1) + 1);
	    }

	  low_hex_ch = element_string[(i << 1) + 1];
	  if ((low_hex_ch >= '0') && (low_hex_ch <= '9'))
	    block_byte |= (low_hex_ch - '0');
	  else if ((low_hex_ch >= 'a') && (low_hex_ch <= 'f'))
	    block_byte |= (low_hex_ch - 'a' + 10);
	  else if ((low_hex_ch >= 'A') && (low_hex_ch <= 'F'))
	    block_byte |= (low_hex_ch - 'A' + 10);
	  else
	    {
	      Ferror (fptr,
		      "Invalid hex character '%c' at position %i"
		      "in BLOCK hex string.", low_hex_ch, (i << 1) + 2);
	    }

	  /* Write byte to block data array */
	  block_data[i] = block_byte;
	}

      /* Set element to block data */
      MD_set_block (field, element_index, block_size, block_data);
      /* Free block_data buffer, if necessary */
      if (block_data != NULL)
	free (block_data);
      break;
    case MD_LINK:
      /* Strip quotes from entry name (if present) */
      if ((element_string[0] == '"') || (element_string[0] == '\''))
	strip_first_and_last_char (element_string);
      /* Remove backslashes from backslashed characters */
      strip_backslashes (element_string);
      /* Search target array for entry name.  It must be in exactly
       * one section.
       */
      target_array = element_req->link;
      target_count = element_req->link_array_size;
      entry = NULL;
      target_entry = NULL;
      for (i = 0; i < target_count; i++)
	{
	  /* See if entry name appears in this section */
	  entry = MD_find_entry (target_array[i], element_string);
	  if (entry != NULL)
	    {
	      /* Error if multiple matches found */
	      if (target_entry != NULL)
		{
		  Ferror (fptr,
			  "%s->%s->%s[%i]:\nAmbiguous entry name '%s' for "
			  "element with requirement\n'%s'.\nEntry name found "
			  "in section '%s' and '%s'.",
			  field->entry->section->name, field->entry->name,
			  field->decl->name, element_index,
			  element_string, element_req->desc,
			  entry->section->name, target_entry->section->name);}
	      target_entry = entry;
	    }
	}

      /* Error if no matches found */
      if (target_entry == NULL)
	{
	  Ferror (fptr,
		  "%s->%s->%s[%i]:\n  Entry name '%s' is not found in any "
		  "section specified by the\n  element requirement '%s'.",
		  field->entry->section->name, field->entry->name,
		  field->decl->name, element_index, element_string,
		  element_req->desc);}

      /* Set link to entry specified */
      MD_set_link (field, element_index, target_entry);
      break;
    default:
      Ferror (fptr, "Element requirement type %i not defined.",
	      element_req->type);}

  /* Free element value */
  free (element_string);
}


/* Create a Entry in the section using the Entry description starting
 * in the mbuf and pointed to by fptr.
 *
 * Destroys contents of mbuf and advances fptr to after entry.
 */
void
build_entry (MD_Section * section, Fptr * fptr, Mbuf * mbuf)
{
  MD_Entry *entry;
  char *entry_name, *stripped_entry_name = NULL;
  MD_Field_Decl *field_decl;
  MD_Field *field;
  MD_Element_Req **require_array, *element_req, *kleene_starred_req;
  char *field_name;
  int element_index, max_require_index;
  /* Get entry name */
  entry_name = copy_Mbuf_buf (mbuf);
  /* Make sure legal entry name (a valid C identifier or a quoted string).
   * If it is, get "stripped" name (without quotes)
   */
  if (is_md_ident (entry_name))
    {
      stripped_entry_name = strdup (entry_name);
    }
  else if ((entry_name[0] == '"') || (entry_name[0] == '\''))
    {
      stripped_entry_name = strdup (entry_name);
      strip_first_and_last_char (stripped_entry_name);
      /* Remove backslashes from backslashed characters */
      strip_backslashes (stripped_entry_name);
    }
  else
    {
      Ferror (fptr,
	      "Expecting one of the following:\n 1) a entry name (must be a "
	      "valid C identifier or a quoted string)\n 2) or a end of "
	      "section delimiter '%s' for section '%s'.",
	      SECTION_END, section->name);}

  /* Use existing entry if exists, otherwise create new entry
   * Use stripped entry name for search and table index.
   * Put actual name (with quotes, if any) in entry
   */
  if ((entry = MD_find_entry (section, stripped_entry_name)) == NULL)
    {
      entry = MD_new_entry (section, stripped_entry_name);
    }

  /* Get the ENTRY_START marker */
  Fget_token (fptr, mbuf);
  if (!match_Mbuf (mbuf, ENTRY_START))
    {
      Ferror (fptr,
	      "Expecting start of entry marker '%s' after '%s'.",
	      ENTRY_START, entry_name);}

  /* Get the entries fields until end of entry encountered */
  while (1)
    {
      Fget_token (fptr, mbuf);
      /* If end of entry marker, exit loop */
      if (match_Mbuf (mbuf, ENTRY_END))
	break;
      /* Assume it is a valid field name */
      field_name = copy_Mbuf_buf (mbuf);
      /* Get field if defined in section, error if not defined */
      if ((field_decl = MD_find_field_decl (section, field_name)) == NULL)
	{
	  /* Assume mispelling if legal identifier */
	  if (is_md_ident (field_name))
	    {
	      Ferror (fptr,
		      "Field '%s' undefined in section '%s'.\nFix spelling or "
		      "declare field at the top of this section.",
		      field_name, section->name);}
	  /* Otherwise, syntax error */
	  else
	    {
	      Ferror (fptr,
		      "Expecting one of the following:\n 1) a field name that "
		      "has been declared in section '%s'\n 2) or a end of "
		      "entry marker  '%s%s' for entry '%s'.",
		      section->name, ENTRY_END, TERMINATOR, entry_name);
	    }
	}

      /* Set Field to NULL for debugging, get or create a field
       * depending on the next token 
       */
      field = NULL;
      /* Get replace field, concat field, or start field marker */
      Fget_token (fptr, mbuf);
      /* Check for concat field marker */
      if (match_Mbuf (mbuf, CONCAT_FIELD))
	{
	  /* Error if field is not kleene_starred */
	  if (field_decl->kleene_starred_req == NULL)
	    {
	      Ferror (fptr,
		      "Cannot concatenate to contents of field '%s',\n"
		      "last datum in declaration of field '%s' not "
		      "Kleene starred '%s'.",
		      field_name, field_name, KLEENE_STAR);}

	  /* Find the field that we are concatenating, error if contents
	   * have not already been set unless first datum is
	   * kleene_starred
	   */
	  if ((field = MD_find_field (entry, field_decl)) == NULL)
	    {
	      /* Is first datum is Kleene starred (if there is only
	       * one datum defined)?  If not, error.
	       */
	      if (field_decl->max_require_index >= 1)
		{
		  Ferror (fptr,
			  "Cannot concatenate to contents of field '%s'.\n"
			  "Expect one of the following:\n 1) The contents of "
			  "field '%s' to be previously set for entry '%s'\n "
			  "2) or the first and ONLY datum declared for this "
			  "field to be Kleene starred.",
			  field_name, field_name, entry_name);
		}

	      else
		{
		  /* Create new field to concat to */
		  field = MD_new_field (entry, field_decl, 0);
		}
	    }

	  /* Get what should be the start field marker into mbuf */
	  Fget_token (fptr, mbuf);
	}

      /* Replace field contents (if field already defined) */
      else if (match_Mbuf (mbuf, REPLACE_FIELD))
	{
	  if ((field = MD_find_field (entry, field_decl)) != NULL)
	    {
	      MD_delete_field (field);
	    }

	  /* Create field in place of field we just destroyed */
	  field = MD_new_field (entry, field_decl, 0);
	  /* Get what should be the start field marker into mbuf */
	  Fget_token (fptr, mbuf);
	}
      else if (match_Mbuf (mbuf, FIELD_START))
	{
	  /* Make sure the contents for this field has not already been set */
	  if ((field = MD_find_field (entry, field_decl)) != NULL)
	    {
	      Ferror (fptr,
		      "Contents of field '%s' already set.\nUse '%s%s' to "
		      "concatenate to the end of the existing contents\nor "
		      "use '%s%s' to replace field's contents.",
		      field_name,
		      CONCAT_FIELD, FIELD_START, REPLACE_FIELD, FIELD_START);}
	  /* Create field */
	  field = MD_new_field (entry, field_decl, 0);
	}

      /* Must be a start field marker by now */
      if (!match_Mbuf (mbuf, FIELD_START))
	{
	  Ferror (fptr,
		  "Expecting one of the following after the field name '%s':\n "
		  "1) a concat field contents marker '%s%s'\n 2) a replace "
		  "field contents marker '%s%s'\n 3) or a start field contents "
		  "marker '%s'.",
		  field_name,
		  CONCAT_FIELD, FIELD_START,
		  REPLACE_FIELD, FIELD_START, FIELD_START);}

      /* Add elements to end of field.  This will be the beginning of
       * the field except for CONCAT field operations.
       */
      element_index = field->max_element_index + 1;
      max_require_index = field_decl->max_require_index;
      require_array = field_decl->require;
      kleene_starred_req = field_decl->kleene_starred_req;
      while (1)
	{
	  /* If element index falls with boundaries of require array
	   * get that requirement
	   */
	  if (element_index <= max_require_index)
	    element_req = require_array[element_index];
	  /* Otherwise, get kleene star requirement (will be NULL if
	   * there is no kleene star requirement.)
	   */
	  else
	    element_req = kleene_starred_req;
	  /* Get next datum, punt on EOF */
	  if (!Fget_token (fptr, mbuf))
	    {
	      Ferror (fptr,
		      "%s->%s->%s:\n  Unexpected EOF encountered while "
		      "reading element %i.",
		      entry->section->name, entry->name,
		      field->decl->name, element_index);}

	  /* Stop now if no more element expected */
	  if (element_req == NULL)
	    break;
	  /* No datum can be end of field (unless quoted) */
	  if (match_Mbuf (mbuf, FIELD_END))
	    {
	      if (element_req->kleene_starred)
		break;
	      else
		{
		  Ferror (fptr,
			  "%s->%s->%s[%i]:\n  Element of type '%s' missing.",
			  entry->section->name, entry->name,
			  field->decl->name, element_index,
			  element_req->desc);}
	    }

	  /* Build the element for the field */
	  build_element (field, element_req, element_index, fptr, mbuf);
	  /* Goto next element */
	  element_index++;
	}

      /* Expect end of field marker here */
      if (!match_Mbuf (mbuf, FIELD_END))
	{
	  if (field_decl->kleene_starred_req == NULL)
	    {
	      Ferror (fptr,
		      "Expecting end of field marker '%s' for field '%s'.\n"
		      "Only %i elements expected for this field.",
		      FIELD_END, field_name,
		      field_decl->max_require_index + 1);}
	  else
	    {
	      Ferror (fptr,
		      "Expecting end of field marker '%s' for field '%s'.\n"
		      "The number of elements required for this field is %i "
		      "or more.",
		      FIELD_END, field_name, field_decl->max_require_index);}
	}

      /* Free malloced field name */
      free (field_name);
    }

  /* Get the terminator at the end of the entry */
  Fget_token (fptr, mbuf);
  if (!match_Mbuf (mbuf, TERMINATOR))
    {
      Ferror (fptr,
	      "Expecting '%s%s' at end of the entry '%s'.",
	      ENTRY_END, TERMINATOR, entry_name);}

  /* Free malloced name buffers */
  free (entry_name);
  free (stripped_entry_name);
}


/* Checks for links that have become ambiguous after the entire database
 * has been built (Most will be caught while building the database).
 *
 * Returns the number of ambiguous links found.
 */
int
check_md_for_ambiguous_links (FILE * out, MD * md)
{
  MD_Section *section, **link_array;
  MD_Entry *entry, *target, *link;
  MD_Field_Decl *field_decl, **field_decl_array;
  MD_Field *field, **field_array;
  MD_Element_Req *element_req, **require_array, *kleene_starred_req;
  MD_Element *element, **element_array;
  char *link_name;
  int field_index, max_field_index, max_require_index;
  int element_index, max_element_index, link_array_size;
  int error_count, i;
  /* Initialize error count */
  error_count = 0;
  /* Scan every section in md */
  for (section = MD_first_section (md); section != NULL;
       section = MD_next_section (section))
    {
      /* Get values into local variables for ease of use */
      field_decl_array = section->field_decl;
      max_field_index = section->max_field_index;
      /* Scan every entry in this section */
      for (entry = MD_first_entry (section); entry != NULL;
	   entry = MD_next_entry (entry))
	{
	  /* Get values into local variables for ease of use */
	  field_array = entry->field;
	  /* Scan every field for multi-target links */
	  for (field_index = 0; field_index <= max_field_index; field_index++)
	    {
	      /* Get the field for this index */
	      field = field_array[field_index];
	      /* Skip undefined fields */
	      if (field == NULL)
		continue;
	      /* Get the declaration for this field */
	      field_decl = field_decl_array[field_index];
	      /* Get values into local variables for ease of use */
	      require_array = field_decl->require;
	      max_require_index = field_decl->max_require_index;
	      kleene_starred_req = field_decl->kleene_starred_req;
	      element_array = field->element;
	      max_element_index = field->max_element_index;
	      /* Scan every element in this field */
	      for (element_index = 0; element_index <= max_element_index;
		   element_index++)
		{
		  /* Get element being checked */
		  element = element_array[element_index];
		  /* Skip NULL elements */
		  if (element == NULL)
		    continue;
		  /* If index in require bounds, get requirement from
		   * array.
		   */
		  if (element_index <= max_require_index)
		    element_req = require_array[element_index];
		  /* Otherwise get the kleene starred requirement
		   * (may be NULL)
		   */
		  else
		    element_req = kleene_starred_req;
		  /* Sanity check, element_req should not be NULL */
		  if (element_req == NULL)
		    {
		      L_punt
			("check_md_for_ambiguous_links: algorithm error, "
			 "element_req == NULL");
		    }

		  /* Skip all elements except multi-target links */
		  if ((element_req->type != MD_LINK) ||
		      (element_req->link_array_size == 1))
		    continue;
		  link_array = element_req->link;
		  link_array_size = element_req->link_array_size;
		  link = element->value.l;
		  link_name = link->name;
		  for (i = 0; i < link_array_size; i++)
		    {
		      target = MD_find_entry (link_array[i], link_name);
		      if ((target != NULL) && (target != link))
			{
			  error_count++;
			  fprintf (out,
				   "\n%s->%s->%s[%i]: Link ambiguous after "
				   "entire md read in.\n'%s' could refer to "
				   "'%s->%s' or '%s->%s'.\n",
				   section->name, target->name,
				   field_decl->name, element_index,
				   link->name, link->section->name,
				   link->name, target->section->name,
				   target->name);}
		    }
		}
	    }
	}

    }

  return (error_count);
}

void
L_punt (char *fmt, ...)
{
  va_list args;
  fprintf (stderr, "Fatal error:\n    ");
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  exit (-1);
}
