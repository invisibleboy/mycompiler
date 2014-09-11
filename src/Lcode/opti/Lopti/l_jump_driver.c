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
 *      Description :   driver for jump optimizations for Lopti
 *      Info Needed :   dominator info
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

/*
 *      Undef'ing these defines overrides parameter settings
 */
#define DO_DECIDABLE_BR
#define DO_DEAD_BLOCK
#define DO_BRANCH_TO_NEXT_BLOCK
#define DO_BRANCHES_TO_SAME_TARGET
#define DO_BRANCH_TO_UNCOND_BRANCH
#define DO_MERGE
#define DO_COMBINE_LABELS
/* Branch target expansion currently disabled from Lopti, messes up loops by
   creating multiple updates of ind vars, etc.. */
#undef DO_BRANCH_TARGET_EXP
#define DO_BRANCH_SWAP

#define MAX_NUM_ITERATION       10

/*======================================================================*/
/*
 *      Invoke all jump optimizations
 *              NOTE these optimizations add/delete cb's so shoudl always
 *              redo data flow, dominator, loop detection after this is done!!!
 */
/*======================================================================*/

/* SAM 1-05: this function is a simple version of the full jump opti below.
   This will be used to cleanup the initial CFG now that Pcode is not doing
   simple optis anymore, thus there can be alot of useless blocks in the code
   that come into Lcode. */
int
L_jump_initial_cleanup(L_Func *fn)
{
  int i, change, opti_applied;

  if (Lopti_do_jump_opti == 0)
    return (0);

  opti_applied = 0;

  if (Lopti_debug_jump_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER jump initial cleanup (fn %s)\n", fn->name);
      fprintf (stderr, "\n");
    }

  for (i = 0; i < MAX_NUM_ITERATION; i++)
    {
      int c1, c2, c3, c4, c5, c6, c7, c8;
      c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = 0;

#ifdef DO_DECIDABLE_BR
      if (Lopti_do_remove_decidable_cond_branches)
	{
	  c8 = L_remove_decidable_cond_branches (fn, 0);
	  STAT_COUNT ("L_remove_decidable_cond_branches", c8, NULL);
	}
#endif

#ifdef DO_DEAD_BLOCK
      if (Lopti_do_jump_dead_block_elim)
	{
	  c1 = L_delete_unreachable_blocks (fn);
	  STAT_COUNT ("L_delete_unreachable_blocks", c1, NULL);
	  Lopti_cnt_jump_dead_block_elim += c1;
	}
#endif
#ifdef DO_BRANCH_TO_NEXT_BLOCK
      if (Lopti_do_jump_br_to_next_block)
	{
	  c2 = L_jump_elim_branch_to_next_block (fn, 0);
	  STAT_COUNT ("L_jump_elim_branch_to_next_block", c2, NULL);
	  Lopti_cnt_jump_br_to_next_block += c2;
	}
#endif
#ifdef DO_BRANCHES_TO_SAME_TARGET
      if (Lopti_do_jump_br_to_same_target)
	{
	  c3 = L_jump_combine_branches_to_same_target (fn, 0);
	  STAT_COUNT ("L_jump_combine_branches_to_same_target", c3, NULL);
	  Lopti_cnt_jump_br_to_same_target += c3;
	}
#endif
#ifdef DO_BRANCH_TO_UNCOND_BRANCH
      if (Lopti_do_jump_br_to_uncond_br)
	{
	  c4 = L_jump_combine_branch_to_uncond_branch (fn, 0);
	  STAT_COUNT ("L_jump_combine_branch_to_uncond_branch", c4, NULL);
	  Lopti_cnt_jump_br_to_uncond_br += c4;
	}
#endif
#ifdef DO_MERGE
      if (Lopti_do_jump_block_merge)
	{
	  c5 = L_jump_merge_always_successive_blocks (fn, 0);
	  STAT_COUNT ("L_jump_merge_always_successive_blocks", c5, NULL);
	  Lopti_cnt_jump_block_merge += c5;
	}
#endif
#ifdef DO_COMBINE_LABELS
      if (Lopti_do_jump_combine_labels)
	{
	  c6 = L_jump_combine_labels (fn, 0);
	  STAT_COUNT ("L_jump_combine_labels", c6, NULL);
	  Lopti_cnt_jump_combine_labels += c6;
	}
#endif
      change = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
      Lopti_cnt_jump_opti += change;
      if (change != 0)
	opti_applied = 1;
      if (opti_applied)
      if (change == 0)
	break;
    }

  return (opti_applied);
}

int
L_jump_optimization (L_Func * fn)
{
  int i, change, opti_applied;

  if (Lopti_do_jump_opti == 0)
    return (0);

  opti_applied = 0;

  if (Lopti_debug_jump_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER jump opti (fn %s)\n", fn->name);
      fprintf (stderr, "\n");
    }

  for (i = 0; i < MAX_NUM_ITERATION; i++)
    {
      int c1, c2, c3, c4, c5, c6, c7, c8;
      c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = 0;

#ifdef DO_DECIDABLE_BR
      if (Lopti_do_remove_decidable_cond_branches)
	{
	  c8 = L_remove_decidable_cond_branches (fn, 0);
	  STAT_COUNT ("L_remove_decidable_cond_branches", c8, NULL);
	}
#endif

#ifdef DO_DEAD_BLOCK
      if (Lopti_do_jump_dead_block_elim)
	{
	  c1 = L_delete_unreachable_blocks (fn);
	  STAT_COUNT ("L_delete_unreachable_blocks", c1, NULL);
	  Lopti_cnt_jump_dead_block_elim += c1;
	}
#endif
#ifdef DO_BRANCH_TO_NEXT_BLOCK
      if (Lopti_do_jump_br_to_next_block)
	{
	  c2 = L_jump_elim_branch_to_next_block (fn, 0);
	  STAT_COUNT ("L_jump_elim_branch_to_next_block", c2, NULL);
	  Lopti_cnt_jump_br_to_next_block += c2;
	}
#endif
#ifdef DO_BRANCHES_TO_SAME_TARGET
      if (Lopti_do_jump_br_to_same_target)
	{
	  c3 = L_jump_combine_branches_to_same_target (fn, 0);
	  STAT_COUNT ("L_jump_combine_branches_to_same_target", c3, NULL);
	  Lopti_cnt_jump_br_to_same_target += c3;
	}
#endif
#ifdef DO_BRANCH_TO_UNCOND_BRANCH
      if (Lopti_do_jump_br_to_uncond_br)
	{
	  c4 = L_jump_combine_branch_to_uncond_branch (fn, 0);
	  STAT_COUNT ("L_jump_combine_branch_to_uncond_branch", c4, NULL);
	  Lopti_cnt_jump_br_to_uncond_br += c4;
	}
#endif
#ifdef DO_MERGE
      if (Lopti_do_jump_block_merge)
	{
	  c5 = L_jump_merge_always_successive_blocks (fn, 0);
	  STAT_COUNT ("L_jump_merge_always_successive_blocks", c5, NULL);
	  Lopti_cnt_jump_block_merge += c5;
	}
#endif
#ifdef DO_COMBINE_LABELS
      if (Lopti_do_jump_combine_labels)
	{
	  c6 = L_jump_combine_labels (fn, 0);
	  STAT_COUNT ("L_jump_combine_labels", c6, NULL);
	  Lopti_cnt_jump_combine_labels += c6;
	}
#endif
#ifdef DO_BRANCH_TARGET_EXP
      if (Lopti_do_jump_br_target_expansion)
	{
	  c7 = L_jump_branch_target_expansion (fn, 0);
	  STAT_COUNT ("L_jump_branch_target_expansion", c7, NULL);
	  Lopti_cnt_jump_br_target_expansion += c7;
	}
#endif
      change = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8;
      Lopti_cnt_jump_opti += change;
      if (change != 0)
	opti_applied = 1;
      if (opti_applied)
	L_invalidate_dataflow ();
      if (change == 0)
	break;
    }

#ifdef DO_BRANCH_SWAP
  if (Lopti_do_jump_br_swap)
    {
      Lopti_cnt_jump_br_swap += L_jump_branch_swap (fn, 0);
    }
#endif
  return (opti_applied);
}
