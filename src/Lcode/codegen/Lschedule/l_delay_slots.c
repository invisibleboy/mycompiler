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
 *  File: l_delay_slots.c 
 *
 *  Description: This file contains the code to address filling branch
 *      delay slots.
 *
 *  Creation Date :  June, 1993
 *
 *  Author:  Roger A. Bringmann, Richard E. Hank
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:02  david
 *  Import of IMPACT source
 *
 * Revision 1.1  1994/01/19  18:49:16  roger
 * Initial revision
 *
 *
\*****************************************************************************/
/* 12/03/02 REK Taking out the lhppa requirement for distribution. */
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_schedule.h"
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
#include <Lcode/lhppa_main.h>
#endif
#endif

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

static L_Cb *L_alter_branch_destination(fn,cb,dest_cb,br,filler)
    L_Func *fn;
    L_Cb *cb,*dest_cb;
    L_Oper *br,*filler;
{
    L_Cb *new_dest;
    L_Flow *br_flow, *filler_flow, *flow;
    
    /* NOTE: The function does not properly maintain cb source flows   */
    /*       it is making the brazen assumption that someone somewhere */
    /*       is going to rebuild the source flows		       */
    if ( filler->next_op != NULL )  {
	/* create a new control block and insert <new_cb> into 
	   function after <dest_cb> */
	new_dest = L_create_cb(dest_cb->weight);
	L_insert_cb_after(fn,dest_cb,new_dest);
	   
	/* place all opers but the filler into the new cb */
	new_dest->first_op = filler->next_op;
	new_dest->first_op->prev_op = NULL;
	
	if ( dest_cb->last_op != dest_cb->first_op )  {
	     new_dest->last_op = dest_cb->last_op;
	}
	else
             new_dest->last_op = NULL;
	dest_cb->last_op = dest_cb->first_op = filler;
	filler->prev_op = NULL;
	filler->next_op = NULL;
	
	/* copy destination flows from <dest_cb> to destination flows
	   of the new cb and alter the src_cb pointer */
	new_dest->dest_flow = dest_cb->dest_flow;
	for ( flow = new_dest->dest_flow; flow != NULL ; flow = flow->next_flow ) 
	    flow->src_cb = new_dest;
	
	
	/* <dest_cb> now falls into <new_dest>, so add a fall thru flow */
	dest_cb->dest_flow = L_concat_flow(NULL,L_new_flow(0,dest_cb,
							   new_dest,dest_cb->weight));
	
	/* Now, if the filler instruction is a branch, there is some additional */
	/* L_Flow * alterations that need to be made				*/
	if ( op_flag_set(filler->proc_opc, OP_FLAG_CBR) ||
	     op_flag_set(filler->proc_opc, OP_FLAG_JMP) ) {
	    filler_flow = L_find_flow_for_branch(new_dest,filler);
	    new_dest->dest_flow = L_delete_flow(new_dest->dest_flow,filler_flow);
	    
	    dest_cb->dest_flow = L_concat_flow(dest_cb->dest_flow,
	    				L_new_flow(filler_flow->cc, dest_cb,
					    filler_flow->dst_cb, dest_cb->weight));
	}
    }
    else
	new_dest = dest_cb->next_cb;
	    
    /* correct the flow information */
    if ( cb == dest_cb )
	br_flow = L_find_flow_for_branch(new_dest,br);
    else 
        br_flow = L_find_flow_for_branch(cb,br);
    
    br_flow->dst_cb = new_dest;
	   
    return(new_dest); 	
}

/*
 * These routine assumes one delay slot.
 */

static int L_valid_branch_filler(br,filler)
L_Oper *br,*filler;
{
    int ext;
    L_Cb *dest_cb;

    L_get_attribute(br,&ext);
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
    if ( INSTR_EXT(ext) == CBR_FORWARD_EXT )
        return(1);
#endif
#endif

    if ( op_flag_set(br->proc_opc,OP_FLAG_CBR))
        dest_cb = br->src[2]->value.cb;
    else if ( op_flag_set(br->proc_opc,OP_FLAG_JMP) )
        dest_cb = br->src[0]->value.cb;
    else
        return(1);

    while ( dest_cb->first_op == NULL )
        dest_cb = dest_cb->next_cb;

    if ( op_flag_set(filler->proc_opc,OP_FLAG_STORE) &&
         op_flag_set(dest_cb->first_op->proc_opc,OP_FLAG_LOAD) ) {
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
	if ( (br->proc_opc == LHPPAop_ADDIB_LT_FWD ||
 	      br->proc_opc == LHPPAop_ADDIB_LT_BWD) &&
	     L_EXTRACT_BIT_VAL(filler->flags,L_OPER_SPILL_CODE) )  {
	    return(1);
	}
#endif
#endif
	return(0);
    }
    return(1);
}

void Lsched_fill_from_target(fn)
    L_Func *fn;
{
    int i, in_out,filler_opc;
    L_Cb *cb, *dest_cb, *next_cb;
    L_Oper *br, *filler;
    Sched_Info *br_info;
    
    for ( cb = fn->first_cb; cb != NULL; cb = next_cb )  {
	double weight = cb->weight;
	L_Flow *flow = NULL;
	next_cb = cb->next_cb;

        if ( (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE)) &&
              L_do_software_pipelining )
	    continue;

        for ( br = cb->first_op; br != NULL; br = br->next_op ) {
	    
	    /* determine if oper is a branch, doesn't make sense to */
	    /* fill jsr or rts from target			    */
	    if ( !op_flag_set(br->proc_opc,OP_FLAG_CBR) &&
	         !op_flag_set(br->proc_opc,OP_FLAG_JMP) ) 
		continue;
	    
	    if ( op_flag_set(br->proc_opc,OP_FLAG_JMP) &&
		 (L_is_reg(br->src[0]) ||
		  L_is_macro(br->src[0])))
		continue;
	    
	    br_info = SCHED_INFO(br);
	    if ( br_info == NULL ) continue;
	    
	    if ( op_flag_set(br->proc_opc,OP_FLAG_CBR) )  {
		if ( flow )  {
		    weight -= flow->weight;
		    flow = flow->next_flow;
		}
		else 
		    flow = cb->dest_flow;
		/* weight = execution frequency of current conditiona branch */
		/* flow->weight = taken frequency			     */
		/* if the branch is more likely to fall through, don't fill  */
		/* from taken path					     */
		if ( flow->weight < (weight - flow->weight) )
		    continue;
	    }
	    
	    /* if the delay slot has been filled or the instruction is */
	    /* interlocking, nothing need be done by this function     */
#if 0		/* OLD - RAB */
	    if ( L_EXTRACT_BIT_VAL(br->flags,FILLED_ABOVE) ||
		 L_EXTRACT_BIT_VAL(br->flags,FILLED_BELOW) )  {
		if ( L_valid_branch_filler(br,br->prev_op) )
		    continue;
		else  {
		    br->flags = L_CLR_BIT_FLAG(br->flags,FILLED_ABOVE);
		    br->flags = L_CLR_BIT_FLAG(br->flags,FILLED_BELOW);
		}
	    }
	    else if ( !op_flag_set(br->proc_opc, OP_FLAG_NI) )
		continue;
#endif
	    if ( L_EXTRACT_BIT_VAL(br->flags,FILLED_ABOVE) )
	    {
		if ( L_valid_branch_filler(br,br->prev_op) )
		    continue;
		else 
		    br->flags = L_CLR_BIT_FLAG(br->flags,FILLED_ABOVE);
	    }
	    else if (L_EXTRACT_BIT_VAL(br->flags,FILLED_BELOW) ) 
	    {
		if ( L_valid_branch_filler(br,br->next_op) )
		    continue;
		else 
		    br->flags = L_CLR_BIT_FLAG(br->flags,FILLED_BELOW);
	    }
	    else if ( !op_flag_set(br->proc_opc, OP_FLAG_NI) )
		continue;
	    
	    /* find first non-empty cb at destination of branch */

	    if ( op_flag_set(br->proc_opc,OP_FLAG_CBR))
	        dest_cb = br->src[2]->value.cb;
	    else
	        dest_cb = br->src[0]->value.cb;

    	    while ( dest_cb->first_op == NULL )
		dest_cb = dest_cb->next_cb;
	    
	    /* if the instruction is already filling a branch */
	    /* delay slot, forget it 			      */
	    if ( L_EXTRACT_BIT_VAL(dest_cb->first_op->flags,FILLER) )
		continue;
	    
	    /* if the first instruction is not a valid branch filler, forget it     */
	    /* at this moment, I don't feel like filling branch slots with branches */
	    filler_opc = dest_cb->first_op->proc_opc;
	    if ( op_flag_set(filler_opc,OP_FLAG_EXPANDS) ||
		 op_flag_set(filler_opc,OP_FLAG_CBR) ||
		 op_flag_set(filler_opc,OP_FLAG_JMP) ||
		 op_flag_set(filler_opc,OP_FLAG_JSR) ||
		 op_flag_set(filler_opc,OP_FLAG_RTS) ||
		 op_flag_set(filler_opc,OP_FLAG_IGNORE) ||
		 op_flag_set(filler_opc,OP_FLAG_SYNC) )
		    continue;

  	    if ( op_flag_set(br->proc_opc,OP_FLAG_CBR) &&
                 op_flag_set(filler_opc,OP_FLAG_EXCEPT) )
                continue;
	    
	    if ( op_flag_set(br->proc_opc,OP_FLAG_CBR) )  {
	    	in_out = 0;
	    	for ( i = 0; i < L_max_dest_operand; i++ )  {
		    L_Operand *dest;
		    if ( (dest = dest_cb->first_op->dest[i]) == NULL )
		    	continue;
		    if ((L_is_reg(dest) ||
			 L_is_macro(dest)) &&
		    	L_in_oper_OUT_set(cb,br,dest,FALL_THRU_PATH) )  {
		    	in_out = 1;
		    	break;
		   }
	        }
	        if ( in_out ) continue;
	    }
#if 0
	    /* if the instruction cannot be scheduled after?? the branch, give up */
	    if ( 1 /* check RU map for schedulability ^^^^^ <- maybe before?? */ )
		continue;
#endif
	    /* alter the branch destination */
	    if ( op_flag_set(br->proc_opc,OP_FLAG_CBR) )  {
	        br->src[2]->value.cb = L_alter_branch_destination(fn,cb,dest_cb,
						 br,dest_cb->first_op);
	        if ( cb == dest_cb ) cb = br->src[2]->value.cb;
	    }
	    else  {
		br->src[0]->value.cb = L_alter_branch_destination(fn,cb,dest_cb,
						br,dest_cb->first_op);
		if ( cb == dest_cb ) cb = br->src[0]->value.cb;
	    }
	    
	    /* copy the instruction and place it before the branch */
	    /* It will be swapped later! */
	    filler = L_copy_operation(dest_cb->first_op);
	    L_insert_oper_before(cb,br,filler);
	    
	    br->flags = L_SET_BIT_FLAG(br->flags,FILLED_ABOVE);
	    filler->flags = L_SET_BIT_FLAG(filler->flags,FILLER);
	}
    }    
}

void Lsched_fill_nonsquashing_slots(fn)
    L_Func *fn;
{
    L_Cb *cb;
    L_Oper *br, *next_op;
    

    for ( cb = fn->first_cb; cb != NULL ; cb = cb->next_cb )  {

        if ( (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE)) &&
              L_do_software_pipelining )
	    continue;


	for ( br = cb->first_op; br != NULL; br = next_op )  {
	    next_op = br->next_op;
	    if ( L_EXTRACT_BIT_VAL(br->flags,FILLED_ABOVE) &&
		 !L_EXTRACT_BIT_VAL(br->flags,L_OPER_SQUASHING) )  {
		L_Oper *filler = br->prev_op;
		L_remove_oper(cb,filler);
		L_insert_oper_after(cb,br,filler);
	    }
	}
    }

}

void Lsched_fill_nonsquashing_branches(fn)
   L_Func *fn;
{
    L_do_flow_analysis(fn, LIVE_VARIABLE);
    
    /* Attempt to fill any remaining branches from target */
    Lsched_fill_from_target(fn);
    Lsched_fill_nonsquashing_slots(fn);
}

void Lsched_fill_squashing_branches(fn)
   L_Func *fn;
{
    L_Cb *cb, *dest_cb, *next_cb; 
    L_Oper *br, *filler, *next_op;
    Sched_Info *br_info;
    int	filler_opc;
    
    for ( cb = fn->first_cb; cb != NULL; cb = next_cb )  {
	next_cb = cb->next_cb;
        if ( (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE)) &&
              L_do_software_pipelining )
	    continue;
        for ( br = cb->first_op; br != NULL; br = next_op ) {
	    next_op = br->next_op;
	    
	    /* determine if oper is a branch, jsr or rts instruction */
	    if ( !op_flag_set(br->proc_opc,OP_FLAG_CBR) &&
	         !op_flag_set(br->proc_opc,OP_FLAG_JMP) &&
	         !op_flag_set(br->proc_opc,OP_FLAG_JSR) &&
		 !op_flag_set(br->proc_opc,OP_FLAG_RTS) )
		continue;

	    br_info = SCHED_INFO(br);
	    if ( br_info == NULL ) continue;
	    
	    /* if the delay slot has been filled or the instruction is */
	    /* interlocking, nothing need be done by this function     */
	    if ( L_EXTRACT_BIT_VAL(br->flags,FILLED_ABOVE) ||
		 L_EXTRACT_BIT_VAL(br->flags,FILLED_BELOW) ||
		 !op_flag_set(br->proc_opc, OP_FLAG_NI) ) 
		continue;
	    
	    /* if branch is squash not taken, fill from destination */
	    if ( alt_flag_set(br->proc_opc,RU_SELECTED_ALT_ID(br_info->ru_info), 
		 		ALT_FLAG_NN) )  {
		/* find first non-empty cb at destination of branch */
	    	dest_cb = br->src[2]->value.cb;
    	    	while ( dest_cb->first_op == NULL )
		    dest_cb = dest_cb->next_cb;
	    
#if 0
	    	/* if the first instruction is not a valid branch filler, forget it */
	    	if ( 1 /* dest_cb->first_op is an invalid branch filler */ )
		    continue;
#endif
	    
	  	/* if the first instruction is not a valid branch filler, forget it     */
	    	/* at this moment, I don't feel like filling branch slots with branches */
	        filler_opc = dest_cb->first_op->proc_opc;
	    	if ( op_flag_set(filler_opc,OP_FLAG_EXPANDS) ||
		     op_flag_set(filler_opc,OP_FLAG_CBR) ||
		     op_flag_set(filler_opc,OP_FLAG_JMP) ||
		     op_flag_set(filler_opc,OP_FLAG_JSR) ||
		     op_flag_set(filler_opc,OP_FLAG_RTS) ||
		     op_flag_set(filler_opc,OP_FLAG_IGNORE) ||
		     op_flag_set(filler_opc,OP_FLAG_SYNC) )
		    continue;
		
#if 0
	    	/* if the instruction cannot be scheduled after?? the branch, give up */
	    	if ( 1 /* check RU map for schedulability ^^^^^ <- maybe before?? */ )
		    continue;
#endif
	    
	    	/* copy the instruction and place it after the branch */
	    	filler = L_copy_operation(dest_cb->first_op);
	    	L_insert_oper_after(cb,br,filler);
		
	    	br->flags = L_SET_BIT_FLAG(br->flags,FILLED_BELOW);
		br->flags = L_SET_BIT_FLAG(br->flags,L_OPER_SQUASHING);
	    	filler->flags = L_SET_BIT_FLAG(filler->flags,FILLER);
		
		/* alter branch destination, needs to be destination + 4 */
	    	br->src[2]->value.cb = L_alter_branch_destination(fn,cb,dest_cb,br,
								  dest_cb->first_op);
		if ( cb == dest_cb ) cb = br->src[2]->value.cb;
	    }
	    /* else if branch is squash taken, fill from below */
	    else if ( alt_flag_set(br->proc_opc,RU_SELECTED_ALT_ID(br_info->ru_info),
				   ALT_FLAG_NT) )  {
	
		if ( br->next_op )
		    filler = br->next_op;
		else  {
		    L_Cb *tmp;
		    
		    if ( op_flag_set(br->proc_opc,OP_FLAG_JMP) ||
			 op_flag_set(br->proc_opc,OP_FLAG_RTS) )
			continue;
		    
		    tmp = cb->next_cb;
		    while ( tmp->first_op == NULL )  
			tmp = tmp->next_cb;
		    filler = tmp->first_op;
		}
		if ( op_flag_set(filler->proc_opc,OP_FLAG_JSR) ||
		     op_flag_set(filler->proc_opc,OP_FLAG_RTS) ||
		     op_flag_set(filler->proc_opc,OP_FLAG_IGNORE) ||
		     op_flag_set(filler->proc_opc,OP_FLAG_SYNC) )
		    continue;
		br->flags = L_SET_BIT_FLAG(br->flags,FILLED_BELOW);
		br->flags = L_SET_BIT_FLAG(br->flags,L_OPER_SQUASHING);
	
	    }
	}
    }
    /* correct the source flows */
}

void Lsched_fill_unfilled_branches(fn)
    L_Func *fn;
{
    int         total_slots=0, total_filled=0;
    L_Cb        *cb;
    L_Oper      *oper, *nop;
    Sched_Info  *sinfo;

    for ( cb = fn->first_cb; cb != NULL; cb = cb->next_cb )  {
        if ( (L_EXTRACT_BIT_VAL(cb->flags, L_CB_SOFTPIPE)) &&
              L_do_software_pipelining )
	    continue;
        for ( oper = cb->first_op; oper != NULL; oper = oper->next_op ) {
	    
	    /* determine if oper is a branch, jsr or rts instruction */
	    if ( !op_flag_set(oper->proc_opc, OP_FLAG_CBR | OP_FLAG_JMP |
			      		      OP_FLAG_JSR | OP_FLAG_RTS) )
		continue;
	    
	    /* if the delay slot has been filled or the instruction is */
	    /* interlocking, nothing need be done by this function     */
	    if ( L_EXTRACT_BIT_VAL(oper->flags,FILLED_ABOVE) ||
		 L_EXTRACT_BIT_VAL(oper->flags,FILLED_BELOW) )  {
	        total_slots++;
		total_filled++;
	    	continue;
	    }
	    sinfo = SCHED_INFO(oper);
	    if ( sinfo && !op_flag_set(oper->proc_opc, OP_FLAG_NI) )
		continue;
	    
	    /* Insert a nop after the unfilled instruction */
	    nop = L_create_new_op_using(Lop_NO_OP, NULL);
	    L_insert_oper_after(cb, oper, nop);
	    oper = nop;
	    
	    total_slots++;
        }
    }

    if ( Lsched_debug_unfilled_branches ) {
	fprintf (stderr, "\n======================================\n");
        fprintf (stderr, "    Total slots filled      : %d\n",
            total_filled);
        fprintf (stderr, "    Total slots             : %d\n",
            total_slots);
        fprintf (stderr, "    Percent of slots filled : %.2f\n",
            (((float) total_filled)/(float) total_slots)*100.0);
	fprintf (stderr, "======================================\n");
    } }

/*****************************************************************************\
 *
 * Routines used to support filling delay slots for instructions within the
 * superblock during list scheduling.
 *
\*****************************************************************************/

/* 
 * This is a hack for the PA-7100.  Handle the case of an add_and_branch
 * instruction.  This instruction has a destination which could be
 * end up allocated to a spill register!  If we do not place the
 * spill code into the delay slot, then there is no way in hell the
 * the code can be expected to work correctly!
 */
int Lsched_handle_spilled_branch(L_Cb *cb, Sched_Info *sinfo)
{
    L_Oper	*oper, *delay_oper;

    oper = sinfo->oper;

    /* Is this a branch with a delay slot? */
    if (!IS_CTL(oper->proc_opc)) return 0;

    if (!HAS_DELAY_SLOT(oper->proc_opc)) return 0;

    /* Is there a destination field */
    if (L_is_null(oper->dest[0])) return 0;

    /* Is the destination spilled */
    delay_oper = oper->next_op;

    if (delay_oper == NULL) return 0;

    if (!IS_MEMORY_STORE(delay_oper->proc_opc)) return 0;

    if (!L_same_operand(oper->dest[0], delay_oper->src[2])) return 0;
    
    if (!L_EXTRACT_BIT_VAL(delay_oper->flags, L_OPER_SPILL_CODE)) return 0;

    /* Mark this instruction pair as a filled/filler. */
    sinfo->delay_sinfo = SCHED_INFO(delay_oper);
    oper->flags = L_SET_BIT_FLAG(oper->flags, FILLED_BELOW);
    delay_oper->flags = L_SET_BIT_FLAG(delay_oper->flags, FILLER);
    L_remove_oper(cb, delay_oper);

    return 1;
}

int Lsched_update_ctl_delay_sinfo(Squeue *queue, Sched_Info *sinfo)
{
    Sched_Info	*ctl_sinfo, *of_sinfo;
    Dep_Info 	*dep_info=sinfo->dep_info;
    L_Dep	*dep;

    /*
     * Conditions for a legal fill candidate:
     *
     * 1) current instruction is not a control operation.
     * 2) current instruction has only control dependence.
     * 3) control dependence is on a control instruction that has delay slots.
     * 4) the control instruction has not been filled with a spill
     *    instruction.
     * 5) the priority of this potential filler is lower than the
     *    current filler and therefore should be scheduled later.
     */
    if (IS_CTL(sinfo->proc_opc)) return 0;

    if (dep_info->n_output_dep != 1) return 0;

    dep = dep_info->output_dep;
    if (dep->type != L_DEP_CNT) return 0;

    ctl_sinfo = SCHED_INFO(dep->to_oper);
    if (!IS_CTL(ctl_sinfo->proc_opc)) return 0;
    if (!HAS_DELAY_SLOT(ctl_sinfo->proc_opc)) return 0;

    if (L_EXTRACT_BIT_VAL(ctl_sinfo->oper->flags, FILLED_BELOW)) return 0;

    if (!ctl_sinfo->delay_sinfo)
    {
        /* No delay slot filler has been found */
        if (Lsched_debug_nonsquashing_branches)
            fprintf (stdout, "> potential filler op %d found for control op %d\n", 
		sinfo->id, ctl_sinfo->id);

	L_dequeue (queue, sinfo);
	ctl_sinfo->delay_sinfo = sinfo;
	ctl_sinfo->num_depend--;
    }
    else 
    {   
	/* 
	 * In the event that there are  multiple candidates that can fill a delay slot,
         * we will chose the one with the lowest priority.  Since we are starting with
	 * the sorted ready queue, the highest priority instructions are encountered 
	 * first.  Thus, the last instruction that we encounter is the one that we 
	 * will choose to use for the filler. 
	 */
	of_sinfo = ctl_sinfo->delay_sinfo;

	if (sinfo->priority < of_sinfo->priority)
	{
            if (Lsched_debug_nonsquashing_branches)
                fprintf (stdout, "> op %d replaces op %d as potential filler for control op %d\n",
                    sinfo->id, of_sinfo->id, ctl_sinfo->id);
    
            /* place instruction on pending_ready queue. */
	    L_enqueue_min_to_max_1 (pending_ready_queue, of_sinfo, 
		(float) of_sinfo->ready_time);

	    /* Remember the new potential delay slot filler */
	    L_dequeue (queue, sinfo);
	    ctl_sinfo->delay_sinfo = sinfo;
	}
	else
	    return 0;
    }

        /* 
         * We wish to make sure that this instruction can issue as soon as possible
	 * since there are no other dependences on it.
	 */
    if (ctl_sinfo->ready_time < ctl_sinfo->delay_sinfo->ready_time)
    {
        ctl_sinfo->ready_time = ctl_sinfo->delay_sinfo->ready_time;
        ctl_sinfo->earliest_slot = 0;
    }

    /* 
     * We can place this control instruction into the ready list if there is only
     * one input dependence since that control dependent instruction will be placed
     * into its delay slot.
     */

    if (ctl_sinfo->num_depend==0)
    {
 
        /* 
	 * Place this instruction into the pending ready list since there are no other
	 * dependences.
	 */
	L_dequeue(not_ready_queue, ctl_sinfo);
	L_enqueue_min_to_max_1 (pending_ready_queue, ctl_sinfo, 
	    (float) ctl_sinfo->ready_time);
    } 

    return 1;
}

/* 
 * This routine scans the specified list for instructions which are potential
 * candidates to fill a control delay slot within the superblock.
 *
 * This routine assumes that it is illegal to place a control oper into the
 * delay slot of another control oper.  This routine also currently assumes
 * that no delay slot dependences are carried across subroutine calls.   This
 * eliminates the need to search the dependence graph to determine if there are
 * any dependences that reach the control oper via another dependent instruction.
 */
void Lsched_check_for_fill_candidates(Squeue *queue)
{
    Sched_Info	*sinfo;

    L_reset_queue_current(queue);

    while((sinfo = L_get_queue_next_entry(queue))!=NULL)
	Lsched_update_ctl_delay_sinfo(queue, sinfo);
}

void Lsched_schedule_with_delay_op(L_Cb *cb, Sched_Info *sinfo, int current_time, 
    int current_block, int issue_slot, int spec)
{
    int 	earliest_slot, sched_slot, sched_time, delay_spec, 
		old_earliest_slot, delay_silent;
    Sched_Info	*delay_sinfo;
    Dep_Info	*dep_info, *delay_dep_info; 

    if (!IS_CTL(sinfo->proc_opc) || (!HAS_DELAY_SLOT(sinfo->proc_opc))) 
    {
	if (sinfo->delay_sinfo)
	    L_punt("Lsched_schedule_with_delay_op: op %d can not have a delay op (%d)\n",
		sinfo->id, sinfo->delay_sinfo->id);

	Lsched_schedule_op(cb, sinfo, spec, current_time); 
    }
    else if (sinfo->delay_sinfo)
    {
	sinfo->scheduled = 1;

	delay_sinfo = sinfo->delay_sinfo;

	/* Determine when and where the delay slot instruction should be scheduled */
	earliest_slot = (issue_slot+1) % (Lsched_total_issue_slots);
	old_earliest_slot = delay_sinfo->earliest_slot;
	delay_sinfo->earliest_slot = earliest_slot;

	if (earliest_slot == (issue_slot+1))
	    sched_time = current_time;
	else
	    sched_time = current_time+1;

	if ( !((sched_time == current_time) && (earliest_slot < old_earliest_slot)) &&
	     (Lsched_can_schedule_op(cb, delay_sinfo, sched_time, current_block, 
	      &sched_slot, &delay_spec, &delay_silent)==CAN_SCHEDULE) )
	{
    	    if (Lsched_debug_nonsquashing_branches)  
	        fprintf (stdout, "> op %d delay slot filled by op %d\n", 
		    sinfo->id, delay_sinfo->id);

	    /* Allocate the resources for the filler instruction */
	    RU_schedule_op_at(delay_sinfo->ru_info, delay_sinfo->mdes_info, 
		delay_sinfo->operand_ready_times,
		sched_time, sched_slot, delay_silent);

	    /* remove dependence between branch and delay_oper */
	    dep_info = sinfo->dep_info;
	    delay_dep_info = delay_sinfo->dep_info;
	    delay_dep_info->output_dep = L_remove_dep(delay_dep_info->output_dep,
                &delay_dep_info->n_output_dep, L_DEP_CNT, delay_sinfo->oper, 
		sinfo->oper);
    	    dep_info->input_dep = L_remove_dep(dep_info->input_dep,
                &dep_info->n_input_dep, L_DEP_CNT, delay_sinfo->oper, sinfo->oper);

	    /* 
	     * Place the instructions in the scheduled list and mark
	     * them as filled/filler.  Note:  we insert speculative
	     * instructions below the branch and non-speculative instructions
	     * above the branch to ensure correct data flow dependence calculations.
	     */
	    if (delay_spec)
	    {   
		/* Insert below control oper */
		delay_sinfo->current_block = current_block;
		delay_sinfo->issue_slot = sched_slot;
		sinfo->delay_sinfo = NULL;

		sinfo->oper->flags=L_SET_BIT_FLAG(sinfo->oper->flags, 
		    FILLED_BELOW);
		Lsched_schedule_op(cb, sinfo, spec, current_time); 

		delay_sinfo->oper->flags=L_SET_BIT_FLAG(delay_sinfo->oper->flags, 
		    FILLER);
		Lsched_schedule_op(cb, delay_sinfo, delay_spec, sched_time); 
	    }
	    else
	    {   
		/* Insert above the control oper  */
		delay_sinfo->current_block = current_block;
		delay_sinfo->issue_slot = sinfo->issue_slot;
		sinfo->issue_slot = sched_slot;
		sinfo->delay_sinfo = NULL;

		delay_sinfo->oper->flags=L_SET_BIT_FLAG(delay_sinfo->oper->flags, 
		    FILLER);
		Lsched_schedule_op(cb, delay_sinfo, delay_spec, current_time); 

		sinfo->oper->flags=L_SET_BIT_FLAG(sinfo->oper->flags, 
		    FILLED_ABOVE);
		Lsched_schedule_op(cb, sinfo, spec, sched_time); 
	    }
	}
	else
	{
	    /* Hold the instruction for later. */
    	    if (Lsched_debug_nonsquashing_branches)  
	        fprintf (stdout, "> Branch (%d) unscheduled since it is not possible to schedule with delay (%d)\n",
			sinfo->id, delay_sinfo->id);

	    /* Unschedule the branch and place it back in the not ready list.  */
	    sinfo->scheduled = 0;
	    RU_unschedule_op(sinfo->ru_info);
	    sinfo->delay_sinfo = NULL;
	    sinfo->num_depend++;
	    L_dequeue_from_all(sinfo);
	    L_enqueue(not_ready_queue, sinfo);

	    /* Put the old delay oper back on the ready list. */
	    delay_sinfo->earliest_slot = old_earliest_slot;
	    L_enqueue_min_to_max_1 (pending_ready_queue, delay_sinfo, 
		(float) delay_sinfo->ready_time);
	}
    }
    else
    {
	sinfo->scheduled = 1;

	/* Determine when and where the delay slot instruction should be scheduled */
	earliest_slot = (issue_slot+1) % (Lsched_total_issue_slots);
	if (earliest_slot == (issue_slot+1))
	    sched_time = current_time;
	else
	    sched_time = current_time+1;


	/* 
	 * Search for a ready instruction that may be placed into the delay 
	 * slot of the branch. 
	 */
	while ((delay_sinfo = L_get_queue_next_entry(priority_ready_queue))!=NULL)
	{
	    old_earliest_slot = delay_sinfo->earliest_slot;
	    delay_sinfo->earliest_slot = earliest_slot;

	    if (!((sched_time == current_time) && (earliest_slot < old_earliest_slot)) &&
	          (Lsched_can_schedule_op(cb, delay_sinfo, sched_time, current_block, 
		   &sched_slot, &delay_spec, &delay_silent)==CAN_SCHEDULE) )
	    {
    	        if (Lsched_debug_nonsquashing_branches)  
	            fprintf (stdout, "> op %d delay slot filled by op %d\n", sinfo->id, 
			delay_sinfo->id);

                RU_schedule_op_at(delay_sinfo->ru_info, delay_sinfo->mdes_info, 
		    delay_sinfo->operand_ready_times,
		    sched_time, sched_slot, delay_silent);

	        /* remove dependence between branch and delay instruction */
	        dep_info = sinfo->dep_info;
	        delay_dep_info = delay_sinfo->dep_info;
	        delay_dep_info->output_dep = L_remove_dep(delay_dep_info->output_dep,
                    &delay_dep_info->n_output_dep, L_DEP_CNT, delay_sinfo->oper, 
		    sinfo->oper);
    	        dep_info->input_dep = L_remove_dep(dep_info->input_dep,
                    &dep_info->n_input_dep, L_DEP_CNT, delay_sinfo->oper, sinfo->oper);

	        /* 
	         * Place the instructions in the scheduled list and mark them as 
		 * filled/filler.  
		 *
		 * Note:  we insert speculative instructions below the branch and 
		 * non-speculative instructions above the branch to ensure correct 
		 * data flow dependence calculations.
	         */
	        if (delay_spec)
	        {   
		    /* Insert below control oper */
		    delay_sinfo->current_block = current_block;
		    delay_sinfo->issue_slot = sched_slot;

		    sinfo->oper->flags=L_SET_BIT_FLAG(sinfo->oper->flags, 
			FILLED_BELOW);
		    Lsched_schedule_op(cb, sinfo, spec, current_time); 

		    delay_sinfo->oper->flags=L_SET_BIT_FLAG(delay_sinfo->oper->flags, 
			FILLER);
		    Lsched_schedule_op(cb, delay_sinfo, delay_spec, sched_time); 
		}
		else
		{   
		    /* Insert above the control oper  */
		    delay_sinfo->current_block = current_block;
		    delay_sinfo->issue_slot = sinfo->issue_slot;
		    sinfo->issue_slot = sched_slot;

		    delay_sinfo->oper->flags=L_SET_BIT_FLAG(delay_sinfo->oper->flags, 
			FILLER);
		    Lsched_schedule_op(cb, delay_sinfo, delay_spec, current_time); 

		    sinfo->oper->flags=L_SET_BIT_FLAG(sinfo->oper->flags, 
			FILLED_ABOVE);
		    Lsched_schedule_op(cb, sinfo, spec, sched_time); 
		}
		break;
	    }
	    else
	    {
		/* Hold the instruction for later. */
		delay_sinfo->earliest_slot = old_earliest_slot;
	    }
	}

	/* 
	 * If delay is NULL, we were not able to find a delay oper and therefore have not
	 * inserted the oper into the scheduled list. 
	 */
	if (!delay_sinfo)
	    Lsched_schedule_op(cb, sinfo, spec, current_time); 

    }
}
