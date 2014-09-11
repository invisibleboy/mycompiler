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
/******************************************************************************\
 *
 *  File:  limpact_phase2.h
 *
 *  Description:  Header file for phase2 of IMPACT code generator
 *
 *  Creation Date :  June 1993
 *
 *  Author:  Scott A. Mahlke, Roger A. Bringmann, Richard Hank, 
 *           John C. Gyllenhaal, Wen-mei Hwu
 *
\******************************************************************************/
#ifndef LIMPACT_PHASE2_H
#define LIMPACT_PHASE2_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#ifdef __cplusplus
extern "C"
{
#endif

  extern int caller_reg_map[];
  extern int callee_reg_map[];

/*
 *      limpact_phase2_func.c prototypes
 */
  void O_perform_init (L_Func * fn);
  void O_process_func (L_Func * fn,
                       Parm_Macro_List * command_line_macro_list);
  void O_init (Parm_Macro_List * command_line_macro_list);
  void O_cleanup ();
  void O_mask_promoted_pei (L_Func * fn);

/*
 *      limpact_phase2_opti.c prototypes
 */
  void O_insert_pred_block_ops (L_Func * fn);

/*
 *      limpact_phase2_reg.c prototypes
 */
  double R_callee_cost (int ctype, int leaf, int callee_allocated);
  double R_caller_cost (int ctype, int leaf);
  double R_spill_load_cost (int ctype);
  double R_spill_store_cost (int ctype);
  L_Oper *O_fill_reg (int reg, int type, L_Operand * operand,
                      int fill_offset, L_Operand ** pred, int type_flag);
  L_Oper *O_spill_reg (int reg, int type, L_Operand * operand,
                       int spill_offset, L_Operand ** pred, int type_flag);
  L_Oper *O_jump_oper (int opc, L_Cb * dest_cb);
  void O_register_init (void);
  void O_register_allocation (L_Func * fn,
                              Parm_Macro_List * command_line_macro_list,
			      int *spill_space);

/*
 *      limpact_phase2_memstack.c prototypes
 */
  void O_postpass_adjust_memory_stack (L_Func *fn, int swap_space_size);


#ifdef __cplusplus
}
#endif

#endif
