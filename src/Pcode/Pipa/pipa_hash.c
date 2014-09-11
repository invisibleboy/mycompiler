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
 *      File:    pipa_hash.c
 *      Author:  Erik Nystrom
 *      Copyright (c) 2003  Erik Nystrom, Wen-mei Hwu
 *              and The Board of Trustees of the University of Illinois.
 *              All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include "pipa_hash.h"

#define POOL 1

#if POOL
static L_Alloc_Pool *IPA_htab_entry_pool = NULL;
static L_Alloc_Pool *IPA_htab_cs_pool = NULL;
static L_Alloc_Pool *IPA_htab_pool = NULL;
#endif

/* Define CAST_AS_LVALUE_OK if your compiler accepts casts as lvalues and
 * does not support statement expressions.  This does not need to be defined
 * for gcc or icc.  This must not be defined for gcc >= 4. */
#undef CAST_AS_LVALUE_OK

/* A list of primes just under powers of 2 */
static unsigned int IPA_htab_sizes[] = 
  {1, 2, 5, 11, 
   23, 31, 73, 127, 
   251, 509, 1021, 2039, 
   4093, 8191, 16381, 32749, 
   65521, 131071, 
   0};

static void
IPA_htab_insert_entry(IPA_Hashtab_t *htab,
		      IPA_Hashtab_entry_t *new_entry, unsigned int key);

#define IPA_HTAB_ISLAST(e)      (((long)((e)->item) & IPA_HTAB_LAST) != 0)
#ifdef CAST_AS_LVALUE_OK
#define IPA_HTAB_SETLAST(e)     ((void*)((long)((e)->item) |= IPA_HTAB_LAST))
#else
#define IPA_HTAB_SETLAST(e)     ({ IPA_Hashtab_entry_t *_e = (e); \
                                   long _t = (long)(_e->item); \
                                   _t |= IPA_HTAB_LAST; \
                                   _e->item = (void *)_t; \
                                   _t; })
#endif
#if CAST_AS_LVALUE_OK
#define IPA_HTAB_CLRLAST(e)     ((void*)((long)((e)->item) &= ~IPA_HTAB_LAST))
#else
#define IPA_HTAB_CLRLAST(e)     ({ IPA_Hashtab_entry_t *_e = (e); \
                                   long _t = (long)(_e->item); \
                                   _t &= ~IPA_HTAB_LAST; \
                                   _e->item = (void *)_t; \
                                   _t; })
#endif

static IPA_Hashtab_entry_t *
IPA_htab_entry_new(void *item)
{
  IPA_Hashtab_entry_t *entry;

#if !POOL
  entry = calloc(1,sizeof(IPA_Hashtab_entry_t));
#else
  entry = L_alloc(IPA_htab_entry_pool);
  bzero (entry, sizeof (IPA_Hashtab_entry_t));
#endif
  entry->item = item;

  return entry;
}

static void
IPA_htab_entry_free(IPA_Hashtab_entry_t *entry)
{
  if (!entry)
    return;

#if !POOL
  free(entry);
#else
  L_free(IPA_htab_entry_pool, entry);
#endif
}


/* This returns the first entry in the hash table */
static IPA_Hashtab_entry_t *
IPA_htab_first_filled(IPA_Hashtab_t *htab)
{
  int i, size;

  size = IPA_htab_sizes[htab->tbl_size];

  for (i=0; i < size; i++)
    {
      if (htab->tbl[i] != NULL)
	return htab->tbl[i];
    }

  return NULL;
}

/* If a new entry is being added at index qI this returns
   the entry after which the new one should be inserted 
   (wrt the ->nxt pointer) */
static IPA_Hashtab_entry_t *
IPA_htab_insert_after(IPA_Hashtab_t *htab, int index)
{
  int i, size;

  size = IPA_htab_sizes[htab->tbl_size];

  for (i=index-1; i >= 0; i--)
    {
      if (htab->tbl[i] != NULL)
	{
	  IPA_Hashtab_entry_t *entry;
	  /* Get the last entry in this row */
	  for (entry = htab->tbl[i]; entry; entry = entry->nxt)
	    {
	      if (IPA_HTAB_ISLAST(entry))
		break;	      
	    }
	  assert(entry);
	  return entry;
	}
    }

  return NULL;
}

static void
IPA_htab_check(IPA_Hashtab_t *htab)
{
  IPA_Hashtab_entry_t *prv_entry;
  IPA_Hashtab_entry_t *entry;
  int i, size;
  int cnt;

  size = IPA_htab_sizes[htab->tbl_size];

  /* CSENTRY */
  {
    IPA_Hashtab_cs_t *csptr;
    IPA_Hashtab_entry_t *cs_entry;
    IPA_Hashtab_entry_t *prv_cs_entry;
    cnt = 0;
    for (csptr = htab->cs; csptr; csptr=csptr->nxt)
      {
	for (prv_cs_entry=NULL, cs_entry=csptr->first; cs_entry; 
	     prv_cs_entry=cs_entry, cs_entry=cs_entry->nxt_cs)
	  {
	    assert(cs_entry->prv_cs == prv_cs_entry);
	    cs_entry->item = (void*)((long)cs_entry->item | 0x2);
	    cnt++;
	  }
      }
    assert(cnt == 0 || htab->entries == cnt);
  }

  cnt = 0;
  prv_entry = NULL;
  for (i=0; i<size; i++)
    {
      for (entry = htab->tbl[i]; entry; entry = entry->nxt)
	{
	  assert(!prv_entry || prv_entry->nxt == entry);
	  if (htab->cs)
	    {
	      assert((long)entry->item & 0x2);
	      entry->item = (void*)((long)entry->item & ~0x2);	      
	    }
	  prv_entry = entry;
	  cnt++;
	  if (IPA_HTAB_ISLAST(entry))
	    {
	      break;
	    }
	}
    }
  assert(prv_entry == NULL || prv_entry->nxt == NULL);
  assert(htab->entries == cnt);
}

IPA_Hashtab_t *
IPA_htab_new(int size)
{
  IPA_Hashtab_t *htab = NULL;

  /* Find close prime
   */
  assert(size < 18);
  if (IPA_htab_sizes[size] == 0)
    size--;
  
#if !POOL
  htab = calloc(1, sizeof(IPA_Hashtab_t));
#else
  htab = L_alloc(IPA_htab_pool);
  bzero (htab, sizeof (IPA_Hashtab_t));
#endif

  htab->tbl = calloc(IPA_htab_sizes[size], sizeof(IPA_Hashtab_entry_t*));
  htab->tbl_size = size;
  
  return htab;
}

void
IPA_htab_free(IPA_Hashtab_t *htab)
{
  IPA_Hashtab_entry_t *entry;
  IPA_Hashtab_entry_t *nxt_entry;
  int cnt = 0;

  if (!htab)  
    return;

  for (entry = IPA_htab_first_filled(htab); entry; entry=nxt_entry)
    {
      nxt_entry = entry->nxt;
      IPA_htab_entry_free(entry);
      cnt++;
    }
  
  /*printf("Freed %d, %d entries\n", size, cnt);*/
  free(htab->tbl);
#if !POOL
  free(htab);
#else
  L_free(IPA_htab_pool, htab);
#endif
}

IPA_Hashtab_cs_t*
IPA_htab_cs_new(IPA_Hashtab_t *htab, int cs)
{
  /* Add new class */
  IPA_Hashtab_cs_t *csptr;

#if !POOL
  csptr = calloc(1,sizeof(IPA_Hashtab_cs_t));
#else
  csptr = L_alloc(IPA_htab_cs_pool);
  bzero (csptr, sizeof (IPA_Hashtab_cs_t));
#endif

  csptr->cs = cs;
  csptr->nxt = htab->cs;
  htab->cs = csptr;

  return csptr;
}

#define IPA_htab_gethash(htab, key)  (((unsigned int)key) % IPA_htab_sizes[htab->tbl_size])

static IPA_Hashtab_t *
IPA_htab_rehash(IPA_Hashtab_t *htab)
{
  IPA_Hashtab_t       *new_htab;
  IPA_HTAB_ITER        iter;

  if (htab->blocked)
    return htab;

  /* New Table */
  new_htab = IPA_htab_new(htab->tbl_size + 1);

  IPA_HTAB_START(iter, htab);
  IPA_HTAB_LOOP(iter)
    {
      IPA_htab_insert_entry(new_htab, 
			    iter.entry, 
			    iter.entry->key);
    }

  /* Free old table */
  free(htab->tbl);

  /* Switch guts */
  htab->entries = new_htab->entries;
  htab->tbl_size = new_htab->tbl_size;
  htab->tbl = new_htab->tbl;
 
  /* Free new_htab wrapper */
#if !POOL
  free(new_htab);
#else
  L_free(IPA_htab_pool, new_htab);
#endif

  return htab;  
}

void
IPA_htab_blockrehash(IPA_Hashtab_t *htab)
{
  if (!htab)
    return;
  htab->blocked = 1;
}

void
IPA_htab_unblockrehash(IPA_Hashtab_t *htab)
{
  if (!htab)
    return;
  htab->blocked = 0;
  if ((htab->entries >> 4) > IPA_htab_sizes[htab->tbl_size])
    {
      if (htab->tbl_size < 17)
	htab = IPA_htab_rehash(htab);
    }
}

IPA_Hashtab_t *
IPA_htab_insert(IPA_Hashtab_t *htab, void *item, unsigned int key)
{
  IPA_Hashtab_entry_t *new_entry;

  assert(htab);
  assert(htab->cs == NULL);

  new_entry = IPA_htab_entry_new(item);
  new_entry->key = key;
  IPA_htab_insert_entry(htab, new_entry, key);

  if ((htab->entries >> 4) > IPA_htab_sizes[htab->tbl_size])
    {
      if (htab->tbl_size < 17)
	htab = IPA_htab_rehash(htab);
    }

#if 0
  IPA_htab_check(htab);
#endif
  return htab;
}

IPA_Hashtab_t *
IPA_htab_insert_cs(IPA_Hashtab_t *htab, void *item, unsigned int key, int cs)
{
  IPA_Hashtab_entry_t *new_entry;

  assert(htab);
  new_entry = IPA_htab_entry_new(item);
  new_entry->key = key;
  IPA_htab_insert_entry(htab, new_entry, key);

  if ((htab->entries >> 4) > IPA_htab_sizes[htab->tbl_size])
    {
      if (htab->tbl_size < 17)
	htab = IPA_htab_rehash(htab);
    }

  /* CSENTRY */
  {
    IPA_Hashtab_cs_t *csptr;
#if 0
    int i = 0;
#endif

    assert(cs != IPA_HTAB_CS_ALL);
#if 0
    printf("CS: ");
#endif
    for (csptr = htab->cs; csptr; csptr=csptr->nxt)
      {
#if 0
	printf("%d ",csptr->cs);
	i++;
#endif
	if (csptr->cs == cs)
	  break;
      }
#if 0
    assert(i<30);
    printf("\n");
#endif

    if (!csptr)
      {
	/* Add new class */
	csptr = IPA_htab_cs_new(htab, cs);
      }
    /* Add to class entry list */
    new_entry->nxt_cs = csptr->first;
    if (csptr->first)
      csptr->first->prv_cs = new_entry;
    csptr->first = new_entry;
  }

#if 0
  IPA_htab_check(htab);
#endif

  return htab;  
}

static void
IPA_htab_insert_entry(IPA_Hashtab_t *htab,
		      IPA_Hashtab_entry_t *new_entry, 
		      unsigned int key)
{
  unsigned int hash;
  IPA_Hashtab_entry_t *prev_entry;

  hash = IPA_htab_gethash(htab, key);

  if ((prev_entry = IPA_htab_insert_after(htab, hash)))
    {
      /* Insert after previous index's entry */
      new_entry->nxt = prev_entry->nxt;
      /* The prev_entry should always be the last at its index */
      assert(IPA_HTAB_ISLAST(prev_entry));
      prev_entry->nxt = new_entry;
    }
  else
    {
      /* Insert as first entry */
      new_entry->nxt = IPA_htab_first_filled(htab);
    }

  /* Is this the first entry at this index */
  if (htab->tbl[hash] == NULL)
    {
      IPA_HTAB_SETLAST(new_entry);
    }
  else
    IPA_HTAB_CLRLAST(new_entry);
  htab->tbl[hash] = new_entry;
  htab->entries++;
}

void*
IPA_htab_remove(IPA_Hashtab_t *htab, unsigned int key, 
		int(*cmpfn)(void*))
{
  IPA_Hashtab_entry_t *prv_entry;
  IPA_Hashtab_entry_t *cur_entry;
  unsigned int hash;
  void *ret;

  hash = IPA_htab_gethash(htab, key);
  prv_entry = NULL;
  cur_entry = htab->tbl[hash];

  /* Locate the entry to be deleted */
  while (cur_entry && (cur_entry->key != key || !(*cmpfn)(IPA_HTAB_MASKED_ITEM(cur_entry))))
    {
      prv_entry = cur_entry;
      if (IPA_HTAB_ISLAST(cur_entry))
	return NULL;
      cur_entry = cur_entry->nxt;
    }

  if (!cur_entry)
    return NULL;

  if (prv_entry)
    {
      /* There is a prev entry at the same index */
      assert(prv_entry->nxt == cur_entry);
      prv_entry->nxt = cur_entry->nxt;
      if (IPA_HTAB_ISLAST(cur_entry))
	{
	  IPA_HTAB_SETLAST(prv_entry);
	}
    }
  else
    {
      /* Locate the preceding entry at a preceding index */
      assert(htab->tbl[hash] == cur_entry);
      prv_entry = IPA_htab_insert_after(htab, hash);
      
      assert(!prv_entry || prv_entry->nxt == cur_entry);

      /* cur entry is the first at this index */
      if (IPA_HTAB_ISLAST(cur_entry))
	{
	  htab->tbl[hash] = NULL;
	}
      else
	htab->tbl[hash] = cur_entry->nxt;
      
      if (prv_entry)
	{
	  prv_entry->nxt = cur_entry->nxt;
	}
    }
  htab->entries--;

  /* CSENTRY */
  {
    if (htab->cs)
      {	
	if (cur_entry->nxt_cs)
	  cur_entry->nxt_cs->prv_cs = cur_entry->prv_cs;

	if (cur_entry->prv_cs)
	  {
	    cur_entry->prv_cs->nxt_cs = cur_entry->nxt_cs;
	  }
	else
	  {
	    IPA_Hashtab_cs_t *csptr;
	    IPA_Hashtab_cs_t *prev_csptr;
	    for (prev_csptr=NULL, csptr = htab->cs; 
		 csptr; prev_csptr=csptr, csptr=csptr->nxt)
	      {
		if (csptr->first == cur_entry)
		  break;
	      }
	    assert(csptr);

	    csptr->first = cur_entry->nxt_cs;
	    if (csptr->first == NULL)
	      {
		if (prev_csptr)
		  {
		    prev_csptr->nxt = csptr->nxt;
		  }
		else
		  {
		    htab->cs = csptr->nxt;
		  }
#if !POOL
		free(csptr);
#else
		L_free(IPA_htab_cs_pool, csptr);
#endif
	      }
	  }
      }
  }
	 
  ret = IPA_HTAB_MASKED_ITEM(cur_entry);
  IPA_htab_entry_free(cur_entry);
#if 0
  IPA_htab_check(htab);
#endif
  return ret;
}


void*
IPA_htab_find(IPA_Hashtab_t *htab, unsigned int key,
	      int(*cmpfn)(void*))
{
  IPA_Hashtab_entry_t *cur_entry;
  unsigned int hash;

  if (!htab)
    return NULL;

  hash = IPA_htab_gethash(htab, key);
  cur_entry = htab->tbl[hash];
  while (cur_entry)
    {
      if ((cur_entry->key == key) && (*cmpfn)(IPA_HTAB_MASKED_ITEM(cur_entry)))
	return IPA_HTAB_MASKED_ITEM(cur_entry);
      if (IPA_HTAB_ISLAST(cur_entry))
	return NULL;
      cur_entry = cur_entry->nxt;
    }

  return NULL;
}

IPA_Hashtab_entry_t *
IPA_htab_first(IPA_Hashtab_t *htab, int cs)
{
  if (!htab)
    return NULL;

  if (cs == IPA_HTAB_CS_ALL)
    return IPA_htab_first_filled(htab);

  /* CSENTRY */
  {
    IPA_Hashtab_cs_t *csptr;
    assert(cs != IPA_HTAB_CS_ALL);
    for (csptr = htab->cs; csptr; csptr=csptr->nxt)
      {
	if (csptr->cs == cs)
	  return csptr->first;
      }
  }
  return NULL;
}

IPA_Hashtab_entry_t *
IPA_htab_nxt(IPA_Hashtab_t *htab, IPA_Hashtab_entry_t *entry)
{
  return entry->nxt;
}


void
IPA_htab_print(IPA_Hashtab_t *htab)
{
  IPA_Hashtab_entry_t *entry;
  int i, size;
  int len;
  int min=100000000, max=0;

  if (htab)
    {
      size = IPA_htab_sizes[htab->tbl_size];
      
      for (i=0; i<size; i++)
	{
	  for (len=0, entry=htab->tbl[i];
	       entry; len++, entry=entry->nxt)
	    {
	      printf("%p \n",IPA_HTAB_MASKED_ITEM(entry));
	      if (IPA_HTAB_ISLAST(entry))
		break;
	    }
	  
	  min = Min(min, len);
	  max = Max(max, len);
	}

      printf("Table %d, Entries %d, Best %d :  Min %d, Max %d \n",
	     size, htab->entries, 
	     (htab->entries / size) + 1,
	     min, max);
    }
}

int
IPA_htab_size(IPA_Hashtab_t *htab)
{
  if (!htab)
    return 0;
  return htab->entries;
}

List
IPA_htab2list(IPA_Hashtab_t *htab)
{
  IPA_Hashtab_entry_t *entry;
  List list = NULL;

  for (entry = IPA_htab_first(htab,IPA_HTAB_CS_ALL);
       entry;
       entry = IPA_htab_nxt(htab, entry))
    {
      list = List_insert_last(list, IPA_HTAB_MASKED_ITEM(entry));
    }

  return list;
}

#if POOL
void
IPA_htab_minit ()
{
  static char init = 0;

  if (init)
    P_punt ("IPA_htab_minit: already called\n");
  init = 1;

  IPA_htab_entry_pool =
    L_create_alloc_pool ("htab_entry", sizeof (IPA_Hashtab_entry_t), 16*1024);
  IPA_htab_cs_pool =
    L_create_alloc_pool ("htab_cs", sizeof (IPA_Hashtab_cs_t), 16*1024);
  IPA_htab_pool =
    L_create_alloc_pool ("htab", sizeof (IPA_Hashtab_t), 16*1024);
}

void
IPA_htab_pool_info()
{
  L_print_alloc_info(stdout,  IPA_htab_entry_pool, 1); 
  L_print_alloc_info(stdout,  IPA_htab_cs_pool, 1); 
  L_print_alloc_info(stdout,  IPA_htab_pool, 1); 
}

void
IPA_htab_mfree ()
{
  fflush(stdout);
  fflush(stderr);

  L_free_alloc_pool (IPA_htab_entry_pool);
  IPA_htab_entry_pool = NULL;
  L_free_alloc_pool (IPA_htab_cs_pool);
  IPA_htab_cs_pool = NULL;
  L_free_alloc_pool (IPA_htab_pool);
  IPA_htab_pool = NULL;
}
#endif
