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
 * ltahoe_main.h                                                             *
 * ------------------------------------------------------------------------- *
 * Itanium Processor Family code generator master include file               *
 *                                                                           *
 * AUTHORS: S.A. Mahlke, D.A. Connors, J.W. Sias                             *
 *****************************************************************************/
/* 10/08/02 REK Declaring initialization and cleanup functions.
 *              Also declaring function so that external modules can call
 *              the process_func and process_data functions without knowing
 *              that there are three phases internally.
 *              Moving the phase selection macros here from ltahoe_main.c.
 */
/* 10/09/02 REK Declaring the read_parm functions so that they can be called
 *              by external modules.
 *              Adding declaration for Ltahoe_init_version.
 */

#ifndef LTAHOE_MAIN_H_
#define LTAHOE_MAIN_H_

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/mia_opti.h>
#include <library/func_list.h>
#include <machine/m_tahoe.h>

#ifndef IT64BIT
#error Ltahoe requires 64-bit host support -- make sure IA64BIT is set! (JWS)
#endif

/* 10/08/02 REK Moving the phase selection macros in from ltahoe_main.c. */
/* PHASE SELECTION */

#define P_NONE    0x00		/* all phases */
#define P_1       0x01
#define P_2       0x02
#define P_3       0x04
#define P_1_2     0x03
#define P_ALL     0x07		/* all phases */

/* USE_IMPLICIT_NOSCHED
 * ----------------------------------------------------------------------
 * If 0, produce explicitly-bundled assembly output, with one non-nop
 * instruction per bundle, when postpass scheduling is turned off.  If
 * 1, rely on the assembler to insert stop bits and to bundle.
 */

#define USE_IMPLICIT_NOSCHED 0

/* Ltahoe parameters
 * ----------------------------------------------------------------------
 */

/* Mcode parameters defined in Lcode/Lcode/l_mcode.h */

/* PHASE I PARAMETERS */

extern int Ltahoe_use_gp_rel_addressing;
extern int Ltahoe_prologue_merge;
extern int Ltahoe_input_param_subst;
extern int Ltahoe_do_lcode_peephole;
extern int Ltahoe_postinc_ld;
extern int Ltahoe_postinc_st;
extern int Ltahoe_print_opti_stats;
extern int Ltahoe_add_mov_ap;
extern int Ltahoe_do_lightweight_pred_opti;
extern int Ltahoe_do_repeated_mopti;
extern int Ltahoe_do_redux;
extern int Ltahoe_do_tbit_redux;
extern int Ltahoe_do_extr_redux;
extern int Ltahoe_do_depo_redux;
extern int Ltahoe_do_ldf_redux;
extern int Ltahoe_do_sp_removal;
extern int Ltahoe_fp_ftz;
extern int Ltahoe_bitopt;
extern int Ltahoe_vulcan;

/* PHASE II PARAMETERS */

extern int Ltahoe_correct_profile;
extern double Ltahoe_padding_threshold;
extern int Ltahoe_debug_stack_frame;
extern int Ltahoe_check_for_stop_bits;
extern int Ltahoe_do_postreg_const_fold;
extern int Ltahoe_clobber_unats;

extern int Ltahoe_insert_branch_hints;
extern int Ltahoe_print_hint_info;
extern int Ltahoe_aggressive_hints;
extern int Ltahoe_use_many_hint_on_all_branches;
extern int Ltahoe_use_many_hint_on_call;
extern int Ltahoe_use_many_hint_on_return;
extern int Ltahoe_use_many_hint_on_brp;
extern int Ltahoe_use_imp_hint_on_brp;
extern int Ltahoe_dont_expand_for_hints;
extern int Ltahoe_min_fe_cycles_for_prefetch_brp;
extern int Ltahoe_use_counted_prefetch_hints;
extern int Ltahoe_use_streaming_only;
extern int Ltahoe_advanced_prefetch;
extern int Ltahoe_insert_with_full_coverage;
extern int Ltahoe_insert_with_retries;
extern int Ltahoe_mckinley_hints;
extern double Ltahoe_dp_upper_prob;
extern double Ltahoe_dp_lower_prob;

/* PHASE III PARAMETERS */

extern int Ltahoe_force_recovery_execution;	/* Test recovery blocks       */
extern int Ltahoe_force_recovery_upper;	/* Test recovery blocks       */
extern int Ltahoe_force_recovery_lower;	/* Test recovery blocks       */
extern int Ltahoe_generate_unwind_directives;	/* Generate stack unwind info */
extern int Ltahoe_print_characteristics;	/* print cb header            */
extern int Ltahoe_print_live_registers;	/* print live regs on each cb */
extern int Ltahoe_print_issue_time;	/* print op issue times       */
extern int Ltahoe_print_latency;	/* print op latency           */
extern int Ltahoe_print_op_id;	/* print op id                */
extern int Ltahoe_print_offset;	/* print instr's index in cb  */
extern int Ltahoe_print_iteration;	/* print iter in unroll loop  */
extern int Ltahoe_print_real_regs;	/* use r- format for all regs */
extern int Ltahoe_generate_map;	/* generate map file          */
extern int Ltahoe_output_for_ias; /* Tweak the assembly output for ias */

extern int Ltahoe_print_cache_stats; /*Include cache miss info w/ loads*/
extern int Ltahoe_tag_loads; /*Generates load tables and tags loads*/

extern int Ltahoe_machine_opt_mask;

/* COMMON UTILITIES */

#define Ltahoe_new_reg(cty) L_new_register_operand (++(L_fn->max_reg_id), \
					    (cty), L_PTYPE_NULL)

#define Ltahoe_new_int_reg() L_new_register_operand (++(L_fn->max_reg_id), \
					    L_CTYPE_LLONG, L_PTYPE_NULL)

#define Ltahoe_new_pred_reg(ptype) L_new_register_operand (++(L_fn-> \
                                            max_reg_id), \
					    L_CTYPE_PREDICATE, (ptype))

#define Ltahoe_copy_or_new(cop,opd) (cop) = ((opd) == NULL) ? \
                                            Ltahoe_new_int_reg() : \
                                            L_copy_operand ((opd))

#define Ltahoe_true_pred(ptype) L_new_macro_operand (TAHOE_MAC_PRED_TRUE, \
                                            L_CTYPE_PREDICATE, \
				            (ptype))

#define Ltahoe_IMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_LLONG, 0)
#define Ltahoe_PMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_PREDICATE, 0)
#define Ltahoe_FMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_DOUBLE, 0)
#define Ltahoe_DMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_DOUBLE, 0)
#define Ltahoe_BMAC(mac) L_new_macro_operand (TAHOE_MAC_ ## mac, \
                                              L_CTYPE_BTR, 0)

/* This is used to clear flags from new opers resulting from annotation.
 * These opers need not inherit the flags of their parents. Use this carefully!
 */
#define CLEAR_FLAGS(oper) ((oper)->flags = \
                           L_CLR_BIT_FLAG( (oper)->flags, \
                                           L_OPER_SPILL_CODE | \
                                           L_OPER_SAFE_PEI | \
                                           L_OPER_MASK_PE | \
                                           L_OPER_SIDE_EFFECT_FREE | \
                                           L_OPER_LABEL_REFERENCE | \
                                           L_OPER_STACK_REFERENCE | \
                                           L_OPER_PROBE_MARK ))

#define L_get_local_space_size( fn ) ((fn)->s_local)

/* 10/08/02 REK Declaring the initialization function so that outside modules
 *              can initialize and cleanup Ltahoe. */
extern void Ltahoe_init (Parm_Macro_List *);
/* 10/09/02 REK Declaring a function to initialize each phase. */
extern void Ltahoe_init_phase (Parm_Macro_List *, int);
extern void Ltahoe_cleanup (void);
/* 10/09/02 REK Declaring functions to read the parameters for Ltahoe. */
extern void Ltahoe_read_parm (Parm_Parse_Info *);
extern void Ltahoe_init_version (void);
/* 10/08/02 REK Declaring a function to run Ltahoe phases 1, 2, and 3.  This
 *              function is declared here so that external modules can call
 *              these phases without having to care that there are three
 *              phases.
 */
extern void Ltahoe_process_func (L_Func *, Parm_Macro_List *);
/* 10/08/02 REK Declaring a function to run Ltahoe phase 3 (data).  This
 *              function is only a wrapper around P_process_data at this point,
 *              but it is needed to allow external modules to run
 *              P_process_data without knowing about the phases.
 */
extern void Ltahoe_process_data (FILE *, L_Data *);

/* DEBUGGING / MESSAGING SUPPORT */

extern void Ltahoe_debug (char *, ...);

#define LTD if (L_debug_messages) Ltahoe_debug

#endif
