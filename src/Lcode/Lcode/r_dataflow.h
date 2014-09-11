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
/*****************************************************************************\
 *      File:   dataflow.h
 *      Author: David I. August, Wen-mei Hwu
 *      Creation Date:  September 1996
\*****************************************************************************/
#ifndef R_DATAFLOW_H
#define R_DATAFLOW_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/* Memory analysis flags */
#define MDF_RET_AMB     0x00000001
#define MDF_RET_DEP     0x00000002
#define MDF_RET_AD      0x00000003
#define MDF_RET_LOADS   0x00000004
#define MDF_RET_STORES  0x00000008
#define MDF_RET_LS      0x0000000C
#define MDF_RET_JSRS    0x00000010
#define MDF_RET_LSJ     0x0000001C

/* Memory allocation pools */
extern L_Alloc_Pool *D_alloc_df_pce_info;
extern L_Alloc_Pool *D_alloc_df_inst_info;
extern L_Alloc_Pool *D_alloc_df_oper_info;
extern L_Alloc_Pool *D_alloc_df_node_info;
extern L_Alloc_Pool *D_alloc_df_bb_info;
extern L_Alloc_Pool *D_alloc_df_cb_info;

/* Predefined Flow Graphs */
extern PRED_FLOW *PF_bb_flow;
extern PRED_FLOW *PF_pred_flow;
extern PRED_FLOW *PF_default_flow;

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   * r_dataflow.c
   * ----------------------------------------------------------------------
   */

  extern void D_delete_dataflow (L_Func * fn);
  extern void D_setup_dataflow (L_Func * fn, int do_operands);
  extern void D_setup_BB_lists (PRED_FLOW * pred_flow);

  /* partial dead code removal */

  extern void D_partial_dead_code_removal (L_Func *fn);
  extern int D_delete_DF_dead_code (L_Func * fn);
  extern void D_clear_partial_dead_code_markings (L_Func *fn);

  extern int D_dataflow_valid (void);
  extern void D_invalidate_dataflow (void);

  extern void D_dataflow_analysis (PRED_FLOW * pred_flow, int mode);

  extern void D_add_src_operand (PRED_FLOW * pred_flow, L_Oper * oper,
				 int reg, int transparent, int ptype);
  extern void D_add_dest_operand (PRED_FLOW * pred_flow, L_Oper * oper,
				  int reg, int transparent, int ptype);
  extern void D_unmap_rdid (PRED_FLOW * pred_flow, int rdid, int *oper_id,
			    int *operand_id);

/* Dan Lavery's enhancement.  ITI/MCM 8/17/99 */

  extern void D_remove_from_oper_RIN_set (PRED_FLOW * pred_flow,
					  L_Oper * oper,
					  L_Oper * reaching_oper,
					  int operand_id);

  extern void D_remove_from_oper_EIN_set (PRED_FLOW * pred_flw, L_Oper * oper,
					  L_Oper * reach_oper);

  extern void D_remove_from_cb_EIN_set (PRED_FLOW * pred_flow, L_Cb * cb,
					L_Oper * reach_oper);

  /*
   * r_df_query.c
   * ----------------------------------------------------------------------
   */

  /* LIVE VARIABLE */

  extern int D_in_cb_IN_set (PRED_FLOW * pred_flow, L_Cb * cb, 
			     int operand_id);
  extern int D_in_cb_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb, 
			      int operand_id);

  extern Set D_get_cb_IN_set (PRED_FLOW * pred_flow, L_Cb * cb);
  extern Set D_get_cb_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb);

  extern void D_add_to_cb_IN_set (PRED_FLOW * pred_flow, L_Cb * cb, 
				  int operand_id);

  extern int D_in_oper_IN_set (PRED_FLOW * pred_flow, L_Oper * oper,
			       int operand_id);
  extern int D_in_oper_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb,
				L_Oper * oper, int operand_id, int path);

  extern Set D_get_oper_IN_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_oper_OUT_set (PRED_FLOW * pred_flow, L_Cb * cb,
				 L_Oper * oper, int path);

  /* REACHING DEFINITION */

  extern int D_in_cb_RIN_set (PRED_FLOW * pred_flow, L_Cb * cb,
			      L_Oper *reaching_oper, int operand_id);

  extern int D_in_cb_RIN_set_reg (PRED_FLOW * pred_flow, L_Cb * cb,
				  L_Oper *reaching_oper, int reg);

  extern int D_in_oper_RIN_set (PRED_FLOW * pred_flow, L_Oper * oper,
				L_Oper * reaching_oper, int operand_id);
  extern int D_in_oper_ROUT_set (PRED_FLOW * pred_flow, L_Oper * oper,
				 L_Oper * reaching_oper, int operand_id,
				 int path);

  extern Set D_get_cb_RIN_set (PRED_FLOW *pred_flow, L_Cb *cb);
  extern Set D_get_oper_RIN_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_oper_ROUT_set (PRED_FLOW * pred_flow, L_Oper * oper);

  extern Set D_get_oper_RIN_defining_opers (PRED_FLOW * pred_flow,
					    L_Oper * oper, int operand_id);
  extern Set D_get_cb_RIN_defining_opers (PRED_FLOW *pred_flow, L_Cb *cb,
					  int operand_id);
  extern Set D_get_oper_ROUT_using_opers (PRED_FLOW * pred_flow,
					  L_Oper * oper, L_Operand * operand,
					  int operand_id);

  /* AVAILABLE DEFINITION */

  extern int D_in_oper_AIN_set (PRED_FLOW * pred_flow, L_Oper * oper,
				L_Oper * reaching_oper, int operand_id);
  extern int D_in_oper_AOUT_set (PRED_FLOW * pred_flow, L_Oper * oper,
				 L_Oper * reaching_oper, int operand_id,
				 int path);

  extern Set D_get_oper_AIN_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_oper_AOUT_set (PRED_FLOW * pred_flow, L_Oper * oper);

  extern Set D_get_oper_AIN_defining_opers (PRED_FLOW * pred_flow,
					    L_Oper * oper, int operand_id);


  /* AVAILABLE EXPRESSION */

  extern int D_in_cb_EIN_set (PRED_FLOW * pred_flow, L_Cb * cb,
			      L_Oper * reach_oper);
  extern int D_in_cb_EOUT_set (PRED_FLOW * pred_flow, L_Cb * cb,
			       L_Oper * reach_oper);

  extern Set D_get_cb_EIN_set (PRED_FLOW * pred_flow, L_Cb * cb);
  extern Set D_get_cb_EOUT_set (PRED_FLOW * pred_flow, L_Cb * cb);

  extern int D_in_oper_EIN_set (PRED_FLOW * pred_flow, L_Oper * oper,
				L_Oper * reach_oper);
  extern int D_in_oper_EOUT_set (PRED_FLOW * pred_flow, L_Oper * oper,
				 L_Oper * reach_oper);

  extern Set D_get_oper_EIN_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_oper_EOUT_set (PRED_FLOW * pred_flow, L_Oper * oper);

  /* MEMORY REACHING DEFINITION */

  extern Set D_get_mem_oper_RIN_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_mem_oper_ROUT_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_mem_oper_RIN_set_rid (PRED_FLOW * pred_flow,
					 L_Oper * oper);
  extern Set D_get_mem_oper_ROUT_set_rid (PRED_FLOW * pred_flow,
					  L_Oper * oper);
  extern Set D_get_mem_oper_RIN_defining_opers (PRED_FLOW * pred_flow,
						L_Oper * oper, int flags);
  extern Set D_get_mem_oper_ROUT_using_opers (PRED_FLOW * pred_flow,
					      L_Oper * oper, int flags);

  /* MEMORY AVAILABLE DEFINITION */

  extern Set D_get_mem_oper_AIN_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_mem_oper_AOUT_set (PRED_FLOW * pred_flow, L_Oper * oper);
  extern Set D_get_mem_oper_AIN_set_rid (PRED_FLOW * pred_flow,
					 L_Oper * oper);
  extern Set D_get_mem_oper_AOUT_set_rid (PRED_FLOW * pred_flow,
					  L_Oper * oper);
  extern Set D_get_mem_oper_AIN_defining_opers (PRED_FLOW * pred_flow,
						L_Oper * oper, int flags);

  /* PARTIAL CODE ELIMINATION */

  extern void D_PRE_speculative_cut_analysis (PRED_FLOW * pred_flow, int mode,
					      int ignore);
  extern void D_PDE_predicated_cut_analysis (PRED_FLOW * pred_flow, int mode,
					     int ignore);

  extern int D_PRE_cb_no_changes (PRED_FLOW * pred_flow, L_Cb * cb, 
                                  Set ignore_set);
  extern int D_PDE_cb_no_changes (PRED_FLOW * pred_flow, L_Cb * cb);
  extern DF_PCE_INFO * D_get_PCE_bb_info (PRED_FLOW * pred_flow, L_Cb *cb,
					  L_Oper * first_op);
  extern Set D_get_PCE_bb_complement_set (PRED_FLOW * pred_flow, L_Cb *cb,
					  L_Oper *first_op);
  extern Set D_get_PCE_bb_nd_safe_set (PRED_FLOW * pred_flow, L_Cb *cb, 
				       L_Oper *first_op);
  extern Set D_get_PCE_bb_xd_safe_set (PRED_FLOW * pred_flow, L_Cb *cb, 
				       L_Oper *last_op);
  extern Set D_get_PCE_bb_n_insert_set (PRED_FLOW * pred_flow, L_Cb *cb, 
					L_Oper *first_op);
  extern Set D_get_PCE_bb_x_insert_set (PRED_FLOW * pred_flow, L_Cb *cb, 
					L_Oper *last_op);
  extern Set D_get_PCE_bb_n_replace_set (PRED_FLOW * pred_flow, L_Cb *cb, 
					L_Oper *first_op);
  extern Set D_get_PCE_bb_x_replace_set (PRED_FLOW * pred_flow, L_Cb *cb, 
					L_Oper *last_op);
  extern Set D_get_PCE_cb_mem_reaching_definitions (PRED_FLOW * pred_flow,
                                                    L_Cb * cb);

  extern PF_BB * D_find_pf_bb (PRED_FLOW * pred_flow, L_Cb * cb,
			       L_Oper *first_op);

#ifdef __cplusplus
}
#endif

#endif
