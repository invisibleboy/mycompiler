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
 * ltahoe_main.c                                                             *
 * ------------------------------------------------------------------------- *
 * Itanium Processor Family code generator driver                            *
 *                                                                           *
 * PHASE I:   Annotation from Lcode to Mcode                                 *
 * PHASE II:  Mcode optimization, register allocation, instruction sched.    *
 * PHASE III: Final annotation and Intel assembly generation                 *
 *                                                                           *
 * AUTHORS: S.A. Mahlke, D.A. Connors, J.W. Sias                             *
 *****************************************************************************/
/* 10/08/02 REK Adding a function to perform phases 1, 2, and 3.  This function
 *              exists so that external modules can run Ltahoe without knowing
 *              that there are three phases.
 *
 *              Moving L_gen_code and process_input to ltahoe_codegen.c.  This
 *              allows external modules to link to libtahoe.a without running
 *              into trouble when redefining these functions.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <time.h>

#include "ltahoe_main.h"
#include "phase1_func.h"
#include "phase2_func.h"
#include "phase3.h"

#include <Lcode/mia_opti.h>
#include <Lcode/l_opti.h>
#include <library/func_list.h>

/* 10/08/02 REK Moving phase selection macros to ltahoe_main.h. */

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

/* PHASE I PARAMETERS */

int Ltahoe_use_gp_rel_addressing = 0;
int Ltahoe_prologue_merge = 0;
int Ltahoe_input_param_subst = 0;
int Ltahoe_do_lcode_peephole = 0;
int Ltahoe_postinc_ld = 0;
int Ltahoe_postinc_st = 0;
int Ltahoe_print_opti_stats = 0;
int Ltahoe_add_mov_ap = 0;
int Ltahoe_do_lightweight_pred_opti = 0;
int Ltahoe_do_repeated_mopti = 0;
int Ltahoe_do_redux = 1;
int Ltahoe_do_tbit_redux = 1;
int Ltahoe_do_extr_redux = 1;
int Ltahoe_do_depo_redux = 1;
int Ltahoe_do_ldf_redux = 1;
int Ltahoe_do_sp_removal = 1;
int Ltahoe_fp_ftz = 0;
int Ltahoe_bitopt = 0;
int Ltahoe_vulcan = 0;

/* PHASE II PARAMETERS */

int Ltahoe_correct_profile = 0;
double Ltahoe_padding_threshold = 0.0;	/* pad everything */
int Ltahoe_debug_stack_frame = 0;
int Ltahoe_check_for_stop_bits;
int Ltahoe_do_postreg_const_fold = 1;
int Ltahoe_clobber_unats = 1;

int Ltahoe_insert_branch_hints = 0;
int Ltahoe_print_hint_info = 1;
int Ltahoe_aggressive_hints = 0;
int Ltahoe_use_many_hint_on_all_branches = 0;
int Ltahoe_use_many_hint_on_call = 0;
int Ltahoe_use_many_hint_on_return = 0;
int Ltahoe_use_many_hint_on_brp = 0;
int Ltahoe_use_imp_hint_on_brp = 0;
int Ltahoe_dont_expand_for_hints = 1;
int Ltahoe_min_fe_cycles_for_prefetch_brp = 9;
int Ltahoe_use_counted_prefetch_hints = 0;
int Ltahoe_use_streaming_only = 0;
int Ltahoe_advanced_prefetch = 0;
int Ltahoe_insert_with_full_coverage = 0;
int Ltahoe_insert_with_retries = 0;
int Ltahoe_mckinley_hints = 0;
double Ltahoe_dp_upper_prob = 0.97;
double Ltahoe_dp_lower_prob = 0.03;

/* PHASE III PARAMETERS */

int Ltahoe_force_recovery_execution = 0;	/* Used to test recovery blocks */
int Ltahoe_force_recovery_upper = 1000000;	/* Used to test recovery blocks */
int Ltahoe_force_recovery_lower = 0;	/* Used to test recovery blocks */
int Ltahoe_generate_unwind_directives = 1;	/* Generate stack unwind info */
int Ltahoe_print_characteristics = 1;	/* print cb header */
int Ltahoe_print_live_registers = 0;	/* print live registers on each cb */
int Ltahoe_print_issue_time = 1;	/* print TAHOE scheduler issue times */
int Ltahoe_print_latency = 0;	/* print TAHOE scheduler latency */
int Ltahoe_print_op_id = 0;	/* print op id */
int Ltahoe_print_offset = 0;	/* print instr # from begin of cb */
int Ltahoe_print_iteration = 0;	/* print iteration in unroll loop */
int Ltahoe_print_real_regs = 0;	/* 1 --> all registers print as r\d+
				 * 0 --> stacked registers print as 
				 * in\d+,loc\d+,out\d+, and gpsav,
				 * prsavm psp, and sp are used.
				 */
int Ltahoe_generate_map = 0;
/* 12/02/02 REK This parm determines whether we generate code for GNU's as or
 *              Intel's ias. */
int Ltahoe_output_for_ias = 0;
int Ltahoe_print_cache_stats = 0;
int Ltahoe_tag_loads = 0;

int Ltahoe_machine_opt_mask = 0xFFFFFFFF;

/* Ltahoe_read_parm (Parm_Parse_Info * ppi)
 * ----------------------------------------------------------------------
 * Read parameters from parameter files and command line 
 */

void
Ltahoe_read_parm (Parm_Parse_Info * ppi)
{
  /* Phase 1 parms
   * ---------------------------------------------------------------------- 
   */

  L_read_parm_b (ppi, "use_gp_rel_addressing", &Ltahoe_use_gp_rel_addressing);
  L_read_parm_b (ppi, "input_param_subst", &Ltahoe_input_param_subst);
  L_read_parm_b (ppi, "do_lcode_peephole", &Ltahoe_do_lcode_peephole);
  L_read_parm_b (ppi, "prologue_merge", &Ltahoe_prologue_merge);
  L_read_parm_b (ppi, "postinc_ld", &Ltahoe_postinc_ld);
  L_read_parm_b (ppi, "postinc_st", &Ltahoe_postinc_st);
  L_read_parm_b (ppi, "print_opti_stats", &Ltahoe_print_opti_stats);
  L_read_parm_b (ppi, "add_mov_ap", &Ltahoe_add_mov_ap);
  L_read_parm_b (ppi, "do_lightweight_pred_opti",
		 &Ltahoe_do_lightweight_pred_opti);
  L_read_parm_b (ppi, "do_repeated_mopti", &Ltahoe_do_repeated_mopti);
  L_read_parm_b (ppi, "?vulcan", &Ltahoe_vulcan);

  L_read_parm_b (ppi, "bitopt", &Ltahoe_bitopt);
  L_read_parm_b (ppi, "do_redux", &Ltahoe_do_redux);
  L_read_parm_b (ppi, "do_tbit_redux", &Ltahoe_do_tbit_redux);
  L_read_parm_b (ppi, "do_extr_redux", &Ltahoe_do_extr_redux);
  L_read_parm_b (ppi, "do_depo_redux", &Ltahoe_do_depo_redux);
  L_read_parm_b (ppi, "do_ldf_redux", &Ltahoe_do_ldf_redux);
  L_read_parm_b (ppi, "do_sp_removal", &Ltahoe_do_sp_removal);

  L_read_parm_b (ppi, "fp_ftz", &Ltahoe_fp_ftz);

  /* Phase 2 parms
   * ----------------------------------------------------------------------
   */

  L_read_parm_b (ppi, "correct_profile_info", &Ltahoe_correct_profile);
  L_read_parm_lf (ppi, "padding_threshold", &Ltahoe_padding_threshold);
  L_read_parm_b (ppi, "debug_stack_frame", &Ltahoe_debug_stack_frame);
  L_read_parm_b (ppi, "check_for_stop_bits", &Ltahoe_check_for_stop_bits);

  L_read_parm_b (ppi, "do_postreg_const_fold", &Ltahoe_do_postreg_const_fold);
  L_read_parm_b (ppi, "clobber_unats", &Ltahoe_clobber_unats);

  /* Branch hint parms
   * ---------------------------------------------------------------------- 
   */

  L_read_parm_b (ppi, "insert_branch_hints", &Ltahoe_insert_branch_hints);
  L_read_parm_b (ppi, "print_hint_info", &Ltahoe_print_hint_info);
  L_read_parm_b (ppi, "aggressive_hints", &Ltahoe_aggressive_hints);
  L_read_parm_b (ppi, "use_many_hint_on_all_branches",
		 &Ltahoe_use_many_hint_on_all_branches);
  L_read_parm_b (ppi, "use_many_hint_on_call", &Ltahoe_use_many_hint_on_call);
  L_read_parm_b (ppi, "use_many_hint_on_return",
		 &Ltahoe_use_many_hint_on_return);
  L_read_parm_b (ppi, "use_many_hint_on_brp", &Ltahoe_use_many_hint_on_brp);
  L_read_parm_b (ppi, "use_imp_hint_on_brp", &Ltahoe_use_imp_hint_on_brp);
  L_read_parm_b (ppi, "dont_expand_for_hints", &Ltahoe_dont_expand_for_hints);
  L_read_parm_i (ppi, "min_fe_cycles_for_prefetch_brp",
		 &Ltahoe_min_fe_cycles_for_prefetch_brp);
  L_read_parm_b (ppi, "use_counted_prefetch_hints",
		 &Ltahoe_use_counted_prefetch_hints);
  L_read_parm_b (ppi, "use_streaming_only", &Ltahoe_use_streaming_only);
  L_read_parm_b (ppi, "advanced_prefetch", &Ltahoe_advanced_prefetch);
  L_read_parm_b (ppi, "insert_with_full_coverage",
		 &Ltahoe_insert_with_full_coverage);
  L_read_parm_b (ppi, "insert_with_retries", &Ltahoe_insert_with_retries);
  L_read_parm_b (ppi, "mckinley_hints", &Ltahoe_mckinley_hints);

  L_read_parm_lf (ppi, "dynamic_hint_upper_prob", &Ltahoe_dp_upper_prob);
  L_read_parm_lf (ppi, "dynamic_hint_lower_prob", &Ltahoe_dp_lower_prob);

  /* Phase 3 parms
   * ---------------------------------------------------------------------- 
   */

  L_read_parm_b (ppi, "generate_unwind_directives",
		 &Ltahoe_generate_unwind_directives);
  L_read_parm_b (ppi, "force_recovery_execution",
		 &Ltahoe_force_recovery_execution);
  L_read_parm_i (ppi, "force_recovery_upper", &Ltahoe_force_recovery_upper);
  L_read_parm_i (ppi, "force_recovery_lower", &Ltahoe_force_recovery_lower);
  L_read_parm_b (ppi, "print_characteristics", &Ltahoe_print_characteristics);
  L_read_parm_b (ppi, "print_live_registers", &Ltahoe_print_live_registers);
  L_read_parm_b (ppi, "print_issue_time", &Ltahoe_print_issue_time);
  L_read_parm_b (ppi, "print_latency", &Ltahoe_print_latency);
  L_read_parm_b (ppi, "print_op_id", &Ltahoe_print_op_id);
  L_read_parm_b (ppi, "print_offset", &Ltahoe_print_offset);
  L_read_parm_b (ppi, "print_iteration", &Ltahoe_print_iteration);
  L_read_parm_b (ppi, "print_real_regs", &Ltahoe_print_real_regs);
  L_read_parm_b (ppi, "output_for_ias", &Ltahoe_output_for_ias);

  L_read_parm_b (ppi, "print_cache_stats", &Ltahoe_print_cache_stats);
  L_read_parm_b (ppi, "tag_loads", &Ltahoe_tag_loads);

  /* Debug parms
   * ----------------------------------------------------------------------
   */

  L_read_parm_i (ppi, "?machine_opt_mask", &Ltahoe_machine_opt_mask);

  if (Ltahoe_machine_opt_mask != 0xFFFFFFFF)
    L_warn ("Machine opt mask is 0x%08X -- some optis may be disabled.",
	    Ltahoe_machine_opt_mask);
}


/* Ltahoe_read_parm (Parm_Parse_Info * ppi)
 * ----------------------------------------------------------------------
 * Read parameters from parameter files and command line 
 */

void
Ltahoe_init_version (void)
{
  switch (M_model)
    {
    case M_IPF_ITANIUM:
    case M_IPF_MCKINLEY:
    case M_IPF_MADISON:
    case M_IPF_DEERFIELD:
      break;
    default:
      L_punt
	("ILLEGAL architecture/model specified for this code generator!");
    }
}

extern void LB_read_parm_lblock (Parm_Parse_Info * ppi);

/* 10/08/02 REK Moving the initialization code to its own function. */
void
Ltahoe_init (Parm_Macro_List * command_line_macro_list)
{
  /* Initialization
   * ---------------------------------------------------------------------- 
   */

  if (M_arch != M_TAHOE)
    L_punt ("Ltahoe requires M_arch == M_TAHOE");

  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Mcode", L_read_parm_mcode);

  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Ltahoe", Ltahoe_read_parm);

  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Mopti", L_read_parm_mopti);

  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Lblock", LB_read_parm_lblock);

  Lopti_init ();

  /* parameter checking and overrides */

  if (L_do_recovery_code && Ltahoe_clobber_unats)
    L_punt ("do_recovery_code and clobber_unats are mutually exclusive!");

  if (!L_do_recovery_code)
    L_generate_spec_checks = 0;
  else if (!L_generate_spec_checks)
    L_punt ("generate_spec_checks is required for do_recovery_code!");

  Ltahoe_init_version ();

  if (L_codegen_phase == P_NONE)
    L_codegen_phase = P_ALL;

  if (L_codegen_phase < 0 || L_codegen_phase > 7)
    L_punt ("L_gen_code: Invalid code generation phase");

  L_init_lmdes2 (L_lmdes_file_name, L_max_pred_operand, L_max_dest_operand,
		 L_max_src_operand, 4 /* Max 4 sync operands */ );

  if (lmdes->mdes2 == NULL)
    L_punt ("Ltahoe: Only supports .lmdes2 files now.\n"
	    "Cannot use '%s'\n", lmdes->file_name);

  Ltahoe_init_phase (command_line_macro_list, L_codegen_phase);
}				/* Ltahoe_init */


/* 10/09/02 REK A function to initialize each Ltahoe phase. */
void
Ltahoe_init_phase (Parm_Macro_List * command_line_macro_list,
		   int codegen_phase)
{
  if (codegen_phase & P_1)
    L_init (command_line_macro_list);
  if (codegen_phase & P_2)
    O_init (command_line_macro_list);
  if (codegen_phase & P_3)
    {
      P_init (command_line_macro_list);
      P_file_init ();
    }
}				/* Ltahoe_init_phase */


/* 10/08/02 REK A function to perform cleanup duty. */
void
Ltahoe_cleanup (void)
{
  if (L_codegen_phase & P_1)
    L_end ();
  if (L_codegen_phase & P_2)
    O_finalize ();
  if (L_codegen_phase & P_3)
    {
      P_end ();
      P_file_end ();
    }
}				/* Ltahoe_cleanup */


/* Ltahoe_debug (char *fmt, ...)
 * ----------------------------------------------------------------------
 * Print a formatted debugging message, with the current time, to
 * stderr Flush stderr.  
 */

void
Ltahoe_debug (char *fmt, ...)
{
  va_list args;
  time_t now;
  char timebuf[64];

  time (&now);
  strftime (timebuf, 64, "%Y%m%d %H:%M:%S", localtime (&now));
  fprintf (stderr, ">LtD %s> ", timebuf);
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  fflush (stderr);
  return;
}


/* 10/08/02 REK Defining a function to run the three process_func phases.  This
 *              is so that external modules can run these without knowing that
 *              there are three phases.
 */
void
Ltahoe_process_func (L_Func * fn, Parm_Macro_List * command_line_macro_list)
{
  L_process_func (L_fn, command_line_macro_list);
  O_process_func (L_fn, command_line_macro_list);
  P_process_func (L_fn);
}				/* Ltahoe_process_func */


/* 10/08/02 REK Defining a function to run P_process_data.  This is simply
 *              a wrapper so that external modules can call P_process_data
 *              without knowing about Ltahoe's different phases.
 */
void
Ltahoe_process_data (FILE * F_OUT, L_Data * data)
{
  P_process_data (F_OUT, data);
}				/* Ltahoe_process_data */
