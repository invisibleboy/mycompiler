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
 *      File :          r_regquery.c
 *      Description :   Register allocation configuration queries
 *      Creation Date : Feb. 1994
 *      Author :        Richard Hank, Wen-mei Hwu
 *
 * Revision 1.1  94/03/16  20:53:14  20:53:14  hank (Richard E. Hank)
 * Initial revision
 *
 *
 *===========================================================================*/
/*===========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "r_regalloc.h"

/*
 * Return the number of available register of type <type>, if
 * <caller_save> == 1, return num caller saved registers, else
 * return callee saved registers
 */
int
R_number_of_registers (int type, int caller_save, int macro)
{
  R_Physical_Bank *caller, *callee, *macro_caller, *macro_callee;
  int registers = 0;

  int reg_type = R_Ltype_to_Rtype (type);

  if (caller_save)
    {
      caller = R_bank + reg_type + R_CALLER;
      if (caller->defined)
	registers += caller->num_reg;
    }
  else
    {
      callee = R_bank + reg_type + R_CALLEE;
      if (callee->defined)
	registers += callee->num_reg;
    }

  if (R_Macro_Allocation && macro)
    {
      if (caller_save)
	{
	  macro_caller = R_bank + reg_type + R_MACRO_CALLER;
	  if (macro_caller->defined)
	    registers += macro_caller->num_reg;
	}
      else
	{
	  macro_callee = R_bank + reg_type + R_MACRO_CALLEE;
	  if (macro_callee->defined)
	    registers += macro_callee->num_reg;
	}
    }


  return (registers);
}

int
R_size_of_register (int type)
{
  R_Physical_Bank *bank = R_bank + R_Ltype_to_Rtype (type);

  if (bank->defined)
    return (bank->reg_size * 32);

  return (0);
}

int
R_register_overlap (int type1, int type2)
{
  R_Physical_Bank *bank1, *bank2;

  bank1 = R_bank + R_Ltype_to_Rtype (type1) + R_CALLER;
  if (!bank1->defined)
    return (0);

  bank2 = R_bank + R_Ltype_to_Rtype (type2) + R_CALLER;
  if (!bank2->defined)
    return (0);

  return (bank1->overlap == bank2->overlap);
}
