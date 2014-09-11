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
 *      File :          l_int_range.c
 *      Description :   Integer range analysis utilities
 *      Creation Date : March 10, 1998
 *      Authors :       John W. Sias
 *
 *      (C) Copyright 1998, John W. Sias and Wen-Mei W. Hwu
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/l_alloc_new.h>
#include "l_int_range.h"

#define IR_DEBUG                  1
#define IR_DEBUG_RANGE_ENUM       IR_DEBUG && (IR_verbose_level >= 2)
#define IR_DEBUG_POSSIBLE_VALUES  IR_DEBUG && (IR_verbose_level >= 1)
#undef IR_DEBUG_MINTERM_IMPLICATION

#define ERR stderr
#define LIR_REG_HASH_SIZE 64

L_Alloc_Pool *L_alloc_int_range_nodes = NULL;
L_Alloc_Pool *L_alloc_int_range = NULL;
L_Alloc_Pool *L_alloc_range_table = NULL;
L_Alloc_Pool *L_alloc_comp_inst = NULL;
L_Alloc_Pool *L_alloc_range_graph = NULL;

int IR_verbose_level = 0;

static IR_Node *IR_new_node (ITuintmax lower, ITuintmax upper);
static IR_Range *IR_new_int_range (int bits, int init_to_true);
static void IR_delete_range (IR_Range * range);
static void IR_zero_range (IR_Range * range);
static int IR_range_subsumes_node (IR_Range * range, IR_Node * node);
static int IR_range_exhaustive (IR_Range * range);

/*===========================================================================*/
/*
 * Const-const comparisons
 */
/*===========================================================================*/


/* IR_cond_const_val
 * ----------------------------------------------------------------------------
 * If a condition is between constants, it has a constant value.  Given
 * a pred comparison, return 0 or 1 if the result is FALSE or TRUE,
 * respectively, or -1 if the value cannot be determined by this method to be
 * constant.                    -- JWS 20000109
 */

int
IR_cond_const_val (L_Oper * oper)
{
  ITintmax v0, v1;
  int s_gt, s_lt, u_gt, u_lt;

  if (!oper)
    L_punt ("PG_cond_const_val: NULL oper");

  if (oper->src[0] && oper->src[1] &&
      L_same_operand(oper->src[0], oper->src[1]))
    {
      if ((oper->com[1] == Lcmp_COM_EQ) ||
	  (oper->com[1] == Lcmp_COM_GE) ||
	  (oper->com[1] == Lcmp_COM_LE))
	return 1;
      else if ((oper->com[1] == Lcmp_COM_NE) ||
	       (oper->com[1] == Lcmp_COM_GT) ||
	       (oper->com[1] == Lcmp_COM_LT))
	return 0;
    }

  if (!L_int_pred_comparison_opcode (oper))
    return -1;

  if (!oper->src[0] || !oper->src[1])
    L_punt ("PG_cond_const_val: SRC missing");

  if (!L_is_int_constant (oper->src[0]) || !L_is_int_constant (oper->src[1]))
    return -1;

  if (oper->com[0] == L_CTYPE_INT)
    {
      v0 = (int) oper->src[0]->value.i;
      v1 = (int) oper->src[1]->value.i;
    }
  else if (oper->com[0] == L_CTYPE_UINT)
    {
      v0 = (unsigned int) oper->src[0]->value.i;
      v1 = (unsigned int) oper->src[1]->value.i;
    }
  else
    {
      v0 = (ITintmax) oper->src[0]->value.i;
      v1 = (ITintmax) oper->src[1]->value.i;
    }

  s_gt = (v0 > v1);
  s_lt = (v0 < v1);
  u_gt = ((ITuintmax) v0 > (ITuintmax) v1);
  u_lt = ((ITuintmax) v0 < (ITuintmax) v1);

  switch (oper->com[1])
    {
    case Lcmp_COM_EQ:
      return (!s_gt && !s_lt);

    case Lcmp_COM_NE:
      return (s_gt || s_lt);

    case Lcmp_COM_GT:
      if (L_ctype_is_signed (oper->com[0]))
        return (s_gt);
      else
        return (u_gt);

    case Lcmp_COM_GE:
      if (L_ctype_is_signed (oper->com[0]))
        return (!s_lt);
      else
        return (!u_lt);

    case Lcmp_COM_LT:
      if (L_ctype_is_signed (oper->com[0]))
        return (s_lt);
      else
        return (u_lt);

    case Lcmp_COM_LE:
      if (L_ctype_is_signed (oper->com[0]))
        return (!s_gt);
      else
        return (!u_gt);

    case Lcmp_COM_TN:
      if (v0 & ((ITuintmax) 1 << v1))
        return 1;
      else
        return 0;

    case Lcmp_COM_TZ:
      if (v0 & ((ITuintmax) 1 << v1))
        return 0;
      else
        return 1;

    default:
      return -1;
    }
}


/*===========================================================================*/
/*
 * IR structure manipulation
 */
/*===========================================================================*/

/*
 * Create a new integer range node with the specified lower and upper bounds
 */
static IR_Node *
IR_new_node (ITuintmax lower, ITuintmax upper)
{
  IR_Node *new_node;

  if (upper < lower)
    L_punt ("IR_new_node: attempt to create invalid node.");

  if (!L_alloc_int_range_nodes)
    L_alloc_int_range_nodes = L_create_alloc_pool ("IR_Node",
                                                   sizeof (IR_Node), 64);
  new_node = (IR_Node *) L_alloc (L_alloc_int_range_nodes);

  new_node->lower = lower;
  new_node->upper = upper;

  return new_node;
}

/*
 *  IR_Range Functions: Integer range manipulation
 *  --------------------------------------------------------------------------
 */

static IR_Range *
IR_new_int_range (int bits, int init_to_true)
{
  IR_Range *range;
  IR_Node *new_node = NULL;

  if (bits != 32)
    {
#ifdef IT64BIT
      if (bits != 64)
#endif
        L_punt ("IR_new_int_range: Unhandled range bitwidth %d.", bits);
    }

  if (!L_alloc_int_range)
    L_alloc_int_range = L_create_alloc_pool ("IR_Range",
                                             sizeof (IR_Range), 32);

  range = (IR_Range *) L_alloc (L_alloc_int_range);

  range->node_list = NULL;
  range->instance_list = NULL;
  range->reg_operand = NULL;

  range->bits = bits;

  if (init_to_true)
    {
      if (bits == 32)
        new_node = IR_new_node (0, ITMAXU32);
#ifdef IT64BIT
      else if (bits == 64)
        new_node = IR_new_node (0, ITMAXU64);
#endif
      else
        L_punt ("IR_new_int_range: Unhandled range bitwidth %d.", bits);

      range->node_list = List_insert_first (range->node_list, new_node);
    }

  return range;
}

/*
 * Delete the specified integer range and associated nodes
 */
static void
IR_delete_range (IR_Range * range)
{
  if (!range)
    L_punt ("IR_delete_range: Attempt to delete NULL range.");

  IR_zero_range (range);

  L_free (L_alloc_int_range, range);

  return;
}

static void
IR_zero_range (IR_Range * range)
{
  List node_list;
  List inst_list;
  IR_Node *node_indx;
  IR_Comp_Inst *inst_indx;

  if (!range)
    L_punt ("IR_zero_range: Attempt to zero NULL range.");

  node_list = range->node_list;
  inst_list = range->instance_list;

  List_start (node_list);
  while ((node_indx = (IR_Node *) List_next (node_list)))
    L_free (L_alloc_int_range_nodes, node_indx);

  List_reset (node_list);


  List_start (inst_list);
  while ((inst_indx = (IR_Comp_Inst *) List_next (inst_list)))
    L_free (L_alloc_comp_inst, inst_indx);

  List_reset (inst_list);

  return;
}

/* IR_range_subsumes_node
 * ----------------------------------------------------------------------------
 * Return TRUE if the node specified is subsumed by the range, FALSE otherwise.
 */
static int
IR_range_subsumes_node (IR_Range * range, IR_Node * node)
{
  IR_Node *node_indx;
  List node_list;
  int subsumed;

  node_list = range->node_list;

  subsumed = FALSE;
  List_start (node_list);
  while ((node_indx = (IR_Node *) List_next (node_list)))
    {
      if (node->lower > node_indx->upper)
        continue;
      if (node->lower < node_indx->lower)
        break;
      if (node->upper > node_indx->upper)
        break;
      subsumed = TRUE;
      break;
    }

  return subsumed;
}

/* IR_range_exhaustive
 * ----------------------------------------------------------------------------
 * Return TRUE if the range nodes cover all integers, FALSE otherwise.
 */
static int
IR_range_exhaustive (IR_Range * range)
{
  IR_Node *node_indx;
  List node_list;
  ITuintmax next_unseen = 0;

  if (!range)
    L_punt ("IR_range_exhaustive: NULL range.");

  node_list = range->node_list;

  if (!List_size (node_list))
    return FALSE;

  List_start (node_list);
  while ((node_indx = (IR_Node *) List_next (node_list)))
    {
      if (node_indx->lower != next_unseen)
        break;
      else
        next_unseen = node_indx->upper + 1;
    }

  if (range->bits == 32)
    next_unseen = next_unseen & ITMAXU32;

  if (!next_unseen && !node_indx)
    return TRUE;
  else
    return FALSE;
}

/* IR_split_node
 * ----------------------------------------------------------------------------
 * Split the current node of the specified range at the value split_val
 */
static void
IR_split_node (IR_Range * range, ITuintmax split_val)
{
  IR_Node *orig_node, *new_node;
  List node_list;

  if (!range)
    L_punt ("IR_split_node: NULL range");

  node_list = range->node_list;

  orig_node = List_current (node_list);
  if (split_val <= orig_node->lower)
    return;
  new_node = IR_new_node (split_val, orig_node->upper);
  orig_node->upper = split_val - 1;
  node_list = List_insert_next (node_list, new_node);

  range->node_list = node_list;

  return;
}

/* IR_add_disj_nodes
 * ----------------------------------------------------------------------------
 * Add the specified range in such a manner that the nodes represent minterms
 */
static void
IR_add_disj_nodes (IR_Range * range, ITuintmax lower, ITuintmax upper)
{
  List node_list;
  IR_Node *new_node, *node_indx;

  if (!range)
    L_punt ("IR_add_disj_nodes: NULL range.");
  if (lower > upper)
    L_punt ("IR_add_disj_nodes: invalid range.");

  node_list = range->node_list;

  List_start (node_list);

  if (!List_size (node_list))
    {
      new_node = IR_new_node (lower, upper);
      range->node_list = List_insert_first (node_list, new_node);
      return;
    }

  while ((node_indx = (IR_Node *) List_next (node_list)))
    {
      if (lower > node_indx->upper)
        continue;
      if (lower < node_indx->lower)
        {
          if (upper < node_indx->lower)
            {
              new_node = IR_new_node (lower, upper);
              range->node_list = List_insert_prev (node_list, new_node);
              break;
            }
          else
            {
              new_node = IR_new_node (lower, node_indx->lower - 1);
              range->node_list = List_insert_prev (node_list, new_node);
              lower = node_indx->lower;
            }
        }
      else if (lower > node_indx->lower)
        {
          IR_split_node (range, lower);
          node_indx = List_next (node_list);
        }
      if (lower == node_indx->lower)
        {
          if (upper > node_indx->upper)
            {
              lower = node_indx->upper + 1;
            }
          else if (upper < node_indx->upper)
            {
              IR_split_node (range, upper + 1);
              break;
            }
          else
            {
              break;
            }
        }
    }

  if (!List_current (node_list))
    {
      new_node = IR_new_node (lower, upper);
      range->node_list = List_insert_last (node_list, new_node);
      return;
    }

  return;
}

/*
 * IR_insert_inst
 * ----------------------------------------------------------------------------
 * Add a new instance record to an integer range structure
 */
void
IR_insert_inst (IR_Range * range, L_Cb * cb, L_Oper * oper,
                L_Operand * operand)
{
  IR_Comp_Inst *new_inst;

  if (!L_alloc_comp_inst)
    L_alloc_comp_inst = L_create_alloc_pool ("IR_Comp_Inst",
                                             sizeof (IR_Comp_Inst), 64);
  new_inst = (IR_Comp_Inst *) L_alloc (L_alloc_comp_inst);
  new_inst->cb = cb;
  new_inst->oper = oper;
  new_inst->reg_operand = operand;
  range->instance_list = List_insert_last (range->instance_list, new_inst);
  return;
}

/*
 * IR_form_disjunctives
 * ----------------------------------------------------------------------------
 * Given a comparison oper and a disjunctive integer range, modify
 * the range such that the disjoint nodes represent minterms in an expression
 * formulated on all the conditions added to the range.
 */
void
IR_form_disjunctives (IR_Range * range, L_Oper * oper)
{
  int const_first;
  ITuint8 compare_type;
  ITuintmax compare_const;
  ITuintmax range_max, range_plus, range_minus;
  L_Operand *cmp_reg_operand;

  if ((!oper) || (!range))
    L_punt ("IR_form_disjunctives: NULL operand");
  if (!L_is_reg_const_pred_compare (oper) && !L_is_reg_const_cond_br (oper))
    L_punt ("IR_form_disjunctives called on inappropriate oper.");

  const_first = L_pred_compare_const_first (oper);
  compare_const = L_pred_compare_get_int_const (oper);
  cmp_reg_operand = L_pred_compare_get_reg_operand (oper);

  if (range->bits == 32)
    {
      range_minus = ITMINS32;
      range_plus = ITMAXS32;
      range_max = ITMAXU32;
      compare_const = compare_const & ITMAXU32;
    }
#ifdef IT64BIT
  else if (range->bits == 64)
    {
      range_minus = ITMINS64;
      range_plus = ITMAXS64;
      range_max = ITMAXU64;
    }
#endif
  else
    {
      L_punt ("IR_form_disjunctives: Range with bad bits");
      return;                   /* L_punt does not return */
    }
  compare_type = oper->com[1];

  if (const_first)
    compare_type = L_inverse_pred_completer (compare_type);

  if (!L_is_ctype_signed_direct (oper->com[0]) &&
      L_inequality_compare(oper))
    compare_type |= LIR_UNSIGNED;

  switch (compare_type)
    {
    case LIR_EQ:
      IR_add_disj_nodes (range, compare_const, compare_const);
      break;

    case LIR_NE:
      if (compare_const != 0)
        IR_add_disj_nodes (range, 0, compare_const - 1);
      if (compare_const != range_max)
        IR_add_disj_nodes (range, compare_const + 1, range_max);
      break;

    case LIR_GT:
      if (compare_const == range_plus)
        break;
      else if (compare_const < range_plus)
        {
          /* Positive constant */
          IR_add_disj_nodes (range, compare_const + 1, range_plus);
        }
      else
        {
          /* Negative constant */
          if (compare_const != range_max)
            IR_add_disj_nodes (range, compare_const + 1, range_max);
          IR_add_disj_nodes (range, 0, range_plus);
        }
      break;

    case LIR_GE:
      if (compare_const <= range_plus)
        {
          /* Positive constant */
          IR_add_disj_nodes (range, compare_const, range_plus);
        }
      else
        {
          /* Negative constant */
          IR_add_disj_nodes (range, compare_const, range_max);
          IR_add_disj_nodes (range, 0, range_plus);
        }
      break;

    case LIR_LT:
      if (compare_const == range_minus)
        break;
      else if (compare_const <= range_plus)
        {
          /* Positive constant */
          if (compare_const != 0)
            IR_add_disj_nodes (range, 0, compare_const - 1);
          IR_add_disj_nodes (range, range_minus, range_max);
        }
      else
        {
          /* Negative constant */
          IR_add_disj_nodes (range, range_minus, compare_const - 1);
        }
      break;

    case LIR_LE:
      if (compare_const <= range_plus)
        {
          /* Positive constant */
          IR_add_disj_nodes (range, 0, compare_const);
          IR_add_disj_nodes (range, range_minus, range_max);
        }
      else
        {
          /* Negative constant */
          IR_add_disj_nodes (range, range_minus, compare_const);
        }
      break;

    case LIR_GT_U:
      if (compare_const != range_max)
        IR_add_disj_nodes (range, compare_const + 1, range_max);
      break;
    case LIR_GE_U:
      IR_add_disj_nodes (range, compare_const, range_max);
      break;
    case LIR_LT_U:
      if (compare_const != 0)
        IR_add_disj_nodes (range, 0, compare_const - 1);
      break;
    case LIR_LE_U:
      IR_add_disj_nodes (range, 0, compare_const);
      break;
    default:
      L_punt ("IR_form_disjunctives: Unhandled compare_type: %d.", compare_type);
    }

  return;
}

/*===========================================================================*/
/*
 *  IR_Range_Table functions
 */
/*===========================================================================*/

/*
 * IR_alloc_range_table
 * ----------------------------------------------------------------------------
 * Allocate the table that maps comparison register operands and their
 * context to integer ranges
 */
IR_Range_Table *
IR_alloc_range_table (void)
{
  IR_Range_Table *table;

  if (!L_alloc_range_table)
    L_alloc_range_table = L_create_alloc_pool ("IR_Range_Table",
                                               sizeof (IR_Range_Table), 4);
  table = (IR_Range_Table *) L_alloc (L_alloc_range_table);

  table->hash_reg_to_range_list = HashTable_create (LIR_REG_HASH_SIZE);

  table->disj_node_count = 0;

  table->rg = NULL;

  return table;
}

/* IR_build_range_table
 * ----------------------------------------------------------------------------
 * Create the table that maps comparison register operands and their
 * context to integer ranges
 */
IR_Range_Table *
IR_build_range_table (L_Func * fn, PG_Pred_Graph * pg)
{
  IR_Range_Table *table;
  List cond_list, range_list;
  L_Cb *cb;
  L_Oper *oper;
  L_Operand *reg_operand;
  IR_Range *range;
  IR_Comp_Inst *prev_inst;
  PG_Cond *cond_indx;
  char comp_ctype;
  int bits;

  table = IR_alloc_range_table ();

  if (!pg)
    L_punt ("NULL pred graph");

  cond_list = pg->pg_conds;

  List_start (cond_list);

  while ((cond_indx = (PG_Cond *) List_next (cond_list)))
    {
      oper = cond_indx->first_oper;

      cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl, oper->id);

      if (!L_is_reg_const_pred_compare (oper) &&
          !L_is_reg_const_cond_br (oper))
        continue;

      if (L_gen_tz_cmp_opcode (oper) || L_gen_tn_cmp_opcode (oper))
        continue;

      reg_operand = L_pred_compare_get_reg_operand (oper);

      comp_ctype = oper->com[0];

      if (L_is_size_int_direct (comp_ctype))
        bits = 32;
      else if (L_is_size_llong_direct (comp_ctype))
        bits = 64;
      else
        {
          L_punt ("IR_build_range_table: Invalid compare ctype");
          return (NULL);
        }
      range_list = (List)
        HashTable_find_or_null (table->hash_reg_to_range_list,
                                L_REG_MAC_INDEX (reg_operand));

      List_start (range_list);
      while ((range = (IR_Range *) List_next (range_list)))
        {
          if (range->bits != bits)
            continue;

          prev_inst = (IR_Comp_Inst *) List_first (range->instance_list);

          if (PG_equiv_operand (prev_inst->cb, prev_inst->oper,
                                prev_inst->reg_operand,
                                cb, oper, reg_operand))
            break;
        }

      if (!range)
        {
          range = IR_new_int_range (bits, 0);
          range->reg_operand = reg_operand;

          if (range_list)
            /* Depends on list not being moved in insert_last */
            List_insert_last (range_list, range);
          else
            {
              range_list = List_insert_first (range_list, range);
              HashTable_insert (table->hash_reg_to_range_list,
                                L_REG_MAC_INDEX (reg_operand),
                                (void *) range_list);
            }
        }

      IR_insert_inst (range, cb, oper, reg_operand);
      IR_form_disjunctives (range, oper);

      cond_indx->range = range;
    }

  return table;
}

void
IR_delete_range_graph (IR_Range_Graph * graph, int deallocate)
{
  List cond_list;
  PG_Cond *cond_indx;

  cond_list = PG_pred_graph->pg_conds;

  List_start (cond_list);
  while ((cond_indx = (PG_Cond *) List_next (cond_list)))
    cond_indx->range_node = NULL;

  if (deallocate && graph->dd)
    {
      Cudd_Quit (graph->dd);
      graph->dd = NULL;
    }

  L_free (L_alloc_range_graph, graph);

  return;
}

/* IR_delete_range_table
 * ----------------------------------------------------------------------------
 * Using the cond list from the specified pred graph, delete the range table.
 * Nullify pointers to the range table in the cond list.
 */
void
IR_delete_range_table (IR_Range_Table * table, PG_Pred_Graph * pg)
{
  List range_list, cond_list;
  IR_Range *range;
  PG_Cond *cond_indx;

  if (!table)
    L_punt ("IR_delete_range_table: Attempt to dispose NULL range table.");
  cond_list = pg->pg_conds;

  List_start (cond_list);
  while ((cond_indx = (PG_Cond *) List_next (cond_list)))
    cond_indx->range = NULL;

  HashTable_start (table->hash_reg_to_range_list);

  while ((range_list = (List) HashTable_next (table->hash_reg_to_range_list)))
    {
      List_start (range_list);
      while ((range = List_next (range_list)))
        IR_delete_range (range);
      List_reset (range_list);
    }

  HashTable_free (table->hash_reg_to_range_list);

  L_free (L_alloc_range_table, table);

#if 0
  if (L_alloc_int_range_nodes)
    L_print_alloc_info (stderr, L_alloc_int_range_nodes, 0);
  if (L_alloc_int_range)
    L_print_alloc_info (stderr, L_alloc_int_range, 0);
  if (L_alloc_range_table)
    L_print_alloc_info (stderr, L_alloc_range_table, 0);
  if (L_alloc_comp_inst)
    L_print_alloc_info (stderr, L_alloc_comp_inst, 0);
  if (L_alloc_range_graph)
    L_print_alloc_info (stderr, L_alloc_range_graph, 0);
#endif

  return;
}


/* IR_build_domain
 * ----------------------------------------------------------------------------
 * Constructs BDD nodes representing the disjoint segments of a range.
 * Each disjoint segment receives a pointer to a BDD node with a 
 * reference count of one.
 */
static void
IR_build_domain (DdManager *dd, IR_Range *range)
{
  List disj_node_list;
  IR_Node *disj_node_indx;
  int range_exhaustive, indx, domain_nodes;
  DdNode **domain_node;

  disj_node_list = range->node_list;

  domain_nodes = List_size (disj_node_list);
	  
  range_exhaustive = IR_range_exhaustive (range);

  indx = 0;

  if (!range_exhaustive)
    {
      domain_nodes++;
      indx++;
    }
	  
  domain_node = (DdNode **) alloca (domain_nodes *
				    sizeof (DdNode *));

  I_BDD_new_mutex_domain(dd, domain_node, domain_nodes);

  List_start (disj_node_list);
  while ((disj_node_indx = (IR_Node *) List_next (disj_node_list)))
    {
      if (IR_DEBUG_RANGE_ENUM)
	{
	  fprintf (stderr, " %4d: ", indx);
	  if (disj_node_indx->lower != disj_node_indx->upper)
	    fprintf (stderr, " " ITuintmaxformat
		     "-" ITuintmaxformat "\n",
		     disj_node_indx->lower, disj_node_indx->upper);
	  else
	    fprintf (stderr, ITintmaxformat
		     "\n", disj_node_indx->lower);
	}
	      
      disj_node_indx->bdd_node = domain_node[indx];
      disj_node_indx->id = indx++;
    }
	
  if (!range_exhaustive)
    {
      /* Node 0 of the mutex is not assigned to any range if the set
       * of ranges is not exhaustive of all outcomes.  Reduce its
       * reference count here to prevent a leak.
       */

      Cudd_RecursiveDeref(dd, domain_node[0]);
    }
  
  range->count = indx;
  return;
}

/* IR_enumerate_disj_ranges
 * ----------------------------------------------------------------------------
 * Create finite domain sub-bdds for each range.
 */
void
IR_enumerate_disj_ranges (IR_Range_Table * table)
{
  HashTable hash_reg_to_range_list;
  IR_Range *range;
  List range_list;
  DdManager *dd;
  DdNode *one, *zero;

  dd = table->rg->dd;
  one = table->rg->one;
  zero = table->rg->zero;

  hash_reg_to_range_list = table->hash_reg_to_range_list;

  HashTable_start (hash_reg_to_range_list);

  while ((range_list = (List) HashTable_next (hash_reg_to_range_list)))
    {
      List_start (range_list);
      while ((range = (IR_Range *) List_next (range_list)))
        {
          if (IR_DEBUG_RANGE_ENUM)
            {
              fprintf (stderr, "Range on ");
              L_print_operand (stderr, range->reg_operand, 0);
              fprintf (stderr, "\n");
            }
	  IR_build_domain (dd, range);
        }
    }
  if (IR_DEBUG_RANGE_ENUM)
    fprintf (stderr, "\n\n");

  return;
}

/*
 * IR_new_range_graph
 * ----------------------------------------------------------------------------
 * Allocate and initialize a new range graph, using the specified DdManager
 * or starting a new one if pased NULL.
 */
IR_Range_Graph *
IR_new_range_graph (L_Func * fn, DdManager * use_dd)
{
  IR_Range_Graph *range_graph;

  if (!L_alloc_range_graph)
    L_alloc_range_graph = L_create_alloc_pool ("IR_Range_Graph",
                                               sizeof (IR_Range_Graph), 1);
  range_graph = (IR_Range_Graph *) L_alloc (L_alloc_range_graph);
  range_graph->fn = fn;

  if (!use_dd)
    use_dd = Cudd_Init (0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);

  range_graph->dd = use_dd;

  range_graph->one = Cudd_ReadOne (use_dd);
  range_graph->zero = Cudd_ReadLogicZero (use_dd);

  return range_graph;
}

/*
 * IR_build_range_bdd
 * ----------------------------------------------------------------------------
 * Using a range table, construct finite domains for condition families.
 * If basis is TRUE, copy pointers to the range nodes to the cond nodes.
 */
IR_Range_Graph *
IR_build_range_bdd (L_Func *fn, IR_Range_Table *table,
                    PG_Pred_Graph *pg, int basis)
{
  IR_Range *range, *cond_range;
  List inst_list, disj_node_list;
  IR_Comp_Inst *inst_indx;
  IR_Node *disj_node_indx;
  IR_Range_Graph *rg = NULL;

  PG_Cond *pg_cond;
  DdManager *dd = NULL;
  DdNode *range_node, *old_range_node, *one, *zero;

  if (!table || !fn || !pg)
    return NULL;

  rg = IR_new_range_graph (fn, pg->dd);
  dd = rg->dd;
  one = rg->one;
  zero = rg->zero;

  table->rg = rg;

  pg->ir_range_graph = rg;

  /* Build range bdd's */

  IR_enumerate_disj_ranges (table);

  /* Add cond nodes */

  List_start (pg->pg_conds);
  while ((pg_cond = (PG_Cond *) List_next (pg->pg_conds)))
    {
      pg_cond->range_node = NULL;

      if (basis)
        pg_cond->cond_indx = -1;

      if (!pg_cond->range)
        {
	  /* A range structure does not exist for this condition */
	  
          if (basis)
            {
              int cval;

              if (pg_cond->first_oper)
                cval = IR_cond_const_val (pg_cond->first_oper);
              else
                cval = -1;

              if (cval == 1)
                pg_cond->node = one;
              else if (cval == 0)
                pg_cond->node = zero;
              else
                pg_cond->node = Cudd_bddNewVar (dd);

	      Cudd_Ref (pg_cond->node);
            }
        }
      else if (!pg_cond->range_node)
	{
	  range = pg_cond->range;
	  inst_list = range->instance_list;

	  List_start (inst_list);
	  while ((inst_indx = (IR_Comp_Inst *) List_next (inst_list)))
	    if (pg_cond->first_oper == inst_indx->oper)
	      break;

	  if (!inst_indx)
	    L_punt ("IR_build_range_bdd: instance not found");

	  range_node = zero;
	  Cudd_Ref (range_node);

	  /* 
	   * Iterate through range nodes, or with those
	   * which are subsets
	   */

	  cond_range = IR_new_int_range (range->bits, 0);
		  
	  IR_form_disjunctives (cond_range, inst_indx->oper);

	  disj_node_list = range->node_list;

	  List_start (disj_node_list);
	  while ((disj_node_indx = (IR_Node *) List_next (disj_node_list)))
	    if (IR_range_subsumes_node (cond_range, disj_node_indx))
	      {
		range_node = Cudd_bddOr (dd, disj_node_indx->bdd_node, 
					 old_range_node = range_node);
		Cudd_Ref (range_node);
		Cudd_RecursiveDeref (dd, old_range_node);
	      }

	  pg_cond->range_node = range_node;

	  if (basis)
	    {
	      pg_cond->node = range_node;
	      if (!range_node)
		L_punt ("No node assigned!");
	    }

	  IR_delete_range (cond_range);
	}
    }

  if (basis)
    {
      /* Clean up unused disjoint segment nodes */

      while ((pg_cond = (PG_Cond *) List_next (pg->pg_conds)))
	{
	  if (!(range = pg_cond->range))
	    continue;

	  disj_node_list = range->node_list;

	  List_start (disj_node_list);
	  while ((disj_node_indx = (IR_Node *) List_next (disj_node_list)))
	    {
	      if (!(range_node = disj_node_indx->bdd_node))
		break;
	      Cudd_RecursiveDeref (dd, range_node);
	      disj_node_indx->bdd_node = NULL;
	    }
	}
    }

  return (rg);
}

void
IR_deref_basis_functions (PG_Pred_Graph * pg)
{
  PG_Cond *pg_cond;

  while ((pg_cond = (PG_Cond *) List_next (pg->pg_conds)))
    if (pg_cond->range_node)
      Cudd_RecursiveDeref (pg->dd, pg_cond->range_node);

  return;
}

/*===========================================================================*/
/*
 * Query interface to minterm-based predicate optimization.
 */
/*===========================================================================*/

/*
 * IR_possible_values
 * ----------------------------------------------------------------------------
 *      Returns  0 if the minterm can only be FALSE.
 *               1 if the minterm can only be TRUE.
 *               2 if the minterm can be TRUE or FALSE.
 *              -1 if the input predicates combination is not possible
 */
int
IR_possible_values (char *mins, int terms, int min_indx)
{
  int term_indx;
  DdManager *dd;
  DdNode *one, *cur_node, *old_node;
  PG_Pred_Graph *pg;
  PG_Cond *pg_cond;
  int rv, range_terms = 0;

  pg = PG_pred_graph;
  dd = pg->ir_range_graph->dd;
  one = pg->ir_range_graph->one;
  cur_node = pg->ir_range_graph->zero;

  if (IR_DEBUG_POSSIBLE_VALUES)
    range_terms = I_BDD_num_terms (pg->ir_range_graph->dd);

  if (IR_DEBUG_POSSIBLE_VALUES)
    {
      fprintf (stderr, "IR_possible_values\n");
      DB_print_minterm (terms, mins);
      fprintf (stderr, "indx = %d.\n", min_indx);
    }

  Cudd_Ref (cur_node);
  for (term_indx = 0; term_indx < terms; term_indx++)
    {
      if (mins[term_indx] == 1)
        {
          pg_cond = PG_FIND_COND (pg, term_indx);
          if (!pg_cond->range_node)
            continue;

          if (IR_DEBUG_POSSIBLE_VALUES)
            {
              fprintf (stderr, "mt %4d = 1: ", term_indx);
              L_print_oper (stderr, pg_cond->first_oper);
              fprintf (stderr, "\n");
              DB_print_node_minterms (range_terms, dd, pg_cond->range_node);
              fprintf (stderr, "\n");
            }

          cur_node = Cudd_bddIte (dd, pg_cond->range_node, 
				  old_node = cur_node, one);
          Cudd_Ref (cur_node);
          Cudd_RecursiveDeref (dd, old_node);

          if (IR_DEBUG_POSSIBLE_VALUES)
            {
              fprintf (stderr, "---\n");
              DB_print_node_minterms (range_terms, dd, cur_node);
              fprintf (stderr, "\n");
            }
        }
      else if (mins[term_indx] == 0)
        {
          pg_cond = PG_FIND_COND (pg, term_indx);
          if (!pg_cond->range_node)
            continue;
          if (IR_DEBUG_POSSIBLE_VALUES)
            {
              fprintf (stderr, "mt %4d = 0: ", term_indx);
              L_print_oper (stderr, pg_cond->first_oper);
              fprintf (stderr, "\n");
              DB_print_node_minterms (range_terms, dd, pg_cond->range_node);
              fprintf (stderr, "\n");
            }
          cur_node = Cudd_bddIte (dd, pg_cond->range_node, one, 
				  old_node = cur_node);
          Cudd_Ref (cur_node);
          Cudd_RecursiveDeref (dd, old_node);

          if (IR_DEBUG_POSSIBLE_VALUES)
            {
              fprintf (stderr, "---\n");
              DB_print_node_minterms (range_terms, dd, cur_node);
              fprintf (stderr, "\n");
            }
        }
    }

  if (one == cur_node)
    {
      rv = -1;                /* Input combination not possible */
    }
  else
    {
      pg_cond = PG_FIND_COND (pg, min_indx);

      if (!pg_cond->range_node)
	rv = 2;
      else if (Cudd_bddLeq (dd, Cudd_Not (pg_cond->range_node), cur_node))
	rv = 1;
      else if (Cudd_bddLeq (dd, pg_cond->range_node, cur_node))
	rv = 0;
      else
	rv = 2;
    }

  Cudd_RecursiveDeref (dd, cur_node);
  return rv;
}

/*
 * IR_minterm_implication
 * ----------------------------------------------------------------------------
 *      Returns  0 if minA implies ~minB
 *               1 if minA implies minB
 *               2 if minA implies neither minB nor ~minB
 *              -1 if either minA or minB is invalid
 */
int
IR_minterm_implication (char *minA, char *minB, int terms)
{
  int term_indx, retval;
  PG_Pred_Graph *pg;
  PG_Cond *pg_cond;
  DdManager *dd;
  DdNode *nodeAnot, *nodeBnot, *old_node, *zero, *one;

#ifdef IR_DEBUG_MINTERM_IMPLICATION
  int range_terms;
#endif

  pg = PG_pred_graph;

  dd = pg->ir_range_graph->dd;
  one = pg->ir_range_graph->one;
  zero = nodeAnot = nodeBnot = pg->ir_range_graph->zero;

#ifdef IR_DEBUG_MINTERM_IMPLICATION
  range_terms = I_BDD_num_terms (dd);
#endif

#ifdef IR_DEBUG_MINTERM_IMPLICATION
  {
    fprintf (stderr, "IR_minterm_implication\n");
    fprintf (stderr, "minterm A: ");
    DB_print_minterm (terms, minA);
    fprintf (stderr, "minterm B: ");
    DB_print_minterm (terms, minB);
  }
#endif

  for (term_indx = 0; term_indx < terms; term_indx++)
    {
      if (minA[term_indx] == 2)
        continue;
      if (minB[term_indx] != minA[term_indx])
        {
          pg_cond = PG_FIND_COND (pg, term_indx);
          if (!pg_cond->range_node)
            {
#ifdef IR_DEBUG_MINTERM_IMPLICATION
              fprintf (stderr, "Minterms fundamentally incompatible\n"
                       "IR_minterm_implication returning 2.\n");
#endif
              return 2;
            }
        }
    }

  Cudd_Ref (nodeAnot);
  for (term_indx = 0; term_indx < terms; term_indx++)
    {
      int lit = minA[term_indx];

      if (lit != 0 && lit != 1)
	continue;

      pg_cond = PG_FIND_COND (pg, term_indx);
      if (!pg_cond->range_node)
	continue;

#ifdef IR_DEBUG_MINTERM_IMPLICATION
          fprintf (stderr, "mt %4d = %d: ", term_indx, lit);
          L_print_oper (stderr, pg_cond->first_oper);
          fprintf (stderr, "\n");
          DB_print_node_minterms (range_terms, dd, pg_cond->range_node);
          fprintf (stderr, "\n");
#endif

      if (lit)
	nodeAnot = Cudd_bddIte (dd, pg_cond->range_node, 
				old_node = nodeAnot, one);
      else
	nodeAnot = Cudd_bddIte (dd, pg_cond->range_node, one, 
				old_node = nodeAnot);

      Cudd_Ref (nodeAnot);
      Cudd_RecursiveDeref (dd, old_node);

#ifdef IR_DEBUG_MINTERM_IMPLICATION
          fprintf (stderr, "---\n");
          DB_print_node_minterms (range_terms, dd, nodeAnot);
          fprintf (stderr, "\n");
#endif
    }

  if (one == nodeAnot)
    {
      Cudd_RecursiveDeref (dd, nodeAnot);
      return -1;                /* Input combination not possible */
    }

  Cudd_Ref (nodeBnot);
  for (term_indx = 0; term_indx < terms; term_indx++)
    {
      int lit = minB[term_indx];

      if (lit != 0 && lit != 1)
	continue;

      pg_cond = PG_FIND_COND (pg, term_indx);
      if (!pg_cond->range_node)
	continue;

#ifdef IR_DEBUG_MINTERM_IMPLICATION
      fprintf (stderr, "mt %4d = %d: ", term_indx, lit);
      L_print_oper (stderr, pg_cond->first_oper);
      fprintf (stderr, "\n");
      DB_print_node_minterms (range_terms, dd, pg_cond->range_node);
      fprintf (stderr, "\n");
#endif

      if (lit)
	nodeBnot = Cudd_bddIte (dd, pg_cond->range_node, 
				old_node = nodeBnot, one);
      else
	nodeBnot = Cudd_bddIte (dd, pg_cond->range_node, one, 
				old_node = nodeBnot);

      Cudd_Ref (nodeBnot);
      Cudd_RecursiveDeref (dd, old_node);

#ifdef IR_DEBUG_MINTERM_IMPLICATION
      fprintf (stderr, "---\n");
      DB_print_node_minterms (range_terms, dd, nodeBnot);
      fprintf (stderr, "\n");
#endif
    }

  if (one == nodeBnot)
    {
      Cudd_RecursiveDeref (dd, nodeBnot);
      Cudd_RecursiveDeref (dd, nodeAnot);
      return -1;                /* Input combination not possible */
    }

  if (Cudd_bddLeq (dd, nodeAnot, nodeBnot))
    retval = 1;
  else if (Cudd_bddLeq (dd, Cudd_Not (nodeBnot), nodeAnot))
    retval = 0;
  else
    retval = 2;

#ifdef IR_DEBUG_MINTERM_IMPLICATION
  fprintf (stderr, "IR_minterm_implication returning %d.\n", retval);
#endif

  Cudd_RecursiveDeref (dd, nodeAnot);
  Cudd_RecursiveDeref (dd, nodeBnot);

  return retval;
}


/*===========================================================================*/
/*
 *  Construction interfaces
 */
/*===========================================================================*/


/* IR_form_basis_functions
 * ----------------------------------------------------------------------------
 * Build basis functions for conds in the pred graph, attach them to
 * appropriate pg structures.
 */

void
IR_form_basis_functions (L_Func * fn, PG_Pred_Graph * pg)
{
  IR_Range_Table *table = NULL;

  if (!fn || !pg)
    L_punt ("IR_form_basis_functions: NULL argument.");

  if ((table = IR_build_range_table (fn, pg)))
    {
      IR_build_range_bdd (fn, table, pg, TRUE);
      if (L_print_int_ranges)
        DB_print_range_table (table);
      IR_delete_range_table (table, pg);
    }

  return;
}

/* IR_form_disjoint_ranges
 * ----------------------------------------------------------------------------
 * Build structures used in Lpred_opti.
 */

IR_Range_Table *
IR_form_disjoint_ranges (L_Func * fn)
{
  IR_Range_Table *table = NULL;

  table = IR_build_range_table (fn, PG_pred_graph);

  IR_build_range_bdd (fn, table, PG_pred_graph, FALSE);

  if (L_print_int_ranges)
    DB_print_range_table (table);

  IR_delete_range_table (table, PG_pred_graph);

  return table;
}


/*===========================================================================*/
/*
 *  Debugging functions
 */
/*===========================================================================*/


void
DB_print_range (IR_Range * range)
{
  IR_Node *node_indx;
  List node_list;

  node_list = range->node_list;

  List_start (node_list);
  while ((node_indx = (IR_Node *) List_next (node_list)))
    {
      if (node_indx->lower != node_indx->upper)
        fprintf (ERR, ITuintmaxformat "-" ITuintmaxformat,
                 node_indx->lower, node_indx->upper);
      else
        fprintf (ERR, ITuintmaxformat, node_indx->lower);

      fprintf (ERR, " ");
    }
  return;
}

void
DB_print_range_table (IR_Range_Table * table)
{
  HashTable hash_reg_to_range_list;
  List range_list, inst_list;
  IR_Range *range;
  IR_Comp_Inst *inst_indx;
  int def_count = 0;
  int found = 0;

  hash_reg_to_range_list = table->hash_reg_to_range_list;

  HashTable_start (hash_reg_to_range_list);
  /* Print reg operand, comparison opers */
  while ((range_list = (List) HashTable_next (hash_reg_to_range_list)))
    {
      def_count = 0;
      found = 1;
      range = List_first (range_list);
      fprintf (stderr,
               "---------------------------------------------------\n");
      fprintf (stderr, "Integer ranges for: ");
      L_print_operand (stderr, range->reg_operand, 0);
      fprintf (stderr, "\n");
      List_start (range_list);
      while ((range = (IR_Range *) List_next (range_list)))
        {
          fprintf (stderr, "- def %d -\n", def_count);
          inst_list = range->instance_list;
          List_start (inst_list);
          while ((inst_indx = (IR_Comp_Inst *) List_next (inst_list)))
            {
              fprintf (stderr, "cb %d: ", inst_indx->cb->id);
              L_print_oper (stderr, inst_indx->oper);
              fprintf (stderr, "\n");
            }
          /* Print range */
          DB_print_range (range);
          fprintf (stderr, "\n");
          def_count++;
        }
    }
  if (found)
    {
      fprintf (stderr,
               "---------------------------------------------------\n");
    }
  else
    {
      fprintf (stderr, "No ranges to print.\n");
    }
}

void
DB_print_minterm (int terms, char *minterm)
{
  int indx;
  for (indx = 0; indx < terms; indx++)
    {
      if (minterm[indx] == 2)
        fprintf (stderr, "-");
      else
        fprintf (stderr, "%d", (int) minterm[indx]);
    }
  fprintf (stderr, "\n");
  return;
}

void
DB_print_node_minterms (int terms, DdManager * dd_mgr, DdNode * dd_node)
{
  List mins;
  char *minterm;

  mins = I_BDD_minterms (dd_mgr, dd_node);

  List_start (mins);

  while ((minterm = (char *) List_next (mins)))
    DB_print_minterm (terms, minterm);

  List_reset_free_ptrs (mins, free);

  return;
}

void
DB_print_table_minterms (IR_Range_Table * table)
{
  PG_Cond *cond;
  int terms;
  DdManager *dd;

  if (!table || !table->rg)
    L_punt ("DB_print_minterms: malformed table");

  dd = table->rg->dd;

  terms = I_BDD_num_terms (dd);

  fprintf (stderr, "%d minterms:\n", terms);

  List_start (PG_pred_graph->pg_conds);

  while ((cond = List_next (PG_pred_graph->pg_conds)))
    {
      if (!cond->range_node)
        continue;

      L_print_oper (stderr, cond->first_oper);

      DB_print_node_minterms (terms, dd, cond->range_node);

      fprintf (stderr, "\n");
    }

  return;
}
