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
/* 10/8/02 Robert Kidd
 * This file defines L_gen_code and process_input for Ltahoe.  These functions
 * have been moved to this file from ltahoe_main.c.  This allows external
 * modules to link to libtahoe.a without running into trouble when redefining
 * L_gen_code.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "phase1_func.h"
#include "phase2_func.h"
#include "phase3.h"

/* Ltahoe_process_input (Parm_Macro_List *command_line_macro_list)
 * ----------------------------------------------------------------------
 * Process a top-level Lcode token (function or data element) 
 */

static void
Ltahoe_process_input (Parm_Macro_List * command_line_macro_list)
{
  if (L_token_type == L_INPUT_FUNCTION)
    {
      if (L_codegen_phase & P_1)
	{
	  L_process_func (L_fn, command_line_macro_list);
	}

      if (L_codegen_phase & P_2)
	{
	  O_process_func (L_fn, command_line_macro_list);
	}

      if (L_codegen_phase & P_3)
	{
	  if (!Ltahoe_vulcan)
	    {
	      P_process_func (L_fn);
	    }
	  else
	    {
	      P_convert_reg_nums (L_fn);
	      L_concat_attr (L_fn->attr,
			     L_new_attr ("architectural_registers", 0));
	      P_fix_pred_compare_dests_func (L_fn);
	      L_print_func (L_OUT, L_fn);
	    }
	}
      else
	{
	  L_print_func (L_OUT, L_fn);
	}

      L_delete_dataflow (L_fn);
      PG_destroy_pred_graph ();

      L_delete_func (L_fn);
    }
  else
    {
      if ((L_codegen_phase & P_3) && !Ltahoe_vulcan)
	P_process_data (L_OUT, L_data);
      else
	L_print_data (L_OUT, L_data);
      L_delete_data (L_data);
    }
  return;
}

/* L_gen_code (Parm_Macro_List * command_line_macro_list)
 * ----------------------------------------------------------------------
 * Module entry point (called from Lcode l_main.c) 
 */

/* 09/17/02 REK Removing references to Tmdes. */
/* 10/08/02 REK Moved initialization code to Ltahoe_init. */
void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  char ld_table_file[100];

  Ltahoe_init (command_line_macro_list);

  strcpy (ld_table_file, L_output_file);
  strcat (ld_table_file, ".lt");

  /* Token processing
   * ---------------------------------------------------------------------- 
   */
  if (L_file_processing_model == L_FILE_MODEL_FILE)
    {
      if (L_codegen_phase & P_3)
	{
	  P_file_init ();
	 
	  if (Ltahoe_tag_loads)
	    {
	      LD_TABLE_OUT = L_open_output_file (ld_table_file);
	    }
	}
      L_open_input_file (L_input_file);
      while (L_get_input () != L_INPUT_EOF)
	Ltahoe_process_input (command_line_macro_list);
      L_close_input_file (L_input_file);
      if (L_codegen_phase & P_3)
	{
	  if (Ltahoe_tag_loads)
	    {
	      L_close_output_file (LD_TABLE_OUT);
	    }
	  P_file_end ();
	}
    }				/* if */
  else if ((L_file_processing_model == L_FILE_MODEL_LIST) ||
	   (L_file_processing_model == L_FILE_MODEL_EXTENSION))
    {
      L_create_filelist ();
      while ((L_file = List_next (L_filelist)))
	{
	  L_input_file = L_file->input_file;
	  L_output_file = L_file->output_file;

	  L_OUT = L_open_output_file (L_output_file);
	  L_generation_info_printed = 0;

	  if (L_codegen_phase & P_3)
	    {
	      P_file_init ();
	      if (Ltahoe_tag_loads)
		{
		  LD_TABLE_OUT = L_open_output_file (ld_table_file);
		}
	    }
	  L_open_input_file (L_input_file);
	  while (L_get_input () != L_INPUT_EOF)
	    Ltahoe_process_input (command_line_macro_list);
	  L_close_input_file (L_input_file);

	  if (L_codegen_phase & P_3)
	    {
	      if (Ltahoe_tag_loads)
		{
		  L_close_output_file (LD_TABLE_OUT);
		} 
	      P_file_end ();
	    }
	  L_close_output_file (L_OUT);
	}			/* while */
      L_delete_filelist ();
    }				/* else if */

  /* Cleanup
   * ---------------------------------------------------------------------- 
   */

  Ltahoe_cleanup ();
}				/* L_gen_code */
