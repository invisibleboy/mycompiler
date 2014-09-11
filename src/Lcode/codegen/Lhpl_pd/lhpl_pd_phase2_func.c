/*************************************************************************\
 *
 *  File:  lhpl_pd_phase2_func.c
 *
 *  Description:
 *    Machine specific annotation, optimization and interfaces to scheduler/register
 *    allocator  for HPL_PD architecture.
 *
 *  Creation Date :  August 1993
 *
 *  Author:  Scott A. Mahlke
 *
 *
\************************************************************************/
#include "lhpl_pd_main.h"

static void L_delete_attr_from_opers(L_Func *fn, char *name)
{
    L_Cb *cb;
    L_Oper *oper;
    L_Attr *attr;

    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) {
	for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
	    attr = L_find_attr(oper->attr, name);
	    if (attr!=NULL)
		oper->attr = L_delete_attr(oper->attr, attr);
	}
    }
}

/******************************************************************************\
 *
 * Perform post "register allocation" mcode annotion:
 *
 * This annotation is specific to the HP PA-RISC processer. Currently annotation
 * generates the function prologue and epilogue, and adjusts stack frame 
 * addresses.
 *
\******************************************************************************/

void O_annotate(L_Func *fn)
{
    
}

/******************************************************************************\
 *
 * Does everything for a function
 *
\******************************************************************************/

void O_perform_init(L_Func *fn)
{
}

/*
 *	Currently only prepass, reg alloc, postpass scheduling.  Other normal
 *	stuff done during code generation is omitted from AVLIW code generator
 */
void O_process_func(L_Func *fn, Parm_Macro_List *command_line_macro_list)
{

    if (L_debug_messages)
      fprintf(stderr, "Optimizing %s\n", fn->name);

    /*
     * Perform any necessary function level initialization
     */
    O_perform_init(fn);

    /*
     * Perform machine level code optimizations.
     *
     * Here is where we will perform:
     * 1) common subexpression ellimination.
     * 2) limited copy propogation (only R-R, R-M, M-R, M-M)
     * 3) dead code removal (unused operations, src[0]=dest[0])
     *
     */
    if (L_do_machine_opt)  {
	Mopti_perform_optimizations(fn, command_line_macro_list);
    }
	
    /*
     * Perform pre-regalloc peep-hole optimization
     *
     */
    if (L_do_peephole_opt) {
    }


    /*
     * Software Pipelining
     */

    if (L_do_software_pipelining) {
        if (L_debug_messages)
            fprintf(stderr, "  Software pipelining...");

        Lpipe_software_pipeline(fn);

        if (L_debug_messages)
            fprintf(stderr, "done\n");
    }


    /*
     * Pre-pass code scheduling:
     *
     */
    if (L_do_prepass_sched) {
        if (L_debug_messages)
            fprintf(stderr, "  Pre-pass code scheduling...");
	
	Lsched_prepass_code_scheduling(fn);


        if (L_debug_messages)
            fprintf(stderr, "done\n");
    }

    /*
     * Perform register allocation
     *
     * global information available after register allocation
     *
     * spill_space_required
     * number_of_registers
     */
    if (L_do_register_allocation) {
        if (L_debug_messages) {
            fprintf(stderr, "  Register Allocation\n");
            fprintf(stderr, "  ======================================\n");
        }
        O_register_allocation(fn, command_line_macro_list);


        if (L_debug_messages)
            fprintf(stderr, "  --------------------------------------\n");
    }

    /*
     * Reset L_CB_SOFTPIPE flag for software pipelined loops that contain
     * spill code so that postpass scheduling can schedule the spill code
     */

    if (L_do_software_pipelining) {
        if (L_debug_messages)
            fprintf(stderr, "  Checking for spill code in pipelined loops...");

        Lpipe_mark_loops_with_spills(fn);

        if (L_debug_messages)
            fprintf(stderr, "done\n");
    }

    /*
     * Perform post "register allocation" code annotation.
     *
     * Certain commands such as prologue and epilogue can not be annotated
     * until all of the register characteristics and memory requirements
     * are known.
     *
     */
    if (L_do_postpass_code_annotation) {
	if (L_debug_messages)
            fprintf(stderr, "  Post-pass code annotation...");

        O_annotate(fn);

        if (L_debug_messages)
            fprintf(stderr, "done\n");
    }

    /*
     * Perform peep-hole optimization
     *
     * This is where we will perform peephole optimization
     */
    if (L_do_peephole_opt) {
    }

    if (L_do_prepass_sched && L_do_register_allocation && !L_do_postpass_sched) {
	L_delete_attr_from_opers(fn, "isl");
	fn->flags = L_CLR_BIT_FLAG(fn->flags, L_FUNC_SCHEDULED);
    }

    /*
     * Perform post-pass instruction scheduling and filling of delay slots.
     */
    if (L_do_postpass_sched) {
        if (L_debug_messages)
            fprintf(stderr, "  Post-pass code scheduling...");
	
	Lsched_postpass_code_scheduling(fn);

        if (L_debug_messages)
            fprintf(stderr, "done\n");
    }
}

/*
 * Global initializations
 */
void O_init(Parm_Macro_List *command_line_macro_list)
{
    O_register_init();
    
    Lsched_init(command_line_macro_list, L_lmdes_file_name);

    if (L_do_software_pipelining) {
        Lpipe_init(command_line_macro_list);
    }
}
