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
 *      File:   s_icache.c
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

#undef DEBUG_ICACHE

/* simulate a bus transaction, calling the appropriate bus routine depending
 * on whether we have an L2 cache or not.
 */
#define BUS_TRANSACTION(type,source,dest,addr,size,stats) \
        if (S_secondary_cache) \
            S_sim_L2_bus_transaction(type,source,dest,addr,size,0,stats); \
        else \
            S_sim_bus_transaction(type,source,dest,addr,size,stats)

/* Depending on whether we have a secondary cache, access the L2_Bus or Bus struct */
#define BUS(field) \
        (S_secondary_cache ? S_L2_bus.field : S_bus.field)

int memory_latency;
int bus_bandwidth;
int S_next_level;

void S_read_parm_icache (Parm_Parse_Info *ppi)
{
    L_read_parm_s (ppi, "icache_model", &S_icache_model_name);
    L_read_parm_i (ppi, "icache_size", &S_icache_size);
    L_read_parm_i (ppi, "icache_block_size", &S_icache_block_size);
    L_read_parm_i (ppi, "icache_assoc", &S_icache_assoc);
}

void S_print_configuration_icache (FILE *out)
{
    fprintf (out, "# ICACHE CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s icache model.\n", S_icache_model_name);
    if (S_icache_model != ICACHE_MODEL_PERFECT)
    {
	fprintf (out, "%12u icache size.\n", S_icache_size);
	fprintf (out, "%12u icache block size.\n", S_icache_block_size);
	fprintf (out, "%12u icache assoc.\n", S_icache_assoc);
	fprintf (out, "%12u icache miss latency (calculated).\n",
		 S_icache_miss_latency);
    }
    fprintf (out, "\n");
}

/*
 * Creates the icache data structure and returns it.
 */
Icache *S_create_icache (pnode)
Pnode *pnode;
{
    Icache *icache;
    int transfer_time;
    char name_buf[100];

    icache = (Icache *) L_alloc (Icache_pool);
    
    /* Point to processsing node icache is in */
    icache->pnode = pnode;

    /* Write Icache name */
    sprintf (name_buf, "Icache %i", pnode->id);
    icache->name = strdup(name_buf);

    /* Initialize icache model */
    if (L_pmatch (S_icache_model_name, "perfect"))
    {
	/* Reassign name to beautify printout :) */
	S_icache_model_name = "Perfect";
	S_icache_model = ICACHE_MODEL_PERFECT;

    } 
    else if (L_pmatch (S_icache_model_name, "standard"))
    {
	/* Reassign name to beautify printout :) */
	S_icache_model_name = "Standard";
	S_icache_model = ICACHE_MODEL_STANDARD;
	
    }
    else if (L_pmatch (S_icache_model_name, "split-block"))
    {
	/* Reassign name to beautify printout :) */
	S_icache_model_name = "Split-block";
	S_icache_model = ICACHE_MODEL_SPLIT_BLOCK;
    }
    else
	S_punt ("Unknown icache model '%s'.\n", S_icache_model_name);


    /* Make sure S_icache_block_size is at least one word */
    if (S_icache_block_size < 4)
	S_punt ("S_create_icache: icache_block_size (%i) must be >= 4.\n",
		S_icache_block_size);

    /* Don't create cache for perfect model */
    if (S_icache_model != ICACHE_MODEL_PERFECT)
    {
	/* Create cache, no data needed */
	icache->cache = S_create_cache (S_icache_size, S_icache_block_size, 
					S_icache_assoc, NULL);
    }

    /* Icache Initially not busy */
    icache->sent_request = 0;
    icache->request_addr = 0;
    
    /* If we have a secondary cache we must use L2 bus rather than
     * main memory bus. Set up pointers to correct bus parameters.
     */
    if (S_secondary_cache)
    {
        memory_latency = S_scache_latency;
        bus_bandwidth = S_L2_bus_bandwidth;
        S_next_level = L2;
    }
    else
    {
        memory_latency = S_memory_latency;
        bus_bandwidth = S_bus_bandwidth;
        S_next_level = MEMORY;
    }

    /* Make sure bus_bandwidth is power of two */
    if (!S_is_power_of_two (bus_bandwidth))
	S_punt ("bus_bandwidth (%i) must be power of two.", 
		bus_bandwidth);

    /* Calculate latencies for dcache */
    transfer_time = (S_icache_block_size/bus_bandwidth);
    S_icache_miss_latency = 1 + memory_latency + (transfer_time - 1);

    /* 
     * Initialize communication lines between simulations
     * on processing node.
     */
    pnode->icache_busy = 0;
    pnode->icache_addr_requested = 0;
    pnode->icache_bytes_returned = 0;
    
    return (icache);
}

void S_sim_icache_first_half_cycle (Pnode *pnode)
{
    Scblock *block, *next_block;
    Icache *icache;
    unsigned int request_addr;
    int useful_bytes;
    Icache_Stats	*istats;

    /* Get icache for ease of use */
    icache = pnode->icache;

    /* Get stats structure for ease of use */
    istats = pnode->stats->icache;

    /* Simulate perfect icache */
    if (S_icache_model == ICACHE_MODEL_PERFECT)
    {
	/* Do stuff only if request */
	if (pnode->icache_addr_requested != 0)
	{
	    /* Update stats */
	    istats->hits++;

	    /* Return block of data */
	    pnode->icache_bytes_returned = S_icache_block_size;
	}
	else
	{
	    /* Return nothing */
	    pnode->icache_bytes_returned = 0;
	}
	return;
    }

    /* If Processing miss */
    if (pnode->icache_busy)
    {
	/* Do we need to send out a request and the bus is available? */
	if ((!icache->sent_request) && BUS(avail))
	{
	    /* Request block of data */
	    BUS_TRANSACTION (READ_REQUEST, ICACHE, S_next_level,
				   icache->request_addr & 
				   icache->cache->start_addr_mask,
				   S_icache_block_size,
				   S_pnode->stats);

	    /* Mark that we have sent request */
	    icache->sent_request = 1;
	}
	/* Otherwise, wait for data to come back (in second half cycle) */
    }

    /* Otherwise handle a new request if there is one */
    else if (pnode->icache_addr_requested != 0)
    {
	
	/* Get the address requested for ease of use */
	request_addr = pnode->icache_addr_requested;

	/* See if block in cache */
	block = S_cache_find_addr (icache->cache, request_addr);

	/* If miss, put cache into busy state start miss handling */
	if (block == NULL)
	{
	    /* Update icache miss stats */
	    istats->misses++;
	    

	    /* set up miss handling mode */
	    pnode->icache_busy = 1;
	    icache->sent_request = 0;
	    icache->request_addr = request_addr;
	    
	    /* Mark that nothing is returned to processor */
	    pnode->icache_addr_requested = 0;
	    pnode->icache_bytes_returned = 0;
	    
#ifdef DEBUG_ICACHE
	    fprintf (debug_out, 
		     "Icache cycle %3i: Miss (start addr %x, request %x)\n", 
		     S_sim_cycle,
		     request_addr & icache->cache->start_addr_mask,
		     request_addr);
#endif
	}

	/* Otherwise process cache hit */
	else
	{
	    /* update icache hit stats */
	    istats->hits++;

	    /* Make block most recently used in cache line */
	    S_cache_make_MRU (icache->cache, block);
	 

#ifdef DEBUG_ICACHE
	    fprintf (debug_out,
		     "Icache cycle %3i: Hit (start addr %x, request %x)\n",
		     S_sim_cycle, 
		     block->start_addr, request_addr);
#endif

	    /* See how much of block can be used (to end of block) */
	    useful_bytes = block->start_addr + S_icache_block_size - 
		request_addr;

	    /* If got less than half a block an have a split block cache
	     * then get half of next block if it is in the cache.
	     */
	    if ((useful_bytes <= (S_icache_block_size >> 1)) &&
		(S_icache_model == ICACHE_MODEL_SPLIT_BLOCK))
	    {
		/* See if next block in cache */
		next_block = S_cache_find_addr (icache->cache,
						request_addr +
						S_icache_block_size);

		/*
		 * If in cache, get another half block of useful bytes
		 */
		if (next_block != NULL)
		{
		    /*
		     * Update the next_block's MRU status.
		     * This may not be realistic.
		     */
		    S_cache_make_MRU (icache->cache, next_block);
		    
		    /* Add another half block to the number of useful bytes */
		    useful_bytes += (S_icache_block_size >> 1); 
		    
		    /* Updates sim stats */
		    istats->split_blocks++;
		    
#ifdef DEBUG_ICACHE
		    fprintf (debug_out,
			     "Icache cycle %i: Split block hit to block %x.\n",
			     S_sim_cycle, 
			     next_block->start_addr);
#endif
		}
	    }
	
	    /* Return the number of bytes returned to the processor */
	    pnode->icache_bytes_returned = useful_bytes;
	    
#ifdef DEBUG_ICACHE
	    fprintf (debug_out, 
		     "Icache cycle %i: Returning %i instructions\n",
		     S_sim_cycle, pnode->icache_bytes_returned);
#endif
	}
    }
}

void S_sim_icache_second_half_cycle (Pnode *pnode)
{
    Icache	*icache;
    Scblock	*block;
    Icache_Stats *istats;

    /* Do nothing in the second half cycle for perfect icache */
    if (S_icache_model == ICACHE_MODEL_PERFECT)
	return;

    /* Check bus only if waiting for block to return */
    if (!pnode->icache_busy)
	return;

    /* Get icache stats structure for ease of use */
    istats = pnode->stats->icache;

    /* Complete miss when last segment of block comes back from memory */
    if ((BUS(dest) == ICACHE) && (BUS(segment) == BUS(length)))
    {
	/* Get icache for ease of use */
	icache = pnode->icache;

	/* Sanity check, make sure proper address returned */
	if ((icache->request_addr & icache->cache->start_addr_mask) !=
	    BUS(addr))
	{
	    S_punt ("Incorrect block %x returned to icache (expecting %x).",
		    BUS(addr),
		    (icache->request_addr & icache->cache->start_addr_mask));
	}
	
	/* Add block to cache */
	block = S_cache_find_LRU (icache->cache, icache->request_addr);
	
	/* Update stats if kicking out valid block */
	if (block->hash_next != (Scblock *) -1)
	{
	    istats->blocks_kicked_out++;
	}
	
	/* Change to new address */
	S_cache_change_addr (icache->cache, block, icache->request_addr);
	
	/* Make block most recently used in cache line */
	S_cache_make_MRU (icache->cache, block);
	
	/* Return request with last of block data */
	pnode->icache_addr_requested = icache->request_addr;

	/* Get number bytes to return to processor */
	pnode->icache_bytes_returned = BUS(addr) + S_icache_block_size - 
	    icache->request_addr;
	
	/* Mark icache as not busy */
	pnode->icache_busy = 0;
#ifdef DEBUG_ICACHE
	    fprintf (debug_out, 
		     "Icache cycle %i: Returning %i bytes\n",
		     S_sim_cycle, pnode->icache_bytes_returned);
#endif
    }
}



