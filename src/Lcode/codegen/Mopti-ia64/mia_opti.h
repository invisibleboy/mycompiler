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
/*****************************************************************************
 * mia_opti.h                                                                *
 * ------------------------------------------------------------------------- *
 *  Description:                                                             *
 *      Performs machine level code optimization:                            *
 *	1) common subexpression ellimination                                 *
 *	2) limited copy propogation (only R-R, R-M, M-R, M-M)                *
 *	3) dead code removal (unused operations, src1=dest)                  *
 *                                                                           *
 *	This code is base-lined off the work developed by Scott Mahlke in    *
 *	l_basic_opti.c                                                       *
 *                                                                           *
 * AUTHORS: R.A. Bringmann                                                   *
 *****************************************************************************/
/* 09/17/02 REK Adding declaration for M_local_constant_folding. */

#ifndef _MIA_OPTI_H
#define _MIA_OPTI_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#ifdef __cplusplus
extern "C"
{
#endif

  extern int Mopti_constant_preloading;
  extern int Mopti_shift_add_merge;
  extern int Mopti_do_epilogue_merge;

  extern int Mopti_print_stats;

  extern int Mopti_do_predicate_opti;
  extern int Mopti_do_opp_cond_combining;
  extern int Mopti_do_and_cmp_promotion;
  extern int Mopti_do_DeMorgan_combining;
  extern int Mopti_do_redundant_compare_removal;
  extern int Mopti_do_remove_decidable_compares;
  extern int Mopti_do_pred_init_combining;
  extern int Mopti_do_dead_pred_def_removal;
  extern int Mopti_do_pred_copy_removal;
  extern int Mopti_do_sp_removal;
  extern int Mopti2_redundant_memory_ops;
  extern int Mopti_debug_messages;

  /* reductions */

  extern void L_read_parm_mopti (Parm_Parse_Info * ppi);
  extern void Mopti_perform_optimizations_tahoe (L_Func * fn);
  void Mopti_phase2_optimizations (L_Func * fn);

  extern int L_no_other_use_in_cb_after (L_Cb * cb, L_Operand * operand,
					 L_Oper * after_oper,
					 L_Oper * except_oper);
  extern int L_no_other_def_use_in_cb (L_Cb * cb, L_Operand * operand,
				       L_Oper * except1, L_Oper * except2,
				       L_Oper * except3);

  extern void Mopti_convert_ldf_to_ldi (L_Func * fn);

  extern int Mopti_global_sxt_elimination (L_Func * fn);

  /* mia_epi_merge.c */

  extern void Mopti_epilogue_merge (L_Func * fn);
  extern void Mopti_epilogue_cleanup (L_Func * fn);

  /* mia_jump_opti.c */

  extern int Mopti_branch_target_expansion (L_Func * fn);

  /* mia_post_inc.c */

  extern void Mia_post_increment_conversion (L_Func * fn, int ld, int st);
  extern void Mia_softpipe_post_increment_conversion (L_Func * fn, int ld,
						      int st);

  /* mia_reductions.c */

  extern void Mopti_tahoe_reductions (L_Func * fn);

  /* mia_shladd.c */

  extern void Mopti_shladd (L_Func * fn);
  extern int M_local_shladdp4 (L_Cb * cb, int *reaching_df_done);
  extern int M_local_constant_folding (L_Cb * cb);
  extern void Mopti_constant_generation (L_Func * fn);

#ifdef __cplusplus
}
#endif

#endif
