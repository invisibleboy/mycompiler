/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    dyn_array.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "dyn_array.h"

/*******************************************************************
 * Dynamically sizing array
 *******************************************************************/

void
DYNA_reset (DYNA_t * darray)
{
  if (darray->ptr)
    free (darray->ptr);
  darray->ptr = NULL;
  darray->maxindex = 0;
  darray->el_size = 0;
  darray->max_assigned_index = -1;
}

void
DYNA_init (DYNA_t * darray, int maxindex, int el_size)
{
  if (maxindex <= 0)
    I_punt ("init_dynarray: initial index count must be > 0\n");
  if (el_size <= 0)
    I_punt ("init_dynarray: initial element size must be > 0\n");

  darray->ptr = NULL;
  DYNA_reset (darray);
  darray->el_size = el_size;
  darray->maxindex = maxindex;
  darray->ptr = calloc (maxindex * el_size, 1);
}

DYNA_t *
DYNA_new (int maxindex, int el_size)
{
  DYNA_t *tmp = (DYNA_t *) malloc (sizeof (DYNA_t));
  DYNA_init (tmp, maxindex, el_size);
  return tmp;
}

DYNA_t *
DYNA_copy (DYNA_t * darray)
{
  DYNA_t *tmp;

  if (darray)
    {
      tmp = DYNA_new (darray->maxindex, darray->el_size);
      memcpy (tmp->ptr, darray->ptr, darray->maxindex * darray->el_size);
      tmp->max_assigned_index = darray->max_assigned_index;

      return tmp;
    }

  return NULL;
}

void
DYNA_free (DYNA_t * darray)
{
  DYNA_reset (darray);
  free (darray);
}

void
DYNA_pad (DYNA_t * darray)
{
  int newmax;
  if (darray->max_assigned_index + 2 > darray->maxindex)
    {
      newmax = darray->maxindex;
      if ((newmax * darray->el_size) < (100 * 1024 * 1024))
        newmax *= 2;
      else
        newmax *= 1.1;

      darray->ptr = realloc (darray->ptr, (newmax * darray->el_size));
      /* Zero out new portion */
      bzero (((char *)darray->ptr + (darray->maxindex * darray->el_size)),
             ((newmax - darray->maxindex) * darray->el_size));
      darray->maxindex = newmax;
    }
}

void
DYNA_add_element (DYNA_t * darray, int index, void *element)
{
  int newmax;
  int freeit = 0;

  if (!darray->ptr)
    I_punt ("add_element: ptr is NULL\n");

  /* Make sure space exists for the new element 
   */
  if (index >= darray->maxindex)
    {
      newmax = darray->maxindex;
      while (index >= newmax)
        {
          if ((newmax * darray->el_size) < (100 * 1024 * 1024))
            newmax *= 2;
          else
            newmax *= 1.1;
        }

      darray->ptr = realloc (darray->ptr, (newmax * darray->el_size));
      /* Zero out new portion */
      bzero (((char *)darray->ptr + (darray->maxindex * darray->el_size)),
             ((newmax - darray->maxindex) * darray->el_size));
      darray->maxindex = newmax;
    }

  /* Now set the element 
   */
  if (element == NULL)
    {
      element = calloc (1, darray->el_size);
      freeit = 1;
    }

  memcpy (((char *)darray->ptr + (index * darray->el_size)), element,
          darray->el_size);
  if (darray->max_assigned_index < index)
    darray->max_assigned_index = index;

  if (freeit)
    {
      free (element);
    }
}

void
DYNA_shrink_to (DYNA_t * darray, int new_max)
{
  if (darray->max_assigned_index > new_max)
    darray->max_assigned_index = new_max;
}

void
DYNA_shrink_one (DYNA_t * darray)
{
  darray->max_assigned_index--;
}

int
DYNA_append_element (DYNA_t * darray, void *element)
{
  int index = darray->max_assigned_index + 1;
  DYNA_add_element (darray, index, element);
  return index;
}

void
DYNA_delete_element (DYNA_t * darray, int index)
{
  int i;
  for (i = index; i < darray->max_assigned_index; i++)
    {
      memcpy (((char *)darray->ptr + (i * darray->el_size)),
              ((char *)darray->ptr + ((i + 1) * darray->el_size)), 
	      darray->el_size);
    }
  darray->max_assigned_index--;
}

void
DYNA_rmlast_element (DYNA_t * darray)
{
  darray->max_assigned_index--;
}

void *
DYNA_get_element (DYNA_t * darray, int index)
{
  if (index >= darray->maxindex || index > darray->max_assigned_index)
    I_punt ("get_element: index too large\n");
  if (index < 0)
    I_punt ("get_element: index < 0\n");

  return ((char *)darray->ptr + (darray->el_size * index));
}

void *
DYNA_get_last_element (DYNA_t * darray)
{
  return ((char *)darray->ptr + 
	  (darray->el_size * darray->max_assigned_index));
}

int
DYNA_get_indexbound (DYNA_t * darray)
{
  if (darray == NULL)
    {
      return 0;
    }

  return darray->max_assigned_index + 1;
}

int
DYNA_find_ptr (DYNA_t * darray, void *ptr)
{
  void **ptr_iter = (void **) darray->ptr;
  int i;

  if (darray->el_size != sizeof (void *))
    I_punt ("DYNA_find_ptr: unexpected ptr size\n");

  for (i = 0; i <= darray->max_assigned_index; i++)
    {
      if (ptr_iter[i] == ptr)
        return i;
    }

  I_punt ("DYNA_find_ptr: not found\n");
  return -1;
}
