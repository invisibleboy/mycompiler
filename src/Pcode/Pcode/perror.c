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
 *	File:	error.c
 *	Author:	Robert Kidd and Wen-mei Hwu
 *	Extends pcode.h written by:  Nancy Warter
 * 	Copyright (c) 2003 Robert Kidd, Nancy Warter, Wen-mei Hwu
 *		and The Board of Trustees of the University of Illinois.
 *		All rights reserved.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
 *
 *      This file contains the definitions for P_punt and P_warn.  These
 *      were originally defined in pcode.c.
 *
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "perror.h"
#include "pcode.h"
#include "parms.h"

/* Set up the file handle for the error functions. */
void
P_error_init ()
{
  Flog = stderr;
}

/* Print error message and then punt the program. */
void
P_punt (char *fmt, ...)
{
  va_list args;

  fprintf (Flog, "Pcode error: ");
  va_start (args, fmt);
  vfprintf (Flog, fmt, args);
  va_end (args);
  fprintf (Flog, "\n");
  exit (-1);
}

/* Print a warning message to the log file and then return. */
void 
P_warn(char *fmt, ...)
{
  va_list     args;

  fprintf(Flog, "Pcode warning: ");
  va_start (args, fmt);
  vfprintf (Flog, fmt, args);
  va_end(args);
  fprintf (Flog,"\n");
  return;
}

/* Print a warning message to the log file and then return. */
void
Warning (char *mesg)
{
  fprintf (Flog, "Pcode warning: %s\n", mesg);
}

