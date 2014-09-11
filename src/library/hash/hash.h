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
 *      File:   hash.h
 *      Author: Pohua Paul Chang
 *      Copyright (c) 1991 Pohua Paul Chang, Wen-Mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_HASH_H
#define IMPACT_HASH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*      
 *      Each entry is uniquely identified by the hash key
 *      and the type field. The ptr field can be used
 *      to connect a user defined data structure to the symbol.
 *      valid types are non-zero.
 */
#define INVALID_ENTRY   -1
typedef struct _Entry
{
  int key;                      /* hash key */
  int type;                     /* symbol type */
  int value;                    /* a general purpose value carrier */
  void * ptr;                  /* a general purpose pointer */
}
_Entry, *Entry;

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *      Create a new hash table and returns its ID.
 *      Return -1 if anything goes wrong in this function,
 *      for example, not able to allocate space for
 *      storing max_size elements.
 *      Initially, no symbol is defined in the table.
 *      It is highly recommanded that max_size be a prime number.
 */
  extern int NewHashTbl (int max_size);

/*
 *      Add a new symbol in a symbol table if it is not already there.
 *      Return a pointer to that symbol. For a newly allocated
 *      symbol, the ptr field is preset to 0.
 */
  extern Entry AddEntry (int tbl_id, int key, int type);

/*
 *      Find an entry in a hash table.
 *      Return a pointer to that symbol, if found.
 *      Otherwise, return 0.
 */
  extern Entry FindEntry (int tbl_id, int key, int type);

/*
 *      Completely erase the content of a symbol table.
 */
  extern void ClearHashTbl (int tbl_id);

#ifdef __cplusplus
}
#endif

#endif
