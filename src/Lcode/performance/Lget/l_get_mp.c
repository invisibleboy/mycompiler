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
 *	File :		l_btbprof.c
 *	Description :	Inserts branch mispredition profile into Lcode
 *	Creation Date :	Oct, 1993
 *	Author : 	John Gyllenhaal
 *
 *	(C) Copyright 1992, John Gyllenhaal and Wen-mei Hwu
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#include <config.h>
#include <stdio.h>
#include <Lcode/l_main.h>

/* Skip 'count' lines in file 'in' */
static void MP_skip_lines (in, count, file_name)
FILE *in;
int count;
char *file_name;
{
    char buf[500];
    int i;
    
    for (i=0; i < count; i++)
    {
	if (fgets (buf, sizeof(buf), in) == NULL)
	    L_punt ("Unexpected EOF in mp profile file '%s'.",
		    file_name);
    }
}

/*
 * Reads the misprediction profile info from 'file_name' and inserts
 * it incode the fn as an attribute (MPC for counter BTB or MP2 for
 * two level BTB).  
 * Punts on error.
 */
void L_get_mp_profile (fn, file_name)
L_Func *fn;
char *file_name;
{
    char func_name[500], btb_type[500];
    int func_size;
    L_Cb *cb;
    L_Oper *op;
    FILE *in;
    char type, expected_type;
    int	mispred_count, branch_count;
    L_Attr *attr;
    char *attr_name = NULL;
   
    /* Open the profile file for reading */
    if ((in = fopen (file_name, "r")) == NULL)
	L_punt ("Error opening mp profile file '%s'.", file_name);
	
    
    /* Skip functions until fn->name is found */
    while ((fscanf (in, "Begin %s %i %s\n", func_name, &func_size, 
		    btb_type) == 3) &&
	   (strcmp (func_name, fn->name) != 0))
	MP_skip_lines (in, func_size + 2, file_name);
    
    /* Make fn->name was found in profile file */
    if (strcmp (func_name, fn->name) != 0)
	L_punt ("Function '%s' not found in mp profile file '%s'.",
		func_name, file_name);
    
    /* Set attribute name depending on BTB type */
    if (strcmp (btb_type, "counter") == 0)
	attr_name = "MPC";
    else if (strcmp (btb_type, "2-level") == 0)
	attr_name = "MP2";
    else
	L_punt ("Unknown BTB type '%s' in mp profile file '%s'.",
		btb_type, file_name);

    /*
     * For each branch (jmp, condiction branch, jsr, ret) in the
     * function, get the misprediction profile info.  It is
     * assumed that the profile info is in the same order as
     * the branches are encountered in the function.
     */
    branch_count = 0;
    for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
	for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	    /* 
	     * Find out what type of branch we are expecting or 
	     * no branch at all.
	     */
	    /* Initialize expected type to not a branch */
	    expected_type = ' ';
	    if (L_cond_branch_opcode(op))
		expected_type = 'b';
	    else if (L_uncond_branch_opcode(op))
		expected_type = 'j';
	    else if (L_register_branch_opcode(op))
		expected_type = 'h';
	    else if (L_subroutine_return_opcode (op))
		expected_type = 'r';
	    else if (L_subroutine_call_opcode (op))
		expected_type = 'f';

	    /* 
	     * If we have a branch, read in profile data on it and
	     * place in attribute.
	     */
	    if (expected_type != ' ')
	    {
		/* Increment branch count */
		branch_count++;

		/* Make sure have this many branches in profile file */
		if (branch_count > func_size)
		    L_punt ("mp profile file '%s' has less branches than fn.",
			    file_name);

		/* Read profile data */
		if (fscanf (in, "%c %i\n",&type, &mispred_count) != 2)
		    L_punt ("Parse error in mp profile file '%s'.",
			    file_name);

		/* Make sure branch types match */
		if (type != expected_type)
		    L_punt ("branch mismatch in mp profile file '%s'.", 
			    file_name);
		
		/* Add or update MP attribute */
		if ((attr = L_find_attr (op->attr, attr_name)) != NULL)
		    attr->field[0]->value.i = mispred_count;
		else
		{
		    attr = L_new_attr (attr_name, 1);
		    L_set_int_attr_field(attr, 0, mispred_count);
		    op->attr = L_concat_attr (op->attr, attr);
		}
		
	    }
	}
    }

    /*
     * Make sure profile file and function have exactly the same
     * number of branches. (Already checked for branch_count > func_size.)
     */
    if (branch_count < func_size)
	L_punt ("mp profile file '%s' has more branches than fn.",
		file_name);

    
    fclose (in);
}

