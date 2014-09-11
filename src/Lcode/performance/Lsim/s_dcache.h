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
 *      File:   s_dcache.h
 *      Author: Teresa Johnson and John C. Gyllenhaal
 *      Creation Date:  1995
 *      Copyright (c) 1995 Teresa Johnson, John C. Gyllenhaal, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_DCACHE_H_
#define _LSIM_S_DCACHE_H_

#include <config.h>

typedef struct Dcache_Data
{
    int		flags;
} Dcache_Data;

typedef struct Dcache
{
    char		*name;
    struct Pnode	*pnode;
    Scache		*cache;
    Scache		*conflict_cache;
    Scache		*pcache;
    Squeue		*input_queue;
    Squeue		*write_buffer;		/* pending writes to memory */
    Squeue		*miss_buffer;		/* pending load/store misses */
    Squeue		*miss_request_buffer;	/* pending miss requests */
    Squeue		*prefetch_request_buffer;/* pending prefetch requests*/
    Squeue		*pending_prefetches;	/* prefetches gone to mem*/
    int			addr_loaded_last_cycle;	/* -1 for none */
    int			L2_busy;
    Squeue		*mc_instr_queue;	/* mem_copy, write_back instr*/
    Squeue		*mc_pending_queue;	/* waiting for memory */
    Squeue		*mc_check_queue;	/* Pending check conmmands */
    Mqueue		*mc_request_buffer;	/* read/write requests */
    struct Dcache_Stats	*dstats;		/* Stats structure to use */
    Scache		*tlb;			/* 1st level tlb */	
    Scache		*tlb2;			/* 2nd level tlb */
    Scache		*page_table;		/* All pages accessed */
} Dcache;


/*
 * Dcache stats
 */
typedef struct Dcache_Stats
{
    unsigned		read_hits;
    unsigned		vcache_hits;
    unsigned		write_hits;
    unsigned		reads_forwarded;
    unsigned		read_misses;
  /* HCH 10-18-99 */
    unsigned            speculative_read_misses;
    unsigned		redundant_read_misses;
    unsigned		redundant_read_misses_off_path;
    unsigned		write_misses;
    unsigned		writes_combined;
    unsigned		written_through;
    unsigned		written_through_due_to_store;
    unsigned		written_through_due_to_load;
    unsigned		writes_to_C1_changed_to_V1;
    unsigned		writes_to_V1_changed_to_C1;
    unsigned		blocks_kicked_out;
    unsigned		dirty_blocks_kicked_out;
    unsigned		dirty_blocks_written_back;
    unsigned		read_hits_off_path;
    unsigned		write_hits_off_path;
    unsigned		conflict_misses[MAX_OPC+1];
    unsigned		anticonflict_hits[MAX_OPC+1];
    unsigned		conflict_misses_off_path[MAX_OPC+1];
    unsigned		anticonflict_hits_off_path[MAX_OPC+1];
    unsigned		reads_forwarded_off_path;
    unsigned		read_misses_off_path;
    unsigned		write_misses_off_path;
    unsigned		prefetch_hits;
    unsigned		redundant_prefetches;
    unsigned		prefetch_buf_full;
    unsigned		prefetch_misses;
    unsigned		prefetch_requests_sent;
    unsigned		loads_used_prefetch_request;
    unsigned		prefetches_completed;
    unsigned		prefetch_hits_off_path;
    unsigned		redundant_prefetches_off_path;
    unsigned		prefetch_buf_full_off_path;
    unsigned		prefetch_misses_off_path;
    unsigned		cycles_blocked_miss_buffer;
    unsigned		cycles_blocked_write_buffer;
    unsigned		cycles_blocked_mem_copy_check;
    unsigned		cycles_blocked_store_completes;

    unsigned            cycles_L2_busy_when_needed;

    unsigned		load_source_C1;
    unsigned		load_source_V1;
    unsigned		load_dest_C1;
    unsigned		load_dest_C3;
    unsigned		load_dest_V1;
    unsigned		loads_became_prefetches;
    unsigned		store_dest_C1;
    unsigned		store_dest_C3;
    unsigned		store_dest_V1;

    unsigned		load_C1_hit_C1;
    unsigned		load_C1_hit_V1;
    unsigned		load_C1_miss;
    unsigned		load_V1_hit_C1;
    unsigned		load_V1_hit_V1;
    unsigned		load_V1_miss;
    unsigned		load_C3_hit_C1;
    unsigned		load_C3_hit_V1;
    unsigned		load_C3_miss;
    unsigned		store_C1_hit_C1;
    unsigned		store_C1_hit_V1;
    unsigned		store_C1_miss;
    unsigned		store_V1_hit_C1;
    unsigned		store_V1_hit_V1;
    unsigned		store_V1_miss;
    unsigned		store_C3_hit_C1;
    unsigned		store_C3_hit_V1;
    unsigned		store_C3_miss;

    unsigned		pcache_blocks_kicked_out;
    unsigned		pcache_dirty_blocks_kicked_out;
    unsigned		pcache_dirty_blocks_written_back;

    unsigned		overlapping_entries_with_different_dests;
    
    unsigned	 	num_mem_copies;
    unsigned	 	num_mem_copy_tags;
    unsigned	 	num_mem_copy_backs;
    unsigned	 	mem_copy_blocks;
    unsigned	 	mem_copy_tag_blocks;
    unsigned	 	blocks_mem_copies_kicked_out;
    unsigned	 	blocks_mem_copy_tags_kicked_out;
    unsigned	 	mem_copy_buffer_conflicts;
    unsigned	 	mem_copy_tag_buffer_conflicts;
    unsigned	 	mem_copy_back_blocks;
    unsigned	 	mem_copy_back_block_misses;
    unsigned	 	mem_copy_back_buffer_conflicts;
    unsigned	 	mem_copy_memory_requests;
    unsigned	 	mem_copy_cache_requests;
    unsigned	 	mem_copy_back_memory_requests;
    unsigned	 	mem_copy_back_cache_requests;

    unsigned		total_TLB_lookups;
    unsigned		total_TLB1_misses;
    unsigned		total_TLB2_misses;
    unsigned		total_pages_accessed;
} Dcache_Stats;

extern void S_debug_tlb (FILE *out);
extern void S_add_stats_dcache (Dcache_Stats *dest, Dcache_Stats *src1,
				Dcache_Stats *src2);
extern void S_print_stats_region_dcache (FILE *out, Stats *stats,
					 char *rname, Stats *total_stats);

#endif

