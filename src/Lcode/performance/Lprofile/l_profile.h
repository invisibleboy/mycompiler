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
 *      File :          l_profile.h
 *      Description :   Header file for l_profile.c
 *                      
 *      Creation Date : April, 1993
 *      Author :        John Gyllenhaal, Wen-mei Hwu
 *
 *==========================================================================*/

#ifndef _L_PROFILE_H
#define _L_PROFILE_H

#include <config.h>
#include <library/i_types.h>
#include <library/i_error.h>
#include <Lcode/l_trace_interface.h>

#define FUNC_HASH_SIZE	1024	/* Must be power of 2 */
#define TRACE_SIZE	1024


/*
 * Swaps byte order in word 'W'.  Used to convert architectures with
 * different byte orders (assumes 32 bit words).
 * Please report bugs found in this macro to John Gyllenhaal.
 */
#define SWAP_BYTES(W)           ((((unsigned long) W) << 24) | \
                                 ((((unsigned long) W) >> 24) & 0xff) | \
                                 ((((unsigned long) W) >> 8) & 0xff00) | \
                                 ((((unsigned long) W) & 0xff00) << 8))

#define FN		1
#define CB		2
#define BR		3
#define JMP		4
#define PJMP		5
#define JSR		6
#define RET		7
#define HASH		8

#define LOAD       	10
#define STORE		11
#define LOAD_ALLOC      12
#define STORE_ALLOC     13
#define LOAD_STACK      14
#define STORE_STACK     15
#define MALLOC          16
#define FREE            17

/* Flags for Pcontrols */
#define LOOP_HEADER	0x00000001

/* Flags for btb entries */
#define IN_BTB	0x00000001

/* Supported BTB models to profile */
#define BTB_MODEL_COUNTER 1
#define BTB_MODEL_2_LEVEL 2

/* Hash table size for loop iterations, must be power of two */
#define ITER_HASH_SIZE	8192

/* 20040201 SER
 * Memory history tracking constants, structs.
 */
#define OBJ_GLOBAL 0x1
#define OBJ_STACK  0x2
#define OBJ_HEAP   0x4

typedef struct Pop_heap_obj
{
  int id;
  int count;
  int malloc_id;
  struct Pop_heap_obj * next;
}
Pop_heap_obj;

typedef struct Pop_missing_obj
{
  int id;
  int type;
  int count;
  struct Pop_missing_obj * next;
}
Pop_missing_obj;

typedef struct pmem_addr_st
{
  int num_objs;
  int num_heap_objs;
  int num_missing_objs;
  int stack_count;
  int *id;
  int *count;
  struct Pop_heap_obj *heap_obj;
  struct Pop_missing_obj *missing_obj;
}
Pmem_addr;

#define TRACK_HEAP_OBJS
#define MAX_GLOB_OBJS 100000
#define BASE_HEAP_OBJ_ID 100000
#define MAX_LIVE_HEAP_OBJS 50000
#define BASE_MALLOC_ID 200000
#define MAX_MALLOCS 200
#define MAX_CALL_DEPTH 20

/* For building linked list of condition codes for hashing jumps */
typedef struct cnode_st
{
  int cond;
  ITintmax weight;
  struct cnode_st *next;
  struct cnode_st *prev;
}
Cnode;

typedef struct piter_st
{
  ITintmax weight;
  ITintmax iterations;
  struct piter_st *next;
}
Piter;

typedef struct ploop_info_st
{
  struct pfunc_st *func;
  struct pcontrol_st *cb;
  char *loop_preheader;		/* Indexed by cb id, 1 if preheader */
  int cur_iter_count;
  int cur_inner_iter_count;
  int entry_count;
  int is_inner;                 /* HCH 5-7-01: Inner loop flag dumped by 
				   Lencode if do_buf_info turned on */
  Piter *iter_hash[ITER_HASH_SIZE];
  Piter **sorted_iters;		/* used just before print time */
}
Ploop_info;


typedef struct pcontrol_st
{
  int type;
  int id;
  ITintmax weight;
  ITintmax branched;
  int bb_size;
  int flags;			/* For extensions like loop iteration info */
  int cb_id;			/* Id of cb the control instruction is in */
  union
  {
    Cnode *cnode_head;
    Ploop_info *loop_info;
  }
  ext;
  Pmem_addr *mem_addr_struct;
}
Pcontrol;

typedef struct pBTB_st
{
  int flags;
  int counter;
  int target;
  int mispred_count;
}
PBTB;

typedef struct pfunc_st
{
  char *name;
  char *asm_name;
  int addr;
  int size;
  int max_cb;
  int num_jsrs;
  ITintmax weight;
  int *cb_tab;
  int *jsr_tab;
  Pcontrol *control_tab;
  PBTB *BTB_tab;
  struct pfunc_st *next_func;
  struct pfunc_st *next_hash;
}
Pfunc;

typedef struct ptrace_st
{
  int *buf;
  int *ptr;
  int *end;
  int source_fd;
  Pfunc *func;
  unsigned trace_count;
}
Ptrace;

#endif
