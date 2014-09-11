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
 *  File: l_static.c
 *
 *  Description:  Computes static execution time for a file.
 *	This requires that the issue and completion time be present in the 
 *	input lcode files.
 *
 *  Creation Date :  March, 1994
 *
 *  Author:  Roger A. Bringmann and Wen-mei Hwu
 *
 *  Revision 1.4  1996/04/14 22:09:57  lavery
 *  Pass file name string to L_print_schedule instead of file pointer to
 *  be compatible with change made to L_print_schedule in acyclic scheduler.
 *
 *  Revision 1.3  1995/10/04 20:50:01  lavery
 *  Fixed bug in initialization of relative latency array.
 *
 *  Revision 1.2  1995/09/14 22:59:52  lavery
 *  Bug fix.  Relative latency field of isl attribute checked before oper
 *  checking to see if oper had a corresponding destination.
 *
 *  Revision 1.1.1.1  1995/08/30 16:49:16  david
 *  Import of IMPACT source
 *
 *
 *  Copyright (c) 1993 Roger A. Bringmann , Wen-mei Hwu and The Board of
 *		  Trustees of the University of Illinois. 
 *		  All rights reserved.
 *
 *  The University of Illinois software License Agreement
 *  specifies the terms and conditions for redistribution.
 *
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1993 Roger A. Bringmann, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include <Lcode/l_schedule.h>

static L_Alloc_Pool *Lstatic_sched_info_pool;

void 
L_gen_code(Parm_Macro_List * command_line_macro_list)
{
    L_Attr	*attr;
    L_Oper	*oper;
    L_Cb	*cb;
    Sched_Info	*sinfo;
    int         i;

    /* Perform mdes initialization (may be .lmdes or .lmdes2 file
     * for static analysis) -JCG 4/30/98
     */

    L_init_lmdes2 (L_lmdes_file_name, L_max_pred_operand, L_max_dest_operand,
		   L_max_src_operand, 4 /* Support up to 4 sync operands */);


    Lstatic_sched_info_pool = L_create_alloc_pool ("Sched_Info", sizeof(Sched_Info), 100);

    L_open_input_file(L_input_file);

    while (L_get_input() != L_INPUT_EOF)
    {
	if (L_token_type==L_INPUT_FUNCTION)
	{
    	    L_do_flow_analysis(L_fn, LIVE_VARIABLE);

	    for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
	    {
		for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
		{
		    if (!IS_IGNORE(oper->proc_opc))
		    {
			sinfo = oper->ext = 
                              (Sched_Info *) L_alloc(Lstatic_sched_info_pool);
                        sinfo->relative_latency = (int *)
                              Lcode_malloc (L_max_dest_operand * sizeof(int));
                        /* initialize relative latency array to -1 */
			for (i = 0; i < L_max_dest_operand; i++) {
                            sinfo->relative_latency[i] = -1;
                        }
			if ((attr=L_find_attr(oper->attr, "isl"))!=0) {
                            if (attr->field[0] != NULL) {
			        sinfo->issue_time = attr->field[0]->value.i;
                            }
                            else {
			        L_punt ("issue time not defined for oper %d!\n", oper->id);
                            }

			    /* now use relative latency. BLD 6/95 */
			    for (i = 0; i < L_max_dest_operand; i++)
			    {
				if (oper->dest[i] != NULL) {
				    sinfo->relative_latency[i] = 
                                                     attr->field[i+2]->value.i;
                                }
			    }
                        }

			/* added for backward compatibility. BLD 6/95 */
			else if ((attr=L_find_attr(oper->attr, "isc"))!=0) {
			    if (attr->field[0] != NULL) {
                                sinfo->issue_time = attr->field[0]->value.i;
                            }
                            else {
                                L_punt ("issue time not defined for oper %d!\n",oper->id);
                            }

                            if (attr->field[2] != NULL) {
                                for (i = 0; i < L_max_dest_operand; i++)
                                {
                                    if (oper->dest[i] != NULL)
                                       sinfo->relative_latency[i] =
                                                attr->field[2] - attr->field[0];
                                }
                            }
                            else {
                                L_punt ("completion time not defined for oper %d!\n", oper->id);
                            }
			}

			else {
			    L_punt ("no isl or isc attribute for oper %d!\n", oper->id);
                        }
		    }
		}
	    }

	    L_print_schedule("IMPACT_001", L_fn);

	    for (cb = L_fn->first_cb; cb != NULL; cb = cb->next_cb)
	    {
		for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
		{
		    if (!IS_IGNORE(oper->proc_opc))
		    {
			L_free(Lstatic_sched_info_pool, oper->ext);
		    }
		}
	    }
            L_delete_func(L_fn);
	}
        else {
            L_delete_data(L_data);
        }
    }
    L_close_input_file(L_input_file);
}
