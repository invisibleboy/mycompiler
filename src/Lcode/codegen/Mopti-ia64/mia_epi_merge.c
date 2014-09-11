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
/**************************************************************************\
 *
 *  File: epilogue_merge.c
 *
 *  Description: Epilogue merge
 *
 *  Modified:
 *               
\*****************************************************************************/
/* 09/17/02 REK Updating to use functions from libtahoeop instead of Tmdes */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "mia_internal.h"

extern int O_is_caller_save_predicate (L_Operand * pred);

/* #define EPILOGUE_DEBUG */

/* Possibly replace predicated branch with epilogue code */
#define MERGE_PREDICATED_RET_BR 1	/* 0 = no, 1 = yes */

/* Merge epilogue if opers_in_merge_space/epi_cb_opers> threshold */
/* Figure that if there is lots of instrs, there is lots of space */
#define EPILOGUE_MERGE_THRESHOLD 2.0
#define EPILOGUE_MERGE_MIN_OPERS 2

static L_Cb *
Find_epilogue_cb (L_Func * fn, int *size_of_cb)
{
  L_Cb *cb;
  L_Oper *oper;
  int count;

  for (cb = fn->last_cb; cb; cb = cb->prev_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_EPILOGUE))
	{
	  count = 0;
	  for (oper = cb->first_op; oper; oper = oper->next_op)
	    {
	      if (!LT_is_non_instr (oper))
		count++;
	    }			/* for oper */
	  *size_of_cb = count;
	  return (cb);
	}			/* if */
    }				/* for cb */
  L_punt ("Find_epilogue_cb: Epilogue cb not found\n");
  return (NULL);
}				/* Find_epilogue_cb */


/****************************************************************************
 *
 * routine: Count_opers_in_merge_space()
 * purpose: This function counts the number of opers between the given op and
 *          the first previous op which satisfies one of these conditions:
 *                1) the top of the block is reached,
 *                2) a previous branch is found,
 *                3) the defining compare of the predicate on the supplied
 *                   branch op if one is given.
 * input: cb - cb to count ops in
 *        branch_oper - If NULL, it is assumed that the cb does not end in
 *                      a branch and ops are counted from the end of the cb.
 *                      Otherwise, this is the branch which conditionally or
 *                      uncondtionally jumps to the epilogue.
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

static int
Count_opers_in_merge_space (L_Cb * cb, L_Oper * branch_oper)
{
  L_Oper *op;
  L_Oper *last_op;
  L_Operand *qp_operand;
  unsigned int count;

  if (branch_oper)
    {
      qp_operand = branch_oper->pred[0];
      last_op = branch_oper->prev_op;
    }				/* if */
  else
    {
      qp_operand = NULL;
      last_op = cb->last_op;
    }				/* else */

  /* Stop count because branch will halt code movement */
  /* Stop count because compare's dest is qualifying predicate */

  count = 0;
  for (op = last_op;
       op && !LT_is_call_br (op) && !LT_is_cond_br (op) &&
       !(qp_operand && LT_is_cmp_op (op) &&
	 L_is_dest_operand (qp_operand, op)); op = op->prev_op)
    {
      count++;
    }				/* for op */

  return (count);
}				/* Count_opers_in_merge_space */

static void
Rev_copy_prop_return_value (L_Func * fn, Set modified_cbs)
{
  L_Cb *cb;
  L_Oper *oper, *last_ret_val_def_oper;
  int delete_ret_oper;
  L_Operand *replace_operand;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* Do optimization only over cbs changed by epilogue merge */
      if (Set_in (modified_cbs, cb->id))
	{
	  last_ret_val_def_oper = NULL;
	  for (oper = cb->last_op; oper; oper = oper->prev_op)
	    {
	      if (L_subroutine_call_opcode (oper))
		break;

	      if (!(L_move_opcode (oper)))
		continue;

	      if (!((L_is_macro (oper->dest[0])) &&
		    (oper->dest[0]->value.mac == L_MAC_P16)))
		continue;

	      last_ret_val_def_oper = oper;
	      break;
	    }			/* for oper */

	  /* ret value move found, try to do reverse copy prop, ie. 
	     oper A     op z = x,y
	     oper B     move P16 = z

	     Right now very limited - will only do it if oper A is
	     unpredicated or if z is used as a source operand between
	     A and B */

	  if (last_ret_val_def_oper)
	    {
	      replace_operand = last_ret_val_def_oper->src[0];
	      delete_ret_oper = FALSE;

	      for (oper = last_ret_val_def_oper; oper; oper = oper->prev_op)
		{
		  /* Can't substitute above a jsr */
		  if (L_subroutine_call_opcode (oper))
		    break;

		  if (L_is_src_operand (replace_operand, oper))
		    break;

		  if (L_is_dest_operand (replace_operand, oper))
		    {
		      /* Only do substitution if not predicated */
		      if (oper->pred[0] == NULL)
			{
			  L_delete_operand (oper->dest[0]);
			  L_copy_operand (replace_operand);
			  delete_ret_oper = TRUE;
			}	/* if */
		      break;
		    }		/* if */
		}		/* for oper */

	      if (delete_ret_oper)
		L_delete_oper (cb, last_ret_val_def_oper);
	    }			/* if */
	}			/* if */
    }				/* for cb */
}				/* Rev_copy_prop_return_value */

void
Mopti_epilogue_merge (L_Func * fn)
{
  L_Cb *epilogue_cb, *rts_cb;
  L_Cb *src_cb;
  L_Oper *after_oper, *br_oper, *oper, *new_oper;
  L_Operand *predicate;
  L_Flow *flow, *epilogue_flow, hack_flow;
  int can_remove_epilogue_cb;
  int epilogue_cb_opers, opers_in_merge_space;
  float oper_ratio;
  Set modified_cbs;

  can_remove_epilogue_cb = TRUE;
  modified_cbs = NULL;
  epilogue_cb = Find_epilogue_cb (fn, &epilogue_cb_opers);

  rts_cb = L_new_cb (++L_fn->max_cb_id);
  L_insert_cb_after (fn, epilogue_cb, rts_cb);
  rts_cb->attr =
    L_concat_attr (rts_cb->attr, L_new_attr ("DEAD-EPILOGUE", 0));
  rts_cb->flags |= L_CB_EPILOGUE;
  /* copy the epilogue operations into this new rts-only cb */
  for (oper = epilogue_cb->first_op; oper; oper = oper->next_op)
    {
      if (LT_is_non_instr (oper))
	continue;
      if (!L_subroutine_return_opcode (oper))
	{
	  if (oper->dest[0] && oper->dest[0]->type == L_OPERAND_MACRO)
	    {
	      new_oper = L_create_new_op (Lop_DEFINE);
	      new_oper->src[0] = L_copy_operand (oper->dest[0]);
	      L_insert_oper_after (rts_cb, rts_cb->last_op, new_oper);
	    }			/* if */
	}			/* if */
      else
	{
	  new_oper = L_copy_operation (oper);
	  L_insert_oper_after (rts_cb, rts_cb->last_op, new_oper);
	}			/* else */
    }				/* for oper */

  for (flow = epilogue_cb->src_flow; flow; flow = flow->next_flow)
    {
      src_cb = flow->src_cb;

      /* don't merge epilogue into kernel of modulo scheduled loop */
      if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_SOFTPIPE))
	{
	  can_remove_epilogue_cb = FALSE;
	  continue;
	}			/* if */

      epilogue_flow =
	(L_Flow *) L_find_complement_flow (src_cb->dest_flow, flow);

      br_oper = L_find_branch_for_flow (src_cb, epilogue_flow);

      if (br_oper)
	{
	  /* branch to epilogue */
	  after_oper = br_oper->prev_op;
	  opers_in_merge_space = Count_opers_in_merge_space (src_cb, br_oper);

#ifdef EPILOGUE_DEBUG
	  fprintf (stderr,
		   "Branch op %d in cb %d (%d ops) to epilogue cb %d "
		   "(%d ops)\n",
		   br_oper->id, src_cb->id, opers_in_merge_space,
		   epilogue_cb->id, epilogue_cb_opers);
#endif
	  predicate = br_oper->pred[0];
	  if (predicate)
	    {
#if !MERGE_PREDICATED_RET_BR
	      /* do not do epilogue merge into a cb that has a conditional
	         branch to the epilogue block. */
	      can_remove_epilogue_cb = FALSE;
#ifdef EPILOGUE_DEBUG
	      fprintf (stderr, "   Never merging to conditional branches\n");
#endif
	      continue;
#else
	      /* First determine if the predicate on the branch is a caller
	         save predicate.  If so, there is no problem.  If not, then
	         we must check the epilogue code for a restore of pr.  This
	         pr restore and the return cannot be guarded by a callee
	         save predicate since the value of the predicate will
	         change. */

	      if (!O_is_caller_save_predicate (predicate))
		{
		  /* check for mov pr = rX, mask */
		  for (oper = epilogue_cb->first_op; oper;
		       oper = oper->next_op)
		    {
		      if (L_is_macro (oper->dest[0]) &&
			  (oper->dest[0]->value.mac == TAHOE_PRED_BLK_REG))
			break;
		    }		/* for oper */

		  if (oper)
		    {
		      /* above loop must have terminated from break */
		      can_remove_epilogue_cb = FALSE;
		      continue;
		    }		/* if */
		}		/* if */

	      /* estimate if it is profitable to merge. */

	      oper_ratio = opers_in_merge_space / epilogue_cb_opers;
	      if ((epilogue_cb_opers < EPILOGUE_MERGE_MIN_OPERS) ||
		  (oper_ratio >= EPILOGUE_MERGE_THRESHOLD))
		{
#ifdef EPILOGUE_DEBUG
		  fprintf (stderr, "Predicated merge after oper %d "
			   "(ratio = %4.2f)\n", after_oper->id, oper_ratio);
#endif
		}		/* if */
	      else
		{
#ifdef EPILOGUE_DEBUG
		  fprintf (stderr,
			   "Not merging after oper %d (ratio = %4.2f)\n",
			   after_oper->id, oper_ratio);
#endif
		  can_remove_epilogue_cb = FALSE;
		  continue;
		}		/* else */
#endif
	    }			/* if */
	  else
	    {
#ifdef EPILOGUE_DEBUG
	      /* no predicate on the branch.  It is a jump to the
	         epilogue. */
	      fprintf (stderr, "Jump op %d in cb %d to epilogue cb %d "
		       "(%d ops)\n",
		       br_oper->id, src_cb->id, epilogue_cb->id,
		       epilogue_cb_opers);
#endif
	    }			/* else */
	}			/* if */
      else
	{
	  /* fall thru to epilogue */
	  predicate = NULL;
	  after_oper = src_cb->last_op;
	  /* Delete this flow */
	  hack_flow.next_flow = flow->next_flow;
	  src_cb->dest_flow =
	    L_delete_flow (src_cb->dest_flow, epilogue_flow);
	  epilogue_cb->src_flow = L_delete_flow (epilogue_cb->src_flow, flow);
	  flow = &hack_flow;

#ifdef EPILOGUE_DEBUG
	  fprintf (stderr, "Fall thru cb %d to epilogue cb %d "
		   "(%d ops)\n",
		   src_cb->id, epilogue_cb->id, epilogue_cb_opers);
#endif
	}			/* else */

      modified_cbs = Set_add (modified_cbs, src_cb->id);

#ifdef EPILOGUE_DEBUG
      fprintf (stderr, "   Copy epilogue ops after op %d in cb %d\n",
	       after_oper->id, src_cb->id);
#endif
      for (oper = epilogue_cb->first_op; oper; oper = oper->next_op)
	{

	  if (LT_is_non_instr (oper))
	    continue;

	  if (IS_PREDICATED (oper))
	    L_punt ("Mopti_epilogue_merge: Epilogue cb oper %d has a "
		    "predicate\n", oper->id);

	    /*** CHAD: If this oper is a return, and br_oper!=NULL, then
		 don't copy this instruction... instead, tag the branch
		 to be converted during phase 3! ****/

	  if ((br_oper) && L_subroutine_return_opcode (oper))
	    {
	      /* add the "cvt to rts" attrib */
	      br_oper->attr =
		L_concat_attr (br_oper->attr, L_new_attr ("TO-RTS", 0));
	      epilogue_flow->dst_cb = rts_cb;
	      /* delete this flow */
	      hack_flow.next_flow = flow->next_flow;
	      epilogue_cb->src_flow =
		L_delete_flow (epilogue_cb->src_flow, flow);
	      flow = &hack_flow;
	      br_oper->src[0]->value.cb = rts_cb;
	      after_oper = br_oper;
	      continue;
	    }			/* if */

	    /*** END CHAD ***/

	  new_oper = L_copy_operation (oper);
	  new_oper->pred[0] = L_copy_operand (predicate);
	  L_insert_oper_after (src_cb, after_oper, new_oper);
	  after_oper = new_oper;
	}			/* for oper */
      src_cb->flags |= epilogue_cb->flags;
    }				/* for flow */

  if (can_remove_epilogue_cb)
    L_delete_cb (fn, epilogue_cb);

  Rev_copy_prop_return_value (fn, modified_cbs);
  Set_dispose (modified_cbs);

  L_rebuild_src_flow (fn);
}				/* Mopti_epilogue_merge */

void
Mopti_epilogue_cleanup (L_Func * fn)
{
  /* check each branch oper for the "TO-RTS" attrib, 
     replace it as an RTS instruction and delete its flow */

  L_Cb *cb, *epilogue_cb;
  L_Oper *op, *rts_op;

  /* fprintf(stderr, "\nepilogue cleanup!\n"); */

  for (epilogue_cb = fn->last_cb; epilogue_cb;
       epilogue_cb = epilogue_cb->prev_cb)
    if (L_find_attr (epilogue_cb->attr, "DEAD-EPILOGUE"))
      break;

  if (!epilogue_cb)
    {
      fprintf (stderr,
	       "Warning - epilogue_cleanup() - no epilogue cb was found!\n");
      return;
    }				/* if */

  for (rts_op = epilogue_cb->last_op;
       (rts_op != NULL) && (!L_subroutine_return_opcode (rts_op));
       rts_op = rts_op->prev_op);

  if (!rts_op)
    {
      fprintf (stderr, "Warning - no rts found in epilogue cb %d!\n",
	       epilogue_cb->id);
      return;
    }				/* if */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (op = cb->first_op; op; op = op->next_op)
      {
	L_Oper *copy_rts;
	L_Flow *br_flow;
	L_Attr *isl_attr;
	if (NULL == L_find_attr (op->attr, "TO-RTS"))
	  continue;

	/* convert op to an rts */
	/* fprintf(stderr, "RTS'ing oper %d\n",op->id); */
	copy_rts = L_copy_operation (rts_op);
	if (op->pred[0])
	  copy_rts->pred[0] = L_copy_operand (op->pred[0]);
	L_insert_oper_after (cb, op, copy_rts);

	/* fprintf(stderr, "new oper = %d\n",op->next_op->id); */
	isl_attr = L_find_attr (copy_rts->attr, "isl");
	if (isl_attr)
	  copy_rts->attr = L_delete_attr (copy_rts->attr, isl_attr);

	isl_attr = L_find_attr (op->attr, "isl");
	if (isl_attr)
	  copy_rts->attr =
	    L_concat_attr (copy_rts->attr, L_copy_attr (isl_attr));

	br_flow = L_find_flow_for_branch (cb, op);
	if (!br_flow)
	  {
	    fprintf (stderr, "Warning! Epilogue cleanup cannot find"
		     " branch flow!\n");
	    continue;
	  }			/* if */

	cb->dest_flow = L_delete_flow (cb->dest_flow, br_flow);

	L_delete_oper (cb, op);
	op = copy_rts;
      }				/* for op */

  L_delete_cb (fn, epilogue_cb);
}				/* Mopti_epilogue_cleanup */
