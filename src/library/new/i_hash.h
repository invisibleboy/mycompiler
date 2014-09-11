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
 *      File :          hash.h
 *      Description :   Hash table function header file.
 *      Creation Date : September 1996
 *      Author :        David August
 *
 *      Copyright (c) 1996 David August, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
 *===========================================================================*/

#ifndef HASH_H
#define HASH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/l_alloc_new.h>

#define HASH_FUNC(value) ((value))
/*
#define HASH_FUNC(value) ((value)>>24 + (value)>>16 + (value)>>8 + (value))
*/
typedef struct _hash_element {
  int key;
  void *ptr;
  struct _hash_element *next;
} *HashElement, _HashElement;

typedef struct _hash_table {
  unsigned int mask;
  HashElement *tbl;

  int key_indx;
  HashElement elem_indx;
} *HashTable, _HashTable;

#ifdef __cplusplus
extern "C"
{
#endif

/*====================
 * Hash Table Globals
 *====================*/
  extern L_Alloc_Pool *HashTablePool;
  extern L_Alloc_Pool *HashElementPool;

/*====================
 * Function Prototypes
 *====================*/
  extern HashTable HashTable_create (int size);
  extern void HashTable_free (HashTable hash_table);
  extern void HashTable_free_func (HashTable hash_table,
                                   void (*ptr_free_func) (void *));
  extern void HashTable_reset (HashTable hash_table);
  extern void HashTable_reset_func (HashTable hash_table,
                                    void (*ptr_free_func) (void *));
  extern void *HashTable_find (HashTable hash_table, int key);
  extern void *HashTable_find_or_null (HashTable hash_table, int key);
  extern void HashTable_insert (HashTable hash_table, int key, void *ptr);
  extern int HashTable_member (HashTable hash_table, int key);
  extern void HashTable_update (HashTable hash_table, int key, void *ptr);

  extern void HashTable_start (HashTable hash_table);
  extern void *HashTable_next (HashTable hash_table);
  extern int HashTable_key (HashTable hash_table);

  extern void HashTable_remove (HashTable hash_table, int key);

#ifdef __cplusplus
}
#endif

#endif
