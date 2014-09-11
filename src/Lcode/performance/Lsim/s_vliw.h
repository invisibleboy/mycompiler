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
 *      File:   s_vliw.h
 *      Author: John Gyllenhaal
 *      Creation Date:  Aug 1994
 *      Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_VLIW_H_
#define _LSIM_S_VLIW_H_

#include <config.h>

typedef struct VLIW
{
    char		*name;
    Pnode		*pnode;
    int			flags;
    int			enable_ifetch_next_cycle;
    int			ifetch_enabled;	/* Allows disabling of ifetch */
    int			ifetch_pc;	/* Fetch pc (may be incorrect) */
    int			ifetch_addr;	/* Fetch addr (may be incorrect) */
    int			on_path_pc;	/* "Context switch" pc */
    int			on_correct_path;/* Fetching from correct path? */
    Reg_File		*reg_file;
    Squeue		**fetch_stage;	/* Array of fetch stages */
    Squeue		*ibuf;		/* Alias to first fetch stage */
    Squeue		*last_fetch_stage; /* Alias to last fetch stage */
    Squeue		*reorder_queue;
    Squeue              *decode_queue;
    Squeue              *exec_queue;
    Squeue              *pending_stores[STORE_HASH_SIZE];
} VLIW;

typedef struct VLIW_Stats
{
    int		cycles_packet_unavailable;
    int		total_virtual_latency_stalls;
    int		load_virtual_latency_stalls;
    int		dcache_busy_stalls;
    int		interlock_stalls;	/* Get with hardware support */
    int		sched_error_stalls;	/* deps across JSR's are not handled 
					   properly by scheduler*/
    int		dest_not_avail_errors;	/* predicated code may mess up */
    int		num_squashed;
} VLIW_Stats;

typedef struct VLIW_Data
{
    Sint	*sint;		/* The sint this structure is the data for */
    int		flags;		/* Superscalar flags */
    int		issue_time;	/* Time that went to Fu's or window */
    Sq_entry	*reorder_queue_entry;/* Pointer to location in reorder queue */} VLIW_Data;

/* Prototypes */
void S_vliw_ifetch_stage (Pnode *pnode);
void S_vliw_debug_ifetch_stage (Pnode *pnode);
void S_vliw_fix_mispredicted_branch (Pnode *pnode, Sint *mispred_sint);
void S_vliw_decode_stage (Pnode *pnode);
void S_vliw_exec_stage (Pnode *pnode);

#endif
