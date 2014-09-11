/******************************************************************************\
 *
 *  File:  lhpl_pd_main.c
 *
 *  Description:
 *    Driver module for Lhpl_pd code generator
 *
 *  Creation Date :  August 1993
 *
 *  Author:  Scott A. Mahlke
 *
 *  Description:
 *    This is the main entry point for the code generator.  It is designed to
 *    be generic.  It permits calling one time initialization routines for each
 *    of the phases.  The calling convention and parameters are discussed below.
 *
 *	Phase 1 :  Annotation from Lcode to Mcode
 *	Phase 2 :  Mcode optimizations, register allocation, 
 *		    instruction scheduling
 *	Phase 3 :  Convert Mcode to target assembly code.
 *
 *	Example:
 *		Lamd29k -verbose -target -c 4 -i in.lc -o out.mco
 *
 *	Execution of these phases is controlled by the following variable:
 *	  int L_codegen_phase;
 *	1 = phase 1 only 	(.lc input, .mc output)
 *	2 = phase 2 only	(.mc input, .mco output)
 *	4 = phase 3 only	(.mc input, .s output)
 *
 *	NOTE:  The extensions listed in () following the numeric values is the
 *	input and output convention.  It is up to the user to maintain this
 *	convention.
 *
\******************************************************************************/
#include "lhpl_pd_main.h"

char	CurrentFunction[1024];

int	Lplaydoh_num_prd_caller_reg = 0;
int	Lplaydoh_num_prd_callee_reg = 0;
int	Lplaydoh_num_btr_caller_reg = 0;
int	Lplaydoh_num_btr_callee_reg = 0;
int	Lplaydoh_num_int_reg = 0;
int	Lplaydoh_num_flt_reg = 0;
int	Lplaydoh_num_dbl_reg = 0;
int	Lplaydoh_num_ctl_reg = 0;
int 	Lplaydoh_preserve_lcode_ids = 1;
int	Lplaydoh_breakup_single_bb_loops = 1;
int	Lplaydoh_convert_to_strict_bb_code = 0;
int     Lplaydoh_build_sync_arcs = 0;
int     Lplaydoh_use_base_disp = 0;
int     Lplaydoh_use_sign_ext = 0;
int     Lplaydoh_retain_post_inc = 0;

#define P_NONE		0x00	/* all phases */
#define P_1		0x01
#define P_2		0x02
#define P_3		0x04
#define P_ALL		0x07	/* all phases */

char *phase_message[8] = 
{
    " NONE!", 
    " 1 only", 
    " 2 only", 
    "s 1 and 2",
    " 3 only", 
    "s 1 and 3",
    "s 2 and 3",
    "s 1, 2 and 3"
};
  
void L_read_parm_lplaydoh (Parm_Parse_Info *ppi)
{
    L_read_parm_i(ppi, "num_int_reg",
			&Lplaydoh_num_int_reg);
    L_read_parm_i(ppi, "num_flt_reg",
			&Lplaydoh_num_flt_reg);
    L_read_parm_i(ppi, "num_dbl_reg",
			&Lplaydoh_num_dbl_reg);
    L_read_parm_i(ppi, "num_prd_caller_reg",
			&Lplaydoh_num_prd_caller_reg);
    L_read_parm_i(ppi, "num_prd_callee_reg",
			&Lplaydoh_num_prd_callee_reg);
    L_read_parm_i(ppi, "num_btr_caller_reg",
			&Lplaydoh_num_btr_caller_reg);
    L_read_parm_i(ppi, "num_btr_callee_reg",
			&Lplaydoh_num_btr_callee_reg);
    L_read_parm_b(ppi, "preserve_lcode_ids",
			&Lplaydoh_preserve_lcode_ids);
    L_read_parm_b(ppi, "breakup_single_bb_loops",
			&Lplaydoh_breakup_single_bb_loops);
    L_read_parm_b(ppi, "convert_to_strict_bb_code",
			&Lplaydoh_convert_to_strict_bb_code);
    L_read_parm_b(ppi, "build_sync_arcs",
			&Lplaydoh_build_sync_arcs);
    L_read_parm_b(ppi, "use_base_disp",
			&Lplaydoh_use_base_disp);
    L_read_parm_b(ppi, "use_sign_ext",
			&Lplaydoh_use_sign_ext);
    L_read_parm_b(ppi, "retain_post_inc",
			&Lplaydoh_retain_post_inc);
}

/*
 *  L_gen_code is the entry point to code generation called from l_main.c
 */
void L_gen_code(Parm_Macro_List *command_line_macro_list)
{

    if (M_arch!=M_PLAYDOH)
        L_punt("ILLEGAL architecture specified for this code generator!");

    /* Load the parameters specific to Lcode code generation */
    L_load_parameters (L_parm_file, command_line_macro_list,
                       "(Mcode", L_read_parm_mcode);
    L_load_parameters (L_parm_file, command_line_macro_list,
		       "(Lhpl_pd", L_read_parm_lplaydoh);
 
    /* check for invalid parameters */
    if ((L_codegen_phase < P_NONE) || (L_codegen_phase > P_ALL))
        L_punt("L_gen_code: Invalid code generation phase");

    L_open_input_file(L_input_file);

    /*
     * Perform global initialization 
     */
    if ((L_codegen_phase == 0) || (L_codegen_phase & P_1))
        L_init(command_line_macro_list);
  
    if ((L_codegen_phase == 0) || (L_codegen_phase & P_2))
        O_init(command_line_macro_list);

    if ((L_codegen_phase == 0) || (L_codegen_phase & P_3))
        P_init();

    if ( (L_do_register_allocation == 0) && (L_do_postpass_sched) &&
         (Lsched_do_postpass_scheduling) )  {
        /* The conflicting operands functions assume that register allocation has */
        /* been done while building the dependence graph for postpass scheduling  */
        /* which can result in incorrect floating point depedences...             */
        L_punt("L_gen_code: Cannot do postpass scheduling without register allocation!!!\n");
    }

    /* Process all data and functions within a file */
    while (L_get_input() != L_INPUT_EOF)
    {
        if (L_token_type==L_INPUT_FUNCTION) 
	{
	    strcpy(CurrentFunction,L_fn->name);
            if (L_debug_messages)
                fprintf (stderr, "Processing %s - phase%s\n", L_fn->name,
		    phase_message[L_codegen_phase]); 

	    /* Sometimes the L_CB_HYPERBLOCK_NOFALLTHRU flag is
               incorrect, this call corrects it for scheduling and
               reg allocation so there are no punts.  Its basically
               a backward compatibility for benchmarks generated before
               the bug was fixed... SAM 11-94 */
            L_set_hb_no_fallthru_flag(L_fn);

	    /* hyperblocks that have no predicate defines cause problems
               for dataflow analysis.  So insert a dummy op as the
               first oper of these blocks... SAM 11-94 */
            L_insert_dummy_op_into_hyperblocks(L_fn);

	    /* Mark all jsrs to setjmp/longjmp with <Y> flag */
	    L_find_synchronization_sub_calls(L_fn);

	    if (L_codegen_phase & P_1)
		L_process_func(L_fn);

	    if (L_codegen_phase & P_2)
		O_process_func(L_fn, command_line_macro_list);

	    if (L_codegen_phase & P_3)
		P_process_func(L_fn);
	    else  {
		L_print_func(L_OUT, L_fn);
	    }

	    L_delete_func(L_fn);
        }
        else
        {  /* 
	    * We will only process the data segments if we are going to
	    * perform phase 3.  Otherwise, we will just print the data
	    * into the new file.
	    */
	    if ((L_codegen_phase == 0) || (L_codegen_phase & P_3))
                P_process_data(L_OUT, L_data);
	    else
                L_print_data(L_OUT, L_data);
	    L_delete_data(L_data);
        }

    }
    
    L_close_input_file(L_input_file);
}
