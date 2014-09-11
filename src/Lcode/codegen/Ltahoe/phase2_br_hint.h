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
/**************************************************************************\
 *
 *  File:  phase2_br_hint.h
 *
 *  Description:
 *    Branch hints header file
 *
 *  Authors:  Jim Pierce
 *  Modified: Kevin Crozier - 6/22/98 added number of bundles for counted
 *                               prefetch study.
 *
 *
\************************************************************************/
/* 09/17/02 REK Merging some constants from Tmdes into this file. */

#ifndef LTAHOE_PHASE2_BR_HINT_H_
#define LTAHOE_PHASE2_BR_HINT_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>

/* outside this prob range, cond br's 
   are statically hinted */

extern double Ltahoe_dp_upper_prob;
extern double Ltahoe_dp_lower_prob;

#ifndef _TMDES_INSTR_H_

/* Branch types */
enum BTYPE_e
{ BR_COND = 0, BR_CALL, BR_ADV };

/* Constants for print routine */
enum
{ INSERTED_HINT, MISS_IN_CB, INSERT_FAILED };

/* Constants for path direction */
enum
{ BRP_TO_BR, BR_TO_BRP };

/* Operation tags for BH_cb_bundles function */

enum
{ BUNDLES_IN_CB, INITIALIZE, CLEANUP, UPDATE, BETWEEN,
  NUM_EXPANDED, NUM_INSERTED, ADD_EXPANDED, ADD_INSERTED
};

/* 09/17/02 REK Bringing these constants over from Tmdes */
/* Mov to Br Modifier enums - don't change order */

enum MWH_e
{ MWH_NONE, MWH_SPTK, MWH_DPTK };

/* Branch Modifier enums - don't change order */

enum BWH_e
{ BWH_SPNT = 0, BWH_SPTK, BWH_DPNT, BWH_DPTK };

enum PH_e
{ PH_NONE = 0, PH_FEW, PH_MANY };

enum DH_e
{ DH_NONE = 0, DH_CLR };

/* Branch Hint Modifier enums - don't change order */

enum IPWH_e
{ IPWH_SPTK = 0, IPWH_LOOP, IPWH_EXIT, IPWH_DPTK };

enum IPDWH_e
{ INDWH_SPTK = 0, INDWH_DPTK };

enum PVEC_e
{ PVEC_DC_DC = 0, PVEC_DC_NT, PVEC_TK_DC, PVEC_TK_TK,
  PVEC_TK_NT, PVEC_NT_DC, PVEC_NT_TK, PVEC_NT_NT
};

enum IH_e
{ IH_NONE = 0, IH_IMP };

#define PVEC_NONE  PVEC_DC_DC

#endif

extern void O_count_bundles_before_branch_func (L_Func * fn);
extern int O_count_bundles_before_branch (L_Cb * starting_cb,
					  L_Oper * starting_oper);
extern void O_insert_branch_hints (L_Func * fn);
extern void O_insert_br_instr_hints_only (L_Func * fn);

#endif
