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
/*===========================================================================
 *      File :          i_types.c
 *      Description :   IMPACT base types
 *      Creation Date : July 2000
 *      Author :        John W. Sias, Wen-mei Hwu
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <library/i_error.h>
#include "i_types.h"

/* This is a function call version of ITicast (i_types.h).  It is useful
   for setting a debugger breakpoint at the punt line.  MCM 7/2000 */
ITint32 ITicast_fn (ITintmax v)
{
  ITintmax w = (ITintmax) (ITint32) (v);
  if (v == w)
    return (ITint32) (v);
  else
    {
      IT_punt (__FILE__ ": %d MCM_ITicast with value %llx", __LINE__, v);
      return ((ITint32) 0);
    }
}

int
IT_typecheck (void)
{
  int sz;

#define IT_TYPECK(type, tname, size)   if ((sz = sizeof(type)) != size) \
    I_punt("IT_typecheck: %s is %d bytes instead of %d", tname, sz, size);

  IT_TYPECK (ITint8, "ITint8", 1);
  IT_TYPECK (ITuint8, "ITuint8", 1);
  IT_TYPECK (ITint16, "ITint16", 2);
  IT_TYPECK (ITuint16, "ITuint16", 2);
  IT_TYPECK (ITint32, "ITint32", 4);
  IT_TYPECK (ITuint32, "ITuint32", 4);

#ifdef IT64BIT
  IT_TYPECK (ITint64, "ITint64", 8);
  IT_TYPECK (ITuint64, "ITuint64", 8);
#endif

  return 0;
}

int
IT_punt (char *fmt, ...)
{
  va_list args;

  fprintf (stderr, "ITicast assertion failed at ");
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  exit (-1);
  return 0;
}
