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
 *      File:   sm_main.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  January 1997
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#define MD_DEBUG_MACROS         /* Use debug version of md */
#include "sm.h"
#include <library/l_alloc_new.h>
#include <library/l_parms.h>
#include <library/dynamic_symbol.h>
#include <library/heap.h>
#include <Lcode/l_main.h>

/* Turn off some of the more expensive trans checks for Ph.D. runs. */
#define CHECK_TRANS 1

/* Parameters to control optimizer */
int do_classic_application = 0;
int do_linear_search = 0;
int do_renaming_with_copy = 1;
int do_expression_reformulation = 1;
char *prof_info = "real";


char *opti_lmdes_file_name =
  "/home/bajor/gyllen/phd/hppa/parms/HP_128G.lmdes2";

/* Set do_expr_without_copy_only when emulating the classic version
 * of expression rewritting that did not create any new operations.
 */
int do_expr_without_copy_only = 0;

/* Set use_classic_renaming_heuristics to 1 when emulating the
 * heuristics used by Lsuperscalar when applying renaming with copy.
 */
int use_classic_renaming_heuristics = 0;

Mdes *opti_lmdes = NULL;
Mdes *eval_lmdes = NULL;

int verbose_optimization = 0;

int print_cb_histogram = 0;
int print_top_cb_stats = 0;
int print_total_stats = 1;
int suppress_lcode_output = 0;

/* If set to one, write a md file will a bunch of cb stats that I am
 * using to generate my phd graphs. -JCG
 */
int write_phd_stats = 0;
char *phd_stats_file_name = "./phd_stats";

int always_undo_opti = 0;
int max_opti_passes = 10;
double min_cb_opti_weight = 0.0;

int opti_cb_lower_bound = -1000000000;
int opti_cb_upper_bound = 1000000000;

int opti_oper_lower_bound = -1000000000;
int opti_oper_upper_bound = 1000000000;

int phd_flags = 0;
int phd_performed_flags = 0;

double picked_cycles = 0.0;
double orig_cycles = 0.0;
double trival_expr_cycles = 0.0;
double rename_cycles = 0.0;
double total_cycles = 0.0;
INT_Symbol_Table *diff_table = NULL;
double total_diff = 0.0;
int diff_count = 0;
int RWC_possible = 0;
int EWC_possible = 0;

time_t start_time = 0;
time_t end_time = 0;

/* Debug, delete and recreate every operation in the cb */
void
SM_test_insert_and_delete (SM_Cb * sm_cb)
{
  SM_Oper *sm_op, *next_sm_op, *prev_sm_op;
  L_Oper *lcode_op;

  /* Scan through each sm_op, delete it and then create it again. */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL; sm_op = next_sm_op)
    {
      /* Get the next and prev sm_op before doing anything */
      next_sm_op = sm_op->next_serial_op;
      prev_sm_op = sm_op->prev_serial_op;

      /* Skip branches for now (just unschedule to be consistent) */
      if (sm_op->mdes_flags & (OP_FLAG_RTS | OP_FLAG_CBR | OP_FLAG_JMP))
        {
          SM_unschedule_oper (sm_op, NULL);
          continue;
        }

      /* Get the lcode op for this sm_op */
      lcode_op = sm_op->lcode_op;

      /* Delete the current oper */
      SM_delete_oper (sm_op);

      /* Remove the current oper from it's cb */
      L_remove_oper (sm_cb->lcode_cb, lcode_op);

      /* Insert the operation back into the lcode and sm cb */
      SM_insert_oper_after (sm_cb, lcode_op, prev_sm_op);
    }
}

typedef struct Cb_Stats
{
  char *fn_name;
  int cb_id;
  double orig_cycles;
  double opti_cycles;
  struct Cb_Stats *next_stats;

}
Cb_Stats;

L_Alloc_Pool *Cb_Stats_pool = NULL;

Cb_Stats *cb_stats_head = NULL;

void
update_cb_stats (SM_Cb * sm_cb, double orig_cycles, double opti_cycles)
{
  Cb_Stats *stats;

  if (Cb_Stats_pool == NULL)
    Cb_Stats_pool = L_create_alloc_pool ("Cb_Stats", sizeof (Cb_Stats), 64);

  stats = (Cb_Stats *) L_alloc (Cb_Stats_pool);
  stats->fn_name = strdup (sm_cb->lcode_fn->name);
  stats->cb_id = sm_cb->lcode_cb->id;
  stats->orig_cycles = orig_cycles;
  stats->opti_cycles = opti_cycles;

  stats->next_stats = cb_stats_head;
  cb_stats_head = stats;
}

void
print_top_weight_stats (FILE * out, int max_count)
{
  Heap *heap;
  Cb_Stats *stats;
  int count;

  fprintf (out, "Top %i cbs by weight:\n", max_count);

  /* Create heap */
  heap = Heap_Create (HEAP_MAX);

  /* Add all cb stats to heap */
  for (stats = cb_stats_head; stats != NULL; stats = stats->next_stats)
    {
      Heap_Insert (heap, (void *) stats, (double) stats->opti_cycles);
    }

  /* Pull off stats in sorted order */
  count = 0;
  while ((stats = (Cb_Stats *) Heap_ExtractTop (heap)) != NULL)
    {
      if (stats->orig_cycles > .1)
        {
          fprintf (out,
                   "%5.2f%%: %s cb %i orig %.0f opti %.0f diff %5.2f%%\n",
                   100.0 * stats->opti_cycles / total_cycles,
                   stats->fn_name, stats->cb_id,
                   stats->orig_cycles, stats->opti_cycles,
                   100.0 * (stats->orig_cycles - stats->opti_cycles) /
                   stats->orig_cycles);
        }
      count++;
      if (count >= max_count)
        break;
    }

  /* Dispose of heap, no need to free remaining stats */
  heap = Heap_Dispose (heap, NULL);
  printf ("\n");
}

void
print_top_diff_stats (FILE * out, int max_count)
{
  Heap *heap;
  Cb_Stats *stats;
  int count;

  fprintf (out, "Top %i cbs by opti benefit:\n", max_count);

  /* Create heap */
  heap = Heap_Create (HEAP_MAX);

  /* Add all cb stats to heap */
  for (stats = cb_stats_head; stats != NULL; stats = stats->next_stats)
    {
      Heap_Insert (heap, (void *) stats,
                   (stats->orig_cycles - stats->opti_cycles));
    }

  /* Pull off stats in sorted order */
  count = 0;
  while ((stats = (Cb_Stats *) Heap_ExtractTop (heap)) != NULL)
    {
      if (stats->orig_cycles > .1)
        {
          fprintf (out,
                   "%5.2f%%: %s cb %i orig %.0f benefit %.0f diff %5.2f%%\n",
                   100.0 * stats->opti_cycles / total_cycles,
                   stats->fn_name, stats->cb_id,
                   stats->orig_cycles,
                   (stats->orig_cycles - stats->opti_cycles),
                   100.0 * (stats->orig_cycles - stats->opti_cycles) /
                   stats->orig_cycles);
        }
      count++;
      if (count >= max_count)
        break;
    }

  /* Dispose of heap, no need to free remaining stats */
  heap = Heap_Dispose (heap, NULL);
  printf ("\n");
}

void
update_diff_stats (double percent_diff)
{
  int int_diff, count;
  INT_Symbol *symbol;

  int_diff = (int) (percent_diff);

  if ((symbol = INT_find_symbol (diff_table, int_diff)) == NULL)
    symbol = INT_add_symbol (diff_table, int_diff, (void *) 0);

  /* Increment data value */
#ifdef LP64_ARCHITECTURE
  count = (int)((long)symbol->data);
#else
  count = (int) symbol->data;
#endif
  count++;
#ifdef LP64_ARCHITECTURE
  symbol->data = (void *)((long)count);
#else
  symbol->data = (void *) count;
#endif

  /* Update total stats */
  total_diff += percent_diff;
  diff_count++;
}

void
print_diff_stats (FILE * out)
{
  Heap *heap;

  INT_Symbol *symbol;

  fprintf (out,
           "\nHistogram of the optimization's effect on each cb "
           "(percent improvement):\n");

  /* Create heap */
  heap = Heap_Create (HEAP_MIN);

  /* Add all symbols to heap */
  for (symbol = diff_table->head_symbol; symbol != NULL;
       symbol = symbol->next_symbol)
    {
      Heap_Insert (heap, (void *) symbol, (double) symbol->value);
    }
  /* Pull of symbols in sorted order */
  while ((symbol = (INT_Symbol *) Heap_ExtractTop (heap)) != NULL)
    {
#ifdef LP64_ARCHITECTURE
      fprintf (out, "%4i%%: %5i\n", symbol->value, (int)((long)symbol->data));
#else
      fprintf (out, "%4i%%: %5i\n", symbol->value, (int) symbol->data);
#endif
    }

  /* Dispose of heap, should be empty, no free routine needed */
  heap = Heap_Dispose (heap, NULL);

  fprintf (out, "\n");

#if 0
  /* Print average diff per cb */
  fprintf (out, "\n Average diff/cb = %4.2f\n",
           total_diff / (double) diff_count);
#endif
}

void
SM_summarize_trans_queue (FILE * out, SM_Trans_Queue * queue)
{
  SM_Trans_Qentry *qentry;
  SM_Trans *trans;

  fprintf (out, " ");
  for (qentry = queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      /* Print out trans indentifier */
      trans = qentry->trans;

      switch (trans->type)
        {
          /* Note, since transformations can delete operations,
           * it is not safe to access trans->target_sm_op, etc.
           */
        case RENAMING_WITH_COPY:
          fprintf (out, " RWC:%i", trans->op_id);
          break;

        case EXPR_WITH_COPY:
          fprintf (out, " EXPR:%i", trans->op_id);
          break;

        default:
          L_punt ("SM_summarize_trans_queue: unknown trans type %i",
                  trans->type);
        }
    }
  fprintf (out, "\n");
}

/* Emulates (almost exactly) the Lsuperscalar renaming_with_copy heuristics.
 *  
 * Returns the new height of the cb after applying these transformations.
 */
double
SM_do_classic_renaming_with_copy (SM_Cb * sm_cb)
{
  SM_Trans_Queue *trans_queue;
  SM_Trans *trans;
  double new_height;

  /* Set flag to indicate classic heuristics should be
   * used to pick potential transformations.
   */
  use_classic_renaming_heuristics = 1;

  /* Unschedule the cb before doing transformations */
  SM_unschedule_cb (sm_cb);

  /* Build transformation queue from the sm_cb */
  trans_queue = SM_find_potential_trans (sm_cb, RENAMING_WITH_COPY);

  if (trans_queue->first_qentry != NULL)
    {
      phd_flags |= RENAMING_WITH_COPY;
      phd_performed_flags |= RENAMING_WITH_COPY;
    }

  /* Do every transformation in the trans queue */
  while (trans_queue->first_qentry != NULL)
    {
      /* Get the trans before dequeuing it */
      trans = trans_queue->first_qentry->trans;
      SM_dequeue_trans (trans_queue->first_qentry);

      /* Do the transformation */
      SM_do_trans (trans);

      /* Delete trans after done with it */
      SM_delete_trans (trans);
    }
  SM_delete_trans_and_queue (trans_queue);

  /* Schedule the cb and calculate its new height */
  SM_schedule_cb (sm_cb);
  new_height = SM_calc_best_case_cycles (sm_cb);

  return (new_height);
}


/* This version creates the potential trans queue only once and
 * at the end tries to undo transformations that show no benefit.
 * 
 * This routine should only be used for renaming_with_copy.
 * The expression reformulation optimizations require the trans
 * queue to be continously rebuilt and these reformulations make
 * straightforward undoing impossible.  However, it was found
 * to be unbeneficial anyway to do expression reformulation at the
 * the same time a renaming with copy, so it is not a problem.
 *
 * Returns the cb's height after the transformations (if any).
 */
double
SM_do_linear_search (SM_Cb * sm_cb, double initial_height,
                     unsigned int allowed_trans)
{
  SM_Trans_Queue *untested_queue, *performed_queue, *undone_queue;
  SM_Trans_Qentry *qentry;
  SM_Trans *trans;
  double best_height, new_height, trans_height;
  int num_trans;

  /* Build transformation queue from the sm_cb */
  untested_queue = SM_find_potential_trans (sm_cb, allowed_trans);

  /* If queue empty, delete and return now */
  if (untested_queue->first_qentry == NULL)
    {
      SM_delete_trans_and_queue (untested_queue);
      return (initial_height);
    }

  /* For phd stats, flag that transformations wer found for
   * these allowed trans.
   */
  phd_flags |= allowed_trans;

  /* Get the number of transformations in queue */
  num_trans = untested_queue->num_qentries;

  /* Create queue for those transformations we decide to do */
  performed_queue = SM_new_trans_queue ("Performed");
  undone_queue = SM_new_trans_queue ("Undone");

  /* Initially, the best height is the initial height */
  best_height = initial_height;

  /* Before unscheduling, update schedule-based trans-info */
  if (sm_cb->num_unsched > 0)
    SM_schedule_cb (sm_cb);
  SM_update_sched_based_trans_info (untested_queue);

  /* Unschedule the cb before calculing priorities */
  SM_unschedule_cb (sm_cb);

  /* Test transformations in priority order */
  while ((trans = SM_dequeue_best_do_trans (untested_queue)) != NULL)
    {
      /* Do the transformation */
      SM_do_trans (trans);

      /* Reschedule cb and see if get better height */
      SM_schedule_cb (sm_cb);
      new_height = SM_calc_best_case_cycles (sm_cb);

      /* Keep trans if it helps performance, or, if doesn't hurt 
       * performance and it get scheduled in the same cycle or earlier
       */
      if ((!always_undo_opti) &&
          ((new_height < best_height) ||
           ((new_height == best_height) &&
            (trans->target_sm_op->sched_cycle <= trans->target_sched_cycle))))
        {
          /* Update height */
          best_height = new_height;

          /* Add to performed queue */
          SM_enqueue_trans_before (performed_queue, trans, NULL);

          /* Update trans info with the new schedule */
          SM_update_sched_based_trans_info (untested_queue);

          /* Unschedule the cb now, so the next priority calculation
           * will not be messed up (but after updating trans info)
           */
          SM_unschedule_cb (sm_cb);
        }

      /* Otherwise, undo transformation and delete it */
      else
        {
          /* Don't update sched-based trans info since didn't to trans */

          /* Need to unschedule the cb before undoing the trans 
           * (since the schedule may be invalid withouth the transformation
           *  and the sm manager punts when this happends).
           */
          SM_unschedule_cb (sm_cb);

          SM_undo_trans (trans);

#if CHECK_TRANS
          /* Reschedule cb and see if get back best height */
          SM_schedule_cb (sm_cb);
          new_height = SM_calc_best_case_cycles (sm_cb);

          if (new_height != best_height)
            {
              fprintf (stderr,
                       "ERROR: %s op %i before trans %.0f after undo %.0f!\n",
                       sm_cb->lcode_fn->name,
                       trans->op_id, best_height, new_height);
            }
          SM_unschedule_cb (sm_cb);
#endif

          SM_delete_trans (trans);
        }
    }

  /* If performed transformations, update keep flags for
   * phd stats, and if desired, print out trans list
   */
  if (performed_queue->first_qentry != NULL)
    {
      phd_performed_flags |= allowed_trans;

      if (verbose_optimization)
        {
          printf ("%s cb %i, linear search performed %i of %i"
                  " transformations:\n",
                  sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                  performed_queue->num_qentries, num_trans);
          SM_summarize_trans_queue (stdout, performed_queue);
          printf ("\n");
        }
    }

  /* Save the trans height so can tell if undoing helps performance */
  trans_height = best_height;

  /* Unschedule before possibly undoing a transformation */
  SM_unschedule_cb (sm_cb);

  /* Unschedule the cb before undoing the trans 
   * For now, only for renaming with copy 
   */
  for (qentry = performed_queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      /* Get the trans for ease of use */
      trans = qentry->trans;

      /* If cannot legally undo this trans, skip it */
      if (!SM_can_undo_trans (trans))
        continue;

      /* Undo the transformation */
      SM_undo_trans (trans);

      /* Reschedule cb and see if get better height */
      SM_schedule_cb (sm_cb);
      new_height = SM_calc_best_case_cycles (sm_cb);

      /* Unschedule before possibly redoing transformation */
      SM_unschedule_cb (sm_cb);

      /* Undo trans if undoing it doesn't hurt performance */
      if (new_height <= best_height)
        {
          /* Update height */
          best_height = new_height;

          /* Add to undo queue */
          SM_enqueue_trans_before (undone_queue, trans, NULL);
        }
      /* Otherwise, redo the transformation */
      else
        {
          SM_do_trans (trans);

#if CHECK_TRANS
          /* Debug */
          /* Reschedule cb and see if get back best height */
          SM_schedule_cb (sm_cb);
          new_height = SM_calc_best_case_cycles (sm_cb);

          if (new_height != best_height)
            {
              fprintf (stderr,
                       "ERROR: %s op %i before trans %.0f after redo %.0f!\n",
                       sm_cb->lcode_fn->name,
                       trans->op_id, best_height, new_height);
            }
          SM_unschedule_cb (sm_cb);
#endif
        }
    }

  /* If undid transformations, print out trans list */
  if ((undone_queue->first_qentry != NULL) && verbose_optimization)
    {
      printf ("%s cb %i, linear search undid %i of %i transformations:\n",
              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
              undone_queue->num_qentries, performed_queue->num_qentries);
      SM_summarize_trans_queue (stdout, undone_queue);
      printf ("\n");
    }
  if ((trans_height != best_height) && verbose_optimization)
    {
      printf ("%s cb %i: Undoing trans reduced cycles from %.0f to %.0f!\n",
              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
              trans_height, best_height);
    }

  SM_delete_trans_and_queue (untested_queue);
  SM_delete_trans_and_queue (undone_queue);
  SM_delete_trans_and_queue (performed_queue);

  /* Return the new cb height */
  return (best_height);
}


/* This version creates a new potential trans queue after each
 * transformation is performed.  This allows new transformations
 * to be spotted, and old transformations to be invalidated.
 *
 * Returns the cb's height after the transformations (if any).
 */
double
SM_do_linear_search2 (SM_Cb * sm_cb, double initial_height,
                      unsigned int allowed_trans)
{
  SM_Trans_Queue *untested_queue, *performed_queue;
  SM_Trans *trans;
  SM_Oper *sm_op;
  double best_height, new_height;
  int num_trans;


  /* Scan sm_cb, clearing away all old "unbeneficial" flags */
  for (sm_op = sm_cb->first_serial_op; sm_op != NULL;
       sm_op = sm_op->next_serial_op)
    {
      sm_op->flags &= ~(SM_RWC_UNBENEFICIAL | SM_EWC_UNBENEFICIAL);
    }

  /* Build transformation queue from the sm_cb */
  untested_queue = SM_find_potential_trans (sm_cb, allowed_trans);

  /* If queue empty, delete and return now */
  if (untested_queue->first_qentry == NULL)
    {
      SM_delete_trans_and_queue (untested_queue);
      return (initial_height);
    }

  /* For phd stats, flag that transformations wer found for
   * these allowed trans.
   */
  phd_flags |= allowed_trans;

  /* Get the number of transformations in queue */
  num_trans = untested_queue->num_qentries;

  /* Create queue for those transformations we decide to do */
  performed_queue = SM_new_trans_queue ("Performed");

  /* Initially, the best height is the initial height */
  best_height = initial_height;

  /* Before unscheduling, update schedule-based trans-info */
  if (sm_cb->num_unsched > 0)
    SM_schedule_cb (sm_cb);
  SM_update_sched_based_trans_info (untested_queue);

  /* Unschedule the cb before calculing priorities */
  SM_unschedule_cb (sm_cb);

  /* Test transformations in priority order */
  while ((trans = SM_dequeue_best_do_trans (untested_queue)) != NULL)
    {
      /* Do the transformation */
      SM_do_trans (trans);

      /* Reschedule cb and see if get better height */
      SM_schedule_cb (sm_cb);
      new_height = SM_calc_best_case_cycles (sm_cb);

      /* Keep trans if it helps performance, or, if doesn't hurt 
       * performance and it get scheduled in the same cycle or earlier
       */
      if ((!always_undo_opti) &&
          ((new_height < best_height) ||
           ((new_height == best_height) &&
            (trans->target_sm_op->sched_cycle <= trans->target_sched_cycle))))
        {
          if (verbose_optimization)
            {
              printf ("Keeping: ");
              SM_print_trans (stdout, trans);
              printf ("\n");
            }

          /* Update height */
          best_height = new_height;

          /* Add to performed queue */
          SM_enqueue_trans_before (performed_queue, trans, NULL);

          /* Delete old trans queue */
          SM_delete_trans_and_queue (untested_queue);

          /* Build a new transformation queue from the sm_cb.
           * This will take into account the transformation just done. 
           */
          untested_queue = SM_find_potential_trans (sm_cb, allowed_trans);

          /* Update trans info with the new schedule */
          SM_update_sched_based_trans_info (untested_queue);

          /* Unschedule the cb now, so the next priority calculation
           * will not be messed up (but after updating trans info)
           */
          SM_unschedule_cb (sm_cb);
        }

      /* Otherwise, undo transformation and delete it */
      else
        {
          /* Don't update sched-based trans info since didn't to trans */

          /* Need to unschedule the cb before undoing the trans 
           * (since the schedule may be invalid withouth the transformation
           *  and the sm manager punts when this happends).
           */
          SM_unschedule_cb (sm_cb);

          SM_undo_trans (trans);

#if CHECK_TRANS
          /* Debug */
          /* Reschedule cb and see if get back best height */
          SM_schedule_cb (sm_cb);
          new_height = SM_calc_best_case_cycles (sm_cb);

          if (new_height != best_height)
            {
              fprintf (stderr,
                       "ERROR: %s op %i before trans %.0f after undo %.0f!\n",
                       sm_cb->lcode_fn->name,
                       trans->op_id, best_height, new_height);
            }
          SM_unschedule_cb (sm_cb);
#endif

          SM_delete_trans (trans);
        }
    }

  /* If performed transformations, print out trans list */
  if (performed_queue->first_qentry != NULL)
    {
      phd_performed_flags |= allowed_trans;
      if (verbose_optimization)
        {
          printf ("%s cb %i, linear search performed %i of %i "
                  "transformations:\n",
                  sm_cb->lcode_fn->name, sm_cb->lcode_cb->id,
                  performed_queue->num_qentries, num_trans);
          SM_summarize_trans_queue (stdout, performed_queue);
          printf ("\n");
        }
    }

  SM_delete_trans_and_queue (untested_queue);
  SM_delete_trans_and_queue (performed_queue);

  /* Return the new height of the cb */
  return (best_height);
}


static void
print_effect (char *header, int pass, SM_Cb * sm_cb,
              double before, double after)
{
  double percent;

  /* Don't print anything unless verbose_optimizations is set and
   * there was a change.
   */
  if ((verbose_optimization == 0) || (before == after))
    return;

  percent = (double) (100.0 * ((double) (before - after) / (double) before));
  printf ("%s pass %i: cb %i weight %f before %.0f after %.0f "
          "diff %.0f (%5.2f%%)\n",
          header, pass, sm_cb->lcode_cb->id, sm_cb->cb_weight,
          before, after, before - after, percent);
}

/* This routine calculates the schedule height of the lcode cb 
 * using eval_lmdes.  This routine is solely for some of the
 * phd numbers where I am optimizing with a different mdes than
 * what is being used to evaluate it.  This routine is very slow
 * and should NOT BE USED during normal compilation.
 */
static double
calc_eval_height (L_Cb * cb)
{
  SM_Cb *sm_cb;
  double height;

  sm_cb = SM_new_cb (eval_lmdes, cb, SM_DHASY | SM_PREPASS);

  /* Schedule it and measure it's height */
  SM_schedule_cb (sm_cb);
  height = SM_calc_best_case_cycles (sm_cb);

  /* Delete the cb without committing */
  SM_delete_cb (sm_cb);

  /* Return the height */
  return (height);
}

void
update_phd_stats (MD * phd_stats, SM_Cb * sm_cb, double cycles,
                  int flags, int performed_flags)
{
  char *fn_name, entry_name[50];
  MD_Section *fn_section;
  MD_Field_Decl *cycles_decl, *weight_decl, *flags_decl;
  MD_Field_Decl *performed_flags_decl;
  MD_Entry *cb_entry;
  MD_Field *cycles_field, *weight_field, *flags_field;
  MD_Field *performed_flags_field;

  /* For efficiency, do not keep stats for zero weight cbs.
   * ( I was running out of disk space :) ) 
   */
  if (cycles < 0.001)
    return;

  /* Get the function name for ease of use */
  fn_name = sm_cb->lcode_fn->name;

  /* Create a section for this function if it doesn't already exist */
  if ((fn_section = MD_find_section (phd_stats, fn_name)) == NULL)
    {
      fn_section = MD_new_section (phd_stats, fn_name, 0, 0);
    }


  /* Print out cb id for entry name */
  sprintf (entry_name, "cb %i", sm_cb->lcode_cb->id);

  /* Better not be an entry for this cb already! */
  if ((cb_entry = MD_find_entry (fn_section, entry_name)) != NULL)
    {
      L_punt ("update_phd_stats, %s cb %i: cb already in stats table!",
              sm_cb->lcode_fn->name, sm_cb->lcode_cb->id);
    }

  /* Create the entry for this cb */
  cb_entry = MD_new_entry (fn_section, entry_name);

  /* 
   * Create and set fields for this entry 
   */

  /* Create the field declarations if they don't exists */
  cycles_decl = MD_find_field_decl (fn_section, "cycles");
  if (cycles_decl == NULL)
    {
      cycles_decl = MD_new_field_decl (fn_section, "cycles",
                                       MD_REQUIRED_FIELD);
      MD_require_double (cycles_decl, 0);
    }

  weight_decl = MD_find_field_decl (fn_section, "weight");
  if (weight_decl == NULL)
    {
      weight_decl = MD_new_field_decl (fn_section, "weight",
                                       MD_REQUIRED_FIELD);
      MD_require_double (weight_decl, 0);
    }

  flags_decl = MD_find_field_decl (fn_section, "flags");
  if (flags_decl == NULL)
    {
      flags_decl = MD_new_field_decl (fn_section, "flags", MD_REQUIRED_FIELD);
      MD_require_int (flags_decl, 0);
    }

  performed_flags_decl = MD_find_field_decl (fn_section, "performed_flags");
  if (performed_flags_decl == NULL)
    {
      performed_flags_decl = MD_new_field_decl (fn_section,
                                                "performed_flags",
                                                MD_REQUIRED_FIELD);
      MD_require_int (performed_flags_decl, 0);
    }

  /* Create and set the fields */
  cycles_field = MD_new_field (cb_entry, cycles_decl, 1);
  MD_set_double (cycles_field, 0, cycles);

  weight_field = MD_new_field (cb_entry, weight_decl, 1);
  MD_set_double (weight_field, 0, sm_cb->lcode_cb->weight);

  flags_field = MD_new_field (cb_entry, flags_decl, 1);
  MD_set_int (flags_field, 0, flags);

  performed_flags_field = MD_new_field (cb_entry, performed_flags_decl, 1);
  MD_set_int (performed_flags_field, 0, performed_flags);

}

int
test_opti (L_Func * fn, MD * phd_stats)
{
  L_Cb *cb;
  SM_Cb *sm_cb;
  double eval_before_height, eval_after_height, eval_rename_height;
  double opti_before_height, /*opti_after_height, */ opti_rename_height = 0.0;
  double opti_expr_height = 0.0, opti_init_height;
  double eval_expr_height, eval_init_height;
  double opti_trival_expr_height, eval_trival_expr_height;
  int change, pass;
  int fn_cycles;
  int optimize_code;

  /* Initialize to num cycles spent in function */
  fn_cycles = 0;

  /* We need to perform dataflow analysis in order to determine
   * what operations can be speculated.
   * Will try to just do LIVE_VARIABLE analysis
   */
  L_do_flow_analysis (fn, LIVE_VARIABLE);

  /* Debug, calculate initial operation weight for debug messages */
  L_compute_oper_weight (fn, 1, 0);

#if 1                           /* dynamic profile info, not possible with TI */
  L_compute_oper_weight (fn, 0, 1);
#endif

  /* Set 'optimize_code' to 1 if we are going to perform optimizations */
  if ((do_linear_search || do_classic_application) &&
      (do_renaming_with_copy || do_expression_reformulation))
    {
      optimize_code = 1;
    }

  /* Otherwise, set to 0 to indicate we are scheduling only */
  else
    {
      optimize_code = 0;
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* To facilitate debugging, skip cb if doesn't fall within bounds. */
      if ((cb->id < opti_cb_lower_bound) || (cb->id > opti_cb_upper_bound))
        continue;

      /* Create a sm_cb for this cb.  This creates the rinfo table
       * and builds all the dependences for this cb.
       */
      sm_cb = SM_new_cb (eval_lmdes, cb, SM_DHASY | SM_PREPASS);

      /* Do some trivial renaming that gets the cases introduced
       * by phase1 of the codegenerator and does not require renaming
       * with copy.
       */
      SM_do_trivial_renaming (sm_cb);

      /* Schedule the cb */
      SM_schedule_cb (sm_cb);

      /* Get its schedule height */
      eval_before_height = SM_calc_best_case_cycles (sm_cb);

      /* If we are not optimizing the code or
       * this cb does not meet the minimum opti cb weight,
       * just commit the schedule and go to the next cb.
       */
      if ((optimize_code == 0) || (cb->weight < min_cb_opti_weight))
        {
          /* Update the cycle counts for this cb before going to next cb */
          orig_cycles += eval_before_height;
          trival_expr_cycles += eval_before_height;
          rename_cycles += eval_before_height;
          picked_cycles += eval_before_height;
          fn_cycles += eval_before_height;

          if (write_phd_stats)
            {
              update_phd_stats (phd_stats, sm_cb, eval_before_height, 0, 0);
            }

          /* Commit the first cb schedule to the lcode cb */
          SM_commit_cb (sm_cb);

          /* Delete the sm_cb, done with it */
          SM_delete_cb (sm_cb);

          /* Goto next cb */
          continue;
        }

      /* Reset phd flags, so we can see what transformations
       * were possible for this cb (even if they were not performed).
       * Also keep track of which transformations were performed.
       */
      phd_flags = 0;
      phd_performed_flags = 0;

      /* If optimizing with a different lmdes, delete this sm_cb and
       * create a new one using the opti_lmdes.  This way all
       * the optimizations will be done with the proper mdes.
       * Since we are changing the cb, leaving the old sm_cb around
       * is not a good idea.
       */
      if (opti_lmdes != eval_lmdes)
        {
          SM_delete_cb (sm_cb);
          sm_cb = SM_new_cb (opti_lmdes, cb, SM_DHASY | SM_PREPASS);

          /* Determine the unoptimized height for this mdes */
          SM_schedule_cb (sm_cb);
          opti_before_height = SM_calc_best_case_cycles (sm_cb);
        }
      else
        {
          /* Set the unoptimized height for this cb */
          opti_before_height = eval_before_height;
        }

      opti_init_height = opti_before_height;
      eval_init_height = eval_before_height;
      change = 1;
      pass = 1;

      /* If performing expression reformulation and doing
       * a linear search (verses classic application), 
       * do a prepass of expression reformulation restricted
       * to transformations that do not add any operations.
       *
       * This should give renaming with copy the best picture
       * of the initial true dependences.
       */
      if (do_linear_search && do_expression_reformulation)
        {
          /* Do a prepass of expr without copy before renaming */
          do_expr_without_copy_only = 1;
          opti_trival_expr_height = SM_do_linear_search2 (sm_cb,
                                                          opti_init_height,
                                                          EXPR_WITH_COPY);

          print_effect ("TRIVAL EXPR opti", 0, sm_cb, opti_init_height,
                        opti_trival_expr_height);

          if (eval_lmdes != opti_lmdes)
            {
              eval_trival_expr_height = calc_eval_height (cb);
              print_effect ("TRIVAL EXPR eval", 0, sm_cb, eval_init_height,
                            eval_trival_expr_height);
            }
          else
            {
              eval_trival_expr_height = opti_trival_expr_height;
            }

          /* Update the initial initial height for the search */
          opti_init_height = opti_trival_expr_height;
          eval_init_height = eval_trival_expr_height;
        }

      /* Update cycles for after trival expression reformulation */
      trival_expr_cycles += eval_init_height;

      /* Do transformations until stabilize */
      while ((change) && (pass <= max_opti_passes))
        {
          if (do_renaming_with_copy)
            {
              if (do_linear_search)
                {
                  /* Do not restrict ourselves to classic heuristics */
                  use_classic_renaming_heuristics = 0;
                  opti_rename_height =
                    SM_do_linear_search (sm_cb, opti_init_height,
                                         RENAMING_WITH_COPY);
                }
              else if (do_classic_application)
                {
                  opti_rename_height =
                    SM_do_classic_renaming_with_copy (sm_cb);
                }
              else
                {
                  L_punt ("test_opti: Unknown opti option!");
                }
            }
          /* Otherwise, the height remains unchanged */
          else
            {
              opti_rename_height = opti_init_height;
            }

          print_effect ("RENAME opti", pass, sm_cb, opti_init_height,
                        opti_rename_height);

          /* If we are evaluating with a different mdes, 
           * reschedule with that mdes to determine the real effect.
           */
          if (eval_lmdes != opti_lmdes)
            {
              eval_rename_height = calc_eval_height (cb);

              print_effect ("RENAME eval", pass, sm_cb, eval_init_height,
                            eval_rename_height);
            }
          /* Otherwise, eval_rename_height is the same as opti's */
          else
            {
              eval_rename_height = opti_rename_height;
            }

          /* Update rename stats for first pass only */
          if (pass == 1)
            {
              rename_cycles += eval_rename_height;
            }

          /* Only do expression reformulation if parameters says to do so */
          if (do_expression_reformulation)
            {
              if (do_linear_search)
                {
                  /* Allow full-blown expr with copy to be done */
                  do_expr_without_copy_only = 0;
                  opti_expr_height =
                    SM_do_linear_search2 (sm_cb, opti_rename_height,
                                          EXPR_WITH_COPY);
                }
              else if (do_classic_application)
                {
                  /* I am assuming that a mdes that has no resource
                   * constraints is being used to emulate the
                   * dep-based algorithm previously used.
                   *
                   * The only other change necessary is to prevent
                   * any operations from being added.
                   */
                  do_expr_without_copy_only = 1;
                  opti_expr_height =
                    SM_do_linear_search2 (sm_cb, opti_rename_height,
                                          EXPR_WITH_COPY);
                }
              else
                {
                  L_punt ("test_opti: Unknown opti option!");
                }
            }
          else
            {
              /* Otherwise, there can be no height difference */
              opti_expr_height = opti_rename_height;
            }

          print_effect ("EXPR opti", pass, sm_cb, opti_rename_height,
                        opti_expr_height);

          /* If we are evaluating with a different mdes, 
           * reschedule with that mdes to determine the real effect.
           */
          if (eval_lmdes != opti_lmdes)
            {
              eval_expr_height = calc_eval_height (cb);
              print_effect ("EXPR eval", pass, sm_cb,
                            eval_rename_height, eval_expr_height);

              eval_init_height = eval_expr_height;
            }
          else
            {
              eval_init_height = opti_expr_height;
            }

          /* If doing a linear search and the 
           * the height has changed this pass, do another pass 
           */
          if (do_linear_search && (opti_init_height != opti_expr_height))
            {
              opti_init_height = opti_expr_height;
              change = 1;
              pass++;
            }
          else
            {
              change = 0;
            }
        }

      /* If optimizing with a different lmdes, delete this sm_cb and
       * create a new one using the eval_lmdes.
       */
      if (eval_lmdes != opti_lmdes)
        {
          SM_delete_cb (sm_cb);
          sm_cb = SM_new_cb (eval_lmdes, cb, SM_DHASY | SM_PREPASS);
        }

      /* Do a post schedule to see the effect */
      SM_schedule_cb (sm_cb);
      eval_after_height = SM_calc_best_case_cycles (sm_cb);

      if (eval_before_height != eval_after_height)
        {
          double percent;
          percent = (double) (100.0 *
                              ((double)
                               (eval_before_height -
                                eval_after_height) /
                               (double) eval_before_height));
          if (verbose_optimization)
            {
              printf ("TOTAL eval: cb %i weight %f  before %.0f after %.0f "
                      "diff %.0f (%5.2f%%)\n",
                      cb->id, sm_cb->cb_weight, eval_before_height,
                      eval_after_height,
                      eval_before_height - eval_after_height, percent);
            }
          if (print_cb_histogram)
            update_diff_stats (percent);
        }
      else if (print_cb_histogram)
        {
          update_diff_stats (0.0);
        }

      if (write_phd_stats)
        {
          update_phd_stats (phd_stats, sm_cb, eval_after_height, phd_flags,
                            phd_performed_flags);
        }
      orig_cycles += eval_before_height;
      fn_cycles += eval_after_height;

      /* Update cb stats */
      if (print_top_cb_stats)
        update_cb_stats (sm_cb, eval_before_height, eval_after_height);

      /* Pick either the original_cycles or the eval_after_height,
       * which ever is better (a la MCB approach)
       */
      if (eval_before_height < eval_after_height)
        picked_cycles += (double) eval_before_height;
      else
        picked_cycles += (double) eval_after_height;

      /* Commit the first cb schedule to the lcode cb */
      SM_commit_cb (sm_cb);

      /* Delete the sm_cb, done with it */
      SM_delete_cb (sm_cb);
    }

  return (fn_cycles);
}
void print_op(SM_Oper * op)
{
	if(op!=NULL)
		fprintf(stderr,"OP(%d) sched(%d) slot(%d) opcode(%s)\n",op->lcode_op->id,op->sched_cycle,op->sched_slot,op->lcode_op->opcode);
	else
		fprintf(stderr,"Fall thru\n");

}
void print_exits(SM_Cb * cb)
{
	      int i;
	      fprintf(stderr,"ID:%d, num_exits:%d\n",cb->lcode_cb->id,cb->num_exits);
	      for(i=0;i<cb->num_exits;i++)
	      {
	    	print_op(cb->exit_op[i]);

	      }

}
void print_all_op(SM_Cb * sm_cb)
{
	SM_Oper * sm_op;
	fprintf(stderr,"\nID:%d, num_exits:%d\n",sm_cb->lcode_cb->id,sm_cb->num_exits);
	for(sm_op = sm_cb->first_sched_op; sm_op != NULL;sm_op = sm_op->next_sched_op)
	{
		print_op(sm_op);

	}

}

int find_flow_wight_cb2(SM_Cb * sm_cb,L_Cb * cb){
  //L_Cb * cb=sm_cb->lcode_cb;
  int i;
  SM_Oper * op;
  //L_Flow * flow=cb->dest_flow;
  int MAX_sched=-1;

  for(op=sm_cb->first_sched_op;op!=NULL;op=op->next_sched_op)
  {
	  if(op->sched_cycle>MAX_sched)
		  MAX_sched=op->sched_cycle;
	  if (!L_is_control_oper (op->lcode_op))
	          continue;
	   //flow=L_find_flow_for_branch(cb,op->lcode_op);

	   //flow->wcet_weight=MAX_sched+1;
	   //fprintf(stderr,"flow(%d):(%d)->(%d)\n",flow->wcet_weight,flow->src_cb->id,flow->dst_cb->id);
	   //flow=flow->next_flow;
  }
  cb->wcet=MAX_sched+1;
 // fprintf(stderr,"((%d))\n",cb->wcet);
//  if(flow!=NULL){
//
// // flow->wcet_weight=MAX_sched+1;
//  //fprintf(stderr,"flow(%d):(%d)->(%d)\n",flow->wcet_weight,flow->src_cb->id,flow->dst_cb->id);
//  }
  return MAX_sched+1;
 }

int find_flow_wight_cb(SM_Cb * sm_cb,L_Cb * cb){
  //L_Cb * cb=sm_cb->lcode_cb;
  int i;
  SM_Oper * op;
  L_Flow * flow=cb->dest_flow;
  int MAX_sched=-1;

  for(op=sm_cb->first_sched_op;op!=NULL;op=op->next_sched_op)
  {
	  if(op->sched_cycle>MAX_sched)
		  MAX_sched=op->sched_cycle;
	  if (!L_is_control_oper (op->lcode_op))
	          continue;
	   //flow=L_find_flow_for_branch(cb,op->lcode_op);

	   flow->wcet_weight=MAX_sched+1;
	   //fprintf(stderr,"flow(%d):(%d)->(%d)\n",flow->wcet_weight,flow->src_cb->id,flow->dst_cb->id);
	   flow=flow->next_flow;
  }
  cb->wcet=MAX_sched+1;
  if(flow!=NULL){

  flow->wcet_weight=MAX_sched+1;
  //fprintf(stderr,"flow(%d):(%d)->(%d)\n",flow->wcet_weight,flow->src_cb->id,flow->dst_cb->id);
  }
  return MAX_sched+1;
 }

//added by morteza
int find_flow_wight(L_Func *fn)
{
   L_Cb *cb;
  SM_Cb *sm_cb;

  L_do_flow_analysis (fn, LIVE_VARIABLE);

  /* Debug, calculate initial operation weight for debug messages */
  L_compute_oper_weight (fn, 1, 0);

#if 1                           /* dynamic profile info, not possible with TI */
  L_compute_oper_weight (fn, 0, 1);
#endif

  /* Set 'optimize_code' to 1 if we are going to perform optimizations */

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* To facilitate debugging, skip cb if doesn't fall within bounds. */
      if ((cb->id < opti_cb_lower_bound) || (cb->id > opti_cb_upper_bound))
        continue;

      /* Create a sm_cb for this cb.  This creates the rinfo table
       * and builds all the dependences for this cb.
       */
      sm_cb = SM_new_cb (lmdes, cb, SM_DHASY | SM_PREPASS);

      /* Do some trivial renaming that gets the cases introduced
       * by phase1 of the codegenerator and does not require renaming
       * with copy.
       */
      SM_do_trivial_renaming (sm_cb);

      /* Schedule the cb */
      SM_schedule_cb (sm_cb);

      /* Get its schedule height */
      SM_calc_best_case_cycles (sm_cb);

      find_flow_wight_cb(sm_cb,cb);
//      fprintf(stderr,"CB(%d){\n",cb->id);
//      SM_print_cb_schedule(sm_cb);
//      fprintf(stderr,"}\n");

      /* If we are not optimizing the code or
       * this cb does not meet the minimum opti cb weight,
       * just commit the schedule and go to the next cb.
       */
       /* Update the cycle counts for this cb before going to next cb */


          /* Commit the first cb schedule to the lcode cb */
//          SM_commit_cb (sm_cb);

          /* Delete the sm_cb, done with it */
          SM_delete_cb (sm_cb);

          /* Goto next cb */

    }

  return 1;
}

int calc_wcet (L_Cb * cb)
{

  SM_Cb *sm_cb;

  //L_do_flow_analysis (fn, LIVE_VARIABLE);
  	  SM_use_fake_dataflow_info = 1;

  	  sm_cb = SM_new_cb (lmdes, cb, SM_DHASY | SM_PREPASS);

  	 // LB_remove_deps_assumed_optimized_away (sm_cb);

  	  SM_do_trivial_renaming (sm_cb);

      /* Schedule the cb */
      SM_schedule_cb (sm_cb);

      /* Get its schedule height */
     // SM_calc_best_case_cycles (sm_cb);

      find_flow_wight_cb2(sm_cb,cb);

      //SM_print_cb_schedule(sm_cb);
          /* Delete the sm_cb, done with it */
          SM_delete_cb (sm_cb);

          /* Goto next cb */

      SM_use_fake_dataflow_info = 0;
  return cb->wcet;
}


int calc_wcet_2 (L_Cb * cb)
{

  SM_Cb *sm_cb;

  //L_do_flow_analysis (fn, LIVE_VARIABLE);
  	  SM_use_fake_dataflow_info = 1;

  	  sm_cb = SM_new_cb (lmdes, cb, SM_DHASY | SM_PREPASS);

  	//  LB_remove_deps_assumed_optimized_away (sm_cb);

  	  SM_do_trivial_renaming (sm_cb);

      /* Schedule the cb */
      SM_schedule_cb (sm_cb);

      /* Get its schedule height */
     // SM_calc_best_case_cycles (sm_cb);

      find_flow_wight_cb(sm_cb,cb);

      //SM_print_cb_schedule(sm_cb);
          /* Delete the sm_cb, done with it */
          SM_delete_cb (sm_cb);

          /* Goto next cb */

      SM_use_fake_dataflow_info = 0;
  return cb->wcet;
}


void find_flow_wight_2(L_Func * fn)
{
	L_Cb * cb;
	L_do_flow_analysis (fn, LIVE_VARIABLE);
	for(cb=fn->first_cb;cb;cb=cb->next_cb)
		calc_wcet_2(cb);





}


void
L_read_parm_dhasy_opti (Parm_Parse_Info * ppi)
{
  L_read_parm_s (ppi, "opti_lmdes", &opti_lmdes_file_name);

  L_read_parm_b (ppi, "do_classic_application", &do_classic_application);
  L_read_parm_b (ppi, "do_linear_search", &do_linear_search);
  L_read_parm_b (ppi, "do_renaming_with_copy", &do_renaming_with_copy);
  L_read_parm_b (ppi, "do_expression_reformulation",
                 &do_expression_reformulation);
  L_read_parm_s (ppi, "prof_info", &prof_info);

  L_read_parm_b (ppi, "verbose_optimization", &verbose_optimization);
  L_read_parm_b (ppi, "print_cb_histogram", &print_cb_histogram);
  L_read_parm_b (ppi, "print_top_cb_stats", &print_top_cb_stats);
  L_read_parm_b (ppi, "print_total_stats", &print_total_stats);
  L_read_parm_b (ppi, "suppress_lcode_output", &suppress_lcode_output);

  L_read_parm_b (ppi, "write_phd_stats", &write_phd_stats);
  L_read_parm_s (ppi, "phd_stats_file", &phd_stats_file_name);

  L_read_parm_b (ppi, "always_undo_opti", &always_undo_opti);
  L_read_parm_i (ppi, "max_opti_passes", &max_opti_passes);
  L_read_parm_lf (ppi, "min_cb_opti_weight", &min_cb_opti_weight);
  L_read_parm_i (ppi, "cb_lower_bound", &opti_cb_lower_bound);
  L_read_parm_i (ppi, "cb_upper_bound", &opti_cb_upper_bound);
  L_read_parm_i (ppi, "oper_lower_bound", &opti_oper_lower_bound);
  L_read_parm_i (ppi, "oper_upper_bound", &opti_oper_upper_bound);
}

void
SM_gen_code (Parm_Macro_List * external_macro_list)
{
  FILE *phd_stats_file;
  char time_buf[100];
  double total_minutes;
  MD *phd_stats;

  /* Get the start time for the optimization */
  if ((start_time = time (NULL)) != -1)
    {
      strftime (time_buf, sizeof (time_buf),
                "%H:%M:%S %p, %A %B %d", localtime (&start_time));
      printf ("Started: %s.\n", time_buf);
    }

  /* Create diff table */
  diff_table = INT_new_symbol_table ("diff_table", 100);

  L_open_input_file (L_input_file);

  L_load_parameters (L_parm_file, external_macro_list,
                     "(dhasy_opti", L_read_parm_dhasy_opti);

  fprintf (stderr, "%s\n", L_lmdes_file_name);

  /* I want parm warnings right away! */
  L_warn_about_unused_macros (stderr, external_macro_list);

  /* Make sure the opti_lmdes_file_name file is a lmdes2 file */
  if ((opti_lmdes_file_name[0] == 0) ||
      (opti_lmdes_file_name[strlen (opti_lmdes_file_name) - 1] != '2'))
    {
      L_punt ("Parameter 'opti_lmdes' must point to a lmdes2 file,\n"
              "not '%s'", opti_lmdes_file_name);
    }

  /* Make sure the L_lmdes_file_name file is a lmdes2 file */
  if ((L_lmdes_file_name[0] == 0) ||
      (L_lmdes_file_name[strlen (L_lmdes_file_name) - 1] != '2'))
    {
      L_punt ("Parameter 'lmdes' must point to a lmdes2 file,\n"
              "not '%s'", L_lmdes_file_name);
    }

  /* Load mdes and move from lmdes to opti_lmdes */
  printf ("Optimizing using '%s'.\n", opti_lmdes_file_name);
  L_init_lmdes2 (opti_lmdes_file_name, L_max_pred_operand,
                 L_max_dest_operand, L_max_src_operand, 4);
  opti_lmdes = lmdes;
  lmdes = NULL;

  /* If using same mdes to evalulate the optimization, just point to the 
   * same mdes.  Otherwise, load another one.
   */
  if (strcmp (L_lmdes_file_name, opti_lmdes_file_name) == 0)
    {
      eval_lmdes = opti_lmdes;
      printf ("Evaluating using the same lmdes2 file.\n");
    }
  else
    {
      printf ("Evaluating using '%s'.\n", L_lmdes_file_name);
      L_init_lmdes2 (L_lmdes_file_name, L_max_pred_operand,
                     L_max_dest_operand, L_max_src_operand, 4);
      eval_lmdes = lmdes;
      lmdes = NULL;
    }

  /* If writing phd stats, create new md file with appropriate name */
  if (write_phd_stats)
    {
      phd_stats = MD_new_md (phd_stats_file_name, 0);
    }

  /* Otherwise, use NULL pointer to flag not writing stats */
  else
    {
      phd_stats = NULL;
    }
  /* Process all data and functions within a file */
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
        {
          L_define_fn_name (L_fn->name);

          total_cycles += test_opti (L_fn, phd_stats);

          /* Print out scheduled function (if not surpressed) */
          if (!suppress_lcode_output)
            {
              L_print_func (L_OUT, L_fn);
            }

          L_delete_func (L_fn);
        }
      else
        {
          /* Pass data through unchanged */
          if (!suppress_lcode_output)
            {
              L_print_data (L_OUT, L_data);
            }

          L_delete_data (L_data);
        }
    }

  L_close_input_file (L_input_file);

  /* If enabled, print histogram of optimization effects */
  if (print_cb_histogram)
    {
      print_diff_stats (stdout);
    }

  /* If enabled, print out the top weighted and most helped cbs */
  if (print_top_cb_stats)
    {
      print_top_weight_stats (stdout, 10);
      print_top_diff_stats (stdout, 10);
    }

  if (print_total_stats)
    {
      end_time = time (NULL);
      total_minutes = (double) (end_time - start_time) / 60.0;

      printf
        ("%4.2f total minutes spent reading lcode/scheduling/optimizing/etc.\n",
         total_minutes);
      printf ("%.0f original case cycles estimated.\n", orig_cycles);
      printf
        ("%.0f (%+.2f%%) picked case cycles estimated (all or nothing...)\n",
         picked_cycles, 100.0 * (orig_cycles - picked_cycles) / orig_cycles);
      printf ("%.0f (%+.2f%%) trival expr case cycles estimated.\n",
              trival_expr_cycles,
              100.0 * (orig_cycles - trival_expr_cycles) / orig_cycles);

      printf
        ("%.0f (%+.2f%%) (%+.2f%% %+.2f%%) rename and trival expr "
         "case cycles estimated.\n",
         rename_cycles, 100.0 * (orig_cycles - rename_cycles) / orig_cycles,
         100.0 * (orig_cycles - trival_expr_cycles) / orig_cycles,
         100.0 * (trival_expr_cycles - rename_cycles) / orig_cycles);
      /* I shorten this line to make it easier to read on my 80 char screen */
      printf ("%.0f (%+.2f%%) (%+.2f%% %+.2f%% %+.2f%%) best case est.\n",
              total_cycles,
              100.0 * (orig_cycles - total_cycles) / orig_cycles,
              100.0 * (orig_cycles - trival_expr_cycles) / orig_cycles,
              100.0 * (trival_expr_cycles - rename_cycles) / orig_cycles,
              100.0 * (rename_cycles - total_cycles) / orig_cycles);
    }

  /* If writing phd stats file, do it now */
  if (write_phd_stats)
    {
      if ((phd_stats_file = fopen (phd_stats_file_name, "w")) == NULL)
        {
          L_punt ("Unable to open phd_stats_file '%s' for writing!",
                  phd_stats_file_name);
        }

      /* Write out md file in low-level format */
      MD_write_md (phd_stats_file, phd_stats);
#if 0
      /* Debug, write out in high level format */
      MD_print_md (phd_stats_file, phd_stats, 80);
#endif

      /* Close the file */
      fclose (phd_stats_file);

      /* Delete the md file */
      MD_delete_md (phd_stats);
      phd_stats = NULL;
    }

  /* Test for memory leaks */
  SM_free_alloc_pools ();
}
