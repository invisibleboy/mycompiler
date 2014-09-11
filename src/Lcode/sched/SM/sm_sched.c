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
 *      File:   sm_sched.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Modulo Scheduling support: IMPACT Technologies (John Gyllenhaal)
 *      Creation Date: July  1996
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "sm.h"
#include <library/l_alloc_new.h>
#include <Lcode/l_main.h>

#define DEBUG_NOP 0
#define DEBUG_PRIORITY 0

#define SM_BRANCHES_AT_END
/* Define this flag to reduce usage of mbb/bbb bundles and to keep
 * branches at the end (reduce nop.b usage).  Strangely enough, this
 * decreases performance.  Turning off for now until we can figure out
 * why -- JWS
 */
extern L_Alloc_Pool *SM_Issue_Group_pool;
extern L_Alloc_Pool *SM_Oper_pool;
extern L_Alloc_Pool *SM_Compatible_Alt_pool;

extern int SM_schedule_oper_schedule (SM_Oper *sm_op,
				      SM_Issue_Group *issue_group_ptr,
				      int issue_time, int slot,
				      unsigned int sched_flags);

/* 20021212 SZU
 * Initialize array to calculate if too many PORTs used
 * 20030602 SZU
 * Altered to accomodate mdes format.
 */
int *
SM_init_port_array (SM_Cb *sm_cb)
{
  int num_ports, size, id;
  int *new_array;

  num_ports = SM_get_num_ports (sm_cb);

  /* Malloc array of size num_ports */
  size = num_ports * sizeof (int);
  if ((new_array = (int *) malloc (size)) == NULL)
    L_punt ("SM_init_restriction_array : Out of memory");

  for (id = 0; id < num_ports; id++)
    new_array[id] = 0;

  return new_array;
}

/* 20021211 SZU
 * Given the scheduled slots, calculate the template bit vector for the bundles
 * DO NOT modify the template type; should be valid
 * 20030603 SZU
 * Alter according to mdes format.
 */
void
SM_calculate_template_vector (SM_Issue_Group *issue_group)
{
  int index, template_shift_var, num_slots, slots_per_template;
  int current_template;
  unsigned int current_syll;
  unsigned int template = 0x0;

  /* Grab variables from mdes */
  template_shift_var = SM_get_template_shift_var (issue_group->sm_cb);
  slots_per_template = SM_get_slots_per_template (issue_group->sm_cb);
  num_slots = SM_get_num_slots (issue_group->sm_cb);

  for (index = 0; index < num_slots; index++)
    {
      if (issue_group->slots[index])
	{
	  current_syll = issue_group->slots[index]->syll_type;
	  template |= current_syll <<
	    ((slots_per_template - (index % slots_per_template) - 1) *
	     template_shift_var);
	}

      /* Reach end of template */
      if (index % slots_per_template == slots_per_template - 1)
	{
	  current_template = index / slots_per_template;
	  /* 20030709 SZU
	   * Change template_mask only if not locked
	   */
	  if (!issue_group->bundles[current_template]->template_lock)
	    issue_group->bundles[current_template]->template_mask = template;
	  template = 0x0;
	}
    }
}

/* 20021211 SZU
 * Given a template bit vector, see if a valid bundle template exists
 * If valid bundle template exists, return index number.
 * Else return SM_NO_TEMPLATE (-1)
 * 20030603 SZU
 * Alter to accomodate mdes format.
 */
int
SM_check_template_validity (SM_Cb *sm_cb, unsigned int temp_template)
{
  int index, num_templates;
  SM_Template *template_array;

  /* Get num_templates from mdes, and pointer to template_array */
  num_templates = sm_cb->sm_mdes->num_templates;
  template_array = sm_cb->sm_mdes->template_array;

  for (index = 0; index < num_templates; index++)
    {
      if ((SM_get_template (sm_cb, index) & temp_template) == temp_template)
	return index;
    }

  return SM_NO_TEMPLATE;
}

/* 20021211 SZU
 * Create a SM_Issue_Group
 * 20030603 SZU
 * Alter according to format changes from mdes.
 */
SM_Issue_Group *
SM_create_issue_group (SM_Cb *sm_cb, int issue_time)
{
  SM_Issue_Group *issue_group_ptr;
  SM_Bundle *bundle;
  int num_slots, max_template_per_issue, templates_size, slots_size, index;
  int ref_time;

  issue_group_ptr = (SM_Issue_Group *) L_alloc (SM_Issue_Group_pool);

  /* Get information from mdes */
  num_slots = SM_get_num_slots (sm_cb);
  max_template_per_issue = sm_cb->sm_mdes->max_template_per_issue;

  /* Allocate memory for slots */
  slots_size = num_slots * (sizeof (SM_Oper *));
  if ((issue_group_ptr->slots = (SM_Oper **) malloc (slots_size)) == NULL)
    L_punt ("SM_create_issue_group: Out of memory");

  for (index = 0; index < num_slots; index++)
    issue_group_ptr->slots[index] = NULL;

  /* Allocate memory for templates */
  templates_size = max_template_per_issue * (sizeof (SM_Bundle *));
  if ((issue_group_ptr->bundles = (SM_Bundle **) malloc (templates_size)) ==
      NULL)
    L_punt ("SM_create_issue_group: Out of memory");

  for (index = 0; index < max_template_per_issue; index++)
    {
      if ((bundle = (SM_Bundle *) malloc (sizeof (SM_Bundle))) == NULL)
	L_punt ("SM_create_issue_group: Out of memory");
      
      bundle->template_mask = 0;
      bundle->template_index = -1;
      bundle->stop = -1;
      bundle->empty = 1;
      issue_group_ptr->bundles[index] = bundle;

      /* 20030709 SZU
       * New fields were added. Need to initialize properly.
       */
      bundle->template_lock = 0;
      bundle->internal_stop_bit = 0;
    }

  issue_group_ptr->sm_cb = sm_cb;
  issue_group_ptr->full = 0;
  issue_group_ptr->num_slots_left = num_slots;
  issue_group_ptr->prev_issue_group = NULL;
  issue_group_ptr->next_issue_group = NULL;

  /* 20021218 SZU
   * Need to adjust for modulo scheduling.
   * Should always be within II time.
   */
  if (sm_cb->flags & SM_MODULO_RESOURCES)
    {
      ref_time = issue_time % sm_cb->II;
      while (ref_time < 0)
	ref_time += sm_cb->II;

      issue_group_ptr->issue_time = ref_time;
    }
  else
    {
      issue_group_ptr->issue_time = issue_time;
    }

  return issue_group_ptr;
}

/* 20021211 SZU
 * Link the issue group appropriately
 */
void
SM_link_issue_group (SM_Issue_Group *issue_grp_ptr)
{
  SM_Issue_Group *after_this_grp;
  SM_Cb *sm_cb;

  sm_cb = issue_grp_ptr->sm_cb;

  /* Search from the end of already scheduled issue groups for the
   * group where this group should be placed after.
   * (Optimized for cb top-down)
   */
  for (after_this_grp = sm_cb->last_issue_group; after_this_grp != NULL;
       after_this_grp = after_this_grp->prev_issue_group)
    {
      if (after_this_grp->issue_time < issue_grp_ptr->issue_time)
	break;
    }

  /* Place after "after_this_grp" if not NULL */
  if (after_this_grp != NULL)
    {
      issue_grp_ptr->prev_issue_group = after_this_grp;
      issue_grp_ptr->next_issue_group = after_this_grp->next_issue_group;
      if (after_this_grp->next_issue_group != NULL)
	after_this_grp->next_issue_group->prev_issue_group = issue_grp_ptr;
      else
	sm_cb->last_issue_group = issue_grp_ptr;
      after_this_grp->next_issue_group = issue_grp_ptr;
    }
  /* Otherwise, place at begining */
  else
    {
      issue_grp_ptr->prev_issue_group = NULL;
      issue_grp_ptr->next_issue_group = sm_cb->first_issue_group;
      if (sm_cb->first_issue_group != NULL)
	sm_cb->first_issue_group->prev_issue_group = issue_grp_ptr;
      else
	sm_cb->last_issue_group = issue_grp_ptr;
      sm_cb->first_issue_group = issue_grp_ptr;
    }
}

/* 20021211 SZU
 * Check if there is already an issue group at issue_time, 
 * or some multiple of II if doing modulo scheduling
 */
SM_Issue_Group *
SM_check_for_issue_group (SM_Cb *sm_cb, int issue_time)
{
  SM_Issue_Group *issue_group_ptr;

  for (issue_group_ptr = sm_cb->first_issue_group; issue_group_ptr != NULL;
       issue_group_ptr = issue_group_ptr->next_issue_group)
    {
      if (issue_group_ptr->issue_time == issue_time)
	return (issue_group_ptr);

      if (sm_cb->flags & SM_MODULO_RESOURCES)
	{
	  if ((issue_group_ptr->issue_time - issue_time) % sm_cb->II == 0)
	    return (issue_group_ptr);
	}
    }

  /* No issue group at issue_time */
  return NULL;
}

/* 20021212 SZU
 * Check if the issue group is full
 * 20030603 SZU
 * Alter to accomodate info from mdes.
 */
void
SM_issue_group_full_check (SM_Issue_Group *issue_group_ptr)
{
  int index, full, num_slots;
  SM_Cb *sm_cb;

  sm_cb = issue_group_ptr->sm_cb;
  num_slots = SM_get_num_slots (sm_cb);
  full = 1;

  for (index = 0; (index < num_slots) && full;)
    {
      /* If NULL, then not full */
      if (!(issue_group_ptr->slots[index]))
	full = 0;
      /* If NOP, then not full */
      else if (SM_is_nop (issue_group_ptr->slots[index]))
	full = 0;
      /* Advance index by num_slots of operation; L_SYLL */
      else
	index += issue_group_ptr->slots[index]->mdes_op->num_slots;
    }

  issue_group_ptr->full = full;
}

/* 20030605 SZU
 * Check up to, including bundle_number, of issue group.
 * Make sure there exists an issue from mdes that will work.
 */
int
SM_verify_issue (SM_Issue_Group *issue_group_ptr, int bundle_number)
{
  int success, index1, index2;
  SM_Cb *sm_cb;
  SM_Issue *issue;

  sm_cb = issue_group_ptr->sm_cb;

  success = 0;
  /* Go through each issue group. */
  for (index1 = 0; (index1 < sm_cb->sm_mdes->num_issues) && !success;
       index1++)
    {
      /* Get the issue */
      issue = &sm_cb->sm_mdes->issue_array[index1];	      

      /* See if issue allows bundle_number+1 templates. */
      if (issue->num_templates < bundle_number + 1)
	continue;

      success = 1;
      /* Check if all previous bundles match allowed
       * templates of issue.
       */
      for (index2 = 0; (index2 <= bundle_number) && success;
	   index2++)
	{
	  /* If not match, fail */
	  if (!(C_pow2(issue_group_ptr->bundles[index2]->template_index) &
		issue->templates[index2]))
	    success = 0;
	}
    }

  return success;
}

/* 20021211 SZU
 * Create a SM_Oper that is a NOP
 * 20030603 SZU
 * Alter to accomodate info from mdes. 
 */
SM_Oper *
SM_create_nop (SM_Cb *sm_cb, int proc_opc)
{
  SM_Oper *sm_op;
  Mdes *version1_mdes;

  SM_Compatible_Alt *first_compatible_alt, *last_compatible_alt;
  SM_Compatible_Alt *compatible_alt;
  Mdes_Alt *alt;
  Mdes_Operation *op;

  int i, array_size;
  int good_opcode = 0;

  sm_op = (SM_Oper *) L_alloc (SM_Oper_pool);

  /* Get the version1 mdes structure this sm_cb is using */
  version1_mdes = sm_cb->version1_mdes;

  /* Initialize the mdes flags */
  sm_op->mdes_flags = version1_mdes->op_table[proc_opc]->op_flags;

  /* Initialize the SM flags */
  sm_op->flags = 0;
  sm_op->ignore = 0;

  /* Initially list is empty */
  first_compatible_alt = NULL;
  last_compatible_alt = NULL;

  /* Get the operation the opcode corresponds to */
  op = version1_mdes->op_table[proc_opc];

  for (i = 0; i < op->num_alts; i++)
    {
      alt = &op->alt[i];

      /* Alloc a SM alt */
      compatible_alt = (SM_Compatible_Alt *) L_alloc (SM_Compatible_Alt_pool);

      /* Get the non-silent version from this mdes_alt */
      compatible_alt->normal_version = alt;

      /* Add to end of compatible alt linked list */
      if (last_compatible_alt != NULL)
        last_compatible_alt->next_compatible_alt = compatible_alt;
      else
        first_compatible_alt = compatible_alt;
      last_compatible_alt = compatible_alt;

      compatible_alt->next_compatible_alt = NULL;
    }

  /* Make sure there is at least one compatible alt */
  if (first_compatible_alt == NULL)
    L_punt ("SM_create_nop: No alts for opcode: %i nop!", proc_opc);

  sm_op->first_compatible_alt = first_compatible_alt;

  /* Set all operands to NULL */
  sm_op->operand = NULL;
  sm_op->dest = NULL;
  sm_op->src = NULL;
  sm_op->pred = NULL;
  sm_op->ext_dest = NULL;
  sm_op->ext_src = NULL;
  sm_op->implicit_dests = NULL;
  sm_op->implicit_srcs = NULL;
  
  sm_op->first_op_action = NULL;
  sm_op->last_op_action = NULL;

  sm_op->sm_cb = sm_cb;
  sm_op->first_queue = NULL;
  sm_op->priority = 0.0;
  sm_op->early_time = SM_MIN_CYCLE;
  sm_op->late_time = NULL;

  sm_op->mdes_op = version1_mdes->op_table[proc_opc];

  sm_op->cycle_lower_bound = SM_MIN_CYCLE;
  sm_op->cycle_upper_bound = SM_MAX_CYCLE;
  sm_op->sched_cycle = sm_op->cycle_upper_bound;
  sm_op->slot_lower_bound = 0;
  sm_op->slot_upper_bound = SM_MAX_SLOT;
  sm_op->sched_slot = sm_op->slot_upper_bound;

  sm_op->dep_lower_bound = NULL;
  sm_op->dep_upper_bound = NULL;
  
  sm_op->alt_chosen = NULL;

  /* Allocate option choice array able to handle the choices for any table */
  array_size = sizeof (unsigned short) * sm_cb->sm_mdes->max_num_choices;
  if ((sm_op->options_chosen = (unsigned short *) malloc (array_size)) ==
      NULL)
    L_punt ("SM_create_nop: Out of memory");

  /* Itanium values */
  sm_op->issue_group = NULL;

  /* NOP shouldn't be A type, should have set syllable */
  sm_op->syll_type = sm_op->mdes_op->syll_mask;

  /* Make sure proc_opc is a nop */
  for (i = 0; (i <= sm_cb->sm_mdes->max_nop_index) && !good_opcode; i++)
    {
      if (sm_cb->sm_mdes->nop_array[i] == proc_opc)
	good_opcode = 1;
    }

  if (!good_opcode)
    L_punt ("SM_create_nop: op code given not NOP!!\n");

  sm_op->old_issue_time = -1;
  sm_op->qentry = NULL;

  /* 20030909
   * Do not attempt to reduce liverange of NOP.
   * Shouldn't happen anyways.
   */
  sm_op->liverange_reduced = 1;

  return sm_op;
}

/* 20021211 SZU
 * Delete a NOP sm_op created by SM_create_nop
 * 20030603 SZU
 * Alter to accomodate info from mdes. 
 */
void
SM_delete_nop (SM_Oper *sm_op)
{
  SM_Compatible_Alt *compatible_alt, *next_compatible_alt;

  /* Check to make sure it is a NOP */
  if (!(SM_is_nop (sm_op)))
    L_punt ("SM_delete_nop: sm_op is not a NOP!");

  for (compatible_alt = sm_op->first_compatible_alt; compatible_alt != NULL;
       compatible_alt = next_compatible_alt)
    {
      next_compatible_alt = compatible_alt->next_compatible_alt;
      L_free (SM_Compatible_Alt_pool, compatible_alt);
    }

  if (sm_op->operand)
    free(sm_op->operand);

  /* Free implicit queues (if they exist) */
  if (sm_op->implicit_dests != NULL)
    SM_delete_action_queue (sm_op->implicit_dests);
  if (sm_op->implicit_srcs != NULL)
    SM_delete_action_queue (sm_op->implicit_srcs);

  /* Free the late time array (if exists) */
  if (sm_op->late_time)
    free (sm_op->late_time);

  /* Free the schedule choices array */
  free (sm_op->options_chosen);
  
  L_free (SM_Oper_pool, sm_op);
}

/* 20021211 SZU
 * Trimmed down SM_unschedule_oper for nop because they're incomplete SM_Oper
 * 20030603 SZU
 * Alter to accomodate info from mdes. 
 */
void
SM_unschedule_nop (SM_Oper * sm_op)
{
  SM_Cb *sm_cb;
  unsigned int op_flags;
  SM_Issue_Group *issue_group_ptr;

  /* Make sure sm_op is a nop */
  if (!(SM_is_nop (sm_op)))
    L_punt ("SM_unschedule_nop: sm_op is not a NOP!");

  /* The the sm_cb the op is in and sm_op's flags for ease of use */
  sm_cb = sm_op->sm_cb;
  op_flags = sm_op->flags;

  /* Sanity check, make sure operation is already scheduled! */
  if (!(sm_op->flags & SM_OP_SCHEDULED))
    {
      L_punt ("SM_unschedule_nop: %s op %i has not been scheduled!",
              sm_cb->lcode_fn->name, sm_op->lcode_op->id);
    }

  /* 20021211 SZU
   * Update Itanium info
   */
  issue_group_ptr = sm_op->issue_group;
  issue_group_ptr->slots[sm_op->sched_slot] = NULL;
  sm_op->issue_group = NULL;

  /* Unschedule the operation using the alt previous chosen.
   * For now, the normal version and the silent version of the
   * alternatives have exactly the same requirements, so 
   * just use the normal version's table.
   *
   * Use different routines for acyclic and cyclic (modulo)
   * scheduling. -ITI/JCG 8/99
   */
  if ((sm_cb->flags & SM_MODULO_RESOURCES) == 0)
    {
      SM_unsched_table (sm_cb, sm_op->alt_chosen->normal_version->table,
                        sm_op->options_chosen, sm_op->sched_cycle);
    }
  else
    {
      SM_modulo_unsched_table (sm_cb,
                               sm_op->alt_chosen->normal_version->table,
                               sm_op->options_chosen, sm_op->sched_cycle);
    }

  /* Flag as not scheduled */
  sm_op->flags &= ~SM_OP_SCHEDULED;

  /* Reinitialize fields to their unscheduled values */
  sm_op->sched_cycle = SM_MIN_CYCLE;
  sm_op->sched_slot = SM_MAX_SLOT;
  sm_op->alt_chosen = NULL;

  sm_op->prev_sched_op = NULL;
  sm_op->next_sched_op = NULL;
}

/* 20021211 SZU
 * If occupied slot, return 0
 * If empty slot, create and schedule a NOP. Must have valid template.
 * If slot has NOP, delete and create and schedule next NOP.
 * Follow following syll priority: FMIB
 * 20030603 SZU
 * Alter to accomodate info from mdes. 
 * Follow syllable order specified in mdes.
 * 20030831 SZU
 * Merge in SM_insert_nop_restrict changes for compaction.
 * Different behavior if template is locked.
 */
int
SM_insert_nop (SM_Issue_Group *issue_group_ptr, int slot,
	       unsigned int sched_flags)
{
  SM_Oper *old_sm_op, *new_nop;
  int success, new_syll, old_nop_index, new_nop_index;
  int proc_opc = 0, slots_per_template;
  int bundle_number, current_template, temp_template;
  int new_template, new_template_index = 0, template_shift_var;
  SM_Mdes *sm_mdes;
  Mdes_Operation *op;
  SM_Bundle *current_bundle;

  old_sm_op = issue_group_ptr->slots[slot];
  new_nop = NULL;
  sm_mdes = issue_group_ptr->sm_cb->sm_mdes;

  /* Check if slot is occupied */
  if (old_sm_op)
    {
      /* If old_sm_op is not NOP, return 0 */
      if (!(SM_is_nop (old_sm_op)))
	return 0;

      /* Find the old nop index */
      for (old_nop_index = 0; old_nop_index <= sm_mdes->max_nop_index;
	   old_nop_index++)
	if (old_sm_op->mdes_op->opcode == sm_mdes->nop_array[old_nop_index])
	  break;

      /* If didn't find opcode in nop_array, something wrong */
      if (old_nop_index > sm_mdes->max_nop_index)
	L_punt ("SM_insert_nop: old_sm_nop opcode not found in nop_array!\n");

      if (old_sm_op->flags & SM_OP_SCHEDULED)
	SM_unschedule_nop (old_sm_op);

    }
  /* Empty slot. Start from old_nop_index = -1 */
  else
    {
      old_nop_index = -1;
    }

  /* Recalculate current template bit vectors */
  SM_calculate_template_vector (issue_group_ptr);

  slots_per_template = SM_get_slots_per_template (issue_group_ptr->sm_cb);
  bundle_number = slot / slots_per_template;
  template_shift_var = SM_get_template_shift_var (issue_group_ptr->sm_cb);
  current_bundle = issue_group_ptr->bundles[bundle_number];
  current_template = current_bundle->template_mask;

  success = 0;
  new_nop_index = old_nop_index + 1;
  while (!success)
    {
      /* If current index is > max, then list of NOP exhausted, FAIL */
      if (new_nop_index > sm_mdes->max_nop_index)
	break;

      /* 20030830 SZU
       * Integrating w/ SM_insert_nop_restrict.
       * Make useable by the compaction algorithm.
       * If the template is locked, get the syllable corresponding to the slot,
       * otherwise continue as usual.
       */
      if (current_bundle->template_lock)
	{
	  /* If the template is locked, only one kind of nop possible.
	   * Therefore fail if previous nop exists.
	   * Need this because old_sm_op may not be max_nop_index syllable.
	   */
	  if (old_sm_op)
	    {
	      SM_schedule_oper_schedule (old_sm_op, issue_group_ptr,
					 issue_group_ptr->issue_time,
					 slot, sched_flags);

	      return 0;
	    }

	  new_syll = 
	    (current_template >>
	     ((slots_per_template - (slot % slots_per_template) - 1) *
	      template_shift_var) & (C_pow2 (template_shift_var) - 1));

	  for (new_nop_index = 0; new_nop_index <= sm_mdes->max_nop_index;
	       new_nop_index++)
	    {
	      proc_opc = sm_mdes->nop_array[new_nop_index];
	      op = issue_group_ptr->sm_cb->version1_mdes->op_table[proc_opc];

	      if (op->syll_mask == new_syll)
		break;
	    }

	  /* In case there isn't a nop w/ the right syllable */
	  if (new_nop_index > sm_mdes->max_nop_index)
	    {
	      /* Warn */
	      printf ("SM_insert_nop: template locked and no nop "
		      "found for syllable type 0x%x\n", new_syll);

	      if (old_sm_op)
		SM_schedule_oper_schedule (old_sm_op, issue_group_ptr,
					   issue_group_ptr->issue_time,
					   slot, sched_flags);
	      return 0;
	    }

	  /* Only try this syllable, so set index to max */
	  new_nop_index = sm_mdes->max_nop_index;

	}
      else
	{
	  proc_opc = sm_mdes->nop_array[new_nop_index];
	  op = issue_group_ptr->sm_cb->version1_mdes->op_table[proc_opc];
	  new_syll = op->syll_mask;

	  temp_template =
	    current_template |
	    (new_syll << ((slots_per_template -
			   (slot % slots_per_template) - 1) *
			  template_shift_var));

	  /* Check if temp_template is valid */
	  new_template_index =
	    SM_check_template_validity (issue_group_ptr->sm_cb, temp_template);

	  /* temp_template not valid, choose next syll */
	  if (new_template_index == SM_NO_TEMPLATE)
	    {
	      new_nop_index++;
	      continue;
	    }
	}
      
      /* If new_nop exists from previous failure, delete it */
      if (new_nop)
	SM_delete_nop (new_nop);

      new_nop = SM_create_nop (issue_group_ptr->sm_cb, proc_opc);

      success = SM_schedule_oper_schedule (new_nop, issue_group_ptr, 
					   issue_group_ptr->issue_time, slot,
					   sched_flags);
      if (!success)
	new_nop_index++;
    }

  /* If unsuccessful, restore original nop if any
   * If successful, delete original nop
   */
  if (!success)
    {
      /* 20030108 SZU
       * !success, get rid of new_nop
       */
      if (new_nop)
	SM_delete_nop (new_nop);

      if (old_sm_op)
	SM_schedule_oper_schedule (old_sm_op, issue_group_ptr, 
				   issue_group_ptr->issue_time, slot,
				   sched_flags);
    }
  else if (success && old_sm_op)
    {
      SM_delete_nop (old_sm_op);
    }

  /* Successful schedule of new_nop. Bundle template changed. Update. */
  /* 20030831 SZU
   * Do not update if template locked
   */
  if ((success) && (!current_bundle->template_lock))
    {
      new_template =
	SM_get_template(issue_group_ptr->sm_cb, new_template_index);

      issue_group_ptr->bundles[bundle_number]->template_mask = new_template;
      issue_group_ptr->bundles[bundle_number]->template_index =
	new_template_index;
    }

#if DEBUG_NOP
  if (!success && !old_sm_op)
    printf ("SM_insert_nop: NOP schedule failed on empty slot\n"
	    "time %i slot %i current template 0x%x\n",
	    issue_group_ptr->issue_time, slot, current_template);
#endif

  return success;
}

/* 20030831 SZU
 * Ported from sm_compact.c.
 * 20030711 SZU
 * Copy information from src to dest.
 * Meant for saving information temporarily, not true copy.
 * Assumes dest and src have already been allocated space.
 */
void
SM_copy_issue (SM_Issue_Group *dest_issue_group_ptr,
	       SM_Issue_Group *src_issue_group_ptr)
{
  SM_Oper *sm_op;
  SM_Cb *sm_cb;
  int num_slots, index;

  num_slots = SM_get_num_slots (src_issue_group_ptr->sm_cb);

  if ((!dest_issue_group_ptr) || (!src_issue_group_ptr))
    L_punt ("SM_copy_issue: Expect dest and src issue_group_ptrs to have "
	    "allocated space already!\n");

  sm_cb = src_issue_group_ptr->sm_cb;

  dest_issue_group_ptr->sm_cb = sm_cb;
  dest_issue_group_ptr->full = src_issue_group_ptr->full;
  dest_issue_group_ptr->next_issue_group =
    src_issue_group_ptr->next_issue_group;
  dest_issue_group_ptr->prev_issue_group =
    src_issue_group_ptr->prev_issue_group;

  for (index = 0; index < num_slots; index++)
    {
      sm_op = src_issue_group_ptr->slots[index];

      if (sm_op)
	{
	  /* Not NOP instruction */
	  if (!SM_is_nop (sm_op))
	    {
	      dest_issue_group_ptr->slots[index] = sm_op;
	      sm_op->old_issue_time = sm_op->sched_cycle; 
	    }
	  /* NOP instruction */
	  else
	    {
	      if (sm_op->flags & SM_OP_SCHEDULED)
		SM_unschedule_nop (sm_op);
	      dest_issue_group_ptr->slots[index] = sm_op;
	    }
	}
    }

  for (index = 0; index < SM_get_template_per_issue (sm_cb); index++)
    {
      SM_Bundle *save_bundle, *bundle;

      save_bundle = dest_issue_group_ptr->bundles[index];
      bundle = src_issue_group_ptr->bundles[index];

      save_bundle->template_index = bundle->template_index;
      save_bundle->stop = bundle->stop;
      save_bundle->template_mask = bundle->template_mask;
      save_bundle->empty = bundle->empty;
      save_bundle->template_lock = bundle->template_lock;
      save_bundle->internal_stop_bit = bundle->internal_stop_bit;
    }

  return;
}

/* 20030831 SZU
 * Ported from sm_compact.c.
 * 20030719 SZU
 * Restores the issue group from saved issue group.
 * Includes scheduling the operations.
 */
void
SM_restore_issue_group (SM_Issue_Group *issue_group_ptr,
			SM_Issue_Group *save_issue_group_ptr)
{
  SM_Cb *sm_cb = issue_group_ptr->sm_cb;
  SM_Oper *sm_op;
  int index, num_slots, max_template_per_issue, template_shift_var;
  int bundle_number, template_mask = 0, slots_per_template, done;

  num_slots = SM_get_num_slots (sm_cb);
  max_template_per_issue = SM_get_template_per_issue (sm_cb);
  template_shift_var = SM_get_template_shift_var (sm_cb);
  slots_per_template = SM_get_slots_per_template (sm_cb);

  /* No guarantee on state of issue_group_ptr.
   * Therefore unschedule everything first.
   */
  for (index = 0; index < num_slots; index++)
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
	      if (sm_op->flags & SM_OP_SCHEDULED)
		SM_unschedule_oper (sm_op, NULL);
	    }
	}
    }

  issue_group_ptr->num_slots_left = num_slots;

  /* Restore all bundle information.
   * Saved version could already be using internal stops.
   */
  for (index = 0; index < max_template_per_issue; index++)
    {
      SM_Bundle *save_bundle_ptr;
      SM_Bundle *bundle_ptr;

      save_bundle_ptr = save_issue_group_ptr->bundles[index];
      bundle_ptr = issue_group_ptr->bundles[index];

      bundle_ptr->template_mask = save_bundle_ptr->template_mask;
      bundle_ptr->template_index = save_bundle_ptr->template_index;
      bundle_ptr->template_lock = save_bundle_ptr->template_lock;
      bundle_ptr->internal_stop_bit =
	save_bundle_ptr->internal_stop_bit;
      bundle_ptr->stop = save_bundle_ptr->stop;
      bundle_ptr->empty = save_bundle_ptr->empty;
    }

  /* Reschedule original ops */
  for (index = 0; index < num_slots; index++)
    {
      bundle_number = index / slots_per_template;

      /* Save the template at the beginning of a bundle */
      if (index % slots_per_template == 0)
	template_mask =
	  save_issue_group_ptr->bundles[bundle_number]->
	  template_mask;

      if ((sm_op = save_issue_group_ptr->slots[index]))
	{
	  /* Need to get saved syll_type for A types */
	  /* 20030606 SZU
	   * Need to get save syll_type for multiple syll op
	   */
	  if ((sm_op->mdes_op->syll_mask % 2 != 0) &&
	      (sm_op->mdes_op->syll_mask != 1))
	    {
	      sm_op->syll_type = 
		(template_mask >>
		 ((slots_per_template -
		   (index % slots_per_template) - 1) *
		  template_shift_var)) & sm_op->mdes_op->syll_mask;
	    }

	  if (sm_op->old_issue_time != -1)
	    {
	      done = SM_schedule_oper_schedule (sm_op, issue_group_ptr,
						sm_op->old_issue_time,
						index, 0);
	      if (!done)
		L_punt ("SM_restore_issue_group: "
			"Error recovering old op from failed "
			"scheduling\n"
			"CB: %i Op: %i Time: %i Slot: %i\n",
			sm_cb->lcode_cb->id, sm_op->lcode_op->id,
			sm_op->old_issue_time, index);
	    }
	  else
	    {
	      done = SM_schedule_oper_schedule (sm_op, issue_group_ptr,
						issue_group_ptr->issue_time,
						index, 0);
	      if (!done)
		L_punt ("SM_restore_issue_group: "
			"Error recovering old op from failed "
			"scheduling\n"
			"CB: %i Op: %i Time: %i Slot: %i\n",
			sm_cb->lcode_cb->id, sm_op->lcode_op->id,
			issue_group_ptr->issue_time, index);
	    }

	  /* 20030909 SZU
	   * Only change num_slots_left if not nop
	   */
	  if (!SM_is_nop (sm_op))
	    issue_group_ptr->num_slots_left -= sm_op->mdes_op->num_slots;
	}
    }

  SM_issue_group_full_check (issue_group_ptr);
  SM_calculate_template_vector (issue_group_ptr);
}

/* 20030302 SZU
 * SMH reconciliation
 * Minor changes, but radical change in structure depending on 
 * SM_update_upper(lower)_bounds functions
 */
/* This routine is called right after sm_op is scheduled, so that
 * the upper and lower bounds of the adjacent operation can be updated.
 */
void
SM_update_adjacent_bounds (SM_Oper * sm_op)
{
  SM_Oper *from_op, *to_op;
  SM_Reg_Action *op_action, *from_action, *to_action;
  SM_Dep *dep_in, *dep_out;
  short min_delay, max_delay, delay_offset;
  short actual_early_use_time, actual_late_use_time;
  int op_sched_cycle, II;
  unsigned short op_sched_slot;
  /* 20031024 SZU */
  SM_Dep_PCLat *dep_pclat;
  int from_penalty, to_penalty;
  int max_from_penalty, min_from_penalty, max_to_penalty, min_to_penalty;

  /* Get the scheduled cycle and slot of this operation */
  op_sched_cycle = sm_op->sched_cycle;
  op_sched_slot = sm_op->sched_slot;

  /* Get II for ease of use -ITI/JCG 8/99 */
  II = sm_op->sm_cb->II;

  /* For all the actions, update other operations bounds using this dep */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* If op action had a range of early use times, update the mdes-based
       * dependences into this action.
       */
      if (op_action->min_early_use_time != op_action->max_early_use_time)
        {
          /* Get the actual early use time for this action after sched */
          actual_early_use_time = op_action->actual_early_use_time;

          /* Update the mdes-based dep ins with the actual early use time */
          for (dep_in = op_action->first_dep_in; dep_in != NULL;
               dep_in = dep_in->next_dep_in)
            {
              /* Only process mdes-based dep-ins */
              if (!(dep_in->flags & SM_MDES_BASED_DELAY))
                continue;

              /* Get the action and operation this dependence is from */
              from_action = dep_in->from_action;
              from_op = from_action->sm_op;

              /* Get the dep_in's delay offset for ease of use */
              delay_offset = dep_in->delay_offset;

              /* If from op is scheduled, then use known use times 
               * (min and max delays will be the same)
               */
              if (from_op->flags & SM_OP_SCHEDULED)
                {
                  min_delay = delay_offset +
                    from_action->actual_late_use_time - actual_early_use_time;
                  max_delay = min_delay;
                }

              /* Otherwise, use min and max times from from_op */
              else
                {
                  /* Min delay is earliest def time - actual use time */
                  min_delay = delay_offset + from_action->min_late_use_time -
                    actual_early_use_time;

                  /* Max delay is latest def time - actual use time */
                  max_delay = delay_offset + from_action->max_late_use_time -
                    actual_early_use_time;
                }

	      /* 20020808 SZU
	       * Check if the producer & consumer fit
	       * any of the special producer_consumer latencies.
	       */
	      max_from_penalty = 0;
	      min_from_penalty = 0;
	      max_to_penalty = 0;
	      min_to_penalty = 0;

	      if (from_action->sm_op->sm_cb->sm_mdes->pclat_array != NULL)
		{
		  if (dep_in->pclat_list != NULL)
		    {
		      dep_pclat = dep_in->pclat_list;

		      max_from_penalty = dep_pclat->from_penalty;
		      min_from_penalty = dep_pclat->from_penalty;
		      max_to_penalty = dep_pclat->to_penalty;
		      min_to_penalty = dep_pclat->to_penalty;

		      for (dep_pclat = dep_pclat->next_dep_pclat;
			   dep_pclat != NULL;
			   dep_pclat = dep_pclat->next_dep_pclat)
			{
			  from_penalty = dep_pclat->from_penalty;
			  to_penalty = dep_pclat->to_penalty;

			  if (from_penalty < min_from_penalty)
			    min_from_penalty = from_penalty;
			  if (from_penalty > max_from_penalty)
			    max_from_penalty = from_penalty;

			  if (to_penalty < min_to_penalty)
			    min_to_penalty = to_penalty;
			  if (to_penalty > max_to_penalty)
			    max_to_penalty = to_penalty;
			}
#if DEBUG_PCLAT
		      printf ("PCLat found\n");
		      printf ("Producer:\n");
		      SM_print_oper (stdout, from_action->sm_op);
		      printf ("Consumer:\n");
		      SM_print_oper (stdout, op_action->sm_op);
		      printf ("max_from_penalty: %i\n", max_from_penalty);
		      printf ("min_from_penalty: %i\n", min_from_penalty);
		      printf ("max_to_penalty: %i\n", max_to_penalty);
		      printf ("min_to_penalty: %i\n", min_to_penalty);
#endif
		    }
		}

	      /* This prod_cons have penalty latency.
	       * And it applies to this set of actions (operands)
	       */
	      if (max_from_penalty && max_to_penalty)
		{
		  min_delay += min_from_penalty;
		  max_delay += max_from_penalty;
		}

              /* Superscalar scheduler, to not allow negative deps */
              if (min_delay < 0)
                min_delay = 0;
              if (max_delay < 0)
                max_delay = 0;

              /* Update the dep_in's delays */
              dep_in->min_delay = min_delay;
              dep_in->max_delay = max_delay;

              /* Reset SM_VARIABLE_DELAY flag if necessary */
              if (min_delay == max_delay)
                dep_in->flags &= ~SM_VARIABLE_DELAY;
            }
        }

      /* For every dep in, update upper bounds of from_op (except
       * for deps with SM_IGNORE_DEP).
       */
      for (dep_in = op_action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Get the action and operation this dependence is from */
          from_action = dep_in->from_action;
          from_op = from_action->sm_op;

          /* Update resolved counts, that is all we do if
           * dependence is marked with SM_IGNORE_DEP
           */
          if (dep_in->ignore)
            {
              from_op->num_unresolved_ignore_dep_out--;
              continue;
            }
          else if (dep_in->flags & SM_HARD_DEP)
            {
              from_op->num_unresolved_hard_dep_out--;
            }
          else
            {
              from_op->num_unresolved_soft_dep_out--;
            }

          SM_update_upper_bounds (dep_in, dep_in->flags);
        }


      /* If op action had a range of late use times, update the mdes-based
       * dependences out of this action.
       */
      if (op_action->min_late_use_time != op_action->max_late_use_time)
        {
          /* Get the actual late use time for this action after sched */
          actual_late_use_time = op_action->actual_late_use_time;

          /* Update the mdes-based dep outs with the actual early use time */
          for (dep_out = op_action->first_dep_out; dep_out != NULL;
               dep_out = dep_out->next_dep_out)
            {
              /* Only process mdes-based dep_outs */
              if (!(dep_out->flags & SM_MDES_BASED_DELAY))
                continue;

              /* Get the action and operation this dependence is to */
              to_action = dep_out->to_action;
              to_op = to_action->sm_op;

              /* Get the dep_out's delay offset for ease of use */
              delay_offset = dep_out->delay_offset;

              /* If to op is scheduled, then use known use times 
               * (min and max delays the same)
               */
              if (to_op->flags & SM_OP_SCHEDULED)
                {
                  min_delay = delay_offset + actual_late_use_time -
                    to_action->actual_early_use_time;
                  max_delay = min_delay;
                }

              /* Otherwise, use min and max times from to_op */
              else
                {
                  /* Min delay is actual def time - latest use time */
                  min_delay = delay_offset + actual_late_use_time -
                    to_action->max_early_use_time;

                  /* Max delay is actual def time - earliest use time */
                  max_delay = delay_offset + actual_late_use_time -
                    to_action->min_early_use_time;
                }

	      /* 20020808 SZU
	       * Check if the producer & consumer fit
	       * any of the special producer_consumer latencies.
	       */
	      max_from_penalty = 0;
	      min_from_penalty = 0;
	      max_to_penalty = 0;
	      min_to_penalty = 0;

	      if (op_action->sm_op->sm_cb->sm_mdes->pclat_array != NULL)
		{
		  if (dep_out->pclat_list != NULL)
		    {
		      dep_pclat = dep_out->pclat_list;

		      max_from_penalty = dep_pclat->from_penalty;
		      min_from_penalty = dep_pclat->from_penalty;
		      max_to_penalty = dep_pclat->to_penalty;
		      min_to_penalty = dep_pclat->to_penalty;

		      for (dep_pclat = dep_pclat->next_dep_pclat;
			   dep_pclat != NULL;
			   dep_pclat = dep_pclat->next_dep_pclat)
			{
			  from_penalty = dep_pclat->from_penalty;
			  to_penalty = dep_pclat->to_penalty;

			  if (from_penalty < min_from_penalty)
			    min_from_penalty = from_penalty;
			  if (from_penalty > max_from_penalty)
			    max_from_penalty = from_penalty;

			  if (to_penalty < min_to_penalty)
			    min_to_penalty = to_penalty;
			  if (to_penalty > max_to_penalty)
			    max_to_penalty = to_penalty;
			}
#if DEBUG_PCLAT
		      printf ("PCLat found\n");
		      printf ("Producer:\n");
		      SM_print_oper (stdout, op_action->sm_op);
		      printf ("Consumer:\n");
		      SM_print_oper (stdout, to_action->sm_op);
		      printf ("max_from_penalty: %i\n", max_from_penalty);
		      printf ("min_from_penalty: %i\n", min_from_penalty);
		      printf ("max_to_penalty: %i\n", max_to_penalty);
		      printf ("min_to_penalty: %i\n", min_to_penalty);
#endif
		    }
		}

	      /* This prod_cons have penalty latency.
	       * And it applies to this set of actions (operands)
	       */
	      if (max_from_penalty && max_to_penalty)
		{
		  min_delay += min_from_penalty;
		  max_delay += max_from_penalty;
		}

              /* Superscalar scheduler, to not allow negative deps */
              if (min_delay < 0)
                min_delay = 0;
              if (max_delay < 0)
                max_delay = 0;

              /* Update the dep_out's delays */
              dep_out->min_delay = min_delay;
              dep_out->max_delay = max_delay;

              /* Reset SM_VARIABLE_DELAY flag if necessary */
              if (min_delay == max_delay)
                dep_out->flags &= ~SM_VARIABLE_DELAY;
            }
        }

      /* For every dep out, update upper bounds of to_op.
       * Ignore deps marked with SM_IGNORE_DEP.
       */
      for (dep_out = op_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          /* Get the action and operation this dependence is to */
          to_action = dep_out->to_action;
          to_op = to_action->sm_op;

          /* Update resolved counts, that is all we do if
           * dependence is marked with SM_IGNORE_DEP
           */
          if (dep_out->ignore)
            {
              to_op->num_unresolved_ignore_dep_in--;
              continue;
            }
          else if (dep_out->flags & SM_HARD_DEP)
            {
              /* Debug */
              if (to_op->num_unresolved_hard_dep_in == 0)
                {
                  L_punt ("SM_update_adjacent_bounds: %s op %i: "
                          "num_unresolved_hard_dep_in already zero",
                          to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
                }
              to_op->num_unresolved_hard_dep_in--;
            }
          else
            {
              to_op->num_unresolved_soft_dep_in--;
            }

          /* If this to_op now has all its dep_in resolved, add it to
           * the end of dep_in_resolved queue (if it is not scheduled!).
           */
#if 1
          if ( /*EMN*/ (dep_out->flags & SM_HARD_DEP) &&
              (to_op->num_unresolved_hard_dep_in == 0) &&
              /* EMN (to_op->num_unresolved_soft_dep_in == 0) && */
              !(to_op->flags & SM_OP_SCHEDULED) && (!to_op->ignore))
#else
          if ((to_op->num_unresolved_hard_dep_in == 0) &&
              /* EMN (to_op->num_unresolved_soft_dep_in == 0) && */
              !(to_op->flags & SM_OP_SCHEDULED) && (!to_op->ignore))
#endif
            {
              /* Sanity check, better not already be in this queue */
              if (to_op->dep_in_resolved_qentry != NULL)
                {
                  L_punt ("SM_update_adjacent_bounds:\n"
                          "    %s op %i already in resolved queue!",
                          to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
                }

              to_op->dep_in_resolved_qentry =
                SM_enqueue_oper_before (to_op->sm_cb->dep_in_resolved,
                                        to_op, NULL);
            }

          SM_update_lower_bounds (dep_out, dep_out->flags);
        }
    }
}

/* This routine is called right after sm_op is unscheduled, so that
 * the upper and lower bounds of the adjacent operation can be recalculated.
 * 20030829 SZU
 * Add priority_queue parameter as a flag and data.
 * Usually data from SM_schedule_oper_priority, when scheduling for Itanium.
 * Avoids unnecessary dependence checks outside of those operations.
 * If not, set to NULL for flag.
 */
static void
SM_recalculate_adjacent_bounds (SM_Oper *sm_op,
				SM_Priority_Queue *priority_queue)
{
  SM_Oper *from_op, *to_op;
  SM_Reg_Action *op_action, *from_action, *to_action;
  SM_Dep *dep_in, *dep_out = NULL;
  short min_delay, max_delay, delay_offset;
  short actual_early_use_time, actual_late_use_time;
  short min_early_use_time, max_early_use_time;
  short min_late_use_time, max_late_use_time;
  /* 20031024 SZU */
  SM_Dep_PCLat *dep_pclat;
  int from_penalty, to_penalty;
  int max_from_penalty, min_from_penalty, max_to_penalty, min_to_penalty;

  /* Make sure operation is marked unscheduled! */
  if (sm_op->flags & SM_OP_SCHEDULED)
    L_punt ("SM_recalculate_adjacent_bounds: Error, op marked scheduled!");

  /* For all the actions, update other operations bounds using this dep */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* If op action had a range of early use times, update the mdes-based
       * dependences into this action.
       */
      if (op_action->min_early_use_time != op_action->max_early_use_time)
        {
          /* Get the min and max early use time for this operation for
           * ease of use.
           */
          min_early_use_time = op_action->min_early_use_time;
          max_early_use_time = op_action->max_early_use_time;

          /* Update the mdes-based dep ins with the original range
           * of early use times 
           */
          for (dep_in = op_action->first_dep_in; dep_in != NULL;
               dep_in = dep_in->next_dep_in)
            {
              /* Only process mdes-based dep-ins */
              if (!(dep_in->flags & SM_MDES_BASED_DELAY))
                continue;

              /* Get the action and operation this dependence is from */
              from_action = dep_in->from_action;
              from_op = from_action->sm_op;

              /* Get the dep_in's delay offset for ease of use */
              delay_offset = dep_in->delay_offset;

              /* If from op is scheduled, then use known use times 
               * (min and max delays will be the same)
               */
              if (from_op->flags & SM_OP_SCHEDULED)
                {
                  actual_late_use_time = from_action->actual_late_use_time;
                  min_delay = delay_offset + actual_late_use_time -
                    max_early_use_time;
                  max_delay = delay_offset + actual_late_use_time -
                    min_early_use_time;
                }

              /* Otherwise, use min and max times from from_op */
              else
                {
                  /* Min delay is earliest def time - max early use time */
                  min_delay = delay_offset + from_action->min_late_use_time -
                    max_early_use_time;

                  /* Max delay is latest def time - min early use time */
                  max_delay = delay_offset + from_action->max_late_use_time -
                    min_early_use_time;
                }

	      /* 20020808 SZU
	       * Check if the producer & consumer fit
	       * any of the special producer_consumer latencies.
	       */
	      max_from_penalty = 0;
	      min_from_penalty = 0;
	      max_to_penalty = 0;
	      min_to_penalty = 0;

	      if (from_action->sm_op->sm_cb->sm_mdes->pclat_array != NULL)
		{
		  if (dep_in->pclat_list != NULL)
		    {
		      dep_pclat = dep_in->pclat_list;

		      max_from_penalty = dep_pclat->from_penalty;
		      min_from_penalty = dep_pclat->from_penalty;
		      max_to_penalty = dep_pclat->to_penalty;
		      min_to_penalty = dep_pclat->to_penalty;

		      for (dep_pclat = dep_pclat->next_dep_pclat;
			   dep_pclat != NULL;
			   dep_pclat = dep_pclat->next_dep_pclat)
			{
			  from_penalty = dep_pclat->from_penalty;
			  to_penalty = dep_pclat->to_penalty;

			  if (from_penalty < min_from_penalty)
			    min_from_penalty = from_penalty;
			  if (from_penalty > max_from_penalty)
			    max_from_penalty = from_penalty;

			  if (to_penalty < min_to_penalty)
			    min_to_penalty = to_penalty;
			  if (to_penalty > max_to_penalty)
			    max_to_penalty = to_penalty;
			}
#if DEBUG_PCLAT
		      printf ("PCLat found\n");
		      printf ("Producer:\n");
		      SM_print_oper (stdout, from_action->sm_op);
		      printf ("Consumer:\n");
		      SM_print_oper (stdout, op_action->sm_op);
		      printf ("max_from_penalty: %i\n", max_from_penalty);
		      printf ("min_from_penalty: %i\n", min_from_penalty);
		      printf ("max_to_penalty: %i\n", max_to_penalty);
		      printf ("min_to_penalty: %i\n", min_to_penalty);
#endif
		    }
		}

	      /* This prod_cons have penalty latency.
	       * And it applies to this set of actions (operands)
	       */
	      if (max_from_penalty && max_to_penalty)
		{
		  min_delay += min_from_penalty;
		  max_delay += max_from_penalty;
		}

              /* Superscalar scheduler, to not allow negative deps */
              if (min_delay < 0)
                min_delay = 0;
              if (max_delay < 0)
                max_delay = 0;

              /* Update the dep_in's delays */
              dep_in->min_delay = min_delay;
              dep_in->max_delay = max_delay;

              /* Set SM_VARIABLE_DELAY flag as necessary */
              if (min_delay != max_delay)
                dep_out->flags |= SM_VARIABLE_DELAY;
            }
        }

      /* For every dep in, update from_op's counters and if this dep 
       * caused the from_op's upper bound, recalculate the 
       * from_op's upper bound.
       */
      for (dep_in = op_action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Get the action and operation this dependence is from */
          from_action = dep_in->from_action;
          from_op = from_action->sm_op;

          /* Update resolved counts, that is all we do if
           * dependence is marked with SM_IGNORE_DEP
           */
          if (dep_in->ignore)
            {
              from_op->num_unresolved_ignore_dep_out++;
              continue;
            }
          else if (dep_in->flags & SM_HARD_DEP)
            {
              from_op->num_unresolved_hard_dep_out++;
            }
          else
            {
              from_op->num_unresolved_soft_dep_out++;
            }

          /* If this dependence caused the from_op's upper cycle bound,
           * we need to recalculate it (which may move it later).
           */
          if (from_op->dep_upper_bound == dep_in)
            SM_recalculate_upper_bound (from_op);
        }


      /* If op action had a range of late use times, update the mdes-based
       * dependences out of this action.
       */
      if (op_action->min_late_use_time != op_action->max_late_use_time)
        {
          /* Get the min and max late use time for this action */
          min_late_use_time = op_action->min_late_use_time;
          max_late_use_time = op_action->max_late_use_time;

          /* Update the mdes-based dep outs with the original range
           * of late use times
           */
          for (dep_out = op_action->first_dep_out; dep_out != NULL;
               dep_out = dep_out->next_dep_out)
            {
              /* Only process mdes-based dep_outs */
              if (!(dep_out->flags & SM_MDES_BASED_DELAY))
                continue;

              /* Get the action and operation this dependence is to */
              to_action = dep_out->to_action;
              to_op = to_action->sm_op;

              /* Get the dep_out's delay offset for ease of use */
              delay_offset = dep_out->delay_offset;

              /* If to op is scheduled, then use known use times 
               * (min and max delays the same)
               */
              if (to_op->flags & SM_OP_SCHEDULED)
                {
                  actual_early_use_time = to_action->actual_early_use_time;
                  min_delay = delay_offset + min_late_use_time -
                    actual_early_use_time;
                  max_delay = delay_offset + max_late_use_time -
                    actual_early_use_time;
                }

              /* Otherwise, use min and max times from to_op */
              else
                {
                  /* Min delay is earliest def time - latest use time */
                  min_delay = delay_offset + min_late_use_time -
                    to_action->max_early_use_time;

                  /* Max delay is latest def time - earliest use time */
                  max_delay = delay_offset + max_late_use_time -
                    to_action->min_early_use_time;
                }

	      /* 20020808 SZU
	       * Check if the producer & consumer fit
	       * any of the special producer_consumer latencies.
	       */
	      max_from_penalty = 0;
	      min_from_penalty = 0;
	      max_to_penalty = 0;
	      min_to_penalty = 0;

	      if (op_action->sm_op->sm_cb->sm_mdes->pclat_array != NULL)
		{
		  if (dep_out->pclat_list != NULL)
		    {
		      dep_pclat = dep_out->pclat_list;

		      max_from_penalty = dep_pclat->from_penalty;
		      min_from_penalty = dep_pclat->from_penalty;
		      max_to_penalty = dep_pclat->to_penalty;
		      min_to_penalty = dep_pclat->to_penalty;

		      for (dep_pclat = dep_pclat->next_dep_pclat;
			   dep_pclat != NULL;
			   dep_pclat = dep_pclat->next_dep_pclat)
			{
			  from_penalty = dep_pclat->from_penalty;
			  to_penalty = dep_pclat->to_penalty;

			  if (from_penalty < min_from_penalty)
			    min_from_penalty = from_penalty;
			  if (from_penalty > max_from_penalty)
			    max_from_penalty = from_penalty;

			  if (to_penalty < min_to_penalty)
			    min_to_penalty = to_penalty;
			  if (to_penalty > max_to_penalty)
			    max_to_penalty = to_penalty;
			}
#if DEBUG_PCLAT
		      printf ("PCLat found\n");
		      printf ("Producer:\n");
		      SM_print_oper (stdout, op_action->sm_op);
		      printf ("Consumer:\n");
		      SM_print_oper (stdout, to_action->sm_op);
		      printf ("max_from_penalty: %i\n", max_from_penalty);
		      printf ("min_from_penalty: %i\n", min_from_penalty);
		      printf ("max_to_penalty: %i\n", max_to_penalty);
		      printf ("min_to_penalty: %i\n", min_to_penalty);
#endif
		    }
		}

	      /* This prod_cons have penalty latency.
	       * And it applies to this set of actions (operands)
	       */
	      if (max_from_penalty && max_to_penalty)
		{
		  min_delay += min_from_penalty;
		  max_delay += max_from_penalty;
		}

              /* Superscalar scheduler, to not allow negative deps */
              if (min_delay < 0)
                min_delay = 0;
              if (max_delay < 0)
                max_delay = 0;

              /* Update the dep_out's delays */
              dep_out->min_delay = min_delay;
              dep_out->max_delay = max_delay;

              /* Set SM_VARIABLE_DELAY flag as necessary */
              if (min_delay != max_delay)
                dep_out->flags |= SM_VARIABLE_DELAY;
            }
        }

      /* For every dep out, update upper bounds of to_op.
       * Ignore deps marked with SM_IGNORE_DEP.
       */
      for (dep_out = op_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
	  /* 20030829 SZU */
	  SM_Priority_Qentry *current_qentry;
	  int in_cycle;

          /* Get the action and operation this dependence is to */
          to_action = dep_out->to_action;
          to_op = to_action->sm_op;

          /* Update resolved counts, that is all we do if
           * dependence is marked with SM_IGNORE_DEP
           */
          if (dep_out->ignore)
            {
              to_op->num_unresolved_ignore_dep_in++;
              continue;
            }
          else if (dep_out->flags & SM_HARD_DEP)
            {
              to_op->num_unresolved_hard_dep_in++;
            }
          else
            {
              to_op->num_unresolved_soft_dep_in++;
            }

          /* If this to_op is in the dep_in_resolved queue, remove it */
#if 0
          if (to_op->dep_in_resolved_qentry != NULL)
#else
          if ((to_op->dep_in_resolved_qentry != NULL) &&
	      (dep_out->flags & SM_HARD_DEP))
#endif
            {
              SM_dequeue_oper (to_op->dep_in_resolved_qentry);
              to_op->dep_in_resolved_qentry = NULL;
            }

          /* If this dependence caused the from_op's upper cycle bound,
           * we need to recalculate it (which may move it later).
           */
	  /* 20030829 SZU
	   * If priority_queue is not NULL, check if to_op is in it.
	   * Only calculate lower bound if to_op is in it.
	   * Cuts down on needless checks when shuffling within an issue time
	   * for Itanium.
	   */
#if 0
          if (to_op->dep_lower_bound == dep_out)
            SM_recalculate_lower_bound (to_op);
#else
	  if (priority_queue)
	    {
	      in_cycle = 0;
	      for (current_qentry = priority_queue->first_qentry;
		   current_qentry != NULL;
		   current_qentry = current_qentry->next_qentry)
		{
		  if (to_op == current_qentry->oper)
		    in_cycle = 1;
		}

	      if ((in_cycle) && (to_op->dep_lower_bound == dep_out))
		SM_recalculate_lower_bound (to_op);
	    }
	  else
	    {
	      if (to_op->dep_lower_bound == dep_out)
		SM_recalculate_lower_bound (to_op);
	    }
#endif
        }
    }

  /* Recalculate this operation's upper and lower bounds */
  SM_recalculate_upper_bound (sm_op);
  SM_recalculate_lower_bound (sm_op);

  /* Add this operation to the dep_in_resolved queue if necessary */
  if ((sm_op->num_unresolved_hard_dep_in == 0) &&
      /* EMN (sm_op->num_unresolved_soft_dep_in == 0) && */
      (!sm_op->ignore))
    {
      /* Sanity check, better not already be in this queue */
      if (sm_op->dep_in_resolved_qentry != NULL)
        {
          L_punt ("SM_recalculate_adjacent_bounds:\n"
                  "    %s op %i already in resolved queue!",
                  sm_op->sm_cb->lcode_fn->name, sm_op->lcode_op->id);
        }

      sm_op->dep_in_resolved_qentry =
        SM_enqueue_oper_before (sm_op->sm_cb->dep_in_resolved, sm_op, NULL);
    }
}

/* This routine recalculates the cycle_upper_bound, slot_upper_bound,
 * and dep_upper_bound for the sm_op.
 */
void
SM_recalculate_upper_bound (SM_Oper * sm_op)
{
  SM_Oper *to_op;
  SM_Reg_Action *op_action;
  SM_Dep *dep_out;
  int dep_delay, dep_omega, II, cycle_dep_resolved;
  unsigned short max_possible_slot;

  /* Get max slot for ease of use */
  max_possible_slot = SM_MAX_SLOT;

  /* Initialized upper bound to the extreme bounds */
  sm_op->cycle_upper_bound = SM_MAX_CYCLE;
  sm_op->slot_upper_bound = max_possible_slot;
  sm_op->dep_upper_bound = NULL;
  sm_op->nosoft_cycle_upper_bound = SM_MAX_CYCLE;
  sm_op->nosoft_slot_upper_bound = max_possible_slot;
  sm_op->nosoft_dep_upper_bound = NULL;

  /* Get II for ease of use -ITI/JCG 8/99 */
  II = sm_op->sm_cb->II;

  /* Scan all the deps for all the actions */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {

      /* Scan the dep out constraints to build upper bound */
      for (dep_out = op_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          /* Ignore deps marked with SM_IGNORE_DEP */
          if (dep_out->ignore)
            continue;

          /* Get the op this action is from */
          to_op = dep_out->to_action->sm_op;

          /* Only look at scheduled operations */
          if (!(to_op->flags & SM_OP_SCHEDULED))
            continue;

          /* Don't need to do anything special for variable delay ops,
           * since they are updated automatically during scheduling.
           *
           * Use min delay for bounding calculation.
           *
           * If omega is not 0, use II in calculation. -ITI/JCG 8/99
           */
          dep_delay = dep_out->min_delay;
          dep_omega = dep_out->omega;
          if (dep_omega == 0)
            {
              cycle_dep_resolved = to_op->sched_cycle - dep_delay;
            }
          else
            {
              cycle_dep_resolved = to_op->sched_cycle - dep_delay +
                (dep_omega * II);
            }

          SM_update_upper_bounds (dep_out, dep_out->flags);
        }
    }
}

/* This routine recalculates the cycle_lower_bound, slot_lower_bound,
 * and dep_lower_bound for the sm_op.
 */
void
SM_recalculate_lower_bound (SM_Oper * sm_op)
{
  SM_Oper *from_op;
  SM_Reg_Action *op_action;
  SM_Dep *dep_in;
  int II;
  unsigned short max_possible_slot;

  /* Get max slot for ease of use */
  max_possible_slot = SM_MAX_SLOT;

  /* Initialized lower bounds to the extreme bounds */
  sm_op->cycle_lower_bound = SM_MIN_CYCLE;
  sm_op->slot_lower_bound = 0;
  sm_op->dep_lower_bound = NULL;
  sm_op->nosoft_cycle_lower_bound = SM_MIN_CYCLE;
  sm_op->nosoft_slot_lower_bound = 0;
  sm_op->nosoft_dep_lower_bound = NULL;

  /* Get II for ease of use -ITI/JCG 8/99 */
  II = sm_op->sm_cb->II;

  /* Scan all the deps for all the actions */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* Scan the dep in constraints to build lower bound */
      for (dep_in = op_action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Ignore deps marked with SM_IGNORE_DEP */
          if (dep_in->ignore)
            continue;

          /* Get the op this action is from */
          from_op = dep_in->from_action->sm_op;

          /* Only look at scheduled operations */
          if (!(from_op->flags & SM_OP_SCHEDULED))
            continue;

          SM_update_lower_bounds (dep_in, dep_in->flags);
        }
    }
}

/* Determines if the passed alt is ready to be scheduled by checking
 * ONLY dependences with variable delays.  Several assumptions have
 * been make this routine efficient:
 * 
 * 1) All non-variable delay dependences are satisfied.
 * 
 * 2) The minimum delay for each variable delay dependence has been satified
 *    (thus zero-cycle variable delays have already been handled and
 *     don't need to be handled here!)
 * 
 * Returns 1 if alt is ready this cycle (with above constraints), otherwise
 * returns 0.
 */
int
SM_alt_ready (SM_Oper * sm_op, SM_Compatible_Alt * alt, int issue_time)
{
  int *early_operand_latency, *late_operand_latency;
  SM_Reg_Action *op_action, *from_action, *to_action;
  SM_Oper *from_op, *to_op;
  SM_Dep *dep_in, *dep_out;
  int cycle_dep_resolved, actual_delay;

  /* For now, both normal and speculative alternatives are required
   * to have the same latency, so use the normal version's latency.
   */
  early_operand_latency = alt->normal_version->latency->operand_latency;
  late_operand_latency = early_operand_latency;

  /* For each variable action, examine each incoming and outgoing
   * dependence to make sure it is satified in this cycle.
   */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* Only variable actions can have unsatified dependences */
      if (!(op_action->flags & SM_VARIABLE_ACTION))
        continue;

      /* Check each dep in that has a variable delay */
      for (dep_in = op_action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          /* Only variable delay dependences can be unsatified */
          if (!(dep_in->flags & SM_VARIABLE_DELAY))
            continue;

          /* Ignore deps marked with SM_IGNORE_DEP */
          if (dep_in->ignore)
            continue;

          /* Get the action and op dependence is coming from */
          from_action = dep_in->from_action;
          from_op = from_action->sm_op;

          /* Handle self-dependence for modulo scheduled code.
           * Check to make sure II is big enough for the actual
           * dependence distance to be resolved.  -ITI/JCG 8/99 
           *
           * Note: Only need to do this check for the dep_in section
           *       since it will also check the self-dependent dep_outs.
           *
           * Note: This check is necessary since RecII calculation can
           *       be fooled and generate an II that is too small when
           *       there are variable latency dependences.  
           */
          if ((from_op == sm_op) &&
              (sm_op->sm_cb->flags & SM_MODULO_RESOURCES))
            {
              /* Determine actual delay of self-dependence */
              actual_delay = late_operand_latency[from_action->index] +
                dep_in->delay_offset -
                early_operand_latency[op_action->index];

              /* If this alternative can never be scheduled due to
               * self-dependence and the current value of II, return
               * 0 now.
               */
              if ((actual_delay - (dep_in->omega * sm_op->sm_cb->II)) > 0)
                return (0);

            }


          /* Dep can only be unsatified if from_op scheduled */
          if (!(from_op->flags & SM_OP_SCHEDULED))
            continue;

          /* Calculate the cycle this dep is resolved using the
           * MDES based use times for this alternative and
           * the scheduled from_op.  (Only MDES-based dependences
           * may have variable delay.)
           * The cycle resolved is from_op->sched_cycle + dep_delay.
           */
          cycle_dep_resolved = from_op->sched_cycle +
            (from_action->actual_late_use_time + dep_in->delay_offset -
             early_operand_latency[op_action->index]);

          /* Alt not ready if dep resolved after issue_time scheduling for */
          if (cycle_dep_resolved > issue_time)
            return (0);
        }

      /* Check each dep out that has a variable delay */
      for (dep_out = op_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          /* Only variable delay dependences can be unsatified */
          if (!(dep_out->flags & SM_VARIABLE_DELAY))
            continue;

          /* Ignore deps marked with SM_IGNORE_DEP */
          if (dep_out->ignore)
            continue;

          /* Get the action and op dependence is coming from */
          to_action = dep_out->to_action;
          to_op = to_action->sm_op;

          /* Dep can only be unsatified if to_op scheduled */
          if (!(to_op->flags & SM_OP_SCHEDULED))
            continue;

          /* Calculate the cycle this dep is resolved using the
           * MDES based use times for this alternative and
           * the scheduled to_op.  (Only MDES-based dependences
           * may have variable delay.)
           * The cycle resolved is to_op->sched_cycle - dep_delay.
           */
          cycle_dep_resolved = to_op->sched_cycle -
            (late_operand_latency[op_action->index] + dep_out->delay_offset -
             to_action->actual_early_use_time);

          /* Alt not ready if dep resolved before issue_time scheduling for */
          if (cycle_dep_resolved < issue_time)
            return (0);
        }
    }

  /* If reached here, alternative must be ready */
  return (1);
}

/* 20021211 SZU
 * Actual function call to schedule
 * 20030602 SZU
 * Altered to accomodate mdes format.
 * 20030831 SZU
 * Merge in changes in SM_schedule_oper_schedule_restrict for compaction.
 * Different behavior if template is locked.
 */
int
SM_schedule_oper_schedule (SM_Oper *sm_op, SM_Issue_Group *issue_group_ptr,
			   int issue_time, int slot, unsigned int sched_flags)
{
  SM_Cb *sm_cb;
  SM_Oper *after_this_op;
  SM_Reg_Action *op_action;
  SM_Compatible_Alt *compatible_alt;
  Mdes_Stats *stats;
  unsigned int op_flags;
  int sched_slot, commit_sched, *operand_latency;
  int current_template, temp_template, bundle_number;
  int new_template, new_template_index, slots_per_template, template_shift_var;
  int syll_type, locked_syll;
  SM_Bundle *current_bundle;

  /* The the sm_cb the op is in and sm_op's flags for ease of use */
  sm_cb = sm_op->sm_cb;
  op_flags = sm_op->flags;
  syll_type = sm_op->syll_type;

  /* Determine the stats to update for this scheduling attempt */
  if (sm_cb->prepass_sched)
    stats = &sm_op->mdes_op->sched_prepass;
  else
    stats = &sm_op->mdes_op->sched_postpass;

  /* Sanity check, make sure operation is not already scheduled! */
  if (sm_op->flags & SM_OP_SCHEDULED)
    {
      L_punt
        ("SM_schedule_oper_schedule: "
	 "%s op %i time %i already scheduled in cycle %i!",
         sm_op->sm_cb->lcode_fn->name, sm_op->lcode_op->id,
	 issue_time, sm_op->sched_cycle);
    }

  /* Perform valid template check */
  /* Recalculate current template bit vectors */
  SM_calculate_template_vector (issue_group_ptr);

  /* Get the current template */
  slots_per_template = SM_get_slots_per_template (sm_cb);
  bundle_number = slot / slots_per_template;
  current_bundle = issue_group_ptr->bundles[bundle_number];
  current_template = current_bundle->template_mask;
  template_shift_var = SM_get_template_shift_var (issue_group_ptr->sm_cb);

  /* 20030831 SZU
   * If template is locked, get syll corresponding to slot.
   * See if sm_op has appropriate syll.
   * Else get the current template, add on op and see if valid template exist.
   */
  if (current_bundle->template_lock)
    {
      locked_syll = (current_template >>
		     ((slots_per_template - (slot % slots_per_template) - 1) *
		      template_shift_var) & (C_pow2 (template_shift_var) - 1));

      /* Continue if only if sm_op's syll matches.
       * sm_op could be something like A type though.
       */
      if (!(syll_type & locked_syll))
	return 0;

      new_template_index = current_bundle->template_index;
      new_template = current_template;
    }
  else
    {
      temp_template = current_template |
	(syll_type << ((slots_per_template - (slot % slots_per_template) - 1) *
		       template_shift_var));

      /* Check if temp_template is valid */
      if ((new_template_index =
	   SM_check_template_validity (sm_cb, temp_template)) == SM_NO_TEMPLATE)
	{
#if DEBUG_PRIORITY
	  printf ("SM_schedule_oper_schedule: "
		  "Valid bundle template not found; fail\n"
		  "cb %i time %i slot %i op %i syll_type %x\n"
		  "current_template %x temp_template %x\n",
		  sm_cb->lcode_cb->id, issue_time, slot, sm_op->lcode_op->id,
		  syll_type, current_template, temp_template);
#endif
	  return 0;
	}

      /* Valid temp_template. Grab the new template. */
      new_template = SM_get_template (sm_cb, new_template_index);
    }

  sched_slot = -1;

  /* Use flags to determine if we should commit a valid schedule
   * when it is found or if we are only testing to see if an op
   * can be scheduled.
   */
  if (sched_flags & SM_TEST_ONLY)
    commit_sched = 0;
  else
    commit_sched = 1;

  /* Find the first compatible alt that has its resources available */
  for (compatible_alt = sm_op->first_compatible_alt; compatible_alt != NULL;
       compatible_alt = compatible_alt->next_compatible_alt)
    {
      /* If operation has variable actions, extra checking is required
       * to determine if dependences are truly satified for alt.
       */
      if (sm_op->flags & SM_OP_VARIABLE_ACTIONS)
        {
          /* If compatible alt is not truly ready, goto next alt */
          if (!SM_alt_ready (sm_op, compatible_alt, issue_time))
            continue;
        }

      /* Schedule the operation if we can with this alternative.
       * For now, the normal version and the silent version of the
       * alternatives have exactly the same requirements, so only
       * need to check the normal version.
       * Can switch between the two versions with no extra checks
       *
       * Use different routines for acyclic and cyclic (modulo)
       * scheduling. -ITI/JCG 8/99
       */
      if ((sm_cb->flags & SM_MODULO_RESOURCES) == 0)
        {
          sched_slot = SM_sched_table (sm_cb, sm_op, new_template_index,
                                       compatible_alt->normal_version->table,
                                       sm_op->options_chosen,
                                       issue_time, slot, slot,
				       commit_sched, stats);
        }
      else
        {
          sched_slot = SM_modulo_sched_table (sm_cb, sm_op, new_template_index,
                                              compatible_alt->normal_version->
                                              table, sm_op->options_chosen,
                                              issue_time, slot, slot,
					      commit_sched, stats);
        }


      /* Stop if could schedule operation */
      if (sched_slot >= 0)
        break;

      /* If we only need to check to resources for the first 
       * alternative to match format and dependence requirements
       * (specified by mdes2 parameter "check_resources_for_only_one_alt"),
       * stop now and flag that we cannot schedule operation this cycle.
       */
      if (sm_cb->flags & SM_CB_TEST_ONLY_ONE_ALT)
        {
          compatible_alt = NULL;
          break;
        }
    }

  /* Return now if could not schedule op */
  if (compatible_alt == NULL)
    return (0);

  /* If not commiting schedule, just return can schedule now */
  if (!commit_sched)
    {
      return (1);
    }

  /* 20030910 SZU
   * SMH reconciliation
   * 20030930 SZU
   * Disable for postpass for now.
   * Depends on L_predicate_demotion after scheduling to fully break soft dep,
   * which has bad behavior after postpass.
   */
  if (sm_cb->prepass_sched)
    SM_fix_soft_dep (sm_op, issue_time, SOFTFIX_COMMIT);

  /* Operation scheduled, set the fields in sm_op to indicate where */
  sm_op->sched_cycle = issue_time;
  sm_op->sched_slot = sched_slot;
  sm_op->flags |= SM_OP_SCHEDULED;
  sm_op->alt_chosen = compatible_alt;

  /* Perform valid template check */
  /* Recalculate current template bit vectors */
  sm_op->issue_group = issue_group_ptr;
  issue_group_ptr->slots[sched_slot] = sm_op;

  SM_calculate_template_vector (issue_group_ptr);

  current_template = issue_group_ptr->bundles[bundle_number]->template_mask;

  /* 20030831 SZU
   * Do not update if template is locked.
   */
  if ((current_template & new_template) != current_template)
    L_punt ("SM_schedule_oper_schedule: "
	    "Final template doesn't match new_template\n");
  else if (!current_bundle->template_lock)
    current_bundle->template_index = new_template_index;

  SM_issue_group_full_check (issue_group_ptr);

  /* Check if issue group needs to be linked
   */
  if ((issue_group_ptr->prev_issue_group == NULL) &&
      (issue_group_ptr->next_issue_group == NULL) &&
      (issue_group_ptr != sm_cb->first_issue_group))
    SM_link_issue_group (issue_group_ptr);

  /* 20021216 SZU
   * Only update this for real ops, not nop
   */
  if (!(SM_is_nop (sm_op)))
    {
      /* Get the latency array for the alternative chosen */
      operand_latency =
	compatible_alt->normal_version->latency->operand_latency;

      /* Update the early and late use times for all register actions */
      for (op_action = sm_op->first_op_action; op_action != NULL;
	   op_action = op_action->next_op_action)
	{
	  /* Until the mdes internal structures are enhanced, the
	   * early and late time use the only time stored (for now).
	   */
	  op_action->actual_early_use_time = operand_latency[op_action->index];
	  op_action->actual_late_use_time = operand_latency[op_action->index];

	  /* 20040516SZU
	   * Fix for Limpact path w/ regards to ADDLAT attr.
	   *
	   * Problem:
	   * Additional latency scheduled but not showing up on isl attribute.
	   *
	   * isl attr uses dest[index]->actual_late_use_time.
	   * ADDLAT uses dest[index]->add_lat, which is added onto dep latency,
	   * but not to actual_late_use_time.
	   * Assumes that op_action->add_lat == 0 if additional latency not
	   * specified. (Should be right.)
	   */
	  if (op_action->add_lat != 0)
	    op_action->actual_late_use_time += op_action->add_lat;
	}

      /* Update the scheduler bounds of dependent operations */
      SM_update_adjacent_bounds (sm_op);

      /* 
       * Place operation in the scheduled list of the cb 
       */

      /* Search from the end of already scheduled ops for the op
       * where this newly scheduled op should be placed after.
       * (This optimizes for scheduling an cb top-down,
       * but works for all cases.)
       */
      for (after_this_op = sm_cb->last_sched_op; after_this_op != NULL;
	   after_this_op = after_this_op->prev_sched_op)
	{
	  /* Stop when hit an already scheduled op that should go before
	   * this operation.
	   */
	  if ((after_this_op->sched_cycle < issue_time) ||
	      ((after_this_op->sched_cycle == issue_time) &&
	       (after_this_op->sched_slot <= sched_slot)))
	    break;
	}

      /* Place sm_op after 'after_this_op' (if not NULL) */
      if (after_this_op != NULL)
	{
	  sm_op->prev_sched_op = after_this_op;
	  sm_op->next_sched_op = after_this_op->next_sched_op;
	  if (after_this_op->next_sched_op != NULL)
	    after_this_op->next_sched_op->prev_sched_op = sm_op;
	  else
	    sm_cb->last_sched_op = sm_op;
	  after_this_op->next_sched_op = sm_op;
	}

      /* Otherwise, place sm_op at begining of scheduled list */
      else
	{
	  sm_op->prev_sched_op = NULL;
	  sm_op->next_sched_op = sm_cb->first_sched_op;
	  if (sm_cb->first_sched_op != NULL)
	    sm_cb->first_sched_op->prev_sched_op = sm_op;
	  else
	    sm_cb->last_sched_op = sm_op;
	  sm_cb->first_sched_op = sm_op;
	}

      /* Update num unscheduled count */
      sm_cb->num_unsched--;

      /* Remove from dep_in_resolved queue, if sm_op is in this queue */
      if (sm_op->dep_in_resolved_qentry != NULL)
	{
	  SM_dequeue_oper (sm_op->dep_in_resolved_qentry);
	  sm_op->dep_in_resolved_qentry = NULL;
	}
    }

  return (1);
}

/* 20020818 SZU
 * Tries to insert the oper into the issue group w/o unscheduling anything.
 * Only attempt if not modulo scheduling;
 * different situations arrise due to unscheduling oper.
 * Only attempt if the current syllable in the slot is compatible.
 * Saves a copy before going through all of the slots; insert NOP and continue.
 * Tries to save time by avoiding unscheduling.
 * Unscheduling can be VERY costly, 197.parser.
 * 20030829 SZU
 * DOES NOT WORK!!! Supporting functions like SM_insert_nop have assumptions
 * that break this. Assumes can/should modify template.
 * 197.parser time problem due to dependences and solved that way.
 * See recalculate_adjacent_bounds.
 */
int
SM_schedule_oper_insert (SM_Oper *sm_op, SM_Issue_Group *issue_group_ptr,
			 int issue_time, unsigned int sched_flags)
{
  SM_Issue_Group *save_issue_group;
  SM_Cb *sm_cb;
  SM_Bundle *sm_bundle;
  SM_Oper *old_sm_op, *oper;
  int num_slots, index, scheduled, current_syll_type, slots_per_template;
  int template_shift_var, current_bundle, try_schedule, current_syll;
  int success, template_per_issue, bundle_number, saved_template;
  int last_orig_op, done;
  unsigned int current_template;

  sm_cb = sm_op->sm_cb;
  num_slots = SM_get_num_slots (sm_cb);
  slots_per_template = SM_get_slots_per_template (sm_cb);
  template_shift_var = SM_get_template_shift_var (sm_cb);
  template_per_issue = SM_get_template_per_issue (sm_cb);

  /* Proceed only if not modulo scheduling */
  if (sm_cb->flags & SM_MODULO_RESOURCES)
    return 0;

  /* Save current issue group */
  save_issue_group = SM_create_issue_group (sm_cb, issue_group_ptr->issue_time);
  save_issue_group->full = issue_group_ptr->full;
  save_issue_group->next_issue_group = issue_group_ptr->next_issue_group;
  save_issue_group->prev_issue_group = issue_group_ptr->prev_issue_group;

  last_orig_op = -1;
  /* Save non NOP instructions and create new NOP instructions. */
  for (index = 0; index < num_slots; index++)
    {
      oper = issue_group_ptr->slots[index];

      if (oper)
	{
	  /* Not NOP */
	  if (!SM_is_nop (oper))
	    {
	      save_issue_group->slots[index] = oper;
	      oper->old_issue_time = oper->sched_cycle;

	      if (last_orig_op < index)
		last_orig_op = index;
	    }
	  /* NOP */
	  else
	    {
	      //SM_unschedule_nop (oper);
	      save_issue_group->slots[index] = oper;
	    }
	}
    }

  /* Save bundle information */
  for (index = 0; index < template_per_issue; index++)
    {
      SM_Bundle *save_bundle, *bundle;

      save_bundle = save_issue_group->bundles[index];
      bundle = issue_group_ptr->bundles[index];

      save_bundle->template_index = bundle->template_index;
      save_bundle->stop = bundle->stop;
      save_bundle->template_mask = bundle->template_mask;
      save_bundle->empty = bundle->empty;
    }

  index = 0;
  scheduled = 0;
  while ((!scheduled) && (index >= 0) && (index < num_slots))
    {
      try_schedule = 0;
      old_sm_op = issue_group_ptr->slots[index];

      /* 200308321 SZU
       * Need to verify issue before inserting!
       */
      bundle_number = index / slots_per_template;
      
      if (bundle_number > 0)
	if (!SM_verify_issue (issue_group_ptr, bundle_number))
	  break;
      
      /* Empty slot. Check current template_mask.
       * Could be assigned but empty if modulo scheduling or NOP.
       */
      if (!old_sm_op)
	{
#if 0
#if 1
	  current_bundle = index / slots_per_template;
	  sm_bundle = issue_group_ptr->bundles[current_bundle];
	  current_template = sm_bundle->template_mask;

	  current_syll_type = 
	    (current_template >>
	     ((slots_per_template - (index % slots_per_template) - 1) *
	      template_shift_var) & (C_pow2(template_shift_var) - 1));

	  /* Try to schedule if current_syll_type compatible w/ sm_op
	   * or was completely empty
	   */
	  if ((current_syll_type == 0) ||
	      (current_syll_type & sm_op->mdes_op->syll_mask))
	    try_schedule = 1;
#else
	  try_schedule = 1;
#endif
#else
	  /* If before last oper, check syll */
	  if (index <= last_orig_op)
	    {
	      current_bundle = index / slots_per_template;
	      sm_bundle = issue_group_ptr->bundles[current_bundle];
	      current_template = sm_bundle->template_mask;

	      current_syll_type = 
		(current_template >>
		 ((slots_per_template - (index % slots_per_template) - 1) *
		  template_shift_var) & (C_pow2(template_shift_var) - 1));

	      /* Try to schedule if current_syll_type compatible w/ sm_op
	       * or was completely empty
	       */
	      if ((current_syll_type == 0) ||
		  (current_syll_type & sm_op->mdes_op->syll_mask))
		{
		  try_schedule = 1;
		}
	      else
		{
		  if (index < num_slots - 1)
		    if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		      break;

		  index++;
		  continue;
		}
	    }
	  /* If after last oper, syll shouldn't matter; overide */
	  else
	    {
	      current_syll_type = 0;
	      try_schedule = 1;
	    }
#endif
	}
      /* Non-empty slot containing NOP */
      /* 20030819 SZU
       * Probably shouldn't ever reach here.
       * NOPs are unscheduled in SM_schedule_oper when creating save_issue_group
       */
      else if (SM_is_nop (old_sm_op))
	{
	  /* If current syllable matches NOP, try schedule */
	  if (old_sm_op->mdes_op->syll_mask & sm_op->mdes_op->syll_mask)
	    {
	      current_syll_type = old_sm_op->mdes_op->syll_mask;
	      try_schedule = 1;
	      SM_unschedule_nop (old_sm_op);
	    }
	  else
	    {
	      index++;
	      continue;
	    }
	}
      /* Non-empty slot containing an operation */
      else
	{
	  index += old_sm_op->mdes_op->num_slots;
	  continue;
	}

      /* If try_schedule, valid slot to try to schedule sm_op in.
       */
      if (try_schedule)
	{
	  /* See if op can be scheduled at this time and slot */
	  if ((sm_op->nosoft_cycle_lower_bound > issue_time) ||
	      (sm_op->nosoft_cycle_upper_bound < issue_time) ||
	      ((sm_op->nosoft_cycle_lower_bound == issue_time) &&
	       (sm_op->nosoft_slot_lower_bound > index)) ||
	      ((sm_op->nosoft_cycle_upper_bound == issue_time) &&
	       (sm_op->nosoft_slot_upper_bound < index)))
	    {
	      /* If not successful, try to put in NOP if empty.
	       * Do not replace current NOP.
	       */
	      if ((!old_sm_op) && (index < num_slots - 1))
		if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		  break;

	      index++;
	      continue;
	    }

	  /* If last branch of the cb
	   * (last_serial_op and unconditional br)
	   * then must be slot[5]
	   */
	  /* 20030725 SZU
	   * Change to must be slot[2] or slot[5]; last slot of bundle.
	   * Somewhat IPF specific, would break if more than 2 bundles.
	   */
	  if ((sm_op == sm_cb->last_serial_op) &&
	      (L_uncond_branch(sm_op->lcode_op)) &&
	      ((index != num_slots - 1) && (index != slots_per_template - 1)))
	    {
	      /* If not successful, try to put in NOP if empty.
	       * Do not replace current NOP.
	       */
	      if ((!old_sm_op) && (index < num_slots - 1))
		if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		  break;

	      index++;
	      continue;
	    }

	  /* Special consideration if loop back branch */
	  if (sm_cb->flags & SM_MODULO_RESOURCES)
	    if ((sm_op->flags & SM_LOOP_BACK_BR) &&
		((index != num_slots -1) && (index != slots_per_template - 1)))
	      {
		/* If not successful, try to put in NOP if empty.
		 * Do not replace current NOP.
		 */
		if ((!old_sm_op) && (index < num_slots - 1))
		  if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		    break;

		index++;
		continue;
	      }

	  /* Pass all restrictions. Try to schedule. */
	  /* If op has multiple syll, then systematically try all the syll. */
	  if ((sm_op->mdes_op->syll_mask % 2 != 0) &&
	      (sm_op->mdes_op->syll_mask != 1))
	    {
	      current_syll = 0;
	      success = 0;

	      /* Haven't tried last syll yet */
	      while ((sm_op->mdes_op->syll_mask - current_syll > current_syll)
		     && !success)
		{
		  /* Get the next syll_mask */
		  if (current_syll == 0)
		    current_syll = 0x1;
		  else
		    current_syll = current_syll << 1;

		  while (!(current_syll & sm_op->mdes_op->syll_mask))
		    current_syll = current_syll << 1;

		  /* Restrict by current_syll_type */
		  if ((current_syll_type != 0) &&
		      (!(current_syll & current_syll_type)))
		    continue;

		  sm_op->syll_type = current_syll;

		  if (SM_schedule_oper_schedule (sm_op, issue_group_ptr, 
						 issue_time, index,
						 sched_flags))
		    success = 1;
		}

	      if (success)
		scheduled = 1;
	    }
	  /* 20030606 SZU
	   * sm_op is not multi-syll.
	   */
	  else
	    {
	      if (SM_schedule_oper_schedule (sm_op, issue_group_ptr, 
					     issue_time, index,
					     sched_flags))
		scheduled = 1;
	    }

	  if (scheduled)
	    {
	      issue_group_ptr->num_slots_left -= sm_op->mdes_op->num_slots;

	      if (old_sm_op)
		{
		  if (SM_is_nop (old_sm_op))
		    SM_delete_nop (old_sm_op);
		  /* Something went wrong, shouldn't schedule if not NOP */
		  else
		    L_punt ("SM_schedule_oper_insert: "
			    "Inserted new sm_op where one existed before.\n"
			    "Function %s CB %i Time %i "
			    "new op %i old op %i slot %i\n",
			    sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
			    issue_time, sm_op->lcode_op->id,
			    old_sm_op->lcode_op->id, index);
		}
	    }
	  else
	    {
	      /* If not successful, try to put in NOP if empty.
	       * Do not replace current NOP.
	       */
	      if ((index < num_slots - 1) && (!old_sm_op))
		if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		  break;
	      
	      index++;
	    }
	}
    }

  /* Restore from save_issue_group if schedule not successful.
   * Opers should never be unscheduled.
   * NOPS may be unscheduled, deleted only if successful.
   * 20030821 SZU
   * Also restore if verify issue fails.
   * IPF specific.
   */
#if 0
  if (!scheduled)
#else
  if ((!scheduled) ||
      ((scheduled) &&
       (!SM_verify_issue (issue_group_ptr, (index / slots_per_template)))))
#endif
    {
      for (index = 0; index < num_slots; index++)
	{
	  bundle_number = index / slots_per_template;

	  /* Save the template at the beginning of a bundle */
	  if (index % slots_per_template == 0)
	    saved_template =
	      save_issue_group->bundles[bundle_number]->template_mask;

	  if ((oper = save_issue_group->slots[index]))
	    {
	      SM_Oper *current_oper;

	      current_oper = issue_group_ptr->slots[index];

	      /* If oper is not NOP, better be scheduled in issue_group_ptr */
	      if (!(SM_is_nop (oper)))
		{
		  if ((!(current_oper->flags & SM_OP_SCHEDULED)) ||
		      (current_oper != oper))
		    {
		      L_punt ("SM_schedule_oper_insert: "
			      "%s cb %i op %i lost its schedule!",
			      sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
			      sm_op->lcode_op->id);
		    }
		}
	      /* If oper is a NOP, check if same and scheduled.
	       * If same, re-schedule if not scheduled.
	       * If not same, something wrong.
	       */
	      else
		{
		  if (current_oper != oper)
		    {
		      L_punt ("SM_schedule_oper_insert: "
			      "%s cb %i NOP in slot %i changed!",
			      sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
			      index);
		    }
		  else if (!(current_oper->flags & SM_OP_SCHEDULED))
		    {
		      done = SM_schedule_oper_schedule (oper, issue_group_ptr,
							issue_time, index,
							sched_flags);
		      if (!done)
			L_punt ("SM_schedule_oper_insert: "
				"%s cb %i NOP in slot %i\n"
				"Error recovering/rescheduling NOP!\n",
				sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
				index);
		    }
		}
	    }
	  /* There wasn't an oper in this slot before, better be inserted NOP.
	   * Delete the new NOP.
	   * 20030822 SZU
	   * If scheduled, but verify issue fail, sm_op could be scheduled here.
	   * Unschedule if current_oper == sm_op. Else punt.
	   */
	  else
	    {
	      SM_Oper *current_oper;

	      current_oper = issue_group_ptr->slots[index];

	      if (current_oper)
		{
		  if (!scheduled)
		    if (!(SM_is_nop (current_oper)))
		      {
			L_punt ("SM_schedule_oper_insert: "
				"%s cb %i non-NOP in slot %i\n"
				"There wasn't an oper here before!\n",
				sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
				index);
		      }
		    else
		      {
			SM_unschedule_nop (current_oper);
			SM_delete_nop (current_oper);
		      }
		  else
		    if (SM_is_nop (current_oper))
		      {
			SM_unschedule_nop (current_oper);
			SM_delete_nop (current_oper);
		      }
		    else if (current_oper == sm_op)
		      {
			SM_unschedule_oper (current_oper, NULL);
		      }
		    else
		      {
			L_punt ("SM_schedule_oper_insert: "
				"%s cb %i non-NOP in slot %i\n"
				"Schedule succeeded bur failed verify issue. "
				"There wasn't an oper here before!\n",
				sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
				index);
		      }
		}
	    }
	}
    }
  return scheduled;
}

/* 20021212 SZU
 * Unschedule all op. Queue all ops by port/resource priority.
 * Schedule left to right.
 * 20030831 SZU
 * Added compaction flag to be used for compaction.
 */
int
SM_schedule_oper_priority (SM_Oper *sm_op, int issue_time, int earliest_slot,
			   int latest_slot, unsigned int sched_flags, 
			   int compact)
{
  SM_Cb *sm_cb;
  unsigned int op_flags;
  Mdes_Stats *stats;
  SM_Issue_Group *issue_group_ptr;
  int port_type, num_slots, index, index1;
  SM_Oper *old_sm_op, *oper = NULL;
  unsigned int current_syll;
  int success, slots_per_template;
  SM_Priority_Queue *priority_queue;
  SM_Priority_Qentry *current_qentry;
  int done, avail_slots;

  sm_cb = sm_op->sm_cb;
  op_flags = sm_op->flags;
  priority_queue = SM_new_priority_queue ();
  num_slots = SM_get_num_slots (sm_cb);
  slots_per_template = SM_get_slots_per_template (sm_cb);
  avail_slots = latest_slot - earliest_slot + 1;

  /* Determine the stats to update for this scheduling attempt */
  stats = sm_cb->prepass_sched ? 
    &sm_op->mdes_op->sched_prepass : &sm_op->mdes_op->sched_postpass;

  /* Sanity check, make sure operation is not already scheduled! */
  if (sm_op->flags & SM_OP_SCHEDULED)
    L_punt ("SM_schedule_oper_priority: "
	    "%s op %i time %i already scheduled in cycle %i!",
	    sm_op->sm_cb->lcode_fn->name, sm_op->lcode_op->id, issue_time,
	    sm_op->sched_cycle);

  /* Check if an issue_group with this issue_time exists */
  issue_group_ptr = SM_check_for_issue_group (sm_cb, issue_time);

  sm_op->syll_type = sm_op->mdes_op->syll_mask;
  port_type = sm_op->mdes_op->port_mask;

  /* If there doesn't exist an issue_group with this issue_time, make one */
  if (!issue_group_ptr)
    {
      /* 20030831 SZU
       * If called during compaction, there MUST be an existing issue.
       */
      if (compact)
	L_punt ("SM_schedule_oper_priority:\n"
		"compact true, no issue group found. SOMETHING WRONG!!\n"
		"function %s cb %i time %i\n",
		sm_cb->lcode_fn->name, sm_cb->lcode_cb->id, issue_time);

      issue_group_ptr = SM_create_issue_group (sm_cb, issue_time);
      SM_link_issue_group (issue_group_ptr);
    }
  else
    {
      /* 20030818 SZU
       * Check if sm_op can be inserted, to avoid unscheduling.
       * DO NOT USE FOR NOW!! Insert is broken.
       */
#if 0
      success = SM_schedule_oper_insert (sm_op, issue_group_ptr, issue_time,
					 sched_flags);

      if (success)
	return 1;
#endif

      /* Prepare existing issue group.
       * Delete all NOP, unschedule all existing op and add to priority queue
       */
      for (index = 0; index < num_slots; index++)
	{
	  if (!(old_sm_op = issue_group_ptr->slots[index]))
	    continue;

	  /* If old_sm_op is not NOP, unschedule and add to queue */
	  if (!(SM_is_nop (old_sm_op)))
	    {
	      if (old_sm_op->flags & SM_OP_SCHEDULED)
		{
		  old_sm_op->old_issue_time = old_sm_op->sched_cycle;
		  SM_unschedule_oper (old_sm_op, NULL);
		}
	      else
		{
		  issue_group_ptr->slots[index] = NULL;
		}

	      SM_enqueue_increasing_priority (priority_queue, old_sm_op, 
					      old_sm_op->mdes_op->port_mask);
	    }
	  /* If NOP, unschedule and delete */
	  else
	    {
	      SM_unschedule_nop (old_sm_op);
	      SM_delete_nop (old_sm_op);
	    }
	}
    }

  /* Add sm_op to priority queue */
  SM_enqueue_increasing_priority (priority_queue, sm_op, port_type);

  /* 20030605 SZU
   * Reset template vectors and template_index
   */
  for (index = 0; index < sm_cb->sm_mdes->max_template_per_issue; index++)
    {
      /* 20030831 SZU
       * Merging w/ compaction version. Check template_lock.
       */
      if (!issue_group_ptr->bundles[index]->template_lock)
	{
	  issue_group_ptr->bundles[index]->template_mask = 0;
	  issue_group_ptr->bundles[index]->template_index = SM_NO_TEMPLATE;
	  issue_group_ptr->bundles[index]->stop = -1;
	}
    }

  /* 20030831 SZU
   * Merging w/ version for compaction. Variable bounds.
   */
  index = earliest_slot;
  current_qentry = NULL;
  /* 20021217 SZU
   * Change to <= num_slots, if slot 4 takes L_SYLL index += 2
   * 20030831 SZU
   * Merging w/ version for compaction. Variable bounds.
   * 20031124 SZU
   * Fixing NOP.i problem.
   * Currently, like HaMM, assumes all remaining slots can be safely
   * filled w/ NOP instructions.
   * NOT TRUE FOR I-TYPE INSTRUCTIONS!!!
   */
  while ((index >= earliest_slot) && 
	 ((priority_queue->num_not_sched && (index <= (latest_slot + 1))) ||
	  (!priority_queue->num_not_sched && (index <= latest_slot))))
    {
      /* 20030821 SZU
       * Attempting to implement automatic backtrack if an op fail due
       * to cycle/slot bound restrictions.
       * If an op fails this way, usually something scheduled restricting it.
       * Therefore go back and unschedule it first.
       */
      int backtrack = 0;

      /* 20031124 SZU
       * If there are no operations left in the queue,
       * finish off the rest with NOP to check for NOP.i cases.
       */
      if (!priority_queue->num_not_sched && (index <= latest_slot))
	{
	  if ((old_sm_op = issue_group_ptr->slots[index]))
	    {
	      /* If old_sm_op is NOP, SM_insert_NOP to insert different NOP */
	      if (SM_is_nop (old_sm_op))
		{
		  /* Exhausted nop options. Backtrack. */
		  if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		    if ((old_sm_op = issue_group_ptr->slots[index]))
		      {
			SM_unschedule_nop (old_sm_op);
			SM_delete_nop (old_sm_op);
			index--;
			continue;
		      }
		  
		  /* Received new nop, continue */
		  index++;
		}
	      /* old_sm_op isn't NOP. Unschedule. */
	      else
		{
		  old_sm_op->old_issue_time = old_sm_op->sched_cycle;
		  SM_unschedule_oper (old_sm_op, priority_queue);

		  old_sm_op->qentry->scheduled = 0;
		  priority_queue->num_not_sched++;

		  /* 20030605 SZU
		   * Need to determine via mdes_op->syll if op has
		   * multiple possible syllables,
		   * then systematically try all the syllables.
		   */
		  /* Determine if op has multiple syllable types */
		  if ((old_sm_op->mdes_op->syll_mask % 2 != 0) &&
		      (old_sm_op->mdes_op->syll_mask != 1))
		    {
		      current_syll = old_sm_op->syll_type;
		      success = 0;

		      /* Haven't tried last syll yet */
		      while ((old_sm_op->mdes_op->syll_mask - current_syll >
			      current_syll) && !success)
			{
			  /* Get the next syll_mask */
			  current_syll = current_syll << 1;
			  while (!(current_syll &
				   old_sm_op->mdes_op->syll_mask))
			    current_syll = current_syll << 1;

			  old_sm_op->syll_type = current_syll;

			  /* Recalculate slot restrictions */
			  SM_recalculate_lower_bound (old_sm_op);
			  SM_recalculate_upper_bound (old_sm_op);

			  /* See if op can still be scheduled
			   * at this time and slot
			   * 20030930 SZU
			   * Disable breaking soft dep in postpass for now.
			   * Depends on L_predicate_demotion after scheduling to
			   * fully break soft dep, which has bad behavior after
			   * postpass.
			   */
			  if ((old_sm_op->nosoft_cycle_lower_bound <
				 old_sm_op->old_issue_time) ||
				(old_sm_op->nosoft_cycle_upper_bound >
				 old_sm_op->old_issue_time) ||
				((old_sm_op->nosoft_cycle_lower_bound ==
				  old_sm_op->old_issue_time) &&
				 (old_sm_op->nosoft_slot_lower_bound <=
				  index)) ||
				((old_sm_op->nosoft_cycle_upper_bound ==
				  old_sm_op->old_issue_time) &&
				 (old_sm_op->nosoft_slot_upper_bound >= index)))
			    if (SM_schedule_oper_schedule (old_sm_op,
							   issue_group_ptr,
							   old_sm_op->
							   old_issue_time,
							   index, sched_flags))
			      {
				success = 1;
			      }
			}

		      if (success)
			{
			  old_sm_op->qentry->scheduled = 1;
			  priority_queue->num_not_sched--;
			  index += old_sm_op->mdes_op->num_slots;

			  /* 20030715 SZU
			   * Update number of slots left
			   */
			  issue_group_ptr->num_slots_left -=
			    old_sm_op->mdes_op->num_slots;
			  
			  continue;
			}
		    }
		  
		  /* 20030606 SZU
		   * Either not multiple compatible syllables,
		   * or list exhausted.
		   * 20031124 SZU
		   * Should be the last or rightmost operation since
		   * num_not_sched == 0.
		   * Backtrack.
		   */
		  index--;
		}
	    }
	  /* Nothing in slots[index], so try inserting NOP. */
	  else
	    {
	      if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		if ((old_sm_op = issue_group_ptr->slots[index]))
		  {
		    SM_unschedule_nop (old_sm_op);
		    SM_delete_nop (old_sm_op);
		    index--;
		    continue;
		  }
	      
	      /* Received new nop, continue */
	      index++;
	    }
	  continue;
	}

      /* Number of op left in queue not scheduled greater than number of 
       * slots left. Backtrack.
       * 20030831 SZU
       * Merging w/ version for compaction. Variable bounds.
       */
      if (priority_queue->num_not_sched >
	  (avail_slots - (index - earliest_slot)))
	{
	  index--;
	  continue;
	}

      /* Check if previous slot is L_SYLL. If so, can't schedule here.
       * Always backtrack because index += 2 when L_SYLL scheduled
       * 20030604 SZU
       * Alter to mdes format. Check how many slots previous operation takes.
       * Always backtrack because index += slots of operation when scheduled.
       * 20030831 SZU
       * Merged w/ compaction version. Variable bounds.
       */
      if (index != earliest_slot)
	{
	  index1 = index - 1;
	  while (index1 >= earliest_slot)
	    {
	      if (issue_group_ptr->slots[index1])
		{
		  if ((issue_group_ptr->slots[index1]->mdes_op->num_slots +
		       index1) <= index)
		    {
		      /* First previous op in slot indicates index is ok */
		      break;
		    }
		  else
		    {
		      /* index in range of previous op. Backup to it */
		      index = index1;
		      break;
		    }
		}
	      index1--;
	    }

	  /* Something went wrong. index wasn't incremented properly */
	  if (index1 < earliest_slot)
	    L_punt("SM_schedule_oper_priority: "
		   "index not incremented properly\n"
		   "function %s cb %i op %i time %i\n",
		   sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
		   sm_op->lcode_op->id, issue_time);
	}

      /* 20030603 SZU
       * Implement split issue by looking at Issue_Group section of mdes
       */
 
      /* Check only if index isn't in first bundle */
      if (index >= slots_per_template &&
	  !SM_verify_issue (issue_group_ptr, index / slots_per_template))
	{
#if DEBUG_PRIORITY
	  printf ("SM_schedule_oper_priority: "
		  "cb %i time %i op %i slot %i failed\n"
		  "Due to no issue_group\n",
		  sm_cb->lcode_cb->id, issue_time,
		  sm_op->lcode_op->id, index);
#endif
	  /* Shouldn't have anything scheduled here, ever.
	   * 20030722 SZU
	   * Not true. Consider case of schedule L in 2nd bundle,
	   * w/ instruction left to schedule.
	   * Backtracks because index == 6, and finds L before
	   * verifying issue.
	   * Instead just unschedule and continue backtracking.
	   * IPF specific.
	   * 
	   * For more general, should try following qentries.
	   */
	  if ((old_sm_op = issue_group_ptr->slots[index]))
	    {
	      if (SM_is_nop (old_sm_op))
		{
		  SM_unschedule_nop (old_sm_op);
		  SM_delete_nop (old_sm_op);
		}
	      else
		{
		  old_sm_op->old_issue_time = old_sm_op->sched_cycle;
		  SM_unschedule_oper (old_sm_op, priority_queue);
		  
		  old_sm_op->qentry->scheduled = 0;
		  priority_queue->num_not_sched++;
		}
	    }

	  index--;
	  continue;
	}
      
      /* Something in slots[index]. This means backtracking */
      if ((old_sm_op = issue_group_ptr->slots[index]))
	{
	  /* If old_sm_op is NOP, try SM_insert_NOP to insert different NOP */
	  if (SM_is_nop (old_sm_op))
	    {
	      /* Exhausted nop options. Backtrack. */
	      if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
		if ((old_sm_op = issue_group_ptr->slots[index]))
		  {
		    SM_unschedule_nop (old_sm_op);
		    SM_delete_nop (old_sm_op);
		    index--;
		    continue;
		  }
	      
	      /* Received new nop, continue */
	      index++;
	      continue;
	    }
	  /* old_sm_op isn't NOP. Unschedule. */
	  else
	    {
	      old_sm_op->old_issue_time = old_sm_op->sched_cycle;
	      SM_unschedule_oper (old_sm_op, priority_queue);

	      old_sm_op->qentry->scheduled = 0;
	      priority_queue->num_not_sched++;

	      /* If A type op, check if M or I syll used.
	       * If M, try I. If I, unschedule.
	       * M always tried before I.
	       */
	      /* 20030605 SZU
	       * Need to determine via mdes_op->syll that op has multiple syll.
	       * Then systematically try all the syll.
	       */
	      /* Determine if op has multiple syllable types */
	      if ((old_sm_op->mdes_op->syll_mask % 2 != 0) &&
		  (old_sm_op->mdes_op->syll_mask != 1))
		{
		  current_syll = old_sm_op->syll_type;
		  success = 0;

		  /* Haven't tried last syll yet */
		  while ((old_sm_op->mdes_op->syll_mask - current_syll >
			  current_syll) && !success)
		    {
		      /* Get the next syll_mask */
		      current_syll = current_syll << 1;
		      while (!(current_syll & old_sm_op->mdes_op->syll_mask))
			current_syll = current_syll << 1;

		      old_sm_op->syll_type = current_syll;

		      /* Recalculate slot restrictions */
		      SM_recalculate_lower_bound (old_sm_op);
		      SM_recalculate_upper_bound (old_sm_op);

		      /* 20030302 SZU
		       * SMH reconciliation
		       * SMH integrates soft dependencies,
		       * need to use the hard boundaries
		       */
		      /* See if op can still be scheduled
		       * at this time and slot
		       * 20030930 SZU
		       * Disable breaking soft dep in postpass for now.
		       * Depends on L_predicate_demotion after scheduling to
		       * fully break soft dep, which has bad behavior after
		       * postpass.
		       */
		      if ((old_sm_op->nosoft_cycle_lower_bound <
			     old_sm_op->old_issue_time) ||
			    (old_sm_op->nosoft_cycle_upper_bound >
			     old_sm_op->old_issue_time) ||
			    ((old_sm_op->nosoft_cycle_lower_bound ==
			      old_sm_op->old_issue_time) &&
			     (old_sm_op->nosoft_slot_lower_bound <= index)) ||
			    ((old_sm_op->nosoft_cycle_upper_bound ==
			      old_sm_op->old_issue_time) &&
			     (old_sm_op->nosoft_slot_upper_bound >= index)))
			if (SM_schedule_oper_schedule (old_sm_op,
						       issue_group_ptr,
						       old_sm_op->
						       old_issue_time,
						       index, sched_flags))
			  {
			    success = 1;
			  }
		    }

		  if (success)
		    {
		      old_sm_op->qentry->scheduled = 1;
		      priority_queue->num_not_sched--;
		      index += old_sm_op->mdes_op->num_slots;

		      /* 20030715 SZU
		       * Update number of slots left
		       */
		      issue_group_ptr->num_slots_left -=
			old_sm_op->mdes_op->num_slots;

		      continue;
		    }
		}
	      
	      /* Either not A_PORT w/ M_SYLL, or I_SYLL schedule unsuccessful */
	      /* 20030606 SZU
	       * Either not multiple compatible syllables, or list exhausted.
	       */
	      current_qentry = old_sm_op->qentry->next_qentry;
	    }
	}
      /* Nothing in slots[index], not backtracking. Take first qentry. */
      else
	{
	  current_qentry = priority_queue->first_qentry;
	}

      done = 0;
      for (; (current_qentry != NULL) && !done && !backtrack;
	   current_qentry = current_qentry->next_qentry)
	{
	  /* Qentry scheduled, move on */
	  if (current_qentry->scheduled)
	    continue;

	  oper = current_qentry->oper;

	  /* Recalculate slot restrictions */
	  SM_recalculate_lower_bound (oper);
	  SM_recalculate_upper_bound (oper);

	  /* 20030302 SZU
	   * SMH reconciliation
	   * SMH integrates soft dependencies, need to use the hard boundaries
	   */
	  /* See if op can still be scheduled at this time and slot */
	  /* 20030930 SZU
	   * Disable breaking soft dep in postpass for now.
	   * Depends on L_predicate_demotion after scheduling to
	   * fully break soft dep, which has bad behavior after
	   * postpass.
	   */
	  if (oper == sm_op)
	    {
	      if ((oper->nosoft_cycle_lower_bound > issue_time) ||
		    (oper->nosoft_cycle_upper_bound < issue_time) ||
		    ((oper->nosoft_cycle_lower_bound == issue_time) &&
		     (oper->nosoft_slot_lower_bound > index)) ||
		    ((oper->nosoft_cycle_upper_bound == issue_time) &&
		     (oper->nosoft_slot_upper_bound < index)))
		{
		  backtrack = 1;
		  continue;
		}
	    }
	  /* Previously scheduled op. Grab previous issue_time. */
	  else
	    {
	      if ((oper->nosoft_cycle_lower_bound > oper->old_issue_time) ||
		  (oper->nosoft_cycle_upper_bound < oper->old_issue_time) ||
		  ((oper->nosoft_cycle_lower_bound == oper->old_issue_time) &&
		   (oper->nosoft_slot_lower_bound > index)) ||
		  ((oper->nosoft_cycle_upper_bound == oper->old_issue_time) &&
		   (oper->nosoft_slot_upper_bound < index)))
		{
		  backtrack = 1;
		  continue;
		}
	    }

	  /* Extra branch slot restrictions. If not on last slot, try next op
	   * If last branch of the cb
	   * (last_serial_op and unconditional br) then must be slot[5]
	   */
	  /* 20030725 SZU
	   * Change to must be slot[2] or slot[5]; last slot of bundle.
	   * Somewhat IPF specific, would break if more than 2 bundles.
	   */
	  if ((oper == sm_cb->last_serial_op) &&
	      (L_uncond_branch(oper->lcode_op)) &&
	      ((index != num_slots - 1) && (index != slots_per_template - 1)))
	    continue;

	  /* Special consideration if loop back branch */
	  if (sm_cb->flags & SM_MODULO_RESOURCES)
	    if ((oper->flags & SM_LOOP_BACK_BR) &&
		((index != num_slots -1) && (index != slots_per_template - 1)))
	      continue;
#ifdef SM_BRANCHES_AT_END
	  /* 20040302SZU
	   * Try to delay all branches until the end of the bundle.
	   * If oper is a B syll, delay as much as possible.
	   * Delay so that there will be space left for remaining oper
	   * in the queue by inserting NOP for now.
	   * IPF specific hardcoded B syll mask used.
	   *
	   * This is to avoid dead code at end of CB and using BBB template.
	   */
	  if (oper->mdes_op->syll_mask == 0x4)
	    {
	      int current_bundle, last_slot_of_bundle;

	      /* Get the last slot of the current bundle */
	      current_bundle = index / slots_per_template;
	      last_slot_of_bundle =
		(current_bundle + 1) * slots_per_template - 1;

	      /* Adjust last slot to latest_slot */
	      if (last_slot_of_bundle > latest_slot)
		last_slot_of_bundle = latest_slot;

	      if (priority_queue->num_not_sched <=
		  (last_slot_of_bundle - index))
		continue;
	    }
#endif

	  /* Pass all restrictions. Try to schedule. */
	  /* For A type op, try M first, then I */
	  /* 20030605 SZU
	   * If op has multiple syll, then systematically try all the syll.
	   */
	  if ((oper->mdes_op->syll_mask % 2 != 0) &&
	      (oper->mdes_op->syll_mask != 1))
	    {
	      current_syll = 0;
	      success = 0;

	      /* Haven't tried last syll yet */
	      while ((oper->mdes_op->syll_mask - current_syll > current_syll)
		     && !success)
		{
		  /* Get the next syll_mask */
		  if (current_syll == 0)
		    current_syll = 0x1;
		  else
		    current_syll = current_syll << 1;

		  while (!(current_syll & oper->mdes_op->syll_mask))
		    current_syll = current_syll << 1;

		  oper->syll_type = current_syll;

		  if (oper == sm_op)
		    {
		      if (SM_schedule_oper_schedule (oper, issue_group_ptr, 
						     issue_time, index,
						     sched_flags))
			{
			  success = 1;
			}
		    }
		  /* Previously scheduled op. Use old_issue_time. */
		  else
		    {
		      if (SM_schedule_oper_schedule (oper, issue_group_ptr, 
						     oper->old_issue_time,
						     index, sched_flags))
			{
			  success = 1;
			}
		    }
		}

	      if (success)
		{
		  current_qentry->scheduled = 1;
		  priority_queue->num_not_sched--;
		  done = 1;
		}
	    }
	  /* oper is not A type */
	  /* 20030606 SZU
	   * oper is not multi-syll.
	   */
	  else
	    {
	      if (oper == sm_op)
		{
		  if (SM_schedule_oper_schedule (oper, issue_group_ptr, 
						 issue_time, index,
						 sched_flags))
		    {
		      current_qentry->scheduled = 1;
		      priority_queue->num_not_sched--;
		      done = 1;
		    }
		}
	      else
		{
		  if (SM_schedule_oper_schedule (oper, issue_group_ptr, 
						 oper->old_issue_time, index,
						 sched_flags))
		    {
		      current_qentry->scheduled = 1;
		      priority_queue->num_not_sched--;
		      done = 1;
		    }
		}
	    }
	}

      /* 20030821 SZU
       * Implement auto-backtrack if op has bounds problems.
       */
      if (backtrack)
	{
	  index--;
	  continue;
	}

      /* Successfully scheduled an op, move to next index */
      /* 20030605 SZU
       * Increment index according to num_slot of operation.
       */
      if (done)
	{
	  index += oper->mdes_op->num_slots;
	  issue_group_ptr->num_slots_left -= oper->mdes_op->num_slots;
	}      
      else if (index < num_slots - 1)
	{
	  /* Didn't schedule op, insert NOP if not last slot */	  
	  /* Exhausted nop options. Backtrack. */
	  if (!SM_insert_nop (issue_group_ptr, index, sched_flags))
	    {
	      if ((old_sm_op = issue_group_ptr->slots[index]))
		{
		  SM_unschedule_nop (old_sm_op);
		  SM_delete_nop (old_sm_op);
		}

	      index--;
	      continue;
	    }
	  
	  /* Received new nop, continue */
	  index++;
	}
      /* Failed on last slot, backtrack; do not insert NOP */
      else
	{
	  index--;
	}
    }

  /* If index < 0, couldn't find valid scheduling
   * 20030831 SZU
   * Merging w/ version for compaction. Variable bounds.
   */
  if (index < earliest_slot)
    {
      SM_delete_priority_queue (priority_queue);
      return 0;
    }
  
  /* If index > NUM_SLOT_PER_ISSUE, or priority_queue->num_not_sched != 0
   * something went wrong
   * */
  if ((index > num_slots) || (priority_queue->num_not_sched != 0))
    L_punt ("SM_schedule_oper_priority: "
	    "index incremented beyond num_slots, approp range.\n");

  /* Otherwise must've exited w/ queue->num_not_scheduled == 0,
   * or all op successfully scheduled
   */
  SM_delete_priority_queue (priority_queue);
  return 1;
}

/* 20021211 SZU
 * High level controller of schedule oper.
 * 20030606 SZU
 * Alterations according to mdes format.
 */
int
SM_schedule_oper_template_bundling (SM_Oper * sm_op, int issue_time,
				    int earliest_slot, int latest_slot,
				    unsigned int sched_flags)
{
  SM_Cb *sm_cb;
  SM_Issue_Group *issue_group_ptr;
  SM_Issue_Group *save_issue_group = NULL;
  int success, index, num_slots;
  SM_Oper *oper;
  int *port_array;
  SM_Oper *old_sm_op;

  num_slots = SM_get_num_slots (sm_op->sm_cb);
  sm_cb = sm_op->sm_cb;

  /* Save current issue group if there is one */
  issue_group_ptr = SM_check_for_issue_group (sm_op->sm_cb, issue_time);
  if (issue_group_ptr)
    {
      if (issue_group_ptr->full)
	{
	  return 0;
	}
      else
	{
	  /* 20030820 SZU
	   * Move port/restrict check from SM_schedule_oper_priority here.
	   * If violation found, shouldn't create save and try to begin w/.
	   * Should save unschedules.
	   */
	  port_array = SM_init_port_array (sm_cb);

	  /* Check if existing op + sm_op exceed resrc constraints */
	  for (index = 0; index < num_slots; index++)
	    {
	      if ((old_sm_op = issue_group_ptr->slots[index]))
		/* 20031204 SZU
		 * Do not add if old_sm_op is a NOP.
		 */
		if (!SM_is_nop(old_sm_op))
		  /* Assumes that ports are all powers of 2. Should be right */
		  port_array[C_log2(old_sm_op->mdes_op->port_mask)]++;
	    }

	  port_array[C_log2(sm_op->mdes_op->port_mask)]++;

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

	  save_issue_group = SM_create_issue_group (sm_op->sm_cb, issue_time);

	  SM_copy_issue (save_issue_group, issue_group_ptr);
	}
    }

  success =
    SM_schedule_oper_priority (sm_op, issue_time, 0, num_slots - 1,
			       sched_flags, 0);

  /* Unsuccessful. Restore. */
  if (!success && issue_group_ptr)
    SM_restore_issue_group (issue_group_ptr, save_issue_group);

  /* Free save_issue_group if necessary */
  if (save_issue_group)
    {
      /* Free NOP instructions that were created, only if success
       * NOP used to schedule if !success
       */
      for (index = 0; index < num_slots && success; index++)
	if (save_issue_group->slots[index])
	  if (SM_is_nop (save_issue_group->slots[index]))
	    SM_delete_nop (save_issue_group->slots[index]);

      free (save_issue_group->slots);
      L_free (SM_Issue_Group_pool, save_issue_group);
    }
  
  /* 20030824 SZU
   * Added to recalculate the adjacent bounds of all of the ops in issue.
   * Needed because SM_unschedule_oper doesn't recalculate all of
   * the bounds when shuffling anymore to save time.
   */
  if (issue_group_ptr)
    for (index = 0; index < num_slots; index++)
      {
	if ((oper = issue_group_ptr->slots[index]))
	  {
	    SM_recalculate_upper_bound (oper);
	    SM_recalculate_lower_bound (oper);
	  }
      }

  if (!success)
    {
      SM_recalculate_upper_bound (sm_op);
      SM_recalculate_lower_bound (sm_op);
    } 

  return success;
}

/* Determines if an operation can be scheduled at time 'issue_time',
 * in a slot >= 'earliest_slot' and <= 'latest_slot'.
 * 
 * The behaivor of this function can be modified with the folowing flags:
 * 1) SM_TEST_ONLY - Only tests to see if sm_op can be scheduled, doesn't
 *                   actually schedule op.  If flag is not present,
 *                   the operation is actually scheduled (resources used).
 * 
 * Returns 1 if the operation can be scheduled, 0 otherwise.
 */
int
SM_schedule_oper_no_bundling (SM_Oper * sm_op, int issue_time,
			      int earliest_slot, int latest_slot,
			      unsigned int sched_flags)
{
  SM_Cb *sm_cb = NULL;
  SM_Oper *after_this_op = NULL;
  SM_Reg_Action *op_action = NULL;
  SM_Compatible_Alt *compatible_alt = NULL;
  SM_Dep *dep_in = NULL;
  Mdes_Stats *stats = NULL;
  int cycle_lower_bound, cycle_upper_bound;
  unsigned short slot_lower_bound, slot_upper_bound;
  unsigned int op_flags;
  int sched_slot, commit_sched;
  int *operand_latency = NULL;


  /* The the sm_cb the op is in and sm_op's flags for ease of use */
  sm_cb = sm_op->sm_cb;
  op_flags = sm_op->flags;

  /* Determine the stats to update for this scheduling attempt */
  if (sm_cb->prepass_sched)
    stats = &sm_op->mdes_op->sched_prepass;
  else
    stats = &sm_op->mdes_op->sched_postpass;

#if 0
  /* Debug */
  if (sm_op->lcode_op->id == 57)
    {
      fprintf (stderr, "Op %i time %i\n", sm_op->lcode_op->id, issue_time);
    }
#endif

  /* Sanity check, make sure operation is not already scheduled! */
  if (sm_op->flags & SM_OP_SCHEDULED)
    {
      L_punt
        ("SM_schedule_oper: %s op %i time %i already scheduled in cycle %i!",
         sm_op->sm_cb->lcode_fn->name, sm_op->lcode_op->id, issue_time,
         sm_op->sched_cycle);
    }

  /* 20030302 SZU
   * SMH reconciliation
   */
  /* Get the current lower and upper bounds 
   * - Use the bounds that exclude soft deps 
   */
  cycle_lower_bound = sm_op->nosoft_cycle_lower_bound;
  slot_lower_bound = sm_op->nosoft_slot_lower_bound;
  cycle_upper_bound = sm_op->nosoft_cycle_upper_bound;
  slot_upper_bound = sm_op->nosoft_slot_upper_bound;
#if 0
  /* Get the current lower and upper bounds */
  cycle_lower_bound = sm_op->cycle_lower_bound;
  slot_lower_bound = sm_op->slot_lower_bound;
  cycle_upper_bound = sm_op->cycle_upper_bound;
  slot_upper_bound = sm_op->slot_upper_bound;
#endif

#if 0
  /* For debugging purposes, recalculate bounds for this op
   * and compare to what we had before.
   */
  SM_recalculate_lower_bound (sm_op);
  SM_recalculate_upper_bound (sm_op);

  if ((cycle_lower_bound != sm_op->cycle_lower_bound) ||
      (slot_lower_bound != sm_op->slot_lower_bound) ||
      (cycle_upper_bound != sm_op->cycle_upper_bound) ||
      (slot_upper_bound != sm_op->slot_upper_bound))
    {
      fprintf (stderr, "%s cb %i op %i:\n",
               sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
               sm_op->lcode_op->id);
      fprintf (stderr, "  Automatic: lower %i/%i  upper %i/%i\n",
               cycle_lower_bound, slot_lower_bound,
               cycle_upper_bound, slot_upper_bound);
      fprintf (stderr, "     Recalc: lower %i/%i  upper %i/%i\n",
               sm_op->cycle_lower_bound, sm_op->slot_lower_bound,
               sm_op->cycle_upper_bound, sm_op->slot_upper_bound);

      fprintf (stderr, "\n");
    }
#endif


  /* For now, check upper and lower bounds to make sure can be scheduled */
  if (issue_time < cycle_lower_bound)
    return (0);
  if (issue_time > cycle_upper_bound)
    return (0);

  /* Adjust slot bounds if right at bound */
  if ((issue_time == cycle_lower_bound) && (earliest_slot < slot_lower_bound))
    earliest_slot = slot_lower_bound;

  if ((issue_time == cycle_upper_bound) && (latest_slot > slot_upper_bound))
    latest_slot = slot_upper_bound;

  /* Make sure that slot bounds are still possible to meet */
  if (earliest_slot > latest_slot)
    return (0);

  /* If doing modulo-resources, make sure self dependences doesn't
   * prevent this operation from being scheduled (due to II being
   * set too low).
   * 
   * Strickly speaking, the modulo scheduler should never let II be
   * set too low and this should not be an issue.  However, I want
   * SM to be as bullet-proof as possible so I am doing this check for
   * completeness (and my simple test schedulers sometimes sets II too low
   * and it bugs me that SM will schedule the operation anyway (without
   * this check)). -ITI/JCG 8/99 
   */
  if (sm_cb->flags & SM_MODULO_RESOURCES)
    {
      /* Scan all the deps for all the actions */
      for (op_action = sm_op->first_op_action; op_action != NULL;
           op_action = op_action->next_op_action)
        {
          /* Scan the dep in constraints for II violations */
          for (dep_in = op_action->first_dep_in; dep_in != NULL;
               dep_in = dep_in->next_dep_in)
            {
              /* Ignore deps where omega is 0 */
              if (dep_in->omega == 0)
                continue;

              /* Ignore deps marked with SM_IGNORE_DEP */
              if (dep_in->ignore)
                continue;

              /* Ignore deps that are not self-dependences */
              if (dep_in->from_action->sm_op != sm_op)
                continue;

              /* If II is set too low (so this operation can never
               * be scheduled due to self-dependence), return 0
               * (cannot schedule) now.
               */
              if ((dep_in->min_delay - (dep_in->omega * sm_cb->II)) > 0)
                return (0);
            }
        }
    }

  sched_slot = -1;

  /* Use flags to determine if we should commit a valid schedule
   * when it is found or if we are only testing to see if an op
   * can be scheduled.
   */
  if (sched_flags & SM_TEST_ONLY)
    commit_sched = 0;
  else
    commit_sched = 1;

  /* Find the first compatible alt that has its resources available */
  for (compatible_alt = sm_op->first_compatible_alt; compatible_alt != NULL;
       compatible_alt = compatible_alt->next_compatible_alt)
    {
      /* If operation has variable actions, extra checking is required
       * to determine if dependences are truly satified for alt.
       */
      if (sm_op->flags & SM_OP_VARIABLE_ACTIONS)
        {
          /* If compatible alt is not truly ready, goto next alt */
          if (!SM_alt_ready (sm_op, compatible_alt, issue_time))
            continue;
        }

      /* Schedule the operation if we can with this alternative.
       * For now, the normal version and the silent version of the
       * alternatives have exactly the same requirements, so only
       * need to check the normal version.
       * Can switch between the two versions with no extra checks
       *
       * Use different routines for acyclic and cyclic (modulo)
       * scheduling. -ITI/JCG 8/99
       */
      if ((sm_cb->flags & SM_MODULO_RESOURCES) == 0)
        {
          sched_slot = SM_sched_table (sm_cb, sm_op, SM_NO_TEMPLATE,
                                       compatible_alt->normal_version->table,
                                       sm_op->options_chosen,
                                       issue_time, earliest_slot,
                                       latest_slot, commit_sched, stats);
#if 0
          sched_slot = SM_sched_table (sm_cb,
                                       compatible_alt->normal_version->table,
                                       sm_op->options_chosen,
                                       issue_time, earliest_slot,
                                       latest_slot, commit_sched, stats);
#endif
        }
      else
        {
          sched_slot = SM_modulo_sched_table (sm_cb, sm_op, SM_NO_TEMPLATE,
                                              compatible_alt->normal_version->
                                              table, sm_op->options_chosen,
                                              issue_time, earliest_slot,
                                              latest_slot, commit_sched,
                                              stats);
#if 0
          sched_slot = SM_modulo_sched_table (sm_cb,
                                              compatible_alt->normal_version->
                                              table, sm_op->options_chosen,
                                              issue_time, earliest_slot,
                                              latest_slot, commit_sched,
                                              stats);
#endif
        }


      /* Stop if could schedule operation */
      if (sched_slot >= 0)
        break;

      /* If we only need to check to resources for the first 
       * alternative to match format and dependence requirements
       * (specified by mdes2 parameter "check_resources_for_only_one_alt"),
       * stop now and flag that we cannot schedule operation this cycle.
       */
      if (sm_cb->flags & SM_CB_TEST_ONLY_ONE_ALT)
        {
          compatible_alt = NULL;
          break;
        }
    }

  /* Return now if could not schedule op */
  if (compatible_alt == NULL)
    return (0);

  /* If not commiting schedule, just return can schedule now */
  if (!commit_sched)
    {
      return (1);
    }

  /* 20030302 SZU
   * SMH reconciliation
   */
  SM_fix_soft_dep (sm_op, issue_time, SOFTFIX_COMMIT);

  /* Operation scheduled, set the fields in sm_op to indicate where */
  sm_op->sched_cycle = issue_time;
  sm_op->sched_slot = sched_slot;
  sm_op->flags |= SM_OP_SCHEDULED;
  sm_op->alt_chosen = compatible_alt;

  /* Get the latency array for the alternative chosen */
  operand_latency = compatible_alt->normal_version->latency->operand_latency;

  /* Update the early and late use times for all register actions */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      /* Until the mdes internal structures are enhanced, the
       * early and late time use the only time stored (for now).
       */
      op_action->actual_early_use_time = operand_latency[op_action->index];
      op_action->actual_late_use_time = operand_latency[op_action->index];

      /* 20040516SZU
       * Fix for Limpact path w/ regards to ADDLAT attr.
       *
       * Problem:
       * Additional latency scheduled but not showing up on isl attribute.
       *
       * isl attr uses dest[index]->actual_late_use_time.
       * ADDLAT uses dest[index]->add_lat, which is added onto dep latency,
       * but not to actual_late_use_time.
       * Assumes that op_action->add_lat == 0 if additional latency not
       * specified. (Should be right.)
       */
      if (op_action->add_lat != 0)
	op_action->actual_late_use_time += op_action->add_lat;
    }

  /* Update the scheduler bounds of dependent operations */
  SM_update_adjacent_bounds (sm_op);

  /* 
   * Place operation in the scheduled list of the cb 
   */

  /* Search from the end of already scheduled ops for the op
   * where this newly scheduled op should be placed after.
   * (This optimizes for scheduling an cb top-down, but works for all cases.)
   */
  for (after_this_op = sm_cb->last_sched_op; after_this_op != NULL;
       after_this_op = after_this_op->prev_sched_op)
    {
      /* Stop when hit an already scheduled op that should go before
       * this operation.
       */
      if ((after_this_op->sched_cycle < issue_time) ||
          ((after_this_op->sched_cycle == issue_time) &&
           (after_this_op->sched_slot <= sched_slot)))
        break;
    }

  /* Place sm_op after 'after_this_op' (if not NULL) */
  if (after_this_op != NULL)
    {
      sm_op->prev_sched_op = after_this_op;
      sm_op->next_sched_op = after_this_op->next_sched_op;
      if (after_this_op->next_sched_op != NULL)
        after_this_op->next_sched_op->prev_sched_op = sm_op;
      else
        sm_cb->last_sched_op = sm_op;
      after_this_op->next_sched_op = sm_op;
    }

  /* Otherwise, place sm_op at begining of scheduled list */
  else
    {
      sm_op->prev_sched_op = NULL;
      sm_op->next_sched_op = sm_cb->first_sched_op;
      if (sm_cb->first_sched_op != NULL)
        sm_cb->first_sched_op->prev_sched_op = sm_op;
      else
        sm_cb->last_sched_op = sm_op;
      sm_cb->first_sched_op = sm_op;
    }

  /* Update num unscheduled count */
  sm_cb->num_unsched--;

  /* Remove from dep_in_resolved queue, if sm_op is in this queue */
  if (sm_op->dep_in_resolved_qentry != NULL)
    {
      SM_dequeue_oper (sm_op->dep_in_resolved_qentry);
      sm_op->dep_in_resolved_qentry = NULL;
    }

  return (1);
}

/* 20031024 SZU
 * Depending on whether SM_do_template_bundling is set,
 * call the appropriate function.
 */
int
SM_schedule_oper (SM_Oper * sm_op, int issue_time, int earliest_slot,
                  int latest_slot, unsigned int sched_flags)
{
  if (SM_do_template_bundling)
    {
      return
	SM_schedule_oper_template_bundling (sm_op, issue_time, earliest_slot,
					    latest_slot, sched_flags);
    }
  else
    {
      return
	SM_schedule_oper_no_bundling (sm_op, issue_time, earliest_slot,
				      latest_slot, sched_flags);
    }
}

/* Unschedules an operation, freeing any resources used and updating
 * the scheduling bounds of dependent operations (before and after).
 * 20030829 SZU
 * Add priority_queue parameter as a flag and data.
 * Usually data from SM_schedule_oper_priority, when scheduling for Itanium.
 * Avoids unnecessary dependence checks outside of those operations.
 * If not, set to NULL for flag.
 */
void
SM_unschedule_oper (SM_Oper * sm_op, SM_Priority_Queue *priority_queue)
{
  SM_Cb *sm_cb;
  SM_Reg_Action *op_action;
  unsigned int op_flags;
  SM_Issue_Group *issue_group_ptr;

  /* The the sm_cb the op is in and sm_op's flags for ease of use */
  sm_cb = sm_op->sm_cb;
  op_flags = sm_op->flags;

  /* Sanity check, make sure operation is already scheduled! */
  if (!(sm_op->flags & SM_OP_SCHEDULED))
    {
      L_punt ("SM_unschedule_oper: %s op %i has not been scheduled!",
              sm_cb->lcode_fn->name, sm_op->lcode_op->id);
    }

  /* 20021211 SZU
   * Update Itanium info
   */
  if (SM_do_template_bundling)
    {
      issue_group_ptr = sm_op->issue_group;
      issue_group_ptr->slots[sm_op->sched_slot] = NULL;
      issue_group_ptr->full = 0;
      issue_group_ptr->num_slots_left += sm_op->mdes_op->num_slots;
      sm_op->issue_group = NULL;
    }

  /* Unschedule the operation using the alt previous chosen.
   * For now, the normal version and the silent version of the
   * alternatives have exactly the same requirements, so 
   * just use the normal version's table.
   *
   * Use different routines for acyclic and cyclic (modulo)
   * scheduling. -ITI/JCG 8/99
   */
  /* 20030302 SZU
   * SMH reconciliation
   */
  if (!sm_op->alt_chosen)
    L_punt ("SM_unschedule_oper: sm_op->alt_chosen NULL\n");

  if ((sm_cb->flags & SM_MODULO_RESOURCES) == 0)
    {
      SM_unsched_table (sm_cb, sm_op->alt_chosen->normal_version->table,
                        sm_op->options_chosen, sm_op->sched_cycle);
    }
  else
    {
      SM_modulo_unsched_table (sm_cb,
                               sm_op->alt_chosen->normal_version->table,
                               sm_op->options_chosen, sm_op->sched_cycle);
    }

  /* 20030910 SZU
   * Maybe should fix dep after adjusting bounds
   */
  /* Flag as not scheduled */
  sm_op->flags &= ~SM_OP_SCHEDULED;

  /* Recalculate the scheduler bounds of dependent operations 
   * NOTE: This routine must be called before any information about
   * where the operation was scheduled, etc. is destroyed.  This routine
   * uses this information to minimize the work it needs to do.
   * 20030829 SZU
   * New version of SM_recalculate_adjacent_bounds w/ new parameter.
   * Pass along priority_queue as paramater.
   */
  SM_recalculate_adjacent_bounds (sm_op, priority_queue);

  /* 20030302 SZU
   * SMH reconciliation
   */
  SM_undo_fix_soft_dep (sm_op);

  /* Reinitialize fields to their unscheduled values */
  sm_op->sched_cycle = SM_MIN_CYCLE;
  sm_op->sched_slot = SM_MAX_SLOT;
  sm_op->alt_chosen = NULL;

  /* Reinitialize the actual early and late use times for all register 
   * actions 
   */
  for (op_action = sm_op->first_op_action; op_action != NULL;
       op_action = op_action->next_op_action)
    {
      op_action->actual_early_use_time = -10000;
      op_action->actual_late_use_time = 10000;
    }

  /* Remove operation from the scheduled list of the cb */
  if (sm_op->prev_sched_op != NULL)
    sm_op->prev_sched_op->next_sched_op = sm_op->next_sched_op;
  else
    sm_cb->first_sched_op = sm_op->next_sched_op;

  if (sm_op->next_sched_op != NULL)
    sm_op->next_sched_op->prev_sched_op = sm_op->prev_sched_op;
  else
    sm_cb->last_sched_op = sm_op->prev_sched_op;

  sm_op->prev_sched_op = NULL;
  sm_op->next_sched_op = NULL;

  /* Update num unscheduled count */
  sm_cb->num_unsched++;
}

/* 20030717 SZU
 * Print out the issue groups and the operations scheduled in them for a cb.
 * Primarily for debugging.
 */
void
SM_spit_cb_issue_groups (SM_Cb *sm_cb, char *name)
{
  FILE *F;
  SM_Issue_Group *issue_group_ptr;
  int i, slots_per_template, num_slots, current_bundle;

  F = L_open_output_file (name);
  fprintf (F, "Function: %s CB: %i\n",
	   sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);

  slots_per_template = SM_get_slots_per_template (sm_cb);
  num_slots = SM_get_num_slots (sm_cb);

  for (issue_group_ptr = sm_cb->first_issue_group; issue_group_ptr != NULL;
       issue_group_ptr = issue_group_ptr->next_issue_group)
    {
      fprintf (F, "Time: %i\n", issue_group_ptr->issue_time);

      for (i = 0; i < num_slots; i++)
	{
	  SM_Oper *sm_op = issue_group_ptr->slots[i];

	  current_bundle = i / slots_per_template;

	  if (i % slots_per_template == 0)
	    {
	      SM_Bundle *bundle_ptr = issue_group_ptr->bundles[current_bundle];

	      fprintf (F, "Mask: 0x%x Index: %i Stop: %i Empty: %i Lock: %i "
		       "Internal: %i\n",
		       bundle_ptr->template_mask, bundle_ptr->template_index,
		       bundle_ptr->stop, bundle_ptr->empty,
		       bundle_ptr->template_lock,
		       bundle_ptr->internal_stop_bit);
	    }

	  if (sm_op)
	    if (SM_is_nop (sm_op))
	      {
		fprintf (F, "\tNOP");
	      }
	    else
	      {
		fprintf (F, "\tOp: %i", sm_op->lcode_op->id);
	      }
	  else
	    fprintf (F, "\t---");

	  if (i % slots_per_template == 2)
	    fprintf (F, "\n");
	}
      fprintf (F, "\n");
    }
  L_close_output_file (F);
}

