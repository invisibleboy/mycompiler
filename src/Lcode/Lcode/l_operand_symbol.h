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
 *      File:   l_operand_symbol.h
 *      Author: Stolen by Richard E. Hank from John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  1996
\*****************************************************************************/
#ifndef L_OPERAND_SYMBOL_H
#define L_OPERAND_SYMBOL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

/*
 * INT symbol table structures and prototypes
 */
typedef struct OPERAND_Symbol
{
  int value;                    /* Int value of symbol */
  int hash_val;                 /* Hash value of symbol */
  void *data;                   /* Data struct pointed to */
  struct OPERAND_Symbol_Table *table;   /* For deletion of symbol */
  struct OPERAND_Symbol *next_hash;     /* For table's hash table */
  struct OPERAND_Symbol *prev_hash;
  struct OPERAND_Symbol *next_symbol;   /* For table's contents list */
  struct OPERAND_Symbol *prev_symbol;

}
OPERAND_Symbol;

typedef struct OPERAND_Symbol_Table
{
  char *name;                   /* For error messages */
  OPERAND_Symbol **hash;        /* Array of size hash_size */
  int hash_size;                /* Must be power of 2 */
  int hash_mask;                /* AND mask, (hash_size - 1) */
  int resize_size;              /* When reached, resize hash table */
  OPERAND_Symbol *head_symbol;  /* Contents list */
  OPERAND_Symbol *tail_symbol;
  int symbol_count;
}
OPERAND_Symbol_Table;

#ifdef __cplusplus
extern "C"
{
#endif

  extern OPERAND_Symbol_Table *OPERAND_new_symbol_table (char *name,
                                                         int expected_size);
  extern void OPERAND_delete_symbol_table (OPERAND_Symbol_Table * table,
                                           void (*free_routine) (void *));
  extern void OPERAND_resize_symbol_table (OPERAND_Symbol_Table * table);
  extern OPERAND_Symbol *OPERAND_add_symbol (OPERAND_Symbol_Table * table,
                                             L_Operand * opd, void *data);
  extern OPERAND_Symbol *OPERAND_find_symbol (OPERAND_Symbol_Table * table,
                                              L_Operand * opd);
  extern void *OPERAND_find_symbol_data (OPERAND_Symbol_Table * table,
                                         L_Operand * opd);
  extern void OPERAND_delete_symbol (OPERAND_Symbol * symbol,
                                     void (*free_routine) (void *));
  extern void OPERAND_print_symbol_table_hash (FILE * out,
                                               OPERAND_Symbol_Table * table);

#ifdef __cplusplus
}
#endif

#endif
