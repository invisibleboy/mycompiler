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
 *      Author: Shane Ryoo, Wen-mei Hwu
 *      Creation Date:  June 2003
\*****************************************************************************/
#ifndef R_PCE_FLOW_H
#define R_PCE_FLOW_H

#include <config.h>

#ifdef __cplusplus
extern "C"
{
#endif

  /*
   * r_dataflow.c
   * ----------------------------------------------------------------------
   */

  extern void D_pce_flow_analysis (PRED_FLOW * pred_flow, int mode);

  /*
   * r_df_query.c
   * ----------------------------------------------------------------------
   */

  /* PARTIAL CODE ELIMINATION */

  extern int D_PCE_cb_insert_size (PRED_FLOW * pred_flow, L_Cb * cb);
  extern int D_PCE_cb_replace_size (PRED_FLOW * pred_flow, L_Cb * cb);
  extern int D_PRE_cb_no_changes (PRED_FLOW * pred_flow, L_Cb * cb, 
                                  Set ignore_set);
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

  extern int D_PCE_cb_no_insertions (PRED_FLOW * pred_flow, L_Cb * cb);
  extern PF_BB * D_find_pf_bb (PRED_FLOW * pred_flow, L_Cb * cb,
			       L_Oper *first_op);

#ifdef __cplusplus
}
#endif

#endif
