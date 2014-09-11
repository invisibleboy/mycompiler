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
 *      Author: Erik Nystrom
 *      Creation Date:  1999
 *      Copyright (c) 1999 Erik Nystrom, Wen-mei Hwu and The Board of
 *                         Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
"@(#) Copyright (c) 1999 Erik Nystrom, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

/* Stub file for s_mcb.c */
#include "s_main.h"


void S_read_parm_ALAT (Parm_Parse_Info *ppi)
{
    L_read_parm_s (ppi, "ALAT_model", &S_ALAT_model_name);
    L_read_parm_i (ppi, "ALAT_size", &S_ALAT_size);
    L_read_parm_b (ppi, "ALAT_all_loads_preloads", 
		   &S_ALAT_all_loads_preloads);
    L_read_parm_b (ppi, "ALAT_debug_load_load_conflicts",
		   &S_ALAT_debug_load_load_conflicts);
    L_read_parm_b (ppi, "ALAT_debug_load_store_conflicts",
		   &S_ALAT_debug_load_store_conflicts);
}

void S_print_configuration_ALAT (FILE *out)
{
    fprintf (out, "# ALAT CONFIGURATION:\n");
    fprintf (out, "\n");
    fprintf (out, "%12s ALAT model.\n", S_ALAT_model_name);
    if ((S_ALAT_model != ALAT_MODEL_NO_ALAT) &&
	(S_ALAT_model != ALAT_MODEL_PERFECT))
    {
	fprintf (out, "%12u ALAT size.\n",S_ALAT_size);
	if (S_ALAT_all_loads_preloads)
	  fprintf (out, "%12s ", "All");
	else
	  fprintf (out, "%12s ", "ALAT");
	fprintf (out, "loads are treated as ALAT preloads.\n");
    }
    fprintf (out, "\n");
}


void S_read_ALAT_table(char *filename, ALAT *alat)
{
  FILE *file;
  char namebuffer[200];
  int cb_id, ld_id, st_cnt, benefit_id, penalty_id;
  ALAT_table_entry *entry;
  int ALAT_SymbolTable = NewSymTbl( 24019 );
  int ALAT_OpSymbolTable = NewSymTbl( 24019 );
  Symbol s;
  int cnt = 0;

  file = fopen(filename,"r");

  if (!file)
    S_punt("S_read_ALAT_table: could not open %s",
	   filename);
  
  while (6 == fscanf(file,"fn %s cb %d ld %d : %d %d %d\n",
		     namebuffer, &cb_id, &ld_id, &st_cnt,
		     &benefit_id, &penalty_id))
    {
      printf("ALAT TABLE: %s %d %d %d %d\n",
	     namebuffer, ld_id, st_cnt,
	     benefit_id, penalty_id);	     

      entry = (ALAT_table_entry*)malloc(sizeof(ALAT_table_entry));
      entry->func = strdup(namebuffer);
      entry->ld_id = ld_id;
      entry->st_cnt = st_cnt;
      entry->benefit_id = benefit_id;
      entry->penalty_id = penalty_id;
      entry->penalty = 0;
      entry->benefit = -1;
      entry->last_exec = -1;
      entry->nxt = NULL;

      s = FindSym(ALAT_SymbolTable, entry->func, ld_id );
      if (s)
	S_punt("Duplicate ALAT ld symbol %s %d", 
	       entry->func, ld_id);
      else
	{
	  s = AddSym (ALAT_SymbolTable, entry->func, ld_id );
	  s->ptr = entry;

	  s = FindSym(ALAT_SymbolTable, namebuffer, ld_id );
	  if (!s)
	    S_punt("Error adding ALAT ld symbol %s %d", 
		   entry->func, ld_id);
	}

      s = FindSym(ALAT_OpSymbolTable, entry->func, benefit_id );
      if (!s)
	{
	  s = AddSym (ALAT_OpSymbolTable, entry->func, benefit_id );
	  s->ptr = entry;
	}
      else
	{
	  printf("Shared benefit id %d\n",benefit_id);
	  entry->nxt = ((ALAT_table_entry*)(s->ptr))->nxt;
	  s->ptr = entry;
	}

      s = FindSym(ALAT_OpSymbolTable, entry->func, penalty_id );
      if (!s)
	{
	  s = AddSym (ALAT_OpSymbolTable, entry->func, penalty_id );
	  s->ptr = entry;
	}
      else
	{
	  printf("Shared penalty id %d\n",penalty_id);
	  entry->nxt = ((ALAT_table_entry*)(s->ptr))->nxt;
	  s->ptr = entry;
	}
      cnt++;
    }

  alat->symboltable = ALAT_SymbolTable;
  alat->opsymboltable = ALAT_OpSymbolTable;
}


ALAT *S_create_ALAT (pnode)
Pnode *pnode;
{
    ALAT *new_alat;
    ALAT_Entry *entry;
    ALAT_Line *line;
    int num_lines;
    int j, i;

    if (S_MCB_model != MCB_MODEL_NO_MCB)
      S_punt("Both MCB and ALAT cannot be active");

    if (L_pmatch (S_ALAT_model_name, "No-ALAT"))
    {
	/* Reassign name to beautify simulation output */
	S_ALAT_model_name = "No-ALAT";
	S_ALAT_model = ALAT_MODEL_NO_ALAT;
    }
    else if (L_pmatch (S_ALAT_model_name, "perfect"))
    {
	/* Reassign name to beautify simulation output */
	S_ALAT_model_name = "Perfect";
	S_ALAT_model = ALAT_MODEL_PERFECT;
    }
    else if (L_pmatch (S_ALAT_model_name, "always-conflict"))
    {
	/* Reassign name to beautify simulation output */
	S_ALAT_model_name = "Always-conflict";
	S_ALAT_model = ALAT_MODEL_ALWAYS_CONFLICT;
    }
    else if (L_pmatch (S_ALAT_model_name, "simple"))
    {
	/* Reassign name to beautify simulation output */
	S_ALAT_model_name = "Simple";
	S_ALAT_model = ALAT_MODEL_SIMPLE_HASH;
    }
    else
      S_punt ("S_create_ALAT: Illegal ALAT model '%s'", S_ALAT_model_name);

    /* Create ALAT */
    if ((new_alat = (ALAT *) malloc (sizeof(ALAT))) == NULL)
	S_punt ("Out of memory");
    new_alat->pnode = pnode;

    /* For no ALAT or perfect, don't allocate ALAT structures */
    if ((S_ALAT_model == ALAT_MODEL_NO_ALAT) || 
	(S_ALAT_model == ALAT_MODEL_PERFECT) ||
	(S_ALAT_model == ALAT_MODEL_ALWAYS_CONFLICT))
    {
	new_alat->conflict = NULL;
	new_alat->line = NULL;
	new_alat->last_flushed = NULL;
	new_alat->context_switched = 0;
    }
    else
    {
	/* Make sure size and assoc reasonable */
	if (S_ALAT_size <= 0)
	    S_punt ("S_create_ALAT: invalid ALAT_size %i", S_ALAT_size);

	/* Calculate number of ALAT lines needed */
	num_lines = S_ALAT_size;

	/* Make sure number of lines is a power of two */
	if (!S_is_power_of_two (num_lines))
	    S_punt ("ALAT_size(%i) must be power of two", 
		    S_ALAT_size);

	/* Mask for line index is just lines - 1*/
	new_alat->line_index_mask = num_lines - 1;

	/* Number of index bits is just log2 of num_lines */
	new_alat->index_size = S_log_base_two (num_lines);

	/* Create conflict array, one entry per register */
	if ((new_alat->conflict = 
	     (int *)malloc((S_max_register_operand+1) * sizeof(int))) == NULL)
	    S_punt ("Out of memory");

	/* Initialize conflict array with no conflicts */
	for (i=0; i <= S_max_register_operand; i++)
	    new_alat->conflict[i] = ALAT_NO_CONFLICT;

	/* Create line structures */
	if ((new_alat->line = 
	     (ALAT_Line *) malloc(num_lines * sizeof(ALAT_Line))) == NULL)
	  S_punt ("Out of memory");

	/* Create entries for each line */
	for (j = 0; j <= num_lines; j++)
	{
	    line = &new_alat->line[j];
	    line->head = NULL;
	    line->tail = NULL;

	    if ((entry = (ALAT_Entry *)malloc(sizeof(ALAT_Entry))) == NULL)
	      S_punt ("Out of memory");

	    /* Init entry */
	    entry->valid = 0;
	    entry->regTAG = 0;
	    entry->addrTAG = 0;
	    entry->access_size = 0;
	    entry->reg = 0;
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

	/* For debug, set oper that last flushed ALAT to NULL */
	new_alat->last_flushed = NULL;
	new_alat->context_switched = 0;

	/* JWS 20000106 - don't initialize an ALAT you don't have */

	S_read_ALAT_table("sched_diff",new_alat);
	new_alat->stbuffer.position = 0;
	new_alat->bpbuffer.position = 0;

    }

    return (new_alat);
}

void S_sim_ALAT_context_switch (Pnode *pnode, int pc)
{
    ALAT *alat;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    ALAT_Entry *entry;
    int i, max_line;
#endif

#if 0
    /* Don't do for Perfect ALAT */
    if ((S_ALAT_model == ALAT_MODEL_PERFECT) ||
	(S_ALAT_model == ALAT_MODEL_ALWAYS_CONFLICT))
	return;

    /* get alat for ease of use */
    alat = pnode->alat;
    max_line = alat->line_index_mask;
    
    /* 
     * ALAT has bad state after a context switch, so clear all valid
     * bits in ALAT and set all conflict bits in register file.
     */
    for (i=0; i<= max_line; i++)
    {
      if ( alat->line[i].head != NULL )
	alat->line[i].head->valid = 0;
    }

    /* Initialize conflict array with context switch conflicts flag */
    for (i=0; i <= S_max_register_operand; i++)
	alat->conflict[i] = ALAT_CONTEXT_SWITCH_CONFLICT;
#endif

    alat = pnode->alat;
    alat->stbuffer.position = 0;
    alat->bpbuffer.position = 0;

    /* For debugging purposes, set last flushed ALAT to where the
     * context switch entered.  Set context switch flag
     */
    alat->last_flushed = oper_tab[pc];
    alat->context_switched = 1;
}


void S_sim_ALAT_taken_branch (pnode, sint)
Pnode *pnode;
Sint *sint;
{
    ALAT *alat;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    ALAT_Entry *entry;
    int i, max_line;
#endif

#if 0
    /* Don't do for Perfect ALAT */
    if ((S_ALAT_model == ALAT_MODEL_PERFECT) ||
	(S_ALAT_model == ALAT_MODEL_ALWAYS_CONFLICT))
	return;

    /* get alat for ease of use */
    alat = pnode->alat;
    max_line = alat->line_index_mask;
    
    /* Clear all valid bits in ALAT */
    for (i=0; i<= max_line; i++)
    {
      if ((entry = alat->line[i].head) != NULL)
	entry->valid = 0;
    }
#endif

    alat = pnode->alat;
    alat->stbuffer.position = 0;
    alat->bpbuffer.position = 0;

    /* For debugging purposes, save oper that last flushed ALAT,
     * and reset context switch flag since not due to a context switch
     */
    alat->last_flushed = sint->oper;
    alat->context_switched = 0;
}


void ALAT_hash(ALAT *alat, unsigned addr, int reg, unsigned *line_no)
{
  int reg_mask = S_max_register_operand-1;
  int addr_mask = ~reg_mask;

  /* Hash = [low order addr][reg] */
  (*line_no) = ((reg & reg_mask) | (addr & addr_mask)) & alat->line_index_mask;

  printf("ALAT hash reg(%d,%x) reg_mask(%d,%x) addr(%d,%x) addr_mask(%x)\n",
	 reg, reg, reg_mask, reg_mask, addr, addr, addr_mask);
  printf("     line_mask(%x)  line_no(%d,%x)\n",
	 alat->line_index_mask, *line_no, *line_no);
}

void S_sim_ALAT_all_ops (Pnode *pnode, Sint *sint, unsigned cycle)
{
  ALAT *alat;
  Symbol s;
  ALAT_table_entry *entry;
  
  alat = pnode->alat;
  s =  FindSym(alat->opsymboltable, sint->oper->cb->fn->name,
	       sint->oper->lcode_id);
  
  if (s)
    {
      entry = (ALAT_table_entry*)(s->ptr);
      if (!entry)
	S_punt("ALAT table entry null");

      while(entry)
	{
	  if (S_ALAT_debug_load_store_conflicts)
	    {
	      printf("ALAT: bp op %s %d\n",
		     sint->oper->cb->fn->name,sint->oper->lcode_id);
	    }

	  if (entry->benefit_id == sint->oper->lcode_id)
	    {
	      entry->benefit = cycle;
	    }

	  if (entry->penalty_id == sint->oper->lcode_id)
	    {
	      if (entry->penalty_id == entry->ld_id)
		entry->last_exec = cycle;

	      if (entry->last_exec > 0)
		entry->penalty = cycle - entry->last_exec;
	      
	      if (S_ALAT_debug_load_store_conflicts)
		{
		  if (entry->penalty != -1)
		    printf("op%d penalty = %d\n",entry->ld_id,entry->penalty);
		  else
		    printf("op%d skipping penalty\n",entry->ld_id);
		}
	    }

	  entry = entry->nxt;
	}
    }
}

int S_sim_ALAT_load (pnode, sint, cycle)
Pnode *pnode;
Sint *sint;
unsigned cycle;
{
    ALAT *alat;
    /*
      ALAT_Line *line;
      ALAT_Entry *entry;
      ALAT_Entry *entry2;
    */
    ALAT_Stats *mstats;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    unsigned line_no; 
    int found;
#endif
    int conflict, pos, i;
    unsigned max_size;
    unsigned store_size, store_low_bits;
    unsigned load_size, load_low_bits;
    Symbol s;
    ALAT_table_entry *entry;
    int adjust;

    alat = pnode->alat;
    mstats = sint->stats->alat;
    s =  FindSym(alat->symboltable, sint->oper->cb->fn->name,
		 sint->oper->lcode_id);

    if (!s)
      {
	/* The load is not speculative */
	return 0;
      }
      
    /*
    printf("ALAT_load: %s %d load found in symboltable\n",
	   sint->oper->cb->fn->name, sint->oper->lcode_id);
    */
    entry = (ALAT_table_entry*)s->ptr;

    entry->last_exec = cycle;
    conflict = 0;
    for ( i=0; i < entry->st_cnt; i++)
      {
	pos = alat->stbuffer.position - 1 - i;
	if (pos < 0)
	  {
	    S_punt("ALAT_load: %d stores expected for load",entry->st_cnt);
	    break;
	  }

	/* Get size of store */
	store_size = alat->stbuffer.buffer[pos].access_size;
	load_size = opc_info_tab[sint->oper->opc].access_size;

	/* Get low bits of store */
	store_low_bits =  alat->stbuffer.buffer[pos].addr & 0x7;
	load_low_bits = sint->trace.mem_addr & 0x7;

	/* Get max access size of store and entry's load */
	if (store_size >= load_size)
	  max_size = store_size;
	else
	  max_size = load_size;
	
	/* Test for conflict */
	if ((alat->stbuffer.buffer[pos].addr | 0x7) !=
	    (sint->trace.mem_addr | 0x7))
	  {
	    continue;
	  }
	conflict = 1;
	switch (max_size)
	  {
	  case 8:
	    /* Always conflict */
	    break;
	  case 4:
	    /* No conflict if bit 2 differ */
	    if ((store_low_bits ^ load_low_bits) & 0x4)
	      conflict = 0;
	    break;
	  case 2:
	    /* No conflict if bits 1 & 2 differ */
	    if ((store_low_bits ^ load_low_bits) & 0x6)
	      conflict = 0;
	    break;
	  case 1:
	    /* No conflict if bits 0:2 differ */
	    if (store_low_bits != load_low_bits)
	      conflict = 0;
	    break;
	  default:
	    S_punt ("S_ALAT_load: invalid max size %i", max_size);
	  }

	if (conflict)
	  break;
      }

    if ((conflict || (S_ALAT_model == ALAT_MODEL_ALWAYS_CONFLICT))
	&& (S_ALAT_model != ALAT_MODEL_PERFECT))
      {
	/* determine penalty */
	mstats->load_store_signals++;
	
	adjust = entry->penalty;
      }
    else
      {
	/* determine benefit */
	mstats->load_load_signals++;


	adjust = 0;
	if (entry->benefit != -1)
	  adjust = entry->benefit - cycle;
	else
	  S_punt("benefit op not found");
	if (S_ALAT_debug_load_store_conflicts)
	  {
	    printf("benefit = %d\n",adjust);
	  }
	entry->benefit = -1;
      }

    if (S_ALAT_debug_load_store_conflicts)
      {
	printf("adjust = %d\n",adjust);
      }    
    return adjust;



#if 0
    /* Do nothing for perfect model */
    if ((S_ALAT_model == ALAT_MODEL_PERFECT) ||
	(S_ALAT_model == ALAT_MODEL_ALWAYS_CONFLICT))
	return;

    /* Get alat for ease of use */
    alat = pnode->alat;

    /* Get ALAT stats for ease of use */
    mstats = sint->stats->alat;

    /* Hash load address into line_no and checksum*/
    ALAT_hash (alat, sint->trace.mem_addr, 
	       sint->oper->operand[S_first_dest], &line_no);

    /* Get line this accesses */
    line = &alat->line[line_no];

    /* Get "oldest" entry for the load's data */
    entry = line->tail;

    /* If entry is valid, have load-load conflict */
    if (entry->valid)
    {
	/* Mark register in entry as having load-load conflict */
	alat->conflict[entry->reg] = ALAT_LOAD_LOAD_CONFLICT;

	/* Update stats */
	mstats->load_load_signals++;

	/* If requested, print out debug messages */
	if (S_ALAT_debug_load_load_conflicts)
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
    entry->penalty = sint->oper->operand[S_first_src];
    /* Make sure it is > 0 (must be register) */
    if (entry->reg <= 0)
	S_punt ("S_sim_ALAT_load: Register destination expected");
    entry->access_size = opc_info_tab[sint->oper->opc].access_size;
    entry->low_bits = sint->trace.mem_addr & 0x7;
    entry->addr = sint->trace.mem_addr; /* Debug */
    entry->oper = sint->oper; /* Debug */

    /* Reset this load's dest register conflict flag */
    alat->conflict[entry->reg] = ALAT_NO_CONFLICT;

    /* If the processor model is PLAYDOH, then we want to clear
     * all entries with matching dest registers.
     */
    /*
      if (S_processor_model == PROCESSOR_MODEL_PLAYDOH_VLIW)
      {
      }
    */
#endif
}


void S_sim_ALAT_store (pnode, sint)
Pnode *pnode;
Sint *sint;
{
    ALAT *alat;
    /* 10/22/04 REK Commenting out unused variables to quiet compiler
     *          warnings. */
#if 0
    ALAT_Line *line;
    ALAT_Entry *entry, *prev_entry;
    ALAT_Stats *mstats;
    unsigned hash, line_no, checksum;
    int store_size, max_size, store_low_bits;
    int conflict;
#endif

    alat = pnode->alat;

    if (alat->stbuffer.position >= 200)
      S_punt("ALAT stbuffer exceeded");

    alat->stbuffer.buffer[alat->stbuffer.position].id = 
      sint->oper->lcode_id;
    alat->stbuffer.buffer[alat->stbuffer.position].access_size =
      opc_info_tab[sint->oper->opc].access_size;
    alat->stbuffer.buffer[alat->stbuffer.position].addr =
      sint->trace.mem_addr;
    alat->stbuffer.position++;


#if 0
    /* Don't do anything for perfect or always conflict model */
    if ((S_ALAT_model == ALAT_MODEL_PERFECT) ||
	(S_ALAT_model == ALAT_MODEL_ALWAYS_CONFLICT))
	return;

    /* Get alat for ease of use */
    alat = pnode->alat;

    /* Get stats structure for ease of use */
    mstats = sint->stats->alat;

    /* Hash store address into line number and checksum */
    ALAT_hash (alat, sint->trace.mem_addr, 
	       sint->oper->operand[S_first_dest], &line_no);

    /* Get line this accesses */
    line = &alat->line[line_no];

    /* Get size of store */
    store_size = opc_info_tab[sint->oper->opc].access_size;

    /* Get low bits of store */
    store_low_bits = sint->trace.mem_addr & 0x7;

    /* Go through each entry on this line looking for conflicts,
     * Do in reverse order so can invalidate and put at end of
     * list as we go
     */
    if ( ((entry = line->tail) != NULL)
	 && (entry->valid) )
      {
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
	    S_punt ("S_ALAT_store: invalid max size %i", max_size);
	}
	    
	/* Handle conflict if occurs */
	if (conflict)
	{
	    /* Mark register in entry as have load-store conflict */
	    alat->conflict[entry->reg] = ALAT_LOAD_STORE_CONFLICT;

	    /* Update stats */
	    mstats->load_store_signals++;

	    /* If requested, print out debug messages */
	    if (S_ALAT_debug_load_store_conflicts)
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
	}
    }
#endif
}

void S_print_alat (out, alat)
FILE *out;
ALAT *alat;
{
    ALAT_Entry *entry;
    int i;

    fprintf (out, "Last op to flush ALAT: ");
    if (alat->last_flushed == NULL)
	fprintf (out, "Startup.\n");
    else
    {
	fprintf (out, "%s op %i", alat->last_flushed->cb->fn->name,
		 alat->last_flushed->lcode_id);
	if (alat->context_switched)
	    fprintf (out, " (context switch).\n");
	else
	    fprintf (out, ".\n");
    }

    fprintf (out, "ALAT (%i by %i): \n",
	     S_ALAT_size, 1);
    for (i=0; i <= alat->line_index_mask; i++)
    {
	fprintf (out,"%3i: ", i);
	if ((entry = alat->line[i].head) != NULL) 
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

