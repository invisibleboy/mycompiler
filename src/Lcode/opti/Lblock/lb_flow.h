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
/*==========================================================================**
**	File :		l_flow.h
**	Description :	Flow library external functions
**	Author : 	David August, Kevin Crozier
**
**	(C) Copyright 1997, David August & Kevin Crozier
**	All rights granted to University of Illinois Board of Regents.
**==========================================================================*/

#ifndef L_FLOW_H
#define L_FLOW_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*==========================================================================**
**
**	External functions
**
**==========================================================================*/

  extern void LB_branch_split_func (L_Func *);
  extern void LB_branch_split_cb (L_Func *, L_Cb *);

  extern void LB_insert_explicit_branches_func (L_Func *);
  extern void LB_insert_explicit_branches_cb (L_Cb *);
  extern void LB_delete_explicit_branches_func (L_Func *);
  extern void LB_delete_explicit_branches_cb (L_Cb *);

  extern L_Cb * LB_breakup_cb (L_Func *, L_Cb *);
  extern void LB_convert_to_strict_basic_block_code (L_Func *, int);

  extern int LB_cb_contains_register_branch (L_Cb *);
  extern void LB_mark_jrg_flag (L_Func *);

  extern void LB_uncond_branch_elim (L_Func * fn);

  extern void LB_elim_all_loop_backedges (L_Func * fn);

#ifdef __cplusplus
}
#endif

#endif
