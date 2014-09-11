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
 *      File:   r_dataflow.c
 *      Author: David August, Wen-mei Hwu
 *      Based loosely on the original by Rick Hank
 *      Creation Date:  November 1996
 *      Update: John W. Sias
 *      Add critical variable analysis, reduce memory consumption,
 *      optimize allocation of sets
 *      Update: Shane Ryoo
 *      Add partial code elimination dataflows.
\****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#undef DEBUG_DOM
#undef DEBUG_A_GEN_KILL_SETS
#undef DEBUG_E_GEN_KILL_SETS
#undef DEBUG_E_IN_OUT_SETS
#undef DEBUG_R_GEN_KILL_SETS
#undef DEBUG_DEF_USE_SETS

/*
 * Internal interfaces
 */

#define RD_SET_CLEAR(set)       ((set) ? ((set)=Set_dispose((set))) : NULL)

#define RD_FIND_OPD_DEF(pf,reg) ((Set) HashTable_find_or_null \
                                 ((pf)->hash_RD_operand_def, (reg)))

#define RD_UPDATE_OPD_DEF(pf,reg, opd) HashTable_update \
                                 ((pf)->hash_RD_operand_def, (reg), (opd))

#define RD_FIND_OPD_USE(pf,reg) ((Set) HashTable_find_or_null \
                                 ((pf)->hash_RD_operand_use, (reg)))

#define RD_UPDATE_OPD_USE(pf,reg, opd) HashTable_update \
                                 ((pf)->hash_RD_operand_use, (reg), (opd))

#define SINK_FIND_OPD_DEF(pf, reg) ((Set) HashTable_find_or_null \
                                 ((pf)->hash_sink_oper_def, (reg)))

#define SINK_UPDATE_OPD_DEF(pf, ref, opd) HashTable_update \
                                 ((pf)->hash_sink_oper_def, (reg), (opd))

#define RD_SET_FLAG(a,f) (a) = L_SET_BIT_FLAG((a),(f))
#define RD_CLR_FLAG(a,f) (a) = L_CLR_BIT_FLAG((a),(f))
#define RD_TST_FLAG(a,f) L_EXTRACT_BIT_VAL((a),(f))

static void D_dominator_postdominator_analysis (PRED_FLOW * pred_flow,
						int mode);
static void D_live_variable_analysis (PRED_FLOW * pred_flow, int mode);
static void D_critical_variable_analysis (PRED_FLOW * pred_flow, int mode);
static void D_compute_operand_use_def_sets (PRED_FLOW * pred_flow);
static void D_reaching_definition_analysis (PRED_FLOW * pred_flow);
static void D_available_definition_analysis (PRED_FLOW * pred_flow);
static void D_available_expression_analysis (PRED_FLOW * pred_flow);
static void D_compute_operand_mem_conflict_sets (PRED_FLOW * pred_flow);
static void D_reaching_mem_definition_analysis (PRED_FLOW * pred_flow);
static void D_available_mem_definition_analysis (PRED_FLOW * pred_flow);
void D_pce_flow_analysis (PRED_FLOW *, int);

void EMN_debug_print (PRED_FLOW * pred_flow, Set s, char *str);


/* 
 * Handles to typically available graphs.
 * ----------------------------------------------------------------------
 */

PRED_FLOW *PF_bb_flow = NULL;
PRED_FLOW *PF_pred_flow = NULL;
PRED_FLOW *PF_default_flow = NULL;

/* 
 * Pool initialization
 * ----------------------------------------------------------------------
 */

/* Memory allocation pools */

L_Alloc_Pool *D_alloc_df_pce_info = NULL;
L_Alloc_Pool *D_alloc_df_inst_info = NULL;
L_Alloc_Pool *D_alloc_df_oper_info = NULL;
L_Alloc_Pool *D_alloc_df_node_info = NULL;
L_Alloc_Pool *D_alloc_df_bb_info = NULL;
L_Alloc_Pool *D_alloc_df_cb_info = NULL;

static void
D_init_dataflow (L_Func * fn)
{
  if (!D_alloc_df_inst_info)
    D_alloc_df_inst_info =
      L_create_alloc_pool ("DF_INST_INFO", sizeof (DF_INST_INFO), 1024);
  if (!D_alloc_df_oper_info)
    D_alloc_df_oper_info =
      L_create_alloc_pool ("DF_OPER_INFO", sizeof (DF_OPER_INFO), 1024);
  if (!D_alloc_df_node_info)
    D_alloc_df_node_info =
      L_create_alloc_pool ("DF_NODE_INFO", sizeof (DF_NODE_INFO), 256);
  if (!D_alloc_df_bb_info)
    D_alloc_df_bb_info =
      L_create_alloc_pool ("DF_BB_INFO", sizeof (DF_BB_INFO), 256);
  if (!D_alloc_df_cb_info)
    D_alloc_df_cb_info =
      L_create_alloc_pool ("DF_CB_INFO", sizeof (DF_CB_INFO), 256);

  if (PF_bb_flow)
    PF_bb_flow = PF_delete_flow_graph (PF_bb_flow);
  if (PF_pred_flow)
    PF_pred_flow = PF_delete_flow_graph (PF_pred_flow);
  PF_default_flow = NULL;
  return;
}

void
D_delete_dataflow (L_Func * fn)
{
  if (PF_bb_flow)
    PF_bb_flow = PF_delete_flow_graph (PF_bb_flow);
  if (PF_pred_flow)
    PF_pred_flow = PF_delete_flow_graph (PF_pred_flow);
  PF_default_flow = NULL;
  return;
}

/*
 * EXTERNAL INTERFACES
 * ----------------------------------------------------------------------
 */

void
D_setup_dataflow (L_Func * fn, int do_operands)
{
  if (L_EXTRACT_BIT_VAL (L_fn->flags, L_FUNC_HYPERBLOCK) &&
      !(do_operands & PF_SUPPRESS_PRED_GRAPH))
    {
      L_stop_time (&L_module_global_dataflow_time);
      PG_setup_pred_graph (fn);
      PG_pred_dead_code_removal (fn);
      if (PG_pred_graph && PG_pred_graph->unreachable_code)
	L_delete_unreachable_blocks (fn);

      L_start_time (&L_module_global_dataflow_time);
    }

  /* Initialize/Reset the dataflow analysis internal data structures */
  D_init_dataflow (fn);

  PF_initialize ();

  if (L_df_max_pred_paths != 0)
    {
      PF_pred_flow = PF_build_pred_flow_graph (fn, do_operands);
      PF_default_flow = PF_pred_flow;
    }
  else
    {
      PF_bb_flow = PF_build_bb_graph (fn, do_operands);
      PF_default_flow = PF_bb_flow;
    }

  return;
}


void
D_setup_BB_lists (PRED_FLOW * pred_flow)
{
  PF_CB * pf_cb;
  PF_BB * pf_bb;
  PF_NODE * pf_node, * other_pf_node;
  int flag;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	pf_node->pf_bb = pf_bb;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
	{
	  pf_bb->pf_nodes_last = List_reset (pf_bb->pf_nodes_last);
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	    {
	      flag = 0;
	      PF_FOREACH_NODE (other_pf_node, pf_node->succ)
		{
		  flag = 1;
		  if (pf_node->pf_bb != other_pf_node->pf_bb)
		    pf_bb->pf_nodes_last =
		      List_insert_last (pf_bb->pf_nodes_last, pf_node);
		}
	      if (flag == 0)
		pf_bb->pf_nodes_last =
		  List_insert_last (pf_bb->pf_nodes_last, pf_node);
	    }
	}
    }
}

/*
 * D_partial_dead_code_removal (JWS)
 * ----------------------------------------------------------------------
 * Removes dead code and updates pred[1] fields to indicate strongest
 * viable predicate
 */

void
D_partial_dead_code_removal (L_Func *fn)
{
  D_init_dataflow (fn);

  PF_initialize ();

  if (L_df_use_max_graph_builder)
    PF_pred_flow = PF_build_max_pred_flow_graph (fn, PF_ALL_OPERANDS);
  else
    PF_pred_flow = PF_build_pred_flow_graph (fn, PF_ALL_OPERANDS);
  PF_default_flow = PF_pred_flow;

  PF_clear_partial_dead_code_markings (PF_default_flow);

  D_critical_variable_analysis (PF_default_flow, CRITICAL_VARIABLE);

  PF_partial_dead_code_removal (PF_default_flow);
  return;
}


int
D_delete_DF_dead_code (L_Func *fn)
{
  L_Cb *cb;
  L_Oper *oper, *next_oper;
  int change;

  change = 0;
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	  L_find_attr (cb->attr, "kernel"))
	continue;
      for (oper = cb->first_op; oper; oper = next_oper)
	{
	  next_oper = oper->next_op;

	  if (L_find_attr (oper->attr, DF_DEAD_CODE_ATTR))
	    {
	      if (L_debug_df_dead_code)
		{
		  fprintf(stderr, "> D_delete_DF_dead_code: \n"
			  "  elide op %d\n", oper->id);
		  L_print_oper(stderr, oper);
		}
	      L_delete_complete_oper (cb, oper);
	      change++;
	    }
#if 0
	  else if (L_general_pred_comparison_opcode (oper))
	    {
	      int i;
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  L_Operand *dst;
		  if (!(dst=oper->dest[i]))
		    continue;
		  if (!D_in_oper_OUT_set (PF_default_flow, cb, oper,
					  L_REG_MAC_INDEX (dst), 
					  BOTH_PATHS))
		    {
		      if (L_debug_df_dead_code)
			{
			  fprintf(stderr, "> D_delete_DF_dead_code: \n"
				  "  elide pred dest %d in op %d\n", 
				  i, oper->id);
			  L_print_oper(stderr, oper);
			}
		      L_delete_operand (dst);
		      oper->dest[i] = NULL;
		    }
		}

	    }
#endif
	}
    }
  return change;
}


void
D_clear_partial_dead_code_markings (L_Func * fn)
{
  PF_clear_partial_dead_code_markings (PF_default_flow);
  return;
}


int
D_dataflow_valid (void)
{
  return (PF_is_valid_graph (PF_bb_flow) && PF_is_valid_graph (PF_pred_flow));
}


void
D_invalidate_dataflow (void)
{
  if (PF_bb_flow)
    PF_invalidate_graph (PF_bb_flow);
  if (PF_pred_flow)
    PF_invalidate_graph (PF_pred_flow);
  return;
}


void
D_dataflow_analysis (PRED_FLOW * pred_flow, int mode)
{
  if (mode & PCE)
    {
      D_pce_flow_analysis (pred_flow, mode);
      pred_flow->poison = FALSE;
      return;
    }

  if (mode & (DOMINATOR | POST_DOMINATOR |
	      DOMINATOR_CB | POST_DOMINATOR_CB |
	      DOMINATOR_INT | POST_DOMINATOR_INT |
	      MEM_REACHING_DEFINITION | MEM_AVAILABLE_DEFINITION))
    D_dominator_postdominator_analysis (pred_flow, mode);

  if (mode & (LIVE_VARIABLE | LIVE_VARIABLE_CB | LIVE_VARIABLE_INT))
    D_live_variable_analysis (pred_flow, mode);

  if (mode & (REACHING_DEFINITION | AVAILABLE_DEFINITION | 
	      AVAILABLE_EXPRESSION))
    D_compute_operand_use_def_sets (pred_flow);    

  if (mode & REACHING_DEFINITION)
    D_reaching_definition_analysis (pred_flow);

  if (mode & (AVAILABLE_DEFINITION |
              MEM_REACHING_DEFINITION | MEM_AVAILABLE_DEFINITION))
    D_available_definition_analysis (pred_flow);

  if (mode & (AVAILABLE_EXPRESSION |
              MEM_REACHING_DEFINITION | MEM_AVAILABLE_DEFINITION))
    D_available_expression_analysis (pred_flow);

  if (mode & (MEM_REACHING_DEFINITION | MEM_AVAILABLE_DEFINITION))
    D_compute_operand_mem_conflict_sets (pred_flow);

  if (mode & MEM_REACHING_DEFINITION)
    D_reaching_mem_definition_analysis (pred_flow);

  if (mode & MEM_AVAILABLE_DEFINITION)
    D_available_mem_definition_analysis (pred_flow);

  if (mode & CRITICAL_VARIABLE)
    D_critical_variable_analysis (pred_flow, mode);

  pred_flow->poison = FALSE;
  return;
}


void
D_add_src_operand (PRED_FLOW * pred_flow, L_Oper * oper, int reg,
                   int transparent, int unconditional)
{
  PF_add_src_operand (pred_flow, oper, reg, transparent, unconditional);
  return;
}


void
D_add_dest_operand (PRED_FLOW * pred_flow, L_Oper * oper, int reg,
                    int transparent, int unconditional)
{
  PF_add_dest_operand (pred_flow, oper, reg, transparent, unconditional);
  return;
}


void
D_unmap_rdid (PRED_FLOW * pred_flow, int rdid, int *oper_id, int *operand_id)
{
  PF_OPERAND *pf_operand;

  pf_operand =
    (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand, rdid);
  if (!pf_operand)
    L_punt ("D_unmap_rdid: pf_operand %d not in hash\n", rdid);

  *oper_id = pf_operand->pf_oper->oper->id;
  *operand_id = pf_operand->reg;
  return;
}


/*
 * DOMINATOR / POST-DOMINATOR ANALYSIS
 * ----------------------------------------------------------------------
 */


static void
D_cb_dominator_postdominator (PRED_FLOW * pred_flow, int mode)
{
  int indx, cnt, *buf;
  Set dom, pdom;
  PF_CB *pf_cb;
  PF_NODE *pf_node;
  PF_NODE *tmp_pf_node;

  buf = (int *) Lcode_malloc (sizeof (int) * 
			  List_size (pred_flow->list_pf_node));

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      pf_node = (PF_NODE *) List_first (pf_cb->pf_nodes_entry);

      /*
       * The cb dominator (post_dominator) information, can be obtained
       * from the in (out) set of the first dataflow_cb of a cb. 
       */

      if (mode & (DOMINATOR | DOMINATOR_CB))
        {
          cnt = Set_2array (pf_node->info->dom, buf);
          dom = NULL;
	  RD_SET_CLEAR(pf_cb->info->dom);
          for (indx = 0; indx < cnt; indx++)
            {
              tmp_pf_node =
                (PF_NODE *) HashTable_find_or_null (pred_flow->hash_pf_node,
                                                    buf[indx]);
              if (!tmp_pf_node)
                L_punt ("D_cb_dominator_postdominator:"
			" pf_node %d not in hash\n", buf[indx]);

              dom = Set_add (dom, tmp_pf_node->pf_cb->cb->id);
            }
          pf_cb->info->dom = dom;
        }

      if (mode & (POST_DOMINATOR | POST_DOMINATOR_CB))
        {
          cnt = Set_2array (pf_node->info->post_dom, buf);
          pdom = NULL;
	  RD_SET_CLEAR(pf_cb->info->post_dom);
          for (indx = 0; indx < cnt; indx++)
            {
              tmp_pf_node =
                (PF_NODE *) HashTable_find_or_null (pred_flow->hash_pf_node,
                                                    buf[indx]);
              if (!tmp_pf_node)
                L_punt ("D_cb_dominator_postdominator: "
			"pf_node %d not in hash\n", buf[indx]);

              pdom = Set_add (pdom, tmp_pf_node->pf_cb->cb->id);
            }
          pf_cb->info->post_dom = pdom;
        }
    }
  Lcode_free (buf);
  return;
}


static void
D_instr_dominator_postdominator (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  HashTable hash_pfnode_instr;
  Set dom, post_dom, node_dom, instr_set;
  int indx, cnt, *buf;

  /* Generate a mapping from cb dom/pdom sets to oper dom/pdom */

  hash_pfnode_instr = HashTable_create (1024);

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      if (!pf_node->pf_insts)
	continue;
      instr_set = NULL;
      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
	instr_set = Set_add (instr_set, pf_inst->pf_oper->oper->id);
      HashTable_insert (hash_pfnode_instr, pf_node->id, instr_set);
    }

  buf = (int *) Lcode_malloc (sizeof (int) * 
			      List_size (pred_flow->list_pf_node));

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      /* Map node dom/pdom sets from cbs to instrs */

      PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes)
	{
	  if (mode & DOMINATOR)
	    {
	      dom = NULL;
	      cnt = Set_2array (pf_node->info->dom, buf);
	      RD_SET_CLEAR (pf_node->info->dom);
	      for (indx = 0; indx < cnt; indx++)
		{
		  if (buf[indx] != pf_node->id)
		    {
		      instr_set =
			(Set) HashTable_find_or_null (hash_pfnode_instr,
						      buf[indx]);
		      if (!instr_set)
			L_punt ("D_instr_dominator_postdominator: "
                            "pf_operand %d not in hash\n", buf[indx]);

		      dom = Set_union_acc (dom, instr_set);
		    }
		}
	      pf_node->info->dom = dom;
	    }

	  if (mode & POST_DOMINATOR)
	    {
	      post_dom = NULL;
	      cnt = Set_2array (pf_node->info->post_dom, buf);
	      RD_SET_CLEAR (pf_node->info->post_dom);
	      for (indx = 0; indx < cnt; indx++)
		{
		  if (buf[indx] != pf_node->id)
		    {
		      instr_set =
			(Set) HashTable_find_or_null (hash_pfnode_instr,
						      buf[indx]);
		      if (!instr_set)
			L_punt ("D_instr_dominator_postdominator: "
				"pf_operand %d not in hash\n", buf[indx]);

		      post_dom = Set_union_acc (post_dom, instr_set);
		    }
		}
	      pf_node->info->post_dom = post_dom;
	    }
	}

      if (mode & DOMINATOR)
	{
	  PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
	    {
	      dom = NULL;
	      RD_SET_CLEAR (pf_oper->info->dom);
	      PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
		{
		  pf_node = pf_inst->pf_node;

		  node_dom = pf_node->info->dom;
		  
		  node_dom = Set_add (node_dom, pf_oper->oper->id);

		  if (pf_inst->pred_true)
		    {
		      dom = dom ? Set_intersect_acc (dom, node_dom) :
			Set_copy (node_dom);
		    }

		  pf_node->info->dom = node_dom;
                }
	      pf_oper->info->dom = dom;
            }
	}

      if (mode & POST_DOMINATOR)
	{
	  PF_FORHCAE_OPER(pf_oper, pf_cb->pf_opers)
	    {
	      post_dom = NULL;
	      RD_SET_CLEAR (pf_oper->info->post_dom);
	      PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
		{
		  pf_node = pf_inst->pf_node;

		  node_dom = pf_node->info->post_dom;

		  node_dom = Set_add (node_dom, pf_oper->oper->id);

		  if (pf_inst->pred_true)
		    {
		      post_dom = post_dom ?
			Set_intersect_acc (post_dom, node_dom) :
			Set_copy (node_dom);
                    }
		  pf_node->info->post_dom = node_dom;
                }
	      pf_oper->info->post_dom = post_dom;
	    }
	}
    }

  HashTable_start (hash_pfnode_instr);
  while ((instr_set = (Set) HashTable_next (hash_pfnode_instr)))
    RD_SET_CLEAR (instr_set);

  HashTable_free (hash_pfnode_instr);
  Lcode_free (buf);
  return;
}


static void
D_dominator_postdominator_analysis (PRED_FLOW * pred_flow, int mode)
{
  int change;
  Set dom, post_dom;
  PF_NODE *pf_node;
  PF_NODE *pred_pf_node;
  PF_NODE *succ_pf_node;
  PF_INST *pf_inst;

  /*   
   * The "in" set of each pf_node is used to hold the dominators of
   * that pf_node and the "out" set of each pf_node is used to hold the post
   * dominators of that pf_node.  
   */

  /*
   * Initialization
   */
  if ((mode & (DOMINATOR | DOMINATOR_CB | DOMINATOR_INT)) ||
      (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT)))
    {
      RD_SET_CLEAR (pred_flow->pf_node_root->info->dom);
      pred_flow->pf_node_root->info->dom =
        Set_add (NULL, pred_flow->pf_node_root->id);
    }

  if (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT))
    {
      RD_SET_CLEAR (pred_flow->pf_node_root->info->post_dom);
      pred_flow->pf_node_root->info->post_dom =
        Set_copy (pred_flow->pf_node_U);
    }

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      if ((mode & (DOMINATOR | DOMINATOR_CB | DOMINATOR_INT)) ||
          (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT)))
        {
	  RD_SET_CLEAR (pf_node->info->dom);
          pf_node->info->dom = Set_copy (pred_flow->pf_node_U);
        }

      if (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT))
        {
	  RD_SET_CLEAR (pf_node->info->post_dom);
          pf_inst = (PF_INST *) List_last (pf_node->pf_insts);
          if (pf_inst && L_subroutine_return_opcode (pf_inst->pf_oper->oper))
	    {
	      pf_node->info->post_dom = Set_add (NULL, pf_node->id);
	    }
          else
            {
              pf_node->info->post_dom =
                Set_copy (pred_flow->pf_node_U);
            }
        }
    }

  if ((mode & (DOMINATOR | DOMINATOR_CB | DOMINATOR_INT)) ||
      (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT)))
    {
      change = 1;
      while (change)
        {
          change = 0;
	  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
            {
              /*
               *  dom[] = intersect(in[] of all predecessors of i)
               */
              dom = NULL;

	      List_start (pf_node->pred);
              if ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
                {
                  dom = Set_copy (pred_pf_node->info->dom);
                  while ((pred_pf_node=(PF_NODE *) List_next (pf_node->pred)))
		    dom = Set_intersect_acc (dom, pred_pf_node->info->dom);
                }
              dom = Set_add (dom, pf_node->id);

              if (!Set_subtract_empty (pf_node->info->dom, dom))
                change++;

	      RD_SET_CLEAR (pf_node->info->dom);
              pf_node->info->dom = dom;
            }
        }
    }
  if (mode & (POST_DOMINATOR | POST_DOMINATOR_CB | POST_DOMINATOR_INT))
    {
      change = 1;
      while (change)
        {
          change = 0;
	  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
            {
              /*
               *  post_dom[] = intersect(out[] of all successors of i) 
               */
              post_dom = NULL;

	      PF_FOREACH_NODE(succ_pf_node, pf_node->succ)
                {
                  if (Set_in (pf_node->info->dom, succ_pf_node->id))
                    continue;
                  if (post_dom)
                    post_dom =
                      Set_intersect_acc (post_dom,
                                         succ_pf_node->info->post_dom);
                  else
                    post_dom = Set_copy (succ_pf_node->info->post_dom);
                }

              post_dom = Set_add (post_dom, pf_node->id);

              if (!Set_subtract_empty (pf_node->info->post_dom, post_dom))
                change++;

	      RD_SET_CLEAR (pf_node->info->post_dom);
              pf_node->info->post_dom = post_dom;
            }
        }
    }

#ifdef DEBUG_DOM
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      fprintf (stdout, "DF_CB %d First: %d  Last: %d\n", pf_node->id,
               ((List_first (pf_node->pf_insts) == NULL) ? 0 :
                List_first (pf_node->pf_insts)->oper->id),
               ((List_last (pf_node->pf_insts) == NULL) ? 0 :
                List_last (pf_node->pf_insts)->oper->id));
      Set_print (stdout, "\tIN", pf_node->info->dom);
      Set_print (stdout, "\tOUT", pf_node->info->post_dom);
    }
#endif

  if ((mode & (DOMINATOR_CB | POST_DOMINATOR_CB | 
	       DOMINATOR | POST_DOMINATOR)))
    D_cb_dominator_postdominator (pred_flow, mode);

  if (mode & (DOMINATOR | POST_DOMINATOR))
    D_instr_dominator_postdominator (pred_flow, mode);

  return;
}


/*
 * LIVE VARIABLE ANALYSIS
 * ----------------------------------------------------------------------
 */


static void
D_compute_cb_du_sets (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set use, def;
  int reg;

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      use = def = NULL;
      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
	  PF_FOREACH_OPERAND(pf_operand, pf_inst->src)
            {
              reg = pf_operand->reg;
              if (!Set_in (def, reg))
                use = Set_add (use, reg);
            }
	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
            {
              reg = pf_operand->reg;
	      if (PF_ASSURED(pf_inst, pf_operand) && !Set_in (use, reg))
		def = Set_add (def, reg);
            }
        }
      pf_node->info->use_gen = use;
      pf_node->info->def_kill = def;
      RD_SET_CLEAR (pf_node->info->in);
      pf_node->info->in = Set_copy (use);
      RD_SET_CLEAR (pf_node->info->out);
      pf_node->info->cnt = 0;
    }

#ifdef DEBUG_DEF_USE_SETS
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      fprintf (stdout, "PF_NODE %d (%d)\n",
               pf_node->id, pf_node->pf_cb->cb->id);
      Set_print (stdout, "\tUSE", pf_node->info->use_gen);
      Set_print (stdout, "\tDEF", pf_node->info->def_kill);
    }
#endif
  return;
}


static void
D_instr_lv_cb (PRED_FLOW * pred_flow, PF_CB *pf_cb, int intf)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  PF_OPER *pf_oper;
  Set in, out, node_out;

  PF_FORHCAE_OPER(pf_oper, pf_cb->pf_opers)
    {
      in = NULL;
      out = NULL;

      PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
	{
	  pf_node = pf_inst->pf_node;

	  node_out = pf_node->info->out;
	  
	  if (pf_inst->pred_true ||
	      (intf && (pf_oper->flags & PF_UNCOND_DEST)))
	    {
	      out = Set_union_acc (out, node_out);
	    }
	  else if (pf_oper->flags & PF_UNCOND_DEST)
	    {
	      PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
		if (pf_operand->unconditional &&
		    Set_in(node_out, pf_operand->reg))
		  out = Set_add (out, pf_operand->reg);
	    }

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
	    if (PF_ASSURED(pf_inst, pf_operand))
	      node_out = Set_delete (node_out, pf_operand->reg);

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->src)
	    node_out = Set_add (node_out, pf_operand->reg);
	  
	  pf_node->info->out = node_out;

	  if (pf_inst->pred_true ||
	      (intf && (pf_oper->flags & PF_UNCOND_DEST)))
	    in = Set_union_acc (in, node_out);
	}

      RD_SET_CLEAR (pf_oper->info->v_in);
      pf_oper->info->v_in = in;
      RD_SET_CLEAR (pf_oper->info->v_out);
      pf_oper->info->v_out = out;
    }

  return;
}


static void
D_cb_live_variable (PRED_FLOW * pred_flow, int mode)
{
  PF_CB *pf_cb;
  PF_NODE *pf_node;
  PF_NODE *succ_pf_node;
  Set entry_ids;

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      RD_SET_CLEAR (pf_cb->info->v_in);
      RD_SET_CLEAR (pf_cb->info->v_out);

      entry_ids = NULL;
      PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes_entry)
        {
          /*
           * The in set of the df_cb is the union of the in sets
           * in the comp_cb_enter list 
           */

          pf_cb->info->v_in = Set_union_acc (pf_cb->info->v_in,
                                             pf_node->info->in);

          entry_ids = Set_add (entry_ids, pf_node->id);
        }

      PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes)
        {
	  PF_FOREACH_NODE(succ_pf_node, pf_node->succ)
            {
              if (succ_pf_node->pf_cb->cb->id == pf_cb->cb->id &&
                  !Set_in (entry_ids, succ_pf_node->id))
                continue;

              /* 
               * The out set of the df_cb is the union of the in sets
               * of its successors.
               */
              pf_cb->info->v_out = Set_union_acc (pf_cb->info->v_out,
                                                  succ_pf_node->info->in);
            }
        }

      RD_SET_CLEAR (entry_ids);
    }
  return;
}


static void
D_live_variable_analysis (PRED_FLOW * pred_flow, int mode)
{
  int change;
  PF_CB *pf_cb;
  PF_NODE *pf_node, *succ_pf_node;
  Set in, out;

  /* GLOBAL PHASE */

  D_compute_cb_du_sets (pred_flow, mode);

  do
    {
      change = 0;
      PF_FORHCAE_NODE(pf_node, pred_flow->list_pf_node)
        {
	  int cnt = 0;
          out = pf_node->info->out;
          /*
           *  out[S] = union(in[] of all successors of S)
           */

	  PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	    out = Set_union_acc (out, succ_pf_node->info->in);

	  cnt = Set_size (out);

          pf_node->info->out = out;

	  if (cnt > pf_node->info->cnt)
	    {
	      pf_node->info->cnt = cnt;

	      /*
	       *  in[S] = use[S] + (out[S] - def[S])
	       */
	      in = Set_subtract_union (out, pf_node->info->def_kill,
				       pf_node->info->use_gen);

	      /* A change has occured if <in> contains more live   */
	      /* variables than <df_cb->in>.  Only this one check */
	      /* is necessary, since once a variable is in the in  */
	      /* set of a cb, it won't be removed. Think about it! */
	      if (!Set_subtract_empty (in, pf_node->info->in))
		change++;

	      RD_SET_CLEAR (pf_node->info->in);
	      pf_node->info->in = in;
	    }
        }
    }
  while (change);

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->use_gen);
      RD_SET_CLEAR (pf_node->info->def_kill);
    }

  if (L_debug_df_live_in_out)
    {
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
          fprintf (stdout, "PF_NODE %d (%d)\n", pf_node->id,
                   pf_node->pf_cb->cb->id);
          Set_print (stdout, "\tIN", pf_node->info->in);
          Set_print (stdout, "\tOUT", pf_node->info->out);
        }
    }

  if (mode & (LIVE_VARIABLE | LIVE_VARIABLE_CB))
    {
      D_cb_live_variable (pred_flow, mode);
    }

  /* LOCAL PHASE */

  if (mode & LIVE_VARIABLE)
    {
      PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
	D_instr_lv_cb (pred_flow, pf_cb, (mode & INTERFERENCE) ? 1 : 0);
    }

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
    }

  return;
}


/*
 * CRITICAL VARIABLE ANALYSIS
 * ----------------------------------------------------------------------
 */


static int
D_local_prop_cv (PF_NODE *pf_node)
{
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set in, out;
  int live, crit, reg, change = 0;

  out = pf_node->info->out;

  in = Set_copy (out);

  PF_FORHCAE_INST(pf_inst, pf_node->pf_insts)
    {
      live = RD_TST_FLAG(pf_inst->flags, PF_LIVE_CODE);
      crit = RD_TST_FLAG(pf_inst->flags, PF_CRIT_CODE);
      
      PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
	{
	  reg = pf_operand->reg;
	  if (!live && Set_in(in, reg))
	    {
	      live = 1;
	      RD_SET_FLAG(pf_inst->flags, PF_LIVE_CODE);
	    }
	  if (PF_ASSURED(pf_inst, pf_operand))
	    in = Set_delete (in, reg);
	}

      PF_FOREACH_OPERAND(pf_operand, pf_inst->src)
	{
	  reg = pf_operand->reg;
	  if (live || crit || pf_operand->unconditional)
	    in = Set_add (in, reg);
	}
    }

  /*
   *  in[S] = use[S] + (out[S] - def[S])
   */
  
  /* A change has occured if <in> contains more live   */
  /* variables than <df_cb->in>.  Only this one check */
  /* is necessary, since once a variable is in the in  */
  /* set of a cb, it won't be removed. Think about it! */
  
  if (!Set_subtract_empty (in, pf_node->info->in))
    change = 1;

  /* update in and out */
  RD_SET_CLEAR (pf_node->info->in);
  pf_node->info->in = in;

  return change;
}


static void
D_mark_unquestionable_instrs(PF_CB *pf_cb)
{
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  int critical, pei_only;
  L_Oper *oper;

  PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
    {
      oper = pf_oper->oper;
      
      if (L_subroutine_return_opcode(oper) ||
	  L_subroutine_call_opcode(oper) ||
	  L_is_control_oper(oper) ||
	  L_store_opcode(oper) ||
	  L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPILL_CODE) ||
	  oper->opc == Lop_DEFINE ||
	  oper->opc == Lop_ALLOC ||
	  L_has_unsafe_macro_dest_operand(oper))
	critical = 1;
      else
	critical = 0;

      if (!critical && L_is_pei (oper))
	{
	  critical = 1;
	  pei_only = 1;
	}
      else
	{
	  pei_only = 0;
	}

      PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
	{
	  if (pei_only)
	    {
	      RD_SET_FLAG(pf_inst->flags, PF_CRIT_CODE);
	      RD_CLR_FLAG(pf_inst->flags, PF_LIVE_CODE);
	    }
	  else
	    {
	      if (critical)
		RD_SET_FLAG(pf_inst->flags, PF_LIVE_CODE);
	      else
		RD_CLR_FLAG(pf_inst->flags, PF_LIVE_CODE);
	      RD_CLR_FLAG(pf_inst->flags, PF_CRIT_CODE);
	    }
	  RD_CLR_FLAG(pf_inst->flags, PF_DEAD_CODE);
	}
    }
  return;
}


static void
D_cb_init_cv (PF_CB *pf_cb)
{
  PF_NODE *pf_node;

  D_mark_unquestionable_instrs(pf_cb);

  PF_FOREACH_NODE (pf_node, pf_cb->pf_nodes)
    {
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
      pf_node->info->cnt = 0;
      D_local_prop_cv (pf_node);
    }
  return;
}

/*
 * D_instr_cv_cb
 * ----------------------------------------------------------------------
 * Compute CRITICAL_VARIABLE IN and OUT sets on opers.
 * During interference graph construction, an unconditional define must
 * be considered to interfere with all live predicates, not simply those
 * live when the guard predicate is TRUE.
 */
static void
D_instr_cv_cb (PRED_FLOW * pred_flow, PF_CB *pf_cb, int intf)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  PF_OPER *pf_oper;
  Set in, out, node_out;
  int reg, live;
  
  PF_FORHCAE_OPER (pf_oper, pf_cb->pf_opers)
    {
      in = out = NULL;

      RD_SET_CLEAR (pf_oper->info->v_in);
      RD_SET_CLEAR (pf_oper->info->v_out);

      PF_FOREACH_INST (pf_inst, pf_oper->pf_insts)
	{
	  pf_node = pf_inst->pf_node;

	  node_out = pf_node->info->out;

	  if (pf_inst->pred_true || 
	      (intf && (pf_oper->flags & PF_UNCOND_DEST)))
	    {
	      out = Set_union_acc (out, node_out);
	    }
	  else if (pf_oper->flags & PF_UNCOND_DEST) 
	    {
	      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
		if (pf_operand->unconditional &&
		    Set_in (node_out, pf_operand->reg))
		  out = Set_add (out, pf_operand->reg);
	    }

	  live = RD_TST_FLAG (pf_inst->flags, PF_LIVE_CODE);

	  if (!L_EXTRACT_BIT_VAL (pf_inst->flags, PF_DEAD_CODE|PF_LIVE_CODE))
	    {
	      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
		{
		  if (Set_in (node_out, pf_operand->reg))
		    break;
		}
	      if (!pf_operand)
		RD_SET_FLAG (pf_inst->flags, PF_DEAD_CODE);
	    }

	  PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
	    {
	      reg = pf_operand->reg;
	      if (PF_ASSURED (pf_inst, pf_operand))
		node_out = Set_delete (node_out, reg);
	    }
	  
	  PF_FOREACH_OPERAND (pf_operand, pf_inst->src)
	    {
	      reg = pf_operand->reg;
	      if (live || pf_operand->unconditional)
		node_out = Set_add (node_out, reg);
	    }

	  if (pf_inst->pred_true || 
	      (intf && (pf_oper->flags & PF_UNCOND_DEST)))
	    {
	      in = Set_union_acc (in, node_out);
	    }

	  pf_node->info->out = node_out;
	}
      pf_oper->info->v_in = in;
      pf_oper->info->v_out = out;
    }

  return;
}


static void
D_critical_variable_analysis (PRED_FLOW * pred_flow, int mode)
{
  int change, cnt;
  PF_CB *pf_cb;
  PF_NODE *pf_node, *pf_node_succ;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
    D_cb_init_cv (pf_cb);

  do
    {
      change = 0;
      PF_FORHCAE_NODE(pf_node, pred_flow->list_pf_node)
        {
          Set out = pf_node->info->out;
          /*
           *  out[S] = union(in[] of all successors of S)
           */

	  cnt = 0;
	  PF_FORHCAE_NODE(pf_node_succ, pf_node->succ)
	    {
	      out = Set_union_acc (out, pf_node_succ->info->in);
	      cnt = 1;
	    }
	  /* Calculate local prop of last node, since it won't be computed
	   * normally. */
	  if (!cnt)
	    D_local_prop_cv (pf_node);

	  /* After the first round, only the out set drives changes --
	   * if the out set doesn't change, don't bother to process
	   * the block again.
	   */

	  cnt = Set_size (out);

	  pf_node->info->out = out;

	  if (cnt > pf_node->info->cnt)
	    {
	      pf_node->info->cnt = cnt;
	      change += D_local_prop_cv (pf_node);
	    }
        }
    }
  while (change);

  D_cb_live_variable (pred_flow, CRITICAL_VARIABLE);

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
    {
      D_instr_cv_cb (pred_flow, pf_cb, (mode & INTERFERENCE) ? 1 : 0);
    }

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
    }

  return;
}


/*
 * REACHING DEFINITION ANALYSIS
 * ----------------------------------------------------------------------
 */


static void
D_compute_operand_use_def_sets (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;
  Set RD_operand;
  /* int num_regs; */
  int reg;

  /* num_regs = Set_size (pred_flow->reg_U); */

  if (pred_flow->hash_RD_operand_def)
    HashTable_reset_func (pred_flow->hash_RD_operand_def,
                          (void (*)(void *)) Set_dispose);
  else
    pred_flow->hash_RD_operand_def = HashTable_create (2048);

  if (pred_flow->hash_RD_operand_use)
    HashTable_reset_func (pred_flow->hash_RD_operand_use,
                          (void (*)(void *)) Set_dispose);
  else
    pred_flow->hash_RD_operand_use = HashTable_create (2048);

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
        {
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->dest)
            {
              reg = pf_operand->reg;
              RD_operand = RD_FIND_OPD_DEF (pred_flow, reg);
              RD_operand = Set_add (RD_operand, pf_operand->id);
	      RD_UPDATE_OPD_DEF (pred_flow, reg, RD_operand);
            }
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->src)
            {
              reg = pf_operand->reg;
              RD_operand = RD_FIND_OPD_USE (pred_flow, reg);
              RD_operand = Set_add (RD_operand, pf_oper->oper->id);
	      RD_UPDATE_OPD_USE (pred_flow, reg, RD_operand);
	    }
        }
    }
  return;
}


static void
D_compute_cb_r_gk_sets (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set gen, kill;
  Set RD_operand;
  int reg;

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
#ifdef DEBUG_R_GEN_KILL_SETS
      printf ("##GK PF_NODE %d, CB %d\n", pf_node->id,
              pf_node->pf_cb->cb->id);
#endif
      gen = kill = NULL;

      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
#ifdef DEBUG_R_GEN_KILL_SETS
          printf ("  GK OPER %d ", pf_inst->pf_oper->oper->id);
#endif
	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
            {
              reg = pf_operand->reg;
              RD_operand = RD_FIND_OPD_DEF (pred_flow, reg);

	      if (PF_ASSURED(pf_inst, pf_operand))
                {
#ifdef DEBUG_R_GEN_KILL_SETS
                  EMN_debug_print (pred_flow, RD_operand, "   GK KILLEDR");
#endif
                  kill = Set_union_acc (kill, RD_operand);
                  gen = Set_subtract_acc (gen, RD_operand);
                }

              gen = Set_add (gen, pf_operand->id);
            }
#ifdef DEBUG_R_GEN_KILL_SETS
          printf ("\n");
#endif
        }

      RD_SET_CLEAR (pf_node->info->def_kill);
      RD_SET_CLEAR (pf_node->info->use_gen);
      pf_node->info->def_kill = Set_subtract (kill, gen);
      pf_node->info->use_gen = gen;
      RD_SET_CLEAR (kill);

      /* Initialize the propagation fields */
      
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
      pf_node->info->out = Set_copy (gen);
      pf_node->info->cnt = 0;
    }

#ifdef DEBUG_R_GEN_KILL_SETS
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      fprintf (stdout, "PF_NODE %d, CB %d\n", pf_node->id,
               pf_node->pf_cb->cb->id);
      Set_print (stdout, "\tGEN", pf_node->info->use_gen);
      Set_print (stdout, "\tKILL", pf_node->info->def_kill);
    }
#endif
  return;
}


static void
D_cb_reaching_def (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_NODE *pf_node;

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      RD_SET_CLEAR (pf_cb->info->r_in);

      PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes_entry)
        {
          /*
           * The in set of the df_cb is the union of the in sets
           * in the comp_cb_enter list 
           */

          pf_cb->info->r_in = Set_union_acc (pf_cb->info->r_in,
                                             pf_node->info->in);
        }
    }
  return;
}


static void
D_instr_rd_cb (PRED_FLOW *pred_flow, PF_CB *pf_cb)
{
  PF_NODE *pf_node;
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set op_in, op_out, RD_operand, node_in;

  PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
    {
      op_in = op_out = NULL;
      PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
	{
	  pf_node = pf_inst->pf_node;

	  node_in = pf_node->info->in;

	  if (pf_inst->pred_true)
	    {
	      op_in = Set_union_acc (op_in, node_in);
	    }
	  else if (pf_oper->flags & PF_UNCOND_SRC)
	    {
	      PF_FOREACH_OPERAND(pf_operand, pf_inst->src)
		if (pf_operand->unconditional)
		  {
		    Set isect;
		    int reg = pf_operand->reg;
		    RD_operand = RD_FIND_OPD_DEF (pred_flow, reg);
			
		    isect = Set_intersect (node_in, RD_operand);
			
		    op_in = Set_union_acc (op_in, isect);
		    op_out = Set_union_acc (op_out, isect);

		    RD_SET_CLEAR (isect);
		  }
	    }

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
            {
              int reg = pf_operand->reg;
	      RD_operand = RD_FIND_OPD_DEF (pred_flow, reg);

	      if (PF_ASSURED(pf_inst, pf_operand))
		node_in = Set_subtract_acc (node_in, RD_operand);

              node_in = Set_add (node_in, pf_operand->id);
            }

	  pf_node->info->in = node_in;

	  if (pf_inst->pred_true)
	    op_out = Set_union_acc (op_out, node_in);
	}
	  
      RD_SET_CLEAR (pf_oper->info->r_in);
      RD_SET_CLEAR (pf_oper->info->r_out);
      pf_oper->info->r_in = op_in;
      pf_oper->info->r_out = op_out;
    }

  return;
}

static void
D_reaching_definition_analysis (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_NODE *pf_node, *pred_pf_node;
  Set in, out;
  int change;

  /* GLOBAL PHASE */
  
  /* requires operand use def sets */

  D_compute_cb_r_gk_sets (pred_flow);

  do
    {
      change = 0;
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
	  int cnt;

          /*
           *  in[] = union(out[] of all predecessors of i)
           */
          in = pf_node->info->in;
	  PF_FOREACH_NODE(pred_pf_node, pf_node->pred)
	    in = Set_union_acc (in, pred_pf_node->info->out);

	  cnt = Set_size (in);

          pf_node->info->in = in;
	  
	  if (cnt > pf_node->info->cnt)
	    {
	      /* The in set has changed */

	      pf_node->info->cnt = cnt;

	      /*
	       *  out[] = gen[] + (in[] - kill[])
	       */
	      out = Set_subtract_union (in, pf_node->info->def_kill,
					pf_node->info->use_gen);

	      /* A change has occured if <out> contains more instructions
	       * than <cur_cb->out>.  Only this one check is necessary,
	       * since once an instr.  is in the in set of a cb, it won't
	       * be removed. Think about it!  
	       */

	      if (!Set_subtract_empty (out, pf_node->info->out))
		change++;

	      /* update in and out */
	      RD_SET_CLEAR (pf_node->info->out);
	      pf_node->info->out = out;
	    }
        }
    }
  while (change);

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->use_gen);
      RD_SET_CLEAR (pf_node->info->def_kill);
    }

  D_cb_reaching_def (pred_flow);

  /* LOCAL PHASE */

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    D_instr_rd_cb (pred_flow, pf_cb);

  if (L_debug_df_reaching_defs)
    {
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
          fprintf (stdout, "PF_NODE %d First: %d  Last: %d\n", pf_node->id,
                   ((List_first (pf_node->pf_insts) == NULL) ? 0 :
                    ((PF_INST *) List_first (pf_node->pf_insts))->pf_oper->
                    oper->id),
                   ((List_last (pf_node->pf_insts) == NULL) ? 0
                    : ((PF_INST *) List_last (pf_node->pf_insts))->pf_oper->
                    oper->id));
          Set_print (stdout, "\tIN", pf_node->info->in);
          Set_print (stdout, "\tOUT", pf_node->info->out);
        }
    }

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
    }

  return;
}


/* Dan Lavery's enhancement.  ITI/MCM 8/17/99 */

/* remove PF_OPERAND id corresponding to specified destination
   (operand_id) of reaching_oper from reaching definition
   IN set of oper. */
void
D_remove_from_oper_RIN_set (PRED_FLOW * pred_flow, L_Oper * oper,
                            L_Oper * reaching_oper, int operand_id)
{
  PF_OPER *pf_oper;
  PF_OPER *reaching_pf_oper;
  PF_OPERAND *pf_operand;

  if (!pred_flow || !oper || !reaching_oper)
    L_punt ("D_remove_from_oper_RIN_set: received NULL!\n");

  pf_oper = (PF_OPER *) HashTable_find (pred_flow->hash_oper_pfoper,
                                        oper->id);

  reaching_pf_oper = (PF_OPER *) HashTable_find (pred_flow->hash_oper_pfoper,
                                                 reaching_oper->id);

  PF_FOREACH_OPERAND(pf_operand, reaching_pf_oper->dest)
    {
      if (pf_operand->reg == operand_id)
        {
          pf_oper->info->r_in =
            Set_delete (pf_oper->info->r_in, pf_operand->id);
          return;
        }
    }
  L_punt ("D_remove_from_oper_RIN_set: "
          "invalid pair, L_Oper: %d,Operand_id %d\n",
          reaching_oper->id, operand_id);
}


/*
 * AVAILABLE DEFINITION ANALYSIS
 * ----------------------------------------------------------------------
 */


static void
D_compute_cb_a_gk_sets (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set gen, kill;
  Set RD_operand;
  int reg;

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      gen = kill = NULL;

      RD_SET_CLEAR (pf_node->info->def_kill);
      RD_SET_CLEAR (pf_node->info->use_gen);

      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
            {
              reg = pf_operand->reg;
              RD_operand = RD_FIND_OPD_DEF (pred_flow, reg);
	      
              kill = Set_union_acc (kill, RD_operand);	      
	      gen = Set_subtract_acc (gen, RD_operand);	      

	      if (PF_ASSURED(pf_inst, pf_operand))
		gen = Set_add (gen, pf_operand->id);
            }
        }

      pf_node->info->def_kill = kill;
      pf_node->info->use_gen = gen;
    }

#ifdef DEBUG_A_GEN_KILL_SETS
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      fprintf (stdout, "PF_NODE %d, CB %d\n", pf_node->id,
               pf_node->pf_cb->cb->id);
      Set_print (stdout, "\tGEN", pf_node->info->use_gen);
      Set_print (stdout, "\tKILL", pf_node->info->def_kill);
    }
#endif
  return;
}


static void
D_instr_ad_cb (PRED_FLOW * pred_flow, PF_CB *pf_cb)
{
  PF_NODE *pf_node;
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set in, out, node_in, RD_operand;
  int reg;

  PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
    {
      int first = 1;
      in = out = NULL;

      RD_SET_CLEAR (pf_oper->info->a_in);
      RD_SET_CLEAR (pf_oper->info->a_out);

      PF_FOREACH_INST (pf_inst, pf_oper->pf_insts)
	{
	  pf_node = pf_inst->pf_node;

	  node_in = pf_node->info->in;

	  if (pf_inst->pred_true)
	    {
	      if (first)
		in = Set_copy (node_in);
	      else
		in = Set_intersect_acc (in, node_in);
	    }

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
            {
              reg = pf_operand->reg;
	      RD_operand = RD_FIND_OPD_DEF (pred_flow, reg);

              node_in = Set_subtract_acc (node_in, RD_operand);

	      if (PF_ASSURED(pf_inst, pf_operand))
		node_in = Set_add (node_in, pf_operand->id);
            }

	  if (pf_inst->pred_true)
	    {
	      if (first)
		{
		  out = Set_copy (node_in);
		  first = 0;	      
		}
	      else
		{
		  out = Set_intersect_acc (out, node_in);
		}
	    }

	  pf_node->info->in = node_in;
	}

      pf_oper->info->a_in = in;
      pf_oper->info->a_out = out;
    }

  return;
}


static void
D_available_definition_analysis (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  PF_CB *pf_cb;
  Set in, out;
  int change;

  /* GLOBAL PHASE */

  /* requires operand use def sets */

  D_compute_cb_a_gk_sets (pred_flow);

  List_start (pred_flow->list_pf_node);
  pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node);
  RD_SET_CLEAR (pf_node->info->out);
  pf_node->info->out = Set_union(NULL, pf_node->info->use_gen);

  while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
    {
      RD_SET_CLEAR (pf_node->info->out);
      pf_node->info->out = Set_subtract_union (pred_flow->pf_operand_U, 
					       pf_node->info->def_kill,
					       pf_node->info->use_gen);
    }

  do
    {
      change = 0;
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
          /*
           *  in[] = intersect(out[] of all predecessors of i)
           */
          in = NULL;

          List_start (pf_node->pred);
          if ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
            {
              in = Set_copy (pred_pf_node->info->out);
              while ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
		in = Set_intersect_acc (in, pred_pf_node->info->out);
            }

          /*
           *  out[] = gen[] + (in[] - kill[])
           */
          out = Set_subtract_union (in, pf_node->info->def_kill,
                                    pf_node->info->use_gen);

	  /*
	   * Out sets converge monotonically downward toward available def
	   */
          if (!Set_subtract_empty (pf_node->info->out, out))
            change++;

          /* update in and out */
	  RD_SET_CLEAR (pf_node->info->in);
	  RD_SET_CLEAR (pf_node->info->out);
          pf_node->info->in = in;
          pf_node->info->out = out;
        }
    }
  while (change);

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->use_gen);
      RD_SET_CLEAR (pf_node->info->def_kill);
    }

  /* LOCAL PHASE */

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      D_instr_ad_cb (pred_flow, pf_cb);
    }

  if (L_debug_df_available_defs)
    {
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
          fprintf (stdout, "PF_NODE %d First: %d  Last: %d\n", pf_node->id,
                   ((List_first (pf_node->pf_insts) == NULL) ? 0 :
                    ((PF_INST *) List_first (pf_node->pf_insts))->pf_oper->
                    oper->id),
                   ((List_last (pf_node->pf_insts) == NULL) ? 0
                    : ((PF_INST *) List_last (pf_node->pf_insts))->pf_oper->
                    oper->id));
          Set_print (stdout, "\tIN", pf_node->info->in);
          Set_print (stdout, "\tOUT", pf_node->info->out);
        }
    }

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
    }

  return;
}


/*
 * AVAILABLE EXPRESSION ANALYSIS
 * ----------------------------------------------------------------------
 */


static void
D_compute_cb_e_gk_sets (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set gen, kill, oper_kill, tmp;
  int reg;

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      gen = kill = NULL;

      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
          oper_kill = NULL;
	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
            {
              reg = pf_operand->reg;
	      tmp = RD_FIND_OPD_USE (pred_flow, reg);
              oper_kill = Set_union_acc (oper_kill, tmp);
            }

          if (!Set_in (oper_kill, pf_inst->pf_oper->oper->id))
            {
              /* this oper generates an expression only if */
              /* the oper->id is not in the kill set, in   */
              /* which case oper_gen = oper->id, so we have */
              /* a one element set                          */
              gen = Set_subtract_acc (gen, oper_kill);
              gen = Set_add (gen, pf_inst->pf_oper->oper->id);

              kill = Set_delete (kill, pf_inst->pf_oper->oper->id);
              kill = Set_union_acc (kill, oper_kill);
            }
          else
            {
              /* this oper generates nothing, so oper_gen is empty */
              gen = Set_subtract_acc (gen, oper_kill);
              kill = Set_union_acc (kill, oper_kill);
            }
	  RD_SET_CLEAR (oper_kill);
        }
      pf_node->info->use_gen = gen;
      pf_node->info->def_kill = kill;
    }

#ifdef DEBUG_E_GEN_KILL_SETS
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      fprintf (stdout, "PF_NODE %d, CB %d\n", pf_node->id,
               pf_node->pf_cb->cb->id);
      Set_print (stdout, "\tGEN", pf_node->info->use_gen);
      Set_print (stdout, "\tKILL", pf_node->info->def_kill);
    }
#endif

  return;
}


static void
D_instr_ae_cb (PRED_FLOW * pred_flow, PF_CB *pf_cb)
{
  PF_NODE *pf_node;
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set in, out, tmp, node_in;
  int reg;

  PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
    {
      int first = 1;
      in = out = NULL;
      
      PF_FOREACH_INST (pf_inst, pf_oper->pf_insts)
	{
	  pf_node = pf_inst->pf_node;
	  node_in = pf_node->info->in;

	  if (pf_inst->pred_true)
	    in = first ? Set_copy (node_in) : Set_intersect_acc (in, node_in);

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
            {
              reg = pf_operand->reg;
	      tmp = RD_FIND_OPD_USE (pred_flow, reg);
              node_in = Set_subtract_acc (node_in, tmp);
            }

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->dest)
	    {
	      if (PF_ASSURED(pf_inst, pf_operand))
		node_in = Set_add (node_in, pf_operand->id);
	    }

	  pf_node->info->in = node_in;

	  if (pf_inst->pred_true)
	    {
	      if (first)
		{
		  out = Set_copy (node_in);
		  first = 0;	      
		}
	      else
		{
		  out = Set_intersect_acc (out, node_in);
		}
	    }
	}

      RD_SET_CLEAR (pf_oper->info->e_in);
      RD_SET_CLEAR (pf_oper->info->e_out);
      pf_oper->info->e_in = in;
      pf_oper->info->e_out = out;
    }

  return;
}


static void
D_available_expression_analysis (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  PF_CB *pf_cb;
  Set in, out;
  int change = 1;

  /* GLOBAL PHASE */

  /* requires operand use def sets */

  D_compute_cb_e_gk_sets (pred_flow);

  List_start (pred_flow->list_pf_node);
  pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node);
  RD_SET_CLEAR (pf_node->info->out);
  pf_node->info->out = Set_copy (pf_node->info->use_gen);
  while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
    {
      RD_SET_CLEAR (pf_node->info->out);
      pf_node->info->out =
        Set_subtract (pred_flow->oper_U, pf_node->info->def_kill);
    }

  do
    {
      change = 0;
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
          /*
           *  in[] = intersect(out[] of all predecessors of i)
           */
          in = NULL;
          List_start (pf_node->pred);
          if ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
            {
              in = Set_copy (pred_pf_node->info->out);
              while ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
		in = Set_intersect_acc (in, pred_pf_node->info->out);
            }

          /*
           *  out[] = gen[] + (in[] - kill[])
           */
          out = Set_subtract_union (in, pf_node->info->def_kill,
                                    pf_node->info->use_gen);

          /* A change has occured if <out> contains more instr- */
          /* uctions than <cur_cb->out>.   Only this one check */
          /* is necessary, since once an instr.  is in the in  */
          /* set of a cb, it won't be removed. Think about it! */
          if (!Set_subtract_empty (pf_node->info->out, out))
            change++;

          /* update in and out */
	  RD_SET_CLEAR (pf_node->info->in);
	  RD_SET_CLEAR (pf_node->info->out);
          pf_node->info->in = in;
          pf_node->info->out = out;
        }
    }
  while (change);

  /* LOCAL PHASE */

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      /* Initialize to value of first node */
      if (!(pf_node = (PF_NODE *) List_first (pf_cb->pf_nodes_entry)))
	L_punt("EIN: cb has zero pf_nodes");
      in = Set_copy (pf_node->info->in);
      out = Set_copy (pf_node->info->out);

      PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes_entry)
	{
	  in = Set_intersect_acc (in, pf_node->info->in);
	  out = Set_intersect_acc (out, pf_node->info->out);
	}

      RD_SET_CLEAR (pf_cb->info->e_in);
      RD_SET_CLEAR (pf_cb->info->e_out);
      pf_cb->info->e_in = in;
      pf_cb->info->e_out = out;

      D_instr_ae_cb (pred_flow, pf_cb);
    }

#if DEBUG_E_IN_OUT_SETS
  {
    PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
      {
        fprintf (stdout, "PF_NODE %d First: %d  Last: %d\n",
                 pf_node->pf_cb->cb->id,
                 ((List_first (pf_node->pf_insts) == NULL) ? 0
                  : ((PF_INST *) List_first (pf_node->pf_insts))->pf_oper->
                  oper->id),
                 ((List_last (pf_node->pf_insts) == NULL) ? 0
                  : ((PF_INST *) List_last (pf_node->pf_insts))->pf_oper->
                  oper->id));
        Set_print (stdout, "\tIN", pf_node->info->in);
        Set_print (stdout, "\tOUT", pf_node->info->out);
      }
  }
#endif

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      RD_SET_CLEAR (pf_node->info->use_gen);
      RD_SET_CLEAR (pf_node->info->def_kill);
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
    }

  return;
}


void
D_remove_from_cb_EIN_set (PRED_FLOW *pred_flow, L_Cb *cb,
			  L_Oper *reach_oper)
{
  PF_CB *pf_cb;

  if (!pred_flow || !cb || !reach_oper)
    L_punt ("D_remove_from_cb_EIN_set: received NULL!\n");

  pf_cb =
    (PF_CB *) HashTable_find_or_null (pred_flow->hash_cb_pfcb,
                                        cb->id);
  if (!pf_cb)
    {
      L_punt ("D_remove_from_cb_EIN_set: pf_cb %d not in hash\n",
              cb->id);
    }

  if (Set_in (pf_cb->info->e_in, reach_oper->id))
    {
      /* remove available expression from oper e_in and e_out sets */
      pf_cb->info->e_in = Set_delete (pf_cb->info->e_in, reach_oper->id);
      pf_cb->info->e_out =
        Set_delete (pf_cb->info->e_out, reach_oper->id);
    }
}


void
D_remove_from_oper_EIN_set (PRED_FLOW * pred_flow, L_Oper * oper,
                            L_Oper * reach_oper)
{
  PF_OPER *pf_oper;

  if (!pred_flow || !oper || !reach_oper)
    L_punt ("D_remove_from_oper_EIN_set: received NULL!\n");

  pf_oper =
    (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                        oper->id);

  if (!pf_oper)
    {
      /* EMN2001: Some safe ops might have been added since dataflow
	 was run. In those cases, just return */
      if (!oper->dest[0] && !L_is_control_oper(oper))
	return;

      L_punt ("D_remove_from_oper_EIN_set: pf_oper %d not in hash\n",
              oper->id);
    }

  if (Set_in (pf_oper->info->e_in, reach_oper->id))
    {
      /* remove available expression from oper e_in and e_out sets */
      pf_oper->info->e_in = Set_delete (pf_oper->info->e_in, reach_oper->id);
      pf_oper->info->e_out =
        Set_delete (pf_oper->info->e_out, reach_oper->id);
    }
}


/*
 * MEMORY REACHING DEFINITION ANALYSIS
 * ----------------------------------------------------------------------
 */


static List
Memset_Add_acc (List memset, int id, Set conflicts)
{
  PF_MEM_SET *ms;

  /* Insert this new id/set in sorted order */
  List_start (memset);
  while ((ms = (PF_MEM_SET *) List_next (memset)))
    {
      if (ms->id == id)
        {
          /* Add conflicts to existing set */
          ms->conflicts = Set_union_acc (ms->conflicts, conflicts);
          return memset;
        }
      else if (ms->id > id)
        {
          break;
        }
    }

  /* Create new set and add to list */
  ms = PF_new_mem_set (id, conflicts);
  memset = List_insert_prev (memset, ms);

  return memset;
}


static List
Memset_Sub_acc (List memset, Set conflicts)
{
  PF_MEM_SET *ms;

  /* Subtract conflicts from every set */
  List_start (memset);
  while ((ms = (PF_MEM_SET *) List_next (memset)))
    {
      ms->conflicts = Set_subtract_acc (ms->conflicts, conflicts);

      if (!Set_size (ms->conflicts))
        {
          /* Conflicts set empty, delete entry */
          memset = List_delete_current (memset);
          ms = PF_delete_mem_set (ms);
        }
    }

  return memset;
}


static int
Memset_same_size (List memset1, List memset2)
{
  PF_MEM_SET *ms1, *ms2;

  List_start (memset1);
  List_start (memset2);
  ms1 = (PF_MEM_SET *) List_next (memset1);
  ms2 = (PF_MEM_SET *) List_next (memset2);

  /* ids are sorted, so process lowest id
     and intersect equal ids */
  while (ms1 || ms2)
    {
      if (!ms1 || !ms2 || (ms1->id != ms2->id) ||
	  (Set_size (ms1->conflicts) != Set_size (ms2->conflicts)))
	return 0;

      ms1 = (PF_MEM_SET *) List_next (memset1);
      ms2 = (PF_MEM_SET *) List_next (memset2);
    }

  return 1;
}


static List
Memset_Union (List memset1, List memset2)
{
  List memset = NULL;
  PF_MEM_SET *ms1, *ms2, *ms;
  Set union_set = NULL;

  List_start (memset1);
  List_start (memset2);
  ms1 = (PF_MEM_SET *) List_next (memset1);
  ms2 = (PF_MEM_SET *) List_next (memset2);

  /* ids are sorted, so process lowest id
     and merge equal ids */
  while (ms1 || ms2)
    {
      if ((!ms1) || (ms2 && ms2->id < ms1->id))
        {
          ms = PF_new_mem_set (ms2->id, ms2->conflicts);
          memset = List_insert_last (memset, ms);
          ms2 = (PF_MEM_SET *) List_next (memset2);
        }
      else if ((!ms2) || (ms1 && ms1->id < ms2->id))
        {
          ms = PF_new_mem_set (ms1->id, ms1->conflicts);
          memset = List_insert_last (memset, ms);
          ms1 = (PF_MEM_SET *) List_next (memset1);
        }
      else
        {
          /* Equal ids, combine and add */
          union_set = Set_union (ms1->conflicts, ms2->conflicts);
          ms = PF_new_mem_set (ms1->id, union_set);
          memset = List_insert_last (memset, ms);
          ms1 = (PF_MEM_SET *) List_next (memset1);
          ms2 = (PF_MEM_SET *) List_next (memset2);
	  RD_SET_CLEAR (union_set);
        }
    }

  return memset;
}


static List
Memset_Union_acc (List memset1, List memset2)
{
  PF_MEM_SET *ms1, *ms2, *ms;

  List_start (memset1);
  List_start (memset2);
  ms1 = (PF_MEM_SET *) List_next (memset1);
  ms2 = (PF_MEM_SET *) List_next (memset2);

  /* ids are sorted, so process lowest id
     and merge equal ids */
  while (ms2)
    {
      if ((!ms1) || (ms2->id < ms1->id))
        {
          ms = PF_new_mem_set (ms2->id, ms2->conflicts);
          memset1 = List_insert_prev (memset1, ms);
          ms2 = (PF_MEM_SET *) List_next (memset2);
        }
      else if (ms1->id < ms2->id)
        {
          ms1 = (PF_MEM_SET *) List_next (memset1);
        }
      else
        {
          /* Equal ids, combine and add */
          ms1->conflicts = Set_union_acc (ms1->conflicts, ms2->conflicts);
          ms1 = (PF_MEM_SET *) List_next (memset1);
          ms2 = (PF_MEM_SET *) List_next (memset2);
        }
    }

  return memset1;
}


static List
Memset_Kill_acc (List memset, List killmemset)
{
  PF_MEM_SET *ms;

  /* Subtract each killmemset conflicts from memset */
  List_start (killmemset);
  while ((ms = (PF_MEM_SET *) List_next (killmemset)))
    memset = Memset_Sub_acc (memset, ms->conflicts);

  return memset;
}


#if 0
static List
Memset_Intersect (List memset1, List memset2)
{
  List memset = NULL;
  PF_MEM_SET *ms1, *ms2, *ms;
  Set inter_set = NULL;

  List_start (memset1);
  List_start (memset2);
  ms1 = (PF_MEM_SET *) List_next (memset1);
  ms2 = (PF_MEM_SET *) List_next (memset2);

  /* ids are sorted, so process lowest id
     and intersect equal ids */
  while (ms1 || ms2)
    {
      if ((!ms1) || (ms2 && ms2->id < ms1->id))
        {
          ms2 = (PF_MEM_SET *) List_next (memset2);
        }
      else if ((!ms2) || (ms1 && ms1->id < ms2->id))
        {
          ms1 = (PF_MEM_SET *) List_next (memset1);
        }
      else
        {
          /* Equal ids, combine and add */
          inter_set = Set_intersect (ms1->conflicts, ms2->conflicts);
          if (Set_size (inter_set))
            {
              ms = PF_new_mem_set (ms1->id, inter_set);
              memset = List_insert_last (memset, ms);
            }
          ms1 = (PF_MEM_SET *) List_next (memset1);
          ms2 = (PF_MEM_SET *) List_next (memset2);
	  RD_SET_CLEAR (inter_set);
        }
    }

  return memset;
}


static List
Memset_Intersect_acc (List memset1, List memset2)
{
  PF_MEM_SET *ms1, *ms2;

  List_start (memset1);
  List_start (memset2);
  ms1 = (PF_MEM_SET *) List_next (memset1);
  ms2 = (PF_MEM_SET *) List_next (memset2);

  /* ids are sorted, so process lowest id
     and intersect equal ids */
  while (ms1)
    {
      if (ms2 && ms2->id < ms1->id)
        {
          ms2 = (PF_MEM_SET *) List_next (memset2);
        }
      else if ((!ms2) || (ms1->id < ms2->id))
        {
          memset1 = List_delete_current (memset1);
          ms1 = PF_delete_mem_set (ms1);
          ms1 = (PF_MEM_SET *) List_next (memset1);
        }
      else
        {
          /* Equal ids, combine and add */
          ms1->conflicts = Set_intersect_acc (ms1->conflicts, ms2->conflicts);
          if (!Set_size (ms1->conflicts))
            {
              memset1 = List_delete_current (memset1);
              ms1 = PF_delete_mem_set (ms1);
            }
          ms1 = (PF_MEM_SET *) List_next (memset1);
          ms2 = (PF_MEM_SET *) List_next (memset2);
        }
    }

  return memset1;
}
#endif


static Set
Memset_Create_Set (List memset, int id)
{
  PF_MEM_SET *ms;
  Set match = NULL;

  List_start (memset);
  while ((ms = (PF_MEM_SET *) List_next (memset)))
    if (Set_in (ms->conflicts, id))
      match = Set_add (match, ms->id);

  return match;
}


static List
Memset_dispose (List memset)
{
  PF_MEM_SET *ms;

  List_start (memset);
  while ((ms = (PF_MEM_SET *) List_next (memset)))
    {
      memset = List_delete_current (memset);
      ms = PF_delete_mem_set (ms);
    }

  return memset;
}


static void
D_compute_operand_mem_conflict_sets (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand, *pf_comp_operand;
  Set opers, amb_opers, opers2, amb_opers2;
  int type;
  L_Cb *cb, *comp_cb;

  if (pred_flow->hash_mem_oper_conflict)
    {
      HashTable_reset_func (pred_flow->hash_mem_oper_conflict,
                            (void (*)(void *)) Set_dispose);
      HashTable_reset_func (pred_flow->hash_amb_mem_oper_conflict,
                            (void (*)(void *)) Set_dispose);
    }
  else
    {
      pred_flow->hash_mem_oper_conflict = HashTable_create (2048);
      pred_flow->hash_amb_mem_oper_conflict = HashTable_create (2048);
    }

  /*
   * The end result is to compare all dests w/ dests and srcs,
   * and all srcs with dests and srcs
   */

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      cb = pf_cb->cb;

      PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
        {
	  PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_dest)
            {
              opers = (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        pf_operand->id);
              amb_opers = (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        pf_operand->id);

              /* Compare dest to all memory destinations 
                 and record conflicts */

	      PF_FOREACH_OPERAND(pf_comp_operand, 
				 pred_flow->pf_mem_dest_operand_U)
                {
		  L_Oper *comp_lop;

		  comp_lop =  pf_comp_operand->pf_oper->oper;
                  comp_cb =
                    L_oper_hash_tbl_find_cb (pred_flow->fn->oper_hash_tbl,
					     comp_lop->id);

                  type = L_is_ida_memory_ops (cb, pf_operand->pf_oper->oper,
                                              comp_cb, comp_lop,
                                              SET_NONLOOP_CARRIED (0));
                  switch (type)
                    {
                    case MEM_DEP:
                      if ((L_debug_df_mem_reaching_defs > 5)
                          || (L_debug_df_mem_available_defs > 5))
                        printf ("EMN: DEP: st%d st%d\n",
                                pf_operand->pf_oper->oper->id,
                                comp_lop->id);

                      opers = Set_add (opers, pf_comp_operand->id);
                      break;
                    case MEM_AMB:
                      if ((L_debug_df_mem_reaching_defs > 5)
                          || (L_debug_df_mem_available_defs > 5))
                        printf ("EMN: AMB: st%d st%d\n",
                                pf_operand->pf_oper->oper->id,
                                comp_lop->id);

                      amb_opers = Set_add (amb_opers, pf_comp_operand->id);
                      break;
                    case MEM_IND:
                      break;
                    default:
                      L_punt ("D_compute_operand_mem_conflict_sets: "
                              "Invalid memory op relationship\n");
                    }
                }

              /* Compare dest to all memory sources and record conflicts */
	      PF_FOREACH_OPERAND(pf_comp_operand, 
				 pred_flow->pf_mem_src_operand_U)
                {
		  L_Oper *comp_lop;

		  comp_lop = pf_comp_operand->pf_oper->oper;
                  comp_cb =
                    L_oper_hash_tbl_find_cb (pred_flow->fn->oper_hash_tbl,
					     comp_lop->id);

                  type = L_is_ida_memory_ops (cb, pf_operand->pf_oper->oper,
                                              comp_cb, comp_lop,
                                              SET_NONLOOP_CARRIED (0));

                  switch (type)
                    {
                    case MEM_DEP:
                      if ((L_debug_df_mem_reaching_defs > 5)
                          || (L_debug_df_mem_available_defs > 5))
                        printf ("EMN: DEP: st%d ld%d\n",
                                pf_operand->pf_oper->oper->id,
                                comp_lop->id);

		      opers2 = (Set)
			HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
						pf_comp_operand->id);

                      opers = Set_add (opers, pf_comp_operand->id);
                      opers2 = Set_add (opers2, pf_operand->id);

		      HashTable_update (pred_flow->hash_mem_oper_conflict,
					pf_comp_operand->id, opers2);
                      break;
                    case MEM_AMB:
                      if ((L_debug_df_mem_reaching_defs > 5)
                          || (L_debug_df_mem_available_defs > 5))
                        printf ("EMN: AMB: st%d ld%d\n",
                                pf_operand->pf_oper->oper->id,
                                comp_lop->id);
		      
		      amb_opers2 = (Set)
			HashTable_find_or_null
			(pred_flow->hash_amb_mem_oper_conflict,
			 pf_comp_operand->id);

                      amb_opers = Set_add (amb_opers, pf_comp_operand->id);
                      amb_opers2 = Set_add (amb_opers2, pf_operand->id);

		      HashTable_update (pred_flow->hash_amb_mem_oper_conflict,
					pf_comp_operand->id, amb_opers2);
                      break;
                    case MEM_IND:
                      break;
                    default:
                      L_punt ("D_compute_operand_mem_conflict_sets: "
                              "Invalid memory op relationship\n");
                    }
                }

              HashTable_update (pred_flow->hash_mem_oper_conflict,
                                pf_operand->id, opers);
              HashTable_update (pred_flow->hash_amb_mem_oper_conflict,
                                pf_operand->id, amb_opers);
            }

	  PF_FOREACH_OPERAND(pf_operand, pf_oper->mem_src)
            {
              opers = (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        pf_operand->id);
              amb_opers = (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        pf_operand->id);

              /* Compare src to all memory sources and record conflicts */
	      PF_FOREACH_OPERAND(pf_comp_operand, 
				 pred_flow->pf_mem_src_operand_U)
                {
		  L_Oper *comp_lop;

		  comp_lop = pf_comp_operand->pf_oper->oper;

                  comp_cb =
                    L_oper_hash_tbl_find_cb (pred_flow->fn->oper_hash_tbl,
                                             comp_lop->id);

                  type = L_is_ida_memory_ops (cb, pf_operand->pf_oper->oper,
                                              comp_cb, comp_lop,
                                              SET_NONLOOP_CARRIED (0));

                  switch (type)
                    {
                    case MEM_DEP:
                      if ((L_debug_df_mem_reaching_defs > 5)
                          || (L_debug_df_mem_available_defs > 5))
                        printf ("EMN: DEP: ld%d ld%d\n",
                                pf_operand->pf_oper->oper->id,
                                comp_lop->id);

                      opers = Set_add (opers, pf_comp_operand->id);
                      break;
                    case MEM_AMB:
                      if ((L_debug_df_mem_reaching_defs > 5)
                          || (L_debug_df_mem_available_defs > 5))
                        printf ("EMN: AMB: ld%d ld%d\n",
                                pf_operand->pf_oper->oper->id,
                                comp_lop->id);

                      amb_opers = Set_add (amb_opers, pf_comp_operand->id);
                      break;
                    case MEM_IND:
                      break;
                    default:
                      L_punt ("D_compute_operand_mem_conflict_sets: "
                              "Invalid memory op relationship\n");
                    }

                }

              HashTable_update (pred_flow->hash_mem_oper_conflict,
                                pf_operand->id, opers);
              HashTable_update (pred_flow->hash_amb_mem_oper_conflict,
                                pf_operand->id, amb_opers);
            }
        }
    }
  return;
}


static void
D_compute_mem_cb_r_gk_sets (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  List mem_gen, mem_kill;
  Set RD_operand_mem, RD_operand_amb_mem, un;
  int id = 0;

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      mem_gen = mem_kill = NULL;

      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
	  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_src)
            {
              if (!pf_operand->memory)
                L_punt ("D_compute_mem_cb_r_gk_sets: source not to memory\n");

              /* We have a load, they conflict with nothing */

              /* Add the load to the gen set */
              /* mem_gen = Set_add(mem_gen, pf_operand->id); */

              RD_operand_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        id);
              RD_operand_amb_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        id);

              un = Set_union (RD_operand_mem, RD_operand_amb_mem);
              mem_gen = Memset_Add_acc (mem_gen, pf_operand->id, un);
	      RD_SET_CLEAR (un);
            }

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_dest)
            {
              if (!pf_operand->memory)
                L_punt
                  ("D_compute_mem_cb_r_gk_sets: destination not to memory\n");

              id = pf_operand->id;

              /* We have a store or jsr. Get all other loads and stores */
              /* with which it conflicts                                */
              RD_operand_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        id);
              RD_operand_amb_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        id);
              un = Set_union (RD_operand_mem, RD_operand_amb_mem);

	      if (PF_ASSURED(pf_inst, pf_operand))
                {
#if 0
		  mem_kill = Set_union_acc(mem_kill, RD_operand_mem);
		  mem_kill = Set_union_acc(mem_kill, RD_operand_amb_mem);
		  mem_gen = Set_subtract_acc(mem_gen, RD_operand_mem);
		  mem_gen = Set_subtract_acc(mem_gen, RD_operand_amb_mem);
#endif

                  /* Add the loads/stores that are killed to the kill set */
                  mem_kill = Memset_Add_acc (mem_kill, pf_operand->id, un);

                  /* Remove any killed loads/stores from the gen set */
                  mem_gen = Memset_Sub_acc (mem_gen, un);
                }

              /* add the store to the gen set */
              mem_gen = Memset_Add_acc (mem_gen, pf_operand->id, un);
	      RD_SET_CLEAR (un);
            }
        }

      pf_node->info->mem_kill = mem_kill;
      pf_node->info->mem_gen = mem_gen;
    }


#ifdef DEBUG_R_GEN_KILL_SETS
  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      fprintf (stdout, "PF_NODE %d, CB %d\n", pf_node->id,
               pf_node->pf_cb->cb->id);

      EMN_debug_print (pred_flow, pf_node->info->mem_gen, "MGEN ");
      EMN_debug_print (pred_flow, pf_node->info->mem_kill, "MKILL");

    }
#endif
  return;
}


static void
D_mem_instr_rd_cb (PRED_FLOW * pred_flow, PF_CB *pf_cb)
{
  PF_NODE *pf_node;
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  List in, out;
  Set RD_operand_mem, RD_operand_amb_mem, un, in_set, out_set;
  int id = 0;

  PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes)
    {
      out = Memset_Union (NULL, pf_node->info->mem_in);

      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
          in = Memset_Union (NULL, out);

          pf_inst->info->mem_r_in = Memset_Create_Set (in, id);

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_src)
            {
              id = pf_operand->id;
              /* Loads never conflict with anything */

              /* Add load to in set */
              RD_operand_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        id);
              RD_operand_amb_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        id);
              un = Set_union (RD_operand_mem, RD_operand_amb_mem);
              out = Memset_Add_acc (out, pf_operand->id, un);
	      RD_SET_CLEAR (un);
            }

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_dest)
            {
              id = pf_operand->id;
              RD_operand_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        id);
              RD_operand_amb_mem = (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        id);
              un = Set_union (RD_operand_mem, RD_operand_amb_mem);

	      /* remove all ops killed by store/jsr */
	      if (PF_ASSURED(pf_inst, pf_operand))
		out = Memset_Sub_acc (out, un);

              /* add stores to in set */
              out = Memset_Add_acc (out, pf_operand->id, un);
	      RD_SET_CLEAR (un);
            }
          /* id should be set only once above */

          pf_inst->info->mem_r_out = Memset_Create_Set (out, id);
          in = Memset_dispose (in);
        }
      out = Memset_dispose (out);
    }

  PF_FOREACH_OPER(pf_oper, pf_cb->pf_opers)
    {
      in_set = out_set = NULL;
      PF_FOREACH_INST(pf_inst, pf_oper->pf_insts)
	{
	  if (pf_inst->pred_true)
	    {
	      in_set = Set_union_acc (in_set, pf_inst->info->mem_r_in);
	      out_set = Set_union_acc (out_set, pf_inst->info->mem_r_out);
	    }
	  RD_SET_CLEAR(pf_inst->info->mem_r_in);
	  RD_SET_CLEAR(pf_inst->info->mem_r_out);
	}
      pf_oper->info->mem_r_in = in_set;
      pf_oper->info->mem_r_out = out_set;
    }

  return;
}


static void
D_reaching_mem_definition_analysis (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_NODE *pf_node, *pred_pf_node;
  List in, out;
  int change;

  /* 
     THIS SECTION COMPUTES THE REACHING
     DEFINITIONS FOR MEMORY REFERENCES 

     This must be done after register reaching analysis 
     is finished because the memory def-use calls 
     L_indep_memory_op which, in turn, uses the register 
     def-use info
   */

  /* Requires operand mem conflict sets */

  /*fprintf(stderr,"gk\n"); */
  D_compute_mem_cb_r_gk_sets (pred_flow);
  /*fprintf(stderr,"gk done\n"); */

  /* GLOBAL PHASE */

  do
    {
      change = 0;
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
          /*
           *  in[] = union(out[] of all predecessors of i)
           */
          in = NULL;
	  PF_FOREACH_NODE(pred_pf_node, pf_node->pred)
	    in = Memset_Union_acc (in, pred_pf_node->info->mem_out);

          /*
           *  out[] = gen[] + (in[] - kill[])
           */
          out = Memset_Union (NULL, in);
          out = Memset_Kill_acc (out, pf_node->info->mem_kill);
          out = Memset_Union_acc (out, pf_node->info->mem_gen);

          /* A change has occured if <out> contains more instr- */
          /* uctions than <cur_cb->out>.   Only this one check */
          /* is necessary, since once an instr.  is in the in  */
          /* set of a cb, it won't be removed. Think about it! */
          if (!Memset_same_size (out, pf_node->info->mem_out))
            change++;

          /* update in and out */
          pf_node->info->mem_in = Memset_dispose (pf_node->info->mem_in);
          pf_node->info->mem_in = in;
          pf_node->info->mem_out = Memset_dispose (pf_node->info->mem_out);
          pf_node->info->mem_out = out;
        }
    }
  while (change);

  /* LOCAL PHASE */

  PF_FOREACH_CB(pf_cb, pred_flow->list_pf_cb)
    {
      D_mem_instr_rd_cb (pred_flow, pf_cb);

      PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes)
	{
	  if (pf_node->info->mem_gen)
	    pf_node->info->mem_gen = Memset_dispose (pf_node->info->mem_gen);
	  if (pf_node->info->mem_kill)
	    pf_node->info->mem_kill = Memset_dispose (pf_node->info->mem_kill);
	  if (pf_node->info->mem_in)
	    pf_node->info->mem_in = Memset_dispose (pf_node->info->mem_in);
	  if (pf_node->info->mem_out)
	    pf_node->info->mem_out = Memset_dispose (pf_node->info->mem_out);
	}
    }

  return;
}


/*
 * MEMORY AVAILABLE DEFINITION ANALYSIS
 * ----------------------------------------------------------------------
 */


static void
D_compute_mem_cb_a_gk_sets (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set RD_operand_mem, RD_operand_amb_mem, gen, kill;
  int id;

  PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
    {
      gen = kill = NULL;
      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
	  /* Loads never kill anything */
	  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_src)
            {
	      gen = Set_add (gen, pf_operand->id);
            }

	  PF_FOREACH_OPERAND(pf_operand, pf_inst->mem_dest)
            {
              id = pf_operand->id;

              RD_operand_mem =
                (Set) HashTable_find (pred_flow->hash_mem_oper_conflict, id);
              RD_operand_amb_mem =
                (Set) HashTable_find (pred_flow->hash_amb_mem_oper_conflict,
                                      id);

              kill = Set_union_acc (kill, RD_operand_mem);
              kill = Set_union_acc (kill, RD_operand_amb_mem);

              gen = Set_subtract_acc (gen, RD_operand_mem);
              gen = Set_subtract_acc (gen, RD_operand_amb_mem);

	      gen = Set_add (gen, pf_operand->id);
            }
        }

      pf_node->info->def_kill = Set_subtract (kill, gen);
      pf_node->info->use_gen = gen;
      RD_SET_CLEAR (kill);
    }

  if (L_debug_df_mem_available_defs > 4)
    {
      PF_FOREACH_NODE(pf_node, pred_flow->list_pf_node)
        {
          fprintf (stdout, "PF_NODE %d, CB %d\n", pf_node->id,
                   pf_node->pf_cb->cb->id);
          EMN_debug_print (pred_flow, pf_node->info->use_gen, "\tGEN");
          EMN_debug_print (pred_flow, pf_node->info->def_kill, "\tKILL");
        }
    }
  return;
}


static void
D_mem_instr_ad_cb (PRED_FLOW * pred_flow, PF_CB *pf_cb)
{
  PF_NODE *pf_node;
  PF_OPER *pf_oper;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set RD_operand_mem, RD_operand_amb_mem, in, out;
  int id;

  PF_FOREACH_NODE(pf_node, pf_cb->pf_nodes)
    {
      in = Set_copy (pf_node->info->in);
      PF_FOREACH_INST(pf_inst, pf_node->pf_insts)
        {
          pf_inst->info->mem_a_in = Set_copy (in);

	  /* Loads never kill anything */
	  PF_FOREACH_OPERAND (pf_operand, pf_inst->mem_src)
	    {
	      in = Set_add (in, pf_operand->id);
	    }

	  PF_FOREACH_OPERAND (pf_operand, pf_inst->mem_dest)
            {
              id = pf_operand->id;
              RD_operand_mem =
                (Set) HashTable_find (pred_flow->hash_mem_oper_conflict, id);
              RD_operand_amb_mem =
                (Set) HashTable_find (pred_flow->hash_amb_mem_oper_conflict,
                                      id);

              in = Set_subtract_acc (in, RD_operand_mem);
              in = Set_subtract_acc (in, RD_operand_amb_mem);

              /* add stores to in set */
              in = Set_add (in, pf_operand->id);
            }

          pf_inst->info->mem_a_out = Set_copy (in);
        }

      RD_SET_CLEAR (in);
    }

  PF_FOREACH_OPER (pf_oper, pf_cb->pf_opers)
    {
      int first = 1;
      in = out = NULL;

      PF_FOREACH_INST (pf_inst, pf_oper->pf_insts)
	{
	  if (pf_inst->pred_true)
	    {
	      if (first)
		{
		  in = Set_copy (pf_inst->info->mem_a_in);
		  out = Set_copy (pf_inst->info->mem_a_out);
		  first = 0;
		}
	      else
		{
		  in = Set_intersect_acc (in, pf_inst->info->mem_a_in);
		  out = Set_intersect_acc (out, pf_inst->info->mem_a_out);
		}
	    }
	  RD_SET_CLEAR (pf_inst->info->mem_a_in);
	  RD_SET_CLEAR (pf_inst->info->mem_a_out);
	}
      
      pf_oper->info->mem_a_in = in;
      pf_oper->info->mem_a_out = out;
    }

  return;
}


static void
D_available_mem_definition_analysis (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_NODE *pf_node, *pred_pf_node;
  PF_OPERAND *pf_operand;
  Set in, out, tmp = NULL;
  int change;

  /* Requires operand mem conflict sets */

  D_compute_mem_cb_a_gk_sets (pred_flow);

  /* convert src_operand_U and dest_operand_U to a set */

  PF_FOREACH_OPERAND (pf_operand, pred_flow->pf_mem_src_operand_U)
    tmp = Set_add (tmp, pf_operand->id);

  PF_FOREACH_OPERAND (pf_operand, pred_flow->pf_mem_dest_operand_U)
    tmp = Set_add (tmp, pf_operand->id);

  List_start (pred_flow->list_pf_node);
  pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node);

  RD_SET_CLEAR (pf_node->info->out);
  while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
    {
      RD_SET_CLEAR (pf_node->info->out);
      pf_node->info->out = Set_subtract (tmp, pf_node->info->def_kill);
    }

  RD_SET_CLEAR (tmp);

  do
    {
      change = 0;

      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
        {
          /*
           *  in[] = intersect(out[] of all predecessors of i)
           */
          in = NULL;
          List_start (pf_node->pred);
          if ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
            {
              in = Set_copy (pred_pf_node->info->out);
              while ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
		in = Set_intersect_acc (in, pred_pf_node->info->out);
            }

          /*
           *  out[] = gen[] + (in[] - kill[])
           */
          out = Set_subtract_union (in, pf_node->info->def_kill,
                                    pf_node->info->use_gen);

          /* A change has occured if <out> contains more instr- */
          /* uctions than <cur_cb->out>.   Only this one check */
          /* is necessary, since once an instr.  is in the in  */
          /* set of a cb, it won't be removed. Think about it! */
          if (!Set_subtract_empty (pf_node->info->out, out))
            change++;

          /* update in and out */
	  RD_SET_CLEAR (pf_node->info->in);
	  RD_SET_CLEAR (pf_node->info->out);
          pf_node->info->in = in;
          pf_node->info->out = out;
        }
    }
  while (change);

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
    {
      D_mem_instr_ad_cb (pred_flow, pf_cb);
    }

  if (L_debug_df_mem_available_defs > 4)
    {
      List_start (pred_flow->list_pf_node);
      while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
        {
          fprintf (stdout, "PF_NODE %d First: %d  Last: %d\n", pf_node->id,
                   ((List_first (pf_node->pf_insts) == NULL) ? 0 :
                    ((PF_INST *) List_first (pf_node->pf_insts))->pf_oper->
                    oper->id),
                   ((List_last (pf_node->pf_insts) == NULL) ? 0
                    : ((PF_INST *) List_last (pf_node->pf_insts))->pf_oper->
                    oper->id));
          EMN_debug_print (pred_flow, pf_node->info->in, "\tIN");
          EMN_debug_print (pred_flow, pf_node->info->out, "\tOUT");
        }
    }

  List_start (pred_flow->list_pf_node);
  while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
    {
      RD_SET_CLEAR (pf_node->info->use_gen);
      RD_SET_CLEAR (pf_node->info->def_kill);
      RD_SET_CLEAR (pf_node->info->in);
      RD_SET_CLEAR (pf_node->info->out);
    }
  return;
}


#if 0

/* 
   The next three routines are the "real" available
   mem definition rouitines. However, the analysis
   routine has a commented out, initialization 
   routine that is too expensive in memory. A replacement
   for this code is needed to make the functions usable.
   Luckly, the reach def does not need the init routine.
   - EMN
*/

static void
D_compute_mem_cb_a_gk_sets (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  List gen, kill;
  Set un;
  Set RD_operand_mem;
  Set RD_operand_amb_mem;
  int id;

  List_start (pred_flow->list_pf_node);
  while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
    {
      gen = kill = NULL;

      List_start (pf_node->pf_insts);
      while ((pf_inst = (PF_INST *) List_next (pf_node->pf_insts)))
        {

          List_start (pf_inst->mem_src);
          while ((pf_operand = (PF_OPERAND *) List_next (pf_inst->mem_src)))
            {
              if (!pf_operand->memory)
                L_punt ("D_compute_cb_a_gk_sets: source not to memory\n");

              /* Loads never kill anything */

              id = pf_operand->id;
              RD_operand_mem =
                (Set) HashTable_find (pred_flow->hash_mem_oper_conflict, id);
              RD_operand_amb_mem =
                (Set) HashTable_find (pred_flow->hash_amb_mem_oper_conflict,
                                      id);
              un = Set_union (RD_operand_mem, RD_operand_amb_mem);

	      gen = Memset_Add_acc (gen, pf_operand->id, un);

	      RD_SET_CLEAR (un);
            }

          List_start (pf_inst->mem_dest);
          while ((pf_operand = (PF_OPERAND *) List_next (pf_inst->mem_dest)))
            {
              id = pf_operand->id;
              RD_operand_mem =
                (Set) HashTable_find (pred_flow->hash_mem_oper_conflict, id);
              RD_operand_amb_mem =
                (Set) HashTable_find (pred_flow->hash_amb_mem_oper_conflict,
                                      id);
              un = Set_union (RD_operand_mem, RD_operand_amb_mem);

              kill = Memset_Add_acc (kill, id, un);
              gen = Memset_Sub_acc (gen, un);

	      gen = Memset_Add_acc (gen, id, un);

	      RD_SET_CLEAR (un);
            }
        }

      pf_node->info->mem_kill = kill;
      pf_node->info->mem_gen = gen;
    }

#if 0
  if (L_debug_df_mem_available_defs > 4)
    {
      List_start (pred_flow->list_pf_node);
      while ((pf_node = List_next (pred_flow->list_pf_node)))
        {
          fprintf (stdout, "PF_NODE %d, CB %d\n", pf_node->id,
                   pf_node->pf_cb->cb->id);
          EMN_debug_print (pred_flow, pf_node->info->use_gen, "\tGEN");
          EMN_debug_print (pred_flow, pf_node->info->def_kill, "\tKILL");
        }
    }
#endif
  return;
}


static void
D_mem_instr_available_definition (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  List in, out;
  Set RD_operand_mem;
  Set RD_operand_amb_mem;
  Set un;
  int reg, id;

  List_start (pred_flow->list_pf_node);
  while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
    {
      out = Memset_Union (NULL, pf_node->info->mem_in);

      List_start (pf_node->pf_insts);
      while ((pf_inst = (PF_INST *) List_next (pf_node->pf_insts)))
        {
          in = Memset_Union (NULL, out);

          List_start (pf_inst->mem_src);
          while ((pf_operand = (PF_OPERAND *) List_next (pf_inst->mem_src)))
            {
              id = pf_operand->id;
              /* Loads never conflict with anything */

              /* Add load to in set */
              RD_operand_mem =
                (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        id);
              RD_operand_amb_mem =
                (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        id);
              un = Set_union (RD_operand_mem, RD_operand_amb_mem);
              out = Memset_Add_acc (out, pf_operand->id, un);

	      RD_SET_CLEAR (un);
            }

          List_start (pf_inst->mem_dest);
          while ((pf_operand = (PF_OPERAND *) List_next (pf_inst->mem_dest)))
            {
              id = pf_operand->id;
              RD_operand_mem =
                (Set)
                HashTable_find_or_null (pred_flow->hash_mem_oper_conflict,
                                        id);
              RD_operand_amb_mem =
                (Set)
                HashTable_find_or_null (pred_flow->hash_amb_mem_oper_conflict,
                                        id);
              un = Set_union (RD_operand_mem, RD_operand_amb_mem);

              /* remove all ops killed by store/jsr */
              out = Memset_Sub_acc (out, un);

              /* add stores to in set */
              out = Memset_Add_acc (out, pf_operand->id, un);
	      RD_SET_CLEAR (un);
            }
          /* id should be set only once above */
          pf_inst->info->mem_a_in = Memset_Create_Set (in, id);
          pf_inst->info->mem_a_out = Memset_Create_Set (out, id);
          in = Memset_dispose (in);
        }
      out = Memset_dispose (out);
    }
  return;
}


static void
D_available_mem_definition_analysis (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  PF_NODE *pred_pf_node;
  PF_INST *pf_inst;
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;
  List in, out, tmp;
  Set un, RD_operand_mem, RD_operand_amb_mem;
  Set in_set, out_set;
  int id, change;

  /* requires mem conflict sets */

  D_compute_mem_cb_a_gk_sets (pred_flow);

  /* convert src_operand_U and dest_operand_U to a memset */
  /*
     (This is the init routine)    
     tmp = NULL;
     List_start(pred_flow->pf_mem_src_operand_U);
     while ((pf_operand = 
     (PF_OPERAND *)List_next(pred_flow->pf_mem_src_operand_U)))
     {
     id = pf_operand->id; 
     RD_operand_mem = 
     (Set) HashTable_find_or_null(pred_flow->hash_mem_oper_conflict, id);
     RD_operand_amb_mem = 
     (Set) HashTable_find_or_null(pred_flow->hash_amb_mem_oper_conflict, id);
     un = Set_union(RD_operand_mem, RD_operand_amb_mem);
     tmp = Memset_Add_acc(tmp, id, un);

     RD_SET_CLEAR (un);
     }
     List_start(pred_flow->pf_mem_dest_operand_U);
     while ((pf_operand = 
     (PF_OPERAND *)List_next(pred_flow->pf_mem_dest_operand_U)))
     {
     id = pf_operand->id; 
     RD_operand_mem = 
     (Set) HashTable_find_or_null(pred_flow->hash_mem_oper_conflict, id);
     RD_operand_amb_mem = 
     (Set) HashTable_find_or_null(pred_flow->hash_amb_mem_oper_conflict, id);
     un = Set_union(RD_operand_mem, RD_operand_amb_mem);
     tmp = Memset_Add_acc(tmp, id, un);
     RD_SET_CLEAR (un);
     }

     List_start(pred_flow->list_pf_node);
     pf_node = (PF_NODE *)List_next(pred_flow->list_pf_node);
     pf_node->info->mem_out = NULL;
     while ((pf_node = (PF_NODE *)List_next(pred_flow->list_pf_node)))
     {
     pf_node->info->mem_out = Memset_Union(NULL,tmp);
     pf_node->info->mem_out = Memset_Kill_acc(pf_node->info->mem_out, 
     pf_node->info->mem_kill);
     }
     tmp = Memset_dispose(tmp);
   */

  do
    {
      change = 0;
      List_start (pred_flow->list_pf_node);
      while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
        {
          /*
           *  in[] = intersect(out[] of all predecessors of i)
           */
          in = NULL;
          List_start (pf_node->pred);
          if ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
            {
              in = Memset_Union (NULL, pred_pf_node->info->mem_out);
              while ((pred_pf_node = (PF_NODE *) List_next (pf_node->pred)))
                {
                  in = Memset_Intersect_acc (in, pred_pf_node->info->mem_out);
                }
            }

          /*
           *  out[] = gen[] + (in[] - kill[])
           */
          out = Memset_Union (NULL, in);
          out = Memset_Kill_acc (out, pf_node->info->mem_kill);
          out = Memset_Union_acc (out, pf_node->info->mem_gen);

          /* A change has occured if <out> contains more instr- */
          /* uctions than <cur_cb->out>.   Only this one check */
          /* is necessary, since once an instr.  is in the in  */
          /* set of a cb, it won't be removed. Think about it! */
          if (!Memset_same_size (out, pf_node->info->mem_out))
            change++;

          /* update in and out */
          pf_node->info->mem_in = Memset_dispose (pf_node->info->mem_in);
          pf_node->info->mem_in = in;
          pf_node->info->mem_out = Memset_dispose (pf_node->info->mem_out);
          pf_node->info->mem_out = out;
        }
    }
  while (change);

  List_start (pred_flow->list_pf_cb);
  while ((pf_cb = (PF_CB *) List_next (pred_flow->list_pf_cb)))
    {
      D_mem_instr_ad_cb (pred_flow, pf_cb);

      List_start (pf_cb->pf_opers);
      while ((pf_oper = (PF_OPER *) List_next (pf_cb->pf_opers)))
        {

          in_set = out_set = NULL;
          List_start (pf_oper->pf_insts);
          while ((pf_inst = (PF_INST *) List_next (pf_oper->pf_insts))
                 && !pf_inst->pred_true);
          if (pf_inst && pf_inst->pred_true)
            {
              in_set = Set_copy (pf_inst->info->mem_a_in);
              out_set = Set_copy (pf_inst->info->mem_a_out);

              while ((pf_inst = (PF_INST *) List_next (pf_oper->pf_insts)))
                {
                  if (pf_inst->pred_true)
                    {
                      in_set = Set_intersect_acc (in_set, 
						  pf_inst->info->mem_a_in);
                      out_set = Set_intersect_acc (out_set, 
						   pf_inst->info->mem_a_out);
                    }
                }
            }

          pf_oper->info->mem_a_in = in_set;
          pf_oper->info->mem_a_out = out_set;
        }
    }

  /*  
     if ( L_debug_df_mem_available_defs > 4 )
     {
     List_start(pred_flow->list_pf_node);
     while ((pf_node = (PF_NODE *)List_next(pred_flow->list_pf_node)))
     { 
     fprintf(stdout, "PF_NODE %d First: %d  Last: %d\n", pf_node->id,
     ((List_first(pf_node->pf_insts) == NULL)? 0 : 
     ((PF_INST *)List_first(pf_node->pf_insts))->pf_oper->oper->id),
     ((List_last(pf_node->pf_insts) == NULL)? 0 : 
     ((PF_INST *)List_last(pf_node->pf_insts))->pf_oper->oper->id));
     EMN_debug_print(pred_flow, pf_node->info->in, "\tIN"); 
     EMN_debug_print(pred_flow, pf_node->info->out, "\tOUT"); 
     }
     }
   */

  List_start (pred_flow->list_pf_node);
  while ((pf_node = (PF_NODE *) List_next (pred_flow->list_pf_node)))
    {
      if (pf_node->info->mem_gen)
        pf_node->info->mem_gen = Memset_dispose (pf_node->info->mem_gen);
      if (pf_node->info->mem_kill)
        pf_node->info->mem_kill = Memset_dispose (pf_node->info->mem_kill);
      if (pf_node->info->mem_in)
        pf_node->info->mem_in = Memset_dispose (pf_node->info->mem_in);
      if (pf_node->info->mem_out)
        pf_node->info->mem_out = Memset_dispose (pf_node->info->mem_out);
    }
  return;
}
#endif


/*
 * DEBUGGING HOOKS
 * ----------------------------------------------------------------------
 */


void
EMN_debug_print (PRED_FLOW * pred_flow, Set s, char *str)
{
  PF_OPERAND *pf_operand;
  int *buf, num, i;

  num = Set_size (s);
  printf ("%s(", str);

  if (num)
    {
      buf = (int *) Lcode_malloc (sizeof (int) * num);
      Set_2array (s, buf);

      for (i = 0; i < num; i++)
        {
          pf_operand =
            (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                                   buf[i]);
          if (!pf_operand)
            L_punt ("EMN_debug_print: pf_operand %d not in hash\n", buf[i]);
          printf (" %d", pf_operand->pf_oper->oper->id);
        }
      Lcode_free (buf);
    }

  printf (")\n");
  return;
}


void
EMN_debug_print_operset (PRED_FLOW * pred_flow, Set s, char *str)
{
  PF_OPER *pf_oper;
  int *buf, num, i;

  num = Set_size (s);
  printf ("%s(", str);

  if (num)
    {
      buf = (int *) Lcode_malloc (sizeof (int) * num);
      Set_2array (s, buf);

      for (i = 0; i < num; i++)
        {
          pf_oper =
            (PF_OPER *) HashTable_find_or_null (pred_flow->hash_oper_pfoper,
                                                buf[i]);
          if (!pf_oper)
            L_punt ("EMN_debug_print: pf_oper %d not in hash\n", buf[i]);
          printf (" %d", pf_oper->oper->id);
        }
      Lcode_free (buf);
    }

  printf (")\n");
  return;
}


void
D_print_memset_rid (PRED_FLOW * pred_flow, Set memset)
{
  PF_OPERAND *pf_operand;
  int *buf, num, i;

  if (!pred_flow || !memset)
    L_punt ("D_get_mem_oper_RIN_set_rid: received NULL!\n");

  num = Set_size (memset);
  printf ("( ");

  if (num)
    {
      buf = (int *) Lcode_malloc (sizeof (int) * num);
      Set_2array (memset, buf);
      for (i = 0; i < num; i++)
        {
          pf_operand =
            (PF_OPERAND *) HashTable_find_or_null (pred_flow->hash_pf_operand,
                                                   buf[i]);
          if (!pf_operand)
            L_punt ("D_get_mem_oper_RIN_set_rid: pf_operand %d not in hash\n",
                    buf[i]);

          printf ("%d ", pf_operand->pf_oper->oper->id);
        }
      Lcode_free (buf);
    }

  printf (")\n");
  return;
}
