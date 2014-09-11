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
 *      File :          lp_pred_opti.c
 *      Description :   predicate optimization
 *      Creation Date : March 2001
 *      Author :        John W. Sias
 *
 *      (C) Copyright 2001, John Sias and Wen-mei Hwu
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"
#include "l_promotion.h"

#undef DEBUG_PRED_OPTI


/*
 * L_local_pred_cse (L_Cb *cb)
 * ----------------------------------------------------------------------
 * For predicate uses, attempt to find logically equivalent, locally
 * preceding predicate definitions
 */

int
L_local_pred_cse (L_Cb *cb)
{
  List avail_ssa = NULL;
  L_Oper *op;
  L_Operand *opd;
  PG_Pred_SSA *gps, *pps;
  int i, change = 0;

  for (op = cb->first_op; op; op = op->next_op)
    {
      if ((opd = op->pred[0]) && (gps = opd->value.pred.ssa))
	{
	  List_start (avail_ssa);
	  while ((pps = List_next (avail_ssa)))
	    if (gps->node == pps->node)
	      {
		if (L_IS_MAPPED_REG (pps->pg_pred->pred))
		  {
		    opd->type = L_OPERAND_REGISTER;
		    opd->value.pred.reg = L_UNMAP_REG (pps->pg_pred->pred);
		  }
		else
		  {
		    opd->type = L_OPERAND_MACRO;
		    opd->value.pred.reg = L_UNMAP_MAC (pps->pg_pred->pred);
		  }
		change++;
		break;
	      }
	}

      if (!L_pred_define_opcode (op))
	continue;

      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (!(opd = op->dest[i]) || 
	      !L_is_ctype_predicate (opd) ||
	      !(gps = opd->value.pred.ssa))
	    continue;
	  
	  List_start (avail_ssa);
	  while ((pps = List_next (avail_ssa)))
	    if (gps->pg_pred == pps->pg_pred)
	      avail_ssa = List_delete_current (avail_ssa);
	  
	  avail_ssa = List_insert_last (avail_ssa, gps);
	}
    }

  List_reset (avail_ssa);
  return change;
}


/*
 * L_remove_red_guards(L_Func *fn)
 * --------------------------------------------------------------------------
 * Removes logically redundant guards on compare ops
 * Assumes pred defs are split and pred graph is up to date
 * JWS 20010117 
 */

int
L_remove_red_guards (L_Func * fn)
{
  int count = 0;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *dest, *guard;
  PG_Pred_SSA *dssa, *gssa;
  PG_Cond *cond;
  PG_Pred_Graph *pg;
  int ptype;
  DdNode *Cnode, *dnode, *chknode;

  pg = PG_pred_graph;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          int opti = 0;

          if (!(guard = op->pred[0]))
            continue;
          if (!(gssa = guard->value.pred.ssa))
	    {
	      L_warn ("L_remove_red_guards: Missing guard SSA");
	      continue;
	    }
	  if (gssa->node == pg->one)
	    {
	      opti = 1;
	    }
          else if (L_general_pred_comparison_opcode (op))
	    {
	      if (!(dest = op->dest[0]))
		continue;

	      if (!(dssa = dest->value.pred.ssa))
		{
		  L_warn ("L_remove_red_guards: Missing dest SSA");
		  continue;
		}

	      dnode = dssa->node;
	      ptype = dest->ptype;

	      if (!(cond = dssa->pg_cond))
		continue;

	      Cnode = ((PG_Cond *) PG_POINTER_REGULAR (cond))->node;

	      if (PG_POINTER_IS_COMPLEMENT (cond))
		Cnode = Cudd_Not (Cnode);

	      if (L_false_ptype(ptype))
		Cnode = Cudd_Not (Cnode);

	      switch (ptype)
		{
		case L_PTYPE_UNCOND_T:
		case L_PTYPE_UNCOND_F:
		case L_PTYPE_COND_T:
		case L_PTYPE_COND_F:

		  /* compare d to C */
		  
		  if (dnode == Cnode)
		    opti = 1;
		  
		  break;
		case L_PTYPE_AND_T:
		case L_PTYPE_AND_F:
		  
		  /* compare d to dC */
		  
		  chknode = Cudd_bddAnd (pg->dd, dnode, Cnode);
		  if (chknode == dnode)
		    opti = 1;
		  Cudd_Ref (chknode);
		  Cudd_RecursiveDeref (pg->dd, chknode);
		  break;
		case L_PTYPE_OR_T:
		case L_PTYPE_OR_F:
		  
		  /* compare d to d+C */
		  
		  chknode = Cudd_bddOr (pg->dd, dnode, Cnode);
		  if (chknode == dnode)
		    opti = 1;
		  Cudd_Ref (chknode);
		  Cudd_RecursiveDeref (pg->dd, chknode);
		  break;
		default:
		  continue;
		}
	    }
          if (opti && !L_promotion_unprofitable (NULL, op))
            {
              count++;
              L_delete_operand (guard);
              op->pred[0] = NULL;

#ifdef DEBUG_PRED_OPTI
	      L_warn ("PRO> Deleting guard on oper %d", op->id);
#endif	      
            }
        }
    }
  return count;
}


/*
 * L_form_and_chains(L_Func *fn)
 * --------------------------------------------------------------------------
 * Forms and chains from uncond chains (for parallelism and cmp combining)
 * Assumes pred defs are split and pred graph is up to date
 * JWS 20010117
 */

int
L_form_and_chains (L_Func * fn)
{
  int count = 0;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *dest;
  PG_Pred_SSA *dssa, *gssa;
  PG_Pred_Graph *pg;

  Set uncond_def_ssa = NULL;
  Set gen_use_ssa = NULL;
  Set new_inits = NULL;

  pg = PG_pred_graph;

  {
    /* Candidate SSA:
     * - defined by an unconditional define
     * - used at most once
     * - guards only a pred define (if anything at all)
     */

    Set single_use_ssa = NULL;
    Set bad_use_ssa = NULL;

    uncond_def_ssa = NULL;
    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (op->pred[0])
            {
              if (!(gssa = op->pred[0]->value.pred.ssa))
		break; /* Ran into unreachable code */

	      if (gssa->oper)
		{
		  /* "Real definition" */
		  if (!L_general_pred_comparison_opcode (op))
		    gen_use_ssa = Set_add (gen_use_ssa, gssa->ssa_indx);
		  else if (Set_in (single_use_ssa, gssa->ssa_indx))
		    bad_use_ssa = Set_add (bad_use_ssa, gssa->ssa_indx);
		  else
		    single_use_ssa = Set_add (single_use_ssa, gssa->ssa_indx);
		}
	      else
		{
		  /* "Phi definition" */
		  if (!L_general_pred_comparison_opcode (op))
		    {
		      gen_use_ssa = Set_add (gen_use_ssa, gssa->ssa_indx);
		      gen_use_ssa = Set_union_acc (gen_use_ssa,
						   gssa->composition);
		    }
		  else
		    {
		      Set bad;

		      if (Set_in (single_use_ssa, gssa->ssa_indx))
			bad_use_ssa = Set_add (bad_use_ssa, gssa->ssa_indx);
		      else
			single_use_ssa = Set_add (single_use_ssa, 
						  gssa->ssa_indx);

		      bad = Set_intersect (single_use_ssa, gssa->composition);

		      bad_use_ssa = Set_union_acc (bad_use_ssa, bad);
		      bad = Set_dispose (bad);
		      single_use_ssa= Set_union_acc (single_use_ssa,
						     gssa->composition);
		    }
		}
            }
          if (L_general_pred_comparison_opcode (op) &&
              (dest = op->dest[0]) && L_uncond_ptype (dest->ptype))
            {
              if (!(dssa = dest->value.pred.ssa))
		break; /* ran into unreachable code */
              uncond_def_ssa = Set_add (uncond_def_ssa, dssa->ssa_indx);
            }
        }

    uncond_def_ssa = Set_subtract_acc (uncond_def_ssa, bad_use_ssa);

    Set_dispose (bad_use_ssa);
    Set_dispose (single_use_ssa);
  }

  /* Set uncond_defs contains potential transformees 
   * gen_use_ssa contains defs with non-pg uses
   */

  if (Set_size (uncond_def_ssa))
    {
      Set working_set = NULL;
      L_Oper *sop;
      PG_Pred_SSA *sssa;
      L_Operand *last_pred = NULL;

      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
        {
          for (op = cb->first_op; op != NULL; op = op->next_op)
            {
              if (!L_general_pred_comparison_opcode (op) ||
                  !(dest = op->dest[0]))
                continue;

	      /* If dssa is null, this is unreachable code */
              if (!(dssa = dest->value.pred.ssa))
		break;

              if (!Set_in (uncond_def_ssa, dssa->ssa_indx) ||
                  Set_in (gen_use_ssa, dssa->ssa_indx))
                continue;

              working_set = Set_add (working_set, op->id);

              for (sop = op->next_op; sop != NULL; sop = sop->next_op)
                {
                  if (!L_general_pred_comparison_opcode (sop) ||
                      !sop->pred[0] || 
		      !sop->dest[0] ||
                      !(sssa = sop->pred[0]->value.pred.ssa) || 
		      (sssa != dssa))
                    continue;

                  dssa = sop->dest[0]->value.pred.ssa;

                  if (!dssa ||
		      !Set_in (uncond_def_ssa, dssa->ssa_indx))
                    break;

                  /* sop is a candidate */

                  working_set = Set_add (working_set, sop->id);

                  last_pred = sop->dest[0];

                  if (Set_in (gen_use_ssa, dssa->ssa_indx))
                    break;
                }

              if (Set_size (working_set) > 2)
                {
                  /* Perform transformations! */

                  int *oparray, numops, i, new_ptype;
                  L_Oper *uop;

                  /* Insert initializer prior to first op */

                  uop = L_create_new_op_using (Lop_CMP, op);
                  uop->src[0] = L_new_gen_int_operand (0);
                  uop->src[1] = L_new_gen_int_operand (0);
                  L_set_compare (uop, L_CTYPE_INT, Lcmp_COM_EQ);
                  uop->dest[0] = L_copy_operand (last_pred);
                  uop->dest[0]->ptype = L_PTYPE_UNCOND_T;
                  L_insert_oper_before (cb, op, uop);

                  new_inits = Set_add (new_inits, uop->id);

                  numops = Set_size (working_set);
                  oparray = Lcode_malloc (numops * sizeof (int));
                  Set_2array (working_set, oparray);

                  for (i = 0; i < numops; i++)
                    {
                      uop = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
                                                       oparray[i]);
                      switch (uop->dest[0]->ptype)
                        {
                        case L_PTYPE_UNCOND_T:
                          new_ptype = L_PTYPE_AND_T;
                          break;
                        case L_PTYPE_UNCOND_F:
                          new_ptype = L_PTYPE_AND_F;
                          break;
                        default:
                          L_punt ("L_form_and_chains: non-uncond dest");
                          return (-1);
                        }

		      uncond_def_ssa = Set_delete (uncond_def_ssa,
				  uop->dest[0]->value.pred.ssa->ssa_indx);

                      if (uop->dest[0] != last_pred)
                        {
                          L_delete_operand (uop->dest[0]);
                          uop->dest[0] = L_copy_operand (last_pred);
                        }

                      uop->dest[0]->ptype = new_ptype;

                      if (uop->pred[0])
                        {
                          L_delete_operand (uop->pred[0]);
                          uop->pred[0] = NULL;
                        }
                      count++;
                    }
                  Lcode_free (oparray);
                }

              Set_dispose (working_set);
              working_set = NULL;
            }
        }
    }

  Set_dispose (uncond_def_ssa);
  Set_dispose (gen_use_ssa);

  /* Migrate inits upward to allow better combination later */

  if (count)
    {
      L_Oper *next_op, *pop;
      L_Operand *dest, *pred;

      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
        for (op = cb->first_op; op != NULL; op = next_op)
          {
            next_op = op->next_op;

            if (!Set_in (new_inits, op->id))
              continue;
            dest = op->dest[0];
            pred = op->pred[0];

            for (pop = op->prev_op; pop; pop = pop->prev_op)
              {
                /* ignore side exits for now */
                if (L_is_src_operand (dest, pop) ||
                    L_is_dest_operand (dest, pop) ||
                    (pred && L_is_dest_operand (pred, pop)) ||
                    (pop->opc == Lop_PROLOGUE) || (pop->prev_op == NULL))
                  break;
              }

            if (pop && (pop != op->prev_op))
              {
                L_remove_oper (cb, op);
                L_insert_oper_after (cb, pop, op);
              }
          }
    }

  Set_dispose (new_inits);

  return count;
}

static int
L_remove_red_cmps (L_Func *fn)
{
  int count = 0;
  L_Cb *cb;
  L_Oper *op, *next_op;
  L_Operand *dest;
  PG_Pred_SSA *dssa;
  PG_Pred_Graph * pg;
  DdNode *dnode, *pnode;

  pg = PG_pred_graph;

  if (!pg || !pg->max_ssa_indx)
    return 0;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = next_op)
        {
	  next_op = op->next_op;
          if (!L_general_pred_comparison_opcode (op))
            continue;
          if (!(dest = op->dest[0]))
            continue;
	  if (!L_is_transparent_predicate_ptype(dest->ptype))
	    continue;
          if (!(dssa = dest->value.pred.ssa))
	    {
	      L_warn ("L_remove_red_guards: Missing dest SSA");
	      continue;
	    }

          dnode = dssa->node;
	  if (!dnode)
	    L_punt("L_remove_red_cmps: No node in SSA");

          pnode = PG_find_prev_dest_node(pg, dssa);

	  if (dnode == pnode)
	    {
#ifdef DEBUG_PRED_OPTI
	      L_warn ("PRO> Deleting pred def oper %d", op->id);
#endif	      
	      L_delete_oper(cb, op);
	      PG_setup_pred_graph (fn);
	      count++;
	    }
	}
    }

  return count;
}

#if 0
static int
L_replace_integer_net_w_cmp (L_Cb *cb, L_Oper *op, List deflist)
{
  int count = 0, cidx, eval, eval_t = 0, eval_f = 0, and_mode;
  L_Oper *def, *newpd;
  L_Attr *attr;
  ITintmax val[2];
  
  cidx = !L_is_int_constant (op->src[0]);

  val[cidx] = op->src[cidx]->value.i;

  List_start (deflist);
  while ((def = (L_Oper *)List_next (deflist)))
    {
      if (!L_is_int_constant (def->src[0]))
	L_punt ("L_replace_integer_net: bad def");

      val[!cidx] = def->src[0]->value.i;

      eval = L_evaluate_int_compare_with_sources (op, val[0], val[1]);

      if (eval)
	eval_t ++;
      else
	eval_f ++;
    }  

  and_mode = (eval_t > eval_f);

  newpd = L_create_new_op (Lop_CMP);
  L_set_compare (newpd, L_CTYPE_INT, and_mode ? Lcmp_COM_EQ : Lcmp_COM_NE);
  newpd->src[0] = L_new_gen_int_operand (0);
  newpd->src[1] = L_new_gen_int_operand (0);
  L_insert_oper_after (cb, newpd, (L_Oper *)List_first (deflist));
  attr = L_new_attr ("INTNET", 0);
  newpd->attr = L_concat_attr (newpd->attr, attr);

  List_start (deflist);
  while ((def = (L_Oper *)List_next (deflist)))
    {
      if (!L_is_int_constant (def->src[0]))
	L_punt ("L_replace_integer_net: bad def");

      val[!cidx] = def->src[0]->value.i;
      eval = L_evaluate_int_compare_with_sources (op, val[0], val[1]);

      if (eval ^ and_mode)
	{
	  newpd = L_create_new_op (Lop_CMP);
	  L_set_compare (newpd, L_CTYPE_INT, 
			 and_mode ? Lcmp_COM_NE : Lcmp_COM_EQ);
	  newpd->src[0] = L_new_gen_int_operand (0);
	  newpd->src[1] = L_new_gen_int_operand (0);

	  if (def->pred[0])
	    newpd->pred[0] = L_copy_operand (def->pred[0]);

	  L_insert_oper_after (cb, def, newpd);
	  attr = L_new_attr ("INTNET", 0);
	  newpd->attr = L_concat_attr (newpd->attr, attr);
	}
    }  


  return count;
}


static int
L_replace_integer_net_cb (L_Cb *cb)
{
  int count = 0, i;
  L_Oper *op, *pop;
  Set creg;
  List worklist = NULL;

  for (op = cb->first_op; op; op = op->next_op)
    {
      L_Operand *vopd, *pred;
      List deflist = NULL;

      if (!L_int_pred_comparison_opcode (op) ||
	  !(L_is_int_constant (op->src[0]) ||
	    L_is_int_constant (op->src[1])))
	continue;

      if (op->pred[0])
	continue;

      if (L_is_variable (op->src[0]))
	vopd = op->src[0];
      else if (L_is_variable (op->src[1]))
	vopd = op->src[1];
      else
	continue;

      for (pop = op->prev_op; pop; pop = pop->prev_op)
	{
	  if (L_is_dest_operand (vopd, pop))
	    deflist = List_insert_first (deflist, vopd);

	  if ((pop->pred[0] && L_same_operand (pop->pred[0], pred)) ||
	      L_is_dest_operand (pred, pop))
	    break;
	}

      if (!deflist)
	continue;

      if (!pop && L_collective_subsumption (deflist, op))
	{
	  /* deflist contains a list of integer constant assignments
	   * to the variable used in the predicate define.  The
	   * predicate define should be evaluable for each constant.
	   */

	  count += L_replace_integer_net_w_cmp (cb, op, deflist);
	}

      deflist = List_reset (deflist);
    }

  if (creg)
    Set_dispose (creg);
  if (worklist)
    List_reset (worklist);

  return count;
}

static int
L_replace_integer_net (L_Func *fn)
{
  int count = 0;
  L_Cb *cb;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
	continue;

      count += L_replace_integer_net_cb (cb);
    }

  return count;
}
#endif

/*
 * L_lightweight_pred_opti(L_Func *fn)
 * --------------------------------------------------------------------------
 * Perform some easy predicate define optimizations
 * Assumes pred defs are split and graph is up to date
 * JWS 20010117
 */

int
L_lightweight_pred_opti (L_Func * fn)
{
  int count = 0;
  L_Cb *cb;
  
  if (!PG_pred_graph)
    return 0;

  for (cb = fn->first_cb ; cb; cb = cb->next_cb)
    count += L_local_pred_cse (cb);

  if (Lopti_remove_red_guards)
    count += L_remove_red_guards (fn);

  count += L_remove_red_cmps (fn);

  if (count)
    PG_setup_pred_graph (fn);

  count += L_form_and_chains (fn);

  return count;
}
