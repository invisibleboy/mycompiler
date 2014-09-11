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
 *      File :          l_error.c
 *      Description :   Lcode error handling routines
 *      Author :        John Gyllenhaal, Wen-mei Hwu
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

void
L_punt (char *fmt, ...)
{
  va_list args;
  char *name;

  fflush(stdout);

  if (L_fn)
     name = L_fn->name;
  else
     name = "nofunc";
 
  fprintf (stderr, "Error while %s was processing %s (%s):\n    ",
           L_curr_pass_name, name, L_input_file);
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  fflush(stderr);

  /* If L_dump_core_on_punt is set, trigger a segfault instead of exiting. */
  if (L_dump_core_on_punt)
    {
      volatile int *zero = NULL;

      *zero = 0;
    }

  exit (-1);
}

void
L_warn (char *fmt, ...)
{
  va_list args;
  char *name;

  if (L_fn)
     name = L_fn->name;
  else
     name = "nofunc";
 
  fprintf (stderr, "Warning while %s was processing %s (%s):\n    ",
           L_curr_pass_name, name, L_input_file);
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  return;
}
