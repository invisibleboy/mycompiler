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
 *
 *  File: mia_compare.c
 *
 *  Description: Height reduction optimization on compares.
 *
 *  Creation Date: June 1997
 *
 *  Author: Bob McGowan
 *
\*****************************************************************************/
/* 09/17/02 REK Updating to use the new Ltahoe opcode map and completers
 *              scheme. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "mia_internal.h"

#define VERBOSE
#undef DEBUG

#ifdef DEBUG
#define VERBOSE
#endif

static L_Oper *find_add_for_compare (L_Oper * compare, int operand_num);
static int Switch_add_compare_order_if_needed (L_Cb * cb, L_Oper * add,
					       L_Oper * compare,
					       int compare_operand_num);
static void Change_lte_to_lt (L_Oper * compare);
static void Change_gte_to_gt (L_Oper * compare);


/****************************************************************************
 *
 * routine: Mopti_compare_height_reduction()
 * purpose: Perform height reduction of compare ops dependent on an
 *          arithmetic op.
 * input:
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

int
Mopti_loop_compare_height_reduction (L_Func * fn)
{
  L_Loop *loop;
  L_Cb *cb;
  int n_loop_cb;
  int cb_spot;
  int *loop_cb_id;
  int change = 0;

  for (loop = fn->first_loop; loop; loop = loop->next_loop)
    {
      n_loop_cb = Set_size (loop->loop_cb);
      if (!n_loop_cb)
	continue;
      loop_cb_id = malloc (n_loop_cb * sizeof (int));
      Set_2array (loop->loop_cb, loop_cb_id);

      for (cb_spot = 0; cb_spot < n_loop_cb; cb_spot++)
	{
	  cb = L_cb_hash_tbl_find (fn->cb_hash_tbl, loop_cb_id[cb_spot]);
	  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) ||
	      !L_find_attr (cb->attr, "kernel"))
	    change += Mopti_compare_height_reduction (fn, cb, loop,
						      n_loop_cb, loop_cb_id);
	}			/* for cb_spot */
      free (loop_cb_id);
    }				/* for loop */
  return change;
}				/* Mopti_loop_compare_height_reduction */


/****************************************************************************
 *
 * routine: Mopti_compare_height_reduction()
 * purpose: Perform height reduction of compare ops dependent on an
 *          arithmetic op.
 *          There are three cases which can be optimized:
 *            case 1:   add r1 = r2,C1    where C is a constant
 *                      cmp r1,C2
 *               becomes
 *                      cmp r2,C2-C1
 *                      add r1 = r2,C1
 *               This also works with subtract.
 *
 *            case 2a:  add       r1 = r2,1
 *                      cmp.le    r1,r3
 *               becomes
 *                      cmp.lt    r2,r3
 *                      add       r1 = r2,1
 *               Notice that the order of the operations must be reversed
 *                 in case r2 is r1.
 *            
 *            case 2b:  add       r1 = r2,-1
 *                      cmp.ge    r1,r3
 *               becomes
 *                      cmp.gt    r2,r3
 *                      add       r1 = r2,-1
 *
 *            case 2c:  add       r3 = r2,1
 *                      cmp.ge    r1,r3
 *               becomes
 *                      cmp.gt    r1,r2
 *                      add       r3 = r2,1
 *            
 *            case 2d:  add       r3 = r2,-1
 *                      cmp.le    r1,r3
 *               becomes
 *                      cmp.lt    r1,r2
 *                      add       r3 = r2,-1
 *
 *            case 3:   add r1 = r1,C     where r1 is a loop induction variable
 *                                          only altered here
 *                      cmp r1,r2         where r2 is loop invariant
 *               becomes
 *                      add r4 = r2,-C    place in the loop preheader
 *                      ...
 *                      cmp r1,r4
 *                      add r1 = r1,C
 * input: cb - cb to optimize
 *        loop - NULL is okay, just means that we are not in the loop.
 * output:
 * returns:
 * modified:
 * note: Assumes that add and compare have the same predicate.
 *-------------------------------------------------------------------------*/

int
Mopti_compare_height_reduction (L_Func * fn, L_Cb * cb, L_Loop * loop,
				int loop_n_cb, int *loop_cb)
{
  L_Oper *compare;
  L_Oper *add_op;
  L_Oper *new_op;
  int new_constant;
  int count = 0;

  for (compare = cb->first_op; compare; compare = compare->next_op)
    {
      if (!L_general_int_predicate_define_opcode (compare))
	continue;

      /* A compare has been found.
         Look for the add which has a constant as src[1] */

      add_op = find_add_for_compare (compare, 1);
      if (add_op)
	{
	  if (L_is_int_constant (compare->src[0]))
	    {
	      /* case 1a:
	         add r1 = C1,r2    where C is a constant
	         cmp C2,r1 */
	      new_constant =
		compare->src[0]->value.i - add_op->src[0]->value.i;
	      if (!SIMM_8 (new_constant))
		continue;

	      if (!Switch_add_compare_order_if_needed (cb, add_op, compare,
						       0))
		continue;

	      compare->src[0]->value.i = new_constant;
	      L_delete_operand (compare->src[1]);
	      compare->src[1] = L_copy_operand (add_op->src[1]);

#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(1a): add const, cmp const on "
		       "add op %d and compare op %d saving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if (LT_is_R0_operand (compare->src[0]))
	    {
	      /* case 1b:  same as 1a except process r0
	         add r1 = C1,r2    where C is a constant
	         cmp C2,r1 */
	      if (!SIMM_8 (add_op->src[0]->value.i))
		break;

	      if (!Switch_add_compare_order_if_needed
		  (cb, add_op, compare, 0))
		continue;

	      L_delete_operand (compare->src[0]);
	      compare->src[0] = L_copy_operand (add_op->src[0]);
	      compare->src[0]->value.i = -compare->src[0]->value.i;
	      L_delete_operand (compare->src[1]);
	      compare->src[1] = L_copy_operand (add_op->src[1]);
#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(1b): add const, cmp const on "
		       "add op %d and compare op %d saving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if ((add_op->src[0]->value.i == 1) && L_int_ge_cmp_opcode (compare))
	    {
	      /* case 2a:
	         add       r1 = 1,r2
	         cmp.ge    r3,r1
	         becomes
	         add       r1 = 1,r2
	         cmp.gt    r3,r2 */
	      if (!Switch_add_compare_order_if_needed (cb, add_op, compare,
						       0))
		continue;

	      Change_gte_to_gt (compare);

#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(2a): add +1, cmp.gt on "
		       "add op %d and compare op %d saving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if ((add_op->src[0]->value.i == -1) &&
	      L_int_le_cmp_opcode (compare))
	    {
	      /* case 2b:
	         add    r1 = -1,r2
	         cmp.le r3,r1
	         becomes
	         add    r1 = -1,r2
	         cmp.lt r3,r2
	       */
	      if (!Switch_add_compare_order_if_needed (cb, add_op, compare,
						       0))
		continue;
	      Change_lte_to_lt (compare);

#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(2b): add -1, cmp.lt on "
		       "add op %d and compare op %d saving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if (loop)
	    {
	      if (L_same_operand (add_op->dest[0], add_op->src[1]) &&
		  L_unique_def_in_loop (loop, loop_cb, loop_n_cb, add_op)
		  && L_is_loop_inv_operand (loop, loop_cb, loop_n_cb,
					    compare->src[0]))
		{
		  /* case 3:
		   *   add r1 = C,r1     where r1 is a loop induction
		   *                              variable only altered here
		   *   cmp r2,r1         where r2 is loop invariant
		   * becomes
		   *   add r4 = -C,r2    place in the loop preheader
		   *   ...
		   *   cmp r4,r1
		   *   add r1 = C,r1 
		   * The add and compare need to be reordered for this to
		   * work */
		  if (!Switch_add_compare_order_if_needed (cb, add_op,
							   compare, 0))
		    continue;

		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] =
		    L_new_register_operand (++(fn->max_reg_id),
					    L_CTYPE_LLONG, L_PTYPE_NULL);
		  new_op->src[0] = L_copy_operand (add_op->src[0]);
		  new_op->src[0]->value.i = -new_op->src[0]->value.i;
		  new_op->src[1] = compare->src[0];
		  L_insert_oper_after (loop->preheader, new_op,
				       loop->preheader->last_op);

		  compare->src[0] = L_copy_operand (new_op->dest[0]);

#ifdef VERBOSE
		  fprintf (stderr, "Compare opt(3a): loop inv cmp operand on "
			   "add op %d and compare op %d saving %f cycles\n",
			   add_op->id, compare->id, cb->weight);
#endif
		  count++;
		  continue;
		}		/* if */
	    }			/* if */
	}			/* if */

      add_op = find_add_for_compare (compare, 0);
      if (add_op)
	{
#ifdef DEBUG
	  fprintf (stderr, "  Found add op %d for compare op %d\n",
		   add_op->id, compare->id);
#endif

	  if (L_is_int_constant (compare->src[1]))
	    {
	      /* case 1c:
	         add r1 = C1,r2    where C is a constant
	         cmp r1,C2 */
	      new_constant =
		compare->src[1]->value.i - add_op->src[0]->value.i;
	      if (!SIMM_8 (new_constant))
		continue;

	      if (!Switch_add_compare_order_if_needed
		  (cb, add_op, compare, 1))
		continue;

	      compare->src[1]->value.i = new_constant;
	      L_delete_operand (compare->src[0]);
	      compare->src[0] = L_copy_operand (add_op->src[1]);

#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(1c): add const, cmp const on "
		       "add op %d and compare op %d saving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if (LT_is_R0_operand (compare->src[1]))
	    {
	      /* case 1d:  same as 1a except process r0
	         add r1 = C1,r2    where C is a constant
	         cmp r1,C2 */
	      if (!SIMM_8 (add_op->src[0]->value.i))
		break;

	      if (!Switch_add_compare_order_if_needed
		  (cb, add_op, compare, 1))
		continue;

	      L_delete_operand (compare->src[0]);
	      compare->src[1] = L_copy_operand (add_op->src[0]);
	      compare->src[1]->value.i = -compare->src[1]->value.i;
	      L_delete_operand (compare->src[0]);
	      compare->src[0] = L_copy_operand (add_op->src[1]);
#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(1d): add const, cmp const on "
		       "add op %d and compare op %d saving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if ((add_op->src[0]->value.i == 1) && L_int_le_cmp_opcode (compare))
	    {
	      /* case 2c:
	         add       r1 = 1,r2
	         cmp.le    r1,r3
	         becomes
	         add       r1 = 1,r2
	         cmp.lt    r2,r3, */
	      if (!Switch_add_compare_order_if_needed (cb, add_op, compare,
						       1))
		continue;

	      Change_lte_to_lt (compare);

#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(2c): add +1, cmp.lt on "
		       "add op %d and compare op %d aving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if ((add_op->src[0]->value.i == -1) &&
	      L_int_ge_cmp_opcode (compare))
	    {
	      /* case 2d:
	         add    r1 = -1,r2
	         cmp.ge r1,r3
	         becomes
	         add    r1 = -1,r2
	         cmp.gt r2,r3
	       */
	      if (!Switch_add_compare_order_if_needed (cb, add_op, compare,
						       1))
		continue;

	      Change_gte_to_gt (compare);

#ifdef VERBOSE
	      fprintf (stderr, "Compare opt(2d): add -1, cmp.gt on "
		       "add op %d and compare op %d saving %f cycles\n",
		       add_op->id, compare->id, cb->weight);
#endif
	      count++;
	      continue;
	    }			/* if */

	  if (loop)
	    {
	      if (L_same_operand (add_op->dest[0], add_op->src[1]) &&
		  L_unique_def_in_loop (loop, loop_cb, loop_n_cb, add_op)
		  && L_is_loop_inv_operand (loop, loop_cb, loop_n_cb,
					    compare->src[0]))
		{
		  /* case 3:
		   *   add r1 = C,r1     where r1 is a loop induction
		   *                              variable only altered here
		   *   cmp r2,r1         where r2 is loop invariant
		   * becomes
		   *   add r4 = -C,r2    place in the loop preheader
		   *   ...
		   *   cmp r4,r1
		   *   add r1 = C,r1 
		   * Add and compare need to be reordered for this to work */
		  if (!Switch_add_compare_order_if_needed (cb, add_op,
							   compare, 1))
		    continue;

		  new_op = L_create_new_op (Lop_ADD);
		  new_op->proc_opc = TAHOEop_ADDS;
		  new_op->dest[0] =
		    L_new_register_operand (++(fn->max_reg_id),
					    L_CTYPE_LLONG, L_PTYPE_NULL);
		  new_op->src[0] = L_copy_operand (add_op->src[0]);
		  new_op->src[0]->value.i = -new_op->src[0]->value.i;
		  new_op->src[1] = compare->src[0];
		  L_insert_oper_after (loop->preheader, new_op,
				       loop->preheader->last_op);

		  compare->src[0] = L_copy_operand (new_op->dest[0]);

#ifdef VERBOSE
		  fprintf (stderr, "Compare opt(3b): loop inv cmp operand on "
			   "add op %d and compare op %d saving %f cycles\n",
			   add_op->id, compare->id, cb->weight);
#endif
		  count++;
		  continue;
		}		/* if */
	    }			/* if */
	}			/* if */
    }				/* for compare */
  return count;
}				/* Mopti_compare_height_reduction */


/****************************************************************************
 *
 * routine: find_add_for_compare()
 * purpose: Find an add or sub that creates the destination for the given
 *          operand on the compare.  The add and compare must have the same
 *          predicate.  This may be overly restrictive.
 *          Find: add cmp_src = const, reg
 * input: compare - the compare op
 *        operand_num - the operand on the compare to look for
 * output:
 * returns: A pointer to the add/sub op, or
 *          NULL if some other op defines the compare src or the define
 *          cannot be found.
 * modified:
 * note: This will convert a sub into an add.
 *       Stop if a branch is encountered or if a definition of
 *         the compare source (r1) is found.
 *         Stopping at a branch may be conservative.  We could check if
 *         the cmp is safe to move above branch.
 *       Also forcing same predicates is also overly restrictive.
 *       Only look for adds with constant as the first argument.
 *-------------------------------------------------------------------------*/

static L_Oper *
find_add_for_compare (L_Oper * compare, int operand_num)
{
  L_Oper *op;
  L_Operand *temp;

  for (op = compare->prev_op;
       op && !L_general_branch_opcode (op); op = op->prev_op)
    {
      if (L_is_dest_operand (compare->src[operand_num], op))
	{
	  if (L_int_add_opcode (op))
	    {
	      if (L_is_int_constant (op->src[0]) &&
		  PG_equivalent_predicates_ops (compare, op))
		{
		  return (op);
		}		/* if */
	    }			/* if */
	  else
	    {
	      if (L_int_sub_opcode (op))
		{
		  if (L_is_int_constant (op->src[1]) &&
		      PG_equivalent_predicates_ops (compare, op))
		    {
		      L_change_opcode (op, Lop_ADD);
		      op->proc_opc = TAHOEop_ADDS;
		      temp = op->src[0];
		      op->src[0] = op->src[1];
		      op->src[1] = temp;
		      op->src[1]->value.i = -op->src[1]->value.i;
		      return (op);
		    }		/* if */
		}		/* if */
	    }			/* else */
	  return (NULL);
	}			/* if */
    }				/* for op */
  return (NULL);
}				/* find_add_for_compare */


/****************************************************************************
 *
 * routine: Switch_add_compare_order_if_needed()
 * purpose: Switch the order of the add and compare if needed.  This will be
 *          necessary if the dest is the same as the source to the add.
 *             add r1 = C,r2    where C is a constant
 *             cmp r3,r1 
 * input:
 * output:
 * returns: 2 if swapped, 1 if no swap is needed, 0 if swap is needed but
 *          cannot be performed.
 * modified:
 * note: Assumes no branches between add and compare.
 *-------------------------------------------------------------------------*/

static int
Switch_add_compare_order_if_needed (L_Cb * cb, L_Oper * add,
				    L_Oper * compare, int compare_operand_num)
{
  if (L_same_operand (add->dest[0], add->src[1]))
    {
      /* The add and compare need to be reordered for this to work */
      if (L_no_uses_between (add->dest[0], add, compare) &&
	  L_no_danger (L_is_macro (add->dest[0]) /* macro flag */ ,
		       0 /* load floag */ , 0 /* store flag */ ,
		       add, compare))
	{
	  /* move the add below the compare */
	  L_remove_oper (cb, add);
	  L_insert_oper_after (cb, compare, add);
	  return (2);
	}			/* if */
      else
	{
	  if (L_no_defs_between (compare->src[compare_operand_num], add,
				 compare) &&
	      L_all_dest_operand_no_defs_between (compare, add,
						  compare) &&
	      L_no_danger (L_is_macro (compare->src[compare_operand_num]),
			   0 /* load floag */ , 0 /* store flag */ ,
			   add, compare))
	    {
	      /* move the compare above the add */
	      L_remove_oper (cb, compare);
	      L_insert_oper_before (cb, add, compare);
	      return (2);
	    }			/* if */
	  else
	    {
	      /* abort the optimization */
	      return (0);
	    }			/* else */
	}			/* else */
    }				/* if */
  return (1);
}				/* Switch_add_compare_order_if_needed */


/****************************************************************************
 *
 * routine: Change_lte_to_lt()
 * purpose: Change a compare from a <= to a <.
 * input: compare op
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

static void
Change_lte_to_lt (L_Oper * compare)
{
  L_set_compare_type (compare, Lcmp_COM_LT);

  switch (TC_GET_CMP_OP (compare->completers))
    {
    case TC_CMP_OP_LE:
      TC_SET_CMP_OP (compare->completers, TC_CMP_OP_LT);
      break;

    case TC_CMP_OP_LEU:
      TC_SET_CMP_OP (compare->completers, TC_CMP_OP_LTU);
      break;

    default:
      L_punt ("Change_lte_to_lt: Unknown compare tahoe op %u on op %d\n",
	      compare->proc_opc, compare->id);
    }				/* switch */
}				/* Change_lte_to_lt */


/****************************************************************************
 *
 * routine: Change_gte_to_gt()
 * purpose: Change a compare from a >= to a >.
 * input: compare op
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/

static void
Change_gte_to_gt (L_Oper * compare)
{
  L_set_compare_type (compare, Lcmp_COM_GT);

  switch (TC_GET_CMP_OP (compare->completers))
    {
    case TC_CMP_OP_GE:
      TC_SET_CMP_OP (compare->completers, TC_CMP_OP_GT);
      break;

    case TC_CMP_OP_GEU:
      TC_SET_CMP_OP (compare->completers, TC_CMP_OP_GTU);
      break;

    default:
      L_punt ("Mopti_compare_height_reductions: Unknown compare "
	      "tahoe op %u on op %d\n", compare->proc_opc, compare->id);
    }				/* switch */
}				/* Change_gte_to_gt */
