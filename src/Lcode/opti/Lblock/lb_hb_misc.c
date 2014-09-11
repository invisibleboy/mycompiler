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
 *	File :		l_misc.c
 *	Description :	miscellaneous functions used with hyperblocks
 *	Creation Date :	September 1993
 *	Authors : 	Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"

#define ERR	stderr
#undef DEBUG_CONVERT
#undef DEBUG_CYCLE

void
LB_hb_reset_max_oper_id (L_Func * fn)
{
  int max;
  L_Cb *cb;
  L_Oper *oper;

  max = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (oper->id > max)
	    max = oper->id;
	}
    }

  fn->max_oper_id = max;
}

int
LB_hb_is_single_block_loop (L_Cb * cb)
{
  L_Flow *flow;
  L_Oper *last_op;

  if (cb == NULL)
    return (0);
  if (cb->dest_flow == NULL)
    return (0);

  flow = L_find_last_flow (cb->dest_flow);
  if (flow->dst_cb == cb)
    return (1);

  last_op = cb->last_op;
  if (L_cond_branch_opcode (last_op) ||
      (L_uncond_branch_opcode (last_op) &&
       L_cond_branch_opcode (last_op->prev_op)))
    {
      flow = flow->prev_flow;
      if (flow->dst_cb == cb)
	return (1);
    }

  return (0);
}


static int
dfs_visit (L_Cb * cb, Set blocks, L_Cb * header)
{
  L_Flow *dest;
  L_Cb *dest_cb;
  int cycle = 0;

  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_VISITED);

  for (dest = cb->dest_flow; dest; dest = dest->next_flow)
    {
      dest_cb = dest->dst_cb;

      /* Ignore backedges to region header and side exit arcs */

      if ((dest_cb == header) ||
	  !Set_in (blocks, dest_cb->id))
	continue;

      if (!L_EXTRACT_BIT_VAL (dest_cb->flags, L_CB_VISITED))
	cycle |= dfs_visit (dest_cb, blocks, header);
      else if (!L_EXTRACT_BIT_VAL (dest_cb->flags, L_CB_VISITED2))
	cycle = 1;
    }

  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_VISITED2);

  return cycle;
}

/*
 * LB_hb_region_contains_cycle
 * ----------------------------------------------------------------------
 * Given a set of CB ids and a header CB, return 1 iff the region
 * contains a cycle which does not include the header; 0 otherwise.
 */
int
LB_hb_region_contains_cycle (Set blocks, L_Cb * header)
{
  int i, num_cb, *buf, cycle = 0;
  L_Cb *cb;

  num_cb = Set_size (blocks);
  buf = (int *) alloca (sizeof (int) * num_cb);
  Set_2array (blocks, buf);

  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_VISITED);
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_VISITED2);
    }

  if (dfs_visit (header, blocks, header))
    {
      cycle = 1;
    }
  else
    {
      for (i = 0; i < num_cb; i++)
	{
	  cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, buf[i]);
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_VISITED))
	    continue;
	  if (dfs_visit (cb, blocks, header))
	    {
	      cycle = 1;
	      break;
	    }
	}
    }

#ifdef DEBUG_CYCLE
  if (cycle)
    fprintf (stderr, "cycle in region (header %d)\n", header->id);
#endif

  return cycle;
}
