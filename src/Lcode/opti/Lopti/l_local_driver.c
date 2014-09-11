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
 *      Description :   driver for cb level optimizations for Lopti
 *      Info Needed :   live variable analysis
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
#define DO_CONST_PROP
#define DO_COPY_PROP
#define DO_REV_COPY_PROP
#define DO_MEM_COPY_PROP
#define DO_COMMON_SUB
#define DO_RED_LOAD
#define DO_RED_STORE
#define DO_CONST_FOLD
#define DO_CONST_COMBINE
#define DO_STR_RED
#define DO_OP_FOLD
#define DO_BR_FOLD
#define DO_OP_CANCEL
#define DO_DEAD_CODE
#define DO_CODE_MOTION
#define DO_REMOVE_SIGN_EXT
#define DO_REG_RENAMING
#define DO_COMMON_SUB_2
#define DO_COPY_PROP_2
#define DO_OPER_BRKDWN
#define DO_OPER_RECOMBINE
#define DO_RED_LOGIC
#define DO_BRANCH_VAL_PROP

#define MAX_NUM_ITERATION       15

#undef DO_REMOVE_SIGN_EXT

/*======================================================================*/
/*
 *      Invoke all basic block level transformations
 *              note this excludes L_local_branch_folding and
 *              L_local_operation_migration which are only useful
 *              after superblocks are formed.
 */
/*======================================================================*/

void
mydebug (char c, L_Func * fn, L_Cb * cb)
{
  if (cb->id == -1)
    {
      printf ("################### %c\n", c);
      L_print_cb (stdout, fn, cb);
      printf ("################### %c\n", c);
    }
}


/* Local code optimizations, operate within cbs. */
/* Assumes live variable analysis done. */
int
L_local_code_optimization (L_Func * fn, int mov_flag)
{
  int k, change, opti_applied, cnt;
  L_Cb *cb;
  int use_sync_arcs = 0;

  if (Lopti_do_local_opti == 0)
    return (0);

  opti_applied = 0;

  if (Lopti_debug_local_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER local opti (fn %s)\n", fn->name);
      fprintf (stderr, "\n");
    }

  L_partial_dead_code_removal (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* REH 9/95 - Don't perform local optimizations within */
      /*   region boundary cb's                              */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	continue;

      for (k = 0; k < MAX_NUM_ITERATION; k++)
	{
	  /* temporary optimization counters */
	  int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13,
	    c14, c15, c16, c17, c18;
	  c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = c9 = c10 = c11 = c12 = c13 =
	    c14 = c15 = c16 = c17 = c18 = 0;

#ifdef DO_REV_COPY_PROP
	  if (Lopti_do_local_rev_copy_prop)
	    {
	      c2 = L_local_rev_copy_propagation (cb);
	      Lopti_cnt_local_rev_copy_prop += c2;
	      STAT_COUNT ("L_local_rev_copy_propagation", c2, cb);
	    }
#endif

#ifdef DO_DEAD_CODE
	  if (Lopti_do_local_dead_code_rem)
	    {
	      c13 = L_local_dead_code_removal (cb);
	      Lopti_cnt_local_dead_code_rem += c13;
	      STAT_COUNT ("L_local_dead_code_removal_total", c13, cb);
	    }
#endif

#ifdef DO_CONST_PROP
	  if (Lopti_do_local_constant_prop)
	    {
	      c1 = L_local_constant_propagation (cb, 0);
	      Lopti_cnt_local_constant_prop += c1;
	      STAT_COUNT ("L_local_constant_propagation", c1, cb);
	    }
#endif

#ifdef DO_COPY_PROP
	  if (Lopti_do_local_copy_prop)
	    {
	      c3 = L_local_copy_propagation (cb);
	      Lopti_cnt_local_copy_prop += c3;
	      STAT_COUNT ("L_local_copy_propagation", c3, cb);
	    }
#endif

#ifdef DO_MEM_COPY_PROP
	  if (Lopti_do_local_mem_copy_prop)
	    {
	      c4 = L_local_memory_copy_propagation (cb);
	      Lopti_cnt_local_mem_copy_prop += c4;
	      STAT_COUNT ("L_local_memory_copy_propagation_total", c4, cb);
	    }
#endif

#ifdef DO_COMMON_SUB
	  if (Lopti_do_local_common_sub_elim)
	    {
	      c5 = L_local_common_subexpression (cb, 0);
	      Lopti_cnt_local_common_sub_elim += c5;
	      STAT_COUNT ("L_local_common_subexpression_total", c5, cb);
	    }
#endif

#ifdef DO_RED_LOAD
	  if (Lopti_do_local_red_load_elim)
	    {
	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{
		  use_sync_arcs = L_use_sync_arcs;
		  L_use_sync_arcs = 0;
		}
	      c6 = L_local_redundant_load (cb);
	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{
		  L_use_sync_arcs = use_sync_arcs;
		}
	      Lopti_cnt_local_red_load_elim += c6;
	      STAT_COUNT ("L_local_redundant_load", c6, cb);
	    }
#endif

	  /* if do alot of red lds or cse's, continue here to prevent
	     wasting time with later opti that likely wont work until
	     the moves introduced by red ld and cse are removed!! */
	  if ((c5 + c6) > 20)
	    {
	      Lopti_cnt_local_opti += c1 + c2 + c3 + c4 + c5 + c6 + c13;
	      continue;
	    }

#ifdef DO_RED_STORE
	  if (Lopti_do_local_red_store_elim)
	    {
	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{
		  use_sync_arcs = L_use_sync_arcs;
		  L_use_sync_arcs = 0;
		}
	      c7 = L_local_redundant_store (cb);
	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{
		  L_use_sync_arcs = use_sync_arcs;
		}
	      Lopti_cnt_local_red_store_elim += c7;
	      STAT_COUNT ("L_local_redundant_store", c7, cb);
	    }
#endif

#ifdef DO_CONST_COMBINE
	  if (Lopti_do_local_constant_comb)
	    {
	      c11 = L_local_constant_combining (cb);
	      Lopti_cnt_local_constant_comb += c11;
	      STAT_COUNT ("L_local_constant_combining_total", c11, cb);
	    }
#endif

#ifdef DO_CONST_FOLD
	  if (Lopti_do_local_constant_fold)
	    {
	      c8 = L_local_constant_folding (cb);
	      Lopti_cnt_local_constant_fold += c8;
	      STAT_COUNT ("L_local_constant_folding_total", c8, cb);
	    }
#endif

#ifdef DO_STR_RED
	  if (Lopti_do_local_strength_red)
	    {
	      c9 = L_local_strength_reduction (cb);
	      Lopti_cnt_local_strength_red += c9;
	      STAT_COUNT ("L_local_strength_reduction_total", c9, cb);
	    }
#endif

#ifdef DO_BR_FOLD
	  if (Lopti_do_local_branch_fold)
	    {
	      c16 = L_local_branch_folding (cb);
	      Lopti_cnt_local_branch_fold += c16;
	      /* control structure modified so redo flow analysis */
	      if (c16 != 0)
		L_do_flow_analysis (fn, LIVE_VARIABLE);
	      STAT_COUNT ("L_local_branch_folding_total", c16, cb);
	    }
#endif

#ifdef DO_DEAD_CODE
	  if (Lopti_do_local_dead_code_rem)
	    {
	      c13 = L_local_dead_code_removal (cb);
	      Lopti_cnt_local_dead_code_rem += c13;
	      STAT_COUNT ("L_local_dead_code_removal_2", c13, cb);
	    }
#endif

#ifdef DO_CODE_MOTION
	  if (Lopti_do_local_code_motion)
	    {
	      c14 = L_local_code_motion (cb);
	      Lopti_cnt_local_code_motion += c14;
	      STAT_COUNT ("L_local_code_motion_total", c14, cb);
	    }
#endif

#ifdef DO_OP_FOLD
	  if (Lopti_do_local_operation_fold)
	    {
	      c10 = L_local_operation_folding (cb);
	      Lopti_cnt_local_operation_fold += c10;
	      STAT_COUNT ("L_local_operation_folding_total", c10, cb);
	    }
#endif

#ifdef DO_OP_CANCEL
	  if (Lopti_do_local_operation_cancel)
	    {
	      c12 = L_local_operation_cancellation (cb);
	      Lopti_cnt_local_operation_cancel += c12;
	      STAT_COUNT ("L_local_operation_cancellation_total", c12, cb);
	    }
#endif

#ifdef DO_REMOVE_SIGN_EXT
	  if (Lopti_do_local_remove_sign_ext)
	    {
	      c15 = L_local_remove_sign_extension (cb);
	      Lopti_cnt_local_remove_sign_ext += c15;
	      STAT_COUNT ("L_local_remove_sign_extension", c15, cb);
	    }
#endif

#ifdef DO_RED_LOGIC
	  if (Lopti_do_local_reduce_logic)
	    {
	      c17 = L_local_logic_reduction (cb);
	      Lopti_cnt_local_remove_sign_ext += c17;
	      STAT_COUNT ("L_local_logic_reduction_total", c17, cb);
	    }
#endif

#ifdef DO_BRANCH_VAL_PROP
	  if (Lopti_do_local_branch_val_prop)
	    {
	      c18 = L_local_branch_val_propagation (cb);
	      STAT_COUNT ("L_local_branch_val_prop_total", c18, cb);
	    }
#endif
	  change =
	    c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9 + c10 + c11 + c12 +
	    c13 + c14 + c15 + c16 + c17 + c18;
	  Lopti_cnt_local_opti += change;
	  if (change != 0)
	    opti_applied = 1;
	  if (change == 0)
	    break;
	}

#ifdef DO_REG_RENAMING
      if (Lopti_do_local_register_rename)
	{
	  cnt = L_local_register_renaming (cb);
	  Lopti_cnt_local_register_rename += cnt;
	  STAT_COUNT ("L_local_register_renaming", cnt, cb);
	}
#endif
      /* Apply common sub expr with moves of constants 
         allowed to be subexprs */

#ifdef DO_COMMON_SUB_2
      if (Lopti_do_local_common_sub_elim)
	{
	  if (mov_flag)
	    cnt = L_local_common_subexpression (cb,
						L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT
						|
						L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT);
	  else
	    cnt = L_local_common_subexpression (cb, 0);
	  Lopti_cnt_local_common_sub_elim += cnt;
	  STAT_COUNT ("L_local_common_subexpression_2", cnt, cb);
	}
#endif
      /* Clean up after above common sub */

#ifdef DO_COPY_PROP_2
      if (Lopti_do_local_copy_prop)
	{
	  cnt = L_local_copy_propagation (cb);
	  Lopti_cnt_local_copy_prop += cnt;
	  STAT_COUNT ("L_local_copy_propagation_2", cnt, cb);
	}
#endif
    }

  /* STAT_DUMP (); */

  return (opti_applied);
}

/*======================================================================*/
/*
 *      Invoke operation breakdown, breakup instructions which cannot be
 *      handled in the target architecture.
 */
/*======================================================================*/

void
L_oper_breakdown (L_Func * fn)
{
  L_Cb *cb;

  if (Lopti_do_local_opti == 0)
    return;

  if (Lopti_debug_local_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER oper breakdown (fn %s)\n", fn->name);
      fprintf (stderr, "\n");
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
#ifdef DO_OPER_BRKDWN
      if (Lopti_do_local_op_breakdown)
	{
	  L_local_oper_breakdown (cb);
	}
#endif
    }
}

void
L_oper_recombine (L_Func * fn)
{
  L_Cb *cb;

  if (Lopti_do_local_opti == 0)
    return;

  if (Lopti_debug_local_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER oper recombine (fn %s)\n", fn->name);
      fprintf (stderr, "\n");
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* REH 9/95 - Don't perform recombination within       */
      /*   region boundary cb's                              */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	continue;

#ifdef DO_OPER_RECOMBINE
      if (Lopti_do_local_op_recombine)
	{
	  L_local_oper_recombine (cb);
	}
#endif
    }
}
