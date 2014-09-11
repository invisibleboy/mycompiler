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
 *      File:   s_super.c
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

#include <Lcode/l_main.h>
#include "s_main.h"
#include "s_super.h"

#undef DEBUG_PROC
#undef DEBUG_IFETCH

#undef DEBUG_NEW


/* New parameters */
int S_loads_per_cycle = 8;
int S_stores_per_cycle = 8;
int S_cache_directives_per_cycle = 8;
int S_ialu_ops_per_cycle = 8;
int S_falu_ops_per_cycle = 8;
int S_squash_off_path_dcache_requests = 0;	/* -JCG 2/5/98 */
int S_delay_stores_until_retirement = 1;	/* -JCG 2/17/98 */

/* Global variables */
int S_halting_simulation;
int S_total_adjust = 0;
int S_total_unadjust_cycle = 0;

/* Alloc pools for superscalar simulation */
L_Alloc_Pool *Superscalar_pool = NULL;
L_Alloc_Pool *Superscalar_Data_pool = NULL;
L_Alloc_Pool *Superscalar_Stats_pool = NULL;
/* HCH 10-20-99 */
L_Alloc_Pool *Spec_check_pool = NULL;
Check_line **S_sload_lines = NULL;
S_Outstanding_Load *S_oload = NULL;

int S_sim_superscalar_scoreboard (Pnode * pnode, int pc, unsigned sim_count);

void
S_read_parm_superscalar (Parm_Parse_Info * ppi)
{
  L_read_parm_s (ppi, "fetch_model", &S_fetch_model_name);
  L_read_parm_i (ppi, "fetch_width", &S_fetch_width);
  L_read_parm_i (ppi, "fetch_buf_size", &S_fetch_buf_size);
  L_read_parm_i (ppi, "fetch_mark", &S_fetch_mark);
  L_read_parm_i (ppi, "issue_width", &S_issue_width);
  L_read_parm_b (ppi, "flush_pipe_on_untraced_jsr",
		 &S_flush_pipe_on_untraced_jsr);
  L_read_parm_b (ppi, "read_dests_of_pred_op", &S_read_dests_of_pred_op);
  L_read_parm_i (ppi, "branches_per_cycle", &S_branches_per_cycle);
  L_read_parm_i (ppi, "loads_per_cycle", &S_loads_per_cycle);
  L_read_parm_i (ppi, "stores_per_cycle", &S_stores_per_cycle);
  L_read_parm_i (ppi, "cache_directives_per_cycle",
		 &S_cache_directives_per_cycle);
  L_read_parm_i (ppi, "ialu_ops_per_cycle", &S_ialu_ops_per_cycle);
  L_read_parm_i (ppi, "falu_ops_per_cycle", &S_falu_ops_per_cycle);
  L_read_parm_i (ppi, "dcache_ports", &S_dcache_ports);
  L_read_parm_i (ppi, "retire_width", &S_retire_width);
  L_read_parm_b (ppi, "squash_off_path_dcache_requests",
		 &S_squash_off_path_dcache_requests);
  L_read_parm_b (ppi, "delay_stores_until_retirement",
		 &S_delay_stores_until_retirement);
}

void
S_print_configuration_superscalar (FILE * out)
{
  fprintf (out, "# SUPERSCALAR PROCESSOR CONFIGURATION:\n");
  fprintf (out, "\n");
  fprintf (out, "superscalar configuration:   %s\n",
	   "scoreboarding and register renaming");
  fprintf (out, "%12s fetch model.\n", S_fetch_model_name);
  fprintf (out, "%12u fetch stages.\n", S_num_fetch_stages);
  fprintf (out, "%12u fetch width.\n", S_fetch_width);
  fprintf (out, "%12u fetch buf size.\n", S_fetch_buf_size);
  fprintf (out, "%12u fetch mark.\n", S_fetch_mark);
  fprintf (out, "%12u issue width.\n", S_issue_width);
  fprintf (out, "%12u branches/cycle.\n", S_branches_per_cycle);
  fprintf (out, "%12u cycle branch misprediction penalty.\n",
	   S_num_fetch_stages + 1);
  fprintf (out, "%12u ialu ops/cycle.\n", S_ialu_ops_per_cycle);
  fprintf (out, "%12u falu ops/cycle.\n", S_falu_ops_per_cycle);
  fprintf (out, "%12u loads/cycle.\n", S_loads_per_cycle);
  fprintf (out, "%12u stores/cycle.\n", S_stores_per_cycle);
  fprintf (out, "%12u cache directives/cycle.\n",
	   S_cache_directives_per_cycle);
  fprintf (out, "%12u dcache ports (max loads + stores per cycles).\n",
	   S_dcache_ports);
  fprintf (out, "%12u retire width.\n", S_retire_width);
  fprintf (out, "\n");
  if (S_squash_off_path_dcache_requests)
    fprintf (out, "%12s", "DOES");
  else
    fprintf (out, "%12s", "DOES NOT");
  fprintf (out,
	   " prevent off-path requests from being sent to the dcache.\n");

  if (S_delay_stores_until_retirement)
    fprintf (out, "%12s", "DOES");
  else
    fprintf (out, "%12s", "DOES NOT");
  fprintf (out,
	   " delay stores until retirement (reorders dcache request stream).\n");

  if (S_read_dests_of_pred_op)
    fprintf (out, "%12s", "DOES");
  else
    fprintf (out, "%12s", "DOES NOT");
  fprintf (out, " read destination registers of predicated operations.\n");

  if (S_flush_pipe_on_untraced_jsr)
    {
      fprintf (out,
	       "%12s flush pipe on untraced jsr (%u cycle penalty).\n",
	       "DOES", S_num_fetch_stages + 1);
    }
  else
    {
      fprintf (out,
	       "%12s flush pipe on untraced jsr (no penalty).\n", "DOES NOT");
    }

  fprintf (out, "\n");

}

Superscalar *
S_create_superscalar (Pnode * pnode)
{
  Superscalar *processor;
  Reg_File *reg;
  int reg_size;
  int i;
  char name_buf[100];

  /* Create alloc pools, etc that need to be done first time
   * this is called.
   */
  if (Superscalar_pool == NULL)
    {
      Superscalar_pool = L_create_alloc_pool ("Superscalar",
					      sizeof (Superscalar), 1);
      Superscalar_Data_pool =
	L_create_alloc_pool ("Superscalar_Data",
			     sizeof (Superscalar_Data), 128);
    }

  processor = (Superscalar *) L_alloc (Superscalar_pool);
  processor->pnode = pnode;
  sprintf (name_buf, "Superscalar %i", pnode->id);
  processor->name = strdup (name_buf);

  /* Initialize processor state */
  processor->flags = 0;
  processor->ifetch_pc = 0;
  processor->ifetch_addr = 0;
  processor->on_correct_path = 0;
  processor->squashing = 0;

  /* Initialize register file */
  reg_size = sizeof (Reg_File) * (S_max_register_operand + 1);
  if ((processor->reg_file = (Reg_File *) malloc (reg_size)) == NULL)
    S_punt ("Out of memory");

  for (i = 0; i <= S_max_register_operand; i++)
    {
      reg = &processor->reg_file[i];

      /* Initialize decoder scoreboard fields */
      reg->tag = NULL;
      reg->value_avail = COMPLETED;
      reg->adjust = 0;
      /* Initialize value tracking fields for predicate-based prediction */
      reg->last_def_fetched = NULL;
      reg->accessible_value = 0;	/* Assume predicates 0 initially */
      reg->value_cb_id = 0;
      reg->value_fetch_cycle = 0;
      reg->value_oper = NULL;
    }

  /* Make sure the number of fetch stages is reasonable */
  if (S_num_fetch_stages < 1)
    S_punt ("At least one fetch stage is required (not %i)",
	    S_num_fetch_stages);

  /* Create an array of fetch stage queues */
  processor->fetch_stage =
    (Squeue **) malloc (S_num_fetch_stages * sizeof (Squeue *));
  if (processor->fetch_stage == NULL)
    S_punt ("Out of memory allocating %i stages", S_num_fetch_stages);

  for (i = 0; i < S_num_fetch_stages; i++)
    {
      sprintf (name_buf, "Ifetch stage (%i of %i)", i + 1,
	       S_num_fetch_stages);
      processor->fetch_stage[i] = S_create_queue (name_buf, pnode->id);
    }

  /* Alias ibuf and last_fetch_stage to the appropriate fetch stages */
  processor->ibuf = processor->fetch_stage[0];
  processor->last_fetch_stage =
    processor->fetch_stage[S_num_fetch_stages - 1];

  /*
   * Initialize and name queue.  The pnode->id will be tacked on to the
   * end of the name.
   */
  processor->reorder_queue = S_create_queue ("Reorder", pnode->id);
  processor->decode_queue = S_create_queue ("Decode", pnode->id);
  processor->exec_queue = S_create_queue ("Exec", pnode->id);

  /* 
   * Create hash queues for pending stores so that conflicts with
   * loads can be determined quickly.
   */
  for (i = 0; i < STORE_HASH_SIZE; i++)
    {
      processor->pending_stores[i] =
	S_create_queue ("Pending stores (hash entry)", pnode->id);
    }

  /* Initialize fetch model */
  if (L_pmatch (S_fetch_model_name, "aggressive"))
    {
      /* Reassign name for printout */
      S_fetch_model_name = "Aggressive";
      S_fetch_model = FETCH_MODEL_AGGRESSIVE;

      /* Make sure values reasonable */
      if (S_fetch_width < 1)
	S_punt ("S_fetch_width (%i) must be > 0.", S_fetch_width);
      if (S_fetch_buf_size < 1)
	S_punt ("S_fetch_buf_size (%i) must be > 0.", S_fetch_buf_size);
      if (S_fetch_mark < 0)
	S_punt ("S_fetch_mark (%i) must be >= 0.", S_fetch_mark);

    }

  else if (L_pmatch (S_fetch_model_name, "conservative"))
    {
      /* Reassign name for printout */
      S_fetch_model_name = "Conservative";
      S_fetch_model = FETCH_MODEL_CONSERVATIVE;

      /* Set parameters to conservative settings */
      S_fetch_width = S_issue_width;
      S_fetch_buf_size = S_issue_width;
      S_fetch_mark = 0;
    }
  else
    S_punt ("Unknown fetch model '%s'.\n", S_fetch_model_name);

  /* Used for generating scheduling stats */
  processor->cb_start_time = 0;


  /* Set pnode field for debugging purposes only */
  pnode->superscalar = processor;

  return (processor);
}

static unsigned int *
create_utilization_array (int elements)
{
  int i;
  unsigned int *array;

  if ((array = (unsigned int *) malloc (elements * sizeof (unsigned int)))
      == NULL)
    {
      S_punt ("Out of memory creating utilization array of %i elements.",
	      elements);
    }

  for (i = 0; i < elements; i++)
    array[i] = 0;

  return (array);
}

Superscalar_Stats *
S_create_stats_superscalar ()
{
  Superscalar_Stats *stats;

  /* Create stats pool on first call */
  if (Superscalar_Stats_pool == NULL)
    {
      Superscalar_Stats_pool =
	L_create_alloc_pool ("Superscalar_Stats",
			     sizeof (Superscalar_Stats), 1);
    }

  /* Create bus stats structure */
  stats = (Superscalar_Stats *) L_alloc (Superscalar_Stats_pool);

  /* Use STATS_ZERO(...) to initialize stats */
  STATS_ZERO (loads_forwarded);
  STATS_ZERO (loads_blocked);
  STATS_ZERO (cycles_loads_blocked);
  STATS_ZERO (cycles_sint_blocked_on_pending_load);
  STATS_ZERO (cycles_dcache_busy_when_needed);
  STATS_ZERO (cycles_dcache_ports_unavailable);

  STATS_ZERO (cycles_branch_unit_unavailable);
  STATS_ZERO (cycles_ialu_unit_unavailable);
  STATS_ZERO (cycles_falu_unit_unavailable);
  STATS_ZERO (cycles_load_unit_unavailable);
  STATS_ZERO (cycles_store_unit_unavailable);
  STATS_ZERO (cycles_cache_directive_unit_unavailable);

  /* Allocate and initialize allocation arrays */
  stats->issue_utilization = create_utilization_array (S_issue_width + 1);
  stats->branch_utilization =
    create_utilization_array (S_branches_per_cycle + 1);
  stats->ialu_op_utilization =
    create_utilization_array (S_ialu_ops_per_cycle + 1);
  stats->falu_op_utilization =
    create_utilization_array (S_falu_ops_per_cycle + 1);
  stats->load_utilization = create_utilization_array (S_loads_per_cycle + 1);
  stats->store_utilization =
    create_utilization_array (S_stores_per_cycle + 1);
  stats->cache_directive_utilization =
    create_utilization_array (S_cache_directives_per_cycle + 1);

  STATS_ZERO (unclassified_operations);

  STATS_ZERO (untraced_fixups);
  STATS_ZERO (num_squashed);

  /* Return the initialized structure */
  return (stats);
}

void
S_add_stats_superscalar (Superscalar_Stats * dest,
			 Superscalar_Stats * src1, Superscalar_Stats * src2)
{
  int i;

  STATS_ADD (loads_forwarded);
  STATS_ADD (loads_blocked);
  STATS_ADD (cycles_loads_blocked);
  STATS_ADD (cycles_sint_blocked_on_pending_load);
  STATS_ADD (cycles_dcache_busy_when_needed);
  STATS_ADD (cycles_dcache_ports_unavailable);

  STATS_ADD (cycles_branch_unit_unavailable);
  STATS_ADD (cycles_ialu_unit_unavailable);
  STATS_ADD (cycles_falu_unit_unavailable);
  STATS_ADD (cycles_load_unit_unavailable);
  STATS_ADD (cycles_store_unit_unavailable);
  STATS_ADD (cycles_cache_directive_unit_unavailable);

  for (i = 0; i < (S_issue_width + 1); i++)
    STATS_ADD (issue_utilization[i]);

  for (i = 0; i < (S_branches_per_cycle + 1); i++)
    STATS_ADD (branch_utilization[i]);

  for (i = 0; i < (S_ialu_ops_per_cycle + 1); i++)
    STATS_ADD (ialu_op_utilization[i]);

  for (i = 0; i < (S_falu_ops_per_cycle + 1); i++)
    STATS_ADD (falu_op_utilization[i]);

  for (i = 0; i < (S_loads_per_cycle + 1); i++)
    STATS_ADD (load_utilization[i]);

  for (i = 0; i < (S_stores_per_cycle + 1); i++)
    STATS_ADD (store_utilization[i]);

  for (i = 0; i < (S_cache_directives_per_cycle + 1); i++)
    STATS_ADD (cache_directive_utilization[i]);

  STATS_ADD (unclassified_operations);

  STATS_ADD (untraced_fixups);
  STATS_ADD (num_squashed);

}

void
S_print_stats_region_superscalar (FILE * out, Stats * stats,
				  char *rname, Stats * total_stats)
{
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  double percent_pred_squashed;
  Region_Stats *rstats;
#endif
  Superscalar_Stats *pstats;
  unsigned sim_cycles;
  int i;

  /* Setup Pstats calls */
  Pstats_out = out;
  Pstats_rname = rname;

  /* Get the number of cycles simulated in this region */
  sim_cycles = total_stats->region->num_sim_cycles;

  /* Get the bus stats structure for ease of use */
  pstats = (Superscalar_Stats *) stats->processor_v;

  Pstats ("# STATIC SUPERSCALAR PROCESSOR:");
  Pstats ("");
  Pstats ("%12u loads bypassed dcache by using a pending store's data.",
	  pstats->loads_forwarded);
  Pstats ("%12u loads blocked due to dependence on a pending store.",
	  pstats->loads_blocked);
  Pstats ("%12u cycles that a load was blocked on a pending store.",
	  pstats->cycles_loads_blocked);
  Pstats ("%12u cycles that a sint was blocked on a pending load.",
	  pstats->cycles_sint_blocked_on_pending_load);
  Pstats ("%12u cycles when the dcache was busy (load blocked).",
	  pstats->cycles_dcache_busy_when_needed);
  Pstats ("%12u cycles when dcache ports were unavailable (load blocked).",
	  pstats->cycles_dcache_ports_unavailable);

  Pstats ("%12u cycles when branch unit was unavailable (issue blocked).",
	  pstats->cycles_branch_unit_unavailable);
  Pstats ("%12u cycles when ialu unit was unavailable (issue blocked).",
	  pstats->cycles_ialu_unit_unavailable);
  Pstats ("%12u cycles when falu unit was unavailable (issue blocked).",
	  pstats->cycles_falu_unit_unavailable);
  Pstats ("%12u cycles when load unit was unavailable (issue blocked).",
	  pstats->cycles_load_unit_unavailable);
  Pstats ("%12u cycles when store unit was unavailable (issue blocked).",
	  pstats->cycles_store_unit_unavailable);
  Pstats ("%12u cycles when cache directive unit was unavailable.",
	  pstats->cycles_cache_directive_unit_unavailable);

  Pstats ("%12u untraced fixup instructions simulated.",
	  pstats->untraced_fixups);
  Pstats ("%12u instructions squashed during simulation.",
	  pstats->num_squashed);


  Pstats ("");

  Pstats
    ("# PROCESSOR UTILIZATION (includes off-path and pred squashed ops):");
  Pstats ("");

  if (sim_cycles < 1)
    {
      Pstats ("              No operations simulated.");
      Pstats ("");
      return;
    }

  for (i = 0; i < (S_issue_width + 1); i++)
    Pstats ("%12.2lf percent of the time issued %i instructions.",
	    (((double) 100 * (double) pstats->issue_utilization[i]) /
	     (double) sim_cycles), i);
  Pstats ("");

  for (i = 0; i < (S_branches_per_cycle + 1); i++)
    Pstats ("%12.2lf percent of the time issued %i branches.",
	    (((double) 100 * (double) pstats->branch_utilization[i]) /
	     (double) sim_cycles), i);
  Pstats ("");


  for (i = 0; i < (S_ialu_ops_per_cycle + 1); i++)
    Pstats ("%12.2lf percent of the time issued %i ialu ops.",
	    (((double) 100 * (double) pstats->ialu_op_utilization[i]) /
	     (double) sim_cycles), i);
  Pstats ("");


  for (i = 0; i < (S_falu_ops_per_cycle + 1); i++)
    Pstats ("%12.2lf percent of the time issued %i falu ops.",
	    (((double) 100 * (double) pstats->falu_op_utilization[i]) /
	     (double) sim_cycles), i);
  Pstats ("");


  for (i = 0; i < (S_loads_per_cycle + 1); i++)
    Pstats ("%12.2lf percent of the time issued %i loads.",
	    (((double) 100 * (double) pstats->load_utilization[i]) /
	     (double) sim_cycles), i);
  Pstats ("");


  for (i = 0; i < (S_stores_per_cycle + 1); i++)
    Pstats ("%12.2lf percent of the time issued %i stores.",
	    (((double) 100 * (double) pstats->store_utilization[i]) /
	     (double) sim_cycles), i);
  Pstats ("");

  for (i = 0; i < (S_cache_directives_per_cycle + 1); i++)
    Pstats ("%12.2lf percent of the time issued %i cache directives.",
	    (((double) 100 * (double) pstats->cache_directive_utilization[i])
	     / (double) sim_cycles), i);
  Pstats ("");

  Pstats ("%12u operations were unclassified (for the above stats).",
	  pstats->unclassified_operations);
  Pstats ("");
}

/*
 * Determine the superscalar simulation routine to use based
 * on processor_type.
 */
int
S_sim_superscalar (Pnode * pnode, int pc, unsigned sim_count)
{
  switch (S_processor_type)
    {
    case PROCESSOR_TYPE_STATIC:
      pc = S_sim_superscalar_scoreboard (pnode, pc, sim_count);
      break;

    default:
      S_punt ("S_sim_superscalar: Undefined processor type '%s'.",
	      S_processor_type_name);
    }

  /* Return the new pc */
  return (pc);
}

/* Fixes ifetch only right now */
void
S_superscalar_fix_mispredicted_branch (Pnode * pnode, Sint * mispred_sint)
{
  Superscalar *processor;
  Sq_entry *entry, *next_entry;
  short *operand;
  int index, id;
  Reg_File *rf;
  Sint *sint;
  Superscalar_Data *sdata;
  int i;

  /* Get processor for ease of use */
  processor = (Superscalar *) pnode->processor_v;

  /* Get superscalar data for mispredicted sint for ease of use */
  sdata = (Superscalar_Data *) mispred_sint->proc_data_v;

  /* Get register file for ease of use */
  rf = processor->reg_file;

  /* Clear fetch stages of sints */
  for (i = S_num_fetch_stages - 1; i >= 0; i--)
    {
      for (entry = processor->fetch_stage[i]->head; entry != NULL;
	   entry = next_entry)
	{
	  next_entry = entry->next_entry;
	  sint = entry->sint;
	  S_dequeue (entry);
	  S_free_sint (sint);
	}
    }

  /* Clear decode buf of sints */
  for (entry = processor->decode_queue->head; entry != NULL;
       entry = next_entry)
    {
      next_entry = entry->next_entry;
      sint = entry->sint;
      S_dequeue (entry);
      S_free_sint (sint);
    }

  /* 
   * Marks entries after sint in reorder buf as SQUASHED,
   * Clear the destination entries in register file.
   */
  for (entry = sdata->reorder_queue_entry->next_entry;
       entry != NULL; entry = entry->next_entry)
    {
      sint = entry->sint;
      sint->flags |= SQUASHED;
      operand = sint->oper->operand;

      /* Update squashed stats for the region the squashed sint is in */
      ((Superscalar_Stats *) sint->stats->processor_v)->num_squashed++;

      for (index = S_first_dest; index <= S_last_dest; index++)
	{
	  /* Get operand id, if >0, it is a register */
	  id = operand[index];

	  /* If register, mark available (will fix below) */
	  if (id > 0)
	    {
	      rf[id].tag = NULL;
	      rf[id].value_avail = COMPLETED;
	      rf[id].adjust = 0;
	    }
	}
    }

  /*
   * Rebuild rf tags and value_avail flags from reorder buf entries
   * before the squashed sint.
   */
  for (entry = sdata->reorder_queue_entry;
       entry != NULL; entry = entry->prev_entry)
    {
      sint = entry->sint;
      operand = sint->oper->operand;

      /* update reg_file if sint not SQUASHED */
      if (!(sint->flags & SQUASHED))
	{
	  for (index = S_first_dest; index <= S_last_dest; index++)
	    {
	      /* Get operand id, if >0, it is a register */
	      id = operand[index];

	      /* If register and tag == NULL, update rf */
	      if ((id > 0) && (rf[id].tag == NULL))
		{
		  rf[id].tag = sint;
		  rf[id].value_avail = sint->flags & COMPLETED;
		  rf[id].adjust = 0;	/*sint->oper->adjust_real_latency; */
		  /*printf("here %d\n",rf[id].adjust); */
		}
	    }
	}
    }

  /*
   * Restore ifetch_pc to proper location, and enable fetching from
   * correct path.
   */
  processor->ifetch_pc = processor->on_path_pc;
  /* Handle end of program case properly (pc == 0) */
  if (oper_tab[processor->ifetch_pc] != NULL)
    processor->ifetch_addr = oper_tab[processor->ifetch_pc]->instr_addr;
  else
    processor->ifetch_addr = 0;
  processor->on_correct_path = 1;

  /* Disable ifetch this cycle, but enable next cycle */
  processor->ifetch_enabled = 0;
  processor->enable_ifetch_next_cycle = 1;
}

void
S_superscalar_ifetch_stage (pnode)
     Pnode *pnode;
{
  Superscalar *processor;
  int num_avail, num_fetched;
  int pc, max_pc;
  Squeue **fetch_stage, *this_stage;
  Sq_entry *entry, *next_entry;
  Sint *sint;
  S_Opc_Info *info;
  int predicted_target;
  int on_correct_path, untraced_fixup, stage;
  int on_path_sint;
  int MCB_conflict;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int ALAT_conflict;
#endif
  int issue_width, adjust;
  Superscalar_Stats *ss_stats;
  MCB_Stats *mstats;
  ALAT_Stats *astats;
  Superscalar_Data *sdata;
  int S_sim_cycle_adjust = 0;

  /* Get local variables for easy of use */
  processor = (Superscalar *) pnode->processor_v;

  /* To detect incorrect use of stats pointers */
  ss_stats = NULL;
  mstats = NULL;

  /* For ease of use */
  issue_width = S_issue_width;

  /* 
   * Allow multiple fetch stages.  Allow instructions in
   * one fetch stage to advance to the next fetch stage, if
   * there is room for it.  This allows "packing" of operations
   * fetched in different cycles as they flow through the fetch
   * stages.  
   * 
   * Although this sounds unrealistic, our multiple fetch
   * stage modeling is a rough approx. anyway and modeling the fetch stages
   * this way this makes the simulation results come out consistently. 
   * (I.e., with perfect branch predication and perfect caches, almost
   *  no performance difference between 1 fetch stages and 2 fetch
   *  stages.)
   * 
   * My first implementation did not allow "packing" since it seemed 
   * unrealistic, but this caused a 5-10% performance lost from
   * just going from one fetch stage to two (perfect everything).
   * Briand and I decided that not allowing "packing" was probably
   * not very much more accurate and therefore something very hard to
   * explain that didn't buy us much.  Therefore, the more 
   * aggressive and consistent model was chosen.  -JCG 10/17/96
   */
  fetch_stage = processor->fetch_stage;
  for (stage = S_num_fetch_stages - 1; stage > 0; stage--)
    {
      /* 
       * Move packet from previous stage to this fetch stage 
       * if this fetch stage has room for at least one more op.
       */
      this_stage = fetch_stage[stage];
      if (this_stage->size < issue_width)
	{
	  for (entry = fetch_stage[stage - 1]->head;
	       entry != NULL; entry = next_entry)
	    {
	      /* Get next entry before moving this one */
	      next_entry = entry->next_entry;

	      /* Move entry to end of this fetch stage */
	      S_move_entry_before (this_stage, entry, NULL);

	      /* Max operations in each stage is set by issue_width */
	      if (this_stage->size >= issue_width)
		break;
	    }
	}
    }


  /*
   * FETCH_MODEL_CONSERVATIVE sets S_ifetch_mark to 0, 
   * FETCH_MODEL_AGGRESSIVE will be >= 1 (usually issue width).
   */

  /* Fetch instructions if ifetch enabled (and there is any room) */
  if ((processor->ibuf->size <= S_fetch_buf_size) &&
      (processor->ifetch_enabled))
    {
      /* Get pc, max_pc, on_correct_path and untraced_fixup for ease of use */
      pc = processor->ifetch_pc;
      max_pc = S_max_pc;
      on_correct_path = processor->on_correct_path;
      untraced_fixup = processor->untraced_fixup;

#ifdef DEBUG_IFETCH
      printf ("   Ifetch: pc = %i\n", processor->ifetch_pc);
#endif

      /* Convert the bytes returned into instructions (divide by 4) */
      if (pnode->icache_addr_requested == processor->ifetch_addr)
	num_avail = pnode->icache_bytes_returned >> 2;
      else
	num_avail = 0;

      /* Limit to fetch width */
      if (num_avail > S_fetch_width)
	num_avail = S_fetch_width;

      /* Limit so afterwards don't exceed ibuf width */
      if ((num_avail + processor->ibuf->size) > S_fetch_buf_size)
	num_avail = S_fetch_buf_size - processor->ibuf->size;

      for (num_fetched = 0;
	   (num_fetched < num_avail) && (pc <= max_pc) && !S_end_of_program;
	   num_fetched++)
	{
	  /* 
	   * Generate sint for this pc, use trace information only if
	   * on the correct path and not simulating an untraced fixup
	   * code segment
	   */
	  on_path_sint = (on_correct_path && !untraced_fixup);
	  sint = S_gen_sint (pnode, pc, on_path_sint);

	  /* Generate the superscalar data needed for this sint */
	  sdata = (Superscalar_Data *) L_alloc (Superscalar_Data_pool);
	  sint->proc_data_v = (void *) sdata;
	  sint->superscalar = sdata;	/* for debugging */

	  /* Point sdata back at sint it is for */
	  sdata->sint = sint;

	  /* Initialize sdata structure */
	  sdata->flags = 0;
	  sdata->input_dep = NULL;
	  sdata->output_dep = NULL;
	  sdata->reorder_queue_entry = NULL;

	  /* Get opc info for easy access */
	  info = &opc_info_tab[sint->oper->opc];

	  /* Marked untraced fixup code */
	  if (untraced_fixup)
	    {
	      sint->flags |= UNTRACED_FIXUP;
	    }

	  /* 
	   * Get Region and Superscalar stats for ease of use,
	   * may change for each sint
	   */
	  ss_stats = (Superscalar_Stats *) sint->stats->processor_v;
	  mstats = sint->stats->mcb;
	  astats = sint->stats->alat;


	  /*
	   * Simple ALAT code. Assumes a fake pre_load exists
	   * in place of speculated load. preload dest is normal
	   * dest reg, preload src1 is penalty for mis-speculation
	   */
	  if ((S_ALAT_model != ALAT_MODEL_NO_ALAT) && on_correct_path)
	    {
	      S_sim_ALAT_all_ops (pnode, sint, S_sim_cycle);

	      if ((info->opc_type == LOAD_OPC) &&
		  ((sint->oper->flags & MCB_ATTR) ||
		   S_ALAT_all_loads_preloads))
		{
		  /* Update stats */
		  astats->loads++;

		  /* Call MCB simulation */
		  adjust = S_sim_ALAT_load (pnode, sint, S_sim_cycle);

		  if (adjust > 0)
		    {
		      /* Penalty, add to latency */
		      /* printf("Penalty (%d)\n",adjust); */
		      sint->oper->real_latency[S_first_dest] += adjust;
		      /*sint->real_complete_time[S_first_dest] += adjust; */
		    }
		  else if (adjust < 0)
		    {
		      /* benefit */
		      if ((sint->oper->real_latency[S_first_dest] + adjust)
			  > 0)
			{
			  /* Just add to latency (adjust is negative) */
			  /* printf("Benefit1 (%d)\n",adjust); */
			  sint->oper->real_latency[S_first_dest] += adjust;
			  /*sint->real_complete_time[S_first_dest] += adjust; */
			}
		      else
			{
			  /* Rollback sim time, mark as complete */
			  /* printf("Benefit2 (%d)\n",adjust); */
			  /* sint->oper->real_latency[S_first_dest] = 1; */
			  sint->oper->adjust_real_latency =
			    sint->oper->real_latency[S_first_dest] + adjust -
			    1;
			  sint->oper->real_latency[S_first_dest] = 1;

			  /*sint->real_complete_time[S_first_dest] -= 
			     (sint->oper->real_latency[S_first_dest]); */

			  /*
			     if (adjust < S_sim_cycle_adjust)
			     S_sim_cycle_adjust = adjust;
			     sint->real_complete_time[S_first_dest] += adjust;
			   */
			}
		    }
		}
	      else if ((info->opc_type == CBR_OPC) &&
		       (sint->oper->flags & MCB_ATTR))
		{
		  S_punt ("ALAT: CBR_OPC not handled");
		}
	      else if (info->opc_type == STORE_OPC)
		{
		  S_sim_ALAT_store (pnode, sint);
		}
	      else if (info->is_branch &&
		       (sint->flags & BRANCHED) &&
		       !(sint->oper->flags & MCB_ATTR))
		{
		  S_sim_ALAT_taken_branch (pnode, sint);
		}
	    }


	  /*
	   * Do MCB simulation first processing branches because
	   * can cause branching to "untraced fixup" code.
	   *
	   * Do not simulate MCB code on off path instructions.
	   * Assumes hardware squashing prevents this
	   * (using same mechanism that AMD uses to prevent speculative
	   *  loads from going to dcache).
	   */
	  if ((S_MCB_model != MCB_MODEL_NO_MCB) && on_correct_path)
	    {
	      /* 
	       * If in untraced fixup code, we should not see any
	       * branches or MCB code except for a jump with a MCB
	       * attr which is used as a "return from untraced fixup"
	       * code calls.
	       */
	      if (untraced_fixup)
		{
		  /* Update stats */
		  ss_stats->untraced_fixups++;

		  /* Test for jump back to traced path */
		  if ((info->opc_type == JMP_OPC) &&
		      (sint->oper->flags & MCB_ATTR))
		    {
		      /* Force return to traced path */
		      sint->trace.target_pc = processor->fixup_return_pc;
		      untraced_fixup = 0;
		      sint->flags |= BRANCHED;
		    }

		  else if (info->is_branch)
		    {
		      S_punt ("%s op %i:Branch in untraced fixup code in %s",
			      sint->fn->name, sint->oper->lcode_id);
		    }

		}

	      /*
	       * Untraced fixup code and OFF_PATH does not go to MCB!
	       */
	      /* 
	       * Simulate hardware for loads and branches with MCB attr.
	       * Simulate hardware for all stores.
	       */
	      else if ((info->opc_type == LOAD_OPC) &&
		       ((sint->oper->flags & MCB_ATTR) ||
			S_MCB_all_loads_preloads))
		{
		  /* Update stats */
		  mstats->loads++;


		  /* Call MCB simulation */
		  S_sim_MCB_load (pnode, sint);
		}
	      else if ((info->opc_type == CBR_OPC) &&
		       (sint->oper->flags & MCB_ATTR))
		{
		  /* Update stats */
		  mstats->beqs++;

		  /* Determine if MCB conflict occurs */
		  MCB_conflict = S_sim_MCB_beq (pnode, sint);

		  /* Did an conflict occur? */
		  if (MCB_conflict)
		    {
		      /* Is it a true conflict? */
		      if (sint->flags & BRANCHED)
			{
			  /* Yes, update stats */
			  mstats->true_conflicts++;
			}

		      /* False conflict */
		      else
			{
			  /* Update stats */
			  mstats->false_conflicts++;

			  /*
			   * Simulate untraced fixup code, 
			   * jump to branch target 
			   */
			  sint->trace.target_pc =
			    sint->fn->cb[sint->oper->br_target].start_pc;
			  sint->flags |= BRANCHED;
			  processor->fixup_return_pc = pc + 1;
			  untraced_fixup = 1;
			}
		    }

		  /* No conflict predicted, better not branch */
		  else if (sint->flags & BRANCHED)
		    S_punt ("MCB conflict missed\n");

		}
	      else if (info->opc_type == STORE_OPC)
		{
		  S_sim_MCB_store (pnode, sint);
		}

	      /* Taken non-MCB branches flush the mcb */
	      else if (info->is_branch &&
		       (sint->flags & BRANCHED) &&
		       !(sint->oper->flags & MCB_ATTR))
		{
		  S_sim_MCB_taken_branch (pnode, sint);
		}
	    }

	  /*
	   * Update stats for instructions on correct path
	   * Do not count UNTRACED_FIXUP code in on_path stats
	   */
	  if (on_correct_path)
	    {
	      if (sint->flags & BRANCHED)
		processor->on_path_pc = sint->trace.target_pc;
	      else
		processor->on_path_pc = pc + 1;
	    }

	  /* Otherwise, mark sint as OFF_PATH */
	  else
	    {
	      sint->flags |= OFF_PATH;
	    }

	  /* Add sint to ibuf */
	  S_enqueue (processor->ibuf, sint);

	  /*
	   * If desired to flush pipe on untraced JSRs, 
	   * then
	   *  JSR, mark as mispredicted and disable ifetch until fixed.  
	   * else
	   *  Treat as correctly predicted fall thru branch.
	   *
	   *  Do before branch prediction so not included
	   *  in stats.
	   */
	  if (sint->flags & UNTRACED_JSR)
	    {
#ifdef DEBUG_IFETCH
	      printf ("   Ifetch: untraced jsr.\n");
#endif
	      if (S_flush_pipe_on_untraced_jsr)
		{
		  /* move next instruction to 0 (will die if try to fetch) */
		  pc = 0;

		  /* Mark JSR as mispredicted */
		  sint->flags |= MISPREDICTED;


		  /* Disable ifetch until Branch fix */
		  on_correct_path = 0;
		  processor->ifetch_enabled = 0;
		  if (S_simulation_with_profile_information)
		    S_profile_simulation_instruction (sint->oper->pc, sint,
						      on_path_sint);
		  break;
		}
	      /* Otherwise, fall thru normally */
	      else
		{
		  pc++;
		}
	    }

	  /* Do branch prediction if a branch */
	  else if (info->is_branch)
	    {
	      predicted_target = S_get_BTB_prediction (pnode, sint);

	      /* If mispredicted branch, mark that we off the correct path */
	      if (sint->flags & MISPREDICTED)
		{
		  on_correct_path = 0;
		}

	      /* 
	       * If predict branched, change pc and throw
	       * away rest of instructions fetched.
	       */
	      if (predicted_target != -1)
		{
		  pc = predicted_target;
		  if (S_simulation_with_profile_information)
		    S_profile_simulation_instruction (sint->oper->pc, sint,
						      on_path_sint);
		  break;
		}
	      else
		pc++;
	    }

	  /* Otherwise, if not a branch, goto next pc */
	  else
	    {
	      pc++;
	    }
	  if (S_simulation_with_profile_information)
	    S_profile_simulation_instruction (sint->oper->pc, sint,
					      on_path_sint);
	}

      /* Update mem copy of ifetch_pc, on_correct_path, untraced_fixup  */
      processor->ifetch_pc = pc;

      /* 
       * Handle end of program case properly (pc == 0) 
       * JCG 9/29/96 Extended to handle random pc misprediction
       * JCG 6/20/97 Fixed so max_pc is a valid address (oops).
       */
      if (((unsigned) pc <= (unsigned) max_pc) && (oper_tab[pc] != NULL))
	processor->ifetch_addr = oper_tab[pc]->instr_addr;
      else
	processor->ifetch_addr = 0;
      processor->on_correct_path = on_correct_path;
      processor->untraced_fixup = untraced_fixup;
    }

  S_total_adjust += S_sim_cycle_adjust;
  S_total_unadjust_cycle++;
  /*S_sim_cycle += S_sim_cycle_adjust; */
}

void
S_superscalar_scoreboard_retire_stage (Pnode * pnode)
{
  static int cycle_of_last_retire = 0;
  Superscalar *processor;
  S_Opc_Info *info;
  Sq_entry *entry, *next_entry;
  Reg_File *rf;
  short *operand;
  Sint *sint;
  int retired, index, id;
  double dynamic_count;

  /* Get processor and register file for ease of use */
  processor = (Superscalar *) pnode->processor_v;
  rf = processor->reg_file;

  /* Initialize count of sint's retired */
  retired = 0;


  /* Retire completed sints from retire buffer until retire width used up */
  for (entry = processor->reorder_queue->head;
       (entry != NULL) && (entry->sint->flags & COMPLETED) &&
       (retired < S_retire_width); entry = next_entry, retired++)
    {
      /* Get next entry before delete this one */
      next_entry = entry->next_entry;

      /* Get sint for ease of use */
      sint = entry->sint;

      /* Get opcode info for ease of use */
      info = &opc_info_tab[sint->oper->opc];

      /* Only update processor state for unsquashed instructions */
      if (!(sint->flags & SQUASHED))
	{
	  /* Make sure unsquashed instructions are from on trace path */
	  if (sint->flags & OFF_PATH)
	    S_punt ("Retire stage: Off path sint unsquashed.");

	  /*
	   * Stores must be able go to dcache now. Make sure
	   * dcache is not busy and has availabe ports.
	   *
	   * (This code only does something when the parameter
	   *  delay_stores_until_retirement = yes.  Otherwise,
	   *  all the stores will be sent to the dcache during
	   *  decode and never placed in the reorder buffer.) -JCG 2/18/98
	   */
	  if ((info->opc_type == STORE_OPC) &&
	      (pnode->dcache_busy ||
	       (pnode->dcache_request->size >= S_dcache_ports)))
	    break;


	  /*
	   * Reset rf tags.
	   */
	  operand = sint->oper->operand;
	  for (index = S_first_dest; index <= S_last_dest; index++)
	    {
	      id = operand[index];

	      if ((id > 0) && (rf[id].tag == sint))
		rf[id].tag = NULL;
	    }
	}

      else
	{
	  /* Make sure squashed instructions are off path */
	  if (!(sint->flags & OFF_PATH))
	    S_punt ("Retire stage: On path sint squashed.");
	}

      /* 
       * Remove sint from all queues (should be in only processor 
       * queues or else something will get messed up)
       */
      S_dequeue_from_all (sint);

      /* 
       * If sint a is non-squashed store, sent to dcache for processing 
       * (This code only does something when the parameter
       * delay_stores_until_retirement = yes.  Otherwise,
       * all the stores will be sent to the dcache during
       * decode and never placed in the reorder buffer.) -JCG 2/18/98
       */
      if ((info->opc_type == STORE_OPC) && !(sint->flags & SQUASHED))
	S_enqueue (pnode->dcache_request, sint);

      /* Otherwise through with sint, free memory */
      else
	S_free_sint (sint);
    }

  /* Detect infinite loop */
  if (retired != 0)
    cycle_of_last_retire = S_sim_cycle;
  else if ((cycle_of_last_retire + 1000) < S_sim_cycle)
    {
      fprintf (stderr,
	       "Deadlock detected in cycle %i, sample %i.\n",
	       S_sim_cycle, S_num_sim_samples);
      fprintf (stderr, "Have not retired anything since cycle %i.\n",
	       cycle_of_last_retire);
      if (processor->reorder_queue->head != NULL)
	{
	  sint = processor->reorder_queue->head->sint;
	  fprintf (stderr,
		   "Instruction causing simulation deadlock (%s op %i):\n",
		   sint->oper->cb->fn->name, sint->oper->lcode_id);
	  S_print_sint (stderr, processor->reorder_queue->head->sint);
	  S_print_queues_in (stderr, processor->reorder_queue->head->sint);
	  dynamic_count = (double) S_num_skip_on_path;
	  dynamic_count += ((double) S_billions_skipped * (double) BILLION);
	  dynamic_count += (double) S_num_sim_on_path;
	  fprintf (stderr,
		   "Sint's serial_no %i, approx dynamic instruction # %2.0lf.\n",
		   sint->serial_no, dynamic_count);
	}
      else
	{
	  fprintf (stderr, "Reorder queue empty!  (unexpected)\n");
	}
      fprintf (stderr, "\n");

      if (pnode->dcache_busy)
	fprintf (stderr, "Dcache marked as busy!.\n");
      else
	fprintf (stderr, "Dcache not marked as busy.  (good)\n");

      if (pnode->icache_busy)
	fprintf (stderr, "Icache marked as busy!.\n");
      else
	fprintf (stderr, "Icache not marked as busy.  (good)\n");

      if (pnode->dcache->mc_pending_queue->head != NULL)
	{
	  fprintf (stderr,
		   "Pending mem_copy instructions(waiting for memory)!\n");
	  S_print_queue (stderr, pnode->dcache->mc_pending_queue);
	}
      if (pnode->dcache->mc_check_queue->head != NULL)
	{
	  fprintf (stderr, "Pending check instructions!\n");
	  S_print_queue (stderr, pnode->dcache->mc_check_queue);
	}

      if (pnode->dcache->mc_instr_queue->head != NULL)
	{
	  fprintf (stderr,
		   "Mem_copy instructions waiting to access memory!\n");
	  S_print_queue (stderr, pnode->dcache->mc_instr_queue);
	}
      if (pnode->dcache->mc_request_buffer->head != NULL)
	{
	  fprintf (stderr,
		   "%i mem_copy requests waiting for memory.\n",
		   pnode->dcache->mc_request_buffer->size);
	}

      if (processor->decode_queue->head != NULL)
	{
	  fprintf (stderr, "Sint at head of decode queue from %s:\n",
		   processor->decode_queue->head->sint->fn->name);
	  S_print_sint (stderr, processor->decode_queue->head->sint);
	}
      else
	{
	  fprintf (stderr, "Decode queue empty!  (unexpected)\n");
	}

      if (processor->ibuf->head != NULL)
	{
	  fprintf (stderr, "Sint at head of ibuf from %s:\n",
		   processor->ibuf->head->sint->fn->name);
	  S_print_sint (stderr, processor->ibuf->head->sint);
	}
      else
	{
	  fprintf (stderr, "Icache queue empty!  (unexpected)\n");
	}

      fprintf (stderr,
	       "on_path_pc = %i  max_pc = %i min_pc = %i ifetch_pc = %i\n",
	       processor->on_path_pc, S_max_pc, S_program_start_pc,
	       processor->ifetch_pc);
      fprintf (stderr, "ifetch_enabled = %i ifetch_address = %o\n",
	       processor->ifetch_enabled, processor->ifetch_addr);

      if (processor->on_path_pc > S_max_pc)
	{
	  fprintf (stderr, "on_path_pc (%i) > max_pc (%i)! (unexpected)\n",
		   processor->on_path_pc, S_max_pc);
	}

      if (processor->on_path_pc < S_program_start_pc)
	{
	  fprintf (stderr,
		   "on_path_pc (%i) < program_start_pc (%i)! (unexpected)\n",
		   processor->on_path_pc, S_program_start_pc);
	}

      fprintf (stderr, "Scanning scoreboard for locked entries:\n");
      fflush (stderr);

      for (index = 0; index < S_max_register_operand; index++)
	{
	  if (rf[index].value_avail == 0)
	    {
	      fprintf (stderr, "%s not available\n",
		       operand_tab[index]->string);
	    }
	  if (rf[index].tag != NULL)
	    {
	      fprintf (stderr,
		       "  Tag specifies this sint (may crash Lsim):\n");
	      S_print_sint (stderr, rf[index].tag);
	    }
	}

      S_punt ("Aborting simulation due to deadlock.");
    }
}

/* HCH/JWS 19991027 Support for analysis of speculated loads */
void
S_add_outstanding_load (unsigned int specid, int dcache_miss)
{
  if (specid > S_max_specid)
    S_punt ("S_add_oustanding_load: S_max_specid exceeded.");

  S_oload[specid].os = 1;
  S_oload[specid].dcache_miss = dcache_miss;

  return;
}

int
S_outstanding_miss (unsigned int specid)
{
  if (specid > S_max_specid)
    S_punt ("S_outstanding_miss: S_max_specid exceeded.");
  return S_oload[specid].dcache_miss;
}

#define PUNT_ON_UNMATCHED_SPECID 0

int
S_match_outstanding_load (unsigned int specid, int delete)
{
  int retval;

  if (specid > S_max_specid)
    S_punt ("S_match_oustanding_load: S_max_specid exceeded.");

  retval = S_oload[specid].os;

  if (delete)
    {
#if PUNT_ON_UNMATCHED_SPECID
      if (!S_oload[specid].os)
	S_punt ("S_match_outstanding_load: SPECID %d is unmatched.", specid);
#endif
      S_oload[specid].os = 0;
    }

  return retval;
}

Check_line *
S_new_sload_line (unsigned int specid)
{
  int i;
  Check_line *line;
  line = (Check_line *) L_alloc (Spec_check_pool);
  for (i = 0; i < 32; i++)
    line->taken_data[i] = 0;
  line->num_entries = 0;
  line->specid = specid;
  return line;
}

void
S_init_sload_data (void)
{
  int i;

  if (!S_max_specid)
    {
      S_sload_lines = NULL;
      return;
    }

  if (!(S_sload_lines = malloc ((S_max_specid + 1) * sizeof (Check_line *))))
    S_punt ("S_init_sload_data: Unable to alloc line array.");

  if (!(S_oload = malloc ((S_max_specid + 1) * sizeof (S_Outstanding_Load))))
    S_punt ("S_init_sload_data: Unable to Oustanding Load array.");

  if (Spec_check_pool == NULL)
    {
      Spec_check_pool =
	L_create_alloc_pool ("Check_line", sizeof (Check_line), 256);
    }

  for (i = 0; i <= S_max_specid; i++)
    {
      S_sload_lines[i] = S_new_sload_line (i);
      S_oload[i].os = 0;
    }
  return;
}


void
S_print_sload_line (Check_line * line)
{
  if (!S_sload_file || !line)
    S_punt ("S_print_sload_line: NULL");
  if ((fwrite (line, sizeof (Check_line), 1, S_sload_file)) != 1)
    S_punt ("Error writing to '%s'.", S_sload_file_name);
  return;
}

void
S_finish_sload_data (void)
{
  int i;
  if (!S_sload_lines)
    return;

  for (i = 0; i <= S_max_specid; i++)
    if (S_oload[i].os)
      {
	S_gen_check_line_entry (i, 0);
	S_gen_check_line_entry (i, S_oload[i].dcache_miss);
      }
  for (i = 0; i <= S_max_specid; i++)
    {
      if (S_sload_lines[i]->num_entries)
	S_print_sload_line (S_sload_lines[i]);
    }

  return;
}


/* HCH 10-26-99 */
/* 	Function to create lines indicating if speculative load was needed.
	Lines contain 32-bit identification number for speculative load,
	32-bit value indicating the number of valid data entries,
	and 256 bits of space for bits indicating if speculative load's
	home block was reached.  					*/
void
S_gen_check_line_entry (unsigned int checkid, int hb_reached)
{
  unsigned int entry_count;
  Check_line *line;

  if (checkid > S_max_specid)
    S_punt ("S_max_specid=%d is not large enough", S_max_specid);

  line = S_sload_lines[checkid];

  entry_count = line->num_entries;

  /* hb_reached will indicate if the homeblock was reached for this 
     execution of the speculative load */
  if (hb_reached)
    line->taken_data[entry_count / 8] |= ((0x01u) << (entry_count % 8));
  else
    line->taken_data[entry_count / 8] &= ~((0x01u) << (entry_count % 8));

  /* update valid entry field in line */
  line->num_entries = entry_count + 1;

  if (line->num_entries == 256)
    {
      S_print_sload_line (line);
      line->num_entries = 0;
    }
  return;
}

/* Just updates register file and frees sints with complete_time is reached */
void
S_superscalar_scoreboard_exec_stage (Pnode * pnode)
{
  Sq_entry *entry, *next_entry;
  Sint *sint;
  int index;
  int completed;
  short *operand, id;
  Reg_File *rf;
  Superscalar *processor;
  Superscalar_Data *sdata;
  S_Opc_Info *info;

#ifdef DEBUG_PROC
  printf ("Completed:\n");
#endif

  /* Get register file for ease of use */
  processor = (Superscalar *) pnode->processor_v;
  rf = processor->reg_file;

  for (entry = processor->exec_queue->head; entry != NULL; entry = next_entry)
    {
      next_entry = entry->next_entry;

      /* Get sint for entry and processor data for ease of use */
      sint = entry->sint;
      sdata = (Superscalar_Data *) sint->proc_data_v;

      /* 
       * Assume infinite width to retire stage.
       */

      /* Make destinations as available if tag matches */
      completed = 1;
      operand = sint->oper->operand;
      for (index = S_first_dest; index <= S_last_dest; index++)
	{
	  id = operand[index];

	  if (!id)
	    continue;

	  if (sint->real_complete_time[index] <= S_sim_cycle)
	    {
	      if ((rf[id].tag == sint))
		{
		  rf[id].value_avail = COMPLETED;
		  /* EMN */
		  rf[id].adjust = sint->oper->adjust_real_latency;
		}
	    }
	  else
	    {
	      completed = 0;
	    }
	}

      /* Is the real complete time up for this sint ? */
      if (completed)
	{
	  int lcode_id;
	  lcode_id = sint->oper->lcode_id;

#ifdef DEBUG_NEW
	  fprintf (debug_out,
		   "%s op %i: issued %6i completed %i latency %2i\n",
		   sint->fn->name, sint->oper->lcode_id,
		   sdata->issue_time, S_sim_cycle,
		   S_sim_cycle - sdata->issue_time);
#endif

#ifdef DEBUG_PROC
	  S_print_sint (stdout, sint);
#endif
	  /* If mispredicted (on path branch), fix branch */
	  if ((sint->flags & MISPREDICTED) && !(sint->flags & OFF_PATH))
	    S_superscalar_fix_mispredicted_branch (pnode, sint);

	  info = &opc_info_tab[sint->oper->opc];

	  /* HCH/JWS 19991021 Speculated load analysis */
	  if (S_gen_sload_data && !(sint->flags & OFF_PATH))
	    {
	      if ((info->opc_type == LOAD_OPC) &&
		  (sint->oper->flags & SPECULATIVE))
		{
		  if (S_match_outstanding_load (sint->oper->br_target, 0))
		    {
		      int prev_dcmiss;
		      prev_dcmiss = S_outstanding_miss (sint->oper->br_target);
		      S_gen_check_line_entry (sint->oper->br_target, 0);
		      S_gen_check_line_entry (sint->oper->br_target,
					      prev_dcmiss);
		    }
		  S_add_outstanding_load (sint->oper->br_target,
					  sint->flags & DCACHE_MISS);
		}
	      else if ((info->opc_type == CHECK_OPC))
		{
		  /* 10/25/04 REK Commenting out unused variable to quiet
		   *              compiler warning. */
#if 0
		  int hb_reached;
#endif
		  int check_specid;
		  check_specid = sint->oper->br_target;
		  /* Look back through linked list of loads to find loads
		     with same specid as check; */
		  if (S_match_outstanding_load (check_specid, 0))
		    {
		      int prev_dcmiss;
		      prev_dcmiss = S_outstanding_miss (check_specid);
		      S_gen_check_line_entry (check_specid, 1);
		      S_gen_check_line_entry (check_specid, prev_dcmiss);
		    }
		  S_match_outstanding_load (check_specid, 1);
		}
	    }

	  /* Mark sint as completed */
	  sint->flags |= COMPLETED;

	  /* Remove from exec queue */
	  S_dequeue (entry);
	}
    }
  return;
}

/*
 * Returns the most resent store in the pending store buffer 
 * that the load's address conflicts with.
 * 
 * Returns NULL if none found.
 */
Sint *
S_find_conflicting_store (Pnode * pnode, Sint * load)
{
  Sq_entry *entry;
  Sint *store;
  int load_mask, load_addr, mask, hash_index;
  Superscalar *processor;

  processor = (Superscalar *) pnode->processor_v;

  /* Get the load's conflict mask and addr for ease of use */
  load_mask = load->conflict_mask;
  load_addr = load->trace.mem_addr;

  /* 
   * Get load hash index (using middle bits of load address)
   * into pending stores queues
   */
  hash_index = (load_addr >> 3) & (STORE_HASH_SIZE - 1);

  /* 
   * Search through pending stores who's addresses have a common
   * middle bits (hash_index) to see if address conflict,
   * start with most recent store
   */
  for (entry = processor->pending_stores[hash_index]->tail;
       entry != NULL; entry = entry->prev_entry)
    {
      /* Get store sint for ease of use */
      store = entry->sint;

      /*
       * Find the mask (to mask out least significant bits of address).
       * Use mask of larger access (AND masks together).
       */
      mask = load_mask & store->conflict_mask;

      /* 
       * To determine if load/store overlap, we mask out the 
       * least significant bits of the smaller access 
       * (one of the masks will not do anything) then compare.  
       * Thus if have a word and a char, we will mask out
       * the char address to see if they hit the same word.
       */
      if ((load_addr & mask) == (store->trace.mem_addr & mask))
	/* if conflict, return store sint */
	return (store);
    }

  return (NULL);
}

void
S_superscalar_scoreboard_decode_stage (Pnode * pnode)
{
  Superscalar *processor;
  Sq_entry *entry, *next_entry;
  Squeue *decode_queue;
  Sint *sint, *conflicting_store;
  Reg_File *rf;
  S_Opc_Info *info;
  int index, hash_index;
  short *operand, id;
  int branch_count, ialu_op_count, falu_op_count;
  int load_count, store_count, directive_count, unclassified_count;
  int issue_count;
  int sint_blocked;
  unsigned opflags, opc_type;
  Superscalar_Stats *ss_stats, *utilization_stats;
  Histogram_Stats *hstats;
  int issue_width;
  int dcache_ports;
  Superscalar_Data *sdata;
  int i;
  int squash_request_because_off_path;
  int max_adjust = 0;

  /* Get processor for ease of use */
  processor = (Superscalar *) pnode->processor_v;

  /* Get the decode queue for ease of use */
  decode_queue = processor->decode_queue;

  /* Prevent unexpected use of region pointers */
  ss_stats = NULL;

  /* Get register file for ease of use */
  rf = processor->reg_file;

  /* 
   * Get issue width for ease of use, may increase for this 
   * cycle due to ZERO_SPACE instructions.
   */
  issue_width = S_issue_width;

  /* 
   * For each ZERO_SPACE instruction already in the decode
   * queue, increase the issue width for this cycle.
   */
  for (entry = decode_queue->head; entry != NULL; entry = entry->next_entry)
    {
      if (entry->sint->oper->flags & ZERO_SPACE)
	issue_width++;
    }

  /* 
   * Get dcache ports for this cycle.  May increase if "ZERO_SIZE" 
   * instructions are present.
   */
  dcache_ports = S_dcache_ports;


#ifdef DEBUG_PROC
  printf ("Transfering into decode buf:\n");
#endif
  /* 
   * Move instructions into decode buf based on fetch model.
   */
  if (S_fetch_model == FETCH_MODEL_AGGRESSIVE)
    {
      /* 
       * Agressive fetch/decode model
       * Get instructions (if available) from last ifetch stage until
       * dcode_queue has issue_width instructions in it.
       */
      while ((decode_queue->size < issue_width) &&
	     (processor->last_fetch_stage->size > 0))
	{
#ifdef DEBUG_PROC
	  S_print_sint (stdout, processor->last_fetch_stage->head->sint);
#endif
	  /* 
	   * If the instruction is ZERO_SPACE, increase the
	   * issue width for this cycle.
	   */
	  if (processor->last_fetch_stage->head->sint->oper->flags &
	      ZERO_SPACE)
	    {
	      issue_width++;
	    }

	  /* Move first instruction in last_fetch_stage to end of decode */
	  S_move_entry_before (decode_queue,
			       processor->last_fetch_stage->head, NULL);
	}
    }

  /* FETCH_MODEL_CONSERVATIVE */
  else
    {
      /* If decode buf empty, transfer sints from last_fetch_stage into it */
      if (decode_queue->size == 0)
	{
	  for (entry = processor->last_fetch_stage->head;
	       (entry != NULL) && (decode_queue->size < issue_width);
	       entry = next_entry)
	    {
	      /* Get next entry before we mess with current entry */
	      next_entry = entry->next_entry;

#ifdef DEBUG_PROC
	      S_print_sint (stdout, entry->sint);
#endif
	      /* 
	       * If the instruction is ZERO_SPACE, increase the
	       * issue width for this cycle.
	       */
	      if (entry->sint->oper->flags & ZERO_SPACE)
		issue_width++;

	      /* Move to end of decode queue */
	      S_move_entry_before (decode_queue, entry, NULL);
	    }
	}
    }

  /* Initialize resource counters */
  branch_count = 0;
  ialu_op_count = 0;
  falu_op_count = 0;
  load_count = 0;
  store_count = 0;
  directive_count = 0;
  unclassified_count = 0;
  issue_count = 0;

  utilization_stats = NULL;

  /* Decode instructions until decode_queue empty or get blocked */
  for (entry = decode_queue->head; entry != NULL; entry = next_entry)
    {
      /* Get next entry before we mess with current entry */
      next_entry = entry->next_entry;

      /* Get sint and superscalar data for entry */
      sint = entry->sint;
      sdata = (Superscalar_Data *) sint->proc_data_v;

      /* Get Superscalar stats for ease of use */
      ss_stats = (Superscalar_Stats *) sint->stats->processor_v;

      /* Put utilization stats into the first sint's region */
      if (utilization_stats == NULL)
	utilization_stats = ss_stats;

      /* Get sint->oper flags for ease of use */
      opflags = sint->oper->flags;

      /* Get operand array for easy access */
      operand = sint->oper->operand;

      /* Assume sint is not blocked */
      sint_blocked = 0;

      /* Make sure all sources are ready */
      /* EMN */
      max_adjust = -10000;
      for (index = S_first_src; index <= S_last_src; index++)
	{
	  /* Get operand id, if > 0, it is a register */
	  id = operand[index];

	  /* If register and not ready, stop decode */
	  if ((id > 0) && !rf[id].value_avail)
	    {
	      sint_blocked = 1;
	      info = &opc_info_tab[rf[id].tag->oper->opc];
	      if (info->opc_type == LOAD_OPC)
		ss_stats->cycles_sint_blocked_on_pending_load++;
	      break;
	    }
	  /* EMN */
	  else if ((id > 0) && rf[id].value_avail)
	    {
	      if (rf[id].adjust > 0)
		{
		  printf ("warning %d should be <= 0\n", rf[id].adjust);
		  rf[id].adjust = 0;
		}
	      if (rf[id].adjust > max_adjust)
		max_adjust = rf[id].adjust;
	    }
	}
      /* EMN */
      if (max_adjust <= -10000)
	max_adjust = 0;

      /* Handle predicated instructions properly */
      if (opflags & PREDICATED)
	{
	  /* Get operand id of predicate */
	  id = operand[S_first_pred];

	  /* If not a register, error */
	  if (id <= 0)
	    S_punt ("Non-register predicate");

	  /* If predicate not ready, stop decode */
	  if (!rf[id].value_avail)
	    sint_blocked = 1;

	  /* 
	   * Read destinations of predicated instruction (if flaged) 
	   * This is to model one what to bypass values in a predicated
	   * architecture.  If S_read_dest_of_pred_op is set to 1,
	   * then the old value for each destination is always read
	   * and then if the operation is squashed, the old value
	   * is forwarded (simplifies bypassing).  If set to 0, then 
	   * some other bypassing hardware is assumed that can handle 
	   * predicated bypassing properly.
	   */
	  else if (S_read_dests_of_pred_op)
	    {
	      for (index = S_first_dest; index <= S_last_dest; index++)
		{
		  /* Get operand id, if > 0, it is a register */
		  id = operand[index];

		  /* 
		   * If register, not ready, and not a predicate
		   * register, stop decode 
		   * 
		   * AND and OR type predicates have special hardware
		   * to allow parallel definition of the same predicate.
		   * 
		   * Unconditional predicates write to the predicate
		   * even if operation is predicate squashed.  Therefore
		   * old value does not need to be read.
		   *
		   * (Conditional predicates would need to stop decode,
		   *  but are not currently generated. -JCG 1/28/98)
		   */
		  if ((id > 0) && !rf[id].value_avail &&
		      !(operand_tab[id]->flags & PREDICATE_OPERAND))
		    {
		      sint_blocked = 1;
		      break;
		    }
		}
	    }

	}

      /* Stop if sint_blocked */
      if (sint_blocked)
	break;


      /* Get opc info for ease of use */
      info = &opc_info_tab[sint->oper->opc];
      /*
         if (max_adjust != 0)
         {
         printf("orig: lat %d, adj %d, max %d",
         sint->oper->real_latency[S_first_dest],
         sint->oper->adjust_real_latency,
         max_adjust
         );
         }
       */
      /* EMN */
      if ((max_adjust + sint->oper->real_latency[S_first_dest]) > 0)
	{
	  sint->oper->real_latency[S_first_dest] += max_adjust;
	}
      else
	{
	  sint->oper->adjust_real_latency +=
	    sint->oper->real_latency[S_first_dest] + max_adjust - 1;
	  sint->oper->real_latency[S_first_dest] = 1;
	}
      if (sint->oper->adjust_real_latency > 0)
	{
	  printf (" lat %d, adj %d, max %d\n",
		  sint->oper->real_latency[S_first_dest],
		  sint->oper->adjust_real_latency, max_adjust);
	}

      /*
         if (max_adjust != 0)
         {
         printf("next: lat %d, adj %d, max %d\n",
         sint->oper->real_latency[S_first_dest],
         sint->oper->adjust_real_latency,
         max_adjust
         );
         }
       */

      /*
       * Check resource constraints here
       */
      opc_type = info->opc_type;

      /* Limit branches per cycle */
      if (info->is_branch)
	{
	  if (branch_count >= S_branches_per_cycle)
	    {
	      ss_stats->cycles_branch_unit_unavailable++;
	      break;
	    }

	  branch_count++;
	}

      /* 
       * Limit number of memory ports used,
       * and keep from bugging busy dcache.
       */
      else if (opc_type == LOAD_OPC)
	{
	  /* 
	   * See if any pending stores conflict with the address of
	   * this load.
	   */

	  if ((conflicting_store = S_find_conflicting_store (pnode,
							     sint)) != NULL)
	    {
	      /* 
	       * Store can forward to load if its data size is at least
	       * as big as the load's.  (conflict_mask will be at least
	       * as small as the load's.)
	       */
	      if (conflicting_store->conflict_mask <= sint->conflict_mask)
		{
		  /* 
		   * Flag that data will be forwarded from a pending 
		   * store so that dcache can be bypassd.
		   */
		  sint->flags |= VALUE_FORWARDED;
		  ss_stats->loads_forwarded++;

		  if (S_dcache_debug_misses)
		    {
		      fprintf (debug_out, "LD BYPASSED %s op %i  %x\n",
			       sint->oper->cb->fn->name,
			       sint->oper->lcode_id, sint->trace.mem_addr);
		    }

		}

	      /* Otherwise, need to block until store completes */
	      else
		{
		  /* Update stats on blocked loads we have not seen before */
		  if (!(sint->flags & BLOCKED_BEFORE))
		    {
		      ss_stats->loads_blocked++;
		    }
		  sint->flags |= BLOCKED_BEFORE;

		  /* Update stats on cycles blocked due to blocked loads */
		  ss_stats->cycles_loads_blocked++;
		  break;
		}

	    }
	  /* If load value not forwarded, make sure can go to dcache */
	  if (!(sint->flags & VALUE_FORWARDED))
	    {
	      if (pnode->dcache_busy)
		{
		  /* Update stats on cycles dcache busy when needed */
		  ss_stats->cycles_dcache_busy_when_needed++;
		  break;
		}

	      /* 
	       * If zero_sized dcache instruction, temporarily increase
	       * the number of dcache ports.
	       */
	      if (sint->oper->flags & ZERO_SPACE)
		dcache_ports++;

	      if (pnode->dcache_request->size >= dcache_ports)
		{
		  /* 
		   * Update stats on cycles when dcache ports needed
		   * but unavailable
		   */
		  ss_stats->cycles_dcache_ports_unavailable++;
		  break;
		}
	    }

	  /* Limit loads per cycle */
	  if (load_count >= S_loads_per_cycle)
	    {
	      ss_stats->cycles_load_unit_unavailable++;
	      break;
	    }

	  load_count++;

	}

      /* 
       * Limit dcache ports and keep prefetch or mem_copies from 
       * bugging busy dcache.
       */
      else if ((opc_type == PREFETCH_OPC) ||
	       (sint->flags & MEM_COPY_DIRECTIVE))
	{
	  if (pnode->dcache_busy)
	    {
	      /* Update stats on cycles dcache busy when needed */
	      ss_stats->cycles_dcache_busy_when_needed++;
	      break;
	    }

	  /* 
	   * If zero_sized dcache instruction, temporarily increase
	   * the number of dcache ports.
	   */
	  if (sint->oper->flags & ZERO_SPACE)
	    dcache_ports++;

	  if (pnode->dcache_request->size >= dcache_ports)
	    {
	      /* 
	       * Update stats on cycles when dcache ports needed
	       * but unavailable
	       */
	      ss_stats->cycles_dcache_ports_unavailable++;
	      break;
	    }

	  /* Limit cache directives per cycle */
	  if (directive_count >= S_cache_directives_per_cycle)
	    {
	      ss_stats->cycles_cache_directive_unit_unavailable++;
	      break;
	    }

	  directive_count++;
	}
      else if (opc_type == STORE_OPC)
	{
	  /* 
	   * If not delaying stores until retirement, get dcache
	   * port now.  Otherwise, dcache port will not be required
	   * until retirement. -JCG 2/17/98
	   */
	  if (!S_delay_stores_until_retirement)
	    {
	      if (pnode->dcache_busy)
		{
		  /* Update stats on cycles dcache busy when needed */
		  ss_stats->cycles_dcache_busy_when_needed++;
		  break;
		}

	      /* 
	       * If zero_sized dcache instruction, temporarily increase
	       * the number of dcache ports.
	       */
	      if (sint->oper->flags & ZERO_SPACE)
		dcache_ports++;

	      if (pnode->dcache_request->size >= dcache_ports)
		{
		  /* 
		   * Update stats on cycles when dcache ports needed
		   * but unavailable
		   */
		  ss_stats->cycles_dcache_ports_unavailable++;
		  break;
		}
	    }

	  /* Limit stores per cycle */
	  if (store_count >= S_stores_per_cycle)
	    {
	      ss_stats->cycles_store_unit_unavailable++;
	      break;
	    }

	  store_count++;
	}

      else if (opc_type == FALU_OPC)
	{
	  /* Limit falu_ops per cycle */
	  if (falu_op_count >= S_falu_ops_per_cycle)
	    {
	      ss_stats->cycles_falu_unit_unavailable++;
	      break;
	    }

	  falu_op_count++;
	}

      else if ((opc_type == MOVE_OPC) || (opc_type == IALU_OPC))
	{
	  /* Limit ialu_ops per cycle */
	  if (ialu_op_count >= S_ialu_ops_per_cycle)
	    {
	      ss_stats->cycles_ialu_unit_unavailable++;
	      break;
	    }

	  ialu_op_count++;
	}
      else
	{
	  unclassified_count++;
	}

      /* 
       * Assume infinite reorder buffer.
       */

      /*
       * After this point, this sint is guaranteed to issue
       */
      issue_count++;

      /* 
       * In the past, all memory sints where sent to the dcache, even
       * if they were OFF_PATH (i.e., they were due to a mispredicted
       * branch).  If the parm S_squash_off_path_dcache_requests is set
       * to true, then prevent OFF_PATH memory requests from going to
       * the dcache.  Treat them as cache hits as far as latency is 
       * concerned (should not matter since the branch misprediction
       * should be detected in the same cycle as the "address calculation"
       * anyway and the off-path operations will not complete anyway).
       * -JCG 2/5/98
       *
       * For ease of use, determine up front of whether to squash
       * the dcache request or send it to the dcache
       * (if it is a dcache request).
       */
      if ((sint->flags & OFF_PATH) && S_squash_off_path_dcache_requests)
	squash_request_because_off_path = 1;
      else
	squash_request_because_off_path = 0;

      /* 
       * If printing branch histograms then keep histograms 
       * statistics (for on-path instructions only).
       */
      if (S_print_branch_histograms && !(sint->flags & OFF_PATH))
	{
	  /* Care only about on-path branches (except untraced JSRs) */
	  if (info->is_branch && !(sint->flags & UNTRACED_JSR))
	    {
	      hstats = sint->stats->histogram;

	      if (sint->flags & MISPREDICTED)
		{
		  L_update_histogram (hstats->ops_between_mispredictions,
				      S_ops_between_mispredictions, 1.0);
		  L_update_histogram (hstats->cycles_between_mispredictions,
				      S_sim_cycle -
				      S_cycle_of_last_misprediction, 1.0);

		  S_ops_between_mispredictions = 0;
		  S_cycle_of_last_misprediction = S_sim_cycle;
		}
	      /* Increment ops between mispredictions if not mispredicted */
	      else
		{
		  S_ops_between_mispredictions++;
		}

	      L_update_histogram (hstats->ops_between_branches,
				  S_ops_between_branches, 1.0);
	      L_update_histogram (hstats->cycles_between_branches,
				  S_sim_cycle - S_cycle_of_last_branch, 1.0);

	      S_ops_between_branches = 0;
	      S_cycle_of_last_branch = S_sim_cycle;
	    }

	  /* Update stats for non-branch ops */
	  else
	    {
	      S_ops_between_branches++;
	      S_ops_between_mispredictions++;
	    }
	}

      /* 
       * Check to see if this instruction is changing simulation
       * state. (Must be on path)
       * Currently force_sim and stats_on/stats_off attributes
       * change simulation state.
       */

      /* Check for stats_on or stats_off */
      if (sint->flags & CHANGES_REGION)
	{
	  /* Updates the region cycle counts based on when the
	   * last sint in each region is sent to the execute stage.
	   */
	  S_update_region_cycle_counts (sint, S_sim_cycle);
	}

      /*
       * Assume register renaming, 
       * Tag dest registers with sint tag 
       *
       * If not delaying stores until retirement, don't mark 
       * post_increment stores dests as unavailable (they will
       * be ready earlier than normal).  Stores do not return! -JCG 1/17/98
       */
      if ((opc_type != STORE_OPC) || !S_delay_stores_until_retirement)
	{
	  for (index = S_first_dest; index <= S_last_dest; index++)
	    {
	      /* Get operand id, if > 0, it is a register */
	      id = operand[index];

	      /* If register, set tag, mark value unavailable */
	      if (id > 0)
		{
		  rf[id].tag = sint;
		  rf[id].value_avail = 0;
		  rf[id].adjust = 0;
		}
	    }
	}

      /* 
       * Mark sint's issue time
       */
      sdata->issue_time = S_sim_cycle;

      /*
       * Mark sint's completion cycle.  Use op latency.
       * (Currently only up to 3 destinations are supported.
       *  Checked at load time.)
       */
      for (i = S_first_dest; i <= S_last_dest; i++)
	{
	  sint->real_complete_time[i] =
	    S_sim_cycle + sint->oper->real_latency[i];
	}

      /* 
       * Put in reorder buffer if not cache directive and 
       * if not a store operation when we are not delaying stores 
       * until retirement. -JCG 2/17/98
       */
      if ((sint->flags & CACHE_DIRECTIVE) ||
	  ((opc_type == STORE_OPC) && !S_delay_stores_until_retirement))
	sdata->reorder_queue_entry = NULL;
      else
	sdata->reorder_queue_entry = S_enqueue (processor->reorder_queue,
						sint);

      /* Put memory load on dcache_request queue */
      if (opc_type == LOAD_OPC)
	{
	  /* 
	   * If load's value is forwarded from store, treat as
	   * a cache hit and use latency specified by machine description.
	   * (Used to use shorter latency but it just adds noise to system
	   *  and part of the latency is meant to be address calculation.)
	   * 
	   * If sint off-path and we are squashing off-path dcache requests,
	   * treat as cache hit.
	   */
	  if ((sint->flags & VALUE_FORWARDED) ||
	      squash_request_because_off_path)
	    {
	      sint->real_complete_time[S_first_dest] =
		S_sim_cycle + sint->oper->real_latency[S_first_dest];
	    }

	  /* Send load request to dcache. */
	  else
	    {
	      /* 
	       * Make loads complete time really far in the future,
	       * dcache will put correct time for when data is ready
	       */
	      sint->real_complete_time[S_first_dest] = (unsigned) -1;

	      /* Add sint entry to dcache request queue */
	      S_enqueue (pnode->dcache_request, sint);
	    }
	}

      /* Put prefetch on dcache request queue */
      else if (opc_type == PREFETCH_OPC)
	{
	  /* 
	   * If not squashing this prefetch request, send to dcache.
	   * Otherwise, it will be deleted below. -JCG 2/5/98 
	   */
	  if (!squash_request_because_off_path)
	    {
	      /* 
	       * Prefetch will never return.
	       */
	      sint->real_complete_time[S_first_dest] = (unsigned) -1;

	      /* Add sint entry to dcache request queue */
	      S_move_entry_before (pnode->dcache_request, entry, NULL);
	    }
	}
      /* 
       * Put mem copy directives on request queue,
       * DO NOT WANT SPECULATIVE VERSIONS OF THESE NOW.
       * ASSUME HARDWARE DETECTS SPEC VERSIONS
       */
      else if ((sint->flags & MEM_COPY_DIRECTIVE) &&
	       (!(sint->flags & OFF_PATH)))
	{
	  /* 
	   * Set complete time way in the future, will 
	   * be reset when completed.
	   */
	  sint->real_complete_time[S_first_dest] = (unsigned) -1;

	  /* Only mem_copy_checks will return, others treat like
	   * prefetches.
	   */
	  if (opc_type == MEM_COPY_CHECK_OPC)
	    {
	      S_enqueue (pnode->dcache_request, sint);
	    }
	  else
	    {
	      S_move_entry_before (pnode->dcache_request, entry, NULL);
	    }
	}

      /* Put memory stores on pending store queue */
      else if (opc_type == STORE_OPC)
	{
	  /* 
	   * If delaying stores until retirement, add store address
	   * to pending_store hash table to facilitate store forwarding.
	   */
	  if (S_delay_stores_until_retirement)
	    {
	      hash_index = (sint->trace.mem_addr >> 3) &
		(STORE_HASH_SIZE - 1);
	      S_enqueue (processor->pending_stores[hash_index], sint);
	    }

	  /* 
	   * Otherwise, send it to the dcache now if on-path
	   * (Otherwise, mark that it should be thrown away.) -JCG 2/18/98 
	   */
	  else
	    {
	      if (!(sint->flags & OFF_PATH))
		{
		  /* Move sint entry to dcache request queue */
		  S_move_entry_before (pnode->dcache_request, entry, NULL);
		}
	      else
		{
		  /* Mark that we should squash request because off path */
		  squash_request_because_off_path = 1;
		}
	    }
	}

      /* 
       * Move sint entry to exec queue (except for cache directives
       * which are moved to the dcache_request queue above) 
       */
      if (!(sint->flags & CACHE_DIRECTIVE))
	{
	  /* 
	   * Always put non-store operations in the execution queue.
	   * If a store operation, put it in only if delaying stores
	   * until retirement. -JCG 2/17/98
	   */
	  if ((opc_type != STORE_OPC) || S_delay_stores_until_retirement)
	    S_move_entry_before (processor->exec_queue, entry, NULL);
	}

      /* Delete squashed CACHE DIRECTIVES if necessary. */
      else if (squash_request_because_off_path ||
	       ((sint->flags & MEM_COPY_DIRECTIVE) &&
		(sint->flags & OFF_PATH)))
	{
	  S_dequeue (entry);
	  S_free_sint (sint);
	  /* Sanity check */
	  entry = NULL;
	  sint = NULL;
	}

    }

  /* 
   * Update statististics for what was issued this cycle in
   * stats region of first instruction issued.
   * If nothing issued, use stats region in pnode.
   */
  if (utilization_stats == NULL)
    utilization_stats = (Superscalar_Stats *) pnode->stats->processor_v;

  utilization_stats->issue_utilization[issue_count]++;
  utilization_stats->branch_utilization[branch_count]++;
  utilization_stats->ialu_op_utilization[ialu_op_count]++;
  utilization_stats->falu_op_utilization[falu_op_count]++;
  utilization_stats->load_utilization[load_count]++;
  utilization_stats->store_utilization[store_count]++;
  utilization_stats->cache_directive_utilization[directive_count]++;
  utilization_stats->unclassified_operations += unclassified_count;

  return;
}

/* Just empties the processor last_fetch_stage */
void
S_superscalar_debug_ifetch_stage (Pnode * pnode)
{
  Sq_entry *entry, *next_entry;
  Sint *sint;
  Superscalar *processor;

  processor = (Superscalar *) pnode->processor_v;

  for (entry = processor->last_fetch_stage->head; entry != NULL;
       entry = next_entry)
    {
      next_entry = entry->next_entry;

      /* Get sint for entry */
      sint = entry->sint;

      /* If mispredicted, fix branch */
      if (sint->flags & MISPREDICTED)
	S_superscalar_fix_mispredicted_branch (pnode, sint);

      S_dequeue (entry);
      S_free_sint (sint);
    }
}

int
S_sim_superscalar_scoreboard (Pnode * pnode, int pc, unsigned sim_count)
{
  Superscalar *processor;
  Icache *icache;
  Squeue **fetch_stage;
  unsigned halt_point, simulation_halted;
  int end_of_cycle_size;
  int i, max_consume;

  /* Get processor node and hint to put in register */
  processor = (Superscalar *) pnode->processor_v;
  icache = pnode->icache;

  /* Set processor on correct path and enable ifetch */
  processor->ifetch_pc = pc;
  processor->ifetch_addr = oper_tab[pc]->instr_addr;
  processor->on_path_pc = pc;
  processor->on_correct_path = 1;
  processor->untraced_fixup = 0;	/* Not in untraced fixup */
  processor->fixup_return_pc = 0;
  processor->ifetch_enabled = 1;
  processor->enable_ifetch_next_cycle = 0;

  /* 
   * Assume bus and memory are in correct mode.  Dangerous to 
   * mess with it.
   */

  /* Find exit point to halt processor at */
  halt_point = S_num_sim_on_path + sim_count;

  /* Set exit conditions to false */
  S_halting_simulation = 0;
  simulation_halted = 0;

  /*
   * If simulating MCB and skipping instructions between samples
   * then need to correct MCB as if a context switch occured.
   */
  if ((S_MCB_model != MCB_MODEL_NO_MCB) && (S_skip_size > 0))
    S_sim_MCB_context_switch (pnode, pc);

  if ((S_ALAT_model != ALAT_MODEL_NO_ALAT) && (S_skip_size > 0))
    S_sim_ALAT_context_switch (pnode, pc);


  while (!simulation_halted)
    {
      /* Increment simulation cycle */
      S_sim_cycle++;

#if (defined  DEBUG_PROC) || (defined DEBUG_IFETCH) || (defined DEBUG_NEW)
      fprintf (debug_out, "\ncycle %i:\n", S_sim_cycle);
#endif

      /* Simulate bus, must be done before memory or cache simulation */
      S_sim_bus ();

      /* 
       * Simulate memory, do before caches to give highest priority for
       * accesing the bus.
       */
      S_sim_memory_first_half_cycle ();

      /* Simulate L2 bus, must be done before cache simulations */
      S_sim_L2_bus ();

      /* 
       * Simulate L2 cache, do before first level caches to give highest
       * priority for accesing the L2 bus.
       */
      S_sim_L2_first_half_cycle (pnode);

      /* Enable ifetch if flaged */
      if (processor->enable_ifetch_next_cycle)
	{
	  /* Ignore if halting simulation */
	  if (!S_halting_simulation)
	    processor->ifetch_enabled = 1;

	  processor->enable_ifetch_next_cycle = 0;
	}

      /* 
       * If ifetch is enabled and icache is not busy,
       * see if should make a request from icache (depends on fetch model)
       */
      if (processor->ifetch_enabled && !pnode->icache_busy)
	{

	  /* 
	   * Request data from icache if at end of this cycle
	   * the ibuf size <= S_fetch_mark.
	   * The CONSERVATIVE model has S_fetch_mark == 0.
	   * The AGGRESSIVE model has S_fetch_mark >= 1 (usually issue_width)
	   *
	   * Get # of instructions in ibuf at end of cycle if
	   * nothing is fetched and a full issue width is 
	   * dispatched to the execution unit.  If any of the
	   * fetch stages are empty, an entire issue width will
	   * be consumed from the ibuf.  Otherwise, estimate
	   * as the space left in the decode queue.
	   */
	  max_consume = (S_issue_width - processor->decode_queue->size);
	  fetch_stage = processor->fetch_stage;
	  for (i = S_num_fetch_stages - 1; i > 0; i--)
	    {
	      if (fetch_stage[i]->size == 0)
		{
		  max_consume = S_issue_width;
		  break;
		}
	    }
	  end_of_cycle_size = processor->ibuf->size - max_consume;

	  /* 
	   * If this optimistic estimate is below the fetch mark,
	   * initiate a fetch.
	   */
	  if (end_of_cycle_size <= S_fetch_mark)
	    {
	      /* Request cache block for fetch addr (predicted path) */
	      pnode->icache_addr_requested = processor->ifetch_addr;

#ifdef DEBUG_IFETCH
	      printf ("   Icache: request for pc %i (addr %x).\n",
		      processor->ifetch_pc, pnode->icache_addr_requested);
#endif
	    }
	  /* Otherwise, flag that there is no request */
	  else
	    pnode->icache_addr_requested = 0;
	}

      /* Otherwise, flag that there is no request */
      else
	pnode->icache_addr_requested = 0;

      /* 
       * The order the icache and dcache are simulated determines
       * their priority for arbitrating for the bus.
       * (The one to go first has higher priority).
       * Currently, icache is given priority over dcache.
       */
      /* Simulate the split block icache */
      S_sim_icache_first_half_cycle (pnode);

      /* Simulate the blocking dcache */
      S_sim_dcache_first_half_cycle (pnode);

      S_sim_memory_second_half_cycle (pnode);
      S_sim_L2_second_half_cycle (pnode);
      S_sim_icache_second_half_cycle (pnode);
      S_sim_dcache_second_half_cycle (pnode);

#ifdef DEBUG_IFETCH
      if (pnode->icache_bytes_returned != 0)
	printf ("   Icache: %i bytes for pc %i returned.\n",
		pnode->icache_bytes_returned, pnode->ifetch_pc);
#endif

      /* Do retire stage simulation */
      S_superscalar_scoreboard_retire_stage (pnode);

      /* Do exec stage simulation */
      S_superscalar_scoreboard_exec_stage (pnode);

      /* Do decode stage simulation */
      S_superscalar_scoreboard_decode_stage (pnode);

      /* Do ifetch stage simulation */
      S_superscalar_ifetch_stage (pnode);

      /* Detect end of sample/program */
      if (((S_num_sim_on_path >= halt_point) &&
	   (!S_force_sim)) || S_end_of_program)
	{
	  /* Disable ifetch stage */
	  processor->ifetch_enabled = 0;

	  /* Flag that halting simulation */
	  S_halting_simulation = 1;
	}

      /* If halting processor, wait for processor to empty */
      if (S_halting_simulation)
	{

	  /* 
	   * JWS 20000221 Require that all queues be empty.
	   * This was breaking branch misprediction recovery,
	   * as inflight mispredictions in fetch stages
	   * were surviving skips.
	   */

	  int inflight;
	  int i;

	  inflight = ((processor->decode_queue->size != 0) ||
		      (processor->exec_queue->size != 0) ||
		      (processor->reorder_queue->size != 0));

	  for (i = 0; !inflight && (i < S_num_fetch_stages); i++)
	    inflight = (processor->fetch_stage[i]->size != 0);

	  if (!inflight)
	    simulation_halted = 1;
	}
      /* If debugging bus, print out this cycles activity */
      if (S_debug_bus)
	S_print_bus_state (debug_out);

      /* If debugging L2 bus, print out this cycles activity */
      if (S_debug_L2_bus)
	S_print_L2_bus_state (debug_out);
    }

  /* 
   * If halted in middle of untraced fixup code, set on_path_pc to
   * fixup return point.  Do not want to count rest of untraced fixup
   * code.  This case should be unlikely.
   */
  if (processor->untraced_fixup)
    {
      processor->on_path_pc = processor->fixup_return_pc;
      processor->untraced_fixup = 0;
    }

  return (processor->on_path_pc);
}
