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
 *      File :          l_multiway.c
 *      Description :   multiway branch optimization
 *      Author :        Scott A. Mahlke
 *      Revised 7-93 :  Scott A. Mahlke
 *              New Lcode
 *      Export funcs :  L_multiway_branch_expansion(fn)
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"

#define MIN_JRG_OPTI_WEIGHT             1000.0
#define LIKELY_JUMP_RG_TAKEN_RATIO      0.25
#define MIN_JUMP_RG_TAKEN_RATIO         0.05
#define MAX_NUM_PRETEST_BRANCH          4
#define MAX_SIZE                        150

typedef struct _L_Jrg_Element {
  double weight;
  int target;
  int cc;
  int id;
} L_Jrg_Element;

static int L_num_jrg_element = 0;
static int L_num_pretest_branch = 0;
static L_Jrg_Element *L_jrg = NULL;

static void
bubble_sort (L_Jrg_Element * x, int n)
{
  int i, j;
  int swap;
  L_Jrg_Element temp;

  for (i = 0; i < n; i++)
    {
      swap = 0;
      for (j = i + 1; j < n; j++)
        {
          if (x[i].weight < x[j].weight)
            {
              temp = x[i];
              x[i] = x[j];
              x[j] = temp;
              swap = 1;
            }
        }
      if (!swap)
        break;
    }
}


static L_Oper *
L_get_branch (L_Cb * cb, int cc, int target, int branch_id)
{
  L_Flow *flow;
  L_Oper *oper;

  if (!(flow = cb->dest_flow))
    L_punt ("L_get_branch: cb has no dest flow arcs");

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (!L_cond_branch (oper))
	continue;

      if ((oper->id == branch_id) && (flow->cc == cc) &&
	  (flow->dst_cb->id == target))
	break;
      else if (!(flow = flow->next_flow))
	L_punt ("L_get_branch: branch not found");
    }

  if (!oper)
    L_punt ("L_get_branch: branch not found");
  return oper;
}


static L_Oper *
L_find_start_of_multiway_branch_sequence (L_Cb * cb)
{
  int count = 0;
  L_Oper *oper, *last;

  if (L_uncond_branch_opcode (cb->last_op))
    last = cb->last_op->prev_op;
  else
    last = cb->last_op;

  for (oper = last; oper != NULL; oper = oper->prev_op)
    {
      if (!L_int_beq_branch_opcode (oper))
        break;
      if (!L_is_register (oper->src[0]))
        break;
      if (!L_same_operand (oper->src[0], last->src[0]))
        break;
      if (!L_is_int_constant (oper->src[1]))
        break;
      count++;
    }

  if (count <= 1)
    return NULL;

  if (!oper)
    return (cb->first_op);
  else
    return (oper->next_op);
}


static L_Cb *
L_find_likely_target_cb_of_multiway_branch_sequence (L_Cb * cb,
                                                     L_Oper * first_branch)
{
  int i, index;
  double w;
  L_Flow *flow, *f;
  L_Oper *oper;
  L_Oper *branch_oper;

  for (i = 0; i < L_num_jrg_element; i++)
    {
      L_jrg[i].weight = -1.0;
      L_jrg[i].target = -1;
      L_jrg[i].id = -1;
    }

  flow = cb->dest_flow;
  w = cb->weight;
  for (oper = cb->first_op; oper != first_branch; oper = oper->next_op)
    {
      if (!L_general_branch_opcode (oper))
        continue;
      if (L_cond_branch_opcode (oper))
        {
          w -= flow->weight;
          flow = flow->next_flow;
        }
      else if (L_is_predicated (oper) && L_uncond_branch_opcode (oper))
        {
          w -= flow->weight;
          flow = flow->next_flow;
        }
      else
        {
          L_punt
            ("L_find_likely_target_cb_of_multiway_branch_sequence: illegal\
                                 op in branch sequence");
        }
    }

  index = 0;
  for (f = flow; f->next_flow != NULL; f = f->next_flow)
    {
      /*
       * REH 10/95 - Don't allow boundary cb's to be
       *  a likely target.
       */
      if (L_EXTRACT_BIT_VAL (f->dst_cb->flags, L_CB_BOUNDARY))
        continue;

      L_jrg[index].cc = f->cc;
      L_jrg[index].target = f->dst_cb->id;
      L_jrg[index].weight = f->weight;

      /* Need to define the branch id in the L_jrg table */
      branch_oper = L_find_branch_for_flow (cb, f);
      L_jrg[index].id = branch_oper->id;

      index++;
    }

  bubble_sort (L_jrg, index + 1);       /* Changed the sort count to index */

  if (L_jrg[0].weight < MIN_JRG_OPTI_WEIGHT)
    return NULL;
  if ((L_jrg[0].weight / w) < LIKELY_JUMP_RG_TAKEN_RATIO)
    return NULL;

  L_num_pretest_branch = 1;
  for (i = 1; i < MAX_NUM_PRETEST_BRANCH; i++)
    {
      if (L_jrg[i].weight < MIN_JRG_OPTI_WEIGHT)
        break;
      if ((L_jrg[i].weight / w) < LIKELY_JUMP_RG_TAKEN_RATIO)
        break;
      L_num_pretest_branch++;
    }

  return (L_cb_hash_tbl_find (L_fn->cb_hash_tbl, L_jrg[0].target));
}


void
L_multiway_branch_expansion (L_Func * fn)
{
  int j;
  L_Cb *cb, *target_cb, *new_cb;
  L_Oper *oper, *new_op, *first_branch;
  L_Flow *flow, *new_flow;
  double ratio;

  if (Lsuper_debug_multiway_branch_opti)
    fprintf (stderr, ">> Enter L_multiway_branch_expansion\n");

  L_num_jrg_element = IMPACT_MAX (fn->max_oper_id, fn->max_cb_id) + 1;
  L_jrg =
    (L_Jrg_Element *) Lcode_malloc (sizeof (L_Jrg_Element) *
                                    L_num_jrg_element);

  L_clear_src_flow (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /*
       * REH - 10/9/95 Don't touch the boundary cb's
       */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_BOUNDARY))
        continue;

      if (!cb->first_op)
        continue;
      if (cb->weight < MIN_JRG_OPTI_WEIGHT)
        continue;
      if (!(first_branch = L_find_start_of_multiway_branch_sequence (cb)))
        continue;
      if (first_branch == cb->first_op)
        continue;
      if (L_is_predicated (first_branch))
        continue;
      target_cb = 
        L_find_likely_target_cb_of_multiway_branch_sequence (cb,
                                                             first_branch);
      if (target_cb == NULL)
        continue;
      if (target_cb->first_op == NULL)
        continue;
      if (L_subroutine_return_opcode (target_cb->last_op))
        continue;
      if ((L_cb_size (cb) + L_cb_size (target_cb)) > MAX_SIZE)
        continue;

      if (Lsuper_debug_multiway_branch_opti)
        {
          fprintf (stderr,
                   ">> Perform br sequence exp, expand cb %d into cb %d\n",
                   target_cb->id, cb->id);
          for (j = 0; j < L_num_pretest_branch; j++)
            {
              fprintf (stderr, "\t> cb %d \tweight %f \tbranch %d\n",
                       L_jrg[j].target, L_jrg[j].weight, L_jrg[j].id);
            }
          STAT_COUNT ("L_branch_sequence_expansion", 1, cb);
        }

      new_cb = L_create_cb (0.0);
      L_insert_cb_after (fn, cb, new_cb);
      for (j = L_num_pretest_branch; j < L_num_jrg_element; j++)
        {
          if (L_jrg[j].target == -1)
            break;
          oper = L_get_branch (cb, L_jrg[j].cc, L_jrg[j].target, L_jrg[j].id);
          flow = L_find_flow_for_branch (cb, oper);
          new_flow =
            L_new_flow (flow->cc, new_cb, flow->dst_cb, flow->weight);
          cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
          new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
          L_remove_oper (cb, oper);
          L_insert_oper_after (new_cb, new_cb->last_op, oper);
        }
      if (L_uncond_branch_opcode (cb->last_op))
        {
          oper = cb->last_op;
          L_remove_oper (cb, oper);
          L_insert_oper_after (new_cb, new_cb->last_op, oper);
        }
      flow = L_find_last_flow (cb->dest_flow);
      new_flow = L_new_flow (flow->cc, new_cb, flow->dst_cb, flow->weight);
      cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
      new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, new_flow);
      /* determine weight of new_cb */
      for (flow = new_cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          new_cb->weight += flow->weight;
        }
      first_branch =
        L_get_branch (cb, L_jrg[0].cc, L_jrg[0].target, L_jrg[0].id);
      flow = L_find_flow_for_branch (cb, first_branch);
      if (target_cb == cb)
        {
          /* make into superblock loop */
          new_flow = L_new_flow (flow->cc, cb, cb, flow->weight);
          cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
          cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);
          L_remove_oper (cb, first_branch);
          L_insert_oper_after (cb, cb->last_op, first_branch);
          /* fallthru arc */
          new_flow = L_new_flow (0, cb, new_cb, new_cb->weight);
          cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);
        }
      else
        {
          /* Insert branch at end of cb to jump to new_cb */
          cb->dest_flow = L_delete_flow (cb->dest_flow, flow);
          L_remove_oper (cb, first_branch);
          L_set_compare_type (first_branch, Lcmp_COM_NE);
          first_branch->src[2]->value.cb = new_cb;
          L_insert_oper_after (cb, cb->last_op, first_branch);
          new_flow = L_new_flow (1, cb, new_cb, new_cb->weight);
          cb->dest_flow = L_concat_flow (cb->dest_flow, new_flow);
          /* Copy contents of target_cb to end of cb */
          for (oper = target_cb->first_op; oper != NULL; oper = oper->next_op)
            {
              L_insert_oper_after (cb, cb->last_op, L_copy_operation (oper));
            }
          if (L_has_fallthru_to_next_cb (target_cb))
            {
              new_op = L_create_new_op (Lop_JUMP);
              flow = L_find_last_flow (target_cb->dest_flow);
              new_op->src[0] = L_new_cb_operand (flow->dst_cb);
              L_insert_oper_after (cb, cb->last_op, new_op);
            }
          flow = L_copy_flow (target_cb->dest_flow);
          L_change_src (flow, target_cb, cb);
          if (target_cb->weight != 0.0)
            ratio = L_jrg[0].weight / target_cb->weight;
          else
            ratio = 1.0;
          L_scale_flow_weights (flow, ratio);
          cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
          target_cb->weight -= L_jrg[0].weight;
          L_scale_flow_weights (target_cb->dest_flow, (1.0 - ratio));
          if (L_EXTRACT_BIT_VAL (target_cb->flags, L_CB_HYPERBLOCK))
            cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
        }
    }

  L_rebuild_src_flow (fn);

  Lcode_free (L_jrg);
}
