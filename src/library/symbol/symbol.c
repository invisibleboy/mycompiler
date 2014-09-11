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
 *      File:   symbol.c
 *      Author: Pohua Chang
 *      Copyright (c) 1991 Pohua Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*
 *      The internal symbol table structure is implementation
 *      specific. In this implementation, a symbol table is
 *      implemented as a hash table.
 *
\*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <library/symbol.h>

/* print error message and punt. */
static void
error (char *mesg)
{
  fprintf (stderr, "symbol table library error : %s\n", mesg);
  exit (-1);
}

/* allocate space for a string and duplicates a string */
char *
StrSave (char *str)
{
  int i, len;
  char *new_str;
  len = strlen (str);
  new_str = (char *) malloc (len + 1);
  if (new_str == 0)
    {
      fprintf (stderr, "# StrSave # lack of space\n");
      exit (-1);
    }
  for (i = 0; i <= len; i++)
    new_str[i] = str[i];
  return new_str;
}

#define MAX     100             /* at most 100 symbol tables */

typedef struct _SymTbl
{
  Symbol array;                 /* symbol array */
  int size;                     /* table size */
}
_SymTbl, *SymTbl;

static _SymTbl TBL[MAX];        /* symbol tables */
static int next_tbl = 0;        /* next symbol table */

/* Compute the hash address. Return a number between 0 and size. 
 * Hash either the symbol is found, or when an INVALID_SYMBOL
 * is found.
 */
static int
Hash (SymTbl tbl, char *name, int type)
                /* symbol table */
                /* symbol name */
                /* symbol type */
{
  Symbol table;
  int length, key;
  register int hash_size, hash_pos, increment, trial;
  /* Quadratic probing */
  /* 1: initialize hash table parameters */
  table = tbl->array;
  hash_size = tbl->size;
  /* 2: compute initial hash position */
  length = strlen (name);
  key =
    (type * 377) + (name[0] * 23) + (name[length] * 12) + (name[length / 2]);
  hash_pos = key % hash_size;
  /* 3: iterate until exit condition is met */
  increment = 1;
  for (trial = 0; trial < hash_size; trial++)
    {
      /* 4: test if there is a hit */
      if ((table[hash_pos].type == type) &&
          !strcmp (table[hash_pos].name, name))
        {
          return hash_pos;
        }
      else
        /* 5: test if there is a miss */
      if (table[hash_pos].type == INVALID_SYMBOL)
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
  fprintf (stderr, "# symbol table size = %d\n", hash_size);
  fprintf (stderr, "# number of hash steps = %d\n", trial);
  error ("symbol: too many hash steps");
  return -1;
}
/** Create a new symbol table. **/
int
NewSymTbl (int max_size)
{
  int new_id, i;
  SymTbl T;
        /** get next table */
  new_id = next_tbl++;
  if (new_id >= MAX)
    {
      error ("symbol: run out of symbol tables");
      return -1;
    }
  T = TBL + new_id;
  T->size = max_size;
  T->array = (Symbol) malloc (sizeof (_Symbol) * T->size);
  if (T->array == 0)
    {
      error ("symbol: failed to allocate symbol table array");
      return -1;
    }
  for (i = 0; i < T->size; i++)
    {
      T->array[i].name = 0;
      T->array[i].type = INVALID_SYMBOL;
      T->array[i].value = 0;
      T->array[i].ptr = 0;
    }
  return new_id;
}
/** Clear a symbol table. */
void
ClearSymTbl (int tbl_id)
{
  register int i;
  SymTbl T = TBL + tbl_id;
  if ((tbl_id < 0) || (tbl_id > next_tbl))
    error ("symbol: in ClearSymTbl: illegal tbl_id");
  for (i = 0; i < T->size; i++)
    {
      T->array[i].name = 0;
      T->array[i].type = INVALID_SYMBOL;
      T->array[i].value = 0;
      T->array[i].ptr = 0;
    }
}
/* BCC -  Clear a symbol table and free its ptr fields - 8/17/96 */
void
ClearFreeSymTbl (int tbl_id, void (*free_xxx) (void *))
{
  register int i;
  SymTbl T = TBL + tbl_id;
  if ((tbl_id < 0) || (tbl_id > next_tbl))
    error ("symbol: in ClearSymTbl: illegal tbl_id");
  for (i = 0; i < T->size; i++)
    {
      if (T->array[i].name)
        free (T->array[i].name);
      T->array[i].name = 0;
      T->array[i].type = INVALID_SYMBOL;
      T->array[i].value = 0;
      if (T->array[i].ptr)
        free_xxx (T->array[i].ptr);
      T->array[i].ptr = 0;
    }
}
/** Add a new symbol in the symbol table. */
Symbol AddSym (int tbl_id, char *name, int type)
{
  int index;
  SymTbl tbl = TBL + tbl_id;
  Symbol sym;
  if ((tbl_id < 0) || (tbl_id > next_tbl))
    error ("symbol: in FindSym: illegal tbl_id");
  index = Hash (tbl, name, type);
  sym = tbl->array + index;
  /* create a new symbol if necessary */
  if (sym->type == INVALID_SYMBOL)
    {
      sym->name = StrSave (name);
      sym->type = type;
      sym->value = 0;
      sym->ptr = 0;
    }
  return sym;
}
/** Find a symbol in the symbol table. */
Symbol FindSym (int tbl_id, char *name, int type)
{
  int index;
  SymTbl tbl = TBL + tbl_id;
  Symbol sym;
  if ((tbl_id < 0) || (tbl_id > next_tbl))
    error ("symbol: in FindSym: illegal tbl_id");
  index = Hash (tbl, name, type);
  sym = tbl->array + index;
  /* if not found, return 0 */
  if (sym->type == INVALID_SYMBOL)
    return 0;
  else
    return sym;
}

#ifdef DEBUG
void
pr (Symbol sym)
{
  if (sym == 0)
    fprintf (stderr, "()\n");
  else
    fprintf (stderr, "(%s %d %d %d)\n",
             sym->name, sym->type, sym->value, sym->ptr);
}
void
main ()
{
  int id, *XXX;
  char *YYY;
  void *ZZZ;
  struct
  {
    int a, b;
  }
   *AAA;
  Symbol s;
  fprintf (stderr, "#####\n");
  id = NewSymTbl (7);
  s = AddSym (id, "aaa", 1);
  pr (s);
  s = FindSym (id, "aaa", 2);
  pr (s);
  s = AddSym (id, "bbb", 2);
  pr (s);
  s = FindSym (id, "bbb", 2);
  pr (s);
  s = AddSym (id, "ccc", 2);
  pr (s);
  s = AddSym (id, "ddd", 2);
  pr (s);
  s->ptr = XXX;
  s = AddSym (id, "ddd", 2);
  pr (s);
  s = AddSym (id, "eee", 2);
  pr (s);
  s = FindSym (id, "eee", 1);
  pr (s);
  s = AddSym (id, "fff", 2);
  pr (s);
  s->ptr = (Symbol) YYY;
  s = AddSym (id, "fff", 2);
  pr (s);
  s->ptr = (Symbol) ZZZ;
  s = AddSym (id, "fff", 2);
  pr (s);
  s->ptr = (Symbol) AAA;
  s = FindSym (id, "fff", 2);
  pr (s);
  s = AddSym (id, "ggg", 2);
  pr (s);
  s = AddSym (id, "hhh", 2);
  pr (s);
  fprintf (stderr, "#####\n");
}
#endif
