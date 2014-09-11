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
 *      File:   set.h
 *      Author: Po-hua Chang
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1993 Po-hua Chang, Richard Hank, and Wen-mei Hwu. 
 *      All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_SET_H
#define IMPACT_SET_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/i_types.h>

/*===========================================================================
 *      Description :   Bit Vector Set Operations
 *==========================================================================*/

typedef ITuint32 _32bit;   /* make sure this is actually 32 bits */

typedef _32bit *Set_Vector;

typedef struct _Set
{
  void *dummy;
  int max_base[2];
  Set_Vector *varray[2];
  int id;
}
_Set, *Set;

typedef struct _SetIterator
{
  Set set;
  int current_varray;
  int current_base;
  int current_bit;
  int nxt;
  int finis;
}
SetIterator;

#define Set_VECTOR_LENGTH       (8*32)  /* 8*32 elements */
#define Set_VARRAY_SIZE         8
#define Set_VECTOR_SIZE         8       /* 8 _32bit entries */
#define Set_INDEX(x)            (x & 0x0FF)     /* extract lower 8 bits */
#define Set_BASE(x)             (x >> 8)        /* discard lower 8 bits */
#define Set_CONSTRUCT_BASE(x)   (x << 8)
#define Set_WORD_SELECT(x)      (x >> 5 )       /* x / 32 */
#define Set_BIT_SELECT(x)       (x & 0x01F)     /* extract lower 5 bits */

#define Set_INVALID     ((Set)-1)

#ifdef __cplusplus
extern "C"
{
#endif

  extern int Set_in (Set, int); /* (set, element) */
  extern int Set_size (Set);    /* (set) */
  extern int Set_same (Set, Set);       /* (set1, set2) */
  extern int Set_empty (Set);   /* (set) */
  extern int Set_subtract_empty (Set, Set);     /* (set, set) */
  extern int Set_intersect_empty (Set, Set);    /* (set, set) */

  extern Set Set_add (Set, int);        /* (set, element) */
  extern Set Set_delete (Set, int);     /* (set, element) */

  extern Set Set_copy (Set);    /* (set) */

  extern Set Set_union (Set, Set);      /* (set, set) */
  extern Set Set_intersect (Set, Set);  /* (set, set) */
  extern Set Set_subtract (Set, Set);   /* (set, set) */
  extern Set Set_subtract_union (Set, Set, Set);
  /* (set1, set2, set3)  (set1 - set2) + set3 */

/* Performs set operations without creating a new set. */
/* The result is accumulated in the first parameter     */
/* (A new set is created iff the first parm is NULL)    */
  extern Set Set_union_acc (Set, Set);  /* (set1, set2) */
  extern Set Set_union_acc_ch(Set set1, Set set2, int *change);
  extern Set Set_intersect_acc (Set, Set);      /* (set1, set2) */
  extern Set Set_subtract_acc (Set, Set);       /* (set1, set2) */
  extern Set Set_subtract_union_acc (Set, Set, Set);
  /* (set1, set2, set3)  (set1 - set2) + set3 */

  extern Set Set_dispose (Set); /* (set) */
  extern Set Set_compress (Set);        /* (set) */
  extern Set Set_new (void);

  extern void Set_print (FILE *, char *, Set);  /* (F, name, set) */

  extern int Set_2array (Set, int *);   /* (set, buf, len) */

  extern SetIterator *Set_iterator (Set set, int finis);
  extern int Set_exhausted (SetIterator * si);
  extern int Set_next (SetIterator * si);
  extern void Set_reset (SetIterator * si, Set set);
  extern SetIterator *Set_end_iterator (SetIterator * si);
  extern void Set_memory_usage (long *pused, long *presv, long *pmax);

#ifdef __cplusplus
}
#endif

#endif
