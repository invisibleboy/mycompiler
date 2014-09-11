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
 *      File :          l_PCE_opti.h
 *      Description :   partial code elimination declarations
 *      Creation Date : September, 2002
 *      Author :        Shane Ryoo, Wen-mei Hwu.
 *==========================================================================*/

#ifndef L_PCE_OPTI_H
#define L_PCE_OPTI_H

#include <config.h>

/*
 *      External functions
 */

extern int L_split_fn_critical_edges (L_Func *);
extern int L_split_fn_loop_out_critical_edges (L_Func *);
extern int L_split_fn_loopback_critical_edges (L_Func *);

extern int L_PRE_lazy_code_motion (L_Func *);
extern int L_partial_redundancy_elimination (L_Func *);
extern int L_speculative_PRE (L_Func *);

extern int L_PDE_code_motion (L_Func *);
extern int L_partial_dead_code_elimination (L_Func *);
extern int L_PDE_combine_pred_guards (L_Func *);
extern int L_min_cut_PDE (L_Func *);

extern int L_coalesce_cbs (L_Func *, int);
extern void L_PCE_disable_subsumed_optis ();
extern void L_PCE_fix_function_weight ();
extern int L_PCE_merge_same_cbs (L_Func *);
extern int L_PCE_cleanup (L_Func *, int);

#endif
