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
 *  File:  mfile.h
 *
 *  Description:  Header file for the IMPACT in-memory file interface
 *
 *  Creation Date :  October 1994
 *
 *  Authors:  John C. Gyllenhaal, Wen-mei Hwu
 *
 *  Revisions:
 *
\*****************************************************************************/

#ifndef MFILE_H
#define MFILE_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/l_alloc_new.h>
#include <stdarg.h>
#define  MAXARGS        100
#include "mbuf.h"


typedef struct Mline
{
  char *buf;
  int len;
  int line_no;
  struct Mfile *mfile;
  struct Mline *next;
  struct Mline *prev;
}
Mline;

typedef struct Mfile
{
  char *name;
  char *desc;
  Mline *head;
  Mline *tail;
  int line_count;
}
Mfile;

typedef struct Mptr
{
  Mfile *mfile;
  Mline *mline;
  int pos;
}
Mptr;


#ifdef __cplusplus
extern "C"
{
#endif

/* External variables */
  extern L_Alloc_Pool *Mfile_pool;
  extern L_Alloc_Pool *Mline_pool;
  extern L_Alloc_Pool *Mptr_pool;


/* Mfile prototypes */
  extern Mfile *create_Mfile (FILE * in, char *input_file_name, char *desc);
  extern void free_Mfile (Mfile * mfile);
  extern Mptr *create_Mptr (Mfile * mfile);
  extern Mptr *copy_Mptr (Mptr * orig_mptr);
  extern void move_Mptr (Mptr * old_mptr, Mptr * new_mptr);
  extern void free_Mptr (Mptr * mptr);
  extern int Mptr_pos (Mptr * mptr);

  extern int Mgetc (Mptr * mptr);
  extern int Mpeekc (Mptr * mptr);
  extern void Mbackupc (Mptr * mptr);

/* Error routine */
  extern void print_buf_with_arrow (FILE * out, char *buf, int pos);
  extern void Merror (Mptr * mptr, char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
