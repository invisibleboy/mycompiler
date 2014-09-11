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
 *      File :          l_superscalar.c
 *      Description :   Driver for superscalar optimizer
 *      Author :        Scott Mahlke
 *
 *      (C) Copyright 1990 Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"
#include <Lcode/l_promotion.h>
#include <Lcode/l_disjvreg.h>

#define REPEAT_LOOP_OPTI        5
#define My_basic_block

/* Defined in l_codegen.c -JCG 6/99 */
extern Func_Name_List *Lsuper_prevent_superblock_list;
extern Func_Name_List *Lsuper_prevent_ILP_opti_list;

static void L_superblock_only (L_Func * fn);
static void L_full_superscalar_optimization (L_Func * fn);

/* Created new top-level function in order to support the following new
 * parameters: do_only_sb_formation, prevent_superblock_functions,
 * prevent_ILP_opti_functions, verbose_prevent_superblock, and
 * verbose_prevent_ILP_opti.
 *
 * These parameters give more control over Lsuperscalar and allow
 * superblock formation/ILP optimization to be turned off and on at
 * the function level. -JCG 6/99
 */
void
Lsuper_optimize (L_Func * fn)
{
  int prevent_ILP_opti, prevent_superblock, old_sb_parm_value;

  /* Handle normal, full-path case */
  if (!Lsuper_do_only_sb_formation)
    {
      /* Allow specific functions to be specified (by hand) that should
       * not be superblocked or ILP optimized (or both)-JCG 6/99
       */
      prevent_superblock =
        is_in_Func_Name_List (Lsuper_prevent_superblock_list, &fn->name[1]);

      prevent_ILP_opti =
        is_in_Func_Name_List (Lsuper_prevent_ILP_opti_list, &fn->name[1]);

      /* Handle normal case, everything done (parameters permitting) */
      if (!prevent_superblock && !prevent_ILP_opti)
        {
          /* Call normal Lsuperscalar driver */
          L_full_superscalar_optimization (fn);
        }

      /* Handle only ILP opti prevented case */
      else if (!prevent_superblock && prevent_ILP_opti)
        {
          /* Make sure user knows these functions are not being
           * ILP optimized!
           */
          if (Lsuper_verbose_prevent_ILP_opti)
            {
              printf ("Note: '%s' not ILP optimized, in "
                      "prevent_ILP_opti_functions!\n", &fn->name[1]);
            }

          /* Call new function that only does superblock formation */
          L_superblock_only (fn);
        }

      /* Handle only superblock prevented case */
      else if (prevent_superblock && !prevent_ILP_opti)
        {
          /* Make sure user knows these functions are not being
           * superblocked!
           */
          if (Lsuper_verbose_prevent_superblock)
            {
              printf ("Note: '%s' not superblocked, in "
                      "prevent_superblock_functions!\n", &fn->name[1]);
            }

          /* Force superblock formation parameter off for this function 
           * and call normal superscalar opti function
           */
          old_sb_parm_value = Lsuper_do_sb_formation;
          Lsuper_do_sb_formation = 0;

          /* Call normal Lsuperscalar driver */
          L_full_superscalar_optimization (fn);

          /* Restore parameter value */
          Lsuper_do_sb_formation = old_sb_parm_value;
        }

      /* Handle both superblock and ILP opti prevented case */
      else
        {
          /* Make sure user knows these functions are not being
           * superblocked!
           */
          if (Lsuper_verbose_prevent_superblock)
            {
              printf ("Note: '%s' not superblocked, in "
                      "prevent_superblock_functions!\n", &fn->name[1]);
            }

          /* Make sure user knows these functions are not being
           * ILP optimized!
           */
          if (Lsuper_verbose_prevent_ILP_opti)
            {
              printf ("Note: '%s' not ILP optimized, in "
                      "prevent_ILP_opti_functions!\n", &fn->name[1]);
            }
        }
    }

  /* Handle case where superblock formation only requested (no optis) */
  else
    {
      /* Allow specific functions to be specified (by hand) that should
       * not be superblocked or ILP optimized (or both)-JCG 6/99
       */
      prevent_superblock =
        is_in_Func_Name_List (Lsuper_prevent_superblock_list, &fn->name[1]);

      /* Handle normal case, superblock done (parameters permitting) */
      if ((!prevent_superblock))
        {
          /* Call new function that only does superblock formation */
          L_superblock_only (fn);
        }
      else
        {
          /* Make sure user knows these functions are not being
           * superblocked!
           */
          if (Lsuper_verbose_prevent_superblock)
            {
              printf ("Note: '%s' not superblocked, in "
                      "prevent_superblock_functions!\n", &fn->name[1]);
            }
        }
    }
}


/* Pulled out the superblock only section to support the new
 * prevent_ILP_opti_functions parameter. -JCG 6/99
 *
 * Note: Still obey Lsuper_do_sb_formation!
 */

static void
L_superblock_only (L_Func * fn)
{
  /*
   *  General loop detection
   */
  L_do_flow_analysis (fn, DOMINATOR_CB);
  L_loop_detection (fn, 0);

  /*
   *  Superblock formation
   */
  if (Lsuper_do_sb_formation)
    {
      L_set_hb_no_fallthru_flag (fn);
      LB_superblock_formation (fn);
    }
}

static void
L_full_superscalar_optimization (L_Func * fn)
{

#ifndef My_basic_block

if(strcmp(fn->name,"_main")==0){
  int i;
  int Lsuper_trace_mode;

  //morteza
//  Lsuper_do_sb_formation=0;
//  Lsuper_do_peel_opt=0;
 // Lsuper_do_loop_unroll=0;
 // Lsuper_do_jump_opti=0;
 // Lsuper_do_multiway_branch_opti=0;
//  Lsuper_do_classic_opti=0;
  //end morteza



  Lsuper_trace_mode = (Lsuper_issue_rate <= Lsuper_height_count_crossover);

  /*
   *  Setup allocation pools used by Lopti functions
   */
  Lopti_init ();

  /*
   *  Lsuperscalar does not handle post-increments well, so
   *  split them up and remove the attributes.
   */
  L_breakup_pre_post_inc_ops (fn);
  L_unmark_all_pre_post_increments (fn);

  /*
   * Split doble-dest predicate defines for predicate expression
   * optimization.
   */
  L_split_pred_defines (fn);

  /*
   *  General loop detection
   */
  L_do_flow_analysis (fn, DOMINATOR_CB);
  L_loop_detection (fn, 0);

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-1");

  /*
   *  Superblock formation
   */
  if (Lsuper_do_sb_formation)
    {
      L_set_hb_no_fallthru_flag (fn);
      LB_superblock_formation (fn);
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-2");

  /*
   *  Jump optimization, don't expand loop headers!!
   */
  if (Lsuper_do_jump_opti)
    {
      L_do_flow_analysis (fn, DOMINATOR_CB);
      PG_setup_pred_graph (fn);
      L_inner_loop_detection (fn, 0);

      STAT_INIT ("Lsuper_jump_optimization", fn);
      Lsuper_jump_optimization (fn, L_JUMP_ALLOW_SUPERBLOCKS);

      Lsuper_switch_optimization(fn);

      STAT_DUMP ();
      L_set_hb_no_fallthru_flag (fn);
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-3");

  /*
   * Initial live variable dataflow run for use in superscalar optimizations.
   * From this point forward, optimizations that corrupt dataflow information
   * are expected to call L_invalidate_dataflow() so that 
   * L_update_dataflow(...) will know to rerun dataflow as appropriate. 
   * -- JWS 3/22/98
   */
  L_do_flow_analysis (fn, LIVE_VARIABLE | DOMINATOR_CB);

  /*
   *  Loop peeling optimization
   */
  if (Lsuper_do_peel_opt && !Lsuper_trace_mode)
    {
      STAT_INIT ("Lsuper_peel_accumulator_expansion", fn);
      Lsuper_peel_accumulator_expansion (fn);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-4");



  /*
   *  Classic local optimization
   */
  if (Lsuper_do_classic_opti)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("Lsuper_local_code_optimization (1)", fn);
      Lsuper_local_code_optimization (fn, 0);
      STAT_DUMP ();

      L_set_hb_no_fallthru_flag (fn);
    }



//KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK
 if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-5");

  /*
   *  Strength reduction optimization
   */
  if (Lsuper_do_str_red && !Lsuper_trace_mode)
    {
      STAT_INIT ("Lsuper_strength_reduction", fn);
      Lsuper_strength_reduction (fn);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-6");

//KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK
  /*
   *  Multi-way branch opti
   */
  if (Lsuper_do_multiway_branch_opti && !Lsuper_trace_mode)
    {
      STAT_INIT ("Lsuper_multiway_branch_expansion", fn);
      L_multiway_branch_expansion (fn);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-7");

  /*
   *  Inner loop detection
   */
  if (Lsuper_do_loop_classic_opti | Lsuper_do_loop_unroll)
    {
      L_reset_loop_headers (fn);
      L_inner_loop_detection (fn, 1);
    }

  /*
   *  Classic loop optimization / Classic local optimization
   */
  if (Lsuper_do_loop_classic_opti)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);

      for (i = 0; i < REPEAT_LOOP_OPTI; i++)
        {
          int local, loop, mig, change;

          local = loop = change = mig = 0;

          L_update_flow_analysis (fn, LIVE_VARIABLE);

          STAT_INIT ("Lsuper_loop_classic_optimization", fn);
          loop = Lsuper_loop_classic_optimization (fn);
          STAT_DUMP ();

          if (Lsuper_do_classic_opti && !Lsuper_trace_mode)
            {
              L_do_flow_analysis (fn, LIVE_VARIABLE);
              STAT_INIT ("Lsuper_local_code_optimization (2)", fn);
              local = Lsuper_local_code_optimization (fn, 0);
              STAT_DUMP ();
            }

          if (Lsuper_do_op_migration && Lsuper_trace_mode)
            {
              L_do_flow_analysis (fn, LIVE_VARIABLE);
              STAT_INIT ("Lsuper_operation_migration", fn);
              mig = Lsuper_operation_migration (fn);
              STAT_DUMP ();
            }

          change = local + loop + mig;
          if (change == 0)
            break;
        }
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-8");

  /*
   *  Mark loops for software pipelining becasue they will be optimized
   *  differently than other loops.  They are unrolled a different amount.
   *  No renaming with copy is done, because modulo variable expansion will
   *  achieve the same effect without adding extra mov instructions.  
   *  No compensation code is allowed in the inner loop cb after the loop 
   *  back branch.  Headers of loops marked for software pipelining are
   *  not expanded into other cb's.
   */
  if (Lsuper_mark_softpipe_loops && !Lsuper_trace_mode)
    {
      L_Inner_Loop *lp;
      L_Attr *attr;
      int counted_loop;

      {
	L_Cb *cb;
	L_Flow *fl;
	L_Oper *br;
	for (cb = fn->first_cb; cb; cb = cb->next_cb)
	  {
	    if (cb->weight < 1.0)
	      continue;

	    for (fl = cb->dest_flow; fl; fl = fl->next_flow)
	      {
		if (fl->dst_cb != cb)
		  continue;

		if (!(br = L_find_branch_for_flow (cb, fl)))
		  break;

		if (fl->weight < (0.25 * cb->weight))
		  continue;

		if (br->next_op)
		  {
		    L_warn ("Splitting cb %d in markpipe", cb->id);
		    L_split_cb_after (fn, cb, br);
		    break;
		  }
	      }
	  }
      }

      if (Lpipe_print_marking_statistics)
        fprintf (softpipe_statfile, "\n..Analyze loops for software "
                 "pipelining in function (%s)\n", fn->name);

      L_reset_loop_headers (fn);
      L_inner_loop_detection (fn, 1);

      for (lp = fn->first_inner_loop; lp != NULL; lp = lp->next_inner_loop)
        {
	  if (lp->feedback_op != lp->cb->last_op)
	    L_split_cb_after (fn, lp->cb, lp->feedback_op);
	}

      L_partial_dead_code_removal (L_fn);
      L_do_flow_analysis (fn, LIVE_VARIABLE | SUPPRESS_PG);

      for (lp = fn->first_inner_loop; lp != NULL; lp = lp->next_inner_loop)
        {
	  if (!Lpipe_is_OK_softpipe_loop (lp, softpipe_statfile, 
					  &counted_loop, 1))
	    continue;

	  if (Lpipe_mark_potential_softpipe_loops)
	    {
	      attr = L_new_attr ("L_CB_SOFTPIPE", 0);
	      lp->cb->attr = L_concat_attr (lp->cb->attr, attr);
	    }
	  else
	    {
	      lp->cb->flags = L_SET_BIT_FLAG (lp->cb->flags, 
					      L_CB_SOFTPIPE);
	    }
	      
	  if (Lpipe_print_marking_statistics)
	    fprintf (softpipe_statfile, "$ Marking loop with header cb "
		     "%d for software pipelining\n", lp->cb->id);
        }
    }

//checkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk

  /*
   *  Loop unrolling
   */

  /* Trace mode unrolling is constrained to 1x unroll */

  if (Lsuper_do_loop_unroll)
    {
      if (!Lsuper_trace_mode)
        {
          STAT_INIT ("Lsuper_loop_unrolling", fn);
          Lsuper_loop_unrolling (fn);
          STAT_DUMP ();
          /* unroll messes loop info up, so redo it! */
          L_reset_loop_headers (fn);
          L_inner_loop_detection (fn, 1);
          if (Lsuper_unroll_with_remainder_loop)
            {
              Lsuper_create_remainder_loops (fn);

              /* Invalidate dataflow.
               * For efficiency, this and the loop detection done
               * below should only be done when remainder loops are actually
               * created.  For now, be safe but inefficent. -ITI/JCG 9/99
               */
              L_invalidate_dataflow ();

              /* creating remainder loop messes loop info up, so redo it! */
              L_reset_loop_headers (fn);
              L_inner_loop_detection (fn, 1);
            }
        }
      else
        {
          L_generate_pre_post_inc_ops (fn);
          L_sb_loop_preprocess2 (fn);

	  for (i = 0; i < REPEAT_LOOP_OPTI; i++)
	    {
	      int change, local, loop, mig;
	      
	      local = loop = mig = 0;
	      L_do_flow_analysis (fn, LIVE_VARIABLE);
	      
	      loop = L_sb_loop_optimization2 (fn);
		  
	      if (Lsuper_do_classic_opti)
		{
		  L_do_flow_analysis (fn, LIVE_VARIABLE);

		  STAT_INIT ("Lsuper_local_code_optimization (lp)", fn);
		  local = Lsuper_local_code_optimization (fn, 0);
		  STAT_DUMP ();
		}
	      if (Lsuper_do_op_migration)
		{
		  L_do_flow_analysis (fn, LIVE_VARIABLE);

		  STAT_INIT ("Lsuper_operation_migration (lp)", fn);
		  mig = Lsuper_operation_migration (fn);
		  STAT_DUMP ();
		}

	      change = local + loop + mig;

	      if (!change)
		break;
	    }
          L_sb_loop_postprocess (fn);
        }
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-9");

  /*
   *  Accumulator/Induction variable expansion
   */
  if (Lsuper_do_acc_exp && Lsuper_do_loop_unroll && !Lsuper_trace_mode)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);

      STAT_INIT ("Lsuper_accumulator_expansion", fn);
      Lsuper_accumulator_expansion (fn);
      STAT_DUMP ();

      L_update_flow_analysis (fn, LIVE_VARIABLE);

      STAT_INIT ("Lsuper_induction_var_optimization", fn);
      Lsuper_induction_var_optimization (fn);
      STAT_DUMP ();
    }

  /* Make sure compensation code from accumulator expansion is put
     into a separate cb for loops marked for software pipelining. */
  if (Lsuper_push_comp_code_out_of_softpipe_loops && !Lsuper_trace_mode)
    {
      STAT_INIT ("Lsuper_move_comp_code_out_of_softpipe_loops", fn);
      Lsuper_move_comp_code_out_of_softpipe_loops (fn);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-10");

  /*
   *  Repeat classic local optimization again
   */
  if (Lsuper_do_classic_opti && !Lsuper_trace_mode)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("Lsuper_local_code_optimization (3)", fn);
      Lsuper_local_code_optimization (fn, 0);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-11");

  /*
   *  Operation migration
   */
  if (Lsuper_do_op_migration && !Lsuper_trace_mode)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("Lsuper_operation_migration", fn);
      Lsuper_operation_migration (fn);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-12");

  /*
   *  Repeat jump optimization, allow loops to be expanded
   */
  if (Lsuper_do_jump_opti)
    {
      L_update_flow_analysis (fn, DOMINATOR_CB);
      L_inner_loop_detection (fn, 0);
      STAT_INIT ("Lsuper_jump_optimization", fn);
      Lsuper_jump_optimization (fn,
                                L_JUMP_ALLOW_SUPERBLOCKS |
                                L_JUMP_ALLOW_BACKEDGE_EXP |
                                L_JUMP_ALLOW_LOOP_BODY_EXP);
      STAT_DUMP ();
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      L_set_hb_no_fallthru_flag (fn);
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-13");

  /*
   *  Repeat classic local optimization again
   */
  if (Lsuper_do_classic_opti)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);

      STAT_INIT ("Lsuper_local_code_optimization (4)", fn);
      Lsuper_local_code_optimization (fn, 0);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-14");

  /*
   *  Renaming, critical path reduction
   *    a. register renaming itself is free, so do it first
   *    b. do special promotion/renaming for unrolled loops, do migration
   *       immediately here since it likely to be effective.
   *    c. do generic promotion/renaming for blocks.
   */

  if (Lsuper_do_rename_disjvreg)
    {
      L_rename_disjoint_virtual_registers (fn, NULL);
    }

  if (Lsuper_do_critical_path_red && !Lsuper_trace_mode)
    {
      STAT_INIT ("Lsuper_critical_path_red", fn);

      L_do_flow_analysis (fn, LIVE_VARIABLE);
      Lsuper_unrolled_loop_renaming (fn);

      if (Lsuper_do_pred_promotion)
	{
	  L_do_flow_analysis (fn, LIVE_VARIABLE|REACHING_DEFINITION);
	  L_predicate_promotion (fn, 0);
	}

      L_do_flow_analysis (fn, LIVE_VARIABLE|REACHING_DEFINITION);
      Lsuper_critical_path_reduction (fn);

      if (Lsuper_do_pred_promotion)
	{
	  L_do_flow_analysis (fn, LIVE_VARIABLE|REACHING_DEFINITION);
	  L_predicate_promotion (fn, 1);
	}

      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-15");

  /*
   *  Operation migration again
   */
  if (Lsuper_do_op_migration && Lsuper_do_critical_path_red &&
      !Lsuper_trace_mode)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("Lsuper_operation_migration (2)", fn);
      Lsuper_operation_migration (fn);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-16");

  /*
   *  Repeat jump optimization again, allow loops to be expanded
   */
  if (Lsuper_do_jump_opti && !Lsuper_trace_mode)
    {
      L_update_flow_analysis (fn, DOMINATOR_CB);
      L_inner_loop_detection (fn, 0);
      STAT_INIT ("Lsuper_jump_optimization (2)", fn);
      Lsuper_jump_optimization (fn,
                                L_JUMP_ALLOW_SUPERBLOCKS |
                                L_JUMP_ALLOW_BACKEDGE_EXP |
                                L_JUMP_ALLOW_LOOP_BODY_EXP);
      STAT_DUMP ();
      L_set_hb_no_fallthru_flag (fn);
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-17");

  /*
   *  Repeat classic local optimization last time, omit rev copy prop,
   *  it undoes critical path reduction!!!
   */
  if (Lsuper_do_classic_opti && !Lsuper_trace_mode)
    {
      L_do_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("Lsuper_local_code_optimization (4)", fn);
      Lsuper_local_code_optimization (fn, 1);
      STAT_DUMP ();
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-18");

  L_delete_unreachable_blocks (fn);

  /*
   *  Branch combining (use OR type predicates to combine unlikely brs)
   */
  if (Lsuper_do_branch_combining && Lsuper_pred_exec_support &&
      !Lsuper_trace_mode)
    {
      /*
       * Compute oper weights for PRIC
       */
      L_compute_oper_weight (fn, 0, 1);

      L_update_flow_analysis (fn, LIVE_VARIABLE);
      STAT_INIT ("Lsuper_combine_branches", fn);
      Lsuper_combine_branches (fn);
      STAT_DUMP ();

      if (Lsuper_do_pred_promotion)
	{
	  STAT_INIT ("Lpredicate_promotion (2)", fn);
	  L_do_flow_analysis (fn, LIVE_VARIABLE|REACHING_DEFINITION);
	  L_predicate_promotion (fn, 1);
	  STAT_DUMP ();
	}
    }

  if (Lsuper_debug_dump_intermediates)
    DB_spit_func(fn,"LSUPER-19");

  /*
   *  Extra cleanup phase to make sure no unreachable blocks in code
   */
  Lsuper_cleanup_jump_optimization (fn, 0);




  /*
   * Recombine predicate defines as conds permit.
   */
  L_combine_pred_defines (fn);

  L_mark_superblocks (fn);
  L_set_hb_no_fallthru_flag (fn);
  LB_code_layout (fn);
  L_set_hb_no_fallthru_flag (fn);

  L_check_func (fn);

  /*
   *  Mark any instrs as safe, if optimizations exposed more safe ops.
   *  Set function MASK_PE flag if any instructions are flagged.
   */
  L_mark_safe_instructions (fn);
  L_set_function_mask_flag (fn);

  /*
   *   Free up allocaton pools used by Lopti functions
   */
  Lopti_deinit ();

//  if(strcmp(fn->name,"_main")==0){
//                    	 // find_flow_wight(fn,NULL);
//                    	  Ldot_display_cfg(fn,"t3.dot",0);
//                    	  FILE * file=fopen("main3.txt","w");
////                    	  		L_print_func(file,fn);
//                    	  		fclose(file);
//          }


//Ldot_display_cfg(fn,"tsuper.dot",0);



}
#endif
if(strcmp(fn->name,"_main")==0)
	Ldot_display_cfg(fn,"tsuper.dot",0);
  return;
}
