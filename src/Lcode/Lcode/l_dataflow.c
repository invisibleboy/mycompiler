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
 *      File :          l_dataflow.c
 *      Description :   data flow functions
 *      Creation Date : February 1993
 *      Author :        Scott Mahlke, David August, Wen-mei Hwu
 *
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#undef PRINT_FLOW_ANALYSIS_CALLS


void
L_do_flow_analysis (L_Func * fn, int mode)
{
  int dom_mode;
  int dead_code;

#ifdef PRINT_FLOW_ANALYSIS_CALLS
  fprintf (stderr, "L_do_flow_analysis...");
#endif

  dom_mode = mode & (DOMINATOR | POST_DOMINATOR |
		     DOMINATOR_CB | POST_DOMINATOR_CB |
		     DOMINATOR_INT | POST_DOMINATOR_INT);
  if (dom_mode)
    L_dominator_analysis (fn, mode);

  mode = mode & ~dom_mode;

  if (!mode)
    return;

  do
    {
      L_start_time (&L_module_global_dataflow_time);

      if (!(mode & SUPPRESS_PG))
	D_setup_dataflow (fn, PF_ALL_OPERANDS);
      else
	D_setup_dataflow (fn, PF_ALL_OPERANDS|PF_SUPPRESS_PRED_GRAPH);

      if (mode & PCE)
	D_setup_BB_lists (PF_default_flow);

      D_dataflow_analysis (PF_default_flow, mode);

      L_stop_time (&L_module_global_dataflow_time);

      dead_code = D_delete_DF_dead_code (fn);
    }
  while (dead_code);
}


void
L_update_flow_analysis (L_Func * fn, int mode)
{
  if (!D_dataflow_valid ())
    L_do_flow_analysis (fn, mode);
}


void
L_invalidate_dataflow (void)
{
  D_invalidate_dataflow ();
}


void
L_do_pred_flow_analysis (L_Func * fn, int mode)
{
  int dom_mode;

  dom_mode = mode & (DOMINATOR | POST_DOMINATOR |
		     DOMINATOR_CB | POST_DOMINATOR_CB |
		     DOMINATOR_INT | POST_DOMINATOR_INT);
  if (dom_mode)
    L_dominator_analysis (fn, mode);

  mode = mode & ~dom_mode;

  if (!mode)
    return;

  D_setup_dataflow (fn, PF_ALL_OPERANDS);

  D_dataflow_analysis (PF_default_flow, mode);

}


void
L_dataflow_analysis (int mode)
{
  int dom_mode;

  dom_mode = mode & (DOMINATOR | POST_DOMINATOR |
		     DOMINATOR_CB | POST_DOMINATOR_CB |
		     DOMINATOR_INT | POST_DOMINATOR_INT);
  if (dom_mode)
    L_dominator_analysis (L_fn, mode);

  mode = mode & ~dom_mode;

  if (!mode)
    return;

  mode = mode & ~dom_mode;

  D_dataflow_analysis (PF_default_flow, mode);
}


void
L_setup_dataflow (L_Func * fn)
{
  D_setup_dataflow (fn, PF_ALL_OPERANDS);
}


void
L_setup_dataflow_no_operands (L_Func * fn)
{
  D_setup_dataflow (fn, PF_NO_OPERANDS);
}


void
L_delete_dataflow (L_Func * fn)
{
  D_delete_dataflow (fn);
}


void
L_add_src_operand_reg (L_Oper * oper, int reg,
		       int transparent, int unconditional)
{
  D_add_src_operand (PF_default_flow, oper, reg, transparent, unconditional);
}


void
L_add_dest_operand_reg (L_Oper * oper, int reg,
			int transparent, int unconditional)
{
  D_add_dest_operand (PF_default_flow, oper, reg, transparent, unconditional);
}


/*
 * L_partial_dead_code_removal (JWS after DIA)
 * ----------------------------------------------------------------------
 * Removes dead code and updates pred[1] fields to indicate strongest
 * viable predicate
 * N.B.: * Rebuilds pred graphs
 *       * Rebuilds pred flow graph
 *       * Uses CRITICAL_VARIABLE analysis, which trumps LIVE_VARIABLE
 */
int
L_partial_dead_code_removal (L_Func * fn)
{
  int count;
  PG_setup_pred_graph (fn);

  L_start_time (&L_module_global_dataflow_time);

  PG_pred_dead_code_removal (fn);
  if (PG_pred_graph && PG_pred_graph->unreachable_code)
    {
      if (L_debug_df_dead_code)
	L_warn ("L_partial_dead_code_removal removing unreachable blocks");
      L_delete_unreachable_blocks (fn);
    }

  D_partial_dead_code_removal(fn);
  count = D_delete_DF_dead_code (fn);
  L_stop_time (&L_module_global_dataflow_time);
  return count;
}


void
L_clear_partial_dead_code_markings (L_Func * fn)
{
  D_clear_partial_dead_code_markings (fn);
}


void
L_demote_branches (L_Func * fn)
{
  if (L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_HYPERBLOCK))
    PF_demote_branches (fn);
  return;
}


/*
 * LIVE VARIABLE QUERIES
 * ----------------------------------------------------------------------
 */


int
L_in_cb_IN_set (L_Cb * cb, L_Operand * operand)
{
  if (operand == NULL)
    return 0;
  return D_in_cb_IN_set (PF_default_flow, cb, L_REG_MAC_INDEX (operand));
}


int
L_in_cb_OUT_set (L_Cb * cb, L_Operand * operand)
{
  if (operand == NULL)
    return 0;
  return D_in_cb_OUT_set (PF_default_flow, cb, L_REG_MAC_INDEX (operand));
}


int
L_in_cb_IN_set_reg (L_Cb * cb, int reg)
{
  return D_in_cb_IN_set (PF_default_flow, cb, reg);
}


int
L_in_cb_OUT_set_reg (L_Cb * cb, int reg)
{
  return D_in_cb_OUT_set (PF_default_flow, cb, reg);
}


Set 
L_get_cb_IN_set (L_Cb * cb)
{
  return (D_get_cb_IN_set (PF_default_flow, cb));
}

void
L_add_to_cb_IN_set (L_Cb *cb, int reg)
{
  D_add_to_cb_IN_set (PF_default_flow, cb, reg);
  return;
}

Set 
L_get_cb_OUT_set (L_Cb * cb)
{
  return (D_get_cb_OUT_set (PF_default_flow, cb));
}


int
L_in_oper_IN_set (L_Oper * oper, L_Operand * operand)
{
  if (!operand)
    return 0;
  return D_in_oper_IN_set (PF_default_flow, oper, L_REG_MAC_INDEX (operand));
}


int
L_in_oper_OUT_set (L_Cb * cb, L_Oper * oper, L_Operand * operand, int path)
{
  if (!operand)
    return 0;
  return D_in_oper_OUT_set (PF_default_flow, cb, oper,
			    L_REG_MAC_INDEX (operand), path);
}


int
L_in_oper_IN_set_reg (L_Oper * oper, int reg)
{
  return (D_in_oper_IN_set (PF_default_flow, oper, reg));
}


int
L_in_oper_OUT_set_reg (L_Cb * cb, L_Oper * oper, int reg, int path)
{
  return (D_in_oper_OUT_set (PF_default_flow, cb, oper, reg, path));
}


Set 
L_get_oper_IN_set (L_Oper * oper)
{
  return (D_get_oper_IN_set (PF_default_flow, oper));
}


Set 
L_get_oper_OUT_set (L_Cb * cb, L_Oper * oper, int path)
{
  return (D_get_oper_OUT_set (PF_default_flow, cb, oper, path));
}


/*
 * REACHING DEFINITION QUERIES
 * ----------------------------------------------------------------------
 */


Set 
L_get_oper_RIN_set (L_Oper * oper)
{
  return (D_get_oper_RIN_set (PF_default_flow, oper));
}


Set 
L_get_oper_ROUT_set (L_Oper * oper)
{
  return (D_get_oper_ROUT_set (PF_default_flow, oper));
}

Set L_get_mem_oper_RIN_set (L_Oper * oper)
{
  return (D_get_mem_oper_RIN_set (PF_default_flow, oper));
}


Set 
L_get_mem_oper_ROUT_set (L_Oper * oper)
{
  return (D_get_mem_oper_ROUT_set (PF_default_flow, oper));
}


Set 
L_get_mem_oper_RIN_set_rid (L_Oper * oper)
{
  return (D_get_mem_oper_RIN_set_rid (PF_default_flow, oper));
}


Set 
L_get_mem_oper_ROUT_set_rid (L_Oper * oper)
{
  return (D_get_mem_oper_ROUT_set_rid (PF_default_flow, oper));
}


int
L_in_oper_RIN_set (L_Oper * oper, L_Oper * reaching_oper, L_Operand * operand)
{
  if (!operand)
    return 0;
  return D_in_oper_RIN_set (PF_default_flow, oper, reaching_oper,
			    L_REG_MAC_INDEX (operand));
}


int
L_in_oper_RIN_set_reg (L_Oper * oper, L_Oper * reaching_oper, int reg)
{
  return D_in_oper_RIN_set (PF_default_flow, oper, reaching_oper, reg);
}


int
L_in_oper_ROUT_set (L_Oper * oper, L_Oper * reaching_oper,
		    L_Operand * operand, int path)
{
  if (!operand)
    return 0;
  return D_in_oper_ROUT_set (PF_default_flow, oper, reaching_oper,
			     L_REG_MAC_INDEX (operand), path);
}


int
L_in_oper_ROUT_set_reg (L_Oper * oper, L_Oper * reaching_oper,
			int reg, int path)
{
  return D_in_oper_ROUT_set (PF_default_flow, oper, reaching_oper,
			     reg, path);
}


Set 
L_get_cb_RIN_set (L_Cb * cb)
{
  return (D_get_cb_RIN_set (PF_default_flow, cb));
}


int
L_in_cb_RIN_set (L_Cb * cb, L_Oper * reaching_oper, L_Operand * operand)
{
  return (D_in_cb_RIN_set (PF_default_flow, cb, reaching_oper, 
			   L_REG_MAC_INDEX (operand)));
}


int
L_in_cb_RIN_set_reg (L_Cb * cb, L_Oper * reaching_oper, int reg)
{
  return (D_in_cb_RIN_set (PF_default_flow, cb, reaching_oper, reg));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_cb_RIN_defining_opers (L_Cb * cb, L_Operand * operand)
{
  return D_get_cb_RIN_defining_opers (PF_default_flow, cb,
				      L_REG_MAC_INDEX (operand));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_oper_RIN_defining_opers (L_Oper * oper, L_Operand * operand)
{
  return D_get_oper_RIN_defining_opers (PF_default_flow, oper,
					L_REG_MAC_INDEX (operand));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_cb_ROUT_using_opers (L_Cb * cb, L_Operand * operand)
{
  return D_get_oper_ROUT_using_opers (PF_default_flow, cb->first_op,
				      operand, L_REG_MAC_INDEX (operand));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_oper_ROUT_using_opers (L_Oper * oper, L_Operand * operand)
{
  return D_get_oper_ROUT_using_opers (PF_default_flow, oper, operand,
				      L_REG_MAC_INDEX (operand));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_mem_cb_RIN_defining_opers (L_Cb * cb, int flags)
{
  return D_get_mem_oper_RIN_defining_opers (PF_default_flow, cb->first_op,
					    flags);
}


/* Warning this creates a set, caller must free it */
Set 
L_get_mem_oper_RIN_defining_opers (L_Oper * oper, int flags)
{
  return D_get_mem_oper_RIN_defining_opers (PF_default_flow, oper, flags);
}


/* Warning this creates a set, caller must free it */
Set 
L_get_mem_cb_ROUT_using_opers (L_Cb * cb, int flags)
{
  return D_get_mem_oper_ROUT_using_opers (PF_default_flow, cb->first_op,
					  flags);
}


/* Warning this creates a set, caller must free it */
Set 
L_get_mem_oper_ROUT_using_opers (L_Oper * oper, int flags)
{
  return D_get_mem_oper_ROUT_using_opers (PF_default_flow, oper, flags);
}


/*
 * AVAILABLE DEFINITION QUERIES
 * ----------------------------------------------------------------------
 */


Set 
L_get_oper_AIN_set (L_Oper * oper)
{
  return (D_get_oper_AIN_set (PF_default_flow, oper));
}


Set 
L_get_oper_AOUT_set (L_Oper * oper)
{
  return (D_get_oper_AOUT_set (PF_default_flow, oper));
}


Set 
L_get_mem_oper_AIN_set (L_Oper * oper)
{
  return (D_get_mem_oper_AIN_set (PF_default_flow, oper));
}


Set 
L_get_mem_oper_AOUT_set (L_Oper * oper)
{
  return (D_get_mem_oper_AOUT_set (PF_default_flow, oper));
}


Set 
L_get_mem_oper_AIN_set_rid (L_Oper * oper)
{
  return (D_get_mem_oper_AIN_set_rid (PF_default_flow, oper));
}


Set 
L_get_mem_oper_AOUT_set_rid (L_Oper * oper)
{
  return (D_get_mem_oper_AOUT_set_rid (PF_default_flow, oper));
}


int
L_in_oper_AIN_set (L_Oper * oper, L_Oper * reaching_oper, L_Operand * operand)
{
  if (!operand)
    return 0;
  return D_in_oper_AIN_set (PF_default_flow, oper, reaching_oper,
			    L_REG_MAC_INDEX (operand));
}


int
L_in_oper_AOUT_set (L_Oper * oper, L_Oper * reaching_oper,
		    L_Operand * operand, int path)
{
  if (!operand)
    return 0;
  return D_in_oper_AOUT_set (PF_default_flow, oper, reaching_oper,
			     L_REG_MAC_INDEX (operand), path);
}


int
L_in_cb_AIN_set (L_Cb * cb, L_Oper * reaching_oper, L_Operand * operand)
{
  return (L_in_oper_AIN_set (cb->first_op, reaching_oper, operand));
}


Set 
L_get_cb_AIN_set (L_Cb * cb)
{
  return (L_get_oper_AIN_set (cb->first_op));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_oper_AIN_defining_opers (L_Oper * oper, L_Operand * operand)
{
  return D_get_oper_AIN_defining_opers (PF_default_flow, oper,
					L_REG_MAC_INDEX (operand));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_cb_AIN_defining_opers (L_Cb * cb, L_Operand * operand)
{
  return D_get_oper_AIN_defining_opers (PF_default_flow, cb->first_op,
					L_REG_MAC_INDEX (operand));
}


/* Warning this creates a set, caller must free it */
Set 
L_get_mem_oper_AIN_defining_opers (L_Oper * oper, int flags)
{
  return D_get_mem_oper_AIN_defining_opers (PF_default_flow, oper, flags);
}


/* Warning this creates a set, caller must free it */
Set 
L_get_mem_cb_AIN_defining_opers (L_Cb * cb, int flags)
{
  return D_get_mem_oper_AIN_defining_opers (PF_default_flow, cb->first_op,
					    flags);
}


/*
 * AVAILABLE EXPRESSION QUERIES
 * ----------------------------------------------------------------------
 */


Set 
L_get_oper_EIN_set (L_Oper * oper)
{
  return (D_get_oper_EIN_set (PF_default_flow, oper));
}


Set 
L_get_oper_EOUT_set (L_Oper * oper)
{
  return (D_get_oper_EOUT_set (PF_default_flow, oper));
}


int
L_in_oper_EIN_set (L_Oper * oper, L_Oper * reaching_oper)
{
  if (!reaching_oper)
    return 0;
  return (D_in_oper_EIN_set (PF_default_flow, oper, reaching_oper));
}


int
L_in_oper_EOUT_set (L_Oper * oper, L_Oper * reaching_oper)
{
  if (!reaching_oper)
    return 0;
  return (D_in_oper_EOUT_set (PF_default_flow, oper, reaching_oper));
}


int
L_in_cb_EIN_set (L_Cb * cb, L_Oper * reaching_oper)
{
  if (!reaching_oper)
    return 0;
  return (D_in_cb_EIN_set (PF_default_flow, cb, reaching_oper));
}


Set 
L_get_cb_EIN_set (L_Cb * cb)
{
  return (D_get_cb_EIN_set (PF_default_flow, cb));
}


void
L_remove_from_oper_EIN_set (L_Oper * oper, L_Oper * reaching_oper)
{
  D_remove_from_oper_EIN_set (PF_default_flow, oper, reaching_oper);
}


void
L_remove_from_cb_EIN_set (L_Cb * cb, L_Oper * reaching_oper)
{
  D_remove_from_cb_EIN_set (PF_default_flow, cb, reaching_oper);
}


void
L_remove_from_all_EIN_set (L_Oper * reaching_oper)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = L_fn->first_cb; cb; cb = cb->next_cb)
    {
      /* Don't update global EIN information for operations in */
      /* exit boundary cbs.  :)                          */
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_EXIT_BOUNDARY))
	continue;

      L_remove_from_cb_EIN_set (cb, reaching_oper);
      for (oper = cb->first_op; oper; oper = oper->next_op)
	L_remove_from_oper_EIN_set (oper, reaching_oper);
    }
}


/*
 * PARTIAL CODE ELIIMINATION QUERIES
 * ----------------------------------------------------------------------
 */

int
L_PRE_cb_no_changes (L_Cb * cb, Set ignore_set)
{
  return D_PRE_cb_no_changes (PF_default_flow, cb, ignore_set);
}

int
L_PDE_cb_no_changes (L_Cb * cb)
{
  return D_PDE_cb_no_changes (PF_default_flow, cb);
}

DF_PCE_INFO *
L_get_PCE_bb_info (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_info (PF_default_flow, cb, first_op);
}


Set
L_get_PCE_bb_complement_set (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_complement_set (PF_default_flow, cb, first_op);
}


Set
L_get_PCE_bb_nd_safe_set (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_nd_safe_set (PF_default_flow, cb, first_op);
}


Set
L_get_PCE_bb_xd_safe_set (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_xd_safe_set (PF_default_flow, cb, first_op);
}


Set
L_get_PCE_bb_n_insert_set (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_n_insert_set (PF_default_flow, cb, first_op);
}


Set
L_get_PCE_bb_x_insert_set (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_x_insert_set (PF_default_flow, cb, first_op);
}


Set
L_get_PCE_bb_n_replace_set (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_n_replace_set (PF_default_flow, cb, first_op);
}


Set
L_get_PCE_bb_x_replace_set (L_Cb * cb, L_Oper * first_op)
{
  return D_get_PCE_bb_x_replace_set (PF_default_flow, cb, first_op);
}


/*
 *  QUERIES
 * ----------------------------------------------------------------------
 */

/* SER: Note that although this called the same D_get function as
 * L_get_mem_oper_AOUT_set, the content of the set is VERY different:
 * it contains anticipable/dead expressions.
 */
Set 
L_get_mem_oper_overwrite_or_dead_set (L_Oper * oper)
{
  return (D_get_mem_oper_AOUT_set (PF_default_flow, oper));
}


/*
 *      The INDEX macros should be setup up to return unique id's for any
 *      operands you perform dataflow analysis on which may have overlapping
 *      values.  In the current implementation, dataflow is performed for
 *      registers and macros, each of which are numbered from 1..k, therefore
 *      the numbers are shifted left by 1 and the LSB is used to distinguish.
 *      If more types of operands are required to be analyzed by dataflow
 *      analysis, serveral of the lower bits should be used to distinguish
 *      among overlapping numbers.
 */

Set 
L_map_reg_set (Set in)
{
  int size, i, *buf;
  Set out = NULL;

  if (!(size = Set_size (in)))
    return (NULL);

  buf = (int *) alloca (sizeof (int) * size);

  for (i = 0; i < size; i++)
    out = Set_add (out, L_REG_INDEX (buf[i]));

  return (out);
}


Set 
L_map_macro_set (Set in)
{
  int i, *buf, size;
  Set out = NULL;

  if (!(size = Set_size (in)))
    return (NULL);

  buf = (int *) alloca (sizeof (int) * size);
  for (i = 0; i < size; i++)
    out = Set_add (out, L_MAC_INDEX (buf[i]));

  return (out);
}


/* Extract registers from in, return set of actual registers numbers */
Set 
L_unmap_reg_set (Set in)
{
  int i, *buf, size;
  Set out = NULL;

  if (!(size = Set_size (in)))
    return (NULL);

  buf = (int *) alloca (sizeof (int) * size);
  Set_2array (in, buf);
  for (i = 0; i < size; i++)
    if (L_IS_MAPPED_REG (buf[i]))
      out = Set_add (out, L_UNMAP_REG (buf[i]));

  return (out);
}


/* Extract macros from in, return set of actual macro numbers */
Set 
L_unmap_macro_set (Set in)
{
  int i, *buf, size;
  Set out = NULL;

  if (!(size = Set_size (in)))
    return (NULL);

  buf = (int *) alloca (sizeof (int) * size);
  Set_2array (in, buf);
  for (i = 0; i < size; i++)
    if (L_IS_MAPPED_MAC (buf[i]))
      out = Set_add (out, L_UNMAP_MAC (buf[i]));

  return (out);
}


/* Extract fragile macros from in, return set of actual macro numbers */
Set 
L_unmap_fragile_macro_set (Set in)
{
  int i, *buf, size;
  Set out = NULL;

  if (!(size = Set_size (in)))
    return (NULL);

  buf = (int *) alloca (sizeof (int) * size);
  Set_2array (in, buf);
  for (i = 0; i < size; i++)
    if (L_IS_MAPPED_MAC (buf[i]) && M_fragile_macro (buf[i]))
      out = Set_add (out, L_UNMAP_MAC (buf[i]));

  return (out);
}


void
L_unmap_rdid (int rdid, int *oper_id, int *operand_id)
{
  D_unmap_rdid (PF_default_flow, rdid, oper_id, operand_id);
}


void
L_print_dataflow (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  Set iset, ofset, otset;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      printf("(cb %d :\n", cb->id);
      iset = L_get_cb_IN_set (cb);
      Set_print (stdout, "CB LIVE IN ", iset);

      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  iset = L_get_oper_IN_set (oper);
	  Set_print (stdout, "LIVE IN ", iset);
	  L_print_oper (stdout, oper);
	  ofset = L_get_oper_OUT_set (cb, oper, FALL_THRU_PATH);
	  otset = L_get_oper_OUT_set (cb, oper, TAKEN_PATH);
	  if (!Set_same(ofset, otset))
	    {
	      Set_print (stdout, "LIVE OUT (FT) ", ofset);
	      Set_print (stdout, "LIVE OUT (TK) ", otset);
	    }
	  else
	    {
	      Set_print (stdout, "LIVE OUT  ", ofset);
	    }
	}
    }
}


/* Dan Lavery's enhancement.  ITI/MCM 8/17/99 */
/* remove specified destination (operand) of reaching_oper from reaching
   definition IN set of oper using standard index. */
void
L_remove_from_oper_RIN_set (L_Oper * oper, L_Oper * reaching_oper,
			    L_Operand * operand)
{
  if (!operand)
    L_punt ("L_remove_from_oper_RIN_set: operand is NULL\n");
  D_remove_from_oper_RIN_set (PF_default_flow, oper, reaching_oper,
			      L_REG_MAC_INDEX (operand));
}


/* remove specified destination (operand) of reaching_oper from reaching
   definition IN set of oper using raw virtual register number. */
void
L_remove_from_oper_RIN_set_reg (L_Oper * oper, L_Oper * reaching_oper,
				int reg)
{
  D_remove_from_oper_RIN_set (PF_default_flow, oper, reaching_oper, reg);
}




