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
 *      File: l_pipe_mspec.c
 *      Description: machine specific routines
 *      Creation Date: February, 1995
 *      Author: Daniel Lavery and Richard Hank
 *
 *  Copyright (c) 1995 Daniel Lavery, Richard Hank, Wen-mei Hwu, 
 *                     and The Board of Trustees of the University of Illinois.
 *                     All rights reserved.
 *      The University of Illinois Software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
/* 12/03/02 REK Taking out the lhppa requirement for distribution. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <stdio.h>
#include <Lcode/l_main.h>
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
#include <Lcode/lhppa_phase1.h>
#endif
#endif

/* returns HP PA comparison op which corresponds to the Lcode compare
   and branch */

#if 0
#ifndef OPENIMPACT_DISTRIBUTION
int
Lpipe_hppa_branch_condition (opc)
     int opc;
{
  int hp_op;

  switch (opc)
    {
    case Lop_BEQ_FS:
    case Lop_BEQ:
      hp_op = COMP_EQ;
      break;
    case Lop_BNE_FS:
    case Lop_BNE:
      hp_op = COMP_NE;
      break;
    case Lop_BGT_FS:
    case Lop_BGT:
      hp_op = COMP_GT;
      break;
    case Lop_BGE_FS:
    case Lop_BGE:
      hp_op = COMP_GE;
      break;
    case Lop_BLT_FS:
    case Lop_BLT:
      hp_op = COMP_LT;
      break;
    case Lop_BLE_FS:
    case Lop_BLE:
      hp_op = COMP_LE;
      break;
    case Lop_BGT_U_FS:
    case Lop_BGT_U:
      hp_op = COMP_GT_U;
      break;
    case Lop_BGE_U_FS:
    case Lop_BGE_U:
      hp_op = COMP_GE_U;
      break;
    case Lop_BLT_U_FS:
    case Lop_BLT_U:
      hp_op = COMP_LT_U;
      break;
    case Lop_BLE_U_FS:
    case Lop_BLE_U:
      hp_op = COMP_LE_U;
      break;
    }

  return (hp_op);
}
#endif
#endif
