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
 *      File :          l_loop_classic.h
 *      Description :   Classical loop opti external decls
 *      Author :        Scott Mahlke
 *
 *      (C) Copyright 1990, Scott Mahlke
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_LOOP_CLASSIC_H
#define L_LOOP_CLASSIC_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

extern int Lsuper_invariant_operand (L_Cb *, L_Operand *);
extern void Lsuper_find_all_ind_info (L_Inner_Loop * loop, L_Cb * cb);
extern void L_duplicate_loop_body (L_Inner_Loop *);
extern int L_additional_loop_opts_to_apply (L_Inner_Loop *);
extern int L_sb_loop_inv_code_rem (L_Inner_Loop *, int);
extern int L_sb_loop_global_var_mig (L_Inner_Loop *, int);
extern int L_sb_loop_copy_propagation (L_Inner_Loop *, int);
extern int L_sb_loop_operation_folding (L_Inner_Loop *, int);
extern int L_sb_loop_dead_code (L_Inner_Loop *, int);
extern int L_sb_loop_ind_var_elim (L_Inner_Loop *, int);
extern int L_sb_loop_ind_var_elim2 (L_Inner_Loop *, int);
extern int L_sb_loop_ind_var_reinit (L_Inner_Loop *, int);
extern int L_sb_loop_post_inc_conversion (L_Inner_Loop *, int);
#endif
