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
 *      File :          l_loop_unroll.c
 *      Description :   superblock loop detection, unrolling, peeling
 *      Author :        Scott Mahlke
 *                      Bob McGowan - fixed length loops
 *      Date :          August 1991
 *      export fncts:   L_superblock_loop_opt(fn)
 * 
 *      Changes: upgraded to new Lcode Sep 93
 *               Added fixed length loops.
 *
 *      (C) Copyright 1991, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define DO_UNROLLING

#undef  DEBUG_FIXED_LENGTH

#define ERR     stderr

/*===========================================================================*/

/* loop unrolling stuff */
#define L_MIN_WEIGHT_FOR_UNROLL         100.0
#define L_CRIT_WEIGHT_FOR_UNROLL        10000.0
#define L_MAX_OPER_FOR_SMALL_LOOP       10      /* was 12 */
#define L_MIN_UNROLL_FOR_SMALL_LOOPS    IMPACT_MIN(Lsuper_issue_rate/2,       \
                                                   Lsuper_max_unroll_allowed)
#define L_UNROLL_LIMIT                  IMPACT_MIN(Lsuper_issue_rate,         \
                                                   Lsuper_max_unroll_allowed)
#define L_CRIT_UNROLL_LIMIT             IMPACT_MIN(Lsuper_issue_rate*2,       \
                                                   Lsuper_max_unroll_allowed)

#define MAX_INDUCTION_VAR_OPS_IN_LOOP   32
#define MAX_LOOP_COUNTER_INITS          12
#define FLOW_FEEDBACK        (0x1)
#define FLOW_FEEDBACK_CLEAR  (~FLOW_FEEDBACK)

struct induction_init_ops_info_struct
{
  L_Cb *cb;
  L_Oper *op;
};
typedef struct induction_init_ops_info_struct induction_init_ops_info;

/*===========================================================================*/

#define UNROLL_SCHEMA_NONE  0
#define UNROLL_SCHEMA_CONT  1
#define UNROLL_SCHEMA_PRED  2

static L_Flow **L_last_flow;
static L_Oper **L_last_br;
static int *L_pred_map;
static int max_reg;

/*
 *      Predicates for loop optimizations
 */

/*
 * Lsuper_can_subjugate
 * ----------------------------------------------------------------------
 * Return 1 iff the feedback op is guarded by a predicate which can be
 * used to guard a successive unrolled iteration of the loop.
 */

static int
Lsuper_can_subjugate (L_Inner_Loop *loop)
{
  L_Cb *lp_cb;
  L_Oper *op, *fb_op;
  L_Operand *fb_pr, *pr;
  Set iter_def = NULL;
  int i;

  lp_cb = loop->cb;
  fb_op = loop->feedback_op;
  fb_pr = fb_op->pred[0];

  if (!fb_pr || !L_uncond_branch_opcode (fb_op) || !L_is_predicated (fb_op))
    return 0;

  for (op = lp_cb->first_op; op; op = op->next_op)
    {
      if ((pr = op->pred[0]) && !Set_in (iter_def, L_REG_MAC_INDEX (pr)))
	break;

      if (L_pred_define_opcode (op))
	{
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!(pr = op->dest[i]) ||
		  !L_is_ctype_predicate (pr) ||
		  !(L_uncond_ptype (pr->ptype) || 
		    L_initializing_pred_define_opcode (op)))
		continue;

	      iter_def = Set_add (iter_def, L_REG_MAC_INDEX (pr));
	    }
	}

      if (op == fb_op)
	break;
    }

  Set_dispose (iter_def);

  if (op != fb_op)
    return 0;

  return 1;
}


/*
 * Lsuper_convert_to_frp
 * ----------------------------------------------------------------------
 * Either convert the loop to frp and return 1 or give up and return 0
 */
static int
Lsuper_convert_to_frp (L_Inner_Loop *loop)
{
  L_Oper *op, *fb_op, *next_op, *new_cmp, *new_br;
  L_Cb *cb;
  L_Operand *gpr;
  int has_fallthrough;

  cb = loop->cb;

  fb_op = loop->feedback_op;

  for (op = cb->first_op; op; op = op->next_op)
    {
      if (op->pred[0])
	return 0;

      if (op == fb_op)
	break;
    }

  /* point of no return */

  has_fallthrough = L_has_fallthru_to_next_cb (cb);

  gpr = NULL;

  for (op = cb->first_op; op; op = next_op)
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
      if (op == fb_op)
	{
	  loop->feedback_op = new_br;
	  break;
	}
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


/*
 *      Valid if cb does not contain rts
 */
static int
Lsuper_valid_loop_for_unrolling (L_Func * fn, L_Inner_Loop * loop)
{
  L_Oper *oper, *next;
  L_Flow *flow;
  int schema = UNROLL_SCHEMA_NONE, is_swp;

  is_swp = L_EXTRACT_BIT_VAL (loop->cb->flags, L_CB_SOFTPIPE);

  if (is_swp && !Lsuper_unroll_pipelined_loops)
    return UNROLL_SCHEMA_NONE;

  for (oper = loop->cb->first_op; oper != NULL; oper = oper->next_op)
    if (L_subroutine_return_opcode (oper))
      return UNROLL_SCHEMA_NONE;

  if (is_swp && Lsuper_convert_to_frp (loop))
    L_warn ("Coverted loop fn %s cb %d to frp\n", fn->name, loop->cb->id);
  
  if (!is_swp && !L_is_predicated (loop->feedback_op))
    schema = UNROLL_SCHEMA_CONT;
#if 0
  else if (Lsuper_can_subjugate (loop))
    schema = UNROLL_SCHEMA_PRED;
#endif
  else
    return UNROLL_SCHEMA_NONE;

  next = loop->feedback_op->next_op;
  if (next && L_is_predicated (next))
    {
      flow = L_find_flow_for_branch (loop->cb, next);
      if (flow && flow->next_flow)
        {
          fprintf (stderr, "************ CB %d cannot be unrolled (2)\n",
                   loop->cb->id);
          return UNROLL_SCHEMA_NONE;
        }
    }

  if (Lsuper_debug_loop_unroll)
    fprintf (ERR, "loop (cb %d) is valid for unrolling\n", loop->cb->id);

  return schema;
}


/*=========================================================================
 *      Functions for loop optimizations
 *=========================================================================*/

static int
Lsuper_cb_contains_jsr (L_Cb * cb)
{
  L_Oper *oper;

  if (cb == NULL)
    return 0;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_subroutine_call_opcode (oper))
        return 1;
    }
  return 0;
}

static int
Lsuper_num_predicates (L_Cb * cb)
{
  Set preds = NULL;
  L_Oper *oper;
  int i, num_pred;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_general_pred_comparison_opcode (oper))
        continue;
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_is_register (oper->dest[i]))
            preds = Set_add (preds, oper->dest[i]->value.r);
        }
    }

  num_pred = Set_size (preds);
  preds = Set_dispose (preds);
  return (num_pred);
}

static ITintmax
Lsuper_really_calc_fixed_length (L_Oper * br_op,
                                 ITintmax start, ITintmax step)
{
  ITintmax end;
  ITintmax diff;
  ITintmax diff_mod_step;
  ITintmax diff_div_step;

  end = br_op->src[1]->value.i;
  diff = end - start;

#ifdef DEBUG_FIXED_LENGTH
  fprintf (stderr, "Start " ITintmaxformat ", End " ITintmaxformat 
	   ", Step " ITintmaxformat "\n", start, end, step);
#endif

  if (diff > 0)
    {
      if (step <= 0)
	return (0);

      diff_mod_step = diff % step;
      diff_div_step = diff / step;

      if (L_int_blt_branch_opcode (br_op))
        {
          if (diff_mod_step)
	    return (diff_div_step + 1);
          else
	    return (diff_div_step);
        }
      else if (L_int_ble_branch_opcode (br_op))
        {
          return (diff_div_step + 1);
        }
    }
  else
    {
      if (step >= 0)
	return (0);

      step = -step;
      diff = -diff;
      diff_mod_step = diff % step;
      diff_div_step = diff / step;

      if (L_int_bgt_branch_opcode (br_op))
        {
          if (diff_mod_step)
	    return (diff_div_step + 1);
          else
	    return (diff_div_step);
        }
      else if (L_int_bge_branch_opcode (br_op))
	return (diff_div_step + 1);
    }

  if (L_int_bne_branch_opcode (br_op))
    {
      if (diff_mod_step == 0)
	return (diff_div_step);
      else
	return (0);
    }

  return (0);
}

#ifndef DEBUG_FIXED_LENGTH
#define Lsuper_calc_fixed_length( cb, br_op, start, step ) \
        Lsuper_really_calc_fixed_length( br_op, start, step )
#else
static ITintmax
Lsuper_calc_fixed_length (L_Cb * cb, L_Oper * br_op,
                          ITintmax start, ITintmax step)
{
  L_Attr *attr;
  L_Operand *field;
  ITintmax length;

  length = Lsuper_really_calc_fixed_length (br_op, start, step);

  if (length)
    {
      /* Check if this is a fixed length for loop marked by the front end. */
      attr = L_find_attr (cb->attr, "LOOP");
      if (attr && (attr->max_field >= 3))
        {
          /* get the number of iterations that this loop will always execute */
          field = attr->field[2];
          if (L_is_int_constant (field))
            {
              if (length != field->value.i)
                {
                  fprintf (stderr, "Back-end fixed length loop has different "
                           "length than front end. (cb %d)\n", cb->id);
                  fprintf (stderr, "   BE: l=" ITintmaxformat 
			   ", FE l=" ITintmaxformat "\n",
                           length, field->value.i);
                }
            }
        }
      else
        {
          fprintf (stderr,
                   "Back-end found fixed length loop that front-end did "
                   "not. (cb %d)\n", cb->id);
        }
    }
  return (length);
}
#endif

/****************************************************************************
 *
 * routine: Lsuper_detect_fixed_length_loop()
 * purpose: Determine if the given loop executes a constant number of times
 *          each invocation.  These are usually for loops that go from 0
 *          to some constant.
 * input: fn - The function the loop is in.
 *        loop - the loop to test.
 * output: inc_op - If it is a fixed length loop, this is the loop increment
 *                  oper, otherwise undefined (maybe NULL).
 *         init_ops_set - Set of op ids which initialize the loop induction
 *                        variable.  Undefined if this is not a fixed length
 *                        loop.
 * returns: 0 if it is not a fixed length loop or there was a problem,
 *          otherwise the length of the fixed length loop is returned.
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

static int
Lsuper_detect_fixed_length_loop (L_Func * fn, L_Inner_Loop * loop,
                                 L_Oper ** inc_op,
                                 induction_init_ops_info
                                 init_ops[MAX_LOOP_COUNTER_INITS],
                                 int *number_init_ops)
{
  L_Oper *ind_op, *add_op;
  L_Flow *flow;
  ITintmax increment, start = 0;
  int start_not_found, reg_id, dest_reg, id, num_ind_ops;
  int ind_var_op_ids[MAX_INDUCTION_VAR_OPS_IN_LOOP];

  *inc_op = NULL;
  *number_init_ops = 0;

  if (!Lsuper_fixed_length_unroll)
    return (0);

  if (!L_int_cond_branch_opcode (loop->feedback_op) || !loop->fall_thru)
    {
#ifdef DEBUG_FIXED_LENGTH
      fprintf (stderr, "end of loop not a cond branch\n");
#endif
      return (0);
    }

  Lsuper_find_all_ind_info (loop, loop->cb);

#ifdef DEBUG_FIXED_LENGTH
  fprintf (stderr, "Cb %d, loop %d\n", loop->cb->id, loop->id);
  L_print_cb (stderr, fn, loop->cb);
  Set_print (stderr, "induction var", loop->basic_ind_var);
  Set_print (stderr, "induction var op", loop->basic_ind_var_op);
  fprintf (stderr, "Feedback op is %d\n", loop->feedback_op->id);
#endif

  reg_id = loop->feedback_op->src[0]->value.r;
  if (Set_in (loop->basic_ind_var, reg_id) &&
      L_is_int_constant (loop->feedback_op->src[1]))
    {
      /* The feedback op is a conditional branch whose sources for comparison
         are one of the induction variables and a constant.
         Now check to see if the induction variable in the feedback op
         is only altered once by an add which adds the induction variable
         to a constant. */
      num_ind_ops = Set_size (loop->basic_ind_var_op);
      if (num_ind_ops > MAX_INDUCTION_VAR_OPS_IN_LOOP)
        {
          L_punt
            ("Lsuper_detect_fixed_length_loop: Loop starting a cb %d has %d "
             "induction variables, which is more than "
             "MAX_INDUCTION_VAR_OPS_IN_LOOP", loop->cb->id, num_ind_ops);
        }
      Set_2array (loop->basic_ind_var_op, ind_var_op_ids);
      add_op = NULL;
      for (id = 0; id < num_ind_ops; id++)
        {
          ind_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
                                              ind_var_op_ids[id]);
          for (dest_reg = 0; dest_reg < L_max_dest_operand; dest_reg++)
            {
              if (L_is_register (ind_op->dest[dest_reg]) &&
                  ind_op->dest[dest_reg]->value.r == reg_id)
                {
                  if (add_op == NULL)
                    {
                      if (ind_op->pred[0])
                        {
                          /* The change in the induction variable is 
                             predicated, so it may not always happen. */
#ifdef DEBUG_FIXED_LENGTH
                          fprintf (stderr, "Predicated ind op\n");
#endif
                          L_delete_all_ind_info (&(loop->ind_info));
                          return (0);
                        }
                      add_op = ind_op;
                    }
                  else
                    {
                      /* Some other op has already altered this register.
                         Can't handle this now. */
                      /* add_op = NULL; */
#ifdef DEBUG_FIXED_LENGTH
                      fprintf (stderr, "Double assignment to ind var\n");
#endif
                      L_delete_all_ind_info (&(loop->ind_info));
                      return (0);
                    }
                }
            }
        }

      if (!add_op ||
          !L_int_add_opcode (add_op) ||
          !L_is_register (add_op->src[0]) ||
          !(add_op->src[0]->value.r == reg_id) ||
          !L_is_int_constant (add_op->src[1]))
        {
          L_delete_all_ind_info (&(loop->ind_info));
          return (0);
        }
      *inc_op = add_op;         /* output variable */

      /* This is the add op which alters the induction variable by a
         constant amount each iteration. */

      increment = add_op->src[1]->value.i;
      /* Find the initialization of the induction variable in the preheader. */
#ifdef DEBUG_FIXED_LENGTH
      fprintf (stderr, "Searching preheader cb %d\n", loop->preheader->id);
      L_print_cb (stderr, fn, loop->preheader);
#endif
      for (ind_op = loop->preheader->last_op;
           ind_op; ind_op = ind_op->prev_op)
        {
          for (dest_reg = 0; dest_reg < L_max_dest_operand; dest_reg++)
            {
              if (L_is_register (ind_op->dest[dest_reg]) &&
                  ind_op->dest[dest_reg]->value.r == reg_id)
                {
                  if (L_int_move_opcode (ind_op) &&
                      L_is_int_constant (ind_op->src[0]))
                    {
#ifdef DEBUG_FIXED_LENGTH
                      fprintf (stderr, "Fixed length loop detected!\n");
#endif
                      init_ops[*number_init_ops].cb = loop->preheader;
                      init_ops[*number_init_ops].op = ind_op;
                      (*number_init_ops)++;
                      L_delete_all_ind_info (&(loop->ind_info));
                      return (Lsuper_calc_fixed_length (loop->cb,
                                                        loop->feedback_op,
                                                        ind_op->src[0]->value.
                                                        i, increment));
                    }
                  else
                    {
                      /* induction variable initialization is not a constant */
#ifdef DEBUG_FIXED_LENGTH
                      fprintf (stderr, "Initialization in preheader is not a "
                               "constant\n");
#endif
                      L_delete_all_ind_info (&(loop->ind_info));
                      return (0);
                    }
                }
            }
        }

      /* at this point either the preheader was empty or no initialization was
         found in the preheader.
         Check the predecessors of the preheader for initialization of the
         induction variables.  All initializations have to be the same. */

      start_not_found = 1;
      for (flow = loop->preheader->src_flow; flow; flow = flow->next_flow)
        {
#ifdef DEBUG_FIXED_LENGTH
          fprintf (stderr, "Searching cb %d\n", loop->preheader->id);
#endif
          for (ind_op = flow->src_cb->last_op;
               ind_op && start_not_found; ind_op = ind_op->prev_op)
            {
              for (dest_reg = 0; dest_reg < L_max_dest_operand; dest_reg++)
                {
                  if (L_is_register (ind_op->dest[dest_reg]) &&
                      ind_op->dest[dest_reg]->value.r == reg_id)
                    {
                      if (L_int_move_opcode (ind_op) &&
                          L_is_int_constant (ind_op->src[0]))
                        {
#ifdef DEBUG_FIXED_LENGTH
                          fprintf (stderr, "Fixed length loop detected!\n");
#endif
                          if (start_not_found == 1)
                            {
                              /* first initialization encountered */
                              start = ind_op->src[0]->value.i;
                              start_not_found = 0;
                              init_ops[*number_init_ops].cb = loop->preheader;
                              init_ops[*number_init_ops].op = ind_op;
                              (*number_init_ops)++;
                              break;
                            }
                          else
                            {
                              if (start != ind_op->src[0]->value.i)
                                {
                                  /* There are two different initializations
                                     for this loop.  Can't handle this
                                     right now. */
                                  return (0);
                                }
                              start_not_found = 0;
                              init_ops[*number_init_ops].cb = loop->preheader;
                              init_ops[*number_init_ops].op = ind_op;
                              (*number_init_ops)++;
                              break;
                            }
                        }
                      else
                        {
                          /* induction variable initialization 
                             is not a constant */
#ifdef DEBUG_FIXED_LENGTH
                          fprintf (stderr,
                                   "Initialization of inducttion reg %d "
                                   "is not a constant in cb %d\n", reg_id,
                                   flow->src_cb->id);
#endif
                          L_delete_all_ind_info (&(loop->ind_info));
                          return (0);
                        }
                    }
                }
            }
          if (start_not_found)
            {
              /* one of the predecessors did not initialize the induction
                 variable.  So the length of the loop cannot be known. */
              return (0);
            }
          start_not_found = 2;
        }
      if (start_not_found == 2)
        {
          L_delete_all_ind_info (&(loop->ind_info));
          return (Lsuper_calc_fixed_length
                  (loop->cb, loop->feedback_op, start, increment));
        }
#ifdef DEBUG_FIXED_LENGTH
      fprintf (stderr, "Start not found is %d\n", start_not_found);
      L_print_cb (stderr, fn, fn->first_cb);
#endif
    }
#ifdef DEBUG_FIXED_LENGTH
  fprintf (stderr, "Exit at bottom\n");
#endif
  L_delete_all_ind_info (&(loop->ind_info));
  return (0);
}


/* Both x and y must be positive and non-zero.
   This is really inefficient */

static int
greatest_common_factor (int x, int y)
{
  int factor;

  if (x < y)
    {
      factor = x;
      x = y;
      y = factor;
    }
  /* x > y */
  factor = y;
  while (x % factor)
    {
      /* find a new factor of y */
      do
	factor--;
      while (y % factor);
    }
  return (factor);
}

/* 
 *      This is kinda a hokey function right now, its hard to
 *      set the number of unrolls intelligently!!
 */
static int
Lsuper_find_num_unroll (L_Inner_Loop * loop, int *fixed_length_loop)
{
  int num_unroll, num_op, num_pred, ave_iter, multiple = 0, mod, loop_length;
  L_Attr *attr;
  L_Operand *field;
  L_Cb *cb = loop->cb;

  /* First look for an attribute in which a divine force has told us
   * how many times to unroll the loop 
   */

  if ((attr = L_find_attr (cb->attr, "unroll_AMP")) &&
      (attr->max_field >= 3) &&
      (field = attr->field[2]) &&
      L_is_int_constant (field))
    {
      num_unroll = ITicast (field->value.i);
      if (num_unroll > Lsuper_max_unroll_allowed)
	L_punt ("Lsuper_find_num_unroll: preset unroll val exceeds "
		"max_unroll_allowed");
      return (num_unroll);
    }

  if (Lsuper_unroll_only_marked_loops)
    return (0);

  /* No preset val, start with average iteration count of the loop */

  ave_iter = (int) (loop->ave_iteration + 0.10);
  num_op = L_cb_size (loop->cb);
  num_unroll = ave_iter;

  /* now use a few heuristics to modify num_unroll */

  /* small loops just unroll more for heck of it */
  if ((Lsuper_allow_extra_unrolling_for_small_loops) &&
      (num_op <= L_MAX_OPER_FOR_SMALL_LOOP) &&
      (ave_iter < L_MIN_UNROLL_FOR_SMALL_LOOPS))
    num_unroll = L_MIN_UNROLL_FOR_SMALL_LOOPS;
  else if (Lsuper_allow_extra_unrolling_for_small_loops &&
           (num_op <= L_MAX_OPER_FOR_SMALL_LOOP) &&
           (loop->weight > L_CRIT_WEIGHT_FOR_UNROLL) &&
           (ave_iter >= 1.25 * L_CRIT_UNROLL_LIMIT) &&
           !Lsuper_cb_contains_jsr (loop->cb))
    num_unroll = L_CRIT_UNROLL_LIMIT;
  else if (num_unroll > L_UNROLL_LIMIT)
    num_unroll = L_UNROLL_LIMIT;

  loop_length = *fixed_length_loop;
  if (loop_length)
    {
      num_unroll <<= 1;
      if (num_unroll > Lsuper_max_unroll_allowed)
        num_unroll = Lsuper_max_unroll_allowed;
    }

  /* in general, do only 1/2 the unrolling for software pipelined loops 
   * because they don't need as much plus may have to do more unrolling
   * for modulo variable expansion 
   */
  if (L_EXTRACT_BIT_VAL (loop->cb->flags, L_CB_SOFTPIPE))
    {
      num_unroll = num_unroll / 2;
      /* If num_unroll = issue rate, for small high ave iter counted loops
       * back off one on unroll to allow slots for induction ops and
       * loop back branch that will not have num_unroll copies after
       * the remainder loop optimization.  Avoids wasting an MRT row for
       * just a couple opers. 
       */
      if ((Lsuper_allow_extra_unrolling_for_small_loops) &&
          (num_unroll == Lsuper_issue_rate) &&
          (num_op <= Lsuper_issue_rate) &&
          (num_op >= 4) &&
          (loop->weight > L_CRIT_WEIGHT_FOR_UNROLL) &&
          (ave_iter >= 1.25 * L_CRIT_UNROLL_LIMIT) &&
          L_find_attr (cb->attr, "COUNTED_LOOP") &&
          Lsuper_unroll_with_remainder_loop)
	num_unroll--;
    }

  /* check size constraints */
  if ((num_op * num_unroll) > Lsuper_max_unroll_op_count)
    {
      while (num_unroll > 1)
        {
          num_unroll--;
          if ((num_op * num_unroll) <= Lsuper_max_unroll_op_count)
            break;
        }
    }

  /* check number of predicate register constraint */
  num_pred = Lsuper_num_predicates (cb);
  if ((num_pred * num_unroll) > Lsuper_max_number_of_predicates)
    {
      while (num_unroll > 1)
        {
          num_unroll--;
          if ((num_pred * num_unroll) <= Lsuper_max_number_of_predicates)
            break;
        }
    }

  /* now check if there is a value the loop must be unrolled a multiple of */
  if ((attr != NULL) && (attr->max_field >= 2))
    {
      field = attr->field[1];
      if (L_is_int_constant (field))
        {
          multiple = ITicast (field->value.i);
          if (loop_length && multiple)
            multiple = greatest_common_factor (multiple, loop_length);

          mod = multiple % num_unroll;
          if (mod != 0)
            {
              while (num_unroll > 1)
                {
                  num_unroll--;
                  mod = multiple % num_unroll;
                  if (mod == 0)
                    break;
                }
            }
          return (num_unroll);
        }
    }
  else
    {
      if (loop_length)
	multiple = loop_length;
    }

  /* If num_unroll is larger the maximum allowed or
     num_unroll is larger the fixed loop iterations, 
     then reduce the amount of unroll. This should
     only be a fail safe check.  MCM corrected 02/2000 */
  if (multiple && num_unroll > multiple)
    num_unroll = multiple;
  if (num_unroll > Lsuper_max_unroll_allowed)
    num_unroll = Lsuper_max_unroll_allowed;

  return (num_unroll);
}


static void
Lsuper_reverse_control (L_Func * fn, L_Cb * cb, L_Oper * op,
                        L_Flow * flow, L_Flow * fall_thru_flow)
{
  if (L_cond_branch_opcode (op))
    {
      if (fall_thru_flow == 0)
        L_punt ("Lsuper_reverse_control: fall_thru_flow should not be NIL");
      flow->dst_cb = fall_thru_flow->dst_cb;
      flow->weight = fall_thru_flow->weight;
      L_negate_compare (op);
      op->src[2]->value.cb = fall_thru_flow->dst_cb;
    }
  else if (L_uncond_branch_opcode (op))
    {
      if (!op->pred[0] || !fall_thru_flow)
        {
          L_delete_oper (cb, op);
          cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
        }
      else
        {
          int regid;
          L_Oper *invert_op;

          invert_op = L_create_new_op (Lop_PRED_COMPL);
          invert_op->pred[0] = L_copy_operand (op->pred[0]);
          invert_op->dest[0] = L_copy_operand (op->pred[0]);
          L_assign_ptype_uncond_true (invert_op->dest[0]);
          regid = ++(fn->max_reg_id);
          op->pred[0]->value.pred.reg = regid;
          invert_op->dest[0]->value.pred.reg = regid;
          L_insert_oper_before (cb, op, invert_op);

          flow->dst_cb = fall_thru_flow->dst_cb;
          flow->weight = fall_thru_flow->weight;

          op->src[0]->value.cb = fall_thru_flow->dst_cb;
        }
    }
  else
    {
      L_punt ("Lsuper_reverse_control: illegal op %d ", op->id);
    }
}


static void
Lsuper_predicate_rename (L_Func * fn, L_Oper * copy)
{
  int j, pred_reg;

  /* rename predicate field */

  if (copy->pred[0])
    {
      pred_reg = copy->pred[0]->value.r;

      if (L_pred_map[pred_reg] >= 0)
        copy->pred[0]->value.r = L_pred_map[pred_reg];
    }

  for (j = 0; j < L_max_src_operand; j++)
    {
      if (!L_is_register (copy->src[j]))
        continue;
      if (L_is_ctype_predicate (copy->src[j]))
        {
          pred_reg = copy->src[j]->value.r;
          if (L_pred_map[pred_reg] < 0)
            L_pred_map[pred_reg] = ++(fn->max_reg_id);
          copy->src[j]->value.r = L_pred_map[pred_reg];
        }
    }

  /* Rename predicates ONLY when a total write is seen in the loop. */

  if (L_pred_define_opcode (copy))
    {
      for (j = 0; j < L_max_dest_operand; j++)
        {
	  int ptype;

          if (!copy->dest[j])
            continue;

	  ptype = copy->dest[j]->ptype;
          pred_reg = copy->dest[j]->value.r;

	  if (!L_is_transparent_predicate_ptype (ptype))
	    L_pred_map[pred_reg] = ++(fn->max_reg_id);

	  if (L_pred_map[pred_reg] >= 0)
	    copy->dest[j]->value.r = L_pred_map[pred_reg];
        }
    }
  else if (copy->opc == Lop_PRED_ST)
    {
      pred_reg = copy->src[2]->value.r;
      if (L_pred_map[pred_reg] < 0)
        L_pred_map[pred_reg] = ++(fn->max_reg_id);
      copy->src[2]->value.r = L_pred_map[pred_reg];
    }

  return;
}


static void
Lsuper_do_unroll (L_Func * fn, L_Inner_Loop * loop, int num_unroll,
		  int schema,
                  int fixed_length_loop, L_Operand * side_induction_var)
{
  int n_branch, i, j;
  L_Cb *cb;
  L_Oper *op, *final_jump_op = NULL, *after_clears = NULL, *clear_op;
  L_Operand *squash_pred = NULL;
  L_Flow *fall_thru_flow, *feed_back_flow, *new_flow, *flow;
  L_Attr *attr, *attr2;

  if (Lsuper_debug_loop_unroll)
    fprintf (ERR, "Unroll loop %d (cb %d) %d times by schema %d\n",
	     loop->id, loop->cb->id, num_unroll, schema);

  cb = loop->cb;
  n_branch = 0;

  /* Determine no. of branches in loop body */
  for (op = cb->first_op; op != NULL; op = op->next_op)
    if (L_cond_branch_opcode (op) || L_uncond_branch_opcode (op))
      n_branch++;

  if (loop->fall_thru)
    {
      fall_thru_flow = L_find_last_flow (cb->dest_flow);
      feed_back_flow = L_find_second_to_last_flow (cb->dest_flow);

      if (loop->feedback_op != cb->last_op)
        {
          if (!L_uncond_branch_opcode (cb->last_op))
            L_punt ("Lsuper_do_unroll: fallthru must be uncond branch");
          final_jump_op = cb->last_op;
          L_remove_oper (cb, final_jump_op);
        }

      cb->dest_flow = L_remove_flow (cb->dest_flow, fall_thru_flow);
    }
  else
    {
      fall_thru_flow = NULL;
      feed_back_flow = L_find_last_flow (cb->dest_flow);
    }

  L_last_flow[0] = feed_back_flow;
  L_last_br[0] = cb->last_op;

  if (fixed_length_loop)
    {
      /* Scale the flow weights */
      double side_exit_weight = 0.0;
      for (flow = cb->dest_flow; flow != feed_back_flow;
           flow = flow->next_flow)
        {
          flow->weight /= num_unroll;
          side_exit_weight += flow->weight;
        }
      cb->weight /= num_unroll;
      feed_back_flow->weight = cb->weight - fall_thru_flow->weight
        - side_exit_weight;
    }
  else
    {
      /* Scale the flow weights */

      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
	flow->weight /= num_unroll;

      if (fall_thru_flow != NULL)
        fall_thru_flow->weight /= num_unroll;

      cb->weight -= (feed_back_flow->weight * (num_unroll - 1));
    }

  /* Insert iteration attributes */
  for (op = cb->first_op; op != 0; op = op->next_op)
    {
      attr = L_new_attr ("iter", 1);
      L_set_int_attr_field (attr, 0, 1);
      op->attr = L_concat_attr (op->attr, attr);
    }

  if (schema == UNROLL_SCHEMA_PRED)
    squash_pred = L_last_br[0]->pred[0];

  /* Duplicate loop body N times */
  new_flow = NULL;

  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    for (j = 0; j <= max_reg; j++)
      L_pred_map[j] = -1;

  for (i = 1; i < num_unroll; i++)
    {
      L_Oper *copy;

      for (op = cb->first_op; op; op = op->next_op)
        {
	  if ((op == L_last_br[0]) && (i < (num_unroll - 1)))
	    {
	      if (fixed_length_loop)
		{
		  /* if this is a fixed length loop, only copy the branch 
		     back to the top of the loop on the last iteration */
		  if (side_induction_var)
		    {
		      /* establish the side exit induction variable */
		      copy = L_create_new_op (Lop_ADD);
		      copy->dest[0] = L_copy_operand (side_induction_var);
		      copy->src[0] = L_copy_operand (side_induction_var);
		      copy->src[1] = L_new_gen_int_operand (-1);
		      L_insert_oper_after (cb, cb->last_op, copy);
		    }
		  break;
		}
	      else if (schema == UNROLL_SCHEMA_PRED)
		{
		  squash_pred = op->pred[0];
		  break;
		}
	    }

          copy = L_copy_operation (op);

	  if (squash_pred && (schema == UNROLL_SCHEMA_PRED) && !copy->pred[0])
	    copy->pred[0] = L_copy_operand (squash_pred);

          /* Update iteration number of unrolled loop */
          if ((attr = L_find_attr (copy->attr, "iter")))
            L_set_int_attr_field (attr, 0, (i + 1));

          /* rename pred_define, pred_clear, and pred_ld/st  */
          if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
            Lsuper_predicate_rename (fn, copy);

          L_insert_oper_after (cb, cb->last_op, copy);

          if (L_cond_branch_opcode (op) || L_uncond_branch_opcode (op))
            {
              L_Flow *old, *new;

              old = L_find_flow_for_branch (cb, op);

              new = L_new_flow (old->cc, old->src_cb, old->dst_cb,
                                old->weight);
              new_flow = L_concat_flow (new_flow, new);
              if (op == L_last_br[0])
                {
                  L_last_flow[i] = new;
                  L_last_br[i] = copy;
                }
            }

          if (op == L_last_br[0])
	    break;
        }

      if (!op)
        L_punt ("Lsuper_do_unroll: loop br not found in cb!");
    }

  if (schema == UNROLL_SCHEMA_PRED)
    {
      if (L_last_br[0])
	L_delete_oper (cb, L_last_br[0]);
      if (L_last_flow[0])
	cb->dest_flow = L_delete_flow (cb->dest_flow, L_last_flow[0]);
    }

  /* DMG - dependence distance has been changed by unrolling */
  if (L_func_contains_dep_pragmas || L_func_acc_omega)
    L_adjust_syncs_after_unrolling (cb, num_unroll);

  cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);

  if (fall_thru_flow != 0)
    cb->dest_flow = L_concat_flow (cb->dest_flow, fall_thru_flow);

  /* now move all pred_clears up to top of block */
  /* we find first pred clear; then we step down we are not on
     a pred clear (ie first inst after first block of pred clear);
     then we search remaining opers for clears and move them to 
     before this instruction */

  op = cb->first_op;

  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    {
      while (op && (op->opc != Lop_PRED_CLEAR))
        op = op->next_op;

      while (op && (op->opc == Lop_PRED_CLEAR))
        op = op->next_op;

      if (op)
        {
          after_clears = op;
          op = op->next_op;
        }

      while (op)
        {
          if (op->opc == Lop_PRED_CLEAR)
            {
              clear_op = op;
              op = op->next_op;
              L_remove_oper (cb, clear_op);
              L_insert_oper_before (cb, after_clears, clear_op);
            }
          else
	    {
	      op = op->next_op;
	    }
        }
    }

  /* Reverse branch directions for backedges */

  if (!fixed_length_loop && (schema != UNROLL_SCHEMA_PRED))
    {
      for (i = 0; i < (num_unroll - 1); i++)
        {
          /* This is for Scott's branch classification info that is put in
             during Lopti, just do a little simple manipulation for the stuff
             that makes sense.  If branch classifications are there, change
             loop back marking to loop exit */
          if (L_cond_branch_opcode (L_last_br[i]))
            {
              attr = L_find_attr (L_last_br[i]->attr,
                                  L_BR_LOOPBACK_INNER_NAME);
              if (attr)
                {
                  attr2 = L_find_attr (L_last_br[i]->attr,
                                       L_BR_LOOPEXIT_INNER_NAME);
                  if (!attr2)
                    {
                      attr2 = L_new_attr (L_BR_LOOPEXIT_INNER_NAME, 0);
                      L_last_br[i]->attr = L_concat_attr (L_last_br[i]->attr,
                                                          attr2);
                    }
                  L_last_br[i]->attr = L_delete_attr (L_last_br[i]->attr,
                                                      attr);
                }
              attr = L_find_attr (L_last_br[i]->attr,
                                  L_BR_LOOPBACK_OUTER_NAME);
              if (attr)
                {
                  attr2 = L_find_attr (L_last_br[i]->attr,
                                       L_BR_LOOPEXIT_OUTER_NAME);
                  if (!attr2)
                    {
                      attr2 = L_new_attr (L_BR_LOOPEXIT_OUTER_NAME, 0);
                      L_last_br[i]->attr = L_concat_attr (L_last_br[i]->attr,
                                                          attr2);
                    }
                  L_last_br[i]->attr = L_delete_attr (L_last_br[i]->attr,
                                                      attr);
                }
            }

          Lsuper_reverse_control (fn, cb, L_last_br[i], L_last_flow[i],
                                  fall_thru_flow);
        }

      if (final_jump_op)
        L_insert_oper_after (cb, cb->last_op, final_jump_op);

      /* mark as unrolled loop! */
      cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_UNROLLED);
      if (!(attr = L_find_attr (cb->attr, "unroll_AMP")))
        {
          attr = L_new_attr ("unroll_AMP", 3);
          cb->attr = L_concat_attr (cb->attr, attr);
        }
      L_set_int_attr_field (attr, 0, num_unroll);
    }
}



/****************************************************************************
 *
 * routine: Detect_side_exits()
 * purpose: Determine if there is side exist which is a tail duplicate of
 *          the loop feedback.  If such a situation exists, check if a
 *          recovery loop can used to recover from the side exit.
 * input:
 * output: side_induction_var - If a side exit recovery loop is created,
 *              this is the operand which is now the induction variable that
 *              controls the recovery loop.  This is owned by another oper so
 *              make your own copy.  Otherwise, it is undefined.
 *              So it is only valid if the return value is 1.
 * returns: 0 if there is a problem and fixed length loops should be aborted.
 *          1 if there is a side exit that is a tail duplicate of the loop
 *            feedback.
 *          2 if there are no side exits.
 * modified:
 * note: This should only be called for fixed length loops.
 *-------------------------------------------------------------------------*/

static int
Lsuper_detect_side_exits (L_Func * fn, L_Inner_Loop * loop, L_Oper * inc_op)
{
  L_Cb *cb;
  L_Cb *tail_cb;
  L_Oper *op;
  L_Operand *ind_var;
  L_Flow *flow;
  L_Flow *src_flow;
  L_Flow *dest_flow;
  int dest_reg;
  int ind_var_change;
  int br_num;
  int inc_op_pos = 0;
  int re_entry_br;
  int re_entry_flow_count;

  cb = loop->cb;

  /* determine if there are any side exits to code that has tail duplicated
     the bottom of the loop.  This code will loop back to the top of this
     loop.  This code needs special attention.
     There is a branch that is not the final jump nor the feedback.
     This could be an early exit or it could be to a side case which
     then loops back (resulting from tail duplication).
     First check the predecessors to the loop header.  If there are only
     two predecessors, then they must be the preheader and the loop itself.
     In this case there is not need to worry. */

  /* Count the source flows. */
  for (flow = cb->src_flow, re_entry_flow_count = 0;
       flow; flow = flow->next_flow, re_entry_flow_count++);
  /* Now subtract 2, one for the feedback and one for the preheader. */
  re_entry_flow_count -= 2;

  if (re_entry_flow_count == 0)
    {
      return (2);
    }

  /* Multiple entries to the loop exist.
     Look at each of these side-exit branches to determine their
     functionality.  The branch class is still left from Lopti. */

  re_entry_br = 0;
  for (op = cb->first_op; op != loop->feedback_op; op = op->next_op)
    {
      if (L_cond_branch_opcode (op) || L_uncond_branch_opcode (op))
        {
          if (L_find_attr (op->attr, "LE_inner") ||
              L_find_attr (op->attr, "LE_outer"))
            {
              /* these are exits from the loop.  If they go to loop
                 entry it will be for another invocation of the loop,
                 and not for another iteration. No need to worry about
                 these. */
              continue;
            }
          if (L_find_attr (op->attr, "LB_inner") ||
              L_find_attr (op->attr, "LB_outer"))
            {
              /* This should not happen in a fixed length loop.
                 Abort fixed length loop. */
#ifdef DEBUG
              fprintf (stderr, "Abort FLL, LB found\n");
#endif
              return (0);
            }
          /* This probably is a branch to some code that has a
             tail duplication of the rest of the loop attached to it.
             This needs special recovery code. */
          re_entry_br++;
        }
      else
        {
          if (inc_op == op)
            {
              /* This is the induction variable increment.  We need to remember
                 how many side exits occured above and below it. */
              inc_op_pos = re_entry_br;
            }
        }
    }

  if (re_entry_br != re_entry_flow_count)
    {
      return (0);
    }

  ind_var = inc_op->dest[0];

  /* At this point, there exist a side exit that is a tail duplicate of the
     loop feedback.  This requires fix up code.
     Mark the source flows that come from the tail duplicate. */

  for (flow = cb->src_flow; flow; flow = flow->next_flow)
    {
      tail_cb = flow->src_cb;
      if ((tail_cb == loop->preheader) || (tail_cb == cb))
        {
          flow->flags &= FLOW_FEEDBACK_CLEAR;
        }
      else
        {
          flow->flags |= FLOW_FEEDBACK;
#ifdef DEBUG
          fprintf (stderr, "(1)feedback flow from cb %d to cb %d\n",
                   flow->src_cb->id, cb->id);
#endif
          /* Walk the cb to see if induction variable is altered in the same
             way as in the main loop. */
          ind_var_change = 0;

          do
            {
              for (op = tail_cb->last_op; op; op = op->prev_op)
                {
                  for (dest_reg = 0; dest_reg < L_max_dest_operand;
                       dest_reg++)
                    {
                      if (L_same_operand (ind_var, op->dest[dest_reg]))
                        {
                          if (ind_var_change)
                            {
                              /* It appears that the induction variable is 
                                 being altered twice in the side exit.  Abort 
                                 fixed length loop. */
                              return (0);
                            }
                          if (!L_same_operation (inc_op, op, 0))
                            {
                              /* Alteration of induction variable is different
                                 here than in the main loop. */
                              return (0);
                            }
                          ind_var_change = 1;
                          break;
                        }
                    }
                }
              src_flow = tail_cb->src_flow;
              if (src_flow->next_flow)
                {
                  /* more than one predecessor */
                  return (0);
                }
              tail_cb = src_flow->src_cb;
            }
          while (tail_cb != cb);

          L_punt ("This appears to be broken.  See RDB.");
          /* Match the source flow that got us here 
             to one of the dest flows. */
          for (dest_flow = cb->dest_flow, br_num = 1;
               dest_flow && (dest_flow->dst_cb != src_flow->dst_cb) &&
               (dest_flow->weight != src_flow->weight);
               dest_flow = dest_flow->next_flow, br_num++);

          /* check to make sure that the induction var is altered the same
             number of times when control stays in the main loop as when the
             side exit is taken. */

          if (flow)
            {
              if (inc_op_pos >= br_num)
                {
                  if (!ind_var_change)
                    {
                      return (0);
                    }
                }
              else
                {
                  if (ind_var_change)
                    {
                      return (0);
                    }
                }
            }
        }
    }
  return (1);
}


/****************************************************************************
 *
 * routine: Lsuper_copy_cb()
 * purpose: Make a copy of the cb with all its ops and dest flows.
 *          The dest flows are copied and the src_cb's are altered along
 *          with any feedback branch in the cb.  However, source flows are
 *          not copied.
 * input: fn - function which own s the cb to copy
 *        cb - block to copy
 * output: 
 * returns: pointer to the new cb
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

static L_Cb *
Lsuper_copy_cb (L_Func * fn, L_Cb * cb)
{
  L_Cb *new_cb;
  L_Flow *flow;
  L_Oper *br_op;

  new_cb = L_create_cb (cb->weight);
  L_copy_block_contents (cb, new_cb);
  new_cb->dest_flow = L_copy_flow (cb->dest_flow);
  new_cb->attr = L_copy_attr (cb->attr);

  for (flow = new_cb->dest_flow; flow; flow = flow->next_flow)
    {
      /* Change the source cb. */
      flow->src_cb = new_cb;
      if (flow->dst_cb == cb)
        {
          /* cb branches to itself. */
          br_op = L_find_branch_for_flow (new_cb, flow);
          L_change_branch_dest (br_op, cb, new_cb);
          flow->dst_cb = new_cb;
        }
    }
  return (new_cb);
}

/****************************************************************************
 *
 * routine: Make_side_exit_loop()
 * purpose: Make a copy of the original loop.  The original loop will be
 *          a fixed length loop so when a side exit is taken we need to
 *          synchronize the re-entrance to the top of the loop.  This loop
 *          copy will be executed after the side exit with tail duplication
 *          is executed.
 * input:
 * output: 
 * returns: The operand which is the induction variable of this new loop.
 *          This is just a pointer.  Make your own copy.
 *          NULL if something bad happened.
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

static L_Operand *
Lsuper_make_side_exit_loop (L_Func * fn, L_Inner_Loop * loop, int num_unroll)
{
  L_Cb *org_cb;
  L_Cb *new_cb;
  L_Cb *new_preheader;
  L_Flow *flow;
  L_Flow *dest_flow;
  L_Flow *next_flow;
  L_Oper *br_op;
  L_Oper *op;
  L_Operand *new_induction_var;
  int roll;
  double org_entry_weight;
  double taken_weight;
  double fall_thru_weight;
  double ratio;
  double eff_exit_weight;
  double eff_cb_weight;
  double side_exit_weight;
  double jump_to_org_weight;

  org_cb = loop->cb;
  new_cb = Lsuper_copy_cb (fn, org_cb);

#ifdef DEBUG
  fprintf (stderr, "Copy of cb %d is cb %d\n", org_cb->id, new_cb->id);
#endif

  /* make a new preheader that will be infront of the new cb loop. */

  new_preheader = L_create_cb (0.0);
  br_op = L_create_new_op (Lop_BR);
  new_induction_var = L_new_register_operand (++(fn->max_reg_id),
                                              L_native_machine_ctype,
                                              L_PTYPE_NULL);
  br_op->src[0] = new_induction_var;
  L_set_compare (br_op, br_op->src[0]->ctype, Lcmp_COM_LE);
  br_op->src[1] = L_new_gen_int_operand (0);
  br_op->src[2] = L_new_cb_operand (org_cb);

  L_insert_oper_after (new_preheader, new_preheader->last_op, br_op);

  /* find the first cb that is a tail duplicate of the loop feedback.
     Put the preheader and the new cb after this one. */
  for (flow = org_cb->src_flow;
       !(flow->flags & FLOW_FEEDBACK); flow = flow->next_flow);
  L_insert_cb_after (fn, flow->src_cb, new_preheader);
  L_insert_cb_after (fn, new_preheader, new_cb);

  /* Delete the source flows in the original cb that came from tail dups and
     put them on the new preheader.
     Updating the source flows is not really needed.  But we do need the
     weight. */
  org_entry_weight = 0.0;
  for (flow = org_cb->src_flow; flow; flow = next_flow)
    {
      next_flow = flow->next_flow;
      if (flow->flags & FLOW_FEEDBACK)
        {
          /* This is a flow from a side exit */
          org_entry_weight += flow->weight;
          org_cb->src_flow = L_remove_flow (org_cb->src_flow, flow);
          new_preheader->src_flow = L_concat_flow (flow,
                                                   new_preheader->src_flow);

          /* Goto the flow source cb and change both the branch and the dest
             flow so that it points to the new preheader. */
          dest_flow = flow->src_cb->dest_flow;

          for (op = flow->src_cb->first_op; op; op = op->next_op)
            {
              if (L_general_branch_opcode (op))
                {
                  L_change_branch_dest (op, org_cb, new_preheader);
                  dest_flow->dst_cb = new_preheader;
#ifdef DEBUG
                  fprintf (stderr,
                           "Changing branch dest of op %d in cb %d from "
                           "cb %d to cb %d\n", op->id, flow->src_cb->id,
                           org_cb->id, new_cb->id);
#endif
                  dest_flow = dest_flow->next_flow;
                }
            }
        }
    }

  /* Calculate the weight for each side exit on the new cb.
     There must be atleast 3 flows at this point;  One for fall thru, one for
     the feedback branch, and one for each side exit.  If there are
     no side exits then we would not be in this routine.
     So the side exit weight can be found by summing all but the last 2 flows.
     After the loop flow will point to the feedback flow.  */

  side_exit_weight = 0.0;

  for (flow = new_cb->dest_flow;
       flow->next_flow->next_flow; flow = flow->next_flow)
    {
      eff_cb_weight = new_cb->weight;
      eff_exit_weight = flow->weight / num_unroll;
      ratio = flow->weight / org_cb->weight;
      flow->weight = flow->weight - eff_exit_weight;
      for (roll = 1; roll < num_unroll; roll++)
        {
          eff_cb_weight -= eff_exit_weight;
          eff_exit_weight = eff_cb_weight * ratio;
          flow->weight -= eff_exit_weight;
        }
      side_exit_weight += flow->weight;
    }

  /* weight out of the preheader to the org loop */
  taken_weight = (1.0 / num_unroll) * org_entry_weight + side_exit_weight;
  /* weight out of the preheader to the loop copy */
  fall_thru_weight = org_entry_weight - taken_weight;
  /* jump back to the original cb from the bottom of the new cb loop */
  jump_to_org_weight = org_entry_weight - taken_weight;

  new_preheader->weight = org_entry_weight;

  new_preheader->dest_flow = L_concat_flow (L_new_flow (1, new_preheader,
                                                        org_cb,
                                                        taken_weight),
                                            L_new_flow (0, new_preheader,
                                                        new_cb,
                                                        fall_thru_weight));
  /* add a source flow from the preheader to the org cb */
  org_cb->src_flow = L_concat_flow (L_new_flow (1, new_preheader, org_cb,
                                                taken_weight),
                                    org_cb->src_flow);
  /* At this point, the new cb does not have any source flows.
     Add a source flow from the preheader to the new cb and one from the
     new cb to itself. */
  new_cb->src_flow = L_concat_flow (L_new_flow (1, new_preheader, new_cb,
                                                fall_thru_weight),
                                    L_new_flow (1, new_cb, new_cb,
                                                fall_thru_weight
                                                - side_exit_weight));

  new_cb->weight = fall_thru_weight;
  /* feedback branch */
  flow->weight = fall_thru_weight - jump_to_org_weight - side_exit_weight;
  /* jump back to the original cb from the new cb */
  flow = flow->next_flow;
  flow->weight = jump_to_org_weight;
  flow->src_cb = new_cb;
  flow->dst_cb = org_cb;
  /* add a source flow from the new_cb to the org_cb */
  org_cb->src_flow = L_concat_flow (org_cb->src_flow,
                                    L_copy_single_flow (flow));

  if (loop->fall_thru)
    {
      if (loop->feedback_op != org_cb->last_op)
        {
          /* The original cb ends in a jump.  Since the new cb is a copy,
             it also ends in a jump.  Therefore redirect the jump in the new cb
             to the original cb. */
          if (L_uncond_branch_opcode (org_cb->last_op))
            {
              new_cb->last_op->src[0]->value.cb = org_cb;
            }
          else
            {
              L_punt
                ("Lsuper_make_side_exit_loop: fallthru must be uncond branch");
            }
        }
      else
        {
          br_op = L_create_new_op (Lop_JUMP);
          br_op->src[0] = L_new_cb_operand (org_cb);
          L_insert_oper_after (new_cb, new_cb->last_op, br_op);
        }
    }
  else
    {
      L_punt
        ("Lsuper_make_side_exit_loop: No fall thru on a fixed length loop.");
    }

  /* Change the loop count and branch condition so that this loop copy
     is only executed enough times to sync up with the fixed length loop.
     For example: If fixed_length loop is unroll 4 times, and the
     side exits occurs on iter 2 in the unrolled loop, then
     the loop copy should only execute 2 (4-2) times.
     Then the fixed length loop should be resumed.
     To accomplish this, the branch condition before the loop must be altered.
     It is assumed that it is a conditional integer branch, otherwise we
     would not be here. */

  br_op = new_cb->last_op->prev_op;
  L_set_compare_type (br_op, Lcmp_COM_GT);
  L_delete_operand (br_op->src[0]);
  br_op->src[0] = L_copy_operand (new_induction_var);
  L_delete_operand (br_op->src[1]);
  br_op->src[1] = L_new_gen_int_operand (0);

  op = L_create_new_op (Lop_ADD);
  op->dest[0] = L_copy_operand (new_induction_var);
  op->src[0] = L_copy_operand (new_induction_var);
  op->src[1] = L_new_gen_int_operand (-1);
  L_insert_oper_before (new_cb, br_op, op);

  return (new_induction_var);
}

/*===========================================================================*/

void
Lsuper_loop_unrolling (L_Func * fn)
{
  L_Inner_Loop *loop;
  L_Operand *side_induction_var = NULL;
  L_Oper *inc_op;
  Set init_ops_set = NULL;
  int num_unroll, fixed_length_loop, side_exit, number_init_ops, schema;
  induction_init_ops_info induction_init_ops[MAX_LOOP_COUNTER_INITS];

  L_compute_oper_weight (fn, 0, 1);

  L_last_flow =
    (L_Flow **) Lcode_malloc (sizeof (L_Flow *) * Lsuper_max_unroll_allowed);
  L_last_br =
    (L_Oper **) Lcode_malloc (sizeof (L_Oper *) * Lsuper_max_unroll_allowed);
  L_pred_map = (int *) Lcode_malloc (sizeof (int) * (fn->max_reg_id + 1));

  max_reg = fn->max_reg_id;

  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {
      if (loop->weight < L_MIN_WEIGHT_FOR_UNROLL)
        continue;

      /*
       * Fixed length loops
       */

      fixed_length_loop = Lsuper_detect_fixed_length_loop (fn, loop,
                                                           &inc_op,
                                                           induction_init_ops,
                                                           &number_init_ops);

      if (number_init_ops > MAX_LOOP_COUNTER_INITS)
	L_punt ("Lsuper_loop_unrolling: Number of induction variable "
		"initialization ops (%d) exceeds "
		"MAX_LOOP_COUNTER_INITS\n", number_init_ops);

      num_unroll = Lsuper_find_num_unroll (loop, &fixed_length_loop);

      if (num_unroll < 2)
	continue;

      if (!(schema = Lsuper_valid_loop_for_unrolling (fn, loop)))
        continue;

      STAT_COUNT ("L_super_loop_unrolling", num_unroll, loop->cb);

      if (fixed_length_loop && (schema == UNROLL_SCHEMA_CONT))
        {
          side_exit = Lsuper_detect_side_exits (fn, loop, inc_op);
          switch (side_exit)
            {
            case 0:
              side_induction_var = NULL;
              fixed_length_loop = 0;
              break;
            case 1:
              side_induction_var = Lsuper_make_side_exit_loop (fn, loop,
                                                               num_unroll);
              break;
            case 2:
              side_induction_var = NULL;
	      break;
	    default:
	      break;
            }
        }
      Set_dispose (init_ops_set);
      init_ops_set = NULL;

      L_clear_src_flow (fn);
      Lsuper_do_unroll (fn, loop, num_unroll, schema, fixed_length_loop,
                        side_induction_var);
      L_rebuild_src_flow (fn);
    }

  Lcode_free (L_last_flow);
  Lcode_free (L_last_br);
  Lcode_free (L_pred_map);
}
/*===========================================================================*/
