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
 *      File :          l_jump_opti.c
 *      Description :   jump optimization
 *      Info required : dominator information
 *      Creation Date : April, 1991
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_opti.h"

#undef DEBUG_JRG_EXPANSION

#define ERR     stderr

/*****************************************************************************
 * REGISTER JUMP OPTIMIZATION                                                *
 * ------------------------------------------------------------------------- *
 * Moved here from Lsuperscalar to expose likely cases of case/switch to     *
 * hyperblock formation                                                      *
 *****************************************************************************/

/*
 * Perform two optimizations here.  First, if switch is dominated by a 
 * single case or a few adjacent cases, pull cases out of the table
 * using explicit conditional branches, to precede the jump_rg.  Second,
 * if the switch is dominated by a single target (possibly of multiple
 * cases) check for that target in the value returned from the table
 * and jump there with a conditional branch.  The hope is that these
 * two optimizations will allow most important paths to be pulled into
 * hyperblocks, as required.
 */

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


static L_Cb *
L_find_jump_rg_most_likely_tgt (L_Cb * cb)
{
  int i, max_element_position, n_jrg_flow = 0;
  double w;
  L_Flow *flow, *f, *first_jrg_flow;
  L_Oper *oper, *branch_oper;

  if (!L_register_branch_opcode (cb->last_op))
    L_punt ("L_find_jump_rg_most_likely_tgt: last must be jrg");

  /* zero out jrg element array */
  for (i = 0; i < L_num_jrg_element; i++)
    {
      L_jrg[i].weight = 0.0;
      L_jrg[i].target = i;
      L_jrg[i].id = -1;
    }

  flow = cb->dest_flow;
  w = cb->weight;
  for (oper = cb->first_op; oper != cb->last_op; oper = oper->next_op)
    {
      if (L_is_control_oper (oper))
        {
          w -= flow->weight;
          flow = flow->next_flow;
        }
    }

  /* Determine the number of elements to sort */
  max_element_position = 0;

  first_jrg_flow = flow;

  for (f = first_jrg_flow; f != NULL; f = f->next_flow)
    n_jrg_flow++;

  for (f = first_jrg_flow; f != NULL; f = f->next_flow)
    {
      /*
       * REH 10/95 - Don't allow boundary cb's to be
       *  a likely target.
       */
      if (L_EXTRACT_BIT_VAL (f->dst_cb->flags, L_CB_BOUNDARY))
        continue;
      L_jrg[f->dst_cb->id].weight += f->weight;

      /* Need to define the branch id in the L_jrg table */
      branch_oper = L_find_branch_for_flow (cb, f);
      L_jrg[f->dst_cb->id].id = branch_oper->id;

      if (f->dst_cb->id > max_element_position)
        max_element_position = f->dst_cb->id;
    }

  max_element_position += 1;
  
  /* Changed to count to max_element_position */
  bubble_sort (L_jrg, max_element_position);    

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

#if 0
typedef struct _L_Target_Info {
  L_Cb *target;
  double weight;
} L_Target_Info;

typedef struct _L_Case_Info {
  L_Flow *flow;
  L_Target_Info *tgt;
} L_Case_Info;

typedef struct _L_JRG_Info {
  L_Cb *cb;
  L_Oper *jrg;
  L_Flow *jrg_flows;
  L_Flow *jrg_default_flow;
  double jrg_weight;
  int case_cnt;
  int tgt_cnt;
  HashTable target_hash;
} L_JRG_Info;

static L_ALLOC_POOL (L_Target_Info);
static L_ALLOC_POOL (L_Case_Info);

void
L_build_jrg_info (L_JRG_Info *jrg_info, L_Cb *cb, L_Oper *jrg_oper)
{
  L_Oper *op;
  L_Flow *fl;
  int case_cnt = 0;
  double weight = cb->weight;
  HashTable h;

  jrg_info->cb = cb;
  jrg_info->jrg = jrg_oper;
  jrg_info->jrg_flows = L_find_flow_for_branch (cb, jrg_oper);
  jrg_info->jrg_default_flow = L_find_last_flow (cb->dest_flow);

  h = HashTable_create (32);
  
  for (op = cb->first_op; op && op != jrg_oper; op = op->next)
    {
      if (!L_is_control_oper (op))
	continue;
      weight -= fl->weight;
      fl = fl->next_flow;
    }

  jrg_info->weight = weight;

  for (; fl; fl = fl->next_flow)
    {
      case_cnt++;
      if ((tgt = HashTable_find_or_null (h, fl->dst_cb->id)))
	{
	  tgt->weight+=fl->weight;
	}
      else
	{
	  tgt = L_ALLOC (L_JRG_Target);
	  tgt->target = fl->dst_cb;
	  tgt->weight = fl->weight;
	  HashTable_insert (h, (void *)tgt);
	}
    }

  jrg_info->target_hash = h;
  return;
}

void
L_clean_jrg_info (L_JRG_Info *jrg_info)
{
  HashTable h;
  L_Target_Info *t;

  h = jrg_info->target_hash;

  HashTable_start (h);
  while ((t = (L_Target_Info *) HashTable_next (h)))
    L_FREE (L_Target_Info, t);

  HashTable_free (h);
  return;
}
#endif

void
L_jump_rg_expansion (L_Func * fn)
{
  int j;
  double new_weight, ratio;
  L_Cb *cb, *new_cb, *tgt_cb, *t;
  L_Oper *oper, *new_op, *last, *branch_oper;
  L_Flow *flow;

#if 0
  L_INIT_ALLOC_POOL (L_Target_Info);
  L_INIT_ALLOC_POOL (L_Case_Info);
#endif

  L_num_jrg_element = IMPACT_MAX (fn->max_oper_id, fn->max_cb_id) + 1;
  L_jrg =
    (L_Jrg_Element *) Lcode_malloc (sizeof (L_Jrg_Element) *
                                    L_num_jrg_element);

#ifdef DEBUG_JRG_EXPANSION
  fprintf (stderr, ">> Enter L_jump_rg_expansion\n");
#endif

  L_clear_src_flow (fn);

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      L_Oper *jrg_oper;
#if 0
      L_JRG_Info jrg_info;
#endif

      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_ENTRANCE_BOUNDARY) &&
          L_EXTRACT_BIT_VAL (cb->flags, L_CB_PROLOGUE))
        continue;
      if (cb->weight < MIN_JRG_OPTI_WEIGHT)
        continue;
      if (!(jrg_oper = cb->last_op))
	continue;
      if (jrg_oper == cb->first_op)
	continue;
      if (!L_register_branch_opcode (jrg_oper))
        continue;
      if (L_is_predicated (jrg_oper))
        continue;

#if 0
      /* We have a significant CB ending with an unconditional
       * jump_rg operation.  Build information about the jump_rg
       * and determine if optimizaion is appropriate.
       */

      L_build_jrg_info (&jrg_info, cb, jrg_oper);

      if (jrg_info->jrg_weight)
	{
	  int change = 0;
	  int psize = M_type_size (M_TYPE_POINTER);
	  do 
	    {
	      L_Flow *maxfl;
	      int ofst = 0, maxofst = 0;
	      for (fl = jrg_info->jrg_flows, maxfl = fl; fl; fl = fl->next)
		{
		  if (fl->weight > maxfl->weight)
		    {
		      maxfl = fl;
		      maxofst = ofst;
		    }
		  ofst += psize;
		}

	      if ((maxfl->weight / jrg_info->jrg_weight) > 0.30)
		{
		  /* Perform optimization: pull a case out of the switch */

		  /* jump_rg tbl[k] --> if (k == maxofst) goto L; else... */
		  change++;
		}
	    }
	  while (change);
	}

      L_clean_jrg_info (&jrg_info);
#endif

      if (!(tgt_cb = L_find_jump_rg_most_likely_tgt (cb)))
        continue;
      if (tgt_cb == cb)
        continue;
      if (!tgt_cb->first_op)
        continue;
      if (L_subroutine_return_opcode (tgt_cb->last_op))
        continue;
      if ((L_cb_size (cb) + L_cb_size (tgt_cb)) > MAX_SIZE)
        continue;

#ifdef DEBUG_JRG_EXPANSION
      fprintf (stderr,
	       ">> Perform reg branch exp, expand cb %d into cb %d\n",
	       tgt_cb->id, cb->id);
      for (j = 0; j < L_num_pretest_branch; j++)
	fprintf (stderr, "\t> cb %d \tweight %f \tbranch %d\n",
		 L_jrg[j].target, L_jrg[j].weight, L_jrg[j].id);
#endif

      new_cb = L_create_cb (0.0);
      L_insert_cb_after (fn, cb, new_cb);
      last = cb->last_op;
      flow = L_find_flow_for_branch (cb, last);
      if (!flow->prev_flow)
	{
	  cb->dest_flow = NULL;
	}
      else
        {
          flow->prev_flow->next_flow = NULL;
          flow->prev_flow = NULL;
        }
      new_cb->dest_flow = L_concat_flow (new_cb->dest_flow, flow);
      L_change_src (new_cb->dest_flow, cb, new_cb);
      L_remove_oper (cb, last);
      L_insert_oper_after (new_cb, new_cb->last_op, last);

      new_weight = 0.0;
      for (flow = new_cb->dest_flow; flow != NULL; flow = flow->next_flow)
        {
          int flag = 0;

          branch_oper = L_find_branch_for_flow (new_cb, flow);
          for (j = 0; j < L_num_pretest_branch; j++)
            {
              if ((flow->dst_cb->id == L_jrg[j].target) &
                  (branch_oper->id == L_jrg[j].id))
                {
                  flow->weight = 0.0;
                  flag = 1;
                }
            }
          if (!flag)
            new_weight += flow->weight;
        }
      new_cb->weight = new_weight;

      /* Insert pretest branches */
      for (j = 1; j < L_num_pretest_branch; j++)
        {
          new_op = L_create_new_op (Lop_BR);
          new_op->src[0] = L_copy_operand (new_cb->last_op->src[0]);
          L_set_compare (new_op, new_op->src[0]->ctype, Lcmp_COM_EQ);
          t = L_cb_hash_tbl_find (fn->cb_hash_tbl, L_jrg[j].target);
          new_op->src[1] = L_new_cb_operand (t);
          new_op->src[2] = L_new_cb_operand (t);
          L_insert_oper_after (cb, cb->last_op, new_op);
          flow = L_new_flow (1, cb, t, L_jrg[j].weight);
          cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
        }

      /* insert branch to original jrg in new_cb */
      new_op = L_create_new_op (Lop_BR);
      new_op->src[0] = L_copy_operand (new_cb->last_op->src[0]);
      L_set_compare (new_op, new_op->src[0]->ctype, Lcmp_COM_NE);
      new_op->src[1] = L_new_cb_operand (tgt_cb);
      new_op->src[2] = L_new_cb_operand (new_cb);
      L_insert_oper_after (cb, cb->last_op, new_op);
      flow = L_new_flow (1, cb, new_cb, new_weight);
      cb->dest_flow = L_concat_flow (cb->dest_flow, flow);

      /* copy contents of tgt_cb to end of cb */
      for (oper = tgt_cb->first_op; oper; oper = oper->next_op)
	L_insert_oper_after (cb, cb->last_op, L_copy_operation (oper));

      if (L_has_fallthru_to_next_cb (tgt_cb))
        {
          new_op = L_create_new_op (Lop_JUMP);
          flow = L_find_last_flow (tgt_cb->dest_flow);
          new_op->src[0] = L_new_cb_operand (flow->dst_cb);
          L_insert_oper_after (cb, cb->last_op, new_op);
        }

      flow = L_copy_flow (tgt_cb->dest_flow);
      L_change_src (flow, tgt_cb, cb);

      if (tgt_cb->weight == 0)
        ratio = 0;
      else
        ratio = L_jrg[0].weight / tgt_cb->weight;
      L_scale_flow_weights (flow, ratio);
      cb->dest_flow = L_concat_flow (cb->dest_flow, flow);
      tgt_cb->weight -= L_jrg[0].weight;
      L_scale_flow_weights (tgt_cb->dest_flow, (1.0 - ratio));
      if (L_EXTRACT_BIT_VAL (tgt_cb->flags, L_CB_HYPERBLOCK))
        cb->flags = L_SET_BIT_FLAG (cb->flags, L_CB_HYPERBLOCK);
    }

  L_rebuild_src_flow (fn);

  Lcode_free (L_jrg);

#if 0
  L_DEINIT_ALLOC_POOL (L_Target_Info);
  L_DEINIT_ALLOC_POOL (L_Case_Info);
#endif

  return;
}

