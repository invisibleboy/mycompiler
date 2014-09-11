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
 *      File :          l_branch.h
 *      Description :   classify branches (analysis only, no opti here :P)
 *      Creation Date : October 1994
 *      Authors :       Scott Mahlke
 *
 *      (C) Copyright 1994 Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_BRANCH_H
#define L_BRANCH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

/*
 *      Oper flags used to mark branches
 */
#define L_OPER_CBR                      L_OPER_RESERVED_TEMP1
#define L_OPER_UBR                      L_OPER_RESERVED_TEMP2
#define L_OPER_JSR                      L_OPER_RESERVED_TEMP3
#define L_OPER_RTS                      L_OPER_RESERVED_TEMP4
#define L_OPER_JRG                      L_OPER_RESERVED_TEMP5


/*
 *      Branch classes and locations
 */
#define L_BR_LOOPBACK_INNER             0x00000001
#define L_BR_LOOPBACK_OUTER             0x00000002
#define L_BR_LOOPEXIT_INNER             0x00000004
#define L_BR_LOOPEXIT_OUTER             0x00000008
#define L_BR_NONLOOP_INNER              0x00000010
#define L_BR_NONLOOP_OUTER              0x00000020
#define L_BR_NONLOOP_STLN               0x00000040


/*
 *      Attribute names for each branch class
 */
#define L_BR_LOOPBACK_INNER_NAME        "LB_inner"
#define L_BR_LOOPBACK_OUTER_NAME        "LB_outer"
#define L_BR_LOOPEXIT_INNER_NAME        "LE_inner"
#define L_BR_LOOPEXIT_OUTER_NAME        "LE_outer"
#define L_BR_NONLOOP_INNER_NAME         "NL_inner"
#define L_BR_NONLOOP_OUTER_NAME         "NL_outer"
#define L_BR_NONLOOP_STLN_NAME          "NL_stln"

/*
 *      Attribute name for loop nest of branch
 */
#define L_BR_LOOPNEST_NAME              "loop_nest"


/*
 *      Function prototypes 
 */
extern void L_mark_branches (L_Func *);
extern void L_classify_branches (L_Func *);

#endif
