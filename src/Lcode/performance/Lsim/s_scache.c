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
 *      File:   s_scache.c
 *      Author: Teresa Johnson
 *      Creation Date:  1996
 *      Copyright (c) 1996 Teresa Johnson, Wen-mei Hwu and The Board of
 *                         Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1996 Teresa Johnson, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include "s_main.h"

#define PLAYDOH_TCHS_ALL 0x000000F0

L_Alloc_Pool    *L2_pool = NULL;
L_Alloc_Pool	*L2_Bus_Stats_pool = NULL;
L_Alloc_Pool	*L2_Stats_pool = NULL;
L_Alloc_Pool	*L2_Request_pool = NULL;
L2_Bus S_L2_bus;

int S_scache_read_block_latency;
int S_scache_write_block_latency;
int S_scache_write_thru_latency;
int S_scache_streaming_benefit;
int S_scache_higher_store_priority = 0;
int S_scache_block_until_store_completed = 0;

Scache *L2_vcache;
int S_L2_victim_cache = 0;
int S_L2_vcache_size = 64*512;
int S_L2_vcache_block_size = 64;

int S_current_L2_dest;
int S_current_L2_size;

int S_C3_addr_mask;
int S_C3_req_size;

L2_Bus_Stats *S_create_stats_L2_bus()
{
    L2_Bus_Stats *stats;

    /* Create bus stats structure */
    if (L2_Bus_Stats_pool == NULL)
    {
	L2_Bus_Stats_pool = L_create_alloc_pool ("L2_Bus_Stats",
					      sizeof (L2_Bus_Stats),
					      1);
    }

    stats = (L2_Bus_Stats *) L_alloc (L2_Bus_Stats_pool);

    STATS_ZERO(icache_read_requests);
    STATS_ZERO(icache_read_results);
    STATS_ZERO(dcache_read_requests);
    STATS_ZERO(dcache_read_results);
    STATS_ZERO(dcache_prefetch_requests);
    STATS_ZERO(dcache_prefetch_results);
    STATS_ZERO(dcache_write_requests);
    STATS_ZERO(waiting_for_result);

    /* Return the initialized structure */
    return (stats);
}

L2_Stats *S_create_stats_L2()
{
    L2_Stats *stats;
    int i;

    /* Create L2 stats structure */
    if (L2_Stats_pool == NULL)
    {
        L2_Stats_pool = L_create_alloc_pool ("L2_Stats",
                                                 sizeof (L2_Stats),
                                                 1);
    }

    stats = (L2_Stats *) L_alloc (L2_Stats_pool);

    for (i=0; i <= MAX_OPC; i++)
    {
	STATS_ZERO(conflict_misses[i]);
	STATS_ZERO(anticonflict_hits[i]);
	STATS_ZERO(conflict_misses_off_path[i]);
	STATS_ZERO(anticonflict_hits_off_path[i]);
    }

    STATS_ZERO(read_hits);
    STATS_ZERO(L2_hits);
    STATS_ZERO(vcache_hits);
    STATS_ZERO(write_hits);
    STATS_ZERO(reads_forwarded);
    STATS_ZERO(read_misses);
    STATS_ZERO(redundant_read_misses);
    STATS_ZERO(redundant_read_misses_off_path);
    STATS_ZERO(write_misses);
    STATS_ZERO(writes_combined);
    STATS_ZERO(blocks_kicked_out);
    STATS_ZERO(dirty_blocks_kicked_out);
    STATS_ZERO(dirty_blocks_written_back);
    STATS_ZERO(read_hits_off_path);
    STATS_ZERO(write_hits_off_path);
    STATS_ZERO(reads_forwarded_off_path);
    STATS_ZERO(read_misses_off_path);
    STATS_ZERO(write_misses_off_path);
    STATS_ZERO(prefetch_hits);
    STATS_ZERO(redundant_prefetches);
    STATS_ZERO(prefetch_buf_full);
    STATS_ZERO(prefetch_misses);
    STATS_ZERO(prefetch_requests_sent);
    STATS_ZERO(loads_used_prefetch_request);
    STATS_ZERO(prefetches_completed);
    STATS_ZERO(prefetch_hits_off_path);
    STATS_ZERO(redundant_prefetches_off_path);
    STATS_ZERO(prefetch_buf_full_off_path);
    STATS_ZERO(prefetch_misses_off_path);
    STATS_ZERO(cycles_blocked_miss_buffer);
    STATS_ZERO(cycles_blocked_write_buffer);
    STATS_ZERO(cycles_blocked_store_completes);

    STATS_ZERO(load_dest_C2);
    STATS_ZERO(load_dest_C3);
    STATS_ZERO(store_dest_C2);
    STATS_ZERO(store_dest_C3);

    STATS_ZERO(load_C2_hit_C2);
    STATS_ZERO(load_C2_miss);
    STATS_ZERO(load_C3_hit_C2);
    STATS_ZERO(load_C3_miss);
    STATS_ZERO(store_C2_hit_C2);
    STATS_ZERO(store_C2_miss);
    STATS_ZERO(store_C3_hit_C2);
    STATS_ZERO(store_C3_miss);

    /* Return the initialized structure */
    return (stats);
}

void S_add_stats_L2 (L2_Stats *dest,
                         L2_Stats *src1,
                         L2_Stats *src2)
{
    int i;

    for (i=0; i <= MAX_OPC; i++)
    {
	STATS_ADD(conflict_misses[i]);
	STATS_ADD(anticonflict_hits[i]);
	STATS_ADD(conflict_misses_off_path[i]);
	STATS_ADD(anticonflict_hits_off_path[i]);
    }

    STATS_ADD(read_hits);
    STATS_ADD(L2_hits);
    STATS_ADD(vcache_hits);
    STATS_ADD(write_hits);
    STATS_ADD(reads_forwarded);
    STATS_ADD(read_misses);
    STATS_ADD(redundant_read_misses);
    STATS_ADD(redundant_read_misses_off_path);
    STATS_ADD(write_misses);
    STATS_ADD(writes_combined);
    STATS_ADD(blocks_kicked_out);
    STATS_ADD(dirty_blocks_kicked_out);
    STATS_ADD(dirty_blocks_written_back);
    STATS_ADD(read_hits_off_path);
    STATS_ADD(write_hits_off_path);
    STATS_ADD(reads_forwarded_off_path);
    STATS_ADD(read_misses_off_path);
    STATS_ADD(write_misses_off_path);
    STATS_ADD(prefetch_hits);
    STATS_ADD(redundant_prefetches);
    STATS_ADD(prefetch_buf_full);
    STATS_ADD(prefetch_misses);
    STATS_ADD(prefetch_requests_sent);
    STATS_ADD(loads_used_prefetch_request);
    STATS_ADD(prefetches_completed);
    STATS_ADD(prefetch_hits_off_path);
    STATS_ADD(redundant_prefetches_off_path);
    STATS_ADD(prefetch_buf_full_off_path);
    STATS_ADD(prefetch_misses_off_path);
    STATS_ADD(cycles_blocked_miss_buffer);
    STATS_ADD(cycles_blocked_write_buffer);
    STATS_ADD(cycles_blocked_store_completes);

    STATS_ADD(load_dest_C2);
    STATS_ADD(load_dest_C3);
    STATS_ADD(store_dest_C2);
    STATS_ADD(store_dest_C3);

    STATS_ADD(load_C2_hit_C2);
    STATS_ADD(load_C2_miss);
    STATS_ADD(load_C3_hit_C2);
    STATS_ADD(load_C3_miss);
    STATS_ADD(store_C2_hit_C2);
    STATS_ADD(store_C2_miss);
    STATS_ADD(store_C3_hit_C2);
    STATS_ADD(store_C3_miss);
}

void S_add_stats_L2_bus (L2_Bus_Stats *dest, 
		      L2_Bus_Stats *src1,
		      L2_Bus_Stats *src2)

{
    STATS_ADD(icache_read_requests);
    STATS_ADD(icache_read_results);
    STATS_ADD(dcache_read_requests);
    STATS_ADD(dcache_read_results);
    STATS_ADD(dcache_prefetch_requests);
    STATS_ADD(dcache_prefetch_results);
    STATS_ADD(dcache_write_requests);
    STATS_ADD(waiting_for_result);
}

void S_print_stats_region_L2 (FILE *out, Stats *stats,
                                  char *rname, Stats *total_stats)
{
    double L2_total_read_hit_ratio;
    double L2_on_path_read_hit_ratio, L2_off_path_read_hit_ratio;
    double L2_write_hit_ratio;
    int L2_requests, L2_requests_off_path, L2_requests_on_path;
    int L2_hits, L2_hits_off_path, L2_hits_on_path;
    double L2_on_path_hit_ratio, L2_off_path_hit_ratio;
    double L2_hit_ratio;
    int prefetch_requests, prefetch_requests_off_path;
    L2_Stats *L2_stats;

    /* Setup Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;

    /* Get L2_stats for ease of use */
    L2_stats = stats->L2_cache;

    /* Want to distiguish parameter and result model lines */

    Pstats ("# L2:");
    Pstats ("");


    Pstats ("%12s L2 cache simulation model.",
            S_scache_model_name);

    Pstats ("%12u L2 cache requests.",
            L2_stats->read_hits + L2_stats->read_misses +
            L2_stats->write_hits + L2_stats->write_misses +
            L2_stats->prefetch_hits + L2_stats->prefetch_misses);

    Pstats ("%12u L2 cache on-path load requests.",
            (L2_stats->read_hits + L2_stats->read_misses) -
            (L2_stats->read_hits_off_path +
             L2_stats->read_misses_off_path));

    Pstats ("%12u L2 cache off-path load requests.",
            L2_stats->read_hits_off_path +
            L2_stats->read_misses_off_path);

    Pstats ("%12u L2 cache store requests (only on-path stores reach L2 cache).",
            L2_stats->write_hits + L2_stats->write_misses);

    Pstats ("%12u L2 cache total read hits (on and off-path).",
            L2_stats->read_hits);

    Pstats ("%12u L2 cache total read hits in L2 cache (on and off-path).",
            L2_stats->L2_hits);

    if (S_L2_victim_cache)
    	Pstats ("%12u L2 cache total read hits in victim cache (on and off-path).",
            L2_stats->vcache_hits);

    Pstats ("%12u L2 cache on-path read hits.",
             L2_stats->read_hits - L2_stats->read_hits_off_path);

    Pstats ("%12u L2 cache on-path reads forwarded.",
            L2_stats->reads_forwarded -
            L2_stats->reads_forwarded_off_path);

    Pstats ("%12u L2 cache off-path read hits.",
            L2_stats->read_hits_off_path);

    Pstats ("%12u L2 cache off-path reads forwarded.",
            L2_stats->reads_forwarded_off_path);

    Pstats ("%12u L2 cache total read misses (on and off-path).",
            L2_stats->read_misses);

    Pstats ("%12u L2 cache on-path read misses.",
            L2_stats->read_misses - L2_stats->read_misses_off_path);

    Pstats ("%12u L2 cache off-path read misses.",
            L2_stats->read_misses_off_path);


    Pstats ("%12u L2 cache total redundant read misses (on and off-path).",
            L2_stats->redundant_read_misses);

    Pstats ("%12u L2 cache on-path redundant read misses.",
            L2_stats->redundant_read_misses -
            L2_stats->redundant_read_misses_off_path);

    Pstats ("%12u L2 cache off-path redundant read misses.",
            L2_stats->redundant_read_misses_off_path);

    Pstats ("%12u L2 cache write hits (only on-path stores reach L2 cache).",
            L2_stats->write_hits);

    Pstats ("%12u L2 cache write misses (only on-path stores reach L2 cache).",
            L2_stats->write_misses);

    Pstats ("%12u L2 cache writes combined (only on-path stores reach L2 cache).",
            L2_stats->writes_combined);

    Pstats ("%12u L2 cache total blocks kicked out.",
            L2_stats->blocks_kicked_out);

    Pstats ("%12u L2 cache clean blocks kicked out.",
            L2_stats->blocks_kicked_out - L2_stats->dirty_blocks_kicked_out);

    Pstats ("%12u L2 cache dirty blocks kicked out.",
            L2_stats->dirty_blocks_kicked_out);

    Pstats ("%12u L2 cache dirty blocks written back.",
            L2_stats->dirty_blocks_written_back);


    Pstats ("%12u L2 cache cycles blocked due to full miss buffer.",
            L2_stats->cycles_blocked_miss_buffer);

    Pstats ("%12u L2 cache cycles blocked due to full write buffer.",
            L2_stats->cycles_blocked_write_buffer);

    Pstats ("%12u L2 cache cycles blocked waiting for a store to complete.",
            L2_stats->cycles_blocked_store_completes);
    Pstats ("");

    if (S_prefetch_cache)
    {
    	Pstats("%12u loads with L2 cache as target.",
                L2_stats->load_dest_C2);
    	Pstats("%12u loads with main memory as target.",
                L2_stats->load_dest_C3);
    	Pstats("%12u stores with L2 cache as target.",
                L2_stats->store_dest_C2);
    	Pstats("%12u stores with main memory as target.",
                L2_stats->store_dest_C3);
    	Pstats ("");

    	Pstats("%12u loads with L2 cache as target hit in L2 cache.",
                L2_stats->load_C2_hit_C2);
    	Pstats("%12u loads with L2 cache as target missed.",
                L2_stats->load_C2_miss);
    	Pstats("%12u loads with main memory as target hit in L2 cache.",
                L2_stats->load_C3_hit_C2);
    	Pstats("%12u loads with main memory as target missed.",
                L2_stats->load_C3_miss);
    	Pstats("%12u stores with L2 cache as target hit in L2 cache.",
                L2_stats->store_C2_hit_C2);
    	Pstats("%12u stores with L2 cache as target missed.",
                L2_stats->store_C2_miss);
    	Pstats("%12u stores with main memory as target hit in L2 cache.",
                L2_stats->store_C3_hit_C2);
    	Pstats("%12u stores with main memory as target missed.",
                L2_stats->store_C3_miss);
    	Pstats ("");
    }

    prefetch_requests = L2_stats->prefetch_hits +
        L2_stats->redundant_prefetches +
            L2_stats->prefetch_buf_full +
                L2_stats->prefetch_misses;

    prefetch_requests_off_path = L2_stats->prefetch_hits_off_path +
        L2_stats->redundant_prefetches_off_path +
            L2_stats->prefetch_buf_full_off_path +
                L2_stats->prefetch_misses_off_path;


    Pstats ("# L2 CACHE PREFETCHES:");
    Pstats ("");

    /* Dont print stats if ignoring prefetches */
    if (S_scache_ignore_prefetches)
    {
        Pstats ("%12s prefetch requests (for debugging purposes).",
                "IGNORING");
    }
    /* Dont print stats if no prefetch requests were made in
     * the whole program.
     */
    else if (!S_has_prefetches)
    {
        Pstats ("%12s prefetch requests.", "No");
    }

    /* Otherwise, print prefetch stats */
    else
    {

        Pstats ("%12u L2 cache total prefetch requests.",
                prefetch_requests);

        Pstats ("%12u L2 cache total prefetch misses.",
                L2_stats->prefetch_misses);

        Pstats ("%12u L2 cache total prefetch requests send to memory.",
                L2_stats->prefetch_requests_sent);

        Pstats ("%12u L2 cache total prefetches cancelled before memory request.",
                L2_stats->prefetch_misses -
                L2_stats->prefetch_requests_sent);

        Pstats ("%12u L2 cache total prefetches cancelled during memory request.",
                L2_stats->prefetch_requests_sent -
                L2_stats->prefetches_completed);

        Pstats ("%12u L2 cache total prefetches completed.",
                L2_stats->prefetches_completed);

        Pstats ("%12u L2 cache total loads used a outstanding prefetch request.",
                L2_stats->loads_used_prefetch_request);


        Pstats ("%12u L2 cache on-path prefetch requests.",
                prefetch_requests - prefetch_requests_off_path);

        Pstats ("%12u L2 cache on-path prefetch hits.",
                L2_stats->prefetch_hits - L2_stats->prefetch_hits_off_path);

        Pstats ("%12u L2 cache on-path redundant prefetches.",
                L2_stats->redundant_prefetches -
                L2_stats->redundant_prefetches_off_path);

        Pstats ("%12u L2 cache on-path prefetches discarded because buffer full.",
                L2_stats->prefetch_buf_full -
                L2_stats->prefetch_buf_full_off_path);

        Pstats ("%12u L2 cache on-path prefetch misses.",
                L2_stats->prefetch_misses -
                L2_stats->prefetch_misses_off_path);


        Pstats ("%12u L2 cache off-path prefetch requests.",
                prefetch_requests_off_path);

        Pstats ("%12u L2 cache off-path prefetch hits.",
                L2_stats->prefetch_hits_off_path);

        Pstats ("%12u L2 cache off-path redundant prefetches.",
                L2_stats->redundant_prefetches_off_path);

        Pstats ("%12u L2 cache off-path prefetches discarded because buffer full."
,
                L2_stats->prefetch_buf_full_off_path);

        Pstats ("%12u L2 cache off-path prefetch misses.",
                L2_stats->prefetch_misses_off_path);
    }
    Pstats ("");

    if (S_scache_measure_conflict_stats)
    {
	Pstats ("# L2 CACHE CONFLICTS:");
	Pstats ("");

	Pstats ("%12u L2 cache total conflict read misses.",
	        L2_stats->conflict_misses[LOAD_OPC]);

	Pstats ("%12u L2 cache on-path conflict read misses.",
	        L2_stats->conflict_misses[LOAD_OPC] -
	        L2_stats->conflict_misses_off_path[LOAD_OPC]);

	Pstats ("%12u L2 cache off-path conflict read misses.",
	        L2_stats->conflict_misses_off_path[LOAD_OPC]);

	Pstats ("%12u L2 cache total anti-conflict read hits.",
	        L2_stats->anticonflict_hits[LOAD_OPC]);

	Pstats ("%12u L2 cache on-path anti-conflict read hits.",
	        L2_stats->anticonflict_hits[LOAD_OPC] -
	        L2_stats->anticonflict_hits_off_path[LOAD_OPC]);

	Pstats ("%12u L2 cache off-path anti-conflict read hits.",
                L2_stats->anticonflict_hits_off_path[LOAD_OPC]);

	Pstats ("");

	if (S_scache_write_allocate)
	{
	    Pstats ("%12u L2 cache total conflict write misses.",
	            L2_stats->conflict_misses[STORE_OPC]);

	    Pstats ("%12u L2 cache on-path conflict write misses.",
	            L2_stats->conflict_misses[STORE_OPC] -
	            L2_stats->conflict_misses_off_path[STORE_OPC]);

	    Pstats ("%12u L2 cache off-path conflict write misses.",
	            L2_stats->conflict_misses_off_path[STORE_OPC]);

	    Pstats ("%12u L2 cache total anti-conflict write hits.",
	            L2_stats->anticonflict_hits[STORE_OPC]);

	    Pstats ("%12u L2 cache on-path anti-conflict write hits.",
	            L2_stats->anticonflict_hits[STORE_OPC] -
	            L2_stats->anticonflict_hits_off_path[STORE_OPC]);

	    Pstats ("%12u L2 cache off-path anti-conflict write hits.",
	            L2_stats->anticonflict_hits_off_path[STORE_OPC]);

	    Pstats ("");
	}
    }

    /*
     * L2 CACHE READ-WRITE HIT RATIOS
     */
    if ((L2_stats->read_hits + L2_stats->read_misses)> 0)
        L2_total_read_hit_ratio = 100.0 *
            ((double) L2_stats->read_hits)/
                ((double) L2_stats->read_hits + L2_stats->read_misses);
    else
        L2_total_read_hit_ratio = 0.0;


    if (((L2_stats->read_hits + L2_stats->read_misses) -
         (L2_stats->read_hits_off_path +
          L2_stats->read_misses_off_path)) > 0)
        L2_on_path_read_hit_ratio = 100.0 *
            ((double) L2_stats->read_hits -
             (double) L2_stats->read_hits_off_path)/
                 (((double) L2_stats->read_hits +
                   (double) L2_stats->read_misses) -
                  ((double) L2_stats->read_hits_off_path +
                   (double) L2_stats->read_misses_off_path));
    else
        L2_on_path_read_hit_ratio = 0.0;

    if ((L2_stats->read_hits_off_path+L2_stats->read_misses_off_path)>0)
        L2_off_path_read_hit_ratio = 100.0 *
            ((double) L2_stats->read_hits_off_path)/
                ((double) L2_stats->read_hits_off_path +
                 (double) L2_stats->read_misses_off_path);
    else
        L2_off_path_read_hit_ratio = 0.0;

    Pstats ("# L2 CACHE SUMMARY:");
    Pstats ("");
    Pstats ("%12.2lf L2 cache total read hit ratio.",
            L2_total_read_hit_ratio);

    Pstats ("%12.2lf L2 cache on-path read hit ratio.",
            L2_on_path_read_hit_ratio);

    Pstats ("%12.2lf L2 cache off-path read hit ratio.",
            L2_off_path_read_hit_ratio);

    if ((L2_stats->write_hits + L2_stats->write_misses)> 0)
        L2_write_hit_ratio = 100.0 * ((double) L2_stats->write_hits)/
            ((double) L2_stats->write_hits + L2_stats->write_misses);
    else
        L2_write_hit_ratio = 0.0;

    Pstats ("%12.2lf L2 cache write hit ratio.",
            L2_write_hit_ratio);


    /* Calculate combined load/store hit ratio */
    L2_requests =
        L2_stats->read_hits + L2_stats->read_misses +
            L2_stats->write_hits + L2_stats->write_misses;
    L2_requests_off_path =  L2_stats->read_hits_off_path +
        L2_stats->read_misses_off_path;
    L2_requests_on_path = L2_requests - L2_requests_off_path;
    L2_hits = L2_stats->read_hits + L2_stats->write_hits;
    L2_hits_off_path = L2_stats->read_hits_off_path;
    L2_hits_on_path = L2_hits - L2_hits_off_path;


    if (L2_requests_on_path > 0)
        L2_on_path_hit_ratio = 100.0 *
            ((double)L2_hits_on_path)/ ((double)L2_requests_on_path);
    else
        L2_on_path_hit_ratio = 0.0;

    if (L2_requests_off_path > 0)
        L2_off_path_hit_ratio = 100.0 *
            ((double)L2_hits_off_path)/ ((double)L2_requests_off_path);
    else
        L2_off_path_hit_ratio = 0.0;

    if (L2_requests > 0)
        L2_hit_ratio = 100.0 *
            ((double)L2_hits)/ ((double)L2_requests);
    else
        L2_hit_ratio = 0.0;


    Pstats ("%12.2lf L2 cache total read-write hit ratio.",
            L2_hit_ratio);

    Pstats ("%12.2lf L2 cache on-path read-write hit ratio.",
            L2_on_path_hit_ratio);

    Pstats ("%12.2lf L2 cache off-path read-write hit ratio (no off-path writes)."
,
            L2_off_path_hit_ratio);


    Pstats ("");

}

void S_print_stats_region_L2_bus (FILE *out, Stats *stats,
			       char *rname, Stats *total_stats)
{
    unsigned bus_cycles_free;
    unsigned cycles_simulated;
    L2_Bus_Stats *bstats;
    
    /* Setup Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;

    /* Get the bus stats structure for ease of use */
    bstats = stats->L2_bus;

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
	 bstats->waiting_for_result);
    

    Pstats ("# L2 BUS:");
    Pstats ("");
    Pstats ("%12u L2 bus cycles simulated.", cycles_simulated);
    Pstats ("%12i L2 bus cycles unused.", bus_cycles_free);
    Pstats ("%12u L2 bus cycles used for icache read requests.",
	     bstats->icache_read_requests);
    Pstats ("%12u L2 bus cycles used for icache read returns.",
	     bstats->icache_read_results);
    Pstats ("%12u L2 bus cycles used for dcache read requests.",
	     bstats->dcache_read_requests);
    Pstats ("%12u L2 bus cycles used for dcache read returns.",
	     bstats->dcache_read_results);
    Pstats ("%12u L2 bus cycles used for dcache prefetch requests.",
	     bstats->dcache_prefetch_requests);
    Pstats ("%12u L2 bus cycles used for dcache prefetch returns.",
	     bstats->dcache_prefetch_results);
    Pstats ("%12u L2 bus cycles spent waiting for read result to return.",
	     bstats->waiting_for_result);
    Pstats ("%12u L2 bus cycles used for dcache write requests.",
	     bstats->dcache_write_requests);
    Pstats ("%12.2lf percent L2 bus utilization.",
	     (double) 100.0 * ((double)(cycles_simulated - bus_cycles_free))/
	     ((double) cycles_simulated));
    Pstats ("");
}



void S_read_parm_L2 (Parm_Parse_Info *ppi)
{
    L_read_parm_b (ppi, "secondary_cache", &S_secondary_cache);
    L_read_parm_i (ppi, "scache_latency", &S_scache_latency);
    L_read_parm_s (ppi, "scache_model", &S_scache_model_name);
    L_read_parm_i (ppi, "scache_size", &S_scache_size);
    L_read_parm_i (ppi, "scache_block_size", &S_scache_block_size);
    L_read_parm_i (ppi, "scache_assoc", &S_scache_assoc);
    L_read_parm_b (ppi, "scache_debug_misses", &S_scache_debug_misses);
    L_read_parm_b (ppi, "scache_measure_conflict_stats",
                   &S_scache_measure_conflict_stats);
    L_read_parm_b (ppi, "scache_combining_write_buf",
                   &S_scache_combining_write_buf);
    L_read_parm_b (ppi, "scache_write_allocate", &S_scache_write_allocate);
    L_read_parm_i (ppi, "scache_miss_bypass_limit",
                   &S_scache_miss_bypass_limit);
    L_read_parm_i (ppi, "scache_prefetch_buf_size",
                   &S_scache_prefetch_buf_size);
    L_read_parm_b (ppi, "scache_debug_prefetch", &S_scache_debug_prefetch);
    L_read_parm_b (ppi, "scache_ignore_prefetch_bit",
                   &S_scache_ignore_prefetch_bit);
    L_read_parm_b (ppi, "scache_ignore_prefetches",
                   &S_scache_ignore_prefetches);
    L_read_parm_i (ppi, "scache_write_buf_size", &S_scache_write_buf_size);

    L_read_parm_b (ppi, "L2_victim_cache", &S_L2_victim_cache);
    L_read_parm_i (ppi, "L2_vcache_size", &S_L2_vcache_size);
    L_read_parm_i (ppi, "L2_vcache_block_size", &S_L2_vcache_block_size);
}

void S_read_parm_L2_bus (Parm_Parse_Info *ppi)
{
    L_read_parm_s (ppi, "L2_bus_model", &S_L2_bus_model_name);
    L_read_parm_b (ppi, "debug_L2_bus", &S_debug_L2_bus); 
    L_read_parm_i (ppi, "L2_bus_bandwidth", &S_L2_bus_bandwidth);
    L_read_parm_b (ppi, "L2_streaming_support", &S_L2_streaming_support);
}

void S_print_configuration_L2_vcache (FILE *out)
{
    fprintf (out, "# L2 VICTIM CACHE CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12u L2 vcache size.\n", S_L2_vcache_size);
    fprintf (out, "%12u L2 vcache block size.\n", S_L2_vcache_block_size);
    fprintf (out, "%12u L2 vcache assoc.\n", 0);
    fprintf (out,"\n");
}

void S_print_configuration_L2 (FILE *out)
{
    if (!S_secondary_cache) return;

    fprintf (out, "# L2 CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s L2 cache model.\n", S_scache_model_name);
    if (S_scache_model == L2_MODEL_NON_BLOCKING)
        fprintf (out, "%12u miss bypass limit.\n", S_scache_miss_bypass_limit);
    if (S_scache_model != L2_MODEL_PERFECT)
    {
        fprintf (out, "%12u L2 cache size.\n", S_scache_size);
        fprintf (out, "%12u L2 cache block size.\n", S_scache_block_size);
        fprintf (out, "%12u L2 cache assoc.\n", S_scache_assoc);
        if (S_scache_measure_conflict_stats)
            fprintf (out, "%12s ", "Do");
        else
            fprintf (out, "%12s ", "Do not");
        fprintf (out, "measure conflict stats (in L2 cache).\n");
        fprintf (out, "%12u L2 cache write buf size.\n",
                 S_scache_write_buf_size);
        if (S_scache_combining_write_buf)
            fprintf (out, "%12s ", "Yes");
        else
            fprintf (out, "%12s ", "No");
        fprintf (out, "L2 cache combining write buf support.\n");
        if (S_scache_write_allocate)
            fprintf (out, "%12s ", "Yes");
        else
            fprintf (out, "%12s ", "No");
        fprintf (out, "L2 cache write allocate protocol.\n");
    }
    fprintf (out, "%12u cycle L2 cache latency.\n", S_scache_latency);
    fprintf (out, "\n");

    if (S_L2_victim_cache)
        S_print_configuration_L2_vcache (out);
}

void S_print_configuration_L2_bus (FILE *out)
{
    if (!S_secondary_cache) return;

    fprintf (out, "# L2 BUS CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s transaction L2 bus model.\n", S_L2_bus_model_name);
    fprintf (out, "%12u bytes/cycle L2 bus bandwidth.\n", S_L2_bus_bandwidth);
    if (S_streaming_support)
	fprintf (out, "%12s ", "Yes");
    else
	fprintf (out, "%12s ", "No");
    fprintf (out, 
	     "streaming support (forwarding miss data as it comes in).\n");
    fprintf (out, "\n");

}

void S_init_L2_bus()
{
    /* Set L2_bus model */
    if (L_pmatch (S_L2_bus_model_name, "single"))
    {
	/* Reassign name to make output look nice */
	S_L2_bus_model_name = "Single";
	S_L2_bus_model = BUS_MODEL_SINGLE;
    }
    else if (L_pmatch (S_L2_bus_model_name, "split"))
    {
	/* Reassign name to make output look nice */
	S_L2_bus_model_name = "Split";
	S_L2_bus_model = BUS_MODEL_SPLIT;
    }
    else 
	S_punt ("S_init_L2_bus: Undefined bus model '%s'.", S_bus_model_name);

    /* S_L2_bus_bandwidth (in bytes) must be >= 1 */
    if (S_L2_bus_bandwidth < 1)
	S_punt ("S_init_L2_bus: bus_bandwidth (%i) must be >= 1.", 
		S_L2_bus_bandwidth);

    /* S_L2_bus_bandwidth must also be a power of 2 */
    if (!S_is_power_of_two (S_L2_bus_bandwidth))
	S_punt ("S_init_L2_bus: L2_bas_bandwidth (%i) must be power of 2.",
		S_L2_bus_bandwidth);
    

    /* Set size_offset and size_shift for calculating length of transaction*/
    S_L2_bus.size_offset = S_L2_bus_bandwidth - 1;
    S_L2_bus.size_shift = S_log_base_two (S_L2_bus_bandwidth);

    /* 
     * Initialize bus settings.  This is the only initialization the
     * bus will get.
     */
    S_L2_bus.avail = 1;
    S_L2_bus.L2_only = 0;
    S_L2_bus.type = 0;
    S_L2_bus.src = NO_ONE;
    S_L2_bus.dest = NO_ONE;
    S_L2_bus.addr = 0;
    S_L2_bus.size = 0;
    S_L2_bus.length = 0;
    S_L2_bus.segment = 0;
    S_L2_bus.stats = NULL;
}

void S_sim_L2_bus ()
{
    L2_Bus_Stats *bstats;


    /* Only need to simulation if bus is currently being used or if
     * use ended last cycle (if already available, do nothing).
     */
    if (!S_L2_bus.avail && !S_L2_bus.L2_only)
    {
	/* Use the current transactions stats structure or the last
	 * transactions stats.  Works as long as don't count unused
	 * cycles.  Also, can cause a region to take more bus cycles
	 * than the region has.  This means that region of code is
	 * really stressing the bus.
	 */
	bstats = S_L2_bus.stats->L2_bus;

	/* Update stats */
	switch (S_L2_bus.type)
	{
	  case READ_REQUEST:
	    if (S_L2_bus.src == DCACHE)
	    {
		bstats->dcache_read_requests++;
	    }
	    else
	    {
		bstats->icache_read_requests++;
	    }
	    break;

	  case READ_RESULT:
	    if (S_L2_bus.dest == DCACHE)
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

	  default:
	    S_punt ("S_sim_L2_bus: Unknown bus type %i.\n", S_L2_bus.type);
	}

    	/* Have we reached the end of the transaction? */
	if (S_L2_bus.segment == S_L2_bus.length)
	{
	    /* At the end of the transaction */
	    /* If in single-transaction mode and just did a read request,
	     * then it is only available to memory.
	     */
	    if (((S_L2_bus.type == READ_REQUEST) || 
		 (S_L2_bus.type == PREFETCH_REQUEST)) && 
		(S_L2_bus_model == BUS_MODEL_SINGLE))
	    {
		S_L2_bus.L2_only = 1;
	    }
	    /* Otherwise, bus available to everyone */
	    else
	    {
		S_L2_bus.avail = 1;
	    }
	    /* Zero src, dest for ease of programming */
	    S_L2_bus.src = NO_ONE;
	    S_L2_bus.dest = NO_ONE;
	}

	/* Still doing transaction, increment segment transfering */
	else
	{
	    S_L2_bus.segment++;
	}

    }
    else if (S_L2_bus.L2_only)
    {
	S_L2_bus.stats->L2_bus->waiting_for_result++;
    }
}

static void S_print_L2_component_name(FILE *out, int comp_id)
{
    switch (comp_id)
    {
      case MEMORY:
	fprintf (out, "memory");
	break;

      case L2:
	fprintf (out, "L2 cache");
	break;

      case DCACHE:
	fprintf (out, "dcache");
	break;
	
      case ICACHE:
	fprintf (out, "icache");
	break;

      default:
	fprintf (out, "unknown type %i", comp_id);
    }
}

/* Print out the bus activity for this cycle */
void S_print_L2_bus_state (FILE *out)
{
    fprintf (out, "L2 Bus cycle %4i: ", S_sim_cycle);
    
    if (S_L2_bus.avail)
	fprintf (out, "L2 Idle\n");
    else if (S_L2_bus.L2_only)
	fprintf (out, "Waiting for L2 result\n");
    else
    {
	switch (S_L2_bus.type)
	{
	  case READ_REQUEST:
	    fprintf (out, "Read request from ");
	    S_print_L2_component_name (out, S_L2_bus.src);
	    fprintf (out, "   ");
	    break;

	  case PREFETCH_REQUEST:
	    fprintf (out, "Prefetch request from ");
	    S_print_L2_component_name (out, S_L2_bus.src);
	    break;

	  case READ_RESULT:
	    fprintf (out, "L2 block to ");
	    S_print_L2_component_name (out, S_L2_bus.dest);
	    fprintf (out, "     ");
	    break;

	  case PREFETCH_RESULT:
	    fprintf (out, "Prefetched block to ");
	    S_print_L2_component_name (out, S_L2_bus.dest);
	    fprintf (out, "  ");
	    break;

	  case WRITE_REQUEST:
	    fprintf (out, "Write from ");
	    S_print_L2_component_name (out, S_L2_bus.src);
	    fprintf (out, "          ");
	    break;

	  default:
	    fprintf (out, "Unknown (%i) from ", S_L2_bus.type);	
	    S_print_L2_component_name (out, S_L2_bus.src);
	    fprintf (out, " to ");
	    S_print_L2_component_name (out, S_L2_bus.dest);
	}
	fprintf (out, " %3i bytes  cycle %i of %i  addr %08x\n", S_L2_bus.size,
		 S_L2_bus.segment, S_L2_bus.length, S_L2_bus.addr);
	
    }
}

void S_sim_L2_bus_transaction (int type, int src, int dest, int addr, 
		int size, int playdoh_flags, Stats *stats)
{
    /* Sanity check, the bus better not be in use */
    if (!S_L2_bus.avail && !(S_L2_bus.L2_only && (src == L2)))
	S_punt ("S_sim_L2_bus_transaction: '%i' accessing bus when in use.",
		src);

    /* Mark bus no longer available */
    S_L2_bus.avail = 0;
    S_L2_bus.L2_only = 0;

    /* Save transaction parameters */
    S_L2_bus.type = type;
    S_L2_bus.src = src;
    S_L2_bus.dest = dest;
    S_L2_bus.addr = addr;
    S_L2_bus.size = size;
    /* If this request has a target of either C1 or V1 need to reset
     * flags locally to target of C2.
     */
    if (playdoh_flags & (PLAYDOH_TCHS_C1 | PLAYDOH_TCHS_V1))
	playdoh_flags = (playdoh_flags&(~(PLAYDOH_TCHS_C1|PLAYDOH_TCHS_V1)))
							|PLAYDOH_TCHS_C2;
    S_L2_bus.playdoh_flags = playdoh_flags;
    S_L2_bus.stats = stats;

    /* Calculate transaction length */
    switch (type)
    {
      case READ_REQUEST:
      case PREFETCH_REQUEST:
	S_L2_bus.length = 1;
	break;
	
      case READ_RESULT:
      case PREFETCH_RESULT:
	S_L2_bus.length = (size + S_L2_bus.size_offset) >> S_L2_bus.size_shift;
	break;
	
      case WRITE_REQUEST:
	S_L2_bus.length = (size + S_L2_bus.size_offset) >> S_L2_bus.size_shift;
	break;

      default:
	S_punt ("S_sim_L2_bus_transaction: unknown type '%i'.", type);
    }


    /* Initialize segment to be first segment of transaction */
    S_L2_bus.segment = 1;
}

/***************************************************************************/

void *S_create_L2_block_data()
{
    L2_Data *data;

    if ((data = (L2_Data *) malloc (sizeof(L2_Data))) == NULL)
	S_punt("S_create_L2_block_data: Out of memory");

    data->flags = 0;

    return (data);
}

void * S_create_L2_vcache_block_data ()
{
    L2_Data *data;

    if ((data = (L2_Data *) malloc (sizeof (L2_Data))) == NULL)
        S_punt ("S_create_L2_vcache_block_data: Out of memory");

    data->flags = 0;

    return (data);
}

L2_Cache *S_create_L2(Pnode *pnode)
{
    char name_buf[5];
    int transfer_time;
    L2_Cache *S_L2;

    if (L_pmatch (S_scache_model_name, "perfect"))
    {
        /* Reassign name to make output look nice */
        S_scache_model_name = "Perfect";
        S_scache_model = L2_MODEL_PERFECT;
    }
    else if (L_pmatch (S_scache_model_name, "blocking"))
    {
        /* Reassign name to make output look nice */
        S_scache_model_name = "Blocking";
        S_scache_model = L2_MODEL_BLOCKING;

        /* Set bypass limit to 0 to force blocking */
        S_scache_miss_bypass_limit = 0;
    }
    else if (L_pmatch (S_scache_model_name, "Non-blocking"))
    {
        /* make sure bypass limit > 0 (else Non-blocking doesn't make sense) */
        if (S_scache_miss_bypass_limit <= 0)
           S_punt("S_init_L2: Invalid scache_miss_bypass_limit of %i \
                for non-blocking cache.\n",S_scache_miss_bypass_limit);

        /* Reassign name to make output look nice */
        S_scache_model_name = "NonBlocking";
        S_scache_model = L2_MODEL_NON_BLOCKING;
    }
    else
        S_punt ("S_create_scache: Undefined scache model '%s'.",
                S_scache_model_name);

    if (L2_pool == NULL)
    {
        L2_pool = L_create_alloc_pool ("L2", sizeof (L2_Cache), 1);
    }


    S_L2 = (L2_Cache *) L_alloc (L2_pool);

    sprintf(name_buf,"L2");
    S_L2->name = strdup(name_buf);

    /* Must have a page size that is a power of two and greater than
     * the block size.
     */
    if (S_memory_page_size < S_scache_block_size)
        S_punt ("memory_page_size (%i) must be > scache_block_size (%i).",
                S_memory_page_size, S_scache_block_size);

    /* If not perfect L2 cache, allocate cache */
    if (S_scache_model != L2_MODEL_PERFECT)
    {
        /* Make sure L2 cache block size is a multiple of 8 bytes */
        if ((S_scache_block_size < 8) || (S_scache_block_size & 7))
        S_punt ("S_init_L2: scache_block_size %i must be multiple of 8.",
                    S_scache_block_size);

        /* Create cache, L2 cache block flags needed */
        S_L2->cache = S_create_cache (S_scache_size, S_scache_block_size,
                                        S_scache_assoc, S_create_L2_block_data);

        /* If measuring cache conflicts, create conflict cache (full assoc).
         * No block data needed.
         */
        if (S_scache_measure_conflict_stats)
        {
            S_L2->conflict_cache = S_create_cache (S_scache_size,
                                                     S_scache_block_size,
                                                     0, NULL);
        }

	if (S_L2_victim_cache)
            L2_vcache = S_create_cache (S_L2_vcache_size,
                        S_L2_vcache_block_size, 0,
                        S_create_L2_vcache_block_data);
    }

    S_L2->new_pending_head = NULL;
    S_L2->new_pending_tail = NULL;
    S_L2->pending_head = NULL;
    S_L2->pending_tail = NULL;

    S_L2->write_buffer = M_create_queue ("L2_write_buffer", 0);
    S_L2->miss_buffer = M_create_queue ("L2_miss_buffer", 0);
    S_L2->miss_request_buffer =
        M_create_queue ("L2_miss_request_buffer", 0);
    S_L2->prefetch_request_buffer =
        M_create_queue ("L2_prefetch_request_buffer", 0);
    S_L2->pending_prefetches =
        M_create_queue ("L2_pending_prefetches", 0);
    S_L2->addr_loaded_last_cycle = -1;
    S_L2->L2_stats = NULL;

    /* Initialize bus */
    S_bus.cycle_avail = 0;

    /* Make sure S_bus_bandwidth is power of two */
    if (!S_is_power_of_two (S_L2_bus_bandwidth))
        S_punt ("S_L2_bus_bandwidth (%i) must be power of two.",
                S_L2_bus_bandwidth);

    /* Calculate latencies for L2 (to memory) */
    transfer_time = (S_scache_block_size/S_bus_bandwidth);

    S_C3_addr_mask = ~(S_bus_bandwidth - 1);
    S_C3_req_size = S_bus_bandwidth;

    /* Assume have separate address lines, and get first word
     * after S_memory_latency cycles
     */
    S_scache_read_block_latency = 1 + S_memory_latency + (transfer_time - 1);
    S_scache_write_block_latency = transfer_time;
    S_scache_write_thru_latency = 1;
    if (S_L2_streaming_support)
        S_scache_streaming_benefit = transfer_time - 1;
    else
        S_scache_streaming_benefit = 0;

    return S_L2;
}

/*
 * Searches queue from tail (most recent) to head for an entry
 * whose memory address overlaps.
 */
Mentry *find_overlapping_L2_entry (Mqueue *queue, int addr,
                                  unsigned req_conflict_mask)
{
    Mentry    *search;
    unsigned    conflict_mask;

    /* Start search at "tail", most recent entry */
    /* => now start at "head", oldest entry */
    for (search = queue->head; search != NULL; search = search->next_entry)
    {
        /* Use mask to compare addresses */
        conflict_mask = req_conflict_mask & ~(search->size - 1);

        /* If overlaps with entry, return this entry */
        if ((addr & conflict_mask) ==
            (search->addr & conflict_mask))
        {
            return (search);
        }
    }

    /* No overlapping entry found */
    return (NULL);
}

/*
 * Searches queue from tail (most recent) to head for an entry
 * whose memory address and size include that of the given address and dest.
 */
Mentry *find_covering_L2_entry(int dest_request_size,
                Mqueue *queue, int addr, unsigned req_conflict_mask)
{
    L2_Request *request;
    Mentry    *search;
    unsigned    conflict_mask;
    int         search_request_size;
    int         search_dest;

    /* Start search at "tail", most recent entry */
    /* => now start at "head", oldest entry */
    for (search = queue->head; search != NULL; search = search->next_entry)
    {
	request = (L2_Request *) search->request;

        search_dest = search->playdoh_flags & PLAYDOH_TCHS_ALL;

        if (search_dest & PLAYDOH_TCHS_C3)
	{
	    search_request_size = S_C3_req_size;
	    /* Sanity check - if we have specified C3 as the
	     * dest, then only C3 should be the dest!.
	     */
	    if (search_dest != PLAYDOH_TCHS_C3)
	      S_punt("Target cache of queue entry is C3 and other cache(s): %d",
								search_dest);
	}
        else
            search_request_size = S_scache_block_size;

        /* Use mask to compare addresses */
        conflict_mask = req_conflict_mask & ~(search_request_size - 1);

        /* If sint operlaps with entry, return this entry */
        if (((addr & conflict_mask) ==
            (request->addr & conflict_mask)) &&
            search_request_size >= dest_request_size)
        {
            return (search);
        }
    }

    /* No overlapping entry found */
    return (NULL);
}

void cancel_L2_prefetch (L2_Cache *S_L2, Scblock *block)
{
    /**** Not handling prefetches yet ****/
    return;
}

/* Returns 1 if already request out for block at the given address */
int S_find_outstanding_L2_request (L2_Cache *S_L2, L2_Request *request)
{
    int block_addr, addr_mask, dest;
    Mentry *overlaps;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler warning.
     */
#if 0
    int overlapping_entry = 0;
#endif
    int request_size;
    unsigned bus_conflict_mask;
    L2_Stats *L2_stats;
    Scache *cache;

    /* Get stats struct */
    L2_stats = S_L2->L2_stats;

    cache = S_L2->cache;

    if ((request->playdoh_flags & PLAYDOH_TCHS_C2) ||
                !(request->playdoh_flags & PLAYDOH_TCHS_ALL))
    {
        dest = PLAYDOH_TCHS_C2;
        request_size = S_scache_block_size;
        /* Get mask to turn addresses into block addresses */
        addr_mask =  cache->start_addr_mask;
    }
    else
    {
        dest = PLAYDOH_TCHS_C3;
        request_size = S_C3_req_size;
        /* Get mask to turn addresses into block addresses */
        addr_mask = S_C3_addr_mask;
    }

    /* Get bus request address */
    block_addr = request->addr & addr_mask;

    bus_conflict_mask = ~(S_bus.size - 1);

    /* If data is coming in on the bus this cycle, then don't need request */
    if ((S_bus.dest == L2) &&
        (S_bus.addr == (block_addr & bus_conflict_mask)) &&
        ((S_current_L2_dest & dest) || (S_bus.size >= request_size)))
    {
           /* must add this dest to the current_dest */
           if (request->playdoh_flags & PLAYDOH_TCHS_ALL)
               S_current_L2_dest |= dest;

           /* if this is a prefetch, make sure it is not thrown away */
	   /**** Don't handle prefetches yet ****/
           if (S_scache_debug_misses)
           {
                fprintf (debug_out, "L2 BUS RESPONSE of %x COMBINED \n",
                        S_bus.addr);
           }
           return (1);
    }

    /* If have a prefetch request that has already gone to memory, then
     * already have request for block. Check prefetches first since
     * they have already gone to memory.
     */
    /**** Don't handle prefetches yet ****/

    /* Look for outstanding loads or write_allocate store (causes load),
     * if found, load request has already been issued
     */
    overlaps = find_covering_L2_entry(request_size,
                        S_L2->miss_buffer, block_addr, addr_mask);
    if (overlaps != NULL)
    {
        if (S_scache_debug_misses)
        {
            fprintf (debug_out,
                 "L2 LOAD of %x COMBINED\n",
                 request->addr);
        }

        overlaps->playdoh_flags |=
                (request->playdoh_flags & PLAYDOH_TCHS_ALL);

        return (1);
    }

    /* Checked everywhere, no outstanding request for block */
    return (0);
}

void replace_L2_block (L2_Cache *S_L2, Scblock *block, int new_addr,
                           Stats *stats)
{
    L2_Data *data, *data2;
    L2_Stats *L2_stats;
    Scache *cache = NULL;
    Scblock *block2 = NULL;
    unsigned addr2 = 0;
    int block_size;

    if (S_current_L2_dest & PLAYDOH_TCHS_C2)
    	cache = S_L2->cache;
    else if (S_L2_victim_cache)
    	cache = L2_vcache;
    else
	S_punt("replace_L2_block: Called with dest != C2: dest %d",
						S_current_L2_dest);
    block_size = cache->block_size;

    /*
     * Get L2 stats structure for ease of use.
     */
    L2_stats = stats->L2_cache;

    /* If we are doing victim caching and we are replacing
     * a block in the main L2 cache, instead replace oldest
     * entry in L2 victim cache.
     */
    if (S_L2_victim_cache && cache == S_L2->cache)
    {
	block2 = block;
	addr2 = block2->start_addr;
	block = S_cache_find_LRU(L2_vcache,addr2);
        block_size = L2_vcache->block_size;
    }

    /* Get block's data */
    data = (L2_Data *) block->data;

    /* If block is valid have conflict and need to check
     * if dirty
     */
    if (block->hash_next != (Scblock *)-1)
    {
        /* Update stats */
        L2_stats->blocks_kicked_out++;

        if (S_scache_debug_misses)
        {
            fprintf (debug_out,
                     "Line full, kicked out %x of L2 cache",
                     block->start_addr);
            if (data->flags & DIRTY_BLOCK)
                fprintf (debug_out, " (DIRTY)\n");
            else
                fprintf (debug_out, "\n");
        }

        /* If we are replacing an dirty block, place write back
         * request into write buffer.
         *
         * Create an fake sint to do this (this could be nasty)
         */
        if (data->flags & DIRTY_BLOCK)
        {
            /* Update stats */
            L2_stats->dirty_blocks_kicked_out++;

            M_enqueue (S_L2->write_buffer, WRITE_REQUEST,
			L2, MEMORY, block->start_addr, block_size, -1,
			S_sim_cycle, stats);
        }
    }

    /* Change block to new address */
    if (S_L2_victim_cache && cache == S_L2->cache)
    {
        S_cache_change_addr (L2_vcache, block, addr2);
        data2 = data;
        data = (L2_Data *) block->data;
	block = block2;
        data2->flags = data->flags;
    }
    S_cache_change_addr (cache, block, new_addr);

    /* Mark block as not dirty */
    data->flags = 0;
}

/* Makes an address MRU if in conflict cache */
void S_make_scache_conflict_cache_MRU (L2_Cache *S_L2, int addr)
{
    Scblock     *block;


    /* Is block in conflict cache */
    block = S_cache_find_addr (S_L2->conflict_cache, addr);

    /* If Hit, make MRU */
    if (block != NULL)
    {
        /* Make block MRU (if hit or miss) */
        S_cache_make_MRU (S_L2->conflict_cache, block);
    }

}

/* Adds an address to the conflicct cache */
void S_add_to_scache_conflict_cache (L2_Cache *S_L2, int addr)
{
    Scblock     *block;

    /* Is block in conflict cache */
    block = S_cache_find_addr (S_L2->conflict_cache, addr);

    /* If Miss, get LRU */
    if (block == NULL)
    {
        block = S_cache_find_LRU (S_L2->conflict_cache, addr);
        S_cache_change_addr (S_L2->conflict_cache, block, addr);
    }

    /* Make MRU if hit or miss */
    S_cache_make_MRU (S_L2->conflict_cache, block);
}

/*
 * Updats conflict stats but does not update conflict cache.
 * Use the two above functions to add to conflict cache or make an
 * address MRU.
 */
void S_update_scache_conflict_stats (int opc_type, L2_Cache *S_L2,
                                       L2_Stats *L2_stats,
                                       int addr, int scache_hit)
{
    Scblock     *block;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler warning.
     */
#if 0
    int         conflict_cache_hit;
#endif


    /* Is block in conflict cache */
    block = S_cache_find_addr (S_L2->conflict_cache, addr);

    /* Hit, update conflict miss stats if scache misses*/
    if (block != NULL)
    {
        if (!scache_hit)
        {
            L2_stats->conflict_misses[opc_type]++;
        }
    }

    /* If miss and scache hits then update anticonflict hit stats */
    else
    {
        if (scache_hit)
        {
            L2_stats->anticonflict_hits[opc_type]++;
        }
    }
}

/* Search write buffer for stores into this block. Remove them and mark
 * block dirty if any found and if WRITE_THROUGH flag not set.
 */
void write_buffer_to_L2_block(L2_Cache *S_L2, Scblock *block,
                L2_Data *data, unsigned start_addr_mask)
{
    Scache *cache = NULL;
    Mentry *entry,*next_entry;
    unsigned addr_mask;

    if (S_current_L2_dest & PLAYDOH_TCHS_C2)
    	cache = S_L2->cache;
    else if (S_L2_victim_cache)
    	cache = L2_vcache;
    else
	S_punt("write_buffer_to_L2_block: Called with dest != C2: dest %d",
						S_current_L2_dest);

    for (entry = S_L2->write_buffer->head; entry != NULL;
                 entry = next_entry)
    {
        /* Get next entry before messing with this one */
        next_entry = entry->next_entry;

        addr_mask = start_addr_mask & ~(entry->size - 1);

        if (block->start_addr == (entry->addr & addr_mask))
        {
            /* Mark block as dirty */
            data->flags |= DIRTY_BLOCK;

            /* Finished with store sint, free entry and it
                    if it is a subset of this block */
            if (~(cache->block_size - 1) <= ~(entry->size - 1))
            {
                M_dequeue (entry);
            }
        }
    }
}

void L2_bus_data_to_cache (L2_Cache *S_L2,unsigned bus_addr,unsigned addr,
                        Stats *bus_stats)
{
    Scache *cache = NULL;
    Scblock *block;
    /* 10/25/04 REK Commenting out unused variables to quiet compiler
     *              warnings. */
#if 0
    Mentry *entry,*next_entry;
    unsigned addr_mask;
#endif
    L2_Data *data;
    unsigned start_addr_mask;

    if (S_current_L2_dest & PLAYDOH_TCHS_C2)
    	cache = S_L2->cache;
    else if (S_L2_victim_cache)
    	cache = L2_vcache;
    else
	S_punt("L2_bus_data_to_cache: Called with dest != C2: dest %d",
						S_current_L2_dest);

    start_addr_mask = cache->start_addr_mask;

    /* Debug, make sure block is not already in cache */
    if (S_cache_find_addr (cache, addr) != NULL)
    {
        if (S_scache_debug_misses)
        {
            fprintf (debug_out,
                     "Block from memory (%8x) already in L2 cache.\n",
                     addr);
        }
    }
    else
    {
        /* Write data into cache */

        /* Get LRU block for this address */
        block = S_cache_find_LRU (cache, addr);

        /* Get block data (flags) */
        data = (L2_Data *) block->data;

	/* If doing conflict stats, add address to conflict cache  */
	if (S_scache_measure_conflict_stats)
	{
	    S_add_to_scache_conflict_cache (S_L2, addr);
	}

        replace_L2_block (S_L2, block, addr, bus_stats);

        /* Make block most recently used in cache line */
        S_cache_make_MRU (cache, block);

        /* Search write buffer for stores into this block
         * Remove them and mark block dirty if any found
         * and if WRITE_THROUGH flag not set.
         */
        write_buffer_to_L2_block(S_L2,block,data,start_addr_mask);
    }

    /* All pending misses (except first one (handled above))
     * on this data will be ready next cycle.
     */
    S_L2->addr_loaded_last_cycle = bus_addr;
}

void S_sim_L2_first_half_cycle (Pnode *pnode)
{
    L2_Request *request, *next_request;
    L2_Stats *L2_stats;
    Mentry *entry, *next_entry;
    Mentry *read_request, *write_request, *prefetch_request, *overlaps;
    Scblock *block, *block2;
    L2_Data *data;
    unsigned request_conflict_mask, request_addr, last_addr, addr2;
    unsigned start_addr_mask, addr_mask;
    int type = 0, allocate_this, load_hit, store_hit;
    int store_combined, store_forwarded, request_combined;
    int dest, size;
    L2_Cache *S_L2;
    Scache *cache;

    /* Get pointers for ease of use */
    S_L2 = pnode->scache;
    cache = S_L2->cache;
    S_L2->L2_stats = pnode->stats->L2_cache;

    /* Is the bus available to L2? */
    if (S_L2_bus.avail || S_L2_bus.L2_only)
    {
	/* Yes, If there a read_result ready to go out, then
	 * send it back, otherwise do nothing.
	 */
	request = S_L2->pending_head;
	
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

	      default:
		S_punt ("S_sim_L2_first_half_cycle: unknown type %i",
			request->type);
	    }

	    /* Result ready to go back and bus available, send it */
	    S_sim_L2_bus_transaction (type, L2, request->src,
				   request->addr, request->size, 0,
				   request->stats);

	    /* Request completed, remove from head of queue */
	    S_L2->pending_head = request->next;
	    if (S_L2->pending_head == NULL)
		S_L2->pending_tail = NULL;

	    /* Free request node */
	    L_free (L2_Request_pool, request);
	}
    }

    L2_stats = S_L2->L2_stats;

    /* Do perfect cache simulation */
    if (S_scache_model == L2_MODEL_PERFECT)
    {
        /* Process everything on request queue as a hit */
        for (request = S_L2->new_pending_head; request != NULL; 
				request = next_request)
        {
            /* Get next entry before we mess with current entry */
            next_request = request->next;

	    /* Update pending queue and new_pending_head */
	    S_L2->new_pending_head = next_request;
	    if (S_L2->new_pending_head == NULL)
		S_L2->new_pending_tail = NULL;

	    /* Put this request at end of pending list */
	    if (request->type != WRITE_REQUEST)
	    {
	      if (S_L2->pending_tail == NULL)
		S_L2->pending_head = request;
	      else
		S_L2->pending_tail->next = request;
	      request->next = NULL;
	      S_L2->pending_tail = request;
	    }

            /* Attempt to handle load */
            switch (request->type)
            {
              case READ_REQUEST:
                /* Set complete time and put in pending queue */
                request->complete_time = S_sim_cycle + S_scache_latency;

                /* Updates stats */
                L2_stats->read_hits++;

                break;

              case WRITE_REQUEST:
                /* Update stats */
                L2_stats->write_hits++;
	        L_free (L2_Request_pool, request);

                break;

              case PREFETCH_REQUEST:
                /* Set complete time and put in pending queue */
                request->complete_time = S_sim_cycle + S_scache_latency;

                /* Update stats */
                L2_stats->prefetch_hits++;
                break;

              default:
                S_punt ("S_sim_L2_first_half_cycle: unknown request type %i.",
                        request->type);
                break;
            }
        }
        return;
    }

    /*
     * Bus manager simulation.
     */

    /* Get start_addr_mask for ease of use */
    start_addr_mask = S_L2->cache->start_addr_mask;


    /* If bus idle, make an request if possible */
    if (S_bus.avail)
    {
        /* If there is a pending miss request, make it */
        read_request = S_L2->miss_request_buffer->head;
        prefetch_request = S_L2->prefetch_request_buffer->head;
        write_request = S_L2->write_buffer->head;


        if (read_request != NULL)
        {
            /* Request block of data for specified cache */
            if ((read_request->playdoh_flags & PLAYDOH_TCHS_C2)
                        || !(read_request->playdoh_flags &
                                                PLAYDOH_TCHS_ALL))
                S_sim_bus_transaction (READ_REQUEST, L2, MEMORY,
                                   read_request->addr &
                                   start_addr_mask,
                                   S_scache_block_size,
                                   read_request->stats);
            else
                S_sim_bus_transaction (READ_REQUEST, L2, MEMORY,
                                   read_request->addr &
                                   S_C3_addr_mask,
                                   S_C3_req_size,
                                   read_request->stats);

            /* Made request, get rid of it */
            M_dequeue (read_request);
        }

        /* Otherwise, if there is something in prefetch buffer, make request*/
        else if ((prefetch_request != NULL) && !S_scache_higher_store_priority)
        {
            /* Request block of data */
            S_sim_bus_transaction (PREFETCH_REQUEST, L2, MEMORY,
                                   prefetch_request->addr &
                                   start_addr_mask,
                                   S_scache_block_size,
                                   prefetch_request->stats);

            /* Made request, move to end of pending prefetches buffer */
            M_move_entry_before (S_L2->pending_prefetches, prefetch_request,
                                 NULL);

            /* Update stats */
            L2_stats->prefetch_requests_sent++;
        }

        /* Otherwise, if there is something in the store buffer, write it */
        else if (write_request != NULL)
        {
            /* Send data in write buffer entry */
            S_sim_bus_transaction (WRITE_REQUEST, L2, MEMORY,
                                   write_request->addr,
                                   write_request->size,
                                   write_request->stats);

            /* Finished with store request, dequeue it */
            M_dequeue (write_request);
        }

        /* Otherwise, if there is something in prefetch buffer, make request*/
        else if ((prefetch_request != NULL) && S_scache_higher_store_priority)
        {
            /* Request block of data */
            S_sim_bus_transaction (PREFETCH_REQUEST, L2, MEMORY,
                                   prefetch_request->addr &
                                   start_addr_mask,
                                   S_scache_block_size,
                                   prefetch_request->stats);

            /* Made request, move to end of pending prefetches buffer */
            M_move_entry_before (S_L2->pending_prefetches, prefetch_request,
                                 NULL);

            /* Update stats */
            L2_stats->prefetch_requests_sent++;
        }

    }

    /* If a block was loaded last cycle, scan the pending misses from the
     * primary dcache and icache to see if any of them can be serviced now.
     */
    last_addr = S_L2->addr_loaded_last_cycle;
    if (last_addr != -1)
    {
        if ((S_current_L2_dest & PLAYDOH_TCHS_ALL) == PLAYDOH_TCHS_C3)
            addr_mask = S_C3_addr_mask;
        else
            addr_mask = start_addr_mask;

        for (entry = S_L2->miss_buffer->head; entry != NULL;
             entry = next_entry)
        {
            /* Get next entry before messing with current one */
            next_entry = entry->next_entry;

            if ((entry->addr & addr_mask) == last_addr &&
                (S_current_L2_size >= entry->size))
            {
		request = (L2_Request *)entry->request;
                /* Return if a read req, otherwise for stores we are finished */
                if (request->type == READ_REQUEST)
                {
                    /*
                     * Ready this cycle (or later if latency > 2)
                     */
                    request->complete_time = S_sim_cycle + S_scache_latency;

                    /* Remove from this queue, still in processors queue */
                    M_dequeue (entry);
                }
                else
                {
                    M_dequeue (entry);
                }
            }
        }

        /* Mark that we have checked buffer */
        S_L2->addr_loaded_last_cycle = -1;

        /* Mark that there is no current dest anymore (no data currently
         * being transferred.
         */
        S_current_L2_dest = 0;
        S_current_L2_size = 0;
    }

    /*
     * Return if L2 cache is blocked.
     *
     * This occures if:
     * 1) If the load miss bypass limit has been exceeded.
     * 2) If the store buffer limit has been exceeded.
     *
     * Use stats region of request causing block.
     */
    if (S_L2->miss_buffer->size > S_scache_miss_bypass_limit)
    {
        S_L2->miss_buffer->tail->stats->L2_cache->cycles_blocked_miss_buffer++;
        return;
    }
    if (S_L2->write_buffer->size > S_scache_write_buf_size)
    {
        S_L2->write_buffer->tail->stats->L2_cache->cycles_blocked_write_buffer++;
        return;
    }

    /* Process all of these L2 cache requests.  This may require exceeding
     * buffer size limits.  The L2 cache then block until the sizes stop
     * exceeding the limit.
     */
    for (request = S_L2->new_pending_head; request != NULL; 
				request = next_request)
    {
        /* Get next entry before we mess with current entry */
        next_request = request->next;

	/* Update new_pending_head */
	S_L2->new_pending_head = next_request;
	if (S_L2->new_pending_head == NULL)
		S_L2->new_pending_tail = NULL;

	/* Put this request at end of pending list */
	if (request->type != WRITE_REQUEST)
	{
	  if (S_L2->pending_tail == NULL)
	    S_L2->pending_head = request;
	  else
	    S_L2->pending_tail->next = request;
	  request->next = NULL;
	  S_L2->pending_tail = request;
	}

        request_addr = request->addr;
        request_conflict_mask = ~(request->size - 1);

        /* Get the stats structure for this sint */
        L2_stats = request->stats->L2_cache;

        overlaps = NULL;

        request_combined = 0;

        /* Attempt to handle load */
        switch (request->type)
        {
          case READ_REQUEST:

	    /**** For now ignore ICACHE requests:
	     ****     Treat them as hits and don't allocate in L2 cache.
	     ****/
	    if (request->src == ICACHE)
	    {
                /* Set complete time */
                request->complete_time = S_sim_cycle + S_scache_latency;

		break;
	    }

            /* Assume load not a hit */
            load_hit = 0;

            /* Assume data not forwarded from a store */
            store_forwarded = 0;

            block = S_cache_find_addr (S_L2->cache, request_addr);
    	    cache = S_L2->cache;

	    if (!block && S_L2_victim_cache)
	    {
            	block = S_cache_find_addr (L2_vcache, request_addr);
    	    	if (block)
		{
		    cache = L2_vcache;
		    L2_stats->vcache_hits++;
		}
	    }
	    else if (block)
		L2_stats->L2_hits++;

            dest = request->playdoh_flags & PLAYDOH_TCHS_ALL;

	    if (dest == PLAYDOH_TCHS_C3)
		size = S_C3_req_size;
	    else
	    {
		dest = PLAYDOH_TCHS_C2;
		size = S_scache_block_size;
	    }

	    request->playdoh_flags = dest;

	    if (S_secondary_cache)
	    {
              if (dest == PLAYDOH_TCHS_C3)
                L2_stats->load_dest_C3++;
              else
                L2_stats->load_dest_C2++;

              if (dest == PLAYDOH_TCHS_C3)
              {
                if (block)
                    L2_stats->load_C3_hit_C2++;
                else
                    L2_stats->load_C3_miss++;
              }
	      else
	      {
                if (block)
                    L2_stats->load_C2_hit_C2++;
                else
                    L2_stats->load_C2_miss++;
	      }
	    }

	    if (block) dest = PLAYDOH_TCHS_C2;

            /* If hit, make block MRU */
            if (block != NULL)
            {
		/* If hit in victim cache, move block to L2 cache */
		if (cache == L2_vcache && S_L2_victim_cache)
		{
		    if (S_L2_vcache_block_size != S_scache_block_size)
			S_punt("L2 Victim cache and L2 block sizes are different!\n");
		    
            	    block2 = S_cache_find_LRU (S_L2->cache, request_addr);
		    addr2 = block2->start_addr;
            	    S_cache_change_addr (S_L2->cache, block2, request_addr);
            	    S_cache_change_addr (L2_vcache, block, addr2);
		    cache = S_L2->cache;
		    block = block2;
		}

                /* Make most recently use */
                S_cache_make_MRU (cache, block);

                /* Mark as hit */
                load_hit = 1;

		data = (L2_Data *)block->data;

                /* If prefetch bit set, reset bit and and remove any
                 * prefetches in prefetch buffer that has the
                 * same index as the current access.
                 */
                cancel_L2_prefetch (S_L2, block);

		/* Update conflict stats if desired, do here so
		 * conflict cache has a chance.  The can also get
		 * hits due to forwarding, but do not model this
		 * for conflicts
		 */
		if (S_scache_measure_conflict_stats &&
					cache != L2_vcache)
		{
		    S_update_scache_conflict_stats (LOAD_OPC,
		                                    S_L2, L2_stats,
		                                    request_addr,
		                                    HIT);
		    S_add_to_scache_conflict_cache (S_L2,
		                                    request_addr);

		}
	    }

            /* Otherwise, see if can forward from write buffer */
            else
            {
                /* Search for most recent store that overlaps load */
                overlaps = find_overlapping_L2_entry (S_L2->write_buffer,
                                                   request_addr,
                                                   request_conflict_mask);

                /* If have overlapping store, forward if store's data
                 * size is at least as big as the load's (conflict_mask
                 * will be as least as small as the loads).
                 */
                if ((overlaps != NULL) &&
                    (~(overlaps->size - 1) <= request_conflict_mask))
                {
                    /* Can forward, update stats */
                    L2_stats->reads_forwarded++;

                    store_forwarded = 1;

                    /* Mark as hit */
                    load_hit = 1;
                }
            }

            /* Process hits */
            if (load_hit)
            {
                /* Set complete time */
                request->complete_time = S_sim_cycle + S_scache_latency;

                /* Updates stats */
                L2_stats->read_hits++;

                if (S_scache_debug_misses)
                {
                    if (store_forwarded)
                        fprintf (debug_out, "(forwarded) ");

                    fprintf (debug_out, "%d: L2 LD HIT %x\n",
                             S_sim_cycle,
                             request->addr);
                }
	    }

	    /* Process misses */
	    else
	    {
                /*
                 * Add a request to fetch the missed block if there
                 * no other pending load misses to this block.
                 *
                 * This will prevent issuing two requests for
                 * the same block (but it will still be counted as
                 * two pending misses).
                 */
                if (!S_find_outstanding_L2_request (S_L2, request))
                {
                    entry = M_enqueue (S_L2->miss_request_buffer, request->type,
			L2, MEMORY, request_addr, size, -1,
			S_sim_cycle, request->stats);
		    entry->playdoh_flags = dest;
		    entry->request = (void *) request;

                    /* Update stats for new request.
                     * Depending on model, a redundant request could
                     * be considered a hit (if blocking cache)
                     * or a redundant miss (if non-blocking cache).
                     * This is handled in else below.
                     */
                    L2_stats->read_misses++;

                    /* Update conflict stats if desired */
                    if (S_scache_measure_conflict_stats &&
                        (dest == PLAYDOH_TCHS_C2))
                    {
                        S_update_scache_conflict_stats (LOAD_OPC,
                                                S_L2, L2_stats,
                                                request_addr,
                                                MISS);
                        /* Add to conflict cache when data comes back */
                    }
                }

                else
		{
                    request_combined = 1;

                    /* For blocking models, these redundent misses
                     * are considered hits, since no extra blocking
                     * time is caused by them.
                     */
                    if (S_scache_model == L2_MODEL_BLOCKING)
                    {
                        L2_stats->read_hits++;
                    }
                    /* For non-blocking models, these redundent misses
                     * are considered misses.
                     */
                    else
                    {
			/* Update conflict stats if desired
			 * Only update stats for redundant misses since
			 * the conflict cache models a non-blocking cache.
			 */
			if (S_scache_measure_conflict_stats &&
			        (dest == PLAYDOH_TCHS_C2))
			{
			    S_update_scache_conflict_stats (LOAD_OPC,
			                            S_L2, L2_stats,
			                            request_addr,
			                            MISS);
			}

                        /* Updates stats */
                        L2_stats->read_misses++;
                        L2_stats->redundant_read_misses++;
                    }
                }
                /* Put entry in pending miss queue */
                entry = M_enqueue (S_L2->miss_buffer, request->type,
			L2, MEMORY, request_addr, size, -1,
			S_sim_cycle, request->stats);
		entry->playdoh_flags = dest;
		entry->request = (void *) request;

                if (S_scache_debug_misses)
                {
                    if (request_combined)
                        fprintf (debug_out, "(request_combined) ");

                    fprintf (debug_out, "%d: L2 LD miss %x\n",
                             S_sim_cycle,
                             request_addr);
                }
            }
	    break;

          case PREFETCH_REQUEST:

	    /**** Ignore prefetches for now, assume they hit ****/
            /* Set complete time and put in pending queue */
            request->complete_time = S_sim_cycle + S_scache_latency;

	    #if 0
            /* If ignoring prefetches (for debugging purposes), ignore it */
            if (S_scache_ignore_prefetches)
            {
                /* Set complete time and put in pending queue */
                request->complete_time = S_sim_cycle + S_scache_latency;

                break;
            }

            /* Assume that we will not initiate prefetch */
            initiated_prefetch = 0;

            block = S_cache_find_addr (S_L2, request_addr);
    	    cache = S_L2->cache;

            /* If hit, done */
            if (block != NULL)
            {
                /* Update stats */
                L2_stats->prefetch_hits++;

                if (S_scache_debug_prefetch)
                {
                    fprintf (debug_out,
                             "%d: L2 PREFETCH hit addr %x\n",
                             S_sim_cycle, request_addr);
                }
            }

            /* Otherwise, start prefetch if this cache block is not already
             * being prefetched.
             */
            else
            {
                /* Get the LRU block for this address */
                block = S_cache_find_LRU (S_L2, request_addr);

                /* Get block's data */
                data = (L2_Data *) block->data;

                /* If block already has prefetch bit set,
                 * don't do prefetch.
                 */
                if (data->flags & PREFETCH_BIT)
                {
                    /* Update stats */
                    L2_stats->redundant_prefetches++;

                    if (S_scache_debug_prefetch)
                    {
                        fprintf (debug_out,
                                 "PREFETCH redundant addr %x\n",
                                 request_addr);
                    }

                }
                /* Otherwise, if prefetch buf full, don't do prefetch */
                else if (S_L2->prefetch_request_buffer->size >=
                         S_scache_prefetch_buf_size)
                {
                    /* Updates stats */
                    S_L2->prefetch_buf_full++;

                    if (S_scache_debug_prefetch)
                    {
                        fprintf (debug_out,
                                 "PREFETCH buf full addr %x\n",
                                 request_addr);
                    }
                }
                /* Otherwise, if have an outstanding request for this
                 * block, then don't do prefetch (treat as hit)
                 */
                else if (S_find_outstanding_L2_request (S_L2, request))
                {
                    /* Update stats */
                    L2_stats->prefetch_hits++;

                    if (S_scache_debug_prefetch)
                    {
                        fprintf (debug_out,
                                 "PREFETCH hit (being loaded) addr %x\n",
                                 request_addr);
                    }
                }
                /* Otherwise, do prefetch */
                else
                {
                    /* Set prefetch bit on block */
                    data->flags |= PREFETCH_BIT;

                    /* Update stats */
                    S_L2->prefetch_misses++;

                    /* Enqueue entry in pending prefetch queue */
                    entry = M_enqueue (S_L2->prefetch_request_buffer,
                		request->type, L2, MEMORY, request_addr, 
				request->size, -1, S_sim_cycle, request->stats);
		    entry->playdoh_flags = S_PLAYDOH_TCHS_C2;
		    entry->request = (void *) request;

                    /* Mark that we initialized prefetch */
                    initiated_prefetch = 1;

                    if (S_scache_debug_prefetch)
                    {
                        fprintf (debug_out,
                                 "%d: PREFETCH miss addr %x\n",
                                 S_sim_cycle, request_addr);
                    }

                }
            }
            /* If didn't initiate prefetch, done with prefetch, throw away */
            if (!initiated_prefetch)
            {
                /* Set complete time */
                request->complete_time = S_sim_cycle + S_scache_latency;

                S_dequeue (entry);
                S_free_sint (sint);
            }
	    #endif

            break;

          case WRITE_REQUEST:
            /* Assume store miss and not combined */
            store_hit = 0;
            store_combined = 0;

            block = S_cache_find_addr (cache, request_addr);
    	    cache = S_L2->cache;
	    if (!block && S_L2_victim_cache)
	    {
            	block = S_cache_find_addr (L2_vcache, request_addr);
    	    	if (block)
		{
		    cache = L2_vcache;
		}
	    }

            dest = request->playdoh_flags & PLAYDOH_TCHS_ALL;

	    if (S_prefetch_cache)
	    {
              if (dest == PLAYDOH_TCHS_C3 || !dest)
              {
                L2_stats->store_dest_C3++;

                if (block)
                    L2_stats->store_C3_hit_C2++;
                else
                    L2_stats->store_C3_miss++;
              }
              else
              {
                L2_stats->store_dest_C2++;

                if (block)
                    L2_stats->store_C2_hit_C2++;
                else
                    L2_stats->store_C2_miss++;
              }
            }

            if (!block && (dest == PLAYDOH_TCHS_C3 || !dest))
		dest = PLAYDOH_TCHS_C3;
	    else
		dest = PLAYDOH_TCHS_C2;

            /* If we have specified a target cache, do write allocation */
            if (S_scache_write_allocate || dest != PLAYDOH_TCHS_C3)
                allocate_this = 1;
            else allocate_this = 0;

            /* If hit, mark block as dirty and make MRU */
            if (block != NULL)
            {
		/* If hit in victim cache, move block to L2 cache */
		if (cache == L2_vcache && S_L2_victim_cache)
		{
		    if (S_L2_vcache_block_size != S_scache_block_size)
			S_punt("L2 Victim cache and L2 block sizes are different!\n");
		    
            	    block2 = S_cache_find_LRU (S_L2->cache, request_addr);
		    addr2 = block2->start_addr;
            	    S_cache_change_addr (S_L2->cache, block2, request_addr);
            	    S_cache_change_addr (L2_vcache, block, addr2);
		    cache = S_L2->cache;
		    block = block2;
		}

                /* Get scache block data */
                data = (L2_Data *) block->data;
                data->flags |= DIRTY_BLOCK;

		/* Update conflict stats if desired */
		if (S_scache_measure_conflict_stats)
		{
		    if (allocate_this)
		    {
		        S_update_scache_conflict_stats (STORE_OPC,
		                                        S_L2, L2_stats,
		                                        request_addr,
		                                        HIT);
		        S_add_to_scache_conflict_cache (S_L2,
		                                        request_addr);

		    }
		    /* If not write allocate, just make MRU */
		    else
		    {
		        S_make_scache_conflict_cache_MRU(S_L2,
		                                         request_addr);
		    }
		}
                /* Make most recently used block in cache line
                 * May want to make dependent on write_allocate policy
                 */
                S_cache_make_MRU (cache, block);

                /* Mark as store hit */
                store_hit = 1;

                /* If prefetch bit set, reset bit and remove any
                 * prefetches in prefetch buffer that has the
                 * same index as the current access.
                 */
                cancel_L2_prefetch (S_L2, block);
            }

            /* Otherwise, try to combine with existing store (if allowed)*/
            else if (S_scache_combining_write_buf)
            {
                /* Search for overlapping store */
                overlaps = find_overlapping_L2_entry (S_L2->write_buffer,
                                                   request_addr,
                                                   request_conflict_mask);

                /* If have overlapping store, combine if overlap's data
                 * size is at least as big as this store's(conflict_mask
                 * will be as least as small as this store's).
                 */
                if ((overlaps != NULL) &&
                    (~(overlaps->size - 1) <= request_conflict_mask))
                {
                    /* Can combine, possibly treat as hit */
                    store_combined = 1;
                    L2_stats->writes_combined++;
                    store_hit = 1;
                }
            }

            /* Process hits */
            if (store_hit)
            {
                /* Update stats */
                L2_stats->write_hits++;

                if (S_scache_debug_misses)
                {
                    if (store_combined)
                        fprintf (debug_out, "(combined) ");

                    fprintf (debug_out, "%d: ST HIT %x\n",
                             S_sim_cycle, request_addr);
                }
		L_free (L2_Request_pool, request);
            }

            /* Process misses */
            else
            {
                /* Move to write buffer for writing */
                entry = M_enqueue (S_L2->write_buffer,
                		request->type, L2, MEMORY, request_addr, 
				request->size, -1, S_sim_cycle, request->stats);
		entry->playdoh_flags = dest;

                /*
                 * If no write allocate protocol, was put in write buffer
                 * to be written to memory.
                 */
                if (!allocate_this)
                {
                    /* Update stats */
                    L2_stats->write_misses++;
	    	    L_free (L2_Request_pool, request);
                }

                /*
                 * If write allocate, put in pending miss queue and
                 * issue request if request has not already gone out.
                 *
                 * This will prevent issuing two requests for the
                 * same block.
                 */
                else
                {
		    /* Update conflict stats if desired and only if write
		     * allocate
		     */
		    if (S_scache_measure_conflict_stats &&
		                                dest & PLAYDOH_TCHS_C2)
		    {
			S_update_scache_conflict_stats (STORE_OPC,
			                                S_L2, L2_stats,
			                                request_addr,
			                                MISS);
			/* Add to conflict cache when comes back from memory*/
		    }

                    if (!S_find_outstanding_L2_request (S_L2, request))
                    {
                	entry = M_enqueue (S_L2->miss_request_buffer,
                		request->type, L2, MEMORY, request_addr, 
				S_scache_block_size, -1, S_sim_cycle, request->stats);
			entry->playdoh_flags = dest;
			entry->request = (void *) request;

                        /* Update stats for new request.
                         * Depending on model, a redundant request could
                         * be considered a hit (if blocking cache)
                         * or a redundant miss (if non-blocking cache).
                         * This is handled below in else.
                         */
                        L2_stats->write_misses++;

                        /* Clear any prefetch bits for this line, and clear
                         * any prefetch requests that have not gone to memory
                         * for this line.
                         */
                        block = S_cache_find_LRU (cache, request_addr);
                        cancel_L2_prefetch (S_L2, block);
                    }
                    else
                    {
                        if (S_scache_model == L2_MODEL_BLOCKING)
                        {
                            L2_stats->write_hits++;
                        }
                        else
                        {
                            L2_stats->write_misses++;
                        }
                        request_combined = 1;
                    }
                    /* Put entry in pending miss queue */
                    entry = M_enqueue (S_L2->miss_buffer,
                		request->type, L2, MEMORY, request_addr, 
				S_scache_block_size, -1, S_sim_cycle, request->stats);
		    entry->playdoh_flags = dest;
		    entry->request = (void *) request;
                }

                if (S_scache_debug_misses)
                {
                    if (request_combined)
                        fprintf (debug_out, "(request_combined) ");

                    fprintf (debug_out, "%d: ST miss %x\n",
                             S_sim_cycle, request_addr);
                }
            }
            break;

        }  /* END switch (request->type) */
    }  /* END pending requests loop */
}

void S_sim_L2_second_half_cycle (Pnode *pnode)
{
    L2_Request *request;
    Dcache *dcache;
    L2_Cache *S_L2;
    L2_Stats *L2_stats;
    Mentry *entry;
    unsigned start_addr_mask;
    Scache *cache;

    S_L2 = pnode->scache;
    dcache = pnode->dcache;
    L2_stats = S_L2->L2_stats;
    cache = S_L2->cache;

    /*
     * Is another request coming in and are we at the
     * last cycle of the request?
     */
    if ((S_L2_bus.dest == L2) && (S_L2_bus.segment == S_L2_bus.length))
    {
	/* Process according to request type */
	switch (S_L2_bus.type)
	{
	    /* 
	     * Hold request for S_scache_latency cycles then
	     * start arbitrating for the bus to return result.
	     */
	  case WRITE_REQUEST:
	  case READ_REQUEST:
	  case PREFETCH_REQUEST:

	    /* Allocate request structure */
	    if (L2_Request_pool == NULL)
	    {
		L2_Request_pool = L_create_alloc_pool ("L2_Request",
							sizeof (L2_Request),
							1);
	    }
	    request = (L2_Request *) L_alloc (L2_Request_pool);

	    /* Fill in request info */
	    request->type = S_L2_bus.type;
	    request->src = S_L2_bus.src;
	    request->addr = S_L2_bus.addr;
	    request->size = S_L2_bus.size;
	    request->playdoh_flags = S_L2_bus.playdoh_flags;
	    request->stats = S_L2_bus.stats;

	    /* Set completion type based on memory latency */
	    request->complete_time = (unsigned) -1;

	    /* Put at end of new pending queue (assume full pipelining
	     * and unlimited buffer space)
	     */
	    if (S_L2->new_pending_tail == NULL)
		S_L2->new_pending_head = request;
	    else
		S_L2->new_pending_tail->next = request;
	    request->next = NULL;
	    S_L2->new_pending_tail = request;
	    break;

	  default:
	    S_punt ("S_sim_L2: unknown request type '%i'.",
		    S_L2_bus.type);
	}
    }

    /* Do nothing else for perfect scache */
    if (S_scache_model == L2_MODEL_PERFECT)
        return;

    /* Get stats structure */
    L2_stats = S_L2->L2_stats;

    /*
     * At end of cycle, if we are the destination of a read result,
     * load block into memory.
     */
    if ((S_bus.dest == L2) && (S_bus.type == READ_RESULT))
    {
        /* Determine if this is the cycle that load's waiting for
         * this block are ready.
         *
         * If streaming support it on, the pending load that initiated
         * the block load is forwarded.  All others have to wait until
         * the load it totallly completed.
         *
         * Otherwise, they are ready the last cycle data comes back.
         * Need to always sweep at end of transaction because requests
         * in the interim could be queued up.
         */

        if ((S_L2_streaming_support && (S_bus.segment == 1)) ||
            ((!S_L2_streaming_support) && (S_bus.segment == S_bus.length)))
        {
            /* The first pending load better be for this block ! */
            entry = S_L2->miss_buffer->head;

            if (entry == NULL) {
                S_punt("No pending load at cycle %d for L2 request of %x: miss queue empty",
                        S_sim_cycle,S_bus.addr);
	    }

            /* Save cache destination */
            S_current_L2_dest |= entry->playdoh_flags;

            if (S_current_L2_dest & PLAYDOH_TCHS_C2)
            {
                start_addr_mask = cache->start_addr_mask;
                S_current_L2_size = S_scache_block_size;
            }
            else
            {
                start_addr_mask = S_C3_addr_mask;
                S_current_L2_size = S_C3_req_size;
            }

            /* Sanity check */
            if ((entry->addr & start_addr_mask) != S_bus.addr)
            {
                S_punt("No pending load at cycle %d for L2 request of %x: expecting %x",
                        S_sim_cycle,S_bus.addr,entry->addr);
            }
            else if (S_scache_debug_misses)
                fprintf(debug_out,"Bus addr matches miss request addr %x\n",
                        entry->addr);

            if (entry->type == READ_REQUEST)
            {
                /* Ready next cycle */
		request = (L2_Request *)entry->request;
                request->complete_time = S_sim_cycle;

                /* Remove from this queue, still in pending queue */
                M_dequeue (entry);
            }

            /* Is a store, done with it (this happens with write allocate) */
            else
            {
                M_dequeue (entry);
            }
        }

        /* Even if no streaming support, must set S_current_L2_dest */
        else if (!S_L2_streaming_support && (S_bus.segment == 1))
        {
            /* The first pending load better be for this block ! */
            entry = S_L2->miss_buffer->head;

            S_current_L2_dest |= entry->playdoh_flags;

            if (S_current_L2_dest & PLAYDOH_TCHS_C2)
            {
                start_addr_mask = cache->start_addr_mask;
                S_current_L2_size = S_scache_block_size;
            }
            else
            {
                start_addr_mask = S_C3_addr_mask;
                S_current_L2_size = S_C3_req_size;
            }

            if (entry == NULL) {
                S_punt("No pending load at cycle %d for L2 request of %x: miss queue empty",
                        S_sim_cycle,S_bus.addr);
	    }
            else if ((entry->addr & start_addr_mask) != S_bus.addr) {
                S_punt("No pending load at cycle %d for L2 request of %x: expecting %x",
                        S_sim_cycle,S_bus.addr,entry->addr);
	    }
        }

        /* If finished loading block, add to cache,
         * and can process all pending loads for
         * this block next cycle (do now, just delay ready time 1 cycle)
         */
        if (S_bus.segment == S_bus.length)
        {
            /*
             * Move block of data into cache if at last cycle of transfer
             */

            if (S_current_L2_dest & PLAYDOH_TCHS_C2)
            {
                L2_bus_data_to_cache(S_L2,S_bus.addr,S_bus.addr,
                        S_bus.stats);
            }
            else if (S_current_L2_dest & PLAYDOH_TCHS_C3)
            {
                /* All pending misses (except first one (handled above))
                 * on this data will be ready next cycle.
                 * Still need to set this in case any other misses with
                 * target of C3 covered by this miss.
                 */
                S_L2->addr_loaded_last_cycle = S_bus.addr;
            }
	    else
            {
		S_punt("S_sim_L2_second_half_cycle: dest of returned data not C2 or C3: %d",
							S_current_L2_dest);
            }
        }
    }

    else if ((S_bus.dest == L2) && (S_bus.type == PREFETCH_RESULT))
    {
    	/**** Ignoring prefetches for now ****/
	S_punt("S_sim_L2_second_half_cycle: Bus returned prefetch result but these are not yet handled");
    }

    /*
     * Mark L2 as busy if:
     * 1) If the load miss bypass limit has been exceeded.
     * 2) If the store buffer limit has been exceeded.
     * 3) If blocking cache, and line being loaded from memory and
     *    it will not finish this cycle.
     *
     */
    if ((S_L2->miss_buffer->size > S_scache_miss_bypass_limit) ||
        (S_L2->write_buffer->size > S_scache_write_buf_size) ||
        ((S_scache_model == L2_MODEL_BLOCKING) && (S_bus.dest == L2) &&
         (S_bus.segment < S_bus.length)) ||
        (S_scache_block_until_store_completed && S_L2->write_buffer->size))
    {
        dcache->L2_busy = 1;
        if (S_scache_debug_misses)
            fprintf (debug_out, "%d: L2 cache BLOCKED this cycle\n",
                        S_sim_cycle);
    }
    else
    {
        S_scache_block_until_store_completed = 0;
        if (S_scache_debug_misses && dcache->L2_busy)
            fprintf (debug_out, "%d: L2 cache unblocked this cycle\n",
                        S_sim_cycle);
        dcache->L2_busy = 0;
    }

}

