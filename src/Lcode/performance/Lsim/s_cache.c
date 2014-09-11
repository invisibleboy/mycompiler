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
/****************************************************************************\
 *      File:   s_cache.c
 *      Author: John Gyllenhaal
 *      Creation Date:  1993
 *      Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and The Board of
 *                         Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include "s_main.h"

/* 
 * cache_size and block_size are in bytes.
 * assoc may be 0 to signal full assoc.
 * create_data is a routine that allocates and initializes the
 * data needed by the cache.  The pointer returned will be 
 * placed in in data.  If NULL is passed in create_data, 
 * data will be initialized to NULL;
 */
Scache *S_create_cache (cache_size, block_size, assoc, create_data)
int cache_size;
int block_size;
int assoc;
void *(*create_data)();
{
    Scache *cache;
    int num_of_blocks, tag_store_size, hash_tab_size;
    int tsize, hsize, bsize;
    int index, i;
    Ststore *tstore;
    Scblock *block = NULL;
 
    /* None of the values may be negative (or zero in some cases) */
    if ((cache_size <= 0) || (block_size <= 0) || (assoc < 0))
	S_punt ("S_create_cache: zero or negative sized cache not allowed.\n");

    /* Calculate size parameters */
    num_of_blocks = cache_size / block_size;

    /* Allow 0 assoc to signal full associativity */
    if (assoc == 0)
	assoc = num_of_blocks;

    tag_store_size = num_of_blocks / assoc;

    /* Make sure everything divides evenly */
    if ((num_of_blocks * block_size) != cache_size)
	S_punt ("S_create_cache: cache and block sizes don't divide evenly.");

    if ((tag_store_size * assoc) != num_of_blocks)
	S_punt ("S_create_cache: # of blocks and assoc don't divide evenly.");
   
    /* Block and tag_store size must be powers of two */
    if (!S_is_power_of_two (block_size))
	S_punt ("S_create_cache: block_size (%i) must be power of 2.",
		block_size);

    if (!S_is_power_of_two (tag_store_size))
	S_punt ("S_create_cache: (# of blocks/assoc) (%i) must be power of 2.",
		tag_store_size);

    /*
     * Make the hash table size as least a big as the number of blocks and
     * also make it a power of 2.
     */
    if (S_is_power_of_two (num_of_blocks))
	hash_tab_size = num_of_blocks;
    else
	hash_tab_size = 1 << (S_log_base_two (num_of_blocks) + 1);

    /* Create a new cache */
    cache = L_alloc (Scache_pool);

    cache->block_size = block_size;
    cache->cache_size = cache_size;
    cache->assoc = assoc;
    cache->block_id_shift = S_log_base_two (block_size);
    cache->start_addr_mask = ~(block_size -1);
    cache->tag_index_mask = tag_store_size -1;
    cache->hash_index_mask = hash_tab_size -1;

    tsize = sizeof (Ststore) * tag_store_size;
    hsize = sizeof (Scblock *) * hash_tab_size;
    bsize = sizeof (Scblock) * num_of_blocks;

    if (((cache->tag_store = (Ststore *) malloc (tsize)) == NULL) ||
	((cache->hash = (Scblock **) malloc (hsize)) == NULL) ||
	((block = (Scblock *) malloc (bsize)) == NULL))
	S_punt ("Out of memory");

    for (index = 0; index < tag_store_size; index++)
    {
	tstore = &cache->tag_store[index];

	tstore->head = NULL;
	tstore->tail = NULL;

	/* Add 'assoc' blocks to tstore linked list */
	for (i = 0; i < assoc; i++)
	{
	    /* Add block to tstore linked list */
	    if (tstore->head == NULL)
		tstore->tail = block;
	    else
		tstore->head->tag_prev = block;
	    block->tag_next = tstore->head;
	    block->tag_prev = NULL;
	    tstore->head = block;

	    /* Create data for block (if function ptr not NULL) */
	    if (create_data != NULL)
		block->data = create_data();
	    else
		block->data = NULL;

	    /* Set start addr and hash pointers to -1 to signify unused */
	    block->start_addr = -1;
	    block->hash_next = (Scblock *)-1;
	    block->hash_prev = (Scblock *)-1;

	    /* Goto next block */
	    block++;
	}
    }

    /* Initialize hash table to all NULL */
    for (i=0; i < hash_tab_size; i++)
	cache->hash[i] = NULL;

    return (cache);
}

/* 
 * Returns a pointer to the block that 'addr' falls into.
 * Returns NULL if that block is not in the cache.
 */
Scblock *S_cache_find_addr (cache, addr)
Scache *cache;
unsigned addr;
{
    unsigned start_addr, hash_index;
    Scblock *block;

    /* Get the start_addr for the block 'addr' falls into */
    start_addr = addr & cache->start_addr_mask;

    /* Get the hash index */
    hash_index = (addr >> cache->block_id_shift) & cache->hash_index_mask;

    /* Search all blocks at this hash entry */
    for (block = cache->hash[hash_index]; block != NULL; 
	 block = block->hash_next)
    {
	/* Break out if block with start_addr found */
	if (block->start_addr == start_addr)
	    break;
    }
    return (block);
}

/*
 * Gets the least recently used block from the cache line the 
 * for_addr would map to.  Should be used in conjunction with
 * S_cache_change_addr().
 */
Scblock *S_cache_find_LRU (Scache *cache, unsigned int for_addr)
{
    unsigned start_addr, tag_index;
    Ststore *tstore;

    /* Get the start_addr for the block 'addr' falls into */
    start_addr = for_addr & cache->start_addr_mask;

    /* Get the hash index */
    tag_index = (for_addr >> cache->block_id_shift) & cache->tag_index_mask;

    /* Get tag store for that this addr indexes */
    tstore = &cache->tag_store[tag_index];

    /* Return tail of linked list (LRU) */
    return (tstore->tail);
}

/*
 * Must be used when changing the start_addr of a block!
 * The new_addr be a valid addr for the cache line the
 * block is out of (assumed to be gotten with S_cache_find_LRU()).
 *
 * Updates the hash table that S_cache_find_addr uses to quickly 
 * finds blocks and changes block->new_addr.
 */
void S_cache_change_addr (Scache *cache, Scblock *block, unsigned new_addr)
{
    unsigned hash_index;
    
    /* Remove from addr hash if not invalid (-1 in hash_next pointer) */
    if (block->hash_next != (Scblock *) -1)
    {
	/* Get hash index of block */
	hash_index = (block->start_addr >> cache->block_id_shift) &
	    cache->hash_index_mask;

	/* Remove from hash linked list */
	if (block->hash_prev == NULL)
	    cache->hash[hash_index] = block->hash_next;
	else
	    block->hash_prev->hash_next = block->hash_next;

	if (block->hash_next != NULL)
	    block->hash_next->hash_prev = block->hash_prev;
    }

    /* Get new hash index */
    hash_index = (new_addr >> cache->block_id_shift) & cache->hash_index_mask;
    
    /* Add to hash linked list */
    if (cache->hash[hash_index] != NULL)
	cache->hash[hash_index]->hash_prev = block;
    block->hash_next = cache->hash[hash_index];
    block->hash_prev = NULL;
    cache->hash[hash_index] = block;

    /* change block addr to new addr */
    block->start_addr = new_addr & cache->start_addr_mask;
}

/*
 * Makes the passed 'block' the most recently used block in
 * the cache line.
 */
void S_cache_make_MRU (cache, block)
Scache *cache;
Scblock *block;
{
    Ststore *tstore;
    unsigned tag_index;

    /* Get tstore for block */
    tag_index = (block->start_addr >> cache->block_id_shift) &
	cache->tag_index_mask;
    tstore = &cache->tag_store[tag_index];

    /* Only move if block is not already MRU */
    if (block->tag_prev != NULL)
    {
	/* Remove from linked list, know block->tag_prev is not NULL */
	block->tag_prev->tag_next = block->tag_next;
	if (block->tag_next == NULL)
	    tstore->tail = block->tag_prev;
	else
	    block->tag_next->tag_prev = block->tag_prev;

	/* Add to front of list, know tblock->tag_prev is not NULL */
	tstore->head->tag_prev = block;
	block->tag_next = tstore->head;
	block->tag_prev = NULL;
	tstore->head = block;
    }
}

/*
 * Makes the passed 'block' the least recently used block in
 * the cache line.
 */
void S_cache_make_LRU (cache, block)
Scache *cache;
Scblock *block;
{
    Ststore *tstore;
    unsigned tag_index;

    /* Get tstore for block */
    tag_index = (block->start_addr >> cache->block_id_shift) &
	cache->tag_index_mask;
    tstore = &cache->tag_store[tag_index];

    /* Only move if block is not already LRU */
    if (block->tag_next != NULL)
    {
	/* Remove from linked list, know block->tag_next is not NULL */
	block->tag_next->tag_prev = block->tag_prev;
	if (block->tag_prev == NULL)
	    tstore->head = block->tag_next;
	else
	    block->tag_prev->tag_next = block->tag_next;

	/* Add to end of list, know tblock->tag_next is not NULL */
	tstore->tail->tag_next = block;
	block->tag_prev = tstore->tail;
	block->tag_next = NULL;
	tstore->tail = block;
    }
}

/*
 * Invalidiates a block (marks block as containing no data) and
 * makes it the LRU block on that cache line.
 * 
 * Invalidated blocks have block->hash_next == (Scblock *) -1;
 */
void S_cache_invalidate (cache, block)
Scache *cache;
Scblock *block;
{
    Ststore *tstore;
    unsigned hash_index, tag_index;

    /* Only invalidate if not already invalidated */
    if (block->hash_next != (Scblock *) -1)
    {
	/* Remove from addr hash */
	hash_index = (block->start_addr >> cache->block_id_shift) &
	    cache->hash_index_mask;
	
	if (block->hash_prev == NULL)
	    cache->hash[hash_index] = block->hash_next;
	else 
	    block->hash_prev->hash_next = block->hash_next;
	if (block->hash_next != NULL)
	    block->hash_next->hash_prev = block->hash_prev;

	/* Set pointers to -1 to flag invalid */
	block->hash_next = (Scblock *) -1;
	block->hash_prev = (Scblock *) -1;

	/* Get tag_store for block */
	tag_index = (block->start_addr >> cache->block_id_shift) &
	    cache->tag_index_mask;
	tstore = &cache->tag_store[tag_index];

	/* Make block LRU, if not already */
	if (block->tag_next != NULL)
	{
	    /* Remove from list, Know block->tag_next != NULL */
	    block->tag_next->tag_prev = block->tag_prev;
	    if (block->tag_prev == NULL)
		tstore->head = block->tag_next;
	    else
		block->tag_prev->tag_next = block->tag_next;

	    /* Add to end of list, know tail pointer != NULL */
	    tstore->tail->tag_next = block;
	    block->tag_prev = tstore->tail;
	    block->tag_next = NULL;
	    tstore->tail = block;
	    
	}

	/* Mark start addr to -1 (this may be a valid address, so don't
	 * use in test for invalid, use hash_next == (Scblock *) -1)
	 */
	block->start_addr = -1;
    }
}
void S_cache_print (out, cache)
FILE *out;
Scache *cache;
{

    Scblock *block;
    int i;

    for (i=0; i <= cache->tag_index_mask; i++)
    {
	fprintf (out, "%2i: ", i);
	for (block = cache->tag_store[i].head; block != NULL; 
	     block = block->tag_next)
	{
	    if (block->hash_next == (Scblock *) -1)
	    {
		fprintf (out, "INVALID  ");
	    }
	    else
	    {
		fprintf (out, "%8X ", block->start_addr);
	    }
	}
	fprintf (out, "\n");
    }
}

void S_cache_contents_print (FILE *out, Scache *cache)
{
    Scblock *block;
    int i;

    for (i=0; i <= cache->tag_index_mask; i++)
    {
	for (block = cache->tag_store[i].head; block != NULL; 
	     block = block->tag_next)
	{
	    if (block->hash_next != (Scblock *) -1)
	    {
		fprintf (out, "%8X\n", block->start_addr);
	    }
	}
    }
}


/*
 * Returns 1 if number is a power of two greater than zero.
 * Returns 0 otherwise.
 */
int S_is_power_of_two (int number)
{
    int shift;

    /* Do not treat 0 or negative numbers as power of two */
    if (number <= 0)
        return (0);

    /* Find rightmost 1 of number (since not zero, must be at least 1)*/
    shift = 0;
    while ((number & (1 << shift)) == 0)
        shift++;

    /* If it is only 1 in number, then number is power of two */
    if (number == (1 << shift))
        return (1);
    else
        return (0);
}

/*
 * Returns the truncated integer log base 2 of a number.
 */
int S_log_base_two (int number)
{
    int shift;

    for (shift = 0; (number & (1 << shift)) == 0; shift ++)
	;

    return (shift);
}

