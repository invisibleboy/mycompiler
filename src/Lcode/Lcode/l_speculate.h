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
 *  File:  l_speculate.h
 *
 *  Description:  Support for Various Speculation Models
 *
 *  Creation Date :  June 1996
 *
 *  Author:  David August, Wen-mei Hwu
 *
\*****************************************************************************/

#ifndef _LCODE_L_SPECULATE_H_
#define _LCODE_L_SPECULATE_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>

/* speculation model names */
/* Depricated 01/2001 RDB */
#define BASIC_BLOCK             0
#define RESTRICTED              1
#define GENERAL                 2
#define WBS                     3
#define WRITEBACK_SUPPRESSION   3
#define SENTINEL                4
#define BOOSTING                5
#define MCB                     6
#define ALAT                    7
#define SRB                     8

extern L_Oper *L_insert_check (L_Cb *, L_Oper *);
extern L_Oper *L_insert_check_after (L_Cb *, L_Oper *, L_Oper *);
extern L_Oper *L_insert_check_before (L_Cb *, L_Oper *, L_Oper *);
extern L_Oper *L_global_insert_check_after (L_Oper *, L_Cb *, L_Oper *);
extern L_Oper *L_global_insert_check_before (L_Oper *, L_Cb *, L_Oper *);
extern void L_assign_all_checks(L_Func *fn, L_Oper *from_op, L_Oper *to_op);

#if 0
extern void L_advance_op (L_Oper *);
#endif

extern void L_mark_oper_speculative (L_Oper *);

#endif
