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
 *      File :          l_codegen.c
 *      Description :   main routine for traditional optimizer
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Paul Chang.
 *
 *      (C) Copyright 1990, Scott Mahlke & Pohua Chang.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/14/02 REK This module uses the following arguments:
 *              -Fallow_jump_expansion_of_pcode_loops=(yes|no)
 *              -Fdebug_global_opti=(yes|no)
 *              -Fdebug_jump_opti=(yes|no)
 *              -Fdebug_local_opti=(yes|no)
 *              -Fdebug_loop_opti=(yes|no)
 *              -Fdebug_memflow=(yes|no)
 *              -Fdo_benchmark_specific_opti=(yes|no)
 *              -Fdo_classify_branches=(yes|no)
 *              -Fdo_code_layout=(yes|no)
 *              -Fdo_complex_ind_elim=(yes|no)
 *              -Fdo_dead_loop_rem=(yes|no)
 *              -Fdo_global_common_sub_elim=(yes|no)
 *              -Fdo_global_constant_prop=(yes|no)
 *              -Fdo_global_copy_prop=(yes|no)
 *              -Fdo_global_dead_code_removal=(yes|no)
 *              -Fdo_global_dead_if_then_else_rem=(yes|no)
 *              -Fdo_global_elim_boolean_ops=(yes|no)
 *              -Fdo_global_mem_copy_prop=(yes|no)
 *              -Fdo_global_opti=(yes|no)
 *              -Fdo_global_red_load_elim=(yes|no)
 *              -Fdo_global_red_store_elim=(yes|no)
 *              -Fdo_jump_block_merge=(yes|no)
 *              -Fdo_jump_br_swap=(yes|no)
 *              -Fdo_jump_br_target_expansion=(yes|no)
 *              -Fdo_jump_br_to_next_block=(yes|no)
 *              -Fdo_jump_br_to_same_target=(yes|no)
 *              -Fdo_jump_br_to_uncond_br=(yes|no)
 *              -Fdo_jump_combine_labels=(yes|no)
 *              -Fdo_jump_dead_block_elim=(yes|no)
 *              -Fdo_jump_opti=(yes|no)
 *              -Fdo_local_branch_fold=(yes|no)
 *              -Fdo_local_code_motion=(yes|no)
 *              -Fdo_local_common_sub_elim=(yes|no)
 *              -Fdo_local_constant_comb=(yes|no)
 *              -Fdo_local_constant_fold=(yes|no)
 *              -Fdo_local_constant_prop=(yes|no)
 *              -Fdo_local_copy_prop=(yes|no)
 *              -Fdo_local_dead_code_rem=(yes|no)
 *              -Fdo_local_mem_copy_prop=(yes|no)
 *              -Fdo_local_operation_fold=(yes|no)
 *              -Fdo_local_opti=(yes|no)
 *              -Fdo_local_reduce_logic=(yes|no)
 *              -Fdo_local_red_load_elim=(yes|no)
 *              -Fdo_local_red_store_elim=(yes|no)
 *              -Fdo_local_register_rename=(yes|no)
 *              -Fdo_local_remove_sign_ext=(yes|no)
 *              -Fdo_local_rev_copy_prop=(yes|no)
 *              -Fdo_local_strength_red=(yes|no)
 *              -Fdo_local_strength_red_for_signed_div_rem=(yes|no)
 *              -Fdo_local_operation_cancel=(yes|no)
 *              -Fdo_local_op_breakdown=(yes|no)
 *              -Fdo_local_op_recombine=(yes|no)
 *              -Fdo_longword_loop_opti=(yes|no)
 *              -Fdo_loop_br_simp=(yes|no)
 *              -Fdo_loop_global_var_mig=(yes|no)
 *              -Fdo_loop_ind_var_str_red=(yes|no)
 *              -Fdo_loop_ind_var_reinit=(yes|no)
 *              -Fdo_loop_inv_code_rem=(yes|no)
 *              -Fdo_loop_opti=(yes|no)
 *              -Fdo_mark_incoming_parms=(yes|no)
 *              -Fdo_mark_memory_labels=(yes|no)
 *              -Fdo_mark_sync_jsrs=(yes|no)
 *              -Fdo_mark_trivial_safe_ops=(yes|no)
 *              -Fdo_mark_trivial_sef_jsrs=(yes|no)
 *              -Fdo_memflow_multistore_load=(yes|no)
 *              -Fdo_memflow_opti=(yes|no)
 *              -Fdo_merge_unification=(yes|no)
 *              -Fdo_post_inc_conv=(yes|no)
 *              -Fdo_remove_decidable_cond_branches=(yes|no)
 *              -Fdo_split_branches=(yes|no)
 *              -Fdo_split_unification=(yes|no)
 *              -Fignore_sync_arcs_for_loop_inv_migration=(yes|no)
 *              -Fignore_sync_arcs_for_red_elim=(yes|no)
 *              -Fmemflow_bypass_jsr=<int>
 *              -Fmemflow_bypass_load=<int>
 *              -Fmemflow_bypass_store=<int>
 *              -Fmemflow_bypass_total=<int>
 *              -Fonly_lvl1_for_zero_weight_fn=(yes|no)
 *              -Fopti_level=<int>
 *              -Fpred_promotion_level=<int>
 *              -Fpreserve_loop_var=(yes|no)
 *              -Fprint_opti_breakdown=(yes|no)
 *              -Fprint_opti_count=(yes|no)
 *              -Fstore_migration_mode=(yes|no)
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

static void
process_input (void)
{
  switch (L_token_type)
    {
    case L_INPUT_EOF:
    case L_INPUT_MS:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_RESERVE:
    case L_INPUT_GLOBAL:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      /* LCW - new tokens for preserving debugging info - 4/21/96 */
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
      L_repair_hashtbl (L_data);
      L_print_data (L_OUT, L_data);
      L_delete_data (L_data);
      break;
    case L_INPUT_FUNCTION:
      L_define_fn_name (L_fn->name);
      L_code_optimize (L_fn);
      L_print_func (L_OUT, L_fn);
      L_delete_func (L_fn);
      break;
    default:
      L_punt ("process_input: illegal token");
    }
}

void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{

  Lopti_init ();
  LB_block_init (command_line_macro_list);

  if (L_file_processing_model == L_FILE_MODEL_FILE)
    {
      L_open_input_file (L_input_file);
      while (L_get_input () != L_INPUT_EOF)
        {
          process_input ();
        }
      L_close_input_file (L_input_file);
    }
  else if ((L_file_processing_model == L_FILE_MODEL_LIST) ||
           (L_file_processing_model == L_FILE_MODEL_EXTENSION))
    {
      L_create_filelist ();
      while ((L_file = List_next (L_filelist)))
        {
          L_input_file = L_file->input_file;
          L_output_file = L_file->output_file;

          L_OUT = L_open_output_file (L_output_file);
          L_generation_info_printed = 0;
          L_open_input_file (L_input_file);
          while (L_get_input () != L_INPUT_EOF)
            {
              process_input ();
            }
          L_close_input_file (L_input_file);
          L_close_output_file (L_OUT);
        }
      L_delete_filelist ();
    }

  Lopti_deinit ();

}
