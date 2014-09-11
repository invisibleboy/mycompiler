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
 *	File :		l_region_general.h
 *	Description :	Data structs for general regions
 *	Creation Date :	February 1994
 *	Authors : 	Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98 
 *        wirth minor changes for traceregion support
 *==========================================================================*/
#ifndef L_REGION_GENERAL_H
#define L_REGION_GENERAL_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/set.h>
#include <Lcode/l_code.h>

/*
 *      Flags to control the block selection routine
 */

#define LB_BLOCK_SEL_NULL                   0x0
#define LB_BLOCK_SEL_NO_NESTED_HAMMOCKS     0x1
#define LB_BLOCK_SEL_NO_NESTED_LOOPS        0x2
#define LB_BLOCK_SEL_AGGRESSIVE             0x4

typedef struct _LB_Cb_Info {
  int flags;
  int dep_height;
  int slots_used;
} LB_Cb_Info;

extern LB_Cb_Info *LB_hb_cb_info;
extern int LB_hb_curr_slots_used;
extern int LB_hb_curr_slots_avail;
extern int LB_hb_curr_dep_height;

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *    External functions
 */

  extern int LB_hb_calc_cb_dep_height (L_Cb * cb);
  extern void LB_hb_print_cb_info (FILE * F, L_Cb * cb);
  extern void LB_hb_print_all_cb_info (FILE * F, L_Func * fn);
  extern void LB_hb_find_cb_info (L_Func * fn);
  extern int LB_hb_can_predicate_cb (L_Cb * cb);
  extern int LB_hb_select_blocks (L_Cb * cb, L_Cb * header, Set * blocks,
				    Set avail_blocks, int main_path,
				    double path_ratio, int simple_formation,
				    LB_TraceRegion_Header * tr_header);
  extern void LB_hb_find_general_regions (L_Func * fn, int simple_formation,
					   LB_TraceRegion_Header *);

#ifdef __cplusplus
}
#endif

#endif
