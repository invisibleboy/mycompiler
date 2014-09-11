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
 *      Description :   driver for loop optimizations for Lopti
 *      Info Needed :   live variable analysis, loop detection
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"
#include "l_pred_opti.h"

/*
 *      Undef'ing these defines overrides parameter settings
 */
#define DO_LOOP_INVAR_CODE_REM
#define DO_LOOP_SIMPLIFY_LOOP_BR
#define DO_LOOP_GLOBAL_VAR_MIG
#define DO_LOOP_IND_STR_RED
#define DO_LOOP_IND_ELIM
#define DO_LOOP_DEAD_REM

int
L_loop_optimization (L_Func * fn)
{
  int i, opti_applied = 0, num_loops, *loop_array, flag;
  L_Loop *loop;
  int use_sync_arcs = 0;

  if (!Lopti_do_loop_opti)
    return (0);

  L_find_basic_ind_var (fn);

  if (Lopti_debug_loop_opti)
    fprintf (stderr, "\n> ENTER loop optimization (fn %s)\n\n", fn->name);

  if (!(loop = fn->first_loop))
    return 0;

  for (num_loops = 0; loop; loop = loop->next_loop)
    num_loops++;

  loop_array = (int *) alloca (sizeof (int) * num_loops);
  L_sort_loops (loop_array, num_loops);

  do
    {
      flag = 0;

      L_compute_oper_weight (fn, 0, 1); /* may be used in heuristics
					 * in loop optimizations
					 */
      
      for (i = num_loops - 1; i >= 0; i--)
	{
	  int c0, c1, c2, c3, c4, c5, c6, change;
	  c0 = c1 = c2 = c3 = c4 = c5 = c6 = 0;

	  loop = L_find_loop (L_fn, loop_array[i]);

	  /* 
	   * REH - You'd be surprised where a boundary cb could
	   *       crop up.  If it is a loop header, the loop can't
	   *         be loop optimized!
	   */
	  if (L_EXTRACT_BIT_VAL (loop->header->flags, L_CB_BOUNDARY))
	    continue;

#ifdef DO_LOOP_INVAR_CODE_REM
	  if (Lopti_do_loop_inv_code_rem)
	    {
	      c0 = L_loop_invariant_code_removal (loop);
	      Lopti_cnt_loop_inv_code_rem += c0;
	      if (c0 > 0)
		{
		  /* code movement - redo all analysis */
		  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE |
				      AVAILABLE_DEFINITION |
				      REACHING_DEFINITION |
				      AVAILABLE_EXPRESSION);
		  STAT_COUNT ("L_loop_invariant_code_removal", c0, NULL);
		}
	    }
#endif
#ifdef DO_LOOP_SIMPLIFY_LOOP_BR
	  if (Lopti_do_loop_br_simp)
	    {
	      c1 = L_loop_simplify_back_branch (loop);
	      Lopti_cnt_loop_br_simp += c1;
	      STAT_COUNT ("L_loop_simplify_back_branch_total", c1, NULL);
	    }
#endif
#ifdef DO_LOOP_GLOBAL_VAR_MIG
          if (Lopti_do_loop_global_var_mig)
            {
              if (Lopti_ignore_sync_arcs_for_loop_inv_migration)
                {
                  use_sync_arcs = L_use_sync_arcs;
                  L_use_sync_arcs = 0;
                }

	      if (L_func_acc_specs || 
		  (L_func_contains_dep_pragmas && L_use_sync_arcs))
		c2 = L_loop_global_var_migration_by_sync (loop);
	      else
                c2 = L_loop_global_var_migration (loop);

              if (Lopti_ignore_sync_arcs_for_loop_inv_migration)
                L_use_sync_arcs = use_sync_arcs;

              Lopti_cnt_loop_global_var_mig += c2;
              /* control structure modified so redo all analysis */
              if (c2 != 0)
                {
                  STAT_COUNT("L_loop_invar_mig", c2, NULL);
                  L_do_flow_analysis (fn, DOMINATOR_CB);
                  L_loop_detection (fn, 1);
                  num_loops = 0;
                  for (loop = fn->first_loop; loop;
                       loop = loop->next_loop)
                    num_loops++;
                  L_sort_loops (loop_array, num_loops);
                  L_find_basic_ind_var (fn);
                  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE |
                                      AVAILABLE_DEFINITION |
                                      REACHING_DEFINITION |
                                      AVAILABLE_EXPRESSION);

		  /* Try to clean up after introducing store-guard
		   * predicates.
		   */

		  if (Lopti_store_migration_mode ==
		      L_STORE_MIGRATION_FULL_PRED)
		    L_lightweight_pred_opti(fn);

                  flag = 1;
                  opti_applied = 1;
                  break;
                }
            }
#endif
#ifdef DO_LOOP_IND_STR_RED
	  if (Lopti_do_loop_ind_var_str_red)
	    {
	      c3 = L_loop_induction_strength_reduction (loop);
	      Lopti_cnt_loop_ind_var_str_red += c3;
	      /* new registers introduced so redo flow analysis */
	      if (c3 != 0)
		{
		  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE |
				      AVAILABLE_DEFINITION |
				      REACHING_DEFINITION |
				      AVAILABLE_EXPRESSION);
		  STAT_COUNT ("L_loop_induction_strength_reduction", c3,
			      NULL);
		}
	    }
#endif
#ifdef DO_LOOP_IND_ELIM
          if (Lopti_do_loop_ind_var_elim && ((c0 + c1 + c2 + c3) == 0))
            {
              c4 = L_loop_induction_elimination (loop);
              STAT_COUNT("L_loop_induction_elimination_total", c4, NULL);
              Lopti_cnt_loop_ind_var_elim += c4;
            }
#endif
          if (Lopti_do_dead_loop_rem)
            {
              /* JEM 7/31/95 */
              c5 = L_dead_loop_removal (loop);
              /* control structure modified so redo all analysis */
              if (c5 != 0)
                {
                  L_do_flow_analysis (fn, DOMINATOR_CB);
                  L_loop_detection (fn, 1);
                  num_loops = 0;
                  for (loop = fn->first_loop; loop != NULL;
                       loop = loop->next_loop)
                    num_loops++;
                  L_sort_loops (loop_array, num_loops);
                  L_find_basic_ind_var (fn);
                  L_do_flow_analysis (fn, DOMINATOR_CB | LIVE_VARIABLE |
                                      AVAILABLE_DEFINITION |
                                      REACHING_DEFINITION |
                                      AVAILABLE_EXPRESSION);
		  STAT_COUNT ("L_loop_dead_loop_rem", c5, NULL);
                  flag = 1;
                  opti_applied = 1;
                  break;
                }
              Lopti_cnt_dead_loop_rem += c5;
            }

          if (Lopti_do_longword_loop_opti)
            {
              c6 = L_do_longword_loop_conversion (loop);
            }

          change = c0 + c1 + c2 + c3 + c4 + c5 + c6;
          Lopti_cnt_loop_opti += change;
          if (change != 0)
            opti_applied = 1;
        }
    }
  while (flag);

  return (opti_applied);
}
