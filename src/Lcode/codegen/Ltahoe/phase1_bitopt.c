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
 *  File:  phase1_bitopt.c
 *
 *  Description:  Does forward and backward bit flow analysis to remove
 *                unnecessary zero and sign extends.
 *
 *                It still has room for improvement as it does not
 *                understand bit flow to a cmp4 through operations that have
 *                possible carry lefts.
 *
 *  Creation Date :  March 1997
 *
 *  Author:  Allan D. Knies
 *
 *
\*****************************************************************************/
/* 09/10/02 REK Updating file to handle the new TAHOEops. Modifying functions
 *              ComputeKnownBitsFlow, ProcessOperUsingTD,
 *              ComputeBitsUsedAndPropagate, ComputeBitsUsedLocal,
 *              TryCMPRedirect, and Ltahoe_morph_xform.
 */
/* 09/30/02 REK Updating this file so that it does not require bv64 from
 *              kapi.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "ltahoe_completers.h"
#include "ltahoe_bitvec.h"
#include "ltahoe_table.h"
#include "phase1_bitopt.h"
#include <machine/lmdes.h>
#include <assert.h>

/* Defining these can turn off certain types of SXT/ZXT removal */
/* They should all be undefined by default */
#undef DISABLE_REDUNDANT_TD_SXT_REMOVAL
#undef DISABLE_BU_REDIRECTION
#undef DISABLE_BU_CMP_REDIRECTION
#undef DISABLE_REDUNDANT_BU_SXT_REMOVAL

#undef VERBOSE_BITOPT

#define REG_BITS_USED_BY_CALLER 64
#define REG_BITS_USED_BY_RETURN 64
#define REG_BITS_USED_BY_UNSAFE 64

#define IS_EFFECTIVE_INT_CONST( operand ) \
        ( L_is_int_constant( operand ) || LT_is_R0_operand( operand ) )

#define INT_VALUE_OPERAND( operand ) \
        ( L_is_int_constant( operand ) ? (operand)->value.i : 0 )

#define ERR stderr

#define pbitGetSrc( oper, i ) \
     ( &(((BIT_INFO *)(oper)->ext)[ operand_index( MDES_SRC, (i) ) ]))
#define pbitGetDest( oper, i ) \
     ( &(((BIT_INFO *)(oper)->ext)[ operand_index( MDES_DEST, (i) ) ]))

void
L_optimize_bit_trace (L_Func * fn)
{
  /* look in L_do_flow_analysis for examples */
  L_Cb *cb;

  L_do_flow_analysis (fn, LIVE_VARIABLE | LIVE_VARIABLE_CB);

  AllocateBitinfoFields (fn);

  PG_setup_pred_graph (fn);
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    L_optimize_bit_trace_cb (fn, cb);

  DeallocateBitinfoFields (fn);
}				/* L_optimize_bit_trace */

void
L_optimize_bit_trace_cb (L_Func * fn, L_Cb * cb)
{
  Top_down_optimizations (fn, cb);
  Bottom_up_optimizations (fn, cb);
}				/* L_optimize_bit_trace_cb */

void
Top_down_optimizations (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper;
  L_Operand *operand;
  BIT_INFO *pbit;
  int i;

  /* initialize all operands bit stuff to zeros */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      for (i = 0; i < L_max_src_operand; i++)
	{
	  operand = oper->src[i];
	  if (!operand)
	    continue;
	  pbit = pbitGetSrc (oper, i);

	  /* 09/30/02 REK Changing this to not use bv64 functions. */
	  pbit->knownOne = 0;
	  pbit->knownZero = 0;
	}			/* for i */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  operand = oper->dest[i];
	  if (!operand)
	    continue;
	  pbit = pbitGetDest (oper, i);

	  /* 09/30/02 REK Changing this to not use bv64 functions. */
	  pbit->knownOne = 0;
	  pbit->knownZero = 0;
	}			/* for i */
    }				/* for oper */

  /* start top down search */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      /* zero srcs */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  operand = oper->src[i];

	  if (!operand)
	    continue;
	  pbit = pbitGetSrc (oper, i);
	  LookupTopDownBitFlow (fn, cb, oper, i,
				&(pbit->knownZero), &(pbit->knownOne));
	}			/* for i */

      ComputeKnownBitsFlow (fn, cb, oper);
      ProcessOperUsingTD (fn, cb, oper);
    }				/* for oper */
  return;
}				/* Top_down_optimizations */

/* 
   Given an oper, the knownZero, KnownUnchanged, and knownOne 
   bitvectors are set using its input operands.
*/
/* 09/10/02 REK Modifying this function to handle the new TAHOEops. */
/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitDest0;

  pbitDest0 = pbitGetDest (oper, 0);

  /* if no dests, return */
  if (!oper->dest[0])
    return;

  /* initialize bits changed to ALL, then remove as necessary */
  pbitDest0->bitsKnownUnchanged = 0;

  /* initialize known bit value info */
  pbitDest0->knownZero = 0;
  pbitDest0->knownOne = 0;

  /* 09/11/02 REK Replacing the switch statement with a table lookup. */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).ComputeKnownBitsFlow)
    LTAHOE_TABLE_ENTRY (Ltahoe_table,
			oper->proc_opc).ComputeKnownBitsFlow (fn, cb, oper);

  /* 09/11/02 REK Commenting the old error message out in case it is needed
   *              at some point.
   * default:
   * {
   *     printf ("Unknown TAHOEop(%d)  %d %s in ComputeKnownBitsFlow\n",
   *             oper->proc_opc, oper->id, oper->opcode);
   *     L_punt ("Unknown TAHOEop in bitopt");
   * }
   */
}				/* ComputeKnownBitsFlow */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_EXTR_U (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  if (L_is_register (oper->src[0])
      && IS_EFFECTIVE_INT_CONST (oper->src[1])
      && IS_EFFECTIVE_INT_CONST (oper->src[2]))
    {
      int lenExtract, posExtract;

      posExtract = INT_VALUE_OPERAND (oper->src[1]);
      lenExtract = INT_VALUE_OPERAND (oper->src[2]);

      /* carry forward bits already known about extracted field */
      pbitDest0->knownZero = LT_ZEROS_SET_LO (lenExtract) |
	LT_EXTRACT (pbitSrc0->knownZero, posExtract, lenExtract);
    }				/* if */
}				/* ComputeKnownBitsFlow_EXTR_U */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_DEP_Z (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  if (L_is_register (oper->src[0])
      && IS_EFFECTIVE_INT_CONST (oper->src[1])
      && IS_EFFECTIVE_INT_CONST (oper->src[2]))
    {
      int lenExtract, posExtract;

      posExtract = INT_VALUE_OPERAND (oper->src[1]);
      lenExtract = INT_VALUE_OPERAND (oper->src[2]);

      /* this is for the zero padding at the high order bits */
      /* this is for the zero padding at the low order bits */
      /* carry forward bits already known about extracted field */
      pbitDest0->knownZero = LT_ZEROS_SET_LO (lenExtract + posExtract) |
	LT_ONES_SET_LO (posExtract) |
	LT_EXTRACT (pbitSrc0->knownZero, 0, lenExtract);
    }				/* if */
}				/* ComputeKnownBitsFlow_DEP_Z */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_SHLADDP4 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitDest0;

  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = LT_ONES_SET_HI (32);
  pbitDest0->knownOne = 0;
}				/* ComputeKnownBitsFlow_SHLADDP4 */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_SXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  /* 
     If bit 7 is zero, then we know that all 
     the bits after it are zero 
   */
  if (pbitSrc0->knownZero & LT_SET_BIT (7))
    {
      pbitDest0->knownZero = LT_ZEROS_SET_LO (8) | pbitSrc0->knownZero;
      pbitDest0->knownOne = LT_ONES_SET_LO (8) & pbitSrc0->knownOne;
    }				/* if */

  pbitDest0->bitsKnownUnchanged = LT_ONES_SET_LO (8);
}				/* ComputeKnownBitsFlow_SXT1 */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_SXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  /* 
     If bit 15 is zero, then we know that all 
     the bits after it are zero 
   */
  if (pbitSrc0->knownZero & LT_SET_BIT (15))
    {
      pbitDest0->knownZero = LT_ZEROS_SET_LO (16) | pbitSrc0->knownZero;
      pbitDest0->knownOne = LT_ONES_SET_LO (16) & pbitSrc0->knownOne;
    }

  pbitDest0->bitsKnownUnchanged = LT_ONES_SET_LO (16);
}				/* ComputeKnownBitsFlow_SXT2 */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_SXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  /* 
     If bit 31 is zero, then we know that all 
     the bits after it are zero 
   */
  if (pbitSrc0->knownZero & LT_SET_BIT (31))
    {
      pbitDest0->knownZero = LT_ZEROS_SET_LO (32) | pbitSrc0->knownZero;
      pbitDest0->knownOne = LT_ONES_SET_LO (32) & pbitSrc0->knownOne;
    }

  pbitDest0->bitsKnownUnchanged = LT_ONES_SET_LO (32);
}				/* ComputeKnownBitsFlow_SXT4 */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_ZXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = LT_ZEROS_SET_LO (8) | pbitSrc0->knownZero;
  pbitDest0->knownOne = LT_ONES_SET_LO (8) & pbitSrc0->knownOne;
  pbitDest0->bitsKnownUnchanged = LT_ONES_SET_LO (8);
}				/* ComputeKnownBitsFlow_ZXT1 */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_ZXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = LT_ZEROS_SET_LO (16) | pbitSrc0->knownZero;
  pbitDest0->knownOne = LT_ONES_SET_LO (16) & pbitSrc0->knownOne;
  pbitDest0->bitsKnownUnchanged = LT_ONES_SET_LO (16);
}				/* ComputeKnownBitsFlow_ZXT2 */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_ZXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = LT_ZEROS_SET_LO (32) | pbitSrc0->knownZero;
  pbitDest0->knownOne = LT_ONES_SET_LO (32) | pbitSrc0->knownOne;
  pbitDest0->bitsKnownUnchanged = LT_ONES_SET_LO (32);
}				/* ComputeKnownBitsFlow_ZXT4 */

/* 09/30/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_MOV (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;
  LT_bit_vector src0KnownZero;
  LT_bit_vector src0KnownOne;
  int iconst;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitDest0 = pbitGetDest (oper, 0);

  /* get info for operand 0 */
  if (L_is_register (oper->src[0]))
    {
      src0KnownZero = pbitSrc0->knownZero;
      src0KnownOne = pbitSrc0->knownOne;

      pbitDest0->bitsKnownUnchanged = LT_ALL_ONES;
    }				/* if */
  else if (IS_EFFECTIVE_INT_CONST (oper->src[0]))
    {
      iconst = INT_VALUE_OPERAND (oper->src[0]);

      src0KnownZero = (LT_bit_vector) (~iconst);
      src0KnownOne = (LT_bit_vector) (iconst);
    }				/* else if */
  else
    {
      src0KnownZero = 0;
      src0KnownOne = 0;
    }				/* else */

  pbitDest0->knownZero = src0KnownZero;
  pbitDest0->knownOne = src0KnownOne;
}				/* ComputeKnownBitsFlow_MOVI */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_AND (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitSrc1;
  BIT_INFO *pbitDest0;
  LT_bit_vector src0KnownZero, src1KnownZero;
  LT_bit_vector src0KnownOne, src1KnownOne;
  int iconst;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitSrc1 = pbitGetSrc (oper, 1);
  pbitDest0 = pbitGetDest (oper, 0);

  /* get info for operand 0 */
  if (L_is_register (oper->src[0]))
    {
      src0KnownZero = pbitSrc0->knownZero;
      src0KnownOne = pbitSrc0->knownOne;
    }				/* if */
  else if (IS_EFFECTIVE_INT_CONST (oper->src[0]))
    {
      iconst = INT_VALUE_OPERAND (oper->src[0]);

      src0KnownZero = (LT_bit_vector) (~iconst);
      src0KnownOne = (LT_bit_vector) (iconst);
    }				/* else if */
  else
    {
      src0KnownZero = 0;
      src0KnownOne = 0;
    }				/* else */

  /* get info for operand 1 */
  if (L_is_register (oper->src[1]))
    {
      src1KnownZero = pbitSrc1->knownZero;
      src1KnownOne = pbitSrc1->knownOne;
    }				/* if */
  else if (IS_EFFECTIVE_INT_CONST (oper->src[1]))
    {
      iconst = INT_VALUE_OPERAND (oper->src[1]);

      src1KnownZero = (LT_bit_vector) (~iconst);
      src1KnownOne = (LT_bit_vector) (iconst);
    }				/* else if */
  else
    {
      src1KnownZero = 0;
      src1KnownOne = 0;
    }				/* else */

  pbitDest0->knownZero = src0KnownZero | src1KnownZero;
  pbitDest0->knownOne = src0KnownOne & src1KnownOne;
}				/* ComputeKnownBitsFlow_AND */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_OR (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitSrc1;
  BIT_INFO *pbitDest0;
  LT_bit_vector src0KnownZero, src1KnownZero;
  LT_bit_vector src0KnownOne, src1KnownOne;
  int iconst;

  pbitSrc0 = pbitGetSrc (oper, 0);
  pbitSrc1 = pbitGetSrc (oper, 1);
  pbitDest0 = pbitGetDest (oper, 0);

  /* get info for operand 0 */
  if (L_is_register (oper->src[0]))
    {
      src0KnownZero = pbitSrc0->knownZero;
      src0KnownOne = pbitSrc0->knownOne;
    }				/* if */
  else if (IS_EFFECTIVE_INT_CONST (oper->src[0]))
    {
      iconst = INT_VALUE_OPERAND (oper->src[0]);

      src0KnownZero = (LT_bit_vector) (~iconst);
      src0KnownOne = (LT_bit_vector) (iconst);
    }				/* else if */
  else
    {
      src0KnownZero = 0;
      src0KnownOne = 0;
    }				/* else */

  /* get info for operand 1 */
  if (L_is_register (oper->src[1]))
    {
      src1KnownZero = pbitSrc1->knownZero;
      src1KnownOne = pbitSrc1->knownOne;
    }				/* if */
  else if (IS_EFFECTIVE_INT_CONST (oper->src[1]))
    {
      iconst = INT_VALUE_OPERAND (oper->src[1]);

      src1KnownZero = (LT_bit_vector) (~iconst);
      src1KnownOne = (LT_bit_vector) (iconst);
    }				/* else if */
  else
    {
      src1KnownZero = 0;
      src1KnownOne = 0;
    }				/* else */

  pbitDest0->knownZero = src0KnownZero & src1KnownZero;
  pbitDest0->knownOne = src0KnownOne | src1KnownOne;
}				/* ComputeKnownBitsFlow_OR */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_ADDP4 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitDest0;

  pbitDest0 = pbitGetDest (oper, 0);

  /* conservative assumptions here */
  pbitDest0->knownZero = LT_ZEROS_SET_LO (32);
  pbitDest0->knownOne = 0;
}				/* ComputeKnownBitsFlow_ADDP4 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_LD1 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitDest0;

  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = LT_ZEROS_SET_LO (8);
  pbitDest0->knownOne = 0;
}				/* ComputeKnownBitsFlow_LD1 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_LD2 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitDest0;

  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = LT_ZEROS_SET_LO (16);
  pbitDest0->knownOne = 0;
}				/* ComputeKnownBitsFlow_LD2 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_LD4 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitDest0;

  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = LT_ZEROS_SET_LO (32);
  pbitDest0->knownOne = 0;
}				/* ComputeKnownBitsFlow_LD4 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeKnownBitsFlow_LD8 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitDest0;

  pbitDest0 = pbitGetDest (oper, 0);

  pbitDest0->knownZero = 0;
  pbitDest0->knownOne = 0;
}				/* ComputeKnownBitsFlow_LD8 */


/* 
    Given source operand known-bit information, and destination 
    known-bit information, process and possibly delete this oper
 */
/* 09/11/02 REK Modifying function so that it supports the new opcode map. */
void
ProcessOperUsingTD (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
#ifdef DISABLE_REDUNDANT_TD_SXT_REMOVAL
  return;
#endif

  /* 09/11/02 REK Replacing the switch statement with a table lookup. */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).ProcessOperUsingTD)
    LTAHOE_TABLE_ENTRY (Ltahoe_table,
			oper->proc_opc).ProcessOperUsingTD (fn, cb, oper);

  /* 09/11/02 REK Commenting the old error message out in case it is needed
   *              at some point.
   * default:
   * {
   *     printf ("Unknown TAHOEop(%d)  %d %s in ProcessOperUsingTD\n",
   *             oper->proc_opc, oper->id, oper->opcode);
   *     L_punt ("Unknown TAHOEop in bitopt");
   * }
   */
}				/* ProcessOperUsingTD */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ProcessOperUsingTD_SXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;

  pbitSrc0 = pbitGetSrc (oper, 0);

  /* if all upper bits are zero already, this does nothing */
  if ((pbitSrc0->knownZero & 0xffffffffffffff80) == 0xffffffffffffff80)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary SXT1 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */

  /* if all upper bits are one already, this does nothing */
  if ((pbitSrc0->knownOne & 0xffffffffffffff80) == 0xffffffffffff80)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary SXT1 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */
}				/* ProcessOperUsingTD_SXT1 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ProcessOperUsingTD_SXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;

  pbitSrc0 = pbitGetSrc (oper, 0);

  /* if all upper bits are zero already, this does nothing */
  if ((pbitSrc0->knownZero & 0xffffffffffff8000) == 0xffffffffffff8000)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary SXT2 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */

  /* if all upper bits are one already, this does nothing */
  if ((pbitSrc0->knownOne & 0xffffffffffff8000) == 0xffffffffffff8000)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary SXT2 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */
}				/* ProcessOperUsingTD_SXT2 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ProcessOperUsingTD_SXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;

  pbitSrc0 = pbitGetSrc (oper, 0);

  /* if all upper bits are zero already, this does nothing */
  if ((pbitSrc0->knownZero & 0xffffffff80000000) == 0xffffffff80000000)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary SXT4 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
    }				/* if */

  /* if all upper bits are one already, this does nothing */
  if ((pbitSrc0->knownOne & 0xffffffff80000000) == 0xffffffff80000000)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary SXT4 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */
}				/* ProcessOperUsingTD_SXT4 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ProcessOperUsingTD_ZXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;

  pbitSrc0 = pbitGetSrc (oper, 0);

  /* if all upper bits are zero already, this does nothing */
  if ((pbitSrc0->knownZero & 0xffffffffffffff00) == 0xffffffffffffff00)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary ZXT4 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */
}				/* ProcessOperUsingTD_ZXT1 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ProcessOperUsingTD_ZXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;

  pbitSrc0 = pbitGetSrc (oper, 0);

  /* if all upper bits are zero already, this does nothing */
  if ((pbitSrc0->knownZero & 0xffffffffffff0000) == 0xffffffffffff0000)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary ZXT2 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */
}				/* ProcessOperUsingTD_ZXT2 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ProcessOperUsingTD_ZXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper)
{
  BIT_INFO *pbitSrc0;

  pbitSrc0 = pbitGetSrc (oper, 0);

  /* if all upper bits are zero already, this does nothing */
  if ((pbitSrc0->knownZero & 0xffffffff00000000) == 0xffffffff00000000)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**TD: Removed unnecessary ZXT4 op=%d, %s\n", oper->id,
	      oper->opcode);
#endif
      oper->proc_opc = TAHOEop_MOV_GR;
      oper->opc = Lop_MOV;
    }				/* if */
}				/* ProcessOperUsingTD_ZXT4 */

/*
    Look up the known bit information for the specified source from 
    feeding operations above it and set the knownbit information for the
    destinations of this oper.
 */
/* 10/01/02 REK Changing this to not use bv64 functions or types. */
void
LookupTopDownBitFlow (L_Func * fn, L_Cb * cb, L_Oper * oper_in,
		      int idxOperandSrc, LT_bit_vector * pSrcKnownZero,
		      LT_bit_vector * pSrcKnownOne)
{
  L_Oper *oper;
  L_Operand *operand, *operandSrc;
  int i, fFoundDominator;
  BIT_INFO *pbitSrc, *pbitDest;

  pbitSrc = pbitGetSrc (oper_in, idxOperandSrc);
  operandSrc = oper_in->src[idxOperandSrc];
  fFoundDominator = 0;

  /* initialize to known, but then we'll AND out the ones we don't know */
  pbitSrc->knownOne = LT_ALL_ONES;
  pbitSrc->knownZero = LT_ALL_ONES;

  for (oper = oper_in->prev_op; oper != NULL; oper = oper->prev_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  operand = oper->dest[i];
	  if (!operand)
	    continue;
	  pbitDest = pbitGetDest (oper, i);
	  if (L_same_operand (operand, operandSrc)
	      && PG_intersecting_predicates_ops (oper, oper_in))
	    {
	      *pSrcKnownZero &= pbitDest->knownZero;
	      *pSrcKnownOne &= pbitDest->knownOne;

	      /* 
	         If oper_in is never executed (pT) unless oper is
	         executed (pT), then we've found a definite writer.
	         (If we are not in a hyperblock then all writes are
	         definite).
	       */
	      if ((!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK)) ||
		  PG_subset_predicate_ops (oper_in, oper))
		{
		  fFoundDominator = 1;
		  goto loop_exit;
		}		/* if */
	    }			/* if */
	}			/* for i */
    }				/* for oper */

loop_exit:
  /* 
     Need to be very careful -- if no definite local producer found, need to 
     zero this out since we don't know which instruction bits are inhereted
     from
   */
  if (!fFoundDominator)
    {
      pbitSrc->knownZero = 0;
      pbitSrc->knownOne = 0;
    }				/* if */
}				/* LookupTopDownBitFlow */


/* 10/01/02 REK Changing this to not use bv64 functions. */
void
Bottom_up_optimizations (L_Func * fn, L_Cb * cb)
{
  L_Oper *oper;
  L_Operand *operand;
  int i;
  BIT_INFO *pbitDest;

  /* initialize all operand's bits used to none */
  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  operand = oper->dest[i];
	  if (!operand)
	    continue;
	  pbitDest = pbitGetDest (oper, i);
	  pbitDest->bitsUsed = 0;
	}			/* for i */
    }				/* for oper */

  /* start bottom up search */
  for (oper = cb->last_op; oper != NULL; oper = oper->prev_op)
    ComputeBitsUsedAndPropagate (fn, cb, oper);
}				/* Bottom_up_optimizations */

void
ComputeBitsUsedAndPropagate (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  L_Operand *operandDest;
  L_Operand *operandSrc;
  int i;
  int procopc, isFlow;

  procopc = oper_in->proc_opc;

  /* 
     For these special cases, see if can rename consumer(s) of oper_in->dest
     to point to an the oper_in->src (i.e. un-sxt/zxt'd input operand to 
     oper_in 
   */
  if (procopc == TAHOEop_SXT1
      || procopc == TAHOEop_SXT2
      || procopc == TAHOEop_SXT4
      || procopc == TAHOEop_ZXT1
      || procopc == TAHOEop_ZXT2 || procopc == TAHOEop_ZXT4)
    {
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  L_Oper *operReader;

	  operandDest = oper_in->dest[i];
	  if (!operandDest)
	    continue;

	  /* search through the consumers */
	  operReader = oper_in;
	  while ((operReader = operFindNextReader (operReader, operandDest)))
	    {
	      /* 
	         Could break this loop when operandDest is written by another
	         oper possibly on the same control path.
	       */

	      /* 
	         If operReader uses operandDest as a src, see if can redirect
	         use to RHS of oper 
	       */
	      isFlow = 0;
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  operandSrc = operReader->src[i];
		  if (!operandSrc)
		    continue;

		  /* 
		     If reading the same operand being written by the
		     sxt/zxt, and if there is a control path between them,
		     and if the reader executes, the producer must have also 
		     executed, and the value written by the sxt is the
		     same register being read by operReader, then try
		     redirect 
		   */
		  if (L_same_operand (operandDest, operandSrc)
		      && PG_intersecting_predicates_ops (oper_in, operReader)
		      && L_no_defs_between (operandDest, oper_in, operReader)
		      && L_no_danger (L_is_macro (oper_in->src[0]),
				      L_general_load_opcode (oper_in),
				      L_general_store_opcode (oper_in),
				      oper_in, operReader)
		      && PG_subset_predicate_ops (operReader, oper_in))
		    {
		      isFlow = 1;
		      TryRedirect (oper_in, operReader, operandSrc,
				   pbitGetSrc (operReader, i));
		    }		/* if */
		}		/* for i */

	      /* do redirect hack for cmp eq/ne against small constants */
	      if (isFlow)
		TryCMPRedirect (oper_in, operReader);
	    }			/* while */
	}			/* for i */
    }				/* if */

  /* now process the oper normally */
  for (i = 0; i < L_max_dest_operand; i++)
    {
      operandDest = oper_in->dest[i];
      if (!operandDest)
	continue;

      /* 
         Find out which bits all of potential consumers might use of
         operand Dest 
       */
      ComputeBitsUsedDest (fn, cb, oper_in, i);
    }				/* for i */

  /* 
     This checks to see which bits of the src operands 
     of the current operand are used based on the bits used
     of the destination operand. 

     If the oper is not useful, change to a MOV and
     let copy prop remove it later.
   */
  ComputeBitsUsedLocal (fn, cb, oper_in);
}				/* ComputeBitsUsedAndPropagate */

/* 
 *  For the specified dest operand, find out which of its bits are referenced
 *  by consuming instructions (OR together when multiple)
 */
/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedDest (L_Func * fn, L_Cb * cb, L_Oper * oper_in,
		     int idxOperandDest)
{
  BIT_INFO *pbitDest;
  L_Operand *operand_in;

  pbitDest = pbitGetDest (oper_in, idxOperandDest);
  operand_in = oper_in->dest[idxOperandDest];

  /* 
     Go searching for uses in the CB and set 
     bits used as OR of all uses.  This needs to be
     done even for the cases that opers are used outside
     of block so that src operands of this oper have their
     BitsUsed fields set.
   */
  pbitDest->bitsUsed = 0;
  SearchAndCombineBitsUsed (fn, cb, oper_in, operand_in,
			    &(pbitDest->bitsUsed));

  if (L_in_cb_OUT_set (cb, operand_in))
    {
      pbitDest->bitsUsed = LT_ALL_ONES;
    }				/* if */

  if (L_is_unsafe_macro (operand_in))
    {
      pbitDest->bitsUsed |= LT_ONES_SET_LO (REG_BITS_USED_BY_UNSAFE);
    }				/* if */

  if (L_is_macro (operand_in) && (operand_in->value.mac == L_MAC_P16))
    {
      pbitDest->bitsUsed |= LT_ONES_SET_LO (REG_BITS_USED_BY_RETURN);
    }				/* if */

  if (LT_is_int_output_param_operand (operand_in))
    {
      pbitDest->bitsUsed |= LT_ONES_SET_LO (REG_BITS_USED_BY_CALLER);
    }				/* if */
}				/* ComputeBitsUsedDest */


/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeBitsUsedLocal (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitSrc1;
  BIT_INFO *pbitSrc2;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitSrc1 = pbitGetSrc (oper_in, 1);
  pbitSrc2 = pbitGetSrc (oper_in, 2);

  /* be conservative at first -- initialize to all useful */
  if (L_is_register (oper_in->src[0]))
    pbitSrc0->bitsUsed = LT_ALL_ONES;
  if (L_is_register (oper_in->src[1]))
    pbitSrc1->bitsUsed = LT_ALL_ONES;
  if (L_is_register (oper_in->src[2]))
    pbitSrc2->bitsUsed = LT_ALL_ONES;

  /* 09/11/02 REK Replacing the switch statement with a table lookup. */
  if (LTAHOE_TABLE_ENTRY (Ltahoe_table,
			  oper_in->proc_opc).ComputeBitsUsedLocal)
    LTAHOE_TABLE_ENTRY (Ltahoe_table,
			oper_in->proc_opc).ComputeBitsUsedLocal (fn, cb,
								 oper_in);

  /* 09/11/02 REK Commenting the old error message out incase it is needed
   *              at some point.
   * default:
   * {
   *     printf ("Unknown TAHOEop(%d)  %d %s in ComputeBitsUsedLocal\n",
   *             oper_in->proc_opc, oper_in->id, oper_in->opcode);
   *     L_punt ("Unknown TAHOEop in bitopt");
   * }
   */
}				/* ComputeBitsUsedLocal */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeBitsUsedLocal_SXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitDest0 = pbitGetDest (oper_in, 0);

  if (L_is_register (oper_in->src[0]))
    {
      pbitSrc0->bitsUsed = LT_ONES_SET_LO (8) & pbitDest0->bitsUsed;
    }				/* if */

#ifndef DISABLE_REDUNDANT_BU_SXT_REMOVAL
  if ((pbitDest0->bitsUsed & LT_ZEROS_SET_LO (8)) == 0)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**BU: Removed unnecessary SXT1 op=%d, %s\n", oper_in->id,
	      oper_in->opcode);
#endif
      oper_in->proc_opc = TAHOEop_MOV_GR;
      oper_in->opc = Lop_MOV;
    }				/* if */
#endif
}				/* ComputeBitsUsedLocal_SXT1 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_SXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitDest0 = pbitGetDest (oper_in, 0);

  if (L_is_register (oper_in->src[0]))
    {
      pbitSrc0->bitsUsed = pbitDest0->bitsUsed & LT_ONES_SET_LO (16);
    }				/* if */

#ifndef DISABLE_REDUNDANT_BU_SXT_REMOVAL
  if ((pbitDest0->bitsUsed & LT_ZEROS_SET_LO (16)) == 0)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**BU: Removed unnecessary SXT2 op=%d, %s\n", oper_in->id,
	      oper_in->opcode);
#endif
      oper_in->proc_opc = TAHOEop_MOV_GR;
      oper_in->opc = Lop_MOV;
    }				/* if */
#endif
}				/* ComputeBitsUsedLocal_SXT2 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_SXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitDest0 = pbitGetDest (oper_in, 0);

  if (L_is_register (oper_in->src[0]))
    {
      pbitSrc0->bitsUsed = pbitDest0->bitsUsed & LT_ONES_SET_LO (32);
    }				/* if */

#ifndef DISABLE_REDUNDANT_BU_SXT_REMOVAL
  if ((pbitDest0->bitsUsed & LT_ZEROS_SET_LO (32)) == 0)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**BU: Removed unnecessary SXT4 op=%d, %s\n", oper_in->id,
	      oper_in->opcode);
#endif
      oper_in->proc_opc = TAHOEop_MOV_GR;
      oper_in->opc = Lop_MOV;
    }				/* if */
#endif
}				/* ComputeBitsUsedLocal_SXT4 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_ZXT1 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitDest0 = pbitGetDest (oper_in, 0);

  if (L_is_register (oper_in->src[0]))
    {
      pbitSrc0->bitsUsed = pbitDest0->bitsUsed & LT_ONES_SET_LO (8);
    }				/* if */

#ifndef DISABLE_REDUNDANT_BU_SXT_REMOVAL
  if ((pbitDest0->bitsUsed & LT_ZEROS_SET_LO (8)) == 0)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**BU: Removed unnecessary ZXT1 op=%d, %s\n",
	      oper_in->id, oper_in->opcode);
      printf ("%s dest bits used\n", bv642pch (bv64DestBitsUsed, pchTmp));
#endif
      oper_in->proc_opc = TAHOEop_MOV_GR;
      oper_in->opc = Lop_MOV;
    }				/* if */
#endif
}				/* ComputeBitsUsedLocal_ZXT1 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_ZXT2 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitDest0 = pbitGetDest (oper_in, 0);

  if (L_is_register (oper_in->src[0]))
    {
      pbitSrc0->bitsUsed = pbitDest0->bitsUsed & LT_ONES_SET_LO (16);
    }				/* if */

#ifndef DISABLE_REDUNDANT_BU_SXT_REMOVAL
  if ((pbitDest0->bitsUsed & LT_ZEROS_SET_LO (16)) == 0)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**BU: Removed unnecessary ZXT2 op=%d, %s\n", oper_in->id,
	      oper_in->opcode);
#endif
      oper_in->proc_opc = TAHOEop_MOV_GR;
      oper_in->opc = Lop_MOV;
    }				/* if */
#endif
}				/* ComputeBitsUsedLocal_ZXT2 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_ZXT4 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitDest0 = pbitGetDest (oper_in, 0);

  if (L_is_register (oper_in->src[0]))
    {
      pbitSrc0->bitsUsed = pbitDest0->bitsUsed & LT_ONES_SET_LO (32);
    }				/* if */

#ifndef DISABLE_REDUNDANT_BU_SXT_REMOVAL
  if ((pbitDest0->bitsUsed & LT_ZEROS_SET_LO (32)) == 0)
    {
#ifdef VERBOSE_BITOPT
      printf ("\t**BU: Removed unnecessary ZXT4 op=%d, %s\n",
	      oper_in->id, oper_in->opcode);
      printf ("%s dest bits used\n", bv642pch (bv64DestBitsUsed, pchTmp));
#endif
      oper_in->proc_opc = TAHOEop_MOV_GR;
      oper_in->opc = Lop_MOV;
    }				/* if */
#endif
}				/* ComputeBitsUsedLocal_ZXT4 */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeBitsUsedLocal_MOV (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitDest0 = pbitGetDest (oper_in, 0);

  /* inheret LHS */
  if (L_is_register (oper_in->src[0]))
    pbitSrc0->bitsUsed = pbitDest0->bitsUsed;
}				/* ComputeBitsUsedLocal_MOVI_L_GR */

/* 10/01/02 REK Changing this to not use bv64 functions. */
void
ComputeBitsUsedLocal_ARITH_OP (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitSrc1;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitSrc1 = pbitGetSrc (oper_in, 1);
  pbitDest0 = pbitGetDest (oper_in, 0);

  /* inheret LHS */
  if (L_is_register (oper_in->src[0]))
    pbitSrc0->bitsUsed = pbitDest0->bitsUsed;
  if (L_is_register (oper_in->src[1]))
    pbitSrc1->bitsUsed = pbitDest0->bitsUsed;
}				/* ComputeBitsUsedLocal_ARITH_OP */

/* 10/01/02 REK Changing this to not use bv64 types. */
void
ComputeBitsUsedLocal_AND (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitSrc1;
  BIT_INFO *pbitDest0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitSrc1 = pbitGetSrc (oper_in, 1);
  pbitDest0 = pbitGetDest (oper_in, 0);

  /* inheret LHS */
  if (L_is_register (oper_in->src[0]))
    pbitSrc0->bitsUsed = pbitDest0->bitsUsed;
  if (L_is_register (oper_in->src[1]))
    pbitSrc1->bitsUsed = pbitDest0->bitsUsed;

  /* 
     This is wierd -- if one of the operands is a const, then the only 
     bits used of the other operand are those corresponding to the 1's 
     in the const (because any zero bits in the constant will always
     cause the destination to be zero in the result).
   */
  if (IS_EFFECTIVE_INT_CONST (oper_in->src[0]))
    {
      pbitSrc1->bitsUsed = LT_ZEROS_SET_HI (32) &
	INT_VALUE_OPERAND (oper_in->src[0]) & pbitDest0->bitsUsed;
    }				/* if */
  if (IS_EFFECTIVE_INT_CONST (oper_in->src[1]))
    {
      pbitSrc0->bitsUsed = LT_ZEROS_SET_HI (32) &
	INT_VALUE_OPERAND (oper_in->src[1]) & pbitDest0->bitsUsed;
    }				/* if */
}				/* ComputeBitsUsedLocal_AND */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_ST1 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc1;

  pbitSrc1 = pbitGetSrc (oper_in, 1);

  if (L_is_register (oper_in->src[1]))
    pbitSrc1->bitsUsed = LT_ONES_SET_LO (8);
}				/* ComputeBitsUsedLocal_ST1 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_ST2 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc1;

  pbitSrc1 = pbitGetSrc (oper_in, 1);

  if (L_is_register (oper_in->src[1]))
    pbitSrc1->bitsUsed = LT_ONES_SET_LO (16);
}				/* ComputeBitsUsedLocal_ST2 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_ST4 (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc1;

  pbitSrc1 = pbitGetSrc (oper_in, 1);

  if (L_is_register (oper_in->src[1]))
    pbitSrc1->bitsUsed = LT_ONES_SET_LO (32);
}				/* ComputeBitsUsedLocal_ST4 */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_TBIT_TNAT (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;

  pbitSrc0 = pbitGetSrc (oper_in, 0);

  if (L_is_register (oper_in->src[0]) &&
      IS_EFFECTIVE_INT_CONST (oper_in->src[1]))
    {
      pbitSrc0->bitsUsed = LT_SET_BIT (INT_VALUE_OPERAND (oper_in->src[1]));
    }				/* if */
}				/* ComputeBitsUsedLocal_TBIT_TNAT */

/* 10/01/02 REK Changing this to not use bv64. */
void
ComputeBitsUsedLocal_CMP (L_Func * fn, L_Cb * cb, L_Oper * oper_in)
{
  BIT_INFO *pbitSrc0;
  BIT_INFO *pbitSrc1;

  pbitSrc0 = pbitGetSrc (oper_in, 0);
  pbitSrc1 = pbitGetSrc (oper_in, 1);

  /* This only change the bits for a 4 byte compare opcode. */
  if (oper_in->completers & TC_CMP_4)
    {
      if (L_is_register (oper_in->src[0]))
	{
	  pbitSrc0->bitsUsed = LT_ONES_SET_LO (32);
	}			/* if */
      if (L_is_register (oper_in->src[1]))
	{
	  pbitSrc1->bitsUsed = LT_ONES_SET_LO (32);
	}			/* if */
    }				/* if */
}				/* ComputeBitsUsedLocal_CMP */

/* 10/01/02 REK Changing this to not use bv64. */
void
SearchAndCombineBitsUsed (L_Func * fn, L_Cb * cb, L_Oper * oper_in,
			  L_Operand * operand_in, LT_bit_vector * pBitsUsed)
{
  L_Oper *oper;
  L_Operand *operandDest;
  L_Operand *operandSrc;
  int i, fExit;
  BIT_INFO *pbitSrc;

  /* search down */
  fExit = 0;
  for (oper = oper_in->next_op; oper != NULL && !fExit; oper = oper->next_op)
    {

      /* if operand_in is potentially used by oper as a source */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  operandSrc = oper->src[i];
	  if (!operandSrc)
	    continue;
	  pbitSrc = pbitGetSrc (oper, i);

	  if (L_same_operand (operand_in, operandSrc)
	      && PG_intersecting_predicates_ops (oper_in, oper))
	    *pBitsUsed |= pbitSrc->bitsUsed;
	}			/* for i */

      /* check stop search criteria - register killed */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  operandDest = oper->dest[i];
	  if (!operandDest)
	    continue;

	  /* 
	     Does oper definitely kill oper_in?  -- If I'm not in a hyper
	     block then all matching writes kill
	   */
	  if (L_same_operand (operand_in, operandDest)
	      && ((!L_EXTRACT_BIT_VAL (cb->flags, L_CB_HYPERBLOCK))
		  || PG_subset_predicate_ops (oper_in, oper)))
	    {
	      fExit = 1;
	      break;
	    }			/* if */
	}			/* for i */
    }				/* for oper */
}				/* SearchAndCombineBitsUsed */

L_Oper *
operFindNextReader (L_Oper * operReader, L_Operand * operandDest)
{
  operReader = operReader->next_op;
  while (operReader)
    {
      if (L_is_src_operand (operandDest, operReader))
	return (operReader);

      operReader = operReader->next_op;
    }				/* while */
  return (NULL);
}				/* operFindNextReader */

/* 10/01/02 REK Changing this to not use bv64. */
void
TryRedirect (L_Oper * operProducer, L_Oper * operReader,
	     L_Operand * operandSrcReader, BIT_INFO * pbitSrcReader)
{
  L_Operand *operandRHSProducer;
  LT_bit_vector bitsProducerDoesntChange;
  int i;

  /* give up if not dest[0] */
  if (!operProducer->dest[0])
    return;

  bitsProducerDoesntChange =
    pbitGetDest (operProducer, 0)->bitsKnownUnchanged;

  /* 
     If any of the bits used by the reader are changed by 
     the producer, cannot redirect 
   */
  if ((bitsProducerDoesntChange & pbitSrcReader->bitsUsed) !=
      pbitSrcReader->bitsUsed)
    return;

  /* only try to redirect producers that are UNARY ops */
  if (L_num_src_operand (operProducer) != 1)
    return;

  operandRHSProducer = NULL;

  /* find the RHS of the producer */
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (NULL == operProducer->src[i])
	continue;
      operandRHSProducer = operProducer->src[i];
    }				/* for i */
  assert (operandRHSProducer);

  /* change operandSrcReader to be the same as operandRHSProducer */
#ifndef DISABLE_BU_REDIRECTION
  *operandSrcReader = *operandRHSProducer;
#ifdef VERBOSE_BITOPT
  printf ("\t**BU: Redirected reader %d(%s) from LHS to RHS of %d(%s)\n",
	  operReader->id, operReader->opcode,
	  operProducer->id, operProducer->opcode);
#endif
#endif
}				/* TryRedirect */

/* 
 *  This is a pretty specialized operation -- it looks for very specialized
 *  patterns similar to:
 *
 *       ld1  r1 = [ r2 ];
 *       sxt1 r3 = r1;
 *       cmp4 px = r3,5
 *
 *  In this case, the sxt is unnecessary (really)
 */
/* 09/11/02 REK Modifying this function so that it uses the new TAHOEops. */
/* 10/01/02 REK Changing this to not use bv64. */
void
TryCMPRedirect (L_Oper * operProducer, L_Oper * operReader)
{
  L_Operand *operandProdSrc0, *operandRHSProducer, *operandReaderReader;
  L_Operand *operandSrc0, *operandSrc1;
  LT_bit_vector ones;
  int i, zCMP, zSXT, iValue;
  BIT_INFO *pbitSrc0;


#ifdef VERBOSE_BITOPT
  printf ("Trying CMPRedirect %d %s -> %d %s\n",
	  operProducer->id, operProducer->opcode,
	  operReader->id, operReader->opcode);
#endif

  /* 09/11/02 REK This check is much simpler now */
  if (operReader->proc_opc != TAHOEop_CMP)
    return;

  operandSrc0 = operReader->src[0];
  operandSrc1 = operReader->src[1];

  /* only match on SXTs */
  if (operProducer->proc_opc != TAHOEop_SXT1
      && operProducer->proc_opc != TAHOEop_SXT2
      && operProducer->proc_opc != TAHOEop_SXT4)
    return;

  /* if high order bits of operProducer are known zero */
  /* 09/16/02 REK Changing to look at the completers field. */
  /* if (M_is_cmp4_op (operReader)) */
  if (operReader->completers & TC_CMP_4)
    zCMP = 4;
  else
    zCMP = 8;

  zSXT = -100;
  if (operProducer->proc_opc == TAHOEop_SXT1)
    zSXT = 1;
  else if (operProducer->proc_opc == TAHOEop_SXT2)
    zSXT = 2;
  else if (operProducer->proc_opc == TAHOEop_SXT4)
    zSXT = 4;

  /* if this is a compare is smaller or equal size than sxt, no need */
  if (zCMP <= zSXT)
    return;

  /* 
     If one of the operands to the compare is an integer constant.
   */
  if (L_is_register (operandSrc0))
    {
      operandReaderReader = operandSrc0;
      if (IS_EFFECTIVE_INT_CONST (operandSrc1))
	iValue = INT_VALUE_OPERAND (operandSrc1);
      else
	return;
    }				/* if */
  else if (L_is_register (operandSrc1))
    {
      operandReaderReader = operandSrc1;
      if (IS_EFFECTIVE_INT_CONST (operandSrc0))
	iValue = INT_VALUE_OPERAND (operandSrc0);
      else
	return;
    }				/* else if */
  else
    {
      return;
    }				/* else */

  /* if the integer constant is in range */
  if (iValue < 0 || iValue >= (1 << ((8 * zSXT) - 1)))
    return;

  operandProdSrc0 = operProducer->src[0];
  pbitSrc0 = pbitGetSrc (operProducer, 0);
  assert (operandProdSrc0);

  ones = LT_ZEROS_SET_LO (8 * zSXT);

  /* if all the upper bits of rhs of SXT are not known zero, quit */
  if ((ones & pbitSrc0->knownZero) != ones)
    return;

#ifdef VERBOSE_BITOPT
  printf (" known zero data verified %d %s\n",
	  operProducer->id, operProducer->opcode);
#endif

  operandRHSProducer = NULL;

  /* find the RHS of the producer */
  for (i = 0; i < L_max_src_operand; i++)
    {
      if (NULL == operProducer->src[i])
	continue;
      operandRHSProducer = operProducer->src[i];
    }				/* for i */
  assert (operandRHSProducer == operandProdSrc0);

  /* change operandReaderReader to be the same as operandRHSProducer */
#ifndef DISABLE_BU_CMP_REDIRECTION
  *operandReaderReader = *operandRHSProducer;
#ifdef VERBOSE_BITOPT
  printf ("\t**BU: CMP Redirected reader %d(%s) from LHS to RHS of %d(%s)\n",
	  operReader->id, operReader->opcode,
	  operProducer->id, operProducer->opcode);
#endif
#endif
}				/* TryCMPRedirect */


void
DeallocateBitinfoFields (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  if (oper->ext != NULL)
	    {
	      free (oper->ext);
	      oper->ext = NULL;
	    }			/* if */
	}			/* for oper */
    }				/* for cb */
}				/* DeallocateBitinfoFields */

void
AllocateBitinfoFields (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  int n;

  n = L_max_src_operand + L_max_dest_operand + L_max_pred_operand;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
	  assert (oper->ext == NULL);
	  oper->ext = (BIT_INFO *) calloc (n, sizeof (BIT_INFO));
	}			/* for oper */
    }				/* for cb */
}				/* AllocateBitinfoFields */
