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
 *      File:   s_profile.h
 *      Author: Teresa Johnson and John Gyllenhaal
 *      Creation Date:  1994
 *      Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_PROFILE_H_
#define _LSIM_S_PROFILE_H_

#include <config.h>
#include "s_main.h"

#define LOAD_HIST_SIZE 5

enum {
  BRANCH_EXEC_NONE =0,
  BRANCH_EXEC_TAKEN ,
  BRANCH_EXEC_FALLTHRU ,
  BRANCH_EXEC_BOTH
};

typedef struct S_Profile_Info
{
   int  flags;
   int num_executed;
   int  num_reuses;
   int  num_spatial_reuses;
   int  num_temporal_reuses;
   int	num_misses;
   int	num_aliases;
   void *ptr;
				/* Branch information */
   int	num_mispredicted;	/* Number mispredictions */
   int  branch_exec_state;      /* Track the executions of each branch */
   int	num_pred_squashed;

				/* Predicate profiling */
   int  num_pred_dest0_set;
   int  num_pred_dest1_set;
   int	num_promoted_pred_squashed;

   int	num_icache_misses;
} S_Profile_Info;

typedef struct Profile_data
{
   int *last_ref_pc;
   int *access_ctr;
   int last_ref_pc_block;
   int num_hits;
   struct Profile_data *next;
} Profile_data;

typedef struct Distr_data
{
    int count;
    int is_miss;
    int is_load;
} Distr_data;

typedef struct S_MemDep_Data_Info
{
	int last_store_pc;
	int last_load_pc[LOAD_HIST_SIZE];
	int num_loads;
	int func_no;
} S_MemDep_Data_Info;

typedef struct S_MemDep_Alias_Info
{
	int source_pc;
	int alias_times;
	struct S_MemDep_Alias_Info *next;
} S_MemDep_Alias_Info;

typedef struct S_MemDep_Access_Info
{
	int pc;
	S_MemDep_Alias_Info *alias_info;
} S_MemDep_Access_Info;

typedef struct S_Stats_Stack {
	int pc;
	int func_name_id;
	int line_no;
	int loop_type_id;
	double start_cycle;
	struct S_Stats_Stack *next;
} S_Stats_Stack;

typedef struct S_Hash_Entry {
	double cycle_count;
	int func_name_id;
	int line_no;
} S_Hash_Entry;

typedef struct S_Addr_Interval {
    unsigned int start;
    unsigned int end;
    struct S_Addr_Interval *next;
} S_Addr_Interval;

typedef struct S_Addr_Func_Summary {
    struct S_Fn *fn;
    char *lib_name;
    int active;
    int store_interval_length;
    int load_interval_length;
    int unsafe;
    S_Addr_Interval *store_head, *store_tail;
    S_Addr_Interval *load_head, *load_tail;
} S_Addr_Func_Summary;

typedef struct S_Activation_Record {
    S_Addr_Func_Summary *afs;
} S_Activation_Record;

extern int S_clear_addrs_on_overflow;
extern S_Profile_Info *prof_info;

#endif
