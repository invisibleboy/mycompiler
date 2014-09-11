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
 *  File:  phase3.h
 *
 *  Description:  Header file for phase3 of TAHOE code generator
 *
 *  Author: Dan Connors and Jim Pierce
 *
 *                       INTEL CONFIDENTIAL
 *
\*****************************************************************************/
/* 09/16/02 REK Adding a declaration for P_file_init */

#ifndef LTAHOE_PHASE3_H_
#define LTAHOE_PHASE3_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#define LOCAL_LABELS

union CONVERT
{
#if defined(X86LIN_SOURCE) || defined(WIN32) || defined(IA64LIN_SOURCE)
  struct
  {
    int lo;
    int hi;
  }
  integer;
#elif defined(_SOLARIS_SOURCE) || defined(_HPUX_SOURCE)
  struct
  {
    int hi;
    int lo;
  }
  integer;
#else
#error Unsupported host platform
#endif
  float sgl;
  double dbl;
  ITint64 q;
};

/* phase3_func.c */

extern int num_reg_stack_inputs;
extern int num_reg_stack_locals;
extern int num_reg_stack_outputs;
extern int num_reg_stack_rots;

extern void P_file_init (void);
extern void P_file_end (void);
extern void P_end (void);
extern void P_init (Parm_Macro_List * command_line_macro_list);
extern void P_process_func (L_Func * fn);
extern void P_convert_reg_nums (L_Func * fn);

/* phase3_oper.c */

extern void P_print_oper (L_Cb * cb, L_Oper * oper,
			  unsigned int *instr_offset,
			  unsigned int *issue_cycle);
extern void P_set_explicit_bundling (void);
extern void P_set_implicit_bundling (void);
extern void P_reset_bundle_indx (void);
extern void P_convert_reg_nums (L_Func * fn);
extern void P_fix_pred_compare_dests_func (L_Func * fn);

/* phase3_data.c */

extern void P_process_data (FILE * F_OUT, L_Data * data);
extern void P_print_standard_file_header (void);
extern void P_print_section_title (int section_type);

/* phase3_symbol.c */

extern void P_symtab_add_label (char *label, int isfunc);
extern void P_symtab_add_def (char *label, int isfunc);
extern void P_symtab_init (void);
extern void P_symtab_deinit (void);
extern void P_symtab_print_extern (void);

/* phase3_load.c */
FILE* LD_TABLE_OUT;
extern void P_print_load_table (L_Oper * oper, L_Cb * cb, 
				unsigned int instr_offset, int slot_no);

#endif





