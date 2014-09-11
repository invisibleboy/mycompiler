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
 *  File: l_ip_database.c
 *
 *  Description:  These routines support the function level data base
 *	used for interprocedural analysis
 *
 *  Creation Date :  August, 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *
 *  Copyright (c) 1994 Roger A. Bringmann , Wen-mei Hwu and The Board of
 *		  Trustees of the University of Illinois. 
 *		  All rights reserved.
 *
 *  The University of Illinois software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_interproc.h"

#undef DEBUG_UPDATE

/******************************************************************************\
 *
 * Local variable declaration
 *
\******************************************************************************/

Database ip_database;
Database *operand_database = &ip_database;
Memory	 *memory_database;

/******************************************************************************\
 *
 * Database values
 *
\******************************************************************************/

void L_db_add_return_value(int func_id, L_Operand *dest, int opc, 
    L_Operand *src0, L_Operand *src1, L_Operand *src2)
{
    Database_Entry	*entry;
    Reg			*reg;

    if ((entry = L_db_find_entry(func_id)) == NULL)
       L_punt("L_db_add_return_value: function does not exist in database");

    if (entry->return_reg==NULL)
	entry->return_reg = L_rf_new_reg(dest);

    reg = entry->return_reg;
    reg->first_value = L_v_add_value(reg->first_value, opc, 
	src0, src1, src2, -1);
}

void L_db_add_param_value(int func_id, int incoming_func_id, 
    int incoming_jsr_id, L_Operand *dest, int opc, L_Operand *src0,
    L_Operand *src1, L_Operand *src2)
{
    Database_Entry	*entry;
    Database_Callsite	*callsite;
    Reg			*reg;

    if (!L_is_macro(dest))
	return;

    if ((entry = L_db_find_entry(func_id)) == NULL)
       L_punt("L_db_add_param_value: function does not exist in database");

    callsite = L_db_find_callsite(entry, incoming_func_id, 
	incoming_jsr_id, NULL);
    if (callsite==NULL)
       L_punt("L_db_add_param_value: callsite does not exist in database");

    reg = L_db_find_param_reg(callsite, dest);
    if (reg==NULL)
    {
	L_db_add_param_reg(func_id, incoming_func_id,
    	    incoming_jsr_id, dest);
        reg = L_db_find_param_reg(callsite, dest);
    }

    reg->first_value = L_v_add_value(reg->first_value, opc,
	src0, src1, src2, -1);
}

void L_db_add_global_memory_value(int func_id, int incoming_func_id, 
    int incoming_jsr_id, L_Operand *src0, L_Operand *src1, int opc, 
    L_Operand *vsrc0, L_Operand *vsrc1, L_Operand *vsrc2)
{
    Database_Entry	*entry;
    Database_Callsite	*callsite;
    Memory_Cell		*cell;

    if ((entry = L_db_find_entry(func_id)) == NULL)
       L_punt("L_db_add_global_memory_value: function does not exist in database");

    if ((callsite = L_db_find_callsite(entry, incoming_func_id, 
	 incoming_jsr_id, NULL)) == NULL)
       L_punt("L_db_add_global_memory_value: callsite does not exist in database");

    cell = L_db_find_global_memory_cell(callsite, src0, src1);
    if (cell==NULL)
    {
	L_db_add_global_memory_cell(func_id, incoming_func_id,
    	    incoming_jsr_id, src0, src1);
        cell = L_db_find_global_memory_cell(callsite, src0, src1);
    }

    cell->first_value = L_v_add_value(cell->first_value, opc,
	vsrc0, vsrc1, vsrc2, -1);
}

/******************************************************************************\
 *
 * Database operands
 *
\******************************************************************************/

Reg *L_db_find_param_reg(Database_Callsite *callsite, 
    L_Operand *dest)
{
    Reg *reg;

    for (reg=callsite->first_param_reg; reg!=NULL; 
	 reg=reg->next_reg)
    {
	if (L_same_operand(reg->dest, dest))
	    return reg;
    }

    return NULL;
}

int L_db_param_reg_defined(Database_Callsite *callsite, 
    L_Operand *dest)
{
    Reg *reg;

    for (reg=callsite->first_param_reg; reg!=NULL; 
	 reg=reg->next_reg)
    {
	if (L_same_operand(reg->dest, dest))
	{
	    return 1;

#if 0
	    if (reg->first_value != NULL)
		return 1;
	    else
		return 0;
#endif
	}
    }
    return 0;
}

void L_db_add_param_reg(int func_id, int incoming_func_id,
    int incoming_jsr_id, L_Operand *dest)
{
    Database_Entry	*entry;
    Database_Callsite	*callsite;
    Reg			*reg;

    entry = L_db_find_entry(func_id);
    if (entry==NULL)
       L_punt("L_db_add_param_reg: function does not exist in database");

    callsite = L_db_find_callsite(entry, incoming_func_id, 
	incoming_jsr_id, NULL);
    if (callsite==NULL)
       L_punt("L_db_add_param_reg: callsite does not exist in database");

    reg = L_db_find_param_reg(callsite, dest);
    if (reg) return;

    /* We must create the register since it did not exist */
    reg = L_rf_new_reg(dest);
    reg->next_reg = callsite->first_param_reg;
    callsite->first_param_reg = reg;
}

Memory_Cell *L_db_find_global_memory_cell(Database_Callsite *callsite, 
    L_Operand *src0, L_Operand *src1)
{
    Memory_Cell *cell;

    for (cell=callsite->first_memory_cell; cell!=NULL; 
	 cell=cell->next_cell)
    {
	if (L_mem_same_address(cell->src0, cell->src1, src0, src1))
	    return cell;
    }

    return NULL;
}

int L_db_global_memory_cell_defined(Database_Callsite *callsite, 
    L_Operand *src0, L_Operand *src1)
{
    Memory_Cell *cell;

    for (cell=callsite->first_memory_cell; cell!=NULL; 
	 cell=cell->next_cell)
    {
	if (L_mem_same_address(cell->src0, cell->src1, src0, src1))
	{
	    return 1;
#if 0
	    if (cell->first_value != NULL)
	        return 1;
	    else
	        return 0;
#endif
	}
    }
    return 0;
}

void L_db_add_global_memory_cell(int func_id, int incoming_func_id,
    int incoming_jsr_id, L_Operand *src0, L_Operand *src1)
{
    Database_Entry	*entry;
    Database_Callsite	*callsite;
    Memory_Cell		*cell;

    entry = L_db_find_entry(func_id);
    if (entry==NULL)
       L_punt("L_db_add_global_memory_cell: function does not exist in database");

    callsite = L_db_find_callsite(entry, incoming_func_id, 
	incoming_jsr_id, NULL);

    if (callsite==NULL)
       L_punt("L_db_add_global_memory_cell: callsite does not exist in database");

    cell = L_db_find_global_memory_cell(callsite, src0, src1);
    if (cell) return;

    /* We must create the register since it did not exist */
    cell = L_mem_new_cell(src0, src1, RUNTIME_DEF);
    cell->next_cell = callsite->first_memory_cell;
    callsite->first_memory_cell = cell;
}

/******************************************************************************\
 *
 * Database callsites
 *
\******************************************************************************/

Database_Callsite *L_db_new_callsite(int incoming_func_id, 
    char *incoming_func_name, int incoming_jsr_id)
{
    Database_Callsite *new_callsite;

    new_callsite = L_alloc (L_alloc_db_callsite);
    new_callsite->func_id = incoming_func_id;
    new_callsite->func_name = incoming_func_name;

    new_callsite->jsr_id = incoming_jsr_id;

    new_callsite->resolved = 0;

    new_callsite->first_param_reg = NULL;
    new_callsite->first_memory_cell = NULL;
    new_callsite->next_callsite = NULL;

    return new_callsite;
}

Database_Callsite *L_db_find_callsite(Database_Entry *entry, 
    int incoming_func_id, int incoming_jsr_id, 
    Database_Callsite *last_callsite)
{
    Database_Callsite	*callsite, *first_callsite;

    if (last_callsite)
	first_callsite = last_callsite->next_callsite;
    else
	first_callsite = entry->first_callsite;

    for (callsite=first_callsite; callsite!=NULL;
	 callsite=callsite->next_callsite)
    {
	if ((incoming_func_id != -1) && 
	    (callsite->func_id!=incoming_func_id))
	    continue;

	if ((incoming_jsr_id == -1) ||
	    (callsite->jsr_id==incoming_jsr_id))
	    return callsite;
    }

    return NULL;
}

void L_db_add_callsite(int func_id, int incoming_func_id,
    char *incoming_func_name, int incoming_jsr_id)
{
    Database_Entry	*entry;
    Database_Callsite	*callsite;

    if ((incoming_func_id==-1) || (incoming_jsr_id==-1))
       L_punt("L_db_add_callsite: func_id and jsr_id must be legal");

    entry = L_db_find_entry(func_id);
    if (entry==NULL)
	L_punt("L_db_add_callsite: function does not exist in database");

    callsite = L_db_find_callsite(entry, incoming_func_id, 
	incoming_jsr_id, NULL);
    if (callsite) return;

    callsite = L_db_new_callsite(incoming_func_id, incoming_func_name,
	incoming_jsr_id);
    callsite->next_callsite = entry->first_callsite;
    entry->first_callsite = callsite;
    entry->num_callsites++;
}

/******************************************************************************\
 *
 * Database entries
 *
\******************************************************************************/

Database_Entry *L_db_new_entry(int func_id, char *func_name)
{
    Database_Entry *new_entry;

    new_entry = L_alloc (L_alloc_db_entry);
    new_entry->func_id = func_id;
    new_entry->func_name = func_name;

    new_entry->num_callsites = 0;
    new_entry->resolved_callsites = 0;
    new_entry->first_callsite = NULL;

    new_entry->return_reg = NULL;

    return new_entry;
}

Database_Entry *L_db_find_entry(int func_id)
{
    if (func_id >= operand_database->num_entry)
	return NULL;

    return operand_database->entry_array[func_id];
}

void L_database_add_entry(int func_id, char *func_name)
{
    int		i;
    Database_Entry **entry_array;

    if (func_id >= operand_database->num_entry)
    {
	entry_array = (Database_Entry**) calloc(func_id + DATABASE_GROWTH,
	    sizeof(Database_Entry*));

	for (i=0; i<operand_database->num_entry; i++)
	    entry_array[i] = operand_database->entry_array[i];

	operand_database->num_entry = func_id + DATABASE_GROWTH;

	if (operand_database->entry_array)
	    free(operand_database->entry_array);

	operand_database->entry_array = entry_array;
    }

    operand_database->entry_array[func_id] = L_db_new_entry(func_id, func_name);
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/
L_Oper *L_find_rts(L_Func *fn)
{
    L_Cb        *cb;
    L_Oper      *oper;

    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
        for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
        {
            if (L_subroutine_return_opcode(oper))
                return oper;
        }
    }

    L_punt ("L_find_rts: No subroutine return???");
    return NULL;
}

void L_db_update_return_reg(CG_Node *target, L_Operand *dest)
{
    L_Cb        	*cb;
    L_Oper      	*rts;
    Value    		*value;
    Reg			*reg, *next_reg;
    Resolved		*resolved;
    L_Func		*old_L_fn;

    old_L_fn = L_fn;

    L_cg_load_func(target);

    if (target->fn==NULL) 
    {
	L_fn = old_L_fn;
        return;
    }

    rts = L_find_rts(target->fn);

    if ((cb = L_oper_hash_tbl_find_cb(target->fn->oper_hash_tbl,
        rts->id)) == NULL)
        L_punt("illegal cb for rts op %d", rts->id);

#ifdef DEBUG_UPDATE
fprintf(stderr, "  RET: traversing to %s, rts_id=%d\n", 
    target->func_name, rts->id);
#endif

    resolved = L_resolve_unknown(target, cb, rts, 1, &dest, NULL);

    for (reg = resolved->first_reg; reg != NULL; reg = next_reg)
    {
        for (value = reg->first_value; value != NULL;
             value = value->next_value)
        {
            /* Update the interprocedural database */
            L_db_add_return_value (target->func_id, reg->dest, value->opc, 
		value->src0, value->src1, value->src2);
	}
    
	next_reg = reg->next_reg;
    }	

    L_fn = old_L_fn;

    L_rs_delete_resolved(resolved);
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void L_db_update_param_reg(Database_Entry *entry, CG_Node *cg_node,
    L_Operand *dest)
{
    CG_Arc      	*src_arc;
    CG_Node     	*src_node;
    L_Cb        	*cb;
    L_Oper      	*oper;
    Value    		*value;
    Reg			*reg, *next_reg;
    Resolved		*resolved;
    Database_Callsite	*callsite;
    L_Operand		*parameters[32];
    int			parameter_count;
    L_Func		*old_L_fn;

    for (src_arc = cg_node->first_src_arc; src_arc != NULL;
         src_arc = src_arc->next_src_arc)
    {

	/* See if we need to search again */

        src_node = src_arc->src_node;

	callsite = L_db_find_callsite(entry, src_node->func_id, 
	    src_arc->jsr_id, NULL);

	if (!L_db_param_reg_defined(callsite, dest))
	{

#ifdef DEBUG_UPDATE
fprintf(stderr, "  PARAM: traversing to %s, jsr_id=%d, arc_id=%d\n         from %s\n", 
    src_node->func_name, src_arc->jsr_id, src_arc->arc_id, cg_node->func_name);
#endif

	    old_L_fn = L_fn;
            L_cg_load_func(src_node);

            if ((cb = L_oper_hash_tbl_find_cb(src_node->fn->oper_hash_tbl,
                src_arc->jsr_id)) == NULL)
                L_punt("illegal cb for jsr op %d", src_arc->jsr_id);

            if ((oper = L_oper_hash_tbl_find_oper(src_node->fn->oper_hash_tbl,
                src_arc->jsr_id)) == NULL)
                L_punt("illegal cb for jsr op %d", src_arc->jsr_id);

	    parameter_count = 0;
	    parameters[parameter_count++] = dest;

	    L_db_add_param_reg(cg_node->func_id, src_arc->src_node->func_id,
    	        oper->id, dest);

            resolved = L_resolve_unknown(src_node, cb, oper, parameter_count, 
	        parameters, NULL);

	    for (reg = resolved->first_reg; reg != NULL; reg = next_reg)
	    {
                for (value = reg->first_value; value != NULL;
                     value = value->next_value)
                {
                    /* Update the interprocedural database */
                    L_db_add_param_value (cg_node->func_id, 
		        src_arc->src_node->func_id, oper->id, reg->dest, 
		        value->opc, value->src0, value->src1, value->src2);
	        }
    
	        next_reg = reg->next_reg;

	    }

	    L_fn = old_L_fn;

	    L_rs_delete_resolved(resolved);

#if 0
	    /* Free up memory */
	    for (i = 0; i<parameter_count; i++)
	        L_delete_operand(parameters[i]);
#endif
	}

    }	
}

void L_db_update_global_memory(Database_Entry *entry, CG_Node *cg_node, 
    L_Oper *load)
{
    CG_Arc      	*src_arc;
    CG_Node     	*src_node;
    L_Cb        	*cb;
    L_Oper      	*oper;
    Value    		*value;
    Database_Callsite	*callsite;
    Resolved		*resolved;
    Memory_Cell		*cell, *next_cell;
    L_Func		*old_L_fn;

    for (src_arc = cg_node->first_src_arc; src_arc != NULL;
         src_arc = src_arc->next_src_arc)
    {
        src_node = src_arc->src_node;

	/* See if we need to search again */
	callsite = L_db_find_callsite(entry, src_node->func_id, 
	    src_arc->jsr_id, NULL);

	/* Prevent unnecessary recursion */
	if (L_db_global_memory_cell_defined(callsite, 
	    load->src[0], load->src[1]))
	    continue;

	old_L_fn = L_fn;
        L_cg_load_func(src_node);

        if ((cb = L_oper_hash_tbl_find_cb(src_node->fn->oper_hash_tbl,
            src_arc->jsr_id)) == NULL)
            L_punt("illegal cb for jsr op %d", src_arc->jsr_id);

        if ((oper = L_oper_hash_tbl_find_oper(src_node->fn->oper_hash_tbl,
            src_arc->jsr_id)) == NULL)
            L_punt("illegal cb for jsr op %d", src_arc->jsr_id);


#ifdef DEBUG_UPDATE
fprintf(stderr, "  MEM: traversing to %s, jsr_id=%d, arc_id=%d\n       from %s\n", 
    src_node->func_name, src_arc->jsr_id, src_arc->arc_id, cg_node->func_name);

fprintf(stderr, "       load address: ");
L_print_operand(stderr, load->src[0], 0);
L_print_operand(stderr, load->src[1], 0);
fprintf(stderr, "\n");
#endif

	L_db_add_global_memory_cell(cg_node->func_id, 
	    src_arc->src_node->func_id, oper->id, load->src[0], load->src[1]);

        resolved = L_resolve_unknown(src_node, cb, oper, 0, NULL, load);

	for (cell = resolved->first_cell; cell != NULL; cell = next_cell)
	{
            for (value = cell->first_value; value != NULL;
                 value = value->next_value)
            {
                /* Update the interprocedural database */
                L_db_add_global_memory_value (cg_node->func_id, 
		    src_arc->src_node->func_id, oper->id, cell->src0, 
		    cell->src1, value->opc, value->src0, value->src1,
		    value->src2);
	    }

	    next_cell = cell->next_cell;

	}

	L_fn = old_L_fn;

	L_rs_delete_resolved(resolved);
    }	
}

/******************************************************************************\
 *
 * Entry points for inter-procedural analysis.
 *
\******************************************************************************/

void L_database_init ()
{
    operand_database->num_entry = 0;
    operand_database->entry_array = NULL;

    memory_database = L_mem_new_memory(PROGRAM);
}

Reg *L_db_query_return_reg(CG_Node *current, CG_Node *target, L_Operand *dest)
{
    Database_Entry	*target_entry;
    Reg			*reg, *new_reg;
    Value		*value;

    if (L_analysis_level != INTER_PROCEDURAL_ANALYSIS)
        return L_rf_new_reg(dest);

    if (!L_is_macro(dest))
	L_punt("L_db_query_return_reg: must be provided a macro operand");

    target_entry = L_db_find_entry(target->func_id);
    if (target_entry==NULL)
       L_punt("L_db_query_return_reg: function does not exist in database");

    /* Update the parameter registers in the database */
    L_db_update_return_reg(target, dest);

    new_reg = L_rf_new_reg(dest);

    reg = target_entry->return_reg;

    if (reg)
    {
        for (value = reg->first_value; value!=NULL; value = value->next_value)
        {
            new_reg->first_value = L_v_add_value(new_reg->first_value,
	        value->opc, value->src0, value->src1, value->src2, 
		value->level);
        }
    }

    return new_reg;
}

Reg *L_db_query_param_reg(CG_Node *cg_node, int incoming_func_id, 
    int incoming_jsr_id, L_Operand *dest)
{
    Database_Entry	*entry;
    Database_Callsite	*callsite, *last_callsite;
    Reg			*reg, *new_reg;
    Value		*value;

    if (L_analysis_level != INTER_PROCEDURAL_ANALYSIS)
        return L_rf_new_reg(dest);

    if (!L_is_macro(dest))
	L_punt("L_db_query_param_reg: must be provided a macro operand");

    entry = L_db_find_entry(cg_node->func_id);
    if (entry==NULL)
       L_punt("L_db_query_param_reg: function does not exist in database");

    /* Update the parameter registers in the database */
    L_db_update_param_reg(entry, cg_node, dest);

    new_reg = L_rf_new_reg(dest);

    /* 
     * Build the list of values that are associated with the desired 
     * parameter register.
     */
    last_callsite = NULL;
    for (callsite  = L_db_find_callsite(entry, incoming_func_id, 
		     incoming_jsr_id, last_callsite);
	 callsite != NULL;
         callsite  = L_db_find_callsite(entry, incoming_func_id, 
		     incoming_jsr_id, last_callsite))
    {
	last_callsite = callsite;

        if ((reg = L_db_find_param_reg(callsite, dest))==NULL)
	    continue;

	for (value = reg->first_value; value!=NULL; value = value->next_value)
	{
	    new_reg->first_value = L_v_add_value(new_reg->first_value,
		value->opc, value->src0, value->src1, value->src2, 
		value->level);
	}
    }

    return new_reg;
}

Memory_Cell *L_db_query_global_memory(CG_Node *cg_node, int incoming_func_id, 
    int incoming_jsr_id, L_Oper *load)
{
    Database_Entry	*entry;
    Database_Callsite	*callsite, *last_callsite;
    Memory_Cell		*cell, *new_cell;
    Value		*value;

    if (L_analysis_level != INTER_PROCEDURAL_ANALYSIS)
        return L_mem_new_cell(load->src[0], load->src[1], RUNTIME_DEF);

    /* Check incoming functions */
    entry = L_db_find_entry(cg_node->func_id);
    if (entry==NULL)
       L_punt("L_db_query_global_memory: function does not exist in database");

    /* Update the parameter registers in the database */
    L_db_update_global_memory(entry, cg_node, load);

    new_cell = L_mem_new_cell(load->src[0], load->src[1], RUNTIME_DEF);

    /* 
     * Build the list of values that are associated with the desired 
     * memory address.
     */
    last_callsite = NULL;
    for (callsite  = L_db_find_callsite(entry, incoming_func_id, 
		     incoming_jsr_id, last_callsite);
	 callsite != NULL;
         callsite  = L_db_find_callsite(entry, incoming_func_id, 
		     incoming_jsr_id, last_callsite))
    {
	last_callsite = callsite;

        if ((cell = L_db_find_global_memory_cell(callsite, load->src[0], 
	    load->src[1])) == NULL)
	    continue;

	for (value = cell->first_value; value!=NULL; value = value->next_value)
	{
	    new_cell->first_value = L_v_add_value(new_cell->first_value,
		value->opc, value->src0, value->src1, value->src2, 
		value->level);
	}
    }

    return new_cell;
}
