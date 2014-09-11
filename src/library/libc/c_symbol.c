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
 *      File:   c_symbol.c
 *      Author: Po-hua Chang
 *      Creation Date:  June 1990
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/*===========================================================================
 *      Description :   Symbol table management.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <library/c_symbol.h>

#undef DEBUG
#undef DEBUG_OPEN
#undef ABORT_ON_SPILL

/*----------------------------------------------------------------------*/
/*       
 *      Each symbol is uniquely identified by the symbol name
 *      and the symbol type fields. The ptr field can be used
 *      to connect a user defined data structure to the symbol.
 *      valid types are non-negative.
 */
#define C_INVALID_SYMBOL        -1
#define C_DELETED_SYMBOL        -2

#if 0
        /* DMG - 5/95 - typdefs moved to .h file */
typedef struct _C_Symbol
{
  char *name;                   /* symbol name */
  int type;                     /* symbol type */
  Integer value;                /* a general purpose value carrier */
  Pointer ptr;                  /* a general purpose pointer */
}
_C_Symbol, *C_Symbol;

typedef struct _SymTbl
{
  C_Symbol array;               /* symbol array */
  Integer size;                 /* table size */
  int mode;                     /* type matching style */
}
_SymTbl, *SymTbl;
#endif

_SymTbl TBL[C_MAX_SYMBOL_TABLE] = { { 0 } };        /* symbol tables */
static int next_tbl = 0;        /* next symbol table */

/*----------------------------------------------------------------------*/
static C_Symbol
find_symbol (SymTbl tbl, char *name, int type, int new_entry)
                /* symbol table */
                /* symbol name */
                /* symbol type */
                /* create new entry */
{
  C_Symbol table;
  int length, key, limit, mode;
  int hash_size, hash_pos, increment, trial;
  length = C_strlen (name);     /* length of the string */
  /* Quadratic probing */
  /* initialize hash table parameters */
  table = tbl->array;
  hash_size = (int) tbl->size;
  mode = tbl->mode;
  /* compute initial hash position */
  if (length < 1)
    fprintf (stderr, "> called Hash with null name\n");
  key =
    (type * 377) + (name[0] * 231) + (name[length - 1] * 17) +
    (name[(length - 1) / 2] * 96) + (name[(length - 1) * 3 / 4] * 1027) +
    (name[(length - 1) / 4] * 3);
  hash_pos = key % hash_size;
  /* iterate until exit condition is met */
  increment = 1;
  limit = hash_size * 4;
  if (new_entry)
    {
      int tp, found = 0;
      C_Symbol sym = NULL;
      for (trial = limit; (trial > 0) && (!found); trial--)
        {
            /** get information from the current symbol **/
          sym = table + hash_pos;
          tp = sym->type;
            /** anticipate rehash **/
          hash_pos = (hash_pos + increment) % hash_size;
          increment++;
          /* test if it is a deleted/invalid entry */
          found = ((tp == C_DELETED_SYMBOL) || (tp == C_INVALID_SYMBOL));
        }
      if (found)
        return sym;
      fprintf (stderr, "> failed to create a new symbol table entry\n");
    }
  else
    {
      if (mode == C_MATCH_BY_EXACT_TYPE)
        {
          C_Symbol sym;
          for (trial = limit; trial > 0; trial--)
            {
                /** get information from the current symbol **/
              sym = table + hash_pos;
                /** anticipate rehash **/
              hash_pos = (hash_pos + increment) % hash_size;
              increment++;
                /** actual match **/
              if ((sym->type == C_INVALID_SYMBOL) ||
                  ((sym->type == type) && !strcmp (sym->name, name)))
                return sym;
            }
        }
      else
        {
          C_Symbol sym;
          for (trial = limit; trial > 0; trial--)
            {
                /** get information from the current symbol **/
              sym = table + hash_pos;
                /** anticipate rehash **/
              hash_pos = (hash_pos + increment) % hash_size;
              increment++;
                /** actual match **/
              if ((sym->type == C_INVALID_SYMBOL) ||
                  (((sym->type & type) != 0) && !strcmp (sym->name, name)))
                return sym;
            }
        }
      fprintf (stderr, "> failed to find a symbol table entry\n");
    }
  /* went into a recursive loop, apparently the table
   * is not designed correctly.
   */
  fprintf (stderr, "> number of rehashes = %d\n", limit - trial);
  fprintf (stderr, "> tbl=%d, name=\"%s\", type=%d, new=%d\n",
#if LP64_ARCHITECTURE
           (int)((long)tbl - (long)TBL),
#else
	   (int)tbl - (int)TBL,
#endif
	   name, type, new_entry);
  fprintf (stderr, "> table size=%d, mode=%d\n", tbl->size, tbl->mode);
#ifdef ABORT_ON_SPILL
  abort ();
#endif
  C_assert (0, "c_symbol: too many hash steps");
  return 0;
}

/*----------------------------------------------------------------------*/
static int
find_empty_entry (SymTbl tbl, char *name, int type, int new_entry)
                /* symbol table */
                /* symbol name */
                /* symbol type */
                /* create new entry */
{
  C_Symbol table;
  int length, key, limit, mode;
  int hash_size, hash_pos, increment, trial;
  length = C_strlen (name);     /* length of the string */
  /* Quadratic probing */
  /* initialize hash table parameters */
  table = tbl->array;
  hash_size = (int) tbl->size;
  mode = tbl->mode;
  /* compute initial hash position */
  if (length < 1)
    fprintf (stderr, "> called Hash with null name\n");
  key =
    (type * 377) + (name[0] * 231) + (name[length - 1] * 17) +
    (name[(length - 1) / 2] * 96) + (name[(length - 1) * 3 / 4] * 1027) +
    (name[(length - 1) / 4] * 3);
  hash_pos = key % hash_size;
  /* iterate until exit condition is met */
  increment = 1;
  limit = hash_size * 4;
  if (new_entry)
    {
      int tp, found = 0;
      C_Symbol sym;
      for (trial = limit; (trial > 0) && (!found); trial--)
        {
            /** get information from the current symbol **/
          sym = table + hash_pos;
          tp = sym->type;
            /** anticipate rehash **/
          hash_pos = (hash_pos + increment) % hash_size;
          increment++;
          /* test if it is a deleted/invalid entry */
          found = ((tp == C_DELETED_SYMBOL) || (tp == C_INVALID_SYMBOL));
        }
      if (found)
        return 1;
      fprintf (stderr, "> failed to create a new symbol table entry\n");
    }
  else
    {
      if (mode == C_MATCH_BY_EXACT_TYPE)
        {
          C_Symbol sym;
          for (trial = limit; trial > 0; trial--)
            {
                /** get information from the current symbol **/
              sym = table + hash_pos;
                /** anticipate rehash **/
              hash_pos = (hash_pos + increment) % hash_size;
              increment++;
                /** actual match **/
              if ((sym->type == C_INVALID_SYMBOL) ||
                  ((sym->type == type) && !strcmp (sym->name, name)))
                return 1;
            }
        }
      else
        {
          C_Symbol sym;
          for (trial = limit; trial > 0; trial--)
            {
                /** get information from the current symbol **/
              sym = table + hash_pos;
                /** anticipate rehash **/
              hash_pos = (hash_pos + increment) % hash_size;
              increment++;
                /** actual match **/
              if ((sym->type == C_INVALID_SYMBOL) ||
                  (((sym->type & type) != 0) && !strcmp (sym->name, name)))
                return 1;
            }
        }
    }
  return 0;
}

/*----------------------------------------------------------------------*/
/** Create a new symbol table. **/
int
C_open (int max_size, int mode)
{
  int new_id;
  SymTbl T;
  C_Symbol sym;
  int size;
  int cur_try;

  if ((mode != C_MATCH_BY_EXACT_TYPE) && (mode != C_MATCH_BY_AND_TYPE))
    {
      C_assert (0, "C_open: illegal match mode");
      return -1;
    }
  if (max_size < 64)
    {
      C_assert (0, "C_open: size must be at least 64");
      return -1;
    }
  size = max_size;
    /** get next table **/
  new_id = next_tbl;

#if 0
  if (new_id >= C_MAX_SYMBOL_TABLE)
    {
      C_assert (0, "C_open: run out of symbol tables");
      return -1;
    }
#endif

  /* DMG - we now allow re-use of the deleted symbol tables;
     search for entry with NULL C_Symbol, wrapping around if
     necessary */

  /* BCC - the following code already accesses out-of-bound memory - 9/9/97 */
#if 0
  while (((TBL + new_id)->array != NULL) && new_id != (next_tbl - 1))
    {

      new_id++;

      if (new_id >= C_MAX_SYMBOL_TABLE)
        new_id = 0;
    }
#endif
  for (cur_try = 0; cur_try < C_MAX_SYMBOL_TABLE; cur_try++, new_id++)
    {
      new_id %= C_MAX_SYMBOL_TABLE;
      if ((TBL + new_id)->array == NULL)
        break;
    }


  T = TBL + new_id;

  if (T->array != NULL)
    {
      C_assert (0, "C_open: run out of symbol tables");
      return -1;
    }

  next_tbl = new_id + 1;

  sym = (C_Symbol) C_malloc (((int) (sizeof (_C_Symbol) * size)));
  if (sym == 0)
    {
#if LP64_ARCHITECTURE
      fprintf (stderr, "> C_malloc(%ld) returns NULL\n",
               sizeof (_C_Symbol) * size);
#else
      fprintf (stderr, "> C_malloc(%d) returns NULL\n",
               sizeof (_C_Symbol) * size);
#endif
      fprintf (stderr, "> C_open(%d, %d)\n", max_size, mode);
      C_assert (0, "C_open: failed to allocate symbol table array");
      return -1;
    }
    /** allocate a table **/
  T->mode = mode;
  T->size = size;
  T->array = sym;
  C_clear (new_id);
#ifdef DEBUG_OPEN
  if (new_id == 2)
    abort ();
#endif
  return new_id;
}


/** Delete a symbol table, leaving ptr to C_Symbol NULL **/

void
C_delete_tbl (int tbl_id)
{
  Integer i;
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;

  if (tbl_id == -1)
    return;

  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_delete_tbl: illegal tbl_id");
  sym = T->array;
  for (i = T->size; i > 0; i--)
    {
      /* BCC - was memory leakage */
      if (sym->type != C_INVALID_SYMBOL)
        free (sym->name);
      sym->name = 0;
      sym->type = C_INVALID_SYMBOL;
      sym->value = 0;
      sym->ptr = 0;
      sym++;
    }

  free (T->array);
  T->array = 0;
}



/** Clear a symbol table. */
void
C_clear (int tbl_id)
{
  Integer i;
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;

  if (tbl_id == -1)
    return;

  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_clear: illegal tbl_id");
  sym = T->array;
  for (i = T->size; i > 0; i--)
    {
      sym->name = "";
      sym->type = C_INVALID_SYMBOL;
      sym->value = 0;
      sym->ptr = 0;
      sym++;
    }
}


/* BCC - added for garbage collection - 8/20/96 */
/** Clear a symbol table and free all fields */
void
C_clear_free_all (int tbl_id)
{
  Integer i;
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;

  if (tbl_id == -1)
    return;

  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_clear: illegal tbl_id");
  sym = T->array;
  for (i = T->size; i > 0; i--)
    {
      if (sym->name && sym->name[0])
        free (sym->name);
      sym->name = "";
      sym->type = C_INVALID_SYMBOL;
      sym->value = 0;
      sym->ptr = 0;
      sym++;
    }
}


/* BCC - added for garbage collection - 8/20/96 */
/** Clear a symbol table and free name fields */
void
C_clear_free_name (int tbl_id)
{
  Integer i;
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;

  if (tbl_id == -1)
    return;

  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_clear: illegal tbl_id");
  sym = T->array;
  for (i = T->size; i > 0; i--)
    {
      if (sym->name && sym->name[0])
        free (sym->name);
      sym->name = "";
      sym->type = C_INVALID_SYMBOL;
      sym->value = 0;
      sym->ptr = 0;
      sym++;
    }
}

/** Clear a symbol table. */
void
C_clear_attr (int tbl_id)
{
  Integer i;
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_clear_attr: illegal tbl_id");
  sym = T->array;
  for (i = T->size; i > 0; i--)
    {
      sym->value = 0;
      sym->ptr = 0;
      sym++;
    }
}
/** Add a new symbol in the symbol table. */
void
C_update (int tbl_id, char *name, int type, Integer value, Pointer ptr)
{
  SymTbl tbl = TBL + tbl_id;
  C_Symbol sym;
  if (type < 0)
    C_assert (0, "C_update: type must be positive");
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_update: illegal tbl_id");
  sym = find_symbol (tbl, name, type, 0);       /* find old entry */
  /* create a new symbol if necessary */
  if (sym->type == C_INVALID_SYMBOL)
    {
      sym = find_symbol (tbl, name, type, 1);   /* find empty slot */
      sym->type = type;
      sym->name = C_strsave (name);
    }
  sym->value = value;
  sym->ptr = ptr;
}
/** Find a symbol in the symbol table. */
int
C_find (int tbl_id, char *name, int type, Integer * value, Pointer * ptr)
{
  SymTbl tbl = TBL + tbl_id;
  Integer v;
  Pointer p;
  C_Symbol sym;
  if (type < 0)
    C_assert (0, "C_find: type must be positive");
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_find: illegal tbl_id");
  sym = find_symbol (tbl, name, type, 0);
  v = sym->value;
  p = sym->ptr;
  /* if not found, return 0 */
  if (sym->type == C_INVALID_SYMBOL)
    {
      if (value != 0)
        *value = 0;
      if (ptr != 0)
        *ptr = 0;
      return 0;
    }
  else
    {
      if (value != 0)
        *value = v;
      if (ptr != 0)
        *ptr = p;
      return 1;
    }
}
/** Find a empty slot in the symbol table. */
int
C_full (int tbl_id, char *name, int type)
{
  SymTbl tbl = TBL + tbl_id;
  if (type < 0)
    C_assert (0, "C_full: type must be positive");
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_full: illegal tbl_id");
  if (find_empty_entry (tbl, name, type, 0))
    return 0;
  return 1;
}
/** Delete a symbol in the symbol table. */
int
C_delete (int tbl_id, char *name, int type)
{
  SymTbl tbl = TBL + tbl_id;
  C_Symbol sym;
  if (type < 0)
    C_assert (0, "C_delete: type must be positive");
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_delete: illegal tbl_id");
  sym = find_symbol (tbl, name, type, 0);
  /* if not found, return 0 */
  if (sym->type == C_INVALID_SYMBOL)
    {
      return 0;
    }
  else
    {
      sym->type = C_DELETED_SYMBOL;
      return 1;
    }
}
/** Traverse a symbol table. */
int
C_forall (int tbl_id, C_Function fn)
{
  Integer i;
  int size;
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;
  int type;
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_forall: illegal tbl_id");
  size = 0;
  if (fn == 0)
    {
      sym = T->array;
      for (i = T->size; i > 0; i--)
        {
          type = sym->type;
          size += (type >= 0);
          sym++;
        }
    }
  else
    {
      sym = T->array;
      for (i = T->size; i > 0; i--)
        {
          type = sym->type;
          if (type >= 0)
            {
              size += 1;
              (*fn) (sym->name, type, sym->value, sym->ptr);
            }
          sym++;
        }
    }
  return size;
}
/* BCC - find out the index - 3/22/97 */
int
C_find_index (int tbl_id, void *ptr)
{
  Integer i;
  int size;
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;
  int type;
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_forall: illegal tbl_id");
  size = 0;
  sym = T->array;
  for (i = T->size; i > 0; i--)
    {
      type = sym->type;
      if (type >= 0)
        {
          size += 1;
          if ((void *) sym->ptr == ptr)
            return T->size - i;
        }
      sym++;
    }
  return -1;
}
/* BCC - find out the index - 3/22/97 */
Pointer C_find_from_index (int tbl_id, int index)
{
  SymTbl T = TBL + tbl_id;
  C_Symbol sym;

  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_forall: illegal tbl_id");
  if (index >= T->size)
    C_assert (0, "C_forall: illegal index value");

  sym = T->array;
  return sym[index].ptr;
}
/** Create a name table **/
int
C_open_name_table (int max_size)
{
  return C_open (max_size, C_MATCH_BY_EXACT_TYPE);
}
char *
C_unique_name (int tbl_id, char *name)
{
  SymTbl tbl = TBL + tbl_id;
  C_Symbol sym;
  if ((tbl_id < 0) || (tbl_id > C_MAX_SYMBOL_TABLE))
    C_assert (0, "C_unique_name: illegal tbl_id");
  sym = find_symbol (tbl, name, 1, 0);  /* find old entry */
  /* create a new symbol if necessary */
  if (sym->type == C_INVALID_SYMBOL)
    {
      sym = find_symbol (tbl, name, 1, 1);      /* find empty slot */
      sym->type = 1;
      sym->name = C_strsave (name);
    }
  return sym->name;
}
/*----------------------------------------------------------------------*/
