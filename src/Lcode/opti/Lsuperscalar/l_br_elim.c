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
 *      File :          l_combine_br.c
 *      Description :   combine branches using OR-type predicate defines.
 *      Author :        Scott Mahlke
 *      Date :          August 1994
 *
 *      (C) Copyright 1994, Scott Mahlke
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define ERR     stderr

#undef  DEBUG_COMBINE

#define L_BYPASS_BR_COMBINE_ATTR                        "bypass_br_comb"

/*
 *      Parameters controlling this opti
 */
#define L_MIN_CB_WEIGHT_FOR_COMBINING                   25.0
#define L_MIN_NUM_BR_FOR_COMBINE                        3
#define L_MAX_INDIVIDUAL_BR_RATIO                       0.02    /* .05 */
#define L_MAX_OVERALL_BR_RATIO                          0.10    /* .45 */



/*
 *      Global vars
 */

static Set BR_full = NULL;
static Set *BR_partial = NULL;
static int num_BR_partial_sets = 0;
static Set ACC_op = NULL;

static int
L_available_predicates (L_Cb * cb)
{
  int used_preds, avail_preds;

  if (Lsuper_max_number_of_predicates < 0)
    return (-1);                /* -1 is infinity */

  used_preds = L_count_cb_predicates (cb);
  avail_preds = Lsuper_max_number_of_predicates - used_preds;
  if (avail_preds <= 0)
    return (0);
  else
    return (avail_preds);
}

static int
L_valid_for_combining (L_Cb * cb)
{
  int num_br, num_other;
  L_Oper *oper;

  /* Need atleast 1 available predicate to do branch combining */
  if (L_available_predicates (cb) == 0)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "CB %d not valid for combining, no free pred regs\n",
               cb->id);
#endif
      return (0);
    }

  num_br = 0;
  num_other = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        num_br++;
      else if (L_pred_define_opcode (oper) || L_general_store_opcode (oper))
        continue;
      else if (L_subroutine_call_opcode (oper) ||
               L_subroutine_return_opcode (oper))
        {
#ifdef DEBUG_COMBINE
          fprintf (ERR, "CB %d not valid for combining, contains jsr/rts\n",
                   cb->id);
#endif
          return (0);
        }
      /* Do not do spec ops if non_excepting_ops=no */
      else if (	!(L_non_excepting_ops) && L_is_pei(oper) && (!(L_is_trivially_safe(oper))))
        {
          return (0);
        }
      else
        num_other++;
    }

  if (num_br < L_MIN_NUM_BR_FOR_COMBINE)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "CB %d not valid for combining only %d brs\n",
               cb->id, num_br);
#endif
      return (0);
    }

  if (num_other == 0)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "CB %d not valid for combining only %d non_br ops\n",
               cb->id, num_other);
#endif
      return (0);
    }

  if (cb->weight < L_MIN_CB_WEIGHT_FOR_COMBINING)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR,
               "CB %d not valid for combining, weight is too small (%f)\n",
               cb->id, cb->weight);
#endif
      return (0);
    }

  /* check for bypass attribute */
  if (L_find_attr (cb->attr, L_BYPASS_BR_COMBINE_ATTR))
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR,
               "CB %d not valid for combining, bypass attribute present\n",
               cb->id);
#endif
      return (0);
    }

  return (1);
}

static void
L_oper_set_to_sorted_buf (L_Cb * cb, Set BR, int *br_buf)
{
  L_Oper *oper;
  int index = 0;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (Set_in (BR, oper->id))
        {
          br_buf[index++] = oper->id;
        }
    }
}

static Set
L_find_accumulator_ops (L_Cb * cb)
{
  int i, src2_allowed;
  Set ACC = NULL, ACC_ops = NULL, OTHER = NULL;
  L_Oper *oper;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_add_opcode (oper) ||
          L_sub_opcode (oper) ||
          L_mul_opcode (oper) ||
          (L_div_opcode (oper) && !L_int_div_opcode (oper)))
        {

          /* If sub or div, accumulator must be src1 */
          if (L_sub_opcode (oper) || L_div_opcode (oper))
            src2_allowed = 0;
          else
            src2_allowed = 1;

          /* Does it have the form of an accumulator? */
          if (L_is_register (oper->dest[0]) &&
              L_same_operand (oper->dest[0], oper->src[0]))
            {
              ACC = Set_add (ACC, L_REG_INDEX (oper->dest[0]->value.r));
              if (L_is_register (oper->src[1]))
                OTHER = Set_add (OTHER, L_REG_INDEX (oper->src[1]->value.r));
              if (L_is_predicated (oper) && L_is_register (oper->pred[0]))
                OTHER = Set_add (OTHER, L_REG_INDEX (oper->pred[0]->value.r));
              continue;
            }
          else if (src2_allowed &&
                   L_is_register (oper->dest[0]) &&
                   L_same_operand (oper->dest[0], oper->src[1]) &&
                   !L_same_operand (oper->src[0], oper->src[1]))
            {
              ACC = Set_add (ACC, L_REG_INDEX (oper->dest[0]->value.r));
              if (L_is_register (oper->src[0]))
                OTHER = Set_add (OTHER, L_REG_INDEX (oper->src[0]->value.r));
              if (L_is_predicated (oper) && L_is_register (oper->pred[0]))
                OTHER = Set_add (OTHER, L_REG_INDEX (oper->pred[0]->value.r));
              continue;
            }
        }

      /* Not an accumulator if reach this pt */
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_is_register (oper->dest[i]))
            OTHER = Set_add (OTHER, L_REG_INDEX (oper->dest[i]->value.r));
        }
      for (i = 0; i < L_max_src_operand; i++)
        {
          if (L_is_register (oper->src[i]))
            OTHER = Set_add (OTHER, L_REG_INDEX (oper->src[i]->value.r));
        }
      for (i = 0; i < 1 /* L_max_pred_operand */ ; i++)
        {
          if (L_is_register (oper->pred[i]))
            OTHER = Set_add (OTHER, L_REG_INDEX (oper->pred[i]->value.r));
        }
    }

  ACC = Set_subtract_acc (ACC, OTHER);
  (void) Set_dispose (OTHER);

  /* find accumulator ops */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_add_opcode (oper) || L_sub_opcode (oper) || L_mul_opcode (oper) ||
          (L_div_opcode (oper) && !L_int_div_opcode (oper)))
        {
          if (L_is_register (oper->dest[0]) &&
              Set_in (ACC, L_REG_INDEX (oper->dest[0]->value.r)))
            {
              ACC_ops = Set_add (ACC_ops, oper->id);
            }
        }
    }

#ifdef DEBUG_COMBINE
  Set_print (ERR, "Accumulator_vars", ACC);
  Set_print (ERR, "Accumulator_ops", ACC_ops);
#endif

  (void) Set_dispose (ACC);

  return (ACC_ops);
}

/*
 *      modified version of L_can_move_below() in l_opti_predicates.c
 */
static int
L_can_move_below_ignoring_stores_and_brs (L_Cb * cb, L_Oper * op,
                                          L_Oper * tomove_op)
{
  int i, j, is_load, is_store, fragile_macro;
  L_Oper *ptr;
  L_Operand *dest, *src;
  /* SAM, 7-97: note shoudl only be SET_NONLOOP_CARRIED, set inner
     carried to cover up a bug in sync arc generation */
  int dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);

  if ((op == NULL) | (tomove_op == NULL))
    L_punt
      ("L_can_move_below_ignoring_stores: op and tomove_op cannot be NULL");

  is_load = L_general_load_opcode (tomove_op);
  is_store = L_general_store_opcode (tomove_op);
  fragile_macro = L_has_fragile_macro_operand (tomove_op);

  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    {
      for (ptr = tomove_op->next_op; ptr != NULL; ptr = ptr->next_op)
        {
          /* stop search at after op is checked */
          if (ptr == op->next_op)
            break;
          /* Check for flow deps */
          for (i = 0; i < L_max_dest_operand; i++)
            {
              dest = tomove_op->dest[i];
              if (dest == NULL)
                continue;
              for (j = 0; j < L_max_src_operand; j++)
                {
                  if (L_same_operand (dest, ptr->src[j]) &&
                      PG_intersecting_predicates_ops (tomove_op, ptr))
                    return 0;
                }
              for (j = 0; j < 1; j++)
                {
                  if (L_same_operand (dest, ptr->pred[j]) &&
                      PG_intersecting_predicates_ops (tomove_op, ptr))
                    return 0;
                }
            }
          /* Check for anti deps */
          for (i = 0; i < L_max_src_operand; i++)
            {
              src = tomove_op->src[i];
              if (src == NULL)
                continue;
              for (j = 0; j < L_max_dest_operand; j++)
                {
                  if (L_same_operand (src, ptr->dest[j]) &&
                      PG_intersecting_predicates_ops (tomove_op, ptr))
                    return 0;
                }
            }
          /* Check for output deps */
          for (i = 0; i < L_max_dest_operand; i++)
            {
              dest = tomove_op->dest[i];
              if (dest == NULL)
                continue;
              for (j = 0; j < L_max_dest_operand; j++)
                {
                  if (L_same_operand (dest, ptr->dest[j]) &&
                      PG_intersecting_predicates_ops (tomove_op, ptr))
                    return 0;
                }
            }
          /* Check for memory deps */
          if ((is_store) && (L_general_load_opcode (ptr)) &&
              (!L_independent_memory_ops (cb, tomove_op, ptr, dep_flags)) &&
              (PG_intersecting_predicates_ops (tomove_op, ptr)))
            return 0;
          /* check for macro deps on jsr's */
          if ((fragile_macro) && (L_subroutine_call_opcode (ptr)) &&
              (PG_intersecting_predicates_ops (tomove_op, ptr)))
            return 0;
        }
    }
  else
    {
      for (ptr = tomove_op->next_op; ptr != NULL; ptr = ptr->next_op)
        {
          /* stop search at after op is checked */
          if (ptr == op->next_op)
            break;
          /* Check for flow deps */
          for (i = 0; i < L_max_dest_operand; i++)
            {
              dest = tomove_op->dest[i];
              if (dest == NULL)
                continue;
              for (j = 0; j < L_max_src_operand; j++)
                {
                  if (L_same_operand (dest, ptr->src[j]))
                    return 0;
                }
            }
          /* Check for anti deps */
          for (i = 0; i < L_max_src_operand; i++)
            {
              src = tomove_op->src[i];
              if (src == NULL)
                continue;
              for (j = 0; j < L_max_dest_operand; j++)
                {
                  if (L_same_operand (src, ptr->dest[j]))
                    return 0;
                }
            }
          /* Check for output deps */
          for (i = 0; i < L_max_dest_operand; i++)
            {
              dest = tomove_op->dest[i];
              if (dest == NULL)
                continue;
              for (j = 0; j < L_max_dest_operand; j++)
                {
                  if (L_same_operand (dest, ptr->dest[j]))
                    return 0;
                }
            }
          /* Check for memory deps */
          if ((is_store) && (L_general_load_opcode (ptr)) &&
              (!L_independent_memory_ops (cb, tomove_op, ptr, dep_flags)))
            return 0;
          /* check for macro deps on jsr's */
          if ((fragile_macro) && (L_general_subroutine_call_opcode (ptr)))
            return 0;
        }
    }

  if (ptr != op->next_op)
    L_punt ("L_can_move_below_ignoring_stores_and_brs: op not found");
  return 1;
}

static int
L_can_move_all_stores_below (L_Cb * cb, L_Oper * first_op, L_Oper * last_op)
{
  L_Oper *oper;

  for (oper = first_op; oper != last_op; oper = oper->next_op)
    {
      if (!L_general_store_opcode (oper))
        continue;
      if (!L_can_move_below_ignoring_stores_and_brs (cb, last_op, oper))
        return (0);
    }

  return (1);
}

/*
 * Unrolled loops are valid for full combining if:
 *      1. no jsrs in loop
 *      2. no jump_rg's in loop
 *      3. no predicated stores in loop
 *      4. nothing in live out of br defined after the br in the loop
 */

static void
L_find_loop_full_combine_set (L_Cb * cb)
{
  int i, num_br, *br_buf, tot_br, use_before_def, gen_sub_call, fmacros_fail;
  L_Oper *oper, *loopback_br = NULL, *lastexit_br = NULL, *first_op, *last_op;
  L_Flow *flow;
  double taken_ratio, tot_taken_weight, tot_taken_ratio;
  Set DEF, OUT;

  /* first find last br before the loop back br, this is where we stop
     all validity checking */
  for (oper = cb->last_op; oper != NULL; oper = oper->prev_op)
    {
      if (!(L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper)))
        continue;
      if (L_find_branch_dest (oper) == cb)
        {
          loopback_br = oper;
          break;
        }
    }
  if (loopback_br == NULL)
    {
      fprintf (stderr, "superblock loop (cb %d) has no backedge!\n", cb->id);
      return;
    }

  for (oper = loopback_br->prev_op; oper != NULL; oper = oper->prev_op)
    {
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          lastexit_br = oper;
          break;
        }
    }


  /* Accumulators will be moved down in the same manner stores are */
  ACC_op = NULL;
  ACC_op = L_find_accumulator_ops (cb);

  tot_br = 0;
  DEF = NULL;
  BR_full = NULL;
  use_before_def = 0;
  tot_taken_weight = 0.0;
  gen_sub_call = fmacros_fail = 0;
  for (oper = lastexit_br; oper != NULL; oper = oper->prev_op)
    {

      /* Since stores and branches will be copied to decode block, they
         cannot use anything in DEF set, else they will use later value
         then they should */
      if (L_uncond_branch_opcode (oper) ||
          L_cond_branch_opcode (oper) ||
          L_general_store_opcode (oper) || Set_in (ACC_op, oper->id))
        {
          for (i = 0; i < L_max_src_operand; i++)
            {
              if ((L_is_register (oper->src[i])) &&
                  (Set_in (DEF, L_REG_INDEX (oper->src[i]->value.r))))
                {
                  use_before_def = 1;
                  break;
                }
              else if ((L_is_macro (oper->src[i])) &&
                       (Set_in (DEF, L_MAC_INDEX (oper->src[i]->value.mac))))
                {
                  use_before_def = 1;
                  break;
                }
            }
        }

      /* nothing in DEF can be in live-out of BR */
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          tot_br++;
          flow = L_find_flow_for_branch (cb, oper);
          tot_taken_weight += flow->weight;
          if (cb->weight > 0.0)
            {
              taken_ratio = flow->weight / cb->weight;
              tot_taken_ratio = tot_taken_weight / cb->weight;
            }
          else
            {
              taken_ratio = 0.0;
              tot_taken_ratio = 0.0;
            }
          OUT = L_get_oper_OUT_set (cb, oper, TAKEN_PATH);

          /* Additional check, SAM 2-97, if gen_sub_call encountered, no
             fragile macros may be in OUT */
          if (gen_sub_call)
            {
              Set OUT_fmacros = L_unmap_fragile_macro_set (OUT);
              if (!Set_empty (OUT_fmacros))
                fmacros_fail = 1;
              OUT_fmacros = Set_dispose (OUT_fmacros);
            }

          if (Set_intersect_empty (OUT, DEF) &&
              (taken_ratio <= L_MAX_INDIVIDUAL_BR_RATIO) &&
              (tot_taken_ratio <= L_MAX_OVERALL_BR_RATIO) && (!fmacros_fail))
            {
              BR_full = Set_add (BR_full, oper->id);
            }
#ifdef DEBUG_COMBINE
          else
            {
              if (!Set_intersect_empty (OUT, DEF))
                {
                  fprintf (ERR, ">> OUT set of %d contains var in DEF set\n",
                           oper->id);
                  Set_print (ERR, "OUT", OUT);
                  Set_print (ERR, "DEF", DEF);
                }
              else if (taken_ratio > L_MAX_INDIVIDUAL_BR_RATIO)
                fprintf (ERR, ">> Taken ratio (%f) too high for op %d\n",
                         taken_ratio, oper->id);
              else if (tot_taken_ratio > L_MAX_OVERALL_BR_RATIO)
                fprintf (ERR,
                         ">> Overall Taken ratio (%f) too high at op %d\n",
                         tot_taken_ratio, oper->id);
              else
                fprintf (ERR, ">> OUT set of %d contains a fragile macro\n",
                         oper->id);
            }
#endif
        }

      /* add dests to DEF */
      else if (!Set_in (ACC_op, oper->id))
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_is_pred_register (oper->dest[i]))
                continue;
              else if (L_is_register (oper->dest[i]))
                DEF = Set_add (DEF, L_REG_INDEX (oper->dest[i]->value.r));
              else if (L_is_macro (oper->dest[i]))
                DEF = Set_add (DEF, L_MAC_INDEX (oper->dest[i]->value.mac));
            }
        }

      /* SAM 2-97, mark if general sub call encountered */
      if (L_general_subroutine_call_opcode (oper))
        {
          gen_sub_call = 1;
        }
    }

#ifdef DEBUG_COMBINE
  Set_print (ERR, "Candidate BR_full", BR_full);
  Set_print (ERR, "Candidate ACC_op", ACC_op);
  fprintf (ERR, "Sum of taken ratios: %f, max allowed %f\n",
           tot_taken_ratio, L_MAX_OVERALL_BR_RATIO);
#endif

  /* must have selected all branches for full combine, also total number
     must be larger than L_MIN_NUM_BR_FOR_COMBINE */
  if (Set_size (BR_full) < tot_br)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "All branches not eligible, no full combining!\n");
#endif
      BR_full = Set_dispose (BR_full);
      ACC_op = Set_dispose (ACC_op);
      return;
    }

  if (tot_br < L_MIN_NUM_BR_FOR_COMBINE)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "Too few branches eligible for full combining!\n");
#endif
      BR_full = Set_dispose (BR_full);
      ACC_op = Set_dispose (ACC_op);
      return;
    }

  /* get outta here if use of reg/macro before define by br or store */
  if (use_before_def)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "Use before define, no full combine possible\n");
#endif
      BR_full = Set_dispose (BR_full);
      ACC_op = Set_dispose (ACC_op);
      return;
    }

  num_br = Set_size (BR_full);
  br_buf = (int *) Lcode_malloc (sizeof (int) * num_br);
  if (num_br < 2)
    L_punt ("L_find_loop_full_combine_set: illegal number of brs");
  L_oper_set_to_sorted_buf (cb, BR_full, br_buf);
  first_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[0]);
  last_op =
    L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[num_br - 1]);
  Lcode_free (br_buf);

  /* must be able to move all stores below last_op */
  if (!L_can_move_all_stores_below (cb, first_op, last_op))
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "Cannot move all stores below last_op, no full comb!\n");
#endif
      BR_full = Set_dispose (BR_full);
      ACC_op = Set_dispose (ACC_op);
      return;
    }

}


static void
L_find_loop_partial_combine_sets (L_Cb * cb)
{
  int i, num_br, *br_buf, tot_br, use_before_def, avail_preds, gen_sub_call,
    fmacros_fail;
  L_Oper *oper, *loopback_br = NULL, *lastexit_br = NULL, *first_op, *last_op;
  L_Flow *flow;
  double taken_ratio, tot_taken_weight, tot_taken_ratio;
  Set DEF, OUT, old_DEF;

  /* first find last br before the loop back br, this is where we stop
     all validity checking */
  for (oper = cb->last_op; oper != NULL; oper = oper->prev_op)
    {
      if (!(L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper)))
        continue;
      if (L_find_branch_dest (oper) == cb)
        {
          loopback_br = oper;
          break;
        }
    }
  if (loopback_br == NULL)
    {
      fprintf (stderr, "superblock loop (cb %d) has no backedge!\n", cb->id);
      return;
    }
  for (oper = loopback_br->prev_op; oper != NULL; oper = oper->prev_op)
    {
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          lastexit_br = oper;
          break;
        }
    }

  tot_br = 0;
  DEF = NULL;
  num_BR_partial_sets = 1;
  use_before_def = 0;
  tot_taken_weight = 0.0;
  gen_sub_call = fmacros_fail = 0;
  for (oper = lastexit_br; oper != NULL; oper = oper->prev_op)
    {

      /* Since stores and branches will be copied to decode block, they
         cannot use anything in DEF set, else they will use later value
         then they should */
      if (L_uncond_branch_opcode (oper) ||
          L_cond_branch_opcode (oper) || L_general_store_opcode (oper))
        {
          for (i = 0; i < L_max_src_operand; i++)
            {
              if ((L_is_register (oper->src[i])) &&
                  (Set_in (DEF, L_REG_INDEX (oper->src[i]->value.r))))
                {
                  use_before_def = 1;
                  break;
                }
              else if ((L_is_macro (oper->src[i])) &&
                       (Set_in (DEF, L_MAC_INDEX (oper->src[i]->value.mac))))
                {
                  use_before_def = 1;
                  break;
                }
            }
        }
      if (use_before_def)
        {                       /* start a new partial set */
          use_before_def = 0;
          gen_sub_call = fmacros_fail = 0;
          num_BR_partial_sets++;
          if (DEF != NULL)
            {
              DEF = Set_dispose (DEF);
            }
        }

      /* nothing in DEF can be in live-out of BR */
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          tot_br++;
          flow = L_find_flow_for_branch (cb, oper);
          tot_taken_weight += flow->weight;
          if (cb->weight > 0.0)
            {
              taken_ratio = flow->weight / cb->weight;
              tot_taken_ratio = tot_taken_weight / cb->weight;
            }
          else
            {
              taken_ratio = 0.0;
              tot_taken_ratio = 0.0;
            }
          OUT = L_get_oper_OUT_set (cb, oper, TAKEN_PATH);

          /* Additional check, SAM 2-97, if gen_sub_call encountered, no
             fragile macros may be in OUT */
          if (gen_sub_call)
            {
              Set OUT_fmacros = L_unmap_fragile_macro_set (OUT);
              if (!Set_empty (OUT_fmacros))
                fmacros_fail = 1;
              OUT_fmacros = Set_dispose (OUT_fmacros);
            }

          if (Set_intersect_empty (OUT, DEF) &&
              (taken_ratio <= L_MAX_INDIVIDUAL_BR_RATIO) &&
              (tot_taken_ratio <= L_MAX_OVERALL_BR_RATIO) && (!fmacros_fail))
            {
              BR_partial[num_BR_partial_sets - 1] =
                Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
            }
          else
            {
              /* start new partial set */
              num_BR_partial_sets++;
              old_DEF = DEF;
              DEF = NULL;
              gen_sub_call = fmacros_fail = 0;
              if (!Set_intersect_empty (OUT, old_DEF))
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR,
                           ">> Starting new partial set: "
                           "OUT set of %d contains var in DEF set\n",
                           oper->id);
#endif
                  BR_partial[num_BR_partial_sets - 1] =
                    Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
                  tot_taken_weight = flow->weight;
                }
              else if ((tot_taken_ratio > L_MAX_OVERALL_BR_RATIO) &&
                       (taken_ratio <= L_MAX_INDIVIDUAL_BR_RATIO))
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR,
                           ">> Staring new partial set: "
                           "op %d, total tkn ratio too high %f\n",
                           oper->id, tot_taken_ratio);
#endif
                  BR_partial[num_BR_partial_sets - 1] =
                    Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
                  tot_taken_weight = flow->weight;
                }
              else if (taken_ratio > L_MAX_INDIVIDUAL_BR_RATIO)
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR,
                           ">>>> Taken ratio (%f) too high for oper %d\n",
                           taken_ratio, oper->id);
#endif
                  tot_taken_weight = 0.0;
                }
              else
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR, ">> OUT set of %d contains a fragile macro\n",
                           oper->id);
#endif
                  BR_partial[num_BR_partial_sets - 1] =
                    Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
                  tot_taken_weight = flow->weight;
                }
              (void) Set_dispose (old_DEF);
            }
        }

      /* add dests to DEF */
      else
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_is_register (oper->dest[i]))
                DEF = Set_add (DEF, L_REG_INDEX (oper->dest[i]->value.r));
              else if (L_is_macro (oper->dest[i]))
                DEF = Set_add (DEF, L_MAC_INDEX (oper->dest[i]->value.mac));
            }
        }

      /* SAM 2-97, mark if general sub call encountered */
      if (L_general_subroutine_call_opcode (oper))
        {
          gen_sub_call = 1;
        }
    }

#ifdef DEBUG_COMBINE
  fprintf (ERR, "\t[%d] partial candidate sets\n", num_BR_partial_sets);
  for (i = 0; i < num_BR_partial_sets; i++)
    {
      Set_print (ERR, "Candidate BR_partial", BR_partial[i]);
    }
#endif

  avail_preds = L_available_predicates (cb);
  for (i = 0; i < num_BR_partial_sets; i++)
    {

#ifdef DEBUG_COMBINE
      fprintf (ERR, "> Processing partial set %d...\n", i + 1);
#endif

      /* must have selected atleast L_MIN_NUM_BR_FOR_COMBINE to combine */
      if (Set_size (BR_partial[i]) < L_MIN_NUM_BR_FOR_COMBINE)
        {
#ifdef DEBUG_COMBINE
          fprintf (ERR, "Too few branches eligible for partial combining!\n");
#endif
          BR_partial[i] = Set_dispose (BR_partial[i]);
          continue;
        }

      num_br = Set_size (BR_partial[i]);
      if (num_br < 2)
        L_punt ("L_find_loop_partial_combine_sets: illegal number of brs");
      br_buf = (int *) Lcode_malloc (sizeof (int) * num_br);
      L_oper_set_to_sorted_buf (cb, BR_partial[i], br_buf);
      first_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[0]);
      last_op =
        L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[num_br - 1]);
      Lcode_free (br_buf);

      /* must be able to move all stores below last_op */
      if (!L_can_move_all_stores_below (cb, first_op, last_op))
        {
#ifdef DEBUG_COMBINE
          fprintf (ERR,
                   "Cannot move all stores below last_op, no partial comb!\n");
#endif
          BR_partial[i] = Set_dispose (BR_partial[i]);
          continue;
        }

      /* check if there is a predicate available to do this br combine with */
      if (avail_preds <= 0)
        {
#ifdef DEBUG_COMBINE
          fprintf (ERR, "Not enough predicates available!!\n");
#endif
          BR_partial[i] = Set_dispose (BR_partial[i]);
          continue;
        }

      avail_preds--;
    }
}


static void
L_find_nonloop_full_combine_set (L_Cb * cb)
{
  int i, num_br, *br_buf, tot_br, use_before_def, gen_sub_call, fmacros_fail;
  L_Oper *oper, *lastexit_br = NULL, *first_op, *last_op;
  L_Flow *flow;
  double taken_ratio, tot_taken_weight, tot_taken_ratio;
  Set DEF, OUT;

  last_op = cb->last_op;
  if (L_uncond_branch_opcode (last_op))
    last_op = last_op->prev_op;
  for (oper = last_op; oper != NULL; oper = oper->prev_op)
    {
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          lastexit_br = oper;
          break;
        }
    }

  tot_br = 0;
  DEF = NULL;
  use_before_def = 0;
  tot_taken_weight = 0.0;
  gen_sub_call = fmacros_fail = 0;
  for (oper = lastexit_br; oper != NULL; oper = oper->prev_op)
    {

      /* Since stores and branches will be copied to decode block, they
         cannot use anything in DEF set, else they will use later value
         then they should */
      if (L_uncond_branch_opcode (oper) ||
          L_cond_branch_opcode (oper) || L_general_store_opcode (oper))
        {
          for (i = 0; i < L_max_src_operand; i++)
            {
              if ((L_is_register (oper->src[i])) &&
                  (Set_in (DEF, L_REG_INDEX (oper->src[i]->value.r))))
                {
                  use_before_def = 1;
                  break;
                }
              else if ((L_is_macro (oper->src[i])) &&
                       (Set_in (DEF, L_MAC_INDEX (oper->src[i]->value.mac))))
                {
                  use_before_def = 1;
                  break;
                }
            }
        }

      /* nothing in DEF can be in live-out of BR */
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          tot_br++;
          flow = L_find_flow_for_branch (cb, oper);
          tot_taken_weight += flow->weight;
          if (cb->weight > 0.0)
            {
              taken_ratio = flow->weight / cb->weight;
              tot_taken_ratio = tot_taken_weight / cb->weight;
            }
          else
            {
              taken_ratio = 0.0;
              tot_taken_ratio = 0.0;
            }
          OUT = L_get_oper_OUT_set (cb, oper, TAKEN_PATH);

          /* Additional check, SAM 2-97, if gen_sub_call encountered, no
             fragile macros may be in OUT */
          if (gen_sub_call)
            {
              Set OUT_fmacros = L_unmap_fragile_macro_set (OUT);
              if (!Set_empty (OUT_fmacros))
                fmacros_fail = 1;
              OUT_fmacros = Set_dispose (OUT_fmacros);
            }

          if (Set_intersect_empty (OUT, DEF) &&
              (taken_ratio <= L_MAX_INDIVIDUAL_BR_RATIO) &&
              (tot_taken_ratio <= L_MAX_OVERALL_BR_RATIO) && (!fmacros_fail))
            {
              BR_full = Set_add (BR_full, oper->id);
            }
#ifdef DEBUG_COMBINE
          else
            {
              if (!Set_intersect_empty (OUT, DEF))
                fprintf (ERR, ">> OUT set of %d contains var in DEF set\n",
                         oper->id);
              else if (taken_ratio > L_MAX_INDIVIDUAL_BR_RATIO)
                fprintf (ERR, ">> Taken ratio (%f) too high for oper %d\n",
                         taken_ratio, oper->id);
              else if (tot_taken_ratio > L_MAX_OVERALL_BR_RATIO)
                fprintf (ERR,
                         ">> Overall Taken ratio (%f) too high at op %d\n",
                         tot_taken_ratio, oper->id);
              else
                fprintf (ERR, ">> OUT set of %d contains a fragile macro\n",
                         oper->id);
            }
#endif
        }

      /* add dests to DEF */
      else
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_is_register (oper->dest[i]))
                DEF = Set_add (DEF, L_REG_INDEX (oper->dest[i]->value.r));
              else if (L_is_macro (oper->dest[i]))
                DEF = Set_add (DEF, L_MAC_INDEX (oper->dest[i]->value.mac));
            }
        }

      /* SAM 2-97, mark if general sub call encountered */
      if (L_general_subroutine_call_opcode (oper))
        {
          gen_sub_call = 1;
        }
    }

  if (DEF)
    DEF = Set_dispose (DEF);

#ifdef DEBUG_COMBINE
  Set_print (ERR, "Candidate BR_full", BR_full);
  fprintf (ERR, "Sum of taken ratios: %f, max allowed %f\n",
           tot_taken_ratio, L_MAX_OVERALL_BR_RATIO);
#endif

  /* must have selected all branches for full combine, also total number
     must be larger than L_MIN_NUM_BR_FOR_COMBINE */
  if (Set_size (BR_full) < tot_br)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "All branches not eligible, no full combining!\n");
#endif
      BR_full = Set_dispose (BR_full);
      return;
    }

  if (tot_br < L_MIN_NUM_BR_FOR_COMBINE)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "Too few branches eligible for full combining!\n");
#endif
      BR_full = Set_dispose (BR_full);
      return;
    }

  /* get outta here if use of reg/macro before define by br or store */
  if (use_before_def)
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "Use before define, no full combine possible\n");
#endif
      BR_full = Set_dispose (BR_full);
      return;
    }

  num_br = Set_size (BR_full);
  br_buf = (int *) Lcode_malloc (sizeof (int) * num_br);
  if (num_br < 2)
    L_punt ("L_find_nonloop_full_combine_set: illegal number of brs");
  L_oper_set_to_sorted_buf (cb, BR_full, br_buf);
  first_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[0]);
  last_op =
    L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[num_br - 1]);
  Lcode_free (br_buf);

  /* must be able to move all stores below last_op */
  if (!L_can_move_all_stores_below (cb, first_op, last_op))
    {
#ifdef DEBUG_COMBINE
      fprintf (ERR, "Cannot move all stores below last_op, no full comb!\n");
#endif
      BR_full = Set_dispose (BR_full);
      return;
    }

}


static void
L_find_nonloop_partial_combine_sets (L_Cb * cb)
{
  int i, num_br, *br_buf, tot_br, use_before_def, avail_preds, gen_sub_call,
    fmacros_fail;
  L_Oper *oper, *lastexit_br = NULL, *first_op, *last_op;
  L_Flow *flow;
  double taken_ratio, tot_taken_weight, tot_taken_ratio;
  Set DEF, OUT, old_DEF;

  last_op = cb->last_op;
  if (L_uncond_branch_opcode (last_op))
    last_op = last_op->prev_op;
  for (oper = last_op; oper != NULL; oper = oper->prev_op)
    {
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          lastexit_br = oper;
          break;
        }
    }

  tot_br = 0;
  DEF = NULL;
  num_BR_partial_sets = 1;
  use_before_def = 0;
  tot_taken_weight = 0.0;
  gen_sub_call = fmacros_fail = 0;
  for (oper = lastexit_br; oper != NULL; oper = oper->prev_op)
    {
      /* Since stores and branches will be copied to decode block, they
         cannot use anything in DEF set, else they will use later value
         then they should */
      if (L_uncond_branch_opcode (oper) ||
          L_cond_branch_opcode (oper) || L_general_store_opcode (oper))
        {
          for (i = 0; i < L_max_src_operand; i++)
            {
              if ((L_is_register (oper->src[i])) &&
                  (Set_in (DEF, L_REG_INDEX (oper->src[i]->value.r))))
                {
                  use_before_def = 1;
                  break;
                }
              else if ((L_is_macro (oper->src[i])) &&
                       (Set_in (DEF, L_MAC_INDEX (oper->src[i]->value.mac))))
                {
                  use_before_def = 1;
                  break;
                }
            }
        }
      if (use_before_def)
        {                       /* start a new partial set */
          use_before_def = 0;
          gen_sub_call = fmacros_fail = 0;
          num_BR_partial_sets++;
          if (DEF != NULL)
            {
              DEF = Set_dispose (DEF);
            }
        }

      /* nothing in DEF can be in live-out of BR */
      if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          tot_br++;
          flow = L_find_flow_for_branch (cb, oper);
          tot_taken_weight += flow->weight;
          if (cb->weight > 0.0)
            {
              taken_ratio = flow->weight / cb->weight;
              tot_taken_ratio = tot_taken_weight / cb->weight;
            }
          else
            {
              taken_ratio = 0.0;
              tot_taken_ratio = 0.0;
            }
          OUT = L_get_oper_OUT_set (cb, oper, TAKEN_PATH);

          /* Additional check, SAM 2-97, if gen_sub_call encountered, no
             fragile macros may be in OUT */
          if (gen_sub_call)
            {
              Set OUT_fmacros = L_unmap_fragile_macro_set (OUT);
              if (!Set_empty (OUT_fmacros))
                fmacros_fail = 1;
              OUT_fmacros = Set_dispose (OUT_fmacros);
            }

          if (Set_intersect_empty (OUT, DEF) &&
              (taken_ratio <= L_MAX_INDIVIDUAL_BR_RATIO) &&
              (tot_taken_ratio <= L_MAX_OVERALL_BR_RATIO) && (!fmacros_fail))
            {
              BR_partial[num_BR_partial_sets - 1] =
                Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
            }
          else
            {
              /* start new partial set */
              num_BR_partial_sets++;
              old_DEF = DEF;
              DEF = NULL;
              gen_sub_call = fmacros_fail = 0;
              if (!Set_intersect_empty (OUT, old_DEF))
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR,
                           ">> Starting new partial set: "
                           "OUT set of %d contains var in DEF set\n",
                           oper->id);
#endif
                  BR_partial[num_BR_partial_sets - 1] =
                    Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
                  tot_taken_weight = flow->weight;
                }
              else if ((tot_taken_ratio > L_MAX_OVERALL_BR_RATIO) &&
                       (taken_ratio <= L_MAX_INDIVIDUAL_BR_RATIO))
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR,
                           ">> Staring new partial set "
                           "op %d, total tkn ratio too high %f\n",
                           oper->id, tot_taken_ratio);
#endif
                  BR_partial[num_BR_partial_sets - 1] =
                    Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
                  tot_taken_weight = flow->weight;
                }

              else if (taken_ratio > L_MAX_INDIVIDUAL_BR_RATIO)
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR, ">> Taken ratio (%f) too high for oper %d\n",
                           taken_ratio, oper->id);
#endif
                  tot_taken_weight = 0.0;
                }
              else
                {
#ifdef DEBUG_COMBINE
                  fprintf (ERR, ">> OUT set of %d contains a fragile macro\n",
                           oper->id);
#endif
                  BR_partial[num_BR_partial_sets - 1] =
                    Set_add (BR_partial[num_BR_partial_sets - 1], oper->id);
                  tot_taken_weight = flow->weight;
                }

              (void) Set_dispose (old_DEF);
            }
        }

      /* add dests to DEF */
      else
        {
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_is_register (oper->dest[i]))
                DEF = Set_add (DEF, L_REG_INDEX (oper->dest[i]->value.r));
              else if (L_is_macro (oper->dest[i]))
                DEF = Set_add (DEF, L_MAC_INDEX (oper->dest[i]->value.mac));
            }
        }

      /* SAM 2-97, mark if general sub call encountered */
      if (L_general_subroutine_call_opcode (oper))
        {
          gen_sub_call = 1;
        }

    }

  if (DEF)
    DEF = Set_dispose (DEF);

#ifdef DEBUG_COMBINE
  fprintf (ERR, "\t[%d] partial candidate sets\n", num_BR_partial_sets);
  for (i = 0; i < num_BR_partial_sets; i++)
    {
      Set_print (ERR, "Candidate BR_partial", BR_partial[i]);
    }
#endif

  avail_preds = L_available_predicates (cb);
  for (i = 0; i < num_BR_partial_sets; i++)
    {

#ifdef DEBUG_COMBINE
      fprintf (ERR, "> Processing partial set %d...\n", i + 1);
#endif

      /* must have selected atleast L_MIN_NUM_BR_FOR_COMBINE to combine */
      if (Set_size (BR_partial[i]) < L_MIN_NUM_BR_FOR_COMBINE)
        {
#ifdef DEBUG_COMBINE
          fprintf (ERR, "Too few branches eligible for partial combining!\n");
#endif
          BR_partial[i] = Set_dispose (BR_partial[i]);
          continue;
        }

      num_br = Set_size (BR_partial[i]);
      if (num_br < 2)
        L_punt ("L_find_nonloop_partial_combine_sets: illegal number of brs");
      br_buf = (int *) Lcode_malloc (sizeof (int) * num_br);
      L_oper_set_to_sorted_buf (cb, BR_partial[i], br_buf);
      first_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[0]);
      last_op =
        L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[num_br - 1]);
      Lcode_free (br_buf);

      /* must be able to move all stores below last_op */
      if (!L_can_move_all_stores_below (cb, first_op, last_op))
        {
#ifdef DEBUG_COMBINE
          fprintf (ERR,
                   "Cannot move all stores below last_op, no partial comb!\n");
#endif
          BR_partial[i] = Set_dispose (BR_partial[i]);
          continue;
        }

      /* check if there is a predicate available to do this br combine with */
      if (avail_preds <= 0)
        {
#ifdef DEBUG_COMBINE
          fprintf (ERR, "Not enough predicates available!!\n");
#endif
          BR_partial[i] = Set_dispose (BR_partial[i]);
          continue;
        }

      avail_preds--;
    }
}


static int
L_find_combine_sets (L_Cb * cb)
{
  int store_flag, pred_store_flag, jsr_flag, jrg_flag, live_flag, i,
    opti_flag;
  L_Oper *oper;

  /*
   *  Do not do branch_combining if there any unsafe jsrs or jrgs.
   */
  store_flag = 0;
  pred_store_flag = 0;
  jsr_flag = 0;
  jrg_flag = 0;
  live_flag = 0;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_general_store_opcode (oper))
        {
          if (L_is_predicated (oper))
            pred_store_flag = 1;
          store_flag = 1;
        }
      if (L_subroutine_call_opcode (oper) &&
          !L_side_effect_free_sub_call (oper))
        jsr_flag = 1;
      if (L_register_branch_opcode (oper))
        jrg_flag = 1;
    }

#ifdef DEBUG_COMBINE
  fprintf (ERR, "-------------------------------\n");
  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_UNROLLED))
    fprintf (ERR, "CB %d (LOOP)\n", cb->id);
  else
    fprintf (ERR, "CB %d (NON LOOP)\n", cb->id);
  fprintf (ERR, "\t Flags:");
  if (jsr_flag)
    fprintf (ERR, " JSR");
  if (jrg_flag)
    fprintf (ERR, " JRG");
  if (store_flag)
    fprintf (ERR, " STORE");
  if (pred_store_flag)
    fprintf (ERR, " PRED_STORE");
  fprintf (ERR, "\n");
#endif

  if (jsr_flag | jrg_flag)
    return (0);

  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_UNROLLED))
    {
      L_find_loop_full_combine_set (cb);
      if (BR_full == NULL)
        L_find_loop_partial_combine_sets (cb);
    }

  else
    {
      L_find_nonloop_full_combine_set (cb);
      if (BR_full == NULL)
        L_find_nonloop_partial_combine_sets (cb);
    }

#ifdef DEBUG_COMBINE
  if (BR_full != NULL)
    {
      Set_print (ERR, "\tBR_full", BR_full);
      Set_print (ERR, "\tACC_op", ACC_op);
      fprintf (ERR, "\tnum partial sets: %d\n", num_BR_partial_sets);
      for (i = 0; i < num_BR_partial_sets; i++)
        {
          Set_print (ERR, "\tBR_partial[i]", BR_partial[i]);
        }
    }
#endif

  opti_flag = 0;
  if (!Set_empty (BR_full))
    opti_flag = 1;
  for (i = 0; i < num_BR_partial_sets; i++)
    {
      if (!Set_empty (BR_partial[i]))
        opti_flag = 1;
    }

  return (opti_flag);
}

static int
L_and_pred_is_needed (L_Cb * cb, Set BR, L_Oper * first_op, L_Oper * last_op)
{
  L_Oper *oper;
  int and_flag;

  and_flag = 0;
  for (oper = first_op; oper != last_op; oper = oper->next_op)
    {
      if (L_general_store_opcode (oper))
        {
          and_flag = 1;
          if (L_is_predicated (oper))
            L_punt ("L_and_pred_is_needed: illegal pred store");
        }
      else if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
        {
          if (Set_in (BR, oper->id))
            continue;
          and_flag = 1;
          if (L_is_predicated (oper))
            L_punt ("L_and_pred_is_needed: illegal pred br");
        }
    }

  return (and_flag);
}

static void
L_convert_branch_to_OR_AND_pred_define (L_Flow * flow, L_Oper * oper,
                                        L_Operand * or_dest,
                                        L_Operand * and_dest)
{
  L_Attr *attr;

  if (oper == NULL)
    L_punt ("L_convert_branch_to_OR_AND_pred_define: oper is NULL");
  if (!L_is_pred_register (or_dest))
    L_punt ("L_convert_branch_to_OR_AND_pred_define: dest not correct type");
  if ((and_dest != NULL) && !L_is_pred_register (and_dest))
    L_punt ("L_convert_branch_to_OR_AND_pred_define: dest not correct type");

  if (L_uncond_branch_opcode (oper))
    {
      L_change_opcode (oper, Lop_CMP);
      L_delete_operand (oper->src[0]);
      oper->src[0] = L_new_gen_int_operand (0);
      L_set_compare (oper, oper->src[0]->ctype, Lcmp_COM_EQ);
      oper->src[1] = L_new_gen_int_operand (0);
      oper->dest[0] =
        L_new_register_operand (or_dest->value.r,
                                L_return_old_ctype (or_dest), L_PTYPE_OR_T);
      if (and_dest != NULL)
        oper->dest[1] = L_new_register_operand (and_dest->value.r,
                                                L_return_old_ctype (and_dest),
                                                L_PTYPE_AND_F);
      attr = L_new_attr ("PD0", 2);

      L_set_double_attr_field (attr, 0, oper->weight);
      L_set_double_attr_field (attr, 1, oper->weight);

      oper->attr = L_concat_attr (oper->attr, attr);
    }

  else
    {
      if (oper->opc == Lop_BR)
        L_change_opcode (oper, Lop_CMP);
      else
        L_change_opcode (oper, Lop_CMP_F);

      L_delete_operand (oper->src[2]);
      oper->src[2] = NULL;
      oper->dest[0] = L_new_register_operand (or_dest->value.r,
                                              L_return_old_ctype (or_dest),
                                              L_PTYPE_OR_T);
      if (and_dest != NULL)
        oper->dest[1] = L_new_register_operand (and_dest->value.r,
                                                L_return_old_ctype (and_dest),
                                                L_PTYPE_AND_F);
      /* Adding a pd0 attr for use with Dave's PRIC.  The attr has two
       * fields, the first is the number of times the define executes
       * and the second is the number of times the pred condition is true
       */
      attr = L_new_attr ("PD0", 2);

      L_set_double_attr_field (attr, 0, oper->weight);
      L_set_double_attr_field (attr, 1, flow->weight);

      oper->attr = L_concat_attr (oper->attr, attr);
    }
}

static void
L_move_all_stores_below_jump (L_Cb * cb, L_Oper * first_op,
                              L_Oper * last_op, L_Oper * jump_op)
{
  L_Oper *oper, *prev;

  for (oper = last_op; oper != first_op; oper = prev)
    {
      prev = oper->prev_op;
      if (!L_general_store_opcode (oper))
        continue;
      L_move_oper_after (cb, oper, jump_op);
    }
}

static void
L_move_all_checks_below_jump (L_Cb * cb, L_Oper * first_op,
                              L_Oper * last_op, L_Oper * jump_op)
{
  L_Oper *oper, *prev;

  for (oper = last_op; oper != first_op; oper = prev)
    {
      prev = oper->prev_op;
      if (!(oper->opc == Lop_CHECK))
        continue;

      L_move_oper_after (cb, oper, jump_op);
#if 0
      if (cb->first_op == oper)
        cb->first_op = oper->next_op;
      if (cb->last_op == oper)
        cb->last_op = oper->prev_op;

      if (oper->prev_op != NULL)
        oper->prev_op->next_op = oper->next_op;
      if (oper->next_op != NULL)
        oper->next_op->prev_op = oper->prev_op;

      oper->next_op = jump_op;
      oper->prev_op = jump_op->prev_op;

      if (oper->prev_op != NULL)
        oper->prev_op->next_op = oper;
      else
        cb->first_op = oper;

      if (oper->next_op != NULL)
        oper->next_op->prev_op = oper;
      else
        cb->last_op = oper;
#endif
    }
}

static void
L_move_all_ACC_op_below_jump (L_Cb * cb, L_Oper * first_op,
                              L_Oper * last_op, L_Oper * jump_op, Set ACC)
{
  L_Oper *oper, *prev;

  if (Set_empty (ACC))
    return;

  for (oper = last_op; oper != first_op; oper = prev)
    {
      prev = oper->prev_op;
      if (!Set_in (ACC, oper->id))
        continue;
      L_move_oper_after (cb, oper, jump_op);
    }
}

static void
L_insert_pred_clear (L_Cb * cb, L_Oper * pred_clear_op)
{
  L_Oper *ptr;

  if (cb != L_fn->first_cb)
    {
      L_insert_oper_before (cb, cb->first_op, pred_clear_op);
    }
  else
    {
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (L_is_opcode (Lop_DEFINE, ptr) ||
              L_is_opcode (Lop_PROLOGUE, ptr))
            continue;
          L_insert_oper_before (cb, ptr, pred_clear_op);
          break;
        }
    }
}

/*
 *      1. create a new cb
 *      2. copy all branches to be eliminated to new cb
 *      3. convert brs to predicate defines
 *      4. copy all stores to new cb
 */
static void
L_do_branch_combining (L_Cb * cb, Set BR, Set ACC)
{
  int num_br, *br_buf, pred_flag;
  L_Cb *new_cb;
  L_Oper *copy_op = NULL, *new_op, *oper, *first_op, *last_op;
  L_Flow *flow, *src_flow, *last_flow = NULL;
  L_Operand *br_or_pred;
  double sum;

#ifdef DEBUG_COMBINE
  fprintf (ERR, "Enter L_do_branch_combining cb %d\n", cb->id);
  Set_print (ERR, "BR", BR);
  Set_print (ERR, "ACC", ACC);
#endif

  num_br = Set_size (BR);
  if (num_br == 0)
    return;
  br_buf = (int *) Lcode_malloc (sizeof (int) * num_br);
  L_oper_set_to_sorted_buf (cb, BR, br_buf);

  first_op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[0]);
  last_op =
    L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, br_buf[num_br - 1]);
  if ((first_op == NULL) || (last_op == NULL))
    L_punt ("L_do_branch_combining: corrupt first or last op in br_buf");
  Lcode_free (br_buf);

  new_cb = L_create_cb (0.0);
  L_insert_cb_after (L_fn, L_fn->last_cb, new_cb);
#ifdef DEBUG_COMBINE
  fprintf (ERR, "$$$$$$$$$$$$ Creating cb %d\n", new_cb->id);
#endif

  br_or_pred = L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_PREDICATE,
                                       L_PTYPE_NULL);
  sum = 0.0;
  pred_flag = 0;
  for (oper = first_op; oper != NULL; oper = oper->next_op)
    {
      if (Set_in (BR, oper->id))
        {
          if (L_is_predicated (oper))
            pred_flag++;
          copy_op = L_copy_operation (oper);
          L_insert_oper_after (new_cb, new_cb->last_op, copy_op);
          flow = L_find_flow_for_branch (cb, oper);
          last_flow = flow->next_flow;
          sum += flow->weight;
          src_flow = L_find_matching_flow (flow->dst_cb->src_flow, flow);
          flow->src_cb = new_cb;
          src_flow->src_cb = new_cb;
          cb->dest_flow = L_remove_flow (cb->dest_flow, flow);
          new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, flow);
          L_convert_branch_to_OR_AND_pred_define (flow, oper, br_or_pred,
                                                  NULL);
        }
      else if (L_general_store_opcode (oper))
        {
          if (L_is_predicated (oper))
            pred_flag++;
          copy_op = L_copy_operation (oper);
          L_insert_oper_after (new_cb, new_cb->last_op, copy_op);
        }
      else if (Set_in (ACC, oper->id))
        {
          if (L_is_predicated (oper))
            pred_flag++;
          copy_op = L_copy_operation (oper);
          L_insert_oper_after (new_cb, new_cb->last_op, copy_op);
        }
      else if (L_check_opcode (oper))
        {
          if (L_is_predicated (oper))
            pred_flag++;
          copy_op = L_copy_operation (oper);
          L_insert_oper_after (new_cb, new_cb->last_op, copy_op);
        }

      /* no need to set M flag on stores, checks, or ACC ops, 
         since they will be moved below the predicated JUMP */
      if (!L_safe_for_speculation (oper) &&
          !L_general_store_opcode (oper) &&
          !L_check_opcode (oper) && !Set_in (ACC, oper->id))
        {
          if (!(L_EXTRACT_BIT_VAL (oper->flags, L_OPER_MASK_PE)))
            {
              if (L_generate_spec_checks)
                copy_op = L_insert_check_after (cb, oper, oper);

              /* Add the S & M flags to oper */
              L_mark_oper_speculative (oper);   

              if (L_generate_spec_checks)
                {
                  if (oper == last_op)
                    {
                      last_op = copy_op;
                    }
                  /*oper = copy_op;*/
                }
            }
        }

      if (oper == last_op)
        break;
    }

  if (L_is_predicated (new_cb->last_op))
    pred_flag--;

  /* update new_cb, weight = sum, convert last branch into unpredicated jump */
  if (pred_flag)
    new_cb->flags = L_SET_BIT_FLAG (new_cb->flags, L_CB_HYPERBLOCK);
  new_cb->weight = sum;
  oper = new_cb->last_op;
  if (L_cond_branch_opcode (oper))
    L_convert_to_jump (oper, L_copy_operand (oper->src[2]));
  if (oper->pred[0] != NULL)
    {
      L_delete_operand (oper->pred[0]);
      oper->pred[0] = NULL;
      oper->flags = L_CLR_BIT_FLAG (oper->flags, L_OPER_PROMOTED);
    }

  /* update cb: add clear of br_or_pred, add predicated jump to new_cb after
     last branch in BR, add corresponding flow arcs for this jump also */
  cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
  /* Added SAM 6-99, to catch case where the func had no HBs prior to
   * br combining
   */
  L_fn->flags = L_SET_BIT_FLAG (L_fn->flags, L_FUNC_HYPERBLOCK);
  new_op = L_create_new_op (Lop_PRED_CLEAR);
  new_op->dest[0] = br_or_pred;
  L_insert_pred_clear (cb, new_op);
  new_op = L_create_new_op (Lop_JUMP);
  new_op->src[0] = L_new_cb_operand (new_cb);
  new_op->pred[0] = L_copy_operand (br_or_pred);
  L_insert_oper_after (cb, last_op, new_op);
  flow = L_new_flow (1, cb, new_cb, sum);
  if (last_flow != NULL)
    cb->dest_flow = L_insert_flow_before (cb->dest_flow, last_flow, flow);
  else
    cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
  flow = L_new_flow (1, cb, new_cb, sum);
  new_cb->src_flow = L_concat_flow (new_cb->src_flow, flow);

  L_move_all_stores_below_jump (cb, first_op, last_op, new_op);
  L_move_all_checks_below_jump (cb, first_op, last_op, new_op);
  L_move_all_ACC_op_below_jump (cb, first_op, last_op, new_op, ACC);

#if 0
  /* associate AND-type predicates to stores and non-eliminated branches */
  if (and_flag)
    {
      for (oper = first_op; oper != last_op; oper = oper->next_op)
        {
          if (L_general_store_opcode (oper) ||
              L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
            {
              if (oper->pred[0] != NULL)
                L_punt ("L_do_branch_combining: op has illegal predicate");
              oper->pred[0] = L_copy_operand (br_and_pred);
            }
        }
    }
#endif

}


/*==========================================================================*/
/*
 *      Export function
 */
/*==========================================================================*/

void
Lsuper_combine_branches (L_Func * fn)
{
  L_Cb *cb, *last_cb;
  int i;

  if (fn->weight <= 0.0)
    return;

  BR_full = NULL;
  BR_partial = (Set *) Lcode_calloc (sizeof (Set), fn->n_oper);
  num_BR_partial_sets = 0;
  ACC_op = NULL;
  last_cb = fn->last_cb;

#ifdef DEBUG_COMBINE
  fprintf (ERR, "Enter L_combine_branches, fn = %s\n", fn->name);
#endif

  PG_setup_pred_graph (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

      /* Don't go beyond the original last_cb, since those are the decode
         blocks, they cannot be combined any further anyway */
      if (cb->prev_cb == last_cb)
        break;

      if (!L_valid_for_combining (cb))
        continue;
      if (!L_find_combine_sets (cb))
        continue;

      /* full branch combining */
      L_do_branch_combining (cb, BR_full, ACC_op);
      if (BR_full != NULL)
        BR_full = Set_dispose (BR_full);
      if (ACC_op != NULL)
        ACC_op = Set_dispose (ACC_op);

      /* partial combining */
      for (i = 0; i < num_BR_partial_sets; i++)
        {
          L_do_branch_combining (cb, BR_partial[i], NULL);
          if (BR_partial[i] != NULL)
            {
              BR_partial[i] = Set_dispose (BR_partial[i]);
            }
        }
      num_BR_partial_sets = 0;

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
        fn->flags = L_SET_BIT_FLAG (fn->flags, L_FUNC_HYPERBLOCK);
    }

  Lcode_free (BR_partial);
}

typedef struct {
  ITintmax low;
  ITintmax high;
  L_Cb *target;
  double weight;
} SwitchSegment;

void
Lsuper_switch_optimization(L_Func *fn)
{
  L_Cb *cb;
  L_Oper *op, *new_op;
  L_Oper *first_br, *last_br;
  L_Flow *first_flow = NULL, *new_flow;
  L_Operand *reg = NULL;
  List segments = NULL;
  SwitchSegment *new_segt, *indx_segt, *prev_segt;
  ITintmax val;
  int ctype = 0;

  L_ALLOC_POOL(SwitchSegment);

  L_INIT_ALLOC_POOL(SwitchSegment, 64);

  for (cb = fn->first_cb; cb != NULL; cb = cb ->next_cb)
    {
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  first_br = last_br = NULL;
	  
	  while (1)
	    {
	      if (!op)
		break;
	      if (!L_is_reg_const_cond_br(op))
		break;
	      if (!L_find_attr (op->attr, "SWITCH"))
		break;
	      if (op->com[1] != Lcmp_COM_EQ)
		break;
	      if (op->pred[0])
		break;

	      /* branch is part of a switch */
	      
	      if (!first_br)
		{
		  first_br = op;
		  first_flow = L_find_flow_for_branch(cb, first_br);
		  reg = L_pred_compare_get_reg_operand(first_br);
		  ctype = first_br->com[0];
		}
	      else if (!L_same_operand(reg,L_pred_compare_get_reg_operand(op)))
		{
		  break;
		}
	      
	      last_br = op;

	      val = L_pred_compare_get_int_const(op);
	      new_segt = L_ALLOC(SwitchSegment);
	      new_segt->low = new_segt->high = val;
	      new_segt->target = op->src[2]->value.cb;
	      new_segt->weight = (L_find_flow_for_branch(cb, op))->weight;
	      
	      List_start(segments);
	      while((indx_segt = List_next(segments)))
		{
		  if (indx_segt->low > val)
		    break;
		}
	      segments = List_insert_prev(segments, new_segt);

	      op = op->next_op;	      
	    }

	  if (first_br != last_br)
	    {
	      L_Oper *next_op;

	      /* Combine all adjacent ranges to same target */

	      List_start(segments);
	      prev_segt = (SwitchSegment *)List_next(segments);
	      while ((indx_segt = (SwitchSegment *)List_next(segments)))
		{
		  if ((prev_segt->target == indx_segt->target) &&
		      (prev_segt->high == (indx_segt->low - 1)))
		    {
		      prev_segt->high = indx_segt->high;
		      prev_segt->weight += indx_segt->weight;
		      segments = List_delete_current(segments);
		      L_FREE(SwitchSegment, indx_segt);
		    }
		  else
		    {
		      prev_segt = indx_segt;
		    }
		}

	      List_start(segments);
	      while ((indx_segt = (SwitchSegment *)List_next(segments)))
		{
		  if (indx_segt->low == indx_segt->high)
		    {
		      new_op = L_create_new_op(Lop_BR);
		      L_set_compare(new_op, ctype, Lcmp_COM_EQ);
		      new_op->src[0] = L_copy_operand(reg);
		      new_op->src[1] = L_new_gen_int_operand(indx_segt->low);
		      new_op->src[2] = L_new_cb_operand(indx_segt->target);
		      L_insert_oper_before(cb, first_br, new_op);
		    }
		  else
		    {
		      L_Operand *pred;
		      pred = L_new_register_operand(++fn->max_reg_id,
						    L_CTYPE_PREDICATE,
						    L_PTYPE_NULL);
		      new_op = L_create_new_op(Lop_CMP);
		      L_set_compare(new_op, ctype, Lcmp_COM_GE);
		      new_op->src[0] = L_copy_operand(reg);
		      new_op->src[1] = L_new_gen_int_operand(indx_segt->low);
		      new_op->dest[0] = L_copy_operand(pred);
		      new_op->dest[0]->ptype = L_PTYPE_UNCOND_T;
		      L_insert_oper_before(cb, first_br, new_op);

		      new_op = L_create_new_op(Lop_CMP);
		      L_set_compare(new_op, ctype, Lcmp_COM_LE);
		      new_op->src[0] = L_copy_operand(reg);
		      new_op->src[1] = L_new_gen_int_operand(indx_segt->high);
		      new_op->dest[0] = L_copy_operand(pred);
		      new_op->dest[0]->ptype = L_PTYPE_AND_T;
		      L_insert_oper_before(cb, first_br, new_op);

		      new_op = L_create_new_op(Lop_JUMP);
		      new_op->pred[0] = pred;
		      new_op->src[0] = L_new_cb_operand(indx_segt->target);
		      L_insert_oper_before(cb, first_br, new_op);
		    }
		  new_flow = L_new_flow(indx_segt->low, cb, 
					indx_segt->target, 
					indx_segt->weight);
		  cb->dest_flow = L_insert_flow_before(cb->dest_flow,
						       first_flow,
						       new_flow);
		}

	      op = first_br;
	      while (op)
		{
		  next_op = op->next_op;
		  L_delete_complete_oper(cb, op);
		  if (op == last_br)
		    break;
		  op = next_op;
		}
	    }

	  List_start(segments);
	  while ((indx_segt = List_next(segments)))
	    L_FREE(SwitchSegment, indx_segt);

	  segments = List_reset(segments);   
	  if (!op)
	    break;
	}
    }

  L_free_alloc_pool(L_alloc_pool_SwitchSegment);
  L_rebuild_src_flow(fn);
}
