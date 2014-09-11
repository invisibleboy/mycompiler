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
 *	File :		l_stack.c
 *	Description :	Generic stack manipulation functions
 *	Creation Date :	February 1994
 *	Authors : 	Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/l_alloc_new.h>
#include "lb_hb_stack.h"

static L_Alloc_Pool *Stack_pool = NULL;
static L_Alloc_Pool *Stack_item_pool = NULL;


static void
Stack_punt (char *message)
{
  fprintf (stderr, "Error in stack library: %s\n", message);
  exit (-1);
}

/*
 *    Allocation/Deallocation routines
 */

Stack_item *
Stack_item_create (void)
{
  Stack_item *s_item;

  if (Stack_pool == NULL)
    {
      Stack_pool = L_create_alloc_pool ("Stack", sizeof (Stack), 1);
      Stack_item_pool =
	L_create_alloc_pool ("Stack_item", sizeof (Stack_item), 16);
    }

  s_item = (Stack_item *) L_alloc (Stack_item_pool);

  s_item->data = 0;
  s_item->prev_item = NULL;
  s_item->next_item = NULL;

  return (s_item);
}

Stack *
Stack_create (void)
{
  Stack *s;

  if (Stack_pool == NULL)
    {
      Stack_pool = L_create_alloc_pool ("Stack", sizeof (Stack), 1);
      Stack_item_pool =
	L_create_alloc_pool ("Stack_item", sizeof (Stack_item), 16);
    }

  s = (Stack *) L_alloc (Stack_pool);
  s->head = NULL;
  s->tail = NULL;
  s->size = 0;

  return (s);
}

void
Stack_item_delete (Stack_item * s_item)
{
  L_free (Stack_item_pool, s_item);
}

void
Stack_delete (Stack * s)
{
  Stack_item *ptr, *temp_ptr;

  if (s->size != 0)
    fprintf (stderr, "Warning: stack library, deleting non-empty stack\n");

  ptr = s->head;
  while (ptr != NULL)
    {
      temp_ptr = ptr;
      ptr = ptr->next_item;
      Stack_item_delete (temp_ptr);
    }

  L_free (Stack_pool, s);
}

void
Stack_print (FILE * F, Stack * s)
{
  Stack_item *tmp;

  fprintf (F, "Stack contents: ");
  if (s == NULL)
    {
      fprintf (F, "\n");
    }
  else
    {
      for (tmp = s->head; tmp != NULL; tmp = tmp->next_item)
	{
	  fprintf (F, "%d ", tmp->data);
	}
      fprintf (F, "\n");
    }
}

int
Stack_size (Stack * s)
{
  return (s->size);
}

void
Stack_clear (Stack * s)
{
  Stack_item *ptr, *next;

  for (ptr = s->head; ptr != NULL; ptr = next)
    {
      next = ptr->next_item;
      Stack_item_delete (ptr);
    }

  s->head = NULL;
  s->tail = NULL;
  s->size = 0;
}

int
Stack_in (Stack * s, int data)
{
  Stack_item *ptr;

  ptr = s->head;
  while (ptr != NULL)
    {
      if (ptr->data == data)
	return (1);
      ptr = ptr->next_item;
    }
  return (0);
}

int
Stack_first (Stack * s)
{
  if (s->size == 0)
    Stack_punt ("Stack_first: stack is empty");

  return (s->head->data);
}

int
Stack_last (Stack * s)
{
  if (s->size == 0)
    Stack_punt ("Stack_last: stack is empty");
  return (s->tail->data);
}

/*
 *    Note this routine allocates memory!!!
 */
int
Stack_get_contents (Stack * s, int **buf)
{
  int count;
  Stack_item *ptr;

  count = 0;
  for (ptr = s->head; ptr != NULL; ptr = ptr->next_item)
    {
      count++;
    }

  if (*buf != NULL)
    Stack_punt ("Stack_get_contents: buf is not NULL ptr");
  *buf = (int *) Lcode_malloc (sizeof (int) * count);
  if (*buf == NULL)
    Stack_punt ("Stack_get_contents: malloc out of space");

  count = 0;
  /* put them in array in reverse order */
  for (ptr = s->tail; ptr != NULL; ptr = ptr->prev_item)
    {
      (*buf)[count] = ptr->data;
      count++;
    }

  return (count);
}

Set
Stack_get_content_set (Stack * s)
{
  Set set;
  Stack_item *ptr;

  set = NULL;

  for (ptr = s->head; ptr != NULL; ptr = ptr->next_item)
    set = Set_add (set, ptr->data);

  return set;
}

void
Stack_push_top (Stack * s, int data)
{
  Stack_item *ptr;

  ptr = Stack_item_create ();
  ptr->data = data;

  if (s->size > 0)
    {
      ptr->next_item = s->head;
      s->head->prev_item = ptr;
      s->head = ptr;
    }
  else
    {
      s->head = ptr;
      s->tail = ptr;
    }

  (s->size)++;
}

void
Stack_push_bottom (Stack * s, int data)
{
  Stack_item *ptr;

  ptr = Stack_item_create ();
  ptr->data = data;

  if (s->size > 0)
    {
      ptr->prev_item = s->tail;
      s->tail->next_item = ptr;
      s->tail = ptr;
    }
  else
    {
      s->head = ptr;
      s->tail = ptr;
    }

  (s->size)++;
}

int
Stack_pop (Stack * s)
{
  int data;
  Stack_item *ptr;

  if (s->size == 0)
    Stack_punt ("Stack_delete: size is 0");

  (s->size)--;
  ptr = s->head;
  data = ptr->data;

  if (s->size > 0)
    {
      s->head = s->head->next_item;
      s->head->prev_item = NULL;
    }
  else
    {
      s->head = NULL;
      s->tail = NULL;
    }

  Stack_item_delete (ptr);

  return (data);
}
