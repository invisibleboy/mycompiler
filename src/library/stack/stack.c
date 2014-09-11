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
 *
 *      File :          stack.c 
 *      Description :   Stack library 
 *      Creation Date : Mar 11, 1993
 *      Author :        Richard Hank
 *
 * Revision 1.2  94/03/16  20:51:53  20:51:53  hank (Richard E. Hank)
 * Generalize stack library and fix numerous bugs
 * 
 * Revision 1.1  93/05/15  20:12:21  20:12:21  hank (Richard E. Hank)
 * Initial revision
 * 
 *===========================================================================*/
/*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/l_alloc_new.h>
#include <library/stack.h>

static L_Alloc_Pool *stackPool = NULL;
static L_Alloc_Pool *stackElmtPool = NULL;

Stack *
New_Stack (void)
{
  Stack *stack;

  if (stackPool == NULL)
    {
      stackPool = L_create_alloc_pool ("Stack", sizeof (Stack), 32);
      stackElmtPool =
	L_create_alloc_pool ("StackElmt", sizeof (StackElmt), 512);
    }

  stack = (Stack *) L_alloc (stackPool);

  stack->ptr = NULL;
  stack->top = NULL;
  stack->bottom = NULL;

  return (stack);
}

StackElmt *
New_Stack_Elmt (void)
{
  return ((StackElmt *) L_alloc (stackElmtPool));
}

void
Free_Stack_Elmt (StackElmt * ptr)
{
  L_free (stackElmtPool, ptr);
}

Stack *
Free_Stack (Stack * stack)
{
  StackElmt *next;
  StackElmt *ptr = stack->top;

  while (ptr != NULL)
    {
      next = ptr->next;
      L_free (stackElmtPool, ptr);
      ptr = next;
    }
  L_free (stackPool, stack);
  return (NULL);
}

void
Clear_Stack (Stack * stack)
{
  StackElmt *next;
  StackElmt *ptr;

  if (stack == NULL)
    return;

  ptr = stack->top;
  while (ptr != NULL)
    {
      next = ptr->next;
      L_free (stackElmtPool, ptr);
      ptr = next;
    }

  stack->top = stack->bottom = NULL;
}

void
Push_Top (Stack * stack, void *ptr)
{
  StackElmt *elmt = (StackElmt *) L_alloc (stackElmtPool);
  elmt->ptr = ptr;

  if (stack->top != NULL)
    {
      stack->top->prev = elmt;
      elmt->next = stack->top;
      elmt->prev = NULL;
      stack->top = elmt;
    }
  else
    {
      stack->top = elmt;
      stack->bottom = elmt;
      elmt->prev = NULL;
      elmt->next = NULL;
    }
}

void
Push_Bot (Stack * stack, void *ptr)
{
  StackElmt *elmt = (StackElmt *) L_alloc (stackElmtPool);
  elmt->ptr = ptr;

  if (stack->top != NULL)
    {
      stack->bottom->next = elmt;
      elmt->next = NULL;
      elmt->prev = stack->bottom;
      stack->bottom = elmt;
    }
  else
    {
      stack->top = elmt;
      stack->bottom = elmt;
      elmt->prev = NULL;
      elmt->next = NULL;
    }
}

void *
Pop (Stack * stack)
{
  StackElmt *pop;
  void *ptr;

  if (stack->top != NULL)
    {
      pop = stack->top;
      stack->top = stack->top->next;
      if (stack->top)
	stack->top->prev = NULL;
      else
	stack->bottom = NULL;
      ptr = pop->ptr;
      L_free (stackElmtPool, pop);
      return (ptr);
    }
  return ((void *) -1);
}

void *
Stack_Top (Stack * stack)
{
  if (stack->top != NULL)
    return (stack->top->ptr);
  else
    return ((void *) -1);
}

void *
Stack_Elmt_Data (StackElmt * elmt)
{
  if (elmt)
    return (elmt->ptr);
  else
    return (NULL);
}

StackElmt *
Stack_Find (Stack * stack, void *ptr)
{
  StackElmt *tmp = stack->top;

  while (tmp)
    {
      if (tmp->ptr == ptr)
	return (tmp);
      tmp = tmp->next;
    }
  return (NULL);
}

StackElmt *
Stack_Ptr_Reset (Stack * stack)
{
  stack->ptr = stack->top;
  return (stack->ptr);
}

void
Stack_Ptr_Set (Stack * stack, StackElmt * ptr)
{
  if (ptr)
    stack->ptr = ptr;
}

StackElmt *
Stack_Find_Next (Stack * stack)
{
  StackElmt *tmp = stack->ptr;

  if (tmp != NULL)
    {
      tmp = stack->ptr->next;
      stack->ptr = tmp;
    }
  return (tmp);
}

int
Stack_Is_Bottom (Stack * stack)
{
  return (stack->ptr != NULL && stack->ptr == stack->bottom);
}

void
Delete_Stack (Stack * stack, StackElmt * ptr)
{
  StackElmt *prev = ptr->prev;
  StackElmt *next = ptr->next;

  if (next)
    if (prev)
      {
	next->prev = prev;
	prev->next = next;
      }
    else
      {
	stack->top = next;
	next->prev = NULL;
      }
  else
    {
      if (prev)
	{
	  prev->next = NULL;
	  stack->bottom = prev;
	}
      else
	stack->top = stack->bottom = NULL;

    }
  L_free (stackElmtPool, ptr);
}

void
Stack_Print (Stack * stack)
{
  StackElmt *tmp = stack->top;

  while (tmp != NULL)
    {
#ifdef LP64_ARCHITECTURE
      fprintf (stdout, " %ld", (long) tmp->ptr);
#else
      fprintf (stdout, " %d", (int) tmp->ptr);
#endif
      tmp = tmp->next;
    }
}
