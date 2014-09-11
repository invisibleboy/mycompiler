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
 *      File:   psymbol.h
 * 
 *      Description: Header file for psymbol.c
 * 
 *      Creation Date:  October 1994
 * 
 *      Authors: John C. Gyllenhaal and Wen-mei Hwu
 *
 *      Revisions:
 *  
\*****************************************************************************/

#ifndef PSYMBOL_H
#define PSYMBOL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <library/l_alloc_new.h>

#define PSYMBOL_HASH_SIZE	32	/* Must be power of 2 */

/* Low-level symbol table structures, used for building the various
 * preprocessor symbol tables.
 */
typedef struct Psymbol
{
  char *name;
  void *data;
  struct Psymbol_Table *table;
  struct Psymbol *next_hash;
  struct Psymbol *prev_hash;
  struct Psymbol *next_symbol;
  struct Psymbol *prev_symbol;
}
Psymbol;

typedef struct Psymbol_Table
{
  char *name;
  Psymbol *hash[PSYMBOL_HASH_SIZE];
  Psymbol *head;
  Psymbol *tail;
  int count;
}
Psymbol_Table;



/* Export pools for debugging purposes */
extern L_Alloc_Pool *Psymbol_Table_pool;
extern L_Alloc_Pool *Psymbol_pool;

#ifdef __cplusplus
extern "C"
{
#endif

/* Low-level symbol table prototypes */
  extern Psymbol_Table *create_Psymbol_Table (char *name);
  extern void free_Psymbol_Table (Psymbol_Table * table,
				  void (*free_routine) (void *));
  extern int hash_Psymbol_name (char *name);
  extern void add_Psymbol (Psymbol_Table * table, char *name, void *data);
  extern Psymbol *find_Psymbol (Psymbol_Table * table, char *name);
  extern void delete_Psymbol (Psymbol * symbol,
			      void (*free_routine) (void *));
  extern void print_Psymbol_hash_table (FILE * out, Psymbol_Table * table);

#ifdef __cplusplus
}
#endif


#endif
