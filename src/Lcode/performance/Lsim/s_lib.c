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
 *      File:   s_lib.c
 *      Author: John Gyllenhaal
 *      Creation Date:  Aug 1994
 *      Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1994 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include "s_main.h"

/*
 * Frees sint after checking that is not in any queues.
 */
void S_free_sint (Sint *sint)
{
    Sq_entry *entry;
    /* Test to see if sint in any queues */
    if (sint->entry_list != NULL)
    {
	fprintf (stderr, "S_free_sint: Sint still in the following queues:\n");
	
	for (entry = sint->entry_list; entry != NULL; 
	     entry = entry->next_queue)
	{
	    fprintf (stderr, "%s\n", entry->queue->name);
	}
	S_print_sint (stderr, sint);
	S_punt ("Cannot continue.");
    }
    S_free_processor_data (sint);
    L_free (Sint_pool, sint);
}

/*
 * Print out sint to look like new lcode.
 * 
 * None of the dependence indicators have been written yet.
 */
void S_print_sint (FILE *out, Sint *sint)
{
    S_Oper *oper;
    /* 10/25/04 REK Commenting out unused variables to quiet compiler
     *              warnings. */
#if 0
    int max_dest, max_src, max_pred;
    int i;
    S_Opc_Info *info;
    int has_attr;
    unsigned int opflags;
#endif

    /* Get oper for easy access */
    oper = sint->oper;


    /* Print function name if first oper in function */
    if (&sint->fn->op[0] == oper)
    {
	fprintf (out, "(function %s)\n", sint->fn->name);
    }

    /* Print cb if first oper in cb */
    if (oper->cb->start_pc == oper->pc)
    {
	fprintf (out, "  (cb %i)\n", oper->cb->lcode_id);
    }
    
    /* Print the sint itself */
    S_print_only_sint (out, sint);
}

void S_print_only_sint (FILE *out, Sint *sint)
{
    S_Oper *oper;
    int max_dest, max_src, max_pred;
    int i;
    S_Opc_Info *info;
    int has_attr, has_flags;
    unsigned int opflags,playdoh_flags;

    /* Get oper for easy access */
    oper = sint->oper;

    /* Find the max dest/src/pred defined */
    max_dest = -1;
    max_src = -1;
    max_pred = -1;
    for (i=S_first_dest; i <= S_last_dest; i++)
	if (oper->operand[i] != 0)
	    max_dest = i;
    for (i=S_first_src; i <= S_last_src; i++)
	if (oper->operand[i] != 0)
	    max_src = i;
    for (i=S_first_pred; i <= S_last_pred; i++)
	if (oper->operand[i] != 0)
	    max_pred = i;


    /* If sint from off correct path, print indicator in indentation */ 
    if (sint->flags & OFF_PATH)
	fprintf (out, "OFF ");
    else
	fprintf (out, "    ");

    /* Print lcode id and name */
    fprintf (out, "(op %i %s ", oper->lcode_id, opc_info_tab[oper->opc].name);

    /* Print out flags, if any */
    opflags = sint->oper->flags;
    if ((opflags & SPECULATIVE) ||
	(opflags & NON_TRAPPING) ||
	(opflags & PROMOTED))
    {
	has_flags = 1;
    }
    else
    {
	has_flags = 0;
    }

    if (has_flags)
	fprintf (out, "<");

    if (opflags & SPECULATIVE)
	fprintf (out, "S");
    
    if (opflags & NON_TRAPPING)
	fprintf (out, "M");

    if (opflags & PROMOTED)
	fprintf (out, "P");

    if (has_flags)
	fprintf (out, "> ");


    /* Print out preds, if any */
    if (max_pred >= 0)
    {
	fprintf (out, "<");
	
	/* Print out preds */
	for (i = S_first_pred; i <= max_pred; i++)
	{
	    fprintf (out, "(%s)", operand_tab[oper->operand[i]]->string);
	}

	fprintf (out, "> ");
    }

    fprintf (out, "[");

    /* Print out dests */
    for (i = S_first_dest; i <= max_dest; i++)
    {
	fprintf (out, "(%s)", operand_tab[oper->operand[i]]->string);
    }

    fprintf (out, "] [");

    /* Print out srcs */
    for (i = S_first_src; i <= max_src; i++)
    {
	fprintf (out, "(%s)", operand_tab[oper->operand[i]]->string);
    }

    fprintf (out, "]");

    /* Detect if we should be printing attributes */
    info = &opc_info_tab[sint->oper->opc];
    if (S_sched_info_avail ||
	(opflags & MCB_ATTR) || 
	(opflags & BRANCH_PREDICTED_TAKEN) ||
	(opflags & BRANCH_PREDICTED_FALLTHRU) ||
	(opflags & CHANGES_STATE) ||
	(opflags & ZERO_SPACE) ||
	(opflags & IMPLICIT_MEMORY_OP) ||
	(info->opc_type == LOAD_OPC) || 
	(info->opc_type == STORE_OPC) ||
	(info->opc_type == PREFETCH_OPC) ||
	(sint->flags & MASKED_BUS_ERROR) ||
	(sint->flags & MASKED_SEG_FAULT) ||
	(sint->playdoh_flags) ||
	(sint->oper->trace_words_read != 0))
    {
	has_attr = 1;
    }
    else
    {
	has_attr = 0;
    }

    /* If has attr, print opening '<' */
    if (has_attr)
	fprintf (out, " <");

    /* Print out stats_on/stats_off attr */
    if (opflags & STATS_ON_ATTR)
	fprintf (out, "(stats_on)");

    if (opflags & STATS_OFF_ATTR)
	fprintf (out, "(stats_off)");

    if (opflags & FORCE_SIM_ON)
	fprintf (out, "(force_sim 1)");
    
    if (opflags & FORCE_SIM_OFF)
	fprintf (out, "(force_sim 0)");

    if (opflags & STOP_SIM)
	fprintf (out, "(stop_sim)");

    if (opflags & ZERO_SPACE)
	fprintf (out, "(ZERO_SPACE)");

    if (opflags & IMPLICIT_MEMORY_OP)
	fprintf (out, "(IMPLICIT_MEM)");

    if (sint->oper->trace_words_read != 0)
	fprintf (out, "(TSKIP %i)", sint->oper->trace_words_read);


    /* If scheduling_info_avail, print cycle, slot, and 
     * start and end of instruction packets.
     */
    if (S_sched_info_avail)
    {
	if (opflags & START_PACKET)
	    fprintf (out, "(startp)");
	
	if (opflags & END_PACKET)
	    fprintf (out, "(endp)");

	fprintf (out, "(cycle %i)(slot %i)",
		 sint->oper->cycle, sint->oper->slot);
    }

    /* Print out load/store/prefetch addresses */
    if ((info->opc_type == LOAD_OPC) || (info->opc_type == STORE_OPC) ||
	(info->opc_type == PREFETCH_OPC))
    {
	fprintf (out, "(Addr %08x)", sint->trace.mem_addr);
    }

    if (sint->flags & MASKED_BUS_ERROR)
	fprintf (out, "(BUS ERROR)");
    
    if (sint->flags & MASKED_SEG_FAULT)
	fprintf (out, "(SEG_FAULT)");
    
    /* Print out static branch predictions */
    if (opflags & BRANCH_PREDICTED_TAKEN)
	fprintf (out, "(bpred 1)");
    if (opflags & BRANCH_PREDICTED_FALLTHRU)
	fprintf (out, "(bpred 0)");

    /* Print out MCB attr */
    if ((S_MCB_model != MCB_MODEL_NO_MCB) && 
	(opflags & MCB_ATTR))
    {
	fprintf (out, "(MCB)");
    }

    /* Print out ALAT attr */
    if ((S_ALAT_model != ALAT_MODEL_NO_ALAT) &&
	(opflags & MCB_ATTR))
    {
	fprintf (out, "(ALAT)");
    }

    /* Print out cache hierarchy specifiers */
    if ((playdoh_flags = sint->playdoh_flags))
    {
	if (playdoh_flags & PLAYDOH_TCHS_V1)
		fprintf (out, "(chs (i 0)(");
	else if (playdoh_flags & PLAYDOH_TCHS_C1)
		fprintf (out, "(chs (i 1)(");
	else if (playdoh_flags & PLAYDOH_TCHS_C2)
		fprintf (out, "(chs (i 2)(");
	else 
		fprintf (out, "(chs (i 3)(");
	if (info->opc_type != STORE_OPC)
	{
	   if (playdoh_flags & PLAYDOH_SCHS_V1)
		fprintf (out, "i 0");
	   else if (playdoh_flags & PLAYDOH_SCHS_C1)
		fprintf (out, "i 1");
	   else if (playdoh_flags & PLAYDOH_SCHS_C2)
		fprintf (out, "i 2");
	   else 
		fprintf (out, "i 3");
	}
	fprintf (out, ")");
    }

    /* If has attr, print closing '>' */
    if (has_attr)
	fprintf (out, ">");

    fprintf (out, "\n");
}


/*
 * Dumps the code image out to file in pseudo-Lcode format.
 */
void S_print_code (FILE *out)
{
    S_Fn *fn;
    Sint sint;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler
     *              warning. */
#if 0
    S_Oper *op;
#endif
    int i;

    for (fn = head_fn; fn != NULL; fn = fn->next_fn)
    {
	for (i=0; i < fn->op_count; i++)
	{
	    sint.oper = &fn->op[i];
	    sint.fn = fn;
	    sint.flags = 0;
	    sint.trace.mem_addr = 0;

	    if (S_mode == X86_TRACE_GENERATOR)
	    {
		fprintf (out, "%6i: ", sint.oper->instr_addr);
	    }

	    S_print_sint (out, &sint);
	}
    }
}
