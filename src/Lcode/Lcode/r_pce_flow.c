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
 *      File:   r_pce_flow.c
 *      Author: Shane Ryoo, Wen-mei Hwu
 *      Creation Date:  June 2003
 *      Split all PCE dataflows out from r_dataflow.c
\****************************************************************************/

#include <config.h>
#include <Lcode/l_main.h>

#undef PRINT_EXPRESSIONS
#undef PRINT_ASSIGNMENTS

#undef DISPLAY_BB_INFO
#undef DEBUG_PRE_ANALYSIS
#undef DEBUG_PRE_OPERAND_USE_SETS
#undef DEBUG_PRE_NODE_LOCAL_SETS
#undef DEBUG_PRE_BB_LOCAL_SETS
#undef DEBUG_PRE_NODE_DS_SETS
#undef DEBUG_PRE_NODE_US_SETS
#undef DEBUG_PRE_NODE_EARL_SETS
#undef DEBUG_PRE_NODE_DELAY_SETS
#undef DEBUG_PRE_NODE_LATE_SETS
#undef DEBUG_PRE_BB_LATE_SETS
#undef DEBUG_PRE_NODE_ISOL_SETS
#undef DEBUG_PRE_BB_ISOL_SETS
#undef DEBUG_PRE_BB_FINAL_SETS

#undef DEBUG_PCE_REACHING_DEF
#undef DEBUG_MEM_CONFLICT
#undef DEBUG_PCE_MEM
#undef DEBUG_MEM_REACHING_LOCATIONS
#undef DEBUG_MEM_ANT_EXPRESSIONS

#define PRE_SPEC_RATIO 0.95
#undef DEBUG_PRE_SPEC_ANALYSIS
#undef DEBUG_PRE_SPEC_NODE_DS_SETS
#undef DEBUG_PRE_SPEC_NODE_US_SETS
#undef DEBUG_PRE_SPEC_NODE_EARL_SETS
#undef DEBUG_PRE_SPEC_NODE_DELAY_SETS
#undef DEBUG_PRE_SPEC_NODE_LATE_SETS

#undef DEBUG_PDE_ANALYSIS
#undef DEBUG_PDE_NODE_LOCAL_SETS
#undef DEBUG_PDE_DEAD_SETS
#undef DEBUG_PDE_NODE_DELAY_SETS
#undef DEBUG_PDE_BB_SETS
#undef DEBUG_PDE_BB_FINAL_SETS

#undef DEBUG_PDE_PRED_ANALYSIS
#define PDE_GENERAL_STORE_PROP

#define PCE_CUTSET_RATIO .5
#define PRESSURE_EXECUTION_THRESHOLD 10
#define MAX_PRE_MOTIONS 10000
#undef DEBUG_CUTSET_METRIC

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

#define MEM_FIND_CONFLICT(pf,reg) ((Set) HashTable_find_or_null \
                                 ((pf)->hash_mem_oper_conflict, (reg)))

#define MEM_UPDATE_CONFLICT(pf,reg, opd) HashTable_update \
                                 ((pf)->hash_mem_oper_conflict, (reg), (opd))

#define RD_SET_FLAG(a,f) (a) = L_SET_BIT_FLAG((a),(f))
#define RD_CLR_FLAG(a,f) (a) = L_CLR_BIT_FLAG((a),(f))
#define RD_TST_FLAG(a,f) L_EXTRACT_BIT_VAL((a),(f))

static void D_PRE_node_correct_trans (PRED_FLOW * pred_flow);

static void D_PRE_analysis (PRED_FLOW * pred_flow, int mode);
static void D_PCE_reaching_definition_analysis (PRED_FLOW * pred_flow,
						int mode);
static void D_PRE_speculative_analysis (PRED_FLOW * pred_flow, int mode);
void D_PRE_speculative_cut_analysis (PRED_FLOW * pred_flow, int mode,
				     int ignore);
static void D_PDE_analysis (PRED_FLOW * pred_flow, int mode);
void D_PDE_predicated_cut_analysis (PRED_FLOW * pred_flow, int mode,
				    int ignore);
static void D_reaching_mem_expression_analysis (PRED_FLOW * pred_flow,
						int mode);
static void D_anticipable_mem_expression_analysis (PRED_FLOW * pred_flow,
						   int mode);
extern void L_PRE_speculate_motion (Graph, PRED_FLOW *, Set *, int, int);
extern void L_PDE_predicate_motion (Graph, PRED_FLOW *, Set *, int, int);
extern int L_PDE_fast_assignment (L_Expression *);

/*
 * EXTERNAL INTERFACES
 * ----------------------------------------------------------------------
 */


void
D_pce_flow_analysis (PRED_FLOW * pred_flow, int mode)
{
  D_setup_BB_lists (pred_flow);

  /* Perform live variable analysis for register pressure metric. */
  if (mode & PCE_CUTSET_METRIC)
    D_dataflow_analysis (pred_flow, LIVE_VARIABLE_CB);

  if (mode & PRE)
    D_PRE_analysis (pred_flow, mode);

  if (mode & MEM_REACHING_LOCATIONS)
    D_reaching_mem_expression_analysis (pred_flow, mode);

  if (mode & (PCE_REACHING_DEFINITION | MEM_REACHING_LOCATIONS))
    D_PCE_reaching_definition_analysis (pred_flow, mode);

  if (mode & PRE_SPEC)
    D_PRE_speculative_analysis (pred_flow, mode);

  if (mode & PDE)
    D_PDE_analysis (pred_flow, mode);

  if (mode & MEM_ANT_EXPRESSIONS)
    D_anticipable_mem_expression_analysis (pred_flow, mode);

  pred_flow->poison = FALSE;
}


#ifdef DISPLAY_BB_INFO
static void
D_print_all_bb_info (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *other_pf_node;
  PF_INST *pf_inst;
  DF_PCE_INFO *pce;
  int num_nodes;

  PF_FOREACH_CB (pf_cb, PF_default_flow->list_pf_cb)
  {
    fprintf (stderr, "In CB %d:\n", pf_cb->cb->id);
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      fprintf (stderr, "\tBB starting at op %d, including nodes",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0));
      num_nodes = 0;
      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
      {
	num_nodes++;
	fprintf (stderr, " %d", pf_node->id);
	if ((pce = pf_node->info->pce_info))
	  if (!Set_empty (pce->speculate_up))
	    fprintf (stderr, "<S> ");

	List_start (pf_bb->pf_nodes_entry);
	while ((other_pf_node = (PF_NODE *) List_next (pf_bb->pf_nodes_entry)))
	  {
	    if (pf_node == other_pf_node)
	      {
		fprintf (stderr, "(F)");
		break;
	      }
	  }
	List_start (pf_bb->pf_nodes_last);
	while ((other_pf_node = (PF_NODE *) List_next (pf_bb->pf_nodes_last)))
	  {
	    if (pf_node == other_pf_node)
	      {
		fprintf (stderr, "(L)");
		break;
	      }
	  }
      }
      fprintf (stderr, "\n");
      if (num_nodes > 1)
	{
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    fprintf (stderr, "\t\tNode %d consists of ops", pf_node->id);
	    PF_FOREACH_INST (pf_inst, pf_node->pf_insts)
	      fprintf (stderr, " %d", pf_inst->pf_oper->oper->id);
	    fprintf (stderr, " , preds");
	    PF_FOREACH_NODE (other_pf_node, pf_node->pred)
	      fprintf (stderr, " %d", other_pf_node->id);
	    fprintf (stderr, ", and succs");
	    PF_FOREACH_NODE (other_pf_node, pf_node->succ)
	      fprintf (stderr, " %d", other_pf_node->id);
	    fprintf (stderr, "\n");
	  }
	}
    }
  }
}
#endif


/*
 * PARTIAL REDUNDANCY ELIMINATION ANALYSIS
 * ----------------------------------------------------------------------
 */


/* This function computes all expressions and operand relations */
/* SER 11/6/02: added support for expression kill sets for PRE  */
/* on memory operations. */

static void
D_compute_expressions_and_operand_use_sets (PRED_FLOW * pred_flow, int mode)
{
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;
  Set PRE_operand, tmp = NULL;
  L_Oper *oper;
  L_Attr * attr = NULL;
  int reg, expr_index, mem_flag = 0, short_flag = 0,
    pei_flag = 0, mem_copy_prop_flag = 0, diff_load_types_flag = 0;
#ifdef PRINT_EXPRESSIONS
  int i;
#endif

  if (pred_flow->hash_RD_operand_use)
    HashTable_reset_func (pred_flow->hash_RD_operand_use,
			  (void (*)(void *)) Set_dispose);
  else
    pred_flow->hash_RD_operand_use = HashTable_create (2048);

  if (mode & (PCE_MEM | MEM_REACHING_LOCATIONS | MEM_ANT_EXPRESSIONS))
    {
      mem_flag = 1;
      if (mode & (MEM_REACHING_LOCATIONS | MEM_ANT_EXPRESSIONS))
	short_flag = 1;
      if (mode & PCE_MEM_COPY_PROP)
	mem_copy_prop_flag = 1;
      if (mode & PRE_LOAD_DIFF_TYPES)
	diff_load_types_flag = 1;
    }
  if ((mode & PRE_SPEC) && (mode & PRE_ONLY_EXCEPTING))
    pei_flag = 1;

  RD_SET_CLEAR (pred_flow->expression_U);
  RD_SET_CLEAR (pred_flow->mem_U);
  RD_SET_CLEAR (pred_flow->store_U);
  RD_SET_CLEAR (pred_flow->mem_stack_U);
  RD_SET_CLEAR (pred_flow->local_var_U);
  RD_SET_CLEAR (pred_flow->fragile_U);	/* hold PEIs for speculative PRE */

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_OPER (pf_oper, pf_cb->pf_opers)
    {
      oper = pf_oper->oper;

      if (!(oper->src[0]))
	continue;

      /* check if expression is one that is optimizable */
      if ((oper->opc == Lop_DEFINE) || (oper->opc == Lop_NO_OP) ||
	  L_is_control_oper (oper) ||
	  L_general_pred_comparison_opcode (oper) ||
	  L_initializing_predicate_define_opcode (oper) ||
	  L_subroutine_call_opcode (oper) ||
	  L_sync_opcode (oper) || L_subroutine_return_opcode (oper))
	continue;

      if (!mem_flag)
	if (L_load_opcode (oper) || L_store_opcode (oper))
	  continue;

      /* skip move ops that aren't from constants
       * only generates more instructions: should be absorbed
       * via copy propagation
       */
      if (L_move_opcode (oper) && !L_is_constant (oper->src[0]))
	continue;

      if (L_has_unsafe_macro_src_operand (oper))
	continue;

      expr_index = L_generate_expression_for_oper (oper, short_flag);
      pred_flow->expression_U = Set_add (pred_flow->expression_U, expr_index);

      if (pei_flag && L_is_pei (oper) &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SAFE_PEI))
	pred_flow->fragile_U = Set_add (pred_flow->fragile_U, expr_index);

      /* Here, compute universe sets for all load, store, and excepting 
       * ops. */
      if (mem_flag)
	{
	  if (L_load_opcode (oper))
	    {
	      pred_flow->mem_U = Set_add (pred_flow->mem_U, expr_index);
	      if (L_stack_reference (oper))
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (attr && attr->field[0]->value.mac == L_MAC_IP)
		    pred_flow->mem_stack_U =
		      Set_add (pred_flow->mem_stack_U, expr_index);
		}
	      else if (L_is_macro (oper->src[0]) &&
		       oper->src[0]->value.mac == L_MAC_IP)
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (!attr)
		    {
		      attr = L_new_attr (STACK_ATTR_NAME, 2);
		      attr->field[0] = L_copy_operand (oper->src[0]);
		      attr->field[1] = L_copy_operand (oper->src[1]);
		      oper->attr = L_concat_attr (oper->attr, attr);
		    }
		  oper->flags = L_SET_BIT_FLAG (oper->flags,
						L_OPER_STACK_REFERENCE);
		  pred_flow->mem_stack_U =
		    Set_add (pred_flow->mem_stack_U, expr_index);
		}
	      if (diff_load_types_flag)
		{
		  tmp = L_create_complement_load_expressions (oper);
		  pred_flow->expression_U =
		    Set_union_acc (pred_flow->expression_U, tmp);
		  pred_flow->mem_U = Set_union_acc (pred_flow->mem_U, tmp);

		  if (L_stack_reference (oper))
		    {
		      if (attr && (attr->field[0]->value.mac == L_MAC_OP ||
				   attr->field[0]->value.mac == L_MAC_IP))
			pred_flow->mem_stack_U =
			  Set_union_acc (pred_flow->mem_stack_U, tmp);
		    }
		  Set_dispose (tmp);
		}
	    }
	  else if (L_store_opcode (oper))
	    {
	      pred_flow->store_U = Set_add (pred_flow->store_U, expr_index);
	      pred_flow->fragile_U = Set_add (pred_flow->fragile_U,
					      expr_index);
	      pred_flow->mem_U = Set_add (pred_flow->mem_U, expr_index);

	      if (L_stack_reference (oper))
		{
		  if (!(attr = L_find_attr (oper->attr, STACK_ATTR_NAME)))
		    {
		      if (L_is_macro (oper->src[0]) &&
			  (oper->src[0]->value.mac == L_MAC_OP ||
			   oper->src[0]->value.mac == L_MAC_LV))
		      {
			attr = L_new_attr (STACK_ATTR_NAME, 2);
			attr->field[0] = L_copy_operand (oper->src[0]);
			attr->field[1] = L_copy_operand (oper->src[1]);
			oper->attr = L_concat_attr (oper->attr, attr);
		      }
		    }
		  if (attr && attr->field[0]->value.mac == L_MAC_OP)
		    pred_flow->mem_stack_U =
		      Set_add (pred_flow->mem_stack_U, expr_index);
		  else if (attr && attr->field[0]->value.mac == L_MAC_LV)
		    pred_flow->local_var_U =
		      Set_add (pred_flow->local_var_U, expr_index);
		}
	      else if (L_is_macro (oper->src[0]))
		{
		  if (oper->src[0]->value.mac == L_MAC_OP)
		    {
		      attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		      if (!attr)
			{
			  attr = L_new_attr (STACK_ATTR_NAME, 2);
			  attr->field[0] = L_copy_operand (oper->src[0]);
			  attr->field[1] = L_copy_operand (oper->src[1]);
			  oper->attr = L_concat_attr (oper->attr, attr);
			}
		      oper->flags = L_SET_BIT_FLAG (oper->flags,
						    L_OPER_STACK_REFERENCE);
		      pred_flow->mem_stack_U =
			Set_add (pred_flow->mem_stack_U, expr_index);
		    }
		  else if (oper->src[0]->value.mac == L_MAC_LV)
		    {
		      pred_flow->local_var_U =
			Set_add (pred_flow->local_var_U, expr_index);
		    }
		}
	      /* Need to generate complementary loads for store ops */
	      if (mem_copy_prop_flag)
		{
		  tmp = L_create_complement_load_expressions (oper);
		  pred_flow->expression_U =
		    Set_union_acc (pred_flow->expression_U, tmp);
		  pred_flow->mem_U = Set_union_acc (pred_flow->mem_U, tmp);

		  if (L_stack_reference (oper))
		    {
		      if (attr && (attr->field[0]->value.mac == L_MAC_OP ||
				   attr->field[0]->value.mac == L_MAC_IP))
			pred_flow->mem_stack_U =
			  Set_union_acc (pred_flow->mem_stack_U, tmp);
		    }
		  Set_dispose (tmp);
		}
	    }
	}

      /* Note a situation here: src[2] of a store operation doesn't matter
       * for reaching_mem_expression or anticipable_mem_expression
       * analysis.
       */
      PF_FOREACH_OPERAND (pf_operand, pf_oper->src)
      {
	if (short_flag && L_store_opcode (oper))
	  {
	    L_Operand *operand = pf_operand->operand;
	    if (!(L_same_operand (operand, oper->src[0]) ||
		  L_same_operand (operand, oper->src[1])))
	      continue;
	  }
	reg = pf_operand->reg;
	PRE_operand = RD_FIND_OPD_USE (pred_flow, reg);
	PRE_operand = Set_add (PRE_operand, expr_index);
	RD_UPDATE_OPD_USE (pred_flow, reg, PRE_operand);
      }
    }
  }

#ifdef PRINT_EXPRESSIONS
  for (i = 1; i <= L_fn->n_expression; i++)
    {
      L_Expression_Hash_Entry *entry;
      entry =
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      L_print_expression (stderr, entry->expression);
    }
#endif

#ifdef DEBUG_PRE_OPERAND_USE_SETS
  fprintf (stderr, "%d expressions found.\n", L_fn->n_expression);
  for (i = 0; i < 2048; i++)
    {
      PRE_operand = RD_FIND_OPD_USE (pred_flow, i);
      if (!Set_empty (PRE_operand))
	{
	  if (i % 2)
	    fprintf (stderr, "Use of mac $P%d: ", i >> 1);
	  else
	    fprintf (stderr, "Use of r %d: ", (i >> 1));
	  Set_print (stderr, "USE", PRE_operand);
	}
    }
#endif
}


static void
D_PRE_node_local_info (PRED_FLOW * pred_flow, int mode, int * max_motions)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  DF_PCE_INFO *pce;
  Set trans, n_comp, x_comp, complement, expression_U, store_U = NULL,
    tmp, eligible;
  L_Expression *expression, *orig_expression;
  L_Oper *oper, *dep_oper;
  int i, reg, token, expr_index, mem_flag = 0, mem_conservative_flag = 0,
    is_trans, cutset_metric_flag = 0, mem_copy_prop_flag = 0,
    reg_threshold = 0, max_noncut_regs = 0, diff_load_types_flag = 0;

  expression_U = pred_flow->expression_U;

  if (mode & PCE_MEM)
    {
      mem_flag = 1;
      if ((mode & PCE_MEM_CONSERVATIVE) ||
	  !(L_func_acc_specs ||
	    (L_func_contains_dep_pragmas && L_use_sync_arcs)))
	{
#ifdef DEBUG_MEM_CONFLICT
	  fprintf (stderr, "PRE: Using conservative memory alias assumptions "
		   "in function %s.\n", pred_flow->fn->name);
#endif
	  mem_conservative_flag = 1;
	}
#ifdef DEBUG_MEM_CONFLICT
      else if (L_func_acc_specs)
	fprintf (stderr, "PRE: Using AccSpecs for memory alias info in "
		 "function %s.\n", pred_flow->fn->name);
      else if (L_func_contains_dep_pragmas && L_use_sync_arcs)
	fprintf (stderr, "PRE: Using sync arcs for memory alias info in "
		 "function %s.\n", pred_flow->fn->name);
#endif
      if (mode & PCE_MEM_COPY_PROP)
	mem_copy_prop_flag = 1;
      if (mode & PRE_LOAD_DIFF_TYPES)
	diff_load_types_flag = 1;
    }
  if (mode & PCE_CUTSET_METRIC)
    {
      cutset_metric_flag = 1;
      store_U = pred_flow->store_U;
      reg_threshold = M_num_registers (M_native_int_register_ctype ()) *
	PCE_CUTSET_RATIO;
    }

  /* compute transparent and n_comp sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    if (!(pf_node->info->pce_info))
      pf_node->info->pce_info = D_new_df_pce_info ();
    pce = pf_node->info->pce_info;

    trans = Set_copy (expression_U);
    n_comp = NULL;

    PF_FORHCAE_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;

      if (L_is_control_oper (oper))
	continue;
      if (oper->opc == Lop_NO_OP)
	continue;
      /* wipe out trans & n_comp at synchronization operations */
      if (L_sync_opcode (oper) || L_subroutine_return_opcode (oper))
	{
	  RD_SET_CLEAR (trans);
	  RD_SET_CLEAR (n_comp);
	  continue;
	}

      /* first remove expressions using redefed      */
      /* operands from trans, n_comp, and complement */
      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);

	trans = Set_subtract_acc (trans, tmp);
	n_comp = Set_subtract_acc (n_comp, tmp);
      }

      if (oper->opc == Lop_DEFINE)
	continue;
      /* currently can't reverse propagate globally, so can't
         get rid of moves to macros */
      if (L_move_opcode (oper) && L_is_macro (oper->dest[0]))
	continue;

      if (mem_flag)
	{
	  /* remove dependent memory expressions  */
	  if (L_load_opcode (oper) || L_store_opcode (oper) ||
	      L_subroutine_call_opcode (oper))
	    {
	      token = L_generate_expression_token_from_oper (oper);
	      orig_expression =
		L_find_oper_expression_in_hash
		(L_fn->expression_token_hash_tbl, token, oper, 0);

	      if (mem_conservative_flag)
		{
		  if (L_load_opcode (oper) ||
		      (L_subroutine_call_opcode (oper) &&
		       L_side_effect_free_sub_call (oper)))
		    {
		      trans = Set_subtract_acc (trans, store_U);
		      n_comp = Set_subtract_acc (n_comp, store_U);
		    }
		  else
		    {
		      if (orig_expression)
			if (Set_in (trans, orig_expression->index))
			  is_trans = 1;
			else
			  is_trans = 0;
		      else
			is_trans = 0;
		      trans = Set_subtract_acc (trans, pred_flow->mem_U);
		      n_comp = Set_subtract_acc (n_comp, pred_flow->mem_U);
		      if (is_trans)
			trans = Set_add (trans, orig_expression->index);
		    }
		}
	      else if (L_func_acc_specs)
		{
		  Set conflicts;

		  if (L_load_opcode (oper) || L_store_opcode (oper))
		    conflicts = orig_expression->conflicts;
		  else /* subroutine call, check all expressions */
		    conflicts = L_get_jsr_conflicting_expressions (oper);

		  trans = Set_subtract_acc (trans, conflicts);
		  n_comp = Set_subtract_acc (n_comp, conflicts);

		  if (L_subroutine_call_opcode (oper))
		    RD_SET_CLEAR (conflicts);
		}
	      else if (oper->sync_info)  /* sync arcs case */
		{
		  for (i = 0; i < oper->sync_info->num_sync_out; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		      if (L_subroutine_call_opcode (dep_oper) ||
			  L_subroutine_return_opcode (dep_oper))
			continue;
		      token =
			L_generate_expression_token_from_oper (dep_oper);
		      expression = L_find_oper_expression_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!expression)
			continue;	/* might have unsafe operand, etc. */
		      if (orig_expression == expression)
			continue;
		      expr_index = expression->index;
		      trans = Set_delete (trans, expr_index);
		      n_comp = Set_delete (n_comp, expr_index);
		    }
		  for (i = 0; i < oper->sync_info->num_sync_in; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		      if (L_subroutine_call_opcode (dep_oper) ||
			  L_subroutine_return_opcode (dep_oper))
			continue;
		      token =
			L_generate_expression_token_from_oper (dep_oper);
		      expression = L_find_oper_expression_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!expression)
			continue;	/* might have unsafe operand, etc. */
		      if (orig_expression == expression)
			continue;
		      expr_index = expression->index;
		      trans = Set_delete (trans, expr_index);
		      n_comp = Set_delete (n_comp, expr_index);
		    }
		}
	    }

	  /* kill stack references across non-side-effect-free jsrs */
	  if (L_subroutine_call_opcode (oper))
	    {
	      if (!L_side_effect_free_sub_call (oper))
		{
		  trans = Set_subtract_acc (trans, pred_flow->mem_stack_U);
		  n_comp = Set_subtract_acc (n_comp, pred_flow->mem_stack_U);
		}
	    }

	  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	    continue;
	  /* SER 20050215: Block moves missing annotations, don't try
	   * to optimize at this time. */
	  if (L_find_attr (oper->attr, "block_move"))
	    continue;
	}
      else if (L_load_opcode (oper) || L_store_opcode (oper))
	continue;

      if (!(pf_inst->pred_true))
	continue;

      if (!(oper->src[0]))
	continue;

      if (L_subroutine_call_opcode (oper))
	continue;

      token = L_generate_expression_token_from_oper (oper);
      expression =
	L_find_oper_expression_in_hash (L_fn->expression_token_hash_tbl,
					token, oper, 0);

      if (expression != NULL)
	n_comp = Set_add (n_comp, expression->index);
    }

    RD_SET_CLEAR (pce->n_comp);
    pce->n_comp = n_comp;

    /* compute x_comp and complement sets */
    x_comp = NULL;
    eligible = NULL;
    complement = NULL;

    PF_FOREACH_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;

      if (L_is_control_oper (oper))
	continue;
      if (oper->opc == Lop_NO_OP)
	continue;
      /* wipe out x_comp and complement at synchronization operations */
      if (L_sync_opcode (oper))
	{
	  RD_SET_CLEAR (x_comp);
	  RD_SET_CLEAR (complement);
	  eligible = Set_union_acc (eligible, expression_U);
	  continue;
	}

      /* remove dependent memory expressions  */
      if (mem_flag)
	{
	  if (L_load_opcode (oper) || L_store_opcode (oper) ||
	      L_subroutine_call_opcode (oper))
	    {
	      token = L_generate_expression_token_from_oper (oper);
	      orig_expression =
		L_find_oper_expression_in_hash
		(L_fn->expression_token_hash_tbl, token, oper, 0);

	      if (mem_conservative_flag)
		{
		  if (L_load_opcode (oper) ||
		      (L_subroutine_call_opcode (oper) &&
		       L_side_effect_free_sub_call (oper)))
		    {
		      x_comp = Set_subtract_acc (x_comp, store_U);
		      eligible = Set_union_acc (eligible, store_U);
		    }
		  else
		    {
		      int not_eligible;

		      if (orig_expression)
			if (!Set_in (eligible, orig_expression->index))
			  not_eligible = 1;
			else
			  not_eligible = 0;
		      else
			not_eligible = 0;
		      x_comp = Set_subtract_acc (x_comp, pred_flow->mem_U);
		      complement = Set_subtract_acc (complement,
						     pred_flow->mem_U);
		      eligible = Set_union_acc (eligible, pred_flow->mem_U);
		      if (not_eligible)
			eligible = Set_delete (eligible,
					       orig_expression->index);
		    }
		}
	      else if (L_func_acc_specs)
		{
		  Set conflicts;

		  if (!L_subroutine_call_opcode (oper))
		    conflicts = orig_expression->conflicts;
		  else
		    conflicts = L_get_jsr_conflicting_expressions (oper);

		  x_comp = Set_subtract_acc (x_comp, conflicts);
		  complement = Set_subtract_acc (complement, conflicts);
		  eligible = Set_union_acc (eligible, conflicts);

		  if (L_subroutine_call_opcode (oper))
		    RD_SET_CLEAR (conflicts);
		}
	      else if (oper->sync_info)  /* sync arcs case */
		{
		  for (i = 0; i < oper->sync_info->num_sync_out; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		      if (L_subroutine_call_opcode (dep_oper) ||
			  L_subroutine_return_opcode (dep_oper))
			continue;
		      token =
			L_generate_expression_token_from_oper (dep_oper);
		      expression = L_find_oper_expression_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!expression)
			continue;	/* might have unsafe operand, etc. */
		      if (orig_expression == expression)
			continue;
		      expr_index = expression->index;
		      x_comp = Set_delete (x_comp, expr_index);
		      complement = Set_delete (complement, expr_index);
		      eligible = Set_add (eligible, expr_index);
		    }
		  for (i = 0; i < oper->sync_info->num_sync_in; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		      if (L_subroutine_call_opcode (dep_oper) ||
			  L_subroutine_return_opcode (dep_oper))
			continue;
		      token =
			L_generate_expression_token_from_oper (dep_oper);
		      expression = L_find_oper_expression_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!expression)
			continue;	/* might have unsafe operand, etc. */
		      if (orig_expression == expression)
			continue;
		      expr_index = expression->index;
		      x_comp = Set_delete (x_comp, expr_index);
		      complement = Set_delete (complement, expr_index);
		      eligible = Set_add (eligible, expr_index);
		    }
		}
	    }

	  /* kill stack references across non-side-effect-free jsrs */
	  if (L_subroutine_call_opcode (oper) &&
	      !L_side_effect_free_sub_call (oper))
	    {
	      x_comp = Set_subtract_acc (x_comp, pred_flow->mem_stack_U);
	      complement = Set_subtract_acc (complement,
					     pred_flow->mem_stack_U);
	      eligible = Set_union_acc (eligible, pred_flow->mem_stack_U);
	    }

	}
      /* Don't 'continue' on loads: need to invalidate dest. */
      else if (L_store_opcode (oper))
	continue;

      /* don't check ops that have no sources, are defines, subroutine
       * calls, volatile, or moves to macros, also kill loads/stores if
       * not doing mem PRE, kill speculative loads if doing 
       * conservative mem PRE. */
      /* 04/21/03 SER: Do not include operations with checks: can't migrate
       * recovery code in PRE due to multiple paths. */
      /* SER 20050215: don't try to optimize block moves at this time */
      if (pf_inst->pred_true && (oper->src[0]) && ((oper->opc) != Lop_DEFINE)
	  && !(L_subroutine_call_opcode (oper)) &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE) &&
	  !(L_move_opcode (oper) && L_is_macro (oper->dest[0])) &&
	  (mem_flag || !(L_load_opcode (oper) || L_store_opcode (oper)))
	  && !L_find_attr (oper->attr, "block_move"))
	{
	  /* first add expressions that are computed */
	  token = L_generate_expression_token_from_oper (oper);
	  expression =
	    L_find_oper_expression_in_hash (L_fn->expression_token_hash_tbl,
					    token, oper, 0);

	  if (expression != NULL)
	    {
	      if (Set_in (eligible, expression->index))
		{
		  x_comp = Set_add (x_comp, expression->index);
		  if (L_load_opcode (expression))
		    complement = Set_delete (complement, expression->index);
		}
	      /* For stores, add complemented loads to x_comp and
	       * complement sets. Eligibility is not necessary. */
	      if (mem_copy_prop_flag && L_store_opcode (expression))
		{
		  tmp = L_create_complement_load_expressions (oper);
		  x_comp = Set_union_acc (x_comp, tmp);
		  trans = Set_subtract_acc (trans, tmp);
		  complement = Set_union_acc (complement, tmp);
		  Set_dispose (tmp);
                }
	      if (diff_load_types_flag && L_load_opcode (expression))
		{
		  tmp = L_create_complement_load_expressions (oper);
		  x_comp = Set_union_acc (x_comp, tmp);
		  complement = Set_union_acc (complement, tmp);
		  Set_dispose (tmp);
		}
	    }
	}

      /* last, remove expressions using redefed operands from x_comp */
      /* also, mark eligible operations */
      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);

	x_comp = Set_subtract_acc (x_comp, tmp);
	complement = Set_subtract_acc (complement, tmp);
	eligible = Set_union_acc (eligible, tmp);
      }
    }

    /* Cutset metric. */
    if (cutset_metric_flag &&
	pf_node->pf_cb->cb->weight > PRESSURE_EXECUTION_THRESHOLD)
      {
	int num_regs = Set_size (pf_node->pf_cb->info->v_in);
	if (num_regs >= reg_threshold)
	  {
	    trans = Set_intersect_acc (trans, store_U);
#ifdef DEBUG_CUTSET_METRIC
	    fprintf (stderr, "PRE cutset metric: limiting motion through cb "
		     "%d, with %d lives.\n", pf_node->pf_cb->cb->id, num_regs);
#endif
	  }
	else
	  {
	    if (max_noncut_regs < num_regs)
	      max_noncut_regs = num_regs;
#ifdef DEBUG_CUTSET_METRIC
	    fprintf (stderr, "PRE cutset metric: free motion through cb %d, "
		     "with %d lives.\n", pf_node->pf_cb->cb->id, num_regs);
#endif
	  }
      }

    RD_SET_CLEAR (pce->x_comp);
    RD_SET_CLEAR (pce->trans);
    RD_SET_CLEAR (pce->complement);
    pce->x_comp = x_comp;
    pce->trans = trans;
    pce->complement = complement;
    RD_SET_CLEAR (eligible);

#ifdef DEBUG_PRE_NODE_LOCAL_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tTRANS", pce->trans);
    Set_print (stderr, "\tN_COMP", pce->n_comp);
    Set_print (stderr, "\tX_COMP", pce->x_comp);
    Set_print (stderr, "\tCOMPLEMENT", pce->complement);
#endif
  }

  if (cutset_metric_flag)
    {
      *max_motions = reg_threshold - max_noncut_regs;
#ifdef DEBUG_CUTSET_METRIC
      fprintf (stderr, "Max number of SPRE motions: %d\n", *max_motions);
#endif
    }
}


/*! \brief This function collects all the transparent information for nodes
 * in each BB and applies it to the first and last nodes, to make motion
 * conservative.  This is necessary for creating safe speculative sets for
 * SPRE and prevent problems with earliest sets in Knoop PRE
 *
 * \note  This will make calculation of n_comp and x_comp extremely
 * conservative, so do not run it until after calculating local BB sets.
 */
static void
D_PRE_node_correct_trans (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node_first, *pf_node_last, *pf_node;
  DF_PCE_INFO *pce;
  Set trans, expression_U;
  int single_first, single_last;

  expression_U = pred_flow->expression_U;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      /* SER 20041204: changes for multiple entry nodes. */
      single_first = (List_size (pf_bb->pf_nodes_entry) == 1);
      single_last = (List_size (pf_bb->pf_nodes_last) == 1);

      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);

      if (single_first && single_last && pf_node_first == pf_node_last)
	continue;

      trans = Set_copy (expression_U);
      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	trans = Set_intersect_acc (trans, pf_node->info->pce_info->trans);

      pce = pf_node_first->info->pce_info;
      pce->trans = Set_intersect_acc (pce->trans, trans);

      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_entry)
      {
	pce = pf_node->info->pce_info;
	pce->trans = Set_intersect_acc (pce->trans, trans);
      }

      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
      {
	pce = pf_node->info->pce_info;
	pce->trans = Set_intersect_acc (pce->trans, trans);
      }
      Set_dispose (trans);
    }
  }
}


static int
Node_In_List (PF_NODE * pf_node, List list)
{
  PF_NODE * list_node;

  PF_FOREACH_NODE (list_node, list)
    {
      if (list_node == pf_node)
	return 1;
    }
  return 0;
}


/*! \brief This function propagates information for BBs in the presence of
 * predication.  It moves computations to entry/exit nodes and prevents
 * motion of expressions through the BB if it is blocked on some paths.
 *
 * \note  There are several cases to take note of:
 *        - Blocks that have conditional redef of an expression's source(s).
 *        - Expressions that modify their own destination in the block.
 *        Dealing with the computation and transparency of such expressions
 *        must be done carefully.
 */
void
D_PRE_bb_local_info (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node_first, *pf_node_last;
  DF_PCE_INFO *bb_pce, *node_pce;
  Set expression_U;
  int size, single_first, single_last;

  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      if (!(pf_bb->info->pce_info))
	{
	  pf_bb->info->pce_info = D_new_df_pce_info ();
	  bb_pce = pf_bb->info->pce_info;
	}
      else
	{
	  bb_pce = pf_bb->info->pce_info;
	  RD_SET_CLEAR (bb_pce->trans);
	  RD_SET_CLEAR (bb_pce->n_comp);
	  RD_SET_CLEAR (bb_pce->x_comp);
	  RD_SET_CLEAR (bb_pce->complement);
	}

      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);

      /* SER 20041204: changes for multiple entry nodes. */
      single_first = (List_size (pf_bb->pf_nodes_entry) == 1);
      single_last = (List_size (pf_bb->pf_nodes_last) == 1);

      if (single_first && single_last && pf_node_first == pf_node_last)
	{
	  node_pce = pf_node_first->info->pce_info;
	  bb_pce->trans = Set_copy (node_pce->trans);
	  bb_pce->n_comp = Set_copy (node_pce->n_comp);
	  bb_pce->x_comp = Set_copy (node_pce->x_comp);
	  bb_pce->complement = Set_copy (node_pce->complement);
	}
      else
	{
	  PF_NODE * pf_node, *pred_pf_node, *succ_pf_node;
	  Set trans, in, out, n_comp_set, x_comp_set, complement_set;
	  int change;

	  trans = Set_copy (expression_U);

	  /* Compute expressions which are effectively n_comp for the BB */
	  /* propagate n_comp, gather trans */
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    node_pce = pf_node->info->pce_info;
	    RD_SET_CLEAR (pf_node->info->in);
	    pf_node->info->in = Set_copy (node_pce->n_comp);
	    RD_SET_CLEAR (pf_node->info->out);
	    pf_node->info->out = Set_copy (expression_U);
	    pf_node->info->cnt = size;
	    trans = Set_intersect_acc (trans, node_pce->trans);
	  }
	  bb_pce->trans = trans;

	  do
	    {
	      change = 0;
	      PF_FORHCAE_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt = 0;

		/* Do not propagate information in last nodes */
		if (single_last)
		  {
		    if (pf_node == pf_node_last)
		      continue;
		  }
		else if (Node_In_List (pf_node, pf_bb->pf_nodes_last))
		  continue;

		out = pf_node->info->out;
		PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
		  out = Set_intersect_acc (out, succ_pf_node->info->in);

		cnt = Set_size (out);
		pf_node->info->out = out;

		if (cnt < pf_node->info->cnt)
		  {
		    node_pce = pf_node->info->pce_info;
		    pf_node->info->cnt = cnt;
		    in = Set_intersect (node_pce->trans, out);
		    in = Set_union_acc (in, node_pce->n_comp);
		    if (!Set_subtract_empty (in, pf_node->info->in))
		      change++;
		    RD_SET_CLEAR (pf_node->info->in);
		    pf_node->info->in = in;
		  }
	      }
	    }
	  while (change);

	  /* The n_comp set of the BB is the intersection of the entry nodes. */
	  /* SER 20050210: Track final n_comp set, add to first nodes. */
	  if (single_first)
	    {
	      bb_pce->n_comp = Set_copy (pf_node_first->info->in);
	      n_comp_set = Set_copy (pf_node_first->info->in);
	      node_pce = pf_node_first->info->pce_info;
	      node_pce->n_comp = Set_union_acc (node_pce->n_comp, n_comp_set);
	    }
	  else
	    {
	      bb_pce->n_comp = Set_copy (expression_U);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_entry)
		bb_pce->n_comp = Set_intersect_acc (bb_pce->n_comp,
						    pf_node->info->in);
	      n_comp_set = Set_copy (bb_pce->n_comp);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_entry)
		{
		  node_pce = pf_node->info->pce_info;
		  node_pce->n_comp = Set_union_acc (node_pce->n_comp,
						    n_comp_set);
		}
	    }

	  /* Propagate eligibility info. Use isolated sets to hold. */
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    node_pce = pf_node->info->pce_info;
	    RD_SET_CLEAR (node_pce->n_isolated);
	    RD_SET_CLEAR (node_pce->x_isolated);
	    node_pce->x_isolated = Set_subtract (expression_U,
						 node_pce->trans);
	    pf_node->info->cnt = 0;
	  }

	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt;

		/* SER 20041204: prevent propagation on multiple entry nodes */
		if (single_first)
		  {
		    if (pf_node == pf_node_first)
		      continue;
		  }
		else if (Node_In_List (pf_node, pf_bb->pf_nodes_entry))
		  continue;

		node_pce = pf_node->info->pce_info;
		in = node_pce->n_isolated;
		PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
		  in = Set_union_acc
		  (in, pred_pf_node->info->pce_info->x_isolated);
		cnt = Set_size (in);
		pf_node->info->pce_info->n_isolated = in;

		if (cnt > pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    out = Set_subtract (expression_U, node_pce->trans);
		    out = Set_union_acc (out, in);
		    if (!Set_subtract_empty (out, node_pce->x_isolated))
		      change++;
		    RD_SET_CLEAR (node_pce->x_isolated);
		    node_pce->x_isolated = out;
		  }
	      }
	    }
	  while (change);

	  /* propagate x_comp */
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    node_pce = pf_node->info->pce_info;
	    RD_SET_CLEAR (pf_node->info->out);
	    pf_node->info->out =
	      Set_intersect (node_pce->n_comp, node_pce->n_isolated);
	    pf_node->info->out = Set_intersect_acc (pf_node->info->out,
						    node_pce->trans);
	    pf_node->info->out = Set_union_acc (pf_node->info->out,
						node_pce->x_comp);
	    RD_SET_CLEAR (pf_node->info->in);
	    pf_node->info->in = Set_copy (expression_U);
	    pf_node->info->cnt = size;
	  }

	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt;

		if (single_first)
		  {
		    if (pf_node == pf_node_first)
		      continue;
		  }
		else if (Node_In_List (pf_node, pf_bb->pf_nodes_entry))
		  continue;

		in = pf_node->info->in;
		PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
		  in = Set_intersect_acc (in, pred_pf_node->info->out);

		cnt = Set_size (in);
		pf_node->info->in = in;

		if (cnt < pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    out = Set_intersect (pf_node->info->pce_info->trans, in);
		    out =
		      Set_union_acc (out, pf_node->info->pce_info->x_comp);
		    if (!Set_subtract_empty (out, pf_node->info->out))
		      change++;
		    RD_SET_CLEAR (pf_node->info->out);
		    pf_node->info->out = out;
		  }
	      }
	    }
	  while (change);

	  /* Copy out x_comp set */
	  /* SER 20050210: Track final x_comp set, add to last nodes. */
	  if (single_last)
	    {
	      bb_pce->x_comp = Set_copy (pf_node_last->info->out);
	      x_comp_set = Set_copy (pf_node_last->info->out);
	      node_pce = pf_node_last->info->pce_info;
	      node_pce->x_comp = Set_union_acc (node_pce->x_comp,
						x_comp_set);
	    }
	  else
	    {
	      bb_pce->x_comp = Set_copy (expression_U);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
		bb_pce->x_comp = Set_intersect_acc (bb_pce->x_comp,
						    pf_node->info->out);
	      x_comp_set = Set_copy (bb_pce->x_comp);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
		{
		  node_pce = pf_node->info->pce_info;
		  node_pce->x_comp = Set_union_acc (node_pce->x_comp,
						    x_comp_set);
		}
	    }

	  /* propagate complement info */
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    node_pce = pf_node->info->pce_info;
	    RD_SET_CLEAR (pf_node->info->out);
	    pf_node->info->out = Set_copy (node_pce->complement);
	    RD_SET_CLEAR (pf_node->info->in);
	    pf_node->info->cnt = 0;
	  }

	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt;

		if (single_first)
		  {
		    if (pf_node == pf_node_first)
		      continue;
		  }
		else if (Node_In_List (pf_node, pf_bb->pf_nodes_entry))
		  continue;

		in = pf_node->info->in;
		PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
		  in = Set_union_acc (in, pred_pf_node->info->out);

		cnt = Set_size (in);
		pf_node->info->in = in;

		if (cnt > pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    node_pce = pf_node->info->pce_info;
		    out = Set_intersect (node_pce->trans, in);
		    out = Set_union_acc (out, node_pce->complement);
		    if (!Set_subtract_empty (out, pf_node->info->out))
		      change++;
		    RD_SET_CLEAR (pf_node->info->out);
		    pf_node->info->out = out;
		  }
	      }
	    }
	  while (change);

	  /* Copy out complement set */
	  /* SER 20050210: Track final complement set, add to last nodes. */
	  if (single_last)
	    {
	      bb_pce->complement = Set_copy (pf_node_last->info->out);
	      complement_set = Set_copy (pf_node_last->info->out);
	      node_pce = pf_node_last->info->pce_info;
	      node_pce->complement = Set_union_acc (node_pce->complement,
						    complement_set);
	    }
	  else
	    {
	      bb_pce->complement = Set_copy (expression_U);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
		bb_pce->complement =
		Set_intersect_acc (bb_pce->complement, pf_node->info->out);
	      complement_set = Set_copy (bb_pce->complement);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
		{
		  node_pce = pf_node->info->pce_info;
		  node_pce->complement = Set_union_acc (node_pce->complement,
							complement_set);
		}
	    }

	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    int first_node;

	    node_pce = pf_node->info->pce_info;
	    RD_SET_CLEAR (pf_node->info->in);
	    RD_SET_CLEAR (pf_node->info->out);
	    RD_SET_CLEAR (node_pce->n_isolated);
	    RD_SET_CLEAR (node_pce->x_isolated);

	    /* SER 20050210: remove BB n_comp, x_comp, complement sets from
	     * nodes which aren't entry/exit points of the BB.  Note that
	     * they've already been added to the entry/exit nodes.  This avoids
	     * the "missing latest" bug that results when a computation
	     * in the middle of the BB gets lost due to trans correction. */
	    if (!(first_node = Node_In_List (pf_node, pf_bb->pf_nodes_entry)))
	      node_pce->n_comp = Set_subtract_acc (node_pce->n_comp,
						   n_comp_set);
	    if (!Node_In_List (pf_node, pf_bb->pf_nodes_last))
	      {
		if (!first_node)
		  node_pce->n_comp = Set_subtract_acc (node_pce->n_comp,
						       x_comp_set);
		node_pce->x_comp = Set_subtract_acc (node_pce->x_comp,
						     x_comp_set);
		node_pce->complement = Set_subtract_acc (node_pce->complement,
							 complement_set);
	      }
	  }

	  RD_SET_CLEAR (n_comp_set);
	  RD_SET_CLEAR (x_comp_set);
	  RD_SET_CLEAR (complement_set);
	}

#ifdef DEBUG_PRE_BB_LOCAL_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tTRANS", bb_pce->trans);
      Set_print (stderr, "\tN_COMP", bb_pce->n_comp);
      Set_print (stderr, "\tX_COMP", bb_pce->x_comp);
      Set_print (stderr, "\tCOMPLEMENT", bb_pce->complement);
#endif
    }
  }

  /* SER 20050208: Transparency correction needed for the following case:
   * Predicated code can create a CB where an expression is
   * available/up-safe at the entry of the cb, is conditionally modified,
   * and computed again in a successor cb.  An earliest position
   * will be established in the node containing the modification, but will
   * be lost when BB sets are computed, resulting in a missing insertion
   * of the computation. */
  D_PRE_node_correct_trans (pred_flow);
}


/*! \brief Annotates BBs with up- and down-safety info for speculative PRE.
 */
static void
D_PRE_bb_local_safety_info (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *pf_node_first, *pf_node_last;
  DF_PCE_INFO *bb_pce;
  Set expression_U;
  int single_first, single_last;

  expression_U = pred_flow->expression_U;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
    {
      PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
	{
	  bb_pce = pf_bb->info->pce_info;

  	  RD_SET_CLEAR (bb_pce->nu_safe);
	  RD_SET_CLEAR (bb_pce->nd_safe);
	  RD_SET_CLEAR (bb_pce->xd_safe);

	  pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
	  pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);

	  single_first = (List_size (pf_bb->pf_nodes_entry) == 1);
	  single_last = (List_size (pf_bb->pf_nodes_last) == 1);

	  if (single_first)
	    {
	      bb_pce->nu_safe =
		Set_copy (pf_node_first->info->pce_info->nu_safe);
	      bb_pce->nd_safe =
		Set_copy (pf_node_first->info->pce_info->nd_safe);
	    }
	  else
	    {
	      bb_pce->nu_safe = Set_copy (expression_U);
	      bb_pce->nd_safe = Set_copy (expression_U);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_entry)
		{
		  bb_pce->nu_safe =
		    Set_intersect_acc (bb_pce->nu_safe,
				       pf_node->info->pce_info->nu_safe);
		  bb_pce->nd_safe =
		    Set_intersect_acc (bb_pce->nd_safe,
				       pf_node->info->pce_info->nd_safe);
		}
	    }

	  if (single_last)
	    bb_pce->xd_safe =
	      Set_copy (pf_node_last->info->pce_info->xd_safe);
	  else
	    {
	      bb_pce->xd_safe = Set_copy (expression_U);
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
		bb_pce->xd_safe =
		Set_intersect_acc (bb_pce->xd_safe,
				   pf_node->info->pce_info->xd_safe);
	    }
	}
    }
}


/*! \brief Computes down-safety, or "anticipability", of expressions for PRE. */
static void
D_PRE_node_down_safety (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *succ_pf_node;
  DF_PCE_INFO *pce, *succ_pce;
  Set nd_safe, xd_safe, expression_U;
  int change, size;

  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);

  /* set nd_safe and xd_safe to initial values */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->nd_safe);
    Set_dispose (pce->xd_safe);
    pce->nd_safe = Set_union (pce->n_comp, pce->trans);
    pce->xd_safe = Set_copy (expression_U);
    pf_node->info->cnt = size;
  }

  /* calculate down-safe sets */
  do
    {
      change = 0;
      PF_FORHCAE_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	xd_safe = pce->xd_safe;

	/* 
	 * xd_safe[S] = (x_comp) + 
	 *              intersection(nd_safe [] of all successors of S)
	 */
	succ_pce = 0;
	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	{
	  succ_pce = succ_pf_node->info->pce_info;
	  xd_safe = Set_intersect_acc (xd_safe, succ_pce->nd_safe);
	}

	/* if no successors were found, set xd_safe to empty set */
	if (succ_pce == 0)
	  RD_SET_CLEAR (xd_safe);

	xd_safe = Set_union_acc (xd_safe, pce->x_comp);

	cnt = Set_size (xd_safe);
	pce->xd_safe = xd_safe;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    /*
	     * nd_safe[S] = n_comp[S] + (trans[S] ^ xd_safe[S])
	     */
	    nd_safe = Set_intersect (pce->trans, pce->xd_safe);
	    nd_safe = Set_union_acc (nd_safe, pce->n_comp);

	    /* A change has occured if nd_safe has shrunk (monotonic) */
	    if (!Set_subtract_empty (pce->nd_safe, nd_safe))
	      change++;
	    RD_SET_CLEAR (pce->nd_safe);
	    pce->nd_safe = nd_safe;
	  }
      }
    }
  while (change);

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
#ifdef DEBUG_PRE_NODE_DS_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tND_SAFE", pf_node->info->pce_info->nd_safe);
    Set_print (stderr, "\tXD_SAFE", pf_node->info->pce_info->xd_safe);
#endif
  }
}


#if 0
/* 6/10/03 SER: This version of down safety is necessary when critical
 * edges are not split. */
static void
D_PRE_node_down_safety_uncut_critical_edges (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node, *succ_pf_node;
  DF_PCE_INFO *pce;
  Set in, nd_safe, xd_safe, expression_U;
  int change, size;

  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);

  /* set nd_safe and xd_safe to initial values */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->nd_safe);
    Set_dispose (pce->xd_safe);
    Set_dispose (pf_node->info->in);
    pce->nd_safe = Set_union (pce->n_comp, pce->trans);
    pce->xd_safe = Set_copy (expression_U);
    pf_node->info->in = Set_copy (pce->nd_safe);
    pf_node->info->cnt = size;
  }

  /* calculate down-safe sets */
  do
    {
      change = 0;
      PF_FORHCAE_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	xd_safe = pce->xd_safe;

	/* 
	 * xd_safe[S] = x_comp + 
	 *              intersection(nd_safe [] of all successors of S)
	 */
	cnt = 0;
	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	{
	  cnt = 1;
	  xd_safe = Set_intersect_acc (xd_safe, succ_pf_node->info->in);
	}

	/* if no successors were found, set xd_safe to empty set */
	if (cnt == 0)
	  RD_SET_CLEAR (xd_safe);

	xd_safe = Set_union_acc (xd_safe, pce->x_comp);
	cnt = Set_size (xd_safe);
	pce->xd_safe = xd_safe;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    /*
	     * nd_safe[S] = n_comp[S] + (trans[S] ^ xd_safe[S])
	     */
	    nd_safe = Set_intersect (pce->trans, pce->xd_safe);
	    nd_safe = Set_union_acc (nd_safe, pce->n_comp);

	    /* A change has occured if nd_safe has shrunk (monotonic) */
	    if (!Set_subtract_empty (pce->nd_safe, nd_safe))
	      change++;

	    in = Set_copy (nd_safe);
	    PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	      in = Set_intersect_acc (in,
				      pred_pf_node->info->pce_info->xd_safe);

	    if (!Set_subtract_empty (pf_node->info->in, in))
	      change++;

	    RD_SET_CLEAR (pce->nd_safe);
	    RD_SET_CLEAR (pf_node->info->in);
	    pce->nd_safe = nd_safe;
	    pf_node->info->in = in;
	  }
      }
    }
  while (change);

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
#ifdef DEBUG_PRE_NODE_DS_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tND_SAFE", pf_node->info->pce_info->nd_safe);
    Set_print (stderr, "\tXD_SAFE", pf_node->info->pce_info->xd_safe);
#endif
    RD_SET_CLEAR (pf_node->info->in);
  }
}
#endif


#if 0
/* NOTE: Not currently called; for debugging purposes. */
static void
D_PRE_bb_down_safety (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node_first, *pf_node_last;
  DF_PCE_INFO *pce;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      Set_dispose (pce->nd_safe);
      Set_dispose (pce->xd_safe);
      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);

      pce->nd_safe = Set_copy (pf_node_first->info->pce_info->nd_safe);
      pce->xd_safe = Set_copy (pf_node_last->info->pce_info->xd_safe);

#ifdef DEBUG_PRE_BB_DS_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tND_SAFE", pf_bb->info->pce_info->nd_safe);
      Set_print (stderr, "\tXD_SAFE", pf_bb->info->pce_info->xd_safe);
#endif
    }
  }
}
#endif


/*! \brief Computes up-safety, or "availability", of expressions for PRE. */
static void
D_PRE_node_up_safety (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  DF_PCE_INFO *pce, *pce_pred;
  Set nu_safe, xu_safe, tmp, expression_U;
  int change, size;

  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);

  /* set nu_safe, xu_safe to initial values */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->nu_safe);
    Set_dispose (pce->xu_safe);
    pce->nu_safe = Set_copy (expression_U);
    pce->xu_safe = Set_copy (pce->trans);
    pf_node->info->cnt = size;
  }

  /* calculate up-safe sets */
  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	nu_safe = pce->nu_safe;

	/* 
	 * nu_safe[S] = intersection(x_comp[] + xu_safe[]
	 *                           of all predecessors of S)
	 */
	pce_pred = 0;
	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	{
	  pce_pred = pred_pf_node->info->pce_info;
	  tmp = Set_union (pce_pred->x_comp, pce_pred->xu_safe);
	  nu_safe = Set_intersect_acc (nu_safe, tmp);
	  RD_SET_CLEAR (tmp);
	}

	/* if no predecessors were found, set out to empty set */
	if (pce_pred == 0)
	  RD_SET_CLEAR (nu_safe);

	cnt = Set_size (nu_safe);
	pce->nu_safe = nu_safe;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    /*
	     * xu_safe[S] = trans[S] ^ (n_comp[S] + nu_safe[S])
	     */
	    xu_safe = Set_union (pce->n_comp, nu_safe);
	    xu_safe = Set_intersect_acc (xu_safe, pce->trans);

	    /* A change has occured if xu_safe has shrunk (monotonic) */
	    if (!Set_subtract_empty (pce->xu_safe, xu_safe))
	      change++;

	    RD_SET_CLEAR (pce->xu_safe);
	    pce->xu_safe = xu_safe;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PRE_NODE_US_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tNU_SAFE", pf_node->info->pce_info->nu_safe);
    Set_print (stderr, "\tXU_SAFE", pf_node->info->pce_info->xu_safe);
  }
#endif
}


#if 0
/* NOTE: Not currently called; for debugging purposes. */
static void
D_PRE_bb_up_safety (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node_first, *pf_node_last;
  DF_PCE_INFO *pce;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->nu_safe);
      RD_SET_CLEAR (pce->xu_safe);
      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);

      pce->nu_safe = Set_copy (pf_node_first->info->pce_info->nu_safe);
      pce->xu_safe = Set_copy (pf_node_last->info->pce_info->xu_safe);

#ifdef DEBUG_PRE_BB_US_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tNU_SAFE", pce->nu_safe);
      Set_print (stderr, "\tXU_SAFE", pce->xu_safe);
#endif
    }
  }
}
#endif


/*! \brief Computes the "earliest" locations for expressions in Knoop PRE.
 *
 *  \note These are the computation points where maximal reuse can occur, but
 *  at the cost of register pressure.  The latest positions remedies the
 *  register pressure problem.  Note that no dataflow is run for this
 *  computation.
 */
static void
D_PRE_node_earliest (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  Set n_movable, tmp, expression_U;
  DF_PCE_INFO *pce;
  int pred;

  expression_U = pred_flow->expression_U;

  /* calculate earliest sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    n_movable = Set_copy (expression_U);
    pce = pf_node->info->pce_info;
    pred = 0;

    PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
    {
      tmp = Set_union (pred_pf_node->info->pce_info->xu_safe,
		       pred_pf_node->info->pce_info->xd_safe);
      n_movable = Set_intersect_acc (n_movable, tmp);
      RD_SET_CLEAR (tmp);
      pred = 1;
    }

    RD_SET_CLEAR (pce->n_earliest);
    if (!pred)
      pce->n_earliest = Set_copy (pce->nd_safe);
    else
      pce->n_earliest = Set_subtract (pce->nd_safe, n_movable);
    RD_SET_CLEAR (n_movable);

    /* x_earliest[] = xd_safe[] - trans[] */
    RD_SET_CLEAR (pce->x_earliest);
    pce->x_earliest = Set_subtract (pce->xd_safe, pce->trans);

#ifdef DEBUG_PRE_NODE_EARL_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_EARLIEST", pce->n_earliest);
    Set_print (stderr, "\tX_EARLIEST", pce->x_earliest);
#endif
  }
}


#if 0
/* NOTE: Not currently called; for debugging purposes. */
static void
D_PRE_bb_earliest (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node_first, *pred_pf_node;
  DF_PCE_INFO *pce;
  Set n_earliest, tmp;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_earliest);
      RD_SET_CLEAR (pce->x_earliest);
      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      n_earliest = Set_copy (pce->nd_safe);

      PF_FOREACH_NODE (pred_pf_node, pf_node_first->pred)
      {
	tmp = Set_union (pred_pf_node->info->pce_info->xu_safe,
			 pred_pf_node->info->pce_info->xd_safe);
	n_earliest = Set_subtract_acc (n_earliest, tmp);
	RD_SET_CLEAR (tmp);
      }
      pce->n_earliest = n_earliest;
      pce->x_earliest = Set_subtract (pce->xd_safe, pce->trans);

#ifdef DEBUG_PRE_BB_EARL_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tN_EARLIEST", pce->n_earliest);
      Set_print (stderr, "\tX_EARLIEST", pce->x_earliest);
#endif
    }
  }
}
#endif


/*! \brief Computes the delay sets, which is essentially taking the earliest
 *  positions of computations and pushing them as late as possible in
 *  terms of program execution.
 */
static void
D_PRE_node_delay (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  DF_PCE_INFO *pce, *pred_pce;
  Set n_delayed, x_delayed, tmp, tmp2 = NULL, expression_U;
  int change, size;

  /* set n_delayed, x_delayed initially to all expressions */
  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->n_delayed);
    Set_dispose (pce->x_delayed);
    pce->n_delayed = Set_copy (expression_U);
    pce->x_delayed = Set_subtract_union (expression_U, pce->n_comp,
					 pce->x_earliest);
    pf_node->info->cnt = size;
  }

  /* calculate delay sets */
  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;

	n_delayed = pce->n_delayed;

	/* 
	 * n_delayed[S] = n_earliest + intersection(-x_comp[] ^ x_delayed[]
	 *                               of all successors of S)
	 *              = n_earliest + intersection(x_delayed[] - x_comp[]
	 *                               of all successors of S)
	 */
	pred_pce = 0;
	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	{
	  pred_pce = pred_pf_node->info->pce_info;
	  tmp = Set_subtract (pred_pce->x_delayed, pred_pce->x_comp);
	  n_delayed = Set_intersect_acc (n_delayed, tmp);
	  RD_SET_CLEAR (tmp);
	  RD_SET_CLEAR (tmp2);
	}
	/* if no predecessors were found, set out to empty set */
	if (pred_pce == 0)
	  RD_SET_CLEAR (n_delayed);

	n_delayed = Set_union_acc (n_delayed, pce->n_earliest);

	cnt = Set_size (n_delayed);
	pce->n_delayed = n_delayed;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    /*
	     * x_delayed[S] = x_earliest[S] + n_delayed[S] - n_comp[S]
	     */
	    x_delayed =
	      Set_subtract_union (pce->n_delayed, pce->n_comp,
				  pce->x_earliest);

	    /* A change has occured if x_delayed has shrunk (monotonic) */
	    if (!Set_subtract_empty (pce->x_delayed, x_delayed))
	      change++;

	    RD_SET_CLEAR (pce->x_delayed);
	    pce->x_delayed = x_delayed;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PRE_NODE_DELAY_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_DELAYED", pf_node->info->pce_info->n_delayed);
    Set_print (stderr, "\tX_DELAYED", pf_node->info->pce_info->x_delayed);
  }
#endif
}


/*! \brief  Computes the latest positions of computations for nodes.
 *
 *  \note No actual dataflow is run for this stage, it's based on the
 *  the delay sets.
 */
static void
D_PRE_node_late (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *succ_pf_node;
  DF_PCE_INFO *pce;
  Set x_latest, tmp, tmp2 = NULL, expression_U;

  expression_U = pred_flow->expression_U;

  /* Calculate latest sets.                                    */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->n_latest);
    pce->n_latest = Set_intersect (pce->n_comp, pce->n_delayed);

    tmp = Set_copy (expression_U);
    PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
      tmp = Set_intersect_acc (tmp, succ_pf_node->info->pce_info->n_delayed);

    x_latest = Set_subtract_union (expression_U, tmp, pce->x_comp);
    x_latest = Set_intersect_acc (x_latest, pce->x_delayed);
    RD_SET_CLEAR (tmp);
    RD_SET_CLEAR (tmp2);

    Set_dispose (pce->x_latest);
    pce->x_latest = x_latest;

#ifdef DEBUG_PRE_NODE_LATE_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_LATEST", pce->n_latest);
    Set_print (stderr, "\tX_LATEST", pce->x_latest);
#endif
  }
}


/*! \brief  Computes the latest positions of computations for BBs. */
static void
D_PRE_bb_late (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *pf_node_first, *pf_node_last, *pred_pf_node;
  DF_PCE_INFO *pce;
  Set in, out, expression_U;
  int change, size, single_first, single_last;

  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_latest);
      RD_SET_CLEAR (pce->x_latest);
      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);
      single_first = (List_size (pf_bb->pf_nodes_last) == 1);
      single_last = (List_size (pf_bb->pf_nodes_last) == 1);

      if (single_first && single_last && pf_node_first == pf_node_last)
	{
	  pce->n_latest = Set_copy (pf_node_first->info->pce_info->n_latest);
	  pce->x_latest = Set_copy (pf_node_first->info->pce_info->x_latest);
	}
      else
	{
	  /* n_latest we can compute from the bb sets */
	  pce->n_latest =
	    Set_intersect (pf_node_first->info->pce_info->n_delayed,
			   pce->n_comp);

	  /* x_latest computation: downward propagation */
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    RD_SET_CLEAR (pf_node->info->in);
	    RD_SET_CLEAR (pf_node->info->out);
	    pf_node->info->out = Set_copy (pf_node->info->pce_info->x_comp);
	    pf_node->info->in = Set_copy (expression_U);
	    pf_node->info->cnt = size;
	  }
	  pf_node_first->info->out =
	    Set_copy (pf_node_first->info->pce_info->x_latest);

	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt;
		if (single_first)
		  {
		    if (pf_node == pf_node_first)
		      continue;
		  }
		else if (Node_In_List (pf_node, pf_bb->pf_nodes_entry))
		  continue;
		in = pf_node->info->in;
		PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
		  in = Set_intersect_acc (in, pred_pf_node->info->out);

		cnt = Set_size (in);
		pf_node->info->in = in;

		if (cnt < pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    out = Set_intersect (pf_node->info->pce_info->trans, in);
		    out =
		      Set_union_acc (out, pf_node->info->pce_info->x_latest);
		    if (!Set_subtract_empty (out, pf_node->info->out))
		      change++;
		    RD_SET_CLEAR (pf_node->info->out);
		    pf_node->info->out = out;
		  }
	      }
	    }
	  while (change);
	  pce->x_latest = Set_copy (expression_U);
	  if (single_last)
	    pce->x_latest = Set_copy (pf_node_last->info->out);
	  else
	    {
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
		pce->x_latest = Set_intersect_acc (pce->x_latest,
						   pf_node->info->out);
	    }

	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
#ifdef DEBUG_PRE_BB_LATE_SETS
	    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
		     pf_node->pf_cb->cb->id);
	    Set_print (stderr, "\t\tPROPAGATION LATE IN SET",
		       pf_node->info->in);
	    Set_print (stderr, "\t\tPROPAGATION LATE OUT SETS",
		       pf_node->info->out);
#endif
	    RD_SET_CLEAR (pf_node->info->in);
	    RD_SET_CLEAR (pf_node->info->out);
	  }
	}
#ifdef DEBUG_PRE_BB_LATE_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tN_LATEST", pce->n_latest);
      Set_print (stderr, "\tX_LATEST", pce->x_latest);
#endif
    }
  }
}


/*! \brief Computes the isolated sets for nodes in PRE: this informs the
 * algorithm whether a computation is redundant and should be replaced. */
static void
D_PRE_node_isolated (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *succ_pf_node;
  DF_PCE_INFO *pce, *succ_pce;
  Set n_isolated, x_isolated, tmp, expression_U;
  int change, size;

  /* set n_isolated, x_isolated initially to all expressions */
  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->n_isolated);
    Set_dispose (pce->x_isolated);
    pce->n_isolated = Set_copy (expression_U);
    pce->x_isolated = Set_copy (expression_U);
    pf_node->info->cnt = size;
  }

  /* calculate isolated sets */
  do
    {
      change = 0;
      PF_FORHCAE_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	x_isolated = pce->x_isolated;

	/*
	 * x_isolated[S] = intersection( n_earliest[] + 
	 *                (n_isolated[] - n_comp[]) of all successors of [S])
	 */
	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	{
	  succ_pce = succ_pf_node->info->pce_info;
	  tmp =
	    Set_subtract_union (succ_pce->n_isolated, succ_pce->n_comp,
				succ_pce->n_earliest);
	  x_isolated = Set_intersect_acc (x_isolated, tmp);
	  RD_SET_CLEAR (tmp);
	}

	cnt = Set_size (x_isolated);

	pce->x_isolated = x_isolated;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    /*
	     * n_isolated[S] = x_earliest[S] + x_isolated[S]
	     */
	    n_isolated = Set_union (pce->x_earliest, pce->x_isolated);

	    /* A change has occured if <n_isolated> contains fewer */
	    /* isolated expressions than <df_node->n_isolated>.    */
	    if (!Set_subtract_empty (pce->n_isolated, n_isolated))
	      change++;

	    RD_SET_CLEAR (pce->n_isolated);
	    pce->n_isolated = n_isolated;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PRE_NODE_ISOL_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_ISOLATED", pf_node->info->pce_info->n_isolated);
    Set_print (stderr, "\tX_ISOLATED", pf_node->info->pce_info->x_isolated);
  }
#endif
}


/*! \brief Computes the isolated sets for BBs in PRE. */
static void
D_PRE_bb_isolated (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *pf_node_first, *pf_node_last, *succ_pf_node;
  DF_PCE_INFO *pce;
  Set in, out, comp, tmp, expression_U;
  int change, size, single_first, single_last;

  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);

  /* an expression is isolated in the cb if it is n_isolated for the
   * entry node(s) or x_isolated for all nodes
   */
  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_isolated);
      RD_SET_CLEAR (pce->x_isolated);
      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);
      single_first = (List_size (pf_bb->pf_nodes_entry) == 1);
      single_last = (List_size (pf_bb->pf_nodes_last) == 1);

      if (single_last)
	pce->x_isolated = Set_copy (pf_node_last->info->pce_info->x_isolated);
      else
	{
	  pce->x_isolated = Set_copy (expression_U);
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
	    pce->x_isolated =
	    Set_intersect_acc (pce->x_isolated,
			       pf_node->info->pce_info->x_isolated);
	}

      if (single_first && single_last && pf_node_first == pf_node_last)
	{
	  pce->n_isolated =
	    Set_copy (pf_node_first->info->pce_info->n_isolated);
	}
      else
	{
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    RD_SET_CLEAR (pf_node->info->in);
	    RD_SET_CLEAR (pf_node->info->out);

	    pf_node->info->in = Set_copy (expression_U);
	    pf_node->info->out = Set_copy (expression_U);
	    pf_node->info->cnt = size;
	  }

	  /* Correct for last nodes */
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
	    {
	      RD_SET_CLEAR (pf_node->info->in);
	      pf_node->info->in =
		Set_copy (pf_node->info->pce_info->n_isolated);
	    }

	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt = 0;

		if (single_last)
		  {
		    if (pf_node == pf_node_last)
		      continue;
		  }
		else if (Node_In_List (pf_node, pf_bb->pf_nodes_last))
		  continue;

		out = pf_node->info->out;
		PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
		{
		  comp = Set_subtract
		    (succ_pf_node->info->pce_info->n_comp,
		     pf_bb->info->pce_info->n_comp);
		  tmp =
		    Set_subtract_union
		    (succ_pf_node->info->in, comp,
		     succ_pf_node->info->pce_info->n_earliest);
		  out = Set_intersect_acc (out, tmp);
		  RD_SET_CLEAR (comp);
		  RD_SET_CLEAR (tmp);
		}
		cnt = Set_size (out);
		pf_node->info->out = out;

		if (cnt < pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    in = Set_union (pf_node->info->pce_info->x_earliest, out);
		    if (!Set_subtract_empty (pf_node->info->in, in))
		      change++;
		    RD_SET_CLEAR (pf_node->info->in);
		    pf_node->info->in = in;
		  }
	      }
	    }
	  while (change);

	  pce->n_isolated = Set_copy (pf_node_first->info->in);
	}

#ifdef DEBUG_PRE_BB_ISOL_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tN_ISOLATED", pce->n_isolated);
      Set_print (stderr, "\tX_ISOLATED", pce->x_isolated);
#endif
    }
  }
}


#if 0
/* Calculate optimal and redundant sets */
/* NOTE: Currently not called; for debugging purposes. */
static void
D_PRE_node_final_sets (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node;
  DF_PCE_INFO *pce;
  Set tmp;

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->n_insert);
    Set_dispose (pce->x_insert);
    pce->n_insert = Set_subtract (pce->n_latest, pce->n_isolated);
    pce->x_insert = Set_subtract (pce->x_latest, pce->x_isolated);

    RD_SET_CLEAR (pce->n_replace);
    RD_SET_CLEAR (pce->x_replace);
    tmp = Set_intersect (pce->n_latest, pce->n_isolated);
    pce->n_replace = Set_subtract (pce->n_comp, tmp);
    RD_SET_CLEAR (tmp);
    tmp = Set_intersect (pce->x_latest, pce->x_isolated);
    pce->x_replace = Set_subtract (pce->x_comp, tmp);
    RD_SET_CLEAR (tmp);

    /* Clean up sets: n_comp, x_comp,
     *                n_latest, x_latest, n_isolated, x_isolated
     */
    RD_SET_CLEAR (pce->n_latest);
    RD_SET_CLEAR (pce->x_latest);
    RD_SET_CLEAR (pce->n_isolated);
    RD_SET_CLEAR (pce->x_isolated);

#ifdef DEBUG_PRE_NODE_FINAL_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_INSERT", pce->n_insert);
    Set_print (stderr, "\tX_INSERT", pce->x_insert);
    Set_print (stderr, "\tN_REPLACE", pce->n_replace);
    Set_print (stderr, "\tX_REPLACE", pce->x_replace);
#endif
  }
}
#endif


/*! \brief Computes the final insertion and deletion points for PRE. */
static void
D_PRE_bb_final_sets (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  DF_PCE_INFO *pce;
  Set tmp;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;

      Set_dispose (pce->n_insert);
      Set_dispose (pce->x_insert);
      pce->n_insert = Set_subtract (pce->n_latest, pce->n_isolated);
      pce->x_insert = Set_subtract (pce->x_latest, pce->x_isolated);

      RD_SET_CLEAR (pce->n_replace);
      RD_SET_CLEAR (pce->x_replace);
      tmp = Set_intersect (pce->n_latest, pce->n_isolated);
      pce->n_replace = Set_subtract (pce->n_comp, tmp);
      RD_SET_CLEAR (tmp);
      tmp = Set_intersect (pce->x_latest, pce->x_isolated);
      pce->x_replace = Set_subtract (pce->x_comp, tmp);
      /* Following line should ALWAYS be safe with our setup. */
      pce->x_replace = Set_intersect_acc (pce->x_replace, pce->x_insert);
      RD_SET_CLEAR (tmp);


#ifdef DEBUG_PRE_BB_FINAL_SETS
      fprintf (stderr, "BB starting at op %d in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tN_INSERT", pce->n_insert);
      Set_print (stderr, "\tX_INSERT", pce->x_insert);
      Set_print (stderr, "\tN_REPLACE", pce->n_replace);
      Set_print (stderr, "\tX_REPLACE", pce->x_replace);
#endif
    }
  }
}


/*! \brief Performs analysis for the Knoop, non-speculative PRE algorithm. */
static void
D_PRE_analysis (PRED_FLOW * pred_flow, int mode)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node;
  DF_PCE_INFO *pce;
  int max_motions = 0;

#ifdef DEBUG_PRE_ANALYSIS
  fprintf (stderr, "Starting PRE analysis on function %s.\n", L_fn->name);
#endif
#ifdef DISPLAY_BB_INFO
  D_print_all_bb_info (pred_flow);
#endif

  if (!D_alloc_df_pce_info)
    D_alloc_df_pce_info =
      L_create_alloc_pool ("DF_PCE_IFO", sizeof (DF_PCE_INFO), 512);

  /* create operand-expression use sets */
  D_compute_expressions_and_operand_use_sets (pred_flow, mode);

  /* calculate transparent and local anticipability sets */
  D_PRE_node_local_info (pred_flow, mode, &max_motions);

  /* Modifies sets to deal with BBs containing predication. */
  D_PRE_bb_local_info (pred_flow);

  /* down-safe determines the safe blocks to move computations of
   * expressions upward
   */
  D_PRE_node_down_safety (pred_flow);

  /* up-safe is essentially classical available expression analysis
   */
  D_PRE_node_up_safety (pred_flow);

  /* earliest sets determine the earliest expression computations
   * can be moved
   */
  D_PRE_node_earliest (pred_flow);

  /* remove unnecessary sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->nd_safe);
    RD_SET_CLEAR (pce->xd_safe);
    RD_SET_CLEAR (pce->nu_safe);
    RD_SET_CLEAR (pce->xu_safe);
  }

  /* late is the furthest forward we can push computations of expressions
   * while still maintaining computational optimality 
   */
  D_PRE_node_delay (pred_flow);
  D_PRE_node_late (pred_flow);
  D_PRE_bb_late (pred_flow);

  /* Delete n_delayed, x_delayed */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->trans);
    RD_SET_CLEAR (pce->n_delayed);
    RD_SET_CLEAR (pce->x_delayed);
  }

  /* isolated determines the places where a computation of the expression is
   * used only with local uses, so there is no point in converting it
   */
  D_PRE_node_isolated (pred_flow);
  D_PRE_bb_isolated (pred_flow);

  /* Clean up node sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->n_comp);
    RD_SET_CLEAR (pce->x_comp);
    RD_SET_CLEAR (pce->n_earliest);
    RD_SET_CLEAR (pce->x_earliest);
    RD_SET_CLEAR (pce->n_latest);
    RD_SET_CLEAR (pce->x_latest);
    RD_SET_CLEAR (pce->n_isolated);
    RD_SET_CLEAR (pce->x_isolated);
  }

  /* Calculate optimal and redundant sets */
  D_PRE_bb_final_sets (pred_flow);

  /* Clean up bb sets */
  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_comp);
      RD_SET_CLEAR (pce->x_comp);
      RD_SET_CLEAR (pce->trans);
      RD_SET_CLEAR (pce->n_latest);
      RD_SET_CLEAR (pce->x_latest);
      RD_SET_CLEAR (pce->n_isolated);
      RD_SET_CLEAR (pce->x_isolated);
    }
  }
}


/*
 * PARTIAL REDUNDANCY ELIMINATION REACHING DEF ANALYSIS
 * ----------------------------------------------------------------------
 * This is a simple reaching definition analysis.
 * It is used to "fix-up" sync and check info after PRE by matching ops to be
 * removed with the memory ops that make it redundant.
 * It is also used for mem expression copy propagation.
 */

static void
D_PCE_compute_mem_operand_use_sets (PRED_FLOW * pred_flow, int mode)
{
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;
  Set RD_operand;
  L_Oper *oper;
  L_Attr * attr;
  int reg, mem_flag = 0, mem_loc = 0;

  RD_SET_CLEAR (pred_flow->mem_U);
  RD_SET_CLEAR (pred_flow->store_U);
  RD_SET_CLEAR (pred_flow->mem_stack_U);

  if (pred_flow->hash_RD_operand_use)
    HashTable_reset_func (pred_flow->hash_RD_operand_use,
			  (void (*)(void *)) Set_dispose);
  else
    pred_flow->hash_RD_operand_use = HashTable_create (2048);

  if (mode & PCE_MEM)
    mem_flag = 1;
  if (mode & MEM_REACHING_LOCATIONS)
    mem_loc = 1;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_OPER (pf_oper, pf_cb->pf_opers)
    {
      oper = pf_oper->oper;

      PF_FOREACH_OPERAND (pf_operand, pf_oper->src)
      {
	if (L_store_opcode (oper))
	  {
	    /* If doing stores, don't care about stored operand. */
	    L_Operand *operand = pf_operand->operand;
	    if (L_same_operand (operand, oper->src[2]))
	      continue;
	  }
	reg = pf_operand->reg;
	RD_operand = RD_FIND_OPD_USE (pred_flow, reg);
	RD_operand = Set_add (RD_operand, oper->id);
	RD_UPDATE_OPD_USE (pred_flow, reg, RD_operand);
      }

      if (mem_flag)
	{
	  if (L_load_opcode (oper))
	    {
	      pred_flow->mem_U = Set_add (pred_flow->mem_U, oper->id);
	      if (L_stack_reference (oper))
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (attr && attr->field[0]->value.mac == L_MAC_IP)
		    pred_flow->mem_stack_U =
		      Set_add (pred_flow->mem_stack_U, oper->id);
		}
	      else if (L_is_macro (oper->src[0]) &&
		       oper->src[0]->value.mac == L_MAC_IP)
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (!attr)
		    {
		      attr = L_new_attr (STACK_ATTR_NAME, 2);
		      attr->field[0] = L_copy_operand (oper->src[0]);
		      attr->field[1] = L_copy_operand (oper->src[1]);
		      oper->attr = L_concat_attr (oper->attr, attr);
		    }
		  oper->flags = L_SET_BIT_FLAG (oper->flags,
						L_OPER_STACK_REFERENCE);
		  pred_flow->mem_stack_U =
		    Set_add (pred_flow->mem_stack_U, oper->id);
		}
	    }
	  else if (L_store_opcode (oper))
	    {
	      pred_flow->mem_U = Set_add (pred_flow->mem_U, oper->id);
	      pred_flow->store_U = Set_add (pred_flow->store_U, oper->id);
	      if (L_stack_reference (oper))
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (attr && attr->field[0]->value.mac == L_MAC_OP)
		    pred_flow->mem_stack_U =
		      Set_add (pred_flow->mem_stack_U, oper->id);
		}
	      else if (L_is_macro (oper->src[0]) &&
		       oper->src[0]->value.mac == L_MAC_OP)
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (!attr)
		    {
		      attr = L_new_attr (STACK_ATTR_NAME, 2);
		      attr->field[0] = L_copy_operand (oper->src[0]);
		      attr->field[1] = L_copy_operand (oper->src[1]);
		      oper->attr = L_concat_attr (oper->attr, attr);
		    }
		  oper->flags = L_SET_BIT_FLAG (oper->flags,
						L_OPER_STACK_REFERENCE);
		  pred_flow->mem_stack_U =
		    Set_add (pred_flow->mem_stack_U, oper->id);
		}
	    }
	}
    }
  }
}


static void
D_PCE_node_reaching_local_info (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set gen, kill, tmp;
  L_Oper *oper, *dep_oper;
  int reg, i, mem_flag = 0, mem_conservative_flag = 0;

  if (mode & PCE_MEM)
    mem_flag = 1;
  if ((mode & PCE_MEM_CONSERVATIVE) ||
      !(L_func_acc_specs || (L_func_contains_dep_pragmas && L_use_sync_arcs)))
    {
#ifdef DEBUG_MEM_CONFLICT
      fprintf (stderr, "PCE Reach: Using conservative memory alias "
	       "assumptions in function %s.\n", pred_flow->fn->name);
#endif
      mem_conservative_flag = 1;
    }
#ifdef DEBUG_MEM_CONFLICT
  else if (L_func_acc_specs)
    fprintf (stderr, "PCE Reach: Using AccSpecs for memory alias info in "
	     "function %s.\n", pred_flow->fn->name);
  else if (L_func_contains_dep_pragmas && L_use_sync_arcs)
    fprintf (stderr, "PCE_Reach: Using sync arcs for memory alias info in "
	     "function %s.\n", pred_flow->fn->name);
#endif

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    gen = NULL;
    kill = NULL;

    PF_FOREACH_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;

      if (L_is_control_oper (oper))
	continue;
      if ((oper->opc == Lop_NO_OP))
	continue;
      if (L_sync_opcode (oper))
	{
	  RD_SET_CLEAR (gen);
	  Set_dispose (kill);
	  kill = Set_copy (pred_flow->oper_U);
	  continue;
	}

      /* adjust sets for dependent mem opers */
      if (mem_flag && (L_load_opcode (oper) || L_store_opcode (oper) ||
		       L_subroutine_call_opcode (oper)))
	{
	  if (mem_conservative_flag)
	    {
	      if (L_load_opcode (oper))
		{
		  gen = Set_subtract_acc (gen, pred_flow->store_U);
		  kill = Set_union_acc (kill, pred_flow->store_U);
		}
	      else if (L_store_opcode (oper) ||
		       !L_side_effect_free_sub_call (oper))
		{
		  int not_killed;

		  if (!Set_in (kill, oper->id))
		    not_killed = 1;
		  else
		    not_killed = 0;
		  gen = Set_subtract_acc (gen, pred_flow->mem_U);
		  kill = Set_union_acc (kill, pred_flow->mem_U);
		  if (not_killed)
		    kill = Set_delete (kill, oper->id);
		}
	    }
	  else if (L_func_acc_specs)
	    {
	      Set conflicts =
		L_mem_find_all_conflicting_expression_opers (oper);

	      gen = Set_subtract_acc (gen, conflicts);
	      kill = Set_union_acc (kill, conflicts);
	      RD_SET_CLEAR (conflicts);
	    }
	  else if (oper->sync_info)  /* sync arcs case */
	    {
	      for (i = 0; i < oper->sync_info->num_sync_out; i++)
		{
		  dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		  /* Can't kill stores of same expression. */
		  if (L_opers_same_expression (oper, dep_oper))
		    continue;
		  gen = Set_delete (gen, dep_oper->id);
		  kill = Set_add (kill, dep_oper->id);
		}
	      for (i = 0; i < oper->sync_info->num_sync_in; i++)
		{
		  dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		  if (L_opers_same_expression (oper, dep_oper))
		    continue;
		  /* Can't kill stores of same expression. */
		  gen = Set_delete (gen, dep_oper->id);
		  kill = Set_add (kill, dep_oper->id);
		}
	    }

	  /* Kill all stack references across non-side-effect-free jsrs */
	  if (L_subroutine_call_opcode (oper) &&
	      !L_side_effect_free_sub_call (oper))
	    {
	      gen = Set_subtract_acc (gen, pred_flow->mem_stack_U);
	      kill = Set_union_acc (kill, pred_flow->mem_stack_U);
	    }

	}
      /* Here, we add all ops that we want to find if reaching. */
      if (pf_inst->pred_true &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	gen = Set_add (gen, pf_inst->pf_oper->oper->id);

      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);
	gen = Set_subtract_acc (gen, tmp);
	kill = Set_union_acc (kill, tmp);
      }
    }

    Set_dispose (pf_node->info->use_gen);
    Set_dispose (pf_node->info->def_kill);
    pf_node->info->use_gen = gen;
    pf_node->info->def_kill = kill;
  }
}


static void
D_PCE_node_reaching_definition (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node, *pred_pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  L_Oper *oper, *dep_oper;
  Set in, out, tmp;
  int change, i, reg, mem_flag = 0, mem_conservative_flag = 0,
    mem_loc_flag = 0;

  if ((mode & PCE_MEM_CONSERVATIVE) ||
      !(L_func_acc_specs || (L_func_contains_dep_pragmas && L_use_sync_arcs)))
    mem_conservative_flag = 1;
  if (mode & MEM_REACHING_LOCATIONS)
    mem_loc_flag = 1;

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    RD_SET_CLEAR (pf_node->info->in);
    RD_SET_CLEAR (pf_node->info->out);
    pf_node->info->out = Set_copy (pf_node->info->use_gen);
    pf_node->info->cnt = 0;
  }

  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	in = pf_node->info->in;
	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	  in = Set_union_acc (in, pred_pf_node->info->out);

	cnt = Set_size (in);
	pf_node->info->in = in;

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    out = Set_subtract_union (in, pf_node->info->def_kill,
				      pf_node->info->use_gen);

	    if (!Set_subtract_empty (out, pf_node->info->out))
	      change++;

	    RD_SET_CLEAR (pf_node->info->out);
	    pf_node->info->out = out;
	  }
      }
    }
  while (change);

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    in = Set_copy (pf_node->info->in);
#ifdef DEBUG_PCE_REACHING_DEF
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "REACHING DEF", in);
#endif

    PF_FOREACH_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;
      if (L_is_control_oper (oper))
	continue;
      if ((oper->opc == Lop_NO_OP))
	continue;
      if (L_sync_opcode (oper) || L_subroutine_return_opcode (oper))
	{
	  RD_SET_CLEAR (in);
	  continue;
	}

      /* Mark instruction. */
      if (!mem_loc_flag || L_general_load_opcode (oper) ||
	  L_general_store_opcode (oper))
	{
	  RD_SET_CLEAR (pf_inst->pf_oper->info->r_in);
	  pf_inst->pf_oper->info->r_in = Set_copy (in);
	}

      if (mem_flag && (L_store_opcode (oper) ||
		       L_subroutine_call_opcode (oper)))
	{
	  if (mem_conservative_flag)
	    {
	      if (L_store_opcode (oper) ||
		  !L_side_effect_free_sub_call (oper))
		in = Set_subtract_acc (in, pred_flow->mem_U);
	    }
	  else if (L_func_acc_specs)
	    {
	      Set conflicts =
		L_mem_find_all_conflicting_expression_opers (oper);
	      in = Set_subtract_acc (in, conflicts);
	    }
	  /* adjust sets for dependent mem opers */
	  /* SER 20041216: Fix: don't kill same store expressions. */
	  else if (oper->sync_info)
	    {
	      for (i = 0; i < oper->sync_info->num_sync_out; i++)
		{
		  dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		  if (L_opers_same_expression (oper, dep_oper))
		    continue;
		  in = Set_delete (in, dep_oper->id);
		}
	      for (i = 0; i < oper->sync_info->num_sync_in; i++)
		{
		  dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		  if (L_opers_same_expression (oper, dep_oper))
		    continue;
		  in = Set_delete (in, dep_oper->id);
		}
	    }

	  /* kill stack references across non-side-effect-free jsrs */
	  if (L_subroutine_call_opcode (oper) &&
	      !L_side_effect_free_sub_call (oper))
	    in = Set_subtract_acc (in, pred_flow->mem_stack_U);

	}
      if (pf_inst->pred_true &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	in = Set_add (in, oper->id);

      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);
	in = Set_subtract_acc (in, tmp);
      }
    }

    RD_SET_CLEAR (in);
  }
}


static void
D_PCE_reaching_definition_analysis (PRED_FLOW * pred_flow, int mode)
{
  D_PCE_compute_mem_operand_use_sets (pred_flow, mode);
  D_PCE_node_reaching_local_info (pred_flow, mode);
  D_PCE_node_reaching_definition (pred_flow, mode);
}


/* 
 * MEMORY EXPRESSION REACHING DEF ANALYSIS
 * -----------------------------------------------------------
 * The following functions are for memory expression reaching 
 * definition analysis.
 */

static void
D_node_mem_expression_reach_local_info (PRED_FLOW * pred_flow,
					int mem_conservative_flag)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set gen, kill, tmp;
  L_Oper *oper, *dep_oper;
  L_Expression *expression;
  int i, reg, token, expr_index;

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    gen = NULL;
    kill = NULL;

    PF_FOREACH_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;

      if (L_is_control_oper (oper) || oper->opc == Lop_NO_OP)
	continue;
      if (L_sync_opcode (oper) || L_subroutine_return_opcode (oper))
	{
	  RD_SET_CLEAR (gen);
	  Set_dispose (kill);
	  kill = Set_copy (pred_flow->mem_U);
	  continue;
	}

      /* Adjust sets for dependent mem opers */
      if (L_store_opcode (oper) || L_subroutine_call_opcode (oper))
	{
	  if (mem_conservative_flag)
	    {
	      if (L_store_opcode (oper) ||
		  !L_side_effect_free_sub_call (oper))
		{
		  RD_SET_CLEAR (gen);
		  Set_dispose (kill);
		  kill = Set_copy (pred_flow->mem_U);
		}
	    }
	  else if (L_func_acc_specs)
	    {
	      if (L_store_opcode (oper))
		{
		  int token;
		  L_Expression * orig_expression;

		  token = L_generate_expression_token_from_oper (oper);
		  orig_expression = L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 0);
		  gen = Set_subtract_acc (gen, orig_expression->conflicts);
		  kill = Set_union_acc (kill, orig_expression->conflicts);
		}
	      else  /* subroutine call, check all expressions */
		{
		  Set conflicts = L_get_jsr_conflicting_expressions (oper);
		  gen = Set_subtract_acc (gen, conflicts);
		  kill = Set_union_acc (kill, conflicts);
		  RD_SET_CLEAR (conflicts);
		}
	    }
	  else if (oper->sync_info)  /* sync arcs case */
	    {
	      for (i = 0; i < oper->sync_info->num_sync_out; i++)
		{
		  dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  expr_index = expression->index;
		  gen = Set_delete (gen, expr_index);
		  kill = Set_add (kill, expr_index);
		}
	      for (i = 0; i < oper->sync_info->num_sync_in; i++)
		{
		  dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  expr_index = expression->index;
		  gen = Set_delete (gen, expr_index);
		  kill = Set_add (kill, expr_index);
		}
	    }
	}

      /* kill stack references across non-side-effect-free jsrs */
      if (L_subroutine_call_opcode (oper) &&
	  !L_side_effect_free_sub_call (oper))
	{
	  gen = Set_subtract_acc (gen, pred_flow->mem_stack_U);
	  kill = Set_union_acc (kill, pred_flow->mem_stack_U);
       
	}

      /* Here, we add all ops that we want to find if reaching. */
      if (pf_inst->pred_true &&
	  (L_load_opcode (oper) || L_store_opcode (oper)) &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	{
	  token = L_generate_expression_token_from_oper (oper);
	  expression = L_find_oper_expression_in_hash
	    (L_fn->expression_token_hash_tbl, token, oper, 1);
	  if (expression)
	    gen = Set_add (gen, expression->index);
	}

      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);
	gen = Set_subtract_acc (gen, tmp);
	kill = Set_union_acc (kill, tmp);
      }
    }

    RD_SET_CLEAR (pf_node->info->use_gen);
    RD_SET_CLEAR (pf_node->info->def_kill);
    pf_node->info->use_gen = gen;
    pf_node->info->def_kill = kill;

#ifdef DEBUG_PCE_MEM
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tGEN", pf_node->info->use_gen);
    Set_print (stderr, "\tKILL", pf_node->info->def_kill);
#endif
  }
}


static void
D_node_mem_expression_reaching_def (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  Set in, out, mem_U;
  int change;

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    RD_SET_CLEAR (pf_node->info->in);
    Set_dispose (pf_node->info->out);
    pf_node->info->out = Set_copy (pf_node->info->use_gen);
    pf_node->info->cnt = 0;
  }

  mem_U = pred_flow->mem_U;

  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt = 0;
	in = Set_copy (mem_U);
	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	{
	  in = Set_intersect_acc (in, pred_pf_node->info->out);
	  cnt = 1;
	}
	if (cnt == 0)
	  RD_SET_CLEAR (in);
	cnt = Set_size (in);
	RD_SET_CLEAR (pf_node->info->in);
	pf_node->info->in = in;

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    out = Set_subtract_union (in, pf_node->info->def_kill,
				      pf_node->info->use_gen);

	    if (!Set_subtract_empty (out, pf_node->info->out))
	      change++;

	    RD_SET_CLEAR (pf_node->info->out);
	    pf_node->info->out = out;
	  }
      }
    }
  while (change);
}


static void
D_mark_inst_mem_available_expressions (PRED_FLOW * pred_flow,
				       int mem_conservative_flag)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  L_Oper *oper, *dep_oper;
  L_Expression *expression;
  Set in, tmp;
  int i, reg, token;

  /* Mark memory instructions */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
#ifdef DEBUG_PCE_MEM
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tIN", pf_node->info->in);
    Set_print (stderr, "\tOUT", pf_node->info->out);
#endif
#ifdef DEBUG_MEM_REACHING_LOCATIONS
    fprintf (stderr, "NODE %d, CB %d\n", pf_node->id, pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tPCE_MEM_REACH", pf_node->info->in);
#endif
    in = Set_copy (pf_node->info->in);

    PF_FOREACH_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;
      if (L_is_control_oper (oper) || oper->opc == Lop_NO_OP)
	continue;
      if (L_sync_opcode (oper) || L_subroutine_return_opcode (oper))
	{
	  RD_SET_CLEAR (in);
	  continue;
	}

      /* To save space, mark only those operations which we will
       * reference later. */
      if (pf_inst->pred_true &&
	  (L_load_opcode (oper) || L_store_opcode (oper)))
	{
	  RD_SET_CLEAR (pf_inst->pf_oper->info->mem_a_in);
	  pf_inst->pf_oper->info->mem_a_in = Set_copy (in);
	}

      if (L_store_opcode (oper) || L_subroutine_call_opcode (oper))
	{
	  if (mem_conservative_flag)
	    {
	      if (L_store_opcode (oper) ||
		  !L_side_effect_free_sub_call (oper))
		RD_SET_CLEAR (in);
	    }
	  else if (L_func_acc_specs)
	    {
	      if (L_store_opcode (oper))
		{
		  int token;
		  L_Expression * orig_expression;

		  token = L_generate_expression_token_from_oper (oper);
		  orig_expression = L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 0);
		  in = Set_subtract_acc (in, orig_expression->conflicts);
		}
	      else  /* subroutine call, check all expressions */
		{
		  Set conflicts = L_get_jsr_conflicting_expressions (oper);
		  in = Set_subtract_acc (in, conflicts);
		  RD_SET_CLEAR (conflicts);
		}
	    }
	  /* adjust sets for dependent mem opers */
	  else if (oper->sync_info)
	    {
	      for (i = 0; i < oper->sync_info->num_sync_out; i++)
		{
		  dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  in = Set_delete (in, expression->index);
		}
	      for (i = 0; i < oper->sync_info->num_sync_in; i++)
		{
		  dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  in = Set_delete (in, expression->index);
		}
	    }

	  if (L_subroutine_call_opcode (oper) &&
	      !L_side_effect_free_sub_call (oper))
	    in = Set_subtract_acc (in, pred_flow->mem_stack_U);

	}

      if (pf_inst->pred_true &&
	  (L_load_opcode (oper) || L_store_opcode (oper)) &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	{
	  token = L_generate_expression_token_from_oper (oper);
	  expression = L_find_oper_expression_in_hash
	    (L_fn->expression_token_hash_tbl, token, oper, 1);
	  if (expression)
	    in = Set_add (in, expression->index);
	}

      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);
	in = Set_subtract_acc (in, tmp);
      }
    }

    RD_SET_CLEAR (in);
  }
}


static void
D_reaching_mem_expression_analysis (PRED_FLOW * pred_flow, int mode)
{
  int mem_flag;

  D_compute_expressions_and_operand_use_sets (pred_flow, mode);
  mem_flag = ((mode & PCE_MEM_CONSERVATIVE) ||
	      !(L_func_acc_specs ||
		(L_func_contains_dep_pragmas && L_use_sync_arcs))) ? 1 : 0;
  D_node_mem_expression_reach_local_info (pred_flow, mem_flag);
  D_node_mem_expression_reaching_def (pred_flow);
  D_mark_inst_mem_available_expressions (pred_flow, mem_flag);
}


/*
 * PARTIAL REDUNDANCY ELIMINATION MEM ANTICIPABLE ANALYSIS
 * ----------------------------------------------------------------------
 * This is a anticipable expression/location analysis for memory ops.
 */


static void
D_node_mem_expression_anticipable_local_info (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  Set gen, kill, tmp;
  L_Oper *oper, *dep_oper;
  L_Expression *expression;
  int i, reg, token, expr_index, mem_conservative_flag = 0;

  if ((mode & PCE_MEM_CONSERVATIVE) ||
      !(L_func_acc_specs || (L_func_contains_dep_pragmas && L_use_sync_arcs)))
    {
#ifdef DEBUG_MEM_CONFLICT
	  fprintf (stderr, "ANT Mem: Using conservative memory alias "
		   "assumptions in function %s.\n", pred_flow->fn->name);
#endif
      mem_conservative_flag = 1;
    }
#ifdef DEBUG_MEM_CONFLICT
      else if (L_func_acc_specs)
	fprintf (stderr, "ANT Mem: Using AccSpecs for memory alias info in "
		 "function %s.\n", pred_flow->fn->name);
      else if (L_func_contains_dep_pragmas && L_use_sync_arcs)
	fprintf (stderr, "ANT Mem: Using sync arcs for memory alias info in "
		 "function %s.\n", pred_flow->fn->name);
#endif

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    gen = NULL;
    kill = NULL;

    PF_FORHCAE_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;

      if (L_is_control_oper (oper) || oper->opc == Lop_NO_OP)
	continue;
      if (L_sync_opcode (oper))
	continue;
      if (L_subroutine_return_opcode (oper))
	{
	  RD_SET_CLEAR (gen);
	  if (mode & DEAD_LOCAL_MEM_VAR)
	    gen = Set_copy (pred_flow->local_var_U);
	}

      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);
	gen = Set_subtract_acc (gen, tmp);
	kill = Set_union_acc (kill, tmp);
      }

      if (L_load_opcode (oper) || L_store_opcode (oper) ||
	  L_subroutine_call_opcode (oper))
	{
	  if (mem_conservative_flag)
	    {
	      RD_SET_CLEAR (gen);
	      kill = Set_union_acc (kill, pred_flow->mem_U);
	    }
	  else if (L_func_acc_specs)
	    {
	      if (L_load_opcode(oper) || L_store_opcode (oper))
		{
		  int token;
		  L_Expression * orig_expression;

		  token = L_generate_expression_token_from_oper (oper);
		  orig_expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 0);
		  gen = Set_subtract_acc (gen, orig_expression->conflicts);
		  kill = Set_union_acc (kill, orig_expression->conflicts);
		}
	      else  /* subroutine call, check all expressions */
		{
		  Set conflicts = L_get_jsr_conflicting_expressions (oper);
		  gen = Set_subtract_acc (gen, conflicts);
		  kill = Set_union_acc (kill, conflicts);
		  RD_SET_CLEAR (conflicts);
		}
	    }
	  else if (oper->sync_info)
	    {
	      for (i = 0; i < oper->sync_info->num_sync_out; i++)
		{
		  dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  expr_index = expression->index;
		  gen = Set_delete (gen, expr_index);
		  kill = Set_add (kill, expr_index);
		}
	      for (i = 0; i < oper->sync_info->num_sync_in; i++)
		{
		  dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  expr_index = expression->index;
		  gen = Set_delete (gen, expr_index);
		  kill = Set_add (kill, expr_index);
		}
	    }

	  if (L_subroutine_call_opcode (oper) &&
	      !L_side_effect_free_sub_call (oper))
	    {
	      gen = Set_subtract_acc (gen, pred_flow->mem_stack_U);
	      kill = Set_union_acc (kill, pred_flow->mem_stack_U);
	    }

	}

      if (pf_inst->pred_true &&
	  (L_load_opcode (oper) || L_store_opcode (oper)) &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	{
	  token = L_generate_expression_token_from_oper (oper);
	  expression = L_find_oper_expression_in_hash
	    (L_fn->expression_token_hash_tbl, token, oper, 1);
	  if (expression)
	    gen = Set_add (gen, expression->index);
	}
    }

    RD_SET_CLEAR (pf_node->info->use_gen);
    RD_SET_CLEAR (pf_node->info->def_kill);
    pf_node->info->use_gen = gen;
    pf_node->info->def_kill = kill;

#ifdef DEBUG_PCE_MEM
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tGEN", pf_node->info->use_gen);
    Set_print (stderr, "\tKILL", pf_node->info->def_kill);
#endif
  }
}


static void
D_node_mem_expression_anticipable (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *succ_pf_node;
  Set in, out, mem_U;
  int change;

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    Set_dispose (pf_node->info->in);
    RD_SET_CLEAR (pf_node->info->out);
    pf_node->info->in = Set_copy (pf_node->info->use_gen);
    pf_node->info->cnt = 0;
  }

  mem_U = pred_flow->mem_U;

  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	out = Set_copy (mem_U);
	cnt = 0;
	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	{
	  cnt = 1;
	  out = Set_intersect_acc (out, succ_pf_node->info->in);
	}
	if (cnt == 0)
	  RD_SET_CLEAR (out);
	cnt = Set_size (out);
	RD_SET_CLEAR (pf_node->info->out);
	pf_node->info->out = out;

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    in = Set_subtract_union (out, pf_node->info->def_kill,
				     pf_node->info->use_gen);

	    if (!Set_subtract_empty (in, pf_node->info->in))
	      change++;

	    RD_SET_CLEAR (pf_node->info->in);
	    pf_node->info->in = in;
	  }
      }
    }
  while (change);
}


static void
D_mark_inst_mem_anticipable_expressions (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  L_Oper *oper, *dep_oper;
  L_Expression *expression;
  Set out, tmp;
  int i, reg, token, mem_conservative_flag = 0;

  if ((mode & PCE_MEM_CONSERVATIVE) ||
      !(L_func_acc_specs || (L_func_contains_dep_pragmas && L_use_sync_arcs)))
    mem_conservative_flag = 1;

  /* Mark memory instructions */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
#ifdef DEBUG_PCE_MEM
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tIN", pf_node->info->in);
    Set_print (stderr, "\tOUT", pf_node->info->out);
#endif
#ifdef DEBUG_MEM_ANT_EXPRESSIONS
    fprintf (stderr, "NODE %d, CB %d\n", pf_node->id, pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tPCE_MEM_ANT", pf_node->info->out);
#endif
    out = Set_copy (pf_node->info->out);

    PF_FORHCAE_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;
      if (L_is_control_oper (oper) || oper->opc == Lop_NO_OP)
	continue;
      if (L_sync_opcode (oper))
	continue;
      if (L_subroutine_return_opcode (oper))
	{
	  Set_dispose (out);
	  if (mode & DEAD_LOCAL_MEM_VAR)
	    out = Set_copy (pred_flow->local_var_U);
	}

      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_USE (pred_flow, reg);
	out = Set_subtract_acc (out, tmp);
      }

      /* To save space, mark only those operations which we will
       * reference later. */
      if (pf_inst->pred_true &&
	  (L_load_opcode (oper) || L_store_opcode (oper)))
	{
	  RD_SET_CLEAR (pf_inst->pf_oper->info->mem_a_out);
	  pf_inst->pf_oper->info->mem_a_out = Set_copy (out);
	}

      if (L_load_opcode (oper) || L_store_opcode (oper) ||
	  L_subroutine_call_opcode (oper))
	{
	  if (mem_conservative_flag)
	    RD_SET_CLEAR (out);
	  else if (L_func_acc_specs)
	    {
	      if (L_load_opcode (oper) || L_store_opcode (oper))
		{
		  int token;
		  L_Expression * orig_expression;

		  token = L_generate_expression_token_from_oper (oper);
		  orig_expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 0);
		  out = Set_subtract_acc (out, orig_expression->conflicts);
		}
	      else  /* subroutine call, check all expressions */
		{
		  Set conflicts = L_get_jsr_conflicting_expressions (oper);
		  out = Set_subtract_acc (out, conflicts);
		  RD_SET_CLEAR (conflicts);
		}
	    }
	  else if (oper->sync_info)
	    {
	      for (i = 0; i < oper->sync_info->num_sync_out; i++)
		{
		  dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  out = Set_delete (out, expression->index);
		}
	      for (i = 0; i < oper->sync_info->num_sync_in; i++)
		{
		  dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		  if (oper == dep_oper)
		    continue;
		  if (L_subroutine_call_opcode (dep_oper) ||
		      L_subroutine_return_opcode (dep_oper))
		    continue;
		  token = L_generate_expression_token_from_oper (dep_oper);
		  expression =
		    L_find_oper_expression_in_hash
		    (L_fn->expression_token_hash_tbl, token, dep_oper, 1);
		  if (!expression)
		    continue;
		  out = Set_delete (out, expression->index);
		}
	    }

	  if (L_subroutine_call_opcode (oper) &&
	      !L_side_effect_free_sub_call (oper))
	    out = Set_subtract_acc (out, pred_flow->mem_stack_U);

	}

      if (pf_inst->pred_true &&
	  (L_load_opcode (oper) || L_store_opcode (oper)) &&
	  !L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	{
	  token = L_generate_expression_token_from_oper (oper);
	  expression = L_find_oper_expression_in_hash
	    (L_fn->expression_token_hash_tbl, token, oper, 1);
	  if (expression)
	    out = Set_add (out, expression->index);
	}
    }

    RD_SET_CLEAR (out);
  }
}


static void
D_anticipable_mem_expression_analysis (PRED_FLOW * pred_flow, int mode)
{
  D_compute_expressions_and_operand_use_sets (pred_flow, mode);
  D_node_mem_expression_anticipable_local_info (pred_flow, mode);
  D_node_mem_expression_anticipable (pred_flow);
  D_mark_inst_mem_anticipable_expressions (pred_flow, mode);
}


/*
 * SPECULATIVE PARTIAL REDUNDANCY ELIMINATION ANALYSIS (SER)
 * ----------------------------------------------------------------------
 * Used for speculative PRE (non-minimum-cut).
 */

/* D_PRE_mark_speculative_motion
 * Marks nodes to be speculated upward.
 * Assumptions: Dynamic profiling information exists.
 * Description: Marks the expressions for each set which we allow to be
 *   speculated upward, even if it is not anticipable along all paths.
 *   This version allow speculation if branches are heavily biased.
 */
static void
D_PRE_mark_speculative_motion (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node, *succ_pf_node;
  PF_INST *pf_inst;
  DF_PCE_INFO *pce;
  Set propagate;
  L_Expression_Hash_Entry *entry;
  int i;
  double total_weight;

  L_compute_oper_weight (L_fn, 0, 1);

  /* For the time being, propagate all instructions at a heavily-weighted
   * node, except for store instructions. If there are not non-excepting
   * versions of excepting operations, then don't do those. */
  if (mode & PRE_ONLY_EXCEPTING)
    {
      propagate = Set_copy (pred_flow->expression_U);
      for (i = 1; i <= L_fn->n_expression; i++)
	{
	  entry =
	    L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl,
					      i);
	  if (L_is_pe_expression (entry->expression))
	    propagate = Set_delete (propagate, i);
	}
    }
  else
    propagate = Set_subtract (pred_flow->expression_U, pred_flow->store_U);


  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    total_weight = 0.0;
    /* First, obtain total weight of nodes. */
    PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
    {
      pf_inst = (PF_INST *) List_get_first (succ_pf_node->pf_insts);
      if (pf_inst)
	total_weight += pf_inst->pf_oper->oper->weight;
      else
	total_weight += succ_pf_node->pf_cb->cb->weight;
    }

    /* Next, mark nodes that satisfy weight requirement */
    PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
    {
      pce = succ_pf_node->info->pce_info;
      pf_inst = (PF_INST *) List_get_first (succ_pf_node->pf_insts);
      if (pf_inst)
	{
	  if (pf_inst->pf_oper->oper->weight > PRE_SPEC_RATIO * total_weight)
	    {
	      RD_SET_CLEAR (pce->speculate_up);
	      pce->speculate_up = Set_copy (propagate);
	    }
	}
      else
	{
	  if (succ_pf_node->pf_cb->cb->weight > PRE_SPEC_RATIO * total_weight)
	    {
	      RD_SET_CLEAR (pce->speculate_up);
	      pce->speculate_up = Set_copy (propagate);
	    }
	}
    }
  }
  Set_dispose (propagate);
}


/* Note that partial down-safety doesn't have the safety requirements
 * of full down-safety, so we don't need a "correction" phase for the 
 * speculative portions.
 * Assumptions: standard down-safety analysis has been performed already,
 *   to maintain monotonicity.
 */
static void
D_PRE_speculative_node_down_safety (PRED_FLOW * pred_flow, int mode)
{
  PF_NODE *pf_node, *succ_pf_node;
  DF_PCE_INFO *pce, *succ_pce;
  Set tmp_all, tmp_node, n_spec_ds, x_spec_ds, expression_U;
  int change, size;

  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);

  /* set all sets to initial values */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->n_spec_ds);
    RD_SET_CLEAR (pce->x_spec_ds);
    pf_node->info->cnt = 0;
  }

  /* calculate down-safe sets */
  do
    {
      change = 0;
      PF_FORHCAE_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;

	/* 
	 * x_spec_ds[S] = intersection(n_(spec)_ds[] of all successors of S)
	 *                + union(n_(spec)_ds[] of all 
	 *                    marked successors of S)
	 */
	succ_pce = 0;
	x_spec_ds = Set_copy (expression_U);
	tmp_all = NULL;
	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	{
	  succ_pce = succ_pf_node->info->pce_info;
	  tmp_node = Set_union (succ_pce->nd_safe, succ_pce->n_spec_ds);
	  x_spec_ds = Set_intersect_acc (x_spec_ds, tmp_node);
	  tmp_node = Set_intersect_acc (tmp_node, succ_pce->speculate_up);
	  tmp_all = Set_union_acc (tmp_all, tmp_node);
	  RD_SET_CLEAR (tmp_node);
	}

	/* if no successors were found, set x_spec_ds to empty set */
	if (succ_pce == 0)
	  RD_SET_CLEAR (x_spec_ds);
	else
	  {
	    x_spec_ds = Set_union_acc (x_spec_ds, tmp_all);
	    RD_SET_CLEAR (tmp_all);
	  }
	x_spec_ds = Set_subtract_acc (x_spec_ds, pce->xd_safe);

	cnt = Set_size (x_spec_ds);
	RD_SET_CLEAR (pce->x_spec_ds);
	pce->x_spec_ds = x_spec_ds;

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    /*
	     * n_spec_ds = (trans[S] ^ x_spec_ds[S]) - nd_safe[S]
	     */
	    n_spec_ds = Set_intersect (pce->trans, pce->x_spec_ds);
	    n_spec_ds = Set_subtract_acc (n_spec_ds, pce->nd_safe);

	    /* A change has occured if nd_safe has shrunk (monotonic) */
	    if (!Set_subtract_empty (n_spec_ds, pce->n_spec_ds))
	      change++;

	    Set_dispose (pce->n_spec_ds);
	    pce->n_spec_ds = n_spec_ds;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PRE_SPEC_NODE_DS_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    pce = pf_node->info->pce_info;
    Set_print (stderr, "\tN_SPEC_DS", pce->n_spec_ds);
    Set_print (stderr, "\tX_SPEC_DS", pce->x_spec_ds);
  }
#endif
}


static void
D_PRE_speculative_node_earliest (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *other_pf_node;
  Set n_movable, out, tmp, n_earliest, x_earliest, expression_U;
  DF_PCE_INFO *pce, *other_pce;
  int cnt;

  expression_U = pred_flow->expression_U;

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;

    out = Set_copy (expression_U);
    cnt = 0;
    PF_FOREACH_NODE (other_pf_node, pf_node->succ)
    {
      cnt = 1;
      other_pce = other_pf_node->info->pce_info;
      tmp = Set_union (other_pce->nd_safe, other_pce->n_spec_ds);
      out = Set_intersect_acc (out, tmp);
      RD_SET_CLEAR (tmp);
    }
    if (!cnt)
      RD_SET_CLEAR (out);
    RD_SET_CLEAR (pf_node->info->out);
    pf_node->info->out = out;
#ifdef DEBUG_PRE_SPEC_NODE_EARL_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tX_SAFE", out);
#endif
  }

  /* calculate earliest sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;

    n_movable = Set_copy (expression_U);
    PF_FOREACH_NODE (other_pf_node, pf_node->pred)
    {
      other_pce = other_pf_node->info->pce_info;
      tmp = Set_union (other_pce->xu_safe, other_pce->xd_safe);
      tmp = Set_union_acc (tmp, other_pf_node->info->out);
      n_movable = Set_intersect_acc (n_movable, tmp);
      RD_SET_CLEAR (tmp);
    }
    n_earliest = Set_union (pce->nd_safe, pce->n_spec_ds);
    n_earliest = Set_subtract_acc (n_earliest, n_movable);
    RD_SET_CLEAR (pce->n_earliest);
    pce->n_earliest = n_earliest;
    RD_SET_CLEAR (n_movable);

    x_earliest = Set_copy (expression_U);
    cnt = 0;
    PF_FOREACH_NODE (other_pf_node, pf_node->succ)
    {
      cnt = 1;
      other_pce = other_pf_node->info->pce_info;
      tmp = Set_union (other_pce->nd_safe, other_pce->n_spec_ds);
      x_earliest = Set_intersect_acc (x_earliest, tmp);
      RD_SET_CLEAR (tmp);
    }
    if (!cnt)
      RD_SET_CLEAR (x_earliest);
    x_earliest = Set_union_acc (x_earliest, pce->xd_safe);
    x_earliest = Set_subtract_acc (x_earliest, pce->trans);
    RD_SET_CLEAR (pce->x_earliest);
    pce->x_earliest = x_earliest;

#ifdef DEBUG_PRE_SPEC_NODE_EARL_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_EARLIEST", pce->n_earliest);
    Set_print (stderr, "\tX_EARLIEST", pce->x_earliest);
#endif
  }

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
    RD_SET_CLEAR (pf_node->info->out);
}



static void
D_PRE_speculative_node_delay (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  DF_PCE_INFO *pce;
  Set n_delayed, x_delayed, tmp, expression_U;
  int change, size;

  /* set n_delayed, x_delayed initially to all expressions */
  expression_U = pred_flow->expression_U;
  size = Set_size (expression_U);
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->n_delayed);
    RD_SET_CLEAR (pce->x_delayed);
    pce->n_delayed = Set_copy (expression_U);
    pce->x_delayed = Set_subtract_union (expression_U, pce->n_comp,
					 pce->x_earliest);
    pf_node->info->cnt = size;
  }

  /* calculate delay sets */
  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;

	n_delayed = pce->n_delayed;

	/* 
	 * n_delayed[S] = n_earliest + intersection(x_delayed[] - x_comp[]
	 *                               of all successors of S)
	 *                ^ (nd_safe[S] + n_spec_ds[S])
	 */
	cnt = 0;
	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	{
	  cnt = 1;
	  tmp = Set_subtract (pred_pf_node->info->pce_info->x_delayed,
			      pred_pf_node->info->pce_info->x_comp);
	  n_delayed = Set_intersect_acc (n_delayed, tmp);
	  RD_SET_CLEAR (tmp);
	}
	/* if no predecessors were found, set out to empty set */
	if (cnt == 0)
	  RD_SET_CLEAR (n_delayed);

	n_delayed = Set_union_acc (n_delayed, pce->n_earliest);
	tmp = Set_union (pce->nd_safe, pce->n_spec_ds);
	n_delayed = Set_intersect_acc (n_delayed, tmp);
	RD_SET_CLEAR (tmp);
	cnt = Set_size (n_delayed);
	pce->n_delayed = n_delayed;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    /*
	     * x_delayed[S] = (x_earliest[S] + n_delayed[S] - n_comp[S])
	     *                ^ (xd_safe[S] + x_spec_ds[S])
	     */
	    x_delayed =
	      Set_subtract_union (pce->n_delayed, pce->n_comp,
				  pce->x_earliest);
	    tmp = Set_union (pce->xd_safe, pce->x_spec_ds);
	    x_delayed = Set_intersect_acc (x_delayed, tmp);
	    RD_SET_CLEAR (tmp);

	    /* A change has occured if x_delayed has shrunk (monotonic) */
	    if (!Set_subtract_empty (pce->x_delayed, x_delayed))
	      change++;

	    RD_SET_CLEAR (pce->x_delayed);
	    pce->x_delayed = x_delayed;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PRE_SPEC_NODE_DELAY_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_DELAYED", pf_node->info->pce_info->n_delayed);
    Set_print (stderr, "\tX_DELAYED", pf_node->info->pce_info->x_delayed);
  }
#endif
}


static void
D_PRE_speculative_node_late (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *succ_pf_node;
  DF_PCE_INFO *pce, *succ_pce;
  Set x_latest, tmp;

  /* Calculate latest sets. */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->n_latest);
    pce->n_latest = Set_intersect (pce->n_comp, pce->n_delayed);

    x_latest = NULL;
    PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
    {
      succ_pce = succ_pf_node->info->pce_info;
      tmp = Set_union (succ_pce->n_spec_ds, succ_pce->nd_safe);
      tmp = Set_subtract_acc (tmp, succ_pce->n_delayed);
      x_latest = Set_union_acc (x_latest, tmp);
      RD_SET_CLEAR (tmp);
    }

    x_latest = Set_union_acc (x_latest, pce->x_comp);
    x_latest = Set_intersect_acc (x_latest, pce->x_delayed);
    RD_SET_CLEAR (pce->x_latest);
    pce->x_latest = x_latest;

#ifdef DEBUG_PRE_SPEC_NODE_LATE_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_LATEST", pce->n_latest);
    Set_print (stderr, "\tX_LATEST", pce->x_latest);
#endif
  }
}


static void
D_PRE_speculative_analysis (PRED_FLOW * pred_flow, int mode)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node;
  DF_PCE_INFO *pce;
  int max_motions = 0;

  fprintf (stderr, "SER: hasn't been tested in a long time, may break.  "
	   "The speculative min-cut analysis is the preferred version.");

#ifdef DEBUG_PRE_SPEC_ANALYSIS
  fprintf (stderr, "Starting speculative PRE analysis on function %s.\n",
	   L_fn->name);
#endif

  if (!D_alloc_df_pce_info)
    D_alloc_df_pce_info =
      L_create_alloc_pool ("DF_PCE_IFO", sizeof (DF_PCE_INFO), 512);

  D_compute_expressions_and_operand_use_sets (pred_flow, mode);

  D_PRE_node_local_info (pred_flow, mode, &max_motions);
  D_PRE_mark_speculative_motion (pred_flow, mode);

#ifdef DISPLAY_BB_INFO
  D_print_all_bb_info (pred_flow);
#endif

  /* SER 20050210: relocated due to potential bugs. */
  D_PRE_bb_local_info (pred_flow);

  D_PRE_node_down_safety (pred_flow);
  D_PRE_speculative_node_down_safety (pred_flow, mode);
  D_PRE_node_up_safety (pred_flow);

  D_PRE_bb_local_safety_info (pred_flow);

  D_PRE_speculative_node_earliest (pred_flow);

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->nu_safe);
    RD_SET_CLEAR (pce->xu_safe);
  }

  D_PRE_speculative_node_delay (pred_flow);
  D_PRE_speculative_node_late (pred_flow);
  D_PRE_bb_late (pred_flow);

  /* Delete n_delayed, x_delayed, spec sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->trans);
    RD_SET_CLEAR (pce->n_delayed);
    RD_SET_CLEAR (pce->x_delayed);
    RD_SET_CLEAR (pce->n_spec_ds);
    RD_SET_CLEAR (pce->x_spec_ds);
  }

  D_PRE_node_isolated (pred_flow);
  D_PRE_bb_isolated (pred_flow);

  /* clean up node sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->nd_safe);
    RD_SET_CLEAR (pce->xd_safe);
    RD_SET_CLEAR (pce->n_comp);
    RD_SET_CLEAR (pce->x_comp);
    RD_SET_CLEAR (pce->n_earliest);
    RD_SET_CLEAR (pce->x_earliest);
    RD_SET_CLEAR (pce->n_latest);
    RD_SET_CLEAR (pce->x_latest);
    RD_SET_CLEAR (pce->n_isolated);
    RD_SET_CLEAR (pce->x_isolated);
  }

  D_PRE_bb_final_sets (pred_flow);

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_comp);
      RD_SET_CLEAR (pce->x_comp);
      RD_SET_CLEAR (pce->trans);
      RD_SET_CLEAR (pce->nd_safe);
      RD_SET_CLEAR (pce->xd_safe);
      RD_SET_CLEAR (pce->n_latest);
      RD_SET_CLEAR (pce->x_latest);
      RD_SET_CLEAR (pce->n_isolated);
      RD_SET_CLEAR (pce->x_isolated);
    }
  }
}


/*
 * Speculative PRE using minimum cut (SER)
 * ----------------------------------------------------------------------
 * Refer to Cai and Que, CGO 2003. This is a similar algorithm, but
 * has some modifications, detailed in Ryoo's M.S. thesis (2004).
 */


/* SER: This function finds all expressions that could potentially benefit
   from SPRE.
   Assumption: Node & BB PRE sets computed: 
     Local, upsafe, downsafe, and speculative downsafe info..
*/
static void
D_PRE_setup_graph_cut (PRED_FLOW * pred_flow, Set * motion)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *pred_pf_node, *succ_pf_node;
  DF_PCE_INFO *pce;
  Set temp, fragile_U;
  int change;

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    Set_dispose (pce->n_spec_ds);
    Set_dispose (pce->x_spec_ds);
    Set_dispose (pce->n_spec_us);
    Set_dispose (pce->x_spec_us);
    pce->n_spec_ds = Set_copy (pce->nd_safe);
    pce->x_spec_ds = Set_copy (pce->xd_safe);
    pf_node->info->cnt = Set_size (pce->xd_safe);
  }

  /* Calculate partial down safety. */
  do
    {
      change = 0;
      PF_FORHCAE_NODE (pf_node, pred_flow->list_pf_node)
      {
	Set n_spec_ds, x_spec_ds;
	int cnt;
	pce = pf_node->info->pce_info;
	x_spec_ds = pce->x_spec_ds;

	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	  x_spec_ds = Set_union_acc (x_spec_ds,
				     succ_pf_node->info->pce_info->n_spec_ds);

	/* since we start out with xd_safe sets, don't need to union
	 * with x_comp set. */
	cnt = Set_size (x_spec_ds);
	pce->x_spec_ds = x_spec_ds;

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;
	    n_spec_ds = Set_intersect (pce->trans, x_spec_ds);
	    n_spec_ds = Set_union_acc (n_spec_ds, pce->n_comp);

	    if (!Set_subtract_empty (n_spec_ds, pce->n_spec_ds))
	      change++;
	    RD_SET_CLEAR (pce->n_spec_ds);
	    pce->n_spec_ds = n_spec_ds;
	  }
      }
    }
  while (change);

  /* Need to kill store and other non-fully-anticipable PEIs. */
  fragile_U = pred_flow->fragile_U;
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    temp = Set_subtract (fragile_U, pce->xd_safe);
    pce->x_spec_ds = Set_subtract_acc (pce->x_spec_ds, temp);
    RD_SET_CLEAR (temp);
    temp = Set_subtract (fragile_U, pce->nd_safe);
    pce->n_spec_ds = Set_subtract_acc (pce->n_spec_ds, temp);
    RD_SET_CLEAR (temp);

#ifdef DEBUG_PRE_SPEC_NODE_DS_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_SPEC_DS", pce->n_spec_ds);
    Set_print (stderr, "\tX_SPEC_DS", pce->x_spec_ds);
#endif
    /* Prepare for partial up-safety run for graph reduction. */
    Set_dispose (pce->n_spec_us);
    Set_dispose (pce->x_spec_us);
    pce->n_spec_us = Set_copy (pce->nu_safe);
    pce->x_spec_us = Set_union (pce->xu_safe, pce->x_comp);
    pf_node->info->cnt = Set_size (pce->nu_safe);
  }

  /* Graph reduction: Calculate partial up-safety (availability) */
  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	Set n_spec_us, x_spec_us;
	int cnt;
	pce = pf_node->info->pce_info;
	n_spec_us = pce->n_spec_us;

	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	  n_spec_us = Set_union_acc (n_spec_us,
				     pred_pf_node->info->pce_info->x_spec_us);
	cnt = Set_size (n_spec_us);
	pce->n_spec_us = n_spec_us;

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;
	    temp = Set_union (pce->n_comp, n_spec_us);
	    temp = Set_intersect_acc (temp, pce->trans);
	    x_spec_us = Set_union (temp, pce->x_spec_us);
	    RD_SET_CLEAR (temp);

	    if (!Set_subtract_empty (x_spec_us, pce->x_spec_us))
	      change++;
	    RD_SET_CLEAR (pce->x_spec_us);
	    pce->x_spec_us = x_spec_us;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PRE_SPEC_NODE_US_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_SPEC_US", pce->n_spec_us);
    Set_print (stderr, "\tX_SPEC_US", pce->x_spec_us);
  }
#endif

  /* Mark BB infomation for usage in SPRE min-cut graph construction. */
  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {	 /* just move sets rather than copy them: we don't need the node
	  * sets anymore. */
      DF_PCE_INFO *pce_first, *pce_last;
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_spec_ds);
      RD_SET_CLEAR (pce->xu_safe);
      RD_SET_CLEAR (pce->n_spec_us);
      RD_SET_CLEAR (pce->x_spec_us);
      RD_SET_CLEAR (pce->n_latest);
      RD_SET_CLEAR (pce->x_latest);

      pred_pf_node = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      succ_pf_node = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);

      if (List_size (pf_bb->pf_nodes_entry) == 1)
	{
	  pce_first = pred_pf_node->info->pce_info;
	  pce->n_spec_ds = Set_copy (pce_first->n_spec_ds);
	  pce->n_spec_us = Set_copy (pce_first->n_spec_us);
	}
      else  /* SER 20041204: support for multiple entry nodes */
	{
	  RD_SET_CLEAR (pce->n_spec_ds);
	  RD_SET_CLEAR (pce->n_spec_us);
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_entry)
	  {
	    pce->n_spec_ds =
	      Set_union_acc (pce->n_spec_ds,
			     pf_node->info->pce_info->n_spec_ds);
	    pce->n_spec_us =
	      Set_union_acc (pce->n_spec_us,
			     pf_node->info->pce_info->n_spec_us);
	  }
	}
      pce->n_latest = Set_subtract (pce->n_comp, pce->n_spec_us);

      /* SER 20040212: Must remove all x_latest which are complement,
       * transparent, and partially up-safe. */
#if 0
      pce->x_latest = Set_copy (pce->x_comp);
#else
      temp = Set_intersect (pce->complement, pce->trans);
#if 0
      temp = Set_intersect (temp, pce_first->nu_safe);
#else
      temp = Set_intersect (temp, pce_first->n_spec_us);
#endif
      pce->x_latest = Set_subtract (pce->x_comp, temp);
      RD_SET_CLEAR (temp);
#endif
      if (List_size (pf_bb->pf_nodes_last) == 1)
	{
	  pce_last = succ_pf_node->info->pce_info;
	  pce->xu_safe = Set_copy (pce_last->xu_safe);
	  pce->x_spec_us = Set_copy (pce_last->x_spec_us);
	}
      else
	{
	  pce->xu_safe = Set_copy (pred_flow->expression_U);
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes_last)
	  {
	    pce->xu_safe =
	      Set_intersect_acc (pce->xu_safe,
				 pf_node->info->pce_info->xu_safe);
	    pce->x_spec_us =
	      Set_union_acc (pce->x_spec_us,
			     pf_node->info->pce_info->x_spec_us);
	  }
	}
    }
  }

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    temp = pf_node->info->pce_info->x_spec_us;
    PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
    {
      Set motion_local;
      motion_local =
	Set_intersect (succ_pf_node->info->pce_info->n_spec_ds, temp);
      *motion = Set_union_acc (*motion, motion_local);
      RD_SET_CLEAR (motion_local);
    }
  }
}


static void
D_PRE_speculative_bb_final_sets (PRED_FLOW * pred_flow, Set * motion)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  DF_PCE_INFO *pce;
  Set tmp, tmp2;

  /* Need to add expressions to motion set that weren't operated on in
     graph cut SPRE, but are fully redundant/available. */

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_replace);
      pce->n_replace = Set_intersect (pce->n_comp, pce->nu_safe);
      *motion = Set_union_acc (*motion, pce->n_replace);
    }
  }

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;

      RD_SET_CLEAR (pce->n_insert);
      RD_SET_CLEAR (pce->x_insert);
      pce->n_insert = Set_subtract (pce->n_latest, pce->n_isolated);
      pce->n_insert = Set_intersect_acc (pce->n_insert, *motion);
      pce->x_insert = Set_subtract (pce->x_latest, pce->x_isolated);
      pce->x_insert = Set_intersect_acc (pce->x_insert, *motion);
#if 0
      tmp = Set_intersect (pce->complement, pce->x_isolated);
      pce->x_insert = Set_subtract_acc (pce->x_insert, tmp);
      RD_SET_CLEAR (tmp);
#endif
      RD_SET_CLEAR (pce->n_replace);
      tmp = Set_intersect (pce->n_latest, pce->n_isolated);
      tmp2 = Set_subtract (pce->n_comp, tmp);
      pce->n_replace = Set_union_acc (pce->n_replace, tmp2);
      pce->n_replace = Set_intersect_acc (pce->n_replace, *motion);
      RD_SET_CLEAR (tmp);
      RD_SET_CLEAR (tmp2);

      RD_SET_CLEAR (pce->x_replace);
      tmp = Set_intersect (pce->x_latest, pce->x_isolated);
      pce->x_replace = Set_subtract (pce->x_comp, tmp);
      pce->x_replace = Set_intersect_acc (pce->x_replace, *motion);
      RD_SET_CLEAR (tmp);

      /* Following line needed to prevent spurious insertion when
       * using complements. It should ALWAYS be safe with our setup. */
      pce->x_replace = Set_intersect_acc (pce->x_replace, pce->x_insert);

#ifdef DEBUG_PRE_BB_FINAL_SETS
      fprintf (stderr, "BB starting at op %d in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tN_INSERT", pce->n_insert);
      Set_print (stderr, "\tX_INSERT", pce->x_insert);
      Set_print (stderr, "\tN_REPLACE", pce->n_replace);
      Set_print (stderr, "\tX_REPLACE", pce->x_replace);
#endif
    }
  }
}


/*! \brief Performs analysis for speculative partial redundancy elimination.
 *  See Ryoo's M.S. thesis (2004) for reference.
 */
void
D_PRE_speculative_cut_analysis (PRED_FLOW * pred_flow, int mode, int ignore)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *pf_node_first, *pf_node_last;
  DF_PCE_INFO *pce;
  Graph bb_weighted_graph;
  Set motion = NULL, ignore_set = NULL;
  int mem_conservative_flag = 0, max_motions = 0;

  if ((mode & PCE_MEM_CONSERVATIVE) ||
      !(L_func_acc_specs || (L_func_contains_dep_pragmas && L_use_sync_arcs)))
    mem_conservative_flag = 1;
  if (mode & PCE_CUTSET_METRIC)
    D_dataflow_analysis (pred_flow, LIVE_VARIABLE_CB);
  else
    max_motions = MAX_PRE_MOTIONS;

#ifdef DEBUG_PRE_SPEC_ANALYSIS
  fprintf (stderr, "Starting speculative min-cut PRE analysis on function "
	   "%s.\n", L_fn->name);
#endif

  if (!D_alloc_df_pce_info)
    D_alloc_df_pce_info =
      L_create_alloc_pool ("DF_PCE_IFO", sizeof (DF_PCE_INFO), 512);

  D_compute_expressions_and_operand_use_sets (pred_flow, mode);

  D_PRE_node_local_info (pred_flow, mode, &max_motions);

#ifdef DISPLAY_BB_INFO
  D_print_all_bb_info (pred_flow);
#endif

  /* SER 20050210: relocated due to potential bugs. */
  D_PRE_bb_local_info (pred_flow);

  D_PRE_node_down_safety (pred_flow);
  D_PRE_node_up_safety (pred_flow);

  D_PRE_bb_local_safety_info (pred_flow);

  D_PRE_setup_graph_cut (pred_flow, &motion);
  ignore_set = L_get_expression_subset (ignore);
  motion = Set_subtract_acc (motion, ignore_set);

  RD_SET_CLEAR (ignore_set);

#ifdef DEBUG_PRE_SPEC_ANALYSIS
  Set_print (stderr, "Motion expressions:", motion);
#endif

  if (!Set_empty (motion))
    {
      bb_weighted_graph = L_create_weighted_bb_graph (L_fn, pred_flow);
      L_PRE_speculate_motion (bb_weighted_graph, pred_flow, &motion,
			      mem_conservative_flag, max_motions);

      /* Mark "earliest" positions at head and tail of BBs. */
      PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
      {
	PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
	{
	  pce = pf_bb->info->pce_info;
#ifdef DEBUG_PRE_BB_LATE_SETS
	  fprintf (stderr,
		   "BB starting at op %d (including node %d) in CB %d\n",
		   ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
		   (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
		   pf_bb->pf_cb->cb->id);
	  Set_print (stderr, "\tN_LATEST", pce->n_latest);
	  Set_print (stderr, "\tX_LATEST", pce->x_latest);
#endif
	  RD_SET_CLEAR (pce->n_spec_ds);
	  RD_SET_CLEAR (pce->x_spec_ds);
	  pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
	  pf_node_first->info->pce_info->n_earliest =
	    Set_copy (pce->n_latest);
	  PF_FOREACH_NODE (pf_node_last, pf_bb->pf_nodes_last)
	  {
	    pf_node_last->info->pce_info->x_earliest =
	      Set_copy (pce->x_latest);
	  }
	}
      }

      D_PRE_node_isolated (pred_flow);
      D_PRE_bb_isolated (pred_flow);
      D_PRE_speculative_bb_final_sets (pred_flow, &motion);

      bb_weighted_graph = L_delete_bb_graph (bb_weighted_graph);
    }
  else
    {
      PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
      {
	PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
	{
	  pce = pf_bb->info->pce_info;
	  RD_SET_CLEAR (pce->n_insert);
	  RD_SET_CLEAR (pce->x_insert);
	  RD_SET_CLEAR (pce->n_replace);
	  RD_SET_CLEAR (pce->x_replace);
	}
      }
    }

  /* Cleanup */
  RD_SET_CLEAR (motion);
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->nd_safe);
    RD_SET_CLEAR (pce->xd_safe);
    RD_SET_CLEAR (pce->n_spec_ds);
    RD_SET_CLEAR (pce->x_spec_ds);
    RD_SET_CLEAR (pce->n_comp);
    RD_SET_CLEAR (pce->x_comp);
    RD_SET_CLEAR (pce->n_earliest);
    RD_SET_CLEAR (pce->x_earliest);
    RD_SET_CLEAR (pce->n_isolated);
    RD_SET_CLEAR (pce->x_isolated);
    RD_SET_CLEAR (pce->trans);
  }

  /* Remember to keep down-safety sets for speculative marking. */
  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_comp);
      RD_SET_CLEAR (pce->x_comp);
      RD_SET_CLEAR (pce->trans);
      RD_SET_CLEAR (pce->n_latest);
      RD_SET_CLEAR (pce->x_latest);
      RD_SET_CLEAR (pce->n_isolated);
      RD_SET_CLEAR (pce->x_isolated);
    }
  }
}

/*
 * PDE ANALYSIS (SER)
 * ----------------------------------------------------------------------
 * Used for partial dead code elimination. Originally based on Knoop,
 * Ruthing, Steffen paper in SIGPLAN '95.
 */

static void
D_compute_assignments_and_operand_use_sets (PRED_FLOW * pred_flow, int mode)
{
  PF_CB *pf_cb;
  PF_OPER *pf_oper;
  PF_OPERAND *pf_operand;
  L_Oper *oper;
  L_Attr * attr = NULL;
  L_Expression *general_assn = NULL, *assignment;
  Set operand_set;
  int reg, assn_index, mem_flag = 0, store_only_flag = 0,
    associate_flag = 0, jsr_flag = 0, pred_flag = 0, spec_safe = 0;
#ifdef PRINT_ASSIGNMENTS
  int i;
#endif

  RD_SET_CLEAR (pred_flow->expression_U);
  if (mode & PCE_MEM)
    {
      mem_flag = 1;
      RD_SET_CLEAR (pred_flow->mem_U);
      RD_SET_CLEAR (pred_flow->store_U);
      RD_SET_CLEAR (pred_flow->mem_stack_U);
      if (mode & PDE_STORE)
	{
	  RD_SET_CLEAR (pred_flow->local_var_U);
	  associate_flag = 1;
#ifdef PDE_GENERAL_STORE_PROP
	  RD_SET_CLEAR (pred_flow->store_specific_U);
#endif
	  if (mode & PDE_STORE_ONLY)
	    store_only_flag = 1;
	}
    }
  RD_SET_CLEAR (pred_flow->fragile_U);
  RD_SET_CLEAR (pred_flow->unpred_U);
  if (mode & PDE_PREDICATED)
    pred_flag = 1;

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

  if (pred_flow->hash_mem_oper_conflict)
    HashTable_reset_func (pred_flow->hash_mem_oper_conflict,
			  (void (*)(void *)) Set_dispose);
  else
    pred_flow->hash_mem_oper_conflict = HashTable_create (2048);

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_OPER (pf_oper, pf_cb->pf_opers)
    {
      oper = pf_oper->oper;

      if ((oper->opc == Lop_DEFINE) || (oper->opc == Lop_NO_OP) ||
	  L_is_control_oper (oper) ||
	  L_initializing_predicate_define_opcode (oper) ||
	  L_sync_opcode (oper) ||
	  L_subroutine_call_opcode (oper) ||
	  L_subroutine_return_opcode (oper))
	continue;

      if (!mem_flag)
	{
	  if (L_load_opcode (oper) || L_store_opcode (oper) ||
	      L_subroutine_call_opcode (oper))
	    continue;
	}
      else
	{
	  if (!jsr_flag && L_subroutine_call_opcode (oper))
	    continue;
	  if (store_only_flag && !L_store_opcode (oper))
	    continue;
	}

      assignment = L_generate_assignment_for_oper (oper, 0);
      assn_index = assignment->index;
      pred_flow->expression_U = Set_add (pred_flow->expression_U, assn_index);

      if (!pred_flag || L_PDE_fast_assignment (assignment))
	pred_flow->unpred_U = Set_add (pred_flow->unpred_U, assn_index);

      if (mem_flag)
	{
	  if (L_load_opcode (oper))
	    {
	      pred_flow->mem_U = Set_add (pred_flow->mem_U, assn_index);
	      if (L_stack_reference (oper))
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (attr && attr->field[0]->value.mac == L_MAC_IP)
		    pred_flow->mem_stack_U =
		      Set_add (pred_flow->mem_stack_U, assn_index);
		}
	      else if (L_is_macro (oper->src[0]) &&
		       oper->src[0]->value.mac == L_MAC_IP)
		{
		  attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		  if (!attr)
		    {
		      attr = L_new_attr (STACK_ATTR_NAME, 2);
		      attr->field[0] = L_copy_operand (oper->src[0]);
		      attr->field[1] = L_copy_operand (oper->src[1]);
		      oper->attr = L_concat_attr (oper->attr, attr);
		    }
		  oper->flags = L_SET_BIT_FLAG (oper->flags,
						L_OPER_STACK_REFERENCE);
		  pred_flow->mem_stack_U =
		    Set_add (pred_flow->mem_stack_U, assn_index);
		}
	    }
	  else if (L_store_opcode (oper))
	    {
	      pred_flow->mem_U = Set_add (pred_flow->mem_U, assn_index);
#ifdef PDE_GENERAL_STORE_PROP
	      pred_flow->store_specific_U =
		Set_add (pred_flow->store_specific_U, assn_index);
#endif

	      spec_safe = 0;
	      if (L_stack_reference (oper))
		{
		  if (!(attr = L_find_attr (oper->attr, STACK_ATTR_NAME)))
		    {
		      if (L_is_macro (oper->src[0]) &&
			  (oper->src[0]->value.mac == L_MAC_OP ||
			   oper->src[0]->value.mac == L_MAC_LV))
		      {
			attr = L_new_attr (STACK_ATTR_NAME, 2);
			attr->field[0] = L_copy_operand (oper->src[0]);
			attr->field[1] = L_copy_operand (oper->src[1]);
			oper->attr = L_concat_attr (oper->attr, attr);
			if (oper->src[0]->value.mac == L_MAC_LV)
			  spec_safe = 1;
		      }
		    }
		  if (attr && attr->field[0]->value.mac == L_MAC_OP)
		    pred_flow->mem_stack_U =
		      Set_add (pred_flow->mem_stack_U, assn_index);
		  else if (attr && attr->field[0]->value.mac == L_MAC_LV)
		    {
		      pred_flow->local_var_U =
		        Set_add (pred_flow->local_var_U, assn_index);
		      spec_safe = 1;
		    }
		}
	      else if (L_is_macro (oper->src[0]))
		{
		  if (oper->src[0]->value.mac == L_MAC_OP)
		    {
		      attr = L_find_attr (oper->attr, STACK_ATTR_NAME);
		      if (!attr)
			{
			  attr = L_new_attr (STACK_ATTR_NAME, 2);
			  attr->field[0] = L_copy_operand (oper->src[0]);
			  attr->field[1] = L_copy_operand (oper->src[1]);
			  oper->attr = L_concat_attr (oper->attr, attr);
			}
		      oper->flags = L_SET_BIT_FLAG (oper->flags,
						    L_OPER_STACK_REFERENCE);
		      pred_flow->mem_stack_U =
			Set_add (pred_flow->mem_stack_U, assn_index);
		    }
		  else if (oper->src[0]->value.mac == L_MAC_LV)
		    {
		      pred_flow->local_var_U =
			Set_add (pred_flow->local_var_U, assn_index);
		      spec_safe = 1;
		    }
		}
	      else if (L_has_label_in_attr (oper) || L_is_label (oper->src[0]) ||
		       L_is_label (oper->src[1]))
		spec_safe = 1;

	      pred_flow->store_U = Set_add (pred_flow->store_U, assn_index);
	      /* Find the general store location assignment and place
	       * assignment index in the associates set. */
	      general_assn = L_generate_assignment_for_oper (oper, 1);
	      general_assn->associates = Set_add
		(general_assn->associates, assn_index);
	      pred_flow->expression_U =
		Set_add (pred_flow->expression_U, general_assn->index);
	      pred_flow->mem_U =
		Set_add (pred_flow->mem_U, general_assn->index);
	      pred_flow->store_U = Set_add (pred_flow->store_U,
					    general_assn->index);
	      if (L_stack_reference (oper))
		{
		  if (attr && attr->field[0]->value.mac == L_MAC_OP)
		    pred_flow->mem_stack_U =
		      Set_add (pred_flow->mem_stack_U, general_assn->index);
		  else if (attr && attr->field[0]->value.mac == L_MAC_SP)
		    pred_flow->local_var_U =
		      Set_add (pred_flow->local_var_U, assn_index);
		}

	      /* Allow speculative sinking of generalized stores: we will
	       * do standard register promotion on those. */
	      if (!spec_safe ||
		  L_PDE_fast_assignment (assignment))
		pred_flow->unpred_U = Set_add (pred_flow->unpred_U,
					       general_assn->index);
	      assignment->general = general_assn;
	    }
	  else if (L_subroutine_call_opcode (oper))
	    {
	      pred_flow->mem_U = Set_add (pred_flow->mem_U, assn_index);
	      if (!L_side_effect_free_sub_call (oper))
		pred_flow->store_U = Set_add (pred_flow->store_U, assn_index);
	    }
	}
      if (L_has_fragile_macro_src_operand (oper))
	pred_flow->fragile_U = Set_add (pred_flow->fragile_U, assn_index);
      PF_FOREACH_OPERAND (pf_operand, pf_oper->dest)
      {
	reg = pf_operand->reg;
	operand_set = RD_FIND_OPD_DEF (pred_flow, reg);
	operand_set = Set_add (operand_set, assn_index);
	RD_UPDATE_OPD_DEF (pred_flow, reg, operand_set);
      }

      PF_FOREACH_OPERAND (pf_operand, pf_oper->src)
      {
	reg = pf_operand->reg;
	operand_set = RD_FIND_OPD_USE (pred_flow, reg);
	operand_set = Set_add (operand_set, assn_index);
	RD_UPDATE_OPD_USE (pred_flow, reg, operand_set);

	if (associate_flag && L_store_opcode (oper) &&
	    !(L_same_operand (pf_operand->operand, oper->src[2])))
	  {
	    operand_set = MEM_FIND_CONFLICT (pred_flow, reg);
	    operand_set = Set_add (operand_set, assn_index);
	    operand_set = Set_add (operand_set, general_assn->index);
	    MEM_UPDATE_CONFLICT (pred_flow, reg, operand_set);

	    operand_set = RD_FIND_OPD_USE (pred_flow, reg);
	    operand_set = Set_add (operand_set, general_assn->index);
	    RD_UPDATE_OPD_USE (pred_flow, reg, operand_set);
	  }
      }
    }
  }

#ifdef PRINT_ASSIGNMENTS
  for (i = 1; i <= L_fn->n_expression; i++)
    {
      L_Expression_Hash_Entry *entry;
      entry =
	L_expression_hash_tbl_find_entry (L_fn->expression_index_hash_tbl, i);
      L_print_expression (stderr, entry->expression);
    }
#endif
}


static void
D_PDE_node_local_info (PRED_FLOW * pred_flow, int mode, int * max_motions)
{
  PF_NODE *pf_node;
  PF_INST *pf_inst;
  PF_OPERAND *pf_operand;
  DF_PCE_INFO *pce;
  L_Oper *oper, *dep_oper;
  L_Expression *assignment, *orig_assignment;
  Set loc_delayed, loc_blocked, x_comp, trans, avail_trans, loc_dead,
    tmp, assignment_U, fragile_U, mem_U = NULL, store_U = NULL,
    mem_stack_U = NULL, x_latest, store_trans;
  int i, index, token, reg, mem_flag = 0, mem_conservative_flag = 0,
    store_sink_flag = 0, store_only_flag = 0, local_var_flag = 0,
    pred_flag = 0, cutset_metric_flag = 0, reg_threshold = 0,
    max_noncut_regs = 0;

  fragile_U = pred_flow->fragile_U;
  assignment_U = pred_flow->expression_U;

  if (mode & PCE_MEM)
    {
      mem_flag = 1;
      mem_stack_U = pred_flow->mem_stack_U;
      store_U = pred_flow->store_U;
      if ((mode & PCE_MEM_CONSERVATIVE) ||
	  !(L_func_acc_specs ||
	    (L_func_contains_dep_pragmas && L_use_sync_arcs)))
	{
#ifdef DEBUG_MEM_CONFLICT
	  fprintf (stderr, "PDE: Using conservative memory alias assumptions "
		   "in function %s.\n", pred_flow->fn->name);
#endif
	  mem_conservative_flag = 1;
	  mem_U = pred_flow->mem_U;
	}
#ifdef DEBUG_MEM_CONFLICT
      else if (L_func_acc_specs)
	fprintf (stderr, "PDE: Using AccSpecs for memory alias info in "
		 "function %s.\n", pred_flow->fn->name);
      else if (L_func_contains_dep_pragmas && L_use_sync_arcs)
	fprintf (stderr, "PDE: Using sync arcs for memory alias info in "
		 "function %s.\n", pred_flow->fn->name);
#endif

      if (mode & PDE_STORE)
	{
	  store_sink_flag = 1;
	  if (mode & PDE_STORE_ONLY)
	    store_only_flag = 1;
	}
      if (mode & DEAD_LOCAL_MEM_VAR)
	local_var_flag = 1;
    }
  if (mode & PCE_CUTSET_METRIC)
    {
      cutset_metric_flag = 1;
      reg_threshold = M_num_registers (M_native_int_register_ctype ()) *
	PCE_CUTSET_RATIO;
    }
  if (mode & PDE_MIN_CUT)
    pred_flag = 1;

  /* NOTE: From first glance, it may appear that loc_blocked and trans are
   * simply opposites of each other. However, trans is not set on dest
   * operands. This is very important: for example, a sync operation
   * is loc_blocked but not trans. It is necessary to have both sets.
   */

  /* compute delay local info */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    if (!(pf_node->info->pce_info))
      pf_node->info->pce_info = D_new_df_pce_info ();
    pce = pf_node->info->pce_info;

    loc_delayed = NULL;
    trans = Set_copy (assignment_U);
    avail_trans = Set_copy (assignment_U);
    loc_dead = NULL;
    loc_blocked = NULL;
    x_comp = NULL;
    x_latest = NULL;
    store_trans = Set_copy (assignment_U);

    PF_FOREACH_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;

      if ((oper->opc == Lop_NO_OP) || (oper->opc == Lop_DEFINE))
	continue;

      if (L_sync_opcode (oper) || L_subroutine_return_opcode (oper))
	{
	  RD_SET_CLEAR (loc_delayed);
	  RD_SET_CLEAR (loc_blocked);
	  RD_SET_CLEAR (x_comp);
	  RD_SET_CLEAR (avail_trans);
	  RD_SET_CLEAR (x_latest);
	  RD_SET_CLEAR (store_trans);
	  loc_blocked = Set_copy (assignment_U);
	  continue;
	}

      /* SER: Should we union the sets before calculations? */
      /* cannot move past a redef of src or dest operand */
      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;

	tmp = RD_FIND_OPD_USE (pred_flow, reg);
	loc_delayed = Set_subtract_acc (loc_delayed, tmp);
	loc_blocked = Set_union_acc (loc_blocked, tmp);
        if (pred_flag)
	  {
	    x_comp = Set_subtract_acc (x_comp, tmp);
	    avail_trans = Set_subtract_acc (avail_trans, tmp);
	  }

	tmp = RD_FIND_OPD_DEF (pred_flow, reg);
	loc_delayed = Set_subtract_acc (loc_delayed, tmp);
	loc_blocked = Set_union_acc (loc_blocked, tmp);
	if (pred_flag)
	  {
	    x_comp = Set_subtract_acc (x_comp, tmp);
	    avail_trans = Set_subtract_acc (avail_trans, tmp);
	  }
#ifdef PDE_GENERAL_STORE_PROP
	tmp = MEM_FIND_CONFLICT (pred_flow, reg);
	x_latest = Set_subtract_acc (x_latest, tmp);
	store_trans = Set_subtract_acc (store_trans, tmp);
#endif
      }

      /* cannot move past a use of an assignment's dest */
      PF_FOREACH_OPERAND (pf_operand, pf_inst->src)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_DEF (pred_flow, reg);
	loc_delayed = Set_subtract_acc (loc_delayed, tmp);
	trans = Set_subtract_acc (trans, tmp);
	loc_blocked = Set_union_acc (loc_blocked, tmp);
      }

      if (L_is_control_oper (oper) || L_has_unsafe_macro_src_operand (oper))
	continue;

      if (L_load_opcode (oper) || L_store_opcode (oper) ||
	  L_subroutine_call_opcode (oper))
	{
	  if (mem_flag)
	    {
	      if (mem_conservative_flag)
		{
		  if (L_load_opcode (oper))
		    {
		      loc_delayed = Set_subtract_acc (loc_delayed, store_U);
		      trans = Set_subtract_acc (trans, store_U);
		      loc_blocked = Set_union_acc (loc_blocked, store_U);
		      x_latest = Set_subtract_acc (x_latest, store_U);
		      store_trans = Set_subtract_acc (store_trans, store_U);
		      if (pred_flag)
			{
			  x_comp = Set_subtract_acc (x_comp, store_U);
			  avail_trans = Set_subtract_acc (avail_trans,
							  store_U);
			}
		    }
		  else if (L_store_opcode (oper) ||
			   (L_subroutine_call_opcode (oper) &&
			    !L_side_effect_free_sub_call (oper)))
		    {
		      loc_delayed = Set_subtract_acc (loc_delayed, mem_U);
		      trans = Set_subtract_acc (trans, store_U);
		      loc_blocked = Set_union_acc (loc_blocked, mem_U);
		      x_latest = Set_subtract_acc (x_latest, store_U);
		      store_trans = Set_subtract_acc (store_trans, store_U);
		      if (pred_flag)
			{
			  x_comp = Set_subtract_acc (x_comp, mem_U);
			  avail_trans = Set_subtract_acc (avail_trans, mem_U);
			}
		    }
		}
	      else if (L_func_acc_specs)
		{
		  Set conflicts;

		  if (L_load_opcode (oper) || L_store_opcode (oper))
		    {
		      token = L_generate_assignment_token_from_oper (oper, 0);
		      orig_assignment = L_find_oper_assignment_in_hash
			(L_fn->expression_token_hash_tbl, token, oper, 0);
		      conflicts = orig_assignment->conflicts;
		      loc_delayed = Set_subtract_acc (loc_delayed, conflicts);
		    }
		  else  /* subroutine call, check all expressions */
		    {
		      conflicts = L_get_jsr_conflicting_expressions (oper);
		    }
		  loc_delayed = Set_subtract_acc (loc_delayed, conflicts);
		  trans = Set_subtract_acc (trans, conflicts);
		  loc_blocked = Set_union_acc (loc_blocked, conflicts);
		  store_trans = Set_subtract_acc (store_trans, conflicts);
		  x_latest = Set_subtract_acc (x_latest, conflicts);
		  if (pred_flag)
		    {
		      x_comp = Set_subtract_acc (x_comp, conflicts);
		      avail_trans = Set_subtract_acc (avail_trans, conflicts);
		    }
		  if (L_subroutine_call_opcode (oper))
		    RD_SET_CLEAR (conflicts);
		}
	      else if (oper->sync_info)
		{
		  token = L_generate_assignment_token_from_oper (oper, 0);
		  orig_assignment = L_find_oper_assignment_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 0);

		  for (i = 0; i < oper->sync_info->num_sync_out; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		      if (L_subroutine_return_opcode (dep_oper))
			continue;
		      token = L_generate_assignment_token_from_oper
			(dep_oper, 0);
		      assignment = L_find_oper_assignment_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!assignment)
			continue;
		      if (assignment == orig_assignment)
			continue;
		      index = assignment->index;
		      loc_delayed = Set_delete (loc_delayed, index);
		      if (!L_load_opcode (dep_oper))
			trans = Set_delete (trans, index);
		      x_latest = Set_delete (x_latest, index);
		      store_trans = Set_delete (store_trans, index);
		      loc_blocked = Set_add (loc_blocked, index);
		      if (pred_flag)
			{
			  if (!L_load_opcode (oper))
			    {
			      x_comp = Set_delete (x_comp, index);
			      avail_trans = Set_delete (avail_trans, index);
			    }
#ifdef PDE_GENERAL_STORE_PROP
			  if (L_store_opcode (assignment))
			    {
			      token = L_generate_assignment_token_from_oper
				(dep_oper, 1);
			      assignment = L_find_oper_assignment_in_hash
				(L_fn->expression_token_hash_tbl, token,
				 dep_oper, 1);
			      if (orig_assignment &&
				  assignment == orig_assignment->general)
				continue;
			      index = assignment->index;
			      loc_delayed = Set_delete (loc_delayed, index);
			      trans = Set_delete (trans, index);
			      loc_blocked = Set_add (loc_blocked, index);
			      x_latest = Set_delete (x_latest, index);
			      store_trans = Set_delete (store_trans, index);
			      if (!L_load_opcode (oper))
				{
				  x_comp = Set_delete (x_comp, index);
				  avail_trans = Set_delete (avail_trans, index);
				}
			    }
#endif
			}
		    }
		  for (i = 0; i < oper->sync_info->num_sync_in; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		      if (L_subroutine_return_opcode (dep_oper))
			continue;
		      token = L_generate_assignment_token_from_oper
			(dep_oper, 0);
		      assignment = L_find_oper_assignment_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!assignment)
			continue;
		      if (assignment == orig_assignment)
			continue;
		      index = assignment->index;
		      loc_delayed = Set_delete (loc_delayed, index);
		      if (!L_load_opcode (dep_oper))
			trans = Set_delete (trans, index);
		      x_latest = Set_delete (x_latest, index);
		      store_trans = Set_delete (store_trans, index);
		      loc_blocked = Set_add (loc_blocked, index);
		      if (pred_flag)
			{
			  if (!L_load_opcode (oper))
			    {
			      x_comp = Set_delete (x_comp, index);
			      avail_trans = Set_delete (avail_trans, index);
			    }
#ifdef PDE_GENERAL_STORE_PROP
			  if (L_store_opcode (assignment))
			    {
			      token = L_generate_assignment_token_from_oper
				(dep_oper, 1);
			      assignment = L_find_oper_assignment_in_hash
				(L_fn->expression_token_hash_tbl, token,
				 dep_oper, 1);
			      if (orig_assignment &&
				  assignment == orig_assignment->general)
				continue;
			      index = assignment->index;
			      loc_delayed = Set_delete (loc_delayed, index);
			      trans = Set_delete (trans, index);
			      loc_blocked = Set_add (loc_blocked, index);
			      x_latest = Set_delete (x_latest, index);
			      store_trans = Set_delete (store_trans, index);
			      if (!L_load_opcode (oper))
				{
				  x_comp = Set_delete (x_comp, index);
				  avail_trans = Set_delete (avail_trans, index);
				}
			    }
#endif
			}
		    }
		}

	      if (L_subroutine_call_opcode (oper) &&
		  !L_side_effect_free_sub_call (oper))
		{
		  loc_delayed = Set_subtract_acc (loc_delayed, mem_stack_U);
		  trans = Set_subtract_acc (trans, mem_stack_U);
		  loc_blocked = Set_union_acc (loc_blocked, mem_stack_U);
		  if (pred_flag)
		    {
		      x_comp = Set_subtract_acc (x_comp, mem_stack_U);
		      avail_trans = Set_subtract_acc (avail_trans,
						      mem_stack_U);
		    }
		}

	    }
	}

      if (L_subroutine_call_opcode (oper))
	{
	  loc_delayed = Set_subtract_acc (loc_delayed, fragile_U);
	  trans = Set_subtract_acc (trans, fragile_U);
	  loc_blocked = Set_union_acc (loc_blocked, fragile_U);
	  if (pred_flag)
	    {
	      x_comp = Set_subtract_acc (x_comp, fragile_U);
	      avail_trans = Set_subtract_acc (avail_trans, fragile_U);
	    }
	  continue;
	}
      if (!mem_flag && (L_load_opcode (oper) || L_store_opcode (oper)))
	continue;
      if (L_store_opcode (oper))
	{
	  if (!store_sink_flag)
	    continue;
	}
      else
	{
	  if (store_only_flag)
	    continue;
	}
      if (EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE))
	continue;
      /* SER 20050215: Block moves don't have proper annotation, don't try
       * to optimize at this time. */
      if (L_find_attr (oper->attr, "block_move"))
	continue;
      if (!(pf_inst->pred_true))
	continue;

      token = L_generate_assignment_token_from_oper (oper, 0);
      orig_assignment = L_find_oper_assignment_in_hash
	(L_fn->expression_token_hash_tbl, token, oper, 0);
      if (orig_assignment)
	{
	  loc_delayed = Set_add (loc_delayed, orig_assignment->index);
	  if (pred_flag)
	    {
	      x_comp = Set_add (x_comp, orig_assignment->index);
#ifdef PDE_GENERAL_STORE_PROP
	      if (L_store_opcode (oper))
		{
		  x_latest = Set_add (x_latest, orig_assignment->index);
		  token = L_generate_assignment_token_from_oper (oper, 1);
		  orig_assignment = L_find_oper_assignment_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 1);
		  loc_delayed = Set_add (loc_delayed, orig_assignment->index);
		  x_comp = Set_add (x_comp, orig_assignment->index);
		  x_latest = Set_add (x_latest, orig_assignment->index);
		}
#endif
	      /* Have to kill availability if operand is redefed: need to
	       * do this here in case op redefs its own operand */
	      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
		{
		  reg = pf_operand->reg;
		  tmp = RD_FIND_OPD_USE (pred_flow, reg);
		  x_comp = Set_subtract_acc (x_comp, tmp);
		}
	    }
	}
    }

    /* Cutset metric. */
    if (cutset_metric_flag &&
	pf_node->pf_cb->cb->weight > PRESSURE_EXECUTION_THRESHOLD)
      {
	int num_regs = Set_size (pf_node->pf_cb->info->v_in);
	if (num_regs >= reg_threshold)
	  {
	    RD_SET_CLEAR (trans);
#ifdef DEBUG_CUTSET_METRIC
	    fprintf (stderr, "PDE cutset metric: limiting motion through cb "
		     "%d.\n", pf_node->pf_cb->cb->id);
#endif
	  }
	else
	  {
	    if (max_noncut_regs < num_regs)
	      max_noncut_regs = num_regs;
#ifdef DEBUG_CUTSET_METRIC
	    fprintf (stderr, "PDE cutset metric: free motion through cb %d, "
		     "with %d lives.\n", pf_node->pf_cb->cb->id, num_regs);
#endif
	  }
      }

    RD_SET_CLEAR (pce->loc_delayed);
    RD_SET_CLEAR (pce->n_comp);
    RD_SET_CLEAR (pce->x_comp);
    RD_SET_CLEAR (pce->n_replace);
    RD_SET_CLEAR (pce->n_latest);
    RD_SET_CLEAR (pce->x_latest);
    pce->loc_delayed = loc_delayed;
    pce->n_comp = loc_blocked;
    pce->x_comp = x_comp;
    pce->n_replace = avail_trans;
    pce->n_latest = store_trans;
    pce->x_latest = x_latest;

    /* Calculate deadness local info */
    PF_FORHCAE_INST (pf_inst, pf_node->pf_insts)
    {
      oper = pf_inst->pf_oper->oper;
      if ((oper->opc == Lop_NO_OP) || (oper->opc == Lop_DEFINE) ||
	  L_sync_opcode (oper))
	continue;

      PF_FOREACH_OPERAND (pf_operand, pf_inst->dest)
      {
	reg = pf_operand->reg;
	/* We don't want to set predicated instructions as dead,
	 * because it will make them falsely partially dead. */
	if (!(oper->pred[0]) && pf_inst->pred_true)
	  {
	    tmp = RD_FIND_OPD_DEF (pred_flow, reg);
	    loc_dead = Set_union_acc (loc_dead, tmp);
	  }
	/* If we are doing store elimination, a store is no longer
	 * dead if its address operands are redefined. */
	if (store_sink_flag)
	  {
	    tmp = MEM_FIND_CONFLICT (pred_flow, reg);
	    loc_dead = Set_subtract_acc (loc_dead, tmp);
	    trans = Set_subtract_acc (trans, tmp);
	  }
      }

      if (L_subroutine_return_opcode (oper) && !(oper->pred[0]))
	{
	  RD_SET_CLEAR (loc_dead);
	  loc_dead = Set_copy (assignment_U);
	  if (mem_flag)
	    {
	      loc_dead = Set_subtract_acc (loc_dead, store_U);

	      if (store_sink_flag && local_var_flag)
		{
		  loc_dead = Set_union_acc (loc_dead, pred_flow->local_var_U);
		  loc_dead = Set_union_acc (loc_dead, pred_flow->mem_stack_U);
		}
	    }
	}

      PF_FOREACH_OPERAND (pf_operand, pf_inst->src)
      {
	reg = pf_operand->reg;
	tmp = RD_FIND_OPD_DEF (pred_flow, reg);
	loc_dead = Set_subtract_acc (loc_dead, tmp);
      }

      /* If we can't push memory ops past other ops, not dead. */
      /* If sinking stores, dead if short expression matches. */
      if (L_load_opcode (oper) || L_store_opcode (oper) ||
	  L_subroutine_call_opcode (oper))
	{
	  if (mem_flag)
	    {
	      if (mem_conservative_flag)
		{
		  if (L_load_opcode (oper))
		    loc_dead = Set_subtract_acc (loc_dead, store_U);
		  if (L_store_opcode (oper) ||
		      !L_side_effect_free_sub_call (oper))
		    loc_dead = Set_subtract_acc (loc_dead, mem_U);
		}
	      else if (L_func_acc_specs)
		{
		  if (L_load_opcode (oper) || L_store_opcode (oper))
		    {
		      token = L_generate_assignment_token_from_oper (oper, 0);
		      orig_assignment = L_find_oper_assignment_in_hash
			(L_fn->expression_token_hash_tbl, token, oper, 0);
		      loc_dead = Set_subtract_acc
			(loc_dead, orig_assignment->conflicts);
		    }
		  else  /* subroutine call, check all expressions */
		    {
		      Set conflicts = L_get_jsr_conflicting_expressions (oper);
		      loc_dead = Set_subtract_acc (loc_dead, conflicts);
		      RD_SET_CLEAR (conflicts);
		    }
		}
	      else if (oper->sync_info)
		{
		  token = L_generate_assignment_token_from_oper (oper, 0);
		  orig_assignment = L_find_oper_assignment_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 0);

		  for (i = 0; i < oper->sync_info->num_sync_out; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_out)[i])->dep_oper;
		      if (L_subroutine_return_opcode (dep_oper) ||
			  L_load_opcode (dep_oper))
			continue;
		      token = L_generate_assignment_token_from_oper
			(dep_oper, 0);
		      assignment = L_find_oper_assignment_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!assignment)
			continue;
		      if (orig_assignment &&
			  assignment->general == orig_assignment->general)
			continue;
		      loc_dead = Set_delete (loc_dead, assignment->index);
		      if (pred_flag)
			{
			  assignment = assignment->general;
			  if (!assignment)
			    continue;
			  loc_dead = Set_delete (loc_dead, assignment->index);
			}
		    }
		  for (i = 0; i < oper->sync_info->num_sync_in; i++)
		    {
		      dep_oper = ((oper->sync_info->sync_in)[i])->dep_oper;
		      if (L_subroutine_return_opcode (dep_oper) ||
			  L_load_opcode (dep_oper))
			continue;
		      token = L_generate_assignment_token_from_oper
			(dep_oper, 0);
		      assignment = L_find_oper_assignment_in_hash
			(L_fn->expression_token_hash_tbl, token, dep_oper, 0);
		      if (!assignment)
			continue;
		      if (orig_assignment &&
			  assignment->general == orig_assignment->general)
			continue;
		      loc_dead = Set_delete (loc_dead, assignment->index);
		      if (pred_flag)
			{
			  assignment = assignment->general;
			  if (!assignment)
			    continue;
			  loc_dead = Set_delete (loc_dead, assignment->index);
			}
		    }
		}
	      if (L_subroutine_call_opcode (oper))
		{
		  loc_dead = Set_subtract_acc (loc_dead, fragile_U);
		  if (!L_side_effect_free_sub_call (oper))
		    loc_dead = Set_subtract_acc (loc_dead, mem_stack_U);
		}
	      if (L_store_opcode (oper) && store_sink_flag &&
		  !(oper->pred[0]))
		{
		  token = L_generate_assignment_token_from_oper (oper, 1);
		  assignment = L_find_oper_assignment_in_hash
		    (L_fn->expression_token_hash_tbl, token, oper, 1);
		  if (assignment)
		    {
		      loc_dead = Set_union_acc (loc_dead,
						assignment->associates);
#ifdef PDE_GENERAL_STORE_PROP
		      if (pred_flag)
			loc_dead = Set_add (loc_dead, assignment->index);
#endif
		    }
		}
	    }
	  else if (L_subroutine_call_opcode (oper) &&
		   !L_side_effect_free_sub_call (oper))
	    loc_dead = Set_subtract_acc (loc_dead, fragile_U);
	}
    }

    RD_SET_CLEAR (pce->complement);
    RD_SET_CLEAR (pce->trans);
    pce->complement = loc_dead;
    pce->trans = trans;

#ifdef DEBUG_PDE_NODE_LOCAL_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tLOC_DELAY", loc_delayed);
    Set_print (stderr, "\tLOC_BLOCKED", loc_blocked);
    Set_print (stderr, "\tTRANS", trans);
    if (pred_flag)
      {
	Set_print (stderr, "\tLOC_AVAIL", x_comp);
	Set_print (stderr, "\tAVAIL_TRANS", avail_trans);
	Set_print (stderr, "\tSTORE_AVAIL", x_latest);
	Set_print (stderr, "\tSTORE_TRANS", store_trans);
      }
    Set_print (stderr, "\tLOC_DEAD", loc_dead);
#endif
  }

  if (cutset_metric_flag)
    {
      *max_motions = reg_threshold - max_noncut_regs;
#ifdef DEBUG_CUTSET_METRIC
      fprintf (stderr, "Max number of P3DE motions: %d\n", *max_motions);
#endif
    }
}


/*! \brief This function propagates blocking information for nodes in each BB
 * in order to prevent false partial sinkability.
 *
 * \note This will prevent correct calculation of full sinkability,
 * so do NOT run it until after computing local sets.
 */
static void
D_PDE_node_correct_blocking_sets (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node_first, *pf_node_last, *pf_node, *succ_pf_node;
  DF_PCE_INFO *pce;
  Set succ_blocked;
  int change, single_first, single_last;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);
      single_first = (List_size (pf_bb->pf_nodes_entry) == 1);
      single_last = (List_size (pf_bb->pf_nodes_last) == 1);

      if (single_first && single_last && pf_node_first == pf_node_last)
	continue;
      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	pf_node->info->cnt = Set_size (pf_node->info->pce_info->n_comp);

      do
	{
	  change = 0;
	  PF_FORHCAE_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    int cnt = 0;

	    if (single_last)
	      {
		if (pf_node == pf_node_last)
		  continue;
	      }
	    else if (Node_In_List (pf_node, pf_bb->pf_nodes_last))
	      continue;

	    pce = pf_node->info->pce_info;
	    succ_blocked = NULL;
	    PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	      succ_blocked =
	      Set_union_acc (succ_blocked,
			     succ_pf_node->info->pce_info->n_comp);
	    pce->n_comp = Set_union_acc (pce->n_comp, succ_blocked);
	    pce->loc_delayed = Set_subtract_acc (pce->loc_delayed,
						 succ_blocked);
	    RD_SET_CLEAR (succ_blocked);
	    cnt = Set_size (pce->n_comp);
	    if (cnt > pf_node->info->cnt)
	      {
		pf_node->info->cnt = cnt;
		change++;
	      }
	  }
	}
      while (change);

#ifdef DEBUG_PDE_NODE_LOCAL_SETS
      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	{
	  pce = pf_node->info->pce_info;
	  fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
		   pf_node->pf_cb->cb->id);
	  Set_print (stderr, "\tLOC_MOD_DELAY", pce->loc_delayed);
	  Set_print (stderr, "\tLOC_MOD_BLOCKED", pce->n_comp);
	}
#endif
    }
  }
}

static void
D_PDE_node_dead (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *succ_pf_node;
  DF_PCE_INFO *pce, *succ_pce;
  Set dead_in, dead_out, part_dead_in, part_dead_out, assignment_U;
  int change, set_size;

  assignment_U = pred_flow->expression_U;
  set_size = Set_size (assignment_U);

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    /* Sets for fully dead code analysis */
    RD_SET_CLEAR (pce->n_isolated);
    RD_SET_CLEAR (pce->x_isolated);
    pce->n_isolated = Set_union (pce->complement, pce->trans);
    pce->x_isolated = Set_copy (assignment_U);
    pf_node->info->cnt = set_size;

    /* Sets for partial dead code analysis. */
    RD_SET_CLEAR (pce->n_spec_ds);
    RD_SET_CLEAR (pce->x_spec_ds);
    pce->n_spec_ds = Set_copy (pce->complement);
  }

  /* First, we simultaneously run full and partial dead dataflows until
   * the full dead dataflow converges. */
  do
    {
      change = 0;
      PF_FORHCAE_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	dead_out = pce->x_isolated;
	part_dead_out = pce->x_spec_ds;

	succ_pce = 0;
	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	{
	  succ_pce = succ_pf_node->info->pce_info;
	  dead_out = Set_intersect_acc (dead_out, succ_pce->n_isolated);
	  part_dead_out = Set_union_acc (part_dead_out, succ_pce->n_spec_ds);
	}

	/* If a block has no successors, clear out dead set. */
	if (succ_pce == 0)
	  RD_SET_CLEAR (dead_out);

	pce->x_isolated = dead_out;
	pce->x_spec_ds = part_dead_out;
	cnt = Set_size (dead_out);

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    dead_in = Set_intersect (dead_out, pce->trans);
	    part_dead_in = Set_intersect (part_dead_out, pce->trans);

	    dead_in = Set_union_acc (dead_in, pce->complement);
	    part_dead_in = Set_union_acc (part_dead_in, pce->complement);

	    if (!Set_subtract_empty (pce->n_isolated, dead_in))
	      change++;

	    RD_SET_CLEAR (pce->n_isolated);
	    RD_SET_CLEAR (pce->n_spec_ds);
	    pce->n_isolated = dead_in;
	    pce->n_spec_ds = part_dead_in;
	  }
      }
    }
  while (change);

  /* When the previous loop terminates, it is possible/probable that the
   * partial dead dataflow is not complete. Need to finish up. */
  /* First, reset sizes to partial dead sets. */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    pf_node->info->cnt = Set_size (pce->x_spec_ds);
    part_dead_in = Set_intersect (pce->x_spec_ds, pce->trans);
    part_dead_in = Set_union_acc (part_dead_in, pce->complement);
    RD_SET_CLEAR (pce->n_spec_ds);
    pce->n_spec_ds = part_dead_in;
  }

  do
    {
      change = 0;
      PF_FORHCAE_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	part_dead_out = pce->x_spec_ds;

	succ_pce = 0;
	PF_FOREACH_NODE (succ_pf_node, pf_node->succ)
	{
	  succ_pce = succ_pf_node->info->pce_info;
	  part_dead_out = Set_union_acc (part_dead_out, succ_pce->n_spec_ds);
	}
	if (succ_pce == 0)
	  RD_SET_CLEAR (part_dead_out);

	pce->x_spec_ds = part_dead_out;
	cnt = Set_size (part_dead_out);

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;
	    part_dead_in = Set_intersect (part_dead_out, pce->trans);
	    part_dead_in = Set_union_acc (part_dead_in, pce->complement);
	    if (!Set_subtract_empty (part_dead_in, pce->n_spec_ds))
	      change++;
	    RD_SET_CLEAR (pce->n_spec_ds);
	    pce->n_spec_ds = part_dead_in;
	  }
      }
    }
  while (change);

  /* Before we go to delay stages, we can reduce the loc_delay sets to
   * remove assignments which are not partially dead. This makes for
   * faster convergence */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    pce->loc_delayed = Set_intersect_acc (pce->loc_delayed, pce->x_spec_ds);
#ifdef DEBUG_PDE_DEAD_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tPROF_LOC_DELAY", pce->loc_delayed);
    Set_print (stderr, "\tDEAD_IN", pce->n_isolated);
    Set_print (stderr, "\tDEAD_OUT", pce->x_isolated);
    Set_print (stderr, "\tPARTIAL_DEAD_IN", pce->n_spec_ds);
    Set_print (stderr, "\tPARTIAL_DEAD_OUT", pce->x_spec_ds);
#endif
  }
}


static void
D_PDE_node_delay (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node, *succ_pf_node;
  DF_PCE_INFO *pce;
  Set n_delayed, x_delayed, assignment_U;
  int change, size;

  assignment_U = pred_flow->expression_U;
  size = Set_size (assignment_U);
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->n_delayed);
    RD_SET_CLEAR (pce->x_delayed);
    pce->n_delayed = Set_copy (assignment_U);
    pce->x_delayed = Set_subtract_union (assignment_U, pce->n_comp,
					 pce->loc_delayed);
    pf_node->info->cnt = size;
  }

  do
    {
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	n_delayed = pce->n_delayed;

	/* n_delayed[S] = intersection (x_delayed[] of all successors of S)
	 */
	cnt = 0;
	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	{
	  cnt = 1;
	  n_delayed =
	    Set_intersect_acc (n_delayed,
			       pred_pf_node->info->pce_info->x_delayed);
	}
	/* The purpose of the following is that we don't want to move things
	 * into the last nodes...generally it's just an epilogue and
	 * return code. Doing so causes some problems with assumptions
	 * in other optimizations.
	 */
	succ_pf_node = (PF_NODE *) List_get_first (pf_node->succ);
	if (cnt == 0 || !(succ_pf_node))
	  RD_SET_CLEAR (n_delayed);
	cnt = Set_size (n_delayed);
	pce->n_delayed = n_delayed;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;
	    x_delayed = Set_subtract_union (n_delayed, pce->n_comp,
					    pce->loc_delayed);
	    /* Following line prevents sinking past profitable points. */
	    x_delayed = Set_intersect_acc (x_delayed, pce->x_spec_ds);
	    x_delayed = Set_subtract_acc (x_delayed, pce->x_isolated);
	    if (!Set_subtract_empty (pce->x_delayed, x_delayed))
	      change++;

	    RD_SET_CLEAR (pce->x_delayed);
	    pce->x_delayed = x_delayed;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PDE_NODE_DELAY_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_DELAYED", pf_node->info->pce_info->n_delayed);
    Set_print (stderr, "\tX_DELAYED", pf_node->info->pce_info->x_delayed);
  }
#endif
}


static void
D_PDE_bb_final_sets (PRED_FLOW * pred_flow)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node_first, *pf_node_last, *pf_node;
  DF_PCE_INFO *pce, *pce_first, *pce_last = NULL;
  Set n_insert, x_insert, loc_delayed, loc_blocked, delete, assignment_U;
  int single_first, single_last;

  assignment_U = pred_flow->expression_U;

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      if (!pce)
	{
	  pf_bb->info->pce_info = D_new_df_pce_info ();
	  pce = pf_bb->info->pce_info;
	}
      else
	{
	  RD_SET_CLEAR (pce->n_comp);
	  RD_SET_CLEAR (pce->loc_delayed);
	  RD_SET_CLEAR (pce->n_delayed);
	  RD_SET_CLEAR (pce->x_delayed);
	  RD_SET_CLEAR (pce->n_isolated);
	  RD_SET_CLEAR (pce->x_isolated);
	  RD_SET_CLEAR (pce->n_insert);
	  RD_SET_CLEAR (pce->x_replace);
	}

      pf_node_first = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      pf_node_last = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);
      single_first = (List_size (pf_bb->pf_nodes_entry) == 1);
      single_last = (List_size (pf_bb->pf_nodes_last) == 1);

      if (single_first)
	{
	  pce_first = pf_node_first->info->pce_info;
	  pce->n_delayed = Set_copy (pce_first->n_delayed);
	  pce->n_isolated = Set_copy (pce_first->n_isolated);
	}
      else
	{
	  pce->n_delayed = Set_copy (assignment_U);
	  pce->x_isolated = Set_copy (assignment_U);
	  List_start (pf_bb->pf_nodes_entry);
	  while ((pf_node_first = (PF_NODE *)
		  List_next (pf_bb->pf_nodes_entry)))
	    {
	      pce_first = pf_node_first->info->pce_info;
	      pce->n_delayed = Set_intersect_acc (pce->n_delayed,
						  pce_first->n_delayed);
	      pce->x_isolated = Set_intersect_acc (pce->x_isolated,
						   pce_first->x_isolated);
	    }
	}

      if (single_last)
	{
	  pce_last = pf_node_last->info->pce_info;
	  pce->x_delayed = Set_copy (pce_last->x_delayed);
	  pce->x_isolated = Set_copy (pce_last->x_isolated);
	}
      else
	{
	  pce->x_delayed = Set_copy (assignment_U);
	  pce->x_isolated = Set_copy (assignment_U);
	  List_start (pf_bb->pf_nodes_last);
	  while ((pf_node_last = (PF_NODE *)
		  List_next (pf_bb->pf_nodes_last)))
	    {
	      pce_last = pf_node_last->info->pce_info;
	      pce->x_delayed = Set_intersect_acc (pce->x_delayed,
						  pce_last->x_delayed);
	      pce->x_isolated = Set_intersect_acc (pce->x_isolated,
						   pce_last->x_isolated);
	    }
	}

      if (single_first && single_last && pf_node_first == pf_node_last)
	{
	  pce->loc_delayed = Set_copy (pce_first->loc_delayed);
	  pce->n_comp = Set_copy (pce_first->n_comp);
	}
      else
	{
	  loc_delayed = Set_subtract (pce->x_delayed, pce->n_delayed);
	  loc_blocked = NULL;
	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	    {
	      loc_blocked =
		Set_union_acc (loc_blocked, pf_node->info->pce_info->n_comp);
	      loc_delayed =
		Set_union_acc (loc_delayed,
			       pf_node->info->pce_info->loc_delayed);
	    }
	  loc_delayed = Set_intersect_acc (loc_delayed, pce->x_delayed);
	  pce->loc_delayed = loc_delayed;
	  pce->n_comp = loc_blocked;
	}

      n_insert = Set_subtract_union (assignment_U, pce->x_delayed,
				     pce->n_comp);
      n_insert = Set_intersect_acc (n_insert, pce->n_delayed);
      n_insert = Set_subtract_acc (n_insert, pce->n_isolated);

      x_insert = Set_subtract (pce->x_delayed, pce->loc_delayed);
      /* If not doing delay fixup, will need to delete dead assignments
       * as well. */
      delete = NULL;
      /* SER 20041215:  Need to check all pf_nodes_last */
      while ((pf_node_last = (PF_NODE *) List_next (pf_bb->pf_nodes_last)))
	{
	  PF_FOREACH_NODE (pf_node, pf_node_last->succ)
	    {
	      delete = Set_union_acc (delete,
				      pf_node->info->pce_info->n_delayed);
	      x_insert = Set_subtract_acc (x_insert,
					   pf_node->info->pce_info->n_delayed);
	    }
	}

      delete = Set_union_acc (delete, pce->x_isolated);
      delete = Set_intersect_acc (delete, pce->loc_delayed);

      /* Union the insert sets together, because it doesn't really matter
       * whether we insert them at the beginning or the end. So we
       * insert all of them at the beginning to simplify things.
       */
      n_insert = Set_union_acc (n_insert, x_insert);
      pce->n_insert = n_insert;
      pce->x_replace = delete;
      RD_SET_CLEAR (x_insert);

#ifdef DEBUG_PDE_BB_FINAL_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tLOC_DELAYED", pce->loc_delayed);
      Set_print (stderr, "\tLOC_BLOCKED", pce->n_comp);
      Set_print (stderr, "\tN_DELAYED", pce->n_delayed);
      Set_print (stderr, "\tX_DELAYED", pce->x_delayed);
      Set_print (stderr, "\tDEAD_IN", pce->n_isolated);
      Set_print (stderr, "\tDEAD_OUT", pce->x_isolated);
      Set_print (stderr, "\tINSERT", pce->n_insert);
      Set_print (stderr, "\tCLEAR", pce->x_replace);
#endif
    }
  }
}


/*! \brief Performs analysis for non-speculative partial dead code elimination.
 * Originally based on Knoop, Ruthing, Steffen paper in SIGPLAN '95. */
static void
D_PDE_analysis (PRED_FLOW * pred_flow, int mode)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node;
  DF_PCE_INFO *pce;
  int max_motions = 0;

  if (!D_alloc_df_pce_info)
    D_alloc_df_pce_info =
      L_create_alloc_pool ("DF_PCE_INFO", sizeof (DF_PCE_INFO), 512);

#ifdef DEBUG_PDE_ANALYSIS
  fprintf (stderr, "Starting PDE analysis on function %s.\n", L_fn->name);
#endif
#ifdef DISPLAY_BB_INFO
  D_print_all_bb_info (pred_flow);
#endif

  D_compute_assignments_and_operand_use_sets (pred_flow, mode);

  /* Calculate local sets: delayed and blocked */
  D_PDE_node_local_info (pred_flow, mode, &max_motions);
  D_PDE_node_correct_blocking_sets (pred_flow);

  /* Calculate partial dead sets to see how far ops can be moved downward
   * and have some uses killed. */
  D_PDE_node_dead (pred_flow);
  D_PDE_node_delay (pred_flow);
  D_PDE_bb_final_sets (pred_flow);

  /* remove all other sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->n_isolated);
    RD_SET_CLEAR (pce->x_isolated);
    RD_SET_CLEAR (pce->n_spec_ds);
    RD_SET_CLEAR (pce->x_spec_ds);
    RD_SET_CLEAR (pce->loc_delayed);
    RD_SET_CLEAR (pce->n_comp);
    RD_SET_CLEAR (pce->complement);
    RD_SET_CLEAR (pce->trans);
    RD_SET_CLEAR (pce->n_delayed);
    RD_SET_CLEAR (pce->x_delayed);
  }

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->n_isolated);
      RD_SET_CLEAR (pce->x_isolated);
      RD_SET_CLEAR (pce->n_comp);
      RD_SET_CLEAR (pce->loc_delayed);
      RD_SET_CLEAR (pce->n_delayed);
      RD_SET_CLEAR (pce->x_delayed);
    }
  }
}


/*! \brief Calculates partial delay sets for speculative partial dead code
 *  elimination.
 */
static void
D_PDE_node_partial_delay (PRED_FLOW * pred_flow)
{
  PF_NODE *pf_node, *pred_pf_node;
  DF_PCE_INFO *pce, *pce_pred;
  Set nu_safe, xu_safe, n_part_delay, x_part_delay, assignment_U;
  int change, set_size;

  assignment_U = pred_flow->expression_U;
  set_size = Set_size (assignment_U);

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->nu_safe);
    RD_SET_CLEAR (pce->xu_safe);
    pce->nu_safe = Set_copy (assignment_U);
    pce->xu_safe = Set_union (pce->n_replace, pce->x_comp);
    RD_SET_CLEAR (pce->n_spec_us);
    RD_SET_CLEAR (pce->x_spec_us);
    pce->x_spec_us = Set_copy (pce->loc_delayed);
    pf_node->info->cnt = set_size;

    /* We can delete dead info from partial dead info. */
    pce->n_spec_ds = Set_subtract_acc (pce->n_spec_ds, pce->n_isolated);
    pce->x_spec_ds = Set_subtract_acc (pce->x_spec_ds, pce->x_isolated);
  }

  do
    {	 	/* calculate availability set, start part delay convergence */
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	nu_safe = pce->nu_safe;
	n_part_delay = pce->n_spec_us;

	pce_pred = 0;
	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	{
	  pce_pred = pred_pf_node->info->pce_info;
	  nu_safe = Set_intersect_acc (nu_safe, pce_pred->xu_safe);
	  n_part_delay = Set_union_acc (n_part_delay, pce_pred->x_spec_us);
	}
	if (pce_pred == 0)
	  RD_SET_CLEAR (nu_safe);
	if (!((PF_NODE *) List_get_first (pf_node->succ)))
	  RD_SET_CLEAR (n_part_delay);

	cnt = Set_size (nu_safe);
	pce->nu_safe = nu_safe;
	pce->n_spec_us = n_part_delay;

	if (cnt < pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;

	    xu_safe = Set_intersect (nu_safe, pce->n_replace);
	    xu_safe = Set_union_acc (xu_safe, pce->x_comp);
	    x_part_delay = Set_subtract_union (n_part_delay, pce->n_comp,
					       pce->loc_delayed);
	    x_part_delay = Set_intersect_acc (x_part_delay, pce->x_spec_ds);
	    /* x_part_delay = Set_subtract_acc (x_part_delay, pce->x_isolated); */

	    if (!Set_subtract_empty (pce->xu_safe, xu_safe))
	      change++;

	    RD_SET_CLEAR (pce->xu_safe);
	    RD_SET_CLEAR (pce->x_spec_us);
	    pce->xu_safe = xu_safe;
	    pce->x_spec_us = x_part_delay;
	  }
      }
    }
  while (change);

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    pf_node->info->cnt = Set_size (pce->n_spec_us);
    x_part_delay = Set_subtract_union (pce->n_spec_us, pce->n_comp,
				       pce->loc_delayed);
    x_part_delay = Set_intersect_acc (x_part_delay, pce->x_spec_ds);
    /* x_part_delay = Set_subtract_acc (x_part_delay, pce->x_isolated); */
    RD_SET_CLEAR (pce->x_spec_us);
    pce->x_spec_us = x_part_delay;
  }

  do
    {				/* finish part delay convergence */
      change = 0;
      PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
      {
	int cnt;
	pce = pf_node->info->pce_info;
	n_part_delay = pce->n_spec_us;

	PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
	{
	  n_part_delay =
	    Set_union_acc (n_part_delay,
			   pred_pf_node->info->pce_info->x_spec_us);
	}
	if (!((PF_NODE *) List_get_first (pf_node->succ)))
	  RD_SET_CLEAR (n_part_delay);

	cnt = Set_size (n_part_delay);
	pce->n_spec_us = n_part_delay;

	if (cnt > pf_node->info->cnt)
	  {
	    pf_node->info->cnt = cnt;
	    x_part_delay = Set_subtract_union (n_part_delay, pce->n_comp,
					       pce->loc_delayed);
	    /* Following line prevents sinking past profitable points. */
	    x_part_delay = Set_intersect_acc (x_part_delay, pce->x_spec_ds);
	    /* x_part_delay = Set_subtract_acc (x_part_delay, pce->x_isolated); */
	    if (!Set_subtract_empty (x_part_delay, pce->x_spec_us))
	      change++;

	    RD_SET_CLEAR (pce->x_spec_us);
	    pce->x_spec_us = x_part_delay;
	  }
      }
    }
  while (change);

#ifdef DEBUG_PDE_NODE_DELAY_SETS
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tN_AVAIL", pce->nu_safe);
    Set_print (stderr, "\tX_AVAIL", pce->xu_safe);
    Set_print (stderr, "\tN_PART_DELAY", pce->n_spec_us);
    Set_print (stderr, "\tX_PART_DELAY", pce->x_spec_us);
  }
#endif
}

static void
D_PDE_setup_graph_cut (PRED_FLOW * pred_flow, Set * motion)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node, *first_node, *last_node;
  DF_PCE_INFO *pce;
  Set motion_local, loc_blocked, unpred_U, expression_U, temp;
  int single_first, single_last;

  /* Need to kill partial sinkability for cheap operations when they are
   * not fully available. */

#ifdef DEBUG_PDE_NODE_DELAY_SETS
  fprintf (stderr, "Modified delay sets:\n");
#endif
  unpred_U = pred_flow->unpred_U;
  expression_U = pred_flow->expression_U;
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    temp = Set_subtract (unpred_U, pce->nu_safe);
    pce->n_spec_us = Set_subtract_acc (pce->n_spec_us, temp);
    RD_SET_CLEAR (temp);
    temp = Set_subtract (unpred_U, pce->xu_safe);
    pce->x_spec_us = Set_subtract_acc (pce->x_spec_us, temp);
    RD_SET_CLEAR (temp);
#ifdef DEBUG_PDE_NODE_DELAY_SETS
    fprintf (stderr, "PF_NODE %d, CB %d\n", pf_node->id,
	     pf_node->pf_cb->cb->id);
    Set_print (stderr, "\tLOC_DELAY", pce->loc_delayed);
    Set_print (stderr, "\tN_PART_DELAY", pce->n_spec_us);
    Set_print (stderr, "\tX_PART_DELAY", pce->x_spec_us);
#endif
  }

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
      {
	/* just move sets rather than copy them: we don't need the node
	 * sets anymore. */
      DF_PCE_INFO *pce_first, *pce_last;
      pce = pf_bb->info->pce_info;
      if (!pce)
	{
	  pf_bb->info->pce_info = D_new_df_pce_info ();
	  pce = pf_bb->info->pce_info;
	}
      else
	{
	  RD_SET_CLEAR (pce->n_comp);
	  RD_SET_CLEAR (pce->loc_delayed);
	  RD_SET_CLEAR (pce->xu_safe);
	  RD_SET_CLEAR (pce->n_spec_us);
	  RD_SET_CLEAR (pce->x_spec_us);
	  RD_SET_CLEAR (pce->n_spec_ds);
	  RD_SET_CLEAR (pce->x_spec_ds);
	  RD_SET_CLEAR (pce->n_isolated);
	  RD_SET_CLEAR (pce->x_isolated);
	  RD_SET_CLEAR (pce->n_insert);
	  RD_SET_CLEAR (pce->x_insert);
	  RD_SET_CLEAR (pce->x_latest);
	  RD_SET_CLEAR (pce->x_replace);
	  RD_SET_CLEAR (pce->pred_guard);
	  RD_SET_CLEAR (pce->pred_set);
	  RD_SET_CLEAR (pce->pred_clear);
	}

      first_node = (PF_NODE *) List_get_first (pf_bb->pf_nodes_entry);
      last_node = (PF_NODE *) List_get_first (pf_bb->pf_nodes_last);
      single_first = (List_size (pf_bb->pf_nodes_entry) == 1);
      single_last = (List_size (pf_bb->pf_nodes_last) == 1);

      if (List_size (pf_bb->pf_nodes_entry) != 1)
	L_punt ("PDE needs modifications to handle multiple entry nodes.");

      if (single_first)
	{
	  pce_first = first_node->info->pce_info;
	  pce->n_spec_us = Set_copy (pce_first->n_spec_us);
	  pce->n_spec_ds = Set_copy (pce_first->n_spec_ds);
	  pce->n_isolated = Set_copy (pce_first->n_isolated);
	}
      else
	{
	  pce->n_isolated = Set_copy (expression_U);
	  while ((first_node = (PF_NODE *) List_next (pf_bb->pf_nodes_entry)))
	    {
	      pce_first = first_node->info->pce_info;
	      pce->n_spec_us = Set_union_acc (pce->n_spec_us,
					      pce_first->n_spec_us);
	      pce->n_spec_ds = Set_union_acc (pce->n_spec_ds,
					      pce_first->n_spec_ds);
	      pce->n_isolated = Set_intersect_acc (pce->n_isolated,
						   pce_first->n_isolated);
	    }
	}

      if (single_last)
	{
	  pce_last = last_node->info->pce_info;
	  pce->xu_safe = Set_copy (pce_last->xu_safe);
	  pce->x_spec_us = Set_copy (pce_last->x_spec_us);
	  pce->x_spec_ds = Set_copy (pce_last->x_spec_ds);
	  pce->x_isolated = Set_copy (pce_last->x_isolated);
	}
      else
	{
	  pce->xu_safe = Set_copy (expression_U);
	  pce->x_isolated = Set_copy (expression_U);
	  List_start (pf_bb->pf_nodes_last);
	  while ((last_node = (PF_NODE *) List_next (pf_bb->pf_nodes_last)))
	    {
	      pce_last = last_node->info->pce_info;
	      pce->xu_safe = Set_intersect_acc (pce->xu_safe,
						pce_last->xu_safe);
	      pce->x_spec_us = Set_union_acc (pce->x_spec_us,
					      pce_last->x_spec_us);
	      pce->x_spec_ds = Set_union_acc (pce->x_spec_ds,
					      pce_last->x_spec_ds);
	      pce->x_isolated = Set_intersect_acc (pce->x_isolated,
						   pce_last->x_isolated);
	    }
	}

      if (single_first && single_last && first_node == last_node)
	{
	  pce->loc_delayed = Set_copy (pce_first->loc_delayed);
	  pce->x_insert = Set_copy (pce_first->loc_delayed);
	  pce->x_latest = Set_copy (pce_first->x_latest);
	  pce->n_comp = Set_copy (pce_first->n_comp);
	}
      else
	{
	  PF_NODE *pred_pf_node;
	  DF_PCE_INFO *node_pce;
	  Set in, out, assignment_U;
	  int size, change;

	  assignment_U = pred_flow->expression_U;
	  size = Set_size (assignment_U);
	  loc_blocked = NULL;

	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    node_pce = pf_node->info->pce_info;
	    loc_blocked = Set_union_acc (loc_blocked, node_pce->n_comp);
	    RD_SET_CLEAR (pf_node->info->in);
	    RD_SET_CLEAR (pf_node->info->out);
	    pf_node->info->in = Set_copy (assignment_U);
	    pf_node->info->out = Set_copy (node_pce->loc_delayed);
	    pf_node->info->cnt = size;
	  }
	  pce->n_comp = loc_blocked;

	  /* compute loc_delayed set for BB */
	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt = 0;
		if (single_first)
		  {
		    if (pf_node == first_node)
		      continue;
		  }
		else if (Node_In_List (pf_node, pf_bb->pf_nodes_entry))
		  continue;

		in = pf_node->info->in;
		PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
		  in = Set_intersect_acc (in, pred_pf_node->info->out);

		cnt = Set_size (in);
		pf_node->info->in = in;

		if (cnt < pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    node_pce = pf_node->info->pce_info;
		    out = Set_subtract (in, node_pce->n_comp);
		    out = Set_union_acc (out, node_pce->loc_delayed);
		    if (!Set_subtract_empty (pf_node->info->out, out))
		      change++;
		    RD_SET_CLEAR (pf_node->info->out);
		    pf_node->info->out = out;
		  }
	      }
	    }
	  while (change);

	  if (List_size (pf_bb->pf_nodes_last) == 1)
	    {
	      pce->loc_delayed = Set_copy (last_node->info->out);
	    }
	  else
	    {
	      pce->loc_delayed = Set_copy (expression_U);
	      List_start (pf_bb->pf_nodes_last);
	      while ((last_node = List_next (pf_bb->pf_nodes_last)))
		{
		  pce->loc_delayed =
		    Set_intersect_acc (pce->loc_delayed,
				       last_node->info->out);
		}
	    }


	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    node_pce = pf_node->info->pce_info;
	    pf_node->info->out = Set_copy (node_pce->loc_delayed);
	    RD_SET_CLEAR (pf_node->info->in);
	    pf_node->info->cnt = 0;
	  }

	  /* compute partial loc_delayed set for BB */
	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt = 0;
		if (pf_node == first_node)
		  continue;
		in = pf_node->info->in;
		PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
		  in = Set_union_acc (in, pred_pf_node->info->out);

		cnt = Set_size (in);
		pf_node->info->in = in;

		if (cnt > pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    node_pce = pf_node->info->pce_info;
		    out = Set_subtract (in, node_pce->n_comp);
		    out = Set_union_acc (out, node_pce->loc_delayed);
		    if (!Set_subtract_empty (out, pf_node->info->out))
		      change++;
		    RD_SET_CLEAR (pf_node->info->out);
		    pf_node->info->out = out;
		  }
	      }
	    }
	  while (change);

	  if (List_size (pf_bb->pf_nodes_last) == 1)
	    {
	      pce->x_insert = Set_copy (last_node->info->out);
	    }
	  else
	    {
	      List_start (pf_bb->pf_nodes_last);
	      while ((last_node = List_next (pf_bb->pf_nodes_last)))
		{
		  pce->x_insert = Set_union_acc (pce->x_insert,
						 last_node->info->out);
		}
	    }

	  PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	  {
	    node_pce = pf_node->info->pce_info;
	    pf_node->info->out = Set_copy (node_pce->x_latest);
	    RD_SET_CLEAR (pf_node->info->in);
	    pf_node->info->cnt = 0;
	  }

	  /* compute partial store delayed set for BB */
	  do
	    {
	      change = 0;
	      PF_FOREACH_NODE (pf_node, pf_bb->pf_nodes)
	      {
		int cnt = 0;
		if (pf_node == first_node)
		  continue;
		in = pf_node->info->in;
		PF_FOREACH_NODE (pred_pf_node, pf_node->pred)
		  in = Set_union_acc (in, pred_pf_node->info->out);

		cnt = Set_size (in);
		pf_node->info->in = in;

		if (cnt > pf_node->info->cnt)
		  {
		    pf_node->info->cnt = cnt;
		    node_pce = pf_node->info->pce_info;
		    out = Set_intersect (in, node_pce->n_latest);
		    out = Set_union_acc (out, node_pce->x_latest);
		    if (!Set_subtract_empty (out, pf_node->info->out))
		      change++;
		    RD_SET_CLEAR (pf_node->info->out);
		    pf_node->info->out = out;
		  }
	      }
	    }
	  while (change);

	  if (List_size (pf_bb->pf_nodes_last) == 1)
	    {
	      pce->x_latest = Set_copy (last_node->info->out);
	    }
	  else
	    {
	      List_start (pf_bb->pf_nodes_last);
	      while ((last_node = List_next (pf_bb->pf_nodes_last)))
		{
		  pce->x_latest = Set_union_acc (pce->x_latest,
						 last_node->info->out);
		}
	    }

	}
      pce->x_replace = Set_intersect (pce->loc_delayed, pce->x_isolated);

#ifdef DEBUG_PDE_BB_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "\tLOC_DELAYED", pce->loc_delayed);
      Set_print (stderr, "\tAVAIL", pce->xu_safe);
      Set_print (stderr, "\tPART_DELAY", pce->x_spec_us);
      Set_print (stderr, "\tDEAD_IN", pce->n_isolated);
      Set_print (stderr, "\tPART_DEAD", pce->x_spec_ds);
#endif
    }
  }

  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    motion_local = Set_intersect (pce->loc_delayed, pce->x_spec_ds);
    motion_local = Set_subtract (motion_local, pce->x_isolated);
    *motion = Set_union_acc (*motion, motion_local);
    RD_SET_CLEAR (motion_local);
  }

#ifdef PDE_GENERAL_STORE_PROP
  *motion = Set_subtract (*motion, pred_flow->store_specific_U);
#endif
}


/*! \brief Performs analysis for speculative partial dead code elimination.
 *  Refer to Ryoo's M.S. thesis (2004).
 */
void
D_PDE_predicated_cut_analysis (PRED_FLOW * pred_flow, int mode, int ignore)
{
  PF_CB *pf_cb;
  PF_BB *pf_bb;
  PF_NODE *pf_node;
  DF_PCE_INFO *pce;
  Graph bb_weighted_graph;
  Set motion = NULL;
  int max_motions = 0;

  if (mode & PCE_CUTSET_METRIC)
    D_dataflow_analysis (pred_flow, LIVE_VARIABLE_CB);

#ifdef DEBUG_PDE_PRED_ANALYSIS
  fprintf (stderr, "Starting predicated min-cut PDE analysis on function "
	   "%s.\n", L_fn->name);
#endif

  if (!D_alloc_df_pce_info)
    D_alloc_df_pce_info =
      L_create_alloc_pool ("DF_PCE_INFO", sizeof (DF_PCE_INFO), 512);

  D_compute_assignments_and_operand_use_sets (pred_flow, mode);

  D_PDE_node_local_info (pred_flow, mode, &max_motions);
  D_PDE_node_correct_blocking_sets (pred_flow);

#ifdef DISPLAY_BB_INFO
  D_print_all_bb_info (pred_flow);
#endif

  D_PDE_node_dead (pred_flow);

  D_PDE_node_partial_delay (pred_flow);
  D_PDE_setup_graph_cut (pred_flow, &motion);

#ifdef DEBUG_PDE_PRED_ANALYSIS
  Set_print (stderr, "Motion assignments:", motion);
#endif

  if (!Set_empty (motion))
    {
      bb_weighted_graph = L_create_weighted_bb_graph (L_fn, pred_flow);
      L_PDE_predicate_motion (bb_weighted_graph, pred_flow, &motion,
			      ((mode & PDE_PREDICATED) ? 0 : 1), max_motions);
      bb_weighted_graph = L_delete_bb_graph (bb_weighted_graph);
    }

  /* remove all other sets */
  PF_FOREACH_NODE (pf_node, pred_flow->list_pf_node)
  {
    pce = pf_node->info->pce_info;
    RD_SET_CLEAR (pce->n_isolated);
    RD_SET_CLEAR (pce->x_isolated);
    RD_SET_CLEAR (pce->n_spec_ds);
    RD_SET_CLEAR (pce->x_spec_ds);
    RD_SET_CLEAR (pce->n_spec_us);
    RD_SET_CLEAR (pce->x_spec_us);
    RD_SET_CLEAR (pce->loc_delayed);
    RD_SET_CLEAR (pce->x_comp);
    RD_SET_CLEAR (pce->n_comp);
    RD_SET_CLEAR (pce->n_latest);
    RD_SET_CLEAR (pce->x_latest);
    RD_SET_CLEAR (pce->complement);
    RD_SET_CLEAR (pce->trans);
    RD_SET_CLEAR (pce->n_replace);
  }

  PF_FOREACH_CB (pf_cb, pred_flow->list_pf_cb)
  {
    PF_FOREACH_BB (pf_bb, pf_cb->pf_bbs)
    {
      pce = pf_bb->info->pce_info;
      RD_SET_CLEAR (pce->xu_safe);
      RD_SET_CLEAR (pce->loc_delayed);
      RD_SET_CLEAR (pce->x_comp);
      RD_SET_CLEAR (pce->x_latest);
      RD_SET_CLEAR (pce->x_insert);
      RD_SET_CLEAR (pce->n_isolated);
      RD_SET_CLEAR (pce->x_isolated);
      RD_SET_CLEAR (pce->x_spec_ds);
      RD_SET_CLEAR (pce->x_spec_us);

#ifdef DEBUG_PDE_BB_FINAL_SETS
      fprintf (stderr,
	       "BB starting at op %d (including node %d) in CB %d\n",
	       ((pf_bb->first_op) ? pf_bb->first_op->id : 0),
	       (((PF_NODE *) List_first (pf_bb->pf_nodes_entry))->id),
	       pf_bb->pf_cb->cb->id);
      Set_print (stderr, "INSERT", pce->n_insert);
      Set_print (stderr, "PRED_GUARD", pce->pred_guard);
      Set_print (stderr, "DELETE", pce->x_replace);
      Set_print (stderr, "PRED_SET", pce->pred_set);
      Set_print (stderr, "PRED_CLEAR", pce->pred_clear);
#endif
    }
  }
}
