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
 *      File: l_pipe_util.c
 *      Description: Queue routines and other general purpose routines
 *      Creation Date: January, 1994
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/
/* 09/19/02 REK Updating to use functions from ltahoe_op_query.h instead
 *              of Tmdes. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_softpipe_int.h"
#include <Lcode/r_regalloc.h>

/*************************************************************************
                Global Variables
*************************************************************************/

/* alloc pools for queues and queue nodes */
L_Alloc_Pool *Queue_pool = NULL;
L_Alloc_Pool *Qnode_pool = NULL;

/*************************************************************************
                Lsoftpipe Global Parameter Declarations
*************************************************************************/

/* parameters - see l_pipe_util.h */

int Lpipe_do_induction_reversal = 0;
int Lpipe_check_loops_in_phase1 = 1;

char *Lpipe_schema_name = NULL;
int Lpipe_schema = MULTI_EPI;

int Lpipe_backward_sched = 1;

float Lpipe_budget_ratio = 5.0;
int Lpipe_min_ii = 0;		/* 0 is no minimum. */
int Lpipe_max_ii = 1000;
int Lpipe_max_stages = 1000;
int Lpipe_max_tries = 50;

int Lpipe_fixed_slots_for_branches = 0;
int Lpipe_do_only_postpass_steps = 0;

int Lpipe_compact_branch_path_opers = 0;
int Lpipe_sort_mrt_rows = 1;

int Lpipe_combine_cbs = 1;

int Lpipe_debug = 0;
int Lpipe_debug_use_cb_bounds = 0;
int Lpipe_debug_lower_cb_bound = 0;
int Lpipe_debug_upper_cb_bound = 100000000;
int Lpipe_print_statistics = 0;
int Lpipe_print_iteration_schedule = 0;
int Lpipe_print_schedules_for_debug = 0;
int Lpipe_print_mve_summary = 0;

int Lpipe_compute_loop_reg_pressure = 0;
int Lpipe_add_spill_attributes = 0;

int Lpipe_dump_dot = 0;

/*************************************************************************
                Initialization Function Definitions
*************************************************************************/

/* Read parameters from Softpipe section */
/* see l_pipe_util.h for parm descriptions */

void
L_read_parm_lpipe (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "do_induction_reversal", &Lpipe_do_induction_reversal);
  L_read_parm_b (ppi, "check_loops_in_phase1", &Lpipe_check_loops_in_phase1);

  L_read_parm_s (ppi, "code_schema", &Lpipe_schema_name);

  L_read_parm_b (ppi, "backward_scheduling", &Lpipe_backward_sched);
  L_read_parm_f (ppi, "budget_ratio", &Lpipe_budget_ratio);
  L_read_parm_i (ppi, "softpipe_min_ii", &Lpipe_min_ii);
  L_read_parm_i (ppi, "softpipe_max_ii", &Lpipe_max_ii);
  L_read_parm_i (ppi, "softpipe_max_stages", &Lpipe_max_stages);
  L_read_parm_i (ppi, "softpipe_max_tries", &Lpipe_max_tries);
  L_read_parm_b (ppi, "fixed_slots_for_branches",
		 &Lpipe_fixed_slots_for_branches);
  L_read_parm_b (ppi, "?do_only_postpass_steps",
		 &Lpipe_do_only_postpass_steps);

  L_read_parm_b (ppi, "compact_branch_path_opers",
		 &Lpipe_compact_branch_path_opers);
  L_read_parm_b (ppi, "sort_mrt_rows", &Lpipe_sort_mrt_rows);
  L_read_parm_b (ppi, "softpipe_combine_cbs", &Lpipe_combine_cbs);

  L_read_parm_i (ppi, "debug_software_pipelining", &Lpipe_debug);
  L_read_parm_b (ppi, "softpipe_debug_use_cb_bounds",
		 &Lpipe_debug_use_cb_bounds);
  L_read_parm_i (ppi, "softpipe_debug_lower_cb_bound",
		 &Lpipe_debug_lower_cb_bound);
  L_read_parm_i (ppi, "softpipe_debug_upper_cb_bound",
		 &Lpipe_debug_upper_cb_bound);
  L_read_parm_b (ppi, "print_pipelining_statistics", &Lpipe_print_statistics);
  L_read_parm_b (ppi, "print_iteration_schedule",
		 &Lpipe_print_iteration_schedule);
  L_read_parm_b (ppi, "print_schedules_for_debug",
		 &Lpipe_print_schedules_for_debug);
  L_read_parm_b (ppi, "print_mve_summary", &Lpipe_print_mve_summary);

  L_read_parm_b (ppi, "?compute_loop_reg_pressure",
		 &Lpipe_compute_loop_reg_pressure);
  L_read_parm_b (ppi, "?add_spill_attributes", &Lpipe_add_spill_attributes);

  L_read_parm_b (ppi, "?pipe_dump_dot", &Lpipe_dump_dot);

  /* Verify correctness of certain parameters. */

  if (Lpipe_budget_ratio < 1.0)
    {
      L_punt ("L_read_parm_lpipe: it is impossible to schedule the loop "
	      "with a budget_ratio less than 1.0\n");
    }

  /* Lpipe_schema options:
     REM_LOOP 0              remainder_loop
     MULTI_EPI 1             multiple_epilogues
     MULTI_EPI_ROT_REG 2     multiple_epilogues_rotating_registers
     KERNEL_ONLY 3           kernel_only
     note that KERNEL_ONLY is a form of modulo scheduling where the
     prologue and epilogue are executed using predication, and
     does NOT just generate a kernel for debugging purposes! MCM
   */

  /* Initialize the integer equivalent of the code generation schema name */
  if (!strcmp (Lpipe_schema_name, "remainder_loop"))
    {
      Lpipe_schema = REM_LOOP;
    }
  else if (!strcmp (Lpipe_schema_name, "multiple_epilogues"))
    {
      Lpipe_schema = MULTI_EPI;
    }
  else
    if (!strcmp (Lpipe_schema_name, "multiple_epilogues_rotating_registers"))
    {
      Lpipe_schema = MULTI_EPI_ROT_REG;
    }
  else if (!strcmp (Lpipe_schema_name, "kernel_only"))
    {
      Lpipe_schema = KERNEL_ONLY;
    }
  else
    {
      L_punt ("L_read_parm_lpipe: %s is an invalid code schema",
	      Lpipe_schema_name);
    }

  return;
}

/*************************************************************************
                Queue Function Definitions
*************************************************************************/

/* create and initialize a queue */
Queue *
Q_create_queue ()
{
  Queue *queue;

  queue = (Queue *) L_alloc (Queue_pool);
  queue->head = queue->tail = 0;

  return (queue);
}

/* free all queue nodes and reinitialize queue */
void
Q_reinit_queue (Queue * queue)
{
  Qnode *qnode;
  Qnode *next_qnode;

  for (qnode = queue->head; qnode != NULL; qnode = next_qnode)
    {
      next_qnode = qnode->next_qnode;
      L_free (Qnode_pool, qnode);
    }

  queue->head = queue->tail = 0;
}

/* free all queue nodes and delete queue */
void
Q_delete_queue (Queue * queue)
{
  Qnode *qnode;
  Qnode *next_qnode;

  for (qnode = queue->head; qnode != NULL; qnode = next_qnode)
    {
      next_qnode = qnode->next_qnode;
      L_free (Qnode_pool, qnode);
    }
  L_free (Queue_pool, queue);
}

/* Enqueue opers in order of increasing priority; i.e., oper with lowest
   numerical priority value is at the head of the queue */
void
Q_priority_enqueue_increasing (Queue * queue, SM_Oper * oper, int priority)
{
  Qnode *qnode;
  Qnode *qtmp;

  /* create qnode for oper */
  qnode = (Qnode *) L_alloc (Qnode_pool);
  qnode->oper = oper;
  qnode->priority = priority;

  if (Lpipe_debug >= 2)
    printf ("enqueue increasing: op %d with priority %d\n",
	    oper->lcode_op->id, priority);

  /* empty queue */
  if (queue->head == NULL)
    {
      queue->head = queue->tail = qnode;
      qnode->next_qnode = qnode->prev_qnode = 0;
      return;
    }

  /* not empty queue */
  for (qtmp = queue->head; qtmp != NULL; qtmp = qtmp->next_qnode)
    {
      /* Find oper's place in queue.  If two opers have the same priority,
         they are dequeued in the same order enqueued. */
      if (qnode->priority < qtmp->priority)
	{
	  qnode->next_qnode = qtmp;
	  qnode->prev_qnode = qtmp->prev_qnode;
	  if (qtmp->prev_qnode != NULL)
	    {
	      qtmp->prev_qnode->next_qnode = qnode;
	    }
	  else
	    {
	      queue->head = qnode;
	    }
	  qtmp->prev_qnode = qnode;
	  break;
	}
      /* oper is last in queue */
      else if (qtmp == queue->tail)
	{
	  qnode->prev_qnode = qtmp;
	  qnode->next_qnode = 0;
	  qtmp->next_qnode = qnode;
	  queue->tail = qnode;
	  break;
	}
    }
}

/* Enqueue opers in order of decreasing priority; i.e., oper with highest
   numerical priority value is at the head of the queue */
void
Q_priority_enqueue_decreasing (Queue * queue, SM_Oper * oper, int priority)
{
  Qnode *qnode;
  Qnode *qtmp;

  /* create qnode for oper */
  qnode = (Qnode *) L_alloc (Qnode_pool);
  qnode->oper = oper;
  qnode->priority = priority;

  if (Lpipe_debug >= 2)
    printf ("enqueue decreasing: op %d with priority %d\n",
	    oper->lcode_op->id, priority);

  /* empty queue */
  if (queue->head == NULL)
    {
      queue->head = queue->tail = qnode;
      qnode->next_qnode = qnode->prev_qnode = 0;
      return;
    }

  /* not empty queue */
  for (qtmp = queue->head; qtmp != NULL; qtmp = qtmp->next_qnode)
    {
      /* Find oper's place in queue.  If two opers have the same priority,
         they are dequeued in the same order enqueued. */
      if (qnode->priority > qtmp->priority)
	{
	  qnode->next_qnode = qtmp;
	  qnode->prev_qnode = qtmp->prev_qnode;
	  if (qtmp->prev_qnode != NULL)
	    {
	      qtmp->prev_qnode->next_qnode = qnode;
	    }
	  else
	    {
	      queue->head = qnode;
	    }
	  qtmp->prev_qnode = qnode;
	  break;
	}
      /* oper is last in queue */
      else if (qtmp == queue->tail)
	{
	  qnode->prev_qnode = qtmp;
	  qnode->next_qnode = 0;
	  qtmp->next_qnode = qnode;
	  queue->tail = qnode;
	  break;
	}
    }
}

/* remove and return oper at the head of the queue */
SM_Oper *
Q_dequeue (Queue * queue)
{
  SM_Oper *oper;
  Qnode *qnode;

  if (queue->head == NULL)
    return 0;

  qnode = queue->head;
  oper = qnode->oper;

  /* last oper in queue */
  if (qnode->next_qnode == NULL)
    {
      queue->head = queue->tail = 0;
    }
  /* not last oper in queue */
  else
    {
      qnode->next_qnode->prev_qnode = 0;
      queue->head = qnode->next_qnode;
    }

  if (Lpipe_debug >= 2)
    printf ("dequeue: op %d with priority %d\n",
	    oper->lcode_op->id, qnode->priority);

  L_free (Qnode_pool, qnode);

  return (oper);
}

/* return oper at tail of queue without removing it */
SM_Oper *
Q_peek_tail (Queue * queue)
{
  if (queue->tail != 0)
    return (queue->tail->oper);
  else
    return 0;
}

/* return oper at head of queue without removing it */
SM_Oper *
Q_peek_head (Queue * queue)
{
  if (queue->head != 0)
    return (queue->head->oper);
  else
    return 0;
}

/*************************************************************************
                Utility Function Definitions
*************************************************************************/

/* Overhauled by MCM for ITI. */
int
Lpipe_reordered_anti_dependent_ops (SM_Oper * oper, SM_Oper * next_oper)
{
  SM_Dep *dep;
  SM_Reg_Action *dest_operand;
  int zero_cycle_dependent = 0;
  int anti_dependent = 0;
  int i, dest_number;

  /* If there is a zero-cycle dependence between these two ops, 
     return 0, reporting that these two ops are not reordered
     anti-dependent ops.  Note that SM creates dependences for anti-
     and output dependent instructions, but sets the ignore flag
     because MVE is assumed which will correct for this situation. */

  for (dest_number = 0; dest_number < L_max_dest_operand; dest_number++)
    {
      dest_operand = oper->dest[dest_number];

      if (dest_operand == NULL)
	continue;

      for (dep = dest_operand->first_dep_out; dep != NULL;
	   dep = dep->next_dep_out)
	{
	  if (dep->to_action->sm_op == next_oper &&
	      dep->min_delay == 0 && dep->omega == 0 && !dep->ignore)
	    {
	      zero_cycle_dependent = 1;
	      break;
	    }
	}

      if (zero_cycle_dependent == 1)
	break;
    }

  if (zero_cycle_dependent)
    return 0;

  anti_dependent = 0;

  /* Check for anti-depedent ops.  This function assumes that op1 and
     op2 have been reversed in order.  Based on that assumption, look
     for a flow dependence between the instructions when using simple
     dest and src operand names.  One could also look for an anti-dep
     SM dependence arc between the instructions with the ignore flag
     (MVE assumed) set.  At the time of this writing, we weren't sure
     if the MVE ignored arcs would be actually deleted or just ignored
     via the flag, so the original register comparison is used. */

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (oper->lcode_op->dest[i] != NULL &&
	  L_is_src_operand (oper->lcode_op->dest[i], next_oper->lcode_op))
	{
	  anti_dependent = 1;
	  break;
	}
    }

  if (anti_dependent)
    return 1;
  else
    return 0;
}


/*************************************************************************
                Dependence Ignoring Utilities
*************************************************************************/

/* Self dependencies are need for rec_mii calculation but
   not for scheduling! */
void
Lpipe_ignore_self_dependences (SM_Cb * sm_cb)
{
  SM_Oper *sm_oper, *from_sm_oper;
  SM_Reg_Action *action;
  SM_Dep *dep_in;
  Softpipe_Op_Info *softpipe_info, *from_softpipe_info;

  for (sm_oper = sm_cb->first_serial_op; sm_oper != NULL;
       sm_oper = sm_oper->next_serial_op)
    {
      softpipe_info = SOFTPIPE_OP_INFO (sm_oper->lcode_op);

      for (action = sm_oper->first_op_action; action != NULL;
	   action = action->next_op_action)
	{
	  for (dep_in = action->first_dep_in; dep_in != NULL;
	       dep_in = dep_in->next_dep_in)
	    {
	      from_sm_oper = dep_in->from_action->sm_op;
	      from_softpipe_info = SOFTPIPE_OP_INFO (from_sm_oper->lcode_op);

	      if (sm_oper == from_sm_oper && !dep_in->ignore)
		SM_ignore_dep (dep_in, SM_MVE_IGNORE);

	      /* If the originating node is a start or stop node, then
	         ignore any created dependences to start or stop
	         nodes. */
	      else
		if ((L_START_NODE (sm_oper->lcode_op)
		     && L_STOP_NODE (from_sm_oper->lcode_op)) ||
		    (L_STOP_NODE (sm_oper->lcode_op)
		     && L_START_NODE (from_sm_oper->lcode_op)))
		SM_ignore_dep (dep_in, SM_MVE_IGNORE);
	    }
	}
    }
  return;
}


/* Find loop back branch.  Single basic block cbs that end with
   an unconditional jump to the fall through path 
   will be broken into two blocks during loop 
   prep: one for the loop, one for the jump after the loop back 
   branch.  Assume loop does not end with an unconditional jump back 
   to header or other operations after the loop back branch. */

L_Oper *
Lpipe_find_loop_back_br (L_Func * fn, L_Cb * header_cb)
{
  L_Oper *oper;
  L_Oper *loop_back_br = NULL;
  int found = 0;		/* flag to indicate that loop back
				   branch has been found */

  for (oper = header_cb->last_op; oper != NULL; oper = oper->prev_op)
    {
      /* loop back branch must be conditional branch */
      if (L_cond_branch (oper) && (L_find_branch_dest (oper) == header_cb))
	{
	  if (!found)
	    {
	      found = 1;
	      loop_back_br = oper;
	    }
	  else
	    {
	      L_punt ("Lpipe_find_loop_back_br: more than one branch back to "
		      "header cb: %d", header_cb->id);
	    }
	}
      else if (L_uncond_branch (oper) &&
	       (L_find_branch_dest (oper) == header_cb))
	{
	  L_punt ("Lpipe_find_loop_back_br: Loop ended with"
		  "unconditional jump to header cb: %d", header_cb->id);
	}
      else if (L_general_branch_opcode (oper))
	{
	  if (found)
	    {
	      L_punt ("Lpipe_find_loop_back_br: "
		      "loop has an early exit. cb: %d", header_cb->id);
	    }
	}
    }

  if (found)
    return (loop_back_br);
  else
    L_punt ("Lpipe_find_loop_back_br: "
	    "could not find loop back branch. cb: %d", header_cb->id);

  return (0);
}


/* For remainder loop, remove all branches in kernel, except last one */
void
Lpipe_remove_branches (L_Func * fn, int unroll)
{
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;
  L_Oper *next_op;

  for (oper = header_cb->first_op; oper != NULL; oper = next_op)
    {
      next_op = oper->next_op;
      softpipe_info = SOFTPIPE_OP_INFO (oper);
      if ((L_int_cond_branch_opcode (oper)) &&
	  (softpipe_info->kernel_copy != (unroll - 1)))
	{
	  L_free (Softpipe_Op_Info_pool, softpipe_info);
	  L_delete_oper (header_cb, oper);
	}
    }
}


void
Lpipe_cb_set_iter (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  L_Oper *oper;
  L_Attr *attr;

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      oper = sm_op->lcode_op;
      /* Set iteration attribute for copy 0 of kernel.  Copy numbers
         go from 0 to unroll-1.  Iteration numbers go from 1 to unroll. 
         The iteration numbers for the first copy need to be set
         before the kernel is unrolled.  The numbers for the other
         copies (if any) will be set during unrolling. */
      if (!(attr = L_find_attr (oper->attr, "iter")))
	{
	  attr = L_new_attr ("iter", 1);
	  oper->attr = L_concat_attr (oper->attr, attr);
	}
      L_set_int_attr_field (attr, 0, 1);
    }
}

void
Lpipe_cb_set_issue_time (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  L_Oper *oper;
  L_Attr *attr;
  Softpipe_Op_Info *softpipe_info;

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      oper = sm_op->lcode_op;

      softpipe_info = SOFTPIPE_OP_INFO (oper);

      if (!(attr = L_find_attr (oper->attr, "issue_time")))
	{
	  attr = L_new_attr ("issue_time", 1);
	  oper->attr = L_concat_attr (oper->attr, attr);
	}
      L_set_int_attr_field (attr, 0, softpipe_info->issue_time);
    }
}

/* This function consumes the L_Operands! */
L_Oper *
Lpipe_gen_uncond_pred_define (L_Operand * pred0, L_Operand * pred1,
			      L_Operand * dest, int opc)
{
  L_Oper *new_oper = L_create_new_op (Lop_CMP);
  L_set_compare (new_oper, M_native_int_register_ctype (), Lcmp_COM_EQ);
  new_oper->pred[0] = pred0;
  new_oper->pred[1] = pred1;
  new_oper->dest[0] = dest;
  new_oper->src[0] = L_new_gen_int_operand (0);
  new_oper->src[1] = L_new_gen_int_operand (0);

  if (opc == Lop_PRED_CLEAR)
    {
      new_oper->dest[0]->ptype = L_PTYPE_UNCOND_F;
    }
  else if (opc == Lop_PRED_SET)
    {
      new_oper->dest[0]->ptype = L_PTYPE_UNCOND_T;
    }
  else if (opc == Lop_PRED_COPY)
    {
      L_punt ("Lpipe_gen_uncond_pred_define: "
	      "Lop_PRED_COPY not implemented.\n");
    }
  else
    {
      L_punt ("Lpipe_gen_uncond_pred_define: " "opc not supported.\n");
    }

  return new_oper;
}

/*
 * Transform pred_clear / pred_xx o{t|f} pairs into 
 * unconditional predciate defines where possible.
 */
void
Lpipe_create_uncond_pred_defines (L_Func * fn)
{
  int i;
  L_Cb *cb;
  L_Oper *oper, *new_oper;
  L_Operand *dest;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  new_oper = NULL;

	  if (!L_initializing_predicate_define_opcode (oper))
	    continue;

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = oper->dest[i];
	      if (dest == NULL)
		continue;
	      if (dest->ptype != L_PTYPE_NULL)
		continue;

	      if (Lpipe_debug >= 1)
		fprintf (stdout, "Change r %d (op %d) to uncond predicate\n",
			 dest->value.r, oper->id);

	      new_oper =
		Lpipe_gen_uncond_pred_define (L_copy_operand (oper->pred[0]),
					      L_copy_operand (oper->pred[1]),
					      L_copy_operand (dest),
					      oper->opc);

	      if (L_pred_clear (oper))
		{
		  new_oper->dest[0]->ptype = L_PTYPE_UNCOND_F;
		}
	      else if (L_pred_set (oper))
		{
		  new_oper->dest[0]->ptype = L_PTYPE_UNCOND_T;
		}
	      else if (L_pred_copy (oper))
		{
		  L_punt ("Lpipe_create_uncond_pred_defines: "
			  "Lop_PRED_COPY not implemented: op %d\n", oper->id);
		}
	      else
		{
		  L_punt ("Lpipe_create_uncond_pred_defines: "
			  "opc not supported: op %d\n", oper->id);
		}

	      L_insert_oper_before (cb, oper, new_oper);
	    }

	  if (new_oper != NULL)
	    {
	      oper = new_oper;
	      L_delete_oper (cb, oper->next_op);
	    }
	}
    }
}


/*************************************************************************
                Start/Stop Function Definitions
*************************************************************************/

/* add start and stop pseudo-nodes for the purpose of computing MinDist,
   priority, and earliest and latest issue time */
void
Lpipe_create_start_stop_nodes (SM_Cb * sm_cb)
{
  L_Oper *start_oper, *stop_oper;
  SM_Oper *sm_oper, *sm_start_oper, *sm_stop_oper;
  SM_Reg_Action *action;
  SM_Dep *dep_in;

  /* create start node */
  start_oper = L_create_new_op (Lop_NO_OP);
  sm_start_oper = SM_insert_oper_after (sm_cb, start_oper, NULL);
  /* Make a sync source so that a sync dep can originate from this instr. */
  if (!sm_start_oper->ext_dest[SM_SYNC_ACTION_INDEX])
    {
      sm_start_oper->ext_dest[SM_SYNC_ACTION_INDEX] =
	SM_add_reg_action (sm_start_oper, MDES_SYNC_OUT, SM_SYNC_ACTION_INDEX,
			   SM_SYNC_ACTION_OPERAND);
    }
  start_oper->ext = (Softpipe_Op_Info *) Lpipe_create_op_info ();
  Lpipe_init_op_info (sm_start_oper, -1, 0);
  start_oper->flags = L_SET_BIT_FLAG (start_oper->flags, L_OPER_START_NODE);

  /* create stop node */
  stop_oper = L_create_new_op (Lop_NO_OP);
  sm_stop_oper = SM_insert_oper_after (sm_cb, stop_oper,
				       sm_cb->last_serial_op);
  stop_oper->ext = (Softpipe_Op_Info *) Lpipe_create_op_info ();
  Lpipe_init_op_info (sm_stop_oper, -1, 0);
  stop_oper->flags = L_SET_BIT_FLAG (stop_oper->flags, L_OPER_STOP_NODE);

  /* Extra dependences (sync) are automatically created between
     the start and stop opers.  They must be ignored. */
  for (action = sm_start_oper->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_in = action->first_dep_in; dep_in != NULL;
	   dep_in = dep_in->next_dep_in)
	{
	  if (dep_in->from_action->sm_op == sm_start_oper ||
	      dep_in->from_action->sm_op == sm_stop_oper)
	    SM_ignore_dep (dep_in, SM_MVE_IGNORE);
	}
    }
  for (action = sm_stop_oper->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_in = action->first_dep_in; dep_in != NULL;
	   dep_in = dep_in->next_dep_in)
	{
	  if (dep_in->from_action->sm_op == sm_start_oper ||
	      dep_in->from_action->sm_op == sm_stop_oper)
	    SM_ignore_dep (dep_in, SM_MVE_IGNORE);
	}
    }

  /* add a flow dependence from start node to all other opers and from
     all other opers to stop node */
  for (sm_oper = sm_cb->first_serial_op; sm_oper != NULL;
       sm_oper = sm_oper->next_serial_op)
    {
      if ((sm_oper == sm_start_oper) || (sm_oper == sm_stop_oper))
	continue;

      /* Make a sync source so that a sync dep can 
         originate from this instr. */
      if (sm_oper->ext_dest[SM_SYNC_ACTION_INDEX] == NULL)
	{
	  sm_oper->ext_dest[SM_SYNC_ACTION_INDEX] =
	    SM_add_reg_action (sm_oper, MDES_SYNC_OUT, SM_SYNC_ACTION_INDEX,
			       SM_SYNC_ACTION_OPERAND);
	}

      SM_add_dep (sm_start_oper->ext_dest[SM_SYNC_ACTION_INDEX],
		  sm_oper->ext_src[SM_SYNC_ACTION_INDEX],
		  SM_FIXED_DELAY | SM_HARD_DEP |
		  SM_START_STOP_DEP | SM_FLOW_DEP | SM_SYNC_DEP, 0, 0, 0,
		  SM_HARD_DEP | SM_SOFT_DEP);
      SM_add_dep (sm_oper->ext_dest[SM_SYNC_ACTION_INDEX],
		  sm_stop_oper->ext_src[SM_SYNC_ACTION_INDEX],
		  SM_FIXED_DELAY | SM_HARD_DEP |
		  SM_START_STOP_DEP | SM_FLOW_DEP | SM_SYNC_DEP, 0, 0, 0,
		  SM_HARD_DEP | SM_SOFT_DEP);
    }
}


/* delete start and stop pseudo-nodes and their associated dependences  */
void
Lpipe_delete_start_stop_nodes (SM_Cb * sm_cb)
{
  SM_Oper *sm_oper;
  L_Cb *lcode_cb;
  L_Oper *lcode_op;

  sm_oper = sm_cb->first_serial_op;

  while (sm_oper)
    {
      if (L_START_STOP_NODE (sm_oper->lcode_op))
	{
	  SM_Oper *next_serial_oper = sm_oper->next_serial_op;

	  /* 20030110 SZU
	   * Memory problems.
	   * SM_delete_oper tries to delete mdes_info in lcode_op.
	   * However lcode_op is already gone via L_delete_oper.
	   * Therefore save lcode_op and delete sm_oper first.
	   * Also need to delete softpipe_op_info from lcode_op.
	   */
#if 0
	  /* Remove the lcode op from the cb */
	  L_delete_oper (sm_oper->sm_cb->lcode_cb, sm_oper->lcode_op);

	  /* Delete the sm_op */
	  SM_delete_oper (sm_oper);
#else
	  lcode_cb = sm_oper->sm_cb->lcode_cb;
	  lcode_op = sm_oper->lcode_op;

	  /* Delete the sm_op */
	  SM_delete_oper (sm_oper);

	  Lpipe_free_oper_op_info (lcode_op);

	  /* Remove the lcode op from the cb */
	  L_delete_oper (lcode_cb, lcode_op);
#endif
	  sm_oper = next_serial_oper;
	}
      else
	sm_oper = sm_oper->next_serial_op;
    }
}


/*************************************************************************
                Debug Function Definitions
*************************************************************************/

/* print cb with isl attributes to show schedule */
void
Lpipe_print_cb_schedule (FILE * file, L_Func * fn, SM_Cb * sm_cb)
{
  SM_Oper_Qentry *q_op;
  SM_Oper *sm_op;
  L_Oper *lcode_op;
  L_Attr *attr;
  int found_isl = 0;

  if (file == NULL)
    {
      L_punt ("Lpipe_print_cb_schedule: sched_file not open - "
	      "function %s, cb %d\n", fn->name, header_cb->id);
    }

  if (sm_cb->kernel_queue->first_qentry == NULL)
    {
      L_punt ("Lpipe_print_cb_schedule: "
	      "kernel queue must be built before printing schedule; "
	      "function %s, cb %d\n", fn->name, header_cb->id);
    }


  /* Use the first kernel instruction as a test to see if the
     kernel scheduling information has been attached. */
  attr =
    L_find_attr (sm_cb->kernel_queue->first_qentry->sm_op->lcode_op->attr,
		 "isl");
  if (attr != NULL)
    found_isl = 1;

  /* Attach isl attribute only when one doesn't already exist */
  if (!found_isl)
    {
      q_op = sm_cb->kernel_queue->first_qentry;
      while (q_op != NULL)
	{
	  sm_op = q_op->sm_op;

	  SM_attach_isl (sm_op, 0);

	  q_op = q_op->next_qentry;
	}
    }

  L_print_cb (sched_file, fn, sm_cb->lcode_cb);

  /* remove isl attributes */
  if (!found_isl)
    {
      q_op = sm_cb->kernel_queue->first_qentry;
      while (q_op != NULL)
	{
	  sm_op = q_op->sm_op;
	  lcode_op = sm_op->lcode_op;

	  attr = L_find_attr (lcode_op->attr, "isl");
	  if (attr == NULL)
	    {
	      L_punt ("Lpipe_print_cb_schedule: "
		      "isl attribute should not be null,"
		      "function = %s, oper = %d\n", fn->name, lcode_op->id);
	    }
	  lcode_op->attr = L_delete_attr (lcode_op->attr, attr);

	  q_op = q_op->next_qentry;
	}
    }

  fflush (sched_file);

  return;
}

void
Lpipe_create_defines (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *rot_reg_op;
  int int_rot_reg_base = -1, int_rot_reg_num = -1;
  int flt_rot_reg_base = -1, flt_rot_reg_num = -1;
  int dbl_rot_reg_base = -1, dbl_rot_reg_num = -1;
  int pred_rot_reg_base = -1, pred_rot_reg_num = -1;
  int int_cb_rot_reg_num = -1;
  int flt_cb_rot_reg_num = -1;
  int dbl_cb_rot_reg_num = -1;
  int pred_cb_rot_reg_num = -1;
  int int_fn_rot_reg_num = -1;
  int flt_fn_rot_reg_num = -1;
  int dbl_fn_rot_reg_num = -1;
  int pred_fn_rot_reg_num = -1;
  int i;
  int chunk;
  int native_ctype = M_native_int_register_ctype ();
  L_Attr *fn_rr_attr = NULL, *cb_rr_attr = NULL;

  /* Retrieve the rotating registers. */

  fn_rr_attr = L_find_attr (L_fn->attr, "rr");
  if (fn_rr_attr == NULL)
    L_punt ("Lpipe_create_defines: "
	    "Function not marked with rotating register IDs.");

  int_rot_reg_base = fn_rr_attr->field[0]->value.i;
  int_fn_rot_reg_num = fn_rr_attr->field[1]->value.i;
  flt_rot_reg_base = fn_rr_attr->field[2]->value.i;
  flt_fn_rot_reg_num = fn_rr_attr->field[3]->value.i;
  dbl_rot_reg_base = fn_rr_attr->field[4]->value.i;
  dbl_fn_rot_reg_num = fn_rr_attr->field[5]->value.i;
  pred_rot_reg_base = fn_rr_attr->field[6]->value.i;
  pred_fn_rot_reg_num = fn_rr_attr->field[7]->value.i;

  /* Round the requested number of rotating registers 
     up to the next chunk boundary. */

  if (M_arch != M_TAHOE)
    {
      chunk = R_get_rot_reg_alloc_multiple (native_ctype);
      if (chunk < 1)
	L_punt ("Lpipe_create_defines: "
		"Invalid integer rot reg chunk %d.", chunk);
      int_fn_rot_reg_num = ((int_fn_rot_reg_num + chunk - 1) / chunk) * chunk;

      chunk = R_get_rot_reg_alloc_multiple (L_CTYPE_DOUBLE);
      if (chunk < 1)
	L_punt ("Lpipe_create_defines: Invalid double rot reg chunk %d.",
		chunk);
      dbl_fn_rot_reg_num = ((dbl_fn_rot_reg_num + chunk - 1) / chunk) * chunk;

      chunk = R_get_rot_reg_alloc_multiple (L_CTYPE_PREDICATE);
      if (chunk < 1)
	L_punt ("Lpipe_create_defines: Invalid predicate rot reg chunk %d.",
		chunk);
      pred_fn_rot_reg_num = ((pred_fn_rot_reg_num + chunk - 1) / chunk) *
	chunk;

      fn_rr_attr->field[1]->value.i = int_fn_rot_reg_num;
      fn_rr_attr->field[5]->value.i = dbl_fn_rot_reg_num;
      fn_rr_attr->field[7]->value.i = pred_fn_rot_reg_num;
    }
  else
    {
      /* Round the number of ints up to the multiple. */

      chunk = 8;
      int_fn_rot_reg_num = ((int_fn_rot_reg_num + chunk - 1) / chunk) * chunk;
      fn_rr_attr->field[1]->value.i = int_fn_rot_reg_num;

      /* The number of doubles and predicates are fixed
         and do not need to be rounded on Tahoe. */
    }

  /* Create the defines that ensure proper live range analysis of 
     the rotating regs. */

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* This is a kernel that uses rotating registers.
         Create a temporary define instruction that will read 
         and write all of the rotating registers.  This will
         ensure a live in condition from the prologue
         a live out condition from all of the exits. */

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	  L_EXTRACT_BIT_VAL (cb->flags, L_CB_ROT_REG_ALLOCATED))
	{
	  cb_rr_attr = L_find_attr (cb->attr, "rr");
	  if (cb_rr_attr == NULL)
	    L_punt ("Lpipe_create_defines: "
		    "Kernel cb not marked with rotating register IDs.");
	  int_cb_rot_reg_num = cb_rr_attr->field[0]->value.i;
	  flt_cb_rot_reg_num = cb_rr_attr->field[1]->value.i;
	  dbl_cb_rot_reg_num = cb_rr_attr->field[2]->value.i;
	  pred_cb_rot_reg_num = cb_rr_attr->field[3]->value.i;
	}
      else if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_ROT_REG_ALLOCATED) &&
	       L_find_attr (cb->attr, "prologue") != NULL)
	{
	  if (cb->next_cb == NULL)
	    L_punt ("Lpipe_create_defines: "
		    "Kernel cb not found after prologue cb (1).");

	  if (L_EXTRACT_BIT_VAL (cb->next_cb->flags, L_CB_SOFTPIPE) &&
	      L_EXTRACT_BIT_VAL (cb->next_cb->flags, L_CB_ROT_REG_ALLOCATED))
	    {
	      cb_rr_attr = L_find_attr (cb->next_cb->attr, "rr");
	      if (cb_rr_attr == NULL)
		L_punt ("Lpipe_create_defines: "
			"Kernel cb not marked with rotating register IDs.");
	      int_cb_rot_reg_num = cb_rr_attr->field[0]->value.i;
	      flt_cb_rot_reg_num = cb_rr_attr->field[1]->value.i;
	      dbl_cb_rot_reg_num = cb_rr_attr->field[2]->value.i;
	      pred_cb_rot_reg_num = cb_rr_attr->field[3]->value.i;
	    }
	  else
	    L_punt ("Lpipe_create_defines: "
		    "Kernel cb not found after prologue cb (2).");
	}
      else
	{
	  continue;
	}

      /* Determine the registers that must be preserved. */

      int_rot_reg_num = int_fn_rot_reg_num;
      /* The number of flts and dbls and preds is fixed.
         However, alloc the cb doubles first, then the 
         cb float then pad the floats to the end of the bank. */
      dbl_rot_reg_num = dbl_fn_rot_reg_num;
#if 0
      flt_rot_reg_num = flt_fn_rot_reg_num - dbl_cb_rot_reg_num;
#endif
      pred_rot_reg_num = pred_fn_rot_reg_num;

      /* Insert the defines depending on the type of cb. */

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	  L_EXTRACT_BIT_VAL (cb->flags, L_CB_ROT_REG_ALLOCATED))
	{
	  for (i = int_rot_reg_base;
	       i < int_rot_reg_base + int_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, native_ctype, L_PTYPE_NULL);
	      rot_reg_op->src[0] =
		L_new_register_operand (i, native_ctype, L_PTYPE_NULL);
	      L_insert_oper_before (cb, cb->last_op,
				    L_copy_operation (rot_reg_op));
	    }
	  for (i = flt_rot_reg_base;
	       i < flt_rot_reg_base + flt_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, L_CTYPE_FLOAT, L_PTYPE_NULL);
	      rot_reg_op->src[0] =
		L_new_register_operand (i, L_CTYPE_FLOAT, L_PTYPE_NULL);
	      L_insert_oper_before (cb, cb->last_op,
				    L_copy_operation (rot_reg_op));
	    }
	  for (i = dbl_rot_reg_base;
	       i < dbl_rot_reg_base + dbl_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	      rot_reg_op->src[0] =
		L_new_register_operand (i, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	      L_insert_oper_before (cb, cb->last_op,
				    L_copy_operation (rot_reg_op));
	    }
	  for (i = pred_rot_reg_base;
	       i < pred_rot_reg_base + pred_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, L_CTYPE_PREDICATE,
					L_PTYPE_UNCOND_T);
	      rot_reg_op->src[0] =
		L_new_register_operand (i, L_CTYPE_PREDICATE, L_PTYPE_NULL);
	      L_insert_oper_before (cb, cb->last_op,
				    L_copy_operation (rot_reg_op));
	    }
	  if (M_arch == M_TAHOE)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->last_op, rot_reg_op);
	      rot_reg_op->src[0] = M_return_epilogue_cntr_register ();
	    }
	}
      else if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_ROT_REG_ALLOCATED) &&
	       L_find_attr (cb->attr, "prologue") != NULL)
	{
	  /* This is a prologue that uses rotating registers.
	     Create a temporary define instruction that will write
	     all of the rotating registers.  This will ensure a live
	     in condition from the prologue and a live out condition from
	     all of the exits.  It will also prevent a live in
	     condition from the epilogue of a predecessor software
	     pipelined loop. */
	  for (i = int_rot_reg_base;
	       i < int_rot_reg_base + int_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, native_ctype, L_PTYPE_NULL);
	    }
	  for (i = flt_rot_reg_base;
	       i < flt_rot_reg_base + flt_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, L_CTYPE_FLOAT, L_PTYPE_NULL);
	    }
	  for (i = dbl_rot_reg_base;
	       i < dbl_rot_reg_base + dbl_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, L_CTYPE_DOUBLE, L_PTYPE_NULL);
	    }
	  for (i = pred_rot_reg_base;
	       i < pred_rot_reg_base + pred_rot_reg_num; i++)
	    {
	      rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
	      L_change_opcode (rot_reg_op, Lop_DEFINE);
	      L_insert_oper_before (cb, cb->first_op, rot_reg_op);
	      rot_reg_op->dest[0] =
		L_new_register_operand (i, L_CTYPE_PREDICATE,
					L_PTYPE_UNCOND_T);
	    }
	}
    }
}


void
Lpipe_delete_defines (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op, *delete_op;

  /* Scan for cbs that are software pipelines and utilize
     rotating registers. (L_CB_ROT_REG_ALLOCATED and L_CB_SOFTPIPE) */

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_ROT_REG_ALLOCATED) == 1 &&
	  (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) == 1 ||
	   L_find_attr (cb->attr, "prologue") != NULL))
	{
	  op = cb->first_op;

	  while (op != NULL)
	    {
	      delete_op = op;
	      op = op->next_op;

	      /* 20031006 SZU
	       * Need to preserve template ops
	       */
	      if (delete_op->opc == Lop_DEFINE && 
		  !M_is_template_op (delete_op))
		L_delete_oper (cb, delete_op);
	    }
	}
    }

  return;
}

void
Lpipe_print_cyclic_stats (FILE * gen_statfile, L_Cb * header_cb,
			  Softpipe_MinII * MinII, int ii, int tries,
			  int schedule_length, int loop_dep_height,
			  int stage_count, int unroll, int theta,
			  int num_oper, int branch_count, L_Inner_Loop * loop)
{
  L_Attr *attr;
  int unroll_AMP;
  float mii_eff, ii_eff, ii_ratio;

  attr = L_new_attr ("II", 1);
  L_set_int_attr_field (attr, 0, ii);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("tries", 1);
  L_set_int_attr_field (attr, 0, tries);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("softpipe_schedule_length", 1);
  L_set_int_attr_field (attr, 0, schedule_length);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("softpipe_loop_dep_height", 1);
  L_set_int_attr_field (attr, 0, loop_dep_height);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("stage_count", 1);
  L_set_int_attr_field (attr, 0, stage_count);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("useful_stage_count", 1);
  L_set_float_attr_field (attr, 0, ((float) schedule_length / (float) ii));
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("kmin", 1);
  L_set_int_attr_field (attr, 0, unroll);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("theta", 1);
  L_set_int_attr_field (attr, 0, theta);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("opers", 1);
  L_set_int_attr_field (attr, 0, num_oper);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);
  attr = L_new_attr ("branches", 1);
  L_set_int_attr_field (attr, 0, branch_count);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);

  unroll_AMP = 1;
  attr = L_find_attr (header_cb->attr, "unroll_AMP");
  if (attr != NULL)
    {
      unroll_AMP = attr->field[0]->value.i;
      mii_eff = ((float) MinII->MinII) / ((float) unroll_AMP);
      ii_eff = ((float) ii) / ((float) unroll_AMP);
    }
  else
    {
      mii_eff = (float) MinII->MinII;
      ii_eff = (float) ii;
    }

  ii_ratio = ii_eff / mii_eff;
  fprintf (gen_statfile, "ResMinII: %d\n", MinII->res_MinII);
  fprintf (gen_statfile, "RecMinII: %d\n", MinII->rec_MinII);
  fprintf (gen_statfile, "II: %d\n", ii);
  fprintf (gen_statfile, "Prior_unroll: %d\n", unroll_AMP);
  fprintf (gen_statfile, "MinII_eff: %f\n", mii_eff);
  fprintf (gen_statfile, "II_eff: %f\n", ii_eff);
  fprintf (gen_statfile, "II_MinII_ratio: %f\n", ii_ratio);
  fprintf (gen_statfile, "Tries: %d\n", tries);
  fprintf (gen_statfile, "Schedule_length: %d\n", schedule_length);
  fprintf (gen_statfile, "Dependence_height: %d\n", loop_dep_height);
  fprintf (gen_statfile, "Stage_count: %d\n", stage_count);
  fprintf (gen_statfile, "Kmin: %d\n", unroll);
  fprintf (gen_statfile, "Theta: %d\n", theta);
  fprintf (gen_statfile, "Number_of_opers: %d\n", num_oper);
  fprintf (gen_statfile, "Number_of_branches: %d\n", branch_count);
  fprintf (gen_statfile, "Loop_freq: %f\n", header_cb->weight);
  fprintf (gen_statfile, "Entry_freq: %f\n", loop->num_invocation);
  fprintf (gen_statfile, "Budget_ratio: %f\n", Lpipe_budget_ratio);
  return;
}


L_Oper *
Lpipe_gen_mov_consuming_operands (L_Operand * dest, L_Operand * src)
{
  L_Oper *new_oper;
  int opc = 0;

  if (!dest || !src)
    L_punt ("Lpipe_gen_mov: NULL operand");

  if (L_is_ctype_integer (src))
    opc = Lop_MOV;
  else if (L_is_ctype_flt (src))
    opc = Lop_MOV_F;
  else if (L_is_ctype_dbl (src))
    opc = Lop_MOV_F2;
  else if (L_is_ctype_predicate (src))
    opc = Lop_CMP;
  else
    L_punt ("Lpipe_gen_mov: Unhandled type");

  new_oper = L_create_new_op (opc);
  new_oper->dest[0] = dest;
  if (opc != Lop_CMP)
    {
      new_oper->src[0] = src;
    }
  else
    {
      new_oper->pred[0] = src;
      L_set_compare (new_oper, L_CTYPE_INT, Lcmp_COM_EQ);
      L_assign_ptype_uncond_true (new_oper->dest[0]);
      new_oper->src[0] = L_new_gen_int_operand (0);
      new_oper->src[1] = L_new_gen_int_operand (0);
    }

  return new_oper;
}


void
Lpipe_move_int_parm_regs (L_Func * fn)
{
  L_Attr *tr_attr;
  int fields, j, pipelined = 0;
  L_Operand *parm_operand, *parm_class_operand, *parm_vreg;
  L_Oper *alloc_op, *insert_before_op, *scan_op, *move_op;
  L_Cb *scan_cb;
  int native_ctype = M_native_int_register_ctype ();

  tr_attr = L_find_attr (fn->attr, "tr");

  scan_cb = fn->first_cb;
  while (scan_cb && pipelined == 0)
    {
      pipelined |= L_EXTRACT_BIT_VAL (scan_cb->flags, L_CB_SOFTPIPE);
      scan_cb = scan_cb->next_cb;
    }

  /* If there are input parms and there are pipelined loops, then
     move the input parms into virtual registers. */
  if (tr_attr != NULL && pipelined == 1)
    {
      /* Find the prologue or alloc instruction. */
      alloc_op = fn->first_cb->first_op;
      while (alloc_op != NULL && alloc_op->opc != Lop_ALLOC)
	{
	  alloc_op = alloc_op->next_op;
	}
      if (!alloc_op)
	L_punt ("Lpipe_move_int_parm_regs: function prologue op not found.");

      insert_before_op = alloc_op->next_op;

      /* Loop through the incoming register parameters. */
      for (fields = 0; fields < tr_attr->max_field; fields++)
	{
	  parm_operand = tr_attr->field[fields];
	  if (!L_is_macro (parm_operand))
	    L_punt ("Lpipe_move_int_parm_regs: "
		    "parm in tr field is not a macro.");

	  if (!L_is_ctype_integer (parm_operand))
	    continue;

	  /* Generate the move into a vreg. */
	  parm_vreg = L_new_register_operand (++(fn->max_reg_id),
					      native_ctype, L_PTYPE_NULL);
	  parm_class_operand = L_new_macro_operand (parm_operand->value.mac,
						    native_ctype,
						    L_PTYPE_NULL);

	  move_op =
	    Lpipe_gen_mov_consuming_operands (L_copy_operand (parm_vreg),
					      L_copy_operand
					      (parm_class_operand));
	  L_insert_oper_before (fn->first_cb, insert_before_op, move_op);
	  L_annotate_oper (fn, fn->first_cb, move_op);
	  L_delete_oper (fn->first_cb, move_op);

	  /* Scan through all the opers in the function looking for
	     matches. For the first cb, start after the prologue/alloc op. */
	  scan_cb = fn->first_cb;

	  while (scan_cb != NULL)
	    {
	      if (scan_cb == fn->first_cb)
		scan_op = insert_before_op;
	      else
		scan_op = scan_cb->first_op;

	      while (scan_op != NULL)
		{
		  for (j = 0; j < L_max_dest_operand; j++)
		    {
		      if (L_is_macro (scan_op->dest[j]) &&
			  (scan_op->dest[j]->value.mac ==
			   parm_operand->value.mac))
			{
			  L_delete_operand (scan_op->dest[j]);
			  scan_op->dest[j] = L_copy_operand (parm_vreg);
			}
		    }
		  for (j = 0; j < L_max_src_operand; j++)
		    {
		      if (L_is_macro (scan_op->src[j]) &&
			  (scan_op->src[j]->value.mac ==
			   parm_operand->value.mac))
			{
			  L_delete_operand (scan_op->src[j]);
			  scan_op->src[j] = L_copy_operand (parm_vreg);
			}
		    }
#if 0
		  /* Can't have an int parm in a qualifying pred field. */
		  for (j = 0; j < L_max_pred_operand; j++)
		    {
		      if (L_is_macro (scan_op->pred[j]) &&
			  (scan_op->pred[j]->value.mac ==
			   parm_operand->value.mac))
			{
			  L_delete_operand (scan_op->pred[j]);
			  scan_op->pred[j] = L_copy_operand (parm_vreg);
			}
		    }
#endif
		  scan_op = scan_op->next_op;
		}

	      scan_cb = scan_cb->next_cb;
	    }

	  L_delete_operand (parm_vreg);
	  parm_vreg = NULL;
	  L_delete_operand (parm_class_operand);
	  parm_class_operand = NULL;
	}
    }

  return;
}

int
Lpipe_can_create_fallthru_cb (L_Cb * cb)
{
  L_Cb *fallthru_cb;
#if 0
  L_Flow *fl;

  fl = L_find_last_flow (cb->dest_flow);

  if (fl->dst_cb == cb)
    return 0;
#endif

  if (L_uncond_branch (cb->last_op))
    {
      fallthru_cb = cb->last_op->src[0]->value.cb;
      if (fallthru_cb == cb)
	return 0;
    }

  return 1;
}


void
Lpipe_reduce_defines (L_Func * fn)
{
  L_Cb *cb = fn->first_cb;
  L_Oper *oper, *next_oper;
  L_Attr *attr;
  Set uses;
  int num_uses;

  while (cb)
    {
      attr = L_find_attr (cb->attr, "prologue");
      if (attr)
	{
	  oper = cb->first_op;
	  while (oper)
	    {
	      next_oper = oper->next_op;

	      if (oper->opc == Lop_DEFINE && L_is_reg (oper->dest[0]))
		{
		  uses = L_get_oper_ROUT_using_opers (oper, oper->dest[0]);
		  num_uses = Set_size (uses);

		  if (!num_uses)
		    {
#if 0
		      printf ("Lpipe_reduce_defines: "
			      "Eliminating DEFINE %d of reg %d.\n",
			      oper->id, oper->dest[0]->value.r);
#endif
		      L_delete_oper (cb, oper);
		    }
                  Set_dispose (uses);
		}

	      oper = next_oper;
	    }
	}

      cb = cb->next_cb;
    }

  return;
}

void
Lpipe_insert_non_rr_defines (L_Func * fn)
{
  L_Cb *cb = fn->first_cb;
  L_Oper *oper, *next_oper, *rot_reg_op;
  L_Attr *attr;
  Set writes = NULL;
  int i;
  Set rr = R_build_rotating_reg_set (fn);

  while (cb)
    {
      attr = L_find_attr (cb->attr, "kernel");
      if (attr)
	{
	  oper = cb->first_op;
	  while (oper)
	    {
	      next_oper = oper->next_op;

	      if (oper->opc != Lop_DEFINE)
		{
		  for (i = 0; i < L_max_src_operand; i++)
		    {
		      if (L_is_reg (oper->src[i])
			  && !Set_in (rr, oper->src[i]->value.r)
			  && !Set_in (writes, oper->src[i]->value.r))
			{
			  rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
			  L_change_opcode (rot_reg_op, Lop_DEFINE);
			  L_insert_oper_after (cb->prev_cb,
					       cb->prev_cb->last_op,
					       rot_reg_op);
			  rot_reg_op->src[0] = L_copy_operand (oper->src[i]);
			  rot_reg_op->dest[0] = L_copy_operand (oper->src[i]);

#if 0
			  rot_reg_op = L_new_oper (L_fn->max_oper_id + 1);
			  L_change_opcode (rot_reg_op, Lop_DEFINE);
			  L_insert_oper_before (cb->next_cb,
						cb->next_cb->first_op,
						rot_reg_op);
			  rot_reg_op->src[0] = L_copy_operand (oper->src[i]);
			  rot_reg_op->dest[0] = L_copy_operand (oper->src[i]);
#endif

			  writes = Set_add (writes, oper->src[i]->value.r);
			}
		    }
		  for (i = 0; i < L_max_dest_operand; i++)
		    {
		      if (L_is_reg (oper->src[i])
			  && !Set_in (rr, oper->src[i]->value.r))
			{
			  writes = Set_add (writes, oper->src[i]->value.r);
			}
		    }
		}

	      oper = next_oper;
	    }
	}

      writes = Set_dispose (writes);

      cb = cb->next_cb;
    }
  return;
}
