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
 *  File:  l_probe.h
 *
 *  Description:
 *    Header file for Lcode probe generator
 *
 *  Creation Date :  March, 1993 
 *
 *  Authors:  John C. Gyllenhaal, Yoji Yamada
 *           (This is John's Lcode+ version of Yoji's code)
 *
 *      Copyright (c) 1993 John Gyllenhaal, Yoji Yamada, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef L_PROBE_H
#define L_PROBE_H

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <Lcode/l_main.h>
#include "l_trace_interface.h"

/* Track the last opcode specified in lcode -JCG 1/23/98 */
#define MAX_PROBE_INDEX         Lop_LAST_OP

#define INSERT_BEFORE 1
#define INSERT_AFTER 2

#define PREDICATE_PROBE_CODE_POSSIBLE 0
#define PREDICATE_PROBE_CODE_DEFINITE 1

#define BUF_SIZE 10000
#define BUF_MARK  5000
#define WORD_SIZE  4

#define DUMP_ROUTINE_NAME       "__TRACE_DUMP"

#define TR_PTR_MEM  "_TR_PTR_MEM"
#define TR_BUF      "__TR_MEM_BUF"

#define YES     1
#define NO      0

typedef struct probe_routine_st
{
  L_Oper *(*pre_func) ();       /* Generates probe code. parms: oper, info */
  L_Oper *(*post_func) ();      /* Generates probe code. parms: oper, info */
}
Probe_Routine;

typedef struct probe_info_st
{
  L_Func *fn;                   /* Function we are probing */
  int jsr_no;                   /* Number of jsr's seen before this oper */
  L_Operand *temp_reg;          /* Trace temp register, for ease of coding */
}
Probe_Info;

/* Global parameters */
extern int trigger_dump_size;
extern int trace_promoted_predicates;

/*Made external for file swap -JCG 6/9/95*/
extern int trace_control_flow;

/*
 * trace generation tool prototypes (used by all machines)
 */
void L_append_oper_list (L_Oper * main_list, L_Oper * append_list);
void L_initialize_Lprobe (Parm_Macro_List * external_list, L_Func * fn);
void L_probe_func_phase1 (Parm_Macro_List * external_list, L_Func * fn);
void L_probe_func_phase2 (Parm_Macro_List * external_list, L_Func * fn);
int L_matches_fn_name (char *fn_name, char *test_name);

/* Hp general functions (shared by playdoh tracing) */
L_Oper *hppa_store_tr_ptr (Probe_Info * info);
L_Oper *hppa_load_tr_ptr (Probe_Info * info);
L_Oper *hppa_write_int (int type, Probe_Info * info);
L_Oper *hppa_write_label (char *string, Probe_Info * info, int type);
L_Oper *hppa_mov_int_into_temp (int value);
L_Oper *hppa_write_reg (L_Operand * operand, Probe_Info * info);


/*
 * Hppa trace generation prototypes
 */
L_Oper *hppa_init_tracer (Probe_Info * info);
L_Oper *hppa_signal_end (Probe_Info * info);
L_Oper *hppa_trace_predicates (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_write_buffer (L_Cb * cb, Probe_Info * info);
L_Oper *hppa_func_entry (L_Func * func, Probe_Info * info);
L_Oper *hppa_func_exit (L_Func * func, Probe_Info * info);
L_Oper *hppa_jsr_setup (Probe_Info * info);
L_Oper *hppa_cb_entry (L_Cb * cb, Probe_Info * info);
L_Oper *hppa_branch_if_full (L_Cb * write_buf_cb, Probe_Info * info);
L_Oper *hppa_dump_trace_if_full (Probe_Info * info);
L_Oper *hppa_flush_buffer (Probe_Info * info);
L_Oper *hppa_detect_child (Probe_Info * info);

L_Oper *hppa_trace_jsr (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_trace_cond_branch (L_Oper * oper, Probe_Info * info);

/* Traces both normal and non_trapping loads (since probei instructions
 * may be used)
 */
L_Oper *hppa_trace_load (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_trace_prefetch (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_trace_store (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_trace_pred_def (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_trace_hashing_jump (L_Oper * oper, Probe_Info * info);

L_Oper *hppa_trace_mem_copy (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_trace_mem_copy_back (L_Oper * oper, Probe_Info * info);
L_Oper *hppa_trace_mem_copy_check (L_Oper * oper, Probe_Info * info);


/*
 * Playdoh trace generation prototypes
 */
int L_check_playdoh_emulation (int model);
int L_check_playdoh_code_scheduled (L_Func * fn);
L_Oper *playdoh_init_tracer (Probe_Info * info);
L_Oper *playdoh_signal_end (Probe_Info * info);
L_Oper *playdoh_trace_predicates (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_write_buffer (L_Cb * cb, Probe_Info * info);
L_Oper *playdoh_func_entry (L_Func * func, Probe_Info * info);
L_Oper *playdoh_func_exit (L_Func * func, Probe_Info * info);
L_Oper *playdoh_jsr_setup (Probe_Info * info);
L_Oper *playdoh_cb_entry (L_Cb * cb, Probe_Info * info);
L_Oper *playdoh_branch_if_full (L_Cb * write_buf_cb, Probe_Info * info);
L_Oper *playdoh_dump_trace_if_full (Probe_Info * info);
L_Oper *playdoh_flush_buffer (Probe_Info * info);
L_Oper *playdoh_detect_child (Probe_Info * info);

L_Oper *playdoh_trace_jsr (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_trace_cond_branch (L_Oper * oper, Probe_Info * info);

/* Traces both normal and non_trapping loads (since probei instructions
 * may be used)
 */
L_Oper *playdoh_trace_load (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_trace_prefetch (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_trace_store (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_trace_pred_def (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_trace_hashing_jump (L_Oper * oper, Probe_Info * info);

L_Oper *playdoh_trace_mem_copy (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_trace_mem_copy_back (L_Oper * oper, Probe_Info * info);
L_Oper *playdoh_trace_mem_copy_check (L_Oper * oper, Probe_Info * info);


/*
 * X86 trace generation prototypes
 */
L_Oper *x86_init_tracer (Probe_Info * info);
L_Oper *x86_signal_end (Probe_Info * info);
L_Oper *x86_trace_predicates (L_Oper * oper, Probe_Info * info);
L_Oper *x86_write_buffer (L_Cb * cb, Probe_Info * info);
L_Oper *x86_func_entry (L_Func * func, Probe_Info * info);
L_Oper *x86_jsr_setup (Probe_Info * info);
L_Oper *x86_cb_entry (L_Cb * cb, Probe_Info * info);
L_Oper *x86_branch_if_full (L_Cb * write_buf_cb, Probe_Info * info);
L_Oper *x86_dump_trace_if_full (Probe_Info * info);
L_Oper *x86_flush_buffer (Probe_Info * info);
L_Oper *x86_detect_child (Probe_Info * info);

L_Oper *x86_trace_jsr (L_Oper * op, Probe_Info * info);
L_Oper *x86_trace_cond_branch (L_Oper * op, Probe_Info * info);
L_Oper *x86_trace_load (L_Oper * op, Probe_Info * info);
L_Oper *x86_trace_implicit_memory_op (L_Oper * op, Probe_Info * info);
L_Oper *x86_trace_non_trapping_load (L_Oper * op, Probe_Info * info);
L_Oper *x86_trace_prefetch (L_Oper * oper, Probe_Info * info);
L_Oper *x86_trace_store (L_Oper * op, Probe_Info * info);
L_Oper *x86_trace_pred_def (L_Oper * oper, Probe_Info * info);
L_Oper *x86_trace_hashing_jump (L_Oper * op, Probe_Info * info);

/*
 * SPARC trace generation prototypes
 */
L_Oper *sparc_init_tracer (Probe_Info * info);
L_Oper *sparc_signal_end (Probe_Info * info);
L_Oper *sparc_trace_predicates (L_Oper * oper, Probe_Info * info);
L_Oper *sparc_write_buffer (L_Cb * cb, Probe_Info * info);
L_Oper *sparc_func_entry (L_Func * func, Probe_Info * info);
L_Oper *sparc_func_exit (L_Func * func, Probe_Info * info);
L_Oper *sparc_jsr_setup (Probe_Info * info);
L_Oper *sparc_cb_entry (L_Cb * cb, Probe_Info * info);
L_Oper *sparc_branch_if_full (L_Cb * write_buf_cb, Probe_Info * info);
L_Oper *sparc_dump_trace_if_full (Probe_Info * info);
L_Oper *sparc_flush_buffer (Probe_Info * info);
L_Oper *sparc_detect_child (Probe_Info * info);

L_Oper *sparc_trace_jsr (L_Oper * op, Probe_Info * info);
L_Oper *sparc_trace_cond_branch (L_Oper * op, Probe_Info * info);
L_Oper *sparc_trace_load (L_Oper * op, Probe_Info * info);
L_Oper *sparc_trace_non_trapping_load (L_Oper * op, Probe_Info * info);
L_Oper *sparc_trace_prefetch (L_Oper * oper, Probe_Info * info);
L_Oper *sparc_trace_store (L_Oper * op, Probe_Info * info);
L_Oper *sparc_trace_pred_def (L_Oper * oper, Probe_Info * info);
L_Oper *sparc_trace_hashing_jump (L_Oper * op, Probe_Info * info);

#endif
