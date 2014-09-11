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
 *  File:  s_mqueue.c
 *
 *  Description:  Memory queue routines.
 *
 *  Creation Date :  June, 1994
 *
 *  Author:  John C. Gyllenhaal
 *
 *  Revisions:
 *
 *      (C) Copyright 1994, John Gyllenhaal and Wen-mei Hwu
 *      All rights granted to University of Illinois Board of Regents.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include "s_main.h"

L_Alloc_Pool	*Mqueue_pool = NULL;
L_Alloc_Pool	*Mentry_pool = NULL;

/* Create queue and initialize it */
Mqueue *M_create_queue (char *name, int id)
{
    Mqueue *queue;
    char buf[100];
    
    /* Alloc queue and initialize it */
    if (Mqueue_pool == NULL)
    {
	Mqueue_pool = L_create_alloc_pool ("Mqueue", sizeof (Mqueue), 1);
    }
    queue = (Mqueue *) L_alloc (Mqueue_pool);

    sprintf (buf, "%s %i", name, id);
    queue->name = strdup (buf);
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;

    return (queue);
}

/*
 * Adds a request to the end of the queue.
 */
Mentry *M_enqueue (Mqueue *queue, int type, int src, int dest, int addr,
		   int size, int serial_no, int cycle, Stats *stats)
{
    Mentry *entry;

    /* Alloc entry and initialize it */
    if (Mentry_pool == NULL)
    {
	Mentry_pool = L_create_alloc_pool ("Mentry", sizeof (Mentry), 16);
    }
    entry = L_alloc (Mentry_pool);

    entry->queue = queue;
    entry->type = type;
    entry->src = src;
    entry->dest = dest;
    entry->addr = addr;
    entry->size = size;
    entry->serial_no = serial_no;
    entry->cycle = cycle;
    entry->stats = stats;
    entry->playdoh_flags = 0;
    entry->request = 0;

    /* Put at the end of the queue */
    if (queue->tail == NULL)
	queue->head = entry;
    else
	queue->tail->next_entry = entry;

    entry->prev_entry = queue->tail;
    entry->next_entry = NULL;
    queue->tail = entry;

    /* Increment queue size */
    queue->size++;

    return (entry);
}

/*
 * Only use this routine when you are completely finished with the
 * queue entry!  The queue entry will be freed and some of its information
 * will be corrupted.
 */
void M_dequeue (Mentry *entry)
{
    Mqueue *queue;

    queue = entry->queue;

    /* Remove from queue */
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

    /* Free entry memory */
    L_free (Mentry_pool, entry);
}

/*
 * Moves the entry before the specified entry (before) in the new_queue.
 * This is more efficient than dequeueing and enqueueing the sint.
 *
 * A NULL pointer for 'before' causes the entry to be appended to the end
 * of the queue.
 */
void M_move_entry_before (new_queue, entry, before)
Mqueue *new_queue;
Mentry *entry;
Mentry *before;
{
    Mqueue *old_queue;

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
