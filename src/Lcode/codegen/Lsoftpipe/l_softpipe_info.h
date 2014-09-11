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
 *      File: l_softpipe_info.h
 *      Description: Structures and external function declarations for
 *                   modulo scheduler
 *      Creation Date: January, 1994
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

#ifndef L_SOFTPIPE_INFO_H
#define L_SOFTPIPE_INFO_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_code.h>

/* Info about a loop variant.  One of these is pointed to by the
   Softpipe_Op_Info structure for each destination register in the
   instruction (except those destination registers that are being
   redefined.  All uses of the loop variant point to a Src_MVE_Info
   structure which in turn points to the Lpipe_LRInfo associated
   with the variant.  There is only one of these for each loop
   variant.  Multiple definitions of the same variant all point to the
   same structure. */

typedef struct _Lpipe_LRInfo {
  SM_Reg_Info *rinfo;
  SM_Oper *def_sm_oper;		 /* pointer to sm oper that is the reference
				    definition for the variant */
  int lifetime;			 /* lifetime of variant in cycles */
  int first_def_time;		 /* issue time of the oper that first defined 
				    the variant */
  int first_def_slot;		 /* issue slot of oper that first defined
				    the variant */
  int live_in_def_time;		 /* live_in def time relative to the
				    reference def (def_time). */
  int last_access_time;		 /* issue time of the oper that last used
				    or defined the variant */
  int last_access_slot;	 	 /* issue slot of oper that last used
				    or defined the variant */
  int num_names;		 /* number of unique names for MVE.
				    for rot regs, this the number of
				    names needed in the kernel, plus 1 for
				    the original name in names[0]. */

  int num_names_w_pro;
  L_Operand **names;		 /* array of pointers to L_Operands to
				    be copied when renaming for MVE.  
				    names[0] points to a copy of the 
				    original name for the variant */
  unsigned int use_after_def_slot:1; 
                                 /* flag to indicate that the last use of 
				    this variant is scheduled in a higher
				    numbered slot than the definition of
				    the variant. */
  unsigned int live_out:1;       /* flag to indicate if variant is live out
				    of loop */
  unsigned int live_in:1;	 /* flag to indicate if the range is live in */
  unsigned int prologue_mov_inserted:1;
                                 /* flag to indicate if a move instruction
				    has been inserted before prologue to
				    fix up live in value.  Currently not
				    used.  */
} Lpipe_LRInfo;

/* Info about the use of each loop variant.  One of these is attached
   to the Softpipe_Op_Info structure for each source register that
   uses the variant. */

typedef struct _Lpipe_MVEInfo {
  struct _Lpipe_LRInfo *live_range; /* pointer to the
                                          Lpipe_LRInfo associated
                                          with the variant */
  int lifetime;		/* lifetime from the reference definition of
				   the variant to this particular use */
  int stage_lifetime_incr;	/* flag to indicate that stage lifetime
				   needs to be rounded up during renaming */
} Lpipe_MVEInfo;

/* This structure is attached to each oper in the original loop body and
   contains all the info (or pointers to it) needed for scheduling and MVE */

typedef struct _Softpipe_Op_Info {
  /* First pass: information gathering. */
  int home_block;		/* Basic block in the cb in which
				   this oper resided before modulo
				   scheduling */
  L_Cb *exit_cb;		/* cb to which control flows on exit
				   for branches (only for branches) */
  double exit_weight;		/* weight of the exit flow for 
				   branches (only for branches) */
  /* Post-scheduling. */
  int intra_iter_issue_time;	/* issue time within a single
				   iteration */
  int issue_time;		/* During scheduling, this is the same
				   as the intra_iter_issue_time below.
				   During unrolling and code schema
				   generation, this is the issue time of
				   the oper within its cb.  Can be used
				   for scheduler estimates for execution
				   time. */
  int issue_slot;
  int stage;			/* The number of the intra-iteration 
				   stage that the oper is in.  Intra-
				   iteration stages are numbered 0 to
				   stage_count-1. */
  /* MVE */
  int kernel_copy;		/* If oper is in the kernel, this is the 
				   copy of the kernel which contains
				   this oper.  The kernel copies are
				   numbered 0 to unroll-1.  If oper is in 
				   prologue or epilogue, this is the copy 
				   of the kernel which the oper was 
				   copied from. */
  Lpipe_MVEInfo **src_mve_info;	   /* pointer to array of pointers
				      to Lpipe_MVEInfo structures */
  Lpipe_MVEInfo **pred_mve_info;   /* pointer to array of pointers
				      to Lpipe_MVEInfo structures */
  Lpipe_MVEInfo **dest_mve_info;   /* pointer to array of pointers
				      to Lpipe_MVEInfo structures */

  List isrc_mve_info;              /* List of implicit src MVE info */

  /* Gen Epi-/Pro-logues */
  int prologue_stage;		   /* The number of the prologue
				      stage that the oper is in. Prologue
				      stages are numbered 0 to
				      stage_count-2. */
  int epilogue_stage; 		   /* The number of the epilogue stage
				      that the oper is in.  Epilogue
				      stages are numbered 0 to
				      stage_count-theta-1.  Epilogue
				      stage 0 is for the opers remaining
				      in same kernel copy as the exit
				      branch. */
  int unrolled_iter_num; 	   /* Unique id for each copy of
				      the unrolled kernel in a
				      prologue or epilogue.  Used
				      for maintaining sync arcs for
				      prologue and epilogues. */
  /* Scheduler-specific fields */
  /* set by Lpipe_compute_slack_time_using_MinDist */
  int estart;			   /* earliest start time for priority */
  int lstart;			   /* latest start time for priority */
  int slack;			   /* lstart - estart for priority */
  /* set by Lpipe_compute_static_priorities */
  int priority;			   /* scheduling priority */
  /* set by Lpipe_compute_ready_time based on scheduled ops */
  int ready_time;		   /* time when all dependences will be
				      satisfied */

  /* flags */

  unsigned int loop_back_br:1;     /* BOOL: op is the loopback branch 
				    *   or a copy thereof. */
  unsigned int branch_path_node:1; /* BOOL: op on which the loopback branch
				    * depends */
  unsigned int scheduled:1;	   /* BOOL: op has been scheduled */

} Softpipe_Op_Info;

extern L_Alloc_Pool *Softpipe_Op_Info_pool;

extern Softpipe_Op_Info *Lpipe_create_op_info ();
extern void Lpipe_init_op_info (SM_Oper *, int, int);
extern void Lpipe_construct_op_info (SM_Cb *, SM_Oper *, int *, int *);
extern Softpipe_Op_Info *Lpipe_copy_op_info (Softpipe_Op_Info *);
extern void Lpipe_free_op_info (L_Cb *);
/* 20030110 SZU
 * Added to free for just one op; used in start_stop nodes
 */
extern void Lpipe_free_oper_op_info (L_Oper *);
extern void Lpipe_print_op_info (SM_Oper *);

extern void Lpipe_print_cb_info (SM_Cb *);

extern void Lpipe_copy_sched_info (SM_Cb *);

#endif
