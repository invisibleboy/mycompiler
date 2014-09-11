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
/* 9/16/02 Robert Kidd
 * This header defines the functions to print each opcode.
 * Insert legalese here.
 */

#ifndef _PHASE3_OPER_H
#define _PHASE3_OPER_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

void P_line_print (char *format, ...);
void P_line_pad (int column);
void P_set_explicit_bundling (void);
void P_set_implicit_bundling (void);
void P_reset_bundle_indx (void);
void P_open_bundle (L_Oper * oper);
void P_bundle_instr (void);
void P_print_comment (L_Oper * oper, unsigned int instr_offset,
		      unsigned int issue_cycle);
void P_print_int_reg_name (int real_reg_id);
void P_print_predicate (L_Oper * oper);
void P_print_register_operand_asm (L_Operand * operand, int real);
void P_print_macro_operand_asm (L_Oper * oper, L_Operand * operand, int real);
void P_print_immed_operand_asm (L_Oper * oper, L_Operand * operand);
void P_print_label_operand_asm (L_Oper * oper, L_Operand * operand);
void P_print_operand_asm (L_Oper * oper, L_Operand * operand);
void P_print_var_operand_asm (L_Oper * oper, L_Operand * operand, int real);
void P_convert_reg_nums_operand (L_Oper * oper, L_Operand * operand);
void P_convert_reg_nums (L_Func * fn);
void P_print_define_oper (L_Oper * oper);

void P_print_nop (L_Oper * oper);
void P_print_break (L_Oper * oper);
void P_print_add (L_Oper * oper);
void P_print_load (L_Oper * oper);
void P_print_fill_int_load (L_Oper * oper);
void P_print_lfetch (L_Oper * oper);
void P_print_check (L_Oper * oper);
void P_print_fchkf (L_Oper * oper);
void P_print_fclrf (L_Oper * oper);
void P_print_fsetc (L_Oper * oper);
void P_print_branch (L_Oper * oper);
void P_print_branch_hint (L_Oper * oper);
void P_print_mnemonic_only (L_Oper * oper);
void P_print_cmpxchg (L_Oper * oper);
void P_print_fetchadd_xchg (L_Oper * oper);
void P_print_store (L_Oper * oper);
void P_print_invala (L_Oper * oper);
void P_print_cc (L_Oper * oper);
void P_print_halt (L_Oper * oper);
void P_print_alloc (L_Oper * oper);
void P_print_fc (L_Oper * oper);
void P_print_itc (L_Oper * oper);
void P_print_itr (L_Oper * oper);
void P_print_probe_fault (L_Oper * oper);
void P_print_purge (L_Oper * oper);
void P_print_mask_op (L_Oper * oper);
void P_print_mov_from_br (L_Oper * oper);
void P_print_mov_from_ar (L_Oper * oper);
void P_print_mov_from_pr (L_Oper * oper);

int P_check_post_src_equal_dest (L_Oper * oper, L_Operand * src,
				 L_Operand * dest);
void P_print_non_instr (L_Oper * oper);
void P_fix_pred_compare_dests (L_Oper * oper);
void P_fix_pred_compare_dests_func (L_Func * fn);
void P_print_pred_deps (L_Oper * oper, int softpipe);
void P_print_oper (L_Cb * cb, L_Oper * oper, unsigned int *instr_offset,
		   unsigned int *issue_cycle);

#endif
