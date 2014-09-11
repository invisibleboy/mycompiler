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
/* This module breaks up big cbs.
 *
 * usage: Lsplit_cbs -Fmax_cb_size=<n> -i <input> -o <output>
 *
 * Any CB with more than <n> opers will be split into one or more CBs,
 * each with no more than <n> opers.
 */
#include <stdio.h>
#include <string.h>
#include <Lcode/l_main.h>
#include <Lcode/l_opti_functions.h>

/* Any CBs larger than Max_CB_Size will be broken up. */
static int Max_CB_Size = 250;

static void
Lsplit_cb_read_parm (Parm_Parse_Info *ppi)
{
  L_read_parm_i (ppi, "max_cb_size", &Max_CB_Size);

  return;
}

void
L_gen_code (Parm_Macro_List *command_line_macro_list)
{
  L_Cb *cb;
  L_Oper *op;
  int i, found_prologue = 0;

  L_load_parameters (L_parm_file, L_command_line_macro_list,
		     "(Lcode", Lsplit_cb_read_parm);

  L_open_input_file (L_input_file);

  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
	{
	  for (cb = L_fn->first_cb; cb; cb = cb->next_cb)
	    {
	      /* Find the Max_CB_Sizeth op.  The first CB in a function must
	       * contain a prologue op, so we don't split a CB before the
	       * prologue. */
	      for (i = 1, op = cb->first_op;
		   op && (i < Max_CB_Size || \
			  (cb == L_fn->first_cb && !found_prologue));
		   i++, op = op->next_op)
		if (op->opc == Lop_PROLOGUE)
		  found_prologue = 1;

	      if (op)
		L_split_cb_after (L_fn, cb, op);

	      L_check_cb (L_fn, cb);

	      /* cb now has Max_CB_Size ops. */
	      /* cb->next is the new CB (the remainder of the original CB. */
	    }
	  L_print_func (L_OUT, L_fn);
	  L_delete_func (L_fn);
	}
      else
	{
	  L_print_data (L_OUT, L_data);
	  L_delete_data (L_data);
	}
    }

  L_close_input_file (L_input_file);

  return;
}
