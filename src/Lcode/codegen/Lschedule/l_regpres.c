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
 *  File:  l_regpres.c
 *
 *  Description:  Support register pressure heuristic.
 *
 *  Creation Date :  February 1994
 *
 *  Author:  Roger A. Bringmann
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:03  david
 *  Import of IMPACT source
 *
 *
 *
 * 	All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/r_regproto.h>
#include "l_schedule.h"

int     	Lsched_use_register_pressure_heuristic = 0;
int     	Lsched_register_pressure_threshhold = 75 /* % */;
int	    	Lsched_regpres_heuristic = 0;
int		Lregpres_problem = 0;

int		num_virt_reg;
Vreg        	*virt_reg = NULL;
CB_REG_INFO     preg_info;
CB_REG_INFO     ireg_info;
CB_REG_INFO     freg_info;
CB_REG_INFO     dreg_info;

#define		CALLEE	0
#define		CALLER	1
/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void Lregpres_compute_benefit(Sched_Info *sinfo)
{
    sinfo->benefit = 
	(sinfo->kill_set.p + sinfo->kill_set.i + 
	 sinfo->kill_set.f + sinfo->kill_set.d) - 
        (sinfo->def_set.p + sinfo->def_set.i + 
	 sinfo->def_set.f + sinfo->def_set.d);
}

int Lregpres_inc_current (int ctype, int count)
{
    switch (ctype)
    {
	case L_CTYPE_INT:
	    preg_info.current_reg += (count * ireg_info.change.p);
	    ireg_info.current_reg += (count * ireg_info.change.i);
	    freg_info.current_reg += (count * ireg_info.change.f);
	    dreg_info.current_reg += (count * ireg_info.change.d);
	    break;
	
	case L_CTYPE_FLOAT:
	    preg_info.current_reg += (count * freg_info.change.p);
	    ireg_info.current_reg += (count * freg_info.change.i);
	    freg_info.current_reg += (count * freg_info.change.f);
	    dreg_info.current_reg += (count * freg_info.change.d);
	    break;
	
	case L_CTYPE_DOUBLE:
	    preg_info.current_reg += (count * dreg_info.change.p);
	    ireg_info.current_reg += (count * dreg_info.change.i);
	    freg_info.current_reg += (count * dreg_info.change.f);
	    dreg_info.current_reg += (count * dreg_info.change.d);
	    break;
	
	case L_CTYPE_PREDICATE:
	    preg_info.current_reg += (count * preg_info.change.p);
	    ireg_info.current_reg += (count * preg_info.change.i);
	    freg_info.current_reg += (count * preg_info.change.f);
	    dreg_info.current_reg += (count * preg_info.change.d);
	    break;

	default:
	    L_punt ("Lregpres_update_current: invalid ctype of %d\n", ctype);
    }	

    Lregpres_problem = ( (preg_info.current_reg > preg_info.threshhold) |
    			 (ireg_info.current_reg > ireg_info.threshhold) |
    			 (freg_info.current_reg > freg_info.threshhold) |
    			 (dreg_info.current_reg > dreg_info.threshhold) );

    return Lregpres_problem;
}

int Lregpres_dec_current (int ctype, int count)
{
    switch (ctype)
    {
	case L_CTYPE_INT:
	    preg_info.current_reg -= (count * ireg_info.change.p);
	    ireg_info.current_reg -= (count * ireg_info.change.i);
	    freg_info.current_reg -= (count * ireg_info.change.f);
	    dreg_info.current_reg -= (count * ireg_info.change.d);
	    break;
	
	case L_CTYPE_FLOAT:
	    preg_info.current_reg -= (count * freg_info.change.p);
	    ireg_info.current_reg -= (count * freg_info.change.i);
	    freg_info.current_reg -= (count * freg_info.change.f);
	    dreg_info.current_reg -= (count * freg_info.change.d);
	    break;
	
	case L_CTYPE_DOUBLE:
	    preg_info.current_reg -= (count * dreg_info.change.p);
	    ireg_info.current_reg -= (count * dreg_info.change.i);
	    freg_info.current_reg -= (count * dreg_info.change.f);
	    dreg_info.current_reg -= (count * dreg_info.change.d);
	    break;
	
	case L_CTYPE_PREDICATE:
	    preg_info.current_reg -= (count * preg_info.change.p);
	    ireg_info.current_reg -= (count * preg_info.change.i);
	    freg_info.current_reg -= (count * preg_info.change.f);
	    dreg_info.current_reg -= (count * preg_info.change.d);
	    break;

	default:
	    L_punt ("Lregpres_update_current: invalid ctype of %d\n", ctype);
    }	

    Lregpres_problem = ( (preg_info.current_reg > preg_info.threshhold) |
    			 (ireg_info.current_reg > ireg_info.threshhold) |
    			 (freg_info.current_reg > freg_info.threshhold) |
    			 (dreg_info.current_reg > dreg_info.threshhold) );

    return Lregpres_problem;
}

void Lregpres_inc_set(Regtypes *set, int ctype)
{
    switch (ctype)
    {
	case L_CTYPE_INT:
	    set->p += ireg_info.change.p;
	    set->i += ireg_info.change.i;
	    set->f += ireg_info.change.f;
	    set->d += ireg_info.change.d;
	    break;
	
	case L_CTYPE_FLOAT:
	    set->p += freg_info.change.p;
	    set->i += freg_info.change.i;
	    set->f += freg_info.change.f;
	    set->d += freg_info.change.d;
	    break;
	
	case L_CTYPE_DOUBLE:
	    set->p += dreg_info.change.p;
	    set->i += dreg_info.change.i;
	    set->f += dreg_info.change.f;
	    set->d += dreg_info.change.d;
	    break;
	
	case L_CTYPE_PREDICATE:
	    set->p += preg_info.change.p;
	    set->i += preg_info.change.i;
	    set->f += preg_info.change.f;
	    set->d += preg_info.change.d;
	    break;

	default:
	    L_punt ("Lregpres_inc_set: invalid ctype of %d\n", ctype);
    }	
}

void Lregpres_add_virt_reg_use (Sched_Info *sinfo, int reg_num)
{
    L_enqueue (virt_reg[reg_num].use_queue, sinfo); 
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void Lregpres_init_func(L_Func *fn)
{
    L_Cb	*cb;
    L_Oper	*oper;
    int 	i, reg_num;

    if (!Lsched_use_register_pressure_heuristic)
    {
	Lsched_regpres_heuristic = 0;
	return;
    }
    else
        Lsched_regpres_heuristic = 1;

    /*
     * Initialize information for register pressure heuristic
     *
     * Currently this is hardcoded for hp architecture.  This
     * will be changed as soon as mdes is modified to provide
     * this information.
     *
     * Given that x is the bank being initialized.
     *
     * if ( there is an overlap )
     *     [pifd]_change = sizeof(x)/sizeof([pifd]);
     * else
     *     [pifd]_change = 0.0;
     #
     */
    preg_info.caller_reg = (float) R_number_of_registers (L_CTYPE_PREDICATE, CALLER, FALSE);
    preg_info.caller_thresh = (float) ceil( (double) (preg_info.caller_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    preg_info.callee_reg = (float) R_number_of_registers (L_CTYPE_PREDICATE, CALLER, FALSE);
    preg_info.callee_thresh = (float) ceil( (double) (preg_info.callee_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    preg_info.size = (float) R_size_of_register (L_CTYPE_PREDICATE);

    ireg_info.caller_reg = (float) R_number_of_registers (L_CTYPE_INT, CALLER, TRUE);
    ireg_info.caller_thresh = (float) ceil( (double) (ireg_info.caller_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    ireg_info.callee_reg = (float) R_number_of_registers (L_CTYPE_INT, CALLEE, TRUE);
    ireg_info.callee_thresh = (float) ceil( (double) (ireg_info.callee_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    ireg_info.size = (float) R_size_of_register (L_CTYPE_INT);

    freg_info.caller_reg = (float) R_number_of_registers (L_CTYPE_FLOAT, CALLER, TRUE);
    freg_info.caller_thresh = (float) ceil( (double) (freg_info.caller_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    freg_info.callee_reg = (float) R_number_of_registers (L_CTYPE_FLOAT, CALLEE, TRUE);
    freg_info.callee_thresh = (float) ceil( (double) (ireg_info.callee_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    freg_info.size = (float) R_size_of_register (L_CTYPE_FLOAT);

    dreg_info.caller_reg = (float) R_number_of_registers (L_CTYPE_DOUBLE, CALLER, TRUE);
    dreg_info.caller_thresh = (float) ceil( (double) (dreg_info.caller_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    dreg_info.callee_reg = (float) R_number_of_registers (L_CTYPE_DOUBLE, CALLEE, TRUE);
    dreg_info.callee_thresh = (float) ceil( (double) (dreg_info.callee_reg *
        (float) Lsched_register_pressure_threshhold / 100.0) );
    dreg_info.size = (float) R_size_of_register (L_CTYPE_DOUBLE);


    /* Initialize predicate overlap sizes */
    preg_info.change.p = 1.0;		/* Always overlaps with itself */

    if (R_register_overlap (L_CTYPE_PREDICATE, L_CTYPE_INT))
        preg_info.change.i = preg_info.size / ireg_info.size;
    else
        preg_info.change.i = 0.0;

    if (R_register_overlap (L_CTYPE_PREDICATE, L_CTYPE_FLOAT))
        preg_info.change.f = preg_info.size / freg_info.size;
    else
        preg_info.change.f = 0.0;

    if (R_register_overlap (L_CTYPE_PREDICATE, L_CTYPE_DOUBLE))
        preg_info.change.d = preg_info.size / dreg_info.size;
    else
        preg_info.change.d = 0.0;

    /* Initialize integer overlap sizes */
    if (R_register_overlap (L_CTYPE_INT, L_CTYPE_PREDICATE))
        ireg_info.change.p = ireg_info.size / preg_info.size;
    else
        ireg_info.change.p = 0.0;

    ireg_info.change.i = 1.0;		/* Always overlaps with itself */

    if (R_register_overlap (L_CTYPE_INT, L_CTYPE_FLOAT))
        ireg_info.change.f = ireg_info.size / freg_info.size;
    else
        ireg_info.change.f = 0.0;

    if (R_register_overlap (L_CTYPE_INT, L_CTYPE_DOUBLE))
        ireg_info.change.d = ireg_info.size / dreg_info.size;
    else
        ireg_info.change.d = 0.0;

    /* Initialize float overlap sizes */
    if (R_register_overlap (L_CTYPE_FLOAT, L_CTYPE_PREDICATE))
        freg_info.change.p = freg_info.size / preg_info.size;
    else
        freg_info.change.p = 0.0;

    if (R_register_overlap (L_CTYPE_FLOAT, L_CTYPE_INT))
        freg_info.change.i = freg_info.size / ireg_info.size;
    else
        freg_info.change.i = 0.0;

    freg_info.change.f = 1.0;		/* Always overlaps with itself */

    if (R_register_overlap (L_CTYPE_FLOAT, L_CTYPE_DOUBLE))
        freg_info.change.d = freg_info.size / dreg_info.size;
    else
        freg_info.change.d = 0.0;

    /* Initialize double overlap sizes */
    if (R_register_overlap (L_CTYPE_DOUBLE, L_CTYPE_PREDICATE))
        dreg_info.change.p = dreg_info.size / preg_info.size;
    else
        dreg_info.change.p = 0.0;

    if (R_register_overlap (L_CTYPE_DOUBLE, L_CTYPE_INT))
        dreg_info.change.i = dreg_info.size / ireg_info.size;
    else
        dreg_info.change.i = 0.0;

    if (R_register_overlap (L_CTYPE_DOUBLE, L_CTYPE_FLOAT))
        dreg_info.change.f = dreg_info.size / freg_info.size;
    else
        dreg_info.change.f = 0.0;

    dreg_info.change.d = 1.0;		/* Always overlaps with itself */


    /* Initialize virtual registers for function */
    num_virt_reg = fn->max_reg_id;
    virt_reg = (Vreg *) calloc(num_virt_reg+1, sizeof (Vreg));

    for (cb = fn->first_cb; cb != NULL; cb=cb->next_cb)
    {
	for (oper = cb->first_op; oper != NULL; oper=oper->next_op)
	{
	    for (i=0; i < L_max_src_operand; i++)
	    {
	        if (L_is_register(oper->src[i])) 
	        {
		    reg_num = oper->src[i]->value.r;

		    if (virt_reg[reg_num].use_queue==NULL)
		    {
			virt_reg[reg_num].use_queue = 
			    L_create_queue("virt_reg", reg_num);
			virt_reg[reg_num].ctype = L_return_old_ctype(oper->src[i]);
		    }
		}
	    }

	    for (i=0; i < L_max_dest_operand; i++)
	    {
	        if (L_is_register(oper->dest[i])) 
	        {
		    reg_num = oper->dest[i]->value.r;

		    if (virt_reg[reg_num].use_queue==NULL)
		    {
			virt_reg[reg_num].use_queue = 
			    L_create_queue("virt_reg", reg_num);
			virt_reg[reg_num].ctype = L_return_old_ctype(oper->dest[i]);
		    }
		}
	    }
	}
    }
}

void Lregpres_deinit_func(L_Func *fn)
{
    int i;

    if (!Lsched_regpres_heuristic) return;

    for (i=0; i<num_virt_reg; i++)
    {
	if (virt_reg[i].use_queue != NULL)
	    L_delete_queue (virt_reg[i].use_queue);
    }

    free (virt_reg);

    Lsched_regpres_heuristic = 0;
}

void Lregpres_init_cb(L_Cb *cb)
{
    L_Oper	*oper;
    Squeue	*queue;
    Sched_Info	*sinfo, *kill_sinfo;
    int		i, *reg, reg_cnt, size, *buf, reg_num,
		jsr_found=0;
    Set		in_set, out_set, kill_set, taken_set, fall_thru_set;

    if (!Lsched_regpres_heuristic) return;

    Lregpres_problem = 0;
    preg_info.current_reg = 0.0;
    ireg_info.current_reg = 0.0;
    freg_info.current_reg = 0.0;
    dreg_info.current_reg = 0.0;

    for (i=0; i<num_virt_reg; i++)
	virt_reg[i].live_in_fall_thru_path = 0;

    /* 
     * Initialize register pressure for cb based upon the live-in of
     * the cb 
     */
    in_set = L_get_cb_IN_set(cb); 
    buf = (int *) malloc (sizeof(int) * Set_size(in_set));
    size = Set_2array (in_set, buf);
    for (i=0; i<size; i++)
    {
	if (L_IS_MAPPED_REG(buf[i]))
	    Lregpres_inc_current (virt_reg[L_UNMAP_REG(buf[i])].ctype, 1);
    }
    free(buf);

    /*
     * We mark all registers that are live in the fall-thru path
     * to prevent the register from being incorrectly killed.
     */
    if (!IS_UCOND_BRANCH(cb->last_op->proc_opc) ||
        (L_is_predicated(cb->last_op) && !L_EXTRACT_BIT_VAL(cb->flags,
                                                L_CB_HYPERBLOCK_NO_FALLTHRU)))
    {
        out_set = L_get_oper_OUT_set(cb, cb->last_op, FALL_THRU_PATH); 
        buf = (int *) malloc (sizeof(int) * Set_size(out_set));
        size = Set_2array (out_set, buf);
        for (i=0; i<size; i++)
	{
	    if (L_IS_MAPPED_REG(buf[i]))
	        virt_reg[L_UNMAP_REG(buf[i])].live_in_fall_thru_path = 1;
	}
        free(buf);
    }

    /*
     * Build use and def sets for all registers within the cb.
     */
    for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
	if (oper->ext==NULL) continue;

	sinfo = SCHED_INFO(oper);

	if (IS_JSR(sinfo->proc_opc)) jsr_found = 1;

	/* 
	 * Any register that is in the taken path of the branch but not in
	 * the fall-thru path of the branch is added to the virtual register
	 * count to support the dynamic kill sets.  We must also keep track
	 * of these registers so that we can appropriately mark any last
	 * use after a branch as the killing instruction.
	 */
	if (IS_BRANCH(sinfo->proc_opc))
	{
            taken_set = L_get_oper_OUT_set(cb, oper, TAKEN_PATH); 
	    size = Set_size(taken_set);
            fall_thru_set = L_get_oper_OUT_set(cb, oper, FALL_THRU_PATH); 
	    size = Set_size(fall_thru_set);
	    kill_set = Set_subtract(taken_set, fall_thru_set);
	    size = Set_size(kill_set);
	    buf = (int *) malloc (sizeof(int) * Set_size(kill_set));
	    size = Set_2array(kill_set, buf);

	    reg_cnt=0;
	    for (i=0; i<size; i++)
		if (L_IS_MAPPED_REG(buf[i])) reg_cnt++;

	    sinfo->branch_kill_set = reg =
		(int *) malloc (sizeof(int) * reg_cnt);
	    sinfo->branch_kill_set_size = reg_cnt; 

	    reg_cnt = 0;
	    for (i=0; i<size; i++)
	    {
		if (L_IS_MAPPED_REG(buf[i]))
		{
		    reg_num = L_UNMAP_REG(buf[i]);
		    reg[reg_cnt++] = reg_num;
		    Lregpres_add_virt_reg_use (sinfo, reg_num);
		}
	    }
	    free(buf);
	}

	/* Initialize virtual register to support dynamic kill set */
	for (i=0; i < L_max_src_operand; i++)
	{ 
	    if (L_is_register(oper->src[i])) 
	    {
		reg_num = oper->src[i]->value.r;
		Lregpres_add_virt_reg_use (sinfo, reg_num);
	    }
	}

	/* Initialize the set of registers defined by this instruction */
	for (i=0; i < L_max_dest_operand; i++)
	{
	    if (L_is_register(oper->dest[i]))
	    {
		Lregpres_inc_set(&sinfo->def_set, L_return_old_ctype(oper->dest[i]));
	    }
	}
    }

    /* Initialize the benefits for each instruction */
    for (oper = cb->first_op; oper!=NULL; oper=oper->next_op)
	if (oper->ext!=NULL)
	    Lregpres_compute_benefit(SCHED_INFO(oper));
     

    /* 
     * Mark any registers that are dead after only one use for 
     * appropriate initialization for the cb.
     */
    for (i=0; i<num_virt_reg; i++)
    {
	queue = virt_reg[i].use_queue;

	if (!queue) continue;

	if ((L_get_queue_size(queue) == 1) && !virt_reg[i].live_in_fall_thru_path)
	{
	    kill_sinfo = L_get_queue_head (queue);
	    Lregpres_inc_set(&kill_sinfo->kill_set, virt_reg[i].ctype);
	    Lregpres_compute_benefit(kill_sinfo);
	}
    }

    if (jsr_found)
    {
	preg_info.threshhold = preg_info.caller_thresh + preg_info.callee_thresh;
	ireg_info.threshhold = ireg_info.caller_thresh + ireg_info.callee_thresh;
	freg_info.threshhold = freg_info.caller_thresh + freg_info.callee_thresh;
	dreg_info.threshhold = dreg_info.caller_thresh + dreg_info.callee_thresh;
    }
    else
    {
	preg_info.threshhold = preg_info.caller_thresh;
	ireg_info.threshhold = ireg_info.caller_thresh;
	freg_info.threshhold = freg_info.caller_thresh;
	dreg_info.threshhold = dreg_info.caller_thresh;
    }
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void L_init_ready_queue()
{
    L_reset_queue_current(priority_ready_queue);

    if (Lsched_regpres_heuristic)
        L_reset_queue_current(regpres_ready_queue);
}

Sched_Info* L_get_next_entry()
{
    if (Lsched_regpres_heuristic && Lregpres_problem &&
	L_get_queue_size(regpres_ready_queue) /* &&
	((L_get_queue_head (regpres_ready_queue))->benefit >= 0.0) */)
	return L_get_queue_next_entry(regpres_ready_queue);
    else
	return L_get_queue_next_entry(priority_ready_queue);
}

void L_insert_entry(Sched_Info *sinfo)
{
    float key2=0.0;

    if (Lsched_regpres_heuristic)
	L_enqueue_regpres (regpres_ready_queue, sinfo, sinfo->benefit, 
	    sinfo->priority);

    if (Lsched_use_fan_out) key2 = (float)sinfo->dep_info->n_output_dep;

    L_enqueue_max_to_min_2 (priority_ready_queue, sinfo, sinfo->priority,
        key2); 
}

void L_update_virt_reg (Sched_Info *sinfo)
{
    Squeue	*queue;
    Sched_Info	*kill_sinfo;
    L_Oper	*oper;
    int 	i, reg;

    if (!Lsched_regpres_heuristic) return;

    /* Decrement the current register approximations  by the kill set */
    Lregpres_dec_current (L_CTYPE_PREDICATE, sinfo->kill_set.p);
    Lregpres_dec_current (L_CTYPE_INT, sinfo->kill_set.i);
    Lregpres_dec_current (L_CTYPE_FLOAT, sinfo->kill_set.f);
    Lregpres_dec_current (L_CTYPE_DOUBLE, sinfo->kill_set.d);

    /* Increment the current register approximations by the def set */
    Lregpres_inc_current (L_CTYPE_PREDICATE, sinfo->def_set.p);
    Lregpres_inc_current (L_CTYPE_INT, sinfo->def_set.i);
    Lregpres_inc_current (L_CTYPE_FLOAT, sinfo->def_set.f);
    Lregpres_inc_current (L_CTYPE_DOUBLE, sinfo->def_set.d);

    /* 
     * Update kill sets for instructions that may be the last use
     * of a register in the taken path of the branch.
     */
    if (IS_BRANCH(sinfo->proc_opc))
    {
	for (i=0; i<sinfo->branch_kill_set_size; i++)
	{
	    reg = sinfo->branch_kill_set[i];
	    queue = virt_reg[reg].use_queue;

	    L_dequeue(queue, sinfo);

	    if ((L_get_queue_size(queue) == 1) && !virt_reg[reg].live_in_fall_thru_path)
	    {
		kill_sinfo = L_get_queue_head (queue);
		Lregpres_inc_set(&kill_sinfo->kill_set, virt_reg[reg].ctype);
		Lregpres_compute_benefit(kill_sinfo);
		if (L_in_queue(regpres_ready_queue, kill_sinfo))
		{
		    L_dequeue(regpres_ready_queue, kill_sinfo);
		    L_enqueue_regpres (regpres_ready_queue, kill_sinfo,
			kill_sinfo->benefit, kill_sinfo->priority);
		}
	    }
	}
    }

    oper = sinfo->oper;

    for (i=0; i < L_max_src_operand; i++)
    { 
        if (L_is_register(oper->src[i])) 
	{
	    reg = oper->src[i]->value.r;
	    queue = virt_reg[reg].use_queue;

	    L_dequeue(queue, sinfo);

	    if ((L_get_queue_size(queue) == 1) && !virt_reg[reg].live_in_fall_thru_path)
	    {
		kill_sinfo = L_get_queue_head (queue);
		Lregpres_inc_set(&kill_sinfo->kill_set, virt_reg[reg].ctype);
		Lregpres_compute_benefit(kill_sinfo);
		if (L_in_queue(regpres_ready_queue, kill_sinfo))
		{
		    L_dequeue(regpres_ready_queue, kill_sinfo);
		    L_enqueue_regpres (regpres_ready_queue, kill_sinfo,
			kill_sinfo->benefit, kill_sinfo->priority);
		}
	    }
	}
    }
}
