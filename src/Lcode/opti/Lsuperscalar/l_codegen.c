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
 *      File :          l_codegen.c
 *      Description :   Driver for superscalar optimizer
 *      Creation Date : July, 1990
 *      Author :        Scott Mahlke, Pohua Paul Chang, Wen-mei Hwu
 *      Revised 7-93 :  Scott A. Mahlke
 *              new Lcode
 *
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_superscalar.h"
#include "lsuper_export.h"
#include "../../tools/wcet_analysis/wcet_analysis.c"
#include "../../tools/Ldot/l_dot.c"

static void
process_input (void)
{
  switch (L_token_type)
    {
    case L_INPUT_EOF:
    case L_INPUT_MS:
    case L_INPUT_VOID:
    case L_INPUT_BYTE:
    case L_INPUT_WORD:
    case L_INPUT_LONG:
    case L_INPUT_LONGLONG:
    case L_INPUT_FLOAT:
    case L_INPUT_DOUBLE:
    case L_INPUT_ALIGN:
    case L_INPUT_ASCII:
    case L_INPUT_ASCIZ:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_RESERVE:
    case L_INPUT_GLOBAL:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
      /* LCW - new tokens for preserving debugging info - 8/19/97 */
    case L_INPUT_DEF_STRUCT:
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_ENUMERATOR:
      L_repair_hashtbl (L_data);
      L_print_data (L_OUT, L_data);
      L_delete_data (L_data);
      break;
    case L_INPUT_FUNCTION:
      L_define_fn_name (L_fn->name);



      Lsuper_optimize (L_fn);        /* JCG 6/99 */
      //Ldot_display_cfg(L_fn,"t3",0);
      //char filename[100];
      //strcpy(filename,L_fn->name);
      //strcat(filename,"(3).txt");
      //ILP_cfg_wcet_analyize(L_fn,filename);

      L_print_func (L_OUT, L_fn);
      L_delete_func (L_fn);
      break;
    default:
      L_punt ("process_input: illegal token", L_ERR_INTERNAL);
    }
}


void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  Lsuper_initialize (command_line_macro_list);

  L_open_input_file (L_input_file);
  while (L_get_input () != L_INPUT_EOF)
    {
      process_input ();
    }

  Lsuper_cleanup ();

  L_close_input_file (L_input_file);
}
