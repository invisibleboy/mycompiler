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
 *  File:  l_mcb.c
 *
 *  Description:  Contains routines to insert MCB code into mcode during 
 *	 	  prepass scheduling.  Adapted from original mcb code 
 *		  written for the old lcode format by William Chen.
 *
 *  Creation Date :  July 1993
 *
 *  Author:  Dave Gallagher, William Chen
 *
 *  Revisions:
 *
 *      (C) Copyright 1993, Dave Gallagher, William Chen
 *      All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_mcb.h"

#define L_MAX_COPY_OPER 1000
#define L_MAX_CB_LOADS 1000
#define L_MAX_CB_STORES 1000
#define MIN_MCB_SPEEDUP 0.00
#define MAX_STORES_BYPASSED 9

/**********************************************************************
    GLOBAL VARIABLES 
**********************************************************************/

L_Alloc_Pool *Lmcb_list_pool = NULL;


/**********************************************************************
    EXTERN VARIABLES 
**********************************************************************/

extern int Lsched_num_opers;
extern L_Oper *L_find_next_branch(L_Cb *cb, L_Oper *oper);
extern int Lmcb_keep_checks;

/**********************************************************************
    FUNCTION DECLARATIONS
**********************************************************************/

static L_Oper *L_get_regs_load_def (L_Oper *oper, L_Operand *reg);
static int L_are_same_pointer_regs(L_Oper *op1,L_Operand *reg1,
				   L_Oper *op2,L_Operand *reg2);
static int L_check_equivalent_regs(L_Oper *op1,L_Operand *reg1,
				   L_Oper *op2,L_Operand *reg2);

static void L_check_flow_op(int dest_reg, L_Oper *begin_op, L_Oper *end_op);
static int L_get_reg_num(L_Operand *operand);
static void L_rename_src_reg(L_Cb *cb, int operand_num, L_Oper *oper, 
				L_Oper *end_op);
static void L_change_ext_operand (L_Oper *oper, int operand_num, 
				L_Operand *new_operand);
static void L_mcb_add_store_to_load_list(L_Oper *store, L_Oper *load);
static void L_add_check_ptr_to_oper (L_Oper *check, L_Oper *load);
static int L_is_mem_or_flow_dep(L_Dep *depend);
static L_Oper *L_mcb_insert_check (L_Oper *oper);
static void L_add_check_to_check_dep (L_Oper *prev_check, L_Oper *check);
#if 0
#ifndef OPENIMPACT_DISTRIBUTION
static int Lhppa_mcb_branch_opcode(int opc, int ext, int dir);
static void L_add_beq_proc_opc(L_Oper *oper);
#endif
#endif
static int L_is_corr_cb(L_Cb *cb);
static void L_insert_correction_code(L_Cb *cb);
static void L_insert_fall_thru_code(L_Cb *cb);
static void L_free_sched_info (L_Cb *cb);
static void L_mcb_delete_check (L_Cb *cb, L_Oper *check, int current_time);

/**********************************************************************
    FUNCTIONS
**********************************************************************/


void
L_change_beq_mcb(L_Cb *cb)
{

    L_Oper *oper, *opB;
    L_Attr *attr = NULL;

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {

        if (oper->opc != Lop_BEQ) continue;

            /* backward search for a corresponding mov_new */

        for (opB=oper->prev_op; opB!=NULL; opB=opB->prev_op) {

            if (opB->opc != Lop_MOV) continue;

	    if (L_find_attr(opB->attr,"SIMNOT") == NULL) continue;

            if (opB->dest[0]->value.r != oper->src[0]->value.r) continue;
            opB = opB->prev_op;
	    if (L_is_reg(opB->dest[0])) {
                attr = L_new_attr("MCB", 1);
		L_set_int_attr_field(attr, 0, opB->dest[0]->value.r);
	    }
	    else if (L_is_macro(opB->dest[0])) {
                attr = L_new_attr("MCB", 1);
		L_set_int_attr_field(attr, 0, -opB->dest[0]->value.mac);
	    }
            else
                L_punt("L_change_beq_mcb: load dest is not reg or mac",
			L_ERR_INTERNAL);
            attr->next_attr = oper->attr;
            oper->attr = attr;
            break;
        }
    }

}



static L_Oper *
L_get_regs_load_def (L_Oper *oper, L_Operand *reg)
{
    L_Oper *def;

    def = L_prev_def (reg,oper);

    if (def == NULL)
	return (NULL);

    if (L_general_load_opcode (def))
	return def;
    else
	return (NULL);

}
    


static int
L_are_same_pointer_regs(L_Oper *op1,L_Operand *reg1,L_Oper *op2,L_Operand *reg2)
{
    L_Oper *load1, *load2;

	/* first find reg1's prev definition and see if load */

    load1 = L_get_regs_load_def (op1, reg1);

    if (load1 == NULL)
	return 0;

	/* then find reg2's prev definition and see if load */

    load2 = L_get_regs_load_def (op2, reg2);

    if (load2 == NULL)
	return 0;

	/* finally compare operands */

    if ( (load1->opc == load2->opc) &&
	 (L_same_operand(load1->src[0],load2->src[0])) &&
	 (L_same_operand(load1->src[1],load2->src[1])) ) 

	return 1;

    else
	return 0;

}


static int
L_check_equivalent_regs(L_Oper *op1,L_Operand *reg1,L_Oper *op2,L_Operand *reg2)
{

    return (L_are_same_pointer_regs(op1,reg1,op2,reg2));

}



int
L_dependent_memory_ops (L_Cb *cb, L_Oper *op1, L_Oper *op2)
{

    L_Operand *base1, *base2, *offset1, *offset2;
    L_Operand *src1, *src2;
    int size1, size2, dependent;
    int stack1, stack2;

    if (! (L_general_load_opcode(op1) || L_general_store_opcode(op1)))
        L_punt("L_independent_memory_ops: operand isn't memory ref",
                        L_ERR_INTERNAL);
    if (! (L_general_load_opcode(op2) || L_general_store_opcode(op2)))
        L_punt("L_independent_memory_ops: operand isn't memory ref",
                        L_ERR_INTERNAL);

    src1 = op1->src[0];
    src2 = op1->src[1];
    if ( (L_is_macro(src2) &&
         ((src2->value.mac==L_MAC_SP)||(src2->value.mac==L_MAC_FP))) ||
          L_is_label(src2) || L_is_int_constant(src1) )  {
        base1 = src2;
        offset1 = src1;
    }
    else {
        base1 = src1;
        offset1 = src2;
    }

    src1 = op2->src[0];
    src2 = op2->src[1];
    if ( (L_is_macro(src2) &&
         ((src2->value.mac==L_MAC_SP)||(src2->value.mac==L_MAC_FP))) ||
          L_is_label(src2) || L_is_int_constant(src1) )  {
        base2 = src2;
        offset2 = src1;
    }
    else {
        base2 = src1;
        offset2 = src2;
    }

    size1 = L_memory_access_size(op1);
    size2 = L_memory_access_size(op2);

    stack1 = stack2 = 0;
    if (M_is_stack_operand(base1))
        stack1 = 1;
    if (M_is_stack_operand(base2))
        stack2 = 1;

    dependent = 0;

    /* Case 2 (cases refer to corresponding case in L_independent_memory_ops) */

    if (stack1 & stack2) {
	if ( L_same_operand(base1, base2)) {
            if (! L_is_int_constant(offset1))
                dependent = 0;
            else if (! L_is_int_constant(offset2))
                dependent = 0;
            else {
                dependent = ! L_no_overlap(offset1->value.i, size1,
                                           offset2->value.i, size2);
            }
	}
    }

    /* Case 4 */

    else if ((L_is_label(base1)) && (L_is_label(base2))) {
        if ( ! strcmp(base1->value.l, base2->value.l)) {
            if (! L_is_int_constant(offset1))
                dependent = 0;
            else if (! L_is_int_constant(offset2))
                dependent = 0;
            else {
                dependent = ! L_no_overlap(offset1->value.i, size1,
                                           offset2->value.i, size2);
            }
        }
    }

    /* Case 7 */

    else if ((L_is_reg(base1)) && (L_is_reg(base2))) {
        if (base1->value.r!=base2->value.r)
            dependent = 0;
	else if ( (L_is_reg(offset1)) &&
		  (L_is_reg(offset2))) {
            if (offset1->value.r==offset2->value.r)
                dependent = 1;
	    else
		dependent = 0;
	}
        else if (! L_is_int_constant(offset1))
            dependent = 0;
        else if (! L_is_int_constant(offset2))
            dependent = 0;
        /* T_same_def_reachs is only a local opti predicate ops must be
         * in same basic block to resolve this case
         */
        else if (! L_in_same_block(op1, op2))
            dependent = 0;
        else if (! L_same_def_reachs(base1, op1, op2))
            dependent = 0;
        else
            dependent = ! L_no_overlap(offset1->value.i, size1,
                                       offset2->value.i, size2);

	if (!dependent) {

	    if ( (L_is_int_constant(offset1)) &&
		 (L_is_int_constant(offset2)) &&
		 (offset1->value.i == offset2->value.i) ) {
		dependent = L_check_equivalent_regs (op1,base1,op2,base2);

	    }
	}


    }

    return (dependent);
}




L_Cb *
L_copy_all_oper_in_cb (L_Func *fn, L_Cb *cb)
{
    L_Oper *oper, *new_oper;
    L_Cb *new_cb;

    new_cb = L_new_cb (++(L_fn->max_cb_id));
    new_cb->weight = cb->weight;

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {
        new_oper = L_copy_operation(oper);
        L_insert_oper_after(new_cb, new_cb->last_op, new_oper);
	L_oper_hash_tbl_delete(fn->oper_hash_tbl, new_oper);
	new_oper->id = oper->id;
    }

    new_cb->src_flow = cb->src_flow;
    new_cb->dest_flow = cb->dest_flow;
    return (new_cb);
}

void
L_remove_sched_cb (L_Func *fn, L_Cb *cb)
{
    L_Oper *oper;
    L_Oper_List *list, *next_list;

    for (oper=cb->first_op; oper != NULL; oper = oper->next_op) {

	if (oper->ext != NULL) {
	    L_delete_ru_info_oper(SCHED_INFO(oper)->ru_info);

            L_free(Operand_ready_pool, SCHED_INFO(oper)->operand_ready_times);

            list = SCHED_INFO(oper)->store_list;
            while (list != NULL) {
	        next_list = list->next_list;
	        list->next_list = NULL;
	        list->oper = NULL;
	        L_free (Lmcb_list_pool,list);
	        list = next_list;
            }

            L_free(Sched_Info_pool, oper->ext);
	    oper->ext = NULL;
	}
    }
    L_delete_cb (fn,cb);
}

void
L_mcb_init()
{
    Lmcb_list_pool = 
	L_create_alloc_pool ("Store_List", sizeof (L_Oper_List),100);

}

#if 0
L_Oper_List *
L_new_oper_list()
{
    L_Oper_List *new_list;

    new_list = (L_Oper_List *) L_alloc(Lmcb_list_pool);

    new_list->oper = NULL;
    new_list->next_list = NULL;

    return (new_list);
}


void
L_free_oper_list(L_Oper *oper)
{
    L_Oper_List *list, *next_list;

    list = SCHED_INFO(oper)->store_list;

    while (list != NULL) {

	next_list = list->next_list;
	list->next_list = NULL;
	list->oper = NULL;
	L_free (Lmcb_list_pool,list);
	list = next_list;
    }
}
#endif

static void
L_free_oper_list_cb( L_Cb *cb)
{
    L_Oper *oper;
    L_Oper_List *list;

    for (oper = cb->first_op; oper != NULL; oper = oper->next_op) {

	if (SCHED_INFO(oper) == NULL)
	    continue;

	list = SCHED_INFO(oper)->store_list;

 	if (list != NULL)
	    L_delete_all_oper_list (list);
    }
}


static void
L_mcb_add_store_to_load_list(L_Oper *store, L_Oper *load)
{
    L_Oper_List *new_store;

    new_store = L_new_oper_list();
    new_store->oper = store;

	/* add to load's store_list */
    new_store->next_list = SCHED_INFO(load)->store_list;
    SCHED_INFO(load)->store_list = new_store;

}

static void
L_add_check_ptr_to_oper (L_Oper *check, L_Oper *load)
{
    SCHED_INFO(load)->check_op = check;
}


static int
L_is_mem_or_flow_dep(L_Dep *depend)
{
    switch (depend->type) {

	case L_DEP_MEM_FLOW:
	case L_DEP_MEM_ANTI:
	case L_DEP_MEM_OUTPUT:
	case L_DEP_CNT:
	    return (1);
	    break;
	default:
	    return (0);
    }
}


static L_Oper *
L_mcb_insert_check (L_Oper *oper)
{
    L_Oper *check_oper, *prev_br, *post_br;
    Sched_Info *sched_info;
    L_Dep *input, *output;
    int from, to, distance, home_block, old_num_dep;

	/* this function is responsible for creating a new check
	   instruction, which inherits all dependences from the incoming
	   load oper.  A register flow dependence is added from the load to
	   the check */

    home_block = SCHED_INFO(oper)->home_block;
    prev_br = SCHED_INFO(oper)->prev_br;
    post_br = SCHED_INFO(oper)->post_br;

    check_oper = L_create_new_op_using (Lop_CHECK,oper->parent_op);
    check_oper->src[0] = L_copy_operand (oper->dest[0]);
	/* don't want to allocate a cb to src 1 yet */
    check_oper->src[1] = NULL;

    check_oper->dep_info = (void *) L_new_dep_info();
    check_oper->weight = oper->weight;

    L_build_oper_mdes_info (check_oper);


	/* give check_oper all of oper's dependences */

    for (input = DEP_INFO(oper)->input_dep;input!=NULL;input=input->next_dep)
	L_add_dep (input->type,input->distance,input->from_index,
		   input->to_index, input->from_oper, check_oper, 0);

   for (output=DEP_INFO(oper)->output_dep;output!=NULL;output=output->next_dep){

	if (L_is_mem_or_flow_dep(output)) {

	    L_add_dep (output->type, 0,output->from_index,
	 	       output->to_index, check_oper, output->to_oper, 0);
	    SCHED_INFO(output->to_oper)->num_depend++;

	}

    }


	/* add reg_flow dependence from oper to check_oper */  
    from = operand_index(MDES_DEST,0);
    to = operand_index(MDES_SRC,0);
/*
    distance = max_operand_time(MDES_INFO(oper), from) -
	       min_operand_time(MDES_INFO(check_oper), to);
    if (distance < 0)
	distance = 0;
*/

    L_add_dep (L_DEP_REG_FLOW, 0, from,to,oper,check_oper, 0);
    {
	Dep_Info *ch_dep,*ld_dep;
	ch_dep = DEP_INFO(check_oper);
	ld_dep = DEP_INFO(oper);
    }

	/* add cnt_flow dependence to prev branch */

    if ((prev_br = SCHED_INFO(oper)->prev_br) != NULL) {
        from = operand_index(MDES_SYNC_OUT, DEP_CNT_OPERAND);
        to = operand_index(MDES_SYNC_IN, DEP_CNT_OPERAND);
        distance = (max_operand_time(MDES_INFO(prev_br), from)) -
	           (min_operand_time(MDES_INFO(check_oper), to));
        if (distance < 0)
	    distance = 0;
        L_add_dep (L_DEP_CNT, distance, from,to,prev_br,check_oper, 0);
    }

	/* add cnt_flow dependence to next branch */

    if ((post_br = SCHED_INFO(oper)->post_br) != NULL) {
        from = operand_index(MDES_SYNC_OUT, DEP_CNT_OPERAND);
        to = operand_index(MDES_SYNC_IN, DEP_CNT_OPERAND);
	distance = 0;
	old_num_dep = DEP_INFO(post_br)->n_input_dep;
        L_add_dep (L_DEP_CNT, distance, from,to,check_oper,post_br, 0);
	if (DEP_INFO(post_br)->n_input_dep > old_num_dep)
	    SCHED_INFO(post_br)->num_depend++;
    }

    check_oper->ext = (Sched_Info *)
        L_create_sched_info(check_oper, home_block, prev_br, post_br);

    sched_info = SCHED_INFO(check_oper);

    return (check_oper);
}


static void
L_add_check_to_check_dep (L_Oper *prev_check, L_Oper *check)
{
    int from, to, distance;

    if (prev_check == NULL)
	return;

    from = operand_index(MDES_SYNC_OUT, DEP_CNT_OPERAND);
    to = operand_index(MDES_SYNC_IN, DEP_CNT_OPERAND);
    distance = 0;
  
    L_add_dep (L_DEP_CNT, distance, from,to,prev_check,check, 0);

    SCHED_INFO(check)->num_depend++;

}



void 
L_mcb_insert_check_and_rem_dependences (L_Cb *cb)
{
    L_Oper *oper, *check_oper, *opB, *prev_check, *prev_op;
    Dep_Info *dinfo;
    int check_added;
    int store_count;
    int dep_flags = SET_NONLOOP_CARRIED(0);
    
    prev_check = NULL;

	/* this function is responsible for inserting check instructions
	   and removing load/store dependences.  All loads in the cb will
	   automatically have a check added following it, which inherits
	   all dependences of load, plus a dependence on the load.  The
	   function will then remove load/store dependences for all stores
	   ahead of the load in the current schedule. */

    for (oper = cb->first_op; oper != NULL; oper = oper->next_op) {
        if (L_general_load_opcode(oper)) {
	    check_added = 0;
	    store_count = 0;
		/* remove load/store dep */ 
	    for (opB = oper->prev_op; opB != NULL; opB = prev_op) {
		prev_op = opB->prev_op;
        	if (L_general_store_opcode(opB)) {
			/* exit loop once have bypassed max # of stores */
		    store_count++;
		    if (store_count > MAX_STORES_BYPASSED)
			prev_op = NULL;
		    if ( (!L_dependent_memory_ops (cb, opB, oper) ) &&
			 (!L_independent_memory_ops (cb,opB,oper,dep_flags))) {

			if (! check_added ) {

	        	    check_oper = L_mcb_insert_check (oper);
	        	    L_insert_oper_after(cb,oper,check_oper);
			    L_add_check_ptr_to_oper (check_oper,oper);

			    check_added = 1;
			}

			L_mcb_add_store_to_load_list(opB,oper);

			if ((dinfo = DEP_INFO(oper)) != NULL) {
			    dinfo->input_dep =
				L_remove_dep(dinfo->input_dep,
					     &dinfo->n_input_dep,
					     L_DEP_MEM_FLOW, opB, oper);
			    SCHED_INFO(oper)->num_depend--;
			}
			if ((dinfo = DEP_INFO(opB)) != NULL) {
			    dinfo->output_dep =
				L_remove_dep(dinfo->output_dep,
					     &dinfo->n_output_dep,
					     L_DEP_MEM_FLOW, opB, oper);
			}
		    }
		}
	    }
	}
    }

	/*to prevent checks from reordering, we add check-check dependence; this
	   is necessary to prevent a Ld R1, Ld R2, Ch R2, Ch R1 sequence. */

    for (oper = cb->first_op; oper != NULL; oper = oper->next_op) {
	if (oper->opc == Lop_CHECK) {
	    L_add_check_to_check_dep (prev_check, oper);
	    prev_check = oper;
	}
    }
/*
    L_print_dependence_graph (stdout, cb);
*/
}


#if 0
#ifndef OPENIMPACT_DISTRIBUTION
static int
Lhppa_mcb_branch_opcode(int opc, int ext, int dir)
{
    int hp_op;

    switch (opc)  {
        case Lop_BEQ_FS: case Lop_BEQ:
            hp_op = LHPPAop_COMB_EQ_FWD;
            break;
        case Lop_BNE_FS: case Lop_BNE:
            hp_op = LHPPAop_COMB_NE_FWD;
            break;
        case Lop_BGT_FS: case Lop_BGT:
            hp_op = LHPPAop_COMB_GT_FWD;
            break;
        case Lop_BGE_FS: case Lop_BGE:
            hp_op = LHPPAop_COMB_GE_FWD;
            break;
        case Lop_BLT_FS: case Lop_BLT:
            hp_op = LHPPAop_COMB_LT_FWD;
            break;
        case Lop_BLE_FS: case Lop_BLE:
            hp_op = LHPPAop_COMB_LE_FWD;
            break;
        case Lop_BGT_U_FS: case Lop_BGT_U:
            hp_op = LHPPAop_COMB_GT_U_FWD;
            break;
        case Lop_BGE_U_FS: case Lop_BGE_U:
            hp_op = LHPPAop_COMB_GE_U_FWD;
            break;
        case Lop_BLT_U_FS: case Lop_BLT_U:
            hp_op = LHPPAop_COMB_LT_U_FWD;
            break;
        case Lop_BLE_U_FS: case Lop_BLE_U:
            hp_op = LHPPAop_COMB_LE_U_FWD;
            break;
    }
    if ( ext )
        /* comparison with immediate operand */
        hp_op += 10;
    if ( dir == CBR_BACKWARD_EXT )
        hp_op += 20;

    return(hp_op);



}
#endif
#endif

#if 0
#ifndef OPENIMPACT_DISTRIBUTION
static void
L_add_beq_proc_opc(L_Oper *oper)
{
    int ext;

    ext = 0;

    if (L_is_int_constant(oper->src[0]) &&
             FIELD_5(oper->src[0]->value.i) )  {
            if ( oper->src[0]->value.i != 0 )
                ext = CBR_IMMED_EXT;
    }

    if ( ext ) L_set_attribute(oper,EXT(CBR_FORWARD_EXT,COMP_EQ,ext));

    oper->proc_opc = Lhppa_mcb_branch_opcode(oper->opc,ext,CBR_FORWARD_EXT);
}
#endif
#endif

static void
L_mcb_convert_sched_info(L_Oper *new_oper, L_Oper *old_oper)
{
    Sched_Info *sched_info;

    sched_info = (Sched_Info *) new_oper->ext;

    sched_info->oper = new_oper;
    sched_info->id = new_oper->id;
    sched_info->proc_opc = new_oper->proc_opc;
    sched_info->dep_info = new_oper->dep_info;
    sched_info->mdes_info = new_oper->mdes_info;
}



static int
L_is_corr_cb(L_Cb *cb)
{
    if (L_find_attr(cb->attr,"MCB_CORR_CB") == NULL)
        return 0;

    return 1;
}


static void
L_insert_correction_code(L_Cb *cb)
{
    L_Cb *new_cb, *correction_cb, *corr_cb;
    L_Oper *oper, *opB;
    int dest;
    L_Attr *attr, *corr_attr;
    int new_dest, src;

    for (oper=cb->first_op;oper!=NULL;oper=oper->next_op) {

	if (L_general_load_opcode(oper)) {
            if (SCHED_INFO(oper)->check_op) {
                L_Oper *new_oper;
                int flow_count;

                oper->attr = L_concat_attr (oper->attr, L_new_attr("MCB",0));

		    /* DMG - add data spec flag */
		oper->flags = L_SET_BIT_FLAG (oper->flags, L_OPER_DATA_SPECULATIVE);
                flow_count = 0;

		opB = SCHED_INFO(oper)->check_op;

		if (opB->opc != Lop_CHECK)
		    L_punt ("L_insert_correction_code: check inst not found");

		    /* Change check oper into BEQ */

                new_oper = L_create_new_op(Lop_BEQ);
                new_oper->src[1] = L_copy_operand(opB->src[0]);
                new_oper->src[0] = L_new_gen_int_operand(1);
                correction_cb = L_create_cb(0.0);
		correction_cb->attr = L_concat_attr (correction_cb->attr,
			L_new_attr("MCB_CORR_CB",0));
		new_oper->src[2] = L_new_cb_operand(correction_cb);

		if (L_is_reg(opB->src[0])) {
                    new_oper->attr = L_new_attr("MCB", 1);
		    L_set_int_attr_field(new_oper->attr, 0, 
					opB->src[0]->value.r);
	        }
		else if (L_is_macro(opB->src[0])) {
                    new_oper->attr = L_new_attr("MCB", 1);
		    L_set_int_attr_field(new_oper->attr, 0, 
					-opB->src[0]->value.mac);
	        }
                else
                    L_punt("L_insert_correction_code: check reg not reg/mac");

		attr = L_new_attr("check",1);
		L_set_int_attr_field(attr, 0, oper->id);
		new_oper->attr = L_concat_attr (new_oper->attr, attr);

#if 0
#ifndef OPENIMPACT_DISTRIBUTION
		L_add_beq_proc_opc(new_oper);
#endif
#endif
                L_insert_oper_after(cb, opB,new_oper);

		SCHED_INFO(oper)->check_op = new_oper;
		new_oper->ext = opB->ext;
		L_mcb_convert_sched_info(new_oper, opB);
		opB->ext = NULL;
		L_delete_oper(cb,opB);

                for (opB=new_oper->prev_op; opB!=NULL; opB=opB->prev_op) {
                    if (L_general_branch_opcode(opB)) flow_count++;
                }
                if (flow_count == 0) {
                    L_Flow *new_flow = L_new_flow(1,cb,correction_cb,0.0);
                    new_flow->next_flow = cb->dest_flow;
                    cb->dest_flow = new_flow;
                } else {
                    int n;
                    L_Flow *new_flow = L_new_flow(1,cb,correction_cb,0.0);
                    L_Flow *flow = cb->dest_flow;
                    for (n=0; n<(flow_count-1); n++) flow = flow->next_flow;
                    new_flow->next_flow = flow->next_flow;
                    flow->next_flow =  new_flow;
                    /* copy the flow arcs to correction cb */
                }

                /*
                 * insertion of correction code
                 * first mark flow dependent operations
                 */
                for (opB=oper->next_op;
		        (opB != NULL) && (opB!=SCHED_INFO(oper)->check_op);
		         opB=opB->next_op) 
		    opB->ambig_info = NULL;

		new_oper = L_copy_operation (oper);

		attr = L_new_attr("corr",1);
		L_set_int_attr_field(attr, 0, oper->id);
		new_oper->attr = L_concat_attr (new_oper->attr, attr);

                oper->ambig_info = new_oper;

		for (dest = 0; dest < L_max_dest_operand; dest++){
		    if (oper->dest[dest] != NULL) {
		        new_dest = L_get_reg_num (oper->dest[dest]);
                        L_check_flow_op(new_dest,oper,
					(SCHED_INFO(oper)->check_op));
		    }
		}

                     /* and then find anti dependent ops to insert a move */

                for (opB=oper;opB!=SCHED_INFO(oper)->check_op;opB=opB->next_op){

                    if (opB->ambig_info != NULL) {
			for (src = 0; src < L_max_src_operand; src++)
			    if (opB->src[src] != NULL)
                                L_rename_src_reg(cb,src,
						opB,SCHED_INFO(oper)->check_op);
		    }
                }

                /* now copy the ops in the correction cb */

                for (opB=oper;opB!=SCHED_INFO(oper)->check_op;opB=opB->next_op){

		    L_Attr *mcb_attr;

                    if (opB->ambig_info != NULL) { 
                        new_oper = opB->ambig_info;
                        if (opB == oper) {
			    mcb_attr = L_find_attr (new_oper->attr,"MCB");
			    if (mcb_attr != NULL)
			        new_oper->attr = 
				    L_delete_attr(new_oper->attr,mcb_attr);
			}
                        L_insert_oper_after(correction_cb,
				correction_cb->last_op,new_oper);
			oper->ambig_info = NULL;
		    }
                }

		L_insert_cb_after (L_fn, L_fn->last_cb, correction_cb);

		    /* the load could have been placed in some prev
			correction cb, prior to having its MCB attr 
			marked.  We need to search previous corr cbs
			and see if the current load is in them */

		for (corr_cb = correction_cb->prev_cb; L_is_corr_cb(corr_cb);
			corr_cb=corr_cb->prev_cb)  {

                    for (opB=corr_cb->first_op; opB!=NULL;opB=opB->next_op) {

                        corr_attr = L_find_attr(opB->attr,"corr");
                        if (corr_attr && 
			    (corr_attr->field[0]->value.i == oper->id) ) {

			    opB->attr = L_concat_attr(opB->attr,
					    L_new_attr("MCB",0));
                        }
                    }
		}

                    /* we make the correction_cb + 1 as the place 
			to go back to */

                new_cb = L_create_cb(0.0);
		new_cb->attr = L_concat_attr (new_cb->attr,
			L_new_attr("MCB_CORR_CB",0));

                correction_cb->dest_flow = 
		    L_new_flow(1,correction_cb,new_cb,0.0);

                new_oper = L_create_new_op(Lop_JUMP);
                new_oper->src[0] = L_new_cb_operand(new_cb);
                attr = L_new_attr("MCB",0);
                new_oper->attr = attr;
                L_insert_oper_after(correction_cb,
				    correction_cb->last_op,new_oper);
            }
	}
    }
}



static void
L_insert_fall_thru_code(L_Cb *cb)
{

    L_Oper *oper, *opB, *new_oper;
    int mark;
    L_Cb *first_linear_cb;
    L_Cb *dest_cb, *fall_cb = NULL;
    L_Attr *attr;

    mark = 0;

    L_color_cb(cb);

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op) {

	if ( (oper->opc == Lop_BEQ) &&
             (L_find_attr(oper->attr, "MCB") != NULL)) {

                /* we want the correction cb's # plus 1 */

            dest_cb = oper->src[2]->value.cb;
	    fall_cb = dest_cb->dest_flow->dst_cb;

            if (mark == 0) first_linear_cb = fall_cb;

            mark = 1;

	    L_insert_cb_after (L_fn, L_fn->last_cb, fall_cb);

            for (opB=oper->next_op; opB!=NULL; opB=opB->next_op) {

                L_Flow *flow_list = NULL;
		new_oper = L_copy_operation(opB);
		if (L_find_attr(new_oper->attr,"MCB") != NULL) {
		    attr = L_new_attr("corr",1);
                    L_set_int_attr_field(attr, 0, opB->id);
		    new_oper->attr = L_concat_attr (new_oper->attr, attr);
		}
                L_insert_oper_after(fall_cb,fall_cb->last_op, new_oper);

                if (L_general_branch_opcode(opB)) {

                    L_Flow *flow; 
                    L_Flow *cp_flow; 

		    flow = L_cnt_flow[L_find_control_op_index(opB)];
		    cp_flow = L_new_flow(flow->cc, 
					 fall_cb, flow->dst_cb, 0.0);

                    if (fall_cb->dest_flow == NULL) 
                        fall_cb->dest_flow = cp_flow;

		    else 
                        flow_list->next_flow = cp_flow;

                    flow_list = cp_flow;
                }

                if ((opB->opc == Lop_BEQ) &&
                    (L_find_attr(opB->attr, "MCB") != NULL)) {


                    if (opB != cb->last_op) {
                        L_Flow *flow;
			L_Cb *dest_cb, *new_fall_cb;

            		dest_cb = opB->src[2]->value.cb;
	    		new_fall_cb = dest_cb->dest_flow->dst_cb;

			flow = L_new_flow(0,fall_cb,new_fall_cb,0.0);

                        if (fall_cb->dest_flow == NULL) {
                            fall_cb->dest_flow = flow;
                        } else {
                            flow_list->next_flow = flow;
                        }
                    }
                    oper = opB->prev_op;
                    break;
                }
            }
	}
    }
    if (mark) {
        if (!(L_uncond_branch_opcode(cb->last_op) ||
            L_register_branch_opcode(cb->last_op))) {

            L_Flow *flow, *tmp;

            oper = L_create_new_op(Lop_JUMP);
	    oper->src[0] = L_new_cb_operand(cb->next_cb);

            L_insert_oper_after(fall_cb, fall_cb->last_op,oper);

            flow = L_new_flow(0,fall_cb,cb->next_cb,0.0);

            if (fall_cb->dest_flow == NULL) {
                fall_cb->dest_flow = flow;
            } 
	    else {
                for (tmp=fall_cb->dest_flow;tmp->next_flow!=NULL;
			tmp=tmp->next_flow);
                tmp->next_flow = flow;
            }
        }
    }
}

static void
L_check_flow_op(int dest_reg, L_Oper *begin_op, L_Oper *end_op)
{
    int new_dest;
    L_Oper *oper, *new_oper;
    L_Attr *attr;
    int src, dest, dependent;

    if (dest_reg == 0) return;

    for (oper=begin_op->next_op; oper!=end_op; oper=oper->next_op) {

	dependent = 0;

	for (src = 0; src < L_max_src_operand; src++) {
	    if (oper->src[src] != NULL) 
	        if (dest_reg == L_get_reg_num (oper->src[src]) )
		    dependent = 1;
	}

	if (dependent) {
	    if (oper->ambig_info == NULL) {
		new_oper = L_copy_operation (oper);
		attr = L_new_attr("corr",1);
                L_set_int_attr_field(attr, 0, oper->id);
		new_oper->attr = L_concat_attr (new_oper->attr, attr);
                oper->ambig_info = new_oper;
	    }
	    for (dest = 0; dest < L_max_dest_operand; dest++) 
		if (oper->dest[dest] != NULL) {
		    new_dest = L_get_reg_num (oper->dest[dest]);
            	    L_check_flow_op(new_dest,oper,end_op);
		}
	    oper->flags = L_SET_BIT_FLAG(oper->flags,L_OPER_DATA_SPECULATIVE);
	}

	    /* if the dest_reg is overwritten, no more opers are
	       dependent upon the previous definition of this register;
	       so return  */

	for (dest = 0; dest < L_max_dest_operand; dest++) {
	    if (oper->dest[dest] != NULL) 
	        if (dest_reg == L_get_reg_num (oper->dest[dest]) )
		    return;
	}
    }
}


/*
 * return negative number for macro
 * return positive number for register
 * return 0 for other
 */

static int 
L_get_reg_num(L_Operand *operand)
{
    switch(L_operand_case_type(operand)) {
    case L_OPERAND_MACRO:
        return(-(operand->value.mac));
        break;
    case L_OPERAND_REGISTER:
        if (operand->value.r == 0)
            L_punt("All register numbers should be > 0",-1);
        return(operand->value.r);
        break;
    default:
        return(0);
    }
}



static void
L_rename_src_reg(L_Cb *cb, int operand_num, L_Oper *oper, L_Oper *end_op)
{
    int dest, dest_reg, src_reg;
    L_Oper *op;
    L_Operand *operand;

    operand = oper->src[operand_num];

    if (!L_is_reg(operand))
	return;

    src_reg = L_get_reg_num(operand);

    if (src_reg != 0) {

        for (op=oper->next_op; op!=end_op; op=op->next_op) {

	    for (dest = 0; dest < L_max_dest_operand; dest++) {

		if (op->dest[dest] != NULL) {

                    dest_reg = L_get_reg_num(op->dest[dest]);

                    if (dest_reg == src_reg) {

                        L_Oper *prev_op = op->prev_op;

		            /* check if this reg has previously been renamed*/

                        if (prev_op->opc == Lop_MOV) {

                            int reg_num = L_get_reg_num(prev_op->src[0]);

                            if (reg_num != dest_reg) {

                                L_Oper *new_oper = L_create_new_op(Lop_MOV);
                                int new_tmp_reg = ++L_fn->max_reg_id;

                                new_oper->dest[0] = 
			    	    L_new_register_operand(new_tmp_reg,
				       L_return_old_ctype(op->dest[dest]),L_PTYPE_NULL);
                                new_oper->src[0] = 
			            L_new_register_operand(dest_reg,
				       L_return_old_ctype(op->dest[dest]),L_PTYPE_NULL);

                                L_insert_oper_before(cb,op,new_oper);

			        L_change_ext_operand (oper,operand_num,
				          L_copy_operand (new_oper->dest[0]));
                            } 
		            else {
			        L_change_ext_operand (oper,operand_num,
				          L_copy_operand (prev_op->dest[0]));
                            }
                        } 
		        else {
                            L_Oper *new_oper = L_create_new_op(Lop_MOV);
                            int new_tmp_reg = ++L_fn->max_reg_id;
    
                            new_oper->dest[0] = 
			        L_new_register_operand(new_tmp_reg,
					   L_return_old_ctype(op->dest[dest]),L_PTYPE_NULL);
                            new_oper->src[0] = 
			        L_new_register_operand(dest_reg,
					   L_return_old_ctype(op->dest[dest]),L_PTYPE_NULL);
                            L_insert_oper_before(cb,op,new_oper);

		            L_change_ext_operand (oper,operand_num,
			              L_copy_operand (new_oper->dest[0]));
                        }
                    }
		}
	    }
        }
    }
}



static void
L_change_ext_operand (L_Oper *oper, int operand_num, L_Operand *new_operand)
{
    L_Oper *corr_oper;
    L_Operand *old_operand;

	/* the ambig_info field of oper should contain the copy of oper which
	   will be placed in the correction cb  */

    corr_oper = (L_Oper *) oper->ambig_info;
    
    if (corr_oper == NULL)
	L_punt ("L_change_ext_operand: oper does not have ext field",
		 L_ERR_INTERNAL);

    old_operand = corr_oper->src[operand_num];
    corr_oper->src[operand_num] = new_operand;
    L_delete_operand (old_operand);

}

static void
L_free_sched_info (L_Cb *cb)
{
    L_Oper *oper;
    Sched_Info *sched_info;

    for (oper=cb->first_op; oper!=NULL; oper=oper->next_op)
    {
        if (oper->ext!=NULL)
        {
            sched_info = SCHED_INFO(oper);
 
            /* free allocated memory to pools */
            L_delete_ru_info_oper(sched_info->ru_info);
	    sched_info->ru_info = NULL;

	    sched_info->oper = NULL;
	    sched_info->dep_info = NULL;
	    sched_info->mdes_info = NULL;
	    sched_info->entry_list = NULL;

	    if (sched_info->branch_kill_set) 
		free(sched_info->branch_kill_set);

	    sched_info->delay_sinfo = NULL;
	    sched_info->prev_br = NULL;
	    sched_info->post_br = NULL;
	    sched_info->check_op = NULL;
	    sched_info->extend_lr_down = NULL;
	    sched_info->store_list = NULL;

            L_free(Operand_ready_pool, SCHED_INFO(oper)->operand_ready_times);
            L_free(Sched_Info_pool, oper->ext);

            /* Set the pointer back to null to prevent multiple freeing */
            oper->ext = NULL;
        }
    }
}


static void
L_mcb_convert_to_parent(L_Cb *cb)
{
    L_Oper *oper;

    for (oper = cb->first_op; oper != NULL; oper=oper->next_op) {
	if (oper->opc != Lop_CHECK)
	   oper->flags = L_SET_BIT_FLAG(oper->flags, L_OPER_PARENT);
    }
}



/******************************************************************************\
 *
 * Main entry points for prepass and post pass code scheduling
 *
\******************************************************************************/

#define MCB_MIN_CB_WEIGHT 1000.0

void
Lsched_mcb_schedule_block(L_Func *fn, L_Cb *cb, L_Cb *nomcb_cb) 
{
    L_Oper 	*oper, *prev_br, *post_br, *next_oper;
    int		home_block, is_branch;
    double old_cycles, new_cycles;

	    /* first decide whether this block should be MCB
		scheduled, based upon execution weight. */

    if ( (cb->weight < MCB_MIN_CB_WEIGHT) ||
         (cb->first_op == NULL) ) {
        L_free_cb_mdes_info (cb);
        L_delete_dependence_graph(cb);
        L_free_sched_info (cb);
	L_mcb_convert_to_parent(cb);
        L_delete_all_oper (cb->first_op, 1);
	cb->first_op = NULL;
	cb->last_op = NULL;
	cb->src_flow = NULL;
	cb->dest_flow = NULL;
	L_free(L_alloc_cb, cb);
        return;
    }

    if ( Lsched_debug_messages )
        fprintf(stdout, "prepass MCB scheduling cb %d\n", cb->id);

    old_cycles = L_approx_cb_time (nomcb_cb);

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
    for (oper=cb->first_op,home_block=0;oper!=NULL;oper=oper->next_op) {
        is_branch = Lsched_is_branch(oper->proc_opc);

        if (is_branch)
            post_br = L_find_next_branch(cb, oper);

        if ((oper->ext==NULL) &&(!IS_IGNORE(oper->proc_opc))) {
            Lsched_num_opers++;
	    oper->ext = (Sched_Info *)
		L_create_sched_info(oper,home_block,prev_br,post_br);
        }
	else if (oper->ext!=NULL) {
            Lsched_num_opers++;
	    SCHED_INFO(oper)->prev_br = prev_br;
	    SCHED_INFO(oper)->post_br = post_br;
	}

        if (is_branch) {
            home_block++;
            prev_br = oper;
        }
    }

    L_mcb_insert_check_and_rem_dependences (cb);

        /* Code scheduling */
    Lsched_schedule_block(cb);

        /* now choose whether to use new MCB schedule or old;
		only keep MCB if shorter schedule */

    new_cycles = L_approx_cb_time (cb);

    if ( ((old_cycles - new_cycles) / old_cycles) <= MIN_MCB_SPEEDUP ) {

	    /* we will use the non-MCB schedule, so we need to delete
		the MCB version of the cb */

	printf ("leaving cb %d in original order\n",nomcb_cb->id);
        L_free_cb_mdes_info (cb);
        L_delete_dependence_graph(cb);
	L_free_oper_list_cb(cb);
        L_free_sched_info (cb);
	    /* as a hack, we convert all opers to parents before deleting
		to prevent deletion from oper_hash_tbl.  (there are no
		corresponding oper_hash_tbl entries for these mcb opers) */
	L_mcb_convert_to_parent(cb);
        L_delete_all_oper (cb->first_op, 1);
	for (oper = nomcb_cb->first_op; oper != NULL; oper=next_oper) {
	    next_oper = oper->next_op;
	    L_oper_hash_tbl_update_cb(fn->oper_hash_tbl, oper->id, nomcb_cb);
	}
	cb->first_op = NULL;
	cb->last_op = NULL;
	cb->src_flow = NULL;
	cb->dest_flow = NULL;
	L_free(L_alloc_cb, cb);

    }
    else {
	    /* gonna use the MCB schedule; now need to add correction
	       code, tail duplication, and Nicolau-like code for trace */

	printf ("using mcb sched for cb %d\n",nomcb_cb->id);
	L_free_cb_mdes_info (nomcb_cb);
	L_delete_dependence_graph(nomcb_cb);
	L_free_sched_info (nomcb_cb);
	L_delete_all_oper (nomcb_cb->first_op, 1);
	nomcb_cb->first_op = NULL;
	nomcb_cb->last_op = NULL;
	for (oper = cb->first_op; oper != NULL; oper=next_oper) {
	    next_oper = oper->next_op;
	    if (SCHED_INFO(oper) != NULL)
	        L_delete_all_oper_list (SCHED_INFO(oper)->store_list);
	    if (oper->opc != Lop_CHECK)
	        L_oper_hash_tbl_insert(fn->oper_hash_tbl, oper);
	    L_insert_oper_after (nomcb_cb, nomcb_cb->last_op, oper);
	}
	cb->first_op = NULL;
	cb->last_op = NULL;
	cb->src_flow = NULL;
	cb->dest_flow = NULL;
	L_free(L_alloc_cb, cb);

	L_insert_correction_code (nomcb_cb);
	L_insert_fall_thru_code (nomcb_cb);


    }

}


static void
L_mcb_delete_check (L_Cb *cb, L_Oper *check, int current_time)
{

    Sched_Info  *sched_info;
    Dep_Info    *dep_info;
    L_Dep       *dep;

    sched_info = SCHED_INFO(check);
    dep_info = DEP_INFO(check);

    /* Remove check from its queued list */
    if (L_in_queue(scheduled_queue, sched_info) )
       L_punt("L_mcb_delete_check: Deleting check that has been scheduled");

    L_dequeue_from_all (sched_info);
/*
    Lsched_remove_list(sched_info->current_list, check);
*/
   
    /* Remove all input dependences */
    for (dep=dep_info->input_dep; dep!=NULL; dep=dep_info->input_dep)
    {
        Dep_Info        *from_dep_info=DEP_INFO(dep->from_oper);
        int             type=dep->type;

        dep_info->input_dep = L_remove_dep(dep_info->input_dep,
            &dep_info->n_input_dep, type, dep->from_oper, check);

        from_dep_info->output_dep = L_remove_dep(from_dep_info->output_dep,
            &from_dep_info->n_output_dep, type, dep->from_oper, check);
    }

    /*
     * make_ready will physically remove the dependence as well place 
	the instruction in the pending_ready list if the instruction 
	is ready to be scheduled.
     */
    sched_info->issue_slot=0;
    for (dep=dep_info->output_dep; dep!=NULL; dep=dep_info->output_dep)
    {
        L_Oper          *to_oper=dep->to_oper;
        Dep_Info        *to_dep_info=DEP_INFO(to_oper);
        Sched_Info      *to_sched_info=SCHED_INFO(to_oper);
        int             type=dep->type, prev_dep, num_dep,
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
            L_punt("L_mcb_delete_check: dependence not removed!");
        else
            if (num_dep>1)
                to_sched_info->num_depend-=(num_dep-1);

        /*
         * We must make sure that dependent instruction is correctly
         * moved to pending_ready list if there are no more
         * dependences.
         */
        Lsched_make_ready(cb, current_time, sched_info, to_sched_info, to_index,
            from_index, dep->distance);
    }

    /* Free up all allocated memory */
    L_delete_dependence_info(dep_info);
/*
    L_delete_ru_info_oper(sched_info->ru_info);
*/
    L_free_oper_mdes_info (check);
    L_delete_sched_info(sched_info);

    if (Lsched_debug_messages)
       fprintf (stdout, "> deleting check op %d\n", check->id);

    /* Delete the check instruction */
    L_delete_oper(cb, check);

}



void
L_mcb_remove_check (L_Cb *cb, L_Oper *oper, int ready_time)
{
    L_Oper_List *store_list;
    int found_bypass;

    if (!L_general_load_opcode(oper))
	return;

    if (SCHED_INFO(oper)->check_op == NULL)
	return;

    if (Lmcb_keep_checks)
	return;

        /* check if the load has bypassed any stores on its store_list */

    store_list = SCHED_INFO(oper)->store_list;

    found_bypass = 0;

    while ((store_list != NULL) && (! found_bypass)) {
/*
	if (SCHED_INFO(store_list->oper)->current_list != scheduled) {
*/
	if (! L_in_queue (scheduled_queue, SCHED_INFO(store_list->oper))) {

	        /* the store hasn't been scheduled yet, so load has bypassed
			it; need to keep check */
	    found_bypass = 1;
	}
	else
	    store_list = store_list->next_list;
    }

/*
    if (found_bypass) {
	oper->attr = L_concat_attr(oper->attr,L_new_attr("MCB",0));
    }
    else {
	L_mcb_delete_check (cb,SCHED_INFO(oper)->check_op,ready_time);
        SCHED_INFO(oper)->check_op = NULL;
    }
*/
    if (! found_bypass) {
	L_mcb_delete_check (cb,SCHED_INFO(oper)->check_op,ready_time);
        SCHED_INFO(oper)->check_op = NULL;
    }


}




