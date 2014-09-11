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
 *
 *      File :          r_regspill.c
 *      Description :   Register allocation spill register selection functions
 *      Creation Date : Nov 4, 1993
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 * Revision 1.5  95/01/10  17:49:28  17:49:28  hank (Richard E. Hank)
 * Changes consistent to data structure changes within r_regalloc.[ch]
 * as well as extensions for region-based allocation.
 *
 * Revision 1.1  94/03/16  20:52:51  20:52:51  hank (Richard E. Hank)
 * Initial revision
 *
 *
 *===========================================================================*/
/*===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "r_regalloc.h"


static void
R_renumber_oper_vreg (L_Oper * oper, int fvreg_id, int tvreg_id)
{
  int i;
  L_Attr *attr;
  L_Operand *opd;

  if (!oper)
    return;

  for (i = 0; i < L_max_pred_operand; i++)
    {
      if (!(opd = oper->pred[i]))
	continue;

      if (L_is_reg (opd) && opd->value.r == fvreg_id)
	opd->value.r = tvreg_id;
    }
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(opd = oper->dest[i]))
	continue;

      if (L_is_reg (opd) && opd->value.r == fvreg_id)
	opd->value.r = tvreg_id;
    }
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!(opd = oper->src[i]))
	continue;

      if (L_is_reg (opd) && opd->value.r == fvreg_id)
	opd->value.r = tvreg_id;
    }
  /* EMN: Extended source support */
  if ((oper->opc == Lop_CHECK) && (attr = L_find_attr (oper->attr, "src")))
    {
      for (i = 0; i < attr->max_field; i++)
	{
	  if (!(opd = attr->field[i]))
	    continue;
	  if (L_is_reg (opd) && opd->value.r == fvreg_id)
	    opd->value.r = tvreg_id;
	}
    }
  return;
}


static void
R_cbsplit_spilled_live_range (L_Func * fn, R_Reg * vreg, Stack * vreg_stack)
{
  int i, j, n_instr, n_cb, ctype;
  R_Reg *new_vreg;
  R_Arc *iarcs, *arc;
  double max_weight;
  int *cbuf = R_buf;
  L_Oper **ibuf = (L_Oper **) R_pbuf;
  Set cbset = NULL;
  int use_invariant = R_Invariant_Vreg_Priorities;

#if 0
  fprintf (stdout, "CB Splitting VREG #%d\n", vreg->index);
#endif

  n_instr = Set_2array (vreg->live_range, (int *) cbuf);
  for (i = 0; i < n_instr; i++)
    {
      L_Cb *cb;

      /* find L_Cb * from the oper id's */

      cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, cbuf[i]);
      cbset = Set_add (cbset, cb->id);

      /* find L_Oper * from the oper id's */
      ibuf[i] = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, cbuf[i]);
    }
  n_cb = Set_2array (cbset, cbuf);
  Set_dispose (cbset);

  if (n_cb <= 1)
    {
      /* there is no point to doing any of this */
      vreg->flags |= R_CB_SPLIT;
      return;
    }

  /* save the interferences arcs of the live range */
  iarcs = vreg->interfere;
  vreg->interfere = NULL;
  ctype = R_conv_type_to_Ltype (vreg->type);

  /* cb split the spilled live range */
  for (i = 0; i < n_cb; i++)
    {
      int new_id;
      Set live_range = NULL;

      /* select id for new live range */
      if (i == 0)
	{
	  /* the first cb will remain in the original live range */
	  new_id = vreg->index;
	  vreg->live_range = Set_dispose (vreg->live_range);
	}
      else
	{
	  new_id = R_find_free_vreg ();
	  new_vreg = R_add_register (new_id, ctype, 0.0, 0);
	}

#if 0
      fprintf (stdout, "\t-> Creating new vreg #%d\n", new_id);
      fprintf (stdout, "\t---> Contains instr: ");
#endif
      /* determine the portion of the live range */
      /* belonging to the current cb             */
      for (j = 0; j < n_instr; j++)
	{
	  int id = ibuf[j]->id;
	  L_Cb *cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, id);
	  if (cb->id == cbuf[i])
	    {
	      live_range = Set_add (live_range, id);
	      R_renumber_oper_vreg (ibuf[j], vreg->index, new_id);
#if 0
	      fprintf (stdout, "%d ", ibuf[j]);
#endif
	    }
	}
#if 0
      fprintf (stdout, "\n");
#endif
      new_vreg = VREG (new_id);
      new_vreg->flags |= (R_SPILLED | R_CB_SPLIT);
      new_vreg->ref_instr = new_vreg->live_range = Set_copy (live_range);

      if (i != 0)
	{
	  new_vreg->spill_loc = vreg->spill_loc;
	  new_vreg->pvreg = vreg;
	}
      new_vreg->illegal_reg = Set_copy (vreg->illegal_reg);
      new_vreg->constraints = Set_copy (vreg->constraints);

      new_vreg->caller_benefit = 1;
      new_vreg->callee_benefit = 0;
      new_vreg->rclass = R_CALLER;

      /* if we are to use "invariant" priorities, use register id
       * rather than profile/static based priority */

      max_weight = (!use_invariant) ? vreg->priority : (double) vreg->index;

      /* determine the interferences of the new live range */
      for (arc = iarcs; arc; arc = arc->next)
	{
	  double arc_weight = (!use_invariant) ? arc->lr->priority:
	    (double) arc->lr->index;

	  if (arc_weight > max_weight)
	    max_weight = arc_weight;

	  if (Set_intersect_empty (new_vreg->ref_instr, arc->lr->live_range))
	    {
	      /* if the live ranges do not interfere, and this is the
	       * original live range, remove the arc, otherwise
	       * nothing needs to be done. */
	      if (i == 0)
		R_remove_interference (arc->lr, vreg);
	    }
	  else
	    {
	      /* if they interefere, and this the original live range
	      * we ned only add an arc to new_vreg.  Otherwise, we
	      * remove the old arc and add new arcs. */
	      if (i != 0)
		R_add_interference_arc (new_vreg, arc->lr);

	      R_add_interference_arc (arc->lr, new_vreg);
	    }
	}
      /* assign new priority */
      new_vreg->priority = max_weight + 1.0;

      if (i != 0)
	Push_Top (vreg_stack, new_vreg);

      live_range = Set_dispose (live_range);
    }
  return;
}


static void
R_isplit_spilled_live_range (L_Func * fn, R_Reg * vreg, Stack * vreg_stack)
{
  int k, ctype, free_register, new_id, n;
  R_Arc *iarcs, *arc, *next_arc;
  R_Reg *new_vreg;
  L_Cb *cb;
  L_Oper *oper;
  double max_weight;
  Set ref_instr = NULL, live_range = NULL, reserved_resource = NULL,
    cres = NULL;
  int *nbor = R_buf;
  int use_invariant = R_Invariant_Vreg_Priorities;

#if 0
  fprintf (stdout, "INSTR Splitting VREG #%d\n", vreg->index);
#endif

  /* save the interferences arcs of the live range */
  iarcs = vreg->interfere;
  vreg->interfere = NULL;
  ctype = R_conv_type_to_Ltype (vreg->type);

  ref_instr = Set_copy (vreg->ref_instr);
  Set_2array (ref_instr, nbor);

  /* identify the largest allocatable portion of the live range */

  cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, nbor[0]);
  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      if (!Set_in (ref_instr, oper->id))
	continue;

      live_range = Set_add (live_range, oper->id);

      R_determine_reserved_registers (&reserved_resource, &cres, NULL, vreg);

      /* see if there is a register available to hold the live range */
      free_register = R_find_free_register (vreg, reserved_resource, 1);
      reserved_resource = Set_dispose (reserved_resource);

      if (free_register == -1)
	{
	  if (Set_size (live_range) > 1)
	    live_range = Set_delete (live_range, oper->id);
	  break;
	}
    }
#if 0
  fprintf (stdout, "\t-> Creating new vreg #%d\n", vreg->index);
  fprintf (stdout, "\t---> Contains instr: ");
  Set_print (stdout, "", live_range);
  fprintf (stdout, "\n");
#endif
  vreg->flags = R_SPILLED | R_INSTR_SPLIT;
  vreg->rclass = R_CALLER;
  vreg->live_range = Set_dispose (vreg->live_range);
  vreg->ref_instr = vreg->live_range = Set_copy (live_range);

  /* if we are to use "invariant" priorities, use register id */
  /* rather than profile/static based priority                */

  max_weight = (!use_invariant) ? vreg->priority : (double) vreg->index;

  /* determine the interferences of the new live range */

  for (arc = iarcs; arc; arc = arc->next)
    {
      double arc_weight = (!use_invariant) ? arc->lr->priority : 
	(double) arc->lr->index;

      if (arc_weight > max_weight)
	max_weight = arc_weight;

      if (Set_intersect_empty (vreg->live_range, arc->lr->live_range))
	R_remove_interference (arc->lr, vreg);
      else
	R_add_interference_arc (arc->lr, vreg);
    }
  /* assign new priority */
  vreg->priority = max_weight + 1.0;

  new_id = R_find_free_vreg ();
  new_vreg = R_add_register (new_id, ctype, 0.0, 0);
  new_vreg->flags |= (R_SPILLED | R_INSTR_SPLIT);
  new_vreg->ref_instr = new_vreg->live_range =
    Set_subtract (ref_instr, live_range);

  new_vreg->illegal_reg = Set_copy (vreg->illegal_reg);
  new_vreg->constraints = Set_copy (vreg->constraints);

  /* renumber the instructions virtual registers */
  n = Set_2array (new_vreg->ref_instr, nbor);
  for (k = 0; k < n; k++)
    {
      L_Oper *oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, nbor[k]);
      R_renumber_oper_vreg (oper, vreg->index, new_id);
    }

  new_vreg->spill_loc = vreg->spill_loc;
  new_vreg->pvreg = ((vreg->pvreg == NULL) ? vreg : vreg->pvreg);

  new_vreg->caller_benefit = 1;
  new_vreg->callee_benefit = 0;
  new_vreg->rclass = R_CALLER;
#if 0
  fprintf (stdout, "\t-> Creating new vreg #%d\n", new_id);
  fprintf (stdout, "\t---> Contains instr: ");
  Set_print (stdout, "", new_vreg->ref_instr);
  fprintf (stdout, "\n");
#endif
  /* determine the interferences of the new live range */

 for (arc = iarcs; arc; arc = next_arc)
    {
      next_arc = arc->next;

      if (!Set_intersect_empty (new_vreg->live_range, 
				arc->lr->live_range))
	{
	  R_add_interference_arc (new_vreg, arc->lr);
	  R_add_interference_arc (arc->lr, new_vreg);
	}

      L_free (R_alloc_interference_arc, arc);
    }
  new_vreg->priority = max_weight + 1.0;

  Push_Top (vreg_stack, new_vreg);

  Set_dispose (ref_instr);
  Set_dispose (live_range);
  Set_dispose (reserved_resource);
  return;
}

extern int R_Prevent_MCB_Preload_Spills;

void
R_handle_unallocatable_vreg (L_Func * fn, R_Reg * vreg, Stack * vreg_stack)
{
  R_Arc *arc, *next_arc, *prev_arc;
  double max_weight;
  L_Region_Regmap *regmap;
  int use_invariant = R_Invariant_Vreg_Priorities;

  /* A live range at this point may be at one of these levels: */
  /*    1) Not yet spilled and thus not split                  */
  /*    2) spilled                                             */
  /*    3) split at the cb level                               */
  /*    4) split at the instruction level                      */
  /*  NOTE: Predicates represent a special case, I'm not       */
  /*        allowed to alter the live range of a predicate     */
  /*        since predicate register may not be reused with    */
  /*        cb.  Thus if a spilled predicate register cannot   */
  /*        be allocated, I will punt.                         */
  /* NOTE2: I'm not sure how demotion will tie into all of this */

  if (R_Prevent_MCB_Preload_Spills && (vreg->flags & R_MCB_PRELOAD))
    {
      if (vreg->priority == 1.79e+308)
	L_punt ("Register Allocation: Unable to allocate mcb preload %d\n",
		vreg->index);

      vreg->priority = (use_invariant == 0) ? 1.78e+308 : 
	(double) L_fn->max_reg_id + 1.0;

      return;
    }

  if (vreg->flags & R_SPILLED)
    {
      /* previously spilled and cb split, case 3) */
      if (vreg->flags & R_CB_SPLIT)
	{
	  R_isplit_spilled_live_range (fn, vreg, vreg_stack);
	}
      /* previously spilled and instruction split, case 4) */
      else if (vreg->flags & R_INSTR_SPLIT)
	{
	  if (Set_size (vreg->live_range) == 1)
	    L_punt ("R_handle_unallocatable_vreg: "
		    "INSUFFICIENT REGISTERS!!!!\n",
		    vreg->index);
	  R_isplit_spilled_live_range (fn, vreg, vreg_stack);
	}
      else
	{
	  /* previously spilled, but not yet split, case 2) */
	  R_cbsplit_spilled_live_range (fn, vreg, vreg_stack);
	}
    }
  else
    {
      /* previously unspilled live range, case 1) */
      prev_arc = NULL;

      /* if we are to use "invariant" priorities, use register id */
      /* rather than profile/static based priority            */

      max_weight = (!use_invariant) ? vreg->priority : 
	(double) vreg->index;

      /* modify interference graph to reflect spilling of vreg */
      for (arc = vreg->interfere; arc; arc = next_arc)
	{
	  double arc_weight = (!use_invariant) ? arc->lr->priority :
	    (double) arc->lr->index;

	  if (arc_weight > max_weight)
	    max_weight = arc_weight;

	  next_arc = arc->next;

	  if (Set_intersect_empty (vreg->ref_instr, arc->lr->live_range))
	    {
	      /* these two live ranges no longer interfere */
	      R_remove_interference (arc->lr, vreg);

	      if (prev_arc)
		prev_arc->next = next_arc;
	      else		/* arc = first arc */
		vreg->interfere = next_arc;

	      L_free (R_alloc_interference_arc, arc);
	    }
	  else
	    {
	      prev_arc = ((arc != vreg->interfere) && prev_arc) ? 
		prev_arc->next : arc;
	    }
	}

      vreg->flags |= R_SPILLED;

      regmap = L_find_region_regmap (R_Region, vreg->index);
      if (regmap && (regmap->spill_loc != -1))
	vreg->spill_loc = regmap->spill_loc;
      else
	vreg->spill_loc = R_spill_loc (vreg->type);

      /* new live range for spilled vreg */
      Set_dispose (vreg->live_range);
      vreg->live_range = vreg->ref_instr;

      /* new caller/callee benefits */
      vreg->caller_benefit = 1;
      vreg->callee_benefit = 0;
      vreg->rclass = R_CALLER;

      /* new priority */
      vreg->priority = max_weight + 1.0;
    }
  return;
}

/*===========================================================================
 *
 *      Func :  R_insert_spill_code_before()
 *		R_insert_spill_code_after()
 *      Desc :  Inserts the linked list of of L_Oper's <spill> 
 *		before(after) <oper> in cb <cb> and returns a
 *		count of the number of L_Oper's in the <spill> 
 *		list.
 *      Input:  L_Cb   *cb    - insertion cb
 *		L_Oper *oper  - insertion point
 *		L_Oper *spill - list of oper's to insert
 *      Output: none
 *
 *      Side Effects:
 *
 *===========================================================================*/
int
R_insert_spill_code_before (L_Cb * cb, L_Oper * oper, L_Oper * spill)
{
  int cnt = 0;
  L_Oper *tmp;
  L_Oper *cur = spill;

  while (cur != NULL)
    {
      tmp = cur->next_op;
      L_insert_oper_before (cb, oper, cur);
      cnt++;
      cur = tmp;
    }
  return (cnt);
}

int
R_insert_spill_code_after (L_Cb * cb, L_Oper * oper, L_Oper * spill)
{
  int cnt = 0;
  L_Oper *tmp, *prev = oper, *cur = spill;

  while (cur)
    {
      tmp = cur->next_op;
      L_insert_oper_after (cb, prev, cur);
      cnt++;
      prev = cur;
      cur = tmp;
    }
  return (cnt);
}

static int
R_in_physical_register (int vreg_id, int base, int vreg_type, int vreg_class)
{
  int i;
  R_Physical_Bank *bank = R_bank + vreg_type + vreg_class;

  for (i = base; i < base + bank->reg_size; i++)
    if (R_register_contents[i] != vreg_id)
      return (0);

  return (1);
}

static void
R_put_in_physical_register (int vreg_id, int base, int vreg_type,
			    int vreg_class)
{
  int i;
  R_Physical_Bank *bank = R_bank + vreg_type + vreg_class;

  for (i = base; i < base + bank->reg_size; i++)
    R_register_contents[i] = vreg_id;
}

static int
R_same_register (R_Reg * vreg1, R_Reg * vreg2)
{
  if (R_bank[vreg1->type + vreg1->rclass].reg_size >
      R_bank[vreg2->type + vreg2->rclass].reg_size)
    {
      if (vreg1->base_index ==
	  (vreg2->base_index & R_bank[vreg1->type + vreg1->rclass].mask))
	return (1);
    }
  else
    {
      if ((vreg1->base_index & R_bank[vreg2->type + vreg2->rclass].mask) ==
	  vreg2->base_index)
	return (1);
    }
  return (0);
}

static int
R_register_spill_is_required (L_Cb * cb, L_Oper * oper, R_Reg * vreg)
{
  int index;
  L_Oper *cur_oper;

  if (!R_Register_Allocation)
    return (1);

  /* If the virtual register's live range has been split */
  /* use the vreg->index of its parent                       */

  index = ((vreg->flags & (R_CB_SPLIT | R_INSTR_SPLIT)) && vreg->pvreg) ? 
    vreg->pvreg->index : vreg->index;

  if (L_in_cb_OUT_set_reg (cb, index))
    return (1);

  for (cur_oper = oper->next_op; cur_oper; cur_oper = cur_oper->next_op)
    {
      int i;
      L_Operand *opd;
      L_Attr *attr;

      if (cur_oper->id > R_n_oper)
	continue;

      if (!L_in_oper_IN_set_reg (cur_oper, index))
	break;

      if (cur_oper->opc == Lop_JSR || cur_oper->opc == Lop_JSR_FS)
	return (1);

      for (i = 0; i < L_max_dest_operand; i++)
	if ((opd = cur_oper->dest[i]) && (L_is_reg (opd)))
	  {
	    /* If the virtual registers are the same, then no spill */
	    /* is required, since the variable is being redefined   */
	    /* Otherwise, I don't need to spill it until a defined  */
	    /* virtual register uses the same physical register     */

	    if (opd->value.r == vreg->index)
	      return (0);
	    else if (R_same_register (VREG (opd->value.r), vreg))
	      return (1);
	  }
      for (i = 0; i < L_max_src_operand; i++)
	if ((opd = cur_oper->src[i]) && (L_is_reg (opd)))
	  {
	    if (R_same_register (VREG (opd->value.r), vreg) &&
		(opd->value.r != vreg->index))
	      return (1);
	  }
      /* EMN: Extended source support */
      if ((oper->opc == Lop_CHECK) && 
	  (attr = L_find_attr (oper->attr, "src")))
	{
	  for (i = 0; i < attr->max_field; i++)
	    {
	      if (!(opd = attr->field[i]))
		continue;
	      if ((opd = cur_oper->src[i]) && (L_is_reg (opd)) &&
		  R_same_register (VREG (opd->value.r), vreg) &&
		  (opd->value.r != vreg->index))
		return (1);
	    }
	}
    }
  return (0);
}

static int
R_max_bank_offset (void)
{
  int i, offset, max = 0;
  R_Physical_Bank *bank;

  for (i = 0; i < R_MAX_BANK; i++)
    {
      bank = R_bank + i;
      if (!bank->defined)
	continue;

      offset = bank->base_index + bank->num_reg * bank->reg_size;
      if (offset > max)
	max = offset;
    }
  return (max);
}

/*===========================================================================
 *
 *      Func :  R_insert_spill_fill_code()
 *      Desc :  Inserts required load/store instructions to handle spilled
 *		virtual registers. Load instructions are not inserted if the
 *		desired value is already guaranteed to be in a spill register 
 *      Input:  L_Func *fn	- current function 
 *      Output: none
 *
 *      Side Effects:  if DEBUG_PRINT_SPILL_PRECENTAGES is defined then
 *		       <total_instruction_weight> and <total_spill_weight>
 *		       are also calculated. 
 *
 *===========================================================================*/
void
R_insert_spill_fill_code (L_Region * region)
{
  int i, k;
  L_Oper *oper, *save_oper;
  L_Cb *cur_cb;
  int n_instr, max_offset, hyperblock;
  L_Oper *spill_instr;
  L_Region_Member *mbr;
  L_Attr *attr;

  double instruction_weight = 0.0;
  double spill_weight = 0.0;

  max_offset = R_max_bank_offset ();
  R_register_contents = (int *) malloc (sizeof (int) * (max_offset + 1));

  for (mbr = region->member_cbs; mbr != NULL; mbr = mbr->next_member)
    {
      cur_cb = mbr->cb;

      /* insert spill code -> load instructions in a forward pass over
         the cb */

      hyperblock = L_EXTRACT_BIT_VAL (cur_cb->flags, L_CB_HYPERBLOCK);

      for (k = 0; k < max_offset + 1; k++)
	R_register_contents[k] = -1;

      for (oper = cur_cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  R_Reg *vreg;
	  L_Oper *next_oper;
	  int vrid;

	  instruction_weight += oper->weight;

	  /* guard predicate */

	  {
	    L_Operand *pred = oper->pred[0];
	    if (pred && L_is_reg (pred))
	      {
		vrid = pred->value.r;
		vreg = VREG (vrid);
		if ((vreg->flags & R_SPILLED) &&
		    (hyperblock ||
		     !R_in_physical_register(vrid, vreg->base_index, 
					     vreg->type, vreg->rclass)))
		  {
		    spill_instr = 
		      O_fill_reg (vreg->phys_reg,
				  R_conv_rclass_to_Lclass(vreg->rclass),
				  pred, vreg->spill_loc, NULL,
				  R_SPILL_CODE);
			
		    n_instr = R_insert_spill_code_before (cur_cb, oper,
							  spill_instr);
			
		    R_register_contents[vreg->base_index] = vrid;
			
		    spill_weight += (oper->weight * n_instr);
		  }

		/* save virtual predicate of jsr */
		if (L_general_subroutine_call_opcode (oper))
		  {
		    L_Attr *new_attr = L_new_attr ("vpred", 1);
		    L_set_int_attr_field (new_attr, 0, vrid);
		    oper->attr = L_concat_attr (oper->attr, new_attr);
		  }
		
		pred->value.r = vreg->phys_reg;
	      }
	  }

	  /* src operands */

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      L_Operand *src = oper->src[i];
	      if (!src || !L_is_reg (src))
		continue;

	      vrid = src->value.r;
	      vreg = VREG (vrid);
	      if ((vreg->flags & R_SPILLED) &&
		  (hyperblock ||
		   !R_in_physical_register (vrid, vreg->base_index, 
					    vreg->type, vreg->rclass)))
		{
		  spill_instr = 
		    O_fill_reg (vreg->phys_reg,
				R_conv_rclass_to_Lclass (vreg->rclass),
				src, vreg->spill_loc, oper->pred,
				R_SPILL_CODE);
			  
		  n_instr = R_insert_spill_code_before (cur_cb, oper,
							spill_instr);

		  R_register_contents[vreg->base_index] = vrid;

		  spill_weight += (oper->weight * n_instr);
		  ld_wgt += (oper->weight * n_instr);
		  ld_cnt += 1;
		}

	      if (vreg->rclass == R_MACRO_CALLER ||
		  vreg->rclass == R_MACRO_CALLEE)
		L_assign_type_general_macro (src);

	      src->value.r = vreg->phys_reg;
	    }

	  /* EMN: Extended source support */

	  if (oper->opc == Lop_CHECK && 
	      (attr = L_find_attr (oper->attr, "src")))
	    {
	      for (i = 0; i < attr->max_field; i++)
		{
		  L_Operand *src = attr->field[i];
		  if (!src || !L_is_reg (src))
		    continue;

		  vrid = src->value.r;
		  vreg = VREG (vrid);
		  if ((vreg->flags & R_SPILLED) &&
		      (hyperblock ||
		       !R_in_physical_register (vrid, vreg->base_index, 
						vreg->type, vreg->rclass)))
		    {
		      spill_instr =
			(L_Oper *) O_fill_reg (vreg->phys_reg,
					       R_conv_rclass_to_Lclass
					       (vreg->rclass), src,
					       vreg->spill_loc,
					       oper->pred,
					       R_SPILL_CODE);

		      n_instr = 
			R_insert_spill_code_before (cur_cb, oper,
						    spill_instr);
			      
		      R_register_contents[vreg->base_index] = vrid;

		      spill_weight += (oper->weight * n_instr);
		      ld_wgt += (oper->weight * n_instr);
		      ld_cnt += 1;
		    }

		  if (vreg->rclass == R_MACRO_CALLER ||
		      vreg->rclass == R_MACRO_CALLEE)
		    L_assign_type_general_macro (src);

		  src->value.r = vreg->phys_reg;
		}
	    }

	  /* dest operands */

	  save_oper = NULL;
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      L_Operand *dest = oper->dest[i];
	      if (!dest || !L_is_reg (dest))
		continue;

	      vrid = dest->value.r;
	      vreg = VREG (vrid);
	      R_put_in_physical_register (vrid, vreg->base_index,
					  vreg->type, vreg->rclass);
	      if ((vreg->flags & R_SPILLED) &&
		  (hyperblock ||
		   R_register_spill_is_required (cur_cb, oper, vreg)))
		{
		  spill_instr =
		    O_spill_reg (vreg->phys_reg,
				 R_conv_rclass_to_Lclass (vreg->rclass), 
				 dest, vreg->spill_loc, oper->pred,
				 R_SPILL_CODE);
		  next_oper = spill_instr;
		  while (next_oper->next_op)
		    next_oper = next_oper->next_op;

		  n_instr = R_insert_spill_code_after (cur_cb, oper, 
						       spill_instr);

		  if (!save_oper)
		    save_oper = next_oper;

		  spill_weight += n_instr * oper->weight;
		  st_wgt += n_instr * oper->weight;
		  st_cnt += n_instr;
		}

	      if (vreg->rclass == R_MACRO_CALLER || 
		  vreg->rclass == R_MACRO_CALLEE)
		L_assign_type_general_macro (dest);

	      dest->value.r = vreg->phys_reg;
	    }

	  if (save_oper)
	    oper = save_oper;
	  /* nullify the current register contents for subroutine calls */
	  if (L_general_subroutine_call_opcode (oper))
	    for (k = 0; k < max_offset + 1; k++)
	      R_register_contents[k] = -1;
	}
    }

  total_instruction_weight += instruction_weight;
  total_spill_weight += spill_weight;

  free (R_register_contents);
  return;
}

int
R_jsr_defines_vreg (L_Oper * jsr, int vreg_id)
{
  int i;
  L_Operand *dst;

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if ((dst = jsr->dest[i]) == NULL)
	continue;

      if (L_is_reg (dst) && (dst->value.r == vreg_id))
	return (1);
    }
  return (0);
}

#if 1
/* TEMPO */
#include <machine/m_tahoe.h>

static L_Oper *
O_pred_save_operation (L_Operand *pred, L_Operand *dest)
{
  L_Oper *mov_oper;

  mov_oper = L_create_new_op (Lop_MOV);
  mov_oper->proc_opc = TAHOEop_MOV_FRPR;
  mov_oper->src[0] = L_new_macro_operand (TAHOE_PRED_BLK_REG, L_CTYPE_LLONG,
					  L_PTYPE_NULL);
  mov_oper->dest[0] = dest;
  mov_oper->pred[0] = pred;
  return (mov_oper);
}

static L_Oper *
O_pred_restore_operation (L_Operand *pred, L_Operand *src, ITintmax mask)
{
  L_Oper *mov_oper;

  mov_oper = L_create_new_op (Lop_MOV);
  mov_oper->proc_opc = TAHOEop_MOV_TOPR;
  mov_oper->src[0] = src;
  mov_oper->src[1] = L_new_gen_int_operand (mask);
  mov_oper->dest[0] = L_new_macro_operand (TAHOE_PRED_BLK_REG, L_CTYPE_LLONG,
					   L_PTYPE_NULL);
  mov_oper->pred[0] = pred;
  return (mov_oper);
}

#endif

/*! \brief Determines if a jsr is the last one in a CB.
 *
 * \param jsr
 *  the jsr oper to inspect
 *
 * \return
 *  If \a jsr is the last jsr in its CB, returns 1.  Otherwise, returns 0.
 */
static int
R_last_jsr_in_cb (L_Oper *jsr)
{
  L_Oper *t;

  for (t = jsr->next_op; t; t = t->next_op)
    if (L_subroutine_call_opcode (t))
      return (0);

  return (1);
}

/*! \brief Determines if a register is used between two jsrs.
 *
 * \param vreg_id
 *  the register's id (L_Operand->value.r, R_Reg->phys_reg)
 * \param jsr1
 *  the first jsr
 * \param jsr2
 *  the second jsr
 *
 * \return
 *  If any opers between \a jsr1 and \a jsr2 use register \a vreg_id, returns
 *  1.  Otherwise, returns 0.
 *
 * This function is used to try to conservatively spill and fill registers
 * between jsrs.  If a register is not used between two jsrs, there's no
 * reason to fill it after the first and spill it before the second.  This
 * is particularly important in C++, where almost every operation is a
 * jsr (See bug 323).
 *
 * Added 11/29/04 REK
 */
static int
R_used_between_jsrs (int vreg_id, L_Oper *jsr1, L_Oper *jsr2)
{
  L_Oper *cur_oper;
  L_Operand *cur_operand;
  int i;

  for (cur_oper = jsr1->next_op; cur_oper != jsr2;
       cur_oper = cur_oper->next_op)
    {
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if ((cur_operand = cur_oper->src[i]) == NULL)
	    continue;

	  if (L_is_reg (cur_operand) && (cur_operand->value.r == vreg_id))
	    return (1);
	}

      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if ((cur_operand = cur_oper->dest[i]) == NULL)
	    continue;

	  if (L_is_reg (cur_operand) && (cur_operand->value.r == vreg_id))
	    return (1);
	}
    }

  return (0);
}

/*===========================================================================
 *
 *      Func :  R_insert_jsr_spill_fill_code()
 *      Desc :  Inserts required load/store instructions to handle spilling
 *		caller saved registers around subroutine calls.
 *      Input:  L_Func *fn      - current function
 *      Output: none
 *
 *      Side Effects:  if DEBUG_PRINT_SPILL_PRECENTAGES is defined then
 *                     <total_caller_weight> is also calculated.
 *
 *===========================================================================*/
int
R_insert_jsr_spill_fill_code (L_Func * fn, int *int_jsr_swap_space,
			      int *fp_jsr_swap_space, int *pred_jsr_swap_space)
{
  int i, ltype, n_jsr;
  L_Cb **jsr_cb_array = NULL;
  int pred_swap_stack = 0, int_swap_stack = 0, fp_swap_stack = 0;
  Set *jsr_save_reg = NULL;
  L_Attr *attr;
  L_Oper *spill_instr, *oper, *next_jsr;
  R_Reg *vreg;
  L_Operand *operand = L_new_register_operand (0, L_CTYPE_VOID, L_PTYPE_NULL);

  double caller_weight = 0.0;

  L_Cb *cur_jsr_cb;

  if ((n_jsr = List_size (R_jsr_list)))
    {
      jsr_save_reg = CALLOC (Set, n_jsr);
      jsr_cb_array = (L_Cb **) malloc (sizeof (L_Cb) * n_jsr);
    }

  /* Find the cb for each jsr and insert it in jsr_cb_list. */
  List_start (R_jsr_list);
  i = 0;
  while ((oper = (L_Oper *)List_next (R_jsr_list)))
    {
      if (!(cur_jsr_cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl,
						  oper->id)))
	L_punt ("r_regspill.c:R_insert_jsr_spill_fill_code:%d Could not find "
		"CB for oper %d", __LINE__, oper->id);
      
      jsr_cb_array[i] = cur_jsr_cb;
      i++;
    }

  /* find all the caller-saved registers that need to be saved */
  /* 12/10/04 REK Note that no vregs with R_SPILLED set will be added
   *              to the list to save for a jsr.  It should be safe to
   *              use this field for our purposes as long as we clear it
   *              before the vreg escapes this function. */
  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
    {
      if (vreg->flags & R_SPILLED)
	continue;

      if (vreg->flags & R_PREALLOCATED_MACRO ||
	  vreg->flags & R_PREALLOCATED_FLYBY)
	continue;

      if (!(vreg->rclass == R_CALLER || vreg->rclass == R_MACRO_CALLER))
	continue;

      List_start (R_jsr_list);
      i = 0;
      while ((oper = (L_Oper *)List_next (R_jsr_list)))
	{
	  if (oper->opc == Lop_JSR_ND)
	    continue;

	  if (Set_in (vreg->live_range, oper->id) &&
	      L_in_oper_IN_set_reg (oper, vreg->index) &&
	      L_in_oper_OUT_set_reg (jsr_cb_array[i], oper, vreg->index,
				     BOTH_PATHS) &&
	      !R_jsr_defines_vreg (oper, vreg->phys_reg))
	    jsr_save_reg[i] = Set_add (jsr_save_reg[i], vreg->index);

	  i++;
	}
    }

  /* determine largest amount of space required to save these registers */
  /* and insert the required load and store instructions                */
  List_start (R_jsr_list);
  next_jsr = (L_Oper *)List_next (R_jsr_list);
  i = 0;
  while ((oper = next_jsr))
    {
      int n_instr, j, *reg_array, n_reg, pred_loc = 0;
      Set predicates = NULL, not_spilled_for_next_jsr = NULL;

      if ((next_jsr = (L_Oper *)List_next (R_jsr_list)))
	not_spilled_for_next_jsr = Set_subtract (jsr_save_reg[i],
						 jsr_save_reg[i + 1]);

      reg_array = MALLOC (int, Set_size (jsr_save_reg[i]));
      n_reg = Set_2array (jsr_save_reg[i], reg_array);

      pred_swap_stack = pred_spill_stack;
      int_swap_stack = int_spill_stack;
      fp_swap_stack = fp_spill_stack;

      if ((attr = L_find_attr (oper->attr, "vpred")))
	{
	  pred_loc = int_swap_stack;
	  if (M_arch != M_TAHOE)
	    {
	      int_swap_stack += 4;
	    }
	  else
	    {
	      int_swap_stack += 8;
	      pred_swap_stack += 1;
	    }
	}

      for (j = 0; j < n_reg; j++)
	{
	  int spill_offset;

	  vreg = VREG (reg_array[j]);

	  if ((M_arch == M_TAHOE) && (vreg->type == R_PREDICATE))
	    {
	      predicates = Set_add (predicates, reg_array[j]);
	      continue;
	    }

	  /* ensure alignment of spills */

	  if ((vreg->type == R_DOUBLE) || (vreg->type == R_FLOAT))
	    {
	      if (M_arch == M_TAHOE)
		{
		  if (fp_swap_stack % 16)
		    fp_swap_stack += (16 - (fp_swap_stack % 16));
		}
	      else
		{
		  int swap_stack = int_swap_stack + fp_swap_stack;
		  if (swap_stack % 8)
		    fp_swap_stack += (8 - (swap_stack % 8));
		}
	    }

	  /* save location of jsr predicate and don't insert the load */
	  if (attr && ((int) attr->field[0]->value.i == vreg->index))
	    {
	      /*
	         pred_loc = swap_stack;
	       */
	      ;
	    }
	  else
	    {
	      /* insert load after oper, from swap_loc */
	      /* to vreg->base_index                           */
	      ltype = R_conv_type_to_Ltype (vreg->type);

	      L_assign_ctype (operand, ltype);

	      /* If we're trying to minimize spills and fills, a register
	       * needs a spill offset that is valid across multiple jsrs.
	       * We compute an unique offset for each vreg.  Note that this
	       * may use more swap space than the old method. */
	      if (R_Minimize_Spill_Fill)
		{
		  if (vreg->spill_loc == -1)
		    vreg->spill_loc = R_spill_loc (vreg->type);

		  spill_offset = vreg->spill_loc;
		}
	      else
		{
		  /* Default to the old behavior.  Live registers are
		   * spilled starting at int_spill_stack, but do not have
		   * necessarily have constant spill offsets across jsrs.
		   * This potentially uses less spill space, but means
		   * a spill is not valid across more than one jsr. */
		  if (M_arch == M_TAHOE)
		    {
		      if ((vreg->type == R_DOUBLE) || (vreg->type == R_FLOAT))
			spill_offset = fp_swap_stack;
		      else if ((vreg->type == R_PREDICATE))
			spill_offset = pred_swap_stack;
		      else
			spill_offset = int_swap_stack;		    
		    }
		  else
		    {
		      spill_offset = int_swap_stack + fp_swap_stack;
		    }
		}

	      /* Spill the register
	       *
	       * 12/10/04 REK I use the R_SPILLED flag here to determine if
	       *              a vreg is still spilled from a previous jsr.
	       *              I believe this to safe (search backward for
	       *              '12/10/04 REK' to find my previous note) since
	       *              the flag will be cleared before the vreg leaves
	       *              this function. */
	      if ((R_Minimize_Spill_Fill == 0) || !(vreg->flags & R_SPILLED))
		{
		  vreg->flags |= R_SPILLED;

		  if (vreg->spill_loc == -1)
		    vreg->spill_loc = R_spill_loc (vreg->type);
		  
		  /* If we're minimizing spills and fills, we do not
		   * apply the jsr's predicate to the spill code.  This
		   * gives us the opportunity to elimininate the
		   * corresponding fill and maintain correctness if
		   * the next jsr is not predicated. */
		  spill_instr = \
		    O_spill_reg (vreg->phys_reg,
				 R_conv_rclass_to_Lclass(vreg->rclass),
				 operand, spill_offset,
				 R_Minimize_Spill_Fill == 0 ? oper->pred : \
				                              NULL,
				 R_JSR_SAVE_CODE);
		  
		  n_instr = R_insert_spill_code_before (jsr_cb_array[i], oper,
							spill_instr);
		  
		  caller_weight += n_instr * oper->weight;
		  st_wgt += n_instr * oper->weight;
		  st_cnt += n_instr;
		}

	      /* Fill the register
	       *
	       * If we're minimizing spills and fills, we have to fill
	       * this register if
	       * -The current jsr is the last jsr in the cb, or
	       * -The register is used before the next jsr, or
	       * -There is a branch before the next jsr, or
	       * -The register does not get spilled for the next jsr.
	       *
	       * If none of these apply, we eliminate this fill and the
	       * corresponding spill. */
	      if ((R_Minimize_Spill_Fill == 0) || 
		  R_last_jsr_in_cb (oper) ||
		  R_used_between_jsrs (vreg->phys_reg, oper, next_jsr) ||
		  !(L_no_br_between (oper, next_jsr)) ||
		  Set_in (not_spilled_for_next_jsr, vreg->index))
		{
		  vreg->flags &= ~R_SPILLED;

		  if (vreg->spill_loc == -1)
		    L_punt ("r_regspill.c:R_insert_jsr_spill_fill_code:%d "
			    "Attempting to fill\nregister with undefined "
			    "location", __LINE__); 
		  
		  /* If we're minimizing spills and fills, we do not
		   * apply the jsr's predicate to the fill code.  The
		   * spill may have happened several jsrs ago for a
		   * non-predicated jsr. */
		  spill_instr = \
		    O_fill_reg (vreg->phys_reg,
				R_conv_rclass_to_Lclass (vreg->rclass),
				operand, spill_offset,
				R_Minimize_Spill_Fill == 0 ? oper->pred : NULL,
				R_JSR_SAVE_CODE);
		  
		  n_instr = R_insert_spill_code_after (jsr_cb_array[i], oper,
						       spill_instr);
		  
		  caller_weight += n_instr * oper->weight;
		  ld_wgt += n_instr * oper->weight;
		  ld_cnt += n_instr;
		}
	    }

	  /* 12/10/04 REK Adjusting the swap stack if only necessary if we're
	   *              using the old scheme where registers are spilled
	   *              at a new location for each jsr.  When minimizing,
	   *              each register gets a unique location from
	   *              R_spill_loc(). */
	  if (R_Minimize_Spill_Fill == 0)
	    {
	      if (M_arch == M_TAHOE)
		{
		  switch (vreg->type)
		    {
		    case R_INT:
		    case R_BTR:
		      int_swap_stack += 8;
		      break;
		    case R_FLOAT:
		    case R_DOUBLE:
		      fp_swap_stack += 16;
		      break;
		    case R_PREDICATE:
		      pred_swap_stack += 1;
		      break;
		    case R_QUAD:
		      fprintf (stderr, "Unable to spill quad or predicate.\n");
		    default:
		      L_punt ("R_register_allocation(jsr): invalid register "
			      "type");
		    }
		}
	      else
		{
		  switch (vreg->type)
		    {
		    case R_INT:
		    case R_PREDICATE:
		    case R_FLOAT:
		    case R_BTR:
		    case R_POINTER:
		      int_swap_stack += 4;
		      break;
		    case R_DOUBLE:
		      int_swap_stack += 8;
		      break;
		    case R_QUAD:
		    default:
		      L_punt ("R_register_allocation(jsr): invalid register "
			      "type");
		    }
		}
	    }
	}
      /* if the jsr was predicated, 
	 insert an unpredicated load of jsr predicate */
      
      /* REH 5/11/95 - Resulted in unpredicated spills/loads of the 
         jsr's predicate even when it was callee-saved
         if ( attr  && (n_reg > 0))  {
       */

      if (attr && Set_in (jsr_save_reg[i], (int) attr->field[0]->value.i))
	{
	  int stack_loc;

	  vreg = VREG ((int) attr->field[0]->value.i);

	  if (vreg->flags & R_SPILLED)
	    stack_loc = vreg->spill_loc;
	  else
	    stack_loc = pred_loc;
	  /* Let the code generator worry about proper predication */
	  /* of the required spill instructions                    */
	  L_assign_ctype (operand, L_CTYPE_PREDICATE);
	  spill_instr = O_spill_reg (vreg->phys_reg, L_OPERAND_REGISTER,
				     operand, stack_loc,
				     NULL, R_JSR_SAVE_CODE);

	  n_instr = R_insert_spill_code_before (jsr_cb_array[i], oper,
						spill_instr);	  

	  caller_weight += n_instr * oper->weight;
	  st_wgt += n_instr * oper->weight;
	  st_cnt += n_instr;

	  L_assign_ctype (operand, L_CTYPE_PREDICATE);
	  spill_instr = O_fill_reg (vreg->phys_reg, L_OPERAND_REGISTER,
				    operand, stack_loc,
				    NULL, R_JSR_SAVE_CODE);

	  n_instr = R_insert_spill_code_after (jsr_cb_array[i], oper,
					       spill_instr);

	  caller_weight += n_instr * oper->weight;
	  ld_wgt += n_instr * oper->weight;
	  ld_cnt += n_instr;
	}

      if ((M_arch == M_TAHOE) && Set_size(predicates))
	{
	  int reg, k, preg;
	  ITintmax mask = 0;
	  Set avail_callee = NULL;
	  R_Physical_Bank *bank;

	  bank = &R_bank[R_INT + R_CALLEE];
	  avail_callee = bank ? Set_copy(*(bank->used_reg)) : NULL;

	  for (vreg = R_vreg; vreg != NULL; vreg = vreg->nextReg)
	    {
	      if (vreg->flags & R_PREALLOCATED_MACRO ||
		  vreg->flags & R_PREALLOCATED_FLYBY)
		continue;
	      
	      if (vreg->rclass != R_CALLEE)
		continue;

	      preg = vreg->phys_reg;

	      if (Set_in (vreg->live_range, oper->id) ||
		  L_in_oper_IN_set_reg (oper, vreg->index) ||
		  L_in_oper_OUT_set_reg (jsr_cb_array[i], oper, vreg->index,
					 BOTH_PATHS) ||
		  R_jsr_defines_vreg (oper, preg))
		avail_callee = Set_delete(avail_callee, preg);
	    }

	  if (Set_size(avail_callee))
	    {
	      int *avail_array = MALLOC (int, Set_size (avail_callee));
	      n_reg = Set_2array (avail_callee, avail_array);

	      reg = avail_array[0];
	      L_warn ("R_insert_jsr_spill_fill_code: scavenging r%d",
		      reg);

	      free (avail_array);
	    }
	  else
	    {
	      L_warn ("R_insert_jsr_spill_fill_code: absconding with r4");
	      reg = 4 + TAHOE_INT_REG_BASE;
	    }

	  Set_dispose(avail_callee);

	  n_reg = Set_2array (predicates, reg_array);
	  Set_dispose(predicates);
	  predicates = NULL;

	  for (k = 0; k < n_reg; k++)
	    {
	      int r;
	      vreg = VREG (reg_array[k]);
	      r = vreg->phys_reg - TAHOE_PRED_REG_BASE;
	      if (r > 15)
		L_punt("Trying to save a callee-save predicate");

	      mask |= 1 << r;
	    }

	  spill_instr = 
	    O_pred_save_operation (L_copy_operand(oper->pred[0]),
				   L_new_register_operand(reg,
							  L_CTYPE_LLONG,
							  L_PTYPE_NULL));
	  n_instr = R_insert_spill_code_before (jsr_cb_array[i], oper,
						spill_instr);

	  caller_weight += n_instr * oper->weight;

	  spill_instr = 
	    O_pred_restore_operation (L_copy_operand(oper->pred[0]),
				      L_new_register_operand(reg,
							     L_CTYPE_LLONG,
							     L_PTYPE_NULL),
				      mask);

	  n_instr = R_insert_spill_code_after (jsr_cb_array[i], oper,
					       spill_instr);
	  caller_weight += n_instr * oper->weight;

	}

      if (attr)
	oper->attr = L_delete_attr (oper->attr, attr);

      if ((pred_swap_stack - pred_spill_stack) > *pred_jsr_swap_space)
	*pred_jsr_swap_space = pred_swap_stack - pred_spill_stack;

      if ((int_swap_stack - int_spill_stack) > *int_jsr_swap_space)
	*int_jsr_swap_space = int_swap_stack - int_spill_stack;

      if ((fp_swap_stack - fp_spill_stack) > *fp_jsr_swap_space)
	*fp_jsr_swap_space = fp_swap_stack - fp_spill_stack;

      if (reg_array)
	free (reg_array);
      Set_dispose (jsr_save_reg[i]);
      if (not_spilled_for_next_jsr)
	Set_dispose (not_spilled_for_next_jsr);

      i++;
    }

  if (jsr_cb_array)
    free (jsr_cb_array);
  if (jsr_save_reg)
    free (jsr_save_reg);

  L_delete_operand (operand);

  total_caller_weight += caller_weight;

  return (*int_jsr_swap_space + *fp_jsr_swap_space + *pred_jsr_swap_space);
}
