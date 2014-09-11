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
 *  File:  s_main.c
 *
 *  Description:  Initializes and invokes lcode+ simulation
 *
 *  Creation Date :  July, 1993
 *
 *  Author:  John C. Gyllenhaal, Roger A. Bringmann
 *
 *  Revisions:
 *
 *      (C) Copyright 1993, John Gyllenhaal and Wen-mei Hwu
 *      All rights granted to University of Illinois Board of Regents.
\*****************************************************************************/

#include <config.h>

#ifndef lint
#define lint
static char copyright[] =
  "@(#) Copyright (c) 1993 John Gyllenhaal, Wen-mei Hwu and The Board of \n\
 Trustees of the University of Illinois. All rights reserved.\n";
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef WIN32
#include <stdio.h>
#include <io.h>
#include "windows.h"
#endif

#include "s_main.h"
#include "s_object.h"

#ifdef WIN32
/* NT apps cannot see mounted directories, sh must be in the path -ART 1/00 */
char *command_shell_path = "sh";
#else
char *command_shell_path = "/bin/sh";	/* Changed from csh to sh -JCG 8/15/98 */
#endif

char *S_parm_file = NULL;

/* Parameters read from LSIM_DEFAULTS */
char *S_mode_name = "Simulator";
char *S_processor_model_name = "Superscalar";
char *S_processor_type_name = "Static";
int S_simulation_with_profile_information = 0;
int S_region_stats = 0;
char *S_source_file = NULL;
int S_program_start_addr = 256;
char *S_trace_file = NULL;
char *S_addr_file = NULL;
char *S_histogram_file_name = NULL;

/* HCH 10-20-99 */
char *S_sload_file_name = NULL;
int S_gen_sload_data = 0;

int S_print_branch_histograms = 0;
int S_use_file_mode = 1;
int S_read_addr_file = 1;
char *S_exec_name = NULL;
char *S_trace_command = NULL;
int S_timeout_delay = 30;
char *S_mdes_file = NULL;
char *S_opc_info_file = NULL;
int S_print_opc_info = 0;
char *S_stats_file_name = "stdout";
char *S_profile_stats_file_name = "stdout";

char *S_debug_output_file = "stdout";
int S_dump_code_image = 0;
char *S_sample_model_name = "Uniform";
int S_trace_promoted_predicates = 0;
unsigned S_sample_size = BILLION;
unsigned S_skip_size = 0;
unsigned S_initialization_skip_size = 0;
unsigned S_max_sample_count = BILLION;
int S_use_random_seed = 1;
long S_seed = 0;
unsigned S_stop_sim_trip_count = 0;
int S_debug_stop_sim_markers = 0;
int S_debug_force_sim_markers = 0;
int S_use_skipped_memory_addresses = 1;
int S_print_sample_stats = 0;
int S_memory_latency_scale_factor = 1;
int S_memory_latency_delta_factor = 0;
int S_move_latency_scale_factor = 1;
int S_move_latency_delta_factor = 0;
int S_ialu_latency_scale_factor = 1;
int S_ialu_latency_delta_factor = 0;
int S_falu_latency_scale_factor = 1;
int S_falu_latency_delta_factor = 0;
int S_default_latency_scale_factor = 1;
int S_default_latency_delta_factor = 0;
char *S_fetch_model_name = "aggressive";
int S_fetch_width = 1;
int S_fetch_buf_size = 2;
int S_fetch_mark = 1;
int S_issue_width = 1;
int S_flush_pipe_on_untraced_jsr = 1;
int S_read_dests_of_pred_op = 1;
int S_branches_per_cycle = 1;
int S_dcache_ports = 1;
int S_retire_width = 1;

/* SCM 7/21/00 */
/* Removed global BTB parms, as they are now included in the BTB type */

int S_memory_latency = 50;
int S_memory_page_size = 4 * 1024;
char *S_bus_model_name = "single";
char *S_L2_bus_model_name = "single";
int S_debug_bus = 0;
int S_debug_L2_bus = 0;
int S_bus_bandwidth = 8;
int S_L2_bus_bandwidth = 32;
int S_streaming_support = 0;
int S_L2_streaming_support = 0;
char *S_icache_model_name = "split-block";
int S_icache_size = 64;
int S_icache_block_size = 8;
int S_icache_assoc = 2;
char *S_dcache_model_name = "blocking";
int S_dcache_size = 64;
int S_dcache_block_size = 8;
int S_dcache_assoc = 2;
int S_dcache_measure_conflict_stats = 0;
int S_dcache_write_buf_size = 8;
int S_dcache_combining_write_buf = 1;
int S_dcache_write_allocate = 0;
int S_dcache_miss_bypass_limit = 0;
int S_dcache_debug_misses = 0;
int S_dcache_prefetch_buf_size = 8;
int S_dcache_debug_prefetch = 0;
int S_dcache_ignore_prefetch_bit = 0;
int S_dcache_ignore_prefetches = 0;
int S_dcache_debug_mem_copy = 0;
int S_mem_copy_version = 1;	/* Version 1, 2, and 3 now defined */

int S_prefetch_cache = 0;
int S_pcache_size = 256 * 32;
int S_pcache_block_size = 256;
int S_pcache_assoc = 0;

int S_secondary_cache = 0;
int S_scache_latency = 4;
char *S_scache_model_name = "blocking";
int S_scache_size = 512 * 1024;
int S_scache_block_size = 256;
int S_scache_assoc = 1;
int S_scache_measure_conflict_stats = 0;
int S_scache_write_buf_size = 8;
int S_scache_combining_write_buf = 1;
int S_scache_write_allocate = 0;
int S_scache_miss_bypass_limit = 0;
int S_scache_debug_misses = 0;
int S_scache_prefetch_buf_size = 8;
int S_scache_debug_prefetch = 0;
int S_scache_ignore_prefetch_bit = 0;
int S_scache_ignore_prefetches = 0;

char *S_MCB_model_name = "No-MCB";
int S_MCB_size = 64;
int S_MCB_assoc = 8;
int S_MCB_checksum_width = 0;
int S_MCB_all_loads_preloads = 0;
int S_MCB_debug_load_load_conflicts = 0;
int S_MCB_debug_load_store_conflicts = 0;
int S_pcache_read_block_latency = 6;
int S_pcache_streaming_benefit = 1;

char *S_ALAT_model_name = "No-ALAT";
int S_ALAT_size = 64;
int S_ALAT_all_loads_preloads = 0;
int S_ALAT_debug_load_load_conflicts = 0;
int S_ALAT_debug_load_store_conflicts = 0;

int S_x86_use_pipe = 0;
char *S_x86_trace_binmap_file = "default.binmap";
char *S_x86_trace_output_file = "default.x86_trace";
char *S_x86_trace_desc = "(No description available)";

int S_nice_value = 0;		/* For auto renicing of simulations */

 /* Parameters determined from the trace header */
unsigned int S_trace_flags = 0x00000000;
int S_trace_pred_defs = 1;
int S_trace_byte_order_reversed = 0;
int S_use_func_ids_not_addrs = 0;

/* Global variables */
int S_mode = SIMULATOR;
int S_processor_model = PROCESSOR_MODEL_SUPERSCALAR;
int S_processor_type = PROCESSOR_TYPE_STATIC;
int S_sample_model = SAMPLE_MODEL_UNIFORM;
Stats *S_program_stats = NULL;
Stats *S_head_stats = NULL;
Stats *S_tail_stats = NULL;
int S_sched_info_avail = -1;
int S_program_start_pc = -1;
int S_trace_fd = -1;
int S_trace_command_pid = -1;
int S_trace_words_read = 0;
int S_function_count = 0;
int S_operation_count = 0;
int S_operation_count_cond = 0;
int S_operation_count_uncond = 0;
int S_operation_count_pred_uncond = 0;
int S_operation_count_pred_ret = 0;
int S_operation_count_pred_call = 0;
int S_operation_count_call = 0;
int S_operation_count_ret = 0;
S_Fn *S_entry_fn = NULL;
Pnode *S_pnode = NULL;

/* SCM 7/21/00 */
/* Added temporary BTB pointer to read the BTB parms into */
BTB *S_temp_btb = NULL;

int S_end_of_program = 0;
int S_normal_termination = 0;
unsigned S_sim_cycle = 0;
unsigned S_dynamic_cb_id = 0;
int S_force_sim = 0;
int S_bus_model = BUS_MODEL_SINGLE;
int S_L2_bus_model = BUS_MODEL_SINGLE;
int S_fetch_model = FETCH_MODEL_AGGRESSIVE;
int S_icache_model = ICACHE_MODEL_SPLIT_BLOCK;
int S_dcache_model = DCACHE_MODEL_BLOCKING;
int S_scache_model = DCACHE_MODEL_BLOCKING;
int S_MCB_model = MCB_MODEL_NO_MCB;
int S_ALAT_model = ALAT_MODEL_NO_ALAT;
int S_icache_miss_latency = 10;
int S_dcache_read_block_latency = 6;
int S_dcache_write_block_latency = 2;
int S_dcache_write_thru_latency = 2;
int S_dcache_streaming_benefit = 1;
int S_start_nice_value = 0;
int S_end_nice_value = 0;
int S_has_mem_copy_directives = 0;
int S_has_prefetches = 0;
int S_ops_between_branches = 0;
int S_ops_between_mispredictions = 0;
int S_cycle_of_last_branch = 0;
int S_cycle_of_last_misprediction = 0;
int S_stop_sim_markers_encountered = 0;

FILE *S_x86_trace_out = NULL;

/* MCM/JWS-Make sure benchmark's output is fully written by waiting
   until it exits before Lsim is allowed to exit. */
int child_pid;

/* Cache of buffer pages for OS support of non-trapping loads */
Scache *S_buffer_page_cache = NULL;

/*
 * Global simulation stats
 * 
 * These stats are kept global (as well as regionally) so that they 
 * can be used to guide sampling and for debugging purposes.
 *
 * Most of the stats are keep regionally in the S_program_stats structure.
 */
unsigned S_num_sim_on_path = 0;
unsigned S_num_sim_samples = 0;
unsigned S_num_skip_on_path = 0;
unsigned S_billions_skipped = 0;
unsigned S_num_packets_sim_on_path = 0;
unsigned S_num_packets_skip_on_path = 0;
unsigned S_billion_packets_skipped = 0;


time_t S_start_time = 0;
time_t S_init_time = 0;
time_t S_end_time = 0;
time_t S_skip_time = 0;

/* File to use for debug output */
FILE *debug_out;

/* Histogram output file */
FILE *S_histogram_file;

/* HCH 10-20-99 */
FILE *S_sload_file;
unsigned int S_max_specid = 0;

int S_trace_objects = 0;
char *latest_fn_name = NULL;
/* 
 * HCH: Use as a check: if S_trace_loop_id is DEFAULT_LOOP_ID,
 *      then need to trace all loops 
 */
int S_trace_loop_id = DEFAULT_LOOP_ID;  

/* Alloc pools */
L_Alloc_Pool *S_Operand_pool = NULL;
L_Alloc_Pool *Squeue_pool = NULL;
L_Alloc_Pool *Sq_entry_pool = NULL;
L_Alloc_Pool *Sdep_pool = NULL;
L_Alloc_Pool *Sint_pool = NULL;
L_Alloc_Pool *Pnode_pool = NULL;
L_Alloc_Pool *Trace_Manager_pool = NULL;
L_Alloc_Pool *Trace_Block_pool = NULL;
L_Alloc_Pool *Scache_pool = NULL;
L_Alloc_Pool *Icache_pool = NULL;
L_Alloc_Pool *Stats_pool = NULL;
L_Alloc_Pool *Region_Stats_pool = NULL;
L_Alloc_Pool *BTB_Stats_pool = NULL;
L_Alloc_Pool *MCB_Stats_pool = NULL;
L_Alloc_Pool *ALAT_Stats_pool = NULL;
L_Alloc_Pool *Icache_Stats_pool = NULL;
L_Alloc_Pool *Histogram_Stats_pool = NULL;

char *program_name;

void
S_init_alloc_pools ()
{
  S_Operand_pool = L_create_alloc_pool ("S_Operand", sizeof (S_Operand), 128);
  Squeue_pool = L_create_alloc_pool ("Squeue", sizeof (Squeue), 16);
  Sq_entry_pool = L_create_alloc_pool ("Sq_entry", sizeof (Sq_entry), 128);
  Sdep_pool = L_create_alloc_pool ("Sdep", sizeof (Sdep), 128);
  Sint_pool = L_create_alloc_pool ("Sint", sizeof (Sint), 128);
  Pnode_pool = L_create_alloc_pool ("Pnode", sizeof (Pnode), 1);
  Trace_Manager_pool = L_create_alloc_pool ("Trace_Manager",
					    sizeof (Trace_Manager), 1);
  Trace_Block_pool = L_create_alloc_pool ("Trace_Block",
					  sizeof (Trace_Block), 1);
  Scache_pool = L_create_alloc_pool ("Scache", sizeof (Scache), 1);
  Icache_pool = L_create_alloc_pool ("Icache", sizeof (Icache), 1);
  Stats_pool = L_create_alloc_pool ("Stats", sizeof (Stats), 1);
  Region_Stats_pool = L_create_alloc_pool ("Region_Stats",
					   sizeof (Region_Stats), 1);
  BTB_Stats_pool = L_create_alloc_pool ("BTB_Stats", sizeof (BTB_Stats), 1);
  MCB_Stats_pool = L_create_alloc_pool ("MCB_Stats", sizeof (MCB_Stats), 1);
  ALAT_Stats_pool = L_create_alloc_pool ("ALAT_Stats",
					 sizeof (ALAT_Stats), 1);
  Icache_Stats_pool = L_create_alloc_pool ("Icache_Stats",
					   sizeof (Icache_Stats), 1);
  Histogram_Stats_pool = L_create_alloc_pool ("Histogram_Stats",
					      sizeof (Histogram_Stats), 1);
}

Pnode *
S_create_pnode (id)
     int id;
{
  Pnode *pnode;
  char name_buf[100];

  pnode = (Pnode *) L_alloc (Pnode_pool);
  sprintf (name_buf, "Pnode %i", id);
  pnode->name = strdup (name_buf);
  pnode->id = id;

  pnode->trace_manager = S_create_trace_manager (pnode);
  pnode->processor_v = S_create_processor (pnode);
  pnode->icache = S_create_icache (pnode);
  pnode->dcache = S_create_dcache (pnode);
  pnode->scache = S_create_L2 (pnode);
  /* SCM 7/21/00 */
  /* S_create_BTB is no longer actually creating the BTB, but rather */
  /* modifying S_temp_btb and then returning it */
  pnode->btb = S_create_BTB (pnode);
  pnode->mcb = S_create_MCB (pnode);
  pnode->alat = S_create_ALAT (pnode);
  pnode->stats = S_program_stats;

  return (pnode);
}

static void
indent_usage (len)
     int len;
{
  int i;
  for (i = 0; i < len; i++)
    fprintf (stderr, " ");
}

void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  S_punt ("L_gen_code should never be called in the simulator");
}

void
print_usage ()
{
  int len;

  len = strlen (program_name) + 8;
  fprintf (stderr, "\n");
  fprintf (stderr,
	   "Usage: %3s [-p parameter_file] [-Dmacro_name=macro_value]\n\n",
	   program_name);
  indent_usage (len);
  fprintf (stderr,
	   "The environment variable SIM_STD_PARMS_FILE specifies the\n");
  indent_usage (len);
  fprintf (stderr, "parameter_file to use by default.\n");

  exit (1);
}

/* 
 * Read in a system parameter
 */
void
S_read_parm_system (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "punt_on_unknown_parm", &L_punt_on_unknown_parm);
  L_read_parm_b (ppi, "warn_parm_not_defined", &L_warn_parm_not_defined);
  L_read_parm_b (ppi, "?warn_dev_parm_not_defined",
		 &L_warn_dev_parm_not_defined);
  L_read_parm_b (ppi, "warn_parm_defined_twice", &L_warn_parm_defined_twice);
  L_read_parm_b (ppi, "warn_parm_not_used", &L_warn_parm_not_used);
  L_read_parm_b (ppi, "dump_parms", &L_dump_parms);
  L_read_parm_s (ppi, "parm_warn_file_name", &L_parm_warn_file_name);
  L_read_parm_s (ppi, "parm_dump_file_name", &L_parm_dump_file_name);


  L_read_parm_s (ppi, "mode", &S_mode_name);
  L_read_parm_b (ppi, "simulation_with_profile_information",
		 &S_simulation_with_profile_information);
  L_read_parm_i (ppi, "nice_value", &S_nice_value);
  L_read_parm_b (ppi, "bypass_alloc_routines", &bypass_alloc_routines);
  L_read_parm_s (ppi, "source_file", &S_source_file);
  L_read_parm_i (ppi, "program_start_addr", &S_program_start_addr);
  L_read_parm_s (ppi, "trace_file", &S_trace_file);
  L_read_parm_s (ppi, "addr_file", &S_addr_file);
  L_read_parm_b (ppi, "use_file_mode", &S_use_file_mode);
  L_read_parm_b (ppi, "read_addr_file", &S_read_addr_file);
  L_read_parm_s (ppi, "exec_name", &S_exec_name);
  L_read_parm_s (ppi, "trace_command", &S_trace_command);
  L_read_parm_i (ppi, "timeout_delay", &S_timeout_delay);
  L_read_parm_s (ppi, "mdes_file", &S_mdes_file);
  L_read_parm_s (ppi, "opc_info_file", &S_opc_info_file);
  L_read_parm_b (ppi, "print_opc_info", &S_print_opc_info);
  L_read_parm_b (ppi, "trace_promoted_predicates",
		 &S_trace_promoted_predicates);
  L_read_parm_s (ppi, "stats_file", &S_stats_file_name);
  L_read_parm_s (ppi, "profile_stats_file", &S_profile_stats_file_name);
  L_read_parm_s (ppi, "debug_output_file", &S_debug_output_file);
  L_read_parm_s (ppi, "sample_model", &S_sample_model_name);
  L_read_parm_i (ppi, "sample_size", (int *) &S_sample_size);
  L_read_parm_i (ppi, "skip_size", (int *) &S_skip_size);
  L_read_parm_i (ppi, "initialization_skip_size",
		 (int *) &S_initialization_skip_size);
  L_read_parm_i (ppi, "max_sample_count", (int *) &S_max_sample_count);
  L_read_parm_b (ppi, "use_random_seed", &S_use_random_seed);
  L_read_parm_i (ppi, "seed", (int *) &S_seed);
  L_read_parm_i (ppi, "stop_sim_trip_count", (int *) &S_stop_sim_trip_count);
  L_read_parm_b (ppi, "debug_stop_sim_markers", &S_debug_stop_sim_markers);
  L_read_parm_b (ppi, "debug_force_sim_markers", &S_debug_force_sim_markers);
  L_read_parm_b (ppi, "use_skipped_memory_addresses",
		 &S_use_skipped_memory_addresses);
  L_read_parm_b (ppi, "print_sample_stats", &S_print_sample_stats);
  L_read_parm_b (ppi, "dump_code_image", &S_dump_code_image);
  L_read_parm_i (ppi, "memory_latency_scale_factor",
		 &S_memory_latency_scale_factor);
  L_read_parm_i (ppi, "memory_latency_delta_factor",
		 &S_memory_latency_delta_factor);
  L_read_parm_i (ppi, "move_latency_scale_factor",
		 &S_move_latency_scale_factor);
  L_read_parm_i (ppi, "move_latency_delta_factor",
		 &S_move_latency_delta_factor);
  L_read_parm_i (ppi, "ialu_latency_scale_factor",
		 &S_ialu_latency_scale_factor);
  L_read_parm_i (ppi, "ialu_latency_delta_factor",
		 &S_ialu_latency_delta_factor);
  L_read_parm_i (ppi, "falu_latency_scale_factor",
		 &S_falu_latency_scale_factor);
  L_read_parm_i (ppi, "falu_latency_delta_factor",
		 &S_falu_latency_delta_factor);
  L_read_parm_i (ppi, "default_latency_scale_factor",
		 &S_default_latency_scale_factor);
  L_read_parm_i (ppi, "default_latency_delta_factor",
		 &S_default_latency_delta_factor);
  L_read_parm_b (ppi, "region_stats", &S_region_stats);
  L_read_parm_s (ppi, "histogram_file", &S_histogram_file_name);
  L_read_parm_b (ppi, "print_branch_histograms", &S_print_branch_histograms);

  /* JWS/HCH 10-15-99: Trace load speculation effectiveness using
   * SPECID / checks.
   */
  L_read_parm_s (ppi, "sload_data_file", &S_sload_file_name);
  L_read_parm_b (ppi, "gen_sload_data", &S_gen_sload_data);
  L_read_parm_i (ppi, "max_specid", (int *) &S_max_specid);

  L_read_parm_b (ppi, "trace_objects",  &S_trace_objects);
  L_read_parm_i (ppi, "?trace_loop_id", (int *) &S_trace_loop_id);
}


/*
 * Reads in parameters used by the simulator
 */
void
L_read_parm_Lsim (Parm_Parse_Info * ppi)
{
  /* Read all the section's parameters */
  S_read_parm_system (ppi);
  S_read_parm_processor (ppi);
  S_read_parm_BTB (ppi);
  S_read_parm_MCB (ppi);
  S_read_parm_ALAT (ppi);
  S_read_parm_mem (ppi);
  S_read_parm_bus (ppi);
  S_read_parm_L2 (ppi);
  S_read_parm_L2_bus (ppi);
  S_read_parm_icache (ppi);
  S_read_parm_dcache (ppi);
  S_read_parm_x86_trace (ppi);
  S_read_parm_profile (ppi);
}

/*
 * creates (if necessary) and opens a fifo.
 * Error checking done (exits on all errors)
 */
int
open_fifo (path, flags, mode)
     char *path;
     int flags;
     int mode;
{
  int fifo_fd;

  /* Make fifo, ok if already exists */
  if ((mknod (path, S_IFIFO | 0666, 0) < 0) && (errno != EEXIST))
    S_punt ("Cannot create fifo %s, unix error no %i\n", path, errno);

  /* Open it in the mode requested */
  if ((fifo_fd = open (path, flags, mode)) < 0)
    S_punt ("Unable to open fifo %s, flags %i, mode %i\n", path, flags, mode);

  return (fifo_fd);
}

/* Close trace file/fifo.  Remove trace fifo if used */
void
S_close_trace_fd ()
{
#ifdef WIN32
  CloseHandle (S_trace_fd);
#else
  /* Close trace pipe if created */
  if (S_trace_fd != -1)
    {
      close (S_trace_fd);
    }
#endif
}

void
S_trace_time_out ()
{
  fprintf (stderr, "Error: Simulation timed out.\n\n");
  fprintf (stderr,
	   "%s timed out after %i seconds while trying to create a trace pipe between\n",
	   program_name, S_timeout_delay);
  fprintf (stderr,
	   "the simulation and the instrumented program in the following trace command:\n");
  fprintf (stderr, "'%s'\n\n", S_trace_command);
  fprintf (stderr, "Possible causes:\n");
  fprintf (stderr, " 1) The trace command failed.\n");
  fprintf (stderr,
	   " 2) The trace command does not contain an instrumented program.\n");
  fprintf (stderr,
	   " 3) The instrumented program takes a long time to start.\n");
  fprintf (stderr,
	   "    (If this case, try increasing the parameter 'timeout_delay')\n");

  fprintf (stderr, "\n");

  S_punt ("Terminating simulation due to TIMEOUT.\n");
}

int
read_trace (int trace_fd, void *buf, int read_size)
{
  int try_count;
  int read_result;
#ifdef WIN32
  char *msg_buf;
#endif

  /* 
   * Sometimes this first read doesn't work.  Try looping a few
   * times to see if it will work later.
   */
  try_count = 0;
  errno = 0;

  while (1)
    {
#ifdef WIN32
      if (ReadFile (trace_fd, buf, read_size, &read_result, NULL))
	break;

      /* Treat broken pipe as end of file reached */
      if (GetLastError () == ERROR_BROKEN_PIPE)
	{
	  read_result = 0;
	  break;
	}
      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
		     NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1, NULL);

      if (try_count < 10)
	S_punt ("read_trace: Could not read trace file: %s", msg_buf);
      else
	fprintf (stderr, "read_trace: Could not read trace file: %s",
		 msg_buf);
      LocalFree (msg_buf);
#else
      /* Usually the read works, so break out of loop */
      if ((read_result = read (trace_fd, buf, read_size)) == read_size)
	break;
      /* If get "try again" result on the read, try again for a while */
      if ((read_result == -1) && (errno == EAGAIN) && (try_count < 10))
	{
	  fprintf (stderr, "Trace pipe busy... Trying again.\n");
	}

      /* Give up after getting "try again" too many times */
      else if ((read_result == -1) && (errno == EAGAIN))
	{
	  fprintf (stderr, "Trace pipe busy... Giving up.\n");
	  fprintf (stderr, "\n");
	  fprintf (stderr,
		   "-> Usually rerunning the '%s' will fix the problem.\n",
		   S_mode_name);
	  fprintf (stderr, "\n");
	  fprintf (stderr,
		   "Please email all the pipe busy messages to John Gyllenhaal.\n");
	  S_punt ("Cannot continue.  Error during first read of trace.");
	}

      /* Otherwise punt if don't understand error message */
      else
	{
	  fprintf (stderr,
		   "Please report the following to John Gyllenhaal:\n");
	  fprintf (stderr,
		   "Read returned %i and errno is set to %i.\n",
		   read_result, errno);

	  if (errno == EAGAIN)
	    fprintf (stderr,
		     "!!! This indicates read should be tried again.\n");

	  S_punt ("Error during first read of trace.");
	}
#endif
      try_count++;
    }

  return (read_result);
}

int TF_flag_errors = 0;

/* Macro wrapper for test_TF_flag  */
#define TEST_TF_FLAG(flag_ptr, flag, cond) \
    test_TF_flag (flag_ptr, #flag , flag, cond)

/* Helper function to check that all the trace flags are set how
 * desired.  Don't call directly, use TEST_TF_FLAG() macro to call.
 * Will print out error message and increment TF_flag_errors on errors.
 *
 * cond indicates the required bit setting and must be 0 or 1! -ITI/JCG 4/99
 */
void
test_TF_flag (unsigned int *flag_ptr, char *flag_string,
	      unsigned int flag_mask, int cond)
{
  int flag_set;

  /* Make sure flag_mask not 0 */
  if (flag_mask == 0)
    {
      S_punt ("test_TF_flag: flag_mask may not be 0!");
    }

  /* Determine if the flag is set */
  flag_set = (*flag_ptr & flag_mask) != 0;

  /* Determine if flag set differently than required */
  if (cond != flag_set)
    {
      /* Increment TF flag error count */
      TF_flag_errors++;

      /* Print out error message for trace flag */
      if (cond == 0)
	{
	  fprintf (stderr,
		   "Lsim doesn't expect trace flag '%s' to be set!\n",
		   flag_string);
	}
      else if (cond == 1)
	{
	  fprintf (stderr,
		   "Lsim expects trace flag '%s' to be set!\n", flag_string);
	}
      /* Sanity check */
      else
	{
	  S_punt ("test_TF_flag: invalid cond '%i'\n", cond);
	}
    }

  /* Clear flag in passed flags */
  *flag_ptr &= ~flag_mask;
}

/* Opens trace file or executes trace command and opens trace fifo to it */
void
S_init_trace_fd ()
{
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int read_size, read_result, try_count, i;
#endif
  int trace[4];
  int tracewd;
  char fifo_name[40], link_name[40];
  int tty;
  unsigned int untested_flags;
#ifdef WIN32
  char cmd_line[512];
  char *msg_buf;
  char event_name[40];
  HANDLE pipe_event;
  BOOL result;
  STARTUPINFO si;		/* for CreateProcess call */
  PROCESS_INFORMATION pi;	/* for CreateProcess call */
  DWORD ret_val;
  OFSTRUCT reopen_buf;
#endif

/* NT does not have setsid, fork, SIGALRM or mknod -ART 1/00 */
#ifdef WIN32
  /* Get fd for trace.  May be file or fifo. */
  if (S_use_file_mode)
    {
      /*
       * Get trace from file
       * Do not use std C "open" because we need a WIN32 Handle
       */
      S_trace_fd = OpenFile (S_trace_file,	// file name
			     &reopen_buf,	// file information
			     OF_READ	// action and attributes
	);

      if (S_trace_fd == HFILE_ERROR)
	S_punt ("Could not open trace file '%s'.", S_trace_file);
    }
  else
    {

      /* This is my version of system that uses csh instead of sh.
       * (Switched back to sh -JCG 8/15/98 )
       * It is a modification of the system() code in
       * 'THE UNIX PROGRAMMING ENVIRONMENT' by Kernighan and Pike,
       *  1984, page 224.
       */

      /*
       * Best to flush outputs before forking (otherwise can get
       * duplicates of messages.
       */
      fflush (stdout);

      /* Create unique trace pipe name */
      strcpy (link_name, "traceXXXXXX");

      if (_mktemp (link_name) == NULL)
	S_punt ("Error creating unique trace name");

      /* Trace pipes ALWAYS begin with //./pipe/ */
      strcpy (fifo_name, "//./pipe/");
      strcat (fifo_name, link_name);

      /* An event name is needed for process synchronization */
      strcpy (event_name, "impact_event_");
      strcat (event_name, link_name);

      SetEnvironmentVariable ("IMPACT_TEMP_TRACE_PIPE", link_name);

      S_trace_fd = CreateNamedPipe (fifo_name,	// pointer to pipe name
				    PIPE_ACCESS_INBOUND,	// pipe open mode
				    PIPE_TYPE_BYTE,	// pipe-specific modes, defaults to blocking mode
				    1,	// maximum number of instances
				    4000,	// output buffer size, in bytes
				    4000,	// input buffer size, in bytes
				    30000,	// time-out time, in milliseconds
				    NULL	// pointer to security attributes
	);

      if (S_trace_fd == INVALID_HANDLE_VALUE)
	{
	  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
			 NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1,
			 NULL);

	  S_punt ("Error creating named pipe %s: %s", fifo_name, msg_buf);
	  LocalFree (msg_buf);	// never called
	}

      pipe_event = CreateEvent (NULL,	// pointer to security attributes
				FALSE,	// flag for manual-reset event
				FALSE,	// flag for initial state
				event_name	// pointer to event-object name
	);

      if (pipe_event == NULL)
	{
	  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
			 NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1,
			 NULL);

	  S_punt ("Error creating event %s: %s", event_name, msg_buf);
	  LocalFree (msg_buf);	// never called
	}

      /* Initialize process structures to default */
      memset (&si, 0, sizeof (STARTUPINFO));
      si.cb = sizeof (STARTUPINFO);
      memset (&pi, 0, sizeof (PROCESS_INFORMATION));

      /* Create the command line */
      strcpy (cmd_line, command_shell_path);
      strcat (cmd_line, " -c \"");
      strcat (cmd_line, S_trace_command);
      strcat (cmd_line, "\"");

      result = CreateProcess (NULL,	// pointer to name of executable module (extracted from cmd_line)
			      cmd_line,	// pointer to command line string
			      NULL,	// process security attributes
			      NULL,	// thread security attributes
			      0,	// handle inheritance flag
			      0,	// creation flags
			      NULL,	// pointer to new environment block
			      NULL,	// pointer to current directory name
			      &si,	// pointer to STARTUPINFO
			      &pi	// pointer to PROCESS_INFORMATION
	);

      if (!result)
	{
	  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
			 NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1,
			 NULL);

	  S_punt ("Error creating process: %s", msg_buf);
	  LocalFree (msg_buf);	// never called
	}

      child_pid = pi.dwProcessId;

      ret_val = WaitForSingleObject (pipe_event, S_timeout_delay * 1000);
      if (ret_val != WAIT_OBJECT_0)
	{
	  if (ret_val == WAIT_TIMEOUT)
	    S_trace_time_out ();
	  if (ret_val == WAIT_FAILED)
	    {
	      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
			     NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1,
			     NULL);

	      S_punt ("init_trace: Error waiting for event: %s", msg_buf);
	      LocalFree (msg_buf);	// never called
	    }
	  S_punt ("init_trace: Error waiting for event");
	}
    }
#else
  /* Get fd for trace.  May be file or fifo. */
  if (S_use_file_mode)
    {
      /* Get trace from file */
      if ((S_trace_fd = open (S_trace_file, 0, 0)) < 0)
	S_punt ("Could not open trace file '%s'.", S_trace_file);
    }
  else
    {

      /* This is my version of system that uses csh instead of sh.
       * (Switched back to sh -JCG 8/15/98 )
       * It is a modification of the system() code in
       * 'THE UNIX PROGRAMMING ENVIRONMENT' by Kernighan and Pike,
       *  1984, page 224.
       */

      /*
       * Best to flush outputs before forking (otherwise can get
       * duplicates of messages).
       */
      fflush (stdout);

      /* Open a terminal for sh (so doesn't complain) */
      tty = open ("/dev/tty", O_RDWR);

      /* 
       * If cannot open terminal, Ie logged out, redirect everything
       * to /dev/null (will lose all stdout and stderr)
       */
      if (tty == -1)
	{
	  fprintf (stderr,
		   "Cannot open /dev/tty redirecting Sim messages to /dev/null\n");
	  tty = open ("/dev/null", O_RDWR);
	}

      /* If still haven't opened something, punt */
      if (tty == -1)
	{
	  S_punt
	    ("Spawning probed executable: could not open /dev/tty or /dev/null.");
	}

      /*
       * Make the simulation the process group leader so
       * that the probed program will create a trace pipe using
       * the pid of the simulation
       */
      setsid ();		/* SAM 4-95 was setpgrp (0, 0); */
      /*
       * Spawn child to execute trace_command and get trace thru fifo.
       */
      if ((child_pid = fork ()) == 0)
	{
	  /*
	   * Make stdin, stdout, and stderr for child be the tty we
	   * just opened.
	   */
	  close (0);
	  dup (tty);
	  close (1);
	  dup (tty);
	  close (2);
	  dup (tty);
	  close (tty);

	  /* Child process.  Make sh and execute trace command in it. */
	  execlp (command_shell_path, command_shell_path, "-c",
		  S_trace_command, (char *) 0);

	  /* Detect failure of execlp, should never get here */
	  S_punt ("execlp failed executing: %s -c %s",
		  command_shell_path, S_trace_command);
	}

      /* Parent doesn't need tty */
      close (tty);

      /* Name of pipe is created using simultions pid */
      S_trace_command_pid = getpid ();

      /* Now trace pipe is always placed in /tmp/trace.pipe.%i */
      sprintf (fifo_name, "/tmp/trace.pipe.%i", S_trace_command_pid);
      sprintf (link_name, "trace.pipe.%i", S_trace_command_pid);

      /* In order to prevent hanging forever, set an alarm to wake us
       * up if we don't read something from S_trace_fd soon.
       */
      signal (SIGALRM, S_trace_time_out);

      /* Give the probed program S_timeout_delay seconds to generate some 
       * trace info before punting.
       */
      alarm (S_timeout_delay);

      /* Make fifo, ok if already exists */
      if ((mknod (fifo_name, S_IFIFO | 0666, 0) < 0) && (errno != EEXIST))
	{
	  S_punt ("Cannot create fifo %s, unix error no %i\n",
		  fifo_name, errno);
	}

      /* Create link to the name buf in current directory -JCG 10/18/96 
       * Ok if already exists since probed executable is also trying
       * to make link
       */
      if ((symlink (fifo_name, link_name) == -1) && (errno != EEXIST))
	{
	  S_punt ("Unable to link '%s' to '%s'\n", link_name, fifo_name);
	}

      /* Open link in the mode requested */
      if ((S_trace_fd = open (link_name, 0, 0)) < 0)
	{
	  S_punt ("Unable to open fifo %s, flags %i, mode %i\n",
		  link_name, 0, 0);
	}

      /* Happy now, disable timeout */
      alarm (0);

      /* Remove trace.pipe and link (neither are necessary at this point)
       * ignore errors since probed executable is also trying to clean up.
       */
      unlink (link_name);
      unlink (fifo_name);
    }				/* !S_use_file_mode */
#endif /* !WIN32 */

  read_trace (S_trace_fd, (void *) &tracewd, sizeof (int));

  /* The first word should be L_TRACE_START_FORMAT3 (or FORMAT[1|2] 
   * for backward compatibility) or the reverse of its bytes 
   */
  if ((tracewd != L_TRACE_START_FORMAT3) &&
      (tracewd != L_TRACE_START_FORMAT2) &&
      (tracewd != L_TRACE_START_FORMAT1))
    {
      if ((SWAP_BYTES (trace[0]) == L_TRACE_START_FORMAT3) ||
	  (SWAP_BYTES (trace[0]) == L_TRACE_START_FORMAT2) ||
	  (SWAP_BYTES (trace[0]) == L_TRACE_START_FORMAT1))
	{
	  S_trace_byte_order_reversed = 1;
	  fprintf (stderr, "Reversing trace byte order.\n");
	}
      else
	S_punt ("Trace corrupt: does not begin with "
		"L_TRACE_START_FORMAT[123]");
    }

  /* Set parameters to make compatibile with trace format */
  switch (tracewd)
    {
    case L_TRACE_START_FORMAT3:
      /* New format to allow trace flags to be passed -ITT (JCG) 1/99 */

      read_trace (S_trace_fd, (void *) &tracewd, sizeof (int));
      if (S_trace_byte_order_reversed)
	tracewd = SWAP_BYTES (tracewd);

      S_trace_flags = tracewd;

      /* Detect supported flags & make sure we know what every flag means! */
      untested_flags = S_trace_flags;

      /* Determine if predicate defs are traced */
      if (S_trace_flags & TF_TRACE_PRED_DEFS)
	S_trace_pred_defs = 1;
      else
	S_trace_pred_defs = 0;
      untested_flags &= ~TF_TRACE_PRED_DEFS;

      /* Determine if promoted predicate defs are traced.
       * Override parameter's value (which is only for FORMAT2)
       */
      if (S_trace_flags & TF_TRACE_PROMOTED_PREDS)
	S_trace_promoted_predicates = 1;
      else
	S_trace_promoted_predicates = 0;
      untested_flags &= ~TF_TRACE_PROMOTED_PREDS;

      /* Determine if function ids (Lemulate) or address are in the trace */
      if (S_trace_flags & TF_FUNC_IDS)
	S_use_func_ids_not_addrs = 1;
      else
	S_use_func_ids_not_addrs = 0;
      untested_flags &= ~TF_FUNC_IDS;

      /* Make sure everything is turned on that Lsim expects in
       * the trace (other than optional flags above).
       */
      TEST_TF_FLAG (&untested_flags, TF_PROBE_FOR_SIMULATION, 1);
      TEST_TF_FLAG (&untested_flags, TF_PREDICATE_PROBE_CODE, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_CONTROL_FLOW, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_MEM_ADDRS, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_MASKED_LOAD_FAULTS, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_PRED_USES, 1);

      /* Make sure everything Lsim doesn't expect in the trace is 
       * turned off 
       */
      TEST_TF_FLAG (&untested_flags, TF_PROBE_FOR_PROFILING, 0);
      TEST_TF_FLAG (&untested_flags, TF_PROBE_FOR_CUSTOM, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_EMPTY_CBS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_JUMP_RG_SRC1, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_PRED_JUMP_FALL_THRU, 0);

      /* Make sure extra trace information is not present that Lsim doesn't
       * currently support is not present.  (Lsim might be enhanced in 
       * the future to allow some of these parameter settings to be used.)
       */
      TEST_TF_FLAG (&untested_flags, TF_TRACE_EXTRA_HEADERS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_OP_IDS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_ENHANCED_OP_IDS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_DEST_REG_VALUES, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_SRC_REG_VALUES, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_SRC_LIT_VALUES, 0);

      if (untested_flags != 0)
	{
	  fprintf (stderr,
		   "Error: Unknown trace flags (0x%08x) present!\n",
		   untested_flags);
	  fprintf (stderr,
		   "         See impact/include/Lcode/l_trace_interface.h "
		   "for details!\n");
	  TF_flag_errors++;
	}

      /* Punt if have trace flag errors */
      if (TF_flag_errors != 0)
	{
	  S_punt ("Lsim exiting due to above unexpected trace flag "
		  "settings!");
	}
      break;

    case L_TRACE_START_FORMAT2:
      /* Predicate defs are in the trace */
      S_trace_pred_defs = 1;

      /* Function addresses are in the trace */
      S_use_func_ids_not_addrs = 0;
      break;

    case L_TRACE_START_FORMAT1:
      /* Predicate defs are not in the trace */
      S_trace_pred_defs = 0;
      break;

    default:
      S_punt ("Unknown trace format: %i\n", trace[0]);
    }

  read_trace (S_trace_fd, (void *) &tracewd, sizeof (int));
  if (S_trace_byte_order_reversed)
    tracewd = SWAP_BYTES (tracewd);

  /* L_TRACE_SAMPLE_START must be next */
  if (tracewd != L_TRACE_SAMPLE_START)
    S_punt ("Trace corrupt: L_TRACE_SAMPLE_START expected 2nd.");

  /* Now that we know the trace format, load the encoded file. */
  S_load_code (S_source_file);
}



void
S_start_first_fn_trace (Pnode * pnode)
{
  int tracewd;

  tracewd = S_get_trace_word (pnode);

  /* L_TRACE_FN must be next */
  if (tracewd != L_TRACE_FN)
    {
      /* Try to figure out what is going on */
      if (tracewd == L_TRACE_SAMPLE_END)
	{
	  S_punt ("Trace error:\nEmpty trace: All executed functions "
		  "were probed for file swapping!\n"
		  "At least one executed function must be probed for "
		  "simulation!\n             ^^^^^^^^");
	}

      S_punt ("Trace corrupt: L_TRACE_FN expected after trace configuration; "
	      "received '%i'.", tracewd);
    }

  /*
   * Get what the trace thinks is the main function.
   */

  tracewd = S_get_trace_word (pnode);

  S_entry_fn = S_get_fn (tracewd);

  /*
   * If could not find main addr and use_file_mode == yes, then
   * it is likely that addr_list is out of date.
   */

  if (S_entry_fn == NULL)
    {
      fprintf (stderr, "Trace's main() mem address (%d) incorrect.\n",
	       tracewd);
      if (S_use_file_mode)
	{
	  fprintf (stderr,
		   "Make sure fn addresses in '%s' are up to date.\n",
		   S_addr_file);
	}
      S_punt ("Cannot continue: trace incorrect");
    }

  latest_fn_name = S_entry_fn->name;

}

void
S_print_configuration_system (FILE * out)
{
  char time_buf[100];

  /* Print out start time */
  if (S_start_time != -1)
    {
      strftime (time_buf, sizeof (time_buf),
		"%H:%M:%S %p, %A %B %d, %Y", localtime (&S_start_time));

      fprintf (out, "%s started: %s.\n", S_mode_name, time_buf);

    }
  else
    fprintf (out, "%s started: time unavailable.\n", S_mode_name);

  fprintf (out, "\n");
  fprintf (out, "# SYSTEM CONFIGURATION:\n");
  fprintf (out, "\n");
  fprintf (out, "Parameter file:  %s\n", S_parm_file);
  fprintf (out, "Mdes file:       %s\n", S_mdes_file);
  fprintf (out, "Encoded source:  %s\n", S_source_file);
  fprintf (out, "opc info file:   %s\n", S_opc_info_file);
  fprintf (out, "Stats file:      %s\n", S_stats_file_name);
  fprintf (out, "nice value:      %i\n", S_nice_value);
  fprintf (out, "\n");

  if (S_print_branch_histograms)
    {
      fprintf (out, "# Will print branch histograms to histogram file\n");
      fprintf (out, "Histogram file:  %s\n", S_histogram_file_name);
      fprintf (out, "\n");
    }

  /* 
   * HCH 10-20-99: If taken/not taken information is to be collected, 
   *  print out file name 
   */
  if (S_gen_sload_data)
    {
      fprintf (out, "# Will print spec load data to sload file\n");
      fprintf (out, "Sload data file:  %s\n", S_sload_file_name);
      fprintf (out, "\n");
    }

  fprintf (out, "# TRACE CONFIGURATION:\n");
  fprintf (out, "\n");
  if (S_use_file_mode == 1)
    {
      fprintf (out, "# Reading trace information from file.\n");
      fprintf (out, "Trace file:      %s\n", S_trace_file);
      fprintf (out, "Addr file:       %s\n", S_addr_file);
    }
  else
    {
      fprintf (out, "# Consuming trace on the fly:\n");
      fprintf (out, "Exec name:       %s\n", S_exec_name);
      fprintf (out, "trace command:   %s\n", S_trace_command);
      fprintf (out, "timeout delay:   %i\n", S_timeout_delay);
    }
  fprintf (out, "\n");

  fprintf (out, "# SAMPLING CONFIGURATION:\n");
  fprintf (out, "\n");
  fprintf (out, "%12s sample model.\n", S_sample_model_name);
  fprintf (out, "%12u sample size.\n", S_sample_size);
  fprintf (out, "%12u skip size.\n", S_skip_size);
  fprintf (out, "%12u initialization skip size.\n",
	   S_initialization_skip_size);
  fprintf (out, "%12u max sample count.\n", S_max_sample_count);

  /* Only print out random seed stuff if using random sampling -JCG 6/99 */
  if (S_sample_model == SAMPLE_MODEL_RANDOM_SKIP)
    {
      if (S_use_random_seed)
	fprintf (out, "%12s", "Random");
      else
	fprintf (out, "%12s", "Specified");
      fprintf (out, " seed for psuedo-random number generator used.\n");
      fprintf (out,
	       "%12lu used as seed (currently used only for random sampling).\n",
	       S_seed);
    }

  if (S_stop_sim_trip_count > 0)
    {
      fprintf (out, "%12u stop sim trip count.\n", S_stop_sim_trip_count);
      if (S_debug_stop_sim_markers)
	fprintf (out, "%12s ", "Do");
      else
	fprintf (out, "%12s ", "Do not");
      fprintf (out, "debug stop sim markers.\n");
    }

  if (S_use_skipped_memory_addresses)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "use skipped memory addresses.\n");
  fprintf (out, "\n");

  if (S_use_func_ids_not_addrs)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out, "use function ids instead of addresses.\n");

  if (S_trace_pred_defs)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out,
	   "trace predicate value at definition (set by trace format).\n");
  fprintf (out, "\n");


  if (S_trace_promoted_predicates)
    fprintf (out, "%12s ", "Do");
  else
    fprintf (out, "%12s ", "Do not");
  fprintf (out,
	   "trace promoted predicate (pred[1]) values (set by trace format).\n");
  fprintf (out, "\n");

  fprintf (out, "# PROGRAM CONFIGURATION:\n");
  fprintf (out, "\n");
  fprintf (out, "%12u functions in program.\n", S_function_count);
  fprintf (out, "%12u operations in program.\n", S_operation_count);
  fprintf (out, "%12u scheduled operation packets in program.\n",
	   S_max_packet_id);
  fprintf (out, "%12u static total branch operations.\n",
	   S_operation_count_cond + S_operation_count_pred_uncond +
	   S_operation_count_pred_call + S_operation_count_pred_ret +
	   S_operation_count_uncond + S_operation_count_call +
	   S_operation_count_ret);
  fprintf (out, "%12u static total predicated branch operations.\n",
	   S_operation_count_pred_uncond + S_operation_count_pred_call +
	   S_operation_count_pred_ret);
  fprintf (out, "%12u static predicated jmp operations.\n",
	   S_operation_count_pred_uncond);
  fprintf (out, "%12u static predicated call operations.\n",
	   S_operation_count_pred_call);
  fprintf (out, "%12u static predicated return branch operations.\n",
	   S_operation_count_pred_ret);
  fprintf (out, "%12u static conditional branch operations.\n",
	   S_operation_count_cond);
  fprintf (out, "%12u static jmp operations.\n", S_operation_count_uncond);
  fprintf (out, "%12u static call operations.\n", S_operation_count_call);
  fprintf (out, "%12u static return operations.\n", S_operation_count_ret);


  if (S_max_slot < 0)
    {
      fprintf (out, "%12s max scheduled slot in program.\n", "(no info)");
    }
  else
    {
      fprintf (out, "%12i max scheduled slot in program.\n", S_max_slot);
    }
  fprintf (out, "%12i program start address.\n", S_program_start_addr);
  fprintf (out, "\n");


  if (S_mode == SIMULATOR)
    {
      fprintf (out, "%12i memory latency scale factor.\n",
	       S_memory_latency_scale_factor);
      fprintf (out, "%12i memory latency delta factor.\n",
	       S_memory_latency_delta_factor);
      fprintf (out, "%12i move latency scale factor.\n",
	       S_move_latency_scale_factor);
      fprintf (out, "%12i move latency delta factor.\n",
	       S_move_latency_delta_factor);
      fprintf (out, "%12i ialu latency scale factor.\n",
	       S_ialu_latency_scale_factor);
      fprintf (out, "%12i ialu latency delta factor.\n",
	       S_ialu_latency_delta_factor);
      fprintf (out, "%12i falu latency scale factor.\n",
	       S_falu_latency_scale_factor);
      fprintf (out, "%12i falu latency delta factor.\n",
	       S_falu_latency_delta_factor);
      fprintf (out, "%12i default latency scale factor.\n",
	       S_default_latency_scale_factor);
      fprintf (out, "%12i default latency delta factor.\n",
	       S_default_latency_delta_factor);
      fprintf (out, "\n");
    }

  if (S_print_opc_info)
    S_print_opc_info_tab (out);
}

void
S_print_configuration (FILE * out)
{
  S_print_configuration_system (out);
  S_print_configuration_processor (out);
  S_print_configuration_BTB (out);
  S_print_configuration_MCB (out);
  S_print_configuration_ALAT (out);
  S_print_configuration_memory (out);
  S_print_configuration_bus (out);
  S_print_configuration_L2 (out);
  S_print_configuration_L2_bus (out);
  S_print_configuration_icache (out);
  S_print_configuration_dcache (out);

  if (S_prefetch_cache)
    S_print_configuration_pcache (out);

  fprintf (out, "# END CONFIGURATION\n");
  fprintf (out, "\n");

  fflush (out);
}


int main (argc, argv, envp)
     int argc;
     char **argv;
     char **envp;
{
  Parm_Macro_List *external_list;
  FILE *stats_file;
  FILE *profile_stats_file = NULL;

  int pc;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  Scache *cache;
  Scblock *block1, *block2, *block3;
  int i;
#endif
  time_t skip_start_time, skip_end_time;
  unsigned long sample_skip_size = 0;
  int x86_trace_fd;

  /* Get the start time for the simulation */
  S_start_time = time (NULL);

  program_name = argv[0];

  /* Get macro definitions from command line and environment */
  external_list = L_create_external_macro_list (argv, envp);

  /*
   * Get std_parm name from command line (-p path), or environment
   * variable "STD_PARMS_FILE", or default to "./STD_PARMS"
   */
  S_parm_file = L_get_std_parm_name (argv, envp, "STD_PARMS_FILE",
				     "./STD_PARMS");

  /* allocate S_temp_BTB here */
  S_temp_btb = (BTB *) malloc (sizeof (BTB));

  L_load_parameters (S_parm_file, external_list, "(Lsim", L_read_parm_Lsim);
  
  /* Print out warning about unused command line parameter settings */
  L_warn_about_unused_macros (stderr, external_list);

  /* Warn if we are bypassing alloc routines */
  if (bypass_alloc_routines)
    fprintf (stderr, "Bypassing alloc routines.\n");

  /* Auto renice process to S_nice_value >0 and nice value is
   * not already higher than S_nice_value.  Max nice value is 19.
   */
  if (S_nice_value > 19)
    S_nice_value = 19;

  if ((S_nice_value > 0) && (S_nice_value > getpriority (PRIO_PROCESS, 0)))
    {
      if (setpriority (PRIO_PROCESS, 0, S_nice_value) != 0)
	{
	  fprintf (stderr, "Unable to renice process to %i.\n", S_nice_value);
	}
    }

  /* Get the starting nice value */
  S_start_nice_value = getpriority (PRIO_PROCESS, 0);

  /* Get the mode of opertion */
  if (L_pmatch (S_mode_name, "Simulator"))
    {
      /* Reassign name to beautify output */
      S_mode_name = "Simulator";
      S_mode = SIMULATOR;
    }
  else if (L_pmatch (S_mode_name, "Profiler"))
    {
      /* Reassign name to beautify output */
      S_mode_name = "Profiler";
      S_mode = PROFILER;
    }
  else if (L_pmatch (S_mode_name, "X86_trace_generator"))
    {
      /* Reassign name to beautify output */
      S_mode_name = "X86_trace_generator";
      S_mode = X86_TRACE_GENERATOR;
    }
  else
    {
      S_punt
	("Invalid operation mode '%s':  Use simulator, profiler or x86_trace_generator.",
	 S_mode_name);
    }

  /* Get sampling model */
  if (L_pmatch (S_sample_model_name, "Uniform"))
    {
      /* Reassign name to beautify output */
      S_sample_model_name = "Uniform";
      S_sample_model = SAMPLE_MODEL_UNIFORM;
    }
  else if (L_pmatch (S_sample_model_name, "Random-skip"))
    {
      /* Reassign name to beautify output */
      S_sample_model_name = "Random-skip";
      S_sample_model = SAMPLE_MODEL_RANDOM_SKIP;
    }
  else
    {
      S_punt ("Invalid sampling model '%s': Use Uniform or Random-skip.",
	      S_sample_model_name);
    }

  /* Use time to intialize random seed */
  if (S_use_random_seed)
    {
      S_seed = time (NULL);
    }

  /* Initialize random number generator used for random sampling */
  srand48 (S_seed);

  /* Churn through a couple of random numbers to make well initialized.
   * Probably not necessary but makes me feel better :)
   */
  lrand48 ();
  lrand48 ();

  /* Set the processor model (uses S_processor_model_name).
   * Must be done before loading code (since it creates stats then)
   *
   * For now, must be valid processor model even if not doing simulation.
   * It makes the stats creation algorithm easier.
   */
  S_set_processor_model ();

  /* Initialize all simulation allocation pools, do after loading
   * parameters so can set the bypass_alloc_routines flag
   */
  S_init_alloc_pools ();

  /* Limit skip size and sample size to 1 billion.
   * We can handle many billions of instructions skipped but
   * don't want the 32-bit counter to overflow during the
   * skip
   */
  if (S_skip_size > BILLION)
    S_punt ("skip_size (%u) may not be > 1000000000", S_skip_size);

  if (S_initialization_skip_size > BILLION)
    S_punt ("S_initialization_skip_size (%u) may not be > 1000000000",
	    S_initialization_skip_size);

  if (S_sample_size > BILLION)
    S_punt ("sample_size (%u) may not be > 1000000000", S_sample_size);

  if (S_max_sample_count > BILLION)
    S_punt ("max_sample_count (%u) may not be > 1000000000",
	    S_max_sample_count);

  /*
   * This would cause infinite loop and very large output file
   * (a million 0 instruction samples)
   */
  if ((S_sample_size == 0) && (S_skip_size == 0))
    S_punt ("skip_size and sample_size may not both be 0");

  /* Load opc info file, call before S_init_trace_fd or S_load_code */
  S_load_opc_info (S_opc_info_file);

  /* Don't currently use mdes, so don't load it -JCG 10/14/96 */
#if 0
  L_init_lmdes (S_mdes_file);
#endif

  if (S_source_file == NULL)
    S_punt ("Source_file (encoded lcode) must be specified.");

  /* Open debug output, if stdout or stderr, route directly */
  if (strcmp (S_debug_output_file, "stdout") == 0)
    debug_out = stdout;
  else if (strcmp (S_debug_output_file, "stderr") == 0)
    debug_out = stderr;
  else if ((debug_out = fopen (S_debug_output_file, "w")) == NULL)
    S_punt ("Cannot open debug output file '%s'.", S_debug_output_file);

  /* If printing a histogram, open the histogram file for output */
  if (S_print_branch_histograms)
    {
      if (strcmp (S_histogram_file_name, "stdout") == 0)
	S_histogram_file = stdout;
      else if (strcmp (S_histogram_file_name, "stderr") == 0)
	S_histogram_file = stderr;
      else if ((S_histogram_file = fopen (S_histogram_file_name, "w")) ==
	       NULL)
	S_punt ("Cannot open histogram output file '%s'.",
		S_histogram_file_name);
    }

  if (S_gen_sload_data)
    {
      if ((S_sload_file = fopen (S_sload_file_name, "w")) == NULL)
	S_punt ("Cannot open sload output file '%s'.", S_sload_file_name);
    }

  /* 
   * Moved S_load_code into S_init_trace_fd on 10/14/96 so 
   * the trace format can determine how the code is loaded 
   * (namely how predicate defs are handled/initialized) -JCG
   */

  /* Useful for debugging code anotation problems */
  if (S_dump_code_image)
    {
      fprintf (stderr, "Dumping code image to '%s' (debug_output_file).\n",
	       S_debug_output_file);

      /* Typically code loaded after opening trace file,
       * so load encoded lcode source file now so can dump
       */
      S_load_code (S_source_file);

      /* Dump code to see attributes for everything */
      S_print_code (debug_out);

      /* Do not simulate if dumping code image */
      S_punt ("Image dump complete.  Exiting before simulation.");
    }

  /* Open trace file/fifo, loads encoded file based on trace format */
  S_init_trace_fd ();

  if (S_trace_objects)
    S_init_object_trace ();

  /* Load anything necessary for the mode of operation we are in */
  switch (S_mode)
    {
    case SIMULATOR:
      /* Initialize memory system */
      S_init_bus ();
      S_init_L2_bus ();
      S_init_memory ();
      S_init_buffer_page_support ();

      /* Also allow simulator to gather profile information */
      if (S_simulation_with_profile_information)
	S_init_profiler ();
      break;


    case PROFILER:
      S_init_profiler ();
      break;


    case X86_TRACE_GENERATOR:
      /* If doing x86_trace, load binmap file */
      S_load_binmap (S_x86_trace_binmap_file);

      /* Open x86_trace output file, allow it to go to stdout */
      if (strcmp (S_x86_trace_output_file, "stdout") == 0)
	S_x86_trace_out = stdout;

      else if (S_x86_use_pipe)
	{
	  x86_trace_fd = open_fifo (S_x86_trace_output_file, O_WRONLY, 0666);
	  if ((S_x86_trace_out = fdopen (x86_trace_fd, "w")) == NULL)
	    S_punt ("Cannot open x86 trace pipe '%s'.",
		    S_x86_trace_output_file);

	}
      else if ((S_x86_trace_out = fopen (S_x86_trace_output_file, "w")) ==
	       NULL)
	S_punt ("Cannot open x86 trace output file '%s'.",
		S_x86_trace_output_file);
      break;

    default:
      S_punt ("Initialization not specified for mode '%s'", S_mode_name);
    }

  /* Create processing node */
  S_pnode = S_create_pnode (1);

  if (S_trace_objects)
    S_read_obj_trace (S_pnode);

  S_start_first_fn_trace (S_pnode);

  /* Start at entry function (usually main) of probed program */
  pc = S_entry_fn->op[0].pc;

  /* Open stats file */
  if (strcmp (S_stats_file_name, "stdout") == 0)
    stats_file = stdout;
  else if ((stats_file = fopen (S_stats_file_name, "w")) == NULL)
    S_punt ("Unable to open stats file '%s'.\n", S_stats_file_name);

  /* Print where stats are going */
  fprintf (stdout, "%s output file: %s\n", S_mode_name, S_stats_file_name);
  fflush (stdout);

  /* 
   * Initialization occurs at declaration for stat and flag variables
   */
  switch (S_mode)
    {
    case SIMULATOR:
      /* Print out simulation configuration */
      S_print_configuration (stats_file);

      fprintf (stats_file, "# BEGIN SIMULATOR\n");
      fprintf (stats_file, "\n");
      /* Update region entry stats */
      S_pnode->stats->region->num_entries++;

      if (S_gen_sload_data)
	S_init_sload_data ();

      if (S_simulation_with_profile_information)
	{

	  /* Open stats file */

	  if (strcmp (S_profile_stats_file_name, "stdout") == 0)
	    profile_stats_file = stdout;
	  else
	    if ((profile_stats_file = fopen (S_profile_stats_file_name, "w"))
		== NULL)
	    S_punt ("Unable to open stats file '%s'.\n",
		    S_profile_stats_file_name);

	  /* Print where stats are going */
	  fprintf (stdout, "%s profile output file: %s\n", S_mode_name,
		   S_profile_stats_file_name);
	  fflush (stdout);

	  S_print_configuration_profiling (profile_stats_file);
	  fprintf (profile_stats_file, "# BEGIN PROFILER\n");
	  fprintf (profile_stats_file, "\n");
	}
      break;

    case PROFILER:
      S_print_configuration_profiling (stats_file);
      fprintf (stats_file, "# BEGIN PROFILER\n");
      fprintf (stats_file, "\n");
      break;

    case X86_TRACE_GENERATOR:
      /* Print out simulation configuration */
      S_print_configuration_x86_trace (stats_file);

      fprintf (stats_file, "# BEGIN X86_TRACE_GENERATOR\n");
      fprintf (stats_file, "\n");
      break;

    default:
      S_punt ("Start message not defined for mode '%s'.", S_mode_name);
    }

  /* Get the init time for the simulation */
  S_init_time = time (NULL);

  /* Initialization skip size for skipping over intialization phase of */
  /* the program being simulated.                                      */
  if (S_initialization_skip_size != 0)
    {
      /* Measure how long we are spending skipping instructions */
      skip_start_time = time (NULL);

      pc = S_skip (pc, S_initialization_skip_size);

      /* Measure how long we are spending skipping instructions */
      skip_end_time = time (NULL);
      S_skip_time += skip_end_time - skip_start_time;
    }

  /* Simulate/skip instructions until end of program reached */
  while ((!S_end_of_program) && (S_num_sim_samples < S_max_sample_count))
    {
      /* Update sample stats */
      S_num_sim_samples++;

      /* Get the skip size for this sample */
      if (S_sample_model == SAMPLE_MODEL_UNIFORM)
	{
	  sample_skip_size = S_skip_size;
	}
      else if (S_sample_model == SAMPLE_MODEL_RANDOM_SKIP)
	{
	  sample_skip_size = lrand48 () % ((2 * S_skip_size) + 1);
	}
      else
	{
	  S_punt ("Unknown sample model %i.", S_sample_model);
	}


      /* If desired, print what sample we are on to stats file */
      if (S_print_sample_stats)
	{
	  fprintf (stats_file,
		   "Sample %i: %12.0lf   to  ", S_num_sim_samples,
		   (double) ((double) S_num_sim_on_path +
			     (double) S_num_profiled +
			     (double) S_num_skip_on_path +
			     ((double) BILLION * S_billions_skipped)));
	  fflush (stats_file);
	  S_check_fn_head ();
#if 0
	  S_dump_activation_record ();
#endif
	}

      /* Do sample simulation, profiling, etc here */
      switch (S_mode)
	{
	case SIMULATOR:
	  pc = S_sim_processor (S_pnode, pc, S_sample_size);
	  break;

	case PROFILER:
	  pc = S_profile (pc, S_sample_size);
	  break;

	case X86_TRACE_GENERATOR:
	  pc = S_write_x86_trace (pc, S_sample_size);
	  break;

	default:
	  S_punt ("Action for operation mode '%s' not specified.",
		  S_mode_name);
	}

      if (S_print_sample_stats)
	{
	  fprintf (stats_file,
		   "%12.0lf   then skipping %lu\n",
		   (double) ((double) S_num_sim_on_path +
			     (double) S_num_profiled +
			     (double) S_num_skip_on_path +
			     ((double) BILLION * S_billions_skipped)),
		   sample_skip_size);

	  fflush (stats_file);
	}

      /* Measure how long we are spending skipping instructions */
      skip_start_time = time (NULL);

      pc = S_skip (pc, sample_skip_size);

      /* Measure how long we are spending skipping instructions */
      skip_end_time = time (NULL);
      S_skip_time += skip_end_time - skip_start_time;

    }

  /* Close trace file/fifo */
  S_close_trace_fd ();

  /* Get the ending nice value */
  S_end_nice_value = getpriority (PRIO_PROCESS, 0);

  /* If ended simulation early due to max_sample_count, print
   * out warning.
   */
  if (!S_end_of_program)
    {
      fprintf (stats_file,
	       "Max sample count of %u reached, ending simulation early!\n",
	       S_max_sample_count);

      fprintf (stdout,
	       "Warning: Max sample count of %u reached, ending simulation early!\n",
	       S_max_sample_count);

    }
  else if (!S_normal_termination)
    {
      fprintf (stats_file,
	       "Stop sim trip count of %i, ending simulation early!\n",
	       S_stop_sim_trip_count);
    }

  /* Print stats, etc */
  switch (S_mode)
    {
    case SIMULATOR:
      /* Update region entry stats */
      S_pnode->stats->region->num_exits++;

      fprintf (stats_file, "\n");
      fprintf (stats_file, "# END SIMULATOR\n");
      fprintf (stats_file, "\n");

      fprintf (stats_file, "# SIMULATOR RESULTS:\n");
      fprintf (stats_file, "\n");


      if (S_region_stats)
	S_print_stats_region (stats_file);
      else
	S_print_stats_global (stats_file);

      S_debug_tlb (debug_out);

      if (S_simulation_with_profile_information)
	{
	  fprintf (profile_stats_file, "\n");
	  fprintf (profile_stats_file, "# END PROFILER\n");
	  fprintf (profile_stats_file, "\n");
	  S_write_profile (profile_stats_file);
	}

      if (S_gen_sload_data)
	{
	  S_finish_sload_data ();
	}

      break;

    case PROFILER:
      fprintf (stats_file, "\n");
      fprintf (stats_file, "# END PROFILER\n");
      fprintf (stats_file, "\n");
      S_write_profile (stats_file);
      break;

    case X86_TRACE_GENERATOR:
      fprintf (stats_file, "\n");
      fprintf (stats_file, "# END X86_TRACE_GENERATOR\n");
      fprintf (stats_file, "\n");

      S_write_x86_end_of_trace ();

      fclose (S_x86_trace_out);
      if (S_x86_use_pipe)
	{
	  if (unlink (S_x86_trace_output_file) == -1)
	    fprintf (stderr, "Unable to remove '%s'\n",
		     S_x86_trace_output_file);
	}
      break;

    default:
      S_punt ("Stats routine not specified for '%s'.", S_mode_name);
    }

  fclose (stats_file);
  fclose (debug_out);
  if (S_gen_sload_data)
    fclose (S_sload_file);

  if (S_trace_objects)
    {
      if (S_trace_loop_id == DEFAULT_LOOP_ID)
	{
	  S_clear_stack ();
	  S_clear_pages ();
	  fprintf(stderr, "num_rd_collapses %i num_wt_collapses %i\n",
		  num_rd_collapses, num_wt_collapses);
	}
      S_print_object_report ();
      S_close_object_trace ();
    }

  /* MCM/JWS-Be sure the child shell exits before exiting Lsim
     in order to allow the shell to finish redirecting benchmark's
     output. */
  waitpid (child_pid, 0, 0);

  return (0);
}


void
S_punt (char *fmt, ...)
{
  va_list args;

  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");

  /* Clean up trace pipes before exiting.  These routines
   * should do nothing if the trace pipe is not opened
   */
  S_close_trace_fd ();

  if (S_x86_use_pipe && (S_x86_trace_out != NULL))
    {
      if (unlink (S_x86_trace_output_file) == -1)
	fprintf (stderr, "Unable to remove '%s'\n", S_x86_trace_output_file);
    }

  exit (1);
}
