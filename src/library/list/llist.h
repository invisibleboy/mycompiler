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
 *      File:   llist.h
 *      Author: Poyung Brian Chang, Pohua Paul Chang
 *      Copyright (c) 1991 Poyung Brian Chang, Pohua Paul Chang, Wen-Mei Hwu.
 *      All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_LLIST_H
#define IMPACT_LLIST_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/l_alloc_new.h>

typedef struct _Lint
{
  int id;
  struct _Lint *next;
}
_Lint, *Lint;

typedef struct _Lptr
{
  void *ptr;
  struct _Lptr *next;
}
_Lptr, *Lptr;

typedef struct _LptrQ
{
  Lptr head, tail;
  int length;
}
_LptrQ, *LptrQ;

#ifdef __cplusplus
extern "C"
{
#endif

  extern int debug_lptr;
  extern int new_lptr_added;
  extern void Init_NewLint ();
  extern Lint NewLint (int id);
  extern Lint AppendLint (Lint list1, Lint list2);
  extern Lint AddToSortedLint (int p, Lint list);
  extern Lint CopyLint (Lint list);
  extern Lint FreeLint (Lint list);
  extern Lint FindNth (Lint list, int N);
  extern int InLint (Lint list, int id);
  extern Lint InSortedLint (Lint list, int ptr);
  extern int LintLength (Lint list);

  extern void Init_NewLptr ();
  extern Lptr NewLptr (void * ptr);
  extern Lptr AppendLptr (Lptr list1, Lptr list2);
  extern Lptr AddToSortedLptr (void * p, Lptr list);
  extern Lptr CopyLptr (Lptr list);
  extern Lptr FreeLptr (Lptr list);
/* BCC - added 4/3/97 */
  extern Lptr InLptr (Lptr list, void * ptr);
  extern Lptr InSortedLptr (Lptr list, void * ptr);
/* BCC - added 9/29/97 */
  extern Lptr NameInLptr (Lptr list, char *ptr);
/* BCC - added 12/8/97 */
  extern int LptrLength (Lptr list);
/* BCC - added 10/13/98 */
  extern int SameSortedLptr (Lptr list1, Lptr list2);
/* BCC - added 12/17/98 */
  extern void CopySortedLptrList (Lptr * base, Lptr list);

  extern void En_FIFO_LptrQ (LptrQ, void *);
  extern void En_FILO_LptrQ (LptrQ, void *);
  extern void *De_LptrQ (LptrQ);
  extern void *Peek_LptrQ (LptrQ Q);	/* CWL - 10/14/02 */


#ifdef __cplusplus
}
#endif

#endif
