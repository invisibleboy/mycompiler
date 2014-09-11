/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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

#include <config.h>
#include <stdlib.h>
#include "ltahoe_main.h"
#include "ltahoe_completers.h"
#include "ltahoe_table.h"
#include "phase3.h"
#include <library/string_symbol.h>




/****************************************************************************
 *
 * routine: P_print_load_table()
 * purpose: Prints load information to a file
 * input: pointers to an L_Oper, L_Cb, and an integer instruction offset
 * output:
 * returns:
 * modified:
 * note:
 *-------------------------------------------------------------------------*/
void
P_print_load_table (L_Oper * oper, L_Cb * cb, unsigned int instr_offset, int slot_no)
{

  L_Attr *attr;
  int load_id = 0;


  // allen begin
  fprintf (LD_TABLE_OUT, "%d:", cb->id);
  fprintf (LD_TABLE_OUT, "%s", LTAHOE_TABLE_ENTRY (Ltahoe_table, oper->proc_opc).opc_str);
  switch (TC_GET_LD_TYPE (oper->completers))
  {
    case TC_LD_TYPE_A:
      fprintf (LD_TABLE_OUT, ".a:");
      break;
    case TC_LD_TYPE_S:
      fprintf (LD_TABLE_OUT, ".s:");
      break;
    case TC_LD_TYPE_SA:
      fprintf (LD_TABLE_OUT, ".sa:");
      break;
    case TC_LD_TYPE_NONE:
    default:
      fprintf (LD_TABLE_OUT, ":");
    break;
  }

  for (attr = oper->attr; attr; attr = attr->next_attr)
    {
       if (!strcmp (attr->name, "load_id"))
         {
  	   load_id = attr->field[0]->value.i;
         }
    }
  fprintf (LD_TABLE_OUT, "%d:", load_id);
  fprintf (LD_TABLE_OUT, "%s:", oper->opcode);
  fprintf (LD_TABLE_OUT, "%d:", instr_offset);
  fprintf (LD_TABLE_OUT, "%d", slot_no);
  fprintf (LD_TABLE_OUT, "\n");

}































