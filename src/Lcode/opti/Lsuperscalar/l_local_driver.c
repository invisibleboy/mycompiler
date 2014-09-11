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
 *      File :          l_local_driver.c
 *      Description :   Classical opti applied to superblocks
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang, Wen-mei Hwu
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define REPEAT_LOCAL_OPTI               10

/*
 *      Driver for cb level optimizations for Lsuperscalar
 */

void
L_normalize_ops (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
          if (L_int_sub_opcode (oper) && L_is_int_constant (oper->src[1]))
            {
	      oper->src[1]->value.i = - oper->src[1]->value.i;
              L_change_opcode (oper, L_inverse_arithmetic (oper->opc));
            }
        }
    }
  return;
}


int
Lsuper_local_code_optimization (L_Func * fn, int renaming_done)
{
  int i, j, opti_applied = 0 , global_dead_total;
  L_Cb *cb;

  L_normalize_ops (fn);

  STAT_INIT ("Lsuper_local_code_optimization", fn);

  L_partial_dead_code_removal (fn);

  L_do_flow_analysis (fn, LIVE_VARIABLE);
#if 0
  printf("Beginning new run of Lsuperscalar.\n");
#endif
  for (i = 0; i < REPEAT_LOCAL_OPTI; i++)
    {
      int global_dead;
#if 0
      printf("Beginning iteration %d of Lsuperscalar loop.\n", i);
#endif

      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
        {
          /* check for empty cb */
          if (!cb->first_op)
            continue;

          /* REH 9/95 - Don't perform local optimizations within */
          /*   region boundary cb's                              */
          if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
            continue;

          for (j = 0; j < REPEAT_LOCAL_OPTI; j++)
            {
              int change, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, 
		c12, c13, c14, c15, c17, c18, c19, use_sync_arcs = 0;
              c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = c9 = c10 = c11 = c12 =
                c13 = c14 = c15 = c17 = c18 = c19 = 0;

              if (Lsuper_do_const_prop)
                {
                  c1 = L_local_constant_propagation (cb, 0);
                  STAT_COUNT ("L_local_constant_propagation", c1, cb);
                }

              if (Lsuper_do_rev_copy_prop && !renaming_done)
                {
		  c2 = L_local_rev_copy_propagation (cb);
		  STAT_COUNT ("L_local_rev_copy_propagation", c2, cb);
                }

              if (Lsuper_do_copy_prop)
                {
                  c3 = L_local_copy_propagation (cb);
                  STAT_COUNT ("L_local_copy_propagation", c3, cb);
                }

              if (Lsuper_do_mem_copy_prop)
                {
                  c4 = L_local_memory_copy_propagation (cb);
                  STAT_COUNT ("L_local_memory_copy_propagation_total", c4, cb);
                }

              if (Lsuper_do_common_sub)
                {
                  c5 = L_local_common_subexpression (cb, 0);
                  STAT_COUNT ("L_local_common_subexpression_total", c5, cb);
                }

	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{

		  use_sync_arcs = L_use_sync_arcs;
		  L_use_sync_arcs = 0;
		}

              if (Lsuper_do_red_load)
                {
                  c6 = L_local_redundant_load (cb);
                  STAT_COUNT ("L_local_redundant_load", c6, cb);
                }

              if (Lsuper_do_red_store)
                {
                  c7 = L_local_redundant_store (cb);
                  STAT_COUNT ("L_local_redundant_store", c7, cb);
                }

	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{
		  L_use_sync_arcs = use_sync_arcs;
		}

              if (Lsuper_do_const_comb)
                {
                  c8 = L_local_constant_combining (cb);
                  STAT_COUNT ("L_local_constant_combining_total", c8, cb);
                }

              if (Lsuper_do_const_fold)
                {
                  c9 = L_local_constant_folding (cb);
                  STAT_COUNT ("L_local_constant_folding_total", c9, cb);
                }

              if (Lsuper_do_str_red)
                {
                  c10 = L_local_strength_reduction (cb);
                  STAT_COUNT ("L_local_strength_reduction_total", c10, cb);
                }

              if (Lsuper_do_br_fold)
                {
                  c11 = L_local_branch_folding (cb);
                  STAT_COUNT ("L_local_branch_folding", c11, cb);

                  /* control structure modified so redo flow analysis */
                  if (c11 != 0)
		    L_do_flow_analysis (fn, LIVE_VARIABLE);
                  STAT_COUNT ("L_local_branch_folding_total", c11, cb);
                }

              if (Lsuper_do_dead_code)
                {
                  c12 = L_local_dead_code_removal (cb);
                  STAT_COUNT ("L_local_dead_code_removal_total", c12, cb);
                }

              if (Lsuper_do_code_motion)
                {
                  c13 = L_local_code_motion (cb);
                  STAT_COUNT ("L_local_code_motion_total", c13, cb);
                }

              if (Lsuper_do_op_fold)
                {
                  c14 = L_local_operation_folding (cb);
                  STAT_COUNT ("L_local_operation_folding_total", c14, cb);
                }

              if (Lsuper_do_op_cancel)
                {
                  c17 = L_local_operation_cancellation (cb);
                  STAT_COUNT ("L_local_operation_cancellation_total", c17, cb);
                }

              if (Lsuper_do_remove_sign_ext)
                {
                  c18 = L_local_remove_sign_extension (cb);
                  STAT_COUNT ("L_local_remove_sign_extension", c18, cb);
                }

              if (Lsuper_do_reduce_logic)
                {
                  c19 = L_local_logic_reduction (cb);
                  STAT_COUNT ("L_local_logic_reduction_total", c19, cb);
                }

              change = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 +
                c11 + c12 + c13 + c14 + c15 + c17 + c18 + c19;

              opti_applied += change;
              if (!change)
                break;
            }
        }

      global_dead_total = L_partial_dead_code_removal (fn);

      if (Lsuper_do_dead_code)
        {
          L_do_flow_analysis (fn, LIVE_VARIABLE);
          for (cb = fn->first_cb; cb; cb = cb->next_cb)
            {
              L_local_pred_dead_code_removal (cb);
              global_dead = L_global_dead_code_removal (cb);
              global_dead_total += global_dead;
              STAT_COUNT ("L_global_dead_code_removal", global_dead, cb);
            }
        }
      if (!global_dead_total)
        break;
    }


  if (opti_applied)
    L_invalidate_dataflow ();

  return opti_applied;
}

int
Lsuper_operation_migration (L_Func * fn)
{
  int count = 0;
  L_Cb *cb;

  if (!Lsuper_do_local_op_mig)
    return 0;

  if (Lsuper_debug_op_migration)
    fprintf (stderr, "Enter oper migration...\n");

  L_normalize_ops (fn);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* REH 9/95 - Don't perform local optimizations within */
      /*   region boundary cb's                              */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

      count += L_local_operation_migration (cb, 0);
    }
  return count;
}
