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
 *      File :          l_jump_driver.c
 *      Description :   driver for jump optimizations for Lsuperscalar
 *      Info Needed :   dominator info
 *      Creation Date : July 1993
 *      Author :        Scott Mahlke
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define REPEAT_JUMP_OPTI                2
#define REPEAT_CLEANUP_JUMP_OPTI        10

/*======================================================================*/
/*
 *      Invoke all jump optimizations
 *              NOTE these optimizations add/delete cb's so shoudl always
 *              redo data flow, dominator, loop detection after this is done!!!
 */
/*======================================================================*/

int
Lsuper_jump_optimization (L_Func * fn, int flags)
{
  int i, change, opti_applied, flags2;
  opti_applied = 0;

  L_compute_oper_weight (fn, 0, 1);

  flags2 = flags;
  if (!Lsuper_allow_backedge_exp)
    flags2 = L_CLR_BIT_FLAG (flags2, L_JUMP_ALLOW_BACKEDGE_EXP);
  if (!Lsuper_allow_expansion_of_loops)
    flags2 = L_CLR_BIT_FLAG (flags2, L_JUMP_ALLOW_LOOP_BODY_EXP);

  for (i = 0; i < REPEAT_JUMP_OPTI; i++)
    {
      int c1, c2, c3, c4, c5, c6, c7;
      c1 = c2 = c3 = c4 = c5 = c6 = c7 = 0;

      if (Lsuper_do_dead_block)
        {
          c1 = L_delete_unreachable_blocks (fn);
          STAT_COUNT ("L_jump_do_dead_block", c1, NULL);
        }

      if (Lsuper_do_branches_to_next_block)
        {
          c2 = L_jump_elim_branch_to_next_block (fn, flags2);
          STAT_COUNT ("L_jump_elim_branch_to_next_block", c2, NULL);
        }

      if (Lsuper_do_branches_to_same_target)
        {
          c3 = L_jump_combine_branches_to_same_target (fn, 0);
          STAT_COUNT ("L_jump_combine_branches_to_same_target", c3, NULL);
        }

      if (Lsuper_do_branches_to_uncond_branch)
        {
          c4 = L_jump_combine_branch_to_uncond_branch (fn, flags2);
          STAT_COUNT ("L_jump_combine_branch_to_uncond_branch", c4, NULL);
        }

      if (Lsuper_do_merge)
        {
          c5 = L_jump_merge_always_successive_blocks (fn, flags2);
          STAT_COUNT ("L_jump_merge_always_successive_blocks", c5, NULL);
        }

      if (Lsuper_do_combine_labels)
        {
          c6 = L_jump_combine_labels (fn, flags2);
          STAT_COUNT ("L_jump_combine_labels", c6, NULL);
        }

      if (Lsuper_do_branch_target_exp)
        {
          c7 = L_jump_branch_target_expansion (fn, flags2);
          STAT_COUNT ("L_jump_branch_target_expansion", c7, NULL);
        }

      change = c1 + c2 + c3 + c4 + c5 + c6 + c7;
      if (change != 0)
        opti_applied = 1;
      if (change == 0)
        break;
    }

  if (Lsuper_do_branch_swap)
    {
      L_jump_branch_swap (fn, 0);
    }

#if 0
  if (Lsuper_do_branch_pred)
    {
      L_jump_branch_prediction (fn);
    }
#endif

  if (Lsuper_do_dead_block)
    {
      int c = L_delete_unreachable_blocks (fn);
      STAT_COUNT ("L_jump_do_dead_block", c, NULL);
    }

  if (opti_applied)
    L_invalidate_dataflow ();

  return (opti_applied);
}


int
Lsuper_cleanup_jump_optimization (L_Func * fn, int flags)
{
  int i, change, opti_applied = 0, flags2;
  opti_applied = 0;

  L_compute_oper_weight (fn, 0, 1);

  flags2 = flags;
  if (!Lsuper_allow_backedge_exp)
    flags2 = L_CLR_BIT_FLAG (flags2, L_JUMP_ALLOW_BACKEDGE_EXP);
  if (!Lsuper_allow_expansion_of_loops)
    flags2 = L_CLR_BIT_FLAG (flags2, L_JUMP_ALLOW_LOOP_BODY_EXP);

  for (i = 0; i < REPEAT_CLEANUP_JUMP_OPTI; i++)
    {
      int c1, c2, c3, c4, c5, c6;
      c1 = c2 = c3 = c4 = c5 = c6 = 0;

      if (Lsuper_do_dead_block)
        {
          c1 = L_delete_unreachable_blocks (fn);
          STAT_COUNT ("L_jump_dead_block_removal", c1, NULL);
        }

      if (Lsuper_do_branches_to_next_block)
        {
          c2 = L_jump_elim_branch_to_next_block (fn, flags2);
          STAT_COUNT ("L_jump_elim_branch_to_next_block", c2, NULL);
        }

      if (Lsuper_do_branches_to_same_target)
        {
          c3 = L_jump_combine_branches_to_same_target (fn, 0);
          STAT_COUNT ("L_jump_combine_branches_to_same_target", c3, NULL);
        }

      if (Lsuper_do_branches_to_uncond_branch)
        {
          c4 = L_jump_combine_branch_to_uncond_branch (fn, flags2);
          STAT_COUNT ("L_jump_combine_branch_to_uncond_branch", c4, NULL);
        }

      if (Lsuper_do_merge)
        {
          c5 = L_jump_merge_always_successive_blocks (fn, flags2);
          STAT_COUNT ("L_jump_merge_always_successive_blocks", c5, NULL);
        }

      if (Lsuper_do_combine_labels)
        {
          c6 = L_jump_combine_labels (fn, flags2);
          STAT_COUNT ("L_jump_combine_labels", c6, NULL);
        }

      change = c1 + c2 + c3 + c4 + c5 + c6;

      opti_applied += change;

      if (!change)
        break;
    }

  if (Lsuper_do_branch_swap)
    {
      L_jump_branch_swap (fn, 0);
    }

  return (opti_applied);
}
