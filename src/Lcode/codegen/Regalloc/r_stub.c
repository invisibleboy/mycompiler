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
 *
 *      File :          r_stub.c
 *      Description :   Stubs for code generator call back routines
 *      Creation Date : Feb. 1994
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 * Revision 1.2  95/01/05  17:57:53  17:57:53  hank (Richard E. Hank)
 * Added default return values to each function
 *
 * Revision 1.1  94/03/16  20:53:24  20:53:24  hank (Richard E. Hank)
 * Initial revision
 *
 *
 *===========================================================================*/
/*===========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_code.h>

double
R_callee_cost (int lcode_ctype, int leaf, int callee_allocated)
{
  return (2.0);
}

double
R_caller_cost (int lcode_ctype, int leaf)
{
  return (2.0);
}

double
R_spill_load_cost (int lcode_ctype)
{
  return (1.0);
}

double
R_spill_store_cost (int lcode_ctype)
{
  return (1.0);
}

void
O_jump_oper (int opc, L_Cb * dst)
{
}

L_Oper *
O_spill_reg (int reg, int type,
	     L_Operand * operand,
	     int spill_offset, L_Operand ** pred, int type_flag)
{

  return NULL;

}

L_Oper *
O_fill_reg (int reg, int type,
	    L_Operand * operand,
	    int fill_offset, L_Operand ** pred, int type_flag)
{

  return NULL;

}

struct R_Physical_Bank;
struct R_Reg;

/* Needed by the register allocator. MCM */
struct R_Physical_Bank *
O_locate_rot_reg_bank (L_Func * fn, struct R_Reg *vreg)
{
  L_punt ("O_locate_rot_reg_bank: Unsupported.");
  return NULL;
}
