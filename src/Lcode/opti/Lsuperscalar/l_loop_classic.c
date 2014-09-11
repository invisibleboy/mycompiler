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
 *      File :          l_loop_classic.c
 *      Description :   superblock-based classic loop optimization
 *      Creation Date : March 1991
 *      Author :        Scott Mahlke, Pohua Chang, Wen-mei Hwu
 *      Revised 7-93    Scott A. Mahlke
 *              new Lcode, a few new opti
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

/*
 *      Debugging Options
 */
#define DEBUG_SB_LOOP_DUP
#define DEBUG_SB_LOOP_ADD_OPTS
#define DEBUG_SB_LOOP_GLOBAL_VAR_MIG  
#define DEBUG_SB_LOOP_INV_CODE_REM
#define DEBUG_SB_LOOP_COPY_PROP
#define DEBUG_SB_LOOP_OP_FOLD
#define DEBUG_SB_LOOP_DEAD_CODE
#define DEBUG_SB_LOOP_IND_ELIM 
#define DEBUG_SB_LOOP_IND_ELIM2 
#define DEBUG_SB_LOOP_IND_REINIT
#define DEBUG_SB_LOOP_POST_INC_CONV

#define IS_PREDICATED(a)        (a->pred[0] != NULL)

#define ERR stderr

/*
 *      Loop opti parameters
 */
#define L_LOOP_MIN_ITER_FOR_DUPL        1.5
#define L_LOOP_MIN_ITER_FOR_IND_ELIM_1  0.7
#define L_LOOP_MIN_ITER_FOR_IND_ELIM_2  2.7

static L_Cb **L_duplicate_cb = NULL;

/*========================================================================*/
/*
 *  Loop optimization predicates
 */

int
Lsuper_invariant_operand (L_Cb * cb, L_Operand * operand)
{
  int i;
  L_Oper *op;

  if (operand == NULL)
    return 1;
  if (L_is_constant (operand))
    return 1;
  if (L_is_fragile_macro(operand))
    return 0;

  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_same_operand (op->dest[i], operand))
            return 0;
        }
    }

  return 1;
}

static int
Lsuper_loop_inv_operands (L_Cb * cb, L_Oper * oper)
{
  int i;
  L_Operand *src;

  for (i = 0; i < L_max_src_operand; i++)
    {
      src = oper->src[i];
      if (!Lsuper_invariant_operand (cb, src))
        return 0;
    }

  return 1;
}

static int
Lsuper_unique_def_in_loop (L_Cb * cb, L_Oper * op)
{
  int i, j;
  L_Oper *oper;
  L_Operand *dest;

  for (i = 0; i < L_max_dest_operand; i++)
    {
      dest = op->dest[i];
      if (dest == NULL)
        continue;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (oper == op)
            continue;
          for (j = 0; j < L_max_dest_operand; j++)
            {
              if (L_same_operand (oper->dest[j], dest))
                return 0;
            }
        }
    }

  return 1;
}

static int
Lsuper_all_uses_in_loop_from (L_Cb * cb, L_Oper * op)
{
  int i, j, op_found;
  L_Operand *dest;
  L_Oper *ptr;

  /*
   *  Original code for non-hyperblock
   */
  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          dest = op->dest[i];
          if (dest == NULL)
            continue;
          for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
            {
              if (ptr == op)
                break;
              for (j = 0; j < L_max_src_operand; j++)
                {
                  if (L_same_operand (ptr->src[j], dest))
                    return 0;
                }
            }
          if (ptr == NULL)
            L_punt ("Lsuper_def_reachs_all_out_cb_of_loop: op never found");
        }

      return 1;
    }

  /*
   *  Hyperblock!!
   */
  else
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          dest = op->dest[i];
          if (dest == NULL)
            continue;
          op_found = 0;
          for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
            {
              if (ptr == op)
                op_found = 1;
              for (j = 0; j < L_max_src_operand; j++)
                {
                  if (L_same_operand (ptr->src[j], dest))
                    {
                      if (!op_found)
                        return 0;
                      if ((L_same_operand (op->pred[0], ptr->pred[0])) ||
                          (PG_superset_predicate_ops (op, ptr)))
                        break;
                      return 0;
                    }
                }
            }
          if (!op_found)
            L_punt ("Lsuper_def_reachs_all_out_cb_of_loop: op never found");
        }

      return 1;
    }
}

static int
Lsuper_def_reachs_all_out_cb_of_loop (L_Cb * cb, L_Oper * op)
{
  int i, op_found;
  L_Oper *ptr;
  L_Operand *dest;

  /*
   *  Original code for non-hyperblock
   */
  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          dest = op->dest[i];
          if (dest == NULL)
            continue;
          for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
            {
              if (ptr == op)
                break;
              if (!L_is_control_oper (ptr))
                continue;
              if (L_in_oper_OUT_set (cb, ptr, dest, TAKEN_PATH))
                return 0;
            }
          if (ptr == NULL)
            L_punt ("Lsuper_def_reachs_all_out_cb_of_loop: op never found");
        }

      return 1;
    }

  /*
   *  Hyperblock!!
   */
  else
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          dest = op->dest[i];
          if (dest == NULL)
            continue;
          op_found = 0;
          for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
            {
              if (ptr == op)
                op_found = 1;
              if (!L_is_control_oper (ptr))
                continue;
              if ((op_found)
                  && ((L_same_operand (op->pred[0], ptr->pred[0]))
                      || (PG_superset_predicate_ops (op, ptr))))
                continue;
              if (L_in_oper_OUT_set (cb, ptr, dest, TAKEN_PATH))
                return 0;
            }
          /* DIA - Check fall through path!! */
          if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
            {
              if (IS_PREDICATED (op) &&
                  L_in_oper_OUT_set (cb, cb->last_op, dest, FALL_THRU_PATH))
                return 0;
            }
          if (!op_found)
            L_punt ("Lsuper_def_reachs_all_out_cb_of_loop: op never found");
        }

      return 1;
    }
}

static int
Lsuper_safe_to_move_out_of_loop (L_Cb * cb, L_Oper * op)
{
  int always_executed;
  ITintmax cd_lev;
  L_Oper *ptr;
  L_Attr *attr;

  /* thing has completely safe to move */
  if (L_safe_for_speculation (op))
    return (1);

  always_executed = 1;
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == op)
        break;
      if (L_is_control_oper (ptr))
        {
          always_executed = 0;
          break;
        }
    }

  if (L_is_predicated (op))
    always_executed = 0;

  /* op executed on every iteration may always be removed */
  if (always_executed)
    return 1;

  /* added 10-17-94, SAM to take advantage of Roger's safety analysis */
  /* check control dep level of header, to see if can possibly move above
     loop header in which case can move out as invariant code */
  attr = L_find_attr (op->attr, "cdl");
  if ((attr) && (L_is_int_constant (attr->field[0])))
    {
      cd_lev = attr->field[0]->value.i;
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (L_general_branch_opcode (ptr))
            {
              attr = L_find_attr (ptr->attr, "cdl");
              if ((attr) && (L_is_int_constant (attr->field[0])) &&
                  (cd_lev <= attr->field[0]->value.i))
                return (1);
              else
                return (0);
            }
        }
    }


  return 0;
}

/*
 *      Dont look at control path here for hyperblocks because this
 *      function implies across any execution path of loop!
 */
static int
Lsuper_no_memory_conflicts_in_loop (L_Cb * cb, L_Oper * op)
{
  L_Oper *ptr;
  int dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!L_store_opcode (ptr))
        continue;
      if (!L_independent_memory_ops (cb, op, ptr, dep_flags))
        return 0;
    }

  return 1;
}

/*
 *      Dont look at control path here for hyperblocks because this
 *      function implies across any execution path of loop!
 */
#if 0
static int
Lsuper_unique_memory_location (L_Cb * cb, L_Oper * op, int *num_read,
                               int *num_write, L_Oper ** store_op)
{
  int n_r, n_w;
  L_Oper *ptr;
  int dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);

  if (op == NULL)
    L_punt ("Lsuper_no_memory_conflicts_in_loop: op cannot be NIL");

  n_r = n_w = 0;
  *num_read = *num_write = 0;

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!(L_general_store_opcode (ptr) || L_general_load_opcode (ptr)))
        continue;
      if (L_same_operand (op->src[0], ptr->src[0]) &&
          L_same_operand (op->src[1], ptr->src[1]))
        {
          /* don't allow pre/post inc to be migrated!! */
          if (!(L_load_opcode (ptr) || L_store_opcode (ptr)))
            return 0;
          if (L_load_opcode (ptr))
            n_r++;
          if (L_store_opcode (ptr))
            {
              n_w++;
              *store_op = ptr;
            }
          if (!L_same_data_types (op, ptr))
            return 0;
          continue;
        }
      if (!L_independent_memory_ops (cb, op, ptr, dep_flags))
        {
          return 0;
        }
    }

  *num_read = n_r;
  *num_write = n_w;
  return 1;
}
#endif

static int
Lsuper_unique_memory_location (L_Cb * cb, L_Oper * op, 
			       List *load_list,
                               List *store_list)
{
  L_Oper *ptr;
  int dep_flags = SET_INNER_CARRIED (0) | SET_NONLOOP_CARRIED (0);

  if (load_list && List_size(*load_list) != 0)
    printf("Lsuper_unique_memory_location: load_list not empty\n");
  if (store_list && List_size(*store_list) != 0)
    printf("Lsuper_unique_memory_location: store_list not empty\n");

  if (op == NULL)
    L_punt ("Lsuper_no_memory_conflicts_in_loop: op cannot be NIL");

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!(L_general_store_opcode (ptr) || L_general_load_opcode (ptr)))
        continue;

      if (L_same_operand (op->src[0], ptr->src[0]) &&
          L_same_operand (op->src[1], ptr->src[1]))
        {
	  /* 04/02/03 SER: Adding volatile oper check, quit if so. */
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	    break;

          /* don't allow pre/post inc to be migrated!! */
          if (!(L_load_opcode (ptr) || L_store_opcode (ptr)))
	    break;
          if (L_load_opcode (ptr) && load_list)
	    {
	      *load_list = List_insert_last(*load_list, ptr);
	    }
          if (L_store_opcode (ptr) && store_list)
            {
	      *store_list = List_insert_last(*store_list, ptr);
            }
          if (!L_same_data_types (op, ptr))
	    break;
          continue;
        }
      if (!L_independent_memory_ops (cb, op, ptr, dep_flags))
        {
	  break;
        }
    }

  if (ptr)
    {
      /* Broke out of above loop */
      if (load_list)
	*load_list = List_reset(*load_list);
      if (store_list)
	*store_list = List_reset(*store_list);
      return 0;
    }

  return 1;
}


/*
 *      Dont look at control path here for hyperblocks because this
 *      function implies across any execution path of loop!
 */
static int
Lsuper_no_danger_in_cb (L_Cb * cb, int macro_flag, int load_flag,
                        int store_flag)
{
  int memory_flag;
  L_Oper *op;

  memory_flag = load_flag | store_flag;
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      if (L_sync_opcode (op))
        return 0;
      if (memory_flag)
        {
          if ((store_flag) && (L_subroutine_call_opcode (op)))
            return 0;
          if ((!store_flag) &&
              (L_subroutine_call_opcode (op)) &&
              (!L_side_effect_free_sub_call (op)))
            return 0;
        }
      if ((macro_flag) && (L_general_subroutine_call_opcode (op)))
        return 0;
    }

  return 1;
}

static int
Lsuper_all_predecessor_blocks_in_loop (L_Cb * loop_cb, L_Cb * cb)
{
  L_Flow *src;

  if ((loop_cb == NULL) | (cb == NULL))
    L_punt
      ("Lsuper_all_predecessor_blocks_in_loop: loop_cb/cb cannot be NULL");

  src = cb->src_flow;
  if (src == NULL)
    {
      fprintf (stderr, "cb = %d\n", cb->id);
      L_punt
        ("Lsuper_all_predecessor_blocks_in_loop: cb has no predecessors");
    }

  for (; src != NULL; src = src->next_flow)
    {
      if (src->src_cb != loop_cb)
        return 0;
    }

  return 1;
}

/*
 *      opA = move in dup_cb of loop
 *      opB = uses of opA->dest in dup_cb
 *      opC = corresonding move in cb of loop
 *
 *      NOT updated for hyperblocks!
 */
static int
Lsuper_no_defs_between_in_loop (L_Inner_Loop * loop, L_Oper * opA,
                                L_Oper * opB, L_Oper * opC,
                                L_Operand * operand)
{
  int i;
  L_Oper *ptr;
  L_Cb *dup_cb;

  if (operand == NULL)
    L_punt ("Lsuper_no_defs_between_in_loop: operand cannot be NULL");
  if ((opA == NULL) | (opB == NULL) | (opC == NULL))
    L_punt
      ("Lsuper_no_defs_between_in_loop: opA, opB, and opC cannot be NIL");


  /* Check (opA, dup_cb->last] */
  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_same_operand (ptr->dest[i], operand))
	    return 0;
        }
    }

  /* Check [dup_cb->first, opB) */
  dup_cb = L_duplicate_cb[loop->id];
  for (ptr = dup_cb->first_op; ptr != opB; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_same_operand (ptr->dest[i], operand))
	    return 0;
        }
    }
  if (ptr == NULL)
    L_punt ("Lsuper_no_defs_between_in_loop: opB not found");

  /* Check (opC, cb->last] */
  for (ptr = opC->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_same_operand (ptr->dest[i], operand))
	    return 0;
        }
    }

  return 1;
}

static int
Lsuper_same_sign_integers (L_Operand * op1, L_Operand * op2)
{
  ITintmax val1, val2;

  if ((op1 == NULL) | (op2 == NULL))
    return 0;
  if ((!L_is_int_constant (op1)) | (!L_is_int_constant (op2)))
    return 0;
  val1 = op1->value.i;
  val2 = op2->value.i;
  if (val1 >= 0)
    return (val2 >= 0);
  else
    return (val2 < 0);
}

static int
Lsuper_add_increment_operation (L_Oper * op)
{
  if (!L_int_add_opcode (op))
    return 0;
  if (L_same_operand (op->dest[0], op->src[1]))
    {
      L_Operand *temp;
      temp = op->src[1];
      op->src[1] = op->src[0];
      op->src[0] = temp;
    }
  if (!L_same_operand (op->dest[0], op->src[0]))
    return 0;
  if (L_same_operand (op->dest[0], op->src[1]))
    return 0;
  return 1;
}

/* a=a+K, a=K+a, a=a-K, K is anything except "a" */
static int
Lsuper_general_increment_operation (L_Oper * op)
{
  if (!(L_int_add_opcode (op) || L_int_sub_opcode (op)))
    return 0;
  if ((L_int_add_opcode (op)) && (L_same_operand (op->dest[0], op->src[1])))
    {
      L_Operand *temp;
      temp = op->src[1];
      op->src[1] = op->src[0];
      op->src[0] = temp;
    }
  if (!L_same_operand (op->dest[0], op->src[0]))
    return 0;
  if (L_same_operand (op->dest[0], op->src[1]))
    return 0;
  return 1;
}

static int
Lsuper_is_defined_before_used_by (L_Cb * cb, L_Operand * operand, L_Oper * op)
{
  int i;
  L_Oper *ptr;

  if (cb == NULL)
    L_punt ("Lsuper_is_defined_before_used_by: cb is NIL");
  if (operand == NULL)
    L_punt ("Lsuper_is_defined_before_used_by: operand is NULL");

  /*
   *  Original code for non hyperblock!
   */
  if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
    {
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (ptr == op)
            return 0;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (ptr->dest[i], operand))
                return 1;
            }
        }
      L_punt ("Lsuper_is_defined_before_used_by: op not found");
    }

  /*
   *  hyperblock!!
   */
  else
    {
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (ptr == op)
            return 0;
          for (i = 0; i < L_max_dest_operand; i++)
            {
              if (L_same_operand (ptr->dest[i], operand))
                {
                  if (L_same_operand (ptr->pred[0], op->pred[0]) ||
                      PG_superset_predicate_ops (ptr, op))
                    return 1;
                }
            }
        }
      L_punt ("Lsuper_is_defined_before_used_by: op not found");
    }

  return (0);
}

static int
Lsuper_is_defined_before_used (L_Cb * cb, L_Operand * operand)
{
  L_Oper *oper;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_is_src_operand (operand, oper))
        continue;
      if (!Lsuper_is_defined_before_used_by (cb, operand, oper))
        return (0);
    }

  return (1);
}

static int
Lsuper_no_uses_in_cb (L_Operand * operand, L_Cb * cb)
{
  int i;
  L_Oper *ptr;

  if (cb == NULL)
    L_punt ("Lsuper_no_uses_in_cb: cb cannot be NIL");
  if (operand == NULL)
    return 1;

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      for (i = 0; i < L_max_src_operand; i++)
        {
          if (L_same_operand (ptr->src[i], operand))
            return 0;
        }
    }

  /* JWS 6/4/98 */
  if (L_is_ctype_predicate (operand))
    {
      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (L_same_operand (ptr->pred[0], operand))
            return 0;
        }

    }
  return 1;
}

static int
Lsuper_no_uses_in_cb_by_others (L_Operand * operand, L_Oper * op, L_Cb * cb)
{
  int i;
  L_Oper *ptr;

  if (cb == NULL)
    L_punt ("Lsuper_no_uses_in_cb: cb cannot be NIL");
  if (operand == NULL)
    return 1;

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == op)
        continue;
      for (i = 0; i < L_max_src_operand; i++)
        {
          if (L_same_operand (ptr->src[i], operand))
            return 0;
        }
    }

  return 1;
}

static int
Lsuper_no_branches_betw (L_Cb * cb, L_Oper * opA, L_Oper * opB)
{
  L_Oper *ptr;
  if ((cb == NULL) | (opA == NULL) | (opB == NULL))
    L_punt ("Lsuper_no_branches_betw: cb, opA, opB cannot be NIL");

  /* See whether opA or opB is first in cb */
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if ((ptr == opA) | (ptr == opB))
        break;
    }
  if (ptr == NULL)
    L_punt ("Lsuper_no_branches_betw: opA and opB not found in cb");

  if (ptr == opA)
    {                           /* opA is first */
      for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (ptr == opB)
            break;
          if (L_general_branch_opcode (ptr))
            return 0;
        }
      if (ptr == NULL)
        L_punt ("Lsuper_no_branches_betw: opB not found");
    }

  else
    {                           /* opB is first */
      for (ptr = opB->next_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (ptr == opA)
            break;
          if (L_general_branch_opcode (ptr))
            return 0;
        }
      if (ptr == NULL)
        L_punt ("Lsuper_no_branches_betw: opA not found");
    }

  return 1;
}

static int
Lsuper_loop_iter_larger_than (L_Inner_Loop * loop, double val)
{
  double ave_iter;
  ave_iter = (loop->weight - loop->num_invocation) / loop->num_invocation;
  return (ave_iter >= val);
}

static int
Lsuper_not_live_at_exits_between (L_Cb * cb, L_Operand * operand,
                                  L_Oper * opA, L_Oper * opB)
{
  L_Oper *ptr;

  if (L_no_br_between (opA, opB))
    return 1;

  for (ptr = opA->next_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == opB)
        break;
      if (!L_is_control_oper (ptr))
        continue;
      if (L_in_oper_OUT_set (cb, ptr, operand, TAKEN_PATH))
        return 0;
    }

  if (ptr != opB)
    L_punt ("Lsuper_not_live_at_exits_between: opB not found");

  return 1;
}

static int
Lsuper_can_make_post_inc (L_Oper * mem_op, L_Oper * ind_op)
{
  int old_num_oper, new_num_oper, old_opc;

  /* data on original memory op */
  old_num_oper = M_num_oper_required_for (mem_op, L_fn->name);
  old_opc = mem_op->opc;

  /* put ind var as first src of mem_op */
  if (L_same_operand (mem_op->src[1], ind_op->dest[0]))
    {
      L_Operand *temp = mem_op->src[0];
      mem_op->src[0] = mem_op->src[1];
      mem_op->src[1] = temp;
    }

  /* create post inc load */
  if (L_load_opcode (mem_op))
    {
      L_change_opcode (mem_op, L_corresponding_postincrement_load (mem_op));
      mem_op->dest[1] = L_copy_operand (ind_op->dest[0]);
      mem_op->src[2] = L_copy_operand (ind_op->src[1]);
      new_num_oper = M_num_oper_required_for (mem_op, L_fn->name);
      /* undo post inc */
      L_delete_operand (mem_op->dest[1]);
      L_delete_operand (mem_op->src[2]);
      mem_op->dest[1] = NULL;
      mem_op->src[2] = NULL;
      L_change_opcode (mem_op, old_opc);
    }

  /* create post inc store */
  else
    {
      L_change_opcode (mem_op, L_corresponding_postincrement_store (mem_op));
      mem_op->dest[0] = L_copy_operand (ind_op->dest[0]);
      mem_op->src[3] = L_copy_operand (ind_op->src[1]);
      new_num_oper = M_num_oper_required_for (mem_op, L_fn->name);
      /* undo post inc */
      L_delete_operand (mem_op->dest[0]);
      L_delete_operand (mem_op->src[3]);
      mem_op->dest[0] = NULL;
      mem_op->src[3] = NULL;
      L_change_opcode (mem_op, old_opc);
    }

  return (new_num_oper <= old_num_oper);
}

/*
 *      Try to reinitialize address operands of mem_op to be reg+0
 */
static int
Lsuper_ind_should_be_reinitialized (L_Cb * cb, L_Oper * mem_op,
                                    L_Oper * ind_op)
{
  int j;
  L_Oper *ptr;
  L_Operand *old, *curr, *operand;

  operand = ind_op->dest[0];
  if (L_same_operand (mem_op->src[0], operand))
    old = mem_op->src[1];
  else if (L_same_operand (mem_op->src[1], operand))
    old = mem_op->src[0];
  else
    return 0;

  /* no need to re-init if it is already 0 */
  if (L_is_int_constant (old) && (old->value.i == 0))
    return 0;

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == ind_op)
        continue;
      if (!L_is_src_operand (operand, ptr))
        continue;
      if (L_load_opcode (ptr) || L_store_opcode (ptr))
        {
          if (!(L_same_operand (operand, ptr->src[0]) ||
                L_same_operand (operand, ptr->src[1])))
            return 0;
          for (j = 2; j < L_max_src_operand; j++)
	    if (L_same_operand (operand, ptr->src[j]))
	      return (0);

          if (L_same_operand (operand, ptr->src[0]))
            curr = ptr->src[1];
          else
            curr = ptr->src[0];
          if (L_is_int_constant (old) && L_is_int_constant (curr))
            continue;
          else if (L_same_operand (curr, old))
            continue;
          return 0;
        }
      else if (L_int_cond_branch_opcode (ptr))
        {
          if (!(L_same_operand (operand, ptr->src[0]) ||
                L_same_operand (operand, ptr->src[1])))
            return 0;
          if (L_same_operand (operand, ptr->src[0]))
            curr = ptr->src[1];
          else
            curr = ptr->src[0];
          if (Lsuper_invariant_operand (cb, curr) &&
              Lsuper_no_danger_in_cb (cb, L_is_macro (curr), 0, 0))
            continue;
          return 0;
        }
      else
        {
          return 0;
        }
    }

  if (L_is_int_constant (old) && (old->value.i != 0))
    return 1;
  else if (L_is_label (old) || L_is_string (old))
    return 1;
  else if (L_is_register (old) &&
           Lsuper_invariant_operand (cb, old) &&
           Lsuper_no_danger_in_cb (cb, 0, 0, 0))
    return 1;
  else if (L_is_macro (old) &&
           Lsuper_invariant_operand (cb, old) &&
           Lsuper_no_danger_in_cb (cb, 1, 0, 0))
    return 1;
  else
    return 0;
}

static int
Lsuper_only_used_as_base_addr_with_const_offset_in_loop (L_Inner_Loop * loop,
                                                         L_Operand * operand,
                                                         ITintmax offset)
{
  int j, old_num_op, new_num_op;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *old_src, *new_src;

  cb = loop->cb;
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      if (!L_is_src_operand (operand, op))
        continue;
      if (L_load_opcode (op) || L_store_opcode (op))
        {
          if (!(L_same_operand (operand, op->src[0]) ||
                L_same_operand (operand, op->src[1])))
            {
              return 0;
            }
          for (j = 2; j < L_max_src_operand; j++)
            {
              if (L_same_operand (operand, op->src[j]))
                {
                  return (0);
                }
            }
          if (!(L_is_int_constant (op->src[0]) ||
                L_is_int_constant (op->src[1])))
            {
              return 0;
            }
          old_num_op = M_num_oper_required_for (op, L_fn->name);
          if (L_is_int_constant (op->src[0]))
            {
              old_src = op->src[0];
              new_src = L_new_gen_int_operand (offset + op->src[0]->value.i);
              op->src[0] = new_src;
              new_num_op = M_num_oper_required_for (op, L_fn->name);
              op->src[0] = old_src;
            }
          else
            {
              old_src = op->src[1];
              new_src = L_new_gen_int_operand (offset + op->src[1]->value.i);
              op->src[1] = new_src;
              new_num_op = M_num_oper_required_for (op, L_fn->name);
              op->src[1] = old_src;
            }
          L_delete_operand (new_src);
          if (new_num_op > old_num_op)
            {
              return 0;
            }
          else
            continue;
        }
      else
        {
          return 0;
        }
    }
  return 1;
}

int
Lsuper_all_uses_of_ind_can_change_offset (L_Inner_Loop * loop, int *out_cb,
                                          int num_out_cb,
                                          L_Operand * operand1,
                                          L_Operand * operand2)
{
  int j, old_num_op, new_num_op;
  ITintmax offset;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *old_src, *new_src;

  cb = loop->cb;
  if (cb == NULL)
    L_punt ("Lsuper_all_uses_of_ind_can_change_offset: cb is NULL");

  offset =
    L_find_ind_initial_offset (loop->preheader, operand1, operand2, NULL,
                               NULL, loop->ind_info);

  for (op = cb->first_op; op != NULL; op = op->next_op)
    {

      /* skip inductive update */
      if (L_is_dest_operand (operand1, op))
        continue;

      if (!L_is_src_operand (operand1, op))
        continue;

      /* temporarily create what new ld/st will be, see how many ops it
         will expand to compared to original, don't do opti if more ops */
      if (L_load_opcode (op) || L_store_opcode (op))
        {
          if (!(L_same_operand (operand1, op->src[0]) ||
                L_same_operand (operand1, op->src[1])))
            return 0;
          for (j = 2; j < L_max_src_operand; j++)
            {
              if (L_same_operand (operand1, op->src[j]))
                return 0;
            }
          if (!(L_is_int_constant (op->src[0]) ||
                L_is_int_constant (op->src[1])))
            return 0;
          old_num_op = M_num_oper_required_for (op, L_fn->name);
          if (L_is_int_constant (op->src[0]))
            {
              old_src = op->src[0];
              new_src = L_new_gen_int_operand (offset + op->src[0]->value.i);
              op->src[0] = new_src;
              new_num_op = M_num_oper_required_for (op, L_fn->name);
              op->src[0] = old_src;
            }
          else
            {
              old_src = op->src[1];
              new_src = L_new_gen_int_operand (offset + op->src[1]->value.i);
              op->src[1] = new_src;
              new_num_op = M_num_oper_required_for (op, L_fn->name);
              op->src[1] = old_src;
            }
          L_delete_operand (new_src);
          if (new_num_op > old_num_op)
            return 0;
          else
            continue;
        }

      /* for rest ops, assume if the constant field can hold original
         constant, it can also hold the new constant!! */
      else if (L_move_opcode (op))
        {
          continue;
        }
      else if (L_int_add_opcode (op))
        {
          if (L_is_int_constant (op->src[0]) ||
              L_is_int_constant (op->src[1]))
            continue;
          if (L_is_register (op->dest[0]) &&
              Lsuper_only_used_as_base_addr_with_const_offset_in_loop (loop,
                                                                       op->dest
                                                                       [0],
                                                                       offset)
              && L_not_live_in_out_cb (out_cb, num_out_cb, op->dest[0])
              && Lsuper_is_defined_before_used (cb, op->dest[0]))
            continue;
        }
      else if (L_int_sub_opcode (op))
        {
          if (L_is_int_constant (op->src[0]) ||
              L_is_int_constant (op->src[1]))
            continue;
        }
      /*
       * DIA - Was:
       * else if (L_int_comparison_opcode(op)) {
       * For unsigned check boundary conditions.
       * Also, pred compares and branches are checked.
       */
      else if (L_unsigned_int_comparative_opcode (op))
        {
          if (L_is_int_constant (op->src[0]))
            {
              if (L_is_legal_unsigned_value_offset_32
                  ((unsigned int) ITicast (op->src[0]->value.i), -offset))
                continue;
            }
          if (L_is_int_constant (op->src[1]))
            {
              if (L_is_legal_unsigned_value_offset_32
                  ((unsigned int) ITicast (op->src[1]->value.i), -offset))
                continue;
            }
        }
      else if (L_signed_int_comparison_opcode (op))
        {
          if (L_is_int_constant (op->src[0]) ||
              L_is_int_constant (op->src[1]))
            continue;
        }
      else if (L_int_pred_comparison_opcode (op))
        {
          if (L_is_int_constant (op->src[0]) ||
              L_is_int_constant (op->src[1]))
            continue;
        }
      /* allow more freedom for branches, so opti will work for loops
         with non-constant final values */
      else if (L_signed_int_cond_branch_opcode (op))
        {
          if (Lsuper_invariant_operand (cb, op->src[0]) ||
              Lsuper_invariant_operand (cb, op->src[1]))
            continue;
        }
      /* DIA - What about pred defines?!?!?! */
      return 0;
    }
  return 1;
}

int
Lsuper_reorder_ops_so_no_use_betw (L_Inner_Loop * loop, L_Cb * cb,
                                   L_Operand * operand, L_Oper * op1,
                                   L_Oper * op2)
{
  L_Oper *op, *first_use, *last_use;

  if ((loop == NULL) | (cb == NULL) | (operand == NULL) | (op1 == NULL) |
      (op2 == NULL))
    L_punt ("Lsuper_reorder_ops_so_no_use_betw: one of the args is NIL");

  first_use = NULL;
  last_use = NULL;

  for (op = op1; op; op = op->next_op)
    {
      if (op == op2)
        break;
      if (Set_in (loop->basic_ind_var_op, op->id))
        continue;
      if (!L_is_src_operand (operand, op))
	continue;

      first_use = op;
      break;
    }

  if (!first_use)
    L_punt ("Lsuper_reorder_ops_so_no_use_betw: first use not found");

  for (op = op2; op; op = op->prev_op)
    {
      if (op == op1)
        break;
      if (Set_in (loop->basic_ind_var_op, op->id))
        continue;
      if (!L_is_src_operand (operand, op))
	continue;

      last_use = op;
      break;
    }

  if (!last_use)
    L_punt ("L_reorder_ops_so_no_use_betw: last use not found");

  if (L_can_move_below (cb, last_use, op1))
    {
      L_move_oper_after (cb, op1, last_use);
      return 1;
    }
  else if (L_can_move_above (cb, first_use, op2))
    {
      L_move_oper_before (cb, op2, first_use);
      return 1;
    }
  return 0;
}

int
Lsuper_no_uses_of_between_first_and_last_defs (L_Inner_Loop * loop,
                                               L_Operand * operand1,
                                               L_Operand * operand2)
{
  int j, num_use_betw;
  L_Cb *cb;
  L_Oper *op1, *op2, *op;

  cb = loop->cb;
  num_use_betw = 0;
  op1 = op2 = NULL;
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      if (L_same_operand (op->dest[0], operand1) ||
          L_same_operand (op->dest[0], operand2))
        {
          op1 = op;
          break;
        }
    }
  if (op1 == NULL)
    L_punt ("Lsuper_no_uses_of_between_first_and_last_defs: no def found");
  for (op = cb->last_op; op != NULL; op = op->prev_op)
    {
      if (L_same_operand (op->dest[0], operand1) ||
          L_same_operand (op->dest[0], operand2))
        {
          op2 = op;
          break;
        }
    }
  for (op = op1; op != op2; op = op->next_op)
    {
      if (Set_in (loop->basic_ind_var_op, op->id))
        continue;
      for (j = 0; j < L_max_src_operand; j++)
        {
          if (L_same_operand (op->src[j], operand1))
            num_use_betw += 1;
        }
    }
  if (num_use_betw == 0)
    {
      return 1;
    }
  else if (Lsuper_reorder_ops_so_no_use_betw (loop, cb, operand1, op1, op2))
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int
Lsuper_no_branches_betw_defs (L_Cb * cb, L_Operand * operand1,
                              L_Operand * operand2)
{
  L_Oper *op1, *op2, *op;

  op1 = op2 = NULL;
  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      if (L_same_operand (op->dest[0], operand1) ||
          L_same_operand (op->dest[0], operand2))
        {
          op1 = op;
          break;
        }
    }
  if (op1 == NULL)
    L_punt ("Lsuper_no_branches_betw_defs: no defs found");

  for (op = cb->last_op; op != NULL; op = op->prev_op)
    {
      if (L_same_operand (op->dest[0], operand1) ||
          L_same_operand (op->dest[0], operand2))
        {
          op2 = op;
          break;
        }
    }

  for (op = op1; op != op2; op = op->next_op)
    {
      if (L_general_branch_opcode (op))
        return 0;
    }

  return 1;
}

/* 
 * DIA - Make sure increments are on the same predicate
 */
int
Lsuper_defs_on_same_pred (L_Cb * cb, L_Operand * operand1,
                          L_Operand * operand2)
{
  L_Oper *op1, *op;

  op1 = NULL;
  for (op = cb->first_op; op; op = op->next_op)
    {
      if (L_same_operand (op->dest[0], operand1) ||
          L_same_operand (op->dest[0], operand2))
        {
          op1 = op;
          break;
        }
    }

  if (op1 == NULL)
    L_punt ("Lsuper_defs_on_same_pred: no defs found");

  for (op = op1; op; op = op->next_op)
    {
      if (L_same_operand (op->dest[0], operand1) ||
          L_same_operand (op->dest[0], operand2))
        {
          if (op1->pred[0] != op->pred[0])
            return 0;
        }
    }

  return 1;
}


/* SAM 11-94 */
Set L_find_sentry_registers (L_Cb * cb)
{
  int i;
  L_Oper *oper;
  L_Operand *dest;
  Set regs = NULL;

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_find_attr (oper->attr, "sentry"))
        continue;
      for (i = 0; i < L_max_dest_operand; i++)
        {
          dest = oper->dest[i];
          if (L_is_register (dest))
            regs = Set_add (regs, dest->value.r);
        }

    }

  return (regs);
}

int
L_can_copy_branch_to_preheader (L_Cb * cb, L_Oper * br_op, L_Oper * inc_op,
                                L_Operand * operand)
{
  int i;
  L_Oper *ptr;

  /* other src operands of br_op must be invariant */
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (L_same_operand (br_op->src[i], operand))
        continue;
      if (!Lsuper_invariant_operand (cb, br_op->src[i]))
        return (0);
    }

  /* no stores between first_op and last, other src operands of uses must be
     invariant operands */
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == br_op)
        break;
      if (ptr == inc_op)
        continue;
      if (L_general_store_opcode (ptr))
        return (0);
      if (L_is_src_operand (operand, ptr))
        return (0);
    }

  return (1);
}

int
L_no_branches_between_not_in_set (L_Cb * cb, Set all_br_ops)
{
  L_Oper *ptr, *first = NULL, *last = NULL;

  /* Find first and last op in set */
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (Set_in (all_br_ops, ptr->id))
        {
          first = ptr;
          break;
        }
    }
  if (first == NULL)
    L_punt ("L_no_branches_between_not_in_set: cannot find op in Set");

  for (ptr = cb->last_op; ptr != NULL; ptr = ptr->prev_op)
    {
      if (Set_in (all_br_ops, ptr->id))
        {
          last = ptr;
          break;
        }
    }
  if (last == NULL)
    L_punt ("L_no_branches_between_not_in_set: cannot find op in Set");

  if (first == last)
    return (1);

  for (ptr = first->next_op; ptr != last; ptr = ptr->next_op)
    {
      if (Set_in (all_br_ops, ptr->id))
        continue;
      if (L_general_branch_opcode (ptr))
        return (0);
    }

  return (1);
}

/*==========================================================================*/
/*
 *  Loop optimization functions
 */
/*==========================================================================*/

void
L_copy_branch_to_preheader (L_Inner_Loop * loop, L_Cb * cb, L_Oper * br_op,
                            L_Oper * inc_op, L_Operand * src, Set all_br_ops)
{
  int i;
  L_Cb *preheader, *target_cb, *new_cb;
  L_Oper *ptr, *new_inc_op, *new_br_op, *new_op;
  L_Flow *flow, *new_flow, *save_flow;

  preheader = loop->preheader;
  if (preheader == NULL)
    L_punt ("L_copy_branch_to_preheader: loop has no preheader");
  if (br_op == NULL)
    L_punt ("L_copy_branch_to_preheader: br_op is NULL");

  /* create a new cb with all non-branch opers from start of cb to br_op */
  new_cb = L_create_cb (0.0);
  L_insert_cb_after (L_fn, L_fn->last_cb, new_cb);
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (ptr == br_op)
        break;
      if (Set_in (all_br_ops, ptr->id))
        continue;
      new_op = L_copy_operation (ptr);
      L_insert_oper_after (new_cb, new_cb->last_op, new_op);
      if (L_general_branch_opcode (ptr))
        {
          flow = L_find_flow_for_branch (cb, ptr);
          target_cb = L_find_branch_dest (ptr);
          new_flow = L_new_flow (flow->cc, new_cb, flow->dst_cb, 0.0);
          new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
          new_flow = L_new_flow (flow->cc, new_cb, flow->dst_cb, 0.0);
          target_cb->src_flow = L_concat_flow (target_cb->src_flow, new_flow);
        }
    }

  /* copy br and inc to preheader */
  if (inc_op != NULL)
    {
      new_inc_op = L_copy_operation (inc_op);
      L_delete_operand (new_inc_op->dest[0]);
      new_inc_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                    L_native_machine_ctype,
                                                    L_PTYPE_NULL);
      L_insert_oper_after (preheader, preheader->last_op, new_inc_op);

      new_br_op = L_copy_operation (br_op);
      L_delete_operand (new_br_op->src[2]);
      new_br_op->src[2] = L_new_cb_operand (new_cb);
      for (i = 0; i < L_max_src_operand; i++)
        {
          if (L_same_operand (new_br_op->src[i], src))
            {
              L_delete_operand (new_br_op->src[i]);
              new_br_op->src[i] = L_copy_operand (new_inc_op->dest[0]);
            }
        }
      L_insert_oper_after (preheader, preheader->last_op, new_br_op);
    }
  else
    {
      new_br_op = L_copy_operation (br_op);
      L_delete_operand (new_br_op->src[2]);
      new_br_op->src[2] = L_new_cb_operand (new_cb);
      L_insert_oper_after (preheader, preheader->last_op, new_br_op);
    }


  /* setup up flow arcs for preheader */
  save_flow = L_find_last_flow (preheader->dest_flow);
  preheader->dest_flow = L_remove_flow (preheader->dest_flow, save_flow);
  new_flow = L_new_flow (1, preheader, new_cb, 0.0);
  preheader->dest_flow = L_concat_flow (preheader->dest_flow, new_flow);
  preheader->dest_flow = L_concat_flow (preheader->dest_flow, save_flow);
  new_flow = L_new_flow (1, preheader, new_cb, 0.0);
  new_cb->src_flow = L_concat_flow (new_cb->src_flow, new_flow);

  /* insert jump from new_cb to target of br_op */
  target_cb = br_op->src[2]->value.cb;
  new_op = L_create_new_op (Lop_JUMP);
  new_op->src[0] = L_new_cb_operand (target_cb);
  L_insert_oper_after (new_cb, new_cb->last_op, new_op);

  /* setup flow arcs for new_cb */
  new_flow = L_new_flow (1, new_cb, target_cb, 0.0);
  new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
  new_flow = L_new_flow (1, new_cb, target_cb, 0.0);
  target_cb->src_flow = L_concat_flow (target_cb->src_flow, new_flow);
}

/*
 *      If (is_dupl) use first iteration of loop to insert loads, otherwise
 *      use preheader.
 */
static L_Oper *
Lsuper_insert_load_ops_for_global_var_mig (L_Inner_Loop * loop, 
					   int ld_opc, L_Operand * ld_dest, 
					   L_Oper *tmpl_op,
                                           int is_dupl, int safe)
{
  L_Cb *cb;
  L_Oper *new_op;

  L_Operand *ld_s1 = tmpl_op->src[0], *ld_s2 = tmpl_op->src[1];
  L_Attr *attr = tmpl_op->attr;
  L_Sync_Info *sync_info = tmpl_op->sync_info;
  int flags = tmpl_op->flags;

  new_op = L_create_new_op (ld_opc);
  new_op->dest[0] = L_copy_operand (ld_dest);
  new_op->src[0] = L_copy_operand (ld_s1);
  new_op->src[1] = L_copy_operand (ld_s2);
  /* attr fields */
  new_op->attr = L_copy_attr (attr);
  /* flag field */
  new_op->flags = flags;
  if ((!is_dupl) && (!safe) && (L_mask_potential_exceptions))
    {
      L_mark_oper_speculative (new_op);
    }

  if (is_dupl)
    cb = loop->cb;
  else
    cb = loop->preheader;

  /* DMG - sync arcs */
  if (sync_info != NULL)
    {
      new_op->sync_info = L_copy_sync_info (sync_info);
      L_insert_all_syncs_in_dep_opers (new_op);
      L_adjust_syncs_for_movement_out_of_loop (new_op, cb);
    }

  if (tmpl_op->acc_info)
    new_op->acc_info = L_copy_mem_acc_spec_list (tmpl_op->acc_info);

  L_insert_oper_after (cb, cb->last_op, new_op);

  return (new_op);
}

static void
Lsuper_insert_store_ops_for_global_var_mig (L_Inner_Loop * loop,
                                            int st_opc,
                                            L_Operand * st_s3,
					    L_Oper *tmpl_op,
                                            int is_dupl, int safe, 
					    L_Oper **cond_set)

{
  int i, num_out_cb, *out_cb = NULL;
  L_Cb *loop_cb, *out, *cond_cb = NULL;
  L_Oper *op, *new_op;
  L_Flow *flow;
  Set out_cb_set;

  L_Operand *st_s1 = tmpl_op->src[0], *st_s2 = tmpl_op->src[1];
  L_Attr * attr = tmpl_op->attr;
  L_Sync_Info *sync_info = tmpl_op->sync_info;
  /* 10/22/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int flags = tmpl_op->flags;
#endif

  if (is_dupl)
    loop_cb = L_duplicate_cb[loop->id];
  else
    loop_cb = loop->cb;

  /* Just recompute out_cb since it is likely to be messed up !!! */
  out_cb_set = NULL;
  for (flow = loop_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      if (flow->dst_cb == loop_cb)
        continue;
      out_cb_set = Set_add (out_cb_set, flow->dst_cb->id);
    }
  num_out_cb = Set_size (out_cb_set);
  if (num_out_cb > 0)
    {
      out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
      Set_2array (out_cb_set, out_cb);
    }
  Set_dispose (out_cb_set);

  /*
   * Creates a predicate for selecting to perform the store
   */
  *cond_set = NULL;
  switch (Lsuper_store_migration_mode)
    {
    case L_STORE_MIGRATION_FULL_PRED:
      *cond_set = L_setup_conditional_op_with_pred (loop->preheader);
      L_fn->flags = L_SET_BIT_FLAG (L_fn->flags, L_FUNC_HYPERBLOCK);
      break;
    case L_STORE_MIGRATION_NO_PRED:
      *cond_set = L_setup_conditional_op_with_cbs (loop->preheader);
      break;
    case L_STORE_MIGRATION_NO_COND:
      break;
    default:
      L_punt ("L_insert_store_ops_for_global_var_mig: unknown mode\n");
    }

  for (i = 0; i < num_out_cb; i++)
    {

      /* create new store op */
      new_op = L_create_new_op (st_opc);
      new_op->src[0] = L_copy_operand (st_s1);
      new_op->src[1] = L_copy_operand (st_s2);
      new_op->src[2] = L_copy_operand (st_s3);

      /* attr fields */
      new_op->attr = L_copy_attr (attr);

      out = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, out_cb[i]);
      if (Lsuper_all_predecessor_blocks_in_loop (loop_cb, out) &&
	  (Lsuper_store_migration_mode != L_STORE_MIGRATION_NO_PRED))
        {
	  switch (Lsuper_store_migration_mode)
	    {
	    case L_STORE_MIGRATION_FULL_PRED:
	      cond_cb =
		L_create_conditional_op_with_pred_in_cb (*cond_set,
							 new_op,
							 out, out->first_op);
	      break;
	    case L_STORE_MIGRATION_NO_COND:
	      L_insert_oper_before (out, out->first_op, new_op);
	      cond_cb = out;
	      break;
	    default:
	      L_punt
		("L_insert_store_ops_for_global_var_mig: unknown mode\n");
	    }
	  
          /* DMG - sync arcs */
          if (sync_info != NULL)
            {
              new_op->sync_info = L_copy_sync_info (sync_info);
              L_insert_all_syncs_in_dep_opers (new_op);
              L_adjust_syncs_for_movement_out_of_loop (new_op, cond_cb);
            }

	  if (tmpl_op->acc_info)
	    new_op->acc_info = L_copy_mem_acc_spec_list (tmpl_op->acc_info);
        }
      else
        {
          L_Cb *new_cb, *src_cb;
          L_Oper *new_jump_op;
          L_Flow *flow, *next_flow, *f, *new_flow;

          new_cb = L_create_cb (out->weight);
          L_insert_cb_after (L_fn, L_fn->last_cb, new_cb);
          for (flow = out->src_flow; flow != NULL; flow = next_flow)
            {
              next_flow = flow->next_flow;
              if (flow->src_cb != loop_cb)
                {
                  new_cb->weight -= flow->weight;
                  if (new_cb->weight < ZERO_EQUIVALENT)
                    new_cb->weight = 0.0;
                  continue;
                }
              src_cb = flow->src_cb;
              f = L_find_flow (src_cb->dest_flow, flow->cc, src_cb, out);
              op = L_find_branch_for_flow (src_cb, f);
              if (op != NULL)
                {
                  L_change_branch_dest (op, out, new_cb);
                }
              else
                {
                  /* this must be fallthru path or something wrong */
                  if (src_cb != out->prev_cb)
                    L_punt ("Lsuper_insert_store_ops_for_global_var_mig:\
                                 illegal control structure");
                  /* new_cb won't be fallthru, so insert explicit jump to it */
                  if (src_cb->weight > L_min_fs_weight)
                    new_jump_op = L_create_new_op (Lop_JUMP_FS);
                  else
                    new_jump_op = L_create_new_op (Lop_JUMP);
                  new_jump_op->src[0] = L_new_cb_operand (new_cb);
                  L_insert_oper_after (src_cb, src_cb->last_op, new_jump_op);
                  /* DIA - Hyperblock has no fall through now */
                  if (L_EXTRACT_BIT_VAL (src_cb->flags, L_CB_HYPERBLOCK))
                    {
                      if (!L_EXTRACT_BIT_VAL
                          (src_cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
                        {
                          src_cb->flags =
                            L_SET_BIT_FLAG (src_cb->flags,
                                            L_CB_HYPERBLOCK_NO_FALLTHRU);
                        }
                    }
                }
              /* update flow arcs */
              f->dst_cb = new_cb;
              out->src_flow = L_delete_flow (out->src_flow, flow);
              new_flow = L_new_flow (f->cc, src_cb, new_cb, f->weight);
              new_cb->src_flow = L_concat_flow (new_cb->src_flow, new_flow);
            }

	  /* RDB Done in case statement below */
          /* L_insert_oper_after (new_cb, new_cb->last_op, new_op); */

          /* DMG - sync arcs */
          if (sync_info != NULL)
            {
              new_op->sync_info = L_copy_sync_info (sync_info);
              L_insert_all_syncs_in_dep_opers (new_op);
              L_adjust_syncs_for_movement_out_of_loop (new_op, new_cb);
            }

	  if (tmpl_op->acc_info)
	    new_op->acc_info = L_copy_mem_acc_spec_list (tmpl_op->acc_info);

          /* insert jump into new_cb to out */
          if (new_cb->weight > L_min_fs_weight)
            new_jump_op = L_create_new_op (Lop_JUMP_FS);
          else
            new_jump_op = L_create_new_op (Lop_JUMP);
          new_jump_op->src[0] = L_new_cb_operand (out);
          L_insert_oper_after (new_cb, new_cb->last_op, new_jump_op);
          /* add correponding flow for jump */
          new_flow = L_new_flow (1, new_cb, out, new_cb->weight);
          new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
          new_flow = L_new_flow (1, new_cb, out, new_cb->weight);
          out->src_flow = L_concat_flow (out->src_flow, new_flow);
          /* update loop structure to reflect new out cb */
          loop->out_cb = Set_delete (loop->out_cb, out->id);
          loop->out_cb = Set_add (loop->out_cb, new_cb->id);

	  switch (Lsuper_store_migration_mode)
	    {
	    case L_STORE_MIGRATION_FULL_PRED:
	      cond_cb =
		L_create_conditional_op_with_pred_in_cb (*cond_set,
							 new_op,
							 new_cb,
							 new_cb->first_op);
	      break;
	    case L_STORE_MIGRATION_NO_PRED:
	      cond_cb =
		L_create_conditional_op_with_cbs_at_cb (L_fn, *cond_set,
							new_op,
							new_cb,
							new_cb->first_op);
	      break;
	    case L_STORE_MIGRATION_NO_COND:
	      L_insert_oper_before (new_cb, new_cb->first_op, new_op);
	      cond_cb = new_cb;
	      break;
	    default:
	      L_punt
		("L_insert_store_ops_for_global_var_mig: unknown mode\n");
	    }
        }
    }
  if (out_cb != NULL)
    Lcode_free (out_cb);
}

static L_Oper *
Lsuper_find_equivalent_op (L_Cb * cb, L_Oper * op)
{
  int i, flag;
  L_Oper *ptr;

  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!L_same_opcode (ptr, op))
        continue;

      flag = 0;
      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (!L_same_operand (ptr->dest[i], op->dest[i]))
            {
              flag = 1;
              break;
            }
        }
      if (flag)
        continue;

      for (i = 0; i < L_max_src_operand; i++)
        {
          if (!L_same_operand (ptr->src[i], op->src[i]))
            {
              flag = 1;
              break;
            }
        }
      if (flag)
        continue;

      for (i = 0; i < L_max_pred_operand; i++)
        {
          if (!L_same_operand (ptr->pred[i], op->pred[i]))
            {
              flag = 1;
              break;
            }
        }
      if (flag)
        continue;

      return ptr;
    }

  return NULL;
}

static void
Lsuper_reinit_ind (L_Inner_Loop * loop, L_Cb * cb, L_Oper * mem_op,
                   L_Oper * ind_op, int is_dupl)
{
  int is_s, is_d;
  ITintmax new_val;
  L_Cb *preheader;
  L_Oper *ptr, *new_op, *restore_op;
  L_Operand *operand, *old, *new;

  operand = L_copy_operand (ind_op->dest[0]);
  if (L_same_operand (mem_op->src[0], operand))
    old = L_copy_operand (mem_op->src[1]);
  else if (L_same_operand (mem_op->src[1], operand))
    old = L_copy_operand (mem_op->src[0]);
  else
    {
      L_punt ("Lsuper_reinit_ind: illegal use of operand");
      return;
    }

  new = L_new_register_operand (++L_fn->max_reg_id, 
				L_native_machine_ctype,
				L_PTYPE_NULL);
  if (is_dupl)
    preheader = loop->cb;
  else
    preheader = loop->preheader;

  /* change uses in loop */
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      is_d = L_is_dest_operand (operand, ptr);
      is_s = L_is_src_operand (operand, ptr);

      if (is_d)
        {
          if (ptr != ind_op)
            L_punt ("L_reinit_induction_var: illegal def of operand");
          L_delete_operand (ptr->dest[0]);
          L_delete_operand (ptr->src[0]);
          ptr->dest[0] = L_copy_operand (new);
          ptr->src[0] = L_copy_operand (new);
          continue;
        }

      if (!is_s)
        continue;

      /* use by load or store */
      if (L_load_opcode (ptr) || L_store_opcode (ptr))
        {
          if (L_same_operand (ptr->src[0], operand))
            {
              if (L_same_operand (ptr->src[1], old))
                {
                  L_delete_operand (ptr->src[0]);
                  L_delete_operand (ptr->src[1]);
                  ptr->src[0] = L_copy_operand (new);
                  ptr->src[1] = L_new_gen_int_operand (0);
                }
              else
                {
                  if (!L_is_int_constant (old))
                    L_punt ("Lsuper_reinit_ind: old must be int const");
                  if (!L_is_int_constant (ptr->src[1]))
                    L_punt ("Lsuper_reinit_ind: src2 must be int const");
                  new_val = ptr->src[1]->value.i - old->value.i;
                  L_delete_operand (ptr->src[0]);
                  L_delete_operand (ptr->src[1]);
                  ptr->src[0] = L_copy_operand (new);
                  ptr->src[1] = L_new_gen_int_operand (new_val);
                }
            }
          else if (L_same_operand (ptr->src[1], operand))
            {
              if (L_same_operand (ptr->src[0], old))
                {
                  L_delete_operand (ptr->src[0]);
                  L_delete_operand (ptr->src[1]);
                  ptr->src[0] = L_new_gen_int_operand (0);
                  ptr->src[1] = L_copy_operand (new);
                }
              else
                {
                  if (!L_is_int_constant (old))
                    L_punt ("Lsuper_reinit_ind: old must be int const");
                  if (!L_is_int_constant (ptr->src[0]))
                    L_punt ("Lsuper_reinit_ind: src1 must be int const");
                  new_val = ptr->src[0]->value.i - old->value.i;
                  L_delete_operand (ptr->src[0]);
                  L_delete_operand (ptr->src[1]);
                  ptr->src[0] = L_new_gen_int_operand (new_val);
                  ptr->src[1] = L_copy_operand (new);
                }
            }
          else
            {
              L_punt ("Lsuper_reinit_ind: illegal ld/st which uses operand");
            }
        }

      /* use by branch */
      else if (L_int_cond_branch_opcode (ptr))
        {
          new_op = L_create_new_op (Lop_ADD);
          new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                    L_native_machine_ctype,
                                                    L_PTYPE_NULL);
          if (L_same_operand (ptr->src[0], operand))
            {
              new_op->src[0] = L_copy_operand (ptr->src[1]);
              new_op->src[1] = L_copy_operand (old);
              L_delete_operand (ptr->src[0]);
              L_delete_operand (ptr->src[1]);
              ptr->src[0] = L_copy_operand (new);
              ptr->src[1] = L_copy_operand (new_op->dest[0]);
            }
          else
            {
              new_op->src[0] = L_copy_operand (ptr->src[0]);
              new_op->src[1] = L_copy_operand (old);
              L_delete_operand (ptr->src[0]);
              L_delete_operand (ptr->src[1]);
              ptr->src[0] = L_copy_operand (new_op->dest[0]);
              ptr->src[1] = L_copy_operand (new);
            }
          L_insert_oper_after (preheader, preheader->last_op, new_op);
        }
      else
        {
          L_punt ("Lsuper_reinit_ind: illegal opcode");
        }
    }

  /* insert new op into preheader to initialize ind var */
  new_op = L_create_new_op (Lop_ADD);
  new_op->dest[0] = new;
  new_op->src[0] = L_copy_operand (operand);
  new_op->src[1] = old;
  L_insert_oper_after (preheader, preheader->last_op, new_op);

  /* insert ops at exits of loop to restore operand if it is live */
  restore_op = L_create_new_op (Lop_SUB);
  restore_op->dest[0] = L_copy_operand (operand);
  restore_op->src[0] = L_copy_operand (new);
  restore_op->src[1] = L_copy_operand (old);
  for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
    {
      if (!L_general_branch_opcode (ptr))
        continue;
      if (L_is_branch_target (ptr, cb))
        continue;
      if (!L_in_oper_OUT_set (cb, ptr, operand, TAKEN_PATH))
        continue;
      L_insert_op_at_dest_of_br (cb, ptr, restore_op, 1);
    }
  /* DIA - Place op at fallthru path if one exists and dest is live out */
  if (((L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
        (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))) ||
       (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
        !L_uncond_branch_opcode (cb->last_op) &&
        !L_register_branch_opcode (cb->last_op))) &&
      L_in_oper_OUT_set (cb, cb->last_op, operand, FALL_THRU_PATH))
    {
      L_insert_op_at_fallthru_dest (cb, restore_op, 1);
    }

  L_delete_oper (cb, restore_op);
  L_delete_operand (operand);
  return;
}

/*
 *      call functions in l_opti_functions.h in Lopti dir
 */

void
Lsuper_find_all_ind_info (L_Inner_Loop * loop, L_Cb * cb)
{
  L_Cb *preheader;
  L_Oper *oper;
  L_Operand *ind_var, *ind_inc;
  L_Ind_Info *info;

  L_delete_all_ind_info (&(loop->ind_info));

  if (loop->basic_ind_var)
    loop->basic_ind_var = Set_dispose (loop->basic_ind_var);

  if (loop->basic_ind_var_op)
    loop->basic_ind_var_op = Set_dispose (loop->basic_ind_var_op);

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!Lsuper_add_increment_operation (oper) ||
	  !L_is_register (oper->dest[0]) ||
	  !L_is_int_constant (oper->src[1]) ||
	  !Lsuper_unique_def_in_loop (cb, oper))
	  continue;

      ind_var = oper->dest[0];
      ind_inc = oper->src[1];

      /* record induction var */
      loop->basic_ind_var = Set_add (loop->basic_ind_var, ind_var->value.r);
      loop->basic_ind_var_op = Set_add (loop->basic_ind_var_op, oper->id);

      /* record induction info */
      info = L_find_ind_info (loop->ind_info, ind_var, 2);
      if (info != NULL)
        L_punt ("Lsuper_find_all_ind_info: multiply defined ind var");
      info = L_new_ind_info (ind_var, 1);
      loop->ind_info = L_concat_ind_info (loop->ind_info, info);
      info->increment = L_copy_operand (ind_inc);
    }

  /* If no ind vars just return */
  if (!loop->ind_info)
    return;

  /* Dont compute initial vals if jsr in preheader */
  preheader = loop->preheader;
  if (!L_no_danger_in_cb (preheader))
    return;

  /* Search both preheader and block before preheader if possible */
  if (preheader->src_flow &&
      !preheader->src_flow->next_flow &&
      L_no_danger_in_cb (preheader->src_flow->src_cb))
    L_find_initial_val_for_cb (preheader->src_flow->src_cb,
                               &(loop->ind_info));
  L_find_initial_val_for_cb (preheader, &(loop->ind_info));

  return;
}

static void
Lsuper_change_offset_for_all_uses (L_Inner_Loop * loop, L_Oper * def_op,
                                   L_Operand * operand, int offset)
{
  ITintmax new;
  L_Cb *cb;
  L_Oper *op;

  cb = loop->cb;
  for (op = def_op->next_op; op != NULL; op = op->next_op)
    {
      if (L_is_src_operand (operand, op))
        {
          if (L_load_opcode (op) || L_store_opcode (op))
            {
              if (L_same_operand (operand, op->src[2]))
                L_punt
                  ("Lsuper_change_offset_for_all_uses: "
                   "illegal use of operand");
              if (!
                  (L_is_int_constant (op->src[0])
                   || L_is_int_constant (op->src[1])))
                L_punt("Lsuper_change_offset_for_all_uses: "
                       "s1 or s2 must be const");
              if (L_same_operand (operand, op->src[0]))
                {
                  new = op->src[1]->value.i + offset;
                  L_delete_operand (op->src[1]);
                  op->src[1] = L_new_gen_int_operand (new);
                }
              else
                {
                  new = op->src[0]->value.i + offset;
                  L_delete_operand (op->src[0]);
                  op->src[0] = L_new_gen_int_operand (new);
                }
            }
          else
            {
              L_punt ("Lsuper_change_offset_for_all_uses: illegal opcode");
            }
        }
      /* break out of here if operand is redefined, no more uses of def_op */
      if (L_is_dest_operand (operand, op))
        break;
    }
}

/*
 *      Eliminate operand1, replace with operand 2
 */
static void
Lsuper_induction_elim_2 (L_Inner_Loop * loop, L_Operand * operand1,
                         L_Operand * operand2)
{
  ITintmax offset, new_offset;
  L_Cb *cb, *preheader;
  L_Oper *op, *next, *new_op;
  L_Operand *new_reg;

  offset =
    L_find_ind_initial_offset (loop->preheader, operand1, operand2, NULL,
                               NULL, loop->ind_info);
#if 0
  if (offset < 0)
    L_punt ("Lsuper_induction_elim_2: offset should be positive");
#endif

  cb = loop->cb;
  preheader = loop->preheader;

  for (op = cb->first_op; op != NULL; op = next)
    {
      next = op->next_op;
      if (L_is_dest_operand (operand1, op))
        {
          if (!Set_in (loop->basic_ind_var_op, op->id))
            L_punt ("Lsuper_induction_elim_2: illegal def of operand1");
          L_delete_oper (cb, op);
        }
      else if (L_is_src_operand (operand1, op))
        {
          if (L_same_operand (op->src[0], operand1))
            {
              L_delete_operand (op->src[0]);
              op->src[0] = L_copy_operand (operand2);
              if (L_move_opcode (op))
                {
                  L_change_opcode (op, Lop_ADD);
                  op->src[1] = L_new_gen_int_operand (offset);
                }
              else if (L_load_opcode (op) || L_store_opcode (op))
                {
                  if (!L_is_int_constant (op->src[1]))
                    L_punt ("Lsuper_induction_elim_2: src2 must be constant");
                  new_offset = op->src[1]->value.i + offset;
                  L_delete_operand (op->src[1]);
                  op->src[1] = L_new_gen_int_operand (new_offset);
                }
              else if (L_int_add_opcode (op))
                {
                  if (L_is_int_constant (op->src[1]))
                    {
                      new_offset = op->src[1]->value.i + offset;
                      L_delete_operand (op->src[1]);
                      op->src[1] = L_new_gen_int_operand (new_offset);
                    }
                  else
                    if
                    (Lsuper_only_used_as_base_addr_with_const_offset_in_loop
                     (loop, op->dest[0], offset))
                    {
                      Lsuper_change_offset_for_all_uses (loop, op,
                                                         op->dest[0], offset);
                    }
                  else
                    {
                      L_punt
                        ("Lsuper_induction_elim_2: illegal case for int add");
                    }
                }
              else if (L_int_sub_opcode (op) || 
		       L_int_comparison_opcode (op)
                       || L_int_pred_comparison_opcode (op))
                {
                  if (!L_is_int_constant (op->src[1]))
                    L_punt ("Lsuper_induction_elim_2: src2 must be constant");
                  new_offset = op->src[1]->value.i - offset;
                  L_delete_operand (op->src[1]);
                  op->src[1] = L_new_gen_int_operand (new_offset);
                }
              else if (L_int_cond_branch_opcode (op))
                {
                  if (L_is_int_constant (op->src[1]))
                    {
                      new_offset = op->src[1]->value.i - offset;
                      L_delete_operand (op->src[1]);
                      op->src[1] = L_new_gen_int_operand (new_offset);
                    }
                  else
                    {
                      new_reg = L_new_register_operand (++L_fn->max_reg_id,
                                                        L_native_machine_ctype,
                                                        L_PTYPE_NULL);
                      new_op = L_create_new_op (Lop_SUB);
                      new_op->dest[0] = new_reg;
                      new_op->src[0] = L_copy_operand (op->src[1]);
                      new_op->src[1] = L_new_gen_int_operand (offset);
                      L_insert_oper_after (preheader, preheader->last_op,
                                           new_op);
                      L_delete_operand (op->src[1]);
                      op->src[1] = L_copy_operand (new_reg);
                    }
                }
              /* DIA - What about pred defs? */
              else
                {
                  L_punt ("Lsuper_induction_elim_2: illegal opcode for op");
                }
            }

          else if (L_same_operand (op->src[1], operand1))
            {
              L_delete_operand (op->src[1]);
              op->src[1] = L_copy_operand (operand2);
              if (L_move_opcode (op))
                {
                  L_punt ("Lsuper_induction_elim_2: move with 2 srcs????");
                }
              else if (L_load_opcode (op) || L_store_opcode (op))
                {
                  if (!L_is_int_constant (op->src[0]))
                    L_punt ("Lsuper_induction_elim_2: src1 must be constant");
                  new_offset = op->src[0]->value.i + offset;
                  L_delete_operand (op->src[0]);
                  op->src[0] = L_new_gen_int_operand (new_offset);
                }
              else if (L_int_add_opcode (op))
                {
                  if (L_is_int_constant (op->src[0]))
                    {
                      new_offset = op->src[0]->value.i + offset;
                      L_delete_operand (op->src[0]);
                      op->src[0] = L_new_gen_int_operand (new_offset);
                    }
                  else if
                    (Lsuper_only_used_as_base_addr_with_const_offset_in_loop
                     (loop, op->dest[0], offset))
                    {
                      Lsuper_change_offset_for_all_uses (loop, op,
                                                         op->dest[0], offset);
                    }
                  else
                    {
                      L_punt
                        ("Lsuper_induction_elim_2: illegal case for int add");
                    }
                }
              else if (L_int_sub_opcode (op) || L_int_comparison_opcode (op))
                {
                  if (!L_is_int_constant (op->src[0]))
                    L_punt ("Lsuper_induction_elim_2: src1 must be constant");
                  new_offset = op->src[0]->value.i - offset;
                  L_delete_operand (op->src[0]);
                  op->src[0] = L_new_gen_int_operand (new_offset);
                }
              else if (L_int_cond_branch_opcode (op) || 
		       L_int_pred_comparison_opcode (op))
                {
                  if (L_is_int_constant (op->src[0]))
                    {
                      new_offset = op->src[0]->value.i - offset;
                      L_delete_operand (op->src[0]);
                      op->src[0] = L_new_gen_int_operand (new_offset);
                    }
                  else
                    {
                      new_reg = L_new_register_operand (++L_fn->max_reg_id,
                                                        L_native_machine_ctype,
                                                        L_PTYPE_NULL);
                      new_op = L_create_new_op (Lop_SUB);
                      new_op->dest[0] = new_reg;
                      new_op->src[0] = L_copy_operand (op->src[0]);
                      new_op->src[1] = L_new_gen_int_operand (offset);
		      L_insert_oper_after (preheader, preheader->last_op,
					   new_op);
                      L_delete_operand (op->src[0]);
                      op->src[0] = L_copy_operand (new_reg);
                    }
                }
	      else
                {
                  L_punt ("Lsuper_induction_elim_2: illegal opcode for op");
                }
            }
          else
            {
              L_punt ("Lsuper_induction_elim_2: illegal use of operand1");
            }
        }
    }
}

 /*==========================================================================*/
/*
 *  Duplicating loop body
 *
 *  Steps:
 *      1. Insert a new bb (sb) into the layout after loop block
 *      2. Copy all block elements from old into new
 *              a. operations
 *              b. flow lists
 *              c. profile weights (need to scale these)
 *      3. Restructure branch at end of old to jump around new
 *      4. Restructure branch at end of new to jump back to new
 */
/*==========================================================================*/

/*
 *      NOT updated for hyperblocks!
 */
void
L_duplicate_loop_body (L_Inner_Loop * loop)
{
  double orig_weight, num_invocation, old_scale, new_scale, old_to_new_weight;
  L_Cb *old_cb, *new_cb;
  L_Oper *feedback_op;
  L_Flow *flow, *f1, *f2, *new_flow;

  /* Step 1 */
  old_cb = loop->cb;
  new_cb = L_create_cb (0.0);
  L_insert_cb_after (L_fn, old_cb, new_cb);
  L_duplicate_cb[loop->id] = new_cb;

  /* Step 2 */
  L_copy_block_contents (old_cb, new_cb);
  for (flow = old_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      new_flow = L_new_flow (flow->cc, new_cb, flow->dst_cb, flow->weight);
      new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
    }
  orig_weight = old_cb->weight;
  num_invocation = loop->num_invocation;
  if (orig_weight == 0.0)
    {
      old_scale = 1.0;
      new_scale = 0.0;
    }
  else
    {
      old_scale = num_invocation / orig_weight;
      new_scale = (orig_weight - num_invocation) / orig_weight;
    }
  old_cb->weight = num_invocation;
  new_cb->weight = orig_weight - num_invocation;
  L_scale_flow_weights (old_cb->dest_flow, old_scale);
  L_scale_flow_weights (new_cb->dest_flow, new_scale);

  /* Step 3 */
  feedback_op = loop->feedback_op;
  if (feedback_op == old_cb->last_op)
    {
      if (L_cond_branch_opcode (feedback_op))
        {
          f1 = L_find_second_to_last_flow (old_cb->dest_flow);
          f2 = f1->next_flow;
          old_to_new_weight = f1->weight;
          L_change_cond_br (feedback_op,
                            L_opposite_pred_completer (L_get_compare_type
                                                       (feedback_op)),
                            f2->dst_cb);
          L_change_flow (f1, f1->cc, f1->src_cb, f2->dst_cb, f2->weight);
          L_change_flow (f2, f2->cc, f2->src_cb, new_cb, old_to_new_weight);
        }
      else if (L_uncond_branch_opcode (feedback_op))
        {
          L_delete_oper (old_cb, feedback_op);
          f1 = L_find_last_flow (old_cb->dest_flow);
          L_change_flow (f1, 0, old_cb, new_cb, f1->weight);
        }
      else
        {
          L_punt ("L_duplicate_loop_body: illegal feedback_op");
        }
    }
  else
    {
      if (!L_cond_branch_opcode (feedback_op))
        L_punt ("L_duplicate_loop_body: feedback op must be cond br");
      f1 = L_find_second_to_last_flow (old_cb->dest_flow);
      f2 = f1->next_flow;
      old_to_new_weight = f1->weight;
      L_change_cond_br (feedback_op,
                        L_opposite_pred_completer (L_get_compare_type
                                                   (feedback_op)),
                        f2->dst_cb);
      L_delete_oper (old_cb, feedback_op->next_op);
      L_change_flow (f1, f1->cc, f1->src_cb, f2->dst_cb, f2->weight);
      L_change_flow (f2, f2->cc, f2->src_cb, new_cb, old_to_new_weight);
    }

  /* Step 4 */
  feedback_op = new_cb->last_op;
  if (L_cond_branch_opcode (feedback_op))
    {
      if (feedback_op->src[2]->value.cb != old_cb)
        L_punt ("L_duplicate_loop_body: cond br to illegal cb in new");
      feedback_op->src[2]->value.cb = new_cb;
      f1 = L_find_second_to_last_flow (new_cb->dest_flow);
      L_change_flow (f1, f1->cc, f1->src_cb, new_cb, f1->weight);
    }
  else if (L_uncond_branch_opcode (feedback_op))
    {
      if (feedback_op->src[0]->value.cb != old_cb)
        {
          feedback_op = feedback_op->prev_op;
          if (feedback_op->src[2]->value.cb != old_cb)
            L_punt ("L_duplicate_loop_body: cond br to illegal cb in new");
          feedback_op->src[2]->value.cb = new_cb;
          f1 = L_find_second_to_last_flow (new_cb->dest_flow);
          L_change_flow (f1, f1->cc, f1->src_cb, new_cb, f1->weight);
        }
      else
        {
          feedback_op->src[0]->value.cb = new_cb;
          f1 = L_find_last_flow (new_cb->dest_flow);
          L_change_flow (f1, f1->cc, f1->src_cb, new_cb, f1->weight);
        }
    }
  else
    {
      L_punt ("L_duplicate_loop_body: illegal loop structure for new");
    }

  /* don't maintain src flow during this, so rebuild it afterwards */
  L_clear_src_flow (L_fn);
  L_rebuild_src_flow (L_fn);
}

/*=========================================================================*/
/*
 *      Determine if additional opti to be applied by duplicating loop body
 *
 *      Inv code rem, global var migration, and loop copy prop can be
 *      additionally applied when unroll loop 1 time.
 */
/*=========================================================================*/

/*
 *      Not updated for hyperblocks!!
 */
int
L_additional_loop_opts_to_apply (L_Inner_Loop * loop)
{
  L_Cb *cb;
  L_Oper *opA, *opB;

  cb = loop->cb;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      int macro_flag;
      /*
       *      Check for global vars to migrate
       */
      if (!(L_load_opcode (opA) || L_store_opcode (opA)))
        continue;
      if (!Lsuper_invariant_operand (cb, opA->src[0]))
        continue;
      if (!Lsuper_invariant_operand (cb, opA->src[1]))
        continue;
      macro_flag = L_has_fragile_macro_operand (opA);
      if (!Lsuper_no_danger_in_cb (cb, macro_flag, 1, 1))
        continue;
      if (!Lsuper_unique_memory_location
          (cb, opA, NULL, NULL))
        continue;

#ifdef DEBUG_SB_LOOP_ADD_OPTS
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "Additional glob var mig (loop %d) (cb %d) (op %d)\n",
                   loop->id, cb->id, opA->id);
        }
#endif

      return 1;
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      int macro_flag, load_flag, store_flag;
      /*
       *      Check for invariant operations
       */
      if (!L_can_move_opcode (opA))
        continue;
      if (!Lsuper_loop_inv_operands (cb, opA))
        continue;
      if (!Lsuper_unique_def_in_loop (cb, opA))
        continue;
      macro_flag = L_has_fragile_macro_operand (opA);
      load_flag = L_general_load_opcode (opA);
      store_flag = L_general_store_opcode (opA);
      if (!Lsuper_no_danger_in_cb (cb, macro_flag, load_flag, store_flag))
        continue;
      if ((load_flag | store_flag) &&
          (!Lsuper_no_memory_conflicts_in_loop (cb, opA)))
        continue;

#ifdef DEBUG_SB_LOOP_ADD_OPTS
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "Additional inv code removal (loop %d) (cb %d) (op %d)\n",
                   loop->id, cb->id, opA->id);
        }
#endif

      return 1;
    }

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      for (opB = cb->first_op; opB != NULL; opB = opB->next_op)
        {
          int macro_flag;
          if (opA == opB)
            continue;
          /*
           *  Check for loop copy props
           */
          if (!L_move_opcode (opA))
            break;
          if (!L_is_variable (opA->src[0]))
            break;
          if (L_same_operand (opA->dest[0], opA->src[0]))
            break;
          if (!Lsuper_unique_def_in_loop (cb, opA))
            break;
          if (!L_is_src_operand (opA->dest[0], opB))
            continue;
          if (!L_can_change_src_operand (opB, opA->dest[0]))
            continue;
          if (!L_no_defs_between (opA->dest[0], opA, cb->last_op))
            continue;
          if (!L_no_defs_between (opA->dest[0], cb->first_op, opB))
            continue;
          /* check boundary conds */
          if (L_is_dest_operand (opA->dest[0], cb->last_op))
            continue;
          if (L_is_dest_operand (opA->dest[0], cb->first_op))
            continue;
          if (!L_no_defs_between (opA->src[0], opA, cb->last_op))
            continue;
          if (!L_no_defs_between (opA->src[0], cb->first_op, opB))
            continue;
          /* check boundary conds */
          if (L_is_dest_operand (opA->src[0], cb->last_op))
            continue;
          if (L_is_dest_operand (opA->src[0], cb->first_op))
            continue;
          macro_flag = L_has_fragile_macro_operand (opA);
          if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
            continue;
#ifdef DEBUG_SB_LOOP_ADD_OPTS
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr,
                       "Additional loop copy prop "
                       "(loop %d)(cb %d)(op %d %d)\n",
                       loop->id, cb->id, opA->id, opB->id);
            }
#endif

          return 1;

        }
    }

  return 0;
}

/*=========================================================================*/
/*
 *      Superblock loop optimizations
 */
/*=========================================================================*/

int
L_sb_loop_inv_code_rem (L_Inner_Loop * loop, int is_dupl)
{
  int i, change;
  L_Cb *cb;
  L_Oper *op, *next, *new_op = NULL;
  L_Attr *attr;
  Set sentry = NULL;
  change = 0;

  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;
  if (cb == NULL)
    return 0;

  for (op = cb->first_op; op != NULL; op = next)
    {
      int macro_flag, load_flag, store_flag, safe;
      next = op->next_op;

      /* 04/02/03 SER: Adding volatile oper check, skip if so. */
      if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	continue;

      /*
       *      Match pattern no. 1
       */
      if (!L_can_move_opcode (op))
        continue;
      if (!Lsuper_loop_inv_operands (cb, op))
        continue;
      if (!Lsuper_unique_def_in_loop (cb, op))
        continue;
      /* Next 3 predicates automatically true if this is duplicate */
      if ((!is_dupl) && (!Lsuper_all_uses_in_loop_from (cb, op)))
        continue;
      if ((!is_dupl) && (!Lsuper_def_reachs_all_out_cb_of_loop (cb, op)))
        continue;
      safe = Lsuper_safe_to_move_out_of_loop (cb, op);
      if ((!is_dupl) && (!L_non_excepting_ops) && (!safe))
        continue;
      macro_flag = L_has_fragile_macro_operand (op);
      load_flag = L_general_load_opcode (op);
      store_flag = L_general_store_opcode (op);
      if (!Lsuper_no_danger_in_cb (cb, macro_flag, load_flag, store_flag))
        continue;
      if ((load_flag | store_flag) &&
          (!Lsuper_no_memory_conflicts_in_loop (cb, op)))
        continue;

#ifdef DEBUG_SB_LOOP_INV_CODE_REM
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "**Apply loop inv code removal to %d (is_dupl = %d)\n",
                   op->id, is_dupl);
        }
#endif

      /*
       *      Replace pattern no. 1
       */

      /* move op to preheader */
      if (is_dupl)
        {
          L_move_op_to_end_of_block (cb, loop->cb, op);
          /* DMG - sync arcs */
          if (op->sync_info)
            {
              L_adjust_syncs_for_movement_out_of_loop (op, loop->cb);
            }
        }
      else
        {
          if ((!safe) && (L_mask_potential_exceptions))
            {
              if (L_general_load_opcode (op)
                  && (!(L_EXTRACT_BIT_VAL (op->flags, L_OPER_MASK_PE))))
                {
                  /* Insert checks at locations of nonspeculative loads that
                     are going to be removed */
                  L_insert_check_after (cb, op, op);
                }
              L_mark_oper_speculative (op);
            }
          L_move_op_to_end_of_block (cb, loop->preheader, op);
          /* DMG - sync arcs */
          if (op->sync_info)
            {
              L_adjust_syncs_for_movement_out_of_loop (op, loop->preheader);
            }
        }

      /* delete the predicate if it has 1 */
      for (i = 0; i < L_max_pred_operand; i++)
        {
          if (op->pred[i] == NULL)
            continue;
          L_delete_operand (op->pred[i]);
          op->pred[i] = NULL;
        }
      STAT_COUNT ("L_sb_loop_inv_code_rem_1", 1, cb);
      change += 1;
    }

  /*
   * this opti is effective when compare and branchs not supported in
   *  architecture, move comparison out of loop if it invariant
   */

  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      int macro_flag;
      /*
       *      Match pattern no. 2 (branches)
       */
      if (!L_cond_branch_opcode (op))
        continue;
      if (M_oper_supported_in_arch (op->opc))
        continue;
      if (!Lsuper_loop_inv_operands (cb, op))
        continue;
      macro_flag = (L_is_fragile_macro (op->src[0]) ||
                    L_is_fragile_macro (op->src[1]));
      if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
        continue;
      /*
       *      Replace pattern no. 2
       */

#ifdef DEBUG_SB_LOOP_INV_CODE_REM
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "**Apply loop inv code removal2 to %d (is_dupl = %d)\n",
                   op->id, is_dupl);
        }
#endif
      L_create_new_op (op->opc);
      L_change_to_rcmp_op (op);

      L_copy_compare (new_op, op);

      new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                L_native_machine_ctype,
                                                L_PTYPE_NULL);
      new_op->src[0] = L_copy_operand (op->src[0]);
      new_op->src[1] = L_copy_operand (op->src[1]);
      if (is_dupl)
        {
          L_Cb *original;
          original = loop->cb;
          L_insert_oper_after (original, original->last_op, new_op);
        }
      else
        {
          L_Cb *preheader;
          preheader = loop->preheader;
          L_insert_oper_after (preheader, preheader->last_op, new_op);
        }

      L_set_compare_type (op, Lcmp_COM_NE);

      L_delete_operand (op->src[0]);
      L_delete_operand (op->src[1]);
      op->src[0] = L_copy_operand (new_op->dest[0]);
      op->src[1] = L_new_gen_int_operand (0);
      STAT_COUNT ("L_sb_loop_inv_code_rem_2", 1, cb);
      change += 1;
    }

    /*************************************************************************/
  /*
   *  Special optimization needed for Roger's safety analysis, make
   *  a copy of invariant branches in preheader.  SAM 11-94.  This is
   *  basically a all or nothing opti, so first check if can do all cases
   *  of it, then do it.  What a mess....
   */

  /* if attr is set, the opti is done already so don't reapply */
  attr = L_find_attr (cb->attr, "sentry");
  sentry = L_find_sentry_registers (loop->preheader);
  if ((sentry != NULL) && (attr == NULL))
    {
      L_Oper **inc_ops = NULL, **br_ops = NULL;
      L_Operand *src = NULL;
      int macro_flag, num_sentry, *buf = NULL, legal;
      Set all_br_ops;

      num_sentry = Set_size (sentry);
      buf = Lcode_malloc (sizeof (int) * num_sentry);
      Set_2array (sentry, buf);
      inc_ops = (L_Oper **) Lcode_calloc (num_sentry, sizeof (L_Oper *));
      br_ops = (L_Oper **) Lcode_calloc (num_sentry, sizeof (L_Oper *));

      legal = 1;
      for (i = 0; i < num_sentry; i++)
        {
          if (src != NULL)
            {
              L_delete_operand (src);
              src = NULL;
            }
          src = L_new_register_operand (buf[i], L_native_machine_ctype,
                                        L_PTYPE_NULL);

          for (op = cb->first_op; op != NULL; op = op->next_op)
            {
              /*
               *      Match pattern no. 3
               */
              if (!L_is_src_operand (src, op))
                continue;
              macro_flag = L_has_fragile_macro_operand (op);
              if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
                continue;
              if (L_general_branch_opcode (op))
                {
                  if (L_can_copy_branch_to_preheader
                      (cb, op, inc_ops[i], src))
                    {
                      br_ops[i] = op;
                      continue;
                    }
                  else
                    {
                      legal = 0;
                      break;
                    }
                }
              else if (Lsuper_general_increment_operation (op))
                {
                  if (Lsuper_invariant_operand (cb, op->src[1]))
                    {
                      inc_ops[i] = op;
                      continue;
                    }
                  else
                    {
                      legal = 0;
                      break;
                    }
                }
              /* illegal use */
              else
                {
                  legal = 0;
                  break;
                }
            }
          if (legal == 0)
            break;
        }

      if (src != NULL)
        {
          L_delete_operand (src);
          src = NULL;
        }

      all_br_ops = NULL;
      if (legal == 1)
        {
          for (i = 0; i < num_sentry; i++)
            {
              all_br_ops = Set_add (all_br_ops, br_ops[i]->id);
            }
#if 0
          Not necessary anymore, SAM 11 - 94
            legal = L_no_branches_between_not_in_set (cb, all_br_ops);
#endif
        }


      /*
       *      Replace pattern no. 3
       */
      if (legal == 1)
        {
          for (i = 0; i < num_sentry; i++)
            {
#ifdef DEBUG_SB_LOOP_INV_CODE_REM
              if (Lsuper_debug_loop_classic_opti)
                {
                  fprintf (stderr, "**Apply sentry loop inv code to %d\n",
                           br_ops[i]->id);
                }
#endif
              src =
                L_new_register_operand (buf[i], L_native_machine_ctype,
                                        L_PTYPE_NULL);
              sentry = Set_delete (sentry, buf[i]);
              L_copy_branch_to_preheader (loop, cb, br_ops[i], inc_ops[i],
                                          src, all_br_ops);
              L_delete_operand (src);
            }

          attr = L_new_attr ("sentry", 0);
          cb->attr = L_concat_attr (cb->attr, attr);
	  STAT_COUNT ("L_sb_loop_inv_code_rem_3", 1, cb);
        }

      Lcode_free (br_ops);
      Lcode_free (inc_ops);
      Lcode_free (buf);
      Set_dispose (all_br_ops);
    }

  /* End of this mess */
    /*************************************************************************/

  return (change);
}

int
L_sb_loop_global_var_mig (L_Inner_Loop * loop, int is_dupl)
{
  int change, new_load_opc, new_store_opc;
  L_Cb *cb;
  L_Oper *op, *ptr, *check_op, *new_load;
  L_Oper *pred_set = NULL, *pred_set_op = NULL;
  L_Operand *new_reg;
  List load_list = NULL;
  List store_list = NULL;
  change = 0;

  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;
  if (cb == NULL)
    return 0;

  for (op = cb->first_op; op != NULL; op = op->next_op)
    {
      int macro_flag, safe;

      /*
       *      Match pattern
       */
      if (!(L_load_opcode (op) || L_store_opcode (op)))
        continue;
      /* 04/02/03 SER: Adding volatile oper check, skip if so. */
      if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_VOLATILE))
	continue;
      if (!Lsuper_invariant_operand (cb, op->src[0]))
        continue;
      if (!Lsuper_invariant_operand (cb, op->src[1]))
        continue;

      safe = Lsuper_safe_to_move_out_of_loop (cb, op);

      if ((!is_dupl) && (!L_non_excepting_ops) && (!safe))
        continue;
      macro_flag = L_has_fragile_macro_operand (op);
      if (!Lsuper_no_danger_in_cb (cb, macro_flag, 1, 1))
        continue;

      if (!Lsuper_unique_memory_location
          (cb, op, &load_list, &store_list))
        continue;

      /*
       *      Replace pattern
       */
#ifdef DEBUG_SB_LOOP_GLOBAL_VAR_MIG
      if (Lsuper_debug_loop_classic_opti)
	{
          fprintf (stderr,
                   "**Apply global var mig cb = %d, op = %d (is_dupl = %d)\n",
                   cb->id, op->id, is_dupl);
	}
#endif
      new_reg =
        L_new_register_operand (++L_fn->max_reg_id, L_opcode_ctype2 (op),
                                L_PTYPE_NULL);

      /* Insert load op into preheader */
      if (List_size(load_list) || 
	  (Lsuper_store_migration_mode == L_STORE_MIGRATION_NO_COND))
	{
	  L_Oper *rep_load = NULL;
	  rep_load = load_list ? (L_Oper*)List_first(load_list) :
	    (L_Oper*)List_first(store_list);
	  new_load_opc = L_corresponding_load (rep_load);
	  new_load = Lsuper_insert_load_ops_for_global_var_mig (loop,
								new_load_opc,
								new_reg,
								rep_load,
								is_dupl, safe);
	  List_start(load_list);
	  while ((ptr=(L_Oper*)List_next(load_list)))
	    {
	      if (ptr->opc != new_load_opc)
		L_warn("L_sb_loop_global_var_mig: loads of different sizes "
		       "detected.  This is very bad.  See RDB.");
              if ((!(L_EXTRACT_BIT_VAL (ptr->flags, L_OPER_MASK_PE))) &&
                  (!is_dupl) && (!safe) && (L_mask_potential_exceptions))
                {
                  /* Insert checks at locations of nonspeculative loads that
                     are removed */
                  if (L_generate_spec_checks)
                    {
                      check_op = L_insert_check_after (cb, new_load, ptr);
                      check_op->src[0] = L_copy_operand (new_reg);
                    }
                }
	      /*
	       * At some point this will have to be changed to generate
	       *  the approriate oper if loads are different sizes/sign
	       */
              L_convert_to_extended_move (ptr, L_copy_operand (ptr->dest[0]),
					  L_copy_operand (new_reg));
#ifdef DEBUG_SB_LOOP_GLOBAL_VAR_MIG
	      if (Lsuper_debug_loop_classic_opti)
		{
                  fprintf (stderr, "\tconvert load to move %d\n", ptr->id);
		}
#endif
	    }
	}

      /* Insert store op into out_cbs */
      if (List_size(store_list))
        {
	  L_Oper *rep_store = NULL;
	  rep_store = (L_Oper*)List_first(store_list);
	  new_store_opc = L_corresponding_store(rep_store);
          Lsuper_insert_store_ops_for_global_var_mig (loop, 
						      new_store_opc,
                                                      new_reg,
						      rep_store,
						      is_dupl, safe, 
						      &pred_set);
	  List_start(store_list);
	  while ((ptr=(L_Oper*)List_next(store_list)))
	    {
	      if (ptr->opc != new_store_opc)
		L_warn("L_sb_loop_global_var_mig: stores of different sizes "
		       "detected.  This is very bad.  See RDB.");
	      if (pred_set)
		{
		  if (Lsuper_store_migration_mode == 
		      L_STORE_MIGRATION_FULL_PRED)
		    cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
		  pred_set_op = L_copy_operation(pred_set);
		  pred_set_op->pred[0] = L_copy_operand(ptr->pred[0]);
		  L_insert_oper_after(cb, ptr, pred_set_op);
		}
              L_convert_to_extended_move (ptr, L_copy_operand (new_reg),
					  L_copy_operand (ptr->src[2]));
#ifdef DEBUG_SB_LOOP_GLOBAL_VAR_MIG
	      if (Lsuper_debug_loop_classic_opti)
		{
                  fprintf (stderr, "\tconvert store to move %d\n", ptr->id);
		}
#endif
	    }

	  if (pred_set)
	    {
	      L_delete_oper (NULL, pred_set);
	      pred_set = NULL;
	    }
        }

      /* dispose of temporary operands */
      load_list = List_reset(load_list);
      store_list = List_reset(store_list);
      L_delete_operand (new_reg);

      change += 1;
    }

  return change;
}

/*
 *      Not updated for hyperblocks!
 */
int
L_sb_loop_copy_propagation (L_Inner_Loop * loop, int is_dupl)
{
  int change;
  L_Cb *cb, *dup;
  L_Oper *opA, *opB, *opC;

  if (!is_dupl)
    L_punt ("L_sb_loop_copy_propagation: can only apply on duplicate");

  change = 0;
  cb = loop->cb;
  dup = L_duplicate_cb[loop->id];
  if (dup == NULL)
    return 0;

  for (opA = dup->first_op; opA != NULL; opA = opA->next_op)
    {
      for (opB = dup->first_op; opB != NULL; opB = opB->next_op)
        {
          int i, macro_flag;
          if (opA == opB)
            continue;
          /*
           *  Match pattern
           */
          if (!L_move_opcode (opA))
            break;
          if (!L_is_variable (opA->src[0]))
            break;
          if (L_same_operand (opA->dest[0], opA->src[0]))
            break;
          if (!Lsuper_unique_def_in_loop (dup, opA))
            break;
          if (!L_is_src_operand (opA->dest[0], opB))
            continue;
          if (!L_can_change_src_operand (opB, opA->dest[0]))
            continue;
          opC = Lsuper_find_equivalent_op (cb, opA);
          if (opC == NULL)
            continue;
          if (!Lsuper_no_defs_between_in_loop
              (loop, opA, opB, opC, opA->dest[0]))
            continue;
          if (!Lsuper_no_defs_between_in_loop
              (loop, opA, opB, opC, opA->src[0]))
            continue;
          macro_flag = L_has_fragile_macro_operand (opA);
          if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
            continue;
          /*
           *  Replace pattern
           */
          for (i = 0; i < L_max_src_operand; i++)
            {
              if (L_same_operand (opA->dest[0], opB->src[i]))
                {
                  L_delete_operand (opB->src[i]);
                  opB->src[i] = L_copy_operand (opA->src[0]);
                  change += 1;

#ifdef DEBUG_SB_LOOP_COPY_PROP
                  if (Lsuper_debug_loop_classic_opti)
                    {
                      fprintf (stderr,
                               "**Apply loop copy prop op%d "
                               "-> op%d (dup %d)\n",
                               opA->id, opB->id, dup->id);
                    }
#endif

                }
            }
        }
    }
  return change;
}

int
L_sb_loop_operation_folding (L_Inner_Loop * loop, int is_dupl)
{
  int change;
  L_Cb *cb;
  L_Oper *opA, *opB, *opC, *new_op;
  L_Operand *base, *offset;

  change = 0;
  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;
  if (cb == NULL)
    return 0;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
        {
          int macro_flag;
          /*
           *  Match pattern no. 1
           */
          if (!L_move_opcode (opA))
            break;
          if (!L_is_variable (opA->src[0]))
            break;
          if (L_same_operand (opA->dest[0], opA->src[0]))
            break;
          if (!L_signed_int_cond_branch_opcode (opB))
            continue;
          if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
                L_same_operand (opA->dest[0], opB->src[1])))
            continue;
          if (!(Lsuper_invariant_operand (cb, opB->src[0]) ||
                Lsuper_invariant_operand (cb, opB->src[1])))
            continue;
          if (!(L_same_operand (opA->pred[0], opB->pred[0]) ||
                PG_superset_predicate_ops (opA, opB)))
            continue;
          if (!L_no_defs_between (opA->dest[0], opA, opB))
            continue;
          if (L_same_def_reachs (opA->src[0], opA, opB))
            continue;
          opC = L_next_def (opA->src[0], opA);
          if (!Lsuper_general_increment_operation (opC))
            continue;
          if (!(L_same_operand (opC->pred[0], opB->pred[0]) ||
                PG_superset_predicate_ops (opC, opB)))
            continue;
          if (!L_no_defs_between (opA->src[0], opC, opB))
            continue;
          macro_flag = L_has_fragile_macro_operand (opB);
          if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
            continue;
          /*
           *  Replace pattern no. 1
           */

#ifdef DEBUG_SB_LOOP_OP_FOLD
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr, "**Apply loop op fold to %d and %d (cb %d)\n",
                       opA->id, opB->id, cb->id);
            }
#endif

          if (L_int_add_opcode (opC))
            new_op = L_create_new_op (Lop_ADD);
          else
            new_op = L_create_new_op (Lop_SUB);

          if (L_same_operand (opA->dest[0], opB->src[0]))
            base = L_copy_operand (opB->src[1]);
          else
            base = L_copy_operand (opB->src[0]);

          if (L_same_operand (opA->src[0], opC->src[0]))
            offset = L_copy_operand (opC->src[1]);
          else
            offset = L_copy_operand (opC->src[0]);

          new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                    L_native_machine_ctype,
                                                    L_PTYPE_NULL);
          new_op->src[0] = base;
          new_op->src[1] = offset;

          if (is_dupl)
            {
              L_Cb *original;
              original = loop->cb;
              L_insert_oper_after (original, original->last_op, new_op);
            }
          else
            {
              L_Cb *preheader;
              preheader = loop->preheader;
              L_insert_oper_after (preheader, preheader->last_op, new_op);
            }

          L_delete_operand (opB->src[0]);
          L_delete_operand (opB->src[1]);
          opB->src[0] = L_copy_operand (opC->dest[0]);
          opB->src[1] = L_copy_operand (new_op->dest[0]);

          change += 1;
        }
    }
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
        {
          int macro_flag, s1A_invariant, s1B_matches;
          int old_num_oper, new_num_oper;
          L_Operand *old_src1, *old_src2;
          /*
           *  Match pattern no. 2 (op folding for later dead code removal)
           */
          if (!L_int_add_opcode (opA))
            break;
          if (Lsuper_add_increment_operation (opA))
            break;
          if (!(L_int_cond_branch_opcode (opB) ||
                L_load_opcode (opB) ||
                L_store_opcode (opB) || L_int_add_opcode (opB)))
            continue;
          if (L_unsigned_int_cond_branch_opcode (opB))
            continue;
          if (!(Lsuper_invariant_operand (cb, opA->src[0]) ||
                Lsuper_invariant_operand (cb, opA->src[1])))
            break;
          if (!Lsuper_unique_def_in_loop (cb, opA))
            break;
          if (!(Lsuper_invariant_operand (cb, opB->src[0]) ||
                Lsuper_invariant_operand (cb, opB->src[1])))
            continue;
          /* Next 2 rules so don't mess up adressing modes for mips */
#if 0
          /* may need to add some calls to Mspec later here to make
             sure the instruction that is generated does not get expanded
             by the code generator, for now just gonna do
             this opti blindly !!!! */

          if (L_is_int_zero (opB->src[0]) || L_is_int_zero (opB->src[1]))
            continue;
          if ((L_load_opcode (opB) ||
               L_store_opcode (opB))
              &&
              (L_is_int_constant (opB->src[0]) ||
               L_is_int_constant (opB->src[1])))
            continue;
#endif
          if (!(L_same_operand (opA->dest[0], opB->src[0]) ||
                L_same_operand (opA->dest[0], opB->src[1])))
            continue;
          if (!(L_same_operand (opA->pred[0], opB->pred[0]) ||
                PG_superset_predicate_ops (opA, opB)))
            continue;
          if (!Lsuper_no_uses_in_cb_by_others (opA->dest[0], opB, cb))
            continue;
          if (!L_same_def_reachs (opA->src[0], opA, opB))
            continue;
          if (!L_same_def_reachs (opA->src[1], opA, opB))
            continue;
          macro_flag = (L_has_fragile_macro_src_operand (opA) ||
                        L_has_fragile_macro_src_operand (opB));
          if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
            continue;
          /*
           *  Replace pattern no. 2
           */

          s1A_invariant = Lsuper_invariant_operand (cb, opA->src[0]);
          s1B_matches = L_same_operand (opA->dest[0], opB->src[0]);

          /* save info on original opB */
          old_num_oper = M_num_oper_required_for (opB, L_fn->name);
          old_src1 = opB->src[0];
          old_src2 = opB->src[1];

          if (L_int_cond_branch_opcode (opB))
            new_op = L_create_new_op (Lop_SUB);
          else
            new_op = L_create_new_op (Lop_ADD);
          new_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                    L_native_machine_ctype,
                                                    L_PTYPE_NULL);
          if (s1B_matches)
            new_op->src[0] = L_copy_operand (opB->src[1]);
          else
            new_op->src[0] = L_copy_operand (opB->src[0]);

          if (s1A_invariant)
            new_op->src[1] = L_copy_operand (opA->src[0]);
          else
            new_op->src[1] = L_copy_operand (opA->src[1]);

          if (s1B_matches)
            {
              if (s1A_invariant)
                opB->src[0] = L_copy_operand (opA->src[1]);
              else
                opB->src[0] = L_copy_operand (opA->src[0]);
              opB->src[1] = L_copy_operand (new_op->dest[0]);
            }
          else
            {
              opB->src[0] = L_copy_operand (new_op->dest[0]);
              if (s1A_invariant)
                opB->src[1] = L_copy_operand (opA->src[1]);
              else
                opB->src[1] = L_copy_operand (opA->src[0]);
            }

          /* see if new opB doesn't get coverted to more instr than original */
          new_num_oper = M_num_oper_required_for (opB, L_fn->name);

          /* Abort opti if new > old */
          if (new_num_oper > old_num_oper)
            {
              L_delete_operand (opB->src[0]);
              L_delete_operand (opB->src[1]);
              opB->src[0] = old_src1;
              opB->src[1] = old_src2;
              L_delete_oper (NULL, new_op);
              continue;
            }

          /* else perform opti */

#ifdef DEBUG_SB_LOOP_OP_FOLD
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr,
                       "**Apply loop op fold 2 to %d and %d (cb %d)\n",
                       opA->id, opB->id, cb->id);
            }
#endif

          if (is_dupl)
            {
              L_Cb *original;
              original = loop->cb;
              L_insert_oper_after (original, original->last_op, new_op);
            }
          else
            {
              L_Cb *preheader;
              preheader = loop->preheader;
              L_insert_oper_after (preheader, preheader->last_op, new_op);
            }

          L_delete_operand (old_src1);
          L_delete_operand (old_src2);

          change += 1;
        }
    }

  /* SAM 7-95, Added to catch a few wierd cases where induction variable ops
     were not being created in hb/sb loops */
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
        {
          int macro_flag;

          /*
           *  Match pattern no. 3     (rx = ry + 1)... (ry = rx)
           *                          ==> ry = ry + 1
           */

          if (!(L_int_add_opcode (opA) || L_int_sub_opcode (opB)))
            break;
          if (L_same_operand (opA->dest[0], opA->src[0]))
            break;
          if (!L_is_int_constant (opA->src[1]))
            break;
          if (!L_move_opcode (opB))
            continue;
          if (!L_same_operand (opA->dest[0], opB->src[0]))
            continue;
          if (!L_same_operand (opA->src[0], opB->dest[0]))
            continue;
          if (!(L_same_operand (opA->pred[0], opB->pred[0]) ||
                PG_superset_predicate_ops (opA, opB)))
            continue;
          if (!L_no_defs_between (opA->dest[0], opA, opB))
            continue;
          if (!L_no_defs_between (opA->src[0], opA, opB))
            continue;
          macro_flag = L_has_fragile_macro_operand (opA);
          if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
            break;

          /*
           *  Replace pattern no. 3
           */

#ifdef DEBUG_SB_LOOP_OP_FOLD
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr,
                       "**Apply loop op fold3 to %d and %d (cb %d)\n",
                       opA->id, opB->id, cb->id);
            }
#endif
          L_change_opcode (opB, opA->opc);
          L_delete_operand (opB->src[0]);
          opB->src[0] = L_copy_operand (opA->src[0]);
          opB->src[1] = L_copy_operand (opA->src[1]);

          change += 1;
        }
    }

  return change;
}

/*
 *      Note at this point only instrs with 1 dest and 2 srcs allowed to
 *      be deleted as loop dead code.
 */
int
L_sb_loop_dead_code (L_Inner_Loop * loop, int is_dupl)
{
  int change, operand_indx, def_flag, use_flag,
    def_before_used, s1_inv, s2_inv, bad_flag, predicated_def;
  L_Cb *cb;
  L_Oper *op, *next, *def, *undo_op, *move_op, *new_op, *ptr;
  L_Attr *attr;
  change = 0;

  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;
  if (cb == NULL)
    return 0;

  for (op = cb->first_op; op != NULL; op = next)
    {
      int macro_flag, load_flag, store_flag;
      next = op->next_op;
      /*
       *      Match pattern no. 1
       */
      if (!L_safe_to_delete_opcode (op))
        continue;
      if (!L_is_register (op->dest[0]))
        continue;
      if (!Lsuper_no_uses_in_cb (op->dest[0], cb))
        continue;
      if (L_is_predicated (op))
        continue;
      if (!L_no_br_between (cb->first_op, op))
        break;
      if (!Lsuper_unique_def_in_loop (cb, op))
        continue;
      if ((!is_dupl) && (!Lsuper_def_reachs_all_out_cb_of_loop (cb, op)))
        continue;
      s1_inv = Lsuper_invariant_operand (cb, op->src[0]);
      s2_inv = Lsuper_invariant_operand (cb, op->src[1]);
      if (!(s1_inv | s2_inv))
        continue;
      /*  If both operands invariant, better to handle with inv code rem */
      if (s1_inv & s2_inv)
        continue;
      if (s1_inv)
        def = L_find_def (cb, op->src[1]);
      else
        def = L_find_def (cb, op->src[0]);
      if (!Lsuper_general_increment_operation (def))
        continue;
      if (!Lsuper_invariant_operand (cb, def->src[1]))
        continue;
      if (!Lsuper_unique_def_in_loop (cb, def))
        continue;
      macro_flag = (L_is_fragile_macro (op->dest[0]) ||
                    L_is_fragile_macro (op->src[0]) ||
                    L_is_fragile_macro (op->src[1]) ||
                    L_is_fragile_macro (def->src[1]));
      load_flag = L_general_load_opcode (op);
      store_flag = L_general_store_opcode (op);
      if (!Lsuper_no_danger_in_cb (cb, macro_flag, load_flag, store_flag))
        continue;
      if ((load_flag | store_flag)
          && (!Lsuper_no_memory_conflicts_in_loop (cb, op)))
        continue;

      bad_flag = 0;

      for (ptr = def->next_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (!L_is_control_oper (ptr))
            continue;
          if (L_is_branch_target (ptr, cb))
            continue;
          if (!L_same_def_reachs (def->pred[0], def, ptr))
            {
              bad_flag = 1;
              break;
            }
        }

      if (bad_flag)
        continue;

      predicated_def = L_is_predicated (def);

      /*
       *      Replace pattern no. 1
       */

#ifdef DEBUG_SB_LOOP_DEAD_CODE
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "**Apply loop dead code to %d (cb %d) (is_dupl = %d)\n",
                   op->id, cb->id, is_dupl);
        }
#endif

      def_before_used =
        Lsuper_is_defined_before_used_by (cb, def->dest[0], op);
      use_flag = def_flag = 0;

      if (L_int_add_opcode (def))
        undo_op = L_create_new_op (Lop_SUB);
      else
        undo_op = L_create_new_op (Lop_ADD);
      undo_op->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                 L_native_machine_ctype,
                                                 L_PTYPE_NULL);
      undo_op->src[0] = L_copy_operand (def->src[0]);
      undo_op->src[1] = L_copy_operand (def->src[1]);
      undo_op->pred[0] = L_copy_operand (def->pred[0]);
      undo_op->pred[1] = L_copy_operand (def->pred[1]);

      /* Recovery code if def was predicated */
      move_op = L_create_new_op (Lop_MOV);
      move_op->src[0] = L_copy_operand (undo_op->src[0]);
      move_op->dest[0] = L_copy_operand (undo_op->dest[0]);


      new_op = L_create_new_op (op->opc);
      new_op->dest[0] = L_copy_operand (op->dest[0]);

      for (operand_indx = 0; operand_indx < L_max_src_operand; operand_indx++)
        {
          if (L_same_operand (op->src[operand_indx], def->dest[0]))
            {
              new_op->src[operand_indx] = L_copy_operand (undo_op->dest[0]);
            }
          else
            {
              new_op->src[operand_indx] =
                L_copy_operand (op->src[operand_indx]);
            }
        }

      /* Copy attribute field */
      attr = L_copy_attr (op->attr);
      new_op->attr = L_concat_attr (new_op->attr, attr);

      /* DMG */
      if (op->sync_info != NULL)
        {
          new_op->sync_info = L_copy_sync_info (op->sync_info);
          L_insert_all_syncs_in_dep_opers (new_op);
          L_make_sync_arcs_conservative (new_op);
        }

      if (op->acc_info)
	new_op->acc_info = L_copy_mem_acc_spec_list (op->acc_info);

      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (ptr == op)
            use_flag = 1;
          if (ptr == def)
            def_flag = 1;
          if (!L_is_control_oper (ptr))
            continue;

          /* Continue on loop back edges since no uses of in loop */
          if (L_is_branch_target (ptr, cb))
            continue;

          /* Dont do anything unless in live-out of br */
          if (!L_in_oper_OUT_set (cb, ptr, op->dest[0], TAKEN_PATH))
            continue;

          if ((!def_flag) & (!use_flag))
            {
              if (def_before_used)
                {
                  L_insert_op_at_dest_of_br (cb, ptr, op, 1);
                }
              else
                {
                  L_insert_op_at_dest_of_br (cb, ptr, new_op, 1);
                  L_insert_op_at_dest_of_br (cb, ptr, undo_op, 1);
                  if (predicated_def)
                    L_insert_op_at_dest_of_br (cb, ptr, move_op, 1);
                }
            }
          else if ((!def_flag) & (use_flag))
            {
              if (def_before_used)
                L_punt ("L_sb_loop_dead_code: illegal use");
              L_insert_op_at_dest_of_br (cb, ptr, op, 1);
            }
          else if ((def_flag) & (!use_flag))
            {
              if (!def_before_used)
                L_punt ("L_sb_loop_dead_code: illegal def");
              L_insert_op_at_dest_of_br (cb, ptr, new_op, 1);
              L_insert_op_at_dest_of_br (cb, ptr, undo_op, 1);
              if (predicated_def)
                L_insert_op_at_dest_of_br (cb, ptr, move_op, 1);
            }
          else
            {
              if (def_before_used)
                {
                  L_insert_op_at_dest_of_br (cb, ptr, op, 1);
                }
              else
                {
                  L_insert_op_at_dest_of_br (cb, ptr, new_op, 1);
                  L_insert_op_at_dest_of_br (cb, ptr, undo_op, 1);
                  if (predicated_def)
                    L_insert_op_at_dest_of_br (cb, ptr, move_op, 1);
                }
            }
        }

      if (!(def_flag & use_flag))
        L_punt ("L_sb_loop_dead_code: def_flag and use_flag should be 1");
      /* DIA - Place op at fallthru path if one exists and dest is live out */
      if (((L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
            (!L_EXTRACT_BIT_VAL (cb->flags,
                                 L_CB_HYPERBLOCK_NO_FALLTHRU)))
           || (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK)
               && !L_uncond_branch_opcode (cb->last_op)
               && !L_register_branch_opcode (cb->last_op)))
          && L_in_oper_OUT_set (cb, cb->last_op, op->dest[0], FALL_THRU_PATH))
        {
          if (def_before_used)
            {
              L_insert_op_at_fallthru_dest (cb, op, 1);
            }
          else
            {
              L_insert_op_at_fallthru_dest (cb, new_op, 1);
              L_insert_op_at_fallthru_dest (cb, undo_op, 1);
              if (predicated_def)
                L_insert_op_at_fallthru_dest (cb, move_op, 1);
            }
        }

      L_delete_oper (NULL, new_op);
      L_delete_oper (NULL, undo_op);
      L_delete_oper (NULL, move_op);
      L_delete_oper (cb, op);

      L_do_flow_analysis (L_fn, LIVE_VARIABLE);

      change += 1;
    }

  for (op = cb->first_op; op != NULL; op = next)
    {
      int macro_flag, load_flag, store_flag;
      L_Operand *src;
      next = op->next_op;
      /*
       *      Match pattern no. 2
       */
      if (!L_safe_to_delete_opcode (op))
        continue;
      if (!L_is_register (op->dest[0]))
        continue;
      /* JWS 98-5-27 -- Do not match pred defines. */
      if (L_pred_define_opcode (op))
        continue;
      if (L_is_predicated (op))
        continue;
      if (!Lsuper_no_uses_in_cb (op->dest[0], cb))
        continue;
      if (!Lsuper_unique_def_in_loop (cb, op))
        continue;
      if ((!is_dupl) && (!Lsuper_def_reachs_all_out_cb_of_loop (cb, op)))
        continue;
      s1_inv = Lsuper_invariant_operand (cb, op->src[0]);
      s2_inv = Lsuper_invariant_operand (cb, op->src[1]);
      if (!(s1_inv | s2_inv))
        continue;
      /*  If both operands invariant, better to handle with inv code rem */
      if (s1_inv & s2_inv)
        continue;
      if (s1_inv)
        {
          src = op->src[1];
          if (L_is_fragile_macro (src))
            continue;
          def = L_find_def (cb, op->src[1]);
        }
      else
        {
          src = op->src[0];
	  if (L_is_fragile_macro (src))
            continue;
          def = L_find_def (cb, op->src[0]);
        }
      if (!Lsuper_unique_def_in_loop (cb, def))
        continue;
      if (!Lsuper_is_defined_before_used_by (cb, src, op))
        continue;
      if (!Lsuper_not_live_at_exits_between (cb, op->dest[0], def, op))
        continue;
      macro_flag = L_has_fragile_macro_operand (op);
      load_flag = L_general_load_opcode (op);
      store_flag = L_general_store_opcode (op);
      if (!Lsuper_no_danger_in_cb (cb, macro_flag, load_flag, store_flag))
        continue;
      if ((load_flag | store_flag)
          && (!Lsuper_no_memory_conflicts_in_loop (cb, op)))
        continue;
      /*
       *      Replace pattern no. 2
       */

#ifdef DEBUG_SB_LOOP_DEAD_CODE
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "**Apply loop dead code2 to %d (cb %d) (is_dupl = %d)\n",
                   op->id, cb->id, is_dupl);
        }
#endif

      for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
        {
          if (!L_is_control_oper (ptr))
            continue;
          if (L_is_branch_target (ptr, cb))
            continue;
          if (!L_in_oper_OUT_set (cb, ptr, op->dest[0], TAKEN_PATH))
            continue;
          L_insert_op_at_dest_of_br (cb, ptr, op, 1);
        }

      if (((L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
            (!L_EXTRACT_BIT_VAL (cb->flags,
                                 L_CB_HYPERBLOCK_NO_FALLTHRU)))
           || (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK)
               && !L_uncond_branch_opcode (cb->last_op)
               && !L_register_branch_opcode (cb->last_op)))
          && L_in_oper_OUT_set (cb, cb->last_op, op->dest[0], FALL_THRU_PATH))
        {
          L_insert_op_at_fallthru_dest (cb, op, 1);
        }

      L_delete_oper (cb, op);

      L_do_flow_analysis (L_fn, LIVE_VARIABLE);

      change += 1;
    }

  return change;
}

/*
 *      Note these ind opti not indended to redo those done in Lopti,
 *      but intended to find additional opportunities for induction elims
 *      that Lopti either cannot perform or are exposed by superblocks!!!
 */
int
L_sb_loop_ind_var_elim (L_Inner_Loop * loop, int is_dupl)
{
  int change, shift_val, shift_opc;
  L_Cb *cb;
  L_Oper *opA, *opB, *next, *new_op1, *new_op2, *new_op3, *new_op4, *ptr;

  change = 0;
  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;
  if (cb == NULL)
    return 0;

  /*
   *  1.
   *          Eliminate induction which is not used in sb loop.  Repair
   *          value from another induction with SAME increment
   *          at all exit points.
   */

  if (!Lsuper_loop_iter_larger_than (loop, L_LOOP_MIN_ITER_FOR_IND_ELIM_1))
    return change;

  for (opA = cb->first_op; opA != NULL; opA = next)
    {
      next = opA->next_op;
      for (opB = cb->first_op; opB != NULL; opB = opB->next_op)
        {
          int macro_flag;
          if (opA == opB)
            continue;
          /*
           *  Match pattern no. 1
           */
          /* opA defines induction var */
          if (!Lsuper_add_increment_operation (opA))
            break;
          if (!L_is_register (opA->dest[0]))
            break;
          if (!L_is_int_constant (opA->src[1]))
            break;
          if (!Lsuper_unique_def_in_loop (cb, opA))
            break;
          /* no uses of opA->dest */
          if (!Lsuper_no_uses_in_cb_by_others (opA->dest[0], opA, cb))
            break;
          /* opB defines induction var */
          if (!Lsuper_add_increment_operation (opB))
            continue;
          if (!L_is_int_constant (opB->src[1]))
            continue;
          if (!Lsuper_unique_def_in_loop (cb, opB))
            continue;
          /* same increment */
          if (!L_same_operand (opA->src[1], opB->src[1]))
            continue;
          /* same predicate */
          if (!L_same_operand (opA->pred[0], opB->pred[0]))
            continue;
          if (!Lsuper_no_branches_betw (cb, opA, opB))
            continue;
          macro_flag = L_has_fragile_macro_operand (opA);
          if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
            continue;
          /*
           *  Replace pattern no. 1
           */

#ifdef DEBUG_SB_LOOP_IND_ELIM
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr,
                       "**Apply ind elim1A to %d ->%d (cb %d) (is_dupl %d)\n",
                       opA->id, opB->id, cb->id, is_dupl);
            }
#endif

          /* Create preheader ops */
          new_op1 = L_create_new_op (Lop_SUB);
          new_op1->dest[0] =
            L_new_register_operand (++L_fn->max_reg_id,
                                    L_native_machine_ctype, L_PTYPE_NULL);
          new_op1->src[0] = L_copy_operand (opA->dest[0]);
          new_op1->src[1] = L_copy_operand (opB->dest[0]);

          if (is_dupl)
            {
              L_Cb *original;
              original = loop->cb;
              L_insert_oper_after (original, original->last_op, new_op1);
            }
          else
            {
              L_Cb *preheader;
              preheader = loop->preheader;
              L_insert_oper_after (preheader, preheader->last_op, new_op1);
            }

          /* Create ops at exit points of sb */
          new_op2 = L_create_new_op (Lop_ADD);
          new_op2->dest[0] = L_copy_operand (opA->dest[0]);
          new_op2->src[0] = L_copy_operand (opB->dest[0]);
          new_op2->src[1] = L_copy_operand (new_op1->dest[0]);

          for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
            {
              if (!L_is_control_oper (ptr))
                continue;
              if (L_is_branch_target (ptr, cb))
                continue;
              if (!L_in_oper_OUT_set (cb, ptr, opA->dest[0], TAKEN_PATH))
                continue;
              L_insert_op_at_dest_of_br (cb, ptr, new_op2, 1);
            }

          if (((L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
                (!L_EXTRACT_BIT_VAL (cb->flags,
                                     L_CB_HYPERBLOCK_NO_FALLTHRU)))
               || (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK)
                   && !L_uncond_branch_opcode (cb->last_op)
                   && !L_register_branch_opcode (cb->last_op)))
              && L_in_oper_OUT_set (cb, cb->last_op, opA->dest[0],
                                    FALL_THRU_PATH))
            {
              L_insert_op_at_fallthru_dest (cb, new_op2, 1);
            }

          L_delete_oper (NULL, new_op2);
          L_delete_oper (cb, opA);

          L_do_flow_analysis (L_fn, LIVE_VARIABLE);

          change += 1;

          break;
        }
    }

  /*
   *  2.
   *          Eliminate induction which is not used in sb loop.  Repair
   *          value from another induction with DIFFERENT increment
   *          at all exit points.
   */

  if (!Lsuper_loop_iter_larger_than (loop, L_LOOP_MIN_ITER_FOR_IND_ELIM_2))
    return change;

  for (opA = cb->first_op; opA != NULL; opA = next)
    {
      next = opA->next_op;
      for (opB = cb->first_op; opB != NULL; opB = opB->next_op)
        {
          int macro_flag;
          if (opA == opB)
            continue;
          /*
           *  Match pattern no. 2
           */
          /* opA defines induction var */
          if (!Lsuper_add_increment_operation (opA))
            break;
          if (!L_is_register (opA->dest[0]))
            break;
          if (!L_abs_is_power_of_two (opA->src[1]))
            break;
          if (!Lsuper_unique_def_in_loop (cb, opA))
            break;
          /* no uses of opA->dest */
          if (!Lsuper_no_uses_in_cb_by_others (opA->dest[0], opA, cb))
            break;
          /* opB defines induction var */
          if (!Lsuper_add_increment_operation (opB))
            continue;
          if (!L_abs_is_power_of_two (opB->src[1]))
            continue;
          if (!Lsuper_unique_def_in_loop (cb, opB))
            continue;
          /* only can apply for same signed int increments */
          if (!Lsuper_same_sign_integers (opA->src[1], opB->src[1]))
            continue;
          /* same predicate */
          if (!L_same_operand (opA->pred[0], opB->pred[0]))
            continue;
          if (!Lsuper_no_branches_betw (cb, opA, opB))
            continue;
          macro_flag = L_has_fragile_macro_operand (opA);
          if (!Lsuper_no_danger_in_cb (cb, macro_flag, 0, 0))
            continue;
          /*
           *  Replace pattern no. 2
           */

#ifdef DEBUG_SB_LOOP_IND_ELIM
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr,
                       "**Apply ind elim1B to %d (cb %d) (is_dupl %d)\n",
                       opA->id, cb->id, is_dupl);
            }
#endif

          if (ITabs (opA->src[1]->value.i) >= ITabs (opB->src[1]->value.i))
            {
              shift_opc = Lop_LSL;
              shift_val =
                C_log2 (ITicast
                        (opA->src[1]->value.i / opB->src[1]->value.i));
            }
          else
            {
              shift_opc = Lop_ASR;
              shift_val =
                C_log2 (ITicast
                        (opB->src[1]->value.i / opA->src[1]->value.i));
            }

          /* Create preheader ops */
          new_op1 = L_create_new_op (shift_opc);
          new_op1->dest[0] =
            L_new_register_operand (++L_fn->max_reg_id,
                                    L_native_machine_ctype, L_PTYPE_NULL);
          new_op1->src[0] = L_copy_operand (opB->dest[0]);
          new_op1->src[1] = L_new_gen_int_operand (shift_val);

          new_op2 = L_create_new_op (Lop_SUB);
          new_op2->dest[0] =
            L_new_register_operand (++L_fn->max_reg_id,
                                    L_native_machine_ctype, L_PTYPE_NULL);
          new_op2->src[0] = L_copy_operand (opA->dest[0]);
          new_op2->src[1] = L_copy_operand (new_op1->dest[0]);

          if (is_dupl)
            {
              L_Cb *original;
              original = loop->cb;
              L_insert_oper_after (original, original->last_op, new_op1);
              L_insert_oper_after (original, original->last_op, new_op2);
            }
          else
            {
              L_Cb *preheader;
              preheader = loop->preheader;
              L_insert_oper_after (preheader, preheader->last_op, new_op1);
              L_insert_oper_after (preheader, preheader->last_op, new_op2);
            }

          /* Create ops at exit pts of sb */
          new_op3 = L_create_new_op (shift_opc);
          new_op3->dest[0] =
            L_new_register_operand (++L_fn->max_reg_id,
                                    L_native_machine_ctype, L_PTYPE_NULL);
          new_op3->src[0] = L_copy_operand (opB->dest[0]);
          new_op3->src[1] = L_new_gen_int_operand (shift_val);

          new_op4 = L_create_new_op (Lop_ADD);
          new_op4->dest[0] = L_copy_operand (opA->dest[0]);
          new_op4->src[0] = L_copy_operand (new_op3->dest[0]);
          new_op4->src[1] = L_copy_operand (new_op2->dest[0]);

          for (ptr = cb->first_op; ptr != NULL; ptr = ptr->next_op)
            {
              if (!L_is_control_oper (ptr))
                continue;
              if (L_is_branch_target (ptr, cb))
                continue;
              if (!L_in_oper_OUT_set (cb, ptr, opA->dest[0], TAKEN_PATH))
                continue;
              L_insert_op_at_dest_of_br (cb, ptr, new_op4, 1);
              L_insert_op_at_dest_of_br (cb, ptr, new_op3, 1);
            }

          if (((L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK) &&
                (!L_EXTRACT_BIT_VAL (cb->flags,
                                     L_CB_HYPERBLOCK_NO_FALLTHRU)))
               || (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK)
                   && !L_uncond_branch_opcode (cb->last_op)
                   && !L_register_branch_opcode (cb->last_op)))
              && L_in_oper_OUT_set (cb, cb->last_op, opA->dest[0],
                                    FALL_THRU_PATH))
            {
              L_insert_op_at_fallthru_dest (cb, new_op4, 1);
              L_insert_op_at_fallthru_dest (cb, new_op3, 1);
            }

          L_delete_oper (NULL, new_op3);
          L_delete_oper (NULL, new_op4);
          L_delete_oper (cb, opA);

          L_do_flow_analysis (L_fn, LIVE_VARIABLE);

          change += 1;

          break;
        }
    }

  return change;
}

/*
 *      After induction variable expansion, may produce some unnecessary
 *      induction vars that can be eliminated.
 */
int
L_sb_loop_ind_var_elim2 (L_Inner_Loop * loop, int is_dupl)
{
  int i, j, change, num_out_cb, num_basic_ind_var; 
  int *out_cb = NULL, *basic_ind_var = NULL;
  L_Cb *cb;
  L_Operand *operand1, *operand2;

  change = 0;
  operand1 = operand2 = NULL;

  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;
  if (cb == NULL)
    return 0;
#ifdef DEBUG_SB_LOOP_IND_ELIM2
  if (Lsuper_debug_loop_classic_opti)
    fprintf (stderr, "*** Enter L_sb_loop_ind_var_elim2 (cb %d)\n", cb->id);
#endif

  Lsuper_find_all_ind_info (loop, cb);

  /* setup out_cb array */
  num_out_cb = Set_size (loop->out_cb);
  if (num_out_cb > 0)
    {
      out_cb = (int *) Lcode_malloc (sizeof (int) * num_out_cb);
      Set_2array (loop->out_cb, out_cb);
    }

  /* setup basic_ind_var array */
  num_basic_ind_var = Set_size (loop->basic_ind_var);
  if (num_basic_ind_var > 0)
    {
      basic_ind_var = (int *) Lcode_malloc (sizeof (int) * num_basic_ind_var);
      Set_2array (loop->basic_ind_var, basic_ind_var);
    }

  /*
   *  1.
   *          Same increment, constant offset of initial val
   *          merge operand1 and operand2 into operand2
   */
  for (i = 0; i < num_basic_ind_var; i++)
    {
      if (operand1)
        L_delete_operand (operand1);
      /* create a temporary operand */
      operand1 = L_new_register_operand (basic_ind_var[i],
                                         L_native_machine_ctype,
                                         L_PTYPE_NULL);
      for (j = 0; j < num_basic_ind_var; j++)
        {
          if (i == j)
            continue;
          if (operand2)
            L_delete_operand (operand2);
          /* create a temporary operand */
          operand2 = L_new_register_operand (basic_ind_var[j],
                                             L_native_machine_ctype,
                                             L_PTYPE_NULL);
          /*
           *  Match pattern no. 1
           */
          if (!Set_in (loop->basic_ind_var, operand1->value.r) ||
	      !Set_in (loop->basic_ind_var, operand2->value.r) ||
	      !L_same_ind_increment (operand1, operand2, loop->ind_info) ||
	      !L_not_live_in_out_cb (out_cb, num_out_cb, operand1))
	      continue;
          if (!L_ind_constant_offset_initial_val
	      (loop->preheader, operand1, operand2, NULL, NULL,
               loop->ind_info))
            continue;
          if (!Lsuper_all_uses_of_ind_can_change_offset
              (loop, out_cb, num_out_cb, operand1, operand2))
            continue;
          if (!Lsuper_no_uses_of_between_first_and_last_defs
              (loop, operand1, operand2))
            continue;
          if (!Lsuper_no_branches_betw_defs (cb, operand1, operand2))
            continue;
          if (!Lsuper_defs_on_same_pred (cb, operand1, operand2))
            continue;
          if (!Lsuper_no_danger_in_cb (cb, 0, 0, 0))
            continue;
          /*
           *  Replace pattern no. 1
           */

#ifdef DEBUG_SB_LOOP_IND_ELIM2
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr, "Apply loop ind elim2: replace  ");
              L_print_operand (stderr, operand1, 1);
              fprintf (stderr, " with ");
              L_print_operand (stderr, operand2, 1);
              fprintf (stderr, "(cb %d) (is_dupl %d)\n", cb->id, is_dupl);
            }
#endif

          /* update all uses of ind var */
          Lsuper_induction_elim_2 (loop, operand1, operand2);

          /* cleanup info in loop structure */
          loop->basic_ind_var = Set_delete (loop->basic_ind_var,
					    operand1->value.r);
          L_invalidate_ind_var (operand1, loop->ind_info);
          change++;
        }
    }

  L_delete_operand (operand1);
  L_delete_operand (operand2);
  L_delete_all_ind_info (&(loop->ind_info));
  if (out_cb)
    Lcode_free (out_cb);
  if (basic_ind_var)
    Lcode_free (basic_ind_var);

  return (change);
}



/*
 *      Reinit ind vars to enable more post increment ld/st
 */
int
L_sb_loop_ind_var_reinit (L_Inner_Loop * loop, int is_dupl)
{
  int change;
  L_Cb *cb;
  L_Oper *opA, *last_use;

  change = 0;
  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;

  if (!cb)
    return 0;

#if 0
  /* might do this later (i.e. in code generator) even if it is
   * turned off here. */
  if (!Lsuper_do_post_inc_conv)
    return 0;
#endif

  /* check if post inc ld/st supported in the arch, just check int type */
#ifdef IA64BIT
  if (!(M_oper_supported_in_arch (Lop_LD_POST_Q) ||
        M_oper_supported_in_arch (Lop_ST_POST_Q)))
#else
  if (!(M_oper_supported_in_arch (Lop_LD_POST_I) ||
        M_oper_supported_in_arch (Lop_ST_POST_I)))
#endif
    return 0;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       *      match pattern
       */
      /* opA defines an induction variable */
      if (!Lsuper_general_increment_operation (opA))
        continue;
      if (!L_is_register (opA->dest[0]))
        continue;
      if (!Lsuper_invariant_operand (cb, opA->src[1]))
        continue;
      if (!Lsuper_unique_def_in_loop (cb, opA))
        continue;
      last_use = L_prev_use (opA->dest[0], opA);
      if (!(L_load_opcode (last_use) || L_store_opcode (last_use)))
        continue;
      if (!(L_same_operand (opA->dest[0], last_use->src[0]) ||
            L_same_operand (opA->dest[0], last_use->src[1])))
        continue;
      /* already can do post inc, no need to change anything */
      if (Lsuper_can_make_post_inc (last_use, opA))
        continue;
      if (!Lsuper_ind_should_be_reinitialized (cb, last_use, opA))
        {
#ifdef DEBUG_SB_LOOP_IND_REINIT
          if (Lsuper_debug_loop_classic_opti)
            {
              fprintf (stderr, "CANNOT reinit op%d (reg %d)\n",
                       opA->id, opA->dest[0]->value.r);
            }
#endif
          continue;
        }
      /*
       *      replace pattern
       */

#ifdef DEBUG_SB_LOOP_IND_REINIT
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "Apply ind var reinit to op%d (reg %d)\n",
                   opA->id, opA->dest[0]->value.r);
        }
#endif

      Lsuper_reinit_ind (loop, cb, last_use, opA, is_dupl);
      L_do_flow_analysis (L_fn, LIVE_VARIABLE);

      change++;
    }

  return change;
}

int
L_sb_loop_post_inc_conversion (L_Inner_Loop * loop, int is_dupl)
{
  int change, old_num_oper, new_num_oper, old_opc, redo_flow;
  L_Cb *cb;
  L_Oper *opA, *next, *last_use, *undo_op, *ptr;

  change = 0;
  if (is_dupl)
    cb = L_duplicate_cb[loop->id];
  else
    cb = loop->cb;

  if (!cb || !Lsuper_do_post_inc_conv)
    return 0;

  /* check if post inc ld/st supported in the arch, just check int type */

  if (((L_native_machine_ctype == L_CTYPE_LLONG) &&
       !(M_oper_supported_in_arch (Lop_LD_POST_Q) ||
	 M_oper_supported_in_arch (Lop_ST_POST_Q))) ||
      ((L_native_machine_ctype == L_CTYPE_INT) &&
       !(M_oper_supported_in_arch (Lop_LD_POST_I) ||
	 M_oper_supported_in_arch (Lop_ST_POST_I))))
    return 0;

  for (opA = cb->first_op; opA != NULL; opA = next)
    {
      next = opA->next_op;
      /*
       *      match pattern
       */
      /* opA defines an induction variable */
      if (!Lsuper_general_increment_operation (opA))
        continue;
      if (!L_is_register (opA->dest[0]))
        continue;
      if (!Lsuper_invariant_operand (cb, opA->src[1]))
        continue;
      if (!Lsuper_unique_def_in_loop (cb, opA))
        continue;
      last_use = L_prev_use (opA->dest[0], opA);
      if (!(L_load_opcode (last_use) || L_store_opcode (last_use)))
        continue;
      if (!(L_same_operand (opA->dest[0], last_use->src[0]) ||
            L_same_operand (opA->dest[0], last_use->src[1])))
        continue;
      /*
       *      replace pattern
       */
      old_num_oper = M_num_oper_required_for (last_use, L_fn->name);
      old_opc = last_use->opc;
      /* put ind var as first src of last_use */
      if (L_same_operand (last_use->src[1], opA->dest[0]))
        {
          L_Operand *temp = last_use->src[0];
          last_use->src[0] = last_use->src[1];
          last_use->src[1] = temp;
        }
      /* create post inc load */
      if (L_load_opcode (last_use))
        {
          L_change_opcode (last_use,
                           L_corresponding_postincrement_load (last_use));
          last_use->dest[1] = L_copy_operand (opA->dest[0]);
          last_use->src[2] = L_copy_operand (opA->src[1]);
          new_num_oper = M_num_oper_required_for (last_use, L_fn->name);
          /* undo opti if post inc load not supported in arch */
          if (new_num_oper > old_num_oper)
            {
              L_delete_operand (last_use->dest[1]);
              L_delete_operand (last_use->src[2]);
              last_use->dest[1] = NULL;
              last_use->src[2] = NULL;
              L_change_opcode (last_use, old_opc);
#ifdef DEBUG_SB_LOOP_POST_INC_CONV
              if (Lsuper_debug_loop_classic_opti)
                {
                  fprintf (stderr,
                           "CANNOT DO POST INCREMENT LOAD %d->%d\n",
                           last_use->id, opA->id);
                }
#endif
              continue;
            }
        }
      /* create post inc store */
      else
        {
          L_change_opcode (last_use,
                           L_corresponding_postincrement_store (last_use));
          last_use->dest[0] = L_copy_operand (opA->dest[0]);
          last_use->src[3] = L_copy_operand (opA->src[1]);
          new_num_oper = M_num_oper_required_for (last_use, L_fn->name);
          /* undo opti if post inc load not supported in arch */
          if (new_num_oper > old_num_oper)
            {
              L_delete_operand (last_use->dest[0]);
              L_delete_operand (last_use->src[3]);
              last_use->dest[0] = NULL;
              last_use->src[3] = NULL;
              L_change_opcode (last_use, old_opc);
#ifdef DEBUG_SB_LOOP_POST_INC_CONV
              if (Lsuper_debug_loop_classic_opti)
                {
                  fprintf (stderr,
                           "CANNOT DO POST INCREMENT STORE %d->%d\n",
                           last_use->id, opA->id);
                }
#endif
              continue;
            }
        }
#ifdef DEBUG_SB_LOOP_POST_INC_CONV
      if (Lsuper_debug_loop_classic_opti)
        {
          fprintf (stderr,
                   "Apply post increment conversion: op%d -> op%d (cb %d)\n",
                   last_use->id, opA->id, cb->id);
        }
#endif
      /* undo increment at every exit between last_use and opA in which
         opA->dest[0] is live */
      if (L_int_add_opcode (opA))
        undo_op = L_create_new_op (Lop_SUB);
      else
        undo_op = L_create_new_op (Lop_ADD);
      undo_op->dest[0] = L_copy_operand (opA->dest[0]);
      undo_op->src[0] = L_copy_operand (opA->src[0]);
      undo_op->src[1] = L_copy_operand (opA->src[1]);
      redo_flow = 0;
      for (ptr = last_use; ptr != opA; ptr = ptr->next_op)
        {
          if (!L_general_branch_opcode (ptr))
            continue;
          if (!L_in_oper_OUT_set (cb, ptr, opA->dest[0], TAKEN_PATH))
            continue;
          L_insert_op_at_dest_of_br (cb, ptr, undo_op, 1);
          redo_flow = 1;
        }

      /* delete opers */
      L_delete_oper (cb, opA);
      L_delete_oper (NULL, undo_op);

      if (redo_flow)
        L_do_flow_analysis (L_fn, LIVE_VARIABLE);

      change += 1;
    }

  return change;
}

/*===========================================================================*/
/*
 * To optimize a loop first duplicate the cb and insert duplicate in the
 * layout after the original.  Original will behave as the first iteration of
 * the loop for each invocation, and the duplicate will be executed on
 * iterations subsequent to the first when the loop back edge is traversed.
 * All back edges other than the loop back edge will be left to return to the
 * original cb and will be considered re-invocations of the loop.
 */
/*===========================================================================*/

#if 0
int
L_inductor_cleanup (L_Inner_Loop *loop)
{
  int change = 0;
  L_Cb *cb;
  L_Oper *op, *fb_op;
  L_Operand *opd;
  int i, id;
  Set exposed_use = NULL, loop_def = NULL, potential_ind = NULL;

  cb = loop->cb;
  fb_op = loop->feedback_op;

  for (op = cb->first_op; op; op = op->next_op)
    {
      if (PG_superset_predicate_ops (op, fb_op))
	{
	  /* Consider ops relevant only if they dominate the feedback
	   * op's predicate.  This is conservative -- consider side
	   * exits!
	   */

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (!(opd = op->src[i]))
		continue;
	      
	      if (!L_is_variable (opd))
		continue;
	      
	      id = L_REG_MAC_INDEX (opd);
	      
	      if (Set_in (loop_def, id))
		continue;

	      exposed_use = Set_add (exposed_use, id);
	    }

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!(opd = op->dest[i]))
		continue;

	      if (!L_is_variable (opd))
		continue;

	      id = L_REG_MAC_INDEX (opd);
	      
	      loop_def = Set_add (loop_def, id);
	    }
	}

      if (op == fb_op)
	break;
    }

  potential_ind = Set_intersect (loop_def, exposed_use);

  Set_dispose (exposed_use);
  Set_dispose (loop_def);

  if (Set_size (potential_ind))
    {
      /* Examine the feedback op to determine if it is a 
       * definite loop bound.  Again, this is conservative... consider
       * predicate newtworks and associated side exits.
       */

      /* For each potential inductor: */

      /* Find a chain of computation between loop def and exposed use.
       * This is the inductive chain, and must consist of single
       * add/sub of a loop inv value and mov/sxt/zxt ops 
       */




      /* If the inductor is suitably bounded, all sign / zero extensions
       * can be eliminated from the inductive chain and its immediate
       * descendants.  Otherwise, move as many as possible off-chain by
       * guarding descendants individually.
       */


    }

  Set_dispose (potential_ind);
  
  return change;
}
#endif
