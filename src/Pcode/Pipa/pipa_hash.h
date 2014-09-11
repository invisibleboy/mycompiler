/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File:    pipa_hash.h
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/


#ifndef _PIPA_HASH_H_
#define _PIPA_HASH_H_

#include "pipa_common.h"

typedef struct IPA_Hashtab_entry_t
{
  /* THIS IS NOT A UNIQUE KEY, JUST CLOSE */
  int                         key;
  void                       *item;
  struct IPA_Hashtab_entry_t *nxt;
  struct IPA_Hashtab_entry_t *nxt_cs;
  struct IPA_Hashtab_entry_t *prv_cs;
} IPA_Hashtab_entry_t;

typedef struct IPA_Hashtab_cs_t
{
  int cs;
  struct IPA_Hashtab_entry_t *first;
  struct IPA_Hashtab_cs_t    *nxt;
} IPA_Hashtab_cs_t;

typedef struct IPA_Hashtab_t
{
  IPA_Hashtab_entry_t **tbl;
  IPA_Hashtab_cs_t     *cs;
  unsigned int          blocked:1, tbl_size:8, entries:23;
} IPA_Hashtab_t;

typedef struct IPA_Hashtab_Iter_t
{ 
  IPA_Hashtab_t       *htab;
  IPA_Hashtab_entry_t *entry;
  IPA_Hashtab_entry_t *nxt_entry;
} IPA_Hashtab_Iter_t;

#define IPA_HTAB_CS_ALL   -2
#define IPA_HTAB_CS_MULTI -1

#define IPA_HTAB_LAST           (0x1)
#define IPA_HTAB_MASKED_ITEM(e) ((void*)((long)((e)->item) & ~IPA_HTAB_LAST))

#define IPA_HTAB_ITER           IPA_Hashtab_Iter_t

#define IPA_HTAB_START(x,t)   ((x.htab) = (t))
#define IPA_HTAB_LOOP(x)                                        \
  for ((x.entry) = IPA_htab_first((x.htab), IPA_HTAB_CS_ALL);   \
       (x.entry) && (((x.nxt_entry) = (x.entry->nxt)) || 1);    \
       (x.entry) = (x.nxt_entry))

#define IPA_HTAB_CS_LOOP(x,c)                                   \
  for ((x.entry) = IPA_htab_first((x.htab),c);                  \
       (x.entry) && (((x.nxt_entry) = (x.entry->nxt_cs)) || 1); \
       (x.entry) = (x.nxt_entry))

#define IPA_HTAB_CUR(x)  \
  IPA_HTAB_MASKED_ITEM(x.entry)


#define IPA_HTAB_FIRST(t) \
   IPA_HTAB_MASKED_ITEM(IPA_htab_first((t), IPA_HTAB_CS_ALL))

IPA_Hashtab_t *
IPA_htab_new(int size);

void
IPA_htab_free(IPA_Hashtab_t *htab);



IPA_Hashtab_t *
IPA_htab_insert(IPA_Hashtab_t *htab, void *item, unsigned int key);

IPA_Hashtab_t *
IPA_htab_insert_cs(IPA_Hashtab_t *htab, void *item, unsigned int key, int cs);

void*
IPA_htab_remove(IPA_Hashtab_t *htab, unsigned int key, 
		int(*cmpfn)(void*));

void*
IPA_htab_find(IPA_Hashtab_t *htab, unsigned int key,
	      int(*cmpfn)(void*));

IPA_Hashtab_entry_t *
IPA_htab_first(IPA_Hashtab_t *htab, int cs);

IPA_Hashtab_entry_t *
IPA_htab_nxt(IPA_Hashtab_t *htab, IPA_Hashtab_entry_t *entry);



int
IPA_htab_size(IPA_Hashtab_t *htab);

void
IPA_htab_print(IPA_Hashtab_t *htab);

List
IPA_htab2list(IPA_Hashtab_t *htab);

void
IPA_htab_blockrehash(IPA_Hashtab_t *htab);

void
IPA_htab_unblockrehash(IPA_Hashtab_t *htab);

void
IPA_htab_minit ();

void
IPA_htab_pool_info();

void
IPA_htab_mfree ();

#endif
