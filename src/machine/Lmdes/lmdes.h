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
 *
 *  File:  lmdes.h
 *
 *  Description:
 *    Header file for low level machine description for architecture (mdes).
 *
 *  Creation Date :  May, 1993
 *
 *  Authors:  John C. Gyllenhaal, Scott Mahlke, Wen-mei Hwu
 *
\*****************************************************************************/
#ifndef LMDES_H
#define LMDES_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/dynamic_symbol.h>
#include <library/set.h>

/* For Lcode, casts void pointer for Mdes_Info */
#define MDES_INFO(oper) ((Mdes_Info *)(oper->mdes_info))

/* Operand/sync specifiers */
#define			MDES_PRED	0
#define			MDES_DEST	1
#define 		MDES_SRC	2
#define			MDES_SYNC_IN	3
#define			MDES_SYNC_OUT	4

/* Processor model, must be identical to hmdes.h declaration */
#define			MDES_SUPERSCALAR	0
#define			MDES_VLIW		1

/* Mdes_Rused flags */
#define MDES_OVERLAPPING_REQUEST        0x00000001

typedef struct mdes_IO_set_st
{
  int id;
  int external_id;
  char *name;
  int *mask;
}
Mdes_IO_Set;

typedef struct mdes_IO_item_st
{
  int id;
  char *name;
  Mdes_IO_Set **operand_type;
}
Mdes_IO_Item;

typedef struct mdes_resource_st
{
  int id;
  char *name;
}
Mdes_Resource;

typedef struct mdes_rmask_st
{
  int *uncond;
  int *pred;
}
Mdes_Rmask;

typedef struct mdes_Rused
{
  int flags;			/* Bit flags for Rused list entry */
  int start_usage;
  int end_usage;
  int num_options;
  Mdes_Rmask *option;
}
Mdes_Rused;

typedef struct mdes_reslist_st
{
  int id;
  char *name;
  int num_used;
  Mdes_Rused *used;
  int num_slot_options;		/* Number of slot options for this ResList */
  int *slot_options;		/* slot numbers, in mdes order (not sorted) */
  int num_RU_entries_required;
}
Mdes_ResList;

typedef struct mdes_latency_st
{
  int id;
  char *name;
  int exception;
  int *operand_latency;
}
Mdes_Latency;

typedef struct mdes_alt_st
{
  int id;
  char *asm_name;
  int alt_flags;
  struct mdes_operation_st *operation;
  Mdes_IO_Item *IO_item;
  Mdes_ResList *reslist;	/* NULL if .lmdes2 used */
  Mdes_Latency *latency;
  struct SM_Table *table;	/* NULL if .lmdes used */
}
Mdes_Alt;

typedef struct Mdes_Stats
{
  int num_oper_checks;
  int num_oper_checks_failed;
  int num_table_checks;
  int num_table_checks_failed;
  int num_option_checks;
  int num_option_checks_failed;
  int num_usage_checks;
  int num_usage_checks_failed;
  int num_slot_checks;
  int num_slot_checks_failed;
  INT_Symbol_Table *first_choice_dist;
  INT_Symbol_Table *num_choice_dist;
  INT_Symbol_Table *succeed_option_check_dist;
  INT_Symbol_Table *fail_option_check_dist;
  INT_Symbol_Table *succeed_usage_check_dist;
  INT_Symbol_Table *fail_usage_check_dist;
}
Mdes_Stats;

typedef struct mdes_operation_st
{
  int id;
  int opcode;
  char *external_name;
  int num_alts;
  Mdes_Alt *alt;
  int op_flags;
  int heuristic_alt;		/* Used for heuristics */

  /* 20030528 SZU
   * Added to keep track of port and syllable types, by hex mask value.
   * Used for scheduling and bundling.
   */
  unsigned int syll_mask;
  unsigned int port_mask;
  unsigned int num_slots;	/* Used for number of slots operation takes */
  				/* Example: L type take 2 slots */
  int nop;			/* 1 if has nop field, 0 if not */

  /* 20020805 SZU
   * Added to keep track of latency classes.
   * 20020823 SZU
   * Modified for a Set instead.
   */
  int num_lat_class;
  Set lat_class;

  /* For mdes statistics */
  Mdes_Stats can_sched_prepass;
  Mdes_Stats sched_prepass;
  Mdes_Stats can_sched_postpass;
  Mdes_Stats sched_postpass;
}
Mdes_Operation;

typedef struct mdes_st
{
  char *file_name;
  int processor_model;		/* MDES_SUPERSCALAR, MDES_VLIW */
  int number[5];		/* Number of each operand/sync type */
  int offset[5];		/* Offset for each operand/sync type */
  char *name[5];		/* Name of each operand/sync type */
  int operand_count;
  int latency_count;
  int num_slots;
  int max_slot;
  int IOmask_width;
  int num_reg_files;
  int max_IO_set_id;
  int null_external_id;
  Mdes_IO_Set **IO_set_table;	/* Indexed by external id */
  int num_IO_sets;
  Mdes_IO_Set *IO_set;
  int num_IO_items;
  Mdes_IO_Item *IO_item;
  int num_resources;
  Mdes_Resource *resource;
  int num_reslists;
  int Rmask_width;
  Mdes_ResList *reslist;
  int num_latencies;
  Mdes_Latency *latency;
  int num_operations;
  Mdes_Operation *operation;
  int max_opcode;
  Mdes_Operation **op_table;	/* Indexed by opcode */
  Mdes_IO_Set **operand_type_buf;	/* Used by Build_Mdes_Info */
  int *index_type;		/* For reverse mapping operand index */
  int *index_number;		/* For reverse mapping operand index */

  /* Start converting routines over to version2 data structures */
  struct Mdes2 *mdes2;

  /* New parameter for version2 mdes.  If set, the scheduler only
   * needs to only check the resource constraints for the first alternative
   * to match format and dependence requirements, otherwise (per original
   * specification) all alternatives need to be checked (until a match
   * is found or there are no more alternatives).  Makes describing
   * the SuperSPARC's cascadable operations more efficient.
   */
  int check_resources_for_only_one_alt;
}
Mdes;

typedef struct mdes_compatable_alt_st
{
  Mdes_Alt *alt;
  struct mdes_compatable_alt_st *next;
}
Mdes_Compatable_Alt;

typedef struct Mdes_Info
{
  int opcode;			/* For debugging */
  int num_compatable_alts;
  Mdes_Compatable_Alt *compatable_alts;	/* Linked list */
}
Mdes_Info;

/* Interface macros */
#define mdes_processor_model()	lmdes->processor_model
#define mdes_total_slots()	lmdes->num_slots

#ifdef __cplusplus
extern "C"
{
#endif

/* How the LMDES is accessed by the outside world */
  extern Mdes *lmdes;

/* Interface functions */
  extern void L_init_lmdes (char *mdes_file_name);
  extern void L_init_lmdes2 (char *mdes_file_name, int num_pred, int num_dest,
			     int num_src, int num_sync);
  extern void print_mdes (FILE * out, Mdes * mdes);
  extern int lmdes_initialized (void);	/* (void) *//* returns 1 or 0 */

  extern Mdes_Info *build_mdes_info (unsigned int opcode, int *io_list);
  extern void free_mdes_info (Mdes_Info * info);

  extern void print_mdes_operand_name (FILE * out, Mdes * mdes,
				       unsigned int index);
  extern void mdes_print_IO_set (FILE * out, unsigned int id);

  extern int mdes_defined_opcode (unsigned int opcode);

  extern int op_flag_set (unsigned int opcode, int mask);
  extern int alt_flag_set (unsigned int opcode, unsigned int alt_no,
			   int mask);
  extern int any_alt_flag_set (Mdes_Info * mdes_info, int mask);

  extern int mdes_max_opcode (void);	/* (void) */
  extern int mdes_operand_count (void);	/* (void) */
  extern int mdes_latency_count (void);	/* (void) */
  extern int mdes_num_operands (unsigned int operand_type);
  extern int mdes_null_operand (void);



  extern int operand_index (unsigned int operand_type,
			    unsigned int operand_number);
  /* Ie. (MDES_SRC, 0) */
  extern int operand_type (unsigned int operand_index);
  extern int operand_number (unsigned int operand_index);

  extern int max_operand_time (Mdes_Info * mdes_info, int operand_index);
  extern int min_operand_time (Mdes_Info * mdes_info, int operand_index);

  extern int mdes_calc_min_ready_time (Mdes_Info * mdes_info,
				       int *ready_time);

  extern int mdes_operand_latency (unsigned int opcode, unsigned int alt_no,
				   unsigned int operand_index);

  extern int mdes_max_completion_time (int opcode, int alt_no);
  extern int mdes_heuristic_alt_id (int opcode);

  extern Mdes *load_lmdes_from_version2 (char *mdes_file_name, int num_pred,
					 int num_dest, int num_src,
					 int num_sync);
  extern void Malloc_struct (void **ptr, int size);
  extern void Malloc_name (char **ptr, char *name);
  extern void print_mdes_stats (FILE * out, int prepass);
  extern void increment_check_history (INT_Symbol_Table * table,
				       int num_checks, int inc);
#ifdef __cplusplus
}
#endif

#endif
