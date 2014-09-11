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
 *      File:   s_dcache.c
 *      Author: Teresa Johnson and John Gyllenhaal
 *      Creation Date:  1993
 *      Copyright (c) 1993 Teresa Johnson, John Gyllenhaal, Wen-mei Hwu and
 *			   The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1993 Teresa Johnson, John Gyllenhaal, Wen-mei Hwu\n\
 and The Board of Trustees of the University of Illinois. \n\
 All rights reserved.\n";
#endif

#include "s_main.h"

#define PLAYDOH_SCHS_ALL 0x0000000F
#define PLAYDOH_TCHS_ALL 0x000000F0

/* Define macro functions below */

/* reassign playdoh flags */
#define set_playdoh_flags(sint,flags) \
	sint->playdoh_flags = flags;

/* returns the target cache specifier or C1 if none specified */
#define target_cache(sint) \
	(sint->playdoh_flags & PLAYDOH_TCHS_ALL) ? \
		(sint->playdoh_flags & PLAYDOH_TCHS_ALL) : PLAYDOH_TCHS_C1;

/* simulate a bus transaction, calling the appropriate bus routine depending
 * on whether we have an L2 cache or not.
 */
#define BUS_TRANSACTION(type,source,dest,addr,size,flags,stats) \
	if (S_secondary_cache) \
	    S_sim_L2_bus_transaction(type,source,dest,addr,size,flags,stats); \
	else \
	    S_sim_bus_transaction(type,source,dest,addr,size,stats)

/* Depending on whether we have a secondary cache, access the L2_Bus or Bus struct */
#define BUS(field) \
	(S_secondary_cache ? S_L2_bus.field : S_bus.field)

static void dcache_TLB_lookup (Dcache *dcache, Dcache_Stats *dstats, 
			       int addr, int flags, Sint *sint);

L_Alloc_Pool	*Dcache_pool = NULL;
L_Alloc_Pool	*Dcache_Stats_pool = NULL;


int S_dcache_lower_mem_copy_priority = 0;
int S_dcache_higher_store_priority = 0;
int S_block_until_store_completed = 0;
int S_current_dest;
int S_current_size;
int S_bypass_addr_mask;
int S_bypass_req_size;

int S_dcache_TLB1_size = 32;
int S_dcache_TLB1_assoc = 0;	/* 0 is full-assoc */
int S_dcache_TLB2_size = 256;
int S_dcache_TLB2_assoc = 0;	/* 0 is full-assoc */
int S_dcache_page_table_size = 4096;
int S_print_TLB_contents = 0;
int S_debug_page_faults = 0;

Scache *vcache;
int S_victim_cache = 0;
int S_vcache_size = 32*64;
int S_vcache_block_size = 32;

int memory_latency;
int bus_bandwidth;
int S_next_level;

void S_read_parm_dcache (Parm_Parse_Info *ppi)
{
    L_read_parm_s (ppi, "dcache_model", &S_dcache_model_name);
    L_read_parm_i (ppi, "dcache_size", &S_dcache_size);
    L_read_parm_i (ppi, "dcache_block_size", &S_dcache_block_size);
    L_read_parm_i (ppi, "dcache_assoc", &S_dcache_assoc);
    L_read_parm_b (ppi, "dcache_debug_misses", &S_dcache_debug_misses);
    L_read_parm_b (ppi, "dcache_lower_mem_copy_priority",
		   &S_dcache_lower_mem_copy_priority);
    L_read_parm_b (ppi, "dcache_higher_store_priority",
		   &S_dcache_higher_store_priority);
    L_read_parm_b (ppi, "dcache_measure_conflict_stats",
		   &S_dcache_measure_conflict_stats);
    L_read_parm_b (ppi, "dcache_combining_write_buf", 
		   &S_dcache_combining_write_buf); 
    L_read_parm_b (ppi, "dcache_write_allocate", &S_dcache_write_allocate); 
    L_read_parm_i (ppi, "dcache_miss_bypass_limit", 
		   &S_dcache_miss_bypass_limit); 
    L_read_parm_i (ppi, "dcache_prefetch_buf_size",
		   &S_dcache_prefetch_buf_size);
    L_read_parm_b (ppi, "dcache_debug_prefetch", &S_dcache_debug_prefetch);
    L_read_parm_b (ppi, "dcache_ignore_prefetch_bit",
		   &S_dcache_ignore_prefetch_bit);
    L_read_parm_b (ppi, "dcache_ignore_prefetches",
		   &S_dcache_ignore_prefetches);
    L_read_parm_i (ppi, "dcache_write_buf_size", &S_dcache_write_buf_size);
    L_read_parm_i (ppi, "mem_copy_version", &S_mem_copy_version);
    L_read_parm_b (ppi, "dcache_debug_mem_copy", &S_dcache_debug_mem_copy) ;

    L_read_parm_i (ppi, "dcache_TLB1_size", &S_dcache_TLB1_size);
    L_read_parm_i (ppi, "dcache_TLB1_assoc", &S_dcache_TLB1_assoc);
    L_read_parm_i (ppi, "dcache_TLB2_size", &S_dcache_TLB2_size);
    L_read_parm_i (ppi, "dcache_TLB2_assoc", &S_dcache_TLB2_assoc);
    L_read_parm_i (ppi, "dcache_page_table_size", & S_dcache_page_table_size);

    L_read_parm_b (ppi, "?print_TLB_contents", &S_print_TLB_contents);
    L_read_parm_b (ppi, "?debug_page_faults", &S_debug_page_faults);

    L_read_parm_b (ppi, "prefetch_cache", &S_prefetch_cache);
    L_read_parm_i (ppi, "pcache_size", &S_pcache_size);
    L_read_parm_i (ppi, "pcache_block_size", &S_pcache_block_size);
    L_read_parm_i (ppi, "pcache_assoc", &S_pcache_assoc);

    L_read_parm_b (ppi, "victim_cache", &S_victim_cache);
    L_read_parm_i (ppi, "vcache_size", &S_vcache_size);
    L_read_parm_i (ppi, "vcache_block_size", &S_vcache_block_size);
}

void S_print_configuration_vcache (FILE *out)
{
    fprintf (out, "# VICTIM CACHE CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12u vcache size.\n", S_vcache_size);
    fprintf (out, "%12u vcache block size.\n", S_vcache_block_size);
    fprintf (out, "%12u vcache assoc.\n", 0);
    fprintf (out,"\n");
}

void S_print_configuration_dcache (FILE *out)
{
    fprintf (out, "# DCACHE CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s dcache model.\n", S_dcache_model_name);
    if (S_dcache_model == DCACHE_MODEL_NON_BLOCKING)
	fprintf (out, "%12u miss bypass limit.\n", S_dcache_miss_bypass_limit);
    if (S_dcache_model != DCACHE_MODEL_PERFECT)
    {
	fprintf (out, "%12u dcache size.\n", S_dcache_size);
	fprintf (out, "%12u dcache block size.\n", S_dcache_block_size);
	fprintf (out, "%12u dcache assoc.\n", S_dcache_assoc);
	if (S_dcache_measure_conflict_stats)
	    fprintf (out, "%12s ", "Do");
	else
	    fprintf (out, "%12s ", "Do not");
	fprintf (out, "measure conflict stats (in dcache).\n");
	fprintf (out, "%12u dcache read block latency (calculated).\n", 
		 S_dcache_read_block_latency);
	fprintf (out, "%12u dcache write block latency (calculated).\n",
		 S_dcache_write_block_latency);
	fprintf (out, "%12u dcache write thru latency (calculated).\n",
		 S_dcache_write_thru_latency);
	fprintf (out, "%12u dcache write buf size.\n",
		 S_dcache_write_buf_size);
	if (S_dcache_combining_write_buf)
	    fprintf (out, "%12s ", "Yes");
	else
	    fprintf (out, "%12s ", "No");
	fprintf (out, "dcache combining write buf support.\n");
	if (S_dcache_write_allocate)
	    fprintf (out, "%12s ", "Yes");
	else
	    fprintf (out, "%12s ", "No");
	fprintf (out, "dcache write allocate protocol.\n");
	fprintf (out, "%12u dcache streaming benefit (in cycles).\n",
		 S_dcache_streaming_benefit);
	if (S_dcache_higher_store_priority)
	    fprintf (out, "%12s ", "Do");
	else
	    fprintf (out, "%12s ", "Do not");
	fprintf (out, "use higher write miss priority than prefetches.\n");
	fprintf (out, "%12u mem_copy version.\n", S_mem_copy_version);
	if (S_dcache_lower_mem_copy_priority)
	    fprintf (out, "%12s ", "Do");
	else
	    fprintf (out, "%12s ", "Do not");
	fprintf (out, "use lower mem_copy priority than write misses.\n");
	fprintf (out, "\n");
	fprintf (out, "%12u dcache first-level TLB size.\n", 
		 S_dcache_TLB1_size);
	fprintf (out, "%12u dcache first-level TLB assoc.\n", 
		 S_dcache_TLB1_assoc);
	fprintf (out, "%12u dcache second-level TLB size.\n", 
		 S_dcache_TLB2_size);
	fprintf (out, "%12u dcache second-level TLB assoc.\n", 
		 S_dcache_TLB2_assoc);
	fprintf (out, "%12u dcache page table size.\n",
		 S_dcache_page_table_size);
    }
    fprintf (out,"\n");

    if (S_victim_cache)
    	S_print_configuration_vcache (out);
}

void S_print_configuration_pcache (FILE *out)
{
    fprintf (out, "# PCACHE CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12u pcache size.\n", S_pcache_size);
    fprintf (out, "%12u pcache block size.\n", S_pcache_block_size);
    fprintf (out, "%12u pcache assoc.\n", S_pcache_assoc);
    if (S_dcache_model != DCACHE_MODEL_PERFECT)
    {
	fprintf (out, "%12u pcache read block latency (calculated)\n", 
		 S_pcache_read_block_latency);
	fprintf (out, "%12u pcache streaming benefit (in cycles).\n",
		 S_pcache_streaming_benefit);
	if (S_dcache_write_allocate)
	    fprintf (out, "%12s ", "Yes");
	else
	    fprintf (out, "%12s ", "No");
	fprintf (out, "pcache write allocate protocol.\n");
    }
    fprintf (out,"\n");
}

Dcache_Stats *S_create_stats_dcache()
{
    Dcache_Stats *stats;
    int i;

    /* Create dcache stats structure */
    if (Dcache_Stats_pool == NULL)
    {
	Dcache_Stats_pool = L_create_alloc_pool ("Dcache_Stats",
						 sizeof (Dcache_Stats),
						 1);
    }

    stats = (Dcache_Stats *) L_alloc (Dcache_Stats_pool);

    for (i=0; i <= MAX_OPC; i++)
    {
	STATS_ZERO(conflict_misses[i]);
	STATS_ZERO(anticonflict_hits[i]);
	STATS_ZERO(conflict_misses_off_path[i]);
	STATS_ZERO(anticonflict_hits_off_path[i]);
    }

    STATS_ZERO(read_hits);
    STATS_ZERO(vcache_hits);
    STATS_ZERO(write_hits);
    STATS_ZERO(reads_forwarded);
    STATS_ZERO(read_misses);
    /* HCH 10-18-99 */
    STATS_ZERO(speculative_read_misses);
    STATS_ZERO(redundant_read_misses);
    STATS_ZERO(redundant_read_misses_off_path);
    STATS_ZERO(write_misses);
    STATS_ZERO(writes_combined);
    STATS_ZERO(written_through_due_to_store);
    STATS_ZERO(written_through_due_to_load);
    STATS_ZERO(writes_to_C1_changed_to_V1);
    STATS_ZERO(writes_to_V1_changed_to_C1);
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
    STATS_ZERO(cycles_blocked_mem_copy_check);
    STATS_ZERO(cycles_blocked_store_completes);

    STATS_ZERO(cycles_L2_busy_when_needed);

    STATS_ZERO(load_source_C1);
    STATS_ZERO(load_source_V1);
    STATS_ZERO(load_dest_C1);
    STATS_ZERO(load_dest_C3);
    STATS_ZERO(load_dest_V1);
    STATS_ZERO(loads_became_prefetches);
    STATS_ZERO(store_dest_C1);
    STATS_ZERO(store_dest_C3);
    STATS_ZERO(store_dest_V1);

    STATS_ZERO(load_C1_hit_C1);
    STATS_ZERO(load_C1_hit_V1);
    STATS_ZERO(load_C1_miss);
    STATS_ZERO(load_V1_hit_C1);
    STATS_ZERO(load_V1_hit_V1);
    STATS_ZERO(load_V1_miss);
    STATS_ZERO(load_C3_hit_C1);
    STATS_ZERO(load_C3_hit_V1);
    STATS_ZERO(load_C3_miss);
    STATS_ZERO(store_C1_hit_C1);
    STATS_ZERO(store_C1_hit_V1);
    STATS_ZERO(store_C1_miss);
    STATS_ZERO(store_V1_hit_C1);
    STATS_ZERO(store_V1_hit_V1);
    STATS_ZERO(store_V1_miss);
    STATS_ZERO(store_C3_hit_C1);
    STATS_ZERO(store_C3_hit_V1);
    STATS_ZERO(store_C3_miss);

    STATS_ZERO(pcache_blocks_kicked_out);
    STATS_ZERO(pcache_dirty_blocks_kicked_out);
    STATS_ZERO(pcache_dirty_blocks_written_back);

    STATS_ZERO(overlapping_entries_with_different_dests);
    
    STATS_ZERO(num_mem_copies);
    STATS_ZERO(num_mem_copy_tags);
    STATS_ZERO(num_mem_copy_backs);
    STATS_ZERO(mem_copy_blocks);
    STATS_ZERO(mem_copy_tag_blocks);
    STATS_ZERO(blocks_mem_copies_kicked_out);
    STATS_ZERO(blocks_mem_copy_tags_kicked_out);
    STATS_ZERO(mem_copy_buffer_conflicts);
    STATS_ZERO(mem_copy_tag_buffer_conflicts);
    STATS_ZERO(mem_copy_back_blocks);
    STATS_ZERO(mem_copy_back_block_misses);
    STATS_ZERO(mem_copy_back_buffer_conflicts);
    STATS_ZERO(mem_copy_memory_requests);
    STATS_ZERO(mem_copy_cache_requests);
    STATS_ZERO(mem_copy_back_memory_requests);
    STATS_ZERO(mem_copy_back_cache_requests);

    STATS_ZERO(total_TLB_lookups);
    STATS_ZERO(total_TLB1_misses);
    STATS_ZERO(total_TLB2_misses);
    STATS_ZERO(total_pages_accessed);

    /* Return the initialized structure */
    return (stats);
}

void S_add_stats_dcache (Dcache_Stats *dest, 
			 Dcache_Stats *src1,
			 Dcache_Stats *src2)
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
    STATS_ADD(vcache_hits);
    STATS_ADD(write_hits);
    STATS_ADD(reads_forwarded);
    STATS_ADD(read_misses);
    /* HCH 10-18-99 */
    STATS_ADD(speculative_read_misses);
    STATS_ADD(redundant_read_misses);
    STATS_ADD(redundant_read_misses_off_path);
    STATS_ADD(write_misses);
    STATS_ADD(writes_combined);
    STATS_ADD(written_through);
    STATS_ADD(written_through_due_to_store);
    STATS_ADD(written_through_due_to_load);
    STATS_ADD(writes_to_C1_changed_to_V1);
    STATS_ADD(writes_to_V1_changed_to_C1);
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
    STATS_ADD(cycles_blocked_mem_copy_check);
    STATS_ADD(cycles_blocked_store_completes);

    STATS_ADD(cycles_L2_busy_when_needed);

    STATS_ADD(load_source_C1);
    STATS_ADD(load_source_V1);
    STATS_ADD(load_dest_C1);
    STATS_ADD(load_dest_C3);
    STATS_ADD(load_dest_V1);
    STATS_ADD(loads_became_prefetches);
    STATS_ADD(store_dest_C1);
    STATS_ADD(store_dest_C3);
    STATS_ADD(store_dest_V1);

    STATS_ADD(load_C1_hit_C1);
    STATS_ADD(load_C1_hit_V1);
    STATS_ADD(load_C1_miss);
    STATS_ADD(load_V1_hit_C1);
    STATS_ADD(load_V1_hit_V1);
    STATS_ADD(load_V1_miss);
    STATS_ADD(load_C3_hit_C1);
    STATS_ADD(load_C3_hit_V1);
    STATS_ADD(load_C3_miss);
    STATS_ADD(store_C1_hit_C1);
    STATS_ADD(store_C1_hit_V1);
    STATS_ADD(store_C1_miss);
    STATS_ADD(store_V1_hit_C1);
    STATS_ADD(store_V1_hit_V1);
    STATS_ADD(store_V1_miss);
    STATS_ADD(store_C3_hit_C1);
    STATS_ADD(store_C3_hit_V1);
    STATS_ADD(store_C3_miss);

    STATS_ADD(pcache_blocks_kicked_out);
    STATS_ADD(pcache_dirty_blocks_kicked_out);
    STATS_ADD(pcache_dirty_blocks_written_back);

    STATS_ADD(overlapping_entries_with_different_dests);
    
    STATS_ADD(num_mem_copies);
    STATS_ADD(num_mem_copy_tags);
    STATS_ADD(num_mem_copy_backs);
    STATS_ADD(mem_copy_blocks);
    STATS_ADD(mem_copy_tag_blocks);
    STATS_ADD(blocks_mem_copies_kicked_out);
    STATS_ADD(blocks_mem_copy_tags_kicked_out);
    STATS_ADD(mem_copy_buffer_conflicts);
    STATS_ADD(mem_copy_tag_buffer_conflicts);
    STATS_ADD(mem_copy_back_blocks);
    STATS_ADD(mem_copy_back_block_misses);
    STATS_ADD(mem_copy_back_buffer_conflicts);
    STATS_ADD(mem_copy_memory_requests);
    STATS_ADD(mem_copy_cache_requests);
    STATS_ADD(mem_copy_back_memory_requests);
    STATS_ADD(mem_copy_back_cache_requests);

    STATS_ADD(total_TLB_lookups);
    STATS_ADD(total_TLB1_misses);
    STATS_ADD(total_TLB2_misses);
    STATS_ADD(total_pages_accessed);
}
void S_print_stats_region_dcache (FILE *out, Stats *stats,
				  char *rname, Stats *total_stats)
{
    double dcache_total_read_hit_ratio;
    double dcache_on_path_read_hit_ratio, dcache_off_path_read_hit_ratio;
    double dcache_write_hit_ratio;
    int dcache_requests, dcache_requests_off_path, dcache_requests_on_path;
    int dcache_hits, dcache_hits_off_path, dcache_hits_on_path;
    double dcache_on_path_hit_ratio, dcache_off_path_hit_ratio;
    double dcache_hit_ratio;
    int prefetch_requests, prefetch_requests_off_path;
    Dcache_Stats *dstats;
    
    /* Setup Pstats calls */
    Pstats_out = out;
    Pstats_rname = rname;
    
    /* Get dstats for ease of use */
    dstats = stats->dcache;
    
    /* Want to distiguish parameter and result model lines */
    
    Pstats ("# DCACHE:");
    Pstats ("");

    
    Pstats ("%12s dcache simulation model.",
	    S_dcache_model_name);
    
    Pstats ("%12u dcache requests.",
	    dstats->read_hits + dstats->read_misses +
	    dstats->write_hits + dstats->write_misses +
	    dstats->prefetch_hits + dstats->prefetch_misses);
    
    Pstats ("%12u dcache on-path load requests.",
	    (dstats->read_hits + dstats->read_misses) - 
	    (dstats->read_hits_off_path + 
	     dstats->read_misses_off_path));
    
    Pstats ("%12u dcache off-path load requests.",
	    dstats->read_hits_off_path + 
	    dstats->read_misses_off_path);
    
    Pstats ("%12u dcache store requests (only on-path stores reach dcache).",
	    dstats->write_hits + dstats->write_misses);
    
    Pstats ("%12u dcache total read hits (on and off-path).",
	    dstats->read_hits);
    
    if (S_victim_cache)
    {
    	Pstats ("%12u dcache total read hits in victim cache (on and off-path).",
	    dstats->vcache_hits);
    }
    
    Pstats ("%12u dcache on-path read hits.",
	     dstats->read_hits - dstats->read_hits_off_path);
    
    Pstats ("%12u dcache on-path reads forwarded.",
	    dstats->reads_forwarded -
	    dstats->reads_forwarded_off_path);
    
    Pstats ("%12u dcache off-path read hits.",
	    dstats->read_hits_off_path);
    
    Pstats ("%12u dcache off-path reads forwarded.",
	    dstats->reads_forwarded_off_path);
    
    Pstats ("%12u dcache total read misses (on and off-path).",
	    dstats->read_misses);

    /* HCH 10-18-99 */
    Pstats ("%12u dcache total read misses caused by speculative loads.",
	    dstats->speculative_read_misses);

    Pstats ("%12u dcache on-path read misses.",
	    dstats->read_misses - dstats->read_misses_off_path);
    
    Pstats ("%12u dcache off-path read misses.",
	    dstats->read_misses_off_path);
    
    
    Pstats ("%12u dcache total redundant read misses (on and off-path).", 
	    dstats->redundant_read_misses);
    
    Pstats ("%12u dcache on-path redundant read misses.",
	    dstats->redundant_read_misses - 
	    dstats->redundant_read_misses_off_path);
    
    Pstats ("%12u dcache off-path redundant read misses.",
	    dstats->redundant_read_misses_off_path);
    
    Pstats ("%12u dcache write hits (only on-path stores reach dcache).",
	    dstats->write_hits);
    
    Pstats ("%12u dcache write misses (only on-path stores reach dcache).",
	    dstats->write_misses);
    
    Pstats ("%12u dcache writes combined (only on-path stores reach dcache).",
	    dstats->writes_combined);
    
    if (S_prefetch_cache)
    { 
    	Pstats ("%12u dcache writes written through.",
	    dstats->written_through);
    
    	Pstats ("%12u dcache writes caused by outstanding store.",
	    dstats->written_through_due_to_store);
    
    	Pstats ("%12u dcache writes caused by outstanding load or prefetch.",
	    dstats->written_through_due_to_load);
    
    	Pstats ("%12u dcache writes to prefetch cache changed to primary cache.",
	    dstats->writes_to_V1_changed_to_C1);
    
    	Pstats ("%12u dcache writes to primary cache changed to prefetch cache.",
	    dstats->writes_to_C1_changed_to_V1);
    }
    
    Pstats ("%12u dcache total blocks kicked out.",
	    dstats->blocks_kicked_out);
    
    Pstats ("%12u dcache clean blocks kicked out.",
	    dstats->blocks_kicked_out - dstats->dirty_blocks_kicked_out);
    
    Pstats ("%12u dcache dirty blocks kicked out.",
	    dstats->dirty_blocks_kicked_out);
    
    Pstats ("%12u dcache dirty blocks written back.",
	    dstats->dirty_blocks_written_back);
    

    Pstats ("%12u dcache cycles blocked due to full miss buffer.",
	    dstats->cycles_blocked_miss_buffer);
    
    Pstats ("%12u dcache cycles blocked due to full write buffer.",
	    dstats->cycles_blocked_write_buffer);
    
    Pstats ("%12u dcache cycles blocked due to mem_copy_check.",
	    dstats->cycles_blocked_mem_copy_check);
    
    Pstats ("%12u dcache cycles blocked waiting for a store to complete.",
	    dstats->cycles_blocked_store_completes);
    Pstats ("");

    if (S_secondary_cache)
    {
    	Pstats ("%12u dcache cycles L2 busy when needed.",
            dstats->cycles_L2_busy_when_needed);
    	Pstats ("");
    }

    if (S_prefetch_cache)
    { 
    	Pstats("%12u loads with primary cache as source.",
	    	dstats->load_source_C1);
    	Pstats("%12u loads with prefetch cache as source.",
	    	dstats->load_source_V1);
    	Pstats("%12u loads with primary cache as target.",
	    	dstats->load_dest_C1);
    	Pstats("%12u loads with main memory as target.",
	    	dstats->load_dest_C3);
    	Pstats("%12u loads with prefetch cache as target.",
	    	dstats->load_dest_V1);
    	Pstats("%12u loads to target cache became prefetches.",
	    	dstats->loads_became_prefetches);
    	Pstats("%12u stores with primary cache as target.",
	    	dstats->store_dest_C1);
    	Pstats("%12u stores with main memory as target.",
	    	dstats->store_dest_C3);
    	Pstats("%12u stores with prefetch cache as target.",
	    	dstats->store_dest_V1);
    	Pstats ("");

    	Pstats("%12u loads with primary cache as target hit in primary cache.",
	    	dstats->load_C1_hit_C1);
    	Pstats("%12u loads with primary cache as target hit in prefetch cache.",
	    	dstats->load_C1_hit_V1);
    	Pstats("%12u loads with primary cache as target missed.",
	    	dstats->load_C1_miss);
    	Pstats("%12u loads with prefetch cache as target hit in primary cache.",
	    	dstats->load_V1_hit_C1);
    	Pstats("%12u loads with prefetch cache as target hit in prefetch cache.",
	    	dstats->load_V1_hit_V1);
    	Pstats("%12u loads with prefetch cache as target missed.",
	    	dstats->load_V1_miss);
    	Pstats("%12u loads with main memory as target hit in primary cache.",
	    	dstats->load_C3_hit_C1);
    	Pstats("%12u loads with main memory as target hit in prefetch cache.",
	    	dstats->load_C3_hit_V1);
    	Pstats("%12u loads with main memory as target missed.",
	    	dstats->load_C3_miss);
    	Pstats("%12u stores with primary cache as target hit in primary cache.",
	    	dstats->store_C1_hit_C1);
    	Pstats("%12u stores with primary cache as target hit in prefetch cache.",
	    	dstats->store_C1_hit_V1);
    	Pstats("%12u stores with primary cache as target missed.",
	    	dstats->store_C1_miss);
    	Pstats("%12u stores with prefetch cache as target hit in primary cache.",
	    	dstats->store_V1_hit_C1);
    	Pstats("%12u stores with prefetch cache as target hit in prefetch cache.",
	    	dstats->store_V1_hit_V1);
    	Pstats("%12u stores with prefetch cache as target missed.",
	    	dstats->store_V1_miss);
    	Pstats("%12u stores with main memory as target hit in primary cache.",
	    	dstats->store_C3_hit_C1);
    	Pstats("%12u stores with main memory as target hit in prefetch cache.",
	    	dstats->store_C3_hit_V1);
    	Pstats("%12u stores with main memory as target missed.",
	    	dstats->store_C3_miss);
    	Pstats ("");
   
    	Pstats ("# PCACHE:");
    	Pstats ("");
    	Pstats ("%12u pcache total blocks kicked out.",
	    dstats->pcache_blocks_kicked_out);
    
    	Pstats ("%12u pcache clean blocks kicked out.",
	    dstats->pcache_blocks_kicked_out - 
			dstats->pcache_dirty_blocks_kicked_out);
    
    	Pstats ("%12u pcache dirty blocks kicked out.",
	    dstats->pcache_dirty_blocks_kicked_out);
    
    	Pstats ("%12u pcache dirty blocks written back.",
	    dstats->pcache_dirty_blocks_written_back);
    	Pstats ("");
    
    	Pstats ("%12u overlapping request entries with different dests.",
	    dstats->overlapping_entries_with_different_dests);
    	Pstats ("");
    }
    
    
    prefetch_requests = dstats->prefetch_hits +
	dstats->redundant_prefetches +
	    dstats->prefetch_buf_full +
		dstats->prefetch_misses;
    
    prefetch_requests_off_path = dstats->prefetch_hits_off_path +
	dstats->redundant_prefetches_off_path +
	    dstats->prefetch_buf_full_off_path +
		dstats->prefetch_misses_off_path;
    
    
    Pstats ("# DCACHE PREFETCHES:");
    Pstats ("");

    /* Dont print stats if ignoring prefetches */
    if (S_dcache_ignore_prefetches)
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
	
	Pstats ("%12u dcache total prefetch requests.",
		prefetch_requests);
	
	Pstats ("%12u dcache total prefetch misses.",
		dstats->prefetch_misses);
	
	Pstats ("%12u dcache total prefetch requests send to memory.",
		dstats->prefetch_requests_sent);
	
	Pstats ("%12u dcache total prefetches cancelled before memory request.",
		dstats->prefetch_misses -
		dstats->prefetch_requests_sent);
	
	Pstats ("%12u dcache total prefetches cancelled during memory request.",
		dstats->prefetch_requests_sent - 
		dstats->prefetches_completed);
	
	Pstats ("%12u dcache total prefetches completed.",
		dstats->prefetches_completed);
	
	Pstats ("%12u dcache total loads used a outstanding prefetch request.",
		dstats->loads_used_prefetch_request);
	
	
	Pstats ("%12u dcache on-path prefetch requests.",
		prefetch_requests - prefetch_requests_off_path);
	
	Pstats ("%12u dcache on-path prefetch hits.",
		dstats->prefetch_hits - dstats->prefetch_hits_off_path);
	
	Pstats ("%12u dcache on-path redundant prefetches.",
		dstats->redundant_prefetches - 
		dstats->redundant_prefetches_off_path);
	
	Pstats ("%12u dcache on-path prefetches discarded because buffer full.",
		dstats->prefetch_buf_full -
		dstats->prefetch_buf_full_off_path);
	
	Pstats ("%12u dcache on-path prefetch misses.",
		dstats->prefetch_misses -
		dstats->prefetch_misses_off_path);
	
	
	Pstats ("%12u dcache off-path prefetch requests.",
		prefetch_requests_off_path);
	
	Pstats ("%12u dcache off-path prefetch hits.",
		dstats->prefetch_hits_off_path);
	
	Pstats ("%12u dcache off-path redundant prefetches.",
		dstats->redundant_prefetches_off_path);
	
	Pstats ("%12u dcache off-path prefetches discarded because buffer full.",
		dstats->prefetch_buf_full_off_path);
	
	Pstats ("%12u dcache off-path prefetch misses.",
		dstats->prefetch_misses_off_path);
    }
    Pstats ("");
    
    
    /* 
     * MEM_COPY stats
     */
    
    Pstats ("# DCACHE MEM_COPY:");
    Pstats ("");

    /* Don't print out all the mem_copy stats if there is no mem_copies
     * in the program.
     */
    if (!S_has_mem_copy_directives)
    {
	Pstats ("%12s dcache mem_copy directives.", "No");
    }
    else 
    {
	Pstats ("%12u dcache mem_copy requests (only on path allowed).",
		dstats->num_mem_copies);
	
	Pstats ("%12u dcache mem_copy_tag requests (only on path allowed).",
		dstats->num_mem_copy_tags);
	
	Pstats ("%12u dcache mem_copy_back requests (only on path allowed).",
		dstats->num_mem_copy_backs);
	
	Pstats ("%12u dcache mem_copy blocks loaded.",
		dstats->mem_copy_blocks);
	
	Pstats ("%12u dcache blocks that mem_copies kicked out.",
		dstats->blocks_mem_copies_kicked_out);
	
	Pstats ("%12u dcache buffer conflicts caused by mem_copies.",
		dstats->mem_copy_buffer_conflicts);
	
	Pstats ("%12u dcache mem_copy_tag blocks loaded.",
		dstats->mem_copy_tag_blocks);
	
	Pstats ("%12u dcache blocks that mem_copy_tags kicked out.",
		dstats->blocks_mem_copy_tags_kicked_out);
	
	Pstats ("%12u dcache buffer conflicts caused by mem_copy_tags.",
		dstats->mem_copy_tag_buffer_conflicts);
	
	Pstats ("%12u dcache mem_copy_back blocks written.",
		dstats->mem_copy_back_blocks);
	
	Pstats ("%12u dcache mem_copy_back block misses.",
		dstats->mem_copy_back_block_misses);
	
	Pstats ("%12u dcache buffer conflicts caused by mem_copy_backs.",
		dstats->mem_copy_back_buffer_conflicts);
	
	Pstats ("%12u dcache mem_copy elements found in memory.",
		dstats->mem_copy_memory_requests);
	
	Pstats ("%12u dcache mem_copy elements found in dcache.",
		dstats->mem_copy_cache_requests);
	
	Pstats ("%12u dcache mem_copy_back elements found in memory.",
		dstats->mem_copy_back_memory_requests);
	
	Pstats ("%12u dcache mem_copy_back elements found in dcache.",
		dstats->mem_copy_back_cache_requests);
    }
    Pstats ("");

    Pstats ("# DCACHE TLB STATS:");
    Pstats ("");
    Pstats ("%12u total dcache TLB lookups.",
	    dstats->total_TLB_lookups);
    Pstats ("%12u total dcache first-level TLB misses.",
	    dstats->total_TLB1_misses);
    Pstats ("%12u total dcache second-level TLB misses.",
	    dstats->total_TLB2_misses);
    if (total_stats->dcache->total_pages_accessed > S_dcache_page_table_size)
	Pstats ("%12s total dcache pages accessed.", "(Overflow)");
    else
	Pstats ("%12u total dcache pages accessed.", 
		dstats->total_pages_accessed);
    Pstats ("");
    
    if (S_dcache_measure_conflict_stats)
    {
	Pstats ("# DCACHE CONFLICTS:");
	Pstats ("");
	
	Pstats ("%12u dcache total conflict read misses.",
		dstats->conflict_misses[LOAD_OPC]);
	
	Pstats ("%12u dcache on-path conflict read misses.",
		dstats->conflict_misses[LOAD_OPC] - 
		dstats->conflict_misses_off_path[LOAD_OPC]);
	
	Pstats ("%12u dcache off-path conflict read misses.",
		dstats->conflict_misses_off_path[LOAD_OPC]);
	
	Pstats ("%12u dcache total anti-conflict read hits.",
		dstats->anticonflict_hits[LOAD_OPC]);
	
	Pstats ("%12u dcache on-path anti-conflict read hits.",
		dstats->anticonflict_hits[LOAD_OPC] - 
		dstats->anticonflict_hits_off_path[LOAD_OPC]);
	
	Pstats ("%12u dcache off-path anti-conflict read hits.",
		dstats->anticonflict_hits_off_path[LOAD_OPC]);

	Pstats ("");

	if (S_dcache_write_allocate)
	{
	    Pstats ("%12u dcache total conflict write misses.",
		    dstats->conflict_misses[STORE_OPC]);
	    
	    Pstats ("%12u dcache on-path conflict write misses.",
		    dstats->conflict_misses[STORE_OPC] - 
		    dstats->conflict_misses_off_path[STORE_OPC]);
	    
	    Pstats ("%12u dcache off-path conflict write misses.",
		    dstats->conflict_misses_off_path[STORE_OPC]);
	    
	    Pstats ("%12u dcache total anti-conflict write hits.",
		    dstats->anticonflict_hits[STORE_OPC]);
	    
	    Pstats ("%12u dcache on-path anti-conflict write hits.",
		    dstats->anticonflict_hits[STORE_OPC] - 
		    dstats->anticonflict_hits_off_path[STORE_OPC]);
	    
	    Pstats ("%12u dcache off-path anti-conflict write hits.",
		    dstats->anticonflict_hits_off_path[STORE_OPC]);
	    
	    Pstats ("");
	}

	/* Print out stats for mem copy directives only if have them */
	if (S_has_mem_copy_directives)
	{
	    Pstats ("%12u dcache total conflict mem_copy misses.",
		    dstats->conflict_misses[MEM_COPY_OPC]);
	    
	    Pstats ("%12u dcache on-path conflict mem_copy misses.",
		    dstats->conflict_misses[MEM_COPY_OPC] - 
		    dstats->conflict_misses_off_path[MEM_COPY_OPC]);
	
	    Pstats ("%12u dcache off-path conflict mem_copy misses.",
		    dstats->conflict_misses_off_path[MEM_COPY_OPC]);
	
	    Pstats ("%12u dcache total anti-conflict mem_copy hits.",
		    dstats->anticonflict_hits[MEM_COPY_OPC]);
	
	    Pstats ("%12u dcache on-path anti-conflict mem_copy hits.",
		    dstats->anticonflict_hits[MEM_COPY_OPC] - 
		    dstats->anticonflict_hits_off_path[MEM_COPY_OPC]);
	    
	    Pstats ("%12u dcache off-path anti-conflict mem_copy hits.",
		    dstats->anticonflict_hits_off_path[MEM_COPY_OPC]);
	    
	    Pstats ("");

	    Pstats ("%12u dcache total conflict mem_copy_back misses.",
		    dstats->conflict_misses[MEM_COPY_BACK_OPC]);
	    
	    Pstats ("%12u dcache on-path conflict mem_copy_back misses.",
		    dstats->conflict_misses[MEM_COPY_BACK_OPC] - 
		    dstats->conflict_misses_off_path[MEM_COPY_BACK_OPC]);
	
	    Pstats ("%12u dcache off-path conflict mem_copy_back misses.",
		    dstats->conflict_misses_off_path[MEM_COPY_BACK_OPC]);
	
	    Pstats ("%12u dcache total anti-conflict mem_copy_back hits.",
		    dstats->anticonflict_hits[MEM_COPY_BACK_OPC]);
	
	    Pstats ("%12u dcache on-path anti-conflict mem_copy_back hits.",
		    dstats->anticonflict_hits[MEM_COPY_BACK_OPC] - 
		    dstats->anticonflict_hits_off_path[MEM_COPY_BACK_OPC]);
	    
	    Pstats ("%12u dcache off-path anti-conflict mem_copy_back hits.",
		    dstats->anticonflict_hits_off_path[MEM_COPY_BACK_OPC]);
	    
	    Pstats ("");

	    Pstats ("%12u dcache total conflict mem_copy_tag misses.",
		    dstats->conflict_misses[MEM_COPY_TAG_OPC]);
	    
	    Pstats ("%12u dcache on-path conflict mem_copy_tag misses.",
		    dstats->conflict_misses[MEM_COPY_TAG_OPC] - 
		    dstats->conflict_misses_off_path[MEM_COPY_TAG_OPC]);
	
	    Pstats ("%12u dcache off-path conflict mem_copy_tag misses.",
		    dstats->conflict_misses_off_path[MEM_COPY_TAG_OPC]);
	
	    Pstats ("%12u dcache total anti-conflict mem_copy_tag hits.",
		    dstats->anticonflict_hits[MEM_COPY_TAG_OPC]);
	
	    Pstats ("%12u dcache on-path anti-conflict mem_copy_tag hits.",
		    dstats->anticonflict_hits[MEM_COPY_TAG_OPC] - 
		    dstats->anticonflict_hits_off_path[MEM_COPY_TAG_OPC]);
	    
	    Pstats ("%12u dcache off-path anti-conflict mem_copy_tag hits.",
		    dstats->anticonflict_hits_off_path[MEM_COPY_TAG_OPC]);
	    
	    Pstats ("");
	}
    }
    
    /*
     * DCACHE READ-WRITE HIT RATIOS
     */
    if ((dstats->read_hits + dstats->read_misses)> 0)
	dcache_total_read_hit_ratio = 100.0 *
	    ((double) dstats->read_hits)/ 
		((double) dstats->read_hits + dstats->read_misses);
    else
	dcache_total_read_hit_ratio = 0.0;
    
    
    if (((dstats->read_hits + dstats->read_misses) -
	 (dstats->read_hits_off_path +
	  dstats->read_misses_off_path)) > 0)
	dcache_on_path_read_hit_ratio = 100.0 *
	    ((double) dstats->read_hits - 
	     (double) dstats->read_hits_off_path)/
		 (((double) dstats->read_hits +
		   (double) dstats->read_misses) - 
		  ((double) dstats->read_hits_off_path +
		   (double) dstats->read_misses_off_path));
    else
	dcache_on_path_read_hit_ratio = 0.0;
    
    if ((dstats->read_hits_off_path+dstats->read_misses_off_path)>0)
	dcache_off_path_read_hit_ratio = 100.0 *
	    ((double) dstats->read_hits_off_path)/
		((double) dstats->read_hits_off_path +
		 (double) dstats->read_misses_off_path);
    else
	dcache_off_path_read_hit_ratio = 0.0;
    
    Pstats ("# DCACHE SUMMARY:");
    Pstats ("");
    Pstats ("%12.2lf dcache total read hit ratio.",
	    dcache_total_read_hit_ratio);
    
    Pstats ("%12.2lf dcache on-path read hit ratio.",
	    dcache_on_path_read_hit_ratio);
    
    Pstats ("%12.2lf dcache off-path read hit ratio.",
	    dcache_off_path_read_hit_ratio);
    
    if ((dstats->write_hits + dstats->write_misses)> 0)
	dcache_write_hit_ratio = 100.0 * ((double) dstats->write_hits)/ 
	    ((double) dstats->write_hits + dstats->write_misses);
    else
	dcache_write_hit_ratio = 0.0;
    
    Pstats ("%12.2lf dcache write hit ratio.",
	    dcache_write_hit_ratio);
    
    
    /* Calculate combined load/store hit ratio */
    dcache_requests = 
	dstats->read_hits + dstats->read_misses +
	    dstats->write_hits + dstats->write_misses;
    dcache_requests_off_path = 	dstats->read_hits_off_path +
	dstats->read_misses_off_path;
    dcache_requests_on_path = dcache_requests - dcache_requests_off_path;
    dcache_hits = dstats->read_hits + dstats->write_hits;
    dcache_hits_off_path = dstats->read_hits_off_path;
    dcache_hits_on_path = dcache_hits - dcache_hits_off_path;
    
    
    if (dcache_requests_on_path > 0)
	dcache_on_path_hit_ratio = 100.0 * 
	    ((double)dcache_hits_on_path)/ ((double)dcache_requests_on_path);
    else
	dcache_on_path_hit_ratio = 0.0;
    
    if (dcache_requests_off_path > 0)
	dcache_off_path_hit_ratio = 100.0 * 
	    ((double)dcache_hits_off_path)/ ((double)dcache_requests_off_path);
    else
	dcache_off_path_hit_ratio = 0.0;
    
    if (dcache_requests > 0)
	dcache_hit_ratio = 100.0 * 
	    ((double)dcache_hits)/ ((double)dcache_requests);
    else
	dcache_hit_ratio = 0.0;
    
    
    Pstats ("%12.2lf dcache total read-write hit ratio.",
	    dcache_hit_ratio);
    
    Pstats ("%12.2lf dcache on-path read-write hit ratio.",
	    dcache_on_path_hit_ratio);
    
    Pstats ("%12.2lf dcache off-path read-write hit ratio (no off-path writes).",
	    dcache_off_path_hit_ratio);
    
    
    Pstats ("");

}

void * S_create_vcache_block_data ()
{
    Dcache_Data *data;
    /* 10/22/04 REK Commenting out unused variable to quiet compiler warning.
     */
#if 0
    int i;
#endif

    if ((data = (Dcache_Data *) malloc (sizeof (Dcache_Data))) == NULL)
        S_punt ("S_create_vcache_block_data: Out of memory");

    data->flags = 0;

    return (data);
}

void * S_create_dcache_block_data ()
{
    Dcache_Data *data;

    if ((data = (Dcache_Data *) malloc (sizeof (Dcache_Data))) == NULL)
	S_punt ("S_create_dcache_block_data: Out of memory");

    data->flags = 0;

    return (data);
}

void * S_create_pcache_block_data ()
{
    Dcache_Data *data;
    /* 10/22/04 REK Commenting out unused variable to quiet compiler warning.
     */
#if 0
    int i;
#endif

    if ((data = (Dcache_Data *) malloc (sizeof (Dcache_Data))) == NULL)
	S_punt ("S_create_pcache_block_data: Out of memory");

    data->flags = 0;

    return (data);
}

Dcache *S_create_dcache (pnode)
Pnode *pnode;
{
    Dcache *dcache;
    int transfer_time;
    char name_buf[100];

    if (L_pmatch (S_dcache_model_name, "perfect"))
    {
	/* Reassign name to make output look nice */
	S_dcache_model_name = "Perfect";
	S_dcache_model = DCACHE_MODEL_PERFECT;
    }
    else if (L_pmatch (S_dcache_model_name, "blocking"))
    {
	/* Reassign name to make output look nice */
	S_dcache_model_name = "Blocking";
	S_dcache_model = DCACHE_MODEL_BLOCKING;

	/* Set bypass limit to 0 to force blocking */
	S_dcache_miss_bypass_limit = 0;
    }
    else if (L_pmatch (S_dcache_model_name, "Non-blocking"))
    {
	/* make sure bypass limit > 0 (else Non-blocking doesn't make sense) */
	if (S_dcache_miss_bypass_limit <= 0)
	   S_punt("S_create_dcache: Invalid dcache_miss_bypass_limit of %i \
		for non-blocking cache.\n",S_dcache_miss_bypass_limit);

	/* Reassign name to make output look nice */
	S_dcache_model_name = "NonBlocking";
	S_dcache_model = DCACHE_MODEL_NON_BLOCKING;
    }
    else
	S_punt ("S_create_dcache: Undefined dcache model '%s'.",
		S_dcache_model_name);

    /* Check mem_copy_version */
    if ((S_mem_copy_version < 1) || (S_mem_copy_version > 3))
	S_punt ("S_create_dcache: Invalid mem_copy version %i specified.\n",
		S_mem_copy_version);

    if (Dcache_pool == NULL)
    {
	Dcache_pool = L_create_alloc_pool ("Dcache", sizeof (Dcache), 1);
    }


    dcache = (Dcache *) L_alloc (Dcache_pool);

    /* Point to processing node icache is in */
    dcache->pnode = pnode;

    /* Write Dcache name */
    sprintf (name_buf, "Dcache %i", pnode->id);
    dcache->name = strdup (name_buf);

    /* Pre-initialize fields (for debugging purposes */
    dcache->cache = NULL;
    dcache->conflict_cache = NULL;
    dcache->pcache = NULL;
    dcache->tlb = NULL;
    dcache->tlb2 = NULL;
    dcache->page_table = NULL;

    dcache->L2_busy = 0;

    /* For now, allocate TLB no matter what */

    /* Must have a page size that is a power of two and greater than 
     * the block size.
     */
    if (!S_is_power_of_two (S_memory_page_size))
	S_punt ("memory_page_size (%i) must be power of two.",
		S_memory_page_size);
    
    if (S_memory_page_size < S_dcache_block_size)
	S_punt ("memory_page_size (%i) must be > dcache_block_size (%i).",
		S_memory_page_size, S_dcache_block_size);
    
    if (S_memory_page_size < S_pcache_block_size)
	S_punt ("memory_page_size (%i) must be > pcache_block_size (%i).",
		S_memory_page_size, S_pcache_block_size);
    
    /* The TLB and TLB2 sizes must be >= 1 */
    if (S_dcache_TLB1_size < 1)
	S_punt ("dcache_TLB1_size (%i) must be >= 1.", S_dcache_TLB1_size);

    if (S_dcache_TLB2_size < 1)
	S_punt ("dcache_TLB2_size (%i) must be >= 1.", S_dcache_TLB2_size);

    /* Create two level of tlb's.  Make block size the page size
     * to get automatic mapping of addresses to tlb entries.
     * No block data needed.
     */
    dcache->tlb = S_create_cache (S_dcache_TLB1_size * S_memory_page_size,
				  S_memory_page_size,
				  S_dcache_TLB1_assoc, NULL);
    
    dcache->tlb2 = S_create_cache (S_dcache_TLB2_size * S_memory_page_size,
				   S_memory_page_size,
				   S_dcache_TLB2_assoc, NULL);
    

    dcache->page_table = 
	S_create_cache (S_dcache_page_table_size * S_memory_page_size,
			S_memory_page_size, 0, NULL);
    
    /* If not perfect dcache, allocate cache */
    if (S_dcache_model != DCACHE_MODEL_PERFECT)
    {
	/* Make sure Dcache block size is a multiple of 8 bytes */
	if ((S_dcache_block_size < 8) || (S_dcache_block_size & 7))
        S_punt ("S_create_dcache: dcache_block_size %i must be multiple of 8.",
		    S_dcache_block_size);
	

	/* Create cache, dcache block flags needed */
	dcache->cache = S_create_cache (S_dcache_size, S_dcache_block_size,
					S_dcache_assoc, 
					S_create_dcache_block_data);

	/* If measuring cache conflicts, create conflict cache (full assoc).
	 * No block data needed.
	 */
	if (S_dcache_measure_conflict_stats)
	{
	    dcache->conflict_cache = S_create_cache (S_dcache_size, 
						     S_dcache_block_size,
						     0, NULL);
	}

	/* If we want a prefetch cache, create it */
	if (S_prefetch_cache)
	{

	   /* Make sure Pcache block size is a multiple of 8 bytes */
	   if ((S_pcache_block_size < 8) || (S_pcache_block_size & 7))
              S_punt (
		"S_create_pcache: pcache_block_size %i must be multiple of 8.",
		    S_pcache_block_size);

	   /* Create cache, dcache block flags needed */
	   dcache->pcache = S_create_cache (S_pcache_size, S_pcache_block_size,
					S_pcache_assoc, 
					S_create_pcache_block_data);

	}

	/* If we have a victim cache allocate it */
        if (S_victim_cache)
	{
            vcache = S_create_cache (S_vcache_size,
                        S_vcache_block_size, 0,
                        S_create_vcache_block_data);
	}

    }

    /* Initially dcache is not busy */
    pnode->dcache_busy = 0;

    /* Create dcache queues */
    dcache->input_queue = S_create_queue ("Dcache_input_queue", pnode->id);
    dcache->write_buffer = S_create_queue ("Dcache_write_buffer", pnode->id);
    dcache->miss_buffer = S_create_queue ("Dcache_miss_buffer", pnode->id);
    dcache->miss_request_buffer = 
	S_create_queue ("Dcache_miss_request_buffer", pnode->id);
    dcache->prefetch_request_buffer = 
	S_create_queue ("Dcache_prefetch_request_buffer", pnode->id);
    dcache->pending_prefetches =
	S_create_queue ("Dcache_pending_prefetches", pnode->id);
    dcache->mc_instr_queue = 
	S_create_queue ("Dcache_mc_instr_queue", pnode->id);
    dcache->mc_pending_queue = 
	S_create_queue ("Dcache_mc_pending_queue", pnode->id);
    dcache->mc_check_queue = 
	S_create_queue ("Dcache_mc_check_queue", pnode->id);
    dcache->mc_request_buffer =
	M_create_queue ("Dcache_mc_request_buffer", pnode->id);

    /* Initialize communication queues on pnode */
    pnode->dcache_request = S_create_queue ("Dcache_request", pnode->id);

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

    /* Initialize bus */
    if (S_secondary_cache) 
	S_L2_bus.cycle_avail = 0;
    else
	S_bus.cycle_avail = 0;

    /* Make sure bus_bandwidth is power of two */
    if (!S_is_power_of_two (bus_bandwidth))
	S_punt ("bus_bandwidth (%i) must be power of two.", 
		bus_bandwidth);

    /* Calculate latencies for dcache */
    transfer_time = (S_dcache_block_size/bus_bandwidth);

    /* Calculate mask and size for data that bypasses the L1 cache */
    S_bypass_addr_mask = ~(bus_bandwidth - 1);
    S_bypass_req_size = bus_bandwidth;

    /* Assume have separate address lines, and get first word
     * after S_memory_latency cycles
     */
    S_dcache_read_block_latency = 1 + S_memory_latency + (transfer_time - 1);
    S_dcache_write_block_latency = transfer_time;
    S_dcache_write_thru_latency = 1;
    if (S_streaming_support)
	S_dcache_streaming_benefit = transfer_time - 1;
    else
	S_dcache_streaming_benefit = 0;

    /* Calculate latencies for pcache - only read block latency differs */
    if (S_prefetch_cache)
    {
       transfer_time = (S_pcache_block_size/bus_bandwidth);
       S_pcache_read_block_latency = 1 + memory_latency + (transfer_time - 1);
       if (S_streaming_support)
	  S_pcache_streaming_benefit = transfer_time - 1;
       else
	  S_pcache_streaming_benefit = 0;
    }

    /* Initialize fields used for efficiency */
    dcache->dstats = NULL;

    /* JCG 9/29/96 intialize addr_loaded_last_cycle*/
    dcache->addr_loaded_last_cycle = -1;
    
    return (dcache);
}


void S_debug_tlb (FILE *out)
{
    /* Debug */
    if (S_print_TLB_contents)
    {
	fprintf (out, 
		 "# BEGIN TLB2 CONTENTS AT END OF SIMULATION\n");
	S_cache_contents_print (out, S_pnode->dcache->tlb2);
	fprintf (out, 
		 "# END TLB2 CONTENTS AT END OF SIMULATION\n");
    }
}

/* 
 * Searches queue from tail (most recent) to head for an entry
 * whose memory address overlaps.
 * => Changed to search from head (oldest) entry to tail
 *    so that request that is sent out has correct combination
 *    of target addrs.
 */
Sq_entry *find_overlapping_entry (Squeue *queue, int sint_addr,
				  unsigned sint_conflict_mask)
{
    Sq_entry	*search;
    unsigned	conflict_mask;
    
    /* Start search at "tail", most recent entry */
    /* => now start at "head", oldest entry */
    for (search = queue->head; search != NULL; search = search->next_entry)
    {
	/* Use mask to compare addresses */
	conflict_mask = sint_conflict_mask & search->sint->conflict_mask;
		    
	/* If sint operlaps with entry, return this entry */
	if ((sint_addr & conflict_mask) ==
	    (search->sint->trace.mem_addr & conflict_mask))
	{
	    return (search);
	}
    }

    /* No overlapping entry found */
    return (NULL);
}    

/* 
 * Searches queue from tail (most recent) to head for an entry
 * whose memory address overlaps and has the same target cache.
 * => Changed to search from head (oldest) entry to tail
 *    so that request that is sent out has correct combination
 *    of target addrs.
 */
Sq_entry *find_overlapping_entry_and_target (int dest,
		Squeue *queue, int sint_addr, unsigned sint_conflict_mask)
{
    Sq_entry	*search;
    unsigned	conflict_mask;
    int		search_dest;

    if (!dest) dest = PLAYDOH_TCHS_C1;

    /* Start search at "tail", most recent entry */
    /* => now start at "head", oldest entry */
    for (search = queue->head; search != NULL; search = search->next_entry)
    {

	/* if this is a wbsint from write buffer then there will be no oper */
	if (!search->sint->oper) search_dest = PLAYDOH_TCHS_C3;
	else
	{
	    search_dest = target_cache(search->sint);
	}

	/* Use mask to compare addresses */
	conflict_mask = sint_conflict_mask & search->sint->conflict_mask;

	/* If sint overlaps with entry, return this entry */
	if (((sint_addr & conflict_mask) ==
	    (search->sint->trace.mem_addr & conflict_mask)) &&
	    (dest & search_dest))
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
 * => Changed to search from head (oldest) entry to tail
 *    so that request that is sent out has correct combination
 *    of target addrs.
 */
Sq_entry *find_covering_entry(int dest_request_size,
		Squeue *queue, int sint_addr, unsigned sint_conflict_mask, int *request_size)
{
    Sq_entry	*search;
    unsigned	conflict_mask;
    int		search_request_size;
    int		search_dest;

    /* Start search at "tail", most recent entry */
    /* => now start at "head", oldest entry */
    for (search = queue->head; search != NULL; search = search->next_entry)
    {

	search_dest = search->sint->playdoh_flags & PLAYDOH_TCHS_ALL;

    	if (S_prefetch_cache && (search_dest & PLAYDOH_TCHS_V1) &&
			!((search_dest & PLAYDOH_TCHS_C1) && 
			(S_dcache_block_size > S_pcache_block_size))) 
	    search_request_size = S_pcache_block_size;
    	else if ((search_dest & PLAYDOH_TCHS_C1) ||
			!(search_dest & PLAYDOH_TCHS_ALL))
	    search_request_size = S_dcache_block_size;
    	else
	    search_request_size = S_bypass_req_size;

	/* Use mask to compare addresses */
	conflict_mask = sint_conflict_mask & ~(search_request_size - 1);

	/* If sint operlaps with entry, return this entry */
	if (((sint_addr & conflict_mask) ==
	    (search->sint->trace.mem_addr & conflict_mask)) &&
	    search_request_size >= dest_request_size)
	{
	    *request_size = search_request_size;
	    return (search);
	}
    }

    /* No overlapping entry found */
    return (NULL);
}

/* Create Writeback Sint for writeback data */
Sint *create_wbsint(unsigned addr, unsigned size, Stats *stats)
{
    Sint *wbsint;

    wbsint = (Sint *) L_alloc (Sint_pool);
    wbsint->serial_no = -1;
    wbsint->fn = NULL;
    wbsint->oper = NULL;
    wbsint->flags = 0;
    wbsint->playdoh_flags = 0;
    wbsint->trace.mem_addr = addr;
    wbsint->access_size = size;
    wbsint->conflict_mask = ~(size - 1);
    wbsint->real_complete_time[S_first_dest] = -1;
    wbsint->virtual_complete_time[S_first_dest] = -1;
    wbsint->entry_list = NULL;
    wbsint->stats = stats;
    wbsint->proc_data_v = NULL;
    wbsint->superscalar = NULL;
    wbsint->vliw = NULL;

    return wbsint;
}

/* if new_addr != -1, replaces block, else invalidates */
void replace_dcache_block (Dcache *dcache, Scblock *block, int new_addr,
			   Stats *stats)
{
    Dcache_Data	*data, *data2;
    Sint *wbsint;
    Dcache_Stats *dstats;
    unsigned addr2 = 0;
    int block_size;
    Scblock *block2 = NULL;

    /*
     * Get dcache stats structure for ease of use.
     * Set in main dcache routines each cycle.
     */
    dstats = stats->dcache;

    /* If we have a victim cache then find victim cache entry to replace */
    if (S_victim_cache && new_addr != -1)
    {
        block2 = block;
        addr2 = block2->start_addr;
        block = S_cache_find_LRU(vcache,addr2);
        block_size = vcache->block_size;
    }

    /* Get block's data */
    data = (Dcache_Data *) block->data;

    /* If block is valid have conflict and need to check 
     * if dirty
     */
    if (block->hash_next != (Scblock *)-1)
    {
	/* Update stats */
	dstats->blocks_kicked_out++;
			
	if (S_dcache_debug_misses)
	{
	    if (new_addr != -1)
	        fprintf (debug_out, 
		     "Line full, kicked out %x of primary cache",
		     block->start_addr);
	    else
	        fprintf (debug_out,
		     "Line written in other cache, kicked out %x of primary cache",
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
	    dstats->dirty_blocks_kicked_out++;
	    
	    wbsint = create_wbsint(block->start_addr,S_dcache_block_size,stats);

	    /* Look up address in TLB */
	    dcache_TLB_lookup (dcache, dstats, wbsint->trace.mem_addr, 0,
			       NULL);
	    
	    S_enqueue (dcache->write_buffer, wbsint);
	}
    }
    
    if (new_addr != -1)
    {
	/* Change block to new address */

	/* If we have a victim cache then place victim from L1 cache there */
        if (S_victim_cache)
        {
          S_cache_change_addr (vcache, block, addr2);
    	  data2 = data;
          block = block2;
    	  data = (Dcache_Data *) block->data;
	  data2->flags = data->flags;
        }
	S_cache_change_addr (dcache->cache, block, new_addr);
    }
    else
	/* invalidate block */
	S_cache_invalidate (dcache->cache, block);
    
    /* Mark block as not dirty */
    data->flags = 0;
}

/* if new_addr != -1, replaces block, else invalidates */
void replace_pcache_block (Dcache *dcache, Scblock *block, int new_addr,
			   Stats *stats)
{
    Dcache_Data	*data;
    Sint *wbsint;
    Dcache_Stats *dstats;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    unsigned mask,next_mask,inx,next_inx;
    int i,size;
#endif

    /*
     * Get dcache stats structure for ease of use.
     * Set in main dcache routines each cycle.
     */
    dstats = stats->dcache;

    /* Get block's data */
    data = (Dcache_Data *) block->data;

    /* If block is valid have conflict and need to check 
     * if dirty
     */
    if (block->hash_next != (Scblock *)-1)
    {
	/* Update stats */
	dstats->pcache_blocks_kicked_out++;
			
	if (S_dcache_debug_misses)
	{
	    if (new_addr != -1)
	        fprintf (debug_out, "Line full, kicked out %x of prefetch cache",
		     block->start_addr);
	    else
	        fprintf (debug_out,"Line written in other cache, kicked out %x of prefetch cache",
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
	    dstats->pcache_dirty_blocks_kicked_out++;
	    
	    wbsint = create_wbsint(block->start_addr,S_pcache_block_size,stats);

	    /* Look up address in TLB */
	    dcache_TLB_lookup (dcache, dstats, wbsint->trace.mem_addr, 0, NULL);
	    
	    S_enqueue (dcache->write_buffer, wbsint);
	}
    }
    
    if (new_addr != -1)
	/* Change block to new address */
	S_cache_change_addr (dcache->pcache, block, new_addr);
    else
	/* invalidate block */
	S_cache_invalidate (dcache->pcache, block);
    
    /* Mark block as not dirty */
    data->flags = 0;
}

/* writes block back to memory and makes it clean in dcache */
void writeback_dcache_block (Dcache *dcache, Scblock *block, Stats *stats)
{
    Dcache_Data	*data;
    Sint *wbsint;
    Dcache_Stats *dstats;

    /*
     * Get dcache stats structure for ease of use.
     * Set in main dcache routines each cycle.
     */
    dstats = stats->dcache;


    /* Get block's data */
    data = (Dcache_Data *) block->data;

    if (S_dcache_debug_misses)
    {
	fprintf (debug_out,"Writeback of DIRTY block %x from primary cache\n", 
		block->start_addr);
    }
	
    /* Create an fake sint to write back block with */

    /* Update stats */
    dstats->dirty_blocks_written_back++;
	    
    wbsint = create_wbsint(block->start_addr,S_dcache_block_size,stats);

    /* Look up address in TLB */
    dcache_TLB_lookup (dcache, dstats, wbsint->trace.mem_addr, 0, NULL);
        
    S_enqueue (dcache->write_buffer, wbsint);
    
    /* Mark block as not dirty */
    data->flags = 0;
}

/* writes block back to memory and makes it clean in pcache */
void writeback_pcache_block (Dcache *dcache, Scblock *block, Stats *stats)
{
    Dcache_Data	*data;
    Sint *wbsint;
    Dcache_Stats *dstats;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    unsigned mask,next_mask,inx,next_inx;
    int i,size;
#endif

    /*
     * Get dcache stats structure for ease of use.
     * Set in main dcache routines each cycle.
     */
    dstats = stats->dcache;


    /* Get block's data */
    data = (Dcache_Data *) block->data;

    if (S_dcache_debug_misses)
    {
	fprintf (debug_out,"Writeback of DIRTY block %x from prefetch cache\n", 
		block->start_addr);
    }
	
    /* Create an fake sint to write back block with */

    /* Update stats */
    dstats->pcache_dirty_blocks_written_back++;
	    
    wbsint = create_wbsint(block->start_addr,S_pcache_block_size,stats);

    /* Look up address in TLB */
    dcache_TLB_lookup (dcache, dstats, wbsint->trace.mem_addr, 0, NULL);
    
    S_enqueue (dcache->write_buffer, wbsint);

    /* Mark block as not dirty */
    data->flags = 0;
}

/* Cancels prefetches to a block and resets the block's PREFETCH_BIT */
void cancel_prefetch (Dcache *dcache, Scblock *block, Sint *sint, Scache *cache)
{
    Dcache_Data *data;
    int index_conflict_mask;
    Sint *temp_sint;
    Sq_entry	*overlaps;

    /* Get data for block */
    data = (Dcache_Data *) block->data;
    
    /* If prefetch bit is not set, do nothing */
    if (!(data->flags & PREFETCH_BIT))
	return;

    /* Clear prefetch bit */
    data->flags ^= PREFETCH_BIT;

    /*
     * Search prefetch buffer for prefetch for the same cache index.
     */
    index_conflict_mask = cache->tag_index_mask <<
	cache->block_id_shift;
    overlaps = find_overlapping_entry (dcache->prefetch_request_buffer,
			       sint->trace.mem_addr,  index_conflict_mask);

    if (overlaps != NULL)
    {
	if (S_dcache_debug_prefetch)
	{
	    fprintf (debug_out,
		     "PREFETCH cancelled op %i addr %x by op %i addr %x\n",
		     overlaps->sint->oper->lcode_id, 
		     overlaps->sint->trace.mem_addr,
		     sint->oper->lcode_id,
		     sint->trace.mem_addr);
	}
	
	/* Done with prefetch, free entry and sint */
	temp_sint = overlaps->sint;
	S_dequeue(overlaps);
	S_free_sint (temp_sint);
    }
}

/* Returns 1 if already request out for block at the given address */
int S_find_outstanding_request (Dcache *dcache, Sint *sint)
{
    int block_addr, addr_mask, dest;
    Sq_entry *overlaps;
    S_Opc_Info *info;
    /* 10/22/04 REK Commenting out unused variable to quiet compiler warning.
     */
#if 0
    Sint *temp_sint;
#endif
    Dcache_Stats *dstats;
    int overlapping_entry = 0;
    int request_size, overlap_request_size;
    unsigned bus_conflict_mask;
    
    /* Get stats struct */
    dstats = sint->stats->dcache;

    /* check destination cache of this request */
    if (S_prefetch_cache && sint->playdoh_flags & PLAYDOH_TCHS_V1)
    {
	dest = PLAYDOH_TCHS_V1;
	request_size = S_pcache_block_size;
    }
    else if ((sint->playdoh_flags & PLAYDOH_TCHS_C1) ||
		!(sint->playdoh_flags & PLAYDOH_TCHS_ALL))
    {
	dest = PLAYDOH_TCHS_C1;
	request_size = S_dcache_block_size;
    }
    else 
    {
	dest = PLAYDOH_TCHS_C3;
	request_size = S_bypass_req_size;
    }

    /* Use the bypass mask in all cases, since only the element needs to
     * be covered to combine requests.
     */
    addr_mask = S_bypass_addr_mask;

    /* Get bus request address */
    block_addr = sint->trace.mem_addr & addr_mask;

    bus_conflict_mask = ~(BUS(size) - 1);

    /* If data is coming in on the bus this cycle, then don't need request */
    if ((BUS(dest) == DCACHE) &&
	(BUS(addr) == (block_addr & bus_conflict_mask)))
    {
	   /* If this is the first cycle of a bus return of data, must set
	    * S_current_dest which wouldn't be set until 2nd half cycle dcache sim.
	    */
	   if (!S_current_dest)
	   {
	     S_current_dest |= target_cache(dcache->miss_buffer->head->sint);
	   }

	   /* Must add this dest to the current_dest (only if current dest is not a 
	    * first-level cache).
	    */
	   if ((sint->playdoh_flags & PLAYDOH_TCHS_ALL) &&
			(request_size <= BUS(size)) &&
			!(S_current_dest&(PLAYDOH_TCHS_C1|PLAYDOH_TCHS_V1)))
	       	S_current_dest |= dest;
	   /* Otherwise reset target cache specifier of this request so it retires 
	    * from miss buffer properly.
	    */
	   else
		set_playdoh_flags(sint,S_current_dest);

	   /* if this is a prefetch, make sure it is not thrown away */
	   if (BUS(type) == PREFETCH_RESULT)
	   {
    		overlaps = find_covering_entry(S_bypass_req_size,
			dcache->pending_prefetches, block_addr,
			bus_conflict_mask, &overlap_request_size);
		if (!overlaps)
			S_punt ("No pending prefetch for dcache request of %x",
								BUS(addr));
		info = &opc_info_tab[GET_OPC(sint)];
		if (info->opc_type != PREFETCH_OPC)
	    	    overlaps->sint->flags |= PREFETCH_TAKEN_OVER;
	   }
	   if (S_dcache_debug_misses)
	   {
	     	fprintf (debug_out, "BUS RESPONSE of %x COMBINED \n", 
		 	BUS(addr));
	   }
	   return (1);
    }

    /* If have a prefetch request that has already gone to memory, then
     * already have request for block. Check prefetches first since
     * they have already gone to memory.
     */
    overlaps = find_covering_entry(S_bypass_req_size, dcache->pending_prefetches, 
				block_addr, addr_mask, &overlap_request_size);
    /* If a load or store miss, then take over the prefetch in addition
     * to not making another request.
     */
    if (overlaps != NULL)
    {
	/* If not a prefetch, remove the pending prefetch request */
	info = &opc_info_tab[GET_OPC(sint)];
	if (info->opc_type != PREFETCH_OPC)
	{
	    if (S_dcache_debug_prefetch)
	    {
		fprintf (debug_out,
			 "PREFETCH of %x TAKEN OVER\n", 
			 overlaps->sint->trace.mem_addr);
	    }

	    /* mark prefetch as taken over */
	    overlaps->sint->flags |= PREFETCH_TAKEN_OVER;
	}

	if (request_size > overlap_request_size)
	{
	    set_playdoh_flags(sint,
		overlaps->sint->playdoh_flags & PLAYDOH_TCHS_ALL);
	}

	return (1);
    }

    /* check if there are overlapping entries with diff targets */
    if (!overlaps &&
    		find_overlapping_entry(dcache->pending_prefetches, 
		block_addr, addr_mask))
    {
	overlapping_entry = 1;
	dstats->overlapping_entries_with_different_dests++;
    }

    /* Look for outstanding loads or write_allocate store (causes load),
     * if found, load request has already been issued
     */
    overlaps = find_covering_entry(S_bypass_req_size, dcache->miss_buffer, 
				block_addr, addr_mask, &overlap_request_size);
    if (overlaps != NULL)
    {
	if (S_dcache_debug_misses)
	{
	    fprintf (debug_out,
		 "LOAD of %x COMBINED\n", 
		 overlaps->sint->trace.mem_addr);
	}

	if (request_size > overlap_request_size)
	{
	    set_playdoh_flags(sint,
			overlaps->sint->playdoh_flags & PLAYDOH_TCHS_ALL);
	}

	return (1);
    }

    /* check if there are overlapping entries with diff targets */
    if (!overlapping_entry && !overlaps &&
		find_overlapping_entry(dcache->miss_buffer, 
		block_addr, addr_mask))
    {
	dstats->overlapping_entries_with_different_dests++;
    }

    /* Checked everywhere, no outstanding request for block */
    return (0);
}

/* Determine if the mem_copy for the check is complete */
int is_mem_copy_complete (Dcache *dcache, int buf)
{
    Sq_entry *entry;
    Sint *sint;
    int opc_type;

    /* Check instr queue */
    for (entry = dcache->mc_instr_queue->head; entry != NULL; 
	 entry = entry->next_entry)
    {
	sint = entry->sint;
	
	/* Return 0 if in queue and is a MEM_COPY instruction 
	 * or a MEM_COPY_TAG instruction
	 */
	if (sint->trace.mem_copy_buf == buf)
	{
	    opc_type = opc_info_tab[GET_OPC(sint)].opc_type;
	    if ((opc_type == MEM_COPY_OPC) ||
		(opc_type == MEM_COPY_TAG_OPC))
		return (0);
	}
    }

    /* Check pending queue */
    for (entry = dcache->mc_pending_queue->head; entry != NULL; 
	 entry = entry->next_entry)
    {
	sint = entry->sint;
	
	/* Return 0 if in queue */
	if (sint->trace.mem_copy_buf == buf)
	    return (0);
    }

    /* If got here, then mem copy is complete */
    return (1);
}

void update_mem_copy_checks(Dcache *dcache)
{
    Sq_entry *entry, *next_entry;
    Sint *sint;
    S_Opc_Info *info;

    /* 
     * MEM_COPY_CHECK handling.
     */
    for (entry = dcache->mc_check_queue->head; entry != NULL; 
	 entry = next_entry)
    {
	/* Get the next entry before we mess with current entry */
	next_entry = entry->next_entry;

	/* Get the sint for this entry */
	sint = entry->sint;

	if (is_mem_copy_complete (dcache, sint->trace.mem_copy_buf))
	{
	    info = &opc_info_tab[GET_OPC(sint)];
	    /* Return mem_copy next cycle */
	    entry->sint->real_complete_time[S_first_dest] = S_sim_cycle + 
		(entry->sint->oper->real_latency[S_first_dest] - 1);

	    /* Remove from this queue, still in processor's queue */
	    S_dequeue (entry);
	}
    }
}

/* Makes an address MRU if in conflict cache */
void S_make_dcache_conflict_cache_MRU (Dcache *dcache, int addr)
{
    Scblock 	*block;
    

    /* Is block in conflict cache */
    block = S_cache_find_addr (dcache->conflict_cache, addr);

    /* If Hit, make MRU */
    if (block != NULL)
    {
	/* Make block MRU (if hit or miss) */
	S_cache_make_MRU (dcache->conflict_cache, block);
    }

}

/* Adds an address to the conflicct cache */
void S_add_to_dcache_conflict_cache (Dcache *dcache, int addr)
{
    Scblock 	*block;
    
    /* Is block in conflict cache */
    block = S_cache_find_addr (dcache->conflict_cache, addr);

    /* If Miss, get LRU */
    if (block == NULL)
    {
	block = S_cache_find_LRU (dcache->conflict_cache, addr);
	S_cache_change_addr (dcache->conflict_cache, block, addr);
    }

    /* Make MRU if hit or miss */
    S_cache_make_MRU (dcache->conflict_cache, block);
}

/*
 * Updats conflict stats but does not update conflict cache.
 * Use the two above functions to add to conflict cache or make an
 * address MRU.
 */
void S_update_dcache_conflict_stats (int opc_type, Dcache *dcache, 
				       Dcache_Stats *dstats, 
				       int addr, int dcache_hit, int flags)
{
    Scblock 	*block;
    /* 10/22/04 REK Commenting out unused variable to quiet compiler warning.
     */
#if 0
    int		conflict_cache_hit;
#endif
    

    /* Is block in conflict cache */
    block = S_cache_find_addr (dcache->conflict_cache, addr);

    /* Hit, update conflict miss stats if dcache misses*/
    if (block != NULL)
    {
	if (!dcache_hit)
	{
	    dstats->conflict_misses[opc_type]++;
	    if (flags & OFF_PATH)
	    {
		dstats->conflict_misses_off_path[opc_type]++;
	    }
	}
    }

    /* If miss and dcache hits then update anticonflict hit stats */
    else
    {
	if (dcache_hit)
	{
	    dstats->anticonflict_hits[opc_type]++;
	    if (flags & OFF_PATH)
	    {
		dstats->anticonflict_hits_off_path[opc_type]++;
	    }
	}
    }
}


/* Search write buffer for stores into this block. Remove them and mark 
 * block dirty if any found and if WRITE_THROUGH flag not set.
 */
void write_buffer_to_block(Scache *cache, Dcache *dcache, Scblock *block,
		Dcache_Data *data, unsigned start_addr_mask)
{
    Sq_entry *entry,*next_entry;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    unsigned mask;
    int inx;
#endif
    unsigned addr_mask;
    Sint *temp_sint;

    for (entry = dcache->write_buffer->head; entry != NULL;
                 entry = next_entry)
    {
	/* Get next entry before messing with this one */
	next_entry = entry->next_entry;

	addr_mask = start_addr_mask & entry->sint->conflict_mask;

	if (block->start_addr == (entry->sint->trace.mem_addr & addr_mask))
	{
	    if (!(entry->sint->flags & WRITE_THROUGH))
	    {
		/* Mark block as dirty */
		data->flags |= DIRTY_BLOCK;

		/* Finished with store sint, free entry and it
                        if it is a subset of this block */
		if (~(cache->block_size - 1) <= entry->sint->conflict_mask)
		{
		    temp_sint = entry->sint;
		    S_dequeue (entry);
		    if (!temp_sint->entry_list)
			S_free_sint (temp_sint);
		}
	    }
	}
    }
}

void bus_data_to_cache (Scache *cache,unsigned bus_addr,unsigned addr,int dest,
			Dcache *dcache, Stats *bus_stats, int prefetch,
			int used_by_load)
{
    Scblock *block;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    Sq_entry *entry,*next_entry;
    Sint *temp_sint;
    unsigned addr_mask;
    int i;
#endif
    Dcache_Data *data;
    unsigned start_addr_mask;

    start_addr_mask = cache->start_addr_mask;

    /* Debug, make sure block is not already in cache */
    if (S_cache_find_addr (cache, addr) != NULL)
    {
	if (S_dcache_debug_misses && !prefetch)
	{
	    if (dest == PLAYDOH_TCHS_V1)
	    	fprintf (debug_out, 
		     "Block from memory (%8x) already in pcache.\n",
		     addr);
	    else
	    	fprintf (debug_out, 
		     "Block from memory (%8x) already in dcache.\n",
		     addr);
	}
    }
    else
    {
      /* Write data into cache */
	/* Get LRU block for this address */
	block = S_cache_find_LRU (cache, addr);
	
	/* Get block data (flags) */
	data = (Dcache_Data *) block->data;
		
        /* If this is not a prefetch, or if prefetch bit set 
	 * (or if we are ignoring the bit),
         * or if this prefetch has been taken over by a load, then
         * replace block with data
         */
        if (!prefetch || (data->flags & PREFETCH_BIT) ||
            S_dcache_ignore_prefetch_bit || used_by_load)
        {
	
	  /* If doing conflict stats, add address to conflict cache  */
	  if (S_dcache_measure_conflict_stats &&
		dest == PLAYDOH_TCHS_C1)
	  {
	    S_add_to_dcache_conflict_cache (dcache, 
					    addr);
	  }

	  /* Replace block in cache with data from memory */
	  if (S_prefetch_cache && dest == PLAYDOH_TCHS_V1)
	  {
	    replace_pcache_block (dcache, block, addr, bus_stats);
	  }
	  else
	    replace_dcache_block (dcache, block, addr, bus_stats);

          /* If load, update block as load finished */
          if (!prefetch || used_by_load)
          {
	    /* Make block most recently used in cache line */
	    S_cache_make_MRU (cache, block);

	    if (prefetch)
	    {
            	/* Update stats */
            	bus_stats->dcache->loads_used_prefetch_request++;

            	if (S_dcache_debug_prefetch)
            	{
		    fprintf (debug_out,
                         "PREFETCH taken over by load for addr %x\n",
                         addr);
            	}
	    }

	    else
	    {
	      /* Search write buffer for stores into this block 
	       * Remove them and mark block dirty if any found
	       * and if WRITE_THROUGH flag not set.
	       */
	      write_buffer_to_block(cache,dcache,block,data,start_addr_mask);
	    }
	  }
	  /* If prefetch, update block as prefetch */
          else
          {
            /* Leave in LRU position and still have prefetch bit set */
            data->flags |= PREFETCH_BIT;

            /* Update stats */
            bus_stats->dcache->prefetches_completed++;

            if (S_dcache_debug_prefetch)
            {
		fprintf (debug_out,
                     "PREFETCH completed for addr %x\n",
                     addr);
            }
          }

	  /* All pending loads on this data will be ready next cycle. */
	  dcache->addr_loaded_last_cycle = bus_addr;
	}
        else if (prefetch)
        {
            if (S_dcache_debug_prefetch)
            {
                fprintf (debug_out,
                             "PREFETCH data thrown away for addr %x\n",
                             addr);
            }
        }
    }

    if (!prefetch)
    {
        /* All pending misses (except first one (handled above))
         * on this data will be ready next cycle.
         */
        dcache->addr_loaded_last_cycle = bus_addr;
    }
}

/* Does TLB lookup, and keeps appropriate stats.
 * 'sint' is used for debugging only and may be NULL.
 */
static void dcache_TLB_lookup (Dcache *dcache, Dcache_Stats *dstats, 
			       int addr, int flags, Sint *sint)
{
    Scblock 	*block, *block2, *page;
    
    /* Assume accesses that bus error do not access the tlb */
    if ((sint != NULL) && 
	(sint->flags & MASKED_BUS_ERROR))
	return;

    /* Update tlb lookups */
    dstats->total_TLB_lookups++;

    /* Is page in first-level tlb */
    block = S_cache_find_addr (dcache->tlb, addr);

    /* If Miss, is page in second-level tlb? */
    if (block == NULL)
    {
	/* Count first level misses */
	dstats->total_TLB1_misses++;

	block2 = S_cache_find_addr (dcache->tlb2, addr);

	/* If miss, put in second-level and page table (if necessary) */
	if (block2 == NULL)
	{
	    /* Count second level misses */
	    dstats->total_TLB2_misses++;

	    /* replace LRU in second level */
	    block2 = S_cache_find_LRU (dcache->tlb2, addr);
	    S_cache_change_addr (dcache->tlb2, block2, addr);

	    /* See if in page table */
	    page = S_cache_find_addr (dcache->page_table, addr);

	    /* If not, add to page table */
	    if (page == NULL)
	    {
		/* Count pages accessed by dcache */
		dstats->total_pages_accessed++;

		/* Replace LRU in page table */
		page = S_cache_find_LRU (dcache->page_table, addr);
		S_cache_change_addr (dcache->page_table, page, addr);

		if (S_debug_page_faults)
		{
		    if (sint != NULL)
		    {
			fprintf (debug_out, "Page fault %08x (Func %s): ",
				 page->start_addr, sint->fn->name);
			S_print_only_sint (debug_out, sint);
		    }
		    else
		    {
			fprintf (debug_out, "Page fault %08x (no sint)\n",
				 page->start_addr);
		    }
		}
	    }

	    /* Make page MRU if hit or miss in page table */
	    S_cache_make_MRU (dcache->page_table, page);
	}

	/* Make MRU if hit or miss in second level tlb */
	S_cache_make_MRU (dcache->tlb2, block2);

	/* Replace LRU in first level */
	block = S_cache_find_LRU (dcache->tlb, addr);
	S_cache_change_addr (dcache->tlb, block, addr);
    }

    /* Make MRU if hit or miss in first level tlb */
    S_cache_make_MRU (dcache->tlb, block);
}

void process_pending_mem_copies(Dcache *dcache)
{
    Sq_entry *entry;
    Sint *sint;
    unsigned	mc_size, mc_count, mc_stride;
    unsigned	mc_setup, mc_stride_offset;
    unsigned 	mc_block_addr, prev_mc_block_addr;
    unsigned	mc_buf, mc_source;
    unsigned    mc_element, mc_byte;
    int		opc_type;
    Scblock	*block;
    Dcache_Data *data;
    unsigned 	start_addr_mask;
    unsigned	start_line, end_line;
    Dcache_Stats *dstats;

    /* Make sure dstats not used before initialized */
    dstats = NULL;

    /* Get start_addr_mask for ease of use */
    start_addr_mask = dcache->cache->start_addr_mask;

    /* 
     * MEM_COPY and MEM_COPY_BACK handling.
     * 
     * Process an mem_copy instruction if one is pending and
     * mc_requests are not already pending.
     */
    if (((entry = dcache->mc_instr_queue->head) != NULL) &&
	(dcache->mc_request_buffer->size == 0))
    {
	/* Get the sint for this entry */
	sint = entry->sint;

	/* Use the dcache stats structure for this sint */
	dstats = sint->stats->dcache;

	/* Get whether mem_copy or mem_copy_back */
	opc_type = opc_info_tab[GET_OPC(sint)].opc_type;

	/* Get the mem_copy setup word */
	mc_setup = sint->trace.mem_copy_setup;
	
	mc_size = mc_setup & 0xFFFF;
	mc_count = sint->trace.mem_copy_count;
	mc_stride = (mc_setup >> 16);
	
	/* Calculate offset between copies of dest */
	mc_stride_offset = mc_stride - mc_size;
	
	/* Get the buf and source addr */
	mc_buf = sint->trace.mem_copy_buf;
	mc_source = sint->trace.mem_copy_array;
	
	/* Update stats */
	if (opc_type == MEM_COPY_OPC)
	{
	    dstats->num_mem_copies++;
	    
	    if (S_dcache_debug_mem_copy)
	    {
		start_line = (mc_buf >> dcache->cache->block_id_shift) &
		    dcache->cache->tag_index_mask;
		end_line = ((mc_buf + ((mc_count -1) * mc_size) ) >>
			    dcache->cache->block_id_shift) &
				dcache->cache->tag_index_mask;
		fprintf (debug_out, 
			 "MEM_COPY  array:%08x buf:%08x(%i-%i) size:%i stride:%i #:%i op %i\n",
			 mc_source, mc_buf, start_line, end_line, mc_size, 
			 mc_stride, mc_count, sint->oper->lcode_id);
	    }
	}
	else if (opc_type == MEM_COPY_TAG_OPC)
	{
	    dstats->num_mem_copy_tags++;
	    
	    if (S_dcache_debug_mem_copy)
	    {
		start_line = (mc_buf >> dcache->cache->block_id_shift) &
		    dcache->cache->tag_index_mask;
		end_line = ((mc_buf + ((mc_count -1) * mc_size) ) >>
			    dcache->cache->block_id_shift) &
				dcache->cache->tag_index_mask;
		fprintf (debug_out, 
			 "COPY_TAG  array:%08x buf:%08x(%i-%i) size:%i stride:%i #:%i op %i\n",
			 mc_source, mc_buf, start_line, end_line, mc_size, 
			 mc_stride, mc_count, sint->oper->lcode_id);
	    }
	}
	else if (opc_type == MEM_COPY_BACK_OPC)
	{
	    dstats->num_mem_copy_backs++;
	    if (S_dcache_debug_mem_copy)
	    {
		start_line = (mc_buf >> dcache->cache->block_id_shift) &
		    dcache->cache->tag_index_mask;
		end_line = ((mc_buf + ((mc_count -1) * mc_size) ) >>
			    dcache->cache->block_id_shift) &
				dcache->cache->tag_index_mask;
		fprintf (debug_out, 
			 "COPY_BACK  array:%08x buf:%08x(%i-%i) size:%i stride:%i #:%i op %i\n",
			 mc_source, mc_buf, start_line, end_line, mc_size, 
			 mc_stride, mc_count, sint->oper->lcode_id);
	    }
	}

	/* Set the previous block addr to an impossible to have value */
	prev_mc_block_addr = (unsigned) -1;

	/* Initialize request counter */
	sint->trace.mem_copy_requests = 0;
	
	/* Loop for each structure we need to copy */
	for (mc_element = 0; mc_element < mc_count; mc_element++)
	{
	    /* Look up address in TLB */
	    dcache_TLB_lookup (dcache, sint->stats->dcache, 
			       mc_source, sint->flags, sint);

	    /* See if source address in cache, if is, dont need mem request.
	     * Otherwise, make mem request.
	     */
	    block = S_cache_find_addr (dcache->cache, mc_source);

	    /* If miss, make memory request */
	    if (block == NULL)
	    {
		/* Count the number of memory requests we are making */
		sint->trace.mem_copy_requests++;

		/* Read request if MEM_COPY */
		if (opc_type == MEM_COPY_OPC)
		{
		    M_enqueue(dcache->mc_request_buffer, MC_READ_REQUEST,
			      DCACHE, MEMORY, mc_source, mc_size,
			      sint->serial_no, S_sim_cycle, sint->stats);
		}
		/* Write request if MEM_COPY_BACK */
		else if (opc_type == MEM_COPY_BACK_OPC)
		{
		    M_enqueue(dcache->mc_request_buffer, MC_WRITE_REQUEST,
			      DCACHE, MEMORY, mc_source, mc_size,
			      sint->serial_no, S_sim_cycle, sint->stats);
		}
		/* Make no requests for MEM_COPY_TAG */
		else if (opc_type != MEM_COPY_TAG_OPC)
		    S_punt ("Unknown mem_copy type %i.\n", opc_type);
	    }
	    
	    for (mc_byte = 0; mc_byte < mc_size; mc_byte++)
	    {
		/* Target depends on whether doing mem_copy or copy_back */
		mc_block_addr = mc_buf & start_addr_mask;
		
		/* Detect when we enter a new block */
		if (mc_block_addr != prev_mc_block_addr)
		{
		    /* Update stats */
		    if (opc_type == MEM_COPY_OPC)
		    {
			dstats->mem_copy_blocks++;
		    }
		    else if (opc_type == MEM_COPY_TAG_OPC)
		    {
			dstats->mem_copy_tag_blocks++;
		    }
		    else
		    {
			dstats->mem_copy_back_blocks++;
		    }
		    
		    /* We are placing buffer blocks in cache */
		    /* See if block is in cache */
		    block = S_cache_find_addr (dcache->cache, 
					       mc_block_addr);
		    
		    /* If hit, make block MRU */
		    if (block != NULL)
		    {
			S_cache_make_MRU (dcache->cache, block);

			/* Update mem_copy stats if desired */
			if (S_dcache_measure_conflict_stats)
			{
			    S_update_dcache_conflict_stats (opc_type,
							      dcache, 
							      dstats,
							      mc_block_addr,
							      HIT, 
							      sint->flags);
			    S_add_to_dcache_conflict_cache (dcache, 
							    mc_block_addr);

			}
		    }
		    /* Otherwise, load new block with addr */
		    else
		    {
			/* Update mem_copy stats if desired */
			if (S_dcache_measure_conflict_stats)
			{
			    S_update_dcache_conflict_stats (opc_type,
							      dcache, 
							      dstats,
							      mc_block_addr,
							      MISS, 
							      sint->flags);
			    S_add_to_dcache_conflict_cache (dcache, 
							    mc_block_addr);
			}

			/* Get LRU block for this addr */
			block = S_cache_find_LRU (dcache->cache, 
						  mc_block_addr);

			/* Get block data to test flags */
			data = (Dcache_Data *) block->data;

			/* If block is being used and if MEM_COPY,
			 * update conflict stats.
			 */
			if (block->hash_next != (Scblock *)-1)
			{
			    if (opc_type == MEM_COPY_OPC)
			    {
				dstats->blocks_mem_copies_kicked_out ++;

				if (data->flags & MEM_COPY_BUFFER)
				{
				    dstats->mem_copy_buffer_conflicts ++;

				    if (S_dcache_debug_mem_copy)
				    {
					fprintf (debug_out, 
						 "MEM_COPY  BUFFER CONFLICT buf:%08x with in cache buf:%08x\n", 
						 mc_block_addr, 
						 block->start_addr);
				    }
				}
			    }
			    else if (opc_type == MEM_COPY_TAG_OPC)
			    {
				dstats->blocks_mem_copy_tags_kicked_out ++;

				if (data->flags & MEM_COPY_BUFFER)
				{
				    dstats->mem_copy_tag_buffer_conflicts ++;

				    if (S_dcache_debug_mem_copy)
				    {
					fprintf (debug_out, 
						 "COPY_TAG  BUFFER CONFLICT buf:%08x with in cache buf:%08x\n", 
						 mc_block_addr, 
						 block->start_addr);
				    }
				}
			    }
			    else 
			    {
				if (data->flags & MEM_COPY_BUFFER)
				{
				    dstats->mem_copy_back_buffer_conflicts ++;

				    if (S_dcache_debug_mem_copy)
				    {
					fprintf (debug_out, 
						 "COPY_BACK BUFFER CONFLICT buf:%08x with in cache buf:%08x\n", 
						 mc_block_addr, 
						 block->start_addr);
				    }
				}
			    }
			}
			/* Replace block in cache with mem_copy data */
			replace_dcache_block (dcache, block,
					      mc_block_addr, sint->stats);
			
			/* Make block most recently used in cache line */
			S_cache_make_MRU (dcache->cache, block);

			/* If have MEM_COPY_BACK, need to tie up
			 * bus for the cycles needed to load list block.
			 */
			if (opc_type == MEM_COPY_BACK_OPC)
			{
			    /* Update stats */
			    dstats->mem_copy_back_block_misses ++;


			    /* Do later the requests to fetch memory */
			    
			}

		    }
		    
		    /* Get dcache block data */
		    data = (Dcache_Data *) block->data;
		    
		    /* If mem_copy  mark block as dirty */
		    if (opc_type == MEM_COPY_OPC)
		    {
			data->flags |= DIRTY_BLOCK;
		    }

		    /* Otherwise if copy_back or copy_tag, mark as clean */
		    else
		    {
			data->flags = 0;
		    }

		    /* Mark the block as a MEM_COPY_BUFFER for stat purposes*/
		    data->flags |= MEM_COPY_BUFFER;
		
		    /* Mark the new block we are in */
		    prev_mc_block_addr = mc_block_addr;
		}
		mc_buf++;
		mc_source++;
	    }

	    /* Move the source pointer to the next structure */
	    mc_source += mc_stride_offset;
	}

	/* Update stats */
	if (opc_type == MEM_COPY_OPC)
	{
	    dstats->mem_copy_memory_requests += sint->trace.mem_copy_requests;
	    dstats->mem_copy_cache_requests += mc_count - 
		sint->trace.mem_copy_requests;
	}
	else
	{
	    dstats->mem_copy_back_memory_requests += sint->trace.mem_copy_requests;
	    dstats->mem_copy_back_cache_requests += mc_count - 
		sint->trace.mem_copy_requests;
	}

	/* If have MEM_COPY that has made memory requests, 
	 * then put into pending queue.
	 */
	if ((opc_type == MEM_COPY_OPC) && (sint->trace.mem_copy_requests > 0))
	{
	    S_move_entry_before (dcache->mc_pending_queue, 
				 entry, NULL);
	}
	else
	{
	    /* Otherwise, done with it (and sint) */
	    S_dequeue (entry);
	    S_free_sint (sint);

	    /* Also need to check any pending checks */
	    update_mem_copy_checks(dcache);
	}
    }
}

void S_sim_dcache_first_half_cycle (Pnode *pnode)
{
    Sq_entry 	*entry, *next_entry, *read_request, *write_request;
    Mentry	*mc_request;
    Sq_entry	*prefetch_request, *overlaps;
    Sq_entry	*load_req1, *pref_req1, *load_req2, *pref_req2;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    Scblock *dest_block;
    int block_addr, inx;
    unsigned mask;
#endif
    Scblock	*block, *block2, *other_block = NULL;
    Dcache_Data *data;
    unsigned	request_conflict_mask;
    int		request_addr, last_addr, allocate_this, addr, addr2;
    int		playdoh_flags;
    Sint	*sint, *temp_sint;
    S_Opc_Info	*info;
    Dcache 	*dcache;
    int		load_hit, store_hit, store_combined, initiated_prefetch;
    int		request_combined,write_through;
    int		store_forwarded;
    int		opc_type, source, dest = 0, i, other_dest = 0;
    int		blocks_to_check;
    unsigned 	start_addr_mask,start_addr_mask_pcache = 0,addr_mask;
    Dcache_Stats *dstats;
    Scache	*cache = NULL,*dest_cache = NULL,*other_cache = NULL;
    char 	*cache_text[] = {"","prefetch cache","primary cache","",
					"secondary cache","","","",
					"both prefetch and primary caches"};

    /* Get dcache for ease of use */
    dcache = pnode->dcache;

    /* Prevent dstats from being used before being intialized */
    dstats = NULL;
    dcache->dstats = NULL;

    /* Do perfect cache simulation */
    if (S_dcache_model == DCACHE_MODEL_PERFECT)
    {
	/* Process everything on request queue as a hit */
	for (entry = pnode->dcache_request->head; entry != NULL;
	     entry = next_entry)
	{
	    /* Get next entry before we mess with current entry */
	    next_entry = entry->next_entry;

	    /* Get sint and opc info for entry */
	    sint = entry->sint;
	    info = &opc_info_tab[GET_OPC(sint)];

	    /* Get the stats structure for this sint */
	    dstats = sint->stats->dcache;

	    /* Attempt to handle load */
	    switch (info->opc_type)
	    {
	      case LOAD_OPC:
		/* Set complete time and put in pending queue */
		sint->real_complete_time[S_first_dest] = S_sim_cycle + 
		    sint->oper->real_latency[S_first_dest] -1;

		/* Remove from input queue, also in processor's queue */
		S_dequeue (entry);
		
		/* Updates stats */
		dstats->read_hits++;

		/* Update off path stats */
		if (sint->flags & OFF_PATH)
		{
		    dstats->read_hits_off_path++;
		}

		break;

	      case STORE_OPC:
		/* Request complete, remove from queue and free sint */
		S_dequeue (entry);
		S_free_sint (sint);
		
		/* Update stats */
		dstats->write_hits++;

		break;

	      case PREFETCH_OPC:
		/* Request complete, remove from queue and free sint */
		S_dequeue (entry);
		S_free_sint (sint);
		
		/* Update stats */
		dstats->prefetch_hits++;
		break;

	      case MEM_COPY_TAG_OPC:
	      case MEM_COPY_OPC:
	      case MEM_COPY_BACK_OPC:
	      case MEM_COPY_RESET_OPC:
		/* Request complete, remove from queue and free sint */
		S_dequeue (entry);
		S_free_sint (sint);
		break;

	      case MEM_COPY_CHECK_OPC:
		/* Just return for perfect simulation */
		/* Return mem_copy next cycle */
		entry->sint->real_complete_time[S_first_dest] = S_sim_cycle + 
		    (entry->sint->oper->real_latency[S_first_dest] - 1);
		
		/* Remove from this queue, still in processor's queue */
		S_dequeue (entry);
		break;	    

	      default:
		S_punt ("S_sim_blocking_dcache: unknown opc type %i.",
			info->opc_type);
		break;
	    }
	}
	return;
    }

    /*
     * Bus manager simulation.
     */

    /* Get start_addr_mask for ease of use */
    start_addr_mask = dcache->cache->start_addr_mask;
    if (S_prefetch_cache)
    	start_addr_mask_pcache = dcache->pcache->start_addr_mask;


    /* If bus idle, make an request if possible */
    if (BUS(avail))
    {
	/* Generate any mem_copy/mem_copy_back requests */
	process_pending_mem_copies(dcache);

	/* If there is a pending miss request, make it */
	read_request = dcache->miss_request_buffer->head;
	prefetch_request = dcache->prefetch_request_buffer->head;
	mc_request = dcache->mc_request_buffer->head;
	write_request = dcache->write_buffer->head;

        if (dcache->L2_busy && (read_request || prefetch_request || 
		mc_request || write_request))
	{
	    dstats = pnode->stats->dcache;
	    dstats->cycles_L2_busy_when_needed++;
	}

	else if (read_request != NULL)
	{
	    /* Request block of data for specified cache */
	    if (S_prefetch_cache && 
		    read_request->sint->playdoh_flags & PLAYDOH_TCHS_V1)
	    {
		if ((S_pcache_block_size < S_dcache_block_size) &&
			(read_request->sint->playdoh_flags & PLAYDOH_TCHS_C1))
	    	  BUS_TRANSACTION (READ_REQUEST, DCACHE, S_next_level,
				   read_request->sint->trace.mem_addr &
				   start_addr_mask,
				   S_dcache_block_size,
				   read_request->sint->playdoh_flags,
				   read_request->sint->stats);
		else
	    	  BUS_TRANSACTION (READ_REQUEST, DCACHE, S_next_level,
				   read_request->sint->trace.mem_addr &
				   start_addr_mask_pcache,
				   S_pcache_block_size,
				   read_request->sint->playdoh_flags,
				   read_request->sint->stats);
	    }
	    else if ((read_request->sint->playdoh_flags & PLAYDOH_TCHS_C1)
			|| !(read_request->sint->playdoh_flags & 
						PLAYDOH_TCHS_ALL))
	    	BUS_TRANSACTION (READ_REQUEST, DCACHE, S_next_level,
				   read_request->sint->trace.mem_addr &
				   start_addr_mask,
				   S_dcache_block_size,
				   read_request->sint->playdoh_flags,
				   read_request->sint->stats);
	    else
	    	BUS_TRANSACTION (READ_REQUEST, DCACHE, S_next_level,
				   read_request->sint->trace.mem_addr &
				   S_bypass_addr_mask,
				   S_bypass_req_size,
				   read_request->sint->playdoh_flags,
				   read_request->sint->stats);
	    
	    /* Made request, get rid of it */
	    S_dequeue (read_request);
	}

	/* Otherwise, if there is something in prefetch buffer, make request*/
	else if ((prefetch_request != NULL) && !S_dcache_higher_store_priority)
	{
	    /* Request block of data */
	    if (S_prefetch_cache && 
		  prefetch_request->sint->playdoh_flags & PLAYDOH_TCHS_V1)
	    	BUS_TRANSACTION (PREFETCH_REQUEST, DCACHE, S_next_level,
				   prefetch_request->sint->trace.mem_addr &
				   start_addr_mask_pcache,
				   S_pcache_block_size,
				   prefetch_request->sint->playdoh_flags,
				   prefetch_request->sint->stats);
	    else
	    	BUS_TRANSACTION (PREFETCH_REQUEST, DCACHE, S_next_level,
				   prefetch_request->sint->trace.mem_addr &
				   start_addr_mask,
				   S_dcache_block_size,
				   prefetch_request->sint->playdoh_flags,
				   prefetch_request->sint->stats);
	    
	    /* Made request, move to end of pending prefetches buffer */
	    S_move_entry_before (dcache->pending_prefetches, prefetch_request,
				 NULL);

	    /* Update stats */
	    prefetch_request->sint->stats->dcache->prefetch_requests_sent++;
	}

	/* If mem_copy request have higher priority than write misses */
	else if ((mc_request != NULL) && (!S_dcache_lower_mem_copy_priority))
	{
	    /* Request block of data */
	    BUS_TRANSACTION (mc_request->type, mc_request->src,
				   mc_request->dest, mc_request->addr,
				   mc_request->size,
				   mc_request->playdoh_flags,
				   mc_request->stats);

	    /* Done with request, delete */
	    M_dequeue(mc_request);
	}

	/* Otherwise, if there is something in the store buffer, write it */
	else if (write_request != NULL)
	{
	    if (write_request->sint->oper)
		playdoh_flags = write_request->sint->playdoh_flags;
	    else
		playdoh_flags = 0;
	    /* Send data in write buffer entry */
	    BUS_TRANSACTION (WRITE_REQUEST, DCACHE, S_next_level,
				   write_request->sint->trace.mem_addr,
				   write_request->sint->access_size,
				   playdoh_flags,
				   write_request->sint->stats);

	    /* Finished with store request, dequeue it and release sint */
	    temp_sint = write_request->sint;
	    S_dequeue (write_request);
	    if (!temp_sint->entry_list)
	    	S_free_sint (temp_sint);
	}

	/* Otherwise, if there is something in prefetch buffer, make request*/
	else if ((prefetch_request != NULL) && S_dcache_higher_store_priority)
	{
	    /* Request block of data */
	    if (S_prefetch_cache && 
		  prefetch_request->sint->playdoh_flags & PLAYDOH_TCHS_V1)
	    	BUS_TRANSACTION (PREFETCH_REQUEST, DCACHE, S_next_level,
				   prefetch_request->sint->trace.mem_addr &
				   start_addr_mask_pcache,
				   S_pcache_block_size,
				   prefetch_request->sint->playdoh_flags,
				   prefetch_request->sint->stats);
	    else
	    	BUS_TRANSACTION (PREFETCH_REQUEST, DCACHE, S_next_level,
				   prefetch_request->sint->trace.mem_addr &
				   start_addr_mask,
				   S_dcache_block_size,
				   prefetch_request->sint->playdoh_flags,
				   prefetch_request->sint->stats);
	    
	    /* Made request, move to end of pending prefetches buffer */
	    S_move_entry_before (dcache->pending_prefetches, prefetch_request,
				 NULL);

	    /* Update stats */
	    prefetch_request->sint->stats->dcache->prefetch_requests_sent++;
	}

	/* If mem_copy request have lower priority than write misses */
	else if ((mc_request != NULL) && (S_dcache_lower_mem_copy_priority))
	{
	    /* Request block of data */
	    BUS_TRANSACTION (mc_request->type, mc_request->src,
				   mc_request->dest, mc_request->addr,
				   mc_request->size,
				   mc_request->playdoh_flags,
				   mc_request->stats);

	    /* Done with request, delete */
	    M_dequeue(mc_request);
	}

    }
    
    /* If a block was loaded last cycle, scan the pending 
     * misses to see if any of them can be serviced now.
     */
    last_addr = dcache->addr_loaded_last_cycle;
    if (last_addr != -1)
    {
	addr_mask = ~(S_current_size - 1);
	for (entry = dcache->miss_buffer->head; entry != NULL;
	     entry = next_entry)
	{
	    /* Get next entry before messing with current one */
	    next_entry = entry->next_entry;

	    if ((entry->sint->trace.mem_addr & addr_mask) == last_addr &&
		(S_current_size >= entry->sint->access_size))
	    {
		/* Get the opc_type for the op (load or store) */
		opc_type = opc_info_tab[GET_OPC(entry->sint)].opc_type;

		/* Return if a load, otherwise for stores we are finished */
		if (opc_type == LOAD_OPC)
		{
		    /*
		     * Ready this cycle (or later if latency > 2) 
		     */
		    entry->sint->real_complete_time[S_first_dest] = S_sim_cycle + 
			(entry->sint->oper->real_latency[S_first_dest] -
			 1);
		
		
		    /* Remove from this queue, still in processors queue */
		    S_dequeue (entry);
		}
		else
		{
		    sint = entry->sint;
		    S_dequeue (entry);
		    if (!sint->entry_list)
		    	S_free_sint (sint);
		}
	    }
	}

	/* Mark that we have checked buffer */
	dcache->addr_loaded_last_cycle = -1;

	/* Mark that there is no current dest anymore (no data currently
	 * being transferred.
	 */
	S_current_dest = 0;
	S_current_size = 0;
    }

    /*
     * Return if cache is blocked.
     *
     * This occures if:
     * 1) If the load miss bypass limit has been exceeded.
     * 2) If the store buffer limit has been exceeded.
     * 3) If a mem_copy_check has blocked.
     *
     * Use stats region of instruction causing block.
     */
    if (dcache->miss_buffer->size > S_dcache_miss_bypass_limit) 
    {
	dcache->miss_buffer->tail->sint->stats->dcache->cycles_blocked_miss_buffer++;
	return;
    }
    if (dcache->write_buffer->size > S_dcache_write_buf_size)
    {
	dcache->write_buffer->tail->sint->stats->dcache->cycles_blocked_write_buffer++;
	return;
    }
    if (dcache->mc_check_queue->size > 0)
    {
	dcache->mc_check_queue->tail->sint->stats->dcache->cycles_blocked_mem_copy_check++;
	return;
    }
    if (S_block_until_store_completed && dcache->write_buffer->size)
    {
	dcache->write_buffer->tail->sint->stats->dcache->cycles_blocked_store_completes++;
	return;
    }

    /* Process all of these dcache requests.  This may require exceeding
     * buffer size limits.  The cache then block until the sizes stop
     * exceeding the limit. 
     */
    for (entry = pnode->dcache_request->head; entry != NULL; 
	 entry = next_entry)
    {
	/* Get next entry before we mess with current entry */
	next_entry = entry->next_entry;

	/* Get sint and opc info for entry */
	sint = entry->sint;
	info = &opc_info_tab[GET_OPC(sint)];
	request_addr = sint->trace.mem_addr;
	request_conflict_mask = sint->conflict_mask;

	/* Get the stats structure for this sint */
	dstats = sint->stats->dcache;

	overlaps = NULL;

	request_combined = 0;

	/* Process each type of request */
	switch (info->opc_type)
	{
	  case LOAD_OPC:
	    /* Assume load not a hit */
	    load_hit = 0;

	    /* Assume data not forwarded from a store */
	    store_forwarded = 0;

	    /* Look up address in TLB */
	    dcache_TLB_lookup (dcache, dstats, request_addr, sint->flags, 
			       sint);

	    source = sint->playdoh_flags & PLAYDOH_SCHS_ALL;
	    dest = sint->playdoh_flags & PLAYDOH_TCHS_ALL;

	    /* Check both caches, no matter what source is specified! */

	    /* First check primary cache */
	    block = S_cache_find_addr (dcache->cache, request_addr);
	    cache = dcache->cache;
	    source = PLAYDOH_SCHS_C1;

	    /* Check prefetch cache */
	    if (S_prefetch_cache)
	    {
		other_cache = dcache->pcache;
	    	other_block = S_cache_find_addr (dcache->pcache, request_addr);

		/* make this the source if missed in C1, or dest is V1 */
		if (other_block && (dest == PLAYDOH_SCHS_V1 || !block))
		{
		    /* save old block in a temp */
		    block2 = block;
		    /* set block and cache */
		    block = other_block;
		    cache = other_cache;
		    /* reset other_cache and other_block to C1 entries */
		    other_cache = dcache->cache;
		    other_block = block2;
		    source = PLAYDOH_SCHS_V1;
		}
	    }

            if (!block && S_victim_cache)
            {
                block = S_cache_find_addr (vcache, request_addr);
                if (block)
                {
                    cache = vcache;
                    dstats->vcache_hits++;
                }
            }

	    /* See if a target cache is specified, if not, set to source */
	    if (!dest)
	    {
		dest = source*16;
		dest_cache = cache;
	    }

	    /* Set dest to primary cache if that is the target specified */
	    else if (!S_prefetch_cache || dest != PLAYDOH_TCHS_V1)
	    {
		dest_cache = dcache->cache;
		dest = PLAYDOH_TCHS_C1;
	    }

	    /* Set dest to prefetch cache if V1 is target cache */
	    else if (S_prefetch_cache && dest == PLAYDOH_TCHS_V1)
	    {
		dest_cache = dcache->pcache;
		dest = PLAYDOH_TCHS_V1;
	    }

	    if (S_prefetch_cache)
	    {
	      if (source == PLAYDOH_SCHS_C1)
		dstats->load_source_C1++;
	      else if (source == PLAYDOH_SCHS_V1)
		dstats->load_source_V1++;
	      if (dest == PLAYDOH_TCHS_C1)
		dstats->load_dest_C1++;
	      else if (dest == PLAYDOH_TCHS_C3)
		dstats->load_dest_C3++;
	      else if (dest == PLAYDOH_TCHS_V1)
		dstats->load_dest_V1++;

	      if (sint->playdoh_flags & PLAYDOH_TCHS_C1)
	      {
		if (block && source & PLAYDOH_SCHS_C1)
		    dstats->load_C1_hit_C1++;
		else if (block && source & PLAYDOH_SCHS_V1)
		    dstats->load_C1_hit_V1++;
		else
		    dstats->load_C1_miss++;
	      }
	      else if (sint->playdoh_flags & PLAYDOH_TCHS_V1)
	      {
		if (block && source & PLAYDOH_SCHS_C1)
		    dstats->load_V1_hit_C1++;
		else if (block && source & PLAYDOH_SCHS_V1)
		    dstats->load_V1_hit_V1++;
		else
		    dstats->load_V1_miss++;
	      }
	      else if (sint->playdoh_flags & PLAYDOH_TCHS_C3)
	      {
		if (block && source & PLAYDOH_SCHS_C1)
		    dstats->load_C3_hit_C1++;
		else if (block && source & PLAYDOH_SCHS_V1)
		    dstats->load_C3_hit_V1++;
		else
		    dstats->load_C3_miss++;
	      }
	    }

	    /* If hit, make block MRU */
	    if (block != NULL)
	    {
                if (cache == vcache && S_victim_cache)
                {
                    if (S_vcache_block_size != S_dcache_block_size)
                        S_punt("dcache victim cache and dcache block sizes are different!\n");

                    block2 = S_cache_find_LRU (dcache->cache, request_addr);
                    addr2 = block2->start_addr;
                    S_cache_change_addr (dcache->cache, block2, request_addr);
                    S_cache_change_addr (vcache, block, addr2);
                    cache = dcache->cache;
                    block = block2;
                }

		/* Make most recently use */
		S_cache_make_MRU (cache, block);
		
		/* Mark as hit */
		load_hit = 1;

                data = (Dcache_Data *)block->data;

		/* If prefetch bit set, reset bit and and remove any
		 * prefetches in prefetch buffer that has the
		 * same index as the current access.
		 */
		cancel_prefetch (dcache, block, sint, cache);

		/* if the target cache is C3 then make block LRU */
		if (sint->playdoh_flags & PLAYDOH_TCHS_C3)
		{
		    S_cache_make_LRU(cache,block);
		    /* if also hit in other cache, make its entry LRU as well */
		    if (S_prefetch_cache && other_block)
		    	S_cache_make_LRU(other_cache,other_block);
		}

		/* Update conflict stats if desired, do here so
		 * conflict cache has a chance.  The can also get
		 * hits due to forwarding, but do not model this
		 * for conflicts
		 */
		if (S_dcache_measure_conflict_stats && 
						source == PLAYDOH_SCHS_C1)
		{
		    S_update_dcache_conflict_stats (LOAD_OPC,
						    dcache, dstats,
						    sint->trace.mem_addr,
						    HIT, 
						    sint->flags);
		    S_add_to_dcache_conflict_cache (dcache, 
						    sint->trace.mem_addr);

		}
	    }

	    /* Otherwise, see if can forward from write buffer */
	    else
	    {
		/* Search for most recent store that overlaps load */
		overlaps = find_overlapping_entry (dcache->write_buffer,
						   sint->trace.mem_addr,
						   sint->conflict_mask);

		/* If have overlapping store, forward if store's data 
		 * size is at least as big as the load's (conflict_mask
		 * will be as least as small as the loads).
		 */
		if ((overlaps != NULL) &&
		    (overlaps->sint->conflict_mask <= request_conflict_mask))
		{
		    /* Can forward, update stats */
		    dstats->reads_forwarded++;

		    if (sint->flags & OFF_PATH)
		    {
			dstats->reads_forwarded_off_path++;
		    }

		    store_forwarded = 1;

		    /* Mark as hit */
		    load_hit = 1;
		}
	    }

	    /* Process hits */
	    if (load_hit)
	    {
		/* Set complete time and put in pending queue */
		sint->real_complete_time[S_first_dest] = S_sim_cycle + 
		    sint->oper->real_latency[S_first_dest] -1;
		
		/* Remove from input queue, also in processor's queue */
		S_dequeue (entry);
		/* Updates stats */
		dstats->read_hits++;

		if (S_dcache_debug_misses)
		{
		    if (store_forwarded)
			fprintf (debug_out, "(forwarded) ");

		    fprintf (debug_out, "%d: LD HIT %s op %i  %x in %s",
			     S_sim_cycle,
			     sint->oper->cb->fn->name, 
			     sint->oper->lcode_id,
			     sint->trace.mem_addr,
			     cache_text[source]);
		    if (sint->flags & OFF_PATH)
			fprintf (debug_out, "  (OFF PATH)\n");
		    else
			fprintf (debug_out, "\n");
		}
		
		
		/* Update off path stats */
		if (sint->flags & OFF_PATH)
		{
		    dstats->read_hits_off_path++;		
		}
	    }

	    /* Process misses in target cache */
	    else
	    {
		/* Load data into specified target cache */
	    	cache = dest_cache;

		sint->flags |= DCACHE_MISS;

		/* if there was an overlapping entry in the store
		 * buffer and was not forwarded, we must block
		 * the processor until the store is complete (just
		 * wait until store buffer drains completely).
	 	 */
		if (overlaps)
		{
		    S_block_until_store_completed = 1;
		    pnode->dcache_busy = 1;
		    break;
		}

		S_dequeue (entry);

		/* 
		 * Add a request to fetch the missed block if there
		 * no other pending load misses to this block.
		 *
		 * This will prevent issuing two requests for
		 * the same block (but it will still be counted as
		 * two pending misses).
		 */
		if (!S_find_outstanding_request (dcache, sint))
		{
		    S_enqueue (dcache->miss_request_buffer,
			       sint);

		    if (S_prefetch_cache)
		    {
		      /* Check if dirty in other cache */
		      /* only need to do this if !load_hit ? */
		      if (dest == PLAYDOH_TCHS_C1)
		      {
			addr = request_addr & cache->start_addr_mask;
			if (S_dcache_block_size > S_pcache_block_size) 
			    blocks_to_check = S_dcache_block_size/S_pcache_block_size;
			else
			    blocks_to_check = 1;
			for (i=0;i<blocks_to_check;i++)
			{
			  if ((block2 = S_cache_find_addr(dcache->pcache,addr)))
			    {
			    	data = (Dcache_Data *)block2->data;
			    	if (data->flags & DIRTY_BLOCK)
				{
				    writeback_pcache_block(dcache,block2,
							sint->stats);
				}
			    }
			    addr += S_pcache_block_size;
			}
		      }
		      else if (dest == PLAYDOH_TCHS_V1)
		      {
			addr = request_addr & cache->start_addr_mask;
			if (S_pcache_block_size > S_dcache_block_size) 
			    blocks_to_check = S_pcache_block_size/S_dcache_block_size;
			else
			    blocks_to_check = 1;
			for (i=0;i<blocks_to_check;i++)
			{
			  if ((block2 = S_cache_find_addr(dcache->cache,addr)))
			    {
			    	data = (Dcache_Data *)block2->data;
			    	if (data->flags & DIRTY_BLOCK)
				{
				    /* old stuff */
				    writeback_dcache_block(dcache,block2,
							sint->stats);
				}
			    }
			    addr += S_dcache_block_size;
			}
		      }
		    }
		    
		    /* Update stats for new request.
		     * Depending on model, a redundant request could
		     * be considered a hit (if blocking cache)
		     * or a redundant miss (if non-blocking cache).
		     * This is handled in else below.
		     */
		    if (!load_hit)
		    {
		    	dstats->read_misses++;
			/* HCH 10-18-99: check to see how many 
			   cache misses are caused by speculative loads */
			if (sint->oper->flags & SPECULATIVE)
			{
			  dstats->speculative_read_misses++;
			}

		    	/* Update off path stats */
		    	if (sint->flags & OFF_PATH)
		    	{
			    dstats->read_misses_off_path++;	
		    	}
		    }
		    
		    /* Update conflict stats if desired */
		    if (S_dcache_measure_conflict_stats &&
			(dest == PLAYDOH_TCHS_C1))
		    {
			S_update_dcache_conflict_stats (LOAD_OPC,
						dcache, dstats,
						sint->trace.mem_addr,
						MISS, sint->flags);
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
		    if (S_dcache_model == DCACHE_MODEL_BLOCKING && !load_hit)
		    {
                        dstats->read_hits++;

                        /* Update off path stats */
                        if (sint->flags & OFF_PATH)
                        {
                            dstats->read_hits_off_path++;
                        }
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
			if (S_dcache_measure_conflict_stats &&
				(dest == PLAYDOH_TCHS_C1))
			{
			    S_update_dcache_conflict_stats (LOAD_OPC,
							    dcache, dstats,
							    sint->trace.mem_addr,
							    MISS, 
							    sint->flags);
			    
			}

			if (!load_hit)
			{
			    /* Updates stats */
                            dstats->read_misses++;
                            dstats->redundant_read_misses++;

			/* HCH 10-18-99: check to see how many 
			   cache misses are caused by speculative loads */
			if (sint->oper->flags & SPECULATIVE)
			{
			  dstats->speculative_read_misses++;
			}

                            if (sint->flags & OFF_PATH)
                            {
                            	dstats->read_misses_off_path++;
                            	dstats->redundant_read_misses_off_path++;
                            }
                        }
		    }
		}

		/* Put entry in pending miss queue */
		S_enqueue(dcache->miss_buffer,sint);
		
		if (S_dcache_debug_misses && !load_hit)
		{
		    if (request_combined)
			fprintf (debug_out, "(request_combined) ");

		    fprintf (debug_out, "%d: LD miss %s op %i  %x in %s",
			     S_sim_cycle,
			     sint->oper->cb->fn->name, 
			     sint->oper->lcode_id,
			     sint->trace.mem_addr,
			     cache_text[dest/16]);
		    if (sint->flags & OFF_PATH)
			fprintf (debug_out, "  (OFF PATH)\n");
		    else
			fprintf (debug_out, "\n");
		}
	    }
	    break;

	   
	    /* 
	     * This prefetch algorithm designed for direct mapped cache.
	     * It should work for higher assoc, but only one prefetch
	     * per cache line will be allowed.
	     */
	  case PREFETCH_OPC:

	    /* If ignoring prefetches (for debugging purposes), ignore it */
	    if (S_dcache_ignore_prefetches)
	    {
		S_dequeue (entry);
		S_free_sint (sint);
		break;
	    }

	    /* Look up address in TLB (for now) */
	    dcache_TLB_lookup (dcache, dstats, request_addr, sint->flags, 
			       sint);

	    /* Assume that we will not initiate prefetch */
	    initiated_prefetch = 0;

	    /*** Assume for now that if we want to prefetch into
	     *** a specific cache, we will prefetch into it
	     *** regardless of whether the data exists in the other cache 
	     ***/

	    /* See if block is in cache */
	    if (!S_prefetch_cache || 
		!(sint->playdoh_flags & PLAYDOH_TCHS_V1))
	    {
	       block = S_cache_find_addr (dcache->cache, request_addr);
	       cache = dcache->cache;
	       dest = PLAYDOH_TCHS_C1;
	    }
	    else if (S_prefetch_cache && 
		(sint->playdoh_flags & PLAYDOH_TCHS_V1 ||
		(!block && !(sint->playdoh_flags & PLAYDOH_TCHS_ALL))))
	    {
	       block = S_cache_find_addr (dcache->pcache, request_addr);
	       if (block || 
		   (S_prefetch_cache && sint->playdoh_flags & PLAYDOH_TCHS_V1))
	       {
	       		cache = dcache->pcache;
			dest = PLAYDOH_TCHS_V1;
	       }
	    }

	    /* If hit, done */
	    if (block != NULL)
	    {
		/* Update stats */
		dstats->prefetch_hits++;

		if (sint->flags & OFF_PATH)
		{
		    dstats->prefetch_hits_off_path++;
		}

		if (S_dcache_debug_prefetch)
		{
		    fprintf (debug_out,
			     "%d: PREFETCH hit op %i addr %x in %s\n",
			     S_sim_cycle,
			     sint->oper->lcode_id,
			     sint->trace.mem_addr,
			     cache_text[dest/16]);
		}
	    }

	    /* Otherwise, start prefetch if this cache block is not already
	     * being prefetched.
	     */
	    else
	    {
		/* Get the LRU block for this address */
		block = S_cache_find_LRU (cache, request_addr);

		/* Get block's data */
		data = (Dcache_Data *) block->data;

		/* If block already has prefetch bit set,
		 * don't do prefetch.
		 */
		if (data->flags & PREFETCH_BIT)
		{
		    /* Update stats */
		    dstats->redundant_prefetches++;
		    
		    if (sint->flags & OFF_PATH)
		    {
			dstats->redundant_prefetches_off_path++;
		    }

		    if (S_dcache_debug_prefetch)
		    {
			fprintf (debug_out,
				 "PREFETCH redundant op %i addr %x\n",
				 sint->oper->lcode_id,
				 sint->trace.mem_addr);
		    }

		}

		/* Otherwise, if prefetch buf full, don't do prefetch */
		else if (dcache->prefetch_request_buffer->size >= 
			 S_dcache_prefetch_buf_size)
		{
		    /* Updates stats */
		    dstats->prefetch_buf_full++;

		    if (sint->flags & OFF_PATH)
		    {
			dstats->prefetch_buf_full_off_path++;
		    }

		    if (S_dcache_debug_prefetch)
		    {
			fprintf (debug_out,
				 "PREFETCH buf full op %i addr %x\n",
				 sint->oper->lcode_id,
				 sint->trace.mem_addr);
		    }
		}

		/* Otherwise, if have an outstanding request for this
		 * block, then don't do prefetch (treat as hit)
		 */
		/* TLJ: added 'else' */
		else if (S_find_outstanding_request (dcache, sint))
		{
		    /* Update stats */
		    dstats->prefetch_hits++;
		    
		    if (sint->flags & OFF_PATH)
		    {
			dstats->prefetch_hits_off_path++;
		    }
		    
		    if (S_dcache_debug_prefetch)
		    {
			fprintf (debug_out,
				 "PREFETCH hit (being loaded) op %i addr %x\n",
				 sint->oper->lcode_id,
				 sint->trace.mem_addr);
		    }
		}
		
		/* Otherwise, do prefetch */
		else
		{
		    /* Set prefetch bit on block */
		    data->flags |= PREFETCH_BIT;

		    /* Update stats */
		    dstats->prefetch_misses ++;

		    if (sint->flags & OFF_PATH)
		    {
			dstats->prefetch_misses_off_path++;
		    }

		    /* Move entry to end of pending prefetch queue */
		    S_move_entry_before (dcache->prefetch_request_buffer, 
					entry, NULL);

		    /* Mark that we initialized prefetch */
		    initiated_prefetch = 1;

		    if (S_dcache_debug_prefetch)
		    {
			fprintf (debug_out,
				 "%d: PREFETCH miss op %i addr %x in %s\n",
				 S_sim_cycle,
				 sint->oper->lcode_id,
				 sint->trace.mem_addr,
				 cache_text[dest/16]);
		    }

		}
	    }

	    /* If didn't initiate prefetch, done with prefetch, throw away */
	    if (!initiated_prefetch)
	    {
		S_dequeue (entry);
		S_free_sint (sint);
	    }
	    
	    break;


	  case STORE_OPC:
	    /* Look up address in TLB */
	    dcache_TLB_lookup (dcache, dstats, request_addr, sint->flags, 
			       sint);

	    /* Assume store miss and not combined */
	    store_hit = 0;
	    store_combined = 0;
	    write_through = 0;

	    /*** Assume for now that we only want to write to
	     *** the specified cache. Invalidate entry in other
	     *** cache if it exists. Also assume that if we
	     *** have specified a cache as the target, we will
	     *** allocate even if not a write-allocate cache.
	     ***/

	    /* For now, check both caches and only use target specifier 
	     * if misses in both caches!
	     */
	    block = S_cache_find_addr (dcache->cache, request_addr);
	    cache = dcache->cache;
	    dest = PLAYDOH_TCHS_C1;
	    /* Check prefetch cache */
	    if (S_prefetch_cache)
	    {
		other_cache = dcache->pcache;
	    	other_block = S_cache_find_addr (other_cache, request_addr);

		/* make this the source if missed in C1, or dest is V1 */
		if ((other_block && !block) ||
			((other_block || !block) && 
			sint->playdoh_flags & PLAYDOH_TCHS_V1))
		{
		    /* save old block in a temp */
		    block2 = block;
		    /* set block and cache */
		    block = other_block;
		    cache = other_cache;
		    /* reset other_cache and other_block to C1 entries */
		    other_cache = dcache->cache;
		    other_block = block2;
		    dest = PLAYDOH_TCHS_V1;
		}
	    }

            if (!block && S_victim_cache)
            {
                block = S_cache_find_addr (vcache, request_addr);
                if (block)
                {
                    cache = vcache;
                }
            }

	    if (!block) dest = sint->playdoh_flags & PLAYDOH_TCHS_ALL;

	    if (S_prefetch_cache)
	    {
	      if (dest == PLAYDOH_TCHS_C1)
		dstats->store_dest_C1++;
	      if (dest == PLAYDOH_TCHS_C3)
		dstats->store_dest_C3++;
	      else if (dest == PLAYDOH_TCHS_V1)
		dstats->store_dest_V1++;

	      if (sint->playdoh_flags & PLAYDOH_TCHS_C1)
	      {
		if (block && dest & PLAYDOH_TCHS_C1)
		    dstats->store_C1_hit_C1++;
		else if (block && dest & PLAYDOH_TCHS_V1)
		    dstats->store_C1_hit_V1++;
		else
		    dstats->store_C1_miss++;
	      }
	      else if (sint->playdoh_flags & PLAYDOH_TCHS_V1)
	      {
		if (block && dest & PLAYDOH_TCHS_C1)
		    dstats->store_V1_hit_C1++;
		else if (block && dest & PLAYDOH_TCHS_V1)
		    dstats->store_V1_hit_V1++;
		else
		    dstats->store_V1_miss++;
	      }
	      else if (sint->playdoh_flags & PLAYDOH_TCHS_C3)
	      {
		if (block && dest & PLAYDOH_TCHS_C1)
		    dstats->store_C3_hit_C1++;
		else if (block && dest & PLAYDOH_TCHS_V1)
		    dstats->store_C3_hit_V1++;
		else
		    dstats->store_C3_miss++;
	      }
	    }

	    /* If we have specified a target cache, do write allocation */
	    if (S_dcache_write_allocate || 
			dest == PLAYDOH_TCHS_C1 || dest == PLAYDOH_TCHS_V1)
		allocate_this = 1;
	    else allocate_this = 0;

	    /* If we have specified a target cache, invalidate entry
	       in other cache if it exists. (If dest == C3 then 
	       we have already checked both other caches so don't
	       need to worry about them) */
	    if (S_prefetch_cache)
	    {
	      if (dest == PLAYDOH_TCHS_C1 &&
		    (block2 = S_cache_find_addr (dcache->pcache, request_addr)))
	      {
		addr = request_addr & cache->start_addr_mask;
		if (S_dcache_block_size > S_pcache_block_size) 
		    blocks_to_check = S_dcache_block_size/S_pcache_block_size;
		else
		    blocks_to_check = 1;
		for (i=0;i<blocks_to_check;i++)
		{
		  if ((block2 = S_cache_find_addr(dcache->pcache, addr)))
		    {
	        	data = (Dcache_Data *) block2->data;
			if (data->flags & DIRTY_BLOCK)
			{
			    if (!(sint->flags & WRITE_THROUGH))
			    	dstats->written_through++;
			    sint->flags |= WRITE_THROUGH;
			fprintf(stdout,"write through 1\n");
			}
			replace_pcache_block (dcache, block2, -1, sint->stats);
		    }
		    addr += S_pcache_block_size;
		}
	      }
	      else if (dest == PLAYDOH_TCHS_V1)
	      {
		addr = request_addr & cache->start_addr_mask;
		if (S_pcache_block_size > S_dcache_block_size) 
		    blocks_to_check = S_pcache_block_size/S_dcache_block_size;
		else
		    blocks_to_check = 1;
		for (i=0;i<blocks_to_check;i++)
		{
		  if ((block2 = S_cache_find_addr(dcache->cache, addr)))
		    {
	        	data = (Dcache_Data *) block2->data;
			if (data->flags & DIRTY_BLOCK)
			{
			    if (!(sint->flags & WRITE_THROUGH))
			    	dstats->written_through++;
			    sint->flags |= WRITE_THROUGH;
			fprintf(stdout,"write through 2\n");
			}
			replace_dcache_block (dcache, block2, -1, sint->stats);
		    }
		    addr += S_dcache_block_size;
		}
	      }
	    }
	    
	    /* If hit, mark block as dirty and make MRU */
	    if (block != NULL)
	    {
                if (cache == vcache && S_victim_cache)
                {
                    if (S_vcache_block_size != S_dcache_block_size)
                        S_punt("Victim cache and dcache block sizes are different!\n");

                    block2 = S_cache_find_LRU (dcache->cache, request_addr);
                    addr2 = block2->start_addr;
                    S_cache_change_addr (dcache->cache, block2, request_addr);
                    S_cache_change_addr (vcache, block, addr2);
                    cache = dcache->cache;
                    block = block2;
                }

		/* Get dcache block data */
		data = (Dcache_Data *) block->data;

		/* if the target cache is C3 then make block LRU */
		if (sint->playdoh_flags & PLAYDOH_TCHS_C3)
		{
		    S_cache_make_LRU(cache,block);
		    /* if hit in other cache, make it LRU as well */
		    if (S_prefetch_cache && other_block)
		    	S_cache_make_LRU(other_cache,other_block);
		}

		/* If we have a prefetch cache we must make sure
		 * there are no outstanding overlapping writes, if
		 * so then block must be put into store buffer and
		 * becomes clean (write through).
		 */
		if (S_prefetch_cache)
		{
		    overlaps = NULL;
	    	    if (S_dcache_combining_write_buf)
	    	    {
			/* Search for overlapping store */
			overlaps = find_overlapping_entry (dcache->write_buffer,
						   sint->trace.mem_addr,
						   sint->conflict_mask);

			/* If have overlapping store, combine if overlap's data 
			 * size is at least as big as this store's(conflict_mask
			 * will be as least as small as this store's).
			 */
			if ((overlaps != NULL) &&
			    		(overlaps->sint->conflict_mask <= 
							sint->conflict_mask))
			    store_combined = 1;

			else if (overlaps) /* Place in store buffer */
			{
			    if (!(sint->flags & WRITE_THROUGH))
			    	dstats->written_through++;
			    fprintf(stdout,"write through 3\n");
			    sint->flags |= WRITE_THROUGH;
		    	    S_move_entry_before
					(dcache->write_buffer, entry, NULL);
			}

			/* If there is an overlapping entry to other cache, 
			 * will need to write both entries through since 
			 * there has now been another store to this address.
			 */
			if (dest == PLAYDOH_TCHS_V1)
			    other_dest = PLAYDOH_TCHS_C1;
			else if (dest == PLAYDOH_TCHS_C1)
			    other_dest = PLAYDOH_TCHS_V1;
			/* Also write overlapping entry through */
			if (overlaps && find_overlapping_entry_and_target(
				other_dest,dcache->write_buffer,
				sint->trace.mem_addr,
				dcache->pcache->start_addr_mask))
			{
    			    for (overlaps=dcache->write_buffer->head;
				overlaps!=NULL;overlaps=overlaps->next_entry)
    			    {
				addr_mask = dcache->pcache->start_addr_mask & 
						overlaps->sint->conflict_mask;
				if ((sint->trace.mem_addr & addr_mask) ==
	    			   (overlaps->sint->trace.mem_addr & addr_mask))
		    		if (!(overlaps->sint->flags & WRITE_THROUGH))
		    		{
		    		    dstats->written_through++;
		    		    overlaps->sint->flags |= WRITE_THROUGH;
		    		}
			    }

			    /* note that we have caused another store */
			    dstats->written_through_due_to_store++;
			    write_through = 1;
			}
		    }

		    /* Check if there is an outstanding load miss request
			from the other cache */
		    if (!overlaps)
		    {
			load_req1 = pref_req1 = load_req2 = pref_req2 = NULL;
			if (dest != PLAYDOH_TCHS_V1)
			{
			    load_req1 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_V1,dcache->miss_buffer,
				sint->trace.mem_addr,
				dcache->pcache->start_addr_mask);
			    if (!load_req1)
			      pref_req1 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_V1,dcache->pending_prefetches,
				sint->trace.mem_addr,
				dcache->pcache->start_addr_mask);
			}
			else if (dest != PLAYDOH_TCHS_C1)
			{
			    if (dest == PLAYDOH_TCHS_V1)
				addr_mask = dcache->pcache->start_addr_mask;
			    else
				addr_mask = dcache->cache->start_addr_mask;
			    load_req2 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_C1,dcache->miss_buffer,
				sint->trace.mem_addr,addr_mask);
			    if (!load_req2)
			      pref_req2 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_C1,dcache->pending_prefetches,
				sint->trace.mem_addr,addr_mask);
			}
			if (((load_req1 || pref_req1 || load_req2 || pref_req2)
				&& (dest != PLAYDOH_TCHS_C3)) ||
			    	((load_req1 || pref_req1) && 
				 (load_req2 || pref_req2) && 
				 (dest == PLAYDOH_TCHS_C3)))
			{
			    if (!(sint->flags & WRITE_THROUGH))
			    	dstats->written_through++;
			    sint->flags |= WRITE_THROUGH;
		    	    S_move_entry_before(dcache->write_buffer,entry,
									NULL);
			    dstats->written_through_due_to_load++;
			    write_through = 1;
			}
			/* no overlapping writes, loads or prefetches so
			 * the block becomes dirty.
			 */
			else
			{
			    /* Mark as dirty */
			    data->flags |= DIRTY_BLOCK;
			}
		    }
		}

		else /* no prefetch cache */
		{
		    /* Mark as dirty */
		    data->flags |= DIRTY_BLOCK;
		}
		
		/* Update conflict stats if desired */
		if (S_dcache_measure_conflict_stats)
		{
		  if (dest & PLAYDOH_TCHS_C1)
		  {
		    if (allocate_this)
		    {
			S_update_dcache_conflict_stats (STORE_OPC,
							dcache, dstats,
							sint->trace.mem_addr,
							HIT, sint->flags);
			S_add_to_dcache_conflict_cache (dcache, 
							sint->trace.mem_addr);

		    }
		    /* If not write allocate, just make MRU */
		    else
		    {
			S_make_dcache_conflict_cache_MRU(dcache, 
							 sint->trace.mem_addr);
		    }
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
		cancel_prefetch (dcache, block, sint, cache);
	    }
		
	    /* Otherwise, try to combine with existing store (if allowed)*/
	    else if (S_dcache_combining_write_buf)
	    {
		/* Search for overlapping store */
		overlaps = find_overlapping_entry (dcache->write_buffer,
						   sint->trace.mem_addr,
						   sint->conflict_mask);

		/* If have overlapping store, combine if overlap's data 
		 * size is at least as big as this store's(conflict_mask
		 * will be as least as small as this store's).
		 */
		if ((overlaps != NULL) &&
		    (overlaps->sint->conflict_mask <= sint->conflict_mask))
		{
		    /* Can combine, possibly treat as hit */
		    store_combined = 1;
		    dstats->writes_combined++;
		    store_hit = 1;
		}

		/* If there is an overlapping entry to other cache, will need 
		 * to write both entries through since there has now been 
		 * another store to this address.
		 */
		if (dest == PLAYDOH_TCHS_V1)
		{
		    other_dest = PLAYDOH_TCHS_C1;
		}
		else if (dest == PLAYDOH_TCHS_C1)
		{
		    other_dest = PLAYDOH_TCHS_V1;
		}
		if (S_prefetch_cache && overlaps && 
			find_overlapping_entry_and_target(
				other_dest,dcache->write_buffer,
				sint->trace.mem_addr,
				dcache->pcache->start_addr_mask))
		{
    		    for (overlaps=dcache->write_buffer->head;overlaps!=NULL;
						overlaps=overlaps->next_entry)
    		    {
			addr_mask = dcache->pcache->start_addr_mask & 
						overlaps->sint->conflict_mask;
			if ((sint->trace.mem_addr & addr_mask) ==
	    		    (overlaps->sint->trace.mem_addr & addr_mask))
		    	if (!(overlaps->sint->flags & WRITE_THROUGH))
		    	{
		    	    dstats->written_through++;
		    	    overlaps->sint->flags |= WRITE_THROUGH;
		    	}
		    }
		    if (!(sint->flags & WRITE_THROUGH))
		    {
		    	dstats->written_through++;
		    	sint->flags |= WRITE_THROUGH;
		    }
		    write_through = 1;
		    dstats->written_through_due_to_store++;
		}
	    }

	    /* Process hits */
	    if (store_hit)
	    {
		/* Update stats */
		dstats->write_hits++;

		if (S_dcache_debug_misses)
		{
		    if (store_combined)
			fprintf (debug_out, "(combined) ");

		    if (write_through)
			fprintf (debug_out, "(write_through) ");

		    fprintf (debug_out, "%d: ST HIT %s op %i  %x in %s\n",
			     S_sim_cycle,
			     sint->oper->cb->fn->name, 
			     sint->oper->lcode_id,
			     sint->trace.mem_addr,
			     cache_text[dest/16]);
		}
		
		/* Request complete, remove from queue and free sint */
		S_dequeue (entry);
		S_free_sint (sint);
	    }

	    /* Process misses */
	    else
	    {
		/* Move to write buffer for writing if not combined */
		if (!store_combined)
		    S_move_entry_before (dcache->write_buffer, entry, NULL);

		/*
		 * If no write allocate protocol, was put in write buffer
		 * to be written to memory.
		 */
		if (!allocate_this)
		{
		    /* Update stats */
		    dstats->write_misses++;
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
		    if (S_dcache_measure_conflict_stats &&
		  				dest & PLAYDOH_TCHS_C1)
		    {
			S_update_dcache_conflict_stats (STORE_OPC,
							dcache, dstats,
							sint->trace.mem_addr,
							MISS, sint->flags);
			/* Add to conflict cache when comes back from memory*/
		    }

		    if (!S_find_outstanding_request (dcache, sint))
		    {
			S_enqueue (dcache->miss_request_buffer, sint);

			/* Update stats for new request.
			 * Depending on model, a redundant request could
			 * be considered a hit (if blocking cache)
			 * or a redundant miss (if non-blocking cache).
			 * This is handled below in else.
			 */
			dstats->write_misses++;

			/* Clear any prefetch bits for this line, and clear
			 * any prefetch requests that have not gone to memory
			 * for this line.
			 */
			block = S_cache_find_LRU (cache, request_addr);
			cancel_prefetch (dcache, block, sint, cache);
		    }
		    else
		    {
			if (S_dcache_model == DCACHE_MODEL_BLOCKING)
			{
			    dstats->write_hits++;
			}
			else
			{
			    dstats->write_misses++;
 			}
			request_combined = 1;
		    }
		    /* Move entry to end pending miss queue */
		    if (store_combined)
			/* moves it out of write_buffer */
		    	S_move_entry_before (dcache->miss_buffer, entry, NULL);
		    else
			/* want to leave it in write_buffer */
			S_enqueue(dcache->miss_buffer,entry->sint);
		}

		if (S_prefetch_cache)
		{
		    /* Check if there is an outstanding load miss request
			from the other cache (or from both C1 and V1
			if the target of this store is C3) */
		    load_req1 = pref_req1 = load_req2 = pref_req2 = NULL;
		    if (dest != PLAYDOH_TCHS_V1)
		    {
			load_req1 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_V1,dcache->miss_buffer,
				sint->trace.mem_addr,
				dcache->pcache->start_addr_mask);
			if (!load_req1)
			  pref_req1 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_V1,dcache->pending_prefetches,
				sint->trace.mem_addr,
				dcache->pcache->start_addr_mask);
		    }
		    else if (dest != PLAYDOH_TCHS_C1)
		    {
			if (dest == PLAYDOH_TCHS_V1)
			    addr_mask = dcache->pcache->start_addr_mask;
			else
			    addr_mask = dcache->cache->start_addr_mask;
			load_req2 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_C1,dcache->miss_buffer,
				sint->trace.mem_addr,addr_mask);
			if (!load_req2)
			  pref_req2 = find_overlapping_entry_and_target(
				PLAYDOH_TCHS_C1,dcache->pending_prefetches,
				sint->trace.mem_addr,addr_mask);
		    }
		    /* If there are now requests from both caches
		     * we need to write the store data through so that
		     * both cache copies will be clean and coherent.
		     */
		    if (((load_req1 || pref_req1 || load_req2 || pref_req2)
                                && (dest != PLAYDOH_TCHS_C3)) ||
                                ((load_req1 || pref_req1) &&
                                 (load_req2 || pref_req2) &&
                                 (dest == PLAYDOH_TCHS_C3)))
		    {
		    	if (!(sint->flags & WRITE_THROUGH))
		    	    dstats->written_through++;
			sint->flags |= WRITE_THROUGH;
			dstats->written_through_due_to_load++;
			write_through = 1;
		    }
		}

		if (S_dcache_debug_misses)
		{
		    if (store_combined)
			fprintf (debug_out, "(combined) ");

		    if (request_combined)
			fprintf (debug_out, "(request_combined) ");

		    if (write_through)
			fprintf (debug_out, "(write_through)");

		    fprintf (debug_out, "%d: ST miss %s op %i  %x in %s\n",
			     S_sim_cycle,
			     sint->oper->cb->fn->name, 
			     sint->oper->lcode_id,
			     sint->trace.mem_addr,
			     cache_text[dest/16]);
		}
	    }
	    break;

	  case MEM_COPY_TAG_OPC:
	  case MEM_COPY_OPC:
	  case MEM_COPY_BACK_OPC:
	    /* Move to mc_instr_queue */
	    S_move_entry_before (dcache->mc_instr_queue, entry, NULL);
	    break;

	  case MEM_COPY_RESET_OPC:
	    S_punt ("MEM_COPY_RESET_OPC: not handled yet in simulator.");
	    break;

	  case MEM_COPY_CHECK_OPC:
	    /* If mem_copy finished, return check write away */
	    if (is_mem_copy_complete (dcache, sint->trace.mem_copy_buf))
	    {
		if (S_dcache_debug_mem_copy)
		{
		    fprintf (debug_out, 
			     "COPY_CHECK SUCCESSFUL for buf:%08x op %i\n",
			     sint->trace.mem_copy_buf, sint->oper->lcode_id);
		}
		/* Return mem_copy next cycle */
		entry->sint->real_complete_time[S_first_dest] = S_sim_cycle + 
		    (entry->sint->oper->real_latency[S_first_dest] - 1);
		
		/* Remove from this queue, still in processor's queue */
		S_dequeue (entry);
	    }
	    /* Otherwise, block cache until done (put in mc_check_queue) */
	    else
	    {
		if (S_dcache_debug_mem_copy)
		{
		    fprintf (debug_out, 
			     "COPY_CHECK BLOCKED for buf:%08x op %i\n",
			     sint->trace.mem_copy_buf,
			     sint->oper->lcode_id);
		    S_print_queue (debug_out, dcache->mc_instr_queue);
		    S_print_queue (debug_out, dcache->mc_pending_queue);
		}
		
		/* Move to mc_instr_queue */
		S_move_entry_before (dcache->mc_check_queue, entry, NULL);
	    }
	    break;	    

	  default:
	    S_print_sint (stderr, sint);
	    S_punt ("S_sim_dcache: unknown opc type %i at cycle %d.", 
				info->opc_type, S_sim_cycle);

	}
	
	/* if we are blocked then quit loop */
	if (S_block_until_store_completed) break;
    }
}



void S_sim_dcache_second_half_cycle (Pnode *pnode)
{
    /* 10/26/04 REK Commenting out unused variables to quiet compiler
     *              warnings. */
#if 0
    Sq_entry    *entry2, *next_entry;
    Sint        *temp_sint;
    Scblock     *block;
    Dcache_Data *data;
    Scache	*cache;
#endif
    Sq_entry    *entry;
    Dcache      *dcache;
    S_Opc_Info	*info;
    Sint	*sint;
    int		used_by_load;
    unsigned	start_addr_mask = 0;
    Dcache_Stats *dstats;
    int		addr,i;
    int		blocks_to_cache;

    /* Do nothing for perfect dcache */
    if (S_dcache_model == DCACHE_MODEL_PERFECT)
	return;

    /* Get dcache for ease of use */
    dcache = pnode->dcache;
    
    /* Get stats structure for ease of use and put in cache structure
     * for calls to other dcache routines.
     */
    dstats = pnode->stats->dcache;
    dcache->dstats = dstats;
   
    /*
     * At end of cycle, if we are the destination of a read result,
     * load block into memory.
     */
    if ((BUS(dest) == DCACHE) && (BUS(type) == READ_RESULT))
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
	if ((S_streaming_support && (BUS(segment) == 1)) ||
	    ((!S_streaming_support) && (BUS(segment) == BUS(length))))
	{
	    /* The first pending load better be for this block ! */
	    entry = dcache->miss_buffer->head;

	    if (entry == NULL)
		S_punt ("No pending load for dcache request of %x: miss queue empty",
			BUS(addr));

	    /* Save cache destination */
	    S_current_dest |= target_cache(entry->sint);

    	    /* Get start_addr_mask for ease of use */
	    if (S_prefetch_cache && (S_current_dest & PLAYDOH_TCHS_V1) && 
			!((S_pcache_block_size < S_dcache_block_size) && 
					(S_current_dest & PLAYDOH_TCHS_C1)))
	    {
	    	start_addr_mask = dcache->pcache->start_addr_mask;
	    	S_current_size = S_pcache_block_size;
	    }
	    else if (S_current_dest & PLAYDOH_TCHS_C1)
	    {
	    	start_addr_mask = dcache->cache->start_addr_mask;
	    	S_current_size = S_dcache_block_size;
	    }
	    else
	    {
	    	start_addr_mask = S_bypass_addr_mask;
	    	S_current_size = S_bypass_req_size;
	    }

	    /* Sanity check */
	    if ((entry->sint->trace.mem_addr & start_addr_mask) !=
		 BUS(addr))
	    {
	    	S_print_sint (stderr, entry->sint);
		fprintf(stderr,"serial no %d, mask %x, dest %d\n",
		    entry->sint->serial_no, start_addr_mask, S_current_dest);
		S_punt("No pending load at cycle %d for dcache request of %x: expecting %x",
			S_sim_cycle,BUS(addr),entry->sint->trace.mem_addr);
	    }
	    else if (S_dcache_debug_misses)
		fprintf(debug_out,"Bus addr matches miss request addr %x\n",
			entry->sint->trace.mem_addr);

	    /* Get info on instruction */
	    info = &opc_info_tab[GET_OPC(entry->sint)];

	    if (info->opc_type == LOAD_OPC)
	    {
		/* Ready next cycle (or later if latency > 2) */
		entry->sint->real_complete_time[S_first_dest] = S_sim_cycle +
		    (entry->sint->oper->real_latency[S_first_dest] - 1);

		/* Remove from this queue, still in processors queue */
		S_dequeue (entry);
	    }
	
	    /* If store, done with it (this happens with write allocate) */
	    else
	    {
		sint = entry->sint;
		S_dequeue (entry);
		/* free sint if in no other queues */
		if (!sint->entry_list)
		    S_free_sint (sint);
	    }
	}

	/* Even if no streaming support, must set S_current_dest */
	if (!S_streaming_support && (BUS(segment) == 1))
	{
	    /* The first pending load better be for this block ! */
	    entry = dcache->miss_buffer->head;

	    S_current_dest |= target_cache(entry->sint);

	    if (S_prefetch_cache && (S_current_dest & PLAYDOH_TCHS_V1) &&
			!((S_pcache_block_size < S_dcache_block_size) && 
					(S_current_dest & PLAYDOH_TCHS_C1)))
	    {
	    	start_addr_mask = dcache->pcache->start_addr_mask;
	    	S_current_size = S_pcache_block_size;
	    }
	    else if (S_current_dest & PLAYDOH_TCHS_C1)
	    {
	    	start_addr_mask = dcache->cache->start_addr_mask;
	    	S_current_size = S_dcache_block_size;
	    }
	    else
	    {
	    	start_addr_mask = S_bypass_addr_mask;
	    	S_current_size = S_bypass_req_size;
	    }

	    if (entry == NULL)
		S_punt ("No pending load for dcache request of %x: miss queue empty",
			BUS(addr));
	    else if ((entry->sint->trace.mem_addr & start_addr_mask) !=
		 BUS(addr))
		S_punt ("No pending load for dcache request of %x: expecting %x",
			BUS(addr),entry->sint->trace.mem_addr);
	}
	
	/* If finished loading block, add to cache, 
	 * and can process all pending loads for
	 * this block next cycle (do now, just delay ready time 1 cycle) 
	 */
	if (BUS(segment) == BUS(length))
	{
	    /*
	     * Move block of data into cache if at last cycle of transfer
	     */

	    /* Figure out target cache */
	    if (S_prefetch_cache && S_current_dest & PLAYDOH_TCHS_V1)
	    {
		addr = BUS(addr);
		if (S_current_dest & PLAYDOH_TCHS_C1 && 
				S_dcache_block_size > S_pcache_block_size) 
		    blocks_to_cache = S_dcache_block_size/S_pcache_block_size;
		else
		    blocks_to_cache = 1;
		for (i=0;i<blocks_to_cache;i++)
		{
	    	    bus_data_to_cache(dcache->pcache,BUS(addr),addr,
			PLAYDOH_TCHS_V1,dcache,BUS(stats),0,0);
		    addr += S_pcache_block_size;
		}
	    }

	    if (S_current_dest & PLAYDOH_TCHS_C1)
	    {
		addr = BUS(addr);
		if (S_prefetch_cache && S_current_dest & PLAYDOH_TCHS_V1 &&
				S_pcache_block_size > S_dcache_block_size) 
		    blocks_to_cache = S_pcache_block_size/S_dcache_block_size;
		else
		    blocks_to_cache = 1;
		for (i=0;i<blocks_to_cache;i++)
		{
	    	    bus_data_to_cache(dcache->cache,BUS(addr),addr,
			PLAYDOH_TCHS_C1,dcache,BUS(stats),0,0);
		    addr += S_dcache_block_size;
		}
	    }

	    if (S_current_dest & PLAYDOH_TCHS_C3 && 
		!(S_current_dest & (PLAYDOH_TCHS_C1|PLAYDOH_TCHS_V1)))
	    {
        	/* All pending misses (except first one (handled above))
        	 * on this data will be ready next cycle.
		 * Still need to set this in case any other misses with
		 * target of C3 covered by this miss.
        	 */
        	dcache->addr_loaded_last_cycle = BUS(addr);
	    }
	}
    }
       
    /* If first cycle of prefetch result data, set S_current_dest */
    else if ((BUS(dest) == DCACHE) && (BUS(type) == PREFETCH_RESULT) &&
	     (BUS(segment) == 1))
    {
	/* See if head of pending prefetches is for this block */
	entry = dcache->pending_prefetches->head;

	if (entry)
	{
	  S_current_dest |= target_cache(entry->sint);

	  if (S_prefetch_cache && (S_current_dest & PLAYDOH_TCHS_V1) &&
			!((S_pcache_block_size < S_dcache_block_size) && 
					(S_current_dest & PLAYDOH_TCHS_C1)))
	  {
	    start_addr_mask = dcache->pcache->start_addr_mask;
	    S_current_size = S_pcache_block_size;
	  }
	  else
	  {
	    start_addr_mask = dcache->cache->start_addr_mask;
	    S_current_size = S_dcache_block_size;
	  }
	}

	if (!entry)
	    S_punt ("Missing prefetch record for request of %x: queue empty",
			BUS(addr));
	else if ((entry->sint->trace.mem_addr & start_addr_mask) !=
								BUS(addr))
	    S_punt ("Missing prefetch record for request of %x: expecting %x",
			BUS(addr),entry->sint->trace.mem_addr);
    }
       
    /* If last cycle of prefetch result data, handle prefetched block */
    else if ((BUS(dest) == DCACHE) && (BUS(type) == PREFETCH_RESULT) &&
	     (BUS(segment) == BUS(length)))
    {
	/* See if head of pending prefetches is for this block */
	entry = dcache->pending_prefetches->head;

    	/* Get start_addr_mask for ease of use */
	if (entry)
	{
	  if (S_prefetch_cache && 
			S_current_dest & PLAYDOH_TCHS_V1)
	  {
	    start_addr_mask = dcache->pcache->start_addr_mask;
	  }
	  else
	  {
	    start_addr_mask = dcache->cache->start_addr_mask;
	  }
	}

	if ((entry != NULL) &&
	    ((entry->sint->trace.mem_addr & start_addr_mask) ==
	     BUS(addr)))
	{
	    if (entry->sint->flags & PREFETCH_TAKEN_OVER)
		used_by_load = 1;
	    else
	        /* If prefetch (not used by load), need to check prefetch bit */
	        used_by_load = 0;
	 
	    /* Remove sint holding address */
	    sint = entry->sint;
	    S_dequeue (entry);
	    S_free_sint (sint);
	}
	
	/* Prefetch should either be in pending buf or have been taken
	 * over by a load.
	 */
	else if (!entry)
	    S_punt ("Missing prefetch record for request of %x: queue empty",
			BUS(addr));
	else
	    S_punt ("Missing prefetch record for request of %x: expecting %x",
			BUS(addr),entry->sint->trace.mem_addr);

	/* If taken over, make sure pending load is waiting for this data */
	if (used_by_load && !find_overlapping_entry_and_target(S_current_dest,
		dcache->miss_buffer, BUS(addr), start_addr_mask))
	{
	    fprintf(debug_out,"S_current_dest=%x\n",S_current_dest);
	    S_punt ("No pending load which took over prefetch of %x",
			BUS(addr));
	}

	/* Figure out target cache */
	if (S_prefetch_cache && S_current_dest & PLAYDOH_TCHS_V1)
	{
	    addr = BUS(addr);
	    if (S_current_dest & PLAYDOH_TCHS_C1 && 
				S_dcache_block_size > S_pcache_block_size) 
		blocks_to_cache = S_dcache_block_size/S_pcache_block_size;
	    else
		blocks_to_cache = 1;
	    for (i=0;i<blocks_to_cache;i++)
	    {
	    	bus_data_to_cache(dcache->pcache,BUS(addr),addr,
			PLAYDOH_TCHS_V1,dcache,BUS(stats),1,used_by_load);
		addr += S_pcache_block_size;
	    }
	}

	if (S_current_dest & PLAYDOH_TCHS_C1)
	{
	    addr = BUS(addr);
	    if (S_prefetch_cache && S_current_dest & PLAYDOH_TCHS_V1 &&
				S_pcache_block_size > S_dcache_block_size) 
		blocks_to_cache = S_pcache_block_size/S_dcache_block_size;
	    else
		blocks_to_cache = 1;
	    for (i=0;i<blocks_to_cache;i++)
	    {
	    	bus_data_to_cache(dcache->cache,BUS(addr),addr,
			PLAYDOH_TCHS_C1,dcache,BUS(stats),1,used_by_load);
		addr += S_dcache_block_size;
	    }
	}
    }

    /* If last cycle of mem_copy result data, update mem_copy count */
    else if ((BUS(dest) == DCACHE) && (BUS(type) == MC_READ_RESULT) &&
	     (BUS(segment) == BUS(length)))
    {
	/* Assume  head of pending mem_copies is for this block */
	entry = dcache->mc_pending_queue->head;

	/* Make sure not NULL */
	if (entry == NULL)
	    S_punt ("MEM_COPY data returned to null mc_pending_queue");

	/* Decrement the count remaining for the mem_copy,
	 * dequeue and delete sint if reaches zero.
	 */
	sint = entry->sint;
	sint->trace.mem_copy_requests--;

	/* If mem_copy complete, delete it and free up any pending checks */
	if (sint->trace.mem_copy_requests <= 0)
	{
	    S_dequeue (entry);
	    S_free_sint (sint);

	    /* Release any pending checks on this copy */
	    update_mem_copy_checks(dcache);
	}
    }

    /*
     * Mark dcache as busy if:
     * 1) If the load miss bypass limit has been exceeded.
     * 2) If the store buffer limit has been exceeded.
     * 3) If blocking cache, and line being loaded from memory and
     *    it will not finish this cycle.
     *
     */
    if ((dcache->miss_buffer->size > S_dcache_miss_bypass_limit) ||
	(dcache->write_buffer->size > S_dcache_write_buf_size) ||
	((S_dcache_model == DCACHE_MODEL_BLOCKING) && (BUS(dest) == DCACHE) &&
	 (BUS(segment) < BUS(length))) ||
	(dcache->mc_check_queue->size > 0) ||
	(S_block_until_store_completed && dcache->write_buffer->size))
    {
	pnode->dcache_busy = 1;
	if (S_dcache_debug_misses)
	    fprintf (debug_out, "%d: Dcache BLOCKED this cycle\n",
			S_sim_cycle);
    }
    else
    {
	S_block_until_store_completed = 0;
	if (S_dcache_debug_misses && pnode->dcache_busy)
	    fprintf (debug_out, "%d: Dcache unblocked this cycle\n",
			S_sim_cycle);
	pnode->dcache_busy = 0;
    }
}


