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


#ifndef __P_CF_hash__
#define __P_CF_hash__

#include <stdlib.h>
#include <string.h>

/*****************************************************************/

#define HASH_TABLE_NAME_LENGTH	512

typedef struct _AddrHashTableEntry
{
  void *key;
  void *value;
  struct _AddrHashTableEntry *next;
}
_AddrHashTableEntry, *AddrHashTableEntry;

typedef struct _AddrHashTable
{
  char name[HASH_TABLE_NAME_LENGTH];
  int size;
  AddrHashTableEntry *entry;
}
_AddrHashTable, *AddrHashTable;

typedef enum {
   DUMP_SUCC_VNODE = 1
} DumpVnodeFormat;

/*****************************************************************/

extern AddrHashTable NewAddrHashTable (int size, char *name);
extern void DeleteAddrHashTable (AddrHashTable tbl, void (*f_delete_value)(void *value));
extern AddrHashTableEntry FindAndCreateAddrHashTableEntry (AddrHashTable tbl, void *key);
extern AddrHashTableEntry GetAddrHashTableEntry (AddrHashTable tbl, void *key);
extern void *RemoveHashTableEntry (AddrHashTable tbl, void *key);
extern void Dump_AddrHashTable (FILE *out_file, AddrHashTable tbl, char *title, void (*dump_func)(FILE *, void *));

#endif /* __P_CF_hash__ */
