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
 *      File:   sm_priority.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  August 1996
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include "sm.h"
#include <Lcode/l_main.h>
#include <library/l_alloc_new.h>
#include <library/heap.h>

void
SM_calculate_priorities (SM_Cb * sm_cb, int min_early_time)
{
  SM_Oper *sm_op;
  double *prob, exit_prob;
  float priority, add_priority;
  int *late_array, late_time;
  int max_late_time, index, num_exits;
  int op_min_late_time, op_max_late_time;

  /* Calculate the early and late times for this cb */
  max_late_time = SM_calculate_early_times (sm_cb, min_early_time);
  SM_calculate_late_times (sm_cb, max_late_time);

  /* Use the exit percentage array as a probability array in calc */
  prob = sm_cb->exit_percentage;

  num_exits = sm_cb->num_exits;

  /* Compute the priority of each operation (using speculative yield) */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      priority = 0.0;

      late_array = sm_op->late_time;

      op_min_late_time = max_late_time; op_max_late_time = 0;

      for (index = 0; index < num_exits; index++)
        {
	  int chain_ht;

          late_time = late_array[index];
          if (late_time == SM_MAX_CYCLE)
            continue;

	  if (late_time < op_min_late_time)
	    op_min_late_time = late_time;

	  if (late_time > op_max_late_time)
	    op_max_late_time = late_time;

          /* Get the exit probability.  For zero weight exits, use
           * prob of .00001 so they are taking into account somehow.
           * (This is how it is done in the old dhasy scheduler.)
           */
          if ((exit_prob = prob[index]) < 0.00001)
            exit_prob = .00001;

	  chain_ht = max_late_time - late_time;

          add_priority = (1.0 + (float) chain_ht) * exit_prob;
          priority += add_priority;

#if 0
          if (sm_op->lcode_op->id == 12)
            {
              printf ("index %i exit_prob %f late_time %i add %f total %f\n",
                      index, exit_prob, late_time, add_priority, priority);
            }
#endif
        }
#if 0
      if (sm_op->lcode_op->id == 12)
        {
          printf ("Total priority %f (num exits %i, max_late_time %i)\n\n",
                  priority, num_exits, max_late_time);
        }
#endif
#if 1
      {
        L_Attr *attr;

        attr = L_find_attr (sm_op->lcode_op->attr, "BEST_PRIC_LOC");
        if (attr && attr->field[0]->value.i == 3)
          {
            /* Up the priority */
            priority = priority * 4;

          }

      }
#endif
#if 0
      /* DIA - FOR TESTING: Add something to schedule longer latency
       * ops first when all other things are equal.  Right now, use
       * only first operand's largest min_delay latency.  
       */
      if (sm_op->dest[0])
        {
          max_latency = 0;
          for (dep_out = sm_op->dest[0]->first_dep_out;
               dep_out != NULL; dep_out = dep_out->next_dep_out)
            {
              if (dep_out->min_delay > max_latency)
                max_latency = dep_out->min_delay;

            }
          /* DIA - priority is tweaked ever so slightly by latency */
          priority -= ((float) max_latency) * 0.000001;
        }
#endif

      sm_op->priority = priority;

      /* 20030227 SZU
       * SMH reconciliation
       */
      if (sm_cb->prepass_sched)
	{
	  L_Attr *pattr;

	  pattr = L_new_attr ("elp", 4);
	  L_set_int_attr_field (pattr, 0, sm_op->early_time);
	  L_set_int_attr_field (pattr, 1, op_min_late_time);
	  L_set_int_attr_field (pattr, 2, op_max_late_time);
	  L_set_double_attr_field (pattr, 3, priority);
	  sm_op->lcode_op->attr = L_concat_attr (sm_op->lcode_op->attr, pattr);
	}
    }
  return;
}

/* Calculates the early time for each operation.
 * If the operation is scheduled, the scheduled time is used for
 * the early time, otherwise dependences will be used to set
 * the early time (with a mimimum setting of min_early_time).
 *
 * Returns the maximum of all the early times calculated
 * (this is usually used as the max_late_time for calculating late times).
 */
int
SM_calculate_early_times (SM_Cb * sm_cb, int min_early_time)
{
  SM_Oper *sm_op;
  SM_Reg_Action *op_action;
  SM_Dep *dep_in;
  int early_time, constraint, max_early_time;
  unsigned int ignore_mask;

  max_early_time = SM_MIN_CYCLE;

  /* Treat some ignored dependences as enabled for priority calc */
  ignore_mask = ~sm_cb->special_dep_ignore_flags;

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* If operation is scheduled, use this time as the early time */
      if (sm_op->flags & SM_OP_SCHEDULED)
        {
          early_time = sm_op->sched_cycle;
        }

      /* Otherwise, calculate the early time for this operation 
       * based on dependences coming into this operation
       */
      else
        {
          /* Start at min_early_time and increase with dep constraints */
          early_time = min_early_time;

          for (op_action = sm_op->first_op_action; op_action != NULL;
               op_action = op_action->next_op_action)
            {
              for (dep_in = op_action->first_dep_in; dep_in != NULL;
                   dep_in = dep_in->next_dep_in)
                {
                  /* Ignore deps marked with SM_IGNORE_DEP */
                  if (dep_in->ignore & ignore_mask)
                    continue;

                  /* Ignore cross-iteration dependences -ITI/JCG 8/99 */
                  if (dep_in->omega != 0)
                    continue;

                  /* Get the constraint this dep imposes */
                  constraint = dep_in->from_action->sm_op->early_time +
                    dep_in->min_delay;

                  /* Update early time with this constraint if necessary */
                  if (early_time < constraint)
                    early_time = constraint;
                }
            }
        }

      /* Set the early time for the operation */
      sm_op->early_time = early_time;

      /* Update max_early_time if necessary */
      if (max_early_time < early_time)
        max_early_time = early_time;
    }

  return (max_early_time);
}

/* Recalculates the early time for the operation.
 * If the operation is scheduled, the scheduled time is used for
 * the early time, otherwise dependences will be used to set
 * the early time (with a mimimum setting of min_early_time).
 *
 * Sets sm_op->early_time and also returns the new early time for 
 * this operation.
 */
int
SM_recalculate_early_time (SM_Oper * sm_op, int min_early_time)
{
  SM_Reg_Action *op_action;
  SM_Dep *dep_in;
  int early_time, constraint;
  unsigned int ignore_mask;

  /* Treat some ignored dependences as enabled for priority calc */
  ignore_mask = ~sm_op->sm_cb->special_dep_ignore_flags;

  /* If operation is scheduled, use this time as the early time */
  if (sm_op->flags & SM_OP_SCHEDULED)
    {
      early_time = sm_op->sched_cycle;
    }

  /* Otherwise, calculate the early time for this operation 
   * based on dependences coming into this operation
   */
  else
    {
      /* Start at min_early_time and increase with dep constraints */
      early_time = min_early_time;

      for (op_action = sm_op->first_op_action; op_action != NULL;
           op_action = op_action->next_op_action)
        {
          for (dep_in = op_action->first_dep_in; dep_in != NULL;
               dep_in = dep_in->next_dep_in)
            {
              /* Ignore deps marked with SM_IGNORE_DEP */
              if (dep_in->ignore & ignore_mask)
                continue;

              /* Ignore cross-iteration dependences -ITI/JCG 8/99 */
              if (dep_in->omega != 0)
                continue;

              /* Get the constraint this dep imposes */
              constraint = dep_in->from_action->sm_op->early_time +
                dep_in->min_delay;

              /* Update early time with this constraint if necessary */
              if (early_time < constraint)
                early_time = constraint;
            }
        }
    }

  /* Set the early time for the operation */
  sm_op->early_time = early_time;

  /* Return the new early time for this operation */
  return (early_time);
}

void
SM_calculate_late_times (SM_Cb * sm_cb, int max_late_time)
{
  SM_Oper *sm_op, *exit_op;
  SM_Reg_Action *op_action;
  SM_Dep *dep_in;
  int *late_array, *from_late_array;
  int late_time, constraint, num_exits, index;
  int initial_late_time, min_delay;
  unsigned int ignore_mask;

  /* Treat some ignored dependences as enabled for priority calc */
  ignore_mask = ~sm_cb->special_dep_ignore_flags;

  /* Get the number of exits for ease of use */
  num_exits = sm_cb->num_exits;

  /* Initially all late times are set to 2000000000.  
   * If after this algorithm runs it is still 2000000000, then
   * that operation is not required by that particular exit.
   */
  initial_late_time = SM_MAX_CYCLE;

  /* Create late_time array (if necessary) and initialize
   * all late times to initial_late_time initially
   */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      /* Get the late time array for ease of use */
      late_array = sm_op->late_time;

      /* Create array if necessary */
      if (late_array == NULL)
        {
          if ((late_array = (int *) malloc (sizeof (int) * num_exits)) ==
              NULL)
            L_punt ("SM_calculate_late_times: Out of memory");
          sm_op->late_time = late_array;
        }

      /* Initialize all late times to initial_late_time */
      for (index = 0; index < num_exits; index++)
        {
          late_array[index] = initial_late_time;
        }
    }

  /* Initialize the late times for all the exit ops
   * including fall-thru ops for the fall-thru case.
   */
  for (index = 0; index < num_exits; index++)
    {
      /* Get the operation for this exit for ease of use */
      exit_op = sm_cb->exit_op[index];

      /* If actually have an operation for the exit, set its late
       * time to its early time for this index.
       */
      if (exit_op != NULL)
        {
          if (!(exit_op->ignore))
            exit_op->late_time[index] = exit_op->early_time;
        }
      else
        {
          /* Otherwise, scan the operation list and for operations with
           * no output dependences (thus must be used on fall-thru), set
           * the late time for this "fall-thru" index to the maximum
           * early time in the cb.
           */
          for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
               sm_op = sm_op->next_serial_op)
            {
              /* Only consider ops with no output dependences */
              if ((sm_op->num_hard_dep_out == 0) &&
                  (sm_op->num_soft_dep_out == 0) && (!sm_op->ignore))
                {
                  sm_op->late_time[index] = max_late_time;
                }
            }
        }
    }

  /* Go though each operation, propagating the late time constraints
   * for each exit as we go.  
   * 
   * Note: I moved the exit index loop from the outer loop to
   *       the inner loops to speed this routine up.  This
   *       more than halved the time spent in this routine. -JCG 2/26/97
   */
  for (sm_op = sm_cb->last_serial_op; sm_op != NULL;
       sm_op = sm_op->prev_serial_op)
    {
      /* Get the late time array for ease of use */
      late_array = sm_op->late_time;

      /* If this operation has been scheduled, force its late time
       * to be its scheduled time (for those exits that depend
       * on this operation).
       */
      if (sm_op->flags & SM_OP_SCHEDULED)
        {
          for (index = 0; index < num_exits; index++)
            {
              /* Get the late time for this operation from earlier
               * dependences
               */
              late_time = late_array[index];

              /* If this operation is not required for this exit 
               * index, then don't update it's late time 
               */
              if (late_time == initial_late_time)
                continue;

              /* Sanity check.  The scheduled cycle should not be later
               * than the calculated late time.
               */
              if (sm_op->sched_cycle > late_time)
                {
                  fprintf (stderr,
                           "SM_calculate_late_time: %s op %i index %i\n"
                           "  sched cycle (%i) > calculated late time (%i)\n",
                           sm_cb->lcode_fn->name, sm_op->lcode_op->id, index,
                           sm_op->sched_cycle, late_time);
                }
              late_array[index] = sm_op->sched_cycle;
            }
        }

      /* Update the late times of all operations that this
       * operation depends on (if necessary).
       */
      for (op_action = sm_op->first_op_action; op_action != NULL;
           op_action = op_action->next_op_action)
        {
          for (dep_in = op_action->first_dep_in; dep_in != NULL;
               dep_in = dep_in->next_dep_in)
            {
              /* Ignore deps marked with SM_IGNORE_DEP */
              if (dep_in->ignore & ignore_mask)
                continue;

              /* Ignore cross-iteration dependences -ITI/JCG 8/99 */
              if (dep_in->omega != 0)
                continue;

              /* For ease of use, get the min delay and the
               * from_late_array for this dep 
               */
              min_delay = dep_in->min_delay;
              from_late_array = dep_in->from_action->sm_op->late_time;

              for (index = 0; index < num_exits; index++)
                {
                  /* Get the late time for this operation from earlier
                   * dependences for this exit.
                   */
                  late_time = late_array[index];

                  /* If this operation is not required for this exit 
                   * index, then don't update any of the operations it
                   * depends on.
                   */
                  if (late_time == initial_late_time)
                    continue;

                  /* Get the constraint this dep imposes */
                  constraint = late_time - min_delay;

                  /* For this exit index, update the late time of the 
                   * operation this dep is from (if necessary)
                   */
                  if (from_late_array[index] > constraint)
                    {
                      from_late_array[index] = constraint;
                    }
                }
            }
        }
    }
}

void
SM_print_early_and_late_times (FILE * out, SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  int num_exits, index;

  fprintf (out, "Early and late times for %s cb %i:\n",
           sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);

  num_exits = sm_cb->num_exits;

  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      fprintf (out, "  Op %3i %-7s %5.2f %3i    ",
               sm_op->lcode_op->id, sm_op->lcode_op->opcode,
               sm_op->priority, sm_op->early_time);

      for (index = 0; index < num_exits; index++)
        {
          if (sm_op->late_time[index] == SM_MAX_CYCLE)
            fprintf (out, "   -");
          else
            fprintf (out, " %3i", sm_op->late_time[index]);
        }
      fprintf (out, "\n");
    }

  fprintf (out, "\n");
}
