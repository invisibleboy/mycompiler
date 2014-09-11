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
 *      File: l_mve.c
 *      Description: Modulo Variable Expansion and related routines
 *      Creation Date: January, 1994
 *                     Some MVE algorithms based on those in 
 *                     l_pred2_sched.c, created November 1991 by Nancy Warter
 *                     Support for renaming predicates added by Noubar 
 *                     Partamian
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_softpipe_int.h"
#include "l_mve.h"
#include <assert.h>

/*************************************************************************
                Global Variables
*************************************************************************/

/* memory allocation pools for arrays of pointers to Lpipe_LRInfo and 
   Lpipe_MVEInfo structures associated with the source and destination 
   registers of an oper.  The arrays are attached to Softpipe_Op_Info
   structures */

L_Alloc_Pool *Lpipe_src_mve_pool = NULL;	/* for array of pointers to
						   Lpipe_MVEInfo structures */
L_Alloc_Pool *Lpipe_pred_mve_pool = NULL;	/* for array of pointers to
						   Lpipe_MVEInfo structures */
L_Alloc_Pool *Lpipe_dest_mve_pool = NULL;	/* for array of pointers to
						   Lpipe_MVEInfo structures */

/* memory allocation pools for the info structures themselves */

static L_Alloc_Pool *Lpipe_LRInfo_pool = NULL;	/* Lpipe_LRInfo structs */
L_Alloc_Pool *Lpipe_MVEInfo_pool = NULL;	/* Lpipe_MVEInfo structures */

static List Lpipe_mve_lr_list = NULL;	
/* pointer to list of loop variants for the loop */

/*************************************************************************
                Live Range Function Definitions
*************************************************************************/

#if 1
#define LPIPE_LR_TYPE (SM_REGISTER_TYPE|SM_MACRO_TYPE)
#else
#define LPIPE_LR_TYPE (SM_REGISTER_TYPE)
#endif

/* create and initialize a Lpipe_LRInfo structure */
static Lpipe_LRInfo *
Lpipe_create_live_range_info (SM_Reg_Info * rinfo)
{
  Lpipe_LRInfo *lr_info;

  if (!Lpipe_LRInfo_pool)
    Lpipe_LRInfo_pool = 
      L_create_alloc_pool ("Lpipe_LRInfo", sizeof (Lpipe_LRInfo), 50);

  lr_info = (Lpipe_LRInfo *) L_alloc (Lpipe_LRInfo_pool);

  lr_info->rinfo = rinfo;
  lr_info->lifetime = 0;
  lr_info->def_sm_oper = NULL;
  lr_info->live_in_def_time = -1;
  lr_info->first_def_time = -1;
  lr_info->first_def_slot = -1;
  lr_info->last_access_slot = -1;
  lr_info->last_access_slot = -1;
  lr_info->use_after_def_slot = 0;
  lr_info->num_names = 0;
  lr_info->num_names_w_pro = 0;
  lr_info->names = NULL;
  lr_info->live_out = 0;
  lr_info->live_in = 0;
  lr_info->prologue_mov_inserted = 0;

  if (rinfo->first_def)
    {
      SM_Reg_Action *raction, *rraction = NULL;
      int rtime = 0, rslot = 0;
      Softpipe_Op_Info *sinfo;

      /* We must identify the first-scheduled definition---this is the
       * "reference" definition for this live range.
       */

      for (raction = rinfo->first_def; raction; raction = raction->next_def)
	{
	  if (!raction->sm_op)
	    continue;

	  sinfo = SOFTPIPE_OP_INFO (raction->sm_op->lcode_op);

#if 0
	  if (sinfo->issue_time != raction->sm_op->sched_cycle + 
	      raction->sm_op->sm_cb->sched_cycle_offset ||
	      sinfo->issue_slot != raction->sm_op->sched_slot)
	    fprintf (stderr, "SWP/SM MISMATCH (op %d) %d:%d != %d:%d\n", 
		     raction->sm_op->lcode_op->id,
		     sinfo->issue_time, sinfo->issue_slot, 
		     raction->sm_op->sched_cycle + 
		     raction->sm_op->sm_cb->sched_cycle_offset, 
		     raction->sm_op->sched_slot);
#endif

	  if (!rraction || 
	      sinfo->issue_time < rtime ||
	      (sinfo->issue_time == rtime && sinfo->issue_slot < rslot))
	    {
	      rraction = raction;
	      rtime = sinfo->issue_time;
	      rslot = sinfo->issue_slot;
	    }
	}
    
      assert (rraction);

      /* first (reference) definition identified */

      lr_info->def_sm_oper = rraction->sm_op;
      lr_info->first_def_time = lr_info->last_access_time = rtime;
      lr_info->first_def_slot = lr_info->last_access_slot = rslot;
    }

  rinfo->ext = (void *)lr_info;

  return (lr_info);
}


static List
Lpipe_delete_all_live_range_info (List lr_list)
{
  Lpipe_LRInfo *lr_info;

  /* free Lpipe_LRInfo structures */

  List_start (lr_list);
  while ((lr_info = (Lpipe_LRInfo *) List_next (lr_list)))
    {
      if (lr_info->names)
	{
	  int i;
	  for (i = 0; i < lr_info->num_names_w_pro; i++)
	    L_delete_operand (lr_info->names[i]);
	  Lcode_free (lr_info->names);
	  lr_info->names = NULL;
	}
      lr_info->num_names = 0;
      lr_info->num_names_w_pro = 0;
      L_free (Lpipe_LRInfo_pool, lr_info);
      lr_list = List_delete_current (lr_list);
    }

  List_reset (lr_list);

  return NULL;
}


/* find live range info structure for vreg if it exists */
static Lpipe_LRInfo *
Lpipe_find_live_range (List lr_list, SM_Reg_Info * rinfo)
{
  Lpipe_LRInfo *lr_info;

  List_start (lr_list);
  while ((lr_info = (Lpipe_LRInfo *) List_next (lr_list)))
    {
      if (lr_info->rinfo == rinfo)
	return lr_info;
    }

  return NULL;
}


static void
Lpipe_update_lr_last_acc (Lpipe_LRInfo * lr_info,
			  int issue_time, int issue_slot, int ii)
{
  if (issue_time > lr_info->last_access_time)
    {
      lr_info->last_access_time = issue_time;
      lr_info->last_access_slot = issue_slot;
    }
  else if (issue_time == lr_info->last_access_time)
    {
      if (issue_slot > lr_info->last_access_slot)
	lr_info->last_access_slot = issue_slot;
    }

  /* Derived entries track changes */

  lr_info->lifetime = lr_info->last_access_time - lr_info->first_def_time;
  lr_info->use_after_def_slot = !(lr_info->lifetime % ii) &&
    (lr_info->last_access_slot > lr_info->first_def_slot);

  return;
}


static Lpipe_MVEInfo *
Lpipe_new_opd_mve_info (Lpipe_LRInfo * lr)
{
  Lpipe_MVEInfo *mve_info;

  mve_info = (Lpipe_MVEInfo *) L_alloc (Lpipe_MVEInfo_pool);

  mve_info->lifetime = 0;
  mve_info->live_range = lr;
  mve_info->stage_lifetime_incr = 0;
  return mve_info;
}


static void
Lpipe_print_live_ranges (List lr_list, int ii)
{
  Lpipe_LRInfo *lr;
  int i;

  fprintf (stderr, "BEGIN LR LIST FROM MVE ANALYSIS (II = %d)\n", ii);

  if (List_size (lr_list))
    fprintf (stderr, "VREG   LT    RD  LIDEF      FDEF      "
	     "LACC UD LI LO  NN NAMES\n");

  List_start (lr_list);
  while ((lr = (Lpipe_LRInfo *) List_next (lr_list)))
    {
      fprintf (stderr, "%4d %4d %5d %6d %6d/%2d %6d/%2d %2u %2u %2u %3d ",
	       lr->rinfo->id, lr->lifetime, lr->def_sm_oper->lcode_op->id,
	       lr->live_in_def_time, lr->first_def_time,
	       lr->first_def_slot, lr->last_access_time, lr->last_access_slot,
	       lr->use_after_def_slot, lr->live_in, lr->live_out,
	       lr->num_names);
      if (!lr->names)
	{
	  fprintf (stderr, "     X");
	}
      else
	{
	  for (i = 0; i < lr->num_names; i++)
	    fprintf (stderr, "%6d", lr->names[i]->value.r);
	}

      fprintf (stderr, "\n");
    }

  fprintf (stderr, "END LR LIST\n");

}

/*************************************************************************
                MVE Service Function Definitions
*************************************************************************/

/* decrement count modulo unroll */
static int
Lpipe_modulo_decrement (int count, int unrolled)
{
  if (count == 0)
    return (unrolled - 1);
  else
    return (count - 1);
}


static void
Lpipe_print_mve_info (SM_Cb * sm_cb)
{
  SM_Oper *sm_op;
  Softpipe_Op_Info *spinfo;
  Lpipe_MVEInfo *mve_info;
  L_Oper *oper;
  int i;

  fprintf (stderr, "BEGIN MVE INFO FROM MVE ANALYSIS\n");

  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      oper = sm_op->lcode_op;
      spinfo = SOFTPIPE_OP_INFO (oper);

      fprintf (stderr, "(OP %d ", oper->id);
      fprintf (stderr, "<");
      mve_info = spinfo->pred_mve_info[0];
      if (mve_info)
	fprintf (stderr, "(%d %d %d)", oper->pred[0]->value.r,
		 mve_info->lifetime, 
		 mve_info->stage_lifetime_incr);
      else
	fprintf (stderr, "()");
      fprintf (stderr, ">[ ");
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  mve_info = spinfo->dest_mve_info[i];
	  if (mve_info)
	    fprintf (stderr, "(%d %d %d)", oper->dest[i]->value.r,
		     mve_info->lifetime,
		     mve_info->stage_lifetime_incr);
	  else
	    fprintf (stderr, "()");
	}
      fprintf (stderr, "][ ");
      for (i = 0; i < L_max_src_operand; i++)
	{
	  mve_info = spinfo->src_mve_info[i];
	  if (mve_info)
	    fprintf (stderr, "(%d %d %d)", oper->src[i]->value.r,
		     mve_info->lifetime,
		     mve_info->stage_lifetime_incr);
	  else
	    fprintf (stderr, "()");
	}
      fprintf (stderr, "])\n");
    }

  fprintf (stderr, "END MVE INFO\n");

}


static void
Lpipe_delete_all_mve_info (L_Oper * oper)
{
  Softpipe_Op_Info *softpipe_info = SOFTPIPE_OP_INFO (oper);
  Lpipe_MVEInfo *mve_info;
  int i;

  for (i = 0; i < L_max_src_operand; i++)
    if (softpipe_info->src_mve_info[i] != NULL)
      {
	L_free (Lpipe_MVEInfo_pool, softpipe_info->src_mve_info[i]);
	softpipe_info->src_mve_info[i] = NULL;
      }
  for (i = 0; i < L_max_pred_operand; i++)
    if (softpipe_info->pred_mve_info[i] != NULL)
      {
	L_free (Lpipe_MVEInfo_pool, softpipe_info->pred_mve_info[i]);
	softpipe_info->pred_mve_info[i] = NULL;
      }
  for (i = 0; i < L_max_dest_operand; i++)
    if (softpipe_info->dest_mve_info[i] != NULL)
      {
	L_free (Lpipe_MVEInfo_pool, softpipe_info->dest_mve_info[i]);
	softpipe_info->dest_mve_info[i] = NULL;
      }

  List_start (softpipe_info->isrc_mve_info);
  while ((mve_info = List_next (softpipe_info->isrc_mve_info)))
    L_free (Lpipe_MVEInfo_pool, mve_info);
  List_reset (softpipe_info->isrc_mve_info);
  softpipe_info->isrc_mve_info = NULL;
  return;
}


void
Lpipe_free_mve_info ()
{
  L_Oper *first_op;
  L_Oper *last_op;
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;

  /* All copies of the kernel, and the prologue and epilogue point to
     the same Lpipe_MVEInfo, Lpipe_MVEInfo and  Lpipe_LRInfo structures 
     and arrays of pointers to them.  So, only need to free them for one 
     copy of the kernel */

  if (Lpipe_schema == REM_LOOP || Lpipe_schema == MULTI_EPI)
    {
      first_op = kernel_copy_first_op[0];
      last_op = kernel_copy_last_op[0];
    }
  else
    {
      first_op = header_cb->first_op;
      last_op = header_cb->last_op;
    }

  /* free Lpipe_MVEInfo, Lpipe_MVEInfo, and MVE-related arrays
     pointed to by Softpipe_Op_Info */
  for (oper = first_op; oper != NULL; oper = oper->next_op)
    {
      if (!Lpipe_ignore_kernel_inst (oper))
	{
	  /* will break out after last_op */

	  if ((softpipe_info = SOFTPIPE_OP_INFO (oper)))
	    {
	      Lpipe_delete_all_mve_info (oper);

	      L_free (Lpipe_src_mve_pool, softpipe_info->src_mve_info);
	      softpipe_info->src_mve_info = 0;
	      L_free (Lpipe_pred_mve_pool, softpipe_info->pred_mve_info);
	      softpipe_info->pred_mve_info = 0;
	      L_free (Lpipe_dest_mve_pool, softpipe_info->dest_mve_info);
	      softpipe_info->dest_mve_info = 0;
	    }
	  if (oper == last_op)
	    break;
	}
    }

  /* free Lpipe_LRInfo structures */
  Lpipe_mve_lr_list = Lpipe_delete_all_live_range_info (Lpipe_mve_lr_list);

  return;
}


/*************************************************************************
                Associate Virtual Registers with Lifetimes
*************************************************************************/

/* 0 for Empty array, 1 for cannot share with any lr, 2 can share with prev lr */
static int
Lpipe_get_best_lr (List * op_array, Lpipe_LRInfo * prev_lr_info,
		   Lpipe_LRInfo ** lr_info, int ii)
{
  int index;
  int end = (8 * ii);
  Lpipe_LRInfo *lr = NULL;

  if (prev_lr_info != NULL)
    index = (8 * (prev_lr_info->last_access_time % ii)) +
      prev_lr_info->last_access_slot + 1;
  else
    index = end;

  for (; index < end; index++)
    {
      if (List_size (op_array[index]) == 0)
	continue;

      List_start (op_array[index]);
      while ((lr = List_next (op_array[index])))
	{
	  if (lr->rinfo->operand->ctype == prev_lr_info->rinfo->operand->ctype)
	    {
	      op_array[index] = List_remove (op_array[index], (void *) lr);
	      *lr_info = lr;
	      return 2;
	    }
	}
    }

  /* None found. Select the first available. */

  for (index = 0; index < end; index++)
    {
      if (List_size (op_array[index]))
	{
	  List_start (op_array[index]);
	  lr = List_next (op_array[index]);
	  op_array[index] = List_remove (op_array[index], (void *) lr);
	  *lr_info = lr;
	  return 1;
	}
    }

  /* All lrs used. */

  *lr_info = NULL;
  return 0;
}


/* return 1 on success; -1 on failure */
static int
Lpipe_associate_rot_regs (List lr_list, SM_Cb *sm_cb, int count_only)
{
  /*
   * ROTATING REGISTERS
   * ------------------------------------------------------------
   */

  L_Func *fn = sm_cb->lcode_fn;
  L_Cb *cb = sm_cb->lcode_cb;
  int ii = sm_cb->II;
  int num_stages = sm_cb->stages;
  SM_Reg_Info *rinfo;
  Lpipe_LRInfo *lr_info;
  L_Attr *rr_attr;
  int int_reg_base, flt_reg_base, dbl_reg_base, pred_reg_base;
  int int_reg_num, flt_reg_num, dbl_reg_num, pred_reg_num;
  int int_reg_next = 0, flt_reg_next = 0, dbl_reg_next = 0, pred_reg_next = 0;
  int def_stage, use_stage, i, num_names = 0, available = 1;

  List *op_array = Lcode_calloc (ii * 8, sizeof (List));

  if (Lpipe_print_mve_summary)
    fprintf (stderr, "\nRotating register summary\n"
	     "Register        PTYPE   Lifetime "
	     "   use_after_def_slot    Number of Names\n");

  /* calculate the number of names needed for each variant */

  List_start (lr_list);
  while ((lr_info = (Lpipe_LRInfo *) List_next (lr_list)))
    {
      int def_time, use_time;

      rinfo = lr_info->rinfo;

      /* Find the earliest write, which may be a live in value. */

      if (lr_info->live_in &&
	  (lr_info->live_in_def_time < lr_info->first_def_time))
	def_time = lr_info->live_in_def_time;
      else
	def_time = lr_info->first_def_time;

      use_time = lr_info->last_access_time;

      def_stage = def_time / ii;
      use_stage = use_time / ii;

      num_names = use_stage - def_stage + 1;

      /* registers that are defined once and never used in loop
         have lifetime and num_names equal to 0 */
      if (lr_info->lifetime == 0)
	num_names = 1;

      if (num_names <= 0)
	L_punt ("Lpipe_alloc_rot_regs: "
		"num_names is not greater than zero");

      /* Special case: if the lifetime is less than or equal to the
         II, then we do not have to allocate a rotating register; it
         can be allocated to a standard register. */

      /* Don't do this for predicates, since on TAHOE there are many
	 more rotating predicate registers than stationary ones.  This
	 is a hack --- the register allocator should eventually deal
	 correctly with this, and allocate single-stage predicates to
	 available rotating registers.  */

      if (lr_info->lifetime <= ii && !lr_info->use_after_def_slot &&
	  lr_info->rinfo->operand->ctype != L_CTYPE_PREDICATE)
	num_names = 0;

      if ((rinfo->type & SM_MACRO_TYPE) && (num_names > 1))
	L_punt ("Lpipe_alloc_rot_regs: " "cannot rename macro registers\n");

      lr_info->num_names = num_names;

      /* Insert live ranges into an array based on start time and slot number.
         When a live range ends, this array will be consulted to see if
         the ending register can be used to start a new live range. */

      if (num_names > 0)
	{
	  int index;

	  /* Find the earliest write, which may be a live in value. */
	  if (lr_info->live_in &&
	      lr_info->live_in_def_time <= lr_info->first_def_time)
	    index = 0;
	  else
	    index =
	      (8 * (lr_info->first_def_time % ii)) + lr_info->first_def_slot;

	  if (index < 0 || index >= (ii * 8))
	    L_punt ("Lpipe_associate_rot_regs: invalid op_array index (%d).",
		    index);

	  op_array[index] =
	    List_insert_last (op_array[index], (void *) lr_info);
	}

      if (Lpipe_print_mve_summary)
	{
	  if (L_is_ctype_predicate_direct (lr_info->rinfo->operand->ctype))
	    fprintf (stderr,
		     "%6d      %6d      %6d      %11d           %9d\n",
		     lr_info->rinfo->id, lr_info->rinfo->operand->ptype,
		     lr_info->lifetime, lr_info->use_after_def_slot,
		     lr_info->num_names);
	  else
	    fprintf (stderr,
		     "%6d      %6d      %6d      %11d           %9d\n",
		     lr_info->rinfo->id, L_PTYPE_NULL,
		     lr_info->lifetime, lr_info->use_after_def_slot,
		     lr_info->num_names);
	}
    }

#if 0
  /* Dump the array of live range start points. */
  for (i = 0; i < (ii * 8); i++)
    {
      if (op_array[i] != NULL)
	{
	  Lpipe_LRInfo *l;

	  List_start (op_array[i]);
	  while ((l = List_next (op_array[i])))
	    {
	      printf ("LR: id %d, index %d\n", l->rinfo->id, i);
	    }
	}
    }
#endif

  R_get_rot_regs (L_fn, &int_reg_base, &int_reg_num,
		  &flt_reg_base, &flt_reg_num,
		  &dbl_reg_base, &dbl_reg_num, &pred_reg_base, &pred_reg_num);

  int_reg_next = int_reg_base;
  flt_reg_next = flt_reg_base;
  dbl_reg_next = dbl_reg_base;
  pred_reg_next = pred_reg_base;

  /* Predicates for the stages of the kernel need predicates. 
     Use the assumption that the first stage_num predicates are the
     predicates for the stages.  */

  if (Lpipe_schema == KERNEL_ONLY)
    pred_reg_next += num_stages;

  if (count_only)
    {
      List_start (lr_list);
      while ((lr_info = (Lpipe_LRInfo *) List_next (lr_list)))
	{
	  if (L_is_ctype_integer (lr_info->rinfo->operand))
	    int_reg_next += lr_info->num_names;
	  else if (L_is_ctype_flt (lr_info->rinfo->operand))
	    L_punt ("Shouldn't encounter a float. MCM");
	  else if (L_is_ctype_dbl (lr_info->rinfo->operand))
	    dbl_reg_next += lr_info->num_names;
	  else if (L_is_ctype_predicate (lr_info->rinfo->operand))
	    pred_reg_next += lr_info->num_names;
	  else
	    L_punt ("Lpipe_alloc_rot_regs: invalid ctype");
	}

      /* Here we need to perform a check to see if the number
         of requested registers in each category is available. */

      if (M_arch == M_TAHOE)
	{
	  if (int_reg_next >= (int_reg_base + 96))
	    available = -1;
	}
      else
	{
	  if (int_reg_next >=
	      (int_reg_base +
	       R_get_rot_reg_max_alloc (M_native_int_register_ctype ())))
	    available = -1;
	}

      if (dbl_reg_next >=
	  (dbl_reg_base + R_get_rot_reg_max_alloc (L_CTYPE_DOUBLE)))
	available = -1;

      if (pred_reg_next >=
	  (pred_reg_base + R_get_rot_reg_max_alloc (L_CTYPE_PREDICATE)))
	available = -1;
    }
  else				/* !count_only */
    {
      int val;
      Lpipe_LRInfo *prev_lr_info = NULL;

      /* For each live range, assign it a set of registers.  The
         registers must be contiguous...a requirement that will make
         reading the code easier, but most of all, make register
         allocation later easier. */

      while ((val = Lpipe_get_best_lr (op_array, prev_lr_info, &lr_info, ii)))
	{
	  if (val == 2)
	    {
#if 0
	      printf ("Lpipe_associate_rot_regs: "
		      "Sharing one rr from prev_lr_info %d with lr_info %d.\n",
		      prev_lr_info->rinfo->id, lr_info->rinfo->id);
#endif
	    }
	  else if (val == 1)
	    {
#if 0
	      printf ("Lpipe_associate_rot_regs: "
		      "No sharing available for lr_info %d.\n",
		      lr_info->rinfo->id);
#endif
	    }
	  else
	    {
	      L_punt ("MVE: How did we get here?");
	    }

	  prev_lr_info = lr_info;

	  num_names = ++lr_info->num_names;
	  lr_info->num_names_w_pro = num_stages + 2;
	  lr_info->names =
	    (L_Operand **) calloc (num_stages + 2, sizeof (L_Operand *));
	  if (!lr_info->names)
	    L_punt ("Lpipe_alloc_rot_regs: malloc out of space\n");

	  /* First name is the original virtual register name.  This
	     name will be used for the definitions in an imaginary
	     iteration just before the first iteration of the loop.
	     This corresponds to the name used outside the loop. */

	  lr_info->names[0] = L_copy_operand (lr_info->rinfo->operand);

	  for (i = 1; i < num_names; i++)
	    {
	      int rnum = 0;

	      if (val == 2)
		{
		  /* Recycling */
		  if (L_is_ctype_integer (lr_info->rinfo->operand))
		    rnum = (int_reg_next - 1);
		  else if (L_is_ctype_flt (lr_info->rinfo->operand))
		    rnum = (flt_reg_next - 1);
		  else if (L_is_ctype_dbl (lr_info->rinfo->operand))
		    rnum = (dbl_reg_next - 1);
		  else if (L_is_ctype_predicate (lr_info->rinfo->operand))
		    rnum = (pred_reg_next - 1);
		  else
		    L_punt ("Lpipe_mve_analyze: invalid ctype");
		}
	      else
		{
		  /* Allocating new */
		  if (L_is_ctype_integer (lr_info->rinfo->operand))
		    rnum = int_reg_next++;
		  else if (L_is_ctype_flt (lr_info->rinfo->operand))
		    rnum = flt_reg_next++;
		  else if (L_is_ctype_dbl (lr_info->rinfo->operand))
		    rnum = dbl_reg_next++;
		  else if (L_is_ctype_predicate (lr_info->rinfo->operand))
		    rnum = pred_reg_next++;
		  else
		    L_punt ("Lpipe_mve_analyze: invalid ctype");
		}

	      val = 0;

	      lr_info->names[i] =
		L_new_register_operand (rnum,
					lr_info->rinfo->operand->ctype,
					lr_info->rinfo->operand->ptype);
	    }
	}

      /* Annotate the cb with the number of each type of register used. */
      rr_attr = L_new_attr ("rr", 4);
      L_set_int_attr_field (rr_attr, 0, int_reg_next - int_reg_base);
      L_set_int_attr_field (rr_attr, 1, flt_reg_next - flt_reg_base);
      L_set_int_attr_field (rr_attr, 2, dbl_reg_next - dbl_reg_base);
      L_set_int_attr_field (rr_attr, 3, pred_reg_next - pred_reg_base);
      cb->attr = L_concat_attr (cb->attr, rr_attr);

      /* Update the function attributes.  Record the maximum
         number of each type used. */
      
      if ((rr_attr = L_find_attr (fn->attr, "rr")))
	{
	  if (int_reg_base != rr_attr->field[0]->value.i)
	    L_punt ("Lpipe_mve_analyze: Incompatible int base rot regs.");
	  if (flt_reg_base != rr_attr->field[2]->value.i)
	    L_punt ("Lpipe_mve_analyze: Incompatible flt base rot regs.");
	  if (dbl_reg_base != rr_attr->field[4]->value.i)
	    L_punt ("Lpipe_mve_analyze: Incompatible dbl base rot regs.");
	  if (pred_reg_base != rr_attr->field[6]->value.i)
	    L_punt ("Lpipe_mve_analyze: Incompatible pred base rot regs.");
	  if (int_reg_next - int_reg_base > rr_attr->field[1]->value.i)
	    L_set_int_attr_field (rr_attr, 1, int_reg_next - int_reg_base);
	  if (flt_reg_next - flt_reg_base > rr_attr->field[3]->value.i)
	    L_set_int_attr_field (rr_attr, 3, flt_reg_next - flt_reg_base);
	  if (dbl_reg_next - dbl_reg_base > rr_attr->field[5]->value.i)
	    L_set_int_attr_field (rr_attr, 5, dbl_reg_next - dbl_reg_base);
	  if (pred_reg_next - pred_reg_base > rr_attr->field[7]->value.i)
	    L_set_int_attr_field (rr_attr, 7, pred_reg_next - pred_reg_base);
	}
      else
	{
	  L_punt ("Unable to find rotating_registers attribute.");
	}
    }

  /* Destroy the array containing live range start information. */
  for (i = 0; i < (ii * 8); i++)
    op_array[i] = List_reset (op_array[i]);

  Lcode_free (op_array);

  return available;
}


static int
Lpipe_associate_mve_regs (List lr_list, SM_Cb *sm_cb)
{
  /*
   * MODULO VARIABLE EXPANSION
   * ------------------------------------------------------------
   */
  L_Func *fn = sm_cb->lcode_fn;
  int ii = sm_cb->II;
  SM_Reg_Info *rinfo;
  Lpipe_LRInfo *lr_info;
  int i, lifetime, num_names, unroll = 0;

  if (Lpipe_print_mve_summary)
    {
      fprintf (stderr, "\nMVE register summary\n");
      fprintf (stderr, "Register        PTYPE   Lifetime "
	       "   use_after_def_slot    Number of Names\n");
    }

  /* calculate the required number of names for each variant
   * and the loop unrolls required for traditional mve renaming
   */

  List_start (lr_list);
  while ((lr_info = (Lpipe_LRInfo *) List_next (lr_list)))
    {
      rinfo = lr_info->rinfo;

      /* number of names is the ceiling of lifetime/II */

      lifetime = lr_info->lifetime;
      num_names = lifetime / ii;

      /* Registers defined once and never used in loop have lifetime
	  and num_names equal to 0.  If definition and last use are
	  scheduled in same cycle (mod ii) and lifetime != 0 and the
	  use is in a later slot than the def, need an extra name. */

      if ((lifetime % ii) || (lifetime == 0) || lr_info->use_after_def_slot)
	num_names++;

      /* check to make sure macro registers don't need to be renamed */
      if (!(rinfo->type & SM_REGISTER_TYPE))
	{
	  if (num_names > 1)
	    L_punt ("Lpipe_mve_analyze: " "cannot rename macro registers\n");
	  else
	    continue;
	}

      lr_info->num_names = num_names;

      if (num_names > unroll)
	unroll = num_names;

      if (Lpipe_print_mve_summary)
	fprintf (stderr, "%6d      %6d      %6d      %11d           %9d\n",
		 lr_info->rinfo->id,
		 L_is_ctype_predicate_direct(lr_info->rinfo->operand->ctype) ? 
		 lr_info->rinfo->operand->ptype : L_PTYPE_NULL,
		 lr_info->lifetime, lr_info->use_after_def_slot,
		 lr_info->num_names);
    }

  /* Make the number of names for each variant a multiple of unroll and
     allocate operands to be used in renaming the variant. */
  List_start (lr_list);
  while ((lr_info = (Lpipe_LRInfo *) List_next (lr_list)))
    {
      rinfo = lr_info->rinfo;

      if (!(rinfo->type & SM_REGISTER_TYPE))
	continue;

      num_names = lr_info->num_names;
      while ((unroll % num_names) != 0)
	num_names++;
      lr_info->num_names = num_names;
      if (!(lr_info->names =
	    (L_Operand **) calloc (num_names, sizeof (L_Operand *))))
	L_punt ("Lpipe_mve_analyze: malloc out of space\n");

      /* First name is the original virtual register name.  This
       * name will be used for the definitions in an imaginary
       * iteration just before the first iteration of the loop.
       * This corresponds to the name used outside the loop. 
       */

      lr_info->names[0] = L_copy_operand (lr_info->rinfo->operand);

      for (i = 1; i < num_names; i++)
	lr_info->names[i] = 
	  L_new_register_operand (++fn->max_reg_id,
				  lr_info->rinfo->operand->ctype,
				  lr_info->rinfo->operand->ptype);
    }

  return unroll;
}


static List
Lpipe_construct_lr (SM_Cb *kernel_cb)
{
  L_Func *fn = kernel_cb->lcode_fn;
  int ii = kernel_cb->II;
  List lr_list = NULL;
  Lpipe_LRInfo *lr_info;
  Lpipe_MVEInfo *mve_info, **mve_ptr;
  SM_Oper *src_oper, *sink_oper;
  SM_Reg_Action *dest_operand;
  SM_Reg_Info *dest_operand_rinfo, *rinfo;
  SM_Dep *dep;
  Softpipe_Op_Info *src_softpipe_info, *sink_softpipe_info;
  int src_issue_time, sink_issue_time, issue_time, use_lifetime;
  int use_mod_ii, def_mod_ii;	/* definition and use time modulo II */

  for (rinfo = kernel_cb->first_rinfo; rinfo; rinfo = rinfo->next_rinfo)
    {
      if (!(rinfo->type & LPIPE_LR_TYPE))
        continue;

      if (!rinfo->first_def)
	continue;

      lr_info = Lpipe_create_live_range_info (rinfo);
      lr_list = List_insert_last (lr_list, lr_info);
    }

  /* Flow dependences represent the definitions of all loop
   * variants.  Output dependences indicate that multiple
   * definitions of the variant exist in the loop body.  For each
   * operation, two passes are made through the dependence info, the
   * first looking for flow dependences, the second for output
   * dependences.  Also build a list of branches for use in the
   * second pass.  
   */

  /* These routines create a list of live range functions and create
     mve_info structures for operands.  The arrays that hold the
     mve_info structures are created prior to this function. */

  for (src_oper = kernel_cb->first_serial_op; src_oper;
       src_oper = src_oper->next_serial_op)
    {
      int dest_number;		/* src_oper->dest[dest_number] 
				   is source of dep */
      int src_number = 0;	/* sink_oper->src[src_number] 
				   is sink of dep    */

      int def_cycle_diff = -1;

      src_softpipe_info = SOFTPIPE_OP_INFO (src_oper->lcode_op);

      if (L_START_STOP_NODE (src_oper->lcode_op))
	continue;

      /* First, go through all the destinations and set up Lpipe_LRInfo
         structures if needed. */
      for (dest_number = 0; dest_number < L_max_dest_operand; dest_number++)
	{
	  if (!(dest_operand = src_oper->dest[dest_number]))
	    continue;

	  dest_operand_rinfo = dest_operand->rinfo;

	  if (!(dest_operand_rinfo->type & LPIPE_LR_TYPE))
	    continue;

	  /* Check list of live ranges to see if live range
	     structure already exists for this variant. */

	  lr_info = Lpipe_find_live_range (lr_list, dest_operand_rinfo);

	  assert (lr_info);

	  src_softpipe_info->dest_mve_info[dest_number] = mve_info =
	    Lpipe_new_opd_mve_info (lr_info);

	  if (!lr_info->def_sm_oper)
	    {
	      /* Need to create live range structure. This is the reference
	         definition. */
	      lr_info->def_sm_oper = src_oper;
	      lr_info->first_def_time = src_softpipe_info->issue_time;
	      lr_info->first_def_slot = src_softpipe_info->issue_slot;
	      lr_info->last_access_time = src_softpipe_info->issue_time;
	      lr_info->last_access_slot = src_softpipe_info->issue_slot;
	    }
	  else
	    {
	      /* Determine relationship between this definition and
	       * the reference definition. If no cross-iteration
	       * output dependence exists between the two
	       * definitions, then the relationship is determined
	       * simply by the issue times. If there is a cross-
	       * iteration output dependence, then that must be
	       * taken into account.  
	       */

	      int output_dep_found = 0;

	      /* first check output dependences from reference oper
	         to oper */
	      /* MCM Check the in dependences to this destination
	         register to see if they come from the definition
	         instruction. */

	      for (dep = dest_operand->first_dep_in; dep;
		   dep = dep->next_dep_in)
		{
		  if ((dep->flags & (SM_REG_DEP | SM_OUTPUT_DEP)) !=
		      (SM_REG_DEP | SM_OUTPUT_DEP))
		    continue;
		  if (dep->ignore)
		    continue;
		  if (dep->omega == 0)
		    continue;
		  if (dep->from_action->sm_op != lr_info->def_sm_oper)
		    continue;

		  /* Ensure that the flow is from a dest to a dest,
		     not a control dep. */
		  if (dep->from_action->operand_type != MDES_DEST ||
		      dep->to_action->operand_type != MDES_DEST)
		    continue;

		  output_dep_found = 1;

		  def_cycle_diff = src_softpipe_info->issue_time -
		    lr_info->first_def_time + ii * dep->omega;

		  issue_time = src_softpipe_info->issue_time + ii * dep->omega;

		  Lpipe_update_lr_last_acc (lr_info, issue_time,
					    src_softpipe_info->issue_slot, ii);
		  break;
		}

	      if (!output_dep_found)
		{
		  /* If no cross-iteration output dependence found
		     so far, check output dependences from oper to
		     reference oper. */

		  for (dep = dest_operand->first_dep_out; dep;
		       dep = dep->next_dep_out)
		    {
		      if ((dep->flags & (SM_REG_DEP | SM_OUTPUT_DEP)) !=
			  (SM_REG_DEP | SM_OUTPUT_DEP))
			continue;
		      if (dep->ignore)
			continue;
		      if (dep->omega == 0)
			continue;
		      if (dep->to_action->sm_op != lr_info->def_sm_oper)
			continue;

		      /* Ensure that the flow is from a dest to a
		         dest, not a control dep. */
		      if (dep->from_action->operand_type != MDES_DEST ||
			  dep->to_action->operand_type != MDES_DEST)
			continue;

		      output_dep_found = 1;

		      /* Find number of cycles from reference definition 
		         to this definition. This number will be 0 or
		         negative because output dependence goes from
		         this definition to reference definition. */

		      def_cycle_diff = src_softpipe_info->issue_time -
			lr_info->first_def_time - ii * dep->omega;

		      issue_time = src_softpipe_info->issue_time - 
			ii * dep->omega;

		      if (issue_time < lr_info->first_def_time ||
			  (issue_time == lr_info->first_def_time && 
			   src_softpipe_info->issue_slot < lr_info->first_def_slot))
			L_punt ("Definition issue time assumptions violated.");
		      break;
		    }

		  if (!output_dep_found)
		    {
		      /* The two definitions are from the same
		         iteration.  Find number of cycles from this
		         definition to reference definition in this
		         iteration.  This number may be positive or
		         negative depending on order of 2
		         definitions in same iteration. */
		      def_cycle_diff = src_softpipe_info->issue_time -
			lr_info->first_def_time;

		      Lpipe_update_lr_last_acc (lr_info,
						src_softpipe_info->
						issue_time,
						src_softpipe_info->
						issue_slot, ii);
		    }
		}

	      /* record destination-operand-related information */

	      /* Lifetime from the reference definition to this
	         definition of the variant.  May be negative. */
	      mve_info->lifetime = def_cycle_diff;

	      def_mod_ii = src_softpipe_info->issue_time % ii;

	      if (def_cycle_diff >= 0)
		mve_info->stage_lifetime_incr =
		  (def_mod_ii < (lr_info->first_def_time % ii));
	      else
		mve_info->stage_lifetime_incr =
		  (def_mod_ii > (lr_info->first_def_time % ii));
	    }
	}

      /* Then check the flow dependences */
      for (dest_number = 0; dest_number < L_max_dest_operand; dest_number++)
	{
	  int explicit_sink;

	  if (!(dest_operand = src_oper->dest[dest_number]))
	    continue;

	  dest_operand_rinfo = dest_operand->rinfo;

	  if (!(dest_operand_rinfo->type & LPIPE_LR_TYPE))
	    continue;

	  for (dep = dest_operand->first_dep_out; dep; dep = dep->next_dep_out)
	    {
	      if ((dep->flags & (SM_REG_DEP | SM_FLOW_DEP)) !=
		  (SM_REG_DEP | SM_FLOW_DEP))
		continue;

	      if (dep->from_action->operand_type != MDES_DEST)
		continue;

	      if (dep->to_action->operand_type != MDES_PRED &&
		  dep->to_action->operand_type != MDES_SRC &&
		  dep->to_action->operand_type != MDES_SYNC_IN)
		continue;

	      explicit_sink = dep->to_action->operand_type != MDES_SYNC_IN;

	      /* find destination and source operands involved in
	         dependence */
	      sink_oper = dep->to_action->sm_op;
	      sink_softpipe_info = SOFTPIPE_OP_INFO (sink_oper->lcode_op);

	      if (!explicit_sink)
		{
		  L_Operand *dep_operand;
		  dep_operand = dep->from_action->rinfo->operand;

		  if (sink_softpipe_info->loop_back_br)
		    {
		      L_Oper *removed_oper = NULL;
		      int c = 0;

		      if (sink_oper->next_serial_op &&
			  L_START_STOP_NODE (sink_oper->next_serial_op->
					     lcode_op))
			{
			  removed_oper = sink_oper->next_serial_op->lcode_op;
			  L_remove_oper (kernel_cb->lcode_cb, removed_oper);
			}

		      if (!L_in_oper_OUT_set (kernel_cb->lcode_cb,
					      sink_oper->lcode_op,
					      dep_operand, FALL_THRU_PATH))
			c = 1;

		      if (removed_oper)
			{
			  L_insert_oper_after (kernel_cb->lcode_cb,
					       sink_oper->lcode_op,
					       removed_oper);
			}

		      if (c)
			continue;
		    }
		  else
		    {
		      if (!L_in_oper_OUT_set (kernel_cb->lcode_cb,
					      sink_oper->lcode_op,
					      dep_operand, TAKEN_PATH))
			continue;
		    }
		}

	      dest_number = dep->from_action->operand_number;

	      if (!(lr_info = Lpipe_find_live_range (lr_list,
						     dest_operand_rinfo)))
		L_punt ("Lpipe_mve_analyze: Could not find live range"
			" structure for oper %d, "
			"destination number %d\n",
			src_oper->lcode_op->id, dest_number);

	      mve_info = src_softpipe_info->dest_mve_info[dest_number];

	      src_issue_time = src_softpipe_info->issue_time;
	      sink_issue_time = sink_softpipe_info->issue_time;

	      /* Compute lifetime from reference def to this use */

	      use_lifetime = sink_issue_time - src_issue_time +
		mve_info->lifetime + ii * dep->omega;

	      /* If using the rotating registers, and this is live
	         out of the loopback branch, then increase then set
	         the last use as the first instruction after the
	         loopback branch, which will eventually model the
	         additional rotation that occurs when the loopback
	         fails. MCM Currently, there is no extra rotation on
	         side exit branches. */

	      if (!explicit_sink && sink_softpipe_info->loop_back_br &&
		  (Lpipe_schema == MULTI_EPI_ROT_REG || 
		   Lpipe_schema == KERNEL_ONLY))
		use_lifetime++;

	      issue_time = lr_info->first_def_time + use_lifetime;

	      Lpipe_update_lr_last_acc (lr_info, issue_time,
					sink_softpipe_info->issue_slot, ii);

	      /* Check for live in.  Exclude registers that are not
	       * live into the cb. 
	       */
	      if (L_in_cb_IN_set (kernel_cb->lcode_cb,
				  dep->from_action->rinfo->operand))
		{
		  int found = 0, temp_live_in_def_time, num_uses;
		  Set uses;

		  /* Look at the reaching defs of this op in the original
		     code to see if any defs reach into the loop. 
		     MCM There has got to be a better way to go this!!! */

		  uses = L_get_oper_RIN_defining_opers (sink_oper->lcode_op,
							dep->from_action->
							rinfo->operand);
		  num_uses = Set_size (uses);
		  if (num_uses > 0)
		    {
		      int u, *buffer = 
			(int *) Lcode_malloc (sizeof (int) * num_uses);
		      L_Cb *cb;
		      Set_2array (uses, buffer);
                      uses = Set_dispose (uses);
		      for (u = 0; u < num_uses; u++)
			{
			  cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl,
							buffer[u]);
			  if (cb->id != kernel_cb->lcode_cb->id)
			    found = 1;
			}
		      Lcode_free (buffer);
		    }

		  /* If only defs from within the kernel cb are found,
		     then register is not live in. */
		  if (found)
		    {
		      if (Lpipe_debug >= 2)
			{
			  printf ("Live in oper %d on src %d\n",
				  src_oper->lcode_op->id, src_number);
			  fflush (stdout);
			}

		      lr_info->live_in = 1;

		      /* This use is mve_info->live_range->lifetime
		         from the def in real cycles.  However, this
		         use may also be a live in variable defined at
		         cycle 0 relative to the use. Cycle 0 is
		         (stage*II + cycle_within_stage) from this
		         use, which is stored as
		         src_softpipe_info->issue_time. Therefore,
		         relating that back to the reference def, the
		         live_in is
		         src_softpipe_info->issue_time-use_lifetime
		         cycles before the ref def. */

		      temp_live_in_def_time = lr_info->first_def_time
			+ use_lifetime - sink_issue_time;

		      if (temp_live_in_def_time != lr_info->live_in_def_time
			  && lr_info->live_in_def_time != -1)
			L_warn ("Lpipe_mve_analyze: "
				"Refusing to change: " 
				"original live_in_def_time %d, "
				"new live_in_def_time %d.\n",
				lr_info->live_in_def_time,
				temp_live_in_def_time);
		      else
			lr_info->live_in_def_time = temp_live_in_def_time;
		    }
		}

	      if (explicit_sink)
		{
		  src_number = dep->to_action->operand_number;

		  /* record source-operand-related information for data or
		     predicate register */
		  if (dep->to_action->flags &
		      (SM_PRED_UNCOND_USE | SM_PRED_COND_USE))
		    mve_ptr = &(sink_softpipe_info->pred_mve_info[src_number]);
		  else
		    mve_ptr = &(sink_softpipe_info->src_mve_info[src_number]);

		  if (*mve_ptr)
		    continue;

		  *mve_ptr = mve_info = Lpipe_new_opd_mve_info (lr_info);
		}
	      else
		{
		  lr_info->live_out = 1;

		  List_start (sink_softpipe_info->isrc_mve_info);
		  while ((mve_info = 
			  List_next (sink_softpipe_info->isrc_mve_info)))
		    if (mve_info->live_range == lr_info)
		      break;

		  if (mve_info)
		    continue;

		  mve_info = Lpipe_new_opd_mve_info (lr_info);
		  sink_softpipe_info->isrc_mve_info =
		    List_insert_last (sink_softpipe_info->isrc_mve_info,
				      mve_info);
		}

	      use_mod_ii = sink_softpipe_info->issue_time % ii;
	      def_mod_ii = lr_info->first_def_time % ii;

	      mve_info->lifetime = use_lifetime;

	      mve_info->stage_lifetime_incr = (use_lifetime >= 0) ?
		(use_mod_ii < def_mod_ii) : (use_mod_ii > def_mod_ii);
	    }
	}
    }
  return lr_list;
}


/*************************************************************************
                MVE Analysis Function Definitions
*************************************************************************/

/* Note that this analysis appears to be similar to the live range
   analysis done by the register allocator.  However, it is more than
   that.  Traditional live range analysis simply computes the set of
   instructions over which a variable is live.  This routine computes
   the length of each live range from the first definition to each use
   and and live out.  An understanding of the output dependences and
   cross-iteration dependences is necessary to compute these lengths.
   Therefore this routine analyzes the flow and output dependences to
   compute the lifetime lengths rather than live range sets. */
/* Analyze the loop variants to determine the amount of unrolling
   required for modulo variable expansion.  Build the data structures
   needed to rename loop variants after unrolling.  Returns the 
   required amount for unrolling. In count_only mode, a 
   1 is returned indicating the number of reqested rotating registers
   is available, while a -1 indicates they are unavailable.  */
int
Lpipe_analyze_lr (SM_Cb *kernel_cb, int count_only)
{
  int unroll, ii = kernel_cb->II;
  List lr_list;

  /*
   * PHASE I
   * ----------------------------------------------------------------------
   * DEPENDENCE ANALYSIS
   */

  lr_list = Lpipe_construct_lr (kernel_cb);

  /*
   * PHASE II
   * ----------------------------------------------------------------------
   * SETUP FOR MVE OR ROTATING REGISTER GENERATION
   */

  if (Lpipe_schema == REM_LOOP || Lpipe_schema == MULTI_EPI)
    unroll = Lpipe_associate_mve_regs (lr_list, kernel_cb);
  else
    unroll = Lpipe_associate_rot_regs (lr_list, kernel_cb, count_only);

  if (count_only || (unroll == -1))
    {
      L_Oper *op;

      /* free mve info structures */
      for (op = kernel_cb->lcode_cb->first_op; op; op = op->next_op)
	if (!Lpipe_ignore_kernel_inst (op))
	  Lpipe_delete_all_mve_info (op);

      /* free Lpipe_LRInfo structures */
      lr_list = Lpipe_delete_all_live_range_info (lr_list);
    }
  else
    {
      if (Lpipe_mve_lr_list)
	L_punt ("Lpipe_mve_analyze: lr list is not empty");

      Lpipe_mve_lr_list = lr_list;

      if (Lpipe_debug >= 1)
	{
	  Lpipe_print_live_ranges (lr_list, ii);
	  Lpipe_print_mve_info (kernel_cb);
	}
    }

  return (unroll);
}


/*************************************************************************
                Rotating Register Function Definitions
*************************************************************************/

int
Lpipe_find_name (SM_Cb *sm_cb, Lpipe_LRInfo *lr_info, Lpipe_MVEInfo *mve_info)
{
  int src_stg, snk_stg, name_index;

  /* Find earliest write, which may be a live in value. */

  if (lr_info->live_in &&
      lr_info->live_in_def_time < lr_info->first_def_time)
    src_stg = lr_info->live_in_def_time / sm_cb->II;
  else
    src_stg = lr_info->first_def_time / sm_cb->II;

  snk_stg = (lr_info->first_def_time + mve_info->lifetime) / sm_cb->II;

  name_index = snk_stg - src_stg + 1;

  if (name_index < 1 || name_index >= lr_info->num_names)
    L_punt ("Lpipe_find_name: name %d is out of range.", name_index);

  return name_index;
}


static int
Lpipe_mov_opc (L_Operand *opd)
{
  int opc = 0;

  if (L_is_ctype_integer (opd))
    opc = Lop_MOV;
  else if (L_is_ctype_flt (opd))
    opc = Lop_MOV_F;
  else if (L_is_ctype_dbl (opd))
    opc = Lop_MOV_F2;
  else if (L_is_ctype_predicate (opd))
    opc = Lop_PRED_COPY;
  else
    L_punt ("Lpipe_mov_opc: Invalid operand ctype.");

  return opc;
}


/* Return the number of rotating registers required */
int
Lpipe_rreg_transform (SM_Cb * sm_cb)
{
  L_Cb *kernel_cb = sm_cb->lcode_cb;
  L_Oper *oper;

  if (!kernel_cb->first_op)
    L_punt ("Lpipe_rreg_transform: Empty kernel.  cb %d", kernel_cb->id);

  /* Using MVE, the registers are renamed for each of the iterations.
     Then, assuming that the loopback branch is in the last stage, 
     stages 1 through n are copied and used as the prologue.  Instructions
     with invalid stage identifiers for that stage of the prologue
     are eliminated.  Then, the incoming registers are copied into their
     MVE copy 1 registers. 

     Using rotating registers, there is only one copy of the kernel.
     We need MVE-like register renames for the prologue still. So,
     maybe generate the unrolled kernel, like normal to be used
     when generating the kernel.  But, the interface registers between
     the prologue and the kernel need to be generated.  
   */

  for (oper = kernel_cb->first_op; oper; oper = oper->next_op)
    {
      Softpipe_Op_Info *softpipe_info = SOFTPIPE_OP_INFO (oper);
      Lpipe_MVEInfo *mve_info;
      Lpipe_LRInfo *lr_info;
      int j, name_index;
      ITuint8 ptype;

      /* rename destination operands */
      for (j = 0; j < L_max_dest_operand; j++)
	{
	  if (!L_is_register (oper->dest[j]))
	    continue;

	  if (!(mve_info = softpipe_info->dest_mve_info[j]))
	    L_punt ("Lpipe_rreg_transform: destination (op %d) "
		    "does not have a dest_mve_info", oper->id);

	  lr_info = mve_info->live_range;

	  /* If set to 1, then no rotating regs were allocated
	     for this variable and it can be allocated to a 
	     non-rotating register. */

	  if (lr_info->num_names <= 1)
	    continue;

	  name_index = Lpipe_find_name (sm_cb, lr_info, mve_info);

	  ptype = oper->dest[j]->ptype;
	  L_delete_operand (oper->dest[j]);
	  oper->dest[j] = L_copy_operand (lr_info->names[name_index]);
	  oper->dest[j]->ptype = ptype;
	}

      /* rename source operands */
      for (j = 0; j < L_max_src_operand; j++)
	{
	  if (!L_is_register (oper->src[j]))
	    continue;

	  /* loop invariants will not have Lpipe_MVEInfo structures */

	  if (!(mve_info = softpipe_info->src_mve_info[j]))
	    continue;

	  lr_info = mve_info->live_range;

	  /* If set to 1, then no rotating regs were allocated
	     for this variable and it can be allocated to a 
	     non-rotating register. */
	  if (lr_info->num_names <= 1)
	    continue;

	  /* Find the copy of the kernel that contains the reference
	     definition for this lifetime.  If the lifetime is 
	     positive, that copy is the current copy minus the
	     number of stages that the lifetime spans.  If the
	     variant is used before defined in a single copy of the
	     kernel, go back 1 stage further.  Because some lifetimes
	     are live across the backedge of the unrolled kernel,
	     the resulting number may be negative.  stage_lifetime
	     is guaranteed to be less than unroll, so add unroll 
	     and then coupute the result modulo unroll.  */

	  if (mve_info->lifetime < 0)
	    L_punt ("Lpipe_rreg_transform (src): "
		    "not sure how we got here yet.");

	  /* (op 39 add [(r 85 i)] [(r 85 i)(i 1)]
	     <(stage (i 1))(iter (i 1))(isl (i 1998)(i 6)(i 1))>)
	     This is defined in stage 1 (not 0) and will be written
	     as r32 during that stage.  This instruction is read
	     from the previous iteration writing of it, which 
	     happened 2 cycles, which is one stage, which is the 
	     same instruction in the previous iteration. So, that 
	     value, is now in r33. r32 is in names [1] and 
	     r33 is names[2]. */

	  name_index = Lpipe_find_name (sm_cb, lr_info, mve_info);
	  
	  L_delete_operand (oper->src[j]);
	  oper->src[j] = L_copy_operand (lr_info->names[name_index]);
	}

      /* rename predicate operand */

      if (L_is_register (oper->pred[0]) &&
	  (mve_info = softpipe_info->pred_mve_info[0]) &&
	  (lr_info = mve_info->live_range) &&
	  (lr_info->num_names > 1))
	{
	  if (mve_info->lifetime < 0)
	    L_punt ("Lpipe_rreg_transform (pred): negative lifetime.");

	  name_index = Lpipe_find_name (sm_cb, lr_info, mve_info);

	  ptype = oper->pred[0]->ptype;
	  L_delete_operand (oper->pred[0]);
	  oper->pred[0] = L_copy_operand (lr_info->names[name_index]);
	  oper->pred[0]->ptype = ptype;
	}
    }

  kernel_cb->flags =
    L_SET_BIT_FLAG (kernel_cb->flags, L_CB_ROT_REG_ALLOCATED);

  return 0;
}


static void
Lpipe_gen_rot_pro_fixup (L_Func *fn, L_Cb *cb, Lpipe_LRInfo *lr_info,
			 int reg)
{
  L_Oper *new_oper;
  int opc;

  if (lr_info->num_names <= 1)
    return;

  /* if live range has been renamed, insert an instruction to move
     the value to the register expected outside the loop */

  opc = Lpipe_mov_opc (lr_info->rinfo->operand);
  new_oper = L_create_new_op (opc);

  /* From the prologue, the live in name can be matched.  reg
     in the live in context is the register number.  The original
     operand is copied to get the types correct. */
  new_oper->dest[0] = L_copy_operand (lr_info->names[0]);
  new_oper->dest[0]->value.r = reg;
  
  if (opc == Lop_PRED_COPY && M_arch == M_TAHOE)
    {
      new_oper->pred[0] = L_copy_operand (lr_info->names[0]);
      L_assign_ptype_null (new_oper->pred[0]);
    }
  else
    {
      new_oper->src[0] = L_copy_operand (lr_info->names[0]);
      L_assign_ptype_null (new_oper->src[0]);
    }

  L_assign_ptype_null (new_oper->dest[0]);

  L_insert_oper_before (cb, cb->first_op, new_oper);
  L_annotate_oper (fn, cb, new_oper);
  L_delete_oper (cb, new_oper);

  return;
}


/* Find variants that are live into the original loop body, and insert
   a move to copy the value defined before the loop to the name used
   inside the loop.  When the were prologue for rotating registers was
   created, the rotating register numbers for reads and writes are
   incremented by one for each stage of unroll to simulate the effects
   of a rotation at the end of each stage. So, the live in values are
   from an imaginary stage -1 (if stage 0 is the first of the
   prologue.  */

void
Lpipe_fix_live_in_rot (SM_Cb *sm_cb)
{
  L_Func *fn = sm_cb->lcode_fn;
  L_Cb *kernel_cb, *prologue_cb;
  L_Oper *oper;
  SM_Oper *sm_op;
  Softpipe_Op_Info *softpipe_info;
  Lpipe_MVEInfo *mve_info;
  Lpipe_LRInfo *lr_info;
  int stage_count = sm_cb->stages, reg = 0, j;

  kernel_cb = sm_cb->lcode_cb;
  prologue_cb = kernel_cb->prev_cb;

  for (sm_op = sm_cb->first_serial_op; sm_op; sm_op = sm_op->next_serial_op)
    {
      oper = sm_op->lcode_op;
      softpipe_info = SOFTPIPE_OP_INFO (oper);

      for (j = 0; j < L_max_dest_operand; j++)
	{
	  if (!L_is_register (oper->dest[j]))
	    continue;

	  /* Exclude loop invariants */
	  if (!(mve_info = softpipe_info->dest_mve_info[j]))
	    continue;

	  lr_info = mve_info->live_range;

	  if ((lr_info->num_names <= 1) ||
	      !lr_info->live_in || 
	      lr_info->prologue_mov_inserted)
	    continue;

	  /* Insert prologue move */

	  if (Lpipe_schema == MULTI_EPI_ROT_REG)
	    {
	      /* Stages start numbering at 0.  stage_count-1 is the
	       * number of prologue stages.  We need to account for
	       * the number of missing shifts in the prologue between
	       * the first read in its stage and the kernel.  Account
	       * for the number of rotations simulated by shifts. The
	       * kernel is in stage stage_count-1.  The current
	       * instruction that sees the live in is in stage
	       * softpipe_info->stage.  The difference is the number
	       * of stage crossings. Since we are looking at the def,
	       * we need to get the previous version of the register,
	       * hence the +1. 
	       */
		
	      int stg_ofst = (stage_count - 1) - softpipe_info->stage + 1;
	      reg = Lpipe_get_pro_reg (sm_cb, lr_info, mve_info, stg_ofst);
	    }
	  else if (Lpipe_schema == KERNEL_ONLY)
	    {
	      /* There are no simulated shifts by the prologue since
	       * there is no prologue!  The kernel itself acts as the
	       * prologue.  So, if read in stage 0, then write into x.
	       * If read in stage 1, then write into x-1, to account
	       * for the real shift into the register. Since we are
	       * looking at the def, we need to get the previous
	       * version of the register, hence the +1. 
	       */

	      reg = oper->dest[j]->value.r - (softpipe_info->stage - 1);
	    }
	  else
	    {
	      L_punt ("Lpipe_fix_live_in_rot: "
		      "Invalid codegen schema %d.", Lpipe_schema);
	    }

	  Lpipe_gen_rot_pro_fixup (fn, prologue_cb, lr_info, reg);

	  lr_info->prologue_mov_inserted = 1;
	}
    }

  return;
}


static void
Lpipe_gen_rot_epi_fixup (L_Func *fn, L_Cb *cb, Lpipe_LRInfo *lr_info,
			 int name_index)
{
  L_Oper *new_oper;
  int opc;

  if (lr_info->num_names <= 1)
    return;

  /* if live range has been renamed, insert an instruction to move
     the value to the register expected outside the loop */

  opc = Lpipe_mov_opc (lr_info->rinfo->operand);
  new_oper = L_create_new_op (opc);

  if (name_index >= lr_info->num_names)
    L_punt ("Lpipe_gen_rot_epi_fixup: requested name %d "
	    "unavailable (name %d highest available)",
	    name_index, (lr_info->num_names - 1));

  /* dest is the original name */

  new_oper->dest[0] = L_copy_operand (lr_info->names[0]);

  if (opc == Lop_PRED_COPY && M_arch == M_TAHOE)
    {
      new_oper->pred[0] = L_copy_operand (lr_info->names[name_index]);
      L_assign_ptype_null (new_oper->pred[0]);
    }
  else
    {
      /* The src of the move is the register number at the time of exit.
	 names[0] is the original, names[1] is the def version, names[2] is
	 after the first rotation, etc. */
      new_oper->src[0] = L_copy_operand (lr_info->names[name_index]);
      L_assign_ptype_null (new_oper->src[0]);
    }

  L_assign_ptype_null (new_oper->dest[0]);

  /* make sure move goes before jump at end of epilogue */
  if (L_uncond_branch (cb->last_op))
    L_insert_oper_before (cb, cb->last_op, new_oper);
  else
    L_insert_oper_after (cb, cb->last_op, new_oper);

  L_annotate_oper (fn, cb, new_oper);
  L_delete_oper (cb, new_oper);

  return;
}


/* The live out fix ups for the loop_back branch for all exits
   are the same.  They are always unrotated from their positions
   in the last stage of the kernel.  For 1 epilogue, the output
   registers are rotated -1, for 2 epis, -2, etc. For
   0 epis, we must insert correction code just after the kernel.
   For 1 or more, we might be able to find some of the live out
   writes in the epilogue, where we can edit, but some might be
   from earlier stages executed from within the kernel.  So
   for simplicity, just put the correction code at the head of
   post-loop code. 
   
   The live out for all copies of a side-exit are also the same.
   Place the instructions at the head of the block after any
   epilogue blocks.

   Rewritten to use implicit src MVE info fields on loop exit opers.
   This fixes a host of problems. -- JWS 20030812
*/

void
Lpipe_fix_live_out_rot (SM_Cb * sm_cb, SM_Oper * sm_loop_back_br)
{
  L_Func *fn = sm_cb->lcode_fn;
  L_Cb *kernel_cb = sm_cb->lcode_cb, *epilogue_cb;
  SM_Oper *sm_exit_op;
  Softpipe_Op_Info *exit_softpipe_info;
  Lpipe_LRInfo *lr_info;
  Lpipe_MVEInfo *mve_info;

  /* There should always be an epilogue on the exit paths. There
     should be only one per each original loop exit. Place the
     compensation code at the end of each epilogue but before any
     branch. */

  for (sm_exit_op = sm_cb->first_serial_op;
       sm_exit_op; sm_exit_op = sm_exit_op->next_serial_op)
    {
      exit_softpipe_info = SOFTPIPE_OP_INFO (sm_exit_op->lcode_op);

      if (!exit_softpipe_info->exit_cb)
	continue;

      if (sm_exit_op == sm_loop_back_br)
	{
	  epilogue_cb = kernel_cb->next_cb;
	}
      else
	{
	  L_Flow *epilogue_flow;

	  if (!(epilogue_flow = L_find_flow_for_branch (kernel_cb,
							sm_exit_op->lcode_op)))
	    L_punt ("Lpipe_fix_live_out_rot: unable to find epilogue flow");

	  epilogue_cb = epilogue_flow->dst_cb;
	}

      List_start (exit_softpipe_info->isrc_mve_info);
      while ((mve_info = List_next (exit_softpipe_info->isrc_mve_info)))
	{
	  int name_index;

	  lr_info = mve_info->live_range;

	  if (lr_info->num_names <= 1)
	    continue;

	  if (!L_in_cb_IN_set (exit_softpipe_info->exit_cb,
			       lr_info->rinfo->operand))
	    continue;

	  name_index = Lpipe_find_name (sm_cb, lr_info, mve_info);

#if 0
	  /* This is already accounted for in setting of use lifetime
	   * to life-to-loopback + 1 -- JWS */

	  /* The IA64 br.?top instructions cause a loop rotation
	   * even when the loop back fails.  So, there is an extra
	   * shift from the exit to the prologue.  The extra name
	   * was already accounted for when the last_access was
	   * computed to be the cycle after the loop iteration
	   * finished. -- MCM */
	  if (sm_exit_op == sm_loop_back_br)
	    name_index++;
#endif
	  Lpipe_gen_rot_epi_fixup (fn, epilogue_cb, lr_info, name_index);
	}
    }
  return;
}


/*************************************************************************
                MVE Function Definitions
*************************************************************************/

/* unroll the kernel and rename the operands */
void
Lpipe_mve_transform (SM_Cb * sm_cb, int unroll)
{
  int ii = sm_cb->II, stage_count = sm_cb->stages;
  L_Oper *oper, *new_oper;
  Softpipe_Op_Info *softpipe_info, *new_softpipe_info;
  int copy;			/* current copy of kernel being made */
  int j;
  Lpipe_MVEInfo *src_mve_info;
  Lpipe_MVEInfo *dest_mve_info;
  Lpipe_LRInfo *lr_info;
  int stage_lifetime;		/* lifetime of variant in stages */
  int def_copy;			/* copy of kernel in which variant is def */
  int def_stage;		/* intra-iter stage in which the variant is
				   first defined */
  int name_index;		/* index into array of names for variant */
  L_Oper *temp_last_op;		/* temporary pointer to the last operation
				   of the last completed copy of the kernel */
  L_Attr *attr;
  L_Cb *kernel_cb;
  L_Attr *isl_attr;

  /* global vars - arrays of pointers to the first and last oper of
     each copy of the kernel code in the unrolled loop.  Copies are
     numbered 0 to unroll-1 */

  if (!(kernel_copy_first_op = (L_Oper **) calloc (unroll, sizeof (L_Oper *))))
    L_punt ("Lpipe_mve_transform: malloc out of space\n");

  if (!(kernel_copy_last_op = (L_Oper **) calloc (unroll, sizeof (L_Oper *))))
    L_punt ("Lpipe_mve_transform: malloc out of space\n");

  kernel_cb = sm_cb->lcode_cb;

  if (kernel_cb->first_op == NULL)
    L_punt ("Lpipe_mve_transform: Empty kernel.  cb %d", kernel_cb->id);

  /* copy 0 is the original one */
  kernel_copy_first_op[0] = kernel_cb->first_op;
  kernel_copy_last_op[0] = kernel_cb->last_op;

  /* temp pointer to the last oper of the last completed copy of the kernel */
  temp_last_op = kernel_cb->last_op;

  /* make unroll-1 more copies of kernel and rename all copies */

  for (copy = 0; copy < unroll; copy++)
    {
      for (oper = kernel_cb->first_op; oper; oper = oper->next_op)
	{
	  if (Lpipe_ignore_kernel_inst (oper))
	    continue;

	  /* will break out of this loop after processing 
	     kernel_copy_last_op[0] */

	  softpipe_info = SOFTPIPE_OP_INFO (oper);

	  if (copy == 0)
	    {
	      /* First copy of kernel is already there.  Just set some
		 pointers so that we can pretend we just created it and
		 rename it. */
	      new_oper = oper;
	      new_softpipe_info = softpipe_info;
	    }
	  else
	    {
	      /* need to create all other copies */
	      new_oper = L_copy_operation (oper);
	      new_oper->ext =
		(Softpipe_Op_Info *) Lpipe_copy_op_info (softpipe_info);
	      new_softpipe_info = SOFTPIPE_OP_INFO (new_oper);
	      if (!(attr = L_find_attr (new_oper->attr, "iter")))
		L_punt ("Lpipe_mve_transform: oper has no iter attribute"
			" - oper: %d\n", new_oper->id);
	      L_set_int_attr_field (attr, 0, (copy + 1));
	      L_insert_oper_after (kernel_cb, kernel_cb->last_op,
				   new_oper);
	    }

	  /* Issue_time now becomes the final issue time within the
	   * unrolled kernel.  The issue time within a single iteration
	   * is still available as the intra_iter_issue_time. 
	   */

	  new_softpipe_info->issue_time = copy * ii +
	    softpipe_info->intra_iter_issue_time % ii;

	  new_softpipe_info->kernel_copy = copy;
	  if ((isl_attr = L_find_attr (new_oper->attr, "isl")))
	    L_set_int_attr_field (isl_attr, 0,
				  isl_attr->field[0]->value.i +
				  (ii * copy));
	  else
	    L_punt ("Lpipe_mve_transform: copied oper has no isl attribute");

	  /* rename destination operands */

	  for (j = 0; j < L_max_dest_operand; j++)
	    {
	      if (!L_is_register (new_oper->dest[j]))
		continue;

	      dest_mve_info = new_softpipe_info->dest_mve_info[j];
	      lr_info = dest_mve_info->live_range;

	      if (oper == lr_info->def_sm_oper->lcode_op)
		{
		  /* COPY OF REFERENCE DEFINITION: Versions of the
		   * variant are referred to using the kernel copy and
		   * intra-iteration stage (def_stage) in which that
		   * version is defined.  Copy x of the kernel uses
		   * version ((stage_count - def_stage + x) mod
		   * num_names) of the variant.  Using this
		   * convention, the first uses of live in variants
		   * always have the same name as outside the loop.
		   */

		  def_copy = copy;
		}
	      else
		{
		  /* Need to rename to match the reference definition.
		   * Find the copy of the kernel that contains the
		   * reference definition. See renaming of source
		   * operands 
		   */
		  if (dest_mve_info->lifetime >= 0)
		    {
		      stage_lifetime = dest_mve_info->lifetime / ii;
		      if (dest_mve_info->stage_lifetime_incr)
			stage_lifetime++;
		      def_copy =
			(copy - stage_lifetime + unroll) % unroll;
		    }
		  else
		    {
		      stage_lifetime = -dest_mve_info->lifetime / ii;
		      if (dest_mve_info->stage_lifetime_incr)
			stage_lifetime++;
		      def_copy = (copy + stage_lifetime) % unroll;
		    }
		}
	      def_stage = lr_info->first_def_time / ii;
	      name_index = 
		(stage_count - def_stage + def_copy) % lr_info->num_names;
	      new_oper->dest[j]->value.r = lr_info->names[name_index]->value.r;
	    }

	  /* rename source operands */

	  for (j = 0; j < L_max_src_operand; j++)
	    {
	      /* loop invariants won't have Lpipe_MVEInfo structures */
	      if (!L_is_register (new_oper->src[j]) ||
		  !(src_mve_info = new_softpipe_info->src_mve_info[j]))
		continue;

	      /* Find the copy of the kernel that contains the
	       * reference definition for this lifetime.  If the
	       * lifetime is positive, that copy is the current copy
	       * minus the number of stages that the lifetime spans.
	       * If the variant is used before defined in a single
	       * copy of the kernel, go back 1 stage further.  Because
	       * some lifetimes are live across the backedge of the
	       * unrolled kernel, the resulting number may be
	       * negative.  stage_lifetime is guaranteed to be less
	       * than unroll, so add unroll and then coupute the
	       * result modulo unroll.  */

	      if (src_mve_info->lifetime >= 0)
		{
		  stage_lifetime = src_mve_info->lifetime / ii;
		  if (src_mve_info->stage_lifetime_incr)
		    stage_lifetime++;
		  def_copy = (copy + unroll - stage_lifetime) % unroll;
		}
	      else
		{
		  stage_lifetime = (-src_mve_info->lifetime) / ii;
		  if (src_mve_info->stage_lifetime_incr)
		    stage_lifetime++;
		  def_copy = (copy + stage_lifetime) % unroll;
		}
	      lr_info = src_mve_info->live_range;
	      def_stage = lr_info->first_def_time / ii;
	      name_index =
		(stage_count - def_stage + def_copy) % lr_info->num_names;
	      new_oper->src[j]->value.r = lr_info->names[name_index]->value.r;
	    }

	  /* rename pred[0] operands */

	  if (L_is_register (new_oper->pred[0]) &&
	      (src_mve_info = new_softpipe_info->pred_mve_info[0]))
	    {
	      /* Find the copy of the kernel in which this lifetime
	       * was defined.  That copy is the current copy minus the
	       * number of stages that the lifetime spans.  If the
	       * variant is used before defined in a single copy of
	       * the kernel, go back 1 stage further.  Because some
	       * lifetimes are live across the backedge of the
	       * unrolled kernel, the resulting number may be
	       * negative.  stage_lifetime is guaranteed to be less
	       * than unroll, so add unroll and then compute the
	       * resulting modulo unroll. */
	      if (src_mve_info->lifetime >= 0)
		{
		  stage_lifetime = src_mve_info->lifetime / ii;
		  if (src_mve_info->stage_lifetime_incr)
		    stage_lifetime++;
		  def_copy = (copy + unroll - stage_lifetime) % unroll;
		}
	      else
		{
		  stage_lifetime = (-src_mve_info->lifetime) / ii;
		  if (src_mve_info->stage_lifetime_incr)
		    stage_lifetime++;
		  def_copy = (copy + stage_lifetime) % unroll;
		}
	      
	      lr_info = src_mve_info->live_range;
	      def_stage = lr_info->first_def_time / ii;
	      name_index =
		(stage_count - def_stage + def_copy) % lr_info->num_names;
	      new_oper->pred[0]->value.r = lr_info->names[name_index]->value.r;
	    }
	  if (oper == kernel_copy_last_op[0])
	    break;
	}

      /* First oper of copies 1 to unroll-1 of the kernel is the oper
         after the last oper of the previous copy of the kernel.  The
         last oper of this copy of the kernel is the last operation in
         the cb. */

      if (copy != 0)
	{
	  kernel_copy_first_op[copy] = temp_last_op->next_op;
	  kernel_copy_last_op[copy] = kernel_cb->last_op;
	  temp_last_op = kernel_cb->last_op;
	}
    }
  return;
}


/* Find variants that are live into the original loop body, and insert
   a move to copy the value defined before the loop to the name used
   inside the loop after MVE. */
void
Lpipe_fix_live_in (SM_Cb * sm_cb)
{
#if 1
  L_punt ("Lpipe_fix_live_in: UNSUPPORTED");
#else
  L_Func *fn = sm_cb->lcode_fn;
  L_Cb *kernel_cb = sm_cb->lcode_cb, *prologue_cb = kernel_cb->prev_cb;
  L_Oper *oper, *new_oper, *first_op, *last_op;
  Softpipe_Op_Info *softpipe_info;
  Lpipe_MVEInfo *src_mve_info;
  Lpipe_LRInfo *lr_info;
  int j, stage_count = sm_cb->stages;

  /* find live in variables */

  /* AARGH! Can't do this!!! Accurate dataflow has been lost! */
  L_do_flow_analysis (fn, LIVE_VARIABLE);

  /* Check source operands in prologue to find live in loop variants. */

  for (oper = prologue_cb->first_op; oper; oper = oper->next_op)
    {
      if (Lpipe_ignore_kernel_inst (oper))
	continue;

      /* for remainder loop schema, some opers in prologue do not have
	 Softpipe_Op_Info structures */
      if (!(softpipe_info = SOFTPIPE_OP_INFO (oper)))
	continue;

      for (j = 0; j < L_max_src_operand; j++)
	{
	  if (!L_is_register (oper->src[j]))
	    continue;

	  /* exclude loop invariants */
	  if (!(src_mve_info = softpipe_info->src_mve_info[j]))
	    continue;

	  lr_info = src_mve_info->live_range;
	      
	  /* if variant is live in, and its current name is not the
	     same as the one in the orig loop body, insert a move */
	  if (L_in_cb_IN_set (prologue_cb, oper->src[j]) &&
	      (!L_same_operand (oper->src[j], lr_info->names[0])) &&
	      (!lr_info->prologue_mov_inserted))
	    {
	      L_punt
		("Lpipe_fix_live_in: Should not need to fix live in.  "
		 "Function %s, cb = %d, oper = %d\n", fn->name,
		 prologue_cb->id, oper->id);

	      new_oper =
		Lpipe_gen_mov_consuming_operands (L_copy_operand
						  (oper->src[j]),
						  L_copy_operand
						  (lr_info->names
						   [0]));
	      L_insert_oper_before (prologue_cb,
				    prologue_cb->first_op, new_oper);
	      lr_info->prologue_mov_inserted = 1;
	    }
	}
    }

  /* Check source operands in last stage of first iteration.  This is
     in the first copy of the kernel */

  first_op = kernel_copy_first_op[0];
  last_op = kernel_copy_last_op[0];

  for (oper = first_op; oper; oper = (oper!=last_op) ? oper->next_op : NULL)
    {
      if (Lpipe_ignore_kernel_inst (oper))
	continue;

      /* Only need to look at first copy of kernel.  Will break out of 
	 this loop after kernel_copy_last_op[0] */

      softpipe_info = SOFTPIPE_OP_INFO (oper);
      /* only look at last stage of first iteration */
      if (softpipe_info->stage != (stage_count - 1))
	continue;

      for (j = 0; j < L_max_src_operand; j++)
	{
	  if (!L_is_register (oper->src[j]))
	    continue;

	  /* exclude loop invariants */
	  if (!(src_mve_info = softpipe_info->src_mve_info[j]))
	    continue;

	  lr_info = src_mve_info->live_range;
	      
	  /* if variant is live in to the prologue and 
	     its current name is not the same as the 
	     one in the original loop body, and a move has not
	     already been inserted for this variant, 
	     insert a move */
	  if (!L_in_cb_IN_set (prologue_cb, oper->src[j]))
	    continue;

	  if (lr_info->prologue_mov_inserted)	  
	    continue;

	  if (L_same_operand (oper->src[j], lr_info->names[0]))
	    continue;

	  L_punt ("Lpipe_fix_live_in: "
		  "Should not need to fix live in.  "
		  "Function %s, cb = %d, oper = %d\n",
		  fn->name, kernel_cb->id, oper->id);
	      
	  new_oper =
	    Lpipe_gen_mov_consuming_operands (L_copy_operand (oper->src[j]),
				       L_copy_operand (lr_info->names [0]));
	  L_insert_oper_before (prologue_cb, prologue_cb->first_op, new_oper);
	  lr_info->prologue_mov_inserted = 1;
	}
    }
#endif
  return;
}


static void
Lpipe_merge_compensation_code (L_Cb * epilogue_cb)
{
  L_Oper *last_oper, *oper, *new_oper;
  L_Cb *ssa_cb;

  last_oper = epilogue_cb->last_op;
  if (L_uncond_branch (last_oper))
    {
      ssa_cb = L_find_branch_dest (last_oper);
      if (L_find_attr (ssa_cb->attr, "ssa_comp_code"))
	{
	  for (oper = ssa_cb->first_op; oper; oper = oper->next_op)
	    {
	      if (!L_general_move_opcode (oper))
		continue;
	      new_oper = L_copy_operation (oper);
	      L_insert_oper_before (epilogue_cb, last_oper, new_oper);
	    }
	}
    }
  return;
}


/* fix up live out for a single epilogue */
static void
Lpipe_fix_live_out_epi (L_Cb * kernel_cb, L_Oper * exit_branch)
{
  Softpipe_Op_Info *softpipe_info, *exit_branch_info;
  int epi_stage, theta, j;
  Lpipe_LRInfo *lr_info;
  L_Oper *oper, *new_oper;
  L_Operand *dest, *original_name;
  L_Cb *epilogue_cb;

  if (!L_cond_branch (exit_branch))
    L_punt ("Lpipe_fix_live_out_epi: Called on non-cond branch");

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);
  theta = exit_branch_info->stage;

  epilogue_cb = (exit_branch == kernel_cb->last_op) ?
    kernel_cb->next_cb : L_find_branch_dest (exit_branch);

  /* Check destination operands in last two iterations in epilogue to find 
     live out loop variants.  For now assume no EVRs. */

  for (oper = epilogue_cb->first_op; oper; oper = oper->next_op)
    {
      if (!Lpipe_ignore_kernel_inst (oper))
	{
	  /* some opers in epilogue (such as the jump) do not have 
	     Softpipe_Op_Info structures */
	  if (!(softpipe_info = SOFTPIPE_OP_INFO (oper)))
	    continue;

	  epi_stage = softpipe_info->epilogue_stage;

	  /* Only look at oper if it is from the last iteration or if it is
	     from the 2nd to last iteration and is from a later home block 
	     than the exit branch.  During epilogue stage x, the last
	     iteration is in pipeline stage (x + theta), etc.  */

	  if ((softpipe_info->stage == (epi_stage + theta)) ||
	      ((softpipe_info->stage == (epi_stage + theta + 1)) &&
	       (softpipe_info->home_block > exit_branch_info->home_block)))
	    {
	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  if (!(dest = oper->dest[j]) || !L_is_register (dest))
		    continue;

		  /* predicate for compare for reversed branch may not have
		     Lpipe_MVEInfo structure */
		  if (!softpipe_info->dest_mve_info[j])
		    continue;

		  lr_info = softpipe_info->dest_mve_info[j]->live_range;

		  /* quick check to see if live out of any branch in loop */
		  if (!lr_info->live_out)
		    continue;

		  /* exit branches from kernel have all been changed,
		     so must check live in of exit cb rather than live
		     out of branch */
		  original_name = lr_info->names[0];
		  if (!L_in_cb_IN_set
		      (exit_branch_info->exit_cb, original_name))
		    continue;

		  /* if live range has been renamed, insert an inst to move
		     the value to the register expected outside the loop */
		  if (!L_same_operand (dest, original_name))
		    {
		      new_oper =
			Lpipe_gen_mov_consuming_operands (L_copy_operand
							  (original_name),
							  L_copy_operand
							  (dest));
		      /* make sure move goes before jump at end of epilogue */
		      if (L_uncond_branch (epilogue_cb->last_op))
			L_insert_oper_before (epilogue_cb,
					      epilogue_cb->last_op, new_oper);
		      else
			L_insert_oper_after (epilogue_cb,
					     epilogue_cb->last_op, new_oper);
		    }
		}
	    }
	}
    }
  return;
}


/* fix up live out for a single epilogue */
static Set
Lpipe_fix_set_out_epi (Set to_fix, L_Cb * kernel_cb, L_Oper * exit_branch)
{
  L_Oper *oper, *new_oper;
  Softpipe_Op_Info *softpipe_info, *exit_branch_info;
  int epi_stage, j, theta;
  Lpipe_LRInfo *lr_info;
  L_Operand *dest, *original_name;
  L_Cb *epilogue_cb;

  if (!L_cond_branch (exit_branch))
    L_punt ("Lpipe_fix_live_out_epi: Called on non-cond branch");

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);
  theta = exit_branch_info->stage;
  if (exit_branch == kernel_cb->last_op)
    epilogue_cb = kernel_cb->next_cb;
  else
    epilogue_cb = L_find_branch_dest (exit_branch);

  /* Check destination operands in last two iterations in epilogue to find 
     live out loop variants.  For now assume no EVRs. */

  for (oper = epilogue_cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!Lpipe_ignore_kernel_inst (oper))
	{
	  softpipe_info = SOFTPIPE_OP_INFO (oper);

	  /* some opers in epilogue (such as the jump) do not have 
	     Softpipe_Op_Info structures */
	  if (softpipe_info == NULL)
	    continue;

	  epi_stage = softpipe_info->epilogue_stage;

	  /* Only look at oper if it is from the last iteration or if
	     it is from the 2nd to last iteration and is from a later
	     home block than the exit branch.  During epilogue stage
	     x, the last iteration is in pipeline stage (x + theta),
	     etc.  */

	  if ((softpipe_info->stage == (epi_stage + theta)) ||
	      ((softpipe_info->stage == (epi_stage + theta + 1)) &&
	       (softpipe_info->home_block > exit_branch_info->home_block)))
	    {
	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  dest = oper->dest[j];
		  if (!L_is_register (dest))
		    continue;

		  /* predicate for compare for reversed branch may not have
		     Lpipe_MVEInfo structure */
		  if (!softpipe_info->dest_mve_info[j])
		    continue;

		  lr_info = softpipe_info->dest_mve_info[j]->live_range;

		  /* quick check to see if live out of any branch in loop */
		  if (!lr_info->live_out)
		    continue;

		  /* exit branches from kernel have all been changed, so must
		     check live in of exit cb rather than live out of branch */
		  original_name = lr_info->names[0];

		  if (!Set_in (to_fix, L_REG_MAC_INDEX (original_name)))
		    continue;

		  /* if live range has been renamed, insert an inst to move
		     the value to the register expected outside the loop */
		  if (!L_same_operand (dest, original_name))
		    {
		      new_oper =
			Lpipe_gen_mov_consuming_operands (L_copy_operand
							  (original_name),
							  L_copy_operand
							  (dest));
		      /* make sure move goes before jump at end of epilogue */
		      if (L_uncond_branch (epilogue_cb->last_op))
			L_insert_oper_before (epilogue_cb,
					      epilogue_cb->last_op, new_oper);
		      else
			L_insert_oper_after (epilogue_cb,
					     epilogue_cb->last_op, new_oper);
		    }

		  to_fix =
		    Set_delete (to_fix, L_REG_MAC_INDEX (original_name));
		}
	    }
	}
    }
  return to_fix;
}


/* Fix up live out for the operations in iteration stage "stage" in
   the kernel copy defined by first and last op.  This is for the opers
   of the last iteration that do not appear in the epilogue.  Also
   check the 2nd to last iteration because of exits from the middle of
   the original loop body. */
static void
Lpipe_fix_live_out_stage (L_Cb * kernel_cb, L_Oper * exit_branch,
			  L_Oper * first_op, L_Oper * last_op, int stage)
{
  L_Oper *oper;
  Softpipe_Op_Info *softpipe_info;
  Softpipe_Op_Info *exit_branch_info;
  int j;
  L_Operand *dest;
  Lpipe_LRInfo *lr_info;
  L_Oper *new_oper;
  L_Cb *epilogue_cb;
  L_Operand *original_name;

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);
  if (exit_branch == kernel_cb->last_op)
    epilogue_cb = kernel_cb->next_cb;
  else
    epilogue_cb = L_find_branch_dest (exit_branch);

  for (oper = first_op; oper != NULL; oper = oper->next_op)
    {
      if (!Lpipe_ignore_kernel_inst (oper))
	{
	  /* Will break out of this loop after last_op */

	  softpipe_info = SOFTPIPE_OP_INFO (oper);

	  /* Only look at oper if its from the last iteration and is _not_
	     from a later home block than the exit branch or if it is
	     from the 2nd to last iteration and _is_ from a later 
	     home block thanthe exit branch.  The last iteration is 
	     executing pipeline stage "stage" and the 2nd to last iteration
	     is executing pipeline stage stage+1.  If stage is -1, want to
	     check the first stage of the 2nd to last iteration. */

	  if (((stage != -1) &&
	       (softpipe_info->stage == stage) &&
	       (softpipe_info->home_block <= exit_branch_info->home_block)) ||
	      ((softpipe_info->stage == stage + 1) &&
	       (softpipe_info->home_block > exit_branch_info->home_block)))
	    {
	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  dest = oper->dest[j];
		  if (!L_is_register (dest))
		    continue;

		  /* predicate for compare for reversed branch may not have
		     Lpipe_MVEInfo structure */
		  if (!softpipe_info->dest_mve_info[j])
		    continue;

		  lr_info = softpipe_info->dest_mve_info[j]->live_range;

		  /* quick check to see if live out of any branch in loop */
		  if (!lr_info->live_out)
		    continue;

		  /* exit branches from kernel have all been changed, so must
		     check live in of exit cb rather than live out of branch */
		  original_name = lr_info->names[0];
		  if (!L_in_cb_IN_set
		      (exit_branch_info->exit_cb, original_name))
		    continue;

		  /* if live range has been renamed, insert an inst to move
		     the value to the register expected outside the loop */

		  if (dest->value.r != original_name->value.r)
		    {
		      new_oper =
			Lpipe_gen_mov_consuming_operands (L_copy_operand
							  (original_name),
							  L_copy_operand
							  (dest));
		      /* make sure move goes before jump at end of epilogue */
		      if (L_uncond_branch (epilogue_cb->last_op))
			L_insert_oper_before (epilogue_cb,
					      epilogue_cb->last_op, new_oper);
		      else
			L_insert_oper_after (epilogue_cb,
					     epilogue_cb->last_op, new_oper);
		    }
		}
	    }

	  if (oper == last_op)
	    break;
	}
    }
}


/* Fix up live out for the operations in iteration stage "stage" in
   the kernel copy defined by first and last op.  This is for the opers
   of the last iteration that do not appear in the epilogue.  Also
   check the 2nd to last iteration because of exits from the middle of
   the original loop body. */
static Set
Lpipe_fix_set_out_stage (Set to_fix, L_Cb * kernel_cb, L_Oper * exit_branch,
			 L_Oper * first_op, L_Oper * last_op, 
			 int stage)
{
  L_Oper *oper, *new_oper;
  Softpipe_Op_Info *softpipe_info, *exit_branch_info;
  Lpipe_LRInfo *lr_info;
  L_Cb *epilogue_cb;
  L_Operand *dest, *original_name;
  int j;

  exit_branch_info = SOFTPIPE_OP_INFO (exit_branch);
  if (exit_branch == kernel_cb->last_op)
    epilogue_cb = kernel_cb->next_cb;
  else
    epilogue_cb = L_find_branch_dest (exit_branch);

  for (oper = last_op; oper != NULL; oper = oper->prev_op)
    {
      if (!Lpipe_ignore_kernel_inst (oper))
	{
	  /* Will break out of this loop after last_op */

	  softpipe_info = SOFTPIPE_OP_INFO (oper);

	  /* Only look at oper if it is from the last iteration and is 
	     _not_ from a later home block than the exit branch or if it is
	     from the 2nd to last iteration and _is_ from a later home block
	     than the exit branch.  The last iteration is is executing
	     pipeline stage "stage" and the 2nd to last iteration is executing
	     pipeline stage stage+1.  If stage is -1, want to check the
	     first stage of the 2nd to last iteration. */

	  if (((stage != -1) &&
	       (softpipe_info->stage == stage) &&
	       (softpipe_info->home_block <= exit_branch_info->home_block)) ||
	      ((softpipe_info->stage == stage + 1) &&
	       (softpipe_info->home_block > exit_branch_info->home_block)))
	    {
	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  dest = oper->dest[j];
		  if (!L_is_register (dest))
		    continue;

		  /* predicate for compare for reversed branch may not have
		     Lpipe_MVEInfo structure */
		  if (!softpipe_info->dest_mve_info[j])
		    continue;

		  lr_info = softpipe_info->dest_mve_info[j]->live_range;

		  /* quick check to see if live out of any branch in loop */
		  if (!lr_info->live_out)
		    continue;

		  /* exit branches from kernel have all been changed, so must
		     check live in of exit cb rather than live out of branch */
		  original_name = lr_info->names[0];

		  if (!Set_in (to_fix, L_REG_MAC_INDEX (original_name)))
		    continue;

		  /* if live range has been renamed, insert an inst to move
		     the value to the register expected outside the loop */

		  if (dest->value.r != original_name->value.r)
		    {
		      new_oper =
			Lpipe_gen_mov_consuming_operands (L_copy_operand
							  (original_name),
							  L_copy_operand
							  (dest));
		      /* make sure move goes before jump at end of epilogue */
		      if (L_uncond_branch (epilogue_cb->last_op))
			L_insert_oper_before (epilogue_cb,
					      epilogue_cb->last_op, new_oper);
		      else
			L_insert_oper_after (epilogue_cb,
					     epilogue_cb->last_op, new_oper);

		      L_annotate_oper (L_fn, epilogue_cb, new_oper);
		      L_delete_oper (epilogue_cb, new_oper);
		    }

		  to_fix =
		    Set_delete (to_fix, L_REG_MAC_INDEX (original_name));
		}
	    }

	  if (oper == first_op)
	    break;
	}
    }

  return to_fix;
}


/* Move any variable live out from the last iteration to register
   expected outside loop if it is not already there.  If branch was in
   the middle of the original loop body, need to go back into the 2nd
   to last iteration as well. */
void
Lpipe_fix_live_out (SM_Cb * sm_cb, int unroll)
{
  Softpipe_Op_Info *softpipe_info;
  L_Oper *oper, *first_op, *last_op;
  L_Cb *kernel_cb = sm_cb->lcode_cb, *prologue_cb;

  prologue_cb = kernel_cb->prev_cb;

  if (Lpipe_schema == REM_LOOP)
    {
      Lpipe_fix_live_out_epi (kernel_cb, kernel_cb->last_op);
      /* Look at first stage of last iteration.  This is in the kernel. */
      first_op = kernel_copy_first_op[unroll - 1];
      last_op = kernel_copy_last_op[unroll - 1];
      Lpipe_fix_live_out_stage (kernel_cb, kernel_cb->last_op, first_op,
				last_op, 0);
    }
  else
    {				/* MULTI_EPI */
      Set to_fix;
      int copy, stage, theta;

      /* for multiple epilogues, need to fix live out for each exit to an
         epilogue in the prologue and kernel */

      for (oper = prologue_cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!Lpipe_ignore_kernel_inst (oper))
	    {
	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      if (L_cond_branch (oper))
		{
		  /* if epilogue for last branch in prologue is the same as
		     for last branch in kernel only fix the live out once when
		     looking at the branch in the kernel */
		  if (oper == prologue_cb->last_op
		      && softpipe_info->loop_back_br)
		    continue;

		  to_fix =
		    Set_copy (L_get_cb_IN_set (softpipe_info->exit_cb));

		  /* check the tail of the last 2 iterations */
		  to_fix =
		    Lpipe_fix_set_out_epi (to_fix, kernel_cb, oper);
		  /* Check live out from head of the last two
		     iterations.  Because the prologue is copied from
		     the kernel, examine the appropriate copy of the
		     kernel instead of the prologue itself.  This is
		     done because there are convenient pointers to
		     each copy of the kernel, but not for each stage
		     of the prologue.  The second to the last
		     iteration is examined even if the exit branch
		     corresponds to the first iteration of the loop.
		     The renaming and handling of live in is such that
		     the correct value will be found by pretending
		     there is an iteration before the first one. */

		  copy = softpipe_info->kernel_copy;
		  theta = softpipe_info->stage;
		  for (stage = theta; stage >= -1; stage--)
		    {
		      first_op = kernel_copy_first_op[copy];
		      last_op = (stage == theta) ? oper :
			kernel_copy_last_op[copy];

		      to_fix =
			Lpipe_fix_set_out_stage (to_fix, kernel_cb, oper,
						 first_op, last_op, stage);
		      copy = Lpipe_modulo_decrement (copy, unroll);
		    }

		  to_fix = Set_dispose (to_fix);
		  Lpipe_merge_compensation_code (L_find_branch_dest (oper));
		}
	    }
	}

      for (oper = kernel_cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (L_cond_branch (oper))
	    {
	      softpipe_info = SOFTPIPE_OP_INFO (oper);
	      to_fix = Set_copy (L_get_cb_IN_set (softpipe_info->exit_cb));
	      /* check the tail of the last 2 iterations */
	      to_fix = Lpipe_fix_set_out_epi (to_fix, kernel_cb, oper);

	      /* Check live out from head of the last 2
	         iterations. */
	      copy = softpipe_info->kernel_copy;
	      theta = softpipe_info->stage;

	      /* if stage = -1, really only checking stage 0 of the 2nd to
	         last iteration */
	      for (stage = theta; stage >= -1; stage--)
		{
		  first_op = kernel_copy_first_op[copy];
		  last_op = (stage == theta) ? oper : 
		    kernel_copy_last_op[copy];

		  to_fix = Lpipe_fix_set_out_stage (to_fix, kernel_cb, oper,
						    first_op, last_op, stage);
		  copy = Lpipe_modulo_decrement (copy, unroll);
		}

	      to_fix = Set_dispose (to_fix);
	      if (oper != kernel_cb->last_op)
		Lpipe_merge_compensation_code (L_find_branch_dest (oper));
	      to_fix = Set_dispose (to_fix);
	    }
	}
    }
  return;
}


int
Lpipe_get_pro_reg (SM_Cb *sm_cb, Lpipe_LRInfo *lr_info, 
		   Lpipe_MVEInfo *mve_info, int stg_ofst)
{
  L_Operand *opd;
  int nam_idx = Lpipe_find_name (sm_cb, lr_info, mve_info) + stg_ofst;

  if (nam_idx >= lr_info->num_names_w_pro)
    L_punt ("Lpipe_get_pro_reg: in fn %s cb %d: %d > %d --- "
	    "too many prologue names",
	    sm_cb->lcode_fn->name, sm_cb->lcode_cb->id, 
	    nam_idx, lr_info->num_names_w_pro);

  if (!(opd = lr_info->names[nam_idx]))
    {
      opd = L_copy_operand (lr_info->names[0]);
      opd->value.r = ++ sm_cb->lcode_fn->max_reg_id;
      lr_info->names[nam_idx] = opd;
    }

  return opd->value.r;
}
