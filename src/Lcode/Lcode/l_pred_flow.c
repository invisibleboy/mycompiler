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
/****************************************************************************\
 *      File:   l_pred_flow.c
 *      Author: David August, Wen-mei Hwu
 *      Creation Date:  January 1997
 *
\****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#define DEBUG_PRED_FLOW 0
#define DEBUG_PFG_PATHS

/* Memory allocation pools */
L_Alloc_Pool *PF_alloc_pf_cb = NULL;
L_Alloc_Pool *PF_alloc_pf_bb = NULL;
L_Alloc_Pool *PF_alloc_pf_oper = NULL;
L_Alloc_Pool *PF_alloc_pf_node = NULL;
L_Alloc_Pool *PF_alloc_pf_inst = NULL;
L_Alloc_Pool *PF_alloc_pf_operand = NULL;
L_Alloc_Pool *PF_alloc_pred_flow = NULL;
L_Alloc_Pool *PF_alloc_pf_mem_set = NULL;

/* 
 * Initialize predicate flow memory pools 
 */
void
PF_initialize (void)
{
  if (!PF_alloc_pf_cb)
    PF_alloc_pf_cb = L_create_alloc_pool ("PF_CB", sizeof (PF_CB), 512);
  if (!PF_alloc_pf_bb)
    PF_alloc_pf_bb = L_create_alloc_pool ("PF_BB", sizeof (PF_BB), 512);
  if (!PF_alloc_pf_oper)
    PF_alloc_pf_oper =
      L_create_alloc_pool ("PF_OPER", sizeof (PF_OPER), 1024);
  if (!PF_alloc_pf_node)
    PF_alloc_pf_node = L_create_alloc_pool ("PF_NODE", sizeof (PF_NODE), 512);
  if (!PF_alloc_pf_inst)
    PF_alloc_pf_inst =
      L_create_alloc_pool ("PF_INST", sizeof (PF_INST), 1024);
  if (!PF_alloc_pf_operand)
    PF_alloc_pf_operand =
      L_create_alloc_pool ("PF_OPERAND", sizeof (PF_OPERAND), 1024);
  if (!PF_alloc_pred_flow)
    PF_alloc_pred_flow =
      L_create_alloc_pool ("PRED_FLOW", sizeof (PRED_FLOW), 4);
  if (!PF_alloc_pf_mem_set)
    PF_alloc_pf_mem_set =
      L_create_alloc_pool ("PF_MEM_SET", sizeof (PF_MEM_SET), 32);
}


static DF_INST_INFO *
D_new_df_inst_info (void)
{
  DF_INST_INFO *df_info = (DF_INST_INFO *) L_alloc (D_alloc_df_inst_info);

  df_info->mem_r_in = df_info->mem_r_out = NULL;
  df_info->mem_a_in = df_info->mem_a_out = NULL;

  return (df_info);
}


static DF_OPER_INFO *
D_new_df_oper_info (void)
{
  DF_OPER_INFO *df_info = (DF_OPER_INFO *) L_alloc (D_alloc_df_oper_info);

  df_info->dom = df_info->post_dom = NULL;
  df_info->v_in = df_info->v_out = NULL;
  df_info->r_in = df_info->r_out = NULL;
  df_info->a_in = df_info->a_out = NULL;
  df_info->e_in = df_info->e_out = NULL;
  df_info->mem_r_in = df_info->mem_r_out = NULL;
  df_info->mem_a_in = df_info->mem_a_out = NULL;

  return (df_info);
}


/* Not static: needs to be dynamically allocated during PCE dataflows. */
DF_PCE_INFO *
D_new_df_pce_info (void)
{
  DF_PCE_INFO *df_info = (DF_PCE_INFO *) L_alloc (D_alloc_df_pce_info);

  df_info->trans = NULL;
  df_info->n_comp = df_info->x_comp = NULL;
  df_info->complement = NULL;
  df_info->nd_safe = df_info->xd_safe = NULL;
  df_info->nu_safe = df_info->xu_safe = NULL;
  df_info->n_earliest = df_info->x_earliest = NULL;
  df_info->n_delayed = df_info->x_delayed = NULL;
  df_info->n_latest = df_info->x_latest = NULL;
  df_info->n_isolated = df_info->x_isolated = NULL;
  df_info->n_insert = df_info->x_insert = NULL;
  df_info->n_replace = df_info->x_replace = NULL;

  df_info->n_spec_us = df_info->x_spec_us = NULL;
  df_info->n_spec_ds = df_info->x_spec_ds = NULL;
  df_info->speculate_up = NULL;

  df_info->loc_delayed = NULL;
  df_info->pred_guard = NULL;
  df_info->pred_set = NULL;
  df_info->pred_clear = NULL;

  return df_info;
}


static DF_PCE_INFO *
D_delete_df_pce_info (DF_PCE_INFO * df_info)
{
  if (!df_info)
    return NULL;

  df_info->trans = Set_dispose (df_info->trans);
  df_info->n_comp = Set_dispose (df_info->n_comp);
  df_info->x_comp = Set_dispose (df_info->x_comp);
  df_info->complement = Set_dispose (df_info->complement);
  df_info->nd_safe = Set_dispose (df_info->nd_safe);
  df_info->xd_safe = Set_dispose (df_info->xd_safe);
  df_info->nu_safe = Set_dispose (df_info->nu_safe);
  df_info->xu_safe = Set_dispose (df_info->xu_safe);
  df_info->n_earliest = Set_dispose (df_info->n_earliest);
  df_info->x_earliest = Set_dispose (df_info->x_earliest);
  df_info->n_delayed = Set_dispose (df_info->n_delayed);
  df_info->x_delayed = Set_dispose (df_info->x_delayed);
  df_info->n_latest = Set_dispose (df_info->n_latest);
  df_info->x_latest = Set_dispose (df_info->x_latest);
  df_info->n_isolated = Set_dispose (df_info->n_isolated);
  df_info->x_isolated = Set_dispose (df_info->x_isolated);
  df_info->n_insert = Set_dispose (df_info->n_insert);
  df_info->x_insert = Set_dispose (df_info->x_insert);
  df_info->n_replace = Set_dispose (df_info->n_replace);
  df_info->x_replace = Set_dispose (df_info->x_replace);

  df_info->n_spec_us = Set_dispose (df_info->n_spec_us);
  df_info->x_spec_us = Set_dispose (df_info->x_spec_us);
  df_info->n_spec_ds = Set_dispose (df_info->n_spec_ds);
  df_info->x_spec_ds = Set_dispose (df_info->x_spec_ds);
  df_info->speculate_up = Set_dispose (df_info->speculate_up);

  df_info->loc_delayed = Set_dispose (df_info->loc_delayed);
  df_info->pred_guard = Set_dispose (df_info->pred_guard);
  df_info->pred_set = Set_dispose (df_info->pred_set);
  df_info->pred_clear = Set_dispose (df_info->pred_clear);

  L_free (D_alloc_df_pce_info, df_info);
  return (NULL);
}


static DF_NODE_INFO *
D_new_df_node_info (void)
{
  DF_NODE_INFO *df_info = (DF_NODE_INFO *) L_alloc (D_alloc_df_node_info);

  df_info->dom = df_info->post_dom = NULL;
  df_info->use_gen = df_info->def_kill = NULL;
  df_info->in = df_info->out = NULL;
  df_info->mem_gen = df_info->mem_kill = NULL;
  df_info->mem_in = df_info->mem_out = NULL;

  df_info->pce_info = NULL;

  df_info->cnt = 0;

  return (df_info);
}


static DF_BB_INFO *
D_new_df_bb_info (void)
{
  DF_BB_INFO *df_info = (DF_BB_INFO *) L_alloc (D_alloc_df_bb_info);

  df_info->pce_info = NULL;

  return (df_info);
}


static DF_CB_INFO *
D_new_df_cb_info (void)
{
  DF_CB_INFO *df_info = (DF_CB_INFO *) L_alloc (D_alloc_df_cb_info);

  df_info->dom = df_info->post_dom = NULL;
  df_info->v_in = df_info->v_out = NULL;
  df_info->r_in = df_info->r_out = NULL;
  df_info->a_in = df_info->a_out = NULL;
  df_info->e_in = df_info->e_out = NULL;

  return (df_info);
}


static DF_INST_INFO *
D_delete_df_inst_info (DF_INST_INFO * df_info)
{
  if (!df_info)
    return (NULL);

  df_info->mem_r_in = Set_dispose (df_info->mem_r_in);
  df_info->mem_r_out = Set_dispose (df_info->mem_r_out);

  df_info->mem_a_in = Set_dispose (df_info->mem_a_in);
  df_info->mem_a_out = Set_dispose (df_info->mem_a_out);

  L_free (D_alloc_df_inst_info, df_info);
  return (NULL);
}


static DF_OPER_INFO *
D_delete_df_oper_info (DF_OPER_INFO * df_info)
{
  if (!df_info)
    return (NULL);

  df_info->dom = Set_dispose (df_info->dom);
  df_info->post_dom = Set_dispose (df_info->post_dom);

  df_info->v_in = Set_dispose (df_info->v_in);
  df_info->v_out = Set_dispose (df_info->v_out);

  df_info->r_in = Set_dispose (df_info->r_in);
  df_info->r_out = Set_dispose (df_info->r_out);

  df_info->a_in = Set_dispose (df_info->a_in);
  df_info->a_out = Set_dispose (df_info->a_out);

  df_info->e_in = Set_dispose (df_info->e_in);
  df_info->e_out = Set_dispose (df_info->e_out);

  df_info->mem_r_in = Set_dispose (df_info->mem_r_in);
  df_info->mem_r_out = Set_dispose (df_info->mem_r_out);

  df_info->mem_a_in = Set_dispose (df_info->mem_a_in);
  df_info->mem_a_out = Set_dispose (df_info->mem_a_out);

  L_free (D_alloc_df_oper_info, df_info);
  return (NULL);
}


static DF_NODE_INFO *
D_delete_df_node_info (DF_NODE_INFO * df_info)
{
  if (!df_info)
    return (NULL);

  df_info->dom = Set_dispose (df_info->dom);
  df_info->post_dom = Set_dispose (df_info->post_dom);

  df_info->use_gen = Set_dispose (df_info->use_gen);
  df_info->def_kill = Set_dispose (df_info->def_kill);

  df_info->in = Set_dispose (df_info->in);
  df_info->out = Set_dispose (df_info->out);

  df_info->mem_gen = List_reset (df_info->mem_gen);
  df_info->mem_kill = List_reset (df_info->mem_kill);

  df_info->mem_in = List_reset (df_info->mem_in);
  df_info->mem_out = List_reset (df_info->mem_out);

  df_info->pce_info = D_delete_df_pce_info (df_info->pce_info);

  L_free (D_alloc_df_node_info, df_info);
  return (NULL);
}


static DF_BB_INFO *
D_delete_df_bb_info (DF_BB_INFO *df_info)
{
  if (!df_info)
    return (NULL);

  df_info->pce_info = D_delete_df_pce_info (df_info->pce_info);

  L_free (D_alloc_df_bb_info, df_info);
  return (NULL);
}


static DF_CB_INFO *
D_delete_df_cb_info (DF_CB_INFO * df_info)
{
  if (!df_info)
    return (NULL);

  df_info->dom = Set_dispose (df_info->dom);
  df_info->post_dom = Set_dispose (df_info->post_dom);

  df_info->v_in = Set_dispose (df_info->v_in);
  df_info->v_out = Set_dispose (df_info->v_out);

  df_info->r_in = Set_dispose (df_info->r_in);
  df_info->r_out = Set_dispose (df_info->r_out);

  df_info->a_in = Set_dispose (df_info->a_in);
  df_info->a_out = Set_dispose (df_info->a_out);

  df_info->e_in = Set_dispose (df_info->e_in);
  df_info->e_out = Set_dispose (df_info->e_out);

  L_free (D_alloc_df_cb_info, df_info);
  return (NULL);
}


PF_MEM_SET *
PF_new_mem_set (int id, Set conflicts)
{
  PF_MEM_SET *new_mem_set;

  new_mem_set = (PF_MEM_SET *) L_alloc (PF_alloc_pf_mem_set);

  new_mem_set->id = id;
  new_mem_set->conflicts = Set_union (NULL, conflicts);

  return new_mem_set;
}


PF_MEM_SET *
PF_delete_mem_set (PF_MEM_SET * mem_set)
{
  if (!mem_set)
    return (NULL);

  mem_set->conflicts = Set_dispose (mem_set->conflicts);

  L_free (PF_alloc_pf_mem_set, mem_set);

  return (NULL);
}


static PF_CB *
PF_new_pf_cb (PRED_FLOW * pred_flow, L_Cb * cb)
{
  PF_CB *new_pf_cb;

  new_pf_cb = (PF_CB *) L_alloc (PF_alloc_pf_cb);

  new_pf_cb->cb = cb;
  new_pf_cb->pf_bbs = NULL;
  new_pf_cb->pf_nodes = NULL;
  new_pf_cb->pf_nodes_entry = NULL;
  new_pf_cb->pf_opers = NULL;
  new_pf_cb->info = D_new_df_cb_info ();

  HashTable_insert (pred_flow->hash_cb_pfcb, cb->id, new_pf_cb);
  pred_flow->list_pf_cb = List_insert_last (pred_flow->list_pf_cb, new_pf_cb);

  return new_pf_cb;
}


static PF_CB *
PF_delete_pf_cb (PF_CB * pf_cb)
{
  D_delete_df_cb_info (pf_cb->info);
  List_reset (pf_cb->pf_bbs);
  List_reset (pf_cb->pf_nodes);
  List_reset (pf_cb->pf_nodes_entry);
  List_reset (pf_cb->pf_opers);
  L_free (PF_alloc_pf_cb, pf_cb);
  return (NULL);
}


static PF_BB *
PF_new_pf_bb (PRED_FLOW * pred_flow, PF_CB * pf_cb)
{
  PF_BB *new_pf_bb;

  new_pf_bb = (PF_BB *) L_alloc (PF_alloc_pf_bb);

  new_pf_bb->pf_cb = pf_cb;
  new_pf_bb->first_op = NULL;
  new_pf_bb->pf_nodes = NULL;
  new_pf_bb->pf_nodes_entry = NULL;
  new_pf_bb->pf_nodes_last = NULL;
  new_pf_bb->info = D_new_df_bb_info();

  return new_pf_bb;
}


static PF_BB *
PF_delete_pf_bb (PF_BB * pf_bb)
{
  D_delete_df_bb_info (pf_bb->info);
  List_reset (pf_bb->pf_nodes);
  List_reset (pf_bb->pf_nodes_entry);
  List_reset (pf_bb->pf_nodes_last);
  L_free (PF_alloc_pf_bb, pf_bb);
  return (NULL);
}


static PF_OPER *
PF_new_pf_oper (PRED_FLOW * pred_flow, L_Oper * oper)
{
  PF_OPER *new_pf_oper;

  new_pf_oper = (PF_OPER *) L_alloc (PF_alloc_pf_oper);

  new_pf_oper->oper = oper;
  new_pf_oper->pf_insts = NULL;
  new_pf_oper->info = D_new_df_oper_info ();

  new_pf_oper->src = NULL;
  new_pf_oper->dest = NULL;
  new_pf_oper->mem_src = NULL;
  new_pf_oper->mem_dest = NULL;

  new_pf_oper->flags = 0;

  HashTable_insert (pred_flow->hash_oper_pfoper, oper->id, new_pf_oper);
  pred_flow->oper_U = Set_add (pred_flow->oper_U, oper->id);

  return new_pf_oper;
}


static PF_OPER *
PF_delete_pf_oper (PF_OPER * pf_oper)
{
  if (!pf_oper)
    return (NULL);
  D_delete_df_oper_info (pf_oper->info);
  List_reset (pf_oper->pf_insts);
  List_reset (pf_oper->src);
  List_reset (pf_oper->dest);
  List_reset (pf_oper->mem_src);
  List_reset (pf_oper->mem_dest);
  L_free (PF_alloc_pf_oper, pf_oper);
  return (NULL);
}


static PF_NODE *
PF_new_pf_node (PRED_FLOW * pred_flow, PF_CB * pf_cb)
{
  PF_NODE *new_pf_node;

  new_pf_node = (PF_NODE *) L_alloc (PF_alloc_pf_node);

  new_pf_node->pf_cb = pf_cb;

  new_pf_node->pf_insts = NULL;
  new_pf_node->pred_def = NULL;
  new_pf_node->pred_true = NULL;
  new_pf_node->succ = NULL;
  new_pf_node->pred = NULL;
  new_pf_node->info = D_new_df_node_info ();
  new_pf_node->count = 0;
  new_pf_node->flags = 0;

  pred_flow->list_pf_node =
    List_insert_last (pred_flow->list_pf_node, new_pf_node);
  new_pf_node->id = List_size (pred_flow->list_pf_node);
  pred_flow->pf_node_U = Set_add (pred_flow->pf_node_U, new_pf_node->id);
  HashTable_insert (pred_flow->hash_pf_node, new_pf_node->id, new_pf_node);

  pf_cb->pf_nodes = List_insert_last (pf_cb->pf_nodes, new_pf_node);

  return new_pf_node;
}


static PF_NODE *
PF_delete_pf_node (PF_NODE * pf_node)
{
  if (!pf_node)
    return (NULL);

  D_delete_df_node_info (pf_node->info);
  List_reset (pf_node->pf_insts);
  List_reset (pf_node->succ);
  List_reset (pf_node->pred);
  pf_node->pred_def = Set_dispose (pf_node->pred_def);
  pf_node->pred_true = Set_dispose (pf_node->pred_true);
  L_free (PF_alloc_pf_node, pf_node);
  return (NULL);
}


static PF_INST *
PF_new_pf_inst (PRED_FLOW * pred_flow, PF_OPER * pf_oper)
{
  PF_INST *new_pf_inst;

  new_pf_inst = (PF_INST *) L_alloc (PF_alloc_pf_inst);

  new_pf_inst->pf_oper = pf_oper;
  new_pf_inst->info = D_new_df_inst_info ();
  pred_flow->num_pf_inst++;
  new_pf_inst->id = pred_flow->num_pf_inst;

  new_pf_inst->src = NULL;
  new_pf_inst->dest = NULL;
  new_pf_inst->mem_src = NULL;
  new_pf_inst->mem_dest = NULL;
  new_pf_inst->flags = 0;

  new_pf_inst->pf_node = NULL;

  new_pf_inst->pred_true = FALSE;
  new_pf_inst->pred_known = FALSE;

  HashTable_insert (pred_flow->hash_pf_inst, new_pf_inst->id, new_pf_inst);

  return new_pf_inst;
}


static PF_INST *
PF_delete_pf_inst (PF_INST * pf_inst)
{
  if (!pf_inst)
    return (NULL);

  D_delete_df_inst_info (pf_inst->info);
  List_reset (pf_inst->src);
  List_reset (pf_inst->dest);
  List_reset (pf_inst->mem_src);
  List_reset (pf_inst->mem_dest);
  L_free (PF_alloc_pf_inst, pf_inst);
  return (NULL);
}


static PF_OPERAND *
PF_new_pf_operand (PRED_FLOW * pred_flow, PF_OPER * pf_oper,
                   L_Operand * operand)
{
  PF_OPERAND *pf_operand;

  pf_operand = (PF_OPERAND *) L_alloc (PF_alloc_pf_operand);
  pf_operand->pf_oper = pf_oper;
  pred_flow->num_pf_operand++;
  pf_operand->id = pred_flow->num_pf_operand;
  pf_operand->operand = operand;

  switch (L_return_old_type (operand))
    {
    case L_OPERAND_RREGISTER:
      pf_operand->reg = L_REG_INDEX (operand->value.rr);
      break;
    case L_OPERAND_REGISTER:
      pf_operand->reg = L_REG_INDEX (operand->value.r);
      break;
    case L_OPERAND_MACRO:
      pf_operand->reg = L_MAC_INDEX (operand->value.mac);
      break;
    default:
      L_punt ("D_create_df_operand: Operand is not a register or macro.");
    }

  pf_operand->memory = FALSE;
  pf_operand->transparent = FALSE;
  pf_operand->unconditional = FALSE;
  if (L_is_ctype_predicate (operand))
    {
      if (L_is_update_predicate_ptype (operand->ptype))
        pf_operand->transparent = TRUE;

      if (L_is_uncond_predicate_ptype (operand->ptype))
	pf_operand->unconditional = TRUE;
    }

  HashTable_insert (pred_flow->hash_pf_operand, pf_operand->id, pf_operand);
  pred_flow->reg_U = Set_add (pred_flow->reg_U, pf_operand->reg);
  pred_flow->pf_operand_U = Set_add (pred_flow->pf_operand_U, pf_operand->id);

  return (pf_operand);
}


static PF_OPERAND *
PF_new_pf_memory_operand (PRED_FLOW * pred_flow, PF_OPER * pf_oper)
{
  PF_OPERAND *pf_operand;

  pf_operand = (PF_OPERAND *) L_alloc (PF_alloc_pf_operand);
  pf_operand->pf_oper = pf_oper;
  pred_flow->num_pf_operand++;
  pf_operand->id = pred_flow->num_pf_operand;
  pf_operand->operand = NULL;
  pf_operand->reg = 0xffffffff;

  pf_operand->memory = TRUE;
  pf_operand->transparent = FALSE;
  pf_operand->unconditional = FALSE;

  HashTable_insert (pred_flow->hash_pf_operand, pf_operand->id, pf_operand);
  HashTable_insert (pred_flow->hash_pf_mem_operand, pf_oper->oper->id,
                    pf_operand);

  return (pf_operand);
}


static PF_OPERAND *
PF_new_dummy_pf_operand (PRED_FLOW * pred_flow, PF_OPER * pf_oper,
                         int reg, int transparent, int unconditional)
{
  PF_OPERAND *pf_operand;

  pf_operand = (PF_OPERAND *) L_alloc (PF_alloc_pf_operand);
  pf_operand->pf_oper = pf_oper;
  pred_flow->num_pf_operand++;
  pf_operand->id = pred_flow->num_pf_operand;
  pf_operand->operand = NULL;
  pf_operand->reg = reg;
  pf_operand->transparent = transparent;
  pf_operand->unconditional = unconditional;

  HashTable_insert (pred_flow->hash_pf_operand, pf_operand->id, pf_operand);
  pred_flow->reg_U = Set_add (pred_flow->reg_U, pf_operand->reg);
  pred_flow->pf_operand_U = Set_add (pred_flow->pf_operand_U, pf_operand->id);

  return (pf_operand);
}


static PF_OPERAND *
PF_delete_pf_operand (PF_OPERAND * pf_operand)
{
  if (pf_operand)
    L_free (PF_alloc_pf_operand, pf_operand);
  return (NULL);
}


static PRED_FLOW *
PF_new_pred_flow ()
{
  PRED_FLOW *new_pred_flow;

  new_pred_flow = (PRED_FLOW *) L_alloc (PF_alloc_pred_flow);
  new_pred_flow->hash_cb_pfcb = HashTable_create (256);
  new_pred_flow->hash_oper_pfoper = HashTable_create (1024);
  new_pred_flow->hash_pf_node = HashTable_create (256);
  new_pred_flow->hash_pf_operand = HashTable_create (1024);
  new_pred_flow->hash_pf_mem_operand = HashTable_create (1024);
  new_pred_flow->hash_pf_inst = HashTable_create (1024);

  new_pred_flow->list_pf_cb = NULL;
  new_pred_flow->list_pf_node = NULL;
  new_pred_flow->pf_node_root = NULL;

  new_pred_flow->pf_node_U = NULL;
  new_pred_flow->pf_operand_U = NULL;
  new_pred_flow->oper_U = NULL;
  new_pred_flow->reg_U = NULL;
  new_pred_flow->expression_U = NULL;
  new_pred_flow->fragile_U = NULL;
  new_pred_flow->mem_U = NULL;
  new_pred_flow->store_U = NULL;
  new_pred_flow->mem_stack_U = NULL;
  new_pred_flow->local_var_U = NULL;
  new_pred_flow->unpred_U = NULL;
  new_pred_flow->store_specific_U = NULL;
  new_pred_flow->pf_mem_dest_operand_U = NULL;
  new_pred_flow->pf_mem_src_operand_U = NULL;

  new_pred_flow->hash_RD_operand_def = NULL;
  new_pred_flow->hash_RD_operand_use = NULL;
  new_pred_flow->hash_mem_oper_conflict = NULL;
  new_pred_flow->hash_amb_mem_oper_conflict = NULL;

  new_pred_flow->num_pf_operand = 0;
  new_pred_flow->num_pf_inst = 0;

  return new_pred_flow;
}


static PRED_FLOW *
PF_delete_pred_flow (PRED_FLOW * pred_flow)
{
  if (!pred_flow)
    return (NULL);

  HashTable_free (pred_flow->hash_cb_pfcb);
  HashTable_free (pred_flow->hash_oper_pfoper);
  HashTable_free (pred_flow->hash_pf_node);
  HashTable_free (pred_flow->hash_pf_operand);
  HashTable_free (pred_flow->hash_pf_mem_operand);
  HashTable_free (pred_flow->hash_pf_inst);
  HashTable_free_func (pred_flow->hash_RD_operand_def,
                       (void (*)(void *)) Set_dispose);
  HashTable_free_func (pred_flow->hash_RD_operand_use,
                       (void (*)(void *)) Set_dispose);
  HashTable_free_func (pred_flow->hash_mem_oper_conflict,
                       (void (*)(void *)) Set_dispose);
  HashTable_free_func (pred_flow->hash_amb_mem_oper_conflict,
                       (void (*)(void *)) Set_dispose);
  pred_flow->pf_node_U = Set_dispose (pred_flow->pf_node_U);
  pred_flow->pf_operand_U = Set_dispose (pred_flow->pf_operand_U);
  List_reset (pred_flow->pf_mem_dest_operand_U);
  List_reset (pred_flow->pf_mem_src_operand_U);
  pred_flow->oper_U = Set_dispose (pred_flow->oper_U);
  pred_flow->reg_U = Set_dispose (pred_flow->reg_U);
  pred_flow->expression_U = Set_dispose (pred_flow->expression_U);
  pred_flow->fragile_U = Set_dispose (pred_flow->fragile_U);
  pred_flow->mem_U = Set_dispose (pred_flow->mem_U);
  pred_flow->store_U = Set_dispose (pred_flow->store_U);
  pred_flow->mem_stack_U = Set_dispose (pred_flow->mem_stack_U);
  pred_flow->local_var_U = Set_dispose (pred_flow->local_var_U);
  pred_flow->unpred_U = Set_dispose (pred_flow->unpred_U);
  pred_flow->store_specific_U = Set_dispose (pred_flow->store_specific_U);
  List_reset (pred_flow->list_pf_cb);
  List_reset (pred_flow->list_pf_node);
  L_free (PF_alloc_pred_flow, pred_flow);
  return (NULL);
}


PRED_FLOW *
PF_delete_flow_graph (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_OPER *pf_oper;
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
        {
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->src)
	    PF_delete_pf_operand (pf_operand);
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->dest)
	    PF_delete_pf_operand (pf_operand);
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_src)
	    PF_delete_pf_operand (pf_operand);
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_dest)
	    PF_delete_pf_operand (pf_operand);
          PF_delete_pf_oper (pf_oper);
        }
      PF_FOREACH_BB(pf_bb, pf_cb->pf_bbs)
        PF_delete_pf_bb (pf_bb);
      PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes)
        {
	  PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
	    PF_delete_pf_inst (pf_inst);
          PF_delete_pf_node (pf_node);
        }

      PF_delete_pf_cb (pf_cb);
    }
  PF_delete_pred_flow (pred_flow);

  if (PF_alloc_pf_cb)
    L_print_alloc_info (stderr, PF_alloc_pf_cb, 0);
  if (PF_alloc_pf_bb)
    L_print_alloc_info (stderr, PF_alloc_pf_bb, 0);
  if (PF_alloc_pf_oper)
    L_print_alloc_info (stderr, PF_alloc_pf_oper, 0);
  if (PF_alloc_pf_node)
    L_print_alloc_info (stderr, PF_alloc_pf_node, 0);
  if (PF_alloc_pf_inst)
    L_print_alloc_info (stderr, PF_alloc_pf_inst, 0);
  if (PF_alloc_pf_operand)
    L_print_alloc_info (stderr, PF_alloc_pf_operand, 0);
  if (PF_alloc_pred_flow)
    L_print_alloc_info (stderr, PF_alloc_pred_flow, 0);
  if (PF_alloc_pf_mem_set)
    L_print_alloc_info (stderr, PF_alloc_pf_mem_set, 0);

  if (D_alloc_df_pce_info)
    L_print_alloc_info (stderr, D_alloc_df_pce_info, 0);
  if (D_alloc_df_inst_info)
    L_print_alloc_info (stderr, D_alloc_df_inst_info, 0);
  if (D_alloc_df_oper_info)
    L_print_alloc_info (stderr, D_alloc_df_oper_info, 0);
  if (D_alloc_df_node_info)
    L_print_alloc_info (stderr, D_alloc_df_node_info, 0);
  if (D_alloc_df_bb_info)
    L_print_alloc_info (stderr, D_alloc_df_bb_info, 0);
  if (D_alloc_df_cb_info)
    L_print_alloc_info (stderr, D_alloc_df_cb_info, 0);

  return NULL;
}


void
PF_add_src_operand (PRED_FLOW * pred_flow, L_Oper * oper, int reg,
                    int transparent, int unconditional)
{
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;

  pf_oper = PF_FIND_OPER (pred_flow, oper->id);

  pf_operand = PF_new_dummy_pf_operand (pred_flow, pf_oper, reg,
                                        transparent, unconditional);
  pf_oper->src = List_insert_last (pf_oper->src, pf_operand);

  PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
    {
      if (unconditional || pf_inst->pred_true)
	pf_inst->src = List_insert_last (pf_inst->src, pf_operand);
    }

  if (unconditional)
    pf_oper->flags |= PF_UNCOND_SRC;
}


void
PF_add_dest_operand (PRED_FLOW * pred_flow, L_Oper * oper, int reg,
                     int transparent, int unconditional)
{
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;

  pf_oper = PF_FIND_OPER (pred_flow, oper->id);

  pf_operand = PF_new_dummy_pf_operand (pred_flow, pf_oper, reg,
                                        transparent, unconditional);
  pf_oper->dest = List_insert_last (pf_oper->dest, pf_operand);

  PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
    {
      if (unconditional || pf_inst->pred_true)
	pf_inst->dest = List_insert_last (pf_inst->dest, pf_operand);
    }

  if (unconditional)
    pf_oper->flags |= PF_UNCOND_DEST;
}


static void
PF_add_standard_operands (PRED_FLOW * pred_flow, PF_OPER * pf_oper,
                          L_Oper * oper, int do_operands)
{
  PF_OPERAND *pf_operand;
  L_Attr *attr;
  int indx;
  L_Operand *opd;

  if (do_operands & PF_PRED_OPERANDS)
    {
      if ((opd = oper->pred[0]))
        {
          pf_operand = PF_new_pf_operand (pred_flow, pf_oper, opd);
	  pf_operand->unconditional = TRUE;
	  pf_oper->flags |= PF_UNCOND_SRC;
          pf_oper->src = List_insert_last (pf_oper->src, pf_operand);
        }
    }

  for (indx = 0; indx < L_max_src_operand; indx++)
    {
      if (!(opd = oper->src[indx]) || !(L_is_reg (opd) || L_is_macro (opd)))
        continue;

      if ((L_is_ctype_predicate (opd) && !(do_operands & PF_PRED_OPERANDS)) ||
          (!L_is_ctype_predicate (opd) && !(do_operands & PF_STD_OPERANDS)))
        continue;

      if (L_is_macro (opd) && !M_dataflow_macro (opd->value.mac))
        continue;

      pf_operand = PF_new_pf_operand (pred_flow, pf_oper, opd);
      pf_oper->src = List_insert_last (pf_oper->src, pf_operand);

      if (pf_operand->unconditional)
	pf_oper->flags |= PF_UNCOND_SRC;
    }

  for (indx = 0; indx < L_max_dest_operand; indx++)
    {
      if (!(opd = oper->dest[indx]) || !(L_is_reg (opd) || L_is_macro (opd)))
        continue;

      if ((L_is_ctype_predicate (opd) && !(do_operands & PF_PRED_OPERANDS)) ||
          (!L_is_ctype_predicate (opd) && !(do_operands & PF_STD_OPERANDS)))
        continue;

      if (L_is_macro (opd) && !M_dataflow_macro (opd->value.mac))
        continue;

      pf_operand = PF_new_pf_operand (pred_flow, pf_oper, opd);
      pf_oper->dest = List_insert_last (pf_oper->dest, pf_operand);

      if (pf_operand->unconditional)
	pf_oper->flags |= PF_UNCOND_DEST;
    }

  if (!(do_operands & PF_STD_OPERANDS))
    return;

  if ((attr = L_find_attr (oper->attr, "tr")))
    {
      for (indx = 0; indx < attr->max_field; indx++)
        {
          pf_operand = PF_new_pf_operand (pred_flow, pf_oper,
                                          attr->field[indx]);
          pf_oper->src = List_insert_last (pf_oper->src, pf_operand);
        }
    }

  if ((attr = L_find_attr (oper->attr, "ret")))
    {
      for (indx = 0; indx < attr->max_field; indx++)
        {
          pf_operand = PF_new_pf_operand (pred_flow, pf_oper,
                                          attr->field[indx]);
          pf_oper->dest = List_insert_last (pf_oper->dest, pf_operand);
        }
    }

  if (L_general_store_opcode (oper))
    {
      pf_operand = PF_new_pf_memory_operand (pred_flow, pf_oper);
      pf_oper->mem_dest = List_insert_last (pf_oper->mem_dest, pf_operand);
      pred_flow->pf_mem_dest_operand_U =
        List_insert_last (pred_flow->pf_mem_dest_operand_U, pf_operand);
    }

  if (L_subroutine_call_opcode (oper))
    {
      pf_operand = PF_new_pf_memory_operand (pred_flow, pf_oper);
      pf_oper->mem_dest = List_insert_last (pf_oper->mem_dest, pf_operand);
      pred_flow->pf_mem_dest_operand_U =
        List_insert_last (pred_flow->pf_mem_dest_operand_U, pf_operand);

      /* Subroutines require SP to be up-to-date */

      pf_operand = PF_new_dummy_pf_operand (pred_flow, pf_oper,
					    L_MAC_INDEX (L_MAC_SP), 
					    FALSE, FALSE);
      pf_oper->src = List_insert_last (pf_oper->src, pf_operand);
    }

  if (L_general_load_opcode (oper))
    {
      pf_operand = PF_new_pf_memory_operand (pred_flow, pf_oper);
      pf_oper->mem_src = List_insert_last (pf_oper->mem_src, pf_operand);
      pred_flow->pf_mem_src_operand_U =
        List_insert_last (pred_flow->pf_mem_src_operand_U, pf_operand);
    }
}


/*
 * PRIMORDIAL FLOW GRAPH
 */


static PRED_FLOW *
PF_build_basic_flow_graph (L_Func *fn, int do_operands, int *max_pred_paths)
{
  L_Cb *cb;
  L_Oper *oper;
  PRED_FLOW *pred_flow;
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node;
  PF_OPER *pf_oper;

  pred_flow = PF_new_pred_flow ();
  pred_flow->fn = fn;

  /* 
   *  Create a PF_CB for each cb 
   *  Create a PF_OPER for each oper
   *  Create a root PF_BB for each PF_CB
   *  Create a root PF_NODE for each PF_CB
   *  Create a PF_OPERAND for each operand
   */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (max_pred_paths && 
	  (L_EXTRACT_BIT_VAL (cb->flags, L_CB_VIOLATES_LC_SEMANTICS) ||
	   (L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_CC_IN_PREDICATE_REGS) &&
	    !L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))))
	*max_pred_paths = 0;

      pf_cb = PF_new_pf_cb (pred_flow, cb);

      pf_node = PF_new_pf_node (pred_flow, pf_cb);
      pf_cb->pf_nodes_entry =
        List_insert_last (pf_cb->pf_nodes_entry, pf_node);

      pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
      pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);

      if (!pred_flow->pf_node_root)
        pred_flow->pf_node_root = pf_node;

      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
          pf_oper = PF_new_pf_oper (pred_flow, oper);
          pf_cb->pf_opers = List_insert_last (pf_cb->pf_opers, pf_oper);

          if (do_operands)
            PF_add_standard_operands (pred_flow, pf_oper, oper, do_operands);
        }
    }

  return pred_flow;
}

/*
 * BASIC BLOCK FLOW GRAPH
 */

static void
PF_bb_place_opers (PRED_FLOW * pred_flow, PF_NODE * pf_node, L_Oper * oper)
{
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;
  PF_INST *pf_inst;

  pf_oper = PF_FIND_OPER (pred_flow, oper->id);

  pf_inst = PF_new_pf_inst (pred_flow, pf_oper);
  pf_inst->pf_node = pf_node;
  pf_node->pf_insts = List_insert_last (pf_node->pf_insts, pf_inst);
  pf_oper->pf_insts = List_insert_last (pf_oper->pf_insts, pf_inst);

  pf_inst->pred_true = TRUE;

  PF_FOREACH_OPERAND(pf_operand, pf_oper->dest);
    pf_inst->dest = List_insert_last (pf_inst->dest, pf_operand);

  PF_FOREACH_OPERAND(pf_operand, pf_oper->src);
    pf_inst->src = List_insert_last (pf_inst->src, pf_operand);

  PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_dest);
    pf_inst->mem_dest = List_insert_last (pf_inst->mem_dest, pf_operand);

  PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_src);
    pf_inst->mem_src = List_insert_last (pf_inst->mem_src, pf_operand);
}


static PF_NODE *
PF_bb_branch (PRED_FLOW * pred_flow, PF_NODE * pf_node, L_Oper * oper)
{
  PF_NODE *new_pf_node;
  PF_NODE *dest_pf_node;
  PF_CB *dest_pf_cb;

  dest_pf_cb = PF_FIND_CB (pred_flow, (L_find_branch_dest (oper))->id);

  dest_pf_node = (PF_NODE *) List_first (dest_pf_cb->pf_nodes_entry);

  pf_node->succ = List_insert_first (pf_node->succ, dest_pf_node);
  dest_pf_node->pred = List_insert_first (dest_pf_node->pred, pf_node);

  if (L_cond_branch (oper))
    {
      if (oper->next_op)
        {
          new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);

          pf_node->succ = List_insert_first (pf_node->succ, new_pf_node);
          new_pf_node->pred = List_insert_first (new_pf_node->pred, pf_node);

          pf_node = new_pf_node;
        }
    }
  else
    pf_node = NULL;

  return pf_node;
}


static PF_NODE *
PF_bb_check_branch (PRED_FLOW * pred_flow, PF_NODE * pf_node, L_Oper * oper)
{
  PF_NODE *new_pf_node;
  PF_NODE *dest_pf_node;
  PF_CB *dest_pf_cb;

  dest_pf_cb = PF_FIND_CB (pred_flow, (L_find_branch_dest (oper))->id);

  dest_pf_node = (PF_NODE *) List_first (dest_pf_cb->pf_nodes_entry);

  pf_node->succ = List_insert_first (pf_node->succ, dest_pf_node);
  dest_pf_node->pred = List_insert_first (dest_pf_node->pred, pf_node);

  if (oper->next_op)
    {
      new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);

      pf_node->succ = List_insert_first (pf_node->succ, new_pf_node);
      new_pf_node->pred = List_insert_first (new_pf_node->pred, pf_node);

      pf_node = new_pf_node;
    }

  return pf_node;
}


static PF_NODE *
PF_bb_register_branch (PRED_FLOW * pred_flow, PF_NODE * pf_node,
                       L_Oper * oper)
{
  PF_NODE *dest_pf_node;
  PF_CB *dest_pf_cb;
  L_Flow *flow;

  for (flow = L_find_flow_for_branch (pf_node->pf_cb->cb, oper);
       flow != NULL; flow = flow->next_flow)
    {
      dest_pf_cb = PF_FIND_CB (pred_flow, flow->dst_cb->id);
      dest_pf_node = (PF_NODE *) List_first (dest_pf_cb->pf_nodes_entry);
      pf_node->succ = List_insert_first (pf_node->succ, dest_pf_node);
      dest_pf_node->pred = List_insert_first (dest_pf_node->pred, pf_node);
    }

  return NULL;
}


static PF_NODE *
PF_bb_return_branch (PRED_FLOW * pred_flow, PF_NODE * pf_node, L_Oper * oper)
{
  return NULL;
}


static PF_NODE *
PF_bb_complete_cb (PRED_FLOW * pred_flow, PF_NODE * pf_node)
{
  PF_NODE *dest_pf_node;
  PF_CB *pf_cb;

  if (pf_node->pf_cb->cb->next_cb)
    {
      pf_cb = PF_FIND_CB (pred_flow, pf_node->pf_cb->cb->next_cb->id);

      dest_pf_node = (PF_NODE *) List_first (pf_cb->pf_nodes_entry);

      pf_node->succ = List_insert_first (pf_node->succ, dest_pf_node);
      dest_pf_node->pred = List_insert_first (dest_pf_node->pred, pf_node);
    }

  return NULL;
}

PRED_FLOW *
PF_build_bb_graph (L_Func * fn, int do_operands)
{
  PRED_FLOW *pred_flow;
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *pf_last_node;
  L_Oper *oper;
  L_Cb *cb;

  pred_flow = PF_build_basic_flow_graph(fn, do_operands, NULL);

  /*  
   *  Now build a BB flow graph
   */

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      cb = pf_cb->cb;

      pf_bb = (PF_BB *) List_first (pf_cb->pf_bbs);
      pf_bb->first_op = cb->first_op;

      pf_node = (PF_NODE *) List_first (pf_cb->pf_nodes_entry);
      pf_bb->pf_nodes = List_insert_last (pf_bb->pf_nodes, pf_node);
      pf_bb->pf_nodes_entry = List_insert_last (pf_bb->pf_nodes_entry,
                                                pf_node);

      pf_last_node = pf_node;

      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
          if (!pf_node)
            pf_node = PF_new_pf_node (pred_flow, pf_cb);

          /* if new node, create a new pf_bb */
          if (pf_last_node != pf_node)
            {
              pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
              pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);
              pf_bb->first_op = oper;
              pf_bb->pf_nodes = List_insert_last (pf_bb->pf_nodes, pf_node);
              pf_bb->pf_nodes_entry = List_insert_last (pf_bb->pf_nodes_entry,
                                                        pf_node);
              pf_last_node = pf_node;
	    }

          PF_bb_place_opers (pred_flow, pf_node, oper);

          /* 
           * For branches, if pred is true then finish df_cb.
           * Also, draw destination arcs.
           */
          if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
            pf_node = PF_bb_branch (pred_flow, pf_node, oper);
          else if (L_register_branch_opcode (oper))
            pf_node = PF_bb_register_branch (pred_flow, pf_node, oper);
          else if (L_subroutine_return_opcode (oper))
            pf_node = PF_bb_return_branch (pred_flow, pf_node, oper);
          else if (L_check_branch_opcode (oper))
            pf_node = PF_bb_check_branch (pred_flow, pf_node, oper);
        }
      if (pf_node)
        pf_node = PF_bb_complete_cb (pred_flow, pf_node);
    }

  pred_flow->poison = FALSE;

  return pred_flow;
}


/*
 * END BASIC BLOCK FLOW GRAPH 
 */


/*
 * PREDICATE FLOW GRAPH CONSTRUCTION 
 */

static void
PF_place_opers (PRED_FLOW * pred_flow, List pf_node_list, L_Oper * oper)
{
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  L_Operand *pred;
  PG_Pred_SSA *pred_ssa;

  pf_oper = PF_FIND_OPER (pred_flow, oper->id);

  pred = oper->pred[0];

  pred_ssa = pred ? pred->value.pred.ssa : NULL;

  PF_FOREACH_NODE(pf_node, pf_node_list)
    {
      pf_inst = PF_new_pf_inst (pred_flow, pf_oper);
      pf_inst->pf_node = pf_node;
      pf_node->pf_insts = List_insert_last (pf_node->pf_insts, pf_inst);
      pf_oper->pf_insts = List_insert_last (pf_oper->pf_insts, pf_inst);

      if (!pred ||
	  (L_is_macro(pred) && !M_dataflow_macro((pred)->value.mac)) ||
          Set_in (pf_node->pred_true, pred_ssa->ssa_indx) ||
          !Set_in (pf_node->pred_def, pred_ssa->ssa_indx))
        pf_inst->pred_true = TRUE;

      if (!pred ||
	  (L_is_macro(pred) && !M_dataflow_macro((pred)->value.mac)) ||
	  Set_in (pf_node->pred_def, pred_ssa->ssa_indx))
	pf_inst->pred_known = TRUE;

      PF_FOREACH_OPERAND(pf_operand, pf_oper->dest)
        if (pf_operand->unconditional || pf_inst->pred_true)
          pf_inst->dest = List_insert_last (pf_inst->dest, pf_operand);

      PF_FOREACH_OPERAND(pf_operand, pf_oper->src)
        if (pf_operand->unconditional || pf_inst->pred_true)
          pf_inst->src = List_insert_last (pf_inst->src, pf_operand);

      PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_dest)
        if (pf_operand->unconditional || pf_inst->pred_true)
          pf_inst->mem_dest = List_insert_last (pf_inst->mem_dest, pf_operand);

      PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_src)
        if (pf_operand->unconditional || pf_inst->pred_true)
          pf_inst->mem_src = List_insert_last (pf_inst->mem_src, pf_operand);
    }
}


static List
PF_branch (PRED_FLOW * pred_flow, List pf_node_list, PF_BB * pf_bb, L_Oper * oper)
{
  PF_NODE *pf_node;
  PF_NODE *new_pf_node;
  PF_NODE *dest_pf_node;
  PF_CB *dest_pf_cb;
  L_Operand *pred;

  dest_pf_cb = PF_FIND_CB (pred_flow, (L_find_branch_dest (oper))->id);

  dest_pf_node = (PF_NODE *) List_first (dest_pf_cb->pf_nodes_entry);

  pred = oper->pred[0];

  PF_FOREACH_NODE(pf_node, pf_node_list)
    {
      if (!pred ||
          !Set_in (pf_node->pred_def, pred->value.pred.ssa->ssa_indx) ||
          Set_in (pf_node->pred_true, pred->value.pred.ssa->ssa_indx))
        {
          pf_node->succ = List_insert_first (pf_node->succ, dest_pf_node);
          dest_pf_node->pred =
            List_insert_first (dest_pf_node->pred, pf_node);

          /* If unconditional branch predicate is not known, 
             treat as conditional */
          if (L_cond_branch_opcode (oper) || L_check_branch_opcode (oper) ||
              (pred &&
               L_uncond_branch_opcode (oper) &&
               !Set_in (pf_node->pred_def, pred->value.pred.ssa->ssa_indx)))
            {
              if (oper->next_op)
                {
                  new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);

                  pf_node->succ =
                    List_insert_first (pf_node->succ, new_pf_node);
                  new_pf_node->pred =
                    List_insert_first (new_pf_node->pred, pf_node);

                  new_pf_node->pred_def = Set_union (pf_node->pred_def, NULL);
                  new_pf_node->pred_true =
                    Set_union (pf_node->pred_true, NULL);

                  pf_bb->pf_nodes = List_insert_first (pf_bb->pf_nodes,
                                                       new_pf_node);
                  pf_bb->pf_nodes_entry =
                    List_insert_first (pf_bb->pf_nodes_entry, new_pf_node);

                  pf_node_list =
                    List_insert_first (pf_node_list, new_pf_node);
                  pf_node_list = List_delete_current (pf_node_list);
                }
            }
          /* If predicate unknown it is caught above, otherwise predicate, 
             if any, is known to be true */
          else if (L_uncond_branch_opcode (oper))
            {
              pf_node_list = List_delete_current (pf_node_list);
            }
          else
            {
              L_punt ("PF_branch: Oper not branch opcode.");
            }
        }
    }
  return pf_node_list;
}


/* 
 * Doesn't handle predicated register branches. Easy to add later. Probably
 * not supported in other parts of IMPACT, anyway.
 */
static List
PF_register_branch (PRED_FLOW * pred_flow, List pf_node_list, L_Oper * oper)
{
  PF_NODE *pf_node;
  PF_NODE *dest_pf_node;
  PF_CB *dest_pf_cb;
  L_Flow *flow;

  PF_FOREACH_NODE(pf_node, pf_node_list)
    {
      for (flow = L_find_flow_for_branch (pf_node->pf_cb->cb, oper);
           flow != NULL; flow = flow->next_flow)
        {
          dest_pf_cb = PF_FIND_CB (pred_flow, flow->dst_cb->id);
          dest_pf_node = (PF_NODE *) List_first (dest_pf_cb->pf_nodes_entry);
          pf_node->succ = List_insert_first (pf_node->succ, dest_pf_node);
          dest_pf_node->pred =
            List_insert_first (dest_pf_node->pred, pf_node);
        }
    }
  pf_node_list = List_reset (pf_node_list);
  return pf_node_list;
}


/* 
 * Doesn't handle predicated returns now.  Easy to add later.  Probably
 * not supported in other parts of IMPACT, anyway.
 */
static List
PF_return_branch (PRED_FLOW * pred_flow, List pf_node_list, L_Oper * oper)
{
  pf_node_list = List_reset (pf_node_list);
  return pf_node_list;
}


static List
PF_split_pf_nodes (PRED_FLOW * pred_flow, List pf_node_list, PF_BB *pf_bb,
                   PG_Pred_SSA * pred_ssa)
{
  PF_NODE *pf_node;
  PF_NODE *new_pf_node;

  PF_FOREACH_NODE(pf_node, pf_node_list)
    {
      if (!Set_in (pf_node->pred_def, pred_ssa->ssa_indx))
        {
          int possible = PG_possible_values (pred_ssa, pf_node->pred_def,
                                             pf_node->pred_true);

#if DEBUG_PRED_FLOW
          printf ("RETURNING %d for %d\n", possible, pred_ssa->ssa_indx);
          Set_print (stdout, "DEF ", pf_node->pred_def);
          Set_print (stdout, "TRU ", pf_node->pred_true);
          if (possible == 2)
            printf ("SPLITTING!!!! Was %d paths.\n",
                    List_size (pf_node_list));
#endif
          switch (possible)
            {
            case -1:
	      /* The specified combination of values is not possible */
	      pf_node_list = List_delete_current (pf_node_list);
	      break;
            case 0:
	      /* The specified predicate must be false */
	      /* Copy pf_node but with new False predicate */
	      new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);
              pf_bb->pf_nodes = List_insert_last(pf_bb->pf_nodes, new_pf_node);
	      pf_node_list = List_insert_first (pf_node_list, new_pf_node);
	      new_pf_node->pred =
		List_insert_first (new_pf_node->pred, pf_node);
	      pf_node->succ =
		List_insert_first (pf_node->succ, new_pf_node);
	      
	      new_pf_node->pred_def = Set_union (pf_node->pred_def, NULL);
	      new_pf_node->pred_true = Set_union (pf_node->pred_true, NULL);
	      new_pf_node->pred_def =
		Set_add (new_pf_node->pred_def, pred_ssa->ssa_indx);
	      
	      pf_node_list = List_delete_current (pf_node_list);
	      break;
            case 1:
	      /* The specified predicate must be true */
	      /* Copy pf_node but with new True predicate */
	      new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);
              pf_bb->pf_nodes = List_insert_last(pf_bb->pf_nodes, new_pf_node);
	      pf_node_list = List_insert_first (pf_node_list, new_pf_node);
	      new_pf_node->pred =
		List_insert_first (new_pf_node->pred, pf_node);
	      pf_node->succ =
		List_insert_first (pf_node->succ, new_pf_node);
	      
	      new_pf_node->pred_def = Set_union (pf_node->pred_def, NULL);
	      new_pf_node->pred_true = Set_union (pf_node->pred_true, NULL);
	      new_pf_node->pred_def =
		Set_add (new_pf_node->pred_def, pred_ssa->ssa_indx);
	      new_pf_node->pred_true =
		Set_add (new_pf_node->pred_true, pred_ssa->ssa_indx);
	      
	      pf_node_list = List_delete_current (pf_node_list);
	      break;
            case 2:
	      /* The specified predicate may be either true or false */
	      /* Copy pf_node but with new True predicate */
	      new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);
              pf_bb->pf_nodes = List_insert_last(pf_bb->pf_nodes, new_pf_node);
	      pf_node_list = List_insert_first (pf_node_list, new_pf_node);
	      new_pf_node->pred =
		List_insert_first (new_pf_node->pred, pf_node);
	      pf_node->succ =
		List_insert_first (pf_node->succ, new_pf_node);
	      
	      new_pf_node->pred_def = Set_union (pf_node->pred_def, NULL);
	      new_pf_node->pred_true = Set_union (pf_node->pred_true, NULL);
	      new_pf_node->pred_def =
		Set_add (new_pf_node->pred_def, pred_ssa->ssa_indx);
	      new_pf_node->pred_true =
		Set_add (new_pf_node->pred_true, pred_ssa->ssa_indx);
	      
	      /* Copy pf_node but with new False predicate */
	      new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);
              pf_bb->pf_nodes = List_insert_last(pf_bb->pf_nodes, new_pf_node);
	      pf_node_list = List_insert_first (pf_node_list, new_pf_node);
	      new_pf_node->pred =
		List_insert_first (new_pf_node->pred, pf_node);
	      pf_node->succ =
		List_insert_first (pf_node->succ, new_pf_node);
	      
	      new_pf_node->pred_def = Set_union (pf_node->pred_def, NULL);
	      new_pf_node->pred_true = Set_union (pf_node->pred_true, NULL);
	      new_pf_node->pred_def =
		Set_add (new_pf_node->pred_def, pred_ssa->ssa_indx);
	      
	      pf_node_list = List_delete_current (pf_node_list);
	      break;

            default:
              L_punt ("PG_possible_values returned %d", possible);
            }
        }
    }
  return pf_node_list;
}

static List
PF_merge_pf_nodes (PRED_FLOW * pred_flow, List pf_node_list, PF_BB *pf_bb,
                   Set preds_to_retire)
{
  PF_NODE *new_pf_node;
  PF_NODE *first_pf_node;
  PF_NODE *second_pf_node;
  PF_NODE *pf_node;
  int list_ptr;
  int change;
  Set new_pred_def;
  Set new_pred_true;
  Set temp_pred_def;
  Set temp_pred_true;

  if (!pf_node_list || Set_empty (preds_to_retire))
    return pf_node_list;

  /*
   * First Merge all nodes that would be the same after retired preds are gone.
   */
  list_ptr = List_register_new_ptr (pf_node_list);

  change = 1;
  while (change)
    {
      change = 0;
      PF_FOREACH_NODE(first_pf_node, pf_node_list)
        {
          List_copy_current_ptr (pf_node_list, list_ptr, 0);
          while ((second_pf_node =
                  (PF_NODE *) List_next_l (pf_node_list, list_ptr)))
            {
              new_pred_def =
                Set_subtract (first_pf_node->pred_def, preds_to_retire);
              new_pred_true =
                Set_subtract (first_pf_node->pred_true, preds_to_retire);

              temp_pred_true =
                Set_subtract (second_pf_node->pred_true, preds_to_retire);
              temp_pred_def =
                Set_subtract (second_pf_node->pred_def, preds_to_retire);

              if (Set_same (new_pred_true, temp_pred_true) &&
                  Set_same (new_pred_def, temp_pred_def))
                {
                  if (Set_same (new_pred_true, second_pf_node->pred_true) &&
                      Set_same (new_pred_def, second_pf_node->pred_def))
                    {
                      first_pf_node->succ =
                        List_insert_first (first_pf_node->succ,
                                           second_pf_node);
                      second_pf_node->pred =
                        List_insert_first (second_pf_node->pred,
                                           first_pf_node);
                      pf_node_list = List_delete_current (pf_node_list);

                      new_pred_def = Set_dispose (new_pred_def);
                      new_pred_true = Set_dispose (new_pred_true);
                    }
                  else if (Set_same (new_pred_true, first_pf_node->pred_true)
                           && Set_same (new_pred_def,
                                        first_pf_node->pred_def))
                    {
                      second_pf_node->succ =
                        List_insert_first (second_pf_node->succ,
                                           first_pf_node);
                      first_pf_node->pred =
                        List_insert_first (first_pf_node->pred,
                                           second_pf_node);

                      pf_node_list =
                        List_delete_current_l (pf_node_list, list_ptr);

                      new_pred_def = Set_dispose (new_pred_def);
                      new_pred_true = Set_dispose (new_pred_true);
                    }
                  else
                    {
                      new_pf_node =
                        PF_new_pf_node (pred_flow, first_pf_node->pf_cb);
                      pf_bb->pf_nodes =
                        List_insert_last(pf_bb->pf_nodes, new_pf_node);
                      pf_node_list =
                        List_insert_first (pf_node_list, new_pf_node);

                      first_pf_node->succ =
                        List_insert_first (first_pf_node->succ, new_pf_node);
                      new_pf_node->pred =
                        List_insert_first (new_pf_node->pred, first_pf_node);
                      second_pf_node->succ =
                        List_insert_first (second_pf_node->succ, new_pf_node);
                      new_pf_node->pred =
                        List_insert_first (new_pf_node->pred, second_pf_node);

                      new_pf_node->pred_def = 
                        Set_dispose (new_pf_node->pred_def);
                      new_pf_node->pred_true = 
                        Set_dispose (new_pf_node->pred_true);
                      new_pf_node->pred_def = new_pred_def;
                      new_pf_node->pred_true = new_pred_true;

                      pf_node_list = List_delete_current (pf_node_list);
                      pf_node_list =
                        List_delete_current_l (pf_node_list, list_ptr);
                    }
                  change++;

#if DEBUG_PRED_FLOW
                  /* MERGE THESE TWO PF_NODES */
                  printf ("MERGE CBS %d and %d to create %d\n",
                          first_pf_node->id, second_pf_node->id,
                          new_pf_node->id);
                  Set_print (stdout, "\tDEF1", first_pf_node->pred_def);
                  Set_print (stdout, "\tTRUE1", first_pf_node->pred_true);
                  Set_print (stdout, "\tDEF1", second_pf_node->pred_def);
                  Set_print (stdout, "\tTRUE1", second_pf_node->pred_true);
#endif
                  temp_pred_true = Set_dispose (temp_pred_true);
                  temp_pred_def = Set_dispose (temp_pred_def);
                  break;
                }
              else
                {
                  new_pred_def = Set_dispose (new_pred_def);
                  new_pred_true = Set_dispose (new_pred_true);
                  temp_pred_true = Set_dispose (temp_pred_true);
                  temp_pred_def = Set_dispose (temp_pred_def);
                }
            }
        }
    }
  List_free_all_ptrs (pf_node_list);

  /*
   * Retired preds should not appear in any pred flow nodes
   */
  PF_FOREACH_NODE(pf_node, pf_node_list)
    {
      new_pred_def = Set_subtract (pf_node->pred_def, preds_to_retire);
      new_pred_true = Set_subtract (pf_node->pred_true, preds_to_retire);

      if (!Set_same (pf_node->pred_def, new_pred_def) ||
          !Set_same (pf_node->pred_true, new_pred_true))
        {
          new_pf_node = PF_new_pf_node (pred_flow, pf_node->pf_cb);
          pf_node_list = List_insert_first (pf_node_list, new_pf_node);

          pf_node->succ = List_insert_first (pf_node->succ, new_pf_node);
          new_pf_node->pred = List_insert_first (new_pf_node->pred, pf_node);

          new_pf_node->pred_def = new_pred_def;
          new_pf_node->pred_true = new_pred_true;

          pf_node_list = List_delete_current (pf_node_list);
        }
      else
        {
          new_pred_def = Set_dispose (new_pred_def);
          new_pred_true = Set_dispose (new_pred_true);
        }
    }
  return pf_node_list;
}

static List
PF_complete_cb (PRED_FLOW * pred_flow, List pf_node_list)
{
  PF_NODE *pf_node;
  PF_NODE *dest_pf_node;
  PF_CB *dest_pf_cb;

  pf_node = (PF_NODE *) List_first (pf_node_list);
  if (pf_node->pf_cb->cb->next_cb)
    {
      dest_pf_cb = PF_FIND_CB (pred_flow, pf_node->pf_cb->cb->next_cb->id);
      dest_pf_node = (PF_NODE *) List_first (dest_pf_cb->pf_nodes_entry);

      PF_FOREACH_NODE(pf_node, pf_node_list)
        {
          pf_node->succ = List_insert_first (pf_node->succ, dest_pf_node);
          dest_pf_node->pred =
            List_insert_first (dest_pf_node->pred, pf_node);
        }
    }
  pf_node_list = List_reset (pf_node_list);

  return pf_node_list;
}


/*
 * PREDICATE FLOW GRAPH CONSTRUCTION 
 */


PRED_FLOW *
PF_build_pred_flow_graph (L_Func * fn, int do_operands)
{
  PRED_FLOW *pred_flow;
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node;
  PF_OPER *pf_oper;
  L_Oper *oper;
  L_Cb *cb;
  List pf_node_list;
  PG_Pred_Graph *pg;

  Set preds_live = NULL;
  Set preds_zombie = NULL;
  Set preds_to_retire = NULL;
  Set preds_killed = NULL;
  Set preds_active = NULL;
  int num_preds;
  int indx;

  int *buf;
  int *last_use;

  int max_pred_paths = L_df_max_pred_paths;

  pg = PG_pred_graph;

  pred_flow = PF_build_basic_flow_graph(fn, do_operands, &max_pred_paths);

  /*  
   *  Now build a pred flow graph
   */
  pf_node_list = NULL;

  if (pg && pg->max_ssa_indx)
    {
      buf = alloca(pg->max_ssa_indx * sizeof(int));
      last_use = alloca(pg->max_ssa_indx * sizeof(int));
    }
  else
    {
      buf = NULL;
      last_use = NULL;
    }

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
#ifdef DEBUG_PFG_PATHS
      int warned = 0;
#endif
      preds_live = Set_dispose (preds_live);
      preds_zombie = Set_dispose (preds_zombie);
      preds_active = Set_dispose (preds_active);

      cb = pf_cb->cb;

      pf_bb = (PF_BB *) List_first (pf_cb->pf_bbs);
      pf_bb->first_op = cb->first_op;

      pf_node_list = List_reset (pf_node_list);
      pf_node = (PF_NODE *) List_first (pf_cb->pf_nodes_entry);
      pf_node_list = List_insert_first (pf_node_list, pf_node);
      pf_bb->pf_nodes = List_insert_first (pf_bb->pf_nodes, pf_node);
      pf_bb->pf_nodes_entry = List_insert_first (pf_bb->pf_nodes_entry,
                                                 pf_node);

      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  if (oper->pred[0] && !L_is_rregister (oper->pred[0]))
	    {
	      preds_live =
		Set_add (preds_live, oper->pred[0]->value.pred.ssa->ssa_indx);
	      last_use[oper->pred[0]->value.pred.ssa->ssa_indx] = oper->id;
	    }
	}

      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
	  L_Operand *pred = NULL;

          if (!List_size (pf_node_list))
            {
              pf_node = PF_new_pf_node (pred_flow, pf_cb);
              pf_node_list = List_insert_first (pf_node_list, pf_node);
              pf_bb->pf_nodes = List_insert_first (pf_bb->pf_nodes, pf_node);
              pf_bb->pf_nodes_entry = List_insert_first (pf_bb->pf_nodes_entry,
                                                         pf_node);
            }
	  
	  if ((pred = oper->pred[0]))
	    {
	      PG_Pred_SSA *ssa = pred->value.pred.ssa;

	      /* SPLIT ON APPEARANCE OF NEW PREDICATE */

	      if (!L_is_rregister (pred) &&
		  !Set_in (preds_active, ssa->ssa_indx))
		{
		  if (max_pred_paths == -1 || 
		      (List_size (pf_node_list) < max_pred_paths))
		    {
		      pf_node_list =
			PF_split_pf_nodes (pred_flow, pf_node_list, pf_bb, ssa);
		      preds_active =
			Set_add (preds_active, ssa->ssa_indx);
		    }
#ifdef DEBUG_PFG_PATHS
		  else if (!warned)
		    {
		      fprintf (stderr, ">PFG> Exhausted %d allowable paths "
			       "(%s cb %d)\n", max_pred_paths,
			       fn->name, cb->id);
		      warned = 1;
		    }
#endif
		}

	      /* MERGE ON LAST USE OF RELATED PREDICATE */
	      
	      if ((List_size (pf_node_list) > 1) &&
		  (last_use[ssa->ssa_indx] == oper->id))
		{
		  preds_live = Set_delete (preds_live, ssa->ssa_indx);
		  preds_zombie = Set_add (preds_zombie, ssa->ssa_indx);

		  if ((num_preds = Set_size (preds_zombie)))
		    {
		      Set_2array (preds_zombie, buf);
		      
		      for (indx = 0; indx < num_preds; indx++)
			{
			  if (!PG_effected_by (buf[indx], preds_live))
			    {
			      preds_to_retire =
				Set_add (preds_to_retire, buf[indx]);
			      preds_zombie =
				Set_delete (preds_zombie, buf[indx]);
			    }
			}
		    }
		}
	    }

          PF_place_opers (pred_flow, pf_node_list, oper);

          /* 
           * For branches, if pred is true then finish df_cb.
           * Also, draw destination arcs.
           */
          if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper)
              || L_check_branch_opcode (oper))
            {
              if (oper->next_op)
                {
                  pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
                  pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);
                  pf_bb->first_op = oper->next_op;
                }
              pf_node_list = PF_branch (pred_flow, pf_node_list, pf_bb, oper);
            }
          else if (L_register_branch_opcode (oper))
            {
              if (oper->next_op)
                {
                  pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
                  pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);
                  pf_bb->first_op = oper->next_op;
                }
              pf_node_list = PF_register_branch (pred_flow, pf_node_list, oper);
            }
          else if (L_subroutine_return_opcode (oper))
            {
              if (oper->next_op)
                {
                  pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
                  pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);
                  pf_bb->first_op = oper->next_op;
                }
              pf_node_list = PF_return_branch (pred_flow, pf_node_list, oper);
            }

#if DEBUG_PRED_FLOW
          if (!Set_empty (preds_to_retire))
            {
              Set_print (stdout, "\tRETIRE", preds_to_retire);
              printf ("  CB ID = %d, OPER ID = %d:\n  BEFORE\n", cb->id,
                      oper->id);
              PF_node_list (pf_node_list);
            }
#endif

          pf_node_list =
            PF_merge_pf_nodes (pred_flow, pf_node_list, pf_bb, preds_to_retire);

#if DEBUG_PRED_FLOW
          if (!Set_empty (preds_to_retire))
            {
              printf ("  AFTER\n");
              PF_node_list (pf_node_list);
            }
#endif
          preds_to_retire = Set_dispose (preds_to_retire);
        }

      if (pf_node_list)
        {
          /* This means that there is a fall through path. */
          if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
	    pf_node_list = PF_complete_cb (pred_flow, pf_node_list);
#ifdef DEBUG_PFG_PATHS
          else
            fprintf (stderr, ">PFG> Applied NO_FALLTHRU hint (%s cb %d)\n", 
		     fn->name, cb->id);
#endif
        }
    }

  preds_live = Set_dispose (preds_live);
  preds_zombie = Set_dispose (preds_zombie);
  preds_active = Set_dispose (preds_active);
  preds_killed = Set_dispose (preds_killed);
  preds_to_retire = Set_dispose (preds_to_retire);

   /* 
    * This portion of the code finds dead code not in pred flow graph
    */
 
  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
	{
	  PF_INST *pf_inst = NULL; 

	  if (L_pred_define_opcode(pf_oper->oper))
	    continue;

	  PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
	    if (pf_inst->pred_true)
	      break;
 
	  if (!pf_inst)
	    {
	      L_Attr *attr;
	      /* Mark oper for complete deletion */

	      if (!L_find_attr(pf_oper->oper->attr, DF_DEAD_CODE_ATTR))
		{
		  attr = L_new_attr (DF_DEAD_CODE_ATTR, 0);
		  pf_oper->oper->attr = 
		    L_concat_attr (pf_oper->oper->attr, attr);
		}
	    }
	}
    }

  pred_flow->poison = FALSE;

  return pred_flow;
}


/*
 * BEGIN MAXIMUM-PREDICATE-FLOW-GRAPH CONSTRUCTION 
 */


PRED_FLOW *
PF_build_max_pred_flow_graph (L_Func * fn, int do_operands)
{
  PRED_FLOW *pred_flow;
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node;
  L_Oper *oper;
  L_Cb *cb;
  List pf_node_list;

  int max_pred_paths = L_df_use_max_graph_builder;

  pred_flow = PF_build_basic_flow_graph(fn, do_operands, &max_pred_paths);

  /*  
   *  Now build a pred flow graph
   */
  pf_node_list = NULL;
  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      cb = pf_cb->cb;

      pf_bb = (PF_BB *) List_first (pf_cb->pf_bbs);
      pf_bb->first_op = cb->first_op;

      pf_node_list = List_reset (pf_node_list);
      pf_node = (PF_NODE *) List_first (pf_cb->pf_nodes_entry);
      pf_node_list = List_insert_first (pf_node_list, pf_node);
      pf_bb->pf_nodes = List_insert_first (pf_bb->pf_nodes, pf_node);
      pf_bb->pf_nodes_entry = List_insert_first (pf_bb->pf_nodes_entry,
                                                 pf_node);

      /*
       * First split all
       */

      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
	  PG_Pred_SSA *ssa;

	  if (oper->pred[0] && !L_is_rregister(oper->pred[0]))
	    ssa = oper->pred[0]->value.pred.ssa;
	  else
	    continue;

          /* Split nodes if there is capacity available */

          if ((max_pred_paths == -1 ||
               (List_size (pf_node_list) < max_pred_paths)))
	    pf_node_list = PF_split_pf_nodes (pred_flow, pf_node_list, pf_bb, 
                                              ssa);
        }

      /* 
       * Now process all opers
       */
      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
          PF_place_opers (pred_flow, pf_node_list, oper);

          /* 
           * For branches, if pred is true then finish df_cb.
           * Also, draw destination arcs.
           */
          if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper)
              || L_check_branch_opcode (oper))
            {
              if (oper->next_op)
                {
                  pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
                  pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);
                  pf_bb->first_op = oper->next_op;
                }
              pf_node_list = PF_branch (pred_flow, pf_node_list, pf_bb, oper);
            }
          else if (L_register_branch_opcode (oper))
            {
             if (oper->next_op)
                {
                  pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
                  pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);
                  pf_bb->first_op = oper->next_op;
                }
              pf_node_list = PF_register_branch (pred_flow, pf_node_list, oper);
            }
          else if (L_subroutine_return_opcode (oper))
            {
              if (oper->next_op)
                {
                  pf_bb = PF_new_pf_bb (pred_flow, pf_cb);
                  pf_cb->pf_bbs = List_insert_last (pf_cb->pf_bbs, pf_bb);
                  pf_bb->first_op = oper->next_op;
                }
              pf_node_list = PF_return_branch (pred_flow, pf_node_list, oper);
            }
        }

      /*
       * Anything left is fall through
       */

      if (pf_node_list)
        {
          /* This means that there is a fall through path. */
          if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK_NO_FALLTHRU))
            pf_node_list = PF_complete_cb (pred_flow, pf_node_list);
#ifdef DEBUG_PFG_PATHS
          else
            fprintf (stderr, ">PFG> Applied NO_FALLTHRU hint (%s cb %d)\n", 
		     fn->name, cb->id);
#endif
        }
    }

  pred_flow->poison = FALSE;

  return pred_flow;
}


/*
 * END MAXIMUM-PREDICATE-FLOW-GRAPH CONSTRUCTION 
 */

/*
 *
 * GENERAL FLOW GRAPH UTILITIES
 *
 */

void
PF_test (L_Func * fn)
{
#if 0

  PRED_FLOW *pf_graph;

  PF_initialize ();

  pf_graph = PF_build_graph (fn);

  file = L_open_output_file ("DF.davinci");
  D_daVinci_df_graph (file);
  L_close_output_file (file);

  D_build_comp_dataflow_graph (fn);

  D_count_dominator ();
  D_count_paths ();

  file = L_open_output_file ("DFC.davinci");
  D_daVinci_df_comp_graph (file);
  L_close_output_file (file);

  List_start (dataflow_comp_cbs);
  while ((df_cb = List_next (dataflow_comp_cbs)))
    {
      /* if(!List_size(df_cb->succ)) */
      {
        printf
          ("************************************************************\n");
        printf ("%d NUMBER OF PATHS = %d", df_cb->id, D_num_paths (df_cb));
        printf
          ("************************************************************\n");
      }
    }

  L_stop_time (&L_module_global_dataflow_time);

#endif

  return;
}

void
PF_daVinci_visit (PRED_FLOW * pred_flow, PF_NODE * pf_node, FILE * file)
{
  PF_NODE *succ_pf_node;

  if (L_EXTRACT_BIT_VAL (pf_node->flags, PF_VISITED))
    {
      fprintf (file, "r(\"DF %d, CB %d - %d\")", pf_node->id,
               pf_node->pf_cb->cb->id, pf_node->count);
      return;
    }
  pf_node->flags = L_SET_BIT_FLAG (pf_node->flags, PF_VISITED);

  /* Define Object */
  fprintf (file, "l(\"DF %d, CB %d - %d\",", pf_node->id,
           pf_node->pf_cb->cb->id, pf_node->count);
  fprintf (file, "n(\"anything\", [a(\"OBJECT\", \"DF %d, CB %d - %d\")],[",
           pf_node->id, pf_node->pf_cb->cb->id, pf_node->count);

#if 0
  printf ("DF_CB->id %d\n", pf_node->id);
  Set_print (stdout, "\tDEF", pf_node->pred_def);
  Set_print (stdout, "\tTRUE", pf_node->pred_true);
#endif

  /* Define Edges */

  PF_FOREACH_NODE(succ_pf_node, pf_node->succ)
    {
      fprintf (file, "e(\"anything\",[");
      if (!succ_pf_node->info->dom ||
          Set_in (pf_node->info->dom, succ_pf_node->id))
        fprintf (file, "a(\"EDGECOLOR\",\"red\")");
      else
        fprintf (file, "a(\"EDGECOLOR\",\"black\")");
      fprintf (file, "],");
      PF_daVinci_visit (pred_flow, succ_pf_node, file);
      fprintf (file, ")");
      if (List_get_next (pf_node->succ))
        {
          fprintf (file, ",");
        }
    }

  /* Define Object */
  fprintf (file, "]))");
}


void
DEBUG_spit_PF (PRED_FLOW * pred_flow, char *name)
{
  FILE *F;
  F = L_open_output_file (name);
  PF_daVinci (F, pred_flow);
  L_close_output_file (F);
}


void
PF_daVinci (FILE * file, PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;

  fprintf (file, "[\n");

  /* Clear the visited bit flag */

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    pf_node->flags = L_CLR_BIT_FLAG (pf_node->flags, PF_VISITED);

  pf_node = (PF_NODE *) List_first (pred_flow->list_pf_node);
  PF_daVinci_visit (pred_flow, pf_node, file);

  fprintf (file, "]\n");
}


static void
PF_count_paths_visit (PF_NODE * pf_node)
{
  PF_NODE *succ_pf_node;

  if (pf_node->flags)
    return;

  /* Define Edges */

  PF_FOREACH_NODE(succ_pf_node, pf_node->succ)
    {
      /* Don't cross backedges */
      if (!Set_in (pf_node->info->dom, succ_pf_node->id))
        {
          succ_pf_node->flags--;
          succ_pf_node->count += pf_node->count;
          PF_count_paths_visit (succ_pf_node);
        }
    }
}

void
PF_count_paths (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_NODE *pred_pf_node;
  PF_NODE *root_pf_node;

  /* Initialize the predecessor count */

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      pf_node->flags = 0;
      pf_node->count = 0;
    }
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      PF_FOREACH_NODE(pred_pf_node, pf_node->pred)
        {
          if (!Set_in (pred_pf_node->info->dom, pf_node->id))
	    pf_node->flags++;
        }
    }

  root_pf_node = (PF_NODE *) List_first (pred_flow->list_pf_node);
  root_pf_node->count = 1;

  PF_count_paths_visit (root_pf_node);
}


void
PF_print_pred_flow (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_NODE *pf_node;

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      fprintf (stdout, " pf_node->id = %d\n", pf_node->id);
      Set_print (stdout, " DEF", pf_node->pred_def);
      Set_print (stdout, "TRUE", pf_node->pred_true);
    }
  fprintf (stdout, "\n\n\n\n");

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      fprintf (stdout, "CB id: %d\n\n", pf_cb->cb->id);
      PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
        {
          PF_INST *pf_inst;

          L_print_oper (stdout, pf_oper->oper);

          fprintf (stdout, "  %d PFNODES []=dead:  ",
                   List_size (pf_oper->pf_insts));

	  PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
            {
              if (L_EXTRACT_BIT_VAL (pf_inst->flags, PF_DEAD_CODE))
		fprintf (stdout, " [%d]", pf_inst->pf_node->id);
              else
		fprintf (stdout, " %d", pf_inst->pf_node->id);
            }
          fprintf (stdout, "\n\n");
        }
    }
}


int
PF_print_operands (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
        {
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->src)
            {
              printf ("oper %d, operand %d  ", pf_oper->oper->id,
                      pf_operand->id);
              if (pf_operand->operand)
                L_print_operand (stdout, pf_operand->operand, 0);
              printf ("\n");
            }
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->dest)
            {
              printf ("oper %d, operand %d  ", pf_oper->oper->id,
                      pf_operand->id);
              if (pf_operand->operand)
                L_print_operand (stdout, pf_operand->operand, 0);
              printf ("\n");
            }
        }
    }
  return 0;
}

void
PF_node_list (List pf_node_list)
{
  PF_NODE *pf_node;

  PF_FOREACH_NODE(pf_node, pf_node_list)
    {
      printf ("PF_NODE %d: \n", pf_node->id);
      Set_print (stdout, "TRUE", pf_node->pred_true);
      Set_print (stdout, " DEF", pf_node->pred_def);
    }
}


/*
 * PARTIAL DEAD CODE REMOVAL
 * ---------------------------------------------------------------------------
 */

void
PF_clear_partial_dead_code_markings (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  L_Cb *cb;
  L_Oper *oper;

  /* 
   * Clear all PF_DEAD_CODE flags for next step.
   */

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        pf_inst->flags = L_CLR_BIT_FLAG (pf_inst->flags, PF_DEAD_CODE);
    }

  for (cb = pred_flow->fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
          L_delete_operand (oper->pred[1]);
          oper->pred[1] = NULL;
        }
    }
  return;
}


#if 0
static void
PF_mark_dead_inst (PF_INST * pf_inst)
{
  PF_OPERAND *pf_operand;

  pf_inst->flags = L_SET_BIT_FLAG (pf_inst->flags, PF_DEAD_CODE);

  /* Remove operands */

  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
    pf_inst->dest = List_delete_current (pf_inst->dest);

  PF_FOREACH_OPERAND(pf_operand, pf_inst->src)
    if (!pf_operand->unconditional)
      pf_inst->src = List_delete_current (pf_inst->src);

  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_dest)
    pf_inst->mem_dest = List_delete_current (pf_inst->mem_dest);

  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_src)
    if (!pf_operand->unconditional)
      pf_inst->mem_src = List_delete_current (pf_inst->mem_src);

  return;
}
#endif

void
PF_partial_dead_code_removal (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PG_Pred_SSA *pg_pred_ssa;
  int *pred_true_array;
  int *usage_count;
  int num_true_preds;
  int indx;
  int min_pred_indx;
  Set preds_true;

  if (pred_flow->num_pf_operand <= 0)
    return;

#if 0
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      PF_OPERAND *pf_operand;
      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
	{
	  PF_OPERAND *pf_operand;
	  if (L_EXTRACT_BIT_VAL (pf_inst->flags, PF_DEAD_CODE|PF_LIVE_CODE))
	    continue; /* already processed as dead, or is definitely
		       * alive (may be unquestionable) */

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
	    {
	      if (Set_in (pf_inst->info->v_out, pf_operand->reg))
		break;
	    }

	  if (!pf_operand)
	    PF_mark_dead_inst (pf_inst);
	}
    }
#endif

  pred_true_array =
    (int *) alloca (sizeof (int) * pred_flow->num_pf_operand);
  usage_count =
    (int *) alloca (sizeof (int) * pred_flow->num_pf_operand);

  /* 
   * This portion of the code computes and changes the new pred[1]
   */

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
        {
          int total_path = 0, total_live = 0;

          /*  
           * Compute set of predicates which are always true when
           * operation is live. 
           */
          preds_true = NULL;
	  PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
            {
              pf_node = pf_inst->pf_node;
              total_path++;
              if (!L_EXTRACT_BIT_VAL (pf_inst->flags, PF_DEAD_CODE))
                {
		  preds_true = total_live ?
		    Set_intersect_acc (preds_true, pf_node->pred_true) :
		    Set_union (NULL, pf_node->pred_true);

		  total_live++;
                }
            }

          /* 
           * Find tightest always true predicate 
           */

          if (!total_live)
            {
	      if ((List_size(pf_oper->dest) != 0) &&
		  L_safe_to_delete_opcode (pf_oper->oper))
		{
		  L_Attr *attr;
 
		  /* Mark oper for complete deletion */

		  if (!L_find_attr(pf_oper->oper->attr, DF_DEAD_CODE_ATTR))
		    {
		      attr = L_new_attr (DF_DEAD_CODE_ATTR, 0);
		      pf_oper->oper->attr = 
			L_concat_attr (pf_oper->oper->attr, attr);
		    }
		}

	      preds_true = Set_dispose (preds_true);

              L_delete_operand (pf_oper->oper->pred[1]);
              pf_oper->oper->pred[1] =
                L_copy_operand (pf_oper->oper->pred[0]);
              continue;
            }

          /* 
           * Clear count array and setup pred_true_array
           */

          num_true_preds = Set_2array (preds_true, pred_true_array);
          preds_true = Set_dispose (preds_true);

          if (num_true_preds == 0)
            {
              L_delete_operand (pf_oper->oper->pred[1]);
              pf_oper->oper->pred[1] =
                L_copy_operand (pf_oper->oper->pred[0]);
              continue;
            }

          for (indx = 0; indx < num_true_preds; indx++)
	    usage_count[indx] = 0;

          /*
           * Count usage for each pred through all paths
           */
	  PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
            {
              pf_node = pf_inst->pf_node;
              if (L_EXTRACT_BIT_VAL (pf_inst->flags, PF_DEAD_CODE))
                for (indx = 0; indx < num_true_preds; indx++)
                  if (Set_in (pf_node->pred_true, pred_true_array[indx]))
                    usage_count[indx]++;
            }

          /* 
           * Find minimum predicate
           */
          min_pred_indx = 0;
          for (indx = 0; indx < num_true_preds; indx++)
            if (usage_count[indx] < usage_count[min_pred_indx])
              min_pred_indx = indx;

#if 0
          printf ("OPER %d:\n", pf_oper->oper->id);
          Set_print (stdout, "  ALWAYS TRUE PREDS: ", preds_true);
          printf ("  Total: %d,  dead %d,      chosen %d\n", total_path,
                  total_path - total_live, buf[min]);
#endif

          /* 
           * Find the actual predicate number for the pred ssa
           * and place it in pred[1].
           */
          pg_pred_ssa =
            (PG_Pred_SSA *) HashTable_find (PG_pred_graph->hash_pgPredSSA,
                                            pred_true_array[min_pred_indx]);

	  if (L_IS_MAPPED_REG(pg_pred_ssa->pg_pred->pred))
	    pf_oper->oper->pred[1] =
	      L_new_register_operand (L_UNMAP_REG(pg_pred_ssa->pg_pred->pred),
				      L_CTYPE_PREDICATE, L_PTYPE_NULL);
	  else
	    pf_oper->oper->pred[1] =
	      L_new_macro_operand (L_UNMAP_MAC(pg_pred_ssa->pg_pred->pred),
				   L_CTYPE_PREDICATE, L_PTYPE_NULL);


          pf_oper->oper->pred[1]->value.pred.ssa = pg_pred_ssa;

          if (!L_safe_to_delete_opcode (pf_oper->oper) &&
              !L_uncond_branch_opcode (pf_oper->oper)
              && !L_cond_branch_opcode (pf_oper->oper)
              && !L_check_branch_opcode (pf_oper->oper))
            {
              L_delete_operand (pf_oper->oper->pred[1]);
              pf_oper->oper->pred[1] =
                L_copy_operand (pf_oper->oper->pred[0]);
            }
	  else if (L_pred_define_opcode (pf_oper->oper))
	    {
              L_delete_operand (pf_oper->oper->pred[1]);
              pf_oper->oper->pred[1] =
                L_copy_operand (pf_oper->oper->pred[0]);
	    }
	  else if (!PG_subset_predicate_ops_explicit (pf_oper->oper, TRUE,
						      pf_oper->oper, FALSE) &&
		   L_df_use_max_graph_builder != -1)
	    {
	      /* Sanity check - for low df_max_pred_path numbers */

#if 0
	      printf ("Correcting demoted value in Func %s Oper %d\n",
		      pred_flow->fn->name, pf_oper->oper->id);
#endif
	      L_delete_operand (pf_oper->oper->pred[1]);
	      pf_oper->oper->pred[1] =
		L_copy_operand (pf_oper->oper->pred[0]);
	    }
        }
    }

  return;
}


static int
L_pred_def_available (L_Cb * cb, L_Oper * use_oper, L_Operand * pred)
{
  PG_Pred_SSA *pred_ssa;
  L_Oper *oper;
  int i;

  if (!cb || !use_oper)
    L_punt ("L_pred_def_available: NULL operand");

  if (!pred)
    return TRUE;

  pred_ssa = pred->value.pred.ssa;

  if (!pred_ssa)
    return 0;

  for (oper = use_oper; oper; oper = oper->prev_op)
    {
      if (L_pred_define_opcode (oper))
        for (i = 0; i < L_max_dest_operand; i++)
          {
            if (L_same_operand (pred, oper->dest[i]))
              {
                if (pred_ssa == oper->dest[i]->value.pred.ssa)
                  return TRUE;
                else
                  return FALSE;
              }
          }
    }

  return FALSE;
}


void
PF_demote_branches (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
        {
          if (L_uncond_branch_opcode (oper) || L_cond_branch_opcode (oper))
            {
              if (!L_same_operand (oper->pred[0], oper->pred[1]) &&
                  L_pred_def_available (cb, oper, oper->pred[1]))
                {
#if 0
                  printf ("DEMOTING JUMP/BRANCH in Func %s oper %d\n",
                          fn->name, oper->id);
#endif
                  L_delete_operand (oper->pred[0]);
                  oper->pred[0] = L_copy_operand (oper->pred[1]);
                }
            }
        }
    }
}


/*
 * GRAPH VERSION CONTROL
 * ---------------------------------------------------------------------------
 */


void
PF_invalidate_graph (PRED_FLOW * pred_flow)
{
  if (!pred_flow)
    L_punt ("PF_invalidate_graph: Attempt to invalidate NULL graph.");
  pred_flow->poison = TRUE;
  return;
}


int
PF_is_valid_graph (PRED_FLOW * pred_flow)
{
  if (!pred_flow || pred_flow->poison)
    return FALSE;
  else
    return TRUE;
}
