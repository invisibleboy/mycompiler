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
/*****************************************************************************
 * l_unification.c (20011126)                                                *
 * ------------------------------------------------------------------------- *
 * Unification at split / merge points                                       *
 *                                                                           *
 * AUTHORS: J.W. Sias                                                        *
 *****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_opti.h>
#include <library/i_hash.h>

static L_Alloc_Pool *UniPath_pool = NULL;

typedef struct _UniPath {
  HashTable map;
  L_Cb *cb;
  L_Oper *op, *mop;
  Set def, use, sync, remap;
} UniPath;


static UniPath *
Luni_new_unipath (L_Cb *cb, L_Oper *op) 
{
  UniPath *up;

  up = (UniPath *) L_alloc (UniPath_pool);
  up->map = HashTable_create (64);
  up->cb = cb;
  up->op = op;
  up->mop = NULL;
  up->def = NULL;
  up->use = NULL;
  up->sync = NULL;
  up->remap = NULL;
  return up;
}


static UniPath *
Luni_free_unipath (UniPath *up)
{
  if (!up)
    return NULL;

  HashTable_free (up->map);
  up->map = NULL;

  if (up->def)
    Set_dispose (up->def);

  if (up->use)
    Set_dispose (up->use);

  if (up->sync)
    Set_dispose (up->sync);

  if (up->remap)
    Set_dispose (up->remap);

  L_free (UniPath_pool, up);

  return NULL;
}


static void
Luni_clean_unipath (UniPath *up)
{
  if (!up)
    return;

  if (up->def)
    up->def = Set_dispose (up->def);

  if (up->use)
    up->use = Set_dispose (up->use);

  if (up->sync)
    up->sync = Set_dispose (up->sync);

  if (up->remap)
    up->remap = Set_dispose (up->remap);

  up->mop = NULL;

  return;
}

static List
L_construct_merge_list (L_Cb *cb)
{
  L_Flow *src_flow, *dst_flow;
  L_Cb *src_cb;
  L_Oper *br_op;
  UniPath *up;
  int count = 0;
  List mlist = NULL;
  Set live_in;

  /* Check that predecessors are "single-successor" */

  for (src_flow = cb->src_flow; src_flow; src_flow = src_flow->next_flow)
    {
      src_cb = src_flow->src_cb;

      if ((src_cb == cb) ||
	  (src_cb->dest_flow->next_flow) ||
	  !(dst_flow = L_find_matching_flow (src_cb->dest_flow, src_flow)))
	break;
      
      br_op = L_find_branch_for_flow (src_cb, dst_flow);

      count++;
    }

  live_in = L_get_cb_IN_set (cb);

  if (!src_flow && (count >= 2))
    {
      /* Valid merge point -- construct list */

      for (src_flow = cb->src_flow; src_flow; src_flow = src_flow->next_flow)
	{
	  src_cb = src_flow->src_cb;
	  dst_flow = L_find_matching_flow (src_cb->dest_flow, src_flow);
	  br_op = L_find_branch_for_flow (src_cb, dst_flow);
	  up = Luni_new_unipath (src_cb, br_op);

	  up->use = Set_copy (live_in);

	  mlist = List_insert_last (mlist, (void *)up);
	}
    }

  return mlist;
}


static int
L_two_way_split (L_Cb *cb, L_Oper *br, L_Cb **plt, L_Cb **prt, L_Oper **pfoplt)
{
  L_Cb *lt, *rt;
  L_Oper *foplt;
  L_Flow *lfl, *rfl;

  if (!L_cond_branch (br))
    return 0;

  rfl = L_find_flow_for_branch (cb, br);

  if (!rfl)
    return 0;

  rt = rfl->dst_cb;

  if (!L_single_predecessor_cb (rt))
    return 0;

  if (br->next_op)
    {
      /* branch in middle of superblock */

      lt = cb;
      foplt = br->next_op;
    }
  else if ((lfl = rfl->next_flow))
    {
      /* branch ends a cb */

      lt = lfl->dst_cb;
      foplt = lt->first_op;
      if (!L_single_predecessor_cb (lt))
	return 0;
    }
  else
    {
      return 0;
    }

  if (plt)
    *plt = lt;

  if (prt)
    *prt = rt;

  if (pfoplt)
    *pfoplt = foplt;

  return 1;
}


#if 0
static int
L_two_way_merge (L_Cb *cb, L_Cb **plt, L_Cb **prt)
{
  L_Cb *lt, *rt;
  L_Flow *lfl, *rfl;

  if (!(lfl = cb->src_flow) ||
      !(rfl = lfl->next_flow) ||
      rfl->next_flow)
    return 0;

  lt = lfl->src_cb;
  rt = rfl->src_cb;

  if (lt->dest_flow->next_flow ||
      rt->dest_flow->next_flow )
    return 0;

  if (plt)
    *plt = lt;

  if (prt)
    *prt = rt;

  return 1;
}
#endif

static Set
L_sync_dep_opers_acc (Set sdset, L_Oper *op)
{
  L_Sync_Info *si;
  int i;

  si = op->sync_info;

  if (si)
    for (i=0; i < si->num_sync_out; i++)
      sdset = Set_add (sdset, si->sync_out[i]->dep_oper->id);

  return sdset;
}


#if 0
static Set
L_sync_sup_opers_acc (Set sdset, L_Oper *op)
{
  L_Sync_Info *si;
  int i;

  si = op->sync_info;

  if (si)
    for (i=0; i < si->num_sync_in; i++)
      sdset = Set_add (sdset, si->sync_in[i]->dep_oper->id);

  return sdset;
}
#endif


static Set
L_defined_rmid_acc (Set rmset, L_Oper *op)
{
  int i;
  L_Attr *attr;

  for (i = 0; i < L_max_dest_operand; i++)
    if (L_is_variable (op->dest[i]))
      rmset = Set_add (rmset, 
		       L_REG_MAC_INDEX (op->dest[i]));

  if ((attr = L_find_attr (op->attr, "ret")))
    for (i = 0; i < attr->max_field; i++)
      {
	L_Operand *dest;

	if ((dest = attr->field[i]) &&
	    L_is_variable (dest))
	  rmset = Set_add (rmset, 
			   L_REG_MAC_INDEX (dest));
      }

  return rmset;
}


static Set
L_consumed_rmid_acc (Set rmset, L_Oper *op)
{
  int i;
  L_Attr *attr;

  for (i=0; i < L_max_src_operand; i++)
    if (L_is_variable (op->src[i]))
      rmset = Set_add (rmset, 
		       L_REG_MAC_INDEX (op->src[i]));

  if ((attr = L_find_attr (op->attr, "tr")))
    for (i = 0; i < attr->max_field; i++)
      {
	L_Operand *src;

	if ((src = attr->field[i]) &&
	    L_is_variable (src))
	  rmset = Set_add (rmset, 
			   L_REG_MAC_INDEX (src));
      }

  return rmset;
}


void
L_update_unipath (UniPath *up, L_Oper *op)
{
  up->def = L_defined_rmid_acc (up->def, op);
  up->use = L_consumed_rmid_acc (up->use, op);
  up->sync = L_sync_dep_opers_acc (up->sync, op);
  return;
}


static L_Oper *
L_insert_compensation_mov_before(L_Cb *cb, L_Oper *before, 
				 L_Operand *dest, L_Operand *src)
{
  int opc;
  L_Oper *mov_op;

  switch (src->ctype)
    {
    case L_CTYPE_DOUBLE:
      opc = Lop_MOV_F2;
      break;
    case L_CTYPE_FLOAT:
      opc = Lop_MOV_F;
      break;
    default:
      opc = Lop_MOV;
    }

  mov_op = L_create_new_op (opc);
  mov_op->src[0] = L_copy_operand (src);
  mov_op->dest[0] = L_copy_operand (dest);
  L_insert_oper_before (cb, before, mov_op);

  return mov_op;
}

#if 0
static L_Oper *
L_insert_compensation_mov_after(L_Cb *cb, L_Oper *after, 
				 L_Operand *dest, L_Operand *src)
{
  int opc;
  L_Oper *mov_op;

  switch (src->ctype)
    {
    case L_CTYPE_DOUBLE:
      opc = Lop_MOV_F2;
      break;
    case L_CTYPE_FLOAT:
      opc = Lop_MOV_F;
      break;
    default:
      opc = Lop_MOV;
    }

  mov_op = L_create_new_op (opc);
  mov_op->src[0] = L_copy_operand (src);
  mov_op->dest[0] = L_copy_operand (dest);
  L_insert_oper_after (cb, after, mov_op);

  return mov_op;
}
#endif

static int
L_is_compensation_mov (HashTable hash, L_Oper *oper)
{
  int did, sid, mid;

  if (!L_move_opcode (oper))
    return 0;

  if (!L_is_variable (oper->src[0]) || !L_is_variable (oper->dest[0]))
    return 0;

  did = L_REG_MAC_INDEX (oper->dest[0]);
  sid = L_REG_MAC_INDEX (oper->src[0]);

  if (!(mid = (int)(long) HashTable_find_or_null (hash, did)) || 
      mid != sid)
    return 0;

  return 1;
}

/*
 * L_conflicting_src
 * ----------------------------------------------------------------------
 * Does any source of op match the reg/mac id set rmset?
 */

static int
L_conflicting_src (Set rmset, L_Oper *op)
{
  int i;
  L_Attr *attr;
  L_Operand *src;

  if (!rmset)
    return 0;

  for (i=0; i < L_max_src_operand; i++)
    if ((src = op->src[i]) &&
	L_is_variable (src) &&
	Set_in (rmset, L_REG_MAC_INDEX (src)))
      break;

  if (i < L_max_src_operand)
    return 1;

  if ((attr = L_find_attr (op->attr, "tr")))
    {
      for (i = 0; i < attr->max_field; i++)
	if ((src = attr->field[i]) &&
	    L_is_variable (src) &&
	    Set_in (rmset, L_REG_MAC_INDEX (src)))
	  break;

      if (i < attr->max_field)
	return 1;
    }

  return 0;
}

/*
 * L_conflicting_dest
 * ----------------------------------------------------------------------
 * Does any dest of op match the reg/mac id set rmset?
 */

static int
L_conflicting_dest (Set rmset, L_Oper *op)
{
  int i;
  L_Attr *attr;
  L_Operand *dest;

  if (!rmset)
    return 0;

  for (i=0; i < L_max_dest_operand; i++)
    if ((dest = op->dest[i]) &&
	L_is_variable (dest) &&
	Set_in (rmset, L_REG_MAC_INDEX (dest)))
      break;

  if (i < L_max_dest_operand)
    return 1;

  if ((attr = L_find_attr (op->attr, "ret")))
    {
      for (i = 0; i < attr->max_field; i++)
	if ((dest = attr->field[i]) &&
	    L_is_variable (dest) &&
	    Set_in (rmset, L_REG_MAC_INDEX (dest)))
	  break;

      if (i < attr->max_field)
	return 1;
    }

  return 0;
}

static int
L_rename_operand (HashTable nh, L_Operand *opd)
{
  int id;

  id = L_REG_MAC_INDEX (opd);

  if ((id = (int)(long) HashTable_find_or_null (nh, id)))
    {
      opd->value.r = L_UNMAP_REG (id);
      opd->type = L_OPERAND_REGISTER;
      return 1;
    }
  return 0;
}


static int
Luni_up_candidate (UniPath *up, L_Oper *op)
{
  int rv;

  rv = !Set_in (up->sync, op->id) &&
    !L_conflicting_src (up->def, op) &&
    !L_is_predicated (op) &&
    !L_pred_define_opcode (op) &&
    !(L_subroutine_call_opcode (op) &&
      (L_conflicting_dest (up->use, op) ||
       L_conflicting_dest (up->def, op)));

  return rv;
}


static int
Luni_legal_up_unification (UniPath *lp, L_Oper *op_lt,
			   UniPath *rp, L_Oper *op_rt,
			   L_Oper *sbr)
{
  int i;

  for (i=0; i < L_max_src_operand; i++)
    {
      L_Operand *src_lt = op_lt->src[i], 
	*src_rt = op_rt->src[i];

      if (L_is_variable (src_lt) &&
	  L_is_variable (src_rt))
	{
	  int lid, rid, rlid, rrid;
	  
	  lid = L_REG_MAC_INDEX (src_lt);
	  rid = L_REG_MAC_INDEX (src_rt);

	  /* can unify if renamed already */
				    
	  if ((rlid = (int)(long) 
	       HashTable_find_or_null (lp->map, 
				       lid)) &&
	      (rrid = (int)(long) 
	       HashTable_find_or_null (rp->map, 
				       rid)) &&
	      (rlid == rrid) &&
	      Set_in (lp->remap, rlid) &&
	      Set_in (rp->remap, rrid))
	    continue;
				  
	  /* can't unify if right-redefined */
				      
	  if (Set_in (rp->def, rid))
	    break;
				  
	  if (lid == rid)
	    continue;
				  
	  /* otherwise, can't unify */
	  break;
	}
      else if (!L_same_operand (src_lt, src_rt))
	{
	  /* otherwise, can't unify */
	  break;
	}
    }

  if (i != L_max_src_operand)
    return 0;

  /* Check for non-sync-arc memory dependences */

  if (L_general_load_opcode (op_lt) ||
      L_general_store_opcode (op_lt) ||
      L_subroutine_call_opcode (op_lt))
    {
      L_Oper *pop;

      pop = op_lt->prev_op;
  
      while (pop && (pop != sbr))
	{
	  if (L_general_load_opcode (pop) ||
	      L_general_store_opcode (pop) ||
	      L_subroutine_call_opcode (pop))
	    {
	      if (PG_intersecting_predicates_ops (pop, op_lt) &&
		  (L_is_ida_memory_ops (lp->cb, pop, lp->cb, op_lt, 
					SET_NONLOOP_CARRIED(0)) != MEM_IND))
		return 0;
	    }
	  pop = pop->prev_op;
	}

      pop = op_rt->prev_op;
  
      while (pop && (pop != sbr))
	{
	  if (L_general_load_opcode (pop) ||
	      L_general_store_opcode (pop) ||
	      L_subroutine_call_opcode (pop))
	    {
	      if (PG_intersecting_predicates_ops (pop, op_rt) &&
		  (L_is_ida_memory_ops (rp->cb, pop, rp->cb, op_rt, 
					SET_NONLOOP_CARRIED(0)) != MEM_IND))
		return 0;
	    }
	  pop = pop->prev_op;
	}

      /* 04/20/03 SER Adding check: cannot merge a speculative and non-
       *              speculative op. */

      if (L_EXTRACT_BIT_VAL (op_lt->flags, L_OPER_SPECULATIVE))
	{
	  if (!L_EXTRACT_BIT_VAL (op_rt->flags, L_OPER_SPECULATIVE))
	    return 0;
	}
      else
	if (L_EXTRACT_BIT_VAL (op_rt->flags, L_OPER_SPECULATIVE))
	  return 0;
    }

  return 1;
}


static L_Oper *
L_unify_up (L_Cb *cb, L_Oper *before, 
	    UniPath *upA, L_Oper *opA, UniPath *upB, L_Oper *opB)
{
  L_Oper *op_pr;
  L_Attr *attr;
  int i;
  L_Cb *cbA = upA->cb, *cbB = upB->cb;

  op_pr = L_copy_operation (opA);

#if 0
  fprintf (stderr, "UNI-UP %s: ops %d %d into %d\n", 
	   L_opcode_name (op_pr->opc), opA->id, opB->id, op_pr->id);
#endif
  
  L_insert_oper_before (cb, before, op_pr);

  /* Unified -- remap src, dest on unification op */
  
  for (i=0; i < L_max_src_operand; i++)
    {
      if (!op_pr->src[i] || !L_is_variable (op_pr->src[i]))
	continue;

      L_rename_operand (upA->map, op_pr->src[i]);
    }

  if ((attr = L_find_attr (op_pr->attr, "tr")))
    for (i = 0; i < attr->max_field; i++)
      {
	int id;
	L_Operand *src;

	if (!(src = attr->field[i]) ||
	    !L_is_variable (src))
	  continue;

	id = L_REG_MAC_INDEX (src);

	if ((id = (int)(long) HashTable_find_or_null (upA->map, id)))
	  {
	    /* Op sources something that has been unified */

	    L_Operand *tmp;

	    tmp = L_copy_operand (src);

	    tmp->value.r = L_UNMAP_REG (id);
	    tmp->type = L_OPERAND_REGISTER;

	    L_insert_compensation_mov_before(cb, op_pr,
					     src, tmp);

	    L_delete_operand (tmp);
	  }	
      }

  /* Rename destination registers and insert compensation
   * moves as necessary.
   */

  for (i=0; i < L_max_dest_operand; i++)
    {
      L_Operand *dstA, *dstB, *dstP;

      dstP = op_pr->dest[i];

      if (!dstP || !L_is_variable (dstP))
	continue;

      /* Check to see if we need a rename or not */

      dstA = opA->dest[i];
      dstB = opB->dest[i];

      if (L_same_operand (dstA, dstB) &&
	  !Set_in (upA->def, L_REG_MAC_INDEX(dstA)) &&
	  !Set_in (upA->use, L_REG_MAC_INDEX(dstA)) &&
	  !Set_in (upB->def, L_REG_MAC_INDEX(dstB)) &&
	  !Set_in (upB->use, L_REG_MAC_INDEX(dstB)))
	continue;

      dstP->value.r = ++L_fn->max_reg_id;
      dstP->type = L_OPERAND_REGISTER;

      L_insert_compensation_mov_before (cbA, opA, dstA, dstP);
	  
      HashTable_insert (upA->map, L_REG_MAC_INDEX (dstA),
			(void *)(long) L_REG_MAC_INDEX (dstP));

      upA->remap = Set_add (upA->remap, L_REG_MAC_INDEX (dstP));
	  
      L_insert_compensation_mov_before (cbB, opB, dstB, dstP);

      upB->remap = Set_add (upB->remap, L_REG_MAC_INDEX (dstP));

      HashTable_insert (upB->map, L_REG_MAC_INDEX (dstB),
			(void *)(long) L_REG_MAC_INDEX (dstP));
    }

  if ((attr = L_find_attr (op_pr->attr, "ret")))
    for (i = 0; i < attr->max_field; i++)
      {
	L_Operand *dest;

	if ((dest = attr->field[i]) &&
	    L_is_variable (dest))
	  {
	    if (!Set_in (upA->def, L_REG_MAC_INDEX(dest)) &&
		!Set_in (upA->use, L_REG_MAC_INDEX(dest)) &&
		!Set_in (upB->def, L_REG_MAC_INDEX(dest)) &&
		!Set_in (upB->use, L_REG_MAC_INDEX(dest)))
	      continue;

	    L_punt ("L_unify_up: Need to rename a return");
	  }
      }

  L_delete_oper (cbA, opA);
  L_delete_oper (cbB, opB);

  return op_pr;
}


static int
Luni_dn_candidate (UniPath *up, L_Oper *op)
{
  int rv;

  rv = !Set_in (up->sync, op->id) &&
    !L_conflicting_dest (up->use, op) &&
    !L_is_predicated (op) &&
    !L_pred_define_opcode (op) &&
    !L_move_opcode (op) &&
    !(L_subroutine_call_opcode (op) &&
      (L_conflicting_dest (up->use, op) ||
       L_conflicting_dest (up->def, op)));

  if (L_general_store_opcode (op) ||
      L_subroutine_call_opcode (op))
    rv = 0;

  return rv;
}


static int
Luni_legal_dn_unification (UniPath *upA, L_Oper *opA,
			   UniPath *upB, L_Oper *opB)
{
  int i;

  for (i=0; i < L_max_dest_operand; i++)
    {
      if (L_is_variable (opA->dest[i]) &&
	  L_is_variable (opB->dest[i]))
	{
	  int lid, rid;
				    
	  lid = L_REG_MAC_INDEX (opA->dest[i]);
	  rid = L_REG_MAC_INDEX (opB->dest[i]);
				    
	  /* can't unify if right-redefined */
	  
	  if (Set_in (upB->use, rid))
	    break;
				  
	  if (lid == rid)
	    continue;
				  
	  if ((lid = (int)(long) 
	       HashTable_find_or_null (upA->map, 
				       lid)) &&
	      (rid = (int)(long) 
	       HashTable_find_or_null (upB->map, 
				       rid)) &&
	      (lid == rid))
	    continue;
				  
	  /* otherwise, can't unify */
	  
	  break;
	}
      else if (!L_same_operand (opA->dest[i],
				opB->dest[i]))
	{
	  /* otherwise, can't unify */
	  break;
	}
    }

  return (i == L_max_dest_operand);
}


static L_Oper *
L_unify_down (L_Cb *mrg_cb, List uplist)
{
  L_Oper *mrg_op;
  L_Operand *mrg_opd;
  UniPath *up;
  int i, unified_dest;

  up = List_first (uplist);

  mrg_op = L_copy_operation (up->mop);

#if 0
  fprintf (stderr, "UNI-DN %s: cb %d ops", 
	   L_opcode_name (mrg_op->opc), mrg_cb->id);
#endif

  for (i=0; i < L_max_src_operand; i++)
    {
      if (!(mrg_opd = mrg_op->src[i]))
	continue;

      List_start (uplist);
      while ((up = (UniPath *) List_next (uplist)))
	if (!L_same_operand (mrg_opd, up->mop->src[i]))
	  break;

      if (up)
	{
	  int ctype, mid;

	  /* Operands differ -- need compensation */

	  if (!L_is_variable (mrg_opd))
	    {
	      if (L_is_ctype_float (mrg_opd))
		ctype = L_CTYPE_DOUBLE;
	      else
		ctype = L_native_machine_ctype;

	      L_delete_operand (mrg_opd);
	      
	      mrg_op->src[i] = mrg_opd =
		L_new_register_operand (++L_fn->max_reg_id,
					ctype,
					L_PTYPE_NULL);
	    }
	  else
	    {
	      mrg_opd->value.r = ++L_fn->max_reg_id;
	      mrg_opd->type = L_OPERAND_REGISTER;	      
	    }

	  mid = L_REG_MAC_INDEX (mrg_opd);

	  L_add_to_cb_IN_set (mrg_cb, mid);

	  List_start (uplist);
	  while ((up = (UniPath *) List_next (uplist)))
	    {
	      L_Oper *op = up->mop;
	      
	      L_insert_compensation_mov_before(up->cb, up->mop,
					       mrg_opd,
					       op->src[i]);

	      if (!L_is_variable (op->src[i]))
		continue;

	      HashTable_insert (up->map, L_REG_MAC_INDEX (op->src[i]),
				(void *)(long) mid);
	    }
	}
    }

  unified_dest = 0;

  for (i=0; i < L_max_dest_operand; i++)
    {
      L_Operand *dest;
      
      if (!(dest = mrg_op->dest[i]) || !L_is_variable (dest))
	continue;

      up = List_first (uplist);

      unified_dest |= L_rename_operand (up->map, dest);
    }

  List_start (uplist);
  while ((up = (UniPath *) List_next (uplist)))
    {
      L_Oper *op = up->mop;

      fprintf (stderr, " %d", op->id);

      if (!unified_dest)
	L_delete_oper (up->cb, op);
    }

  L_insert_oper_before (mrg_cb, mrg_cb->first_op, mrg_op);

  fprintf(stderr, " to op %d\n", mrg_op->id);
  
  return mrg_op;
}


static void
L_activate_remap (UniPath *up, L_Oper *mov)
{
  up->remap = Set_add (up->remap, L_REG_MAC_INDEX (mov->dest[0]));
  return;
}

int
L_unification (L_Func *fn)
{
  int count = 0;

  UniPath_pool = L_create_alloc_pool ("UniPath", sizeof (UniPath), 32);

  /* Split unification
   * ----------------------------------------------------------------------
   * At a control flow split (two-way branch with two single-predecessor
   * targets), pull two identical operations across the branch, unifying
   * them.
   */

  if (Lopti_do_split_unification)
    {
      L_Cb *par, *lt, *rt;
      UniPath *lp = NULL, *rp = NULL;

      /* Up-unification */

      for (par = fn->last_cb; par; par = par->prev_cb)
	{
	  L_Oper *sbr, *nxt_sbr;
	  
	  for (sbr = par->first_op; sbr; sbr = nxt_sbr)
	    {
	      L_Oper *op_lt, *nxt_op_lt, *op_rt;
	      
	      nxt_sbr = sbr->next_op;
	      
	      if (!L_cond_branch(sbr) ||
		  !L_two_way_split (par, sbr, &lt, &rt, &op_lt))
		continue;

	      /* Found a split branch -- a conditional branch
	       * with two single-entry successors (one may be the
	       * branch tail).
	       */

	      lp = Luni_new_unipath (lt, op_lt);
	      rp = Luni_new_unipath (rt, rt->first_op);
	      L_update_unipath (lp, sbr);
	      L_update_unipath (rp, sbr); 

	      for (; op_lt; op_lt = nxt_op_lt)
		{
		  nxt_op_lt = op_lt->next_op;
		  
		  if (L_is_control_oper (op_lt) ||
		      L_subroutine_call_opcode (op_lt))
		    break;

		  if (Luni_up_candidate (lp, op_lt))
		    {
		      /* Look for unificiation with a right-oper */
		 
		      Luni_clean_unipath (rp);

		      for (op_rt = rt->first_op; op_rt; op_rt = op_rt->next_op)
			{
			  if (L_is_control_oper (op_rt) ||
			      L_subroutine_call_opcode (op_rt))
			    {
			      /* Terminate search at a control oper */
			      op_rt = NULL;
			      break;		      
			    }

			  if (L_same_opcode (op_lt, op_rt) &&
			      L_same_compare (op_lt, op_rt) &&
			      Luni_up_candidate (rp, op_rt) &&
			      Luni_legal_up_unification (lp, op_lt, 
							 rp, op_rt, sbr))
			    {
			      /* found valid match */
			      L_unify_up (par, sbr, lp, op_lt, rp, op_rt);
			      STAT_COUNT ("L_unify_up", 1, NULL);
			      count++;
			      break;
			    }
			  else if (!L_is_compensation_mov(rp->map, op_rt))
			    {
			      /* Update right-redef and
			       * right-sync. Compensation moves
			       * inserted in unification don't count
			       * because their destinations did not
			       * previously exist.  */
			
			      L_update_unipath (rp, op_rt);
			    }
			  else
			    {
			      L_activate_remap (rp, op_rt);
			    }
			}

		      /* avoid the update on op_lt if we have unified it */

		      if (op_rt)
			continue;
		    }

		  if (!L_is_compensation_mov(lp->map, op_lt))
		    L_update_unipath (lp, op_lt);
		}
	      
	      lp = Luni_free_unipath (lp);
	      rp = Luni_free_unipath (rp);
	    }
	}
    }

  L_do_flow_analysis (fn, LIVE_VARIABLE);
  
  /* Merge unification
   * ----------------------------------------------------------------------
   * At a control flow merge (block with multiple incoming arcs)
   * pull identical operations down across the merge, unifying them.
   * For now, perform only if the predecessor blocks are single-successor.
   */

 if (Lopti_do_merge_unification)
    {
      L_Cb *mrg_cb;
      DB_spit_func (fn,"PREUD");

      for (mrg_cb = fn->first_cb; mrg_cb; mrg_cb = mrg_cb->next_cb)
	{
	  List uplist;
	  UniPath *upA, *upB;
	  L_Oper *opA, *opB, *nxt_opA, *nxt_opB;

	  if (!(uplist = L_construct_merge_list (mrg_cb)))
	    continue;

	  /* mrg_cb is a valid merge point, and uplist contains merge paths */

	  upA = List_first (uplist);

	  for (opA = upA->op ? upA->op->prev_op : upA->cb->last_op; 
	       opA; opA = nxt_opA)
	    {
	      upA->op = opA;
	      
	      if (L_is_control_oper (opA))
		break;

	      nxt_opA = opA->prev_op;

	      if (Luni_dn_candidate (upA, opA))
		{
		  /* Canditate in leftmost path -- find matches in
                     other paths */

		  List_start (uplist);
		  List_next (uplist); /* skip upA */
		  while ((upB = (UniPath *)List_next (uplist)))
		    {
		      Luni_clean_unipath (upB);

		      for (opB = upB->op ? 
			     upB->op->prev_op : upB->cb->last_op; 
			   opB; opB = nxt_opB)
			{
			  nxt_opB = opB->prev_op;
		    
			  if (L_is_control_oper (opB))
			    {
			      opB = NULL;
			      break;
			    }

			  if (L_same_opcode (opA, opB) &&
			      L_same_compare (opA, opB) &&
			      Luni_dn_candidate (upB, opB) &&
			      Luni_legal_dn_unification (upA, opA, upB, opB))
			    {
			      upB->mop = opB;
			      break;
			    }

			  if (!L_is_compensation_mov(upB->map, opB))
			    L_update_unipath (upB, opB);
			}
		
		      if (!upB->mop)
			break;          /* match not found */
		    }

		  if (!upB)
		    {
		      /* All paths succeeded -- OK to unify current ops
		       * in all lists
		       */

		      upA->mop = opA;
		      L_unify_down (mrg_cb, uplist);
		      STAT_COUNT ("L_unify_down", 1, NULL);
		      count++;
		      continue;
		    }
		}

	      if (!L_is_compensation_mov(upA->map, opA))
		L_update_unipath (upA, opA);
	    }

	  List_reset_free_ptrs (uplist, (void (*)(void *))Luni_free_unipath);
	}
      
      DB_spit_func (fn, "POSTUD");
    }

  L_free_alloc_pool (UniPath_pool);
  UniPath_pool = NULL;

  return count;
}




