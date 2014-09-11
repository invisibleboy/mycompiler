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
 *      File:   sm_queue.c
 *      Author: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  August 1996
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include "sm.h"
#include <library/l_alloc_new.h>

L_Alloc_Pool *SM_Oper_Queue_pool = NULL;
L_Alloc_Pool *SM_Oper_Qentry_pool = NULL;

L_Alloc_Pool *SM_Action_Queue_pool = NULL;
L_Alloc_Pool *SM_Action_Qentry_pool = NULL;

L_Alloc_Pool *SM_Trans_Queue_pool = NULL;
L_Alloc_Pool *SM_Trans_Qentry_pool = NULL;

/* 20021210 SZU
 * Priority Queue
 */
L_Alloc_Pool *SM_Priority_Queue_pool = NULL;
L_Alloc_Pool *SM_Priority_Qentry_pool = NULL;

/*
 * Create a new SM_Oper queue.  Returns the newly created sm_op queue.
 */
SM_Oper_Queue *
SM_new_oper_queue ()
{
  SM_Oper_Queue *queue;

  /* Initialize Oper queue pools if necessary */
  if (SM_Oper_Queue_pool == NULL)
    {
      SM_Oper_Queue_pool = L_create_alloc_pool ("SM_Oper_Queue",
                                                sizeof (SM_Oper_Queue), 8);
      SM_Oper_Qentry_pool = L_create_alloc_pool ("SM_Oper_Qentry",
                                                 sizeof (SM_Oper_Qentry), 64);
    }

  /* Alloc a new queue */
  queue = (SM_Oper_Queue *) L_alloc (SM_Oper_Queue_pool);

  /* Initialize empty queue's fields */
  queue->num_qentries = 0;
  queue->first_qentry = NULL;
  queue->last_qentry = NULL;

  /* Return the new queue */
  return (queue);
}

/*
 * Frees all the qentries in the queue and then frees the queue.
 */
void
SM_delete_oper_queue (SM_Oper_Queue * queue)
{
  SM_Oper_Qentry *qentry, *next_qentry;

  /* Delete all qentries in the queue. */
  for (qentry = queue->first_qentry; qentry != NULL; qentry = next_qentry)
    {
      /* Get the next qentry before deleting this one */
      next_qentry = qentry->next_qentry;

      SM_dequeue_oper (qentry);
    }

  /* Free queue memory */
  L_free (SM_Oper_Queue_pool, queue);
}

/* 
 * Enqueues the sm_op before the 'before_qentry'.  If 'before_qentry' is NULL,
 * place entry at the end of the queue.
 *
 * The new qentry is also added to the front of the sm_op's queue list.
 *
 */
SM_Oper_Qentry *
SM_enqueue_oper_before (SM_Oper_Queue * queue, SM_Oper * sm_op,
                        SM_Oper_Qentry * before_qentry)
{
  SM_Oper_Qentry *qentry;


#if 0
  /* Debug */
  if ((sm_op->lcode_op->id == 57) && (queue == sm_op->sm_cb->dep_in_resolved))
    {
      fprintf (stderr, "Enqueing op %i\n", sm_op->lcode_op->id);
    }
#endif

  /* Alloc the qentry */
  qentry = (SM_Oper_Qentry *) L_alloc (SM_Oper_Qentry_pool);

  /* Initialize the qentry's fields */
  qentry->queue = queue;
  qentry->sm_op = sm_op;

  /* Add entry to appropriate place in queue'e qentry list.
   * If before_qentry is NULL, add to end of qentry list.
   */
  if (before_qentry == NULL)
    {
      qentry->next_qentry = NULL;
      qentry->prev_qentry = queue->last_qentry;
      if (queue->last_qentry != NULL)
        queue->last_qentry->next_qentry = qentry;
      else
        queue->first_qentry = qentry;
      queue->last_qentry = qentry;
    }

  /* Otherwise, add qentry before 'before_qentry' */
  else
    {
      /* Sanity check, make sure before_qentry is for this queue! */
      if (before_qentry->queue != queue)
        {
          L_punt ("SM_enqueue_oper_before:"
                  " before_qentry not for this queue!");
        }

      qentry->next_qentry = before_qentry;
      qentry->prev_qentry = before_qentry->prev_qentry;
      if (before_qentry->prev_qentry != NULL)
        before_qentry->prev_qentry->next_qentry = qentry;
      else
        queue->first_qentry = qentry;
      before_qentry->prev_qentry = qentry;
    }

  /* Update number of qentries in queue */
  queue->num_qentries++;

  /* Add qentry to head of sm_op's queue list (list of qentries 
   * pointing to this sm_op).
   */
  qentry->prev_queue = NULL;
  qentry->next_queue = sm_op->first_queue;
  if (sm_op->first_queue != NULL)
    sm_op->first_queue->prev_queue = qentry;
  sm_op->first_queue = qentry;

  /* Return the just created qentry (in case it is needed by caller) */
  return (qentry);
}


SM_Oper_Qentry *
SM_enqueue_priority (SM_Oper_Queue *queue, SM_Oper *sm_op)
{
  SM_Oper_Qentry *entry;

  for (entry = queue->first_qentry; entry; entry = entry->next_qentry)
    {
      if (entry->sm_op->priority < sm_op->priority)
	break;
    }

  return SM_enqueue_oper_before (queue, sm_op, entry);
}


/* 
 * Enqueues the sm_op after the 'after_qentry'.  If 'after_qentry' is NULL,
 * place entry at beginning of queue.
 *
 * Just converts 'after_qentry' into a 'before_qentry' and calls
 * SM_enqueue_oper_before().
 */
SM_Oper_Qentry *
SM_enqueue_oper_after (SM_Oper_Queue * queue, SM_Oper * sm_op,
                       SM_Oper_Qentry * after_qentry)
{
  SM_Oper_Qentry *before_qentry;

  if (after_qentry != NULL)
    before_qentry = after_qentry->next_qentry;
  else
    before_qentry = queue->first_qentry;

  return (SM_enqueue_oper_before (queue, sm_op, before_qentry));
}

/*
 * Removes the qentry from the queue's entry list and the sm_op's 
 * queue list.  
 *
 * Only use this routine when you are completely finished with the
 * qentry!  It will be freed and some of its info will be corrupted.
 */
void
SM_dequeue_oper (SM_Oper_Qentry * qentry)
{
  SM_Oper_Queue *queue;

  queue = qentry->queue;

  /* Remove qentry from queue's list */
  if (qentry->prev_qentry != NULL)
    qentry->prev_qentry->next_qentry = qentry->next_qentry;
  else
    queue->first_qentry = qentry->next_qentry;

  if (qentry->next_qentry != NULL)
    qentry->next_qentry->prev_qentry = qentry->prev_qentry;
  else
    queue->last_qentry = qentry->prev_qentry;

  /* Update num_qentries */
  queue->num_qentries--;

  /* Remove qentry from oper's queue list */
  if (qentry->prev_queue != NULL)
    qentry->prev_queue->next_queue = qentry->next_queue;
  else
    qentry->sm_op->first_queue = qentry->next_queue;

  if (qentry->next_queue != NULL)
    qentry->next_queue->prev_queue = qentry->prev_queue;

  /* Free qentry memory */
  L_free (SM_Oper_Qentry_pool, qentry);
}

/*
 * Dequeues the sm_op from every queue it is in.
 *
 * Only use this routine when you are completely finished with all
 * the qentries associated with this oper!.  All of them will be
 * freed and some of their info will be corrupted when they are freed.
 */
void
SM_dequeue_oper_from_all (SM_Oper * sm_op)
{
  SM_Oper_Qentry *qentry, *next_qentry;

  /* Delete this sm_op from every queue it is in. */
  for (qentry = sm_op->first_queue; qentry != NULL; qentry = next_qentry)
    {
      /* Get the next qentry from the sm_op's queue list before deleting it */
      next_qentry = qentry->next_queue;

      SM_dequeue_oper (qentry);
    }
}

/* 
 * Prints oper queue to 'out'.
 */
void
SM_print_oper_queue (FILE * out, SM_Oper_Queue * queue)
{
  SM_Oper_Qentry *qentry;

  fprintf (out, "SM_Oper queue contents (%i entries):\n",
           queue->num_qentries);

  for (qentry = queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      SM_print_oper (out, qentry->sm_op);
    }

  fprintf (out, "\n");
}



/*
 * Create a new SM_Reg_Action queue.  Returns the newly created action queue.
 */
SM_Action_Queue *
SM_new_action_queue ()
{
  SM_Action_Queue *queue;

  /* Initialize Action queue pools if necessary */
  if (SM_Action_Queue_pool == NULL)
    {
      SM_Action_Queue_pool = L_create_alloc_pool ("SM_Action_Queue",
                                                  sizeof (SM_Action_Queue),
                                                  8);
      SM_Action_Qentry_pool = L_create_alloc_pool ("SM_Action_Qentry",
                                                   sizeof (SM_Action_Qentry),
                                                   64);
    }

  /* Alloc a new queue */
  queue = (SM_Action_Queue *) L_alloc (SM_Action_Queue_pool);

  /* Initialize empty queue's fields */
  queue->num_qentries = 0;
  queue->first_qentry = NULL;
  queue->last_qentry = NULL;

  /* Return the new queue */
  return (queue);
}

/*
 * Frees all the qentries in the queue and then frees the queue.
 */
void
SM_delete_action_queue (SM_Action_Queue * queue)
{
  SM_Action_Qentry *qentry, *next_qentry;

  /* Delete all qentries in the queue. */
  for (qentry = queue->first_qentry; qentry != NULL; qentry = next_qentry)
    {
      /* Get the next qentry before deleting this one */
      next_qentry = qentry->next_qentry;

      SM_dequeue_action (qentry);
    }

  /* Free queue memory */
  L_free (SM_Action_Queue_pool, queue);
}

/* 
 * Enqueues the action before the 'before_qentry'.  If 'before_qentry' is NULL,
 * place entry at the end of the queue.
 *
 * The new qentry is also added to the front of the action's queue list.
 *
 */
SM_Action_Qentry *
SM_enqueue_action_before (SM_Action_Queue * queue,
                          SM_Reg_Action * action,
                          SM_Action_Qentry * before_qentry)
{
  SM_Action_Qentry *qentry;

  /* Alloc the qentry */
  qentry = (SM_Action_Qentry *) L_alloc (SM_Action_Qentry_pool);

  /* Initialize the qentry's fields */
  qentry->queue = queue;
  qentry->action = action;

  /* Add entry to appropriate place in queue'e qentry list.
   * If before_qentry is NULL, add to end of qentry list.
   */
  if (before_qentry == NULL)
    {
      qentry->next_qentry = NULL;
      qentry->prev_qentry = queue->last_qentry;
      if (queue->last_qentry != NULL)
        queue->last_qentry->next_qentry = qentry;
      else
        queue->first_qentry = qentry;
      queue->last_qentry = qentry;
    }

  /* Otherwise, add qentry before 'before_qentry' */
  else
    {
      /* Sanity check, make sure before_qentry is for this queue! */
      if (before_qentry->queue != queue)
        {
          L_punt ("SM_enqueue_action_before:"
                  " before_qentry not for this queue!");
        }

      qentry->next_qentry = before_qentry;
      qentry->prev_qentry = before_qentry->prev_qentry;
      if (before_qentry->prev_qentry != NULL)
        before_qentry->prev_qentry->next_qentry = qentry;
      else
        queue->first_qentry = qentry;
      before_qentry->prev_qentry = qentry;
    }

  /* Update number of qentries in queue */
  queue->num_qentries++;

  /* Add qentry to head of action's queue list (list of qentries 
   * pointing to this action).
   */
  qentry->prev_queue = NULL;
  qentry->next_queue = action->first_queue;
  if (action->first_queue != NULL)
    action->first_queue->prev_queue = qentry;
  action->first_queue = qentry;

  /* Return the just created qentry (in case it is needed by caller) */
  return (qentry);
}

/* 
 * Enqueues the action after the 'after_qentry'.  If 'after_qentry' is NULL,
 * place entry at beginning of queue.
 *
 * Just converts 'after_qentry' into a 'before_qentry' and calls
 * SM_enqueue_action_before().
 */
SM_Action_Qentry *
SM_enqueue_action_after (SM_Action_Queue * queue,
                         SM_Reg_Action * action,
                         SM_Action_Qentry * after_qentry)
{
  SM_Action_Qentry *before_qentry;

  if (after_qentry != NULL)
    before_qentry = after_qentry->next_qentry;
  else
    before_qentry = queue->first_qentry;

  return (SM_enqueue_action_before (queue, action, before_qentry));
}

/*
 * Removes the qentry from the queue's entry list and the action's 
 * queue list.  
 *
 * Only use this routine when you are completely finished with the
 * qentry!  It will be freed and some of its info will be corrupted.
 */
void
SM_dequeue_action (SM_Action_Qentry * qentry)
{
  SM_Action_Queue *queue;

  queue = qentry->queue;

  /* Remove qentry from queue's list */
  if (qentry->prev_qentry != NULL)
    qentry->prev_qentry->next_qentry = qentry->next_qentry;
  else
    queue->first_qentry = qentry->next_qentry;

  if (qentry->next_qentry != NULL)
    qentry->next_qentry->prev_qentry = qentry->prev_qentry;
  else
    queue->last_qentry = qentry->prev_qentry;

  /* Update num_qentries */
  queue->num_qentries--;

  /* Remove qentry from action's queue list */
  if (qentry->prev_queue != NULL)
    qentry->prev_queue->next_queue = qentry->next_queue;
  else
    qentry->action->first_queue = qentry->next_queue;

  if (qentry->next_queue != NULL)
    qentry->next_queue->prev_queue = qentry->prev_queue;

  /* Free qentry memory */
  L_free (SM_Action_Qentry_pool, qentry);
}

/*
 * Dequeues the action from every queue it is in.
 *
 * Only use this routine when you are completely finished with all
 * the qentries associated with this action!.  All of them will be
 * freed and some of their info will be corrupted when they are freed.
 */
void
SM_dequeue_action_from_all (SM_Reg_Action * action)
{
  SM_Action_Qentry *qentry, *next_qentry;

  /* Delete this action from every queue it is in. */
  for (qentry = action->first_queue; qentry != NULL; qentry = next_qentry)
    {
      /*Get the next qentry from the action's queue list before deleting it */
      next_qentry = qentry->next_queue;

      SM_dequeue_action (qentry);
    }
}

/* 
 * Prints action queue to 'out'.
 */
void
SM_print_action_queue (FILE * out, SM_Action_Queue * queue)
{
  SM_Action_Qentry *qentry;

  fprintf (out, "SM_Reg_Action queue contents (%i entries):\n",
           queue->num_qentries);

  for (qentry = queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      SM_print_reg_action (out, qentry->action);
    }

  fprintf (out, "\n");
}



/*
 * Create a new SM_Trans queue.  Returns the newly created trans queue.
 */
SM_Trans_Queue *
SM_new_trans_queue (char *name)
{
  SM_Trans_Queue *queue;

  /* Initialize Trans queue pools if necessary */
  if (SM_Trans_Queue_pool == NULL)
    {
      SM_Trans_Queue_pool = L_create_alloc_pool ("SM_Trans_Queue",
                                                 sizeof (SM_Trans_Queue), 8);
      SM_Trans_Qentry_pool = L_create_alloc_pool ("SM_Trans_Qentry",
                                                  sizeof (SM_Trans_Qentry),
                                                  64);
    }

  /* Alloc a new queue */
  queue = (SM_Trans_Queue *) L_alloc (SM_Trans_Queue_pool);

  /* Initialize empty queue's fields */
  queue->name = strdup (name);
  queue->num_qentries = 0;
  queue->first_qentry = NULL;
  queue->last_qentry = NULL;

  /* Return the new queue */
  return (queue);
}

/*
 * Frees all the qentries in the queue and then frees the queue.
 */
void
SM_delete_trans_queue (SM_Trans_Queue * queue)
{
  SM_Trans_Qentry *qentry, *next_qentry;

  /* Delete all qentries in the queue. */
  for (qentry = queue->first_qentry; qentry != NULL; qentry = next_qentry)
    {
      /* Get the next qentry before deleting this one */
      next_qentry = qentry->next_qentry;

      SM_dequeue_trans (qentry);
    }

  /* Free the queue name */
  free (queue->name);

  /* Free queue memory */
  L_free (SM_Trans_Queue_pool, queue);
}

/* 
 * Enqueues the trans before the 'before_qentry'.  If 'before_qentry' is NULL,
 * place entry at the end of the queue.
 *
 * The new qentry is also added to the front of the trans's queue list.
 *
 */
SM_Trans_Qentry *
SM_enqueue_trans_before (SM_Trans_Queue * queue,
                         SM_Trans * trans, SM_Trans_Qentry * before_qentry)
{
  SM_Trans_Qentry *qentry;

  /* Alloc the qentry */
  qentry = (SM_Trans_Qentry *) L_alloc (SM_Trans_Qentry_pool);

  /* Initialize the qentry's fields */
  qentry->queue = queue;
  qentry->trans = trans;

  /* Add entry to appropriate place in queue'e qentry list.
   * If before_qentry is NULL, add to end of qentry list.
   */
  if (before_qentry == NULL)
    {
      qentry->next_qentry = NULL;
      qentry->prev_qentry = queue->last_qentry;
      if (queue->last_qentry != NULL)
        queue->last_qentry->next_qentry = qentry;
      else
        queue->first_qentry = qentry;
      queue->last_qentry = qentry;
    }

  /* Otherwise, add qentry before 'before_qentry' */
  else
    {
      /* Sanity check, make sure before_qentry is for this queue! */
      if (before_qentry->queue != queue)
        {
          L_punt ("SM_enqueue_trans_before:"
                  " before_qentry not for this queue!");
        }

      qentry->next_qentry = before_qentry;
      qentry->prev_qentry = before_qentry->prev_qentry;
      if (before_qentry->prev_qentry != NULL)
        before_qentry->prev_qentry->next_qentry = qentry;
      else
        queue->first_qentry = qentry;
      before_qentry->prev_qentry = qentry;
    }

  /* Update number of qentries in queue */
  queue->num_qentries++;

  /* Add qentry to head of trans's queue list (list of qentries 
   * pointing to this trans).
   */
  qentry->prev_queue = NULL;
  qentry->next_queue = trans->first_queue;
  if (trans->first_queue != NULL)
    trans->first_queue->prev_queue = qentry;
  trans->first_queue = qentry;

  /* Return the just created qentry (in case it is needed by caller) */
  return (qentry);
}

/* 
 * Enqueues the trans after the 'after_qentry'.  If 'after_qentry' is NULL,
 * place entry at beginning of queue.
 *
 * Just converts 'after_qentry' into a 'before_qentry' and calls
 * SM_enqueue_trans_before().
 */
SM_Trans_Qentry *
SM_enqueue_trans_after (SM_Trans_Queue * queue,
                        SM_Trans * trans, SM_Trans_Qentry * after_qentry)
{
  SM_Trans_Qentry *before_qentry;

  if (after_qentry != NULL)
    before_qentry = after_qentry->next_qentry;
  else
    before_qentry = queue->first_qentry;

  return (SM_enqueue_trans_before (queue, trans, before_qentry));
}

/*
 * Removes the qentry from the queue's entry list and the trans's 
 * queue list.  
 *
 * Only use this routine when you are completely finished with the
 * qentry!  It will be freed and some of its info will be corrupted.
 */
void
SM_dequeue_trans (SM_Trans_Qentry * qentry)
{
  SM_Trans_Queue *queue;

  queue = qentry->queue;

  /* Remove qentry from queue's list */
  if (qentry->prev_qentry != NULL)
    qentry->prev_qentry->next_qentry = qentry->next_qentry;
  else
    queue->first_qentry = qentry->next_qentry;

  if (qentry->next_qentry != NULL)
    qentry->next_qentry->prev_qentry = qentry->prev_qentry;
  else
    queue->last_qentry = qentry->prev_qentry;

  /* Update num_qentries */
  queue->num_qentries--;

  /* Remove qentry from trans's queue list */
  if (qentry->prev_queue != NULL)
    qentry->prev_queue->next_queue = qentry->next_queue;
  else
    qentry->trans->first_queue = qentry->next_queue;

  if (qentry->next_queue != NULL)
    qentry->next_queue->prev_queue = qentry->prev_queue;

  /* Free qentry memory */
  L_free (SM_Trans_Qentry_pool, qentry);
}

/*
 * Dequeues the trans from every queue it is in.
 *
 * Only use this routine when you are completely finished with all
 * the qentries associated with this trans!.  All of them will be
 * freed and some of their info will be corrupted when they are freed.
 */
void
SM_dequeue_trans_from_all (SM_Trans * trans)
{
  SM_Trans_Qentry *qentry, *next_qentry;

  /* Delete this trans from every queue it is in. */
  for (qentry = trans->first_queue; qentry != NULL; qentry = next_qentry)
    {
      /*Get the next qentry from the trans's queue list before deleting it */
      next_qentry = qentry->next_queue;

      SM_dequeue_trans (qentry);
    }
}

/* 
 * Prints trans queue to 'out'.
 */
void
SM_print_trans_queue (FILE * out, SM_Trans_Queue * queue)
{
  SM_Trans_Qentry *qentry;

  fprintf (out, "SM_Trans queue '%s' contents (%i entries):\n",
           queue->name, queue->num_qentries);

  for (qentry = queue->first_qentry; qentry != NULL;
       qentry = qentry->next_qentry)
    {
      SM_print_trans (out, qentry->trans);
    }

  fprintf (out, "\n");
}

/* 20021210 SZU
 * SM_Priority_Queue functions
 * Derived from Lsoftpipe Queue and Oper Queue
 */
/* Create and initialize new queue */
SM_Priority_Queue *
SM_new_priority_queue ()
{
  SM_Priority_Queue *queue;

  /* Initialize Priority queue pools if necessary */
  if (SM_Priority_Queue_pool == NULL)
    {
      SM_Priority_Queue_pool = L_create_alloc_pool
	                         ("SM_Priority_Queue",
				  sizeof (SM_Priority_Queue), 8);
      SM_Priority_Qentry_pool = L_create_alloc_pool
	                          ("SM_Priority_Qentry",
				   sizeof (SM_Priority_Qentry), 64);
    }

  /* Alloc a new queue */
  queue = (SM_Priority_Queue *) L_alloc (SM_Priority_Queue_pool);

  /* Initialize empty queue's fields */
  queue->num_qentries = 0;
  queue->num_not_sched = 0;
  queue->first_qentry = NULL;
  queue->last_qentry = NULL;

  /* Return the new queue */
  return (queue);
}

/* Free all queue nodes and reinitialize queue */
void
SM_reinit_priority_queue (SM_Priority_Queue *queue)
{
  SM_Priority_Qentry *qentry;
  SM_Priority_Qentry *next_qentry;

  for (qentry = queue->first_qentry; qentry != NULL; qentry = next_qentry)
    {
      next_qentry = qentry->next_qentry;
      L_free (SM_Priority_Qentry_pool, qentry);
    }

  queue->num_qentries = 0;
  queue->num_not_sched = 0;
  queue->first_qentry = NULL;
  queue->last_qentry = NULL;
}

/*
 * Frees all the qentries in the queue and then frees the queue.
 */
void
SM_delete_priority_queue (SM_Priority_Queue * queue)
{
  SM_Priority_Qentry *qentry, *next_qentry;

  /* Delete all qentries in the queue. */
  for (qentry = queue->first_qentry; qentry != NULL; qentry = next_qentry)
    {
      /* Get the next qentry before deleting this one */
      next_qentry = qentry->next_qentry;
      L_free (SM_Priority_Qentry_pool, qentry);
    }

  /* Free queue memory */
  L_free (SM_Priority_Queue_pool, queue);
}

/* Enqueue opers in order of increasing priority; i.e., oper with lowest
 * numerical priority value is at the head of the queue
 * Assumes oper not scheduled yet
 */
void
SM_enqueue_increasing_priority (SM_Priority_Queue *queue, SM_Oper *oper,
				int priority)
{
  SM_Priority_Qentry *qentry, *qtmp;

  /* create qentry for oper */
  qentry = (SM_Priority_Qentry *) L_alloc (SM_Priority_Qentry_pool);
  qentry->oper = oper;
  qentry->priority = priority;
  qentry->scheduled = 0;
  oper->qentry = qentry;

  /* empty queue */
  if (queue->first_qentry == NULL)
    {
      queue->first_qentry = queue->last_qentry = qentry;
      qentry->next_qentry = qentry->prev_qentry = 0;
      queue->num_qentries++;
      queue->num_not_sched++;
      return;
    }

  /* not empty queue */
  for (qtmp = queue->first_qentry; qtmp != NULL; qtmp = qtmp->next_qentry)
    {
      /* Find oper's place in queue.  If two opers have the same priority,
         they are dequeued in the same order enqueued. */
      if (qentry->priority < qtmp->priority)
	{
	  qentry->next_qentry = qtmp;
	  qentry->prev_qentry = qtmp->prev_qentry;
	  if (qtmp->prev_qentry != NULL)
	    {
	      qtmp->prev_qentry->next_qentry = qentry;
	    }
	  else
	    {
	      queue->first_qentry = qentry;
	    }
	  qtmp->prev_qentry = qentry;
	  break;
	}
      /* oper is last in queue */
      else if (qtmp == queue->last_qentry)
	{
	  qentry->prev_qentry = qtmp;
	  qentry->next_qentry = 0;
	  qtmp->next_qentry = qentry;
	  queue->last_qentry = qentry;
	  break;
	}
    }

  queue->num_qentries++;
  queue->num_not_sched++;
}

/* remove and return oper at the head of the queue */
SM_Oper *
SM_dequeue_priority (SM_Priority_Queue *queue)
{
  SM_Oper *oper;
  SM_Priority_Qentry *qentry;

  if (queue->first_qentry == NULL)
    return 0;

  qentry = queue->first_qentry;
  oper = qentry->oper;

  /* last oper in queue */
  if (qentry->next_qentry == NULL)
    {
      queue->first_qentry = queue->last_qentry = 0;
    }
  /* not last oper in queue */
  else
    {
      qentry->next_qentry->prev_qentry = 0;
      queue->first_qentry = qentry->next_qentry;
    }

  queue->num_qentries--;
  
  if (qentry->scheduled)
    queue->num_not_sched--;

  L_free (SM_Priority_Qentry_pool, qentry);

  return (oper);
}

/* return oper at end of queue without removing it */
SM_Oper *
SM_peek_end (SM_Priority_Queue *queue)
{
  if (queue->last_qentry != 0)
    return (queue->last_qentry->oper);
  else
    return 0;
}

/* return oper at head of queue without removing it */
SM_Oper *
SM_peek_head (SM_Priority_Queue * queue)
{
  if (queue->first_qentry != 0)
    return (queue->first_qentry->oper);
  else
    return 0;
}
