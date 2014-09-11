/******************************************************************************\
 *
 *  File:  lhpl_pd_main.h
 *
 *  Description:
 *    Driver module include files for Lhpl_pd code generation.
 *
 *  Creation Date :  August 1993
 *
 *  Author:  Scott A. Mahlke
 *
\******************************************************************************/
#ifndef LHPL_PD_MAIN_H
#define LHPL_PD_MAIN_H

#include <Lcode/l_main.h>
#include <Lcode/lhpl_pd_phase1.h>
#include <Lcode/lhpl_pd_phase2.h>
#include <Lcode/lhpl_pd_phase3.h>
#include <Lcode/r_regalloc.h>
#include <Lcode/l_schedule.h>
#include <Lcode/l_loop_prep.h>
#include <Lcode/l_softpipe.h>
#include <Lcode/m_opti.h>

#include <machine/m_hpl_pd.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char CurrentFunction[];

/*
 *	Declare code generator specific parameter variables
 */
extern int L_debug_messages;
extern int L_do_machine_opt;
extern int L_do_prepass_sched;
extern int L_do_register_allocation;
extern int L_do_postpass_code_annotation;
extern int L_do_peephole_opt;
extern int L_do_fill_squashing_branches;
extern int L_do_fill_non_squashing_branches;
extern int L_do_fill_unfilled_branches;


/*
 *	Lplaydoh specific parameter variables
 */
extern int Lplaydoh_num_prd_caller_reg;
extern int Lplaydoh_num_prd_callee_reg;
extern int Lplaydoh_num_btr_caller_reg;
extern int Lplaydoh_num_btr_callee_reg;
extern int Lplaydoh_num_int_reg;
extern int Lplaydoh_num_flt_reg;
extern int Lplaydoh_num_dbl_reg;
extern int Lplaydoh_num_ctl_reg;
extern int Lplaydoh_preserve_lcode_ids;
extern int Lplaydoh_breakup_single_bb_loops;
extern int Lplaydoh_convert_to_strict_bb_code;
extern int Lplaydoh_build_sync_arcs;
extern int Lplaydoh_use_base_disp;
extern int Lplaydoh_use_sign_ext;
extern int Lplaydoh_retain_post_inc;

/*
 *	Function prototypes
 */
extern void L_read_parm_lplaydoh ( Parm_Parse_Info *ppi );
extern void L_gen_code ( Parm_Macro_List *command_line_macro_list );
extern void Lhpl_pd_adjust_fp_parameter_passing(L_Func *fn);

#ifdef __cplusplus
}
#endif

#endif
