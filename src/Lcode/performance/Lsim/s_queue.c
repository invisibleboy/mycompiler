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
 *
 *  File:  s_queue.c
 *
 *  Description:  Sint queue routines
 *
 *  Creation Date :  September, 1993
 *
 *  Author:  John Gyllenhaal, Roger A. Bringmann
 *
 *  Revisions:
 *
 *      Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and The Board of
 *                         Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include "s_main.h"

/* Create queue and initialize it */
Squeue *S_create_queue (name, id)
char *name;
int id;
{
    Squeue *queue;
    char name_buf[100];

    /* Alloc queue and initialize it */
    queue = (Squeue *) L_alloc (Squeue_pool);

    sprintf (name_buf, "%s %i", name, id);
    queue->name = strdup (name_buf);
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return (queue);
}
    
/*
 * Adds a sint to the end of the queue
 */
Sq_entry *S_enqueue (queue, sint)
Squeue *queue;
Sint *sint;
{
    Sq_entry *entry;

    /* Alloc entry */
    entry = (Sq_entry *) L_alloc (Sq_entry_pool);

    /* Initialize fields */
    entry->queue = queue;
    entry->sint = sint;

    /* Add entry to sint's entry list */
    entry->prev_queue = NULL;
    entry->next_queue = sint->entry_list;
    if (sint->entry_list != NULL)
	sint->entry_list->prev_queue = entry;
    sint->entry_list = entry;

    /* Add entry to end of queue's entry list */
    if (queue->tail == NULL)
	queue->head = entry;
    else
	queue->tail->next_entry = entry;
    entry->prev_entry = queue->tail;
    entry->next_entry = NULL;
    queue->tail = entry;

    /* Increment queue size */
    queue->size++;

    /* Entry needed by reorder buf algorithm */
    return (entry);
}

/*
 * Only use this routine when you are completely finished with the
 * queue entry!  The queue entry will be freed and some of its information
 * will be corrupted.
 */
void S_dequeue (entry)
Sq_entry *entry;
{
    Squeue *queue;

    queue = entry->queue;

    /* Remove entry from current queue */
    if (entry->prev_entry == NULL)
	queue->head = entry->next_entry;
    else
	entry->prev_entry->next_entry = entry->next_entry;

    if (entry->next_entry == NULL)
	queue->tail = entry->prev_entry;
    else
	entry->next_entry->prev_entry = entry->prev_entry;

    /* Decrement queue size */
    queue->size--;

    /* Remove entry from sint's entry list */
    if (entry->prev_queue == NULL)
	entry->sint->entry_list = entry->next_queue;
    else
	entry->prev_queue->next_queue = entry->next_queue;

    if (entry->next_queue != NULL)
	entry->next_queue->prev_queue = entry->prev_queue;

    /* Free entry memory */
    L_free (Sq_entry_pool, entry);
}


/*
 * Only use this routine when you are completely finished with all queue
 * entries associated with the sint!  The queue entries will be freed and
 * some of their information will be corrupted.
 */
void S_dequeue_from_all (sint)
Sint *sint;
{
    Sq_entry *entry, *next_queue;

    for (entry = sint->entry_list; entry != NULL; entry = next_queue)
    {
	next_queue = entry->next_queue;
	S_dequeue (entry);
    }
}
/*
 * Enqueue the sint before the specified entry in the queue(before).
 * 
 * A NULL pointer will cause the sint to be enqueued at the end of the queue.
 */
Sq_entry *S_enqueue_before (queue, sint, before)
Squeue *queue;
Sint *sint;
Sq_entry *before;
{
    Sq_entry *entry;

    /* Alloc entry */
    entry = (Sq_entry *) L_alloc (Sq_entry_pool);

    /* Initialize fields */
    entry->queue = queue;
    entry->sint = sint;

    /* Add entry to sint's entry list */
    entry->prev_queue = NULL;
    entry->next_queue = sint->entry_list;
    if (sint->entry_list != NULL)  /* JCG 2/17/94 bug fix I think */
	sint->entry_list->prev_queue = entry;
    sint->entry_list = entry;

    /* if before NULL, add entry to end of queue's entry list */
    if (before == NULL)
    {
	if (queue->tail == NULL)
	    queue->head = entry;
	else
	    queue->tail->next_entry = entry;
	entry->prev_entry = queue->tail;
	entry->next_entry = NULL;
	queue->tail = entry;
    }

    /* Otherwise add before 'before' */
    else
    {
	if (before->prev_entry == NULL)
	    queue->head = entry;
	else
	    before->prev_entry->next_entry = entry;
	entry->prev_entry = before->prev_entry;
	entry->next_entry = before;
	before->prev_entry = entry;
    }

    /* Increment queue size */
    queue->size++;

    /* Entry needed by reorder buf algorithm */
    return (entry);
}

/*
 * Moves the entry before the specified entry (before) in the new_queue.
 * This is more efficient than dequeueing and enqueueing the sint.
 *
 * A NULL pointer for 'before' causes the entry to be appended to the end
 * of the queue.
 */
void S_move_entry_before (new_queue, entry, before)
Squeue *new_queue;
Sq_entry *entry;
Sq_entry *before;
{
    Squeue *old_queue;

    /* Remove entry from current queue */
    old_queue = entry->queue;
    if (entry->prev_entry == NULL)
	old_queue->head = entry->next_entry;
    else
	entry->prev_entry->next_entry = entry->next_entry;

    if (entry->next_entry == NULL)
	old_queue->tail = entry->prev_entry;
    else
	entry->next_entry->prev_entry = entry->prev_entry;

    /* Decrease old queue's size */
    old_queue->size--;

    /* Change entry to current queue */
    entry->queue = new_queue;

    /* if before NULL, add entry to end of queue's entry list */
    if (before == NULL)
    {
	if (new_queue->tail == NULL)
	    new_queue->head = entry;
	else
	    new_queue->tail->next_entry = entry;
	entry->prev_entry = new_queue->tail;
	entry->next_entry = NULL;
	new_queue->tail = entry;
    }

    /* Otherwise add before 'before' */
    else
    {
	if (before->prev_entry == NULL)
	    new_queue->head = entry;
	else
	    before->prev_entry->next_entry = entry;
	entry->prev_entry = before->prev_entry;
	entry->next_entry = before;
	before->prev_entry = entry;
    }

    /* Increment new queues size */
    new_queue->size++;
}


/*
 * Print the queues the sint is in.
 */
void S_print_queues_in (FILE *out, Sint *sint)
{
    Sq_entry *entry, *next_queue;

    fprintf (out, "The queues %s op %i is in are:\n", sint->oper->cb->fn->name,
	     sint->oper->lcode_id);
    for (entry = sint->entry_list; entry != NULL; entry = next_queue)
    {
	fprintf (out, "%s\n", entry->queue->name);	
	next_queue = entry->next_queue;
    }
    if (sint->entry_list == NULL)
	fprintf (out, "(none)");
    fprintf (out, "\n");
}

void S_print_queue (FILE *out, Squeue *queue)
{
    Sq_entry *entry;

    fprintf (out, "%s:\n", queue->name);

    for (entry = queue->head; entry != NULL; entry = entry->next_entry)
    {
	S_print_sint (out, entry->sint);
    }
}
