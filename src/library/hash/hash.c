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
 *      File:   hash.c
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================*\
 *      The internal hash table structure is implementation
 *      specific. In this implementation, a straight-forward hash table is
 *      implemented as a hash table.
 *
\*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <library/hash.h>

#define MAX     100             /* at most 100 symbol tables */

typedef struct _Tbl
{
  Entry array;                  /* hash entry array */
  int size;                     /* table size */
}
_Tbl, *Tbl;

static _Tbl TBL[MAX];           /* hash tables */
static int next_tbl = 0;        /* next hash table */

/* print error message and punt. */
static void
error (char *mesg)
{
  fprintf (stderr, "hash table error : %s\n", mesg);
  exit (-1);
}

/* Compute the hash address. Return a number between 0 and size. 
 * Hash either the symbol is found, or when an INVALID_ENTRY
 * is found.
 */
static int
Hash (Tbl tbl, int key, int type)
                /* hash table */
                /* hash key */
                /* entry type */
{
  Entry table;
  register int hash_size, hash_pos, increment, trial;
  /* Quadratic probing */
  /* 1: initialize hash table parameters */
  table = tbl->array;
  hash_size = tbl->size;
  /* 2: compute initial hash position */
  hash_pos = key % hash_size;
  /* 3: iterate until exit condition is met */
  increment = 1;
  for (trial = 0; trial < hash_size; trial++)
    {
      /* 4: test if there is a hit */
      if ((table[hash_pos].type == type) && (table[hash_pos].key == key))
        {
          return hash_pos;
        }
      else
        /* 5: test if there is a miss */
      if (table[hash_pos].type == INVALID_ENTRY)
        {
          return hash_pos;
        }
      /* 6: else rehash */
      hash_pos = (hash_pos + increment) % hash_size;
      increment++;
    }
  /* went into a recursive loop, apparently the table
   * is not designed correctly.
   */
  fprintf (stderr, "# hash table size = %d\n", hash_size);
  fprintf (stderr, "# number of trials = %d\n", trial);
  error ("hash: too many hash steps");
  return -1;
}
/** Create a new symbol table. **/
int
NewHashTbl (int max_size)
{
  int new_id, i;
  Tbl T;
        /** get next table */
  new_id = next_tbl++;
  if (new_id >= MAX)
    {
      error ("hash: run out of hash tables");
      return -1;
    }
  T = TBL + new_id;
  T->size = max_size;
  T->array = (Entry) malloc (sizeof (_Entry) * T->size);
  if (T->array == 0)
    {
      error ("hash: failed to allocate hash table array");
      return -1;
    }
  for (i = 0; i < T->size; i++)
    {
      T->array[i].key = 0;
      T->array[i].type = INVALID_ENTRY;
      T->array[i].value = 0;
      T->array[i].ptr = 0;
    }
  return new_id;
}
/** Add a new entry in the hash table. */
Entry AddEntry (int tbl_id, int key, int type)
{
  int index;
  Tbl tbl = TBL + tbl_id;
  Entry sym;
  if ((tbl_id < 0) || (tbl_id > next_tbl))
    error ("hash: in AddHash: illegal tbl_id");
  if (type == INVALID_ENTRY)
    error ("hash: in AddHash: illegal type");
  index = Hash (tbl, key, type);
  sym = tbl->array + index;
  /* create a new symbol if necessary */
  if (sym->type == INVALID_ENTRY)
    {
      sym->key = key;
      sym->type = type;
      sym->value = 0;
      sym->ptr = 0;
    }
  return sym;
}
/** Find an entry in the hash table. */
Entry FindEntry (int tbl_id, int key, int type)
{
  int index;
  Tbl tbl = TBL + tbl_id;
  Entry sym;
  if ((tbl_id < 0) || (tbl_id > next_tbl))
    error ("hash: in FindHash: illegal tbl_id");
  if (type == INVALID_ENTRY)
    error ("hash: in FindHash: illegal type");
  index = Hash (tbl, key, type);
  sym = tbl->array + index;
  /* if not found, return 0 */
  if (sym->type == INVALID_ENTRY)
    return 0;
  else
    return sym;
}
/** Completely erase a hash table **/
void
ClearHashTbl (int tbl_id)
{
  int i;
  Tbl tbl = TBL + tbl_id;
  if ((tbl_id < 0) || (tbl_id > next_tbl))
    error ("hash: in ClearHashTbl: illegal tbl_id");
  for (i = 0; i < tbl->size; i++)
    {
      tbl->array[i].key = 0;
      tbl->array[i].type = INVALID_ENTRY;
      tbl->array[i].value = 0;
      tbl->array[i].ptr = 0;
    }
}

#ifdef DEBUG_HASH
void
pr (Entry sym)
{
  if (sym == 0)
    fprintf (stderr, "()\n");
  else
    fprintf (stderr, "(key=%d, type=%d, value=%d, ptr=%d)\n",
             sym->key, sym->type, sym->value, sym->ptr);
}
void
main ()
{
  int aaa, bbb, ccc, ddd, eee, fff, ggg, hhh;
  int id, *XXX;
  char *YYY;
  void *ZZZ;
  struct
  {
    int a, b;
  }
   *AAA;
  Entry s;
  fprintf (stderr, "#####\n");
  id = NewHashTbl (8);
  aaa = 1;
  bbb = 2;
  ccc = 3;
  ddd = 4;
  eee = 5;
  fff = 6;
  ggg = 7;
  hhh = 8;
  s = AddEntry (id, aaa, 1);
  pr (s);
  s = FindEntry (id, aaa, 2);
  pr (s);
  s = AddEntry (id, bbb, 2);
  pr (s);
  s = FindEntry (id, bbb, 2);
  pr (s);
  s = AddEntry (id, ccc, 2);
  pr (s);
  s = AddEntry (id, ddd, 2);
  pr (s);
  s->ptr = XXX;
  s = AddEntry (id, ddd, 2);
  pr (s);
  s = AddEntry (id, eee, 2);
  pr (s);
  s = FindEntry (id, eee, 1);
  pr (s);
  s = AddEntry (id, fff, 2);
  pr (s);
  s->ptr = YYY;
  s = AddEntry (id, fff, 2);
  pr (s);
  s->ptr = ZZZ;
  s = AddEntry (id, fff, 2);
  pr (s);
  s->ptr = AAA;
  s = FindEntry (id, fff, 2);
  pr (s);
  s = AddEntry (id, ggg, 2);
  pr (s);
  s = AddEntry (id, hhh, 2);
  pr (s);
  fprintf (stderr, "#####\n");
}
#endif
