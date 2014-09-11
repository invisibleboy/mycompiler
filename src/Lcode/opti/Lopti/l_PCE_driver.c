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
 *      File :          l_PCE_driver.c 
 *      Author:         Shane Ryoo, Wen-mei Hwu
 *      Description :   driver for partial code elimination
 *      Creation Date : September, 2002
 *==========================================================================*/
#include <config.h>
#include "l_opti.h"

#define MAX_PRE_ITER 1
/*
 *      Undef'ing these defines overrides parameter settings
 */

#undef TRACK_PRE_ITERATIONS

extern int L_jump_optimization (L_Func *);

/*========================================================================
  Partial code elimination
  This optimization has two different sections. The first is partial
    redundancy elimination from "Lazy Code Motion" 1992 by Knoop, 
    Ruthing, and Steffen. This takes care of global common subexpression
    elimination as well as some loop-invariant code motion. The second
    part is an iterative assignment sinking and dead code elimination
    loop which performs partial dead code elimination (code which is dead
    along some future paths), via "Partial Dead Code Elimination" 1994 
    by Knoop, Ruthing, and Steffen.
========================================================================*/

int
L_partial_code_elimination (L_Func * fn)
{
  int i, change, c1, c2, pre, pde;

  change = c1 = c2 = pre = pde = 0;

  if (!Lopti_do_PCE)
    return 0;

  /* First need to split critical edges in order to expose code motion
     opportunities */
  if (Lopti_do_PCE_split_critical_edges)
    {
      c1 = L_split_fn_critical_edges (fn);
      STAT_COUNT ("Lopti_PCE_split_critical_edges", c1, NULL);
    }

  if (Lopti_do_PDE)
    {
      /* Note: partial dead code elimination must be done with the split of
       * critical edges, or at least splitting every critical branch leaving 
       * a loop. Otherwise, operations cannot be moved past loops, which is 
       * bad, and loop-invariant code motion will ping-pong.
       */

      if (!Lopti_do_PCE_split_critical_edges)
	L_split_fn_loop_out_critical_edges (fn);

      change += L_partial_dead_code_elimination (fn);
    }

  if (Lopti_do_PRE)
    {
      L_do_flow_analysis (fn, DOMINATOR_CB);
      L_loop_detection (fn, 1);

      for (i = 0; i < MAX_PRE_ITER; i++)
	{
#ifdef TRACK_PRE_ITERATIONS
	  fprintf (stderr, "Iteration %d of PRE loop of function "
		   "%s.\n", i + 1, fn->name);
	  if (i == MAX_PRE_ITER - 1)
	    fprintf (stderr, "HELP!!! LAST ITERATION REACHED IN PRE LOOP "
		     "OF FUNCTION %s!\n", fn->name);
#endif
	  if (Lopti_do_PRE_speculative_code_motion)
	    pre = L_speculative_PRE (fn);
	  else
	    pre = L_partial_redundancy_elimination (fn);
	  change += pre;
	  if (!pre)
	    break;
	}
    }

  if (fn->weight != 0.0)
    change += L_min_cut_PDE (fn);

  c2 = L_jump_optimization (fn);
  STAT_COUNT ("Lopti_PCE_coalesce_cbs", c2, NULL);

  return (change);
}


int
L_partial_dead_code_optimization (L_Func * fn)
{
  int change = 0, c1 = 0, c2 = 0;

  if (!Lopti_do_PDE)
    return 0;

  if (Lopti_do_PCE_split_critical_edges)
    c1 = L_split_fn_critical_edges (fn);

  STAT_COUNT ("Lopti_PCE_split_critical_edges", c1, NULL);

  change = L_partial_dead_code_elimination (fn);

  if (fn->weight != 0.0)
    L_min_cut_PDE (fn);

  c2 = L_jump_optimization (fn);
  STAT_COUNT ("Lopti_PCE_coalesce_cbs", c2, NULL);

  return (change);
}


int 
L_PRE_optimization (L_Func * fn)
{
  int i, change, total_change = 0, old_max;

  if (!Lopti_do_PRE)
    return 0;

  old_max = L_fn->max_reg_id;

  for (i = 0; i < MAX_PRE_ITER; i++)
    {
#ifdef TRACK_PRE_ITERATIONS
      fprintf (stderr, "Iteration %d of PRE loop of function %s.\n",
	       i + 1, fn->name);
      if (i == MAX_PRE_ITER - 1)
	fprintf (stderr, "HELP!!! LAST ITERATION REACHED IN SPEC PRE LOOP "
		 "OF FUNCTION %s!\n", fn->name);
#endif
      if (Lopti_do_PRE_speculative_code_motion && fn->weight > 0.0)
	change = L_speculative_PRE (fn);
      else
	change = L_partial_redundancy_elimination (fn);

      total_change += change;
      if (!change)
	break;
    }

  return total_change;
}
