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
 *      File :          l_critical.c
 *      Description :   superblock critical path reducing optimizations
 *      Author :        Scott Mahlke
 *      Date :          October 1991
 *      export fns :    L_critical_path_reduction(fn, output_dep_only)
 *
 *      Changes: upgraded to new Lcode Oct93, DMG
 *
 *      (C) Copyright 1991, Scott Mahlke
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define L_MIN_WEIGHT_FOR_CRIT_PATH_RED  100.0

#define DO_CRIT_PATH_RED

/*==========================================================================*/

static int
L_dest_is_used_in_cb (L_Oper * oper, L_Operand * dest)
{
  L_Oper *ptr;
  int i;

  if (!oper)
    L_punt ("L_dest_is_used_in_cb: op  is NULL");

  if (!L_is_variable (dest))
    return 0;

  for (ptr = oper->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!PG_intersecting_predicates_ops (oper, ptr))
	continue;

      for (i = 0; i < L_max_src_operand; i++)
	if (L_same_operand (dest, ptr->src[i]))
	  return 1;

      if (!PG_superset_predicate_ops (ptr, oper))
	continue;

      for (i = 0; i < L_max_dest_operand; i++)
	if (L_same_operand (dest, ptr->dest[i]))
	  return 0;
    }
  return 0;
}


static L_Oper *
L_prev_def_of_reg (int reg_id, L_Oper * oper)
{
  L_Operand *operand;
  L_Oper *prev;

  operand = L_new_register_operand (reg_id, Lopti_ctype_array[reg_id],
                                    L_PTYPE_NULL);
  prev = L_prev_def (operand, oper);
  L_delete_operand (operand);
  return (prev);
}


static int
L_are_flow_dependent (L_Oper * opA, L_Oper * opB)
{
  int i;
  L_Operand *dest;

  if (!PG_intersecting_predicates_ops (opA, opB))
    return (0);

  for (i = 0; i < L_max_dest_operand; i++)
    {
      if (!(dest = opA->dest[i]))
        continue;
      if (L_is_src_operand (dest, opB))
        return 1;
    }
  return 0;
}


/*
 * Lsuper_critical_path_reduction_cb
 * ----------------------------------------------------------------------
 * Remove anti-dependences by register renaming
 */

static void
Lsuper_critical_path_reduction_cb (L_Func * fn, L_Cb * cb)
{
  int i, j;
  L_Oper *opA, *opB, *nextB, *new_op;
  L_Operand *new_reg, *src, *dest;

  if (isnan (cb->weight) || 
      (cb->weight < L_MIN_WEIGHT_FOR_CRIT_PATH_RED) ||
      L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
    return;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      for (i = 0; i < L_max_src_operand; i++)
        {
          if (!(src = opA->src[i]) ||
	      L_is_dest_operand (src, opA))
            continue;

          for (opB = opA->next_op; opB; opB = nextB)
            {
              int is_ind_operand = 0;

              nextB = opB->next_op;

              if (L_general_move_opcode (opB) ||
		  L_pred_define_opcode (opB) ||
		  L_are_flow_dependent (opA, opB))
                continue;

              /* this is used to allow some renaming for ops with common
                 dest and src operands, previously not allowed */

              if (L_general_arithmetic_opcode (opB))
                {
                  for (j = 0; j < L_max_src_operand; j++)
                    {
                      if (!opB->src[j])
                        continue;
                      if (Lsuper_invariant_operand (cb, opB->src[j]))
                        {
                          is_ind_operand = 1;
                          break;
                        }
                    }
                }

	      for (j = 0; j < L_max_dest_operand; j++)
		{
		  if (!(dest = opB->dest[j]))
		    continue;
		  if (!L_is_register (dest))
		    continue;
		  if (!L_same_operand (src, dest))
		    continue;
		  if (is_ind_operand && L_is_src_operand (dest, opB) &&
		      L_in_oper_RIN_set (opB, opB, dest))
		    continue;
		  if (!L_dest_is_used_in_cb (opB, dest))
		    continue;
		      
		  /*
		   *  Replace pattern
		   */

		  if (Lsuper_debug_critical_path_red)
		    {
		      if (L_is_src_operand (dest, opB))
			fprintf (stderr,
				 "Crit path red (anti dep, same src/dest) "
				 "%d and %d (cb %d)\n",
				 opA->id, opB->id, cb->id);
		      else
			fprintf (stderr,
				 "Crit path red (anti dep) "
				 "%d and %d (cb %d)\n",
				 opA->id, opB->id, cb->id);
		    }

		  new_reg = L_new_register_operand (++(fn->max_reg_id),
						    L_return_old_ctype (dest),
						    L_PTYPE_NULL);

		  new_op = L_create_move (L_copy_operand (src), new_reg);
		  new_op->weight = opB->weight;

		  if (opB->pred[0])
		    new_op->pred[0] = L_copy_operand (opB->pred[0]);

		  L_insert_oper_after (cb, opB, new_op);

		  opB->dest[j] = L_copy_operand (new_reg);

		  L_rename_subsequent_uses (new_op->next_op, opB,
					    dest, new_reg);

		  L_delete_operand (dest);
		}
	    }
        }

      if (!L_cond_branch_opcode (opA))
	continue;

      for (opB = opA->next_op; opB; opB = nextB)
        {
	  int is_ind_operand = 0;

          nextB = opB->next_op;

          if (L_general_move_opcode (opB) ||
	      L_pred_define_opcode (opB))
	    continue;

	  /* this is used to allow some renaming for ops with common
	     dest and src operands, previously not allowed */

	  if (L_general_arithmetic_opcode (opB))
	    {
	      for (j = 0; j < L_max_src_operand; j++)
		{
		  if (!opB->src[j])
		    continue;
		  if (Lsuper_invariant_operand (cb, opB->src[j]))
		    {
		      is_ind_operand = 1;
		      break;
		    }
		}
	    }

          for (j = 0; j < L_max_dest_operand; j++)
            {
              if (!(dest = opB->dest[j]))
                continue;
              if (!L_is_register (dest))
                continue;
              /* dont wanna do induction vars here */
              if (is_ind_operand && L_is_src_operand (dest, opB) &&
		L_in_oper_RIN_set (opB, opB, dest))
                continue;
              if (!L_dest_is_used_in_cb (opB, dest))
                continue;

              /*
               *      Match pattern
               */

              if (!L_in_oper_OUT_set (cb, opA, dest, TAKEN_PATH))
                continue;

              /*
               *      Replace pattern
               */

              if (Lsuper_debug_critical_path_red)
		fprintf (stderr, "Crit path red (cnt dep) %d and %d (cb %d)\n",
			 opA->id, opB->id, cb->id);

              new_reg = L_new_register_operand (++(fn->max_reg_id),
                                                L_return_old_ctype (dest),
                                                L_PTYPE_NULL);

              new_op = L_create_move (L_copy_operand (dest), new_reg);
              new_op->weight = opB->weight;
              if (opB->pred[0])
		new_op->pred[0] = L_copy_operand (opB->pred[0]);

              opB->dest[j] = L_copy_operand (new_reg);
              L_insert_oper_after (cb, opB, new_op);

	      L_rename_subsequent_uses (new_op->next_op, opB,
					dest, new_reg);

              L_delete_operand (dest);
            }
        }
    }
  return;
}


/*==========================================================================*/
/*
 *      Special renaming for unrolled loops
 */
/*==========================================================================*/

/*
 *      DEF = certain define
 *      maybe_DEF = conditional define
 *      USE = use or live-out before certain define
 */

static Set
L_find_loop_def_set (L_Cb * cb)
{
  int i, num_out, *buf;
  ITintmax cur_iter;
  L_Oper *oper, *prev;
  L_Operand *dest, *src;
  L_Attr *attr;
  Set DEF = NULL, maybe_DEF = NULL, USE = NULL, tmp1 = NULL, tmp2 =
    NULL, OUT = NULL, OUT2 = NULL, iter_DEF = NULL, iter_USE =
    NULL, iter_maybe_DEF = NULL;

  buf = (int *) Lcode_malloc (sizeof (int) * L_fn->max_reg_id);

  cur_iter = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      /* update iteration counter, reset iter DEF and USE sets */
      attr = L_find_attr (oper->attr, "iter");
      if ((attr) && (attr->field[0]->value.i != cur_iter))
        {
          cur_iter = attr->field[0]->value.i;
          DEF = Set_union_acc (DEF, iter_DEF);
          maybe_DEF = Set_union_acc (maybe_DEF, iter_maybe_DEF);
          USE = Set_union_acc (USE, iter_USE);
          iter_DEF = Set_dispose (iter_DEF);
          iter_maybe_DEF = Set_dispose (iter_maybe_DEF);
          iter_USE = Set_dispose (iter_USE);
        }

      /* update iter_USE according to src operands */

      /* iter_USE: Used before defined in current iter */

      for (i = 0; i < L_max_src_operand; i++)
        {
          src = oper->src[i];
          if (!L_is_register (src))
            continue;
          if (Set_in (iter_DEF, src->value.r))
            continue;
          if (Set_in (iter_maybe_DEF, src->value.r))
            {
              if (!(prev = L_prev_def (src, oper)))
                continue;

              if (PG_superset_predicate_ops (prev, oper))
                continue;
            }
          iter_USE = Set_add (iter_USE, src->value.r);
        }

      /* update iter_USE according to live-out info */
      if (L_cond_branch_opcode (oper) || L_uncond_branch_opcode (oper))
        {
          OUT = L_get_oper_OUT_set (cb, oper, TAKEN_PATH);
          OUT2 = L_unmap_reg_set (OUT);
          OUT2 = Set_subtract_acc (OUT2, iter_DEF);
          if (Set_empty (OUT2))
            {
              Set_dispose (OUT2);
              continue;
            }
          tmp1 = Set_intersect (OUT2, iter_maybe_DEF);
          num_out = Set_size (tmp1);
          Set_2array (tmp1, buf);
          for (i = 0; i < num_out; i++)
            {
              if (!(prev = L_prev_def_of_reg (buf[i], oper)))
                continue;

              if (!PG_superset_predicate_ops (prev, oper))
                continue;
              OUT2 = Set_delete (OUT2, buf[i]);
            }
          num_out = Set_size (OUT2);
          Set_2array (OUT2, buf);
          for (i = 0; i < num_out; i++)
	    iter_USE = Set_add (iter_USE, buf[i]);

          Set_dispose (OUT2);
          Set_dispose (tmp1);
          continue;
        }

      /* update iter_DEF according to dest operands */
      for (i = 0; i < L_max_dest_operand; i++)
        {
          dest = oper->dest[i];
          if (!L_is_register (dest))
            continue;
          if (L_is_ctype_predicate (dest))
            continue;           /* dont rename predicate regs */
          if (L_is_src_operand (dest, oper))
            continue;           /* dont rename induction vars */
          if (Set_in (iter_DEF, dest->value.r))
            continue;
          if (L_is_predicated (oper))
            iter_maybe_DEF = Set_add (iter_maybe_DEF, dest->value.r);
          else
            iter_DEF = Set_add (iter_DEF, dest->value.r);
        }
    }

  /* update DEF, maybe_DEF, USE for last iteration */
  DEF = Set_union_acc (DEF, iter_DEF);
  maybe_DEF = Set_union_acc (maybe_DEF, iter_maybe_DEF);
  USE = Set_union_acc (USE, iter_USE);
  Set_dispose (iter_DEF);
  Set_dispose (iter_maybe_DEF);
  Set_dispose (iter_USE);

  /* return ((DEF+maybe_DEF)-USE) */

  tmp1 = Set_union (DEF, maybe_DEF);
  tmp2 = Set_subtract (tmp1, USE);
  Set_dispose (DEF);
  Set_dispose (maybe_DEF);
  Set_dispose (USE);
  Set_dispose (tmp1);
  Lcode_free (buf);

  return (tmp2);
}


/* find last instr in each unrolled iteration */
static L_Oper_List *
L_construct_first_oper_in_iter_list (L_Cb * cb)
{
  L_Oper *oper;
  L_Attr *attr;
  L_Oper_List *head, *new;
  ITintmax cur_iter;

  cur_iter = 0;
  head = NULL;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      attr = L_find_attr (oper->attr, "iter");
      if ((attr != NULL) && (attr->field[0]->value.i != cur_iter))
        {
          new = L_new_oper_list ();
          new->oper = oper;
          head = L_concat_oper_list (head, new);
          cur_iter = attr->field[0]->value.i;
        }
    }

  if (!head)
    L_punt ("L_construct_first_oper_in_iter_list: no list created!!");

  return (head);
}


static void
L_setup_unrolled_cb_for_migration (L_Cb * cb, int num_regs, int *regs,
                                   L_Oper_List * first_iter)
{
  L_Cb *dst_cb, *fallthru_cb;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  L_Oper *jump_op;
  L_Flow *new_flow, *src_flow;
#endif
  L_Oper *oper;
  L_Flow *flow;
  int insert_flag, i;

  if (!first_iter->next_list)
    L_punt ("L_setup_unrolled_cb_for_migration: no 2nd iteration of loop");

  for (oper = first_iter->next_list->oper; oper != NULL; oper = oper->next_op)
    {
      if (!(L_cond_branch_opcode (oper) || L_uncond_branch_opcode (oper)))
        continue;
      flow = L_find_flow_for_branch (cb, oper);
      dst_cb = flow->dst_cb;
      if (L_single_predecessor_cb (dst_cb))
        continue;
      insert_flag = 0;
      for (i = 0; i < num_regs; i++)
        {
          if (L_in_oper_OUT_set_reg (cb, oper, L_REG_INDEX (regs[i]),
				     TAKEN_PATH))
            {
              insert_flag = 1;
              break;
            }
        }
      if (!insert_flag)
        continue;

      /* need to create new cb to insert resets of live-out vars */

      L_split_arc (L_fn, cb, flow);
    }

  /* handle fallthru path if it exists */
  fallthru_cb = cb->next_cb;
  if (L_has_fallthru_to_next_cb (cb) &&
      !L_single_predecessor_cb (fallthru_cb))
    {
      insert_flag = 0;
      for (i = 0; i < num_regs; i++)
        {
          if (L_in_cb_IN_set_reg (fallthru_cb, L_REG_INDEX (regs[i])))
            {
              insert_flag = 1;
              break;
            }
        }
      if (insert_flag && !L_single_predecessor_cb (fallthru_cb))
        {
          flow = L_find_last_flow (cb->dest_flow);
	  L_split_arc (L_fn, cb, flow);
        }
    }
}


static int *
L_gen_remap_array (L_Func * fn, int num, int *input)
{
  int i, max, *output;

  /* find the max id */
  max = 0;
  for (i = 0; i < num; i++)
    {
      if (input[i] > max)
        max = input[i];
    }

  output = (int *) Lcode_calloc ((max + 1), sizeof (int));

  /* insert first mapping */
  for (i = 0; i < num; i++)
    output[input[i]] = ++fn->max_reg_id;

  return (output);
}


static void
L_update_remap_array (L_Func * fn, int num, int *input, int *output)
{
  int i;

  for (i = 0; i < num; i++)
    output[input[i]] = ++fn->max_reg_id;
}


static void
Lsuper_unrolled_loop_renaming_cb (L_Func * fn, L_Cb * cb)
{
  int i, num_regs, *regs, *renamed_regs, reg_id;
  Set DEF;
  L_Oper_List *first_iter, *next_iter;
  L_Oper *oper, *new_op;
  L_Operand *dest, *src;
  L_Cb *target_cb, *fallthru_cb;

  L_setup_ctype_array (fn);

  DEF = L_find_loop_def_set (cb);
  if (Set_empty (DEF))
    {
      L_reset_ctype_array ();
      Set_dispose (DEF);
      return;
    }

  first_iter = L_construct_first_oper_in_iter_list (cb);
  if ((first_iter == NULL) || (first_iter->next_list == NULL))
    {
      L_reset_ctype_array ();
      Set_dispose (DEF);
      if (first_iter != NULL)
        L_delete_all_oper_list (first_iter);
      return;
    }

  num_regs = Set_size (DEF);
  regs = (int *) Lcode_malloc (sizeof (int) * num_regs);
  Set_2array (DEF, regs);
  renamed_regs = L_gen_remap_array (fn, num_regs, regs);

  L_setup_unrolled_cb_for_migration (cb, num_regs, regs, first_iter);

  /* redo flow since we changed CFG */
  L_do_flow_analysis (L_fn, LIVE_VARIABLE);

  next_iter = first_iter->next_list;
  for (oper = next_iter->oper; oper != NULL; oper = oper->next_op)
    {
      /* update which iteration is being processed */
      if ((next_iter) && (oper == next_iter->oper))
        {
          next_iter = next_iter->next_list;
          L_update_remap_array (fn, num_regs, regs, renamed_regs);
        }

      /* take care of live out path */
      if (L_cond_branch_opcode (oper) || L_uncond_branch_opcode (oper))
        {
          for (i = 0; i < num_regs; i++)
            {
              reg_id = regs[i];
              if (!L_in_oper_OUT_set_reg (cb, oper, L_REG_INDEX (reg_id),
					  TAKEN_PATH))
                continue;
              dest = L_new_register_operand (reg_id,
                                             Lopti_ctype_array[reg_id],
                                             L_PTYPE_NULL);
              src = L_new_register_operand (renamed_regs[reg_id],
					    Lopti_ctype_array[reg_id],
					    L_PTYPE_NULL);
              new_op = L_create_move (dest, src);
              target_cb = L_find_branch_dest (oper);
              L_insert_oper_before (target_cb, target_cb->first_op, new_op);
            }
        }

      /* rename dest operands */
      for (i = 0; i < L_max_dest_operand; i++)
        {
          dest = oper->dest[i];
          if (!L_is_register (dest))
            continue;
          if (!Set_in (DEF, dest->value.r))
            continue;
          if (Lsuper_debug_critical_path_red)
            fprintf (stderr, "Unroll renaming: Rename dest of %d\n", oper->id);
          oper->dest[i] = L_new_register_operand (renamed_regs[dest->value.r],
                                                  L_return_old_ctype (dest),
                                                  dest->ptype);
          L_delete_operand (dest);
        }

      /* rename src operands */
      for (i = 0; i < L_max_src_operand; i++)
        {
          src = oper->src[i];
          if (!L_is_register (src))
            continue;
          if (!Set_in (DEF, src->value.r))
            continue;
          if (Lsuper_debug_critical_path_red)
            fprintf (stderr, "Unroll renaming: Rename src of %d\n", oper->id);
          oper->src[i] = L_new_register_operand (renamed_regs[src->value.r],
                                                 L_return_old_ctype (src),
                                                 src->ptype);
          L_delete_operand (src);
        }
      /* predicates renamed during unrolling, so dont rename here!! */
    }

  /* handle fallthru path if it exists */
  fallthru_cb = cb->next_cb;
  if (L_has_fallthru_to_next_cb (cb))
    {
      for (i = 0; i < num_regs; i++)
        {
          reg_id = regs[i];
          if (!L_in_cb_IN_set_reg (fallthru_cb, L_REG_INDEX (reg_id)))
            continue;
          dest = L_new_register_operand (reg_id,
                                         Lopti_ctype_array[reg_id],
                                         L_PTYPE_NULL);
          src = L_new_register_operand (renamed_regs[reg_id],
					Lopti_ctype_array[reg_id],
					L_PTYPE_NULL);
          new_op = L_create_move (dest, src);
          L_insert_oper_before (fallthru_cb, fallthru_cb->first_op, new_op);
        }
    }

  L_reset_ctype_array ();
  Set_dispose (DEF);
  Lcode_free (regs);
  Lcode_free (renamed_regs);
  L_delete_all_oper_list (first_iter);
  return;
}


/*==========================================================================*/
/*
 *      Note should not do copy propagation 2 from L_basic_code_optimize()
 *      after this, it will undo the effects!!!!
 */
/*==========================================================================*/

void
Lsuper_critical_path_reduction (L_Func * fn)
{
  L_Cb *cb;

  if (fn->weight <= 0.0 || fn->max_reg_id == 0)
    return;

  PG_setup_pred_graph (fn);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      /* REH 11/95 - Leave my boundary blocks alone!!! */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

      Lsuper_critical_path_reduction_cb (fn, cb);
    }
}


void
Lsuper_unrolled_loop_renaming (L_Func * fn)
{
  L_Cb *cb;

  if (fn->weight <= 0.0 || fn->max_reg_id == 0)
    return;

  PG_setup_pred_graph (fn);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_UNROLLED))
        continue;

      Lsuper_unrolled_loop_renaming_cb (fn, cb);
    }
}
