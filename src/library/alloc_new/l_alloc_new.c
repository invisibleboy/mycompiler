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
/*===========================================================================
 *
 *      File :          l_alloc_new.c
 *      Description :   New and improved data structure allocation/mgmt
 *      Creation Date : May 1993
 *      Authors :       John C. Gyllenhaal and Wen-mei Hwu
 *      Contributors:   Roger A. Bringmann and Scott Mahlke
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include "l_alloc_new.h"
#include <library/i_list.h>

/* 
 * Set this to 1 before creating an alloc pool to bypass alloc routines
 * so that malloc and free are called each time L_alloc and L_free are called
 * with that alloc pool. 
 * This allows efficient use of the debug malloc routines.
 */

int bypass_alloc_routines = 0;

static L_Alloc_Pool *active_alloc_pools = NULL;

L_Alloc_Pool *
L_create_alloc_pool (char *name, int size, int num_in_block)
{
    L_Alloc_Pool *pool;
    int pool_size, name_size;

    /* Make sure valid values are passed */
    if (name == NULL)
    {
        fprintf (stderr, "L_create_alloc_pool: name NULL\n");
        exit (1);
    }

    /* Force size to be at least big enough to hold header,
     * (so can make linked list of free elements, even
     * if element size is < 4)
     */
    if (size < (int) sizeof (L_Alloc_Pool_Header))
        size = (int) sizeof (L_Alloc_Pool_Header);

    /*
     * Force size to be a multiple of 4.
     * Otherwise get alignment problems with the L_Alloc_Pool_Header.
     * We may need to force to multiples of 8 (if double alignment 
     * becomes a problem), but I think sizeof() forces the size 
     * to be a mutliple of 8 if any doubles are in the structure. -JCG
     */
    size = (((size + 3) >> 2) << 2);

    /* Must allocate at least one elment in the block */
    if (num_in_block < 1)
    {
        fprintf (stderr, "L_create_alloc_pool: num_in_block (%i) < 1\n",
                 num_in_block);
        exit (1);
    }

    /* Get size to allocate */
    pool_size = sizeof (L_Alloc_Pool);
    name_size = strlen (name) + 1;
    
    /* Allocate pool and buffer space for name */
    if (((pool = (L_Alloc_Pool *) malloc (pool_size)) == NULL) ||
        ((pool->name = (char *) malloc (name_size)) == NULL))
    {
        fprintf (stderr, "L_create_alloc_pool: Out of memory\n");
        exit (1);
    }

    /* Initialize structure for nothing allocated */
    strcpy (pool->name, name);
    pool->element_size = size;
    pool->num_in_block = num_in_block;
    /*
     * Get the size of block to allocate, include space for
     * block header (to keep track of blocks allocated.
     *
     * Note:  We must double align the header and the input size to
     * prevent access violations for doubles.
     */
    pool->block_size = (size * num_in_block) + 
       (((sizeof (L_Alloc_Pool_Header)+7)>>3)<<3);

    pool->head = NULL;
    pool->allocated = 0;
    pool->free = 0;
    pool->blocks_allocated = 0;
    pool->block_list = NULL;

    /* Read flag about whether the routines should be bypassed and
     * malloc and free used every time.  See message at top of file.
     */
    pool->bypass_routines = bypass_alloc_routines;

    pool->next = active_alloc_pools;
    active_alloc_pools = pool;

    /* Return the pool */
    return (pool);
}

/*
 * Frees all the memory allocated for a memory pool.
 * All the individual elements in the pool must be
 * free or this command will print warning messages
 * and not free the memory.
 */
void 
L_free_alloc_pool (L_Alloc_Pool *pool)
{
    L_Alloc_Pool_Header *block, *next_block;

    /* Make sure NULL pointer not passed */
    if (pool == NULL)
    {
        fprintf (stderr, "L_free_alloc_pool: pool is NULL\n");
        exit (1);
    }

    /* Cannot free pool if elements are still in use */
    if (pool->allocated != pool->free)
    {
        fprintf (stderr,
              "Warning: Cannot free pool '%s'.  %d elements still in use!\n",
                 pool->name, (pool->allocated - pool->free));
        return;
    }
    
    /* Free each block allocated for the pool */
    for (block = pool->block_list; block != NULL; block = next_block)
    {
        /* Get next block before we free block */
        next_block = block->next;
        
        /* Free the block */
        free (block);
    }

    /* Free the alloc pool name */
    free (pool->name);

    {
      L_Alloc_Pool *ptr;

      ptr = active_alloc_pools;

      if (pool == ptr)
	{
	  active_alloc_pools = pool->next;
	}
      else if (ptr)
	{
	  do
	    {
	      if (ptr->next == pool)
		{
		  ptr->next = pool->next;
		  break;
		}
	    }
	  while ((ptr = ptr->next));
	}
    }

    /* Free the pool itself */
    free (pool);
}


/* Allocates one element from the pool specified */
void *
L_alloc(L_Alloc_Pool *pool)
{
    int  i;
    L_Alloc_Pool_Header *header, *block_header;
    char *block;

    /* Make sure NULL pointer is not passed */
    if (pool == NULL)
    {
        fprintf (stderr, "L_alloc: NULL pool pointer\n");
        exit (1);
    }

    /*
     * If routine should be bypassed, call malloc directly.
     * allows debug malloc library to be used effectively
     */
    if (pool->bypass_routines)
    {
        /* Update alloc stats */
        pool->allocated++;

        return ((void *)malloc (pool->element_size));
    }

    /* If there are no more free elements, allocate a block of them */
    if (pool->head == NULL)
    {
        /* Allocate the block of memory */
        if ((block = (char *) malloc (pool->block_size)) == NULL)
        {
            fprintf (stderr,
                     "L_alloc (%s): Out of memory (request size %i)\n",
                     pool->name,  pool->block_size);
            exit(1);
        }

        /* Get the head of block for block list */
        block_header = (L_Alloc_Pool_Header *) block;
        block += (((sizeof (L_Alloc_Pool_Header)+7)>>3)<<3);

        /* Add to head of block list */
        block_header->next = pool->block_list;
        pool->block_list = block_header;

        /* Break rest of block into pieces and put into free list */
        for (i=0; i < pool->num_in_block; i++)
        {
            /* typecast current element so can put into free list */
            header = (L_Alloc_Pool_Header *) block;
            header->next = pool->head;
            pool->head = header;

            /* Goto next element */
            block += pool->element_size;
        }

        /* Update stats */
        pool->allocated += pool->num_in_block;
        pool->free += pool->num_in_block;
        pool->blocks_allocated += 1;
    }

    /* Get element from head of list */
    header = pool->head;
    pool->head = header->next;

    /* Update stats */
    pool->free--;

    /* Return element */
    return ((void *) header);
}

/* Puts the element pointed to in ptr into the pool specified */
void 
L_free (L_Alloc_Pool *pool, void *ptr)
{
    L_Alloc_Pool_Header *header;

    /* Make sure don't try to free NULL pointer */
    if (ptr == NULL)
    {
        fprintf (stderr, "L_free (%s): NULL pointer passed\n", pool->name);
        exit(-1);
    }

    /*
     * If routine should be bypassed, call free directly.
     * Allows debug malloc library to be used effectively.
     */
    if (pool->bypass_routines)
    {
        /* Update free stats */
        pool->free++;
        free (ptr);
        return;
    }

    /* Typecast so can put info free list */
    header = (L_Alloc_Pool_Header *)ptr;

    /* Put into free list */
    header->next = pool->head;
    pool->head = header;

    /* Update stats */
    pool->free++;

    /* If we have freed more than we have allocated, something is wrong */
    if (pool->free > pool->allocated)
    {
        fprintf (stderr, "L_free (%s): More freed than allocated\n",
                 pool->name);
        exit (1);
    }
}

/*
 * Prints how many elements were allocated, how many blocks of memory
 * were allocated and how many free elements in the list there are.
 */
long
L_print_alloc_info(FILE *F, L_Alloc_Pool *pool,  int verbose)
{
  long size;

  size = pool->blocks_allocated * pool->block_size;

  if ((verbose) ||
      (pool->allocated != pool->free))
    {
      if (pool->bypass_routines)
        {
	  fprintf(F, "    %16s (BYPASSED): allocated %5d  free %5d  "
		  "blocks %5d  size %5ldk\n",
		  pool->name, pool->allocated, pool->free,
		  pool->blocks_allocated, size / 1024);

        }
      else
        {
	  fprintf(F, "    %-16s: allocated %5d  free %5d  "
		  "blocks %5d  size %5ldk\n",
		  pool->name, pool->allocated, pool->free,
		  pool->blocks_allocated, size / 1024);
        }
    }
  return size;
}

void
L_summarize_allocation_info (FILE *fp)
{
  L_Alloc_Pool *pool;
  long size = 0;
  if (!fp)
    fp = stderr;

  pool = active_alloc_pools;
  while (pool)
    {
      size += L_print_alloc_info (fp, pool, 1);
      pool = pool->next;
    }

  fprintf (fp, "Total allocated in pools: %8ldk\n", size / 1024);
  return;
}


