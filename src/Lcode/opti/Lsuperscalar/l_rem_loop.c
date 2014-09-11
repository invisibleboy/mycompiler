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
/*****************************************************************************\
 *      File: l_rem_loop.c
 *      Description: Remove copies of loop back branch from unrolled loops
 *                   and create a separate loop to execute the remainder
 *                   iterations.
 *      Creation Date: March, 1995
 *      Author: Daniel Lavery
 *      Version: 0.0
 *
 *  Copyright (c) 1995 Daniel Lavery, Wen-mei Hwu, and
 *                The Board of Trustees of the University of Illinois.
 *                All rights reserved.
 *      The University of Illinois Software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_opti_functions.h>
#include "l_superscalar.h"

#define L_LOOP_REM_MIN_WEIGHT   500.0   /* minimum original (non-unrolled)
                                           weight for loop to be considered */
#define L_COST_FOR_REM_LOOP  3.0        /* overhead in instructions for 
                                           invoking unrolled loop with 
                                           remainder loop     */
#define L_MIN_PERCENT_OPS_REMOVED  2.0  /* minimum percentage of operations 
                                           removed from loop body to make 
                                           it worth creating remainder loop */
#define L_MAX_OPER_FOR_SMALL_LOOP  10   /* max number of operations in body for
                                           a loop to be considered small */
#define ERR stderr


/*****************************************************************************
   Returns 1 if branch 1 has the same src[0] and src[1] and the opposite
   branch opcode of branch 2.  The targets do not have to be the same.
   Used to determine if a branch in the body of the unrolled loop, is a
   (modified) copy of the loop back branch made during unrolling. 
*****************************************************************************/

static int
Lsuper_is_opposite_branch (L_Oper * branch1, L_Oper * branch2)
{
  if (!L_opposite_compare (branch1, branch2))
    return 0;

  if (!L_same_operand (branch1->src[0], branch2->src[0]))
    return 0;

  if (!L_same_operand (branch1->src[1], branch2->src[1]))
    return 0;

  return 1;
}

/********************************************************* 
  Return 1 if oper ind_op is an induction operation. 
***********************************************************/

static int
Lsuper_is_induction_op (L_Inner_Loop * loop, L_Oper * ind_op)
{
  L_Cb *cb;
  L_Operand *dest;
  L_Operand *incr_src;          /* induction variable increment */
  int invariant = 0;            /* flag indicating incr_src 
                                   is loop invariant */
  L_Oper *oper;
  int j;

  cb = loop->cb;
  dest = ind_op->dest[0];

  /* only check for add since Lcode converts induction subtracts to adds */
  if (!L_int_add_opcode (ind_op))
    return 0;

  if (!L_is_reg (dest))
    return 0;

  /*  check if a source == dest */
  if (L_same_operand (dest, ind_op->src[0]))
    {
      incr_src = ind_op->src[1];
    }
  else if (L_same_operand (dest, ind_op->src[1]))
    {
      incr_src = ind_op->src[0];
    }
  else
    {
      return 0;
    }

  /*  check if incr_src is loop invariant */
  if (L_is_constant (incr_src))
    {
      invariant = 1;
    }
  else
    {
      invariant = 1;
      /* check if written in loop body */
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          for (j = 0; j < L_max_dest_operand; j++)
            {
              if (L_same_operand (oper->dest[j], incr_src))
                invariant = 0;
            }
        }
    }
  if (invariant == 0)
    return 0;

  /* check if dest is only defined by ind_op or a copy of it */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      for (j = 0; j < L_max_dest_operand; j++)
        {
          if ((L_same_operand (oper->dest[j], dest)) &&
              (!L_same_operation (oper, ind_op, USE_FS)))
            {
              return 0;
            }
        }
    }

  return 1;
}

/************************************************************************
  Return loop counter increment operation if loop is a counted loop.
  Otherwise return 0.
*************************************************************************/

static L_Oper *
Lsuper_is_counted_loop (L_Inner_Loop * loop)
{
  int counted_loop = 0;         /* flag indicating loop is counted loop */
  L_Oper *loop_back_br;
  L_Oper *oper;
  L_Cb *cb;
  L_Operand *max_src;           /* Upper bound operand of loop back branch */
  int j;
  L_Oper *ptr;

  cb = loop->cb;
  loop_back_br = loop->feedback_op;

  /* search for induction operation whose destination is used by
     loop back branch */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (Lsuper_is_induction_op (loop, oper))
        {
          if (L_same_operand (oper->dest[0], loop_back_br->src[0]))
            {
              max_src = loop_back_br->src[1];
            }
          else if (L_same_operand (oper->dest[0], loop_back_br->src[1]))
            {
              max_src = loop_back_br->src[0];
            }
          else
            {
              continue;
            }

          /* check if max_src is loop invariant */
          if (L_is_constant (max_src))
            {
              counted_loop = 1;
              break;
            }
          else
            {
              counted_loop = 1;
              /* check if written in loop body */
              for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
                {
                  for (j = 0; j < L_max_dest_operand; j++)
                    {
                      if (L_same_operand (ptr->dest[j], max_src))
                        counted_loop = 0;
                    }
                }
              if (counted_loop == 1)
                {
                  break;
                }
            }
        }
    }

  if (counted_loop)
    {
      return (oper);
    }
  else
    {
      return 0;
    }
}

/************************************************************************
  If remainder loop creation is worth it for the expected
  benefit, return the number of times the loop is unrolled.
  Otherwise return 0.
*************************************************************************/

static int
Lsuper_remainder_loop_is_win (L_Inner_Loop * loop)
{
  L_Cb *cb;
  L_Attr *attr;
  int unroll;                   /* number of times loops was unrolled */
  double benefit;               /* dynamic number of branches removed per 
                                   invocation of the loop */
  int ops_removed;              /* static number of branches removed from
                                   the unrolled loop body */
  L_Oper *oper;
  int oper_count = 0;           /* total number of operations in loop body */
  double percent_ops_removed;   /* percentage of static operations removed 
                                   from the unrolled loop body */
  double min_weight;            /* minimum weight for unrolled loop body to
                                   do this optimization */

  cb = loop->cb;

  /* loop must be unrolled */
  attr = L_find_attr (cb->attr, "unroll_AMP");
  if (attr == NULL)
    return (0);
  unroll = ITicast (attr->field[0]->value.i);
  if (unroll < 2)
    return (0);

  /* can mark header cb with attribute to force remainder loop to be created */
  if (L_find_attr (cb->attr, "force_rem_loop"))
    {
      if (Lsuper_debug_loop_unroll)
        {
          fprintf (ERR, "force remainder loop for cb %d\n", cb->id);
        }
      return (unroll);
    }

  /* Number of branches removed must be twice the number of overhead operations
     per invocation of the loop. */
  benefit = loop->ave_iteration * (unroll - 1);
  if (benefit < 2 * L_COST_FOR_REM_LOOP)
    return (0);

  /* must remove a minimum percentage of operations */
  ops_removed = unroll - 1;
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      oper_count++;
    }
  percent_ops_removed =
    (((double) ops_removed) / ((double) oper_count)) * 100;
  if (percent_ops_removed < L_MIN_PERCENT_OPS_REMOVED)
    return (0);

  /* Must execute unrolled body a minimum number of times total.  Because
     of code expansion, requireed weight for large loops is higher. */
  min_weight = L_LOOP_REM_MIN_WEIGHT / ((double) unroll);
  if ((oper_count / unroll) > L_MAX_OPER_FOR_SMALL_LOOP &&
      loop->ave_iteration <= 25)
    {
      min_weight *= 5;
    }
  if (loop->weight < min_weight)
    return (0);

  return (unroll);
}

/* The optimization is not valid for unsigned branches because subtracting
   from the max for a low trip count loop could cause the new max to 
   be negative.  The optimization is not valid for BEQ and BNE because
   the exit condition only occurs for one value of the induction variable
   and subtracting from the max may bypass that point. Both these may be
   fixable with more complexity. */
static int
Lsuper_valid_branch_for_rem_loop (L_Oper * loop_back_br)
{
  if (L_unsigned_int_cond_branch_opcode (loop_back_br))
    return 0;
  if (L_gen_bne_branch_opcode (loop_back_br))
    return 0;
  if (L_gen_beq_branch_opcode (loop_back_br))
    return 0;

  return 1;
}

/*************************************************************************** 
  Create the remainder loop.     If original loop is of the form:

           Loop:      ~~~~~~~~~~
                      ~~~~~~~~~~
                      r100 = r100 + incr_reg
                      bgt r100, max_reg, Fallthru
                      ~~~~~~~~~~
                      ~~~~~~~~~~
                      r100 = r100 + incr_reg
                      ble r100, max_reg, Loop
           Fallthru: 


   Generate:

           Preheader: r200 = (unroll - 1) * incr_reg
                      new_max_reg = max_reg - r200
                      bgt r100, new_max_reg, Rem_loop
           Loop:      ~~~~~~~~~~
                      ~~~~~~~~~~
                      r100 = r100 + incr_reg
                      ~~~~~~~~~~
                      ~~~~~~~~~~
                      r100 = r100 + incr_reg
                      ble r100, new_max_reg, Loop
           Epilogue:  bgt r100, max_reg, Fallthru
           Rem_loop:  ~~~~~~~~~~
                      ~~~~~~~~~~
                      r100 = r100 + incr_reg
                      bgt r100, max_reg, Fallthru
                      ~~~~~~~~~~
                      ~~~~~~~~~~
                      r100 = r100 + incr_reg
                      ble r100, max_reg, Rem_loop
           Fallthru:


   Note: Original loop back branch must be a signed compare and branch,
         because new_max could be negative.
******************************************************************************/

static void
Lsuper_make_remainder_loop (L_Func * fn, L_Inner_Loop * loop,
                            L_Oper * loop_incr_op, int unroll)
{
  L_Cb *preheader_cb;
  L_Cb *header_cb;              /* original loop cb */
  L_Cb *fall_thru_cb;
  L_Cb *jump_cb;
  L_Cb *epilogue_cb;
  L_Cb *remainder_cb;
  L_Cb *dest_cb;

  L_Flow *fall_thru_flow;       /* last dest flow from header cb */
  L_Flow *flow;
  L_Flow *new_flow;
  L_Flow *header_flow;          /* flow from header cb */
  L_Flow *rem_flow;             /* flow from remainder loop */
  L_Flow *start_flow = NULL;

  L_Operand *ind_var_src;       /* loop counter */
  L_Operand *ind_incr_src;      /* loop counter increment */
  L_Operand *max_src;           /* loop bound */

  L_Oper *loop_back_br, *oper, *next_oper, *header_oper, *rem_oper, *branch;
  L_Oper *new_oper1, *new_oper2 = NULL, *new_br;

  L_Attr *attr;

  double weight;
  double scale;                 /* scale factor for remainder loop flows */

  int max_pos;                  /* index of source which is the loop
                                   bound in the loop back branch */

  preheader_cb = loop->preheader;
  header_cb = loop->cb;
  loop_back_br = loop->feedback_op;
  fall_thru_cb = loop->fall_thru;
  fall_thru_flow = L_find_last_flow (header_cb->dest_flow);

  attr = L_new_attr ("has_rem_loop", 0);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);

  if (Lsuper_debug_loop_unroll)
    {
      fprintf (ERR, "creating remainder loop for loop (cb %d)\n",
               header_cb->id);
    }

  /* Remove all src flows.  Will fix up all the dest flows and then rebuild
     all the src flows. */
  L_clear_src_flow (fn);

  /* If there is an unconditional jump at the end of the header, put it
     in its own cb  */
  if (L_uncond_branch_opcode (header_cb->last_op))
    {
      weight = fall_thru_flow->weight;
      jump_cb = L_create_cb (weight);
      L_insert_cb_after (fn, header_cb, jump_cb);
      L_change_flow (fall_thru_flow, fall_thru_flow->cc, header_cb,
                     jump_cb, weight);
      new_flow = L_new_flow (0, jump_cb, fall_thru_cb, weight);
      jump_cb->dest_flow = L_concat_flow (jump_cb->dest_flow, new_flow);
      fall_thru_cb = loop->fall_thru = jump_cb;
      L_move_op_to_start_of_block (header_cb, jump_cb, header_cb->last_op);
    }

  /* Create epilogue cb.  Its weight is equal to the sum of the weights
     of all the flows related to the original loop back branch */
  weight = 0.0;
  for (flow = header_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      if (flow == fall_thru_flow)
        {
          weight += flow->weight;
        }
      else if (flow->dst_cb == fall_thru_cb)
        {
          branch = L_find_branch_for_flow (header_cb, flow);
          if (Lsuper_is_opposite_branch (branch, loop_back_br))
            {
              weight += flow->weight;
            }
        }
    }
  epilogue_cb = L_create_cb (weight);
  L_insert_cb_after (fn, header_cb, epilogue_cb);
  attr = L_new_attr ("rem_epilogue", 1);
  L_set_int_attr_field (attr, 0, header_cb->id);
  epilogue_cb->attr = L_concat_attr (epilogue_cb->attr, attr);

  /* create remainder loop */
  /* for all the cases where control exited at the bottom of the unrolled
     loop, there will be no remainder iterations. */
  weight = epilogue_cb->weight - fall_thru_flow->weight;
  remainder_cb = L_create_cb (weight);
  L_copy_block_contents (header_cb, remainder_cb);
  remainder_cb->dest_flow = L_copy_flow (header_cb->dest_flow);
  remainder_cb->attr = L_copy_attr (header_cb->attr);
  remainder_cb->flags = header_cb->flags;
  /* don't want to software pipeline remainder loop */
  remainder_cb->flags = L_CLR_BIT_FLAG (remainder_cb->flags, L_CB_SOFTPIPE);
  attr = L_find_attr (remainder_cb->attr, "L_CB_SOFTPIPE");
  if (attr != NULL)
    {
      remainder_cb->attr = L_delete_attr (remainder_cb->attr, attr);
    }
  L_insert_cb_after (fn, epilogue_cb, remainder_cb);


  attr = L_find_attr (remainder_cb->attr, "has_rem_loop");
  remainder_cb->attr = L_delete_attr (remainder_cb->attr, attr);
  attr = L_new_attr ("rem_loop", 1);
  L_set_int_attr_field (attr, 0, header_cb->id);
  remainder_cb->attr = L_concat_attr (remainder_cb->attr, attr);

  /* add flows */

  /* preheader_flows */
  /* assume there are always enough iterations to enter the header cb */
  new_flow = L_new_flow (1, preheader_cb, remainder_cb, 0.0);
  flow = L_find_flow_with_dst_cb (preheader_cb->dest_flow, header_cb);
  preheader_cb->dest_flow = L_insert_flow_before (preheader_cb->dest_flow,
                                                  flow, new_flow);

  /* epilogue_cb flows */
  weight = epilogue_cb->weight - remainder_cb->weight;
  new_flow = L_new_flow (1, epilogue_cb, fall_thru_cb, weight);
  epilogue_cb->dest_flow = L_concat_flow (epilogue_cb->dest_flow, new_flow);
  new_flow = L_new_flow (0, epilogue_cb, remainder_cb, remainder_cb->weight);
  epilogue_cb->dest_flow = L_concat_flow (epilogue_cb->dest_flow, new_flow);

  /* remainder_cb flows and branches */
  scale = remainder_cb->weight / header_cb->weight;
  L_scale_flow_weights (remainder_cb->dest_flow, scale);
  for (oper = remainder_cb->last_op; oper != NULL; oper = oper->prev_op)
    {
      if (L_int_cond_branch_opcode (oper) &&
          Lsuper_is_opposite_branch (oper, loop_back_br))
        {
          start_flow = L_find_flow_for_branch (remainder_cb, oper);
          break;
        }
    }
  /* last copy of original loop body in remainder loop is newer executed */
  for (flow = start_flow; flow != NULL; flow = flow->next_flow)
    {
      flow->weight = 0.0;
    }
  /* since copied flows from header cb, need to change all src cbs */
  for (flow = remainder_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      dest_cb = flow->dst_cb;
      if (dest_cb == header_cb)
        {
          dest_cb = remainder_cb;
          remainder_cb->last_op->src[2]->value.cb = remainder_cb;
        }
      L_change_flow (flow, flow->cc, remainder_cb, dest_cb, flow->weight);
    }

  /* header and remainder loop flows, remove branches */
  /* set weights for flows associated with copies of the original loop
     back branch */
  rem_oper = remainder_cb->first_op;
  header_oper = header_cb->first_op;
  while (header_oper != NULL)
    {
      next_oper = header_oper->next_op;
      if (L_int_cond_branch_opcode (header_oper) &&
          Lsuper_is_opposite_branch (header_oper, loop_back_br))
        {
          rem_flow = L_find_flow_for_branch (remainder_cb, rem_oper);
          header_flow = L_find_flow_for_branch (header_cb, header_oper);
          rem_flow->weight = header_flow->weight;
          header_cb->dest_flow = L_delete_flow (header_cb->dest_flow,
                                                header_flow);
          L_delete_oper (header_cb, header_oper);
        }
      header_oper = next_oper;
      rem_oper = rem_oper->next_op;
    }
  L_change_flow (fall_thru_flow, 0, header_cb, epilogue_cb,
                 epilogue_cb->weight);

  /* find induction variable, induction variable increment, and loop bound */
  if (L_same_operand (loop_incr_op->dest[0], loop_incr_op->src[0]))
    {
      ind_var_src = loop_incr_op->src[0];
      ind_incr_src = loop_incr_op->src[1];
    }
  else
    {
      ind_var_src = loop_incr_op->src[1];
      ind_incr_src = loop_incr_op->src[0];
    }

  if (L_same_operand (loop_back_br->src[0], ind_var_src))
    {
      max_src = loop_back_br->src[1];
      max_pos = 1;
    }
  else
    {
      max_src = loop_back_br->src[0];
      max_pos = 0;
    }

  /* add instructions to preheader */
  if ((L_is_reg (ind_incr_src)) || (L_is_macro (ind_incr_src)))
    {
      /* multiply loop increment by unroll - 1 */
      new_oper1 = L_create_new_op (Lop_MUL);
      new_oper1->src[0] = L_new_gen_int_operand (unroll - 1);
      new_oper1->src[1] = L_copy_operand (ind_incr_src);
      new_oper1->dest[0] = L_new_register_operand (++(fn->max_reg_id),
                                                   L_native_machine_ctype,
                                                   L_PTYPE_NULL);
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_oper1);
      /* subtract from max */
      new_oper2 = L_create_new_op (Lop_SUB);
      new_oper2->src[0] = L_copy_operand (max_src);
      new_oper2->src[1] = L_copy_operand (new_oper1->dest[0]);
      new_oper2->dest[0] = L_new_register_operand (++(fn->max_reg_id),
                                                   L_native_machine_ctype,
                                                   L_PTYPE_NULL);
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_oper2);
    }
  else if (L_is_int_constant (ind_incr_src))
    {
      /* if induction increment is constant, 
         the multiplication is not needed */
      new_oper2 = L_create_new_op (Lop_SUB);
      new_oper2->src[0] = L_copy_operand (max_src);
      new_oper2->src[1] =
        L_new_gen_int_operand ((unroll - 1) * ind_incr_src->value.i);
      new_oper2->dest[0] = L_new_register_operand (++(fn->max_reg_id),
                                                   L_native_machine_ctype,
                                                   L_PTYPE_NULL);
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_oper2);
    }
  else
    {
      L_punt
        ("Lsuper_make_remainder_loop: invalid induction increment - op: %d\n",
         loop_incr_op->id);
    }

  /* branch to check if there are enough iterations to enter header cb */

  new_br = L_create_new_op (loop_back_br->opc);
  L_copy_compare (new_br, loop_back_br);
  L_negate_compare (new_br);
  if (max_pos == 1)
    {
      new_br->src[0] = L_copy_operand (loop_back_br->src[0]);
      new_br->src[1] = L_copy_operand (new_oper2->dest[0]);
    }
  else
    {
      new_br->src[0] = L_copy_operand (new_oper2->dest[0]);
      new_br->src[1] = L_copy_operand (loop_back_br->src[1]);
    }
  new_br->src[2] = L_new_cb_operand (remainder_cb);
  L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_br);

  /* add branch to epilogue_cb */
  new_oper1 = L_create_new_op (new_br->opc);
  L_copy_compare (new_oper1, new_br);
  new_oper1->src[0] = L_copy_operand (loop_back_br->src[0]);
  new_oper1->src[1] = L_copy_operand (loop_back_br->src[1]);
  new_oper1->src[2] = L_new_cb_operand (fall_thru_cb);
  L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper1);

  /* change max in header cb */
  L_delete_operand (loop_back_br->src[max_pos]);
  loop_back_br->src[max_pos] = L_copy_operand (new_oper2->dest[0]);

  L_rebuild_src_flow (fn);

  if (Lsuper_debug_loop_unroll)
    {
      L_check_func (fn);
    }

  /* remove sync arcs which cannot possibly exist and make others 
     conservative */
  L_adjust_syncs_for_remainder (header_cb, remainder_cb);
}

void
Lsuper_create_remainder_loops (L_Func * fn)
{
  L_Inner_Loop *loop;
  L_Cb *header_cb;
  L_Oper *loop_incr_op;         /* loop counter induction op */
  int unroll;                   /* number of times loop was unrolled */

  /* Conditions for doing optimization: 1) Loop body is not a hyperblock, 
     2) loop back branch is based on a loop counter, 3) compare and branch
     is signed and not bne or beq, 4) optimization appears to be a win. */
  for (loop = fn->first_inner_loop; loop;
       loop = loop->next_inner_loop)
    {
      header_cb = loop->cb;
      if ((!L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_HYPERBLOCK)) &&
          (loop_incr_op = Lsuper_is_counted_loop (loop)) &&
          Lsuper_valid_branch_for_rem_loop (loop->feedback_op) &&
          (unroll = Lsuper_remainder_loop_is_win (loop)))
        {
          Lsuper_make_remainder_loop (fn, loop, loop_incr_op, unroll);
        }
    }
}

