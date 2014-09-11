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
 *      File:   s_vliw.c
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
#include "s_vliw.h"

/* VLIW_STALL_MODELs */
#define VLIW_STALL_MODEL_ON_USE		1
#define VLIW_STALL_MODEL_ON_ISSUE	2

#undef DEBUG_IFETCH

/* VLIW parameters */
char *S_vliw_stall_model_name = "on-use";
int S_vliw_fixed_length_instructions = 1;
int S_vliw_instruction_size = 4;	/* In bytes */
int S_vliw_debug_sched_errors = 0;
int S_vliw_support_interlocking = 0;


/* VLIW global variables */
int S_vliw_stall_model = VLIW_STALL_MODEL_ON_USE;
int S_stall_vliw_issue = 0;

L_Alloc_Pool *VLIW_pool = NULL;
L_Alloc_Pool *VLIW_Data_pool = NULL;
L_Alloc_Pool *VLIW_Stats_pool = NULL;

/* HCH 11/13/00: for tracking memory accesses */
S_Cb *cur_cb;
S_Loop *last_loop = NULL;
S_Loop *cur_loop = NULL;

int S_sim_vliw_static (Pnode * pnode, int pc, unsigned sim_count);

void
S_read_parm_vliw (Parm_Parse_Info * ppi)
{
  L_read_parm_s (ppi, "vliw_stall_model", &S_vliw_stall_model_name);
  L_read_parm_b (ppi, "vliw_fixed_length_instructions",
		 &S_vliw_fixed_length_instructions);
  L_read_parm_i (ppi, "vliw_instruction_size", &S_vliw_instruction_size);
  L_read_parm_b (ppi, "vliw_debug_sched_errors", &S_vliw_debug_sched_errors);
  L_read_parm_b (ppi, "vliw_support_interlocking",
		 &S_vliw_support_interlocking);
}

void
S_print_configuration_vliw (FILE * out)
{
  fprintf (out, "# STATIC VLIW PROCESSOR CONFIGURATION:\n");
  fprintf (out, "\n");
  fprintf (out, "%12s VLIW stall model used.\n", S_vliw_stall_model_name);
  if (S_vliw_support_interlocking)
    fprintf (out, "%12s", "Does");
  else
    fprintf (out, "%12s", "Does not");
  fprintf (out, " support interlocking.\n");

  if (S_vliw_fixed_length_instructions)
    fprintf (out, "%12s", "Fixed");
  else
    fprintf (out, "%12s", "Variable");
  fprintf (out, " length instructions.\n");

  if (S_vliw_fixed_length_instructions)
    {
      fprintf (out, "%12u byte fixed instruction size.\n",
	       S_vliw_instruction_size);
    }
  fprintf (out, "\n");
  fprintf (out, "%12u issue width.\n", S_issue_width);
  fprintf (out, "%12u fetch stages.\n", S_num_fetch_stages);
  fprintf (out, "%12u cycle branch misprediction penalty.\n",
	   S_num_fetch_stages + 1);
  fprintf (out, "\n");
}

void
S_layout_vliw_code ()
{
  S_Fn *fn;
  S_Oper *op, *head_op;
  int instr_addr;
  int branch_section;
  int op_count = 0;
  int i;

  head_op = NULL;

  /* Make sure vliw packet info available */
  if (!S_sched_info_avail)
    S_punt ("S_layout_vliw_code: scheduling info not present in code.");

  /* Get starting address for code */
  instr_addr = S_program_start_addr;

  /* Start not in branch section */
  branch_section = 0;

  /* 
   * For each vliw long word, give the first sub-operation in it the 
   * long words address, and the rest 0.
   *
   * For our current emulation support, all the branches must be after the 
   * other operations.  
   */
  for (fn = head_fn; fn != NULL; fn = fn->next_fn)
    {
      for (i = 0; i < fn->op_count; i++)
	{
	  op = &fn->op[i];

	  /* Set the start address for this the first op in the long word */
	  if (op->flags & START_PACKET)
	    {
	      op->instr_addr = instr_addr;
	      op_count = 1;
	      op->instr_size = 100000;	/* Debug, set correctly below */
	      head_op = op;
	    }
	  /* 
	   * For the rest of the ops in long word, set to 0 
	   * (for debugging purposes, since should not access this info 
	   *  for them).
	   */
	  else
	    {
	      op->instr_addr = 0;
	      op_count++;
	      op->instr_size = 0;
	    }

	  /*
	   * For our current emulation support, all branches must be after the 
	   * other operations.  Check that this is the case.
	   */
	  if (opc_info_tab[op->opc].is_branch)
	    {
	      branch_section = 1;
	    }
	  else
	    {
	      /* Check for non-branch in branch section */
	      if (branch_section)
		{
		  fprintf (stderr,
			   "S_layout_vliw_code: non-branch after branch in ");
		  fprintf (stderr, "vliw long word (%s op %i).\n",
			   op->cb->fn->name, op->lcode_id);
		  fprintf (stderr,
			   "In our current emulation environment, all branches");
		  fprintf (stderr,
			   "must occur after all non-branch operations.\n");
		  fprintf (stderr,
			   "This allows a superscalar trace to emulate all ");
		  fprintf (stderr,
			   "instructions in a vliw long word executing.\n");
		  S_punt ("Cannot continue: unsupported vliw schedule.");
		}
	    }

	  /* Increment address at end of longword */
	  if (op->flags & END_PACKET)
	    {
	      if (S_vliw_fixed_length_instructions)
		{
		  head_op->instr_size = S_vliw_instruction_size;
		  instr_addr += S_vliw_instruction_size;
		}
	      else
		{
		  head_op->instr_size = 4 * op_count;
		  instr_addr += 4 * op_count;
		}
	      branch_section = 0;
	    }
	}
    }
}

VLIW *
S_create_vliw (Pnode * pnode)
{
  VLIW *processor;
  Reg_File *reg;
  char name_buf[100];
  int reg_size;
  int i;


  /* 
   * Must have scheduling info available in order to do
   * VLIW simulation.
   */
  if (!S_sched_info_avail)
    {
      S_punt ("May not use VLIW model with unscheduled code.\n");
    }


  if (S_icache_block_size < S_vliw_instruction_size)
    {
      S_punt ("Icache block size (%i) must be >= instruction size (%i).",
	      S_icache_block_size, S_vliw_instruction_size);
    }

  /* Convert stall model  into integer */
  if (L_pmatch (S_vliw_stall_model_name, "on-issue"))
    {
      /* Reassign name to beautify output */
      S_vliw_stall_model_name = "On-issue";
      S_vliw_stall_model = VLIW_STALL_MODEL_ON_ISSUE;
    }
  else if (L_pmatch (S_vliw_stall_model_name, "on-use"))
    {
      /* Reassign name to beautify output */
      S_vliw_stall_model_name = "On-use";
      S_vliw_stall_model = VLIW_STALL_MODEL_ON_USE;
    }
  else
    {
      S_punt ("Invalid VLIW stall model '%s':  Use on-use or on-issue.",
	      S_vliw_stall_model_name);
    }

  /* Initialize alloc pool if not already  */
  if (VLIW_pool == NULL)
    {
      VLIW_pool = L_create_alloc_pool ("VLIW", sizeof (VLIW), 1);
      VLIW_Data_pool = L_create_alloc_pool ("VLIW_Data",
					    sizeof (VLIW_Data), 128);

    }

  processor = (VLIW *) L_alloc (VLIW_pool);
  processor->pnode = pnode;

  sprintf (name_buf, "VLIW %i", pnode->id);
  processor->name = strdup (name_buf);

  processor->flags = 0;
  processor->ifetch_pc = 0;
  processor->ifetch_addr = 0;
  processor->on_correct_path = 0;
  processor->flags = 0;

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

      /* Initialize value tracking fields for predicate-enhanced branch
       *  prediction 
       */
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

  /* Put vliw structure in pnode for debugging purposes only */
  pnode->vliw = processor;

  return (processor);
}

VLIW_Stats *
S_create_stats_vliw ()
{
  VLIW_Stats *stats;

  /* Initialize Stats pool if not already created */
  if (VLIW_Stats_pool == NULL)
    {
      VLIW_Stats_pool =
	L_create_alloc_pool ("VLIW_Stats", sizeof (VLIW_Stats), 1);
    }

  /* Create bus stats structure */
  stats = (VLIW_Stats *) L_alloc (VLIW_Stats_pool);

  /* Use STATS_ZERO(...) to initialize stats */
  STATS_ZERO (cycles_packet_unavailable);
  STATS_ZERO (total_virtual_latency_stalls);
  STATS_ZERO (load_virtual_latency_stalls);
  STATS_ZERO (dcache_busy_stalls);
  STATS_ZERO (interlock_stalls);
  STATS_ZERO (sched_error_stalls);
  STATS_ZERO (dest_not_avail_errors);
  STATS_ZERO (num_squashed);


  /* Return the initialized structure */
  return ((void *) stats);
}

void
S_add_stats_VLIW (VLIW_Stats * dest, VLIW_Stats * src1, VLIW_Stats * src2)
{
  STATS_ADD (cycles_packet_unavailable);
  STATS_ADD (total_virtual_latency_stalls);
  STATS_ADD (load_virtual_latency_stalls);
  STATS_ADD (dcache_busy_stalls);
  STATS_ADD (interlock_stalls);
  STATS_ADD (sched_error_stalls);
  STATS_ADD (dest_not_avail_errors);
  STATS_ADD (num_squashed);
}

void
S_print_stats_region_VLIW (FILE * out, Stats * stats,
			   char *rname, Stats * total_stats)
{
  VLIW_Stats *sv_stats;

  /* Setup Pstats calls */
  Pstats_out = out;
  Pstats_rname = rname;

  /* Get the bus stats structure for ease of use */
  sv_stats = (VLIW_Stats *) stats->processor_v;

  Pstats ("# STATIC VLIW PROCESSOR:");
  Pstats ("");
  Pstats ("%12u total virtual latency stalls.",
	  sv_stats->total_virtual_latency_stalls);
  Pstats ("%12u load virtual latency stalls.",
	  sv_stats->load_virtual_latency_stalls);
  Pstats ("%12u dcache busy stalls.", sv_stats->dcache_busy_stalls);
  Pstats ("%12u stalls due to interlock.", sv_stats->interlock_stalls);
  Pstats ("%12u VLIW scheduling errors stalls.",
	  sv_stats->sched_error_stalls);
  Pstats ("%12u cycles decoder idle (packets unavailable).",
	  sv_stats->cycles_packet_unavailable);

  Pstats ("");
  Pstats ("%12u dest not avail errors detected (for debugging).",
	  sv_stats->dest_not_avail_errors);
#if 0
  /* Speculation not currently done */
  Pstats ("%12u operations squashed.", sv_stats->num_squashed);
#endif

  Pstats ("");
}

int
S_sim_vliw (Pnode * pnode, int pc, unsigned sim_count)
{
  switch (S_processor_type)
    {
    case PROCESSOR_TYPE_STATIC:
      pc = S_sim_vliw_static (pnode, pc, sim_count);
      break;

    default:
      S_punt ("S_sim_vliw: Undefined processor type '%s'.",
	      S_processor_type_name);
    }

  /* Return the new pc */
  return (pc);
}

int
S_sim_vliw_static (Pnode * pnode, int pc, unsigned sim_count)
{
  VLIW *processor;
  Icache *icache;
  unsigned halt_point, simulation_halted;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int end_of_cycle_size;
#endif

  /* Get processor node and hint to put in register */
  processor = (VLIW *) pnode->processor_v;
  icache = pnode->icache;

  /* Set processor on correct path and enable ifetch */
  processor->ifetch_pc = pc;
  processor->ifetch_addr = oper_tab[pc]->instr_addr;
  processor->on_path_pc = pc;
  processor->on_correct_path = 1;
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
	   * Request data from icache if decode and ifetch buffers 
	   * are not both full (blocked last cycle).
	   */
	  if ((processor->ibuf->size == 0) ||
	      (processor->decode_queue->size == 0))
	    {
	      /* Request cache block for fetch pc (predicted path) */
	      pnode->icache_addr_requested = processor->ifetch_addr;

#ifdef DEBUG_IFETCH
	      printf ("   Icache: request for pc %i (addr %x).\n",
		      processor->ifetch_pc, pnode->icache_addr_requested);
#endif
	    }
	}

      /* Otherwise, flag that there is no request */
      else
	{
	  pnode->icache_addr_requested = 0;
	  pnode->icache_bytes_returned = 0;
	}

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
	printf ("   Icache: %i bytes returned for pc %i returned.\n",
		pnode->icache_bytes_returned, processor->ifetch_pc);
      fflush(stdout);
#endif

#if 0
      /* Do retire stage simulation */
      S_vliw_retire_stage (pnode);
#endif

      /* Do exec stage simulation */
      S_vliw_exec_stage (pnode);

      /* Do decode stage simulation */
      S_vliw_decode_stage (pnode);

      /* Do ifetch stage simulation */
      S_vliw_ifetch_stage (pnode);


      /* Detect end of sample/program */
      if (((S_num_sim_on_path >= halt_point) && (!S_force_sim)) || 
	  S_end_of_program)
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

      /* If debugging bus, print out this cycle's activity */
      if (S_debug_bus)
	S_print_bus_state (debug_out);

      /* If debugging L2 bus, print out this cycle's activity */
      if (S_debug_L2_bus)
	S_print_L2_bus_state (debug_out);
    }

  return (processor->on_path_pc);
}

void
S_create_vliw_data (Sint * sint)
{
  VLIW_Data *vdata;

  /* Generate the VLIW data needed for this sint */
  vdata = (VLIW_Data *) L_alloc (VLIW_Data_pool);
  sint->proc_data_v = (void *) vdata;
  sint->vliw = vdata;		/* for debugging */

  /* Point vdata back at sint it is for */
  vdata->sint = sint;

  /* Initialize vdata structure */
  vdata->flags = 0;
  vdata->reorder_queue_entry = NULL;
}

/* HCH 11/13/00:  Function added for code to track memory accesses  */
void 
S_add_loop_info(Sint * sint)
{
  int *cur_preheaders = NULL;
  int cur_cb_id;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int valid_preheader = 0;
#endif

  cur_cb = sint->oper->cb;
  cur_cb_id = cur_cb->lcode_id;
  cur_loop = sint->oper->loop;
  
  cur_preheaders = cur_cb->preheaders;
  
  /* 
   * sint is not part of a loop or is not in a header cb, and is not 
   * at the top of the cb -> do nothing 
   */
  if ( (cur_cb->start_pc != sint->oper->pc) ||
      !(cur_loop) || !(cur_preheaders) )
    { 
      /* Do nothing */   
    }

  /* 
   * At this point: in a loop, in the header cb for that loop, at start
   * of that header cb -> if still in same loop, increment iteration  
   */
  else if  (last_loop == cur_loop)
    {
      cur_loop->iter++;
    }

  /* At this point: in a loop, in the header cb for that loop, at start 
   * of that header cb, & not in the same loop we were last time */
  else  
    {
      cur_loop->instance++;   
      cur_loop->iter = 1;
    }  
  
  last_loop = cur_loop;
  return;
}

void
S_vliw_ifetch_stage (Pnode * pnode)
{
  VLIW *processor;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int num_avail, num_fetched;
#endif
  int pc, max_pc;
  Squeue **fetch_stage, *this_stage;
  Sq_entry *entry, *next_entry;
  Sint *sint;
  S_Opc_Info *info;
  int predicted_target;
  int on_correct_path, traced_op, stage;
  VLIW_Stats *vstats;
  static unsigned int cycle_of_last_ifetch = 0;
  int id, index;
  double dynamic_count;

  /* Get local variables for easy of use */
  processor = (VLIW *) pnode->processor_v;

  /* To detect incorrect use of stats pointers */
  vstats = NULL;

  /* Detect infinite loop */
  if ((cycle_of_last_ifetch + 1000) < S_sim_cycle)
    {
      fprintf (stderr,
	       "Deadlock detected in cycle %i, sample %i.\n",
	       S_sim_cycle, S_num_sim_samples);
      fprintf (stderr, "Have not fetched anything since cycle %i.\n",
	       cycle_of_last_ifetch);

      if (processor->exec_queue->head != NULL)
	{
	  sint = processor->exec_queue->head->sint;
	  fprintf (stderr,
		   "Instruction causing simulation deadlock (%s op %i):\n",
		   sint->oper->cb->fn->name, sint->oper->lcode_id);
	  S_print_sint (stderr, sint);
	  fprintf (stderr, "\n");
	  S_print_queues_in (stderr, sint);
	  dynamic_count = (double) S_num_skip_on_path;
	  dynamic_count += ((double) S_billions_skipped * (double) BILLION);
	  dynamic_count += (double) S_num_sim_on_path;
	  fprintf (stderr,
		   "Sint's serial_no %i, approx dynamic instruction # %2.0lf.\n",
		   sint->serial_no, dynamic_count);
	}
      else
	{
	  fprintf (stderr, "Execution queue empty!\n");
	}
      fprintf (stderr, "\n");

      if (pnode->dcache_busy)
	fprintf (stderr, "Dcache marked as busy!.\n");
      if (pnode->icache_busy)
	fprintf (stderr, "Icache marked as busy!.\n");
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

      S_punt ("Aborting simulation due to deadlock.");
    }

  /* 
   * Allow multiple fetch stages.  Allow instructions in
   * one fetch stage to advance to the next fetch stage, if
   * the next fetch stage is currently empty.  Does not
   * "pack" operations fetched in different cycles from stage 0
   * together.
   */
  fetch_stage = processor->fetch_stage;
  for (stage = S_num_fetch_stages - 1; stage > 0; stage--)
    {
      /* 
       * Move packet from previous stage to this fetch stage 
       * if this fetch stage is currently empty
       */
      this_stage = fetch_stage[stage];
      if (this_stage->size == 0)
	{
	  for (entry = fetch_stage[stage - 1]->head;
	       entry != NULL; entry = next_entry)
	    {
	      /* Get next entry before moving this one */
	      next_entry = entry->next_entry;

	      /* Move entry to end of this fetch stage */
	      S_move_entry_before (this_stage, entry, NULL);

	      /* Stop if end of packet */
	      if (entry->sint->oper->flags & END_PACKET)
		break;
	    }
	}
    }

  /* 
   * Do not fetch a long instruction if last long word was not consumed
   * or if ifetch is disabled.
   */
  if ((processor->ibuf->size > 0) || !(processor->ifetch_enabled))
    return;

  /* Get pc, max_pc, on_correct_path and untraced_fixup for ease of use */
  pc = processor->ifetch_pc;
  max_pc = S_max_pc;
  on_correct_path = processor->on_correct_path;

#ifdef DEBUG_IFETCH
  printf ("   Ifetch: pc = %i\n", processor->ifetch_pc);
  fflush(stdout);
#endif

  /* If icache returned data we don't need, stop now */
  if (pnode->icache_addr_requested != processor->ifetch_addr)
    return;

  /* If nothing returned by icache, stop now */
  if (pnode->icache_bytes_returned == 0)
    return;

  /* 
   * If fixed length instructions, make sure we got a whole instruction
   * back.
   */
  if (S_vliw_fixed_length_instructions)
    {
      /* 
       * We are forcing alignment, so must get back at least one long
       * word, punt if do not.
       */
      if (pnode->icache_bytes_returned < S_vliw_instruction_size)
	S_punt ("S_vliw_ifetch_stage: partial instr fetched (%i bytes).",
		pnode->icache_bytes_returned);
    }
  /* 
   * Otherwise, stall if didn't get a whole instruction back. 
   * (Don't care if at end of program.)
   */
  else if (pc <= max_pc)
    {
      /* 
       * For now, cheat.  Assume if get anything that get whole instruction.
       * Fix later.
       */
#if 0
      fprintf (debug_out, "pc %i returned %i, size %i.\n", pc,
	       pnode->icache_bytes_returned, oper_tab[pc]->instr_size);
      if (pnode->icache_bytes_returned < oper_tab[pc]->instr_size)
	{
	  return;
	}
#endif
    }

  /* Mark that ifetch successful in this cycle for deadlock detection */
  cycle_of_last_ifetch = S_sim_cycle;

  /* Initialize traced operation flag */
  traced_op = on_correct_path;

  /* 
   * Fetch all of longword into ibuf,
   * do not fetch past end of program
   */
  while (pc <= max_pc)
    {
      /* Create sint for sub-operation */
      sint = S_gen_sint (pnode, pc, traced_op);
      S_create_vliw_data (sint);

      /* 
       * Look at instructions in trace to get the next pc in the 
       * actual trace.
       * Operations after taken branch are not traced 
       */
      if (traced_op)
	{
	  if (sint->flags & BRANCHED)
	    {
	      processor->on_path_pc = sint->trace.target_pc;

	      /* No traced ops after taken branch */
	      traced_op = 0;
	    }
	  else
	    {
	      processor->on_path_pc = pc + 1;
	    }

	  /* 
	   * To faciliate analysis, track the dynamic cb that each
	   * sint is in.  Increment this "dynamic_cb_id" whenever
	   * a on-path sint enters a cb.
	   */
	  if (sint->oper->cb->start_pc == sint->oper->pc)
	    {
	      S_dynamic_cb_id++;
	    }

	  /* 
	   * For each on-path register destination, update
	   * the last_def_fetched field in the register file.
	   */
	  for (index = S_first_dest; index <= S_last_dest; index++)
	    {
	      /* Get operand id for dest */
	      id = sint->oper->operand[index];

	      /* If dest a register, update last_def_fetched */
	      if (id > 0)
		{
		  processor->reg_file[id].last_def_fetched = sint;
		}
	    }
	}
      /* 
       * Mark operations in long instruction as that were not traced
       * as off-path.
       */
      else
	{
	  sint->flags |= OFF_PATH;
	}

      /* 
       * Keep track of which dynamic cb this sint is in and which
       * cycle the sint was fetched in.
       */
      sint->dynamic_cb_id = S_dynamic_cb_id;
      sint->fetch_cycle = S_sim_cycle;

      /* Add sint to ibuf */
      S_enqueue (processor->ibuf, sint);

      /* Move pc to next operation */
      pc = pc + 1;

      /* Get opc info for easy access */
      info = &opc_info_tab[sint->oper->opc];
      
      /* HCH 11/13/00: track memory accesses  */
      
      if( !(sint->flags & OFF_PATH) && !(sint->flags & PRED_SQUASHED) && 
	  !(sint->flags & MASKED_SEG_FAULT) && !(sint->flags & MASKED_BUS_ERROR) &&
	  S_trace_objects) 
	{
	  /* Set loop_instance and loop_iter values  */
	  S_add_loop_info(sint);
	  
	  /* Print out information about stores and loads through s_object.c */
	  if(info->opc_type == STORE_OPC) {
	    S_dump_store(sint);
	  }
	  
	  if(info->opc_type == LOAD_OPC) {
	    S_dump_load(sint);
	  }
	} 
      
      /* If not at the end of the instruction, get next operation */
      if (sint->oper->flags & END_PACKET)
	break;
    }

  /* Do branch prediction for long word to get next pc to fetch */
  for (entry = processor->ibuf->head; entry != NULL;
       entry = entry->next_entry)
    {
      sint = entry->sint;

      /* Get opc info for easy access */
      info = &opc_info_tab[sint->oper->opc];
      
      /* 
       * Do branch prediction if a branch.
       * Ignore untraced jsr's for now. 
       */
      if (info->is_branch && !(sint->flags & UNTRACED_JSR))
	{
	  predicted_target = S_get_BTB_prediction (pnode, sint);

	  /* If mispredicted branch, mark that we're off the correct path */
	  if (sint->flags & MISPREDICTED)
	    {
	      on_correct_path = 0;
	    }

	  /* 
	   * If predict branched, change pc to predicted target.
	   * (otherwise, pc will point to next instruction).
	   * Do not predict branches after the first predicted taken.
	   */
	  if (predicted_target != -1)
	    {
	      pc = predicted_target;
	      break;
	    }
	}
    }

  /* Update mem copy of ifetch_pc, on_correct_path, untraced_fixup  */
  processor->ifetch_pc = pc;
  /* Handle end of program case properly (pc == 0) */
  if (oper_tab[pc] != NULL)
    processor->ifetch_addr = oper_tab[pc]->instr_addr;
  else
      processor->ifetch_addr = 0;

  /* Debug */
  if (on_correct_path && (processor->ifetch_addr == 0) && !S_end_of_program)
    {
      for (entry = processor->ibuf->head; entry != NULL;
	   entry = entry->next_entry)
	{
	  sint = entry->sint;
	  fprintf (stdout, "%i: ", sint->oper->pc);
	  S_print_sint (stdout, sint);
	}
      S_punt ("Algorithm error: pc %i fetching addr 0.", pc);
    } 

  processor->on_correct_path = on_correct_path;
}


/* Just empties the processor ibuf */
void
S_vliw_debug_ifetch_stage (Pnode * pnode)
{
  Sq_entry *entry, *next_entry;
  Sint *sint;
  VLIW *processor;

  processor = (VLIW *) pnode->processor_v;

  for (entry = processor->ibuf->head; entry != NULL; entry = next_entry)
    {
      next_entry = entry->next_entry;

      /* Get sint for entry */
      sint = entry->sint;

      /* If mispredicted, fix branch */
      if (sint->flags & MISPREDICTED)
	{
	  S_vliw_fix_mispredicted_branch (pnode, sint);

	  /* fix_mispredicted branch clears rest of ibuf */
	  break;
	}

      S_dequeue (entry);
      S_free_sint (sint);
    }
}


void
S_vliw_fix_mispredicted_branch (Pnode * pnode, Sint * mispred_sint)
{
  VLIW *processor;
  Sq_entry *entry, *next_entry;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  short *operand;
  int index, id;
#endif
  Reg_File *rf;
  Sint *sint;
  VLIW_Data *vdata;
  int i;

  /* Get processor for ease of use */
  processor = (VLIW *) pnode->processor_v;

  /* Get VLIW data for mispredicted sint for ease of use */
  vdata = (VLIW_Data *) mispred_sint->proc_data_v;

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

#if 0
  /* 
   * Marks entries after sint in reorder buf as SQUASHED,
   * Clear the destination entries in register file.
   */
  for (entry = vdata->reorder_queue_entry->next_entry;
       entry != NULL; entry = entry->next_entry)
    {
      sint = entry->sint;
      sint->flags |= SQUASHED;
      operand = sint->oper->operand;

      /* Update squashed stats for the region the squashed sint is in */
      ((VLIW_Stats *) sint->stats->processor_v)->num_squashed++;

      for (index = S_first_dest; index <= S_last_dest; index++)
	{
	  /* Get operand id, if >0, it is a register */
	  id = operand[index];

	  /* If register, mark available (will fix below) */
	  if (id > 0)
	    {
	      rf[id].tag = NULL;
	      rf[id].value_avail = COMPLETED;
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
		}
	    }
	}
    }

#endif

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
S_print_vliw_sched_error (FILE * out, Sint * consumer, Sint * producer,
			  int dest)
{
  int virtual_latency, dynamic_distance;

  virtual_latency = producer->oper->virtual_latency[dest];
  dynamic_distance = S_sim_cycle -
    (producer->virtual_complete_time[dest] - virtual_latency);
  fprintf (out, "VLIW scheduling error from:\n");
  S_print_sint (out, producer);
  fprintf (out, "\nto:\n");
  S_print_sint (out, consumer);
  fprintf (out,
	   "\nDynamic distance %i, Virtual latency %i.\n",
	   dynamic_distance, virtual_latency);
}

void
S_virtual_latency_stall_on (Sint * producer)
{
  VLIW_Stats *vstats;

  /* Update stats in region of sint that produced the virtual latency stall */
  vstats = (VLIW_Stats *) producer->stats->processor_v;

  vstats->total_virtual_latency_stalls++;

  if (opc_info_tab[producer->oper->opc].opc_type == LOAD_OPC)
    vstats->load_virtual_latency_stalls++;

  return;
}

void
S_vliw_decode_stage (Pnode * pnode)
{
  VLIW *processor;
  Sq_entry *entry, *next_entry;
  Squeue *decode_queue;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  Sint *conflicting_store;
  int hash_index, branch_count, sint_blocked, issue_width, dcache_ports;
  Histogram_Stats *hstats;
#endif
  Sint *sint, *producer;
  Reg_File *rf;
  S_Opc_Info *info;
  int index;
  short *operand, id;
  unsigned opflags;
  VLIW_Stats *vstats;
  VLIW_Data *vdata;
  int stall_model;
  int i, dest;

  /* Do nothing if stalling issue */
  if (S_stall_vliw_issue)
    {
      /* Clear flag */
      S_stall_vliw_issue = 0;
    }

  /* Get processor for ease of use */
  processor = (VLIW *) pnode->processor_v;

  /* Get stall model for ease of use */
  stall_model = S_vliw_stall_model;

  /* Get the decode queue for ease of use */
  decode_queue = processor->decode_queue;

  /* Prevent unexpected use of region pointers */
  vstats = NULL;

  /* Get register file for ease of use */
  rf = processor->reg_file;


  /* 
   * If there is not already a packet in the decode stage, move one 
   * in from the last fetch stage and shift the packets down the
   * fetch stages.
   */
  if (processor->decode_queue->size == 0)
    {
      if (processor->last_fetch_stage->size > 0)
	{
	  /* Move a packet from last_fetch_stage into decode_queue */
	  for (entry = processor->last_fetch_stage->head; entry != NULL;
	       entry = next_entry)
	    {
	      /* Get next entry before moving this one */
	      next_entry = entry->next_entry;

	      /* Move entry to end of decode_queue */
	      S_move_entry_before (processor->decode_queue, entry, NULL);

	      /* Stop if end of packet */
	      if (entry->sint->oper->flags & END_PACKET)
		break;
	    }

	  /* The last entry in the decode_queue must be an end of packet */
	  if (!(processor->decode_queue->tail->sint->oper->flags &
		END_PACKET))
	    S_punt ("S_vliw_decode_stage: END_PACKET expected.");
	}

    }

  /* If don't have packet available, cannot decode packet yet */
  if (processor->decode_queue->size == 0)
    {
      /* The pnode should have the latest region */
      vstats = (VLIW_Stats *) pnode->stats->processor_v;

      vstats->cycles_packet_unavailable++;
      return;
    }

  /*
   * Make sure that all the packet's sources are available.
   * Check both virtual time and real time. 
   *
   * If not available in virtual time then have scheduling error.
   * If not available in real time then need to stall or have
   * stall algorithm error.
   */
  for (entry = processor->decode_queue->head; entry != NULL;
       entry = entry->next_entry)
    {
      /* Get sint and VLIW data for entry */
      sint = entry->sint;
      vdata = (VLIW_Data *) sint->proc_data_v;

      /* Get VLIW stats for ease of use */
      vstats = (VLIW_Stats *) sint->stats->processor_v;

      /* Get sint->oper flags for ease of use */
      opflags = sint->oper->flags;

      /* Get operand array for easy access */
      operand = sint->oper->operand;

      /* Get opc info for ease of use */
      info = &opc_info_tab[sint->oper->opc];

      if (opflags & PREDICATED)
	{
	  /* Get operand id of predicate */
	  id = operand[S_first_pred];

	  /* If not a register, error */
	  if (id <= 0)
	    S_punt ("Non-register predicate");

	  /* If predicate not ready, determine why */
	  if (!rf[id].value_avail)
	    {
	      producer = rf[id].tag;
	      /* determine corresponding dest of producer */
	      dest = -1;
	      for (i = S_first_dest; i <= S_last_dest; i++)
		{
		  if (producer->oper->operand[i] == id)
		    dest = i;
		}
	      if (dest == -1)
		S_punt ("corresponding dest of producer not found");
	      if (producer->virtual_complete_time[dest] > S_sim_cycle)
		{
		  if (S_vliw_support_interlocking)
		    {
		      vstats->interlock_stalls++;
		    }
		  else
		    {
		      if (S_vliw_debug_sched_errors)
			{
			  S_print_vliw_sched_error (debug_out, sint,
						    producer, dest);
			}
		      vstats->sched_error_stalls++;
		    }
		  return;
		}

	      if (stall_model == VLIW_STALL_MODEL_ON_USE)
		{
		  S_virtual_latency_stall_on (producer);
		  return;
		}
	      else if (stall_model == VLIW_STALL_MODEL_ON_ISSUE)
		{
		  S_punt ("VLIW stall on issue algorithm error.");
		}
	      else
		{
		  S_punt ("Unknown stall model %i.\n", stall_model);
		}
	    }
	}

      /* Do not need to check sources of predicate squashed sints */
      if (!(sint->flags & PRED_SQUASHED))
	{
	  /* Make sure all sources are ready */
	  for (index = S_first_src; index <= S_last_src; index++)
	    {
	      /* Get operand id, if > 0, it is a register */
	      id = operand[index];

	      /* If register and not ready, find out why */
	      if ((id > 0) && !rf[id].value_avail)
		{
		  producer = rf[id].tag;
		  /* determine corresponding dest of producer */
		  dest = -1;
		  for (i = S_first_dest; i <= S_last_dest; i++)
		    {
		      if (producer->oper->operand[i] == id)
			dest = i;
		    }
		  if (dest == -1)
		    S_punt ("corresponding dest of producer not found");
		  if (producer->virtual_complete_time[dest] > S_sim_cycle)
		    {
		      if (S_vliw_support_interlocking)
			{
			  vstats->interlock_stalls++;
			}
		      else
			{
			  if (S_vliw_debug_sched_errors)
			    {
			      S_print_vliw_sched_error (debug_out, sint,
							producer, dest);
			    }
			  vstats->sched_error_stalls++;
			}
		      return;
		    }

		  if (stall_model == VLIW_STALL_MODEL_ON_USE)
		    {
		      S_virtual_latency_stall_on (producer);
		      return;
		    }
		  else if (stall_model == VLIW_STALL_MODEL_ON_ISSUE)
		    {
		      S_punt ("VLIW stall on issue algorithm error.");
		    }
		  else
		    {
		      S_punt ("Unknown stall model %i.\n", stall_model);
		    }
		}
	    }


	  /* Make sure dcache is not busy for cache directives */
	  if ((pnode->dcache_busy) &&
	      ((sint->flags & CACHE_DIRECTIVE) ||
	       (info->opc_type == LOAD_OPC) ||
	       (info->opc_type == STORE_OPC) ||
	       (info->opc_type == PREFETCH_OPC)))
	    {
	      vstats->dcache_busy_stalls++;
	      return;
	    }
	}
    }

  /* At this point, have a VLIW packet that has all data dependences
   * satisfied.
   */


  /* 
   * Set the sint's virtual and actual completion times.
   * Throw away all OFF_PATH sints.  How to VLIW speculation
   * is a unsolved research topic at this point.  Assume that
   * the processor has a deeply pipelined fetch and decode stages
   * so that all these speculative operations can be flushed before
   * starting to execute.
   */
  for (entry = processor->decode_queue->head; entry != NULL;
       entry = next_entry)
    {
      /* Get next entry before we move this entry */
      next_entry = entry->next_entry;

      /* Get sint and VLIW data for entry */
      sint = entry->sint;
      vdata = (VLIW_Data *) sint->proc_data_v;

      /* Throw away off path operations */
      if (sint->flags & OFF_PATH)
	{
	  /* Remove from exec queue */
	  S_dequeue (entry);

	  /* For now, free sint */
	  S_free_sint (sint);
	}

      /* 
       * Otherwise, if not an untraced jsr (dests never used because
       * jsr not taken), mark destinations as unavailable, and 
       * put into executione stage.
       * Do not mark post_increment stores dest as unavailable
       * right now.  Stores do not return! (Fix later)
       */
      else
	{
	  /* Get opc info for ease of use */
	  info = &opc_info_tab[sint->oper->opc];

	  /* Update stats of sint is on region boundary */
	  if (sint->flags & CHANGES_REGION)
	    {
	      /* Updates the region cycle counts based on when the
	       * last sint in each region is sent to the execute stage.
	       */
	      S_update_region_cycle_counts (sint, S_sim_cycle);
	    }

	  /* 
	   * Mark destinations as unavailable, 
	   * Do not mark post_increment stores dest as unavailable
	   * right now.  Stores do not return! (Fix later)
	   */
	  operand = sint->oper->operand;
	  if (!(sint->flags & UNTRACED_JSR) &&
	      !(sint->flags & PRED_SQUASHED) && (info->opc_type != STORE_OPC))
	    {
	      for (index = S_first_dest; index <= S_last_dest; index++)
		{
		  id = operand[index];

		  /* If have a register dest (id == 0 is NULL dest) */
		  if (id > 0)
		    {
		      /* Sanity check, dest better be available
		       * Just ignore if a predicate operand, since
		       * wired OR/AND support is assumed.
		       */
		      if ((rf[id].value_avail != COMPLETED) &&
			  !(operand_tab[id]->flags & PREDICATE_OPERAND))
			{
			  /* determine corresponding dest of producer */
			  dest = -1;
			  for (i = S_first_dest; i <= S_last_dest; i++)
			    {
			      if (rf[id].tag->oper->operand[i] == id)
				dest = i;
			    }
			  if (dest == -1)
			    S_punt
			      ("corresponding dest of producer not found");
			  if (S_vliw_debug_sched_errors)
			    {
			      S_print_vliw_sched_error (debug_out, sint,
							rf[id].tag, dest);
			    }

			  /* Update stats */
			  vstats->dest_not_avail_errors++;
			}


		      /* Mark register value as unavailable */
		      rf[id].tag = sint;
		      rf[id].value_avail = 0;

		    }
		}
	    }

	  /* Get the info for this sint */
	  info = &opc_info_tab[sint->oper->opc];

	  /* 
	   * For now, just issue them all the the uniform function units.
	   * Except load/stores now goto cache
	   */
	  vdata->issue_time = S_sim_cycle;

	  /* 
	   * Mark the scheduled complete time for all operations.
	   * Uniform function units for all operations (special cases
	   * handled below).
	   */
	  for (i = S_first_dest; i <= S_last_dest; i++)
	    {
	      sint->real_complete_time[i] =
		S_sim_cycle + sint->oper->real_latency[i];
	      sint->virtual_complete_time[i] =
		S_sim_cycle + sint->oper->virtual_latency[i];
	    }


	  /* 
	   * Loads/mem_copy_check real complete time is set by dcache 
	   * simulation.
	   */
	  if ((info->opc_type == LOAD_OPC) ||
	      (info->opc_type == MEM_COPY_CHECK_OPC))
	    {
	      /* 
	       * Make loads/mem_copy_checks  complete time really far in 
	       * the future, dcache will put correct time for when 
	       * data is ready
	       */
	      sint->real_complete_time[S_first_dest] = (unsigned) -1;

	      /* Add sint entry to dcache request queue */
	      S_enqueue (pnode->dcache_request, sint);
	    }

#if 0
	  /* Put in reorder queue */
	  vdata->reorder_queue_entry = S_enqueue (processor->reorder_queue,
						  sint);
#endif

	  /* 
	   * If cache directive, store, or prefetch, no result comes back
	   * to processor, and it should not be put in the exec queue.
	   *
	   * SPECULATIVE CACHE DIRECTIVES ARE NOT ALLOWED!  If a 
	   * speculative VLIW is written, there must be special
	   * hardware (code) that prevents off-path cache directives from
	   * going to dcache.
	   */
	  if ((sint->flags & CACHE_DIRECTIVE) ||
	      (info->opc_type == STORE_OPC) ||
	      (info->opc_type == PREFETCH_OPC))
	    {
	      /* These instructions will never return */
	      sint->real_complete_time[S_first_dest] = (unsigned) -1;

	      /* Add sint entry to dcache request queue */
	      S_move_entry_before (pnode->dcache_request, entry, NULL);
	    }

	  /* Otherwise, put in execution unit waiting for completion */
	  else
	    {
	      /* Put at head of exec queue */
	      S_move_entry_before (processor->exec_queue, entry, NULL);
	    }
	}
    }
}

/* 
 * Update register file and frees sints when completion_time is reached
 */
void
S_vliw_exec_stage (Pnode * pnode)
{
  Sq_entry *entry, *next_entry;
  Sint *sint;
  int index;
  int completed, virtual_completed;
  short *operand, id;
  Reg_File *rf, *reg;
  VLIW *processor;
  VLIW_Data *vdata;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  VLIW_Stats *vstats;
#endif
  int stall_model;

  /* Get stall model for ease of use */
  stall_model = S_vliw_stall_model;

  /* Get register file for ease of use */
  processor = (VLIW *) pnode->processor_v;
  rf = processor->reg_file;

  /* 
   * Look at all sints in the execute stage and processor those
   * that have completed.
   */
  for (entry = processor->exec_queue->head; entry != NULL; entry = next_entry)
    {
      next_entry = entry->next_entry;

      /* Get sint for entry and processor data for ease of use */
      sint = entry->sint;
      vdata = (VLIW_Data *) sint->proc_data_v;

      /* Make destinations as available if tag matches */
      completed = 1;
      virtual_completed = 0;
      operand = sint->oper->operand;
      for (index = S_first_dest; index <= S_last_dest; index++)
	{
	  id = operand[index];

	  /* Only care about register destinations */
	  if (id <= 0)
	    continue;

	  /* 
	   * If this destination is ready, update the scoreboard 
	   * (if necessary)
	   */
	  if (sint->real_complete_time[index] <= S_sim_cycle)
	    {
	      reg = &rf[id];
	      if (reg->tag == sint)
		reg->value_avail = COMPLETED;

	      /* 
	       * If this in an on-path operation, update the
	       * register file fields for tracking register values
	       * (currently used for predicate-enhanced branch prediction).
	       */
	      if (!(sint->flags & OFF_PATH))
		{
		  /* 
		   * If this was the last def fetched for this register,
		   * clear the last_def_fetched field.
		   */
		  if (reg->last_def_fetched == sint)
		    {
		      reg->last_def_fetched = NULL;
		    }

		  /* Keep track of last oper to update this register */
		  reg->value_cb_id = sint->dynamic_cb_id;
		  reg->value_fetch_cycle = sint->fetch_cycle;
		  reg->value_oper = sint->oper;

		  /* 
		   * For predicate defines, update register file
		   * with value written.
		   */
		  if (sint->oper->flags & PRED_DEF)
		    {
		      /* Detect if updating dest[0] or dest[1] */
		      if (index == S_first_dest)
			{
			  if (sint->flags & PRED_DEST0_SET)
			    reg->accessible_value = 1;
			  else
			    reg->accessible_value = 0;
			}

		      /* Assumes max of two pred dests */
		      else
			{
			  if (sint->flags & PRED_DEST1_SET)
			    reg->accessible_value = 1;
			  else
			    reg->accessible_value = 0;
			}
		    }
		}
	    }

	  /* 
	   * Otherwise, mark the operation as not being done and see
	   * if this destination would be completed if the processor
	   * was using the same latencies as the code was scheduled
	   * for.  If so, flag that oper has been virtual_completed.
	   */
	  else
	    {
	      completed = 0;
	      if (sint->virtual_complete_time[index] <= S_sim_cycle)
		virtual_completed = 1;
	    }
	}

      /* Is the real complete time up for this sint ? */
      if (completed)
	{
	  /* If mispredicted (on path branch), fix branch */
	  if ((sint->flags & MISPREDICTED) && !(sint->flags & OFF_PATH))
	    S_vliw_fix_mispredicted_branch (pnode, sint);

	  /* Mark sint as completed */
	  sint->flags |= COMPLETED;

	  /* Remove from exec queue */
	  S_dequeue (entry);

	  /* For now, free sint */
	  S_free_sint (sint);
	}

      /* If not, is the virtual complete time up for this sint ? */
      else if (virtual_completed)
	{
	  /* 
	   * If stall on issue, mark that issue should not happen 
	   * this cycle.  (Do if not already stalled).
	   */
	  if ((stall_model == VLIW_STALL_MODEL_ON_ISSUE) &&
	      (S_stall_vliw_issue == 0))
	    {
	      S_stall_vliw_issue = 1;

	      /* Update stats */
	      S_virtual_latency_stall_on (sint);
	    }
	}
    }
}
