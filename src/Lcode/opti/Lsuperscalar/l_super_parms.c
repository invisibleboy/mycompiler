/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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
 *      File :          l_super_parms.h
 *      Description :   Superscalar optimization parms and config
 *      Author :        Scott Mahlke
 *
 *      (C) Copyright 1990, Scott Mahlke
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

#include "l_superscalar.h"

int Lsuper_do_sb_formation = 1;
int Lsuper_do_only_sb_formation = 0;    /* Turns off all optis! -JCG 6/99 */
int Lsuper_mark_softpipe_loops = 0;     /* mark loops for software 
                                           pipelining */
int Lsuper_do_jump_opti = 1;
int Lsuper_do_dead_block = 1;
int Lsuper_do_branches_to_next_block = 1;
int Lsuper_do_branches_to_same_target = 1;
int Lsuper_do_branches_to_uncond_branch = 1;
int Lsuper_do_merge = 1;
int Lsuper_do_combine_labels = 1;
int Lsuper_do_branch_target_exp = 1;
int Lsuper_do_branch_swap = 1;
int Lsuper_do_branch_pred = 1;
int Lsuper_do_classic_opti = 1;
int Lsuper_do_const_prop = 1;
int Lsuper_do_rev_copy_prop = 1;
int Lsuper_do_copy_prop = 1;
int Lsuper_do_mem_copy_prop = 1;
int Lsuper_do_common_sub = 1;
int Lsuper_do_red_load = 1;
int Lsuper_do_red_store = 1;
int Lsuper_do_local_str_red = 1;
int Lsuper_do_const_comb = 1;
int Lsuper_do_const_fold = 1;
int Lsuper_do_br_fold = 1;
int Lsuper_do_dead_code = 1;
int Lsuper_do_code_motion = 1;
int Lsuper_do_op_fold = 1;
int Lsuper_do_op_cancel = 1;
int Lsuper_do_local_op_mig = 1;
int Lsuper_do_remove_sign_ext = 1;
int Lsuper_do_reduce_logic = 1;
int Lsuper_do_str_red = 1;
int Lsuper_do_multiway_branch_opti = 1;
int Lsuper_do_loop_classic_opti = 1;
int Lsuper_do_loop_inv_code_rem = 1;
int Lsuper_do_loop_global_var_mig = 1;
int Lsuper_do_loop_op_fold = 1;
int Lsuper_do_loop_dead_code = 1;
int Lsuper_do_loop_ind_var_elim = 1;
int Lsuper_do_loop_ind_var_elim2 = 1;
int Lsuper_do_loop_ind_reinit = 1;
int Lsuper_do_loop_post_inc_conv = 1;
int Lsuper_do_loop_unroll = 1;
int Lsuper_unroll_with_remainder_loop = 0;      /* when profitable for counted 
                                                   loops, remove copies of loop 
                                                   back branch and create 
                                                   separate loop to execute 
                                                   the remaining iterations */
int Lsuper_unroll_pipelined_loops = 0;  /* enable unrolling for software
                                           pipelined loops.  Will only work
                                           with remainder loop. */
int Lsuper_fixed_length_unroll = 0;     /* Support for fixed-count loop
                                           unrolling (after Bob McGowan) */
int Lsuper_do_acc_exp = 1;
int Lsuper_do_float_acc_exp = 1;
    /* Make sure compensation code from accumulator expansion is put
       into a separate cb for loops marked for software pipelining. */
int Lsuper_push_comp_code_out_of_softpipe_loops = 0;

int Lsuper_do_peel_opt = 1;
int Lsuper_do_op_migration = 1;
int Lsuper_do_critical_path_red = 1;
int Lsuper_do_rename_disjvreg = 1;
int Lsuper_do_branch_combining = 0;

/* Added to allow user more hand-tuned control of superblock formation
 * and ILP optimization. -JCG 6/99
 */
char *Lsuper_prevent_superblock_functions = "";
int Lsuper_verbose_prevent_superblock = 1;
char *Lsuper_prevent_ILP_opti_functions = "";
int Lsuper_verbose_prevent_ILP_opti = 1;

/* more detailed switches */
int Lsuper_do_post_inc_conv = 0;
int Lsuper_issue_rate = 8;

/* DIA, JWS 13 Aug 98
 * Height / Operation Count Exchange Parameter
 * --------------------------------------------------------
 * issue_rate > height_count_crossover
 *   --> reduce height (traditonal Lsuperscalar behavior)
 * issue rate <= height_count crossover
 *   --> reduce dyn op cnt (traditional Ltrace behavior)
 */
int Lsuper_height_count_crossover = 2;

int Lsuper_pred_exec_support = 0;       /* optimizations specific for pred
                                           exec controlled by this flag */
int Lsuper_do_pred_promotion = 1;
int Lsuper_allow_extra_unrolling_for_small_loops = 1;
int Lsuper_max_unroll_allowed = 16;
int Lsuper_max_unroll_op_count = 256;
int Lsuper_unroll_only_marked_loops = 0;
int Lsuper_allow_backedge_exp = 1;
int Lsuper_allow_expansion_of_loops = 1;

int Lsuper_max_number_of_predicates = -1;
int Lsuper_store_migration_mode = L_STORE_MIGRATION_NO_PRED;

/* debugging parameters */
int Lsuper_debug_str_red = 0;
int Lsuper_debug_multiway_branch_opti = 0;
int Lsuper_debug_loop_classic_opti = 0;
int Lsuper_debug_loop_unroll = 0;
int Lsuper_debug_acc_exp = 0;
int Lsuper_debug_peel_opt = 0;
int Lsuper_debug_op_migration = 0;
int Lsuper_debug_critical_path_red = 0;

int Lsuper_debug_dump_intermediates = 0;

/* file for statistics on loops marked for pipelining */
FILE *softpipe_statfile;

/* Used in l_superscalar.c -JCG 6/99 */
Func_Name_List *Lsuper_prevent_superblock_list = NULL;
Func_Name_List *Lsuper_prevent_ILP_opti_list = NULL;

/*
 *      Read module specific parameters
 */

void
L_read_parm_lsuperscalar (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "do_sb_formation", &Lsuper_do_sb_formation);
  L_read_parm_b (ppi, "do_only_sb_formation", &Lsuper_do_only_sb_formation);
  L_read_parm_b (ppi, "mark_softpipe_loops", &Lsuper_mark_softpipe_loops);
  L_read_parm_b (ppi, "do_jump_opti", &Lsuper_do_jump_opti);
  L_read_parm_b (ppi, "do_classic_opti", &Lsuper_do_classic_opti);
  L_read_parm_b (ppi, "do_str_red", &Lsuper_do_str_red);
  L_read_parm_b (ppi, "do_multiway_branch_opti",
                 &Lsuper_do_multiway_branch_opti);
  L_read_parm_b (ppi, "do_loop_classic_opti", &Lsuper_do_loop_classic_opti);
  L_read_parm_b (ppi, "do_loop_unroll", &Lsuper_do_loop_unroll);
  L_read_parm_b (ppi, "unroll_with_remainder_loop",
                 &Lsuper_unroll_with_remainder_loop);
  L_read_parm_b (ppi, "unroll_pipelined_loops",
                 &Lsuper_unroll_pipelined_loops);
  L_read_parm_b (ppi, "fixed_length_unroll", &Lsuper_fixed_length_unroll);
  L_read_parm_b (ppi, "do_acc_exp", &Lsuper_do_acc_exp);
  L_read_parm_b (ppi, "do_float_acc_exp", &Lsuper_do_float_acc_exp);
  L_read_parm_b (ppi, "push_comp_code_out_of_softpipe_loops",
                 &Lsuper_push_comp_code_out_of_softpipe_loops);
  L_read_parm_b (ppi, "do_peel_opt", &Lsuper_do_peel_opt);
  L_read_parm_b (ppi, "do_op_migration", &Lsuper_do_op_migration);
  L_read_parm_b (ppi, "do_rename_disjvreg", &Lsuper_do_rename_disjvreg);
  L_read_parm_b (ppi, "do_critical_path_red", &Lsuper_do_critical_path_red);
  L_read_parm_b (ppi, "do_branch_combining", &Lsuper_do_branch_combining);

  L_read_parm_b (ppi, "do_post_inc_conv", &Lsuper_do_post_inc_conv);
  L_read_parm_i (ppi, "issue_rate", &Lsuper_issue_rate);
  L_read_parm_i (ppi, "height_count_crossover",
                 &Lsuper_height_count_crossover);
  L_read_parm_b (ppi, "pred_exec_support", &Lsuper_pred_exec_support);
  L_read_parm_b (ppi, "do_pred_promotion", &Lsuper_do_pred_promotion);
  L_read_parm_b (ppi, "allow_extra_unrolling_for_small_loops",
                 &Lsuper_allow_extra_unrolling_for_small_loops);
  L_read_parm_i (ppi, "max_unroll_allowed", &Lsuper_max_unroll_allowed);
  L_read_parm_i (ppi, "max_unroll_op_count", &Lsuper_max_unroll_op_count);
  L_read_parm_b (ppi, "unroll_only_marked_loops",
                 &Lsuper_unroll_only_marked_loops);
  L_read_parm_b (ppi, "allow_backedge_exp", &Lsuper_allow_backedge_exp);
  L_read_parm_b (ppi, "allow_expansion_of_loops",
                 &Lsuper_allow_expansion_of_loops);

  L_read_parm_i (ppi, "max_number_of_predicates",
                 &Lsuper_max_number_of_predicates);
  L_read_parm_i (ppi, "store_migration_mode",
		 &Lsuper_store_migration_mode);
  L_read_parm_b (ppi, "do_super_dead_block", &Lsuper_do_dead_block);
  L_read_parm_b (ppi, "do_super_branches_to_next_block",
                 &Lsuper_do_branches_to_next_block);
  L_read_parm_b (ppi, "do_super_branches_to_same_target",
                 &Lsuper_do_branches_to_same_target);
  L_read_parm_b (ppi, "do_super_branches_to_uncond_branch",
                 &Lsuper_do_branches_to_uncond_branch);
  L_read_parm_b (ppi, "do_super_merge", &Lsuper_do_merge);
  L_read_parm_b (ppi, "do_super_combine_labels", &Lsuper_do_combine_labels);
  L_read_parm_b (ppi, "do_super_branch_target_exp",
                 &Lsuper_do_branch_target_exp);
  L_read_parm_b (ppi, "do_super_branch_swap", &Lsuper_do_branch_swap);
  L_read_parm_b (ppi, "do_super_branch_pred", &Lsuper_do_branch_pred);

  L_read_parm_b (ppi, "do_super_const_prop", &Lsuper_do_const_prop);
  L_read_parm_b (ppi, "do_super_rev_copy_prop", &Lsuper_do_rev_copy_prop);
  L_read_parm_b (ppi, "do_super_copy_prop", &Lsuper_do_copy_prop);
  L_read_parm_b (ppi, "do_super_mem_copy_prop", &Lsuper_do_mem_copy_prop);
  L_read_parm_b (ppi, "do_super_common_sub", &Lsuper_do_common_sub);
  L_read_parm_b (ppi, "do_super_red_load", &Lsuper_do_red_load);
  L_read_parm_b (ppi, "do_super_red_store", &Lsuper_do_red_store);
  L_read_parm_b (ppi, "do_super_local_str_red", &Lsuper_do_local_str_red);
  L_read_parm_b (ppi, "do_super_const_comb", &Lsuper_do_const_comb);
  L_read_parm_b (ppi, "do_super_const_fold", &Lsuper_do_const_fold);
  L_read_parm_b (ppi, "do_super_br_fold", &Lsuper_do_br_fold);
  L_read_parm_b (ppi, "do_super_dead_code", &Lsuper_do_dead_code);
  L_read_parm_b (ppi, "do_super_code_motion", &Lsuper_do_code_motion);
  L_read_parm_b (ppi, "do_super_op_fold", &Lsuper_do_op_fold);
  L_read_parm_b (ppi, "do_super_op_cancel", &Lsuper_do_op_cancel);
  L_read_parm_b (ppi, "do_super_local_op_mig", &Lsuper_do_local_op_mig);
  L_read_parm_b (ppi, "do_super_remove_sign_ext", &Lsuper_do_remove_sign_ext);
  L_read_parm_b (ppi, "do_super_reduce_logic", &Lsuper_do_reduce_logic);

  L_read_parm_b (ppi, "do_super_loop_inv_code_rem",
                 &Lsuper_do_loop_inv_code_rem);
  L_read_parm_b (ppi, "do_super_loop_global_var_mig",
                 &Lsuper_do_loop_global_var_mig);
  L_read_parm_b (ppi, "do_super_loop_op_fold", &Lsuper_do_loop_op_fold);
  L_read_parm_b (ppi, "do_super_loop_dead_code", &Lsuper_do_loop_dead_code);
  L_read_parm_b (ppi, "do_super_loop_ind_var_elim",
                 &Lsuper_do_loop_ind_var_elim);
  L_read_parm_b (ppi, "do_super_loop_ind_var_elim2",
                 &Lsuper_do_loop_ind_var_elim2);
  L_read_parm_b (ppi, "do_super_loop_ind_reinit", &Lsuper_do_loop_ind_reinit);
  L_read_parm_b (ppi, "do_super_loop_post_inc_conv",
                 &Lsuper_do_loop_post_inc_conv);

  L_read_parm_s (ppi, "prevent_superblock_functions", 
                 &Lsuper_prevent_superblock_functions);     /* JCG 6/99 */
  L_read_parm_b (ppi, "verbose_prevent_superblock", 
                 &Lsuper_verbose_prevent_superblock);       /* JCG 6/99 */
  L_read_parm_s (ppi, "prevent_ILP_opti_functions", 
                 &Lsuper_prevent_ILP_opti_functions);       /* JCG 6/99 */
  L_read_parm_b (ppi, "verbose_prevent_ILP_opti", 
                 &Lsuper_verbose_prevent_ILP_opti);         /* JCG 6/99 */

  L_read_parm_b (ppi, "debug_str_red", &Lsuper_debug_str_red);
  L_read_parm_b (ppi, "debug_multiway_branch_opti",
                 &Lsuper_debug_multiway_branch_opti);
  L_read_parm_b (ppi, "debug_loop_classic_opti",
                 &Lsuper_debug_loop_classic_opti);
  L_read_parm_b (ppi, "debug_loop_unroll", &Lsuper_debug_loop_unroll);
  L_read_parm_b (ppi, "debug_acc_exp", &Lsuper_debug_acc_exp);
  L_read_parm_b (ppi, "debug_peel_opt", &Lsuper_debug_peel_opt);
  L_read_parm_b (ppi, "debug_op_migration", &Lsuper_debug_op_migration);
  L_read_parm_b (ppi, "debug_critical_path_red", &Lsuper_debug_critical_path_red);
  L_read_parm_b (ppi, "debug_dump_intermediates", &Lsuper_debug_dump_intermediates);
  return;
}

void
Lsuper_initialize (Parm_Macro_List * command_line_macro_list)
{
  /* Load the parameters specific to Lcode code generation */
  L_load_parameters (L_parm_file, command_line_macro_list,
                     "(Lsuperscalar", L_read_parm_lsuperscalar);

  Lsuper_prevent_superblock_list =
    new_Func_Name_List (Lsuper_prevent_superblock_functions);
  Lsuper_prevent_ILP_opti_list =
    new_Func_Name_List (Lsuper_prevent_ILP_opti_functions);

  if (Lsuper_mark_softpipe_loops)
    {
      /* Load the parameters specific to the software pipelining library */
      L_load_parameters (L_parm_file, command_line_macro_list,
                         "(Lmarkpipe", L_read_parm_lmarkpipe);

      if (Lpipe_print_marking_statistics)
        {
          softpipe_statfile = fopen ("markpipe.stats", "a");
          if (softpipe_statfile == 0)
            L_punt ("Error while processing input file %s: can not append "
                    "to file " "markpipe.stats" "\n", L_input_file);
        }
    }

  /* Initializes lmdes as well as reads the Lblock parms. */

  LB_block_init (command_line_macro_list);
  SM_init (command_line_macro_list);

  /* Let users know if they have turned off all optis! -JCG 6/99 */
  if (Lsuper_do_only_sb_formation)
    {
      printf ("Note: Performing superblock formation only.  "
              "No ILP optis!\n");

      /* Sanity check */
      if (!Lsuper_do_sb_formation)
        {
          fprintf (stderr,
                   "*** Warning: Both superblock formation and ILP optis "
                   "turned off! ***\n");
        }
    }

  return;
}

void
Lsuper_cleanup (void)
{
  if (Lsuper_mark_softpipe_loops && Lpipe_print_marking_statistics)
    fclose (softpipe_statfile);

  /* Free lists */
  delete_Func_Name_List (Lsuper_prevent_superblock_list);
  delete_Func_Name_List (Lsuper_prevent_ILP_opti_list);

  return;
}
