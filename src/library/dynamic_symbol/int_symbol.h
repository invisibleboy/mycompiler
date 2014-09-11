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
 *      File:   int_symbol.h
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  1995 (Split into separate files Sept. 1998 -JCG)
 *      Copyright (c) 1995 The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef INT_SYMBOL_H
#define INT_SYMBOL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 * INT symbol table structures and prototypes
 */
typedef struct INT_Symbol
{
  /* 20020909 SZU
   * Change to long for 64-bit pure
   * 20040112 SZU
   * long could have portability issues; couldn't find reason for fix.
   * Therefore change back to int.
   */
  int value;                    /* Int value of symbol */
  //long value;                    /* Int value of symbol */
  void *data;                   /* Data struct pointed to */
  struct INT_Symbol_Table *table;       /* For deletion of symbol */
  struct INT_Symbol *next_hash; /* For table's hash table */
  struct INT_Symbol *prev_hash;
  struct INT_Symbol *next_symbol;       /* For table's contents list */
  struct INT_Symbol *prev_symbol;

}
INT_Symbol;

typedef struct INT_Symbol_Table
{
  char *name;                   /* For error messages */
  INT_Symbol **hash;            /* Array of size hash_size */
  int hash_size;                /* Must be power of 2 */
  int hash_mask;                /* AND mask, (hash_size - 1) */
  int resize_size;              /* When reached, resize hash table */
  INT_Symbol *head_symbol;      /* Contents list */
  INT_Symbol *tail_symbol;
  int symbol_count;
}
INT_Symbol_Table;

#ifdef __cplusplus
extern "C"
{
#endif

  extern INT_Symbol_Table *INT_new_symbol_table (char *name,
                                                 int expected_size);
  extern void INT_delete_symbol_table (INT_Symbol_Table * table,
                                       void (*free_routine) (void *));
  extern INT_Symbol *INT_add_symbol (INT_Symbol_Table * table, int value,
                                     void *data);
  extern INT_Symbol *INT_find_symbol (INT_Symbol_Table * table, int value);
  extern void *INT_find_symbol_data (INT_Symbol_Table * table, int value);
  extern void INT_delete_symbol (INT_Symbol * symbol,
                                 void (*free_routine) (void *));

/* For debugging hashing function only */
  extern void INT_print_symbol_table_hash (FILE * out,
                                           INT_Symbol_Table * table);

#ifdef __cplusplus
}
#endif

#endif
