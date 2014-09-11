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
 *      File:   s_scache.h
 *      Author: Teresa Johnson
 *      Creation Date:  1996
 *      Copyright (c) 1996 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_SCACHE_H_
#define _LSIM_S_SCACHE_H_

#include <config.h>

typedef struct L2_Bus
{
    int			cycle_avail;
    int			avail;		/* 1 if bus available to anyone */
    int			L2_only;	/* 1 if bus available to memory only */
    int			type;		/* Transaction type */
    int			src;		/* Transaction source */
    int			dest;		/* Transaction destination */
    unsigned		addr;		/* Base address for transaction */
    int			size;		/* Transaction size (in bytes) */
    int			length;		/* Transaction length (in cycles) */
    int			segment;	/* Segment of trans. (1 to length) */
    int			size_offset;	/* Used to calc length */
    int			size_shift;	/* Used to calc length */
    int			playdoh_flags;
    struct Stats	*stats;		/* Stats structure to update */
} L2_Bus;

typedef struct L2_Request
{
    int			type;
    int			src;
    unsigned		addr; 
    int			size;
    int			complete_time;
    int			playdoh_flags;
    struct Stats	*stats;		/* Stats structure to update */
    struct L2_Request	*next;
} L2_Request;

typedef struct L2_Data
{
    int			flags;
} L2_Data;

typedef struct L2_Cache
{
    L2_Request		*new_pending_head;
    L2_Request		*new_pending_tail;
    L2_Request		*pending_head;
    L2_Request		*pending_tail;
    char                *name;
    struct Pnode        *pnode;
    Scache              *cache;
    Scache		*conflict_cache;
    Mqueue              *write_buffer;          /* pending writes to memory */
    Mqueue              *miss_buffer;           /* pending load/store misses */
    Mqueue              *miss_request_buffer;   /* pending miss requests */
    Mqueue              *prefetch_request_buffer;/* pending prefetch requests*/
    Mqueue              *pending_prefetches;    /* prefetches gone to mem*/
    int                 addr_loaded_last_cycle; /* -1 for none */
    struct L2_Stats 	*L2_stats;                /* Stats structure to use */
} L2_Cache;


/*
 * L2 stats
 */
typedef struct L2_Stats
{
    unsigned            read_hits;
    unsigned            L2_hits;
    unsigned            vcache_hits;
    unsigned            write_hits;
    unsigned            reads_forwarded;
    unsigned            read_misses;
    unsigned            redundant_read_misses;
    unsigned            redundant_read_misses_off_path;
    unsigned            write_misses;
    unsigned            writes_combined;
    unsigned            blocks_kicked_out;
    unsigned            dirty_blocks_kicked_out;
    unsigned            dirty_blocks_written_back;
    unsigned            read_hits_off_path;
    unsigned            write_hits_off_path;
    unsigned            conflict_misses[MAX_OPC+1];
    unsigned            anticonflict_hits[MAX_OPC+1];
    unsigned            conflict_misses_off_path[MAX_OPC+1];
    unsigned            anticonflict_hits_off_path[MAX_OPC+1];
    unsigned            reads_forwarded_off_path;
    unsigned            read_misses_off_path;
    unsigned            write_misses_off_path;
    unsigned            prefetch_hits;
    unsigned            redundant_prefetches;
    unsigned            prefetch_buf_full;
    unsigned            prefetch_misses;
    unsigned            prefetch_requests_sent;
    unsigned            loads_used_prefetch_request;
    unsigned            prefetches_completed;
    unsigned            prefetch_hits_off_path;
    unsigned            redundant_prefetches_off_path;
    unsigned            prefetch_buf_full_off_path;
    unsigned            prefetch_misses_off_path;
    unsigned            cycles_blocked_miss_buffer;
    unsigned            cycles_blocked_write_buffer;
    unsigned            cycles_blocked_store_completes;

    unsigned            load_dest_C3;
    unsigned            load_dest_C2;
    unsigned            store_dest_C2;
    unsigned            store_dest_C3;

    unsigned            load_C2_hit_C2;
    unsigned            load_C2_miss;
    unsigned            load_C3_hit_C2;
    unsigned            load_C3_miss;
    unsigned            store_C2_hit_C2;
    unsigned            store_C2_miss;
    unsigned            store_C3_hit_C2;
    unsigned            store_C3_miss;
} L2_Stats;

/*
 * Bus utilization stats
 */
typedef struct L2_Bus_Stats
{
    unsigned		icache_read_requests;
    unsigned		icache_read_results;
    unsigned		dcache_read_requests;
    unsigned		dcache_read_results;
    unsigned		dcache_prefetch_requests;
    unsigned		dcache_prefetch_results;
    unsigned		dcache_write_requests;
    unsigned		waiting_for_result;
} L2_Bus_Stats;

extern void S_add_stats_L2_bus (L2_Bus_Stats *dest, L2_Bus_Stats *src1,
				L2_Bus_Stats *src2);

#endif
