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
 *	File :		l_misc.h
 *	Description :	Prototypes for miscellaneous hyperblock functs
 *	Creation Date :	September 1993
 *	Authors : 	Scott Mahlke
 *       Included with Lblock in its original form from LB_hb -- KMC 4/98 
 *
 *==========================================================================*/
#ifndef L_MISC_H
#define L_MISC_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#ifdef __cplusplus
extern "C"
{
#endif

  extern void LB_hb_reset_max_oper_id (L_Func *);
  extern int LB_hb_is_single_block_loop (L_Cb *);

  extern void LB_elim_loop_backedges (L_Func * fn, L_Loop * loop);
  extern int LB_hb_loop_collapsible (LB_TraceRegion_Header *trh, 
				     L_Loop * loop);
  extern L_Loop * LB_hb_do_collapse_loops (LB_TraceRegion_Header *trh,
					   L_Func * fn, L_Loop * loop);

  extern int LB_hb_region_contains_cycle (Set blocks, L_Cb * header);

#ifdef __cplusplus
}
#endif

#endif
