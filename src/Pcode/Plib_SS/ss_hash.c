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


#include <stdio.h>
#include <stdlib.h>
#include <Pcode/struct.h>
#include "ss_hash.h"

/**********************************************************************************
        Static/private variables
***********************************************************************************/


/**********************************************************************************
        Static/private function header
***********************************************************************************/

static int AddrHashKey(int size, void *key);
static AddrHashTableEntry NewAddrHashTableEntry(void *key, void *value, AddrHashTableEntry next);


/**********************************************************************************
        Export function body
***********************************************************************************/

AddrHashTable
NewAddrHashTable(int size, char *name)
{
  AddrHashTable new_table;

  assert (size > 0);
  new_table = ALLOCATE(_AddrHashTable);
  if (name) { 
    if (strlen(name) > HASH_TABLE_NAME_LENGTH-1)
      P_punt("NewAddrHashTable: name too long");
    sprintf(new_table->name, "%s", name);
  }   
  new_table->size = size;
  new_table->entry = calloc((size_t) size, sizeof(AddrHashTableEntry));
  return new_table;
}

void
DeleteAddrHashTable(AddrHashTable tbl, void (*f_delete_value)(void *value))
{
  int i;
  AddrHashTableEntry curr_entry;
  AddrHashTableEntry prev_entry;

  for (i = 0 ; i < tbl->size ; i++)
    {
      if (tbl->entry[i])
        {
          curr_entry = tbl->entry[i];
          while (curr_entry)
            {
              if (f_delete_value)
                (*f_delete_value)(curr_entry->value);
              curr_entry->key = 0;
              curr_entry->value = 0;
              prev_entry = curr_entry;
              curr_entry = curr_entry->next;
              DISPOSE(prev_entry);
            }
        }
    }

  free(tbl->entry);
  DISPOSE(tbl);
}

AddrHashTableEntry
FindAndCreateAddrHashTableEntry(AddrHashTable tbl, void *key)
{
  int index;
  AddrHashTableEntry entry;

  index = AddrHashKey(tbl->size, key);
  entry = tbl->entry[index];
  while (entry)
    {
      if (entry->key == key)
        return entry;
      entry = entry->next;
    }
  tbl->entry[index] = entry = NewAddrHashTableEntry(key, NULL, tbl->entry[index]);
  return entry;
}

AddrHashTableEntry
GetAddrHashTableEntry(AddrHashTable tbl, void *key)
{
  int index;
  AddrHashTableEntry entry;

  index = AddrHashKey(tbl->size, key);
  entry = tbl->entry[index];
  while (entry)
    {
      if (entry->key == key)
        return entry;
      entry = entry->next;
    }
  return NULL;
}

void *
RemoveHashTableEntry(AddrHashTable tbl, void *key)
{
  int index;
  void *value;
  AddrHashTableEntry entry;
  AddrHashTableEntry prev_entry;

  index = AddrHashKey(tbl->size, key);
  prev_entry = NULL;
  entry = tbl->entry[index];
  while (entry && (entry->key != key)) 
    {
      prev_entry = entry;
      entry = entry->next;
    }
  if (entry)
    {
      if (prev_entry)
        prev_entry->next = entry->next;
      else 
        tbl->entry[index] = entry->next;
      value = entry->value;
      DISPOSE(entry);
      return value; 
    }
  else
    {
      return NULL;
    }
}

void
Dump_AddrHashTable(FILE *out_file, AddrHashTable tbl, char *title, void (*dump_entry_func) (FILE *, void *))
{
  int i;
  AddrHashTableEntry entry;

  fprintf(out_file, "%s", title);
  fprintf(out_file, ">>>   %s   <<<\n", tbl->name);
  for (i = 0 ; i < tbl->size ; i++)
    {
      if (tbl->entry[i])
        {
          fprintf(out_file, "%d: ", i);
          entry = tbl->entry[i];
          while (entry)
            {
#if LP64_ARCHITECTURE
              fprintf(out_file, "[key = 0x%lx]\n", (long) entry->key);
#else
              fprintf(out_file, "[key = 0x%x]\n", (int) entry->key);
#endif
              fprintf(out_file, "[value = ");
              if (dump_entry_func) 
                (*dump_entry_func) (out_file, entry->value);
              fprintf(out_file, "]\n\n");
              entry = entry->next;
            }
        }
    }
}

/**********************************************************************************
        Static function body
***********************************************************************************/

static int
AddrHashKey(int size, void *key)
{
#if LP64_ARCHITECTURE
  return ((int)((long) key) % size);
#else
  return ((int) key) % size;
#endif
}

static AddrHashTableEntry
NewAddrHashTableEntry(void *key, void *value, AddrHashTableEntry next)
{
  AddrHashTableEntry entry;

  entry = ALLOCATE(_AddrHashTableEntry);
  entry->key = key;
  entry->value = value;
  entry->next = next;
  return entry;
}


