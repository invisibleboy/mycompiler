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
 *      File :          list.c
 *      Description :   Functions to maintain lists.
 *      Creation Date : September 1996
 *      Author :        David August
 *
 *      Copyright (c) 1996 David August, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
 *===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdlib.h>
#include <library/i_error.h>
#include <library/l_alloc_new.h>
#include <library/i_global.h>
#include <library/i_list.h>

L_Alloc_Pool *ListPool = NULL;
L_Alloc_Pool *ListElementPool = NULL;

static List
List_new_list ()
{
  List new_list;

  if (ListPool == NULL)
    {
      ListPool = L_create_alloc_pool ("_List", sizeof (_List), 8);
    }

  new_list = (List) L_alloc (ListPool);
  new_list->size = 0;
  new_list->first = NULL;
  new_list->last = NULL;
  new_list->current = (ListElement *) malloc (1 * sizeof (ListElement));
  if (new_list->current == NULL)
    I_punt ("L_new_list: malloc out of space");
  new_list->current[0] = NULL;
  new_list->num_currents = 1;

  return new_list;
}

static ListElement
List_new_list_element ()
{
  ListElement new_list_element;

  if (ListElementPool == NULL)
    {
      ListElementPool =
        L_create_alloc_pool ("_ListElement", sizeof (_ListElement), 128);
    }

  new_list_element = (ListElement) L_alloc (ListElementPool);
  new_list_element->ptr = NULL;
  new_list_element->next = NULL;
  new_list_element->prev = NULL;

  return new_list_element;
}

int
List_size (List list)
{
  if (!list)
    return 0;

  return list->size;
}

void *
List_get_first (List list)
{
  if (!list)
    return NULL;

  return list->first->ptr;
}

void *
List_get_last (List list)
{
  if (!list)
    return NULL;

  return list->last->ptr;
}

void *
List_get_next_l (List list, int level)
{
  if (!list)
    return NULL;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  if (!list->current[level])
    {
      return list->first->ptr;
    }

  if (!list->current[level]->next)
    return NULL;

  return list->current[level]->next->ptr;
}

void *
List_get_next (List list)
{
  return List_get_next_l (list, 0);
}

void *
List_next_l (List list, int level)
{
  if (!list)
    return NULL;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  if (!list->current[level])
    {
      list->current[level] = list->first;
      if (!list->current[level])
	return NULL;
      return list->current[level]->ptr;
    }

  list->current[level] = list->current[level]->next;

  if (!list->current[level])
    return NULL;

  return list->current[level]->ptr;
}

void *
List_next (List list)
{
  if (!list)
    return NULL;

  if (!list->current[0])
    list->current[0] = list->first;
  else
    list->current[0] = list->current[0]->next;

  if (!list->current[0])
    return NULL;

  return list->current[0]->ptr;
}

void *
List_prev_l (List list, int level)
{
  if (!list)
    return NULL;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  if (!list->current[level])
    {
      list->current[level] = list->last;
      return list->current[level]->ptr;
    }

  list->current[level] = list->current[level]->prev;

  if (!list->current[level])
    return NULL;

  return list->current[level]->ptr;
}

void *
List_prev (List list)
{
  return List_prev_l (list, 0);
}


void
List_change_current_l (List list, void *new_ptr, int level)
{
  if (!list)
    return;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  if (!list->current[level])
    {
      return;
    }

  list->current[level]->ptr = new_ptr;
}

void
List_change_current (List list, void *new_ptr)
{
  List_change_current_l (list, new_ptr, 0);
}

void *
List_current_l (List list, int level)
{
  if (!list)
    return NULL;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  if (!list->current[level])
    {
      return NULL;
    }

  return list->current[level]->ptr;
}

void *
List_current (List list)
{
  return List_current_l (list, 0);
}

void *
List_first_l (List list, int level)
{
  if (!list)
    return NULL;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  list->current[level] = list->first;

  return list->current[level]->ptr;
}

void *
List_first (List list)
{
  return List_first_l (list, 0);
}


void *
List_last_l (List list, int level)
{
  if (!list)
    return NULL;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  list->current[level] = list->last;

  if (!list->current[level])
    return NULL;

  return list->current[level]->ptr;
}

void *
List_last (List list)
{
  return List_last_l (list, 0);
}

void
List_start_l (List list, int level)
{
  if (!list)
    return;

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  list->current[level] = NULL;
}

void
List_start (List list)
{
  List_start_l (list, 0);
}

List List_insert_first (List list, void *ptr)
{
  List new_list = list;
  ListElement tmp;

  if (!new_list)
    {
      new_list = List_new_list ();
    }
  tmp = new_list->first;
  new_list->first = List_new_list_element ();
  if (tmp)
    {
      tmp->prev = new_list->first;
      new_list->first->next = tmp;
    }
  else
    {
      new_list->last = new_list->first;
    }
  new_list->first->ptr = ptr;
  new_list->size++;

  return new_list;
}

List List_insert_last (List list, void *ptr)
{
  List new_list = list;
  ListElement tmp;

  if (!new_list)
    {
      new_list = List_new_list ();
    }
  tmp = new_list->last;
  new_list->last = List_new_list_element ();
  if (tmp)
    {
      tmp->next = new_list->last;
      new_list->last->prev = tmp;
    }
  else
    {
      new_list->first = new_list->last;
    }
  new_list->last->ptr = ptr;
  new_list->size++;

  return new_list;
}

List List_insert_prev_l (List list, void *ptr, int level)
{
  List new_list = list;
  ListElement tmp;
  ListElement new_element;

  if (!new_list)
    {
      new_list = List_new_list ();
    }

  if (level >= new_list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  /* 
   * When nothing is being pointed to, append to end of list 
   */
  if (!new_list->current[level])
    {
      tmp = new_list->last;
      new_list->last = List_new_list_element ();
      if (tmp)
        {
          tmp->next = new_list->last;
          new_list->last->prev = tmp;
        }
      else
        {
          new_list->first = new_list->last;
        }
      new_list->last->ptr = ptr;
      new_list->size++;

      return new_list;
    }

  tmp = new_list->current[level]->prev;
  new_element = List_new_list_element ();
  new_list->current[level]->prev = new_element;
  new_element->next = new_list->current[level];
  if (tmp)
    {
      tmp->next = new_element;
      new_element->prev = tmp;
    }
  else
    {
      new_list->first = new_element;
    }
  new_element->ptr = ptr;
  new_list->size++;

  return new_list;
}

List List_insert_prev (List list, void *ptr)
{
  return List_insert_prev_l (list, ptr, 0);
}

List List_insert_next_l (List list, void *ptr, int level)
{
  List new_list = list;
  ListElement tmp;
  ListElement new_element;

  if (!new_list)
    {
      new_list = List_new_list ();
    }

  if (level >= new_list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  /* 
   * When nothing is being pointed to, append to end of list 
   */
  if (!new_list->current[level])
    {
      tmp = new_list->last;
      new_list->last = List_new_list_element ();
      if (tmp)
        {
          tmp->next = new_list->last;
          new_list->last->prev = tmp;
        }
      else
        {
          new_list->first = new_list->last;
        }
      new_list->last->ptr = ptr;
      new_list->size++;

      return new_list;
    }

  tmp = new_list->current[level]->next;
  new_element = List_new_list_element ();
  new_list->current[level]->next = new_element;
  new_element->prev = new_list->current[level];
  if (tmp)
    {
      tmp->prev = new_element;
      new_element->next = tmp;
    }
  else
    {
      new_list->last = new_element;
    }
  new_element->ptr = ptr;
  new_list->size++;

  return new_list;
}

List List_insert_next (List list, void *ptr)
{
  return List_insert_next_l (list, ptr, 0);
}

List List_append (List list, List append_list)
{
  if (!list || list->size == 0)
    {
      List_reset (list);
      return append_list;
    }

  if (!append_list || append_list->size == 0)
    {
      List_reset (append_list);
      return list;
    }

  list->size += append_list->size;
  list->last->next = append_list->first;
  append_list->first->prev = list->last;

  list->last = append_list->last;

  return list;
}


List List_delete_current_l (List list, int level)
{
  ListElement elem_old_curr;
  ListElement elem_old_next;
  ListElement elem_old_prev;
  int indx;

  if (!list)
    {
      return NULL;
    }

  if (level >= list->num_currents || level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  if (!list->current[level])
    {
      return list;
    }

  elem_old_curr = list->current[level];
  elem_old_next = elem_old_curr->next;
  elem_old_prev = elem_old_curr->prev;


  /* Update all current pointers */
  for (indx = 0; indx < list->num_currents; indx++)
    {
      if (list->current[indx] == elem_old_curr)
        {
          list->current[indx] = elem_old_prev;
        }
    }
  if (elem_old_next)
    {
      elem_old_next->prev = elem_old_prev;
    }
  else
    {
      list->last = elem_old_prev;
    }
  if (elem_old_prev)
    {
      elem_old_prev->next = elem_old_next;
    }
  else
    {
      list->first = elem_old_next;
    }

  L_free (ListElementPool, elem_old_curr);
  list->size--;
  if (list->size == 0 && list->num_currents == 1)
    {
      return List_reset (list);
    }
  return list;
}

List List_delete_current (List list)
{
  return List_delete_current_l (list, 0);
}


List List_reset (List list)
{
  ListElement elem_ptr;
  ListElement elem_tmp;

  if (!list)
    {
      return NULL;
    }

  elem_ptr = list->first;
  while (elem_ptr)
    {
      elem_tmp = elem_ptr;
      elem_ptr = elem_ptr->next;
      L_free (ListElementPool, elem_tmp);
    }
  free (list->current);
  list->current = NULL;
  L_free (ListPool, list);
  return NULL;
}

List List_reset_free_ptrs (List list, void (*free_fn) (void *))
{
  ListElement elem_ptr;
  ListElement elem_tmp;

  if (!list)
    {
      return NULL;
    }

  elem_ptr = list->first;
  while (elem_ptr)
    {
      elem_tmp = elem_ptr;
      elem_ptr = elem_ptr->next;
      (*free_fn) (elem_tmp->ptr);
      L_free (ListElementPool, elem_tmp);
    }
  free (list->current);
  L_free (ListPool, list);
  return NULL;
}

void
List_copy_current_ptr (List list, int to_level, int from_level)
{
  if (to_level >= list->num_currents || to_level < 0 ||
      from_level >= list->num_currents || from_level < 0)
    {
      I_punt ("List: Invalid pointer level");
    }

  list->current[to_level] = list->current[from_level];
}

extern int
List_max_registered_current(List list)
{
  return (list->num_currents - 1);
}

extern int
List_register_new_ptr (List list)
{
  ListElement *new_current;
  int indx;

  new_current =
    (ListElement *) malloc (sizeof (ListElement) * (list->num_currents + 1));
  if (new_current == NULL)
    I_punt ("L_register_new_ptr: malloc out of space");

  for (indx = 0; indx < list->num_currents; indx++)
    {
      new_current[indx] = list->current[indx];
    }
  new_current[list->num_currents] = NULL;
  list->num_currents++;
  free (list->current);
  list->current = new_current;

  return (list->num_currents - 1);
}

extern void
List_free_all_ptrs (List list)
{
  ListElement current;

  current = list->current[0];
  free (list->current);
  list->current = (ListElement *) malloc (sizeof (ListElement));
  if (list->current == NULL)
    I_punt ("L_free_all_ptrs: malloc out of space");

  list->current[0] = current;
  list->num_currents = 1;
}


List List_remove (List list, void *ptr)
{
  ListElement elem_old_curr;
  ListElement elem_old_next;
  ListElement elem_old_prev;
  int indx;

  if (!list)
    {
      return NULL;
    }

  for (elem_old_curr = list->first; elem_old_curr;
       elem_old_curr = elem_old_curr->next)
    {
      if (elem_old_curr->ptr == ptr)
        break;
    }
  if (!elem_old_curr)
    I_punt ("List_find_and_delete: Element not found.\n");


  elem_old_next = elem_old_curr->next;
  elem_old_prev = elem_old_curr->prev;

  /* Update all current pointers */
  for (indx = 0; indx < list->num_currents; indx++)
    {
      if (list->current[indx] == elem_old_curr)
        {
          list->current[indx] = elem_old_prev;
        }
    }
  if (elem_old_next)
    {
      elem_old_next->prev = elem_old_prev;
    }
  else
    {
      list->last = elem_old_prev;
    }
  if (elem_old_prev)
    {
      elem_old_prev->next = elem_old_next;
    }
  else
    {
      list->first = elem_old_next;
    }

  L_free (ListElementPool, elem_old_curr);
  list->size--;
  if (list->size == 0 && list->num_currents == 1)
    {
      return List_reset (list);
    }
  return list;
}

List
List_copy (List list)
{
  void *ptr;
  List ret = NULL;
  
  for (ptr = List_first (list); ptr; ptr = List_next (list))
    ret = List_insert_last (ret, ptr);

  return ret;
}


int
List_member (List list, void *ptr)
{
  ListElement elem_curr;

  if (!list)
    return 0;

  for (elem_curr = list->first; elem_curr; elem_curr = elem_curr->next)
    {
      if (elem_curr->ptr == ptr)
        break;
    }
  if (!elem_curr)
    return 0;

  return 1;
}
