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
 * phase3_unwind.h                                                           *
 * ------------------------------------------------------------------------- *
 * Generation of unwind directives                                           *
 *                                                                           *
 * AUTHORS: R.D. Barnes                                                      *
 *****************************************************************************/

#ifndef _PHASE3_UNWIND_H_
#define _PHASE3_UNWIND_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>

extern struct unwind_info unwind;
void L_get_unwind_info (L_Func * fn);

typedef struct _UnwindReg
{
  L_Oper *save_op;
  int in_reg;
  union
  {
    L_Operand *reg;
    int ofst;
  }
  loc;
}
UnwindReg;

typedef struct unwind_info
{
  UnwindReg pfs;
  UnwindReg rp;
  UnwindReg unat;
  UnwindReg rnat;
  UnwindReg lc;
  UnwindReg fpsr;
  UnwindReg pr;

  L_Oper *first_prologue_inst;

  L_Oper *last_prologue_inst;

  L_Oper *mem_stack;
  L_Oper *mem_stack_dealloc;
  int mem_stack_size;

  short temp_reg_sp_rel;
  int temp_reg_offset;
  int temp_reg_absolute;
}
unwind_info;

#endif
