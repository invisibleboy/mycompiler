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
 *      File: l_softpipe_info.c
 *      Description: Modulo scheduler
 *      Creation Date: January, 1994
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_softpipe_int.h"
#include "l_mve.h"
#include <Lcode/r_regalloc.h>

/* memory pools for Softpipe_Op_Info structures and relative latency and
   operand ready times arrays */
L_Alloc_Pool *Softpipe_Op_Info_pool = NULL;

/*************************************************************************
                Function Definitions
*************************************************************************/

/*----------------------------------------------------------------------
                     Softpipe_Op_Info structure routines
  ----------------------------------------------------------------------*/

/* The Softpipe_Op_Info structure is initialized before scheduling at each
   candidate II.  If lite = 1, don't initialize ru_info.  Lite = 1 is used
   for instructions that don't exist until after scheduling. */

void
Lpipe_init_op_info (SM_Oper * sm_op, int init_ready_time, int lite)
{
  Softpipe_Op_Info *softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);

  softpipe_info->intra_iter_issue_time = -1;
  softpipe_info->issue_time = -1;
  softpipe_info->issue_slot = -1;
  softpipe_info->stage = -1;
  softpipe_info->kernel_copy = -1;
  softpipe_info->prologue_stage = -1;
  softpipe_info->epilogue_stage = -1;
  softpipe_info->unrolled_iter_num = -1;

  softpipe_info->estart = -1;
  softpipe_info->lstart = -1;
  softpipe_info->slack = -1;
  softpipe_info->priority = IMPOSSIBLE_PRIORITY;
  softpipe_info->scheduled = 0;
  softpipe_info->ready_time = init_ready_time;
}


/* Create a Softpipe_Op_Info structure and return a pointer to it.  
   Initialize fields that don't need to be reinitialized for each 
   candidate II. */

Softpipe_Op_Info *
Lpipe_create_op_info ()
{
  Softpipe_Op_Info *softpipe_info;
  int i;

  softpipe_info = (Softpipe_Op_Info *) L_alloc (Softpipe_Op_Info_pool);
  softpipe_info->home_block = -1;
  softpipe_info->loop_back_br = 0;
  softpipe_info->exit_cb = NULL;
  softpipe_info->exit_weight = -1.0;
  softpipe_info->branch_path_node = 0;

  softpipe_info->src_mve_info =
    (Lpipe_MVEInfo **) L_alloc (Lpipe_src_mve_pool);
  softpipe_info->pred_mve_info =
    (Lpipe_MVEInfo **) L_alloc (Lpipe_pred_mve_pool);
  softpipe_info->dest_mve_info =
    (Lpipe_MVEInfo **) L_alloc (Lpipe_dest_mve_pool);

  softpipe_info->isrc_mve_info = NULL;

  for (i = 0; i < L_max_src_operand; i++)
    softpipe_info->src_mve_info[i] = NULL;
  for (i = 0; i < L_max_pred_operand; i++)
    softpipe_info->pred_mve_info[i] = NULL;
  for (i = 0; i < L_max_dest_operand; i++)
    softpipe_info->dest_mve_info[i] = NULL;

  return (softpipe_info);
}


void
Lpipe_construct_op_info (SM_Cb * sm_cb, SM_Oper * loop_back_br,
			 int *num_oper, int *branch_count)
{
  SM_Oper *sm_op;
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;
  L_Flow *flow;
  int home_block = 0;
  *num_oper = 0;
  *branch_count = 0;

  /* Create scheduling info and mark home block and other info for oper */
  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      oper = sm_op->lcode_op;

      /* Start and stop nodes will be created later. */
      if (oper->ext)
	continue;

      oper->ext = softpipe_info =
	(Softpipe_Op_Info *) Lpipe_create_op_info ();
      Lpipe_init_op_info (sm_op, -1, 0);
      softpipe_info->home_block = home_block;
      num_oper++;

      if (!L_cond_branch (oper) && !L_uncond_branch (oper))
	continue;

      /* next oper starts new basic block */
      home_block++;
      branch_count++;

      /* mark loop back branch and weight of exit */
      flow = L_find_flow_for_branch (sm_cb->lcode_cb, oper);
      if (sm_op == loop_back_br)
	{
	  softpipe_info->loop_back_br = 1;
	  /* L_cond_branch detects regular cond branches and
	     predicated uncond branches. */
	  if (L_cond_branch (oper) &&
	      !L_EXTRACT_BIT_VAL (sm_cb->lcode_cb->flags,
				  L_CB_HYPERBLOCK_NO_FALLTHRU))
	    {
	      if (!flow->next_flow)
		{
		  DB_print_cb (sm_cb->lcode_cb);
		  DB_print_oper (oper);
		  L_punt ("Lpipe_construct_op_info: "
			  "Expected a fall-through flow");
		}
	      /* exit is fall through path for loop back branch */
	      softpipe_info->exit_weight = flow->next_flow->weight;
	      softpipe_info->exit_cb = flow->next_flow->dst_cb;
	    }
	  else
	    {
	      if (flow->next_flow)
		L_punt ("Lpipe_construct_op_info: "
			"Expected no fall-through flow");
	      softpipe_info->exit_weight = 0.0;
	      softpipe_info->exit_cb = NULL;
	    }
	}
      else
	{
	  softpipe_info->exit_weight = flow->weight;
	  softpipe_info->exit_cb = L_find_branch_dest (oper);
	}
    }

  return;
}


/* Make a copy of the given Softpipe_Op_Info structure.  This is done
   when opers are copied for MVE and prologue/epilogue generation.  To
   save memory space and time, the structures pointed to by the
   Softpipe_Op_Info structure are NOT copied, but the pointer to them
   is.  Thus, only the sturctures pointed to by the Softpipe_Op_Info
   structures for the opers in the first copy of the kernel should
   ever be freed.  The structures pointed to by the Softpipe_Op_Info
   are all read only after the copying is done. */

Softpipe_Op_Info *
Lpipe_copy_op_info (Softpipe_Op_Info * softpipe_info)
{
  Softpipe_Op_Info *new_info;

  new_info = (Softpipe_Op_Info *) L_alloc (Softpipe_Op_Info_pool);
  new_info->loop_back_br = softpipe_info->loop_back_br;
  new_info->branch_path_node = softpipe_info->branch_path_node;
  new_info->issue_time = softpipe_info->issue_time;
  new_info->intra_iter_issue_time = softpipe_info->intra_iter_issue_time;
  new_info->issue_slot = softpipe_info->issue_slot;
  new_info->kernel_copy = softpipe_info->kernel_copy;
  new_info->stage = softpipe_info->stage;
  new_info->prologue_stage = softpipe_info->prologue_stage;
  new_info->epilogue_stage = softpipe_info->epilogue_stage;
  new_info->home_block = softpipe_info->home_block;
  new_info->exit_weight = softpipe_info->exit_weight;
  new_info->exit_cb = softpipe_info->exit_cb;
  new_info->unrolled_iter_num = softpipe_info->unrolled_iter_num;
  new_info->src_mve_info = softpipe_info->src_mve_info;
  new_info->pred_mve_info = softpipe_info->pred_mve_info;
  new_info->dest_mve_info = softpipe_info->dest_mve_info;
  new_info->estart = softpipe_info->estart;
  new_info->lstart = softpipe_info->lstart;
  new_info->slack = softpipe_info->slack;
  new_info->priority = softpipe_info->priority;
  new_info->ready_time = softpipe_info->ready_time;
  new_info->scheduled = softpipe_info->scheduled;
  return (new_info);
}


void
Lpipe_copy_sched_info (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  L_Oper *oper;
  L_Attr *attr_isl, *attr_stage;
  Softpipe_Op_Info *softpipe_info;

  /* In sm_op, the sched_cycle is normalized to 0, so this is not an 
     accurate number for issue time.  For example, if the first instruction
     is really issued in cycle 1, each instruction's sched_time will be 
     one cycle short. */

  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      oper = sm_op->lcode_op;
      softpipe_info = SOFTPIPE_OP_INFO (oper);

      if (L_START_STOP_NODE (oper))
	continue;

      if (!(attr_stage = L_find_attr (oper->attr, "stage")))
	L_punt ("Lpipe_copy_sched_info: scheduled oper found with "
		"no stage attribute.");
      softpipe_info->stage = attr_stage->field[0]->value.i;

      if ((attr_isl = L_find_attr (oper->attr, "isl")))
	softpipe_info->intra_iter_issue_time = attr_isl->field[0]->value.i +
	  softpipe_info->stage * sm_cb->II;
      else if ((attr_isl = L_find_attr (oper->attr, "cycle")))
	softpipe_info->intra_iter_issue_time = attr_isl->field[0]->value.i;
      else
	L_punt ("Lpipe_copy_sched_info: scheduled oper found with "
		"no isl or cycle attribute.");

      softpipe_info->issue_time = softpipe_info->intra_iter_issue_time;
      softpipe_info->issue_slot = sm_op->sched_slot;
    }
  return;
}


/* free Softpipe_Op_Info structure for each oper that has one in cb */
void
Lpipe_free_op_info (L_Cb * cb)
{
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      softpipe_info = SOFTPIPE_OP_INFO (oper);
      if (softpipe_info != NULL)
	{
	  L_free (Softpipe_Op_Info_pool, softpipe_info);
	  oper->ext = 0;
	}
    }
}

/* 20031119 SZU */
/* free Softpipe_Op_Info structure for the oper */
void
Lpipe_free_oper_op_info (L_Oper * oper)
{
  Softpipe_Op_Info *softpipe_info;

  softpipe_info = SOFTPIPE_OP_INFO (oper);
  if (softpipe_info != NULL)
    {
      if (softpipe_info->src_mve_info)
	L_free (Lpipe_src_mve_pool, softpipe_info->src_mve_info);
      if (softpipe_info->pred_mve_info)
	L_free (Lpipe_pred_mve_pool, softpipe_info->pred_mve_info);
      if (softpipe_info->dest_mve_info)
	L_free (Lpipe_dest_mve_pool, softpipe_info->dest_mve_info);

      L_free (Softpipe_Op_Info_pool, softpipe_info);
      oper->ext = 0;
    }
}

void
Lpipe_print_cb_info (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;

  printf ("**** sm_cb %d\n", sm_cb->lcode_cb->id);

  for (sm_op = sm_cb->first_serial_op;
       sm_op != NULL; sm_op = sm_op->next_serial_op)
    Lpipe_print_op_info (sm_op);

  printf ("****\n");

  return;
}

void
Lpipe_print_op_info (SM_Oper * sm_op)
{
  Softpipe_Op_Info *softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);

  printf ("** sm_op id %d\n", sm_op->lcode_op->id);

  if (softpipe_info == NULL)
    {
      printf ("No info\n");
      return;
    }

  printf ("intra_iter_issue_time %d, issue_time %d, issue_slot %d\n",
	  softpipe_info->intra_iter_issue_time,
	  softpipe_info->issue_time, softpipe_info->issue_slot);

  printf ("stage %d, epilogue_stage %d, prologue_stage %d\n",
	  softpipe_info->stage,
	  softpipe_info->prologue_stage, softpipe_info->epilogue_stage);

  printf ("kernel_copy %d, unrolled_iter_num %d\n",
	  softpipe_info->kernel_copy, softpipe_info->unrolled_iter_num);

  printf ("estart %d, lstart %d, slack %d\n",
	  softpipe_info->estart, softpipe_info->lstart, softpipe_info->slack);

  printf ("priority %d, scheduled %d, ready_time %d\n\n",
	  softpipe_info->priority,
	  softpipe_info->scheduled,
	  softpipe_info->ready_time);

  return;
}
