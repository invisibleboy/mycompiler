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
 *  File: l_ip_resolve.c
 *
 *  Description:  
 *	This files contains routines that are used to resolve unknown operands
 *	by traversing use-def chains.
 *
 *  Creation Date :  August, 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *
 *  Copyright (c) 1994 Roger A. Bringmann , Wen-mei Hwu and The Board of
 *		  Trustees of the University of Illinois. 
 *		  All rights reserved.
 *
 *  The University of Illinois software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_interproc.h"

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

Resolved *L_rs_new_resolved()
{
    Resolved *resolved;
    
    resolved = L_alloc(L_alloc_resolved);

    resolved->first_reg = NULL;
    resolved->first_cell = NULL;

    return resolved;
}

void L_rs_delete_resolved(Resolved *resolved)
{
   Reg 		*reg, *next_reg;
   Memory_Cell	*cell, *next_cell;

   for (reg = resolved->first_reg; reg != NULL; reg = next_reg)
   {
       next_reg = reg->next_reg;
       L_rf_delete_reg(reg);
   }

   for (cell = resolved->first_cell; cell != NULL; cell = next_cell)
   {
       next_cell = cell->next_cell;
       L_mem_delete_cell(cell);
   }

   L_free(L_alloc_resolved, resolved);
}

/******************************************************************************\
 *
 * Routines used to evaluate arithmetic expressions
 *
\******************************************************************************/

int L_rs_perform_int_arith(int opc, L_Operand *src0, L_Operand *src1,
    L_Operand *src2)
{
    switch (opc) {
    case Lop_ADD: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
           L_punt("ADD: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1+s2);
    }
    case Lop_ADD_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("ADDU:src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1+s2);
    }
    case Lop_SUB: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("SUB: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1-s2);
    }
    case Lop_SUB_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("SUBU:src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1-s2);
    }
    case Lop_MUL: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("MUL: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1*s2);
    }
    case Lop_MUL_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("MULU:src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1*s2);
    }
    case Lop_DIV: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("DIV: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1/s2);
    }
    case Lop_DIV_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("DIVU:src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1/s2);
    }
    case Lop_REM: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("REM: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1%s2);
    }
    case Lop_REM_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("REMU:src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1%s2);
    }
    case Lop_ABS: {
        int s1, s2;
        int is_neg, mag;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("ABS: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        is_neg = (s1<0);
        if (s2<0)
            mag = -s2;
        else
            mag = s2;
        if (is_neg)
            return (-mag);
        return mag;
    }
    case Lop_MUL_ADD: {
        int s1, s2, s3;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)) |
	    (!L_is_int_constant(src2)))
            L_punt("MUL_ADD: src0,2,3 must all be ints");
        s1 = src0->value.i;
        s2 = src1->value.i;
	s3 = src2->value.i;
        return (s1*s2+s3);
    }
    case Lop_MUL_ADD_U: {
        unsigned int s1, s2, s3;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)) |
	    (!L_is_int_constant(src2)))
            L_punt("MUL_ADD_U: src0,2,3 must all be ints");
        s1 = src0->value.i;
        s2 = src1->value.i;
        s3 = src2->value.i;
        return (s1*s2+s3);
    }
    case Lop_MUL_SUB: {
        int s1, s2, s3;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)) |
	    (!L_is_int_constant(src2)))
            L_punt("MUL_SUB: src0,2,3 must all be ints");
        s1 = src0->value.i;
        s2 = src1->value.i;
        s3 = src2->value.i;
        return (s1*s2-s3);
    }
    case Lop_MUL_SUB_U: {
        unsigned int s1, s2, s3;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)) |
	    (!L_is_int_constant(src2)))
            L_punt("MUL_SUB_U: src0,2,3 must all be ints");
        s1 = src0->value.i;
        s2 = src1->value.i;
        s3 = src2->value.i;
        return (s1*s2-s3);
    }
    case Lop_MUL_SUB_REV: {
        int s1, s2, s3;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)) |
	    (!L_is_int_constant(src2)))
            L_punt("MUL_SUB_REV: src0,2,3 must all be ints");
        s1 = src0->value.i;
        s2 = src1->value.i;
        s3 = src2->value.i;
        return (-(s1*s2)+s3);
    }
    case Lop_MUL_SUB_REV_U: {
        unsigned int s1, s2, s3;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)) |
	    (!L_is_int_constant(src2)))
            L_punt("MUL_SUB_REV_U: src0,2,3 must all be ints");
        s1 = src0->value.i;
        s2 = src1->value.i;
        s3 = src2->value.i;
        return (-(s1*s2)+s3);
    }
    case Lop_OR: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("OR: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1|s2);
    }
    case Lop_AND: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("AND: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1&s2);
    }
    case Lop_XOR: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("XOR: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1^s2);
    }
    case Lop_NOR: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("NOR: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (~(s1|s2));
    }
    case Lop_NAND: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("NAND: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (~(s1&s2));
    }
    case Lop_NXOR: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("NXOR: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (~(s1^s2));
    }
    case Lop_OR_NOT: {
	int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
	    L_punt("OR_NOT: s1 or s2 not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1 | (!s2));
    }
    case Lop_AND_NOT: {
	int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
	    L_punt("AND_NOT: s1 or s2 not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1 & (!s2));
    }
    case Lop_EQ: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("EQ: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1==s2);
    }
    case Lop_NE: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("NE: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1!=s2);
    }
    case Lop_GT: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("GT: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1>s2);
    }
    case Lop_GT_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("GTU: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1>s2);
    }
    case Lop_GE: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("GE: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1>=s2);
    }
    case Lop_GE_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("GEU: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1>=s2);
    }
    case Lop_LT: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("LT: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1<s2);
    }
    case Lop_LT_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("LTU: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1<s2);
    }
    case Lop_LE: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("LE: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1<=s2);
    }
    case Lop_LE_U: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("LEU: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1<=s2);
    }
    case Lop_LSL: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("LSL: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1<<s2);
    }
    case Lop_LSR: {
        unsigned int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("LSR: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1>>s2);
    }
    case Lop_ASR: {
        int s1, s2;
	if ((!L_is_int_constant(src0)) | (!L_is_int_constant(src1)))
            L_punt("ASR: src0 or src1 is not an int");
        s1 = src0->value.i;
        s2 = src1->value.i;
        return (s1>>s2);
    }
    case Lop_EQ_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src0)) | (!L_is_dbl_constant(src1)))
            L_punt("EQ_F2:src0 or src1 is not a dbl");
        s1 = src0->value.f2;
        s2 = src1->value.f2;
        return (s1==s2);
    }
    case Lop_NE_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src0)) | (!L_is_dbl_constant(src1)))
            L_punt("NE_F2:src0 or src1 is not a dbl");
        s1 = src0->value.f2;
        s2 = src1->value.f2;
        return (s1!=s2);
    }
    case Lop_GT_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src0)) | (!L_is_dbl_constant(src1)))
            L_punt("GT_F2:src0 or src1 is not a dbl");
        s1 = src0->value.f2;
        s2 = src1->value.f2;
        return (s1>s2);
    }
    case Lop_GE_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src0)) | (!L_is_dbl_constant(src1)))
            L_punt("GE_F2:src0 or src1 is not a dbl");
        s1 = src0->value.f2;
        s2 = src1->value.f2;
        return (s1>=s2);
    }
    case Lop_LT_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src0)) | (!L_is_dbl_constant(src1)))
            L_punt("LT_F2:src0 or src1 is not a dbl");
        s1 = src0->value.f2;
        s2 = src1->value.f2;
        return (s1<s2);
    }
    case Lop_LE_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src0)) | (!L_is_dbl_constant(src1)))
            L_punt("LE_F2:src0 or src1 is not a dbl");
        s1 = src0->value.f2;
        s2 = src1->value.f2;
        return (s1<=s2);
    }
    case Lop_EQ_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src0)) | (!L_is_flt_constant(src1)))
            L_punt("EQ_F:src0 or src1 is not a flt");
        s1 = src0->value.f;
        s2 = src1->value.f;
        return (s1==s2);
    }
    case Lop_NE_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src0)) | (!L_is_flt_constant(src1)))
            L_punt("NE_F: src0 or src1 is not a flt");
        s1 = src0->value.f;
        s2 = src1->value.f;
        return (s1!=s2);
    }
    case Lop_GT_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src0)) | (!L_is_flt_constant(src1)))
            L_punt("GT_F: src0 or src1 is not a flt");
        s1 = src0->value.f;
        s2 = src1->value.f;
        return (s1>s2);
    }
    case Lop_GE_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src0)) | (!L_is_flt_constant(src1)))
            L_punt("GE_F: src0 or src1 is not a flt");
        s1 = src0->value.f;
        s2 = src1->value.f;
        return (s1>=s2);
    }
    case Lop_LT_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src0)) | (!L_is_flt_constant(src1)))
            L_punt("LT_F: src0 or src1 is not a flt");
        s1 = src0->value.f;
        s2 = src1->value.f;
        return (s1<s2);
   }
    case Lop_LE_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src0)) | (!L_is_flt_constant(src1)))
            L_punt("LE_F: src0 or src1 is not a flt");
        s1 = src0->value.f;
        s2 = src1->value.f;
        return (s1<=s2);
    }
    case Lop_F2_I: {
        double s1;
	if (!L_is_dbl_constant(src0))
            L_punt("F2_I: src0 is not a double");
        s1 = src0->value.f2;
        return ((int) s1);
    }
    case Lop_F_I: {
        double s1;
	if (!L_is_flt_constant(src0))
            L_punt("F_I: src0 is not a float");
        s1 = src0->value.f;
        return ((int) s1);
    }
    default:
        L_punt("L_rs_perform_int_arith: improper opcode sent");
	return (0);
    }
}

double L_rs_perform_flt_arith(int opc, L_Operand *src1, L_Operand *src2,
    L_Operand *src3)
{
    switch (opc) {
    case Lop_ADD_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src1)) | (!L_is_flt_constant(src2)))
            L_punt("ADD_F: src1 or src2 is not a flt");
        s1 = src1->value.f;
        s2 = src2->value.f;
        return (s1+s2);
    }
    case Lop_SUB_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src1)) | (!L_is_flt_constant(src2)))
            L_punt("SUB_F: src1 or src2 is not a flt");
        s1 = src1->value.f;
        s2 = src2->value.f;
        return (s1-s2);
    }
    case Lop_MUL_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src1)) | (!L_is_flt_constant(src2)))
            L_punt("MUL_F: src1 or src2 is not a flt");
        s1 = src1->value.f;
        s2 = src2->value.f;
        return (s1*s2);
    }
    case Lop_DIV_F: {
        double s1, s2;
	if ((!L_is_flt_constant(src1)) | (!L_is_flt_constant(src2)))
            L_punt("DIV_F:  src1 or src2 is not a flt");
        s1 = src1->value.f;
        s2 = src2->value.f;
        return (s1/s2);
    }
    case Lop_ABS_F: {
        int s1, is_neg;
        double s2, mag;
	if ((!L_is_int_constant(src1)) | (!L_is_flt_constant(src2)))
            L_punt("ABS_F:s1 or s2 is not correct type");
        s1 = src1->value.i;
        s2 = src2->value.f;
        is_neg = (s1<0);
        if (s2<0)
            mag = -s2;
        else
            mag = s2;
        if (is_neg)
            return (-mag);
        return mag;
    }
    case Lop_MUL_ADD_F: {
        double s1, s2, s3;
	if ((!L_is_flt_constant(src1)) | (!L_is_flt_constant(src2)) |
	    (!L_is_flt_constant(src3)))
            L_punt("MUL_ADD_F: src1,2,3 must all be flts");
        s1 = src1->value.f;
        s2 = src2->value.f;
        s3 = src3->value.f;
        return (s1*s2+s3);
    }
    case Lop_MUL_SUB_F: {
        double s1, s2, s3;
	if ((!L_is_flt_constant(src1)) | (!L_is_flt_constant(src2)) |
	    (!L_is_flt_constant(src3)))
            L_punt("MUL_SUB_F: src1,2,3 must all be flts");
        s1 = src1->value.f;
        s2 = src2->value.f;
        s3 = src3->value.f;
        return (s1*s2-s3);
    }
    case Lop_MUL_SUB_REV_F: {
        double s1, s2, s3;
	if ((!L_is_flt_constant(src1)) | (!L_is_flt_constant(src2)) |
	    (!L_is_flt_constant(src3)))
            L_punt("MUL_SUB_REV_F: src1,2,3 must all be flts");
        s1 = src1->value.f;
        s2 = src2->value.f;
        s3 = src3->value.f;
        return (-(s1*s2)+s3);
    }
    case Lop_F2_F: {
        double s1;
	if (!L_is_flt_constant(src1))
            L_punt("F2_F: src1 is not a double");
        s1 = src1->value.f2;
        return ((double) s1);
    }
    case Lop_I_F: {
        int s1;
	if (!L_is_int_constant(src1))
            L_punt("F2_F: src1 is not a integer");
        s1 = src1->value.i;
        return ((double) s1);
    }
    default:
        L_punt("L_rs_perform_flt_arith: improper opcode sent");
	return (0);
    }
}

double L_rs_perform_dbl_arith(int opc, L_Operand *src1, L_Operand *src2,
    L_Operand *src3)
{
    switch (opc) {
    case Lop_ADD_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src1)) | (!L_is_dbl_constant(src2)))
            L_punt("ADD_F2: src1 or src2 is not dble");
        s1 = src1->value.f2;
        s2 = src2->value.f2;
        return (s1+s2);
    }
    case Lop_SUB_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src1)) | (!L_is_dbl_constant(src2)))
            L_punt("SUB_F2: src1 or src2 is not dble");
        s1 = src1->value.f2;
        s2 = src2->value.f2;
        return (s1-s2);
    }
    case Lop_MUL_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src1)) | (!L_is_dbl_constant(src2)))
            L_punt("MUL_F2: src1 or src2 is not dble");
        s1 = src1->value.f2;
        s2 = src2->value.f2;
        return (s1*s2);
    }
    case Lop_DIV_F2: {
        double s1, s2;
	if ((!L_is_dbl_constant(src1)) | (!L_is_dbl_constant(src2)))
            L_punt("DIV_F2: src1 or src2 is not dble");
        s1 = src1->value.f2;
        s2 = src2->value.f2;
        return (s1/s2);
    }
    case Lop_ABS_F2: {
        int s1, is_neg;
        double s2, mag;
	if ((!L_is_int_constant(src1)) | (!L_is_dbl_constant(src2)))
            L_punt("ABS_F2: src1 or src2 is not dble");
        s1 = src1->value.i;
        s2 = src2->value.f2;
        is_neg = (s1<0);
        if (s2<0)
            mag = -s2;
        else
            mag = s2;
        if (is_neg)
            return (-mag);
        return mag;
    }
    case Lop_MUL_ADD_F2: {
        double s1, s2, s3;
	if ((!L_is_dbl_constant(src1)) | (!L_is_dbl_constant(src2)) |
	    (!L_is_dbl_constant(src3)))
            L_punt("MUL_ADD_F2: src1 or src2 is not dble");
        s1 = src1->value.f2;
        s2 = src2->value.f2;
        s3 = src3->value.f2;
        return (s1*s2+s3);
    }
    case Lop_MUL_SUB_F2: {
        double s1, s2, s3;
	if ((!L_is_dbl_constant(src1)) | (!L_is_dbl_constant(src2)) |
	    (!L_is_dbl_constant(src3)))
            L_punt("MUL_SUB_F2: src1 or src2 is not dble");
        s1 = src1->value.f2;
        s2 = src2->value.f2;
        s3 = src3->value.f2;
        return (s1*s2-s3);
    }
    case Lop_MUL_SUB_REV_F2: {
        double s1, s2, s3;
	if ((!L_is_dbl_constant(src1)) | (!L_is_dbl_constant(src2)) |
	    (!L_is_dbl_constant(src3)))
            L_punt("MUL_SUB_REV_F2: src1 or src2 is not dble");
        s1 = src1->value.f2;
        s2 = src2->value.f2;
        s3 = src3->value.f2;
        return (-(s1*s2)+s3);
    }
    case Lop_I_F2: {
        int s1;
	if (!L_is_int_constant(src1))
            L_punt("I_F2: src1 is not a integer");
        s1 = src1->value.i;
        return ((double) s1);
    }
    case Lop_F_F2: {
        double s1;
	if (!L_is_flt_constant(src1))
            L_punt("F_F2: src1 is not a float");
        s1 = src1->value.f;
        return ((double) s1);
    }
    default:
        L_punt("L_rs_perform_dbl_arith: improper opcode sent");
	return (0);
    }
}

/******************************************************************************\
 *
 * Routines used to solve integer arithmetic expressions.
 *
\******************************************************************************/

int L_can_swap_int_operands(int opc)
{
    switch (opc)
    {
	case Lop_SUB:
	case Lop_SUB_U:
	case Lop_DIV:
	case Lop_DIV_U:
	case Lop_REM:
	case Lop_REM_U:
	case Lop_MUL_ADD:
	case Lop_MUL_ADD_U:
	case Lop_MUL_SUB:
	case Lop_MUL_SUB_U:
	case Lop_MUL_SUB_REV:
	case Lop_MUL_SUB_REV_U:
	    return 0;

	default:
	    return 1;
    }
}

Value *L_rs_solve_2op_int_arith(RegFile *reg_file, L_Operand *dest, int opc, 
    Value *value0, Value *value1, int level, int return_value)
{
    int		result;
    L_Operand	*src;
    Value	*value=NULL;

    /* Simple case - opc is a move */
    if ((value0->opc == Lop_MOV) && (value1->opc == Lop_MOV))
    {
	if (L_is_numeric_constant(value0->src0) && 
	    L_is_numeric_constant(value1->src0))
	{
	    result = L_rs_perform_int_arith(opc, value0->src0, 
		value1->src0, NULL);

	    src = L_new_gen_int_operand(result);

	    if (return_value)
		value = L_v_new_value(Lop_MOV, src, NULL, NULL, level);
	    else
	        L_rf_define_register(reg_file, dest, Lop_MOV,
	            src, NULL, NULL, level);

	    L_delete_operand(src);
	}
	else
	{
	    if (return_value)
		value = L_v_new_value(opc, value0->src0, value1->src0, 
		    NULL, level);
	    else
	        L_rf_define_register(reg_file, dest, opc,
	            value0->src0, value1->src0, NULL, level);
	}
    }
    else if ((value0->opc == Lop_MOV) && (value1->opc != Lop_MOV))
    {
	if (L_is_numeric_constant(value1->src0))
	{
	    result = L_rs_perform_int_arith(opc, value0->src0, 
		value1->src0, NULL);

	    src = L_new_gen_int_operand(result);

	    if (return_value)
		value = L_v_new_value(value1->opc, src, value1->src1, 
		    NULL, level);
	    else
	        L_rf_define_register(reg_file, dest, value1->opc,
	            src, value1->src1, NULL, level);

	    L_delete_operand(src);
	}
	else if (L_can_swap_int_operands(value1->opc))
	{
	    result = L_rs_perform_int_arith(opc, value0->src0, 
		value1->src1, NULL);

	    src = L_new_gen_int_operand(result);

	    if (return_value)
		value = L_v_new_value(value1->opc, src, value1->src0, 
		    NULL, level);
	    else
	        L_rf_define_register(reg_file, dest, value1->opc,
	            src, value1->src0, NULL, level);

	    L_delete_operand(src);
	}
    }
    else if ((value0->opc != Lop_MOV) && (value1->opc == Lop_MOV))
    {
	if (L_is_numeric_constant(value0->src1))
	{
	    result = L_rs_perform_int_arith(opc, value0->src1, 
		value1->src0, NULL);

	    src = L_new_gen_int_operand(result);

	    if (return_value)
		value = L_v_new_value(value0->opc, value0->src0, src, 
		    NULL, level);
	    else
	        L_rf_define_register(reg_file, dest, value0->opc,
	            value0->src0, src, NULL, level);

	    L_delete_operand(src);
	}
	else if (L_can_swap_int_operands(value0->opc))
	{
	    result = L_rs_perform_int_arith(opc, value0->src0, 
		value1->src0, NULL);

	    src = L_new_gen_int_operand(result);

	    if (return_value)
		value = L_v_new_value(value0->opc, value0->src1, src, 
		    NULL, level);
	    else
	        L_rf_define_register(reg_file, dest, value0->opc,
	            value0->src1, src, NULL, level);

	    L_delete_operand(src);
	}
    }

    /* We do not handle the case where both are numeric constants */

    return value;
}

/******************************************************************************\
 *
 * Routines used to perform Lcode operations
 *
\******************************************************************************/

void L_rs_evaluate_mov(CG_Node *cg_node, RegFile *reg_file, L_Oper *oper, 
    int level)
{
    Value 	*value;
    Reg         *reg, *new_reg, *next_reg;
    int		cur_level;

    if (L_is_register(oper->src[0]) || L_is_macro(oper->src[0]))
    {
	if (L_is_macro(oper->src[0]))
	    cur_level = -1;
	else
	    cur_level = level;

        new_reg = L_rf_get_register(cg_node, reg_file, oper->src[0]);

        for (reg = new_reg; reg != NULL; reg=next_reg)
        {
            for (value = reg->first_value; value != NULL;
                 value = value->next_value)
            {
                L_rf_define_register(reg_file, oper->dest[0], value->opc,
                    value->src0, value->src1, value->src2, cur_level);
            }

            next_reg = reg->next_reg;

            L_rf_delete_reg(reg);
        }
    }
    else
    {
	L_rf_define_register(reg_file, oper->dest[0], oper->opc,
	     oper->src[0], NULL, NULL, level);
    }
}

void L_rs_perform_store(CG_Node *cg_node, RegFile *reg_file, 
    L_Operand *src0, L_Operand *src1, L_Operand *src2, int level)
{
    Value 	*value;
    Reg		*reg, *new_reg, *next_reg;
    int		cur_level;


    if (L_is_register(src2) || L_is_macro(src2))
    {
        if (L_is_macro(src2))
            cur_level = -1;
        else
            cur_level = level;

        /* Get the of the requested register */
        new_reg = L_rf_get_register(cg_node, reg_file, src2);

        /* Update memory */
        for (reg = new_reg; reg != NULL; reg=next_reg)
        {
            for (value = reg->first_value; value != NULL;
                 value = value->next_value)
            {
                L_mem_define_cell(cg_node, src0, src1, value->opc, 
		    value->src0, value->src1, value->src2, cur_level);
            }

            next_reg = reg->next_reg;

            L_rf_delete_reg(reg);
        }

    }
    else
    {
	/* Update memory */
	L_mem_define_cell(cg_node, src0, src1, Lop_MOV, 
	    src2, NULL, NULL, level);
    }
}

void L_rs_evaluate_store(CG_Node *cg_node, RegFile *reg_file, 
    L_Oper *oper, int level)
{
    Value 	*value0, *value1, *lv0, *lv1, *addr;
    Reg		*reg;
    L_Operand	*src0, *src1;

#if 0
    fprintf (stderr, "L_rs_evaluate_store: ");
    L_print_oper(stderr, oper);
#endif

    if (L_is_register(oper->src[0]) || L_is_macro(oper->src[0]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[0]);

	value0 = reg->first_value;
    }
    else if (oper->src[0] != NULL)
    {
	value0 = L_v_new_value(Lop_MOV, oper->src[0], NULL, NULL, level);
    }
    else
	value0 = NULL;

    if (L_is_register(oper->src[1]) || L_is_macro(oper->src[1]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[1]);

	value1 = reg->first_value;
    }
    else if (oper->src[1] != NULL)
    {
	value1 = L_v_new_value(Lop_MOV, oper->src[1], NULL, NULL, level);
    }
    else
	value1 = NULL;

    /* Compute all possible effective addresses */
    for (lv0 = value0; lv0 != NULL; lv0 = lv0->next_value)
    {
	if (value1)
	{
	    for (lv1 = value1; lv1 != NULL; lv1 = lv1->next_value)
	    {
	        addr = L_rs_solve_2op_int_arith(reg_file, NULL, 
		    Lop_ADD, lv0, lv1, level, 1);
	    
	        /* Load the contents of memory */
	        if (addr->opc == Lop_MOV)
	        {
		    src0 = addr->src0;
		    src1 = zero;
	        }
	        else
	        {
		    src0 = addr->src0;
		    src1 = addr->src1;
	        }
    
		L_rs_perform_store(cg_node, reg_file, src0, src1,
    		     oper->src[2], level);

    	        L_v_delete_all_value(addr);
	    }
	}
	else
	{
	    if (lv0->opc != Lop_MOV)
	    {
		L_warn("L_rs_evaluate_store: don't know how to handle this");
		continue;
	    }

	    src0 = lv0->src0;
	    src1 = zero;

	    L_rs_perform_store(cg_node, reg_file, src0, src1,
    		oper->src[2], level);
	}
    }

    L_v_delete_all_value(value0);
    L_v_delete_all_value(value1);
}

void L_rs_evaluate_load(CG_Node *cg_node, RegFile *reg_file, 
    L_Oper *oper, int level)
{
    Value 	*value, *value0, *value1, *lv0, *lv1, *addr;
    Memory_Cell	*cell;
    Reg		*reg;
    L_Oper	*load;

#if 0
    fprintf (stderr, "L_rs_evaluate_load: ");
    L_print_oper(stderr, oper);
#endif

    load = L_create_new_op(Lop_LD_I);

    if (L_is_register(oper->src[0]) || L_is_macro(oper->src[0]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[0]);

	value0 = reg->first_value;
    }
    else if (oper->src[0] != NULL)
    {
	value0 = L_v_new_value(Lop_MOV, oper->src[0], NULL, NULL, level);
    }
    else
	value0 = NULL;

    if (L_is_register(oper->src[1]) || L_is_macro(oper->src[1]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[1]);

	value1 = reg->first_value;
    }
    else if (oper->src[1] != NULL)
    {
	value1 = L_v_new_value(Lop_MOV, oper->src[1], NULL, NULL, level);
    }
    else
	value1 = NULL;

    /* Compute all possible effective addresses */
    for (lv0 = value0; lv0 != NULL; lv0 = lv0->next_value)
    {
	if (value1)
	{
	    for (lv1 = value1; lv1 != NULL; lv1 = lv1->next_value)
	    {
	        addr = L_rs_solve_2op_int_arith(reg_file, NULL, 
		    Lop_ADD, lv0, lv1, level, 1);
	    
	        /* Load the contents of memory */
	        if (addr->opc == Lop_MOV)
	        {
		    load->src[0] = addr->src0;
		    load->src[1] = zero;
	        }
	        else
	        {
		    load->src[0] = addr->src0;
		    load->src[1] = addr->src1;
	        }
    
    	        cell = L_mem_get_cell(cg_node, load);

    	        /* Update the specified register */
    	        for (value = cell->first_value; value != NULL; 
	             value = value->next_value)
    	        {
                    L_rf_define_register(reg_file, oper->dest[0], value->opc,
	    	        value->src0, value->src1, value->src2, level);
    	        }
    
    	        L_mem_delete_cell(cell);

    	        L_v_delete_all_value(addr);
	    }
	}
	else
	{
	    if (lv0->opc != Lop_MOV)
	    {
		L_warn("L_rs_evaluate_load: don't know how to handle this");
		continue;
	    }

	    load->src[0] = lv0->src0;
	    load->src[1] = zero;
    
    	    cell = L_mem_get_cell(cg_node, load);

    	    /* Update the specified register */
    	    for (value = cell->first_value; value != NULL; 
	         value = value->next_value)
    	    {
                L_rf_define_register(reg_file, oper->dest[0], value->opc,
	    	    value->src0, value->src1, value->src2, level);
    	    }
    
    	    L_mem_delete_cell(cell);
	}
    }

    L_v_delete_all_value(value0);
    L_v_delete_all_value(value1);

    load->src[0] = NULL;
    load->src[1] = NULL;
    L_delete_oper(NULL, load);
}

void L_rs_evaluate_int_arith(CG_Node *cg_node, RegFile *reg_file, 
    L_Oper *oper, int level)
{
    Value	*value0, *value1, *value2, *lv0, *lv1, *lv2;
    Reg		*reg;

    /* Setup for arithmetic evaluation */
    if (L_is_register(oper->src[0]) || L_is_macro(oper->src[0]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[0]);

	value0 = reg->first_value;
    }
    else if (oper->src[0] != NULL)
    {
	value0 = L_v_new_value(Lop_MOV, oper->src[0], NULL, NULL, level);
    }
    else
	value0 = NULL;

    if (L_is_register(oper->src[1]) || L_is_macro(oper->src[1]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[1]);

	value1 = reg->first_value;
    }
    else if (oper->src[1] != NULL)
    {
	value1 = L_v_new_value(Lop_MOV, oper->src[1], NULL, NULL, level);
    }
    else
	value1 = NULL;

    if (L_is_register(oper->src[2]) || L_is_macro(oper->src[2]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[2]);

	value2 = reg->first_value;
    }
    else if (oper->src[2] != NULL)
    {
	value2 = L_v_new_value(Lop_MOV, oper->src[2], NULL, NULL, level);
    }
    else
	value2 = NULL;
    
    /* Solve the arithmetic expressions for each combination */
    for (lv0 = value0; lv0 != NULL; lv0 = lv0->next_value)
    {
	/* 
	 * We won't update anything if there is no value1 
	 * because, this would be an illegal arithmetic operation
	 * in Lcode.
	 */
	if (value1)
	{
	    for (lv1 = value1; lv1 != NULL; lv1 = lv1->next_value)
	    {
		/*
		 * Since most of the Lcode arithmetic operations do not
		 * have 3 source operands, we will still attempt
		 * to solve the arithmetic operation.
		 */
		if (value2)
		{
		    L_punt ("L_rs_evaluate_int_arith: 3 src operands!");

            	    for (lv2 = value2; lv2 != NULL; lv2 = lv2->next_value)
	    	    {
			/* Not supported at this time */
	    	    }
		}
		else
		{
		    L_rs_solve_2op_int_arith(reg_file, oper->dest[0], 
			oper->opc, lv0, lv1, level, 0);
		}
	    }
	}
	else
	{
	    /* 
	     * Since we will stop use-def traversal when we build the callgraph
	     * if we we encounter a hash table base address, we need to
	     * ensure that the table address is propogated on!
	     */
	    if (L_mem_find_hash(lv0->src0, lv0->src1))
            	L_rf_define_register(reg_file, oper->dest[0], lv0->opc,
	            lv0->src0, lv0->src1, lv0->src2, level);
	}
    }
}

void L_rs_evaluate_float_arith(CG_Node *cg_node, RegFile *reg_file, 
    L_Oper *oper, int level)
{

}

void L_rs_evaluate_double_arith(CG_Node *cg_node, RegFile *reg_file, 
    L_Oper *oper, int level)
{

}

L_Operand *L_get_return_param(L_Oper *oper)
{
    L_Attr      *attr;
    L_Operand	*operand;
    int		mac, ctype;

    if ((attr = L_find_attr(oper->attr, "ret_i")) != NULL)
	ctype = L_CTYPE_INT;
    else if ((attr = L_find_attr(oper->attr, "ret_f")) != NULL)
	ctype = L_CTYPE_FLOAT;
    else if ((attr = L_find_attr(oper->attr, "ret_f2")) != NULL)
	ctype = L_CTYPE_DOUBLE;
    else
	return NULL;

    mac = L_MAC_INDEX((attr->field[0]->value.i) + L_MAC_P0);
    operand = L_new_macro_operand(mac, ctype, 0);

    return operand;
}

void L_rs_evaluate_jsr(CG_Node *cg_node, RegFile *reg_file, L_Oper *oper, 
    int level)
{
    L_Operand	*dest;
    Reg		*new_reg, *reg, *next_reg;
    Value	*value, *value0, *val;
 
    /* Setup for arithmetic evaluation */
    if (L_is_register(oper->src[0]) || L_is_macro(oper->src[0]))
    {
	reg = L_rf_get_register(cg_node, reg_file, oper->src[0]);

	value0 = reg->first_value;
    }
    else 
	value0 = L_v_new_value(Lop_MOV, oper->src[0], NULL, NULL, level);

    for (value = value0; value != NULL; value = value->next_value)
    {
	if (value->opc != Lop_MOV)
	{
	    L_warn ("L_rs_evaluate_jsr: reached a jsr that I don't know how to handle!");
	    continue;
	}
	else if (!L_is_label(value->src0))
	{
	    L_warn ("L_rs_evaluate_jsr: reached a jsr that I don't know how to handle!");
	    continue;
	}

        dest = L_get_return_param(oper);

	new_reg = L_rf_get_return_reg(cg_node, reg_file, oper, value->src0,
	    dest);

        for (reg = new_reg; reg != NULL; reg=next_reg)
        {
            for (val = reg->first_value; val != NULL;
                 val = val->next_value)
            {
                L_rf_define_register(reg_file, dest, val->opc,
                    val->src0, val->src1, val->src2, level);
            }

            next_reg = reg->next_reg;

            L_rf_delete_reg(reg);
        }
    }
}

/******************************************************************************\
 *
 * Resolve unknown operands 
 *
\******************************************************************************/

Resolved *L_rs_resolve_unknown(Resolved *resolved, CG_Node *cg_node, 
    RegFile *reg_file, UD_Node *term_node, UD_Node *node, int level, 
    int num_src_operands, L_Operand **src_operands, L_Oper *load)
{
    Def_Oper	*def_oper;
    L_Oper	*oper;
    UD_Arc	*arc;
    int		i;
    Reg		*reg, *new_reg, *next_reg;
    Memory_Cell	*new_cell, *cell, *next_cell;

    /* Everytime we recurse downward, increase the branch level */
    level++;

    /* Traverse all instructions in the current block. */
    for (def_oper = node->first_op; def_oper != NULL; 
	 def_oper = def_oper->next_op)
    {
	oper = def_oper->oper;

	/* If the current node matches the term node, we are done */
	if ((node == term_node) && (oper == term_node->term_oper))
	{
	    /* Handle any information that was passed through registers */
	    for (i=0; i<num_src_operands; i++)
	    {
		new_reg = L_rf_get_register(cg_node, reg_file, src_operands[i]);

		for (reg = new_reg; reg != NULL; reg=next_reg)
		{
		    resolved = L_rs_add_reg(resolved, reg, level); 

		    next_reg = reg->next_reg;

		    L_rf_delete_reg(reg);
		}
	    }

	    /* Handle any information that was passed through memory */
	    if (load)
	    {
    		new_cell = L_mem_get_cell(cg_node, load);

		for (cell = new_cell; cell != NULL; cell=next_cell)
		{
		    resolved = L_rs_add_cell(resolved, cell, level); 

		    next_cell = cell->next_cell;

		    L_mem_delete_cell(cell);
		}
	    }

	    return resolved;
	}
	else
	{   
	    /* 
	     * Evaluate Lcode oper based upon current state of 
	     * "register file" and "memory"
	     */
	    if (L_general_move_opcode(oper))	
	    {
		L_rs_evaluate_mov(cg_node, reg_file, oper, level);
	    }
	    else if (L_general_store_opcode(oper))
	    {
		L_rs_evaluate_store(cg_node, reg_file, oper, level);
	    }
	    else if (L_general_load_opcode(oper))
	    {
		L_rs_evaluate_load(cg_node, reg_file, oper, level);
	    }
	    else if (L_int_arithmetic_opcode(oper))
	    {
		L_rs_evaluate_int_arith(cg_node, reg_file, oper, level);
	    }
	    else if (L_subroutine_call_opcode(oper))
	    {
		L_rs_evaluate_jsr(cg_node, reg_file, oper, level);
	    }
	    else if (oper->dest[0] != NULL)
	    {
		L_rf_define_register(reg_file, oper->dest[0], 
		    Lop_MOV, NULL, NULL, NULL, level);
	    }
	}
    }

    /* Now traverse all paths */
    for (arc = node->dst_arc; arc != NULL; arc = arc->next_dst_arc)
    {
	resolved = L_rs_resolve_unknown(resolved, cg_node, reg_file, term_node, 
	    arc->dst_node, 0, num_src_operands, src_operands, load);

	/* 
	 * Free up any definitions that are a greater level than the
	 * current level.
	 */
	L_rf_delete_register_gt_level(reg_file, level);
    }

    return resolved;
}

/******************************************************************************\
 *
 * 
 *
\******************************************************************************/

Resolved *L_rs_add_reg(Resolved *resolved, Reg *new_reg, int level)
{
    Value 	*value;
    Reg		*reg;

    if (new_reg == NULL)
	return resolved;

    for (reg = resolved->first_reg; reg != NULL; reg = reg->next_reg)
    {
	if (L_same_operand(reg->dest, new_reg->dest))
	    break;
    }

    if (reg==NULL)
    {
	reg = L_rf_new_reg(new_reg->dest);

        for (value = new_reg->first_value; value != NULL; 
	     value = value->next_value)
        {
	    reg->first_value = L_v_add_value(reg->first_value, value->opc,
	        value->src0, value->src1, value->src2, level);
        }

	reg->next_reg = resolved->first_reg;
	resolved->first_reg = reg;
    }
    else
    {
        for (value = new_reg->first_value; value != NULL; 
	     value = value->next_value)
        {
	    reg->first_value = L_v_add_value(reg->first_value, value->opc,
	        value->src0, value->src1, value->src2, level);
        }
    }

    return resolved;
}

Resolved *L_rs_add_cell(Resolved *resolved, Memory_Cell *new_cell, int level)
{
    Value 	*value;
    Memory_Cell	*cell;

    if (new_cell == NULL)
	return resolved;

    for (cell = resolved->first_cell; cell != NULL; cell = cell->next_cell)
    {
	if (L_mem_same_global_address(new_cell->src0, new_cell->src1,
	    cell->src0, cell->src1))
	    break;
    }

    if (cell==NULL)
    {
	cell = L_mem_new_cell(new_cell->src0, new_cell->src1, RUNTIME_DEF);

        for (value = new_cell->first_value; value != NULL; 
	     value = value->next_value)
        {
	    cell->first_value = L_v_add_value(cell->first_value, value->opc,
	        value->src0, value->src1, value->src2, level);
        }

	cell->next_cell = resolved->first_cell;
	resolved->first_cell = cell;
    }
    else
    {
        for (value = new_cell->first_value; value != NULL; 
	     value = value->next_value)
        {
	    cell->first_value = L_v_add_value(cell->first_value, value->opc,
	        value->src0, value->src1, value->src2, level);
        }
    }

    return resolved;
}

/******************************************************************************\
 *
 * 
 *
\******************************************************************************/

Resolved *L_resolve_unknown(CG_Node *cg_node, L_Cb *cb, L_Oper *oper, 
    int num_src_operands, L_Operand **src_operands, L_Oper *load)
{
    UD_Graph	*graph = NULL;
    UD_Node	*node;
    Resolved 	*resolved;
    RegFile	*reg_file;
    
    resolved = L_rs_new_resolved();

    if (L_analysis_level > TRIVIAL_ANALYSIS)
    {
        if (num_src_operands || load)
            graph = L_usedef_build_graph(cg_node, cb, oper, num_src_operands, 
	        src_operands, load);

        else
	    return resolved;
    }

    reg_file = L_rf_new_reg_file();

    /* Determine all possible values that this operand resolves to */
    if (graph->start_node_list)
    {
        for (node = graph->start_node_list; node != NULL; 
	     node = node->next_snode)
        {
	    resolved = L_rs_resolve_unknown(resolved, cg_node, reg_file,
	        graph->end_node, node, 0, num_src_operands, src_operands, 
		load);
        }
    }
    else
    {
	resolved = L_rs_resolve_unknown(resolved, cg_node, reg_file,
	    graph->end_node, graph->end_node, 0, num_src_operands, 
	    src_operands, load);
    }

    L_usedef_delete_graph(graph);

    L_rf_delete_reg_file(reg_file);

    L_mem_reset(cg_node);

    return resolved;
}
