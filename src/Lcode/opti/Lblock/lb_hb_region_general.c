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
 *      File :          l_region_general.c
 *      Description :   Identify from non-loop regions 
 *      Creation Date : February 1994
 *      Authors :       Scott Mahlke, Wen-mei Hwu
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"

#define ERR	stderr

#undef DEBUG_CB_INFO
#undef DEBUG_BLOCK_SEL
#undef PRINT_REGION_SUMMARY

/*
 *    General hyperblock formation parameters
 */

#define LB_MAX_CB_IN_HYPERBLOCK		128

/*===========================================================================*/
/*
 *      Global vars
 */
/*===========================================================================*/

LB_Cb_Info *LB_hb_cb_info = NULL;
static LB_hb_Sort_List *LB_hb_cb_sort_buf = NULL;
static Set LB_hb_nested_region_blocks = NULL;

int LB_hb_curr_slots_used = 0;
int LB_hb_curr_slots_avail = 0;
int LB_hb_curr_dep_height = 0;




/*===========================================================================*/
/*
 *    Calculate cb info
 */
/*===========================================================================*/

static int
LB_hb_calc_cb_flags (L_Cb * cb)
{
  int flags;
  L_Oper *oper;

  flags = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_subroutine_call_opcode (oper))
	{
	  flags = L_SET_BIT_FLAG (flags, L_TRACEREGION_FLAG_HAS_JSR);
	  if (!L_side_effect_free_sub_call (oper))
	    flags = L_SET_BIT_FLAG (flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR);
	}
      if (L_general_store_opcode (oper) && L_pointer_store (oper))
	{
	  flags = L_SET_BIT_FLAG (flags, L_TRACEREGION_FLAG_HAS_POINTER_ST);
	}
    }
  return (flags);
}

/* Assume most control dependences will be optimized away after
 * hyperblock formation and ILP opti.
 *
 * Remove all liveout-based and memory dependences to branches.
 * (This is to emulate the old scheduler's parameter settings,
 *  which ignored liveout and memory dependences on branches
 * and JSR's).  Based on Scott Mahlke's suggestions -JCG 6/99
 */
static void
LB_remove_deps_assumed_optimized_away (SM_Cb * sm_cb)
{
  SM_Reg_Action *ctrl_action, *mem_action, *liveout_action;
  SM_Action_Qentry *action_qentry;
  SM_Oper *ctrl_sm_op;
  unsigned int mdes_flags;

  /* For every branch (ctrl def) in the cb, remove all dependences 
   * to implicit reg uses (liveout) and to memory srcs/dest (if exist).
   */
  for (ctrl_action = sm_cb->ctrl_rinfo->first_def; ctrl_action;
       ctrl_action = ctrl_action->next_def)
    {
      /* Get the mdes flags for the branch this ctrl_action belongs to */
      ctrl_sm_op = ctrl_action->sm_op;
      mdes_flags = ctrl_sm_op->mdes_flags;

      /* JWS -- JSR deps won't go away! */
      if (mdes_flags & OP_FLAG_JSR)
	continue;

      /* For all control operations, remove all memory dependence, 
       * in and out */
      if ((mem_action = ctrl_sm_op->ext_dest[SM_MEM_ACTION_INDEX]) != NULL)
	{
	  while (mem_action->first_dep_in != NULL)
	    SM_delete_dep (mem_action->first_dep_in);
	  while (mem_action->first_dep_out != NULL)
	    SM_delete_dep (mem_action->first_dep_out);
	}
      if ((mem_action = ctrl_sm_op->ext_src[SM_MEM_ACTION_INDEX]) != NULL)
	{
	  while (mem_action->first_dep_in != NULL)
	    SM_delete_dep (mem_action->first_dep_in);
	  while (mem_action->first_dep_out != NULL)
	    SM_delete_dep (mem_action->first_dep_out);
	}
      /* For conditional branches and jumps, remove all dependences
       * to implicit sources (liveout variables).
       */
      if (mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP))
	{
	  for (action_qentry = ctrl_sm_op->implicit_srcs->first_qentry;
	       action_qentry != NULL;
	       action_qentry = action_qentry->next_qentry)
	    {
	      liveout_action = action_qentry->action;
	      while (liveout_action->first_dep_in != NULL)
		SM_delete_dep (liveout_action->first_dep_in);
	      while (liveout_action->first_dep_out != NULL)
		SM_delete_dep (liveout_action->first_dep_out);
	    }
	}
    }
  return;
}

int
LB_hb_calc_cb_dep_height (L_Cb * cb)
{
  int max;
  SM_Cb *sm_cb;

  /* Ported to use SM instead of old scheduler. 
   * Cannot trust dataflow, so use 'fake' dataflow that conservatively
   * answers dataflow questions and then do pass to agressively
   * remove dependences that we assume will be optimized away -JCG 6/99 
   */
  SM_use_fake_dataflow_info = 1;

  /* Create a sm_cb for this cb.  This creates the rinfo table
   * and builds all the dependences for this cb.
   */

  sm_cb = SM_new_cb (lmdes, cb, SM_PREPASS | SM_DHASY);

  /* Assume most control dependences will be optimized away after
   * hyperblock formation and ILP opti.
   */
  LB_remove_deps_assumed_optimized_away (sm_cb);

  /* Calculate early times for all operations and return
   * the maximum of all the early times calculated (this is the dep height)
   */
  max = SM_calculate_early_times (sm_cb, 0);

  /* Delete the sm_cb, done with it */
  SM_delete_cb (sm_cb);

  /* Reset flag, everyone else really wants to use dataflow info */
  SM_use_fake_dataflow_info = 0;

  return (max + 1);
}

void
LB_hb_print_cb_info (FILE * F, L_Cb * cb)
{
  fprintf (F, "CB %d\n", cb->id);
  fprintf (F, "\tflags: ");
  /* probably don't need this function */
  /*  LB_hb_print_region_flags(F, LB_hb_cb_info[cb->id].flags);  */
  fprintf (F, "\tdep_height = %d\n", LB_hb_cb_info[cb->id].dep_height);
  fprintf (F, "\tslots_used = %d\n", LB_hb_cb_info[cb->id].slots_used);
}

void
LB_hb_print_all_cb_info (FILE * F, L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    LB_hb_print_cb_info (F, cb);
}

void
LB_hb_find_cb_info (L_Func * fn)
{
  L_Cb *cb;

  SM_ignore_pred_analysis = 1;
  /* allocate sort buffer */
  LB_hb_cb_sort_buf =
    (LB_hb_Sort_List *) Lcode_malloc (sizeof (LB_hb_Sort_List) * fn->n_cb);
  if (LB_hb_cb_sort_buf == NULL)
    L_punt ("L_find_general_regions: malloc out of space");
  LB_hb_sort_cb_by_weight (fn);

  /* allocate array indexed by cb id to hold cb info */
  LB_hb_cb_info =
    (LB_Cb_Info *) Lcode_malloc (sizeof (LB_Cb_Info) *
				     (fn->max_cb_id + 1));
  if (LB_hb_cb_info == NULL)
    L_punt ("L_find_general_regions: malloc out of space");


  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      LB_hb_cb_info[cb->id].flags = LB_hb_calc_cb_flags (cb);
      LB_hb_cb_info[cb->id].dep_height = LB_hb_calc_cb_dep_height (cb);
      LB_hb_cb_info[cb->id].slots_used = L_cb_size (cb);
    }

#ifdef DEBUG_CB_INFO
  LB_hb_print_all_cb_info (ERR, fn);
#endif
}

/* Can predicate if no prologue, epilogue, rts */
int
LB_hb_can_predicate_cb (L_Cb * cb)
{
  L_Oper *oper;

  if (cb == NULL)
    L_punt ("L_can_predicate_cb: cb is NULL");

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_subroutine_return_opcode (oper))
	return 0;
      if (L_is_opcode (Lop_PROLOGUE, oper))
	return 0;
      if (L_is_opcode (Lop_EPILOGUE, oper))
	return 0;
    }
  return 1;
}

static double
LB_hb_scale_weight_ratio_for_hazards (double val, L_Cb * cb)
{
  int flags;
  double scaled_val;

  flags = LB_hb_cb_info[cb->id].flags;
  scaled_val = val;

  if (L_EXTRACT_BIT_VAL (flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR))
    scaled_val *= LB_hb_unsafe_jsr_priority_penalty;
  else if (L_EXTRACT_BIT_VAL (flags, L_TRACEREGION_FLAG_HAS_JSR))
    scaled_val *= LB_hb_safe_jsr_priority_penalty;
  if (L_EXTRACT_BIT_VAL (flags, L_TRACEREGION_FLAG_HAS_POINTER_ST))
    scaled_val *= LB_hb_pointer_st_priority_penalty;

  return (scaled_val);
}

int
LB_hb_cb_contains_excludable_hazard (L_Cb * cb, int main_path)
{
  int flags;

  flags = LB_hb_cb_info[cb->id].flags;
  if (L_EXTRACT_BIT_VAL (flags, L_TRACEREGION_FLAG_HAS_UNSAFE_JSR))
    {
      switch (LB_hb_unsafe_jsr_hazard_method)
	{
	case LB_HAZARD_EXCLUDE_ALL:
	  return (1);
	case LB_HAZARD_EXCLUDE_NON_TRACE:
	  if (!main_path)
	    return (1);
	  break;
	case LB_HAZARD_EXCLUDE_HEURISTIC:
	  /* The path priority should be lower so that will take care
             of exclusion */
	  break;
	case LB_HAZARD_IGNORE:
	  break;
	default:
	  break;
	}
    }
  else if (L_EXTRACT_BIT_VAL (flags, L_TRACEREGION_FLAG_HAS_JSR))
    {
      switch (LB_hb_safe_jsr_hazard_method)
	{
	case LB_HAZARD_EXCLUDE_ALL:
	  return (1);
	case LB_HAZARD_EXCLUDE_NON_TRACE:
	  if (!main_path)
	    return (1);
	  break;
	case LB_HAZARD_EXCLUDE_HEURISTIC:
	  /* The path priority should be lower so that will take care
             of exclusion */
	  break;
	case LB_HAZARD_IGNORE:
	  break;
	default:
	  break;
	}
    }
  if (L_EXTRACT_BIT_VAL (flags, L_TRACEREGION_FLAG_HAS_POINTER_ST))
    {
      switch (LB_hb_pointer_st_hazard_method)
	{
	case LB_HAZARD_EXCLUDE_ALL:
	  return (1);
	case LB_HAZARD_EXCLUDE_NON_TRACE:
	  if (!main_path)
	    return (1);
	  break;
	case LB_HAZARD_EXCLUDE_HEURISTIC:
	  /* The path priority should be lower so that will take care
             of exclusion */
	  break;
	case LB_HAZARD_IGNORE:
	  break;
	default:
	  break;
	}
    }

  return (0);
}

int
LB_hb_select_blocks (L_Cb * cb, L_Cb * header, Set * blocks,
		      Set avail_blocks, int main_path, double path_ratio,
		      int flags, LB_TraceRegion_Header * tr_header)
{
  int new_slots_used, new_slots_avail,
    in_nested_region, mpincl = 0;
  double pr1, pr2;
  L_Cb *dst_cb1, *dst_cb2 = NULL;
  LB_TraceRegion *region;

  /* already in current hyperblock */
  if (Set_in (*blocks, cb->id))
    return 1;

  if (!(in_nested_region = Set_in (LB_hb_nested_region_blocks, cb->id)))
    {
      /* not in nested region, do normal checks */

      if (cb->weight < LB_hb_min_cb_weight)
	return 0;

      if (LB_hb_ignore_block (cb))
	return 0;

      /* check for already selected blocks which cannot be subsumed */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) ||
	  L_EXTRACT_BIT_VAL (cb->flags, L_CB_SUPERBLOCK))
	return 0;
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_LOOP) &&
	  !L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	return 0;
      if (LB_hb_is_single_block_loop (cb) &&
	  !L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	return 0;
      if (((flags & LB_BLOCK_SEL_NO_NESTED_HAMMOCKS) ||
	   (flags & LB_BLOCK_SEL_NO_NESTED_LOOPS)) &&
	  (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_LOOP) ||
	   L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_HAMMOCK)))
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR, "   REJECT (cb %d) nesting\n", cb->id);
#endif
	  return 0;
	}

      if (Set_size (*blocks) >= LB_MAX_CB_IN_HYPERBLOCK)
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR, "   REJECT (cb %d) max blocks %d exceeded\n",
		   cb->id, LB_MAX_CB_IN_HYPERBLOCK);
#endif
	  return 0;
	}

      /* header must dominate the block */
      if (!L_in_cb_DOM_set (cb, header->id))
	return 0;

      /* doesn't contain any instrs that cannot predicate */
      if (!LB_hb_can_predicate_cb (cb))
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR, "   REJECT (cb %d) cannot predicate\n", cb->id);
#endif
	  return 0;
	}

      /* no jrg for now */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HAS_JRG))
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR, "   REJECT (cb %d) indirect jump\n", cb->id);
#endif
	  return 0;
	}

      /* Check for hazard */
      if (LB_hb_cb_contains_excludable_hazard (cb, main_path))
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR, "   REJECT (cb %d) hazard\n", cb->id);
#endif
	  return 0;
	}

      /* if avail blocks exists, cb must be a member */
      if (avail_blocks && !Set_in (avail_blocks, cb->id))
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR, "   REJECT (cb %d) unavailable\n",
		   cb->id);
#endif
	  return 0;
	}

      /* Compare the weight of the cb with that of the header block */
      if (!main_path)
	{
	  double w_ratio = cb->weight / header->weight;
	  w_ratio = LB_hb_scale_weight_ratio_for_hazards (w_ratio, cb);
	  if (w_ratio < LB_hb_block_min_weight_ratio)
	    {
#ifdef DEBUG_BLOCK_SEL
	      fprintf (ERR, "   REJECT (cb %d) weight ratio (%0.3f < %0.3f)\n", 
		       cb->id, w_ratio, LB_hb_block_min_weight_ratio);
#endif
	      return 0;
	    }
	}
    }
  else
    {
      /* Inside nested region, just do some error checking */

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	L_punt ("L_select_blocks: nested cb is already a gen hyperblock");
      if (!L_in_cb_DOM_set (cb, header->id))
	L_punt ("L_select_blocks: header doesnt dom nested cb ");
      if (!LB_hb_can_predicate_cb (cb))
	L_punt ("L_select_blocks: cannot predicate nested cb");
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HAS_JRG))
	L_punt ("L_select_blocks: nested cb contains jrg");
    }

  /*
   *        Check if cb is a header for existing region, if so, must
   *  include all cb's from nested region in this region.
   */

  if ((L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_LOOP) ||
       L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_HAMMOCK)) &&
      (region = LB_find_traceregion_by_cb (tr_header, cb)))
    {
      Set tr_cbs;

#ifdef DEBUG_BLOCK_SEL
      fprintf (ERR, "Adding header block %d of nested region\n", cb->id);
#endif
      tr_cbs = LB_return_cbs_region_as_set (region);
      LB_hb_nested_region_blocks =
	Set_union_acc (LB_hb_nested_region_blocks, tr_cbs);
      Set_dispose (tr_cbs);
    }

  /*
   *        Block satisfies all criteria, so add to Block set
   */

  *blocks = Set_add (*blocks, cb->id);

  LB_hb_curr_slots_used += LB_hb_cb_info[cb->id].slots_used;

  if (main_path)
    {
      LB_hb_curr_dep_height += LB_hb_cb_info[cb->id].dep_height;
      LB_hb_curr_slots_avail +=
	(LB_hb_cb_info[cb->id].dep_height * LB_hb_issue_width);
    }

#ifdef DEBUG_BLOCK_SEL
  fprintf (ERR, "   ACCEPT (cb %d) ", cb->id);
  fprintf (ERR, "dep height %d slots (%d/%d) ",
	   LB_hb_curr_dep_height, LB_hb_curr_slots_used,
	   LB_hb_curr_slots_avail);
  fprintf (ERR, "main_path %d path_ratio %7.6f\n", main_path,
	   path_ratio);
#endif

  /*
   *        Consider successor CB's
   */

  {
    L_Flow *flow1, *flow2;

    if (!(flow1 = cb->dest_flow))
      L_punt ("L_select_blocks: cb %d has no dest flows", cb->id);

    if ((flow2 = flow1->next_flow))
      {
	if (flow2->next_flow)
	  L_punt ("L_select_blocks: too many arcs leaving cb %d", cb->id);
	
	if (flow2->weight > flow1->weight)
	  {
	    flow1 = flow2;
	    flow2 = cb->dest_flow;
	  }

	pr1 = path_ratio * (flow1->weight / cb->weight);
	pr2 = path_ratio * (flow2->weight / cb->weight);
	dst_cb2 = flow2->dst_cb;
      }
    else
      {
	dst_cb2 = NULL;
	pr1 = path_ratio;
	pr2 = 0.0;
      }

    dst_cb1 = flow1->dst_cb;
  }

  /* Flow 1 is the sole or higher-weight flow emanating from the seed */

  /* calc data assuming main path is included */
  if (main_path)
    {
      new_slots_avail =
	LB_hb_curr_slots_avail +
	(LB_hb_cb_info[dst_cb1->id].dep_height * LB_hb_issue_width);
      new_slots_used =
	LB_hb_curr_slots_used + LB_hb_cb_info[dst_cb1->id].slots_used;
    }
  else
    {
      new_slots_avail = LB_hb_curr_slots_avail;
      new_slots_used = LB_hb_curr_slots_used;

      if (!Set_in (LB_hb_nested_region_blocks, dst_cb1->id))
	{
	  double scaled_pr1 = 
	    LB_hb_scale_weight_ratio_for_hazards (pr1, dst_cb1);

	  if (flags & LB_BLOCK_SEL_AGGRESSIVE)
	    scaled_pr1 *= 10000.0;

	  if (scaled_pr1 < LB_hb_block_min_path_ratio)
	    {
#ifdef DEBUG_BLOCK_SEL
	      fprintf (ERR, "   REJECT (cb %d) min path ratio %f\n",
		       dst_cb1->id, scaled_pr1);
#endif
	      return 1;
	    }

	  if ((LB_hb_curr_slots_used + 
	       LB_hb_cb_info[dst_cb1->id].slots_used) >
	      (LB_hb_curr_slots_avail))
	    {
#ifdef DEBUG_BLOCK_SEL
	      fprintf (ERR, "   REJECT (cb %d) slots\n",
		       dst_cb1->id);
#endif
	      return 1;
	    }
	}
    }

  mpincl = LB_hb_select_blocks (dst_cb1, header, blocks, avail_blocks,
				main_path, pr1, flags, tr_header);

  /* JWS 20020911 - terminate growth if main path is not included */

  /* less important path, if it exists */
  if (mpincl && dst_cb2)
    {
      if (!Set_in (LB_hb_nested_region_blocks, dst_cb2->id))
	{
	  double scaled_pr2 = 
	    LB_hb_scale_weight_ratio_for_hazards (pr2, dst_cb2);

	  if (flags & LB_BLOCK_SEL_AGGRESSIVE)
	    scaled_pr2 *= 10000.0;

	  if (scaled_pr2 < LB_hb_block_min_path_ratio)
	    {
#ifdef DEBUG_BLOCK_SEL
	      fprintf (ERR, "   REJECT (cb %d) min path ratio %f\n",
		       dst_cb2->id, scaled_pr2);
#endif
	      return 1;
	    }

	  if ((LB_hb_cb_info[dst_cb2->id].slots_used + new_slots_used) >
	      new_slots_avail)
	    {
#ifdef DEBUG_BLOCK_SEL
	      fprintf (ERR, "   REJECT (cb %d) not enuf slots\n",
		       dst_cb2->id);
#endif
	      return 1;
	    }
	}

      LB_hb_select_blocks (dst_cb2, header, blocks, avail_blocks, 0,
			   pr2, flags, tr_header);
    }

  return 1;
}

void
LB_hb_sort_cb_by_weight (L_Func * fn)
{
  int i, j, n_cb;
  L_Cb *cb;

  i = 0;
  n_cb = fn->n_cb;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      LB_hb_cb_sort_buf[i].weight = cb->weight;
      LB_hb_cb_sort_buf[i].ptr = (void *) cb;
      i++;
    }

  /* bubble sort, yay */
  for (i = 0; i < n_cb; i++)
    {
      for (j = i + 1; j < n_cb; j++)
	{
	  if (LB_hb_cb_sort_buf[i].weight < LB_hb_cb_sort_buf[j].weight)
	    {
	      double temp_weight;
	      void *temp_ptr;
	      temp_weight = LB_hb_cb_sort_buf[i].weight;
	      temp_ptr = LB_hb_cb_sort_buf[i].ptr;
	      LB_hb_cb_sort_buf[i].weight = LB_hb_cb_sort_buf[j].weight;
	      LB_hb_cb_sort_buf[i].ptr = LB_hb_cb_sort_buf[j].ptr;
	      LB_hb_cb_sort_buf[j].weight = temp_weight;
	      LB_hb_cb_sort_buf[j].ptr = temp_ptr;
	    }
	}
    }

}

void
LB_hb_find_general_regions (L_Func * fn, int flags,
			    LB_TraceRegion_Header * header)
{
  int i;
  L_Cb *cb;
  LB_TraceRegion *new_region;
  Set blocks;

#ifdef DEBUG_BLOCK_SEL
  fprintf (ERR,
	   "##########################################################\n");
  fprintf (ERR, "## Identify general regions (fn %s)\n", fn->name);
  fprintf (ERR,
	   "##########################################################\n");
#endif


  LB_hb_find_cb_info (fn);

  /*
   *        Do these from highest weight to lowest
   */
  for (i = 0; i < fn->n_cb; i++)
    {
      cb = (L_Cb *) LB_hb_cb_sort_buf[i].ptr;
      if (cb->weight < LB_hb_min_cb_weight)
	continue;
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SUPERBLOCK))
	continue;
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_LOOP) &&
	  !L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	continue;
      if (LB_hb_is_single_block_loop (cb) &&
	  !L_find_attr (cb->attr, LB_MARKED_FOR_LOOP_PEEL))
	continue;
      if (LB_hb_ignore_block (cb))
	continue;
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_LOOP) ||
	  L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_HAMMOCK))
	{
	  if ((flags & LB_BLOCK_SEL_NO_NESTED_HAMMOCKS) ||
	      (flags & LB_BLOCK_SEL_NO_NESTED_LOOPS))
	    continue;
	  /* lets wait for the header of this region!! */
	  if (!LB_find_traceregion_by_cb (header, cb))
	    continue;
	}

#ifdef DEBUG_BLOCK_SEL
      fprintf (ERR, "Attempting to form general region with seed %d\n",
	       cb->id);
#endif

      if (LB_hb_nested_region_blocks)
	LB_hb_nested_region_blocks = Set_dispose (LB_hb_nested_region_blocks);

      blocks = NULL;
      LB_hb_curr_slots_used = 0;
      LB_hb_curr_slots_avail = 0;
      LB_hb_curr_dep_height = 0;
      LB_hb_select_blocks (cb, cb, &blocks, NULL, 1, 1.0, flags, header);

      if (!blocks || Set_size (blocks) == 1)
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR, "\tNo expansion could be done for seed %d\n", cb->id);
#endif
	  blocks = Set_dispose (blocks);
	  continue;
	}

      new_region =
	LB_create_traceregion (L_fn, header->next_id++, cb, blocks,
			       L_TRACEREGION_GENERAL);

      if (LB_traceregion_is_subsumed (new_region, header))
	{
#ifdef DEBUG_BLOCK_SEL
	  fprintf (ERR,
		   "\tExpansion of %d led to subset of existing region\n",
		   cb->id);
#endif
	  blocks = Set_dispose (blocks);
	  LB_free_traceregion (new_region);
	  continue;
	}

#ifdef DEBUG_BLOCK_SEL
      fprintf (ERR, "\tHyperblock formed (header %d)\n", cb->id);
      fprintf (ERR, "\t");
      Set_print (ERR, "blocks", blocks);
#endif

      /*
       *    Create a new region
       */

      header->traceregions =
	List_insert_last (header->traceregions, new_region);

      new_region->slots_used = LB_hb_curr_slots_used;
      new_region->slots_avail = LB_hb_curr_slots_avail;
      new_region->dep_height = LB_hb_curr_dep_height;
      blocks = Set_dispose (blocks);
    }

#ifdef PRINT_REGION_SUMMARY
  fprintf (ERR, "After removal...\n");
  LB_print_traceregions (ERR, header);
#endif

  if (LB_hb_nested_region_blocks)
    LB_hb_nested_region_blocks = Set_dispose (LB_hb_nested_region_blocks);

  Lcode_free (LB_hb_cb_sort_buf);
  Lcode_free (LB_hb_cb_info);
  LB_hb_cb_info = NULL;

  return;
}

