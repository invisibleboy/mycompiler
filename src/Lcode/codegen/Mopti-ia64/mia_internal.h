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
 * mia_internal.h                                                            *
 * ------------------------------------------------------------------------- *
 * Internal defines and includes for Mopti-ia64                              *
 *                                                                           *
 * AUTHORS: J.W. Sias                                                        *
 *****************************************************************************/

#ifndef _MIA_INTERNAL_H
#define _MIA_INTERNAL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_opti.h>
#include <Lcode/ltahoe_op_query.h>
#include <Lcode/ltahoe_completers.h>
#include <machine/m_tahoe.h>
#include "mia_opti.h"
#include <Lcode/l_opti_predicates.h>

/* mia_opti.c */

extern void Mopti_debug (char *fmt, ...);

#define MOD if (Mopti_debug_messages) Mopti_debug

/* mia_compare.c */

extern int Mopti_loop_compare_height_reduction (L_Func * fn);
extern int Mopti_compare_height_reduction (L_Func * fn, L_Cb * cb,
					   L_Loop * loop, int loop_n_cb,
					   int *loop_cb);

#define Ltahoe_new_int_reg() L_new_register_operand (++(L_fn->max_reg_id), \
					       L_CTYPE_LLONG, 0)

#define Ltahoe_new_pred_reg(ptype) L_new_register_operand (++(L_fn-> \
                                               max_reg_id), \
					       L_CTYPE_PREDICATE, (ptype))

#define Ltahoe_copy_or_new(cop,opd) (cop) = ((opd) == NULL) ? \
                                            Ltahoe_new_int_reg() : \
                                            L_copy_operand ((opd))

#define Ltahoe_true_pred(ptype) L_new_macro_operand (TAHOE_MAC_PRED_TRUE, \
                                                     L_CTYPE_PREDICATE, \
				                     (ptype))

/* COMMON TAHOE MACROS */

#define Ltahoe_IMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_LLONG, 0)
#define Ltahoe_PMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_PREDICATE, 0)
#define Ltahoe_FMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_DOUBLE, 0)
#define Ltahoe_DMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_DOUBLE, 0)
#define Ltahoe_BMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_BTR, 0)

#endif
