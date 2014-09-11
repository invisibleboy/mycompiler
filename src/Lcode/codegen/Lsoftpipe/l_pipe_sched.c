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
 *      File: l_pipe_sched.c
 *      Description: Scheduler for single candidate II
 *      Creation Date: January, 1994
 *      Author: Daniel Lavery
 *
 *  Copyright (c) 1994-1997 Daniel Lavery, Wen-mei Hwu, and 
 *                     The Board of Trustees of the University of Illinois.
 *                     All rights reserved.
 *      The University of Illinois Software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include "l_softpipe_int.h"
#include "l_pipe_sched.h"
#include "l_mve.h"

/*****************************************************************************/

static Queue *Lpipe_ready_queue = NULL;

#define LARGE_NUMBER 1000	/* An iteration should never be stretched
				   over more than 1000 stages */

/* Sort opers within each row of mrt to mimimize register pressure and
   speculation.  Use bubble sort because it is conceptually simple to
   interchange opers and swap slot numbers.  The number of opers in a
   row of the MRT is small. 
   Overhauled by MCM for ITI. */
static void
Lpipe_sort_MRT_rows (SM_Cb * sm_cb)
{
  int row;			/* Current row of MRT for sorting */
  SM_Oper_Qentry *row_start, *row_end;	/* Beginning and end of row */
  SM_Oper_Qentry *next_row_end;	/* Node being checked for inclusion in 
				   current row */
  SM_Oper_Qentry *last_unsorted_oper;	/* All opers after this in queue
					   are sorted */
  SM_Oper_Qentry *last_index;	/* Last oper which was interchanged */
  int pass_done;		/* Flag indicating that
				   last_unsorted_oper
				   has been reached */
  SM_Oper_Qentry *q_opA, *q_opB;
  SM_Oper *temp_sm_opA;
  L_Oper *lcode_opA, *lcode_opB;
  Softpipe_Op_Info *softpipe_info_opA, *softpipe_info_opB;
  int A_sched_slot, A_sched_cycle;
  int B_sched_slot, B_sched_cycle;
  int ii = sm_cb->II;

  /* sort each row of mrt */
  for (row_start = sm_cb->kernel_queue->first_qentry; row_start;
       row_start = row_end->next_qentry)
    {
      /* what row are we in? */
      softpipe_info_opA = SOFTPIPE_OP_INFO (row_start->sm_op->lcode_op);
      row = (row_start->sm_op->sched_cycle) % ii;

      /* find end of row */

      for (row_end = row_start, next_row_end = row_end->next_qentry;
	   next_row_end && (next_row_end->sm_op->sched_cycle % ii == row);
	   row_end = next_row_end, next_row_end = row_end->next_qentry);

      /* bubble sort */

      for (last_unsorted_oper = row_end; last_unsorted_oper;
	   last_unsorted_oper = last_index)
	{
	  last_index = NULL;
	  pass_done = 0;
	  q_opA = row_start;
	  while (!pass_done && row_start != last_unsorted_oper)
	    {
	      q_opB = q_opA->next_qentry;
	      if (q_opB == last_unsorted_oper)
		pass_done = 1;
	      lcode_opA = q_opA->sm_op->lcode_op;
	      lcode_opB = q_opB->sm_op->lcode_op;
	      softpipe_info_opA = SOFTPIPE_OP_INFO (lcode_opA);
	      softpipe_info_opB = SOFTPIPE_OP_INFO (lcode_opB);

	      if (Lpipe_debug >= 2)
		printf ("Lpipe_sort_MRT_rows: comparing opers: %d %d\n",
			lcode_opA->id, lcode_opB->id);

	      /* loop back branch must stay in its slot */
	      if (softpipe_info_opB->loop_back_br)
		{
		  q_opA = q_opA->next_qentry;
		  continue;
		}

	      /* handle processors which require branches 
	         to be in last slots */
	      if (Lpipe_fixed_slots_for_branches &&
		  (IS_MDES_FLAG_BR (lcode_opA) ||
		   IS_MDES_FLAG_BR (lcode_opB)))
		{
		  q_opA = q_opA->next_qentry;
		  continue;
		}
	      /* want latest issue times in the earliest slots */
	      if (q_opA->sm_op->sched_cycle > q_opB->sm_op->sched_cycle)
		{
		  q_opA = q_opA->next_qentry;
		  continue;
		}
	      /* wasted speculation - oper moved above branch, but issued in 
	         same cycle */
	      if (q_opA->sm_op->sched_cycle == q_opB->sm_op->sched_cycle &&
		  !(IS_MDES_FLAG_BR (lcode_opB) &&
		    softpipe_info_opA->home_block >
		    softpipe_info_opB->home_block)
		  && !Lpipe_reordered_anti_dependent_ops (q_opA->sm_op,
							  q_opB->sm_op))
		{
		  q_opA = q_opA->next_qentry;
		  continue;
		}

	      if (Lpipe_debug >= 2)
		printf ("Lpipe_sort_MRT_rows: swapping opers %d and %d.\n",
			lcode_opA->id, lcode_opB->id);

	      A_sched_cycle = q_opA->sm_op->sched_cycle;
	      A_sched_slot = q_opA->sm_op->sched_slot;
	      B_sched_cycle = q_opB->sm_op->sched_cycle;
	      B_sched_slot = q_opB->sm_op->sched_slot;

	      SM_unschedule_oper (q_opA->sm_op, NULL);
	      SM_unschedule_oper (q_opB->sm_op, NULL);

	      if (!SM_schedule_oper (q_opA->sm_op, A_sched_cycle, 
				     B_sched_slot, B_sched_slot, 0))
		L_punt ("Lpipe_sort_MRT_rows: Can't reschedule opA!");
	      if (!SM_schedule_oper (q_opB->sm_op, B_sched_cycle, 
				     A_sched_slot, A_sched_slot, 0))
		L_punt ("Lpipe_sort_MRT_rows: Can't reschedule opB!");

	      /* Swap qentries in SM_Oper_Queue */
	      temp_sm_opA = q_opA->sm_op;
	      SM_dequeue_oper (q_opA);
	      SM_enqueue_oper_after (sm_cb->kernel_queue, temp_sm_opA, q_opB);

	      /* opA was the row start, then they both point to the
	         same invalid memory location! */

	      /* keep track of start of row */
	      if (q_opA == row_start)
		row_start = q_opB;

	      q_opA = q_opB->next_qentry;

	      /* keep track of end of row */
	      if (q_opB == row_end)
		row_end = q_opA;

	      /* keep track of last unsorted oper */
	      last_index = q_opB;
	    }
	}
    }
}


/* Mark the chain of opers on which the loop back branch depends.  For 
   counted loops without early exits, these
   opers may need to be speculatively executed to keep the recurrence
   associated with this dependence chain from limiting the MinII. 
   Do not mark the start and stop nodes. */

void
Lpipe_mark_branch_path_opers (SM_Cb * sm_cb, SM_Oper * sm_loop_back_br)
{
  SM_Oper *sink_sm_op, *src_sm_op;
  Softpipe_Op_Info *swp_info;
  SM_Reg_Action *action;
  SM_Dep *dep_in;

  swp_info = SOFTPIPE_OP_INFO (sm_loop_back_br->lcode_op);
  swp_info->branch_path_node = 1;

  /* Determine the chain of opers on which the loop back branch depends */
  for (sink_sm_op = sm_cb->last_serial_op; sink_sm_op;
       sink_sm_op = sink_sm_op->prev_serial_op)
    {
      /* If oper is in the chain, then mark the instructions that this
         instruction is flow dependent on.  Do not follow cross-iteration
         dependences. */
      if (!(swp_info = SOFTPIPE_OP_INFO (sink_sm_op->lcode_op)) ||
	  !swp_info->branch_path_node)
	continue;

      for (action = sink_sm_op->first_op_action; action;
	   action = action->next_op_action)
	{
	  for (dep_in = action->first_dep_in; dep_in;
	       dep_in = dep_in->next_dep_in)
	    {
	      if (dep_in->ignore || 
		  (dep_in->omega != 0) ||
		  (((dep_in->flags) & (SM_REG_DEP | SM_FLOW_DEP)) !=
		   (SM_REG_DEP | SM_FLOW_DEP)))
		continue;

	      src_sm_op = dep_in->from_action->sm_op;

	      if (L_START_STOP_NODE (src_sm_op->lcode_op))
		continue;

	      swp_info = SOFTPIPE_OP_INFO (src_sm_op->lcode_op);
	      swp_info->branch_path_node = 1;
	    }
	}
    }
  return;
}


/* Opers in the dependence chain on which the loop back
   branch depends may have been scheduled later than they need
   to be.  Push this dependence chain to the top to avoid unnecessary
   speculation and shorten the schedule length. */

static void
Lpipe_do_compact_branch_path_opers (SM_Cb * sm_cb, SM_Oper * sm_loop_back_br)
{
  SM_Oper *sm_op, *src_sm_op;
  SM_Reg_Action *action;
  SM_Dep *dep_in;
  Softpipe_Op_Info *sink_softpipe_info;	/* for oper at sink of dependence */
  Softpipe_Op_Info *softpipe_info;
  int minslack;			/* min amount of slack that oper 
				   has to move up */
  int tempslack;		/* slack with respect to current dependence */
  int issue_time, issue_slot;
  int earliest_issue_time;	/* earliest possible issue time for 
				   oper at sink of dependence */
  int earliest_non_spec_issue_time;	/* issue time of the earliest oper
					   which is not on the dependence chain
					   leading to loop back branch */
  int earliest_non_spec_stage;	/* stage which contains the earliest oper
				   which is not on the dependence chain
				   leading to loop back branch */
  int stage_diff;		/* number of stages between the loop
				   back branch and the earliest oper
				   which is not on the dependence
				   chain leading to loop back branch */

  earliest_non_spec_issue_time = SM_MAX_CYCLE;

  /* compact the dependence chain leading to the loop back branch */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      sink_softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);

      /* dependence chain is compacted starting from start node */
      if (L_START_STOP_NODE (sm_op->lcode_op))
	continue;

      /* If oper is not in dependence chain leading to loop back branch,
         it does not have to be speculatively executed.  Find the earliest
         issue time among all such opers.  Find the latest issue time
         among all opers excluding the stop oper */
      if (!sink_softpipe_info->branch_path_node)
	{
	  if (sm_op->sched_cycle < earliest_non_spec_issue_time)
	    {
	      earliest_non_spec_issue_time = sm_op->sched_cycle;
	    }
	  continue;
	}

      minslack = IMPOSSIBLE_SLACK;

      /* Find out how much earlier oper can be moved. */
      for (action = sm_op->first_op_action; action != NULL;
	   action = action->next_op_action)
	{
	  for (dep_in = action->first_dep_in; dep_in != NULL;
	       dep_in = dep_in->next_dep_in)
	    {

	      if (dep_in->ignore)
		continue;

	      src_sm_op = dep_in->from_action->sm_op;

	      if (src_sm_op == sm_op ||
		  L_START_STOP_NODE (src_sm_op->lcode_op))
		continue;

	      tempslack = sm_op->sched_cycle -
		src_sm_op->sched_cycle - dep_in->min_delay +
		dep_in->omega * sm_cb->II;
	      if (tempslack < minslack)
		minslack = tempslack;
	    }
	}

      /* move oper earlier by a multiple of II cycles that is less than or
         equal to the slack */
      issue_time = sm_op->sched_cycle;
      earliest_issue_time = issue_time - minslack;
      while ((issue_time - sm_cb->II) >= earliest_issue_time)
	issue_time = issue_time - sm_cb->II;

      sink_softpipe_info->intra_iter_issue_time = issue_time;

      issue_slot = sm_op->sched_slot;

      if (Lpipe_debug >= 2)
	printf ("Lpipe_do_compact_branch_path_opers: "
		"(1) move op %d from time %d to %d\n",
		sm_op->lcode_op->id, sm_op->sched_cycle, issue_time);

      SM_unschedule_oper (sm_op, NULL);
      if (!SM_schedule_oper (sm_op, issue_time, issue_slot, issue_slot, 0))
	L_punt ("Lpipe_do_compact_branch_path_opers: "
		"(1) Can't reschedule sm_op!");
    }

  /* After the dependence chain has been compacted, push the whole chain up
     so that non-speculative operations are issued in the same stage as the
     loop back branch or later.  */

  earliest_non_spec_stage = earliest_non_spec_issue_time / sm_cb->II;
  stage_diff =
    earliest_non_spec_stage - sm_loop_back_br->sched_cycle / sm_cb->II;
  if (stage_diff < 0)
    {
      for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
	   sm_op = sm_op->next_serial_op)
	{
	  softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);
	  if (softpipe_info->branch_path_node
	      && !L_START_STOP_NODE (sm_op->lcode_op))
	    {
	      issue_time = sm_op->sched_cycle + stage_diff * sm_cb->II;
	      softpipe_info->intra_iter_issue_time = issue_time;

	      if (Lpipe_debug >= 2)
		printf ("Lpipe_do_compact_branch_path_opers: "
			"(2) move op %d from time %d to %d\n",
			sm_op->lcode_op->id, sm_op->sched_cycle, issue_time);

	      issue_slot = softpipe_info->issue_slot;

	      SM_unschedule_oper (sm_op, NULL);
	      if (!SM_schedule_oper (sm_op, issue_time, issue_slot,
				     issue_slot, 0))
		L_punt ("Lpipe_do_compact_branch_path_opers: "
			"(2) Can't reschedule sm_op!");
	    }
	}
    }
}


/* Move operations with no incoming register flow dependences as late as
   possible in the schedule and operations with no outgoing dependences as
   early as possible in the schedule */
static void
Lpipe_optimize_lifetimes (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  Softpipe_Op_Info *src_softpipe_info;	/* for oper at source of dependence */
  Softpipe_Op_Info *sink_softpipe_info;	/* for oper at sink of dependence */
  int minslack;			/* minimum amount of slack that
				   oper has to move */
  int tempslack;		/* slack with respect to current dependence */
  int issue_time, issue_slot;
  int latest_issue_time;	/* latest possible issue time for
				   oper at source of dependence */
  int earliest_issue_time;	/* earliest possible issue time for
				   oper at sink of dependence */
  int num_outgoing_reg_deps;
  int num_incoming_reg_deps;
  SM_Reg_Action *action;
  SM_Oper *src_sm_op, *sink_sm_op;
  SM_Dep *dep_in, *dep_out;

  if (Lpipe_backward_sched)
    {
      for (sm_op = sm_cb->last_serial_op; sm_op;
	   sm_op = sm_op->prev_serial_op)
	{
	  sink_softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);

	  if (L_START_STOP_NODE (sm_op->lcode_op) ||
	      sink_softpipe_info->loop_back_br)
	    continue;

	  num_outgoing_reg_deps = 0;

	  /* find opers with no outgoing register flow dependences */
	  for (action = sm_op->first_op_action; action != NULL;
	       action = action->next_op_action)
	    {
	      for (dep_out = action->first_dep_out; dep_out != NULL;
		   dep_out = dep_out->next_dep_out)
		{
		  if (dep_out->ignore)
		    continue;

		  if ((dep_out->flags & SM_FLOW_DEP) &&
		      (dep_out->flags & SM_REG_DEP))
		    num_outgoing_reg_deps++;
		}
	    }

	  if (num_outgoing_reg_deps != 0)
	    continue;

	  minslack = IMPOSSIBLE_SLACK;

	  /* find out how much earlier oper can be moved */

	  for (action = sm_op->first_op_action; action != NULL;
	       action = action->next_op_action)
	    {
	      for (dep_in = action->first_dep_in; dep_in != NULL;
		   dep_in = dep_in->next_dep_in)
		{
		  if (dep_in->ignore)
		    continue;

		  src_sm_op = dep_in->from_action->sm_op;

		  src_softpipe_info = SOFTPIPE_OP_INFO (src_sm_op->lcode_op);

		  if (L_START_STOP_NODE (src_sm_op->lcode_op))
		    continue;

		  tempslack = sm_op->sched_cycle -
		    src_sm_op->sched_cycle - dep_in->min_delay +
		    (dep_in->omega * sm_cb->II);
		  if (dep_in->min_delay == 0)
		    {
		      if (sm_op->sched_slot < src_sm_op->sched_slot)
			{
			  /* cannot move sink oper to same cycle as
			     source oper */
			  tempslack--;
			}
		    }
		  if (tempslack < minslack)
		    minslack = tempslack;
		}
	    }

	  if (minslack == IMPOSSIBLE_SLACK)
	    L_punt ("Lpipe_optimize_lifetimes:"
		    "minslack cannot be equal to IMPOSSIBLE_SLACK (op %d).",
		    sm_op->lcode_op->id);

	  /* move oper earlier by a multiple of II cycles that is less
	     than or equal to the slack */
	  issue_time = sm_op->sched_cycle;
	  earliest_issue_time = issue_time - minslack;
	  while ((issue_time - sm_cb->II) >= earliest_issue_time)
	    issue_time = issue_time - sm_cb->II;

	  if (issue_time == sm_op->sched_cycle)
	    continue;

	  sink_softpipe_info->intra_iter_issue_time = issue_time;

	  issue_slot = sm_op->sched_slot;

	  if (Lpipe_debug >= 2)
	    printf ("Lpipe_optimize_lifetimes(B): "
		    "move op %d from time %d to %d\n",
		    sm_op->lcode_op->id, sm_op->sched_cycle, issue_time);

	  SM_unschedule_oper (sm_op, NULL);
	  if (!SM_schedule_oper (sm_op, issue_time, issue_slot, issue_slot, 0))
	    L_punt ("Lpipe_optimize_lifetimes(B): Can't reschedule sm_op!");
	}
    }
  else				/* forward scheduling */
    {
      for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
	   sm_op = sm_op->next_serial_op)
	{
	  src_softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);

	  if (L_START_STOP_NODE (sm_op->lcode_op) ||
	      src_softpipe_info->loop_back_br)
	    continue;

	  num_incoming_reg_deps = 0;

	  /* find opers with no outgoing register flow dependences */
	  for (action = sm_op->first_op_action; action != NULL;
	       action = action->next_op_action)
	    {
	      for (dep_in = action->first_dep_in; dep_in != NULL;
		   dep_in = dep_in->next_dep_in)
		{
		  if (dep_in->ignore)
		    continue;

		  if ((dep_in->flags & SM_FLOW_DEP) &&
		      (dep_in->flags & SM_REG_DEP))
		    num_incoming_reg_deps++;
		}
	    }

	  if (num_incoming_reg_deps != 0)
	    continue;

	  minslack = IMPOSSIBLE_SLACK;

	  /* find out how much later oper can be moved */
	  for (action = sm_op->first_op_action; action != NULL;
	       action = action->next_op_action)
	    {
	      for (dep_out = action->first_dep_out; dep_out != NULL;
		   dep_out = dep_out->next_dep_out)
		{
		  if (dep_out->ignore)
		    continue;

		  sink_sm_op = dep_out->to_action->sm_op;

		  sink_softpipe_info =
		    SOFTPIPE_OP_INFO (sink_sm_op->lcode_op);

		  if (L_START_STOP_NODE (sink_sm_op->lcode_op))
		    continue;

		  tempslack = sink_sm_op->sched_cycle -
		    sm_op->sched_cycle - dep_out->min_delay +
		    (dep_out->omega * sm_cb->II);

		  if (dep_out->min_delay == 0)
		    {
		      if (sink_sm_op->sched_slot < sm_op->sched_slot)
			{
			  /* cannot move source oper to same cycle as
			     sink oper */
			  tempslack--;
			}
		    }
		  if (tempslack < minslack)
		    minslack = tempslack;
		}
	    }

	  /* move oper later by a multiple of II cycles that is less
	     than or equal to the slack */
	  issue_time = sm_op->sched_cycle;
	  latest_issue_time = issue_time + minslack;
	  while ((issue_time + sm_cb->II) <= latest_issue_time)
	    issue_time = issue_time + sm_cb->II;

	  if (issue_time == sm_op->sched_cycle)
	    continue;

	  src_softpipe_info->intra_iter_issue_time = issue_time;

	  issue_slot = sm_op->sched_slot;

	  if (Lpipe_debug >= 2)
	    printf ("Lpipe_optimize_lifetimes(F): "
		    "move op %d from time %d to %d\n",
		    sm_op->lcode_op->id, sm_op->sched_cycle, issue_time);

	  SM_unschedule_oper (sm_op, NULL);
	  if (!SM_schedule_oper (sm_op, issue_time, issue_slot, issue_slot, 0))
	    L_punt ("Lpipe_optimize_lifetimes(F): Can't reschedule sm_op!");
	}
    }

  return;
}


/*--------------------------------------------------------------------------
                    Priority calculation routines
  --------------------------------------------------------------------------*/

/* Compute the earliest and latest start time and slack for each oper using
   MinDist */
static void
Lpipe_compute_slack_time_using_MinDist (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  L_Oper *l_oper;
  Softpipe_Op_Info *softpipe_info;
  int op_num = 0;
  int max = 0;			/* max dep height in loop body */

  /* compute earliest issue time and find max height of dependence graph from
     top down */

  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      l_oper = sm_op->lcode_op;
      if (!(softpipe_info = SOFTPIPE_OP_INFO (l_oper)))
	L_punt ("Lpipe_compute_slack_time_using_MinDist: oper has no "
		"Softpipe_Op_Info structure\nCb = %d, Op = %d\n",
		sm_cb->lcode_cb->id, l_oper->id);

      if (L_START_NODE (l_oper))
	{
	  softpipe_info->estart = 0;
	}
      else
	{
	  softpipe_info->estart = Lpipe_get_MinDist (0, op_num);
	  if (softpipe_info->estart > max)
	    max = softpipe_info->estart;
	}
      op_num++;
    }

  loop_dep_height = max + 1;	/* dependence height of original loop body */

  /* compute latest start time for each oper from the bottom up */
  op_num = sm_cb->op_count - 1;
  for (sm_op = sm_cb->last_serial_op; sm_op; sm_op = sm_op->prev_serial_op)
    {
      l_oper = sm_op->lcode_op;
      softpipe_info = SOFTPIPE_OP_INFO (l_oper);

      if (L_STOP_NODE (l_oper))
	softpipe_info->lstart = max;
      else
	softpipe_info->lstart =
	  max - Lpipe_get_MinDist (op_num, sm_cb->op_count - 1);
      op_num--;
    }

  /* compute slack for each oper */
  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      L_Attr *slack_attr;
      softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);
      softpipe_info->slack = softpipe_info->lstart - softpipe_info->estart;

      if (!(slack_attr = L_find_attr (sm_op->lcode_op->attr, "slack")))
	{
	  slack_attr = L_new_attr ("slack", 1);
	  sm_op->lcode_op->attr = L_concat_attr (sm_op->lcode_op->attr,
						 slack_attr);
	}
      L_set_int_attr_field (slack_attr, 0, softpipe_info->slack);
    }
}


/* Compute height-based priorities for all the operations before 
   scheduling.  */
static void
Lpipe_compute_static_priorities (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  Softpipe_Op_Info *softpipe_info;

  /* For forward scheduling, the priority is equal to the
     latest start time for the operation.  A lower numerical value means
     a higher priority.  For backward scheduling, the priority is equal to
     the earliest start time.  A higher numerical value means a higher
     priority. */

  if (Lpipe_backward_sched)
    {
      for (sm_op = sm_cb->first_serial_op; sm_op; 
	   sm_op = sm_op->next_serial_op)
	{
	  if (!(softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op)))
	    continue;

	  softpipe_info->priority = 3 * softpipe_info->estart;

	  if (!softpipe_info->loop_back_br)
	    softpipe_info->priority += 128 / (1 + softpipe_info->slack);
	  else
	    softpipe_info->priority += 128;

	  /* Give higher priority for estart ties to the branches
	     and stores. */

	  if (L_is_control_oper (sm_op->lcode_op) ||
	      L_general_store_opcode (sm_op->lcode_op))
	    softpipe_info->priority += 2;

	  /* Give higher priority for estart ties to the loads. */
	  if (L_general_load_opcode (sm_op->lcode_op))
	    softpipe_info->priority += 1;
	}
    }
  else
    {				/* forward scheduling */
      for (sm_op = sm_cb->first_serial_op; sm_op;
	   sm_op = sm_op->next_serial_op)
	{
	  if ((softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op)))
	    softpipe_info->priority = softpipe_info->lstart;
	}
    }

#if 0
  /* For giving higher priority to operations in earlier iterations for 
     modulo scheduling of unrolled loops. */
  {
    L_Attr *attr;
    int iter_num;
    int priority_adjust;

    if ((attr = L_find_attr (oper->attr, "iter")))
      {
	iter_num = attr->field[0]->value.i - 1;
	priority_adjust = (int) (((float) iter_num) * ii_eff);
	softpipe_info->priority = softpipe_info->lstart + priority_adjust;
      }
    else
      {
	softpipe_info->priority = softpipe_info->lstart;
      }
  }
#endif

  return;
}


/*----------------------------------------------------------------------
               Scheduling routines
  ----------------------------------------------------------------------*/

/* Compute the ready times for each of the operands and an overall ready
   time for the operation based on the scheduled predecessors/successors. */

static void
Lpipe_compute_ready_time (SM_Cb * sm_cb, SM_Oper * sm_op, int init_ready_time)
{
  Softpipe_Op_Info *src_softpipe_info;	/* info for oper at source of
					   the dependence */
  Softpipe_Op_Info *sink_softpipe_info;	/* info for oper at sink of
					   the dependence */
  SM_Reg_Action *action;
  int temp_ready_time;		/* ready time associated with the
				   current dependence */

  /* For forward scheduling, examine predecessors.  For reverse
     scheduling, examine the successors. */

  if (Lpipe_backward_sched)
    {
      SM_Oper *sink_sm_op;
      SM_Dep *dep_out;

      src_softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);

      src_softpipe_info->ready_time = init_ready_time;

      for (action = sm_op->first_op_action; action;
	   action = action->next_op_action)
	{
	  for (dep_out = action->first_dep_out; dep_out;
	       dep_out = dep_out->next_dep_out)
	    {
	      if (dep_out->ignore)
		continue;

	      sink_sm_op = dep_out->to_action->sm_op;

	      sink_softpipe_info = SOFTPIPE_OP_INFO (sink_sm_op->lcode_op);

	      /* Do nothing if successor is not scheduled.  Also neatly
	         handles dependences from an operation to itself */
	      if (!sink_softpipe_info->scheduled ||
		  L_START_STOP_NODE (sink_sm_op->lcode_op))
		continue;

	      temp_ready_time = sink_sm_op->sched_cycle
		- dep_out->min_delay + dep_out->omega * sm_cb->II;
	      /* update ready time for source oper */

	      if (temp_ready_time < src_softpipe_info->ready_time)
		src_softpipe_info->ready_time = temp_ready_time;
	    }
	}
    }
  else				/* forward scheduling */
    {
      SM_Oper *src_sm_op;
      SM_Dep *dep_in;

      sink_softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);

      sink_softpipe_info->ready_time = init_ready_time;

      for (action = sm_op->first_op_action; action != NULL;
	   action = action->next_op_action)
	{
	  for (dep_in = action->first_dep_in; dep_in != NULL;
	       dep_in = dep_in->next_dep_in)
	    {
	      if (dep_in->ignore)
		continue;

	      src_sm_op = dep_in->from_action->sm_op;

	      src_softpipe_info = SOFTPIPE_OP_INFO (src_sm_op->lcode_op);

	      /* Do nothing if predecessor is not scheduled.  Also neatly
	         handles dependences from an operation to itself */
	      if (!src_softpipe_info->scheduled ||
		  L_START_STOP_NODE (src_sm_op->lcode_op))
		continue;

	      temp_ready_time = src_sm_op->sched_cycle
		+ dep_in->min_delay - dep_in->omega * sm_cb->II;

	      /* update ready time for sink oper */

	      if (temp_ready_time > sink_softpipe_info->ready_time)
		sink_softpipe_info->ready_time = temp_ready_time;
	    }
	}
    }

  return;
}


/* Unschedule predecessors/successors if there is a dependence conflict 
   and put them back in the queue to be scheduled again.
   Deps to unschedule in reverse mode are selected only if they are 
   predecessors in the dependence graph and if the unschedulable oper
   can be placed in a cycle that will break the dependence.  Therefore,
   we want to know the earliest cycle that the unschedulable oper 
   could be scheduled. For forward scheduling, use the lasted desired
   issue time for the smallest actual latency because the unschedulable
   oper is the source of the dependences. */

static void
Lpipe_eject_opers_if_dependence_conflict (SM_Oper * sm_op, int II,
					  int desired_cycle)
{
  L_Oper *l_oper = sm_op->lcode_op;
  SM_Reg_Action *action;

  Softpipe_Op_Info *src_softpipe_info;	/* info for oper at source of
					   the dependence */
  Softpipe_Op_Info *sink_softpipe_info;	/* info for oper at sink of
					   the dependence */
  int actual_delay;		/* difference in issue time between source and
				   sink opers */
  int required_delay;		/* required separation in cycles
				   between source and sink oper to statisfy
				   dependence */
  int eject_oper;		/* flag to indicate that the source or
				   sink oper has a dependence conflict
				   with the oper that was just scheduled */

  /* For forward scheduling, examine successors.  For reverse
     scheduling, examine the predecessors. */

  if (Lpipe_backward_sched)
    {
      SM_Oper *src_sm_op;
      SM_Dep *dep_in;

      sink_softpipe_info = SOFTPIPE_OP_INFO (l_oper);

      for (action = sm_op->first_op_action; action;
	   action = action->next_op_action)
	{
	  for (dep_in = action->first_dep_in; dep_in;
	       dep_in = dep_in->next_dep_in)
	    {
	      if (dep_in->ignore)
		continue;

	      src_sm_op = dep_in->from_action->sm_op;

	      src_softpipe_info = SOFTPIPE_OP_INFO (src_sm_op->lcode_op);
	      eject_oper = 0;

	      /* do nothing if predecessor is not scheduled */
	      if (!src_softpipe_info->scheduled ||
		  L_START_STOP_NODE (src_sm_op->lcode_op))
		continue;

	      actual_delay = desired_cycle - src_sm_op->sched_cycle;

	      /* Compute delay required between the operations.  Treat start 
	         and stop dummy nodes differently because they have no
	         Mdes information.  Also properly handles dependences from
	         an operation to itself. */
	      if (!L_START_STOP_NODE (src_sm_op->lcode_op) &&
		  !L_START_STOP_NODE (l_oper))
		{
		  /* find dest/src latency for operand associated with the 
		     dependence */

		  required_delay = dep_in->min_delay;

		  if (required_delay < 0)
		    required_delay = 0;

		  if (actual_delay < required_delay - dep_in->omega * II)
		    {
		      eject_oper = 1;
		    }
		  else if ((required_delay == 0) &&
			   (actual_delay ==
			    required_delay - dep_in->omega * II))
		    {
		      /* if 0 cycle dependence and dependent opers
		         will execute in the same cycle, need to check
		         slot */
		      if (sm_op->sched_slot < src_sm_op->sched_slot)
			eject_oper = 1;
		    }
		}
	      else
		{
		  if (actual_delay < dep_in->min_delay - dep_in->omega * II)
		    eject_oper = 1;
		}

	      if (eject_oper &&
		  !SOFTPIPE_OP_INFO (src_sm_op->lcode_op)->loop_back_br)
		{
		  /* unschedule operation, reinitialize Softpipe_Op_Info
		     fields, and throw it back in queue */
		  if (!L_START_STOP_NODE (src_sm_op->lcode_op))
		    {
		      /* Start and stop nodes never had resources allocated
		         to begin with. */

		      if (Lpipe_debug >= 1)
			printf ("(XD) op %i ejected from cycle %i slot %i\n",
				src_sm_op->lcode_op->id,
				src_sm_op->sched_cycle,
				src_sm_op->sched_slot);

		      SM_unschedule_oper (src_sm_op, NULL);
		    }

		  src_softpipe_info->scheduled = 0;
		  src_softpipe_info->issue_time = -1;
		  src_softpipe_info->intra_iter_issue_time = -1;
		  Q_priority_enqueue_decreasing (Lpipe_ready_queue, src_sm_op,
						 src_softpipe_info->priority);
		}
	    }
	}
    }
  else
    {				/* forward scheduling */
      SM_Oper *sink_sm_op;
      SM_Dep *dep_out;

      src_softpipe_info = SOFTPIPE_OP_INFO (l_oper);

      for (action = sm_op->first_op_action; action;
	   action = action->next_op_action)
	{
	  for (dep_out = action->first_dep_out; dep_out;
	       dep_out = dep_out->next_dep_out)
	    {
	      if (dep_out->ignore)
		continue;

	      sink_sm_op = dep_out->to_action->sm_op;

	      sink_softpipe_info = SOFTPIPE_OP_INFO (sink_sm_op->lcode_op);
	      eject_oper = 0;

	      /* do nothing if successor is not scheduled */
	      if (!sink_softpipe_info->scheduled ||
		  L_START_STOP_NODE (sink_sm_op->lcode_op))
		continue;

	      actual_delay = sink_sm_op->sched_cycle - desired_cycle;

	      /* Compute delay required between the operations.  Treat start 
	         and stop dummy nodes differently because they have no
	         Mdes information.  Also properly handles dependences from
	         an operation to itself. */
	      if (!L_START_STOP_NODE (l_oper) &&
		  !L_START_STOP_NODE (sink_sm_op->lcode_op))
		{
		  /* find dest/src latency for operand associated with the 
		     dependence */

		  required_delay = dep_out->min_delay;

		  if (required_delay < 0)
		    required_delay = 0;

		  if (actual_delay < required_delay - dep_out->omega * II)
		    {
		      eject_oper = 1;
		    }
		  else if ((required_delay == 0) &&
			   (actual_delay ==
			    required_delay - dep_out->omega * II))
		    {
		      /* if 0 cycle dependence and dependent opers
		         will execute in the same cycle, need to check
		         slot */
		      if (sink_sm_op->sched_slot < sm_op->sched_slot)
			eject_oper = 1;
		    }
		}
	      else
		{
		  if (actual_delay < dep_out->min_delay - dep_out->omega * II)
		    eject_oper = 1;
		}

	      if (eject_oper &&
		  !SOFTPIPE_OP_INFO (sink_sm_op->lcode_op)->loop_back_br)
		{

		  /* unschedule operation, reinitialize Softpipe_Op_Info
		     fields, and throw it back in queue */
		  if (!L_START_STOP_NODE (sink_sm_op->lcode_op))
		    {
		      /* Start and stop nodes never had resources allocated
		         to begin with. */

		      if (Lpipe_debug >= 1)
			printf ("(XD) op %i ejected from cycle %i slot %i\n",
				sink_sm_op->lcode_op->id,
				sink_sm_op->sched_cycle,
				sink_sm_op->sched_slot);

		      SM_unschedule_oper (sink_sm_op, NULL);
		    }

		  sink_softpipe_info->scheduled = 0;
		  sink_softpipe_info->issue_time = -1;
		  sink_softpipe_info->intra_iter_issue_time = -1;
		  Q_priority_enqueue_increasing (Lpipe_ready_queue,
						 sink_sm_op,
						 sink_softpipe_info->
						 priority);
		}
	    }
	}
    }
  return;
}


static void
Lpipe_eject_oper_for_resource_conflict (SM_Oper * sm_op)
{
  Softpipe_Op_Info *softpipe_info;
  L_Oper *l_oper = sm_op->lcode_op;

  softpipe_info = SOFTPIPE_OP_INFO (l_oper);

  if (softpipe_info->loop_back_br)
    return;

  if (Lpipe_debug >= 1)
    printf ("(XR) op %i ejected from cycle %i slot %i\n",
	    sm_op->lcode_op->id, sm_op->sched_cycle, sm_op->sched_slot);

  SM_unschedule_oper (sm_op, NULL);

  softpipe_info->scheduled = 0;
  softpipe_info->issue_time = -1;
  softpipe_info->intra_iter_issue_time = -1;

  if (Lpipe_backward_sched)
    Q_priority_enqueue_decreasing (Lpipe_ready_queue, sm_op,
				   softpipe_info->priority);
  else
    Q_priority_enqueue_increasing (Lpipe_ready_queue, sm_op,
				   softpipe_info->priority);

  return;
}


static void
Lpipe_eject_earlier_branches_and_last_slot (SM_Cb * sm_cb,
					    int row, int last_slot)
{
  SM_Oper *sm_op;
  SM_Oper *low_priority_oper = NULL;
  L_Oper *l_oper;
  Softpipe_Op_Info *softpipe_info;
  int opers_in_row = 0;

  int min_priority = 10000;	/* higher numerical value is higher priority */
  int min_slot = last_slot + 1;

  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      l_oper = sm_op->lcode_op;

      softpipe_info = SOFTPIPE_OP_INFO (l_oper);

      if (!softpipe_info->scheduled || L_START_STOP_NODE (l_oper))
	continue;

      if ((sm_op->sched_cycle % sm_cb->II) != row)
	continue;

      if (sm_op->sched_slot == last_slot || IS_MDES_FLAG_BR (l_oper))
	{
	  Lpipe_eject_oper_for_resource_conflict (sm_op);
	}
      else
	{
	  opers_in_row++;
	  if (softpipe_info->priority < min_priority ||
	      (softpipe_info->priority == min_priority &&
	       sm_op->sched_slot < min_slot))
	    {
	      low_priority_oper = sm_op;
	      min_slot = sm_op->sched_slot;
	      min_priority = softpipe_info->priority;
	    }
	}
    }

  if (opers_in_row == Lpipe_total_issue_slots)
    Lpipe_eject_oper_for_resource_conflict (low_priority_oper);
  return;
}


static void
Lpipe_eject_opers_if_resource_conflict (SM_Cb * sm_cb, int cycle)
{
  SM_Oper *sm_op;
  L_Oper *l_oper;
  Softpipe_Op_Info *softpipe_info;

  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      l_oper = sm_op->lcode_op;

      softpipe_info = SOFTPIPE_OP_INFO (l_oper);

      if (!softpipe_info->scheduled || L_START_STOP_NODE (l_oper))
	continue;

      if ((sm_op->sched_cycle % sm_cb->II) == (cycle % sm_cb->II))
	Lpipe_eject_oper_for_resource_conflict (sm_op);
    }
  return;
}


void
Lpipe_sm_terminate (SM_Cb *sm_cb)
{
  SM_unschedule_cb (sm_cb);
  return;
}


/* Schedule loop body at the candidate II.  Return stage count if successful
   and 0 if not */

int
Lpipe_sm_schedule (SM_Cb *sm_cb, int II, SM_Oper *loop_back_br, int budget)
{
  SM_Oper *sm_op;
  L_Oper *l_oper;
  Softpipe_Op_Info *softpipe_info;
  int init_ready_time;		/* initialization value for oper ready time */
  int test_slot;		/* iterate through valid issue slots
				   for operation */
  int cycle = -1;		/* candidate cycle for scheduling oper */
  int ready_time, earliest_cycle, latest_cycle, success = 1;
  int sched_length;

  /* Create scheduling queues */
  Lpipe_ready_queue = Q_create_queue ();

  SM_set_cb_II (sm_cb, II);

  /* initialize ready time for opers */
  if (Lpipe_backward_sched)
    {
      /* The stop node is scheduled in the last row of the MRT.
         LARGE_NUMBER is chosen so that LARGE_NUMBER * ii is much
         longer than the maximum expected schedule length.  This keeps
         the candidate cycle for scheduling from becoming negative as
         the opers are scheduled from the bottom up.  For the
         candidate cycle to become negative, more than LARGE_NUMBER
         iterations have to be overlapped.  The stop node is scheduled
         at init_ready_time.  For each oper, the ready time is
         initialized to init_ready_time and decreased as the opers
         that depend on it are scheduled. */
      init_ready_time = LARGE_NUMBER * II - 1;
    }
  else
    {				/* forward scheduling */
      /* The start node is scheduled in the first row of the MRT.
         LARGE_NUMBER is chosen so that LARGE_NUMBER * ii is much
         longer than the the maximum expected schedule length.  This
         keeps the candidate cycle for scheduling from becoming
         negative when there are cross-iteration dependences.  For
         each oper, the ready time is initialized to init_ready_time
         and decreased as the opers that it depends on are
         scheduled. */
      init_ready_time = LARGE_NUMBER * II;
    }

  /* (re)initialize Softpipe_Op_Info for each oper */
  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    Lpipe_init_op_info (sm_op, init_ready_time, 0);

  Lpipe_compute_slack_time_using_MinDist (sm_cb);

  Lpipe_compute_static_priorities (sm_cb);

  /* Initialize ready/priority list */
  if (Lpipe_backward_sched)
    {
      for (sm_op = sm_cb->last_serial_op; sm_op;
	   sm_op = sm_op->prev_serial_op)
	{
	  l_oper = sm_op->lcode_op;
	  if (!(softpipe_info = SOFTPIPE_OP_INFO (l_oper)))
	    L_punt ("Lpipe_sm_schedule(B): "
		    "oper %d has no Softpipe_Op_Info structure",
		    l_oper->id);

	  /* higher numerical value is higher priority */
	  Q_priority_enqueue_decreasing (Lpipe_ready_queue, sm_op,
					 softpipe_info->priority);
	}
    }
  else
    {				/* forward scheduling */
      for (sm_op = sm_cb->first_serial_op; sm_op;
	   sm_op = sm_op->next_serial_op)
	{
	  l_oper = sm_op->lcode_op;
	  if (!(softpipe_info = SOFTPIPE_OP_INFO (l_oper)))
	    L_punt ("Lpipe_sm_schedule(F): "
		    "oper %d has no Softpipe_Op_Info structure",
		    l_oper->id);

	  /* lower numerical value is higher priority */
	  Q_priority_enqueue_increasing (Lpipe_ready_queue, sm_op,
					 softpipe_info->priority);
	}
    }
                                                                               
  /* schedule all operations */
  while ((sm_op = Q_dequeue (Lpipe_ready_queue)) && (budget > 0) && success)
    {
      Lpipe_compute_ready_time (sm_cb, sm_op, init_ready_time);

      softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);
      ready_time = softpipe_info->ready_time;

      /* Fake things for start or stop node.  Do not want to allocate
         resources for them. */
      if (L_START_STOP_NODE (sm_op->lcode_op))
	{
	  softpipe_info->scheduled = 1;
	  softpipe_info->intra_iter_issue_time = ready_time;

	  /* set issue slot such that it does not restrict the
	     dependent ops */

	  softpipe_info->issue_slot = L_STOP_NODE (sm_op->lcode_op) ?
	    Lpipe_total_issue_slots : -1;
	}
      else if (sm_op == loop_back_br)
	{
	  if (Lpipe_backward_sched)
	    {
	      int br_slot = Lpipe_total_issue_slots - 1;
#if 0
	      /* MCM Always put the loop back in the last cycle for now. */

	      /* Try ii + 1 cycles.  Because of 0-cycle dependences, all the
	         slots in the first cycle may not have been checked. */
	      earliest_cycle = ready_time - ii;

	      if (sm_op->slot_upper_bound > (Lpipe_total_issue_slots - 1))
		br_slot = Lpipe_total_issue_slots - 1;
	      else
		br_slot = sm_op->slot_upper_bound;

	      /* compute real ready time and earliest slot for branch */
	      if (br_slot != Lpipe_total_issue_slots - 1)
		{
		  cycle = ready_time - 1;
		  br_slot = Lpipe_total_issue_slots - 1;
		}
	      else
		{
		  cycle = ready_time;
		}
#endif

	      cycle = ready_time;
	      br_slot = Lpipe_total_issue_slots - 1;

	      if (cycle != init_ready_time)
		{
		  Lpipe_eject_opers_if_dependence_conflict (sm_op, II, cycle);
		  cycle = init_ready_time;
		}

	      /* remove any branches in earlier slots and any oper in
	         last slot */
	      Lpipe_eject_earlier_branches_and_last_slot (sm_cb, cycle % II,
							  br_slot);
	      /* schedule loop back branch in last slot */
	      if (SM_schedule_oper (sm_op, cycle, br_slot, br_slot, 0))
		{
		  if (Lpipe_debug >= 1)
		    printf ("(LB) op %i placed in cycle %i slot %i\n",
			    sm_op->lcode_op->id, cycle, sm_op->sched_slot);
		}
	      else
		{
		  L_warn ("Lpipe_sm_schedule: "
			  "Incrementing II (from %d) - "
			  "Unable to schedule loop back branch "
			  "(function %s, cb %d, oper %d)\n",
			  II, sm_cb->lcode_fn->name, header_cb->id,
			  loop_back_br->lcode_op->id);

		  if (Lpipe_debug >= 2)
		    SM_print_map (stdout, sm_cb->sm_mdes, sm_cb);

		  SM_unschedule_cb (sm_cb);
		  return 0;
		}
	    }
	  else
	    {
	      L_punt ("Lpipe_sm_schedule: "
		      "Forward scheduling not yet completely implemented.");
	    }
	}
      else			/* Not the loop back branch */
	{
	  if (Lpipe_backward_sched)
	    {
	      /* Try ii + 1 cycles.  Because of 0-cycle dependences, all the
	         slots in the first cycle may not have been checked. */
	      earliest_cycle = ready_time - II;

	      /* try II candidate cycles */
	      for (cycle = ready_time; cycle >= earliest_cycle; cycle--)
		{
		  /* When scheduled, sched_slot will be changed from 
		     SM_MAX_SLOT (65535) to real slot */
		  /* 20030731 SZU
		   * Help reduce scheduling time.
		   * New MCKINLEY, or IPF template bundling algorithm (SZU)
		   * automatically tries all slots.
		   */
		  if (SM_do_template_bundling)
		    {
		      if (SM_schedule_oper (sm_op, cycle, 0, 
					    (Lpipe_total_issue_slots - 1), 0))
			{
			  if (Lpipe_debug >= 1)
			    printf ("(1B) op %i placed in cycle %i slot %i\n",
				    sm_op->lcode_op->id,
				    cycle, sm_op->sched_slot);
			  break;
			}
		    }
		  else
		    {
		      int earliest_slot, latest_slot;

		      /* If ready cycle, start slot search at last
			 available slot based on dependences.  If
			 other cycles, successor dependences are met
			 as of the ready cycle, so try all.  Also
			 ASSUME that the last slot is currently
			 reserved for a potential branch. */

		      if (cycle == ready_time)
			{
			  if (sm_op->slot_upper_bound >
			      (Lpipe_total_issue_slots - 2))
			    latest_slot = Lpipe_total_issue_slots - 2;
			  else
			    latest_slot = sm_op->slot_upper_bound;
			}
		      else
			{
			  latest_slot = Lpipe_total_issue_slots - 1;
			}

		      earliest_slot = 0;

		      for (test_slot = latest_slot; test_slot >= earliest_slot;
			   test_slot--)
			{
			  if (SM_schedule_oper
			      (sm_op, cycle, test_slot, test_slot, 0))
			    {
			      if (Lpipe_debug >= 1)
				printf ("(1B) op %i placed in cycle %i "
					"slot %i\n",
					sm_op->lcode_op->id,
					cycle, sm_op->sched_slot);
			      break;
			    }
			}

		      if (sm_op->sched_slot == SM_MAX_SLOT && 
			  cycle == ready_time)
			{
			  if (SM_schedule_oper
			      (sm_op, cycle, (Lpipe_total_issue_slots - 1),
			       (Lpipe_total_issue_slots - 1), 0))
			    {
			      if (Lpipe_debug >= 1)
				printf ("(1L) op %i placed in cycle %i "
					"slot %i\n",
					sm_op->lcode_op->id,
					cycle, sm_op->sched_slot);
			    }
			}
		    }

		  if (sm_op->sched_slot != SM_MAX_SLOT)
		    break;	/* If scheduled, do not try any more cycles. */
		}
	      /* If scheduling attempts were unsuccessful, delete
	         dependent opers and put them back into the queue,
	         then scheduled this oper. */
	      if (sm_op->sched_slot == SM_MAX_SLOT)
		{
		  /* try II candidate cycles */
		  for (cycle = ready_time; cycle >= earliest_cycle; cycle--)
		    {
		      Lpipe_eject_opers_if_dependence_conflict (sm_op, II,
								cycle);

		      /* When scheduled, sched_slot will be changed from 
		         SM_MAX_SLOT (65535) to real slot */
		      /* 20030731 SZU
		       * Help reduce scheduling time.
		       * New MCKINLEY, or IPF template bundling algorithm (SZU)
		       * automatically tries all slots.
		       */
		      if (SM_do_template_bundling)
			{
			  if (SM_schedule_oper (sm_op, cycle, 0, 
						Lpipe_total_issue_slots - 1,
						0))
			    {
			      if (Lpipe_debug >= 1)
				printf ("(2B) op %i placed in cycle %i "
					"slot %i\n",
					sm_op->lcode_op->id,
					cycle, sm_op->sched_slot);
			      break;
			    }
			}
		      else
			{
			  int earliest_slot, latest_slot;

			  /* If ready cycle, start slot search at last
			     available slot based on dependences.  If
			     other cycles, successor dependences are met
			     as of the ready cycle, so try all. */

			  if (cycle == ready_time)
			    {
			      if (sm_op->slot_upper_bound >
				  (Lpipe_total_issue_slots - 2))
				latest_slot = Lpipe_total_issue_slots - 2;
			      else
				latest_slot = sm_op->slot_upper_bound;
			    }
			  else
			    {
			      latest_slot = Lpipe_total_issue_slots - 1;
			    }

			  earliest_slot = 0;

			  for (test_slot = latest_slot;
			       test_slot >= earliest_slot; test_slot--)
			    {
			      if (SM_schedule_oper
				  (sm_op, cycle, test_slot, test_slot, 0))
				{
				  if (Lpipe_debug >= 1)
				    printf
				      ("(2B) op %i placed in cycle %i "
				       "slot %i\n",
				       sm_op->lcode_op->id, cycle,
				       sm_op->sched_slot);
				  break;
				}
			    }
			  if (sm_op->sched_slot == SM_MAX_SLOT
			      && cycle == ready_time)
			    {
			      if (SM_schedule_oper (sm_op, cycle, 
					       Lpipe_total_issue_slots - 1,
				   Lpipe_total_issue_slots - 1, 0))
				{
				  if (Lpipe_debug >= 1)
				    printf ("(2L) op %i placed in cycle %i "
					    "slot %i\n", sm_op->lcode_op->id,
					    cycle, sm_op->sched_slot);
				}
			    }
			}

		      if (sm_op->sched_slot != SM_MAX_SLOT)
			break;	/* If scheduled, do not try any more cycles. */
		    }
		}

	      /* If scheduling attempts were unsuccessful, delete
	         dependent opers and put them back into the queue,
	         then scheduled this oper. */
	      if (sm_op->sched_slot == SM_MAX_SLOT)
		{
		  /* try II candidate cycles */
		  for (cycle = ready_time; cycle >= earliest_cycle; cycle--)
		    {
		      Lpipe_eject_opers_if_resource_conflict (sm_cb, cycle);

		      /* When scheduled, sched_slot will be changed from 
		         SM_MAX_SLOT (65535) to real slot */
		      /* 20030731 SZU
		       * Help reduce scheduling time.
		       * New MCKINLEY, or IPF template bundling algorithm (SZU)
		       * automatically tries all slots.
		       */
		      if (SM_do_template_bundling)
			{
			  if (SM_schedule_oper (sm_op, cycle, 0,
						(Lpipe_total_issue_slots - 1),
						0))
			    {
			      if (Lpipe_debug >= 1)
				printf ("(1B) op %i placed in cycle %i "
					"slot %i\n",
					sm_op->lcode_op->id,
					cycle, sm_op->sched_slot);
			      break;
			    }
			}
		      else
			{
			  int earliest_slot, latest_slot;

			  /* If ready cycle, start slot search at last
			     available slot based on dependences.  If
			     other cycles, successor dependences are met
			     as of the ready cycle, so try all. */

			  if (cycle == ready_time)
			    {
			      if (sm_op->slot_upper_bound >
				  (Lpipe_total_issue_slots - 2))
				latest_slot = Lpipe_total_issue_slots - 2;
			      else
				latest_slot = sm_op->slot_upper_bound;
			    }
			  else
			    latest_slot = Lpipe_total_issue_slots - 1;

			  earliest_slot = 0;

			  for (test_slot = latest_slot;
			       test_slot >= earliest_slot; test_slot--)
			    {
			      if (SM_schedule_oper
				  (sm_op, cycle, test_slot, test_slot, 0))
				{
				  if (Lpipe_debug >= 1)
				    printf
				      ("(2B) op %i placed in cycle %i "
				       "slot %i\n",
				       sm_op->lcode_op->id, cycle,
				       sm_op->sched_slot);
				  break;
				}
			    }
			  if (sm_op->sched_slot == SM_MAX_SLOT
			      && cycle == ready_time)
			    {
			      if (SM_schedule_oper
				  (sm_op, cycle, (Lpipe_total_issue_slots - 1),
				   (Lpipe_total_issue_slots - 1), 0))
				{
				  if (Lpipe_debug >= 1)
				    printf
				      ("(2L) op %i placed in cycle %i "
				       "slot %i\n",
				       sm_op->lcode_op->id, cycle,
				       sm_op->sched_slot);
				}
			    }
			}

		      if (sm_op->sched_slot != SM_MAX_SLOT)
			break;	/* If scheduled, do not try any more cycles. */
		    }
		}
	      if (sm_op->sched_slot == SM_MAX_SLOT)
		{
		  L_warn ("Lpipe_sm_schedule: "
			  "Incrementing II (from %d) - "
			  "Unable to schedule inst even though dep op "
			  "unscheduled (function %s, cb %d, oper %d)\n",
			  II, sm_cb->lcode_fn->name, header_cb->id,
			  sm_op->lcode_op->id);

		  if (Lpipe_debug >= 2)
		    SM_print_map (stdout, sm_cb->sm_mdes, sm_cb);

		  SM_unschedule_cb (sm_cb);
		  return 0;
		}
	    }
	  else			/* forward scheduling */
	    {
	      /* Try ii + 1 cycles.  Because of 0-cycle dependences, all the
	         slots in the first cycle may not have been checked. */
	      latest_cycle = ready_time + II;

	      /* try II candidate cycles */
	      for (cycle = ready_time; cycle <= latest_cycle; cycle++)
		{
		  if (SM_schedule_oper
		      (sm_op, cycle, 0, Lpipe_total_issue_slots - 1, 0))
		    {
		      if (Lpipe_debug >= 1)
			printf ("(1F) op %i placed in cycle %i slot %i\n",
				sm_op->lcode_op->id,
				cycle, sm_op->sched_slot);
		      break;
		    }
		}

	      /* If scheduling attempts were unsuccessful, delete
	         dependent opers and put them back into the queue,
	         then scheduled this oper. */
	      if (sm_op->sched_slot == SM_MAX_SLOT)
		{
		  Lpipe_eject_opers_if_dependence_conflict (sm_op, II,
							    latest_cycle);

		  /* try II candidate cycles */
		  for (cycle = ready_time; cycle <= latest_cycle; cycle++)
		    {
		      if (SM_schedule_oper
			  (sm_op, cycle, 0, Lpipe_total_issue_slots - 1, 0))
			{
			  if (Lpipe_debug >= 1)
			    printf ("(2F) op %i placed in cycle %i slot %i\n",
				    sm_op->lcode_op->id,
				    cycle, sm_op->sched_slot);
			  break;
			}
		    }
		  if (sm_op->sched_slot == SM_MAX_SLOT)
		    L_punt ("Lpipe_sm_schedule(F):"
			    "Unable to schedule inst even though "
			    "dependent ops unscheduled.");
		}
	    }			/* end forward or backward scheduling
				   attempt on real instruction */
	}			/* end schedule oper */

      /* Check for any real operation */
      if (sm_op->sched_slot != SM_MAX_SLOT)
	{
	  softpipe_info->scheduled = 1;
	  budget--;
	  softpipe_info->issue_time = cycle;
	  softpipe_info->intra_iter_issue_time = cycle;
	}

      /* Current instruction could not be scheduled. */
      if (!softpipe_info->scheduled)
	{
	  L_warn ("Lpipe_sm_schedule: "
		  "Incrementing II (from %d) - could not find a "
		  "slot without resource conflict "
		  "(function %s, cb %d)\n",
		  II, sm_cb->lcode_fn->name, header_cb->id);
	  success = 0;
	}
    }

  Q_delete_queue (Lpipe_ready_queue);

  /* If exhaust budget before scheduling all opers, inc II and try again */
  if (success)
    {
      if (sm_op)
	{
	  L_warn ("Lpipe_sm_schedule: "
		  "Incrementing II (from %d) - "
		  "budget exhausted (function %s, cb %d)\n",
		  II, sm_cb->lcode_fn->name, header_cb->id);
	  success = 0;
	}
      else
	{
	  sched_length = (sm_cb->last_sched_op->sched_cycle -
			  sm_cb->first_sched_op->sched_cycle);

	  /* This test controls the maximum number of stages.  When the
	     number of stages gets too large, the register pressure becomes
	     unmanagable.  Consider a register that is live from top to bottom
	     of the schedule...it uses num_stages number of registers. */
  
	  if ((sched_length / sm_cb->II) > Lpipe_max_stages)
	    {
	      L_warn ("Lpipe_sm_schedule: "
		      "Incrementing II (from %d) - too many stages "
		      "(function %s, cb %d)\n",
		      II, sm_cb->lcode_fn->name, header_cb->id);
	      success = 0;
	    }
	}
    }

  if (success)
    {
      int unroll;

      SM_construct_temp_kernel_queue (sm_cb);
      Lpipe_copy_sched_info (sm_cb);

      unroll = Lpipe_analyze_lr (sm_cb, 1);

      if (unroll == -1)
	{
	  L_warn ("Lpipe_sm_schedule: "
		  "Incrementing II (from %d) - not enough "
		  "available rotating registers "
		  "(function %s, cb %d)\n",
		  sm_cb->II, L_fn->name, sm_cb->lcode_cb->id);
	  success = 0;
	}
      else if ((unroll > 1) && 
	       (Lpipe_schema == MULTI_EPI) &&
	       loop_back_br->lcode_op->pred[0] &&
	       L_cond_branch_opcode (loop_back_br->lcode_op))
	{
	  L_warn ("Lpipe_sm_schedule: unable to reverse pbr in cb %d",
		  sm_cb->lcode_cb->id);
	  success = 0;
	}
    }

  if (!success)
    {
      Lpipe_sm_terminate (sm_cb);
      return 0;
    }

  /* Issue chain of opers leading to loop back branch as early as
     possible to avoid speculation of operations that the loop back
     branch does not depend on.  Can only be done for counted loops
     without early exits. */
  if (Lpipe_compact_branch_path_opers && Lpipe_counted_loop)
    Lpipe_do_compact_branch_path_opers (sm_cb, loop_back_br);

  if (Lpipe_debug >= 1)
    {
      printf ("Pipelining successful for II = %d (function %s, cb %d)\n",
	      II, sm_cb->lcode_fn->name, header_cb->id);
      if (Lpipe_debug >= 2)
	Lpipe_print_cb_schedule (stdout, L_fn, sm_cb);
    }

  if (Lpipe_print_schedules_for_debug)
    {
      fprintf (sched_file,
	       "\n\nPrinting cb schedule before mrt optimizations %s:"
	       "cb %d\n\n", sm_cb->lcode_fn->name, header_cb->id);
      Lpipe_print_cb_schedule (sched_file, L_fn, sm_cb);
    }

  if (M_arch != M_TAHOE)
    {
      /* MCM Disable for Tahoe at this point. */

      /* move operations with no incoming register dependence arcs as
         late as possible and opers with no outgoing dependence arcs
         as early as possible to save registers */
      Lpipe_optimize_lifetimes (sm_cb);
    }

  SM_construct_temp_kernel_queue (sm_cb);

  if (Lpipe_sort_mrt_rows)
    Lpipe_sort_MRT_rows (sm_cb);

  SM_set_sched_cycle_offset (sm_cb);
		 
  return 1;
}


void
Lpipe_sm_commit (SM_Cb *sm_cb)
{
  if (Lpipe_dump_dot)
    {
      char *buf = alloca (strlen (L_fn->name) + 32);
      sprintf (buf, "PipeDAG-%s-%d.dot", L_fn->name, sm_cb->lcode_cb->id);
#if 0
      SM_print_cb_dot_graph (sm_cb, buf);
#endif
    }

  /* Commit the cb's schedule */
  SM_commit_cb (sm_cb);

  Lpipe_copy_sched_info (sm_cb);
  Lpipe_cb_set_iter (sm_cb);
  Lpipe_cb_set_issue_time (sm_cb);

  return;
}
