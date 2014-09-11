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
 *      File :          l_superscalar.h
 *      Description :   Superscalar optimization global decls
 *      Author :        Scott Mahlke
 *
 *      (C) Copyright 1990, Scott Mahlke
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_SUPERSCALAR_H
#define L_SUPERSCALAR_H

#define VAR_EXPANSION_USE_CUTSET_METRIC
#define VAR_EXPANSION_CUTSET_RATIO 0.5
#undef DEBUG_VAR_EXPANSION_METRICS

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_opti.h>
#include <Lcode/lb_block.h>
#include "l_super_parms.h"
#include "l_loop_driver.h"
#include "l_loop_classic.h"
#include "l_rem_loop.h"
#include <Lcode/l_markpipe.h>
#include <Lcode/l_opti_count.h>

/*==========================================================================*/
/*
 *      Main optimization routines
 */
/*==========================================================================*/

extern void Lsuper_accumulator_expansion (L_Func *);
extern void Lsuper_move_comp_code_out_of_softpipe_loops (L_Func *);
extern void Lsuper_critical_path_reduction (L_Func *);
extern void Lsuper_register_renaming (L_Func *);
extern int Lsuper_jump_optimization (L_Func *, int);
extern int Lsuper_cleanup_jump_optimization (L_Func *, int);
extern int Lsuper_local_code_optimization (L_Func *, int);
extern int Lsuper_operation_migration (L_Func *);
extern int Lsuper_loop_classic_optimization (L_Func *);
extern int Lsuper_induction_var_optimization (L_Func *);
extern void Lsuper_loop_unrolling (L_Func *);
extern void L_multiway_branch_expansion (L_Func *);
extern void Lsuper_strength_reduction (L_Func *);
extern void Lsuper_unrolled_loop_renaming (L_Func *);
extern void Lsuper_combine_branches (L_Func *);
extern void Lsuper_switch_optimization(L_Func *);
extern void Lsuper_peel_accumulator_expansion (L_Func *);

#endif





