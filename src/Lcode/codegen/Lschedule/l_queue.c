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
 *  File:  l_queue.c
 *
 *  Description:  Sched_Info queue routines
 *
 *  Creation Date :  September, 1993
 *
 *  Author:  John Gyllenhaal, Roger A. Bringmann
 *
 *  Revisions:
 *    Revised to support schedular requirements.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_schedule.h"

/* Create queue and initialize it */
Squeue *L_create_queue (char* name, int id)
{
    Squeue *queue;

    /* Alloc queue and initialize it */
    queue = (Squeue *) L_alloc (Squeue_pool);

    sprintf (queue->name, "%s %d", name, id);
    queue->size = 0;
    queue->current = NULL;
    queue->head = NULL;
    queue->tail = NULL;

    return (queue);
}

void L_delete_queue (Squeue *queue)
{
    if (queue->size != 0)
	L_punt ("L_delete_queue: queue %s is not empty!\n", queue->name);

    L_free(Squeue_pool, queue);
}

void L_reset_queue_current (Squeue *queue)
{
    queue->current = (Sq_entry *) -1;
}

Sched_Info* L_get_queue_next_entry(Squeue *queue)
{
    if (queue->current == (Sq_entry *) -1)
	queue->current = queue->head;
    else
	queue->current = queue->current->next_entry;

    if (queue->current != NULL)
        return queue->current->sinfo;
    else
    {
        queue->current = (Sq_entry *) -1;
	return NULL;
    }
}

Sched_Info* L_get_queue_head(Squeue *queue)
{
    if (queue->size==0) 
	return NULL;
    else
        return queue->head->sinfo;
}

int L_get_queue_size(Squeue *queue)
{
    return queue->size;
}

    
/*
 * Adds a sinfo to the end of the queue
 */
void L_enqueue (Squeue *queue, Sched_Info *sinfo)
{
    Sq_entry *entry;

    /* Alloc entry */
    entry = (Sq_entry *) L_alloc (Sq_entry_pool);

    /* Initialize fields */
    entry->queue = queue;
    entry->sinfo = sinfo;

    /* Add entry to sinfo's entry list */
    entry->prev_queue = NULL;
    entry->next_queue = sinfo->entry_list;
    if (sinfo->entry_list != NULL)
	sinfo->entry_list->prev_queue = entry;
    sinfo->entry_list = entry;

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
}

void L_dequeue (Squeue *queue, Sched_Info *sinfo)
{
    Sq_entry *entry;

    for (entry=queue->head; entry!=NULL; entry=entry->next_entry)
    {
	if (entry->sinfo == sinfo)
	{
	    if (queue->current == entry)
	    {
		queue->current = entry->prev_entry;
		if (queue->current == NULL)
		    L_reset_queue_current(queue);
	    }

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
        
            /* Remove entry from sinfo's entry list */
            if (entry->prev_queue == NULL)
	        entry->sinfo->entry_list = entry->next_queue;
            else
	        entry->prev_queue->next_queue = entry->next_queue;
        
            if (entry->next_queue != NULL)
	        entry->next_queue->prev_queue = entry->prev_queue;
        
            /* Free entry memory */
            L_free (Sq_entry_pool, entry);

	    break;
        }
    }
}
/*
 * Only use this routine when you are completely finished with the
 * queue entry!  The queue entry will be freed and some of its information
 * will be corrupted.
 */
void L_dequeue_entry (Sq_entry *entry)
{
    Squeue *queue;

    queue = entry->queue;

    if (queue->current == entry)
    {
	queue->current = entry->prev_entry;
	if (queue->current == NULL)
	    L_reset_queue_current(queue);
    }

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

    /* Remove entry from sinfo's entry list */
    if (entry->prev_queue == NULL)
	entry->sinfo->entry_list = entry->next_queue;
    else
	entry->prev_queue->next_queue = entry->next_queue;

    if (entry->next_queue != NULL)
	entry->next_queue->prev_queue = entry->prev_queue;

    /* Free entry memory */
    L_free (Sq_entry_pool, entry);
}

/*
 * Only use this routine when you are completely finished with all queue
 * entries associated with the sinfo!  The queue entries will be freed and
 * some of their information will be corrupted.
 */
void L_dequeue_from_all (Sched_Info *sinfo)
{
    Sq_entry *entry, *next_queue;

    for (entry = sinfo->entry_list; entry != NULL; entry = next_queue)
    {
	next_queue = entry->next_queue;
	L_dequeue_entry (entry);
    }
}

/*
 * Enqueue the sinfo before the specified entry in the queue(before).
 * 
 * A NULL pointer will cause the sinfo to be enqueued at the end of the queue.
 */
void L_enqueue_before (Squeue *queue, Sched_Info *sinfo, Sq_entry *before)
{
    Sq_entry *entry;

    /* Alloc entry */
    entry = (Sq_entry *) L_alloc (Sq_entry_pool);

    /* Initialize fields */
    entry->queue = queue;
    entry->sinfo = sinfo;

    /* Add entry to sinfo's entry list */
    entry->prev_queue = NULL;
    entry->next_queue = sinfo->entry_list;
    if (sinfo->entry_list != NULL)
	sinfo->entry_list->prev_queue = entry;
    sinfo->entry_list = entry;

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
}

/*
 * Moves the entry before the specified entry (before) in the new_queue.
 * This is more efficient than dequeueing and enqueueing the sinfo.
 *
 * A NULL pointer for 'before' causes the entry to be appended to the end
 * of the queue.
 */
void L_move_entry_before (Squeue *new_queue, Sq_entry *entry, Sq_entry *before)
{
    Squeue *old_queue;

    /* Remove entry from current queue */
    old_queue = entry->queue;
    if (entry->prev_entry == NULL)
	old_queue->head = entry->next_entry;
    else
	entry->prev_entry = entry->next_entry;

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


void L_enqueue_min_to_max_1 (Squeue *queue, Sched_Info *sinfo, float key)
{
    int		found, passed_current = 0;
    float	current_key;
    Sq_entry 	*current;

    sinfo->sort_key1 = key;

    if (queue->tail == NULL)
    {   /* empty queue */
	L_enqueue (queue, sinfo);
    }
    else
    {
	/* non-empty queue */
	current = queue->head;
	if (queue->current == (Sq_entry *) -1) passed_current = 1;

	/* Search for appropriate location in queue */
	found = 0;
	while ((current) && (!found))
	{
	    current_key = current->sinfo->sort_key1;

	    if (key < current_key)
	    {
		L_enqueue_before ( queue, sinfo, current);
		if (!passed_current) queue->current = current->prev_entry;
		found = 1;
	    }
	    else
	    {
		if (current == queue->current) passed_current = 1;

		current = current->next_entry;
	    }
	}

	if (!found)
	{ /* Append to end of list */
	    L_enqueue (queue, sinfo);
	}
    }
}

void L_enqueue_max_to_min_1 (Squeue *queue, Sched_Info *sinfo, float key)
{
    int		found, passed_current=0;
    float	current_key;
    Sq_entry 	*current;

    sinfo->sort_key1 = key;

    if (queue->tail == NULL)
    {   /* empty queue */
	L_enqueue (queue, sinfo);
    }
    else
    {
	/* non-empty queue */
	current = queue->head;
	if (queue->current == (Sq_entry *) -1) passed_current = 1;

	/* Search for appropriate location in queue */
	found = 0;
	while ((current) && (!found))
	{
	    current_key = current->sinfo->sort_key1;

	    if (key > current_key)
	    {
		L_enqueue_before ( queue, sinfo, current);
		if (!passed_current) queue->current = current->prev_entry;
		found = 1;
	    }
	    else
	    {
		if (current == queue->current) passed_current = 1;

		current = current->next_entry;
	    }
	}

	if (!found)
	{ /* Append to end of list */
	    L_enqueue (queue, sinfo);
	}
    }
}

void L_enqueue_min_to_max_2 (Squeue *queue, Sched_Info *sinfo, float key1, float key2)
{
    int		found, passed_current=0;
    float	current_key1, current_key2;
    Sq_entry 	*current;

    sinfo->sort_key1 = key1;
    sinfo->sort_key2 = key2;

    if (queue->tail == NULL)
    {   /* empty queue */
	L_enqueue (queue, sinfo);
    }
    else
    {
	/* non-empty queue */
	current = queue->head;
	if (queue->current == (Sq_entry *) -1) passed_current = 1;

	/* Search for appropriate location in queue */
	found = 0;
	while ((current) && (!found))
	{
	    current_key1 = current->sinfo->sort_key1;
	    current_key2 = current->sinfo->sort_key2;

	    if ((key1 < current_key1) ||
	        ((key1 == current_key1) && (key2 < current_key2)))
	    {
		L_enqueue_before ( queue, sinfo, current);
		if (!passed_current) queue->current = current->prev_entry;
		found = 1;
	    }
	    else
	    {
		if (current == queue->current) passed_current = 1;

		current = current->next_entry;
	    }
	}

	if (!found)
	{ /* Append to end of list */
	    L_enqueue (queue, sinfo);
	}
    }
}

void L_enqueue_max_to_min_2 (Squeue *queue, Sched_Info *sinfo, float key1, float key2)
{
    int		found, passed_current = 0;
    float	current_key1, current_key2;
    Sq_entry 	*current;

    sinfo->sort_key1 = key1;
    sinfo->sort_key2 = key2;

    if (queue->tail == NULL)
    {   /* empty queue */
	L_enqueue (queue, sinfo);
    }
    else
    {
	/* non-empty queue */
	current = queue->head;
	if (queue->current == (Sq_entry *) -1) passed_current = 1;

	/* Search for appropriate location in queue */
	found = 0;
	while ((current) && (!found))
	{
	    current_key1 = current->sinfo->sort_key1;
	    current_key2 = current->sinfo->sort_key2;

	    if ((key1 > current_key1) ||
	        ((key1 == current_key1) && (key2 > current_key2)))
	    {
		L_enqueue_before ( queue, sinfo, current);
		if (!passed_current) queue->current = current->prev_entry;
		found = 1;
	    }
	    else
	    {
		if (current == queue->current) passed_current = 1;

		current = current->next_entry;
	    }
	}

	if (!found)
	{ /* Append to end of list */
	    L_enqueue (queue, sinfo);
	}
    }
}

void L_enqueue_regpres (Squeue *queue, Sched_Info *sinfo, float key1, float key2)
{
    int		found, passed_current = 0;
    float	current_key1, current_key2;
    Sq_entry 	*current;

    sinfo->rsort_key1 = key1;
    sinfo->rsort_key2 = key2;

    if (queue->tail == NULL)
    {   /* empty queue */
	L_enqueue (queue, sinfo);
    }
    else
    {
	/* non-empty queue */
	current = queue->head;
	if (queue->current == (Sq_entry *) -1) passed_current = 1;

	/* Search for appropriate location in queue */
	found = 0;
	while ((current) && (!found))
	{
	    current_key1 = current->sinfo->rsort_key1;
	    current_key2 = current->sinfo->rsort_key2;

	    if ((key1 > current_key1) ||
	        ((key1 == current_key1) && (key2 > current_key2)))
	    {
		L_enqueue_before ( queue, sinfo, current);
		if (!passed_current) queue->current = current->prev_entry;
		found = 1;
	    }
	    else
	    {
		if (current == queue->current) passed_current = 1;

		current = current->next_entry;
	    }
	}

	if (!found)
	{ /* Append to end of list */
	    L_enqueue (queue, sinfo);
	}
    }
}

int L_in_queue (Squeue *queue, Sched_Info *sinfo)
{
    Sq_entry 	*current;

    for (current = sinfo->entry_list; current!= NULL; current = current->next_entry)
	if (current->queue == queue) return 1;
    
    return 0;
}
