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
 *  File:  mbuf.c
 *
 *  Description:  Self-resizing buffer useful for reading in files
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
#include <ctype.h>
#include <library/i_error.h>

#include "mbuf.h"


L_Alloc_Pool *Mbuf_pool = NULL;

Mbuf *
create_Mbuf ()
{
  Mbuf *mbuf;

  /* Get memory for mbuf, alloc pool on first use */
  if (Mbuf_pool == NULL)
    {
      Mbuf_pool = L_create_alloc_pool ("Mbuf", sizeof (Mbuf), 16);
    }
  mbuf = (Mbuf *) L_alloc (Mbuf_pool);

  /* Initialize fields */
  mbuf->buf = NULL;

  clear_Mbuf (mbuf);

  /* Return mbuf ready to go */
  return (mbuf);
}

void
free_Mbuf (Mbuf * mbuf)
{
  /* Free buffer, then mbuf */
  free (mbuf->buf);
  L_free (Mbuf_pool, mbuf);
}


void
clear_Mbuf (Mbuf * mbuf)
{
  /* If never initialized before, malloc initial buffer */
  if (mbuf->buf == NULL)
    {
      mbuf->max_len = 2;
      if ((mbuf->buf = (char *) malloc (mbuf->max_len)) == NULL)
        I_punt ("Out of memory in clear_Mbuf");
    }

  /* Clear out buffer */
  mbuf->cur_len = 0;
  mbuf->buf[0] = 0;
}

void
addc_to_Mbuf (Mbuf * mbuf, char ch)
{
  char *old_buf;

  /* If at end of current buffer, malloc new one and copy over
   * contents.
   */
  if ((mbuf->cur_len + 1) >= mbuf->max_len)
    {
      old_buf = mbuf->buf;

      /* Double size of buffer */
      mbuf->max_len = mbuf->max_len * 2;

      /* Malloc new buffer */
      if ((mbuf->buf = (char *) malloc (mbuf->max_len)) == NULL)
        I_punt ("Out of memory in addc_to_Mbuf");

      /* Copy over old buffer */
      strcpy (mbuf->buf, old_buf);

      /* Free old buffer */
      free (old_buf);
    }

  /* Add character to buf (and terminate) */
  mbuf->buf[mbuf->cur_len] = ch;
  mbuf->buf[mbuf->cur_len + 1] = 0;
  mbuf->cur_len++;
}

void
adds_to_Mbuf (Mbuf * mbuf, char *str)
{
  char *ptr;

  for (ptr = str; *ptr != 0; ptr++)
    addc_to_Mbuf (mbuf, *ptr);
}

/* Pads buffer until length specified.  Does nothing if aleady longer 
 * than desired length
 */
void
pad_Mbuf_to_len (Mbuf * mbuf, int desired_len, char pad_ch)
{
  while (desired_len > mbuf->cur_len)
    addc_to_Mbuf (mbuf, pad_ch);
}

/* Strips trailing whitespace from mbuf */
void
strip_Mbuf (Mbuf * mbuf)
{
  int i;

  /* Find last non-whitespace */
  for (i = mbuf->cur_len - 1; i >= 0; i--)
    {
      if (!isspace (mbuf->buf[i]))
        break;
    }

  /* Adjust length and terminator */
  mbuf->cur_len = i + 1;
  mbuf->buf[i + 1] = 0;
}

/* return's a pointer to the actual mbuf (must not write to or free) */
char *
get_Mbuf_buf (Mbuf * mbuf)
{
  return (mbuf->buf);
}

/* Copy mbuf's contents, may have NULLs in string so use len info */
char *
copy_Mbuf_buf (Mbuf * mbuf)
{
  char *buf, *src_buf;
  int i, len;

  len = mbuf->cur_len;

  if ((buf = (char *) malloc (len + 1)) == NULL)
    I_punt ("copy_Mbuf_buf: Out of memory");

  src_buf = mbuf->buf;
  for (i = 0; i < len; i++)
    buf[i] = src_buf[i];

  /* Terminate string */
  buf[len] = 0;

  return (buf);
}


/* Returns length of mbuf */
int
len_of_Mbuf (Mbuf * mbuf)
{
  return (mbuf->cur_len);
}

/* Returns 1 if string matches mbuf, 0 otherwise */
int
match_Mbuf (Mbuf * mbuf, char *string)
{
  if (strcmp (mbuf->buf, string) == 0)
    return (1);
  else
    return (0);
}
