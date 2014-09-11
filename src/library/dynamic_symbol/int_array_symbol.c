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
 *      File:   int_array_symbol.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  Sept. 1998 (adapted from string_symbol.c)
 *      Copyright (c) 1998 The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "int_array_symbol.h"
#include <library/l_alloc_new.h>
#include <library/i_error.h>

L_Alloc_Pool *INT_ARRAY_Symbol_Table_pool = NULL;
L_Alloc_Pool *INT_ARRAY_Symbol_pool = NULL;

/****
 ****
 **** INT_ARRAY routines
 ****
 ****/

/* Create and initialize INT_ARRAY_Symbol_Table.
 * Creates a hash table of initial size 2 * expected_size rounded up
 * to the closest power of two.  (Min hash size 32)
 */
INT_ARRAY_Symbol_Table *
INT_ARRAY_new_symbol_table (char *name, int expected_size)
{
  INT_ARRAY_Symbol_Table *table;
  INT_ARRAY_Symbol **hash;
  unsigned int min_size, hash_size;
  int i;

  /* If expected size negative, force to be 0 */
  if (expected_size < 0)
    expected_size = 0;

  /* To prevent infinite loop by sizing algorithm (and running out of
   * memory :) ), expected_size must be <= a billion.
   */
  if (expected_size > 1000000000)
    I_punt ("INT_ARRAY_Symbol_Table: unreasonable expected_size (%u)",
            expected_size);

  /* Want a minumum size of at least twice the expected size */
  min_size = expected_size * 2;

  /* Start with a minumum hash size of 32.  
   * (Smaller sizes don't work as well with the hashing algorithm)
   */
  hash_size = 32;

  /* Double hash_size until min_size is reached or exceeded */
  while (hash_size < min_size)
    hash_size = hash_size << 1;


  /* Create new symbol table pool (and symbol pool if necessary) */
  if (INT_ARRAY_Symbol_Table_pool == NULL)
    {
      INT_ARRAY_Symbol_Table_pool =
        L_create_alloc_pool ("INT_ARRAY_Symbol_Table",
                             sizeof (INT_ARRAY_Symbol_Table), 16);

      INT_ARRAY_Symbol_pool =
        L_create_alloc_pool ("INT_ARRAY_Symbol",
                             sizeof (INT_ARRAY_Symbol), 64);
    }

  /* Allocate symbol table */
  table = (INT_ARRAY_Symbol_Table *) L_alloc (INT_ARRAY_Symbol_Table_pool);

  /* Allocate array for hash */
  hash =
    (INT_ARRAY_Symbol **) malloc (hash_size * sizeof (INT_ARRAY_Symbol *));
  if (hash == NULL)
    {
      I_punt ("INT_ARRAY_new_symbol_table: Out of memory, "
              "hash array size %i.", hash_size);
    }

  /* Initialize hash table */
  for (i = 0; i < hash_size; i++)
    hash[i] = NULL;

  /* Initialize fields */
  table->name = strdup (name);
  table->hash = hash;
  table->hash_size = hash_size;
  table->hash_mask = hash_size - 1;   /* AND mask, works only for power of 2 */
  /* Resize when count at 75% of hash_size */
  table->resize_size = hash_size - (hash_size >> 2);
  table->head_symbol = NULL;
  table->tail_symbol = NULL;
  table->symbol_count = 0;

  return (table);
}

/* Frees the symbol table, and optionally frees the data pointed to */
void
INT_ARRAY_delete_symbol_table (INT_ARRAY_Symbol_Table * table,
                               void (*free_routine) (void *))
{
  INT_ARRAY_Symbol *symbol, *next_symbol;

  /* For all the symbols in the table, free each one */
  for (symbol = table->head_symbol; symbol != NULL; symbol = next_symbol)
    {
      /* Get the next symbol before deleting this one */
      next_symbol = symbol->next_symbol;

      /* If free routine specified, free data */
      if (free_routine != NULL)
        free_routine (symbol->data);

      /* Free symbol structure and int_array */
      free (symbol->int_array);
      L_free (INT_ARRAY_Symbol_pool, symbol);
    }

  /* Free the hash array and table name */
  free (table->hash);
  free (table->name);

  /* Free the table structure */
  L_free (INT_ARRAY_Symbol_Table_pool, table);
}

/* Doubles the symbol table hash array size */
void
INT_ARRAY_resize_symbol_table (INT_ARRAY_Symbol_Table * table)
{
  INT_ARRAY_Symbol **new_hash, *symbol, *hash_head;
  int new_hash_size;
  unsigned int new_hash_mask, new_hash_index;
  int i;

  /* Double the size of the hash array */
  new_hash_size = table->hash_size * 2;

  /* Allocate new hash array */
  new_hash =
    (INT_ARRAY_Symbol **) malloc (new_hash_size *
                                  sizeof (INT_ARRAY_Symbol *));
  if (new_hash == NULL)
    {
      I_punt ("INT_ARRAY_resize_symbol_table: Out of memory, new size %i.",
              new_hash_size);
    }

  /* Initialize new hash table */
  for (i = 0; i < new_hash_size; i++)
    new_hash[i] = NULL;

  /* Get the hash mask for the new hash table */
  new_hash_mask = new_hash_size - 1;  /* AND mask, works only for power of 2 */

  /* Go though all the symbol and add to new hash table.
   * Can totally disreguard old hash links.
   */
  for (symbol = table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      /* Get index into hash table to use for this int_array */
      new_hash_index = symbol->hash_val & new_hash_mask;

      /* Add symbol to head of linked list */
      hash_head = new_hash[new_hash_index];
      symbol->next_hash = hash_head;
      symbol->prev_hash = NULL;
      if (hash_head != NULL)
        hash_head->prev_hash = symbol;
      new_hash[new_hash_index] = symbol;
    }

  /* Free old hash table */
  free (table->hash);

  /* Initialize table fields for new hash table */
  table->hash = new_hash;
  table->hash_size = new_hash_size;
  table->hash_mask = new_hash_mask;
  /* Resize when count at 75% of new_hash_size */
  table->resize_size = new_hash_size - (new_hash_size >> 2);
}

/* Hashes a int array, returning an unsigned 32 bit number. */
static unsigned int
INT_ARRAY_hash_int_array (int *int_array, int array_length)
{
  unsigned int hash_val;
  int i;

  hash_val = 0;

  /* Scan through the int_array, adding the ints to the
   * hash value.  Multiply the hash value by 17 (using shifts) before
   * adding each integer in.
   *
   * This very quick hash algorithm was tuned to work well with
   * strings ending with numbers, but should work OK for int arrays as well.
   */
  for (i = 0; i < array_length; i++)
    {
      /* Multiply hash_val by 17 by adding 16*hash_val to it.
       * (Use a shift, integer multiply is usually very expensive)
       */
      hash_val += (hash_val << 4);

      /* Add in int_array value */
      hash_val += int_array[i];
    }

  /* Return the hash value */
  return (hash_val);
}


/* Adds structure to symbol table, data is not copied (int_array is)!!! 
 * Dynamically increases symbol table's hash array.
 * Returns pointer to added symbol.
 */
INT_ARRAY_Symbol *
INT_ARRAY_add_symbol (INT_ARRAY_Symbol_Table * table,
                      int *int_array, int array_length, void *data)
{
  INT_ARRAY_Symbol *symbol, *hash_head, *check_symbol, *tail_symbol;
  unsigned int hash_val, hash_index;
  int symbol_count, i, match;

  /* Sanity check, array_length must be >= 1 */
  if (array_length < 1)
    {
      I_punt ("INT_ARRAY_add_symbol: Error array_length (%i) < 1!",
              array_length);
    }

  /* Increase symbol table size if necessary before adding new symbol.  
   * This will change the hash_mask if the table is resized!
   */
  symbol_count = table->symbol_count;
  if (symbol_count >= table->resize_size)
    {
      INT_ARRAY_resize_symbol_table (table);
    }

  /* Allocate a symbol (pool initialized in create table routine) */
  symbol = (INT_ARRAY_Symbol *) L_alloc (INT_ARRAY_Symbol_pool);

  /* Alloc int_array and Initialize it and rest of fields */
  if ((symbol->int_array = (int *) malloc (array_length * sizeof (int))) ==
      NULL)
    {
      I_punt ("INT_ARRAY_Symbol: Out of memory, int_array size %i",
              array_length * sizeof (int));
    }
  for (i = 0; i < array_length; i++)
    symbol->int_array[i] = int_array[i];

  symbol->array_length = array_length;
  symbol->data = data;
  symbol->table = table;

  /* Get tail symbol for ease of use */
  tail_symbol = table->tail_symbol;

  /* Add to linked list of symbols */
  symbol->next_symbol = NULL;
  symbol->prev_symbol = tail_symbol;

  if (tail_symbol == NULL)
    table->head_symbol = symbol;
  else
    tail_symbol->next_symbol = symbol;
  table->tail_symbol = symbol;


  /* Get hash value of int_array and put in symbol structure for 
   * quick compare and table resize.
   */
  hash_val = INT_ARRAY_hash_int_array (int_array, array_length);
  symbol->hash_val = hash_val;

  /* Get index into hash table */
  hash_index = hash_val & table->hash_mask;

  /* Get head symbol in current linked list for ease of use */
  hash_head = table->hash[hash_index];


  /* Sanity check (may want to ifdef out later).

   * Check that this symbol's int_array is not already in the symbol table.
   * Punt if it is, since can cause a major debugging nightmare.
   */
  for (check_symbol = hash_head; check_symbol != NULL;
       check_symbol = check_symbol->next_hash)
    {
      /* Check hash value and array_length before doing int_array compare to
       * minimize overhead.
       */
      if ((check_symbol->hash_val == hash_val) &&
          (check_symbol->array_length == array_length))
        {
          /* Determine if int_arrays really do match */
          match = 1;
          for (i = 0; i < array_length; i++)
            {
              if (check_symbol->int_array[i] != int_array[i])
                {
                  match = 0;
                  break;
                }
            }

          if (match)
            {
              fprintf (stderr, "%s: Cannot add int array:\n  ", table->name);
              fprintf (stderr, "(%i", int_array[0]);
              for (i = 1; i < array_length; i++)
                fprintf (stderr, ", %i", int_array[i]);
              fprintf (stderr, ")\n");
              I_punt ("   INT_ARRAY_add_symbol:int array already in table!");
            }
        }
    }


  /* Add symbol to head of linked list */
  symbol->next_hash = hash_head;
  symbol->prev_hash = NULL;
  if (hash_head != NULL)
    hash_head->prev_hash = symbol;
  table->hash[hash_index] = symbol;

  /* Update table's symbol count */
  table->symbol_count = symbol_count + 1;

  /* Return symbol added */
  return (symbol);
}

/* Returns a INT_ARRAY_Symbol structure with the desired int_array, or NULL
 * if the int_array does not match something in the symbol table.
 */
INT_ARRAY_Symbol *
INT_ARRAY_find_symbol (INT_ARRAY_Symbol_Table * table,
                       int *int_array, int array_length)
{
  INT_ARRAY_Symbol *symbol;
  unsigned int hash_val, hash_index;
  int match, i;

  /* Sanity check, array_length must be >= 1 */
  if (array_length < 1)
    {
      I_punt ("INT_ARRAY_find_symbol: Error array_length (%i) < 1!",
              array_length);
    }

  /* Get the hash value for the int_array */
  hash_val = INT_ARRAY_hash_int_array (int_array, array_length);

  /* Get the index into the hash table */
  hash_index = hash_val & table->hash_mask;

  /* Search the linked list for a matching int_array */
  for (symbol = table->hash[hash_index]; symbol != NULL;
       symbol = symbol->next_hash)
    {
      /* Check hash value and array_length before doing int_array compare to
       * minimize overhead.
       */
      if ((symbol->hash_val == hash_val) &&
          (symbol->array_length == array_length))
        {
          /* Determine if int_array really does match */
          match = 1;
          for (i = 0; i < array_length; i++)
            {
              if (symbol->int_array[i] != int_array[i])
                {
                  match = 0;
                  break;
                }
            }
          if (match)
            {
              return (symbol);
            }
        }
    }

  return (NULL);
}

/* 
 * Returns the data for desired int_array, or NULL
 * if the int_array is not in the symbol table.
 */
void *
INT_ARRAY_find_symbol_data (INT_ARRAY_Symbol_Table * table,
                            int *int_array, int array_length)
{
  INT_ARRAY_Symbol *symbol;
  unsigned int hash_val, hash_index;
  int i, match;

  /* Sanity check, array_length must be >= 1 */
  if (array_length < 1)
    {
      I_punt ("INT_ARRAY_find_symbol_data: Error array_length (%i) < 1!",
              array_length);
    }

  /* Get the hash value for the int_array */
  hash_val = INT_ARRAY_hash_int_array (int_array, array_length);

  /* Get the index into the hash table */
  hash_index = hash_val & table->hash_mask;

  /* Search the linked list for matching int_array */
  for (symbol = table->hash[hash_index]; symbol != NULL;
       symbol = symbol->next_hash)
    {
      /* Check hash value and array_length before doing int_array compare to
       * minimize overhead.
       */
      if ((symbol->hash_val == hash_val) &&
          (symbol->array_length == array_length))
        {
          /* Determine if int_array really does match */
          match = 1;
          for (i = 0; i < array_length; i++)
            {
              if (symbol->int_array[i] != int_array[i])
                {
                  match = 0;
                  break;
                }
            }
          if (match)
            {
              return (symbol->data);
            }
        }
    }

  return (NULL);
}

/* Deletes symbol and optionally deletes the data using the free routine */
void
INT_ARRAY_delete_symbol (INT_ARRAY_Symbol * symbol,
                         void (*free_routine) (void *))
{
  INT_ARRAY_Symbol_Table *table;
  INT_ARRAY_Symbol *next_hash, *prev_hash, *next_symbol, *prev_symbol;
  unsigned int hash_index;

  /* Get the table the symbol is from */
  table = symbol->table;

  /* Get the hash index from the symbol's hash_val */
  hash_index = symbol->hash_val & table->hash_mask;

  /* Remove symbol from hash table */
  prev_hash = symbol->prev_hash;
  next_hash = symbol->next_hash;
  if (prev_hash == NULL)
    table->hash[hash_index] = next_hash;
  else
    prev_hash->next_hash = next_hash;

  if (next_hash != NULL)
    next_hash->prev_hash = prev_hash;

  /* Remove symbol from symbol list */
  prev_symbol = symbol->prev_symbol;
  next_symbol = symbol->next_symbol;
  if (prev_symbol == NULL)
    table->head_symbol = next_symbol;
  else
    prev_symbol->next_symbol = next_symbol;

  if (next_symbol == NULL)
    table->tail_symbol = prev_symbol;
  else
    next_symbol->prev_symbol = prev_symbol;


  /* If free routine specified, free symbol data */
  if (free_routine != NULL)
    free_routine (symbol->data);


  /* Free symbol structure and int_array */
  free (symbol->int_array);
  L_free (INT_ARRAY_Symbol_pool, symbol);

  /* Decrement table symbol count */
  table->symbol_count--;
}


/* Prints out the symbol table's hash table (debug routine) */
void
INT_ARRAY_print_symbol_table_hash (FILE * out, INT_ARRAY_Symbol_Table * table)
{
  INT_ARRAY_Symbol *symbol;
  int hash_index, lines, i;

  /* Count lines used in table */
  lines = 0;
  for (hash_index = 0; hash_index < table->hash_size; hash_index++)
    {
      if (table->hash[hash_index] != NULL)
        lines++;
    }
  fprintf (out, "%s has %i entries (hash size %i, used %i):\n",
           table->name, table->symbol_count, table->hash_size, lines);

  /* For each hash_index in hash table */
  for (hash_index = 0; hash_index < table->hash_size; hash_index++)
    {
      /* Skip empty lines */
      if (table->hash[hash_index] == NULL)
        continue;

      fprintf (out, "%4i:", hash_index);
      for (symbol = table->hash[hash_index]; symbol != NULL;
           symbol = symbol->next_hash)
        {
          fprintf (out, "(%i", symbol->int_array[0]);
          for (i = 0; i < symbol->array_length; i++)
            fprintf (out, ", %i", symbol->int_array[i]);
          fprintf (out, ") ");
        }
      fprintf (out, "\n");
    }
}
