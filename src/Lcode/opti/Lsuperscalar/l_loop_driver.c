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
 *      File :          l_loop_driver.c
 *      Description :   driver for classic loop optimizations for Lsuperscalar
 *      Info Needed :   inner loop detection
 *      Creation Date : July 1993
 *      Author :        Scott Mahlke, Wen-mei Hwu
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

/*
 *      Optimization switches
 */

#define L_LOOP_MIN_WEIGHT               10.0

/* From Ltrace */
#define L_LOOP_MIN_ITER_FOR_DUPL        1.5

L_Cb **L_duplicate_cb = NULL;

int
Lsuper_loop_classic_optimization (L_Func * fn)
{
  int c0, c1, c2, c3, c4, c5, c6, change, opti_applied;
  L_Inner_Loop *loop;
  L_Cb *cb;
  opti_applied = 0;

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      c0 = c1 = c2 = c3 = c4 = c5 = c6 = 0;

      if (loop->weight < L_LOOP_MIN_WEIGHT)
        continue;

      cb = loop->cb;

      if (Lsuper_do_loop_inv_code_rem)
        {
          c0 = L_sb_loop_inv_code_rem (loop, 0);
          STAT_COUNT ("L_sb_loop_inv_code_rem", c0, NULL);
        }

      if (Lsuper_do_loop_global_var_mig)
        {
          c1 = L_sb_loop_global_var_mig (loop, 0);
          if (c1)
	      L_do_flow_analysis (fn, LIVE_VARIABLE);
          STAT_COUNT ("L_sb_loop_global_var_mig", c1, NULL);
        }

      if (Lsuper_do_loop_op_fold)
        {
          c2 = L_sb_loop_operation_folding (loop, 0);
          STAT_COUNT ("L_sb_loop_operation_folding", c2, NULL);
        }

      if (Lsuper_do_loop_dead_code)
        {
          c3 = L_sb_loop_dead_code (loop, 0);
          STAT_COUNT ("L_sb_loop_dead_code", c3, NULL);
        }

      if (Lsuper_do_loop_ind_var_elim)
        {
          c4 = L_sb_loop_ind_var_elim (loop, 0);
          STAT_COUNT ("L_sb_loop_ind_var_elim", c4, NULL);
        }

      if (Lsuper_do_loop_ind_reinit)
        {
          c5 = L_sb_loop_ind_var_reinit (loop, 0);
          STAT_COUNT ("L_sb_loop_ind_var_reinit", c5, NULL);
        }

      if (Lsuper_do_loop_post_inc_conv)
        {
          c6 = L_sb_loop_post_inc_conversion (loop, 0);
          STAT_COUNT ("L_sb_loop_post_inc_conversion", c6, NULL);
        }

      change = c0 + c1 + c2 + c3 + c4 + c5 + c6;
      opti_applied += change;
    }

  if (opti_applied)
    L_invalidate_dataflow ();

  return opti_applied;
}

int
Lsuper_induction_var_optimization (L_Func * fn)
{
  int c0, change, opti_applied;
  L_Inner_Loop *loop;
  L_Cb *cb;
  opti_applied = 0;

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      c0 = 0;

      if (loop->weight < L_LOOP_MIN_WEIGHT)
        continue;

      cb = loop->cb;

      if (Lsuper_do_loop_ind_var_elim2)
	c0 = L_sb_loop_ind_var_elim2 (loop, 0);

      change = c0;
      opti_applied += change;
    }

  return opti_applied;
}

void
L_sb_loop_preprocess2 (L_Func * fn)
{
  int i;
  L_Inner_Loop *loop;

  if (L_duplicate_cb != NULL)
    L_punt ("L_sb_loop_preprocess2: L_duplicate_cb not freed");

  L_duplicate_cb =
    (L_Cb **) Lcode_malloc (sizeof (L_Cb *) * (fn->max_inner_loop_id + 1));

  for (i = 0; i <= fn->max_inner_loop_id; i++)
    {
      L_duplicate_cb[i] = NULL;
    }

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      if (loop->ave_iteration < L_LOOP_MIN_ITER_FOR_DUPL)
        {
          if (Lsuper_debug_loop_classic_opti)
            fprintf (stderr, "Loop %d (cb %d) not duplicated! -- weight\n",
                     loop->id, loop->cb->id);
        }
      else if (L_additional_loop_opts_to_apply (loop))
        {
          if (Lsuper_debug_loop_classic_opti)
            fprintf (stderr, "Duplicating loop %d (cb %d)\n", loop->id,
                     loop->cb->id);
          L_duplicate_loop_body (loop);
        }
      else if (Lsuper_debug_loop_classic_opti)
        fprintf (stderr, "Loop %d (cb %d) not duplicated!\n", loop->id,
                 loop->cb->id);
    }

  if (Lsuper_debug_loop_classic_opti)
    L_print_inner_loop_data (fn);
}

int
L_sb_loop_optimization2 (L_Func * fn)
{
  int c0, c1, c2, c3, c4, c5, c6, c7, change, opti_applied;
  L_Inner_Loop *loop;
  opti_applied = 0;

  if (Lsuper_debug_loop_classic_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER loop opti2 (fn %s)\n", fn->name);
      fprintf (stderr, "\n");
    }

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      c0 = c1 = c2 = c3 = c4 = c5 = c6 = c7 = 0;
#ifdef DO_SB_LOOP_INV_CODE_REM
      c0 = L_sb_loop_inv_code_rem (loop, 1);
#endif
#ifdef DO_SB_LOOP_GLOBAL_VAR_MIG
      c1 = L_sb_loop_global_var_mig (loop, 1);
      if (c1)
        L_do_flow_analysis (fn, LIVE_VARIABLE);
#endif
      /* Can only apply copy prop to duplicated loop */
#ifdef DO_SB_LOOP_COPY_PROP
      c2 = L_sb_loop_copy_propagation (loop, 1);
#endif
#ifdef DO_SB_LOOP_OP_FOLD
      c3 = L_sb_loop_operation_folding (loop, 1);
#endif
#ifdef DO_SB_LOOP_DEAD_CODE
      c4 = L_sb_loop_dead_code (loop, 1);
#endif
#ifdef DO_SB_LOOP_IND_ELIM
      c5 = L_sb_loop_ind_var_elim (loop, 1);
#endif
#ifdef DO_SB_LOOP_IND_REINIT
      c6 = L_sb_loop_ind_var_reinit (loop, 1);
#endif
#ifdef DO_SB_LOOP_POST_INC_CONV
      c7 = L_sb_loop_post_inc_conversion (loop, 1);
#endif
      change = c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7;
      opti_applied += change;
    }

  if (Lsuper_debug_loop_classic_opti)
    fprintf (stderr,
             "/* Loop optimization rules applied for fn %s %d times*/\n",
             fn->name, opti_applied);

  return opti_applied;
}

void
L_sb_loop_postprocess (L_Func * fn)
{
  Lcode_free (L_duplicate_cb);
  L_duplicate_cb = NULL;
}
