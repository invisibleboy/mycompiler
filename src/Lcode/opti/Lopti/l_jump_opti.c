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
 *      File :          l_jump_opti.c
 *      Description :   jump optimization
 *      Info required : dominator information
 *      Creation Date : April, 1991
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

#undef DEBUG_BRANCH_TO_NEXT_BLOCK
#undef DEBUG_BRANCHES_TO_SAME_TARGET
#undef DEBUG_BRANCH_TO_UNCOND_BRANCH
#undef DEBUG_MERGE
#undef DEBUG_COMBINE_LABELS
#undef DEBUG_BRANCH_TARGET_EXP
#undef DEBUG_BRANCH_SWAP
#undef DEBUG_BRANCH_PRED
#undef DEBUG_REMOVE_DECIDABLE_BRANCHES

#define ERR     stderr

/*
 *      These are somewhat random numbers, but don't mess with em
 *      unless you know what you are doing!!!
 */
#define L_MAX_CB_SIZE_FOR_JUMP_OPTI             256	/* was 100 */
#define L_MIN_WEIGHT_FOR_BRANCH_EXP             100	/* was 100 */
#define L_MIN_WEIGHT_FOR_LOOP_BRANCH_EXP        500	/* was 500 */
#define L_MIN_COND_BR_TAKEN_RATIO               0.51	/* was .51 */
#define L_MIN_LOOP_COND_BR_TAKEN_RATIO          0.75	/* was .75 */
#define L_MIN_FLOW_RATIO                        0.25	/* was .25 */
#define L_MIN_LOOP_FLOW_RATIO                   0.55	/* was .65 */
#define L_MIN_TARGET_CB_RATIO                   0.10	/* was .10 */

/*
 * Redundant branch removal
 * ----------------------------------------------------------------------
 */

int
L_jump_elim_branch_to_next_block (L_Func * fn, int flags)
{
  int change;
  double weight;
  L_Cb *cb, *next_cb, *target_cb;
  L_Oper *last;
  L_Flow *flow, *flow2;
  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = next_cb)
    {
      next_cb = cb->next_cb;
      if (!(last = cb->last_op))
	continue;
      if (L_uncond_branch_opcode (last) &&
	  (!L_is_predicated (last) ||
	   L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU)))
	{
	  target_cb = last->src[0]->value.cb;
	  if (target_cb != next_cb)
	    continue;
	  /* REH 7/20/95 cannot jump optimize off region transitions */
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY) ||
	      L_EXTRACT_BIT_VAL (target_cb->flags, L_CB_EXIT_BOUNDARY))
	    continue;
#ifdef DEBUG_BRANCH_TO_NEXT_BLOCK
	  fprintf (ERR, "Elim branch to next block (jump) (cb %d)\n", cb->id);
#endif
	  flow = L_find_last_flow (cb->dest_flow);
	  flow2 = L_find_matching_flow (next_cb->src_flow, flow);
	  flow->cc = 0;
	  flow2->cc = 0;
	  cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
	  L_delete_oper (cb, last);
	  STAT_COUNT ("L_jump_elim_uncond", 1, cb);
	  change++;
	}
      else if (L_cond_branch (last))
	{
	  target_cb = L_branch_dest (last);
	  if (target_cb != next_cb)
	    continue;
	  /* REH 7/20/95 cannot jump optimize off region transitions */
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY) ||
	      L_EXTRACT_BIT_VAL (target_cb->flags, L_CB_EXIT_BOUNDARY))
	    continue;
#ifdef DEBUG_BRANCH_TO_NEXT_BLOCK
	  fprintf (ERR, "Elim branch to next block (condbr) (cb %d)\n",
		   cb->id);
#endif

	  /* Delete dest_flow corresponding to taken path of cond br , also
	     delete corresonding src flow in next_cb */

	  flow = L_find_flow_for_branch (cb, last);
	  weight = flow->weight;
	  flow2 = L_find_matching_flow (next_cb->src_flow, flow);
	  cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
	  next_cb->src_flow = L_delete_flow (next_cb->src_flow, flow2);

	  /* modify weight of last flow of cb and weight of corresp src flow */
	  flow = L_find_last_flow (cb->dest_flow);
	  flow2 = L_find_matching_flow (next_cb->src_flow, flow);
	  flow->weight += weight;
	  flow2->weight += weight;

	  L_delete_oper (cb, last);
	  STAT_COUNT ("L_jump_elim_cond", 1, cb);
	  change++;
	}
    }
  return change;
}

int
L_jump_combine_branches_to_same_target (L_Func * fn, int flags)
{
  int change;
  double weight;
  L_Cb *cb, *target_cb1, *target_cb2;
  L_Oper *last, *prev_last;
  L_Flow *flow, *flow2;
  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      last = cb->last_op;
      /* REH 7/20/95 cannot jump optimize off region transitions */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	continue;
      if (L_uncond_branch (last))
	{
	  prev_last = last->prev_op;
	  if (!L_cond_branch_opcode (prev_last))
	    continue;
	  target_cb1 = last->src[0]->value.cb;
	  target_cb2 = prev_last->src[2]->value.cb;
	  if (target_cb1 != target_cb2)
	    continue;
	  if (!(PG_equivalent_predicates_ops (last, prev_last) ||
		!last->pred[0]))
	    continue;
#ifdef DEBUG_BRANCHES_TO_SAME_TARGET
	  fprintf (ERR, "Combine branches to same target (cb %d)\n", cb->id);
#endif
	  flow = L_find_second_to_last_flow (cb->dest_flow);
	  weight = flow->weight;
	  flow2 = L_find_matching_flow (target_cb1->src_flow, flow);
	  cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
	  target_cb1->src_flow = L_delete_flow (target_cb1->src_flow, flow2);
	  L_delete_oper (cb, prev_last);
	  flow = L_find_last_flow (cb->dest_flow);
	  flow2 = L_find_matching_flow (target_cb1->src_flow, flow);
	  flow->weight += weight;
	  flow2->weight += weight;
	  change += 1;
	}
    }

  return change;
}

/*==========================================================================*/
/*
 *     Search for block with only uncond branch in it, then change all branches
 *     to the block to the target of the uncond branch.
 */
/*==========================================================================*/

int
L_jump_combine_branch_to_uncond_branch (L_Func * fn, int flags)
{
  int change;
  L_Cb *cb, *src_cb, *target_cb, *next_cb;
  L_Flow *flow, *flow2, *new_flow, *next_flow;
  L_Oper *op, *new_op;

  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = next_cb)
    {
      int incomplete = 0;

      next_cb = cb->next_cb;
      if (!L_only_uncond_branch_in_block (cb))
	continue;
      target_cb = cb->first_op->src[0]->value.cb;
      if (target_cb == cb)	/* otherwise, we may infinite loop */
	continue;

#ifdef DEBUG_BRANCH_TO_UNCOND_BRANCH
      fprintf (ERR, "Combine all branches to cb %d (target %d)\n",
	       cb->id, target_cb->id);
#endif
      for (flow = cb->src_flow; flow != NULL; flow = next_flow)
	{
	  next_flow = flow->next_flow;
	  src_cb = flow->src_cb;

	  flow2 = L_find_matching_flow (src_cb->dest_flow, flow);

	  if ((op = L_find_branch_for_flow (src_cb, flow2)))
	    {
	      L_change_branch_dest (op, cb, target_cb);
	    }
	  else if (!L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_SOFTPIPE) &&
		   !L_find_attr (src_cb->attr, "L_CB_SOFTPIPE"))
	    {
	      /* src_cb fell through into trampoline cb.  Add an
	       * explicit jump to the eventual destination
	       */
	      new_op = L_create_new_op (Lop_JUMP);
	      new_op->src[0] = L_new_cb_operand (target_cb);
	      L_insert_oper_after (src_cb, src_cb->last_op, new_op);
	      flow2->cc = 1;
	    }
	  else
	    {
	      /* Can't insert new unconditional jump into softpipe loop. */

	      incomplete = 1;
	      continue;
	    }

	  flow2->dst_cb = target_cb;
	  new_flow = L_new_flow (flow2->cc, src_cb, target_cb, flow2->weight);
	  target_cb->src_flow = L_concat_flow (target_cb->src_flow, new_flow);
	  cb->src_flow = L_delete_flow (cb->src_flow, flow);
	  change++;
	}

      /* If updated along all incoming paths, can delete trampoline block */

      if (!incomplete)
	{
	  if (cb->src_flow)
	    L_punt ("L_jump_combine_branch_to_uncond_branch: "
		    "bb should have no src");

	  /* delete outgoing flow from cb */
	  flow = cb->dest_flow;
	  flow2 = L_find_matching_flow (target_cb->src_flow, flow);
	  cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
	  target_cb->src_flow = L_delete_flow (target_cb->src_flow, flow2);

	  /* delete the cb */
	  L_delete_cb (fn, cb);
	}
    }

  return change;
}

/*==========================================================================*/
/*
 *  cb1 and cb2 are consecutive blocks in the layout, cb1 does not end with
 *  a branch operation, cb1 has 1 successor (cb2), and cb2 has 1 predecessor
 *  (cb1), can move all operations from cb1 into cb2 and eliminate cb1.
 */
/*==========================================================================*/

int
L_jump_merge_always_successive_blocks (L_Func * fn, int flags)
{
  int change;
  L_Cb *cb, *next_cb;
  L_Oper *op, *prev;
  L_Flow *flow, *max_flow, *next_flow, *old_flow, *match, *new_flow;
  change = 0;

  if (fn->first_cb == NULL)
    L_punt ("L_jump_merge_always_successive_blocks: empty funct");

  /* omit first cb of function from this opti if SB not formed */
  if (flags & L_JUMP_ALLOW_SUPERBLOCKS)
    cb = fn->first_cb;
  else
    cb = fn->first_cb->next_cb;

  for (; cb != NULL; cb = next_cb)
    {
      if (!(next_cb = cb->next_cb))
	continue;

      /* REH 7/20/95 - Don't merge successive blocks if one is a boundary,
         MCM 11/6/00 - Don't merge successive blocks into fn prologue
         block because pred save blk insts get reordered with pred clears. */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_PROLOGUE) ||
	  L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY) ||
	  L_EXTRACT_BIT_VAL (next_cb->flags, L_CB_EXIT_BOUNDARY))
	continue;

      /* do not expand blocks into softpipe or potential softpipe loops */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) ||
	  L_find_attr (cb->attr, "L_CB_SOFTPIPE"))
	continue;
      if (L_EXTRACT_BIT_VAL (next_cb->flags, L_CB_SOFTPIPE) ||
	  L_find_attr (next_cb->attr, "L_CB_SOFTPIPE"))
	continue;

      if (L_empty_block (cb))
	continue;
      if (L_empty_block (next_cb))
	continue;
      if ((!L_EXTRACT_BIT_VAL (flags, L_JUMP_ALLOW_SUPERBLOCKS)) &&
	  (L_general_branch_opcode (cb->last_op) ||
	   L_check_branch_opcode (cb->last_op)))
	continue;

      /* never make copy of rts in Lcode */
      if (L_subroutine_return_opcode (cb->last_op))
	continue;
      if (L_subroutine_return_opcode (next_cb->last_op))
	continue;

      /* next cb has 1 predecessor */
      if (!L_single_predecessor_cb (next_cb))
	continue;

      /* make sure flow important enough */
      flow = L_find_last_flow (cb->dest_flow);
      max_flow = L_find_max_weight_flow (cb->dest_flow);
      if (flow->dst_cb != next_cb)
	continue;
      if ((max_flow->weight != 0.0) && (flow != max_flow))
	continue;

      /* make sure resultant cb is not too large */
      if ((L_cb_size (cb) + L_cb_size (next_cb) >
	   L_MAX_CB_SIZE_FOR_JUMP_OPTI))
	continue;

      /* don't expand a hyperblock for now */
      if (L_EXTRACT_BIT_VAL (next_cb->flags, L_CB_HYPERBLOCK))
	continue;

      /* don't expand loop header, this check shouldn't be necessary, 
         but oh well */
      if (L_EXTRACT_BIT_VAL (next_cb->flags, L_CB_LOOP_HEADER))
	continue;

      /* don't copy either current or next end with jump_rg */
      if (L_register_branch_opcode (cb->last_op))
	continue;
      if (L_register_branch_opcode (next_cb->last_op))
	continue;

      if ((!next_cb->src_flow) || (next_cb->src_flow->src_cb != cb))
	L_punt
	  ("L_merge_always_successive_blocks: next cb incorrect src_flow");

#ifdef DEBUG_MERGE
      fprintf (ERR, "Merge blocks %d and %d together\n", cb->id, next_cb->id);
#endif

      /* special case here, uncond branch to next cb */
      op = cb->last_op;
      if (L_uncond_branch (op) && (L_branch_dest (op) == next_cb))
	L_delete_oper (cb, op);

      /* move opers down */
      for (op = cb->last_op; op != NULL; op = prev)
	{
	  prev = op->prev_op;
	  L_move_op_to_start_of_block (cb, next_cb, op);
	}

      /* move flows down */
      old_flow = next_cb->dest_flow;
      next_cb->dest_flow = NULL;
      for (flow = cb->dest_flow; flow; flow = next_flow)
	{
	  /* don't move last one */
	  if (!(next_flow = flow->next_flow))
	    break;

	  match = L_find_matching_flow (flow->dst_cb->src_flow, flow);
	  match->src_cb = next_cb;
	  new_flow =
	    L_new_flow (flow->cc, next_cb, flow->dst_cb, flow->weight);
	  next_cb->dest_flow = L_concat_flow (next_cb->dest_flow, new_flow);
	  cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
	}
      next_cb->dest_flow = L_concat_flow (next_cb->dest_flow, old_flow);

      /* copy important flags down */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	{
	  next_cb->flags = L_SET_BIT_FLAG (next_cb->flags, L_CB_HYPERBLOCK);
	  if (!(next_cb->dest_flow))
	    next_cb->flags = L_SET_BIT_FLAG (next_cb->flags,
					     L_CB_HYPERBLOCK_NO_FALLTHRU);
	}
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_PROLOGUE))
	next_cb->flags = L_SET_BIT_FLAG (next_cb->flags, L_CB_PROLOGUE);

      next_cb->attr = L_merge_attr_lists (next_cb->attr, cb->attr);

      change += 1;
    }

  return change;
}

/*==========================================================================*/
/*
 *      Delete empty cb labels, redirect all jumps to next cb label
 */
/*==========================================================================*/


int
L_jump_combine_labels (L_Func * fn, int flags)
{
  int change;
  L_Cb *cb, *src_cb, *next_cb;
  L_Flow *flow, *next_flow, *new_flow, *match_flow;
  L_Oper *op;
  change = 0;

  for (cb = fn->first_cb; cb != NULL; cb = next_cb)
    {
      next_cb = cb->next_cb;
      if (!L_empty_block (cb))
	continue;
#ifdef DEBUG_COMBINE_LABELS
      fprintf (ERR, "Combine label of cb %d into bb %d\n", cb->id,
	       next_cb->id);
#endif

      /* change destflow arcs from cb to next_cb, delete src_flow arcs from
         cb and insert corresponding flow arcs for next_cb */
      for (flow = cb->src_flow; flow != NULL; flow = next_flow)
	{
	  next_flow = flow->next_flow;
	  src_cb = flow->src_cb;
	  match_flow = L_find_matching_flow (src_cb->dest_flow, flow);
	  if ((op = L_find_branch_for_flow (src_cb, match_flow)))
	    L_change_branch_dest (op, cb, next_cb);
	  match_flow->dst_cb = next_cb;
	  new_flow = L_new_flow (match_flow->cc, src_cb,
				 next_cb, match_flow->weight);
	  next_cb->src_flow = L_concat_flow (next_cb->src_flow, new_flow);
	  cb->src_flow = L_delete_flow (cb->src_flow, flow);
	}

      next_cb->attr = L_merge_attr_lists (next_cb->attr, cb->attr);

      /* delete flow arc from cb to next_cb */
      flow = cb->dest_flow;
      match_flow = L_find_matching_flow (next_cb->src_flow, flow);
      cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
      next_cb->src_flow = L_delete_flow (next_cb->src_flow, match_flow);

      /* delete the cb */
      L_delete_cb (fn, cb);

      change += 1;
    }

  return change;
}

/*==========================================================================*/
/*
 *      Expand frequent jumps and likely cond brs and fallthru paths
 */
/*==========================================================================*/

#define NO_EXPANSION    0
#define EXPAND_TAKEN    1
#define EXPAND_FALLTHRU 2

static double expand_ratio = 0.0;
static double expand_flow_ratio = 0.0;
static L_Oper *expand_br = NULL;
static L_Flow *expand_flow = NULL;


/*
 *      return 1 if going to do expansion
 */
static int
L_likely_cb_ending_branch (L_Cb * cb)
{
  L_Oper *last, *prev_last;
  L_Flow *taken_flow, *ft_flow;
  double taken_ratio, ft_ratio;
  int expand_status = 0;

  last = cb->last_op;
  prev_last = last->prev_op;

  if (L_register_branch_opcode (last))
    {
      /* Cannot currently expand a jump_rg */
      ;
    }
  else if (L_cond_branch_opcode (last))
    {
      /* conditional branch, can expand either tk or ft path */
      taken_flow = L_find_second_to_last_flow (cb->dest_flow);
      ft_flow = taken_flow->next_flow;
      if ((taken_flow->weight + ft_flow->weight) == 0.0)
	return NO_EXPANSION;
      taken_ratio =
	taken_flow->weight / (taken_flow->weight + ft_flow->weight);
      ft_ratio = 1.0 - taken_ratio;
      if (taken_ratio > L_MIN_COND_BR_TAKEN_RATIO)
	{
	  expand_status = EXPAND_TAKEN;
	  expand_br = last;
	  expand_flow = taken_flow;
	  expand_ratio = taken_ratio;
	}
      else if (ft_ratio > L_MIN_COND_BR_TAKEN_RATIO)
	{
	  expand_status = EXPAND_FALLTHRU;
	  expand_br = NULL;
	  expand_flow = ft_flow;
	  expand_ratio = ft_ratio;
	}
    }
  else if (L_uncond_branch (last) && L_cond_branch_opcode (prev_last))
    {
      /* this is really a cond branch whose fallthru is not a sequential cb,
	 so treat as prev case */

      taken_flow = L_find_second_to_last_flow (cb->dest_flow);
      ft_flow = taken_flow->next_flow;
      if ((taken_flow->weight + ft_flow->weight) == 0.0)
	return NO_EXPANSION;
      taken_ratio =
	taken_flow->weight / (taken_flow->weight + ft_flow->weight);
      ft_ratio = 1.0 - taken_ratio;
      if (taken_ratio > L_MIN_COND_BR_TAKEN_RATIO)
	{
	  expand_status = EXPAND_TAKEN;
	  expand_br = prev_last;
	  expand_flow = taken_flow;
	  expand_ratio = taken_ratio;
	}
      /* this is kinda funny, mark as taken even though is fallthru path,
         this is because there is an explicit jump there, so it is
         the taken path for that jump */
      else if (ft_ratio > L_MIN_COND_BR_TAKEN_RATIO)
	{
	  expand_status = EXPAND_TAKEN;
	  expand_br = last;
	  expand_flow = ft_flow;
	  expand_ratio = ft_ratio;
	}
    }
  else if (L_uncond_branch (last) &&
	   (!L_general_branch_opcode (prev_last)) &&
	   (!L_check_branch_opcode (prev_last)))
    {
      /* last is jump */
      expand_status = EXPAND_TAKEN;
      expand_br = last;
      expand_flow = L_find_last_flow (cb->dest_flow);
      expand_ratio = 1.0;
    }
  else if (!L_general_branch_opcode (last) &&
	   !L_check_branch_opcode (last) && 
	   cb->dest_flow)
    {
      /* last is not a branch, and current cb not epilogue block */
      expand_status = EXPAND_FALLTHRU;
      expand_br = NULL;
      expand_flow = L_find_last_flow (cb->dest_flow);
      expand_ratio = 1.0;
    }

  return expand_status;
}


/*
 *      A flow you wanna do expansion on has several characteristics:
 *      1. flow->weight > some minimum weight
 *      2. flow is the largest flow of the cb
 *      3. flow->weight is some fraction of the total flows for this cb
 */
static int
L_likely_flow_for_expansion (L_Cb * cb, L_Flow *fl)
{
  L_Flow *max_flow;

  if (fl->weight < L_MIN_WEIGHT_FOR_BRANCH_EXP)
    return (0);

  max_flow = L_find_max_weight_flow (cb->dest_flow);
  if (fl->weight * 1.05 <= max_flow->weight)
    return (0);

  expand_flow_ratio = fl->weight / cb->weight;

  if (expand_flow_ratio < L_MIN_FLOW_RATIO)
    return (0);

  return (1);
}


/*
 *      Last 2 arguments are for return vals
 */
static void
L_find_superblock_loop_flow (L_Cb * cb, L_Cb * target_cb,
			     L_Oper ** p_loop_br, L_Flow ** p_loop_flow,
			     int flags)
{
  L_Flow *flow, *loop_flow = NULL;
  L_Oper *loop_br = NULL;

  /* If we have already optimized loops, no sense going thru the work
     to try to create xtra loops */
  if (L_EXTRACT_BIT_VAL (flags, L_JUMP_ALLOW_LOOP_BODY_EXP))
    return;

  for (flow = target_cb->dest_flow; flow; flow = flow->next_flow)
    {
      if (flow->dst_cb != cb)
	continue;
      /* these cases are ok, sb loop will be formed naturally */
      if (!flow->next_flow)
	continue;
      loop_br = L_find_branch_for_flow (target_cb, flow);
      if (!loop_br)
	L_punt ("L_find_superblock_loop_flow: loop_br not found from flow");
      if (loop_br == target_cb->last_op)
	continue;
      if (flow->weight < L_MIN_WEIGHT_FOR_BRANCH_EXP)
	continue;
      if (L_uncond_branch (loop_br->next_op) && !(loop_br->next_op->next_op))
	continue;
#ifdef DEBUG_BRANCH_TARGET_EXP
      fprintf (ERR, "######Loop br found (op %d) (cb %d) (target_cb %d)\n",
	       (*loop_br)->id, cb->id, target_cb->id);
#endif

      /* sb loop won't be formed naturally, so lets force it */
      loop_flow = flow;
      break;
    }

  if (loop_flow)
    {
      *p_loop_flow = loop_flow;
      *p_loop_br = loop_br;
    }

  return;
}


static int
L_br_expand (L_Flow *expand_fl, int flags)
{
  int opcnt = 0;
  L_Cb *new_cb, *dst_cb;
  L_Flow *flow, *flow2, *new_flow;
  L_Oper *first_copied_oper = NULL, *new_op, *op;
  double o_ratio, n_ratio;
  L_Cb *cb = expand_fl->src_cb;
  L_Cb *target_cb = expand_fl->dst_cb;

  L_Flow *loop_flow = NULL;
  L_Oper *loop_br = NULL;

  if ((target_cb->weight == 0.0) || (target_cb->weight < expand_fl->weight))
    o_ratio = 0.0;
  else
    o_ratio = (target_cb->weight - expand_fl->weight) / target_cb->weight;

  n_ratio = 1.0 - o_ratio;

  /* delete corresponding flow arcs */
  flow = L_find_matching_flow (target_cb->src_flow, expand_fl);
  target_cb->src_flow = L_delete_flow (target_cb->src_flow, flow);
  cb->dest_flow = L_delete_flow (cb->dest_flow, expand_fl);

  /* check for flow in target_cb to cb, which will allow superblock loop */
  L_find_superblock_loop_flow (cb, target_cb, &loop_br, &loop_flow, flags);

  /* 
   * if loop_br and loop_flow are set, there is a superblock loop we
   *  want to force to form while doing unconditional branch
   *  expansion.  
   */

  for (op = target_cb->first_op; op; op = op->next_op)
    {
      new_op = L_copy_operation (op);
      opcnt++;
      if (op == target_cb->first_op)
	first_copied_oper = new_op;
      L_insert_oper_after (cb, cb->last_op, new_op);
      new_op->weight = op->weight * n_ratio;
      op->weight *= o_ratio;
      if (op == loop_br)
	break;
    }

  if (L_func_acc_specs || L_func_contains_dep_pragmas)
    L_adjust_syncs_for_target_expansion (first_copied_oper, target_cb, cb);

  if (loop_flow)
    {
      if (!op)
	L_punt ("L_br_expand: loop_br not found");

      new_cb = L_create_cb (0.0);
      L_add_loop_structure_for_new_cb (new_cb, target_cb);
      L_insert_cb_after (L_fn, cb, new_cb);
      for (op = loop_br->next_op; op; op = op->next_op)
	{
	  new_op = L_copy_operation (op);
	  L_insert_oper_after (new_cb, new_cb->last_op, new_op);
	  new_op->weight = op->weight * n_ratio;
	  op->weight *= o_ratio;
	  opcnt++;
	}
      new_cb->weight = new_cb->first_op->weight;
    }
  else
    {
      new_cb = cb;
    }

  /* if target_cb had fallthru path, 
     insert a jump instr to fallthru path */
  if (L_need_fallthru_path (target_cb))
    {
      if (target_cb == L_fn->last_cb)
	L_punt ("L_br_expand: corrupt funct");
      new_op = L_create_new_op (Lop_JUMP);
      new_op->src[0] = L_new_cb_operand (target_cb->next_cb);
      L_insert_oper_after (new_cb, new_cb->last_op, new_op);
    }

  /* copy all flow arcs from target_cb , add corresponding src arcs */
  for (flow = target_cb->dest_flow; flow; flow = flow->next_flow)
    {
      dst_cb = flow->dst_cb;
      /* add new dst flow */
      new_flow = L_new_flow (flow->cc, cb, dst_cb, 
			     flow->weight * n_ratio);
      cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);
      /* add new src flow */
      new_flow = L_copy_single_flow (new_flow);
      dst_cb->src_flow = L_concat_flow (dst_cb->src_flow, new_flow);
      /* scale original flow and its corresponding src flow */
      flow2 = L_find_matching_flow (dst_cb->src_flow, flow);
      flow->weight *= o_ratio;
      flow2->weight *= o_ratio;
      if (flow == loop_flow)
	break;
    }

  if (loop_flow)
    {
      if (!flow)
	L_punt ("L_br_expand: loop_flow not found");

      for (flow = loop_flow->next_flow; flow; flow = flow->next_flow)
	{
	  dst_cb = flow->dst_cb;
	  /* add new dst flow */
	  new_flow = L_new_flow (flow->cc, new_cb, dst_cb, 
				 flow->weight * n_ratio);
	  new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
	  /* add new src flow */
	  new_flow = L_copy_single_flow (new_flow);
	  dst_cb->src_flow = L_concat_flow (dst_cb->src_flow, new_flow);
	  /* scale original flow and its corresponding src flow */
	  flow2 = L_find_matching_flow (dst_cb->src_flow, flow);
	  flow->weight *= o_ratio;
	  flow2->weight *= o_ratio;
	}
  
      /* add flow arcs between cb and new_cb */
      new_flow = L_new_flow (0, cb, new_cb, new_cb->weight);
      cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);
      new_flow = L_new_flow (0, cb, new_cb, new_cb->weight);
      new_cb->src_flow = L_concat_flow (new_cb->src_flow, new_flow);

      /* redo dominator analysis since added new cb */
      L_do_flow_analysis (L_fn, DOMINATOR);
    }

  /* update weight of target_cb */
  target_cb->weight *= o_ratio;

  return opcnt;
}


int
L_jump_branch_target_expansion (L_Func * fn, int flags)
{
  int change = 0, loop_attr, doall_attr, softpipe_attr, expand_status;
  int opcnt = 0, targ_size;
  double target_ratio;
  L_Cb *cb, *target_cb;
  L_Attr *attr;

  L_do_flow_analysis (fn, DOMINATOR);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* REH 7/20/95 cannot jump optimize region boundary blocks */
      /* do not expand blocks into softpipe or potential softpipe loops */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY) ||
	  L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) ||
	  (cb->weight < L_MIN_WEIGHT_FOR_BRANCH_EXP) ||
	  L_empty_block (cb) ||
	  L_cb_contains_prologue (cb) ||
	  L_cb_contains_epilogue (cb) ||
	  !(expand_status = L_likely_cb_ending_branch (cb)))
	continue;

      /*
       * if superblocks not formed, don't mess up basic block nature
       * of code.
       */

      if (!L_EXTRACT_BIT_VAL (flags, L_JUMP_ALLOW_SUPERBLOCKS) &&
	  (!L_uncond_branch (expand_br) ||
	   L_general_branch_opcode (expand_br->prev_op) ||
	   L_check_branch_opcode (expand_br->prev_op)))
	continue;

      /* not sure if this restriction is necessary */
      if (L_is_predicated (expand_br))
	continue;

      if (!L_likely_flow_for_expansion (cb, expand_flow))
	continue;

      /* don't waste time with empty blocks */
      target_cb = expand_flow->dst_cb;
      if (L_empty_block (target_cb))
	continue;

      targ_size = L_cb_size (target_cb);

#if 0
      /* JWS new heuristic to control code size expn */

      if (target_cb->weight == 0.0)
	continue;

      if ((targ_size > 4) &&
	  ((expand_flow->weight / target_cb->weight) < 0.6))
	continue;

      /* JWS end new heuristic */
#endif
      /* REH 7/20/95 cannot jump optimize region boundary blocks */

      if (L_EXTRACT_BIT_VAL (target_cb->flags, L_CB_BOUNDARY))
	continue;

      if (L_in_cb_DOM_set (cb, target_cb->id) &&
	  !L_EXTRACT_BIT_VAL (flags, L_JUMP_ALLOW_BACKEDGE_EXP))
	continue;

      /* check for loop attr, these are set in Pcode */
      loop_attr = 0;
      doall_attr = 0;
      softpipe_attr = 0;
      for (attr = target_cb->attr; attr; attr = attr->next_attr)
	{
	  if (!strcmp (attr->name, "LOOP"))
	    loop_attr = 1;
	  else if (!strcmp (attr->name, "DOALL"))
	    doall_attr = 1;
	  else if (!strcmp (attr->name, "L_CB_SOFTPIPE"))
	    softpipe_attr = 1;
	}

      if (loop_attr && !Lopti_allow_jump_expansion_of_pcode_loops)
	continue;

      if (doall_attr)
	continue;

      /* do not expand softpipe or potential softpipe loops */
      if (softpipe_attr || 
	  L_EXTRACT_BIT_VAL (target_cb->flags, L_CB_SOFTPIPE))
	continue;

      /* dont expand a loop header unless enabled */
      if (loop_attr ||
	  L_EXTRACT_BIT_VAL (target_cb->flags, L_CB_LOOP_HEADER))
	{
	  if (!L_EXTRACT_BIT_VAL (flags, L_JUMP_ALLOW_LOOP_BODY_EXP))
	    continue;
	  if (expand_status == EXPAND_FALLTHRU)
	    continue;
	  if (expand_flow->weight < L_MIN_WEIGHT_FOR_LOOP_BRANCH_EXP)
	    {
#ifdef DEBUG_BRANCH_TARGET_EXP
	      fprintf (ERR, "*** Exp %d -> %d failed, flow weight\n",
		       cb->id, target_cb->id);
#endif
	      continue;
	    }
	  if (expand_ratio < L_MIN_LOOP_COND_BR_TAKEN_RATIO)
	    {
#ifdef DEBUG_BRANCH_TARGET_EXP
	      fprintf (ERR, "*** Exp %d -> %d failed, min ratio\n",
		       cb->id, target_cb->id);
#endif
	      continue;
	    }
	  if (expand_flow_ratio < L_MIN_LOOP_FLOW_RATIO)
	    {
#ifdef DEBUG_BRANCH_TARGET_EXP
	      fprintf (ERR, "*** Exp %d -> %d failed, min flow ratio\n",
		       cb->id, target_cb->id);
#endif
	      continue;
	    }
	  target_ratio = expand_flow->weight / target_cb->weight;
	  if (target_ratio < L_MIN_TARGET_CB_RATIO)
	    {
#ifdef DEBUG_BRANCH_TARGET_EXP
	      fprintf (ERR, "*** Exp %d -> %d failed, min target ratio\n",
		       cb->id, target_cb->id);
#endif
	      continue;
	    }
	}

      if (target_cb == cb)
	continue;

      /* don't allow to expand a hyperblock for now, messes up pred graph */
      if (L_EXTRACT_BIT_VAL (target_cb->flags, L_CB_HYPERBLOCK))
	continue;

      /* Don't expand if target_cb contains epilogue. */
      if (L_cb_contains_epilogue (target_cb))
	continue;

      /* Don't expand if target_cb ends with jump_rg, they are just nasty */
      if (L_register_branch_opcode (target_cb->last_op))
	continue;

      /* Make sure cb + target_cb not too large. */
      if ((L_cb_size (cb) + targ_size) > L_MAX_CB_SIZE_FOR_JUMP_OPTI)
	continue;

      if (L_uncond_branch (expand_br))
	{
#ifdef DEBUG_BRANCH_TARGET_EXP
	  fprintf (ERR, "Expand uncond branch (cb %d) (target_cb %d)\n",
		   cb->id, target_cb->id);
#endif
	  if (expand_br != cb->last_op)
	    L_punt ("L_do_uncond_branch_exp: "
		    "trying to expand jump that is not last op");

	  /* delete jump op */
	  L_delete_oper (cb, expand_br);
	}
      else if (L_cond_branch (expand_br))
	{
	  L_Cb *ft_cb = NULL;
	  L_Oper *jump = NULL;
	  L_Flow *ft_flow, *ft_flow2;

#ifdef DEBUG_BRANCH_TARGET_EXP
	  fprintf (ERR,
		   "Expand taken path of cond br (cb %d) (target_cb %d)\n",
		   cb->id, target_cb->id);
#endif

	  if (!(ft_flow = expand_flow->next_flow))
	    L_punt ("L_do_cond_branch_exp: corrupt flow list");

	  if (expand_br == cb->last_op)
	    ft_cb = cb->next_cb;
	  else if ((jump = expand_br->next_op))
	    ft_cb = jump->src[0]->value.cb;
	  else
	    L_punt ("L_do_cond_branch_exp: jump not found");

	  /* restructure cbr to go to ft_cb */
	  ft_flow2 = L_find_matching_flow (ft_cb->src_flow, ft_flow);
	  expand_br->com[1] = L_opposite_pred_completer (expand_br->com[1]);
	  L_change_branch_dest (expand_br, target_cb, ft_cb);
	  ft_flow->cc = 1;
	  ft_flow2->cc = 1;
	  
	  /* if jump existed for ft_cb, delete it */
	  if (jump)
	    L_delete_oper (cb, jump);
	}
      else if (!expand_br)
	{
#ifdef DEBUG_BRANCH_TARGET_EXP
	  fprintf (ERR, "Expand fallthru path (cb %d) (target %d)\n",
		   cb->id, target_cb->id);
#endif
	  ;
	}
      else
	{
	  continue;
	}
      
      opcnt+= L_br_expand (expand_flow, flags);
      change++;
      L_do_flow_analysis (fn, DOMINATOR);
    }

#ifdef DEBUG_BRANCH_TARGET_EXP
  if (change)
    fprintf (stderr, ">> BTX created %d ops.\n", opcnt);
#endif
  return change;
}


/*
 * L_jump_branch_swap
 * ----------------------------------------------------------------------
 * Swap a conditional branch and a subsequent predicate-equivalent
 * unconditional branch if either:
 * 1) the unconditonal branch flow is heavier than the conditional branch
 *    flow
 * 2) the conditional branch flow is to the layout-next cb.
 */
int
L_jump_branch_swap (L_Func * fn, int flags)
{
  int old_cc, change = 0;
  L_Cb *cb, *nxt_cb, *br_dst, *jump_dst;
  L_Oper *br, *jump;
  L_Flow *flow_br, *flow_br2, *flow_jump = NULL, *flow_jump2, *last_flow;

  for (cb = fn->first_cb; cb != NULL; cb = nxt_cb)
    {
      nxt_cb = cb->next_cb;

      /* REH 7/20/95 cannot jump optimize region boundary blocks */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

      if (!(jump = cb->last_op) ||
	  !(L_uncond_branch_opcode (jump)) ||
	  !(br = jump->prev_op) ||
	  !L_cond_branch_opcode (br) ||
	  !PG_equivalent_predicates_ops (br, jump))
        continue;

      if (!(flow_br = L_find_flow_for_branch (cb, br)) ||
	  !(flow_jump = flow_br->next_flow))
        L_punt ("L_jump_branch_swap: corrupt flow list cb %d", cb->id);

      br_dst = br->src[2]->value.cb;
      jump_dst = jump->src[0]->value.cb;

      if (jump_dst == cb->next_cb)
	continue;
      if (!((br_dst == cb->next_cb) ||
	    (flow_jump->weight >= (flow_br->weight + ZERO_EQUIVALENT))))
	continue;

      /* special case, superblock loops, don't want to create extra exit
         branch in loop body */
      if (br_dst == cb)
	continue;
#ifdef DEBUG_BRANCH_SWAP
      fprintf (ERR, "Invert br %d and jump %d (cb %d)\n",
	       br->id, jump->id, cb->id);
#endif
      flow_br2 = L_find_matching_flow (br_dst->src_flow, flow_br);
      flow_jump2 = L_find_matching_flow (jump_dst->src_flow, flow_jump);

      /* invert cond br, swap destinations */

      L_negate_compare (br);
      L_change_branch_dest (br, br_dst, jump_dst);
      L_change_branch_dest (jump, jump_dst, br_dst);

      /* re-order flow arcs */
      last_flow = flow_jump->next_flow;

      cb->dest_flow = L_remove_flow (cb->dest_flow, flow_br);
      if (last_flow)
        cb->dest_flow = L_remove_flow (cb->dest_flow, last_flow);
      cb->dest_flow = L_concat_flow (cb->dest_flow, flow_br);
      if (last_flow)
        cb->dest_flow = L_concat_flow (cb->dest_flow, last_flow);

      old_cc = flow_jump->cc;
      flow_jump->cc = flow_br->cc;
      flow_jump2->cc = flow_br->cc;
      flow_br->cc = old_cc;
      flow_br2->cc = old_cc;

      if (!last_flow && (br_dst == nxt_cb))
	{
	  flow_br->cc = 0;
	  flow_br2->cc = 0;
	  L_nullify_operation (jump);
	}

      change++;
    }

  return (change);
}

/* Remove all opers in cb after unconditional branch */

void
L_remove_opers_after_uncond_branch (L_Cb * cb, L_Oper * jump_oper)
{
  L_Cb *tgt_cb;
  L_Oper *oper, *next_oper;
  L_Flow *flow, *jflow, *bflow, *next_flow;

  /* First remove the opers */

  for (oper = jump_oper->next_op; oper; oper = next_oper)
    {
      next_oper = oper->next_op;
      L_delete_oper (cb, oper);
    }

  /* Now remove all flows after new jump flow */

  jflow = L_find_flow_for_branch (cb, jump_oper);

  for (flow = jflow->next_flow; flow; flow = next_flow)
    {
      next_flow = flow->next_flow;

      tgt_cb = flow->dst_cb;
      bflow = L_find_matching_flow (tgt_cb->src_flow, flow);
      cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
      tgt_cb->src_flow = L_delete_flow (tgt_cb->src_flow, bflow);
    }
  return;
}


int
L_remove_decidable_cond_branches (L_Func * fn, int flags)
{
  int change = 0, always_taken;
  L_Cb *cb, *tgt_cb;
  L_Flow *flow, *flow2, *bflow, *next_flow, *next_flow2;
  L_Oper *br_oper, *oper, *next_oper, *jump_oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (flow = cb->dest_flow; flow != NULL; flow = next_flow)
	{
	  L_Operand *s0, *s1;

	  next_flow = flow->next_flow;

	  if (!(br_oper = L_find_branch_for_flow (cb, flow)))
	    continue;

	  if (!L_cond_branch_opcode (br_oper))
	    continue;

	  s0 = br_oper->src[0];
	  s1 = br_oper->src[1];

	  if (!L_is_constant (s0) || !L_is_constant (s1))
	    continue;

	  if (L_int_cond_branch_opcode (br_oper) &&
	      L_is_int_constant (s0) && L_is_int_constant (s1))
	    always_taken = L_evaluate_int_compare (br_oper);
 	  else if (L_flt_cond_branch_opcode (br_oper) &&
		   L_is_flt_constant (s0) && L_is_flt_constant (s1))
	    always_taken = L_evaluate_flt_compare (br_oper);
 	  else if (L_dbl_cond_branch_opcode (br_oper) &&
		   L_is_dbl_constant (s0) && L_is_dbl_constant (s1))
	    always_taken = L_evaluate_dbl_compare (br_oper);
	  else
	    continue;

#ifdef DEBUG_REMOVE_DECIDABLE_BRANCHES
	  fprintf (stderr, "Found branch %d is taken %d in cb %d\n",
		   br_oper->id, always_taken, cb->id);
	  L_print_oper (stderr, br_oper);
#endif
	  change ++;

	  if (always_taken)
	    {
	      /* Change conditional branch to jump */
	      jump_oper = L_create_new_op_using (Lop_JUMP, br_oper);
	      jump_oper->src[0] = L_copy_operand (br_oper->src[2]);
	      L_insert_oper_before (cb, br_oper, jump_oper);

#ifdef DEBUG_REMOVE_DECIDABLE_BRANCHES
	      L_print_oper (stderr, jump_oper);
#endif
	      /* Remove all opers after new jump */
	      L_remove_opers_after_uncond_branch (cb, jump_oper);

		  for (oper = jump_oper->next_op; oper; oper = next_oper)
		    {
		      next_oper = oper->next_op;
#ifdef DEBUG_REMOVE_DECIDABLE_BRANCHES
		      fprintf (stderr, "Deleting  ");
		      L_print_oper (stderr, oper);
#endif
		      L_delete_oper (cb, oper);
		    }

		  /* Remove all flows after new jump flow */
		  for (flow2 = flow->next_flow; flow2; flow2 = next_flow2)
		    {
		      next_flow2 = flow2->next_flow;

		      tgt_cb = flow2->dst_cb;
		      bflow =
			L_find_matching_flow (tgt_cb->src_flow, flow2);
		      cb->dest_flow = L_delete_flow (cb->dest_flow, flow2);
		      tgt_cb->src_flow = L_delete_flow (tgt_cb->src_flow, bflow);
		    }

	      break;
	    }
	  else
	    {
	      /* Branch never taken, delete it */
	      L_delete_oper (cb, br_oper);

	      /* delete outgoing flow from cb */
	      tgt_cb = flow->dst_cb;
	      bflow = L_find_matching_flow (tgt_cb->src_flow, flow);
	      cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
	      tgt_cb->src_flow = L_delete_flow (tgt_cb->src_flow, bflow);
	    }
	}
    }
  return change;
}


/*
 * L_split_node (L_Func *fn, L_Cb *cb)
 * ----------------------------------------------------------------------
 * Create a separate copy of a cb for each incoming flow and redirect
 * control flow, splitting the node. -- JWS 20011020
 */
static void
L_split_node (L_Func * fn, L_Cb * orig_cb)
{
  L_Flow *in_flow, *out_flow;
  double orig_wt, ratio, orig_ratio;

  if (!orig_cb || !orig_cb->src_flow)
    return;

  orig_wt = orig_cb->weight;

  in_flow = orig_cb->src_flow;
  orig_cb->weight = in_flow->weight;
  orig_ratio = orig_wt ? in_flow->weight / orig_wt : 0.0;
  in_flow = in_flow->next_flow;
  orig_cb->src_flow->next_flow = NULL;

  while (in_flow)
    {
      L_Cb *new_cb;
      L_Attr *attr;
      L_Cb *src_cb;
      L_Oper *br;

      ratio = orig_wt ? in_flow->weight / orig_wt : 0.0;
      new_cb = L_create_cb (in_flow->weight);
      L_insert_cb_after (fn, fn->last_cb, new_cb);
      L_copy_block_contents (orig_cb, new_cb);

      attr = L_new_attr ("node-split", 1);
      L_set_int_attr_field (attr, 0, orig_cb->id);
      new_cb->attr = L_concat_attr (new_cb->attr, attr);

      /* Fix exit flows */

      new_cb->dest_flow = L_copy_flow (orig_cb->dest_flow);

      for (out_flow = new_cb->dest_flow; out_flow;
	   out_flow = out_flow->next_flow)
	{
	  L_Cb *dst_cb;
	  out_flow->src_cb = new_cb;
	  dst_cb = out_flow->dst_cb;
	  out_flow->weight *= ratio;
	  if (!out_flow->cc)
	    out_flow->cc = 1;
	  dst_cb->src_flow = L_concat_flow (dst_cb->src_flow,
					    L_copy_single_flow (out_flow));
	}

      /* Fix up block having flow into copy */
      /* 4/28/03 SER: Originally this took place before fixing exit flows,
       * but if this flow is a loop-back to orig_cb, then the flow was changed
       * (incorrectly) before it was copied to the new cb. */
      src_cb = in_flow->src_cb;

      out_flow = L_find_matching_flow (src_cb->dest_flow, in_flow);
      out_flow->dst_cb = new_cb;
      if ((br = L_find_branch_for_flow (src_cb, out_flow)))
	{
	  L_change_branch_dest (br, orig_cb, new_cb);
	}
      else
	{
	  if (!L_has_fallthru_to_next_cb (src_cb) ||
	      (src_cb->next_cb != orig_cb))
	    L_punt ("L_split_node: fallthrough expected");
	  br = L_create_new_op (Lop_JUMP);
	  br->src[0] = L_new_cb_operand (new_cb);
	  L_insert_oper_after (src_cb, src_cb->last_op, br);
	  out_flow->cc = 1;
	}

      if (!L_uncond_branch (orig_cb->last_op))
	{
	  L_Oper *br;

	  out_flow = L_find_last_flow (new_cb->dest_flow);
	  br = L_create_new_op (Lop_JUMP);
	  br->src[0] = L_new_cb_operand (out_flow->dst_cb);
	  L_insert_oper_after (new_cb, new_cb->last_op, br);
	  out_flow->cc = 1;
	}

      new_cb->src_flow = in_flow;
      in_flow->dst_cb = new_cb;
      in_flow = in_flow->next_flow;
      new_cb->src_flow->next_flow = NULL;
    }

  for (out_flow = orig_cb->dest_flow; out_flow;
       out_flow = out_flow->next_flow)
    {
      L_Cb *dst_cb;
      L_Flow *match_flow;

      dst_cb = out_flow->dst_cb;
      match_flow = L_find_matching_flow (dst_cb->src_flow, out_flow);
      out_flow->weight *= orig_ratio;
      match_flow->weight = out_flow->weight;
    }

  return;
}


/*
 * L_split_multidef_branches (L_Func *fn)
 * ----------------------------------------------------------------------
 * Split CB's which contain only a branch, which have multiple
 * predecessors, and whose branch is fed by register compares and/or
 * constant moves.  This transformation aids later hyperblock formation.
 *                              -- JWS 20011020
 */
#define MAX_DEFS 4
void
L_split_multidef_branches (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op;
  L_Flow *src_flow;
  int orig_max_cb_id;

  L_rebuild_src_flow (fn);
  orig_max_cb_id = fn->max_cb_id;
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      L_Operand *reg;
      L_Oper *defoper;
      ITintmax val;
      Set rd_set;
      int count, i, defs, defop[MAX_DEFS];

      /* avoid running off the end of available dataflow info! */
      if (cb->id > orig_max_cb_id)
	break;

      if (L_cb_size (cb) > 4)
	continue;

      /* 4/28/03 SER: Avoid splitting loop headers: creates very ugly
       * control flow that looks a bad case from a textbook. */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_LOOP_HEADER))
	continue;

      count = 0;
      src_flow = cb->src_flow;
      while (src_flow)
	{
	  count++;
	  src_flow = src_flow->next_flow;
	}

      if (count >= 2)
	{
	  for (op = cb->first_op; op; op = op->next_op)
	    {
	      if (!L_int_beq_branch_opcode (op) &&
		  !L_int_bne_branch_opcode (op))
		continue;

	      if (!L_is_reg_const_cond_br (op))
		continue;

	      val = L_pred_compare_get_int_const (op);

	      if (val != 0 && val != 1)
		continue;

	      reg = L_pred_compare_get_reg_operand (op);

	      rd_set = L_get_oper_RIN_defining_opers (op, reg);

	      defs = Set_size (rd_set);

	      if (defs > MAX_DEFS || defs < 2)
		{
		  Set_dispose (rd_set);
		  continue;
		}

	      Set_2array (rd_set, defop);

	      Set_dispose (rd_set);

	      for (i = 0; i < defs; i++)
		{
		  defoper = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						       defop[i]);

		  if (L_int_comparison_opcode (defoper))
		    break;

		  if (L_move_opcode (defoper) &&
		      L_is_int_constant (defoper->src[0]) &&
		      ((defoper->src[0]->value.i == 0) ||
		       (defoper->src[0]->value.i == 1)))
		    break;
		}

	      /* Perform the opti only if we find constant
	       * moves or register compares feeding the branch.
	       */

	      if (i != defs)
		break;
	    }

	  if (op && (L_cb_size (cb) <= 4))
	    {
	      L_split_node (fn, cb);
	      STAT_COUNT ("L_split_multidef_branches", 1, cb);
	    }
	}
    }

  L_rebuild_src_flow (fn);
}


