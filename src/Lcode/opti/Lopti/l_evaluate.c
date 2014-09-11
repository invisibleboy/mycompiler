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
 *      File :          l_evaluate.c
 *                      (formerly part of l_opti_functions.c)
 *      Description :   data struct manipulation functs for optimizer
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Chang.
 *
 *      (C) Copyright 1990, Pohua Chang, Scott Mahlke.
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

#include <config.h>
#include "l_opti.h"

#define ERR stderr

/*=========================================================================*/
/*
 *      Evaluation of opers with constant operands
 */
/*=========================================================================*/

/* ITI/JWJ 8.12.1999
 * Added cases for Lop_ADD_SAT, ADD_SAT_U, SUB_SAT, SUB_SAT_U,
 *                 MUL_SAT, MUL_SAT_U, SAT, SAT_U, L_MAC, L_MSU.
 * Warning: these cases are not perfect evaluators.
 */

ITintmax
L_evaluate_int_arithmetic (L_Oper * oper)
{
  if (!oper)
    L_punt ("L_evaluate_int_arithmetic: oper is NULL");
  switch (oper->opc)
    {
    case Lop_L_MAC:
      {
	ITintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) ||
	    (!L_is_int_constant (src2)) || (!L_is_int_constant (src3)))
	  L_punt ("L_evaluate_int_arithmetic L_MAC: "
		  "src1, src2 or src3 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (s3 + s1 * s2);
      }
    case Lop_L_MSU:
      {
	ITintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) ||
	    (!L_is_int_constant (src2)) || (!L_is_int_constant (src3)))
	  L_punt ("L_evaluate_int_arithmetic L_MSU: "
		  "src1, src2 or src3 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (s3 - s1 * s2);
      }
    case Lop_SAT:
    case Lop_SAT_U:
      {
	if (!L_is_int_constant (oper->src[0]))
	  L_punt ("L_evaluate_int_arithmetic SAT: src1 is not an int");
	return (oper->src[0]->value.i);
      }
    case Lop_ADD_SAT:
    case Lop_ADD:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic ADD: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 + s2);
      }
    case Lop_ADD_SAT_U:
    case Lop_ADD_U:
      {
	ITuintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic ADDU:src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 + s2);
      }
    case Lop_SUB_SAT:
    case Lop_SUB:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic SUB: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 - s2);
      }
    case Lop_SUB_SAT_U:
    case Lop_SUB_U:
      {
	ITuintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic SUBU:src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 - s2);
      }
    case Lop_MUL_SAT:
    case Lop_MUL:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic MUL: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 * s2);
      }
    case Lop_MUL_SAT_U:
    case Lop_MUL_U:
      {
	ITuintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic MULU:src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 * s2);
      }
    case Lop_DIV:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic DIV: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 / s2);
      }
    case Lop_DIV_U:
      {
	ITuintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic DIVU:src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 / s2);
      }
    case Lop_REM:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic REM: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 % s2);
      }
    case Lop_REM_U:
      {
	ITuintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic REMU:src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 % s2);
      }
    case Lop_ABS:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt
	    ("L_evaluate_int_arithmetic ABS: src1 is not an int");
	s1 = src1->value.i;
	if (s1 < 0)
	  return (-s1);
	return (s1);
      }
    case Lop_MUL_ADD:
      {
	ITintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)) ||
	    (!L_is_int_constant (src3)))
	  L_punt
	    ("L_evaluate_int_arithmetic MUL_ADD: src1,2,3 must all be ints");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (s1 * s2 + s3);
      }
    case Lop_MUL_ADD_U:
      {
	ITuintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)) ||
	    (!L_is_int_constant (src3)))
	  L_punt
	    ("L_evaluate_int_arithmetic MUL_ADD_U: src1,2,3 must all be ints");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (s1 * s2 + s3);
      }
    case Lop_MUL_SUB:
      {
	ITintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)) ||
	    (!L_is_int_constant (src3)))
	  L_punt
	    ("L_evaluate_int_arithmetic MUL_SUB: src1,2,3 must all be ints");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (s1 * s2 - s3);
      }
    case Lop_MUL_SUB_U:
      {
	ITuintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)) ||
	    (!L_is_int_constant (src3)))
	  L_punt
	    ("L_evaluate_int_arithmetic MUL_SUB_U: src1,2,3 must all be ints");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (s1 * s2 - s3);
      }
    case Lop_MUL_SUB_REV:
      {
	ITintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)) ||
	    (!L_is_int_constant (src3)))
	  L_punt ("L_evaluate_int_arithmetic MUL_SUB_REV: "
		  "src1,2,3 must all be ints");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (-(s1 * s2) + s3);
      }
    case Lop_MUL_SUB_REV_U:
      {
	ITuintmax s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)) ||
	    (!L_is_int_constant (src3)))
	  L_punt ("L_evaluate_int_arithmetic MUL_SUB_REV_U: "
		  "src1,2,3 must all be ints");
	s1 = src1->value.i;
	s2 = src2->value.i;
	s3 = src3->value.i;
	return (-(s1 * s2) + s3);
      }
    case Lop_OR:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt ("L_evaluate_int_arithmetic OR: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 | s2);
      }
    case Lop_AND:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic AND: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 & s2);
      }
    case Lop_XOR:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic XOR: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 ^ s2);
      }
    case Lop_NOR:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic NOR: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (~(s1 | s2));
      }
    case Lop_NAND:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic NAND: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (~(s1 & s2));
      }
    case Lop_NXOR:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic NXOR: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (~(s1 ^ s2));
      }
    case Lop_OR_NOT:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt ("L_evaluate_int_arithmetic OR_NOT: s1 or s2 not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 | (!s2));
      }
    case Lop_AND_NOT:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt ("L_evaluate_int_arithmetic AND_NOT: s1 or s2 not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 & (!s2));
      }
    case Lop_OR_COMPL:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt ("L_evaluate_int_arithmetic OR_COMPL: s1 or s2 not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 | (~s2));
      }
    case Lop_AND_COMPL:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt ("L_evaluate_int_arithmetic AND_COMPL: s1 or s2 not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 & (~s2));
      }
    case Lop_RCMP:
    case Lop_RCMP_F:
      return L_evaluate_compare (oper);
    case Lop_LSL:
      {
	ITuintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic LSL: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 << s2);
      }
    case Lop_LSR:
      {
	ITuintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic LSR: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	return (s1 >> s2);
      }
    case Lop_ASR:
      {
	ITintmax s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_int_constant (src1)) || (!L_is_int_constant (src2)))
	  L_punt
	    ("L_evaluate_int_arithmetic ASR: src1 or src2 is not an int");
	s1 = src1->value.i;
	s2 = src2->value.i;
	/* Impact bug fix: without this explicit cast, constant folding is  *
	 * incorrect.            <lieberm 07/15/2006>                       */
	return ((int)s1 >> (int)s2);
	/* return (s1 >> s2); */
      }
    case Lop_EXTRACT_C:
    case Lop_SXT_C:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic SXT_C: src1 is not an int");
	s1 = src1->value.i;
	return ((ITint8) s1);
      }
    case Lop_ZXT_C:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic ZXT_C: src1 is not an int");
	s1 = src1->value.i;
	return ((ITuint8) s1);
      }
    case Lop_EXTRACT_C2:
    case Lop_SXT_C2:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic SXT_C2: src1 is not an int");
	s1 = src1->value.i;
	return ((ITint16) s1);
      }
    case Lop_ZXT_C2:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic ZXT_C2: src1 is not an int");
	s1 = src1->value.i;
	return ((ITuint16) s1);
      }
    case Lop_SXT_I:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic SXT_I: src1 is not an int");
	s1 = src1->value.i;
	return ((ITint32) s1);
      }
    case Lop_ZXT_I:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic ZXT_I: src1 is not an int");
	s1 = src1->value.i;
	return ((ITuint32) s1);
      }
    case Lop_F2_I:
      {
	double s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_dbl_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic F2_I: src1 is not a double");
	s1 = src1->value.f2;
	return ((ITintmax) s1);
      }
    case Lop_F_I:
      {
	float s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_flt_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic F_I: src1 is not a float");
	s1 = src1->value.f;
	return ((ITintmax) s1);
      }
    case Lop_EXTRACT:
    case Lop_EXTRACT_U:
      {
	L_Operand *src0 = oper->src[0], 
	  *src1 = oper->src[1],
	  *src2 = oper->src[2];
	ITintmax d0, s0, s1, s2;

	if (!L_is_int_constant (src0) ||
	    !L_is_int_constant (src1) ||
	    !L_is_int_constant (src2))
	  L_punt ("L_evaluate_int_arithmetic EXTRACT: src not an int");

	s0 = src0->value.i;
	s1 = src1->value.i;
	s2 = src2->value.i;

	d0 = (s0 >> s1) & ((1LL << (s2)) - 1);

	if ((oper->opc == Lop_EXTRACT) &&
	    (s0 & (1LL << (s1 + s2 - 1))))
	  d0 |= (((ITintmax) -1) << s2);

	return d0;
      }
    case Lop_ADD_CARRY:
    case Lop_ADD_CARRY_U:
    case Lop_SUB_CARRY:
    case Lop_SUB_CARRY_U:
    case Lop_MUL_WIDE:
    case Lop_MUL_WIDE_U:
      {
	L_Operand *src0 = oper->src[0], 
	  *src1 = oper->src[1];
	ITintmax d0, s0, s1;

	if (!L_is_int_constant (src0) ||
	    !L_is_int_constant (src1))
	  L_punt ("L_evaluate_int_arithmetic : src not an int");
	s0 = src0->value.i;
	s1 = src1->value.i;
        L_punt("L_evaluate_int_arithmetic : not handling the new opcodes");
        return 0;
      }
    case Lop_DEPOSIT:
    default:
      L_punt ("L_evaluate_int_arithmetic: improper opcode sent");
      return (0);
    }
}


float
L_evaluate_flt_arithmetic (L_Oper * oper)
{
  switch (oper->opc)
    {
    case Lop_ADD_F:
      {
	float s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)))
	  L_punt ("L_evaluate_flt_arith ADD_F: src1 or src2 is not a flt");
	s1 = src1->value.f;
	s2 = src2->value.f;
	return (s1 + s2);
      }
    case Lop_SUB_F:
      {
	float s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)))
	  L_punt ("L_evaluate_flt_arith SUB_F: src1 or src2 is not a flt");
	s1 = src1->value.f;
	s2 = src2->value.f;
	return (s1 - s2);
      }
    case Lop_MUL_F:
      {
	float s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)))
	  L_punt ("L_evaluate_flt_arith MUL_F: src1 or src2 is not a flt");
	s1 = src1->value.f;
	s2 = src2->value.f;
	return (s1 * s2);
      }
    case Lop_DIV_F:
      {
	float s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)))
	  L_punt ("L_evaluate_flt_arith DIV_F:  src1 or src2 is not a flt");
	s1 = src1->value.f;
	s2 = src2->value.f;
	return (s1 / s2);
      }
    case Lop_ABS_F:
      {
	float s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_flt_constant (src1))
	  L_punt ("L_evaluate_flt_arith ABS_F:s1 is not correct type");
	s1 = src1->value.f;
	if (s1 < 0)
	  return (-s1);
	return (s1);
      }
    case Lop_MUL_ADD_F:
      {
	float s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)) |
	    (!L_is_flt_constant (src3)))
	  L_punt
	    ("L_evaluate_flt_arith MUL_ADD_F: src1,2,3 must all be flts");
	s1 = src1->value.f;
	s2 = src2->value.f;
	s3 = src3->value.f;
	return (s1 * s2 + s3);
      }
    case Lop_MUL_SUB_F:
      {
	float s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)) |
	    (!L_is_flt_constant (src3)))
	  L_punt
	    ("L_evaluate_flt_arith MUL_SUB_F: src1,2,3 must all be flts");
	s1 = src1->value.f;
	s2 = src2->value.f;
	s3 = src3->value.f;
	return (s1 * s2 - s3);
      }
    case Lop_MUL_SUB_REV_F:
      {
	float s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)) |
	    (!L_is_flt_constant (src3)))
	  L_punt
	    ("L_evaluate_flt_arith MUL_SUB_REV_F: src1,2,3 must all be flts");
	s1 = src1->value.f;
	s2 = src2->value.f;
	s3 = src3->value.f;
	return (-(s1 * s2) + s3);
      }
    case Lop_F2_F:
      {
	float s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_dbl_constant (src1))
	  L_punt ("L_evaluate_flt_arithmetic F2_F: src1 is not a double");
	s1 = src1->value.f2;
	return ((float) s1);
      }
    case Lop_I_F:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_flt_arithmetic F2_F: src1 is not a integer");
	s1 = src1->value.i;
	switch (oper->com[0])
	{
	  case L_CTYPE_UINT:
	    return ((float) (unsigned int) s1);
	  case L_CTYPE_ULONG:
	    return ((float) (unsigned long) s1);
	  case L_CTYPE_ULLONG:
	    return ((float) (unsigned long long) s1);
	  default:
	    return ((float) s1);
	}
      }
    default:
      L_punt ("L_evaluate_double_arithmetic: improper opcode sent");
      return (0);
    }
}


double
L_evaluate_dbl_arithmetic (L_Oper * oper)
{
  switch (oper->opc)
    {
    case Lop_ADD_F2:
      {
	double s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)))
	  L_punt ("L_evaluate_dbl_arith ADD_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	return (s1 + s2);
      }
    case Lop_SUB_F2:
      {
	double s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)))
	  L_punt ("L_evaluate_dbl_arith SUB_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	return (s1 - s2);
      }
    case Lop_MUL_F2:
      {
	double s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)))
	  L_punt ("L_evaluate_dbl_arith MUL_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	return (s1 * s2);
      }
    case Lop_DIV_F2:
      {
	double s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)))
	  L_punt ("L_evaluate_dbl_arith DIV_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	return (s1 / s2);
      }
    case Lop_ABS_F2:
      {
	double s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_dbl_constant (src1))
	  L_punt ("L_evaluate_dbl_arith ABS_F2: src1 is not dble");
	s1 = src1->value.f2;
	if (s1 < 0)
	  return (-s1);
	return (s1);
      }
    case Lop_MUL_ADD_F2:
      {
	double s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)) ||
	    (!L_is_dbl_constant (src3)))
	  L_punt
	    ("L_evaluate_dbl_arith MUL_ADD_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	s3 = src3->value.f2;
	return (s1 * s2 + s3);
      }
    case Lop_MUL_SUB_F2:
      {
	double s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)) |
	    (!L_is_dbl_constant (src3)))
	  L_punt
	    ("L_evaluate_dbl_arith MUL_SUB_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	s3 = src3->value.f2;
	return (s1 * s2 - s3);
      }
    case Lop_MUL_SUB_REV_F2:
      {
	double s1, s2, s3;
	L_Operand *src1, *src2, *src3;
	src1 = oper->src[0];
	src2 = oper->src[1];
	src3 = oper->src[2];
	if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)) ||
	    (!L_is_dbl_constant (src3)))
	  L_punt
	    ("L_evaluate_dbl_arith MUL_SUB_REV_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	s3 = src3->value.f2;
	return (-(s1 * s2) + s3);
      }
    case Lop_I_F2:
      {
	ITintmax s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_int_constant (src1))
	  L_punt ("L_evaluate_dbl_arithmetic I_F2: src1 is not a integer");
	s1 = src1->value.i;
	switch (oper->com[0])
	{
	  case L_CTYPE_UINT:
	    return ((double) (unsigned int) s1);
	  case L_CTYPE_ULONG:
	    return ((double) (unsigned long) s1);
	  case L_CTYPE_ULLONG:
	    return ((double) (unsigned long long) s1);
	  default:
	    return ((double) s1);
	}
      }
    case Lop_F_F2:
      {
	double s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_flt_constant (src1))
	  L_punt ("L_evaluate_dbl_arithmetic F_F2: src1 is not a float");
	s1 = src1->value.f;
	return ((double) s1);
      }
    case Lop_SQRT_F2:
      {
	double s1;
	L_Operand *src1;
	src1 = oper->src[0];
	if (!L_is_dbl_constant (src1))
	  L_punt ("L_evaluate_dbl_arith SQRT_F2: src1 is not dble");
	s1 = src1->value.f2;
	s1 = sqrt(s1);
	return (s1);
      }
    case Lop_MIN_F2:
      {
	double s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if (!L_is_dbl_constant (src1) || !L_is_dbl_constant (src2))
	  L_punt ("L_evaluate_dbl_arith MIN_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	s1 = (s1 < s2) ? s1 : s2;
	return (s1);
      }
    case Lop_MAX_F2:
      {
	double s1, s2;
	L_Operand *src1, *src2;
	src1 = oper->src[0];
	src2 = oper->src[1];
	if (!L_is_dbl_constant (src1) || !L_is_dbl_constant (src2))
	  L_punt ("L_evaluate_dbl_arith MAX_F2: src1 or src2 is not dble");
	s1 = src1->value.f2;
	s2 = src2->value.f2;
	s1 = (s1 > s2) ? s1 : s2;
	return (s1);
      }
    default:
      L_punt ("L_evaluate_double_arithmetic: improper opcode sent");
      return (0);
    }
}


int
L_evaluate_int_compare_with_sources (L_Oper * oper, ITintmax s1, ITintmax s2)
{
  int uns;

  uns = L_is_ctype_signed_direct (oper->com[0]) ? 0 : 1;

  if (oper->com[0] == L_CTYPE_INT)
    {
      s1 = (int) s1;
      s2 = (int) s2;
    }
  else if (oper->com[0] == L_CTYPE_UINT)
    {
      s1 = (unsigned int) s1;
      s2 = (unsigned int) s2;
    }

  switch (oper->com[1])
    {
    case Lcmp_COM_EQ:
      return (s1 == s2);
    case Lcmp_COM_NE:
      return (s1 != s2);
    case Lcmp_COM_GT:
      if (!uns)
	return (s1 > s2);
      else
	return ((ITuintmax) s1 > (ITuintmax) s2);
    case Lcmp_COM_GE:
      if (!uns)
	return (s1 >= s2);
      else
	return ((ITuintmax) s1 >= (ITuintmax) s2);
    case Lcmp_COM_LT:
      if (!uns)
	return (s1 < s2);
      else
	return ((ITuintmax) s1 < (ITuintmax) s2);
    case Lcmp_COM_LE:
      if (!uns)
	return (s1 <= s2);
      else
	return ((ITuintmax) s1 <= (ITuintmax) s2);
    default:
      L_punt ("L_evaluate_int_compare_with_sources - "
	      "unknown compare %d\n", oper->id);
    }
  return -1;
}


int
L_evaluate_int_compare (L_Oper * oper)
{
  ITintmax s1, s2;
  L_Operand *src1, *src2;

  src1 = oper->src[0];
  src2 = oper->src[1];

  if ((!L_is_constant (src1)) || (!L_is_constant (src2)))
    L_punt ("L_evaluate_int_compare : src1 or src2 is not constant");

  
  s1 = src1->value.i;
  s2 = src2->value.i;

  return L_evaluate_int_compare_with_sources (oper, s1, s2);
}


int
L_evaluate_flt_compare_with_sources (L_Oper * oper, float s1, float s2)
{
  switch (oper->com[1])
    {
    case Lcmp_COM_EQ:
      return (s1 == s2);
    case Lcmp_COM_NE:
      return (s1 != s2);
    case Lcmp_COM_GT:
      return (s1 > s2);
    case Lcmp_COM_GE:
      return (s1 >= s2);
    case Lcmp_COM_LT:
      return (s1 < s2);
    case Lcmp_COM_LE:
      return (s1 <= s2);
    default:
      L_punt ("L_evaluate_flt_compare_with_sources - "
	      "unknown compare %d\n", oper->id);
    }
  return -1;
}


int
L_evaluate_flt_compare (L_Oper * oper)
{
  float s1, s2;
  L_Operand *src1, *src2;
  src1 = oper->src[0];
  src2 = oper->src[1];

  if ((!L_is_flt_constant (src1)) || (!L_is_flt_constant (src2)))
    L_punt ("L_evaluate_flt_cond_br: src1 or src2 is not float");

  s1 = src1->value.f;
  s2 = src2->value.f;

  return (L_evaluate_flt_compare_with_sources (oper, s1, s2));
}


int
L_evaluate_dbl_compare_with_sources (L_Oper * oper, double s1, double s2)
{
  switch (oper->com[1])
    {
    case Lcmp_COM_EQ:
      return (s1 == s2);
    case Lcmp_COM_NE:
      return (s1 != s2);
    case Lcmp_COM_GT:
      return (s1 > s2);
    case Lcmp_COM_GE:
      return (s1 >= s2);
    case Lcmp_COM_LT:
      return (s1 < s2);
    case Lcmp_COM_LE:
      return (s1 <= s2);
    default:
      L_punt ("L_evaluate_dbl_compare_with_sources - "
	      "unknown branch opc %d\n", oper->id);
    }
  return -1;
}


int
L_evaluate_dbl_compare (L_Oper * oper)
{
  double s1, s2;
  L_Operand *src1, *src2;
  src1 = oper->src[0];
  src2 = oper->src[1];

  if ((!L_is_dbl_constant (src1)) || (!L_is_dbl_constant (src2)))
    L_punt ("L_evaluate_dbl_cond_br: src1 or src2 is not double");

  s1 = src1->value.f2;
  s2 = src2->value.f2;

  return (L_evaluate_dbl_compare_with_sources (oper, s1, s2));
}


int
L_evaluate_compare (L_Oper * oper)
{
  if ((oper->opc != Lop_BR) &&
      (oper->opc != Lop_BR_F) &&
      (oper->opc != Lop_RCMP) &&
      (oper->opc != Lop_RCMP_F) &&
      (oper->opc != Lop_CMP) && (oper->opc != Lop_CMP_F))
    L_punt ("L_evaluate_compare - unknown compare opc %d\n", oper->id);

  if (!L_operand_ctype_same (oper->src[0], oper->src[1]))
    {
      L_print_oper (stdout, oper);
      L_punt ("L_evaluate_compare: op types must be equal");
    }

  switch (L_operand_case_ctype (oper->src[0]))
    {
    case L_CTYPE_INT:
    case L_CTYPE_LLONG:
      return (L_evaluate_int_compare (oper));
    case L_CTYPE_FLOAT:
      return (L_evaluate_flt_compare (oper));
    case L_CTYPE_DOUBLE:
      return (L_evaluate_dbl_compare (oper));
    default:
      L_punt ("L_evaluate_compare: illegal ctype");
      return (0);
    }
}
