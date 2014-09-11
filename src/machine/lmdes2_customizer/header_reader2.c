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
 *  File:   header_reader2.c
 *  Author: John C. Gyllenhaal, Wen-mei Hwu
 *
 *  Description:
 *    Reads a header file and builds a symbol table from it
 *
 *  Creation Date :  April, 1993
 *  Updated for hmdes2_customizer :  August, 1995
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <library/i_error.h>
#include <library/dynamic_symbol.h>

#define MLINE_SIZE 4001

typedef struct hparse_info_st
{
  STRING_Symbol_Table *table;
  char *name;
  FILE *in;
  int line;
  int comment_level;
  int endif_level;
}
Hparse_Info;

/* Prototypes */
void Hskip_to_endif (Hparse_Info * hinfo, char *source, int line);

int Hget_next_line (Hparse_Info * hinfo, char *ret_buf);
void Hparse_line (char *ptr, char *name, char *arg);

STRING_Symbol_Table *
read_header_file (char *file_name, int print_warnings)
{
  Hparse_Info hinfo;
  char buf[MLINE_SIZE];
  char name[MLINE_SIZE];
  char arg_buf[MLINE_SIZE];
  char *prev_val;
  STRING_Symbol_Table *table;

  /* Extra unused variables; Wall clean; 01-03-24 Sain-Zee Ueng */
  /* char     *arg_ptr; */
  /* int              i; */

  /* Initialize fields for parsing */
  hinfo.name = (char *) strdup (file_name);
  hinfo.line = 0;
  hinfo.table = STRING_new_symbol_table (file_name, 256);
  hinfo.comment_level = 0;
  hinfo.endif_level = 0;

  if ((hinfo.in = fopen (file_name, "r")) == NULL)
    I_punt ("Cannot open C header file '%s'", file_name);

  while (Hget_next_line (&hinfo, buf) != EOF)
    {
      if (buf[0] != '#')
	continue;

      if ((strncmp ("#define", buf, 7) == 0) && (isspace (buf[7])))
	{
	  /* Break line into name and arg (if any) */
	  Hparse_line (&buf[8], name, arg_buf);

	  /* Make sure name defined */
	  if (name[0] == 0)
	    I_punt ("error (%s line %i):\n    Name expected in define ('%s')",
		    hinfo.name, hinfo.line, buf);

	  /* Make sure not already defined */
	  prev_val = (char *) STRING_find_symbol_data (hinfo.table, name);
	  if (prev_val != NULL)
	    {
	      I_punt
		("error (%s line %i):\n    Redefinition of '%s' "
		 "(prev val '%s')",
		 hinfo.name, hinfo.line, name, prev_val);
	    }

	  /* Insert defined value into symbol table */
	  STRING_add_symbol (hinfo.table, name, (void *) strdup (arg_buf));
	}
      else if ((strncmp ("#undef", buf, 6) == 0) && (isspace (buf[6])))
	{
	  /* Break line into name and arg (if any) */
	  Hparse_line (&buf[7], name, arg_buf);

	  /* Make sure name defined */
	  if (name[0] == 0)
	    I_punt ("error (%s line %i):\n    Name expected in define ('%s')",
		    hinfo.name, hinfo.line, buf);

	  /* Make sure not already defined */
	  prev_val = (char *) STRING_find_symbol_data (hinfo.table, name);
	  if (prev_val != NULL)
	    {
	      I_punt ("Someone needs to implement #undef");
	    }
	}
      else if ((strncmp ("#ifdef", buf, 6) == 0) && (isspace (buf[6])))
	{
	  /* Break line into name and arg (if any) */
	  Hparse_line (&buf[7], name, arg_buf);

	  /* If name not defined, skip to matching #endif */
	  if (STRING_find_symbol (hinfo.table, name) == NULL)
	    Hskip_to_endif (&hinfo, buf, hinfo.line);
	}
      else if ((strncmp ("#ifndef", buf, 7) == 0) && (isspace (buf[7])))
	{
	  /* Break line into name and arg (if any) */
	  Hparse_line (&buf[8], name, arg_buf);

	  /* If name defined, skip to matching #endif */
	  if (STRING_find_symbol (hinfo.table, name) != NULL)
	    Hskip_to_endif (&hinfo, buf, hinfo.line);
	}
      else if (strncmp ("#endif", buf, 6) == 0)
	{
	  /* Do nothing */
	}
      else if ((strncmp ("#include", buf, 8) == 0) && (isspace (buf[8])))
	{
	  if (print_warnings)
	    {
	      fprintf (stdout, "Warning (%s line %i):\n    '%s' ignored\n",
		       hinfo.name, hinfo.line, buf);
	    }
	}
      else
	{
	  I_punt ("Warning (%s line %i):\n    Unsupported directive '%s'",
		  hinfo.name, hinfo.line, buf);
	}
    }
  fclose (hinfo.in);

  table = hinfo.table;
  free (hinfo.name);

  return (table);
}

/*
 * Skips to the matching #endif for  a #ifdef or #ifndef
 */
void
Hskip_to_endif (Hparse_Info * hinfo, char *source, int line)
{
  char buf[MLINE_SIZE];

  hinfo->endif_level = 1;
  while (Hget_next_line (hinfo, buf) != EOF)
    {
      if (strncmp ("#ifdef ", buf, 7) == 0)
	{
	  hinfo->endif_level++;
	}
      else if (strncmp ("#ifndef ", buf, 8) == 0)
	{
	  hinfo->endif_level++;
	}
      else if (strncmp ("#endif", buf, 6) == 0)
	{
	  hinfo->endif_level--;
	  if (hinfo->endif_level == 0)
	    return;
	}
    }
  I_punt ("%s: Hit EOF searching for the #endif matching '%s' on line %i",
	  hinfo->name, source, line);
}

void
Hparse_line (char *ptr, char *name, char *arg)
{
  int i;

  /* Strip white space before name */
  while ((*ptr != 0) && isspace (*ptr))
    ptr++;

  /* Read name from buf */
  for (i = 0; (ptr[i] != 0) && !isspace (ptr[i]); i++)
    name[i] = ptr[i];
  name[i] = 0;

  ptr += i;

  /* Skip white space before argument */
  while ((*ptr != 0) && isspace (*ptr))
    ptr++;

  /* Read arg from buf */
  for (i = 0; ptr[i] != 0; i++)
    arg[i] = ptr[i];
  arg[i] = 0;
}

int
Hget_next_line (Hparse_Info * hinfo, char *ret_buf)
{
  char in_buf[MLINE_SIZE], *buf;
  char *ptr;
  int i, pos;

  while (fgets (in_buf, MLINE_SIZE, hinfo->in) != NULL)
    {
      hinfo->line++;

      /* Detect lines that are too long */
      if (strlen (in_buf) >= (MLINE_SIZE - 1))
	I_punt ("%s line %i: Line too long. Max length = %i",
		hinfo->name, hinfo->line, MLINE_SIZE - 1);

      /* Move to pointer so we can manipulate */
      buf = in_buf;

      /* Strip leading white space */
      for (; (*buf != 0) && isspace (*buf); buf++)
	;

      pos = 0;
      for (ptr = buf; *ptr != 0; ptr++)
	{
	  /* Detect start of comment */
	  if ((ptr[0] == '/') && (ptr[1] == '*'))
	    {
	      hinfo->comment_level++;
	      ptr++;
	      continue;
	    }

	  /* Detect end of comment */
	  if ((ptr[0] == '*') && (ptr[1] == '/'))
	    {
	      hinfo->comment_level--;
	      if (hinfo->comment_level < 0)
		I_punt ("%s line %i: Unmatched '*/' (nesting allowed)",
			hinfo->name, hinfo->line);
	      ptr++;
	      continue;
	    }

	  /* Copy info to ret buffer if not commented out */
	  if (hinfo->comment_level == 0)
	    {
	      ret_buf[pos] = *ptr;
	      pos++;
	    }
	}

      /* Terminate ret_buf */
      ret_buf[pos] = 0;

      /* Strip trailing white space */
      for (i = pos - 1; (i >= 0) && isspace (ret_buf[i]); i--)
	ret_buf[i] = 0;

      /* If return buffer is not empty, return 0 */
      if (strlen (ret_buf) != 0)
	return (0);
    }

  /* Reaches here on EOF only */
  return (EOF);
}
