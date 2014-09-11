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
 *	File :		l_path.h
 *	Description :	Data structs for path information
 *	Creation Date :	February 1994
 *	Authors : 	Scott Mahlke
 *       Included with Lblock in its original form from Lhyper -- KMC 4/98  
 *
 *==========================================================================*/
#ifndef L_PATH_H
#define L_PATH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <library/set.h>
#include <Lcode/l_code.h>
#include <Lcode/sm.h>

#define HB_SELECT_ALL_ATTR              "HB_select_all"
#define HB_SELECT_EXACT_ATTR            "HB_select_exact"
#define HB_IGNORE_ATTR			"HB_ignore"

typedef struct _LB_hb_Sort_List
{
  double weight;
  void *ptr;
}
LB_hb_Sort_List;

extern int LB_hb_path_max_path_exceeded;

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *    External functions
 */

  extern int LB_hb_select_all_blocks (L_Cb *);
  extern int LB_hb_select_exact_blocks (L_Cb *);
  extern int LB_hb_ignore_block (L_Cb *);

  extern void LB_hb_init_path_globals (void);
  extern void LB_hb_deinit_path_globals (void);

  extern int LB_hb_find_all_paths (L_Cb *, L_Cb *, Set);
  extern void LB_hb_find_path_info (L_Cb *, L_Cb *, Set);
  extern LB_TraceRegion * LB_hb_select_paths (int, L_Cb *, L_Cb *, Set, int);
  extern LB_TraceRegion * LB_hb_select_trivial (int, L_Cb *, L_Cb *, Set, int);

  extern LB_TraceRegion * LB_hb_path_region_formation (L_Cb *, L_Cb *,
						       Set, int,
						       LB_TraceRegion_Header *);
#ifdef __cplusplus
}
#endif

#endif
