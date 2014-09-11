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
 *  File: l_ip_value.c
 *
 *  Description:  These routines support creation of values stored in memory,
 *	register files 
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
 *  
 *
\******************************************************************************/

Value *L_v_new_value(int opc, L_Operand *src0, L_Operand *src1, 
    L_Operand *src2, int level)
{
    Value *new_value;

    new_value = L_alloc(L_alloc_value);

    new_value->level = level;

    new_value->opc = opc;
    new_value->src0 = L_copy_operand(src0);
    new_value->src1 = L_copy_operand(src1);
    new_value->src2 = L_copy_operand(src2);

    new_value->next_value = NULL;

    return new_value;
}

Value *L_v_copy_value(Value *value)
{
    return L_v_new_value(value->opc, value->src0, value->src1, value->src2, 
	value->level);
}

Value *L_v_delete_value_gt_level(Value *first_value, int level) 
{
    Value *value, *next_value, *prev_value;

    prev_value = NULL;

    for (value=first_value; value!=NULL; value=next_value)
    {
	next_value = value->next_value;

	if (value->level <= level)
	    break;

	if (value->level > level)
	{
	    if (first_value == value)
	        first_value = value->next_value;

	    if (prev_value)
	        prev_value->next_value = value->next_value;

    	    if (value->src0)
		L_delete_operand(value->src0);

    	    if (value->src1)
		L_delete_operand(value->src1);

    	    if (value->src2)
		L_delete_operand(value->src2);

    	    L_free(L_alloc_value, value);
	}
	else
	    prev_value = value;
    }

    return first_value;
}

void L_v_delete_all_value(Value *first_value) 
{
    Value *value, *next_value;

    for (value=first_value; value!=NULL; value=next_value)
    {
	next_value = value->next_value;

    	if (value->src0)
	    L_delete_operand(value->src0);

    	if (value->src1)
	    L_delete_operand(value->src1);

    	if (value->src2)
	    L_delete_operand(value->src2);

    	L_free(L_alloc_value, value);
    }
}

Value *L_v_find_matching_value(Value *first_value, int opc, L_Operand *src0, 
    L_Operand *src1, L_Operand *src2, int level)
{
    Value *value;

    for (value = first_value; value!=NULL; value=value->next_value)
    {
	if (value->level < level)
	    return NULL;

	if (value->level != level) continue;

	if (value->opc != opc) continue;

	if (!L_same_operand(value->src0, src0)) 
	    continue;

	if (!L_same_operand(value->src1, src1)) 
	    continue;

	if (L_same_operand(value->src2, src2)) 
	    return value;
    }

    return NULL;
}

Value *L_v_add_value (Value *first_value, int opc, L_Operand *src0, 
    L_Operand *src1, L_Operand *src2, int level)
{
    Value	*new_value;

    if (L_v_find_matching_value(first_value, opc, src0, src1, src2, level))
	return first_value;

    new_value = L_v_new_value(opc, src0, src1, src2, level);

    /* We always add the value to the head of the list */
    if (first_value)
	new_value->next_value = first_value;

    return new_value;

#if 0
    prev_value = NULL;
    if (first_value)
    {
        for (value = first_value; value != NULL; value=value->next_value)
        {
	    if (level >= value->level)
	    {
		new_value->next_value = value;

		if (prev_value)
		{
		    prev_value->next_value = value;
		    return first_value;
		}
		else
		    return new_value;
	    }	

	    prev_value = value;
	}

        return first_value;
    }
    else
	return new_value;
#endif
}

Value *L_v_get_value_same_level (Value *first_value, Value *last_value)
{
    if (last_value)
    {
	if ((last_value->next_value) &&
	    (last_value->next_value->level == last_value->level))
	    return last_value->next_value;
	else
	    return NULL;
    }
    else
	return first_value;
}
