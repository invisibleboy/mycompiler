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
 *      File:   s_mem.c
 *      Author: John Gyllenhaal
 *      Creation Date:  March 1994
 *      Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and The Board of
 *                         Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include "s_main.h"

/* Local parameters */
int S_page_buffer_size = 2048;	/* Must be power of two */

/* Global variables */
int S_page_buffer_overflowed = 0; /*Set to 1 if page_buffer_size is too small*/

L_Alloc_Pool	*Bus_Stats_pool = NULL;
L_Alloc_Pool	*Mem_Request_pool = NULL;
Bus S_bus;
Memory S_memory;

Bus_Stats *S_create_stats_bus()
{
    Bus_Stats *stats;

    /* Create bus stats structure */
    if (Bus_Stats_pool == NULL)
    {
	Bus_Stats_pool = L_create_alloc_pool ("Bus_Stats",
					      sizeof (Bus_Stats),
					      1);
    }

    stats = (Bus_Stats *) L_alloc (Bus_Stats_pool);

    STATS_ZERO(icache_read_requests);
    STATS_ZERO(icache_read_results);
    STATS_ZERO(dcache_read_requests);
    STATS_ZERO(dcache_read_results);
    STATS_ZERO(dcache_prefetch_requests);
    STATS_ZERO(dcache_prefetch_results);
    STATS_ZERO(dcache_write_requests);
    STATS_ZERO(waiting_for_result);
    STATS_ZERO(dcache_mc_read_requests);
    STATS_ZERO(dcache_mc_read_results);
    STATS_ZERO(dcache_mc_write_requests);

    /* Return the initialized structure */
    return (stats);
}

void S_add_stats_bus (Bus_Stats *dest, 
		      Bus_Stats *src1,
		      Bus_Stats *src2)

{
    STATS_ADD(icache_read_requests);
    STATS_ADD(icache_read_results);
    STATS_ADD(dcache_read_requests);
    STATS_ADD(dcache_read_results);
    STATS_ADD(dcache_prefetch_requests);
    STATS_ADD(dcache_prefetch_results);
    STATS_ADD(dcache_write_requests);
    STATS_ADD(waiting_for_result);
    STATS_ADD(dcache_mc_read_requests);
    STATS_ADD(dcache_mc_read_results);
    STATS_ADD(dcache_mc_write_requests);
}

void S_print_stats_region_bus (FILE *out, Stats *stats,
			       char *rname, Stats *total_stats)
{
    unsigned bus_cycles_free;
    unsigned cycles_simulated;
    Bus_Stats *bstats;
    
    /* Setup Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;

    /* Get the bus stats structure for ease of use */
    bstats = stats->bus;

    /* Get the cycles simulated for ease of use */
    cycles_simulated = stats->region->num_sim_cycles;

    /* 
     * BUS STATS
     */

    /* May get negative bus cycles free if the region generated
     * more bus traffic than the number of cycles simulated
     * in the region.
     */
    bus_cycles_free = cycles_simulated - 
	(bstats->icache_read_requests +
	 bstats->icache_read_results +
	 bstats->dcache_read_requests +
	 bstats->dcache_read_results +
	 bstats->dcache_prefetch_requests +
	 bstats->dcache_prefetch_results +
	 bstats->dcache_write_requests +
	 bstats->dcache_mc_read_requests +
	 bstats->dcache_mc_read_results +
	 bstats->dcache_mc_write_requests +
	 bstats->waiting_for_result);
    

    Pstats ("# BUS:");
    Pstats ("");
    Pstats ("%12u bus cycles simulated.", cycles_simulated);
    Pstats ("%12i bus cycles unused.", bus_cycles_free);
    Pstats ("%12u bus cycles used for icache read requests.",
	     bstats->icache_read_requests);
    Pstats ("%12u bus cycles used for icache read returns.",
	     bstats->icache_read_results);
    Pstats ("%12u bus cycles used for dcache read requests.",
	     bstats->dcache_read_requests);
    Pstats ("%12u bus cycles used for dcache read returns.",
	     bstats->dcache_read_results);
    Pstats ("%12u bus cycles used for dcache prefetch requests.",
	     bstats->dcache_prefetch_requests);
    Pstats ("%12u bus cycles used for dcache prefetch returns.",
	     bstats->dcache_prefetch_results);
    Pstats ("%12u bus cycles used for dcache mem_copy read requests.",
	     bstats->dcache_mc_read_requests);
    Pstats ("%12u bus cycles used for dcache mem_copy read returns.",
	     bstats->dcache_mc_read_results);
    Pstats ("%12u bus cycles spent waiting for read result to return.",
	     bstats->waiting_for_result);
    Pstats ("%12u bus cycles used for dcache write requests.",
	     bstats->dcache_write_requests);
    Pstats ("%12u bus cycles used for dcache mem_copy write requests.",
	     bstats->dcache_mc_write_requests);
    Pstats ("%12.2lf percent bus utilization.",
	     (double) 100.0 * ((double)(cycles_simulated - bus_cycles_free))/
	     ((double) cycles_simulated));
    Pstats ("");
}



void S_read_parm_mem (Parm_Parse_Info *ppi)
{
    L_read_parm_i (ppi, "memory_latency", &S_memory_latency);
    L_read_parm_i (ppi, "memory_page_size", &S_memory_page_size);
    L_read_parm_i (ppi, "page_buffer_size", &S_page_buffer_size);
}

void S_print_configuration_memory (FILE *out)
{
    fprintf (out, "# MEMORY CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12u cycle memory latency.\n", S_memory_latency);
    fprintf (out, "%12u memory page size.\n", S_memory_page_size);
    fprintf (out, "%12u page buffer size (in pages).\n",
	     S_page_buffer_size);
    fprintf (out, "\n");
}

void S_read_parm_bus (Parm_Parse_Info *ppi)
{
    L_read_parm_s (ppi, "bus_model", &S_bus_model_name);
    L_read_parm_b (ppi, "debug_bus", &S_debug_bus); 
    L_read_parm_i (ppi, "bus_bandwidth", &S_bus_bandwidth);
    L_read_parm_b (ppi, "streaming_support", &S_streaming_support);

}
void S_print_configuration_bus (FILE *out)
{
    fprintf (out, "# BUS CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s transaction bus model.\n", S_bus_model_name);
    fprintf (out, "%12u bytes/cycle bus bandwidth.\n", S_bus_bandwidth);
    if (S_secondary_cache ? S_L2_streaming_support : S_streaming_support)
	fprintf (out, "%12s ", "Yes");
    else
	fprintf (out, "%12s ", "No");
    fprintf (out, 
	     "streaming support (forwarding miss data as it comes in).\n");
    fprintf (out, "\n");

}

void S_init_bus()
{
    /* Set bus model */
    if (L_pmatch (S_bus_model_name, "single"))
    {
	/* Reassign name to make output look nice */
	S_bus_model_name = "Single";
	S_bus_model = BUS_MODEL_SINGLE;
    }
    else if (L_pmatch (S_bus_model_name, "split"))
    {
	/* Reassign name to make output look nice */
	S_bus_model_name = "Split";
	S_bus_model = BUS_MODEL_SPLIT;
    }
    else 
	S_punt ("S_init_bus: Undefined bus model '%s'.", S_bus_model_name);

    /* S_bus_bandwidth (in bytes) must be >= 1 */
    if (S_bus_bandwidth < 1)
	S_punt ("S_init_bus: bus_bandwidth (%i) must be >= 1.", 
		S_bus_bandwidth);

    /* S_bus_bandwidth must also be a power of 2 */
    if (!S_is_power_of_two (S_bus_bandwidth))
	S_punt ("S_init_bus: bas_bandwidth (%i) must be power of 2.",
		S_bus_bandwidth);
    

    /* Set size_offset and size_shift for calculating length of transaction*/
    S_bus.size_offset = S_bus_bandwidth - 1;
    S_bus.size_shift = S_log_base_two (S_bus_bandwidth);

    /* 
     * Initialize bus settings.  This is the only initialization the
     * bus will get.
     */
    S_bus.avail = 1;
    S_bus.mem_only = 0;
    S_bus.type = 0;
    S_bus.src = NO_ONE;
    S_bus.dest = NO_ONE;
    S_bus.addr = 0;
    S_bus.size = 0;
    S_bus.length = 0;
    S_bus.segment = 0;
    S_bus.stats = NULL;
}

void S_sim_bus ()
{
    Bus_Stats *bstats;


    /* Only need to simulation if bus is currently being used or if
     * use ended last cycle (if already available, do nothing).
     */
    if (!S_bus.avail && !S_bus.mem_only)
    {
	/* Use the current transactions stats structure or the last
	 * transactions stats.  Works as long as don't count unused
	 * cycles.  Also, can cause a region to take more bus cycles
	 * than the region has.  This means that region of code is
	 * really stressing the bus.
	 */
	bstats = S_bus.stats->bus;

	/* Update stats */
	switch (S_bus.type)
	{
	  case READ_REQUEST:
	    if (S_bus.src != ICACHE)
	    {
		bstats->dcache_read_requests++;
	    }
	    else
	    {
		bstats->icache_read_requests++;
	    }
	    break;

	  case READ_RESULT:
	    if (S_bus.dest != ICACHE)
	    {
		bstats->dcache_read_results++;
	    }
	    else
	    {
		bstats->icache_read_results++;
	    }
	    break;

	  case WRITE_REQUEST:
	    bstats->dcache_write_requests++;
	    break;

	  case PREFETCH_REQUEST:
	    bstats->dcache_prefetch_requests++;
	    break;

	  case PREFETCH_RESULT:
	    bstats->dcache_prefetch_results++;
	    break;

	  case MC_READ_REQUEST:
	    bstats->dcache_mc_read_requests++;
	    break;

	  case MC_READ_RESULT:
	    bstats->dcache_mc_read_results++;
	    break;

	  case MC_WRITE_REQUEST:
	    bstats->dcache_mc_write_requests++;
	    break;

	  default:
	    S_punt ("S_sim_bus: Unknown bus type %i.\n", S_bus.type);
	}

    	/* Have we reached the end of the transaction? */
	if (S_bus.segment == S_bus.length)
	{
	    /* At the end of the transaction */
	    /* If in single-transaction mode and just did a read request,
	     * then it is only available to memory.
	     */
	    if (((S_bus.type == READ_REQUEST) || 
		 (S_bus.type == PREFETCH_REQUEST)) && 
		(S_bus_model == BUS_MODEL_SINGLE))
	    {
		S_bus.mem_only = 1;
	    }
	    /* Otherwise, bus available to everyone */
	    else
	    {
		S_bus.avail = 1;
	    }
	    /* Zero src, dest for ease of programming */
	    S_bus.src = NO_ONE;
	    S_bus.dest = NO_ONE;
	}

	/* Still doing transaction, increment segment transfering */
	else
	{
	    S_bus.segment++;
	}

    }
    else if (S_bus.mem_only)
    {
	S_bus.stats->bus->waiting_for_result++;
    }
}

static void S_print_component_name(FILE *out, int comp_id)
{
    switch (comp_id)
    {
      case MEMORY:
	fprintf (out, "memory");
	break;

      case DCACHE:
	fprintf (out, "dcache");
	break;

      case L2:
	fprintf (out, "L2 cache");
	break;
	
      case ICACHE:
	fprintf (out, "icache");
	break;

      default:
	fprintf (out, "unknown type %i", comp_id);
    }
}

/* Print out the bus activity for this cycle */
void S_print_bus_state (FILE *out)
{
    fprintf (out, "Bus cycle %4i: ", S_sim_cycle);
    
    if (S_bus.avail)
	fprintf (out, "Idle\n");
    else if (S_bus.mem_only)
	fprintf (out, "Waiting for memory result\n");
    else
    {
	switch (S_bus.type)
	{
	  case READ_REQUEST:
	    fprintf (out, "Read request from ");
	    S_print_component_name (out, S_bus.src);
	    fprintf (out, "   ");
	    break;

	  case PREFETCH_REQUEST:
	    fprintf (out, "Prefetch request from ");
	    S_print_component_name (out, S_bus.src);
	    break;

	  case MC_READ_REQUEST:
	    fprintf (out, "MC read request from ");
	    S_print_component_name (out, S_bus.src);
	    fprintf (out, "");
	    break;


	  case READ_RESULT:
	    fprintf (out, "Memory block to ");
	    S_print_component_name (out, S_bus.dest);
	    fprintf (out, "     ");
	    break;

	  case PREFETCH_RESULT:
	    fprintf (out, "Prefetched block to ");
	    S_print_component_name (out, S_bus.dest);
	    fprintf (out, "  ");
	    break;

	  case MC_READ_RESULT:
	    fprintf (out, "MC memory value to ");
	    S_print_component_name (out, S_bus.dest);
	    fprintf (out, "  ");
	    break;
	    
	  case WRITE_REQUEST:
	    fprintf (out, "Write from ");
	    S_print_component_name (out, S_bus.src);
	    fprintf (out, "          ");
	    break;

	  case MC_WRITE_REQUEST:
	    fprintf (out, "MC write from ");
	    S_print_component_name (out, S_bus.src);
	    fprintf (out, "       ");
	    break;
	   
	  default:
	    fprintf (out, "Unknown (%i) from ", S_bus.type);	
	    S_print_component_name (out, S_bus.src);
	    fprintf (out, " to ");
	    S_print_component_name (out, S_bus.dest);
	}
	fprintf (out, " %3i bytes  cycle %i of %i  addr %08x\n", S_bus.size,
		 S_bus.segment, S_bus.length, S_bus.addr);
	
    }
}

void S_sim_bus_transaction (int type, int src, int dest, int addr, 
		int size, Stats *stats)
{
    /* Sanity check, the bus better not be in use */
    if (!S_bus.avail && !(S_bus.mem_only && (src == MEMORY)))
	S_punt ("S_sim_bus_transaction: '%i' accessing bus when in use.",
		src);

    /* Mark bus no longer available */
    S_bus.avail = 0;
    S_bus.mem_only = 0;

    /* Save transaction parameters */
    S_bus.type = type;
    S_bus.src = src;
    S_bus.dest = dest;
    S_bus.addr = addr;
    S_bus.size = size;
    S_bus.stats = stats;

    /* Calculate transaction length */
    switch (type)
    {
      case READ_REQUEST:
      case PREFETCH_REQUEST:
      case MC_READ_REQUEST:
	S_bus.length = 1;
	break;
	
      case READ_RESULT:
      case PREFETCH_RESULT:
      case MC_READ_RESULT:
	S_bus.length = (size + S_bus.size_offset) >> S_bus.size_shift;
	break;
	
      case WRITE_REQUEST:
      case MC_WRITE_REQUEST:
	S_bus.length = (size + S_bus.size_offset) >> S_bus.size_shift;
	break;

      default:
	S_punt ("S_sim_bus_transaction: unknown type '%i'.", type);
    }


    /* Initialize segment to be first segment of transaction */
    S_bus.segment = 1;
}

void S_init_memory()
{
    S_memory.pending_head = NULL;
    S_memory.pending_tail = NULL;
}

void S_sim_memory_first_half_cycle ()
{
    Mem_Request *request;
    int type = 0;

    /* Is the bus available to memory? */
    if (S_bus.avail || S_bus.mem_only)
    {
	/* Yes, If there a read_result ready to go out, then
	 * send it back, otherwise do nothing.
	 */
	request = S_memory.pending_head;
	
	if ((request != NULL) &&
	    (request->complete_time <= S_sim_cycle))
	{
	    /* Get type of transaction */
	    switch (request->type)
	    {
	      case  READ_REQUEST:
		type = READ_RESULT;
		break;

	      case PREFETCH_REQUEST:
		type = PREFETCH_RESULT;
		break;

	      case MC_READ_REQUEST:
		type = MC_READ_RESULT;
		break;

	      default:
		S_punt ("S_sim_memory_first_half_cycle: unknown type %i",
			request->type);
	    }

	    /* Result ready to go back and bus available, send it */
	    S_sim_bus_transaction (type, MEMORY, request->src,
				   request->addr, request->size,
				   request->stats);

	    /* Request completed, remove from head of queue */
	    S_memory.pending_head = request->next;
	    if (S_memory.pending_head == NULL)
		S_memory.pending_tail = NULL;

	    /* Free request node */
	    L_free (Mem_Request_pool, request);
	}
    }
}

void S_sim_memory_second_half_cycle ()
{
    Mem_Request *request;

    /*
     * Is another request coming in and are we at the
     * last cycle of the request?
     */
    if ((S_bus.dest == MEMORY) && (S_bus.segment == S_bus.length))
    {
	/* Process according to request type */
	switch (S_bus.type)
	{
	    /* 
	     * Hold request for S_memory_latency cycles then
	     * start arbitrating for the bus to return result.
	     */
	  case READ_REQUEST:
	  case PREFETCH_REQUEST:
	  case MC_READ_REQUEST:

	    /* Allocate request structure */
	    if (Mem_Request_pool == NULL)
	    {
		Mem_Request_pool = L_create_alloc_pool ("Mem_Request",
							sizeof (Mem_Request),
							1);
	    }
	    request = (Mem_Request *) L_alloc (Mem_Request_pool);

	    /* Fill in request info */
	    request->type = S_bus.type;
	    request->src = S_bus.src;
	    request->addr = S_bus.addr;
	    request->size = S_bus.size;
	    request->stats = S_bus.stats;

	    /* Set completion type based on memory latency */
	    request->complete_time = S_sim_cycle + S_memory_latency;

	    /* Put at end of pending queue (assume full pipelining
	     * and unlimited buffer space)
	     */
	    if (S_memory.pending_tail == NULL)
		S_memory.pending_head = request;
	    else
		S_memory.pending_tail->next = request;
	    request->next = NULL;
	    S_memory.pending_tail = request;
	    break;

	    /*
	     * Do nothing for write requests.
	     */
	  case WRITE_REQUEST:
	  case MC_WRITE_REQUEST:
	    break;

	  default:
	    S_punt ("S_sim_memory: unknown request type '%i'.",
		    S_bus.type);
	}
    }
}

/* 
 * Buffer page support is the OS adding "buffer pages" to a processes 
 * page table whenever a speculative load has a segmentation fault.  This
 * prevents later speculative loads to that page from excepting.  Typically
 * this page is read-only, and filled with zeros.
 */
void S_init_buffer_page_support ()
{
    /* Make sure memory page size is power of two */
    if (!S_is_power_of_two (S_memory_page_size))
	S_punt ("Error: memory_page_size (%i) must be power of two.",
		S_memory_page_size);

    /* Make sure page buffer size is a power of two */
    if (!S_is_power_of_two (S_page_buffer_size))
	S_punt ("Error: page_buffer_size (%i) must be power of two.",
		S_page_buffer_size);


    /* Create a large, fully-assoc cache of buffer pages.
     * No block data is necessary.  Each block is made the size of a page.
     * 
     * 1024 entries should take up around 16-32k of memory, but it
     * will be very fast.  May come back and do dynamically sized
     * version later but I doubt that more than 10 entries will be
     * used.
     */
    S_buffer_page_cache = S_create_cache(S_page_buffer_size*S_memory_page_size,
					 S_memory_page_size, 0, NULL);
}

/*
 * OS buffer page support.  Creates buffer pages for non-trapping loads,
 * so that only the first load to a invalid page will cause an trap
 * that has to be handled by the OS.
 */
void S_sim_buffer_page_support (Sint *sint)
{
    Region_Stats *rstats;
    Scblock 	*page;
    int addr, opflags;

    rstats = sint->stats->region;

    /* Sanity checks */
    if (!(sint->flags & MASKED_SEG_FAULT))
	S_punt ("S_sim_buffer_page_support: masked seg fault expected.");

    /* For now, assume that instructions that bus error do not add
     * buffer pages. 
     */
    if (sint->flags & MASKED_BUS_ERROR)
	S_punt ("S_sim_buffer_page_support: masked bus error not expected.");

    /* Get address and op_flags for ease of use */
    addr = sint->trace.mem_addr;
    opflags = sint->oper->flags;

    /* Is page in buffer page cache */
    page = S_cache_find_addr (S_buffer_page_cache, addr);

    /* If Miss, add page to cache */
    if (page == NULL)
    {
	if (opflags & PROMOTED)
	{
	    rstats->promoted_added_buffer_pages ++;
	}
	else if (opflags & SPECULATIVE)
	{
	    rstats->speculative_added_buffer_pages ++;
	}
	else
	{
	    rstats->unmarked_added_buffer_pages ++;
	}

	/* Add to buffer page cache.
	 * better be invalid or we have
	 * run out of pages (unlikely, but could happen).
	 */
	page = S_cache_find_LRU (S_buffer_page_cache, addr);
	if (page->hash_next != (Scblock*) -1)
	{
	    /* Print warning if have not already warned */
	    if (!S_page_buffer_overflowed)
	    {
	        fprintf (stderr, 
			 "Warning: page buffer (%i pages) overflowed (invalidating stats...)\n",
			 S_page_buffer_size);
		S_page_buffer_overflowed = 1;
	    }
	}
	
	S_cache_change_addr (S_buffer_page_cache, page, addr);
	S_cache_make_MRU (S_buffer_page_cache, page);
    }
}




