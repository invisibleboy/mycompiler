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
 *  File: l_ip_regfile.c
 *
 *  Description:  These routines support the pseudo register file used
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

/******************************************************************************\
 *
 * Register routines
 *
\******************************************************************************/

Reg *L_rf_new_reg(L_Operand *dest)
{
    Reg *reg;
    
    reg = L_alloc (L_alloc_reg);

    reg->dest = L_copy_operand(dest);
    reg->first_value = NULL;
    reg->next_reg = NULL;

    return reg;
}

void L_rf_delete_reg(Reg *reg)
{
    if (reg == NULL) return;

    L_v_delete_all_value(reg->first_value);

    L_delete_operand(reg->dest);

    L_free(L_alloc_reg, reg);
}

Reg *L_rf_delete_reg_gt_level(Reg *reg, int level)
{
    if (reg == NULL) return NULL;

    reg->first_value = 
	L_v_delete_value_gt_level(reg->first_value, level);
    
    if (reg->first_value == NULL)
    {
	L_rf_delete_reg(reg);
	return NULL;
    }
    else
	return reg;
}

/******************************************************************************\
 *
 * Register file declaration routines
 *
\******************************************************************************/

RegFile *L_rf_new_reg_file()
{
    RegFile *reg_file;
    
    reg_file = L_alloc (L_alloc_reg_file);

    reg_file->reg = L_alloc(L_alloc_reg_bank);
    reg_file->reg->size = REGBANK_GROWTH;
    reg_file->reg->dest = (Reg **)calloc(REGBANK_GROWTH,sizeof(Reg *));

    reg_file->mac = L_alloc(L_alloc_reg_bank);
    reg_file->mac->size = REGBANK_GROWTH;
    reg_file->mac->dest = (Reg **)calloc(REGBANK_GROWTH,sizeof(Reg *));

    return reg_file;
}

RegBank *L_rf_enlargen_bank(RegBank *bank, int new_size)
{
    int		i;
    Reg 	**new_reg;

    if (bank->size > new_size) return bank;

    /* Allocate the new bank */
    new_reg = (Reg **)calloc(new_size+REGBANK_GROWTH, sizeof(Reg *));

    /* Copy the contents from the old bank to the new bank */
    for (i=0; i<bank->size; i++)
	new_reg[i] = bank->dest[i];
    
    free(bank->dest);

    bank->size = new_size+REGBANK_GROWTH;
    bank->dest = new_reg;

    return bank;
}

void L_rf_reset_reg_file(RegFile *reg_file)
{
    int		i;

    for (i=0; i<reg_file->reg->size; i++)
	if (reg_file->reg->dest[i])
	{
	    L_rf_delete_reg(reg_file->reg->dest[i]);
	    reg_file->reg->dest[i] = NULL;
	}

    for (i=0; i<reg_file->mac->size; i++)
	if (reg_file->mac->dest[i])
	{
	    L_rf_delete_reg(reg_file->mac->dest[i]);
	    reg_file->mac->dest[i] = NULL;
	}
}

void L_rf_delete_reg_file(RegFile *reg_file)
{
    L_rf_reset_reg_file(reg_file);

    if (reg_file->reg->dest)
    {
        free(reg_file->reg->dest);
        L_free(L_alloc_reg_bank, reg_file->reg);
    }

    if (reg_file->mac->dest)
    {
        free(reg_file->mac->dest);
        L_free(L_alloc_reg_bank, reg_file->mac);
    }

    L_free(L_alloc_reg_file, reg_file);
}

/******************************************************************************\
 *
 *  Support routines for "pseudo" register file.
 *
\******************************************************************************/

void L_rf_delete_register_gt_level(RegFile *reg_file, int level) 
{
    int 	i;

    for (i=0; i<reg_file->reg->size; i++)
    {
	if (reg_file->reg->dest[i])
	{
	    reg_file->reg->dest[i] = 
		L_rf_delete_reg_gt_level(reg_file->reg->dest[i], level);
	}
    }

    for (i=0; i<reg_file->mac->size; i++)
    {
	if (reg_file->mac->dest[i])
	{
	    reg_file->mac->dest[i] = 
		L_rf_delete_reg_gt_level(reg_file->mac->dest[i], level);
	}
    }
}

void L_rf_define_register(RegFile *reg_file, L_Operand *dest, 
    int opc, L_Operand *src0, L_Operand *src1, L_Operand *src2, int level)
{
    int		num = 0;
    RegBank 	*bank = NULL;

    if (L_is_register(dest))
    {
	num = dest->value.r;

	if (reg_file->reg->size <= num)
	    reg_file->reg = L_rf_enlargen_bank(reg_file->reg, num);

	bank = reg_file->reg;
    }
    else if (L_is_macro(dest))
    {
	num = dest->value.mac;

	if (reg_file->mac->size <= num)
	    reg_file->mac = L_rf_enlargen_bank(reg_file->mac, num);

	bank = reg_file->mac;
    }
    else
	L_punt("L_rf_define_register: illegal operand type");
    
    if (bank->dest[num]==NULL)
	bank->dest[num] = L_rf_new_reg(dest);

    bank->dest[num]->first_value = L_v_add_value(bank->dest[num]->first_value, 
	opc, src0, src1, src2, level);
}

/*
 * This routine is used to get the values assigned for registers and
 * parameter macro registers.
 *
 * The routine will create a register with all of the highest level
 * values defined in the register file.  Thus, this function only
 * needs to be queried once per register.
 *
 * Since incoming parameter registers and return registers are the only
 * types of registers that can pass information inter-procedurally, we
 * will query the inter-procedural database if the macro is not defined
 * in the register file.
 */
Reg *L_rf_get_register(CG_Node *cg_node, RegFile *reg_file, L_Operand *dest) 
{
    int			num;
    Value		*first_value, *value, *last_value;
    Reg			*new_reg = NULL, *reg;
    RegBank		*bank;

    if (L_is_register(dest))
    {
        num = dest->value.r;
        bank = reg_file->reg;

	new_reg = L_rf_new_reg(dest);

	if (num >= reg_file->reg->size)
	    return new_reg;

	if (bank->dest[num] == NULL) return new_reg;

	first_value = bank->dest[num]->first_value;
	last_value = NULL;

	for (value = L_v_get_value_same_level(first_value, last_value);
	     value != NULL;
	     value = L_v_get_value_same_level(first_value, last_value))
	{
    	    new_reg->first_value = L_v_add_value(new_reg->first_value, 
	        value->opc, value->src0, value->src1, value->src2, 
		value->level);
	    last_value = value;
	}

    }
    else if (L_is_macro(dest))
    {
        num = dest->value.mac;
        bank = reg_file->mac;

	/* Update local register file from data base */
        if (num >= bank->size)
	    bank = L_rf_enlargen_bank(bank, num);

	if (bank->dest[num] == NULL) 
	{
	    bank->dest[num] = L_rf_new_reg(dest);
	    reg = bank->dest[num];

	    /* Get any newer parameter values from the database */
	    new_reg = L_db_query_param_reg(cg_node, -1, -1, dest);

	    /* Update the local register file */
	    reg = bank->dest[num];
	    first_value = new_reg->first_value;
	    last_value = NULL;

	    for (value = new_reg->first_value; value != NULL; 
	         value = value->next_value)
	    {
    	        reg->first_value = L_v_add_value(reg->first_value, 
	            value->opc, value->src0, value->src1, value->src2, 
		    value->level);
	    }

	    /* free up memory used to return values from database */
	    L_rf_delete_reg(new_reg);
	}
	else
	    reg = bank->dest[num];

	/* Get any values from the register file for the specified destination*/
	new_reg = L_rf_new_reg(dest);
	first_value = reg->first_value;
	last_value = NULL;

	for (value = L_v_get_value_same_level(first_value, last_value);
	     value != NULL;
	     value = L_v_get_value_same_level(first_value, last_value))
	{
    	    new_reg->first_value = L_v_add_value(new_reg->first_value, 
	        value->opc, value->src0, value->src1, value->src2, 
		value->level);
	    last_value = value;
	}
    }
    else
	L_punt ("L_rf_get_register: illegal operand type");

    return new_reg;
}

/*
 * This routine has to do things a little different then L_rf_get_register
 * because it is looking for the return register of the function.
 * 
 * 1) It will update the incoming parameter registers for the target
 *    function with the appropriate contents of the current register file.
 *
 * 2) It will update the return register with all possible results from
 *    the target function.
 *
 * 3) It will return the values discovered to the calling routine.
 *
 * 
 */
Reg *L_rf_get_return_reg(CG_Node *current, RegFile *reg_file, L_Oper *jsr, 
    L_Operand *target_operand, L_Operand *dest) 
{
    int			num, pnum, i;
    Value		*first_value, *value, *last_value;
    Reg			*new_reg = NULL, *reg;
    RegBank		*bank;
    L_Attr		*attr;
    CG_Node		*target;
    L_Operand		*tmp_operand;

    if (L_is_macro(dest))
    {
	target = L_cg_find_node(target_operand->value.l);

        num = dest->value.mac;
        bank = reg_file->mac;

	/* Update local register file from data base */
        if (num >= bank->size)
	    bank = L_rf_enlargen_bank(bank, num);

	if (bank->dest[num] == NULL) 
	    bank->dest[num] = L_rf_new_reg(dest);

	/* 
	 * Update the incoming parameter registers for this function
	 * from the state of the current register.
	 */
	if ((attr=L_find_attr(jsr->attr, "tr"))!=NULL)
	{
	    for (i=0; i<attr->max_field; i++)
	    {
	        /* Update local register file from data base */
	        pnum = attr->field[i]->value.mac;
	    
                if (pnum >= bank->size)
	            bank = L_rf_enlargen_bank(bank, pnum);

	        reg = bank->dest[pnum];

	        first_value = reg->first_value;
	        last_value = NULL;

	        for (value = L_v_get_value_same_level(first_value, last_value);
	             value != NULL;
	             value = L_v_get_value_same_level(first_value, last_value))
	        {
	            L_db_add_param_value(target->func_id, current->func_id,
		        jsr->id, attr->field[i], value->opc, value->src0,
		        value->src1, value->src2);

	            last_value = value;
	        }
	    }
	}

	if ((attr=L_find_attr(jsr->attr, "tm"))!=NULL)
	{
	    for (i=0; i<attr->max_field; i++)
	    {
	        /* Update local register file from data base */
	        pnum = attr->field[i]->value.mac;
		tmp_operand = L_new_macro_operand(pnum, L_return_old_ctype(attr->field[i]),
		    0);
	    
                if (pnum >= bank->size)
	            bank = L_rf_enlargen_bank(bank, pnum);

	        reg = bank->dest[pnum];

	        first_value = reg->first_value;
	        last_value = NULL;

	        for (value = L_v_get_value_same_level(first_value, last_value);
	             value != NULL;
	             value = L_v_get_value_same_level(first_value, last_value))
	        {
	            L_db_add_param_value(target->func_id, current->func_id,
		        jsr->id, tmp_operand, value->opc, value->src0,
		        value->src1, value->src2);

	            last_value = value;
	        }

		L_delete_operand(tmp_operand);
	    }
	}

	/* Get any newer parameter values from the database */
	new_reg = L_db_query_return_reg(current, target, dest);

	/* Update the local register file */
	reg = bank->dest[num];
	first_value = new_reg->first_value;
	last_value = NULL;

	for (value = new_reg->first_value; value != NULL; 
	     value = value->next_value)
	{
    	    reg->first_value = L_v_add_value(reg->first_value, 
	        value->opc, value->src0, value->src1, value->src2, 
		value->level);
	}

	/* free up memory used to return values from database */
	L_rf_delete_reg(new_reg);

	/* Get any values from the register file for the specified destination*/
	new_reg = L_rf_new_reg(dest);
	first_value = reg->first_value;
	last_value = NULL;

	for (value = L_v_get_value_same_level(first_value, last_value);
	     value != NULL;
	     value = L_v_get_value_same_level(first_value, last_value))
	{
    	    new_reg->first_value = L_v_add_value(new_reg->first_value, 
	        value->opc, value->src0, value->src1, value->src2, 
		value->level);
	    last_value = value;
	}
    }
    else
	L_punt ("L_rf_get_register: illegal operand type");

    return new_reg;
}
