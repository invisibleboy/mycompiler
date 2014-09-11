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
 *  File:  RU_manager.h	(Resource Usage Manager)
 *
 *  Description:
 *    Header file for resource usage manager
 *
 *  Creation Date : May 1993
 *
 *  Authors : Scott Mahlke, John Gyllenhaal
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:02  david
 *  Import of IMPACT source
 *
 * Revision 1.1  1994/01/19  18:49:06  roger
 * Initial revision
 *
 * Lavery - Added external functions RU_number_of_alts() and 
 * 6/94     RU_update_usage_count() 
 *
\*****************************************************************************/
#ifndef RU_MANAGER_H
#define RU_MANAGER_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <Lcode/l_code.h>
#include <machine/lmdes.h>

#define RU_MAP_DEFAULT_SIZE		128

#define RU_MODE_ACYCLIC			0
#define RU_MODE_CYCLIC			1

/*=========================================================================*/
/*
 *	Data structures
 */
/*=========================================================================*/

/*
 *	Memory allocation structures
 */

typedef struct _RU_Alloc_Header
{
        struct _RU_Alloc_Header   *next;
} RU_Alloc_Header;

typedef struct _RU_Alloc_Data
{
        RU_Alloc_Header	*head;          /* Pointer to head of free list */
        int		allocated;      /* Number allocated */
        int		free;           /* Number free */
} RU_Alloc_Data;

/*
 *	RU info structure associated with each Lcode op
 */

#define RU_SELECTED_ALT(info)((Mdes_Alt *)(info->selected_alt))
#define RU_SELECTED_ALT_ID(info)((int)(info->selected_alt->id))

typedef struct _RU_Info
{
	struct L_Oper		*op;		/* debugging purpose only */
	int		proc_opc;	/* opcode */
	int 		*pred;		/* indices of predicates used by op */
	Mdes_Alt	*selected_alt;	/* alternative selected by RU manager */
	int		issue_time;	/* time op is issued at */
	int 		slot_used;	/* slot op is placed in */
} RU_Info;

/*
 *	RU map structure
 */

typedef struct _RU_Node
{
	RU_Info		*info;
	Mdes_Rused	*rused;
	int 		option_num;
	struct _RU_Node *prev_node;
	struct _RU_Node	*next_node;
} RU_Node;

typedef struct _RU_Map
{
	Mdes_Rmask	mask;
	RU_Node		*first_node;
	RU_Node		*last_node;
} RU_Map;

/*=========================================================================*/
/*
 *	Global variables
 */
/*=========================================================================*/

extern RU_Map *RU_map;
extern int RU_map_length;
extern int RU_map_mode;
extern int RU_map_cycles;
extern int *RU_mask;
extern int RU_mask_width;
extern int RU_max_pred;

/*=========================================================================*/
/*
 *	External functions
 */
/*=========================================================================*/

/*
 *	RU_pred
 */
extern void RU_set_max_pred (int);
	/* (int max) */

extern int *RU_pred_alloc ();
	/* () */

extern void RU_pred_free (int *);
	/* (int *ptr) */

extern void RU_pred_print_alloc_data (FILE *, int);
	/* (FILE *F, int verbose) */

/*
 *	RU_info
 */
extern RU_Info *RU_info_alloc ();
	/* () */

extern void RU_info_free (RU_Info *);
	/* (RU_Info *ptr) */

extern void RU_info_print_alloc_data (FILE *, int);
	/* (FILE *F, int verbose) */

extern RU_Info *RU_info_create (L_Oper *, int *);
	/* (L_Oper *op, int *pred) */

extern void RU_info_delete (RU_Info *);
	/* (RU_Info *info) */ 

/*
 *	RU_node
 */
extern RU_Node *RU_node_alloc ();
	/* () */

extern void RU_node_free (RU_Node *);
	/* (RU_Node *ptr) */

extern void RU_node_print_alloc_data (FILE *, int);
	/* (FILE *F, int verbose) */

extern RU_Node *RU_node_create (RU_Info *);
	/* (RU_Info *info) */

extern void RU_node_delete (RU_Map *, RU_Node *);
	/* (RU_Map *map, RU_Node *node) */

extern void RU_node_delete_all (RU_Node *);
	/* (RU_Node *list) */

extern void RU_node_insert_before (RU_Map *, RU_Node *, RU_Node *);
	/* (RU_Map *map, RU_Node *before_node, RU_Node *node) */

extern void RU_node_insert_after (RU_Map *, RU_Node *, RU_Node *);
	/* (RU_Map *map, RU_Node *after_node, RU_Node *node) */

extern RU_Node *RU_node_find (RU_Node *, RU_Info *, Mdes_Rused *);
	/* (RU_Node *list, RU_Info *ru_info, Mdes_Rused *rused) */

/*
 *	RU_map
 */
extern void RU_map_create (int);
	/* (int length) */

extern void RU_map_realloc ();
	/* () */

extern void RU_map_init (int, int);
	/* (int mode, int cycles) */
        /* mode = RU_ACYCLIC_MODE or RU_CYCLIC_MODE, cycles only matters for
           cyclic mode to specify ii for cyclic schedule. */

extern void RU_map_delete ();
	/* () */

/*
 *	Resource MII functions
 */
extern int RU_number_of_alts(RU_Info *, Mdes_Info *, int);
	/* (RU_Info *ru_info, Mdes_Info *mdes_info, int flags) */

extern void RU_update_usage_count(RU_Info *, Mdes_Info *, int *, int);
      /* (RU_Info *ru_info, Mdes_Info *mdes_info, int *ru_count, int flags) */

/*
 *	RU_map scheduling functions
 */
extern int RU_alt_flags_compatible (RU_Info *, Mdes_Alt *, int);
	/* (RU_Info *ru_info, Mdes_Alt *alt, int flags) */

extern int RU_can_place (int, Mdes_Rused *, int);
	/* (int time, Mdes_Rused *rused, int option_num) */

extern void RU_place (int, Mdes_Rused *, int, RU_Info *);
	/* (int time, Mdes_Rused *rused, int option_num,
	    RU_Info *ru_info) */

extern void RU_unplace (int, Mdes_Rused *, RU_Info *);
	/* (int time, Mdes_Rused *rused, RU_Info *ru_info) */

extern int RU_schedule_op (RU_Info *, Mdes_Info *, int *, int, int, int, int);
	/* (RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times,
	    int issue_time, int earliest_slot, int latest_slot, int flags) */
	/* (return = slot scheduled in, -1 if cannot be scheduled) */

extern int RU_schedule_op_reverse (RU_Info *, Mdes_Info *, int *, int, int, int, int);
	/* (RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times,
	    int issue_time, int earliest_slot, int latest_slot, int flags) */
	/* (return = slot scheduled in, -1 if cannot be scheduled) */

extern int RU_can_schedule_op (RU_Info *, Mdes_Info *, int *, int, int, int, int);
	/* (RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times,
	    int issue_time, int earliest_slot, int latest_slot, int flags) */
        /* (return = slot can be scheduled in, -1 if cannot be scheduled) */

extern int RU_schedule_op_at (RU_Info *, Mdes_Info *, int *, int, int, int);
	/* (RU_Info *ru_info, Mdes_Info *mdes_info, int *operand_ready_times,
	    int issue_time, int slot, int flags) */
	/* (return = slot scheduled in, -1 if cannot be scheduled) */

extern void RU_unschedule_op (RU_Info *);
	/* (RU_Info *ru_info) */

#endif
