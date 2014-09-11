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
/*****************************************************************************
 * phase1_opgen.c                                                            *
 * ------------------------------------------------------------------------- *
 * Routines to generate sub-operations used in integer division and          *
 * remainder operations                                                      *
 *                                                                           *
 * AUTHORS: J.W. Sias, C.J. Shannon                                          *
 *****************************************************************************/

/* 09/12/02 REK Updating file to support the new opcode map. */

#ifndef PHASE1_OPGEN_H
#define PHASE1_OPGEN_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

extern void Ltahoe_annotate_EM_int_divide8_thr (L_Cb * cb, L_Oper * oper);
extern void Ltahoe_annotate_EM_int_divide16_thr (L_Cb * cb, L_Oper * oper);
extern void Ltahoe_annotate_EM_int_divide32 (L_Cb * cb, L_Oper * oper);
extern void Ltahoe_annotate_EM_int_divide64_lat (L_Cb * cb, L_Oper * oper);
extern void Ltahoe_annotate_EM_int_divide64_thr (L_Cb * cb, L_Oper * oper);

extern L_Oper *Ltahoe_new_fma (L_Operand * pred, L_Operand * dest,
			       L_Operand * src0, L_Operand * src1,
			       L_Operand * src2, int fsf, int fpc,
			       L_Oper * using);

extern L_Oper *Ltahoe_new_fnma (L_Operand * pred, L_Operand * dest,
				L_Operand * src0, L_Operand * src1,
				L_Operand * src2, int fsf, int fpc,
				L_Oper * using);

extern L_Oper *Ltahoe_new_fadd (L_Operand * pred, L_Operand * dest,
				L_Operand * src0, L_Operand * src1,
				int fsf, int fpc, L_Oper * using);

extern L_Oper *Ltahoe_new_frcpa (L_Operand * pred,
				 L_Operand * dest0, L_Operand * dest1,
				 L_Operand * src0, L_Operand * src1, int fsf,
				 L_Oper * using);

extern L_Oper *Ltahoe_new_fcvt_fx_trunc (L_Operand * pred, L_Operand * dest0,
					 L_Operand * src0, int fsf,
					 L_Oper * using);

extern L_Oper *Ltahoe_new_getf_sig (L_Operand * pred, L_Operand * dest0,
				    L_Operand * src0, L_Oper * using);

extern L_Oper *Ltahoe_new_movi (L_Operand * pred, L_Operand * dest0,
				ITint64 value, L_Oper * using);

extern L_Oper *Ltahoe_new_movl (L_Operand * pred, L_Operand * dest0,
				ITint64 value, L_Oper * using);

extern L_Oper *Ltahoe_new_setf_exp (L_Operand * pred, L_Operand * dest0,
				    L_Operand * src0, L_Oper * using);

extern L_Oper *Ltahoe_new_setf_s (L_Operand * pred, L_Operand * dest0,
				  L_Operand * src0, L_Oper * using);

extern L_Oper *Ltahoe_new_setf_sig (L_Operand * pred, L_Operand * dest0,
				    L_Operand * src0, L_Oper * using);

extern L_Oper *Ltahoe_new_xma_l (L_Operand * pred, L_Operand * dest,
				 L_Operand * src0, L_Operand * src1,
				 L_Operand * src2, L_Oper * using);

extern int Ltahoe_get_fsf (L_Oper * oper);
extern void Ltahoe_set_fsf (L_Oper * oper, int fsf);
extern int Ltahoe_get_fpc (L_Oper * oper);
extern void Ltahoe_set_fpc (L_Oper * oper, int fpc);

/* 09/12/02 REK These enums were originally in tmdes_instr.h.  I'm bringing
 *              them here for backward compatibility. */
enum
{ FPC_NONE = 0, FPC_S, FPC_D };
enum
{ FSF_S0 = 0, FSF_S1, FSF_S2, FSF_S3 };

#endif
