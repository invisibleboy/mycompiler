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
 *  File:  l_schedule.c
 *
 *  Description:  Major rewrite of top level scheduler for instruction 
 *	scheduling
 *
 *  Creation Date :  May 1993
 *
 *  Author:  Roger A. Bringmann
 *
 *
 * 	All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_schedule.h"
#include "l_mcb.h"

/* Debug flags */
int 	Lsched_debug_operation_priority = 0; 
int 	Lsched_debug_prepass_scheduling = 0; 
int 	Lsched_debug_postpass_scheduling = 0; 
int 	Lsched_debug_squashing_branches = 0;
int 	Lsched_debug_nonsquashing_branches = 0;
int 	Lsched_debug_unfilled_branches = 0;

/* JCG 5-4-95 */
int	Lsched_debug_use_sched_cb_bounds = 0;
int     Lsched_debug_lower_sched_cb_bound = 0;
int     Lsched_debug_upper_sched_cb_bound = 100000000;


/* Misc variables */
int	Lsched_num_branches_per_cycle = 0; 
int	Lsched_static_fall_thru_weight = 1; 
int	Lsched_pad_height = 1;
int	Lsched_use_fan_out = 1;
int	Lsched_infinite_issue = 0;
int 	Lsched_do_postpass_scheduling = 0;
int 	Lsched_demote_all_the_way = 0;
int 	Lsched_do_fill_squashing_branches = 0;
int 	Lsched_do_fill_nonsquashing_branches = 0;
int 	Lsched_do_fill_unfilled_branches = 0;
int 	Lsched_include_only_oper_lt_hb = 0;
int 	Lsched_print_statistics = 0;
int     Lsched_print_prepass_statistics = 0;
int 	Lsched_print_spec_condition = 0;
int 	Lsched_do_sentinel_recovery = 0;
int 	Lsched_prepass = 0;
int	Lsched_print_hb_and_spec = 0;
int	Lsched_print_cycle_delimiter = 0;	/* JCG 10/6/95 */
int 	Lsched_num_opers = 0;
int	Lsched_loads_each_cycle = 0;
int	Lsched_stores_each_cycle = 0;
int 	current_time = 0;
int	Lsched_infinite_issue_slot = 0;
int	Lsched_pad_vliw_slots_with_nops = 0;
int	Lsched_vliw_has_interlocking = 0;
int	Lsched_do_renumber_issue_slots = 0;
char    *Lsched_profile_info_to_use = NULL;
L_Oper	*Lsched_latest_br = NULL;

Squeue	*scheduled_queue,
	*priority_ready_queue,
	*regpres_ready_queue,
	*pending_ready_queue,
	*not_ready_queue;

int	Lsched_processor_model = MDES_SUPERSCALAR;
int	Lsched_total_issue_slots = 0;
float	Lsched_branch_increase = 0.0;
int	Lsched_fill_delay_slots=0;
int	Lsched_debug_messages=0;

int	Lmcb_keep_checks = 0;
/*
 * Other misc. variables
 */
L_Alloc_Pool	*Squeue_pool = NULL;
L_Alloc_Pool	*Sq_entry_pool = NULL;
L_Alloc_Pool	*Sched_Info_pool = NULL;
L_Alloc_Pool	*Operand_ready_pool = NULL;

/* dataflow interface function which performs crude analysis on
 * predicate registers to enable correct scheduling of branch combinined code
 */

/******************************************************************************\
 *
 * Special routines to determine the number of loads, stores and branches per
 * cycle from the current mdes.
 *
\******************************************************************************/

int Lsched_loads_per_cycle()
{
    if (Lsched_loads_each_cycle)
        return Lsched_loads_each_cycle;
    else
    return mdes_total_slots();
}

int Lsched_stores_per_cycle()
{
    if (Lsched_stores_each_cycle)
        return Lsched_stores_each_cycle;
    else
    return mdes_total_slots();
}

int Lsched_branches_per_cycle()
{
    if (Lsched_num_branches_per_cycle>0)
	return Lsched_num_branches_per_cycle;
    else
        return mdes_total_slots();
}

int Lsched_issue_time(L_Oper *oper)
{
    if (!L_do_prepass_sched || oper->ext==NULL)
        return -1;
    else
        return SCHED_INFO(oper)->issue_time;
}

int Lsched_completion_time(L_Oper *oper)
/* added to replace sinfo->completion_time which has been deleted. BLD 6/95 */
{
    if (!L_do_prepass_sched || oper->ext==NULL)
        return -1;
    else
        return (SCHED_INFO(oper)->issue_time + Lsched_latency(oper));
}

int Lsched_latency(L_Oper *oper)
/* returns the max latency over all the dest operands.  BLD 6/95 */
{
    int i, latency;
    Sched_Info  *sinfo;

    if (!L_do_prepass_sched || oper->ext==NULL)
	latency = -1;
    else
    {
        sinfo = SCHED_INFO(oper);
        latency = 0;
	for (i = 0; i < L_max_dest_operand; i++)
            if (oper->dest[i] != NULL) {
	        if (sinfo->relative_latency[i] > latency)
		    latency = sinfo->relative_latency[i];
            }
    }

    return latency;
}

/******************************************************************************\
 *
 * Initialization routines
 *
\******************************************************************************/

void L_read_parm_lsched (ppi)
Parm_Parse_Info *ppi;
{
    L_read_parm_b(ppi, "debug_operation_priority",
		&Lsched_debug_operation_priority);
    L_read_parm_b(ppi, "debug_prepass_scheduling", 
		&Lsched_debug_prepass_scheduling);
    L_read_parm_b(ppi, "debug_postpass_scheduling", 
		&Lsched_debug_postpass_scheduling);
    L_read_parm_b(ppi, "debug_squashing_branches", 
		&Lsched_debug_squashing_branches);
    L_read_parm_b(ppi, "debug_nonsquashing_branches", 
		&Lsched_debug_nonsquashing_branches);
    L_read_parm_b(ppi, "debug_unfilled_branches", 
		&Lsched_debug_unfilled_branches);

    /* JCG 5-4-95 */
    L_read_parm_b (ppi, "?debug_use_sched_cb_bounds", 
		   &Lsched_debug_use_sched_cb_bounds);
    L_read_parm_i (ppi, "?debug_lower_sched_cb_bound", 
		   &Lsched_debug_lower_sched_cb_bound);
    L_read_parm_i (ppi, "?debug_upper_sched_cb_bound", 
		   &Lsched_debug_upper_sched_cb_bound);

    L_read_parm_i(ppi, "branches_per_cycle", 
		&Lsched_num_branches_per_cycle);

    L_read_parm_b(ppi, "print_hb_and_spec", 
		&Lsched_print_hb_and_spec);
    L_read_parm_b(ppi, "print_statistics", 
		&Lsched_print_statistics);
    L_read_parm_b(ppi, "print_prepass_statistics",
                &Lsched_print_prepass_statistics);

    L_read_parm_b(ppi, "print_cycle_delimiter", 
		&Lsched_print_cycle_delimiter);

    L_read_parm_b(ppi, "print_spec_condition", 
		&Lsched_print_spec_condition);

    L_read_parm_b(ppi, "infinite_issue", 
		&Lsched_infinite_issue);

    L_read_parm_i(ppi, "static_fall_thru_weight", 
		&Lsched_static_fall_thru_weight);

    L_read_parm_i(ppi, "loads_per_cycle", 
		&Lsched_loads_each_cycle);
    L_read_parm_i(ppi, "stores_per_cycle", 
		&Lsched_stores_each_cycle);

    L_read_parm_s(ppi, "profile_info_to_use",
                &Lsched_profile_info_to_use);

    L_read_parm_b(ppi, "pad_vliw_slots_with_nops", 
		&Lsched_pad_vliw_slots_with_nops);
    L_read_parm_b(ppi, "vliw_has_interlocking", 
		&Lsched_vliw_has_interlocking);
    L_read_parm_b(ppi, "?do_renumber_issue_slots",
		  &Lsched_do_renumber_issue_slots);
    L_read_parm_b(ppi, "pad_height", 
		&Lsched_pad_height);
    L_read_parm_b(ppi, "use_fan_out", 
		&Lsched_use_fan_out);
    L_read_parm_b(ppi, "do_postpass_scheduling", 
		&Lsched_do_postpass_scheduling);
    L_read_parm_b(ppi, "do_fill_squashing_branches", 
		&Lsched_do_fill_squashing_branches);
    L_read_parm_b(ppi, "do_fill_nonsquashing_branches", 
		&Lsched_do_fill_nonsquashing_branches);
    L_read_parm_b(ppi, "do_fill_unfilled_branches", 
		&Lsched_do_fill_unfilled_branches);
    L_read_parm_b(ppi, "do_sentinel_recovery", 
		&Lsched_do_sentinel_recovery);
    L_read_parm_b(ppi, "use_register_pressure_heuristic",
                &Lsched_use_register_pressure_heuristic);
    L_read_parm_i(ppi, "register_pressure_threshhold",
                &Lsched_register_pressure_threshhold);
}

void Lsched_init(command_line_macro_list, lmdes_file_name)
Parm_Macro_List *command_line_macro_list;
char *lmdes_file_name;
{
    /* Perform mdes initialization (may be .lmdes or .lmdes2 file
     * for list scheduling) -JCG 4/7/98
     */
    L_init_lmdes2(lmdes_file_name, L_max_pred_operand, L_max_dest_operand,
		  L_max_src_operand, 4 /* Support up to 4 sync operands */);

    /* Perform dependence initialization */
    Ldep_init(command_line_macro_list);

    /* Initialize parameters used for scheduling */
    Lsched_processor_model = mdes_processor_model();
    Lsched_total_issue_slots = mdes_total_slots();

    /* Load the parameters specific to Lcode code generation */
    /* Renamed 'Scheduler' to 'Lschedule' -JCG 5/26/98 */
    L_load_parameters_aliased (L_parm_file, command_line_macro_list,
                       "(Lschedule", "(Scheduler", L_read_parm_lsched);

    /* Support computation of static priorities */
    Lsched_branch_increase = floor (1.0 / Lsched_branches_per_cycle());
 
    /* Initialize the integer equivalent to the scheduling model */
    if (L_spec_model == BASIC_BLOCK)
    {
	Ldep_allow_upward_code_perc = 0;
	Ldep_allow_downward_code_perc = 0;
    }
    else if ((L_spec_model == RESTRICTED) || (L_spec_model == GENERAL) ||
	     (L_spec_model == MCB))
    {
        Lsched_include_only_oper_lt_hb = 0;
    }

    else if (L_spec_model == WRITEBACK_SUPPRESSION)
    {
	Lsched_include_only_oper_lt_hb = 1;
    }
#if 0
    else if ((L_spec_model == SENTINEL) || (L_spec_model == SRB)) 
    {
        Lsched_include_only_oper_lt_hb = 0;
	Lsched_demote_all_the_way = Lsched_do_sentinel_recovery;
	L_sentinel_init();
    }
#endif

    /* Create memory pools for allocation of data structures */
    Sched_Info_pool = 
	L_create_alloc_pool ("Sched_Info", sizeof(Sched_Info), 100);
    Operand_ready_pool =
        L_create_alloc_pool ("operand_ready_times", 
	    mdes_latency_count()*sizeof(int), 100);

    /* initialize MCB memory pool */
    if (L_spec_model == MCB)
        L_mcb_init();

    /* Perform resource manager initialization */
    RU_set_max_pred(1 /* was L_max_pred_operand */);	/* hard code for 1 now */
    RU_map_create(0);

    /* Create queue pools */
    Squeue_pool = L_create_alloc_pool ("Squeue", sizeof (Squeue), 16);
    Sq_entry_pool = L_create_alloc_pool ("Sq_entry", sizeof(Sq_entry), 128);

    /* Create scheduling queues */
    scheduled_queue = L_create_queue ("scheduled_queue", 0);
    priority_ready_queue = L_create_queue ("priority_ready_queue", 0);
    regpres_ready_queue = L_create_queue ("regpres_ready_queue", 0);
    pending_ready_queue = L_create_queue ("pending_ready_queue", 0);
    not_ready_queue = L_create_queue ("not_ready_queue", 0);

}


/* Renumbers issue slots to be the lowest slot number while maintaining
 * scheduled order.  Totally ignores MDES specification.  Used to
 * format slots for vliw code  (slot number doesn't mater).
 *
 * Written by JCG 5/8/95 to facilitate using anaysis tools with my
 * vliw mdes (which put branches in slots 30-59).
 */
void Lsched_renumber_issue_slots (L_Func *fn)
{
    L_Cb *cb;
    L_Oper *op;
    L_Attr *isl;
    int desired_slot, cur_cycle, sched_cycle;

    /* Scan all cb's in function */
    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
	/* Reset slot/cycle for each cb */
	cur_cycle = -1;
	desired_slot = 0;

	/* Scan all ops in function */
	for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	    /* Get isl attribute (if available) */
	    isl = L_find_attr (op->attr, "isl");

	    /* If not found, goto next op */
	    if (isl == NULL)
		continue;

	    /* If does not have  slot/cycle specifies, goto next op */
	    if ((isl->max_field < 1) || (isl->field[0] == NULL) || 
		(isl->field[1] == NULL))
		continue;

	    /* Get scheduled slot/cycle */
	    sched_cycle = isl->field[0]->value.i;
	    
	    /* If we change cycles, reset desired_slot */
	    if (cur_cycle != sched_cycle)
	    {
		cur_cycle = sched_cycle;
		desired_slot = 0;
	    }

	    /* Set slot is isl to the desired leftmost slot */
	    isl->field[1]->value.i = desired_slot;

	    /* Increment desired_slot for next instruction */
	    desired_slot++;
	}
    }
}

void Lsched_func_complete(fn)
L_Func *fn;
{
    L_Cb 	*cb;
    L_Oper 	*oper;
    Sched_Info	*sinfo;
    Mdes_Info   *mdes_info;
    L_Attr	*new_attr;
    int		last_issue_time;

    for (cb = fn->first_cb; cb!=NULL; cb = cb->next_cb)
    {
        /* Used for delimiting cycles -JCG */
        last_issue_time = 0;

        /*
         * Free memory associated with scheduler memory pools
         */
	for (oper=cb->first_op; oper!=NULL; oper=oper->next_op)
	{
	    if (oper->ext!=NULL)
	    {
		sinfo = SCHED_INFO(oper);
		mdes_info = sinfo->mdes_info;

		if (Lsched_print_cycle_delimiter)
		{
		    /* Print newline before each cycle's ops -JCG 10/6/95*/
		    if (sinfo->issue_time != last_issue_time)
		    {
			oper->flags = L_SET_BIT_FLAG(oper->flags,
					         L_OPER_PRINT_CYCLE_DELIMITER);
			
			last_issue_time = sinfo->issue_time;
		    }
		    else
		    {
			oper->flags = L_CLR_BIT_FLAG(oper->flags,
						 L_OPER_PRINT_CYCLE_DELIMITER);
			
		    }
		}

		if (L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SPECULATIVE))
		{
		    switch (sinfo->spec_cond)
		    {
			case DEP_NON_EXCEPTING:
			    oper->flags = L_CLR_BIT_FLAG(oper->flags, L_OPER_SAFE_PEI);
			    if (Lsched_print_spec_condition)
			    {
		    		new_attr = L_new_attr("NON_EXCEPTING_SAFE", 0);
	            	        oper->attr = L_concat_attr(oper->attr,
				    new_attr);
			    }
			    break;

			case DEP_ALWAYS_SAFE:
		            oper->flags = L_SET_BIT_FLAG(oper->flags, 
				L_OPER_SAFE_PEI);

			    if (Lsched_print_spec_condition)
			    {
		    		new_attr = L_new_attr("ALWAYS_SAFE", 0);
	            	        oper->attr = L_concat_attr(oper->attr,
				    new_attr);
			    }
			    break;

			case DEP_CTL_DEP_SAFE:
		            oper->flags = L_SET_BIT_FLAG(oper->flags, 
				L_OPER_SAFE_PEI);

			    if (Lsched_print_spec_condition)
			    {
		    		new_attr = L_new_attr("CTL_DEP_SAFE", 0);
	            	        oper->attr = L_concat_attr(oper->attr,
				    new_attr);
			    }
			    break;

			case DEP_COMPLEX_SAFE:
		            oper->flags = L_SET_BIT_FLAG(oper->flags, 
				L_OPER_SAFE_PEI);

			    if (Lsched_print_spec_condition)
			    {
		    		new_attr = L_new_attr("COMPLEX_SAFE", 0);
	            	        oper->attr = L_concat_attr(oper->attr,
				    new_attr);
			    }
			    break;

			case DEP_SILENT:
			    if (Lsched_print_spec_condition)
			    {
		    		new_attr = L_new_attr("SILENT", 0);
	            	        oper->attr = L_concat_attr(oper->attr,
				    new_attr);
			    }
			    break;

			case DEP_DELAYS_EXCEPTION:
			    if (Lsched_print_spec_condition)
			    {
		    		new_attr = L_new_attr("DELAYS_EXCEPTION", 0);
	            	        oper->attr = L_concat_attr(oper->attr,
				    new_attr);
			    }
			    break;

			case DEP_SAFE_STORE:
			    if (Lsched_print_spec_condition)
			    {
		    		new_attr = L_new_attr("SAFE_STORE", 0);
	            	        oper->attr = L_concat_attr(oper->attr,
				    new_attr);
			    }
			    break;

			default:
			    L_punt("Invalid speculation condition\n");

		    }
		}
#if 1
		else
		    /* We don't need this information for non-speculated instructions */
		    oper->flags = L_CLR_BIT_FLAG(oper->flags, 
			L_OPER_SAFE_PEI);
#endif

		/* delete Sched_info struture */
		L_delete_sched_info(sinfo);
	    }
	}
    }

    /* If set in parm file, renumber issue slots to that the leftmost
     * slots are used. 
     */
    if (Lsched_do_renumber_issue_slots)
    {
	Lsched_renumber_issue_slots (fn);
    }

    /* REH 6-95, only mark if function was actually scheduled */
    if ( L_do_prepass_sched || Lsched_do_postpass_scheduling )
        /* SAM 5-95, mark function as scheduled for correct emulation */
        fn->flags = L_SET_BIT_FLAG(fn->flags, L_FUNC_SCHEDULED);
}

/******************************************************************************\
 *
 * Allocation routines for Sched_Info structure
 *
\******************************************************************************/
 
Sched_Info* L_create_sched_info (L_Oper *oper, int home_block, L_Oper *prev_br, 
		                 L_Oper *post_br)
{
    int 	i, max_ready;
    Sched_Info 	*sinfo;

    sinfo = (Sched_Info *) L_alloc(Sched_Info_pool);

    sinfo->sort_key1 = 0.0;
    sinfo->sort_key1 = 0.0;
    sinfo->rsort_key1 = 0.0;
    sinfo->rsort_key2 = 0.0;

    sinfo->oper = oper;
    sinfo->id = oper->id;
    sinfo->proc_opc = oper->proc_opc;
    sinfo->ru_info = L_create_ru_info_oper(oper);
    sinfo->dep_info = (Dep_Info *)oper->dep_info;
    sinfo->mdes_info = (Mdes_Info *)oper->mdes_info;
    sinfo->entry_list = NULL;

    if (sinfo->dep_info)
    {
        sinfo->spec_cond = sinfo->dep_info->spec_cond;
        sinfo->num_depend = sinfo->dep_info->n_input_dep;
    }

    sinfo->operand_ready_times = (int *) L_alloc(Operand_ready_pool);
    for (i=0, max_ready=mdes_latency_count(); i<max_ready; i++)
	sinfo->operand_ready_times[i] = -100; /* Changed from 0 -JCG 2/22/96 */
    
    sinfo->benefit = 0.0;
    sinfo->kill_set.p = 0.0;
    sinfo->kill_set.i = 0.0;
    sinfo->kill_set.f = 0.0;
    sinfo->kill_set.d = 0.0;
    sinfo->def_set.p = 0.0;
    sinfo->def_set.i = 0.0;
    sinfo->def_set.f = 0.0;
    sinfo->def_set.d = 0.0;
    sinfo->branch_kill_set_size = 0;
    sinfo->branch_kill_set = NULL;

    sinfo->priority = 0.0;
    sinfo->weight = oper->weight;
    sinfo->instr_prob = 0.0;
    sinfo->taken_instr_prob = 0.0;
    sinfo->not_taken_instr_prob = 0.0;
    sinfo->index = 0;
    sinfo->on_stack = 0;

    sinfo->etime = -1000;
    sinfo->ltimes = NULL;

    sinfo->scheduled = 0;
    sinfo->issue_time = 0;
    sinfo->issue_slot = -1;
    sinfo->relative_latency = (int *) Lcode_malloc (L_max_dest_operand * sizeof(int));
    for (i=0; i < L_max_dest_operand; i++) /* init to -1.  BLD 6/95 */
        sinfo->relative_latency[i] = -1;
    sinfo->ready_time = 0;
    sinfo->earliest_slot = 0;

    sinfo->delay_sinfo = NULL;

    sinfo->home_block = home_block;
    sinfo->current_block = home_block;
    sinfo->orig_block = home_block;
    sinfo->prev_br = prev_br;
    sinfo->post_br = post_br;

    sinfo->extend_lr_down = NULL;

    sinfo->check_op = NULL;
    sinfo->store_list = NULL;

    return sinfo;
}

void L_update_sched_info (Sched_Info *sinfo, L_Oper *oper, int home_block)
{
    int i, max_ready;

    sinfo->dep_info = (Dep_Info *)oper->dep_info;
    sinfo->mdes_info = (Mdes_Info *)oper->mdes_info;
    sinfo->entry_list = NULL;

    if (sinfo->dep_info->spec_cond != DEP_NON_EXCEPTING)
        sinfo->spec_cond = sinfo->dep_info->spec_cond;
    sinfo->num_depend = sinfo->dep_info->n_input_dep;

    for (i=0, max_ready=mdes_latency_count(); i<max_ready; i++)
        sinfo->operand_ready_times[i] = 0;

    sinfo->on_stack = 0;

    sinfo->scheduled = 0;
    sinfo->issue_time = 0;
    sinfo->issue_slot = -1;
    for (i=0; i < L_max_dest_operand; i++) /* init to -1.  BLD 6/95 */
        sinfo->relative_latency[i] = -1;
    sinfo->ready_time = 0;
    sinfo->earliest_slot = 0;

    sinfo->delay_sinfo = NULL;

    sinfo->current_block = home_block;
    sinfo->orig_block = home_block;

    sinfo->extend_lr_down = NULL;
    sinfo->check_op = NULL;
}

void L_delete_sched_info(Sched_Info *sinfo)
{
    L_Oper *oper = sinfo->oper;

    if (sinfo->branch_kill_set) free(sinfo->branch_kill_set);
    if (sinfo->ltimes) free(sinfo->ltimes);
    Lcode_free (sinfo->relative_latency);
    L_delete_ru_info_oper(sinfo->ru_info);
    L_free(Operand_ready_pool, sinfo->operand_ready_times);
    L_free(Sched_Info_pool, sinfo);

    oper->ext = NULL;
}

#if 1
void L_print_sched_info(F, s_info)
FILE *F;
Sched_Info *s_info;
{

	if (!s_info) return;

	fprintf(F, "OP %d\n", s_info->id);
        fprintf(F, "\thome_block\t%d\n", s_info->home_block);
        fprintf(F, "\tcurrent_block\t%d\n", s_info->current_block);
	if (s_info->prev_br)
	    fprintf(F, "\tprev_br\t%d\n", s_info->prev_br->id);
	else
	    fprintf(F, "\tprev_br\tNULL\n");
	if (s_info->post_br)
	    fprintf(F, "\tpost_br\t%d\n", s_info->post_br->id);
	else
	    fprintf(F, "\tpost_br\tNULL\n");
}
#endif


/******************************************************************************\
 *
 * Routines to compute early times and (re)compute late times.
 * Priority values are also calculated here too.
 * These routines were added to handle some of the shortcomings of static
 *   scheduling heuristics.
 (
\******************************************************************************/

void Lsched_init_etimes_and_ltimes (L_Cb *cb)
/* initializes the etimes and ltimes so everything will work correctly.
 * postpass scheduling does not create new sinfo structures if they are already
 * present for prepass scheduling.  SO BE WARNED!!!!! */
{
    L_Oper      *oper;
    Sched_Info  *sinfo;

    for (oper = cb->first_op; oper; oper = oper->next_op) {
        if (!oper->ext) /* oper does not get scheduled, so don't consider it */
            continue;
        sinfo = SCHED_INFO(oper);
        sinfo->etime = -1000;
        if (sinfo->ltimes)
            free (sinfo->ltimes);
        sinfo->ltimes = NULL;
    }
}

int Lsched_determine_etimes(L_Cb *cb, L_Oper ***exit_ops)
/* returns the number of exits present in this cb.  also fills in the *oper
 * fields of exit_ops and initializes the ltimes. */
{
    L_Oper      *oper, *dep_oper;
    int         num_exits = 0, exit_num = 0, temp_etime, i;
    L_Dep       *dep;
    Sched_Info  *sinfo, *dsinfo;

    /* compute early times for each operation.  also collect info on
     * number of exits found in this cb. */
    for (oper = cb->first_op; oper; oper = oper->next_op) {
        if (!oper->ext) /* oper does not get scheduled, so don't consider it. */
            continue;
        sinfo = SCHED_INFO(oper);
        if (IS_UCOND_BRANCH(oper->proc_opc) || IS_COND_BRANCH(oper->proc_opc))
            num_exits++;
        for (dep = DEP_INFO(oper)->input_dep; dep; dep = dep->next_dep) {
            if (dep->omega == 0) {
                dep_oper = dep->from_oper;
                dsinfo = SCHED_INFO(dep_oper);
                if (dsinfo->etime < 0) /* value should be init'ed */
                    L_punt ("ERROR: uninitialized etime found during etime \
calculation.\n");
                temp_etime = dsinfo->etime + dep->distance;
                /* We only increase the etime for control dependence path */

                if ((dep->type == L_DEP_CNT) &&
                    ((L_general_branch_opcode(dep_oper) ||
                      L_general_subroutine_call_opcode(dep_oper)) &&
                     (L_general_branch_opcode(oper) ||
                      L_general_subroutine_call_opcode(oper))))
                    if (dep->distance == 0)
                        temp_etime += Lsched_branch_increase;

                if (temp_etime > sinfo->etime)
                    sinfo->etime = temp_etime;
            }
        }
        if (sinfo->etime == -1000)
            sinfo->etime = 0;
    }

    /* if last instruction in cb is not an unconditional branch, another exit
     * is needed. (another exit is also needed if the last instruction is
     * predicated) */
    if (!IS_UCOND_BRANCH(cb->last_op->proc_opc) || 
        (L_is_predicated(cb->last_op) && !L_EXTRACT_BIT_VAL(cb->flags,
                                                L_CB_HYPERBLOCK_NO_FALLTHRU)))
        num_exits++;

    /* set values for exit_ops and malloc for ltimes. */
    *exit_ops = (L_Oper **) malloc (sizeof (L_Oper*) * num_exits);
    exit_num = 0;
    for (oper = cb->first_op; oper; oper = oper->next_op) {
        if (!oper->ext) /* oper does not get scheduled, so don't consider it. */
            continue;
        if (IS_UCOND_BRANCH(oper->proc_opc) || IS_COND_BRANCH(oper->proc_opc)) {
            (*exit_ops)[exit_num] = oper;
            exit_num++;
        }
        sinfo = SCHED_INFO(oper);

        /* take care of initializing the ltimes field for this oper. */
        sinfo->ltimes = (int *) malloc (sizeof (int) * num_exits);
        for (i = 0; i < num_exits; i++)
            sinfo->ltimes[i] = -1000;
    }

    /* if last instruction in cb is not an unconditional branch, another exit
     * is needed. (another exit is also needed if the last instruction is
     * predicated) */
    if (!IS_UCOND_BRANCH(cb->last_op->proc_opc) ||
        (L_is_predicated(cb->last_op) && !L_EXTRACT_BIT_VAL(cb->flags,
                                                L_CB_HYPERBLOCK_NO_FALLTHRU)))
      {
        if (cb->last_op->ext)
	  {
	    (*exit_ops)[exit_num] = cb->last_op;
	  }
	else
	  { /* find the latest instruction which will be scheduled. */
            for (oper = cb->last_op; !oper->ext; oper = oper->prev_op);
            (*exit_ops)[exit_num] = oper;
	  }
      }

    return (num_exits);
}

void Lsched_determine_ltimes (L_Cb *cb, int num_exits, L_Oper **exit_ops)
/* calculates the lates times for every operation relative to every exit. */
{
    L_Oper      *oper, *oper1, *dep_oper;
    int         temp_ltime, ltime_index, max_fallthru_ltime = 0;
    int         fallthru_present = 0;
    L_Dep       *dep;
    Sched_Info  *sinfo;

    /* compute the late times for each operation based on each exit. */
    for (ltime_index = 0; ltime_index < num_exits; ltime_index++) {
        if ((ltime_index == (num_exits - 1)) &&
            (!IS_UCOND_BRANCH(exit_ops[ltime_index]->proc_opc) ||
             (L_is_predicated(cb->last_op) && !L_EXTRACT_BIT_VAL(cb->flags,
                                               L_CB_HYPERBLOCK_NO_FALLTHRU)))) {
        /* if last exit is fallthru, wait until later to account for ltimes. */
            fallthru_present = 1;
            continue;
        }

        /* FOR SUCCESSIVE RETIREMENT, ltime = etime FOR AN EXIT IS NOT USED.
         * INSTEAD, WE SET LTIMES SO THAT EXITS ARE RETIRED IN ORDER ASAP. */
        if (!strcmp (Lsched_profile_info_to_use, "succ_ret"))
            temp_ltime = 100 * (ltime_index + 1);
        else
            temp_ltime = SCHED_INFO(exit_ops[ltime_index])->etime;
        SCHED_INFO(exit_ops[ltime_index])->ltimes[ltime_index] = temp_ltime;

        for (oper = exit_ops[ltime_index]->prev_op; oper;
             oper = oper->prev_op) {
            if (!oper->ext) /* oper does not get scheduled, so don't consider
                             * it. */
                continue;

            for (dep = DEP_INFO(oper)->output_dep; dep; dep = dep->next_dep) {
                if (dep->omega == 0) {
                    dep_oper = dep->to_oper;
                    if (SCHED_INFO(dep_oper)->ltimes[ltime_index] == -1000)
                    /* dependence goes to oper after exit op, don't use it. */
                        continue;
                    temp_ltime = SCHED_INFO(dep_oper)->ltimes[ltime_index] -
                                                        dep->distance;
                    if ((dep->type == L_DEP_CNT) &&
                        ((L_general_branch_opcode(dep_oper) ||
                         L_general_subroutine_call_opcode(dep_oper)) &&
                         (L_general_branch_opcode(oper) ||
                         L_general_subroutine_call_opcode(oper))))
                        if (dep->distance == 0)
                            temp_ltime -= Lsched_branch_increase;

                    if ((SCHED_INFO(oper)->ltimes[ltime_index] == -1000) ||
                        (temp_ltime < SCHED_INFO(oper)->ltimes[ltime_index]))
                        SCHED_INFO(oper)->ltimes[ltime_index] = temp_ltime;
                }
            }
        }
    }

    if (fallthru_present) {
        /* special-case for opers that fall through the cb, and don't
         * have any outgoing arcs.  ltime in this case is set to ltime of fall-
         * through exit operation.  all its other ltimes are undefined. */
        for (oper = cb->last_op; oper; oper = oper->prev_op) {
            sinfo = SCHED_INFO(oper);
            if (!oper->ext) /* oper doesn't get scheduled, don't consider it. */
                continue;

            if (DEP_INFO(oper)->n_output_dep == 0) {
                if (max_fallthru_ltime < sinfo->etime)
                    max_fallthru_ltime = sinfo->etime;
            }
        }

        /* IF SUCCESSIVE RETIREMENT USED, NEED TO ALTER max_fallthru_ltime */
        if (!strcmp (Lsched_profile_info_to_use, "succ_ret"))
            max_fallthru_ltime = 100 * num_exits;

        if (DEP_INFO(exit_ops[num_exits - 1])->n_output_dep != 0)
            L_punt ("ERROR: last fall-thru oper in cb has an outgoing dep");
        for (oper = cb->last_op; oper; oper = oper->prev_op) {
            if (!oper->ext) /* oper doesn't get scheduled, don't consider it. */
                continue;

            if (DEP_INFO(oper)->n_output_dep == 0) {
                SCHED_INFO(oper)->ltimes[num_exits - 1] = max_fallthru_ltime;

                for (oper1 = oper->prev_op; oper1; oper1 = oper1->prev_op) {
                    if (!oper1->ext) /* oper doesn't get scheduled, so don't
                                      * consider it. */
                        continue;
                    for (dep = DEP_INFO(oper1)->output_dep; dep;
                         dep = dep->next_dep) {
                        if (dep->omega == 0) {
                            dep_oper = dep->to_oper;
                            if (SCHED_INFO(dep_oper)->ltimes[num_exits - 1] == -1000)
                            /* dependence goes to oper after exit op, so don't
                             * use it */
                                continue;
                            temp_ltime = SCHED_INFO(dep_oper)->ltimes[num_exits - 1]
                                                                - dep->distance;
                            if ((dep->type == L_DEP_CNT) &&
                                ((L_general_branch_opcode(dep_oper) ||
                                  L_general_subroutine_call_opcode(dep_oper)) &&
                                 (L_general_branch_opcode(oper1) ||
                                  L_general_subroutine_call_opcode(oper1))))
                                if (dep->distance == 0)
                                    temp_ltime -= Lsched_branch_increase;
    
                            if ((SCHED_INFO(oper1)->ltimes[num_exits - 1] == -1000)
                                  || (temp_ltime <
                                      SCHED_INFO(oper1)->ltimes[num_exits - 1]))
                                SCHED_INFO(oper1)->ltimes[num_exits - 1] =
                                                                     temp_ltime;
                        }
                    }
                }
            }
        }
    }
}

void Lsched_calculate_priority_normal(L_Cb *cb, int num_exits, 
                                      L_Oper **exit_ops)
/* calculates priority for each operation.  this is done the normal
 * way (the method used previously by the scheduler). */
{
    L_Oper      *oper;
    int         i, max_ltime = 0;
    float       priority, *probs;
    L_Flow      *flow;
    Sched_Info  *sinfo;

    /* compute the probs present for each exit. */
    probs = (float *) malloc (sizeof (float) * num_exits);
    if (!strcmp (Lsched_profile_info_to_use, "real")) {
        if (cb->weight == 0) {
            for (i = 0; i < (num_exits - 1); i++)
                probs[i] = 0;
            probs[num_exits - 1] = 1.0;
        }
        else {
            probs[num_exits - 1] = 1.0;
            /* NOTE: I am assuming that jump_rg's are not predicated.  This is a
             *       valid assumption for the current IMPACT */
            for (i = 0; i < (num_exits - 1); i++) {
                flow = L_find_flow_for_branch (cb, exit_ops[i]);
                probs[i] = flow->weight / cb->weight;
                probs[num_exits - 1] -= probs[i];
            }
        }
    }
    else if (!strcmp (Lsched_profile_info_to_use, "all_1")) {
        for (i = 0; i < num_exits; i++)
            probs[i] = (float) 1 / (float) num_exits;
    }
    else if (!strcmp (Lsched_profile_info_to_use, "last_1")) {
        for (i = 0; i < (num_exits - 1); i++)
            probs[i] = 0;
        probs[num_exits - 1] = 1.0;
    }
    else if (!strcmp (Lsched_profile_info_to_use, "first_1")) {
        probs[0] = 1.0;
        for (i = 1; i < num_exits; i++)
            probs[i] = 0;
    }
    else if (!strcmp (Lsched_profile_info_to_use, "succ_ret")) {
    /* late times have been modified earlier for successive retirement, so
     * setting all the probabilities to one is what is desired here. */
        for (i = 0; i < num_exits; i++)
            probs[i] = 1;
    }
    else
        L_punt ("ERROR: illegal value for parameter, \"profile_info_to_use\" \
encountered.");

    for (oper = cb->last_op; oper; oper = oper->prev_op) {
    /* find the max late time present in this cb. */
        sinfo = SCHED_INFO(oper);
        if (!oper->ext) /* oper does not get scheduled, don't consider it. */
            continue;
        if (max_ltime < sinfo->etime)
            max_ltime = sinfo->etime;
    }

    /* max_ltime FOR SUCCESSIVE RETIREMENT MUST BE EXPLICITELY SET TO ITS
     * CORRECT VALUE BECAUSE ltimes ARE INDEPENDENT OF etimes */
    if (!strcmp (Lsched_profile_info_to_use, "succ_ret"))
       max_ltime = 100 * num_exits;

    /* compute the priority for each operation. */
    for (oper = cb->first_op; oper; oper = oper->next_op) {
        if (!oper->ext) /* oper does not get scheduled, so don't consider it. */
            continue;
        sinfo = SCHED_INFO(oper);

        priority = 0;
        for (i = 0; i < num_exits; i++)
            if (sinfo->ltimes[i] != -1000) {
                if (probs[i] == 0) /* added because zero weight exits that end
                                    * a block cannot use dependence height to
                                    * determine importance otherwise. */
                    probs[i] = 0.00001;
                priority += ((float) max_ltime + 1.0 - 
                                          (float) sinfo->ltimes[i]) * probs[i];
            }
        sinfo->priority = priority;
    }

    free (probs);
}


/******************************************************************************\
 *
 * Routines to compute static scheduling heuristics
 *
\******************************************************************************/

/* Leaf node types */
#define FALL_THRU	0
#define UBR_TAKEN	1
#define CBR_TAKEN	2

typedef struct 
{
    int		num_opers;
    L_Oper	**oper;
    int		type;
    int		initial_height;
    float	prob;
    int		*heights;
} LEAF;

#if 0
static void Lsched_compute_height(cb, leaf, num_opers, stack)
    L_Cb	*cb;
    LEAF	*leaf;
    int		num_opers;
    L_Oper	**stack;
{
    L_Dep	*dep;
    L_Oper	*oper, *dep_oper;
    Sched_Info	*dep_sinfo, *sinfo;
    int		i, height, temp_height, top_of_stack=0, dep_index;

    /* Initialize distance to end of computation chain */
    for (i=0; i<num_opers; i++)
	leaf->heights[i] = -1;

    /* Put all of the opers on the stack */
    for (i = 0; i < leaf->num_opers; i++)
    {
        stack[top_of_stack++] = oper = leaf->oper[i];
        sinfo = SCHED_INFO(oper);
        leaf->heights[sinfo->index] = leaf->initial_height;
        sinfo->on_stack=1;
    }

    while (top_of_stack > 0)
    {
	oper = stack[--top_of_stack];
	sinfo = SCHED_INFO(oper);
  	sinfo->on_stack = 0;
	height = leaf->heights[sinfo->index];

	for (dep=DEP_INFO(oper)->input_dep; dep!=0; dep=dep->next_dep)
	{
	    dep_oper = dep->from_oper;

	    dep_sinfo = SCHED_INFO(dep_oper);
	    dep_index = dep_sinfo->index;

	    temp_height = height + dep->distance;

	    /* We only increase the height for the control dependence path */
	    if ((dep->type == L_DEP_CNT) && (dep->distance == 0))
		temp_height += Lsched_branch_increase;

	    if (temp_height > leaf->heights[dep_index])
	    {
	        leaf->heights[dep_index] = temp_height;
		if (dep_sinfo->on_stack==0)
		{
		    stack[top_of_stack++] = dep_oper;
		    dep_sinfo->on_stack = 1;
		}
	    }
	}
    }
}

static void Lsched_compute_static_priorities (cb)
    L_Cb	*cb;
{
    L_Oper	*oper, *stack;
    double	cb_weight;		
    float	fall_thru_prob;
    L_Flow	*flow;
    Sched_Info	*sinfo;
    int 	i, j, num_leaves=0, num_opers=0, max_level=0,
		fall_thru_leaf_needed=0, num_fall_thru_opers=0;
    LEAF	*leaves;

    L_compute_dependence_level(cb);

    /* 
     * Determine number of instructions, assign them a unique zero-
     * relative index, and determine the number of leaf nodes.
     */
    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
        if (oper->ext!=NULL)
	{
	    if (IS_UCOND_BRANCH(oper->proc_opc))
		num_leaves++;
	    else if (IS_COND_BRANCH(oper->proc_opc))
		num_leaves++;

	    SCHED_INFO(oper)->index = num_opers++;

	    if (DEP_INFO(oper)->n_output_dep == 0)
		num_fall_thru_opers++;

	    if (DEP_INFO(oper)->level > max_level)
		max_level = DEP_INFO(oper)->level;

	}
    }

    /* 
     * If the last instruction in the CB is not an unconditional branch
     * then we need to create a fall-thru node.
     */
    if (!IS_UCOND_BRANCH(cb->last_op->proc_opc))
    {
	num_leaves++;
	fall_thru_leaf_needed = 1;
    }

    /* Create and initialize information for leaf nodes structures */
    leaves = (LEAF*) malloc (sizeof(LEAF) * num_leaves);
    stack = (L_Oper*) malloc(sizeof(L_Oper*) * num_opers);

    /* Initialize leaves and probabilities */
    if (cb->weight!=0)
        cb_weight = cb->weight;
    else
	cb_weight = 1.0;
    fall_thru_prob = 1.0;

    for (i=0, oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
        if (oper->ext!=NULL)
	{
	    if (IS_UCOND_BRANCH(oper->proc_opc))
	    {
		leaves[i].num_opers = 1;
    		leaves[i].oper = (L_Oper**) malloc(sizeof(L_Oper**));
    		leaves[i].oper[0] = oper;
		leaves[i].type = UBR_TAKEN;
		if (Lsched_pad_height)
		    leaves[i].initial_height = max_level - DEP_INFO(oper)->level + 1;
		else
		    leaves[i].initial_height = 1;

		flow = L_find_flow_for_branch(cb, oper);
	        leaves[i].prob = flow->weight / cb_weight;
    		fall_thru_prob -= leaves[i].prob;

	        leaves[i].heights = (int *) malloc(sizeof(int *) * (num_opers));
	        i++;
	    }
	    else if (IS_COND_BRANCH(oper->proc_opc))
	    {
		/* Taken path */
		leaves[i].num_opers = 1;
    		leaves[i].oper = (L_Oper**) malloc(sizeof(L_Oper**));
    		leaves[i].oper[0] = oper;
		leaves[i].type = CBR_TAKEN;
		if (Lsched_pad_height)
		    leaves[i].initial_height = max_level - DEP_INFO(oper)->level + 1;
		else
		    leaves[i].initial_height = 1;

		flow = L_find_flow_for_branch(cb, oper);
	        leaves[i].prob = flow->weight / cb_weight;
    		fall_thru_prob -= leaves[i].prob;

	        leaves[i].heights = (int *) malloc(sizeof(int *) * (num_opers));
	        i++;
	    }
	}
    }

    /*
     * We will link all instructions that have no outgoing arcs
     * to the fall-thru leaf.
     */
    if (fall_thru_leaf_needed)
    {
	leaves[i].num_opers = num_fall_thru_opers;
    	leaves[i].oper = (L_Oper**) malloc(sizeof(L_Oper**) * num_fall_thru_opers);
	leaves[i].type = FALL_THRU;
	leaves[i].initial_height = 1; 
	leaves[i].prob = fall_thru_prob;
	leaves[i].heights = (int *) malloc(sizeof(int *) * (num_opers));
	j = 0;

	for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
	{
            if (oper->ext!=NULL)
	    {
	        if (DEP_INFO(oper)->n_output_dep == 0)
	        {
		    leaves[i].oper[j] = oper;
		    j++;
	        }
	    }
	}
    }

    /* Determine heights from all leaves to all parent nodes */
    for (i=0; i < num_leaves; i++)
	Lsched_compute_height(cb, &leaves[i], num_opers, stack);

    if (Lsched_debug_operation_priority)
    {
	fprintf (stdout, "> CB %d - leaf node probabilities and heights\n", cb->id);

        for (i=0; i < num_leaves; i++)
	{
	    switch (leaves[i].type)
	    {
		case FALL_THRU:
                    fprintf (stdout, 
			">   FALL_THRU num_opers=%d: instr prob=%f, initial_height=%d\n", 
		        leaves[i].num_opers, leaves[i].prob, leaves[i].initial_height);
		    break;
		case UBR_TAKEN:
                    fprintf (stdout, 
			">   UBR_TAKEN LEAF op %d: taken prob=%f, initial_height=%d\n", 
		        leaves[i].oper[0]->id, leaves[i].prob, leaves[i].initial_height);
		    break;
		case CBR_TAKEN:
                    fprintf (stdout, 
			">   CBR_TAKEN LEAF op %d: taken prob=%f, initial_height=%d\n", 
		        leaves[i].oper[0]->id, leaves[i].prob, leaves[i].initial_height);
		    break;
	    }
	}

	fprintf (stdout, "> operation priorities, (heights, ...)\n");
    }

    /* Compute static priority proportional to the average height */
    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op)
    {
	int index;

	if (oper->ext!=NULL)
	{
	    sinfo = SCHED_INFO(oper);
	    index = sinfo->index;
            sinfo->priority = 0; /* needs to be initialized (for postpass
                                  * scheduling when sinfo is saved from
                                  * prepass scheduling.) */

#if 0
	    if (IS_BRANCH(oper->proc_opc))
		sinfo->priority = .01;
	    else
	        sinfo->priority = 0.0;
#endif

	    for (i = 0; i < num_leaves; i++)
		if (leaves[i].heights[index] != -1)
		    sinfo->priority += leaves[i].prob * leaves[i].heights[index];

	    if (Lsched_debug_operation_priority)
	    {
	        fprintf (stdout, ">   op %d: %f (",  oper->id, 
		    sinfo->priority); 

		for (i=0; i < num_leaves; i++)
		{
		    if (leaves[i].heights[index] != -1)
		        fprintf (stdout, "%d", leaves[i].heights[index]);
		    else
		        fprintf (stdout, "-");

		    if (i+1 < num_leaves) fprintf (stdout, ",");
		}

		fprintf (stdout, ")\n");
	    }
	}
    }

    /* release memory */
    for (i=0; i < num_leaves; i++)
	free(leaves[i].heights);

    free (leaves);
    free (stack);
}
#endif

/******************************************************************************\
 *
 *  Misc. routines
 *
\******************************************************************************/

int	Lsched_is_branch(int proc_opc)
{
    if ((L_spec_model == SENTINEL) || (L_spec_model == SRB)) 
	return IS_CTL(proc_opc);
    else
	return IS_BRANCH(proc_opc);
}

L_Oper *L_find_next_branch(L_Cb *cb, L_Oper *oper)
{
    L_Oper *ptr;

    if (oper==NULL)
	ptr = cb->first_op;
    else
	ptr = oper->next_op;

    for (; ptr!=NULL; ptr=ptr->next_op) 
	if (Lsched_is_branch(ptr->proc_opc)) return ptr;

    return NULL;
}

void L_annotate_instr(L_Cb *cb)
{
    L_Oper 	*oper;
    Sched_Info	*sinfo;
    L_Attr	*new_attr;

    if (!Lsched_print_hb_and_spec) return;

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op)
    {
	if (oper->ext!=0)
	{
	    sinfo = SCHED_INFO(oper);

	    new_attr = L_new_attr("spec_dist", 1);
	    L_set_int_attr_field(new_attr, 0, sinfo->home_block -
		sinfo->current_block);
	    oper->attr = L_concat_attr(oper->attr, new_attr);

	    new_attr = L_new_attr("hb", 1);
	    L_set_int_attr_field(new_attr, 0, sinfo->home_block);
	    oper->attr = L_concat_attr(oper->attr, new_attr);
	}
    }
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void Lsched_make_ready(L_Cb *cb, int oper_issue_time, Sched_Info *sinfo, 
		       Sched_Info *dep_sinfo, int to_index, int from_index,
		       int distance)
{
    int		complete_time, is_delay_op, dep_ready_time; 

    /* Remove the dependence from the instruction */ 
    dep_sinfo->num_depend--;

    /* 
     * Determine the completion time for the dependent operand
     *
     * In the event that we are deleting a check instruction,
     * we must be able to put any dependent operations
     * on the pending_ready list.  Since the check was
     * never scheduled, it will have no alternative.
     * Zero is used for the default base case.
     */
    if (!Lsched_infinite_issue && (RU_SELECTED_ALT(sinfo->ru_info)==NULL) && 
	(sinfo->proc_opc!=Lop_CHECK))
	L_punt("Lsched_make_ready: instruction without alt was not a check!");

    if (sinfo->proc_opc==Lop_CHECK)
        complete_time = oper_issue_time;
    else if (Lsched_infinite_issue)
        complete_time = oper_issue_time + mdes_operand_latency(sinfo->proc_opc, 
            mdes_heuristic_alt_id(sinfo->proc_opc), from_index);
    else
        complete_time = oper_issue_time + mdes_operand_latency(sinfo->proc_opc, 
            RU_SELECTED_ALT_ID(sinfo->ru_info), from_index);
    
#if 0
    if ((Lsched_processor_model==MDES_VLIW) && 
	(L_register_branch_opcode(dep_sinfo->oper) || 
	 L_subroutine_call_opcode(dep_sinfo->oper)))
        complete_time = oper_issue_time + distance;
    else if (sinfo->proc_opc==Lop_CHECK)
        complete_time = oper_issue_time;
    else if (Lsched_infinite_issue)
        complete_time = oper_issue_time + mdes_operand_latency(sinfo->proc_opc, 
            mdes_heuristic_alt_id(sinfo->proc_opc), from_index);
    else
        complete_time = oper_issue_time + mdes_operand_latency(sinfo->proc_opc, 
            RU_SELECTED_ALT_ID(sinfo->ru_info), from_index);
#endif

    /* Update the ready time for the dependent operand */
    if (complete_time > dep_sinfo->operand_ready_times[to_index])
        dep_sinfo->operand_ready_times[to_index] = complete_time;

	/* 
	 * Determine the earliest ready time based upon the operand
	 * ready times.
	 *
	 * This code keeps a running tally of the latest zero-cycle
	 * dependency.  Given a tie for the latest, it chooses the
	 * zero-cycle dependency forcing the latest slot in
	 * a cycle.
	 */
        if (oper_issue_time > dep_sinfo->ready_time)
        {
            dep_sinfo->ready_time = oper_issue_time;
            dep_sinfo->earliest_slot = sinfo->issue_slot;
        }
        else if (oper_issue_time == dep_sinfo->ready_time)
        {
            if (dep_sinfo->earliest_slot < (sinfo->issue_slot))
                dep_sinfo->earliest_slot = sinfo->issue_slot;
        }

#if 0
    if (Lsched_processor_model!=MDES_VLIW)
    {
	/* 
	 * Determine the earliest ready time based upon the operand
	 * ready times.
	 *
	 * This code keeps a running tally of the latest zero-cycle
	 * dependency.  Given a tie for the latest, it chooses the
	 * zero-cycle dependency forcing the latest slot in
	 * a cycle.
	 */
        if (oper_issue_time > dep_sinfo->ready_time)
        {
            dep_sinfo->ready_time = oper_issue_time;
            dep_sinfo->earliest_slot = sinfo->issue_slot;
        }
        else if (oper_issue_time == dep_sinfo->ready_time)
        {
            if (dep_sinfo->earliest_slot < (sinfo->issue_slot))
                dep_sinfo->earliest_slot = sinfo->issue_slot;
        }
    }
#endif

    /* Take care of the "ready" instructions */
    if ((dep_sinfo->num_depend==0) && 
	(!L_EXTRACT_BIT_VAL(dep_sinfo->oper->flags, FILLER)))
    {
	/* Initialize the ready time for the current instruction */
	dep_ready_time = mdes_calc_min_ready_time(dep_sinfo->mdes_info, 
	    dep_sinfo->operand_ready_times);

	/* 
	 * If the actual ready time for this instruction is not 
	 * equal to the running tally of the latest zero-cycle
	 * dependancy, then we must reset the earliest_slot
	 * to prevent poor schedules at later issue_times.
	 */
        if (dep_ready_time > dep_sinfo->ready_time)
        {
            dep_sinfo->ready_time = dep_ready_time;
            dep_sinfo->earliest_slot = 0;
        }

        if (Lsched_fill_delay_slots) 
	    is_delay_op = Lsched_update_ctl_delay_sinfo(not_ready_queue, dep_sinfo);
	else
	    is_delay_op = 0;

	if (!is_delay_op)
	{
	    L_dequeue(not_ready_queue, dep_sinfo);

	    L_enqueue_min_to_max_1 (pending_ready_queue, dep_sinfo, 
		(float) dep_sinfo->ready_time);
	}
    }
}

void Lsched_schedule_op(L_Cb *cb, Sched_Info *sinfo, int spec, int ready_time) 
{
    int         i, to_index, latency;
    L_Oper	*oper=sinfo->oper;
    Dep_Info	*dep_info=sinfo->dep_info;
    L_Dep	*dep;

    /* Update register pressure estimates */
    L_update_virt_reg(sinfo);

    /* Mark the instruction as speculative */
    if (spec)
    {
	oper->flags=L_SET_BIT_FLAG(oper->flags, L_OPER_SPECULATIVE);

	if (!(L_EXTRACT_BIT_VAL(oper->flags, L_OPER_SAFE_PEI)) && 
            (any_alt_flag_set(sinfo->mdes_info, ALT_FLAG_SILENT)))  {
	    oper->flags = L_SET_BIT_FLAG(oper->flags, L_OPER_MASK_PE);
	    /* REH 4/19/95 - Make sure that function mask flag is set */
	    L_fn->flags   = L_SET_BIT_FLAG(L_fn->flags, L_FUNC_MASK_PE);
        }
    }
    else
	oper->flags=L_CLR_BIT_FLAG(oper->flags, L_OPER_SPECULATIVE);


    /* Mark the issue time and completion times */
    sinfo->issue_time = ready_time;
#if 0
/* no longer needed. BLD 6/95 */
    if (Lsched_infinite_issue)
        sinfo->completion_time = ready_time + mdes_max_completion_time(sinfo->proc_opc, 
            mdes_heuristic_alt_id(sinfo->proc_opc));
    else
        sinfo->completion_time = ready_time + mdes_max_completion_time(sinfo->proc_opc, 
	    RU_SELECTED_ALT_ID(sinfo->ru_info)); 
#endif
    /* Also mark the relative latencies associated with each dest operand */
    for (i = 0; i < L_max_dest_operand; i++)
    {
	if ( oper->dest[i] != NULL )
        {
	    to_index = operand_index(MDES_DEST, i);
	    latency = max_operand_time(sinfo->mdes_info, to_index);
	    sinfo->relative_latency[i] = latency;
        }
    }

    switch (L_spec_model)
    {
#if 0
        case SENTINEL:
        case SRB:
	    L_sentinel_update_block(cb, oper, ready_time);
	    break;
#endif
	
	case WRITEBACK_SUPPRESSION:
	    wbs_remove_check(cb, sinfo, ready_time);
	    break;

        case MCB:
	    L_mcb_remove_check(cb,oper,ready_time);
	    break;
    }

    /* Place the sorted instructions into scheduled queue */
    sinfo->scheduled = 1;
    L_dequeue_from_all(sinfo);
    L_enqueue_min_to_max_2 (scheduled_queue, sinfo, (float) sinfo->issue_time,
	(float) sinfo->issue_slot);
 
    if (Lsched_debug_messages)  
    {
        if (spec)
	    fprintf (stdout, "> <S> ");
	else
	    fprintf (stdout, "> ");

	fprintf (stdout, "scheduled op %d: ready=%d, issue=%d, slot=%d, sched block=%d",
	    oper->id, sinfo->ready_time, sinfo->issue_time, sinfo->issue_slot, 
	    sinfo->current_block);

        if (spec)
	    fprintf (stdout, ", home block=%d\n", sinfo->home_block);
	else
	    fprintf (stdout, "\n");
    }
 
    /* 
     * Remove dependences from instruction that are dependent on  instruction
     * that was just scheduled.  Place any of them that have no remaining 
     * dependences into the pending ready list.
     */
    for (dep=dep_info->output_dep; dep!=0; dep=dep->next_dep)
    {
        Lsched_make_ready(cb, ready_time, sinfo, SCHED_INFO(dep->to_oper), 
	    dep->to_index, dep->from_index, dep->distance);
    }
}

int Lsched_can_schedule_op (L_Cb *cb, Sched_Info *sinfo, int current_time, 
    int current_block, int *issue_slot, int *spec, int *silent)
{

    int		earliest_slot, latest_slot, can_sched;
    Sched_Info	*prev_br_sinfo=NULL;

    *spec = 0;
    *silent = 0;

#if 0
    /* Determine if operation is speculative, and attempt to schedule it */
    if (Lsched_processor_model==MDES_VLIW)
    {
        /*
         * We are assuming that any instruction scheduled in the
         * same cycle of a branch or earlier than the branch whose
         * home block is physically after the branch, is speculative
         *
         * We also ignore zero-cycle latency problems with vliw
         * models.
         */
        if (sinfo->home_block>current_block)
	{
            *spec = 1;

	    if ( (sinfo->dep_info->spec_cond == DEP_SILENT) || 
		 (sinfo->dep_info->spec_cond == DEP_DELAYS_EXCEPTION) )
		*silent = ALT_FLAG_SILENT;
	}

        earliest_slot = 0;
	latest_slot = Lsched_total_issue_slots;

        if ((*issue_slot = RU_can_schedule_op(sinfo->ru_info, sinfo->mdes_info,
	    sinfo->operand_ready_times,
	    current_time, earliest_slot, latest_slot, *silent))==-1)
	    return CANT_SCHEDULE;
    }
    else
#endif
    {
        /*
	 * There are three cases for scheduling an instruction for superscalar
	 * processors:
	 *
	 * 1) Instruction is scheduled in an earlier cycle than its previous 
	 *    branch - this instruction is clearly speculative.
	 *
	 * 2) Instruction is scheduled in a later cycle than its previous 
	 *    branch - this instruction is clearly not speculative.
	 *
	 * 3) Instruction is scheduled in the same cycle as its previous
	 *    branch - this instruction will be speculative if it is scheduled
	 *    in a slot prior to its previous branch and non-speculative
	 *    otherwise.
	 *
	 * We must handle all of these conditions.
	 *
	 * Earliest slot handles zero cycle dependences, latest slot handles
	 * speculative condition.
	 */

        if (sinfo->ready_time < current_time)
	    sinfo->earliest_slot=0;
	earliest_slot = sinfo->earliest_slot;

	if (sinfo->prev_br)
	    prev_br_sinfo = SCHED_INFO(sinfo->prev_br);
	else
	    prev_br_sinfo = NULL;

	if (prev_br_sinfo && 
	    ((!prev_br_sinfo->scheduled) || (prev_br_sinfo->issue_time > current_time)))
	{
	    /* Case 1 - must be speculative */

            *spec = 1;
	    if ( (sinfo->dep_info->spec_cond == DEP_SILENT) || 
		 (sinfo->dep_info->spec_cond == DEP_DELAYS_EXCEPTION) )
		*silent = ALT_FLAG_SILENT;

	    latest_slot = Lsched_total_issue_slots;

            if (Lsched_infinite_issue) 
	    {
		*issue_slot = Lsched_infinite_issue_slot++;
	        return CAN_SCHEDULE;
	    }

            /* Is it feasible to schedule the instruction in this cycle ? */
            if ((*issue_slot = RU_can_schedule_op(sinfo->ru_info, sinfo->mdes_info,
	         sinfo->operand_ready_times,
	         current_time, earliest_slot, latest_slot, *silent))==-1)
	            return CANT_SCHEDULE;
	}
	else if (!prev_br_sinfo || 
		 (prev_br_sinfo->scheduled && prev_br_sinfo->issue_time < current_time))
	{
	    /* Case 2 - definitely not speculative */
	    latest_slot = Lsched_total_issue_slots;

            if (Lsched_infinite_issue) 
	    {
		*issue_slot = Lsched_infinite_issue_slot++;
	        return CAN_SCHEDULE;
	    }

            /* Is it feasible to schedule the instruction in this cycle ? */
            if ((*issue_slot = RU_can_schedule_op(sinfo->ru_info, sinfo->mdes_info,
	            sinfo->operand_ready_times,
	            current_time, earliest_slot, latest_slot, *silent))==-1)
	        return CANT_SCHEDULE;
	}
	else
	{
	    if (prev_br_sinfo->issue_time != current_time)
		L_punt ("Lsched_can_schedule_op: error in logic dummy!");

	    /* 
	     * Case 3 - Maybe speculative depending on where it is scheduled.
	     * This case means that there is a prev_branch and its scheduled
	     * time is the current_time.
	     */

	    /* Try to schedule the instruction speculatively first. */
	    can_sched = CANT_SCHEDULE;
	    latest_slot = prev_br_sinfo->issue_slot;


	    if (earliest_slot <= latest_slot) 
	    {
	        if ( (sinfo->dep_info->spec_cond == DEP_SILENT) || 
		     (sinfo->dep_info->spec_cond == DEP_DELAYS_EXCEPTION) )
		    *silent = ALT_FLAG_SILENT;

                if (Lsched_infinite_issue) 
	        {
		    *issue_slot = Lsched_infinite_issue_slot++;
	            return CAN_SCHEDULE;
	        }

                *spec = 1;

                /* Is it feasible to schedule the instruction in this cycle ? */
                if ((*issue_slot = RU_can_schedule_op(sinfo->ru_info, sinfo->mdes_info,
	            sinfo->operand_ready_times,
	            current_time, earliest_slot, latest_slot, *silent))!=-1)
	             can_sched = CAN_SCHEDULE;
	    }

	    if (can_sched != CAN_SCHEDULE)
	    {
	        /* 
	         * Since it was not possible to schedule this instruction speculatively,
	         * try to schedule it non-speculatively.
	         */
	        if (latest_slot > earliest_slot) earliest_slot = latest_slot;

	        latest_slot = Lsched_total_issue_slots;

                *spec = 0;
		*silent = 0;

                if (Lsched_infinite_issue) 
	        {
		    *issue_slot = Lsched_infinite_issue_slot++;
	            return CAN_SCHEDULE;
	        }

                /* Is it feasible to schedule the instruction in this cycle ? */
                if ((*issue_slot = RU_can_schedule_op(sinfo->ru_info, sinfo->mdes_info, 
	            sinfo->operand_ready_times,
		    current_time, earliest_slot, latest_slot, *silent))==-1)
	            return CANT_SCHEDULE;
	    }
	}
    }

    return CAN_SCHEDULE;
}

/******************************************************************************\
 *
 * Corrects vliw schedule for cross basic block dependences.  This ensures
 * accurate execution in the absense of interlock.
 *
\******************************************************************************/

static L_Oper *Lsched_create_nop(int issue_time, int issue_slot)
{
    L_Oper	*new_oper;
    Sched_Info	*sinfo;
    L_Attr	*new_attr;

    new_oper = L_create_new_op(Lop_NO_OP);
    sinfo = L_create_sched_info(new_oper, 0, NULL, NULL);
    new_oper->ext = (void *) sinfo;

    new_attr = L_new_attr("vliw_pad", 0);
    new_oper->attr = L_concat_attr(new_oper->attr, new_attr);
    sinfo->issue_time = issue_time;
    sinfo->issue_slot = issue_slot;
#if 0
/* no dest operands, so relative latencies are unchanged. BLD 6/95 */
    sinfo->completion_time = issue_time + 
	mdes_max_completion_time(Lop_NO_OP, mdes_heuristic_alt_id(Lop_NO_OP));
#endif

    return new_oper;
}

static L_Oper *Lsched_create_jump(int issue_time, int issue_slot, 
    L_Operand *target)
{
    L_Oper	*new_oper;
    Sched_Info	*sinfo;

    new_oper = L_create_new_op(Lop_JUMP);
    new_oper->src[0] = L_copy_operand(target);
    sinfo = L_create_sched_info(new_oper, 0, NULL, NULL);
    new_oper->ext = (void *) sinfo;
    sinfo->issue_time = issue_time;
    sinfo->issue_slot = issue_slot;
#if 0
/* no dest operands, so relative latencies are unchanged. BLD 6/95 */
    sinfo->completion_time = issue_time + 
	mdes_max_completion_time(Lop_JUMP, mdes_heuristic_alt_id(Lop_JUMP));
#endif

    return new_oper;
}

static void Lsched_pad_slots_with_nops(L_Cb *cb)
{
    L_Oper 	*oper;
    Sched_Info	*sinfo;
    int		i, j, issue_time, last_time, issue_slot, last_slot = 0,
		max_slot, found=0;

    last_time = 0;
    max_slot = Lsched_total_issue_slots-1;

    for (oper = cb->first_op; oper!= NULL; oper=oper->next_op)
    {
	if (IS_IGNORE(oper->proc_opc)) continue;

	found = 1;

	sinfo = SCHED_INFO(oper);
	issue_time = sinfo->issue_time;
	issue_slot = sinfo->issue_slot;

	if (issue_time != last_time)
	{
	    /* Fill in the unused slots from the last cycle */
    	    if (Lsched_pad_vliw_slots_with_nops)
	        for (i=last_slot+1; i<=max_slot; i++)
	            L_insert_oper_before(cb, oper, 
			Lsched_create_nop(last_time, i));

	    /* Fill in the unused cycles */
	    for (j=last_time+1; j<issue_time; j++)
	    {
		L_insert_oper_before(cb, oper, Lsched_create_nop(j, 0));

    	        if (Lsched_pad_vliw_slots_with_nops)
	    	    for (i=1; i<max_slot; i++)
		        L_insert_oper_before(cb, oper, Lsched_create_nop(j, i));
	    }

	    if (Lsched_pad_vliw_slots_with_nops && (issue_slot != 0))
	    {
	        for (i=0; i<issue_slot; i++)
	            L_insert_oper_before(cb, oper, 
		        Lsched_create_nop(issue_time, i));
	    }
	}
	else if (Lsched_pad_vliw_slots_with_nops && 
	    (issue_slot != ((last_slot+1)%Lsched_total_issue_slots)))
	{
	    for (i=(last_slot+1)%Lsched_total_issue_slots; 
		 i<issue_slot; i++)
	        L_insert_oper_before(cb, oper, 
		    Lsched_create_nop(issue_time, i));
	}
	last_time = issue_time;
	last_slot = issue_slot;
    }

    /* Fill in the unused slots from the last cycle */
    if ((found) && (Lsched_pad_vliw_slots_with_nops))
        for (i=last_slot+1; i<=max_slot; i++)
            L_insert_oper_after(cb, cb->last_op, 
		Lsched_create_nop(last_time, i));

}

#define NO_CB_PAD	0x7FFFFFFF
int Lsched_get_cb_start_pad_value(L_Cb *cb)
{
    L_Attr *attr;

    if ((attr = L_find_attr(cb->attr, "start_pad"))!= 0)
	return attr->field[0]->value.i;
    else
	return NO_CB_PAD;
}

void Lsched_create_cb_start_pad(L_Cb *cb, int pad_value)
{
    L_Attr *attr;

    if ((attr = L_find_attr(cb->attr, "start_pad"))!= 0)
	attr->field[0]->value.i = pad_value;
    else
    {
	attr = L_new_attr("start_pad", 1);
	L_set_int_attr_field(attr, 0, pad_value);
	cb->attr = L_concat_attr(cb->attr, attr);
    }
}

int Lsched_get_cb_end_pad_value(L_Cb *cb)
{
    L_Attr *attr;

    if ((attr = L_find_attr(cb->attr, "end_pad"))!= 0)
	return attr->field[0]->value.i;
    else
	return 0;
}

void Lsched_create_cb_end_pad(L_Cb *cb, int pad_value)
{
    L_Attr *attr;

    if ((attr = L_find_attr(cb->attr, "end_pad"))!= 0)
	attr->field[0]->value.i = pad_value;
    else
    {
	attr = L_new_attr("end_pad", 1);
	L_set_int_attr_field(attr, 0, pad_value);
	cb->attr = L_concat_attr(cb->attr, attr);
    }
}

int Lsched_get_br_pad_value(L_Oper *oper)
{
    L_Attr *attr;

    if ((attr = L_find_attr(oper->attr, "br_pad"))!= 0)
	return attr->field[0]->value.i;
    else
	return 0;
}

void Lsched_create_br_pad(L_Oper *oper, int pad_value)
{
    L_Attr *attr;

    if ((attr = L_find_attr(oper->attr, "br_pad"))!= 0)
	attr->field[0]->value.i = pad_value;
    else
    {
	attr = L_new_attr("br_pad", 1);
	L_set_int_attr_field(attr, 0, pad_value);
	oper->attr = L_concat_attr(oper->attr, attr);
    }
}


int Lsched_cb_completion_time(L_Cb *cb)
{
    if (!cb->last_op) return 0;

    if (IS_IGNORE(cb->last_op->proc_opc)) return 0;

    return (SCHED_INFO(cb->last_op)->issue_time + 1);
}

int Lsched_need_pad(L_Cb *target, L_Oper *oper, int overextend)
{
    L_Oper 	*cur_oper;
    int		src, dest;

    if (overextend <= 0) return 0;

    for (cur_oper = target->first_op; cur_oper != NULL; 
	 cur_oper = cur_oper->next_op)
    {
        if (IS_IGNORE(cur_oper->proc_opc)) continue;

	if (SCHED_INFO(cur_oper)->issue_time >= overextend) return 0;

	/* 
	 * If any of the dest operands match a source operand, then
	 * we have an interlock condition and must pad with nops.
	 */
	for (dest=0; dest<L_max_dest_operand; dest++)
	    for (src=0; src<L_max_src_operand; src++)
		if ((oper->dest[dest]!=NULL) &&
		     L_same_operand(oper->dest[dest], cur_oper->src[src]))
		    return 1;
    }

    return 0;
}

static void Lsched_determine_pad_for_vliw(L_Cb *cb)
{
    L_Cb	*target_cb, *fall_thru_cb;
    L_Oper 	*oper, *dep_oper;
    L_Dep	*dep;
    int		br_pad, cb_pad, overextend, cb_overextend, max_cb_overextend,
		cb_completion_time; 

    fall_thru_cb = L_fall_thru_path(cb);
    cb_completion_time = Lsched_cb_completion_time(cb);
    max_cb_overextend = 0;

    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
	if (IS_IGNORE(oper->proc_opc)) continue;

	if (L_cond_branch_opcode(oper))
	{
	    br_pad = Lsched_get_br_pad_value(oper);

	    target_cb = oper->src[2]->value.cb;
	    cb_pad = Lsched_get_cb_start_pad_value(target_cb);

	    for (dep = DEP_INFO(oper)->input_dep; dep != 0; dep=dep->next_dep)
	    {
		dep_oper = dep->from_oper;

		if (IS_IGNORE(dep_oper->proc_opc)) continue;

	        overextend = Lsched_completion_time(dep_oper) - 
		    Lsched_completion_time(oper);

		if (overextend < 0) overextend = 0;

		if ((overextend != 0) && 
		    (Lsched_need_pad(target_cb, oper, overextend)))
		    continue;

	        if (overextend > br_pad)
		    Lsched_create_br_pad(oper, overextend);

		if (overextend < cb_pad)
		    Lsched_create_cb_start_pad(target_cb, overextend);
	    }
	}
	else if (L_uncond_branch_opcode(oper))
	{
	    br_pad = Lsched_get_br_pad_value(oper);

	    target_cb = oper->src[0]->value.cb;
	    cb_pad = Lsched_get_cb_start_pad_value(target_cb);

	    for (dep = DEP_INFO(oper)->input_dep; dep != 0; dep=dep->next_dep)
	    {
		dep_oper = dep->from_oper;

		if (IS_IGNORE(dep_oper->proc_opc)) continue;

	        overextend = Lsched_completion_time(dep_oper) - 
		    Lsched_completion_time(oper);

		if (overextend < 0) overextend = 0;

		if ((overextend != 0) && 
		    (Lsched_need_pad(target_cb, oper, overextend)))
		    continue;

	        if (overextend > br_pad)
		    Lsched_create_br_pad(oper, overextend);

		if (overextend < cb_pad)
		    Lsched_create_cb_start_pad(target_cb, overextend);
	    }
	}

	cb_overextend = Lsched_completion_time(oper) - cb_completion_time;

	if (fall_thru_cb && (cb_overextend > max_cb_overextend) && 
	    Lsched_need_pad(fall_thru_cb, oper, cb_overextend))
	{
	    max_cb_overextend = cb_overextend;
	}
    }

    if (fall_thru_cb)
    {
	cb_pad = Lsched_get_cb_end_pad_value(fall_thru_cb);
	if (max_cb_overextend > cb_pad)
	    Lsched_create_cb_end_pad(cb, max_cb_overextend);

	cb_pad = Lsched_get_cb_start_pad_value(fall_thru_cb);
	if (max_cb_overextend < cb_pad)
	    Lsched_create_cb_start_pad(fall_thru_cb, max_cb_overextend);
    }
}

static void Lsched_pad_cycles_start_of_cb_for_vliw(L_Cb *cb)
{
    L_Oper	*oper;
    int		cb_pad, issue_time;

    cb_pad=Lsched_get_cb_start_pad_value(cb);

    if ((cb_pad == 0) || (cb_pad == NO_CB_PAD)) return;

    /* Delay the issue and completion time for all instructions */
    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
	if (!IS_IGNORE(oper->proc_opc))
	{
	    SCHED_INFO(oper)->issue_time+=cb_pad;
#if 0
BLD 6/95
	    SCHED_INFO(oper)->completion_time+=cb_pad;
#endif
	}
    }

    /* Now add the padded number of no-op cycles to beginning of cb */
    for (issue_time=0; issue_time < cb_pad; issue_time++)
    {
	L_insert_oper_before(cb, cb->first_op, 
	    Lsched_create_nop(issue_time, 0));
    }
}

static void Lsched_pad_cycles_end_of_cb_for_vliw(L_Cb *cb)
{
    L_Cb	*fall_thru_cb;
    int		end_pad, start_pad, issue_time, last_time; 

    end_pad=Lsched_get_cb_end_pad_value(cb);

    if (end_pad <= 0) return;

    fall_thru_cb = L_fall_thru_path(cb);

    start_pad = Lsched_get_cb_start_pad_value(fall_thru_cb);

    end_pad -= start_pad;

    if (end_pad <= 0) return;

    if ((cb->last_op) && (!IS_IGNORE(cb->last_op->proc_opc)))
	last_time = SCHED_INFO(cb->last_op)->issue_time+1;
    else
	last_time = 0;

    /* Now add the padded number of no-op cycles to beginning of cb */
    for (issue_time=last_time; issue_time < last_time + end_pad; issue_time++)
    {
	L_insert_oper_after(cb, cb->last_op, 
	    Lsched_create_nop(issue_time, 0));
    }
}

static void Lsched_pad_cycles_for_vliw(L_Func *fn, L_Cb *cb)
{
    L_Cb	*target_cb, *new_cb;
    L_Oper 	*oper, *new_oper;
    L_Operand	*target;
    L_Flow	*dest_flow, *new_flow, *src_flow;
    int		i, br_pad, cb_pad, issue_time; 

    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
	if ((L_cond_branch_opcode(oper)) && 
	    (br_pad = Lsched_get_br_pad_value(oper))!=0)
	{
	    target_cb = oper->src[2]->value.cb;
	    cb_pad = Lsched_get_cb_start_pad_value(target_cb);

	    if (cb_pad == br_pad) continue;

	    /* Correct/add the flow arcs to point to the new cb and back */
	    dest_flow = L_find_flow_for_branch(cb, oper);
	    src_flow = L_find_flow(target_cb->src_flow, 
		dest_flow->cc, dest_flow->src_cb, dest_flow->dst_cb);

	    /* Create a new cb */
            new_cb = L_create_cb (dest_flow->weight);

	    dest_flow->dst_cb = new_cb;

	    new_flow = L_new_flow(dest_flow->cc, cb, new_cb,
		dest_flow->weight);
	    new_cb->src_flow = L_concat_flow(new_cb->src_flow, 
		new_flow);


	    /* 
	     * Correct/add the flow arcs to point from new cb to target 
	     * and back.
	     */
	    new_flow = L_new_flow(1.0, new_cb, target_cb, dest_flow->weight);
	    new_cb->dest_flow = L_concat_flow(new_cb->dest_flow, 
		new_flow);

	    src_flow->src_cb = new_cb;
	    src_flow->cc = 1.0;


	    target = oper->src[2];
	    oper->src[2] = L_new_cb_operand(new_cb);
	    L_insert_cb_after(fn, fn->last_cb, new_cb);

	    /* Create a jump to the original target cb */
	    br_pad = br_pad - cb_pad - 1;
	    new_oper = Lsched_create_jump(br_pad, 0, target);
	    L_delete_operand(target);
	    L_insert_oper_before(new_cb, new_cb->first_op, new_oper);

	    /* Add the remaining pad cycles */
	    for (issue_time=0; issue_time<br_pad; issue_time++)
		L_insert_oper_before(new_cb, new_cb->first_op, 
	    	    Lsched_create_nop(issue_time, 0));
	}
	else if ((L_uncond_branch_opcode(oper)) && 
	    (br_pad = Lsched_get_br_pad_value(oper))!=0)
	{
	    target_cb = oper->src[0]->value.cb;
	    cb_pad = Lsched_get_cb_start_pad_value(target_cb);

	    if (cb_pad == br_pad) continue;

	    /* 
	     * Delay the initiation and completion time of the
	     * jump to br_pad - cb_pad cycles later.
	     */
	    issue_time = SCHED_INFO(oper)->issue_time;
	    br_pad -= cb_pad;
	    SCHED_INFO(oper)->issue_time += br_pad;
#if 0
BLD 6/95
	    SCHED_INFO(oper)->completion_time += br_pad;
#endif

	    /* Add the remaining pad cycles */
	    if (((oper->prev_op) && 
		 (SCHED_INFO(oper->prev_op)->issue_time == issue_time)) ||
		((oper->next_op) &&
		 (SCHED_INFO(oper->next_op)->issue_time == issue_time)))
		issue_time++;

	    for (i = issue_time+1; i<issue_time + br_pad; i++)
		L_insert_oper_before(cb, oper, Lsched_create_nop(i, 0));
	}
    }
}

static void Lsched_correct_schedule_for_vliw(L_Func *fn)
{
    L_Cb	*cb;

    if ((M_arch != M_HPPA) && (M_arch != M_IMPACT) && (M_arch != M_PLAYDOH))
	L_punt ("VLIW correction only supported for Lhppa, Limpact, or Lplaydoh.");
    /* 
     * Determine the number of cycles that must be padded to ensure correct
     * execution.
     */
    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
	Lsched_determine_pad_for_vliw(cb);
    }

    /* 
     * First we will pad the the beginning and end of any cb's to minimize
     * the number of cycles padded.
     */
    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
	Lsched_pad_cycles_start_of_cb_for_vliw(cb);
	Lsched_pad_cycles_end_of_cb_for_vliw(cb);
    }

    /* 
     * Pad the CBs with no-op cycles to ensure correct operation without
     * interlocking hardware.
     */
    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
	Lsched_pad_cycles_for_vliw(fn, cb);
    }

    /* 
     * Padding cbs with no-ops can create new cbs which will invalidate
     * the existing dataflow.  
     */
    if (Lsched_print_statistics)
        L_do_pred_flow_analysis(fn, LIVE_VARIABLE);

    /*
     * Assuming simplistic VLIW model, we will pad any unfilled slots
     * with no-ops.
     */
    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
	Lsched_pad_slots_with_nops(cb);
    }
}



/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void Lsched_add_isl_attr (L_Func *fn)
{
  int i, dest_count;
  L_Oper *oper;
  L_Cb *cb;
  Sched_Info *sinfo;
  L_Attr *attr;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (oper = cb->first_op; oper; oper = oper->next_op)
      if (oper->ext) {
        sinfo = SCHED_INFO(oper);
	attr = L_find_attr(oper->attr, "isl");
        if (attr != NULL)  /* delete the old isl attr */
	  oper->attr = L_delete_attr(oper->attr, attr);

	dest_count = 0;
	for(i = 0; i < L_max_dest_operand; i++)
	  {
	    if (oper->dest[i] != NULL)
	      dest_count = i + 1;
	  }

	attr = L_new_attr("isl", 2 + dest_count);
	
	for (i = 0; i < dest_count; i++ )
	  {
	    if (oper->dest[i] != NULL) {
	      L_set_int_attr_field(attr, i+2, 
				   sinfo->relative_latency[i]);
	    }
	  }

        L_set_int_attr_field (attr, 0 , sinfo->issue_time);
        L_set_int_attr_field (attr, 1, sinfo->issue_slot);
        oper->attr = L_concat_attr (oper->attr, attr);
      }
}

void Lsched_delete_isl_attr (L_Func *fn)
{
  L_Oper *oper;
  L_Cb   *cb;
  L_Attr *attr;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (oper = cb->first_op; oper; oper = oper->next_op)
      if (oper->ext) {
        attr = L_find_attr (oper->attr, "isl");
        if (!attr)
          L_punt ("ERROR: isl attribute expected, but not found!!!!!");
        oper->attr = L_delete_attr (oper->attr, attr);
      }
}

void Lsched_fix_flows (L_Cb *cb)
/* Reorders flows so they match the order of branches.  With predication, 
 * branch order can change */
{
    L_Oper      *oper;
    Sched_Info  *sinfo;
    L_Flow      **flow_ptrs, *flow, *next_flow;
    int         flow_count, index, branch_count;
    L_Cb        *target_cb;

    /* only execture the remainder of this function if flows exist which can
     * be reordered. */
    if (!cb->dest_flow) return;

    /* keep tabs of all flow pointers so that their order can be easily
     * manipulated. */
    flow_count = 0;
    for (flow = cb->dest_flow; flow; flow = flow->next_flow)
        flow_count++;

    flow_ptrs = (L_Flow **) malloc (flow_count * sizeof (L_Flow *));

    index = 0;
    for (flow = cb->dest_flow; flow; flow = next_flow) {
        next_flow = flow->next_flow;
        flow_ptrs[index++] = flow;
        flow->next_flow = NULL;
        flow->prev_flow = NULL;
    }

    /* change the actual order of the outgoing flows */
    cb->dest_flow = NULL;
    branch_count = 0;
    for (oper = cb->first_op; oper; oper = oper->next_op) {
        if (L_general_branch_opcode (oper)) {
            branch_count++;
            sinfo = SCHED_INFO(oper);
            cb->dest_flow = L_concat_flow (cb->dest_flow, 
                                           flow_ptrs[sinfo->orig_block]);
            if (!L_register_branch_opcode(oper)) {
                target_cb = L_find_branch_dest (oper);
                if (target_cb != flow_ptrs[sinfo->orig_block]->dst_cb)
                    L_punt ("ERROR: branch target cb doesn't match flow target");
            }
        }
    }

    if ((branch_count + 1) == flow_count) /* take care of fall-thru */
        cb->dest_flow = L_concat_flow (cb->dest_flow,
                                       flow_ptrs[flow_count - 1]);
    else if (L_register_branch_opcode(cb->last_op)) { /* take care of jump rg */
        for (index = branch_count; index < flow_count; index++)
            cb->dest_flow = L_concat_flow (cb->dest_flow, flow_ptrs[index]);
    }
    else if ((branch_count > flow_count) || ((branch_count + 1) < flow_count))
        L_punt ("ERROR: comparison error between flow_count and branch_count");

    free (flow_ptrs);
}

void Lsched_schedule_block(cb)
    L_Cb        *cb;
{
    int         current_block=0, issue_slot, spec, work, 
		silent, num_exits;
    L_Oper      *oper, *next_op, **exit_ops;
    Sched_Info  *sinfo;

    if (Lsched_num_opers==0) return;

    /* Calculate early and late times, and then calculate the priority function
     * from these values.  This is used in place of 
     * Lsched_compute_static_priorities. */
    Lsched_init_etimes_and_ltimes (cb);
    num_exits = Lsched_determine_etimes (cb, &exit_ops);
    Lsched_determine_ltimes (cb, num_exits, exit_ops);
    Lsched_calculate_priority_normal (cb, num_exits, exit_ops);
    free (exit_ops);

    /* Perform initialization for dynamic scheduling heuristic */
    if (Lsched_regpres_heuristic) Lregpres_init_cb (cb);

    /* Initialize the RU manager */
    RU_map_init(RU_MODE_ACYCLIC, 0);

    /*
     * Place the operations into the either the pending_ready queue or 
     * the not_ready queue based upon dependences.
     */
    for (oper=cb->first_op; oper!=NULL; oper=next_op)
    {
        next_op = oper->next_op;

	/* We only schedule instructions that are not to be ignored */
        if (oper->ext!=NULL)
        {
            sinfo = SCHED_INFO(oper);

    	    if ( (Lsched_fill_delay_slots) &&
		 (Lsched_handle_spilled_branch(cb, sinfo)) )
		    next_op = oper->next_op;

            L_remove_oper(cb, oper);

            if (sinfo->num_depend==0)
            {
                sinfo->ready_time = sinfo->dep_info->level;
		L_enqueue_min_to_max_1 (pending_ready_queue,
                    sinfo, (float) sinfo->ready_time);
            }
            else
            {
		L_enqueue(not_ready_queue, sinfo);
            }
        }
    }

    /* Perform initial seeding for branch delay slots */
    if (Lsched_fill_delay_slots)
        Lsched_check_for_fill_candidates(pending_ready_queue);

    /*
     * The following outlines the steps followed during prepass list scheduling
     * each cycle.
     *
     * 1) Attempt to schedule all instructions from the priority ready queue.
     *	  skip over those that can't be scheduled.  Every time an instruction
     *    is aged into the priority ready queue prior to the last entry that
     *    had been attempted to be scheduled, the current entry is set to 
     *    the new entry.
     *
     * 2) Reset the current entry to the head of the queue.
     *
     * 3) If the priority ready queue is not empty, we are not able to schedule any 
     *	  more instructions at the current cycle, therefore we will increment the 
     *    timer by one tick.
     *
     *    If the queue is empty, there are no more instructions to schedule.  We
     *    increment the timer to the time of the earliest instruction in the pending
     *    ready queue.
     *
     * 4) We will move all instructions that can be scheduled at the current time
     *    from the pending ready queue to the priority ready queue.
     */
    L_init_ready_queue();

    work = L_get_queue_size(pending_ready_queue);

    while (work)
    {
        if (current_time > 50000)
            L_punt ("Lsched_schedule_block:  Infinite loop ?\n");


        /* 1) Attempt to schedule all instructions in the ready list */
        while ((sinfo = L_get_next_entry())!=NULL)
        {
            /* Determine if it is legal to schedule this instruction */
            if (Lsched_can_schedule_op(cb, sinfo, current_time, current_block, 
	          &issue_slot, &spec, &silent) == CAN_SCHEDULE)
	    {
                /* Allocate the processor resources for this operation */
                RU_schedule_op_at(sinfo->ru_info, sinfo->mdes_info,
	            sinfo->operand_ready_times,
                    current_time, issue_slot, silent);

                sinfo->issue_slot = issue_slot;
                sinfo->current_block = current_block;

                if (Lsched_fill_delay_slots)
                    Lsched_schedule_with_delay_op(cb, sinfo, current_time,
                        current_block, issue_slot, spec);
                else
                    Lsched_schedule_op(cb, sinfo, spec, current_time);

                if (Lsched_is_branch(sinfo->proc_opc))
                    current_block=sinfo->home_block+1;

                /* Age instructions from the pending ready queue.  */
                while (L_get_queue_size(pending_ready_queue))
                {
		    sinfo = L_get_queue_head(pending_ready_queue);
                    if (sinfo->ready_time <= current_time)
                    {
                        L_dequeue(pending_ready_queue, sinfo);
			L_insert_entry (sinfo);
                    }
                    else
                        break;
                }
	    }
        }


        /* 2) Reset to the first entry in the queue */
        L_init_ready_queue();


        /* 3) Update the scheduling timer. */
        if (L_get_queue_size(priority_ready_queue))
            current_time++;
        else
            if (L_get_queue_size(pending_ready_queue))
                current_time = L_get_queue_head(pending_ready_queue)->ready_time;

        if (Lsched_infinite_issue) Lsched_infinite_issue_slot=0;

        /* 4) Age instructions from the pending_ready list to the ready list.  */
        while (L_get_queue_size(pending_ready_queue))
        {
	    sinfo = L_get_queue_head(pending_ready_queue);
            if (sinfo->ready_time <= current_time)
            {
                L_dequeue(pending_ready_queue, sinfo);
		L_insert_entry (sinfo);
            }
            else
                break;
	}

        /* If we still have no instructions in either list, we are done. */
        work = L_get_queue_size(priority_ready_queue);
    }

    /* Add any scheduled instructions back to the end of the control block.  */
    while (L_get_queue_size(scheduled_queue))
    {
	sinfo = L_get_queue_head(scheduled_queue);
	L_dequeue(scheduled_queue, sinfo);

        if (L_subroutine_return_opcode (sinfo->oper)) {
            int epilogue_found = FALSE;
            for (oper = cb->first_op; oper && !epilogue_found;
                 oper = oper->next_op)
                if (L_is_opcode (Lop_EPILOGUE, oper)) {
                    epilogue_found = TRUE;
                    L_remove_oper (cb, oper);
                    L_insert_oper_after (cb, cb->last_op, oper);
                }
        }

        L_insert_oper_after(cb, cb->last_op, sinfo->oper);
    }

    /*
     * If we reached this point and the not_ready list has any operations still
     * in it, we have a bug!  Print out the appropriate error message and state.
     */
    if (L_get_queue_size(not_ready_queue))
    {
        fprintf (stdout, "Lsched_schedule_block: scheduled opers, current_time=%d!\n",
            current_time);
        L_print_cb (stdout, NULL, cb);
        L_punt("Lsched_schedule_block: not_ready_queue is not empty!\n");
    }

    /* In order to support FRP, check to see if any branches were reordered.
     * If branch reordering occurred, fix the corresponding flows. */
    Lsched_fix_flows (cb);
}

/******************************************************************************\
 *
 * Main entry points for prepass and post pass code scheduling
 *
\******************************************************************************/

void Lsched_prepass_code_scheduling(fn) 
L_Func *fn;
{
    L_Cb 	*cb, *old_cb = NULL;
    L_Oper 	*oper, *prev_br, *post_br;
    int		home_block, is_branch;
    int 	mcb_orig_cb_maxid, mcb_already_redone_flow = 0;

    Lsched_prepass = 1;

    Lsched_debug_messages = Lsched_debug_prepass_scheduling;

    if (Lsched_debug_messages)
	fprintf (stdout, "prepass scheduling: %s\n", fn->name);

    /* We do not fill delay slots during prepass at all */
    Lsched_fill_delay_slots=0;

    /* Perform dataflow analysis. */
    L_do_flow_analysis(fn, LIVE_VARIABLE | DOMINATOR_CB);

    if (Lsched_static_fall_thru_weight > 0)
    {
	L_compute_static_cb_weight(fn);
        L_compute_oper_weight(fn, Lsched_static_fall_thru_weight, 0);
    }
    else
        L_compute_oper_weight(fn, 0, 1);

    Ldep_mark_safe(fn);

    /* Perform function level initialization for register pressure heuristic */
    Lregpres_init_func(fn);

#if 0
    /* breakup self anti dep instrs if doing sentinel scheudling */
    if ((L_spec_model == SENTINEL) || (L_spec_model == SRB) 
	L_sentinel_anti_dep_breakup(fn);
#endif

    mcb_orig_cb_maxid = fn->max_cb_id;

    /* Main scheduling loop */
    for (cb = fn->first_cb; cb!=NULL; cb = cb->next_cb)
    {
	/* To aid debugging the scheduler and/or the dependence graph,
	 * allow the cbs scheduled to be limited by putting a bound
	 * on the cb id's that allowed to be scheduled. -JCG 5/4/95
	 */
	if ((Lsched_debug_use_sched_cb_bounds) &&
	    ((cb->id > Lsched_debug_upper_sched_cb_bound) ||
	     (cb->id < Lsched_debug_lower_sched_cb_bound)))
	    continue;


        if (L_spec_model==MCB) {
	    if (cb->id > mcb_orig_cb_maxid && (! mcb_already_redone_flow)) {
		L_do_pred_flow_analysis(fn, LIVE_VARIABLE);
		mcb_already_redone_flow = 1;
	    }
	}

	if ( (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE)) &&
              L_do_software_pipelining )
	    continue;

        if ( Lsched_debug_messages )
	    fprintf(stdout, "prepass scheduling cb %d\n", cb->id);

	if ((cb->last_op) && IS_UCOND_BRANCH(cb->last_op->proc_opc) &&
            (!L_is_predicated(cb->last_op) || L_EXTRACT_BIT_VAL(cb->flags,
                                                L_CB_HYPERBLOCK_NO_FALLTHRU)))
	    Lsched_latest_br = cb->last_op;
	else
	    Lsched_latest_br = NULL;

	/* Build MDES information */
	L_build_cb_mdes_info (cb);

        /*  Build dependence graph */
        L_build_dependence_graph(cb, LDEP_PREPASS, LDEP_MODE_ACYCLIC);

        /* Initialize branch ptrs */
        prev_br = NULL;
        post_br = L_find_next_branch(cb, NULL);

	/* Build Scheduler information */
        Lsched_num_opers = 0;
	for (oper=cb->first_op, home_block=0; oper!=NULL; oper=oper->next_op)
	{
	    is_branch = Lsched_is_branch(oper->proc_opc);

	    if (is_branch)
		post_br = L_find_next_branch(cb, oper);

	    if (!IS_IGNORE(oper->proc_opc))
	    {
		Lsched_num_opers++;
                oper->ext = (Sched_Info *) 
		    L_create_sched_info(oper, home_block, prev_br, post_br);
	    }

	    if (is_branch)
	    {
	        home_block++;
		prev_br = oper;
	    }
	}

	/* 
	 * Insert check operations for WBS/Sentinel scheduling
	 */
	if (L_spec_model==WRITEBACK_SUPPRESSION)
	    wbs_insert_check(cb);
#if 0
	else if ((L_spec_model == SENTINEL) || (L_spec_model == SRB))
	    L_sentinel_insert_check(cb);
#endif

#if 0
        /* setup code for sentinel scheduling */
        if ((L_spec_model == SENTINEL) || (L_spec_model == SRB))
	  L_sentinel_init_update_block();
#endif

        if (L_spec_model==MCB)
            old_cb = L_copy_all_oper_in_cb (fn, cb);

        /* Code scheduling */
        Lsched_schedule_block(cb);

#if 0
	/* SAM 11-94, Reorder flow arcs if branches reordered */
	if (L_dest_flow_out_of_order(cb))
	    L_reorder_dest_flows(cb);
#endif

 	/* 
	 * Extend live ranges to control register pressure
	 */
	switch (L_spec_model)
	{
	    case WRITEBACK_SUPPRESSION:
	        wbs_extend_live_range(cb);
		break;
	
#if 0
	    case SENTINEL:
	    case SRB:
		L_sentinel_extend_live_range(cb);
#endif
   	    
	}

	if (L_spec_model == MCB)
	    Lsched_mcb_schedule_block(fn,old_cb,cb);

#if 0
        L_print_sched_info(stdout, cb);
	L_print_dependence_graph(stdout, cb);
#endif

    	if (!L_do_postpass_sched) L_annotate_instr(cb);

#if 0
        /* reset code for sentinel scheduling */
        if ((L_spec_model == SENTINEL) || (L_spec_model == SRB))
	    L_sentinel_deinit_update_block();
#endif

	L_delete_dependence_graph(cb);

	/* Delete MDES information */
	L_free_cb_mdes_info (cb);

    }

    Lsched_add_isl_attr(fn);
    /* Free up any memory allocated for the function */
    if (!L_do_postpass_sched)
    {
        /* Print out the schedule */
        if ( Lsched_print_statistics )
            L_print_schedule(EXTERN_LOG_FILE, fn);

	if (!L_do_register_allocation)
            Lsched_func_complete(fn);
    }

    if ( Lsched_print_prepass_statistics )
        L_print_schedule(EXTERN_LOG_FILE_PREPASS, fn);

    /* Perform cleanup for register pressure heuristic */
    Lregpres_deinit_func(fn);
}

void Lsched_postpass_code_scheduling(fn) 
L_Func *fn;
{
    L_Cb 	*cb; 
    L_Oper 	*oper, *prev_br, *post_br;
    int		home_block, is_branch;
    int         MCB_old_upward = 0, MCB_old_downward = 0;

    Lsched_prepass = 0;
    Lsched_num_opers=0;

    Lsched_debug_messages=Lsched_debug_postpass_scheduling;

#if 0
    if ((L_spec_model == SENTINEL) || (L_spec_model == SRB))
    {
	Ldep_allow_upward_code_perc = 0;
	Ldep_allow_downward_code_perc = 0;
    }
#endif

    if (L_spec_model == MCB) 
    {
	MCB_old_upward = Ldep_allow_upward_code_perc;
	MCB_old_downward = Ldep_allow_downward_code_perc;
	Ldep_allow_upward_code_perc = 0;
	Ldep_allow_downward_code_perc = 0;
    }

    if (Lsched_debug_messages)
	fprintf (stdout, "postpass scheduling: %s\n", fn->name);

    L_clear_all_reserved_oper_flags(fn);

    /* Determine if we are to attempt to fill non-squashing delay slots */
    Lsched_fill_delay_slots=Lsched_do_fill_nonsquashing_branches;


#if 0
    /* 
     * Perform dataflow analysis.   This information may not exist
     * if prepass scheduling is not performed.  If register allocation
     * was performed, the information is corrupted and must be redone.
     */
    if ((!L_do_prepass_sched) || L_do_register_allocation)
    {
        L_do_pred_flow_analysis(fn, LIVE_VARIABLE | DOMINATOR_CB);

        if (Lsched_static_fall_thru_weight > 0)
        {
	    L_compute_static_cb_weight(fn);
            L_compute_oper_weight(fn, Lsched_static_fall_thru_weight, 0);
        }
        else
            L_compute_oper_weight(fn, 0, 1);
    }

    if (!L_do_prepass_sched)
        Ldep_mark_safe(fn);

#endif
    if (Lsched_do_postpass_scheduling)
    {
	/* 
	 * Perform dataflow analysis.  This is always performed, which wasn't
         * true in the past.
	 */
	L_do_pred_flow_analysis(fn, LIVE_VARIABLE | DOMINATOR_CB);
	
	if (Lsched_static_fall_thru_weight > 0)
	{
	    L_compute_static_cb_weight(fn);
  	    L_compute_oper_weight(fn, Lsched_static_fall_thru_weight, 0);
	}
	else
	    L_compute_oper_weight(fn, 0, 1);
	
	if (!L_do_prepass_sched)
	    Ldep_mark_safe(fn);

        for (cb = fn->first_cb; cb!=NULL; cb = cb->next_cb)
        {
	    /* To aid debugging the scheduler and/or the dependence graph,
	     * allow the cbs scheduled to be limited by putting a bound
	     * on the cb id's that allowed to be scheduled. -JCG 5/4/95
	     */
	    if ((Lsched_debug_use_sched_cb_bounds) &&
		((cb->id > Lsched_debug_upper_sched_cb_bound) ||
		 (cb->id < Lsched_debug_lower_sched_cb_bound)))
		continue;

            if ( (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE)) &&
                  L_do_software_pipelining )
	        continue;

            if ( Lsched_debug_messages )
	        fprintf(stdout, "postpass scheduling cb %d\n", cb->id);

	    if ((cb->last_op) && IS_UCOND_BRANCH(cb->last_op->proc_opc) &&
                (!L_is_predicated(cb->last_op) || L_EXTRACT_BIT_VAL(cb->flags,
                                                L_CB_HYPERBLOCK_NO_FALLTHRU)))
		Lsched_latest_br = cb->last_op;
	    else
		Lsched_latest_br = NULL;

	    /* Build MDES information */
	    L_build_cb_mdes_info (cb);

            /*  Build dependence graph */
            L_build_dependence_graph(cb, LDEP_POSTPASS, LDEP_MODE_ACYCLIC);

	    /* Initialize branch ptrs */
            prev_br = NULL;
            post_br = L_find_next_branch(cb, NULL);

	    /* 
	     * Build Scheduler information 
	     *
	     * Note:  We don't build scheduler information for
	     * opcodes which are not real instructions
	     */
            Lsched_num_opers = 0;
	    for (oper=cb->first_op, home_block=0; oper!=NULL; 
		oper=oper->next_op)

	    {
	        is_branch = Lsched_is_branch(oper->proc_opc);

		if (is_branch)
                    post_br = L_find_next_branch(cb, oper);

	        if ((oper->ext==NULL) && !IS_IGNORE(oper->proc_opc))
	        {
	    	    Lsched_num_opers++;
                    oper->ext = (Sched_Info *) 
		        L_create_sched_info(oper, home_block, prev_br, post_br);
	        }
		else if (oper->ext!=NULL)
		{
	    	    Lsched_num_opers++;
		    L_update_sched_info(SCHED_INFO(oper), oper, home_block);
		}
		    
		if (is_branch)
                {
                    home_block++;
                    prev_br = oper;
                }
	    }

	    /* 
	     * Insert check operations for WBS scheduling
	     */
	    if (L_spec_model==WRITEBACK_SUPPRESSION)
	        wbs_insert_check(cb);
#if 0
	    else if ((L_spec_model == SENTINEL) || (L_spec_model == SRB))
	    {
                L_sentinel_insert_check(cb);
		L_sentinel_preserve_interval(cb);
	    }
#endif

#if 0
            /* setup code for sentinel scheduling */
            if ((L_spec_model == SENTINEL) || (L_spec_model == SRB))
	        L_sentinel_init_update_block();
#endif

            /* Code scheduling */
            Lsched_schedule_block(cb);

	    /* SAM 11-94, Reorder flow arcs if branches reordered */
	    if (L_dest_flow_out_of_order(cb))
	        L_reorder_dest_flows(cb);

 	    /* Append scheduler information to instrutions */ 
	    L_annotate_instr(cb);

#if 0
            /* reset code for sentinel scheduling */
            if ((L_spec_model == SENTINEL) || (L_spec_model == SRB))
	        L_sentinel_deinit_update_block();
#endif
        }
    }

    if (Lsched_processor_model == MDES_VLIW)
    {
	/* VLIW will not support delay slot filling at this time */
	if (Lsched_do_fill_nonsquashing_branches ||
            Lsched_do_fill_squashing_branches ||
            Lsched_do_fill_unfilled_branches)
	    L_warn("Filling of branch delay slots not supported for VLIW - ignored!");
        if (!Lsched_do_postpass_scheduling && !L_do_prepass_sched)
	    L_punt("Can't correct vliw schedule if code is not scheduled!");

	if (! Lsched_vliw_has_interlocking)
	    Lsched_correct_schedule_for_vliw(fn);
    }
    else
    {
        /* Handle nonsquashing branches */
        if (Lsched_do_fill_nonsquashing_branches)
            Lsched_fill_nonsquashing_branches(fn);

        /* Handle squashing branches */
        if (Lsched_do_fill_squashing_branches)
            Lsched_fill_squashing_branches(fn);

        /* Fill any unfilled branch slots */
        if (Lsched_do_fill_unfilled_branches)
            Lsched_fill_unfilled_branches(fn);
    }

    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
        /* Delete dependence graph */
	L_delete_dependence_graph(cb);

	/* Delete MDES information */
	L_free_cb_mdes_info (cb);
    }

    if (L_spec_model == MCB)
    {
	Ldep_allow_upward_code_perc = MCB_old_upward;
	Ldep_allow_downward_code_perc = MCB_old_downward;
    }

    /*
     * Print out the schedule
     */
    if ( Lsched_print_statistics )
      {
        if (Lsched_do_postpass_scheduling || L_do_prepass_sched)
	  {
            L_print_schedule(EXTERN_LOG_FILE, fn);
	  }
	else
	  {
	    L_warn("You can not estimate the static execution frequency unless the code is scheduled!\n Disable print_statistics in Scheduler section!");
	  }
      }

    Lsched_add_isl_attr(fn);
    /* Free up any memory allocated for the function */
    Lsched_func_complete(fn);
}
