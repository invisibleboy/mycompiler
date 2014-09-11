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
 *      File:   s_x86_trace.c
 *      Author: John Gyllenhaal
 *      Creation Date:  April 1994
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

#undef DEBUG_X86_TRACE

extern S_Fn 	*head_fn;
static int	binmap_line;
static char	*binmap_name;
static FILE	*binmap_in;
static char	binmap_peekbuf[200] = "";

void S_read_parm_x86_trace (Parm_Parse_Info *ppi)
{
    L_read_parm_b (ppi, "x86_use_pipe", &S_x86_use_pipe);
    L_read_parm_s (ppi, "x86_trace_binmap_file", &S_x86_trace_binmap_file);
    L_read_parm_s (ppi, "x86_trace_desc", &S_x86_trace_desc);
    L_read_parm_s (ppi, "x86_trace_output_file", &S_x86_trace_output_file);
}

void S_print_configuration_x86_trace (FILE *out)
{
    S_print_configuration_system (out);

    fprintf (out,
	     "# X86_TRACE CONFIGURATION\n");
    fprintf (out, "x86 Binmap file:   %s\n", S_x86_trace_binmap_file);
    fprintf (out, "x86 trace output:  %s\n", S_x86_trace_output_file);
    fprintf (out, "use trace pipe:    ");
    if (S_x86_use_pipe)
	fprintf (out, "Yes\n");
    else
	fprintf (out, "No\n");
    fprintf (out, "x86 trace desc:    %s\n", S_x86_trace_desc);
    fprintf (out, "\n");

    fprintf (out, "# END CONFIGURATION\n");
    fprintf (out, "\n");

    fflush (out);
}


static void S_binmap_peek_next (char *ret_buf)
{
    if (binmap_peekbuf[0] == 0)
    {
	/* Do not care if peek past eof, just return nothing */
	if (fscanf (binmap_in, "%s", binmap_peekbuf) != 1)
	    binmap_peekbuf[0] = 0;
    }

    strcpy (ret_buf, binmap_peekbuf);
}

static void S_binmap_get_next (char *ret_buf)
{
    if (binmap_peekbuf[0] != 0)
    {
	strcpy (ret_buf, binmap_peekbuf);
	binmap_peekbuf[0] = 0;
    }
    else
    {
	if (fscanf (binmap_in, "%s", ret_buf) != 1)
	    S_punt ("Unexpected EOF in '%s' line %i.", 
		    binmap_name, binmap_line);
    }
}

int S_binmap_atoh (char *buf)
{
    char *end_ptr;
    int i;

    i = strtol (buf, &end_ptr, 16);

    if (*end_ptr != 0)
	S_punt ("'%s' line %i: Error  converting '%s' to hex.",
		binmap_name, binmap_line, buf);

    return (i);
}

int S_binmap_atoi (char *buf)
{
    char *end_ptr;
    int i;

    i = strtol (buf, &end_ptr, 10);

    if (*end_ptr != 0)
	S_punt ("'%s' line %i: Error  converting '%s' to int.",
		binmap_name, binmap_line, buf);

    return (i);
}

int S_read_binmap_instr (FILE *in, S_Oper *op)
{
    int lcode_id, size, skip_size;
    char	buf[200];
    unsigned char binary[20], *binptr;
    unsigned shift_temp;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler
     *              warning. */
#if 0
    unsigned int opc_byte;
#endif
    int instr_desc;
    int i;

    /* Increment the line we are on */
    binmap_line ++;

    /* Read lcode id and size of instruction */
    S_binmap_get_next (buf);
    lcode_id = S_binmap_atoi (buf);

    S_binmap_get_next (buf);
    size = S_binmap_atoi (buf);

    /* Make sure lcode id matches op's */
    if (lcode_id != op->lcode_id)
    {
	S_punt ("Error '%s' line %i: Lcode id %i expected not %i.\n",
		binmap_name, binmap_line, op->lcode_id, lcode_id);
    }
    
    /* Set the size of the operation */
    op->instr_size = size;

    /* Read in binary for instruction */
    for (i=0; i < size; i++)
    {
	S_binmap_get_next (buf);
	binary[i] = S_binmap_atoh (buf);
    }

    /* Pad binary by two bytes so that get 0  for modr/m byte and sib byte if
     * they doesn't exist.
     */
    binary[size] = 0;
    binary[size + 1] = 0;

    /* point to beginning of binary instr and zero instr_desc*/
    binptr = binary;
    instr_desc = 0;

    /* Check for 16-bit operand size prefix */
    if (*binptr == 0x66)
    {
	/* Set bit in instr_desc */
	instr_desc |= PREFIX_16_BIT_OPERANDS;

	/* Skip byte */
	binptr++;
    }
    
    /* Check for two-byte opcode prefix (prefix is first byte of opcode) */
    if (*binptr == 0x0f)
    {
	/* Set bit in instr_desc */
	instr_desc |= PREFIX_TWO_BYTE_OPCODE;

	/* Skip byte */
	binptr++;
    }
    
    /* Get opcode byte and put into bits 23:16 */
    shift_temp = *binptr;
    instr_desc |= (shift_temp << 16);
    binptr++;

    /* Get modr/m byte and put into bits 15:08 
     * (will put garbage in this byte if it doesn't exist) 
     */
    shift_temp = *binptr;
    instr_desc |= (shift_temp << 8);
    binptr++;

    /* Get sib byte and put into bits 07:00
     * (will put garbage in this byte if it doesn't exist) 
     */
    shift_temp = *binptr;
    instr_desc |= shift_temp;
    binptr++;

    op->instr_desc = instr_desc;

    /* Detect alignment no-op's (lcode id == -1),
     * while present, increase size to by no op sizes
     * to maintain alignment.
     */
    skip_size = size;
    S_binmap_peek_next (buf);
    while (strcmp (buf, "-1") == 0)
    {
	/* Reading in noop line, so update line no */
	binmap_line++;

	/* Read in noop information, throw out -1 already read */
	S_binmap_get_next (buf);
	S_binmap_get_next (buf);
	size = S_binmap_atoi (buf);
	
	/* Increment skip size by size of noop*/
	skip_size += size;

	/* skip noop binary */
	for (i=0; i < size; i++)
	    S_binmap_get_next (buf);

	/* Peek at next instruction to see if noop */
	S_binmap_peek_next (buf);
    }

    return (skip_size);
}

/* 
 * Loads an x86 binmap file.
 * Gets instruction sizes and calculates a compressed binary representation
 * for the instruction.  As it is processing the file, it changes the oper's
 * instr_addr to be correct for the x86.
 */
void S_load_binmap (char *file_name)
{
    S_Fn	*fn;
    S_Oper	*op;
    FILE	*in;
    int		instr_addr;
    int		i;
    char	func_buf[200];
    int		skip_size;

    if ((in = fopen (file_name, "r")) == NULL)
	S_punt ("Unable to open binmap file '%s'.", file_name);

    /* Set up binmap parser */
    binmap_in = in;
    binmap_line = 0;
    binmap_name = file_name;
    binmap_peekbuf[0] = 0;
    func_buf[0] = 0;

    /* Start addr at S_program_start_addr */
    instr_addr = S_program_start_addr;

    /* Expect binmap to map exactly (with some alignment info) with
     * the code image.  Punt if anything unexpected occurs.
     */
    for (fn = head_fn; fn != NULL; fn = fn->next_fn)
    {
	/* Make sure the function in the binmap is what we expect */
	binmap_line++;

	/* Skip "Function:" and get function name */
	S_binmap_get_next (func_buf);
	S_binmap_get_next (func_buf);
	
	if (strcmp (func_buf, fn->name) != 0)
	    S_punt ("Error in '%s' line %i: expected function '%s' not '%s'.",
		    binmap_name, binmap_line, fn->name, func_buf);
		    
#ifdef DEBUG_X86_TRACE
	fprintf (debug_out, "Function: %s\n", fn->name);
#endif

	for (i=0; i < fn->op_count; i++)
	{
	    op = &fn->op[i];

	    /* Set the start address for this op */
	    op->instr_addr = instr_addr;

	    skip_size = S_read_binmap_instr (in, op);

	    instr_addr += skip_size;

#ifdef DEBUG_X86_TRACE
	    fprintf (debug_out, "  %5i: op %3i (%2i) %08x\n", 
		    op->instr_addr, op->lcode_id, op->instr_size, 
		    op->instr_desc);

	    if (skip_size != op->instr_size)
	    {
	        fprintf (debug_out, "  Skipped %i bytes to align.\n",
			 (skip_size - op->instr_size));
	    }
#endif
	}
    }

    fclose (in);
}

void S_write_x86_start_of_trace ()
{
    int desc, temp;

    desc = (1 << 24) | X86_START_OF_TRACE;
    
    if (!S_trace_byte_order_reversed)
	temp = desc;
    else
	temp = SWAP_BYTES(desc);
    if (fwrite (&temp, sizeof (int), 1, S_x86_trace_out) != 1)
	S_punt ("Error writing to '%s'.", S_x86_trace_output_file);
}

void S_write_x86_end_of_trace ()
{
    int desc, temp;

    desc = (1 << 24) | X86_END_OF_TRACE;
    
    if (!S_trace_byte_order_reversed)
	temp = desc;
    else
	temp = SWAP_BYTES(desc);
    if (fwrite (&temp, sizeof (int), 1, S_x86_trace_out) != 1)
	S_punt ("Error writing to '%s'.", S_x86_trace_output_file);
}

void S_write_x86_info_int (int info_flag, int word)
{
    int temp;
    int record[5];
    int i;
    int size;

    size = 2;
    record[0] = (size << 24) | info_flag;
    record[1] = word;

    /* Write record out, doing byte reversal if necessary */
    for (i=0; i < size; i++)
    {
	if (!S_trace_byte_order_reversed)
	    temp = record[i];
	else
	    temp = SWAP_BYTES(record[i]);
	if (fwrite (&temp, sizeof (int), 1, S_x86_trace_out) != 1)
	    S_punt ("Error writing to '%s'.", S_x86_trace_output_file);
    }
}

void S_write_x86_info_string (char *str)
{
    int str_bytes, str_words, pad_bytes, record_size, temp;
    int record[5];
    int i;
    char pad_temp;

    /* Get string size */
    str_bytes = strlen (str) + 1;
    
    /* Cannot be longer than 1000 characters */
    if (str_bytes > 1000)
    {
	S_punt ("S_write_x86_info_string: length must be <= 1000 characters.\n'%s'\n", str);
    }
	
    /* Calculate how many ints needed for string */
    str_words = (str_bytes + 3) / 4;

    /* Calculate how many pad bytes are needed */
    pad_bytes = (str_words * 4) - str_bytes;

    /* Size of record is 1 word plus word for string */
    record_size = str_words + 1;

    record[0] = (record_size << 24) | X86_TEXTUAL_INFO;
    

    /* Write out record header */
    if (!S_trace_byte_order_reversed)
	temp = record[0];
    else
	temp = SWAP_BYTES(record[0]);
    if (fwrite (&temp, sizeof (int), 1, S_x86_trace_out) != 1)
	S_punt ("Error writing to '%s'.", S_x86_trace_output_file);

    /* Write out string */
    if (fwrite (str, 1, str_bytes, S_x86_trace_out) != str_bytes)
	S_punt ("Error writing info record to '%s'", S_x86_trace_output_file);

    /* Pad to word alignment */
    pad_temp = 0;
    for (i=0; i < pad_bytes; i++)
    {
	if (fwrite (&pad_temp, 1, 1, S_x86_trace_out) != 1)
	    S_punt ("Error writing info record to '%s'", 
		    S_x86_trace_output_file);
    }

}

static int x86_trace_initialized = 0;

/* Use simulation start time from s_main.c */
extern time_t S_start_time;

/*
 * Traces 'S_sample_size' instructions, generating an instruction
 * level trace for the x86.
 */
int S_write_x86_trace (int pc, int S_sample_size)
{
    Sint sint;
    int num_traced;
    int opflags;
    int record[10];
    int size, temp;
    S_Opc_Info *info;
    int i;
    char time_buf[100];

    /* Print out start up info */
    if (!x86_trace_initialized)
    {
	x86_trace_initialized = 1;

	S_write_x86_start_of_trace ();
	S_write_x86_info_string (S_x86_trace_desc);

	/* Get date and time of trace */
	if (S_start_time != -1)
	{
	    strftime (time_buf, sizeof (time_buf),
		      "%D %T %p",
		      localtime(&S_start_time));
	    S_write_x86_info_string (time_buf);
	}
	else
	{
	    S_write_x86_info_string ("Date unavailable");
	}

	S_write_x86_info_int (X86_SAMPLE_SIZE_PARM, S_sample_size);
	S_write_x86_info_int (X86_SKIP_SIZE_PARM, S_skip_size);
    
	S_write_x86_info_int (X86_SAMPLE_START_ADDR, oper_tab[pc]->instr_addr);
    }

    /* Otherwise, if starting a new sample, with a non-zero skip 
     * size, print out new start pc (number skipped printed in
     * skip routine).
     */
    else if (S_skip_size != 0) 
    {
	S_write_x86_info_int (X86_SAMPLE_START_ADDR, oper_tab[pc]->instr_addr);
    }

    /* Initialize number traced */
    num_traced = 0;

    /* Loop until trace desired number of instructions or hit end of program */
    for (num_traced = 0; (((num_traced < S_sample_size) || S_force_sim) && 
			  !S_end_of_program); num_traced++)
    {
	/* Increment simulation counter for now */
	S_num_sim_on_path++;

	/* Build sint and get trace info about sint */
	sint.oper = oper_tab[pc];
	sint.fn = sint.oper->cb->fn;
	sint.flags = 0;
	S_read_trace_info (S_pnode, &sint);

	/* Get operation flags for ease of use */
	opflags = sint.oper->flags;

	/* Detect changes in simulation state */
	if (opflags & CHANGES_STATE)
	{
	    /* Check for force_sim directives */
	    if (opflags & FORCE_SIM_ON)
	    {
                /*
                 * Do not allow instructions to be skipped until
                 * a FORCE_SIM_OFF is encountered.
                 */
                S_force_sim = 1;

		if (S_debug_force_sim_markers)
		{
		    fprintf (debug_out, 
			     "%s op %i: force sim on (S_write_x86_trace)\n",
			     sint.oper->cb->fn->name, sint.oper->lcode_id);
		}    
	    }
            else if (opflags & FORCE_SIM_OFF)
            {
                S_force_sim = 0;

		if (S_debug_force_sim_markers)
		{
		    fprintf (debug_out, 
			     "%s op %i: force sim off (S_write_x86_trace)\n",
			     sint.oper->cb->fn->name, sint.oper->lcode_id);
		}    
            }

	    /* Single end of program if at stop sim marker and
	     * have encountered stop_sim markes S_stop_sim_trip_count times.
	     */
	    if (opflags & STOP_SIM)
	    {
		S_stop_sim_markers_encountered ++;
		
		if (S_debug_stop_sim_markers)
		{
		    fprintf (debug_out, 
			     "%s op %i: tripped stop sim marker #%i\n",
			     sint.oper->cb->fn->name, sint.oper->lcode_id,
			     S_stop_sim_markers_encountered);
		}

		if (S_stop_sim_markers_encountered >= S_stop_sim_trip_count)
		{
		    S_end_of_program = 1;
		    if (S_debug_stop_sim_markers)
		    {
			fprintf (debug_out, 
				 "%s op %i: stop_sim marker terminating program\n",
				 sint.oper->cb->fn->name, sint.oper->lcode_id);
		    }
		}
	    }
	}

	/* Build trace record from sint (put instruction size in bits 23:20)*/
	record[0] = (sint.oper->instr_size << 20) | X86_NORMAL_INSTR;
	record[1] = sint.oper->instr_desc;
	size = 2;


	/* Detect memory instruction, and put address in info record
	 * if it is.
	 */
	info = &opc_info_tab[sint.oper->opc];
	if ((info->opc_type == LOAD_OPC) ||
	    (info->opc_type == STORE_OPC) ||
	    (info->opc_type == PREFETCH_OPC) ||
	    (opflags & IMPLICIT_MEMORY_OP))
	{
	    record[0] |= X86_CONTAINS_MEM_ADDR;
	    record[size] = sint.trace.mem_addr;
	    size++;
	}
	
	/* If branched, set pc to target, otherwise increment.
	 * Also put branch target address in record
	 */
	if (sint.flags & BRANCHED)
	{
	    pc = sint.trace.target_pc;

	    record[0] |= X86_CONTAINS_BR_TARGET;

	    /* Detect return/exit at end of program */
	    if (sint.trace.target_pc != 0)
		record[size] = oper_tab[sint.trace.target_pc]->instr_addr;
	    else
		record[size] = 0;
	    size++;
	}
	else
	    pc++;

	/* Detect if was call to untraced function */
	if (sint.flags & UNTRACED_JSR)
	{
	    record[0] |= X86_UNTRACED_CALL;
	    record[size] = 0;	/* For now */
	    size++;
	}

	/* Set size filed in trace record */
	record[0] |= (size << 24);

	/* Write record out, doing byte reversal if necessary */
	for (i=0; i < size; i++)
	{
	    if (!S_trace_byte_order_reversed)
		temp = record[i];
	    else
		temp = SWAP_BYTES(record[i]);
	    if (fwrite (&temp, sizeof (int), 1, S_x86_trace_out) != 1)
		S_punt ("Error writing to '%s'.", S_x86_trace_output_file);
	}
    }

    return (pc);
}



