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
 *      File:   string_symbol.h
 *      Author: John C. Gyllenhaal
 *      Creation Date:  1995 (Split into separate files Sept. 1998 -JCG)
 *      Copyright (c) 1995 The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef STRING_SYMBOL_H
#define STRING_SYMBOL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 * STRING symbol table structures and prototypes
 */
typedef struct STRING_Symbol
{
  char *name;                   /* Name of symbol */
  unsigned int hash_val;        /* Hashed value of name */
  void *data;                   /* Data struct pointed to */
  struct STRING_Symbol_Table *table;    /* For deletion of symbol */
  struct STRING_Symbol *next_hash;      /* For table's hash table */
  struct STRING_Symbol *prev_hash;
  struct STRING_Symbol *next_symbol;    /* For table's contents list */
  struct STRING_Symbol *prev_symbol;
}
STRING_Symbol;

typedef struct STRING_Symbol_Table
{
  char *name;                   /* For error messages */
  STRING_Symbol **hash;         /* Array of size hash_size */
  int hash_size;                /* Must be power of 2 */
  int hash_mask;                /* AND mask, (hash_size - 1) */
  int resize_size;              /* When reached, resize hash table */
  STRING_Symbol *head_symbol;   /* Contents list */
  STRING_Symbol *tail_symbol;
  int symbol_count;
}
STRING_Symbol_Table;

#ifdef __cplusplus
extern "C"
{
#endif

  extern STRING_Symbol_Table *STRING_new_symbol_table (char *name,
                                                       int expected_size);
  extern void STRING_delete_symbol_table (STRING_Symbol_Table * table,
                                          void (*free_routine) (void *));
  extern STRING_Symbol *STRING_add_symbol (STRING_Symbol_Table * table,
                                           char *name, void *data);
  extern STRING_Symbol *STRING_find_symbol (STRING_Symbol_Table * table,
                                            char *name);
  extern void *STRING_find_symbol_data (STRING_Symbol_Table * table,
                                        char *name);
  extern void STRING_delete_symbol (STRING_Symbol * symbol,
                                    void (*free_routine) (void *));

/* For debugging hashing function only */
  extern void STRING_print_symbol_table_hash (FILE * out,
                                              STRING_Symbol_Table * table);

#ifdef __cplusplus
}
#endif

#endif
