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
/****************************************************************************n
 *
 *  File:  s_main.h
 *
 *  Description:  Main header file for lcode+ simulation
 *
 *  Creation Date :  July, 1993
 *
 *  Author:  John Gyllenhaal, Roger A. Bringmann
 *
 *  Revisions:
 *
 *      Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef _LSIM_S_MAIN_H_
#define _LSIM_S_MAIN_H_

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef _HPUX_SOURCE
#include <nlist.h>
#endif
#include <time.h>
#include <signal.h>
#include <string.h>
#ifdef _HPUX_SOURCE
char *strdup ();		/* since string.h is not working */
#endif
#include <sys/types.h>
#include <sys/stat.h>
#if defined (_SOLARIS_SOURCE)
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#include <sys/time.h>
#include <sys/resource.h>
#include <library/i_types.h>
#include <library/l_alloc_new.h>
#include <library/i_list.h>
#include <library/l_parms.h>
#include <library/l_histogram.h>
#include <library/symbol.h>
#include <Lcode/l_trace_interface.h>
#include <Lcode/l_encode_interface.h>
#include <Lcode/l_opc.h>

/* Don't currently use mdes info, so don't include it -JCG 10/14/96 */
/* #include <machine/lmdes.h> */

/* NEED TO UNDEFINE 'MCB' SINCE USED AS STRUCTURE NAME IN SIMULATOR 
 * -JCG 2/27/98 
 */
#undef MCB
#undef ALAT

/*
 * *** Lsim includes at bottom of this file ***
 */

/* PROCESSOR MODELS */
#define PROCESSOR_MODEL_SUPERSCALAR	1
#define PROCESSOR_MODEL_VLIW		2
#define PROCESSOR_MODEL_NYFO_VLIW	3	/* Kevin W. Rudd's research */
#define PROCESSOR_MODEL_PLAYDOH_VLIW	4

/* PROCESSOR TYPES */
#define PROCESSOR_TYPE_STATIC		1
#define PROCESSOR_TYPE_DYNAMIC		2

/* SAMPLE MODELS */
#define SAMPLE_MODEL_UNIFORM		1
#define SAMPLE_MODEL_RANDOM_SKIP	2

/* Lsim modes of operation */
#define SIMULATOR		1
#define PROFILER		2
#define X86_TRACE_GENERATOR	3

/* For profiling and s_object */
#define LOAD            0x00000001
#define STORE		0x00000002

/* Flags for S_Oper (Some flags in l_encode_interface.h) */
#define TRACED_CB_ENTRY		0x00010000
#define PREDICATED		0x00020000
#define CHANGES_STATE		0x00100000
#define REGION_BOUNDARY		0x00200000
#define ZERO_SPACE		0x00400000
#define TRACE_PROMOTED_PRED	0x00800000

/* Flags for Sint */
#define BRANCHED		0x00000001
#define MISPREDICTED		0x00000004
#define OFF_PATH		0x00000008
#define PRED_SQUASHED		0x00000010
#define UNTRACED_JSR		0x00000020
#define SQUASHED		0x00000040
#define COMPLETED		0x00000080
#define VALUE_FORWARDED		0x00000100
#define BLOCKED_BEFORE		0x00000200
#define DCACHE_MISS		0x00000400
#define LONGJMP			0x00000800
#define UNTRACED_FIXUP		0x00001000
#define MEM_COPY_DIRECTIVE     	0x00002000
#define CACHE_DIRECTIVE		0x00004000
#define CHANGES_REGION		0x00008000
#define MASKED_SEG_FAULT	0x00010000
#define MASKED_BUS_ERROR	0x00020000
#define PRED_DEST0_SET		0x00040000  /*For now assume max 2 pred dests */
#define PRED_DEST1_SET		0x00080000
#define PROMOTED_PRED_SQUASHED  0x00100000  /* Value of pred before promotion */
#define WRITE_THROUGH		0x00200000
#define PREFETCH_TAKEN_OVER	0x00400000
#define MCB_CONFLICT		0x00800000
#define ALAT_CONFLICT		0x01000000

/* Opcode types for S_Opc_Info */
#define UNTRACED_OPC		0
#define CBR_OPC			1
#define JMP_OPC			2
#define JRG_OPC			3
#define JSR_OPC			4
#define RTS_OPC			5
#define LOAD_OPC		6
#define STORE_OPC		7
#define PREFETCH_OPC		8
#define MEM_COPY_OPC		9
#define MEM_COPY_BACK_OPC	10
#define MEM_COPY_CHECK_OPC	11
#define MEM_COPY_RESET_OPC	12
#define MEM_COPY_TAG_OPC	13
#define MOVE_OPC		14
#define IALU_OPC		15
#define FALU_OPC		16
/* HCH 10-20-99 */
#define CHECK_OPC		17
#define MAX_OPC			17

/* Flags for Dcache_Block */
#define DIRTY_BLOCK		0x00000001
#define PREFETCH_BIT		0x00000002
#define MEM_COPY_BUFFER		0x00000004

/* Models for fetch unit */
#define FETCH_MODEL_AGGRESSIVE 		1
#define FETCH_MODEL_CONSERVATIVE	2

/* define BTB models */
#define BTB_MODEL_PERFECT 		0
#define BTB_MODEL_ALWAYS_WRONG		1
#define BTB_MODEL_COUNTER 		2
#define BTB_MODEL_TWO_LEVEL 		3
#define BTB_MODEL_STATIC		4
#define BTB_MODEL_BTC			5
#define BTB_MODEL_PREDICATE             6
#define BTB_MODEL_GAG			7
#define BTB_MODEL_GAS			8
#define BTB_MODEL_GAP			9
#define BTB_MODEL_SAG			10
#define BTB_MODEL_SAS			11
#define BTB_MODEL_SAP			12
#define BTB_MODEL_PAG			13
#define BTB_MODEL_PAS			14
#define BTB_MODEL_PAP			15
#define BTB_MODEL_GSHARE		16
#define BTB_MODEL_GSELECT		17

/* Define MCB models */
#define MCB_MODEL_NO_MCB		0
#define MCB_MODEL_PERFECT		1
#define MCB_MODEL_ALWAYS_CONFLICT	2
#define MCB_MODEL_SIMPLE_HASH		3
#define MCB_MODEL_REG_HASH		4
#define MCB_MODEL_KNUTH_HASH		5

/* Define MCB conflicts */
#define MCB_NO_CONFLICT			0
#define MCB_LOAD_LOAD_CONFLICT		1
#define MCB_LOAD_STORE_CONFLICT		2
#define MCB_CONTEXT_SWITCH_CONFLICT	3

/* Define ALAT models */
#define ALAT_MODEL_NO_ALAT		0
#define ALAT_MODEL_PERFECT		1
#define ALAT_MODEL_ALWAYS_CONFLICT	2
#define ALAT_MODEL_SIMPLE_HASH		3

/* Define ALAT conflicts */
#define ALAT_NO_CONFLICT		0
#define ALAT_LOAD_LOAD_CONFLICT		1
#define ALAT_LOAD_STORE_CONFLICT	2
#define ALAT_CONTEXT_SWITCH_CONFLICT	3

/* define icache models */
#define ICACHE_MODEL_PERFECT		1
#define ICACHE_MODEL_STANDARD		2
#define ICACHE_MODEL_SPLIT_BLOCK	3

/* define dcache models */
#define DCACHE_MODEL_PERFECT		1
#define DCACHE_MODEL_BLOCKING		2
#define DCACHE_MODEL_NON_BLOCKING	3

/* define scache models */
#define L2_MODEL_PERFECT                1
#define L2_MODEL_BLOCKING               2
#define L2_MODEL_NON_BLOCKING   	3

/* Defines for signaling hit/miss */
#define	MISS				0
#define HIT				1

/* debug bus models */
#define BUS_MODEL_SINGLE		1
#define BUS_MODEL_SPLIT			2

/* define memory bus transactions */
#define READ_REQUEST			1
#define READ_RESULT			2
#define PREFETCH_REQUEST		3
#define PREFETCH_RESULT			4
#define WRITE_REQUEST			5
#define MC_READ_REQUEST			6
#define MC_READ_RESULT			7
#define MC_WRITE_REQUEST		8

/* define memory subsystem ids for bus transaction src and dest */
#define NO_ONE				0
#define MEMORY				1
#define DCACHE				2
#define ICACHE				3
#define L2				4


/* 
 * X86_trace defines 
 */

#define	PREFIX_TWO_BYTE_OPCODE		0x01000000
#define	PREFIX_16_BIT_OPERANDS		0x02000000

/* General record bits */
#define X86_NORMAL_INSTR		0x00000001

/* Normal instruction record bits */
#define X86_CONTAINS_MEM_ADDR		0x00000008
#define X86_CONTAINS_BR_TARGET		0x00000010
#define X86_UNTRACED_CALL		0x00000200

/* Info record bits */
#define X86_TEXTUAL_INFO		0x00000010
#define X86_SAMPLE_START_ADDR		0x00000020
#define X86_INSTRUCTIONS_SKIPPED	0x00000030
#define X86_SAMPLE_SIZE_PARM		0x00000040
#define X86_SKIP_SIZE_PARM		0x00000050
#define X86_START_OF_TRACE		0x000000E0
#define X86_END_OF_TRACE		0x000000F0


/* Simulation defines */
#define MAX_LINE_SIZE           5000
#define READER_VERSION          7
#define OBJTR_VERSION           8
#define OPERAND_HASH_SIZE       1024	/* Must be power of two */
#define FN_HASH_SIZE            1024	/* Must be power of two */
#define STORE_HASH_SIZE		64	/* Must be power of two */
#define MAX_POSITIVE_SHORT      32767
#define MIN_NEGATIVE_SHORT      -32767
#define TRACE_BLOCK_SIZE	2048
#define MAX_LAT			4	/* Latency arrays size */


/* Global error number varable used by the pipe/file functions */
extern int errno;

/* Structure prototypes */
struct Stats;
struct Dcache_Stats;
struct BTB;
struct BTB_data;
struct BTB_Stats;
struct Bus_Stats;
struct L2_Bus_Stats;
struct L2_Stats;
struct VLIW;
struct VLIW_Data;
struct VLIW_Stats;
struct Superscalar;
struct Superscalar_Data;
struct Superscalar_Stats;
struct Mentry;
struct Mqueue;


/*
 * Source code structures
 */
typedef struct S_Operand
{
  int id;			/* < 0 if const, > 0 if register */
  int flags;			/* Must be 32 bits */
  int hash_value;		/* Used to speed linked list search */
  char *string;
  struct S_Operand *next_hash;
}
S_Operand;

typedef struct S_Loop
{
  int loop_id;
  int iter;                     /* Iter count is reset for each instance */
  int instance;
}
S_Loop;

typedef struct S_Oper
{
  int pc;			/* pc in final layout */
  int flags;			/* Must be 32 bits */
  int playdoh_flags;		/* Must be 32 bits */
  struct S_Cb *cb;		/* Cb that op is in */
  struct S_Loop *loop;		/* Loop that op is in */
  int lcode_id;
  int opc;			/* Lcode internal opcode */
  int proc_opc;			/* Processor specific opcode */
  short *operand;		/* Array of indexes into operand tab */
  int packet_id;		/* Unique id of for a VLIW packet */
  int cycle;			/* Scheduled cycle, 0 if unscheduled */
  int slot;			/* Scheduled slot, -1 if unscheduled */
  char virtual_latency[MAX_LAT];/* Sched latency, -1 if unsched */
  char max_virtual_latency;
  char real_latency[MAX_LAT];
  char max_real_latency;
  int adjust_real_latency;
  int dep_id;			/* DEP id (sync arcs), 0 if no id */
  int last_addr;		/* Last mem addr accessed */
  int br_target;		/* Branch target cb id */
  int trace_words_read;		/* Trace words to skip while tracing */
  int instr_addr;		/* Address in image */
  int instr_size;		/* In bytes */
  int instr_desc;		/* Compressed binary for instruction */
  struct Stats *stats;
/* Added capability for per-address BTB schemes */
  int BTB_branch_history;
  char *BTB_prediction_table;
}
S_Oper;

typedef struct S_Cb
{
  struct S_Fn *fn;		/* Function cb is in */
  int lcode_id;
  int start_pc;			/* First op in cb, -1 if cb doesn't exist */
  int *preheaders;
}
S_Cb;

typedef struct S_Fn
{
  char *name;
  char *asm_name;		/* Name as appears in assembly code */
  int addr;			/* Address in executable */
  S_Oper *op;			/* Array of ops (pointer to storage) */
  S_Cb *cb;			/* Array of cbs (indexed by lcode_id) */
  int *jsr_tab;			/* Array of pcs (indexed by jsr_id) */
  int op_count;			/* ops in fn */
  int max_cb;			/* max legal cb */
  int max_jsr_id;		/* max legal jsr_id */
  struct S_Guide_Table *guide_table;
  /* Access to store id->pc table lookup */
  struct S_Fn *next_fn;		/* For linked list of functions */
  struct S_Fn *next_hash;	/* For hash table by address */
  List loops;
}
S_Fn;

typedef struct S_Opc_Info
{
  char *name;			/* Lcode name */
  int opc_type;			/* UNTRACED_OP, CBR_OP, etc */
  int opc;			/* For debugging */
  char *opc_type_name;		/* For debugging */
  int virtual_latency;		/* Latency scheduled for */
  int real_latency;		/* Actual latency in processor */
  int is_branch;		/* Flags if branch opcode */
  int access_size;		/* Mem access size */
  unsigned conflict_mask;	/* ~(access_size -1) */
  void (*trace_info) ();	/* Reads trace for opcode */
  void (*guess_info) ();	/* Guesses info for off trace path */
  int trace_words_read;		/* Trace words to skip while tracing */
}
S_Opc_Info;

/* 
 * Simulation structures
 */

typedef struct Scblock
{
  unsigned start_addr;		/* Block beginning address */
  void *data;			/* Pointer to data block holds */
  struct Scblock *tag_next;	/* For tag table linked list */
  struct Scblock *tag_prev;
  struct Scblock *hash_next;	/* For hash table linked list */
  struct Scblock *hash_prev;
}
Scblock;

/* Linked list of blocks that match a given tag */
typedef struct Ststore
{
  Scblock *head;		/* Most recently used block */
  Scblock *tail;		/* Least recently used block */
}
Ststore;

typedef struct Scache
{
  int block_size;		/* size in bytes (or addressable elements) */
  int cache_size;		/* total size in bytes */
  int assoc;			/* For full, set to cache_size/block_size */
  unsigned block_id_shift;	/* # to right shift to get block id */
  unsigned start_addr_mask;	/* to strip addr bits for byte inside block */
  unsigned tag_index_mask;	/* to strip out tag index from shifted addr */
  unsigned hash_index_mask;	/* to strip out hash index from shifted addr */
  Ststore *tag_store;		/* Index by tag index */
  Scblock **hash;		/* Index by hash index */
}
Scache;

typedef struct Squeue
{
  char *name;			/* Name of queue */
  int size;			/* Number of sints in queue */
  struct Sq_entry *head;	/* First entry in queue */
  struct Sq_entry *tail;	/* Last entry in queue */
}
Squeue;

typedef struct Sq_entry
{
  Squeue *queue;		/* The queue this entry is in */
  struct Sint *sint;		/* The sint this entry holds */
  struct Sq_entry *next_entry;	/* next sint in queue */
  struct Sq_entry *prev_entry;	/* prev sint in queue */
  struct Sq_entry *next_queue;	/* next queue this sint is in */
  struct Sq_entry *prev_queue;	/* prev queue this sint is in */
}
Sq_entry;

typedef struct Sdep
{
  int flags;			/* PURE, ANTI or OUTPUT DEPENDENCE */
  short from_index;		/* index of source operand of dep arc */
  short to_index;		/* index of dest operand of dep arc */
  struct Sint *from_sint;	/* source sint of dep arc */
  struct Sint *to_sint;		/* dest sint of dep arc */
  struct Sdep *from_next_dep;	/* output deps are in a single linked list */
  struct Sdep *to_next_dep;	/* input deps are in a double linked list */
  struct Sdep *to_prev_dep;
}
Sdep;

typedef struct Trace_Info
{
  int mem_addr;			/* load/store address */
  int target_pc;		/* branch target pc */
  int mem_copy_buf;		/* mem_copy buffer address */
  int mem_copy_array;		/* mem_copy array address */
  int mem_copy_setup;		/* mem_copy setup info */
  int mem_copy_count;		/* mem_copy count */
  int mem_copy_requests;	/* mem_copy elements fetched */
}
Trace_Info;

typedef struct Sint
{
  int serial_no;		/* Unique serial number for sint */
  S_Fn *fn;			/* Source function */
  S_Oper *oper;			/* Source oper, all the operation info */
  int flags;			/* Must be 32 bits, used by simulation */
  int playdoh_flags;		/* Must be 32 bits, used so we can modify 
				   playdoh_flags from S_oper */
  Trace_Info trace;		/* Trace info structure */
  int access_size;		/* Mem access size */
  unsigned conflict_mask;	/* For determining load/store addr conflicts */
  unsigned real_complete_time[MAX_LAT];	  /* Real Time out of FU */
  unsigned virtual_complete_time[MAX_LAT];/* Virtual (promised) time out of FU */
  Sq_entry *entry_list;		/* Linked list of Sq_entries */
  struct Stats *stats;		/* Stats structure for this operation */
  int dynamic_cb_id;		/* Dynamic cb id of sint */
  int fetch_cycle;		/* Cycle sint was fetched */

  void *proc_data_v;		/* Processor-specific data */

  struct Superscalar_Data *superscalar;	  /* For debugging only */
  struct VLIW_Data *vliw;	          /* For debugging only */
}     
Sint;

typedef struct Reg_File
{
  /* Fields used by decoder scoreboard logic */
  Sint *tag;			/* Defining sint (as seen by decoder) */
  int value_avail;		/* Flags if value available (for execution) */
  int adjust;
  /* 
   * Fields used to track register file contents and pending definitions
   * of the register.  Used by predicate-enhanced branch predictors to
   * make predictions on the accessible predicate value which may not
   * be the correct predicate value.  Currently only predicate register
   * values are tracked.
   */
  Sint *last_def_fetched;	/* NULL=>none in flight, on-path sints only */
  int accessible_value;		/* Val avail that cycle, may be out of date */
  unsigned value_cb_id;		/* Dynamic cb id of sint that wrote value */
  unsigned value_fetch_cycle;	/* Fetch cycle of sint that wrote value */
  S_Oper *value_oper;		/* S_Oper that wrote value */
}
Reg_File;

typedef struct Trace_Block
{
  int trace_block[TRACE_BLOCK_SIZE];
  int size;
}
Trace_Block;

typedef struct Trace_Manager
{
  char *name;
  struct Pnode *pnode;
  Trace_Block *trace_block;
  int *trace_ptr;		/* Ptr into trace block buf */
  int size_left;		/* Ints left to read in trace block */
}
Trace_Manager;

typedef struct MCB_Entry
{
  int valid;			/* Valid bit */
  int reg;			/* Register number entry is for */
  int access_size;		/* Size of memory access */
  int low_bits;			/* Lower three bits of address */
  int checksum;			/* Used for reducing false conflicts */
  int addr;			/* For Debuging purposes only */
  S_Oper *oper;			/* For Debuging purposes only */
  struct MCB_Entry *next;
  struct MCB_Entry *prev;
}
MCB_Entry;

typedef struct MCB_Line
{
  MCB_Entry *head;		/* Most recently added entry */
  MCB_Entry *tail;		/* Least recently added entry */
}
MCB_Line;


typedef struct MCB
{
  struct Pnode *pnode;
  int *conflict;		/* Indexed by register number */
  MCB_Entry **reg_entry;	/* Indexed by register number */
  MCB_Line *line;		/* Indexed by MCB line number */
  unsigned line_index_mask;	/* to pull out line index */
  unsigned checksum_mask;	/* For compairing subset of bits */
  int index_size;		/* in bits, log2 of # of lines */
  int hash_size;		/* Domain to hash into */
  S_Oper *last_flushed;		/* For debug, last that flushed MCB */
  int context_switched;		/* For debug, flush due to switch */
}
MCB;

typedef struct ALAT_Entry
{
  int valid;			/* Valid bit */
  int regTAG;
  int addrTAG;
  int access_size;		/* Size of memory access */
  int reg;			/* Register number entry is for */
  int penalty;			/* penalty for conflict */
  int addr;			/* For Debuging purposes only */
  int low_bits;			/* Lower three bits of address */
  S_Oper *oper;			/* For Debuging purposes only */
  struct ALAT_Entry *next;
  struct ALAT_Entry *prev;
}
ALAT_Entry;

typedef struct ALAT_Line
{
  ALAT_Entry *head;		/* Most recently added entry */
  ALAT_Entry *tail;		/* Least recently added entry */
}
ALAT_Line;

typedef struct ALAT_table_entry
{
  char *func;
  int ld_id;
  int st_cnt;
  int benefit_id;
  int penalty_id;
  int last_exec;
  int penalty;
  int benefit;
  struct ALAT_table_entry *nxt;
}
ALAT_table_entry;

typedef struct ALAT_buffer_entry
{
  int id;
  int cycle;
  unsigned addr;
  unsigned access_size;
}
ALAT_buffer_entry;

typedef struct ALAT_buffer
{
  ALAT_buffer_entry buffer[200];
  int position;
}
ALAT_buffer;

typedef struct ALAT
{
  struct Pnode *pnode;
  int *conflict;		/* Indexed by register number */
  ALAT_Line *line;		/* Indexed by ALAT line number */
  unsigned line_index_mask;	/* to pull out line index */
  int index_size;		/* in bits, log2 of # of lines */
  S_Oper *last_flushed;		/* For debug, last that flushed MCB */
  int context_switched;		/* For debug, flush due to switch */

  int symboltable;
  int opsymboltable;
  ALAT_buffer stbuffer;
  ALAT_buffer bpbuffer;
}
ALAT;



typedef struct Icache
{
  char *name;
  struct Pnode *pnode;
  Scache *cache;
  int sent_request;		/* Has a request been sent? */
  int request_addr;		/* requested addr of miss */
}
Icache;


/*
 * Cycle and instruction characterists stats
 */
typedef struct Region_Stats
{
  /* Stats */
  unsigned num_entries;
  unsigned num_exits;
  unsigned num_sim_cycles;
  unsigned num_sim_on_path;
  unsigned num_skip_on_path;
  unsigned billions_skipped;

  unsigned num_packets_sim_on_path;
  unsigned num_packets_skip_on_path;
  unsigned billion_packets_skipped;

  /* All these stats are for onpath only */
  unsigned branches;
  unsigned untraced_jsrs;
  unsigned longjmps;

  unsigned promoted_loads;
  unsigned promoted_non_trapping_loads;
  unsigned promoted_masked_seg_faults;
  unsigned promoted_masked_bus_errors;
  unsigned promoted_masked_both_traps;
  unsigned promoted_added_buffer_pages;
  unsigned speculative_loads;
  unsigned speculative_non_trapping_loads;
  unsigned speculative_masked_seg_faults;
  unsigned speculative_masked_bus_errors;
  unsigned speculative_masked_both_traps;
  unsigned speculative_added_buffer_pages;
  unsigned unmarked_loads;
  unsigned unmarked_non_trapping_loads;
  unsigned unmarked_masked_seg_faults;
  unsigned unmarked_masked_bus_errors;
  unsigned unmarked_masked_both_traps;
  unsigned unmarked_added_buffer_pages;

  unsigned total_preds;
  unsigned promoted_preds;
  unsigned promoted_unpreds;
  unsigned total_preds_squashed;
  unsigned promoted_preds_squashed;

  /* Variables used for calculating stats efficiently */
  unsigned entry_cycle;
}
Region_Stats;

/* 
 * Histogram stats (for lack of a better name)
 * Generates histograms on branch behaivor.
 */
typedef struct Histogram_Stats
{
  L_Histogram *ops_between_branches;
  L_Histogram *cycles_between_branches;
  L_Histogram *ops_between_mispredictions;
  L_Histogram *cycles_between_mispredictions;
}
Histogram_Stats;


/*
 * MCB stats
 */
typedef struct MCB_Stats
{
  unsigned beqs;
  unsigned loads;
  unsigned true_conflicts;
  unsigned false_conflicts;
  unsigned load_load_conflicts;
  unsigned load_store_conflicts;
  unsigned context_switch_conflicts;
  unsigned load_load_signals;
  unsigned load_store_signals;
}
MCB_Stats;

/*
 * ALAT stats
 */
typedef struct ALAT_Stats
{
  unsigned beqs;
  unsigned loads;
  unsigned true_conflicts;
  unsigned false_conflicts;
  unsigned load_load_conflicts;
  unsigned load_store_conflicts;
  unsigned context_switch_conflicts;
  unsigned load_load_signals;
  unsigned load_store_signals;
}
ALAT_Stats;

/* 
 * Icache stats
 */
typedef struct Icache_Stats
{
  unsigned hits;
  unsigned misses;
  unsigned blocks_kicked_out;
  unsigned split_blocks;
}
Icache_Stats;


/*
 * Stats for whole simulation
 */
typedef struct Stats
{
  char *name;
  int id;
  Region_Stats *region;
  void *processor_v;
  struct BTB_Stats *btb;
  MCB_Stats *mcb;
  ALAT_Stats *alat;
  struct L2_Bus_Stats *L2_bus;
  struct Bus_Stats *bus;
  Icache_Stats *icache;
  struct Dcache_Stats *dcache;
  struct L2_Stats *L2_cache;
  Histogram_Stats *histogram;

  struct Stats *next;
}
Stats;

/*
 * P(rocessing)node
 */
typedef struct Pnode
{
  char *name;
  int id;
  Trace_Manager *trace_manager;
  void *processor_v;		        /* Processor's data structure */
  struct Superscalar *superscalar;	/* Used for debugging */
  struct VLIW *vliw;		        /* Used for debugging */
  Icache *icache;
  struct Dcache *dcache;
  struct L2_Cache *scache;
  struct BTB *btb;
  MCB *mcb;
  ALAT *alat;

  /* Variables for communication between simulations */
  int icache_busy;		        /* Flags, no requests accepted */
  int icache_addr_requested;
  int icache_bytes_returned;
  int dcache_busy;
  Squeue *dcache_request;
  Stats *stats;
}
Pnode;

/* HCH 10-20-99 */
typedef struct Check_line
{
  unsigned int specid;
  int num_entries;
  unsigned char taken_data[32];
}
Check_line;

typedef struct _S_Outstanding_Load
{
  int os;
  int dcache_miss;
}
S_Outstanding_Load;

/*
 * Swaps byte order in word 'W'.  Used to convert architectures with
 * different byte orders (assumes 32 bit words).
 * Please report bugs found in this macro to John Gyllenhaal.
 */
#define SWAP_BYTES(W)           ((((unsigned long) W) << 24) | \
                                 ((((unsigned long) W) >> 24) & 0xff) | \
                                 ((((unsigned long) W) >> 8) & 0xff00) | \
                                 ((((unsigned long) W) & 0xff00) << 8))


/* 
 * TLJ 7/17/96 - Returns either the sint's opc or, if this is
 * a playdoh simulation and the PLAYDOH_PREFETCH flag is set, return
 * the prefetch opc.
 * (Used to access opc_info_tab)
 */
#define GET_OPC(s)	((S_processor_model == PROCESSOR_MODEL_PLAYDOH_VLIW && \
			 s->oper->playdoh_flags & PLAYDOH_PREFETCH) ? \
			 Lop_PREF_LD : s->oper->opc)


/* So I don't miscount the zeros */
#define BILLION 1000000000

/* For ansi */

#define STATS_ADD(name) dest-> name = src1-> name + src2-> name
#define STATS_ZERO(name) stats-> name = 0

/* Prototypes */
void S_calc_fn_addresses_from_order (S_Fn * fn_list);
void S_read_fn_addresses_from_list (S_Fn * fn_list, char *list_file_name);
void S_read_fn_addresses_from_exec (S_Fn * fn_list, char *exec_name);
int S_get_trace_word (Pnode * pnode);
int S_peek_trace_word (Pnode * pnode);
char *S_alloc_string (int size);
S_Fn *S_get_fn (int fn_addr);
Sint *S_gen_sint (Pnode * pnode, int pc, int read_trace);
void S_free_sint (Sint * sint);
void S_print_sint (FILE * out, Sint * sint);
void S_print_only_sint (FILE * out, Sint * sint);
void S_print_code (FILE * out);
void S_print_queue (FILE * out, Squeue * queue);
void print_debug_info (Pnode * pnode, Sint * sint);
/* 10/25/04 REK Set the noreturn attribute for GCC so the compiler knows S_punt
 *              exits. */
void S_punt (char *fmt, ...)
#ifdef __GNUC__
     __attribute__ ((noreturn))
#endif
     ;
void Pstats (char *fmt, ...);
int S_is_power_of_two (int number);
int S_log_base_two (int number);
void S_load_code (char *file_name);
void S_load_opc_info (char *file_name);
void S_print_opc_info_tab (FILE * out);
void S_update_region_cycle_counts (Sint * sint, int sim_cycle);
int S_get_BTB_prediction (Pnode * pnode, Sint * branch_sint);
void S_free_processor_data (Sint *sint);
/* HCH 10-26-99 */
void S_gen_check_line_entry (unsigned int checkid, int hb_reached);
void S_set_processor_model ();
int S_skip (int pc, int skip_count);
void S_print_queues_in (FILE *out, Sint *sint);
int S_sim_MCB_verify (Pnode *pnode, Sint *sint);
void S_trace_jmp_finish (Pnode * pnode, Sint * sint);
void S_trace_jsr_finish (Pnode * pnode, Sint * sint);
void S_trace_rts_finish (Pnode * pnode, Sint * sint);
void S_read_trace_info (Pnode * pnode, Sint * sint);
void S_close_trace_fd ();
void S_init_sload_data (void);
void S_finish_sload_data (void);
void S_check_fn_head ();
void S_dump_load (Sint * sint);
void S_dump_store (Sint * sint);

/* VLIW prototypes */
void S_layout_vliw_code ();
void S_layout_playdoh_vliw_code();


/* Queue routines.  Warning: dequeuing frees memory */
Squeue *S_create_queue (char *name, int id);
Sq_entry *S_enqueue (Squeue * queue, Sint * sint);
void S_dequeue (Sq_entry * entry);
void S_dequeue_from_all (Sint * sint);
Sq_entry *S_enqueue_before (Squeue * queue, Sint * sint, Sq_entry * before);
void S_move_entry_before (Squeue * new_queue, Sq_entry * entry,
			  Sq_entry * before);



/* Mqueue routines.  Warning: dequeuing frees memory */
struct Mqueue *M_create_queue (char *name, int id);
struct Mentry *M_enqueue (struct Mqueue *queue, int type, int src, int dest,
			  int addr, int size, int serial_no, int cycle,
			  Stats * stats);
void M_dequeue (struct Mentry *entry);
void M_move_entry_before (struct Mqueue *new_queue, struct Mentry *entry,
			  struct Mentry *before);


/* Cache routines. */
Scache *S_create_cache (int cache_size, int block_size, int assoc,
			void *(*create_data) ());
Scblock *S_cache_find_addr (Scache * cache, unsigned addr);

/* To be used in conjunction with S_cache_change_addr to
 * allocate a block (possibly kicking a block out) for a 
 * new address
 */
Scblock *S_cache_find_LRU (Scache * cache, unsigned for_addr);

/*
 * Must change address to one that falls on same cache line!
 * Should only be used after doing a S_cache_find_LRU on new_addr
 */
void S_cache_change_addr (Scache * cache, Scblock * block, unsigned new_addr);
void S_cache_make_MRU (Scache * cache, Scblock * block);
void S_cache_make_LRU (Scache * cache, Scblock * block);
void S_cache_invalidate (Scache * cache, Scblock * block);

void S_cache_print (FILE * out, Scache * cache);
void S_cache_contents_print (FILE * out, Scache * cache);


/* Creation routines */
Pnode *S_create_pnode (int id);
Trace_Manager *S_create_trace_manager (Pnode * pnode);
void *S_create_processor (Pnode * pnode);
struct Superscalar *S_create_superscalar (Pnode * pnode);
struct VLIW *S_create_vliw (Pnode * pnode);
struct VLIW *S_create_playdoh_vliw (Pnode *pnode);
Icache *S_create_icache (Pnode * pnode);
struct Dcache *S_create_dcache (Pnode * pnode);
struct L2_Cache *S_create_L2 (Pnode * pnode);
struct BTB *S_create_BTB (Pnode * pnode);
MCB *S_create_MCB (Pnode * pnode);
ALAT *S_create_ALAT (Pnode * pnode);

/* Parameter file routines */
void S_read_parm_system (Parm_Parse_Info * ppi);
void S_read_parm_processor (Parm_Parse_Info * ppi);
void S_read_parm_superscalar (Parm_Parse_Info * ppi);
void S_read_parm_vliw (Parm_Parse_Info * ppi);
void S_read_parm_BTB (Parm_Parse_Info * ppi);
void S_read_parm_MCB (Parm_Parse_Info * ppi);
void S_read_parm_ALAT (Parm_Parse_Info * ppi);
void S_read_parm_mem (Parm_Parse_Info * ppi);
void S_read_parm_bus (Parm_Parse_Info * ppi);
void S_read_parm_L2 (Parm_Parse_Info * ppi);
void S_read_parm_L2_bus (Parm_Parse_Info * ppi);
void S_read_parm_icache (Parm_Parse_Info * ppi);
void S_read_parm_dcache (Parm_Parse_Info * ppi);
void S_read_parm_x86_trace (Parm_Parse_Info * ppi);
void S_read_parm_profile (Parm_Parse_Info * ppi);
void S_read_parm_playdoh_vliw (Parm_Parse_Info *ppi);

/* Print configuration routines */
void S_print_configuration_profiling (FILE * out);
void S_print_configuration_system (FILE * out);
void S_print_configuration_processor (FILE * out);
void S_print_configuration_superscalar (FILE * out);
void S_print_configuration_vliw (FILE * out);
void S_print_configuration_MCB (FILE * out);
void S_print_configuration_ALAT (FILE * out);
void S_print_configuration_memory (FILE * out);
void S_print_configuration_bus (FILE * out);
void S_print_configuration_L2 (FILE * out);
void S_print_configuration_L2_bus (FILE * out);
void S_print_configuration_icache (FILE * out);
void S_print_configuration_dcache (FILE * out);
void S_print_configuration_pcache (FILE * out);
void S_print_configuration_BTB (FILE * out);
void S_print_configuration_x86_trace (FILE *out);
void S_print_configuration_playdoh_vliw (FILE *out);

/* Profiling routines */
void S_init_profiler ();
int S_profile (int pc, int skip_count);
void S_profile_simulation_instruction (int pc, Sint *sint, int on_path);
void S_write_profile ();

/* x86 trace routines */
void S_load_binmap (char *file_name);
int S_write_x86_trace (int pc, int S_sample_size);
void S_write_x86_info_int (int info_flag, int word);
void S_write_x86_end_of_trace ();

/* Simulation routines */
void S_sim_icache_first_half_cycle (Pnode * pnode);
void S_sim_icache_second_half_cycle (Pnode * pnode);

void S_init_opcode_trace ();

void S_init_bus ();
void S_sim_bus ();
void S_sim_bus_transaction (int type, int src, int dest, int addr, int size,
			    Stats * stats);
void S_print_bus_state (FILE * out);

void S_init_L2_bus ();
void S_sim_L2_bus ();
void S_sim_L2_bus_transaction (int type, int src, int dest, int addr,
			       int size, int playdoh_flags, Stats * stats);
void S_print_L2_bus_state (FILE * out);

void S_init_L2 ();
void S_sim_L2_first_half_cycle ();
void S_sim_L2_second_half_cycle ();

void S_init_memory ();
void S_sim_memory_first_half_cycle ();
void S_sim_memory_second_half_cycle ();

void S_init_buffer_page_support ();
void S_sim_buffer_page_support (Sint * sint);

void S_sim_dcache_first_half_cycle (Pnode * pnode);
void S_sim_dcache_second_half_cycle (Pnode * pnode);

int S_sim_processor (Pnode * pnode, int pc, unsigned int sample_size);
int S_sim_superscalar (Pnode * pnode, int pc, unsigned int sample_size);
int S_sim_vliw (Pnode * pnode, int pc, unsigned int sample_size);
int S_sim_playdoh_vliw (Pnode *pnode, int pc, unsigned sim_count);
int S_sim_nyfo_vliw (Pnode * pnode, int pc, unsigned int sample_size);

/* HCH 11/13/00: for mem address tracking */
extern void S_add_loop_info (Sint * sint);

/* MCB routines */
void S_sim_MCB_load (Pnode * pnode, Sint * sint);
int S_sim_MCB_beq (Pnode * pnode, Sint * sint);
void S_sim_MCB_store (Pnode * pnode, Sint * sint);
void S_sim_MCB_taken_branch (Pnode * pnode, Sint * sint);
void S_sim_MCB_context_switch (Pnode * pnode, int pc);

/* ALAT routines */
int S_sim_ALAT_load (Pnode * pnode, Sint * sint, unsigned cycle);
void S_sim_ALAT_store (Pnode * pnode, Sint * sint);
void S_sim_ALAT_context_switch (Pnode * pnode, int pc);
void S_sim_ALAT_taken_branch (Pnode * pnode, Sint * sint);
void S_sim_ALAT_all_ops (Pnode * pnode, Sint * sint, unsigned cycle);

/* Stats Creation routines */

Stats *S_create_stats (char *name);

Region_Stats *S_create_stats_region ();
void *S_create_stats_processor ();
struct Superscalar_Stats *S_create_stats_superscalar ();
struct VLIW_Stats *S_create_stats_vliw ();
struct VLIW_Stats *S_create_stats_playdoh_vliw ();
struct BTB_Stats *S_create_stats_btb ();
MCB_Stats *S_create_stats_mcb ();
ALAT_Stats *S_create_stats_alat ();
Icache_Stats *S_create_stats_icache ();
struct Dcache_Stats *S_create_stats_dcache ();
struct L2_Stats *S_create_stats_L2 ();
struct L2_Bus_Stats *S_create_stats_L2_bus ();
struct Bus_Stats *S_create_stats_bus ();
Histogram_Stats *S_create_stats_histogram ();

/* Stats region adding routines */

void S_add_stats (Stats * dest, Stats * src1, Stats * src2);

void S_add_stats_Region (Region_Stats * dest,
			 Region_Stats * src1, Region_Stats * src2);
void S_add_stats_processor (void *dest, void *src1, void *src2);
void S_add_stats_superscalar (struct Superscalar_Stats *dest,
			      struct Superscalar_Stats *src1,
			      struct Superscalar_Stats *src2);
void S_add_stats_VLIW (struct VLIW_Stats *dest,
		       struct VLIW_Stats *src1, struct VLIW_Stats *src2);
void S_add_stats_playdoh_VLIW (struct VLIW_Stats *dest,
			       struct VLIW_Stats *src1,
			       struct VLIW_Stats *src2);

void S_add_stats_BTB (struct BTB_Stats *dest,
		      struct BTB_Stats *src1, struct BTB_Stats *src2);

void S_add_stats_MCB (MCB_Stats * dest, MCB_Stats * src1, MCB_Stats * src2);

void S_add_stats_ALAT (ALAT_Stats * dest,
		       ALAT_Stats * src1, ALAT_Stats * src2);

void S_add_stats_Icache (Icache_Stats * dest,
			 Icache_Stats * src1, Icache_Stats * src2);

void S_add_stats_Dcache (struct Dcache_Stats *dest,
			 struct Dcache_Stats *src1,
			 struct Dcache_Stats *src2);

void S_add_stats_Bus (struct Bus_Stats *dest,
		      struct Bus_Stats *src1, struct Bus_Stats *src2);

void S_add_stats_L2 (struct L2_Stats *dest,
		     struct L2_Stats *src1, struct L2_Stats *src2);
void S_add_stats_L2_Bus (struct L2_Bus_Stats *dest,
			 struct L2_Bus_Stats *src1,
			 struct L2_Bus_Stats *src2);

void S_add_stats_Histogram (Histogram_Stats * dest,
			    Histogram_Stats * src1, Histogram_Stats * src2);

/* Global Stats Printing routines */
void S_print_stats_global (FILE * out);

/* System stats cannot be taken for region */
void S_print_stats_global_system (FILE *out);

/* These global stats routines may go away */
void S_print_stats_global_Region (FILE * out);
void S_print_stats_global_processor (FILE * out);
void S_print_stats_global_Superscalar (FILE * out);
void S_print_stats_global_BTB (FILE * out);
void S_print_stats_global_MCB (FILE * out);
void S_print_stats_global_ALAT (FILE * out);
void S_print_stats_global_Icache (FILE * out);
void S_print_stats_global_Dcache (FILE * out);
void S_print_stats_global_Bus (FILE * out);

/* Region Stats Printing routines */
void S_print_stats_region (FILE * out);

void S_print_stats_region_Region (FILE * out, Stats * stats,
				  char *rname, Stats * total_stats);
void S_print_stats_region_processor (FILE * out, Stats * stats,
				     char *rname, Stats * total_stats);
void S_print_stats_region_superscalar (FILE * out, Stats * stats,
				       char *rname, Stats * total_stats);
void S_print_stats_region_VLIW (FILE * out, Stats * stats,
				char *rname, Stats * total_stats);
void S_print_stats_region_playdoh_VLIW (FILE *out, Stats *stats, char *rname,
					Stats *total_stats);
void S_print_stats_region_BTB (FILE * out, Stats * stats,
			       char *rname, Stats * total_stats);
void S_print_stats_region_MCB (FILE * out, Stats * stats,
			       char *rname, Stats * total_stats);
void S_print_stats_region_ALAT (FILE * out, Stats * stats,
				char *rname, Stats * total_stats);
void S_print_stats_region_Icache (FILE * out, Stats * stats,
				  char *rname, Stats * total_stats);
void S_print_stats_region_Dcache (FILE * out, Stats * stats,
				  char *rname, Stats * total_stats);
void S_print_stats_region_Bus (FILE * out, Stats * stats,
			       char *rname, Stats * total_stats);
void S_print_stats_region_L2 (FILE * out, Stats * stats,
			      char *rname, Stats * total_stats);
void S_print_stats_region_L2_bus (FILE * out, Stats * stats,
				  char *rname, Stats * total_stats);
void S_print_stats_region_Histogram (FILE * out, Stats * stats,
				     char *rname, Stats * total_stats);

/* Parameters determined from the trace header */
extern unsigned int S_trace_flags;
extern int S_use_func_ids_not_addrs;
extern int S_trace_pred_defs;
extern int S_trace_byte_order_reversed;
extern int S_trace_promoted_predicates;

/* Global parameters */
extern char *S_mode_name;
extern char *S_processor_model_name;
extern char *S_processor_type_name;
extern int S_simulation_with_profile_information;
extern int S_region_stats;
extern int S_nice_value;
extern char *S_source_file;
extern char *S_histogram_file_name;
extern char *S_loop_map_file_name;

/* HCH 10-20-99 */
extern char *S_sload_file_name;
extern int S_gen_sload_data;

extern int S_print_branch_histograms;
extern int S_program_start_addr;
extern char *S_trace_file;
extern char *S_addr_file;
extern int S_use_file_mode;
extern int S_read_addr_file;
extern char *S_exec_name;
extern char *S_trace_command;
extern int S_timeout_delay;
extern char *S_mdes_file;
extern char *S_opc_info_file;
extern int S_print_opc_info;
extern char *S_debug_output_file;
extern int S_dump_code_image;
extern unsigned S_sample_size;
extern unsigned S_skip_size;
extern unsigned S_initialization_skip_size;
extern unsigned S_max_sample_count;
extern unsigned S_stop_sim_trip_count;
extern int S_debug_stop_sim_markers;
extern int S_debug_force_sim_markers;
extern int S_use_skipped_memory_addresses;
extern int S_memory_latency_scale_factor;
extern int S_memory_latency_delta_factor;
extern int S_move_latency_scale_factor;
extern int S_move_latency_delta_factor;
extern int S_ialu_latency_scale_factor;
extern int S_ialu_latency_delta_factor;
extern int S_falu_latency_scale_factor;
extern int S_falu_latency_delta_factor;
extern int S_default_latency_scale_factor;
extern int S_default_latency_delta_factor;
extern char *S_fetch_model_name;
extern int S_num_fetch_stages;
extern int S_fetch_width;
extern int S_fetch_buf_size;
extern int S_fetch_mark;
extern int S_issue_width;
extern int S_flush_pipe_on_untraced_jsr;
extern int S_read_dests_of_pred_op;
extern int S_branches_per_cycle;
extern int S_dcache_ports;
extern int S_retire_width;
extern char *S_MCB_model_name;
extern int S_MCB_size;
extern int S_MCB_assoc;
extern int S_MCB_checksum_width;
extern int S_MCB_all_loads_preloads;
extern int S_MCB_debug_load_load_conflicts;
extern int S_MCB_debug_load_store_conflicts;
extern char *S_ALAT_model_name;
extern int S_ALAT_size;
extern int S_ALAT_all_loads_preloads;
extern int S_ALAT_debug_load_load_conflicts;
extern int S_ALAT_debug_load_store_conflicts;
extern int S_memory_latency;
extern int S_memory_page_size;
extern char *S_bus_model_name;
extern char *S_L2_bus_model_name;
extern int S_debug_bus;
extern int S_debug_L2_bus;
extern int S_bus_bandwidth;
extern int S_L2_bus_bandwidth;
extern int S_streaming_support;
extern int S_L2_streaming_support;
extern char *S_icache_model_name;
extern int S_icache_size;
extern int S_icache_block_size;
extern int S_icache_assoc;
extern char *S_dcache_model_name;
extern int S_dcache_size;
extern int S_dcache_block_size;
extern int S_dcache_assoc;
extern int S_dcache_measure_conflict_stats;
extern int S_dcache_write_buf_size;
extern int S_dcache_combining_write_buf;
extern int S_dcache_write_allocate;
extern int S_dcache_miss_bypass_limit;
extern int S_dcache_debug_misses;
extern int S_dcache_prefetch_buf_size;
extern int S_dcache_debug_prefetch;
extern int S_dcache_ignore_prefetch_bit;
extern int S_dcache_ignore_prefetches;
extern int S_dcache_debug_mem_copy;
extern int S_mem_copy_version;	/* Version 1, 2, and 3 now defined */

extern int S_prefetch_cache;
extern int S_pcache_size;
extern int S_pcache_block_size;
extern int S_pcache_assoc;
extern int S_pcache_subblocks;
extern int S_pcache_subblock_size;

extern int S_secondary_cache;
extern int S_scache_latency;
extern char *S_scache_model_name;
extern int S_scache_size;
extern int S_scache_block_size;
extern int S_scache_assoc;
extern int S_scache_measure_conflict_stats;
extern int S_scache_write_buf_size;
extern int S_scache_combining_write_buf;
extern int S_scache_write_allocate;
extern int S_scache_miss_bypass_limit;
extern int S_scache_debug_misses;
extern int S_scache_prefetch_buf_size;
extern int S_scache_debug_prefetch;
extern int S_scache_ignore_prefetch_bit;
extern int S_scache_ignore_prefetches;

extern int S_x86_use_pipe;
extern char *S_x86_trace_binmap_file;
extern char *S_x86_trace_output_file;
extern char *S_x86_trace_desc;

/* Profiler global variables */
extern double S_num_profiled;

/* Global variables */
extern int S_mode;
extern int S_processor_model;
extern int S_processor_type;
extern int S_sched_info_avail;
extern Stats *S_program_stats;
extern Stats *S_head_stats;
extern Stats *S_tail_stats;
extern int S_program_start_pc;
extern int S_trace_fd;
extern int S_trace_command_pid;
extern int S_trace_words_read;
extern int S_function_count;
extern int S_operation_count;
extern int S_operation_count_cond;
extern int S_operation_count_pred_uncond;
extern int S_operation_count_pred_call;
extern int S_operation_count_pred_ret;
extern int S_operation_count_uncond;
extern int S_operation_count_call;
extern int S_operation_count_ret;
extern int S_fetch_model;
extern int S_bus_model;
extern int S_L2_bus_model;
extern int S_MCB_model;
extern int S_ALAT_model;
extern int S_icache_model;
extern int S_dcache_model;
extern int S_scache_model;
extern S_Fn *S_entry_fn;
extern S_Fn *head_fn;
extern S_Fn *tail_fn;
extern S_Fn *fn_hash[FN_HASH_SIZE];	/* Indexed using hash of fn address */
extern S_Oper **oper_tab;	        /* Indexed by pc */
extern S_Operand **operand_tab;	        /* by operand id (id may be negative) */
extern S_Opc_Info *opc_info_tab;	/* by opc */
extern int S_max_dest_operand;
extern int S_max_src_operand;
extern int S_max_pred_operand;
extern int S_first_dest;	        /* Index of first dest operand */
extern int S_last_dest;		        /* Index of last dest operand */
extern int S_first_src;		        /* Index of first src operand */
extern int S_last_src;		        /* Index of last src operand */
extern int S_first_pred; 	        /* Index of first pred operand */
extern int S_last_pred;		        /* Index of last pred operand */
extern int S_min_const_operand;	        /* This will be negative */
extern int S_max_register_operand;	/* This will be positive */
extern int S_max_pc;
extern int S_max_packet_id;
extern int S_max_slot;
extern int S_max_opc;
extern Pnode *S_pnode;
/* temporary BTB pointer for reading in parms */
extern struct BTB *S_temp_btb;
extern struct Bus S_bus;
extern struct Memory S_memory;
extern struct L2_Bus S_L2_bus;
extern int S_icache_miss_latency;
extern int S_dcache_read_block_latency;
extern int S_dcache_write_block_latency;
extern int S_dcache_write_thru_latency;
extern int S_dcache_streaming_benefit;
extern int S_end_of_program;	        /* Flags end of program reached */
extern int S_normal_termination;	/* Flags ended normally (not early) */
extern int S_halting_simulation;
extern int S_serial_no;
extern unsigned S_sim_cycle;
extern unsigned S_dynamic_cb_id;
extern int S_force_sim;
extern FILE *S_x86_trace_out;
extern int S_has_mem_copy_directives;
extern int S_has_prefetches;
extern int S_ops_between_branches;
extern int S_ops_between_mispredictions;
extern int S_cycle_of_last_branch;
extern int S_cycle_of_last_misprediction;
extern int S_stop_sim_markers_encountered;
extern int S_total_adjust;
extern int S_total_unadjust_cycle;

/* HCH 11/13/00: for tracking memory accesses */
extern S_Cb *cur_cb;
extern S_Loop *last_loop;
extern S_Loop *cur_loop;

/*Set to 1 if page_buffer_size is too small*/
extern int S_page_buffer_overflowed;

extern int S_pcache_read_block_latency;
extern int S_pcache_streaming_benefit;


extern Scache *S_buffer_page_cache;

extern FILE *debug_out;		        /* For debug message output */
extern FILE *S_histogram_file;
extern FILE *S_loop_map_out;
extern FILE *S_OBJ_OUT;
/* HCH 10-20-99 */
extern FILE *S_sload_file;
extern unsigned int S_max_specid;

extern int S_trace_objects;
extern int S_trace_loop_id;
extern int num_rd_collapses;
extern int num_wt_collapses;
extern char *latest_fn_name;

/* Global parameters for Pstats */
extern FILE *Pstats_out;
extern char *Pstats_rname;

/* System statistics */
extern time_t S_start_time;
extern time_t S_init_time;
extern time_t S_end_time;
extern time_t S_skip_time;
extern int S_start_nice_value;
extern int S_end_nice_value;

/* Global Simulation statistics */
extern unsigned S_num_skip_on_path;
extern unsigned S_num_sim_on_path;
extern unsigned S_billions_skipped;
extern unsigned S_num_packets_sim_on_path;
extern unsigned S_num_packets_skip_on_path;
extern unsigned S_billion_packets_skipped;
extern unsigned S_num_sim_samples;

#if 0
extern unsigned S_num_sim_branches_on_path;
extern unsigned S_num_sim_pred_on_path;
extern unsigned S_num_sim_pred_squashed;
extern unsigned S_num_sim_untraced_jsrs;
extern unsigned S_num_sim_longjmps;

extern unsigned S_num_sim_squashed;
extern unsigned S_num_sim_untraced_fixup;
extern unsigned S_num_sim_loads_forwarded;
extern unsigned S_num_sim_loads_blocked;
extern unsigned S_num_sim_cycles_loads_blocked;
extern unsigned S_sim_BTB_hits;
extern unsigned S_sim_BTB_miss_pred;
extern unsigned S_sim_BTB_miss_addr;
extern unsigned S_sim_BTB_entries_kicked_out;
extern unsigned S_sim_MCB_beqs;
extern unsigned S_sim_MCB_loads;
extern unsigned S_sim_MCB_true_conflicts;
extern unsigned S_sim_MCB_false_conflicts;
extern unsigned S_sim_MCB_load_load_conflicts;
extern unsigned S_sim_MCB_load_store_conflicts;
extern unsigned S_sim_MCB_context_switch_conflicts;
extern unsigned S_sim_MCB_load_load_signals;
extern unsigned S_sim_MCB_load_store_signals;
extern unsigned S_sim_ALAT_beqs;
extern unsigned S_sim_ALAT_loads;
extern unsigned S_sim_ALAT_true_conflicts;
extern unsigned S_sim_ALAT_false_conflicts;
extern unsigned S_sim_ALAT_load_load_conflicts;
extern unsigned S_sim_ALAT_load_store_conflicts;
extern unsigned S_sim_ALAT_context_switch_conflicts;
extern unsigned S_sim_ALAT_load_load_signals;
extern unsigned S_sim_ALAT_load_store_signals;
extern unsigned S_cycles_dcache_busy_when_needed;
extern unsigned S_cycles_dcache_ports_unavailable;
extern unsigned S_sim_icache_hits;
extern unsigned S_sim_icache_misses;
extern unsigned S_sim_icache_blocks_kicked_out;
extern unsigned S_sim_icache_split_blocks;
extern unsigned S_sim_dcache_read_hits;
extern unsigned S_sim_dcache_write_hits;
extern unsigned S_sim_dcache_reads_forwarded;
extern unsigned S_sim_dcache_read_misses;
extern unsigned S_sim_dcache_redundant_read_misses;
extern unsigned S_sim_dcache_redundant_read_misses_off_path;
extern unsigned S_sim_dcache_write_misses;
extern unsigned S_sim_dcache_writes_combined;
extern unsigned S_sim_dcache_blocks_kicked_out;
extern unsigned S_sim_dcache_dirty_blocks_kicked_out;
extern unsigned S_sim_dcache_read_hits_off_path;
extern unsigned S_sim_dcache_write_hits_off_path;
extern unsigned S_sim_dcache_reads_forwarded_off_path;
extern unsigned S_sim_dcache_read_misses_off_path;
extern unsigned S_sim_dcache_write_misses_off_path;
extern unsigned S_sim_dcache_prefetch_hits;
extern unsigned S_sim_dcache_redundant_prefetches;
extern unsigned S_sim_dcache_prefetch_buf_full;
extern unsigned S_sim_dcache_prefetch_misses;
extern unsigned S_sim_dcache_prefetch_requests_sent;
extern unsigned S_sim_dcache_loads_used_prefetch_request;
extern unsigned S_sim_dcache_prefetches_completed;
extern unsigned S_sim_dcache_prefetch_hits_off_path;
extern unsigned S_sim_dcache_redundant_prefetches_off_path;
extern unsigned S_sim_dcache_prefetch_buf_full_off_path;
extern unsigned S_sim_dcache_prefetch_misses_off_path;
extern unsigned S_sim_dcache_cycles_blocked_miss_buffer;
extern unsigned S_sim_dcache_cycles_blocked_write_buffer;
extern unsigned S_sim_dcache_cycles_blocked_mem_copy_check;

extern unsigned S_sim_dcache_num_mem_copies;
extern unsigned S_sim_dcache_num_mem_copy_tags;
extern unsigned S_sim_dcache_num_mem_copy_backs;
extern unsigned S_sim_dcache_mem_copy_blocks;
extern unsigned S_sim_dcache_mem_copy_tag_blocks;
extern unsigned S_sim_dcache_blocks_mem_copies_kicked_out;
extern unsigned S_sim_dcache_blocks_mem_copy_tags_kicked_out;
extern unsigned S_sim_dcache_mem_copy_buffer_conflicts;
extern unsigned S_sim_dcache_mem_copy_tag_buffer_conflicts;
extern unsigned S_sim_dcache_mem_copy_back_blocks;
extern unsigned S_sim_dcache_mem_copy_back_block_misses;
extern unsigned S_sim_dcache_mem_copy_back_buffer_conflicts;
extern unsigned S_sim_dcache_mem_copy_memory_requests;
extern unsigned S_sim_dcache_mem_copy_cache_requests;
extern unsigned S_sim_dcache_mem_copy_back_memory_requests;
extern unsigned S_sim_dcache_mem_copy_back_cache_requests;

extern unsigned S_sim_bus_cycles_icache_read_requests;
extern unsigned S_sim_bus_cycles_icache_read_results;
extern unsigned S_sim_bus_cycles_dcache_read_requests;
extern unsigned S_sim_bus_cycles_dcache_read_results;
extern unsigned S_sim_bus_cycles_dcache_prefetch_requests;
extern unsigned S_sim_bus_cycles_dcache_prefetch_results;
extern unsigned S_sim_bus_cycles_dcache_write_requests;
extern unsigned S_sim_bus_cycles_waiting_for_result;
extern unsigned S_sim_bus_cycles_dcache_mc_read_requests;
extern unsigned S_sim_bus_cycles_dcache_mc_read_results;
extern unsigned S_sim_bus_cycles_dcache_mc_write_requests;

#endif

/* Alloc pools */
extern L_Alloc_Pool *S_Operand_pool;
extern L_Alloc_Pool *Squeue_pool;
extern L_Alloc_Pool *Sq_entry_pool;
extern L_Alloc_Pool *Sdep_pool;
extern L_Alloc_Pool *Sint_pool;
extern L_Alloc_Pool *Pnode_pool;
extern L_Alloc_Pool *Trace_Manager_pool;
extern L_Alloc_Pool *Trace_Block_pool;
extern L_Alloc_Pool *Scache_pool;
extern L_Alloc_Pool *Icache_pool;
extern L_Alloc_Pool *Dcache_pool;
extern L_Alloc_Pool *Mem_Request_pool;
extern L_Alloc_Pool *Mqueue_pool;
extern L_Alloc_Pool *Mentry_pool;
extern L_Alloc_Pool *Stats_pool;
extern L_Alloc_Pool *Region_Stats_pool;
extern L_Alloc_Pool *Bus_Stats_pool;
extern L_Alloc_Pool *Dcache_Stats_pool;
extern L_Alloc_Pool *Superscalar_Stats_pool;
extern L_Alloc_Pool *BTB_Stats_pool;
extern L_Alloc_Pool *MCB_Stats_pool;
extern L_Alloc_Pool *ALAT_Stats_pool;
extern L_Alloc_Pool *Icache_Stats_pool;
extern L_Alloc_Pool *Histogram_Stats_pool;

#endif


/*
 * Lsim includes 
 */
#include "s_mem.h"
#include "s_dcache.h"
#include "s_scache.h"
#include "s_btb.h"
