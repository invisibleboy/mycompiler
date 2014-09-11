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
 *      File :          l_codegen.c
 *      Description :   pseudo main routine - all Lcode modules should have
 *                      a set of functions with similar functionality
 *      Author: Po-hua Chang, Wen-mei Hwu
 *      Creation Date:  June 1990
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include "l_event.h"
#include "l_bdf_flow.h"
#include "l_ssa.h"

/* Parameters local to Lcode executable */
int Lcode_echo = 0;
int Lcode_print_stat = 0;
int Lcode_unit_time = 0;
int Lcode_run_df_live_variable = 0;
int Lcode_run_df_print_pred_flow = 0;
int Lcode_run_loop_detection = 0;
int Lcode_test_new_df = 0;

#if 0
void
JWS_print_mem_usage (void)
{
  static long start = 0;
  int u, r;

  if (!start)
    start = (long) sbrk(0);

  Set_memory_usage (&u,&r);

  fprintf (stderr, "MEM: %10ldk su: %10ldk sr: %10ldk\n", 
	   ((long) sbrk(0) - start) / 1024, (long) u/1024, (long) r/1024);
  return;
}
#endif

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
    case L_INPUT_RESERVE:
    case L_INPUT_GLOBAL:
    case L_INPUT_WB:
    case L_INPUT_WW:
    case L_INPUT_WI:
    case L_INPUT_WQ:
    case L_INPUT_WF:
    case L_INPUT_WF2:
    case L_INPUT_WS:
    case L_INPUT_ELEMENT_SIZE:
    case L_INPUT_DEF_STRUCT:    /* Folded in SAM fix.  -JCG 5/99 */
    case L_INPUT_DEF_UNION:
    case L_INPUT_DEF_ENUM:
    case L_INPUT_FIELD:
    case L_INPUT_SKIP:
    case L_INPUT_ENUMERATOR:
      if (Lcode_echo)
        L_print_data (L_OUT, L_data);
      L_delete_data (L_data);
      break;
    case L_INPUT_EVENT_LIST:
      if (Lcode_echo)
        L_print_event_list (L_OUT, L_event_list);
      L_delete_event_list (L_event_list);
      break;
    case L_INPUT_RESULT_LIST:
      if (Lcode_echo)
        L_print_result_list (L_OUT, L_result_list);
      L_delete_event_list (L_result_list);
      break;
    case L_INPUT_FUNCTION:
      if (Lcode_print_stat)
        L_record_stat (L_fn);


      PG_setup_pred_graph (L_fn);

      /* Testing options for Lcode executable */
      if (Lcode_run_df_live_variable)
        {
          L_do_flow_analysis (L_fn, POST_DOMINATOR_CB | LIVE_VARIABLE);
          PF_daVinci (stdout, PF_pred_flow);
        }

      /* Testing options for Lcode executable */
      if (Lcode_run_df_print_pred_flow)
        {
	  L_punt("PDF is not supported in this build");
        }

      if (Lcode_run_loop_detection)
        {
          L_do_flow_analysis (L_fn, DOMINATOR);
          L_loop_detection (L_fn, 0);
        }

      if (Lcode_test_new_df == 1)
        {
	  L_do_new_flow_analysis (L_fn, LIVE_VARIABLE);
	}
      else if (Lcode_test_new_df == 2)
	{
	  L_do_flow_analysis (L_fn, LIVE_VARIABLE);
	  L_print_dataflow (L_fn);
	}
      else if (Lcode_test_new_df == 3)
	{
#if 0
	  LD_dominator (L_fn);
#else
	  L_form_ssa (L_fn);
#endif
	}

      if (Lcode_echo)
        L_print_func (L_OUT, L_fn);

      PG_destroy_pred_graph ();      

      L_delete_func (L_fn);

      break;
    default:
      L_punt ("process_input: illegal token");
    }
}

/*
 *      Read module specific parameters
 */
void
L_read_parm_lcode (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "echo", &Lcode_echo);
  L_read_parm_b (ppi, "print_stat", &Lcode_print_stat);
  L_read_parm_b (ppi, "unit_time", &Lcode_unit_time);
  L_read_parm_b (ppi, "run_df_live_variable", &Lcode_run_df_live_variable);
  L_read_parm_b (ppi, "run_df_print_pred_flow",
                 &Lcode_run_df_print_pred_flow);
  L_read_parm_b (ppi, "run_loop_detection", &Lcode_run_loop_detection);
  L_read_parm_i (ppi, "?test_new_df", &Lcode_test_new_df);
}

void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  /* Load the parameters specific to Lcode code generation */
  L_load_parameters (L_parm_file, command_line_macro_list,
                     "(Lcode", L_read_parm_lcode);

  L_open_input_file (L_input_file);
  while (L_get_input () != L_INPUT_EOF)
    {
      process_input ();
    }

  L_close_input_file (L_input_file);
  /*
   *  print statistic.
   */
  if (Lcode_print_stat)
    L_print_stat ();
}
