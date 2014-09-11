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
 *  File:  mfile.c
 *
 *  Description:  The IMPACT in-memory file interface
 *
 *  Creation Date :  October 1994
 *
 *  Authors:  John C. Gyllenhaal, Wen-mei Hwu
 *
 *  Revisions:
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library/i_error.h>

#include "mfile.h"

/* Alloc pools */
L_Alloc_Pool *Mfile_pool = NULL;
L_Alloc_Pool *Mline_pool = NULL;
L_Alloc_Pool *Mptr_pool = NULL;

/* Print out buffer and put an arrow to the character specified */
void
print_buf_with_arrow (FILE * out, char *buf, int pos)
{
  int i, ch;
  int col, arrow_col;

  /* Start in column 0 */
  col = 0;
  arrow_col = 0;

  /* Print out text, upto newline after pos.
   * (Since expanded text can have many newlines in it)
   */
  for (i = 0; buf[i] != 0; i++)
    {
      ch = buf[i];

      /* Get column arrow should go at */
      if (i == pos)
        arrow_col = col;

      col++;

      /* Expand tabs */
      if (ch == '\t')
        {
          /* Put out first character */
          putc (' ', out);

          /* Add spaces until at multiple of 8 */
          while ((col % 8) != 0)
            {
              col++;
              putc (' ', out);
            }

        }
      else
        putc (ch, out);

      /* Handle newlines */
      if (ch == '\n')
        {
          /* Return to column 0 at newline */
          col = 0;

          /* Stop at newline after pos */
          if (i >= pos)
            break;
        }
    }

  /* Get column arrow should go at (if past end of buf) */
  if (i <= pos)
    arrow_col = col;

  /* Add trailing newline if none in buffer */
  if (buf[i] != '\n')
    putc ('\n', out);



  /* Put arrow at appropriate column detected in above loop */
  for (i = 0; i < arrow_col; i++)
    putc (' ', out);
  fprintf (out, "^\n");
}


/*
 * Mfile error handler.
 * First argument is a mptr that points to where the error occured.
 * The second argument is a printf format string, and the rest
 * are the arguments for the format string.
 */
void
Merror (Mptr * mptr, char *fmt, ...)
{
  va_list args;

  /* 
   * Print out text line of text where error occured and put arrow to
   * location.  (pos starts at 0, so add 1 to get column)
   */
  fprintf (stderr, "\n");
  /* If not at EOF */
  if (mptr->mline != NULL)
    {
      fprintf (stderr,
               "Error during %s (line %i char %i of %s):\n",
               mptr->mfile->desc,
               mptr->mline->line_no, mptr->pos + 1, mptr->mfile->name);
    }
  /* If at EOF */
  else
    {
      fprintf (stderr,
               "Error during %s (%s at EOF):\n",
               mptr->mfile->desc, mptr->mfile->name);
    }

  /*Print error message */
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  fprintf (stderr, "\n");

  /* Do not print out file text if at EOF */
  if (mptr->mline != NULL)
    {
      /* Print out file text where error occurred */
      fprintf (stderr, "File text where error occurred:\n");
      print_buf_with_arrow (stderr, mptr->mline->buf, mptr->pos);
    }

  fprintf (stderr, "Fatal error, cannot continue.\n");
  fprintf (stderr, "\n");

  exit (-1);
}

/* Loads file into memory (Mfile structure) and returns a newly 
 * allocated Mfile structure.
 */
Mfile *
create_Mfile (FILE * in, char *input_file_name, char *desc)
{
  Mfile *mfile;
  Mline *mline;
  Mbuf *mbuf;
  int ch;
  int len;

  /* Allocate structure for Mfile, create pools if necessary */
  if (Mfile_pool == NULL)
    {
      Mfile_pool = L_create_alloc_pool ("Mfile", sizeof (Mfile), 16);
    }
  if (Mline_pool == NULL)
    {
      Mline_pool = L_create_alloc_pool ("Mline", sizeof (Mline), 256);
    }

  mfile = (Mfile *) L_alloc (Mfile_pool);

  /* Initialize fields */
  mfile->name = strdup (input_file_name);
  mfile->desc = strdup (desc);
  mfile->head = 0;
  mfile->tail = 0;
  mfile->line_count = 0;

  /* Create mbuf for file */
  mbuf = create_Mbuf ();

  /* Read in file */
  while (1)
    {
      /* Clear mbuf so we can put the next line it it */
      clear_Mbuf (mbuf);

      while ((ch = getc (in)) != EOF)
        {
          /* Add character to mbuf */
          addc_to_Mbuf (mbuf, ch);

          /* If hit newline, end line */
          if (ch == '\n')
            break;
        }

      /* Add line if Mbuf not empty */
      if ((len = len_of_Mbuf (mbuf)) > 0)
        {
          /* Increment line count */
          mfile->line_count++;

          /* Allocate a Mline for this line */
          mline = (Mline *) L_alloc (Mline_pool);

          /* Init contents of line */
          mline->buf = copy_Mbuf_buf (mbuf);
          mline->len = len;
          mline->line_no = mfile->line_count;
          mline->mfile = mfile;

          /* Add to mfile's linked list of lines */
          if (mfile->tail == NULL)
            mfile->head = mline;
          else
            mfile->tail->next = mline;
          mline->prev = mfile->tail;
          mline->next = NULL;
          mfile->tail = mline;
        }

      /* Stop at EOF */
      if (ch == EOF)
        break;
    }

  /* Free up allocated memory */
  free_Mbuf (mbuf);

  return (mfile);
}

/* Free memory Mfile is using */
void
free_Mfile (Mfile * mfile)
{
  Mline *mline, *next_mline;

  /* Free all the lines in the mfile */
  for (mline = mfile->head; mline != NULL; mline = next_mline)
    {
      /* Get next mline before we delete anything */
      next_mline = mline->next;

      /* Free buffer and L_free structure */
      free (mline->buf);
      L_free (Mline_pool, mline);
    }

  /* Free name buffer and L_free structure */
  free (mfile->name);
  free (mfile->desc);
  L_free (Mfile_pool, mfile);
}

/* Create mptr to start of mfile */
Mptr *
create_Mptr (Mfile * mfile)
{
  Mptr *mptr;

  if (Mptr_pool == NULL)
    {
      Mptr_pool = L_create_alloc_pool ("Mptr", sizeof (Mptr), 16);
    }
  mptr = (Mptr *) L_alloc (Mptr_pool);

  mptr->mfile = mfile;
  mptr->mline = mfile->head;
  mptr->pos = 0;

  return (mptr);
}
/* Duplicates a Mptr exactly 
 * Returns the duplicated Mptr (must be freed)
 */
Mptr *
copy_Mptr (Mptr * orig_mptr)
{
  Mptr *new_mptr;

  /* alloc and copy over fields */
  new_mptr = (Mptr *) L_alloc (Mptr_pool);

  new_mptr->mfile = orig_mptr->mfile;
  new_mptr->mline = orig_mptr->mline;
  new_mptr->pos = orig_mptr->pos;

  return (new_mptr);
}

/* Moves the old mptr to the new mptr location */
void
move_Mptr (Mptr * old_mptr, Mptr * new_mptr)
{
  old_mptr->mfile = new_mptr->mfile;
  old_mptr->mline = new_mptr->mline;
  old_mptr->pos = new_mptr->pos;
}


void
free_Mptr (Mptr * mptr)
{
  /* L_free structure */
  L_free (Mptr_pool, mptr);
}

/* Gets the next character from the mfile. */
int
Mgetc (Mptr * mptr)
{
  int ch;

  /* If at EOF, return EOF */
  if (mptr->mline == NULL)
    return (EOF);

  /* Get next character in file */
  ch = mptr->mline->buf[mptr->pos];

  /* Goto next character, and next line if necessary */
  mptr->pos++;
  if (mptr->pos >= mptr->mline->len)
    {
      mptr->mline = mptr->mline->next;
      mptr->pos = 0;
    }

  return (ch);
}

/* Returns char position on line of Mptr */
int
Mptr_pos (Mptr * mptr)
{
  return (mptr->pos);
}

/* Peeks at the next character from the mfile. */
int
Mpeekc (Mptr * mptr)
{
  char ch;

  if (mptr->mline == NULL)
    ch = EOF;
  else
    ch = mptr->mline->buf[mptr->pos];

  return (ch);
}

/* Backup one character in the file.
 * Cannot backup before beginning of file.
 */
void
Mbackupc (Mptr * mptr)
{
  /* If at EOF, goto last character in file */
  if (mptr->mline == NULL)
    {
      mptr->mline = mptr->mfile->tail;
      if (mptr->mline != NULL)
        mptr->pos = mptr->mline->len - 1;
    }

  /* If at beginning of file, goto end of last line */
  else if (mptr->pos == 0)
    {
      mptr->mline = mptr->mline->prev;
      if (mptr->mline != NULL)
        mptr->pos = mptr->mline->len - 1;
      else
        I_punt ("Mbackupc: backing up before beginning of file.");
    }

  /* Otherwise, just go backwards one character */
  else
    {
      mptr->pos--;
    }
}
