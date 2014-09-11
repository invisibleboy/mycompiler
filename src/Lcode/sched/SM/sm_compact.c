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
/*****************************************************************************\
 *      File:   sm.c
 *      Author: Sain-Zee Ueng, Wen-mei Hwu
 *      Creation Date:  February 2002
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "sm.h"
#include <library/l_alloc_new.h>

/* 20030709 SZU
 * Helper function for compaction w/ internal stop bits.
 * Given an issue group w/ fixed templates and appropriate range,
 * try to reschedule the operations in the issue group.
 * If fail, return.
 */
int
SM_reschedule_issue (SM_Issue_Group *issue_group_ptr, int earliest_slot,
		     int latest_slot)
{
  int index, num_slots, unscheduled = 0;
  SM_Cb *sm_cb;
  SM_Oper *sm_op = NULL, *unscheduled_op = NULL;
  int *port_array;
  SM_Oper *old_sm_op;

  sm_cb = issue_group_ptr->sm_cb;
  num_slots = SM_get_num_slots (sm_cb);

  for (index = num_slots - 1; index >= 0; index--)
    {
      if ((sm_op = issue_group_ptr->slots[index]))
	{
	  /* sm_op == NOP */
	  if (SM_is_nop (sm_op))
	    {
	      if (sm_op->flags & SM_OP_SCHEDULED)
		SM_unschedule_nop (sm_op);
	      SM_delete_nop (sm_op);
	    }
	  else
	    {
	      /* Only need to unschedule one operation.
	       * Will unschedule and reschedule the rest in
	       * SM_schedule_oper_priority(restrict).
	       * May change later when optimized for speed!
	       */
	      if (!unscheduled)
		{
		  if (sm_op->flags & SM_OP_SCHEDULED)
		    {
		      sm_op->old_issue_time = sm_op->sched_cycle;
		      SM_unschedule_oper (sm_op, NULL);
		    }
		  /* Still need to remove from issue_group_ptr->slots */
		  else
		    {
		      issue_group_ptr->slots[index] = NULL;
		      sm_op->issue_group = NULL;
		    }
		  unscheduled_op = sm_op;
		  unscheduled = 1;
		}
	    }
	}
    }

  /* 20030831 SZU
   * SM_schedule_oper_priority merged. Need to do port/restrict check here.
   */
  port_array = SM_init_port_array (sm_cb);

  /* Check if existing op + sm_op exceed resrc constraints */
  for (index = 0; index < num_slots; index++)
    {
      if ((old_sm_op = issue_group_ptr->slots[index]))
	/* 20031204 SZU
	 * Do not add if old_sm_op is a nop;
	 * shouldn't be any, but just in case.
	 */
	if (!SM_is_nop(old_sm_op))
	  /* Assumes that ports are all powers of 2. Should be right */
	  port_array[C_log2(old_sm_op->mdes_op->port_mask)]++;
    }

  port_array[C_log2(unscheduled_op->mdes_op->port_mask)]++;

  /* Check against variable restrict_array */
  for (index = 0; index < SM_get_num_restricts (sm_cb); index++)
    {
      int index1;
      int num_restrictions = 0;
      int restriction_limit = SM_get_restrict_num (sm_cb, index);
      int restriction_mask = SM_get_restrict_mask (sm_cb, index);

      for (index1 = 0; index1 < SM_get_num_ports (sm_cb); index1++)
	{
	  /* If the port applies to the restriction, add it */
	  if (C_pow2 (index1) & restriction_mask)
	    num_restrictions += port_array[index1];
	}

      /* If restriction's violated, return */
      if (num_restrictions > restriction_limit)
	{
	  free (port_array);
	  return 0;
	}
    }
  free (port_array);

  return
    SM_schedule_oper_priority (unscheduled_op, unscheduled_op->old_issue_time,
			       earliest_slot, latest_slot, 0, 1);
}

/* 20030707 SZU
 * Starting function for compacting using templates w/ internal stop bits.
 * Hard coded for Itanium
 */
void
SM_compact_w_internal_sbits (SM_Cb *sm_cb)
{
  SM_Issue_Group *issue_group_ptr, *nxt_issue_group_ptr;
  SM_Issue_Group *save_issue_group_ptr = NULL;
  SM_Issue_Group *save_nxt_issue_group_ptr = NULL;
  SM_Oper *sm_op;
  int compact, max_template_per_issue, last_bundle, slots_per_template;
  int index, num_slots, success, save_current, save_nxt;
  int earliest_slot, latest_slot, first_part_succeed, j;

  num_slots = SM_get_num_slots (sm_cb);
  max_template_per_issue = SM_get_template_per_issue (sm_cb);
  slots_per_template = SM_get_slots_per_template (sm_cb);

  for (issue_group_ptr = sm_cb->first_issue_group; issue_group_ptr;
       issue_group_ptr = issue_group_ptr->next_issue_group)
    {
      save_current = 0;
      save_nxt = 0;

      /* continue if issue_group full */
      if (issue_group_ptr->full)
	{
	  SM_assign_stop_bit (issue_group_ptr);
	  continue;
	}

      /* Get the last non-empty bundle of the current issue */
      for (index = num_slots - 1; index >= 0; index--)
	{
	  sm_op = issue_group_ptr->slots[index];

	  if (sm_op)
	    {
	      if (!(SM_is_nop (sm_op)))
		break;
	    }
	}

      if (index >= 0)
	last_bundle = index / slots_per_template;
      else
	last_bundle = -1;

      /* Empty issue. Warn and continue. */
      if (last_bundle < 0)
	{
#if 0
	  printf ("SM_compact_w_internal_sbits: Empty issue found. Check!!\n"
		  "function %s cb %i time %i\n", sm_cb->lcode_fn->name,
		  sm_cb->lcode_cb->id, issue_group_ptr->issue_time);
#endif
	  continue;
	}

      /* 20030731 SZU
       * Check number of slots left and the last bundle.
       * Due to new insertion time saver, last bundle could be erroneous.
       * If there are three or more slots left and the last bundle is not 0,
       * reschedule first to be sure as compact as can be to start w/.
       * 20030819 SZU
       * Add additional check. Make sure not internal stop bit already.
       */

      if ((last_bundle > 0) && (issue_group_ptr->num_slots_left >= 3) &&
	  (!issue_group_ptr->bundles[0]->internal_stop_bit))
	{
	  SM_reschedule_issue (issue_group_ptr, 0, 5);

	  /* Recalculate last_bundle */
	  for (index = num_slots - 1; index >= 0; index--)
	    {
	      sm_op = issue_group_ptr->slots[index];

	      if (sm_op)
		{
		  if (!(SM_is_nop (sm_op)))
		    break;
		}
	    }

	  if (index >= 0)
	    last_bundle = index / slots_per_template;
	  else
	    L_punt ("SM_compact_w_internal_sbits: "
		    "Reschedule end up w/ empty issue. Bad!!\n"
		    "function %s cb %i time %i\n", sm_cb->lcode_fn->name,
		    sm_cb->lcode_cb->id, issue_group_ptr->issue_time);
	}

      nxt_issue_group_ptr = issue_group_ptr->next_issue_group;

      /* Need to find the first non-empty issue group. */
      while ((nxt_issue_group_ptr) &&
	     (nxt_issue_group_ptr->num_slots_left == 6))
	nxt_issue_group_ptr = nxt_issue_group_ptr->next_issue_group;

      /* stop if no next issue */
      if (!nxt_issue_group_ptr)
	{
	  SM_assign_stop_bit (issue_group_ptr);
	  break;
	}

      /* continue if next issue full */
      if (nxt_issue_group_ptr->full)
	{
	  SM_assign_stop_bit (issue_group_ptr);
	  continue;
	}

      /* 20030725 SZU
       * Softpipe scheduling can unschedule oper w/o rescheduling issue group.
       * This can cause errors in compaction; erroneous last bundle.
       * Therefore reschedule if NULL found
       */
      for (j = index - 1; j >= 0; j--)
	{
	  SM_Issue_Group *prev_issue;

	  if ((sm_op = issue_group_ptr->slots[j]))
	    continue;

	  prev_issue = issue_group_ptr->prev_issue_group;

	  /* Need to make sure not empty because of L syll */
	  if ((j > 0) && 
	      (issue_group_ptr->slots[j - 1]) &&
	      (issue_group_ptr->slots[j - 1]->mdes_op->port_mask == 0x40))
	    continue;

	  /* Need to find previous non-empty issue */
	  while ((prev_issue) && (prev_issue->num_slots_left == 6))
	    prev_issue = prev_issue->prev_issue_group;

	  /* Need to make sure not empty because of internal stop bit */
	  if (prev_issue)
	    if ((prev_issue->bundles[1]->internal_stop_bit == 1) &&
		(j <= prev_issue->bundles[1]->stop))
	      earliest_slot = prev_issue->bundles[1]->stop + 1;
	    else if ((prev_issue->bundles[0]->internal_stop_bit == 1) &&
		     (j <= prev_issue->bundles[0]->stop))
	      earliest_slot = prev_issue->bundles[0]->stop + 1;
	    else
	      earliest_slot = 0;
	  else
	    earliest_slot = 0;

	  if (j >= earliest_slot)
	    {
	      SM_reschedule_issue (issue_group_ptr, earliest_slot, 5);

	      /* Recalculate last_bundle */
	      for (index = num_slots - 1; index >= 0; index--)
		{
		  if ((sm_op = issue_group_ptr->slots[index]) &&
		      !(SM_is_nop (sm_op)))
		    break;
		}

	      if (index >= 0)
		last_bundle = index / slots_per_template;
	      else
		L_punt ("SM_compact_w_internal_sbits: "
			"Reschedule end up w/ empty issue. Bad!!\n"
			"function %s cb %i time %i\n",
			sm_cb->lcode_fn->name,
			sm_cb->lcode_cb->id, issue_group_ptr->issue_time);

	      break;
	    }
	}

      if ((issue_group_ptr->num_slots_left == 0) ||
	  ((issue_group_ptr->num_slots_left == 1) &&
	   (nxt_issue_group_ptr->num_slots_left == 1)) ||
	  ((issue_group_ptr->num_slots_left == 3) && (last_bundle == 0)))
	{
	  SM_assign_stop_bit (issue_group_ptr);
	  continue;
	}

      /* 2003717 SZU
       * If the last bundle is the first bundle,
       * fail if contains F, B, or L syll.
       * 20031204 SZU
       * Only check if not NOP.
       */
      compact = 1;
      if (last_bundle == 0)
	for (index = 0; (index < 3) && (compact); index++)
	  {
	    sm_op = issue_group_ptr->slots[index];

	    /* 20030819 SZU
	     * The hard code numbers don't look right.
	     */
#if 0
	    if (sm_op)
	      if ((sm_op->mdes_op->syll_mask == 0x40) ||
		  (sm_op->mdes_op->syll_mask == 0x100) ||
		  (sm_op->mdes_op->syll_mask == 0x10))
#else
	    if ((sm_op) && (!SM_is_nop(sm_op)))
	      if ((sm_op->mdes_op->syll_mask == 0x4) ||
		  (sm_op->mdes_op->syll_mask == 0x8) ||
		  (sm_op->mdes_op->syll_mask == 0x10))
#endif
		compact = 0;
	  }

      if (!compact)
	{
	  SM_assign_stop_bit (issue_group_ptr);
	  continue;
	}

      success = 0;
      earliest_slot = -1;
      latest_slot = -1;
      first_part_succeed = 0;

      /* If scheduled_op == 1 and is in first slot, use m;mi */
      /* 20030717 SZU
       * Change condition
       */
#if 0
      if ((scheduled_op == 1) &&
	  (!(SM_is_nop (issue_group_ptr->slots[last_bundle * 3]))))
#else
      if (((last_bundle == 0) &&
	   (!issue_group_ptr->bundles[0]->template_lock) && 
	   (issue_group_ptr->num_slots_left == 5)) ||
	  ((last_bundle != 0) && (issue_group_ptr->num_slots_left >= 2)))
#endif
	{
	  /* Save current issue group scheduling */
	  save_issue_group_ptr =
	    SM_create_issue_group (sm_cb, issue_group_ptr->issue_time);
	  save_current = 1;
	  SM_copy_issue (save_issue_group_ptr, issue_group_ptr);

	  /* 20030721 SZU
	   * Need to determine earliest and latest slots first.
	   */
	  /* All instructions in first bundle */
	  if (last_bundle == 0)
	    {
	      earliest_slot = 0;
	      latest_slot = 0;
	    }
	  /* Bundle 0 does not have internal stop bit */
	  else if (!issue_group_ptr->bundles[0]->template_lock)
	    {
	      earliest_slot = 0;
	      latest_slot = 3;
	    }
	  else
	    {
	      earliest_slot = issue_group_ptr->bundles[0]->stop + 1;
	      latest_slot = 3;
	    }

	  /* Fix the last non-empty template to m;mi
	   * Use last_bundle because could be first bundle.
	   */
	  issue_group_ptr->bundles[last_bundle]->template_lock = 1;
	  issue_group_ptr->bundles[last_bundle]->template_index = 1;
	  issue_group_ptr->bundles[last_bundle]->template_mask =
	    SM_get_template (sm_cb, 1);
	  issue_group_ptr->bundles[last_bundle]->stop = 0;
	  issue_group_ptr->bundles[last_bundle]->internal_stop_bit = 1;

	  /* Schedule w/ fixed template m;mi for last non-empty bundle */
	  /* 20030721 SZU
	   * Restrict slots more based on previous compactions.
	   * Determine early and late slots earlier.
	   */
#if 0
	  if (last_bundle == 0)
	    {
	      success = SM_reschedule_issue (issue_group_ptr, 0, 0);
	    }
#if 0
	  else 
	    success = SM_reschedule_issue (issue_group_ptr, 0, 3);
#else
	  /* Bundle 0 does not have internal stop bit */
	  else if (!issue_group_ptr->bundles[0]->template_lock)
	    success = SM_reschedule_issue (issue_group_ptr, 0, 3);
	  /* Bundle 0 does have internal stop bit */
	  else
	    success =
	      SM_reschedule_issue (issue_group_ptr,
				   issue_group_ptr->bundles[0]->stop + 1, 3);
#endif
#else
#endif
	  first_part_succeed =
	    SM_reschedule_issue (issue_group_ptr, earliest_slot, latest_slot);

	  /* If success, save/schedule nxt issue w/ fixed 1st template m;mi */
	  if (first_part_succeed)
	    {
	      /* 20030715 SZU
	       * Set the number of slots left accordingly
	       * Take away two slots
	       */
	      issue_group_ptr->num_slots_left -= 2;
	      
	      /* Save nxt issue group */
	      save_nxt_issue_group_ptr =
		SM_create_issue_group (sm_cb, nxt_issue_group_ptr->issue_time);
	      save_nxt = 1;

	      SM_copy_issue (save_nxt_issue_group_ptr, nxt_issue_group_ptr);

	      /* Fix the first to m;mi.
	       */
	      nxt_issue_group_ptr->bundles[0]->template_lock = 1;
	      nxt_issue_group_ptr->bundles[0]->template_index = 1;
	      nxt_issue_group_ptr->bundles[0]->template_mask =
		SM_get_template (sm_cb, 1);
	      nxt_issue_group_ptr->bundles[0]->stop = 0;
	      nxt_issue_group_ptr->bundles[0]->internal_stop_bit = 2;
	      
	      /* Schedule w/ fixed template m;mi for first bundle */
	      success = SM_reschedule_issue (nxt_issue_group_ptr, 1, 5);

	      if (success)
		{
		  /* 20030715 SZU
		   * Set the number of slots left accordingly
		   */
		  nxt_issue_group_ptr->num_slots_left -= 1;
		}
	      else
		{
		  issue_group_ptr->num_slots_left += 2;

		  /* Also get rid of template lock */
		  issue_group_ptr->bundles[last_bundle]->template_lock = 0;
		}
	    }
	  else
	    {
	      issue_group_ptr->bundles[last_bundle]->template_lock = 0;
	    }
	}

      /* If scheduling w/ m;mi failed,
       * or if scheduled_op > 1 or is not in first slot and nxt_issue_num_ops
       * is < 5, use mi;i
       */
      /* 20030717 SZU
       * Change condition
       */
#if 0
      if ((!success) && (nxt_issue_num_ops < 5))
#else
      if ((((last_bundle != 0)) ||
	   ((last_bundle == 0) &&
	    (!issue_group_ptr->bundles[0]->template_lock) &&
	    (issue_group_ptr->num_slots_left >= 4))) &&
	  (nxt_issue_group_ptr->num_slots_left >= 2) &&
	  (!success))
#endif
	{
	  /* Save current issue group scheduling */
	  if (!save_current)
	    {
	      save_issue_group_ptr =
		SM_create_issue_group (sm_cb, issue_group_ptr->issue_time);
	      save_current = 1;
	      SM_copy_issue (save_issue_group_ptr, issue_group_ptr);
	    }
	  /* Previous try, if possible, failed.
	   * Restore before trying this one.
	   * 20030722 SZU
	   * Only do this if first part didn't succeed.
	   */
	  else if (!first_part_succeed)
	    {
	      /* 20030719 SZU
	       * Do full restoration; safer, simpler
	       * Instead of full restoration, just copy back non-nop
	       */
#if 0
	      SM_restore_issue_group (issue_group_ptr, save_issue_group_ptr);
#else
	      for (index = 0; index < num_slots; index++)
		{
		  SM_Oper *sm_op = save_issue_group_ptr->slots[index];

		  if ((sm_op) && (!(SM_is_nop (sm_op))))
		    issue_group_ptr->slots[index] = sm_op;
		}
#endif
	    }

	  /* 20030721 SZU
	   * Need to determine earliest and latest slots first.
	   */
	  /* All instructions in first bundle */
	  if (last_bundle == 0)
	    {
	      earliest_slot = 0;
	      latest_slot = 1;
	    }
	  /* Bundle 0 does not have internal stop bit */
	  else if (!issue_group_ptr->bundles[0]->template_lock)
	    {
	      earliest_slot = 0;
	      latest_slot = 4;
	    }
	  else
	    {
	      earliest_slot = issue_group_ptr->bundles[0]->stop + 1;
	      latest_slot = 4;
	    }

	  /* Fix the last non-empty template to mi;i
	   * Use last_bundle because could be first bundle.
	   */
	  issue_group_ptr->bundles[last_bundle]->template_lock = 1;
	  issue_group_ptr->bundles[last_bundle]->template_index = 4;
	  issue_group_ptr->bundles[last_bundle]->template_mask =
	    SM_get_template (sm_cb, 4);
	  issue_group_ptr->bundles[last_bundle]->stop = 1;
	  issue_group_ptr->bundles[last_bundle]->internal_stop_bit = 1;
	      
	  /* Schedule w/ fixed template mi;i for last non-empty bundle */
	  success = SM_reschedule_issue (issue_group_ptr, earliest_slot, 
					 latest_slot);

	  /* If success, save/schedule nxt issue w/ fixed 1st template mi;i */
	  if (success)
	    {
	      /* 20030715 SZU
	       * Set the number of slots left accordingly
	       */
	      issue_group_ptr->num_slots_left -= 1;
	      
	      /* Save nxt issue group */
	      if (!save_nxt)
		{
		  save_nxt_issue_group_ptr =
		    SM_create_issue_group (sm_cb,
					   nxt_issue_group_ptr->issue_time);
		  save_nxt = 1;
		  SM_copy_issue (save_nxt_issue_group_ptr, nxt_issue_group_ptr);
		}
	      /* Previous try, if possible, failed.
	       * Restore before trying this one.
	       */
	      else
		{
		  /* 20030719 SZU
		   * Do full restoration; safer, simpler
		   * Instead of full restoration, just copy back non-nop
		   */
#if 0
		  for (index = 0; index < num_slots; index++)
		    {
		      issue_group_ptr->slots[index] =
			save_issue_group_ptr->slots[index];
		    }
#else
#if 0
	      SM_restore_issue_group (issue_group_ptr, save_issue_group_ptr);
#else
		  for (index = 0; index < num_slots; index++)
		    {
		      SM_Oper *sm_op = save_nxt_issue_group_ptr->slots[index];

		      if ((sm_op) && (!(SM_is_nop (sm_op))))
			nxt_issue_group_ptr->slots[index] = sm_op;
		    }
#endif

#endif
		}
		  

	      /* Fix the first to m;mi. */
	      nxt_issue_group_ptr->bundles[0]->template_lock = 1;
	      nxt_issue_group_ptr->bundles[0]->template_index = 4;
	      nxt_issue_group_ptr->bundles[0]->template_mask =
		SM_get_template (sm_cb, 4);
	      nxt_issue_group_ptr->bundles[0]->stop = 1;
	      nxt_issue_group_ptr->bundles[0]->internal_stop_bit = 2;
	      
	      /* Schedule w/ fixed template m;mi for first bundle */
	      success = SM_reschedule_issue (nxt_issue_group_ptr, 2, 5);

	      if (success)
		/* 20030715 SZU
		 * Set the number of slots left accordingly
		 */
		nxt_issue_group_ptr->num_slots_left -= 2;
	    }
	}

      /* Not successful. Restore from saved issue groups as necessary.
       * Also assign the stop bit to current issue
       */
      if (!success)
	{
	  if (save_current)
	    {
	      SM_restore_issue_group (issue_group_ptr, save_issue_group_ptr);

	      if (issue_group_ptr->bundles[0]->internal_stop_bit)
		issue_group_ptr->num_slots_left -= 
		  issue_group_ptr->bundles[0]->stop + 1;

	      if (issue_group_ptr->bundles[1]->internal_stop_bit)
		issue_group_ptr->num_slots_left -=
		  2 - issue_group_ptr->bundles[1]->stop;
	    }

	  if (save_nxt)
	    {
	      SM_restore_issue_group (nxt_issue_group_ptr,
				      save_nxt_issue_group_ptr);
	    }
	}

      /* Go through current issue and assign stop bits */
      SM_assign_stop_bit (issue_group_ptr);
    }
}
