/*****************************************************************************\
 *
 *		      Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004 The University of Illinois at Urbana-Champaign.
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
 *      File :          r_region_ext.c
 *      Description :   Region-based extensions to global register allocation
 *      Creation Date : September 1994
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 * Revision 1.2  95/01/10  17:46:21  17:46:21  hank (Richard E. Hank)
 * Contains all region-based extensions to the global register allocator.
 *
 * Revision 1.1  95/01/10  17:45:16  17:45:16  hank (Richard E. Hank)
 * Initial revision
 *
 *
 *===========================================================================*/
/*===========================================================================*/
/* 12/03/02 REK Taking out the lhppa requirement for distribution. */
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "r_regalloc.h"

#if 0
#ifndef OPENIMPACT_DISTRIBUTION
#include <Lcode/lhppa_phase1.h>
#endif
#endif

extern L_Oper *O_jump_oper (int opc, L_Cb * target_cb);

#undef  DEBUG_REGION_CONSTRAINTS
#undef  DEBUG_REGION_RECONCILE

void
R_init_flow_hash_tbl (L_Func * fn)
{
  L_Flow *flow;
  L_Cb *cb;
  L_Oper *oper;
  HashTable hashTbl = flowHashTbl;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      flow = cb->dest_flow;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_general_branch_opcode (oper))
	    continue;
	  HashTable_insert (hashTbl, oper->id, flow);
	  flow = flow->next_flow;
	}
    }
}

void
R_determine_region_flybys (L_Region * region)
{
  Set in;
  int i, n_lv, *live_in = R_buf, vreg_id;
  L_Region_Boundary *bndry;
  R_Reg *vreg;
  L_Region_Regmap *regmap;

  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      if (bndry->bcb != NULL)
	{
	  in = bndry->live_in;

	  n_lv = Set_2array (in, live_in);
	  for (i = 0; i < n_lv; i++)
	    {
	      if (!L_IS_MAPPED_REG (live_in[i]))
		continue;

	      vreg_id = L_UNMAP_REG (live_in[i]);
	      regmap = L_find_region_regmap (R_Region, vreg_id);

	      vreg = VREG (vreg_id);
	      if (vreg == NULL)
		{
		  /* We have an unrefereced flyby, we must create a vreg */
		  /* structure for it.  :)                               */

		  vreg = R_add_register (vreg_id, regmap->ctype, 0.0, 0);
		  vreg->flags |= R_PREALLOCATED_FLYBY;
		}
	      if (regmap->spill_loc != -1)
		{
		  vreg->spill_loc = regmap->spill_loc;
		}
	    }
	}
    }
}

int
R_find_base_from_phys_reg (int phys_reg, int type, int rclass)
{
  int j, *map, size;
  int base;
  R_Physical_Bank *bank;

  base = -1;
  bank = R_bank + type + rclass;
  if (bank->defined)
    {
      map = R_map[type + rclass];
      size = bank->reg_size;
      for (j = 0; j < bank->num_reg; j++)
	{
	  if (map[j * size] == phys_reg)
	    {
	      /*
	         fprintf(stderr,"Unmapping physical register %d "
		                "to base index %d.\n",
	                        phys_reg,bank->base_index + j*bank->reg_size);
	       */
	      base = bank->base_index + j * bank->reg_size;
	      break;
	    }
	}
    }
  return (base);
}

void
R_unmap_physical_registers (L_Region * region)
{
  int i, type, base;
  int n_vreg, *vreg_array = R_buf;
  INT_Symbol *sym;
  L_Region_Regmap *regmap;
  Set occupied_base;

  if (region->regmap == NULL)
    return;

  if ((sym = region->regmap->head_symbol) != NULL)
    {
      for (sym = region->regmap->head_symbol;
	   sym != NULL; sym = sym->next_symbol)
	{
	  regmap = (L_Region_Regmap *) sym->data;

	  type = R_Ltype_to_Rtype (regmap->ctype);

	  if (regmap->phys_reg != -1)
	    {

	      if (regmap->type == L_OPERAND_REGISTER)
		{
		  base = R_find_base_from_phys_reg (regmap->phys_reg,
						    type, R_CALLER);
		  if (base != -1)
		    {
		      regmap->phys_reg = base;
		      regmap->type = R_CALLER;
		    }
		}
	      else
		{
		  base = R_find_base_from_phys_reg (regmap->phys_reg,
						    type, R_MACRO_CALLER);
		  if (base != -1)
		    {
		      regmap->phys_reg = base;
		      regmap->type = R_MACRO_CALLER;
		    }
		}
	      if (base == -1)
		{
		  if (regmap->type == L_OPERAND_REGISTER)
		    {
		      base = R_find_base_from_phys_reg (regmap->phys_reg,
							type, R_CALLEE);
		      if (base != -1)
			{
			  regmap->phys_reg = base;
			  regmap->type = R_CALLEE;
			}
		    }
		  else
		    {
		      base = R_find_base_from_phys_reg (regmap->phys_reg,
							type, R_MACRO_CALLEE);
		      if (base != -1)
			{
			  regmap->phys_reg = base;
			  regmap->type = R_MACRO_CALLEE;
			}
		    }
		  if (base == -1)
		    L_warn ("R_unmap_physical_registers: "
			    "Unable to unmap phys_reg %d\n",
			    regmap->phys_reg);
		}
	    }
	  n_vreg = Set_2array (regmap->occupied, vreg_array);
	  occupied_base = NULL;
	  for (i = 0; i < n_vreg; i++)
	    {
	      if ((base = R_find_base_from_phys_reg (vreg_array[i], type,
						     R_CALLER)) != -1 ||
		  (base = R_find_base_from_phys_reg (vreg_array[i], type,
						     R_MACRO_CALLER)) != -1 ||
		  (base = R_find_base_from_phys_reg (vreg_array[i], type,
						     R_CALLEE)) != -1 ||
		  (base = R_find_base_from_phys_reg (vreg_array[i], type,
						     R_MACRO_CALLEE)) != -1)
		{
		  occupied_base = Set_add (occupied_base, base);
		}
	      else
		{
		  L_warn ("R_unmap_physical_registers: "
			  "Unable to unmap phys_reg %d\n",
			  vreg_array[i]);
		}
	    }
	  if (occupied_base != NULL)
	    {
	      Set_dispose (regmap->occupied);
	      regmap->occupied = occupied_base;
	    }
	}
    }
  return;
}

void
R_determine_allocation_constraints (L_Region * region)
{
  int i, n_vreg, *vreg_array = R_buf;
  int vreg_id;
  L_Cb *cb, *src_cb, *dst_cb;
  Set in, out, tmp;
  R_Reg *vreg;
  L_Region_Boundary *bndry;
  L_Region_Regmap *regmap;

#ifdef  DEBUG_REGION_CONSTRAINTS
  fprintf (stderr, "****************************************\n");
  fprintf (stderr, "* Constraints on Region %d\n", region->id);
  fprintf (stderr, "*\n");
#endif

  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      cb = bndry->ecb;
      src_cb = bndry->bcb;

      if (src_cb == NULL)
	continue;

      in = bndry->live_in;
      n_vreg = Set_2array (in, vreg_array);
      /*
       * Unmap the virtual register id's
       */
      tmp = NULL;
      for (i = 0; i < n_vreg; i++)
	{
	  if (L_IS_MAPPED_REG (vreg_array[i]))
	    {
	      vreg_array[i] = L_UNMAP_REG (vreg_array[i]);
	      tmp = Set_add (tmp, vreg_array[i]);
	    }
	  else
	    vreg_array[i] = -1;
	}

#ifdef DEBUG_REGION_CONSTRAINTS
      fprintf (stderr, "* Entry from cb%d to cb%d\n", src_cb->id, cb->id);
      fprintf (stderr, "*\tBoundary Conditions : ");
#endif

      for (i = 0; i < n_vreg; i++)
	{
	  vreg_id = vreg_array[i];

	  if (vreg_id == -1)
	    continue;

	  vreg = VREG (vreg_id);

	  if (vreg->flags & R_PREALLOCATED_MACRO)
	    continue;

	  regmap = L_find_region_regmap (region, vreg->index);

#ifdef DEBUG_REGION_CONSTRAINTS
	  fprintf (stderr, "*\tVREG	%d :", vreg->index);
	  if (vreg->flags & R_PREALLOCATED_FLYBY)
	    fprintf (stderr, "(FB) ");
#endif

	  if (regmap->phys_reg != -1)
	    {
#ifdef DEBUG_REGION_CONSTRAINTS
	      fprintf (stderr, " allocated (GLOBAL) -> %d\n",
		       regmap->phys_reg);
#endif
	      vreg->flags |= R_REGION_CONSTRAINED;
	    }
	  else if (regmap->spill_loc != -1)
	    {
#ifdef DEBUG_REGION_CONSTRAINTS
	      fprintf (stderr, " spilled (GLOBAL) -> mem[%d]\n",
		       regmap->spill_loc);
#endif
	    }
	  else
	    {
	      vreg->constraints = Set_union_acc (vreg->constraints,
						 regmap->occupied);
#ifdef DEBUG_REGION_CONSTRAINTS
	      Set_print (stderr, " unusable", vreg->constraints);
#endif
	    }
	}
      Set_dispose (tmp);
    }

  for (bndry = region->exit_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      cb = bndry->ecb;
      dst_cb = bndry->bcb;

      if (dst_cb == NULL)
	continue;

      out = L_get_region_exit_OUT_set (region, dst_cb);
      n_vreg = Set_2array (out, vreg_array);
      /*
       * Unmap the virtual register id's
       */
      tmp = NULL;
      for (i = 0; i < n_vreg; i++)
	{
	  if (L_IS_MAPPED_REG (vreg_array[i]))
	    {
	      vreg_array[i] = L_UNMAP_REG (vreg_array[i]);
	      tmp = Set_add (tmp, vreg_array[i]);
	    }
	  else
	    {
	      vreg_array[i] = -1;
	    }
	}
#ifdef  DEBUG_REGION_CONSTRAINTS
      fprintf (stderr, "* Exit from cb%d to cb%d\n", cb->id, dst_cb->id);
      fprintf (stderr, "* 	Boundary Conditions : ");
      Set_print (stderr, "", out);
#endif

      for (i = 0; i < n_vreg; i++)
	{
	  vreg_id = vreg_array[i];

	  if (vreg_id == -1)
	    continue;

	  vreg = VREG (vreg_id);

	  if (vreg->flags & R_PREALLOCATED_MACRO)
	    continue;

	  regmap = L_find_region_regmap (region, vreg->index);

#ifdef DEBUG_REGION_CONSTRAINTS
	  fprintf (stderr, "*\tVREG	%d :", vreg->index);
	  if (vreg->flags & R_PREALLOCATED_FLYBY)
	    fprintf (stderr, "(FB) ");
#endif

	  if (regmap->phys_reg != -1)
	    {
#ifdef DEBUG_REGION_CONSTRAINTS
	      fprintf (stderr, " allocated (GLOBAL) -> %d\n",
		       regmap->phys_reg);
#endif
	      vreg->flags |= R_REGION_CONSTRAINED;
	    }
	  else if (regmap->spill_loc != -1)
	    {
#ifdef DEBUG_REGION_CONSTRAINTS
	      fprintf (stderr, " spilled (GLOBAL) -> mem[%d]\n",
		       regmap->spill_loc);
#endif
	    }
	  else
	    {
	      vreg->constraints = Set_union_acc (vreg->constraints,
						 regmap->occupied);
#ifdef DEBUG_REGION_CONSTRAINTS
	      Set_print (stderr, " unusable", vreg->constraints);
#endif
	    }
	}
      Set_dispose (tmp);
    }
#ifdef DEBUG_REGION_CONSTRAINTS
  fprintf (stderr, "****************************************\n");
#endif
}

double
R_determine_reconcile_cost (L_Region * region, R_Reg * vreg,
			    double *store_cost, double *load_cost)
{
  L_Cb *cb, *src_cb, *dst_cb;
  L_Flow *flow;
  L_Region_Boundary *bndry;
  L_Region_Regmap *regmap;
  L_Region_Regcon *regcon;
  Set in, out;
  double allocated, spilled, weight;
  double load, store;

  allocated = spilled = 0.0;
  load = *load_cost;
  store = *store_cost;

  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      cb = bndry->ecb;
      src_cb = bndry->bcb;

      if (src_cb == NULL)
	continue;

      in = bndry->live_in;
      if (Set_in (in, L_REG_INDEX (vreg->index)))
	vreg->flags |= R_LIVE_OUTSIDE_REGION;

      /*
       * Check to see what action was taken on the incoming src_cb.
       * If nothing, then we may ignore it.
       */
      regmap = L_find_region_regmap (region, vreg->index);
      if (regmap == NULL)
	continue;

      regcon = L_find_regcon_for_cb (regmap, src_cb);
      if (regcon == NULL)
	continue;

      /*
       * Determine the weight of all incoming arcs from src_cb
       */
      weight = 0.0;
      for (flow = src_cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  if (flow->dst_cb == cb)
	    weight += flow->weight;
	}

      /*
       * If the current vreg were to be allocated
       */
      if (regcon->flags & L_REGION_VREG_ALLOC)
	{
	  /* locally allocated and allocated in src, */
	  /* thus no correction code is required.    */
	}
      else if (regcon->flags & L_REGION_VREG_SPILL)
	{
	  /* locally allocated, but spilled in src,   */
	  /* a load is required in the current region */
	  allocated += weight * load;
	}


      /*
       * If the current vreg were to be spilled 
       */
      if (regcon->flags & L_REGION_VREG_ALLOC)
	{
	  /* locally spilled, but allocated in src,    */
	  /* a store is required in the current region */
	  spilled += weight * store;
	}
      else if (regcon->flags & L_REGION_VREG_SPILL)
	{
	  /* locally spilled and spilled in src, */
	  /* therefore no correction code is required */
	}
    }
  for (bndry = region->exit_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      cb = bndry->ecb;
      dst_cb = bndry->bcb;

      if (dst_cb == NULL)
	continue;

      out = L_get_region_exit_OUT_set (region, dst_cb);
      if (Set_in (out, L_REG_INDEX (vreg->index)))
	vreg->flags |= R_LIVE_OUTSIDE_REGION;

      /*
       * Check to see what action was taken on the incoming src_cb.
       * If nothing, then we may ignore it.
       */
      regmap = L_find_region_regmap (region, vreg->index);
      if (regmap == NULL)
	continue;

      regcon = L_find_regcon_for_cb (regmap, dst_cb);
      if (regcon == NULL)
	continue;

      /*
       * Determine the weight of all incoming arcs from src_cb
       */
      weight = 0.0;
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
	{
	  if (flow->dst_cb == dst_cb)
	    weight += flow->weight;
	}
      /*
         * If the current vreg were to be allocated
       */
      if (regcon->flags & L_REGION_VREG_ALLOC)
	{
	  /* locally allocated, and allocated in dst, */
	  /* therefore no correction code is required */
	}
      else if (regcon->flags & L_REGION_VREG_SPILL)
	{
	  /* locally allocated, but spilled in dst, thus */
	  /* a store is required in the current region   */
	  allocated += weight * store;
	}


      /*
       * If the current vreg were to be spilled 
       */
      if (regcon->flags & L_REGION_VREG_ALLOC)
	{
	  /* locally spilled, and allocated in dst,        */
	  /* thus a load is required in the current region */
	  spilled += weight * load;
	}
      else if (regcon->flags & L_REGION_VREG_SPILL)
	{
	  /* locally spilled, and spilled in dst */
	  /* No correction code is required      */
	}
    }
  return (spilled - allocated);
}



RCB *rcbList = NULL;

static RCB *
newRCB ()
{
  RCB *new_rcb = (RCB *) malloc (sizeof (RCB));

  new_rcb->location = NULL;
  new_rcb->fromCb = NULL;
  new_rcb->toCb = NULL;
  new_rcb->flow = NULL;
  new_rcb->branch = NULL;
  new_rcb->stores = NULL;
  new_rcb->store_loc = NULL;
  new_rcb->loads = NULL;
  new_rcb->load_loc = NULL;
  new_rcb->nextRCB = NULL;

  return (new_rcb);
}

static void
freeRCB (RCB * rcb)
{
  Set_dispose (rcb->store_loc);
  Set_dispose (rcb->load_loc);
  free (rcb);
}


static RCB *
findRCB (L_Cb * fromCb, L_Cb * toCb)
{
  RCB *tmp = rcbList;

  while (tmp != NULL)
    {
      if (tmp->fromCb == fromCb && tmp->toCb == toCb	/* &&
							   tmp->branch == branch */ )
	break;
      tmp = tmp->nextRCB;
    }
  return (tmp);
}

void
R_add_store (L_Region * region, L_Cb * fromCb, L_Cb * toCb, R_Reg * vreg,
	     L_Region_Regmap * regmap)
{
  L_Cb *insCb;
  L_Oper *oper;
  L_Operand opd;
  RCB *rcb;

  if (!L_EXTRACT_BIT_VAL (fromCb->flags, L_CB_BOUNDARY))
    {
      insCb = fromCb;
#ifdef DEBUG_REGION_RECONCILE
      fprintf (stderr, "*\tInsert Store %d -> mem[%d] into Cb %d\n",
	       vreg->base_index, regmap->spill_loc, insCb->id);
#endif

    }
  else
    {
      insCb = toCb;
#ifdef DEBUG_REGION_RECONCILE
      fprintf (stderr, "*\tInsert Store %d -> mem[%d] into Cb %d\n",
	       vreg->base_index, regmap->spill_loc, insCb->id);
#endif

    }
  opd.type = R_conv_rclass_to_Lclass (vreg->rclass);
  opd.ctype = R_conv_type_to_Ltype (vreg->type);
  opd.value.r = vreg->index;

  rcb = findRCB (fromCb, toCb);

  if (rcb && Set_in (rcb->store_loc, regmap->spill_loc))
    {
      /* The current location has already been spilled in this block */
      /* thus there is no need to perform another                    */
      return;
    }

  oper =
    O_spill_reg (regmap->phys_reg, R_conv_rclass_to_Lclass (vreg->rclass),
		 &opd, regmap->spill_loc, NULL, R_SPILL_CODE);

  if (rcb != NULL)
    {
      oper->next_op = rcb->stores;
      rcb->stores = oper;
    }
  else
    {
      rcb = newRCB ();
#if 0
      if (branch == NULL)
	rcb->location = fromCb;
#endif
      rcb->fromCb = fromCb;
      rcb->toCb = toCb;
#if 0
      rcb->flow = flow;
      rcb->branch = branch;
#endif
      rcb->stores = oper;
      oper->next_op = NULL;

      rcb->nextRCB = rcbList;
      rcbList = rcb;
    }
  rcb->store_loc = Set_add (rcb->store_loc, vreg->spill_loc);
}


void
R_add_load (L_Region * region, L_Cb * fromCb, L_Cb * toCb, R_Reg * vreg,
	    L_Region_Regmap * regmap)
{
  L_Cb *insCb;
  L_Oper *oper;
  L_Operand opd;
  RCB *rcb;

  if (!L_EXTRACT_BIT_VAL (fromCb->flags, L_CB_BOUNDARY))
    {
      insCb = fromCb;
#ifdef DEBUG_REGION_RECONCILE
      fprintf (stderr, "*\tInsert Load %d <- mem[%d] into Cb %d\n",
	       vreg->base_index, regmap->spill_loc, insCb->id);
#endif
    }
  else
    {
      insCb = toCb;
#ifdef DEBUG_REGION_RECONCILE
      fprintf (stderr, "*\tInsert Load %d <- mem[%d] into Cb %d\n",
	       vreg->base_index, regmap->spill_loc, insCb->id);
#endif
    }

  opd.type = R_conv_rclass_to_Lclass (vreg->rclass);
  opd.ctype = R_conv_type_to_Ltype (vreg->type);
  opd.value.r = vreg->index;

  rcb = findRCB (fromCb, toCb);
  if (rcb && Set_in (rcb->load_loc, regmap->spill_loc))
    {
      /* The current location has already been spilled in this block */
      /* thus there is no need to perform another                    */
      /*
         fprintf(stderr,"Redundant load detected\n");
       */
      return;
    }

  oper = O_fill_reg (regmap->phys_reg, R_conv_rclass_to_Lclass (vreg->rclass),
		     &opd, regmap->spill_loc, NULL, R_SPILL_CODE);

  if (rcb != NULL)
    {
      oper->next_op = rcb->loads;
      rcb->loads = oper;
    }
  else
    {
      rcb = newRCB ();
#if 0
      if (branch == NULL)
	rcb->location = fromCb;
#endif
      rcb->fromCb = fromCb;
      rcb->toCb = toCb;
#if 0
      rcb->flow = flow;
      rcb->branch = branch;
#endif
      rcb->loads = oper;
      oper->next_op = NULL;

      rcb->nextRCB = rcbList;
      rcbList = rcb;
    }
  rcb->load_loc = Set_add (rcb->load_loc, vreg->spill_loc);
}

void
R_insert_reconciliation_code (L_Func * fn, L_Region * region)
{
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int ext;
#endif
  RCB *rcb, *next;
  L_Flow *flow;
  L_Oper *oper, *next_op, *jump;
  L_Cb *new_cb;
  double weight;

  for (rcb = rcbList; rcb != NULL; rcb = next)
    {
      next = rcb->nextRCB;

      weight = 0.0;
      new_cb = L_create_cb (weight);
      for (oper = rcb->fromCb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (!L_general_branch_opcode (oper))
	    continue;

	  flow = L_find_flow_for_branch (rcb->fromCb, oper);
	  if (L_register_branch_opcode (oper))
	    {
	      while (flow)
		{
		  if (flow->dst_cb == rcb->toCb)
		    {
		      flow->dst_cb = new_cb;
		      weight += flow->weight;
		    }
		  flow = flow->next_flow;
		}
	    }
	  else
	    {
	      if (flow->dst_cb == rcb->toCb)
		{
		  flow->dst_cb = new_cb;
		  weight += flow->weight;
		}
	    }
	  L_change_branch_dest (oper, rcb->toCb, new_cb);
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
	  if (M_arch == M_HPPA)
	    {
	      L_get_attribute (oper, &ext);
	      L_set_attribute (oper,
			       EXT (CBR_FORWARD_EXT, NULL_COND (ext),
				    INSTR_EXT (ext)));
	    }
#endif
#endif
	}
      if (!L_uncond_branch_opcode (rcb->fromCb->last_op) &&
	  !L_register_branch_opcode (rcb->fromCb->last_op) &&
	  ((flow = L_find_last_flow (rcb->fromCb->dest_flow)) != NULL))
	{
	  /* Make sure the fall thru flow is also changed if necessary */
	  if (flow->dst_cb == rcb->toCb)
	    {
	      flow->dst_cb = new_cb;
	      weight += flow->weight;

	      if (rcb->fromCb->next_cb != new_cb)
		{
		  /* Must insert unconditional branch */
		  L_Oper *new_oper = L_create_new_op (Lop_JUMP);
		  new_oper->src[0] = L_new_cb_operand (new_cb);

		  L_insert_oper_after (rcb->fromCb, rcb->fromCb->last_op,
				       new_oper);
		}
	    }
	}

      new_cb->weight = weight;
#if 0
      fprintf (stderr, "# NEWCB %d between Cb %d and Cb %d\n",
	       new_cb->id, rcb->fromCb->id, rcb->toCb->id);
#endif

      /* Insert instructions into cb */
      for (oper = rcb->stores; oper != NULL; oper = next_op)
	{
	  next_op = oper->next_op;
	  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SPILL_CODE);
	  L_insert_oper_after (new_cb, new_cb->first_op, oper);
#if 0
	  L_print_oper (stderr, oper);
#endif
	  total_region_spill_weight += new_cb->weight;
	}
      for (oper = rcb->loads; oper != NULL; oper = next_op)
	{
	  next_op = oper->next_op;
	  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_SPILL_CODE);
	  L_insert_oper_after (new_cb, new_cb->last_op, oper);
#if 0
	  L_print_oper (stderr, oper);
#endif
	  total_region_spill_weight += new_cb->weight;
	}


      /* Insert jump instruction into cb */
      jump = (L_Oper *) O_jump_oper (Lop_JUMP, rcb->toCb);
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
      if (M_arch == M_HPPA)
	{
	  L_set_attribute (jump, EXT (CBR_BACKWARD_EXT, 0, 0));
	}
#endif
#endif
      L_insert_oper_after (new_cb, new_cb->last_op, jump);



      new_cb->dest_flow = L_new_flow (1, new_cb, rcb->toCb, weight);

      L_insert_cb_after (fn, fn->last_cb, new_cb);

      freeRCB (rcb);

      /*
       * Update region exit boundary information
       */
      if (L_EXTRACT_BIT_VAL (rcb->toCb->flags, L_CB_EXIT_BOUNDARY))
	{
	  L_change_region_exit_cb (fn->first_region, rcb->fromCb,
				   new_cb, rcb->toCb);
	}
    }
}

void
R_reconcile_allocated_region (L_Region * region)
{
  int i, vreg_id, n_vreg, *vreg_array = R_buf;
  L_Cb *cb, *src_cb, *dst_cb;
  Set in, out;
  R_Reg *vreg;
  L_Region_Boundary *bndry;
  L_Region_Regmap *regmap;
  L_Region_Regcon *regcon;

#ifdef DEBUG_REGION_RECONCILE
  fprintf (stderr, "****************************************\n");
  fprintf (stderr, "* Reconciling Region %d\n", region->id);
  fprintf (stderr, "*\n");
#endif
  rcbList = NULL;
#ifdef DEBUG_REGION_RECONCILE
  fprintf (stderr, "* Entry points\n");
#endif
  for (bndry = region->entry_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      cb = bndry->ecb;
      src_cb = bndry->bcb;

      if (src_cb == NULL)
	continue;

      in = bndry->live_in;

      n_vreg = Set_2array (in, vreg_array);
      for (i = 0; i < n_vreg; i++)
	{
	  vreg_id = vreg_array[i];

	  if (!L_IS_MAPPED_REG (vreg_id))
	    continue;

	  vreg = VREG (L_UNMAP_REG (vreg_id));

	  if (vreg->flags & R_PREALLOCATED_MACRO)
	    continue;

	  regmap = L_find_region_regmap (region, vreg->index);

	  regcon = L_find_regcon_for_cb (regmap, src_cb);
	  if (regcon == NULL)
	    continue;
#ifdef DEBUG_REGION_RECONCILE
	  fprintf (stderr, "VREG %d\n", vreg->index);
#endif
	  if (vreg->flags & R_SPILLED)
	    {
	      if (regcon->flags & L_REGION_VREG_ALLOC)
		{
		  /* locally spilled, but allocated in src */
#ifdef DEBUG_REGION_RECONCILE
		  fprintf (stderr, "\tlocally spilled, src allocated -> %d\n",
			   regmap->phys_reg);
#endif
		  R_add_store (region, src_cb, cb, vreg, regmap);
		}
	      else if (regcon->flags & L_REGION_VREG_SPILL)
		{
		  /* locally spilled and spilled in src */
		}
	      else if (regcon->flags & L_REGION_VREG_IGNORE)
		{
		  /* If a cb says the lifetime was ignored, then this is */
		  /* the first action taken, i.e. it is spilled first.   */
		  /* Nothing need be done.  Future regions will see the  */
		  /* lifetime as spilled in the src region.              */
		}
	    }
	  else if (vreg->base_index != -1)
	    {
	      /* Aaah, so we locally allocated the bloomin' thing. */
	      if (regcon->flags & L_REGION_VREG_ALLOC)
		{
		  /* locally allocated and src allocated */
		}
	      else if (regcon->flags & L_REGION_VREG_SPILL)
		{
		  /* locally allocated, but src spilled */
#ifdef DEBUG_REGION_RECONCILE
		  fprintf (stderr,
			   "\tlocally allocated, src spilled -> mem[%d]\n",
			   regmap->spill_loc);
#endif
		  R_add_load (region, src_cb, cb, vreg, regmap);
		}
	      else if (regcon->flags & L_REGION_VREG_IGNORE)
		{
		  /* If a cb says the lifetime was ignored, then this is */
		  /* the first action taken, i.e. it is allocated first. */
		  /* Nothing need be done.  Future regions will see the  */
		  /* lifetime as allocated in the src region.            */
		}
	    }
	  else
	    {
	      /* Otherwise we ignored it completely */
	      if (regcon->flags & L_REGION_VREG_ALLOC)
		{
		  /* locally ignored, src allocated */
		  if (regmap->flags & L_REGION_VREG_SPILL)
		    {
#ifdef DEBUG_REGION_RECONCILE
		      fprintf (stderr, "\tlocally ignored, src allocated\n");
#endif
		      R_add_store (region, src_cb, cb, vreg, regmap);
		    }
		}
	      else if (regcon->flags & L_REGION_VREG_SPILL)
		{
		  /* locally ignores, but spilled in source */
		  if (regmap->flags & L_REGION_VREG_ALLOC)
		    {
#ifdef DEBUG_REGION_RECONCILE
		      fprintf (stderr, "\tlocally ignored, src spilled\n");
#endif
		      R_add_load (region, src_cb, cb, vreg, regmap);
		    }
		}
	    }
	}
    }
#ifdef DEBUG_REGION_RECONCILE
  fprintf (stderr, "* Exit points\n");
#endif
  for (bndry = region->exit_cbs; bndry != NULL; bndry = bndry->next_boundary)
    {
      cb = bndry->ecb;
      dst_cb = bndry->bcb;

      if (dst_cb == NULL)
	continue;

      out = L_get_region_exit_OUT_set (region, dst_cb);

      n_vreg = Set_2array (out, vreg_array);
      for (i = 0; i < n_vreg; i++)
	{
	  vreg_id = vreg_array[i];

	  if (!L_IS_MAPPED_REG (vreg_id))
	    continue;

	  vreg = VREG (L_UNMAP_REG (vreg_id));

	  if (vreg->flags & R_PREALLOCATED_MACRO)
	    {
	      continue;
	    }
	  regmap = L_find_region_regmap (region, vreg->index);

	  regcon = L_find_regcon_for_cb (regmap, dst_cb);
	  if (regcon == NULL)
	    continue;
#ifdef DEBUG_REGION_RECONCILE
	  fprintf (stderr, "VREG %d\n", vreg->index);
#endif
	  if (vreg->flags & R_SPILLED)
	    {
	      if (regcon->flags & L_REGION_VREG_ALLOC)
		{
		  /* locally spilled, but allocated in src */
#ifdef DEBUG_REGION_RECONCILE
		  fprintf (stderr, "\tlocally spilled, dst allocated -> %d\n",
			   regmap->phys_reg);
#endif
		  R_add_load (region, cb, dst_cb, vreg, regmap);
		}
	      else if (regcon->flags & L_REGION_VREG_SPILL)
		{
		  /* locally spilled and spilled in dst */
		}
	      else if (regcon->flags & L_REGION_VREG_IGNORE)
		{
		  /* If a cb says the lifetime was ignored, then this is */
		  /* the first action taken, i.e. it is spilled first.   */
		  /* Nothing need be done.  Future regions will see the  */
		  /* lifetime as spilled in the dst region.              */
		}
	    }
	  else if (vreg->base_index != -1)
	    {
	      /* Aaah, so we locally allocated the bloomin' thing. */
	      if (regcon->flags & L_REGION_VREG_ALLOC)
		{
		  /* locally allocated and dst allocated */
		}
	      else if (regcon->flags & L_REGION_VREG_SPILL)
		{
		  /* locally allocated, but dst spilled */
#ifdef DEBUG_REGION_RECONCILE
		  fprintf (stderr,
			   "\tlocally allocated, dst spilled -> mem[%d]\n",
			   regmap->spill_loc);
#endif
		  R_add_store (region, cb, dst_cb, vreg, regmap);
		}
	      else if (regcon->flags & L_REGION_VREG_IGNORE)
		{
		  /* If a cb says the lifetime was ignored, then this is */
		  /* the first action taken, i.e. it is allocated first. */
		  /* Nothing need be done.  Future regions will see the  */
		  /* lifetime as allocated in the src region.            */
		}
	    }
	  else
	    {
	      /* Otherwise we ignored it completely */
	      if (regcon->flags & L_REGION_VREG_ALLOC)
		{
		  /* locally ignored, src allocated */
		  if (regmap->flags & L_REGION_VREG_SPILL)
		    {
#ifdef DEBUG_REGION_RECONCILE
		      fprintf (stderr, "\tlocally ignored, dst allocated\n");
#endif
		      R_add_load (region, cb, dst_cb, vreg, regmap);
		    }
		}
	      else if (regcon->flags & L_REGION_VREG_SPILL)
		{
		  /* locally ignores, but spilled in source */
		  if (regmap->flags & L_REGION_VREG_ALLOC)
		    {
#ifdef DEBUG_REGION_RECONCILE
		      fprintf (stderr, "\tlocally ignored, dst spilled\n");
#endif
		      R_add_store (region, cb, dst_cb, vreg, regmap);
		    }
		}
	    }
	}
    }

#ifdef DEBUG_REGION_RECONCILE
  fprintf (stderr, "****************************************\n");
#endif
}

void
R_update_alloc_state (L_Region * region)
{
  int reg, *phys_map;
  int i, j, n_reg, *reg_array = R_buf;
  R_Physical_Bank *bank;
  R_Reg *vreg_ptr;
  Set flyby_constraints = NULL;
  L_Region_Regmap *regmap;
  Set occupied_phys_reg;

  for (vreg_ptr = R_vreg; vreg_ptr != NULL; vreg_ptr = vreg_ptr->nextReg)
    {
      if (vreg_ptr->flags & R_PREALLOCATED_MACRO)
	{
	  continue;
	}

      if (vreg_ptr->flags & R_PREALLOCATED_FLYBY &&
	  !(vreg_ptr->flags & R_SPILLED))
	{
	  continue;
	}
      if (vreg_ptr->base_index != -1)
	flyby_constraints = Set_add (flyby_constraints, vreg_ptr->base_index);
    }
  for (vreg_ptr = R_vreg; vreg_ptr != NULL; vreg_ptr = vreg_ptr->nextReg)
    {
      if (vreg_ptr->flags & R_PREALLOCATED_MACRO)
	{
	  continue;
	}
      regmap = L_find_region_regmap (region, vreg_ptr->index);
      if (regmap == NULL)
	continue;

      if (vreg_ptr->flags & R_SPILLED)
	{
	  /*
	     if ( regmap->spill_loc == -1 )
	   */
	  regmap->flags = L_REGION_VREG_SPILL;
	  regmap->spill_loc = vreg_ptr->spill_loc;

	  if (regmap->phys_reg != -1)
	    {
	      /* Only convert from base_index to phys_register if the */
	      /* thing has been allocated once to begin with.         */
	      bank = R_bank + R_Ltype_to_Rtype (regmap->ctype) + regmap->type;
	      phys_map =
		R_map[R_Ltype_to_Rtype (regmap->ctype) + regmap->type];
	      regmap->phys_reg =
		phys_map[regmap->phys_reg - bank->base_index];
	      regmap->type = R_conv_rclass_to_Lclass (regmap->type);
	    }
	}
      else if (vreg_ptr->base_index != -1)
	{
	  /*
	     if ( regmap->phys_reg == -1 )
	   */
	  regmap->flags = L_REGION_VREG_ALLOC;
	  regmap->phys_reg = vreg_ptr->phys_reg;
	  regmap->type = R_conv_rclass_to_Lclass (vreg_ptr->rclass);
	}

      if (regmap->phys_reg == -1)
	{
#if 0
	  fprintf (stderr, "Constraining %d with flybys:", vreg_ptr->index);
	  Set_print (stderr, "", flyby_constraints);
#endif
	  /* only need to add further constraints if not yet allocated */
	  regmap->occupied =
	    Set_union_acc (regmap->occupied, flyby_constraints);

	  if (vreg_ptr->flags & R_CONTAINS_JSR)
	    {
	      Set occupied = NULL;
	      /* A flyby virtual register that has not yet been allocated */
	      /* cannot be allocated to a caller saved register.  Doing so */
	      /* would result in an incorrect allocation for the current  */
	      /* region, since spill code would be required around the    */
	      /* subroutine calls.                                        */
	      bank = R_bank + vreg_ptr->type + R_CALLER;
	      if (bank->defined)
		for (reg = bank->base_index;
		     reg < bank->base_index + bank->max; reg += bank->res_inc)
		  {
		    occupied = Set_add (occupied, reg);
		  }
	      bank = R_bank + vreg_ptr->type + R_MACRO_CALLER;
	      if (bank->defined)
		for (reg = bank->base_index;
		     reg < bank->base_index + bank->max; reg += bank->res_inc)
		  {
		    occupied = Set_add (occupied, reg);
		  }
	      regmap->occupied = Set_union_acc (regmap->occupied, occupied);
	    }
	}
      occupied_phys_reg = NULL;
      n_reg = Set_2array (regmap->occupied, reg_array);
      for (i = 0; i < n_reg; i++)
	{
	  int found;
	  reg = reg_array[i];
	  found = 0;
	  for (j = 0; j < R_MAX_BANK; j++)
	    {
	      bank = R_bank + j;
	      if (bank->defined &&
		  bank->base_index <= reg &&
		  reg < (bank->base_index + bank->max))
		{
		  int *phys_map;

		  phys_map = R_map[bank->type + bank->rclass];
		  if (bank->type == vreg_ptr->type)
		    {
		      occupied_phys_reg = Set_add (occupied_phys_reg,
						   phys_map[reg -
							    bank->
							    base_index]);
		      found = 1;
		    }
		}
	      if (found)
		break;
	    }
	}
      if (occupied_phys_reg != NULL)
	{
	  Set_dispose (regmap->occupied);
	  regmap->occupied = occupied_phys_reg;
	}
    }

  if (flyby_constraints)
    flyby_constraints = Set_dispose (flyby_constraints);
}

int
R_init_spill_stack (L_Region * region, int *int_spill_stack,
		    int *fp_spill_stack, int *pred_spill_stack)
{
  L_Attr *attr;

  if (R_Region_Based_Allocation == 1)
    {
      /* There better be a swap attribute on the region so we know */
      /* how big the swap space is at the moment.                  */
      if ((attr = L_find_attr (region->attr, "swap")) != NULL)
	{
	  *int_spill_stack = (int) attr->field[0]->value.i;
	  if (attr->max_field >= 2)
	    {
	      *fp_spill_stack = (int) attr->field[1]->value.i;
	    }
	  else
	    {
	      *fp_spill_stack = 0;
	    }
	  if (attr->max_field >= 3)
	    {
	      *pred_spill_stack = (int) attr->field[2]->value.i;
	    }
	  else
	    {
	      *pred_spill_stack = 0;
	    }
	  return (*int_spill_stack + *fp_spill_stack +*pred_spill_stack);
	}
      else
	{
	  L_punt ("R_init_spill_stack: No swap size provided on region %d\n",
		  region->id);
	  return 0;
	}
    }
  else
    {
      /* Function based allocation starts at zero */
      *int_spill_stack = 0;
      *fp_spill_stack = 0;
      *pred_spill_stack = 0;
      return (0);
    }
}


void
R_update_region_spill_stack (L_Region * region)
{
  L_Attr *attr;

  if (R_Region_Based_Allocation == 1)
    {
      if ((attr = L_find_attr (region->attr, "swap")) != NULL)
	{
	  attr->field[0]->value.i = (ITintmax) int_spill_stack;
	  attr->field[1]->value.i = (ITintmax) fp_spill_stack;
	}
      else
	{
	  L_punt ("R_update_region_spill_stack: cannot update swap size for "
		  "region %d\n", region->id);
	}
    }
}


L_Region *
R_assemble_region (L_Func * fn)
{
  L_Cb *cb;
  L_Attr *attr;
  L_Region *region;

  if (R_Region_Based_Allocation == 1 && fn->first_region != NULL)
    {
      if (fn->first_region->next_region != NULL)
	L_punt ("R_assemble_region: "
		"Function contains multiple regions, which one??\n");

      if ((attr = L_find_attr (fn->first_region->attr, "reg_func_weight")) !=
	  NULL)
	fn->weight = attr->field[0]->value.f2;

      return (fn->first_region);
    }
  else
    {
      /* Build a region from the function body */
      region = L_new_region_in_func (fn, 0);

      region->next_region = fn->first_region;
      fn->first_region = region;

      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	{
	  if (cb->region != NULL)
	    L_remove_cb_from_region (cb->region, cb);

	  L_add_cb_to_region (region, cb);
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_PROLOGUE))
	    L_add_entry_cb_to_region (region, cb, NULL);
	  else if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_EPILOGUE))
	    L_add_entry_cb_to_region (region, cb, NULL);
	}
      return (region);
    }
  return (NULL);
}

void
R_disassemble_regions (L_Func * fn)
{
  L_Cb *cb;

  /* 
   * We are not performing region-base register allocation, so let's
   * rip the region data structures back out of the function so that
   * they don't cause any trouble.
   */

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (cb->region != NULL)
	{
	  L_remove_cb_from_region (cb->region, cb);
	  cb->region = NULL;
	}
    }
  L_delete_region (fn->first_region);
  fn->first_region = NULL;
}
