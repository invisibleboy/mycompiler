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
 *      File:   llist.c
 *      Author: Poyung Chang, Pohua Chang.
 *      Copyright (c) 1991 Poyung Chang, Pohua Chang, Wen-Mei Hwu.
 *      All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "llist.h"

#if 0
#define DEBUG_APPEND
#endif
int debug_lptr;
int new_lptr_added;

#ifdef DEBUG_APPEND
static int free_list = 0;
#endif

static void
Punt (char *mesg)
{
  fprintf (stderr, "llist: %s\n", mesg);
  exit (-1);
}

/*----------------------------------------------------------------------*/
static Lint d_lint = 0;
L_Alloc_Pool *Lint_Pool;

void
Init_NewLint ()
{
  Lint_Pool = L_create_alloc_pool ("Lint", sizeof (_Lint), 256);
}

Lint NewLint (int id)
{
  Lint new_lint;
  if (d_lint != 0)
    {
      new_lint = d_lint;
      d_lint = d_lint->next;
    }
  else
    {
      new_lint = (Lint) L_alloc (Lint_Pool);
      if (new_lint == 0)
        Punt ("NewLint: out of memory space");
    }
  new_lint->next = 0;
  new_lint->id = id;
  return new_lint;
}

Lint AppendLint (Lint list1, Lint list2)
{
  Lint ptr;

#ifdef DEBUG_APPEND
  {
    Lint ptr1, ptr2;
    for (ptr1 = list1; ptr1; ptr1 = ptr1->next)
      {
        for (ptr2 = list2; ptr2; ptr2 = ptr2->next)
          {
            if (ptr1 == ptr2)
              Punt ("ptr1 == ptr2 found");
            if (free_list == 0)
              {
                if (ptr1->id == ptr2->id)
                  Punt ("ptr1->id == ptr2->id found");
              }
          }
      }
  }
#endif
  if (list1 == 0)
    return list2;
  for (ptr = list1; ptr->next != 0; ptr = ptr->next)
    ;
  ptr->next = list2;
  return list1;
}
Lint CopyLint (Lint list)
{
  Lint new_lint;
  if (list == 0)
    return 0;
  new_lint = NewLint (list->id);
  new_lint->next = CopyLint (list->next);
  return new_lint;
}
Lint FreeLint (Lint list)
{
#ifdef DEBUG_APPEND
  free_list = 1;
#endif
  d_lint = AppendLint (list, d_lint);
#ifdef DEBUG_APPEND
  free_list = 0;
#endif
  return 0;
}
Lint FindNth (Lint list, int N)
{
  int i;
  if (list == 0)
    return 0;
  i = 1;
  while (list != 0)
    {
      if (i == N)
        return list;
      i += 1;
      list = list->next;
    }
  return 0;
}
int
InLint (Lint list, int id)
{
  while (list != 0)
    {
      if (list->id == id)
        return 1;
      list = list->next;
    }
  return 0;
}
int
PrintLint (Lint list)
{
  while (list != 0)
    {
      fprintf (stderr, "%d\n", list->id);
      list = list->next;
    }
  return 0;
}
/*----------------------------------------------------------------------*/
static Lptr d_lptr = 0;
L_Alloc_Pool *Lptr_Pool;

void
Init_NewLptr ()
{
  Lptr_Pool = L_create_alloc_pool ("Lptr", sizeof (_Lptr), 256);
}

Lptr NewLptr (void * ptr)
{
  Lptr new_ptr;
  if (d_lptr != 0)
    {
      new_ptr = d_lptr;
      d_lptr = d_lptr->next;
    }
  else
    {
      if (!Lptr_Pool)
	Init_NewLptr();
      new_ptr = (Lptr) L_alloc (Lptr_Pool);
      if (new_ptr == 0)
        Punt ("NewLptr: out of memory space");
    }
  new_ptr->next = 0;
  new_ptr->ptr = ptr;
  return new_ptr;
}

Lptr AppendLptr (Lptr list1, Lptr list2)
{
  Lptr ptr;

#ifdef DEBUG_APPEND
  {
    Lptr ptr1, ptr2;
    for (ptr1 = list1; ptr1; ptr1 = ptr1->next)
      {
        for (ptr2 = list2; ptr2; ptr2 = ptr2->next)
          {
            if (ptr1 == ptr2)
              Punt ("ptr1 == ptr2 found");
            if (free_list == 0)
              {
                if (ptr1->ptr == ptr2->ptr)
                  Punt ("ptr1->ptr == ptr2->ptr found");
              }
          }
      }
  }
#endif
  if (list1 == 0)
    return list2;
  for (ptr = list1; ptr->next != 0; ptr = ptr->next)
    ;
  ptr->next = list2;
  return list1;
}

Lptr AddToSortedLptr (void * p, Lptr list)
{
  Lptr ptr1, ptr2;

  if (!Lptr_Pool)
    Init_NewLptr ();

#if 0
  if (debug_lptr)
    {
      length = LptrLength (list);
      pos = 0;
    }
#endif
  new_lptr_added = 0;
  /* New list */
  if (list == 0)
    {
      ptr1 = NewLptr ((void *) p);
      new_lptr_added = 1;
      return ptr1;
    }

  /* New element is the largest */
  if (p > list->ptr)
    {
      ptr1 = NewLptr ((void *) p);
      new_lptr_added = 1;
      ptr1->next = list;
#if 0
      if (debug_lptr)
        fprintf (stderr, "%d %d\n", pos, length);
#endif
      return ptr1;
    }

  for (ptr1 = list; ptr1->next != 0; ptr1 = ptr1->next)
    {
#if 0
      if (debug_lptr)
        pos++;
#endif

      /* duplicated entry */
      if (ptr1->ptr == p)
        {
#if 0
          if (debug_lptr)
            fprintf (stderr, "%d %d\n", pos, length);
#endif
          return list;
        }

      /* found position */
      if (ptr1->ptr > p && ptr1->next->ptr < p)
        {
          ptr2 = NewLptr ((void *) p);
          new_lptr_added = 1;
          ptr2->next = ptr1->next;
          ptr1->next = ptr2;
#if 0
          if (debug_lptr)
            fprintf (stderr, "%d %d\n", pos, length);
#endif
          return list;
        }
    }

  if (ptr1->ptr != p)
    {
      /* New element is the smallest */
      ptr2 = NewLptr ((void *) p);
      new_lptr_added = 1;
      ptr1->next = ptr2;
#if 0
      if (debug_lptr)
        fprintf (stderr, "%d %d\n", pos, length);
#endif
      return list;
    }
#if 0
  if (debug_lptr)
    fprintf (stderr, "%d %d\n", pos, length);
#endif
  return list;
}

Lint AddToSortedLint (int p, Lint list)
{
  Lint ptr1, ptr2;

  if (!Lint_Pool)
    Init_NewLint ();

  /* New list */
  if (list == 0)
    {
      ptr1 = NewLint (p);
      return ptr1;
    }

  /* New element is the largest */
  if (p > list->id)
    {
      ptr1 = NewLint (p);
      ptr1->next = list;
      return ptr1;
    }

  for (ptr1 = list; ptr1->next != 0; ptr1 = ptr1->next)
    {

      /* duplicated entry */
      if (ptr1->id == p)
        return list;

      /* found position */
      if (ptr1->id > p && ptr1->next->id < p)
        {
          ptr2 = NewLint (p);
          ptr2->next = ptr1->next;
          ptr1->next = ptr2;
          return list;
        }
    }

  if (ptr1->id != p)
    {
      /* New element is the smallest */
      ptr2 = NewLint (p);
      ptr1->next = ptr2;
    }
  return list;
}

Lptr InSortedLptr (Lptr list, void * p)
{
  void *p1;
  while (list != 0)
    {
      p1 = list->ptr;
      if (p1 == p)
        return list;
      if (p1 < p)
        return 0;
      list = list->next;
    }
  return 0;
}

Lint InSortedLint (Lint list, int p)
{
  int p1;
  while (list != 0)
    {
      p1 = list->id;
      if (p1 == p)
        return list;
      if (p1 < p)
        return 0;
      list = list->next;
    }
  return 0;
}
/* BCC - 12/8/97 */
int
LintLength (Lint list)
{
  int count = 0;

  while (list != 0)
    {
      count++;
      list = list->next;
    }
  return count;
}

Lptr CopyLptr (Lptr list)
{
  Lptr new_ptr;
  if (list == 0)
    return 0;
  new_ptr = NewLptr (list->ptr);
  new_ptr->next = CopyLptr (list->next);
  return new_ptr;
}

Lptr FreeLptr (Lptr list)
{
#ifdef DEBUG_APPEND
  free_list = 1;
#endif
  d_lptr = AppendLptr (list, d_lptr);
#ifdef DEBUG_APPEND
  free_list = 0;
#endif
  return 0;
}

/* BCC - 4/3/97 */
Lptr InLptr (Lptr list, void * ptr)
{
  while (list != 0)
    {
      if (list->ptr == ptr)
        return list;
      list = list->next;
    }
  return 0;
}
/* BCC - 9/29/97 */
Lptr NameInLptr (Lptr list, char *ptr)
{
  while (list != 0)
    {
      if (!strcmp ((char *) list->ptr, ptr))
        return list;
      list = list->next;
    }
  return 0;
}
/* BCC - 12/8/97 */
int
LptrLength (Lptr list)
{
  int count = 0;

  while (list != 0)
    {
      count++;
      list = list->next;
    }
  return count;
}

/* BCC - 10/13/98 */
int
SameSortedLptr (Lptr list1, Lptr list2)
{
  for (; list1 && list2; list1 = list1->next, list2 = list2->next)
    {
      if (list1->ptr != list2->ptr)
        return 0;
    }
  if (list1 == 0 && list2 == 0)
    return 1;
  else
    return 0;
}

/* BCC - 12/17/98 */
void
CopySortedLptrList (Lptr * base, Lptr list)
{
  if (list == 0)
    return;
  CopySortedLptrList (base, list->next);
  *base = AddToSortedLptr (list->ptr, *base);
}

void
En_FIFO_LptrQ (LptrQ Q, void *element)
{
  Lptr lptr;

  lptr = NewLptr (element);
  if (Q->length == 0)
    {
      Q->head = Q->tail = lptr;
    }
  else
    {
      Q->tail->next = lptr;
      Q->tail = lptr;
    }
  Q->length++;
}

void
En_FILO_LptrQ (LptrQ Q, void *element)
{
  Lptr lptr;

  lptr = NewLptr (element);
  if (Q->length == 0)
    {
      Q->head = Q->tail = lptr;
    }
  else
    {
      lptr->next = Q->head;
      Q->head = lptr;
    }
  Q->length++;
}

void *
De_LptrQ (LptrQ Q)
{
  Lptr lptr;
  void *element;

  if (Q->length == 0)
    return 0;
  lptr = Q->head;
  Q->head = lptr->next;
  if (Q->head == 0)
    Q->tail = 0;
  element = lptr->ptr;
  lptr->next = 0;
  FreeLptr (lptr);
  Q->length--;
  return element;
}

/*
 * assume Q->length is initialized to 0;
 */
void *
Peek_LptrQ (LptrQ Q)
{
   if (Q->length == 0) return NULL;
   return Q->head->ptr;
}
/*----------------------------------------------------------------------*/
