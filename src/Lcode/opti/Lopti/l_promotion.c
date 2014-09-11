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
 *      File :          l_promotion.c
 *      Description :   predicate promotion optimizations
 *      Creation Date : September 1993
 *      Authors :       Scott Mahlke, Jim Pierce, and David August
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"
#include <machine/lmdes_interface.h>
#include "l_promotion.h"

#define ERR     stderr

#undef DEBUG
#undef DEBUG_PROM1
#undef DEBUG_PROM2
#undef DEBUG_PROM3

void
L_set_pred_promotion_level (int level)
{
  if (level < 0 || level > 3)
    L_punt ("L_set_pred_promotion_level: 0 <= level <= 3");

  Lopti_pred_promotion_level = level;
  return;
}


/* Moves pred[0] to pred[1],
   sets promoted flag (P),
   moves input parm pred into pred[0] (can be NULL),

   pred[1] is lost */

static void
L_promote_oper (L_Cb *cb, L_Oper * oper, L_Operand * pred)
{
  L_Operand *old_pred;

  if (!oper)
    L_punt ("L_promote_oper: oper is NULL");

  if (!L_is_predicated (oper))
    {
      if (!pred)
        return;
      L_punt ("L_promote_oper: oper not predicated, pred not NULL");
    }

  if (L_max_pred_operand < 2)
    L_punt ("L_promote_oper: L_max_pred_operand is less than two");

  old_pred = oper->pred[0];

  oper->pred[0] = pred ? L_copy_operand (pred) : NULL;

  if (oper->pred[1])
    L_delete_operand (old_pred);
  else
    oper->pred[1] = old_pred;

  oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_PROMOTED);

  if (L_mask_potential_exceptions &&
      !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_MASK_PE) &&
      !L_is_trivially_safe (oper))
    {
      L_insert_check_after (cb, oper, oper);
      oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_MASK_PE);
    }

  return;
}


static L_Oper *
L_uncond_def_or_null (L_Operand * pred)
{
  L_Oper *def;
  PG_Pred_SSA *ssa;

  if (!pred || !L_is_ctype_predicate (pred))
    L_punt ("L_uncond_def_or_null: Not a predicate operand.");

  if (!pred->value.pred.ssa)
    L_punt ("L_uncond_def_or_null: no pred ssa.");

  ssa = pred->value.pred.ssa;

  if (ssa->oper && L_uncond_ptype (ssa->def_type))
    def = ssa->oper;
  else
    def = NULL;

  return def;
}


static int
L_def_reaches_all_subsequent_uses (L_Cb * cb, L_Oper * oper, 
				   L_Oper *poper, L_Operand * dest)
{
  L_Oper *cur_op;

  for (cur_op = oper->next_op; cur_op; cur_op = cur_op->next_op)
    {
      if (PG_superset_predicate_ops (oper, cur_op) ||
	  !PG_intersecting_predicates_ops (poper, cur_op))
	continue;

      /* cur_op interacts with the promoted op but did not
       * always interact with the original op 
       */

      if (L_is_src_operand (dest, cur_op))
	return 0;

      if (L_is_control_oper (cur_op) &&
	  L_in_oper_OUT_set (cb, cur_op, dest, TAKEN_PATH))
	return 0;
    }

  if (L_has_fallthru_to_next_cb (cb) && L_in_cb_IN_set (cb->next_cb, dest))
    return 0;

  return 1;
}


int
L_promotion_may_make_unsafe(L_Func *fn, L_Operand *pred, L_Oper *op)
{
  L_Oper *def_op = NULL;
  Set       defs = NULL;
  int *buffer;
  int num_defs, i, d;

  /* For safe oper, is any src fed by 
   * a predicated oper 
   */
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!op->src[i] || !L_is_variable (op->src[i]))
	continue;

      defs = L_get_oper_RIN_defining_opers (op, op->src[i]);

      num_defs = Set_size (defs);
      if (num_defs <= 0)
	{
	  printf("no defs for source op%d src%d - assuming promoted\n",
		 op->id,i);
	  continue;
	}

      buffer = (int *) Lcode_malloc (sizeof (int) * num_defs);
      Set_2array (defs, buffer);
      for (d = 0; d < num_defs; d++)
        {
	  if (!(def_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						    buffer[d])))
	    L_punt("L_promotion_may_make_unsafe: oper not found\n");
	   
	  if (def_op->pred[0])
	    {
	      printf("Suppressing promotion op%d\n",op->id);
	      return 1;
	    }
        }
      Lcode_free (buffer);
      defs = Set_dispose (defs);
    }
  
  return 0;
}


int
L_promotion_unprofitable (L_Oper *pop, L_Oper *op)
{
  int i;
  for (i = 0; i < L_max_src_operand; i++)
    {
      L_Operand *src;
      L_Oper *prv_def;

      if (!(src = op->src[i]))
	continue;
      
      if (!(prv_def = L_prev_def (src, op)))
	continue;

      if (!prv_def->pred[0])
	continue;

      if (!pop || !PG_superset_predicate_ops (prv_def, pop))
	return 1;
    }

  return 0;
}


int
L_oper_promotable (L_Oper *op)
{
  if (!L_non_excepting_ops && !L_safe_for_speculation (op))
    return (0);

  if (L_general_subroutine_call_opcode (op))
    return 0;

  /* Check the Mdes to make sure that if the op can except there is
   * a silent version of it */
  if (!op->mdes_info)
    {
      L_build_oper_mdes_info (op);

      if (op_flag_set (op->proc_opc, OP_FLAG_EXCEPT) &&
          !any_alt_flag_set (op->mdes_info, ALT_FLAG_SILENT))
        {
          L_free_oper_mdes_info (op);
          return (0);
        }

      L_free_oper_mdes_info (op);
    }
  else
    {
      if (op_flag_set (op->proc_opc, OP_FLAG_EXCEPT) &&
          !any_alt_flag_set (op->mdes_info, ALT_FLAG_SILENT))
	return (0);
    }

  /* JWS - prevent speculation of marked dangerous loads */
  if (L_load_opcode (op) && L_find_attr (op->attr, "NCSPEC"))
    return 0;

  return 1;
}


int
L_oper_promotable_without_check (L_Oper *oper)
{
  if (!L_oper_promotable (oper))
    return 0;
  else if (L_mask_potential_exceptions &&
      !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_MASK_PE) &&
      !L_is_trivially_safe (oper))
    return 0;
  
  return 1;
}


static int
L_safe_to_promote1 (L_Cb * cb, L_Oper * op, L_Oper * pop)
{
  /* check if op really can be speculated */

  if (!L_oper_promotable (op))
    return 0;

  /* Ensure that if the op is marked as a safe pei the promotion
   * won't render the op unsafe.
   */

  if (L_is_pei (op) && L_EXTRACT_BIT_VAL (op->flags,L_OPER_SAFE_PEI) &&
      L_promotion_may_make_unsafe (L_fn, pop->pred[0], op))
    return 0;

  /* JWS 20040203 Check to see if promotion will be profitable */

  if (L_promotion_unprofitable (pop, op))
    return 0;

  return 1;
}


/*
 * L_safe_to_promote2
 * ----------------------------------------------------------------------
 * Check that the operation is safe to promote without destination
 * renaming.  Make sure the destination to be promoted reaches
 * all uses that would receive the promoted destination.  Also check
 * for intersecting definitions between poper and oper.
 */
static int
L_safe_to_promote2 (L_Cb * cb, L_Oper * oper, L_Oper * poper)
{
  int i;
  L_Operand *dest;
  L_Oper *ptr;

  /* check live out of poper */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(dest = oper->dest[i]))
        continue;

      if (!L_is_variable (dest) || L_is_unsafe_macro (dest))
        return 0;

      if (!L_def_reaches_all_subsequent_uses (cb, oper, poper, dest))
        return 0;
    }

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(dest = oper->dest[i]))
        continue;

      for (ptr = oper->prev_op; ptr && ptr != poper; ptr = ptr->prev_op)
        {
          if (!L_is_dest_operand (dest, ptr))
            continue;
          if (PG_intersecting_predicates_ops (poper, ptr))
            return 0;
        }
    }

  return 1;
}


int
L_safe_to_promote (L_Cb * cb, L_Oper * oper, L_Oper * poper)
{
  if (!L_safe_to_promote1 (cb, oper, poper))
    return 0;

  if (!L_safe_to_promote2 (cb, oper, poper))
    return 0;
  
  return 1;
}


static int
L_all_dest_not_live_into_cb (L_Cb * cb, L_Oper * oper)
{
  int i;
  L_Operand *dest;

  for (i = 0; i < L_max_dest_operand; i++)
    {
      dest = oper->dest[i];
      if (dest == NULL)
        continue;
      if (L_is_macro (dest) || L_is_register (dest))
        {
          if (L_in_cb_IN_set (cb, dest))
            return (0);
        }
    }
  return (1);
}


static int
L_profitable_for_renaming (L_Oper * oper)
{
  L_Oper *ptr;
  L_Operand *dest;

  dest = oper->dest[0];
  if (dest == NULL)
    return 0;
  if (!L_is_variable (dest) ||
      L_is_unsafe_macro (dest) || L_is_fragile_macro (dest))
    return (0);
  if (L_is_src_operand (dest, oper) && L_general_arithmetic_opcode (oper))
    return 0;
  if (L_general_move_opcode (oper))
    return 0;

  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (oper, ptr))
        continue;
      if ((PG_superset_predicate_ops (oper, ptr) ||
           L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_PROMOTED)) &&
          L_is_src_operand (dest, ptr))
        return 1;
      if (L_is_dest_operand (dest, ptr))
        break;
    }
  return 0;
}


L_Oper *
L_find_pred_definition (L_Oper * oper)
{
  int i;
  L_Operand *pred, *dest;
  L_Oper *ptr;

  if (oper == NULL)
    L_punt ("L_find_pred_defintion: oper is NULL");
  if (!L_is_predicated (oper))
    L_punt ("L_find_pred_defintion: oper not predicated");

  pred = oper->pred[0];
  for (ptr = oper->prev_op; ptr != NULL; ptr = ptr->prev_op)
    {
      if (!L_general_pred_comparison_opcode (ptr))
        continue;
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (!(dest = ptr->dest[i]))
            continue;
          if (L_same_operand (pred, dest))
            return ptr;
        }
    }
  return NULL;
}


static int
L_kill_after_promotion (L_Cb *cb, L_Oper *op)
{
  int i, change = 0;
  L_Operand *src;
  L_Oper *pr_op;

  for (i = 0; i < L_max_src_operand; i++)
    {
      List deflist = NULL;

      if (!L_is_register ((src = op->src[i])) ||
	  !L_is_ctype_integer(src) ||
	  L_in_cb_IN_set (cb, src))
	continue;
      
      for (pr_op = cb->first_op; pr_op && (pr_op != op); 
	   pr_op = pr_op->next_op)
	{
	  if (L_is_dest_operand (src, pr_op))
	    {
	      deflist = List_insert_last (deflist, pr_op);
	      if (!pr_op->pred[0])
		break;
	    }
	}

      if (pr_op && List_size (deflist) && 
	  !PG_collective_subsumption (deflist, op))
	{
	  L_Attr *attr;
	  L_Oper *new_op;

	  new_op = L_create_new_op (Lop_MOV);
	  attr = L_new_attr ("PromoFix", 0);
	  new_op->attr = L_concat_attr (new_op->attr, attr);
	  new_op->dest[0] = L_copy_operand (src);
	  new_op->src[0] = L_new_gen_int_operand (0);
	  L_insert_oper_before (cb, (L_Oper *)List_first (deflist), new_op);
	  change++;
	}

      deflist = List_reset (deflist);
    }

  if (change)
    L_warn ("L_predicate_promotion: Fixed liveness in cb %d", cb->id);

  return change;
}


static void
L_predicate_promotion_cb (L_Cb * cb, int level)
{
  L_Oper *oper, *next, *def_op, *new_op, *ptr;  
  L_Operand *new, *old, *src;
  int operand, i;
  Set poly = NULL;

  if (level < 1)
    return;

  for (oper = cb->first_op; oper != NULL; oper = next)
    {
      next = oper->next_op;

      if (L_general_load_opcode (oper))
	{
	  if (L_find_attr (oper->attr, "polyacc"))
	    poly = Set_add (poly, L_REG_MAC_INDEX (oper->dest[0]));

	  if (poly &&
	      ((L_is_variable (oper->src[0]) && 
		Set_in (poly, L_REG_MAC_INDEX (oper->src[0]))) ||
	       (L_is_variable (oper->src[1]) && 
		Set_in (poly, L_REG_MAC_INDEX (oper->src[1])))))
	    {
	      L_Attr *attr;

	      if (!L_find_attr (oper->attr, "POLYSTUCK"))
		{
		  attr = L_new_attr ("POLYSTUCK", 0);
		  oper->attr = L_concat_attr (oper->attr, attr);
		}
	      continue;
	    }
	}

      if (oper->dest[0])
	{
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_is_variable (oper->src[i]) && 
		  Set_in (poly, L_REG_MAC_INDEX (oper->src[i])))
		{
		  poly = Set_add (poly, L_REG_MAC_INDEX (oper->dest[0]));
		  break;
		}
	    }
	}

      if (L_pred_define_opcode (oper) || !L_is_predicated (oper))
        continue;

      if (!L_is_variable (oper->dest[0]) ||
          L_is_unsafe_macro (oper->dest[0]) || 
          L_is_fragile_macro (oper->dest[0]))
        continue;

      /* Do not promote loads marked as dereferences of polymorphic 
       * (mixed ptr/int) variables 
       */

      if ((def_op = L_uncond_def_or_null (oper->pred[0])))
        {
	  /* LEVEL 1: Promote to guard of controlling predicate define */

	  /* Do not promote operations guarded by a "non-standard"
	   * predicate, such as that produced by a reciprocal approximation
	   */
	  if (!L_general_pred_comparison_opcode (def_op))
	    continue;
	  
          /* promotion level 1 */
          if (L_safe_to_promote (cb, oper, def_op))
            {
#ifdef DEBUG_PROM1
              if (L_is_predicated (def_op))
                fprintf (ERR, "Promote1 op %d from p%d to p%d\n", oper->id,
                         oper->pred[0]->value.r, def_op->pred[0]->value.r);
              else
                fprintf (ERR, "Promote1 op %d from p%d to NULL\n", oper->id,
                         oper->pred[0]->value.r);
#endif

              L_promote_oper (cb, oper, def_op->pred[0]);
	      L_kill_after_promotion (cb, oper);
              /* retry oper, to promote it as far as it will go */
              next = oper;
              continue;
            }
        }

      if (level < 2)
	continue;

      if (!L_safe_to_promote1 (cb, oper, cb->first_op))
	continue;

      if (L_safe_to_promote2 (cb, oper, cb->first_op) &&
	  L_all_dest_not_live_into_cb (cb, oper))
	{
	  /* LEVEL 2: Promote to TRUE predicate */

#ifdef DEBUG_PROM2
	  fprintf (ERR, "Promote2 op %d to NULL predicate\n", oper->id);
#endif
	  L_promote_oper (cb, oper, NULL);
	  L_kill_after_promotion (cb, oper);
	  continue;
	}

      if (level < 3)
	continue;

      if (L_profitable_for_renaming (oper))
	{
	  /* LEVEL 3: Promote to TRUE predicate by renaming destination */

#ifdef DEBUG_PROM3
	  fprintf (ERR, "Promote3 op %d to NULL predicate by renaming dest\n",
		   oper->id);
#endif
	  old = oper->dest[0];
	  new =
	    L_new_register_operand (++L_fn->max_reg_id,
				    L_return_old_ctype (old),
				    old->ptype);
	  switch (L_operand_case_ctype (old))
	    {
	    case L_CTYPE_INT:
	    case L_CTYPE_LLONG:
	    case L_CTYPE_POINTER:
	      new_op = L_create_new_op (Lop_MOV);
	      break;
	    case L_CTYPE_FLOAT:
	      new_op = L_create_new_op (Lop_MOV_F);
	      break;
	    case L_CTYPE_DOUBLE:
	      new_op = L_create_new_op (Lop_MOV_F2);
	      break;
	    default:
	      L_punt ("L_predicate_promotion_cb: illegal ctype");
	      return;
	    }
	  L_insert_oper_after (cb, oper, new_op);
	  oper->dest[0] = new;
	  new_op->dest[0] = old;
	  new_op->src[0] = L_copy_operand (new);
#ifdef DEBUG_PROM3
	  fprintf (ERR, "\tNew move %d : %d <- %d\n", new_op->id,
		   new_op->dest[0]->value.r, new_op->src[0]->value.r);
#endif
	  /* rename subsequent uses of dest in same cb */
	  for (ptr = new_op->next_op; ptr != NULL; ptr = ptr->next_op)
	    {
	      if (!PG_intersecting_predicates_ops (oper, ptr))
		continue;
	      if (PG_superset_predicate_ops (oper, ptr))
		{
		  for (operand = 0; operand < L_max_src_operand; operand++)
		    {
		      if (!(src = ptr->src[operand]))
			continue;
		      if (L_same_operand (src, old))
			{
#ifdef DEBUG_PROM3
			  fprintf (ERR, "\top %d rename src %d\n", ptr->id,
				   operand);
#endif
			  L_delete_operand (src);
			  ptr->src[operand] = L_copy_operand (new);
			}
		    }
		}
	      if (L_is_dest_operand (old, ptr))
		break;
	    }
	  new_op->pred[0] = L_copy_operand (oper->pred[0]);

	  if (L_is_promoted (oper))
	    {
	      new_op->pred[1] = L_copy_operand (oper->pred[1]);
	      L_mark_oper_promoted (new_op);
	    }

	  L_promote_oper (cb, oper, NULL);
	  L_kill_after_promotion (cb, oper);
	}
    }

  Set_dispose (poly);

  return;
}


/* requires LIVE_VARIABLE|REACHING_DEFINITION */

void
L_predicate_promotion (L_Func * fn, int only_simple)
{
  L_Cb *cb;
  int level;

  if (Lopti_pred_promotion_level < 1)
    return;

  level = only_simple ? 1 : Lopti_pred_promotion_level;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        continue;

      L_predicate_promotion_cb (cb, level);
    }

  return;
}

