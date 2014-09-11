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
 *      File:   l_code_gen.c
 *              See l_build_prootype_info.c for usage info.
 *
 *      Authors: John C. Gyllenhaal, Wen-mei Hwu
 *      Creation Date:  May 1998
 *
 *      Enhanced to create libproto.a - John C. Gyllenhaal 3/99
 *
\*****************************************************************************/

#include <config.h>
#include "l_build_prototype_info.h"

/* Lcode entry point, scan all code presented to Lbuild_prototype_info,
 * gather call_info strings, and deduce "legal" (but perhaps not exact if
 * from a library) function prototypes for everything called or declared
 * in the program.  Written to support NYU's Elcor emulator for the
 * Aug 1998 Trimaran release.
 */
void
L_gen_code (Parm_Macro_List * external_macro_list)
{

  /* Create and initialize the call info tables */
  L_init_call_info ();

  L_open_input_file (L_input_file);

  /* Process all data and functions within a file */
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
        {
          L_define_fn_name (L_fn->name);

          /* Build up call_info database using the info in this function */
          L_collect_call_info (L_fn);

          /* Don't print out the function */
          L_delete_func (L_fn);
        }
      else
        {
          /* Do nothing with data */
          L_delete_data (L_data);
        }
    }

  L_close_input_file (L_input_file);

#if 0
  L_print_debug_prototypes (L_OUT, func_table);
  L_print_debug_prototypes (L_OUT, jsr_table);
#endif

  /* Using func_table, jsr_table, and varargs_table, deduce "valid"
   * prototypes for every called function and place them in prototype_table.
   * Pass NULL as argument because we didn't scan the data and functions
   * for function labels used (possible future extension). -ITI/JCG 4/99
   *
   * Don't set any of the flags for now. -ITI/JCG 4/99
   */
  L_deduce_prototypes (NULL, 0);

#if 0
  /* Debug output, print out verbose info on deduced prototypes */
  L_print_debug_prototypes (L_OUT, prototype_table);
#endif

  /* Print out Hcode output */
  L_print_hcode_prototypes (L_OUT);
}
