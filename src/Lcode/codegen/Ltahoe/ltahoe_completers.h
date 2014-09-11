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

#ifndef _COMPLETERS_H
#define _COMPLETERS_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_code.h>

/* Standard fields:
 * Temporal Locality: NONE | NT1 | NT2 | NTA (2 bits (30 & 31))
 * Branch Hint 1: NONE | TK | NT | DC (2 bits (28 & 29))
 * Branch Hint 2: NONE | TK | NT | DC (2 bits (26 & 27))
 * Branch Whether: NONE | DPTK | DPNT | SPTK | SPNT (3 bits (23, 24 & 25))
 * Comparison Op: NONE | EQ | NE | LT | LE | GT | GE | LTU | LEU | GTU | GEU |
 * 		NEQ | NLT | NLE | NGT | NGE | ORD | UNORD | Z | NZ | M | NM
 * 	(5 bits (18, 19, 20, 21, & 22))
 * Comparison Type: NONE | UNC | OR | AND | OR.ANDCM | ORCM | ANDCM | AND.ORCM
 * 	(3 bits (15, 16, 17))
 * Load Type: NONE | A | S | SA (2 bits (13 & 14))
 */

#define TC_TEMPORAL_LOCALITY_START	30	/* The low bit for the field. */
#define TC_TEMPORAL_LOCALITY_MASK	0xC0000000
#define TC_TEMPORAL_LOCALITY_NONE	0x0	/* Will be 0x00000000 */
#define TC_TEMPORAL_LOCALITY_NT1	0x1	/* Will be 0x40000000 */
#define TC_TEMPORAL_LOCALITY_NT2	0x2	/* Will be 0x80000000 */
#define TC_TEMPORAL_LOCALITY_NTA	0x3	/* Will be 0xC0000000 */

#define TC_BR_HNT1_START		28	/* The low bit for the field. */
#define TC_BR_HNT1_MASK			0x30000000
#define TC_BR_HNT2_START		26	/* The low bit for the field. */
#define TC_BR_HNT2_MASK			0x0C000000
#define TC_BR_HNT_NONE			0x0	/* Will be 0x00000000 */
#define TC_BR_HNT_TK			0x1	/* 0x10000000 or 0x04000000 */
#define TC_BR_HNT_NT			0x2	/* 0x20000000 or 0x08000000 */
#define TC_BR_HNT_DC			0x3	/* 0x30000000 or 0x0C000000 */

#define TC_BR_WTHR_START		23	/* The low bit for the field. */
#define TC_BR_WTHR_MASK			0x03800000
#define TC_BR_WTHR_NONE			0x0	/* Will be 0x00000000 */
#define TC_BR_WTHR_DPTK			0x1	/* Will be 0x00800000 */
#define TC_BR_WTHR_DPNT			0x2	/* Will be 0x01000000 */
#define TC_BR_WTHR_SPTK			0x3	/* Will be 0x01800000 */
#define TC_BR_WTHR_SPNT			0x4	/* Will be 0x02000000 */

#define TC_CMP_OP_START			18	/* The low bit for the field. */
#define TC_CMP_OP_MASK			0x007C0000
#define TC_CMP_OP_NONE			0x0	/* Will be 0x00000000 */
#define TC_CMP_OP_EQ			0x1	/* Will be 0x00040000 */
#define TC_CMP_OP_NE			0x2	/* Will be 0x00080000 */
#define TC_CMP_OP_LT			0x3	/* Will be 0x000C0000 */
#define TC_CMP_OP_LE			0x4	/* Will be 0x00100000 */
#define TC_CMP_OP_GT			0x5	/* Will be 0x00140000 */
#define TC_CMP_OP_GE			0x6	/* Will be 0x00180000 */
#define TC_CMP_OP_LTU			0x7	/* Will be 0x001C0000 */
#define TC_CMP_OP_LEU			0x8	/* Will be 0x00200000 */
#define TC_CMP_OP_GTU			0x9	/* Will be 0x00240000 */
#define TC_CMP_OP_GEU			0xA	/* Will be 0x00280000 */
#define TC_CMP_OP_NEQ			0xB	/* Will be 0x002C0000 */
#define TC_CMP_OP_NLT			0xC	/* Will be 0x00300000 */
#define TC_CMP_OP_NLE			0xD	/* Will be 0x00340000 */
#define TC_CMP_OP_NGT			0xE	/* Will be 0x00380000 */
#define TC_CMP_OP_NGE			0xF	/* Will be 0x003C0000 */
#define TC_CMP_OP_ORD			0x10	/* Will be 0x00400000 */
#define TC_CMP_OP_UNORD			0x11	/* Will be 0x00440000 */
#define TC_CMP_OP_Z                     0x12	/* Will be 0x00480000 */
#define TC_CMP_OP_NZ                    0x13	/* Will be 0x004C0000 */
#define TC_CMP_OP_M                     0x14	/* Will be 0x00500000 */
#define TC_CMP_OP_NM                    0x15	/* Will be 0x00540000 */

#define TC_CMP_TYPE_START		15	/* The low bit for the field. */
#define TC_CMP_TYPE_MASK		0x00038000
#define TC_CMP_TYPE_NONE		0x0	/* Will be 0x00000000 */
#define TC_CMP_TYPE_UNC			0x1	/* Will be 0x00008000 */
#define TC_CMP_TYPE_OR			0x2	/* Will be 0x00010000 */
#define TC_CMP_TYPE_AND			0x3	/* Will be 0x00018000 */
#define TC_CMP_TYPE_OR_ANDCM		0x4	/* Will be 0x00020000 */
#define TC_CMP_TYPE_ORCM		0x5	/* Will be 0x00028000 */
#define TC_CMP_TYPE_ANDCM		0x6	/* Will be 0x00030000 */
#define TC_CMP_TYPE_AND_ORCM		0x7	/* Will be 0x00038000 */

#define TC_LD_TYPE_START                13	/* The low bit for the field. */
#define TC_LD_TYPE_MASK                 0x00006000
#define TC_LD_TYPE_NONE                 0x0	/* Will be 0x00000000 */
#define TC_LD_TYPE_A                    0x1	/* Will be 0x00002000 */
#define TC_LD_TYPE_S                    0x2	/* Will be 0x00004000 */
#define TC_LD_TYPE_SA                   0x3	/* Will be 0x00006000 */

/* Macros to set and get the various fields.
 * f is the bitfield, and x is the completer to add.
 */
#define TC_SET_TEMPORAL_LOCALITY(f, x) \
            ((f)=(((f)&~TC_TEMPORAL_LOCALITY_MASK)|\
                  ((x)<<TC_TEMPORAL_LOCALITY_START)))
#define TC_GET_TEMPORAL_LOCALITY(f)    (((f) & TC_TEMPORAL_LOCALITY_MASK) \
                                                 >> TC_TEMPORAL_LOCALITY_START)
#define TC_SET_BR_HNT1(f, x)	((f)=(((f) & ~TC_BR_HNT1_MASK) | \
                                      ((x) << TC_BR_HNT1_START)))
#define TC_GET_BR_HNT1(f)	(((f) & TC_BR_HNT1_MASK) >> TC_BR_HNT1_START)
#define TC_SET_BR_HNT2(f, x)	((f)=(((f) & ~TC_BR_HNT2_MASK) | \
                                      ((x) << TC_BR_HNT2_START)))
#define TC_GET_BR_HNT2(f)	(((f) & TC_BR_HNT2_MASK) >> TC_BR_HNT2_START)
#define TC_SET_BR_WTHR(f, x)	((f)=(((f) & ~TC_BR_WTHR_MASK) | \
                                      ((x) << TC_BR_WTHR_START)))
#define TC_GET_BR_WTHR(f)	(((f) & TC_BR_WTHR_MASK) >> TC_BR_WTHR_START)
#define TC_SET_CMP_OP(f, x)	((f)=(((f) & ~TC_CMP_OP_MASK) | \
                                      ((x) << TC_CMP_OP_START)))
#define TC_GET_CMP_OP(f)	(((f) & TC_CMP_OP_MASK) >> TC_CMP_OP_START)
#define TC_SET_CMP_TYPE(f, x)	((f)=(((f) & ~TC_CMP_TYPE_MASK) | \
                                      ((x) << TC_CMP_TYPE_START)))
#define TC_GET_CMP_TYPE(f)	(((f) & TC_CMP_TYPE_MASK) >> TC_CMP_TYPE_START)
#define TC_SET_LD_TYPE(f, x)    ((f)=(((f) & ~TC_LD_TYPE_MASK) | \
                                      ((x) << TC_LD_TYPE_START)))
#define TC_GET_LD_TYPE(f)       (((f) & TC_LD_TYPE_MASK) >> TC_LD_TYPE_START)

/* LFETCH* uses TC_TEMPORAL_LOCALITY_NONE, NT1, NT2, and NTA */
#define TC_LFETCH_EXCL		0x00000001

/* LD*_C uses TC_TEMPORAL_LOCALITY_NONE, NT1, and NTA */
/* For LD*_C */
#define TC_LD_C_NC		0x00000001
#define TC_LD_C_CLR		0x00000002
#define TC_LD_C_CLR_ACQ		0x00000004	/* Not valid for floating point
						 * loads */

/* Floating point status register selectors.
 * Just store the number of the status register in the two low bits.
 */
#define TC_FP_STATUS_START      0              /* The low bit for the field. */
#define TC_FP_STATUS_MASK       0x00000003

#define TC_FP_STATUS_S0         0
#define TC_FP_STATUS_S1         1
#define TC_FP_STATUS_S2         2
#define TC_FP_STATUS_S3         3

#define TC_SET_FP_STATUS_REG(f, x)  ((f)=(((f) & ~TC_FP_STATUS_MASK) | \
                                          ((x) << TC_FP_STATUS_START)))
#define TC_GET_FP_STATUS_REG(f)     (((f) & TC_FP_STATUS_MASK) >> TC_FP_STATUS_START)

/* For FCVT* */
#define TC_FCVT_TRUNC		0x00000010

/* MOV_TOBR uses TC_BR_WTHR_DPTK, SPTK, TC_BR_HNT1, and TC_BR_HNT2 */
#define TC_MOV_TOBR_IMP	        0x00000001
#define TC_MOV_TOBR_RET	        0x00000002
#define TC_MOV_TOBR_MANY	0x00000004

/* CMP uses TC_CMP_TYPE and TC_CMP_OP. */
#define TC_CMP_4                0x00000001

/* BR* (BR_CALL, BR_COND, BR_IA, BR_RET, BRL_CALL, BRL_COND, BR_CEXIT,
 * BR_CTOP, BR_WEXIT, BR_WTOP) use TC_BR_WTHR_DPTK, DPNT, SPTK, SPNT.
 */
#define TC_BR_NONE              0x00000000
#define TC_BR_FEW		0x00000001
#define TC_BR_MANY		0x00000002
#define TC_BR_FEW_MANY_MASK     0x00000003
#define TC_BR_CLR		0x00000004

/* BRP and BRP_RET use TC_BR_WTHR_SPTK, DPTK, TC_BR_HNT1, HNT2.  LOOP and
 * EXIT are only valid for BRP. */
#define TC_BRP_LOOP		0x00000001
#define TC_BRP_EXIT		0x00000002
#define TC_BRP_LOOP_EXIT_MASK   0x00000003
#define TC_BRP_MANY		0x00000010
#define TC_BRP_IMP		0x00000020
#define TC_BRP_MANY_IMP_MASK    0x00000030

/* Functions to print the standard completers. */
void print_standard_completers (L_Oper * op);
void print_branch_whether_completer (L_Oper * op);
void print_load_type_completer (L_Oper * op);
void print_comparison_op_completer (L_Oper * op);
void print_comparison_type_completer (L_Oper * op);
void print_branch_hint_completer (L_Oper * op, int whichHint);
void print_temporal_locality_completer (L_Oper * op);

/* Function to print completers specific to single opcodes. */
void print_LFETCH_completers (L_Oper * op);
void print_LD_C_completers (L_Oper * op);
void print_LDF_C_completers (L_Oper * op);
void print_CHK_A_completers (L_Oper * op);
void print_FP_S_REG_completers (L_Oper * op);
void print_MOV_TOBR_completers (L_Oper * op);
void print_CMP_completers (L_Oper * op);
void print_BR_completers (L_Oper * op);
void print_BRP_completers (L_Oper * op);
void print_FCMP_completers (L_Oper * op);
void print_FCVT_completers (L_Oper * op);
void print_FPCMP_completers (L_Oper * op);

/* Sanity checking functions for various opcodes.  These function check
 * to make sure that the oper only has valid bits set in the completers
 * field.
 */
int check_branch_hints (L_Oper * op);
int check_LD (L_Oper * op);
int check_ACQ_REL_BIAS_FILL (L_Oper * op);
int check_LDF (L_Oper * op);
int check_LDF_A (L_Oper * op);
int check_MOV_TOBR (L_Oper * op);
int check_CMP (L_Oper * op);
int check_TBIT_TNAT (L_Oper * op);
int check_BRP (L_Oper * op);
int check_BRP_RET (L_Oper * op);
int check_FCLASS (L_Oper * op);
int check_FCMP (L_Oper * op);
int check_FPCMP (L_Oper * op);
int check_ST (L_Oper * op);

#endif
