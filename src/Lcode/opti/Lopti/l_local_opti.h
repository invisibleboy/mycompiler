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
 *      File :          l_local_opti.h
 *      Description :   local optimization declarations
 *      Creation Date : July, 1990
 *      Authors :       Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_LOCAL_OPTI_H
#define L_LOCAL_OPTI_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *      Local optimization flags
 */
#define L_COMMON_SUB_MOVES_WITH_INT_CONSTANT            0x00000001
#define L_COMMON_SUB_MOVES_WITH_FLT_CONSTANT            0x00000002
#define L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT          0x00000004
#define L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT         0x00000008


/*
 *      External functions
 */

extern int L_local_constant_propagation (L_Cb *, int);
extern int L_local_copy_propagation (L_Cb *);
extern int L_local_rev_copy_propagation (L_Cb *);
extern int L_local_memory_copy_propagation (L_Cb *);
extern int L_local_common_subexpression (L_Cb *, int);
extern int L_local_redundant_load (L_Cb *);
extern int L_local_redundant_store (L_Cb *);
extern int L_local_constant_folding (L_Cb *);
extern int L_local_constant_combining (L_Cb *);
extern int L_local_strength_reduction (L_Cb *);
extern int L_local_operation_folding (L_Cb *);
extern int L_local_branch_folding (L_Cb *);
extern int L_local_operation_cancellation (L_Cb *);
extern int L_local_dead_code_removal (L_Cb *);
extern int L_local_pred_dead_code_removal (L_Cb *);
extern int L_local_code_motion (L_Cb *);
extern int L_local_remove_sign_extension (L_Cb *);
extern int L_local_register_renaming (L_Cb *);
extern int L_local_operation_migration (L_Cb *, int);
extern int L_local_logic_reduction (L_Cb *);
extern int L_local_oper_breakdown (L_Cb *);
extern int L_local_oper_recombine (L_Cb *);
extern int L_local_branch_val_propagation (L_Cb *);

#endif
