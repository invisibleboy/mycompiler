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
/**************************************************************************\
 *
 *  File: mia_jump_opti.c
 *
 *  Description: jump optimizations for Ltahoe
 *
 *  Authors: John W. Sias
 *
 *
\**************************************************************************/
/* 09/17/02 REK Updating to use functions from libtahoeop instead of Tmdes. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "mia_internal.h"

#define DEBUG_BR_TGT_EXPN 0

/*
 * Mopti_branch_target_expansion
 * ----------------------------------------------------------------------
 * Select arcs for expansion.  Target arcs to single-predecessor cbs
 * (no code expansion cost) and, heuristically, important, high-probability
 * arcs to small cbs (some code expansion cost)
 */

int
Mopti_branch_target_expansion (L_Func * fn)
{
  L_Cb *cb, *ncb;
  int change = 0, ichg;
  L_Flow *nfl;

  do
    {
      ichg = 0;

      for (cb = fn->first_cb; cb; cb = ncb)
	{
	  L_Cb *dst_cb;
	  L_Oper *br;
	  L_Flow *fl;
	  int chg = 0;

	  ncb = cb->next_cb;

	  for (fl = cb->dest_flow; fl; fl = nfl)
	    {
	      int dst_sz;
	      double tk_prob = 1.0;

	      nfl = fl->next_flow;

	      if ((br = L_find_branch_for_flow (cb, fl)))
		{
		  tk_prob = L_find_taken_prob (cb, br);

		  /* Cannot expand through a register branch */
		  if (L_register_branch_opcode (br))
		    break;
		}

	      dst_cb = fl->dst_cb;

	      /* Disallow unrolling */

	      if (cb == dst_cb)
		continue;

	      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) ||
		  L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_SOFTPIPE) ||
		  L_EXTRACT_BIT_VAL (dst_cb->flags, L_CB_EPILOGUE))
		continue;

	      dst_sz = L_cb_size (dst_cb);

	      /* JWS 20030730 - new heuristic for determining
	       * when to inline a block using predication.
	       * Consider average IPC to be around 3.0 and allow
	       * for a constant single cycle of expected penalty.
	       */
		  
	      if (fl->next_flow)
		{
		  double pen = (dst_sz + 1.2222) / 2;

		  if (pen < 1.0)
		    pen = 1.0;
		  if (tk_prob < (1.0 - 1.0 / pen))
		    continue;
		}

	      if (L_single_predecessor_cb (dst_cb))
		{
#if DEBUG_BR_TGT_EXPN
		  fprintf (stderr, "\n>> fn %s: tgt exp (%c) "
			   "sp %d(%d) -> %d ",
			   fn->name, fl->next_flow ? 'T' : 'F',
			   cb->id, br ? br->id : -1, dst_cb->id);
#endif
		}		/* if */
	      else
		{
		  /* Expansion would result in code growth.  Exercise
		   * additional care.  
		   */

		  if (dst_cb->weight < 1.0)
		    continue;

		  if ((fl->weight / dst_cb->weight) < 0.6)
		    continue;

		  if (dst_sz > 6)
		    continue;
#if DEBUG_BR_TGT_EXPN
		  fprintf (stderr, "\n>> fn %s: tgt exp (%c) "
			   "%d(%d) -> %d (%d ops repl) ",
			   fn->name, fl->next_flow ? 'T' : 'F',
			   cb->id, br ? br->id : -1, dst_cb->id, dst_sz);
#endif
		}		/* else */

	      if (L_expand_flow (fn, cb, fl,
				 LOPTI_EXPAND_ALLOW_FALLTHRU |
				 LOPTI_EXPAND_ALLOW_UNCOND))
		{
#if DEBUG_BR_TGT_EXPN
		  fprintf (stderr, "SUCCEEDED\n");
#endif
		  ichg++;
		  chg++;
		}		/* if */
#if DEBUG_BR_TGT_EXPN
	      else
		{
		  fprintf (stderr, "FAILED\n");
		}		/* else */
#endif
	    }			/* for fl */

	  ncb = cb->next_cb;

	  /* clean up redundant cond branches */

	  if (chg)
	    {
	      L_Flow *lfl, *nlfl;

	      if ((lfl = L_find_last_flow (cb->dest_flow)) &&
		  (nlfl = lfl->prev_flow) && (lfl->dst_cb == nlfl->dst_cb))
		{
		  L_Oper *lbr, *nlbr;

		  lbr = L_find_branch_for_flow (cb, lfl);
		  nlbr = L_find_branch_for_flow (cb, nlfl);

		  if ((!lbr || L_uncond_branch (lbr)) && nlbr &&
		      (nlbr->next_op == lbr))
		    {
		      L_delete_complete_oper (cb, nlbr);

		      /* Try again to merge with next cb */

		      if (L_single_predecessor_cb (lfl->dst_cb))
			ncb = cb;
		    }		/* if */
		}		/* if */
	    }			/* if */
	}			/* for cb */
      change += ichg;
    }				/* do */
  while (ichg);

  return (change);
}				/* Mopti_branch_target_expansion */
