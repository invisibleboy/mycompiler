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
 *      File :          l_global_driver.c 
 *      Description :   driver for inter-cb optimizations for Lopti
 *      Info Needed :   
 *              global_dead_code : live variable analysis
 *              global_code_opti : live variable, reaching defn, reaching expr,
 *                                 dominator info, danger info
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
#define DO_GLOB_DEAD_CODE
#define DO_GLOB_CONST_PROP
#define DO_GLOB_BRANCH_VAL_PROP
#define DO_GLOB_COPY_PROP
#define DO_GLOB_MEM_COPY_PROP
#define DO_GLOB_COMMON_SUB
#define DO_GLOB_RED_LOAD
#define DO_GLOB_RED_STORE
#define DO_GLOB_UNNEC_BOOL

#define DEBUG_COMPLETE_STORE_LOAD_REMOVAL
/*
 * Do not define this.  Infinite Loops will be the result!!!!!
 */
#undef  DO_GLOB_COMMON_SUB_2  /* common sub with moves, 
				 go into infinte loop now */

#define MAX_NUM_GLOB_ITERATION 10

/*======================================================================*/
/*
 *      Invoke global dead code removal
 */
/*======================================================================*/

int
L_global_dead_code_optimization (L_Func * fn)
{
#if 0
  int k;
#endif
  int change, opti_applied;
  L_Cb *cb;

  if (Lopti_do_global_opti == 0)
    return (0);

  opti_applied = 0;

  if (Lopti_debug_global_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER global dead code removal (fn %s)\n",
	       fn->name);
      fprintf (stderr, "\n");
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
#if 0
      /* SER: Note that since live variables aren't updated after a removal,
       * running this loop is useless.
       */
      for (k = 0; k < MAX_NUM_GLOB_ITERATION; k++)
	{
#endif
	  int c1 = 0;

#ifdef DO_GLOB_DEAD_CODE
	  if (Lopti_do_global_dead_code_rem)
	    {
	      c1 = L_global_dead_code_removal (cb);
	      Lopti_cnt_global_dead_code_rem += c1;
	    }
#endif
	  change = c1;
	  Lopti_cnt_global_opti += change;
	  if (change != 0)
	    {
	      opti_applied = 1;
	      STAT_COUNT ("L_global_dead_code_optimization", change, NULL);
	    }
#if 0
	  if (change == 0)
	    break;
	}
#endif
    }

  return (opti_applied);
}

/*======================================================================*/
/*
 *      Invoke all inter-cb transformations
 */
/*======================================================================*/

int
L_global_code_optimization (L_Func * fn)
{
  int opti_applied;
  L_Cb *cb1, *cb2;

  if (!Lopti_do_global_opti)
    return (0);

  opti_applied = 0;

  if (Lopti_debug_global_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER global opti (fn %s)\n", fn->name);
      fprintf (stderr, "\n");
    }

  for (cb1 = fn->first_cb; cb1 != NULL; cb1 = cb1->next_cb)
    {
      int c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, change, inserted;

      if (L_EXTRACT_BIT_VAL (cb1->flags, L_CB_BOUNDARY))
	continue;
      c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = c9 = c10 = 0;

      for (cb2 = fn->first_cb; cb2 != NULL; cb2 = cb2->next_cb)
	{
	  if (cb1 == cb2)
	    continue;

          if (Lopti_do_global_dead_if_then_else_rem)
            {
              c9 = L_global_dead_if_then_else_rem (cb1, cb2);
              Lopti_cnt_global_dead_if_then_else_rem += c9;
              STAT_COUNT ("L_global_dead_if_then_else_rem", c9, NULL);
            }

	  if (!(L_in_cb_DOM_set (cb2, cb1->id)))
	    continue;

#ifdef DO_GLOB_CONST_PROP
	  if (Lopti_do_global_constant_prop)
	    {
	      c1 = L_global_constant_propagation (cb1, cb2, 0);
	      Lopti_cnt_global_constant_prop += c1;
	      if (cb1->region != cb2->region)
		{
		  Lopti_inter_region_global_opti += c1;
		  Lopti_inter_region_global_constant_prop += c1;
		}
	      STAT_COUNT ("L_global_constant_propagation_total", c1, NULL);
	    }
#endif

#ifdef DO_GLOB_BRANCH_VAL_PROP
	  if (Lopti_do_global_branch_val_prop)
	    {
	      c10 = L_global_branch_val_propagation (cb1, cb2);
	      STAT_COUNT ("L_global_branch_val_prop_total", c10, NULL);
	    }
#endif

#ifdef DO_GLOB_COMMON_SUB
	  if (Lopti_do_global_common_sub_elim)
	    {
	      c2 = L_global_common_subexpression (cb1, cb2, 0);
	      Lopti_cnt_global_common_sub_elim += c2;
	      if (cb1->region != cb2->region)
		{
		  Lopti_inter_region_global_opti += c2;
		  Lopti_inter_region_global_common_sub_elim += c2;
		}
	      STAT_COUNT ("L_global_common_subexpression_total_1", c2, NULL);
	    }
#endif

#ifdef DO_GLOB_RED_LOAD
	  if (Lopti_do_global_red_load_elim)
	    {
	      c3 = L_global_memflow_redundant_load (cb1, cb2, &inserted);

	      Lopti_cnt_global_red_load_elim += c3;
	      if (cb1->region != cb2->region)
		{
		  Lopti_inter_region_global_opti += c3;
		  Lopti_inter_region_global_red_load_elim += c3;
		}
	      STAT_COUNT ("L_global_redundant_load", c3, NULL);
	    }
#endif

#ifdef DO_GLOB_RED_STORE
	  if (Lopti_do_global_red_store_elim)
	    {
	      c4 = L_global_memflow_redundant_store (cb1, cb2);

	      Lopti_cnt_global_red_store_elim += c4;
	      if (cb1->region != cb2->region)
		{
		  Lopti_inter_region_global_opti += c4;
		  Lopti_inter_region_global_red_store_elim += c4;
		}
	      STAT_COUNT ("L_global_redundant_store", c4, NULL);
	    }
#endif

#ifdef DO_GLOB_MEM_COPY_PROP
	  if (Lopti_do_global_mem_copy_prop)
	    {
	      c6 = L_global_memflow_redundant_load_with_store (cb1, cb2,
							       &inserted);
	      Lopti_cnt_global_mem_copy_prop += c6;
	      if (cb1->region != cb2->region)
		{
		  Lopti_inter_region_global_opti += c6;
		  Lopti_inter_region_global_mem_copy_prop += c6;
		}
	      STAT_COUNT ("L_global_memory_copy_propagation_total", c6, NULL);
	    }
#endif

#ifdef DO_GLOB_COPY_PROP
	  if (Lopti_do_global_copy_prop)
	    {
	      c5 = L_global_copy_propagation (cb1, cb2);
	      Lopti_cnt_global_copy_prop += c5;
	      if (cb1->region != cb2->region)
		{
		  Lopti_inter_region_global_opti += c5;
		  Lopti_inter_region_global_copy_prop += c5;
		}
	      STAT_COUNT ("L_global_copy_propagation_total", c5, NULL);
	    }
#endif

#ifdef DO_GLOB_COMMON_SUB_2
	  if (Lopti_do_global_common_sub_elim)
	    {
	      c7 = L_global_common_subexpression (cb1, cb2,
						  L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT
						  |
						  L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT);
	      Lopti_cnt_global_common_sub_elim += c7;
	      if (cb1->region != cb2->region)
		{
		  Lopti_inter_region_global_opti += c7;
		  Lopti_inter_region_global_common_sub_elim += c7;
		}
	      STAT_COUNT ("L_global_common_subexpression_total_2", c7, NULL);
	    }
#endif

	  change = c1 + c2 + c3 + c4 + c5 + c6 + c7 + c10;
	  Lopti_cnt_global_opti += change;
	  if (change != 0)
	    opti_applied = 1;
	}

#ifdef DO_GLOB_UNNEC_BOOL
      if (Lopti_do_global_elim_boolean_ops)
	{
	  c8 = L_global_remove_unnec_boolean (cb1);
	  Lopti_cnt_global_elim_boolean_ops += c8;
	  STAT_COUNT ("L_global_remove_unnec_boolean", c8, NULL);
	}
#endif

      change = c8;
      Lopti_cnt_global_opti += change;
      if (change != 0)
	opti_applied = 1;
    }

  return (opti_applied);
}
