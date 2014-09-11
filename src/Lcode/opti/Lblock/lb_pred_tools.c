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
 *	File :		l_pred_tools.c
 *	Description :	Useful tools for defining and using predicate
 *	Creation Date :	December 1997
 *	Authors : 	Kevin Crozier
 *      (C) Copyright 1997, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

#include <config.h>
#include <alloca.h>
#include "lb_b_internal.h"
#include <library/i_graph.h>

#undef DEBUG_PRED
#undef DEBUG_COMBINE

#define LB_FLOW_BACKWARD 1
#define LB_FLOW_EXIT     2
#define LB_FLOW_FORWARD  3

typedef struct _LB_Predicate {
  int id;
} LB_Predicate;

/*=========================================================================*/
/*
 *    Global vars for this file
 */
/*=========================================================================*/

static L_Cb *TEMP_CB;

static L_Alloc_Pool *LB_Predicate_Pool = NULL;


void
LB_predicate_init ()
{
  TEMP_CB = L_create_cb (0.0);
  return;
}


void
LB_predicate_deinit (L_Func * fn)
{
  L_delete_cb (fn, TEMP_CB);
  return;
}


static LB_Predicate *
LB_create_predicate (L_Func * fn)
{
  LB_Predicate *new_pred;

  if (!LB_Predicate_Pool)
    LB_Predicate_Pool = L_create_alloc_pool ("LB_Predicate",
					     sizeof (LB_Predicate), 16);

  new_pred = (LB_Predicate *) L_alloc (LB_Predicate_Pool);
  new_pred->id = ++fn->max_reg_id;
  return new_pred;
}


void
LB_free_predicate (LB_Predicate * pred)
{
  L_free (LB_Predicate_Pool, pred);
  return;
}


/*
 * LB_predicate_clear
 * ----------------------------------------------------------------------
 * Set up the predicates for the current hyperblock region, inserting
 * pred_clear operations at the top of the new block.
 */

static void
LB_predicate_clear (L_Func * fn, LB_TraceRegion * tr)
{
  L_Oper *new_op;
  GraphEquivCD equiv_cd;
  GraphNode start_node;
  GraphArc start_arc;
  LB_Predicate *pred;

  start_node = GraphRootNode (tr->flow_graph);
  start_arc = (GraphArc) List_get_first (start_node->succ);

  List_start (FlowGraph (tr)->equiv_cds);
  while ((equiv_cd = List_next (FlowGraph (tr)->equiv_cds)))
    {
      if (Set_empty (equiv_cd->contr_dep))
	continue;		/* True predicate */

      /* No need for a predicate for the set containing the start and
       * stop node arc.
       */

      if (Set_in (equiv_cd->contr_dep, start_arc->id))
	continue;

      pred = LB_create_predicate (fn);
      GraphEquivCDContents (equiv_cd) = pred;

#if 1
      new_op = L_create_new_op (Lop_PRED_CLEAR);
      new_op->dest[0] = L_new_register_operand (pred->id, L_CTYPE_PREDICATE,
						L_PTYPE_NULL);
#else
      new_op = L_create_new_op (Lop_CMP);
      L_set_compare (new_op, L_CTYPE_INT, Lcmp_COM_NE);
      new_op->src[0] = L_new_gen_int_operand (0);
      new_op->src[1] = L_new_gen_int_operand (0);
      new_op->dest[0] = L_new_register_operand (pred->id, L_CTYPE_PREDICATE,
						L_PTYPE_UNCOND_T);
#endif

      L_insert_oper_after (TEMP_CB, TEMP_CB->last_op, new_op);
    }
  return;
}


/*
 * LB_predicate_define
 * ----------------------------------------------------------------------
 * Insert the predicate defining operations into the current hyperblock
 * region, based on control dependence information.
 */

static void
LB_predicate_define (LB_TraceRegion * tr)
{
  L_Attr *attr;
  L_Flow *flow;
  L_Cb *cb;
  L_Oper *branch, *new_op;
  GraphEquivCD equiv_cd;
  GraphArc arc;
  LB_Predicate *pred;

  List_start (FlowGraph (tr)->equiv_cds);
  while ((equiv_cd = (GraphEquivCD) List_next (FlowGraph (tr)->equiv_cds)))
    {
      if (!(pred = GraphEquivCDContents (equiv_cd)))
	continue;

      List_start (equiv_cd->arcs);
      while ((arc = (GraphArc) List_next (equiv_cd->arcs)))
	{
	  cb = GraphNodeCB (arc->pred);
	  branch = cb->last_op;
	  if (L_uncond_branch_opcode (branch))
	    branch = branch->prev_op;
	  if (!L_cond_branch_opcode (branch))
	    L_punt ("L_insert_pred_defines: no conditional branch found");

	  new_op = L_create_new_op (branch->opc);
	  L_change_to_cmp_op (new_op);
	  L_copy_compare (new_op, branch);

	  if ((*(int *) arc->ptr) == FALLTHRU)
	    L_negate_compare (new_op);

	  /* Copy all attributes to new op - maybe not all of them? */
	  new_op->attr = L_copy_attr (branch->attr);
	  new_op->weight = branch->weight;
	  new_op->dest[0] = L_new_register_operand (pred->id, 
						    L_CTYPE_PREDICATE,
						    L_PTYPE_OR_T);
	  new_op->src[0] = L_copy_operand (branch->src[0]);
	  new_op->src[1] = L_copy_operand (branch->src[1]);
	  /* PD0 attributes preserve control flow profile onto pred defines.
	   * Field 0 contains the execution weight of the define;
	   * Field 1 contains the "TRUE" assignment weight.
	   */
	  attr = L_new_attr ("PD0", 2);
	  L_set_double_attr_field (attr, 0, cb->weight);
	  flow = L_find_flow_for_branch (cb, branch);
	  L_set_double_attr_field (attr, 1, flow->weight);
	  new_op->attr = L_concat_attr (new_op->attr, attr);
	  L_insert_oper_before (cb, branch, new_op);
	}
    }
  return;
}


static int
LB_flow_type (L_Flow * flow, LB_TraceRegion * tr)
{
  if (!flow)
    L_punt ("L_edge_type: flow is NULL");

  if (flow->dst_cb == LB_get_first_cb_in_region (tr))
    return LB_FLOW_BACKWARD;
  else if (!LB_return_cb_in_region (tr, flow->dst_cb->id))
    return LB_FLOW_EXIT;
  else
    return LB_FLOW_FORWARD;
}


static void
LB_predicate_this_oper (L_Cb *cb, L_Oper * oper, LB_Predicate * pred,
			double weight, double taken_weight)
{
  L_Attr *attr;

  if (oper->pred[0])
    {
      L_Oper *cmp_op;
      L_Operand *new_pg;

      L_warn ("LB_predicate_this_oper: Overriding original predicate "
	      "(op %d)\n", oper->id);

      cmp_op = L_create_new_op (Lop_CMP);
      L_set_compare (cmp_op, L_CTYPE_INT, Lcmp_COM_EQ);
      cmp_op->pred[0] = oper->pred[0];
      cmp_op->src[0] = L_new_gen_int_operand (0);
      cmp_op->src[1] = L_new_gen_int_operand (0);
      cmp_op->dest[0] = new_pg = 
	L_new_register_operand (++L_fn->max_reg_id, 
				L_CTYPE_PREDICATE, L_PTYPE_UNCOND_T);
      L_insert_oper_before (cb, oper, cmp_op);

      cmp_op = L_create_new_op (Lop_CMP);
      L_set_compare (cmp_op, L_CTYPE_INT, Lcmp_COM_EQ);
      cmp_op->pred[0] = L_new_register_operand (pred->id, L_CTYPE_PREDICATE,
						L_PTYPE_NULL);
      cmp_op->src[0] = L_new_gen_int_operand (0);
      cmp_op->src[1] = L_new_gen_int_operand (0);
      cmp_op->dest[0] = L_copy_operand (new_pg);
      L_assign_ptype_sand_true (cmp_op->dest[0]);
      L_insert_oper_before (cb, oper, cmp_op);

      oper->pred[0] = L_copy_operand (new_pg);
      L_assign_ptype_null (oper->pred[0]);
    }
  else
    {
      oper->pred[0] = L_new_register_operand (pred->id, L_CTYPE_PREDICATE,
					      L_PTYPE_NULL);
    }

  attr = L_new_attr ("wgt", 1);
  L_set_float_attr_field (attr, 0, weight);
  oper->attr = L_concat_attr (oper->attr, attr);
  if (L_cond_branch_opcode (oper))
    {
      attr = L_new_attr ("tkn", 1);
      L_set_float_attr_field (attr, 0, taken_weight);
      oper->attr = L_concat_attr (oper->attr, attr);
    }

  return;
}


static void
LB_predicate_branch (LB_TraceRegion * tr, L_Cb * cb, L_Oper * oper,
		     LB_Predicate * pred)
{
  int flow1_type, flow2_type;
  double frac, frac2;
  L_Cb *header;
  L_Flow *flow1, *flow2, *new_flow;
  L_Oper *new_op, *next;

  header = LB_get_first_cb_in_region (tr);
  if (L_num_dst_cb (cb) == 1)
    {
      if (oper && !L_uncond_branch_opcode (oper))
	L_punt ("ERROR! predicate_branch:  illegal block with 1 flow");

      flow1 = cb->dest_flow;
      flow1_type = LB_flow_type (flow1, tr);
      if (flow1_type != LB_FLOW_FORWARD)
	{
	  new_op = L_create_new_op (Lop_JUMP);
	  new_op->src[0] = L_new_cb_operand (flow1->dst_cb);
	  L_insert_oper_after (TEMP_CB, TEMP_CB->last_op, new_op);
	  if (pred)
	    {
	      frac = (oper && (header->weight != 0.0)) ?
		oper->weight / header->weight : 0.0;

	      LB_predicate_this_oper (TEMP_CB, new_op, pred, frac, 0.0);
	    }
	  new_flow = L_new_flow (1, header, flow1->dst_cb, flow1->weight);
	  TEMP_CB->dest_flow = L_concat_flow (TEMP_CB->dest_flow, new_flow);
	}
      /* delete branches corresponding to flows */
      L_delete_oper (cb, oper);
    }
  else if (L_num_dst_cb (cb) == 2)
    {
      /* error check */
      if (!oper || !L_cond_branch_opcode (oper))
	L_punt ("LB_predicate_branch: illegal block with 2 flows");

      flow1 = cb->dest_flow;
      flow2 = flow1->next_flow;
      flow1_type = LB_flow_type (flow1, tr);
      flow2_type = LB_flow_type (flow2, tr);

      /* 4 subcases [BE][BE], F[BE], [BE]F, FF (Backward, Exit, Forward) */
      if ((flow1_type != LB_FLOW_FORWARD) && 
	  (flow2_type != LB_FLOW_FORWARD))
	{
	  /* flow1 */
	  new_op = L_copy_operation (oper);
	  L_insert_oper_after (TEMP_CB, TEMP_CB->last_op, new_op);
	  if (pred)
	    {
	      if (header->weight != 0.0)
		{
		  frac = oper->weight / header->weight;
		  frac2 = flow1->weight / header->weight;
		}
	      else
		{
		  frac = 0.0;
		  frac2 = 0.0;
		}
	      LB_predicate_this_oper (TEMP_CB, new_op, pred, frac, frac2);
	    }
	  new_flow = L_new_flow (1, header, flow1->dst_cb, flow1->weight);
	  TEMP_CB->dest_flow = L_concat_flow (TEMP_CB->dest_flow, new_flow);
	  /* flow2 */
	  new_op = L_create_new_op (Lop_JUMP);
	  new_op->src[0] = L_new_cb_operand (flow2->dst_cb);
	  L_insert_oper_after (TEMP_CB, TEMP_CB->last_op, new_op);
	  if (pred)
	    {
	      frac = (header->weight != 0.0) ?
		flow2->weight / header->weight : 0.0;

	      LB_predicate_this_oper (TEMP_CB, new_op, pred, frac, 0.0);
	    }
	  new_flow = L_new_flow (1, header, flow2->dst_cb, flow2->weight);
	  TEMP_CB->dest_flow = L_concat_flow (TEMP_CB->dest_flow, new_flow);
	}
      else if ((flow1_type == LB_FLOW_FORWARD) &&
	       (flow2_type != LB_FLOW_FORWARD))
	{
	  /* flow2 */
	  new_op = L_copy_operation (oper);
	  L_negate_compare (new_op);
	  new_op->src[2]->value.cb = flow2->dst_cb;
	  L_insert_oper_after (TEMP_CB, TEMP_CB->last_op, new_op);
	  if (pred)
	    {
	      if (header->weight != 0.0)
		{
		  frac = oper->weight / header->weight;
		  frac2 = flow2->weight / header->weight;
		}
	      else
		{
		  frac = 0.0;
		  frac2 = 0.0;
		}
	      LB_predicate_this_oper (TEMP_CB, new_op, pred, frac, frac2);
	    }
	  new_flow = L_new_flow (1, header, flow2->dst_cb, flow2->weight);
	  TEMP_CB->dest_flow = L_concat_flow (TEMP_CB->dest_flow, new_flow);
	}
      else if ((flow1_type != LB_FLOW_FORWARD) &&
	       (flow2_type == LB_FLOW_FORWARD))
	{
	  /* flow1 */
	  new_op = L_copy_operation (oper);
	  L_insert_oper_after (TEMP_CB, TEMP_CB->last_op, new_op);
	  if (pred)
	    {
	      if (header->weight != 0.0)
		{
		  frac = oper->weight / header->weight;
		  frac2 = flow1->weight / header->weight;
		}
	      else
		{
		  frac = 0.0;
		  frac2 = 0.0;
		}
	      LB_predicate_this_oper (TEMP_CB, new_op, pred, frac, frac2);
	    }
	  new_flow = L_new_flow (1, header, flow1->dst_cb, flow1->weight);
	  TEMP_CB->dest_flow = L_concat_flow (TEMP_CB->dest_flow, new_flow);
	}
      /* delete branches corresponding to flows */
      next = oper->next_op;
      L_delete_oper (cb, oper);
      if (next)
	L_delete_oper (cb, next);
    }
  else
    {
      L_punt ("LB_predicate_branch: illegal number of targets of cb");
    }
  return;
}


static void
LB_add_fall_thru_flow_arc (L_Cb * cb)
{
  L_Oper *oper;
  L_Flow *flow, *new_flow;

  /* is there a fall thru arc there already, if yes no need to insert one */

  for (oper = cb->last_op; oper; oper = oper->prev_op)
    if (L_general_branch_opcode (oper))
      break;

  if (!oper)
    {
      if (cb->dest_flow)
	return;
    }
  else
    {
      flow = L_find_flow_for_branch (cb, oper);
      if (flow->next_flow)
	return;

      /*
       * JWP - L_has_fallthru_to_next_cb() assumes a valid CFG and returns
       * false if(!cb->dest_flow). In this case, the CFG may not be valid
       * since cb may not contain a flow arc.  So only check for fallthru
       * case when no branches are found in the current CB.
       */

      /* return if dont have fallthru path */
      if (!L_has_fallthru_to_next_cb (cb))
	{
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	    cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
	  return;
	}
    }


  /* insert fallthru flow arc */
  new_flow = L_new_flow (0, cb, cb->next_cb, 0.0);
  cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);
  return;
}


static void
LB_add_dummy_block_to_end_of_fn (L_Func * fn, L_Cb * cb)
{
  L_Oper *new_op;
  L_Cb *new_cb;
  L_Flow *new_flow;

  if ((cb == fn->last_cb) && L_has_fallthru_to_next_cb (cb))
    {
      L_punt ("L_add_dummy_block_to_end_of_fn: should never enter here!!");
      new_cb = L_create_cb (0.0);
      L_insert_cb_after (fn, fn->last_cb, new_cb);
      new_op = L_create_new_op (Lop_JUMP);
      new_op->src[0] = L_new_cb_operand (new_cb);
      L_insert_oper_after (new_cb, new_cb->last_op, new_op);
      new_flow = L_new_flow (1, new_cb, new_cb, 0.0);
      new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
    }
  return;
}


static void
LB_predicate_region (L_Func * fn, LB_TraceRegion * tr)
{
  L_Cb *cb, *header;
  L_Oper *oper, *next;
  GraphNode node;
  double weight;
  LB_Predicate *pred;
  int is_predicated  = 0;

  header = LB_get_first_cb_in_region (tr);
  List_start (FlowGraphTopoList (tr));
  while ((node = LB_next_graphnode (tr)))
    {
      if ((pred = (LB_Predicate *)GraphEquivCDContents(node->equiv_cd)))
	is_predicated = 1;

      cb = GraphNodeCB (node);
      for (oper = cb->first_op; oper; oper = next)
	{
	  next = oper->next_op;
	  if (L_general_branch_opcode (oper))
	    break;
	  
	  weight = (header->weight != 0.0) ?
	    oper->weight / header->weight :
	    0.0;

	  L_remove_oper (cb, oper);
	  L_insert_oper_after (TEMP_CB, TEMP_CB->last_op, oper);

	  if (pred)
	    LB_predicate_this_oper (TEMP_CB, oper, pred, weight, 0.0);
	}
      LB_predicate_branch (tr, cb, oper, pred);
    }

  /* fix up the TEMP CB so it can replace the current list of cbs. */

 // if (header->first_op || header->last_op)
   // L_punt ("L_cleanup: corrupt oper list for header");

  L_delete_all_flow (header->src_flow);
  header->src_flow = NULL;
  L_delete_all_flow (header->dest_flow);
  header->dest_flow = NULL;

  /* put opers/flows of TEMP_CB into header */
  header->first_op = TEMP_CB->first_op;
  TEMP_CB->first_op = NULL;
  header->last_op = TEMP_CB->last_op;
  TEMP_CB->last_op = NULL;
  header->dest_flow = TEMP_CB->dest_flow;
  TEMP_CB->dest_flow = NULL;

  {
    L_Oper *fix_op;

    for (fix_op = header->first_op; fix_op; fix_op = fix_op->next_op)
      L_oper_hash_tbl_update_cb (fn->oper_hash_tbl, fix_op->id, header);
  }

  /* delete all other cbs */
  List_start (FlowGraphTopoList (tr));
  LB_next_cb_in_region (tr);	/* skip the header cb */
  while ((cb = LB_next_cb_in_region (tr)))
    L_delete_cb (fn, cb);

  /* mark header as a hyperblock  or superblock */
  if (is_predicated)
    {
      header->flags = L_SET_BIT_FLAG (header->flags, L_CB_HYPERBLOCK);
      if (!L_EXTRACT_BIT_VAL (tr->flags, L_TRACEREGION_FALLTHRU))
	header->flags =
	  L_SET_BIT_FLAG (header->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
      fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
    }
  else
    {
      header->flags = L_SET_BIT_FLAG (header->flags, L_CB_SUPERBLOCK);
      fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_SUPERBLOCK);
      header->flags = L_CLR_BIT_FLAG (header->flags, L_CB_HYPERBLOCK);
      header->flags = L_CLR_BIT_FLAG (header->flags, L_CB_HYPERBLOCK_LOOP);
      header->flags = L_CLR_BIT_FLAG (header->flags, L_CB_HYPERBLOCK_HAMMOCK);
    }

  /* Build and rebuild any time predicates get messed with */
  PG_setup_pred_graph (fn);

  /* rebuild traceregion graph */
  LB_free_flow_graph (FlowGraph (tr));
  FlowGraph (tr) = LB_create_flow_graph (L_fn, header, NULL);
  LB_finish_frp_graph (FlowGraph (tr));

  /* Now must do a few corrections to ensure Lcode semantics are maintained!
   * 1. last instr of hyperblock is conditional branch or predicated jump
   * fall thru flow arc may need be inserted if not there to ensure
   * all possible flow paths are accounted for even though this
   * may never be taken.
   * 2. last block in function is a hyperblock, if has fallthru path, must
   * create a dummy block to end function.
   * 3. If predicate clears are inserted in block with prologue, make sure
   * they occur after the prologue. 
   */

  LB_add_fall_thru_flow_arc (header);

  LB_add_dummy_block_to_end_of_fn (fn, header);

  L_rebuild_src_flow (fn);

  return;
}


static void
LB_combine_tr_exits (LB_TraceRegion *tr)
{
  List eflows = NULL;
  Set cbset;
  int cbcnt, *cbbuf, i;

  cbset = LB_return_cbs_region_as_set (tr);
  cbcnt = Set_size (cbset);

  if (!cbcnt)
    return;

  cbbuf = alloca (cbcnt * sizeof (int));

  Set_2array (cbset, cbbuf);

  /* Collect eflows, set of flows that leave the hyperblock region */

  for (i = 0; i < cbcnt; i++)
    {
      L_Cb *cb;
      L_Flow *oflow;
      int cbid = cbbuf[i];

      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cbid);

      oflow = cb->dest_flow;

      while (oflow)
	{
	  if (!Set_in (cbset, oflow->dst_cb->id))
	    eflows = List_insert_last (eflows, oflow);
	  oflow = oflow->next_flow;
	}
    }

  if (List_size (eflows) > 1)
    {
      L_Flow *fa, *fb;
      L_Cb *mrg_cb = NULL;

      while ((fa = (L_Flow *) List_first (eflows)))
	{
	  L_Cb *tgt_cb;
	  int cnt;

	  mrg_cb = NULL;
	  tgt_cb = fa->dst_cb;

	  eflows = List_delete_current (eflows);
	  List_start (eflows);
	  while ((fb = (L_Flow *) List_next (eflows)))
	    {
	      if (fb->dst_cb == tgt_cb)
		break;
	    }

	  if (fb)
	    {
	      L_Flow *fm, *ft;
	      L_Oper *br;

	      /* There are at least two flows to fa->dst_cb.
	       * Start combining...
	       */

	      cnt = 1;

	      mrg_cb = L_create_cb (fa->weight);
	      L_insert_cb_after (L_fn, L_fn->last_cb, mrg_cb);

	      br = L_create_new_op (Lop_JUMP);
	      br->src[0] = L_new_cb_operand (tgt_cb);
	      L_insert_oper_after (mrg_cb, mrg_cb->last_op, br);

	      mrg_cb->dest_flow = L_new_flow (1, mrg_cb, fa->dst_cb, 
					      fa->weight);
	      fm = L_find_matching_flow (tgt_cb->src_flow, fa);
	      tgt_cb->src_flow = L_remove_flow (tgt_cb->src_flow, fm);
	      mrg_cb->src_flow = L_concat_flow (mrg_cb->src_flow, fm);
	      fm->dst_cb = mrg_cb;
	      fm->cc = 1;

	      ft = L_copy_single_flow (fm);
	      tgt_cb->src_flow = L_concat_flow (tgt_cb->src_flow, ft);
	      ft->dst_cb = tgt_cb;
	      ft->src_cb = mrg_cb;

	      if ((br = L_find_branch_for_flow (fa->src_cb, fa)))
		{
		  L_change_branch_dest (br, tgt_cb, mrg_cb);
		}
	      else
		{
		  br = L_create_new_op (Lop_JUMP);
		  br->src[0] = L_new_cb_operand (mrg_cb);
		  L_insert_oper_after (fa->src_cb, fa->src_cb->last_op, br);
		  fa->cc = 1;
		}
	      fa->dst_cb = mrg_cb;

	      do
		{
		  if (fb->dst_cb == tgt_cb)
		    {
		      cnt++;
		      mrg_cb->weight += fb->weight;
		      mrg_cb->dest_flow->weight += fb->weight;
		      ft->weight += fb->weight;
		      fm = L_find_matching_flow (tgt_cb->src_flow, fb);
		      tgt_cb->src_flow = L_remove_flow (tgt_cb->src_flow, fm);
		      mrg_cb->src_flow = L_concat_flow (mrg_cb->src_flow, fm);
		      fm->dst_cb = mrg_cb;
		      if ((br = L_find_branch_for_flow (fb->src_cb, fb)))
			{
			  L_change_branch_dest (br, tgt_cb, mrg_cb);
			}
		      else
			{
			  br = L_create_new_op (Lop_JUMP);
			  br->src[0] = L_new_cb_operand (mrg_cb);
			  L_insert_oper_after (fb->src_cb, 
					       fb->src_cb->last_op, br);
			  fb->cc = 1;
			}
		      fb->dst_cb = mrg_cb;
		      eflows = List_delete_current (eflows);
		    }
		}
	      while ((fb = (L_Flow *) List_next (eflows)));

#ifdef DEBUG_COMBINE
	      fprintf (stderr, "Combined %d flows "
		       "from hb %d to cb %d --> mrg %d\n",
		       cnt, tr->header->id, tgt_cb->id, mrg_cb->id);
#endif
	      
	      if (mrg_cb)
		{
		  cbset = Set_add (cbset, mrg_cb->id);
		  LB_free_flow_graph (tr->flow_graph);
		  tr->flow_graph = LB_create_flow_graph (L_fn, tr->header, 
							 cbset);
		}
	    }
	}
    }

  List_reset (eflows);
  Set_dispose (cbset);

  return;
}


static int
LB_materialize_tr_exits (LB_TraceRegion *tr)
{
  List eflows = NULL;
  L_Cb *cb, *hdr;
  L_Flow *fl;
  L_Oper *new_op;
  Set cbset;
  int cbcnt, *cbbuf, i, chg = 0;

  cbset = LB_return_cbs_region_as_set (tr);
  if (!(cbcnt = Set_size (cbset)))
    {
      if (cbset)
	Set_dispose (cbset);
      return chg;
    }

  hdr = tr->header;

  cbbuf = alloca (cbcnt * sizeof (int));

  Set_2array (cbset, cbbuf);

  /* Collect eflows, set of flows that leave the hyperblock region */

  for (i = 0; i < cbcnt; i++)
    {
      L_Flow *iflow, *oflow;
      int cbid = cbbuf[i];

      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cbid);

      for (oflow = cb->dest_flow; oflow; oflow = oflow->next_flow)
	{
	  if (!Set_in (cbset, oflow->dst_cb->id) ||
	      oflow->dst_cb->id == hdr->id)
	    eflows = List_insert_last (eflows, oflow);
	}

      if (L_has_fallthru_to_next_cb (cb))
	{
	  oflow = L_find_last_flow (cb->dest_flow);
	  iflow = L_find_matching_flow (oflow->dst_cb->src_flow, oflow);
	  oflow->cc = iflow->cc = 1;
	  new_op = L_create_new_op (Lop_JUMP);
	  new_op->src[0] = L_new_cb_operand (oflow->dst_cb);
	  L_insert_oper_after (cb, cb->last_op, new_op);
	}
    }

  if (List_size (eflows))
    {
      List_start (eflows);
      while ((fl = (L_Flow *) List_next (eflows)))
	{
	  L_Cb *src_cb, *new_cb;

	  src_cb = fl->src_cb;
	  new_cb = L_split_arc (L_fn, src_cb, fl);

	  cbset = Set_add (cbset, new_cb->id);
	  chg++;

	  if (L_has_fallthru_to_next_cb (new_cb))
	    {
	      L_Flow *iflow, *oflow;
	      oflow = L_find_last_flow (new_cb->dest_flow);
	      iflow = L_find_matching_flow (oflow->dst_cb->src_flow, oflow);
	      oflow->cc = iflow->cc = 1;
	      new_op = L_create_new_op (Lop_JUMP);
	      new_op->src[0] = L_new_cb_operand (oflow->dst_cb);
	      L_insert_oper_after (new_cb, new_cb->last_op, new_op);
	    }
	}
    }

  if (chg)
    {
      LB_free_flow_graph (tr->flow_graph);
      tr->flow_graph = LB_create_flow_graph (L_fn, tr->header, cbset);
    }

  List_reset (eflows);
  Set_dispose (cbset);

  return chg;
}


static void
LB_mark_traceregion (L_Func *fn, LB_TraceRegion *tr)
{
  GraphNode node;
  Graph_dfs_topo_sort (tr->flow_graph);
  List_start (tr->flow_graph->topo_list);
  while ((node = LB_next_graphnode (tr)))
    {
      L_Cb *cb = ((LB_BB *) GraphNodeContents (node))->cb;
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
    }
  return;
}


void
LB_predicate_traceregion (L_Func * fn, LB_TraceRegion * tr, int do_frp)
{
  if (LB_hb_do_combine_exits)
    LB_combine_tr_exits (tr);


  if (do_frp)
    LB_materialize_tr_exits (tr);

  /* force an actual toposort to take place (want bfs list!) */

  (FlowGraph (tr))->flags &= ~GRAPHTOPOSORT;

  /* topo sort the graph */
  Graph_bfs_topo_sort (FlowGraph (tr));

  LB_traceregion_set_fallthru_flag (tr);

  /* dom, post dom, and imm post dom info */
  Graph_dominator (FlowGraph (tr));
  Graph_post_dominator (FlowGraph (tr));
  Graph_imm_post_dominator (FlowGraph (tr));

  /* control dependencies */
  Graph_control_dependence (FlowGraph (tr));

  /* equiv cd determination */
  Graph_equiv_cd (FlowGraph (tr));

#ifdef DEBUG_PRED
  LB_print_tr (stderr, tr);
  Graph_daVinci (FlowGraph (tr), "Graph.daVinci", LB_bb_print_hook);
#endif


  if (List_size (FlowGraph (tr)->equiv_cds) > 1)
    {
      /* There exist at least two equivalent CD sets -- need to predicate */

      /* pred clears and pred defines */
      LB_predicate_clear (fn, tr);

      LB_predicate_define (tr);

      /* predicate blocks */
      LB_predicate_region (fn, tr);
    }


  return;
}


void
LB_predicate_traceregions (L_Func * fn, LB_TraceRegion_Header * header)
{
  LB_TraceRegion *tr;
  int do_frp;

  do_frp = !strcmp (LB_predicate_formation_type, "frp");

  LB_predicate_init ();

  List_start (header->traceregions);
  while ((tr = (LB_TraceRegion *) List_next (header->traceregions)))
    {
      if (!LB_setup_only)
	LB_predicate_traceregion (fn, tr, do_frp);
      else
	LB_mark_traceregion (fn, tr);
    }

  LB_predicate_deinit (fn);

  return;
}


void
LB_set_hyperblock_flag (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_LOOP |
			   L_CB_HYPERBLOCK_HAMMOCK))
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
  return;
}


void
LB_remove_unnec_hyperblock_flags (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;

      for (oper = cb->first_op; oper; oper = oper->next_op)
	if (L_is_predicated (oper))
	  break;

      if (!oper)
	{
	  cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
	  cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
	}
    }
  return;
}


void
LB_set_hyperblock_func_flag (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
      break;

  if (cb)
    fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
  else
    fn->flags = L_CLR_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
  return;
}


void
LB_clr_hyperblock_flag (L_Func * fn)
{
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;
      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
    }
  return;
}

