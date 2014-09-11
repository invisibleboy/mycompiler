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
 *      File :          l_accumulator.c
 *      Description :   Accumulator/Induction var expansion
 *      Creation Date : July 1993
 *      Author :        John Gyllenhaal Scott Mahlke
 *      Modifications:
 *          Dan Lavery, April 1996
 *          - added Lsuper_move_comp_code_out_of_softpipe_loops
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define DEBUG_IND_EXP   Lsuper_debug_acc_exp
#define DEBUG_ACC_EXP   Lsuper_debug_acc_exp

#define ERR     stderr

#define DONT_INSERT     0
#define INSERT_BEFORE   1
#define INSERT_AFTER    2

typedef struct _AccNode {
  L_Operand *dest;
  L_Operand *src;
  int same_src;
  int opcode;
  int is_sum;
  int eligible;
  int count;
  int pred_flag;
  Set ops;
  struct _AccNode *next;
} AccNode;

AccNode *acc_list_head = NULL;


static AccNode *
add_accum (L_Oper * oper)
{
  int opcode;
  AccNode *cur;

  opcode = oper->opc;

  /* Look for acc in current list first */
  cur = acc_list_head;
  while (cur)
    {
      if (L_same_operand (cur->dest, oper->dest[0]))
        {
          if (L_is_compatible_opc (cur->opcode, opcode))
            {
              cur->ops = Set_add (cur->ops, oper->id);
              cur->count++;
              if (L_is_predicated (oper))
                cur->pred_flag = 1;
              if (L_same_operand (oper->dest[0], oper->src[0]))
                {
                  if (!L_same_operand (cur->src, oper->src[1]))
                    cur->same_src = 0;
                }
              else
                {
                  if (!L_same_operand (cur->src, oper->src[0]))
                    cur->same_src = 0;
                }
            }
          /* Otherwise, used two different opcodes with same accumulator
           * This register is no longer eligible for acc expansion
           */
          else
            {
              cur->eligible = 0;
            }
          break;
        }
      cur = cur->next;
    }

  if (!cur)
    {
      cur = (AccNode *) Lcode_malloc (sizeof (AccNode));
      cur->dest = L_copy_operand (oper->dest[0]);
      if (L_same_operand (oper->dest[0], oper->src[0]))
        cur->src = L_copy_operand (oper->src[1]);
      else
        cur->src = L_copy_operand (oper->src[0]);
      cur->same_src = 1;
      cur->opcode = opcode;
      /* Get whether this is a sum or product accumulation */
      if (L_add_opcode (oper) || L_sub_opcode (oper))
        cur->is_sum = 1;
      else
        cur->is_sum = 0;
      cur->eligible = 1;
      cur->count = 1;
      cur->pred_flag = L_is_predicated (oper);
      cur->ops = Set_add (NULL, oper->id);
      cur->next = acc_list_head;
      acc_list_head = cur;
    }
  return (cur);
}

static void
free_accum_list ()
{
  AccNode *cur, *temp;
  cur = acc_list_head;
  while (cur != NULL)
    {
      temp = cur;
      cur = cur->next;
      Set_dispose (temp->ops);
      L_delete_operand (temp->dest);
      L_delete_operand (temp->src);
      free (temp);
    }
  acc_list_head = NULL;
}

static void
L_replace_induction_var (L_Func * fn, L_Inner_Loop * loop, AccNode * acc)
{
  int i, c;
  ITintmax inci;
  double incf2;
  L_Cb *cb, *preheader;
  int mov_opcode, inc_count, *used_flag;
  L_Operand **inc_array, **var_array, *group_inc;
  L_Oper *new_op, *cur_op, *next_op;

  if (DEBUG_IND_EXP)
    {
      fprintf (ERR, "$$$ Eliminating induction variable ");
      L_print_operand (ERR, acc->dest, 1);
      fprintf (ERR, " in cb %d\n", loop->cb->id);
    }

  inc_array = (L_Operand **) alloca ((acc->count + 1) * 
				     sizeof (L_Operand *));
  var_array = (L_Operand **) alloca ((acc->count + 1) * 
				     sizeof (L_Operand *));
  used_flag = (int *) alloca ((acc->count + 1) * sizeof (int));

  /* initialize L_Operand arrays to contain NULL ptrs */
  for (i = 0; i <= acc->count; i++)
    {
      inc_array[i] = NULL;
      var_array[i] = NULL;
    }

  cb = loop->cb;
  preheader = loop->preheader;

  if (L_is_int_constant (acc->src))
    {
      mov_opcode = Lop_MOV;
      /* Create registers for induction variables */
      for (i = 0; i <= acc->count; i++)
        var_array[i] =
          L_new_register_operand (++L_fn->max_reg_id, L_native_machine_ctype,
                                  L_PTYPE_NULL);

      /* Generate int const increments */
      inci = acc->src->value.i;
      for (i = 1; i <= acc->count; i++)
        inc_array[i] = L_new_gen_int_operand (inci * i);

      group_inc = inc_array[acc->count];
    }
  else if (L_is_flt_constant (acc->src))
    {
      mov_opcode = Lop_MOV_F;
      /* Create registers for induction variables */
      for (i = 0; i <= acc->count; i++)
        var_array[i] =
          L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_FLOAT,
                                  L_PTYPE_NULL);

      incf2 = acc->src->value.f;
      for (i = 1; i <= acc->count; i++)
        inc_array[i] = L_new_float_operand (incf2 * (float) i);

      group_inc = inc_array[acc->count];
    }
  else if (L_is_dbl_constant (acc->src))
    {
      mov_opcode = Lop_MOV_F2;
      /* Create registers for induction variables */
      for (i = 0; i <= acc->count; i++)
        var_array[i] =
          L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_DOUBLE,
                                  L_PTYPE_NULL);

      incf2 = acc->src->value.f2;
      for (i = 1; i <= acc->count; i++)
        inc_array[i] = L_new_double_operand (incf2 * (double) i);

      group_inc = inc_array[acc->count];
    }
  else if (L_is_register (acc->src) && L_is_ctype_integer (acc->src))
    {
      mov_opcode = Lop_MOV;
      /* Create registers for induction variables  and increments */
      for (i = 0; i <= acc->count; i++)
        {
          var_array[i] =
            L_new_register_operand (++L_fn->max_reg_id,
                                    L_native_machine_ctype, L_PTYPE_NULL);
        }

      /* Don't need any calcs for 1 * acc->src, just copy */
      inc_array[1] = L_copy_operand (acc->src);

      for (i = 2; i < acc->count; i++)
        {
          inc_array[i] = L_copy_operand (var_array[i]);
        }

      /* Allocate register for holding group increment and put inc in there */
      group_inc = L_new_register_operand (++L_fn->max_reg_id, 
					  L_native_machine_ctype, 
					  L_PTYPE_NULL);
      inc_array[acc->count] = group_inc;

      for (i = 2; i <= acc->count; i++)
        {
          new_op = L_create_new_op (Lop_MUL);
          new_op->dest[0] = L_copy_operand (inc_array[i]);
          new_op->src[0] = L_copy_operand (acc->src);
          new_op->src[1] = L_new_gen_int_operand (i);
          L_insert_oper_after (preheader, preheader->last_op, new_op);
        }
    }
  else if (L_is_register (acc->src) && (L_is_ctype_flt (acc->src)))
    {
      mov_opcode = Lop_MOV_F;
      /* Create registers for induction variables  and increments */
      for (i = 0; i <= acc->count; i++)
        {
          var_array[i] =
            L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_FLOAT,
                                    L_PTYPE_NULL);
        }

      /* Don't need any calcs for 1 * acc->src, just copy */
      inc_array[1] = L_copy_operand (acc->src);

      for (i = 2; i < acc->count; i++)
        {
          inc_array[i] = L_copy_operand (var_array[i]);
        }

      /* Allocate register for holding group increment and put inc in there */
      group_inc = L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_FLOAT,
                                          L_PTYPE_NULL);
      inc_array[acc->count] = group_inc;

      for (i = 2; i <= acc->count; i++)
        {
          new_op = L_create_new_op (Lop_MUL_F);
          new_op->dest[0] = L_copy_operand (inc_array[i]);
          new_op->src[0] = L_copy_operand (acc->src);
          new_op->src[1] = L_new_float_operand ((double) i);
          L_insert_oper_after (preheader, preheader->last_op, new_op);
        }
    }
  else if (L_is_register (acc->src) && (L_is_ctype_dbl (acc->src)))
    {
      mov_opcode = Lop_MOV_F2;
      /* Create registers for induction variables  and increments */
      for (i = 0; i <= acc->count; i++)
        {
          var_array[i] =
            L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_DOUBLE,
                                    L_PTYPE_NULL);
        }

      /* Don't need any calcs for 1 * acc->src, just copy */
      inc_array[1] = L_copy_operand (acc->src);

      for (i = 2; i < acc->count; i++)
        {
          inc_array[i] = L_copy_operand (var_array[i]);
        }

      /* Allocate register for holding group increment and put inc in there */
      group_inc = L_new_register_operand (++L_fn->max_reg_id, L_CTYPE_DOUBLE,
                                          L_PTYPE_NULL);
      inc_array[acc->count] = group_inc;

      for (i = 2; i <= acc->count; i++)
        {
          new_op = L_create_new_op (Lop_MUL_F2);
          new_op->dest[0] = L_copy_operand (inc_array[i]);
          new_op->src[0] = L_copy_operand (acc->src);
          new_op->src[1] = L_new_double_operand ((double) i);
          L_insert_oper_after (preheader, preheader->last_op, new_op);
        }

    }
  else
    {
      L_punt ("Unknown source type in L_replace_induction_var");
      return;                   /* L_punt doesn't returns */
    }

  /* Move the initial value to var[0], mov_opcode determined above */
  new_op = L_create_new_op (mov_opcode);
  new_op->dest[0] = L_copy_operand (var_array[0]);
  new_op->src[0] = L_copy_operand (acc->dest);
  L_insert_oper_after (preheader, preheader->last_op, new_op);

  /* Add/sub the increment*i to each of the vars[i] */
  for (i = 1; i <= acc->count; i++)
    {
      new_op = L_create_new_op (acc->opcode);
      new_op->dest[0] = L_copy_operand (var_array[i]);
      new_op->src[0] = L_copy_operand (acc->dest);
      new_op->src[1] = L_copy_operand (inc_array[i]);
      L_insert_oper_after (preheader, preheader->last_op, new_op);
    }

  /* Initialize used flags */
  for (i = 0; i <= acc->count; i++)
    used_flag[i] = 0;

  /* Always use var_array[0] in last branch */
  used_flag[0] = 1;

  /* Now, go through loop replacing induction variable */
  inc_count = 0;
  for (cur_op = cb->first_op; cur_op != NULL; cur_op = next_op)
    {
      /* Get next op now, since may delete cur_op */
      next_op = cur_op->next_op;

      /* If at end of loop, add in increment statements before loop back */
      if (cur_op == loop->feedback_op)
        {
          for (i = 0; i <= acc->count; i++)
            {
              /* Insert increment code if variable used */
              if (used_flag[i] == 1)
                {
                  new_op = L_create_new_op (acc->opcode);
                  new_op->dest[0] = L_copy_operand (var_array[i]);
                  new_op->src[0] = L_copy_operand (var_array[i]);
                  new_op->src[1] = L_copy_operand (group_inc);
                  L_insert_oper_before (cb, cur_op, new_op);
                }
            }
          /* var_array[0] now holds the current value of the induc var */
          inc_count = 0;
        }

      /* If we are at an increment/decrement instruction */
      if (Set_in (acc->ops, cur_op->id))
        {
          inc_count++;
          /* Remove the inc/dec instruction */
          L_delete_oper (cb, cur_op);
          continue;
        }

      /* Replace induction variable in the sources of cur_op */
      for (c = 0; c < L_max_src_operand; c++)
        {
          if (L_same_operand (acc->dest, cur_op->src[c]))
            {
              L_delete_operand (cur_op->src[c]);
              cur_op->src[c] = L_copy_operand (var_array[inc_count]);
              used_flag[inc_count] = 1;
            }
        }

      /* Replace induction variable in the pred of cur_op */

      if (cur_op->pred[0] && L_same_operand (acc->dest, cur_op->pred[0]))
	{
	  L_delete_operand (cur_op->pred[0]);
	  cur_op->pred[0] = L_copy_operand (var_array[inc_count]);
	  used_flag[inc_count] = 1;
        }

      /* Handle branches, update value of var if necessary */
      if (L_general_branch_opcode (cur_op))
        {
	  L_Cb *branch_cb = NULL;
	  int insert_var;

          /* If we have a known target, see if var live at target */
          if (L_cond_branch_opcode (cur_op) ||
	      L_uncond_branch_opcode (cur_op))
            {
	      branch_cb = L_find_branch_dest (cur_op);
	      insert_var = DONT_INSERT;

              /* If not a backedge, just check live_in of target */
              if (branch_cb != cb)
                {
                  if (L_in_oper_OUT_set (cb, cur_op, acc->dest, TAKEN_PATH))
                    insert_var = INSERT_BEFORE;
                }
              /* If backedge, and is last branch before end of cb,
               * see if need to insert update after branch.
               * Updates are never needed before backedges.
               */
              else
                {
                  /* If there is a fall through id then
                   * assume this is the last branch in cb and
                   * see if we need to update var for fall_through
                   */

                  if (L_has_fallthru_to_next_cb (cb) &&
		      L_in_cb_IN_set (cb->next_cb, acc->dest))
		    insert_var = INSERT_AFTER;
                }
            }
	  else
	    {
              /* Hash jump, just assume need to insert update before */
	      insert_var = INSERT_BEFORE;
	    }

          /* Do we need to insert var update ? */
          if (insert_var != DONT_INSERT)
            {
              /* Yes, insert instruction to get current value of
               * induction var
               */
              new_op = L_create_new_op (mov_opcode);
              new_op->dest[0] = L_copy_operand (acc->dest);
              new_op->src[0] = L_copy_operand (var_array[inc_count]);
              used_flag[inc_count] = 1;
              if (insert_var == INSERT_BEFORE)
                L_insert_oper_before (cb, cur_op, new_op);
              else
                L_insert_oper_after (cb, cur_op, new_op);
            }
        }
    }

  /* Free allocated memory */
  for (i = 0; i <= acc->count; i++)
    {
      if (inc_array[i])
	L_delete_operand (inc_array[i]);
      if (var_array[i])
	L_delete_operand (var_array[i]);
    }

  L_invalidate_dataflow ();
  return;
}

#if 0
/*
 * Returns 1 if there is exactly one predecessor cb (but there may be
 * multiple arcs from that cb), 0 otherwise.
 */
static int
L_exactly_one_predecessor_cb (L_Cb * cb)
{
  L_Flow *flow;
  L_Cb *src_cb;

  if (!cb || !cb->src_flow)
    return (0);

  flow = cb->src_flow;
  src_cb = flow->src_cb;
  while ((flow = flow->next_flow))
    if (src_cb != flow->src_cb)
      return (0);

  return (1);
}
#endif

static int
L_exactly_one_predecessor_arc (L_Cb * cb)
{
  L_Flow *flow;

  if (!cb || !cb->src_flow)
    return (0);
  flow = cb->src_flow;

  return (flow->next_flow ? 0 : 1);
}

/* Adds the sum tree to the front of the cb specfied */
/*
 *      SAM changed to take any number for sum_size, not just power of 2!!
 */
static void
add_sum_tree_to_front (L_Cb * cb, L_Operand ** sum_array, int sum_size,
                       int sum_opcode, L_Operand * dest)
{
  int s, d, i;
  L_Operand **temp_array;
  int temp_size;
  L_Oper *new_op, *after_op;
  double weight;

#if 0
  /* make temp_size smallest power of 2 >= sum_size */
  temp_size = C_log2 (sum_size);
  temp_size = C_pow2 (temp_size);
#endif
  temp_size = sum_size;

  /* Make a copy of sum_array that we can destroy */
  temp_array = (L_Operand **) alloca (temp_size * sizeof (L_Operand *));

  for (i = 0; i < sum_size; i++)
    temp_array[i] = sum_array[i];
  for (i = sum_size; i < temp_size; i++)
    temp_array[i] = NULL;


  /* Get weight to assign to new ops */
  weight = cb->weight;
  after_op = NULL;

  /* Form add/mult tree from temp array (temp_size must be power of two) */
  while (temp_size > 0)
    {
      d = 0;
      for (s = 0; (s + 1) < temp_size; s += 2)
        {
          /* If have two operands in array, insert lcode operation */
          if (temp_array[s + 1] != NULL)
            {
              new_op = L_create_new_op (sum_opcode);
              new_op->weight = weight;
              /* If last operation in add tree, put result into 'dest' */
              if (temp_size == 2)
                new_op->dest[0] = L_copy_operand (dest);
              else
                new_op->dest[0] = L_copy_operand (temp_array[s]);
              new_op->src[0] = L_copy_operand (temp_array[s]);
              new_op->src[1] = L_copy_operand (temp_array[s + 1]);
              if (after_op == NULL)
                L_insert_oper_before (cb, cb->first_op, new_op);
              else
                L_insert_oper_after (cb, after_op, new_op);
              after_op = new_op;
            }
          /* Copy results of add/mult only into temp_array,
           * the array is now half as big.
           */
          temp_array[d] = temp_array[s];
          d++;
        }
      temp_size = temp_size >> 1;
    }

  return;
}

/*
 * Adds a cb between from_cb and to_cb, altering the flow of the
 * cb's as necessary.  If cb_before fall through to cb_after, the
 * new cb is inserted between them, otherwise, the new cb is placed at
 * the end of the function.
 * The new cb is returned.
 */
static L_Cb *
L_add_cb_between (L_Func * fn, L_Cb * from_cb, L_Cb * to_cb)
{
  L_Cb *new_cb;
  L_Oper *new_op;
  double weight;
  L_Flow *flow, *new_flow;

  /* Get weight of all the flows currently going to to_cb_id */
  weight = 0.0;
  flow = from_cb->dest_flow;
  while (flow != NULL)
    {
      if (flow->dst_cb == to_cb)
        weight += flow->weight;
      flow = flow->next_flow;
    }

  /* Create new cb */
  new_cb = L_create_cb (weight);

  /*
   * If from_cb falls through to to_cb, place new_cb between them,
   * otherwise, place at end of function.
   */
  if ((from_cb->next_cb) == to_cb)
    L_insert_cb_after (fn, from_cb, new_cb);
  else
    {
      L_insert_cb_after (fn, fn->last_cb, new_cb);
      new_op = L_create_new_op (Lop_JUMP);
      new_op->weight = weight;
      new_op->src[0] = L_new_cb_operand (to_cb);
      L_insert_oper_after (new_cb, NULL, new_op);
    }

  /* Change all the dest flow info in from_cb to goto new cb */
  L_change_dest_cb (from_cb, to_cb, new_cb);
  /* Change all the branches in from_cb to goto new cb */
  L_change_all_branch_dest (from_cb, to_cb, new_cb);

  /* Add flow from new_cb to to_cb */
  new_flow = L_new_flow (1, new_cb, to_cb, weight);
  new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);

  /* Source flows corrupted, will do L_rebuild_src_flow when finished */
  /* Return id of new_cb */
  return (new_cb);
}

static void
L_replace_accum (L_Func * fn, L_Inner_Loop * loop, AccNode * acc)
{
  L_Cb *cb, *preheader, *new_cb, *target_cb;
  L_Flow *flow;
  L_Oper *new_op, *cur_op;
  L_Operand **sum_array, *initial_val;
  int sum_size, i;
  int mov_opcode, sum_opcode;
  Set flow_cbs = NULL;
  int *flow_array, flow_size;

  if (DEBUG_ACC_EXP)
    {
      fprintf (ERR, "$$$ Eliminating accumulator variable ");
      L_print_operand (ERR, acc->dest, 1);
      fprintf (ERR, " in cb %i\n", loop->cb->id);
      fprintf (ERR, "acc->count = %d\n", acc->count);
    }

  /* make temp_size smallest power of 2 >= sum_size */
  sum_size = C_pow2 (C_log2 (acc->count));

  /* Allocate memory for sum_array */
  sum_array = (L_Operand **) alloca (sum_size * sizeof (L_Operand *));

  /* Allocate the tempory registers */
  for (i = 0; i < acc->count; i++)
    sum_array[i] =
      L_new_register_operand (++L_fn->max_reg_id,
                              L_return_old_ctype (acc->dest), L_PTYPE_NULL);

  /* Nullify the rest of the array */
  for (i = acc->count; i < sum_size; i++)
    sum_array[i] = NULL;

  /*
   * For adds/subs, intializize new accums to 0,
   * For mults/divs, initialize new accums to 1.
   * Also select appropriate move opcode.
   * Also select appropriate sum opcode.
   */
  switch (L_operand_case_ctype (acc->dest))
    {
    case L_CTYPE_INT:
    case L_CTYPE_LLONG:
      mov_opcode = Lop_MOV;
      if (acc->is_sum == 1)
        {
          initial_val = L_new_gen_int_operand (0);
          sum_opcode = Lop_ADD;
        }
      else
        {
          initial_val = L_new_gen_int_operand (1);
          sum_opcode = Lop_MUL;
        }
      break;

    case L_CTYPE_FLOAT:
      mov_opcode = Lop_MOV_F;
      if (acc->is_sum == 1)
        {
          initial_val = L_new_float_operand (0.0);
          sum_opcode = Lop_ADD_F;
        }
      else
        {
          initial_val = L_new_float_operand (1.0);
          sum_opcode = Lop_MUL_F;
        }
      break;

    case L_CTYPE_DOUBLE:
      mov_opcode = Lop_MOV_F2;
      if (acc->is_sum == 1)
        {
          initial_val = L_new_double_operand (0.0);
          sum_opcode = Lop_ADD_F2;
        }
      else
        {
          initial_val = L_new_double_operand (1.0);
          sum_opcode = Lop_MUL_F2;
        }
      break;

    default:
      L_punt ("Unknown register type in L_replace_accum");
      return;
    }

    /****  Insert new accumulators initialization in preheader ***/

  cb = loop->cb;
  preheader = loop->preheader;

  /*
   * Here I assume it is always a fall-through from preheader to the loop.
   * Also I am not worrying about weight or color, these can be recomputed
   * later if needed.
   */

  for (i = 0; i < acc->count; i++)
    {
      new_op = L_create_new_op (mov_opcode);
      new_op->weight = 0.0;
      new_op->dest[0] = L_copy_operand (sum_array[i]);

      /*
       * If first temp sum, copy in current value of sum
       * Otherwise, initialize to 0 or 1
       */

      new_op->src[0] = L_copy_operand (i ? initial_val : acc->dest);
      L_insert_oper_after (preheader, preheader->last_op, new_op);
    }

  for (cur_op = cb->first_op, i = 0; cur_op; cur_op = cur_op->next_op)
    {
      if (!Set_in (acc->ops, cur_op->id))
        continue;
      if (i >= acc->count)
        L_punt ("L_replace_accum: too many acc ops");
      /* Plug in temp accumulator */
      if (L_same_operand (cur_op->dest[0], cur_op->src[0]))
        {
          L_delete_operand (cur_op->src[0]);
          cur_op->src[0] = L_copy_operand (sum_array[i]);
        }
      else
        {
          L_delete_operand (cur_op->src[1]);
          cur_op->src[1] = L_copy_operand (sum_array[i]);
        }
      L_delete_operand (cur_op->dest[0]);
      cur_op->dest[0] = L_copy_operand (sum_array[i]);
      i++;
    }

  /* Get the Set of cbs the loop goes to */

  for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
    flow_cbs = Set_add (flow_cbs, flow->dst_cb->id);

  /* Remove the loop back flow */
  flow_cbs = Set_delete (flow_cbs, cb->id);

  /* Eliminate Cb's that dont have the accum var in their in list */

  flow_size = Set_size (flow_cbs);

  flow_array = alloca (flow_size * sizeof (int));
  Set_2array (flow_cbs, flow_array);

  for (i = 0; i < flow_size; i++)
    {
      if (!L_in_cb_IN_set (L_cb_hash_tbl_find (L_fn->cb_hash_tbl, 
					       flow_array[i]), acc->dest))
        flow_cbs = Set_delete (flow_cbs, flow_array[i]);
    }

  /* Get final list of cbs we need to add sum calculation to */

  flow_size = Set_2array (flow_cbs, flow_array);

  for (i = 0; i < flow_size; i++)
    {
      target_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, flow_array[i]);
      /* JWS 19991221--changed from one_predecessor_cb to one_predecessor_arc.
       * When multiple definitions are available on different arcs, one would 
       * get squashed. 
       */
      if (L_exactly_one_predecessor_arc (target_cb))
	{
	  add_sum_tree_to_front (target_cb, sum_array, sum_size,
				 sum_opcode, acc->dest);
	}
      else
        {
          new_cb = L_add_cb_between (fn, cb, target_cb);
          add_sum_tree_to_front (new_cb, sum_array, sum_size,
                                 sum_opcode, acc->dest);
          L_clear_src_flow (fn);
          L_rebuild_src_flow (fn);
          L_do_flow_analysis (fn, LIVE_VARIABLE);
        }
    }

  /* free up space */
  for (i = 0; i < acc->count; i++)
    if (sum_array[i])
      L_delete_operand (sum_array[i]);

  L_delete_operand (initial_val);

  L_invalidate_dataflow ();

  Set_dispose (flow_cbs);

  return;
}

static void
L_do_accum_exp (L_Func * fn, L_Inner_Loop * loop)
{
  L_Oper *oper;
  L_Cb *cb;
  int potential_accum, src2_allowed, c, legal_opc, fp_arithmetic;
  Set used = NULL, defined = NULL, accumed = NULL;
  AccNode *acc;

  /* Remove any accum lists hanging around */
  free_accum_list ();

  cb = loop->cb;

  for (oper = cb->first_op; oper; oper = oper->next_op)
    {
      potential_accum = 0;

      /* Is it an opcode that an accumulator can be? */
      legal_opc = L_add_opcode (oper) || L_sub_opcode (oper) ||
	L_mul_opcode (oper) || 
	(L_div_opcode (oper) && !L_int_div_opcode (oper));
      /* DMG - don't do floating point acc exp unless
         flag set - causes precision problems */
      fp_arithmetic = L_flt_arithmetic_opcode (oper) ||
        L_dbl_arithmetic_opcode (oper);

      if (legal_opc && (Lsuper_do_float_acc_exp || !fp_arithmetic))
        {
          /* If sub or div, accumulator must be src1 */

	  src2_allowed = !(L_sub_opcode (oper) || L_div_opcode (oper));

          /* Does it have the form of an accumulator? */
          if (L_is_register (oper->dest[0]) &&
	      !L_same_operand (oper->src[0], oper->src[1]) &&
              (L_same_operand (oper->dest[0], oper->src[0]) ||
               (src2_allowed && L_same_operand (oper->dest[0], oper->src[1]))))
            {
              /* Add to list of potential accumulators */
              add_accum (oper);

              /*
               * Flag that have accumulator statement so accumulator
               * register will not be added to non_accum set
               */
              potential_accum = 1;
              accumed = Set_add (accumed, oper->dest[0]->value.r);

              /* Add non-accumulator source to non accum register set */
              if (L_same_operand (oper->dest[0], oper->src[0]))
                {
                  if (L_is_register (oper->src[1]))
		    used = Set_add (used, oper->src[1]->value.r);
                }
              else
                {
                  if (L_is_register (oper->src[0]))
		    used = Set_add (used, oper->src[0]->value.r);
                }
            }
        }

      /*
       * If just didn't find a potential acummulator statement,
       * add the registers used in this op to the set of non_accum
       * register.
       */

      if (!potential_accum)
        {
          for (c = 0; c < L_max_dest_operand; c++)
	    if (L_is_register (oper->dest[c]))
	      defined = Set_add (defined, oper->dest[c]->value.r);

          for (c = 0; c < L_max_src_operand; c++)
	    if (L_is_register (oper->src[c]))
	      used = Set_add (used, oper->src[c]->value.r);

	  if (oper->pred[0] && L_is_register (oper->pred[0]))
	    used = Set_add (used, oper->pred[0]->value.r);
        }
    }

  /* Now go through the linked list of potential accumulators. */

  acc = acc_list_head;
  while (acc)
    {
      int src_constant, src_numeric_constant;
      int add_sub_opcode;

      /*
       * Determine if the "increment" is a constant with respect to the
       * loop.  The tests for this are:
       * 1) Same "increment" used in every accum.
       * 2) Its a numeric constant or if it is a register, it hasn't
       *    been redefined in the loop by an non-accum instruction
       *    or a accum instruction.
       *
       *    This last requirement is to prevent the following from looking
       *    like a const.
       *
       *    i = i + 1;
       *    sum = sum + i;  <--i not const, but not in defined set
       */

      src_constant = 0;
      src_numeric_constant = 0;
      add_sub_opcode = 0;

      if (acc->same_src &&
          (L_is_numeric_constant (acc->src) ||
           (L_is_register (acc->src) &&
            !Set_in (defined, acc->src->value.r) &&
            !Set_in (accumed, acc->src->value.r))))
        {
          src_constant = 1;
          if (L_is_numeric_constant (acc->src))
            src_numeric_constant = 1;
        }

      if ((acc->opcode == Lop_ADD) || (acc->opcode == Lop_ADD_U) ||
          (acc->opcode == Lop_ADD_F) || (acc->opcode == Lop_ADD_F2) ||
          (acc->opcode == Lop_SUB) || (acc->opcode == Lop_SUB_U) ||
          (acc->opcode == Lop_SUB_F) || (acc->opcode == Lop_SUB_F2))
        add_sub_opcode = 1;

#if defined (VAR_EXPANSION_USE_CUTSET_METRIC)
      {
        int cnt_livein, num_reg;

        cnt_livein = Set_size (L_get_cb_IN_set(loop->preheader));
        num_reg = M_num_registers (L_native_machine_ctype);

        if (cnt_livein > (VAR_EXPANSION_CUTSET_RATIO * num_reg))
          {
#if defined (DEBUG_VAR_EXPANSION_METRICS)
            fprintf (ERR, ">Accumulator expansion suppressed at block %d "
                     "(cutset %d/%d)\n", peel_region->block->id, cnt_livein,
                      num_reg);
#endif
            break;
          }
      }
#endif

      /*
       * Do not use the potental accumulators that are ineligible for one
       * of the following reasons:
       * 1) Only one accumulator in the loop (no opti possible)
       * 2) There is two opr more types of accumulators using the
       *    same register (the eligible flag flags this)
       * 3) The accumulator register is used somewhere else in the loop.
       * 4) SAM- if accumulator is just accumulating numeric constant,
       *      use induction expansion it is more efficient.
       */

      if ((acc->count > 1) && (acc->eligible == 1) &&
	  !Set_in (defined, acc->dest->value.r))
	{
	  if ((!src_numeric_constant || !add_sub_opcode) &&  
	      !Set_in (used, acc->dest->value.r))
	    {
	      L_replace_accum (fn, loop, acc);
	      STAT_COUNT ("L_replace_accum", 1, NULL);
	    }
	  /*
	   * Do induction variable expansion if the source is constant
	   * and the destination is not redefined anywhere in the loop.
	   */
	  else if (src_constant && add_sub_opcode && !acc->pred_flag)
	    {
	      L_replace_induction_var (fn, loop, acc);
	      STAT_COUNT ("L_replace_induction_var", 1, NULL);
	    }
	}

      acc = acc->next;
    }

  /* Free memory used */
  free_accum_list ();
  used = Set_dispose (used);
  defined = Set_dispose (defined);
  accumed = Set_dispose (accumed);
}


/* Make sure compensation code from accumulator expansion is put
   into a separate cb for loops marked for software pipelining. */
void
Lsuper_move_comp_code_out_of_softpipe_loops (L_Func * fn)
{
  L_Inner_Loop *loop;
  L_Cb *cb, *fallthru_cb;
  L_Oper *oper, *feedback_op, *prev_oper;
  L_Oper *last_comp_op;

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      feedback_op = loop->feedback_op;
      cb = loop->cb;
      if ((L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) ||
           L_find_attr (cb->attr, "L_CB_SOFTPIPE")) &&
          (feedback_op->next_op != NULL) &&
          !L_uncond_branch_opcode (feedback_op->next_op))
        {
          fallthru_cb = L_create_cb_at_fall_thru_path (cb, 1);

          /* Handle potential case where loop cb ends in jump after the
             compensation code and target of jump has only 1 predecessor.
             Don't move the jump out of loop cb. */

	  last_comp_op = L_uncond_branch_opcode (cb->last_op) ?
	    cb->last_op->prev_op : cb->last_op;

          for (oper = last_comp_op; oper != feedback_op; oper = prev_oper)
            {
              prev_oper = oper->prev_op;
              L_move_op_to_start_of_block (cb, fallthru_cb, oper);
            }
        }
    }
}


/*
 *      Export function
 */
void
Lsuper_accumulator_expansion (L_Func * fn)
{
  L_Inner_Loop *loop;

  if (fn->weight == 0.0)
    return;
  if (!fn->first_inner_loop)
    return;

  if (DEBUG_ACC_EXP | DEBUG_IND_EXP)
    fprintf (ERR, "Enter accumulator exp...\n");

  L_update_flow_analysis (fn, LIVE_VARIABLE);

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    L_do_accum_exp (fn, loop);

  L_invalidate_dataflow ();

  /* if L_do_accum_exp has run, we need to rebuild src flow info */

  L_clear_src_flow (fn);
  L_rebuild_src_flow (fn);
  return;
}
