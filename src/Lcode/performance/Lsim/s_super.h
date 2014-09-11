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
 *      File:   s_super.h
 *      Author: John Gyllenhaal
 *      Creation Date:  Aug 1994
 *      Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_SUPER_H_
#define _LSIM_S_SUPER_H_

#include <config.h>

/* Superscalar processor */
typedef struct Superscalar
{
    char		*name;
    struct Pnode	*pnode;
    int			flags;
    int                 squashing;	/* Set if processor is squashing */
    int			enable_ifetch_next_cycle;
    int			ifetch_enabled;	/* Allows disabling of ifetch */
    int			ifetch_pc;	/* Fetch pc (may be incorrect) */
    int			ifetch_addr;	/* Fetch addr (may be incorrect) */
    int			on_path_pc;	/* "Context switch" pc */
    int			on_correct_path;/* Fetching from correct path? */
    int			untraced_fixup; /* On untraced "on_path" fixup path */
    int			fixup_return_pc;/* Return to after untraced fixup */
    Reg_File		*reg_file;
    Squeue		**fetch_stage;	/* Array of fetch stages */
    Squeue		*ibuf;
    Squeue		*last_fetch_stage; /* Alias to last fetch stage */
    Squeue		*reorder_queue;
    Squeue              *decode_queue;
    Squeue              *exec_queue;
    Squeue              *pending_stores[STORE_HASH_SIZE];
    int                 cb_start_time;  /* Needed for sched stats */
} Superscalar;


typedef struct Superscalar_Data
{
    Sint	*sint;		/* The sint this structure is the data for */
    int		flags;		/* Superscalar flags */
    int		issue_time;	/* Time that went to Fu's or window */
    Sdep 	*input_dep;	/* Sints dependent on, can fire when NULL */
    Sdep	*output_dep;	/* Sints that on dependent on this Sint */
    Sq_entry	*reorder_queue_entry;/* Pointer to location in reorder queue */
} Superscalar_Data;


/*
 * Superscalar processor stats
 */
typedef struct Superscalar_Stats
{
    unsigned		loads_forwarded;
    unsigned		loads_blocked;
    unsigned		cycles_loads_blocked;
    unsigned		cycles_sint_blocked_on_pending_load;
    unsigned		cycles_dcache_busy_when_needed;
    unsigned		cycles_dcache_ports_unavailable;
    unsigned		cycles_branch_unit_unavailable;
    unsigned		cycles_ialu_unit_unavailable;
    unsigned		cycles_falu_unit_unavailable;
    unsigned		cycles_load_unit_unavailable;
    unsigned		cycles_store_unit_unavailable;
    unsigned		cycles_cache_directive_unit_unavailable;

    unsigned		*issue_utilization;
    unsigned		*branch_utilization;
    unsigned		*ialu_op_utilization;
    unsigned		*falu_op_utilization;
    unsigned		*load_utilization;
    unsigned		*store_utilization;
    unsigned		*cache_directive_utilization;

    unsigned		unclassified_operations;


    unsigned		untraced_fixups;
    unsigned		num_squashed;

} Superscalar_Stats;

#endif
