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
 *      File: l_softpipe.c
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
#include "l_softpipe.h"
#include "l_pipe_sched.h"
#include "l_pipe_sync.h"
#include "l_pipe_rename.h"
#include "l_mve.h"
#include <Lcode/r_regalloc.h>
#include <Lcode/l_opti.h>
#include <Lcode/l_disjvreg.h>

/*************************************************************************
                Defines
*************************************************************************/

#define MAX_TRIES 100		/* Upper bound on the of candidate
				   II's to try before giving up */

/*************************************************************************
                Global Variables
*************************************************************************/

/* global variables - see l_softpipe.h */

L_Cb *header_cb;
L_Cb *preheader_cb;
L_Cb *prologue_cb;
L_Cb *remainder_cb;
int Lpipe_stage_count;
int Lpipe_counted_loop = 0;

L_Oper **kernel_copy_last_op = 0;
L_Oper **kernel_copy_first_op = 0;

int loop_dep_height = -1;
int Lpipe_total_issue_slots = -1;

/*************************************************************************
                Global FILE Variables
*************************************************************************/

/* Scheduled code output */
FILE *sched_file = NULL;
/* file to which to print stats when checking if loop can be
   software pipelined */
FILE *pipe_statfile = NULL;
/* file to which to print pipelining stats */
FILE *gen_statfile = NULL;
/* file to which to print average iters for each loop before pipelining */
FILE *iters_statfile = NULL;
/* file to which to print loop register pressure stats */
FILE *reg_statfile = NULL;
/* file to which to print func register pressure stats */
FILE *func_reg_statfile = NULL;
/* file to which to print loop spill and code size stats */
FILE *spill_statfile = NULL;
/* file to which to print function code size stats */
FILE *code_statfile = NULL;
/* file to which to print swp function code size stats */
FILE *swp_code_statfile = NULL;
/* file to which to print stats for acyclic scheduling */
FILE *acyclic_statfile = NULL;

/*************************************************************************
                Function Definitions
*************************************************************************/


/*************************************************************************
                 Interface to Code Generator
*************************************************************************/

/* Once per file initialization of memory pools and other software
   pipelining info.  Called by code generator during phase 2 initialization.
   Assumes Lsched_init has been called to perform mdes and dependence 
   initialization.  The structures allocated in this function in this
   function are never freed.  They are allocated only once and then die
   when all functions in the file have been processed. */

void
Lpipe_init (Parm_Macro_List * command_line_macro_list)
{
  /* initialize default code generation schema */
  Lpipe_schema_name = strdup ("multiple_epilogues");

  /* Load the software pipelining parameters */
  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Lmarkpipe", L_read_parm_lmarkpipe);

  /* Renamed 'softpipe' to 'Lsoftpipe' -HCH 11/04/00 */
  L_load_parameters_aliased (L_parm_file, command_line_macro_list,
			     "(Lsoftpipe", "(Softpipe", L_read_parm_lpipe);

  /* Create memory pools for allocation of data structures */
  Lpipe_src_mve_pool = L_create_alloc_pool ("src_mve_info",
					    L_max_src_operand *
					    sizeof (Lpipe_MVEInfo *), 50);
  Lpipe_pred_mve_pool =
    L_create_alloc_pool ("pred_mve_info",
			 L_max_pred_operand * sizeof (Lpipe_MVEInfo *), 50);
  Lpipe_dest_mve_pool =
    L_create_alloc_pool ("dest_mve_info",
			 L_max_dest_operand * sizeof (Lpipe_MVEInfo *), 50);
  Lpipe_MVEInfo_pool =
    L_create_alloc_pool ("Lpipe_MVEInfo", sizeof (Lpipe_MVEInfo), 50);

  Softpipe_Op_Info_pool =
    L_create_alloc_pool ("Softpipe_Op_Info", sizeof (Softpipe_Op_Info), 50);

  Qnode_pool = L_create_alloc_pool ("Qnode", sizeof (Qnode), 50);
  Queue_pool = L_create_alloc_pool ("Queue", sizeof (Queue), 3);

  if (Lpipe_print_iteration_schedule || Lpipe_print_schedules_for_debug)
    {
      char *sched_file_name;

      /* open one .loop_sched file per input file */
      sched_file_name = (char *) malloc (strlen (L_output_file) + 12);
      if (sched_file_name == NULL)
	L_punt ("Lpipe_init: malloc out of space for sched_file_name\n");

      strcpy (sched_file_name, L_output_file);
      strcat (sched_file_name, ".loop_sched");
      sched_file = fopen (sched_file_name, "w");
      if (sched_file == 0)
	L_punt ("Error while processing file %s: can not open"
		"file %s\n", L_input_file, sched_file_name);

      Lcode_free (sched_file_name);
    }
  else
    sched_file = stdout;

  reg_statfile = stdout;
  func_reg_statfile = stdout;
  spill_statfile = stdout;
  code_statfile = stdout;
  swp_code_statfile = stdout;
  acyclic_statfile = stdout;

  if (Lpipe_print_statistics)
    {
      pipe_statfile = fopen ("mark_ph2.stats", "a");
      if (pipe_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"mark_ph2.stats" "\n", L_input_file);
      gen_statfile = fopen ("softpipe.stats", "a");
      if (gen_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"softpipe.stats" "\n", L_input_file);
      iters_statfile = fopen ("iters.stats", "a");
      if (iters_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"iters.stats" "\n", L_input_file);
    }
  else
    {
      pipe_statfile = stdout;
      gen_statfile = stdout;
      iters_statfile = stdout;
    }

  Lpipe_measurement_init (command_line_macro_list);
}


void
Lpipe_measurement_init (Parm_Macro_List * command_line_macro_list)
{
  if (Lpipe_compute_loop_reg_pressure)
    {
      reg_statfile = fopen ("register.stats", "a");
      if (reg_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"register.stats" "\n", L_input_file);
      func_reg_statfile = fopen ("func_reg.stats", "a");
      if (func_reg_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"func_reg.stats" "\n", L_input_file);
    }

  if (Lpipe_add_spill_attributes)
    {
      spill_statfile = fopen ("spillandsize.stats", "a");
      if (spill_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"spillandsize.stats" "\n", L_input_file);
      code_statfile = fopen ("code_size.stats", "a");
      if (code_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"code_size.stats" "\n", L_input_file);
      swp_code_statfile = fopen ("swp_code_size.stats", "a");
      if (swp_code_statfile == 0)
	L_punt ("Error while processing file %s: cannot append to file "
		"swp_code_size.stats" "\n", L_input_file);
    }
}


void
Lpipe_cleanup (void)
{
  if (Lpipe_print_iteration_schedule || Lpipe_print_schedules_for_debug)
    {
      fclose (sched_file);
    }

  if (Lpipe_print_statistics)
    {
      fclose (pipe_statfile);
      fclose (gen_statfile);
      fclose (iters_statfile);
    }

  Lpipe_measurement_cleanup ();
}


void
Lpipe_measurement_cleanup (void)
{
  if (Lpipe_compute_loop_reg_pressure)
    {
      fclose (reg_statfile);
      fclose (func_reg_statfile);
    }
  if (Lpipe_add_spill_attributes)
    {
      fclose (spill_statfile);
      fclose (code_statfile);
      fclose (swp_code_statfile);
    }
}


static void
Lpipe_failure (SM_Cb *sm_cb, Softpipe_MinII *MinII)
{
  L_Attr *attr;
  L_Cb *cb = sm_cb->lcode_cb;

  /* If softpipe failed, then mark the II as -1 as an indicator. */
  attr = L_new_attr ("II", 1);
  L_set_int_attr_field (attr, 0, -1);
  cb->attr = L_concat_attr (cb->attr, attr);

  /* Clear the kernel flag and remove the SOFTPIPE flag
     from the kernel so it can be acyclic scheduled. */
  if ((attr = L_find_attr (cb->attr, "kernel")))
    cb->attr = L_delete_attr (cb->attr, attr);

  header_cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_SOFTPIPE);

  Lpipe_delete_start_stop_nodes (sm_cb);

  SM_delete_cb (sm_cb);
      
  Lcode_free (MinII);
  Lpipe_free_op_info (cb);

  return;
}

static void Lpipe_compute_start_stop_issue_time (SM_Cb * sm_cb);

/* make sure start operation is scheduled as late as possible and stop
   operation is scheduled as early as possible */
static void
Lpipe_compute_start_stop_issue_time (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  Softpipe_Op_Info *softpipe_info;
  int earliest_real_issue_time;	/* issue time of the earliest oper
				   which is not the start oper */
  int latest_real_issue_time;	/* issue time of the latest oper
				   which is not the stop oper */

  earliest_real_issue_time = SM_MAX_CYCLE;
  latest_real_issue_time = SM_MIN_CYCLE;

  /* find earliest issue time and latest issue time among all opers
     exlcuding the start and stop nodes */
  for (sm_op = sm_cb->first_serial_op; sm_op;
       sm_op = sm_op->next_serial_op)
    {
      softpipe_info = SOFTPIPE_OP_INFO (sm_op->lcode_op);
      if (L_START_STOP_NODE (sm_op->lcode_op))
	continue;
      if (sm_op->sched_cycle < earliest_real_issue_time)
	earliest_real_issue_time = sm_op->sched_cycle;
      if (sm_op->sched_cycle > latest_real_issue_time)
	latest_real_issue_time = sm_op->sched_cycle;
    }

  /* issue time of the start oper should be the earliest_real_issue_time */
  softpipe_info = SOFTPIPE_OP_INFO (sm_cb->first_serial_op->lcode_op);
  softpipe_info->intra_iter_issue_time = earliest_real_issue_time;

  /* issue time of the stop oper should be the latest_real_issue_time */
  softpipe_info = SOFTPIPE_OP_INFO (sm_cb->last_serial_op->lcode_op);
  softpipe_info->intra_iter_issue_time = latest_real_issue_time;
  return;
}


/* Requires oper wt */

static int
Lpipe_pipeline_loop (L_Func *fn, L_Inner_Loop *loop)
{
  L_Oper *loop_incr, *loop_back_br, *oper, *last_oper;
  SM_Cb *sm_cb;
  SM_Oper *sm_loop_incr, *sm_loop_back_br;
  L_Attr *attr;
  int num_oper, branch_count, ii, unroll, budget, tries, success;

  Softpipe_MinII *MinII;

  header_cb = loop->cb;

  header_cb->flags = L_SET_BIT_FLAG (header_cb->flags, L_CB_SOFTPIPE);

  /* find loop back branch */
  loop_back_br = loop->feedback_op;

  if (L_uncond_branch_opcode (header_cb->last_op) &&
      L_is_predicated (header_cb->last_op) &&
      L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
    {
      L_warn ("Promoting uncond br of nofallthru pipelined loop (cb %d)",
	      header_cb->id);
      L_delete_operand (header_cb->last_op->pred[0]);
      header_cb->last_op->pred[0] = NULL;
    }

  if (Lpipe_print_statistics)
    {
      fprintf (pipe_statfile, ":) Loop with header cb %d still OK "
	       "for software pipelining\n", header_cb->id);
      fprintf (gen_statfile, "\n");
      /* print source line number for loop if available and print cb id */
      if ((attr = L_find_attr (header_cb->attr, "LOOP")))
	fprintf (gen_statfile,
		 "Pipelining statistics for loop on line: " ITintmaxformat
		 "\n", attr->field[1]->value.i);
      else
	fprintf (gen_statfile,
		 "Pipelining_statistics for loop:\n");
      fprintf (gen_statfile, "Cb: %d\n", header_cb->id);
      fprintf (gen_statfile, "Function: %s\n", fn->name);
      fprintf (gen_statfile, "Average_iterations: %f\n",
	       loop->ave_iteration);
    }

  /* Find preheader */
  if (Lpipe_schema == REM_LOOP)
    {
      L_punt ("Lpipe_pipeline_loop: Remainder loops unsupported");
      /* for remainder loop, prologue cb was inserted between the
	 preheader and the header during loop prep */
      preheader_cb = header_cb->prev_cb->prev_cb;
    }
  else
    {
      preheader_cb = loop->preheader;
    }

  /* initialize prologue and remainder cb */
  prologue_cb = 0;
  remainder_cb = 0;

  /* print source line number and/or cb id for loop */
  if (Lpipe_debug >= 1)
    {
      attr = L_find_attr (header_cb->attr, "LOOP");
      if ((attr != NULL) &&
	  (!strcmp (attr->field[0]->value.s, "\"dosuper\"") ||
	   !strcmp (attr->field[0]->value.s, "\"doserial\"") ||
	   !strcmp (attr->field[0]->value.s, "\"while\"") ||
	   !strcmp (attr->field[0]->value.s, "\"for\"") ||
	   !strcmp (attr->field[0]->value.s, "\"do\"")))
	{
	  fprintf (stderr,
		   "Pipelining loop (function %s, line " ITintmaxformat
		   ", cb %d)\n", fn->name, attr->field[1]->value.i,
		   header_cb->id);
	}
      else
	{
	  fprintf (stderr, "Pipelining loop (function %s, cb %d)\n",
		   fn->name, header_cb->id);
	}

      if (Lpipe_debug >= 2)
	L_print_cb (stdout, fn, header_cb);
    }

  if (Lpipe_print_statistics)
    {
      fprintf (iters_statfile, "\n");
      attr = L_find_attr (header_cb->attr, "LOOP");
      /* print source line number for loop if available and print cb id */

      fprintf (iters_statfile,
	       "Pipelining_statistics_for_loop_on_line: " ITintmaxformat
	       "\n", attr ? attr->field[1]->value.i : 0);

      fprintf (iters_statfile, "Cb: %d\n", header_cb->id);
      fprintf (iters_statfile, "Function: %s\n", fn->name);
      fprintf (iters_statfile, "Average_iterations: %f\n",
	       loop->ave_iteration);
    }

  /* Construct the SM CB for MRT.  Anti- and output dependences are
   * constructed by SM, but ignored assuming that a subsequent run
   * of MVE will alleviate any broken dependences.
   */

  sm_cb = SM_new_cb (lmdes, header_cb,
		     SM_PREPASS | SM_MODULO | SM_NORMALIZE_ISSUE);

  /* Find induction op for iteration variable */
  loop_incr = L_find_inner_loop_counter (loop);

  /* Set up SM structures */
  sm_loop_back_br = SM_find_sm_op (sm_cb, loop_back_br);

  /* 20030107 SZU
   * Add to SM_Oper->flags that this is loop_back_br.
   * Was for bundling algorithm only; shouldn't cause problem.
   */
  sm_loop_back_br->flags |= SM_LOOP_BACK_BR;

  sm_loop_incr = SM_find_sm_op (sm_cb, loop_incr);
  /* MCM There is also a num_slots that could also be used. */
  Lpipe_total_issue_slots = sm_cb->version1_mdes->max_slot + 1;

  Lpipe_construct_op_info (sm_cb, sm_loop_back_br,
			   &num_oper, &branch_count);

  if (Lpipe_debug >= 1)
    Lpipe_print_cb_info (sm_cb);

  Lpipe_create_start_stop_nodes (sm_cb);
  Lpipe_mark_branch_path_opers (sm_cb, sm_loop_back_br);

  MinII = Lpipe_determine_mii (sm_cb, Lpipe_max_ii);
  ii = MinII->MinII;

  attr = L_new_attr ("softpipe_MinII_res_rec", 2);
  L_set_int_attr_field (attr, 0, MinII->res_MinII);
  L_set_int_attr_field (attr, 1, MinII->rec_MinII);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);

  if (MinII->MinII == -1)
    {
      L_warn ("Lpipe_pipeline_loop: "
	      "MinII > MAX_MinII %d, giving up.", Lpipe_max_ii);
      Lpipe_failure (sm_cb, MinII);
      return 1;
    }

  if (Lpipe_debug >= 1)
    printf ("Pipelining loop with res_MinII = %d, rec_MinII = %d, "
	    "eff_MinII = %f (function %s, cb %d)\n",
	    MinII->res_MinII, MinII->rec_MinII, MinII->eff_MinII,
	    sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);

  if (Lpipe_min_ii && Lpipe_min_ii > ii)
    {
      ii = Lpipe_min_ii;
      L_warn ("Lpipe_pipeline_loop: "
	      "Limiting to Lpipe_min_ii of %d\n", ii);
    }

  budget = (int) (Lpipe_budget_ratio * (float) (sm_cb->op_count - 2));

  /* Neutralize the self-dependences for scheduling purposes. 
     These deps are needed for ii calculation. */
  Lpipe_ignore_self_dependences (sm_cb);

  if (Lpipe_debug >= 2)
    SM_print_cb_dependences (stdout, sm_cb);

  /* Attempt to schedule at candidate IIs until successful, or until
     II is too large or too many candidate IIs have been tried. */
  tries = 1;

  while (!(success = Lpipe_sm_schedule (sm_cb, ii, sm_loop_back_br, budget)))
    {
      if (tries >= MAX_TRIES)
	{
	  L_warn ("Lpipe_pipeline_loop: "
		  "too many tries (%d) on this loop, giving up.", MAX_TRIES);
	  break;
	}
      if (ii >= Lpipe_max_ii)
	{
	  L_warn ("Lpipe_pipeline_loop: "
		  "II > %d, giving up.", Lpipe_max_ii);
	  break;
	}

      ii++;
      tries++;

      /* Need to recompute MinDist for the new candidate II.  RecMinII is 
	 not changing, only the achieved II. */
      if (Lpipe_build_MinDist (sm_cb, ii))
	L_punt ("Failed recalculating MinDist");
    }

  if (!success)
    {
      /* Unable to schedule within constraints --- punting */
      Lpipe_failure (sm_cb, MinII);
      return 1;
    }

  /* CB Scheduled within constraints! */
  
  Lpipe_compute_start_stop_issue_time (sm_cb);
  Lpipe_delete_start_stop_nodes (sm_cb);

  Lpipe_stage_count = sm_cb->stages;

  Lpipe_sm_commit (sm_cb);

  /* MODULO VARIABLE RECTIFICATION */

  /* the number of speculatively executed stages is equal to the stage
     number in which the loop back branch is scheduled */

  if ((unroll = Lpipe_analyze_lr (sm_cb, 0)) == -1)
    {
      Lpipe_sm_terminate (sm_cb);
      Lpipe_failure (sm_cb, MinII);
      return 1;
    }

  if (Lpipe_debug >= 2)
    Lpipe_print_cb_schedule (stdout, fn, sm_cb);

  if (Lpipe_print_statistics)
    {
      Softpipe_Op_Info *softpipe_info = SOFTPIPE_OP_INFO (loop_back_br);
      int theta = softpipe_info->stage;
      int schedule_length = sm_cb->last_sched_op->sched_cycle -
	sm_cb->first_sched_op->sched_cycle + 1;

      Lpipe_print_cyclic_stats (gen_statfile, header_cb,
				MinII, ii, tries, schedule_length,
				loop_dep_height, Lpipe_stage_count,
				unroll, theta, num_oper, branch_count,
				loop);
    }

  /* From this point until the epilogues are generated and live out values
     and branches are fixed up, the Lcode is in an inconsistent state. 
     If printed during this time, it is almost surely not runnable
     or even compileable by other modules.  
     There is enough information in the Softpipe_Op_Info and MVE 
     data structures to now construct the software pipeline from the 
     original loop body. */

  /* adjust sync arcs for modulo scheduled loop body */
  Lpipe_adjust_syncs_for_modulo_sched (sm_cb);
  
  /* If stage count is greater than 1, intra iteration memory 
     dependences in the original loop can become cross-iteration
     dependences for the kernel.  Therefore the DOALL attribute
     no longer applies. */
  if (Lpipe_stage_count > 1)
    {
      attr = L_find_attr (header_cb->attr, "DOALL");
      header_cb->attr = L_delete_attr (header_cb->attr, attr);
    }

  /* Assign rotating register names and annotate the cb
     for register rotation, or unroll the loop and perform MVE.
     Lpipe_gen_rotating_regs only generates the rotating
     registers for the kernel.  The prologue is generated from
     the kernel in Lpipe_multi_epi_gen_rot.  MVE unrolls the loop
     but a later call to Lpipe_multi_epi_gen forms the prologue
     and epilogue from the MVE'ed kernel. */

  switch (Lpipe_schema)
    {
    case REM_LOOP:
      Lpipe_mve_transform (sm_cb, unroll);
      Lpipe_rem_loop_gen (fn, loop_incr, ii, unroll, Lpipe_stage_count);
      Lpipe_remove_branches (fn, unroll);
      break;
    case MULTI_EPI:
      Lpipe_mve_transform (sm_cb, unroll);
      Lpipe_multi_epi_gen (fn, Lpipe_stage_count, loop_back_br, ii, unroll);
      /* Fix up live in/out if kernel had to be unrolled for renaming.
	 Call these functions even if kernel was not unrolled, because
	 they also merge the SSA compensation code blocks into the 
	 epilogues. */
      Lpipe_fix_live_out (sm_cb, unroll);
#if 0
      /* Can't do this without working MS dataflow... comments indicate
       * it's not useful now anyway
       */
      Lpipe_fix_live_in (sm_cb);
#endif
      /* remove empty epilogues */
      Lpipe_remove_empty_epilogues (fn, header_cb);
      break;
    case MULTI_EPI_ROT_REG:
      Lpipe_rreg_transform (sm_cb);
      Lpipe_multi_epi_gen_rot (sm_cb, loop_back_br);
      Lpipe_fix_live_out_rot (sm_cb, sm_loop_back_br);
      Lpipe_fix_live_in_rot (sm_cb);
      break;
    case KERNEL_ONLY:
      /* Generate kernel-only code.  No epilogues are currently needed,
	 but some may in the future. */
      Lpipe_rreg_transform (sm_cb);
      Lpipe_kernel_gen_rot (sm_cb, loop_back_br);
      Lpipe_fix_live_out_rot (sm_cb, sm_loop_back_br);
      Lpipe_fix_live_in_rot (sm_cb);
      break;
    default:
      L_punt ("Lpipe_pipeline_loop: Unsupported schema type %d.", 
	      Lpipe_schema);
    }

  /* adjust sync arcs for the unrolling done by MVE */
  if (unroll > 1)
    L_adjust_syncs_after_unrolling (header_cb, unroll);

  /* Delete latency and mve info */
  Lpipe_free_mve_info ();
  Lcode_free (MinII);

  /* Free arrays of pointers to kernel copies */
  if (kernel_copy_first_op)
    {
      Lcode_free (kernel_copy_first_op);
      kernel_copy_first_op = NULL;
    }
  if (kernel_copy_last_op)
    {
      Lcode_free (kernel_copy_last_op);
      kernel_copy_last_op = NULL;
    }

  SM_delete_cb (sm_cb);

  /* Delete scheduling info for each oper */
  Lpipe_free_op_info (header_cb);

  if (Lpipe_stage_count > 1)
    {
      switch (Lpipe_schema)
	{
	case REM_LOOP:
	  Lpipe_free_op_info (prologue_cb);
          Lpipe_free_op_info (header_cb->next_cb);
          break;
	case MULTI_EPI:
	case MULTI_EPI_ROT_REG:
	  Lpipe_free_op_info (prologue_cb);
	  for (oper = prologue_cb->first_op; oper != NULL;
	       oper = oper->next_op)
	    {
	      if (L_cond_branch_opcode (oper))
		Lpipe_free_op_info (L_find_branch_dest (oper));
	    }
	  /* last exit from header cb is fall through and goes to the 
	     same epilogue as the last exit from prologue */
	  last_oper = header_cb->last_op;
	  for (oper = header_cb->first_op; oper != last_oper;
	       oper = oper->next_op)
	    {
	      if (L_cond_branch_opcode (oper))
		Lpipe_free_op_info (L_find_branch_dest (oper));
	    }
	  Lpipe_free_op_info (header_cb->next_cb);
	  break;
	default:
	  ;
	}
    }

  return 0;
}


/* Modulo schedules all loops in function.  Called by code generator during
   phase 2 just before prepass acyclic code scheduling. */
void
Lpipe_software_pipeline (L_Func * fn)
{
  L_Inner_Loop *loop;
  L_Attr *attr;

  if (Lpipe_do_only_postpass_steps)
    return;

  /*******************************************************************
   * Phase 1: Perform fn-level pred def conversion and global        *
   *          disjoint reg lifetime renaming.                        *
   *          Then perform dataflow and dead code elimination.       *
   ******************************************************************/

  /* Transform pred_clear / pred_xx o{t|f} pairs into 
     unconditional predicate defines where possible. */
  Lpipe_create_uncond_pred_defines (fn);

  /* To ensure maximum flexibility, break disjoint live
     ranges into distinct virtual registers. */
  {
    Set R_rot_reg = R_build_rotating_reg_set (fn);
    L_rename_disjoint_virtual_registers (fn, R_rot_reg);
    Set_dispose (R_rot_reg);
  }

  if (Lpipe_print_statistics)
    {
      fprintf (pipe_statfile,
	       "\n..Analyze loops for software pipelining in function (%s)\n",
	       fn->name);
    }

  L_partial_dead_code_removal (fn);
  L_demote_branches (fn);
  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE |
		      REACHING_DEFINITION | SUPPRESS_PG);

  L_compute_oper_weight (fn, 0, 1);

  /* Detect inner loops and find preheaders. We modulo schedule
     only a single cb, so look for marked header cb.  Full loop detection is
     not designed to be used after superblock formation and superscalar
     optimization, so don't use it here.  For remainder loop, preheader
     and prologue have already been set up, so don't create preheaders. */

  L_reset_loop_headers (fn);
  if (Lpipe_schema == REM_LOOP)
    {
      L_punt ("Lpipe_software_pipeline: Remainder loops unsupported");
      L_inner_loop_detection (fn, 0 /*Do not find preheaders */ );
    }
  else
    {
      L_inner_loop_detection (fn, 1 /*Find preheaders */ );
    }

  /*******************************************************************
   * Phase 2: Screen loops to softpipe through debugging bounds.     *
   *          Further screen loops through the Lmarkpipe checks.     *
   *          Finally split cbs after the loop back branch and       *
   *          run dataflow.                                          *
   ******************************************************************/

  for (loop = fn->first_inner_loop; loop; loop = loop->next_inner_loop)
    {
      header_cb = loop->cb;

      /* Check if loop is marked for software pipelining */
      if (!L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_SOFTPIPE))
	continue;

      /* Check for loops within target cb bounds */
      if (Lpipe_debug_use_cb_bounds &&
	  (header_cb->id < Lpipe_debug_lower_cb_bound ||
	   header_cb->id > Lpipe_debug_upper_cb_bound))
	{
	  header_cb->flags = L_CLR_BIT_FLAG (header_cb->flags, L_CB_SOFTPIPE);
	  continue;
	}

      /* Check to see if loop is still OK for software pipelining.  This
         is necessary because function calls and writes to macro registers
         can be added during phase 1 annotation. */

      if (!Lpipe_is_OK_softpipe_loop (loop, pipe_statfile, 
				      &Lpipe_counted_loop, 0))
	{
	  if (Lpipe_print_statistics)
	    fprintf (pipe_statfile, ":( Loop with header cb %d no "
		     "longer OK for software pipelining\n", header_cb->id);
	  header_cb->flags = L_CLR_BIT_FLAG (header_cb->flags, L_CB_SOFTPIPE);
	  continue;
	}

      if (loop->feedback_op->id != loop->cb->last_op->id)
	L_split_cb_after (fn, loop->cb, loop->feedback_op);

      /* Remove impossible sync arcs */
      L_adjust_invalid_sync_arcs_in_cb (header_cb);
    }

  /* JWS 20001115
   * Must do infinite lifetime repair before final
   * dataflow analysis to ensure all resulting truly disjoint 
   * LpipeLiveRanges are split.  Furthermore, the kernel
   * attribute must be attached after fix_infinite_lifetimes
   * but before the next call to dataflow, so that complete
   * write moves do not get deleted.
   */

  L_partial_dead_code_removal (fn);
  L_demote_branches (fn);
  L_do_flow_analysis (fn, LIVE_VARIABLE | SUPPRESS_PG);

  /*******************************************************************
   * Phase 3: Fix all infinite lifetimes so rotating registers work  *
   *          correctly, then rename disjoint lifetimes in loop      *
   *          Finanally, run dataflow.                               *
   ******************************************************************/

  for (loop = fn->first_inner_loop; loop; loop = loop->next_inner_loop)
    {
      header_cb = loop->cb;

      if (!L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_SOFTPIPE))
	continue;

      Lpipe_fix_infinite_lifetimes (fn, header_cb);
      Lpipe_create_comp_code_blocks (fn, loop);
      Lpipe_rename_defined_macros (fn, loop);
    }

  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE | REACHING_DEFINITION);

  for (loop = fn->first_inner_loop; loop; loop = loop->next_inner_loop)
    {
      L_Oper *loop_back_br;
      header_cb = loop->cb;

      /* Check if loop is marked for software pipelining */
      if (!L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_SOFTPIPE))
	continue;

      /* find loop back branch */
      loop_back_br = loop->feedback_op;

      attr = L_new_attr ("kernel", 0);
      header_cb->attr = L_concat_attr (header_cb->attr, attr);

      /* Ensure that each virtual register has only one lifetime. */
      Lpipe_rename_disj_vregs_in_loop (fn, loop);

      /* Check if L_CB_SOFTPIPE flag reset because of infinite lifetime */
      if (!L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_SOFTPIPE))
	{
	  if (Lpipe_print_statistics)
	    fprintf (pipe_statfile, ":([ Loop with header cb %d has infinite "
		     "lifetime.  Cannot software pipeline.\n", header_cb->id);
	  continue;
	}
    }

  L_partial_dead_code_removal (fn);
  L_demote_branches (fn);
  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE |
		      REACHING_DEFINITION | SUPPRESS_PG);
  L_compute_oper_weight (fn, 0, 1);

  /*******************************************************************
   * Phase 4: Pipeline each loop.                                    *
   ******************************************************************/

  for (loop = fn->first_inner_loop; loop; loop = loop->next_inner_loop)
    {
      if (!L_EXTRACT_BIT_VAL (loop->cb->flags, L_CB_SOFTPIPE))
	continue;
      Lpipe_pipeline_loop (fn, loop);
    }

  /*******************************************************************
   * Phase 5: Insert defines to protect rotating register lifetimes  *
   *          and perform cleanup steps.                             *
   ******************************************************************/

  if (Lpipe_schema == MULTI_EPI_ROT_REG || Lpipe_schema == KERNEL_ONLY)
    {
      /* Allocate the rotating registers in chunks as specified by the
         bank settings and create the operand defines to ensure
         correct live range analysis. */
      Lpipe_delete_defines (fn);
      Lpipe_create_defines (fn);

      L_partial_dead_code_removal (fn);
      L_do_flow_analysis (fn, DOMINATOR_CB | REACHING_DEFINITION);

      L_inner_loop_detection (fn, 0);

      Lpipe_reduce_defines (fn);

      if (Lpipe_combine_cbs)
	{
	  /* Remainder loop and MVE schemas require the prologue attribute
	     to be present on said cb.  These optis often merge the prologue
	     into the previous cb and eliminate that attribute. */
	  L_jump_elim_branch_to_next_block (fn, 0);
	  L_jump_combine_branch_to_uncond_branch (fn, 0);
	  L_jump_merge_always_successive_blocks (fn, 0);
	  L_jump_combine_labels (fn, 0);
	  L_jump_branch_target_expansion (fn, 0);
	}
    }

  L_set_hb_no_fallthru_flag (fn);

  return;
}


/* Search each software pipelined loop for spill code.  If spill code is 
   found, reset software pipelining flag so that acyclic scheduler can 
   schedule the spill code.  Called by code generator right after register 
   allocation. */

void
Lpipe_mark_loops_with_spills (L_Func * fn)
{
  L_Inner_Loop *loop;
  L_Cb *cb, *next_cb, *epilogue_cb, *prologue_cb,
    *rem_epi_cb = NULL, *rem_loop_cb = NULL, *first_cb = NULL;
  L_Oper *oper;
  L_Attr *attr;
  int num_cb, num_int_spills, num_flt_spills, num_dbl_spills, has_spills,
    spill_opers, swp, num_opers, rem_num_opers, reroll_num_opers,
    ko_num_opers, rem_ko_num_opers, reroll_ko_num_opers,
    has_rem_loop, acyclic, rem_epi_found, rem_loop_found, reroll = -1;
  double dyn_spill_opers;

  /* 20031023 SZU
   * Removed for Limpact to work. Don't see reason for restriction.
   */
  if (Lpipe_schema == KERNEL_ONLY)
    return;

  L_inner_loop_detection (fn, 0);
  L_compute_oper_weight (fn, 0, 1);

  for (loop = fn->first_inner_loop; loop; loop = loop->next_inner_loop)
    {
      cb = loop->cb;

#ifdef LP64_ARCHITECTURE
      has_rem_loop = (int)((long)L_find_attr (cb->attr, "has_rem_loop"));
      acyclic = (int)((long)L_find_attr (cb->attr, "L_CB_SOFTPIPE"));
#else
      has_rem_loop = (int) L_find_attr (cb->attr, "has_rem_loop");
      acyclic = (int) L_find_attr (cb->attr, "L_CB_SOFTPIPE");
#endif

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) || acyclic)
	{
	  has_spills = 0;
	  num_int_spills = 0;
	  num_flt_spills = 0;
	  num_dbl_spills = 0;
	  num_opers = 0;
	  ko_num_opers = 0;
	  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	    {
	      if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPILL_CODE))
		{
		  if (L_int_load_opcode (oper) || L_int_store_opcode (oper) ||
		      L_int_move_opcode (oper))
		    num_int_spills++;
		  if (L_flt_load_opcode (oper) || L_flt_store_opcode (oper) ||
		      L_flt_move_opcode (oper))
		    num_flt_spills++;
		  if (L_dbl_load_opcode (oper) || L_dbl_store_opcode (oper) ||
		      L_dbl_move_opcode (oper))
		    num_dbl_spills++;
		  has_spills = 1;
		}
	      num_opers++;

	      if (acyclic ||
		  ((attr = L_find_attr (oper->attr, "iter")) &&
		   (attr->field[0]->value.i == 1)))
		ko_num_opers++;

	      if (L_cond_branch_opcode (oper))
		{
		  epilogue_cb = (oper == cb->last_op) ? cb->next_cb : 
		    L_find_branch_dest (oper);

		  if (L_find_attr (epilogue_cb->attr, "epilogue"))
		    num_opers += L_cb_size (epilogue_cb);
		}
	    }

	  prologue_cb = cb->prev_cb;

	  if (L_find_attr (prologue_cb->attr, "prologue"))
	    {
	      for (oper = prologue_cb->first_op; oper != NULL;
		   oper = oper->next_op)
		{
		  num_opers++;

		  if (L_cond_branch_opcode (oper))
		    {
		      epilogue_cb = L_find_branch_dest (oper);

		      if (L_find_attr (epilogue_cb->attr, "epilogue") &&
			  epilogue_cb != cb->next_cb)
			num_opers += L_cb_size (epilogue_cb);
		    }
		}
	    }
	  else if ((!L_find_attr (cb->attr, "L_CB_SOFTPIPE")) &&
		   (!(L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))))
	    {
	      L_punt ("Lpipe_mark_loops_with_spills: "
		      "Expected to find prologue attribute, but did not.  "
		      "Cb: %d, Function: %s\n", prologue_cb->id, fn->name);
	    }

	  if (has_spills)
	    {
	      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_SOFTPIPE);

	      /* if flag was cleared, need to remove isl attributes since 
	         acyclic scheduler will add them */
	      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
		{
		  if ((attr = L_find_attr (oper->attr, "isl")))
		    oper->attr = L_delete_attr (oper->attr, attr);
		}
	    }

	  rem_num_opers = num_opers;
	  rem_ko_num_opers = ko_num_opers;
	  reroll_num_opers = num_opers;
	  reroll_ko_num_opers = ko_num_opers;

	  if (has_rem_loop)
	    {
	      for (next_cb = cb->next_cb; next_cb != NULL;
		   next_cb = next_cb->next_cb)
		{
		  if (!(attr = L_find_attr (next_cb->attr, "epilogue")))
		    {
		      first_cb = next_cb;
		      break;
		    }
		}

	      rem_epi_found = 0;
	      num_cb = 0;
	      for (; next_cb != NULL; next_cb = next_cb->next_cb)
		{
		  num_cb++;
		  attr = L_find_attr (next_cb->attr, "rem_epilogue");
		  if (attr != NULL && attr->field[0]->value.i == cb->id)
		    {
		      rem_epi_found = 1;
		      rem_epi_cb = next_cb;
		      break;
		    }
		}

	      if (!rem_epi_found)
		{
		  L_punt ("Lpipe_mark_loops_with_spills: "
			  "could not find epilogue for remainder loop.  "
			  "cb: %d, function: %s\n", cb->id, fn->name);
		}
	      if (num_cb > 4)
		{
		  L_punt ("Lpipe_mark_loops_with_spills: "
			  "epilogue for remainder loop more than 4 cbs "
			  "after original loop.  cb: %d, function: %s\n",
			  cb->id, fn->name);
		}

	      rem_loop_found = 0;
	      num_cb = 0;
	      for (; next_cb != NULL; next_cb = next_cb->next_cb)
		{
		  num_cb++;
		  attr = L_find_attr (next_cb->attr, "rem_loop");
		  if (attr != NULL && attr->field[0]->value.i == cb->id)
		    {
		      rem_loop_found = 1;
		      rem_loop_cb = next_cb;
		      break;
		    }
		}

	      if (!rem_loop_found)
		{
		  /* ITI/MCM Certain optimizations move the rem_loop
		     up above the rem_epilogue and modulo scheduled
		     loop.  Since this is for statistics only, skip
		     the remaining code in the has_rem_loop if
		     conditional. */
#if 0
		  L_punt ("Lpipe_mark_loops_with_spills: "
			  "could not find remainder loop.  "
			  "cb: %d, function: %s\n", cb->id, fn->name);
#endif
		  has_rem_loop = 0;
		}
	    }

	  if (has_rem_loop)
	    {
	      for (next_cb = first_cb; next_cb != rem_loop_cb;
		   next_cb = next_cb->next_cb)
		{
		  rem_num_opers += L_cb_size (next_cb);
		  rem_ko_num_opers += L_cb_size (next_cb);
		}

	      rem_num_opers += L_cb_size (rem_loop_cb);
	      rem_ko_num_opers += L_cb_size (rem_loop_cb);

	      attr = L_find_attr (rem_loop_cb->attr, "reroll");
#ifdef LP64_ARCHITECTURE
	      reroll = (int)((long)attr);
#else
	      reroll = (int) attr;
#endif

	      if (reroll)
		{
		  for (next_cb = first_cb; next_cb != rem_epi_cb;
		       next_cb = next_cb->next_cb)
		    {
		      reroll_num_opers += L_cb_size (next_cb);
		      reroll_ko_num_opers += L_cb_size (next_cb);
		    }

		  for (oper = rem_epi_cb->first_op; oper != NULL;
		       oper = oper->next_op)
		    {
		      reroll_num_opers++;
		      reroll_ko_num_opers++;
		      if (L_cond_branch_opcode (oper))
			break;
		    }

		  reroll_num_opers += attr->field[0]->value.i;
		  reroll_ko_num_opers += attr->field[0]->value.i;
		}
	      else
		{
		  reroll_num_opers = rem_num_opers;
		  reroll_ko_num_opers = rem_ko_num_opers;
		}
	    }

	  if (Lpipe_add_spill_attributes)
	    {
	      attr = L_new_attr ("code_size", 1);
	      L_set_int_attr_field (attr, 0, num_opers);
	      cb->attr = L_concat_attr (cb->attr, attr);
	      attr = L_new_attr ("int_flt_dbl_spills", 3);
	      L_set_int_attr_field (attr, 0, num_int_spills);
	      L_set_int_attr_field (attr, 1, num_flt_spills);
	      L_set_int_attr_field (attr, 2, num_dbl_spills);
	      cb->attr = L_concat_attr (cb->attr, attr);

	      fprintf (spill_statfile, "\n");
	      attr = L_find_attr (cb->attr, "LOOP");
	      /* print source line number for loop if available and
	         print cb id */
	      if (attr != NULL)
		fprintf (spill_statfile,
			 "Spill_stats_and_code_size_for_loop_on_line: "
			 ITintmaxformat "\n", attr->field[1]->value.i);
	      else
		fprintf (spill_statfile,
			 "Spill_stats_and_code_size_for_loop_on_line: %d\n",
			 0);
	      fprintf (spill_statfile, "Cb: %d\n", cb->id);
	      fprintf (spill_statfile, "Function: %s\n", fn->name);
	      if (has_rem_loop)
		{
		  fprintf (spill_statfile, "Has_remainder_loop: %d\n", 1);
		  if (reroll)
		    fprintf (spill_statfile, "Reroll: %d\n", 1);
		  else
		    fprintf (spill_statfile, "Reroll: %d\n", 0);
		}
	      else
		{
		  fprintf (spill_statfile, "Has_remainder_loop: %d\n", 0);
		  fprintf (spill_statfile, "Reroll: %d\n", 0);
		}
	      fprintf (spill_statfile, "code_size: %d\n", num_opers);
	      fprintf (spill_statfile, "rem_code_size: %d\n", rem_num_opers);
	      fprintf (spill_statfile, "reroll_code_size: %d\n",
		       reroll_num_opers);
	      fprintf (spill_statfile, "ko_code_size: %d\n", ko_num_opers);
	      fprintf (spill_statfile, "rem_ko_code_size: %d\n",
		       rem_ko_num_opers);
	      fprintf (spill_statfile, "reroll_ko_code_size: %d\n",
		       reroll_ko_num_opers);
	      fprintf (spill_statfile, "int_flt_dbl_spills: %d %d %d\n",
		       num_int_spills, num_flt_spills, num_dbl_spills);
	    }
	}
    }

  if (Lpipe_add_spill_attributes)
    {
      num_opers = 0;
      reroll_num_opers = 0;
      spill_opers = 0;
      dyn_spill_opers = 0.0;
      swp = 0;
      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	{
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) ||
	      L_find_attr (cb->attr, "L_CB_SOFTPIPE"))
	    {
	      swp = 1;
	    }
	  attr = L_find_attr (cb->attr, "reroll");
	  if (attr != NULL)
	    {
	      reroll_num_opers += attr->field[0]->value.i;
	      num_opers += L_cb_size (cb);
	    }
	  else
	    {
	      reroll_num_opers += L_cb_size (cb);
	      num_opers += L_cb_size (cb);
	    }
	  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	    {
	      if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPILL_CODE))
		{
		  spill_opers++;
		  dyn_spill_opers += oper->weight;
		}
	    }
	}

      fprintf (code_statfile, "\n");
      fprintf (code_statfile, "Function: %s\n", fn->name);
      fprintf (code_statfile, "Total_code_size: %d\n", num_opers);
      fprintf (code_statfile, "Total_reroll_code_size: %d\n",
	       reroll_num_opers);

      if (swp)
	{
	  fprintf (swp_code_statfile, "\n");
	  fprintf (swp_code_statfile, "Function: %s\n", fn->name);
	  fprintf (swp_code_statfile, "Total_code_size: %d\n", num_opers);
	  fprintf (swp_code_statfile, "Total_reroll_code_size: %d\n",
		   reroll_num_opers);
	}
    }
}


/* This assumes the impact architecture's register file model */
void
Lpipe_compute_reg_pressure (L_Func * fn)
{
  Set int_regs = 0;
  Set flt_regs = 0;
  Set inv_int_regs = 0;
  Set inv_flt_regs = 0;
  Set var_int_regs = 0;
  Set var_flt_regs = 0;
  Set rem_int_regs = 0;
  Set reroll_int_regs = 0;
  Set func_int_regs = 0;
  Set func_reroll_int_regs = 0;
  L_Inner_Loop *loop;
  L_Oper *oper;
  L_Operand *dest;
  L_Operand *src;
  int i;
  int num_int_regs;
  int num_rem_int_regs;
  int num_reroll_int_regs;
  int num_func_int_regs;
  int num_func_reroll_int_regs;
  int num_flt_regs;
  int num_inv_int_regs;
  int num_inv_flt_regs;
  int num_var_int_regs;
  int num_var_flt_regs;
  L_Attr *attr;
  int has_rem_loop;
  L_Cb *cb = NULL, *next_cb = NULL;
  int rem_loop_found;
  int swp;
  int num_live;
  int max_live;
  int rem_max_live;
  int reroll_max_live;
  int *int_reg_array;
  Set live_reg_set = 0;

  if (!Lpipe_compute_loop_reg_pressure)
    return;

  L_inner_loop_detection (fn, 0);
  L_do_flow_analysis (fn, LIVE_VARIABLE);

  for (loop = fn->first_inner_loop; loop; loop = loop->next_inner_loop)
    {
      header_cb = loop->cb;

      if (!L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_SOFTPIPE) &&
	  !L_find_attr (header_cb->attr, "L_CB_SOFTPIPE"))
	continue;

#ifdef LP64_ARCHITECTURE
      has_rem_loop = (int)((long)L_find_attr (header_cb->attr,
					      "has_rem_loop"));
#else
      has_rem_loop = (int) L_find_attr (header_cb->attr, "has_rem_loop");
#endif

      for (oper = header_cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = oper->dest[i];
	      if (L_is_register (dest))
		{
		  if (L_is_ctype_integer (dest))
		    {
		      int_regs = Set_add (int_regs, dest->value.r);
		      var_int_regs = Set_add (var_int_regs, dest->value.r);
		    }
		  else if (L_is_ctype_flt (dest))
		    {
		      flt_regs = Set_add (flt_regs, dest->value.r);
		      var_flt_regs = Set_add (var_flt_regs, dest->value.r);
		    }
		  else if (L_is_ctype_dbl (dest))
		    {
		      flt_regs = Set_add (flt_regs, dest->value.r);
		      flt_regs = Set_add (flt_regs, dest->value.r + 1);
		      var_flt_regs = Set_add (var_flt_regs, dest->value.r);
		      var_flt_regs =
			Set_add (var_flt_regs, dest->value.r + 1);
		    }
		}
	    }

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      src = oper->src[i];
	      if (L_is_register (src))
		{
		  if (L_is_ctype_integer (src))
		    {
		      int_regs = Set_add (int_regs, src->value.r);
		    }
		  else if (L_is_ctype_flt (src))
		    {
		      flt_regs = Set_add (flt_regs, src->value.r);
		    }
		  else if (L_is_ctype_dbl (src))
		    {
		      flt_regs = Set_add (flt_regs, src->value.r);
		      flt_regs = Set_add (flt_regs, src->value.r + 1);
		    }
		}
	    }
	}

      max_live = 0;
      num_int_regs = Set_size (int_regs);
      int_reg_array = (int *) calloc (num_int_regs, sizeof (int));
      Set_2array (int_regs, int_reg_array);
      for (oper = header_cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  num_live = 0;
	  live_reg_set = L_get_oper_OUT_set (header_cb, oper, BOTH_PATHS);
	  for (i = 0; i < num_int_regs; i++)
	    {
	      if (Set_in (live_reg_set, L_REG_INDEX (int_reg_array[i])))
		{
		  num_live++;
		}
	    }
	  if (num_live > max_live)
	    {
	      max_live = num_live;
	    }
	}
      Lcode_free (int_reg_array);

      rem_max_live = 0;
      reroll_max_live = 0;

      if (has_rem_loop)
	{

	  rem_loop_found = 0;
	  for (next_cb = header_cb->next_cb; next_cb != NULL;
	       next_cb = next_cb->next_cb)
	    {
	      attr = L_find_attr (next_cb->attr, "rem_loop");
	      if (attr != NULL && attr->field[0]->value.i == header_cb->id)
		{
		  rem_loop_found = 1;
		  break;
		}
	    }

	  if (!rem_loop_found)
	    {
	      /* ITI/MCM Certain optimizations move the rem_loop up
	         above the rem_epilogue and modulo scheduled loop.
	         Since this is for statistics only, skip the remaining
	         code in the has_rem_loop if conditional. */
#if 0
	      L_punt ("Lpipe_compute_reg_pressure: "
		      "could not find remainder loop. "
		      "cb: %d, function: %s\n", header_cb->id, fn->name);
#endif
	      has_rem_loop = 0;
	    }
	}

      if (has_rem_loop)
	{
	  for (oper = next_cb->first_op; oper != NULL; oper = oper->next_op)
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  dest = oper->dest[i];
		  if (L_is_register (dest))
		    {
		      if (L_is_ctype_integer (dest))
			{
			  rem_int_regs =
			    Set_add (rem_int_regs, dest->value.r);
			}
		    }
		}

	      for (i = 0; i < L_max_src_operand; i++)
		{
		  src = oper->src[i];
		  if (L_is_register (src))
		    {
		      if (L_is_ctype_integer (src))
			{
			  rem_int_regs = Set_add (rem_int_regs, src->value.r);
			}
		    }
		}
	    }

	  rem_max_live = 0;
	  num_int_regs = Set_size (rem_int_regs);
	  int_reg_array = (int *) calloc (num_int_regs, sizeof (int));
	  Set_2array (rem_int_regs, int_reg_array);
	  for (oper = next_cb->first_op; oper != NULL; oper = oper->next_op)
	    {
	      num_live = 0;
	      live_reg_set = L_get_oper_OUT_set (next_cb, oper, BOTH_PATHS);
	      for (i = 0; i < num_int_regs; i++)
		{
		  if (Set_in (live_reg_set, L_REG_INDEX (int_reg_array[i])))
		    {
		      num_live++;
		    }
		}
	      if (num_live > rem_max_live)
		{
		  rem_max_live = num_live;
		}
	    }
	  Lcode_free (int_reg_array);

	  attr = L_find_attr (next_cb->attr, "reroll");
	  if (attr == NULL)
	    {
	      reroll_int_regs = Set_copy (rem_int_regs);
	      reroll_max_live = rem_max_live;
	    }
	}

      rem_int_regs = Set_union_acc (rem_int_regs, int_regs);
      rem_max_live = IMPACT_MAX (rem_max_live, max_live);
      reroll_int_regs = Set_union_acc (reroll_int_regs, int_regs);
      reroll_max_live = IMPACT_MAX (reroll_max_live, max_live);

      inv_int_regs = Set_subtract (int_regs, var_int_regs);
      inv_flt_regs = Set_subtract (flt_regs, var_flt_regs);

      num_int_regs = Set_size (int_regs);
      num_rem_int_regs = Set_size (rem_int_regs);
      num_reroll_int_regs = Set_size (reroll_int_regs);
      num_flt_regs = Set_size (flt_regs);
      num_var_int_regs = Set_size (var_int_regs);
      num_var_flt_regs = Set_size (var_flt_regs);
      num_inv_int_regs = Set_size (inv_int_regs);
      num_inv_flt_regs = Set_size (inv_flt_regs);

      fprintf (reg_statfile, "\n");
      attr = L_find_attr (header_cb->attr, "LOOP");
      /* print source line number for loop if available and print cb id */
      if (attr != NULL)
	fprintf (reg_statfile,
		 "Register_pressure_for_loop_on_line: " ITintmaxformat "\n",
		 attr->field[1]->value.i);
      else
	fprintf (reg_statfile, "Register_pressure_for_loop_on_line: %d\n", 0);
      fprintf (reg_statfile, "Cb: %d\n", header_cb->id);
      fprintf (reg_statfile, "Function: %s\n", fn->name);
      if (has_rem_loop)
	{
	  fprintf (reg_statfile, "Has_remainder_loop: %d\n", 1);
	  attr = L_find_attr (next_cb->attr, "reroll");
	  if (attr != NULL)
	    fprintf (reg_statfile, "Reroll: %d\n", 1);
	  else
	    fprintf (reg_statfile, "Reroll: %d\n", 0);
	}
      else
	{
	  fprintf (reg_statfile, "Has_remainder_loop: %d\n", 0);
	  fprintf (reg_statfile, "Reroll: %d\n", 0);
	}
/*
      fprintf(reg_statfile,"Number_of_variant_int_regs: %d\n", 
        num_var_int_regs);
      fprintf(reg_statfile,"Number_of_variant_flt_regs: %d\n", 
        num_var_flt_regs);
      fprintf(reg_statfile,"Number_of_invariant_int_regs: %d\n", 
        num_inv_int_regs);
      fprintf(reg_statfile,"Number_of_invariant_flt_regs: %d\n", 
        num_inv_flt_regs);
      fprintf(reg_statfile,"Total_number_of_flt_regs: %d\n", 
        num_flt_regs);
*/
      fprintf (reg_statfile, "Total_number_of_int_regs: %d\n", num_int_regs);
      fprintf (reg_statfile, "Total_number_of_rem_int_regs: %d\n",
	       num_rem_int_regs);
      fprintf (reg_statfile, "Total_number_of_reroll_int_regs: %d\n",
	       num_reroll_int_regs);
      fprintf (reg_statfile, "Max_live: %d\n", max_live);
      fprintf (reg_statfile, "Max_live_rem: %d\n", rem_max_live);
      fprintf (reg_statfile, "Max_live_reroll: %d\n", reroll_max_live);

      attr = L_new_attr ("var_inv_int_regs", 2);
      L_set_int_attr_field (attr, 0, num_var_int_regs);
      L_set_int_attr_field (attr, 1, num_inv_int_regs);
      header_cb->attr = L_concat_attr (header_cb->attr, attr);
      attr = L_new_attr ("var_inv_flt_regs", 2);
      L_set_int_attr_field (attr, 0, num_var_flt_regs);
      L_set_int_attr_field (attr, 1, num_inv_flt_regs);
      header_cb->attr = L_concat_attr (header_cb->attr, attr);

      int_regs = Set_dispose (int_regs);
      flt_regs = Set_dispose (flt_regs);
      var_int_regs = Set_dispose (var_int_regs);
      var_flt_regs = Set_dispose (var_flt_regs);
      inv_int_regs = Set_dispose (inv_int_regs);
      inv_flt_regs = Set_dispose (inv_flt_regs);
      rem_int_regs = Set_dispose (rem_int_regs);
      reroll_int_regs = Set_dispose (reroll_int_regs);
    }

  swp = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) ||
	  L_find_attr (cb->attr, "L_CB_SOFTPIPE"))
	{
	  swp = 1;
	}

      attr = L_find_attr (cb->attr, "reroll");

      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      dest = oper->dest[i];
	      if (L_is_register (dest))
		{
		  if (L_is_ctype_integer (dest))
		    {
		      func_int_regs = Set_add (func_int_regs, dest->value.r);
		      if (attr == NULL)
			{
			  func_reroll_int_regs =
			    Set_add (func_reroll_int_regs, dest->value.r);
			}
		    }
		}
	    }

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      src = oper->src[i];
	      if (L_is_register (src))
		{
		  if (L_is_ctype_integer (src))
		    {
		      func_int_regs = Set_add (func_int_regs, src->value.r);
		      if (attr == NULL)
			{
			  func_reroll_int_regs =
			    Set_add (func_reroll_int_regs, src->value.r);
			}
		    }
		}
	    }
	}
    }

  if (swp)
    {
      num_func_int_regs = Set_size (func_int_regs);
      num_func_reroll_int_regs = Set_size (func_reroll_int_regs);
      fprintf (func_reg_statfile, "\n");
      fprintf (func_reg_statfile, "Register_usage_for_function: %s\n",
	       fn->name);
      fprintf (func_reg_statfile, "Total_number_of_int_regs: %d\n",
	       num_func_int_regs);
      fprintf (func_reg_statfile, "Total_number_of_reroll_int_regs: %d\n",
	       num_func_reroll_int_regs);
    }

  func_int_regs = Set_dispose (func_int_regs);
  func_reroll_int_regs = Set_dispose (func_reroll_int_regs);
}


#if 0

/* Fill delay slot for loop back branch in kernel for remainder loop
   code schema.  Only used when pipelining for a real PA processor.  
   Called by code generator just before postpass acyclic scheduling. */
void
Lpipe_fill_delay_slots (fn)
     L_Func *fn;
{
  L_Cb *header_cb;
  L_Cb *prologue_cb;
  L_Oper *oper;
  L_Oper *new_oper;
  L_Oper *loop_back_br;
  L_Attr *attr;


  /* Fill delay slot for loop back branch for each software pipelined loop */
  for (header_cb = fn->first_cb; header_cb != NULL;
       header_cb = header_cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_SOFTPIPE))
	{

	  /* remove first operation from the kernel and place a copy of
	     it and the end of the prologue and after the loop back branch */
	  prologue_cb = header_cb->prev_cb;
	  loop_back_br = Lpipe_find_loop_back_br (fn, header_cb);
	  oper = header_cb->first_op;
	  L_remove_oper (header_cb, oper);
	  new_oper = L_copy_operation (oper);
	  L_insert_oper_after (header_cb, header_cb->last_op, oper);
	  L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper);
	  /* remove attr on new_oper because prologue will be scheduled
	     by acyclic scheduler */
	  attr = L_find_attr (new_oper->attr, "isl");
	  new_oper->attr = L_delete_attr (new_oper->attr, attr);
	  /* squash oper if loop back branch not taken */
	  loop_back_br->flags = L_SET_BIT_FLAG (loop_back_br->flags,
						L_OPER_SQUASHING);
	}
    }
}

#endif
