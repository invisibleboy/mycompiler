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
 *      File :          hash.c
 *      Description :   Functions to maintain hash tables.
 *      Creation Date : September 1996
 *      Author :        Richard Hank and David August
 *
 *      Copyright (c) 1996 Richard Hank, David August, Wen-mei Hwu and 
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
#include <library/i_hash.h>


L_Alloc_Pool *HashElementPool = NULL;
L_Alloc_Pool *HashTablePool = NULL;

HashTable HashTable_create (int size)
{
  int indx;
  HashTable hash_table;

  for (indx = 3; indx <= 31; indx++)
    {
      if (size <= (1 << indx))
        {
          size = (1 << indx);
          break;
        }
    }

  if (HashTablePool == NULL)
    {
      HashTablePool =
        L_create_alloc_pool ("_HashTable", sizeof (_HashTable), 8);
    }
  if (HashElementPool == NULL)
    {
      HashElementPool =
        L_create_alloc_pool ("_HashElement", sizeof (_HashElement), 512);
    }
  hash_table = (HashTable) L_alloc (HashTablePool);

  hash_table->tbl = (HashElement *) malloc (sizeof (HashElement) * size);
  hash_table->mask = size - 1;

  for (indx = 0; indx < size; indx++)
    hash_table->tbl[indx] = NULL;

  return hash_table;
}

void
HashTable_free (HashTable hash_table)
{
  if (!hash_table)
    return;

  HashTable_reset (hash_table);
  free (hash_table->tbl);
  L_free (HashTablePool, hash_table);
}

void
HashTable_free_func (HashTable hash_table, void (*ptr_free_func) (void *))
{
  if (!hash_table)
    return;

  HashTable_reset_func (hash_table, ptr_free_func);
  free (hash_table->tbl);
  L_free (HashTablePool, hash_table);
}

void
HashTable_reset (HashTable hash_table)
{
  int indx;

  if (!hash_table)
    return;

  for (indx = 0; indx <= hash_table->mask; indx++)
    {
      HashElement element = hash_table->tbl[indx];
      while (element)
        {
          HashElement tmp = element->next;
          L_free (HashElementPool, element);
          element = tmp;
        }
      hash_table->tbl[indx] = NULL;
    }
}

void
HashTable_reset_func (HashTable hash_table, void (*ptr_free_func) (void *))
{
  int indx;

  if (!hash_table)
    return;

  for (indx = 0; indx <= hash_table->mask; indx++)
    {
      HashElement element = hash_table->tbl[indx];
      while (element)
        {
          HashElement tmp = element->next;
          ptr_free_func (element->ptr);
          L_free (HashElementPool, element);
          element = tmp;
        }
      hash_table->tbl[indx] = NULL;
    }
}

void *
HashTable_find (HashTable hash_table, int key)
{
  unsigned int hash_val = HASH_FUNC (key) & hash_table->mask;
  HashElement element = hash_table->tbl[hash_val];

  while (element != NULL)
    {
      if (element->key == key)
        return (element->ptr);
      element = element->next;
    }
  I_punt ("Hash Table Element not found.");

  return (NULL);
}

void *
HashTable_find_or_null (HashTable hash_table, int key)
{
  unsigned int hash_val = HASH_FUNC (key) & hash_table->mask;
  HashElement element = hash_table->tbl[hash_val];

  while (element != NULL)
    {
      if (element->key == key)
        return (element->ptr);
      element = element->next;
    }

  return (NULL);
}

int
HashTable_member (HashTable hash_table, int key)
{
  unsigned int hash_val = HASH_FUNC (key) & hash_table->mask;
  HashElement element = hash_table->tbl[hash_val];

  while (element != NULL)
    {
      if (element->key == key)
        return (TRUE);
      element = element->next;
    }

  return (FALSE);
}

void
HashTable_insert (HashTable hash_table, int key, void *ptr)
{
  unsigned int hash_val;
  HashElement element;

  hash_val = HASH_FUNC (key) & hash_table->mask;

  element = (HashElement) L_alloc (HashElementPool);
  element->key = key;
  element->ptr = ptr;

  element->next = hash_table->tbl[hash_val];
  hash_table->tbl[hash_val] = element;
}

/* This does not free anything pointed to by element->ptr. */
void
HashTable_remove (HashTable hash_table, int key)
{
  unsigned int hash_val = HASH_FUNC (key) & hash_table->mask;
  HashElement element = hash_table->tbl[hash_val];
  HashElement prev_element = NULL;

  while (element != NULL)
    {
      if (element->key == key)
        {
          if (prev_element != NULL)
            {
              prev_element->next = element->next;
            }
          else
            {
              hash_table->tbl[hash_val] = element->next;
            }
          L_free (HashElementPool, element);
          return;
        }
      prev_element = element;
      element = element->next;
    }
  I_punt ("Hash Table Element not found.");

}

/* Don't forget to free original */
void
HashTable_update (HashTable hash_table, int key, void *ptr)
{
  unsigned int hash_val = HASH_FUNC (key) & hash_table->mask;
  HashElement element = hash_table->tbl[hash_val];

  while (element != NULL)
    {
      if (element->key == key)
        {
          /* Found */
          element->ptr = ptr;
          return;
        }
      element = element->next;
    }

  /* Not found, so insert */
  element = (HashElement) L_alloc (HashElementPool);
  element->key = key;
  element->ptr = ptr;

  element->next = hash_table->tbl[hash_val];
  hash_table->tbl[hash_val] = element;
}

void
HashTable_start (HashTable hash_table)
{
  hash_table->key_indx = -1;
  hash_table->elem_indx = NULL;
}


void *
HashTable_next (HashTable hash_table)
{
  HashElement element;
  int indx;

  if (hash_table->elem_indx && hash_table->elem_indx->next)
    {
      hash_table->elem_indx = hash_table->elem_indx->next;
      return hash_table->elem_indx->ptr;
    }

  for (indx = hash_table->key_indx + 1; indx <= hash_table->mask; indx++)
    {
      element = hash_table->tbl[indx];
      if (element)
        {
          hash_table->key_indx = indx;
          hash_table->elem_indx = element;
          return hash_table->elem_indx->ptr;
        }
    }
  hash_table->key_indx = -1;
  hash_table->elem_indx = NULL;
  return NULL;
}

int
HashTable_key (HashTable hash_table)
{
  return (hash_table->elem_indx ? hash_table->elem_indx->key : -1);
}
