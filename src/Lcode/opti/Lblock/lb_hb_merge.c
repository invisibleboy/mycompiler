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
/*===========================================================================
 *	File :		lb_hb_merge.c
 *	Description :	merge ops on opposite predicates
 *	Creation Date :	September 1993
 *	Authors : 	Scott Mahlke, Kevin Crozier, Dan Connors
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *       New instruction merging optimizations 
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_hb_hyperblock.h"

#define ERR	stderr

#define DO_MERGES          0
#define DO_MERGE1          1
#define DO_MERGE2          1
#define DO_MERGE3          1
#define DO_MERGE4          1
#define DO_MIGRATION_MERGE 1

#define DEBUG_MERGE        0

#define L_MAX_ITERATION	  10

#if DO_MERGES
static int LB_hb_do_pred_mergeS (L_Cb * cb);
#endif
static int LB_hb_do_pred_merge1 (L_Cb * cb);
static int LB_hb_do_pred_merge2 (L_Cb * cb);
static int LB_hb_do_pred_merge3 (L_Cb * cb);
static int LB_hb_do_pred_merge4 (L_Cb * cb);
static int LB_hb_do_migration_merging (L_Cb * cb);

/*======================================================================*/

/*
 *    Combine identical operations on opposite predicates
 */

static int
LB_hb_do_pred_merge1 (L_Cb * cb)
{
  int change = 0;
  int leave_marked_masked;

  L_Oper *opA, *opB, *nextA, *nextB, *def_op;

  /*
   * PATTERN I:
   * ----------------------------------------------------------------------
   *     (q) p1,p2 = ...
   * A: (p1) op x = y,z --> X
   * B: (p2) op x = y,z --> B:  (q) op x = y,z
   */

  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      int load_flag, store_flag;
      if (!L_is_predicated (opA))
	continue;

      store_flag = L_general_store_opcode (opA);

      for (opB = opA->next_op; opB; opB = nextB)
	{
	  nextB = opB->next_op;

	  if (!L_is_predicated (opB) ||
	      !L_same_opcode (opA, opB) || !L_same_compare (opA, opB) ||
	      !L_same_src_operands (opA, opB))
	    continue;

	  if ((def_op = L_find_pred_definition (opA)) &&
	      !PG_rel_complementary_predicates_ops (opA, opB, def_op))
	    def_op = NULL;

	  if (!def_op && !PG_complementary_predicates_ops (opA, opB))
	    continue;

	  if (!store_flag)
	    {
	      if (!L_same_dest_operands (opA, opB) ||
		  !L_is_variable (opB->dest[0]))
		continue;

	      if (L_pred_define_opcode (opA))
		{
		  int i;
		  L_Operand *destA, *destB;

		  for (i = 0; i < L_max_dest_operand; i++)
		    {
		      if (!(destA = opA->dest[i]))
			continue;
		      destB = opB->dest[i];
		      if (destA->ptype != destB->ptype)
			break;
		    }
		  
		  if (i < L_max_dest_operand)
		    continue;
		}
	    }

	  if (!L_can_merge_with_op_above (cb, opA, opB))
	    continue;

	  /* REPLACE */

	  if (!store_flag)
	    {
	      load_flag = L_general_load_opcode (opA);

	      if (load_flag)
		{
		  L_union_sync_arc_info (opA, opB);
		  L_merge_acc_spec_list (opA, opB);
		}
	      
	      if (load_flag && (!L_no_br_between(opA, opB) ||
				L_EXTRACT_BIT_VAL(opA->flags,
						  L_OPER_MASK_PE) ||
				L_EXTRACT_BIT_VAL(opB->flags,
						  L_OPER_MASK_PE)))
		{
		  /* Need to insert up to two checks */
		  if (!L_EXTRACT_BIT_VAL(opA->flags, L_OPER_MASK_PE))	       
		    L_insert_check(cb, opA); /* Otherwise it should
						already have necessary
						checks in place */
		  
		  if (!L_EXTRACT_BIT_VAL(opB->flags, L_OPER_MASK_PE))
		    L_insert_check(cb, opB); /* Otherwise it should
						already have necessary
						checks in place */
		  leave_marked_masked = 1;
		}
	      else
		{
		  leave_marked_masked = 0;
		}

	      /* Unmask loads that aren't really executing more often
		 than they did before and assign checks for the
		 deleted oper to the promoted oper */
	      
	      if (load_flag)
		{
		  if (!leave_marked_masked)  
		    {
		      opA->flags = L_CLR_BIT_FLAG (opA->flags, L_OPER_MASK_PE);
		      opA->flags = L_CLR_BIT_FLAG (opA->flags, 
						   L_OPER_PROMOTED);
		    }
		  else
		    {
		      L_assign_all_checks(L_fn, opB, opA);
		    }
		}
	    }
	  else
	    {
	      L_union_sync_arc_info (opA, opB);
	      L_merge_acc_spec_list (opA, opB);
	    }

#if DEBUG_MERGE
	  fprintf (ERR, "MRG> 1.1 %d,%d into %d\n", opA->id, opB->id,
		   opA->id);
#endif

	  L_delete_operand (opA->pred[0]);
	  opA->pred[0] = (def_op && def_op->pred[0]) ?
	    L_copy_operand (def_op->pred[0]) : NULL;

	  change++;
	  L_nullify_operation (opB);
	  if (!L_is_predicated (opA))
	    break;
	}
    }

  /*
   * PATTERN II:
   * ----------------------------------------------------------------------
   *     (q) p1,p2 = ...
   * A: (p1) op x = y,z --> A:  (q) op x = y,z
   * B: (p2) op x = y,z --> X
   */

  for (opB = cb->last_op; opB; opB = opB->prev_op)
    {
      if (!L_is_predicated (opB))
	continue;

      for (opA = opB->prev_op; opA; opA = nextA)
	{
	  int load_flag, store_flag;
	  nextA = opA->prev_op;

	  if (!L_is_predicated (opA) ||
	      !L_same_opcode (opA, opB) || !L_same_compare (opA, opB) ||
	      !L_same_src_operands (opA, opB) ||
	      !L_same_dest_operands (opA, opB) ||
	      !L_is_register (opB->dest[0]))
	    continue;

	  if ((def_op = L_find_pred_definition (opA)))
	    {
	      if (!PG_rel_complementary_predicates_ops (opA, opB, def_op))
		def_op = NULL;
	    }

	  if (!def_op && !PG_complementary_predicates_ops (opA, opB))
	    continue;

	  if (L_pred_define_opcode (opA))
	    {
	      int i;
	      L_Operand *destA, *destB;

	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (!(destA = opA->dest[i]))
		    continue;
		  destB = opB->dest[i];
		  if (destA->ptype != destB->ptype)
		    break;
		}

	      if (i < L_max_dest_operand)
		continue;
	    }

	  if (!L_can_merge_with_op_below (cb, opB, opA))
	    continue;

	  load_flag = L_general_load_opcode (opA);
	  store_flag = L_general_store_opcode (opA);

	  /* REPLACE */

#if DEBUG_MERGE
	  fprintf (ERR, "MRG> 1.2 %d,%d into %d\n", opA->id, opB->id,
		   opB->id);
#endif

	  if (load_flag && 
	      ((L_EXTRACT_BIT_VAL(opA->flags, L_OPER_MASK_PE)) ||
	       (L_EXTRACT_BIT_VAL(opB->flags, L_OPER_MASK_PE))))
	    {
	      /* Need to insert up to two checks */
	      if (!L_EXTRACT_BIT_VAL(opA->flags, L_OPER_MASK_PE))	       
		L_insert_check_after(cb, opA, opB); /* Otherwise it should
						       already have necessary
						       checks in place */
	      
	      if (!L_EXTRACT_BIT_VAL(opB->flags, L_OPER_MASK_PE))
		L_insert_check(cb, opB); /* Otherwise it should
					    already have necessary
					    checks in place */
	      leave_marked_masked = 1;
	    }
	  else
	    {
	      leave_marked_masked = 0;
	    }

	  L_delete_operand (opB->pred[0]);
	  opB->pred[0] = (def_op && def_op->pred[0]) ?
	    L_copy_operand (def_op->pred[0]) : NULL;

	  /* Unmask loads that aren't really executing more often than they
	     did before and assign checks for the deleted oper to the
	     promoted oper */
	  if (load_flag)
	    {
	      if (!leave_marked_masked)  
		{
		  opA->flags = L_CLR_BIT_FLAG (opA->flags, L_OPER_MASK_PE);
		  opA->flags = L_CLR_BIT_FLAG (opA->flags, L_OPER_PROMOTED);
		}
	      else
		{
		  L_assign_all_checks(L_fn, opA, opB);
		}
	    }

	  L_nullify_operation (opA);
	  change++;
	  break;
	}
    }

  /*
   * PATTERN III: Downward branch / store combining
   * ----------------------------------------------------------------------
   *     (q) p1,p2 = ...
   * A: (p1) op y,z --> X
   * B: (p2) op y,z --> B:  (q) op y,z
   */

  for (opA = cb->first_op; opA != NULL; opA = nextA)
    {
      nextA = opA->next_op;

      if (!L_is_predicated (opA) ||
	  L_pred_define_opcode (opA) ||
	  !(L_general_branch_opcode (opA) || 
	    L_general_store_opcode (opA)))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  L_Cb *target_cb;
	  L_Flow *flow, *flow2;
	  double weight;

	  if (!L_is_predicated (opB) ||
	      !L_same_opcode (opA, opB) || !L_same_compare (opA, opB) ||
	      !L_same_src_operands (opA, opB))
	    continue;

	  if ((def_op = L_find_pred_definition (opA)))
	    {
	      if (!PG_rel_complementary_predicates_ops (opA, opB, def_op))
		def_op = NULL;
	    }

	  if (!def_op && !PG_complementary_predicates_ops (opA, opB))
	    continue;

	  if (!L_can_merge_with_op_below (cb, opB, opA))
	    continue;

	  /* REPLACE */

#if DEBUG_MERGE
	  fprintf (ERR, "MRG> 1.3 stores/branches %d,%d into %d\n",
		   opA->id, opB->id, opB->id);
#endif

	  L_delete_operand (opB->pred[0]);
	  opB->pred[0] = (def_op && def_op->pred[0]) ?
	    L_copy_operand (def_op->pred[0]) : NULL;

	  /* fix appropriate flow arcs */
	  if (L_general_branch_opcode (opA))
	    {
	      flow = L_find_flow_for_branch (cb, opA);
	      weight = flow->weight;
	      target_cb = flow->dst_cb;

	      L_delete_complete_oper (cb, opA);

	      flow = L_find_flow_for_branch (cb, opB);
	      flow2 = L_find_matching_flow (target_cb->src_flow, flow);
	      flow->weight += weight;
	      flow2->weight += weight;
	    }
	  else
	    {
	      L_nullify_operation (opA);
	    }
	  change++;
	  break;
	}
    }

  return (change);
}


/*
 *    Combine operations in which 1 is ancestor of other
 */
static int
LB_hb_do_pred_merge2 (L_Cb * cb)
{
  int change = 0;
  L_Oper *opA, *opB, *nextB;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_is_predicated (opA))
	continue;
      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;
	  if (!L_is_predicated (opB) ||
	      !L_same_opcode (opA, opB) || !L_same_compare (opA, opB) ||
	      L_pred_define_opcode (opB) ||
	      !L_same_src_operands (opA, opB) ||
	      !L_same_dest_operands (opA, opB) ||
	      !PG_superset_predicate_ops (opA, opB) ||
	      !L_is_register (opB->dest[0]) ||
	      !L_all_dest_operand_no_defs_between (opA, opA, opB) ||
	      !L_all_src_operand_same_def_reachs (opA, opA, opB))
	    continue;

	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = L_general_load_opcode (opA);
	  store_flag = L_general_store_opcode (opA);
	  if ((load_flag || store_flag)
	      && !L_no_overlap_write (cb, opA, opA, 1, opB, 1))
	    break;
	  if (store_flag && !L_no_overlap_read (cb, opA, opA, 1, opB, 1))
	    break;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;
#if DEBUG_MERGE
	  fprintf (ERR, "MRG> 2 %d,%d (delete %d)\n", opA->id, opB->id,
		   opB->id);
#endif
	  L_nullify_operation (opB);
	  change++;
	  if (!L_is_predicated (opA))
	    break;
	}
    }

  return (change);
}


/*
 *	Ret 1 iff no write to same locn of op between opA and opB
 *      prevents the merging of opA and opB to the location of opA.
 *      JWS 19991019
 */
static int
L_can_merge_mems (L_Cb * cb, L_Oper * opA, L_Oper * opB)
{
  L_Oper *ptr;
  int dep_flags;

  dep_flags = SET_NONLOOP_CARRIED (0);

  if (!(L_general_load_opcode (opA) || L_general_store_opcode (opA)))
    L_punt ("L_can_merge_mems: opA not a memory op");
  if (!(L_general_load_opcode (opB) || L_general_store_opcode (opB)))
    L_punt ("L_can_merge_mems: opB not a memory op");

  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == opB)
	break;
      if (L_general_store_opcode (ptr) &&
	  !L_independent_memory_ops (cb, ptr, opB, dep_flags) &&
	  PG_intersecting_predicates_ops (ptr, opB))
	return 0;
      if (L_subroutine_call_opcode (ptr) &&
	  !L_independent_memory_and_jsr (cb, ptr, opB) &&
	  PG_intersecting_predicates_ops (opB, ptr))
	return 0;
    }

  if (ptr != opB)
    L_punt ("L_can_merge_mems: opA does not reach opB");

  return 1;
}


/*
 *    Merging ops w/o identical destinations
 */
static int
LB_hb_do_pred_merge3 (L_Cb * cb)
{
  int i, leave_marked_masked, change = 0;
  L_Oper *opA, *opB, *nextB, *def_opA, *def_opB, *ptr;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_is_predicated (opA) ||
          !L_safe_to_delete_opcode (opA) ||
	  L_pred_define_opcode (opA))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;

	  /*
	   *  Match pattern no. 1 (op with destination, elim op B)
	   */

	  if (!L_is_predicated (opB) ||
	      !L_same_opcode (opA, opB) || !L_same_compare (opA, opB) ||
	      !L_same_src_operands (opA, opB) ||
	      L_same_dest_operands (opA, opB) ||
	      !L_is_register (opA->dest[0]) ||
	      !L_is_register (opB->dest[0]))
	    continue;

	  def_opA = L_find_pred_definition (opA);
	  def_opB = L_find_pred_definition (opB);
	  if (!def_opA || !def_opB)
	    continue;
	  if (!PG_sum_predicates_ops (opA, opB, def_opA))
	    continue;
	  if (!L_safe_to_promote (cb, opA, def_opA))
	    continue;
	  if (!L_safe_to_promote (cb, opA, def_opB))
	    continue;
	  if (!L_safe_to_promote (cb, opB, def_opA))
	    continue;
	  if (!L_safe_to_promote (cb, opB, def_opB))
	    continue;
	  if (!L_all_dest_operand_no_defs_between (opA, opA, cb->last_op))
	    continue;
	  if (!L_all_src_operand_same_def_reachs (opA, opA, opB))
	    continue;
	  if (L_live_outside_cb_after (cb, opB->dest[0], opB))
	    continue;
	  if (!L_all_uses_can_be_renamed (cb, opB, opB->dest[0]))
	    continue;
	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = L_general_load_opcode (opA);
	  store_flag = L_general_store_opcode (opA);
	  if ((load_flag || store_flag) && !L_can_merge_mems (cb, opA, opB))
	    break;
	  if (store_flag && !L_no_overlap_read (cb, opA, opA, 1, opB, 1))
	    break;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;

	  /*
	   *        Replace pattern no. 1
	   */
#if DEBUG_MERGE
	  fprintf (ERR, "MRG> 3 %d,%d into %d\n", opA->id, opB->id,
		   opA->id);
#endif
	  
	  if(load_flag && ((!L_no_br_between(opA, opB)) ||
			   (L_EXTRACT_BIT_VAL(opA->flags, L_OPER_MASK_PE)) ||
			   (L_EXTRACT_BIT_VAL(opB->flags, L_OPER_MASK_PE))))
	    {
	      /* Need to insert up to two checks */
	      if (!L_EXTRACT_BIT_VAL(opA->flags, L_OPER_MASK_PE))	       
		L_insert_check(cb, opA); /* Otherwise it should
					    already have necessary
					    checks in place */
	      
	      if (!L_EXTRACT_BIT_VAL(opB->flags, L_OPER_MASK_PE))
		L_insert_check(cb, opB); /* Otherwise it should
					    already have necessary
					    checks in place */
	      leave_marked_masked = 1;
	    }
	  else
	    {
	      leave_marked_masked = 0;
	    }
	  
	  L_promote_predicate (opA, def_opA->pred[0]);
	  
	  /* Unmask loads that aren't really executing more often than they
	     did before and assign checks for the deleted oper to the
	     promoted oper */
	  if (load_flag)
	    {
	      if (!leave_marked_masked)  
		{
		  opA->flags = L_CLR_BIT_FLAG (opA->flags, L_OPER_MASK_PE);
		  opA->flags = L_CLR_BIT_FLAG (opA->flags, L_OPER_PROMOTED);
		}
	      else
		{
		  L_assign_all_checks(L_fn, opB, opA);
		}
	    }

	  /* change all uses of opB->dest[0] to opA->dest[0] */
	  for (ptr = opB->next_op; ptr != NULL; ptr = ptr->next_op)
	    {
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (L_same_operand (ptr->src[i], opB->dest[0]))
		    {
		      L_delete_operand (ptr->src[i]);
		      ptr->src[i] = L_copy_operand (opA->dest[0]);
		    }
		}
	      if (L_is_dest_operand (opB->dest[0], ptr))
		break;
	    }

	  L_nullify_operation (opB);
	  change++;
	  if (!L_is_predicated (opA))
	    break;
	}
    }
  return (change);
}


static int
LB_hb_do_pred_merge4 (L_Cb * cb)
{
  int change = 0;
  L_Oper *opA, *opB, *nextB, *ptr;
  int i;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (L_is_predicated (opA))
	continue;
      if (L_pred_define_opcode (opA))
	continue;
      if (!L_is_register (opA->dest[0]))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;

	  /*
	   *        Match pattern no. 1 (op with destination, elim op B)
	   *        A:  tp op x = y,z
	   *        B   !p op x = y,z
	   */

	  if (!L_is_predicated (opB))
	    continue;
	  if (!L_same_opcode (opA, opB) || !L_same_compare (opA, opB))
	    continue;
	  if (!L_same_src_operands (opA, opB))
	    continue;
	  if (!L_same_dest_operands (opA, opB))
	    continue;

	  /* 04/20/03 SER Adding check: cannot merge a speculative and non-
	   *              speculative op. */
	  if (L_EXTRACT_BIT_VAL (opA->flags, L_OPER_SPECULATIVE))
	    {
	      if (!L_EXTRACT_BIT_VAL (opB->flags, L_OPER_SPECULATIVE))
	        continue;
	    }
	  else
	    if (L_EXTRACT_BIT_VAL (opB->flags, L_OPER_SPECULATIVE))
	      continue;

	  if (!L_all_dest_operand_no_defs_between (opA, opA, opB))
	    continue;
	  if (!L_all_src_operand_same_def_reachs (opA, opA, opB))
	    continue;
	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = L_general_load_opcode (opA);
	  store_flag = L_general_store_opcode (opA);
	  if ((load_flag || store_flag)
	      && !L_no_overlap_write (cb, opA, opA, 1, opB, 1))
	    break;
	  if (store_flag && !L_no_overlap_read (cb, opA, opA, 1, opB, 1))
	    break;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;

	  /* REPLACE */

#if DEBUG_MERGE
	  fprintf (ERR, "MRG> 4.1 %d,%d into %d\n", opA->id, opB->id,
		   opA->id);
#endif
	  if (load_flag || store_flag)
	    {
	      L_union_sync_arc_info (opA, opB);
	      L_merge_acc_spec_list (opA, opB);
	    }
	  L_nullify_operation (opB);
	  change++;
	  if (!L_is_predicated (opA))
	    break;
	}
    }

  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      if (L_is_predicated (opA) ||
	  L_pred_define_opcode (opA) ||
	  !L_is_register (opA->dest[0]))
	continue;

      for (opB = opA->next_op; opB; opB = nextB)
	{
	  int macro_flag, load_flag, store_flag;
	  nextB = opB->next_op;

	  /*
	   *  Match pattern no. 1 (op with destination, elim op B)
	   */
	  if (!L_is_predicated (opB))
	    continue;
	  if (!L_same_opcode (opA, opB) || !L_same_compare (opA, opB))
	    continue;
	  if (!L_same_src_operands (opA, opB))
	    continue;
	  if (L_same_dest_operands (opA, opB))
	    continue;		/* candidiate for early merging */
	  if (!L_is_register (opB->dest[0]))
	    continue;
	  if (!L_all_dest_operand_no_defs_between (opA, opA, cb->last_op))
	    continue;
	  if (!L_all_src_operand_same_def_reachs (opA, opA, opB))
	    continue;
	  if (L_live_outside_cb_after (cb, opB->dest[0], opB))
	    continue;
	  if (!L_all_uses_can_be_renamed (cb, opB, opB->dest[0]))
	    continue;
	  macro_flag = L_has_fragile_macro_operand (opA);
	  load_flag = L_general_load_opcode (opA);
	  store_flag = L_general_store_opcode (opA);
	  if ((load_flag || store_flag) &&
	      !L_no_overlap_write (cb, opA, opA, 1, opB, 1))
	    break;
	  if (store_flag && 
	      !L_no_overlap_read (cb, opA, opA, 1, opB, 1))
	    break;
	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;

	  /* REPLACE */

#if DEBUG_MERGE
	  fprintf (ERR, "MRG> 4.2 %d,%d into %d\n", opA->id, opB->id,
		   opA->id);
#endif

	  /* change all uses of opB->dest[0] to opA->dest[0] */
	  for (ptr = opB->next_op; ptr; ptr = ptr->next_op)
	    {
	      if (!PG_intersecting_predicates_ops (opB, ptr))
		continue;
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (L_same_operand (ptr->src[i], opB->dest[0]))
		    {
		      L_delete_operand (ptr->src[i]);
		      ptr->src[i] = L_copy_operand (opA->dest[0]);
		    }
		}
	      if (L_is_dest_operand (opB->dest[0], ptr))
		break;
	    }

	  L_nullify_operation (opB);
	  change++;
	  if (!L_is_predicated (opA))
	    break;
	}
    }
  return (change);
}

static int
LB_hb_do_migration_merging (L_Cb * cb)
{
  L_Oper *opA, *opB, *opC;
  L_Oper *next_opA, *next_opB, *next_op;
  L_Oper *def_opA, *def_opB;
  L_Oper *new_opA, *new_opB;
  L_Operand *dest;
  int change = 0;

  for (opA = cb->first_op; opA != NULL; opA = next_opA)
    {
      /*
       *       match pattern
       */
      next_opA = opA->next_op;
      if (!L_move_opcode (opA))
	continue;
      if (!L_is_predicated (opA))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = next_opB)
	{
	  /*
	   *  match pattern
	   */
	  int pass;
	  next_opB = opB->next_op;

	  if (!L_move_opcode (opB))
	    continue;
	  if (!L_is_predicated (opB))
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->dest[0]))
	    continue;

	  pass = 0;
	  def_opA = L_find_pred_definition (opA);
	  def_opB = L_find_pred_definition (opB);
	  if (!def_opA || !def_opB)
	    continue;

	  if (!PG_sum_predicates_ops (opA, opB, def_opA))
	    continue;

	  for (opC = opB->next_op; opC != NULL; opC = next_op)
	    {
	      int i, src_index, load_flag, store_flag, found, macro_flag;

	      next_op = opC->next_op;

	      if (L_general_branch_opcode (opC))
		continue;

	      if (L_same_operand (opA->dest[0], opC->src[0]))
		src_index = 0;
	      else if (L_same_operand (opA->dest[0], opC->src[1]))
		src_index = 1;
	      else if ((opC->src[2])
		       && (L_same_operand (opA->dest[0], opC->src[2])))
		src_index = 2;
	      else
		continue;

	      /* Check for other uses of opA->dest and opB->dest */

	      if (!L_all_dest_operand_not_live_outside_cb_between
		  (cb, opA, opA, opC))
		continue;

	      if (!L_no_uses_between (opA->dest[0], opA, opC))
		continue;

	      if (!L_all_dest_operand_not_live_outside_cb_between
		  (cb, opB, opB, opC))
		continue;

	      if (!L_no_uses_between (opB->dest[0], opB, opC))
		continue;

	      macro_flag = L_has_fragile_macro_operand (opC);
	      load_flag = L_general_load_opcode (opC);
	      store_flag = L_general_store_opcode (opC);

	      if (load_flag || store_flag)
		{
		  if (!L_no_overlap_write (cb, opC, opA, 1, opC, 1))
		    break;
		  if (store_flag
		      && !L_no_overlap_read (cb, opC, opA, 1, opC, 1))
		    break;
		  if (!L_no_danger
		      (macro_flag, load_flag, store_flag, opA, opC))
		    break;
		  if (!L_no_overlap_write (cb, opC, opB, 1, opC, 1))
		    break;
		  if (store_flag
		      && !L_no_overlap_read (cb, opC, opB, 1, opC, 1))
		    break;
		  if (!L_no_danger
		      (macro_flag, load_flag, store_flag, opB, opC))
		    break;
		}

	      /* Check if opC destination is used between opA->opC, opB->opC */
	      if (load_flag || !store_flag)
		{
		  if (!L_no_uses_between (opC->dest[0], opA, opC))
		    continue;
		  if (!L_no_uses_between (opC->dest[0], opB, opC))
		    continue;
		}

	      if (!L_can_merge_with_op_above (cb, opA, opC))
		break;
	      if (!L_can_merge_with_op_above (cb, opB, opC))
		break;

	      /* Predicate conditioning */
	      if (!PG_equivalent_predicates_ops (def_opA, opC))
		continue;
	      if (!PG_equivalent_predicates_ops (def_opB, opC))
		continue;
	      found = 0;
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (!L_no_defs_between (opC->src[i], opA, opB))
		    {
		      found = 1;
		      break;
		    }

		  if (!L_no_defs_between (opC->src[i], opB, opC))
		    {
		      found = 1;
		      break;
		    }
		}

	      if (found)
		break;

#if DEBUG_MERGE
	      fprintf (ERR,
		       "MRG> migration/demotion %d,%d and %d,%d\n",
		       opC->id, opA->id, opC->id, opB->id);
#endif

	      /* Migrate/Demote operation C into operation A and B */
	      new_opA = L_copy_operation (opC);
	      L_delete_operand (new_opA->src[src_index]);
	      L_delete_operand (new_opA->pred[0]);
	      L_delete_operand (new_opA->pred[1]);
	      for (i = 0; i < L_max_pred_operand; i++)
		new_opA->pred[i] = L_copy_operand (opA->pred[i]);
	      new_opA->src[src_index] = L_copy_operand (opA->src[0]);
	      L_insert_oper_after (cb, opA, new_opA);


	      new_opB = L_copy_operation (opC);
	      L_delete_operand (new_opB->src[src_index]);
	      L_delete_operand (new_opB->pred[0]);
	      L_delete_operand (new_opB->pred[1]);
	      for (i = 0; i < L_max_pred_operand; i++)
		new_opB->pred[i] = L_copy_operand (opB->pred[i]);
	      new_opB->src[src_index] = L_copy_operand (opB->src[0]);
	      L_insert_oper_after (cb, opB, new_opB);

	      L_nullify_operation (opC);

	      change++;
	      pass = 1;
	      break;
	    }

	  if (pass)
	    break;
	}
    }

  for (opA = cb->first_op; opA != NULL; opA = next_opA)
    {
      /*
       *       match pattern
       */
      next_opA = opA->next_op;

      dest = opA->dest[0];

      if (!L_is_predicated (opA))
	continue;
      if (!L_is_variable (dest))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = next_opB)
	{
	  /*
	   *  match pattern
	   */
	  int pass;

	  next_opB = opB->next_op;

	  if (!L_is_predicated (opB))
	    continue;
	  dest = opB->dest[0];
	  if (!L_is_variable (dest))
	    continue;

	  if (!L_same_operand (opA->dest[0], opB->dest[0]))
	    continue;

	  pass = 0;
	  def_opA = L_find_pred_definition (opA);
	  def_opB = L_find_pred_definition (opB);
	  if (!def_opA || !def_opB)
	    continue;
	  if (!PG_sum_predicates_ops (opA, opB, def_opA))
	    continue;

	  for (opC = opB->next_op; opC != NULL; opC = next_op)
	    {
	      int i, load_flag, store_flag, macro_flag;

	      next_op = opC->next_op;

	      if (!L_move_opcode (opC))
		continue;

	      if (!L_same_operand (opA->dest[0], opC->src[0]))
		continue;

	      /* Check whether there are other uses of opA->dest and opB->dest */
	      if (!L_all_dest_operand_not_live_outside_cb_between
		  (cb, opA, opA, opC))
		continue;

	      if (!L_no_uses_between (opA->dest[0], opA, opC))
		continue;

	      if (!L_no_uses_in_range (opA->dest[0], opA, opA))
		continue;

	      if (!L_all_dest_operand_not_live_outside_cb_between
		  (cb, opB, opB, opC))
		continue;

	      if (!L_no_uses_between (opB->dest[0], opB, opC))
		continue;

	      if (!L_no_uses_in_range (opB->dest[0], opB, opB))
		continue;

	      macro_flag = L_has_fragile_macro_operand (opC);
	      load_flag = L_general_load_opcode (opC);
	      store_flag = L_general_store_opcode (opC);

	      if (load_flag || store_flag)
		{
		  if (!L_no_overlap_write (cb, opC, opA, 1, opC, 1))
		    break;
		  if (store_flag
		      && !L_no_overlap_read (cb, opC, opA, 1, opC, 1))
		    break;
		  if (!L_no_danger
		      (macro_flag, load_flag, store_flag, opA, opC))
		    break;
		  if (!L_no_overlap_write (cb, opC, opB, 1, opC, 1))
		    break;
		  if (store_flag
		      && !L_no_overlap_read (cb, opC, opB, 1, opC, 1))
		    break;
		  if (!L_no_danger
		      (macro_flag, load_flag, store_flag, opB, opC))
		    break;
		}

	      /* Check if opC destination is used between opA->opC, opB->opC */
	      if (load_flag || !store_flag)
		{
		  if (!L_no_uses_between (opC->dest[0], opA, opC))
		    continue;
		  if (!L_no_uses_between (opC->dest[0], opB, opC))
		    continue;
		}

	      if (!L_can_merge_with_op_below (cb, opC, opA))
		break;
	      if (!L_can_merge_with_op_below (cb, opC, opB))
		break;

	      /* Predicate conditioning */
	      if (!PG_equivalent_predicates_ops (def_opA, opC))
		continue;
	      if (!PG_equivalent_predicates_ops (def_opB, opC))
		continue;

	      if (!L_no_defs_between (opC->src[0], opA, opB))
		break;

	      if (!L_no_defs_between (opC->src[0], opB, opC))
		break;

#if DEBUG_MERGE
	      fprintf (ERR,
		       "MRG> inverted migration/demotion %d,%d and %d,%d\n",
		       opC->id, opA->id, opC->id, opB->id);
#endif

	      /* Migrate/Demote operation C into operation A and B */
	      new_opA = L_copy_operation (opA);
	      L_delete_operand (new_opA->dest[0]);
	      L_delete_operand (new_opA->pred[0]);
	      L_delete_operand (new_opA->pred[1]);
	      for (i = 0; i < L_max_pred_operand; i++)
		new_opA->pred[i] = L_copy_operand (opA->pred[i]);
	      new_opA->dest[0] = L_copy_operand (opC->dest[0]);
	      L_insert_oper_before (cb, opC, new_opA);


	      /* Migrate/Demote operation C into operation A and B */
	      new_opB = L_copy_operation (opB);
	      L_delete_operand (new_opB->dest[0]);
	      L_delete_operand (new_opB->pred[0]);
	      L_delete_operand (new_opB->pred[1]);
	      for (i = 0; i < L_max_pred_operand; i++)
		new_opB->pred[i] = L_copy_operand (opB->pred[i]);
	      new_opB->dest[0] = L_copy_operand (opC->dest[0]);
	      L_insert_oper_before (cb, opC, new_opB);

	      /* Remove original operation opC */
	      L_nullify_operation (opC);

	      change++;
	      pass = 1;
	      break;
	    }

	  if (pass)
	    break;
	}
    }

  return (change);
}

void
LB_hb_pred_merging (L_Func * fn)
{
  int i, change, merge1, merge2, merge3, merge4;
  int migration_merge;
  L_Cb *cb;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;

      for (i = 0; i < L_MAX_ITERATION; i++)
	{
	  merge1 = merge2 = merge3 = merge4 = 0;

#if DO_MERGES
	  LB_hb_do_pred_mergeS (cb);
#endif
#if DO_MERGE1
	  merge1 = LB_hb_do_pred_merge1 (cb);
#endif
#if DO_MERGE2
	  merge2 = LB_hb_do_pred_merge2 (cb);
#endif
#if DO_MERGE3
	  merge3 = LB_hb_do_pred_merge3 (cb);
#endif
#if DO_MERGE4
	  merge4 = LB_hb_do_pred_merge4 (cb);
#endif
	  change = merge1 + merge2 + merge3 + merge4;

	  if (!change)
	    break;
	}
      /* Migration merge creates new opers, so it invalidates
       * reaching def.  Therefore it cannot be included in the
       * above iteration.
       */
    }

#if DO_MIGRATION_MERGE
  PG_setup_pred_graph (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;

      migration_merge = LB_hb_do_migration_merging (cb);
    }
#endif
  return;
}

#if DO_MERGES
static int
LB_hb_do_pred_mergeS (L_Cb *cb)
{
  L_Oper *opA, *opAA, *opAD, *opAnxt, *opB, *opBB;
  L_Operand *dest;
  int i;
  Set defssa = NULL;

  PG_Pred_SSA *pssa;

  for (opA = cb->first_op; opA; opA = opAnxt)
    {
      opAnxt = opA->next_op;
      if (!L_is_predicated (opA))
	continue;

      opBB = NULL;

      opAA = opA;
      while (opAA->next_op &&
	     PG_equivalent_predicates_ops (opA, opAA->next_op))
	opAA = opAA->next_op;

      opAD = opA->pred[0]->value.pred.ssa->oper;

      opAnxt = opAA->next_op;

      opB = opAA->next_op;

      if (!L_is_predicated (opB) || !opAD ||
	  !PG_rel_complementary_predicates_ops (opA, opB, opAD))
	continue;

      opBB = opB;
      while (opBB->next_op &&
	     PG_equivalent_predicates_ops (opB, opBB->next_op))
	opBB = opBB->next_op;
      
      opAnxt = opBB->next_op;

      /* Now we have successive groups of instructions
       * [opA,opAA], [opB,opBB] on complementary predicates 
       */

      printf ("MRG> cb %d triad of p%d --> (p%d p%d)\n",
	      cb->id,
	      opAD->pred[0] ? opAD->pred[0]->value.pred.reg : 0,
	      opA->pred[0]->value.pred.reg,
	      opB->pred[0]->value.pred.reg);

      {
	List Alist, Blist;

	
      }

    }
  return 0;
}
#endif




