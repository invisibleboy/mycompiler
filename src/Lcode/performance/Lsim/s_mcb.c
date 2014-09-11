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
 *      File:   s_mcb.c
 *      Author: John Gyllenhaal
 *      Creation Date:  1993
 *      Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and The Board of
 *                         Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

/* Stub file for s_mcb.c */
#include "s_main.h"

int S_print_mcb (FILE *out, MCB *mcb);

void S_read_parm_MCB (Parm_Parse_Info *ppi)
{
    L_read_parm_s (ppi, "MCB_model", &S_MCB_model_name);
    L_read_parm_i (ppi, "MCB_size", &S_MCB_size);
    L_read_parm_i (ppi, "MCB_assoc", &S_MCB_assoc);
    L_read_parm_i (ppi, "MCB_checksum_width", &S_MCB_checksum_width);
    L_read_parm_b (ppi, "MCB_all_loads_preloads", 
		   &S_MCB_all_loads_preloads);
    L_read_parm_b (ppi, "MCB_debug_load_load_conflicts",
		   &S_MCB_debug_load_load_conflicts);
    L_read_parm_b (ppi, "MCB_debug_load_store_conflicts",
		   &S_MCB_debug_load_store_conflicts);
}

void S_print_configuration_MCB (FILE *out)
{
    fprintf (out, "# MCB CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s MCB model.\n", S_MCB_model_name);
    if ((S_MCB_model != MCB_MODEL_NO_MCB) &&
	(S_MCB_model != MCB_MODEL_PERFECT))
    {
	fprintf (out, "%12u MCB size.\n",S_MCB_size);
	fprintf (out, "%12u MCB assoc.\n",S_MCB_assoc);
	fprintf (out, "%12u MCB checksum width.\n", S_MCB_checksum_width);
	if (S_MCB_all_loads_preloads)
	    fprintf (out, "%12s ", "All");
	else
	    fprintf (out, "%12s ", "MCB");
	fprintf (out, "loads are treated as MCB preloads.");
	
    }
    fprintf (out, "\n");
}

MCB *S_create_MCB (pnode)
Pnode *pnode;
{
    MCB *new_mcb;
    MCB_Entry *entry;
    MCB_Line *line;
    int num_lines;
    int j, i;

    if (L_pmatch (S_MCB_model_name, "No-MCB"))
    {
	/* Reassign name to beautify simulation output */
	S_MCB_model_name = "No-MCB";
	S_MCB_model = MCB_MODEL_NO_MCB;
    }
    else if (L_pmatch (S_MCB_model_name, "perfect"))
    {
	/* Reassign name to beautify simulation output */
	S_MCB_model_name = "Perfect";
	S_MCB_model = MCB_MODEL_PERFECT;
    }
    else if (L_pmatch (S_MCB_model_name, "always-conflict"))
    {
	/* Reassign name to beautify simulation output */
	S_MCB_model_name = "Always-conflict";
	S_MCB_model = MCB_MODEL_ALWAYS_CONFLICT;
    }
    else if (L_pmatch (S_MCB_model_name, "simple"))
    {
	/* Reassign name to beautify simulation output */
	S_MCB_model_name = "Simple";
	S_MCB_model = MCB_MODEL_SIMPLE_HASH;
    }
    else if (L_pmatch (S_MCB_model_name, "reg"))
    {
	/* Reassign name to beautify simulation output */
	S_MCB_model_name = "Reg";
	S_MCB_model = MCB_MODEL_REG_HASH;
    }
    else if (L_pmatch (S_MCB_model_name, "knuth"))
    {
	/* Reassign name to beautify simulation output */
	S_MCB_model_name = "Knuth";
	S_MCB_model = MCB_MODEL_KNUTH_HASH;
    }
    else
	S_punt ("S_create_MCB: Illegal MCB model '%s'", S_MCB_model_name);

    /* Create MCB */
    if ((new_mcb = (MCB *) malloc (sizeof(MCB))) == NULL)
	S_punt ("Out of memory");
    new_mcb->pnode = pnode;

    /* For no MCB or perfect, don't allocate MCB structures */
    if ((S_MCB_model == MCB_MODEL_NO_MCB) || 
	(S_MCB_model == MCB_MODEL_PERFECT) ||
	(S_MCB_model == MCB_MODEL_ALWAYS_CONFLICT))
    {
	new_mcb->conflict = NULL;
	new_mcb->reg_entry = NULL;
	new_mcb->line = NULL;
	new_mcb->line_index_mask = 0;
	new_mcb->index_size = 0;
	new_mcb->last_flushed = NULL;
	new_mcb->context_switched = 0;
    }
    else
    {
	/* Make sure size and assoc reasonable */
	if (S_MCB_size <= 0)
	    S_punt ("S_create_MCB: invalid MCB_size %i", S_MCB_size);

	if (S_MCB_assoc < 0)
	    S_punt ("S_create_MCB: invalid MCB_assoc %i", S_MCB_assoc);
	
	/* Calculate number of MCB lines needed */
	num_lines = S_MCB_size / S_MCB_assoc;

	/* Make sure it divides evenly */
	if ((num_lines * S_MCB_assoc) != S_MCB_size)
	    S_punt("S_create_MCB: MCB_assoc %i must divide MCB_size %i evenly",
		   S_MCB_assoc, S_MCB_size);

	/* Make sure number of lines is a power of two */
	if (!S_is_power_of_two (num_lines))
	    S_punt ("MCB_size(%i)/MCB_assoc(%i) must be power of two", 
		    S_MCB_size, S_MCB_assoc);

	/* Mask for line index is just lines - 1*/
	new_mcb->line_index_mask = num_lines - 1;

	/* Number of index bits is just log2 of num_lines */
	new_mcb->index_size = S_log_base_two (num_lines);

	/* Make sure checksum_width >= 0 and <= (32 - 3 -index_size) */
	if (S_MCB_checksum_width < 0)
	    S_punt ("MCB_checksum_width (%i) must be >= 0",
		    S_MCB_checksum_width);

	/* If checksum width too big, force to largest useful size */
	if (S_MCB_checksum_width > ((32 - 3) - new_mcb->index_size))
	    S_MCB_checksum_width = ((32 - 3) - new_mcb->index_size);
	
	/* Build checksum mask, shift in a 1 for each bit in mask  */
	new_mcb->checksum_mask = 0;
	for (i=0; i < S_MCB_checksum_width; i++)
	    new_mcb->checksum_mask = (new_mcb->checksum_mask << 1) | 1;
	
	/* Get domain of hash size, just enough bits for the index and
	 * the checksum
	 */
	new_mcb->hash_size = num_lines * (1 << S_MCB_checksum_width);

	/* Create conflict array, one entry per register */
	if ((new_mcb->conflict = 
	     (int *)malloc((S_max_register_operand+1) * sizeof(int))) == NULL)
	    S_punt ("Out of memory");

	/* Initialize conflict array with no conflicts */
	for (i=0; i <= S_max_register_operand; i++)
	    new_mcb->conflict[i] = MCB_NO_CONFLICT;

	/* Create reg_entry array, one entry per register */
	if ((new_mcb->reg_entry = (MCB_Entry **)malloc((S_max_register_operand+1) * 
						sizeof(MCB_Entry *))) == NULL)

	/* Initialize reg_entry array with no entries */
	for (i=0; i <= S_max_register_operand; i++)
	    new_mcb->reg_entry[i] = NULL;

	/* Create line structures */
	if ((new_mcb->line = 
	     (MCB_Line *) malloc(num_lines * sizeof(MCB_Line))) == NULL)
	    S_punt ("Out of memory");

	/* Create entries for each line */
	for (j = 0; j <= num_lines; j++)
	{
	    line = &new_mcb->line[j];
	    line->head = NULL;
	    line->tail = NULL;

	    /* Create assoc entries for this line */
	    for (i=0; i < S_MCB_assoc; i++)
	    {
		if ((entry = (MCB_Entry *)malloc(sizeof(MCB_Entry))) == NULL)
		    S_punt ("Out of memory");

		/* Init entry */
		entry->valid = 0;
		entry->reg = 0;
		entry->access_size = 0;
		entry->low_bits = 0;
		entry->checksum = 0;
		entry->addr = 0;
		
		/* Add to head of line */
		if (line->head == NULL)
		    line->tail = entry;
		else
		    line->head->prev = entry;
		
		entry->next = line->head;
		entry->prev = NULL;
		line->head = entry;
	    }
	}

	/* For debug, set oper that last flushed MCB to NULL */
	new_mcb->last_flushed = NULL;
	new_mcb->context_switched = 0;
    }

    return (new_mcb);
}

void S_sim_MCB_context_switch (Pnode *pnode, int pc)
{
    MCB *mcb;
    MCB_Entry *entry;
    int i, max_line;

    /* Don't do for Perfect MCB */
    if ((S_MCB_model == MCB_MODEL_PERFECT) ||
	(S_MCB_model == MCB_MODEL_ALWAYS_CONFLICT))
	return;

    /* get mcb for ease of use */
    mcb = pnode->mcb;
    max_line = mcb->line_index_mask;
    
    /* 
     * MCB has bad state after a context switch, so clear all valid
     * bits in MCB and set all conflict bits in register file.
     */
    for (i=0; i<= max_line; i++)
    {
	for (entry = mcb->line[i].head; entry != NULL; 
	     entry = entry->next)
	    entry->valid = 0;
    }

    /* Initialize conflict array with context switch conflicts flag */
    for (i=0; i <= S_max_register_operand; i++)
	mcb->conflict[i] = MCB_CONTEXT_SWITCH_CONFLICT;

    /* Initialize reg_entry array to NULL */
    for (i=0; i <= S_max_register_operand; i++)
	mcb->reg_entry[i] = NULL;

    /* For debugging purposes, set last flushed MCB to where the
     * context switch entered.  Set context switch flag
     */
    mcb->last_flushed = oper_tab[pc];
    mcb->context_switched = 1;

}

void S_sim_MCB_taken_branch (pnode, sint)
Pnode *pnode;
Sint *sint;
{
    MCB *mcb;
    MCB_Entry *entry;
    int i, max_line;

    /* Don't do for Perfect MCB */
    if ((S_MCB_model == MCB_MODEL_PERFECT) ||
	(S_MCB_model == MCB_MODEL_ALWAYS_CONFLICT))
	return;

    /* get mcb for ease of use */
    mcb = pnode->mcb;
    max_line = mcb->line_index_mask;
    
    /* Clear all valid bits in MCB */
    for (i=0; i<= max_line; i++)
    {
	for (entry = mcb->line[i].head; entry != NULL; 
	     entry = entry->next)
	    entry->valid = 0;
    }

    /* For debugging purposes, save oper that last flushed MCB,
     * and reset context switch flag since not due to a context switch
     */
    mcb->last_flushed = sint->oper;
    mcb->context_switched = 0;
}

static int xor_bits[32];

/* Converts the string representation of a binary number to a integer */
static int str_to_bin (string)
char *string;
{
    char *end_ptr;
    int bin;

    bin = strtol (string, &end_ptr, 2);

    if (*end_ptr != 0)
	S_punt ("str_to_bin: invalid binary number '%s'", string);

    return (bin);
}

static void MCB_hash (mcb, raw_addr, line_no, checksum)
MCB *mcb;
unsigned raw_addr;
unsigned *line_no;
unsigned *checksum;
{
    static double knuth_const = 0.6180339887777777777;
    static int reg_init = 0;
    unsigned hash = 0, inttemp;
    double fphash, fpaddr, fptemp;
    unsigned addr, shifted_addr, i;

    /* Do not want to hash lower 3 bits so shift them out */
    addr = raw_addr >> 3;

    if (S_MCB_model == MCB_MODEL_SIMPLE_HASH)
    {
	/* Simple model, just use address */
	hash = addr;
    }
    else if (S_MCB_model == MCB_MODEL_KNUTH_HASH)
    {
	fpaddr = addr;
	fptemp = fpaddr * knuth_const;
	inttemp = fptemp;
	fphash = (fptemp - inttemp) * mcb->hash_size;
	hash = fphash;
    }
    else if (S_MCB_model == MCB_MODEL_REG_HASH)
    {
	/* Initialize xor_bits table if not initialized */
	if (!reg_init)
	{
	    /* Flag initialized */
	    reg_init = 1;
#if 0
	    /* Set all the xor bits for for each address bit */
	    xor_bits[0]  = str_to_bin("00000000000000000000010111101010");
	    xor_bits[1]  = str_to_bin("00000000000000000000010010110101");
	    xor_bits[2]  = str_to_bin("00000000000000000000011101100011");
	    xor_bits[3]  = str_to_bin("00000000000000000000011100010110");
	    xor_bits[4]  = str_to_bin("00000000000000000000000101101100");
	    xor_bits[5]  = str_to_bin("00000000000000000000101001010111");
	    xor_bits[6]  = str_to_bin("00000000000000000000100110011010");
	    xor_bits[7]  = str_to_bin("00000000000000000000101010011100");
	    xor_bits[8]  = str_to_bin("00000000000000000000100011100011");
	    xor_bits[9]  = str_to_bin("00000000000000000000111001111001");
	    xor_bits[10] = str_to_bin("00000000000000000000010111001100");
	    xor_bits[11] = str_to_bin("00000000000000000000101110010101");
	    xor_bits[12] = str_to_bin("00000000000000000001000000000000");
	    xor_bits[13] = str_to_bin("00000000000000000010000000000000");
	    xor_bits[14] = str_to_bin("00000000000000000100000000000000");
	    xor_bits[15] = str_to_bin("00000000000000001000000000000000");
	    xor_bits[16] = str_to_bin("00000000000000010000000000000000");
	    xor_bits[17] = str_to_bin("00000000000000100000000000000000");
	    xor_bits[18] = str_to_bin("00000000000001000000000000000000");
	    xor_bits[19] = str_to_bin("00000000000010000000000000000000");
	    xor_bits[20] = str_to_bin("00000000000100000000000000000000");
	    xor_bits[21] = str_to_bin("00000000001000000000000000000000");
	    xor_bits[22] = str_to_bin("00000000010000000000000000000000");
	    xor_bits[23] = str_to_bin("00000000100000000000000000000000");
	    xor_bits[24] = str_to_bin("00000001000000000000000000000000");
	    xor_bits[25] = str_to_bin("00000010000000000000000000000000");
	    xor_bits[26] = str_to_bin("00000100000000000000000000000000");
	    xor_bits[27] = str_to_bin("00001000000000000000000000000000");
	    xor_bits[28] = str_to_bin("00010000000000000000000000000000");
	    xor_bits[29] = str_to_bin("00100000000000000000000000000000");
	    xor_bits[30] = str_to_bin("01000000000000000000000000000000");
	    xor_bits[31] = str_to_bin("10000000000000000000000000000000");
#endif
	    /* Set all the xor bits for for each address bit */
	    xor_bits[0]  = str_to_bin("00000000000000000000010111101010");
	    xor_bits[1]  = str_to_bin("00000000000000000000010010110101");
	    xor_bits[2]  = str_to_bin("00000000000000000000011101100011");
	    xor_bits[3]  = str_to_bin("00000000000000000000011100010110");
	    xor_bits[4]  = str_to_bin("00000000000000000000000101101100");
	    xor_bits[5]  = str_to_bin("00000000000000000000101001010111");
	    xor_bits[6]  = str_to_bin("00000000000000000000100110011010");
	    xor_bits[7]  = str_to_bin("00000000000000000000101010011100");
	    xor_bits[8]  = str_to_bin("00000000000000000000100011100011");
	    xor_bits[9]  = str_to_bin("00000000000000000000111001111001");
	    xor_bits[10] = str_to_bin("00000000000000000000010111001100");
	    xor_bits[11] = str_to_bin("00000000000000000000101110010101");
	    xor_bits[12] = str_to_bin("01101000110101110111000111000001");
	    xor_bits[13] = str_to_bin("00011011001000110010011101111100");
	    xor_bits[14] = str_to_bin("01000000011110111101100001111001");
	    xor_bits[15] = str_to_bin("01110101001100011011101111000100");
	    xor_bits[16] = str_to_bin("01010101110110101111011011000110");
	    xor_bits[17] = str_to_bin("01001010100011001011000111000100");
	    xor_bits[18] = str_to_bin("01001011101011010111101100101000");
	    xor_bits[19] = str_to_bin("01110010110101000000000011011101");
	    xor_bits[20] = str_to_bin("01100010101110010011100000100100");
	    xor_bits[21] = str_to_bin("00111110000111010110111111110110");
	    xor_bits[22] = str_to_bin("01100101101100000101001000110001");
	    xor_bits[23] = str_to_bin("00000111111011100111100100001000");
	    xor_bits[24] = str_to_bin("00011110001100101101100111000011");
	    xor_bits[25] = str_to_bin("01001001101100110001100001011010");
	    xor_bits[26] = str_to_bin("01110101101001100111101011011000");
	    xor_bits[27] = str_to_bin("00110100001100111010011010011100");
	    xor_bits[28] = str_to_bin("01110111100000000000110100101000");
	    xor_bits[29] = str_to_bin("01100111011000011101111111000011");
	    xor_bits[30] = str_to_bin("01100101101001100000000101001100");
	    xor_bits[31] = str_to_bin("01001111110101100101010011110000");
	}

	/* Set hash to zero before start XORING bits */
	hash = 0;

	/* Set temp that we will shift to test all the bits */
	shifted_addr = addr;

	/* Xor bits depending on what bits are set in address */
	for (i=0; i < 32; i++)
	{
	    if (shifted_addr & 1)
		hash ^= xor_bits[i];

	    /* Shift address one bit to right */
	    shifted_addr >>= 1;
	}
    }
    else
    {
	S_punt ("MCB_hash: unsupported model '%s'", S_MCB_model_name);
    }
    
    /* Set line_no using lower bits, checksum using higer bits,
     * Use masks to get desired number of bits
     */
    *line_no = hash & mcb->line_index_mask;
    *checksum = (hash >> (mcb->index_size)) & mcb->checksum_mask;
}

/* Invalidates entry associated with dest reg, if it exists */
void S_sim_MCB_reg_conflict(MCB *mcb, MCB_Stats *mstats, int reg)
{
    MCB_Line *line;
    MCB_Entry *entry;
    unsigned line_no, checksum;

    entry = mcb->reg_entry[reg];

    if (!entry) return;

    /* Update stats */
    mstats->load_load_signals++;

    /* Invalidate entry, put at end of list */
    entry->valid = 0;

    /* Put invalid entry at end of list so it will be reused,
     * if not at end of list already
     */
    if (entry->next != NULL)
    {
	/* Hash load address into line_no and checksum*/
	MCB_hash (mcb, entry->addr, &line_no, &checksum);
    
	/* Get line this accesses */
	line = &mcb->line[line_no];

	/* Remove from list */
	entry->next->prev = entry->prev;
	if (entry->prev == NULL)
	    line->head = entry->next;
	else
	    entry->prev->next = entry->next;

	/* Add to end of list */
	entry->next = NULL;
	entry->prev = line->tail;
	line->tail->next = entry;
	line->tail = entry;
    }

    /* reset reg_entry to NULL */
    mcb->reg_entry[reg] = NULL;
}

void S_sim_MCB_load (pnode, sint)
Pnode *pnode;
Sint *sint;
{
    MCB *mcb;
    MCB_Line *line;
    MCB_Entry *entry;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler
     *              warning. */
#if 0
    MCB_Entry *entry2;
#endif
    MCB_Stats *mstats;
    unsigned line_no, checksum;

    /* Do nothing for perfect model */
    if ((S_MCB_model == MCB_MODEL_PERFECT) ||
	(S_MCB_model == MCB_MODEL_ALWAYS_CONFLICT))
	return;

    /* Get mcb for ease of use */
    mcb = pnode->mcb;

    /* Get MCB stats for ease of use */
    mstats = sint->stats->mcb;

    /* Hash load address into line_no and checksum*/
    MCB_hash (mcb, sint->trace.mem_addr, &line_no, &checksum);
    
    /* Get line this accesses */
    line = &mcb->line[line_no];

    /* Get "oldest" entry for the load's data */
    entry = line->tail;

    /* Make "oldest" entry the "newest" entry, if assoc > 1 */
    if (S_MCB_assoc > 1)
    {
	/* Remove from tail */
	line->tail = entry->prev;
	entry->prev->next = NULL;
	
	/* Add to head */
	line->head->prev = entry;
	entry->next = line->head;
	entry->prev = NULL;
	line->head = entry;
    }

    /* If entry is valid, have load-load conflict */
    if (entry->valid)
    {
	/* Mark register in entry as having load-load conflict */
	mcb->conflict[entry->reg] = MCB_LOAD_LOAD_CONFLICT;

	/* Update stats */
	mstats->load_load_signals++;

	/* If requested, print out debug messages */
	if (S_MCB_debug_load_load_conflicts)
	{
	    fprintf (debug_out, 
	        "%s: ld op %i addr %x size %i ld op %i addr %x size %i(%s)\n",
		sint->fn->name, sint->oper->lcode_id, sint->trace.mem_addr,
		opc_info_tab[sint->oper->opc].access_size,
		entry->oper->lcode_id, entry->addr, entry->access_size, 
		operand_tab[entry->reg]->string);
	}
    }
    
    /* Put this load's info in entry */
    entry->valid = 1;
    entry->reg = sint->oper->operand[S_first_dest];
    /* Make sure it is > 0 (must be register) */
    if (entry->reg <= 0)
	S_punt ("S_sim_MCB_load: Register destination expected");
    entry->access_size = opc_info_tab[sint->oper->opc].access_size;
    entry->low_bits = sint->trace.mem_addr & 0x7;
    entry->addr = sint->trace.mem_addr; /* Debug */
    entry->oper = sint->oper; /* Debug */
    entry->checksum = checksum;

    /* Reset this load's dest register conflict flag */
    mcb->conflict[entry->reg] = MCB_NO_CONFLICT;

    /* If the processor model is PLAYDOH, then we want to clear
     * all entries with matching dest registers.
     */
    if (S_processor_model == PROCESSOR_MODEL_PLAYDOH_VLIW)
    {
	S_sim_MCB_reg_conflict(mcb, mstats, entry->reg);
    }

    mcb->reg_entry[entry->reg] = entry;
}

int S_sim_MCB_beq (pnode, sint)
Pnode *pnode;
Sint *sint;
{
    MCB_Stats *mstats;
    int reg;
    int conflict;


    /* Don't want perfect/always conflict numbers to effect false_conflict 
     * stats
     */
    if (S_MCB_model == MCB_MODEL_PERFECT)
	return (sint->flags & BRANCHED);

    else if (S_MCB_model == MCB_MODEL_ALWAYS_CONFLICT)
	return (1);

    /* Get MCB stats structure for ease of use. */
    mstats = sint->stats->mcb;

    /* Get register to do check on from beq*/
    reg = sint->oper->operand[S_first_src + 1];
    
    /* Sanity check, must be > 0 */
    if (reg <= 0)
	S_punt ("S_sim_MCB_beq: expect src[1] to be register");

    /* Get conflict flag based on register */
    conflict = pnode->mcb->conflict[reg];

    /* Did it have a true conflict? */
    if (sint->flags & BRANCHED)
    {
	/* Better simulate a conflict if have true conflict */
	if (conflict == MCB_NO_CONFLICT)
	{
	    fprintf (stderr, "%s cb %i:\n", sint->oper->cb->fn->name,
		     sint->oper->cb->lcode_id);
	    S_print_sint (stderr, sint);
	    S_print_mcb (stderr, pnode->mcb);
	    S_punt ("S_sim_MCB_beq:No conflict detected for true conflict");
	}
    }
    /* Did it have a false conflict? */
    else if (conflict != MCB_NO_CONFLICT)
    {
	/* Yes, update false conflict stats */
	if (conflict == MCB_LOAD_LOAD_CONFLICT)
	{
	    mstats->load_load_conflicts++;
	}
	else if (conflict == MCB_LOAD_STORE_CONFLICT)
	{
	    mstats->load_store_conflicts++;
	}
	else if (conflict == MCB_CONTEXT_SWITCH_CONFLICT)
	{
	    /* Update stats, but for now ignore these conflicts */
	    mstats->context_switch_conflicts++;
	    conflict = MCB_NO_CONFLICT;
	}
	else
	    S_punt ("S_sim_MCB_beq: Unknown conflict flag %i", conflict);
    }
    
    /* Return simulation's conflict bit */
    if (conflict == MCB_NO_CONFLICT)
	return (0);
    else
	return (1);
}

void S_sim_MCB_store (pnode, sint)
Pnode *pnode;
Sint *sint;
{
    MCB *mcb;
    MCB_Line *line;
    MCB_Entry *entry, *prev_entry;
    MCB_Stats *mstats;
    /* 10/25/04 REK Commenting out unused variable to quiet compiler
     *              warning. */
#if 0
    unsigned hash;
#endif
    unsigned line_no, checksum;
    int store_size, max_size, store_low_bits;
    int conflict;


    /* Don't do anything for perfect or always conflict model */
    if ((S_MCB_model == MCB_MODEL_PERFECT) ||
	(S_MCB_model == MCB_MODEL_ALWAYS_CONFLICT))
	return;

    /* Get mcb for ease of use */
    mcb = pnode->mcb;

    /* Get stats structure for ease of use */
    mstats = sint->stats->mcb;

    /* Hash store address into line number and checksum */
    MCB_hash (mcb, sint->trace.mem_addr, &line_no, &checksum);

    /* Get line this accesses */
    line = &mcb->line[line_no];

    /* Get size of store */
    store_size = opc_info_tab[sint->oper->opc].access_size;

    /* Get low bits of store */
    store_low_bits = sint->trace.mem_addr & 0x7;

    /* Go through each entry on this line looking for conflicts,
     * Do in reverse order so can invalidate and put at end of
     * list as we go
     */
    for (entry=line->tail; entry != NULL; entry = prev_entry)
    {
	/* Get previous entry before we do anything */
	prev_entry = entry->prev;

	/* 
	 * Skip entrys that are not valid or checksum bits do
	 * not match that of store address.
	 */
	if (!(entry->valid) || (entry->checksum != checksum))
	    continue;

	/* Get max access size of store and entry's load */
	if (store_size >= entry->access_size)
	    max_size = store_size;
	else
	    max_size = entry->access_size;

	/* Test for conflict */
	conflict = 1;	/* Assume conflict */
	switch (max_size)
	{
	  case 8:
	    /* Always conflict */
	    break;

	  case 4:
	    /* No conflict if bit 2 differ */
	    if ((store_low_bits ^ entry->low_bits) & 0x4)
		conflict = 0;
	    break;

	  case 2:
	    /* No conflict if bits 1 & 2 differ */
	    if ((store_low_bits ^ entry->low_bits) & 0x6)
		conflict = 0;
	    break;
	  case 1:
	    /* No conflict if bits 0:2 differ */
	    if (store_low_bits != entry->low_bits)
		conflict = 0;
	    break;
	  default:
	    S_punt ("S_MCB_store: invalid max size %i", max_size);
	}
	    
	/* Handle conflict if occurs */
	if (conflict)
	{
	    /* Mark register in entry as have load-store conflict */
	    mcb->conflict[entry->reg] = MCB_LOAD_STORE_CONFLICT;

	    /* Update stats */
	    mstats->load_store_signals++;

	    /* If requested, print out debug messages */
	    if (S_MCB_debug_load_store_conflicts)
	    {
	      fprintf (debug_out, 
		"%s: st op %i addr %x size %i ld op %i addr %x size %i(%s)\n",
		sint->fn->name, sint->oper->lcode_id, sint->trace.mem_addr,
		store_size, 
		entry->oper->lcode_id, entry->addr, entry->access_size, 
		operand_tab[entry->reg]->string);
	    }

	    /* Invalidate entry, put at end of list */
	    entry->valid = 0;

	    /* Put invalid entry at end of list so it will be reused,
	     * if not at end of list already
	     */
	    if (entry->next != NULL)
	    {
		/* Remove from list */
		entry->next->prev = entry->prev;
		if (entry->prev == NULL)
		    line->head = entry->next;
		else
		    entry->prev->next = entry->next;

		/* Add to end of list */
		entry->next = NULL;
		entry->prev = line->tail;
		line->tail->next = entry;
		line->tail = entry;
	    }

	    /* reset reg_entry to NULL */
	    mcb->reg_entry[entry->reg] = NULL;
	}
    }
}

int S_print_mcb (out, mcb)
FILE *out;
MCB *mcb;
{
    MCB_Entry *entry;
    int i;

    fprintf (out, "Last op to flush MCB: ");
    if (mcb->last_flushed == NULL)
	fprintf (out, "Startup.\n");
    else
    {
	fprintf (out, "%s op %i", mcb->last_flushed->cb->fn->name,
		 mcb->last_flushed->lcode_id);
	if (mcb->context_switched)
	    fprintf (out, " (context switch).\n");
	else
	    fprintf (out, ".\n");
    }

    fprintf (out, "MCB (%i by %i): \n",
	     S_MCB_size / S_MCB_assoc, S_MCB_assoc);
    for (i=0; i <= mcb->line_index_mask; i++)
    {
	fprintf (out,"%3i: ", i);
	for (entry = mcb->line[i].head; entry != NULL; entry = entry->next)
	{
	    if (entry->valid)
	    {
		fprintf (out, "(r%i %i %8x op %i) ", entry->reg,
			 entry->access_size, entry->addr,
			 entry->oper->lcode_id);
	    }

	}
	fprintf (out, "\n");
    }
    fprintf (out, "\n");
}

int S_sim_MCB_verify (pnode, sint)
Pnode *pnode;
Sint *sint;
{
    MCB_Stats *mstats;
    int reg;
    int conflict;
    MCB_Entry *entry;
    MCB *mcb;

    /* Don't want perfect/always conflict numbers to effect false_conflict 
     * stats
     */
    if (S_MCB_model == MCB_MODEL_PERFECT)
	return (0);

    else if (S_MCB_model == MCB_MODEL_ALWAYS_CONFLICT)
	return (1);

    /* Get mcb for ease of use */
    mcb = pnode->mcb;

    /* Get MCB stats structure for ease of use. */
    mstats = sint->stats->mcb;

    /* Get register to do check on from beq*/
    reg = sint->oper->operand[S_first_dest];
    
    /* Sanity check, must be > 0 */
    if (reg <= 0)
	S_punt ("S_sim_MCB_verify: expect src[1] to be register");

    /* Get conflict flag based on register */
    conflict = pnode->mcb->conflict[reg];

    /* Get entry associated with this reg */
    entry = pnode->mcb->reg_entry[reg];

    /* Sanity check - address of entry should match that of this ldv sint */
    if (entry && entry->addr != sint->trace.mem_addr)
	S_punt ("S_sim_MCB_verify: Addresses of reg entry and ldv do not match");

    /* Did it have a conflict? */
    if (conflict == MCB_NO_CONFLICT)
    {
	/* Better have an entry */
	if (!entry)
	{
	    fprintf (stderr, "%d: %s cb %i:\n", S_sim_cycle,
			sint->oper->cb->fn->name,
		     sint->oper->cb->lcode_id);
	    S_print_sint (stderr, sint);
	    S_print_mcb (stderr, pnode->mcb);
	    S_punt ("S_sim_MCB_verify: No conflict detected for reg but no matching entry");
	}
    }
    else if (conflict != MCB_NO_CONFLICT)
    {
	/* Better not have an entry */
	if (entry)
	{
	    fprintf (stderr, "%s cb %i:\n", sint->oper->cb->fn->name,
		     sint->oper->cb->lcode_id);
	    S_print_sint (stderr, sint);
	    S_print_mcb (stderr, pnode->mcb);
	    S_punt ("S_sim_MCB_verify: Conflict detected for reg with matching entry");
	}
	/* Yes, update conflict stats */
	if (conflict == MCB_LOAD_LOAD_CONFLICT)
	{
	    mstats->load_load_conflicts++;
	}
	else if (conflict == MCB_LOAD_STORE_CONFLICT)
	{
	    mstats->load_store_conflicts++;
	}
	else if (conflict == MCB_CONTEXT_SWITCH_CONFLICT)
	{
	    /* Update stats, but for now ignore these conflicts */
	    mstats->context_switch_conflicts++;
	    conflict = MCB_NO_CONFLICT;
	}
	else
	    S_punt ("S_sim_MCB_verify: Unknown conflict flag %i", conflict);
    }

    /* Invalidate this entry */
    S_sim_MCB_reg_conflict(mcb, mstats, reg);
    
    /* Return simulation's conflict bit */
    if (conflict == MCB_NO_CONFLICT)
	return (0);
    else
	return (1);
}
