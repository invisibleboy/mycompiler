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
 *      File :          l_PCE_opti.c
 *      Author:         Shane Ryoo, Wen-mei Hwu
 *      Description :   partial code elimination functions
 *      Creation Date : September, 2002 
 *==========================================================================*/

#include <config.h>
#include "l_opti.h"

#define MAX_PCE_LOCAL_CLEANUP_ITER 5
#define MAX_PDE_ITER 10

#undef DEBUG_SPLIT_CRITICAL_EDGES
#undef DEBUG_MERGE_SAME_CBS

#undef DEBUG_LAZY_CODE_MOTION
#undef DEBUG_SPECULATIVE_CODE_MOTION
#undef TRACK_EXPRESSION_WEIGHT
#undef DEBUG_PCE_REACH

#undef DEBUG_PDE_MOTION
#undef TRACK_PDE_ITERATIONS
#define P3DE_GENERAL_STORE_MOTION
#undef DEBUG_PDE_COMBINE_PRED_GUARDS

extern void L_do_SPRE_analysis (L_Func *, int, int);
extern void L_do_P3DE_analysis (L_Func *, int, int);

/*
 * L_split_fn_critical_edges
 * ------------------------------
 * Splits critical edges so that partial redundancy elimination can be 
 * performed.
 * A critical edge/arc is one which leaves a basic block with two or more 
 * outgoing edges and enters a block also with two or more outgoing edges. 
 * Doing so makes sure that code motion is not blocked during partial
 * redundancy elimination.
 */
static int
L_split_cb_critical_edges (L_Cb * cb)
{
  L_Cb *dst_cb, *new_cb;
  L_Oper *op;
  L_Flow *flow, *flow2;
  int change = 0;

  flow = cb->dest_flow;

  /* If no outbound flows (last cb or dead code) */
  if (flow == NULL)
    return 0;

  /* If there is only one outbound flow, skip: don't need to split. */
  if (flow->next_flow == NULL)
    return 0;

  /* Otherwise, there are multiple outbound flows */
  for (; flow; flow = flow->next_flow)
    {
      /* The one case we can ignore is an unconditional branch at
       * the end of the cb preceded by non-control-opers, since it
       * is a basic block with only one exit
       */
      op = L_find_branch_for_flow (cb, flow);
      if (op == cb->last_op && L_uncond_branch_opcode (op))
	if (!L_is_control_oper (op->prev_op))
	  break;		/* last eligible flow */

      dst_cb = flow->dst_cb;
      flow2 = L_find_matching_flow (dst_cb->src_flow, flow);

      /* Check for multiple flows at dest cb, skip if only one or if
       * not epiloque. */
      if ((flow2->prev_flow == NULL) && (flow2->next_flow == NULL)
	  && (!(dst_cb->first_op) || (dst_cb->first_op->opc != Lop_EPILOGUE)))
	continue;

      new_cb = L_split_arc (L_fn, cb, flow);
#ifdef DEBUG_SPLIT_CRITICAL_EDGES
      fprintf (stderr, "Created cb %d in function %s.\n", new_cb->id,
	       L_fn->name);
      fprintf (stderr, "\tSplits arc between cb %d and cb %d.\n",
	       cb->id, dst_cb->id);
#endif
      change++;
    }

  STAT_COUNT ("Lopti_PCE_split_critical_edges", change, NULL);
  return change;
}

int
L_split_fn_critical_edges (L_Func * fn)
{
  int change;
  L_Cb *cb;

  change = 0;

  if (!Lopti_do_PCE_split_critical_edges)
    L_punt ("PCE currently not set up to operate correctly without splitting "
	    "critical edges.");

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    change += L_split_cb_critical_edges (cb);

  return change;
}


/*
 * L_split_fn_loop_out_critical_edges
 * ------------------------------
 * In order to prevent ping-ponging optimizations between partial dead
 * code elimination's assignment sinking and loop-invariant code motion,
 * critical edges out of a loop must be split in order to allow code motion
 * of ops before a loop through the loop.
 * NOTE: This splits ALL edges in an exit cb, to be clean about it we
 * should split only those that actually exit the loop.
 */

int
L_split_fn_loop_out_critical_edges (L_Func * fn)
{
  int i, num_exit_cb = 0, *exit_cb = NULL, change;
  L_Loop *loop;
  L_Cb *cb;

  L_loop_detection (fn, 1);

  change = 0;
  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    {
      if ((num_exit_cb = Set_size (loop->exit_cb)))
	{
	  exit_cb = (int *) malloc (sizeof (int) * num_exit_cb);
	  Set_2array (loop->exit_cb, exit_cb);
	}

      for (i = 0; i < num_exit_cb; i++)
	{
	  if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, exit_cb[i])))
	    L_punt
	      ("L_split_loop_out_critical_edges: could not find exit cb %d.\n",
	       exit_cb[i]);
	  change += L_split_cb_critical_edges (cb);
	}
    }
  if (num_exit_cb)
    free (exit_cb);
  return change;
}


/*
 * L_split_fn_loopback_critical_edges
 * ------------------------------
 * The dataflow for partial redundancy elimination breaks when dealing with
 * loopback branches, specifically cbs that loop back to themselves, because
 * the correct analysis is NOT monotonic (if maintaining one set of data).
 * All cbs containing loopback edges must have their edges split.
 * NOTE: This splits ALL edges in a loopback cb, to be clean about it we
 * should split only those that actually loop back.
 */

int
L_split_fn_loopback_critical_edges (L_Func * fn)
{
  int i, num_back_edge_cb = 0, *back_edge_cb = NULL, change;
  L_Loop *loop;
  L_Cb *cb;

  change = 0;
  for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
    {
      if ((num_back_edge_cb = Set_size (loop->back_edge_cb)))
	{
	  back_edge_cb = (int *) malloc (sizeof (int) * num_back_edge_cb);
	  Set_2array (loop->back_edge_cb, back_edge_cb);
	}

      for (i = 0; i < num_back_edge_cb; i++)
	{
	  if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, back_edge_cb[i])))
	    L_punt
	      ("L_split_loop_out_critical_edges: could not find backedge cb %d.\n",
	       back_edge_cb[i]);
	  change += L_split_cb_critical_edges (cb);
	}
    }
  if (num_back_edge_cb)
    free (back_edge_cb);
  return change;
}


/*
 * PARTIAL REDUNDANCY ELIMINATION FUNCTIONS
 * ----------------------------------------
 */

static L_Oper *
L_PRE_n_insert_replace (int expression_index, L_Cb * cb, L_Oper * first_op,
			Set * oper_insert)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *expression;
  L_Oper *orig_op, *new_op;
  L_Operand *expression_operand;
  int match = 0;

  /* Find expression */
  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    expression_index);
  expression = entry->expression;

  for (orig_op = first_op; orig_op; orig_op = orig_op->next_op)
    {
      if (L_is_control_oper (orig_op) || L_cond_ret (orig_op))
	break;
      if (L_EXTRACT_BIT_VAL (orig_op->flags, L_OPER_VOLATILE))
	continue;
      if (orig_op->src[0] == NULL || orig_op->opc == Lop_NO_OP)
	continue;

      if (L_oper_matches_expression (orig_op, 0, expression, 0, 0))
	{
	  if (!(L_move_opcode (expression) && L_is_macro (orig_op->dest[0])))
	    {
	      match = 1;
	      break;
	    }
	}
    }
  if (!match)
    L_punt ("L_PRE_n_insert_replace: no matching oper found for expression "
	    "%d in cb %d!", expression_index, cb->id);

#ifdef DEBUG_LAZY_CODE_MOTION
  fprintf (stderr,
	   "N_Insert_Replacing expression %d at op %d in cb %d.\n",
	   expression_index, orig_op->id, cb->id);
#endif

  if (!L_move_opcode (orig_op))
    *oper_insert = Set_add (*oper_insert, orig_op->id);

  if (L_general_store_opcode (expression))
    return orig_op;

  /* Create & insert move */
  expression_operand =
    L_create_operand_for_expression_index (expression_index);

  /* normal case: expression writes to temp var, move from var inserted */
  new_op = L_create_move (orig_op->dest[0], expression_operand);
  expression_operand =
    L_create_operand_for_expression_index (expression_index);
  /* don't delete dest of orig op: used in move */
  orig_op->dest[0] = expression_operand;
  L_insert_oper_after (cb, orig_op, new_op);
  if (L_EXTRACT_BIT_VAL (orig_op->flags, L_OPER_SPECULATIVE))
    new_op->flags = L_SET_BIT_FLAG (new_op->flags, L_OPER_SPECULATIVE);

  /* Predicate fixup */
  new_op->pred[0] = orig_op->pred[0];
  new_op->pred[1] = orig_op->pred[1];
  orig_op->pred[0] = NULL;
  orig_op->pred[1] = NULL;

  return new_op;
}


static L_Oper *
L_PRE_x_insert_replace (int expression_index, L_Cb * cb, L_Oper * last_op,
			Set * oper_insert, Set * oper_new_insert,
			int complement_flag)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *expression;
  L_Oper *orig_op, *new_op;
  L_Operand *expression_operand;
  int match = 0;

  /* Find expression */
  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    expression_index);
  expression = entry->expression;

  for (orig_op = last_op; orig_op; orig_op = orig_op->prev_op)
    {
      if (orig_op->src[0] == NULL || orig_op->opc == Lop_NO_OP)
	continue;
      if (L_EXTRACT_BIT_VAL (orig_op->flags, L_OPER_VOLATILE))
	continue;
      if (L_oper_matches_expression (orig_op, 0, expression, 0,
				     (complement_flag ? expression->opc : 0)))
	{
	  if (!(L_move_opcode (expression) && L_is_macro (orig_op->dest[0])))
	    {
	      match = 1;
	      break;
	    }
	}

      if (L_is_control_oper (orig_op->prev_op) ||
	  L_cond_ret (orig_op->prev_op))
	break;
    }
  if (!match)
    L_punt
      ("L_PRE_x_insert_replace: no matching oper found for expression %d "
       "in cb %d!", expression_index, cb->id);

#ifdef DEBUG_LAZY_CODE_MOTION
  fprintf (stderr,
	   "X_Insert_Replacing expression %d at op %d in cb %d.\n",
	   expression_index, orig_op->id, cb->id);
#endif

  if (L_general_store_opcode (expression))
    {
      *oper_insert = Set_add (*oper_insert, orig_op->id);
      return orig_op;
    }

  /* Create & insert move */
  if (!complement_flag)
    {
      expression_operand =
	L_create_operand_for_expression_index (expression_index);
      new_op = L_create_move (orig_op->dest[0], expression_operand);
      expression_operand =
	L_create_operand_for_expression_index (expression_index);
      /* don't delete dest of orig_op: used in move */
      orig_op->dest[0] = expression_operand;
      L_insert_oper_after (cb, orig_op, new_op);
      if (!L_move_opcode (orig_op))
	*oper_insert = Set_add (*oper_insert, orig_op->id);

      if (L_EXTRACT_BIT_VAL (orig_op->flags, L_OPER_SPECULATIVE))
	new_op->flags = L_SET_BIT_FLAG (new_op->flags, L_OPER_SPECULATIVE);

      /* Predicate fixup */
      new_op->pred[0] = orig_op->pred[0];
      new_op->pred[1] = orig_op->pred[1];
      orig_op->pred[0] = NULL;
      orig_op->pred[1] = NULL;
    }
  /* complement case: insert loads */
  else
    {
      /* Must always insert loads, since stores can be killed by JSRs but
       * the corresponding loads may not. */
      new_op = L_generate_oper_from_expression_index (expression->index);
      L_insert_oper_after (cb, orig_op, new_op);
      *oper_new_insert = Set_add (*oper_new_insert, new_op->id);
      STAT_COUNT ("Lopti_PRE_mem_copy_prop", 1, cb);
    }

  return new_op;
}

static L_Oper *
L_PRE_n_insert (int expression_index, L_Cb * cb, L_Oper * first_op,
		Set * oper_insert, int speculate_flag)
{
  L_Oper *new_op;
#ifdef TRACK_EXPRESSION_WEIGHT
  L_Expression_Hash_Entry *entry;
#endif

#ifdef DEBUG_LAZY_CODE_MOTION
  fprintf (stderr, "N_Inserting PRE computation %d at op %d in cb %d.\n",
	   expression_index, first_op ? first_op->id : 0, cb->id);
#endif
  fprintf (stderr, "L_PRE_n_insert shouldn't be called...notify SER.\n");

  new_op = L_generate_oper_from_expression_index (expression_index);

  L_insert_oper_before (cb, first_op, new_op);

  /* move expressions in PRE are never speculative: from constants */
  if (!L_move_opcode (new_op))
    {
      *oper_insert = Set_add (*oper_insert, new_op->id);

      if (speculate_flag)
	{
	  new_op->flags = L_SET_BIT_FLAG (new_op->flags, L_OPER_SPECULATIVE);
	  STAT_COUNT ("Lopti_PRE_speculative", 1, cb);
	  if (L_mask_potential_exceptions && L_is_pei (new_op))
	    {
	      new_op->flags = L_SET_BIT_FLAG (new_op->flags, L_OPER_MASK_PE);
	      STAT_COUNT ("Lopti_PRE_mask_pe", 1, cb);
#ifdef DEBUG_LAZY_CODE_MOTION
	      if (L_general_store_opcode (new_op))
		L_punt ("L_PRE_n_insert: Stores should not be speculated!");
#endif
	    }
	}
    }

#ifdef TRACK_EXPRESSION_WEIGHT
  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    expression_index);
  if (new_op->next_op)
    new_op->weight = new_op->next_op->weight;
  else
    new_op->weight = cb->weight;
  entry->expression->weight += new_op->weight;
#endif

  return new_op;
}


static L_Oper *
L_PRE_x_insert (int expression_index, L_Cb * cb, L_Oper * last_op,
		Set * oper_insert, int speculate_flag)
{
  L_Oper *new_op;
#ifdef TRACK_EXPRESSION_WEIGHT
  L_Expression_Hash_Entry *entry;
#endif

#ifdef DEBUG_LAZY_CODE_MOTION
  fprintf (stderr, "X_Inserting expression %d at op %d in cb %d.\n",
	   expression_index, last_op ? last_op->id : 0, cb->id);
#endif

  new_op = L_generate_oper_from_expression_index (expression_index);

  if ((L_is_control_oper (last_op) && !L_sync_opcode (last_op)) ||
      L_cond_ret (last_op))
    L_insert_oper_before (cb, last_op, new_op);
  else
    L_insert_oper_after (cb, last_op, new_op);

  /* move expressions in PRE are never speculative */
  if (!L_move_opcode (new_op))
    {
      *oper_insert = Set_add (*oper_insert, new_op->id);

      if (speculate_flag)
	{
	  new_op->flags = L_SET_BIT_FLAG (new_op->flags, L_OPER_SPECULATIVE);
	  STAT_COUNT ("Lopti_PRE_speculative", 1, cb);
	  if (L_mask_potential_exceptions && L_is_pei (new_op))
	    {
	      new_op->flags = L_SET_BIT_FLAG (new_op->flags, L_OPER_MASK_PE);
	      STAT_COUNT ("Lopti_PRE_mask_pe", 1, cb);
#ifdef DEBUG_LAZY_CODE_MOTION
	      if (L_general_store_opcode (new_op))
		L_punt ("L_PRE_x_insert: Stores should not be speculated!");
#endif
	    }
	}
    }

#ifdef TRACK_EXPRESSION_WEIGHT
  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    expression_index);
  if (new_op->next_op)
    new_op->weight = new_op->next_op->weight;
  else if (new_op->prev_op)
    new_op->weight = new_op->prev_op->weight;
  else
    new_op->weight = cb->weight;
  entry->expression->weight += new_op->weight;
#endif

  return new_op;
}


static L_Oper *
L_PRE_n_replace (int expression_index, L_Cb * cb, L_Oper * first_op,
		 Set * oper_replace)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *expression;
  L_Oper *replace_op;
  L_Operand *src_operand;
  int match = 0;

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    expression_index);
  expression = entry->expression;

  for (replace_op = first_op; replace_op; replace_op = replace_op->next_op)
    {
      if (L_is_control_oper (replace_op) || L_cond_ret (replace_op))
	break;
      if (L_EXTRACT_BIT_VAL (replace_op->flags, L_OPER_VOLATILE))
	continue;
      if (replace_op->src[0] == NULL || replace_op->opc == Lop_NO_OP)
	continue;

      if (L_oper_matches_expression (replace_op, 0, expression, 0, 0))
	{
	  if (!(L_move_opcode (expression) &&
		L_is_macro (replace_op->dest[0])))
	    {
	      match = 1;
	      break;
	    }
	}
    }
  if (!match)
    L_punt ("L_PRE_n_replace: no matching oper found for expression %d in "
	    "cb %d!", expression_index, cb->id);

#ifdef DEBUG_LAZY_CODE_MOTION
  fprintf (stderr, "N_Replacing expression %d at op %d in cb %d.\n",
	   expression_index, replace_op->id, cb->id);
#endif

#ifdef TRACK_EXPRESSION_WEIGHT
  expression->weight -= replace_op->weight;
#endif

  if (L_EXTRACT_BIT_VAL (replace_op->flags, L_OPER_SPECULATIVE) ||
      L_general_load_opcode (replace_op) ||
      L_general_store_opcode (replace_op))
    {
      *oper_replace = Set_add (*oper_replace, replace_op->id);
    }
  else
    {
      if (L_move_opcode (replace_op))
	STAT_COUNT ("Lopti_PRE_move", 1, cb);

      src_operand = L_create_operand_for_expression_index (expression_index);
      L_convert_to_move (replace_op, L_copy_operand (replace_op->dest[0]),
			 src_operand);
    }

  return replace_op;
}


#if 0
/* SER: Note that L_PRE_x_replace should never be called...anything that
 * is x_replace in PRE must be an x_insert_replace with our dataflow. */
static L_Oper *
L_PRE_x_replace (int expression_index, L_Cb * cb, L_Oper * last_op,
		 Set * oper_replace)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *expression;
  L_Oper *replace_op;
  L_Operand *src_operand;
  int i, match = 0;

#ifdef DEBUG_LAZY_CODE_MOTION
  fprintf (stderr, "X_Replacing expression %d at op %d in cb %d.\n",
	   expression_index, last_op ? last_op->id : 0, cb->id);
#endif
  fprintf (stderr, "L_PRE_x_replace shouldn't be called...notify SER.\n");

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    expression_index);
  expression = entry->expression;

  for (replace_op = last_op; replace_op; replace_op = replace_op->prev_op)
    {
      if (replace_op->src[0] == NULL || replace_op->opc == Lop_NO_OP)
	continue;
      if (L_EXTRACT_BIT_VAL (replace_op->flags, L_OPER_VOLATILE))
	continue;

      if (L_oper_matches_expression (replace_op, 0, expression, 0, 0))
	{
	  if (!(L_move_opcode (expression) &&
		L_is_macro (replace_op->dest[0])))
	    {
	      match = 1;
	      break;
	    }
	}

      if (L_is_control_oper (replace_op->prev_op) ||
	  L_cond_ret (replace_op->prev_op))
	break;
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (!(expression->src[i]))
	    break;
	  if (L_is_dest_operand (expression->src[i], replace_op))
	    break;
	}
    }
  if (!match)
    L_punt
      ("L_PRE_n_replace: no matching oper found for expressoin %d in cb %d!",
       expression_index, cb->id);

#ifdef TRACK_EXPRESSION_WEIGHT
  expression->weight -= replace_op->weight;
#endif

  if (L_EXTRACT_BIT_VAL (replace_op->flags, L_OPER_SPECULATIVE) ||
      L_general_load_opcode (replace_op) ||
      L_general_store_opcode (replace_op))
    {
      *oper_replace = Set_add (*oper_replace, replace_op->id);
    }
  else
    {
      src_operand = L_create_operand_for_expression_index (expression_index);
      L_convert_to_move (replace_op, L_copy_operand (replace_op->dest[0]),
			 src_operand);
    }
  return replace_op;
}
#endif


static void
L_PRE_correct_spec_and_sync_info (Set * all_insert, Set * new_insert,
				  Set * replace)
{
  L_Cb *replace_cb, *insert_cb;
  L_Oper *replace_op, *insert_op, **inserted_ops;
  L_Operand *src_operand;
  L_Loop *replace_loop, *insert_loop;
  L_Expression *expression;
  L_Attr * attr;
  int *replace_array, *insert_array, replace_size, insert_size = 0,
    new_insert_size = 0, num_inserted_ops, i, j, k, token;
  Set reaching, safe_pei_ops, useless;

  replace_array = (int *) Lcode_malloc (sizeof (int) * L_fn->n_oper);
  insert_array = (int *) Lcode_malloc (sizeof (int) * L_fn->n_oper);
  replace_size = Set_2array (*replace, replace_array);
  inserted_ops = (L_Oper **) Lcode_malloc (sizeof (L_Oper *) * L_fn->n_oper);
  new_insert_size = Set_2array (*new_insert, insert_array);
  useless = Set_copy (*new_insert);

  if (new_insert_size && replace_size)
    {
      safe_pei_ops = Set_copy (*new_insert);

      /* First go through all things that need to be changed for new insert ops
         (propagation of sync info, speculative/mask potential exception and
         other flags). */
      for (i = 0; i < replace_size; i++)
	{
	  replace_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						  replace_array[i]);
	  replace_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
						replace_array[i]);
	  reaching = L_get_oper_RIN_set (replace_op);

	  for (j = 0; j < new_insert_size; j++)
	    {
	      if (!(Set_in (reaching, insert_array[j])))
		continue;
	      insert_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						     insert_array[j]);
	      if (!(L_opers_same_expression (replace_op, insert_op)))
		continue;
	      useless = Set_delete (useless, insert_array[j]);
	      insert_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
						   insert_array[j]);

	      /* Migrate flags, sync info */
	      /* if any of an insert_op's replace_ops is speculative or 
	       * masks pe's, must be the same for the insert_op */
	      if (L_EXTRACT_BIT_VAL (replace_op->flags, L_OPER_SPECULATIVE))
		{
		  if (!L_EXTRACT_BIT_VAL
		      (insert_op->flags, L_OPER_SPECULATIVE))
		    STAT_COUNT ("Lopti_PRE_speculative", 1, insert_cb);
		  insert_op->flags =
		    L_SET_BIT_FLAG (insert_op->flags, L_OPER_SPECULATIVE);
		}
	      if (!(insert_op->attr) && replace_op->attr)
		insert_op->attr = L_copy_attr (replace_op->attr);

	      /* At this point, if not a load or store op, can cut out. */
	      if (!(L_general_load_opcode (replace_op) ||
		    L_general_store_opcode (replace_op)))
		continue;
	      if (L_EXTRACT_BIT_VAL (replace_op->flags, L_OPER_MASK_PE))
		{
		  if (!L_EXTRACT_BIT_VAL (insert_op->flags, L_OPER_MASK_PE))
		    STAT_COUNT ("Lopti_PRE_mask_pe", 1, insert_cb);
		  insert_op->flags =
		    L_SET_BIT_FLAG (insert_op->flags, L_OPER_MASK_PE);
		}
	      if (L_EXTRACT_BIT_VAL (replace_op->flags,
				     L_OPER_LABEL_REFERENCE))
		insert_op->flags =
		  L_SET_BIT_FLAG (insert_op->flags, L_OPER_LABEL_REFERENCE);
	      if (L_EXTRACT_BIT_VAL (replace_op->flags,
				     L_OPER_STACK_REFERENCE))
		{
		  insert_op->flags =
		    L_SET_BIT_FLAG (insert_op->flags, L_OPER_STACK_REFERENCE);
		  if (!(attr = L_find_attr (insert_op->attr, STACK_ATTR_NAME)))
		    {
		      attr = L_find_attr (replace_op->attr, STACK_ATTR_NAME);
		      if (attr)
			insert_op->attr =
			  L_concat_attr (insert_op->attr,
					 L_copy_attr_element (attr));
		    }
		}
	      if (!L_EXTRACT_BIT_VAL (replace_op->flags, L_OPER_SAFE_PEI))
		safe_pei_ops = Set_delete (safe_pei_ops, insert_op->id);

	      if (L_func_contains_dep_pragmas &&
		  !(insert_op->sync_info) && (replace_op->sync_info))
		{
		  insert_op->sync_info =
		    L_copy_sync_info (replace_op->sync_info);
		  L_insert_all_syncs_in_dep_opers (insert_op);
		  replace_loop = replace_cb->deepest_loop;
		  insert_loop = insert_cb->deepest_loop;
		  if (replace_loop && insert_loop &&
		      replace_loop->nesting_level >
		      insert_loop->nesting_level)
		    {
#ifdef DEBUG_PCE_REACH
		      fprintf (stderr,
			       "Adjusting syncs of op %d for movement "
			       "out of loop.\n", insert_op->id);
#endif
		      L_adjust_syncs_for_movement_out_of_loop (insert_op,
							       insert_cb);
		    }
		  /* This sync arc update is too pessimistic...
		     but may have to enable at some point. */
		  /* L_update_sync_arcs_for_new_cb (replace_cb, insert_cb, 
		     insert_op); */

#ifdef DEBUG_PCE_REACH
		  fprintf (stderr, "Sync info migrated from replace_op %d to "
			   "op %d.\n", replace_op->id, insert_op->id);
#endif
		}

	      if (replace_op->acc_info && !insert_op->acc_info)
		{
		  insert_op->acc_info = 
		    L_copy_mem_acc_spec_list (replace_op->acc_info);
		}
	    }
	}

      /* Mark SAFE_PEI ops */
      insert_size = Set_2array (safe_pei_ops, insert_array);
      for (i = 0; i < insert_size; i++)
	{
	  insert_op =
	    L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, insert_array[i]);
	  /* SER 6/26/03: It is possible for ops to be deleted in dataflow
	   * setup when they are not reachable. */
	  if (!insert_op)
	    continue;
	  if (L_EXTRACT_BIT_VAL (insert_op->flags, L_OPER_SAFE_PEI))
	    continue;
	  insert_op->flags =
	    L_SET_BIT_FLAG (insert_op->flags, L_OPER_SAFE_PEI);
	  insert_op->flags =
	    L_CLR_BIT_FLAG (insert_op->flags, L_OPER_MASK_PE);
	  insert_op->flags =
	    L_CLR_BIT_FLAG (insert_op->flags, L_OPER_SPECULATIVE);
	  STAT_COUNT ("Lopti_PRE_mem_safe_pei", 1, NULL);
	}
      Set_dispose (safe_pei_ops);
    }

  /* For each replace op, go through insert ops: if insert op reaches that
   * op and they have matching expressions, migrate sync, check info.
   */
  if (replace_size)
    insert_size = Set_2array (*all_insert, insert_array);

  for (i = 0; i < replace_size; i++)
    {
      num_inserted_ops = 0;
      replace_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					      replace_array[i]);
      replace_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					    replace_array[i]);

      /* We only need to do the following for memory ops. */
      if (L_general_load_opcode (replace_op) ||
	  L_general_store_opcode (replace_op))
	{
	  /* It is possible that this is a replace in a dead block, which
	   * due to SPRE semantics can happen. In this case, continue. */
	  if (!(replace_cb->src_flow))
	    continue;

	  reaching = L_get_oper_RIN_set (replace_op);
#ifdef DEBUG_PCE_REACH
	  fprintf (stderr, "Reaching ops for op %d:\n", replace_op->id);
	  Set_print (stderr, "\t", reaching);
#endif

	  for (j = 0; j < insert_size; j++)
	    {
	      if (!(Set_in (reaching, insert_array[j])))
		continue;
	      insert_op =
		L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					   insert_array[j]);
	      if (!(L_opers_same_expression (replace_op, insert_op)))
		continue;
	      inserted_ops[num_inserted_ops++] = insert_op;

	      /* If weak/incomplete sync info, may want to do a union 
	         of sync info here. */

	      /* Migrate check info */
	      if (L_general_load_opcode (replace_op) &&
		  L_EXTRACT_BIT_VAL (insert_op->flags, L_OPER_SPECULATIVE))
		{
		  STAT_COUNT ("Lopti_PRE_mem_check_migration", 1, replace_cb);
		  if (!L_EXTRACT_BIT_VAL
		      (replace_op->flags, L_OPER_SPECULATIVE))
		    L_global_insert_check_before (insert_op, replace_cb,
						  replace_op);
		  else
		    L_assign_all_checks (L_fn, replace_op, insert_op);
		}
	    }

	  if (!num_inserted_ops)
	    {
	      L_print_oper (stderr, replace_op);
	      L_punt ("L_PRE_correct_spec_and_sync_info: No matching reaching "
		      "ops found for op %d, cb %d in func %s", replace_op->id,
		      replace_cb->id, L_fn->name);
	    }
	}

      /* Convert replaced ops */
      if (!L_general_store_opcode (replace_op))
	{
	  token = L_generate_expression_token_from_oper (replace_op);
	  expression = L_find_oper_expression_in_hash
	    (L_fn->expression_token_hash_tbl, token, replace_op, 0);
	  src_operand = L_create_operand_for_expression_index
	    (expression->index);
	  if (L_general_load_opcode (replace_op))
	    STAT_COUNT ("Lopti_PRE_mem_load", 1, replace_cb);
	  /* clear out some flags, but not speculative ones */
	  replace_op->flags =
	    L_CLR_BIT_FLAG (replace_op->flags, L_OPER_SAFE_PEI);
	  replace_op->flags =
	    L_CLR_BIT_FLAG (replace_op->flags, L_OPER_MASK_PE);
	  L_convert_to_move (replace_op,
			     L_copy_operand (replace_op->dest[0]),
			     src_operand);
	}
      else			/* delete store ops */
	{
	  L_delete_oper (replace_cb, replace_op);
	  STAT_COUNT ("Lopti_PRE_mem_store", 1, replace_cb);

	  /* associate all inserted store ops with each other & selves */
	  if (L_func_contains_dep_pragmas && L_use_sync_arcs)
	    {
	      for (j = 0; j < num_inserted_ops; j++)
		{
		  for (k = 0; k < num_inserted_ops; k++)
		    {
		      L_add_sync_between_opers (inserted_ops[j],
						inserted_ops[k]);
		    }
		}
	    }
	}			/* End of store fixup. */
    }

  /* Get rid of any ops that were uselessly inserted. */
  new_insert_size = Set_2array (useless, insert_array);
  for (i = 0; i < new_insert_size; i++)
    {
      insert_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					     insert_array[i]);
      /* Because only memory and speculative opers are written to the replace
       * set, there may be standard opers in here which should not be removed.
       */
      if (!L_general_load_opcode (insert_op))
	continue;
      insert_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					   insert_array[i]);
      L_delete_oper (insert_cb, insert_op);
    }

  Set_dispose (useless);
  Lcode_free (replace_array);
  Lcode_free (insert_array);
  Lcode_free (inserted_ops);
  return;
}


/*
 * L_PRE_lazy_code_motion
 * ------------------------------
 * Performs lazy code motion as per the 1994 Knoop/Ruthing/Steffen paper.
 * Warning: Destroys pf_bb information! (first ops)
 * WARNING: MAY BE UNSAFE FOR PREDICATED CODE!!!
 * Assumption: PRE dataflow already run and not invalidated.
 * Also note that at the end of code motion, we must correct sync and
 * exception markings on new operations.
 */

int
L_PRE_lazy_code_motion (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *boundary_op, *first_bb_op = NULL, *next_op;
  Set insert, replace, insert_replace, ignore_set, complement_set,
    oper_all_insert, oper_new_insert, oper_replace;
  int i, new_flag, set_size, ignore, *expression_array, complement_flag,
    change = 0;

  if (!Lopti_do_PRE_lazy_code_motion)
    return 0;

  /* return 0 if no expressions found, Lcode_malloc will punt during alloc,
   * and nothing can be done here anyway */
  if (!(fn->n_expression))
    return 0;

  oper_all_insert = NULL;
  oper_new_insert = NULL;
  oper_replace = NULL;
  ignore_set = NULL;
  ignore = 0;

  if (!Lopti_do_PRE_optimize_single_source_ops)
    ignore |= L_EXPRESSION_SINGLE_SOURCE;
  else if (!Lopti_do_PRE_optimize_moves)
    ignore |= L_EXPRESSION_MOVE;
  else if (!Lopti_do_PRE_optimize_moves_of_numerical_constants)
    ignore |= L_EXPRESSION_MOVE_NUM_CONST;

  if (!Lopti_do_PCE_optimize_memory_ops)
    ignore |= L_EXPRESSION_MEMORY;

  ignore_set = L_get_expression_subset (ignore);

  expression_array = (int *) Lcode_malloc (sizeof (int) * fn->n_expression);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_PRE_cb_no_changes (cb, ignore_set))
	{
	  continue;
	}

      if (cb->first_op == NULL)
	{
	  boundary_op = NULL;
	  insert = L_get_PCE_bb_x_insert_set (cb, boundary_op);
	  set_size = Set_2array (insert, expression_array);
	  for (i = 0; i < set_size; i++)
	    {
	      if (Set_in (ignore_set, expression_array[i]))
		continue;
	      boundary_op = L_PRE_x_insert (expression_array[i], cb,
					    boundary_op, &oper_new_insert, 0);
	      STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
	      change++;
	    }
	  continue;
	}


      new_flag = 1;
      for (boundary_op = cb->first_op; boundary_op; boundary_op = next_op)
	{
	  next_op = boundary_op->next_op;
	  if (new_flag)
	    {			/* first instruction of bb */
	      first_bb_op = boundary_op;
	      insert = L_get_PCE_bb_n_insert_set (cb, first_bb_op);
	      replace = L_get_PCE_bb_n_replace_set (cb, first_bb_op);
	      insert_replace = Set_intersect (insert, replace);
	      insert = Set_subtract_acc (insert, insert_replace);
	      replace = Set_subtract_acc (replace, insert_replace);

	      set_size = Set_2array (insert_replace, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  L_PRE_n_insert_replace (expression_array[i], cb,
					  boundary_op, &oper_all_insert);
		  STAT_COUNT ("Lopti_PRE_insert_repl", 1, cb);
		  next_op = boundary_op->next_op;
		}
	      Set_dispose (insert_replace);

	      /* Can remove below eventually...shouldn't have anything. */
	      set_size = Set_2array (insert, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  L_PRE_n_insert (expression_array[i], cb, boundary_op,
				  &oper_new_insert, 0);
		  STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
		  change++;
		}

	      set_size = Set_2array (replace, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  L_PRE_n_replace (expression_array[i], cb,
				   boundary_op, &oper_replace);
		  STAT_COUNT ("Lopti_PRE_replace_redn", 1, cb);
		  change++;
		}

	      if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		  || (!(boundary_op->next_op)))
		{		/* also last instruction of bb */
		  insert = L_get_PCE_bb_x_insert_set (cb, first_bb_op);
		  replace = L_get_PCE_bb_x_replace_set (cb, first_bb_op);
		  complement_set = L_get_PCE_bb_complement_set (cb,
								first_bb_op);
		  insert = Set_subtract_acc (insert, replace);

		  set_size = Set_2array (insert, expression_array);
		  for (i = 0; i < set_size; i++)
		    {
		      if (Set_in (ignore_set, expression_array[i]))
			continue;
		      L_PRE_x_insert (expression_array[i], cb, boundary_op,
				      &oper_new_insert, 0);
		      STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
		      change++;
		    }

		  set_size = Set_2array (replace, expression_array);
		  for (i = 0; i < set_size; i++)
		    {
		      if (Set_in (ignore_set, expression_array[i]))
			continue;
		      complement_flag =
			Set_in (complement_set, expression_array[i]);
		      L_PRE_x_insert_replace (expression_array[i], cb,
					      boundary_op, &oper_all_insert,
					      &oper_new_insert,
					      complement_flag);
		      STAT_COUNT ("Lopti_PRE_insert_repl", 1, cb);
		    }
		  continue;
		}
	      new_flag = 0;
	      continue;
	    }
	  else if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		   || (!(boundary_op->next_op)))
	    {			/* last oper of bb, reset new_flag */
	      insert = L_get_PCE_bb_x_insert_set (cb, first_bb_op);
	      replace = L_get_PCE_bb_x_replace_set (cb, first_bb_op);
	      complement_set = L_get_PCE_bb_complement_set (cb, first_bb_op);
	      insert = Set_subtract_acc (insert, replace);

	      set_size = Set_2array (insert, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  L_PRE_x_insert (expression_array[i], cb, boundary_op,
				  &oper_new_insert, 0);
		  STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
		  change++;
		}

	      set_size = Set_2array (replace, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  complement_flag =
		    Set_in (complement_set, expression_array[i]);
		  L_PRE_x_insert_replace (expression_array[i], cb,
					  boundary_op, &oper_all_insert,
					  &oper_new_insert, complement_flag);
		  STAT_COUNT ("Lopti_PRE_insert_repl", 1, cb);
		}
	      new_flag = 1;
	    }
	}
    }

  Lcode_free (expression_array);
  ignore_set = Set_dispose (ignore_set);

  if (Lopti_do_PCE_optimize_memory_ops)
    {
      oper_all_insert = Set_union_acc (oper_all_insert, oper_new_insert);

#ifdef DEBUG_PCE_REACH
      fprintf (stderr, "FIX-UP OPERS:\n");
      Set_print (stderr, "Insert All", oper_all_insert);
      Set_print (stderr, "Insert New", oper_new_insert);
      Set_print (stderr, "Replace", oper_replace);
#endif

      new_flag = (PCE | PCE_REACHING_DEFINITION);
      new_flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
      new_flag |= (L_non_excepting_ops) ? 0 : PRE_ONLY_EXCEPTING;
      new_flag |= (Lopti_do_PCE_conservative_memory_opti) ?
	PCE_MEM_CONSERVATIVE : 0;

      L_do_flow_analysis (fn, new_flag);
      L_PRE_correct_spec_and_sync_info (&oper_all_insert, &oper_new_insert,
					&oper_replace);
    }
	
  Set_dispose (oper_all_insert);
  Set_dispose (oper_new_insert);
  Set_dispose (oper_replace);

  return (change);
}


/*========================================================================
  Partial Redundancy Elimination
	From "Optimal Code Motion" 1994 by Knoop, Ruthing, and Steffen.
  Algorithm for the "optimal and economical placement of 
	computations within flow graphs."
  Assumptions: Global dead code removal has already been performed. 
	Local common subexpression elimination has already been performed.
  Note: If/when IMPACT is transitioned to SSA, we will need to do this
	differently. Refer to Fred Chow and other papers on PRE circa
	1997.
========================================================================*/

int
L_partial_redundancy_elimination (L_Func * fn)
{
  int c1, c2 = 0, change, flag;

  if (Lopti_do_PRE == 0)
    return 0;

  flag = (PCE | PRE);
  flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
  flag |= (L_non_excepting_ops) ? 0 : PRE_ONLY_EXCEPTING;
  flag |= (Lopti_do_PRE_cutset_metric) ? PCE_CUTSET_METRIC : 0;
  flag |= (Lopti_do_PCE_conservative_memory_opti) ? PCE_MEM_CONSERVATIVE : 0;
  flag |= (Lopti_do_PRE_mem_copy_prop) ? PCE_MEM_COPY_PROP : 0;
  flag |= (Lopti_do_PRE_merge_loads_diff_types) ? PRE_LOAD_DIFF_TYPES : 0;
  /* Perform dataflow analysis to do lazy code motion */
  L_do_flow_analysis (fn, flag);

#ifdef DEBUG_LAZY_CODE_MOTION
  fprintf (stderr, "Performing lazy code motion on function %s.\n", fn->name);
#endif

  /* Clear reg ids after first time...can cause problems
   * if two expressions get assigned to each others' old
   * assignment variables (fold-const in 176.gcc) */
  L_clear_expression_reg_ids (fn);

  /* Perform lazy code motion */
  c1 = L_PRE_lazy_code_motion (fn);
  STAT_COUNT ("Lopti_PRE_lazy_code_motion", c1, NULL);

  /* c2 = L_PCE_cleanup (fn); */

  change = c1 + c2;

  return (change);
}


static int
L_PRE_speculative_code_motion (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *boundary_op, *first_bb_op = NULL, *insert_op, *replace_op, *next_op;
  Set insert, replace, insert_replace, ignore_set, ds_set,
    oper_all_insert, oper_new_insert, oper_replace, complement_set;
  int i, new_flag, set_size, ignore, *expression_array, speculate_flag,
    complement_flag, change = 0;

  if (!Lopti_do_PRE_lazy_code_motion)
    return 0;

  if (!(fn->n_expression))
    return 0;

  oper_all_insert = NULL;
  oper_new_insert = NULL;
  oper_replace = NULL;
  ignore_set = NULL;
  ignore = 0;

  if (!Lopti_do_PRE_optimize_single_source_ops)
    ignore |= L_EXPRESSION_SINGLE_SOURCE;
  else if (!Lopti_do_PRE_optimize_moves)
    ignore |= L_EXPRESSION_MOVE;
  else if (!Lopti_do_PRE_optimize_moves_of_numerical_constants)
    ignore |= L_EXPRESSION_MOVE_NUM_CONST;

  if (!Lopti_do_PCE_optimize_memory_ops)
    ignore |= L_EXPRESSION_MEMORY;

  ignore_set = L_get_expression_subset (ignore);
  expression_array = (int *) Lcode_malloc (sizeof (int) * fn->n_expression);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_PRE_cb_no_changes (cb, ignore_set))
	{
	  continue;
	}

      if (cb->first_op == NULL)
	{
	  boundary_op = NULL;
	  insert = L_get_PCE_bb_x_insert_set (cb, boundary_op);
	  ds_set = L_get_PCE_bb_xd_safe_set (cb, boundary_op);
	  set_size = Set_2array (insert, expression_array);
	  for (i = 0; i < set_size; i++)
	    {
	      if (Set_in (ignore_set, expression_array[i]))
		continue;
	      speculate_flag = (Set_in (ds_set, expression_array[i]) ? 0 : 1);
	      boundary_op = L_PRE_x_insert (expression_array[i], cb,
					    boundary_op, &oper_new_insert,
					    speculate_flag);
	      STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
	      change++;
	    }
	  continue;
	}

      new_flag = 1;
      for (boundary_op = cb->first_op; boundary_op; boundary_op = next_op)
	{
	  next_op = boundary_op->next_op;
	  if (new_flag)
	    {
	      first_bb_op = boundary_op;
	      /* first instruction of bb */

	      insert = L_get_PCE_bb_n_insert_set (cb, first_bb_op);
	      replace = L_get_PCE_bb_n_replace_set (cb, first_bb_op);
	      insert_replace = Set_intersect (insert, replace);
	      insert = Set_subtract_acc (insert, insert_replace);
	      replace = Set_subtract_acc (replace, insert_replace);
	      ds_set = L_get_PCE_bb_nd_safe_set (cb, first_bb_op);
#ifdef DEBUG_LAZY_CODE_MOTION
	      fprintf (stderr, "BB starting at op %d in CB %d\n",
		       first_bb_op->id, cb->id);
	      Set_print (stderr, "N_INSERT", insert);
	      Set_print (stderr, "N_REPLACE", replace);
	      Set_print (stderr, "N_INSERT_REPLACE", insert_replace);
#endif
	      set_size = Set_2array (insert_replace, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  L_PRE_n_insert_replace (expression_array[i], cb,
					  boundary_op, &oper_all_insert);
		  STAT_COUNT ("Lopti_PRE_insert_repl", 1, cb);
		  next_op = boundary_op->next_op;
		}
	      Set_dispose (insert_replace);

	      set_size = Set_2array (insert, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  speculate_flag =
		    (Set_in (ds_set, expression_array[i]) ? 0 : 1);
		  insert_op = L_PRE_n_insert (expression_array[i], cb,
					      boundary_op,
					      &oper_new_insert,
					      speculate_flag);
		  STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
		  change++;
		}

	      set_size = Set_2array (replace, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  replace_op = L_PRE_n_replace (expression_array[i], cb,
						boundary_op, &oper_replace);
		  STAT_COUNT ("Lopti_PRE_replace_redn", 1, cb);
		  change++;
		}

	      if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		  || (!(boundary_op->next_op)))
		{		/* also last instruction of bb */
		  insert = L_get_PCE_bb_x_insert_set (cb, first_bb_op);
		  replace = L_get_PCE_bb_x_replace_set (cb, first_bb_op);
		  complement_set = L_get_PCE_bb_complement_set (cb,
								first_bb_op);
		  insert = Set_subtract_acc (insert, replace);
		  ds_set = L_get_PCE_bb_xd_safe_set (cb, first_bb_op);

		  set_size = Set_2array (insert, expression_array);
		  for (i = 0; i < set_size; i++)
		    {
		      if (Set_in (ignore_set, expression_array[i]))
			continue;
		      speculate_flag =
			(Set_in (ds_set, expression_array[i]) ? 0 : 1);
		      insert_op = L_PRE_x_insert (expression_array[i], cb,
						  boundary_op,
						  &oper_new_insert,
						  speculate_flag);
		      STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
		      change++;
		    }
		  set_size = Set_2array (replace, expression_array);
		  for (i = 0; i < set_size; i++)
		    {
		      if (Set_in (ignore_set, expression_array[i]))
			continue;
		      complement_flag =
			Set_in (complement_set, expression_array[i]);
		      L_PRE_x_insert_replace (expression_array[i], cb,
					      boundary_op, &oper_all_insert,
					      &oper_new_insert,
					      complement_flag);
		      STAT_COUNT ("Lopti_PRE_insert_repl", 1, cb);
		    }
		  continue;
		}
	      new_flag = 0;
	      continue;
	    }
	  else if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		   || (!(boundary_op->next_op)))
	    {			/* last instruction of bb, reset new_flag */
	      insert = L_get_PCE_bb_x_insert_set (cb, first_bb_op);
	      replace = L_get_PCE_bb_x_replace_set (cb, first_bb_op);
	      complement_set = L_get_PCE_bb_complement_set (cb, first_bb_op);
	      insert = Set_subtract_acc (insert, replace);
	      ds_set = L_get_PCE_bb_xd_safe_set (cb, first_bb_op);
#if 0
#ifdef DEBUG_LAZY_CODE_MOTION
	      fprintf (stderr, "BB starting at op %d in CB %d\n",
		       first_bb_op->id, cb->id);
	      Set_print (stderr, "X_INSERT", insert);
	      Set_print (stderr, "X_INSERT_REPLACE", replace);
#endif
#endif
	      set_size = Set_2array (insert, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  speculate_flag =
		    (Set_in (ds_set, expression_array[i]) ? 0 : 1);
		  insert_op = L_PRE_x_insert (expression_array[i], cb,
					      boundary_op, &oper_new_insert,
					      speculate_flag);
		  STAT_COUNT ("Lopti_PRE_insert_new", 1, cb);
		  change++;
		}

	      set_size = Set_2array (replace, expression_array);
	      for (i = 0; i < set_size; i++)
		{
		  if (Set_in (ignore_set, expression_array[i]))
		    continue;
		  complement_flag =
		    Set_in (complement_set, expression_array[i]);
		  L_PRE_x_insert_replace (expression_array[i], cb,
					  boundary_op, &oper_all_insert,
					  &oper_new_insert, complement_flag);
		  STAT_COUNT ("Lopti_PRE_insert_repl", 1, cb);
		}
	      new_flag = 1;
	    }
	}
    }

  Lcode_free (expression_array);
  ignore_set = Set_dispose (ignore_set);

  if (Lopti_do_PCE_optimize_memory_ops)
    {
      oper_all_insert = Set_union_acc (oper_all_insert, oper_new_insert);

#ifdef DEBUG_PCE_REACH
      fprintf (stderr, "FIX-UP OPERS:\n");
      Set_print (stderr, "Insert All", oper_all_insert);
      Set_print (stderr, "Insert New", oper_new_insert);
      Set_print (stderr, "Replace", oper_replace);
#endif

      new_flag = (PCE | PCE_REACHING_DEFINITION);
      new_flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
      new_flag |= (L_non_excepting_ops) ? 0 : PRE_ONLY_EXCEPTING;
      new_flag |= (Lopti_do_PCE_conservative_memory_opti) ?
	PCE_MEM_CONSERVATIVE : 0;

      L_do_flow_analysis (fn, new_flag);
      L_PRE_correct_spec_and_sync_info (&oper_all_insert, &oper_new_insert,
					&oper_replace);
    }
  Set_dispose (oper_all_insert);
  Set_dispose (oper_new_insert);
  Set_dispose (oper_replace);

  return (change);
}


/*========================================================================
  Speculative Partial Redundancy Elimination
	Original description from Bodik Ph.D. 2000, implementation from
        Cai CGO 2003.
  Algorithm for the "optimal and economical placement of 
	computations within flow graphs."
  Assumptions: Global dead code removal has already been performed. 
	Local common subexpression elimination has already been performed.
  Potential Improvements: We can do SPRE on loads to safe addresses
        (global labels, stack addresses) but currently do not do so.
	Something to try when speculation support is limited.
  Note: If/when IMPACT is transitioned to SSA, we will need to do this
	differently. Refering to Fred Chow and other papers on PRE circa
	1997.
========================================================================*/

int
L_speculative_PRE (L_Func * fn)
{
  int c1, c2 = 0, change, flag, ignore = 0;
#ifdef TRACK_EXPRESSION_WEIGHT
  L_Expression_Hash_Entry *entry;
#endif

  if (!(Lopti_do_PRE && Lopti_do_PRE_speculative_code_motion))
    return 0;

  flag = (PCE | PRE_SPEC);
  flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
  flag |= (L_non_excepting_ops) ? 0 : PRE_ONLY_EXCEPTING;
  flag |= (Lopti_do_PRE_cutset_metric) ? PCE_CUTSET_METRIC : 0;
  flag |= (Lopti_do_PCE_conservative_memory_opti) ? PCE_MEM_CONSERVATIVE : 0;
  flag |= (Lopti_do_PRE_mem_copy_prop) ? PCE_MEM_COPY_PROP : 0;
  flag |= (Lopti_do_PRE_merge_loads_diff_types) ? PRE_LOAD_DIFF_TYPES : 0;

  if (!Lopti_do_PRE_optimize_single_source_ops)
    ignore |= L_EXPRESSION_SINGLE_SOURCE;
  else if (!Lopti_do_PRE_optimize_moves)
    ignore |= L_EXPRESSION_MOVE;
  else if (!Lopti_do_PRE_optimize_moves_of_numerical_constants)
    ignore |= L_EXPRESSION_MOVE_NUM_CONST;

  if (!Lopti_do_PCE_optimize_memory_ops)
    ignore |= L_EXPRESSION_MEMORY;

  /* This function is for the not-as-good allow-if-heavy-bias SPRE. */
  /* L_do_flow_analysis (fn, flag); */
  L_do_SPRE_analysis (fn, flag, ignore);
#ifdef DEBUG_SPECULATIVE_CODE_MOTION
  fprintf (stderr, "Performing speculative PRE code motion on function %s.\n",
	   fn->name);
#endif

  L_clear_expression_reg_ids (fn);

  c1 = L_PRE_speculative_code_motion (fn);
  STAT_COUNT ("Lopti_PRE_lazy_code_motion", c1, NULL);

#ifdef TRACK_EXPRESSION_WEIGHT
  for (i = 1; i <= L_fn->n_expression; i++)
    {
      entry =
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      c1 = entry->expression->weight;
      if (!c1)
	continue;
      fprintf (stderr, "%d:\t%.0f", i, entry->expression->weight);
      if (c1 > 0.0)
	fprintf (stderr, "\tWARNING: INCREASE IN WEIGHT!\n");
      else
	fprintf (stderr, "\n");
    }
#endif

  /* c2 = L_PCE_cleanup (fn); */

  change = c1 + c2;

  return change;
}


/*
 * PARTIAL DEAD CODE ELIMINATION FUNCTIONS
 * ----------------------------------------
 */

static L_Oper *
L_PDE_insert (int assignment_index, L_Cb * cb, L_Oper * first_op,
	      Set * oper_insert, Set * oper_generalized, int pred_guard)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *assignment;
  L_Oper *new_op;

  new_op = L_generate_oper_from_assignment_index (assignment_index,
						  pred_guard);
  if (pred_guard)
    {
      /* All cbs with predicated operations should be marked as hyperblocks. */
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      L_fn->flags = L_SET_BIT_FLAG (cb->flags, L_FUNC_HYPERBLOCK);
      STAT_COUNT ("Lopti_PDE_pred_guard", 1, cb);
    }
  else if (L_store_opcode (new_op))
    {
      entry =
        L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
                                          assignment_index);
      assignment = entry->expression;
      if (!(assignment->src[2]))
        *oper_generalized = Set_add (*oper_generalized, new_op->id);
    }

  L_insert_oper_before (cb, first_op, new_op);
  *oper_insert = Set_add (*oper_insert, new_op->id);


#ifdef DEBUG_PDE_MOTION
  fprintf (stderr, "Inserting assignment ");
  if (pred_guard)
    fprintf (stderr, "(with pred guard) ");
  fprintf (stderr, "%d as op %d at cb %d, op %d.\n",
	   assignment_index, new_op->id, cb->id, first_op ? first_op->id : 0);
#endif

  return new_op;
}


static L_Oper *
L_PDE_delete (int assignment_index, L_Cb * cb, L_Oper * last_op,
	      Set * oper_delete, int pred_set)
{
  L_Expression_Hash_Entry *entry;
  L_Expression *assignment;
  L_Oper *new_op, *return_op, *delete_op;
  int match = 0, short_flag = 0;

  entry = L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					    assignment_index);
  assignment = entry->expression;
#ifdef P3DE_GENERAL_STORE_MOTION
  if (L_store_opcode (assignment) && !(assignment->src[2]))
    short_flag = 1;
#endif

  for (delete_op = last_op; delete_op; delete_op = delete_op->prev_op)
    {
      if (L_EXTRACT_BIT_VAL (delete_op->flags, L_OPER_VOLATILE))
	continue;
      if (delete_op->opc == Lop_NO_OP)
	continue;
      if (L_oper_matches_assignment (delete_op, 0, assignment, short_flag))
	{
	  match = 1;
	  break;
	}
    }
  if (!match)
    L_punt
      ("L_PDE_delete: no matching oper found for assignment %d in cb %d!",
       assignment_index, cb->id);
#ifdef DEBUG_PDE_MOTION
  fprintf (stderr, "Deleting op %d (assignment %d) in cb %d.\n",
	   delete_op->id, assignment_index, cb->id);
#endif

#ifdef P3DE_GENERAL_STORE_MOTION
  /* Need to create a compensating move if a general store. */
  if (short_flag)
    {
      switch (assignment->dest_ctype)
	{
	case L_CTYPE_DOUBLE:
	  new_op = L_create_new_op (Lop_MOV_F2);
	  break;
	case L_CTYPE_FLOAT:
	  new_op = L_create_new_op (Lop_MOV_F);
	  break;
	default:
	  new_op = L_create_new_op (Lop_MOV);

	}
      new_op->dest[0] = L_create_operand_for_expression_index
	(assignment_index);
      new_op->src[0] = L_copy_operand (delete_op->src[2]);
      new_op->pred[0] = L_copy_operand (delete_op->pred[0]);
      new_op->pred[1] = L_copy_operand (delete_op->pred[1]);
      L_insert_oper_before (cb, delete_op, new_op);
      STAT_COUNT ("Lopti_PDE_comp_move_del", 1, cb);
#ifdef DEBUG_PDE_MOTION
      fprintf (stderr, "Inserting compensating move (with delete) op %d "
	       "(assignment %d) in cb %d.\n", new_op->id, assignment_index,
	       cb->id);
#endif
    }
#endif

  if (pred_set)
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      L_fn->flags = L_SET_BIT_FLAG (cb->flags, L_FUNC_HYPERBLOCK);

      new_op = L_create_new_op (Lop_CMP);
      new_op->dest[0] = L_create_pred_for_assignment_index (assignment_index);
      new_op->dest[0]->ptype = L_PTYPE_COND_T;
      new_op->com[0] = L_CTYPE_INT;
      new_op->com[1] = Lcmp_COM_EQ;
      new_op->src[0] = L_new_gen_int_operand (0);
      new_op->src[1] = L_new_gen_int_operand (0);
      new_op->pred[0] = L_copy_operand (delete_op->pred[0]);
      new_op->pred[1] = L_copy_operand (delete_op->pred[1]);
      L_insert_oper_before (cb, delete_op, new_op);
      STAT_COUNT ("Lopti_PDE_pred_set", 1, cb);

#ifdef DEBUG_PDE_MOTION
      fprintf (stderr, "Inserting pred set op %d (assignment %d) in cb %d.\n",
	       new_op->id, assignment_index, cb->id);
#endif
    }

  if (L_EXTRACT_BIT_VAL (delete_op->flags, L_OPER_SPECULATIVE) ||
      L_general_load_opcode (delete_op) ||
      L_general_store_opcode (delete_op) ||
      L_subroutine_call_opcode (delete_op))
    {
      *oper_delete = Set_add (*oper_delete, delete_op->id);
      return_op = last_op;
    }
  else
    {
      if (delete_op == last_op)
	return_op = last_op->prev_op;
      else
	return_op = last_op;
      L_delete_oper (cb, delete_op);
    }
  return return_op;
}


static L_Oper *
L_PDE_clear_pred (int assignment_index, L_Cb * cb, L_Oper * last_op,
		  Set * oper_load_insert)
{
  L_Oper *new_op;

  if (Lopti_do_PDE_predicated)
    {
      new_op = L_create_new_op (Lop_CMP);
      new_op->dest[0] = L_create_pred_for_assignment_index (assignment_index);
      new_op->dest[0]->ptype = L_PTYPE_COND_T;
      new_op->com[0] = L_CTYPE_INT;
      new_op->com[1] = Lcmp_COM_NE;
      new_op->src[0] = L_new_gen_int_operand (0);
      new_op->src[1] = L_new_gen_int_operand (0);

      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      L_fn->flags = L_SET_BIT_FLAG (cb->flags, L_FUNC_HYPERBLOCK);

#ifdef DEBUG_PDE_MOTION
      fprintf (stderr, "Inserting pred clear op %d (assignment %d) at cb %d, "
	       "op %d.\n", new_op->id, assignment_index, cb->id, last_op ?
	       last_op->id : 0);
#endif
    }
  else /* Insert corresponding load for speculatively sunken store. */
    {
      new_op = L_generate_complement_load_oper_from_assignment_index
	(assignment_index);
      *oper_load_insert = Set_add (*oper_load_insert, new_op->id);

#ifdef DEBUG_PDE_MOTION
      fprintf (stderr, "Inserting compensating load op %d (assignment %d) "
	       "at cb %d, op %d.\n", new_op->id, assignment_index, cb->id,
	       last_op ? last_op->id : 0);
#endif
    }


  if ((L_is_control_oper (last_op) && !L_sync_opcode (last_op)) ||
      L_cond_ret (last_op))
    L_insert_oper_before (cb, last_op, new_op);
  else
    L_insert_oper_after (cb, last_op, new_op);

  return new_op;
}


static void
L_PDE_correct_spec_and_sync_info (Set * insert, Set * delete,
				  Set * generalized)
{
  L_Cb *insert_cb, *delete_cb, *reach_cb;
  L_Oper *insert_op, *delete_op, *reach_op, *new_op;
  L_Loop *insert_loop, *delete_loop;
  L_Expression *assignment;
  L_Attr * attr;
  Set reaching, general_avail_move;
  int i, j, *insert_array, *delete_array, *reach_array,
    insert_size, delete_size, reach_size, token, assn_index;

  delete_array = (int *) Lcode_malloc (sizeof (int) * L_fn->n_oper);
  delete_size = Set_2array (*delete, delete_array);
  if (delete_size == 0)
    {
      Lcode_free (delete_array);
      return;
    }
  insert_array = (int *) Lcode_malloc (sizeof (int) * L_fn->n_oper);
  insert_size = Set_2array (*insert, insert_array);
  reach_array = (int *) Lcode_malloc (sizeof (int) * L_fn->n_oper);
  general_avail_move = NULL;

  /* Note that unlike PRE, it is possible to have deletions without
   * any insertions, since we automatically delete dead ops. */
  for (i = 0; i < insert_size; i++)
    {
      insert_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					     insert_array[i]);
      insert_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					   insert_array[i]);
      reaching = L_get_oper_RIN_set (insert_op);

      for (j = 0; j < delete_size; j++)
	{
	  if (!Set_in (reaching, delete_array[j]))
	    continue;
	  delete_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						 delete_array[j]);
	  /* Unlike PRE, stores may not have the exact same assignment. */
	  if (!L_opers_same_assignment (insert_op, delete_op, 1))
	    continue;
	  delete_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					       delete_array[j]);
	  /* Copy flags and attributes over. There is a possibility that the
	     flags may be different, so OR them together. */
	  insert_op->flags |= delete_op->flags;
	  if (!(insert_op->attr) && delete_op->attr)
	    insert_op->attr = L_copy_attr (delete_op->attr);
	  /* Check that stack attr exists on insert_op. */
	  else if (L_stack_reference (insert_op) &&
	      (!(attr = L_find_attr (insert_op->attr, STACK_ATTR_NAME))))
	    {
	      attr = L_find_attr (delete_op->attr, STACK_ATTR_NAME);
	      if (attr)
		insert_op->attr = L_concat_attr (insert_op->attr,
						 L_copy_attr_element (attr));
	    }
	  if (L_func_contains_dep_pragmas &&
	      !(insert_op->sync_info) && (delete_op->sync_info))
	    {
	      insert_op->sync_info = L_copy_sync_info (delete_op->sync_info);
	      L_insert_all_syncs_in_dep_opers (insert_op);
	      delete_loop = delete_cb->deepest_loop;
	      insert_loop = insert_cb->deepest_loop;
	      if (delete_loop && insert_loop &&
		  delete_loop->nesting_level > insert_loop->nesting_level)
		L_adjust_syncs_for_movement_out_of_loop (insert_op,
							 insert_cb);
	      if (L_store_opcode (delete_op))
		{
		  L_add_sync_between_opers (insert_op, insert_op);
		  L_add_sync_between_opers (delete_op, insert_op);
		}
	    }
	  if (delete_op->acc_info && !insert_op->acc_info)
	    {
	      insert_op->acc_info = 
		L_copy_mem_acc_spec_list (delete_op->acc_info);
	    }
	  if (L_general_load_opcode (delete_op) &&
	      L_EXTRACT_BIT_VAL (insert_op->flags, L_OPER_MASK_PE))
	    L_assign_all_checks (L_fn, delete_op, insert_op);
	}
#ifdef P3DE_GENERAL_STORE_MOTION
      if (!L_store_opcode (insert_op))
	continue;
      if ((generalized == NULL) || !Set_in (*generalized, insert_op->id))
	continue;
      token = L_generate_assignment_token_from_oper (insert_op, 1);
      assignment =
	L_find_oper_assignment_in_hash (L_fn->expression_token_hash_tbl,
					token, insert_op, 1);
      assn_index = assignment->index;

      /* Here, we have to make sure that every matching store assignment
       * that reaches a inserted, not predicated, generalized store op
       * moves its store operand to the appropriate temporary register.
       * By doing this we can use availability to reduce predicate usage. */
      reach_size = Set_2array (reaching, reach_array);
      for (j = 0; j < reach_size; j++)
	{
	  if (Set_in (*delete, reach_array[j]) ||
	      Set_in (*insert, reach_array[j]))
	    continue;
	  reach_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						reach_array[j]);
	  if (!L_opers_same_assignment (insert_op, reach_op, 1))
	    continue;

	  /* Avoid repeated inserts of a move from the same oper. */
	  if (Set_in (general_avail_move, reach_op->id))
	    continue;
	  else
	    general_avail_move = Set_add (general_avail_move, reach_op->id);

	  reach_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					      reach_array[j]);
	  switch (assignment->dest_ctype)
	    {
	    case L_CTYPE_DOUBLE:
	      new_op = L_create_new_op (Lop_MOV_F2);
	      break;
	    case L_CTYPE_FLOAT:
	      new_op = L_create_new_op (Lop_MOV_F);
	      break;
	    default:
	      new_op = L_create_new_op (Lop_MOV);
	    }
	  new_op->dest[0] = L_create_operand_for_expression_index
	    (assn_index);
	  new_op->src[0] = L_copy_operand (reach_op->src[2]);
	  new_op->pred[0] = L_copy_operand (reach_op->pred[0]);
	  new_op->pred[1] = L_copy_operand (reach_op->pred[1]);
	  L_insert_oper_before (reach_cb, reach_op, new_op);
	  STAT_COUNT ("Lopti_PDE_comp_move_avail", 1, reach_cb);
#ifdef DEBUG_PDE_MOTION
	  fprintf (stderr, "Inserting compensating move (avail) op %d "
		   "for store op %d (assignment %d) in cb %d.\n", new_op->id,
		   reach_op->id, assn_index, reach_cb->id);
#endif
	}
#endif
    }

  /* Delete all opers that are supposed to be deleted. */
  for (j = 0; j < delete_size; j++)
    {
      delete_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					     delete_array[j]);
      delete_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					   delete_array[j]);
      if (L_store_opcode (delete_op))
	STAT_COUNT ("Lopti_PDE_store", 1, delete_cb);
      L_delete_oper (delete_cb, delete_op);
    }

  Set_dispose (general_avail_move);
  Lcode_free (insert_array);
  Lcode_free (delete_array);
  Lcode_free (reach_array);
}

static void
L_PDE_correct_inserted_load_sync_info (Set * generalized, Set * load_insert)
{
  L_Cb *load_cb, *store_cb;
  L_Oper *load_op, *store_op;
  L_Loop *load_loop, *store_loop;
  L_Attr * attr;
  Set reaching;
  int i, j, *load_array, *store_array, load_size, store_size;

  load_array = (int *) Lcode_malloc (sizeof (int) * L_fn->n_oper);
  load_size = Set_2array (*load_insert, load_array);
  if (load_size == 0)
    {
      Lcode_free (load_array);
      return;
    }
  store_array = (int *) Lcode_malloc (sizeof (int) * L_fn->n_oper);
  store_size = Set_2array (*generalized, store_array);

  for (i = 0; i < store_size; i++)
    {
      store_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					    store_array[i]);
      store_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					  store_array[i]);
      reaching = L_get_oper_RIN_set (store_op);

      for (j = 0; j < load_size; j++)
	{
	  if (!Set_in (reaching, load_array[j]))
	    continue;
	  load_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
					       load_array[j]);
	  /* Unlike PRE, stores may not have the exact same assignment. */
	  if (!L_opers_same_or_complementary_expression (load_op, store_op))
	    continue;
	  load_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
					     load_array[j]);
	  /* Copy flags and attributes over. */
	  load_op->flags |= store_op->flags;
	  if (!(load_op->attr) && store_op->attr)
	    load_op->attr = L_copy_attr (store_op->attr);
	  /* Check that stack attr exists on insert_op. */
	  else if (L_stack_reference (load_op) &&
	      (!(attr = L_find_attr (load_op->attr, STACK_ATTR_NAME))))
	    {
	      attr = L_find_attr (store_op->attr, STACK_ATTR_NAME);
	      if (attr)
		load_op->attr = L_concat_attr (load_op->attr,
					       L_copy_attr_element (attr));
	    }
	  if (L_func_contains_dep_pragmas &&
	      !(load_op->sync_info) && (store_op->sync_info))
	    {
	      load_op->sync_info = L_copy_sync_info (store_op->sync_info);
	      L_insert_all_syncs_in_dep_opers (load_op);
	      store_loop = store_cb->deepest_loop;
	      load_loop = load_cb->deepest_loop;
	      if (store_loop && load_loop &&
		  store_loop->nesting_level > load_loop->nesting_level)
		L_adjust_syncs_for_movement_out_of_loop (load_op,
							 load_cb);
	      L_add_sync_between_opers (store_op, load_op);
	    }
	  if (store_op->acc_info && !load_op->acc_info)
	    {
	      load_op->acc_info = 
		L_copy_mem_acc_spec_list_as_use (store_op->acc_info);
	    }
	}
    }

  /* Removes load-load sync arcs that we probably created. */
  L_adjust_invalid_sync_arcs_in_func (L_fn);

  Lcode_free (load_array);
  Lcode_free (store_array);
}


/* SER: This function is part of partial dead code elimination. It pushes down
 * operations that can be executed later which hopefully exposes those that
 * can be eliminated along some paths.
 * Assumption: Sinking dataflow analysis already performed and valid. 
 */
int
L_PDE_code_motion (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *boundary_op, *next_op, *first_bb_op = NULL;
  Set insert, delete, oper_insert, oper_delete;
  int i, set_size, new_flag, change, *assignment_array;

  if (fn->n_expression == 0)
    return 0;

  change = 0;
  oper_insert = NULL;
  oper_delete = NULL;

  assignment_array = (int *) Lcode_malloc (sizeof (int) * fn->n_expression);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_PDE_cb_no_changes (cb))
	continue;

      if (cb->first_op == NULL)
	{
	  boundary_op = NULL;
	  insert = L_get_PCE_bb_n_insert_set (cb, NULL);
	  set_size = Set_2array (insert, assignment_array);
	  for (i = 0; i < set_size; i++)
	    {
	      boundary_op =
		L_PDE_insert (assignment_array[i], cb, boundary_op,
			      &oper_insert, NULL, 0);
	    }
	  STAT_COUNT ("Lopti_PDE_insert", set_size, cb);
	  change += set_size;
	  continue;
	}

      new_flag = 1;
      for (boundary_op = cb->first_op; boundary_op; boundary_op = next_op)
	{
	  next_op = boundary_op->next_op;
	  if (new_flag)
	    {
	      first_bb_op = boundary_op;
	      insert = L_get_PCE_bb_n_insert_set (cb, first_bb_op);
	      set_size = Set_2array (insert, assignment_array);
	      for (i = 0; i < set_size; i++)
		{
		  L_PDE_insert (assignment_array[i], cb, boundary_op,
				&oper_insert, NULL, 0);
		}
	      STAT_COUNT ("Lopti_PDE_insert", set_size, cb);
	      change += set_size;

	      if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		  || (!(boundary_op->next_op)))
		{		/* first oper also last oper of BB */
		  delete = L_get_PCE_bb_x_replace_set (cb, first_bb_op);
		  set_size = Set_2array (delete, assignment_array);
		  for (i = 0; i < set_size; i++)
		    {
		      boundary_op = L_PDE_delete (assignment_array[i], cb,
						  boundary_op, &oper_delete,
						  0);
		    }
		  STAT_COUNT ("Lopti_PDE_delete", set_size, cb);
		  change += set_size;
		  continue;
		}
	      new_flag = 0;
	      continue;
	    }
	  else if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		   || (!(boundary_op->next_op)))
	    {			/* last oper of BB, reset new flag */
	      delete = L_get_PCE_bb_x_replace_set (cb, first_bb_op);
	      set_size = Set_2array (delete, assignment_array);
	      for (i = 0; i < set_size; i++)
		{
		  boundary_op = L_PDE_delete (assignment_array[i], cb,
					      boundary_op, &oper_delete, 0);
		}
	      STAT_COUNT ("Lopti_PDE_delete", set_size, cb);
	      change += set_size;
	      new_flag = 1;
	    }
	}
    }

  Lcode_free (assignment_array);

  new_flag = (PCE | PCE_REACHING_DEFINITION);
  new_flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
  new_flag |= (Lopti_do_PCE_conservative_memory_opti) ?
    PCE_MEM_CONSERVATIVE : 0;

  L_do_flow_analysis (fn, new_flag);
  L_PDE_correct_spec_and_sync_info (&oper_insert, &oper_delete, NULL);
  Set_dispose (oper_insert);
  Set_dispose (oper_delete);

  return change;
}


/*========================================================================
  Partial Dead Code Elimination
	From the paper of the same name, 1994 by Knoop, Ruthing, and Steffen.
  Sinks code which is dead along some paths, so global dead code elimination
        can remove the original use and uses that are dead along their paths.
  Differences: this is able to move operations that write to their own
  operand, due to the addition of removal sets, which allows further sinking.
  The paper relied on global dead code to remove all instructions, which
  would not work with these operations.
  Note: Partial dead code elimination should be done with the split of 
  every critical branch exiting a loop.
========================================================================*/

int
L_partial_dead_code_elimination (L_Func * fn)
{
  int i, dead_code, pde, change, total_change, sink_flag;

  if (Lopti_do_PDE == 0)
    return 0;

  total_change = dead_code = pde = 0;

  sink_flag = (PCE | PDE);
  sink_flag |= (Lopti_do_PDE_cutset_metric) ? PCE_CUTSET_METRIC : 0;
  sink_flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
  sink_flag |= (Lopti_do_PDE_sink_stores) ? PDE_STORE : 0;
  sink_flag |= (Lopti_do_PDE_sink_only_stores) ? PDE_STORE_ONLY : 0;
  sink_flag |=
    (Lopti_do_dead_local_var_store_removal) ? DEAD_LOCAL_MEM_VAR : 0;
  sink_flag |= (Lopti_do_PCE_conservative_memory_opti) ?
    PCE_MEM_CONSERVATIVE : 0;

  for (i = 0; i < MAX_PDE_ITER; i++)
    {
#ifdef TRACK_PDE_ITERATIONS
      fprintf (stderr, "Iteration %d of PDE in func %s.\n", i + 1, fn->name);
      if (i == MAX_PDE_ITER - 1)
	fprintf (stderr, "HELP! Last iteration reached in PDE in func %s.\n",
		 fn->name);
#endif
      if (i == MAX_PDE_ITER - 1)
	return (total_change);

      L_do_flow_analysis (fn, sink_flag);
      change = L_PDE_code_motion (fn);

      total_change += change;

      if (!change || i == MAX_PDE_ITER - 1)
	break;
    }

  return total_change;
}


/*========================================================================
  Speculative Partial Dead Code Elimination
	New work. Sinks assignments to points where they will be executed
  fewer times. If an assignment is sunk past a "safe" point (as in it may
  not have been executed previously through that point), then original
  positions are marked with pred sets and paths which do not have the
  assignment.
========================================================================*/

static int
L_PDE_predicated_code_motion (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *boundary_op, *first_bb_op, *next_op;
  Set insert, delete, pred_guard, pred_set, pred_clear,
    oper_insert, oper_delete, oper_generalized, oper_load_insert;
  DF_PCE_INFO *pce = NULL;
  int i, change, *assignment_array, set_size, new_flag, pred_guard_flag,
    pred_set_flag;

  if (!(fn->n_expression))
    return 0;

  change = 0;
  oper_insert = NULL;
  oper_delete = NULL;
  oper_load_insert = NULL;
  oper_generalized = NULL;

#ifdef DEBUG_PDE_MOTION
  fprintf (stderr, "Beginning PDE predicated code motion on function %s.\n",
	   L_fn->name);
#endif

  assignment_array = (int *) Lcode_malloc (sizeof (int) * fn->n_expression);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_PDE_cb_no_changes (cb))
	continue;

      if (cb->first_op == NULL)
	{
	  boundary_op = NULL;
	  pce = L_get_PCE_bb_info (cb, boundary_op);
	  insert = pce->n_insert;
	  pred_guard = pce->pred_guard;
	  set_size = Set_2array (insert, assignment_array);
	  for (i = 0; i < set_size; i++)
	    {
	      pred_guard_flag = Lopti_do_PDE_predicated ?
		Set_in (pred_guard, assignment_array[i]) : 0;
	      boundary_op = L_PDE_insert (assignment_array[i], cb,
					  boundary_op, &oper_insert,
					  &oper_generalized, pred_guard_flag);
	    }
	  STAT_COUNT ("Lopti_PDE_insert", set_size, cb);
	  change += set_size;
	  boundary_op = cb->last_op;
	  pred_clear = pce->pred_clear;
	  set_size = Set_2array (pred_clear, assignment_array);
	  for (i = 0; i < set_size; i++)
	    {
	      L_PDE_clear_pred (assignment_array[i], cb, boundary_op,
				&oper_load_insert);
	    }
	  STAT_COUNT ("Lopti_PDE_pred_clear", set_size, cb);
	  continue;
	}

      new_flag = 1;
      for (boundary_op = cb->first_op; boundary_op; boundary_op = next_op)
	{
	  next_op = boundary_op->next_op;
	  if (new_flag)
	    {
	      first_bb_op = boundary_op;
	      pce = L_get_PCE_bb_info (cb, first_bb_op);
	      insert = pce->n_insert;
	      pred_guard = pce->pred_guard;
	      set_size = Set_2array (insert, assignment_array);
	      for (i = 0; i < set_size; i++)
		{
		  pred_guard_flag = Lopti_do_PDE_predicated ?
		    Set_in (pred_guard, assignment_array[i]) : 0;
		  L_PDE_insert(assignment_array[i], cb, boundary_op,
			       &oper_insert, &oper_generalized,
			       pred_guard_flag);
		}
	      STAT_COUNT ("Lopti_PDE_insert", set_size, cb);

	      if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		  || (!(boundary_op->next_op)))
		{		/* first oper also last oper of BB */
		  pred_clear = pce->pred_clear;
		  set_size = Set_2array (pred_clear, assignment_array);
		  for (i = 0; i < set_size; i++)
		    {
		      L_PDE_clear_pred (assignment_array[i], cb, boundary_op,
					&oper_load_insert);
		    }
		  STAT_COUNT ("Lopti_PDE_pred_clear", set_size, cb);

		  delete = pce->x_replace;
		  pred_set = pce->pred_set;
		  set_size = Set_2array (delete, assignment_array);
		  for (i = 0; i < set_size; i++)
		    {
		      pred_set_flag = Lopti_do_PDE_predicated ?
			Set_in (pred_set, assignment_array[i]) : 0;
		      boundary_op = L_PDE_delete (assignment_array[i], cb,
						  boundary_op, &oper_delete,
						  pred_set_flag);
		    }
		  STAT_COUNT ("Lopti_PDE_delete", set_size, cb);
		  change += set_size;
		  continue;
		}
	      new_flag = 0;
	      continue;
	    }
	  else if (L_is_control_oper (boundary_op) || L_cond_ret (boundary_op)
		   || (!(boundary_op->next_op)))
	    {			/* last oper of BB, reset new flag */
	      pred_clear = pce->pred_clear;
	      set_size = Set_2array (pred_clear, assignment_array);
	      for (i = 0; i < set_size; i++)
		{
		  L_PDE_clear_pred (assignment_array[i], cb, boundary_op,
				    &oper_load_insert);
		}
	      STAT_COUNT ("Lopti_PDE_pred_clear", set_size, cb);

	      delete = pce->x_replace;
	      pred_set = pce->pred_set;
	      set_size = Set_2array (delete, assignment_array);
	      for (i = 0; i < set_size; i++)
		{
		  pred_set_flag = Lopti_do_PDE_predicated ?
		    Set_in (pred_set, assignment_array[i]) : 0;
		  boundary_op = L_PDE_delete (assignment_array[i], cb,
					      boundary_op, &oper_delete,
					      pred_set_flag);
		}
	      STAT_COUNT ("Lopti_PDE_delete", set_size, cb);
	      change += set_size;
	      new_flag = 1;
	    }
	}
    }

  Lcode_free (assignment_array);

  new_flag = (PCE | PCE_REACHING_DEFINITION);
  new_flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
  new_flag |= (Lopti_do_PCE_conservative_memory_opti) ?
    PCE_MEM_CONSERVATIVE : 0;

  L_do_flow_analysis (fn, new_flag);
  L_PDE_correct_spec_and_sync_info (&oper_insert, &oper_delete,
				    &oper_generalized);

  if (!Lopti_do_PDE_predicated && Lopti_do_PDE_min_cut)
    {
      L_PDE_correct_inserted_load_sync_info (&oper_generalized,
					   &oper_load_insert);
    }

  Set_dispose (oper_insert);
  Set_dispose (oper_delete);
  Set_dispose (oper_generalized);
  Set_dispose (oper_load_insert);

  return change;
}


/* If two predicates are equivalent, use the numerically smaller one. Note
 * that the predicate analysis isn't capable of detecting equivalence across
 * backedges, so this isn't all that helpful for truly interesting cases of
 * P3DE, but it may still be helpful to remove some cases and may result in
 * removal of some pred defines. */
int
L_PDE_combine_pred_guards (L_Func * fn)
{
  L_Cb * cb;
  L_Oper * opA, * opB;
  int change = 0;

  L_partial_dead_code_removal (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
	{
	  if (!(opA->pred[0]))
	    continue;
	  for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	    {
	      if (!(opB->pred[0]))
		continue;
	      if (L_same_operand (opA->pred[0], opB->pred[0]))
		continue;
	      if (!PG_equivalent_predicates_ops (opA, opB))
		continue;
#ifdef DEBUG_PDE_COMBINE_PRED_GUARDS
              fprintf (stderr, "Merging pred %d and pred %d.\n",
		       opB->pred[0]->value.r, opA->pred[0]->value.r);
#endif
	      if (opB->pred[0]->value.r < opA->pred[0]->value.r)
		{
		  L_delete_operand (opA->pred[0]);
		  L_delete_operand (opA->pred[1]);
		  opA->pred[0] = L_copy_operand (opB->pred[0]);
		  opA->pred[1] = L_copy_operand (opB->pred[1]);
		}
	      else
		{
		  L_delete_operand (opB->pred[0]);
		  L_delete_operand (opB->pred[1]);
		  opB->pred[0] = L_copy_operand (opA->pred[0]);
		  opB->pred[1] = L_copy_operand (opA->pred[1]);
		}
	      STAT_COUNT ("Lopti_PDE_combine_pred_guards", 1, cb);
	      change++;
	    }
	}
    }
  return change;
}


int
L_min_cut_PDE (L_Func * fn)
{
  int i, change, total_change, sink_flag;

  if (!(Lopti_do_PDE && Lopti_do_PDE_min_cut))
    return 0;

  sink_flag = (PCE | PDE_MIN_CUT);
  sink_flag |= (Lopti_do_PDE_predicated) ? PDE_PREDICATED : 0;
  sink_flag |= (Lopti_do_PDE_cutset_metric) ? PCE_CUTSET_METRIC : 0;
  sink_flag |= (Lopti_do_PCE_optimize_memory_ops) ? PCE_MEM : 0;
  sink_flag |= (Lopti_do_PDE_sink_stores) ? PDE_STORE : 0;
  sink_flag |=
    (Lopti_do_dead_local_var_store_removal) ? DEAD_LOCAL_MEM_VAR : 0;
  sink_flag |= (Lopti_do_PCE_conservative_memory_opti) ?
    PCE_MEM_CONSERVATIVE : 0;

  total_change = 0;
  for (i = 0; i < MAX_PDE_ITER; i++)
    {
      L_clear_expression_reg_ids (fn);

#ifdef TRACK_PDE_ITERATIONS
      fprintf (stderr, "Iteration %d of min-cut PDE in func %s.\n", i + 1,
	       fn->name);
      if (i == MAX_PDE_ITER - 1)
	fprintf (stderr, "HELP! Last iteration reached in min-cut PDE in "
		 "func %s.\n", fn->name);
#endif

      L_do_P3DE_analysis (fn, sink_flag, 0);
      change = L_PDE_predicated_code_motion (fn);
      total_change += change;

#ifdef P3DE_GENERAL_STORE_MOTION
      {
	L_Cb * cb;
	L_do_flow_analysis (fn, LIVE_VARIABLE);
	for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	  {
	    int j, local_change;
	    if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	      continue;

	    for (j = 0; j < MAX_PCE_LOCAL_CLEANUP_ITER; j++)
	      {
		local_change = L_local_rev_copy_propagation (cb);
		if (!local_change)
		  break;
		STAT_COUNT ("Lopti_PDE_local_rev_copy_propagation",
			    local_change, cb);
	      }
	  }
      }
#endif

      if (!change || i == MAX_PDE_ITER - 1)
	break;
    }

  return total_change;
}


/*
 * L_coalesce_cbs
 * ------------------------------
 * Essentially undoes the change made by splitting critical cbs, if they
 * can be undone. Removes empty or jump-only cbs and combines cbs that 
 * are always successive. L_jump_optimization does all of the below and more,
 * so this function is only necessary when we don't want recombinations to
 * count as changes in loops (such as the first loop in Lopti).
 */

int
L_coalesce_cbs (L_Func * fn, int jump_cleanup_flag)
{
  int change, c1, c2, c3;

  change = c1 = c2 = c3 = 0;

  /* First, remove cbs with only a jump in them, and redirect */
  c1 = L_jump_combine_branch_to_uncond_branch (fn, 0);
  STAT_COUNT ("Lopti_PRE_jump_combine_branch_to_uncond_branch", c1, NULL);
#ifdef DEBUG_SPLIT_CRITICAL_EDGES
  fprintf (stderr,
	   "%d cbs recombined via L_jump_combine_branch_to_uncond_branch.\n",
	   c1);
#endif

  /* Next, combine cbs that are always successive */
  c2 = L_jump_merge_always_successive_blocks (fn, 0);
  STAT_COUNT ("Lopti_PRE_jump_merge_always_successive_blocks", c2, NULL);
#ifdef DEBUG_SPLIT_CRITICAL_EDGES
  fprintf (stderr,
	   "%d cbs recombined via L_jump_merge_always_successive_blocks.\n",
	   c2);
#endif

  if (jump_cleanup_flag)
    {
      /* Last, combine labels */
      c3 = L_jump_combine_labels (fn, 0);
      STAT_COUNT ("Lopti_PRE_jump_combine_labels", c3, NULL);
    }
#ifdef DEBUG_SPLIT_CRITICAL_EDGES
  fprintf (stderr, "%d cbs recombined via L_jump_combine_labels.\n", c3);
#endif

  change = c1 + c2 + c3;

  return change;
}


/* SER 08/30/03: This function turns off all optimizations which are subsumed
 * by SPRE and/or P3DE, to save time. */
void
L_PCE_disable_subsumed_optis ()
{
  if (Lopti_do_PRE && Lopti_do_PRE_speculative_code_motion &&
      ((L_use_sync_arcs && L_func_contains_dep_pragmas) || L_func_acc_specs) &&
      !Lopti_do_PCE_conservative_memory_opti)
    {
      if (L_fn->weight > 0.0)
	Lopti_do_loop_inv_code_rem = 0;
      else
	Lopti_do_loop_inv_code_rem = 1;
    }
#if 0
  /* SER: disabled since we overuse predication with P3DE currently.
   * Eventually we will want to do unpredicated P3DE by default on stores. */
  if (Lopti_do_PRE && Lopti_do_PRE_speculative_code_motion &&
      Lopti_do_PCE_optimize_memory_ops && Lopti_do_PRE_mem_copy_prop &&
      Lopti_do_PDE && Lopti_do_PDE_min_cut && Lopti_do_PDE_predicated &&
      !Lopti_do_PCE_conservative_memory_opti &&
      ((L_use_sync_arcs && L_func_contains_dep_pragmas) || L_func_acc_specs))
    {
      if (L_fn->weight > 0.0)
	Lopti_do_loop_global_var_mig = 0;
      else
	Lopti_do_loop_global_var_mig = 1;
    }
#endif
}


/* 02/11/04: Some functions have 0 weights at .gp although they are
 * exercised and have weights inside. Modify the function weight if this
 * is the case, so that SPRE and P3DE will be executed. */
void
L_PCE_fix_function_weight ()
{
  L_Cb * cb;

  if (L_fn->weight > 0.0)
    return;

  cb = L_fn->first_cb;
  cb = cb->next_cb;
  if (cb->weight != 0.0)
    {
      L_fn->weight = cb->weight;
      fprintf (stderr, "Zero weight corrected via cb for function %s.\n",
	       L_fn->name);
    }
  else
    {
      while ((cb = cb->next_cb))
	{
	  if (cb->weight != 0.0)
	    {
	      L_fn->weight = 1.0;
	      fprintf (stderr, "Zero weight corrected for function %s.\n",
		       L_fn->name);
	      break;
	    }
	}
    }
}


/* SER 10/06/03: After PRE or PDE, multiple blocks to the same cb that have
 * the same contents can occur. This can make code relatively unclean in
 * later steps. */
int
L_PCE_merge_same_cbs (L_Func * fn)
{
  L_Cb * cbBase, * cbA, * cbB, * next_cb;
  L_Oper * opA, * opB, * opC, * new_oper;
  L_Flow * src_flow1, * src_flow2, * next_flow, * flow_replace, * flow_switch,
    * dest_flow;
  int change;

  if (!Lopti_do_PCE_merge_same_cbs)
    return 0;

  change = 0;

  for (cbBase = fn->first_cb; cbBase != NULL; cbBase = next_cb)
    {
      next_cb = cbBase->next_cb;
      for (src_flow1 = cbBase->src_flow; src_flow1 != NULL;
	   src_flow1 = src_flow1->next_flow)
	{
	  cbA = src_flow1->src_cb;
	  if (cbA == cbBase)
	    continue;

	  for (src_flow2 = src_flow1->next_flow; src_flow2 != NULL;
	       src_flow2 = next_flow)
	    {
	      int same = 1;
	      next_flow = src_flow2->next_flow;
	      /* it's possible that next_flow and src_flow2 are from the
	       * same cb, and since this cb gets deleted, we have to
	       * get the next flow to a different cb */
	      while (next_flow && next_flow->src_cb == src_flow2->src_cb)
		next_flow = next_flow->next_flow;
	      cbB = src_flow2->src_cb;
	      if (cbB == cbBase || cbA == cbB)
		continue;
	      /* check if cbs match */
	      opA = cbA->first_op;
	      opB = cbB->first_op;

	      /* We don't want to optimize cases that are handled by
	       * jump opti: want an accurate count. */
	      if ((!opA || L_uncond_branch_opcode (opA)) &&
		  (!opB || L_uncond_branch_opcode (opB)))
		continue;

	      while (opA || opB)
		{
		  if (!L_same_operation (opA, opB, 0))
		    {
		      /* The cbs are still the same if one of them is an
		       * uncond branch and the other is a fallthrough. */
		      if (!((L_uncond_branch_opcode (opA) && opB == NULL) ||
			    (L_uncond_branch_opcode (opB) && opA == NULL)))
			same = 0;
		      break;
		    }

		  opA = opA->next_op;
		  opB = opB->next_op;
		}

	      if (!same)
		continue;

	      /* Need to check that fallthrough paths are the same. */
	      flow_switch = L_find_last_flow (cbA->dest_flow);
	      flow_replace = L_find_last_flow (cbB->dest_flow);
	      if (flow_switch->dst_cb != flow_replace->dst_cb)
		continue;

#ifdef DEBUG_MERGE_SAME_CBS
	      fprintf (stderr, "Merging cb %d into cb %d, both entering "
		       "cb %d.\n", cbB->id, cbA->id, cbBase->id);
#endif

	      /* Redirect all branches to cbB into cbA */
	      for (flow_replace = cbB->src_flow; flow_replace;
		   flow_replace = flow_replace->next_flow)
		{
		  L_Flow * new_flow;
		  L_Cb * cbC = flow_replace->src_cb;

		  dest_flow = L_find_matching_flow (cbC->dest_flow,
						    flow_replace);

		  if ((opC = L_find_branch_for_flow (cbC, dest_flow)))
		    {
		      L_change_branch_dest (opC, cbB, cbA);
		    }
		  else
		    {
		      new_oper = L_create_new_op (Lop_JUMP);
		      new_oper->src[0] = L_new_cb_operand (cbA);
		      L_insert_oper_after (cbC, cbC->last_op, new_oper);
		      dest_flow->cc = 1;
		    }

		  dest_flow->dst_cb = cbA;
		  new_flow = L_new_flow (dest_flow->cc, cbC, cbA,
					 dest_flow->weight);
		  cbA->src_flow = L_concat_flow (cbA->src_flow, new_flow);
		  /* don't delete cbB->src_flow, get to it later */
		}

	      /* Merge flow weights of cbA and cbB, delete src_flows
	       * coming from cbB. */
	      flow_switch = cbA->dest_flow;
	      flow_replace = cbB->dest_flow;
	      for (; flow_switch && flow_replace; )
		{
		  L_Cb * cbD;
		  cbD = flow_switch->dst_cb;
		  dest_flow = L_find_matching_flow (cbD->src_flow,
						    flow_switch);
		  flow_switch->weight += flow_replace->weight;
		  dest_flow->weight += flow_replace->weight;
		  dest_flow = L_find_matching_flow (cbD->src_flow,
						    flow_replace);
		  cbD->src_flow = L_delete_flow (cbD->src_flow, dest_flow);
		  flow_switch = flow_switch->next_flow;
		  flow_replace = flow_replace->next_flow;
		}

	      /* Update cbA's weight, delete cbB */
	      cbA->weight += cbB->weight;
	      L_delete_cb (fn, cbB);
	      STAT_COUNT ("Lopti_PCE_merge_same_cbs", 1, cbA);
	      change++;
	    }
	}
    }

  return change;
}


int
L_PCE_cleanup (L_Func * fn, int mov_flag)
{
  int j, change, local_change, total_change,
    c1, c2, c3, c4, c5, c6, c7, c8, c9, c10, c11, c12, c13, c14, c15,
    c16, c17, c18, c19, c20, c21, c22, c23, c24, c25, inserted, use_sync_arcs;
  L_Cb *cb, *cb2;

  c1 = c2 = c3 = c4 = c5 = c6 = c7 = c8 = c9 = c10 = c11 = c12 = c13 = c14 =
    c15 = c16 = c17 = c18 = c19 = c20 = c21 = c22 = c23 = c24 = c25 =
    local_change = total_change = use_sync_arcs = 0;

  L_partial_dead_code_removal (fn);
  L_do_flow_analysis (fn, LIVE_VARIABLE);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	continue;

      for (j = 0; j < MAX_PCE_LOCAL_CLEANUP_ITER; j++)
	{
#ifdef TRACK_PCE_CLEANUP_ITERATIONS
	  fprintf (stderr, "Iteration %d of PCE local cleanup in cb %d.\n",
		   j + 1, cb->id);
#endif

          if (Lopti_do_local_rev_copy_prop)
            {
	      c4 = L_local_rev_copy_propagation (cb);
	      STAT_COUNT ("Lopti_PCE_local_rev_copy_propagation", c4, cb);
            }

          if (Lopti_do_local_copy_prop)
            {
	      c2 = L_local_copy_propagation (cb);
	      STAT_COUNT ("Lopti_PCE_local_copy_propagation", c2, cb);
            }

          if (Lopti_do_local_dead_code_rem)
            {
	      c3 = L_local_dead_code_removal (cb);
	      STAT_COUNT ("Lopti_PCE_local_dead_code_removal", c3, cb);
            }

	  /* don't do constant prop when doing PRE on moves: ping-pong */
          if (Lopti_do_local_constant_prop)
            {
	      int flag = (Lopti_do_PRE &&
			  Lopti_do_PRE_optimize_single_source_ops &&
			  Lopti_do_PRE_optimize_moves);
	      c5 = L_local_constant_propagation (cb, flag);
	      STAT_COUNT ("Lopti_PCE_local_constant_propagation", c5, cb);
            }

          if (Lopti_do_local_mem_copy_prop)
            {
              c6 = L_local_memory_copy_propagation (cb);
              STAT_COUNT ("Lopti_PCE_local_memory_copy_propagation", c6, cb);
            }

          if (Lopti_do_local_common_sub_elim)
            {
	      if (mov_flag)
		c7 = L_local_common_subexpression (cb, L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT
						   | L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT);
	      else
		c7 = L_local_common_subexpression (cb, 0);
	      STAT_COUNT ("Lopti_PCE_local_common_subexpression", c7, cb);
            }

          if (Lopti_do_local_red_load_elim)
            {
              if (Lopti_ignore_sync_arcs_for_red_elim)
                {
                  use_sync_arcs = L_use_sync_arcs;
                  L_use_sync_arcs = 0;
                }
              c8 = L_local_redundant_load (cb);
              if (Lopti_ignore_sync_arcs_for_red_elim)
                {
                  L_use_sync_arcs = use_sync_arcs;
                }
              STAT_COUNT ("Lopti_PCE_local_redundant_load", c8, cb);
            }

	  if (Lopti_do_local_red_store_elim)
	    {
	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{
		  use_sync_arcs = L_use_sync_arcs;
		  L_use_sync_arcs = 0;
		}
	      c9 = L_local_redundant_store (cb);
	      if (Lopti_ignore_sync_arcs_for_red_elim)
		{
		  L_use_sync_arcs = use_sync_arcs;
		}
	      STAT_COUNT ("Lopti_PCE_local_redundant_store", c9, cb);
	    }

          if (Lopti_do_local_constant_comb)
            {
	      c10 = L_local_constant_combining (cb);
	      STAT_COUNT ("Lopti_PCE_local_constant_combining", c10, cb);
            }

          if (Lopti_do_local_constant_fold)
            {
	      c11 = L_local_constant_folding (cb);
	      STAT_COUNT ("Lopti_PCE_local_constant_folding", c11, cb);
            }

	  if (Lopti_do_local_strength_red)
	    {
	      c25 = L_local_strength_reduction (cb);
	      STAT_COUNT ("Lopti_PCE_local_strength_reduction", c25, cb);
	    }

	  /* Not used in SPEC 2000 */
          if (Lopti_do_local_branch_fold)
            {
	      c12 = L_local_branch_folding (cb);
	      STAT_COUNT ("Lopti_PCE_local_branch_folding", c12, cb);
            }

          if (Lopti_do_local_code_motion)
            {
	      c13 = L_local_code_motion (cb);
	      STAT_COUNT ("Lopti_PCE_local_code_motion", c13, cb);
            }

          if (Lopti_do_local_operation_fold)
            {
	      c14 = L_local_operation_folding (cb);
	      STAT_COUNT ("Lopti_PCE_local_operation_folding", c14, cb);
            }

	  /* Not used in SPEC 2000 */
          if (Lopti_do_local_operation_cancel)
            {
	      c15 = L_local_operation_cancellation (cb);
	      STAT_COUNT ("Lopti_PCE_local_operation_cancellation", c15, cb);
            }

          if (Lopti_do_local_reduce_logic)
            {
	      c16 = L_local_logic_reduction (cb);
	      STAT_COUNT ("Lopti_PCE_local_logic_reduction", c16, cb);
            }

	  if (Lopti_do_local_branch_val_prop)
	    {
	      c17 = L_local_branch_val_propagation (cb);
	      STAT_COUNT ("Lopti_PCE_local_branch_val_prop", c17, cb);
	    }

	  /* note that because of predicated instructions, it is possible
	   * to hit a ping-pong situation of insert-replace, local rev copy prop,
	   * and local dead removal. Thus, ignore these when checking for
	   * changes. */
	  change = c2 + c5 + c6 + c7 + c8 + c9 + c10 + c11 + c12 + c13 +
	           c14 + c15 + c16 + c17 + c25;
	  if (!change)
	    break;
	  local_change += change;
	}
    }
  total_change += local_change;

  if (Lopti_do_global_opti == 0)
    return total_change;

  /* SER: note that global constant prop is necessary only if PRE
   * affects moves of constants and expressions which include it
   */
  if (Lopti_do_PRE && Lopti_do_PCE_optimize_memory_ops &&
      Lopti_do_PRE_mem_copy_prop)
    L_do_flow_analysis (fn, DOMINATOR_CB | AVAILABLE_EXPRESSION |
			AVAILABLE_DEFINITION | REACHING_DEFINITION);
  else
    L_do_flow_analysis (fn, DOMINATOR_CB | MEM_AVAILABLE_DEFINITION |
			AVAILABLE_EXPRESSION | AVAILABLE_DEFINITION |
			REACHING_DEFINITION);
  L_compute_danger_info (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
	continue;

      for (cb2 = fn->first_cb; cb2 != NULL; cb2 = cb2->next_cb)
	{
	  if (cb == cb2)
	    continue;
	  if (!(L_in_cb_DOM_set (cb2, cb->id)))
	    continue;

	  /* don't do constant prop when doing PRE on moves: ping-pong */
          if (Lopti_do_global_constant_prop)
            {
	      int flag = (Lopti_do_PRE &&
			  Lopti_do_PRE_optimize_single_source_ops &&
			  Lopti_do_PRE_optimize_moves);
	      c18 = L_global_constant_propagation (cb, cb2, flag);
	      STAT_COUNT ("Lopti_PCE_global_constant_propagation", c18, NULL);
            }

	  if (Lopti_do_global_common_sub_elim && !Lopti_do_PRE)
	    {
	      c19 = L_global_common_subexpression (cb, cb2, 0);
	      STAT_COUNT ("Lopti_PCE_global_common_subexpression", c19, NULL);
	    }

          if (Lopti_do_global_copy_prop)
            {
	      c20 = L_global_copy_propagation (cb, cb2);
	      STAT_COUNT ("Lopti_PCE_global_copy_propagation", c20, NULL);
            }

	  if (!(Lopti_do_PCE_optimize_memory_ops && Lopti_do_PRE)) 
	    {
	      if (Lopti_do_global_red_load_elim)
		{
		  c21 = L_global_memflow_redundant_load (cb, cb2, &inserted);
		  STAT_COUNT ("Lopti_PCE_global_redundant_load", c21, NULL);
		}

	      if (Lopti_do_global_red_store_elim)
		{
		  c22 = L_global_memflow_redundant_store (cb, cb2);
		  STAT_COUNT ("Lopti_PCE_global_redundant_store", c22, NULL);
		}

	      if (!Lopti_do_PRE_mem_copy_prop && Lopti_do_global_mem_copy_prop)
		{
		  c23 = L_global_memflow_redundant_load_with_store (cb, cb2,
								    &inserted);
		  STAT_COUNT ("Lopti_PCE_global_memory_copy_propagation",
			      c23, NULL);
		}
	    }

	  if (Lopti_do_global_branch_val_prop)
	    {
	      c24 = L_global_branch_val_propagation (cb, cb2);
	      STAT_COUNT ("Lopti_PCE_global_branch_val_prop", c24, NULL);
	    }

	  total_change += c18 + c19 + c20 + c21 + c22 + c23 + c24;
	}
    }

  /* This is subsumed by the mem PRE extension. */
#if 0
  if (!(Lopti_do_PCE_optimize_memory_ops && Lopti_do_PRE_mem_copy_prop))
    total_change += L_global_mem_expression_copy_prop (fn);
#endif
  L_delete_all_danger_ext (fn);

  return total_change;
}
