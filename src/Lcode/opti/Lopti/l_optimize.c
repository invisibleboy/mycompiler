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
 *      File :          l_optimize.c
 *      Description :   driver for global traditional optimizder
 *      Creation Date : July, 1990
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"
#include "l_disjvreg.h"

#define MAX_ITER        10	/* remember, second loop in opti level 4 */
#define MAX_ITER1       12
#undef TRACK_LOPTI_ITERATIONS

/* extern declarations */
extern int L_oper_breakdown (L_Func *);
extern int L_local_code_optimization (L_Func *, int);
extern int L_global_dead_code_optimization (L_Func *);
extern int L_global_code_optimization (L_Func *);
extern int L_partial_code_elimination (L_Func *);
extern int L_PRE_optimization (L_Func *);
extern int L_partial_dead_code_elimination (L_Func *);
extern int L_PCE_cleanup (L_Func *, int);
extern int L_min_cut_PDE (L_Func *);
extern int L_jump_initial_cleanup(L_Func *);
extern int L_jump_optimization (L_Func *);
extern int L_loop_optimization (L_Func *);
extern void L_oper_recombine (L_Func *);
extern int L_unification (L_Func *);
extern int L_global_mem_expression_copy_prop (L_Func *);
extern int L_global_dead_store_removal (L_Func *);


void
L_code_optimize (L_Func * fn)
{
  int i, change, local = 0, dead_code = 0, global = 0, jump = 0,
    loop = 0, pre_post_inc = 0, unif = 0, memflow = 0,
    pce = 0, cb_count = 0;

#ifdef TRACK_ITERATIONS
  printf ("Beginning new Lopti run on function %s.\n", fn->name);
#endif

  /*
   *  LEVEL = 0: No optimization
   * ----------------------------------------------------------------------
   */

  if (Lopti_opti_level == 0)
    {
      fprintf (stderr, "# level 0 does no optimization\n");
      return;
    }

  /*
   *  LEVEL != 0: Optimizations applied for all non-zero levels
   * ----------------------------------------------------------------------
   */

  /* eliminate any unreachable code */
  L_delete_unreachable_blocks (fn);

  /* SAM 1-05, added because Pcode not doing simple optis anymore */
  STAT_INIT ("L_jump_optimization", fn);
  jump = L_jump_initial_cleanup (fn);
  STAT_DUMP ();

  L_do_flow_analysis (fn, DOMINATOR_CB);

  /* record memory label addresses n attribute field of loads and stores */
  if (Lopti_do_mark_memory_labels)
    L_find_memory_labels (fn);

  /* mark lds/sts based on incoming parmeters, 
     in Fortran these MUST be indep */
  if (Lopti_do_mark_incoming_parms)
    L_find_incoming_parameters (fn);

  /* set <E> flag for jsrs, simple form of Roger's analysis */
  if (Lopti_do_mark_trivial_sef_jsrs)
    L_find_side_effect_free_sub_calls (fn);

  /* set <Y> flag for jsrs, to disable opti across these ops */
  if (Lopti_do_mark_sync_jsrs)
    L_find_synchronization_sub_calls (fn);

  /* set <F> flag for ld/sts, simple form of Roger's analysis */
  if (Lopti_do_mark_trivial_safe_ops)
    L_mark_safe_instructions (fn);

  /* breakup incoming pre/post inc ld/st, put back together later */
  L_breakup_pre_post_inc_ops (fn);

  /* breakup some ops the code gen will split up to expose opti 
     opportunities */
  L_oper_breakdown (fn);

  /*
   * LEVEL = 1: local opti and dead code removal
   * ----------------------------------------------------------------------
   */

  /* REH - 10/95 - Restrict zero weight functions to level 1 opti */
  if ((Lopti_opti_level == 1) || ((Lopti_only_lvl1_for_zero_weight_fn) &&
				  (fn->weight <= 0.0)))
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("L_local_code_optimization", fn);
      local = L_local_code_optimization (fn, 1);
      STAT_DUMP ();
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("L_global_dead_code_optimization", fn);
      dead_code = L_global_dead_code_optimization (fn);

      STAT_DUMP ();
      pre_post_inc = L_remove_uncombinable_pre_post_inc_ops (fn);
    }

  /*
   * LEVEL = 2: local opti, dead code removal, and global opti
   * ----------------------------------------------------------------------
   */

  else if (Lopti_opti_level == 2)
    {
      for (i = 0; i < MAX_ITER; i++)
	{
	  L_do_flow_analysis (fn, LIVE_VARIABLE);
	  STAT_INIT ("L_local_code_optimization", fn);
	  local = L_local_code_optimization (fn, 1);
	  STAT_DUMP ();
	  L_do_flow_analysis (fn,
			      DOMINATOR_CB | REACHING_DEFINITION |
			      AVAILABLE_DEFINITION |
			      MEM_AVAILABLE_DEFINITION |
			      AVAILABLE_EXPRESSION);
	  L_compute_danger_info (fn);
	  STAT_INIT ("L_global_code_optimization", fn);
	  global = L_global_code_optimization (fn);
	  STAT_DUMP ();
	  L_delete_all_danger_ext (fn);
	  L_do_flow_analysis (fn, LIVE_VARIABLE);
	  STAT_INIT ("L_global_dead_code_optimization", fn);
	  dead_code = L_global_dead_code_optimization (fn);
	  STAT_DUMP ();
	  pre_post_inc = L_remove_uncombinable_pre_post_inc_ops (fn);
	  change = local + global +dead_code + pre_post_inc;
	  if (!change)
	    break;
	}
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("L_local_code_optimization", fn);
      local = L_local_code_optimization (fn, 1);
      STAT_DUMP ();
    }

  /*
   * LEVEL = 3: local opti, dead code removal, global opti, and jump opti
   * ----------------------------------------------------------------------
   */

  else if (Lopti_opti_level == 3)
    {
      for (i = 0; i < MAX_ITER; i++)
	{
	  L_do_flow_analysis (fn, LIVE_VARIABLE);
	  STAT_INIT ("L_local_code_optimization", fn);
	  local = L_local_code_optimization (fn, 1);
	  STAT_DUMP ();
	  L_do_flow_analysis (fn,
			      DOMINATOR_CB | REACHING_DEFINITION |
			      AVAILABLE_DEFINITION |
			      MEM_AVAILABLE_DEFINITION |
			      AVAILABLE_EXPRESSION);
	  L_compute_danger_info (fn);
	  STAT_INIT ("L_global_code_optimization", fn);
	  global = L_global_code_optimization (fn);
	  STAT_DUMP ();
	  L_delete_all_danger_ext (fn);
	  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE);
	  STAT_INIT ("L_global_dead_code_optimization", fn);
	  dead_code = L_global_dead_code_optimization (fn);
	  STAT_DUMP ();
	  STAT_INIT ("L_jump_optimization", fn);
	  jump = L_jump_optimization (fn);
	  STAT_DUMP ();
	  pre_post_inc = L_remove_uncombinable_pre_post_inc_ops (fn);
	  change = local + global +dead_code + jump + pre_post_inc;
	  if (!change)
	    break;
	}
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("L_local_code_optimization", fn);
      local = L_local_code_optimization (fn, 1);
      STAT_DUMP ();
    }

  /*
   *  LEVEL = 4: local opti, dead code removal, global opti, jump opti,
   *             and loop opti
   * ----------------------------------------------------------------------
   */

  else if (Lopti_opti_level == 4)
    {
#ifdef TRACK_PCE_CB
      int max_cb_before_PCE = fn->max_cb_id;
#endif

      L_do_flow_analysis (fn, DOMINATOR_CB);
      L_loop_detection (fn, 1);

      L_compute_oper_weight (fn, 0, 1);

      if (Lopti_do_benchmark_specific_opti)	/* deprecated */
	L_do_benchmark_tuning (fn);

      if (Lopti_do_jrg_expansion)
	{
	  L_jump_rg_expansion (fn);
	}

      for (i = 0; i < MAX_ITER1; i++)
	{
#ifdef TRACK_LOPTI_ITERATIONS
	  fprintf (stderr, "Iteration %d of Lopti loop 1 in function %s.\n",
		   i + 1, fn->name);
#endif
	  /* SER: Live variable analysis done inside local code opti */
	  L_do_flow_analysis (fn, LIVE_VARIABLE);
	  STAT_INIT ("L_local_code_optimization", fn);
	  local = L_local_code_optimization (fn, i % 2);
	  STAT_DUMP ();

	  L_do_flow_analysis (fn, DOMINATOR_CB | REACHING_DEFINITION |
			      AVAILABLE_DEFINITION |
			      MEM_AVAILABLE_DEFINITION |
			      AVAILABLE_EXPRESSION);
	  L_compute_danger_info (fn);
	  STAT_INIT ("L_global_code_optimization", fn);
	  global = L_global_code_optimization (fn);
	  STAT_DUMP ();
	  L_delete_all_danger_ext (fn);

	  /* L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE); */
	  L_do_flow_analysis (fn, LIVE_VARIABLE);
	  STAT_INIT ("L_global_dead_code_optimization", fn);
	  dead_code = L_global_dead_code_optimization (fn);
	  STAT_DUMP ();

	  STAT_INIT ("L_jump_optimization", fn);
	  jump = L_jump_optimization (fn);
	  STAT_COUNT ("L_jump_optimization1", jump, NULL);
	  STAT_DUMP ();
	  pre_post_inc = L_remove_uncombinable_pre_post_inc_ops (fn);
	  change = local + global +dead_code + jump + pre_post_inc;
	  if (!change)
	    break;
	}

#ifdef TRACK_LOPTI_ITERATIONS
      if (i == MAX_ITER1)
	{
	  fprintf (stderr, "WARNING! LAST ITERATION REACHED IN LOOP 1 OF "
		   "FUNCTION %s!\n", fn->name);
	  if (local != 0)
	    fprintf (stderr, "L_local_code_optimization reflecting.\n");
	  if (global !=0)
	    fprintf (stderr, "L_global_code_optimization reflecting.\n");
	  if (dead_code != 0)
	    fprintf (stderr,
		     "L_global_dead_code_optimization reflecting.\n");
	  if (jump != 0)
	    fprintf (stderr, "L_jump_optimization reflecting.\n");
	  if (pre_post_inc != 0)
	    fprintf (stderr, "L_pre_post_inc reflecting.\n");
	}
#endif

      if (Lopti_do_split_branches)
	{
	  L_do_flow_analysis (fn, REACHING_DEFINITION);
	  L_split_multidef_branches (fn);
	}

      L_unification (fn);

      if (Lopti_do_PCE)
	{
	  local = global = 0;
	  /* Disable optis subsumed by PRE/PDE. */
	  L_PCE_fix_function_weight ();
	  L_PCE_disable_subsumed_optis ();
	}

      for (i = 0; i < MAX_ITER + 5; i++)
	{
#ifdef TRACK_LOPTI_ITERATIONS
	  fprintf (stderr, "Iteration %d of Lopti PCE loop of function "
		   "%s.\n", i + 1, fn->name);
#endif
	  L_do_flow_analysis (fn, DOMINATOR_CB);
	  L_loop_detection (fn, 1);
	  L_do_flow_analysis (fn,
			      DOMINATOR_CB | LIVE_VARIABLE |
			      AVAILABLE_EXPRESSION | AVAILABLE_DEFINITION |
			      REACHING_DEFINITION);

	  STAT_INIT ("L_loop_optimization", fn);
	  loop = L_loop_optimization (fn);
	  STAT_DUMP ();

	  if (Lopti_do_PCE && Lopti_do_PRE)
	    {
	      STAT_INIT ("PRE", fn);
	      /* SER 20041215: Some loop optis create new control-flow, so
	       * need to split critical edges again. */
	      if (i == 0 || loop)
		{
		  cb_count = fn->n_cb;
		  L_split_fn_critical_edges (fn);
		}

	      if (Lopti_do_PRE_speculative_code_motion && fn->weight > 0.0)
		pce = L_speculative_PRE (fn);
	      else
		pce = L_partial_redundancy_elimination (fn);

	      pce += L_PCE_cleanup (fn, (i+1)%2);

	      /* Do this for a little cleanup. */
	      if (i == 0 && Lopti_do_PRE_speculative_code_motion &&
		  fn->weight > 0.0)
		{
		  pce += L_partial_redundancy_elimination (fn);
		  pce += L_PCE_cleanup (fn, 0);
		}

	      STAT_DUMP ();
	    }
	  else
	    {
	      L_do_flow_analysis (fn, LIVE_VARIABLE);
	      STAT_INIT ("L_local_code_optimization", fn);
	      local = L_local_code_optimization (fn, (i+1)%2);
	      STAT_DUMP ();

	      L_do_flow_analysis (fn, DOMINATOR_CB |
				  REACHING_DEFINITION | AVAILABLE_DEFINITION |
				  MEM_AVAILABLE_DEFINITION |
				  AVAILABLE_EXPRESSION);
	      L_compute_danger_info (fn);
	      STAT_INIT ("L_global_code_optimization", fn);
	      global = L_global_code_optimization (fn);
	      STAT_DUMP ();
	      L_delete_all_danger_ext (fn);
	    }

	  STAT_INIT ("L_global_memflow_optimization", fn);
	  memflow = L_global_memflow_optimization (fn);
	  STAT_DUMP ();

	  L_do_flow_analysis (fn, LIVE_VARIABLE);
	  STAT_INIT ("L_global_dead_code_optimization", fn);
	  dead_code = L_global_dead_code_optimization (fn);
	  STAT_DUMP ();

	  unif = (i == 0) ? L_unification (fn) : 0;

	  pre_post_inc = L_remove_uncombinable_pre_post_inc_ops (fn);

	  change = loop + pce + local + global + dead_code + pre_post_inc
	    + unif;
	  if (!change)
	    break;

#ifdef TRACK_LOPTI_ITERATIONS
	  if (i == MAX_ITER + 4)
	    {
	      fprintf (stderr, "WARNING! LAST ITERATION REACHED IN PCE "
		       "LOOP OF FUNCTION %s!\n", fn->name);
	      if (loop != 0)
		fprintf (stderr, "L_loop_optimization reflecting.\n");
	      if (pce != 0)
		fprintf (stderr, "PCE reflecting.\n");
	      if (local != 0)
		fprintf (stderr, "L_local_code_optimization reflecting.\n");
	      if (global !=0)
		fprintf (stderr, "L_global_code_optimization reflecting.\n");
	      if (memflow != 0)
		fprintf (stderr, "L_global_memflow_optimization "
			 "reflecting.\n");
	      if (dead_code != 0)
		fprintf (stderr,
			 "L_global_dead_code_optimization reflecting.\n");
	      if (unif != 0)
		fprintf (stderr, "L_unification not terminating within "
			 "bounds.\n");
	      if (pre_post_inc != 0)
		fprintf (stderr, "L_pre_post_inc reflecting.\n");
	    }
#endif
	}

      if (Lopti_do_PCE)
	{
	  L_global_dead_store_removal (fn);

	  if (Lopti_do_PDE)
	    {
	      STAT_INIT ("PDE", fn);
	      /* Clean out for PDE run. */
	      if (Lopti_do_PRE)
		{
		  L_delete_all_expressions (fn);
		  fn->n_expression = 0;
		}
	      else
		{
		  cb_count = fn->n_cb;
		}

	      /* SER 20041215: split critical edges just in case an opti
	       * introduced new control flow. */
	      L_split_fn_critical_edges (fn);
	      L_partial_dead_code_elimination (fn);
	      if (fn->weight != 0.0)
		L_min_cut_PDE (fn);

	      if (Lopti_do_PDE_min_cut && Lopti_do_PDE_predicated)
		L_PDE_combine_pred_guards (fn);

	      if (Lopti_do_PRE_speculative_code_motion)
		L_speculative_PRE (fn);
	      else
		L_partial_redundancy_elimination (fn);
	      L_PCE_cleanup (fn, 0);

	      STAT_DUMP();
	    }

	  STAT_INIT ("PCE", fn);
	  L_PCE_merge_same_cbs (fn);
	  STAT_DUMP ();
	}

      STAT_INIT ("PCE", fn);
      jump = L_jump_optimization (fn);
      STAT_COUNT ("Lopti_PCE_coalesce_cbs", jump, NULL);
      STAT_COUNT ("Lopti_PCE_extra_cbs", (fn->n_cb - cb_count), NULL);

      L_unification (fn);

#ifdef TRACK_PCE_CB
      {
	L_Cb * cb;
	L_Oper * oper;
	int count, new_cb_count, oper_count;
	count = new_cb_count = 0;
	for (cb = fn->first_cb; cb; cb = cb->next_cb)
	  {
	    if (cb->id < max_cb_before_PCE)
	      continue;
	    new_cb_count++;
	    oper_count = 0;
	    for (oper = cb->first_op; oper; oper = oper->next_op)
	      oper_count++;
	    if (oper_count < 2)
	      count++;
	  }
	STAT_COUNT ("Lopti_PCE_new_cbs", new_cb_count, NULL);
	STAT_COUNT ("Lopti_PCE_small_split_edge", count, NULL);
      }
#endif
      STAT_DUMP();

      L_rename_coalesce_disjoint_virtual_registers (fn, NULL);
    }
  else
    {
      fprintf (stderr, "Illegal opti level\n");
    }

  /*
   *  LEVEL != 0: Functions applied for all non-zero levels
   * ----------------------------------------------------------------------
   */

  /* Remove any pre/post incs that cannot be recombined due to some opti */
  L_remove_uncombinable_pre_post_inc_ops (fn);

  /* Combine marked ld/sts and increments into post increments */
  L_generate_pre_post_inc_ops (fn);

  /* recombine any ops broken up at the beginning of opti */
  L_oper_recombine (fn);

  /* last phase of dead code elim to cleanup any junk */
  L_delete_unreachable_blocks (fn);
  L_do_flow_analysis (fn, LIVE_VARIABLE);

  STAT_INIT ("L_global_dead_code_optimization", fn);
  dead_code = L_global_dead_code_optimization (fn);
  STAT_DUMP ();

  /* profile-based trace selection and code layout */
  if (Lopti_do_code_layout)
    LB_code_layout (fn);

  {
    L_Cb *cb;

    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      L_local_dead_code_removal (cb);
  }

  /* redo some analyses here, in case more info was exposed during opti */
  if (Lopti_do_mark_memory_labels)
    L_find_memory_labels (fn);
  if (Lopti_do_mark_trivial_safe_ops)
    L_mark_safe_instructions (fn);

  /* set function <M> flag (mask exceptions) if any ops have it set */
  L_set_function_mask_flag (fn);

  /* classify branches by type and location */
  if (Lopti_do_classify_branches)
    L_classify_branches (fn);

  L_reset_ctype_array ();


  /*
   *  Summarize applied optimizations
   * ----------------------------------------------------------------------
   */

  if (Lopti_print_opti_count)
    {
      fprintf (stderr, "# Summary of optimizations (fn %s) (level %d)\n",
	       fn->name, Lopti_opti_level);
      fprintf (stderr, "\t> Local       %4d\n", Lopti_cnt_local_opti);
      fprintf (stderr, "\t> Global      %4d\n", Lopti_cnt_global_opti);
      fprintf (stderr, "\t> Global (IR) %4d\n",
	       Lopti_inter_region_global_opti);
      fprintf (stderr, "\t> Jump        %4d\n", Lopti_cnt_jump_opti);
      fprintf (stderr, "\t> Loop        %4d\n", Lopti_cnt_loop_opti);

      fprintf (stderr, "IRCom: %f\n",
	       Lopti_inter_region_global_common_sub_elim_wgt);
      fprintf (stderr, "IRCpp: %f\n",
	       Lopti_inter_region_global_copy_prop_wgt);
      fprintf (stderr, "IRLicm: %f\n", Lopti_inter_region_loop_inv_wgt);
      fprintf (stderr, "IRGvm: %f\n", Lopti_inter_region_gvm_wgt);
    }

  if (Lopti_print_opti_breakdown)
    {
      fprintf (stderr, "# Breakdown of optimizations (fn %s) (level %d)\n",
	       fn->name, Lopti_opti_level);
      fprintf (stderr, "    Local opti        %4d\n", Lopti_cnt_local_opti);
      fprintf (stderr, "        > constant propagation        %4d\n",
	       Lopti_cnt_local_constant_prop);
      fprintf (stderr, "        > copy propagation            %4d\n",
	       Lopti_cnt_local_copy_prop);
      fprintf (stderr, "        > reverse copy propagation    %4d\n",
	       Lopti_cnt_local_rev_copy_prop);
      fprintf (stderr, "        > memory copy propagation     %4d\n",
	       Lopti_cnt_local_mem_copy_prop);
      fprintf (stderr, "        > common subexpr elim         %4d\n",
	       Lopti_cnt_local_common_sub_elim);
      fprintf (stderr, "        > redundant load elim         %4d\n",
	       Lopti_cnt_local_red_load_elim);
      fprintf (stderr, "        > redundant store elim        %4d\n",
	       Lopti_cnt_local_red_store_elim);
      fprintf (stderr, "        > constant folding            %4d\n",
	       Lopti_cnt_local_constant_fold);
      fprintf (stderr, "        > strength reduction          %4d\n",
	       Lopti_cnt_local_strength_red);
      fprintf (stderr, "        > constant combining          %4d\n",
	       Lopti_cnt_local_constant_comb);
      fprintf (stderr, "        > operation folding           %4d\n",
	       Lopti_cnt_local_operation_fold);
      fprintf (stderr, "        > branch folding              %4d\n",
	       Lopti_cnt_local_branch_fold);
      fprintf (stderr, "        > operation cancellation      %4d\n",
	       Lopti_cnt_local_operation_cancel);
      fprintf (stderr, "        > dead code removal           %4d\n",
	       Lopti_cnt_local_dead_code_rem);
      fprintf (stderr, "        > code motion                 %4d\n",
	       Lopti_cnt_local_code_motion);
      fprintf (stderr, "        > sign extension removal      %4d\n",
	       Lopti_cnt_local_remove_sign_ext);
      fprintf (stderr, "        > logic reduction             %4d\n",
	       Lopti_cnt_local_reduce_logic);
      fprintf (stderr, "        > register renaming           %4d\n",
	       Lopti_cnt_local_register_rename);

      fprintf (stderr, "    Global opti       %4d\n", Lopti_cnt_global_opti);
      fprintf (stderr, "        > dead code removal           %4d\n",
	       Lopti_cnt_global_dead_code_rem);
      fprintf (stderr, "        > constant propagation        %4d\n",
	       Lopti_cnt_global_constant_prop);
      fprintf (stderr, "        > copy propagation            %4d\n",
	       Lopti_cnt_global_copy_prop);
      fprintf (stderr, "        > memory copy propagation     %4d\n",
	       Lopti_cnt_global_mem_copy_prop);
      fprintf (stderr, "        > common subexpr elim         %4d\n",
	       Lopti_cnt_global_common_sub_elim);
      fprintf (stderr, "        > redundant load elim         %4d\n",
	       Lopti_cnt_global_red_load_elim);
      fprintf (stderr, "        > redundant store elim        %4d\n",
	       Lopti_cnt_global_red_store_elim);
      fprintf (stderr, "        > dead if_then_else elim      %4d\n",
	       Lopti_cnt_global_dead_if_then_else_rem);
      fprintf (stderr, "        > remove unnec boolean        %4d\n",
	       Lopti_cnt_global_elim_boolean_ops);
      fprintf (stderr, "    Jump opti         %4d\n", Lopti_cnt_jump_opti);
      fprintf (stderr, "        > dead block elim             %4d\n",
	       Lopti_cnt_jump_dead_block_elim);
      fprintf (stderr, "        > branch to next block        %4d\n",
	       Lopti_cnt_jump_br_to_next_block);
      fprintf (stderr, "        > branches to same target     %4d\n",
	       Lopti_cnt_jump_br_to_same_target);
      fprintf (stderr, "        > branch to uncond branch     %4d\n",
	       Lopti_cnt_jump_br_to_uncond_br);
      fprintf (stderr, "        > block merge                 %4d\n",
	       Lopti_cnt_jump_block_merge);
      fprintf (stderr, "        > combine labels              %4d\n",
	       Lopti_cnt_jump_combine_labels);
      fprintf (stderr, "        > branch target expansion     %4d\n",
	       Lopti_cnt_jump_br_target_expansion);
      fprintf (stderr, "        > branch swap                 %4d\n",
	       Lopti_cnt_jump_br_swap);

      fprintf (stderr, "    Loop opti         %4d\n", Lopti_cnt_loop_opti);
      fprintf (stderr, "        > branch simplification       %4d\n",
	       Lopti_cnt_loop_br_simp);
      fprintf (stderr, "        > invariant code removal      %4d\n",
	       Lopti_cnt_loop_inv_code_rem);
      fprintf (stderr, "        > global var migration        %4d\n",
	       Lopti_cnt_loop_global_var_mig);
      fprintf (stderr, "        > induction var str red       %4d\n",
	       Lopti_cnt_loop_ind_var_str_red);
      fprintf (stderr, "        > induction var reinit        %4d\n",
	       Lopti_cnt_loop_ind_var_reinit);
      fprintf (stderr, "        > induction var elim          %4d\n",
	       Lopti_cnt_loop_ind_var_elim);
      fprintf (stderr, "        > dead loop removal           %4d\n",
	       Lopti_cnt_dead_loop_rem);

      fprintf (stderr, "    Global opti (inter-region)        %4d\n",
	       Lopti_inter_region_global_opti);
      fprintf (stderr, "        > (IR) constant propagation   %4d\n",
	       Lopti_inter_region_global_constant_prop);
      fprintf (stderr, "        > (IR) copy propagation       %4d\n",
	       Lopti_inter_region_global_copy_prop);
      fprintf (stderr, "        > (IR) memory copy propagation%4d\n",
	       Lopti_inter_region_global_mem_copy_prop);
      fprintf (stderr, "        > (IR) common subexpr elim    %4d\n",
	       Lopti_inter_region_global_common_sub_elim);
      fprintf (stderr, "        > (IR) redundant load elim    %4d\n",
	       Lopti_inter_region_global_red_load_elim);
      fprintf (stderr, "        > (IR) redundant store elim   %4d\n",
	       Lopti_inter_region_global_red_store_elim);
    }
}
