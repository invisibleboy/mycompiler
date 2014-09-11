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
 *  File:  l_mcb.h
 *
 *  Description: header file for mcb code.
 *
 *  Creation Date :  July, 1993
 *
 *  Author:  Dave Gallagher
 *
 *  Revisions:
 *
 * 	(C) Copyright 1993, Dave Gallagher
 * 	All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/
/* 12/03/02 REK Taking out the lhppa requirement for distribution. */

#ifndef MCB_H
#define MCB_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
#include <Lcode/lhppa_phase1.h>
#endif
#endif
#include "l_schedule.h"
#include <machine/m_hppa.h>

/******************************************************************************\
 *
 *  External Function Declarations
 *
\******************************************************************************/

extern void L_change_beq_mcb(L_Cb *cb);
extern L_Cb *L_copy_all_oper_in_cb (L_Func *fn, L_Cb *cb);
extern void L_mcb_insert_check_and_rem_dependences (L_Cb *cb);
extern int L_dependent_memory_ops (L_Cb *cb, L_Oper *op1, L_Oper *op2);
extern void L_remove_sched_cb (L_Func *fn, L_Cb *cb);
extern void L_mcb_init();
extern void L_free_oper_list(L_Oper *oper);
extern void L_mcb_remove_check (L_Cb *cb, L_Oper *oper, int ready_time);
extern L_Oper_List *L_new_oper_list();
extern void L_free_oper_list(L_Oper *oper);

#endif
