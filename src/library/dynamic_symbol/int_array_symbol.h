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
 *      File:   int_array_symbol.h
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  Sept. 1998 (adapted from string_array.h)
 *      Copyright (c) 1998 The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef INT_ARRAY_SYMBOL_H
#define INT_ARRAY_SYMBOL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 * INT_ARRAY symbol table structures and prototypes
 */
typedef struct INT_ARRAY_Symbol
{
  int *int_array;               /* int array values of symbol */
  int array_length;             /* Length of int_array */
  unsigned int hash_val;        /* Hashed value of int_array */
  void *data;                   /* Data struct pointed to */
  struct INT_ARRAY_Symbol_Table *table; /* For deletion of symbol */
  struct INT_ARRAY_Symbol *next_hash;   /* For table's hash table */
  struct INT_ARRAY_Symbol *prev_hash;
  struct INT_ARRAY_Symbol *next_symbol; /* For table's contents list */
  struct INT_ARRAY_Symbol *prev_symbol;

}
INT_ARRAY_Symbol;

typedef struct INT_ARRAY_Symbol_Table
{
  char *name;                   /* For error messages */
  INT_ARRAY_Symbol **hash;      /* Array of size hash_size */
  int hash_size;                /* Must be power of 2 */
  int hash_mask;                /* AND mask, (hash_size - 1) */
  int resize_size;              /* When reached, resize hash table */
  INT_ARRAY_Symbol *head_symbol;        /* Contents list */
  INT_ARRAY_Symbol *tail_symbol;
  int symbol_count;
}
INT_ARRAY_Symbol_Table;

#ifdef __cplusplus
extern "C"
{
#endif

  extern INT_ARRAY_Symbol_Table *INT_ARRAY_new_symbol_table (char *name,
                                                             int
                                                             expected_size);
  extern void INT_ARRAY_delete_symbol_table (INT_ARRAY_Symbol_Table * table,
                                             void (*free_routine) (void *));
  extern INT_ARRAY_Symbol *INT_ARRAY_add_symbol (INT_ARRAY_Symbol_Table *
                                                 table, int *int_array,
                                                 int array_length,
                                                 void *data);
  extern INT_ARRAY_Symbol *INT_ARRAY_find_symbol (INT_ARRAY_Symbol_Table *
                                                  table, int *int_array,
                                                  int array_length);
  extern void *INT_ARRAY_find_symbol_data (INT_ARRAY_Symbol_Table * table,
                                           int *int_array, int array_length);
  extern void INT_ARRAY_delete_symbol (INT_ARRAY_Symbol * symbol,
                                       void (*free_routine) (void *));

#ifdef __cplusplus
}
#endif


#endif
