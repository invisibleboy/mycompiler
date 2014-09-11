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
 *
 *  File:  limpact_main.h
 *
 *  Description:
 *    Driver module include files for Limpact code generation.
 *
 *  Creation Date :  June 1993
 *
 *  Author:  Scott A. Mahlke, Roger A. Bringmann, Richard E. Hank, 
 *           John C. Gyllenhaal, Wen-mei Hwu
 *
\*****************************************************************************/
#ifndef LIMPACT_MAIN_H
#define LIMPACT_MAIN_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include "limpact_phase1.h"
#include "limpact_phase2.h"
#include "limpact_phase3.h"
#include <Lcode/r_regalloc.h>
#include <Lcode/sm.h>
#include <Lcode/l_loop_prep.h>
#include <Lcode/l_softpipe.h>
#include <Lcode/m_opti.h>

#include <machine/m_impact.h>

#ifdef __cplusplus
extern "C"
{
#endif

  extern char CurrentFunction[];

  extern FILE *prepass_out_file;  /* used if Limpact_print_prepass_schedule is
				     set */

/*
 *      Declare code generator specific parameter variables
 */
  extern int L_debug_messages;
  extern int L_do_machine_opt;
  extern int L_do_prepass_sched;
  extern int L_do_register_allocation;
  extern int L_do_postpass_code_annotation;
  extern int L_do_peephole_opt;
  extern int L_do_fill_squashing_branches;
  extern int L_do_fill_non_squashing_branches;
  extern int L_do_fill_unfilled_branches;

/*
 *      Limpact specific parameter variables
 */
  extern int Limpact_predicated_lcode;
  extern int Limpact_num_int_caller_reg;
  extern int Limpact_num_int_callee_reg;
  extern int Limpact_num_flt_caller_reg;
  extern int Limpact_num_flt_callee_reg;
  extern int Limpact_num_dbl_caller_reg;
  extern int Limpact_num_dbl_callee_reg;
  extern int Limpact_num_prd_caller_reg;
  extern int Limpact_num_prd_callee_reg;
  extern int Limpact_loads_per_cycle;
  extern int Limpact_stores_per_cycle;
  extern int Limpact_do_hb_spill_opti;
  extern int Limpact_print_hb_spill_opti_stats;
  extern int Limpact_print_prepass_schedule;
  extern int Limpact_annotate_varargs;
  extern int Limpact_adjust_memory_stack;
  extern int Limpact_debug_stack_frame;
  extern int Limpact_do_branch_split;
  extern int Limpact_convert_init_pred_to_uncond_defs;
  extern int Limpact_return_address_in_reg;
/*
 *      Function prototypes
 */
  void L_read_parm_limpact (Parm_Parse_Info * ppi);
  void L_gen_code (Parm_Macro_List * command_line_macro_list);

  void L_mark_static_branch_pred (L_Func * fn, char *bp_model_name);

#ifdef __cplusplus
}
#endif

#endif
