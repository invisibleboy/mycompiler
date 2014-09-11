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
/*===========================================================================
 *      File :          l_statistic.c
 *      Description :   Execution timing info.
 *      Creation Date : June, 1990
 *      Author :        Pohua Paul Chang.
 *	Modifications:
 *	Roger A. Bringmann, February 1993
 *	Modified to support new Lcode format.  Reduces memory requirements
 *	for a code generator.  Also, adds a more friendly interface for
 *	code generation.
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:04  david
 *  Import of IMPACT source
 *
 * Revision 1.1  1994/01/19  18:49:34  roger
 * Initial revision
 *
 *
 *      All rights granted to University of Illinois Board of Regents. 
 *==========================================================================*/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_schedule.h"

/*---------------------------------------------------------------------------*/
static double L_taken_weight(cb, oper)
L_Cb *cb;
L_Oper *oper;
{
    L_Flow *flow;

    if (op_flag_set(oper->proc_opc, OP_FLAG_JMP | OP_FLAG_RTS))
    {
	return (oper->weight);
    }
    else if (op_flag_set(oper->proc_opc, OP_FLAG_CBR))
    {
	flow = L_find_flow_for_branch (cb, oper);
	return flow->weight;
    }
    else
	return 0.0;
}
static double L_fall_thru_weight(cb, oper)
L_Cb *cb;
L_Oper *oper;
{
    if (op_flag_set(oper->proc_opc, OP_FLAG_JMP | OP_FLAG_CBR)) 
	return (oper->weight - L_taken_weight(cb, oper));
    else 
	return (oper->weight);
}

double L_approx_cb_time(L_Cb *cb)
{
    int 	k, issue, max_issue, complete, max_complete;
    L_Oper 	*oper;
    double 	best_cycle_count=0.0, weight;
    int 	*max_num_oper;
    double 	*max_oper_weight;

    if (cb->first_op==0) return 0.0;

    /* FIRST GET SOME STATIC INFORMATION.  */
    max_issue = 0;
    max_complete = 0;
    weight = cb->weight;

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
    {
	int opcode;
	oper->weight = 0.0;
	if (oper->ext==NULL) continue;
	oper->weight = weight;
	opcode = oper->opc;
	if (op_flag_set(oper->proc_opc, OP_FLAG_JMP | OP_FLAG_CBR)) 
	{
	    double taken_weight, fall_thru_weight = 0.0;
	    L_Flow *flow;
	    flow = L_find_flow_for_branch (cb, oper);
	    switch (opcode) 
	    {
		case Lop_JUMP:
		    taken_weight = flow->weight;
		    fall_thru_weight = weight - taken_weight;
		    break; 
		case Lop_JUMP_FS:
		    taken_weight = flow->weight;
		    fall_thru_weight = weight - taken_weight;
		    break; 
		case Lop_JUMP_RG:
		    taken_weight = weight;
		    fall_thru_weight = 0.0;
		    break;
		case Lop_JUMP_RG_FS:
		    taken_weight = weight;
		    fall_thru_weight = 0.0;
		    break;
	        case Lop_BR:
	        case Lop_BR_F:
		    taken_weight = flow->weight;
		    fall_thru_weight = weight - taken_weight;
		    break;
		default:
		    L_punt("L_approx_cb_time: illegal control oper %d",
			oper->id);
		    break;
	    }
	    weight = fall_thru_weight;
        }  
    }

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
    {
	if (oper->ext!=NULL)
	{
            issue = SCHED_INFO(oper)->issue_time;
            if (issue > max_issue) max_issue = issue;

   	    /* change in Sched_Info structure. BLD 6/95 */
            complete = Lsched_completion_time(oper);
            if (complete > max_complete) max_complete = complete;
	}
    }

    max_oper_weight = (double *) calloc((max_complete+1), sizeof (double));
    if (max_oper_weight==0)
        L_punt ("L_approx_cb_time: system out of memory on max_oper_weight!!\n");
    max_num_oper = (int *) calloc((max_complete+1), sizeof (int));
    if (max_num_oper==0)
        L_punt ("L_approx_cb_time: system out of memory on max_num_oper!!\n");

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
    {
	if (oper->ext!=NULL)
	{
            issue = SCHED_INFO(oper)->issue_time;
	    weight = oper->weight;
	    max_num_oper[issue] += 1;
	    if (weight > max_oper_weight[issue])
	        max_oper_weight[issue] = weight;
	}
    }

    for (k=0; k<=max_issue; k++) 
    {
	/*
	 *	there may be cycles that issue zero operation.
         *  because of the nature of trace
         *  scheduling, we simply look for the
         *  max. weight in subsequent instructions.
         */
        int m;
        for (m=k+1; m<=max_issue; m++) 
	{
            if (max_oper_weight[k] < max_oper_weight[m])
                max_oper_weight[k] = max_oper_weight[m];
        }
	best_cycle_count += max_oper_weight[k];
    }


    free (max_oper_weight);
    free (max_num_oper);

    return best_cycle_count;
}

/*---------------------------------------------------------------------------*/
/*
 * This function assumes that the schedular has been used to generate
 * issue times for instructions.  Also, dataflow and compute_oper_weight
 * should have been run.
 */
void 
L_print_schedule(char *F, L_Func *fn)
{
  int 	k;
  double	total_static_opers=0;
  L_Cb   	*cb;
  L_Oper 	*oper;
  FILE    	*stat_file;
  double 	br_t_t=0.0, br_t_n=0.0, br_n_t=0.0, br_n_n=0.0,
                jump_t_t=0.0, jump_n_t=0.0, jump_rg_t_t=0.0, jump_rg_n_t=0.0,
                jsr_t_t=0.0, jsr_n_t=0.0, rts_t_t=0.0, rts_n_t=0.0;
  /* 10/22/04 REK Commenting out unused variables. */
#if 0
  double        ld=0.0,st=0.0;
#endif
  double 	vliw_oper_count=0.0, superscalar_issue_count=0.0,
                best_cycle_count=0.0, worst_cycle_count=0.0;
  double 	br_add_penalty = 0.0;
  
  stat_file = fopen(F, "a");
  if (stat_file == 0)
    L_punt("L_print_schedule: Error while processing function %s:"
	   " cannot append to file %s\n", fn->name, F);

  /* Compute branch statistics.  */
  for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) 
    {
      double weight;
      weight = cb->weight;
      for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
	{
	  int opcode;
	  oper->weight = 0.0;

	  if (oper->ext==NULL) continue;

	  /* Don't count EPILOGUE compiler directives even if not
	   * marked as IGNORE by the mdes.  (Marking the epilogue 
	   * as IGNORE can mess up postpass scheduling when other code 
	   * is pulled into the same block).
	   * 
	   * This will make the scheduler statistics match the simulator 
	   * statistics for all machine descriptions.
	   * -JCG 2/3/98
	   */
	  if (oper->proc_opc == Lop_EPILOGUE) 
	    continue;

	  total_static_opers++;
	  oper->weight = weight;
	  opcode = oper->opc;
	  if (op_flag_set(oper->proc_opc, OP_FLAG_JMP | OP_FLAG_CBR)) 
	    {
	      double taken_weight, fall_thru_weight = 0.0;
	      L_Flow *flow;
	      flow = L_find_flow_for_branch (cb, oper);
	      switch (opcode) 
		{
		case Lop_JUMP_RG:
		case Lop_JUMP_RG_FS:
		  {
		    taken_weight = weight;
		    fall_thru_weight = 0.0;
		    if(taken_weight > 0)
		      {
			jump_rg_n_t += 1;
			jump_rg_t_t += taken_weight - 1;
		      }
		    break;
		  }
		case Lop_JUMP:
		case Lop_JUMP_FS:
		  {
		    taken_weight = flow->weight;
		    fall_thru_weight = weight - taken_weight;
		    
		    if(!oper->pred[0])
		      {
			if(taken_weight > 0)
			  {
			    jump_n_t += 1;
			    jump_t_t += taken_weight - 1;
			  }
		      }
		    else
		      {
			if(taken_weight > fall_thru_weight)
			  {
			    br_t_t += taken_weight;
			    br_t_n += fall_thru_weight;
			  }
			else
			  {
			    br_n_t += taken_weight;
			    br_n_n += fall_thru_weight;
			  }
		      }
		    break; 
		  }
		case Lop_BR:
		case Lop_BR_F:
		  {
		    taken_weight = flow->weight;
		    fall_thru_weight = weight - taken_weight;
		    
		    if(taken_weight > fall_thru_weight)
		      {
			br_t_t += taken_weight;
			br_t_n += fall_thru_weight;
		      }
		    else
		      {
			br_n_t += taken_weight;
			br_n_n += fall_thru_weight;
		      }
		    break;
		  }
		default:
		    L_punt("L_print_schedule: illegal control oper");
		    break;
		}
		weight = fall_thru_weight;
	    } 
	    else
	      if ((opcode>=Lop_JSR) & (opcode<=Lop_RTS_FS)) 
	      {
		switch (opcode) 
		  {
		  case Lop_JSR:
		  case Lop_JSR_FS:
		    {
		      if(weight > 0)
			{
			  jsr_n_t += 1;
			  jsr_t_t += weight - 1;
			}
		      break;
		    }
		  case Lop_RTS:
		  case Lop_RTS_FS:
		    {
		      if(weight > 0)
			{
			  rts_n_t += 1;
			  rts_t_t += weight - 1;
			}
		      break;
		    }
		default:
		    L_punt("L_print_schedule: illegal call/return oper");
		    break;
		}
	    }
	}
    }

    /* Print branch statistics */
    fprintf (stat_file, " ( CONTROL %s %s ) \n", fn->name, L_output_file);
    fprintf (stat_file, " ( br_t_t %.10f %s ) \n", br_t_t, fn->name);
    fprintf (stat_file, " ( br_t_n %.10f %s ) \n", br_t_n, fn->name);
    fprintf (stat_file, " ( br_n_t %.10f %s ) \n", br_n_t, fn->name);
    fprintf (stat_file, " ( br_n_n %.10f %s ) \n", br_n_n, fn->name);
    fprintf (stat_file, " ( jump_t_t %.10f %s ) \n", jump_t_t, fn->name);
    fprintf (stat_file, " ( jump_n_t %.10f %s ) \n", jump_n_t, fn->name);
    fprintf (stat_file, " ( jump_rg_t_t %.10f %s ) \n", jump_rg_t_t, fn->name);
    fprintf (stat_file, " ( jump_rg_n_t %.10f %s ) \n", jump_rg_n_t, fn->name);
    fprintf (stat_file, " ( jsr_t_t %.10f %s ) \n", jsr_t_t, fn->name);
    fprintf (stat_file, " ( jsr_n_t %.10f %s ) \n", jsr_n_t, fn->name);
    fprintf (stat_file, " ( rts_t_t %.10f %s ) \n", rts_t_t, fn->name);
    fprintf (stat_file, " ( rts_n_t %.10f %s ) \n", rts_n_t, fn->name);
    fprintf (stat_file, " ( static_opers %.10f %s ) \n", total_static_opers, fn->name);

    /*	Compute cycle count.  */
    for (cb=fn->first_cb; cb!=NULL; cb=cb->next_cb) 
    {
	int issue, max_issue, complete, max_complete;
	double weight, penalty;
	int *max_num_oper;
	double *max_oper_weight;

	L_Oper *ptr;
	int next_cycle, max_diff, cur_max, dest_max;

        if (cb->first_op==0)
            continue;

	/*
	 *  FIRST GET SOME STATIC INFORMATION.
	 */
        max_issue = 0;
	max_complete = 0;
        for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
	{
	    /* Always ignore EPILOGUE directive, see note above. -JCG 2/3/98 */
	    if ((oper->ext!=NULL) && (oper->proc_opc != Lop_EPILOGUE))
	    {
                issue = SCHED_INFO(oper)->issue_time;
                if (issue > max_issue) max_issue = issue;

		/* change in Sched_Info structure. BLD 6/95 */
                complete = Lsched_completion_time(oper);
                if (complete > max_complete) max_complete = complete;
	    }
        }

        max_oper_weight = (double *) calloc((max_complete+1), sizeof (double));
	if (max_oper_weight==0)
	    L_punt ("L_print_schedule: system out of memory on max_oper_weight!!\n");
        max_num_oper = (int *) calloc((max_complete+1), sizeof (int));
	if (max_num_oper==0)
	    L_punt ("L_print_schedule: system out of memory on max_num_oper!!\n");

        for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
	{
	    /* Always ignore EPILOGUE directive, see note above. -JCG 2/3/98 */
	    if ((oper->ext!=NULL) && (oper->proc_opc != Lop_EPILOGUE))
	    {
                issue = SCHED_INFO(oper)->issue_time;
	        weight = oper->weight;
	        superscalar_issue_count += weight;
	        max_num_oper[issue] += 1;
	        if (weight > max_oper_weight[issue])
		    max_oper_weight[issue] = weight;
	    }
 	}

        for (k=0; k<=max_issue; k++) 
	{
            /*
	     *	there may be cycles that issue zero operation.
             *  becasue of the nature of trace
             *  scheduling, we simply look for the
             *  max. weight in subsequent instructions.
             */
            int m;
            for (m=k+1; m<=max_issue; m++) 
	    {
                if (max_oper_weight[k] < max_oper_weight[m])
                    max_oper_weight[k] = max_oper_weight[m];
            }
	    vliw_oper_count += (max_num_oper[k] * max_oper_weight[k]);
	    best_cycle_count += max_oper_weight[k];
	    worst_cycle_count += max_oper_weight[k];
        }

	/*  CONSERVATIVELY ESTIMATE OPERATION LATENCY FOR  TAKEN BRANCHES.  */
	penalty = 0.0;
        for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) 
	{
	    if (op_flag_set(oper->proc_opc, OP_FLAG_JMP | OP_FLAG_CBR) &&
                (oper->ext!=NULL)) 
	    {
		/*
		 *  Here, we are been conservative and ignore
		 *  the additional delay due to branch misses.
		 */
		max_diff = 0;
		next_cycle = SCHED_INFO(oper)->issue_time + 1;
		for (ptr=oper->prev_op; ptr!=0; ptr=ptr->prev_op) 
		{
		    int ii, diff;

		    if (ptr->ext==NULL) continue;

		    /*
		     *	We care only if the destination variable
		     *	is in the OUT set.
		     */
	            issue = SCHED_INFO(ptr)->issue_time;
		    dest_max = issue;

		    for (ii=0; ii<L_max_dest_operand; ii++)
		    {
		        if ( (ptr->dest[ii]!=NULL) &&
		             (L_in_oper_OUT_set(cb, oper,ptr->dest[ii],TAKEN_PATH)))
		        {
#if 0
			    if (Lsched_infinite_issue)

			        cur_max = issue + mdes_operand_latency(ptr->proc_opc, 
			            mdes_heuristic_alt_id(ptr->proc_opc), 
				    operand_index(MDES_DEST, ii));
			    else
			        cur_max = issue + mdes_operand_latency(ptr->proc_opc, 
			            RU_SELECTED_ALT_ID(SCHED_INFO(ptr)->ru_info), 
				    operand_index(MDES_DEST, ii));
#endif
			        cur_max = issue + mdes_operand_latency(ptr->proc_opc, 
			            mdes_heuristic_alt_id(ptr->proc_opc), 
				    operand_index(MDES_DEST, ii));

			    if (cur_max > dest_max)
			        dest_max = cur_max;
		        }
		    }
		    diff = dest_max - next_cycle;

		    if (diff > max_diff)
			max_diff = diff;
		}
		penalty += L_taken_weight(cb, oper) * max_diff;
	    }
	    /*
	     *	We can ignore jsr/rts, because arguments are
	     *	assigned into macro registers, and the delay
	     *	has been accounted for.
	     */
	}

	/* CONSERVATIVELY ESTIMATE OPERATION LATENCY FOR THE FALL-THRU PATH. */
	oper = cb->last_op;
	/* Always ignore EPILOGUE directive, see note above. -JCG 2/3/98 */
        if (!op_flag_set(oper->proc_opc, OP_FLAG_JMP | OP_FLAG_RTS) &&
	    (oper->ext!=NULL) && (oper->proc_opc != Lop_EPILOGUE)) 
	{
	    max_diff = 0;
	    next_cycle = SCHED_INFO(oper)->issue_time + 1;
	    for (ptr=oper->prev_op; ptr!=NULL; ptr=ptr->prev_op) 
	    {
		int diff, ii;
		if (ptr->ext==NULL) continue;

		/*
		 *	We care only if the destination variable
		 *	is in the OUT set.
		 */
	        issue = SCHED_INFO(ptr)->issue_time;
		dest_max = issue;

		for (ii=0; ii<L_max_dest_operand; ii++)
		{
		    if ( (ptr->dest[ii]!=NULL) &&
		         (L_in_cb_OUT_set(cb, ptr->dest[ii])) )
		    {
#if 0
			if (Lsched_infinite_issue)
			    cur_max = issue + mdes_operand_latency(ptr->proc_opc, 
			        mdes_heuristic_alt_id(ptr->proc_opc), 
			        operand_index(MDES_DEST, ii));
			else
			    cur_max = issue + mdes_operand_latency(ptr->proc_opc, 
			        RU_SELECTED_ALT_ID(SCHED_INFO(ptr)->ru_info), 
				operand_index(MDES_DEST, ii));
#endif
			    cur_max = issue + mdes_operand_latency(ptr->proc_opc, 
			        mdes_heuristic_alt_id(ptr->proc_opc), 
			        operand_index(MDES_DEST, ii));

			if (cur_max > dest_max)
			    dest_max = cur_max;
		    }
		}
		diff = dest_max - next_cycle;

		if (diff > max_diff)
		    max_diff = diff;
	    }
	    penalty += L_fall_thru_weight(cb, oper) * max_diff;
	}
	worst_cycle_count += penalty;

	free (max_oper_weight);
	free (max_num_oper);
    }
    worst_cycle_count += br_add_penalty;	/* WYC 10-2-91 */

    /* Print cycle times */
    fprintf (stat_file, " ( TIME %s %s ) \n", fn->name, L_output_file);
    fprintf (stat_file, " ( vliw_oper_count %.10f %s ) \n", 
	vliw_oper_count, fn->name);
    fprintf (stat_file, " ( superscalar_issue_count %.10f %s ) \n",
        superscalar_issue_count, fn->name);
    fprintf (stat_file, " ( best_cycle_count %.10f %s ) \n", 
	best_cycle_count, fn->name);
    fprintf (stat_file, " ( worst_cycle_count %.10f %s ) \n", 
	worst_cycle_count, fn->name);

    fclose (stat_file);
}
