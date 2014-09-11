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
 *  File:  phase1_bitopt.h
 *
 *  Description:  Contains function declarations for phase1_bitopt.c
 *
 *  Creation Date :  September 2002
 *
 *  Author:  Robert Kidd
 *
 *
\*****************************************************************************/
/* 09/30/02 REK Updating this file so that it does not require bv64 from
 *              kapi.
 */

#ifndef _PHASE1_BITOPT_H
#define _PHASE1_BITOPT_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_bitvec.h"

/* this definition is not needed outside this file */
typedef struct _BIT_INFO
{
  LT_bit_vector knownZero;
  LT_bit_vector knownOne;
  LT_bit_vector bitsKnownUnchanged;
  LT_bit_vector bitsUsed;

/*    bv64_t bv64KnownZero; */
/*    bv64_t bv64KnownOne; */
/*    bv64_t bv64BitsKnownUnchanged; */
/*    bv64_t bv64BitsUsed; */
}
BIT_INFO;

/* prototypes */
extern void L_optimize_bit_trace (L_Func * fn);
extern void L_optimize_bit_trace_cb (L_Func * fn, L_Cb * cb);
extern void ComputeKnownBitsFlow (L_Func * fn, L_Cb * cb, L_Oper * oper);
extern void ProcessOperUsingTD (L_Func * fn, L_Cb * cb, L_Oper * oper);

extern void Top_down_optimizations (L_Func * fn, L_Cb * cb);
extern void Bottom_up_optimizations (L_Func * fn, L_Cb * cb);
extern void LookupTopDownBitFlow (L_Func * fn, L_Cb * cb, L_Oper * oper_in,
				  int idxOperandSrc,
				  LT_bit_vector * pKnownZero,
				  LT_bit_vector * pKnownOne);
extern void ComputeBitsUsedLocal (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
extern void ComputeBitsUsedAndPropagate (L_Func * fn, L_Cb * cb,
					 L_Oper * oper_in);
extern void SearchAndCombineBitsUsed (L_Func * fn, L_Cb * cb,
				      L_Oper * oper_in,
				      L_Operand * operand_in,
				      LT_bit_vector * pBitsUsed);
extern L_Oper *operFindNextReader (L_Oper * operReader,
				   L_Operand * operandDest);
extern void TryRedirect (L_Oper * operDest, L_Oper * operReader,
			 L_Operand * operandSrc, BIT_INFO * pbitSrcReader);
extern void ComputeBitsUsedDest (L_Func * fn, L_Cb * cb, L_Oper * oper_in,
				 int idxOperandDest);
extern void TryCMPRedirect (L_Oper * operProducer, L_Oper * operReader);
extern void DeallocateBitinfoFields (L_Func * fn);
extern void AllocateBitinfoFields (L_Func * fn);

/* Opcode specific functions that are accessed through the opcode
 * map table. */
void ComputeKnownBitsFlow_EXTR_U (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_DEP_Z (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_SHLADDP4 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_SXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_SXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_SXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_ZXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_ZXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_ZXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_MOV (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_AND (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_OR (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_ADDP4 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_LD1 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_LD2 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_LD4 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ComputeKnownBitsFlow_LD8 (L_Func * fn, L_Cb * cb, L_Oper * oper);

void ProcessOperUsingTD_SXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ProcessOperUsingTD_SXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ProcessOperUsingTD_SXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ProcessOperUsingTD_ZXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ProcessOperUsingTD_ZXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper);
void ProcessOperUsingTD_ZXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper);

void ComputeBitsUsedLocal_SXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_SXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_SXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_ZXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_ZXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_ZXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_MOV (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_ARITH_OP (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_AND (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_ST1 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_ST2 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_ST4 (L_Func * fn, L_Cb * cb, L_Oper * oper_in);
void ComputeBitsUsedLocal_TBIT_TNAT (L_Func * fn, L_Cb * cb,
				     L_Oper * oper_in);
void ComputeBitsUsedLocal_CMP (L_Func * fn, L_Cb * cb, L_Oper * oper_in);

#endif
