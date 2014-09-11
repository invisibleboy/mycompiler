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
 *      File:   l_trace_interface.h
 *      Author: John Gyllenhaal
 *      Creation Date:  April 1994
 *      Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and 
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef L_TRACE_INTERFACE_H
#define L_TRACE_INTERFACE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#define L_TRACE_PRED_CLR        0
#define L_TRACE_PRED_SET        1
#define L_TRACE_PRED_UNDEF      2
#define L_TRACE_FN              3
#define L_TRACE_STUB            4   /* type for long value */
#define L_TRACE_BRTHRU          5
#define L_TRACE_START           6   /* Will go away soon, use FORMAT1 below */
#define L_TRACE_START_FORMAT1   6   /* Format before 10/14/96 */
#define L_TRACE_END             7
#define L_TRACE_WRITE           8
#define L_TRACE_RTS             9
#define L_TRACE_SAMPLE_START    10
#define L_TRACE_SAMPLE_END      11
#define L_TRACE_PREFETCH        12
#define L_TRACE_MEM_COPY        13
#define L_TRACE_MEM_COPY_BACK   14
#define L_TRACE_MEM_COPY_CHECK  15
#define L_TRACE_MASKED_SEG_FAULT        16
#define L_TRACE_NO_SEG_FAULT    17      /* Must be MASKED_SEG_FAULT + 1 */
#define L_TRACE_START_FORMAT2   18      /* New trace format 10/14/96 */
                                        /* Pred defs now also traced */
#define L_TRACE_START_FORMAT3   19      /* New trace format -ITI/JCG 1/99 */
/*
 * Additional trace headers to support alternate tracing modes in 
 * Lemulate version 2.2. -ITI/JCG 4/99 
 *
 * Based on ideas and suggestions from Dan Connors, Scott Mahlke, 
 * Jason Fritts, Jean-Michel Puiatti, and many others. 
 */

/* Headers to support parsing trace without benchmark assembly -ITI/JCG 4/99 */
#define L_TRACE_READ            20      /* Load address next trace word */
#define L_TRACE_PRED_DEF        21      /* Pred value after def (next word) */
#define L_TRACE_PROMOTED_PRED   22      /* Pred value after def (next word) */
#define L_TRACE_SWITCH_CASE     23      /* src[1] on jump_rg's */
#define L_TRACE_RET_FROM_JSR    24      /* JSR location in next two words */

/* Two different headers for tracing every operation executed -ITI/JCG 4/99*/
#define L_TRACE_OP_ID           27      /* Lcode op id in next word */
#define L_TRACE_FN_OP_ID        28      /* Lcode fn & op id in next two words */

/* Headers to support various levels of value tracing -ITI/JCG 4/99*/
#define L_TRACE_VALUE           29      /* General value profiling */
#define L_TRACE_DEST_REG_I      30      /* Integer dest register value */
#define L_TRACE_DEST_REG_F      31      /* Float dest register value */
#define L_TRACE_DEST_REG_F2     32      /* Double dest register value */
#define L_TRACE_SRC_REG_I       33      /* Integer src register value */
#define L_TRACE_SRC_REG_F       34      /* Float src register value */
#define L_TRACE_SRC_REG_F2      35      /* Double src register value */
#define L_TRACE_SRC_LIT_I       36      /* Integer src literal value */
#define L_TRACE_SRC_LIT_F       37      /* Float src literal value */
#define L_TRACE_SRC_LIT_F2      38      /* Double src literal value */

/* JWS / HCH 19991015 Support for measuring use of speculative loads */
#define L_TRACE_CHECK           39

#define L_TRACE_ASYNCH          40

/* JSR id (from top of function) is encoded as (L_TRACE_JSR_OFFSET - id)
 * when put into the trace. 
 */
#define L_TRACE_JSR_OFFSET      -2048


/* To support Lemulates tracing options and future trace formats 
 * seamlessly, allow a set of trace flags (TF) to be placed after
 * L_TRACE_START_FORMAT3. -ITI/JCG 1/99
 *
 * Note: These are bit flags, assume 32 bit words.
 */

/* Lemulate's function id method is being used (verses function address).
 * The ids start at 1000 and are numbered in the order found in
 * the encoded file. -ITI/JCG 1/99
 */
#define TF_FUNC_IDS                     0x0000001

/* These TRACE_FLAGs correspond directly to the Lemulate trace parameters.  
 * See impact/parms/LEMULATE_DEFAULTS for description of each parameter.  
 * These flags allow tools to adapt to different trace settings or punt 
 * if something unexpected is thrown at them. -ITI/JCG 4/99
 */
#define TF_PROBE_FOR_PROFILING          0x0000002
#define TF_PROBE_FOR_SIMULATION         0x0000004
#define TF_PROBE_FOR_CUSTOM             0x0000008
#define TF_PREDICATE_PROBE_CODE         0x0000010
#define TF_TRACE_CONTROL_FLOW           0x0000020
#define TF_TRACE_EMPTY_CBS              0x0000040
#define TF_TRACE_MEM_ADDRS              0x0000080
#define TF_TRACE_MASKED_LOAD_FAULTS     0x0000100
#define TF_TRACE_JUMP_RG_SRC1           0x0000200
#define TF_TRACE_PRED_USES              0x0000400
#define TF_TRACE_PRED_DEFS              0x0000800
#define TF_TRACE_PROMOTED_PREDS         0x0001000
#define TF_TRACE_PRED_JUMP_FALL_THRU    0x0002000
#define TF_TRACE_EXTRA_HEADERS          0x0004000
#define TF_TRACE_OP_IDS                 0x0008000
#define TF_TRACE_ENHANCED_OP_IDS        0x0010000
#define TF_TRACE_DEST_REG_VALUES        0x0020000
#define TF_TRACE_SRC_REG_VALUES         0x0040000
#define TF_TRACE_SRC_LIT_VALUES         0x0080000

/* If need more than 32 flags, use last bit to flag that there is a 
 * second 32-bit word of flags.  Not needed yet, so delaying implementation
 * until required.  -ITI/JCG 4/99
 */


/* The following union is used to split a double value 'f2' into two 
 * 32-bit words for transmision thru the trace pipe. This structure may 
 * be to reconstruct the double on the receiving end and to store value 
 * tracing values in general.  -ITI/JCG 4/99
 *
 * Note: Value tracing of doubles in this manner should be verified to
 *       work with the host compiler before trusting this hack!  The trace
 *       with value profiles is unlikely to be readable on another
 *       platform.
 */
typedef union Trace_Value
{
  int i;
  float f;
  double f2;
  struct
  {
    int w1;                     /* Assumed to be the first word of double f2 */
    int w2;                     /* Assumed to be the second word of double f2 */
  }
  word;
}
Trace_Value;

#define L_TRACE_OBJ_GLOB               0x00000001
#define L_TRACE_OBJ_STAK               0x00000002
#define L_TRACE_OBJ_HEAP               0x00000004

#endif
