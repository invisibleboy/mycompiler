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
 *  File:  RU_manager.c	(Resource Usage Manager)
 *
 *  Description:
 *    Interface to scheduler to handle resouce usage management
 *
 *  Creation Date : May 1993
 *
 *  Authors : Scott Mahlke, John Gyllenhaal
 *
 * Revisions:
 * Lavery -  Added functions RU_number_of_alts(), RU_update_usage_count(),
 * 6/94      RU_find_resource_index(), and RU_find_max().
 * 
 * JCG 4/98  Folded in .lmdes2 support from development version, using
 *           initial support developed for my MICRO29 paper (using a 
 *           very old and prelinary version of SM (OLD_SM) to provide the
 *           functionality).
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <stdarg.h>
#include "l_schedule.h"
#include "old_sm.h"

#undef DEBUG
#define ERR stdout


/*=========================================================================*/
/*
 *	RU pred allocation/deallocation routines
 */
/*=========================================================================*/

int RU_max_pred = 0;
int RU_pred_alloc_target_size = 128;
static RU_Alloc_Data RU_pred_alloc_data = {0, 0, 0};

void RU_set_max_pred (int max)
{
    RU_max_pred = max;
}

int *RU_pred_alloc ()
{
    int num, size, element_size, i;
    RU_Alloc_Data *data;
    RU_Alloc_Header *header;
    char *block;

    /* Get pointer to info structure (so only dereference by name once) */
    data = &RU_pred_alloc_data;

    /* If there are no more free elements, allocate a block of them */
    if (data->head == NULL) {
        /* Get the size of what we are allocating */
        element_size = (sizeof (int) * RU_max_pred);

        /* Force it to be big enough to hold Alloc_Header structure */
        if (element_size < sizeof (RU_Alloc_Header *))
            element_size = sizeof (RU_Alloc_Header *);

        /* See how many will fit in block (malloc has 4 byte overhead)*/
        num = (RU_pred_alloc_target_size - 4) / element_size;

        /* Force it to allocate at least one */
        if (num <= 0)
            num = 1;

        /* Get size of allocation */
        size = num * element_size;

        /* Allocate the block of memory */
        if ((block = (char *) malloc (size)) == NULL) {
            fprintf (stderr, "RU_pred_alloc: Out of memory (request size %i)\n",
                     size);
            exit(1);
        }

        /* Break into individual elements and put into free list */
        for (i=0; i < num; i++) {
            /* typecast current element so can put into free list */
            header = (RU_Alloc_Header *) block;
            header->next = data->head;
            data->head = header;

            /* Goto next element */
            block += element_size;
        }

        /* Update stats */
        data->allocated += num;
        data->free += num;
    }

    /* Get element from head of list */
    header = data->head;
    data->head = header->next;

    /* Update stats */
    data->free--;

    /* Return element */
    return ((int *) header);
}

void RU_pred_free (int *ptr)
{
    RU_Alloc_Header *header;
    RU_Alloc_Data *data;

    /* Get pointer to info structure (so only dereference by name once) */
    data = &RU_pred_alloc_data;

    /* Make sure don't try to free NULL pointer */
    if (ptr == NULL)
    {
        fprintf (stderr, "RU_pred_free: NULL pointer passed\n");
        exit(-1);
    }

    /* Typecast so can put info free list */
    header = (RU_Alloc_Header *)ptr;

    /* Put into free list */
    header->next = data->head;
    data->head = header;

    /* Update stats */
    data->free++;
}

void RU_pred_print_alloc_data (FILE *F, int verbose)
{
    if (  (verbose) ||
          (RU_pred_alloc_data.allocated!=RU_pred_alloc_data.free)) {
        fprintf(F, "\tRU_pred:\t\t\tallocated\t%d\tfree\t%d\n",
                    RU_pred_alloc_data.allocated,
                    RU_pred_alloc_data.free);
    }
}

/*=========================================================================*/
/*
 *	RU_Info allocation/deallocation routines
 */
/*=========================================================================*/

int RU_info_alloc_target_size = 128;
static RU_Alloc_Data RU_info_alloc_data = {0, 0, 0};

RU_Info *RU_info_alloc ()
{
    int num, size, element_size, i;
    RU_Alloc_Data *data;
    RU_Alloc_Header *header;
    char *block;

    /* Get pointer to info structure (so only dereference by name once) */
    data = &RU_info_alloc_data;

    /* If there are no more free elements, allocate a block of them */
    if (data->head == NULL) {
        /* Get the size of what we are allocating */
        element_size = sizeof (RU_Info);

        /* Force it to be big enough to hold Alloc_Header structure */
        if (element_size < sizeof (RU_Alloc_Header *))
            element_size = sizeof (RU_Alloc_Header *);

        /* See how many will fit in block (malloc has 4 byte overhead)*/
        num = (RU_info_alloc_target_size - 4) / element_size;

        /* Force it to allocate at least one */
        if (num <= 0)
            num = 1;

        /* Get size of allocation */
        size = num * element_size;

        /* Allocate the block of memory */
        if ((block = (char *) malloc (size)) == NULL) {
            fprintf (stderr, "RU_info_alloc: Out of memory (request size %i)\n",
                     size);
            exit(1);
        }

        /* Break into individual elements and put into free list */
        for (i=0; i < num; i++) {
            /* typecast current element so can put into free list */
            header = (RU_Alloc_Header *) block;
            header->next = data->head;
            data->head = header;

            /* Goto next element */
            block += element_size;
        }

        /* Update stats */
        data->allocated += num;
        data->free += num;
    }

    /* Get element from head of list */
    header = data->head;
    data->head = header->next;

    /* Update stats */
    data->free--;

    /* Return element */
    return ((RU_Info *) header);
}

void RU_info_free (RU_Info *ptr)
{
    RU_Alloc_Header *header;
    RU_Alloc_Data *data;

    /* Get pointer to info structure (so only dereference by name once) */
    data = &RU_info_alloc_data;

    /* Make sure don't try to free NULL pointer */
    if (ptr == NULL)
    {
        fprintf (stderr, "RU_info_free: NULL pointer passed\n");
        exit(-1);
    }

    /* Typecast so can put info free list */
    header = (RU_Alloc_Header *)ptr;

    /* Put into free list */
    header->next = data->head;
    data->head = header;

    /* Update stats */
    data->free++;
}

void RU_info_print_alloc_data (FILE *F, int verbose)
{
    if (  (verbose) ||
          (RU_info_alloc_data.allocated!=RU_info_alloc_data.free)) {
        fprintf(F, "\tRU_Info:\t\t\tallocated\t%d\tfree\t%d\n",
                    RU_info_alloc_data.allocated,
                    RU_info_alloc_data.free);
    }
}

RU_Info *RU_info_create (L_Oper *op, int *pred)
{
    RU_Info *info;

    info = RU_info_alloc();
    info->op = op;
    info->pred = pred;
    info->selected_alt = NULL;
    info->issue_time = -1;
    info->slot_used = -1;

    return (info);
}

void RU_info_delete (RU_Info *info)
{
    if (info==NULL)
	return;

    info->op = NULL;
    if (info->pred!=NULL)
        RU_pred_free(info->pred);
    info->pred = NULL;
    info->selected_alt = NULL;
    RU_info_free(info);
}

/*=========================================================================*/
/*
 *      RU_Node Allocation/Deallocation routines
 */
/*=========================================================================*/

int RU_node_alloc_target_size = 128;
static RU_Alloc_Data RU_node_alloc_data = {0, 0, 0};

RU_Node *RU_node_alloc ()
{
    int num, size, element_size, i;
    RU_Alloc_Data *data;
    RU_Alloc_Header *header;
    char *block;

    /* Get pointer to info structure (so only dereference by name once) */
    data = &RU_node_alloc_data;

    /* If there are no more free elements, allocate a block of them */
    if (data->head == NULL) {
        /* Get the size of what we are allocating */
        element_size = sizeof (RU_Node);

        /* Force it to be big enough to hold Alloc_Header structure */
        if (element_size < sizeof (RU_Alloc_Header *))
            element_size = sizeof (RU_Alloc_Header *);

        /* See how many will fit in block (malloc has 4 byte overhead)*/
        num = (RU_node_alloc_target_size - 4) / element_size;

        /* Force it to allocate at least one */
        if (num <= 0)
            num = 1;

        /* Get size of allocation */
        size = num * element_size;

        /* Allocate the block of memory */
        if ((block = (char *) malloc (size)) == NULL) {
            fprintf (stderr, "RU_node_alloc: Out of memory (request size %i)\n",
                     size);
            exit(1);
        }

        /* Break into individual elements and put into free list */
        for (i=0; i < num; i++) {
            /* typecast current element so can put into free list */
            header = (RU_Alloc_Header *) block;
            header->next = data->head;
            data->head = header;

            /* Goto next element */
            block += element_size;
        }

        /* Update stats */
        data->allocated += num;
        data->free += num;
    }

    /* Get element from head of list */
    header = data->head;
    data->head = header->next;

    /* Update stats */
    data->free--;

    /* Return element */
    return ((RU_Node *) header);
}

void RU_node_free (RU_Node *ptr)
{
    RU_Alloc_Header *header;
    RU_Alloc_Data *data;

    /* Get pointer to info structure (so only dereference by name once) */
    data = &RU_node_alloc_data;

    /* Make sure don't try to free NULL pointer */
    if (ptr == NULL)
    {
        fprintf (stderr, "RU_node_free: NULL pointer passed\n");
        exit(-1);
    }

    /* Typecast so can put info free list */
    header = (RU_Alloc_Header *)ptr;

    /* Put into free list */
    header->next = data->head;
    data->head = header;

    /* Update stats */
    data->free++;
}

void RU_node_print_alloc_data (FILE *F, int verbose)
{
    if (  (verbose) ||
          (RU_node_alloc_data.allocated!=RU_node_alloc_data.free)) {
        fprintf(F, "\tRU_Node:\t\t\tallocated\t%d\tfree\t%d\n",
                    RU_node_alloc_data.allocated,
                    RU_node_alloc_data.free);
    }
}

RU_Node *RU_node_create (RU_Info *info)
{
    RU_Node *node;

    node = RU_node_alloc();
    node->info = info;
    node->rused = NULL;
    node->option_num = -1;
    node->prev_node = NULL;
    node->next_node = NULL;

    return (node);
}

void RU_node_delete (RU_Map *map, RU_Node *node)
{
    RU_Node *prev, *next;

    if (node==NULL)
	return;

    /* disconnext node from doubly linked list */
    prev = node->prev_node;
    next = node->next_node;
    if (prev!=NULL)
	prev->next_node = next;
    if (next!=NULL)
	next->prev_node = prev;
    node->prev_node = NULL;
    node->next_node = NULL;
    /* fix up map first_node and last_node ptrs if necessary */
    if (map->first_node==node) {
	if (map->last_node==node) {
	    map->first_node = NULL;
	    map->last_node = NULL;
	}
	else {
	    map->first_node = next;
	}
    }
    else if (map->last_node==node) {
	map->last_node = prev;
    }

    /* free node */
    RU_node_free(node);
}

void RU_node_delete_all (RU_Node *list)
{
    RU_Node *ptr, *next;

    for (ptr=list; ptr!=NULL; ptr=next) {
	next = ptr->next_node;
	ptr->prev_node = NULL;
	ptr->next_node = NULL;
	RU_node_free(ptr);
    }
}

void RU_node_insert_before (RU_Map *map, RU_Node *before_node, RU_Node *node)
{
    node->next_node = before_node;
    node->prev_node = ((before_node!=NULL) ? before_node->prev_node : NULL);
    if (node->prev_node!=NULL) {
	node->prev_node->next_node = node;
    }
    else {
	map->first_node = node;
    }
    if (node->next_node!=NULL) {
	node->next_node->prev_node = node;
    }
    else {
	map->last_node = node;
    }
}

void RU_node_insert_after (RU_Map *map, RU_Node *after_node, RU_Node *node)
{
    node->prev_node = after_node;
    node->next_node = ((after_node!=NULL) ? after_node->next_node : NULL);
    if (node->prev_node!=NULL) {
	node->prev_node->next_node = node;
    }
    else {
	map->first_node = node;
    }
    if (node->next_node!=NULL) {
	node->next_node->prev_node = node;
    }
    else {
	map->last_node = node;
    }
}

RU_Node *RU_node_find(RU_Node *list, RU_Info *ru_info, Mdes_Rused *rused)
{
    RU_Node *ptr;

    for (ptr=list; ptr!=NULL; ptr=ptr->next_node) {
	if ((ptr->info==ru_info) && (ptr->rused==rused))
	    return ptr;
    }
    return NULL;
}

/*=========================================================================*/
/*
 *      RU_Map Allocation/Deallocation routines
 */
/*=========================================================================*/
RU_Map *RU_map = NULL;
int RU_map_length = 0;
int RU_map_mode = -1;
int RU_map_cycles = -1;
int *RU_mask = NULL;
int RU_mask_width = -1;

/* For version2 support */
OLD_SM_Cb *RU_sm_cb = NULL;

/*
 *	Allocate a resource usage map of specified size, if size<=0,
 *	use default size for allocation.
 */
void RU_map_create (int length)
{
    int i, *mask_ptr;

    if (length<=0)
        RU_map_length = RU_MAP_DEFAULT_SIZE;
    else
	RU_map_length = length;
    RU_mask_width = lmdes->Rmask_width;

#ifdef DEBUG
    fprintf(ERR, "Create map of size %d\n", RU_map_length);
    fprintf(ERR, "\t mask width : %d\n", RU_mask_width);
#endif
    /* allocate RU_map */
    RU_map = (RU_Map *) malloc(sizeof(RU_Map)*RU_map_length);
    if (RU_map==NULL)
	L_punt("RU_create_map: RU_map not created: malloc out of space");

    /* allocate 1 huge array of ints to hold all masks */
    RU_mask = (int *) malloc(sizeof(int)*RU_map_length*RU_mask_width*2);
    if (RU_mask==NULL)
	L_punt("RU_create_map: RU_mask not created: malloc out of space");

    /* initialize map entries */
    mask_ptr = RU_mask;
    for (i=0; i<RU_map_length; i++) {
	RU_map[i].mask.uncond = mask_ptr;
	mask_ptr += RU_mask_width;
	RU_map[i].mask.pred = mask_ptr;
	mask_ptr += RU_mask_width;
        RU_map[i].first_node = NULL;
        RU_map[i].last_node = NULL;
    }
}

/*
 *	Allocate a new map 2x as big as the current one and copy all
 *	data from old map into new map.
 */
void RU_map_realloc ()
{
    int i, j, new_length, *new_mask, *new_mask_ptr;
    RU_Map *new_map;

    new_length = RU_map_length * 2;
    new_map = (RU_Map *) malloc(sizeof(RU_Map)*new_length);
    if (new_map==NULL)
	L_punt("RU_map_realloc: malloc out of space");
    new_mask = (int *) malloc(sizeof(int)*new_length*RU_mask_width*2);

#ifdef DEBUG
    fprintf(ERR, "Realloc map : size %d\n", new_length);
#endif
    /* copy old map entries */
    new_mask_ptr = new_mask;
    for (i=0; i<RU_map_length; i++) {
	new_map[i].mask.uncond = new_mask_ptr;
	new_mask_ptr += RU_mask_width;
	new_map[i].mask.pred = new_mask_ptr;
	new_mask_ptr += RU_mask_width;
	for (j=0; j<RU_mask_width; j++) {
	    new_map[i].mask.uncond[j] = RU_map[i].mask.uncond[j];
	    new_map[i].mask.pred[j] = RU_map[i].mask.pred[j];
	}
	new_map[i].first_node = RU_map[i].first_node;
	new_map[i].last_node = RU_map[i].last_node;
    }

    /* initialize rest of entries */
    for (i=RU_map_length; i<new_length; i++) {
	new_map[i].mask.uncond = new_mask_ptr;
	new_mask_ptr += RU_mask_width;
	new_map[i].mask.pred = new_mask_ptr;
	new_mask_ptr += RU_mask_width;
	for (j=0; j<RU_mask_width; j++) {
            new_map[i].mask.uncond[j] = 0;
            new_map[i].mask.pred[j] = 0;
        }
        new_map[i].first_node = NULL;
        new_map[i].last_node = NULL;
    }

    /* free up old structures */
    free(RU_map);
    free(RU_mask);

    /* setup global vars for new map */
    RU_map = new_map;
    RU_map_length = new_length;
    RU_mask = new_mask;

    /* Create/delete the sm_cb only with RU_map_init */
    RU_sm_cb = NULL;
}

/*
 *	Initialize all the map elements when scheduling new region
 *	Zero out all masks, and delete all nodes
 */
void RU_map_init (int mode, int cycles)
{
    int i, j;
    Mdes_Rmask *mask;

    if (RU_map==NULL)
	L_punt("RU_map_init: RU_map_create must be called first");

    RU_map_mode = mode;
    RU_map_cycles = cycles;

    for (i=0; i<RU_map_length; i++) {
	/* zero out all mask elements */
	mask = &RU_map[i].mask;
	if (mask==NULL)
	    L_punt("RU_map_init: corrupted mask");
	for (j=0; j<RU_mask_width; j++) {
	    mask->uncond[j] = 0;
	    mask->pred[j] = 0;
	}

	/* delete all nodes */
	RU_node_delete_all(RU_map[i].first_node);
	RU_map[i].first_node = NULL;
	RU_map[i].last_node = NULL;
    }


    /* If using version2 mdes, create a dummy sm_cb to manage
     * the the resources
     */
    if (lmdes->mdes2 != NULL)
    {
	/* Only a subset of the old scheduler is supported with the
	 * .lmdes2 (for now).  Punt if try to do cyclic scheduling.
	 */
	if (mode == RU_MODE_CYCLIC)
	{
	    L_punt ("RU_map_init: Cyclic mode (modulo scheduling) not currently supported with .lmdes2\n"
		    "Please use .lmdes file for now.");

	}

	if (RU_sm_cb != NULL)
	{
#if 0
	    printf ("\nResource map:\n");
	    OLD_SM_print_map (stdout, lmdes->mdes2->sm_mdes, RU_sm_cb);
#endif

	    OLD_SM_delete_cb (RU_sm_cb);
	}
	
	RU_sm_cb = OLD_SM_new_cb (lmdes->mdes2, NULL, NULL);
    }
}

/*
 *	Delete the entire map structure
 */
void RU_map_delete ()
{
    int i;

    if (RU_map==NULL)
	return;

    for (i=0; i<RU_map_length; i++) {
	RU_node_delete_all(RU_map[i].first_node);
	RU_map[i].first_node = NULL;
	RU_map[i].last_node = NULL;
    }

    free(RU_map);
    free(RU_mask);
    RU_map = NULL;
    RU_map_length = 0;
    RU_mask = NULL;
}

/*=========================================================================*/
/*
 *      Resource MII functions
 */
/*=========================================================================*/


/* Return number of alternatives that are compatable with flag given oper's
   resource usage info and MDES info */

int RU_number_of_alts(RU_Info *ru_info, Mdes_Info *mdes_info, int flags)
{
    Mdes_Compatable_Alt *c_alt;
    Mdes_Alt *alt;
    int num_alts = 0;

    /* input checking */
    if (ru_info==NULL)
        L_punt("RU_number_of_alts: ru_info cannot be NULL");
    if (mdes_info==NULL)
        L_punt("RU_number_of_alts: mdes_info cannot be NULL");
    if (mdes_info->num_compatable_alts<=0)
        L_punt("RU_number_of_alts: no compatible alternatives exist");

    for (c_alt=mdes_info->compatable_alts; c_alt!=NULL; c_alt=c_alt->next) {
        alt = c_alt->alt;
        if (RU_alt_flags_compatible(ru_info, alt, flags)) {
            num_alts++;
        }
    }
    return (num_alts);
}

/* Find index of resource used in given resource mask. */

static int RU_find_resource_index(Mdes_Rmask *mask)
{
  int word_count = 0;
  int bit_count = 0;
  unsigned int word;  /* first word with bit set */
  int num_bits;  /* number of bits in an integer */
  int num_words; /* number of words in resource mask */
  int word_index; /* index of current word in mask */

  /* The resource mask is an array of integers with each bit representing a 
     resource.  The resource corresponding to the LSB of the last integer
     in the array has index 0 and corresponds to the first resource declared
     in the Hmdes file.  If lmdes->num_resources is not evenly divided by
     the number of bits in an integer, the most significant bits of the 
     first word of will not correspond to any resource and will thus be
     invalid.  Therefore, start from the end of the mask to search
     for the set bit. */

  num_bits = sizeof(unsigned int) * 8;

  num_words = lmdes->num_resources/num_bits;
  num_words = (lmdes->num_resources%num_bits == 0) ? num_words : num_words + 1;

  /* input checking */
  if (mask==NULL)
      L_punt("RU_find_resource_index: mask cannot be NULL");

  /* find first word with bit set, starting from the end */
  word_index = num_words - 1;
  while (mask->uncond[word_index] == 0) {
     word_count++;
     word_index--;
  }
  word = mask->uncond[word_index];

  /* Find bit set in word.  LSB has lowest index. */
  for (bit_count = 0; bit_count < num_bits; bit_count++) {
      if ((word >> bit_count) == 1) {
          return ( word_count * num_bits + bit_count);
      }
  }
  L_punt("RU_find_resource_index: no bit set in word");
  return 0;
}

/* find max resource usage in count array */

static int RU_find_max(int *ru_count) 
{
  int i;
  int max;

    /* input checking */
    if (ru_count==NULL)
        L_punt("RU_find_max: ru_count cannot be NULL");

    max = ru_count[0];
    for (i=1; i<lmdes->num_resources; i++ ) {
        if (ru_count[i] > max) {
            max = ru_count[i];
        }
    }
    return (max);
}

/* Choose the alternative whose resource usage pattern, when added to the
   total resource usage produces the smallest value of MAX(total usage
   count for each resource) (i.e. smallest MII).  Add the resource usage of 
   this alternative to the total resource usage count.  Modifies ru_count.  
   Given resource usage info and MDES info for oper, current total resource 
   usage count, and oper flags.  ru_count is an array of integers, one for
   every resource in the machine.  Each array location contains the
   count of the number of cycles the corresponding resource is used by
   the opers examined so far.  This routine will add to the count
   for one more oper.  This routine is called once for each oper in the
   loop.  Resource MII is determined from the final values in ru_count. */

void RU_update_usage_count(RU_Info *ru_info, Mdes_Info *mdes_info, 
                                            int *ru_count, int flags)
{
  int *current_ru_count;  /* ru_count updated with options selected so far
                             for current alternative */
  int *min_ru_count;  /* total resource usage count for the best alternative  
                         found so far. */
  int first_alt = 1;  /* flag to indicate if the current alternative is the
                         first one examined */
  Mdes_Compatable_Alt *c_alt;
  Mdes_Alt *alt;  /* current alternative */
  int i, j;
  Mdes_ResList *reslist;
  Mdes_Rused *rused;
  int sel_option;  /* index of resource used by best option found so far */ 
  int current_option; /* index of resource used by option currently being
                         examined */
  Mdes_Rmask *mask;
  int min_max = 0;  /* smallest MII found so far */
  int current_max;  /* MII if current alternative is used */

    /* input checking */
    if (ru_info==NULL)
        L_punt("RU_update_usage_count: ru_info cannot be NULL");
    if (mdes_info==NULL)
        L_punt("RU_update_usage_count: mdes_info cannot be NULL");
    if (mdes_info->num_compatable_alts<=0)
        L_punt("RU_update_usage_count: no compatible alternatives exist");
    if (ru_count == NULL)
        L_punt("RU_update_usage_count: ru_count cannot be NULL");

    current_ru_count = (int *) malloc(sizeof(int) * lmdes->num_resources);
    min_ru_count = (int *) malloc(sizeof(int) * lmdes->num_resources);

    /* examine each alternative */
    for (c_alt=mdes_info->compatable_alts; c_alt!=NULL; c_alt=c_alt->next) {
        alt = c_alt->alt;
        if (! RU_alt_flags_compatible(ru_info, alt, flags)) {
            continue;
        }
        /* need a new working copy of ru_count */
        for (i=0; i<lmdes->num_resources; i++ ) {
            current_ru_count[i] = ru_count[i];
        }
        reslist = alt->reslist;
        /* examine each resource type */
        for (i=0; i<reslist->num_used; i++) {
            rused = &reslist->used[i];
            mask = &rused->option[0];
            /* initially select the first option (refer to an option by the 
               index of the resource it uses) */
            sel_option = RU_find_resource_index(mask);
            /* examine the other options for that resource type */
            for (j=1; j<rused->num_options; j++) {
                mask = &rused->option[j];
                current_option = RU_find_resource_index(mask);
                /* select the option which uses the least used resource */
                if (ru_count[current_option] < ru_count[sel_option]) {
                    sel_option = current_option;
                }
            }
            /* add the resource usage of the selected option to the current
               count */
            current_ru_count[sel_option] = current_ru_count[sel_option] + 
                                   rused->end_usage - rused->start_usage + 1;
        }
        if (first_alt) {
            /* initialize min_ru_count and min_max */
            for (i=0; i<lmdes->num_resources; i++ ) {
                min_ru_count[i] = current_ru_count[i];
            }
            min_max = RU_find_max(min_ru_count);
            first_alt = 0;
        }
        else {
            /* compare current alternative against best so far */
            current_max = RU_find_max(current_ru_count);
            if (current_max < min_max) {
                for (i=0; i<lmdes->num_resources; i++ ) {
                    min_ru_count[i] = current_ru_count[i];
                }
                min_max = current_max;
            }
        }
    }
    /* copy the final count back to ru_count */
    for (i=0; i<lmdes->num_resources; i++ ) {
        ru_count[i] = min_ru_count[i];
    }
    free (min_ru_count);
    free (current_ru_count);
}


/*=========================================================================*/
/*
 *      RU_Map scheduling functions
 */
/*=========================================================================*/

/*
 *	Currently only flag passed is speculative flag:
 *		if (speculative_flag), alt is compatible if either
 *			1. it is a speculative version (alt flag)
 *			2. it is non-excepting (op flag)
 *		if (! speculative_flag), alt is compatible
 *			1. it is not a speculative version (alt flag)
 */
int RU_alt_flags_compatible(RU_Info *ru_info, Mdes_Alt *alt, int flags)
{
    int compatible;

    if (L_EXTRACT_BIT_VAL(flags, ALT_FLAG_SILENT)) {
	compatible = ((alt_flag_set(ru_info->proc_opc, alt->id, ALT_FLAG_SILENT)) ||
		      (! op_flag_set(ru_info->proc_opc, OP_FLAG_EXCEPT)));
    }
    else {
	compatible = (! alt_flag_set(ru_info->proc_opc, alt->id, ALT_FLAG_SILENT));
    }
    return compatible;
}

/*
 *	Check whether mask conflicts with anything in RU_map already.
 *	For now only uncond mask looked at, need to at predicate mask
 *	in future!!!!
 */
int RU_can_place (int time, Mdes_Rused *rused, int option_num)
{
    int i, conflict;
    Mdes_Rmask *mask;

    if (RU_map_mode == RU_MODE_CYCLIC) {
	time = time % RU_map_cycles;
    }

    /* check if current RU_map is large enough */
    if (time>=RU_map_length) {
#ifdef DEBUG
	fprintf(ERR, "RU_can_place: Reallocating map : old size = %d : time = %d\n",
			RU_map_length, time);
#endif
	RU_map_realloc();
    }

    conflict = 0;
    mask = &rused->option[option_num];
    for (i=0; i<RU_mask_width; i++) {
	conflict = mask->uncond[i] & RU_map[time].mask.uncond[i];
	if (conflict)
	    break;
    }
#ifdef DEBUG
    if (conflict==0)
        fprintf(ERR, "RU_can_place: NO conflict\n");
    else
        fprintf(ERR, "RU_can_place: YES conflict\n");
#endif

    return (conflict==0);
}

void RU_place (int time, Mdes_Rused *rused, int option_num, RU_Info *ru_info)
{
    int i;
    Mdes_Rmask *mask;
    RU_Node *node;

    if (RU_map_mode == RU_MODE_CYCLIC) {
	time = time % RU_map_cycles;
    }

    /* check if current RU_map is large enough */
    if (time>=RU_map_length) {
#ifdef DEBUG
        fprintf(ERR, "RU_place: Reallocating map : old size = %d : time = %d\n",
                        RU_map_length, time);
#endif
        RU_map_realloc();
    }   

    /* modify mask */
    mask = &rused->option[option_num];
    for (i=0; i<RU_mask_width; i++) {
        RU_map[time].mask.uncond[i] |= mask->uncond[i];
    }

    /* create RU_Node and add to RU_map */
    node = RU_node_create(ru_info);
    node->rused = rused;
    node->option_num = option_num;
    RU_node_insert_before(&RU_map[time], RU_map[time].first_node, node);
}

void RU_unplace (int time, Mdes_Rused *rused, RU_Info *ru_info)
{
    int i, option_num;
    Mdes_Rmask *mask;
    RU_Node *node;

    if (RU_map_mode == RU_MODE_CYCLIC) {
	time = time % RU_map_cycles;
    }

    /* get option number selected from corresponding RU_Node */
    node = RU_node_find(RU_map[time].first_node, ru_info, rused);
    option_num = node->option_num;

    /* modify mask */
    mask = &rused->option[option_num];
    for (i=0; i<RU_mask_width; i++) {
	RU_map[time].mask.uncond[i] &= ~(mask->uncond[i]);
    }

    /* delete node */
    RU_node_delete(&RU_map[time], node);
}

int RU_schedule_op (RU_Info *ru_info, Mdes_Info *mdes_info,
	int *operand_ready_times, int issue_time,
	int earliest_slot, int latest_slot, int flags)
{
    int i, j, k, *sel_option, sel_slot = 0, alt_flag, option_flag;
    int latency_count, operand_flag;
    Mdes_Compatable_Alt *c_alt;
    Mdes_Alt *alt, *sel_alt = NULL;
    Mdes_ResList *reslist;
    Mdes_Rused *rused, *check_rused;
    int p, c, overlaps, conflict;
    Mdes_Operation *op;
    Mdes_Stats *stats;
			  
    /* Take stats for .lmdes2 scheduling runs */
    if (lmdes->mdes2 != NULL)
    {
	op = lmdes->op_table[mdes_info->opcode];

	if(Lsched_prepass)
	{
	    stats = &op->sched_prepass;
	}
	else
	{
	    stats = &op->sched_postpass;
	}
	stats->num_oper_checks++;
    }
    else
	stats = NULL;

    /* input checking */
    if (ru_info==NULL)
	L_punt("RU_schedule_op: ru_info cannot be NULL");
    if (mdes_info==NULL)
	L_punt("RU_schedule_op: mdes_info cannot be NULL");
    if (mdes_info->num_compatable_alts<=0)
	L_punt("RU_schedule_op: no compatible alternatives exist");
    if (operand_ready_times==NULL)
	L_punt("RU_schedule_op: operand_ready_times is NULL");

#ifdef DEBUG
     fprintf(ERR, "RU_schedule_op: op %d : issue_time %d : earliest slot %d : latest slot %d\n",
			ru_info->op->id, issue_time, earliest_slot, latest_slot);
#endif
#ifdef DEBUG_READY_TIME
    fprintf(ERR, "RU_schedule_op: op %d : ready times\n", ru_info->op->id);
    fprintf(ERR, "\t[%d], lmdes->latency_count");
    for (i=0; i<lmdes->latency_count; i++) {
	fprintf(ERR, " %d", operand_ready_times[i]);
    }
    fprintf(ERR, "\n");
#endif

    /*
     *	First loop thru and see if any alternative can be scheduled, record
     *	option selected for each resource in sel_option array.  Then if
     *	successful, go thru and reserve all resources and create necessary
     *	structures to schedule op at issue_time.
     */

    sel_option = (int *)malloc(sizeof(int)*lmdes->num_resources);
    latency_count = lmdes->latency_count;

    /*
     *	Consider each compatible alternative
     */
    alt_flag = 0;
    for (c_alt=mdes_info->compatable_alts; c_alt!=NULL; c_alt=c_alt->next) {
	alt = c_alt->alt;
#ifdef DEBUG
	fprintf(ERR, "\tAttempt to schedule alt %d\n", alt->id);
#endif


	/*
	 *	Check that flags for alt compatable with passed flags
	 */
	if (! RU_alt_flags_compatible(ru_info, alt, flags)) {
#ifdef DEBUG
	    fprintf(ERR, "\tFlags imcompatible for alt %d\n", alt->id);
#endif
	    continue;
	}


	/*
	 *	Check if operands really ready for this alternative, scheduler
	 *	has best case numbers, so here we really enforce this restriction!
	 */
	operand_flag = 1;
	for (i=0; i<latency_count; i++) {
	    /* check for operand which isnt really ready */
	    if (issue_time < operand_ready_times[i] - alt->latency->operand_latency[i]) {
#ifdef DEBUG_READY_TIME
	        fprintf(ERR, "\tOperand %d not ready for alt %d\n", i, alt->id);
#endif
		operand_flag = 0;
		break;
	    }
	}

	if (! operand_flag)
	    continue;

	/*
	 *	Check resources required by the alternative are free
	 */
	alt_flag = 1;
	reslist = alt->reslist;
	/* Detect if we should be using version2 sm functions */
	if (reslist == NULL)
	{
	    /* Try to schedule, if can, commit resources */
	    sel_slot = OLD_SM_sched_table (RU_sm_cb, alt->table, issue_time, 
					   earliest_slot, latest_slot, 1, 
					   stats);

	    /* Can we schedule this? */
	    if (sel_slot < 0)
	    {
		/* No.  Mark at unable to schedule and continue */
		alt_flag = 0;
		continue;
	    }
	    else
	    {
		/* Yes.  Select this slot and stop the search */
		sel_alt = alt;
		break;
	    }
	}    
	for (i=0; i<reslist->num_used; i++) {
	    rused = &reslist->used[i];
	    /* loop thru each resource option */
	    option_flag = 0;
	    for (j=0; j<rused->num_options; j++) {

		/* resource0 is slot so need to check earliest slot for i=0 */
		if (i==0) {
		    if (reslist->slot_options[j] < earliest_slot) {
#ifdef DEBUG
			fprintf(ERR, "\tslot %d not compatible\n",
					reslist->slot_options[j]);
#endif
		        continue;
		    }
		    else if (reslist->slot_options[j] > latest_slot) {
#ifdef DEBUG
			fprintf(ERR, "\tslot %d not compatible\n",
					reslist->slot_options[j]);
#endif
		        continue;
		    }
		    else {
			sel_slot = reslist->slot_options[j];
#ifdef DEBUG
			fprintf(ERR, "\tslot %d selected\n", sel_slot);
#endif
		    }
		}

		/* If this Rused option could try to use the same resource
		 * that a previous Rused option in this reslist used
		 * (marked with flag), then do a check to prevent the
		 * improper reuse of this resource.  RU_can_place DOES NOT
		 * detect this situation.
		 * -JCG 1/12/95
		 */
		overlaps = 0;
		if (rused->flags & MDES_OVERLAPPING_REQUEST)
		{
		    for (p = 0; p < i; p++)
		    {
			check_rused = &reslist->used[p];
			if ((check_rused->end_usage < rused->start_usage)||
			    (check_rused->start_usage > rused->end_usage))
			    continue;
			
			for (c=0; c < RU_mask_width; c++)
			{
			    conflict = rused->option[j].uncond[c] &
				check_rused->option[sel_option[p]].uncond[c];
			    if (conflict)
			    {
				/* Mark that this option cannot be used,
				 * and stop search by setting p = i
				 */
				overlaps = 1;
#ifdef DEBUG
				fprintf(ERR,
					"\tconflict: resource %s entry %d with entry %d\n",
					reslist->name, i, p);
#endif			       
				p = i;
				break;
			    }
			}
		    }
		}

		/* If overlaps, set that not valid option and continue search*/
		if (overlaps)
		{
		    option_flag = 0;
		    continue;
		}
		
		/* loop thru each cycle required by the option */
		option_flag = 1;
		for (k=(issue_time+rused->start_usage);
			k<=(issue_time+rused->end_usage); k++) {

		    if (! RU_can_place(k, rused, j)) {
#ifdef DEBUG
			fprintf(ERR, "\tconflict: resource %s option %d time %d\n",
					reslist->name, j, k);
#endif
			option_flag = 0;
			break;
		    }
		}

		/* record if found successful option for current resource */
		if (option_flag) {
#ifdef DEBUG
		    fprintf(ERR, "\toption selected: resource %s option %d\n",
					reslist->name, j);
#endif
		    sel_option[i] = j;
		    break;
		}
	    }

	    /* no option for resource is found, mark alternative as unusable */
	    if (! option_flag) {
#ifdef DEBUG
		fprintf(ERR, "\tNO option found : resource %s alt %d\n",
				reslist->name, alt->id);
#endif
		alt_flag = 0;
		break;
	    }
	}

	/* this alternative is selected for scheduling */
	if (alt_flag) {
#ifdef DEBUG
	    fprintf(ERR, "\talt %d selected\n", alt->id);
#endif
	    sel_alt = alt;
	    break;
	}
    }

    /* not successful at scheduling this op */
    if (! alt_flag) {
#ifdef DEBUG
	fprintf(ERR, ">>>>>>>> RU_schedule_op NOT successful for op %d\n",
			ru_info->op->id);
#endif
	free(sel_option);
	if (stats != NULL)
	    stats->num_oper_checks_failed++;
	return -1;
    }

    /* otherwise are successful so reserve resources and build RU_Node structs */
#ifdef DEBUG
	fprintf(ERR, ">>>>>>>>>RU_schedule_op IS successful for op %d\n",
			ru_info->op->id);
#endif

    /* modify RU_Info structure */
    ru_info->selected_alt = sel_alt;
    ru_info->issue_time = issue_time;
    ru_info->slot_used = sel_slot;

    /* reserve resources */
    reslist = sel_alt->reslist;
    for (i=0; i<reslist->num_used; i++) {
	rused = &reslist->used[i];
	for (k=(issue_time+rused->start_usage);
		k<=(issue_time+rused->end_usage); k++) {
	    RU_place(k, rused, sel_option[i], ru_info);
	}
    }
    free(sel_option);
    return (sel_slot);
}

/* Same as RU_schedule_op, but go through options in reverse order.  This
   is for modulo scheduling from last oper to first oper.  This is done
   so that the latest permissible slot is chosen instead of the earliest.
   When scheduling from last oper to first oper, this makes more sense
   when there are dependences with a delay of 0. */
int RU_schedule_op_reverse (RU_Info *ru_info, Mdes_Info *mdes_info,
	int *operand_ready_times, int issue_time,
	int earliest_slot, int latest_slot, int flags)
{
    int i, j, k, *sel_option, sel_slot = 0, alt_flag, option_flag;
    int latency_count, operand_flag;
    Mdes_Compatable_Alt *c_alt;
    Mdes_Alt *alt, *sel_alt = NULL;
    Mdes_ResList *reslist;
    Mdes_Rused *rused, *check_rused;
    int p, c, overlaps, conflict;
    Mdes_Operation *op;
    Mdes_Stats *stats;
			  
    /* Take stats for .lmdes2 scheduling runs */
    if (lmdes->mdes2 != NULL)
    {
	op = lmdes->op_table[mdes_info->opcode];

	if(Lsched_prepass)
	{
	    stats = &op->sched_prepass;
	}
	else
	{
	    stats = &op->sched_postpass;
	}
	stats->num_oper_checks++;
    }
    else
	stats = NULL;

    /* input checking */
    if (ru_info==NULL)
	L_punt("RU_schedule_op: ru_info cannot be NULL");
    if (mdes_info==NULL)
	L_punt("RU_schedule_op: mdes_info cannot be NULL");
    if (mdes_info->num_compatable_alts<=0)
	L_punt("RU_schedule_op: no compatible alternatives exist");
    if (operand_ready_times==NULL)
	L_punt("RU_schedule_op: operand_ready_times is NULL");

#ifdef DEBUG
     fprintf(ERR, "RU_schedule_op: op %d : issue_time %d : earliest slot %d : latest slot %d\n",
			ru_info->op->id, issue_time, earliest_slot, latest_slot);
#endif
#ifdef DEBUG_READY_TIME
    fprintf(ERR, "RU_schedule_op: op %d : ready times\n", ru_info->op->id);
    fprintf(ERR, "\t[%d], lmdes->latency_count");
    for (i=0; i<lmdes->latency_count; i++) {
	fprintf(ERR, " %d", operand_ready_times[i]);
    }
    fprintf(ERR, "\n");
#endif

    /*
     *	First loop thru and see if any alternative can be scheduled, record
     *	option selected for each resource in sel_option array.  Then if
     *	successful, go thru and reserve all resources and create necessary
     *	structures to schedule op at issue_time.
     */

    sel_option = (int *)malloc(sizeof(int)*lmdes->num_resources);
    latency_count = lmdes->latency_count;

    /*
     *	Consider each compatible alternative
     */
    alt_flag = 0;
    for (c_alt=mdes_info->compatable_alts; c_alt!=NULL; c_alt=c_alt->next) {
	alt = c_alt->alt;
#ifdef DEBUG
	fprintf(ERR, "\tAttempt to schedule alt %d\n", alt->id);
#endif


	/*
	 *	Check that flags for alt compatable with passed flags
	 */
	if (! RU_alt_flags_compatible(ru_info, alt, flags)) {
#ifdef DEBUG
	    fprintf(ERR, "\tFlags imcompatible for alt %d\n", alt->id);
#endif
	    continue;
	}


	/*
	 *	Check if operands really ready for this alternative, scheduler
	 *	has best case numbers, so here we really enforce this restriction!
	 */
	operand_flag = 1;
	for (i=0; i<latency_count; i++) {
	    /* check for operand which isnt really ready */
	    if (issue_time < operand_ready_times[i] - alt->latency->operand_latency[i]) {
#ifdef DEBUG_READY_TIME
	        fprintf(ERR, "\tOperand %d not ready for alt %d\n", i, alt->id);
#endif
		operand_flag = 0;
		break;
	    }
	}

	if (! operand_flag)
	    continue;

	/*
	 *	Check resources required by the alternative are free
	 */
	alt_flag = 1;
	reslist = alt->reslist;
	/* Detect if we should be using version2 sm functions */
	if (reslist == NULL)
	{
	    /* Try to schedule, if can commit resources */
	    sel_slot = OLD_SM_sched_table (RU_sm_cb, alt->table, issue_time, 
					   earliest_slot, latest_slot, 1, stats);
	    
	    /* Can we schedule this? */
	    if (sel_slot < 0)
	    {
		/* No.  Mark at unable to schedule and continue */
		alt_flag = 0;
		if (stats != NULL)
		    stats->num_oper_checks_failed++;
		return (-1);
	    }
	    else
	    {
		/* Yes.  Select this slot and stop the search */
		sel_alt = alt;
		/* modify RU_Info structure */
		ru_info->selected_alt = sel_alt;
		ru_info->issue_time = issue_time;
		ru_info->slot_used = sel_slot;
		return (sel_slot);
	    }
	}    
	for (i=0; i<reslist->num_used; i++) {
	    rused = &reslist->used[i];
	    /* loop thru each resource option */
	    option_flag = 0;
	    for (j=rused->num_options-1; j>=0; j--) {

		/* resource0 is slot so need to check earliest slot for i=0 */
		if (i==0) {
		    if (reslist->slot_options[j] < earliest_slot) {
#ifdef DEBUG
			fprintf(ERR, "\tslot %d not compatible\n",
					reslist->slot_options[j]);
#endif
		        continue;
		    }
		    else if (reslist->slot_options[j] > latest_slot) {
#ifdef DEBUG
			fprintf(ERR, "\tslot %d not compatible\n",
					reslist->slot_options[j]);
#endif
		        continue;
		    }
		    else {
			sel_slot = reslist->slot_options[j];
#ifdef DEBUG
			fprintf(ERR, "\tslot %d selected\n", sel_slot);
#endif
		    }
		}

		/* If this Rused option could try to use the same resource
		 * that a previous Rused option in this reslist used
		 * (marked with flag), then do a check to prevent the
		 * improper reuse of this resource.  RU_can_place DOES NOT
		 * detect this situation.
		 * -JCG 1/12/95
		 */
		overlaps = 0;
		if (rused->flags & MDES_OVERLAPPING_REQUEST)
		{
		    for (p = 0; p < i; p++)
		    {
			check_rused = &reslist->used[p];
			if ((check_rused->end_usage < rused->start_usage)||
			    (check_rused->start_usage > rused->end_usage))
			    continue;
			
			for (c=0; c < RU_mask_width; c++)
			{
			    conflict = rused->option[j].uncond[c] &
				check_rused->option[sel_option[p]].uncond[c];
			    if (conflict)
			    {
				/* Mark that this option cannot be used,
				 * and stop search by setting p = i
				 */
				overlaps = 1;
#ifdef DEBUG
				fprintf(ERR,
					"\tconflict: resource %s entry %d with entry %d\n",
					reslist->name, i, p);
#endif			       
				p = i;
				break;
			    }
			}
		    }
		}

		/* If overlaps, set that not valid option and continue search*/
		if (overlaps)
		{
		    option_flag = 0;
		    continue;
		}
		
		/* loop thru each cycle required by the option */
		option_flag = 1;
		for (k=(issue_time+rused->start_usage);
			k<=(issue_time+rused->end_usage); k++) {

		    if (! RU_can_place(k, rused, j)) {
#ifdef DEBUG
			fprintf(ERR, "\tconflict: resource %s option %d time %d\n",
					reslist->name, j, k);
#endif
			option_flag = 0;
			break;
		    }
		}

		/* record if found successful option for current resource */
		if (option_flag) {
#ifdef DEBUG
		    fprintf(ERR, "\toption selected: resource %s option %d\n",
					reslist->name, j);
#endif
		    sel_option[i] = j;
		    break;
		}
	    }

	    /* no option for resource is found, mark alternative as unusable */
	    if (! option_flag) {
#ifdef DEBUG
		fprintf(ERR, "\tNO option found : resource %s alt %d\n",
				reslist->name, alt->id);
#endif
		alt_flag = 0;
		break;
	    }
	}

	/* this alternative is selected for scheduling */
	if (alt_flag) {
#ifdef DEBUG
	    fprintf(ERR, "\talt %d selected\n", alt->id);
#endif
	    sel_alt = alt;
	    break;
	}
    }

    /* not successful at scheduling this op */
    if (! alt_flag) {
#ifdef DEBUG
	fprintf(ERR, ">>>>>>>> RU_schedule_op NOT successful for op %d\n",
			ru_info->op->id);
#endif
	free(sel_option);
	if (stats != NULL)
	    stats->num_oper_checks_failed++;
	return -1;
    }

    /* otherwise are successful so reserve resources and build RU_Node structs */
#ifdef DEBUG
	fprintf(ERR, ">>>>>>>>>RU_schedule_op IS successful for op %d\n",
			ru_info->op->id);
#endif

    /* modify RU_Info structure */
    ru_info->selected_alt = sel_alt;
    ru_info->issue_time = issue_time;
    ru_info->slot_used = sel_slot;

    /* reserve resources */
    reslist = sel_alt->reslist;
    for (i=0; i<reslist->num_used; i++) {
	rused = &reslist->used[i];
	for (k=(issue_time+rused->start_usage);
		k<=(issue_time+rused->end_usage); k++) {
	    RU_place(k, rused, sel_option[i], ru_info);
	}
    }
    free(sel_option);
    return (sel_slot);
}

int RU_can_schedule_op (RU_Info *ru_info, Mdes_Info *mdes_info,
	int *operand_ready_times, int issue_time,
	int earliest_slot, int latest_slot, int flags)
{
    int i, j, k, *sel_option, sel_slot = 0, alt_flag, option_flag,
      latency_count, operand_flag;
    Mdes_Compatable_Alt *c_alt;
    Mdes_Alt *alt, *sel_alt;
    Mdes_ResList *reslist;
    Mdes_Rused *rused, *check_rused;
    int p, c, overlaps, conflict;
    Mdes_Operation *op;
    Mdes_Stats *stats;
			  
#if DEBUG
    printf ("  RU_can_schedule: cycle %i earlist_slot %i lastest_slot %i\n",
	    issue_time, earliest_slot, latest_slot);
#endif

    /* Take stats for .lmdes2 scheduling runs */
    if (lmdes->mdes2 != NULL)
    {
	op = lmdes->op_table[mdes_info->opcode];

	if(Lsched_prepass)
	{
	    stats = &op->can_sched_prepass;
	}
	else
	{
	    stats = &op->can_sched_postpass;
	}
	stats->num_oper_checks++;
    }
    else
	stats = NULL;

    /* input checking */
    if (ru_info==NULL)
        L_punt("RU_can_schedule_op: ru_info cannot be NULL");
    if (mdes_info==NULL)
        L_punt("RU_can_schedule_op: mdes_info cannot be NULL");
    if (mdes_info->num_compatable_alts<=0)
        L_punt("RU_can_schedule_op: no compatible alternatives exist");
    if (operand_ready_times==NULL)
	L_punt("RU_can_schedule_op: operand_ready_times is NULL");
 
#ifdef DEBUG
     fprintf(ERR, "RU_can_schedule_op: op %d : issue_time %d : earliest slot %d : latest slot %d\n",
                        ru_info->op->id, issue_time, earliest_slot, latest_slot);
#endif
#ifdef DEBUG_READY_TIME
    fprintf(ERR, "RU_can_schedule_op: op %d : ready times\n", ru_info->op->id);
    fprintf(ERR, "\t[%d], lmdes->latency_count");
    for (i=0; i<lmdes->latency_count; i++) {
        fprintf(ERR, " %d", operand_ready_times[i]);
    }
    fprintf(ERR, "\n");
#endif

 
    /*
     *  First loop thru and see if any alternative can be scheduled, record
     *  option selected for each resource in sel_option array.  Then if
     *  successful, go thru and reserve all resources and create necessary
     *  structures to schedule op at issue_time.
     */
 
    sel_option = (int *)malloc(sizeof(int)*lmdes->num_resources);
    latency_count = lmdes->latency_count;

    /*
     *  Consider each compatible alternative
     */
    alt_flag = 0;
    for (c_alt=mdes_info->compatable_alts; c_alt!=NULL; c_alt=c_alt->next) {
        alt = c_alt->alt;
#ifdef DEBUG
        fprintf(ERR, "\tAttempt to schedule alt %d\n", alt->id);
#endif

        /*
         *      Check that flags for alt compatable with passed flags
         */
        if (! RU_alt_flags_compatible(ru_info, alt, flags)) {
#ifdef DEBUG
            fprintf(ERR, "\tFlags imcompatible for alt %d\n", alt->id);
#endif
            continue;
        }

        /*
         *      Check if operands really ready for this alternative, scheduler
         *      has best case numbers, so here we really enforce this restriction!
         */
        operand_flag = 1;
        for (i=0; i<latency_count; i++) {
            /* check for operand which isnt really ready */
            if (issue_time < operand_ready_times[i] - alt->latency->operand_latency[i]) {
#ifdef DEBUG_READY_TIME
                fprintf(ERR, "\tOperand %d not ready for alt %d\n", i, alt->id);
#endif
                operand_flag = 0;
                break;
            }
        }

        if (! operand_flag)
            continue;


        /*
         *      Check resources required by the alternative are free
         */
        alt_flag = 1;
        reslist = alt->reslist;
	/* Detect if we should be using version2 sm functions */
	if (reslist == NULL)
	{
	    /* Try to schedule but don't commit resources */
	    sel_slot = OLD_SM_sched_table (RU_sm_cb, alt->table, issue_time, 
					   earliest_slot, latest_slot, 0, 
					   stats);

	    /* Can we schedule this? */
	    if (sel_slot < 0)
	    {
		/* No.  Mark at unable to schedule and continue */
		alt_flag = 0;
		continue;
	    }
	    else
	    {
#ifdef DEBUG
		printf ("  Can schedule alt %i (%s) in cycle %i slot %i\n", 
			alt->id, alt->asm_name, issue_time, sel_slot);
#endif

		/* Yes.  Select this slot and stop the search */
		sel_alt = alt;
		break;
	    }
	}    
	for (i=0; i<reslist->num_used; i++) {
            rused = &reslist->used[i];
            /* loop thru each resource option */
            option_flag = 0;
            for (j=0; j<rused->num_options; j++) {

                /* resource0 is slot so need to check earliest slot for i=0 */
                if (i==0) {
                    if (reslist->slot_options[j] < earliest_slot) {
#ifdef DEBUG
                        fprintf(ERR, "\tslot %d not compatible\n",
                                        reslist->slot_options[j]);
#endif
                        continue;
                    }
                    else if (reslist->slot_options[j] > latest_slot) {
#ifdef DEBUG
                        fprintf(ERR, "\tslot %d not compatible\n",
                                        reslist->slot_options[j]);
#endif
                        continue;
                    }
                    else {
                        sel_slot = reslist->slot_options[j];
#ifdef DEBUG
                        fprintf(ERR, "\tslot %d selected\n", sel_slot);
#endif
                    }
                }

		/* If this Rused option could try to use the same resource
		 * that a previous Rused option in this reslist used
		 * (marked with flag), then do a check to prevent the
		 * improper reuse of this resource.  RU_can_place DOES NOT
		 * detect this situation.
		 * -JCG 1/12/95
		 */
		overlaps = 0;
		if (rused->flags & MDES_OVERLAPPING_REQUEST)
		{
		    for (p = 0; p < i; p++)
		    {
			check_rused = &reslist->used[p];
			if ((check_rused->end_usage < rused->start_usage)||
			    (check_rused->start_usage > rused->end_usage))
			    continue;
			
			for (c=0; c < RU_mask_width; c++)
			{
			    conflict = rused->option[j].uncond[c] &
				check_rused->option[sel_option[p]].uncond[c];
			    if (conflict)
			    {
				/* Mark that this option cannot be used,
				 * and stop search by setting p = i
				 */
				overlaps = 1;
#ifdef DEBUG
				fprintf(ERR,
					"\tconflict: resource %s entry %d with entry %d\n",
					reslist->name, i, p);
#endif
				p = i;
				break;
			    }
			}
		    }
		}

		/* If overlaps, set that not valid option and continue search*/
		if (overlaps)
		{
		    option_flag = 0;
		    continue;
		}
		
                /* loop thru each cycle required by the option */
                option_flag = 1;
                for (k=(issue_time+rused->start_usage);
                        k<=(issue_time+rused->end_usage); k++) {
                    if (! RU_can_place(k, rused, j)) {
#ifdef DEBUG
                        fprintf(ERR, "\tconflict: resource %s option %d time %d\n",
                                        reslist->name, j, k);
#endif
                        option_flag = 0;
                        break;
                    }
                }

                /* record if found successful option for current resource */
                if (option_flag) {
#ifdef DEBUG
                    fprintf(ERR, "\toption selected: resource %s option %d\n",
                                        reslist->name, j);
#endif
                    sel_option[i] = j;
                    break;
                }
            }

            /* no option for resource is found, mark alternative as unusable */
            if (! option_flag) {
#ifdef DEBUG
                fprintf(ERR, "\tNO option found : resource %s alt %d\n",
                                reslist->name, alt->id);
#endif
                alt_flag = 0;
                break;
            }
        }

	/* this alternative is selected for scheduling */
        if (alt_flag) {
#ifdef DEBUG
            fprintf(ERR, "\talt %d selected\n", alt->id);
#endif
            sel_alt = alt;
            break;
        }
    }
 
    /* not successful at scheduling this op */
    if (! alt_flag) {
#ifdef DEBUG
        fprintf(ERR, ">>>>>>>> RU_can_schedule_op NOT successful for op %d\n",
                        ru_info->op->id);
#endif
        free(sel_option);
	if (stats != NULL)
	    stats->num_oper_checks_failed++;
        return -1;
    }
 
    /* otherwise are successful */
#ifdef DEBUG
        fprintf(ERR, ">>>>>>>>>RU_can_schedule_op IS successful for op %d\n",
                        ru_info->op->id);
#endif

    free(sel_option); 
    return (sel_slot);
}


/*
 *	Same as above, but try to schedule op at issue_time in the
 *	specified slot, return -1 if cannot do so.
 */
int RU_schedule_op_at (RU_Info *ru_info, Mdes_Info *mdes_info,
	int *operand_ready_times, int issue_time,
	int slot, int flags)
{
    int i, j, k, *sel_option, sel_slot = 0, alt_flag, option_flag,
      latency_count, operand_flag;
    
    Mdes_Compatable_Alt *c_alt;
    Mdes_Alt *alt, *sel_alt = NULL;
    Mdes_ResList *reslist = NULL;
    Mdes_Rused *rused, *check_rused;
    int p, c, overlaps, conflict;
    Mdes_Operation *op;
    Mdes_Stats *stats;
			  
    /* Take stats for .lmdes2 scheduling runs */
    if (lmdes->mdes2 != NULL)
    {
	op = lmdes->op_table[mdes_info->opcode];

	if(Lsched_prepass)
	{
	    stats = &op->sched_prepass;
	}
	else
	{
	    stats = &op->sched_postpass;
	}
	stats->num_oper_checks++;
    }
    else
	stats = NULL;

    /* input checking */
    if (ru_info==NULL)
        L_punt("RU_schedule_op_at: ru_info cannot be NULL");
    if (mdes_info==NULL)
        L_punt("RU_schedule_op_at: mdes_info cannot be NULL");
    if (mdes_info->num_compatable_alts<=0)
        L_punt("RU_schedule_op_at: no compatible alternatives exist");
    if (operand_ready_times==NULL)
	L_punt("RU_can_schedule_op: operand_ready_times is NULL");
 
#ifdef DEBUG
     fprintf(ERR, "RU_schedule_op_at: op %d : issue_time %d : slot %d\n",
                        ru_info->op->id, issue_time, slot);
#endif
#ifdef DEBUG_READY_TIME
    fprintf(ERR, "RU_schedule_op_at: op %d : ready times\n", ru_info->op->id);
    fprintf(ERR, "\t[%d], lmdes->latency_count");
    for (i=0; i<lmdes->latency_count; i++) {
        fprintf(ERR, " %d", operand_ready_times[i]);
    }
    fprintf(ERR, "\n");
#endif
 
    /*
     *  First loop thru and see if any alternative can be scheduled, record
     *  option selected for each resource in sel_option array.  Then if
     *  successful, go thru and reserve all resources and create necessary
     *  structures to schedule op at issue_time.
     */
 
    sel_option = (int *)malloc(sizeof(int)*lmdes->num_resources);
    latency_count = lmdes->latency_count;

    /*
     *  Consider each compatible alternative
     */
    alt_flag = 0;
    for (c_alt=mdes_info->compatable_alts; c_alt!=NULL; c_alt=c_alt->next) {
        alt = c_alt->alt;
#ifdef DEBUG
        fprintf(ERR, "\tAttempt to schedule alt %d\n", alt->id);
#endif


        /*
         *      Check that flags for alt compatable with passed flags
         */
        if (! RU_alt_flags_compatible(ru_info, alt, flags)) {
#ifdef DEBUG
            fprintf(ERR, "\tFlags imcompatible for alt %d\n", alt->id);
#endif
            continue;
        }


        /*
         *      Check if operands really ready for this alternative, scheduler
         *      has best case numbers, so here we really enforce this restriction!
         */
        operand_flag = 1;
        for (i=0; i<latency_count; i++) {
            /* check for operand which isnt really ready */
            if (issue_time < operand_ready_times[i] - alt->latency->operand_latency[i]) {
#ifdef DEBUG_READY_TIME
                fprintf(ERR, "\tOperand %d not ready for alt %d\n", i, alt->id);
#endif
                operand_flag = 0;
                break;
            }
        }

        if (! operand_flag)
            continue;


        /*
         *      Check resources required by the alternative are free
         */
        alt_flag = 1;
        reslist = alt->reslist;

	/* Detect if we should be using version2 sm functions */
	if (reslist == NULL)
	{
	    /* Try to schedule table and commit resource if can schedule */
	    sel_slot = OLD_SM_sched_table (RU_sm_cb, alt->table, issue_time, 
					   slot, slot, 1, stats);

	    /* Can we schedule this? */
	    if (sel_slot < 0)
	    {
		/* No.  Mark at unable to schedule and continue */
		alt_flag = 0;
		continue;
	    }
	    else
	    {
		/* Yes.  Select this slot and stop the search */
		sel_alt = alt;
		break;
	    }
	}
        for (i=0; i<reslist->num_used; i++) {
            rused = &reslist->used[i];
            /* loop thru each resource option */
            option_flag = 0;
            for (j=0; j<rused->num_options; j++) {

                /* resource0 is slot so need to check earliest slot for i=0 */
                if (i==0) {
                    if (reslist->slot_options[j] != slot) {
#ifdef DEBUG
                        fprintf(ERR, "\tslot %d not compatible\n",
                                        reslist->slot_options[j]);
#endif
                        continue;
                    }
                    else {
                        sel_slot = reslist->slot_options[j];
#ifdef DEBUG
                        fprintf(ERR, "\tslot %d selected\n", sel_slot);
#endif
                    }
                }

		/* If this Rused option could try to use the same resource
		 * that a previous Rused option in this reslist used
		 * (marked with flag), then do a check to prevent the
		 * improper reuse of this resource.  RU_can_place DOES NOT
		 * detect this situation.
		 * -JCG 1/12/95
		 */
		overlaps = 0;
		if (rused->flags & MDES_OVERLAPPING_REQUEST)
		{
		    for (p = 0; p < i; p++)
		    {
			check_rused = &reslist->used[p];
			if ((check_rused->end_usage < rused->start_usage)||
			    (check_rused->start_usage > rused->end_usage))
			    continue;
			
			for (c=0; c < RU_mask_width; c++)
			{
			    conflict = rused->option[j].uncond[c] &
				check_rused->option[sel_option[p]].uncond[c];
			    if (conflict)
			    {
				/* Mark that this option cannot be used,
				 * and stop search by setting p = i
				 */
				overlaps = 1;
#ifdef DEBUG
				fprintf(ERR,
					"\tconflict: resource %s entry %d with entry %d\n",
					reslist->name, i, p);
#endif
				p = i;
				break;
			    }
			}
		    }
		}

		/* If overlaps, set that not valid option and continue search*/
		if (overlaps)
		{
		    option_flag = 0;
		    continue;
		}

                /* loop thru each cycle required by the option */
                option_flag = 1;
                for (k=(issue_time+rused->start_usage);
                        k<=(issue_time+rused->end_usage); k++) {
                    if (! RU_can_place(k, rused, j)) {
#ifdef DEBUG
                        fprintf(ERR, "\tconflict: resource %s option %d time %d\n",
                                        reslist->name, j, k);
#endif
                        option_flag = 0;
                        break;
                    }
                }

                /* record if found successful option for current resource */
                if (option_flag) {
#ifdef DEBUG
                    fprintf(ERR, "\toption selected: resource %s option %d\n",
                                        reslist->name, j);
#endif
                    sel_option[i] = j;
                    break;
                }
            }
            /* no option for resource is found, mark alternative as unusable */
            if (! option_flag) {
#ifdef DEBUG
                fprintf(ERR, "\tNO option found : resource %s alt %d\n",
                                reslist->name, alt->id);
#endif
                alt_flag = 0;
                break;
            }
        }

        /* this alternative is selected for scheduling */
        if (alt_flag) {
#ifdef DEBUG
            fprintf(ERR, "\talt %d selected\n", alt->id);
#endif
            sel_alt = alt;
            break;
        }
    }
 
    /* not successful at scheduling this op */
    if (! alt_flag) {
#ifdef DEBUG
        fprintf(ERR, ">>>>>>>> RU_schedule_op_at NOT successful for op %d\n",
                        ru_info->op->id);
#endif
        free(sel_option);
	if (stats != NULL)
	    stats->num_oper_checks_failed++;
        return -1;
    }
 
    /* otherwise are successful so reserve resources and build RU_Node structs */
#ifdef DEBUG
        fprintf(ERR, ">>>>>>>>>RU_schedule_op_at IS successful for op %d\n",
                        ru_info->op->id);
#endif
 
    /* modify RU_Info structure */
    ru_info->selected_alt = sel_alt;
    ru_info->issue_time = issue_time;
    ru_info->slot_used = sel_slot;
 
    /* reserve resources (if version1)*/
    if (reslist != NULL)
    {
	reslist = sel_alt->reslist;
	for (i=0; i<reslist->num_used; i++) {
	    rused = &reslist->used[i];
	    for (k=(issue_time+rused->start_usage);
		 k<=(issue_time+rused->end_usage); k++) {
		RU_place(k, rused, sel_option[i], ru_info);
	    }
	}
    }
    free(sel_option);
    return (sel_slot);
}

void RU_unschedule_op (RU_Info *ru_info)
{
    int i, k, issue_time;
    Mdes_Alt *sel_alt;
    Mdes_ResList *reslist;
    Mdes_Rused *rused;

    /* input checking */
    if (ru_info==NULL)
	L_punt("RU_unschedule_op: ru_info cannot be NULL");

    sel_alt = ru_info->selected_alt;
    issue_time = ru_info->issue_time;

    /* free up the resources */
    reslist = sel_alt->reslist;
    for (i=0; i<reslist->num_used; i++) {
	rused = &reslist->used[i];
	for (k=(issue_time+rused->start_usage);
		k<=(issue_time+rused->end_usage); k++) {
	    RU_unplace(k, rused, ru_info);
	}
    }

    /* reset RU_Info structure */
    ru_info->selected_alt = NULL;
    ru_info->issue_time = -1;
    ru_info->slot_used = -1;
}

/*=========================================================================*/
/*
 *      RU_Map debugging only!!!!!!!!!!!!
 */
/*=========================================================================*/

#if 0
main()
{
    L_init_lmdes ("lmdes");
    RU_map_create(64);
    RU_map_init(RU_MODE_ACYCLIC);
    RU_map_realloc();
    RU_map_delete();

    RU_pred_print_alloc_data(stdout, 1);
    RU_info_print_alloc_data(stdout, 1);
    RU_node_print_alloc_data(stdout, 1);
}
#endif


