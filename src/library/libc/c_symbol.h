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
 *      File:   c_symbol.h
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
#ifndef C_SYMBOL_H
#define C_SYMBOL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/c_basic.h>

/*
 *      Most number of symbol tables that we will allocate.
 */
#define C_MAX_SYMBOL_TABLE      200

/*      
 *      Each symbol is uniquely identified by the symbol name
 *      and the symbol type fields. The ptr field can be used
 *      to connect a user defined data structure to the symbol.
 *      valid types are non-negative.
 */

/*
 *      Create a new symbol table and returns its ID.
 *      Return -1 if anything goes wrong in this function,
 *      for example, not able to allocate space for
 *      storing max_size elements.
 *      Initially, no symbol is defined in the table.
 */
#define C_MATCH_BY_EXACT_TYPE   0       /* match=(type1==type2) */
#define C_MATCH_BY_AND_TYPE     1       /* match=(type1&type2) */

    /* DMG - 5/95 - typedefs moved from .c file to make visible */
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

extern _SymTbl TBL[C_MAX_SYMBOL_TABLE];

#ifdef __cplusplus
extern "C"
{
#endif

  extern int C_open (int max_size, int mode);

/*
 *      Update a symbol table entry.
 *      Add a new symbol in a symbol table if it is not already there.
 */
  extern void C_update (int tbl_id, char *name, int type, Integer value,
                        Pointer ptr);

/*
 *      Delete a symbol entry.
 */
  extern int C_delete (int tbl_id, char *name, int type);
/*
 *      Find attributes of a symbol in a symbol table.
 *      Return 1 if the symbol was found.
 *      Return 0 if the symbol was not found.
 */
  extern int C_find (int tbl_id, char *name, int type, Integer * value,
                     Pointer * ptr);

/*
 *      See if the table has space to hold name
 */
  extern int C_full (int tbl_id, char *name, int type);

/*
 *      Delete all elements from a symbol table.
 */

  extern void C_delete_tbl (int tbl_id);
  extern void C_clear (int tbl_id);
  extern void C_clear_free_all(int tbl_id);

/*
 *      Delete all information from a symbol table.
 *      But keep the symbol entries.
 */
  extern void C_clear_attr (int tbl_id);

/*
 *      Call fn for all entry.
 *      Returns the number of calls made.
 *      If (fn==0) then returns the size of the table.
 */
  extern int C_forall (int tbl_id, C_Function fn);

/*
 *      Find out the index in the hash table
 */
  extern int C_find_index (int tbl_id, void *ptr);
  extern Pointer C_find_from_index (int tbl_id, int index);

/*
 *      Open a name table.
 *      C_unique_name() guarantees a unique address for each name,
 *      except when C_delete() has been called.
 */
  extern int C_open_name_table (int max_size);
  extern char *C_unique_name (int tbl_id, char *name);

  extern void C_clear_free_name(int tbl_id);

#ifdef __cplusplus
}
#endif

#endif
