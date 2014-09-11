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
 *      File :          l_peel_accum.c
 *      Description :   Accumulator/Induction var expansion for peeled loops
 *      Creation Date : July 1995
 *      Author :        David August
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define ERR     stderr

#define DONT_INSERT     0
#define INSERT_BEFORE   1
#define INSERT_AFTER    2

typedef struct peelregion
{
  L_Oper *first_op;
  L_Oper *last_op;
  L_Cb *block;
  Set flow_ops;
  int peels;
  int *iter_pred;
  struct peelregion *next;
} PeelRegion;

PeelRegion *peel_region_head = NULL;

typedef struct accnode
{
  L_Operand *dest;
  L_Operand *src;
  int same_src;
  int opcode;
  int is_sum;
  int eligible;
  int count;
  int bad_pred_flag;
  Set ops;
  struct accnode *next;
} AccNode;

static AccNode *acc_list_head = NULL;

static void
print_peel_region (PeelRegion * peel_reg)
{
  int indx;

  fprintf (ERR, "PEEL_LOOP_REGION:\n");
  fprintf (ERR, "  First Op: %d\n", peel_reg->first_op->id);
  fprintf (ERR, "  Last Op: %d\n", peel_reg->last_op->id);

  fprintf (ERR, "  Peels: %d\n", peel_reg->peels);
  for (indx = 0; indx < peel_reg->peels; indx++)
    fprintf (ERR, "  Iter_Pred %d: %d\n", indx, peel_reg->iter_pred[indx]);

  Set_print (ERR, "  Flow_ops: ", peel_reg->flow_ops);
  fprintf (ERR, "\n");
}

static PeelRegion *
new_peel_region ()
{
  PeelRegion *new_p_reg;

  if ((new_p_reg = (PeelRegion *) malloc (sizeof (PeelRegion))) == NULL)
    L_punt ("Out of memory in new_peel_region");

  new_p_reg->first_op = NULL;
  new_p_reg->last_op = NULL;
  new_p_reg->flow_ops = NULL;
  new_p_reg->peels = 0;
  new_p_reg->iter_pred = NULL;
  new_p_reg->next = NULL;
  new_p_reg->block = NULL;

  return new_p_reg;
}

static PeelRegion *
free_peel_region_list (PeelRegion *p_reg)
{
  PeelRegion *p_reg_n;

  while (p_reg)
    {
      p_reg_n = p_reg->next;

      if (p_reg->flow_ops)
	Set_dispose (p_reg->flow_ops);
      if (p_reg->iter_pred)
	free (p_reg->iter_pred);

      free (p_reg);

      p_reg = p_reg_n;
    }

  return NULL;
}

static PeelRegion *
create_peel_region_list (L_Cb * hyperblock)
{
  L_Oper *prev_op = NULL, *term_op;
  L_Oper *op_indx;
  L_Attr *attr;
  ITintmax cur_peel_num, cur_peel_id = -1;
  PeelRegion *cur_p_reg = NULL, *new_p_reg, *p_reg_indx, *root_p_reg = NULL;

  /*
   * Go through each op in the hyperblock.  Mark off each peelregion.
   * Only do innermost peels for now.
   */

  for (op_indx = hyperblock->first_op; op_indx; op_indx = op_indx->next_op)
    {
      if ((attr = L_find_attr (op_indx->attr, "peel")))
        {
          if (attr->field[1]->value.i != cur_peel_id)
            {
              cur_peel_id = attr->field[1]->value.i;

              new_p_reg = new_peel_region ();
              if (cur_p_reg)
                {
                  cur_p_reg->next = new_p_reg;
                  if (!cur_p_reg->last_op)
		    cur_p_reg->last_op = prev_op;
                }
              if (!root_p_reg)
		root_p_reg = new_p_reg;

              new_p_reg->first_op = op_indx;
              cur_p_reg = new_p_reg;
            }
        }
      else
        {
          cur_peel_id = -1;
          if (cur_p_reg && !cur_p_reg->last_op)
	    cur_p_reg->last_op = prev_op;
        }
      prev_op = op_indx;
    }

  if (cur_p_reg && !cur_p_reg->last_op)
    cur_p_reg->last_op = prev_op;

  for (p_reg_indx = root_p_reg; p_reg_indx; p_reg_indx = p_reg_indx->next)
    {
      p_reg_indx->block = hyperblock;
      p_reg_indx->peels = 0;
      term_op = p_reg_indx->last_op->next_op;
      for (op_indx = p_reg_indx->first_op;
           op_indx != term_op;
           op_indx = op_indx->next_op)
        {
	  if (!(attr = L_find_attr (op_indx->attr, "peel")))
	    L_punt ("create_peel_region_list: peel attribute missing.");

	  if (attr->field[0]->value.i > p_reg_indx->peels)
	    p_reg_indx->peels = ITicast (attr->field[0]->value.i);
        }
      p_reg_indx->iter_pred = malloc (sizeof (int) * p_reg_indx->peels);
    }

  /*
   * Find iteration predicates.
   */

  for (p_reg_indx = root_p_reg; p_reg_indx; p_reg_indx = p_reg_indx->next)
    {
      int indx = 0;
      cur_peel_num = -1;
      op_indx = p_reg_indx->first_op;
      term_op = p_reg_indx->last_op->next_op;
      while (op_indx != term_op)
        {
          if (L_is_control_oper (op_indx))
	    p_reg_indx->flow_ops =
	      Set_add (p_reg_indx->flow_ops, op_indx->id);

          if ((attr = L_find_attr (op_indx->attr, "peel")))
            {
              if (attr->field[0]->value.i != cur_peel_num)
                {
                  /* With no preds, use 0 as predicate register */

		  p_reg_indx->iter_pred[indx++] = op_indx->pred[0] ?
		    op_indx->pred[0]->value.r : 0;

                  cur_peel_num = attr->field[0]->value.i;
                }
            }

          op_indx = op_indx->next_op;
        }
      /* Add unconditional branch if it immediately follows the peels */
      if (L_uncond_branch_opcode (op_indx))
	p_reg_indx->flow_ops = Set_add (p_reg_indx->flow_ops, op_indx->id);

      if (!op_indx && !L_uncond_branch_opcode (p_reg_indx->last_op))
	/* Add fall through, flag with -1 */
	p_reg_indx->flow_ops = Set_add (p_reg_indx->flow_ops, -1);
      else if (!L_uncond_branch_opcode (p_reg_indx->last_op->next_op))
	/* Add fall through to rest of hyperblock, flag with -2 */
	p_reg_indx->flow_ops = Set_add (p_reg_indx->flow_ops, -2);
    }

  return root_p_reg;
}

static AccNode *
add_accum (L_Oper * oper, PeelRegion * peel_reg)
{
  int opcode;
  AccNode *cur;
  L_Oper *search_op;

  opcode = oper->opc;

  /* Look for acc in current list first */
  cur = acc_list_head;

  while (cur != NULL)
    {
      if (L_same_operand (cur->dest, oper->dest[0]))
        {
          if (L_is_compatible_opc (cur->opcode, opcode))
            {
              cur->ops = Set_add (cur->ops, oper->id);
              cur->count++;
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
      if (!(cur = (AccNode *) malloc (sizeof (AccNode))))
        L_punt ("Out of memory in addaccum");
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
      cur->bad_pred_flag = 0;
      cur->ops = Set_add ((Set) 0, oper->id);
      cur->next = acc_list_head;
      acc_list_head = cur;

      if (L_is_predicated (oper))
	{
	  int indx;
	  cur->bad_pred_flag = 1;
	  for (indx = 0; indx < peel_reg->peels; indx++)
	    {
	      if (oper->pred[0]->value.r == peel_reg->iter_pred[indx])
		{
		  cur->bad_pred_flag = 0;
		  break;
		}
	    }
	}
    }

  /*
   * Ensure accumulator uses are dominated by the induction operation
   */

  if (L_is_predicated (oper) && !cur->bad_pred_flag)
    {
      int indx;

      search_op = oper->next_op;
      while ((search_op != NULL) && (cur->bad_pred_flag == 0))
	{
	  /* A non-dominated use prohibits the optimization */

	  if (!PG_superset_predicate_ops(oper, search_op))
	    {
	      if (L_general_branch_opcode (search_op) &&
                  L_in_oper_OUT_set (peel_reg->block, 
				     search_op, oper->dest[0], 
				     TAKEN_PATH))
		cur->bad_pred_flag = 1;
	      
	      /* Check all uses of accumulator register below
	       * this operation*/
	      for (indx = 0; indx < L_max_src_operand; indx++)
		{
		  if (L_same_operand (oper->dest[0],
				      search_op->src[indx]))
		    {
		      cur->bad_pred_flag = 1;
		      break;
		    }
		}	      
	    }

	  /* Need not look for non-dominated uses beyond a
	   * dominating redefinition.
	   */

	  if (PG_superset_predicate_ops(search_op, oper))
	    {
	      /* Stop if hit a redefintion that dominates this one */
	      for (indx = 0; indx < L_max_dest_operand; indx++)
		{
		  if (L_same_operand (oper->dest[0],
				      search_op->dest[indx]))
		    {
		      search_op = NULL;
		      break;
		    }
		}
	    }

	  if (search_op)
	    search_op = search_op->next_op;
	}
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


/*
 * Returns 1 if there is exactly one predecessor cb (but there may be
 * multiple arcs from that cb), 0 otherwise.
 */
static int
L_exactly_one_predecessor_cb (L_Cb * cb)
{
  L_Flow *flow;
  L_Cb *src_cb;

  if (cb == NULL)
    L_punt ("L_only_one_predecessor_cb: cb is NIL");
  if (cb->src_flow == NULL)
    return (0);
  flow = cb->src_flow;
  src_cb = flow->src_cb;
  flow = flow->next_flow;
  while (flow != NULL)
    {
      if (src_cb != flow->src_cb)
        return (0);
      flow = flow->next_flow;
    }
  return (1);
}

static int
L_exactly_one_predecessor_arc (L_Cb * cb)
{
  L_Flow *flow;

  if (cb == NULL)
    L_punt ("L_only_one_predecessor_cb: cb is NIL");
  if (cb->src_flow == NULL)
    return (0);
  flow = cb->src_flow;
  if (flow->next_flow)
    return 0;
  else
    return 1;
}


static int
Lsuper_peel_replace_ind_var (L_Func * fn, PeelRegion * peel_reg, AccNode * acc)
{
  int i, c;
  ITintmax inci;
  double incf2;
  L_Cb *branch_cb;
  int mov_opcode, inc_count, insert_var;
  L_Operand **inc_array, **var_array, *group_inc;
  L_Oper *new_op, *cur_op, *next_op, *term_op;

  if (Lsuper_debug_peel_opt)
    {
      fprintf (ERR, "$$$ Eliminating induction variable ");
      L_print_operand (ERR, acc->dest, 1);
      fprintf (ERR, " in cb %d\n", peel_reg->block->id);
    }

  /* JWS: check and abort early */

  {
    int chk = 0;

    if (Set_in (peel_reg->flow_ops, -1))
      {
	if (L_in_cb_IN_set (peel_reg->block->next_cb, acc->dest))
	  chk = 1;
      }
    else if (Set_in (peel_reg->flow_ops, -2))
      {
	if (L_is_predicated (peel_reg->last_op->next_op) ||
	    L_in_oper_IN_set (peel_reg->last_op->next_op, acc->dest))
	  chk = 1;
      }

    if (chk && (acc->count % peel_reg->peels))
      {
	L_warn ("Lsuper_peel_replace_ind_var: "
		"Number of peels is not a multiple of accumulators.");
	return -1;
      }
  }

  inc_array = (L_Operand **) alloca ((acc->count + 1) * sizeof (L_Operand *));
  var_array = (L_Operand **) alloca ((acc->count + 1) * sizeof (L_Operand *));

  /* initialize L_Operand arrays to contain NULL ptrs */
  for (i = 0; i <= acc->count; i++)
    {
      inc_array[i] = NULL;
      var_array[i] = NULL;
    }

  if (L_is_int_constant (acc->src))
    {
      mov_opcode = Lop_MOV;
      /* Create registers for induction variables */
      for (i = 0; i <= acc->count; i++)
        var_array[i] = L_new_register_operand (++L_fn->max_reg_id,
                                               L_native_machine_ctype,
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

      incf2 = acc->src->value.f;
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
          var_array[i] = L_new_register_operand (++L_fn->max_reg_id,
                                                 L_native_machine_ctype,
                                                 L_PTYPE_NULL);
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
          if (peel_reg->iter_pred[0] != 0)
            new_op->pred[0] = L_new_register_operand (peel_reg->iter_pred[0],
                                                      L_CTYPE_PREDICATE,
                                                      L_PTYPE_NULL);
          L_insert_oper_before (peel_reg->block, peel_reg->first_op, new_op);
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
          if (peel_reg->iter_pred[0] != 0)
            new_op->pred[0] = L_new_register_operand (peel_reg->iter_pred[0],
                                                      L_CTYPE_PREDICATE,
                                                      L_PTYPE_NULL);
          L_insert_oper_before (peel_reg->block, peel_reg->first_op, new_op);
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
          if (peel_reg->iter_pred[0] != 0)
            new_op->pred[0] = L_new_register_operand (peel_reg->iter_pred[0],
                                                      L_CTYPE_PREDICATE,
                                                      L_PTYPE_NULL);
          L_insert_oper_before (peel_reg->block, peel_reg->first_op, new_op);
        }
    }
  else
    {
      L_punt ("Unknown source type in Lsuper_peel_replace_ind_var");
      return -1;
    }

  /* Move the initial value to var[0], mov_opcode determined above */

  new_op = L_create_new_op (mov_opcode);
  new_op->dest[0] = L_copy_operand (var_array[0]);
  new_op->src[0] = L_copy_operand (acc->dest);
  if (peel_reg->iter_pred[0] != 0)
    new_op->pred[0] = L_new_register_operand (peel_reg->iter_pred[0],
                                              L_CTYPE_PREDICATE,
                                              L_PTYPE_NULL);
  L_insert_oper_before (peel_reg->block, peel_reg->first_op, new_op);

  /* Add/sub the increment*i to each of the vars[i] */
  for (i = 1; i <= acc->count; i++)
    {
      new_op = L_create_new_op (acc->opcode);
      new_op->dest[0] = L_copy_operand (var_array[i]);
      new_op->src[0] = L_copy_operand (acc->dest);
      new_op->src[1] = L_copy_operand (inc_array[i]);
      if (peel_reg->iter_pred[0] != 0)
        new_op->pred[0] = L_new_register_operand (peel_reg->iter_pred[0],
                                                  L_CTYPE_PREDICATE,
                                                  L_PTYPE_NULL);
      L_insert_oper_before (peel_reg->block, peel_reg->first_op, new_op);
    }

  /* Now, go through loop replacing induction variable */
  inc_count = 0;
  term_op = peel_reg->last_op->next_op;
  for (cur_op = peel_reg->first_op; cur_op != term_op; cur_op = next_op)
    {
      /* Get next op now, since may delete cur_op */
      next_op = cur_op->next_op;

      /* if we are at an increment/decrement instruction */
      if (Set_in (acc->ops, cur_op->id))
        {
          inc_count++;
          /* Remove the inc/dec instruction */
          L_nullify_operation (cur_op);
        }
      else
	{
	  /* Replace induction variable in the sources of cur_op */
	  for (c = 0; c < L_max_src_operand; c++)
	    {
	      if (L_same_operand (acc->dest, cur_op->src[c]))
		{
		  L_delete_operand (cur_op->src[c]);
		  cur_op->src[c] = L_copy_operand (var_array[inc_count]);
		}
	    }

	  /* Replace induction variable in the pred of cur_op */

	  if (cur_op->pred[0] && L_same_operand (acc->dest, cur_op->pred[0]))
	    {
	      L_delete_operand (cur_op->pred[0]);
	      cur_op->pred[0] = L_copy_operand (var_array[inc_count]);
	    }

	  /* Handle branches, update value of var if necessary */
	  if (L_general_branch_opcode (cur_op))
	    {
	      if (!Set_in (peel_reg->flow_ops, cur_op->id))
		L_punt ("Lsuper_peel_replace_ind_var: flow_ops is corrupted.");

	      /* Initialize to not needing to insert update before branch */
	      insert_var = DONT_INSERT;

	      /* Get Target of branch */
	      if (L_cond_branch_opcode (cur_op))
		{
		  branch_cb = cur_op->src[2]->value.cb;
		}
	      else if (L_uncond_branch_opcode (cur_op))
		{
		  branch_cb = cur_op->src[0]->value.cb;
		}
	      else
		{
		  /* Hash jump, just assume need to insert update before */
		  insert_var = INSERT_BEFORE;
		  branch_cb = NULL;
		}

	      /* If we have a known target, see if var live at target */
	      if (branch_cb)
		{
		  /* If a backedge, just check live_in of target */
		  if (branch_cb == peel_reg->block)
		    {
		      if (L_in_oper_OUT_set (peel_reg->block, cur_op, 
					     acc->dest, TAKEN_PATH))
			insert_var = INSERT_BEFORE;
		    }
		  /* If not a backedge, create a post loop block if necessary */
		  else
		    {
		      if (L_in_cb_IN_set (branch_cb, acc->dest))
			insert_var = INSERT_AFTER;
		    }
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
		  if (insert_var == INSERT_BEFORE)
		    {
		      L_punt ("Lsuper_peel_replace_ind_var: Unimplemented code.  "
			      "Maybe something wrong");
		      L_insert_oper_before (peel_reg->block, cur_op, new_op);
		    }
		  else
		    {
		      /* JWS 19991221--changed from one_predecessor_cb to 
		       * one_predecessor_arc. When multiple definitions are 
		       * available on different arcs, one would get squashed. 
		       */
		      if (L_exactly_one_predecessor_arc (branch_cb))
			{
			  L_insert_oper_before (branch_cb, branch_cb->first_op,
						new_op);
			}
		      else
			{
			  L_Flow *fl;
			  L_Cb *new_cb;
			  
			  fl = L_find_flow_for_branch (peel_reg->block, cur_op);
			  new_cb = L_split_arc (fn, peel_reg->block, fl);
			  
			  L_insert_oper_before (new_cb, new_cb->first_op, new_op);
			  L_do_flow_analysis (fn, LIVE_VARIABLE);
			}
		    }
		}
	    }
	}

      /* If at end of peels, add in increment statements if needed */
      if (cur_op == peel_reg->last_op)
	{
	  L_Oper *after_op = NULL;
	  L_Cb *target_cb = NULL;

	  if (Set_in (peel_reg->flow_ops, -1))
	    {
	      if (L_in_cb_IN_set (peel_reg->block->next_cb, acc->dest))
		{
		  target_cb = peel_reg->block->next_cb;
		  after_op = NULL;

		  if (!L_exactly_one_predecessor_arc (target_cb))
		    {
		      L_Flow *fl;
		      fl = L_find_last_flow (peel_reg->block->dest_flow);
		      target_cb = L_split_arc (fn, peel_reg->block, fl);
		    }
		}
	    }
	  else if (Set_in (peel_reg->flow_ops, -2))
	    {
	      if (L_is_predicated (cur_op->next_op) ||
		  L_in_oper_IN_set (cur_op->next_op, acc->dest))
		{
		  target_cb = peel_reg->block;
		  after_op = cur_op;
		}
	    }

	  if (target_cb)
	    {
	      int acc_indx, pred_indx, acc_incr;
	      
#if 0
	      if (acc->count % peel_reg->peels)
		L_punt ("Lsuper_peel_replace_ind_var: "
			"Number of peels is not a multiple of accumulators.");
#endif

	      acc_incr = acc->count / peel_reg->peels;
	      for (pred_indx = peel_reg->peels - 1; pred_indx >= 0;
		   pred_indx--)
		{
		  acc_indx = (pred_indx + 1) * acc_incr;
		  new_op = L_create_new_op (mov_opcode);
		  new_op->dest[0] = L_copy_operand (acc->dest);
		  new_op->src[0] = L_copy_operand (var_array[acc_indx]);
		  if (peel_reg->iter_pred[pred_indx] != 0)
		    {
		      new_op->pred[0] =
			L_new_register_operand (peel_reg->
						iter_pred[pred_indx],
						L_CTYPE_PREDICATE,
						L_PTYPE_NULL);
		    }
		  L_insert_oper_after (target_cb, after_op, new_op);
		}
	      /* This is the last thing we can do to a peel region */
	      break;
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

  L_do_flow_analysis (fn, LIVE_VARIABLE);

  return 0;
}

/* 
 *  Adds the sum tree after the cb and op specified
 *
 *  SAM changed to take any number for sum_size, not just power of 2!!
 *  DIA added after_op.  If after_op == NULL then it is put at the front of cb
 */
static void
add_sum_tree_after (L_Cb * cb, L_Oper * after_op, L_Operand ** sum_array,
                    int sum_size, int sum_opcode, L_Operand * dest)
{
  int s, d, i;
  L_Operand **temp_array;
  int temp_size;
  L_Oper *new_op;
  double weight;

  temp_size = sum_size;

  /* Make a copy of sum_array that we can destroy */
  temp_array = (L_Operand **) alloca (temp_size * sizeof (L_Operand *));
  
  for (i = 0; i < sum_size; i++)
    temp_array[i] = sum_array[i];

  /* Get weight to assign to new ops */
  weight = cb->weight;

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

	      new_op->dest[0] = (temp_size == 2) ? L_copy_operand (dest) :
		L_copy_operand (temp_array[s]);

              new_op->src[0] = L_copy_operand (temp_array[s]);
              new_op->src[1] = L_copy_operand (temp_array[s + 1]);
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


static void
Lsuper_peel_replace_accum (L_Func * fn, PeelRegion * peel_reg, AccNode * acc)
{
  int sum_size, i;
  int mov_opcode, sum_opcode;
  int *flow_array, flow_size;
  L_Operand **sum_array, *initial_val;
  L_Oper *new_op, *cur_op, *term_op;

  if (Lsuper_debug_peel_opt)
    {
      fprintf (ERR, "$$$ Eliminating accumulator variable ");
      L_print_operand (ERR, acc->dest, 1);
      fprintf (ERR, " in peel region:\n   first_op %i\n",
               peel_reg->first_op->id);
      fprintf (ERR, "   last_op %i\n", peel_reg->last_op->id);
      fprintf (ERR, "acc->count = %d\n", acc->count);
    }

  /* make temp_size smallest power of 2 >= sum_size */
  sum_size = C_log2 (acc->count);
  sum_size = C_pow2 (sum_size);

  /* Allocate memory for sum_array */
  sum_array = (L_Operand **) alloca (sum_size * sizeof (L_Operand *));

  /* Allocate the tempory registers */
  for (i = 0; i < acc->count; i++)
    sum_array[i] = L_new_register_operand (++L_fn->max_reg_id,
                                           L_return_old_ctype (acc->dest),
                                           L_PTYPE_NULL);

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
      L_punt ("Unknown register type in Lsuper_peel_replace_accum");
      return;
    }

  for (i = 0; i < acc->count; i++)
    {
      new_op = L_create_new_op (mov_opcode);
      new_op->weight = 0.0;
      new_op->dest[0] = L_copy_operand (sum_array[i]);

      /*
       * If first temp sum, copy in current value of sum
       * Otherwise, initialize to 0 or 1
       */
      new_op->src[0] = L_copy_operand ((i == 0) ? acc->dest : initial_val);
      L_insert_oper_before (peel_reg->block, peel_reg->first_op, new_op);
    }

  term_op = peel_reg->last_op->next_op;
  for (cur_op = peel_reg->first_op, i = 0;
       cur_op != term_op;
       cur_op = cur_op->next_op)
    {
      int j;

      if (!Set_in (acc->ops, cur_op->id))
	continue;

      if (i >= acc->count)
        L_punt ("Lsuper_peel_replace_accum: too many acc ops");

      /* Plug in temp accumulator */

      j = L_same_operand (cur_op->dest[0], cur_op->src[0]) ? 0 : 1;

      L_delete_operand (cur_op->src[j]);
      cur_op->src[j] = L_copy_operand (sum_array[i]);

      L_delete_operand (cur_op->dest[0]);
      cur_op->dest[0] = L_copy_operand (sum_array[i]);

      i++;
    }

  /* Determine which flows require compensation code */

  flow_size = Set_size (peel_reg->flow_ops);
  flow_array = alloca (flow_size * sizeof (int));
  Set_2array (peel_reg->flow_ops, flow_array);

  for (i = 0; i < flow_size; i++)
    {
      L_Cb *target_cb;
      L_Oper *branch;

      switch (flow_array[i])
        {
        case -1:
	  target_cb = peel_reg->block->next_cb;
          if (!L_in_cb_IN_set (target_cb, acc->dest))
	    flow_array[i] = -3;
	  break;

	case -2:
	  /* IN set test is insufficient if the op tested is predicated
	   * This fix is conservative... but dead compensation code should
	   * be removed by subsequent optimization.
	   * JWS - 20021014 
	   */
	  if (!L_in_oper_IN_set (peel_reg->last_op->next_op, acc->dest) &&
	      !L_is_predicated (peel_reg->last_op->next_op))
	    flow_array[i] = -3;
	  break;

	default:
	  branch = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						      flow_array[i]);
	  target_cb = L_find_flow_for_branch (peel_reg->block, branch)->dst_cb;
	  if (!L_in_cb_IN_set (target_cb, acc->dest))
	    flow_array[i] = -3;
	}
    }

  /* Insert compensation code */

  for (i = 0; i < flow_size; i++)
    {
      L_Cb *target_cb;
      L_Oper *branch, *after_op = NULL;
      L_Flow *fl;

      switch (flow_array[i])
        {
        case -1:
	  target_cb = peel_reg->block->next_cb;
	  if (!L_exactly_one_predecessor_cb (target_cb))
	    {
	      fl = L_find_last_flow (peel_reg->block->dest_flow);
	      target_cb = L_split_arc (fn, peel_reg->block, fl);
	    }
	  break;

        case -2:
	  target_cb = peel_reg->block;
	  after_op = peel_reg->last_op;
	  break;

	case -3:
	  /* No compensation code required, as accumulator is not
	   * live here.  Removed in previous switch.
	   */
	  continue;

        default:
	  branch = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl, 
					      flow_array[i]);

	  fl = L_find_flow_for_branch (peel_reg->block, branch);
	  target_cb = fl->dst_cb;

	  if (!L_exactly_one_predecessor_cb (target_cb))
	    target_cb = L_split_arc (fn, peel_reg->block, fl);
        }

      add_sum_tree_after (target_cb, after_op, sum_array, sum_size,
			  sum_opcode, acc->dest);
    }

  /* free up space */
  for (i = 0; i < sum_size; i++)
    if (sum_array[i])
      L_delete_operand (sum_array[i]);

  L_delete_operand (initial_val);

  L_do_flow_analysis (fn, LIVE_VARIABLE);
  return;
}


static void
Lsuper_do_accum_exp (L_Func * fn, PeelRegion * peel_region)
{
  L_Oper *oper, *term_op;
  int potential_accum, src2_allowed, c;
  Set used = NULL, defined = NULL, accumed = NULL;
  AccNode *acc;

  /* Remove any accum lists hanging around */
  free_accum_list ();

  term_op = peel_region->last_op->next_op;
  for (oper = peel_region->first_op; oper != term_op;
       oper = oper->next_op)
    {
      potential_accum = 0;

      /* Is it an opcode that an accumulator can be? */
      if (L_add_opcode (oper) || L_sub_opcode (oper) ||
          L_mul_opcode (oper) ||
          (L_div_opcode (oper) && !L_int_div_opcode (oper)))
        {
          /* If sub or div, accumulator must be src1 */

	  src2_allowed = !(L_sub_opcode (oper) || L_div_opcode (oper));

          /* Does it have the form of an accumulator? */
          if (L_is_register (oper->dest[0]) &&
              (L_same_operand (oper->dest[0], oper->src[0]) ||
               (src2_allowed && L_same_operand (oper->dest[0], oper->src[1])))
              && !L_same_operand (oper->src[0], oper->src[1]))
            {
              /* Add to list of potential accumulators */
              add_accum (oper, peel_region);

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

      if (potential_accum == 0)
        {
          for (c = 0; c < L_max_dest_operand; c++)
            {
              if (L_is_register (oper->dest[c]))
                defined = Set_add (defined, oper->dest[c]->value.r);
            }
          for (c = 0; c < L_max_src_operand; c++)
            {
              if (L_is_register (oper->src[c]))
                used = Set_add (used, oper->src[c]->value.r);
            }
          for (c = 0; c < 1 /* L_max_pred_operand */ ; c++)
            {
              if (L_is_register (oper->pred[c]))
                used = Set_add (used, oper->pred[c]->value.r);
            }
        }
    }


  /* Now go through the linked list of potential accumulators. */
  acc = acc_list_head;
  while (acc != NULL)
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
      if ((acc->same_src == 1) &&
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

        cnt_livein = Set_size (L_get_cb_IN_set(peel_region->block));
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

      if ((acc->count > 1) && (acc->eligible == 1) &&
	  !Set_in (defined, acc->dest->value.r))
	{
	  /*
	   * Do not use the potental accumulators that are ineligible for one
	   * of the following reasons:
	   * 1) Only one accumulator in the loop (no opti possible)
	   * 2) There is two opr more types of accumulators using the
	   *    same register (the eligible flag flags this)
	   * 3) The accumulator register is used somewhere else in the loop.
	   * 4) SAM- if accumulator is just accumulating numeric constant,
	   *        use induction expansion it is more efficient.
	   *
	   * Do induction variable expansion if the source is constant
	   * and the destination is not redefined anywhere in the loop.
	   */
	  if ((!((src_numeric_constant == 1) && 
		 (add_sub_opcode == 1))) &&
	      !Set_in (used, acc->dest->value.r))
	    Lsuper_peel_replace_accum (fn, peel_region, acc);
	  else if ((src_constant == 1) &&
		   (add_sub_opcode == 1) &&
		   (acc->bad_pred_flag == 0))
	    Lsuper_peel_replace_ind_var (fn, peel_region, acc);
	}

      acc = acc->next;
    }

  /* Free memory used */
  free_accum_list ();
  Set_dispose (used);
  Set_dispose (defined);
  Set_dispose (accumed);
  return;
}


/*
 *      Export function
 */
void
Lsuper_peel_accumulator_expansion (L_Func * fn)
{
  L_Cb *cb_indx;
  PeelRegion *p_reg_indx;

  if (fn->weight == 0.0)
    return;

  if (Lsuper_debug_peel_opt)
    {
      fprintf (ERR, "Enter peel accumulator exp...\n");
    }

  for (cb_indx = fn->first_cb; cb_indx; cb_indx = cb_indx->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb_indx->flags, L_CB_HYPERBLOCK) &&
	  !L_find_attr (cb_indx->attr, "tail"))
        {
          peel_region_head = create_peel_region_list (cb_indx);

          for (p_reg_indx = peel_region_head; p_reg_indx;
               p_reg_indx = p_reg_indx->next)
            {
              if (Lsuper_debug_peel_opt)
                print_peel_region (p_reg_indx);

              Lsuper_do_accum_exp (fn, p_reg_indx);
            }

	  peel_region_head = free_peel_region_list (peel_region_head);
        }
    }

  return;
}
