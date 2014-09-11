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
/*! \file
 * \brief Routines to read Pcode from a file.
 *
 * \author Robert Kidd, Wen-mei Hwu
 *
 * This file contains routines to read Pcode from a file.
 */
/*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "pcode.h"
#include "read.h"
#include "parse.h"
#include "perror.h"
#include "lex.h"

/* P_yyparse() is defined in parse.c, which is generated from parse.y. */
extern int P_yyparse (void);

/*! \brief Reads a single Dcl from the input file.
 *
 * \param in
 *  the file to read from.
 *
 * \return
 *  A pointer to a Dcl containing the structure read from \a in.
 *
 * Reads a Dcl from the file handle \a in and returns a pointer.
 * It is the caller's responsibility to free this pointer.
 */
Dcl
P_read_dcl (FILE *in)
{
  Dcl result = NULL;

  lexSetInput (in);

  if (P_yyparse () == 0)
    result = P_Input;

  return (result);
}

/*! \brief Moves the current file position in the input file.
 *
 * \param file
 *  the file to seek.
 * \param pos
 *  the new position in the file.
 *
 * Moves the current file position in the input file and notifies the
 * lexer, if necessary.
 */
void
P_input_seek (FILE *file, long pos)
{
  if (fseek (file, pos, SEEK_SET) != 0)
    P_punt ("read.c:P_seek:%d Could not seek", __LINE__);

  lexNotifySeek (file);

  return;
}

/*! \brief Cleans up the lexer when reading is finished.
 *
 * \param file
 *  the file to finish.
 */
void
P_read_finish (FILE *file)
{
  lexSetInput (file);

  lexFinish ();

  return;
}
