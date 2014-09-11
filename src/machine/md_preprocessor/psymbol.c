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
 *      File:   psymbol.c
 * 
 *      Description: Symbol table routines for the IMPACT Meta-Description 
 *                   Language preprocessor
 * 
 *      Creation Date:  October 1994
 * 
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *  
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <string.h>
#include "psymbol.h"

L_Alloc_Pool *Psymbol_Table_pool = NULL;
L_Alloc_Pool *Psymbol_pool = NULL;

/* Create Psymbol_Table */
Psymbol_Table *
create_Psymbol_Table (char *name)
{
  Psymbol_Table *table;
  int i;

  if (Psymbol_Table_pool == NULL)
    {
      Psymbol_Table_pool = L_create_alloc_pool ("Psymbol_Table",
						sizeof (Psymbol_Table), 16);
    }
  table = (Psymbol_Table *) L_alloc (Psymbol_Table_pool);

  /* Initialize fields */
  table->name = strdup (name);
  for (i = 0; i < PSYMBOL_HASH_SIZE; i++)
    table->hash[i] = NULL;
  table->head = NULL;
  table->tail = NULL;
  table->count = 0;

  return (table);
}

/* Free Psymbol table */
void
free_Psymbol_Table (Psymbol_Table * table, void (*free_routine) (void *))
{
  Psymbol *symbol, *next_symbol;

  /* Free all the symbols in the symbol table */
  for (symbol = table->head; symbol != NULL; symbol = next_symbol)
    {
      /* Get next symbol before deleting this one */
      next_symbol = symbol->next_symbol;

      /* Free symbol's contents */
      free (symbol->name);
      if (free_routine != NULL)
	free_routine (symbol->data);

      L_free (Psymbol_pool, symbol);
    }

  /* Free symbol table itself */
  free (table->name);

  L_free (Psymbol_Table_pool, table);
}

/* Hashes a Psymbol name, 
 * returning a number between 0 and PSYMBOL_HASH_SIZE-1
 */
int
hash_Psymbol_name (char *name)
{
  int hash;
  char *ptr;

  hash = 0;

  /* Sum all the characters, shift hash left 1 with each character */
  for (ptr = name; *ptr != 0; ptr++)
    {
      hash = hash << 1;
      hash += *ptr;
    }

  /* Mask out unused bits */
  hash = hash & (PSYMBOL_HASH_SIZE - 1);

  return (hash);
}

/* Low level add routine, data is not copied, it is just pointed to */
void
add_Psymbol (Psymbol_Table * table, char *name, void *data)
{
  int hash_val;
  Psymbol *symbol;

  /* Create symbol */
  if (Psymbol_pool == NULL)
    {
      Psymbol_pool = L_create_alloc_pool ("Psymbol", sizeof (Psymbol), 32);
    }
  symbol = (Psymbol *) L_alloc (Psymbol_pool);

  /* Initialize fields */
  symbol->name = strdup (name);
  symbol->data = data;
  symbol->table = table;

  /* Add to list of symbols */
  symbol->next_symbol = NULL;
  symbol->prev_symbol = table->tail;

  if (table->tail == NULL)
    table->head = symbol;
  else
    table->tail->next_symbol = symbol;
  table->tail = symbol;

  /* Add to hash table of symbol names */
  hash_val = hash_Psymbol_name (name);
  symbol->next_hash = table->hash[hash_val];
  symbol->prev_hash = NULL;

  if (table->hash[hash_val] != NULL)
    table->hash[hash_val]->prev_hash = symbol;
  table->hash[hash_val] = symbol;

  /* Increment table's symbol count */
  table->count++;
}

/* low-level find symbol routines,
 * returns symbol, or NULL if not found
 */
Psymbol *
find_Psymbol (Psymbol_Table * table, char *name)
{
  Psymbol *symbol;
  int hash_val;

  /* Search for name in hash table */
  hash_val = hash_Psymbol_name (name);
  for (symbol = table->hash[hash_val]; symbol != NULL;
       symbol = symbol->next_hash)
    {
      /* Stop when match found */
      if (strcmp (symbol->name, name) == 0)
	break;
    }

  /* Return symbol or NULL if not found */
  return (symbol);
}

/* Low-level delete symbol routine. */
void
delete_Psymbol (Psymbol * symbol, void (*free_routine) (void *))
{
  Psymbol_Table *table;
  int hash_val;

  /* Get the table the symbol is from */
  table = symbol->table;

  /* hash symbol name to find in symbol table */
  hash_val = hash_Psymbol_name (symbol->name);

  /* Remove symbol from hash table */
  if (symbol->prev_hash == NULL)
    table->hash[hash_val] = symbol->next_hash;
  else
    symbol->prev_hash->next_hash = symbol->next_hash;

  if (symbol->next_hash != NULL)
    symbol->next_hash->prev_hash = symbol->prev_hash;

  /* Remove symbol from symbol list */
  if (symbol->prev_symbol == NULL)
    table->head = symbol->next_symbol;
  else
    symbol->prev_symbol->next_symbol = symbol->next_symbol;

  if (symbol->next_symbol == NULL)
    table->tail = symbol->prev_symbol;
  else
    symbol->next_symbol->prev_symbol = symbol->prev_symbol;

  /* Free symbols contents */
  free (symbol->name);
  if (free_routine != NULL)
    free_routine (symbol->data);

  L_free (Psymbol_pool, symbol);

  /* Decrement table symbol count */
  table->count--;
}


/* Debug routine, prints Psymbol table's hash table */
void
print_Psymbol_hash_table (FILE * out, Psymbol_Table * table)
{
  int i;
  Psymbol *symbol;

  fprintf (out, "%s symbol table has %i entries:\n", table->name,
	   table->count);

  /* For each hash table list */
  for (i = 0; i < PSYMBOL_HASH_SIZE; i++)
    {
      /* Print out only those hash table lines with symbols in them */
      if (table->hash[i] != NULL)
	{
	  fprintf (out, "%4i:", i);

	  for (symbol = table->hash[i]; symbol != NULL;
	       symbol = symbol->next_hash)
	    {
	      fprintf (out, " %s", symbol->name);
	    }

	  fprintf (out, "\n");
	}
    }
}
