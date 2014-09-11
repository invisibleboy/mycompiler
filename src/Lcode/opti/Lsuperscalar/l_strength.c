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
 *      File :          l_strength.c
 *      Description :   Strength reduction optimization
 *      Creation Date : July 1993
 *      Author :        John Gyllenhaal Scott Mahlke
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

/*
 *      Currently only strength reduction of multipy is done to
 *      a sequence of shifts and adds!!
 */

/* Optimization switches */
#define DO_MUL_STR_RED
#undef DO_DIV_STR_RED

/* Debugging switches */
#define DEBUG_MUL_STR_RED       Lsuper_debug_str_red
#define DEBUG_DIV_STR_RED       0

/* 10/22/04 REK max is also defined in Lmarkpipe/l_markpipe.h. */
#ifndef max
#define max(a,b) (((a) < (b)) ? (b) : (a))
#endif

#define ERR                     stderr
#define ISSUE_RATE              8
#define STR_RED_MUL_LIMIT       3
#define STR_RED_DIV_LIMIT       10

#define QUEUE_SIZE              max(STR_RED_MUL_LIMIT, STR_RED_DIV_LIMIT)

#define NO                      0
#define YES                     1
#define GEN_CODE                0
#define COUNT_ONLY              1



/* Queue for add and sub operations for "add" tree implementation */
static struct
{
  L_Operand *src;
  char op;
}
arth_queue[QUEUE_SIZE];

/*
 * Holds the first divide reduction fixup code cb generated to
 * prevent fixup code generated from the div reduction from being
 * reduced again.
 * Should be initialized to -1 and tested by L_divide_strength_reduce caller.
 */
int L_first_div_fixup;

static int bits_in_int = sizeof (int) * 8;
static int shift_ct;
static int arth_ct;
static int negate;


/*=======================================================================*/
/*
 *      Internal functions
 */
/*=======================================================================*/

/*
 * Return the number of continuous 1 bits starting at i, and
 * extending to the right of i.
 */
static int
L_get_group_len (ITintmax num, int i)
{
  int cnt;
  cnt = 0;

  while ((i >= 0) && ((num & (ITintmax) (1 << i)) != 0))
    {
      cnt++;
      i--;
    }
  return (cnt);
}

/* Queue an subtract operation for the add/sub tree */
static void
L_queue_add (int flag, L_Operand * src)
{
  if (flag == GEN_CODE)
    {
      if (arth_ct >= QUEUE_SIZE)
        L_punt ("L_queue_add:  Storage overflow.");
      else
        {
          arth_queue[arth_ct].src = src;
          arth_queue[arth_ct].op = Lop_ADD;
        }
    }
  arth_ct++;
}

/* Queue an subtract operation for the add/sub tree */
static void
L_queue_sub (int flag, L_Operand * src)
{
  if (flag == GEN_CODE)
    {
      if (arth_ct >= QUEUE_SIZE)
        L_punt ("L_queue_sub: Storage overflow.");
      else
        {
          arth_queue[arth_ct].src = src;
          arth_queue[arth_ct].op = Lop_SUB;
        }
    }
  arth_ct++;
}

/* Insert a shift instruction */
static L_Operand *
L_do_lshift (int flag, L_Operand * src, int cnt, L_Cb * cb, L_Oper * repl_op)
{
  int c;
  L_Oper *new_op;
  shift_ct++;

  /* Just fall through if just counting number of operations */
  if (flag == GEN_CODE)
    {
      new_op = L_create_new_op (Lop_LSL);
      new_op->weight = repl_op->weight;
      new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                L_native_machine_ctype,
                                                L_PTYPE_NULL);
      new_op->src[0] = L_copy_operand (src);
      new_op->src[1] = L_new_gen_int_operand (cnt);

      /* copy the predicates of repl_op */
      for (c = 0; c < L_max_pred_operand; c++)
        {
          new_op->pred[c] = L_copy_operand (repl_op->pred[c]);
        }

      L_insert_oper_before (cb, repl_op, new_op);
      return (new_op->dest[0]);
    }
  else
    return (NULL);
}

/*
 * Generate add/sub tree from queued operations.
 * Implicitely it is assumed the result of all the adds will
 * goto the first queued operations register.
 * The only register that will not be destroyed by this is the
 * last source register queued (Thus this is the only place
 * the mult source register may be placed).
 * This can be modified to allocate new destination registers if
 * this turns out to be a problem.
 *
 */


static void
L_do_addsub_tree (L_Cb * cb, L_Oper * repl_op)
{
  int i, j, c;
  L_Oper *new_op;


  /* This algorithm assumes the first queued operation is an add. */
  if ((arth_ct > 0) && (arth_queue[0].op != Lop_ADD))
    L_punt ("L_do_addsub_tree: First queued op not ADD.");

  /*
   * Form binary tree of adds.
   * First add pairs of items, then pair off results and add them, and so on.
   * i is the distance between result pairs in the queue.
   */
  for (i = 1; i < arth_ct; i *= 2)
    {
      /* j is the index of the first item of the result pair to add */
      for (j = 0; (j + i) < arth_ct; j += 2 * i)
        {
          /*
           * If they are both adds or both subtracts, add the results
           * together.  Ie two adds become one add, two subs becomes
           * one sub.
           */
          if (arth_queue[j].op == arth_queue[j + i].op)
            {
              new_op = L_create_new_op (Lop_ADD);
            }

          /*
           * If one is an add operation and the other subtract,
           * just subtract them.
           */
          else
            {
              new_op = L_create_new_op (Lop_SUB);
            }

          /* Set up fields and insert operation */
          new_op->weight = repl_op->weight;
          new_op->dest[0] = L_copy_operand (arth_queue[j].src);
          new_op->src[0] = L_copy_operand (arth_queue[j].src);
          new_op->src[1] = L_copy_operand (arth_queue[j + i].src);

          /* copy the predicates of repl_op */
          for (c = 0; c < L_max_pred_operand; c++)
            {
              new_op->pred[c] = L_copy_operand (repl_op->pred[c]);
            }

          L_insert_oper_before (cb, repl_op, new_op);
        }
    }
}

/* Calculates the height of a binary add tree containing 'ops' operations */
static int
L_calc_add_tree_height (int ops)
{
  int i;

  if (ops <= 0)
    return (0);


  for (i = 0; i < 32; i++)
    {
      if ((ops >> i) == 0)
        break;
    }

  return (i);
}

/*
 * Generates shifts/adds/subs to realize multiplying by the constant cval.
 * It produces good but sub-optimal results for some values of cval.
 * It produces code that minimizes the dependence between adjacent
 * instructions.  It generates all the (independent)shift operations needed,
 * then forms an add/sub tree out of the adds/sub.
 *
 * If flag == COUNT_ONLY, it only counts the number of instructions needed
 * and calculates the number of cycles needed on a general ISSUE_RATE issue
 * machine (assuming any non-dependent instructions can be executed in
 * parallel up to ISSUE_RATE).
 *
 * If flag == GEN_CODE, the code is generated and inserted before repl_op.
 * The weight/color is taken from repl_op.  Repl_op is not modified in
 * any way.
 *
 * This function requires that the constant is representable as a single
 * number in the architecture. Check before calling.
 */
static void
L_gen_str_red_mult_ops (int flag, L_Operand * dest, L_Operand * src,
                        ITintmax cval, L_Cb * cb, L_Oper * repl_op,
                        int *instr_ct, int *cycle_ct)
{
  int group_ct, t_ct;
  int i, j, c;
  L_Oper *new_op = NULL;
  int dependent_adds;

  /* Test for special cases */
  if ((cval == -1) || (cval == 0) || (cval == 1))
    {
      /* Generate code if required */
      if (flag == GEN_CODE)
        {
          /* Multiply by 1 */
          if (cval == 1)
            {
              new_op = L_create_new_op (Lop_MOV);
              new_op->src[0] = L_copy_operand (src);
            }

          /* Multiply by 0 */
          else if (cval == 0)
            {
              new_op = L_create_new_op (Lop_MOV);
              new_op->src[0] = L_new_gen_int_operand (0);
            }

          /* Multiply by -1 */
          else if (cval == -1)
            {
              new_op = L_create_new_op (Lop_SUB);
              new_op->src[0] = L_new_gen_int_operand (0);
              new_op->src[1] = L_copy_operand (src);
            }
          new_op->weight = repl_op->weight;
          new_op->dest[0] = L_copy_operand (dest);

          /* copy the predicates of repl_op */
          for (c = 0; c < L_max_pred_operand; c++)
            {
              new_op->pred[c] = L_copy_operand (repl_op->pred[c]);
            }

          L_insert_oper_before (cb, repl_op, new_op);
        }
      /* These special cases only take one instr and thus one cycle */
      *instr_ct = 1;
      *cycle_ct = 1;
      return;
    }

  /* Handle negate numbers by negating at end */
  if (cval < 0)
    {
      negate = 1;
      cval = -cval;
    }
  else
    negate = 0;

  /* Initialize counters */
  shift_ct = 0;
  arth_ct = 0;

  /* Search for 1 bits starting with most significant bit */
  i = bits_in_int - 1;
  while (i >= 0)
    {
      /*
       * Find number of consecutive 1 bits to the right of i
       * including the ith bit
       */
      group_ct = L_get_group_len (cval, i);

      /* No high bit at i, move bit to right, continue search */
      if (group_ct == 0)
        --i;

      /* The sign bit set can screw this up, but it shouldn't happen */
      else if (i == (bits_in_int - 1))
        {
          L_punt ("L_multiply_str_red: high bit cannot be set");
          break;
        }

      /*
       * Just shift out and add if single 1 bit or
       * a group of two that is not followed by 0 and another group
       * of two or more bits.  (ie 11011 can be optimized by next routine
       */
      else if ((group_ct == 1) ||
               ((group_ct == 2) &&
                (L_get_group_len (cval, i - group_ct - 1) < 2)))
        {
          for (j = 0; j < group_ct; j++)
            {
              /* Optimize shift by 0 */
              if ((i - j) > 0)
                /* Shift src out and add result (the add is queued) */
                L_queue_add (flag, L_do_lshift (flag, src, i - j, cb,
                                                repl_op));
              else
                L_queue_add (flag, src);
            }
          /* Skip the number of 1 bits processed */
          i -= group_ct;
        }
      /* Optimize groups of 1 bits */
      else
        {
          /* Set up sub optimization (ie 10000 - 00010 = 01110) */
          L_queue_add (flag, L_do_lshift (flag, src, i + 1, cb, repl_op));

          /*
           * Optimize for single zeros in groups of one bits.
           * Extend group if current group is followed by a single
           * zero and another group of ones.
           */
          while ((t_ct = L_get_group_len (cval, i - group_ct - 1)) >= 2)
            {
              /* Subtract out zero from the middle of a groupt of ones */
              L_queue_sub (flag, L_do_lshift (flag, src, i - group_ct, cb,
                                              repl_op));
              /* The group can now be expanded to include the next group */
              group_ct += 1 + t_ct;
            }

          /* Optimize shift 0, (group including bit 0) */
          if ((i - group_ct) >= 0)
            /* Subtract out to get a group of one bits */
            L_queue_sub (flag, L_do_lshift (flag, src, i - group_ct + 1,
                                            cb, repl_op));
          else
            L_queue_sub (flag, src);

          /* Continue search after just after this group */
          i -= group_ct;
        }

    }

  /* If generating code, generate the add/sub tree */
  if (flag == GEN_CODE)
    L_do_addsub_tree (cb, repl_op);

  /* If generating code, redirect last operation to correct destination */
  if (flag == GEN_CODE)
    {
      L_delete_operand ((repl_op->prev_op)->dest[0]);
      (repl_op->prev_op)->dest[0] = L_copy_operand (dest);
    }

  /* If generating code and value negative, generate code to negate result */
  if ((flag == GEN_CODE) && negate)
    {
      new_op = L_create_new_op (Lop_SUB);
      new_op->weight = repl_op->weight;
      new_op->dest[0] = L_copy_operand (dest);
      new_op->src[0] = L_new_gen_int_operand (0);
      new_op->src[1] = L_copy_operand (dest);
      /* copy the predicates of repl_op */
      for (c = 0; c < L_max_pred_operand; c++)
        {
          new_op->pred[c] = L_copy_operand (repl_op->pred[c]);
        }
      L_insert_oper_before (cb, repl_op, new_op);
    }

  /* Calculate the number of instructions needed */
  *instr_ct = shift_ct + arth_ct + negate - 1;

  /* Calculate the number of cycles needed on an issue ISSUE_RATE machiner */

  /*
   * First for add tree, calculate the number of instructions who's
   * issue is dependent on tree dependencies not the issue capability.
   * ie for an issue 4 machine, the last 3 levels (7 instructions) of
   * the addtree would take the same number of  cycles on a issue 8 machine.
   * The other add tree instruction (the ones higher on the tree) are
   * limited by the issue rate, not the tree structure.  These issue
   * at ISSUE_RATE ops/cycle and for this calculate it is assumed that some
   * of them can be issued with the shifts (to give absolute the minimum
   * possible number of cycles).
   */

  dependent_adds = (1 << L_calc_add_tree_height (ISSUE_RATE)) - 1;

  /* Are the number of cycles limited by the tree structure? */
  if ((arth_ct - 1) <= dependent_adds)
    {
      /*
       * Yes, cycle time is just the time needed for shifts plus
       * the height of add tree (and of course the negation if needed).
       */
      *cycle_ct = ((shift_ct + (ISSUE_RATE - 1)) / ISSUE_RATE)
        + L_calc_add_tree_height (arth_ct - 1) + negate;
    }
  else
    {
      /*
       * No, the top part of the add tree is limited by the issue rate.
       * Thus the number of cycles is the height of the depended part
       * of the tree, plus the cycles needed to issue the issue rate
       * depended instructions (and of course the negation if needed).
       */
      *cycle_ct = L_calc_add_tree_height (dependent_adds) +
        ((((arth_ct - 1) - dependent_adds) + shift_ct + (ISSUE_RATE - 1)) /
         ISSUE_RATE) + negate;
    }
}

/*
 * Scan though code looking for multiplies (MULT(_U)) with one const argument.
 * Change into an equivalent series of lsl, add and sub operations
 * if this expansion requires <= STR_RED_MUL_LIMIT instructions.
 * The necessary add/sub instructions are queued, then converted
 * into an equivalent add/sub tree to maximize instruction parallelism.
 */
static void
L_multiply_strength_reduce (L_Cb * cb)
{
  ITintmax cval;
  L_Operand *dest, *src;
  L_Oper *cur_op, *next_op;
  int op_count, cycle_count;

  /* Search though entire linked list of code for appropriate mults */
  for (cur_op = cb->first_op; cur_op != NULL; cur_op = next_op)
    {
      /* Get next op now, since may invalidate cur_op in this loop */
      next_op = cur_op->next_op;

      /* Goto next instruction unless this is an int multiply */
      if (!L_int_mul_opcode (cur_op))
        continue;

      /*
       * Goto next instruction unless there is one register/macro
       * operand and one constant operand.  Get constant and
       * reg/macro operands and the computations destination.
       */
      if ((L_is_register (cur_op->src[0]) ||
           L_is_macro (cur_op->src[0])) &&
          L_is_numeric_constant (cur_op->src[1]) &&
	  (cur_op->src[1]->value.i <= ITINTMAX) &&
	  (cur_op->src[1]->value.i >= ITINTMIN))
        {
          cval = cur_op->src[1]->value.i;
          src = cur_op->src[0];
          dest = cur_op->dest[0];
        }
      else if ((L_is_register (cur_op->src[1]) ||
                L_is_macro (cur_op->src[1])) &&
               L_is_numeric_constant (cur_op->src[0]) &&
	       (cur_op->src[1]->value.i <= ITINTMAX) &&
	       (cur_op->src[1]->value.i >= ITINTMIN))
        {
          cval = cur_op->src[0]->value.i;
          src = cur_op->src[1];
          dest = cur_op->dest[0];
        }
      else
        continue;



      /* Count the operations required to replace mult */
      L_gen_str_red_mult_ops (COUNT_ONLY, dest, src, cval, cb, cur_op,
                              &op_count, &cycle_count);

      /* Do replacement if doesn't exceed operation limit */
      if (op_count <= STR_RED_MUL_LIMIT)
        {
          L_gen_str_red_mult_ops (GEN_CODE, dest, src, cval, cb,
                                  cur_op, &op_count, &cycle_count);

          /* Show what we are doing */
          if (DEBUG_MUL_STR_RED)
            {
              fprintf (ERR, "$$$ Applying multiply str red to %d (cb %d)\n",
                       cur_op->id, cb->id);
              fprintf (ERR, "$$$ %i ops, %i cycles on an issue %i machine\n",
                       op_count, cycle_count, ISSUE_RATE);
              STAT_COUNT ("L_multiply_strength_reduce", 1, cb);
              L_invalidate_dataflow ();
            }

          /* Delete replaced instruction, finished */
          L_delete_oper (cb, cur_op);

        }
    }
}

/*=======================================================================*/
/*
 *      Exported functions
 */
/*=======================================================================*/

void
Lsuper_strength_reduction (L_Func * fn)
{
  L_Cb *cb;

  if (DEBUG_MUL_STR_RED | DEBUG_DIV_STR_RED)
    fprintf (ERR, "Enter multiply/divide strength reduction\n");

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {

      /* REH 11/95 - Leave my boundary blocks alone */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

#ifdef DO_MUL_STR_RED
      L_multiply_strength_reduce (cb);
#endif
    }
}
