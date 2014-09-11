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
 *  File:  l_ip_main.c
 *
 *  Description:  
 *	Main entry point for safe speculation.
 *
 *  Creation Date :  May 1994
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

/*
 *  L_gen_code is the entry point to code generation called from l_main.c
 */
void L_gen_code(Parm_Macro_List *command_line_macro_list)
{
    CallGraph	*callgraph;
    FILE	*filelist;
    CG_Node     *node;
    char        in_file[128], out_file[128];

    L_interproc_init(command_line_macro_list);

    filelist = fopen(L_filelist_file, "r");
    if (filelist==NULL)
	L_punt("Lsafe: invalid filename for filelist");

    /* A callgraph will always be generated based upon the analysis level */
    callgraph = L_callgraph_build(filelist);

    /* Handle the interprocedural optimizations */
    if (L_mark_sef_jsr || L_mark_safe_pei)
    {
        fseek(filelist, 0, 0);

	if (L_mark_sef_jsr)
	{
	    L_find_side_effect_free(callgraph, filelist);
            fseek(filelist, 0, 0);
	}

	/* Mark instructions that are safe for speculation */
	if (L_mark_safe_pei)
	{
            fseek(filelist, 0, 0);
	}

	/* Print the new functions */
        while (fgets(in_file, sizeof(in_file), filelist) != NULL)
        {
            in_file[strlen(in_file)-1] = '\0';
            L_open_input_file(in_file);

            sprintf(out_file, "%s%s", in_file, L_sef_file_ext);

            L_OUT = fopen(out_file, "w");

            while (L_get_input() != L_INPUT_EOF)
            {
                if (L_token_type==L_INPUT_FUNCTION)
                {
                    node = L_cg_find_node(L_fn->name);

                    if (node->fn)
                    {
                        L_delete_func(L_fn);
                        L_fn = node->fn;
                    }

                    L_print_func(L_OUT, L_fn);
                    L_delete_func(L_fn);
                }
                else
                {
                    L_print_data(L_OUT, L_data);
                    L_delete_data(L_data);
                }
            }
            L_close_input_file(in_file);
            fclose(L_OUT);
	}
    }

    fclose(filelist);


    if (L_print_callgraph)
    	L_callgraph_print(stdout, callgraph);

    L_callgraph_delete(callgraph);
}
