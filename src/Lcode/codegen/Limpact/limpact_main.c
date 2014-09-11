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
 *  File:  limpact_main.c
 *
 *  Description:
 *    Driver module for Limpact code generator
 *
 *  Creation Date :  June 1993
 *
 *  Author:  Scott A. Mahlke, Roger A. Bringmann, Richard Hank, 
 *           John C. Gyllenhaal, Wen-mei Hwu
 *
 *  Description:
 *    This is the main entry point for the code generator.  It is designed to
 *    be generic.  It permits calling one time initialization routines for each
 *    of the phases.  The calling convention and parameters are discussed below.
 *
 *      Phase stuff left in for impact code generator, HOWEVER, there really
 *      is no phase 1 or 3, only a phase 2 exists since we are generating
 *      code for an Lcode machine!!!!!!!!!!!!!
 *
 *      Phase 1 :  Annotation from Lcode to Mcode
 *      Phase 2 :  Mcode optimizations, register allocation, 
 *                  instruction scheduling
 *      Phase 3 :  Convert Mcode to target assembly code.
 *
 *      Example:
 *              Lamd29k -verbose -target -c 4 -i in.lc -o out.mco
 *
 *      Execution of these phases is controlled by the following variable:
 *        int L_codegen_phase;
 *      1 = phase 1 only        (.lc input, .mc output)
 *      2 = phase 2 only        (.mc input, .mco output)
 *      4 = phase 3 only        (.mc input, .s output)
 *
 *      NOTE:  The extensions listed in () following the numeric values is the
 *      input and output convention.  It is up to the user to maintain this
 *      convention.
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "limpact_main.h"

char CurrentFunction[1024];
/* do we produce all of the detailed file header information in a .s file */

int Limpact_predicated_lcode = 0;       /* processing predicated Lcode */
int Limpact_do_echo = 1;
int Limpact_do_static_br_pred = 0;
char *Limpact_br_pred_model = NULL;
int Limpact_num_int_caller_reg = 0;
int Limpact_num_int_callee_reg = 0;
int Limpact_num_flt_caller_reg = 0;
int Limpact_num_flt_callee_reg = 0;
int Limpact_num_dbl_caller_reg = 0;
int Limpact_num_dbl_callee_reg = 0;
int Limpact_num_prd_caller_reg = 0;
int Limpact_num_prd_callee_reg = 0;
int Limpact_do_hb_spill_opti = 0;
int Limpact_print_hb_spill_opti_stats = 0;
int Limpact_print_prepass_schedule = 0;
int Limpact_annotate_varargs = 0;
int Limpact_adjust_memory_stack = 0;
int Limpact_return_address_in_reg = 0;
int Limpact_debug_stack_frame = 0;
int Limpact_do_branch_split = 0;
int Limpact_convert_init_pred_to_uncond_defs = 0;
FILE *prepass_out_file;

#define P_NONE          0x00    /* all phases */
#define P_1             0x01
#define P_2             0x02
#define P_3             0x04
#define P_ALL           0x07    /* all phases */

char *phase_message[8] = {
  " NONE!",
  " 1 only",
  " 2 only",
  "s 1 and 2",
  " 3 only",
  "s 1 and 3",
  "s 2 and 3",
  "s 1, 2 and 3"
};

void
L_read_parm_limpact (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "predicated_lcode", &Limpact_predicated_lcode);
  L_read_parm_b (ppi, "do_echo", &Limpact_do_echo);
  L_read_parm_b (ppi, "do_static_br_pred", &Limpact_do_static_br_pred);
  L_read_parm_s (ppi, "br_pred_model", &Limpact_br_pred_model);

  L_read_parm_b (ppi, "do_hb_spill_opti", &Limpact_do_hb_spill_opti);
  L_read_parm_b (ppi, "print_hb_spill_opti_stats",
                 &Limpact_print_hb_spill_opti_stats);
  L_read_parm_b (ppi, "print_prepass_schedule",
                 &Limpact_print_prepass_schedule);
  L_read_parm_b (ppi, "annotate_varargs", &Limpact_annotate_varargs);
  L_read_parm_b (ppi, "adjust_memory_stack", &Limpact_adjust_memory_stack);
  L_read_parm_b (ppi, "return_address_in_reg", &Limpact_return_address_in_reg);
  L_read_parm_b (ppi, "debug_stack_frame", &Limpact_debug_stack_frame);
  L_read_parm_b (ppi, "branch_split", &Limpact_do_branch_split);
  L_read_parm_b (ppi, "convert_init_pred_to_uncond_defs",
                 &Limpact_convert_init_pred_to_uncond_defs);

  Limpact_num_int_caller_reg = Mspec_num_int_caller_reg;
  Limpact_num_int_callee_reg = Mspec_num_int_callee_reg;
  Limpact_num_flt_caller_reg = Mspec_num_flt_caller_reg;
  Limpact_num_flt_callee_reg = Mspec_num_flt_callee_reg;
  Limpact_num_dbl_caller_reg = Mspec_num_dbl_caller_reg;
  Limpact_num_dbl_callee_reg = Mspec_num_dbl_callee_reg;
  Limpact_num_prd_caller_reg = Mspec_num_prd_caller_reg;
  Limpact_num_prd_callee_reg = Mspec_num_prd_callee_reg;  
}

void
L_fix_SEF_flag (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
          if (!L_subroutine_call_opcode (oper))
            continue;
          if (!L_side_effect_free_sub_call (oper))
            continue;
          if (!L_op_in_side_effect_free_func_table (oper))
            oper->flags =
              L_CLR_BIT_FLAG (oper->flags, L_OPER_SIDE_EFFECT_FREE);
        }
    }
}

/*
 *  L_gen_code is the entry point to code generation called from l_main.c
 */
void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  char *prepass_output_filename;

  /* can either use Limpact with Lcode or HPPA Mcode right now!! */
  if (M_arch != M_IMPACT)
    L_punt ("ILLEGAL architecture specified for this code generator!");

  /* Load the parameters specific to Lcode code generation */
  L_load_parameters (L_parm_file, command_line_macro_list,
                     "(Mcode", L_read_parm_mcode);
  L_load_parameters (L_parm_file, command_line_macro_list,
                     "(Limpact", L_read_parm_limpact);

  /* check for invalid parameters */
  if ((L_codegen_phase < P_NONE) || (L_codegen_phase > P_ALL))
    L_punt ("L_gen_code: Invalid code generation phase");

  if (Limpact_print_prepass_schedule)
    {
      if (!L_do_prepass_sched)
        L_punt ("ERROR: cannot print out prepass schedule if prepass is \
not going to be performed.");
      prepass_output_filename = (char *) malloc (strlen (L_output_file) + 10);
      strcpy (prepass_output_filename, L_output_file);
      strcat (prepass_output_filename, ".prepass");
      prepass_out_file = fopen (prepass_output_filename, "w");
      free (prepass_output_filename);
    }

  L_open_input_file (L_input_file);

  /*
   * Perform global initialization 
   */

  if ((L_codegen_phase == 0) || (L_codegen_phase & P_1))
    L_init (command_line_macro_list);

  if ((L_codegen_phase == 0) || (L_codegen_phase & P_2))
    O_init (command_line_macro_list);

  if ((L_codegen_phase == 0) || (L_codegen_phase & P_3))
    P_init ();

  if ((L_do_register_allocation == 0) && (L_do_postpass_sched))
    {
      /* The conflicting operands functions assume that register 
       * allocation has been done while building the dependence graph 
       * for postpass scheduling which can result in incorrect floating 
       * point depedences...             
       */
      L_punt ("L_gen_code: Cannot do postpass scheduling without "
              "register allocation!!!\n");
    }

  /* Process all data and functions within a file */
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
        {
          strcpy (CurrentFunction, L_fn->name);
          if (L_debug_messages)
            fprintf (stderr, "Processing %s - phase%s\n", L_fn->name,
                     phase_message[L_codegen_phase]);

          /* Sometimes the L_CB_HYPERBLOCK_NOFALLTHRU flag is
             incorrect, this call corrects it for scheduling and
             reg allocation so there are no punts.  Its basically
             a backward compatibility for benchmarks generated before
             the bug was fixed... SAM 11-94 */
          L_set_hb_no_fallthru_flag (L_fn);

          L_fix_SEF_flag (L_fn);

          /* hyperblocks that have no predicate defines cause problems
             for dataflow analysis.  So insert a dummy op as the
             first oper of these blocks... SAM 11-94 */
          L_insert_dummy_op_into_hyperblocks (L_fn);

          if (L_codegen_phase & P_1)
            L_process_func (L_fn);

          if (Limpact_do_static_br_pred)
            L_mark_static_branch_pred (L_fn, Limpact_br_pred_model);

          if (L_codegen_phase & P_2)
            O_process_func (L_fn, command_line_macro_list);

          if (Limpact_do_echo)
            {
              if (L_codegen_phase & P_3)
                P_process_func (L_fn);
              else
                L_print_func (L_OUT, L_fn);
            }

          L_delete_func (L_fn);
        }
      else
        {  /*  We will only process the data segments if we are going to
	    * perform phase 3. Otherwise, we will just print the data into the
	    * new file. */
          if ((L_codegen_phase == 0) || (L_codegen_phase & P_3))
            {
              if (Limpact_do_echo)
                P_process_data (L_OUT, L_data);
              if (Limpact_do_echo && Limpact_print_prepass_schedule)
                L_print_data (prepass_out_file, L_data);
            }
          else
            {
              if (Limpact_do_echo)
                L_print_data (L_OUT, L_data);
              if (Limpact_do_echo && Limpact_print_prepass_schedule)
                L_print_data (prepass_out_file, L_data);
            }
          L_delete_data (L_data);
        }

    }

  /*
   * Perform global cleanup 
   */

  if ((L_codegen_phase == 0) || (L_codegen_phase & P_1))
    L_cleanup ();
  if ((L_codegen_phase == 0) || (L_codegen_phase & P_2))
    O_cleanup ();

  if (Limpact_print_prepass_schedule)
    fclose (prepass_out_file);

  L_close_input_file (L_input_file);
}
