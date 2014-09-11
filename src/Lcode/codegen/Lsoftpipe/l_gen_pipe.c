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
 *      File: l_gen_pipe.c
 *      Description: code generation schema routines
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
#include "l_pipe_sync.h"
#include <Lcode/r_regalloc.h>

#define UNKNOWN_WEIGHT 0	/* cb has weight which can't be estimated   */

#define FALL_THRU_EPI 0		/* epilogue for fall through out of kernel  */
#define TARGET_EPI 1		/* epilogue for taken kernel or prolog exit */

/*------------------------------------------------------------------------
                     Remainder loop code schema
                  -- MCM Not currently supported --
  ------------------------------------------------------------------------*/

/* Generate remainder loop code schema given the iteration variable
   increment operation */

/*
   If original loop is of the form:
           Loop:  ~~~~~~~~~~
                  ~~~~~~~~~~
                  r100 = r100 + incr_reg
                  ble r100, max_reg
   generate:
           Preheader: ~~~~~~~~~
                      ~~~~~~~~~
                      r200 = num_increments * incr_reg
                      r201 = r200 + r100
           Prologue:  bgt r201, max_reg, Rem_loop
                      ~~~~~~~~~
                      ~~~~~~~~~
                      r300 = (unroll - 1) * incr_reg
                      new_max_reg = max_reg - r300
           Kernel:    ~~~~~~~~~
                      ~~~~~~~~~
                      ble r100, new_max_reg, Kernel
           Epilogue:  ~~~~~~~~~
                      ~~~~~~~~~
                      bgt r100, max, End
           Rem_loop:  same as original loop
           End:


   num_increments is the number of iterations initiated in the prologue and
   first iteration of the kernel minus 1. */

void
Lpipe_rem_loop_gen (L_Func * fn, L_Oper * loop_incr, 
		    int ii, int unroll, int stage_count)
{
  L_Cb *epilogue_cb;
  L_Oper *rem_loop_br;		/* loop back branch for the remainder loop */
  L_Operand *ind_var_src;	/* loop index variable */
  L_Operand *ind_incr_src;	/* increment operand for induction op */
  L_Operand *max_src;		/* loop bound source */
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;
  int starting_copy;
  int last_copy;
  int pro_stage;
  int epi_stage;
  int i;
  int copy;
  L_Oper *first_oper;
  L_Oper *last_oper;
  int intra_iter_issue_time;
  L_Oper *new_oper;
  Softpipe_Op_Info *new_info;
  int num_increments;
  int total_increment;
  L_Oper *last_prologue_op;

  /* find prologue, epilogue, and remainder cb's */
  prologue_cb = header_cb->prev_cb;
  epilogue_cb = header_cb->next_cb;
  remainder_cb = epilogue_cb->next_cb;
  rem_loop_br = Lpipe_find_loop_back_br (fn, remainder_cb);

  /* fill in instructions that were inserted during phase 1 */

  /* find index variable and its increment operand */
  if (L_same_operand (loop_incr->dest[0], loop_incr->src[0]))
    {
      ind_var_src = loop_incr->src[0];
      ind_incr_src = loop_incr->src[1];
    }
  else
    {
      ind_var_src = loop_incr->src[1];
      ind_incr_src = loop_incr->src[0];
    }

  /* find loop bound */
  if (L_same_operand (ind_var_src, rem_loop_br->src[0]))
    {
      max_src = rem_loop_br->src[1];
    }
  else
    {
      max_src = rem_loop_br->src[0];
    }

  /* need to check whether there are at least as many iterations as will
     be initiated in the prologue and the first iteration of the kernel.
     stage count - 1 increments of index variable during prologue, unroll 
     increments during the first iteration of the kernel.  Subtract one to
     get the value of the induction variable used to determine if the
     iteration initiated in the last stage of the kernel will execute. */

  num_increments = stage_count + unroll - 2;
  if (L_is_register (ind_incr_src))
    {
      /* Need to multiply the number of increments by the increment operand.
         This multiplication is the 2nd to the last oper in preheader. */
      oper = preheader_cb->last_op->prev_op;
      /* src[0] is an integer constant equal to the number of increments */
      L_delete_operand (oper->src[0]);
      oper->src[0] = L_new_gen_int_operand (num_increments);
    }
  else
    {
      /* increment is a constant, so compiler can do the multiplication.
         The add is the last operation in preheader */
      oper = preheader_cb->last_op;
      L_delete_operand (oper->src[0]);
      /* src[0] is the total increment of the index variable */
      total_increment = num_increments * (int) ind_incr_src->value.i;
      oper->src[0] = L_new_gen_int_operand (total_increment);
    }

  /* Each iteration of the kernel, unroll iterations will be initiated.
     Subtract once from max rather than adding to induction variable 
     every iteration of kernel. */
  num_increments = unroll - 1;
  if (L_is_register (ind_incr_src))
    {
      /* Need to multiply the number of increments by the increment operand.
         This multiplication is the 2nd to the last oper in prologue. */
      oper = prologue_cb->last_op->prev_op;
      /* src[0] is an integer constant equal to the number of increments */
      L_delete_operand (oper->src[0]);
      oper->src[0] = L_new_gen_int_operand (num_increments);
    }
  else
    {
      if (L_is_register (max_src))
	{
	  /* increment is a constant, so compiler can do the multiplication.
	     The add is the last operation in prologue */
	  oper = prologue_cb->last_op;
	  L_delete_operand (oper->src[0]);
	  /* src[0] is the total increment of the index variable */
	  total_increment = num_increments * (int) ind_incr_src->value.i;
	  /* negate total_increment to do subtract */
	  oper->src[0] = L_new_gen_int_operand (-total_increment);
	}
      else
	{
	  /* Total increment is moved to a register and then subtracted
	     from the constant max.  The move operation is the 2nd to last
	     in prologue */
	  oper = prologue_cb->last_op->prev_op;
	  L_delete_operand (oper->src[0]);
	  total_increment = num_increments * (int) ind_incr_src->value.i;
	  oper->src[0] = L_new_gen_int_operand (total_increment);
	}
    }

  /* Generate the stages of the prologue from last to first.
     Last stage of the prologue is stage_count - 2.  This should be a copy of
     kernel_copy[unroll - 1] with instructions from the last stage deleted.
     Each preceding stage of the prologue will use the preceding kernel
     copy modulo unroll.  There will be less than stage_count stages in the
     prologue.  (stage_count / unroll + 1) is a ceiling on the number of
     copies of the unrolled kernel needed to make the prologue (note that
     stage_count does not have to be a multiple of unroll).
     (stage_count / unroll + 1) * unroll is an upper bound on the number of
     kernel stages that need to be copied.  This number modulo unroll is 0.
     ((stage_count / unroll + 1) * unroll - 1) modulo unroll gives unroll-1.
     Thus, starting_copy modulo unroll gives the kernel copy needed for the
     last stage of the prologue. The second to the last stage of the prologue
     is copied from kernel_copy[(starting_copy - 1) mod unroll)] and so on.
     Since starting_copy is an upper bound on the number of kernel stages 
     that need to be copied, i below will remain positive for until all
     stages of the prologue have been generated. */

  last_prologue_op = prologue_cb->last_op;
  starting_copy = (stage_count / unroll + 1) * unroll - 1;
  last_copy = starting_copy - (stage_count - 2);
  pro_stage = stage_count - 2;
  for (i = starting_copy; i >= last_copy; i--)
    {
      copy = i % unroll;
      first_oper = kernel_copy_first_op[copy];
      last_oper = kernel_copy_last_op[copy];
      for (oper = last_oper; oper != NULL; oper = oper->prev_op)
	{
	  if (!Lpipe_ignore_kernel_inst (oper))
	    {
	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      intra_iter_issue_time = softpipe_info->intra_iter_issue_time;
	      /* At stage x of prologue, instructions from stage x+1 of the
	         pipeline are not executed (the first iteration has not 
	         progressed that far yet).  The first instruction of stage x+1
	         has intra-iteration issue time (x+1)*ii.  Also, don't copy
	         loop back branches to prologue. */
	      if ((intra_iter_issue_time < (pro_stage + 1) * ii) &&
		  (!L_int_cond_branch_opcode (oper)))
		{
		  new_oper = L_copy_operation (oper);
		  new_oper->ext =
		    (Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
		  new_info = SOFTPIPE_OP_INFO (new_oper);
		  new_info->kernel_copy = copy;
		  new_info->prologue_stage = pro_stage;
		  /* issue time relative to beginning of prologue */
		  new_info->issue_time = pro_stage * ii +
		    intra_iter_issue_time % ii;
		  L_insert_oper_after (prologue_cb, last_prologue_op,
				       new_oper);
		}
	      if (oper == first_oper)
		break;
	    }
	}
      pro_stage = pro_stage - 1;
    }

  Lpipe_adjust_syncs_for_prologue ();

  /* Generate the stages of the epilogue from first to last.
     First stage of the epilogue is stage 0.  This should be a copy
     of kernel_copy[0] with instructions from the first stage deleted.
     Each succeeding stage of the prologue will use the succeeding kernel
     copy modulo unroll.  */
  epi_stage = 1;
  for (i = 0; i < stage_count - 1; i++)
    {
      copy = i % unroll;
      first_oper = kernel_copy_first_op[copy];
      last_oper = kernel_copy_last_op[copy];
      for (oper = first_oper; oper != NULL; oper = oper->next_op)
	{
	  if (!Lpipe_ignore_kernel_inst (oper))
	    {
	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      intra_iter_issue_time = softpipe_info->intra_iter_issue_time;
	      /* At stage x of epilogue, instructions from stage x-1 of 
	         the final iteration are already executed.  Therefore 
	         only instructions from stage x or later of the pipeline 
	         are included. The first instruction of stage x has 
	         intra-iteration issue time x*ii.  Also, don't copy loop 
	         back branches to epilogue. */
	      if ((intra_iter_issue_time >= epi_stage * ii) &&
		  (!L_int_cond_branch_opcode (oper)))
		{
		  new_oper = L_copy_operation (oper);
		  new_oper->ext =
		    (Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
		  new_info = SOFTPIPE_OP_INFO (new_oper);
		  /* issue time relative to beginning of epilogue */
		  new_info->issue_time = (epi_stage - 1) * ii +
		    intra_iter_issue_time % ii;
		  new_info->kernel_copy = copy;
		  new_info->epilogue_stage = epi_stage;
		  L_insert_oper_before (epilogue_cb, epilogue_cb->last_op,
					new_oper);
		}
	      if (oper == last_oper)
		break;
	    }
	}
      epi_stage = epi_stage + 1;
    }

  Lpipe_adjust_syncs_for_epilogue (epilogue_cb);
}


/*------------------------------------------------------------------------
                     Muliple epilogue code schema
  ------------------------------------------------------------------------*/

/* Generate an epilogue for an exit branch from the kernel. */

static void
Lpipe_gen_epi (L_Func * fn, L_Cb * header_cb, L_Oper * exit_branch,
	       L_Cb * fall_thru_cb, int epi_type, int ii, int unroll,
	       int stage_count)
{
  double epi_weight;		/* weight of epilogue cb */
  L_Cb *epilogue_cb;
  L_Cb *exit_cb = NULL;
  Softpipe_Op_Info *exit_branch_info;
  Softpipe_Op_Info *softpipe_info;
  L_Attr *attr;
  L_Oper *new_oper;

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);

  /* Create epilogue cb and add it to control flow graph.  The last 
     epilogue cb created is placed the closest to the kernel cb. */
  epi_weight = exit_branch_info->exit_weight / unroll;
  epilogue_cb = L_create_cb (epi_weight);
  attr = L_new_attr ("epilogue", 0);
  epilogue_cb->attr = L_concat_attr (epilogue_cb->attr, attr);

  L_insert_cb_after (fn, header_cb, epilogue_cb);

  if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK))
    epilogue_cb->flags = L_SET_BIT_FLAG (epilogue_cb->flags, L_CB_HYPERBLOCK);

  {
    /*
     * FLOW MANIPULATION
     * ----------------------------------------------------------------------
     */
    L_Flow *new_flow = NULL;
    int cc;

    /* kernel to epilogue flows */
    if (epi_type == FALL_THRU_EPI)
      {
	new_flow = L_new_flow (0, header_cb, epilogue_cb, epi_weight);
	header_cb->dest_flow = L_concat_flow (header_cb->dest_flow, new_flow);
	exit_cb = fall_thru_cb;
      }
    else if (epi_type == TARGET_EPI)
      {
	new_flow = L_new_flow (1, header_cb, epilogue_cb, epi_weight);
	header_cb->dest_flow = L_insert_flow_before (header_cb->dest_flow,
						     header_cb->dest_flow,
						     new_flow);
	exit_cb = L_find_branch_dest (exit_branch);
	L_change_branch_dest (exit_branch, exit_cb, epilogue_cb);
      }
    else
      {
	L_punt ("gen_epi: Unsupported epi_type");
      }

    epilogue_cb->src_flow = L_copy_single_flow (new_flow);

    /* epilogue to exit cb flows */

    cc = (epilogue_cb->next_cb != exit_cb);
    epilogue_cb->dest_flow =
      L_new_flow (cc, epilogue_cb, exit_cb, epi_weight);
    new_flow = L_new_flow (cc, epilogue_cb, exit_cb, epi_weight);

    exit_cb->src_flow = L_insert_flow_before (exit_cb->src_flow,
					      exit_cb->src_flow, new_flow);
  }
  {
    /*
     * OPER COPYING
     * ----------------------------------------------------------------------
     */
    int theta;			/* intra-iter stage with exit branch         */
    int epi_stage;		/* current epilogue stage number             */
    int iter_stage;		/* stage of the iter which contains oper     */
    int copy;			/* copy of kernel which contains exit branch */
    int unrolled_iter_num;	/* Number to identify the current
				   copy of the whole unrolled kernel.
				   Copy numbers are positive numbers 
				   starting with 1. */

    L_Oper *first_oper, *last_oper, *oper;
    L_Flow *new_flow;
    Softpipe_Op_Info *new_info;
    L_Cb *dest_cb;

    /* Generate the stages of the epilogue from first to last.  The 
       first stage of the epilogue is called stage 0.  Epilogue stage 0 is for
       the opers remaining in same kernel copy as the exit branch.  The number
       of stages in the epilogue is reduced by theta.  Start copying 
       from the kernel copy which contains the exit branch and wrap around 
       the kernel until all epilogue stages are generated. */

    theta = exit_branch_info->stage;
    copy = exit_branch_info->kernel_copy - 1;
    unrolled_iter_num = 1;
    for (epi_stage = 0; epi_stage < stage_count - theta; epi_stage++)
      {
	copy = (copy + 1) % unroll;

	last_oper = kernel_copy_last_op[copy];

	if (epi_stage == 0)
	  {
	    /* if exit branch is the last oper in stage - i.e., a copy of the
	       loop back branch, there will be no opers in epilogue stage 0 */
	    if (exit_branch == last_oper)
	      continue;
	    first_oper = exit_branch->next_op;
	  }
	else
	  {
	    first_oper = kernel_copy_first_op[copy];
	  }

	/* if starting to copy from the beginning of the unrolled kernel and
	 * the epilogue already contains opers from a previous major iteration
	 * of the unrolled kernel, update the number for the "iter" attibute. 
	 */
	if ((copy == 0) && (epilogue_cb->first_op != NULL))
	  unrolled_iter_num++;

	for (oper = first_oper; oper != NULL; oper = oper->next_op)
	  {
	    if (!Lpipe_ignore_kernel_inst (oper))
	      {
		softpipe_info = SOFTPIPE_OP_INFO (oper);
		iter_stage = softpipe_info->stage;

		/* At stage x of epilog, instructions from stage x-1+theta of 
		 * the final iteration are already executed.  Therefore only 
		 * instructions from stage x+theta or later of the iteration 
		 * are included.
		 * This effectively excludes instructions from iterations after
		 * the final iteration since those iterations are executing in 
		 * earlier stages.  If the oper is from the final iteration,
		 * only copy if it was originally above the branch in the 
		 * original loop body. 
		 */

		if ((iter_stage > (epi_stage + theta))
		    || ((iter_stage == (epi_stage + theta)) &&
			(softpipe_info->home_block <=
			 exit_branch_info->home_block)))
		  {
		    new_oper = L_copy_operation (oper);
		    new_oper->ext =
		      (Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
		    new_info = SOFTPIPE_OP_INFO (new_oper);
		    /* issue time relative to beginning of epilogue */
		    new_info->issue_time = epi_stage * ii +
		      softpipe_info->intra_iter_issue_time % ii;
		    new_info->kernel_copy = copy;
		    new_info->epilogue_stage = epi_stage;
		    new_info->unrolled_iter_num = unrolled_iter_num;
		    attr = L_new_attr ("unrolled_iter", 1);
		    L_set_int_attr_field (attr, 0, unrolled_iter_num);
		    new_oper->attr = L_concat_attr (new_oper->attr, attr);
		    L_insert_oper_after (epilogue_cb, epilogue_cb->last_op,
					 new_oper);
		    if (L_cond_branch (oper))
		      {
			dest_cb = L_find_branch_dest (oper);
			new_flow = L_new_flow (1, epilogue_cb, dest_cb,
					       UNKNOWN_WEIGHT);
			epilogue_cb->dest_flow =
			  L_concat_flow (epilogue_cb->dest_flow, new_flow);
			new_flow = L_copy_single_flow (new_flow);
			dest_cb->src_flow =
			  L_concat_flow (dest_cb->src_flow, new_flow);
		      }
		  }
		if (oper == last_oper)
		  break;
	      }
	  }
      }
  }

  /* add jump to fall through cb */
  if (epilogue_cb->next_cb != exit_cb)
    {
      new_oper = L_create_new_op (Lop_JUMP);
      new_oper->src[0] = L_new_cb_operand (exit_cb);
      L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper);
      L_annotate_oper (fn, epilogue_cb, new_oper);
      L_delete_oper (epilogue_cb, new_oper);
    }

  Lpipe_adjust_syncs_for_epilogue (epilogue_cb);
  return;
}


/* Find the corresponding copy of exit_branch in the kernel copy defined
   by first_oper and last_oper */
L_Oper *
Lpipe_find_exit_branch_copy (L_Oper * exit_branch, L_Oper * first_oper,
			     L_Oper * last_oper)
{
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;
  Softpipe_Op_Info *exit_branch_info;

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);

  for (oper = first_oper; oper != NULL; oper = oper->next_op)
    {
      if (!Lpipe_ignore_kernel_inst (oper))
	{
	  softpipe_info = SOFTPIPE_OP_INFO (oper);
	  if (softpipe_info->intra_iter_issue_time ==
	      exit_branch_info->intra_iter_issue_time)
	    {
	      if (softpipe_info->issue_slot == exit_branch_info->issue_slot)
		{
		  return (oper);
		}
	    }
	  if (oper == last_oper)
	    {
	      L_punt ("Lpipe_find_exit_branch_copy: "
		      "Copy of exit branch not found.\n");
	    }
	}
    }

  return 0;
}


/* Generate an epilogue for an exit branch from the prologue.  Place all
   partial epilogues at the end of the function. */
static void
Lpipe_gen_partial_epi (L_Func * fn, L_Cb * header_cb, L_Oper * exit_branch,
		       L_Cb * fall_thru_cb, int ii, int unroll, int stage_count)
{
  L_Cb *epilogue_cb, *exit_cb, *dest_cb;
  L_Flow *new_flow;
  Softpipe_Op_Info *exit_branch_info;
  Softpipe_Op_Info *softpipe_info;
  int copy;			/* copy of kernel from which the prolog 
				   code which contains the exit branch 
				   was copied */
  int pro_stage;		/* stage of prologue which 
				   contains exit branch */
  int epi_stage;		/* current epilogue stage number */
  int iter_stage;		/* stage of the iter which contains oper */
  L_Oper *first_oper;
  L_Oper *last_oper;
  L_Oper *oper;
  L_Oper *new_oper;
  Softpipe_Op_Info *new_info;
  L_Attr *attr;
  int unrolled_iter_num;	/* Number to identify the current
				   copy of the whole unrolled kernel.
				   Copy numbers are positive numbers
				   starting with 1. */
  int theta;			/* intra-iter stage containing exit branch */
  L_Oper *exit_branch_copy;	/* corresponding copy of prologue exit branch
				   in kernel */

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);

  /* Create epilogue cb and add it to control flow graph.  The 
     epilogue cbs for exits from the prologue are placed at the end
     of the function because it is assumed that most of the time the
     kernel will be entered. */
  epilogue_cb = L_create_cb (UNKNOWN_WEIGHT);
  L_insert_cb_after (fn, fn->last_cb, epilogue_cb);
  attr = L_new_attr ("epilogue", 0);
  epilogue_cb->attr = L_concat_attr (epilogue_cb->attr, attr);

  if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK))
    epilogue_cb->flags = L_SET_BIT_FLAG (epilogue_cb->flags, L_CB_HYPERBLOCK);

  /* prologue to epilogue flows */
  new_flow = L_new_flow (1, prologue_cb, epilogue_cb, UNKNOWN_WEIGHT);
  prologue_cb->dest_flow = L_insert_flow_before (prologue_cb->dest_flow,
						 prologue_cb->dest_flow,
						 new_flow);
  epilogue_cb->src_flow = L_copy_single_flow (new_flow);

  /* find exit cb */

  exit_cb = L_find_branch_dest (exit_branch);

  /* epilogue to exit cb flows */
  epilogue_cb->dest_flow =
    L_new_flow (1, epilogue_cb, exit_cb, UNKNOWN_WEIGHT);
  new_flow = L_new_flow (1, epilogue_cb, exit_cb, UNKNOWN_WEIGHT);
  exit_cb->src_flow = L_insert_flow_before (exit_cb->src_flow,
					    exit_cb->src_flow, new_flow);

  L_change_branch_dest (exit_branch, L_find_branch_dest (exit_branch),
			epilogue_cb);


  /* Generate the stages of the epilogue from first to last.  The
     first stage of the epilogue is called stage 0.  Epilogue stage 0 is for
     the opers remaining in same kernel copy as the exit branch.  The number
     of stages in the epilogue is reduced by theta.  Start copying
     from the kernel copy which contains the exit branch and wrap around
     the kernel until all epilogue stages are generated. */
  theta = exit_branch_info->stage;
  pro_stage = exit_branch_info->prologue_stage;
  copy = exit_branch_info->kernel_copy - 1;
  unrolled_iter_num = 1;
  for (epi_stage = 0; epi_stage < stage_count - theta; epi_stage++)
    {
      copy = (copy + 1) % unroll;

      first_oper = kernel_copy_first_op[copy];
      last_oper = kernel_copy_last_op[copy];
      if (epi_stage == 0)
	{
	  exit_branch_copy =
	    Lpipe_find_exit_branch_copy (exit_branch, first_oper, last_oper);
	  /* if exit branch is the last oper in stage - i.e., a copy of the
	     loop back branch, there will be no opers in epilogue stage 0 */
	  if (exit_branch_copy == last_oper)
	    continue;
	  first_oper = exit_branch_copy->next_op;
	}

      /* if starting to copy from the beginning of the unrolled kernel and
         the epilogue already contains opers from a previous major iteration
         of the unrolled kernel, update the number for the "iter" attibute. */
      if ((copy == 0) && (epilogue_cb->first_op != NULL))
	unrolled_iter_num++;

      for (oper = first_oper; oper != NULL; oper = oper->next_op)
	{
	  if (!Lpipe_ignore_kernel_inst (oper))
	    {
	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      iter_stage = softpipe_info->stage;

	      /* At stage x of epilogue, instructions from stage x-1+theta 
	         of the final iteration are already executed.  Therefore 
	         only instructions from stage x+theta or later of the 
	         pipeline are included.  This effectively excludes 
	         instructions from iterations after the final iteration 
	         since those iterations are executing in earlier stages.  
	         If the oper is from the final iteration, only copy if it 
	         was originally above the branch in the original loop body. 

	         In the prologue, execution of the pipeline is ramping up so,
	         if an exit branch is taken, there are not as many iterations
	         that need to be finished up as in the kernel.  Therefore
	         don't copy instructions from any stage later than the
	         stage that the first iteration is currently executing.  For
	         an exit from prologue stage pro_stage and during epilogue
	         stage epi_stage, the first iteration is executing stage
	         pro_stage + epi_stage.  */


	      if (((iter_stage > (epi_stage + theta)) ||
		   (iter_stage == (epi_stage + theta) &&
		    softpipe_info->home_block <=
		    exit_branch_info->home_block))
		  && (iter_stage <= (pro_stage + epi_stage)))
		{
		  new_oper = L_copy_operation (oper);
		  new_oper->ext =
		    (Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
		  new_info = SOFTPIPE_OP_INFO (new_oper);
		  /* issue time relative to beginning of epilogue */
		  new_info->issue_time = epi_stage * ii +
		    softpipe_info->intra_iter_issue_time % ii;
		  new_info->kernel_copy = copy;
		  new_info->epilogue_stage = epi_stage;
		  new_info->unrolled_iter_num = unrolled_iter_num;
		  attr = L_new_attr ("unrolled_iter", 1);
		  L_set_int_attr_field (attr, 0, unrolled_iter_num);
		  new_oper->attr = L_concat_attr (new_oper->attr, attr);
		  L_insert_oper_after (epilogue_cb, epilogue_cb->last_op,
				       new_oper);
		  if (L_cond_branch (oper))
		    {
		      dest_cb = L_find_branch_dest (oper);
		      new_flow = L_new_flow (1, epilogue_cb, dest_cb,
					     UNKNOWN_WEIGHT);
		      epilogue_cb->dest_flow =
			L_concat_flow (epilogue_cb->dest_flow, new_flow);
		      new_flow = L_copy_single_flow (new_flow);
		      dest_cb->src_flow =
			L_concat_flow (dest_cb->src_flow, new_flow);
		    }
		}
	      if (oper == last_oper)
		break;
	    }
	}
    }
  /* add jump to fall through cb */
  if (epilogue_cb->next_cb != exit_cb)
    {
      new_oper = L_create_new_op (Lop_JUMP);
      new_oper->src[0] = L_new_cb_operand (exit_cb);
      L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper);
      L_annotate_oper (fn, epilogue_cb, new_oper);
      L_delete_oper (epilogue_cb, new_oper);
    }

  Lpipe_adjust_syncs_for_epilogue (epilogue_cb);
}

extern void M_starcore_negate_compare (L_Oper *);

/* generate multiple epilogues */
void
Lpipe_multi_epi_gen (L_Func * fn, int stage_count, L_Oper * loop_back_br,
		     int ii, int unroll)
{
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;

  int starting_copy;
  int last_copy;
  int pro_stage;
  int i;
  int copy;
  L_Oper *first_oper, *last_oper, *prev_oper;
  int iter_stage;
  L_Oper *new_oper;
  Softpipe_Op_Info *new_info;

  L_Cb *epilogue_cb;
  L_Cb *dest_cb;

  L_Flow *flow;
  L_Flow *new_flow;
  L_Flow *src_flow;
  L_Flow *next_flow;

  L_Flow *fall_thru_flow;	/* flow for fall thru path of loop back br. */
  L_Cb *fall_thru_cb;		/* cb in fall thru path of loop back branch */

  L_Flow *loop_back_flow;	/* taken flow of loop back branch */
  double loop_back_weight;	/* weight of loop_back_flow */

  L_Attr *attr;
  int num_unrolled_kernel_iters;	/* Number of different copies of the
					   unrolled kernel that appear in the
					   prologue (in whole or in part). */
  int unrolled_iter_num;	/* Number to identify the current
				   copy of the unrolled kernel. 
				   Copies are numbered 1 to 
				   num_unrolled_kernel_iters. */

  /*
   * PROFILE INFORMATION UPDATE
   * ----------------------------------------------------------------------
   */

  loop_back_flow = L_find_flow_with_dst_cb (header_cb->dest_flow, header_cb);
  src_flow = L_find_matching_flow (header_cb->src_flow, loop_back_flow);
  loop_back_weight = loop_back_flow->weight / unroll;
  L_change_flow (loop_back_flow, 1, header_cb, header_cb, loop_back_weight);
  L_change_flow (src_flow, 1, header_cb, header_cb, loop_back_weight);
  header_cb->weight -= loop_back_weight * (unroll - 1);

  /*
   * PROLOGUE GENERATION
   * ----------------------------------------------------------------------
   */

  /* There is guaranteed to be a single preheader cb that flows
     into the loop header.  The preheader is created */

  prologue_cb = L_create_cb (preheader_cb->dest_flow->weight);
  L_insert_cb_before (fn, header_cb, prologue_cb);
  attr = L_new_attr ("prologue", 0);
  prologue_cb->attr = L_concat_attr (prologue_cb->attr, attr);

  if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK))
    prologue_cb->flags = L_SET_BIT_FLAG (prologue_cb->flags, L_CB_HYPERBLOCK);

  /* preheader to prologue flows */
  if (preheader_cb->dest_flow->dst_cb != header_cb)
    L_punt ("Lpipe_multi_epi_gen: dest cb for preheader flow is not header:"
	    " preheader_cb is %d, header_cb is %d, dest of flow is %d\n",
	    preheader_cb->id, header_cb->id,
	    preheader_cb->dest_flow->dst_cb->id);

  preheader_cb->dest_flow->dst_cb = prologue_cb;
  prologue_cb->src_flow = L_new_flow (preheader_cb->dest_flow->cc,
				      preheader_cb, prologue_cb,
				      prologue_cb->weight);

  /* Prologue to header flows.  Assume only flows into header cb are
     from preheader and loop back branch */

  flow = header_cb->src_flow;
  if (flow->src_cb == header_cb)
    flow = flow->next_flow;
  flow->src_cb = prologue_cb;

  prologue_cb->dest_flow = L_new_flow (flow->cc, prologue_cb, header_cb,
				       prologue_cb->weight);

  /* 
   * Generate the stages of the prologue from last to first.  The stages
   * of the prologue are numbered 0 through stage_count-2.  The last
   * stage of the prologue should be a copy of kernel_copy[unroll - 1] with 
   * instructions from the last intra-iteration stage deleted.  Each 
   * preceding stage of the prologue is copied from the preceding 
   * kernel copy modulo unroll.   The number of stages in the prologue
   * may be larger than the degree of unrolling of the kernel, so as
   * the copy number is decremented, it may become negative, producing
   * and invalid copy number.  So, we start copying using a copy number 
   * which is at least as large as the number of stages in the prologue
   * and which, modulo unroll, is equal to unroll-1.
   *
   * There are fewer than stage_count stages in the prologue, so
   * (stage_count / unroll + 1) is an upper bound on the number 
   * of copies of the unrolled kernel needed to make the prologue (note that
   * stage_count does not have to be a multiple of unroll).
   * (stage_count / unroll + 1) * unroll is an upper bound on the number of
   * kernel stages that need to be copied.  This number modulo unroll is 0.
   * ((stage_count / unroll + 1) * unroll - 1) modulo unroll gives 
   * unroll - 1.  Thus, starting_copy modulo unroll gives the kernel 
   * copy needed for the last stage of the prologue.  The second to the 
   * last stage of the prologue is copied from 
   * kernel_copy[(starting_copy - 1) mod unroll)] and so on.
   * Since starting_copy is an upper bound on the number of kernel stages 
   * that need to be copied, i below will remain positive until all
   * stages of the prologue have been generated. 
   */

  starting_copy = (stage_count / unroll + 1) * unroll - 1;
  last_copy = starting_copy - (stage_count - 2);
  pro_stage = stage_count - 2;
  num_unrolled_kernel_iters = (stage_count - 1) / unroll;
  if (((stage_count - 1) % unroll) != 0)
    num_unrolled_kernel_iters++;

  unrolled_iter_num = num_unrolled_kernel_iters + 1;
  for (i = starting_copy; i >= last_copy; i--)
    {
      copy = i % unroll;
      first_oper = kernel_copy_first_op[copy];
      last_oper = kernel_copy_last_op[copy];
      if (copy == (unroll - 1))
	unrolled_iter_num--;
      for (oper = last_oper; oper != NULL; oper = oper->prev_op)
	{
	  if (!Lpipe_ignore_kernel_inst (oper))
	    {
	      /* will break out of this loop after copying first_oper */

	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      iter_stage = softpipe_info->stage;

	      /* At stage x of prologue, instructions from stage x+1 of the
	         pipeline (and later) are not executed (the first iteration 
	         has not progressed that far yet). */
	      if (iter_stage <= pro_stage)
		{
		  new_oper = L_copy_operation (oper);
		  new_oper->ext =
		    (Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
		  new_info = SOFTPIPE_OP_INFO (new_oper);
		  new_info->kernel_copy = copy;
		  new_info->prologue_stage = pro_stage;
		  /* issue time relative to beginning of prologue */
		  new_info->issue_time = pro_stage * ii +
		    softpipe_info->intra_iter_issue_time % ii;
		  new_info->unrolled_iter_num = unrolled_iter_num;
		  attr = L_new_attr ("unrolled_iter", 1);
		  L_set_int_attr_field (attr, 0, unrolled_iter_num);
		  new_oper->attr = L_concat_attr (new_oper->attr, attr);
		  L_insert_oper_before (prologue_cb, prologue_cb->first_op,
					new_oper);
#if 0
		  if (L_EXTRACT_BIT_VAL (new_oper->flags, L_OPER_SPECULATIVE))
		    {
		      attr = L_new_attr ("L_OPER_SPECULATIVE", 0);
		      new_oper->attr = L_concat_attr (new_oper->attr, attr);
		    }
#endif
		}
	      if (oper == first_oper)
		break;
	    }
	}
      pro_stage = pro_stage - 1;
    }

  /* adjust sync arcs after prologue generation */
  Lpipe_adjust_syncs_for_prologue ();

  /*
   * EPILOGUES FOR KERNEL SIDE EXITS
   * ----------------------------------------------------------------------
   */

  /* find fall through cb of original loop */
  fall_thru_flow = loop_back_flow->next_flow;

  /* delete flows corresponding to every branch in the original loop except
   * the loop back branch 
   */

  for (flow = header_cb->dest_flow; flow != loop_back_flow; flow = next_flow)
    {
      next_flow = flow->next_flow;
      dest_cb = flow->dst_cb;
      src_flow = L_find_matching_flow (dest_cb->src_flow, flow);
      dest_cb->src_flow = L_delete_flow (dest_cb->src_flow, src_flow);
      header_cb->dest_flow = L_delete_flow (header_cb->dest_flow, flow);
    }

  if (fall_thru_flow)
    {
      /* delete flow from header cb to fall through cb since all exits from
       * header will go to epilogues 
       */

      fall_thru_cb = fall_thru_flow->dst_cb;

      src_flow = L_find_matching_flow (fall_thru_cb->src_flow,
				       fall_thru_flow);

      fall_thru_cb->src_flow = L_delete_flow (fall_thru_cb->src_flow,
					      src_flow);

      header_cb->dest_flow = L_delete_flow (header_cb->dest_flow,
					    fall_thru_flow);
    }
  else
    {
      fall_thru_cb = NULL;
    }

  /* Generate multiple epilogues from the second to last branch in the
   * kernel to the first branch.  Generate the epilogues in this order
   * to make it easy to insert the flows into the header cb. 
   * Each new epilogue is inserted right after
   * the kernel, so the last epilogue generated is laid out the closest
   * to the kernel.  Generate the epilogue for the loop back
   * branch last because the kernel reaches this epilogue by falling
   * through the loop back branch. 
   */

  last_oper = header_cb->last_op;
  for (prev_oper = last_oper->prev_op, oper = prev_oper;
       oper != NULL; oper = prev_oper)
    {
      if (!Lpipe_ignore_kernel_inst (oper))
	{
	  softpipe_info = SOFTPIPE_OP_INFO (oper);
	  prev_oper = oper->prev_op;

	  if (L_cond_branch (oper))
	    {
	      if (softpipe_info->loop_back_br)
		{
		  if (!fall_thru_cb)
		    L_punt ("Lpipe_multi_epi_gen: cond branch loopback "
			    "with no FT flow");
		  if (M_arch != M_STARCORE)
		    L_negate_compare (oper);
		  else
		    M_starcore_negate_compare (oper);

		  L_change_branch_dest (oper,
					L_find_branch_dest (oper),
					fall_thru_cb);
		}
	      Lpipe_gen_epi (fn, header_cb, oper, fall_thru_cb,
			     TARGET_EPI, ii, unroll, stage_count);
	    }
	  else if (L_uncond_branch (oper))
	    {
	      if (softpipe_info->loop_back_br)
		{
		  /* if loop ends with unconditional jump, copies of this 
		   * branch always fall through. No epilogue is generated, 
		   * and the extraneous jump is removed.
		   */
		  kernel_copy_last_op[SOFTPIPE_OP_INFO (oper)->kernel_copy] =
		    prev_oper;
		  oper->ext = 0;
		  L_delete_oper (header_cb, oper);
		}
	      else
		L_punt ("Lpipe_multi_epi_gen: Unexpected uncond branch");
	    }
	}
    }

  /*
   * EPILOGUE FOR FALL-THROUGH PATH
   * ----------------------------------------------------------------------
   */

  if (L_cond_branch (last_oper))
    Lpipe_gen_epi (fn, header_cb, last_oper, fall_thru_cb,
		   FALL_THRU_EPI, ii, unroll, stage_count);

  /*
   * EPILOGUES FOR PROLOGUE EXITS
   * ----------------------------------------------------------------------
   * Prologue may be empty if stage_count == 1
   */

  last_oper = prologue_cb->last_op;
  if (last_oper == NULL)
    {
      if (stage_count > 1)
	L_punt ("Lpipe_multi_epi_gen: stage count is greater than 1, "
		"but prologue is empty.  cb = %d\n", header_cb->id);
    }
  else
    {
      /* Epilogue for last branch in prologue may be the same as for last 
         branch in kernel. */
      softpipe_info = SOFTPIPE_OP_INFO (last_oper);
      if (softpipe_info->loop_back_br)
	{
	  if (L_cond_branch (last_oper))
	    {
	      epilogue_cb = header_cb->next_cb;
	      /* change condition of branch so it is now taken when it used to 
	         fall thru and change target of branch */

	      L_negate_compare (last_oper);
	      L_change_branch_dest (last_oper, L_find_branch_dest (last_oper),
				    epilogue_cb);

	      /* prologue to epilogue flows */
	      new_flow = L_new_flow (1, prologue_cb, epilogue_cb,
				     UNKNOWN_WEIGHT);
	      prologue_cb->dest_flow =
		L_insert_flow_before (prologue_cb->dest_flow,
				      prologue_cb->dest_flow, new_flow);
	      new_flow = L_new_flow (1, prologue_cb, epilogue_cb,
				     UNKNOWN_WEIGHT);
	      epilogue_cb->src_flow =
		L_insert_flow_before (epilogue_cb->src_flow,
				      epilogue_cb->src_flow, new_flow);
	      last_oper = last_oper->prev_op;
	    }
	  else if (L_uncond_branch (last_oper))
	    {
	      oper->ext = 0;
	      L_delete_oper (prologue_cb, oper);
	      last_oper = prologue_cb->last_op;
	    }
	}

      /* Generate partial epilogues for other exits from prologue.  Go from
         last branch to first branch.  Place all partial epilogues at the
         end of the function.  */
      for (oper = last_oper; oper != NULL; oper = prev_oper)
	{
	  if (!Lpipe_ignore_kernel_inst (oper))
	    {
	      prev_oper = oper->prev_op;
	      softpipe_info = SOFTPIPE_OP_INFO (oper);

	      if (L_cond_branch (oper))
		{
		  if (softpipe_info->loop_back_br)
		    {
		      if (!fall_thru_cb)
			L_punt ("Lpipe_multi_epi_gen: cond branch loopback "
				"with no FT flow");
		      L_negate_compare (oper);
		      L_change_branch_dest (oper,
					    L_find_branch_dest (oper),
					    fall_thru_cb);
		    }
		  Lpipe_gen_partial_epi (fn, prologue_cb, oper, fall_thru_cb,
					 ii, unroll, stage_count);
		}
	      else if (L_uncond_branch (oper))
		{
		  if (softpipe_info->loop_back_br)
		    {
		      /* loop ends with unconditional jump, copies of this
		       * branch always fall through. No epilogue is 
		       * generated, and the extraneous jump is removed.
		       */
		      oper->ext = 0;
		      L_delete_oper (prologue_cb, oper);
		    }
		  else
		    L_punt ("Lpipe_multi_epi_gen: Unexpected uncond branch");
		}
	    }
	}
    }
  return;
}


static void
Lpipe_remove_comp_code_block (L_Cb * ssa_cb)
{
  L_Cb *exit_cb, *epi_cb;
  L_Flow *src_flow, *exit_flow, *epi_flow, *new_flow;

  exit_cb = ssa_cb->dest_flow->dst_cb;

  /* delete src flow for exit cb */
  exit_flow = L_find_matching_flow (exit_cb->src_flow, ssa_cb->dest_flow);
  exit_cb->src_flow = L_delete_flow (exit_cb->src_flow, exit_flow);

  /* Update dest flow for each epilogue that jumps to compensation code
     cb.  Add corresponding src flow to exit cb.  Change destination
     of jump in epilogue. */
  for (src_flow = ssa_cb->src_flow; src_flow != NULL;
       src_flow = src_flow->next_flow)
    {

      epi_cb = src_flow->src_cb;
      epi_flow = epi_cb->dest_flow;
      L_change_flow (epi_flow, 1, epi_cb, exit_cb, epi_flow->weight);
      new_flow = L_copy_single_flow (epi_flow);
      exit_cb->src_flow = L_insert_flow_before (exit_cb->src_flow,
						exit_cb->src_flow, new_flow);
      L_change_branch_dest (epi_cb->last_op,
			    L_find_branch_dest (epi_cb->last_op), exit_cb);
    }

  L_delete_cb (L_fn, ssa_cb);
}


static void
Lpipe_remove_epilogue (L_Func * fn, L_Oper * exit, L_Cb * pipe_cb,
		       L_Cb * epilogue_cb, L_Cb * fall_thru_cb, int epi_type)
{
  L_Flow *flow;
  int cc;

  if (epi_type == TARGET_EPI)
    {
      L_change_branch_dest (exit, L_find_branch_dest (exit), fall_thru_cb);
      flow = L_find_flow_for_branch (pipe_cb, exit);
      cc = 1;
    }
  else
    {
      flow = L_find_last_flow (pipe_cb->dest_flow);
      cc = 0;
    }

  L_change_flow (flow, cc, pipe_cb, fall_thru_cb, flow->weight);
  flow = L_find_flow_with_src_cb (fall_thru_cb->src_flow, epilogue_cb);
  L_change_flow (flow, cc, pipe_cb, fall_thru_cb, flow->weight);
  L_delete_cb (fn, epilogue_cb);
}


void
Lpipe_remove_empty_epilogues (L_Func * fn, L_Cb * header_cb)
{
  L_Oper *oper;
  L_Cb *epilogue_cb;
  L_Cb *fall_thru_cb = NULL;
  int empty_epilogue;
  L_Oper *last_oper;
  L_Cb *dest_cb;

  for (oper = prologue_cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_cond_branch (oper))
	{
	  epilogue_cb = L_find_branch_dest (oper);
	  if (!L_find_attr (epilogue_cb->attr, "epilogue"))
	    L_punt ("Lpipe_remove_empty_epilogues: Expected to find "
		    "epilogue attribute, but did not.  "
		    "Cb: %d, Function: %s\n", epilogue_cb->id, fn->name);

	  last_oper = epilogue_cb->last_op;
	  if (L_uncond_branch (last_oper))
	    {
	      dest_cb = L_find_branch_dest (last_oper);
	      if (L_find_attr (dest_cb->attr, "rename_comp_code"))
		{
		  if (dest_cb->first_op == NULL)
		    Lpipe_remove_comp_code_block (dest_cb);
		  else if (L_uncond_branch (dest_cb->first_op))
		    Lpipe_remove_comp_code_block (dest_cb);
		}
	    }

	  if (epilogue_cb->first_op == NULL)
	    {
	      empty_epilogue = 1;
	      fall_thru_cb = epilogue_cb->next_cb;
	    }
	  else if (L_uncond_branch (epilogue_cb->first_op))
	    {
	      empty_epilogue = 1;
	      fall_thru_cb = L_find_branch_dest (epilogue_cb->first_op);
	    }
	  else
	    {
	      empty_epilogue = 0;
	    }

	  if (empty_epilogue)
	    Lpipe_remove_epilogue (fn, oper, prologue_cb, epilogue_cb,
				   fall_thru_cb, TARGET_EPI);
	}
    }

  for (oper = header_cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_cond_branch (oper))
	{
	  if (oper == header_cb->last_op)
	    epilogue_cb = header_cb->next_cb;
	  else
	    epilogue_cb = L_find_branch_dest (oper);

	  if (!L_find_attr (epilogue_cb->attr, "epilogue"))
	    L_punt ("Lpipe_remove_empty_epilogues: Expected to find "
		    "epilogue attribute, but did not.  "
		    "Cb: %d, Function: %s\n", epilogue_cb->id, fn->name);

	  last_oper = epilogue_cb->last_op;
	  if (L_uncond_branch (last_oper))
	    {
	      dest_cb = L_find_branch_dest (last_oper);
	      if (L_find_attr (dest_cb->attr, "rename_comp_code"))
		{
		  if (dest_cb->first_op == NULL)
		    Lpipe_remove_comp_code_block (dest_cb);
		  else if (L_uncond_branch (dest_cb->first_op))
		    Lpipe_remove_comp_code_block (dest_cb);
		}

	      empty_epilogue = 0;
	      fall_thru_cb = NULL;
	      if (epilogue_cb->first_op == NULL)
		{
		  empty_epilogue = 1;
		  fall_thru_cb = epilogue_cb->next_cb;
		}
	      else if (L_uncond_branch (epilogue_cb->first_op))
		{
		  empty_epilogue = 1;
		  fall_thru_cb = L_find_branch_dest (epilogue_cb->first_op);
		}

	      if (empty_epilogue)
		{
		  if (oper != header_cb->last_op)
		    Lpipe_remove_epilogue (fn, oper, header_cb, epilogue_cb,
					   fall_thru_cb, TARGET_EPI);
		  /* can only remove epilogue for loop back branch if epilogue
		     falls through to original loop fall through path */
		  else if (fall_thru_cb == epilogue_cb->next_cb)
		    Lpipe_remove_epilogue (fn, oper, header_cb, epilogue_cb,
					   fall_thru_cb, FALL_THRU_EPI);
		}
	    }
	}
    }
}




/*------------------------------------------------------------------------
                     Muliple epilogue code schema
		     with rotating registers
  ------------------------------------------------------------------------*/

L_Oper *
Lpipe_gen_epilogue_counter (L_Func * fn, L_Cb * cb, int i, int top)
{
  L_Oper *epilogue_cntr_init = NULL, *new_op = NULL;
  L_Operand *epilogue_cntr_operand = NULL;

  /* If no specific register is returned from mspec, assume
     a virtual register can be used. */
  if (!(epilogue_cntr_operand = M_return_epilogue_cntr_register ()))
    epilogue_cntr_operand =
      L_new_register_operand (++(fn->max_reg_id),
			      M_native_int_register_ctype (), L_PTYPE_NULL);

  epilogue_cntr_init =
    Lpipe_gen_mov_consuming_operands (epilogue_cntr_operand,
				      L_new_gen_int_operand (i));

  if (top)
    L_insert_oper_before (cb, cb->first_op, epilogue_cntr_init);
  else				/* bottom */
    L_insert_oper_after (cb, cb->last_op, epilogue_cntr_init);

  L_annotate_oper (fn, cb, epilogue_cntr_init);
  new_op = epilogue_cntr_init->prev_op;	/* Annotate always insert op before. */
  L_delete_oper (cb, epilogue_cntr_init);

  return new_op;
}


L_Oper *
Lpipe_gen_loop_counter (L_Func * fn, L_Cb * cb, int i, int top)
{
  L_Oper *loop_cntr_init = NULL, *new_op = NULL;
  L_Operand *loop_cntr_operand = NULL;

  /* If no specific register is returned from mspec, assume
     a virtual register can be used. */
  if (!(loop_cntr_operand = M_return_loop_cntr_register ()))
    loop_cntr_operand = L_new_register_operand (++(fn->max_reg_id),
						M_native_int_register_ctype
						(), L_PTYPE_NULL);

  loop_cntr_init = Lpipe_gen_mov_consuming_operands (loop_cntr_operand,
						     L_new_gen_int_operand
						     (i));

  if (top)
    L_insert_oper_before (cb, cb->first_op, loop_cntr_init);
  else				/* bottom */
    L_insert_oper_after (cb, cb->last_op, loop_cntr_init);
  L_annotate_oper (fn, cb, loop_cntr_init);
  new_op = loop_cntr_init->prev_op;	/* Annotate always insert op before. */
  L_delete_oper (cb, loop_cntr_init);

  return new_op;
}


/* For the multiple epilogue code schema, the copies of the loop back
   branch are turned into exits by changing the branch direction. */
static void
Lpipe_change_branch_dir (L_Oper * branch, L_Cb * target)
{
  /* change ble to bgt, etc. */
  L_negate_compare (branch);

  /* new target is the epilogue associated with this exit */
  L_change_branch_dest (branch, L_find_branch_dest (branch), target);
}

/* Generate an epilogue for an exit branch from the prologue.  Place all
   partial epilogues at the end of the function. */
static void
Lpipe_gen_partial_epi_rot (L_Func * fn, L_Cb * header_cb, L_Oper * exit_branch,
		     L_Cb * fall_thru_cb, int ii, int stage_count)
{
  L_Cb *epilogue_cb;
  L_Cb *exit_cb;
  L_Flow *new_flow;
  Softpipe_Op_Info *exit_branch_info;
  Softpipe_Op_Info *softpipe_info;
  int pro_stage;		/* stage of prologue which contains exit br */
  int epi_stage;		/* current epilogue stage number */
  int iter_stage;		/* stage of iteration which contains oper */
  L_Oper *first_oper;
  L_Oper *last_oper;
  L_Oper *oper;
  L_Oper *new_oper;
  L_Attr *attr;
  int theta;			/* intra-iter stage containing exit branch */
  int do_copy;			/* flag indicating current oper should be
				   copied to epilogue */
  L_Oper *exit_branch_copy;	/* corresponding copy of prologue exit branch
				   in kernel */

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);

  /* Create epilogue cb and add it to control flow graph.  The 
     epilogue cbs for exits from the prologue are placed at the end
     of the function because it is assumed that most of the time the
     kernel will be entered. */
  epilogue_cb = L_create_cb (UNKNOWN_WEIGHT);
  L_insert_cb_after (fn, fn->last_cb, epilogue_cb);
  attr = L_new_attr ("epilogue", 0);
  epilogue_cb->attr = L_concat_attr (epilogue_cb->attr, attr);

  if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK))
    {
      epilogue_cb->flags =
	L_SET_BIT_FLAG (epilogue_cb->flags, L_CB_HYPERBLOCK);
    }

  /* prologue to epilogue flows */
  new_flow = L_new_flow (1, prologue_cb, epilogue_cb, UNKNOWN_WEIGHT);
  prologue_cb->dest_flow = L_insert_flow_before (prologue_cb->dest_flow,
						 prologue_cb->dest_flow,
						 new_flow);
  epilogue_cb->src_flow =
    L_new_flow (1, prologue_cb, epilogue_cb, UNKNOWN_WEIGHT);

  /* find exit cb */

  exit_cb = exit_branch_info->loop_back_br ?
    fall_thru_cb : L_find_branch_dest (exit_branch);

  /* epilogue to exit cb flows */
  epilogue_cb->dest_flow =
    L_new_flow (1, epilogue_cb, exit_cb, UNKNOWN_WEIGHT);
  new_flow = L_new_flow (1, epilogue_cb, exit_cb, UNKNOWN_WEIGHT);
  exit_cb->src_flow = L_insert_flow_before (exit_cb->src_flow,
					    exit_cb->src_flow, new_flow);

  /* If oper is a copy of the loop back branch, change condition of branch so
     it is now taken when it used to fall thru and change target of branch.
     Otherwise, just change branch target. */
  if (exit_branch_info->loop_back_br)
    {
      Lpipe_change_branch_dir (exit_branch, epilogue_cb);
    }
  else
    {
      L_change_branch_dest (exit_branch, L_find_branch_dest (exit_branch),
			    epilogue_cb);
    }

  /* Generate the stages of the epilogue from first to last.  The
     first stage of the epilogue is called stage 0.  Epilogue stage 0 is for
     the opers remaining in same kernel copy as the exit branch.  The number
     of stages in the epilogue is reduced by theta.  Start copying
     from the kernel copy which contains the exit branch and wrap around
     the kernel until all epilogue stages are generated. */
  theta = exit_branch_info->stage;
  pro_stage = exit_branch_info->prologue_stage;
  for (epi_stage = 0; epi_stage < stage_count - theta; epi_stage++)
    {
      first_oper = header_cb->first_op;
      last_oper = header_cb->last_op;;

      if (epi_stage == 0)
	{
	  exit_branch_copy =
	    Lpipe_find_exit_branch_copy (exit_branch, first_oper, last_oper);

	  /* if exit branch is the last oper in stage - i.e., a copy of the
	     loop back branch, there will be no opers in epilogue stage 0 */

	  if (exit_branch_copy == last_oper)
	    continue;
	  first_oper = exit_branch_copy->next_op;
	}

      for (oper = first_oper; oper != NULL; oper = oper->next_op)
	{
	  if (!Lpipe_ignore_kernel_inst (oper))
	    {
	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      iter_stage = softpipe_info->stage;

	      /* At stage x of epilogue, insts from stage x-1+theta of the 
	         final iteration are already executed.  Therefore only insts
	         from stage x+theta or later of the pipeline are included.
	         This effectively excludes instructions from iterations after
	         the final iteration since those iterations are executing in 
	         earlier stages.  If the oper is from the final iteration,
	         only copy if it was originally above the branch in the
	         original loop body. 

	         In the prologue, execution of the pipeline is ramping up so,
	         if an exit branch is taken, there are not as many iterations
	         that need to be finished up as in the kernel.  Therefore
	         don't copy instructions from any stage later than the
	         stage that the first iteration is currently executing.  For
	         an exit from prologue stage pro_stage and during epilogue
	         stage epi_stage, the first iteration is executing stage
	         pro_stage + epi_stage.  */

	      do_copy = 1;
	      if (iter_stage < (epi_stage + theta))
		{
		  do_copy = 0;
		}
	      if (iter_stage == (epi_stage + theta) &&
		  softpipe_info->home_block > exit_branch_info->home_block)
		{
		  do_copy = 0;
		}
	      if (iter_stage > (pro_stage + epi_stage))
		{
		  do_copy = 0;
		}

	      if (do_copy)
		{
		  L_punt ("Lpipe_gen_partial_epi_rot: Can't rotate all "
			  "registers because some might be invariant!\n");
#if 0
		  Softpipe_Op_Info *new_info;
		  int j;

		  new_oper = L_copy_operation (oper);
		  new_oper->ext =
		    (Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
		  new_info = SOFTPIPE_OP_INFO (new_oper);
		  /* issue time relative to beginning of epilogue */
		  new_info->issue_time = epi_stage * ii +
		    softpipe_info->intra_iter_issue_time % ii;
		  new_info->epilogue_stage = epi_stage;
		  L_insert_oper_after (epilogue_cb, epilogue_cb->last_op,
				       new_oper);

		  /* For epi stage 0, we use the same registers as in the
		     current kernel iteration.  When writing subsequent
		     epi stages, a rotation would have occurred, 
		     moving the contents of register x into x+1.  
		     So, we need to adjust the register values by -1 
		     for each rotation that would have occurred. */

		  for (j = 0; j < L_max_dest_operand; j++)
		    if (L_is_register (new_oper->dest[j]))
		      new_oper->dest[j]->value.r -= epi_stage;
		  for (j = 0; j < L_max_src_operand; j++)
		    if (L_is_register (new_oper->src[j]))
		      new_oper->src[j]->value.r -= epi_stage;
		  for (j = 0; j < L_max_pred_operand; j++)
		    if (L_is_register (new_oper->pred[j]))
		      new_oper->pred[j]->value.r -= epi_stage;
#endif
		}

	      if (oper == last_oper)
		break;
	    }
	}
    }
  /* add jump to fall through cb */
  if (epilogue_cb->next_cb != exit_cb)
    {
      new_oper = L_create_new_op (Lop_JUMP);
      new_oper->src[0] = L_new_cb_operand (exit_cb);
      L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper);
      L_annotate_oper (fn, epilogue_cb, new_oper);
      L_delete_oper (epilogue_cb, new_oper);
    }

  Lpipe_adjust_syncs_for_epilogue (epilogue_cb);
  return;
}


/* Generate an epilogue for an exit branch from the kernel. */
/* fall through path of original loop body */
/* flag indicating whether epilogue is the target
   of an exit branch, or on the fall through
   path of the kernel */
static void
Lpipe_gen_epi_rot (SM_Cb *sm_cb, L_Oper *exit_branch, L_Cb *fall_thru_cb,
		   int epi_type)
{
  int stage_count = sm_cb->stages;
  L_Cb *header_cb = sm_cb->lcode_cb, *epilogue_cb, *exit_cb;
  L_Func *fn = sm_cb->lcode_fn;
  double epi_weight;		/* weight of epilogue cb */
  L_Flow *new_flow;
  Softpipe_Op_Info *exit_branch_info, *softpipe_info;
  L_Oper *first_oper, *last_oper, *oper, *new_oper;
  L_Attr *attr;
  int epi_stage;
  int theta;			/* intra-iter stage containing exit branch */

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);

  /* Create epilogue cb and add it to control flow graph.  The last 
     epilogue cb created is placed the closest to the kernel cb. */

  epi_weight = exit_branch_info->exit_weight;
  epilogue_cb = L_create_cb (epi_weight);
  L_insert_cb_after (fn, header_cb, epilogue_cb);
  attr = L_new_attr ("epilogue", 0);
  epilogue_cb->attr = L_concat_attr (epilogue_cb->attr, attr);
  epilogue_cb->flags =
    L_SET_BIT_FLAG (epilogue_cb->flags, L_CB_ROT_REG_ALLOCATED);

  if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK))
    epilogue_cb->flags =
      L_SET_BIT_FLAG (epilogue_cb->flags, L_CB_HYPERBLOCK);

  {
    /* Splice the epilogue_cb into the CFG */

    int tk = (epi_type != FALL_THRU_EPI);
    
    new_flow = L_new_flow (tk, header_cb, epilogue_cb, epi_weight);  
    epilogue_cb->src_flow = L_copy_single_flow (new_flow);

    if (!tk)
      header_cb->dest_flow = L_concat_flow (header_cb->dest_flow, new_flow);
    else
      header_cb->dest_flow = L_insert_flow_before (header_cb->dest_flow,
						   header_cb->dest_flow,
						   new_flow);

    /* find exit cb */

    exit_cb = exit_branch_info->loop_back_br ? 
      fall_thru_cb : L_find_branch_dest (exit_branch);

    /* epilogue to exit cb flows */

    new_flow = L_new_flow ((epilogue_cb->next_cb != exit_cb), 
			   epilogue_cb, exit_cb, epi_weight);
    epilogue_cb->dest_flow = L_copy_single_flow (new_flow);

    exit_cb->src_flow = L_insert_flow_before (exit_cb->src_flow,
					      exit_cb->src_flow, new_flow);
  }

  /* If oper is a copy of the loop back branch, change condition of branch so
     it is now taken when it used to fall thru and change target of branch.
     Otherwise, just change target of branch. */

  if (epi_type == TARGET_EPI)
    {
      if (exit_branch_info->loop_back_br)
	Lpipe_change_branch_dir (exit_branch, epilogue_cb);
      else
	L_change_branch_dest (exit_branch, L_find_branch_dest (exit_branch),
			      epilogue_cb);
    }

  /* Generate the stages of the epilogue from first to last.  The 
     first stage of the epilogue is called stage 0.  Epilogue stage 0 is for
     the opers remaining in same kernel copy as the exit branch.  The number
     of stages in the epilogue is reduced by theta.  Start copying 
     from the kernel copy which contains the exit branch and wrap around 
     the kernel until all epilogue stages are generated. */

  theta = exit_branch_info->stage;

  for (epi_stage = 0; epi_stage < stage_count - theta; epi_stage++)
    {
      last_oper = header_cb->last_op;

      if (epi_stage == 0)
	{
	  /* if exit branch is the last oper in stage - i.e., a copy of the
	     loop back branch, there will be no opers in epilogue stage 0 */
	  if (exit_branch == last_oper)
	    continue;
	  first_oper = exit_branch->next_op;
	}
      else
	{
	  first_oper = header_cb->first_op;
	}

      for (oper = first_oper; oper; oper = oper->next_op)
	{
	  softpipe_info = SOFTPIPE_OP_INFO (oper);

	  if (Lpipe_ignore_kernel_inst (oper))
	    continue;

	  if (softpipe_info->stage <= (epi_stage + theta))
	    continue;

	  L_punt ("Lpipe_gen_epi_rot: Can't rotate all registers because "
		  "some might be invariant!\n");

#if 0
	  int ii = sm_cb->II, j, rotate;
	  Softpipe_Op_Info *new_info;
	  new_oper = L_copy_operation (oper);
	  new_oper->ext = (void *)Lpipe_copy_op_info (softpipe_info);
	  new_info = SOFTPIPE_OP_INFO (new_oper);
	  /* issue time relative to beginning of epilogue */
	  new_info->issue_time = epi_stage * ii +
	    softpipe_info->intra_iter_issue_time % ii;
	  new_info->epilogue_stage = epi_stage;
	  L_insert_oper_after (epilogue_cb, epilogue_cb->last_op,
			       new_oper);

	  /* For epi stage 0, we use the same registers as in the
	     current kernel iteration.  When writing subsequent
	     epi stages, a rotation would have occurred, 
	     moving the contents of register x into x+1.  
	     So, we need to adjust the register
	     values by -1 for each rotation that would have occurred.
	     
	     Actually, when we execute epi stage 0 from kernel,
	     we get an additional rotation at the exit of the loop
	     through the rotating register branch.  When we exit
	     from the prologue, then we are executing a non-rotating
	     branch and the registers are the same as in the kernel. */

	  rotate = (epi_type == TARGET_EPI) ? epi_stage : epi_stage + 1;

	  for (j = 0; j < L_max_dest_operand; j++)
	    if (L_is_register (new_oper->dest[j]))
	      new_oper->dest[j]->value.r -= rotate;

	  for (j = 0; j < L_max_src_operand; j++)
	    if (L_is_register (new_oper->src[j]))
	      new_oper->src[j]->value.r -= rotate;

	  for (j = 0; j < L_max_pred_operand; j++)
	    if (L_is_register (new_oper->pred[j]))
	      new_oper->pred[j]->value.r -= rotate;
#endif
	  if (oper == last_oper)
	    break;
	}
    }

  /* add jump to fall through cb */
  if (epilogue_cb->next_cb != exit_cb)
    {
      new_oper = L_create_new_op (Lop_JUMP);
      new_oper->src[0] = L_new_cb_operand (exit_cb);
      L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper);
      L_annotate_oper (fn, epilogue_cb, new_oper);
      L_delete_oper (epilogue_cb, new_oper);
    }

  Lpipe_adjust_syncs_for_epilogue (epilogue_cb);

  return;
}


/* generate multiple epilogues with rotating registers */
void
Lpipe_multi_epi_gen_rot (SM_Cb *sm_cb, L_Oper *loop_back_br)
{
  L_Func *fn = sm_cb->lcode_fn;
  L_Cb *kernel_cb = sm_cb->lcode_cb, *dest_cb, *fall_thru_cb;
  L_Flow *flow, *src_flow, *next_flow, *fall_thru_flow, *loop_back_flow;
  L_Oper *oper, *last_oper, *new_oper;
  Softpipe_Op_Info *softpipe_info, *new_info;
  int i, j, stage_count = sm_cb->stages, ii = sm_cb->II;
  int stg_ofst;

  /* find loop back flow */
  loop_back_flow = L_find_flow_with_dst_cb (kernel_cb->dest_flow, kernel_cb);

  /* create prologue cb */
  prologue_cb = L_create_cb (preheader_cb->dest_flow->weight);
  L_insert_cb_before (fn, kernel_cb, prologue_cb);

  prologue_cb->attr = L_concat_attr (prologue_cb->attr,
				     L_new_attr ("prologue", 0));
  prologue_cb->flags = L_SET_BIT_FLAG (prologue_cb->flags,
				       L_CB_ROT_REG_ALLOCATED);

  if (L_EXTRACT_BIT_VAL (kernel_cb->flags, L_CB_HYPERBLOCK))
    prologue_cb->flags = L_SET_BIT_FLAG (prologue_cb->flags, L_CB_HYPERBLOCK);

  /* preheader to prologue flows */
  if (preheader_cb->dest_flow->dst_cb != kernel_cb)
    L_punt ("Lpipe_multi_epi_gen: dest cb for preheader flow is not header:"
	    " preheader_cb is %d, kernel_cb is %d, dest of flow is %d\n",
	    preheader_cb->id, kernel_cb->id,
	    preheader_cb->dest_flow->dst_cb->id);

  preheader_cb->dest_flow->dst_cb = prologue_cb;
  prologue_cb->src_flow = L_new_flow (preheader_cb->dest_flow->cc,
				      preheader_cb, prologue_cb,
				      prologue_cb->weight);

  /* Prologue to kernel flows.  Assume only flows into kernel cb are
     from preheader and loop back branch */
  flow = kernel_cb->src_flow;
  if (flow->src_cb == kernel_cb)
    flow = flow->next_flow;
  flow->src_cb = prologue_cb;

  prologue_cb->dest_flow = L_new_flow (flow->cc, prologue_cb, kernel_cb,
				       prologue_cb->weight);

  /* Generate the stages of the prologue from last to first.  The
     stages of the prologue are numbered 0 through stage_count-2.  The
     last stage of the prologue should be a copy of kernel_copy[unroll
     - 1] with instructions from the last intra-iteration stage
     deleted.  Each preceding stage of the prologue is copied from the
     preceding kernel copy modulo unroll.  The number of stages in the
     prologue may be larger than the degree of unrolling of the
     kernel, so as the copy number is decremented, it may become
     negative, producing and invalid copy number.  So, we start
     copying using a copy number which is at least as large as the
     number of stages in the prologue and which, modulo unroll, is
     equal to unroll-1.

     There are less than stage_count stages in the prologue, so
     (stage_count / unroll + 1) is an upper bound on the number of
     copies of the unrolled kernel needed to make the prologue (note
     that stage_count does not have to be a multiple of unroll).
     (stage_count / unroll + 1) * unroll is an upper bound on the
     number of kernel stages that need to be copied.  This number
     modulo unroll is 0.  ((stage_count / unroll + 1) * unroll - 1)
     modulo unroll gives unroll - 1.  Thus, starting_copy modulo
     unroll gives the kernel copy needed for the last stage of the
     prologue.  The second to the last stage of the prologue is copied
     from kernel_copy[(starting_copy - 1) mod unroll)] and so on.
     Since starting_copy is an upper bound on the number of kernel
     stages that need to be copied, i below will remain positive until
     all stages of the prologue have been generated.

     This is actually much simpler.  There are stage_count - 1 kernel
     copies in the prologue.  Replicate the kernel, deleting
     instructions for stages not present in the particular prologue
     stage.

     Start generating from the LAST stage of the prologue which 
     is stage_count-2, then working toward stage 0. */

  for (i = stage_count - 2, stg_ofst = 1; i >= 0; i--, stg_ofst++)
    {
      for (oper = kernel_cb->last_op; oper; oper = oper->prev_op)
	{
	  Lpipe_MVEInfo *mve_info;
	  Lpipe_LRInfo *lr_info;

	  if (Lpipe_ignore_kernel_inst (oper))
	    continue;

	  softpipe_info = SOFTPIPE_OP_INFO (oper);

	  /* At stage x of prologue, instructions from stage x+1 of the
	     pipeline (and later) are not executed (the first 
	     iteration has not progressed that far yet). */
	  if (softpipe_info->stage > i)
	    continue;

	  new_oper = L_copy_operation (oper);
	  new_oper->ext =
	    (Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
	  new_info = SOFTPIPE_OP_INFO (new_oper);
	  new_info->prologue_stage = i;
	  /* issue time relative to beginning of prologue */
	  new_info->issue_time = i * ii +
	    softpipe_info->intra_iter_issue_time % ii;
	  L_insert_oper_before (prologue_cb, prologue_cb->first_op,
				new_oper);

	  /* Undo the effect of the register rotation between stages
	   * of the prologue.  At the boundary of the prologue to the
	   * kernel, each of the live registers in all active
	   * iterations have a unique location in the physical
	   * register file.  Rather than rotate, preserve these
	   * locations backward into the prologue. The last stage of
	   * the prologue (not the last stage of each iteration)
	   * should have the register IDs incremented by one to
	   * account for the shift that would have taken place. */

	  for (j = 0; j < L_max_dest_operand; j++)
	    {
	      if (!L_is_register (new_oper->dest[j]) ||
		  !(mve_info = softpipe_info->dest_mve_info[j]) ||
		  (mve_info->live_range->num_names <= 1))
		continue;
	      lr_info = mve_info->live_range;
	      new_oper->dest[j]->value.r = 
		Lpipe_get_pro_reg (sm_cb, lr_info, mve_info, stg_ofst);
	    }
	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      if (!L_is_register (new_oper->src[j]) ||
		  !(mve_info = softpipe_info->src_mve_info[j]) ||
		  (mve_info->live_range->num_names <= 1))
		continue;		  
	      lr_info = mve_info->live_range;
	      new_oper->src[j]->value.r = 
		Lpipe_get_pro_reg (sm_cb, lr_info, mve_info, stg_ofst);
	    }
	  for (j = 0; j < L_max_pred_operand; j++)
	    {
	      if (!L_is_register (new_oper->pred[j]) ||
		  !(mve_info = softpipe_info->pred_mve_info[j]) ||
		  (mve_info->live_range->num_names <= 1))
		continue;
	      lr_info = mve_info->live_range;
	      new_oper->pred[j]->value.r = 
		Lpipe_get_pro_reg (sm_cb, lr_info, mve_info, stg_ofst);
	    }
	}
    }

  /* adjust sync arcs after prologue generation */
  Lpipe_adjust_syncs_for_prologue ();

  /* generate mutliple epilogues */

  /* find fall through flow of original loop.  This may be NULL. */
  fall_thru_flow = loop_back_flow->next_flow;

  /* delete flows corresponding to every branch in the original loop except
     the loop back branch */
  for (flow = kernel_cb->dest_flow; flow != loop_back_flow; flow = next_flow)
    {
      if (flow == fall_thru_flow)
	L_punt ("Lpipe_multi_epi_gen_rot: Unlinking flows:"
		"tried to unlink the fall-through flow\n");
      next_flow = flow->next_flow;
      dest_cb = flow->dst_cb;
      src_flow = L_find_matching_flow (dest_cb->src_flow, flow);
      dest_cb->src_flow = L_delete_flow (dest_cb->src_flow, src_flow);
      kernel_cb->dest_flow = L_delete_flow (kernel_cb->dest_flow, flow);
    }

  /* delete flow from header cb to fall through cb since all exits from
     header will go to epilogues. There may not be a fall through flow/cb. */
  if (fall_thru_flow)
    {
      fall_thru_cb = fall_thru_flow->dst_cb;
      src_flow =
	L_find_matching_flow (fall_thru_cb->src_flow, fall_thru_flow);
      fall_thru_cb->src_flow =
	L_delete_flow (fall_thru_cb->src_flow, src_flow);
      kernel_cb->dest_flow =
	L_delete_flow (kernel_cb->dest_flow, fall_thru_flow);
    }
  else
    {
      fall_thru_cb = NULL;
    }

  /* Generate multiple epilogues from the second to last branch in the
     kernel to the first branch.  Generate the epilogues in this order
     to make it easy to insert the flows into the header cb. 
     Each new epilogue is inserted right after
     the kernel, so the last epilogue generated is laid out the closest
     to the kernel.  Generate the epilogue for the loop back
     branch last because the kernel reaches this epilogue by falling
     through the loop back branch. */

  last_oper = kernel_cb->last_op;
  for (oper = last_oper->prev_op; oper; oper = oper->prev_op)
    {
      if (oper == loop_back_br)
	L_punt ("Encountered loop back branch in epi gen!");
      else if (L_cond_branch (oper))
	Lpipe_gen_epi_rot (sm_cb, oper, fall_thru_cb, TARGET_EPI);
      else if (L_uncond_branch (oper))
	L_punt ("MCM Should never get here.");
    }

  /* generate the epilogue for the fall through path of the loop
     back branch if it is a conditional branch */

  if (L_cond_branch (last_oper))
    Lpipe_gen_epi_rot (sm_cb, last_oper, fall_thru_cb, FALL_THRU_EPI);

  /* Generate epilogues for exits from the prologue.  Prologue may be 
     empty if stage_count = 1. */

  if ((last_oper = prologue_cb->last_op))
    {
      /* Generate partial epilogues for other exits from prologue.  Go from
         last branch to first branch.  Place all partial epilogues at the
         end of the function.  */
      for (oper = last_oper; oper; oper = oper->prev_op)
	{
	  if (L_cond_branch (oper))
	    Lpipe_gen_partial_epi_rot (fn, kernel_cb, oper, fall_thru_cb, 
				       ii, stage_count);
	  else if (L_uncond_branch (oper))
	    L_punt ("MCM Should never get here.");
	}
    }
  else if (stage_count > 1)
    {
      L_punt ("Lpipe_multi_epi_gen: stage count is greater than 1, "
	      "but prologue is empty.  cb = %d\n", kernel_cb->id);
    }


  {
    L_Oper *epilogue_cntr_init;
    L_Operand *epilogue_cntr_operand;

    epilogue_cntr_init = Lpipe_gen_epilogue_counter (fn, prologue_cb, 1, 0);
    epilogue_cntr_operand = epilogue_cntr_init->dest[0];

#if 0
    /* Alter the loop back branch to branch when the epilogue counter is
       non-zero or follow its condition when the epilogue counter is 0.
       This branch must also perform register rotation and decrement
       the epilogue counter when it is not zero. */

    if (!(isl_attr = L_find_attr (loop_back_br->attr, "isl")))
      L_punt ("Lpipe_multi_epi_gen_rot: Unable to find the isl for the loop"
	      "back branch.");

    loop_back_br->src[3] = L_copy_operand (epilogue_cntr_operand);
    loop_back_br->dest[0] = L_copy_operand (epilogue_cntr_operand);

    /* Set the latency on dest[0] to 1...automatically creates the field. */
    L_set_int_attr_field (isl_attr, 2, 1);
#endif
  }

  loop_back_br->flags =
    L_SET_BIT_FLAG (loop_back_br->flags, L_OPER_ROTATE_REGISTERS);

  /* 20031119 SZU */
  if (M_arch == M_TAHOE)
    {
      if (loop_back_br->proc_opc != TAHOEop_BR_CLOOP)
	loop_back_br->proc_opc = TAHOEop_BR_WTOP;
      else
	loop_back_br->proc_opc = TAHOEop_BR_CTOP;
    }

  return;
}


/* generate multiple epilogues */
void
Lpipe_kernel_gen_rot (SM_Cb *sm_cb, L_Oper *loop_back_br)
{
  L_Func *fn = sm_cb->lcode_fn;
  int stage_count = sm_cb->stages;
  int int_base, int_num, flt_base, flt_num, dbl_base, dbl_num, pred_base,
    pred_num;
  L_Operand *epilogue_cntr_operand = NULL;
  L_Operand *pred = NULL;
  L_Oper *epilogue_cntr_init = NULL;
  L_Oper *new_op, *oper = NULL, *last_oper;
  L_Attr *isl_attr = NULL, *attr = NULL;
  int loop;
  int loop_back_stage;
  Softpipe_Op_Info *info = NULL;
  L_Cb *prologue_cb = NULL, *dest_cb = NULL, *fall_thru_cb = NULL;
  L_Flow *flow = NULL, *next_flow = NULL, *src_flow = NULL;
  L_Flow *fall_thru_flow;	/* flow for fall thru path of loop back br  */
  L_Flow *loop_back_flow;	/* flow corresponding to taken loop back br */

  /* find loop back flow */
  loop_back_flow = L_find_flow_with_dst_cb (header_cb->dest_flow, header_cb);

  /* create prologue cb */
  prologue_cb = L_create_cb (preheader_cb->dest_flow->weight);
  L_insert_cb_before (fn, header_cb, prologue_cb);
  attr = L_new_attr ("prologue", 0);
  prologue_cb->attr = L_concat_attr (prologue_cb->attr, attr);
  prologue_cb->flags = L_SET_BIT_FLAG (prologue_cb->flags,
				       L_CB_ROT_REG_ALLOCATED);

  if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK))
    prologue_cb->flags = L_SET_BIT_FLAG (prologue_cb->flags, L_CB_HYPERBLOCK);

  /* preheader to prologue flows */
  if (preheader_cb->dest_flow->dst_cb != header_cb)
    L_punt ("Lpipe_multi_epi_gen: dest cb for preheader flow is not header:"
	    " preheader_cb is %d, header_cb is %d, dest of flow is %d\n",
	    preheader_cb->id, header_cb->id,
	    preheader_cb->dest_flow->dst_cb->id);

  preheader_cb->dest_flow->dst_cb = prologue_cb;
  prologue_cb->src_flow = L_new_flow (preheader_cb->dest_flow->cc,
				      preheader_cb, prologue_cb,
				      prologue_cb->weight);

  /* Prologue to header flows.  Assume only flows into header cb are
     from preheader and loop back branch */
  flow = header_cb->src_flow;
  if (flow->src_cb == header_cb)
    flow = flow->next_flow;
  flow->src_cb = prologue_cb;

  prologue_cb->dest_flow = L_new_flow (flow->cc, prologue_cb, header_cb,
				       prologue_cb->weight);

  /* Add epilogue counter to prologue as the number of stages - 1. */
  if (prologue_cb->dest_flow->dst_cb != header_cb)
    L_punt ("Lpipe_kernel_gen_rot: dest cb for prologuer flow is not header:"
	    " prologue_cb is %d, header_cb is %d, dest of flow is %d\n",
	    prologue_cb->id, header_cb->id,
	    prologue_cb->dest_flow->dst_cb->id);

#if 0
  epilogue_cntr_init = Lpipe_gen_epilogue_counter (fn, prologue_cb,
						   stage_count - 1, 0);
#else
  epilogue_cntr_init = Lpipe_gen_epilogue_counter (fn, prologue_cb,
						   stage_count, 0);
#endif

  epilogue_cntr_operand = epilogue_cntr_init->dest[0];

  /* Alter the loop back branch to branch when the epilogue counter is
     non-zero or follow its condition when the epilogue counter is 0.
     This branch must also perform register rotation and decrement
     the epilogue counter when it is not zero. */

  if ((isl_attr = L_find_attr (loop_back_br->attr, "isl")) == NULL)
    L_punt ("Lpipe_kernel_gen_rot: Unable to find the isl for the"
	    "loop back branch.");

  loop_back_br->flags =
    L_SET_BIT_FLAG (loop_back_br->flags, L_OPER_ROTATE_REGISTERS);
  loop_back_br->src[3] = L_copy_operand (epilogue_cntr_operand);
  loop_back_br->dest[0] = L_copy_operand (epilogue_cntr_operand);
  /* Set the latency on dest[0] to 1...automatically creates the field. */
  L_set_int_attr_field (isl_attr, 2, 1);
  
  if (!(info = SOFTPIPE_OP_INFO (loop_back_br)))
    L_punt ("Lpip_kernel_gen_rot: no SOFTPIPE_OP_INFO found for loop"
	    "back branch.");
  loop_back_stage = info->stage;

  /* Assign the various instructions the appropriate predicate based
     on their stage.  For already predicated instructions, nothing
     additional should be needed. */

  R_get_rot_regs (L_fn, &int_base, &int_num, &flt_base, &flt_num,
		  &dbl_base, &dbl_num, &pred_base, &pred_num);

  /* Predicate all non-speculative stages.  The loop back comparison
     (pred define) must be made conditional so that if its incoming 
     predicate is 0, then no changes are made to the target predicates. */
  pred = NULL;
  for (oper = header_cb->last_op; oper != NULL; oper = oper->prev_op)
    {
      if (Lpipe_ignore_kernel_inst (oper))
	continue;

      if (!(info = SOFTPIPE_OP_INFO (oper)))
	L_punt ("Lpipe_kernel_gen_rot: "
		"no SOFTPIPE_OP_INFO found for loop back branch.");

#if 0
      /* Non-speculative stages begin with the stage that contains the 
	 loop back branch and continue to the end of the iteration. */
      if (info->stage >= loop_back_stage)
	{
	  if (oper == loop_back_br)
	    {
	      /* The loop back branch gets the predicate for the last
		 speculative stage, while its comparision will be in the
		 first non-speculative stage. */
	      
	      /* Save the loop back controlling predicate. */
	      if (oper->pred[0])
		pred = oper->pred[0];
	      else
		L_punt ("Lpipe_kernel_gen_rot: loop back without a "
			"qualifying predicate.");
	      
	      oper->pred[0] =
		L_new_register_operand (pred_base + info->stage - 1,
					L_CTYPE_PREDICATE, L_PTYPE_NULL);
	    }
	  else
	    {
	      if (!oper->pred[0])
		oper->pred[0] =
		  L_new_register_operand (pred_base + info->stage,
					  L_CTYPE_PREDICATE,
					  L_PTYPE_NULL);

	      if (L_same_operand (oper->dest[0], pred))
		{
		  /* This is the loop back condition 
		     generating comparison. */
		  if (oper->dest[0]->ptype != L_PTYPE_UNCOND_T &&
		      oper->dest[0]->ptype != L_PTYPE_UNCOND_F)
		    L_punt ("Lpipe_kernel_gen_rot: "
				"loop back condition not uncondition.");
		  
		  if (L_is_ptype_uncond_t (oper->dest[0]))
		    L_assign_ptype_cond_t (oper->dest[0]);
		}
	      else if (L_same_operand (oper->dest[1], pred))
		{
		  /* This is the loop back condition 
		     generating comparison. */
		  if (oper->dest[1]->ptype != L_PTYPE_UNCOND_T &&
		      oper->dest[1]->ptype != L_PTYPE_UNCOND_F)
		    {
		      L_punt ("Lpipe_kernel_gen_rot: "
			      "loop back condition not uncondition.");
		    }
		  
		  if (L_is_ptype_uncond_t (oper->dest[1]))
		    L_assign_ptype_cond_t (oper->dest[1]);
		}
	    }
	}
#else
      if (!oper->pred[0] &&
	  !(oper == loop_back_br &&
	    (oper->opc == Lop_JUMP || oper->opc == Lop_JUMP_FS)))
	{
	  oper->pred[0] =
	    L_new_register_operand (pred_base + info->stage,
				    L_CTYPE_PREDICATE, L_PTYPE_NULL);
	}
#endif
    }
  
#if 0
  L_delete_operand (pred);
#endif

  /* Need to make sure that the stage predicates begin clear,
     except for the first predicate, which should be set. 
     Clear the other predicate registers since it is possible
     that they might be read (guarding an op) where it is never
     modified (destination of a staged compare). */

  pred = L_new_register_operand (pred_base, L_CTYPE_PREDICATE, L_PTYPE_NULL);
  new_op = Lpipe_gen_uncond_pred_define (NULL, NULL, pred, Lop_PRED_SET);
  L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_op);
  L_annotate_oper (fn, prologue_cb, new_op);
  L_delete_oper (prologue_cb, new_op);

  for (loop = 1; loop < pred_num; loop++)
    {
      pred = L_new_register_operand (pred_base + loop,
				     L_CTYPE_PREDICATE, L_PTYPE_NULL);
      new_op =
	Lpipe_gen_uncond_pred_define (NULL, NULL, pred, Lop_PRED_CLEAR);
      L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_op);
    }

  /* Need to shift a 1 into the stage predicate stream when the
     loop back branch takes. */
  loop_back_br->dest[1] = L_new_register_operand (pred_base,
						  L_CTYPE_PREDICATE,
						  L_PTYPE_UNCOND_T);
  /* Set the latency on dest[1] to 1...automatically creates the field. */
  L_set_int_attr_field (isl_attr, 3, 1);

  /* Set the kernel hyperblock flag. */
  header_cb->flags = L_SET_BIT_FLAG (header_cb->flags, L_CB_HYPERBLOCK);

  /* Set the preheader hyperblock flag. */
  prologue_cb->flags = L_SET_BIT_FLAG (prologue_cb->flags, L_CB_HYPERBLOCK);

  /* generate mutliple epilogues */

  /* find fall through flow of original loop.  This may be NULL. */
  fall_thru_flow = loop_back_flow->next_flow;

  /* delete flows corresponding to every branch in the original loop except
     the loop back branch */
  for (flow = header_cb->dest_flow; flow != loop_back_flow; flow = next_flow)
    {
      if (flow == fall_thru_flow)
	L_punt ("Lpipe_multi_epi_gen_rot: Unlinking flows:"
		"tried to unlink the fall-through flow\n");
      next_flow = flow->next_flow;
      dest_cb = flow->dst_cb;
      src_flow = L_find_matching_flow (dest_cb->src_flow, flow);
      dest_cb->src_flow = L_delete_flow (dest_cb->src_flow, src_flow);
      header_cb->dest_flow = L_delete_flow (header_cb->dest_flow, flow);
    }

  /* delete flow from header cb to fall through cb since all exits from
     header will go to epilogues. There may not be a fall through flow/cb. */
  if (fall_thru_flow != NULL)
    {
      fall_thru_cb = fall_thru_flow->dst_cb;
      src_flow =
	L_find_matching_flow (fall_thru_cb->src_flow, fall_thru_flow);
      fall_thru_cb->src_flow =
	L_delete_flow (fall_thru_cb->src_flow, src_flow);
      header_cb->dest_flow =
	L_delete_flow (header_cb->dest_flow, fall_thru_flow);
    }
  else
    {
      fall_thru_cb = NULL;
    }

  /* Generate multiple epilogues from the second to last branch in the
     kernel to the first branch.  Generate the epilogues in this order
     to make it easy to insert the flows into the header cb. 
     Each new epilogue is inserted right after
     the kernel, so the last epilogue generated is laid out the closest
     to the kernel.  Generate the epilogue for the loop back
     branch last because the kernel reaches this epilogue by falling
     through the loop back branch. */

  last_oper = header_cb->last_op;
  for (oper = last_oper->prev_op; oper; oper = oper->prev_op)
    {
      if (L_cond_branch (oper))
	{
	  Lpipe_gen_epi_rot (sm_cb, oper, fall_thru_cb, TARGET_EPI);
	}
      else if (L_uncond_branch (oper))
	{
	  L_punt ("MCM Should never get here.");
	  /* if loop ends with unconditional jump, copies of this branch
	     always fall through. No epilogue is generated.  */
	  oper->ext = 0;
	  L_delete_oper (header_cb, oper);
	}
    }

  /* generate the epilogue for the fall through path of the loop
     back branch if it is a conditional branch */
  if (L_cond_branch (last_oper))
    Lpipe_gen_epi_rot (sm_cb, last_oper, fall_thru_cb, FALL_THRU_EPI);

  return;
}
