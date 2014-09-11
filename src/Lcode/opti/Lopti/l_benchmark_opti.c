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
 *      File :          l_bench_tune.c  
 *      Description :   Benchmark-specific hacks and cheats for ispec92 
 *      Creation Date : Jan, 1995
 *      Author :        Ben Sander      
 *
 *      (C) Copyright 1990, Scott Mahlke & Pohua Chang.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

#define PRUNE_LOOP_ATTR         /* delete unroll_AMP attrs from loop copies */

#undef DEBUG_LONGWORD_LOOP      /* print annoying debug and status messages */

#define MAX_NUM_TUNE_ITERATION 5

#define MAX_IND_VAR_BRANCHES 10
#define MAX_SHORT_OPERANDS    40

/* global variables */
static int num_exit_cb, num_cb;
static int *loop_cb, *exit_cb;

typedef struct L_Cb_Map
{
  L_Cb **src;
  L_Cb **copy;
  int num_entries;
}
L_Cb_Map;

typedef struct L_Short_Info
{
  L_Operand *alias_operand[MAX_SHORT_OPERANDS];
  L_Operand *orig_operand[MAX_SHORT_OPERANDS];
  L_Oper *oper[MAX_SHORT_OPERANDS];
  int num_entries;
}
L_Short_Info;

typedef struct L_Ind_Var_Info
{
  int *var;
  int *inc;
  int *op;
  L_Operand **preheader_dest;
}
L_Ind_Var_Info;

typedef struct L_Ind_Var_Branch_Info
{
  L_Oper *branch;
  int branch_dir;
  int ind_var_index;
  int needs_remainder;
  int skip_branch;
}
L_Ind_Var_Branch_Info;


/* Prototypes */
int L_do_benchmark_tuning (L_Func * fn);
static int L_do_tott_branch_tuning (L_Func * fn);
static int L_same_cb (L_Cb * cb1, L_Cb * cb2);

/* Driver function for the benchmark hacks */
int
L_do_benchmark_tuning (L_Func * fn)
{
  int c1, opti_applied;

  /*if (Lopti_do_tott_branch_tuning) */
  if (1)
    c1 = L_do_tott_branch_tuning (fn);

#if 0
  opti_applied = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (k = 0; k < MAX_NUM_TUNE_ITERATION; k++)
        {
          int c1 = 0;


          change = c1;
          opti_applied += change;

          if (change == 0)
            break;
        }
    }
#endif
  opti_applied = c1;

  return (opti_applied);
}


/*======================================================================*/
/* The fabled EQNTOTT cheat.  Look for a pattern of:

          cb -> bne     r1, c1  cbA
         cb? -> mov     r1, c2
        cb A -> bne     r2, c1  cbB
         cb? -> mov     r2, c2
        cb B -> bne     r1, r2  cb?
        cb C -> ...     

    Insert an extra branch to get:
                beq     r1, r2  cbC

                bne     r1, c1  cbA
         cb? -> mov     r1, c2
        cb A -> bne     r2, c1  cbB
         cb? -> mov     r2, c2
        cb B -> bne     r1, r2  cb?
        cb C -> ...     
*/
/*======================================================================*/

static int
L_do_tott_branch_tuning (L_Func * fn)
{
  L_Cb *cb, *cbA, *cbB, *cbC;
  L_Oper *op1, *op2, *op3, *op4, *op5;
  L_Operand *r1, *r2;
  L_Operand *c1, *c2;
  L_Flow *flow, *flow2;
  L_Attr *new_attr;
  L_Oper *new_op1, *new_op2;
  int tott_opti_cnt = 0;


  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {

      /*
       *  Match pattern
       */

      /* 
       * First op: bne r1, c1 
       */
      op1 = cb->last_op;
      if (!(L_int_bne_branch_opcode (op1)))
        continue;

      r1 = op1->src[0];
      c1 = op1->src[1];
      if (!(L_is_register (r1) && L_is_int_constant (c1)))
        continue;

      cbA = L_find_branch_dest (op1);

      /* 
       * Second op: mov r1, c2 
       */
      op2 = L_fall_thru_path (cb)->first_op;

      if (op2 != cb->next_cb->last_op)
        continue;

      if (!(L_int_move_opcode (op2)))
        continue;

      if (!(L_same_operand (r1, op2->dest[0])))
        continue;

      c2 = op2->src[0];
      if (!(L_is_int_constant (c2)))
        continue;

      if (!(L_same_cb (cb->next_cb->next_cb, cbA)))
        continue;

      /* 
       * Third op: bne r2, c1 
       */
      op3 = cbA->first_op;
      if (op3 != cbA->last_op)
        continue;

      if (!(L_int_bne_branch_opcode (op3)))
        continue;

      r2 = op3->src[0];
      if (!(L_is_register (r2) && L_same_operand (c1, op3->src[1])))
        continue;

      cbB = L_find_branch_dest (op3);

      /* 
       * Fourth op: mov r2, c2 
       */
      op4 = cbA->next_cb->first_op;

      if (op4 != cbA->next_cb->last_op)
        continue;

      if (!(L_int_move_opcode (op2)))
        continue;

      if (!(L_same_operand (r2, op4->dest[0]) &&
            L_same_operand (c2, op4->src[0])))
        continue;


      /* 
       * Fifth op: bne r1, r2, cb? 
       */
      op5 = cbB->first_op;
      if (!(L_int_bne_branch_opcode (op5)))
        continue;

      if (!(L_is_src_operand (r1, op5) && L_is_src_operand (r2, op5)))
        continue;

#if 1
      printf ("*******************************************************\n");
      printf ("*\n*\n*\n*\n*\n*\n*\n*\n");
      printf ("* Invoking the EQNTOTT branch-tuning' cheat...        *\n");
      printf ("*\n*\n*\n*\n*\n*\n*\n*\n");
      printf ("*******************************************************\n");
#endif

      tott_opti_cnt++;

      /* 
       * Insert the extra branch
       */

      cbC = L_fall_thru_path (cbB);
#if 0
      /* The eqntott branch cheat */
      total_weight = 0.0;
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          total_weight += flow->weight;
          flow->weight *= 0.25;
        }

      total_weight *= 0.75;
      new_flow = L_new_flow (1, cb, cbC, total_weight);
      cb->dest_flow =
        L_insert_flow_before (cb->dest_flow, cb->dest_flow, new_flow);

      new_flow = L_new_flow (1, cb, cbC, total_weight);
      cbC->src_flow = L_concat_flow (cbC->src_flow, new_flow);

      new_op = L_create_new_op_using (op1->parent_op->opc, op1->parent_op);
      L_copy_compare (new_op, op1);
      L_set_compare_type (new_op, Lcmp_COM_EQ);
      new_op->src[0] = L_copy_operand (r1);
      new_op->src[1] = L_copy_operand (r2);
      new_op->src[2] = L_new_cb_operand (cbC);
      L_insert_oper_before (cb, op1, new_op);
#endif

#if 1
      new_attr = L_new_attr ("unroll_AMP", 3);
      L_set_int_attr_field (new_attr, 2, 8);
      cb->attr = L_concat_attr (cb->attr, new_attr);
#endif

      /*
       * Try to invoke the eqntott AND cheat:
       *   replace:   
       bne    r1, c1  cbA     (op1)
       cb->   mov     r1, c2          (op2)
       cb A ->        bne     r2, c1  cbB     (op3)
       cb-> mov       r2, c2          (op4)
       cb B -> bne    r1, r2  cb?     (op5)

       with:
       and    r1 <- r1 & $1
       and    r2 <- r2 & $1 
       bne    r1, r2, cb?
       */
      if ((L_is_int_constant (c1)) && (c1->value.i == 2) &&
          (L_is_int_constant (c2)) && (c2->value.i == 0))
        {

          new_op1 = L_create_new_op_using (Lop_AND, op1->parent_op);
          new_op1->dest[0] = L_copy_operand (r1);
          new_op1->src[0] = L_copy_operand (r1);
          new_op1->src[1] = L_new_gen_int_operand (1);
          L_insert_oper_before (cbB, op5, new_op1);

          new_op2 = L_create_new_op_using (Lop_AND, op3->parent_op);
          new_op2->dest[0] = L_copy_operand (r2);
          new_op2->src[0] = L_copy_operand (r2);
          new_op2->src[1] = L_new_gen_int_operand (1);
          L_insert_oper_before (cbB, op5, new_op2);

          /* Delete first branch */
          L_delete_oper (cb, op1);
          flow = L_find_flow (cb->dest_flow, 1, cb, cbA);
          cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
          flow = L_find_flow (cbA->src_flow, 1, cb, cbA);
          cbA->src_flow = L_delete_flow (cbA->src_flow, flow);

          /* Delete second branch */
          L_delete_oper (cbA, op3);
          flow = L_find_flow (cbA->dest_flow, 1, cbA, cbB);
          cbA->dest_flow = L_delete_flow (cbA->dest_flow, flow);
          flow = L_find_flow (cbB->src_flow, 1, cbA, cbB);
          cbB->src_flow = L_delete_flow (cbB->src_flow, flow);

          /* Make first cb flow to cbB */
          flow = L_find_flow (cb->dest_flow, 0, cb, cb->next_cb);
          flow->dst_cb = cbB;
          flow->weight = cb->weight;
          flow2 = L_find_flow (cbB->src_flow, 1, cbA->next_cb, cbB);
          flow2->src_cb = cb;
          flow2->cc = 0;
          flow2->weight = flow->weight;
/*
                cbB->weight = flow->weight;
*/
          L_delete_cb (fn, cbA->next_cb);
          L_delete_cb (fn, cbA);
          L_delete_cb (fn, cb->next_cb);



/*
                L_delete_oper (cb->next_cb, op2);
                L_delete_oper (cbA->next_cb, op4);
*/

        }
    }

  return (tott_opti_cnt);
}

static int
L_same_cb (L_Cb * cb1, L_Cb * cb2)
{
  if ((cb1 == NULL) && (cb2 == NULL))
    return 1;

  if ((cb1 == NULL) || (cb2 == NULL))
    return 0;

  return (cb1 == cb2);
}


/* 
 * Returns:
 *    2 if the operand is a real short operand (result of char or char2 load), 
 *    1 if the operand is the result of operation that operates on a
 *      "real" short operand (an "alias").  For example:
 *
 *      opA:    A <- LD_C ();
 *      opB:    B <- A + 7;
 *
 *    A is a "real" short operand (this function returns 2).  B is the result
 *    of an operation that uses A, so the functionreturns 1 for B.
 *    0 if not.  
 *
 * Index returns the index into short_info of the found oper
 */
static int
L_short_operand (L_Operand * operand, L_Short_Info * short_info, int *index)
{
  int i;

  for (i = 0; i < short_info->num_entries; i++)
    {
      if (L_same_operand (operand, short_info->alias_operand[i]))
        {
          *index = i;
          if (operand == short_info->orig_operand[i])
            return 2;
          else
            return 1;
        }
    }
  return 0;
}


static int
L_add_short_operand (L_Short_Info * short_info, L_Oper * op)
{
  int i = short_info->num_entries;

  short_info->orig_operand[i] = op->src[0];
  short_info->alias_operand[i] = op->dest[0];
  short_info->oper[short_info->num_entries] = op;
  short_info->num_entries++;
  if (short_info->num_entries >= MAX_SHORT_OPERANDS)
    return 1;
  else
    return 0;
}


static int
L_can_longword_convert_loop (L_Loop * loop, L_Short_Info * short_info)
{
  int i, s;
  L_Cb *cb;
  L_Oper *op;


  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {

          /* all branches can be converted */
          if (L_general_branch_opcode (op))
            continue;

          /* 
           * Other opcode can be converted if they use loop-invariant
           * operands.
           */

          switch (op->opc)
            {
            case Lop_LD_C:
            case Lop_LD_UC:
            case Lop_LD_C2:
            case Lop_LD_UC2:
            case Lop_ST_C:
            case Lop_ST_C2:
              break;
            case Lop_ADD:
            case Lop_ADD_U:
            case Lop_SUB:
            case Lop_SUB_U:
              break;
            case Lop_MOV:
              /* if loop induction or char, update alias table */
              break;

            case Lop_AND:
              if (!L_is_int_constant (op->src[1]))
                {
#ifdef DEBUG_LONGWORD_LOOP
                  fprintf (stderr, "Bad oper in char loop: ");
                  L_print_oper (stderr, op);
#endif
                  return 0;
                }
              else if (L_short_operand (op->src[0], short_info, &s))
                {
                  if (L_add_short_operand (short_info, op))
                    return 0;
                }
              break;

            case Lop_MUL:
            case Lop_LSL:
              break;

            case Lop_REM:
            case Lop_DIV:

            default:
#ifdef DEBUG_LONGWORD_LOOP
              fprintf (stderr, "Bad oper in char loop: ");
              L_print_oper (stderr, op);
#endif
              return 0;
#if 0
              for (j = 0; j < L_max_src_operand; j++)
                {
                  if (op->src[j])
                    {
                      if (L_same_operand (op->src[j], char_operand))
                        return 0;
                      if (L_basic_induction_var (loop, op->src[j]))
                        return 0;
                    }
                }
#endif
            }
        };
    }

  /* no bad instructions found, so we must be ok */
  return 1;
}

/* 
 * Save all character operation in the loop in the short_info structure.
 * Return 0 if no char operations are found in the loop.
 *
 * Assumes loop_cb and num_cb variables have already been set up.
 */
static int
L_find_character_oper (L_Loop * loop, L_Short_Info * short_info,
                       L_Operand ** index_operand)
{
  int i, size;
  L_Cb *cb;
  L_Oper *op;
  L_Operand *index;

  size = 0;
  index = NULL;

  short_info->num_entries = 0;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          switch (op->opc)
            {
            case Lop_LD_C:
            case Lop_LD_UC:
              if ((size) && (size != 4))
                return 0;
              short_info->orig_operand[short_info->num_entries] =
                short_info->alias_operand[short_info->num_entries] =
                op->dest[0];
              short_info->oper[short_info->num_entries] = op;
              short_info->num_entries++;
              if (index)
                {
                  if ((!(L_same_operand (index, op->src[0]))) &&
                      (!(L_same_operand (index, op->src[1]))))
                    {
#ifdef DEBUG_LONGWORD_LOOP
                      fprintf (stderr, "Bad index\n");
                      L_print_oper (stderr, op);
#endif
                      return 0;
                    }
                }
              else if (L_basic_induction_var (loop, op->src[0]))
                {
                  index = op->src[0];
                }
              else if (L_basic_induction_var (loop, op->src[1]))
                {
                  index = op->src[1];
                }

              if (short_info->num_entries >= MAX_SHORT_OPERANDS)
                return 0;
              size = 4;
              break;

            case Lop_LD_C2:
            case Lop_LD_UC2:
              if ((size) && (size != 2))
                return 0;
              short_info->orig_operand[short_info->num_entries] =
                short_info->alias_operand[short_info->num_entries] =
                op->dest[0];
              short_info->oper[short_info->num_entries] = op;
              short_info->num_entries++;
              if (index)
                {
                  if (!(L_same_operand (index, op->src[0])) &&
                      !(L_same_operand (index, op->src[1])))
                    {
#ifdef DEBUG_LONGWORD_LOOP
                      fprintf (stderr, "Bad index\n");
                      L_print_oper (stderr, op);
#endif
                      return 0;
                    }
                }
              else if (L_basic_induction_var (loop, op->src[0]))
                {
                  index = op->src[0];
                }
              else if (L_basic_induction_var (loop, op->src[1]))
                {
                  index = op->src[1];
                }
              if (short_info->num_entries >= MAX_SHORT_OPERANDS)
                return 0;
              size = 2;
              break;

            case Lop_ST_C:
              if ((size) && (size != 4))
                return 0;
              short_info->orig_operand[short_info->num_entries] = NULL;
              short_info->alias_operand[short_info->num_entries] = NULL;
              short_info->oper[short_info->num_entries] = op;
              short_info->num_entries++;
              if (index)
                {
                  if ((!(L_same_operand (index, op->src[0]))) &&
                      (!(L_same_operand (index, op->src[1]))))
                    {
#ifdef DEBUG_LONGWORD_LOOP
                      fprintf (stderr, "Bad index\n");
                      L_print_oper (stderr, op);
#endif
                      return 0;
                    }
                }
              else if (L_basic_induction_var (loop, op->src[0]))
                {
                  index = op->src[0];
                }
              else if (L_basic_induction_var (loop, op->src[1]))
                {
                  index = op->src[1];
                }
              if (short_info->num_entries >= MAX_SHORT_OPERANDS)
                return 0;
              size = 4;
              break;
            }
        }
    }
  *index_operand = index;
  return size;
}

/*
 * Returns 1 if the cb is in the loop, 0 if not 
 */
static int
L_cb_in_loop (L_Cb * cb, int *loop_cb, int num_cb)
{
  int i;

  if (cb == NULL)
    return 0;
  for (i = 0; i < num_cb; i++)
    {
      if (cb->id == loop_cb[i])
        {
          return 1;
        }
    }
  return 0;
}

static int
L_find_ind_var (L_Operand * ind_var, L_Ind_Var_Info * ind_var_info,
                int num_ind_var)
{
  int i;

  for (i = 0; i < num_ind_var; i++)
    {
      if (ind_var->value.i == (ITintmax) ind_var_info->var[i])
        return i;
    }

  return -1;
}

static L_Cb *
L_copy_cb (L_Cb * cb, L_Cb * exit_cb, L_Cb * insert_after_cb,
           L_Cb_Map * cb_map)
{
  L_Oper *branch_oper, *new_oper;
  L_Cb *new_cb = NULL, *new_dest_cb = NULL, *fall_thru_cb = NULL;
  L_Flow *flow;
  L_Attr *attr;
  int i;

  new_cb = L_create_cb (cb->weight);
  /*new_cb = L_create_cb (0.0); */
  L_copy_block_contents (cb, new_cb);
  new_cb->dest_flow = L_copy_flow (cb->dest_flow);
  new_cb->attr = L_copy_attr (cb->attr);
#ifdef PRUNE_LOOP_ATTR
  if ((attr = L_find_attr (new_cb->attr, "unroll_AMP")) != NULL)
    new_cb->attr = L_delete_attr (new_cb->attr, attr);

#endif
  if (insert_after_cb)
    L_insert_cb_after (L_fn, insert_after_cb, new_cb);
  else
    L_insert_cb_after (L_fn, L_fn->last_cb, new_cb);


  cb_map->src[cb_map->num_entries] = cb;
  cb_map->copy[cb_map->num_entries] = new_cb;
  cb_map->num_entries++;

  fall_thru_cb = L_fall_thru_path (cb);
  if (L_cb_in_loop (fall_thru_cb, loop_cb, num_cb))
    {
      new_dest_cb = L_copy_cb (fall_thru_cb, exit_cb, new_cb, cb_map);
    }


  /*
   * Walk the flows.  Any flow which leaves the loop is ok.  Flows which
   * stay within the loop require a duplicate copy of the dest cb.
   */
  for (flow = new_cb->dest_flow; flow != NULL; flow = flow->next_flow)
    {
      int already_duped = 0;

      /* Change the source cb */
      flow->src_cb = new_cb;

      /* Change the dest cb if it is in the loop */
      if (L_cb_in_loop (flow->dst_cb, loop_cb, num_cb))
        {
          for (i = 0; i < cb_map->num_entries; i++)
            {
              if (cb_map->src[i] == flow->dst_cb)
                {
                  already_duped = 1;
                  new_dest_cb = cb_map->copy[i];
                  break;
                }
            }
          if (already_duped == 0)
            {
              new_dest_cb = L_copy_cb (flow->dst_cb, exit_cb, new_cb, cb_map);
            }
          branch_oper = L_find_branch_for_flow (new_cb, flow);
          if (branch_oper)
            L_change_branch_dest (branch_oper, flow->dst_cb, new_dest_cb);
          else
            {
              if (new_dest_cb != new_cb->next_cb)
                {
                  new_oper = L_create_new_op (Lop_JUMP);
                  new_oper->src[0] = L_new_cb_operand (new_dest_cb);
                  L_insert_oper_after (new_cb, new_cb->last_op, new_oper);
                  flow->cc = 1;
                }
            }
          flow->dst_cb = new_dest_cb;
        }
      else
        {
          /* Flow is an exit from the loop */
          branch_oper = L_find_branch_for_flow (new_cb, flow);
          if (branch_oper == NULL)
            {
              if (new_dest_cb != new_cb->next_cb)
                {
                  new_oper = L_create_new_op (Lop_JUMP);
                  if (exit_cb == NULL)
                    new_oper->src[0] = L_new_cb_operand (flow->dst_cb);
                  else
                    {
                      new_oper->src[0] = L_new_cb_operand (exit_cb);
                      flow->dst_cb = exit_cb;
                    }
                  L_insert_oper_after (new_cb, new_cb->last_op, new_oper);
                  flow->cc = 1;
                }
            }
          else if (exit_cb)
            {
              /* If exit_cb is non-null, all exits from the loop must
                 flow to the exit_cb */
              L_change_branch_dest (branch_oper, flow->dst_cb, exit_cb);
              flow->dst_cb = exit_cb;
            }
        }

    }

  return new_cb;
}


/* 
 * Copy the loop.  Make all intra-loop branches branch to cbs inside
 * the new loop.
 */
static L_Cb *
L_copy_loop (L_Loop * loop, L_Cb * exit_cb)
{
  struct L_Cb_Map cb_map;
  L_Cb *remainder_loop, *cb;
  L_Flow *flow;
  int i;

  cb_map.src = (L_Cb **) Lcode_malloc (sizeof (L_Cb *) * num_cb);
  cb_map.copy = (L_Cb **) Lcode_malloc (sizeof (L_Cb *) * num_cb);
  cb_map.num_entries = 0;

  remainder_loop = L_copy_cb (loop->header, exit_cb, NULL, &cb_map);

  /* Reweight the flows */
  for (i = 0; i < cb_map.num_entries; i++)
    {
      cb = cb_map.copy[i];
      cb->weight = 0.0;
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          flow->weight = 0.0;
        }
    }

  Lcode_free (cb_map.src);
  Lcode_free (cb_map.copy);

  return (remainder_loop);
}


static L_Cb *
L_make_char_loop (L_Loop * loop, L_Cb * exit_cb,
                  L_Cb * insert_after, int delete_back_edge)
{
  struct L_Cb_Map cb_map;
  L_Cb *char_loop, *back_cb, *char_back_cb, *cb;
  L_Oper *char_back_branch;
  L_Flow *flow, *char_flow;
  int h, i, j, found, num_back_edge_cb = 0, *back_edge_cb = NULL;

  cb_map.src = (L_Cb **) Lcode_malloc (sizeof (L_Cb *) * num_cb);
  cb_map.copy = (L_Cb **) Lcode_malloc (sizeof (L_Cb *) * num_cb);
  cb_map.num_entries = 0;


  /*
   * Make a copy of the current loop
   */
  char_loop = L_copy_cb (loop->header, exit_cb, insert_after, &cb_map);


  if (delete_back_edge)
    {
      /*
       * Find the back edge cb.  Delete the back_edge flow and branch 
       * in the char_loop.
       */

      num_back_edge_cb = Set_size (loop->back_edge_cb);
      if (num_back_edge_cb > 0)
        {
          back_edge_cb =
            (int *) Lcode_malloc (sizeof (int) * num_back_edge_cb);
          Set_2array (loop->back_edge_cb, back_edge_cb);
        }

      found = 0;
      for (h = 0; h < num_back_edge_cb; h++)
        {
          back_cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, back_edge_cb[h]);
          for (i = 0; i < cb_map.num_entries; i++)
            {
              if (cb_map.src[i] == back_cb)
                {
                  char_flow = NULL;
                  char_back_cb = cb_map.copy[i];
                  for (j = 0; j < cb_map.num_entries; j++)
                    {
                      if (cb_map.src[j] == loop->header)
                        {
                          char_flow =
                            L_find_flow_with_dst_cb (char_back_cb->dest_flow,
                                                     cb_map.copy[j]);
                          break;
                        }
                    }
                  char_back_branch =
                    L_find_branch_for_flow (char_back_cb, char_flow);
                  char_back_cb->dest_flow =
                    L_delete_flow (char_back_cb->dest_flow, char_flow);
                  L_delete_oper (char_back_cb, char_back_branch);
                  found = 1;
                  break;
                }
            }
        }
      /* This may be ok since the char loop may not contain the backedge
         branch... */
      /*      if (found = 0)        JEM     6/31/95 */
      if (found == 0)
        L_punt ("L_make_char_loop: Couldn't delete backedge branch.");
    }


  /* 
   * Zero the flow information.
   */

  for (i = 0; i < cb_map.num_entries; i++)
    {
      cb = cb_map.copy[i];
      cb->weight = 0.0;
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          flow->weight = 0.0;
        }
    }

  Lcode_free (cb_map.src);
  Lcode_free (cb_map.copy);

  return (char_loop);
}

static L_Oper *
L_find_reaching_def (L_Operand * src, L_Oper * use_op)
{
  L_Oper *oper;
  int pred1, pred2, i;

  if (use_op->pred[0])
    pred1 = use_op->pred[0]->value.r;
  else
    pred1 = 0;

  for (oper = use_op->prev_op; oper != NULL; oper = oper->prev_op)
    {

      if (oper->pred[0])
        pred2 = oper->pred[0]->value.r;
      else
        pred2 = 0;

      if ((pred2 != 0) && (pred1 != pred2))
        continue;

      for (i = 0; i < L_max_dest_operand; i++)
        {
          if (L_same_operand (oper->dest[i], src))
            return (oper);
        }
    }
  return (NULL);
}

/*
 * Find the first operation in the cb which defines data needed 
 * by oper. 
*/
L_Oper *
L_find_first_reaching_oper (L_Cb * cb, L_Oper * oper)
{
  L_Oper *def, *first_def, *opA;
  int i;

  first_def = NULL;
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!(L_is_variable (oper->src[i])))
        continue;
      def = L_find_reaching_def (oper->src[i], oper);
      if (def != NULL)
        {
          for (opA = def; opA != NULL; opA = opA->prev_op)
            {
              if (opA == first_def)
                break;
            }
          if (opA == NULL)
            {
              /* def precedes exisiting first_def */
              first_def = def;
            }
        }
    }
  return first_def;
}


static void
L_mark_needed_opers (L_Cb * cb, L_Oper * oper, L_Short_Info * short_info)
{
  L_Oper *def;
  L_Attr *new_attr;
  int i, s;

  if (L_find_attr (oper->attr, "needed"))
    return;

  new_attr = L_new_attr ("needed", 0);
  oper->attr = L_concat_attr (oper->attr, new_attr);

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!(L_is_variable (oper->src[i])))
        continue;
      if (L_short_operand (oper->src[i], short_info, &s))
        {
          /*
           * The oper uses a short operand.  Find the oper which
           * defines the short operand, and find its reaching defs */
          def = L_find_reaching_def (oper->src[i], oper);
          if (def)
            L_mark_needed_opers (cb, def, short_info);

        }
    }
}


/* 
void L_trim_char_loops(L_Cb* char_loop1, L_Cb* char_loop2, L_Cb* char_loop3, 
                       L_Cb* char_loop4, L_Loop* loop)
{
}
 */

void
L_trim_char2_loops (L_Cb * char_loop1, L_Cb * char_loop2)
{
  L_Oper *new_op, *oper, *next_op;
  L_Operand *new_reg, *br_src0, *br_src1;
#define         CHAR1_MASK      65535
#define         CHAR2_MASK      4294901800U

  /* check parameters */
  if ((char_loop1 == NULL) || (char_loop2 == NULL))
    L_punt ("L_trim_char2_loops:  NULL parameter");


  /* trim char_loop1 */
  /* delete all opers except the branch */
  for (oper = char_loop1->first_op; oper != NULL; oper = next_op)
    {
      next_op = oper->next_op;
      if (!L_cond_branch_opcode (oper))
        L_delete_oper (char_loop1, oper);
      else
        break;
    }

  if (oper == NULL)
    L_punt ("L_trim_char2_loops:  no conditional branch");

  br_src0 = oper->src[0];
  br_src1 = oper->src[1];

  /* mask off the appropriate bits of the branch opers and update the 
     branch opers */
  new_op = L_create_new_op (Lop_AND);
  new_reg = L_new_register_operand (++(L_fn->max_reg_id),
                                    L_native_machine_ctype, L_PTYPE_NULL);
  new_op->dest[0] = new_reg;
  new_op->src[0] = L_copy_operand (br_src0);
  new_op->src[1] = L_new_gen_int_operand (CHAR1_MASK);
  L_insert_oper_before (char_loop1, oper, new_op);
  oper->src[0] = L_copy_operand (new_op->dest[0]);

  new_op = L_create_new_op (Lop_AND);
  new_reg = L_new_register_operand (++(L_fn->max_reg_id),
                                    L_native_machine_ctype, L_PTYPE_NULL);
  new_op->dest[0] = new_reg;
  new_op->src[0] = L_copy_operand (br_src1);
  new_op->src[1] = L_new_gen_int_operand (CHAR1_MASK);
  L_insert_oper_before (char_loop1, oper, new_op);
  oper->src[1] = L_copy_operand (new_op->dest[0]);

  /* trim char_loop2 */
  /* delete all opers except the branch */
  for (oper = char_loop2->first_op; oper != NULL; oper = next_op)
    {
      next_op = oper->next_op;
      if (!L_cond_branch_opcode (oper))
        L_delete_oper (char_loop2, oper);
      else
        break;
    }

  if (oper == NULL)
    L_punt ("L_trim_char2_loops:  no branch");


  /* mask off the appropriate bits of the branch opers */
  new_op = L_create_new_op (Lop_AND);
  new_reg = L_new_register_operand (++(L_fn->max_reg_id),
                                    L_native_machine_ctype, L_PTYPE_NULL);
  new_op->dest[0] = new_reg;
  new_op->src[0] = L_copy_operand (br_src0);
  new_op->src[1] = L_new_gen_int_operand (CHAR2_MASK);
  L_insert_oper_before (char_loop2, oper, new_op);
  oper->src[0] = L_copy_operand (new_op->dest[0]);

  new_op = L_create_new_op (Lop_AND);
  new_reg = L_new_register_operand (++(L_fn->max_reg_id),
                                    L_native_machine_ctype, L_PTYPE_NULL);
  new_op->dest[0] = new_reg;
  new_op->src[0] = L_copy_operand (br_src1);
  new_op->src[1] = L_new_gen_int_operand (CHAR2_MASK);
  L_insert_oper_before (char_loop2, oper, new_op);
  oper->src[1] = L_copy_operand (new_op->dest[0]);

  L_delete_operand (br_src0);
  L_delete_operand (br_src1);
}


/*
 * Expand the exit so that it figures out which iteration caused the
 * loop exit.

 * If induction variables are live out of the loop, then we should
 * make sure they are set correctly for the iteration.
 * If short_oper variables are live out of the loop, then make
 * sure they are set correctly.
 * For now, make sure everything is correct.
 */
static void
L_expand_longword_exit (L_Loop * loop, L_Cb * cb, L_Oper * branch,
                        L_Short_Info * short_info, int size)
{

  L_Oper *oper;
  L_Cb *char_loop1, *char_loop2, *char_loop3, *char_loop4, *last_cb;
  L_Flow *flow, *short_flow;

#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr, "Expand longword exit for:\n    ");
  L_print_oper (stderr, branch);
#endif

  /* 
   * Copy the loop body to the branch exit - basically, we will re-run
   * the loop to figure out which iteration caused the loop exit.
   */
  last_cb = L_fn->last_cb;
  flow = L_find_flow_for_branch (cb, branch);
  if (size == 4)
    {
      char_loop4 = L_make_char_loop (loop, flow->dst_cb, last_cb, 1);
      char_loop3 = L_make_char_loop (loop, char_loop4, last_cb, 1);
      char_loop2 = L_make_char_loop (loop, char_loop3, last_cb, 1);
      char_loop1 = L_make_char_loop (loop, char_loop2, last_cb, 1);
    }
  else
    {
      char_loop2 = L_make_char_loop (loop, flow->dst_cb, last_cb, 1);
      char_loop1 = L_make_char_loop (loop, char_loop2, last_cb, 1);
      for (oper = char_loop1->first_op; oper != NULL; oper = oper->next_op)
        {
          if ((oper->opc == branch->opc) &&
              (L_same_operand (oper->src[0], branch->src[0])) &&
              (L_same_operand (oper->src[1], branch->src[1])))
            {

              short_flow = L_find_flow_for_branch (char_loop1, oper);
              short_flow->dst_cb = flow->dst_cb;
              oper->src[2]->value.cb = flow->dst_cb;
            }
        }
    }


  /* get rid of duplicate code */

/*
    if(size==4)
        L_trim_char_loops(char_loop1, char_loop2, char_loop3, char_loop4);
    else if (size==2)
        L_trim_char2_loops(char_loop1, char_loop2); 
 */

  if (size == 2)
    L_trim_char2_loops (char_loop1, char_loop2);



  /* 
   * Make the exit flow to the new code
   */
  L_change_branch_dest (branch, flow->dst_cb, char_loop1);
  flow->dst_cb = char_loop1;


  L_mark_needed_opers (cb, branch, short_info);

#if 0
  if (size == 4)
    {
      for (oper = cb->first_op; (oper != start_oper) && (oper != NULL);
           oper = oper->next_op)
        {

          /* This doesn't handle flows... */
          L_delete_oper (char_loop1, char_loop1->first_op);
          L_delete_oper (char_loop2, char_loop2->first_op);
          L_delete_oper (char_loop3, char_loop3->first_op);
          L_delete_oper (char_loop4, char_loop4->first_op);
        }
    }
  else
    {
      op1 = char_loop1->first_op;
      op2 = char_loop2->first_op;
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {

          if (!(L_find_attr (oper->attr, "needed")))
            {
              temp_op = op1;
              op1 = op1->next_op;
              L_delete_oper (char_loop1, temp_op);
              temp_op = op2;
              op2 = op2->next_op;
              L_delete_oper (char_loop2, temp_op);
            }
          else
            {
              op1 = op1->next_op;
              op2 = op2->next_op;
            }

        }
    }
#endif
#ifdef DEBUG_LONGWORD_LOOP
  L_print_cb (stderr, NULL, char_loop1);
  L_print_cb (stderr, NULL, char_loop2);
#endif
}

/* 
 * Expand the branch so that it can handle 4 character iterations or
 * 2 char2 iterations. 
 */
static void
L_expand_longword_branch (L_Loop * loop, L_Cb * cb, L_Oper * branch,
                          int size, L_Operand * index_operand)
{
  L_Oper *oper, *start_oper;
  L_Cb *char_loop1 = NULL, *char_loop2 = NULL, *char_loop3 = NULL,
    *char_loop4 = NULL, *last_cb = NULL;
  L_Flow *flow;


#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr, "Expand longword branch for:\n    ");
  L_print_oper (stderr, branch);
#endif

  /*
   * Copy the CBs on the taken branch path so that we have
   * four "unrolled" character iterations. 
   */

  last_cb = L_fn->last_cb;
  if (size == 4)
    {
      char_loop4 = L_make_char_loop (loop, loop->header, last_cb, 0);
      char_loop3 = L_make_char_loop (loop, char_loop4, last_cb, 1);
      char_loop2 = L_make_char_loop (loop, char_loop3, last_cb, 1);
      char_loop1 = L_make_char_loop (loop, char_loop2, last_cb, 1);
    }
  else
    {
      char_loop2 = L_make_char_loop (loop, loop->header, last_cb, 1);
      char_loop1 = L_make_char_loop (loop, char_loop2, last_cb, 0);
    }

  flow = L_find_flow_for_branch (cb, branch);
  L_change_branch_dest (branch, flow->dst_cb, char_loop1);
  flow->dst_cb = char_loop1;

  /*
   * Find operations in the longword loop which execute before the
   * branch to the character loop and which are not needed by the
   * the branch.  Delete the corresponding opers in the character loop. 
   */

#if 0
  if (size == 4)
    {
      op1 = char_loop1->first_op;
      op2 = char_loop2->first_op;
      op3 = char_loop3->first_op;
      op4 = char_loop4->first_op;
      for (oper = cb->first_op; oper != branch; oper = oper->next_op)
        {
          if (L_basic_induction_var (loop, oper->dest[0]))
            {
              if (L_same_operand (oper->dest[0], index_operand))
                {
                  /* Index incremented in loop before loads... need
                     to correct by subtracting the loop increment.
                     No, this is not very efficient.  Might be better
                     for x86 to merge the add into the addressing
                     mode, so we get something like:

                     i <- i + 4;
                     bne  c[i], lab ------>    bne c[i-4]
                     bne c[i-3]
                     bne c[i-2]
                     bne c[i-1] */
                  L_punt ("Longword: index var incremented before load");
                }
              else
                {
                  temp_op = op1;
                  op1 = op1->next_op;
                  L_delete_oper (char_loop1, temp_op);
                  temp_op = op2;
                  op2 = op2->next_op;
                  L_delete_oper (char_loop2, temp_op);
                  temp_op = op3;
                  op3 = op3->next_op;
                  L_delete_oper (char_loop3, temp_op);
                  temp_op = op4;
                  op4 = op4->next_op;
                  L_delete_oper (char_loop4, temp_op);
                }
            }
          else
            {
              op1 = op1->next_op;
              op2 = op2->next_op;
              op3 = op3->next_op;
              op4 = op4->next_op;
            }
        }
    }
  else
    {
      for (oper = cb->first_op; (oper != start_oper) && (oper != NULL);
           oper = oper->next_op)
        {
          L_delete_oper (char_loop1, char_loop1->first_op);
          L_delete_oper (char_loop2, char_loop2->first_op);
        }
    }
#endif

#if 1
  start_oper = L_find_first_reaching_oper (cb, branch);
#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr, "First reaching oper:");
  L_print_oper (stderr, start_oper);
#endif

  if (size == 4)
    {
      for (oper = cb->first_op; (oper != start_oper) && (oper != NULL);
           oper = oper->next_op)
        {
          /* This doesn't handle flows... */
          L_delete_oper (char_loop1, char_loop1->first_op);
          L_delete_oper (char_loop2, char_loop2->first_op);
          L_delete_oper (char_loop3, char_loop3->first_op);
          L_delete_oper (char_loop4, char_loop4->first_op);
        }
    }
  else
    {
      for (oper = cb->first_op; (oper != start_oper) && (oper != NULL);
           oper = oper->next_op)
        {
          /* This doesn't handle flows... */
          L_delete_oper (char_loop1, char_loop1->first_op);
          L_delete_oper (char_loop2, char_loop2->first_op);
        }
    }
#endif
}


/* 
 * Attempt to change loops which process char variables into
 * loops which process longs, theoretically executing four times
 * as fast.  
 */
int
L_do_longword_loop_conversion (L_Loop * loop)
{
  int i, j;
  int need_align_loop, need_remainder_loop;
  int num_back_edge_cb, *back_edge_cb;
  int num_ind_var_branches;
  L_Cb *cb, *remainder_head = NULL;
  L_Oper *op, *new_oper, *branch_oper, *ind_branch;
  L_Short_Info short_info;
  int s;
  int num_ind_var, num_ind_var_ops;
  L_Ind_Var_Branch_Info branch_info[MAX_IND_VAR_BRANCHES];
  L_Ind_Var_Info ind_var_info;

  L_Cb *header, *preheader;
  L_Attr *new_attr;
  L_Flow *flow;
  double iterations;
  int size;
  L_Operand *index_operand;


  header = loop->header;
  preheader = loop->preheader;
  if (loop->num_invocation)
    iterations = header->weight / loop->num_invocation;
  else
    iterations = 0.0;

  if (iterations < 8)
    return 0;

  if (L_find_attr (header->attr, "longword"))
    return 0;

  if (header->weight == 0.0)
    return 0;


#if 0
  if (header->id != 14)
    return 0;
#endif



  /* build arrays for sets of info about the loop */
  num_ind_var = Set_size (loop->basic_ind_var);
  num_ind_var_ops = Set_size (loop->basic_ind_var_op);

  if (num_ind_var > 0)
    {
      ind_var_info.var = (int *) Lcode_malloc (sizeof (int) * num_ind_var);
      ind_var_info.op = (int *) Lcode_malloc (sizeof (int) * num_ind_var_ops);
      ind_var_info.inc = (int *) Lcode_calloc (num_ind_var, sizeof (int));
      ind_var_info.preheader_dest =
        (L_Operand **) Lcode_calloc (num_ind_var, sizeof (L_Operand *));
      Set_2array (loop->basic_ind_var, ind_var_info.var);
      Set_2array (loop->basic_ind_var_op, ind_var_info.op);

      /*
       * Find the amount each induction variable is incremented/decremented
       * while in the loop.
       */
      for (i = 0; i < num_ind_var_ops; i++)
        {
          int increment;

          if (!(op = L_oper_hash_tbl_find_oper (L_fn->oper_hash_tbl,
						ind_var_info.op[i])))
            continue;

          j = L_find_ind_var (op->dest[0], &ind_var_info, num_ind_var);
          if (j == -1)
            {
#ifdef DEBUG_LONGWORD_LOOP
              fprintf (stderr,
                       "Longword: Can't find induction var for oper.\n");
#endif
              return 0;
            }

          if (L_int_add_opcode (op) && (L_is_int_constant (op->src[1])))
	    {
	      increment = (int) op->src[1]->value.i;
	    }
          else if (L_int_add_opcode (op) && (L_is_int_constant (op->src[1])))
	    {
	      increment = -(int) op->src[1]->value.i;
	    }
          else
            {
#ifdef DEBUG_LONGWORD_LOOP
              fprintf (stderr, "Longword: bad ind var operation\n");
              L_print_oper (stderr, op);
#endif
              return 0;
            }

          if ((ind_var_info.inc[j]) && (ind_var_info.inc[j] != increment))
	    L_punt ("Longword:Different increments of same induction var.");
          else
            ind_var_info.inc[j] = increment;
        }
    }

  num_exit_cb = Set_size (loop->exit_cb);
  if (num_exit_cb > 0)
    {
      exit_cb = (int *) Lcode_malloc (sizeof (int) * num_exit_cb);
      Set_2array (loop->exit_cb, exit_cb);
    }

  num_cb = Set_size (loop->loop_cb);
  if (num_cb > 0)
    {
      loop_cb = (int *) Lcode_malloc (sizeof (int) * num_cb);
      Set_2array (loop->loop_cb, loop_cb);
    }

  num_back_edge_cb = Set_size (loop->back_edge_cb);
  if (num_back_edge_cb > 0)
    {
      back_edge_cb = (int *) Lcode_malloc (sizeof (int) * num_back_edge_cb);
      Set_2array (loop->back_edge_cb, back_edge_cb);
    }


  /* Profile-based heuristic */

  if (!(size = L_find_character_oper (loop, &short_info, &index_operand)))
    return 0;

#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr,
           " Loop with short load: header cb=%i "
           "(wt: %7.0f, num_invok: %4.0f, iters: %4.0f)\n",
           header->id, header->weight, loop->num_invocation, iterations);
#endif

  if (!(L_can_longword_convert_loop (loop, &short_info)))
    return 0;

#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr,
           "Can convert loop with header cb=%i "
           "(wt: %7.0f, num_invok: %4.0f, iters: %4.0f)\n",
           header->id, header->weight, loop->num_invocation, iterations);
  Set_print (stderr, "Induction Var:", loop->basic_ind_var);
#endif

  /* 
   * Make sure the index variable has a positive increment of +1.  We don't 
   currently support the downward index operand since the longword access 
   is more difficult.  If we have: 
   r = c[i];
   i = i -1; 

   The for a longword loop we need to access:
   r = c[i-4];
   i = i - 4;

   The [i-4] index is more complicated than the [i] index used if
   the index has a positive increment.

   So don't handle it now and exit if the loop requires it.
   */

#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr, "Index operand = ");
  L_print_operand (stderr, index_operand, 0);
  fprintf (stderr, "\n");
#endif

  need_align_loop = 1;
  if (index_operand)
    {
      for (i = 0; i < num_ind_var; i++)
        {
          if (index_operand->value.r == ind_var_info.var[i])
            {
              if (ind_var_info.inc[i] != 1)
                {
#ifdef DEBUG_LONGWORD_LOOP
                  fprintf (stderr, "longword: Index inc must be one.\n");
#endif
                  return 0;
                }
              else
                {
                  break;
                }
            }
        }
      if (i == num_ind_var)
        L_punt ("L_do_longword: index_operand not found as induction var.");
    }

  if (!index_operand)
    need_align_loop = 0;        /* Since we don't know what the index_operand
                                   is, we can't make sure it's aligned. */

  /* 
   * Check to make sure all back_edge branches in the loop
   * are bgt/bge/blt/ble branches - no beq or bne.  Add
   * all the exit branches to an array so we can change
   * them later.
   */
  need_remainder_loop = 0;
  num_ind_var_branches = 0;
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
      for (flow = cb->dest_flow; flow; flow = flow->next_flow)
        {
          branch_oper = L_find_branch_for_flow (cb, flow);
#ifdef DEBUG_LONGWORD_LOOP
          L_print_oper (stderr, branch_oper);
#endif
          if (!branch_oper)
            {
              /* fall-thru flow */
            }
          else if (L_uncond_branch_opcode (branch_oper))
            {

            }
          else if (L_short_operand (branch_oper->src[0], &short_info, &s) ||
                   L_short_operand (branch_oper->src[1], &short_info, &s))
            {

              if ((header->weight > 0.0) &&
                  (flow->weight / header->weight > 0.15))
                {
#ifdef DEBUG_LONGWORD_LOOP
                  fprintf (stderr, "flow: (%d %d %f) header: %f",
                           flow->src_cb->id, flow->dst_cb->id,
                           flow->weight, header->weight);
                  fprintf (stderr,
                           " Branch exits loop too often. Longword exiting.\n"
                           );
#endif
                  return 0;
                }


            }
          else
            {
              if (L_basic_induction_var (loop, branch_oper->src[1]))
                L_punt ("L_do_longword: branch with src1 as induction var");
              if (!L_basic_induction_var (loop, branch_oper->src[0]))
                {
#ifdef DEBUG_LONGWORD_LOOP
                  fprintf (stderr, "Bad branch - no induction var.\n");
                  L_print_oper (stderr, branch_oper);
#endif
                  return 0;
                }
              if (num_ind_var_branches >= MAX_IND_VAR_BRANCHES)
                {
#ifdef DEBUG_LONGWORD_LOOP
                  fprintf (stderr, "Too many branches for longword loop");
#endif
                  return 0;
                }

              if (L_int_blt_branch_opcode (branch_oper) ||
                  L_int_ble_branch_opcode (branch_oper))
                {
                  branch_info[num_ind_var_branches].branch_dir = +1;
                }
              else if (L_int_bgt_branch_opcode (branch_oper) ||
                       L_int_bge_branch_opcode (branch_oper))
                {
                  branch_info[num_ind_var_branches].branch_dir = -1;
                }
              else
                {
#ifdef DEBUG_LONGWORD_LOOP
                  fprintf (stderr, "Bad branch - bne/beq.\n");
                  L_print_oper (stderr, branch_oper);
#endif
                  return 0;
                }

              if (flow->dst_cb != header)
                {
                  /* Not a backedge branch - assume it is skipping
                     a backedge branch */
                  /* flip polarity */
                  branch_info[num_ind_var_branches].branch_dir *= -1;
                  branch_info[num_ind_var_branches].skip_branch = 1;
                }
              else
                branch_info[num_ind_var_branches].skip_branch = 0;

              if ((L_is_int_constant (branch_oper->src[1])) &&
                  ((branch_oper->src[1]->value.i & 3) == 0) && (0))
                {

                  branch_info[num_ind_var_branches].needs_remainder = 0;
                }
              else
                {
                  need_remainder_loop = 1;
                  branch_info[num_ind_var_branches].needs_remainder = 1;
                }

              branch_info[num_ind_var_branches].branch = branch_oper;
              j = L_find_ind_var (branch_oper->src[0], &ind_var_info,
                                  num_ind_var);
              if (j != -1)
                branch_info[num_ind_var_branches].ind_var_index = j;
              else
                L_punt ("Longword: No info found for induction variable.");
              num_ind_var_branches++;
            }
        }
    }

  /* 
   * We always need a remainder loop...
   */
  need_remainder_loop = 1;

  /* 
   * Figure out if we need an alignment loop before the 
   * longword loop and if we need a remainder loop after
   * the longword loop. 

   * Look at all the branches in the loop.  If they are based on induction
   * vars, then make sure they are bge/bg/bl/ble - not beq, bne.  Also,
   * add them to the branch table. 
   */

#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr, "Preheader cb=%i", preheader->id);
  L_print_cb (stderr, NULL, preheader);
  fprintf (stderr, "Longloop with header cb=%i (%4.0f)\n",
           header->id, header->weight);
  L_print_cb (stderr, NULL, header);
#endif


  /* Remove all src flows.  Will fix up all the dest flows and then rebuild
     all the src flows. */
  L_clear_src_flow (L_fn);

  new_attr = L_new_attr ("longword", 0);
  header->attr = L_concat_attr (header->attr, new_attr);

  /* 
   * Make an alignment loop at the beginning to make sure all
   * vars used for exit conditions are aligned before we enter. 
   */
  if (need_align_loop)
    {
      need_remainder_loop = 1;
      /*align_head = L_copy_loop (loop, header); */
    }

  /* 
   *  Make a "remainder loop" to handle odd characters at the end that
   *  cannot be handled with longwords - for example, if the
   *  loop is executed 43 times, the first loop will do 10 longword
   *  iterations and the remainder loop will handle the final three 
   *  with character operations.
   */
  if (need_remainder_loop)
    remainder_head = L_copy_loop (loop, NULL);


  if (need_align_loop)
    {
      L_Oper *new_branch_oper;
      /*
       * Add a branch to the header to make sure the index variable
       * for the short loads is aligned before we fall into the longword
       * loop.  For now, skip to the remainder loop if the index is
       * not aligned.
       */

      new_oper = L_create_new_op (Lop_AND);
      new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                  L_native_machine_ctype,
                                                  L_PTYPE_NULL);
      new_oper->src[0] = L_copy_operand (index_operand);
      if (size == 4)
        new_oper->src[1] = L_new_gen_int_operand (3);
      else
        new_oper->src[1] = L_new_gen_int_operand (1);
      L_insert_oper_after (preheader, preheader->last_op, new_oper);

      new_branch_oper = L_create_new_op (Lop_BR);
      new_branch_oper->src[0] = L_copy_operand (new_oper->dest[0]);
      new_branch_oper->src[1] = L_new_gen_int_operand (0);
      L_set_compare (new_branch_oper, new_branch_oper->src[0]->ctype,
                     Lcmp_COM_NE);
      new_branch_oper->src[2] = L_new_cb_operand (remainder_head);
      L_insert_oper_after (preheader, new_oper, new_branch_oper);
      flow = L_new_flow (1, preheader, remainder_head, 0.0);
      preheader->dest_flow = L_concat_flow (flow, preheader->dest_flow);
    }


  /*
   * Add branches to the preheader so that we are sure there are
   * at least size iterations remaining in the loop before we enter
   * the longword version.  Others should branch to the remainder 
   * loop.
   */
  for (i = 0; i < num_ind_var_branches; i++)
    {
      int index;
      L_Oper *new_branch_oper;

      branch_oper = branch_info[i].branch;
      index = branch_info[i].ind_var_index;

      /*
       * Check to see if we have already added a check for a branch
       * that is identical to this one. 
       */
      for (j = 0; j < i; j++)
        {
          if (L_same_operation
              (branch_oper, branch_info[j].branch, IGNORE_FS)) break;
        }
      if (j != i)
        continue;

      if (L_is_int_constant (branch_oper->src[1]))
        {
          new_oper = L_create_new_op (Lop_BR);
          L_copy_compare (new_oper, branch_oper);
          if (!branch_info[i].skip_branch)
            L_negate_compare (new_oper);
          new_oper->src[0] = L_copy_operand (branch_oper->src[0]);
          new_oper->src[1] = L_copy_operand (branch_oper->src[1]);
          new_oper->src[2] = L_new_cb_operand (remainder_head);
          new_oper->src[1]->value.i -=
            (ITintmax) (ind_var_info.inc[index] * size);
          L_insert_oper_after (preheader, preheader->last_op, new_oper);
          flow = L_new_flow (1, preheader, remainder_head, 0.0);
          preheader->dest_flow = L_concat_flow (flow, preheader->dest_flow);
        }
      else
        {
          new_oper = L_create_new_op (Lop_ADD);
          new_oper->dest[0] = L_new_register_operand (++L_fn->max_reg_id,
                                                      L_native_machine_ctype,
                                                      L_PTYPE_NULL);
          new_oper->src[0] = L_copy_operand (branch_oper->src[1]);
          new_oper->src[1] = L_new_gen_int_operand
            (-ind_var_info.inc[index] * size);

          L_insert_oper_after (preheader, preheader->last_op, new_oper);

          new_branch_oper = L_create_new_op (branch_oper->opc);
          L_copy_compare (new_branch_oper, branch_oper);
          if (!branch_info[i].skip_branch)
            L_negate_compare (new_branch_oper);
          new_branch_oper->src[0] = L_copy_operand (branch_oper->src[0]);
          new_branch_oper->src[1] = L_copy_operand (new_oper->dest[0]);
          new_branch_oper->src[2] = L_new_cb_operand (remainder_head);
          L_insert_oper_after (preheader, new_oper, new_branch_oper);
          flow = L_new_flow (1, preheader, remainder_head, 0.0);
          preheader->dest_flow = L_concat_flow (flow, preheader->dest_flow);
        }
    }


  /*
   * Walk the exit flows
   *     1) If the flow is based on loop iteration, change it to flow to the
   *        remainder loop.
   *     2) If the flow is based on the character iteration, leave it
   *           alone.
   *     3) Change the fallthru path of the loop to flow to the remainder
   *        loop.
   */
  for (i = 0; i < num_exit_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, exit_cb[i]);
#ifdef DEBUG_LONGWORD_LOOP
      L_print_cb (stderr, NULL, cb);
#endif
      for (flow = cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          int ind_var;

          if (L_cb_in_loop (flow->dst_cb, loop_cb, num_cb))
            continue;

          branch_oper = L_find_branch_for_flow (cb, flow);

          /* 
           * Determine if the flow should go to the remainder loop.
           * If the branch is based on a constant that is divisible
           * by 4, then we can finish all iterations in the original
           * loop and the remainder loop isn't needed.  This applies 
           * to fall-thru paths in which the last branch in the loop
           * is a compare to a constant divis by 4.  We have already
           * tagged these branches in the ind_var_need_remainder array
           * so we know which branches need remainders.  
           */
          if (branch_oper == NULL)
            {
              ind_branch = L_find_last_branch (cb);
            }
          else if (L_uncond_branch_opcode (branch_oper))
            {
              for (ind_branch = branch_oper->prev_op; ind_branch != NULL;
                   ind_branch = ind_branch->prev_op)
                {

                  if (L_general_branch_opcode (ind_branch))
                    break;
                }
              if (ind_branch == NULL)
                L_punt ("L_do_longword: exit branch not induction.");
            }
          else
            {
              ind_branch = branch_oper;
            }

          for (ind_var = 0; ind_var < num_ind_var_branches; ind_var++)
            if (branch_info[ind_var].branch == ind_branch)
              {
                break;
              }

          if (ind_var == num_ind_var_branches)
            {
              /* Exit branch is not induction based. */
              if ((branch_oper) &&
                  (!L_uncond_branch_opcode (ind_branch)) &&
                  (L_short_operand (branch_oper->src[0], &short_info, &s) ||
                   L_short_operand (branch_oper->src[1], &short_info, &s)))
                {

                  L_expand_longword_exit (loop, cb, branch_oper,
                                          &short_info, size);
                  continue;
                }
              else
                {
                  L_print_oper (stderr, branch_oper);
                  L_print_oper (stderr, ind_branch);
                  L_punt ("L_do_longword: Bad exit branch.");
                }
            }

          if (!branch_info[ind_var].needs_remainder)
            continue;

          if (L_uncond_branch_opcode (branch_oper))
            {
              if (!L_cb_in_loop (flow->dst_cb, loop_cb, num_cb))
                {
                  L_change_branch_dest (branch_oper, flow->dst_cb,
                                        remainder_head);
                  flow->dst_cb = remainder_head;
                }
            }
          else if (branch_oper)
            {
              if ((!L_cb_in_loop (flow->dst_cb, loop_cb, num_cb)) &&
                  (L_basic_induction_var (loop, branch_oper->src[0]) ||
                   L_basic_induction_var (loop, branch_oper->src[1])))
                {
                  L_change_branch_dest (branch_oper, flow->dst_cb,
                                        remainder_head);
                  flow->dst_cb = remainder_head;
                }
            }
          else if (need_remainder_loop)
            {
              if (!(L_cb_in_loop (flow->dst_cb, loop_cb, num_cb)))
                {
                  /* Fall-thru and need remainder loop */
                  flow->dst_cb = remainder_head;
                  new_oper = L_create_new_op (Lop_JUMP);
                  new_oper->src[0] = L_new_cb_operand (remainder_head);
                  L_insert_oper_after (cb, cb->last_op, new_oper);
                }
            }
        }
    }

  /*
   * Fix induction var branches so that the remainder loop is reached 
   * This involves changing them so that they exit when 4 or fewer iterations 
   * remain, which may require adding code to the loop preheader. 
   */
  if (need_remainder_loop)
    {
      for (i = 0; i < num_ind_var_branches; i++)
        {
          int index = branch_info[i].ind_var_index;
          branch_oper = branch_info[i].branch;

          if (branch_info[i].branch_dir == +1)
            {

              if (L_is_int_constant (branch_oper->src[1]))
                {
                  branch_oper->src[1]->value.i -=
                    (ITintmax) (ind_var_info.inc[index] * size);
                }
              else if (ind_var_info.preheader_dest[index])
                {
                  L_delete_operand (branch_oper->src[1]);
                  branch_oper->src[1] =
                    L_copy_operand (ind_var_info.preheader_dest[index]);
                }
              else
                {
                  new_oper = L_create_new_op (Lop_SUB);
                  new_oper->dest[0] =
                    L_new_register_operand (++L_fn->max_reg_id,
                                            L_native_machine_ctype,
                                            L_PTYPE_NULL);
                  new_oper->src[0] = branch_oper->src[1];
                  new_oper->src[1] =
                    L_new_gen_int_operand (ind_var_info.inc[index] * size);
                  branch_oper->src[1] = L_copy_operand (new_oper->dest[0]);
                  L_insert_oper_after (preheader, preheader->last_op,
                                       new_oper);
                  ind_var_info.preheader_dest[index] = new_oper->dest[0];
                }
            }
          else if (branch_info[i].branch_dir == -1)
            {
              if (L_is_int_constant (branch_oper->src[1]))
                {
                  branch_oper->src[1]->value.i +=
                    (ITintmax) (ind_var_info.inc[index] * size);
                }
              else if (ind_var_info.preheader_dest[index])
                {
                  L_delete_operand (branch_oper->src[1]);
                  branch_oper->src[1] =
                    L_copy_operand (ind_var_info.preheader_dest[index]);
                }
              else
                {
                  new_oper = L_create_new_op (Lop_ADD);
                  new_oper->dest[0] =
                    L_new_register_operand (++L_fn->max_reg_id,
                                            L_native_machine_ctype,
                                            L_PTYPE_NULL);
                  new_oper->src[0] = branch_oper->src[1];
                  new_oper->src[1] =
                    L_new_gen_int_operand (ind_var_info.inc[index] * size);
                  L_insert_oper_after (preheader, preheader->last_op,
                                       new_oper);

                  branch_oper->src[1] = L_copy_operand (new_oper->dest[0]);
                  ind_var_info.preheader_dest[index] = new_oper->dest[0];
                }
            }
          else
            L_punt ("L_do_longword_loop: unknown branch type.");
        }
    }

#ifdef DEBUG_LONGWORD_LOOP
  fprintf (stderr, "Loop: %i\n", loop->id);
#endif
  for (i = 0; i < num_cb; i++)
    {
      cb = L_cb_hash_tbl_find (L_fn->cb_hash_tbl, loop_cb[i]);
#ifdef DEBUG_LONGWORD_LOOP
      L_print_cb (stderr, NULL, cb);
#endif

      /*
       * First convert any branch instructions.  Since this
       * may require duplicating the original CBs, we need
       * need to convert the branches before longword-converting the
       * other operands in the CB
       */
      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          if (L_cond_branch_opcode (op))
            {
              if (L_cb_in_loop (L_find_branch_dest (op), loop_cb, num_cb))
                {
                  if (L_short_operand (op->src[0], &short_info, &s))
		    L_expand_longword_branch (loop, cb, op, size,
					      index_operand);

                  if (L_short_operand (op->src[1], &short_info, &s))
		    L_expand_longword_branch (loop, cb, op, size,
					      index_operand);
                }
            }
        }

      for (op = cb->first_op; op != NULL; op = op->next_op)
        {
          int val;

          switch (op->opc)
            {
            case Lop_ADD:
            case Lop_ADD_U:
            case Lop_SUB:
            case Lop_SUB_U:
              if (L_is_int_constant (op->src[1]))
                {
                  op->src[1]->value.i *= (ITintmax) size;
                }
              else if (L_is_variable (op->src[1]))
                {
#ifdef DEBUG_LONGWORD_LOOP
                  L_print_oper (stderr, op);
                  fprintf (stderr, "Add with variable.\n");
#endif

                }
              else
                {
                  L_print_oper (stderr, op);
                  L_punt ("L_do_longword: can't convert oper");
                }
              break;
            case Lop_AND:
            case Lop_OR:
            case Lop_XOR:
#ifdef IT64BIT
              L_punt ("L_do_longword_loop_conversion: Not 64-bit compliant.");
#endif
              if (L_is_int_constant (op->src[1]))
                {
                  val = op->src[1]->value.i;
                  if (size == 4)
                    val = val | (val << 8) | (val << 16) | (val << 24);
                  else
                    val = val | (val << 16);
                  op->src[1]->value.i = val;
                }
              else
                L_punt ("L_do_longword: logic oper must have constant src1");
              break;
            case Lop_LD_C:
            case Lop_LD_UC:
            case Lop_LD_C2:
            case Lop_LD_UC2:
              L_change_opcode (op, Lop_LD_I);
              break;
            case Lop_ST_C:
              L_change_opcode (op, Lop_ST_I);
              break;
            };
        }
    }

  Lcode_free (loop_cb);
  Lcode_free (exit_cb);
  L_rebuild_src_flow (L_fn);
  if (loop->header->id == 0)
    L_print_func (stderr, L_fn);
  L_check_func (L_fn);
  return 1;
}
