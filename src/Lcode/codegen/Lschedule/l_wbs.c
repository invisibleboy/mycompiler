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
 *  File:  l_wbs.c
 *
 *  Description:  Support for write-back suppression scheduling.
 *
 *  Creation Date :  June 1993
 *
 *  Author:  Roger A. Bringmann
 *
 *  Revision 1.2  1995/09/27 16:14:56  hank
 *  Added new parameter to L_print_cb calls.
 *
 * Revision 1.1.1.1  1995/08/30  16:49:04  david
 * Import of IMPACT source
 *
 * Revision 1.2  1994/02/05  22:26:21  roger
 * Changed to support speculative yield heuristic
 *
 * Revision 1.1  1994/01/19  18:49:36  roger
 * Initial revision
 *
 *
 * 	All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_schedule.h"


/******************************************************************************\
 *
 * Routines which support write-back suppression
 *
\******************************************************************************/

static void wbs_delete_check(cb, check, current_time)
L_Cb	*cb;
L_Oper 	*check;
int	current_time;
{
    Sched_Info	*sinfo=SCHED_INFO(check);
    Dep_Info	*dep_info=DEP_INFO(check);
    L_Dep	*dep;

    /* Remove check from its queued list */
    if (L_in_queue(scheduled_queue, sinfo))
       L_punt("wbs_delete_check: Deleting check that has been scheduled");

    L_dequeue_from_all (sinfo);
    
    /* Remove all input dependences */
    for (dep=dep_info->input_dep; dep!=NULL; dep=dep_info->input_dep)
    {
	Dep_Info 	*from_dep_info=DEP_INFO(dep->from_oper);	
	int		type=dep->type;

	dep_info->input_dep = L_remove_dep(dep_info->input_dep, 
	    &dep_info->n_input_dep, type, dep->from_oper, check);
	
	from_dep_info->output_dep = L_remove_dep(from_dep_info->output_dep, 
	    &from_dep_info->n_output_dep, type, dep->from_oper, check);
    }

    /* 
     * make_ready will physically remove the dependence as well place the instruction in the 
     * pending_ready list if the instruction is ready to be scheduled.
     */
    sinfo->issue_slot=0;
    for (dep=dep_info->output_dep; dep!=NULL; dep=dep_info->output_dep)
    {
        L_Oper   	*to_oper=dep->to_oper;
        Dep_Info 	*to_dep_info=DEP_INFO(to_oper);
        Sched_Info      *to_sinfo=SCHED_INFO(to_oper);
        int      	type=dep->type, prev_dep, num_dep,
			to_index=dep->to_index,
			from_index=dep->from_index;

        dep_info->output_dep = L_remove_dep(dep_info->output_dep,
            &dep_info->n_output_dep, type, check, to_oper);

        prev_dep = to_dep_info->n_input_dep;

        to_dep_info->input_dep = L_remove_dep(to_dep_info->input_dep,
            &to_dep_info->n_input_dep, type, check, to_oper);

        /* L_remove_dep can remove multiple dependences of a given type */
        num_dep = prev_dep-to_dep_info->n_input_dep;
        if (num_dep <= 0)
            L_punt("wbs_delete_check: dependence not removed!");
        else
            if (num_dep>1)
                to_sinfo->num_depend-=(num_dep-1);

        /*
         * We must make sure that dependent instruction is correctly
         * moved to pending_ready list if there are no more
         * dependences.
         */
        Lsched_make_ready(cb, current_time, sinfo, to_sinfo, to_index,
            from_index, dep->distance);
    }

    /* Free up all allocated memory */
    L_delete_dependence_info(dep_info);
    L_free_oper_mdes_info (check);
    L_delete_sched_info(sinfo);

    if (Lsched_debug_messages)
       fprintf (stdout, "> deleting check op %d\n", check->id);

    /* Delete the check instruction */
    L_delete_oper(cb, check);

}

void wbs_insert_check(cb)
L_Cb 		*cb;
{
    L_Oper 	*check=NULL, *oper, *dep_oper;
    Sched_Info	*check_sinfo, *sinfo;
    int		current_block = 0, branch, last_op,
		in_index, out_index, distance, suppress,
		check_current_block;

    out_index = operand_index(MDES_SYNC_OUT, DEP_CNT_OPERAND);
    in_index = operand_index(MDES_SYNC_IN, DEP_CNT_OPERAND);

#if 0
	if (cb->id==23)
	    L_print_cb(stdout, NULL, cb);
#endif

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op)
    {
	if (oper->ext==NULL) continue;

	sinfo = SCHED_INFO(oper);

#if 0
	if (cb->id==23)
	    L_print_oper(stdout, oper);
#endif

	current_block = sinfo->current_block;

	if (sinfo->proc_opc==Lop_CHECK)
	    check = oper;
        
	branch = Lsched_is_branch(sinfo->proc_opc);
	last_op = (oper==cb->last_op);

	if (branch || last_op)
	{
	    if ((current_block!=0) && (!check))
	    {
    		/* create the check instruction and initialize all parameters */
    		check = L_create_new_op(Lop_CHECK);
		check->weight = sinfo->weight;
    		check->dep_info = (void *) L_new_dep_info();

		L_build_oper_mdes_info (check);

		check->ext = (Sched_Info *) 
		    L_create_sched_info(check, current_block, sinfo->prev_br, 
		        sinfo->oper);

		if (branch)
		    L_insert_oper_after(cb, oper->prev_op, check);
		else
		    L_insert_oper_after(cb, oper, check);
	    }

	    /* 
	     * If we have determined that we should not create a check,
	     * we certainly should not add dependences to it!!
	     */
	    if (!check) continue;

	    check_sinfo = SCHED_INFO(check);

	    /* 
	     * Make the check instruction control dependent on previous
	     * and subsequent branch
	     */
	    check_sinfo->prev_br = sinfo->prev_br;

	    if (check_sinfo->prev_br!=NULL)
	    {
		if (!L_find_dep(DEP_INFO(check)->input_dep, 
		    check_sinfo->prev_br, check, L_DEP_CNT))
		{
	            distance = max_operand_time(sinfo->mdes_info, out_index)
	 	        - min_operand_time(check_sinfo->mdes_info, in_index);
    
		    if (distance<0) distance=0;
        
		    L_add_dep(L_DEP_CNT, distance, out_index, in_index, check_sinfo->prev_br, check, 0);
		}
	    }

	    if ( branch )
	    {
		if (!L_find_dep(DEP_INFO(check)->output_dep, check, 
		    oper, L_DEP_CNT))
		{

		    check_sinfo->post_br = oper;

		    distance = min_operand_time(check->mdes_info, in_index)
		        - max_operand_time(sinfo->mdes_info, out_index);

		    if (distance<0) distance=0;

		    L_add_dep(L_DEP_CNT, distance, in_index, out_index, check, oper, 0);
		}
	    }
	    else
		check_sinfo->post_br=NULL;


	    /*
	     * Make the check instruction control dependent on all 
	     * previous potentially excepting instructions in the control
    	     * block
	     */
	    check_current_block = check_sinfo->current_block;

	    for (dep_oper = cb->first_op; dep_oper != check; 
		 dep_oper=dep_oper->next_op)
	    {
	        /* 
		 * Note:  this code is assuming that the check and branch
		 * will not be potentially excepting instructions.
		 */
		if (IS_EXCEPTING(dep_oper->proc_opc) &&
		    (SCHED_INFO(dep_oper)->home_block==check_current_block) &&
		    !L_find_dep(check_sinfo->dep_info->input_dep, dep_oper, 
		    check, L_DEP_CNT))

		{
		    distance = max_operand_time(MDES_INFO(dep_oper), 
		        out_index) - min_operand_time(check_sinfo->mdes_info, 
			in_index);

		    if (distance<0) distance=0;
    
		    L_add_dep(L_DEP_CNT, distance, out_index, in_index, dep_oper, check, 0);
		}
	    }

	    /*
	     * Make the check instruction control dependent on all
	     * potentially excepting instructions that are exactly 
	     * except_branch_perc_limit control blocks later.
	     */

	    if (Ldep_except_branch_perc_limit != -1)
	    {
	        suppress = check_current_block+Ldep_except_branch_perc_limit;

	        for (dep_oper = cb->last_op; dep_oper != check; 
	            dep_oper=dep_oper->prev_op)
	        {
	            /* 
	             * Note: this code is assuming that the check and branch
	             * will not be potentially excepting instructions.
	             */
	            if (IS_EXCEPTING(dep_oper->proc_opc) &&
	                (SCHED_INFO(dep_oper)->home_block==suppress) &&
	                !L_find_dep(check_sinfo->dep_info->output_dep, check, 
	    	        dep_oper, L_DEP_CNT))
	            {
	                distance = min_operand_time(check_sinfo->mdes_info, in_index) 
		            - max_operand_time(MDES_INFO(dep_oper), out_index);
           
    	                if (distance<0) distance=0;
               
		        L_add_dep(L_DEP_CNT, distance, in_index, out_index, check, dep_oper, 0);
		    }
	        }
	    }

	    if (oper->next_op == check)
		oper = check;

	    check = NULL;
	}
    }
}

/* 
 * This routine will extend the live range of operands for all
 * potentially excepting instructions.  It will also indicate
 * the point which speculated instructions should maximally
 * be demoted to prevent unecessary spill code
 *
 * The demotion point is determined for all speculated instructions!
 */
void wbs_extend_live_range(cb)
L_Cb *cb;
{
    int spec_dist;
    L_Oper *oper, *ext_oper;
    Sched_Info	*sinfo, *ext_sinfo;

    /*
     * Determine if the live range for potentially excepting instructions
     * should be extended to prevent register allocation from corrupting
     * source operands.
     */
    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op)
    {
	/*
	 * Search backwards through cb to determine if their is an
	 * operation with an earlier home block located below
	 * this operation.
  	 *
	 * The first operation encountered determines where to extend
	 * the live range of the source operands.
	 */
	sinfo = SCHED_INFO(oper);

	if ( !sinfo ) continue;

	spec_dist = sinfo->home_block - sinfo->current_block;

	if (spec_dist > 0)
	{
	    for (ext_oper=sinfo->prev_br; ext_oper!=oper; 
	         ext_oper=ext_oper->prev_op)
            {
		ext_sinfo = SCHED_INFO(ext_oper);

		if (!ext_sinfo) continue;

		/* 
		 * We stop extending the live range when we find the first
		 * instruction above the home block of the speculated 
		 * potential excepting instruction that has a destination
		 * register.
		 */
                if ( (ext_sinfo->home_block < sinfo->home_block) &&
		     (ext_oper->dest!=NULL) )
                    break;
            }

	    if (IS_EXCEPTING(oper->proc_opc))
	        sinfo->extend_lr_down = ext_oper;

	}
    }
}

void wbs_remove_check(cb, branch_sinfo, current_time)
    L_Cb 	*cb;
    Sched_Info	*branch_sinfo;
    int		current_time;
{
    L_Dep       *tdep;
    Sched_Info  *tsinfo = NULL;
    Dep_Info    *tdep_info = NULL, *dep_info=branch_sinfo->dep_info;
    L_Oper      *tdep_oper=NULL;

    if (!Lsched_is_branch(branch_sinfo->proc_opc)) return;
 
    /* Search for a check instruction */
    for (tdep=dep_info->output_dep; tdep!=NULL;
         tdep=tdep->next_dep)
    {
	if (tdep->to_oper->proc_opc==Lop_CHECK)
	{
	    tdep_oper=tdep->to_oper;
	    tsinfo=SCHED_INFO(tdep_oper);
	    tdep_info=DEP_INFO(tdep_oper);
	    break;
	}
    }

    /*
     * If no input dependent instructions have been speculated,
     * delete the check
     */
    if ( (tdep_oper!=NULL) &&
         (tsinfo->num_depend==tdep_info->n_input_dep) )
	wbs_delete_check(cb, tdep_oper, current_time);
}
