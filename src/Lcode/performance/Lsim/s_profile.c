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
 *      File:   s_profile.c
 *      Author: Teresa Johnson and John Gyllenhaal
 *      Creation Date:  1994
 *      Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and
 *                         The Board of Trustees of the University of Illinois.
 *                         All rights reserved.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
  "@(#) Copyright (c) 1994 Teresa Johnson, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#define DEBUG_PROFILER 0

#include <math.h>
#include "s_main.h"
#include "s_profile.h"
#include "s_mem_profile.h"
#include "s_hash.h"
#include <library/dynamic_symbol.h>
#include <library/attr_mngr.h>
#include <library/l_histogram.h>

#define MR "dmr"
#define RR "drr"
#define SRR "srr"
#define TRR "trr"
#define MC "dmc"
#define PMC "pmc"
#define LMC "lmc"
#define PLAYDOH_TCHS_ALL 0x000000F0

#define HASH_SIZE 6

#define PCACHE_MULT 2

#define MAX_FILE_ALIASES 10000

#define S_min(a,b) (a < b ? a : b);
#define S_max(a,b) (a > b ? a : b);
#define JSR_Macroblock	6

/* Function Declarations */
static void *S_create_Profile_data ();
static void *S_create_Distr_data ();
void S_print_distr ();
void S_init_distr ();
S_MemDep_Alias_Info *S_insert_alias_info (S_MemDep_Alias_Info * alias_info,
					  int last_store_pc, int alias_times,
					  int *num_alias);
void S_read_alias_info (S_Fn * fn, int pc);
void S_consolidate_aliases_file ();
void S_incr_temporal_reuses (Profile_data * data, int block_size);
void S_incr_all_temp_reuses (Scache * cache);
void S_create_store_alias (int sink_pc, S_MemDep_Data_Info * data_info);
void S_create_load_alias (int source_pc, int sink_pc);


/* JCG branches attributes depend on the model used */
static char *branch_attr = NULL;
static char *branch_exec_attr = NULL;
static char *pred_attr = NULL;
static char *exec_attr = "EXEC";

/* JCG 6/8/95 */
static char *pred_dest0_set_attr = NULL;
static char *pred_dest1_set_attr = NULL;
static char *promoted_pred_attr = NULL;

/* Flags for S_Profile_Info */
#define JSR		0x00000004
#define PREFETCH	0x00000008

/* Added by JCG 8/16/94 because needed while Teresa on vacation.
 * Please excuse my hack.
 */
#define PROF_BRANCH		0x00000008
#define PROF_PREDICATE		0x00000010
#define PROF_PRED_DEF		0x00000020
#define PROF_PROMOTED		0x00000040
#define PROMOTED_DATA_INVALID  	0x00000080	/* Happens if code is scheduled */
#define PROF_ICACHE		0x00000100
#define PROF_INSTR		0x00000200	/* Indicates instr is profiled */

#define INTERVAL_MATCH		1
#define INTERVAL_OVERLAP	2
#define INTERVAL_LESS		3
#define INTERVAL_GREATER	4

/* Parameters read from SIM_PARMS */
char *S_annot_name = NULL;
char *S_mem_dep_name = NULL;
char *S_distr_file_name = NULL;

/* Parameters for memory dependence profiling */
char *S_mem_dep_guide_file_name = "guide";
char *S_mem_dep_profile_file_name = "database";
FILE *mem_dep_guide_file = NULL;
FILE *mem_dep_profile_file = NULL;
int S_mem_dep_model = MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT;
char *S_mem_dep_model_name = "unguided";
char *S_mem_dep_guide_model_name = "local";
int S_mem_dep_guide_model = MEM_DEP_GUIDE_MODEL_LOCAL;

/* Switches to indicate what to profile */
int S_profile_cycle_counts = 0;
int S_profile_cycle_counts_only = 0;
int S_profile_memory_accesses = 0;
int S_profile_branches = 0;
int S_profile_predicates = 0;
int S_profile_pred_defs = 0;
int S_profile_promoted_predicates = 0;
int S_mem_dep_profiling = 0;
int S_profile_issue_weight = 0;
int S_profile_icache_misses = 0;

int S_clear_addrs_on_overflow = 1;

int S_distribution = 0;
int S_distr_base_addr = 0;
int S_distr_entry_size = 0;
int S_distr_entries = 0;
int S_distr_time_delta = 0;
int S_distr_time_start = 0;
int S_distr_time_end = 0;

FILE *distr_file = NULL;
FILE *annot_file = NULL;
FILE *mem_dep_file = NULL;
FILE *tmp_file = NULL;
char *tmp_name;
double S_num_profiled;
long int S_num_loads, S_num_stores, S_num_prefetches;
long int S_num_loads_secondary, S_num_misses_secondary;
long int S_num_misses, S_num_misses_pref, S_num_misses_stores;
long int S_num_misses;
long int S_num_icache_misses;
long int S_num_prof_samples;
long int S_num_branches, S_num_predicates, S_num_pred_defs, S_num_promoted;
long int S_num_branch_mispredictions;
long int S_stats_region_ctr, S_region_cycles_ctr,
  S_region_billions_cycles_ctr;
long int srr_window;
struct S_Stats_Stack *S_stack;
double *S_stats_ctr;
STRING_Symbol_Table *tbl;
long unsigned int S_min_addr, S_max_addr;
int S_current_func_no;

long int S_max_file_aliases = MAX_FILE_ALIASES;

extern int num_load_alias;
extern int num_store_alias;

Scache *cache, *pcache, *scache, *distr_cache;
Scache *icache;
S_Profile_Info *prof_info;
L_Histogram *dmr_histo, *drr_histo, *dmc_histo, *srr_histo, *trr_histo;

/* Alloc pools */
L_Alloc_Pool *S_MemDep_Data_Info_pool = NULL;
L_Alloc_Pool *S_MemDep_Access_Info_pool = NULL;
L_Alloc_Pool *S_MemDep_Alias_Info_pool = NULL;
L_Alloc_Pool *S_Pdc_MemDep_Data_Info_pool = NULL;
L_Alloc_Pool *S_Addr_Interval_pool = NULL;
L_Alloc_Pool *S_Addr_Func_Summary_pool = NULL;

S_Activation_Record *activation_record;
int activation_record_depth, max_activation_record_depth;
INT_Symbol_Table *afs_table;

/* 
 * JCG added BTB profiling (BTB interface is not clean now, so needs
 * a pnode passed to it.)
 */
static Pnode prof_pnode;

void
S_read_parm_profile (Parm_Parse_Info * ppi)
{
  L_read_parm_s (ppi, "annot_name", &S_annot_name);
  L_read_parm_b (ppi, "profile_memory_accesses", &S_profile_memory_accesses);
  L_read_parm_b (ppi, "profile_cycle_counts", &S_profile_cycle_counts);
  L_read_parm_b (ppi, "profile_branches", &S_profile_branches);
  L_read_parm_b (ppi, "profile_predicates", &S_profile_predicates);
  L_read_parm_b (ppi, "profile_pred_defs", &S_profile_pred_defs);	/*JCG 6/8/95 */
  L_read_parm_b (ppi, "profile_promoted_predicates", &S_profile_promoted_predicates);	/*JCG 6/10/95 */
  L_read_parm_b (ppi, "profile_issue_weight", &S_profile_issue_weight);	/* JCG 6/11/95 */
  L_read_parm_b (ppi, "mdp_clear_addrs_on_overflow",
		 &S_clear_addrs_on_overflow);
  L_read_parm_b (ppi, "find_access_distribution", &S_distribution);
  L_read_parm_i (ppi, "distr_base_addr", &S_distr_base_addr);
  L_read_parm_i (ppi, "distr_entry_size", &S_distr_entry_size);
  L_read_parm_i (ppi, "distr_entries", &S_distr_entries);
  L_read_parm_i (ppi, "distr_time_delta", &S_distr_time_delta);
  L_read_parm_i (ppi, "distr_time_start", &S_distr_time_start);
  L_read_parm_i (ppi, "distr_time_end", &S_distr_time_end);
  L_read_parm_s (ppi, "distr_file_name", &S_distr_file_name);
  L_read_parm_b (ppi, "profile_icache_misses", &S_profile_icache_misses);	/* JCG 2/18/97 */

  /* Memory dependence profiling */
  L_read_parm_b (ppi, "mem_dep_profiling", &S_mem_dep_profiling);
  L_read_parm_s (ppi, "mem_dep_guide_file_name", &S_mem_dep_guide_file_name);
  L_read_parm_s (ppi, "mem_dep_profile_file_name",
		 &S_mem_dep_profile_file_name);
  L_read_parm_s (ppi, "mem_dep_model_name", &S_mem_dep_model_name);
  L_read_parm_s (ppi, "mem_dep_guide_model_name",
		 &S_mem_dep_guide_model_name);

  if (S_mem_dep_profiling)
    {
      if (L_pmatch (S_mem_dep_model_name, "unguided"))
	{
	  S_mem_dep_model = MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT;
	}
      else if (L_pmatch (S_mem_dep_model_name, "guided"))
	{
	  S_mem_dep_model = MEM_DEP_MODEL_GUIDED_PDC_COLLECT;
	}
      else
	{
	  S_punt
	    ("S_read_parm_profile: mem_dep_model_name must be (unguided,guided)");
	}

      if (L_pmatch (S_mem_dep_guide_model_name, "local"))
	{
	  S_mem_dep_guide_model = MEM_DEP_GUIDE_MODEL_LOCAL;
	}
      else if (L_pmatch (S_mem_dep_guide_model_name, "sync"))
	{
	  S_mem_dep_guide_model = MEM_DEP_GUIDE_MODEL_SYNC;
	}
      else
	{
	  S_punt
	    ("S_read_parm_profile: mem_dep_guide_model_name must be (local,sync)");
	}
    }
}

void
S_print_cache_configuration (FILE * out, char *cache_text, int size,
			     int block_size, int assoc, int write_allocate)
{
  fprintf (out, "# %s CONFIGURATION:\n", cache_text);
  fprintf (out, "\n");
  fprintf (out, "%12u cache size.\n", size);
  fprintf (out, "%12u cache block size.\n", block_size);
  fprintf (out, "%12u cache assoc.\n", assoc);
  if (!strcmp (cache_text, "CACHE"))
    {
      if (write_allocate)
	fprintf (out, "%12s ", "Yes");
      else
	fprintf (out, "%12s ", "No");
      fprintf (out, "write allocate protocol.\n");
    }
}

void
S_print_configuration_profiling (FILE * out)
{
  S_print_configuration_system (out);
  fprintf (out, "Annotation file:       %s\n", S_annot_name);
  fprintf (out, "Histogram file:        %s\n", S_histogram_file_name);

  /* JCG 8/16/94 */
  fprintf (out, "\n");
  if (S_profile_cycle_counts)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile cycle counts.\n");

  if (S_profile_memory_accesses)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile memory accesses.\n");

  if (S_profile_predicates)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile predicates.\n");

  /* JCG 6/8/95 */
  if (S_profile_pred_defs)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile pred defs.\n");

  /* JCG 6/10/95 */
  if (S_profile_promoted_predicates)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile promoted predicates.\n");

  if (S_profile_issue_weight)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile issue weight.\n");

  if (S_profile_branches)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile branches.\n");

  if (S_profile_icache_misses)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "profile icache misses.\n");

  if (S_profile_memory_accesses && S_mem_dep_profiling)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not do");
  fprintf (out, "memory dependence profiling.\n");

  if (S_profile_icache_misses)
    {
      fprintf (out, "\n");
      fprintf (out, "# ICACHE CONFIGURATION:\n");
      fprintf (out, "\n");
      fprintf (out, "%12u icache size.\n", S_icache_size);
      fprintf (out, "%12u icache block size.\n", S_icache_block_size);
      fprintf (out, "%12u icache assoc.\n", S_icache_assoc);
    }

  fprintf (out, "\n");

  if (S_profile_memory_accesses || S_mem_dep_profiling)
    {
      S_print_cache_configuration (out, "CACHE", S_dcache_size,
				   S_dcache_block_size, S_dcache_assoc,
				   S_dcache_write_allocate);
      if (S_prefetch_cache)
	S_print_cache_configuration (out, "PREFETCH CACHE", S_pcache_size,
				     S_pcache_block_size, S_pcache_assoc, 0);
      if (S_secondary_cache)
	S_print_cache_configuration (out, "SECONDARY CACHE", S_scache_size,
				     S_scache_block_size, S_scache_assoc, 0);
    }

  fprintf (out, "\n");

  if (S_profile_branches)
    {
      S_print_configuration_BTB (out);
    }


  fprintf (out, "\n");
}


void
S_init_profiler ()
{
  int i;
  S_Opc_Info *info;
  S_Oper *oper;

  /* Open files */
  if (strcmp (S_annot_name, "stdout") == 0)
    annot_file = stdout;
  else if ((annot_file = fopen (S_annot_name, "w")) == NULL)
    S_punt ("Unable to open annot file '%s'.\n", S_annot_name);

  if (S_profile_memory_accesses && !S_histogram_file)
    {
      if (strcmp (S_histogram_file_name, "stdout") == 0)
	S_histogram_file = stdout;
      else if ((S_histogram_file = fopen (S_histogram_file_name, "w")) ==
	       NULL)
	S_punt ("Unable to open histogram file '%s'.\n",
		S_histogram_file_name);

      if (S_mem_dep_profiling)
	{

	  if (S_mem_dep_model == MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT)
	    {

	      if (!(tmp_name = tmpnam (NULL)))
		S_punt ("Unable to create temp file name\n");
	      if ((tmp_file = fopen (tmp_name, "w")) == 0)
		S_punt ("Unable to open temp file\n");
	    }
	  else if (S_mem_dep_model == MEM_DEP_MODEL_GUIDED_PDC_COLLECT)
	    {
	      /* Read from file */
	      if (
		  (mem_dep_guide_file =
		   fopen (S_mem_dep_guide_file_name, "r")) == NULL)
		S_punt ("Unable to open annot file '%s'.\n",
			S_mem_dep_guide_file_name);

	    }
	}

      if (S_distribution)
	{
	  if (strcmp (S_distr_file_name, "stdout") == 0)
	    distr_file = stdout;
	  else if ((distr_file = fopen (S_distr_file_name, "w")) == NULL)
	    S_punt ("Unable to open distribution file '%s'.\n",
		    S_distr_file_name);
	}
    }

  srr_window = PCACHE_MULT * S_pcache_size / S_pcache_block_size;

  /* Make sure we are profiling something */
  if (!S_profile_cycle_counts && !S_profile_memory_accesses &&
      !S_profile_branches &&
      !S_profile_icache_misses &&
      !S_profile_predicates &&
      !S_profile_pred_defs &&
      !S_profile_promoted_predicates && !S_profile_issue_weight)
    S_punt ("Profiler: All profiling options turned off.  Exiting early.");

  /* For optimizing the trace reading, need to know if 
   * we are only profiling cycle counts.
   */
  if (S_profile_cycle_counts && !(S_profile_memory_accesses ||
				  S_profile_branches ||
				  S_profile_icache_misses ||
				  S_profile_predicates ||
				  S_profile_pred_defs ||
				  S_profile_promoted_predicates ||
				  S_profile_issue_weight))
    S_profile_cycle_counts_only = 1;

  if (S_profile_cycle_counts)
    {
      if ((S_stats_ctr = (double *)
	   malloc ((S_max_pc + 1) * sizeof (double))) == NULL)
	S_punt ("S_init_profiler: Out of memory");

      /* init hash table for hashing on loop types */
      tbl = STRING_new_symbol_table ("Loop Cycle Counts", HASH_SIZE);
    }

  /* Create data structures */
  if (S_profile_memory_accesses)
    {
      cache = S_create_cache (S_dcache_size, S_dcache_block_size,
			      S_dcache_assoc, S_create_Profile_data);
      if (S_prefetch_cache)
	pcache = S_create_cache (S_pcache_size, S_pcache_block_size,
				 S_pcache_assoc, S_create_Profile_data);
      if (S_secondary_cache)
	scache = S_create_cache (S_scache_size, S_scache_block_size,
				 S_scache_assoc, S_create_Profile_data);

      if (S_mem_dep_profiling)
	{

	  if (S_mem_dep_model == MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT)
	    {

	      S_MemDep_Data_Info_pool =
		L_create_alloc_pool ("S_MemDep_Data_Info",
				     sizeof (S_MemDep_Data_Info), 1);
	      S_MemDep_Access_Info_pool =
		L_create_alloc_pool ("S_MemDep_Access_Info",
				     sizeof (S_MemDep_Access_Info), 1);
	      S_MemDep_Alias_Info_pool =
		L_create_alloc_pool ("S_MemDep_Alias_Info",
				     sizeof (S_MemDep_Alias_Info), 1);
	      aliases_in_file = 0;
	      S_current_func_no = 0;
	    }
	  else if (S_mem_dep_model == MEM_DEP_MODEL_GUIDED_PDC_COLLECT)
	    {
	      S_Pdc_MemDep_Data_Info_pool =
		L_create_alloc_pool ("S_Pdc_MemDep_Data_Info",
				     sizeof (S_Pdc_MemDep_Data_Info), 1);
	      S_Addr_Interval_pool =
		L_create_alloc_pool ("S_Addr_Interval",
				     sizeof (S_Addr_Interval), 1);
	      S_Addr_Func_Summary_pool =
		L_create_alloc_pool ("S_Addr_Func_Summary",
				     sizeof (S_Addr_Func_Summary), 1);
	      max_activation_record_depth = 20;
	      activation_record = malloc (sizeof (S_Activation_Record) *
					  max_activation_record_depth);
	      afs_table = INT_new_symbol_table ("afs_table", 20);
	    }


	}

      S_min_addr = S_max_addr = 0;
      if (S_distribution)
	{
	  S_init_distr ();
	}
    }

  if (S_profile_branches)
    {
      prof_pnode.btb = S_create_BTB (&prof_pnode);
    }

  if (S_profile_predicates)
    {
      pred_attr = "PS";
    }

  /* JCG 6/8/95 */
  if (S_profile_pred_defs)
    {
      pred_dest0_set_attr = "PD0";
      pred_dest1_set_attr = "PD1";
      if (!S_trace_pred_defs)
	S_punt ("Error: Must set trace_pred_defs if profiling pred defs!");
    }

  if (S_profile_promoted_predicates)
    {
      promoted_pred_attr = "PPS";
      if (!S_trace_promoted_predicates)
	S_punt
	  ("Error: Must set trace_promoted_predicates if profiling promoted predicates!");
    }

  /* JCG 2/17/97 */
  if (S_profile_icache_misses)
    {
      icache = S_create_cache (S_icache_size, S_icache_block_size,
			       S_icache_assoc, NULL);
    }

  if ((prof_info = (S_Profile_Info *)
       malloc ((S_max_pc + 1) * sizeof (S_Profile_Info))) == NULL)
    S_punt ("S_init_profiler: Out of memory");

  /* Init info */
  S_num_profiled = S_num_loads = S_num_stores = S_num_prefetches = 0;
  S_num_loads_secondary = S_num_misses_secondary = 0;
  S_num_misses = S_num_misses_pref = S_num_misses_stores = 0;
  S_num_icache_misses = 0;
  S_num_prof_samples = 0;
  S_num_branches = S_num_predicates = 0;
  S_num_branch_mispredictions = 0;
  S_num_pred_defs = 0;		/* JCG 5/8/95 */
  S_num_promoted = 0;		/* JCG 5/11/95 */
  S_stats_region_ctr = 0;
  S_region_cycles_ctr = 0;
  S_region_billions_cycles_ctr = 0;

  for (i = 0; i <= S_max_pc; i++)
    {
      prof_info[i].num_executed = prof_info[i].num_misses = 0;
      prof_info[i].num_reuses = 0;
      prof_info[i].num_temporal_reuses = prof_info[i].num_spatial_reuses = 0;
      prof_info[i].num_mispredicted = 0;
      prof_info[i].branch_exec_state = BRANCH_EXEC_NONE;
      prof_info[i].num_icache_misses = 0;
      prof_info[i].num_pred_squashed = 0;
      prof_info[i].num_pred_dest0_set = 0;
      prof_info[i].num_pred_dest1_set = 0;
      prof_info[i].num_promoted_pred_squashed = 0;
      prof_info[i].num_aliases = 0;
      prof_info[i].ptr = NULL;
      oper = oper_tab[i];
      if (!oper)
	continue;
      info = &opc_info_tab[oper->opc];

      /* Set flags to zero, set flags based on what is being profiled */
      prof_info[i].flags = 0;

      if (S_profile_memory_accesses)
	{
	  /* Added PROF_INSTR for later speed enhancements -JCG 5/11/95 */
	  if (info->opc_type == LOAD_OPC)
	    prof_info[i].flags |= LOAD | PROF_INSTR;
	  else if (info->opc_type == STORE_OPC)
	    prof_info[i].flags |= STORE | PROF_INSTR;
	  if (info->opc_type == JSR_OPC)
	    prof_info[i].flags |= JSR | PROF_INSTR;
	  else if (info->opc_type == PREFETCH_OPC)
	    prof_info[i].flags |= PREFETCH | PROF_INSTR;
	}

      /* JCG */
      if (S_profile_branches)
	{
	  if (info->is_branch)
	    prof_info[i].flags |= PROF_BRANCH | PROF_INSTR;
	}

      if (S_profile_predicates)
	{
	  if (oper_tab[i]->flags & PREDICATED)
	    prof_info[i].flags |= PROF_PREDICATE | PROF_INSTR;
	}
      /* JCG 6/8/95 */
      if (S_profile_pred_defs)
	{
	  if (oper_tab[i]->flags & PRED_DEF)
	    prof_info[i].flags |= PROF_PRED_DEF | PROF_INSTR;
	}
      if (S_profile_promoted_predicates)
	{
	  if (oper_tab[i]->flags & PROMOTED)
	    prof_info[i].flags |= PROF_PROMOTED | PROF_INSTR;
	}
      /* JCG 2/17/97 */
      if (S_profile_icache_misses)
	{
	  prof_info[i].flags |= PROF_ICACHE | PROF_INSTR;
	}

      if (S_profile_cycle_counts)
	{
	  S_stats_ctr[i] = 0.0;
	}
    }

  /* Allocate guide tables for loads if mem_dep_profiling */
  if ((S_mem_dep_profiling)
      && (S_mem_dep_model == MEM_DEP_MODEL_GUIDED_PDC_COLLECT))
    {
      S_read_guide_file ();
    }
}


/* Create a conflict based on the access size and the address */
int
S_mem_data_conflict (S_Oper * store_oper, unsigned int store_addr,
		     S_Oper * load_oper, unsigned int load_addr)
{
  int store_size, load_size;
  int i, j;

  if (store_addr == load_addr)
    return 1;

  store_size = opc_info_tab[store_oper->opc].access_size;
  load_size = opc_info_tab[load_oper->opc].access_size;

  for (i = 0; i < store_size; i++)
    {
      for (j = 0; j < load_size; j++)
	{

	  if ((store_addr + i) == (load_addr + j))
	    return (1);
	}
    }
  return (0);
}

int allocated_interval;

S_Addr_Interval *
S_new_addr_interval ()
{
  S_Addr_Interval *new;

  allocated_interval++;
  new = (S_Addr_Interval *) L_alloc (S_Addr_Interval_pool);
  new->start = 0;
  new->end = 0;
  new->next = NULL;
  return new;
}

void
S_remove_addr_interval (S_Addr_Interval * ptr)
{
  S_Addr_Interval *this, *next;

  for (this = ptr; this; this = next)
    {
      next = this->next;
      this->next = 0;
      L_free (S_Addr_Interval_pool, this);
      allocated_interval--;
      if (allocated_interval < 0)
	S_punt ("S_remove_addr_interval: S_Addr_Interval underflow\n");
    }
#if 0
  S_remove_addr_interval (ptr->next);
  L_free (S_Addr_Interval_pool, ptr);
#endif
}

void
S_verify_addr_func_summary (S_Addr_Func_Summary * ptr)
{
  int i;
  S_Addr_Interval *this;

  for (i = 0, this = ptr->store_head;
       this && i < ptr->store_interval_length; i++, this = this->next)
    {
      if (this->start > this->end)
	S_punt ("S_verify_addr_func_summary failed (1)\n");
      if (this->next)
	{
	  if (this->next->start - this->end < 2)
	    S_punt ("S_verify_addr_func_summary failed (2)\n");
	}
    }
  if (i < ptr->store_interval_length)
    S_punt ("S_verify_addr_func_summary failed (3)\n");

  if (this)
    S_punt ("S_verify_addr_func_summary failed (4)\n");

  for (i = 0, this = ptr->load_head;
       this && i < ptr->load_interval_length; i++, this = this->next)
    {
      if (this->start > this->end)
	S_punt ("S_verify_addr_func_summary failed (1)\n");
      if (this->next)
	{
	  if (this->next->start - this->end < 2)
	    S_punt ("S_verify_addr_func_summary failed (2)\n");
	}
    }
  if (i < ptr->load_interval_length)
    S_punt ("S_verify_addr_func_summary failed (3)\n");

  if (this)
    S_punt ("S_verify_addr_func_summary failed (4)\n");
}

void
S_dump_activation_record ()
{
  int i;
  S_Addr_Func_Summary *afs;

  fprintf (stderr, "\n----------\n");
  for (i = activation_record_depth - 1; i >= 0; i--)
    {
      afs = activation_record[i].afs;
      fprintf (stderr, "%s(length=(%d,%d)\n",
	       afs->fn ? afs->fn->name : afs->lib_name,
	       afs->store_interval_length, afs->load_interval_length);
      S_verify_addr_func_summary (afs);
    }
  fprintf (stderr, "main\n");
  fprintf (stderr, "----------\n");
}

void
S_remove_addr_func_summary (void *ptr)
{
  S_Addr_Func_Summary *afs = ptr;

  S_verify_addr_func_summary (afs);
  S_remove_addr_interval (afs->store_head);
  S_remove_addr_interval (afs->load_head);
  L_free (S_Addr_Func_Summary_pool, afs);
}

int
S_addr_overlap (unsigned int s1, unsigned int e1, unsigned s2, unsigned e2)
{
  if (s1 >= s2 && e1 <= e2)
    return INTERVAL_MATCH;
  if (e1 < s2 - 1)
    return INTERVAL_LESS;
  if (e2 < s1 - 1)
    return INTERVAL_GREATER;
  return INTERVAL_OVERLAP;
}

S_Addr_Interval *
S_get_previous_interval (S_Addr_Interval * head, S_Addr_Interval * cur)
{
  if (head == cur)
    return 0;
  for (; head; head = head->next)
    {
      if (head->next == cur)
	return head;
    }
  S_punt ("S_get_previous_interval: cur not in head list\n");
  return NULL;
}

void
S_merge_with_previous (S_Addr_Func_Summary * afs, S_Addr_Interval * head,
		       S_Addr_Interval * cur, int is_store)
{
  S_Addr_Interval *prev;
  int olap;

  prev = S_get_previous_interval (head, cur);
  if (prev == 0)
    return;
  olap = S_addr_overlap (prev->start, prev->end, cur->start, cur->end);
  /* JWS 20000117 Need to eliminate redundant intervals as well. */
  if (olap == INTERVAL_OVERLAP || olap == INTERVAL_MATCH)
    {
      prev->start = S_min (prev->start, cur->start);
      prev->end = S_max (prev->end, cur->end);
      prev->next = cur->next;
      if (prev->next == 0)
	{
	  if (is_store)
	    afs->store_tail = prev;
	  else
	    afs->load_tail = prev;
	}
      cur->next = 0;
      S_remove_addr_interval (cur);
      if (is_store)
	{
	  afs->store_interval_length--;
	  S_merge_with_previous (afs, afs->store_head, prev, is_store);
	}
      else
	{
	  afs->load_interval_length--;
	  S_merge_with_previous (afs, afs->load_head, prev, is_store);
	}
    }
}

void
S_merge_with_next (S_Addr_Func_Summary * afs, S_Addr_Interval * cur,
		   int is_store)
{
  S_Addr_Interval *next;
  int olap;

  next = cur->next;
  if (next == 0)
    return;
  olap = S_addr_overlap (cur->start, cur->end, next->start, next->end);
  /* JWS 20000117 Need to eliminate redundant intervals as well. */
  if (olap == INTERVAL_OVERLAP || olap == INTERVAL_MATCH)
    {
      cur->start = S_min (cur->start, next->start);
      cur->end = S_max (cur->end, next->end);
      cur->next = next->next;
      if (cur->next == 0)
	{
	  if (is_store)
	    afs->store_tail = cur;
	  else
	    afs->load_tail = cur;
	}
      next->next = 0;
      S_remove_addr_interval (next);
      if (is_store)
	afs->store_interval_length--;
      else
	afs->load_interval_length--;
      S_merge_with_next (afs, cur, is_store);
    }
}

int
S_insert_addr_interval (S_Addr_Func_Summary * afs, unsigned start,
			unsigned size, int is_store)
{
  S_Addr_Interval *this, *prev, *new;
  unsigned int end;
  int i;

  end = start + size - 1;

  /* initially empty list */
  if (is_store)
    {
      if (afs->store_head == 0)
	{
	  this = S_new_addr_interval ();
	  afs->store_head = afs->store_tail = this;
	  this->start = start;
	  this->end = end;
	  afs->store_interval_length = 1;
	  return 1;
	}
    }
  else
    {
      if (afs->load_head == 0)
	{
	  this = S_new_addr_interval ();
	  afs->load_head = afs->load_tail = this;
	  this->start = start;
	  this->end = end;
	  afs->load_interval_length = 1;
	  return 1;
	}
    }

  switch (S_addr_overlap (start, end,
			  is_store ? afs->store_tail->start :
			  afs->load_tail->start,
			  is_store ? afs->store_tail->end :
			  afs->load_tail->end))
    {
    case INTERVAL_MATCH:
      return 0;
    case INTERVAL_OVERLAP:
      if (is_store)
	{
	  afs->store_tail->start = S_min (afs->store_tail->start, start);
	  afs->store_tail->end = S_max (afs->store_tail->end, end);
	  S_merge_with_previous (afs, afs->store_head, afs->store_tail,
				 is_store);
	}
      else
	{
	  afs->load_tail->start = S_min (afs->load_tail->start, start);
	  afs->load_tail->end = S_max (afs->load_tail->end, end);
	  S_merge_with_previous (afs, afs->load_head, afs->load_tail,
				 is_store);
	}
      return 1;
    case INTERVAL_GREATER:
      new = S_new_addr_interval ();
      new->start = start;
      new->end = end;
      if (is_store)
	{
	  afs->store_tail->next = new;
	  afs->store_tail = new;
	  afs->store_interval_length++;
	}
      else
	{
	  afs->load_tail->next = new;
	  afs->load_tail = new;
	  afs->load_interval_length++;
	}
      return 1;
    }

  for (i = 0, prev = 0, this = (is_store ? afs->store_head : afs->load_head);
       (this != 0)
       && (i <
	   (is_store ? afs->store_interval_length : afs->
	    load_interval_length)); prev = this, this = this->next, i++)
    {
      switch (S_addr_overlap (start, end, this->start, this->end))
	{
	case INTERVAL_LESS:
	  new = S_new_addr_interval ();
	  new->start = start;
	  new->end = end;
	  new->next = this;
	  if (is_store)
	    afs->store_interval_length++;
	  else
	    afs->load_interval_length++;
	  if (prev == 0)
	    {
	      if (is_store)
		afs->store_head = new;
	      else
		afs->load_head = new;
	    }
	  else
	    {
	      prev->next = new;
	    }
	  return 1;
	case INTERVAL_OVERLAP:
	  this->start = S_min (this->start, start);
	  this->end = S_max (this->end, end);
	  S_merge_with_next (afs, this, is_store);
	  return 1;
	case INTERVAL_MATCH:
	  return 0;
	}
    }
  S_punt ("S_insert_addr_interval: entry is not inserted\n");
  return 0;
}

S_Addr_Func_Summary *
S_new_addr_func_summary ()
{
  S_Addr_Func_Summary *new;

  new = (S_Addr_Func_Summary *) L_alloc (S_Addr_Func_Summary_pool);
  new->fn = 0;
  new->lib_name = 0;
  new->active = 1;
  new->unsafe = 0;
  new->store_interval_length = 0;
  new->load_interval_length = 0;
  new->store_head = new->store_tail = 0;
  new->load_head = new->load_tail = 0;
  return new;
}

void
S_update_addr_func_summary (Sint * sint, int pc, int is_store)
{
  int i, changed;
  unsigned int addr, start, end, size;

  addr = sint->trace.mem_addr;
  size = opc_info_tab[sint->oper->opc].access_size;
  start = addr >> JSR_Macroblock;
  end = (addr + size - 1) >> JSR_Macroblock;
  changed = 0;
  for (i = activation_record_depth - 1; i >= 0; i--)
    {
      if (activation_record[i].afs->active == 0)
	S_punt ("Obslete afs in stack\n");
      changed = S_insert_addr_interval (activation_record[i].afs, start,
					end - start + 1, is_store);
      /* optimization */
      if (changed == 0)
	break;
    }
}

void
S_update_jsr_side_effects ()
{
  int i;

  for (i = activation_record_depth - 1; i >= 0; i--)
    {
      if (activation_record[i].afs->unsafe == 1)
	break;
      activation_record[i].afs->unsafe = 1;
    }
}

int
S_mem_data_conflict_jsr (unsigned int addr, S_Oper * s_oper, int key,
			 int is_store)
{
  INT_Symbol *symbol;
  S_Addr_Func_Summary *afs;
  S_Addr_Interval *addr_interval;
  unsigned int size, start, end;

  symbol = INT_find_symbol (afs_table, key);
  if (symbol == 0)
    return 1;
  afs = symbol->data;
  if (afs->unsafe)
    return 1;

  start = addr >> JSR_Macroblock;
  size = opc_info_tab[s_oper->opc].access_size;
  end = (addr + size - 1) >> JSR_Macroblock;

  for (addr_interval = (is_store ? afs->store_head : afs->load_head);
       addr_interval; addr_interval = addr_interval->next)
    {
      switch (S_addr_overlap (start, end,
			      addr_interval->start, addr_interval->end))
	{
	case INTERVAL_MATCH:
	case INTERVAL_OVERLAP:
	  return 1;
	}
    }
  return 0;
}

void
S_mem_dep_guided_collection_profiling (Sint * sint, int pc)
{
  int flags, store, load;
  int conflict = 0, conflict_flags;
  int i, j;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int load_pc, offset;
#endif
  int store_pc = pc, conflict_pc;
  int func_no;
  S_Pdc_MemDep_Data_Info *data_info;
  S_Guide_Info *guide_info, *load_guide_info;
  S_Oper *store_oper, *load_oper, *jsr_oper, *conflict_oper;

  flags = prof_info[pc].flags;
  store = flags & STORE;
  load = flags & LOAD;

  if (S_mem_dep_guide_model == MEM_DEP_GUIDE_MODEL_LOCAL)
    {

      if ((store || load) && activation_record_depth)
	{
	  S_update_addr_func_summary (sint, pc, store ? 1 : 0);
	}

      /* Check the guide info */
      guide_info = (S_Guide_Info *) prof_info[pc].ptr;
      if (!guide_info)
	return;

      /* Check the opcode */
      if (store)
	{
	  store_oper = sint->oper;

	  /* Search the guide list for the store id */
	  for (i = 0; i < guide_info->index; i++)
	    {

	      /* Get the oper */
	      conflict_pc = guide_info->guide_pc[i];
	      conflict_oper = oper_tab[conflict_pc];
#if 0
	      conflict =
		S_mem_data_conflict (store_oper, sint->trace.mem_addr,
				     conflict_oper, conflict_oper->last_addr);
#endif
	      if (opc_info_tab[conflict_oper->opc].opc_type == JSR_OPC)
		{
		  conflict =
		    S_mem_data_conflict_jsr (sint->trace.mem_addr, sint->oper,
					     (conflict_pc << 16) |
					     activation_record_depth, 1);
		  if (conflict == 0)
		    conflict =
		      S_mem_data_conflict_jsr (sint->trace.mem_addr,
					       sint->oper,
					       (conflict_pc << 16) |
					       activation_record_depth, 0);
		}
	      else
		{
		  conflict =
		    S_mem_data_conflict (store_oper, sint->trace.mem_addr,
					 conflict_oper,
					 conflict_oper->last_addr);
		}

	      if (!conflict)
		continue;

	      conflict_flags = prof_info[conflict_pc].flags;

	      /* Found a conflict within the guide list */
	      /* Increment the conflict rate            */
	      guide_info->conflict[i] = guide_info->conflict[i] + 1;

#if 0
	      if (conflict_flags & LOAD)
		{

		  /* Annotate the respective load with a conflict */
		  load_guide_info =
		    (S_Guide_Info *) prof_info[conflict_pc].ptr;
		  for (j = 0; j < load_guide_info->index; j++)
		    {
		      if (load_guide_info->guide_pc[j] == store_pc)
			{
			  /* Found a conflict within the guide list */
			  /* Increment the conflict rate            */
			  load_guide_info->conflict[j] =
			    load_guide_info->conflict[j] + 1;
			  break;
			}
		    }
		}
#endif
	    }
	}
      else if (load)
	{
	  load_oper = sint->oper;
	  /* Search the guide list for the store id */
	  for (i = 0; i < guide_info->index; i++)
	    {

	      /* Get the oper */
	      conflict_pc = guide_info->guide_pc[i];
	      conflict_oper = oper_tab[conflict_pc];
	      if (opc_info_tab[conflict_oper->opc].opc_type == JSR_OPC)
		{
		  conflict =
		    S_mem_data_conflict_jsr (sint->trace.mem_addr, sint->oper,
					     (conflict_pc << 16) |
					     activation_record_depth, 1);
		}
	      else
		{
		  conflict =
		    S_mem_data_conflict (load_oper, sint->trace.mem_addr,
					 conflict_oper,
					 conflict_oper->last_addr);
		}

	      if (!conflict)
		continue;

	      /* Found a conflict within the guide list */
	      /* Increment the conflict rate            */
	      guide_info->conflict[i] = guide_info->conflict[i] + 1;
	    }
	}
      /* jsr */
      else
	{
	  jsr_oper = sint->oper;
	  /* Search the guide list for the store id */
	  for (i = 0; i < guide_info->index; i++)
	    {

	      /* Get the oper */
	      conflict_pc = guide_info->guide_pc[i];
	      conflict_oper = oper_tab[conflict_pc];
	      conflict = S_mem_data_conflict_jsr (conflict_oper->last_addr,
						  conflict_oper,
						  (pc << 16) |
						  activation_record_depth, 1);
	      if (conflict == 0
		  && opc_info_tab[conflict_oper->opc].opc_type == STORE_OPC)
		conflict =
		  S_mem_data_conflict_jsr (conflict_oper->last_addr,
					   conflict_oper,
					   (pc << 16) |
					   activation_record_depth, 0);
	      if (!conflict)
		continue;

	      /* Found a conflict within the guide list */
	      /* Increment the conflict rate            */
	      guide_info->conflict[i] = guide_info->conflict[i] + 1;
	    }
	}
    }
  else
    {
      data_info =
	S_get_pdc_data_info_for_address ((unsigned) sint->trace.mem_addr);

      /* Check the opcode */
      if (store)
	{

	  data_info->store_pc = pc;
	  data_info->func_no = S_current_func_no;
	  store_oper = sint->oper;
	  guide_info = (S_Guide_Info *) prof_info[pc].ptr;

	  if (!guide_info)
	    return;

	  /* Search the guide list for the store id */
	  for (i = 0; i < guide_info->index; i++)
	    {

	      /* Get the oper */
	      conflict_pc = guide_info->guide_pc[i];
	      conflict_oper = oper_tab[conflict_pc];
	      conflict =
		S_mem_data_conflict (store_oper, sint->trace.mem_addr,
				     conflict_oper, conflict_oper->last_addr);

	      if (!conflict)
		continue;

	      conflict_flags = prof_info[conflict_pc].flags;

	      if (conflict_flags & STORE)
		{
		  /* Found a conflict within the guide list */
		  /* Increment the conflict rate            */
		  guide_info->conflict[i] = guide_info->conflict[i] + 1;
		}
	      else
		{
		  /* Found a conflict within the guide list */
		  /* Increment the conflict rate            */
		  guide_info->conflict[i] = guide_info->conflict[i] + 1;

		  load_guide_info =
		    (S_Guide_Info *) prof_info[conflict_pc].ptr;
		  for (j = 0; j < load_guide_info->index; j++)
		    {
		      if (load_guide_info->guide_pc[j] == store_pc)
			{
			  /* Found a conflict within the guide list */
			  /* Increment the conflict rate            */
			  load_guide_info->conflict[j] =
			    load_guide_info->conflict[j] + 1;
			  break;
			}
		    }
		}
	    }
	}
      else if (load)
	{
	  store_pc = data_info->store_pc;
	  func_no = data_info->func_no;

	  guide_info = (S_Guide_Info *) prof_info[pc].ptr;

	  if (!guide_info)
	    return;

	  /* Search the guide list for the store id */
	  for (i = 0; i < guide_info->index; i++)
	    {

	      if (guide_info->guide_pc[i] == store_pc)
		{
		  /* Found a conflict within the guide list */
		  /* Increment the conflict rate            */
		  conflict = 1;
		  guide_info->conflict[i] = guide_info->conflict[i] + 1;
		  break;
		}
	    }

	  /* Check to see if the data was altered within another function */
	  if (!conflict)
	    {
	      if (func_no != S_current_func_no)
		{
		  /* increment jsr call conflict */
		  guide_info->call_conflict++;
		}
	    }
	}
    }
}

void
S_mem_dep_unguided_collection_profiling (Sint * sint, int pc)
{
  int offset;
  int i;
  S_hash_node *data_entry;
  S_MemDep_Data_Info *data_info;

  int flags, store;

  flags = prof_info[pc].flags;
  store = flags & STORE;

  for (offset = 0; offset < opc_info_tab[sint->oper->opc].access_size;
       offset += 4)
    {
      /* Get the hash entry for this memory address to get
       * the last load and stores to this address.
       */
      data_entry = S_hash_find_data_info (
					  (unsigned) sint->trace.mem_addr +
					  (unsigned) offset);

      /* No data entry, allocate one in hash table. */
      if (!data_entry)
	{
	  data_entry =
	    S_hash_insert_data_info ((unsigned) sint->trace.mem_addr +
				     (unsigned) offset);
	  data_info =
	    (S_MemDep_Data_Info *) L_alloc (S_MemDep_Data_Info_pool);
	  data_info->last_store_pc = 0;
	  for (i = 0; i < LOAD_HIST_SIZE; i++)
	    data_info->last_load_pc[i] = 0;
	  data_info->num_loads = 0;
	  data_info->func_no = S_current_func_no;
	  data_entry->ptr = (S_MemDep_Data_Info *) data_info;
	}

      /* Was an entry in hash table, get its info */
      else
	{
	  data_info = (S_MemDep_Data_Info *) data_entry->ptr;

	  /* If entry in hash table is for a different
	   * dynamic function invocation then reinit info.
	   */
	  if (data_info->func_no != S_current_func_no)
	    {
	      data_info->last_store_pc = 0;
	      for (i = 0; i < LOAD_HIST_SIZE; i++)
		data_info->last_load_pc[i] = 0;
	      data_info->num_loads = 0;
	      data_info->func_no = S_current_func_no;
	    }
	}

      /* If this was a store, set the last store pc to
       * this address and also update the alias info for this pc.
       */
      if (store)
	{
	  /* If there was a previous load or store to this address,
	   * update alias info.
	   */
	  if (data_info->last_store_pc || data_info->num_loads)
	    {
	      S_create_store_alias (pc, data_info);
	    }

	  /* Set the last store pc to this address. */
	  data_info->last_store_pc = pc;

	  /* reset the last load pc */
	  data_info->num_loads = 0;
	}

      /* Else this is a load or prefetch, so update the alias info
       * for this pc.
       */
      else
	{
	  /* If there was a previous store to this address,
	   * update alias info.
	   */
	  if (data_info->last_store_pc)
	    {
	      S_create_load_alias (data_info->last_store_pc, pc);
	    }

	  /* Set the last load pc to this address. */
	  if (data_info->num_loads < LOAD_HIST_SIZE)
	    data_info->last_load_pc[data_info->num_loads++] = pc;
	  else
	    {
	      /* Create load-load dependences between first load in array
	       * and second load in array, and remove first from array.
	       */
	      S_create_load_alias (data_info->last_load_pc[0],
				   data_info->last_load_pc[1]);
	      for (i = 0; i < LOAD_HIST_SIZE - 1; i++)
		{
		  data_info->last_load_pc[i] = data_info->last_load_pc[i + 1];
		}
	      data_info->last_load_pc[LOAD_HIST_SIZE - 1] = pc;
	    }

	  /* DON'T reset the last store pc to this address, so
	   * that we get all aliases between loads and prev store.
	   */
	}
    }

}

void
S_profile_simulation_instruction (int pc, Sint * sint, int on_path)
{
  S_Profile_Info *pinfo;
  int flags, sint_flags;
  int branch_state;

  /* Currently only track stats for on-path instructions */
  if (!on_path)
    return;

  /* Get profile operation flags */
  flags = prof_info[pc].flags;
  pinfo = &prof_info[pc];
  sint_flags = sint->flags;

  /* Detect if this instruction needs profiling */
  if (flags & PROF_INSTR)
    {

      /* If this is a memory operation, do appropriate cache operations */
      /* Do branch profiling if directed */
      if (S_profile_branches)
	{
	  if (flags & PROF_BRANCH)
	    {
	      /* Detect misprediction */
	      if (sint_flags & MISPREDICTED)
		{
		  pinfo->num_mispredicted++;
		  S_num_branch_mispredictions++;
		}

	      /* Examine the branch execution state */
	      branch_state = pinfo->branch_exec_state;
	      if (branch_state != BRANCH_EXEC_BOTH)
		{

		  /* Taken path */
		  if (sint_flags & BRANCHED)
		    {
		      if (branch_state == BRANCH_EXEC_NONE)
			pinfo->branch_exec_state = BRANCH_EXEC_TAKEN;
		      else if (branch_state == BRANCH_EXEC_FALLTHRU)
			pinfo->branch_exec_state = BRANCH_EXEC_BOTH;
		    }
		  else
		    {
		      if (branch_state == BRANCH_EXEC_NONE)
			pinfo->branch_exec_state = BRANCH_EXEC_FALLTHRU;
		      else if (branch_state == BRANCH_EXEC_TAKEN)
			pinfo->branch_exec_state = BRANCH_EXEC_BOTH;
		    }
		}

	      /* Trace the transistions */

	      /* Count the number of profile branches */
	      S_num_branches++;
	    }
	}

      if (S_profile_predicates)
	{
	  /* Detect if profiling predicates */
	  if (flags & PROF_PREDICATE)
	    {
	      if (sint_flags & PRED_SQUASHED)
		pinfo->num_pred_squashed++;
	      S_num_predicates++;
	    }
	}

      if (S_profile_pred_defs)
	{
	  if (flags & PROF_PRED_DEF)
	    {
	      if (sint_flags & PRED_DEST0_SET)
		pinfo->num_pred_dest0_set++;

	      if (sint_flags & PRED_DEST1_SET)
		pinfo->num_pred_dest1_set++;
	      S_num_pred_defs++;
	    }
	}

      if (S_profile_promoted_predicates)
	{
	  /* Profile predicate before promotion.  Invalid data is
	   * flagged when current predicate indicates squash, but
	   * predicate before promotion doesn't.  (Promoted data
	   * is invalid after scheduling and after some loop optimizations).
	   */
	  if (flags & PROF_PROMOTED)
	    {
	      if (sint_flags & PROMOTED_PRED_SQUASHED)
		pinfo->num_promoted_pred_squashed++;

	      /* If actual predicate says squash, but the before
	       * promotion one doesn't, mark data as invalid
	       */
	      else if (sint_flags & PRED_SQUASHED)
		pinfo->flags |= PROMOTED_DATA_INVALID;
	      S_num_promoted++;
	    }
	}
    }
  prof_info[pc].num_executed++;
}

int
S_profile (int pc, int prof_count)
{
  int trace_words_read, **trace_ptr_ptr = NULL, *size_left_ptr = NULL,
    size_left = 0, *trace_ptr = NULL;
  int num_profiled, flags, store, load, prefetch, playdoh_flags, sint_flags;
  int none, v1, c1, c2, c3;
  int predicted_target;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int last_cb = 0, offset;
  S_hash_node *data_entry, *load_entry, *store_entry;
  S_MemDep_Data_Info *data_info;
  S_MemDep_Access_Info *load_info, *store_info;
  S_MemDep_Alias_Info *alias_info;
  S_Oper *oper, *oper2;
  int value;
#endif
  int first_byte;
  int i, num_bytes, last, block_size, write_allocate;
  int is_temporal, access_mask_ls, access_mask_ms;
  int last_ref_pc, access_ctr, int_size;
  S_Profile_Info *pinfo, *info;
  Sint sint, jsr_sint;
  int jsr_pc;
  Scblock *block;
  Profile_data *data, *next;
  Distr_data *distr_data;
  int outside_range;
  Scache *this_cache;
  S_Stats_Stack *temp_stack;
  S_Hash_Entry *hash_entry;
  char loop_type[4];
  double sim_cycle = 0.0;
  S_Addr_Func_Summary *afs;
  INT_Symbol *symbol;

  if (S_mem_dep_profiling)
    {
      S_init_hash ();
    }

  int_size = sizeof (int);
  outside_range = 0;

  /* Get trace manager fields for ease of use */
  if (S_profile_cycle_counts_only)
    {
      trace_ptr_ptr = &S_pnode->trace_manager->trace_ptr;
      size_left_ptr = &S_pnode->trace_manager->size_left;

      /* Preload trace_ptr and size_left and save and load around
       *"long" part of code below.
       */
      size_left = *size_left_ptr;
      trace_ptr = *trace_ptr_ptr;
    }

  /* Need to set stats structure for sint */
  sint.stats = S_program_stats;

  S_num_prof_samples++;
  for (num_profiled = 0; num_profiled < prof_count && !S_end_of_program;
       num_profiled++)
    {

      sim_cycle = (double) ((double) S_num_sim_on_path +
			    S_num_profiled + (double) num_profiled +
			    (double) S_num_skip_on_path +
			    ((double) BILLION * S_billions_skipped));

      if (S_distribution && sim_cycle >= (double) S_distr_time_start &&
	  sim_cycle < (double) S_distr_time_end)
	{
	  if (num_profiled
	      && !((int) (sim_cycle - (double) S_distr_time_start) %
		   S_distr_time_delta))
	    {
	      S_print_distr (outside_range, sim_cycle);
	    }
	}

      sint.oper = oper_tab[pc];
      sint.fn = sint.oper->cb->fn;
      sint.flags = 0;

      if (!S_profile_cycle_counts_only)
	{
	  S_read_trace_info (S_pnode, &sint);
	}
      else
	{
	  /* Get number of trace words this instruction reads.
	   * This will be negative for branches and some predicate
	   * instructions that read a variable number of trace words.
	   */
	  trace_words_read = sint.oper->trace_words_read;

	  /* No trace words read, do nothing. The 70% case. */
	  if (trace_words_read != 0)
	    {
	      /* 1 or more trace words read. The 18% case.
	       * Have inlined part of S_get_trace word to allow
	       * quick skipping of trace information.
	       *
	       * Use unsigned compare to test both for enough words
	       * in buffer to skip and that trace_words_read >= 0.
	       */
	      if (((unsigned) size_left) >= ((unsigned) trace_words_read))
		{
		  size_left -= trace_words_read;
		  trace_ptr += trace_words_read;
		}

	      /*
	       * Long case, handles all possiblities (even if above tests were
	       * not done).  The 12% case.
	       */

	      else
		{
		  /* Save size_left and trace_ptr to global memory */
		  *size_left_ptr = size_left;
		  *trace_ptr_ptr = trace_ptr;

		  /* Fix, do long process for only branches */
		  if (trace_words_read == -1)
		    {
		      S_read_trace_info (S_pnode, &sint);
		    }
		  else
		    {
		      /*
		       * Skip over trace words using function call
		       * (so that trace buffer will be reloaded)
		       */
		      while (trace_words_read > 0)
			{
			  S_get_trace_word (S_pnode);
			  trace_words_read--;
			}
		    }
		  /* Reload size_left and trace_ptr from global memory                             * to registers
		   */
		  size_left = *size_left_ptr;
		  trace_ptr = *trace_ptr_ptr;
		}
	    }
	}

      if (S_profile_cycle_counts)
	{
	  if (sint.oper->flags & STATS_ON_ATTR)
	    {
	      S_stats_region_ctr++;
	      /* add an entry to stats stack */
	      temp_stack = S_stack;
	      S_stack = (S_Stats_Stack *) malloc (sizeof (S_Stats_Stack));
	      S_stack->next = temp_stack;
	      S_stack->pc = pc;
	      S_stack->func_name_id = sint.oper->operand[S_first_src];
	      S_stack->line_no = sint.oper->operand[S_first_src + 1];
	      S_stack->loop_type_id = sint.oper->operand[S_first_src + 2];
	      S_stack->start_cycle = S_num_profiled + (double) num_profiled;
	      /* get loop type */
	      strncpy (loop_type,
		       &operand_tab[sint.oper->operand[S_first_src + 2]]->
		       string[3], 3);
	      /* loop type in format 's "loop_type"', 
	       * hash on 'loop_type[3]'
	       */
	      loop_type[3] = 0;
	      if (!
		  (hash_entry =
		   (S_Hash_Entry *) STRING_find_symbol_data (tbl, loop_type)))
		{
		  hash_entry =
		    (S_Hash_Entry *) malloc (sizeof (S_Hash_Entry));
		  hash_entry->cycle_count = 0;
		  hash_entry->line_no = 0;
		  STRING_add_symbol (tbl, loop_type, (void *) hash_entry);
		}
	      if (hash_entry->line_no == 0)
		{
		  hash_entry->func_name_id = S_stack->func_name_id;
		  hash_entry->line_no = S_stack->line_no;
		}
	    }
	  if (S_stats_region_ctr)
	    {
	      S_region_cycles_ctr++;
	      if (sint.oper->flags & STATS_OFF_ATTR)
		{
		  S_stats_region_ctr--;
		  /* make sure we are in a stats region (stack != NULL) */
		  if (!S_stack)
		    S_punt
		      ("Too many stats_off attributes seen in function %s at oper %d\n",
		       sint.fn->name, sint.oper->lcode_id);
		  /* make sure stats region at top of stack matches
		   * current function.
		   */
		  if (
		      (S_stack->func_name_id !=
		       sint.oper->operand[S_first_src])
		      || (S_stack->line_no !=
			  sint.oper->operand[S_first_src + 1]))
		    S_punt
		      ("stats_off func %s line %s doesn't match stats_on func %s line %s\n",
		       operand_tab[sint.oper->operand[S_first_src]]->string,
		       operand_tab[sint.oper->operand[S_first_src + 1]]->
		       string, operand_tab[S_stack->func_name_id]->string,
		       operand_tab[S_stack->line_no]->string);
		  /* add number of cycles in this invocation of
		   * the stats region to counter associated with it.
		   */
		  S_stats_ctr[S_stack->pc] += S_num_profiled +
		    (double) num_profiled - S_stack->start_cycle;
		  /* add number of cycles in this invocation of
		   * the stats region to counter associated with its
		   * loop type (if any). */
		  strncpy (loop_type,
			   &operand_tab[sint.oper->operand[S_first_src + 2]]->
			   string[3], 3);
		  /* loop type in format 's "loop_type"', 
		   * hash on 'loop_type[3]'
		   */
		  loop_type[3] = 0;
		  if (!
		      (hash_entry =
		       (S_Hash_Entry *) STRING_find_symbol_data (tbl,
								 loop_type)))
		    {
		      S_punt
			("Should have entry for loop type %s at stats_off func %s line %s\n",
			 loop_type,
			 operand_tab[sint.oper->operand[S_first_src]]->string,
			 operand_tab[sint.oper->operand[S_first_src + 1]]->
			 string);
		    }
		  if (sint.oper->operand[S_first_src] ==
		      hash_entry->func_name_id
		      && sint.oper->operand[S_first_src + 1] ==
		      hash_entry->line_no)
		    {
		      hash_entry->cycle_count += S_num_profiled +
			(double) num_profiled - S_stack->start_cycle;
		      /* reset id fields in hash_entry */
		      hash_entry->func_name_id = 0;
		      hash_entry->line_no = 0;
		    }
		  /* pop off last stats_on */
		  temp_stack = S_stack;
		  S_stack = S_stack->next;
		  free (temp_stack);
		}
	    }
	}

      /* get flags */
      flags = prof_info[pc].flags;

      /* Detect if this instruction needs profiling */
      if (flags & PROF_INSTR)
	{
	  /* If this is a memory operation, do appropriate cache operations */
	  if (flags & (LOAD | STORE | JSR | PREFETCH))
	    {
	      /* Moved inside if to make my predicate profiling go faster.
	       * -JCG 5/11/95.
	       */

	      if (S_min_addr == 0 ||
		  (unsigned) sint.trace.mem_addr < S_min_addr)
		S_min_addr = (unsigned) sint.trace.mem_addr;
	      if ((unsigned) sint.trace.mem_addr > S_max_addr)
		S_max_addr = (unsigned) sint.trace.mem_addr;
	      distr_data = 0;

	      if (S_distribution && sim_cycle >= (double) S_distr_time_start
		  && sim_cycle < (double) S_distr_time_end)
		{
		  if ((block =
		       S_cache_find_addr (distr_cache, sint.trace.mem_addr)))
		    {
		      distr_data = (Distr_data *) block->data;
		      distr_data->count++;
		    }
		  else
		    outside_range++;
		}
	      load = flags & LOAD;
	      store = flags & STORE;
	      prefetch = flags & PREFETCH;
	      if (distr_data)
		distr_data->is_load |= load;
	      playdoh_flags = sint.playdoh_flags;
	      v1 = playdoh_flags & PLAYDOH_TCHS_V1;
	      c1 = playdoh_flags & PLAYDOH_TCHS_C1;
	      c2 = playdoh_flags & PLAYDOH_TCHS_C2;
	      c3 = playdoh_flags & PLAYDOH_TCHS_C3;
	      none = !(playdoh_flags & PLAYDOH_TCHS_ALL);	/* check if in the data cache */

	      block_size = S_dcache_block_size;
	      write_allocate = S_dcache_write_allocate;
	      this_cache = cache;
	      block = S_cache_find_addr (cache, sint.trace.mem_addr);
	      if (!block && S_prefetch_cache)
		{
		  block = S_cache_find_addr (pcache, sint.trace.mem_addr);
		  if (block || v1)
		    {
		      block_size = S_pcache_block_size;
		      write_allocate = 0;
		      this_cache = pcache;
		    }
		}

	      if (S_distribution)
		{
		  if (!block && distr_data)
		    distr_data->is_miss = 1;

		  /* Duplicate the below code from other sections of outer
		   * loop so that we can continue on to next iteration 
		   * without missing anything.
		   */

		  /* Update counts */
		  if (load)
		    S_num_loads++;
		  if (store)
		    S_num_stores++;
		  if (prefetch)
		    S_num_prefetches++;

		  /* Update miss statistics. */
		  prof_info[pc].num_misses++;
		  if (!prefetch && !store)
		    {
		      S_num_misses++;	/* Number of load misses */
		    }
		  if (!store)
		    S_num_misses_pref++;	/* Number of load/pref misses */
		  S_num_misses_stores++;	/* Number of load/pref/store misses */

		  prof_info[pc].num_executed++;

		  /* check if we are entering a new func */
		  if (opc_info_tab[sint.oper->opc].opc_type == JSR_OPC &&
		      sint.flags & BRANCHED)
		    S_current_func_no++;
		  else if (opc_info_tab[sint.oper->opc].opc_type == RTS_OPC)
		    S_current_func_no--;

		  if (sint.flags & BRANCHED)
		    pc = sint.trace.target_pc;
		  else
		    pc++;

		  continue;
		}

	      /* If a miss in c1 and v1 caches, take appropriate actions. */
	      if (!block)
		{
		  /* Allocate block in cache (replace LRU) if:
		   * 1) no specifiers set, and this is either not a store
		   *    operation, or if it is a write allocate cache.
		   * 2) target cache specifier is c1 or v1.
		   */
		  if ((none && (write_allocate || !store)) || c1 ||
		      (v1 && S_prefetch_cache))
		    {
		      block =
			S_cache_find_LRU (this_cache, sint.trace.mem_addr);
		      data = (Profile_data *) block->data;
		      S_incr_temporal_reuses (data, block_size);
		      S_cache_change_addr (this_cache, block,
					   sint.trace.mem_addr);
		      first_byte =
			((unsigned) sint.trace.mem_addr % block_size);
		      num_bytes = opc_info_tab[sint.oper->opc].access_size;
		      data->last_ref_pc_block = pc;
		      data->num_hits = 0;
		      for (i = 0; i < block_size; i++)
			{
			  if (i > first_byte
			      && i < first_byte + (unsigned) num_bytes)
			    {
			      data->last_ref_pc[i] = pc;
			      data->access_ctr[i] = S_num_loads;
			    }
			  else
			    data->last_ref_pc[i] = 0;
			}
		      /* need to deallocate rest of linked list */
		      next = data->next;
		      data->next = 0;
		      for (data = next; data; data = next)
			{
			  next = data->next;
			  free (data->last_ref_pc);
			  free (data->access_ctr);
			  free (data);
			}
		    }

		  /* Update miss statistics. */
		  prof_info[pc].num_misses++;
		  if (!prefetch && !store)
		    {
		      S_num_misses++;	/* Number of load misses */
		    }
		  if (!store)
		    S_num_misses_pref++;	/* Number of load/pref misses */
		  S_num_misses_stores++;	/* Number of load/pref/store misses */

		  /* If there is a secondary cache then check if there
		   * was a hit, otherwise allocate if:
		   * 1) no specifiers set, and this is either not a store
		   *    operation, or if primary cache is a write allocate cache.
		   * 2) target cache specifier is c1, v1, or c2.
		   */
		  if (S_secondary_cache)
		    {
		      block = S_cache_find_addr (scache, sint.trace.mem_addr);
		      if (!prefetch && !store)
			S_num_loads_secondary++;
		      if (!block)
			{
			  if (!prefetch && !store)
			    S_num_misses_secondary++;
			  if ((none && (write_allocate || !store)) || c1 || v1
			      || c2)
			    {
			      block =
				S_cache_find_LRU (scache,
						  sint.trace.mem_addr);
			      S_cache_change_addr (scache, block,
						   sint.trace.mem_addr);
			    }
			}
		    }
		}

	      /* Was a hit, just get block and update reuse counts for block
	       */
	      else
		{
		  first_byte = ((unsigned) sint.trace.mem_addr % block_size);
		  num_bytes = opc_info_tab[sint.oper->opc].access_size;
		  access_mask_ls = access_mask_ms = 0;
		  for (i = first_byte;
		       i < first_byte + (unsigned) num_bytes
		       && i < block_size; i++)
		    {
		      access_mask_ls |= (1 << (i % int_size));
		      access_mask_ms |= (1 << (i / int_size));
		    }

		  data = (Profile_data *) block->data;
		  last = 0;
#if 0
		  for (data = (Profile_data *) block->data; data;
		       data = data->next)
		    {
#endif
		      /* figure out byte accessed within block */
		      for (i = 0; i < block_size; i++)
			{
			  /*
			     is_temporal = (access_mask_ls & (1 << (i%int_size))) &&
			     (access_mask_ms & (1 << (i/int_size)));
			   */
			  is_temporal = ((i >= first_byte) &&
					 (i < first_byte + num_bytes));
			  last_ref_pc = data->last_ref_pc[i];
			  if (!last_ref_pc)
			    continue;
			  if (last_ref_pc == last)
			    continue;
			  access_ctr = data->access_ctr[i];
			  info = &prof_info[last_ref_pc];

			  /* temporal reuse */
			  if (is_temporal)
			    {
			      info->num_temporal_reuses++;
			      last = last_ref_pc;
			    }

			  /* spatial reuse */
			  if (!is_temporal
			      && (S_num_loads - access_ctr) < srr_window)
			    {
			      info->num_spatial_reuses++;
			      last = last_ref_pc;
			    }
			}

		      if (data->last_ref_pc_block)
			{
			  prof_info[data->last_ref_pc_block].num_reuses++;
			}
#if 0
		    }
#endif

		  /* Keep track of last pc to access block for reuse counts and
		   * make this block MRU (if wasn't a store miss in no write allocate
		   * cache).
		   */
		  if (block)
		    {
		      data = (Profile_data *) block->data;
		      data->last_ref_pc_block = pc;
		      data->num_hits++;
		      for (i = first_byte;
			   i < first_byte + num_bytes && i < block_size; i++)
			{
			  data->last_ref_pc[i] = pc;
			  data->access_ctr[i] = S_num_loads;
			}
#if 0
		      if (data->next)
			{
			  data = data->next;
			  for (; data->next && data->last_ref_pc_block != pc;
			       data = data->next);
			}
		      if (!(data->last_ref_pc_block == pc &&
			    data != (Profile_data *) block->data))
			{
			  data->next = S_create_Profile_data ();
			  data = data->next;
			  for (i = first_byte;
			       i < first_byte + num_bytes && i < block_size;
			       i++)
			    {
			      if (!data->last_ref_pc[i])
				data->access_ctr[i] = S_num_loads;
			      data->last_ref_pc[i] = pc;
			    }
			  data->last_ref_pc_block = pc;
			}
#endif

		      S_cache_make_MRU (this_cache, block);
		    }

		  /* Target cache specifier not c1 (primary cache) - invalidate */
		  /* if (!none && !c1) S_cache_invalidate(this_cache,block); */

		  /* If was a hit in c1 but target specifier is v1, move to v1 */
		  if (v1 && S_prefetch_cache && (this_cache == cache))
		    {
		      /* Have to invalidate any sub-blocks of new pcache
		       * block that are in the primary cache.
		       */
		      first_byte =
			(unsigned) sint.trace.mem_addr % S_pcache_block_size;
		      for (i = first_byte; i < S_pcache_block_size;
			   i += (unsigned) S_dcache_block_size)
			{
			  block = S_cache_find_addr (cache, first_byte);
			  if (block)
			    {
			      data = (Profile_data *) block->data;
			      S_incr_temporal_reuses (data,
						      S_dcache_block_size);
			      S_cache_invalidate (cache, block);
			    }
			}
		      block = S_cache_find_LRU (pcache, sint.trace.mem_addr);
		      /* Assume rest of pcache block brought in from mem */
		      data = (Profile_data *) block->data;
		      S_incr_temporal_reuses (data, S_pcache_block_size);
		      S_cache_change_addr (pcache, block,
					   sint.trace.mem_addr);
		      first_byte =
			((unsigned) sint.trace.mem_addr %
			 S_pcache_block_size);
		      num_bytes = opc_info_tab[sint.oper->opc].access_size;
		      data->last_ref_pc_block = pc;
		      data->num_hits = 0;
		      for (i = 0; i < S_pcache_block_size; i++)
			{
			  if (i > first_byte
			      && i < first_byte + (unsigned) num_bytes)
			    {
			      data->last_ref_pc[i] = pc;
			      data->access_ctr[i] = S_num_loads;
			    }
			  else
			    data->last_ref_pc[i] = 0;
			}
		      /* need to deallocate rest of linked list */
		      next = data->next;
		      data->next = 0;
		      for (data = next; data; data = next)
			{
			  next = data->next;
			  free (data->last_ref_pc);
			  free (data->access_ctr);
			  free (data);
			}
		      /* Already in scache since was in primary cache */
		    }
		}

	      /* Update counts */
	      if (load)
		S_num_loads++;
	      if (store)
		S_num_stores++;
	      if (prefetch)
		S_num_prefetches++;

	      /* If we are doing memory dependence profiling, then
	       * check if we are aliasing with anyone.
	       */
	      if (S_mem_dep_profiling)
		{
		  switch (S_mem_dep_model)
		    {
		    case MEM_DEP_MODEL_GUIDED_PDC_COLLECT:
		      S_mem_dep_guided_collection_profiling (&sint, pc);
		      break;
		    case MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT:
		      S_mem_dep_unguided_collection_profiling (&sint, pc);
		      break;
		    default:
		      S_punt ("S_mem_dep_model not defined");
		      break;
		    }
		}
	    }

	  /* Start JCG */
	  /* Get profile info structure for ease of use */
	  pinfo = &prof_info[pc];
	  sint_flags = sint.flags;

	  /* Do branch profiling if directed */
	  if (flags & PROF_BRANCH)
	    {
	      predicted_target = S_get_BTB_prediction (&prof_pnode, &sint);

	      /* sint flags changed, update sint_flags -JCG 9/7/95 */
	      sint_flags = sint.flags;

	      /* Detect misprediction */
	      if (sint_flags & MISPREDICTED)
		{
		  pinfo->num_mispredicted++;
		  S_num_branch_mispredictions++;
		}
	      else if (sint_flags & UNTRACED_JSR)
		{
		  if (S_flush_pipe_on_untraced_jsr)
		    {
		      pinfo->num_mispredicted++;
		      S_num_branch_mispredictions++;
		    }
		}
	      S_num_branches++;
	    }

	  /* Detect if profiling predicates */
	  if (flags & PROF_PREDICATE)
	    {
	      if (sint_flags & PRED_SQUASHED)
		pinfo->num_pred_squashed++;
	      S_num_predicates++;
	    }

	  /* Detect setting of dest predicates by pred def instruction */
	  if (flags & PROF_PRED_DEF)	/* JCG 6/8/95 */
	    {
	      if (sint_flags & PRED_DEST0_SET)
		pinfo->num_pred_dest0_set++;

	      if (sint_flags & PRED_DEST1_SET)
		pinfo->num_pred_dest1_set++;
	      S_num_pred_defs++;
	    }

	  /* Profile predicate before promotion.  Invalid data is
	   * flagged when current predicate indicates squash, but
	   * predicate before promotion doesn't.  (Promoted data
	   * is invalid after scheduling and after some loop optimizations).
	   */
	  if (flags & PROF_PROMOTED)
	    {
	      if (sint_flags & PROMOTED_PRED_SQUASHED)
		pinfo->num_promoted_pred_squashed++;

	      /* If actual predicate says squash, but the before
	       * promotion one doesn't, mark data as invalid
	       */
	      else if (sint_flags & PRED_SQUASHED)
		pinfo->flags |= PROMOTED_DATA_INVALID;
	      S_num_promoted++;
	    }

	  /* Track icache misses caused by this instruction */
	  if (flags & PROF_ICACHE)	/* JCG 2/18/97 */
	    {
	      /* See if this instruction is in the cache */
	      block = S_cache_find_addr (icache, sint.oper->instr_addr);

	      /* If not, count miss and put into icache */
	      if (block == NULL)
		{
		  S_num_icache_misses++;
		  pinfo->num_icache_misses++;

		  /* We are going to kick out the LRU block */
		  block = S_cache_find_LRU (icache, sint.oper->instr_addr);

		  /* Change LRU block to new address so this instruction
		   * is in the cache
		   */
		  S_cache_change_addr (icache, block, sint.oper->instr_addr);
		}

	      /* If hit or miss, make block that has instruction now 
	       * most recently used in cache line 
	       */
	      S_cache_make_MRU (icache, block);
	    }

	}
      /* end JCG */


      prof_info[pc].num_executed++;

      /* check if we are entering a new func */
      /* lib functions all have side-effects since side-effect-free functions
       * are not profiled at all
       */


      if (opc_info_tab[sint.oper->opc].opc_type == JSR_OPC &&
	  (sint.flags & BRANCHED) == 0)
	{

	  /* JWS 20000106 - Conditional. */
	  if (S_mem_dep_profiling &&
	      S_mem_dep_model == MEM_DEP_MODEL_GUIDED_PDC_COLLECT)
	    {
	      S_update_jsr_side_effects ();
	      symbol = INT_find_symbol (afs_table,
					(pc << 16) | activation_record_depth);
	      if (symbol == 0)
		{
		  afs = S_new_addr_func_summary ();
		  afs->fn = 0;
		  afs->active = 0;
		  afs->lib_name = operand_tab[sint.oper->operand[2]]->string;
		  INT_add_symbol (afs_table,
				  (pc << 16) | activation_record_depth, afs);
		}
	      else
		{
		  afs = symbol->data;
		  if (afs->active == 1)
		    S_punt ("Activation record error\n");
		}
	      afs->unsafe = 1;
	    }
	}
      if (opc_info_tab[sint.oper->opc].opc_type == JSR_OPC &&
	  sint.flags & BRANCHED)
	{

	  jsr_pc = sint.trace.target_pc;
	  if (jsr_pc)
	    {
	      jsr_sint.oper = oper_tab[jsr_pc];
	      jsr_sint.fn = jsr_sint.oper->cb->fn;
	      jsr_sint.flags = 0;

	      /* JWS 20000106 - Conditional. */
	      if (S_mem_dep_profiling &&
		  S_mem_dep_model == MEM_DEP_MODEL_GUIDED_PDC_COLLECT)
		{
		  S_current_func_no++;

		  symbol = INT_find_symbol (afs_table,
					    (pc << 16) |
					    activation_record_depth);
		  if (symbol == 0)
		    {
		      afs = S_new_addr_func_summary ();
		      afs->fn = jsr_sint.fn;
		      INT_add_symbol (afs_table,
				      (pc << 16) | activation_record_depth,
				      afs);
		    }
		  else
		    {
		      afs = symbol->data;
		      if (afs->active == 1)
			S_punt ("Activation record error\n");
		      afs->active = 1;
		    }
		  activation_record[activation_record_depth].afs = afs;
		  activation_record_depth++;
		  if (activation_record_depth == max_activation_record_depth)
		    {
		      max_activation_record_depth += 20;
		      activation_record = realloc (activation_record,
						   sizeof
						   (S_Activation_Record) *
						   max_activation_record_depth);
		    }
		}
	    }
	}
      else if (opc_info_tab[sint.oper->opc].opc_type == RTS_OPC)
	{

	  /* JWS 20000106 - Conditional. */
	  if (S_mem_dep_profiling &&
	      S_mem_dep_model == MEM_DEP_MODEL_GUIDED_PDC_COLLECT)
	    {
	      S_current_func_no--;

	      if (S_current_func_no != -1)
		{
		  for (symbol = afs_table->head_symbol;
		       symbol; symbol = symbol->next_symbol)
		    {
		      afs = symbol->data;
		      if (afs->active == 0)
			{
			  INT_delete_symbol (symbol,
					     S_remove_addr_func_summary);
			}
		    }
		  afs = activation_record[activation_record_depth - 1].afs;
#if 0
		  S_dump_activation_record ();
#endif
		  afs->active = 0;
		  activation_record_depth--;
		  if (activation_record_depth == -1)
		    S_punt ("activation_record_depth underflow\n");
		  activation_record[activation_record_depth].afs = 0;
		}
	    }
	}

      if (sint.flags & BRANCHED)
	pc = sint.trace.target_pc;
      else
	pc++;
    }

  /* Save size_left and trace_ptr to global memory */
  if (S_profile_cycle_counts_only)
    {
      *size_left_ptr = size_left;
      *trace_ptr_ptr = trace_ptr;
    }

  /* Update stats */
  S_num_profiled += (double) num_profiled;

  if (S_profile_cycle_counts && S_region_cycles_ctr > 1000000000)
    {
      S_region_cycles_ctr -= 1000000000;
      S_region_billions_cycles_ctr++;
    }

  if (S_mem_dep_profiling)
    {
      switch (S_mem_dep_model)
	{
	case MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT:
	  S_delete_hash ();
	  if (aliases_in_file > S_max_file_aliases)
	    S_consolidate_aliases_file ();
	  break;
	case MEM_DEP_MODEL_GUIDED_PDC_COLLECT:
	  S_delete_pdc_hash ();
	  break;
	default:
	  S_punt ("S_mem_dep_model not defined");
	  break;
	}
    }

  if (S_distribution)
    {
      S_print_distr (outside_range, sim_cycle);
    }

  return (pc);
}

void
S_print_distr (int outside_range, double sim_cycle)
{
  Scblock *block;
  Distr_data *data;
  int i;

  for (i = 0; i <= distr_cache->tag_index_mask; i++)
    {
      for (block = distr_cache->tag_store[i].head; block != NULL;
	   block = block->tag_next)
	{
	  if (block->hash_next != (Scblock *) - 1)
	    {
	      data = (Distr_data *) block->data;
	      if (data->count)
		{
		  fprintf (distr_file, "%d %d %12.0lf\t%12u\n",
			   data->is_load,
			   data->is_miss, sim_cycle, block->start_addr);
		  data->count = 0;
		  data->is_miss = 0;
		  data->is_load = 0;
		}
	    }
	}
    }
  fflush (distr_file);
  /*
     if (outside_range)
     fprintf (distr_file, "%12.0lf\t%12u\n", sim_cycle, 
     S_distr_entries*S_distr_entry_size+S_distr_base_addr);
   */
}

void
S_init_distr (int outside_range)
{
  Scblock *block;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int i;
#endif
  unsigned addr;

  distr_cache = S_create_cache (S_distr_entry_size * S_distr_entries,
				S_distr_entry_size, 0, S_create_Distr_data);
  for (addr = S_distr_base_addr;
       addr < S_distr_base_addr + S_distr_entry_size * S_distr_entries;
       addr += S_distr_entry_size)
    {
      block = S_cache_find_LRU (distr_cache, addr);
      S_cache_change_addr (distr_cache, block, addr);
      S_cache_make_MRU (distr_cache, block);
    }
}


void
S_init_annotation_stat_names ()
{

  if (S_profile_memory_accesses)
    {
      dmr_histo = L_create_histogram ("dmr's", 11);
      drr_histo = L_create_histogram ("drr's", 11);
      srr_histo = L_create_histogram ("spatial rr's", 11);
      trr_histo = L_create_histogram ("temporal rr's", 11);
      dmc_histo = L_create_histogram ("dmc's", 11);
    }
  if (S_profile_branches)
    {
      /* Set the attribute name based on the BTB model */
/*** changed from global to in btb here ***/
      branch_exec_attr = "BRANCH_STATE";
      switch (S_pnode->btb->model)
	{
	case BTB_MODEL_PERFECT:
	case BTB_MODEL_ALWAYS_WRONG:

/*** need a btb struct to reference model_name ***/
	  S_punt ("Profiling BTB with %s model.  Doesn't make sense.",
		  S_pnode->btb->model_name);
	case BTB_MODEL_COUNTER:
	  branch_attr = "MP_CTR";
	  break;
	case BTB_MODEL_STATIC:
	  branch_attr = "MP_STA";
	  break;
	case BTB_MODEL_BTC:
	  branch_attr = "MP_BTC";
	  break;
	case BTB_MODEL_TWO_LEVEL:
	  branch_attr = "MP_2LV";
	  break;
	case BTB_MODEL_PREDICATE:
/*** reference predicate_prediction_model through some global pnode variable? ***/
	  switch (S_pnode->btb->predicate_prediction_model)
	    {
	    case BTB_PREDICATE_MODEL_POP:
	      branch_attr = "MP_PRED_POP";
	      break;
	    case BTB_PREDICATE_MODEL_PEP:
	      branch_attr = "MP_PRED_PEP";
	      break;
	    case BTB_PREDICATE_MODEL_PEP_HISTORY:
	      branch_attr = "MP_PRED_PEPH";
	      break;
	    default:
	      S_punt
		("Profiling predicate BTB with %s model. Need to set attr.",
		 S_pnode->btb->predicate_prediction_model);
	    }
	  break;
	default:
	  S_punt ("Profiling BTB with %s model. Need to set attr.",
		  S_pnode->btb->model_name);
	}
    }
}

void
S_write_profile (FILE * out)
{
  /* Scan data gathered and write the annotation files */
  L_AttrMngr *L_AM;
  S_Fn *fn;
  S_Oper op;
  double count;
  double doub_val;
  float float_val;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  long int_val;
  int j, pc, source_pc, alias_times;
  char attr_name[50];
  int value;
#endif
  int i, flags;
  int ll_count, sl_count, ls_count, ss_count;
  char dep_name[15];
  S_Profile_Info *pinfo;
  S_MemDep_Alias_Info *info;
  double num_skipped, num_profiled;
  char command[100];
  S_Hash_Entry *hash_entry;
  A_Attr_Field **field;

  if (S_profile_memory_accesses)
    {
      S_incr_all_temp_reuses (cache);
      if (S_prefetch_cache)
	S_incr_all_temp_reuses (pcache);
    }

  S_init_annotation_stat_names ();

  if (S_mem_dep_profiling)
    {
      if (S_mem_dep_model == MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT)
	{
	  fclose (tmp_file);
	  // sprintf (command, "sort -o %s %s", tmp_name, tmp_name);
	  sprintf (command, "sort -n -k2 -o %s %s", tmp_name, tmp_name);
	  system (command);
	  if ((tmp_file = fopen (tmp_name, "r")) == 0)
	    S_punt ("Unable to open temp file\n");
	}
      else if (S_mem_dep_model == MEM_DEP_MODEL_GUIDED_PDC_COLLECT)
	{
	  S_print_pdc_profile ();
	}
      else
	S_punt ("S_write_profile : illegal memory profile model");
    }

  for (fn = head_fn; fn; fn = fn->next_fn)
    {
      L_AM = L_create_AttrMngr (fn->name, 1.0);

      /* insert function attrs */
      if (S_profile_memory_accesses)
	L_insert_fn_attr_int (L_AM, LMC, S_num_misses);

      /* insert op attrs */
      for (i = 0; i < fn->op_count; i++)
	{
	  op = fn->op[i];
	  flags = prof_info[op.pc].flags;

	  /* Get pinfo for ease of use */
	  pinfo = &prof_info[op.pc];

	  /* Add attribute for stats cycle counter if this is
	   * a sim_dir oper with the stats_on attribute.
	   */
	  if (S_profile_cycle_counts && (op.flags & STATS_ON_ATTR))
	    {
	      L_insert_op_attr_float (L_AM, op.lcode_id, "cycle_ctr",
				      S_stats_ctr[op.pc], 0);
	    }

	  /* Allow annotation of the issue weight of each instruction.
	   * Mainly for debugging profiler/ease of code persual.
	   * (Allows original weights to be seen after scheduleing).
	   * -JCG 6/11/95
	   */
	  if (S_profile_issue_weight)
	    {
	      L_insert_op_attr_int (L_AM, op.lcode_id, "IW",
				    pinfo->num_executed, 0);
	    }


	  if (flags & PROF_INSTR)
	    {

	      if (flags & PROF_BRANCH)
		{
		  L_insert_op_attr_int (L_AM, op.lcode_id, exec_attr,
					pinfo->num_executed, 0);
		  L_insert_op_attr_int (L_AM, op.lcode_id, branch_attr,
					pinfo->num_mispredicted, 0);
		  L_insert_op_attr_int (L_AM, op.lcode_id, branch_exec_attr,
					pinfo->branch_exec_state, 0);
		}

	      if (flags & PROF_PREDICATE)
		{
		  L_insert_op_attr_int (L_AM, op.lcode_id, exec_attr,
					pinfo->num_executed, 0);
		  L_insert_op_attr_int (L_AM, op.lcode_id, pred_attr,
					pinfo->num_pred_squashed, 0);
		}

	      if (flags & PROF_PRED_DEF)
		{
		  /* JWS 20000106 - don't add again if already counted */
		  if (!(flags & PROF_PREDICATE))
		    L_insert_op_attr_int (L_AM, op.lcode_id, exec_attr,
					  pinfo->num_executed, 0);
		  if (op.operand[S_first_dest] > 0)
		    {
		      L_insert_op_attr_int (L_AM, op.lcode_id,
					    pred_dest0_set_attr,
					    pinfo->num_pred_dest0_set, 0);
		    }
		  if (op.operand[S_first_dest + 1] > 0)
		    {
		      L_insert_op_attr_int (L_AM, op.lcode_id,
					    pred_dest1_set_attr,
					    pinfo->num_pred_dest1_set, 0);
		    }
		}
	      if (flags & PROF_PROMOTED)
		{
		  if (flags & PROMOTED_DATA_INVALID)
		    {
		      L_insert_op_attr_int (L_AM, op.lcode_id, exec_attr,
					    pinfo->num_executed, 0);
		      L_insert_op_attr_int (L_AM, op.lcode_id,
					    promoted_pred_attr, -1, 0);
		      fprintf (stderr,
			       "Warning %s op %i: Invalid promoted predicate profile data!\n",
			       fn->name, op.lcode_id);
		    }
		  else
		    {
		      L_insert_op_attr_int (L_AM, op.lcode_id,
					    promoted_pred_attr,
					    pinfo->num_promoted_pred_squashed,
					    0);
		    }

		}

	      /* Write out the profile info for icache misses, if any */
	      if (flags & PROF_ICACHE)	/* JCG 2/18/96 */
		{
		  if (pinfo->num_icache_misses > 0)
		    {
		      L_insert_op_attr_int (L_AM, op.lcode_id, "IM",
					    pinfo->num_icache_misses, 0);
		    }
		}
	    }

	  /* Make dmr attributes */
	  if (!(flags & LOAD || flags & STORE || flags & PREFETCH))
	    continue;
	  if (!prof_info[op.pc].num_executed)
	    continue;
	  if (!S_profile_memory_accesses)
	    continue;

	  if (prof_info[op.pc].num_executed)
	    doub_val = ((double) prof_info[op.pc].num_misses) /
	      prof_info[op.pc].num_executed;
	  else
	    doub_val = 0.0;
	  float_val = (float) doub_val;
	  L_insert_op_attr_float (L_AM, op.lcode_id, MR, float_val,
				  op.dep_id);
	  L_update_histogram (dmr_histo, ceil (doub_val * 10.0), 1.0);

	  L_insert_op_attr_int (L_AM, op.lcode_id, "exec_freq",
				prof_info[op.pc].num_executed, op.dep_id);

	  /* make dmc attributes */
	  if (flags & PREFETCH)
	    {
	      L_insert_op_attr_int (L_AM, op.lcode_id, PMC,
				    prof_info[op.pc].num_misses, op.dep_id);
	    }
	  else if (flags & LOAD)
	    {
	      L_insert_op_attr_int (L_AM, op.lcode_id, MC,
				    prof_info[op.pc].num_misses, op.dep_id);
	      if (S_num_misses != 0.0)
		doub_val =
		  ((double) prof_info[op.pc].num_misses) * 10.0 / S_num_misses;
	      else
		doub_val = 0.0;
	      L_update_histogram (dmc_histo, ceil (doub_val), 1.0);
	    }
	  else if (flags & STORE)
	    {
	      L_insert_op_attr_int (L_AM, op.lcode_id, MC,
				    prof_info[op.pc].num_misses, op.dep_id);
	    }

	  /* Make drr attributes */
	  if (prof_info[op.pc].num_executed)
	    doub_val = ((double) prof_info[op.pc].num_reuses) /
	      prof_info[op.pc].num_executed;
	  else
	    doub_val = 0.0;
	  float_val = (float) doub_val;
	  L_insert_op_attr_float (L_AM, op.lcode_id, RR, float_val,
				  op.dep_id);
	  if (doub_val > 49.0)
	    doub_val = 50.0;
	  L_update_histogram (drr_histo, ceil (doub_val * 10.0), 1.0);

	  /* Make srr attributes */
	  if (prof_info[op.pc].num_executed)
	    doub_val = ((double) prof_info[op.pc].num_spatial_reuses) /
	      prof_info[op.pc].num_executed;
	  else
	    doub_val = 0.0;
	  float_val = (float) doub_val;
	  L_insert_op_attr_float (L_AM, op.lcode_id, SRR, float_val,
				  op.dep_id);
	  if (doub_val > 49.0)
	    doub_val = 50.0;
	  L_update_histogram (srr_histo, ceil (doub_val * 10.0), 1.0);

	  /* Make trr attributes */
	  if (prof_info[op.pc].num_executed)
	    doub_val = ((double) prof_info[op.pc].num_temporal_reuses) /
	      prof_info[op.pc].num_executed;
	  else
	    doub_val = 0.0;
	  float_val = (float) doub_val;
	  L_insert_op_attr_float (L_AM, op.lcode_id, TRR, float_val,
				  op.dep_id);
	  if (doub_val > 49.0)
	    doub_val = 50.0;
	  L_update_histogram (trr_histo, ceil (doub_val * 10.0), 1.0);

	  /* make mdp attributes */
	  if ((S_mem_dep_profiling)
	      && (S_mem_dep_model == MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT))
	    {
	      S_read_alias_info (fn, op.pc);
	      ll_count = sl_count = ls_count = ss_count = 0;
	      for (info = (S_MemDep_Alias_Info *) prof_info[op.pc].ptr;
		   info; info = info->next)
		{
		  if (strcmp (oper_tab[info->source_pc]->cb->fn->name,
			      op.cb->fn->name))
		    continue;
		  field =
		    (A_Attr_Field **) malloc (sizeof (A_Attr_Field *) * 2);
		  field[0] = (A_Attr_Field *) malloc (sizeof (A_Attr_Field));
		  field[0]->type = A_INT;
		  field[0]->value.i = oper_tab[info->source_pc]->lcode_id;
		  field[1] = (A_Attr_Field *) malloc (sizeof (A_Attr_Field));
		  field[1]->type = A_INT;
		  field[1]->value.i = info->alias_times;
		  if (prof_info[op.pc].flags & (LOAD | PREFETCH))
		    {
		      if (prof_info[info->source_pc].
			  flags & (LOAD | PREFETCH))
			{
			  sprintf (dep_name, "dep_ll_%d", ++ll_count);
			  L_insert_op_attr_list (L_AM, op.lcode_id, dep_name,
						 field, 2, op.dep_id);
			}
		      else
			{
			  sprintf (dep_name, "dep_sl_%d", ++sl_count);
			  L_insert_op_attr_list (L_AM, op.lcode_id, dep_name,
						 field, 2, op.dep_id);
			}
		    }
		  else
		    {
		      if (prof_info[info->source_pc].
			  flags & (LOAD | PREFETCH))
			{
			  sprintf (dep_name, "dep_ls_%d", ++ls_count);
			  L_insert_op_attr_list (L_AM, op.lcode_id, dep_name,
						 field, 2, op.dep_id);
			}
		      else
			{
			  sprintf (dep_name, "dep_ss_%d", ++ss_count);
			  L_insert_op_attr_list (L_AM, op.lcode_id, dep_name,
						 field, 2, op.dep_id);
			}
		    }
		}
	      S_delete_alias_info ((S_MemDep_Alias_Info *) prof_info[op.pc].
				   ptr, 0);
	    }

	}
      L_write_attr_to_file (annot_file, L_AM);
      L_free_AttrMngr (L_AM);
    }

  if (S_profile_memory_accesses)
    {
      L_print_histogram (S_histogram_file, dmr_histo, L_PRINT_ALL_ENTRIES,
			 0.0);
      L_delete_histogram (dmr_histo);
      fprintf (S_histogram_file, "\n");
      L_print_histogram (S_histogram_file, dmc_histo, L_PRINT_ALL_ENTRIES,
			 0.0);
      L_delete_histogram (dmc_histo);
      fprintf (S_histogram_file, "\n");
      L_print_histogram (S_histogram_file, drr_histo, L_PRINT_ALL_ENTRIES,
			 0.0);
      L_delete_histogram (drr_histo);
      fprintf (S_histogram_file, "\n");
      L_print_histogram (S_histogram_file, srr_histo, L_PRINT_ALL_ENTRIES,
			 0.0);
      L_delete_histogram (srr_histo);
      fprintf (S_histogram_file, "\n");
      L_print_histogram (S_histogram_file, trr_histo, L_PRINT_ALL_ENTRIES,
			 0.0);
      L_delete_histogram (trr_histo);
    }

  /* JCG 5/8/95, Converted over to doubles to get accurate results
   * for big programs
   */
  num_skipped = (double) S_billions_skipped *(double) BILLION;
  num_skipped += (double) S_num_skip_on_path;
  num_profiled = S_num_profiled;

  /* Write other stats */
  fprintf (out, "#PROFILE SUMMARY:\n");
  fprintf (out, "%12lu profile samples.\n", S_num_prof_samples);
  fprintf (out, "%12.0f total instructions profiled.\n", num_profiled);
  fprintf (out, "%12lu loads profiled.\n", S_num_loads);
  fprintf (out, "%12lu stores profiled.\n", S_num_stores);
  fprintf (out, "%12lu prefetches profiled.\n", S_num_prefetches);
  fprintf (out, "%12lu branches profiled.\n", S_num_branches);
  fprintf (out, "%12lu predicates profiled.\n", S_num_predicates);
  fprintf (out, "%12lu promoted predicates profiled.\n", S_num_promoted);
  fprintf (out, "%12lu pred defs profiled.\n", S_num_pred_defs);
  fprintf (out, "%12.0f instructions skipped in program.\n", num_skipped);
  fprintf (out, "%12.0f total instruction count.\n",
	   num_profiled + num_skipped);
  fprintf (out, "%12.2lf percent of execution profiled.\n",
	   100.0 * num_profiled / (num_profiled + num_skipped));
  if (S_profile_cycle_counts)
    {
      count = ((double) S_region_billions_cycles_ctr) * 1000000000.0 +
	(double) S_region_cycles_ctr;
      fprintf (out, "%12.0f total stats region instruction count.\n", count);
      fprintf (out, "%12.2lf percent of execution in stats regions.\n",
	       100.0 * count / (num_profiled + num_skipped));
      if (
	  (hash_entry =
	   (S_Hash_Entry *) STRING_find_symbol_data (tbl, "dop")))
	{
	  fprintf (out,
		   "%12.0f total instruction count in doparallel loops.\n",
		   hash_entry->cycle_count);
	  fprintf (out, "%12.2lf percent of execution in doparallel loops.\n",
		   100.0 * hash_entry->cycle_count / (num_profiled +
						      num_skipped));
	}
      if (
	  (hash_entry =
	   (S_Hash_Entry *) STRING_find_symbol_data (tbl, "dos")))
	{
	  fprintf (out,
		   "%12.0f total instruction count in doserial loops.\n",
		   hash_entry->cycle_count);
	  fprintf (out, "%12.2lf percent of execution in doserial loops.\n",
		   100.0 * hash_entry->cycle_count / (num_profiled +
						      num_skipped));
	}
      if (
	  (hash_entry =
	   (S_Hash_Entry *) STRING_find_symbol_data (tbl, "for")))
	{
	  fprintf (out,
		   "%12.0f total instruction count in for loops.\n",
		   hash_entry->cycle_count);
	  fprintf (out, "%12.2lf percent of execution in for loops.\n",
		   100.0 * hash_entry->cycle_count / (num_profiled +
						      num_skipped));
	}
      if (
	  (hash_entry =
	   (S_Hash_Entry *) STRING_find_symbol_data (tbl, "whi")))
	{
	  fprintf (out,
		   "%12.0f total instruction count in while loops.\n",
		   hash_entry->cycle_count);
	  fprintf (out, "%12.2lf percent of execution in while loops.\n",
		   100.0 * hash_entry->cycle_count / (num_profiled +
						      num_skipped));
	}
      if (
	  (hash_entry =
	   (S_Hash_Entry *) STRING_find_symbol_data (tbl, "do\"")))
	{
	  fprintf (out,
		   "%12.0f total instruction count in do loops.\n",
		   hash_entry->cycle_count);
	  fprintf (out, "%12.2lf percent of execution in do loops.\n",
		   100.0 * hash_entry->cycle_count / (num_profiled +
						      num_skipped));
	}
    }

  if (S_profile_memory_accesses)
    {
      fprintf (out, "%12.4lu data cache misses (loads only).\n",
	       S_num_misses);
      fprintf (out, "%12.4lu data cache misses including prefetches.\n",
	       S_num_misses_pref);
      fprintf (out,
	       "%12.4lu data cache misses including stores and prefetches.\n",
	       S_num_misses_stores);
      fprintf (out, "%12.4lf data cache miss ratio (loads only).\n",
	       ((double) S_num_misses) / S_num_loads);
      if (S_secondary_cache)
	{
	  fprintf (out, "%12.4lu secondary data cache misses (loads only).\n",
		   S_num_misses_secondary);
	  fprintf (out,
		   "%12.4lf secondary data cache miss ratio (loads only).\n",
		   ((double) S_num_misses_secondary) / S_num_loads_secondary);
	}
      fprintf (out, "%12.4lu was lowest address accessed.\n", S_min_addr);
      fprintf (out, "%12.4lu was highest address accessed.\n", S_max_addr);
    }

  if (S_profile_branches)
    {
      fprintf (out, "%12lu branch mispredictions.\n",
	       S_num_branch_mispredictions);
      fprintf (out, "%12.4lf branch miss ratio.\n",
	       ((double) S_num_branch_mispredictions) / S_num_branches);
    }

  if (S_profile_icache_misses)
    {
      fprintf (out, "%12ld icache misses.\n", S_num_icache_misses);
    }

  fprintf (out, "\n");

  S_print_stats_global_system (out);

  if (S_mem_dep_profiling)
    {
      switch (S_mem_dep_model)
	{
	case MEM_DEP_MODEL_UNGUIDED_PDC_COLLECT:
	  L_free_alloc_pool (S_MemDep_Data_Info_pool);
	  L_free_alloc_pool (S_MemDep_Access_Info_pool);
	  L_free_alloc_pool (S_MemDep_Alias_Info_pool);
	  sprintf (command, "rm %s", tmp_name);
	  system (command);
	  break;
	case MEM_DEP_MODEL_GUIDED_PDC_COLLECT:
	  L_free_alloc_pool (S_Pdc_MemDep_Data_Info_pool);
	  break;
	default:
	  S_punt ("Illegal memory dependence profiling model\n");
	}
    }

  /* Check that ctr is = 0 at end */
  if (S_stats_region_ctr != 0)
    S_punt ("Profiling finished without ending stats_off attribute.\n");

  if (S_profile_cycle_counts)
    STRING_delete_symbol_table (tbl, 0);
}

static void *
S_create_Profile_data ()
{
  Profile_data *new_data;
  int i, block_size;

  block_size = S_dcache_block_size;
  if (S_prefetch_cache && S_pcache_block_size > block_size)
    block_size = S_pcache_block_size;
  if (S_secondary_cache && S_scache_block_size > block_size)
    block_size = S_scache_block_size;
  new_data = (Profile_data *) malloc (sizeof (Profile_data));
  new_data->last_ref_pc = (int *) malloc (sizeof (int) * block_size);
  new_data->access_ctr = (int *) malloc (sizeof (int) * block_size);
  for (i = 0; i < block_size; i++)
    {
      new_data->last_ref_pc[i] = 0;
      new_data->access_ctr[i] = 0;
    }
  new_data->last_ref_pc_block = 0;
  new_data->num_hits = 0;
  new_data->next = 0;

  return ((void *) new_data);
}

static void *
S_create_Distr_data ()
{
  Distr_data *new_data;

  new_data = (Distr_data *) malloc (sizeof (Distr_data));
  new_data->count = 0;
  new_data->is_miss = 0;
  new_data->is_load = 0;
  return ((void *) new_data);
}

void
S_consolidate_aliases_file ()
{
  S_Fn *fn;
  S_Oper op;
  char command[100];
  int i;

#if DEBUG_PROFILER
  printf ("Consolidating %d lines", aliases_in_file);
#endif
  i = fclose (tmp_file);
  // sprintf (command, "sort -o %s %s", tmp_name, tmp_name);
  sprintf (command, "sort -n -k2 -o %s %s", tmp_name, tmp_name);
  i = system (command);
  if ((tmp_file = fopen (tmp_name, "r")) == 0)
    S_punt ("Unable to open temp file\n");
  for (fn = head_fn; fn; fn = fn->next_fn)
    {
      for (i = 0; i < fn->op_count; i++)
	{
	  op = fn->op[i];
	  S_read_alias_info (fn, op.pc);
	}
    }
  fclose (tmp_file);
  sprintf (command, "rm %s", tmp_name);
  system (command);
  if (!(tmp_name = tmpnam (NULL)))
    S_punt ("Unable to create temp file name\n");
  if ((tmp_file = fopen (tmp_name, "w")) == 0)
    S_punt ("Unable to open temp file\n");
  aliases_in_file = 0;
  for (fn = head_fn; fn; fn = fn->next_fn)
    {
      for (i = 0; i < fn->op_count; i++)
	{
	  op = fn->op[i];
	  S_print_alias_info (
			      (S_MemDep_Alias_Info *) prof_info[op.pc].ptr,
			      op.pc);
	  S_delete_alias_info ((S_MemDep_Alias_Info *) prof_info[op.pc].ptr,
			       0);
	  prof_info[op.pc].ptr = 0;
	}
    }
  if (aliases_in_file > S_max_file_aliases * 0.9)
    S_max_file_aliases += MAX_FILE_ALIASES;
#if DEBUG_PROFILER
  printf (" to %d lines!\n", aliases_in_file);
#endif
}

S_MemDep_Alias_Info *
S_insert_alias_info (S_MemDep_Alias_Info * alias_info,
		     int source_pc, int alias_times, int *num_alias)
{
  S_MemDep_Alias_Info *info;

  if (!alias_info)
    {
      info = (S_MemDep_Alias_Info *) L_alloc (S_MemDep_Alias_Info_pool);
      info->source_pc = source_pc;
      info->alias_times = alias_times;
      info->next = NULL;
      if (num_alias)
	(*num_alias)++;
      return info;
    }
  info = alias_info;
  if (info->source_pc != source_pc)
    for (; info->next; info = info->next)
      if (info->source_pc == source_pc)
	break;
  if (info->source_pc == source_pc)
    {
      info->alias_times += alias_times;
    }
  else
    {
      info->next = (S_MemDep_Alias_Info *) L_alloc (S_MemDep_Alias_Info_pool);
      info = info->next;
      info->source_pc = source_pc;
      info->alias_times = alias_times;
      info->next = NULL;
      if (num_alias)
	(*num_alias)++;
    }
  return alias_info;
}

void
S_read_alias_info (S_Fn * load_fn, int pc)
{
  int i, read_pc, source_pc, alias_times, err;
  long int fn;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  S_Fn *fn1;
#endif

  for (i = 0; i < prof_info[pc].num_aliases; i++)
    {
      if ((err = fscanf (tmp_file, "%ld %d %d %d\n", &fn, &read_pc,
			 &source_pc, &alias_times)) != 4)
	{
	  printf ("%d\n", err);
	  S_punt ("Error in tmp_file format\n");
	}
      if (read_pc != pc)
	{
	  S_punt ("pc %d and read pc %d do not match\n", pc, read_pc);
	}
      if (oper_tab[read_pc]->cb->fn != load_fn)
	S_punt ("Functions of pc's do not match\n");
      prof_info[read_pc].ptr = (S_MemDep_Alias_Info *) S_insert_alias_info (
									    (S_MemDep_Alias_Info
									     *)
									    prof_info
									    [read_pc].
									    ptr,
									    source_pc,
									    alias_times,
									    0);
    }
  prof_info[pc].num_aliases = 0;
}

void
S_incr_temporal_reuses (Profile_data * data, int block_size)
{
  int hits, i;

  return;
  hits = data->num_hits - 1;
  for (data = data->next; data; data = data->next)
    {
      for (i = 0; i < block_size; i++)
	{
	  if (!data->last_ref_pc[i])
	    continue;
	  prof_info[data->last_ref_pc_block].num_temporal_reuses += hits--;
	  break;
	}
    }
}

void
S_incr_all_temp_reuses (Scache * cache)
{
  Scblock *block;
  Profile_data *data;
  int hits, i, j;

  return;
  for (i = 0; i <= cache->tag_index_mask; i++)
    {
      for (block = cache->tag_store[i].head; block != NULL;
	   block = block->tag_next)
	{
	  if (block->hash_next != (Scblock *) - 1)
	    {
	      data = (Profile_data *) block->data;
	      hits = data->num_hits - 1;
	      for (data = data->next; data; data = data->next)
		{
		  for (j = 0; j < cache->block_size; j++)
		    {
		      if (!data->last_ref_pc[j])
			continue;
		      prof_info[data->last_ref_pc_block].num_temporal_reuses
			+= hits--;
		      break;
		    }
		}
	    }
	}
    }
}

void S_create_load_alias (int source_pc, int sink_pc)
{
  S_hash_node *load_entry;
  S_MemDep_Access_Info *load_info;

  /* See if there is an entry in the hash table for this op */
  load_entry = S_hash_find_load_info (sink_pc);

  /* No entry, allocate a new one */
  if (!load_entry)
    {
      load_entry = S_hash_insert_load_info (sink_pc);
      if (aliases_in_file > S_max_file_aliases)
	S_consolidate_aliases_file ();
      load_info =
	(S_MemDep_Access_Info *) L_alloc (S_MemDep_Access_Info_pool);
      load_info->alias_info = NULL;
      load_info->pc = sink_pc;

      /* Set the alias info to show one alias with the last
       * store to this address.
       */
      load_info->alias_info = S_insert_alias_info (NULL, source_pc, 1,
						   &num_load_alias);
      load_entry->ptr = (S_MemDep_Access_Info *) load_info;
    }

  /* Was an entry for this load op, update alias info */
  else
    {
      load_info = (S_MemDep_Access_Info *) load_entry->ptr;

      /* Add one to the alias count for the last store 
       * to this address.
       */
      load_info->alias_info =
	S_insert_alias_info (load_info->alias_info, source_pc, 1,
			     &num_load_alias);
    }
}

void
S_create_store_alias (int sink_pc, S_MemDep_Data_Info * data_info)
{
  S_hash_node *store_entry;
  S_MemDep_Access_Info *store_info;
  int i;

  /* See if there is an entry in the hash table for this op */
  store_entry = S_hash_find_store_info (sink_pc);

  /* No entry, allocate a new one */
  if (!store_entry)
    {
      store_entry = S_hash_insert_store_info (sink_pc);
      if (aliases_in_file > S_max_file_aliases)
	S_consolidate_aliases_file ();
      store_info = (S_MemDep_Access_Info *)
	L_alloc (S_MemDep_Access_Info_pool);
      store_info->alias_info = NULL;
      store_info->pc = sink_pc;

      if (data_info->last_store_pc)
	{
	  /* Set the alias info to show one alias with the last
	   * store to this address.
	   */
	  store_info->alias_info = S_insert_alias_info (NULL,
							data_info->
							last_store_pc, 1,
							&num_store_alias);
	}
      for (i = 0; i < data_info->num_loads; i++)
	{
	  /* Set the alias info to show one alias with the last
	   * load to this address.
	   */
	  store_info->alias_info =
	    S_insert_alias_info (store_info->alias_info,
				 data_info->last_load_pc[i], 1,
				 &num_store_alias);
	}
      store_entry->ptr = (S_MemDep_Access_Info *) store_info;
    }

  /* Was an entry for this store op, update alias info */
  else
    {
      store_info = (S_MemDep_Access_Info *) store_entry->ptr;

      if (data_info->last_store_pc)
	{
	  /* Add one to the alias count for the last store 
	   * to this address.
	   */
	  store_info->alias_info =
	    S_insert_alias_info (store_info->alias_info,
				 data_info->last_store_pc, 1,
				 &num_store_alias);
	}
      for (i = 0; i < data_info->num_loads; i++)
	{
	  /* Add one to the alias count for the last load 
	   * to this address.
	   */
	  store_info->alias_info =
	    S_insert_alias_info (store_info->alias_info,
				 data_info->last_load_pc[i], 1,
				 &num_store_alias);
	}
    }

}
