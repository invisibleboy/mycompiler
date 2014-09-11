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
 *  File: l_ip_memory.c
 *
 *  Description:  These routines support the pseudo memory space used
 *	to resolve unknown expressions.
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

Memory		*global_hash = NULL;
Memory		*global_memory = NULL;

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

void L_mem_load_data(L_Data *data)
{
    L_Operand	*src0, *src1, *vsrc0 = NULL, *vsrc1 = NULL;
    int		opc = 0;
    Memory_Cell	*cell;

    if (global_hash == NULL)
	global_hash = L_mem_new_memory(HASH);

    if (global_memory == NULL)
	global_memory = L_mem_new_memory(PROGRAM);

    switch (data->type)
    {
	case L_INPUT_GLOBAL:
	    if ((!strcmp(data->address->value.l, "_bcc_gen_fctn")) ||
	        (!strcmp(data->address->value.l, "_setcc_gen_fctn")))
	    {
		src0 = L_new_gen_label_operand(data->address->value.l);

    		cell = L_mem_add_hash(src0);

		/* Free up temporary operands used to create the data element */
		L_delete_operand(src0);
	    }
	    break;

	case L_INPUT_WB:
	case L_INPUT_WW:
	case L_INPUT_WI:
	case L_INPUT_WQ:
	case L_INPUT_WF:
	case L_INPUT_WF2:
	case L_INPUT_WS:
	    if (data->value->type == L_EXPR_LABEL)
	    {
		/* Create the address operands */
		if (data->address->type == L_EXPR_ADD)
		    src0 = L_new_gen_label_operand(data->address->A->value.l);
		else
		    src0 = L_new_gen_label_operand(data->address->value.l);

		/* Create the data operand */
		vsrc0 = L_new_gen_label_operand(
		    M_fn_name_from_label(data->value->value.l));

    		cell = L_mem_add_hash(src0);

    		cell->first_value = L_v_add_value(cell->first_value, Lop_MOV, 
		    vsrc0, NULL, NULL, -1); 

		/* Free up temporary operands used to create the data element */
		L_delete_operand(src0);
		L_delete_operand(vsrc0);
	    }
	    else
	    {
		/* Create the address operands */
		if (data->address->type == L_EXPR_ADD)
		{
		    src0 = L_new_gen_label_operand(data->address->A->value.l);
		    src1 = L_new_gen_int_operand(data->address->B->value.i);
		}
		else
		{
		    src0 = L_new_gen_label_operand(data->address->value.l);
		    src1 = L_new_gen_int_operand(0);
		}

		/* Create the data operand */
		switch(data->value->type)
		{
		    case L_EXPR_INT:
			opc = Lop_MOV;
			vsrc0 = L_new_gen_int_operand(data->value->value.i);
			vsrc1 = NULL;
			break;
		    case L_EXPR_FLOAT:
			opc = Lop_MOV;
			vsrc0 = L_new_float_operand(data->value->value.f);
			vsrc1 = NULL;
			break;
		    case L_EXPR_DOUBLE:
			opc = Lop_MOV;
			vsrc0 = L_new_double_operand(data->value->value.f2);
			vsrc1 = NULL;
			break;
		    case L_EXPR_STRING:
			opc = Lop_MOV;
			vsrc0 = L_new_gen_string_operand(data->value->value.s);
			vsrc1 = NULL;
			break;
		    case L_EXPR_ADD:
			opc = Lop_ADD;
		        vsrc0 = L_new_gen_label_operand(data->value->A->value.l);
		    	vsrc1 = L_new_gen_int_operand(data->value->B->value.i);
			break;
		    case L_EXPR_SUB:
			opc = Lop_SUB;
		        vsrc0 = L_new_gen_label_operand(data->value->A->value.l);
		    	vsrc1 = L_new_gen_int_operand(data->value->B->value.i);
			break;
		    default:
			L_punt("L_mem_load_data: unsupported data type of %d",
			    data->value->type);
		}

		cell = L_mem_add_cell(global_memory, src0, src1, LOAD_TIME_DEF);

    		cell->first_value = L_v_add_value(cell->first_value, opc, 
		    vsrc0, vsrc1, NULL, -1); 

		/* Free up temporary operands used to create the data element */
		L_delete_operand(src0);
		L_delete_operand(src1);
		L_delete_operand(vsrc0);
		L_delete_operand(vsrc1);
	    }
	    break;
    }

}

/*
 * This routine not only determines if the memory cell is
 * in the hash table, but will also move it to the head of
 * the hash table linked list.
 */
Memory_Cell *L_mem_find_hash(L_Operand *src0, L_Operand *src1)
{
    int		hash;
    Memory_Cell	*cell, *prev_cell;
    L_Operand	*base;

    if (L_is_label(src0))
        base = src0;
    else
        base = src1;

    hash = L_mem_compute_hash(global_hash, base, zero);

    prev_cell = NULL;
    for (cell = global_hash->hash_table[hash]; cell != NULL; 
	cell = cell->next_cell)
    {
	if (L_mem_same_address(cell->src0, cell->src1, base, NULL))
	{
	    /* 
	     * Move the cell to the start of the list - if there is
	     * no previous cell, then we are already at the start of the
	     * list.
	     */
	    if (prev_cell)
	    {
	        prev_cell->next_cell = cell->next_cell;
	        cell->next_cell = global_hash->hash_table[hash];
	        global_hash->hash_table[hash] = cell;
	    }

	    return cell;
	}
	prev_cell = cell;
    }

    return NULL;
}

Memory_Cell *L_mem_add_hash(L_Operand *base)
{
    Memory_Cell	*cell;
    int		hash;

    if (base == NULL)
	L_punt ("L_mem_add_memory_cell: src0 is null");

    /* See if the cell already exists */
    if ((cell = L_mem_find_hash(base, NULL)) != NULL)
	return cell;

    cell = L_mem_new_cell(base, NULL, RUNTIME_DEF);

    hash = L_mem_compute_hash(global_hash, base, zero);

    cell->next_cell = global_hash->hash_table[hash];

    global_hash->hash_table[hash] = cell;

    return cell;
}

void L_mem_delete_hash()
{
    int 	i;
    Memory_Cell	*cell, *next_cell;

    if (global_hash == NULL) return;

    for (i=0; i < global_hash->hash_mask+1; i++)
    {
	for (cell = global_hash->hash_table[i]; cell != NULL; cell = next_cell)
	{
	    next_cell = cell->next_cell;
	    L_mem_delete_cell(cell);
	}
    }

    L_free(L_alloc_memory, global_hash);
    global_hash = NULL;
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

Memory_Cell *L_mem_new_cell(L_Operand *src0, L_Operand *src1, int flag)
{
    Memory_Cell	*cell;

    cell = L_alloc(L_alloc_memory_cell);

    cell->flag = flag;
    cell->src0 = L_copy_operand(src0);
    cell->src1 = L_copy_operand(src1);

    cell->first_value = NULL;
    cell->next_cell = NULL;

    return cell;
}

void L_mem_delete_cell(Memory_Cell *cell)
{
    if (cell == NULL) return;

    L_v_delete_all_value(cell->first_value);

    L_delete_operand(cell->src0);
    L_delete_operand(cell->src1);

    L_free(L_alloc_memory_cell, cell);
}

int L_mem_compute_hash(Memory *memory, L_Operand *src0, L_Operand *src1)
{
    int hash, val0, val1;

    if (src0==NULL)
	val0=0;
    else
	val0=src0->value.i;

    if (src1==NULL)
	val1=0;
    else
	val1=src1->value.i;

    hash = (val0 + val1) & memory->hash_mask;

    return hash;
}

/*
 * This routine not only determines if the memory cell is
 * in the hash table, but will also move it to the head of
 * the hash table linked list.
 */
Memory_Cell *L_mem_find_cell(Memory *memory, L_Operand *src0, L_Operand *src1)
{
    int		hash;
    Memory_Cell	*cell, *prev_cell;

    hash = L_mem_compute_hash(memory, src0, src1);

    prev_cell = NULL;
    for (cell = memory->hash_table[hash]; cell != NULL; cell = cell->next_cell)
    {
	if (L_mem_same_address(cell->src0, cell->src1, src0, src1))
	{
	    /* 
	     * Move the cell to the start of the list - if there is
	     * no previous cell, then we are already at the start of the
	     * list.
	     */
	    if (prev_cell)
	    {
	        prev_cell->next_cell = cell->next_cell;
	        cell->next_cell = memory->hash_table[hash];
	        memory->hash_table[hash] = cell;
	    }

	    return cell;
	}
	prev_cell = cell;
    }

    return NULL;
}

Memory_Cell *L_mem_add_cell(Memory *memory, L_Operand *src0, 
    L_Operand *src1, int flag)
{
    Memory_Cell	*cell;
    int		hash;

    if (memory == NULL)
	L_punt ("L_mem_add_memory_cell: no memory space provided");

    if (src0 == NULL)
	L_punt ("L_mem_add_memory_cell: src0 is null");

    if (src1 == NULL)
	L_punt ("L_mem_add_memory_cell: src1 is null");

    /* See if the cell already exists */
    if ((cell = L_mem_find_cell(memory, src0, src1)) != NULL)
	return cell;

    cell = L_mem_new_cell(src0, src1, flag);

    hash = L_mem_compute_hash(memory, src0, src1);

    cell->next_cell = memory->hash_table[hash];

    memory->hash_table[hash] = cell;

    return cell;
}

void L_mem_define_cell(CG_Node *cg_node, L_Operand *src0, L_Operand *src1,
    int opc, L_Operand *vsrc0, L_Operand *vsrc1, L_Operand *vsrc2, int level)
{
    Memory	*memory;
    Memory_Cell *cell;

    if ((opc == Lop_MOV) && 
	((cell = (L_mem_find_hash(src0, src1)))!=NULL))
    {
	/* This is our best guess on what is a hashing entry */
    	cell->first_value = L_v_add_value(cell->first_value, Lop_MOV, 
	    vsrc0, NULL, NULL, level); 
    }
    else
    {
        if (L_mem_global_ref(src0, src1))
            memory = global_memory;

        else if (L_mem_local_ref(src0, src1))
	    memory = cg_node->local_refs;

        else /* Do not handle ptr definitions at the moment! */
            return;

        cell = L_mem_add_cell(memory, src0, src1, RUNTIME_DEF);
    
        cell->first_value = L_v_add_value(cell->first_value, opc, vsrc0, vsrc1, 
	    vsrc2, level);
    }
}

Memory_Cell *L_mem_get_cell(CG_Node *cg_node, L_Oper *load)
{
    Memory_Cell	*cell, *new_cell;
    Value	*first_value, *value, *last_value;
    L_Operand	*src0=load->src[0], *src1=load->src[1];

    new_cell = L_mem_new_cell(src0, src1, RUNTIME_DEF); 

    if (L_mem_global_ref(src0, src1))
    {
	/* 
	 * This is slightly risky - I am going to assume that any
	 * reference with the base of a symbol table is to a symbol
	 * table!
	 */
        if ((cell = L_mem_find_hash(src0, src1)) != NULL)
        {
            first_value = cell->first_value;
            last_value = NULL;

            for (value = L_v_get_value_same_level(first_value, last_value);
	         value != NULL;
	         value = L_v_get_value_same_level(first_value, last_value))
            {
	        new_cell->first_value = L_v_add_value(new_cell->first_value,
	            value->opc, value->src0, value->src1, value->src2,
	            value->level);
	        last_value = value;
            }

	    if (new_cell->first_value) 
		return new_cell;
        }

	/* Check global memory */
        if ((cell = L_mem_find_cell(global_memory, src0, src1)) != NULL)
        {
            first_value = cell->first_value;
            last_value = NULL;

            for (value = L_v_get_value_same_level(first_value, last_value);
	         value != NULL;
	         value = L_v_get_value_same_level(first_value, last_value))
            {
	        new_cell->first_value = L_v_add_value(new_cell->first_value,
	            value->opc, value->src0, value->src1, value->src2,
	            value->level);
	        last_value = value;
            }

	    if (new_cell->first_value) 
		return new_cell;
        }


	/* 
	 * Since we were unable to find the definition, we must not have
	 * encountered the definition in the graph traversal.  Thus,
	 * we will look for the definition intra-procedurally.
	 */
        if ((cell = L_db_query_global_memory(cg_node, -1, -1, load)) != NULL)
        {
            first_value = cell->first_value;
            last_value = NULL;

            for (value = L_v_get_value_same_level(first_value, last_value);
	         value != NULL;
	         value = L_v_get_value_same_level(first_value, last_value))
            {
	        new_cell->first_value = L_v_add_value(new_cell->first_value,
	            value->opc, value->src0, value->src1, value->src2,
	            value->level);
	        last_value = value;
            }
        }
    }
    else if (L_mem_local_ref(src0, src1))
    {
        if ((cell = L_mem_find_cell(cg_node->local_refs, src0, src1)) != NULL)
        {
            first_value = cell->first_value;
            last_value = NULL;

            for (value = L_v_get_value_same_level(first_value, last_value);
	         value != NULL;
	         value = L_v_get_value_same_level(first_value, last_value))
            {
	        new_cell->first_value = L_v_add_value(new_cell->first_value,
	            value->opc, value->src0, value->src1, value->src2,
	            value->level);
	        last_value = value;
            }
        }
    }

    /* Do not handle ptr references at the moment! */

    return new_cell;
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

Memory	*L_mem_new_memory(int use)
{
    int		size = 0, mask = 0;
    Memory 	*new_memory;

    new_memory = L_alloc(L_alloc_memory);

    switch (use)
    {
        case PROGRAM:
	    size = PROGRAM_HASH_TABLE_SIZE;
	    mask = PROGRAM_HASH_TABLE_MASK;
	    break;

	case FUNCTION:
	    size = FUNCTION_HASH_TABLE_SIZE;
	    mask = FUNCTION_HASH_TABLE_MASK;
	    break;
    
	case HASH:
	    size = HASH_HASH_TABLE_SIZE;
	    mask = HASH_HASH_TABLE_MASK;
	    break;

	default:
	    L_punt("L_mem_new_memory: illegal context");
    }

    new_memory->num_entries = 0;
    new_memory->hash_mask = mask;
    new_memory->hash_table =(Memory_Cell **)calloc(size, sizeof(Memory_Cell *));

    return new_memory;
}

void L_mem_delete_value_gt_level(Memory *memory, int level)
{
    int 	i;
    Memory_Cell	*cell, *next_cell;

    if (memory == NULL) return;

    for (i=0; i < memory->hash_mask+1; i++)
    {
	for (cell = memory->hash_table[i]; cell != NULL; cell = next_cell)
	{
	    next_cell = cell->next_cell;

    	    cell->first_value = L_v_delete_value_gt_level(cell->first_value, 
		level);

	    if ((cell->first_value == NULL) && (cell->flag != LOAD_TIME_DEF))
		L_mem_delete_cell(cell);
	}
    }
}

void L_mem_reset(CG_Node *cg_node)
{
    L_mem_delete_value_gt_level(cg_node->local_refs, -1);
    L_mem_delete_value_gt_level(cg_node->ptr_refs, -1);
}

void L_mem_delete(Memory *memory)
{
    int 	i;
    Memory_Cell	*cell, *next_cell;

    if (memory == NULL) return;

    for (i=0; i < memory->hash_mask+1; i++)
    {
	for (cell = memory->hash_table[i]; cell != NULL; cell = next_cell)
	{
	    next_cell = cell->next_cell;
	    L_mem_delete_cell(cell);
	}
    }

    L_free(L_alloc_memory, memory);
}

/******************************************************************************\
 *
 *
 *
\******************************************************************************/

int L_mem_local_ref(L_Operand *src0, L_Operand *src1)
{
    if (!(L_is_macro(src0) && src0->value.mac == L_MAC_LV) &&
        !(L_is_macro(src1) && src1->value.mac == L_MAC_LV)) 
        return 0;
    else
        return 1;
}

int L_mem_local_store(L_Oper *oper)
{
    if (!L_general_store_opcode(oper))
	return 0;

    return L_mem_local_ref(oper->src[0], oper->src[1]);
}

int L_mem_local_load(L_Oper *oper)
{
    if (!L_general_load_opcode(oper))
	return 0;

    return L_mem_local_ref(oper->src[0], oper->src[1]);
}

int L_mem_global_ref(L_Operand *src0, L_Operand *src1)
{
    if ((L_is_label(src0) || L_is_label(src1)) &&
        (L_is_int_constant(src1) || L_is_int_constant(src0) ||
         src1==NULL || src0==NULL))
	return 1;
    else
	return 0;
}

int L_mem_same_address(L_Operand *op1src0, L_Operand *op1src1,
    L_Operand *op2src0, L_Operand *op2src1)
{
    if ((L_same_operand(op1src0, op2src0) &&
         L_same_operand(op1src1, op2src1)) ||
        (L_same_operand(op1src0, op2src1) &&
         L_same_operand(op1src1, op2src0)))
        return 1;
    else
        return 0;
}

int L_mem_same_global_address(L_Operand *op1src0, L_Operand *op1src1,
    L_Operand *op2src0, L_Operand *op2src1)
{

    if (!L_mem_global_ref(op1src0, op1src1)) return 0;

    if (!L_mem_global_ref(op2src0, op2src1)) return 0;

    return L_mem_same_address(op1src0, op1src1, op2src0, op2src1);
}

int L_mem_global_store(L_Oper *oper)
{
    if (!L_general_store_opcode(oper))
	return 0;

    return L_mem_global_ref(oper->src[0], oper->src[1]);
}

int L_mem_global_load(L_Oper *oper)
{
    if (!L_general_load_opcode(oper))
	return 0;

    return L_mem_global_ref(oper->src[0], oper->src[1]);
}

int L_mem_ptr_ref(L_Operand *src0, L_Operand *src1)
{
    return !(L_mem_local_ref(src0, src1) | 
	     L_mem_global_ref(src0, src1));
}

int L_mem_ptr_store(L_Oper *oper)
{
    if (!L_general_store_opcode(oper))
	return 0;

    return L_mem_ptr_ref(oper->src[0], oper->src[1]);
}

int L_mem_ptr_load(L_Oper *oper)
{
    if (!L_general_load_opcode(oper))
	return 0;

    return L_mem_ptr_ref(oper->src[0], oper->src[1]);
}
