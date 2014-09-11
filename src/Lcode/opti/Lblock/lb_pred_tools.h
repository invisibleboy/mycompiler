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
 *	File :		l_pred_tools.h
 *	Description :	Useful tools for defining and using predicate
 *	Creation Date :	December 1997
 *	Authors : 	Kevin Crozier
 *      (C) Copyright 1997, Kevin Crozier
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#ifndef L_PRED_TOOLS_H
#define L_PRED_TOOLS_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_code.h>

struct _LB_Predicate;

#ifdef __cplusplus
extern "C"
{
#endif
/*====================
 * External Function Prototypes
 *====================*/
  extern void LB_predicate_init (void);
  extern void LB_predicate_deinit (L_Func *);
  extern void LB_free_predicate (struct _LB_Predicate *);
  extern void LB_predicate_traceregions (L_Func *, LB_TraceRegion_Header *);
  extern void LB_predicate_traceregion (L_Func *, LB_TraceRegion *, 
					int do_frp);
  extern void LB_set_hyperblock_func_flag (L_Func *);
  extern void LB_set_hyperblock_flag (L_Func *);
  extern void LB_clr_hyperblock_flag (L_Func *);
  extern void LB_remove_unnec_hyperblock_flags (L_Func *);
#ifdef __cplusplus
}
#endif

#endif
