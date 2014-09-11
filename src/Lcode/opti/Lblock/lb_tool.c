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
 *      File :          l_tool.c
 *      Description :   Block manipulation tools
 *      Creation Date : April 1995
 *      Authors :       David August
 *
 *      (C) Copyright 1995, David August
 *      All rights granted to University of Illinois Board of Regents.
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "lb_b_internal.h"
#include "lb_tool.h"


int
LB_remove_empty_cbs (L_Func * fn)
{
  L_Cb *cb, *cb_fix, *ncb;
  L_Flow *sfl, *nsfl, *dfl;
  L_Oper *branch;
  int cnt = 0;

  for (cb = fn->first_cb; cb; cb = ncb)
    {
      ncb = cb->next_cb;

      if (cb->first_op)
	continue;

      if (!(dfl = cb->dest_flow) || dfl->next_flow)
	L_punt ("LB_remove_empty_cbs: flow assumption violated");

      sfl = L_find_matching_flow (ncb->src_flow, dfl);

      ncb->src_flow = L_delete_flow (ncb->src_flow, sfl);

      for (sfl = cb->src_flow; sfl; sfl = nsfl)
	{
	  nsfl = sfl->next_flow;

	  cb_fix = sfl->src_cb;
	  dfl = L_find_matching_flow (cb_fix->dest_flow, sfl);

	  branch = L_find_branch_for_flow (cb_fix, dfl);

	  dfl->dst_cb = ncb;
	  sfl->dst_cb = ncb;

	  if (branch)
	    L_change_branch_dest (branch, cb, ncb);

	  cb->src_flow = L_remove_flow (cb->src_flow, sfl);
	  ncb->src_flow = L_concat_flow (ncb->src_flow, sfl);
	}

      L_delete_cb (fn, cb);
      cnt++;
    }

  return cnt;
}


/*
 *    Return number of ops contained in a set of cb's
 */
int
LB_hb_num_ops_in_cb_set (Set cb_set)
{
  int i, num_op, num_cb, *cb_array;
  L_Cb *cb;
  L_Oper *oper;

  num_cb = Set_size (cb_set);
  if (num_cb <= 0)
    return (0);

  cb_array = (int *) alloca (sizeof (int) * num_cb);
  Set_2array (cb_set, cb_array);

  num_op = 0;
  for (i = 0; i < num_cb; i++)
    {
      if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cb_array[i])))
	L_punt ("LB_hb_num_ops_in_cb_set: corrupt cb_set");
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	num_op++;
    }

  return (num_op);
}


int
LB_hb_jsr_in_cb_set (Set cb_set)
{
  int i, num_jsr, num_cb, *cb_array;
  L_Cb *cb;
  L_Oper *oper;

  num_cb = Set_size (cb_set);
  if (num_cb <= 0)
    return (0);

  cb_array = (int *) alloca (sizeof (int) * num_cb);
  Set_2array (cb_set, cb_array);

  num_jsr = 0;
  for (i = 0; i < num_cb; i++)
    {
      if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cb_array[i])))
	L_punt ("LB_hb_jsr_in_cb_set: corrupt cb_set");
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	if (L_subroutine_call_opcode (oper))
	  num_jsr++;
    }

  return (num_jsr > 0);
}


void
LB_hb_mark_all_cbs_with_attr (Set cb_set, L_Attr * attr)
{
  int i, num_cb, *cb_array;
  L_Attr *new_attr;
  L_Cb *cb;

  if (!(num_cb = Set_size (cb_set)))
    return;
  cb_array = (int *) alloca (sizeof (int) * num_cb);
  Set_2array (cb_set, cb_array);

  for (i = 0; i < num_cb; i++)
    {
      if (!(cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, cb_array[i])))
	L_punt ("LB_hb_mark_all_cbs_with_attr: corrupt cb_set");
      if (!L_find_attr (cb->attr, attr->name))
	{
	  new_attr = L_copy_attr (attr);
	  cb->attr = L_concat_attr (cb->attr, new_attr);
	}
    }
  return;
}


int
LB_fn_split_critical_edges (L_Func *fn)
{
  L_Cb *src_cb, *dst_cb;
  int cnt = 0;

  for (src_cb = fn->first_cb; src_cb; src_cb = src_cb->next_cb)
    {
      L_Flow *dfl, *sfl;

      if (!(dfl = src_cb->dest_flow) ||
	  !dfl->next_flow)
	continue;

      for (; dfl; dfl = dfl->next_flow)
	{
	  dst_cb = dfl->dst_cb;

	  sfl = L_find_matching_flow (dst_cb->src_flow, dfl);

	  if (!sfl->prev_flow && !sfl->next_flow)
	    continue;

	  L_split_arc (fn, src_cb, dfl);
	  cnt++;
	}
    }

  return cnt;
}


int
LB_convert_to_frp (L_Cb *cb, L_Oper *fr_op, L_Oper *to_op)
{
  L_Flow *fl, *mfl;
  L_Oper *op, *next_op, *new_cmp, *new_br;
  L_Operand *gpr;
  int has_fallthrough;

  if (!fr_op)
    fr_op = cb->first_op;

  for (op = fr_op; op; op = op->next_op)
    {
      if (op->pred[0])
	return 0;
      if (op == to_op)
	break;
    }

  /* point of no return */

  has_fallthrough = L_has_fallthru_to_next_cb (cb);

  gpr = NULL;

  for (op = fr_op; op; op = next_op)
    {
      next_op = op->next_op;

      if (op->pred[0])
	L_punt ("Lsuper_convert_to_frp: can't handle this case");

      if (gpr)
	{
	  op->pred[0] = L_copy_operand (gpr);
	  L_assign_ptype_null (op->pred[0]);
	}

      if (!L_cond_branch_opcode (op))
	continue;

      new_cmp = L_create_new_op (op->opc);
      L_change_to_cmp_op (new_cmp);
      L_copy_compare (new_cmp, op);

      new_cmp->attr = L_copy_attr (op->attr);
      new_cmp->weight = op->weight;

      if (op->pred[0])
	new_cmp->pred[0] = L_copy_operand (op->pred[0]);

      new_cmp->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
						 L_CTYPE_PREDICATE,
						 L_PTYPE_UNCOND_T);
      new_cmp->dest[1] = L_new_register_operand (++L_fn->max_reg_id,
						 L_CTYPE_PREDICATE,
						 L_PTYPE_UNCOND_F);
      new_cmp->src[0] = L_copy_operand (op->src[0]);
      gpr = new_cmp->src[1] = L_copy_operand (op->src[1]);

      new_br = L_create_new_op (Lop_JUMP);

      new_br->src[0] = L_new_cb_operand (op->src[2]->value.cb);
      new_br->pred[0] = L_copy_operand (new_cmp->dest[0]);
      L_assign_ptype_null (new_br->pred[0]);

      new_br->attr = L_copy_attr (op->attr);
      new_br->weight = op->weight;

      L_insert_oper_before (cb, op, new_cmp);
      L_insert_oper_after (cb, new_cmp, new_br);
      L_delete_oper (cb, op);

      if (op == to_op)
	break;
    }

  fl = L_find_last_flow (cb->dest_flow);

  if (!L_find_branch_for_flow (cb, fl))
    {
      mfl = L_find_matching_flow (fl->dst_cb->src_flow, fl);
      new_br = L_create_new_op (Lop_JUMP);
      new_br->src[0] = L_new_cb_operand (fl->dst_cb);
      new_br->pred[0] = L_copy_operand (gpr);
      L_assign_ptype_null (new_br->pred[0]);
      L_insert_oper_after (cb, cb->last_op, new_br);
      mfl->cc = fl->cc = 1;
    }

  if (gpr)
    {
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
      L_fn->flags = L_SET_BIT_FLAG (L_fn->flags, L_FUNC_HYPERBLOCK);
      if (!has_fallthrough)
	cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU);
    }

  return 1;
}

