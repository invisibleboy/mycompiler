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
 *      File:   s_recorder.c
 *      Author: Po-hua Chang
 *      Creation Date:  July 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================
 *      Description :   Record experiment result
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <library/c_basic.h>
#include <library/s_recorder.h>

#define DEBUG

extern double atof (const char *);

static C_File file = 1;

int
S_create_record (char *name)
{
  int id;
  id = C_open_file (name, C_OPEN_APPEND_ONLY);
  file = id;
  return id;
}

void
S_store_record (void)
{
  C_write_word (file, "\n");
  C_close_file (file);
}

void
S_print_integer (C_Integer value)
{
  C_write_integer (file, value);
}

void
S_print_real (C_Double value)
{
  C_write_double (file, value);
}

void
S_print_name (C_String str)
{
  int i, len;
  /*
   *  convert spaces to underscores.
   */
  len = strlen (str);
  for (i = 0; i < len; i++)
    {
      if (str[i] == ' ')
        str[i] = '_';
    }
  C_write_word (file, str);
}

int
S_sum_of_column (char *name, int N, double *sum)
{
  int i, id, row;
  double value;
  char line[5120], word[512], *ptr;
  id = C_open_file (name, C_OPEN_READ_ONLY);
  if (id < 0)
    return -1;
  if (sum != 0)
    *sum = 0.0;
  row = 0;
  value = 0.0;
  for (;;)
    {
      C_read_line (id, line, 5120);
      if (C_eof (id))
        break;
      row += 1;
      ptr = line;
      for (i = 0; i < N; i++)
        {
          if (sscanf (ptr, "%s", word) != 1)
            {
              return -row;
            }
          ptr += (strlen (word) + 1);
        }
      value += atof (word);
#ifdef DEBUG
      printf ("> %s (%f)\n", word, value);
#endif
    }
  C_close_file (id);
  if (sum != 0)
    *sum = value;
  return row;
}
