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
 *      File :          l_jump_opti.h
 *      Description :   jump optimization declarations
 *      Creation Date : July, 1990
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_JUMP_OPTI_H
#define L_JUMP_OPTI_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *      Flags used for jump optimization
 */

#define L_JUMP_ALLOW_SUPERBLOCKS        0x00000001
#define L_JUMP_ALLOW_BACKEDGE_EXP       0x00000002
#define L_JUMP_ALLOW_LOOP_BODY_EXP      0x00000004

/*
 *      External functions
 */

extern int L_jump_dead_block_removal (L_Func *, int);
extern int L_jump_elim_branch_to_next_block (L_Func *, int);
extern int L_jump_combine_branches_to_same_target (L_Func *, int);
extern int L_jump_combine_branch_to_uncond_branch (L_Func *, int);
extern int L_jump_merge_always_successive_blocks (L_Func *, int);
extern int L_jump_combine_labels (L_Func *, int);
extern int L_jump_branch_target_expansion (L_Func *, int);
extern int L_jump_branch_swap (L_Func *, int);
extern void L_jump_branch_prediction (L_Func *);

extern int L_remove_decidable_cond_branches (L_Func *, int);

extern void L_split_multidef_branches (L_Func *fn);
extern void L_jump_rg_expansion (L_Func * fn);
#endif
