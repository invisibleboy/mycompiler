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
 *      File: l_pipe_util.h
 *      Description: Queue structures and external utility function
 *                   declarations
 *      Creation Date: January, 1994
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

#ifndef L_PIPE_UTIL_H
#define L_PIPE_UTIL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/sm.h>

/*************************************************************************
                General Macros
*************************************************************************/

/* code generation schemas */
#define REM_LOOP 0
#define MULTI_EPI 1
#define MULTI_EPI_ROT_REG 2
#define KERNEL_ONLY 3

#define IS_MDES_FLAG_CBR(a) ( op_flag_set(a->proc_opc, OP_FLAG_CBR) || \
			     (op_flag_set(a->proc_opc, OP_FLAG_JMP) && \
                             IS_PREDICATED(a)) )
#define IS_MDES_FLAG_BR(a) ( op_flag_set(a->proc_opc, OP_FLAG_CBR) || \
			     op_flag_set(a->proc_opc, OP_FLAG_JMP) )

/*************************************************************************
                Data Structures
*************************************************************************/

/* Queue data structures */

typedef struct Qnode
{
  SM_Oper *oper;
  int priority;
  struct Qnode *next_qnode;
  struct Qnode *prev_qnode;
}
Qnode;

typedef struct Queue
{
  Qnode *head;
  Qnode *tail;
}
Queue;

/*************************************************************************
                Lsoftpipe Global Parameter Declarations
*************************************************************************/

/* parms that affect phase 1 loop preparation only */
extern int Lpipe_do_induction_reversal;	/* move induction operations
					   from the end of the loop
					   body to the beginning */
extern int Lpipe_check_loops_in_phase1;	/* check to see if loops marked
					   for pipelining are still OK
					   for pipelining at beginning
					   of phase 1 */

/* parms that affect phase 1 and phase 2 of code generation */
extern char *Lpipe_schema_name;	/* code generation schema name */
extern int Lpipe_schema;	/* integer code for schema */


/* parms that affect phase 2 only */
extern int Lpipe_backward_sched;	/* schedule from last operation
					   to first operation - default 
					   is first to last */
extern float Lpipe_budget_ratio;	/* ratio of the number of 
					   scheduling steps performed
					   before giving up to the
					   number of operations in the
					   loop body */
extern int Lpipe_min_ii;	/* If nonzero, limit ii
				   to the specified value.
				   (debug flag) */
extern int Lpipe_max_ii;	/* Prevent pipelining when II is too large. */
extern int Lpipe_max_stages;	/* Increase II until loop is under this
				   many stages. */
extern int Lpipe_max_tries;	/* Try this many different IIs before
				   giving up. */

extern int Lpipe_fixed_slots_for_branches;	/* processor has fixed
						   slots for branches, so 
						   don't change slots when 
						   sorting */
extern int Lpipe_do_only_postpass_steps;	/* assume modulo scheduling
						   was done in an earlier Lcode
						   pass, so only mark loops 
						   with spills */

/* parms that affect phase 2 optimization only */
extern int Lpipe_compact_branch_path_opers;	/* compact opers on
						   which the loop back
						   branch depends and
						   shift them upward
						   in schedule to
						   reduce speculation.
						   Only works for
						   counted loops. */
extern int Lpipe_sort_mrt_rows;	/* sort opers within each row
				   of MRT to reduce register
				   pressure and speculation */

extern int Lpipe_combine_cbs;	/* after softpipe, merge created cbs that 
				   fall through into one another. */

/* parms that affect debugging only */
extern int Lpipe_debug;		/* print debug information */
 /* If debug_use_cb_bounds is set to yes, the pipeliner will only
    pipeline those cbs with ids between the upper and lower bound.
    I.e., set both upper and lower bound to 10 to pipeline only cb
    10. */
extern int Lpipe_debug_use_cb_bounds;
extern int Lpipe_debug_lower_cb_bound;
extern int Lpipe_debug_upper_cb_bound;
extern int Lpipe_print_statistics;	/* Attach attributes to header
					   cb containing stats about each 
					   loop such as II, unroll, etc.
					   Print statistics on pipelined
					   loops in file softpipe.stats */
extern int Lpipe_print_iteration_schedule;	/* for each loop,
						   print the schedule
						   for a single
						   iteration to file */
extern int Lpipe_print_schedules_for_debug;	/* for each loop,
						   print the schedule
						   for the header cb
						   to file at various
						   points in the
						   scheduling process */
extern int Lpipe_print_mve_summary;	/* for each loop, print the
					   length of each lifetime and
					   the number of names
					   required for MVE to stdout */

extern int Lpipe_print_acyclic_stats;	/* print statistics for acyclicly
					   scheduled loops that are
					   potentially pipelineable */
extern int Lpipe_compute_loop_reg_pressure;	/* compute register
						   pressure for each
						   loop and print to
						   file */
extern int Lpipe_add_spill_attributes;	/* add attribute indicating
					   the number of spill code
					   opers in the loop for each
					   register type */

extern int Lpipe_dump_dot;

/*************************************************************************
                Lsoftpipe Global Variable Declarations
*************************************************************************/

/* alloc pools for queues and queue nodes */
extern L_Alloc_Pool *Queue_pool;
extern L_Alloc_Pool *Qnode_pool;

/*************************************************************************
                Lsoftpipe Parameter Function Declarations
*************************************************************************/

extern void L_read_parm_lpipe (Parm_Parse_Info *);

/*************************************************************************
                Lsoftpipe Queue Function Declarations
*************************************************************************/

extern Queue *Q_create_queue ();
extern void Q_reinit_queue (Queue *);
extern void Q_delete_queue (Queue *);
extern void Q_priority_enqueue_increasing (Queue *, SM_Oper *, int);
extern void Q_priority_enqueue_decreasing (Queue *, SM_Oper *, int);
extern SM_Oper *Q_dequeue (Queue *);
extern SM_Oper *Q_peek_tail (Queue *);
extern SM_Oper *Q_peek_head (Queue *);

/*************************************************************************
                Lsoftpipe Service Function Declarations
*************************************************************************/

extern int Lpipe_reordered_anti_dependent_ops (SM_Oper *, SM_Oper *);
extern void Lpipe_ignore_self_dependences (SM_Cb *);
extern void Lpipe_ignore_branch_flow_dependences (SM_Cb *);
extern L_Oper *Lpipe_find_loop_back_br (L_Func *, L_Cb *);
extern void Lpipe_remove_branches (L_Func *, int);
extern void Lpipe_cb_set_issue_time (SM_Cb *);
extern void Lpipe_cb_set_iter (SM_Cb *);
extern L_Oper *Lpipe_gen_uncond_pred_define (L_Operand *, L_Operand *,
					     L_Operand *, int);
extern void Lpipe_create_uncond_pred_defines (L_Func *);
extern void Lpipe_move_int_parm_regs (L_Func *);
extern void Lpipe_create_defines (L_Func *);
extern void Lpipe_delete_defines (L_Func *);
extern void Lpipe_reduce_defines (L_Func *);
extern L_Oper *Lpipe_gen_mov_consuming_operands (L_Operand *, L_Operand *);
extern int Lpipe_can_create_fallthru_cb (L_Cb *);


/*************************************************************************
                Lsoftpipe Start/Stop Function Declarations
*************************************************************************/

extern void Lpipe_create_start_stop_nodes (SM_Cb *);
extern void Lpipe_delete_start_stop_nodes (SM_Cb *);

/*************************************************************************
                Lsoftpipe Debug Function Declarations
*************************************************************************/

extern void Lpipe_print_cb_schedule (FILE *, L_Func *, SM_Cb *);

extern void Lpipe_print_cyclic_stats (FILE * gen_statfile, L_Cb * header_cb,
				      Softpipe_MinII * MinII, int ii,
				      int tries, int schedule_length,
				      int loop_dep_height, int stage_count,
				      int unroll, int theta, int num_oper,
				      int branch_count, L_Inner_Loop * loop);

#endif
