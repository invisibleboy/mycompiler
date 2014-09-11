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
 *      File:   sm_dep.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Additions, Modifications: David August
 *      Modulo Scheduling support: IMPACT Technologies (John Gyllenhaal)
 *      Creation Date:  July 1996
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sm.h"
#include <Lcode/l_main.h>
#include <Lcode/l_accspec.h>
#include <library/l_alloc_new.h>
#include <library/heap.h>
#include "sm_cudd.h"

L_Alloc_Pool *SM_Dep_pool = NULL;
/* 20020824 SZU */
L_Alloc_Pool *SM_Dep_PCLat_pool = NULL;

/* These will become parameters eventually */

/*
 * If set, assumes that Lbx86 mcode and allows the SM to make
 * (usually) stronger assumptions when drawing dependences.
 *
 * Assumptions made (currently):
 *
 * 1) It is legal to reorder dead definitions.  This is 
 *    designed to allow arithmetic instructions to reorder, since
 *    they all define flags.  I believe all other codegenerators
 *    (I know Lhppa does) assumes that dead defs are not reordered, 
 *    so this cannot be made a general scheduling optimization.
 *    (For evaluation/debugging purposes, may be turned off with
 *     SM_prevent_dead_Lbx86_defs_from_reordering.)
 * 
 *
 * DO NOT TURN ON FOR ANYTHING OTHER THAN Lbx86.  This flag causes
 * more analysis to be done (slows down SM) and probably will not
 * draw dependences correctly for any other codegenerator.
 *
 * This global variable is set to 1 in Lbx86/lbx86_main.c -JCG 3/6/98.
 */
int SM_make_Lbx86_assumptions = 0;

/* Set to one to assume dataflow has not been performend. 
 * As a result, conservative assumptions must be made.
 * This is primary to support the Lhyper port to SM. -JCG 6/99
 */
int SM_use_fake_dataflow_info = 0;

/* 
 * Set to one to prevent Lbx86 from allowing dead defs to reorder.
 * Only has effect if SM_make_Lbx86_assumptions = 1 (Lbx86 specific).
 */
int SM_prevent_dead_Lbx86_defs_from_reordering = 0;

/*
 * Set to one to assume dataflow is not done yet (needs to be enhanced to
 *    take advantage of the stronger assumptions).  As a result,
 *    conservative assumptions must be made.
 */
int SM_use_fake_Lbx86_flow_analysis = 1;

/* 20020804 SZU
 * Return SM_PCLat if this prod_cons_lat penalty exists in sm_mdes.
 * Also need to determine which action of the op the action is,
 * since latency is for specific operand(action) of an op.
 * Maybe determining action of the op should be done by caller.
 * This function returns the SM_PCLat entries for caller to make decision.
 * 20020823 SZU
 * JWS suggests putting action match in here as well.
 * Also need to modify for Set of hash int instead of string.
 * Do not return SM_PCLat. Make list of matching pclat and tag onto SM_Dep.
 */
void
SM_check_prod_cons_lat_match (SM_Dep *dep, SM_Reg_Action *prod_action, 
			      SM_Reg_Action *cons_action)
{
  Mdes_Operation *prod_mdes_op, *cons_mdes_op;
  SM_Mdes *sm_mdes;
  Mdes *mdes;
  SM_PCLat *pclat_array, *pclat;
  int index1, prod_index, cons_index;
  SM_Dep_PCLat *dep_pclat, *next_dep_pclat;
  int from_penalty, to_penalty;

  /* Initialize dep_pclat pool if necessary */
  if (SM_Dep_PCLat_pool == NULL)
    {
      SM_Dep_PCLat_pool = L_create_alloc_pool ("SM_Dep_PCLat",
					       sizeof (SM_Dep_PCLat), 128);
    }

  if (dep->pclat_list != NULL)
    {
      /* Free the current pclat list. Should be invalid.
       */
      for (dep_pclat = dep->pclat_list; dep_pclat != NULL;
	   dep_pclat = next_dep_pclat)
	{
          /* Get the next dep_pclat before deleting this one */
          next_dep_pclat = dep_pclat->next_dep_pclat;

          /* Delete this dep_pclat */
	  L_free (SM_Dep_PCLat_pool, (void *)dep_pclat);
	}

      dep->pclat_list = NULL;
    }

  /* Get the Mdes_Operation info */
  prod_mdes_op = prod_action->sm_op->mdes_op;
  cons_mdes_op = cons_action->sm_op->mdes_op;

  /* Get a pointer to the sm_mdes and the prod_cons_lat info */
  sm_mdes = prod_action->sm_op->sm_cb->sm_mdes;
  pclat_array = sm_mdes->pclat_array;
  mdes = sm_mdes->mdes2->version1_mdes;

  /* Checking each prod_cons_lat entry */
  for (index1 = 0; index1 < sm_mdes->pclat_array_size; index1++)
    {
      /* Get pointer to current pclat entry */
      pclat = &pclat_array[index1];

      if (!(Set_intersect_empty (prod_mdes_op->lat_class, pclat->plat)))
	{
	  if (!(Set_intersect_empty (cons_mdes_op->lat_class, pclat->clat)))
	    {
	      /* Both producer and consumer oper match.
	       * Check action (operand)
	       */
	      prod_index = operand_number (prod_action->index);
	      cons_index = operand_number (cons_action->index);

	      /* Actions also match. Add to list of penalties of dep.
	       */
	      if ((pclat->pdest_latency_penalty[prod_index] != 0) &&
		  (pclat->csrc_latency_penalty[cons_index] != 0))
		{
#if DEBUG_PCLAT
		  printf ("PCLat found\n");
		  printf ("Producer:\n");
		  SM_print_oper (stdout, prod_action->sm_op);
		  printf ("Consumer:\n");
		  SM_print_oper (stdout, cons_action->sm_op);
		  fprintf (stdout, "\n");
#endif

	          from_penalty = pclat->pdest_latency_penalty[prod_index];
	          to_penalty = pclat->csrc_latency_penalty[cons_index];

		  /* Allocate dep_pclat */
		  dep_pclat = (SM_Dep_PCLat *) L_alloc (SM_Dep_PCLat_pool);
		  dep_pclat->pclat = pclat;
		  dep_pclat->from_penalty = from_penalty;
		  dep_pclat->to_penalty = to_penalty;

		  if (dep->pclat_list == NULL)
		    {
		      dep_pclat->next_dep_pclat = NULL;
		      dep_pclat->prev_dep_pclat = NULL;
		      dep->pclat_list = dep_pclat;
		    }
		  else
		    {
		      /* Assume for now that order of PCLat makes no difference.
		       * So put at beginning
		       */
		      dep->pclat_list->prev_dep_pclat = dep_pclat;
		      dep_pclat->next_dep_pclat = dep->pclat_list;
		      dep->pclat_list = dep_pclat;
		    }
		}
	    }
	}
    }
}

void
SM_update_upper_bounds (SM_Dep * dep, int flags)
{
  SM_Oper *from_op, *to_op;
  int cycle_dep_resolved;
  unsigned short max_possible_slot;

  /* Cache from and to opers, and dep flags,  for ease of use */
  from_op = dep->from_action->sm_op;
  to_op = dep->to_action->sm_op;

#if 0
  printf ("Op %d: upper %d %d\n",
          from_op->lcode_op->id,
          from_op->cycle_upper_bound, from_op->nosoft_cycle_upper_bound);
#endif

  if ((to_op->flags & SM_OP_SCHEDULED) == 0)
    L_punt ("SM_update_upper_bounds: op%d not scheduled\n",
            to_op->lcode_op->id);

  /* Get max slot for ease of use */
  max_possible_slot = SM_MAX_SLOT;

  /* Don't need to do anything special for variable delay ops,
   * since they are updated automatically during scheduling.
   *
   * Use min delay for bounding calculation.
   *
   * If omega is not 0, use II in calculation. -ITI/JCG 8/99
   */

  cycle_dep_resolved = to_op->sched_cycle - dep->min_delay;

  if (dep->omega != 0)
    cycle_dep_resolved += (dep->omega * to_op->sm_cb->II);
  
#if 0
  /* JWS 20040105 : delay slack consumers of loads */

  if (SM_sched_slack_loads_for_miss && (dep->flags & SM_REG_DEP) &&
      (dep->flags & (SM_FLOW_DEP | SM_OUTPUT_DEP)) &&
      (from_op->mdes_flags & OP_FLAG_LOAD))
    {
      int slack;

      if (from_op->early_time < from_op->cycle_lower_bound)
	slack = cycle_dep_resolved - from_op->cycle_lower_bound;
      else
	slack = cycle_dep_resolved - from_op->early_time;

      if (slack >= 4)
	fprintf (stderr,"(%d,%d-%d/%d)", from_op->lcode_op->id,
		 cycle_dep_resolved,
		 from_op->early_time, from_op->cycle_lower_bound);

      if (slack >= 6)
	cycle_dep_resolved -= 4;
      else if (slack >= 4)
	cycle_dep_resolved -= 2;
    }
#endif

  /* Handle zero-cycle intra-interation dependences separately.
   * Cross-iteration dependences all handled same way -ITI/JCG 8/99
   */
  /* 20031022 SZU */
#if 0
  if ((dep->min_delay != 0) || (dep->omega != 0))
#else
  if ((dep->min_delay != 0) || ((dep->omega != 0) && (dep->flags & SM_REG_DEP)))
#endif
    {
      /* Update upper bound for non-zero-cycle dependences */
      if (from_op->cycle_upper_bound > cycle_dep_resolved)
        {
          from_op->cycle_upper_bound = cycle_dep_resolved;
          from_op->slot_upper_bound = max_possible_slot;
          from_op->dep_upper_bound = dep;
        }
    }
  else
    {
      /* Update upper bound for zero-cycle dependences */
      if ((from_op->cycle_upper_bound > cycle_dep_resolved) ||
          ((from_op->cycle_upper_bound == cycle_dep_resolved) &&
           (from_op->slot_upper_bound >= to_op->sched_slot)))
        {
          if (to_op->sched_slot != 0)
            {
              from_op->cycle_upper_bound = cycle_dep_resolved;
              from_op->slot_upper_bound = to_op->sched_slot - 1;
              from_op->dep_upper_bound = dep;
            }
          else
            {
              from_op->cycle_upper_bound = cycle_dep_resolved - 1;
              from_op->slot_upper_bound = max_possible_slot;
              from_op->dep_upper_bound = dep;
            }
        }
    }

  /* Same as above, but excludes soft_deps
   */
  if (flags & SM_HARD_DEP)
    {
      /* 20031022 SZU */
#if 0
      if ((dep->min_delay != 0) || (dep->omega != 0))
#else
      if ((dep->min_delay != 0) ||
	  ((dep->omega != 0) && (dep->flags & SM_REG_DEP)))
#endif
        {
          /* Update upper bound for non-zero-cycle dependences */
          if (from_op->nosoft_cycle_upper_bound > cycle_dep_resolved)
            {
              from_op->nosoft_cycle_upper_bound = cycle_dep_resolved;
              from_op->nosoft_slot_upper_bound = max_possible_slot;
              from_op->nosoft_dep_upper_bound = dep;
            }
        }
      else
        {
          /* Update upper bound for zero-cycle dependences */
          if ((from_op->nosoft_cycle_upper_bound > cycle_dep_resolved) ||
              ((from_op->nosoft_cycle_upper_bound == cycle_dep_resolved) &&
               (from_op->nosoft_slot_upper_bound >= to_op->sched_slot)))
            {
              if (to_op->sched_slot != 0)
                {
                  from_op->nosoft_cycle_upper_bound = cycle_dep_resolved;
                  from_op->nosoft_slot_upper_bound = to_op->sched_slot - 1;
                  from_op->nosoft_dep_upper_bound = dep;
                }
              else
                {
                  from_op->nosoft_cycle_upper_bound = cycle_dep_resolved - 1;
                  from_op->nosoft_slot_upper_bound = max_possible_slot;
                  from_op->nosoft_dep_upper_bound = dep;
                }
            }
        }
    }

#if 0
  printf ("Op %d: upper %d %d\n",
          from_op->lcode_op->id,
          from_op->cycle_upper_bound, from_op->nosoft_cycle_upper_bound);
#endif
}


void
SM_update_lower_bounds (SM_Dep * dep, int flags)
{
  SM_Oper *from_op, *to_op;
  int cycle_dep_resolved;
  unsigned short max_possible_slot;

  /* Cache from and to opers, and dep flags,  for ease of use */
  from_op = dep->from_action->sm_op;
  to_op = dep->to_action->sm_op;

  /* Get max slot for ease of use */
  max_possible_slot = SM_MAX_SLOT;

  if ((from_op->flags & SM_OP_SCHEDULED) == 0)
    L_punt ("SM_update_lower_bounds: op%d not scheduled\n",
            from_op->lcode_op->id);

  /* Don't need to do anything special for variable delay ops,
   * since they are updated automatically during scheduling.
   *
   * Use min delay for bounding calculation.
   *
   * If omega is not 0, use II in calculation. -ITI/JCG 8/99
   */

  cycle_dep_resolved = from_op->sched_cycle + dep->min_delay;

  if (dep->omega != 0)
    cycle_dep_resolved -= (dep->omega * from_op->sm_cb->II);

  /* JWS 20040105 : delay slack consumers of loads */

  if (SM_sched_slack_loads_for_miss && (dep->flags & SM_REG_DEP) &&
      (dep->flags & (SM_FLOW_DEP | SM_OUTPUT_DEP)) &&
      (from_op->mdes_flags & OP_FLAG_LOAD) && to_op->late_time)
    {
      int i, slack, min_late_time;

      min_late_time = to_op->late_time[0];
      for (i = 1; i < to_op->sm_cb->num_exits; i++)
	if (to_op->late_time[i] < min_late_time)
	  min_late_time = to_op->late_time[i];

      slack = min_late_time - cycle_dep_resolved;

      if (slack >= 6)
	cycle_dep_resolved += 4;
      else if (slack >= 4)
	cycle_dep_resolved += 2;
    }

  /* Handle zero-cycle intra-interation dependences separately.
   * Cross-iteration dependences all handled same way -ITI/JCG 8/99
   */
  /* 20031022 SZU */
#if 0
  if ((dep->min_delay != 0) || (dep->omega != 0))
#else
  if ((dep->min_delay != 0) || ((dep->omega != 0) && (dep->flags & SM_REG_DEP)))
#endif
    {
      /* Update lower bound for non-zero-cycle dependences */
      if (to_op->cycle_lower_bound < cycle_dep_resolved)
        {
          to_op->cycle_lower_bound = cycle_dep_resolved;
          to_op->slot_lower_bound = 0;
          to_op->dep_lower_bound = dep;
        }
    }
  else
    {
      /* Update lower bound for zero-cycle dependences */
      if ((to_op->cycle_lower_bound < cycle_dep_resolved) ||
          ((to_op->cycle_lower_bound == cycle_dep_resolved) &&
           (to_op->slot_lower_bound <= from_op->sched_slot)))
        {
          if (from_op->sched_slot != max_possible_slot)
            {
              to_op->cycle_lower_bound = cycle_dep_resolved;
              to_op->slot_lower_bound = from_op->sched_slot + 1;
              to_op->dep_lower_bound = dep;
            }
          else
            {
              to_op->cycle_lower_bound = cycle_dep_resolved + 1;
              to_op->slot_lower_bound = 0;
              to_op->dep_lower_bound = dep;

            }
        }
    }

  /* Same as above but ignores soft deps
   */
  if (flags & SM_HARD_DEP)
    {
      /* 20031022 SZU */
#if 0
      if ((dep->min_delay != 0) || (dep->omega != 0))
#else
      if ((dep->min_delay != 0) ||
	  ((dep->omega != 0) && (dep->flags & SM_REG_DEP)))
#endif
        {
          /* Update lower bound for non-zero-cycle dependences */
          if (to_op->nosoft_cycle_lower_bound < cycle_dep_resolved)
            {
              to_op->nosoft_cycle_lower_bound = cycle_dep_resolved;
              to_op->nosoft_slot_lower_bound = 0;
              to_op->nosoft_dep_lower_bound = dep;
            }
        }
      else
        {
          /* Update lower bound for zero-cycle dependences */
          if ((to_op->nosoft_cycle_lower_bound < cycle_dep_resolved) ||
              ((to_op->nosoft_cycle_lower_bound == cycle_dep_resolved) &&
               (to_op->nosoft_slot_lower_bound <= from_op->sched_slot)))
            {
              if (from_op->sched_slot != max_possible_slot)
                {
                  to_op->nosoft_cycle_lower_bound = cycle_dep_resolved;
                  to_op->nosoft_slot_lower_bound = from_op->sched_slot + 1;
                  to_op->nosoft_dep_lower_bound = dep;
                }
              else
                {
                  to_op->nosoft_cycle_lower_bound = cycle_dep_resolved + 1;
                  to_op->nosoft_slot_lower_bound = 0;
                  to_op->nosoft_dep_lower_bound = dep;
                }
            }
        }
    }

    /* Sanity check, if both operations are scheduled,
     * make sure they are not violating the bounds 
     */
    if (to_op->flags & SM_OP_SCHEDULED)
      {
	if ((to_op->sched_cycle < to_op->cycle_lower_bound) ||
	    ((to_op->sched_cycle == to_op->cycle_lower_bound) &&
	     (to_op->sched_slot < to_op->slot_lower_bound)))
	  {
	    fprintf (stderr, "\n%s cb %i:\n",
		     to_op->sm_cb->lcode_fn->name,
		     to_op->sm_cb->lcode_cb->id);
	    fprintf (stderr, "Cycle %i slot %i:",
		     from_op->sched_cycle, from_op->sched_slot);
	    SM_print_oper (stderr, from_op);
	    fprintf (stderr, "        ");
	    SM_print_dep (stderr, dep);
	    fprintf (stderr, "Cycle %i slot %i:",
		     to_op->sched_cycle, to_op->sched_slot);
	    SM_print_oper (stderr, to_op);

	    L_warn ("SM_update_lower_bounds: This new dependence makes "
		    "schedule invalid! -- slots are probably just"
		    " messed up");
	  }
      }
}

/* Analyzes the actions predicates (if any), and returns 1 if
 * the action are on mutually exclusive predicate control paths,
 * otherwise 0.
 *
 * Used to determine of a dependence should be drawn.
 *
 * Unpredicated code always returns 0 (not mutually exclusive).
 *
 * JWS 20000107 - WARNING! Do not compare pred operands for "sameness"
 * using L_same_operand().  This *does not work*.  *Always* reference 
 * the predicate graph.
 */
int
SM_mutually_exclusive_actions (SM_Reg_Action * action1, int demoted1,
                               SM_Reg_Action * action2, int demoted2)
{
  L_Oper *lcode_op1, *lcode_op2;

  /* Return 0 if predicate operands are not supported by this architecture */
  if ((L_max_pred_operand <= 0) || SM_ignore_pred_analysis)
    return (0);

  /* Get the ops for the two actions for ease of use */
  lcode_op1 = action1->sm_op->lcode_op;
  lcode_op2 = action2->sm_op->lcode_op;

  /* Query predicate graph to see if there is a control path between
   * these operations.  If there is, they are not mutually exclusive,
   * so return 0.
   */
  if (PG_intersecting_predicates_ops_explicit (lcode_op1, demoted1,
                                               lcode_op2, demoted2))
    {
      return (0);
    }

  /* Otherwise, they are mutually exclusive, return 1 */
  else
    {
#if 0
      fprintf (stdout, "Mutually exclusive:\n");
      SM_print_oper (stdout, action1->sm_op);
      SM_print_oper (stdout, action2->sm_op);
      fprintf (stdout, "\n");
#endif
      return (1);
    }
}

void
SM_check_deps (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  SM_Reg_Action *action;
  SM_Dep *dep_in;
  int num_hard_in;
  int num_soft_in;

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      if ((sm_op->flags & SM_OP_SCHEDULED) || (sm_op->ignore))
        continue;

      num_hard_in = 0;
      num_soft_in = 0;
      for (action = sm_op->first_op_action; action != NULL;
           action = action->next_op_action)
        {
          for (dep_in = action->first_dep_in; dep_in != NULL;
               dep_in = dep_in->next_dep_in)
            {
              if (dep_in->ignore ||
                  dep_in->from_action->sm_op->flags & SM_OP_SCHEDULED ||
                  dep_in->from_action->sm_op->ignore)
                continue;
              if (dep_in->flags & SM_HARD_DEP)
                num_hard_in++;
              if (dep_in->flags & SM_SOFT_DEP)
                num_soft_in++;
            }
        }
      if (sm_op->num_unresolved_hard_dep_in != num_hard_in)
        {
          L_punt ("Bad hard unresolved in dep count op %d.\n"
                  "Was %d, should be %d",
                  sm_op->lcode_op->id, sm_op->num_unresolved_hard_dep_in,
                  num_hard_in);
        }

      if (sm_op->num_unresolved_soft_dep_in != num_soft_in)
        {
          L_punt ("Bad soft unresolved in dep count op %d.\n"
                  "Was %d, should be %d",
                  sm_op->lcode_op->id, sm_op->num_unresolved_soft_dep_in,
                  num_soft_in);
        }

    }
}

/* Determines if the result of a Lbx86 def action is never used (dead).
 * Returns 1 if the def action is dead, 0 otherwise.  Action is
 * required to be a def action.  It may be an actual implicit or 
 * actual explicit action.
 *
 * This routine is used by SM_add_dep(), SM_def_dominates_action(), 
 * and SM_def_post_dominates_action().  
 *
 * Only legal to call this for Lbx86 (SM_make_Lbx86_assumptions = 1)!
 *
 * (Among other things, this code does not handle predicates properly.)
 */
int
SM_Lbx86_def_action_is_dead (SM_Reg_Action * def_action)
{
  int reaches_cb_end, can_reach_end;
  unsigned int def_killed_at;
  SM_Reg_Action **conflict_ptr, *conflict, *action_after;

  /* Sanity check, make sure in Lbx86 */
  if (!SM_make_Lbx86_assumptions)
    L_punt
      ("SM_Lbx86_def_action_is_dead: SM_make_Lbx86_assumptions not set!");

  /* Extension operands (control, mem, sync, and vliw) are never dead! */
  if (def_action->rinfo->type & SM_EXT_ACTION_TYPE)
    return (0);

  /* Sanity check, make sure action is a def action */
  if (!(def_action->flags & SM_DEF_ACTION))
    L_punt ("SM_Lbx86_def_action_is_dead: def_action not a def!");

  /* Sanity check, make sure action is an actual action */
  if (!(def_action->flags & SM_ACTUAL_ACTION))
    L_punt ("SM_Lbx86_def_action_is_dead: def_action not an actual action!");

  /* First look to see where this actual def is killed (if it is).
   * This helps bound where to look for possible uses when considering
   * overlapping registers
   */
  action_after = def_action->next_actual;

  /* If no action after, no bounds can be used. */
  if (action_after == NULL)
    {
      def_killed_at = (unsigned int) -1;        /* Largest number */
      can_reach_end = 1;
    }

  /* If use, don't look further, this def is not dead */
  else if (action_after->flags & SM_USE_ACTION)
    {
      return (0);
    }

  /* Otherwise, all "potential" uses must occur at or before this op */
  else
    {
      def_killed_at = action_after->sm_op->serial_number;
      can_reach_end = 0;
    }

  /* Check to make sure this def is dead for every register this
   * def_action conflicts with.  Hopefully this will not be too
   * conservative to be useful!  (It seems necessary for correctness.)
   */
  for (conflict_ptr = def_action->conflict;
       (conflict = *conflict_ptr) != NULL; conflict_ptr++)
    {
      /* Initially assume that def reaches cb end (if there is
       * not a known def of the same register that kills this 
       * def before the use
       */
      reaches_cb_end = can_reach_end;

      /* Look at all the actions after this def until either hit an
       * use (which means it is not dead) or hit a definition that
       * post-dominates this def (which means it is dead for this
       * particular conflict).  (Assumes post-dominates for any
       * def encountered, since should not have predicate code.)
       */
      for (action_after = conflict->next_actual;
           action_after != NULL; action_after = action_after->next_actual)
        {
          /* If action_after is after the explicit kill of the def,
           * don't need to look further and def does not reach
           * cb end.
           */
          if (action_after->sm_op->serial_number > def_killed_at)
            {
              reaches_cb_end = 0;
              break;
            }

          /* If found a use, then this def is not dead */
          if (action_after->flags & SM_USE_ACTION)
            {
              return (0);
            }

          /* Otherwise, found a def, then def is dead for this conflict 
           * and we can stop search.
           */
          else
            {
              /* Stop for this conflict if this action does post-dominate 
               * (and mark that def does not reach cb end)
               */
              reaches_cb_end = 0;
              break;
            }
        }

      /* If def is not redefined before the end of the cb, 
       * check the cb's liveout set to deterimine if there
       * is a use.  (A conservative answer would be to assume
       * that there is a use liveout.)  Check the conflict operand
       * instead of the actual def operand, to get the most correct
       * answer.
       *
       * (The code for determining if a def is live out is 
       *  taken from SM_def_has_exactly_one_use ().  If it is
       *  incorrect, please fix the other function also in sm_opti.c.)
       */
      if (reaches_cb_end)
        {
#if 0
          /* Use dataflow once ported to Lbx86 assumptions! */
          if (1                 
              /*L_in_cb_OUT_set (def_action->sm_op->sm_cb->lcode_cb, 
                conflict->rinfo->operand) */ )
            {
#endif
              return (0);
#if 0
            }
#endif
        }
    }

  /* If reaches here, the definition must be dead */
  return (1);
}


/*
 * Conditionally adds a dependence between two sm_ops between the
 * from_action and the to_action, depending on the predicates for
 * the operations and the actions involved.  
 *
 * Conditions where a dependence will not be added:
 * 1) The two action's predicates are mutually exclusive AND
 *    neither action is an SM_PRED_UNCOND_DEF or a SM_PRED_UNCOND_USE
 *    (since these actions define/use the predicate independently 
 *    of the operation's predicate).  
 *    Also requires that omega == 0 -ITI/JCG 8/99
 * 
 *
 * 2) The omega is zero (not a cross-cycle dependence), and we are
 *    trying to add an dependence from an operation to itself
 *    (e.g. for an operation that increments r1, having a self-anti 
 *     dependence from the src r1 to the dest r1).  
 *    While "technically" correct, the schedule mangager does not handle
 *    these properly, so just delete them.
 *
 * 3) If SM_PREVENT_REDUNDANT_DEPS is set in 'flags' and this exact
 *    dep already exists (ignoring BUILD_DEP_IN/OUT), then this dep
 *    will not be added.  This flag is used when adding or changing
 *    operations in a cb with the dependence graph is already drawn.
 * 
 * 4) If SM_make_Lbx86_assumptions == 1 and 
 *    SM_prevent_dead_Lbx86_defs_from_reordering == 0, and drawing output 
 *    dependence between two DEAD defs, then this output dependence will 
 *    not be added.  This functionality is primarily for handling the fact
 *    that a lot of instructions define the flags in Lbx86 but these 
 *    definitions are not used.
 *
 * 5) If drawing a dependence between different register operands 
 *    (due to the conflicting action mechanism) and the from_action's
 *    register is refined before to_action, and the redefinition post
 *    dominates from_action, then the dependence is clearly not
 *    necessary since from_action's value can never reach to_action.
 *    -JCG 6/29/98  (added to handle Lbx86 flags better.)
 *    For now, also requires that omega == 0.  May be able to do an enhanced
 *    version for cross-iteration dependences later, if desired. -ITI/JCG 8/99
 */

void
SM_add_dep (SM_Reg_Action * from_action, SM_Reg_Action * to_action,
            unsigned int dep_flags, unsigned int ignore,
            short delay_offset, short omega, unsigned int mode)
{
  SM_Dep *dep, *dep_out;
  SM_Oper *from_op, *to_op;
  SM_Cb *sm_cb;
  SM_Reg_Action *def_after;
  short min_delay = 0, max_delay = 0;
  short min_early_use_time, max_early_use_time;
  short min_late_use_time, max_late_use_time, max_possible_slot;
  unsigned int old_dep_flags, new_dep_flags, cb_flags;
  SM_Dep_PCLat *dep_pclat;
  int from_penalty, to_penalty;
  int max_from_penalty, min_from_penalty, max_to_penalty, min_to_penalty;

  /* Return now if a dependence does not need to be added due to
   * the action's predicates (namely the action's predicates indicate
   * that actions are mutually exclusive).  However, a dependence must
   * always be added if either action is a SM_PRED_UNCOND_DEF or a
   * SM_PRED_UNCOND_USE, since these def/use actions always modify/use 
   * the register contents, even when the operation is predicate squashed.
   *
   * This test holds only for inter-iteration dependences (omega = 0).
   * For cross-iteration dependences (omega != 0), predicate relationships
   * cannot currently be determined and conservative assumptions must be 
   * made (actions are never mutually exclusive). -ITI/JCG 8/99
   *
   * For some architectures, special soft dependences need to be added for
   * machine related dependences. Namely, it might not be possible to have 
   * a promoted consumption of a value written in the same cycle even if
   * the consumer's result is not used (because it would not have executed
   * had it not been promoted). The search for forward hard deps
   * can be halted when a postdominating definition is found.  However, 
   * soft deps still need to be drawn to select ops following the 
   * postdominating definition.  Such ops are not dataflow dependent
   * on the postdominating def and may be scheduled above it. -EMN 3/01
   */
  if ((omega == 0) && !(mode & SM_FORCE_DEP))
    {
      if (from_action->flags & SM_USE_ACTION)
        {
          if (to_action->flags & SM_USE_ACTION)
            {
              fprintf (stderr, "From action: ");
              SM_print_action_id (stderr, from_action);
              fprintf (stderr, "\nTo action: ");
              SM_print_action_id (stderr, to_action);
              L_punt ("\nSM_add_dep: USE->USE dependence request.");
            }
          else if (to_action->flags & SM_DEF_ACTION)
            {
              /*  ANTI:TRUE  use [demoted pred]  - when actually consumed
               *       FALSE def [promoted pred] - when result produced
               *  moving def b/f use must not _ever_ change use
               */
              if (!(from_action->flags & (SM_PRED_UNCOND_USE)) &&
                  !(to_action->flags & (SM_PRED_UNCOND_DEF)) &&
                  SM_mutually_exclusive_actions (from_action, TRUE, to_action,
                                                 FALSE))
                {
                  if (!L_EXTRACT_BIT_VAL
                      (from_action->sm_op->lcode_op->flags, L_OPER_PROMOTED)
                      && !L_EXTRACT_BIT_VAL (to_action->sm_op->
                                             lcode_op->flags,
                                             L_OPER_PROMOTED))
                    return;
#if 1
		  if (mode & SM_SOFT_DEP)
		    printf
		      ("Using a soft ANTI dependence because of promotion: "
		       "op%d -> op%d\n",
		       from_action->sm_op->lcode_op->id,
		       to_action->sm_op->lcode_op->id);
#endif
                  dep_flags &= ~(SM_HARD_DEP);
                  dep_flags |= (SM_SOFT_DEP | SM_SOFTFIX_PROMOTION);
                }
            }
          else
            {
              SM_print_action_id (stderr, to_action);
              L_punt ("SM_add_dep: Action is not SRC or DST.");
            }
        }
      else if (from_action->flags & SM_DEF_ACTION)
        {
          if ((to_action->flags & SM_USE_ACTION))
            {
              int to_demoted;

              /* For control dependences it is not safe to treat the
               * dependent action as on its demoted predicate, as
               * there must be a soft dependence broken for an oper to
               * be silenced.  JWS 20000629 */

              if (dep_flags & SM_CTRL_DEP)
                to_demoted = FALSE;
              else
                to_demoted = TRUE;

              /*For non-control ops
               *  FLOW: FALSE def [promoted pred] - either ok
               *        TRUE  use [demoted pred]  - when value is consumed
               *  if def's demoted pred were exclusive, the promoted pred
               *    would have to be too or else promotion is invalid
               *For control ops
               *  FLOW: FALSE def [promoted pred] 
               *        FALSE use [promoted pred] 
               *  control flow is special, pred that will actually be 
               *    on use must be exclusive with def
               */
              if (!(from_action->flags & (SM_PRED_UNCOND_DEF)) &&
                  !(to_action->flags & (SM_PRED_UNCOND_USE)) &&
                  SM_mutually_exclusive_actions (from_action, FALSE,
                                                 to_action, to_demoted))
                {
                  if (!L_EXTRACT_BIT_VAL
                      (from_action->sm_op->lcode_op->flags, L_OPER_PROMOTED)
                      && !L_EXTRACT_BIT_VAL (to_action->sm_op->
                                             lcode_op->flags,
                                             L_OPER_PROMOTED))
                    return;
#if 1
		  if (mode & SM_SOFT_DEP)
		    printf
		      ("Using a soft FLOW dependence because of promotion: "
		       "op%d -> op%d\n",
		       from_action->sm_op->lcode_op->id,
		       to_action->sm_op->lcode_op->id);
#endif
                  dep_flags &= ~(SM_HARD_DEP);
                  dep_flags |= (SM_SOFT_DEP | SM_SOFTFIX_PROMOTION);
                }
            }
          else if (to_action->flags & SM_DEF_ACTION)
            {
              /*  OUTPUT: FALSE def [promoted pred] 
               *          TRUE  def [demoted pred]
               *  first def must use promoted value because reordering
               *   could cause it to clobber second def's value
               */
              if (!(from_action->flags & (SM_PRED_UNCOND_DEF)) &&
                  !(to_action->flags & (SM_PRED_UNCOND_DEF)) &&
                  SM_mutually_exclusive_actions (from_action, FALSE,
                                                 to_action, TRUE))
                {
                  if (!L_EXTRACT_BIT_VAL
                      (from_action->sm_op->lcode_op->flags, L_OPER_PROMOTED)
                      && !L_EXTRACT_BIT_VAL (to_action->sm_op->
                                             lcode_op->flags,
                                             L_OPER_PROMOTED))
                    return;
#if 1
		  if (mode & SM_SOFT_DEP)
		    printf
		      ("Using a soft OUTPUT dependence because of promotion: "
		       "op%d -> op%d\n",
		       from_action->sm_op->lcode_op->id,
		       to_action->sm_op->lcode_op->id);
#endif
                  dep_flags &= ~(SM_HARD_DEP);
                  dep_flags |= (SM_SOFT_DEP | SM_SOFTFIX_PROMOTION);
                }
            }
          else
            {
              SM_print_action_id (stderr, to_action);
              L_punt ("SM_add_dep: Action is not SRC or DST.");
            }
        }
      else
        {
          SM_print_action_id (stderr, from_action);
          L_punt ("SM_add_dep: Action is not SRC or DST.");
        }
    }

  /* Return now if trying to add a dependence between two implicit operands.
   * They are unnecessary and get added when dealing with predicate jsrs.
   *
   * The dependence checker also considers these dependences mistakes. :)
   */
  if ((from_action->flags & SM_IMPLICIT_ACTION) &&
      (to_action->flags & SM_IMPLICIT_ACTION))
    {
      return;
    }

  /* Return now if trying to add a dependence between a implicit action
   * and a conflicting action.  With the new dataflow that operates on
   * operand components, these dependences are not necessary (dataflow
   * queries return the correct answers, even for Lbx86's pathological 
   * cases) and they unnecessarily constrain scheduling/optimization for
   * Lbx86.  For non-Lbx86 codegenerators, not adding these dependences 
   * is also OK (the old scheduler didn't) because operands overlap was 
   * either very simple (Lhppa) or was handled internally in the 
   * codegenerator (i.e., Lx86 always used EAX, even in instructions that 
   * only wrote AL, AX, etc.). -JCG 7/8/98.
   */
  if (((from_action->flags & SM_IMPLICIT_ACTION) &&
       (to_action->flags & SM_CONFLICTING_ACTION)) ||
      ((from_action->flags & SM_CONFLICTING_ACTION) &&
       (to_action->flags & SM_IMPLICIT_ACTION)))
    {
      return;
    }

  /* Cache from and to opers for ease of use */
  from_op = from_action->sm_op;
  to_op = to_action->sm_op;

  /* Don't add non-cross iteration self-dependences */
  if ((omega == 0) && (from_op == to_op))
    {
      return;
    }


  /* If allowing dead Lbx86 defs to reorder, don't add output dependence
   * between two dead defs.
   */
  if (SM_make_Lbx86_assumptions &&
      (!SM_prevent_dead_Lbx86_defs_from_reordering) &&
      (dep_flags & SM_OUTPUT_DEP) &&
      SM_Lbx86_def_action_is_dead (from_action) &&
      SM_Lbx86_def_action_is_dead (to_action))
    {
      return;
    }

  /* If drawing a dependence between two different registers (due to
   * the conflicting operand mechanism) and the from_action register is 
   * redefined before the to_action, and that redefinition post dominates 
   * from_action, the dependence is not needed.
   */
  if (from_action->rinfo != to_action->rinfo)
    {
      /* This test is only sufficient for omega == 0.  For cross-iteration
       * dependences, be conservative for now. -ITI/JCG 8/99
       */
      if (omega == 0)
        {
          /* Scan ALL of the defs after from_action and before to_action
           * to see if it post dominates the from action.  This loop
           * is necessary to catch cases missed due to the 
           * 'dead def' optimization done for Lbx86, which pushes down
           * the post dominating def to severel defs after from_action.
           * -JCG 6/30/98
           */
          for (def_after = from_action->next_def;
               ((def_after != NULL) &&
                (def_after->sm_op->serial_number <
                 to_action->sm_op->serial_number));
               def_after = def_after->next_def)
            {
              if (SM_def_post_dominates_action (def_after, from_action))
                {
                  return;
                }
            }
        }
    }

  /* Cache sm_cb info for ease of use */
  sm_cb = from_op->sm_cb;
  cb_flags = sm_cb->flags;

  /* If MVE (modulo variable expansion) is going to be performed 
   * (currently always after modulo scheduling), many anti and output 
   * dependences should be ignored since MVE will fix it them up
   * afterwards (this maximizes scheduling freedom).  MVE only
   * makes sense for prepass scheduling.  -ITI/JCG 8/99
   *
   * Currently ignore anti and output dependences unless cannot be
   * renamed by MVE.  In order to be renamed, the dependence must
   * be between identical virtual registers.  To handle predicated
   * code, the following rules also must be used:
   * 
   *   1) Can ignore dep if omega > 0.  (MVE can always rename 
   *      cross-iteration assuming predicate definitions are transformed 
   *      so the there are no infinite lifetimes.)
   *
   *   2) Can ignore dep if both operations are predicated 
   *      on the same thing (primarily for omega == 0).
   *      (Dependences between opposite predicates are prevented earlier.
   *      When have partially overlapping predicates within an iteration,
   *      MVE *cannot* rename them.)
   *
   *    * See JWS 20000107 above.
   */
  if ((cb_flags & SM_ASSUME_MVE) &&
      (dep_flags & (SM_ANTI_DEP | SM_OUTPUT_DEP)) &&
      (from_action->rinfo->type & SM_REGISTER_TYPE) &&
      (from_action->rinfo == to_action->rinfo))
    {
      /* Does it match rule 1 or 2 above? */

      if ((omega > 0) ||
          /*  !L_is_predicated(from_action->sm_op->lcode_op) && */
          (!L_is_predicated (to_action->sm_op->lcode_op) &&
           !L_is_ctype_predicate (to_action->rinfo->operand)))
        ignore |= SM_MVE_IGNORE;

      /* JWS 20001030 Renaming of predicated operands is not happening
       * properly
       */
#if 0
      if ((omega > 0) ||
          PG_equivalent_predicates_ops (from_action->sm_op->lcode_op,
                                        to_action->sm_op->lcode_op) &&
          !L_is_ctype_predicate (to_action->rinfo->operand))
        {
          /* Prevent ignorance of arcs between pred defs to (conservatively)
           * prevent reordering of an uncond write with a trans write 
           * -- JWS 
           */
          ignore |= SM_MVE_IGNORE;
        }
#endif
    }

  /* Do not add cross-iteration dependences if an intra-iteration
   * dependence already exists.  For efficiency, assumes that
   * the intra-iteration dependences for the from_action are
   * already drawn (this is currently the case). -ITI/JCG 8/99
   */
  if (omega != 0)
    {
      /* Search all of the from_action's dep_out to see if
       * there is an dep to this dep in.  If there is,
       * this cross-iteration dependance is redundant and not needed.
       */
      for (dep_out = from_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          /* If already have an dep between the actions, then don't
           * add another one.  To be conservative, don't add
           * if there is a dependence with a smaller (or equal)
           * omega. -ITI/JCG 8/99
           */
          if ((dep_out->to_action == to_action) && (dep_out->omega <= omega))
            {
              return;
            }
        }
    }

  /* Do we need to prevent redundant deps from being added? */
  if (dep_flags & SM_PREVENT_REDUNDANT_DEPS)
    {
      /* Search all of the from_action's dep_out to see if
       * there is an dep to this dep in (with the same omega).  If there is,
       * this dependance is redundant and not needed.
       */
      for (dep_out = from_action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          /* If already have an dep between the actions, then don't
           * add another one.
           */
          if ((dep_out->to_action == to_action) && (dep_out->omega == omega))
            {
              /* Sanity check, this dep BETTER be exactly the same
               * as the dep being added.  Ignore differences in
               * SM_BUILD_DEP_IN/OUT, SM_HARD_DEP (since implicit),
               * and SM_VARIABLE_DELAY.
               */

              /* Filter out SM_BUILD_DEP_IN & SM_BUILD_DEP_OUT */
              old_dep_flags = dep_out->flags &
                ~(SM_BUILD_DEP_IN | SM_BUILD_DEP_OUT | SM_HARD_DEP |
                  SM_VARIABLE_DELAY | SM_PREVENT_REDUNDANT_DEPS);
              new_dep_flags = dep_flags &
                ~(SM_BUILD_DEP_IN | SM_BUILD_DEP_OUT | SM_HARD_DEP |
                  SM_VARIABLE_DELAY | SM_PREVENT_REDUNDANT_DEPS);

              /* Perfrom sanity check */
              if ((dep_out->delay_offset != delay_offset) ||
                  (new_dep_flags != old_dep_flags))
                {
                  fprintf (stderr, "%s cb %i:\n",
                           sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);
                  SM_print_oper (stderr, from_action->sm_op);
                  SM_print_dep (stderr, dep_out);
                  SM_print_oper (stderr, to_action->sm_op);

                  fprintf (stderr,
                           "Old dep: delay_offset %i flags %x (%x)\n",
                           dep_out->delay_offset, dep_out->flags,
                           old_dep_flags);
                  fprintf (stderr,
                           "New dep: delay_offset %i flags %x (%x)\n",
                           delay_offset, dep_flags, new_dep_flags);
                  L_punt ("SM_add_dep: dep mismatch when preventing "
                          "redundant dep!");
                }
              return;
            }
        }
    }

  /* May have SM_SOFT_DEP or SM_HARD_DEP marked, if not a soft-dep,
   * automatically mark as a SM_HARD_DEP.
   */
  /* 20030930 SZU
   * To avoid generating and trying to break soft dep during postpass,
   * mark all deps as SM_HARD_DEP.
   */
#if 0
  if (dep_flags & SM_SOFT_DEP)
    {
      /* Make sure SM_HARD_DEP is not also marked */
      if (dep_flags & SM_HARD_DEP)
        {
          L_punt ("SM_add_dep: May not mark dep as both SOFT and HARD!");
        }
    }
  else
    {
      /* Automatically set hard flag */
      dep_flags |= SM_HARD_DEP;
    }
#else
  if (sm_cb->prepass_sched)
    if (dep_flags & SM_SOFT_DEP)
      {
	/* Make sure SM_HARD_DEP is not also marked */
	if (dep_flags & SM_HARD_DEP)
	  {
	    L_punt ("SM_add_dep: May not mark dep as both SOFT and HARD!");
	  }
      }
    else
      {
	/* Automatically set hard flag */
	dep_flags |= SM_HARD_DEP;
      }
  else
    dep_flags |= SM_HARD_DEP;
#endif


  /*
   * Mode specifies the types of deps that are allowed
   * to be created. It provides an easy way to limit the
   * number of HARD deps created while emabling certain
   * SOFT deps to still get placed.
   */
  if (!(mode & dep_flags))
    {
      return;
    }

  /* Initialize dep pool if necessary */
  if (SM_Dep_pool == NULL)
    {
      SM_Dep_pool = L_create_alloc_pool ("SM_Dep", sizeof (SM_Dep), 128);
    }

  /* Allocate dep */
  dep = (SM_Dep *) L_alloc (SM_Dep_pool);
  
  /* 20020825 SZU 
   * Initialize the pclat_list field of dep.
   */
  dep->pclat_list = NULL;

  /* Depending of the flags, the delay for the dep is either fixed
   * (not based on use times) and is just the delay_offset, or is
   * based on the mdes use times, adjusted with the delay offset.
   * May not have both flags set.
   */
  if ((dep_flags & (SM_FIXED_DELAY | SM_MDES_BASED_DELAY)) ==
      (SM_FIXED_DELAY | SM_MDES_BASED_DELAY))
    {
      L_punt ("SM_add_dep: May not have both SM_FIXED_DELAY and"
              " SM_MDES_BASED_DELAY set!");
    }

  /* Handle the fixed delay case */
  if (dep_flags & SM_FIXED_DELAY)
    {
      /* Set min and max delay to the delay offset specified */
      min_delay = delay_offset;
      max_delay = delay_offset;
    }

  /* Handled the mdes use time based delay case */
  else if (dep_flags & SM_MDES_BASED_DELAY)
    {
      /* 20020804 SZU
       * Check if mdes has Prod_Cons_Latency section.
       * Check if the producer & consumer (from_action & to_action) fit
       * any of the special producer_consumer latencies.
       */
      max_from_penalty = 0;
      min_from_penalty = 0;
      max_to_penalty = 0;
      min_to_penalty = 0;

      if (from_action->sm_op->sm_cb->sm_mdes->pclat_array != NULL)
	{
	  SM_check_prod_cons_lat_match (dep, from_action, to_action);

	  if (dep->pclat_list != NULL)
	    {
	      dep_pclat = dep->pclat_list;

	      max_from_penalty = dep_pclat->from_penalty;
	      min_from_penalty = dep_pclat->from_penalty;
	      max_to_penalty = dep_pclat->to_penalty;
	      min_to_penalty = dep_pclat->to_penalty;

	      for (dep_pclat = dep_pclat->next_dep_pclat; dep_pclat != NULL;
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
	      SM_print_oper (stdout, to_action->sm_op);
	      printf ("max_from_penalty: %i\n", max_from_penalty);
	      printf ("min_from_penalty: %i\n", min_from_penalty);
	      printf ("max_to_penalty: %i\n", max_to_penalty);
	      printf ("min_to_penalty: %i\n", min_to_penalty);
#endif
	    }
	}

      /* If the from_op is scheduled, use actual late use times */
      if (from_op->flags & SM_OP_SCHEDULED)
        {
          max_late_use_time = from_action->actual_late_use_time;
          min_late_use_time = max_late_use_time;
        }

      /* Otherwise, use the min and max late use times */
      else
        {
          max_late_use_time = from_action->max_late_use_time;
          min_late_use_time = from_action->min_late_use_time;
        }

      if (to_op->flags & SM_OP_SCHEDULED)
        {
          max_early_use_time = to_action->actual_early_use_time;
          min_early_use_time = max_early_use_time;
        }
      else
        {
          max_early_use_time = to_action->max_early_use_time;
          min_early_use_time = to_action->min_early_use_time;
        }

      /* Use action's use times (plus the delay_offset) to calculate
       * the min and max delays
       * 
       * The min delay is earliest def time - latest use time.
       * The max delay is latest def time - earliest use time.
       */
      min_delay = delay_offset + min_late_use_time - max_early_use_time;
      max_delay = delay_offset + max_late_use_time - min_early_use_time;

      /* 20020808 SZU
       * This prod_cons have penalty latency.
       * And it applies to this set of actions (operands).
       */
      if (max_from_penalty && max_to_penalty)
	{
	  min_delay += min_from_penalty;
	  max_delay += max_from_penalty;
	}
    }

  /* Make sure executed one of the above two cases */
  else
    {
      L_punt ("SM_add_dep: Must flag as either SM_FIXED_DELAY or"
              " SM_MDES_BASED_DELAY!");
    }

  /* Prevent negative delays (don't allow non-interlocking machine tricks) */
  if (min_delay < 0)
    min_delay = 0;
  if (max_delay < 0)
    max_delay = 0;

  dep->min_delay = min_delay;
  dep->max_delay = max_delay;

  /* If the min and max delay are not the same, flag that the delay
   * is variable.
   */
  if (min_delay != max_delay)
    dep_flags |= SM_VARIABLE_DELAY;

  /* Initialize the other info fields */
  dep->from_action = from_action;
  dep->to_action = to_action;
  dep->flags = dep_flags;
  dep->ignore = ignore;
  dep->delay_offset = delay_offset;
  dep->omega = omega;

  /* Add to front of from_action's dep_out list */
  if (from_action->first_dep_out != NULL)
    from_action->first_dep_out->prev_dep_out = dep;
  dep->next_dep_out = from_action->first_dep_out;
  dep->prev_dep_out = NULL;
  from_action->first_dep_out = dep;


  /* Add to fron of to_action's dep_in list */
  if (to_action->first_dep_in != NULL)
    to_action->first_dep_in->prev_dep_in = dep;
  dep->next_dep_in = to_action->first_dep_in;
  dep->prev_dep_in = NULL;
  to_action->first_dep_in = dep;

  /* Update from oper's dep counts and bounds if necessary */
  if (ignore)
    {
      from_op->num_ignore_dep_out++;
      to_op->num_ignore_dep_in++;
    }
  else if (dep_flags & SM_HARD_DEP)
    {
      from_op->num_hard_dep_out++;
      to_op->num_hard_dep_in++;
    }
  else if (dep_flags & SM_SOFT_DEP)
    {
      from_op->num_soft_dep_out++;
      to_op->num_soft_dep_in++;
    }

  /* Get max slot for ease of use */
  max_possible_slot = SM_MAX_SLOT;

  /* If to_op scheduled, update from_op's cycle and slot upper bound
   * (if necessary)
   */
  if (to_op->flags & SM_OP_SCHEDULED)
    {
      /* Update bounds only if operation not ignored -ITI/JCG 8/99 */
      if (!ignore)
        {
          SM_update_upper_bounds (dep, dep_flags);
        }
    }
  /* Otherwise, increment unresolved counts  */
  else
    {
      if (ignore)
        {
          from_op->num_unresolved_ignore_dep_out++;
        }
      else if (dep_flags & SM_HARD_DEP)
        {
          from_op->num_unresolved_hard_dep_out++;
        }
      else
        {
          from_op->num_unresolved_soft_dep_out++;
        }
    }

  /* If from_op scheduled, update to_op's lower bounds if necessary */
  if (from_op->flags & SM_OP_SCHEDULED)
    {
      /* Update bounds only if operation not ignored -ITI/JCG 8/99 */
      if (!ignore)
        {
          SM_update_lower_bounds (dep, dep_flags);
        }
    }
  /* Otherwise, increment unresolved counts and remove from
   * dep_in_resolved queue (if necessary).
   */
  else
    {
      if (ignore)
        {
          to_op->num_unresolved_ignore_dep_in++;
        }
      else if (dep_flags & SM_HARD_DEP)
        {
          to_op->num_unresolved_hard_dep_in++;
        }
      else
        {
          to_op->num_unresolved_soft_dep_in++;
        }

      if (to_op->dep_in_resolved_qentry != NULL)
        {
          SM_dequeue_oper (to_op->dep_in_resolved_qentry);
          to_op->dep_in_resolved_qentry = NULL;
        }
    }

  /* If omega is zero, then test to make sure we have not drawn a
   * dependence that is not legal.
   */
  if (omega == 0)
    {
      /* Make sure from_op has a serial_number before to_op!  
       * (This is a strong indication that the parm order was messed up!)
       */
      if (from_op->serial_number > to_op->serial_number)
        {
          fprintf (stderr,
                   "SM_add_dep: Adding zero-omega dep the wrong way!\n");
          fprintf (stderr,
                   "  from_op->serial_number=%i to_op->serial_number=%i\n",
                   from_op->serial_number, to_op->serial_number);
          fprintf (stderr,
                   "  Check parameter order in call to SM_add_dep() for:\n");
          fprintf (stderr, "   ");
          SM_print_dep (stderr, dep);
          fprintf (stderr, "\n");
        }
    }

#if 0
  /* Debug */
  if ((from_op->lcode_op->id == 33) && (to_op->lcode_op->id == 37) &&
      (dep->flags & SM_ANTI_DEP))
    {
      SM_print_reg_info (stderr, from_action->rinfo);
      SM_print_dep (stderr, dep);
    }
#endif
}

/* 
 * SM_enable_ignored_dep
 */
void
SM_enable_ignored_dep (SM_Dep * dep, int flag)
{
  SM_Oper *from_op, *to_op;
  unsigned int old_flags;
  short max_possible_slot;

  /* Cache from and to opers, and dep flags,  for ease of use */
  from_op = dep->from_action->sm_op;
  to_op = dep->to_action->sm_op;
  old_flags = dep->flags;

  /* If dependence is not ignored with this flag, nothing to do. */
  if (!(dep->ignore & flag))
    {
      return;
    }

  /* Don't enable on this flag if to_op or from_op are not both
   * enabled on this flag */

  if (to_op->ignore & flag || from_op->ignore & flag)
    {
      L_punt ("SM_enable_ignored_dep: Can't enable dep when ops are "
              "not already enabled.");
      return;
    }

  dep->ignore &= ~flag;

  /* If dependence still ignored, nothing else to do */
  if (dep->ignore)
    {
      return;
    }

  from_op->num_ignore_dep_out--;
  to_op->num_ignore_dep_in--;
  if (old_flags & SM_HARD_DEP)
    {
      from_op->num_hard_dep_out++;
      to_op->num_hard_dep_in++;
    }
  else if (old_flags & SM_SOFT_DEP)
    {
      from_op->num_soft_dep_out++;
      to_op->num_soft_dep_in++;
    }
  else
    L_punt ("SM_enable_ignored_dep: dependence must be HARD or SOFT.");


  /* Get max slot for ease of use */
  max_possible_slot = SM_MAX_SLOT;

  /* If to_op scheduled, update from_op's cycle and slot upper bound
   * (if necessary)
   */
  if (to_op->flags & SM_OP_SCHEDULED)
    {
      SM_update_upper_bounds (dep, old_flags);
    }
  /* Otherwise, increment unresolved counts  */
  else
    {
      /* Debug */
      if (from_op->num_unresolved_ignore_dep_out == 0)
        {
          L_punt ("SM_enable_ignored_dep: %s op %i: "
                  "bad num_unresolved_ignore_dep_out",
                  to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
        }
      from_op->num_unresolved_ignore_dep_out--;
      if (old_flags & SM_HARD_DEP)
        {
          from_op->num_unresolved_hard_dep_out++;
        }
      else if (old_flags & SM_SOFT_DEP)
        {
          from_op->num_unresolved_soft_dep_out++;
        }
    }

  /* If from_op scheduled, update to_op's lower bounds if necessary */
  if (from_op->flags & SM_OP_SCHEDULED)
    {
      SM_update_lower_bounds (dep, old_flags);
    }
  /* Otherwise, increment unresolved counts and remove from
   * dep_in_resolved queue (if necessary).
   */
  else
    {
      /* Debug */
      if (to_op->num_unresolved_ignore_dep_in == 0)
        {
          L_punt ("SM_enable_ignored_dep: %s op %i: "
                  "bad num_unresolved_ignore_dep_in",
                  to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
        }
      to_op->num_unresolved_ignore_dep_in--;
      if (old_flags & SM_HARD_DEP)
        {
          to_op->num_unresolved_hard_dep_in++;
        }
      else if (old_flags & SM_SOFT_DEP)
        {
          to_op->num_unresolved_soft_dep_in++;
        }

      if (to_op->dep_in_resolved_qentry != NULL)
        {
          SM_dequeue_oper (to_op->dep_in_resolved_qentry);
          to_op->dep_in_resolved_qentry = NULL;
        }
    }
}



/* 
 * SM_ignore_dep
 */
void
SM_ignore_dep (SM_Dep * dep, int flag)
{
  SM_Oper *from_op, *to_op;
  unsigned int old_flags;

  /* Cache from and to opers, and dep flags,  for ease of use */
  from_op = dep->from_action->sm_op;
  to_op = dep->to_action->sm_op;
  old_flags = dep->flags;

  /* If dependence is already ignored, set flag and nothing else to do. */
  if (dep->ignore)
    {
      dep->ignore |= flag;
      return;
    }
  dep->ignore |= flag;

  /* Update ignore dependence counts */
  from_op->num_ignore_dep_out++;
  to_op->num_ignore_dep_in++;

  /* Update hard/soft dependence counts */
  if (old_flags & SM_HARD_DEP)
    {
      from_op->num_hard_dep_out--;
      to_op->num_hard_dep_in--;
    }
  else if (old_flags & SM_SOFT_DEP)
    {
      from_op->num_soft_dep_out--;
      to_op->num_soft_dep_in--;
    }
  else
    L_punt ("SM_ignore_dep: dependence must be HARD or SOFT.");

  /* If to_op is scheduled, update cycle and slot bounds 
   * of the from_op, if necessary 
   */
  if (to_op->flags & SM_OP_SCHEDULED)
    {
      /* Recalculate the from_op's upper bound if this dep caused
       * the from_op's upper bound.
       */
      if (from_op->dep_upper_bound == dep)
        SM_recalculate_upper_bound (from_op);
    }
  /* Otherwise, adjust unscheduled count */
  else
    {
      from_op->num_unresolved_ignore_dep_out++;
      if (old_flags & SM_HARD_DEP)
        {
          /* Debug */
          if (from_op->num_unresolved_hard_dep_out == 0)
            {
              L_punt ("SM_ignore_dep: %s op %i: "
                      "bad num_unresolved_hard_dep_out",
                      to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
            }
          from_op->num_unresolved_hard_dep_out--;
        }
      else if (old_flags & SM_SOFT_DEP)
        {
          /* Debug */
          if (from_op->num_unresolved_soft_dep_out == 0)
            {
              L_punt ("SM_ignore_dep: %s op %i: "
                      "bad num_unresolved_soft_dep_out",
                      to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
            }
          from_op->num_unresolved_soft_dep_out--;
        }
    }

  /* If from_op scheduled, update cycle and slot bounds of
   * to_op if necessary 
   */
  if (from_op->flags & SM_OP_SCHEDULED)
    {
      /* Recaculate the to_op's lower bound if this dep caused it */
      if (to_op->dep_lower_bound == dep)
        SM_recalculate_lower_bound (to_op);
    }
  /* Otherwise, scheduled unscheduled count */
  else
    {
      to_op->num_unresolved_ignore_dep_in++;
      if (old_flags & SM_HARD_DEP)
        {
          /* Debug */
          if (to_op->num_unresolved_hard_dep_in == 0)
            {
              L_punt ("SM_ignore_dep: %s op %i: "
                      "bad num_unresolved_hard_dep_in",
                      to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
            }
          to_op->num_unresolved_hard_dep_in--;
        }
      else if (old_flags & SM_SOFT_DEP)
        {
          /* Debug */
          if (to_op->num_unresolved_soft_dep_in == 0)
            {
              L_punt ("SM_ignore_dep: %s op %i: "
                      "bad num_unresolved_soft_dep_in",
                      to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
            }
          to_op->num_unresolved_soft_dep_in--;
        }

      /* If this operation now has all of its incoming dependences resolved,
       * then place it at the end of the dep_in_resolved queue if
       * the operation is not scheduled and not already in the queue.
       * (If just deleted a soft dependence, it may already be in the queue.)
       */
      if ((to_op->num_unresolved_hard_dep_in == 0) &&
          /* EMN (to_op->num_unresolved_soft_dep_in == 0) && */
          (to_op->dep_in_resolved_qentry == NULL) &&
          !(to_op->flags & SM_OP_SCHEDULED) && !to_op->ignore)
        {
          to_op->dep_in_resolved_qentry =
            SM_enqueue_oper_before (to_op->sm_cb->dep_in_resolved,
                                    to_op, NULL);
        }
    }
}


void
SM_ignore_dep_out (SM_Oper * sm_op, int flag)
{
  SM_Reg_Action *action;
  SM_Dep *dep_out;

  for (action = sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_out = action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          SM_ignore_dep (dep_out, flag);
        }
    }
}

void
SM_enable_ignored_dep_out (SM_Oper * sm_op, int flag)
{
  SM_Reg_Action *action;
  SM_Dep *dep_out;
  SM_Oper *to_op;

  if (sm_op->ignore & flag)
    L_punt ("Cannot enable ignored deps out until op is enabled.");

  for (action = sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_out = action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          to_op = dep_out->to_action->sm_op;

          if (to_op->ignore & flag)
            continue;

          SM_enable_ignored_dep (dep_out, flag);
        }
    }
}

void
SM_ignore_dep_in (SM_Oper * sm_op, int flag)
{
  SM_Reg_Action *action;
  SM_Dep *dep_in;

  for (action = sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_in = action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          SM_ignore_dep (dep_in, flag);
        }
    }
}

void
SM_enable_ignored_dep_in (SM_Oper * sm_op, int flag)
{
  SM_Reg_Action *action;
  SM_Dep *dep_in;
  SM_Oper *from_op;

  if (sm_op->ignore & flag)
    L_punt ("Cannot enable ignored deps out until op is enabled.");

  for (action = sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_in = action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          from_op = dep_in->from_action->sm_op;

          if (from_op->ignore & flag)
            continue;

          SM_enable_ignored_dep (dep_in, flag);
        }
    }
}

void
SM_delete_dep (SM_Dep * dep)
{
  SM_Oper *from_op, *to_op;
  unsigned int flags;
  SM_Dep_PCLat *dep_pclat, *next_dep_pclat;

  /* Remove dependence from the from_action's dep_out list */
  if (dep->prev_dep_out != NULL)
    dep->prev_dep_out->next_dep_out = dep->next_dep_out;
  else
    dep->from_action->first_dep_out = dep->next_dep_out;

  if (dep->next_dep_out != NULL)
    dep->next_dep_out->prev_dep_out = dep->prev_dep_out;

  /* Remove dependence from the to_action's dep_in list */
  if (dep->prev_dep_in != NULL)
    dep->prev_dep_in->next_dep_in = dep->next_dep_in;
  else
    dep->to_action->first_dep_in = dep->next_dep_in;

  if (dep->next_dep_in != NULL)
    dep->next_dep_in->prev_dep_in = dep->prev_dep_in;

  /* Cache from and to opers, and dep flags,  for ease of use */
  from_op = dep->from_action->sm_op;
  to_op = dep->to_action->sm_op;
  flags = dep->flags;

  /* Update dependence counts */
  if (dep->ignore)
    {
      from_op->num_ignore_dep_out--;
      to_op->num_ignore_dep_in--;
    }
  else if (flags & SM_HARD_DEP)
    {
      from_op->num_hard_dep_out--;
      to_op->num_hard_dep_in--;
    }
  else
    {
      from_op->num_soft_dep_out--;
      to_op->num_soft_dep_in--;
    }

  /* If to_op is scheduled, update cycle and slot bounds 
   * of the from_op, if necessary 
   */
  if (to_op->flags & SM_OP_SCHEDULED)
    {
      /* Recalculate the from_op's upper bound if this dep caused
       * the from_op's upper bound.
       */
      if (from_op->dep_upper_bound == dep)
        SM_recalculate_upper_bound (from_op);
    }

  /* Otherwise, decrement unscheduled count */
  else
    {
      if (dep->ignore)
        {
          from_op->num_unresolved_ignore_dep_out--;
        }
      else if (flags & SM_HARD_DEP)
        {
          /* Debug */
          if (from_op->num_unresolved_hard_dep_out == 0)
            {
              L_punt ("SM_delete_dep: %s op %i (from_op): "
                      "num_unresolved_hard_dep_out already zero",
                      from_op->sm_cb->lcode_fn->name, from_op->lcode_op->id);
            }
          from_op->num_unresolved_hard_dep_out--;
        }
      else
        {
          /* Debug */
          if (from_op->num_unresolved_soft_dep_out == 0)
            {
              L_punt ("SM_delete_dep: %s op %i (from_op): "
                      "num_unresolved_soft_dep_out already zero",
                      from_op->sm_cb->lcode_fn->name, from_op->lcode_op->id);
            }
          from_op->num_unresolved_soft_dep_out--;
        }
    }

  /* If from_op scheduled, update cycle and slot bounds of
   * to_op if necessary 
   */
  if (from_op->flags & SM_OP_SCHEDULED)
    {
      /* Recaculate the to_op's lower bound if this dep caused it */
      if (to_op->dep_lower_bound == dep)
        SM_recalculate_lower_bound (to_op);
    }

  /* Otherwise, decrement unscheduled count */
  else
    {
      if (dep->ignore)
        {
          to_op->num_unresolved_ignore_dep_in--;
        }
      else if (flags & SM_HARD_DEP)
        {
          /* Debug */
          if (to_op->num_unresolved_hard_dep_in == 0)
            {
              L_punt ("SM_delete_dep: %s op %i (to_op): "
                      "num_unresolved_hard_dep_in already zero",
                      to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
            }
          to_op->num_unresolved_hard_dep_in--;
        }
      else
        {
          /* Debug */
          if (to_op->num_unresolved_soft_dep_in == 0)
            {
              L_punt ("SM_delete_dep: %s op %i (to_op): "
                      "num_unresolved_soft_dep_in already zero",
                      to_op->sm_cb->lcode_fn->name, to_op->lcode_op->id);
            }
          to_op->num_unresolved_soft_dep_in--;
        }

      /* If this operation now has all of its incoming dependences resolved,
       * then place it at the end of the dep_in_resolved queue if
       * the operation is not scheduled and not already in the queue.
       * (If just deleted a soft dependence, it may already be in the queue.)
       */
      if ((to_op->num_unresolved_hard_dep_in == 0) &&
          /* EMN (to_op->num_unresolved_soft_dep_in == 0) && */
          (to_op->dep_in_resolved_qentry == NULL) &&
          !(to_op->flags & SM_OP_SCHEDULED) && !to_op->ignore)
        {
          to_op->dep_in_resolved_qentry =
            SM_enqueue_oper_before (to_op->sm_cb->dep_in_resolved,
                                    to_op, NULL);
        }
    }

  /* 20020824 SZU
   * Free the SM_Dep_PCLat structures
   */
  if (dep->pclat_list != NULL)
    {
      /* Free the dep_pclat list */
      for (dep_pclat = dep->pclat_list; dep_pclat != NULL;
	   dep_pclat = next_dep_pclat)
	{
          /* Get the next dep_pclat before deleting this one */
          next_dep_pclat = dep_pclat->next_dep_pclat;

          /* Delete this dep_pclat */
	  L_free (SM_Dep_PCLat_pool, (void *)dep_pclat);
	}

      dep->pclat_list = NULL;
    }

  /* Free the structures memory */
  L_free (SM_Dep_pool, (void *) dep);
}

int
SM_def_dominates_action (SM_Reg_Action * def_before, SM_Reg_Action * action)
{
  SM_Oper *def_sm_op, *action_sm_op;
  L_Oper *def_lcode_op, *action_lcode_op;

  /* Get sm_ops for actions for ease of use */
  def_sm_op = def_before->sm_op;
  action_sm_op = action->sm_op;

  /* Sanity check on argument order.  (There was subtle bugs in old dep 
   * graph due to incorrect arguement order.)
   * Make sure def_before is a definition.
   */
  if (!(def_before->flags & SM_DEF_ACTION))
    {
      L_punt ("SM_def_dominates_action: def_before must be a def!"
              "  Check arguments!");
    }

  /* Make sure def is not from an op later than the action */
  if (def_sm_op->serial_number > action_sm_op->serial_number)
    {
      L_punt ("SM_def_dominates_action: def_before must occur "
              "before action!  Check arguments!");
    }

  /* Make sure actions are from the same cb */
  if (def_sm_op->sm_cb != action_sm_op->sm_cb)
    {
      L_punt ("SM_def_dominates_action: actions from different cbs!");
    }

  /* Transparent definitions cannot dominate anything. -JCG 7/8/98 */
  if (def_before->flags & SM_TRANSPARENT_ACTION)
    return (0);

  /* If allowing dead Lbx86 defs to reorder, don't allow dead defs to 
   * dominate anything.  This will allow the dependence to be removed 
   * between dead defs while preventing them from reordering with live 
   * defs/uses.
   */
  if (SM_make_Lbx86_assumptions &&
      (!SM_prevent_dead_Lbx86_defs_from_reordering) &&
      SM_Lbx86_def_action_is_dead (def_before))
    {
      return (0);
    }

  /* 
   * For unpredicated code, def_after always post_dominates action.
   * (The code below also must work for unpredicate code also!
   *  This test just prevents illegal access to pred[0].)
   */
  if (L_max_pred_operand <= 0)
    return (1);

  /* For predicate code, SM_PRED_TRANS_DEF can
   * NEVER dominate the action, since it only conditionally 
   * modifies the predicate.
   */
  if (def_before->flags & (SM_PRED_TRANS_DEF))
    return (0);

  /* For predicated code, SM_PRED_UNCOND_DEF always dominates
   * the action, since it always modifies the predicate, even
   * when the operation is predicate squashed.
   */
  if (def_before->flags & SM_PRED_UNCOND_DEF)
    return (1);

  /* Get lcode ops for def and action for ease of use */
  def_lcode_op = def_sm_op->lcode_op;
  action_lcode_op = action_sm_op->lcode_op;

  /* Go to predicate graph to see if def's predicate causes
   * def to dominate the action.
   *
   * The def dominates the action if the action's execution (predicate true)
   * implies the def will have always been executed (predicate true).
   *
   * WAS:  if (PG_subset_predicate_ops(action_lcode_op, def_lcode_op))
   */

  if (((action->flags & SM_DEF_ACTION) &&
       PG_subset_predicate_ops_explicit (action_lcode_op, FALSE,
					 def_lcode_op, FALSE)) ||
      ((action->flags & SM_USE_ACTION) &&
       PG_subset_predicate_ops_explicit (action_lcode_op, TRUE,
					 def_lcode_op, FALSE)))
    {
      return (1);
    }
  else
    {
      return (0);
    }
}

/* Cross-iteration aware domination determination.  Currently
 * very conservative since predicate analysis is not cross-iteration
 * aware. -ITI/JCG 8/99
 */
int
SM_def_dominates_cross_iter_action (SM_Reg_Action * def_after,
                                    SM_Reg_Action * action)
{
  SM_Oper *def_sm_op, *action_sm_op;
  L_Oper *def_lcode_op;

  /* Get sm_ops for actions for ease of use */
  def_sm_op = def_after->sm_op;
  action_sm_op = action->sm_op;

  /* Sanity check on argument order.  
   * Make sure def_after is a definition.
   */
  if (!(def_after->flags & SM_DEF_ACTION))
    {
      L_punt ("SM_def_dominates_cross_iter_action: def_after must be a def!"
              "  Check arguments!");
    }

  /* Make sure def is from an op later than the action (this
   * should be used only for cross iteration checks)
   */
  if (def_sm_op->serial_number < action_sm_op->serial_number)
    {
      L_punt ("SM_def_dominates_cross_iter_action: def_after must occur "
              "after action!  Check arguments!");
    }

  /* Make sure actions are from the same cb */
  if (def_sm_op->sm_cb != action_sm_op->sm_cb)
    {
      L_punt ("SM_def_dominates_cross_iter_action: actions from different "
              "cbs!");
    }

  /* Transparent definitions cannot dominate anything. -JCG 7/8/98 */
  if (def_after->flags & SM_TRANSPARENT_ACTION)
    return (0);

  /* If allowing dead Lbx86 defs to reorder, don't allow dead defs to 
   * dominate anything.  This will allow the dependence to be removed 
   * between dead defs while preventing them from reordering with live 
   * defs/uses.
   */
  if (SM_make_Lbx86_assumptions &&
      (!SM_prevent_dead_Lbx86_defs_from_reordering) &&
      SM_Lbx86_def_action_is_dead (def_after))
    {
      return (0);
    }

  /* 
   * For unpredicated code, def_after always post_dominates action.
   * (The code below also must work for unpredicate code also!
   *  This test just prevents illegal access to pred[0].)
   */
  if (L_max_pred_operand <= 0)
    return (1);

  /* For predicate code, SM_PRED_TRANS_DEF can
   * NEVER dominate the action, since it only conditionally 
   * modifies the predicate.
   */
  if (def_after->flags & SM_PRED_TRANS_DEF)
    return (0);

  /* For predicated code, SM_PRED_UNCOND_DEF always dominates
   * the action, since it always modifies the predicate, even
   * when the operation is predicate squashed.
   */
  if (def_after->flags & SM_PRED_UNCOND_DEF)
    return (1);


  /* Get lcode ops for def and action for ease of use */
  def_lcode_op = def_sm_op->lcode_op;

  /* If def does not have a predicate, always dominates action. */
  if (def_lcode_op->pred[0] == NULL)
    {
      return (1);
    }
  /* Otherwise, we must conservatively assume definition does
   * not dominate action because predicate analysis (or simple
   * deduction using predicate registers) does not currently
   * apply cross-iteration. -ITI/JCG 8/99.
   */
  else
    {
      return (0);
    }
}

int
SM_def_post_dominates_action (SM_Reg_Action * def_after,
                              SM_Reg_Action * action)
{
  SM_Oper *def_sm_op, *action_sm_op;
  L_Oper *def_lcode_op, *action_lcode_op;

  /* Get sm_ops for actions for ease of use */
  def_sm_op = def_after->sm_op;
  action_sm_op = action->sm_op;

  /* Sanity check on argument order.  (There was subtle bugs in old dep 
   * graph due to incorrect arguement order.)
   * Make sure def_after is a definition.
   */
  if (!(def_after->flags & SM_DEF_ACTION))
    {
      L_punt ("SM_def_post_dominates_action: def_after must be a def!"
              "  Check arguments!");
    }

  /* Make sure def is not from an op earlier than the action */
  if (def_sm_op->serial_number < action_sm_op->serial_number)
    {
      L_punt ("SM_def_post_dominates_action: def_after must occur "
              "after action!  Check arguments!");
    }

  /* Make sure actions are from the same cb */
  if (def_sm_op->sm_cb != action_sm_op->sm_cb)
    {
      L_punt ("SM_def_post_dominates_action: actions from different cbs!");
    }

  /* Transparent definitions cannot post dominate anything. -JCG 7/8/98 */
  if (def_after->flags & SM_TRANSPARENT_ACTION)
    return (0);

  /* If allowing dead Lbx86 defs to reorder, don't allow dead defs to 
   * post-dominate anything.  This will allow the dependence to be 
   * removed between dead defs while preventing them from reordering 
   * with live defs/uses.
   */
  if (SM_make_Lbx86_assumptions &&
      (!SM_prevent_dead_Lbx86_defs_from_reordering) &&
      SM_Lbx86_def_action_is_dead (def_after))
    {
      return (0);
    }

  /* 
   * For unpredicated code, def_after always post_dominates action.
   * (The code below also must work for unpredicate code also!
   *  This test just prevents illegal access to pred[0].)
   */
  if (L_max_pred_operand <= 0)
    return (1);

  /* For predicate code, SM_PRED_TRANS_DEF can
   * NEVER post-dominate the action, since it only conditionally 
   * modifies the predicate.
   */
  if (def_after->flags & (SM_PRED_TRANS_DEF))
    return (0);

  /* For predicated code, SM_PRED_UNCOND_DEF always post-dominates
   * the action, since it always modifies the predicate, even
   * when the operation is predicate squashed.
   */
  if (def_after->flags & SM_PRED_UNCOND_DEF)
    return (1);

  /* Get lcode ops for def and action for ease of use */
  def_lcode_op = def_sm_op->lcode_op;
  action_lcode_op = action_sm_op->lcode_op;

  /* If the predicate guarding the def action is true, then post-dominates.
   */
  if (PG_true_predicate_op (def_lcode_op))
    return (1);

  /* Go to predicate graph to see if def's predicate causes
   * def to post-dominate the action.
   *
   * The def post-dominates the action if the action's execution 
   * (predicate true) implies the def will be executed (predicate true).
   */
  /* DIA - was:
   *    if (PG_subset_predicate_ops(action_lcode_op, def_lcode_op))
   */
  if (((action->flags & SM_DEF_ACTION) &&
       !(action->flags & SM_PRED_UNCOND_DEF) &&
       PG_subset_predicate_ops_explicit (action_lcode_op, FALSE,
                                         def_lcode_op, FALSE)) ||
      ((action->flags & SM_USE_ACTION) &&
       !(action->flags & SM_PRED_UNCOND_USE) &&
       PG_subset_predicate_ops_explicit (action_lcode_op, TRUE,
                                         def_lcode_op, FALSE)))
    {
#if 0
      SM_print_oper (stdout, def_sm_op);
      fprintf (stdout, "Post-dominates\n");
      SM_print_oper (stdout, action_sm_op);
      fprintf (stdout, "\n");
#endif
      return (1);
    }
  else
    {
#if 0
      SM_print_oper (stdout, def_sm_op);
      fprintf (stdout, "Does NOT Post-dominate\n");
      SM_print_oper (stdout, action_sm_op);
      fprintf (stdout, "\n");
#endif
      return (0);
    }
}


/* Cross-iteration aware post-domination determination.  Currently
 * very conservative since predicate analysis is not cross-iteration
 * aware. -ITI/JCG 8/99
 */
int
SM_def_post_dominates_cross_iter_action (SM_Reg_Action * def_before,
                                         SM_Reg_Action * action)
{
  SM_Oper *def_sm_op, *action_sm_op;
  L_Oper *def_lcode_op;

  /* Get sm_ops for actions for ease of use */
  def_sm_op = def_before->sm_op;
  action_sm_op = action->sm_op;

  /* Sanity check on argument order.  
   * Make sure def_before is a definition.
   */
  if (!(def_before->flags & SM_DEF_ACTION))
    {
      L_punt ("SM_def_post_dominates_cross_iter_action: def_before must be "
              "a def!  Check arguments!");
    }

  /* Make sure def *is* from an op earlier than the action */
  if (def_sm_op->serial_number > action_sm_op->serial_number)
    {
      L_punt ("SM_def_post_dominates_cross_iter_action: def_before must "
              "occur before action!  Check arguments!");
    }

  /* Make sure actions are from the same cb */
  if (def_sm_op->sm_cb != action_sm_op->sm_cb)
    {
      L_punt ("SM_def_post_dominates_cross_iter_action: actions from "
              "different cbs!");
    }

  /* Transparent definitions cannot post dominate anything. -JCG 7/8/98 */
  if (def_before->flags & SM_TRANSPARENT_ACTION)
    return (0);

  /* If allowing dead Lbx86 defs to reorder, don't allow dead defs to 
   * post-dominate anything.  This will allow the dependence to be 
   * removed between dead defs while preventing them from reordering 
   * with live defs/uses.
   */
  if (SM_make_Lbx86_assumptions &&
      (!SM_prevent_dead_Lbx86_defs_from_reordering) &&
      SM_Lbx86_def_action_is_dead (def_before))
    {
      return (0);
    }

  /* 
   * For unpredicated code, def_before always post_dominates action.
   * (The code below also must work for unpredicate code also!
   *  This test just prevents illegal access to pred[0].)
   */
  if (L_max_pred_operand <= 0)
    return (1);

  /* For predicate code, SM_PRED_TRANS_DEF can
   * NEVER post-dominate the action, since it only conditionally 
   * modifies the predicate.
   */
  if (def_before->flags & SM_PRED_TRANS_DEF)
    return (0);

  /* For predicated code, SM_PRED_UNCOND_DEF always post-dominates
   * the action, since it always modifies the predicate, even
   * when the operation is predicate squashed.
   */
  if (def_before->flags & SM_PRED_UNCOND_DEF)
    return (1);

  /* Get lcode ops for def and action for ease of use */
  def_lcode_op = def_sm_op->lcode_op;

  /* If def does not have a predicate, always post_dominates action. */
  if (def_lcode_op->pred[0] == NULL)
    {
      return (1);
    }

  /* Otherwise, we must conservatively assume definition does
   * not dominate action because predicate analysis (or simple
   * deduction using predicate registers) does not currently
   * apply cross-iteration. -ITI/JCG 8/99.
   */
  else
    {
      return (0);
    }
}

void
SM_build_src_reg_deps (SM_Reg_Action * src_action, unsigned int flags)
{
  SM_Reg_Action **conflict_ptr, *conflict, *def_before, *def_after;
  void *live_expr;
  int reg_live_cross_iter;
  unsigned int cb_flags;
  unsigned int mode;

  /* Cache the src_action's cb_flags for ease of use */
  cb_flags = src_action->sm_op->sm_cb->flags;

  /* Add deps for every register this src_action conflicts with */
  for (conflict_ptr = src_action->conflict;
       (conflict = *conflict_ptr) != NULL; conflict_ptr++)
    {
      /* Build the dep in list for this action if necessary */
      if (flags & SM_BUILD_DEP_IN)
        {
	  SM_logexpr_init (&live_expr, src_action);

          /* Assume register live cross-iteration until find out
           * otherwise-ITI/JCG 8/99
           */
          reg_live_cross_iter = 1;

          /* Draw reg flow dependence(s) from definition(s) before
           * this register use, until hit a definition that dominates
           * this register's usage.  This will always be the first
           * def for unpredicated code.
           *
           * Use Mdes information to set delay (no offset) if actions
           * are explicit.  If one action is explicit, draw zero-cycle dep.
           *
           * For efficiencies sake, if neither are explicit, don't 
           * draw an dependence.
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (def_before = conflict->prev_def; def_before;
               def_before = def_before->prev_def)
            {
              /* If both actions are explicit, draw normal flow dep */
              if ((def_before->flags & SM_EXPLICIT_ACTION) &&
                  (src_action->flags & SM_EXPLICIT_ACTION))
                {
                  SM_add_dep (def_before, src_action,
                              flags | SM_MDES_BASED_DELAY |
                              SM_REG_DEP | SM_FLOW_DEP, 0, 
			      def_before->add_lat, 0, mode);
                }

              /* If one action is explicit, draw zero-cycle flow dep */
              else if ((def_before->flags & SM_EXPLICIT_ACTION) ||
                       (src_action->flags & SM_EXPLICIT_ACTION))
                {
                  SM_add_dep (def_before, src_action,
                              flags | SM_FIXED_DELAY |
                              SM_REG_DEP | SM_FLOW_DEP, 0, 0, 0, mode);
                }
              /* If neither action is explicit, don't draw a dep at all */
              /* Stop hard deps when reach a definition that 
		 dominates the src_action */
              if (SM_def_dominates_action (def_before, src_action) ||
		  !SM_logexpr_sub_accum_def (&live_expr, def_before))
                {
                  /* Register not live cross iteration -ITI/JCG 8/99 */
                  reg_live_cross_iter = 0;
		  mode = SM_SOFT_DEP;
                }
            }

	  if (SM_logexpr_ne(live_expr))
	    src_action->flags |= SM_LIVEIN_ACTION;

	  SM_logexpr_dispose (&live_expr);

          /* Draw cross iteration dependences if cb flag set and
           * register live around backedge.
           *
           * This assumes that the last operation in the cb is
           * the backedge and the only backedge. -ITI/JCG 8/99
           */
          if ((cb_flags & SM_CROSS_ITERATION) && reg_live_cross_iter)
            {
              /* Draw cross-iteration reg flow dependence(s) from 
               * definition(s) before this register use (around the
               * backedge), until hit a definition that dominates 
               * (cross-iteration) this register's usage or until we
               * reach this use (includes this op's dest).  -ITI/JCG 8/99
               *
               * Use Mdes information to set delay (no offset) if actions
               * are explicit.  If one action is explicit, 
               * draw zero-cycle dep.
               *
               * For efficiencies sake, if neither are explicit, don't 
               * draw an dependence.
               */
	      mode = SM_HARD_DEP | SM_SOFT_DEP;
              for (def_before = conflict->rinfo->last_def;
                   ((def_before != NULL) &&
                    (def_before->sm_op->serial_number >=
                     src_action->sm_op->serial_number));
                   def_before = def_before->prev_def)
                {
                  /* If both actions are explicit, draw normal flow dep */
                  if ((def_before->flags & SM_EXPLICIT_ACTION) &&
                      (src_action->flags & SM_EXPLICIT_ACTION))
                    {
                      SM_add_dep (def_before, src_action,
                                  flags | SM_MDES_BASED_DELAY |
                                  SM_REG_DEP | SM_FLOW_DEP, 0, 
				  def_before->add_lat, 1, mode);
                    }

                  /* If one action is explicit, draw zero-cycle flow dep */
                  else if ((def_before->flags & SM_EXPLICIT_ACTION) ||
                           (src_action->flags & SM_EXPLICIT_ACTION))
                    {
                      SM_add_dep (def_before, src_action,
                                  flags | SM_FIXED_DELAY |
                                  SM_REG_DEP | SM_FLOW_DEP, 0, 0, 1, mode);
                    }
                  /* If neither action is explicit, don't draw a dep at all */

                  /* Stop hard deps when reach a definition that dominates the 
                   * src_action (must use cross-iter version) -ITI/JCG 8/99
		   */
                  if (SM_def_dominates_cross_iter_action (def_before,
                                                          src_action))
                    {
		      mode = SM_SOFT_DEP;
                    }
                }
            }
        }

      /* Build the dep out list for this action if necessary */
      if (flags & SM_BUILD_DEP_OUT)
        {
	  /* Assume register live cross-iteration until find out
           * otherwise-ITI/JCG 8/99
           */
          reg_live_cross_iter = 1;

          /* Draw reg anti-dependence(s) to definition(s) after this
           * register use, until hit a definition that post-dominates
           * the register's usage.  For unpredicate code, this will
           * always be the first definition.
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (def_after = conflict->next_def; def_after != NULL;
               def_after = def_after->next_def)
            {
              /* Draw reg anti dep with fixed delay of 0 */
              SM_add_dep (src_action, def_after,
                          flags | SM_FIXED_DELAY |
                          SM_REG_DEP | SM_ANTI_DEP, 0, 0, 0, mode);

              /* Can stop hard deps when hit def that post-dominates action */
              if (SM_def_post_dominates_action (def_after, src_action))
                {
                  /* Register not live cross iteration -ITI/JCG 8/99 */
                  reg_live_cross_iter = 0;
		  mode = SM_SOFT_DEP;
                }
            }

          /* Draw cross iteration dependences if cb flag set and
           * register live around backedge.
           *
           * This assumes that the last operation in the cb is
           * the backedge and the only backedge. -ITI/JCG 8/99
           */
          if ((cb_flags & SM_CROSS_ITERATION) && reg_live_cross_iter)
            {
              /* Draw reg anti-dependence(s) to definition(s) after this
               * register use around the backedge, until hit a definition 
               * that post-dominates (cross-iteration) the register's usage
               * or we reach this use (do not include this op's dest).  
               */
	      mode = SM_HARD_DEP | SM_SOFT_DEP;
              for (def_after = conflict->rinfo->first_def;
                   ((def_after != NULL) &&
                    (def_after->sm_op->serial_number <
                     src_action->sm_op->serial_number));
                   def_after = def_after->next_def)
                {
                  /* Draw reg anti dep with fixed delay of 0 */
                  SM_add_dep (src_action, def_after,
                              flags | SM_FIXED_DELAY |
                              SM_REG_DEP | SM_ANTI_DEP, 0, 0, 1, mode);

                  /* Can stop hard deps when hit def that post-dominates action
                   * (must use cross-iteration version) -ITI/JCG 8/99
                   */
                  if (SM_def_post_dominates_cross_iter_action (def_after,
                                                               src_action))
                    {
		      mode = SM_SOFT_DEP;
                    }
                }
            }
        }
    }
}

void
SM_build_dest_reg_deps (SM_Reg_Action * dest_action, unsigned int flags)
{
  SM_Reg_Action **conflict_ptr, *conflict, *action_before, *action_after;
  SM_Oper *dest_op;
  unsigned int cb_flags;
  int reg_live_cross_iter;
  unsigned int mode;

  /* Cache the dest_action's sm_op and cb_flags for ease of use */
  dest_op = dest_action->sm_op;
  cb_flags = dest_op->sm_cb->flags;

  /* Add deps for every register this dest_action conflicts with */
  for (conflict_ptr = dest_action->conflict;
       (conflict = *conflict_ptr) != NULL; conflict_ptr++)
    {

      /* Build the dep in list for this action if necessary */
      if (flags & SM_BUILD_DEP_IN)
        {
          /* Assume register live cross-iteration until find out 
           * otherwise-ITI/JCG 8/99
           */
          reg_live_cross_iter = 1;

          /* Draw a reg anti dependence to every source register 
           * until hit a previous definition for this register 
           * that dominates this definition.
           *
           * Draw output dependences to other defs we see along
           * the way.
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (action_before = conflict->prev_actual;
               action_before != NULL;
               action_before = action_before->prev_actual)
            {
              /* If a use of this register, draw anti dependence.
               * The delay will always be zero.
               */
              if (action_before->flags & SM_USE_ACTION)
                {
                  SM_add_dep (action_before, dest_action,
                              flags | SM_FIXED_DELAY |
                              SM_REG_DEP | SM_ANTI_DEP, 0, 0, 0, mode);
                }

              /* Otherwise, if a def of this register, draw output 
               * dependence.  Stop the scan if it dominates this def.
               * This delay will also always be zero.
               */
              else
                {
                  /* To reduce the dependence graph size, don't draw
                   * output dependences to implicit dests (since there
                   * will always be a matching implicit src to get an anti
                   * dep.)
                   *
                   * Also, don't draw output dependences between 
                   * SM_PRED_TRANS_DEF and other
                   * SM_PRED_TRANS_DEF actions.  These
                   * definitions may be reordered.
                   */
                  if ((dest_action->flags & SM_EXPLICIT_ACTION) &&
                      !((dest_action->flags & SM_PRED_TRANS_DEF) &&
                        (action_before->flags & SM_PRED_TRANS_DEF)))
                    {
                      SM_add_dep (action_before, dest_action,
                                  flags | SM_FIXED_DELAY |
                                  SM_REG_DEP | SM_OUTPUT_DEP, 0,
                                  SM_output_dep_distance, 0, mode);
                    }

                  /* Stop the hard deps if the previous def dominates this def
		   */
                  if (SM_def_dominates_action (action_before, dest_action))
                    {
                      /* Register not live cross iteration -ITI/JCG 8/99 */
                      reg_live_cross_iter = 0;
		      mode = SM_SOFT_DEP;
                    }
                }
            }

          /* Draw cross iteration dependences if cb flag set and
           * register live around backedge.
           *
           * This assumes that the last operation in the cb is
           * the backedge and the only backedge. -ITI/JCG 8/99
           */
          if ((cb_flags & SM_CROSS_ITERATION) && reg_live_cross_iter)
            {
              /* Draw a cross-iteration reg anti dependence to every source 
               * register around the backedge until we reach this 
               * definition (do not include this definition) or 
               * a definition that post-dominates (cross-iteration) 
               * this def. -ITI/JCG 8/99
               *
               * Draw output dependences to other defs we see along
               * the way.
               */
	      mode = SM_HARD_DEP | SM_SOFT_DEP;
              for (action_before = conflict->rinfo->last_actual;
                   ((action_before != NULL) &&
                    (action_before->sm_op->serial_number >
                     dest_op->serial_number));
                   action_before = action_before->prev_actual)
                {
                  /* If a use of this register, draw anti dependence.
                   * The delay will always be zero.
                   */
                  if (action_before->flags & SM_USE_ACTION)
                    {
                      SM_add_dep (action_before, dest_action,
                                  flags | SM_FIXED_DELAY |
                                  SM_REG_DEP | SM_ANTI_DEP, 0, 0, 1, mode);
                    }

                  /* Otherwise, if a def of this register, draw output 
                   * dependence.  Stop the scan if it dominates this def.
                   * This delay will also always be zero.
                   */
                  else
                    {
                      /* To reduce the dependence graph size, don't draw
                       * output dependences to implicit dests (since there
                       * will always be a matching implicit src to get an 
                       * anti dep.)
                       *
                       * Also, don't draw output dependences between 
                       * SM_PRED_TRANS_DEF and other
                       * SM_PRED_TRANS_DEF actions.  These
                       * definitions may be reordered.
                       */
                      if ((dest_action->flags & SM_EXPLICIT_ACTION) &&
                          !((dest_action->flags & SM_PRED_TRANS_DEF) &&
                            (action_before->flags & SM_PRED_TRANS_DEF)))
                        {
                          SM_add_dep (action_before, dest_action,
                                      flags | SM_FIXED_DELAY |
                                      SM_REG_DEP | SM_OUTPUT_DEP, 0,
                                      SM_output_dep_distance, 1, mode);
                        }

                      /* Stop the hard deps if the previous def dominates this 
                       * def (must use cross-iteration version)-ITI/JCG 8/99
                       */
                      if (SM_def_dominates_cross_iter_action (action_before,
                                                              dest_action))
                        {
			  mode = SM_SOFT_DEP;
                        }
                    }
                }
            }
        }

      /* Build the dep out list for this action if necessary */
      if (flags & SM_BUILD_DEP_OUT)
        {
	  void *live_expr;

	  SM_logexpr_init (&live_expr, dest_action);

          /* Assume register live cross-iteration until find out 
           * otherwise-ITI/JCG 8/99
           */
          reg_live_cross_iter = 1;

          /* Draw a reg flow dependence to every source register 
           * until hit a definition afterwards that post-dominates
           * this def.
           *
           * Draw output dependences to other defs we see along
           * the way.
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (action_after = conflict->next_actual;
               action_after != NULL; action_after = action_after->next_actual)
            {
              /* If a use of this register, draw flow dependence.
               * If both actions are explicit, the delay will be mdes 
               * based with zero offset.  If one action is explicit,
               * then a zero-cycle flow dep will be draw.  If neither
               * actions are explicit, don't draw the dep.
               */
              if (action_after->flags & SM_USE_ACTION)
                {
                  /* Draw mdes-based flow dep if both actions are explicit */
                  if ((dest_action->flags & SM_EXPLICIT_ACTION) &&
                      (action_after->flags & SM_EXPLICIT_ACTION))
                    {
                      SM_add_dep (dest_action, action_after,
                                  flags | SM_MDES_BASED_DELAY |
                                  SM_REG_DEP | SM_FLOW_DEP, 0, 
				  dest_action->add_lat, 0, mode);
                    }

                  /* Draw zero-cycle flow dep if one action is explicit */
                  else if ((dest_action->flags & SM_EXPLICIT_ACTION) ||
                           (action_after->flags & SM_EXPLICIT_ACTION))
                    {
                      SM_add_dep (dest_action, action_after,
                                  flags | SM_FIXED_DELAY |
                                  SM_REG_DEP | SM_FLOW_DEP, 0, 0, 0, mode);
                    }
                  /* Otherwise, don't draw a flow dep */
                }

              /* Otherwise, if a def of this register, draw output 
               * dependence and stop scan.
               * This delay will also always be zero.
               */
              else
                {
                  /* To reduce the dependence graph size, don't draw
                   * output dependences to implicit dests (since there
                   * will always be a matching implicit src to get an anti
                   * dep.)
                   *
                   * Also, don't draw output dependences between 
                   * SM_PRED_TRANS_DEF and other
                   * SM_PRED_TRANS_DEF actions.  These
                   * definitions may be reordered.
                   */
                  if ((action_after->flags & SM_EXPLICIT_ACTION) &&
                      !((action_after->flags & SM_PRED_TRANS_DEF) &&
                        (dest_action->flags & SM_PRED_TRANS_DEF)))
                    {
                      SM_add_dep (dest_action, action_after,
                                  flags | SM_FIXED_DELAY |
                                  SM_REG_DEP | SM_OUTPUT_DEP, 0,
                                  SM_output_dep_distance, 0, mode);
                    }

                  /* Stop the hard deps if this def post-dominates the 
		   * dest_action */
                  if (SM_def_post_dominates_action (action_after,
                                                    dest_action) ||
		      !SM_logexpr_sub_accum_def (&live_expr, action_after))
                    {
                      /* Register not live cross iteration -ITI/JCG 8/99 */
                      reg_live_cross_iter = 0;
		      mode = SM_SOFT_DEP;
                    }
                }
            }

	  SM_logexpr_dispose (&live_expr);

          /* Draw cross iteration dependences if cb flag set and
           * register live around backedge.
           * This assumes that the last operation in the cb is
           * the backedge and the only backedge. -ITI/JCG 8/99
           */
          if ((cb_flags & SM_CROSS_ITERATION) && reg_live_cross_iter)
            {
              /* Draw a cross-iteration reg flow dependence to every 
               * source register around the backedge until we reach
               * this definition (include the definition) or reach a 
               * definition afterwards that post-dominates
               * (cross-iteration) this def. -ITI/JCG 8/99 
               *
               * Draw output dependences to other defs we see along
               * the way.
               */
	      mode = SM_HARD_DEP | SM_SOFT_DEP;
              for (action_after = conflict->rinfo->first_actual;
                   ((action_after != NULL) &&
                    (action_after->sm_op->serial_number <=
                     dest_op->serial_number));
                   action_after = action_after->next_actual)
                {
                  /* If a use of this register, draw flow dependence.
                   * If both actions are explicit, the delay will be mdes 
                   * based with zero offset.  If one action is explicit,
                   * then a zero-cycle flow dep will be draw.  If neither
                   * actions are explicit, don't draw the dep.
                   */
                  if (action_after->flags & SM_USE_ACTION)
                    {
                      /* Draw mdes-based flow dep if both actions are
                       * explicit 
                       */
                      if ((dest_action->flags & SM_EXPLICIT_ACTION) &&
                          (action_after->flags & SM_EXPLICIT_ACTION))
                        {
                          SM_add_dep (dest_action, action_after,
                                      flags | SM_MDES_BASED_DELAY |
                                      SM_REG_DEP | SM_FLOW_DEP, 0, 
				      dest_action->add_lat, 1, mode);
                        }

                      /* Draw zero-cycle flow dep if one action 
                       * is explicit
                       */
                      else if ((dest_action->flags & SM_EXPLICIT_ACTION) ||
                               (action_after->flags & SM_EXPLICIT_ACTION))
                        {
                          SM_add_dep (dest_action, action_after,
                                      flags | SM_FIXED_DELAY |
                                      SM_REG_DEP | SM_FLOW_DEP, 0, 0, 1, mode);
                        }
                      /* Otherwise, don't draw a flow dep */
                    }

                  /* Otherwise, if a def of this register, draw output 
                   * dependence and stop scan.
                   * This delay will also always be zero.
                   */
                  else
                    {
                      /* To reduce the dependence graph size, don't draw
                       * output dependences to implicit dests (since there
                       * will always be a matching implicit src to get an 
                       * anti dep.)
                       *
                       * Also, don't draw output dependences between 
                       * SM_PRED_TRANS_DEF and other
                       * SM_PRED_TRANS_DEF actions.  These
                       * definitions may be reordered.
                       */
                      if ((action_after->flags & SM_EXPLICIT_ACTION) &&
                          !((action_after->flags & SM_PRED_TRANS_DEF) &&
                            (dest_action->flags & SM_PRED_TRANS_DEF)))
                        {
                          SM_add_dep (dest_action, action_after,
                                      flags | SM_FIXED_DELAY |
                                      SM_REG_DEP | SM_OUTPUT_DEP, 0,
                                      SM_output_dep_distance, 1, mode);
                        }

                      /* Stop the hard deps if def post-dominates the
		       * dest_action (must use cross-iteration version)
		       * -ITI/JCG 8/99
                       */
                      if (SM_def_post_dominates_cross_iter_action
                          (action_after, dest_action))
                        {
			  mode = SM_SOFT_DEP;
                        }
                    }
                }
            }
        }
    }
}

/* Rebuilds dependences for a dest reg action by deleting all current
 * dependences and then rebuilding them using SM_build_dest_reg_deps.
 * The flags SM_BUILD_DEP_IN and SM_BUILD_DEP_OUT control which
 * dependences are rebuilt.
 */
void
SM_rebuild_dest_reg_deps (SM_Reg_Action * dest_action, unsigned int flags)
{
  SM_Dep *dep_in, *next_dep_in;
  SM_Dep *dep_out, *next_dep_out;

  /* Delete existing incoming dependences, if necessary */
  if (flags & SM_BUILD_DEP_IN)
    {
      for (dep_in = dest_action->first_dep_in; dep_in != NULL;
           dep_in = next_dep_in)
        {
          /* Get the next dep_in before deleting this one */
          next_dep_in = dep_in->next_dep_in;

          /* Delete this dep */
          SM_delete_dep (dep_in);
        }
    }

  /* Delete existing outgoing dependences, if necessary */
  if (flags & SM_BUILD_DEP_OUT)
    {
      for (dep_out = dest_action->first_dep_out; dep_out != NULL;
           dep_out = next_dep_out)
        {
          /* Get the next dep_out before deleting this one */
          next_dep_out = dep_out->next_dep_out;

          /* Delete this dep */
          SM_delete_dep (dep_out);
        }
    }

  /* Rebuild the dependences just deleted */
  SM_build_dest_reg_deps (dest_action, flags);
}


/* Returns 1 if the two actions can be determined to be independent.
 * Otherwise, returns 0.
 *
 * Handles memory dependences due to loads, stores, and jsrs.
 *
 * Removed third parameter 'flags' since it appears that it must
 * always be set to 'nonloop_carried'. -JCG 8/99.
 *
 * This routine only handles intra-iteration memory dependences.
 * Use SM_independent_cross_iter_memory_actions for cross-iteration
 * dependences.
 */
static int
SM_independent_memory_actions (SM_Reg_Action * def_action,
                               SM_Reg_Action * other_action)
{
  SM_Oper *def_sm_op, *other_sm_op;

  /* Make sure the first action is a def action */
  if (!(def_action->flags & SM_DEF_ACTION))
    L_punt ("SM_independent_memory_actions: def action expected first!");

  /* Get the action's sm_ops for ease of use */
  def_sm_op = def_action->sm_op;
  other_sm_op = other_action->sm_op;

  /* If def_action is a store, determine case where not independent */
  if (def_sm_op->mdes_flags & OP_FLAG_STORE)
    {
      /* If other action loads from or stores to memory */
      if (other_sm_op->mdes_flags & (OP_FLAG_LOAD | OP_FLAG_STORE))
        {
	  if (!L_independent_memory_ops (def_sm_op->sm_cb->lcode_cb,
					 def_sm_op->lcode_op,
					 other_sm_op->lcode_op,
					 nonloop_carried))
	    return (0);
        }

      /* If other action is a jsr (may be load and/or store also) */
      if (other_sm_op->mdes_flags & OP_FLAG_JSR)
        {
          /* Use sync arcs for jsrs, if available */
          if (L_use_sync_arcs && L_func_contains_jsr_dep_pragmas &&
              !L_stack_reference (def_sm_op->lcode_op))
            {
              /* If the sync arcs say there is a dependence, return 0 */
              if (!L_sync_no_jsr_dependence (other_sm_op->lcode_op,
                                             def_sm_op->lcode_op))
		return (0);
            }
          /* Otherwise by default stores may not move past JSRs */
          else
            {
              return (0);
            }
        }

      /* Never let stores move past checks */
      if (other_sm_op->mdes_flags & OP_FLAG_CHK)
        return (0);
    }

  /* If def_action is a jsr (may also be a store!), determine cases
   * where not independent.
   */
  if (def_sm_op->mdes_flags & OP_FLAG_JSR)
    {
      /* If the other_action is also for a jsr, assume they are dependent! */
      if (other_sm_op->mdes_flags & OP_FLAG_JSR)
        return (0);

      /* Never let jsrs move past checks */
      if (other_sm_op->mdes_flags & OP_FLAG_CHK)
        return (0);

      /* If other action is a load or store (may also be a jsr) */
      if (other_sm_op->mdes_flags & (OP_FLAG_LOAD | OP_FLAG_STORE))
        {
          /* Use sync arcs for jsrs, if available */
          if (L_use_sync_arcs && L_func_contains_jsr_dep_pragmas &&
              !L_stack_reference (def_sm_op->lcode_op))
            {
              /* If the sync arcs say there is a dependence, return 0 */
              if (!L_sync_no_jsr_dependence (def_sm_op->lcode_op,
                                             other_sm_op->lcode_op))
		return (0);
            }
          else
            {
	      /* Otherwise, use list of side effect free jsrs for
	       * loads.  Never let stores move past jsrs. */
              if ((other_sm_op->mdes_flags & OP_FLAG_STORE) ||
		  !L_side_effect_free_sub_call (def_sm_op->lcode_op))
		return (0);
            }
        }
    }

  /* If have reached here, must be independent */
  return (1);
}

/* Returns 1 if their is no cross iteration memory dependence between
 * from_action and to_action. Otherwise, returns 0 and the distance/omega
 *
 * Requires correct sync-arcs and basically is a wrapper for 
 * L_analyze_syncs_for_cross_iter_independence.  -ITI/JCG 8/99.
 *
 * Note: Due to the way cross-iteration dependences work, 
 *       the dependence usually only goes one way!  So the
 *       order of the actions matters!
 */
static int
SM_independent_cross_iter_memory_actions (SM_Reg_Action * from_action,
                                          SM_Reg_Action * to_action,
                                          int *omega)
{
  SM_Oper *from_sm_op, *to_sm_op;
  int forward, is_indep;

  /* Get the action's sm_ops for ease of use */
  from_sm_op = from_action->sm_op;
  to_sm_op = to_action->sm_op;

  /* Determine if this dependences is a forward dependence.
   * If from_action and to_action are from the same operation,
   * consider it backward dependence.  (So can have recursion
   * to same operation.)
   */
  if (from_sm_op->serial_number < to_sm_op->serial_number)
    forward = 1;
  else
    forward = 0;

  /* Initialize omega to bad value as sanity check */
  *omega = -1;

  /* Use Lcode function to determine if cross-iteration independent.
   * Care about inner-loop carried dependences only.  This is
   * the only dependences this function currently handles.
   */

  if (L_func_acc_specs)
    {
      is_indep =
	L_mem_indep_acc_specs_cross (from_sm_op->lcode_op, to_sm_op->lcode_op,
				     inner_carried, forward, omega);
    }
  else if (L_use_sync_arcs && L_func_contains_dep_pragmas)
    {
      is_indep =
	L_analyze_syncs_for_cross_iter_independence (from_sm_op->lcode_op,
						     to_sm_op->lcode_op,
						     inner_carried,
						     forward, omega);
    }
  else
    {
      is_indep =
	L_independent_memory_ops (from_sm_op->sm_cb->lcode_cb,
				  from_sm_op->lcode_op, to_sm_op->lcode_op,
				  nonloop_carried|inner_carried);
      if (!is_indep)
	*omega = 1;
    }

  /* If not independent, make sure omega has valid value */
  if (!is_indep)
    {
      /* Sanity check, omega must be > 0 (otherwise not cross-iteration) */
      if (*omega <= 0)
	L_punt ("SM_independent_cross_iter_memory_actions: "
		"Invalid omega returned: %i\n", *omega);

      return (0);
    }
  else
    {
      /* Return 1, there is no cross-iteration dependence from
       * from_action to to_action.
       */
      return (1);
    }
}

void
SM_build_src_mem_deps (SM_Reg_Action * src_action, unsigned int flags)
{
  SM_Reg_Action *action_before, *action_after, *def_action;
  L_Oper *src_lcode_op;
  L_Cb *lcode_cb;
  unsigned int cb_flags, mode;
  int omega;

  /* Cache the src_action's lcode_oper and lcode_cb for ease of use */
  src_lcode_op = src_action->sm_op->lcode_op;
  lcode_cb = src_action->sm_op->sm_cb->lcode_cb;

  /* Cache the src_action's cb_flags for ease of use */
  cb_flags = src_action->sm_op->sm_cb->flags;

  /* 
   * Mem actions cannot have conflicts, so don't use conflict array 
   *
   * This routine is only called operations that reads from memory (loads).
   */

  /* Build the dep in list for this action if necessary */
  if (flags & SM_BUILD_DEP_IN)
    {
      /* Draw a memory flow dependences from every store operation 
       * before this load that that this load may be dependent on.
       * (Follow the def linked list).
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_before = src_action->prev_def;
           action_before != NULL; action_before = action_before->prev_def)
        {
          /* Draw a mem flow dep from the action_before's store/jsr to 
           * this load, unless they can be determined to be independent.
           *
           * The delay will be mdes based with zero offset.
           */
          if (!SM_independent_memory_actions (action_before, src_action))
            {
              SM_add_dep (action_before, src_action,
                          flags | SM_MDES_BASED_DELAY |
                          SM_MEM_DEP | SM_FLOW_DEP, 0, 0, 0, mode);
            }
        }

      /* Handle incoming cross-iteration (cyclic) memory dependences here.  

       * This assumes that the last operation in the cb is
       * the backedge and the only backedge. -ITI/JCG 8/99
       */
      if (cb_flags & SM_CROSS_ITERATION)
        {
          /* Scan all memory writes to determine if a cross iteration
           * dependence is needed into this src_action. 
           */
          for (def_action = src_action->rinfo->first_def;
               def_action != NULL; def_action = def_action->next_def)
            {
              /* Draw an intra-iteration mem flow dep from the 
               * def_action's store/jsr to this load, if not independent.
               * Use omega value specified by sync arcs.
               */
              if (!SM_independent_cross_iter_memory_actions (def_action,
                                                             src_action,
                                                             &omega))
                {
                  SM_add_dep (def_action, src_action,
                              flags | SM_MDES_BASED_DELAY |
                              SM_MEM_DEP | SM_FLOW_DEP, 0, 0, omega, mode);
                }
            }
        }
    }

  /* Build the dep out list for this action if necessary */
  if (flags & SM_BUILD_DEP_OUT)
    {
      /* Draw a mem anti dep from this load to all stores/jsrs after it,
       * unless that store/jsr can be determined to be independent.
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_after = src_action->next_def;
           action_after != NULL; action_after = action_after->next_def)
        {
          /* Determine if store associated with the action_after
           * is indepenent from this load.  If not, draw the 
           * anti-dependence.
           */
          if (!SM_independent_memory_actions (action_after, src_action))
            {
              /* Draw a mem anti dep to the store.
               * The delay is always zero. (This may become mdes-based
               * in the future, but the mdes specification needs more work.)
               */
              SM_add_dep (src_action, action_after,
                          flags | SM_FIXED_DELAY |
                          SM_MEM_DEP | SM_ANTI_DEP, 0, 0, 0, mode);
            }
        }

      /* Handle incoming cross-iteration (cyclic) memory dependences here.  

       * This assumes that the last operation in the cb is
       * the backedge and the only backedge. -ITI/JCG 8/99
       */
      if (cb_flags & SM_CROSS_ITERATION)
        {
          /* Scan all memory writes to determine if a cross iteration
           * dependence is needed from this src_action. 
           */
          for (def_action = src_action->rinfo->first_def;
               def_action != NULL; def_action = def_action->next_def)
            {
              /* Draw an intra-iteration mem anti dep from src_action to
               * the def_action's store/jsr if not independent.
               * Use omega value specified by sync arcs.
               */
              if (!SM_independent_cross_iter_memory_actions (src_action,
                                                             def_action,
                                                             &omega))
                {
                  SM_add_dep (src_action, def_action,
                              flags | SM_FIXED_DELAY |
                              SM_MEM_DEP | SM_ANTI_DEP, 0, 0, omega, mode);
                }
            }
        }
    }
}

void
SM_build_dest_mem_deps (SM_Reg_Action * dest_action, unsigned int flags)
{
  SM_Reg_Action *action_before, *action_after, *actual_action;
  L_Oper *dest_lcode_op;
  L_Cb *lcode_cb;
  unsigned int cb_flags;
  int omega;
  unsigned int mode;

  /* Cache the dest_action's lcode_op and lcode_cb for ease of use */
  dest_lcode_op = dest_action->sm_op->lcode_op;
  lcode_cb = dest_action->sm_op->sm_cb->lcode_cb;

  /* Cache the src_action's cb_flags for ease of use */
  cb_flags = dest_action->sm_op->sm_cb->flags;

  /* 
   * Mem actions cannot have conflicts, so don't use conflict array 
   * 
   * This routine is only called operations that writes to memory (stores).
   */

  /* Build the dep in list for this action if necessary */
  if (flags & SM_BUILD_DEP_IN)
    {
      /* Draw a mem anti dep from this store/jsr to all loads before it,
       * unless that load can be determined to be independent.
       *
       * Draw a mem output dep from this store/jsr to all stores/jsrs before 
       * it unless that store or jsr can be determined to be independent.
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_before = dest_action->prev_actual;
           action_before != NULL; action_before = action_before->prev_actual)
        {
          /* Determine if the load or store associated with the action_before
           * is indepenent from this store.  If not, draw the appropriate
           * dependence.
           */
          if (!SM_independent_memory_actions (dest_action, action_before))
            {
              /* If action_before is for a load, draw a mem anti dep.
               * The delay is always zero. (This may become mdes-based
               * in the future, but the mdes specification needs more work.)
               */
              if (action_before->flags & SM_USE_ACTION)
                {
                  SM_add_dep (action_before, dest_action,
                              flags | SM_FIXED_DELAY |
                              SM_MEM_DEP | SM_ANTI_DEP, 0, 0, 0, mode);
                }

              /* If action_before is for a store/jsr, draw a mem output dep.
               * The delay is always zero.  (This may become mdes-based
               * in the future, but the mdes specification needs more work.)
               */
              else
                {
                  SM_add_dep (action_before, dest_action,
                              flags | SM_FIXED_DELAY |
                              SM_MEM_DEP | SM_OUTPUT_DEP, 0, 0, 0, mode);
                }
            }
        }

      /* Handle incoming cross-iteration (cyclic) memory dependences here.  

       * This assumes that the last operation in the cb is
       * the backedge and the only backedge. -ITI/JCG 8/99
       */
      if (cb_flags & SM_CROSS_ITERATION)
        {
          /* Scan all memory reads and writes to determine if a cross 
           * iteration dependence is needed into this dest_action. 
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (actual_action = dest_action->rinfo->first_actual;
               actual_action != NULL;
               actual_action = actual_action->next_actual)
            {
              /* Is there a dependence from the actual action to
               * this dest action?
               * Use omega value specified by sync arcs.
               */
              if (!SM_independent_cross_iter_memory_actions (actual_action,
                                                             dest_action,
                                                             &omega))
                {
                  /* If actual_action is for a load, draw a cross-iteration
                   * mem anti dep. The delay is always zero. 
                   * (This may become mdes-based in the future, but the 
                   *  mdes specification needs more work.)
                   */
                  if (actual_action->flags & SM_USE_ACTION)
                    {
                      SM_add_dep (actual_action, dest_action,
                                  flags | SM_FIXED_DELAY |
                                  SM_MEM_DEP | SM_ANTI_DEP, 0, 0, omega, mode);
                    }

                  /* If actual_action is for a store/jsr, draw a
                   * cross-iteration mem output dep.
                   * The delay is always zero.  (This may become mdes-based
                   * in the future, but the mdes specification needs 
                   * more work.)
                   */
                  else
                    {
                      SM_add_dep (actual_action, dest_action,
                                  flags | SM_FIXED_DELAY |
                                  SM_MEM_DEP | SM_OUTPUT_DEP,
				  0, 0, omega, mode);
                    }
                }
            }
        }
    }

  /* Build the dep out list for this action if necessary */
  if (flags & SM_BUILD_DEP_OUT)
    {
      /* Draw a mem flow dep from this store/jsr to all loads after it,
       * unless that load can be determined to be independent.
       * 
       * Draw a mem output dep from this store/jsr to all stores/jsrs after 
       * it unless that store/jsr can be determined to be independent.
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_after = dest_action->next_actual;
           action_after != NULL; action_after = action_after->next_actual)
        {
          /* Determine if the load/store/jsr associated with the action_after
           * is indepenent from this store/jsr.  If not, draw the appropriate
           * dependence.
           */
          if (!SM_independent_memory_actions (dest_action, action_after))
            {
              /* If action_after is for a load, draw a mem flow dep.
               * The delay will be mdes-based with zero offset.
               */
              if (action_after->flags & SM_USE_ACTION)
                {
                  SM_add_dep (dest_action, action_after,
                              flags | SM_MDES_BASED_DELAY |
                              SM_MEM_DEP | SM_FLOW_DEP, 0, 0, 0, mode);
                }

              /* If action_after is for a store/jsr, draw a mem output dep.
               * The delay is always zero.  (This may become mdes-based
               * in the future, but the mdes specification needs more work.)
               */
              else
                {
                  SM_add_dep (dest_action, action_after,
                              flags | SM_FIXED_DELAY |
                              SM_MEM_DEP | SM_OUTPUT_DEP, 0, 0, 0, mode);
                }
            }

        }

      /* Handle incoming cross-iteration (cyclic) memory dependences here.  

       * This assumes that the last operation in the cb is
       * the backedge and the only backedge. -ITI/JCG 8/99
       */
      if (cb_flags & SM_CROSS_ITERATION)
        {
          /* Scan all memory read/writes to determine if a cross iteration
           * dependence is needed from this dest_action. 
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (actual_action = dest_action->rinfo->first_actual;
               actual_action != NULL;
               actual_action = actual_action->next_actual)
            {
              /* Is there a dependence from the dest action to this
               * actual action?
               * Use omega value specified by sync arcs.
               */
              if (!SM_independent_cross_iter_memory_actions (dest_action,
                                                             actual_action,
                                                             &omega))
                {
                  /* If actual_action is for a load, draw a 
                   * cross-iteration mem flow dep.
                   * The delay will be mdes-based with zero offset.
                   */
                  if (actual_action->flags & SM_USE_ACTION)
                    {
                      SM_add_dep (dest_action, actual_action,
                                  flags | SM_MDES_BASED_DELAY |
                                  SM_MEM_DEP | SM_FLOW_DEP, 0, 0, omega, mode);
                    }

                  /* If actual_action is for a store/jsr, draw a 
                   * cross-iteration mem output dep.
                   * The delay is always zero.  (This may become mdes-based
                   * in the future, but the mdes specification needs more 
                   * work.)
                   */
                  else
                    {
                      SM_add_dep (dest_action, actual_action,
                                  flags | SM_FIXED_DELAY |
                                  SM_MEM_DEP | SM_OUTPUT_DEP,
				  0, 0, omega, mode);
                    }
                }
            }
        }
    }
}

/* Returns 0 if can be moved above branch, otherwise, returns the
 * flags that should be used when drawing the dependence.
 * Currently will use SM_HARD_DEP for all deps except where the
 * operation can be speculated and has an excepting version.
 * Then, SM_SOFT_DEP will be returned and SM_CAN_SPEC_IGNORE will be returned 
 * as the ignore value.
 */
int
SM_prevent_from_moving_above_branch (int prepass, unsigned int *ignore,
				     SM_Reg_Action * branch,
                                     SM_Reg_Action * action)
{
  *ignore = 0;

  if (!prepass)
    {
      /* Only allow speculation of ops in post-pass
       *  if op is for spill code
       */
      if (!L_spill_code (action->sm_op->lcode_op))
        return (SM_HARD_DEP);
    }

  /* Do not allow speculation of branches, stores, or operations marked
   * with NOSPEC
   */
  if (action->sm_op->mdes_flags &
      (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS | OP_FLAG_JSR | OP_FLAG_CHK |
       OP_FLAG_NOSPEC))
    {
      return (SM_HARD_DEP);
    }

  /* Stores are already prevented from moving above jsrs, so only prevent
   * stores from moving above other types of branches.
   */
  if ((action->sm_op->mdes_flags & OP_FLAG_STORE) &&
      !(branch->sm_op->mdes_flags & OP_FLAG_JSR))
    {
      return (SM_HARD_DEP);
    }

  /* Currently there are no control deps draw to non-branch operations */
  if (branch->sm_op->mdes_flags & OP_FLAG_JSR)
    {
      /* Nothing can currently move past JSR's in postpass scheduling */
      if (branch->sm_op->sm_cb->prepass_sched)
        return (0);
      else
        return (SM_HARD_DEP);
    }


  /* Can speculate if not excepting */
  if (!(action->sm_op->mdes_flags & OP_FLAG_EXCEPT))
    return (0);

  /* If the operation can be determined to be safe to speculate above
   * this branch, then allow it to be speculated even though it is excepting.
   * 
   * Support the simple PEI marking now.  The rest of the safe analysis
   * needs to be written to take advantage of the rinfo table!
   */
  if (action->sm_op->lcode_op->flags & L_OPER_SAFE_PEI)
    return (0);

  /* If this operation has a silent version, then allow it to speculate
   * Place a soft dep that is ignored, so the schedule manager will
   * know when to make it a silent version.
   */
  if (action->sm_op->flags & SM_HAS_SILENT_VERSION)
    {
#if 0
      /* JWS - prevent speculation of marked dangerous loads */
      if (L_find_attr (action->sm_op->lcode_op->attr, "MKLD"))
	return SM_HARD_DEP;
#endif

      /* Ops that can speculate above a branch do not generally
       * need to be speculative to move above a check.
       * Recovery code generation handles the ones that need speculation.
       */
      if (branch->sm_op->lcode_op->opc == Lop_CHECK)
        {
          return (0);
        }
      *ignore = SM_CAN_SPEC_IGNORE;
      return (SM_SOFT_DEP);
    }

  /* If cannot come up with a good reason why it can be speculated, 
   * then mark as not being able to speculate.
   */
  return (SM_HARD_DEP);
}

int
SM_safe_to_move_below_branch (SM_Reg_Action * branch, SM_Reg_Action * action)
{
  /* Do not allow percolation downwards of branches, 
   * or operations marked with NOSPEC
   */
  if (action->sm_op->mdes_flags &
      (OP_FLAG_CBR | OP_FLAG_JMP | OP_FLAG_RTS | OP_FLAG_JSR | OP_FLAG_CHK |
       OP_FLAG_NOSPEC))
    {
      return (0);
    }

  /* Stores are already prevented from moving below jsrs, so only prevent
   * stores from moving below other types of branches.
   */
  if ((action->sm_op->mdes_flags & OP_FLAG_STORE) &&
      !(branch->sm_op->mdes_flags & OP_FLAG_JSR))
    {
      return (0);
    }

  /* Do not allow percolation downwards if the branch is uncondition jump
   * or an rts.  Lcode does not allow operations to be moved below these.
   * DIA - PLEASE CHECK ALL JUMPS FOR UNCONDITIONALITY BEFORE TREATING THEM
   * AS UNCONDITIONAL!
   */
  if ((branch->sm_op->mdes_flags & (OP_FLAG_JMP | OP_FLAG_RTS)) &&
      PG_true_predicate_op (branch->sm_op->lcode_op))
    {
      return (0);
    }

  /* During postpass, nothing may move past a jsr. */
  if ((branch->sm_op->mdes_flags & OP_FLAG_JSR) &&
      (!branch->sm_op->sm_cb->prepass_sched))
    {
      return (0);
    }

  /* Otherwise, it can be moved below the branch */
  return (1);
}

void
SM_build_src_ctrl_deps (SM_Cb * sm_cb, SM_Reg_Action * src_action,
                        unsigned int flags)
{
  SM_Reg_Action *def_before, *def_after;
  unsigned int dep_flags;
  unsigned int dep_ignore;
  unsigned int cb_flags;
  unsigned int mode;

  /* Cache the src_action's cb_flags for ease of use */
  cb_flags = src_action->sm_op->sm_cb->flags;

  /* 
   * Ctrl actions cannot have conflicts, so don't use conflict array 
   */

  dep_ignore = 0;
  /* Build the dep in list for this action if necessary */
  if (flags & SM_BUILD_DEP_IN)
    {
      /* Draw ctrl flow deps to all branches above this op that
       * this op cannot be moved above.
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (def_before = src_action->prev_def;
           def_before != NULL; def_before = def_before->prev_def)
        {
          /* Do we need to prevent this operation from moving above
           * the branch?  If non-zero, then indicates the dep hardness.
           */
          dep_flags =
            SM_prevent_from_moving_above_branch (sm_cb->prepass_sched,
                                                 &dep_ignore, def_before,
                                                 src_action);

          /* Need a dep, add using dep_flags to select proper hardness
           * for the dep.
           */
          if (dep_flags != 0)
            {
              /* No, draw control dependence to this operation
               * The delay is mdes-based with zero offset.
               */
              SM_add_dep (def_before, src_action,
                          flags | dep_flags | SM_MDES_BASED_DELAY |
                          SM_CTRL_DEP | SM_FLOW_DEP, dep_ignore, 0, 0, mode);
            }
        }

      /* Draw cross iteration dependences if cb flag set -ITI/JCG 8/99 */
      if (cb_flags & SM_CROSS_ITERATION)
        {
          /* Draw cross-iteration ctrl flow deps to all branches 
           * after this op (before if looking around the backedge) that
           * this op cannot be moved above.
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
	  for (def_before = src_action->rinfo->last_def;
               ((def_before != NULL) &&
                (def_before->sm_op->serial_number >=
                 src_action->sm_op->serial_number));
               def_before = def_before->prev_def)
            {
              /* Do we need to prevent this operation from moving above
               * the branch?  If non-zero, then indicates the dep hardness.
               */
              dep_flags =
                SM_prevent_from_moving_above_branch (sm_cb->prepass_sched,
                                                     &dep_ignore, def_before,
                                                     src_action);

              /* Need a dep, add using dep_flags to select proper hardness
               * for the dep.
               */
              if (dep_flags != 0)
                {
                  /* No, draw cross-iteration control dependence to this 
                   * operation.  To force control dependent operations
                   * into the same stage, the delay must be 1 or greater
                   * (otherwise may speculate stores, etc. during modulo
                   * scheduling).
                   *
                   * For now, use fixed 1 cycle delay (otherwise requires
                   * non-trivial enhancement to allow minimum delay to be
                   * specified for a dep that might be variable latency).
                   * Punts below if this quick fix violates the MDES 
                   * specification. -ITI/JCG 9/99
                   */
                  SM_add_dep (def_before, src_action,
                              flags | dep_flags | SM_FIXED_DELAY |
                              SM_CTRL_DEP | SM_FLOW_DEP, dep_ignore,
			      1, 1, mode);

                  /* Sanity check, make sure the MDES didn't specify a 
                   * control dependence > 1 (which my quick fix above would
                   * violate). -ITI/JCG 9/99
                   */
                  if ((def_before->max_late_use_time -
                       src_action->min_early_use_time) > 1)
                    {
                      L_punt ("SM_build_src_ctrl_deps: modulo-scheduler \n"
                              "expects control dependence delay to be 0 or "
                              "1, not %i!\n"
                              "See comments in this function for details!\n",
                              (def_before->max_late_use_time -
                               src_action->min_early_use_time));
                    }
                }
            }
        }
    }

  /* Build the dep out list for this action if necessary */
  if (flags & SM_BUILD_DEP_OUT)
    {
      /* If this is a src action for a branch, don't draw the 
       * anti-deps from this branch, since there will be flow deps.
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      if ((src_action->next_def != NULL) &&
          (src_action->next_def->sm_op != src_action->sm_op))
        {
          /* Draw ctrl anti deps to all branches below this op that
           * this op cannot be moved below.
           */
          for (def_after = src_action->next_def;
               def_after != NULL; def_after = def_after->next_def)
            {
              /* Add dep if this action is not safe to move below the 
               * def_after.
               */
              if (!SM_safe_to_move_below_branch (def_after, src_action))
                {
                  /* Draw ctrl anti from this op to the branch.
                   * The delay is always fixed at zero.
                   */
                  SM_add_dep (src_action, def_after,
                              flags | SM_FIXED_DELAY |
                              SM_CTRL_DEP | SM_ANTI_DEP, 0, 0, 0, mode);
                }
            }

          /* Draw cross iteration dependences if cb flag set -ITI/JCG 8/99 */
          if (cb_flags & SM_CROSS_ITERATION)
            {
              /* Draw cross-iteration ctrl anti deps to all branches 
               * below this op (looking around the backedge) that
               * this op cannot be moved below.
               */
              for (def_after = src_action->rinfo->first_def;
                   ((def_after != NULL) &&
                    (def_after->sm_op->serial_number <=
                     src_action->sm_op->serial_number));
                   def_after = def_after->next_def)
                {
                  /* Add dep if this action is not safe to move below the 
                   * def_after.
                   */
                  if (!SM_safe_to_move_below_branch (def_after, src_action))
                    {
                      /* Draw cross-iteration ctrl anti from this op to 
                       * the branch.
                       * The delay is always fixed at zero.
                       */
                      SM_add_dep (src_action, def_after,
                                  flags | SM_FIXED_DELAY |
                                  SM_CTRL_DEP | SM_ANTI_DEP, 0, 0, 1, mode);
                    }
                }
            }
        }
    }

}

void
SM_build_dest_ctrl_deps (SM_Cb * sm_cb, SM_Reg_Action * dest_action,
                         unsigned int flags)
{
  SM_Reg_Action *action_before, *action_after;
  unsigned int dep_flags;
  unsigned int dep_ignore;
  unsigned int cb_flags;
  unsigned int mode;

  /* Cache the dest_action's cb_flags for ease of use */
  cb_flags = dest_action->sm_op->sm_cb->flags;

  /* 
   * Ctrl actions cannot have conflicts, so don't use conflict array 
   */

  dep_ignore = 0;
  /* Build the dep in list for this action if necessary */
  if (flags & SM_BUILD_DEP_IN)
    {
      /* Draw ctrl anti deps to all ops above this branch that
       * cannot be moved below this branch.
       * 
       * Skip the ctrl action right before this one since it is 
       * the src ctrl action for the same operation!
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_before = dest_action->prev_actual->prev_actual;
           action_before != NULL; action_before = action_before->prev_actual)
        {
          /* Draw dependences to only src_actions before dest_action.
           * (No need to draw output dependences, since the ctrl flow
           *  dependences do the job, so they are not drawn)
           */
          if (action_before->flags & SM_DEF_ACTION)
            {
              /*
               * Also, skip the src right before the dest, so don't draw
               * the extra and unnecessary anti-dependence, since
               * the ctrl flow dep does the job.
               */
              action_before = action_before->prev_actual;
              continue;
            }

          /* Add dep if action_before is not safe to move below the branch */
          if (!SM_safe_to_move_below_branch (dest_action, action_before))
            {
              /* Draw ctrl anti from this op to this branch.
               * The delay is always fixed at zero.
               */
              SM_add_dep (action_before, dest_action,
                          flags | SM_FIXED_DELAY |
                          SM_CTRL_DEP | SM_ANTI_DEP, 0, 0, 0, mode);
            }
        }

      /* Draw cross iteration dependences if cb flag set -ITI/JCG 8/99 */
      if (cb_flags & SM_CROSS_ITERATION)
        {
          /* Draw cross-iteration ctrl anti deps to all ops above 
           * this branch (looking around the backedge) that
           * cannot be moved below this branch.
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (action_before = dest_action->rinfo->last_actual;
               ((action_before != NULL) &&
                (action_before->sm_op->serial_number >
                 dest_action->sm_op->serial_number));
               action_before = action_before->prev_actual)
            {
              /* Draw dependences to only src_actions before dest_action.
               * (No need to draw output dependences, since the ctrl flow
               *  dependences do the job, so they are not drawn)
               */
              if (action_before->flags & SM_DEF_ACTION)
                {
                  /*
                   * Also, skip the src right before the dest, so don't draw
                   * the extra and unnecessary anti-dependence, since
                   * the ctrl flow dep does the job.
                   */
                  action_before = action_before->prev_actual;
                  continue;
                }

              /* Add cross-iteration dep if action_before is not safe to 
               * move below the  branch */
              if (!SM_safe_to_move_below_branch (dest_action, action_before))
                {
                  /* Draw ctrl anti from this op to this branch.
                   * The delay is always fixed at zero.
                   */
                  SM_add_dep (action_before, dest_action,
                              flags | SM_FIXED_DELAY |
                              SM_CTRL_DEP | SM_ANTI_DEP, 0, 0, 1, mode);
                }
            }
        }
    }


  /* Build the dep out list for this action if necessary */
  if (flags & SM_BUILD_DEP_OUT)
    {
      /* Draw ctrl flow deps to all ops below this branch that
       * cannot be moved above this branch.  
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_after = dest_action->next_actual;
           action_after != NULL; action_after = action_after->next_actual)
        {
          /* Draw dependences to only src_actions after dest_action.
           * (No need to draw output dependences, so they are not drawn)
           */
          if (action_after->flags & SM_DEF_ACTION)
            continue;

          /* If we need to add a dep, get it's hardness */
          dep_flags =
            SM_prevent_from_moving_above_branch (sm_cb->prepass_sched,
                                                 &dep_ignore, dest_action,
                                                 action_after);

          /* If we need to add a dep, use dep_flags to set hardness of dep */
          if (dep_flags != 0)
            {
              /* No, draw control dependence to this operation
               * The delay is mdes-based with zero offset.
               */
              SM_add_dep (dest_action, action_after,
                          flags | dep_flags | SM_MDES_BASED_DELAY |
                          SM_CTRL_DEP | SM_FLOW_DEP, dep_ignore, 0, 0, mode);
            }
        }

      /* Draw cross iteration dependences if cb flag set -ITI/JCG 8/99 */
      if (cb_flags & SM_CROSS_ITERATION)
        {
          /* Draw cross-iteration ctrl flow deps to all ops below this 
           * branch (looking around the backedge) that
           * cannot be moved above this branch.  
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (action_after = dest_action->rinfo->first_actual;
               ((action_after != NULL) &&
                (action_after->sm_op->serial_number <=
                 dest_action->sm_op->serial_number));
               action_after = action_after->next_actual)
            {
              /* Draw dependences to only src_actions after dest_action.
               * (No need to draw output dependences, so they are not drawn)
               */
              if (action_after->flags & SM_DEF_ACTION)
                continue;

              /* If we need to add a dep, get it's hardness */
              dep_flags =
                SM_prevent_from_moving_above_branch (sm_cb->prepass_sched,
                                                     &dep_ignore, dest_action,
                                                     action_after);

              /* If we need to add a cross-iteration dep, use dep_flags to 
               * set hardness of dep 
               */
              if (dep_flags != 0)
                {
                  /* No, draw cross-iteration control dependence to this 
                   * operation.  To force control dependent operations
                   * into the same stage, the delay must be 1 or greater
                   * (otherwise may speculate stores, etc. during modulo
                   * scheduling).
                   *
                   * For now, use fixed 1 cycle delay (otherwise requires
                   * non-trivial enhancement to allow minimum delay to be
                   * specified for a dep that might be variable latency).
                   * Punts below if this quick fix violates the MDES 
                   * specification. -ITI/JCG 9/99
                   */
                  SM_add_dep (dest_action, action_after,
                              flags | dep_flags | SM_FIXED_DELAY |
                              SM_CTRL_DEP | SM_FLOW_DEP, dep_ignore,
			      1, 1, mode);

                  /* Sanity check, make sure the MDES didn't specify a 
                   * control dependence > 1 (which my quick fix above would
                   * violate). -ITI/JCG 9/99
                   */
                  if ((dest_action->max_late_use_time -
                       action_after->min_early_use_time) > 1)
                    {
                      L_punt ("SM_build_dest_ctrl_deps: modulo-scheduler \n"
                              "expects control dependence delay to be 0 or "
                              "1, not %i!\n"
                              "See comments in this function for details!\n",
                              (dest_action->max_late_use_time -
                               action_after->min_early_use_time));
                    }
                }
            }
        }
    }
}

void
SM_build_src_sync_deps (SM_Reg_Action * src_action, unsigned int flags)
{
  unsigned int mode;
  /* 
   * Sync actions cannot have conflicts, so don't use conflict array 
   */

  /* Build the dep in list for this action if necessary */
  if (flags & SM_BUILD_DEP_IN)
    {
      /* Draw a sync dep(s) to the sync operation(s) before this one.
       * (For now, assume sync operations ignore predicates!).
       * 
       * Use Mdes information to set delay (no offset).
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      if (src_action->prev_def != NULL)
        {
          SM_add_dep (src_action->prev_def, src_action,
                      flags | SM_MDES_BASED_DELAY |
                      SM_SYNC_DEP | SM_FLOW_DEP, 0, 0, 0, mode);
        }

      /* If no sync operation before, look around backedge if drawing
       * cross-iteration dependences. -ITI/JCG 8/99
       */
      if ((src_action->sm_op->sm_cb->flags & SM_CROSS_ITERATION) &&
          (src_action->prev_def == NULL) &&
          (src_action->rinfo->last_def != NULL))
        {
          SM_add_dep (src_action->rinfo->last_def, src_action,
                      flags | SM_MDES_BASED_DELAY |
                      SM_SYNC_DEP | SM_FLOW_DEP, 0, 0, 1, mode);
        }
    }

  /* Build the dep out list for this action if necessary */
  if (flags & SM_BUILD_DEP_OUT)
    {
      /* Draw a sync anti dep to the sync operation (if any) after this one,
       * unless this src_action is for a sync operation (then no anti 
       * dependence is necessary).
       * 
       * This distance will always be zero.
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      if ((src_action->next_def != NULL) &&
          (src_action->sm_op != src_action->next_def->sm_op))
        {
          SM_add_dep (src_action, src_action->next_def,
                      flags | SM_FIXED_DELAY |
                      SM_SYNC_DEP | SM_ANTI_DEP, 0, 0, 0, mode);
        }

      /* If no sync operation afterwards, look around backedge if drawing
       * cross-iteration dependences. -ITI/JCG 8/99
       */
      if ((src_action->sm_op->sm_cb->flags & SM_CROSS_ITERATION) &&
          (src_action->next_def == NULL) &&
          (src_action->rinfo->first_def != NULL))
        {
          SM_add_dep (src_action, src_action->rinfo->first_def,
                      flags | SM_FIXED_DELAY |
                      SM_SYNC_DEP | SM_ANTI_DEP, 0, 0, 1, mode);
        }
    }
}

void
SM_build_dest_sync_deps (SM_Reg_Action * dest_action, unsigned int flags)
{
  SM_Reg_Action *action_before, *action_after;
  unsigned int mode;

  /* 
   * Sync actions cannot have conflicts, so don't use conflict array 
   */

  /* Build the dep in list for this action if necessary */
  if (flags & SM_BUILD_DEP_IN)
    {
      /* Draw a sync anti dep to every operation before this one
       * until hit the previous sync opertion.
       * 
       * Skip the sync action right before this one since it is 
       * the src sync action for the same operation!
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_before = dest_action->prev_actual->prev_actual;
           action_before != NULL; action_before = action_before->prev_actual)
        {
          /* Stop if just hit to another sync operation def.
           * Adding an output dependence is unnecessary, so it is omitted.
           */
          if (action_before->flags & SM_DEF_ACTION)
            break;

          /* For all operations before this sync operation, draw 
           * anti dependences until hit another sync operation.
           * The delay will always be zero.
           */
          SM_add_dep (action_before, dest_action,
                      flags | SM_FIXED_DELAY |
                      SM_SYNC_DEP | SM_ANTI_DEP, 0, 0, 0, mode);
        }

      /* Draw cross-iteration sync deps if desired and if need
       * to look around backedge (no prev def) -ITI/JCG 8/99 
       */
      if ((dest_action->sm_op->sm_cb->flags & SM_CROSS_ITERATION) &&
          (dest_action->prev_def == NULL))
        {
          /* Draw a cross-iteration anti dep to every operation before this
           * one (looking around the backedge) until hit the previous sync 
           * operation (which may be this dest_action).
           */
	  mode = SM_HARD_DEP | SM_SOFT_DEP;
          for (action_before = dest_action->rinfo->last_actual;
               action_before != NULL;
               action_before = action_before->prev_actual)
            {
              /* Stop if just hit to another sync operation def.
               * Adding an output dependence is unnecessary, 
               * so it is omitted.
               */
              if (action_before->flags & SM_DEF_ACTION)
                break;

              /* For all operations before this sync operation (around
               * the backedge), draw cross-iteration anti dependences 
               * until hit another sync operation.
               * The delay will always be zero.
               */
              SM_add_dep (action_before, dest_action,
                          flags | SM_FIXED_DELAY |
                          SM_SYNC_DEP | SM_ANTI_DEP, 0, 0, 1, mode);
            }
        }
    }

  /* Build the dep out list for this action if necessary */
  if (flags & SM_BUILD_DEP_OUT)
    {
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      for (action_after = dest_action->next_actual;
           action_after != NULL; action_after = action_after->next_actual)
        {
          /* Stop if hit another sync operation def.
           * Adding an output dependence is unnecessary, so it is omitted.
           */
          if (action_after->flags & SM_DEF_ACTION)
            break;

          /* For all operations after this sync operation, draw
           * flow dependences until hit another sync operation.
           * This delay will be mdes based with zero offset.
           */
          SM_add_dep (dest_action, action_after,
                      flags | SM_MDES_BASED_DELAY |
                      SM_SYNC_DEP | SM_FLOW_DEP, 0, 0, 0, mode);
        }

      /* Draw cross-iteration sync deps if desired and if need
       * to look around backedge (no next def) -ITI/JCG 8/99 
       */
      mode = SM_HARD_DEP | SM_SOFT_DEP;
      if ((dest_action->sm_op->sm_cb->flags & SM_CROSS_ITERATION) &&
          (dest_action->next_def == NULL))
        {
          for (action_after = dest_action->rinfo->first_actual;
               action_after != NULL; action_after = action_after->next_actual)
            {
              /* Stop if hit another sync operation def (may be dest_action).
               * Adding an output dependence is unnecessary, 
               * so it is omitted.
               */
              if (action_after->flags & SM_DEF_ACTION)
                break;

              /* For all operations after this sync operation (around the
               * backedge), draw cross-iteration flow dependences until 
               * hit another sync operation.
               * This delay will be mdes based with zero offset.
               */
              SM_add_dep (dest_action, action_after,
                          flags | SM_MDES_BASED_DELAY |
                          SM_SYNC_DEP | SM_FLOW_DEP, 0, 0, 1, mode);
            }
        }
    }
}

void
SM_build_oper_dependences (SM_Cb * sm_cb, SM_Oper * sm_op, unsigned int flags)
{
  int i;
  SM_Action_Qentry *action_qentry;

  /* Must set at least one of SM_BUILD_DEP_IN and SM_BUILD_DEP_OUT! */
  if ((!(flags & SM_BUILD_DEP_IN)) && (!(flags & SM_BUILD_DEP_OUT)))
    {
      L_punt ("SM_build_oper_dependences: SM_BUILD_DEP_IN, \n"
              "SM_BUILD_DEP_OUT, or both must be specified in flags!");
    }

  /* Build dependences for every explicit src action */
  for (i = 0; i < L_max_src_operand; i++)
    {
      /* Add actions only for non-NULL src actions */
      if (sm_op->src[i] != NULL)
        {
          SM_build_src_reg_deps (sm_op->src[i], flags);
        }
    }

  /* Build dependences for every implicit src action */
  if (sm_op->implicit_srcs != NULL)
    {
      for (action_qentry = sm_op->implicit_srcs->first_qentry;
           action_qentry != NULL; action_qentry = action_qentry->next_qentry)
        {
          SM_build_src_reg_deps (action_qentry->action, flags);
        }
    }

  /* Build dependences for every explicit pred action
   * Currently only pred[0] is a valid action (pred[1] is informational
   * only)
   */
  if ((L_max_pred_operand > 0) && (sm_op->pred[0] != NULL))
    {
      SM_build_src_reg_deps (sm_op->pred[0], flags);
    }


  /* Build src memory dependences if have non-NULL src action */
  if (sm_op->ext_src[SM_MEM_ACTION_INDEX] != NULL)
    SM_build_src_mem_deps (sm_op->ext_src[SM_MEM_ACTION_INDEX], flags);

  /* Build src control dependences (all have non-NULL src actions) */
  SM_build_src_ctrl_deps (sm_cb, sm_op->ext_src[SM_CTRL_ACTION_INDEX], flags);

  /* Build src synce dependences (all have non-NULL src actions) */
  SM_build_src_sync_deps (sm_op->ext_src[SM_SYNC_ACTION_INDEX], flags);

  /* Need to figure out what src VLIW dependences are! */

  /* Build dependences for every explicit dest action */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      /* Add actions only for non-NULL dest actions */
      if (sm_op->dest[i] != NULL)
        {
          SM_build_dest_reg_deps (sm_op->dest[i], flags);
        }
    }

  /* Build dependences for every implicit dest action */
  if (sm_op->implicit_dests != NULL)
    {
      for (action_qentry = sm_op->implicit_dests->first_qentry;
           action_qentry != NULL; action_qentry = action_qentry->next_qentry)
        {
          SM_build_dest_reg_deps (action_qentry->action, flags);
        }
    }


  /* Build dest control dependences if have non-NULL dest action */
  if (sm_op->ext_dest[SM_CTRL_ACTION_INDEX] != NULL)
    SM_build_dest_ctrl_deps (sm_cb, sm_op->ext_dest[SM_CTRL_ACTION_INDEX],
                             flags);

  /* Build dest sync dependences if have non-NULL dest action */
  if (sm_op->ext_dest[SM_SYNC_ACTION_INDEX] != NULL)
    SM_build_dest_sync_deps (sm_op->ext_dest[SM_SYNC_ACTION_INDEX], flags);

  /* Build dest memory dependences if have non-NULL dest action */
  if (sm_op->ext_dest[SM_MEM_ACTION_INDEX] != NULL)
    SM_build_dest_mem_deps (sm_op->ext_dest[SM_MEM_ACTION_INDEX], flags);


  /* Need to figure out what dest VLIW dependences are! */

  /* If this operation has all of its incoming dependences resolved,
   * then place it at the end of the dep_in_resolved queue
   * (if not already in the queue from calling this routine
   *  twice to check dependences).
   */
  if ((sm_op->num_unresolved_hard_dep_in == 0) &&
      /* EMN (sm_op->num_unresolved_soft_dep_in == 0) && */
      (!sm_op->ignore))
    {
      if (sm_op->dep_in_resolved_qentry == NULL)
        {
          sm_op->dep_in_resolved_qentry =
            SM_enqueue_oper_before (sm_op->sm_cb->dep_in_resolved,
                                    sm_op, NULL);
        }
    }

  /* Otherwise, make sure not in queue, since really don't know
   * how to handle this right now, punt
   */
  else if (sm_op->dep_in_resolved_qentry != NULL)
    {
      L_punt
        ("SM_build_oper_dependences: Incorrectly in dep_in_resolved queue!");
    }
}

/* Due to asymmetries in the way conflicting operand and predication
 * are handled, "harmless" extraneous dependences are added.  This
 * reduces the error-checking possible in these cases, but this routine
 * tries to detect some of the extrameous dependences that are not harmless. 
 *
 * To determine if an dependence is harmless, it determines the
 * dependence height between the test_dep->from_action and test_dep->to_action,
 * ONLY TAKING into account dependences for thoses types of actions.
 * If the test_dep would increase this height, then it is determined
 * to be harmful (returning 0).  Otherwise, it returns 1.
 *
 * By only taking into account dependences for test_dep's from and to actions,
 * other dependences (for other actions) are prevented from masking the 
 * harmful effects of test_dep.  Truely harmless dependences added
 * only because of the asymmetries should always pass this test.
 */
int
SM_harmless_dep (SM_Dep * test_dep)
{
  SM_Oper *start_op, *end_op, *sm_op;
  SM_Reg_Action *action;
  SM_Reg_Info *from_rinfo, *to_rinfo, *extra_rinfo;
  SM_Dep *dep;
  int calc_delay, op_height, calc_height, test_height;


  /* Get the sm_ops to start and stop the scan for */
  start_op = test_dep->from_action->sm_op;
  end_op = test_dep->to_action->sm_op->next_serial_op;

  /* Initialize all the test heights to a big negative number,
   * to show that initially there are not dependence height between them 
   */
  for (sm_op = start_op; sm_op != end_op; sm_op = sm_op->next_serial_op)
    {
      sm_op->temp_height = SM_MIN_CYCLE;
    }

  /* Set the starting op to height 0 */
  start_op->temp_height = 0;

  /* Cache the rinfos for ease of use */
  from_rinfo = test_dep->from_action->rinfo;
  to_rinfo = test_dep->to_action->rinfo;

  /* If either action in implicit, we need to also include control
   * dependences in our harmless calculation (since we minimize
   * the implicit dependences by assuming the control dependences
   * will keep jsrs from reordering.)
   */
  if ((test_dep->from_action->flags & SM_IMPLICIT_ACTION) ||
      (test_dep->to_action->flags & SM_IMPLICIT_ACTION))
    {
      extra_rinfo = start_op->sm_cb->ctrl_rinfo;
    }

  /* Otherwise, don't consider control deps */
  else
    {
      extra_rinfo = NULL;
    }


  /* For each operation, in serial order between the from_action and
   * the to_action of test_dep, calculate the dep height constraints
   * for ONLY the actions operating on the same type of operands 
   * as test_dep.
   */
  for (sm_op = start_op; sm_op != end_op; sm_op = sm_op->next_serial_op)
    {
      /* Get the height of this operation for ease of use */
      op_height = sm_op->temp_height;

      for (action = sm_op->first_op_action; action != NULL;
           action = action->next_op_action)
        {
          /* Only look at actions for the same rinfo(s) as
           * the test dependence (or ctrl rinfo in some cases).
           */
          if ((action->rinfo != from_rinfo) &&
              (action->rinfo != to_rinfo) && (action->rinfo != extra_rinfo))
            continue;

          /* Update the heights of operations on the other
           * side of all the dep outs for this action.
           */
          for (dep = action->first_dep_out; dep != NULL;
               dep = dep->next_dep_out)
            {
              /* Don't include 'test_dep' in calculation */
              if (dep == test_dep)
                continue;

              /* Don't include cross-iteration dependences */
              if (dep->omega != 0)
                continue;

              /* Want to give zero cycle dependences a very small
               * delay, so shift over the normal (short) delay by 16.
               */
              calc_delay = (((int) dep->min_delay) << 16) + 1;

              calc_height = op_height + calc_delay;

              if (dep->to_action->sm_op->temp_height < calc_height)
                dep->to_action->sm_op->temp_height = calc_height;
            }
        }
    }

  /* Calculate the height caused by 'test_dep' */
  test_height = (((int) test_dep->min_delay) << 16) + 1;

#if 0
  /* Debug */
  SM_print_dep (stderr, test_dep);
  fprintf (stderr, "Without %i, with %i\n\n",
           test_dep->to_action->sm_op->temp_height, test_height);
#endif

  /* The dependence is not harmless if effects the temp_height of the to_op */
  if (test_dep->to_action->sm_op->temp_height < test_height)
    return (0);

  /* Otherwise, it is likely to be harmless */
  else
    return (1);
}

/* Returns 1 if the dependence is "extraneous" (can be left in or removed
 * without affecting the schedule) AND was added due to a know asymmetry
 * (we want as few extraneous, unmatched dependences as possible).
 */
int
SM_extraneous_dep (SM_Dep * dep)
{
  /* Currently know asymmetries are due to:
   * 1) Drawing dependences between conflicting (different) operands.
   * 2) Drawing dependences between operations with different predicates.
   *
   * For these cases, check to make sure dependence is extraneous
   * before returning 1.
   *
   *    * See JWS 20000107 above.
   */
  if ((dep->from_action->rinfo != dep->to_action->rinfo) ||
      ((L_max_pred_operand > 0) &&
       !PG_equivalent_predicates_ops (dep->from_action->sm_op->lcode_op,
                                      dep->to_action->sm_op->lcode_op)))
    {
      /* If the dependence seems harmless, assume the dep is
       * truely extraneous.  Otherwise, return 0 so that
       * the check routine gives an error message.
       */
      if (SM_harmless_dep (dep))
        return (1);
      else
        return (0);
    }

  /* Otherwise, if not caused by an asymmetry, by definition
   * it is not extraneous (we don't want unmatched dependences!).
   */
  return (0);
}

void
SM_check_and_delete_symmetric_deps (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  SM_Dep *dep1, *dep2;
  unsigned dep1_flags, dep2_flags;
  SM_Reg_Action *op_action, *dep1_from_action;
  int mismatch_count, harmless_count;

  /* There should be no mismatches */
  mismatch_count = 0;
  harmless_count = 0;

  /* Check every operation in sm_cb */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* Check every action for op */
      for (op_action = sm_op->first_op_action; op_action != NULL;
           op_action = op_action->next_op_action)
        {
          /* Only need to check incoming dependences.
           * For each dependence marked with SM_BUILD_DEP_OUT, find and
           * delete the equivalent dependence marked with SM_BUILD_DEP_IN
           */
          for (dep1 = op_action->first_dep_in; dep1 != NULL;
               dep1 = dep1->next_dep_in)
            {
              /* Cache the flags parameter to speed up test */
              dep1_flags = dep1->flags;

              /* Only test parameters marked with SM_BUILD_DEP_OUT */
              if (!(dep1_flags & SM_BUILD_DEP_OUT))
                continue;

              /* Change dep1 flags to have SM_BUILD_DEP_IN instead of OUT */
              dep1_flags ^= SM_BUILD_DEP_OUT | SM_BUILD_DEP_IN;

              /* Cache the from_action to fields to speed up the test */
              dep1_from_action = dep1->from_action;

              /* Find the equivalent dependence marked with SM_BUILD_DEP_IN */
              for (dep2 = op_action->first_dep_in; dep2 != NULL;
                   dep2 = dep2->next_dep_in)
                {
                  /* Cache dep2 flags */
                  dep2_flags = dep2->flags;

                  /* Only test parameters not marked with SM_BUILD_DEP_OUT.
                   * Test this instead of SM_BUILD_DEP_IN to catch the
                   * possible cases where a dep is marked with both.
                   */
                  if (dep2_flags & SM_BUILD_DEP_OUT)
                    continue;

                  /* Stop if this is the equivalent dependence to dep1 */
                  if ((dep2_flags == dep1_flags) &&
                      (dep2->from_action == dep1_from_action) &&
                      (dep2->delay_offset == dep1->delay_offset) &&
                      (dep2->min_delay == dep1->min_delay) &&
                      (dep2->max_delay == dep1->max_delay) &&
                      (dep2->omega == dep1->omega))
                    {
                      /* Found, break out of search loop */
                      break;
                    }
                }

              /* If matching dep2 found, delete it */
              if (dep2 != NULL)
                {
                  SM_delete_dep (dep2);
                }

              /* Otherwise, check to see if an error message should be
               * printed.  Unfortunately, there is built in asymmetry
               * with the handling of conflicting operand and the
               * handling of predicated code, so an unmatched dependence
               * may not be an error.  Fortunately, the dependences
               * drawn by these asymmetries are "extraneous" and can
               * be left in or deleted without affecting the schedule.
               * So, we just have to detect if the dependence not
               * extraneous, and print the error messages in those cases.
               */
              else if (!SM_extraneous_dep (dep1))
                {
                  /* Use global L_fn until get SM_Func going */
                  fprintf (stderr,
                           "SM_check_and_delete_symmetric_deps: %s cb %i:\n",
                           dep1->to_action->sm_op->sm_cb->lcode_fn->name,
                           dep1->to_action->sm_op->sm_cb->lcode_cb->id);

                  fprintf (stderr,
                           "  Dep built only for op %i (with the "
                           "SM_BUILD_DEP_OUT flag):\n",
                           dep1->from_action->sm_op->lcode_op->id);

                  fprintf (stderr, "    From:");
                  SM_print_oper (stderr, dep1->from_action->sm_op);

                  fprintf (stderr, "    Dep:   ");
                  SM_print_dep (stderr, dep1);

                  fprintf (stderr, "           ");
                  SM_print_reg_info_operand (stderr,
                                             dep1->from_action->rinfo);
                  fprintf (stderr, "---------> ");
                  SM_print_reg_info_operand (stderr, dep1->to_action->rinfo);
                  fprintf (stderr, "\n");

                  fprintf (stderr, "    To  :");
                  SM_print_oper (stderr, dep1->to_action->sm_op);
                  fprintf (stderr, "\n");



                  mismatch_count++;
                }
              else
                {
                  harmless_count++;
                }
            }

          /* There should not be any deps left with SM_BUILD_DEP_IN now! */
          for (dep1 = op_action->first_dep_in; dep1 != NULL;
               dep1 = dep1->next_dep_in)
            {
              /* Only looking for dependences without SM_BUILD_DEP_OUT */
              if (dep1->flags & SM_BUILD_DEP_OUT)
                continue;

              /* If find an dep without SM_BUILD_DEP_OUT, 
               * check to see if an error message should be
               * printed.  Unfortunately, there is built in asymmetry
               * with the handling of conflicting operand and the
               * handling of predicated code, so an unmatched dependence
               * may not be an error.  Fortunately, the dependences
               * drawn by these asymmetries are "extraneous" and can
               * be left in or deleted without affecting the schedule.
               * So, we just have to detect if the dependence not
               * extraneous, and print the error messages in those cases.
               */
              if (!SM_extraneous_dep (dep1))
                {
                  /* Use global L_fn until get SM_Func going */
                  fprintf (stderr,
                           "SM_check_and_delete_symmetric_deps: %s cb %i:\n",
                           dep1->to_action->sm_op->sm_cb->lcode_fn->name,
                           dep1->to_action->sm_op->sm_cb->lcode_cb->id);

                  fprintf (stderr,
                           "  Dep built only for op %i (with the "
                           "SM_BUILD_DEP_IN flag):\n",
                           dep1->to_action->sm_op->lcode_op->id);


                  fprintf (stderr, "    From:");
                  SM_print_oper (stderr, dep1->from_action->sm_op);

                  fprintf (stderr, "    Dep :  ");
                  SM_print_dep (stderr, dep1);

                  fprintf (stderr, "           ");
                  SM_print_reg_info_operand (stderr,
                                             dep1->from_action->rinfo);
                  fprintf (stderr, "---------> ");
                  SM_print_reg_info_operand (stderr, dep1->to_action->rinfo);
                  fprintf (stderr, "\n");

                  fprintf (stderr, "    To  :");
                  SM_print_oper (stderr, dep1->to_action->sm_op);
                  fprintf (stderr, "\n");

                  mismatch_count++;
                }
              else
                {
                  harmless_count++;
                }
            }

        }
    }

  /* Punt if there are any mismatches */
  if (mismatch_count != 0)
    {
#if 0
      SM_print_cb_dependences (stderr, sm_cb);
#endif

      fprintf (stderr,
               "For these dependences, the same dependences should be "
               "added with SM_BUILD_DEP_OUT\n"
               "as with SM_BUILD_DEP_IN!\n\n");
      if (harmless_count > 0)
        {
          fprintf (stderr,
                   "Note: %i asymmetric but harmless dependences were not "
                   "displayed.\n"
                   "      These deps were believed to be cause by "
                   "predication and/or conflicting\n"
                   "      operand asymmetries and determined to be "
                   "harmless, thus safe to ignore.\n\n", harmless_count);
        }
      fprintf (stderr, "%i dependences not added properly!\n\n",
               mismatch_count);
    }
}

void
SM_build_cb_dependences (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;

  /* For each operation, build its dependences */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* Build the outgoing dependences for each operation */
      SM_build_oper_dependences (sm_cb, sm_op, SM_BUILD_DEP_OUT);

      /* If parameter set, build incoming dependences also.
       * Will check at end to make sure that exactly the
       * same dependences as above will be drawn.
       */
      if (SM_check_dependence_symmetry)
        {
          /* Debug, build the incoming dependences */
          SM_build_oper_dependences (sm_cb, sm_op, SM_BUILD_DEP_IN);
        }
    }

  /* If parameter set, check that for every dependence drawn
   * with SM_BUILD_DEP_OUT, there is also the same dependence
   * drawn with SM_BUILD_DEP_IN!  Punt if there is a mismatch.
   * Deletes the duplicate dependence as this check is done.
   */
  if (SM_check_dependence_symmetry)
    {
      SM_check_and_delete_symmetric_deps (sm_cb);
    }

  if (sm_cb->flags & SM_MODULO_RESOURCES)
    {
      SM_Oper *lb_sm_op;
      SM_Reg_Action *lb_reg_action;

      /* Create control dependences to prevent side exit branches from
       * moving below the loop back branch, even if their predicates
       * do not conflict.
       */

      lb_sm_op = sm_cb->last_serial_op;
      lb_reg_action = lb_sm_op->ext_src[SM_CTRL_ACTION_INDEX];
      for (sm_op = lb_sm_op->prev_serial_op; sm_op != NULL;
	   sm_op = sm_op->prev_serial_op)
	{
	  SM_Reg_Action *br_reg_action;
	  if (!(sm_op->mdes_flags & (OP_FLAG_CBR | OP_FLAG_JMP)))
	    continue;

	  br_reg_action = sm_op->ext_dest[SM_CTRL_ACTION_INDEX];

	  SM_add_dep (br_reg_action, lb_reg_action, SM_HARD_DEP |
		      SM_BUILD_DEP_OUT | SM_CTRL_DEP | SM_FLOW_DEP |
		      SM_FIXED_DELAY,
		      0, 0, 0, SM_HARD_DEP | SM_FORCE_DEP);

	}
    }
}


void
SM_print_action_operand (FILE * out, SM_Reg_Action * action)
{
  SM_Reg_Info *rinfo;
  unsigned int flags;

  /* Handle NULL action case */
  if (action == NULL)
    {
      fprintf (out, "()");
      return;
    }

  /* Get the rinfo for this action */
  rinfo = action->rinfo;

  /* Print out the rinfo we are dealing with */
  if (rinfo->type & SM_REGISTER_TYPE)
    {
      fprintf (out, "(r %d %s", rinfo->id,
               L_ctype_name (rinfo->operand->ctype));
    }
  else if (rinfo->type & SM_MACRO_TYPE)
    {
      fprintf (out, "(mac %s %s", L_macro_name (rinfo->id),
               L_ctype_name (rinfo->operand->ctype));
    }
  else if (rinfo->type & SM_EXT_ACTION_TYPE)
    {
      switch (rinfo->id)
        {
        case SM_MEM_ACTION_INDEX:
          fprintf (out, "(memory");
          break;

        case SM_CTRL_ACTION_INDEX:
          fprintf (out, "(control");
          break;

        case SM_SYNC_ACTION_INDEX:
          fprintf (out, "(sync");
          break;

        case SM_VLIW_ACTION_INDEX:
          fprintf (out, "(vliw");
          break;

        default:
          L_punt ("SM_print_action_operand: Unknown ext id %i!\n", rinfo->id);
        }
    }
  else
    L_punt ("SM_print_action_operand: Unknown reg type %i!\n", rinfo->type);

  /* Print out extension to name for predicate types */
  flags = action->flags;

  if (flags & SM_PRED_UNCOND_DEF)
    fprintf (out, "_u");
  if (flags & SM_PRED_TRANS_DEF)
    fprintf (out, "_t");

  fprintf (out, ")");
}

void
SM_print_oper (FILE * out, SM_Oper * sm_op)
{
  int i;

  fprintf (out, "  op %i %s <", sm_op->lcode_op->id, sm_op->lcode_op->opcode);

  for (i = 0; i < L_max_pred_operand; i++)
    SM_print_action_operand (out, sm_op->pred[i]);

  fprintf (out, "> [");

  for (i = 0; i < L_max_dest_operand; i++)
    SM_print_action_operand (out, sm_op->dest[i]);

  fprintf (out, "] [");

  for (i = 0; i < L_max_src_operand; i++)
    SM_print_action_operand (out, sm_op->src[i]);

  fprintf (out, "]");

  /* Print out flag delimiter if any flag set */
  if (sm_op->flags != 0 || sm_op->ignore != 0)
    fprintf (out, "  < ");

  if (sm_op->ignore)
    fprintf (out, "IGNORE ");

  /* Print out flag delimiter if any flag set */
  if (sm_op->flags != 0 || sm_op->ignore != 0)
    fprintf (out, ">");

  fprintf (out, "\n");
}

void
SM_print_action_id (FILE * out, SM_Reg_Action * action)
{
  fprintf (out, "(");


  if (action->operand_type == MDES_DEST)
    fprintf (out, "dest[%i]", action->operand_number);
  else if (action->operand_type == MDES_SRC)
    fprintf (out, "src[%i] ", action->operand_number);
  else if (action->operand_type == MDES_PRED)
    fprintf (out, "pred[%i]", action->operand_number);
  else if (action->operand_type == MDES_SYNC_OUT)
    fprintf (out, "ext_dest[%i]", action->operand_number);
  else if (action->operand_type == MDES_SYNC_IN)
    fprintf (out, "ext_src[%i] ", action->operand_number);
  else
    L_punt ("SM_print_reg_action: unknown operand type '%i'!",
            action->operand_type);

  if (action->flags & SM_DEF_ACTION)
    fprintf (out, "d");

  if (action->flags & SM_USE_ACTION)
    fprintf (out, "u");

  fprintf (out, " op %-3i)", action->sm_op->lcode_op->id);
}

void
SM_print_dep (FILE * out, SM_Dep * dep)
{
  int flags;

  /* Get dep flags for ease of use */
  flags = dep->flags;


  SM_print_action_id (out, dep->from_action);

  /* Mark ignored dependences -ITI/JCG 8/99 */
  if (dep->ignore)
    fprintf (out, " .. ");
  else
    fprintf (out, " -- ");

  if (flags & SM_VARIABLE_DELAY)
    {
      fprintf (out, "%i-%i,%i", dep->min_delay, dep->max_delay, dep->omega);
    }
  else
    {
      fprintf (out, "%i,%i", dep->min_delay, dep->omega);
    }

  if (dep->ignore)
    fprintf (out, " ..> ");
  else
    fprintf (out, " --> ");

  SM_print_action_id (out, dep->to_action);

  fprintf (out, "  < ");

  if (flags & SM_REG_DEP)
    fprintf (out, "REG ");

  if (flags & SM_MEM_DEP)
    fprintf (out, "MEM ");

  if (flags & SM_CTRL_DEP)
    fprintf (out, "CTRL ");

  if (flags & SM_SYNC_DEP)
    fprintf (out, "SYNC ");

  if (flags & SM_VLIW_DEP)
    fprintf (out, "VLIW ");


  if (flags & SM_FLOW_DEP)
    fprintf (out, "FLOW ");

  if (flags & SM_ANTI_DEP)
    fprintf (out, "ANTI ");

  if (flags & SM_OUTPUT_DEP)
    fprintf (out, "OUTPUT ");

  if (flags & SM_SOFT_DEP)
    fprintf (out, "SOFT ");

  if (dep->ignore)
    fprintf (out, "IGNORE");

  fprintf (out, ">\n");

  /* Debug, warn if dependence is going the wrong way 
   * Only for omega == 0 -ITI/JCG 8/99
   */
  if ((dep->omega == 0) &&
      (dep->from_action->sm_op->serial_number >=
       dep->to_action->sm_op->serial_number))
    {
      fprintf (out, "    Warning: op %i AFTER op %i in the serial order!\n",
               dep->from_action->sm_op->lcode_op->id,
               dep->to_action->sm_op->lcode_op->id);
    }
}

void
SM_print_oper_dependences (FILE * out, SM_Oper * sm_op)
{
  SM_Reg_Action *action;
  SM_Dep *dep_in, *dep_out;

  fprintf (out, "  [%i, %i]", sm_op->num_hard_dep_in,
           sm_op->num_hard_dep_out);
  SM_print_oper (out, sm_op);

  /* Print dependences into this op */
  for (action = sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_in = action->first_dep_in; dep_in != NULL;
           dep_in = dep_in->next_dep_in)
        {
          fprintf (out, "     IN  ");
          SM_print_dep (out, dep_in);
        }
    }


  /* Print dependences out of this op */
  for (action = sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_out = action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
          fprintf (out, "     OUT ");
          SM_print_dep (out, dep_out);
        }
    }

  fprintf (out, "\n");
}

void
SM_print_cb_dependences (FILE * out, SM_Cb * sm_cb)
{
  SM_Oper *sm_op;

  fprintf (out, "Dependence graph for cb %i:\n\n", sm_cb->lcode_cb->id);
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      SM_print_oper_dependences (out, sm_op);
    }
}


/* 20040712SZU
 * Adapted from SM_insert_ds_sync_dep by IMS
 */
/** \fn void SM_insert_sync_dep_betw_ops (SM_Oper* from_op, SM_Oper * to_op)
 *
 * \brief Create a SYNC_DEP between from_op and to_op.
 *
 * \param from_op
 * \param to_op
 */
void
SM_insert_sync_dep_betw_ops (SM_Oper *from_op, SM_Oper *to_op)
{
  int flags, mode;
  SM_Reg_Action *from_action, *to_action;

#if 0
  /* we are going to create a new dep now */
  if (SM_print_scheduling_trace)
    printf(">> Created SYNC_DEP from %d (%s) to %d (%s)\n",
	   from_op->lcode_op->id, from_op->lcode_op->opcode,
	   to_op->lcode_op->id, to_op->lcode_op->opcode);
#endif

  /* these are just some variable defs for easy use */
  from_action = from_op->ext_dest[SM_SYNC_ACTION_INDEX];
  to_action = to_op->ext_src[SM_SYNC_ACTION_INDEX]; 

  flags = SM_BUILD_DEP_OUT;
  mode = SM_HARD_DEP | SM_SOFT_DEP | SM_FORCE_DEP;

  /* need to make sure that we are ready for sync deps */
  if (!from_action)
    from_action = SM_add_reg_action(from_op, MDES_SYNC_OUT,
				   SM_SYNC_ACTION_INDEX,
				   SM_SYNC_ACTION_OPERAND);
  if (!to_action)
    to_action = SM_add_reg_action(to_op, MDES_SYNC_IN,
				    SM_SYNC_ACTION_INDEX,
				    SM_SYNC_ACTION_OPERAND);
				   
  SM_add_dep (from_action, to_action,
	      flags | SM_SYNC_DEP | SM_FIXED_DELAY,
	      0, 0, 0, mode);
}


void
SM_print_dep_dot (FILE *out, SM_Dep *dep)
{
  int flags = dep->flags;
  int src_id, snk_id;
  char lab[256], tmp[128], *col, *sty, *arrowhead, *constraint;

#if 0 /* this is weak...lets use these to show what type of
	 dep we are dealing with */
  sty = "solid";
  col = "red";
#endif

  lab[0] = '\0';

  src_id = dep->from_action->sm_op->lcode_op->id;
  snk_id = dep->to_action->sm_op->lcode_op->id;

  if (flags & SM_VARIABLE_DELAY)
    sprintf (tmp, "[%i-%i,%i]", dep->min_delay, dep->max_delay, dep->omega);
  else
    sprintf (tmp, "[%i,%i]", dep->min_delay, dep->omega);

  strcat (lab, tmp);

  /* color / style defaults */
  sty = "solid";
  col = "black";
  arrowhead = "normal";
  constraint = "true";
  
  if (flags & SM_REG_DEP)
    col = "red";

  if (flags & SM_MEM_DEP)
    col = "orange";

  if (flags & SM_CTRL_DEP)
    col = "darkturquoise";

  if (flags & SM_SYNC_DEP)
    col = "grey";

#if 0 /* this is not ever used */
  if (flags & SM_VLIW_DEP)
    col = "black";
#endif

  if (flags & SM_FLOW_DEP)
    arrowhead = "normal";

  if (flags & SM_ANTI_DEP)
    arrowhead = "tee";

  if (flags & SM_OUTPUT_DEP)
    arrowhead = "dot";

#if 0
  if ((flags & SM_SOFTFIX_DATASPEC) && (flags & SM_SOFT_DEP))
    {
      constraint = "false";
    }
#endif

  if (flags & SM_SOFT_DEP)
    {
      sty = "dashed";
      constraint = "false";
    }

  if (dep->ignore)
    sty = "dotted";    

  fprintf (out, "\t\"op %d\" -> \"op %d\" "
	   "[label=\"%s\",color=%s,style=%s,"
	   "arrowhead=%s,"
	   "constraint=%s];\n",
	   src_id, snk_id, 
	   lab, col, sty, 
	   arrowhead,
	   constraint);

  return;
}


void
SM_print_oper_deps_dot (FILE * out, SM_Oper * sm_op)
{
  SM_Reg_Action *action;
  SM_Dep *dep_out;

  /* Print dependences out of this op */
  for (action = sm_op->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_out = action->first_dep_out; dep_out != NULL;
           dep_out = dep_out->next_dep_out)
        {
#if 0
	  /* lets just ignore self deps to clean up
	   * the picture
	   */
	  if (dep_out->from_action->sm_op == dep_out->to_action->sm_op)
	    continue;
#endif
	  SM_print_dep_dot (out, dep_out);
        }
    }
}


void
SM_print_cb_deps_dot (FILE *out, SM_Cb *sm_cb)
{
  SM_Oper *sm_op;
  int id;
  char *color;

  fprintf (out, "digraph G {\n");
  fprintf (out, "\tsize=\"7.5,10\"\n");

  fprintf (out, "\"key1\" [shape=box,label=\"=== KEY - NODES ===\\n "
	   "Yellow: Unscheduled\\n "
	   "Orange: Ready Queue\\n "
	   "Red: Scheduled\\n\"];\n");
  fprintf (out, "\"key2\" [shape=box,label=\"=== KEY - EDGES ===\\n "
	   "Red: Register\\n "
	   "Orange: Memory\\n "
	   "Blue: Control\\n "
	   "Sync: Grey\\n "
	   "Dashed: Soft\\n "
	   "Dotted: Ignored\"];\n");
  
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      id = sm_op->lcode_op->id;
      if (sm_op->flags & SM_OP_SCHEDULED)
	color = "red";
#if 0
      else if (SM_search_oper_queue(sm_cb->dep_in_resolved, sm_op))
	color = "orange";
#endif
      else
        color = "yellow";
      
      fprintf (out, "\t\"op %d\" [shape=ellipse,"
	       "style=filled,color=%s,"
	       "label=\"op %d %s\"];\n", 
	       id, color, id, 
	       L_opcode_name (sm_op->lcode_op->opc));
    }

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      SM_print_oper_deps_dot (out, sm_op);
    }

  fprintf (out, "}\n");
  return;
}

void
SM_print_cb_dot_graph (SM_Cb *sm_cb, char *file)
{
  FILE *Fout;

  if (!(Fout = fopen (file, "w")))
    L_punt ("SM_print_cb_dot_graph: unable to open output file");

  SM_print_cb_deps_dot (Fout, sm_cb);
  fclose (Fout);
  return;
}

void
SM_print_cb_deps_dot2 (SM_Cb * sm_cb)
{
  FILE * dot_file;

  dot_file = fopen("cb_deps.dot", "w");
  SM_print_cb_deps_dot (dot_file, sm_cb);
  fclose(dot_file);
}
