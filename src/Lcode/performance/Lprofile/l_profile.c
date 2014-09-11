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
/*===========================================================================
 *      File :          l_profile.c
 *      Description :   Produces profile info file for an lcode program
 *                      
 *      Creation Date : April, 1993
 *      Author :        John Gyllenhaal, Wen-mei Hwu
 *
 *      Made compatible with Lemulate by IMPACT Technologies, Inc. (JCG) 1/99
 *
 *==========================================================================*/

#include <config.h>

#ifdef WIN32
#include <stdio.h>
#include <io.h>
#include "windows.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#if defined (_SOLARIS_SOURCE)
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#ifdef _HPUX_SOURCE
#include <nlist.h>
#endif
#include <signal.h>

#include "l_profile.h"
#include <library/l_parms.h>
#include <library/heap.h>

#undef DEBUG_MEM_ADDR
#undef DEBUG_STORE_KILL
#undef DEBUG_OBJ
#undef DEBUG_HEAP_OBJ
#define OBJS_PER_LINE 12

#define PERMS 0666

#ifdef WIN32
/* NT apps cannot see mounted sirectories, sh must be in the path -ART 1/00 */
char *command_shell_path = "sh";
#else
char *command_shell_path = "/bin/sh";	/* Changed from csh to sh -JCG 8/15/98 */
#endif

int ptrace_byte_order_reversed = 0;

int profile_BTB = 1;
int BTB_model = BTB_MODEL_COUNTER;
char *BTB_model_name = NULL;
char *mp_output_name = NULL;
char *S_parm_file = NULL;
int L_do_buf_info = 0;
char *L_buf_file_name = "NULL";
FILE *buf_file = NULL;

char *program_name;
Pfunc *func_list = NULL;
int func_count = 0;
Pfunc **func_hash_tab = NULL;

char *input_name = NULL;
char *output_name = NULL;
char *exec_name = NULL;
char *generate_using = NULL;

int timeout_delay = 30;
int use_func_ids_not_addrs = 0;
unsigned int trace_flags;

/* For HCH MICRO '04 */
int Lprofile_mem_addrs = 0;
int heap_index_full = 0;
int heap_index_max = 0;
int glob_index_full = 1;
int glob_index_max = 0;
int num_malloc = 0;

int static_glob_obj_info[MAX_GLOB_OBJS][2] = {{0}};
int stack_table[MAX_CALL_DEPTH][2] = {{0}};
int call_depth = 0;
int stack_loads = 0;
int stack_stores = 0;
int stack_ld_sites = 0;
int stack_st_sites = 0;

int unmarked_access = 0;
int missing_global = 0;
int missing_stack = 0;
int missing_heap = 0;
int free_nonvis_malloc = 0;
/* End of HCH MICRO '04 */

/* MCM/JWS-Make sure benchmark's output is fully written by waiting 
   until it exits before Lprofile is allowed to exit. */
static int child_pid;

static void print_usage (void);
static void L_read_parm_Lprofile (Parm_Parse_Info *ppi);

static void load_input (char *name);
static Pfunc *create_func (char *name, char *asm_name, int addr,
			   int size, int max_cb, int num_jsrs);

static void read_symbol_table (char *exec_name);
static void build_id_based_symbol_table (char *exec_name);

static Ptrace *init_trace (char *trace_source);
static void gather_profile_info (Ptrace * ptrace);
static int get_ptrace (Ptrace * ptrace);
static void Ptrace_reverse_byte_order (int *ptr, int *end);

static void print_output (char *name);

static Pfunc *find_func (int addr);
static char *strip (char *);
static void increment_cond_count (Pcontrol * pint, int cond);
static void print_condition_codes (FILE * out, Pcontrol * pint);

static void P_model_BTB (PBTB * btb_entry, int actual_direction, 
			 int actual_target);
static void print_BTB_output (char *name);

#undef DEBUG_LOOP_ITERS

int
main (int argc, char **argv, char **envp)
{
  Parm_Macro_List *external_list;
  Ptrace *ptrace;

  /* Get the program name */
  program_name = argv[0];

  if (argc < 5)
    print_usage ();

  /* Get required parameters */
  input_name = argv[1];
  output_name = argv[2];
  exec_name = argv[3];
  generate_using = argv[4];

  /* Get macro definitions from command line (after argv[4]) and env */
  external_list = L_create_external_macro_list (&argv[4], envp);

  /*
   * Get std_parm name from command line (-p path), or environment
   * variable "STD_PARMS_FILE", or default to "./STD_PARMS"
   */
  S_parm_file = L_get_std_parm_name (argv, envp, "STD_PARMS_FILE",
				     "./STD_PARMS");

  L_load_parameters (S_parm_file, external_list, "(Lprofile",
		     L_read_parm_Lprofile);

  if (L_do_buf_info)
    if ((buf_file = fopen (L_buf_file_name, "w")) == NULL)
      I_punt("Could not open output file %s.\n", L_buf_file_name);

  /* Print out warning about unused command line parameter settings */
  L_warn_about_unused_macros (stderr, external_list);

  /* Get misprediction profiling parameters */
  if (profile_BTB)
    {
      if (strcmp (BTB_model_name, "counter") == 0)
	BTB_model = BTB_MODEL_COUNTER;
      else if (strcmp (BTB_model_name, "2-level") == 0)
	BTB_model = BTB_MODEL_2_LEVEL;
      else
	I_punt("BTB_model '%s' not supported.\n", BTB_model_name);
    }
  load_input (input_name);

  /* Read first part of trace early so can get trace flags -ITI/JCG 1/99 */
  ptrace = init_trace (generate_using);

  if (use_func_ids_not_addrs)
    build_id_based_symbol_table (exec_name);
  else
    read_symbol_table (exec_name);

  gather_profile_info (ptrace);
  print_output (output_name);
  if (profile_BTB)
    print_BTB_output (mp_output_name);

  if (L_do_buf_info)
    fclose (buf_file);

  /* MCM/JWS-Be sure the child shell exits before exiting Lprofile
     in order to allow the shell to finish redirecting benchmark's 
     output. */
  waitpid (child_pid, 0, 0);

  return (0);
}

static void
print_usage (void)
{
  I_punt ("Usage:\n"
	  "%s src prof_dest exec_name 'profile command' "
	  "[parm file directives]\n\n"
	  "src               - lcode, encoded for profiling\n"
	  "prof_dest         - dest. of profile info\n"
	  "exec_name         - probed exec name "
	  "(symbol table read from it)\n"
	  "                    (ie: compress or wc)\n"
	  "'profile command' - quoted invocation "
	  "command for profiling\n"
	  "                    (ie: 'compress <in > out'"
	  " or 'wc cccp.c')\n", program_name);
}

/*
 * Read in Lprofile parameters
 */
static void
L_read_parm_Lprofile (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "profile_BTB", &profile_BTB);
  L_read_parm_s (ppi, "BTB_model", &BTB_model_name);
  L_read_parm_s (ppi, "mp_output_name", &mp_output_name);

  L_read_parm_b (ppi, "dump_parms", &L_dump_parms);
  L_read_parm_s (ppi, "parm_dump_file_name", &L_parm_dump_file_name);
  L_read_parm_b (ppi, "?do_buf_info", &L_do_buf_info);
  L_read_parm_s (ppi, "?buf_file_name", &L_buf_file_name);

  /* SER: For HCH MICRO '04 */
  L_read_parm_b (ppi, "profile_mem_addrs", &Lprofile_mem_addrs);
}

/* Returns hash addr that fits into table of FUNC_HASH_SIZE 
 */
static int
hash_func_addr (int addr)
{
  int hash_val;

  hash_val = (addr >> 2) & (FUNC_HASH_SIZE - 1);
  return (hash_val);
}

/* Generate function id's from order listed in .encoded file
 * and generate symbol table.  For use with new Lemulate tools. -ITI (JCG) 1/99
 */
static void
build_id_based_symbol_table (char *exec_name)
{
  Pfunc *func;
  int hash_val;
  int id;

  func = func_list;

  /* Use .encoded file order to assign sequential ids.
   * Since func_list in reverse order, start with max id first.
   */
  id = 1000 + func_count - 1;
  while (func != NULL)
    {
      /* Write address (really func #) to function */
      func->addr = id;
      id--;

      /* Add function to hash table based on addr */
      hash_val = hash_func_addr (func->addr);
      func->next_hash = func_hash_tab[hash_val];
      func_hash_tab[hash_val] = func;

      func = func->next_func;
    }
}

/*
 * Reads the symbol table, getting the address for all the
 * functions in the program.
 */
#ifndef _HPUX_SOURCE
/* The new Lemulate based framework does not need this functionality anymore.
 * Leave in only for backward compatibility on HPUX systems. -JCG 5/99
 */
static void
read_symbol_table (char *exec_name)
{
  I_punt ("read_symbol_table: "
	  "Should not be called (only supported on HPUX)!\n");
  return;
}
#else
static void
read_symbol_table (char *exec_name)
{
  int nlist_size, name_size;
  struct nlist *func_nlist;
  Pfunc *func;
  FILE *exec_in;
  int i, error_flag;
  int hash_val;


  nlist_size = (func_count + 2) * sizeof (struct nlist);
  if ((func_nlist = (struct nlist *) malloc (nlist_size)) == NULL)
    I_punt ("read_symbol_table: Out of memory");

  func = func_list;

  /* Build nlist entry for each function */
  for (i = 0; i < func_count; i++)
    {
      if (func == NULL)
	I_punt
	  ("read_symbol_table: func should not be NULL (i = %i func_count = %i\n",
	   i, func_count);

      func_nlist[i].n_name = strdup (func->asm_name);
      func = func->next_func;
    }

  /* func better be NULL here */
  if (func != NULL)
    I_punt ("read_symbol_table: func_count (%i) is off", func_count);

  /* Terminate nlist arrya with 0 for name */

  func_nlist[i].n_name = 0;

  /* Make sure can read executable */
  if ((exec_in = fopen (exec_name, "r")) == NULL)
    I_punt ("Error reading %s.", exec_name);

  /* Close, opened for error message */
  fclose (exec_in);

  /*
   * Call nlist to get all the addresses of the functions
   */
  if (nlist (exec_name, func_nlist) < 0)
    {
      fprintf (stderr,
	       "Error: nlist() failed.  Incompatable executable file %s.\n",
	       exec_name);
      I_punt ("Rerun on same machine arch type as '%s' was compiled on.\n",
	      exec_name);
    }

  error_flag = 0;

  func = func_list;
  /* Put the nlist info back into function */
  for (i = 0; i < func_count; i++)
    {
      if (func == NULL)
	I_punt ("read_symbol_table: func should not be NULL\n");

      /* Make sure name found */
      if (func_nlist[i].n_value == 0)
	{
	  fprintf (stderr, "Error: func %s not found in %s symbol table.\n",
		   func_nlist[i].n_name, exec_name);
	  error_flag == 1;
	}

      /* Write address to function */
      func->addr = func_nlist[i].n_value;

      /* Add function to hash table based on addr */
      hash_val = hash_func_addr (func->addr);
      func->next_hash = func_hash_tab[hash_val];
      func_hash_tab[hash_val] = func;

      /*
       * We cannot free name buf, since nlist() moves pointers around 
       */

      func = func->next_func;
    }

  if (error_flag)
    I_punt ("Error: Cannot run profile with unknown function addresses");

  /* Free nlist array, (cannot free name buffers, since nlist() moves ptrs) */
  free (func_nlist);
  return;
}
#endif


/* 
 * Finds the function corresponding to the address passed.
 * Returns NULL if not found
 */
static Pfunc *
find_func (int addr)
{
  int hash_val;
  Pfunc *func;

  /* Search linked list of hash functions for function addr */
  hash_val = hash_func_addr (addr);
  func = func_hash_tab[hash_val];
  while (func != NULL)
    {
      /* If found, return pointer to func */
      if (func->addr == addr)
	return (func);
      func = func->next_hash;
    }
  /* If no match, return NULL */
  return (NULL);
}

/* Prints out a trace word and its likely meaning */
static void
print_trace_word (int index, int word)
{
  Pfunc *func;

  fprintf (stderr, "%3i: 0x%08X (%15d)   ", index, word, word);

  switch (word)
    {
    case L_TRACE_PRED_CLR:
      fprintf (stderr, "(L_TRACE_PRED_CLR)");
      break;
    case L_TRACE_PRED_SET:
      fprintf (stderr, "(L_TRACE_PRED_SET)");
      break;
    case L_TRACE_PRED_UNDEF:
      fprintf (stderr, "(L_TRACE_PRED_UNDEF)");
      break;
    case L_TRACE_FN:
      fprintf (stderr, "(L_TRACE_FN)");
      break;
    case L_TRACE_STUB:
      fprintf (stderr, "(L_TRACE_STUB)");
      break;
    case L_TRACE_BRTHRU:
      fprintf (stderr, "(L_TRACE_BRTHRU)");
      break;
    case L_TRACE_START_FORMAT1:
      fprintf (stderr, "(L_TRACE_START_FORMAT1)");
      break;
    case L_TRACE_START_FORMAT2:
      fprintf (stderr, "(L_TRACE_START_FORMAT2)");
      break;
    case L_TRACE_START_FORMAT3:
      fprintf (stderr, "(L_TRACE_START_FORMAT3)");
      break;
    case L_TRACE_END:
      fprintf (stderr, "(L_TRACE_END)");
      break;
    case L_TRACE_WRITE:
      fprintf (stderr, "(L_TRACE_WRITE)");
      break;
    case L_TRACE_RTS:
      fprintf (stderr, "(L_TRACE_RTS)");
      break;
    case L_TRACE_SAMPLE_START:
      fprintf (stderr, "(L_TRACE_SAMPLE_START)");
      break;
    case L_TRACE_SAMPLE_END:
      fprintf (stderr, "(L_TRACE_SAMPLE_END)");
      break;
    default:
      /* Print out what it most likely is tracing */
      if ((func = find_func (word)) != NULL)
	{
	  fprintf (stderr, "(Function %s)", func->name);
	}
      else if (word < L_TRACE_JSR_OFFSET)
	{
	  fprintf (stderr, "(Return to jsr %i in function below)",
		   (L_TRACE_JSR_OFFSET - word));
	}
      else if (word < 0)
	{
	  fprintf (stderr, "(Cb %i)", -word);
	}
      break;
    }
  fprintf (stderr, "\n");
}

/*
 * Prints as much debug info (to stderr) as possible when the profiler
 * breaks
 */
static void
print_debug_info (Ptrace * ptrace, Pfunc * func, unsigned int pc)
{
  int i;
  int cb, count;
  int type, flags;
  int start, end, start_dist, end_dist;
  int *cur;

  /* Make sure no NULL pointers passed */
  if (ptrace == NULL)
    {
      fprintf (stderr, "print_debug_info: ptrace NULL\n");
      return;
    }

  if (func == NULL)
    {
      fprintf (stderr, "print_debug_info: func NULL\n");
      return;
    }

  /* Make sure pc legal */
  if (pc >= func->size)
    {
      fprintf (stderr, "print_debug_info: func %s, pc (%i) out of bounds\n",
	       func->name, pc);
      return;
    }

  cb = -1;
  count = 0;

  /* Get the type of the current instruction */
  type = func->control_tab[pc].type;
  flags = func->control_tab[pc].flags;

  /* Find which cb we are in */
  for (i = pc; i >= 0; i--)
    {
      /* Count the number of instr of this type in the cb */
      if (func->control_tab[i].type == type)
	count++;

      if (func->control_tab[i].type == CB)
	{
	  cb = func->control_tab[i].id;
	  break;
	}
    }

  fprintf (stderr, "Profile error in function %s, cb %i:\n", func->name, cb);
  fprintf (stderr, "At the %i", count);
  switch (count)
    {
    case 1:
      fprintf (stderr, "st ");
      break;
    case 2:
      fprintf (stderr, "nd ");
      break;
    case 3:
      fprintf (stderr, "rd ");
      break;
    default:
      fprintf (stderr, "th ");
    }

  switch (type)
    {
    case FN:
      fprintf (stderr, "FN");
      break;
    case CB:
      if (flags & LOOP_HEADER)
	fprintf (stderr, "CB_LOOP_HEADER");
      else
	fprintf (stderr, "CB");
      break;
    case BR:
      fprintf (stderr, "BR");
      break;
    case JMP:
      fprintf (stderr, "JMP");
      break;
    case PJMP:
      fprintf (stderr, "PJMP");
      break;
    case JSR:
      fprintf (stderr, "JSR");
      break;
    case RET:
      fprintf (stderr, "RET");
      break;
    case HASH:
      fprintf (stderr, "HASH");
      break;
    case LOAD:
      fprintf (stderr, "LOAD");
      break;
    case STORE:
      fprintf (stderr, "STORE");
      break;
    case LOAD_ALLOC:
      fprintf (stderr, "LOAD_ALLOC");
      break;
    case STORE_ALLOC:
      fprintf (stderr, "STORE_ALLOC");
      break;
#if 0
    case LOAD_STACK:
      fprintf (stderr, "LOAD_STACK");
      break;
    case STORE_STACK:
      fprintf (stderr, "STORE_STACK");
      break;
#endif
    case MALLOC:
      fprintf (stderr, "MALLOC");
      break;
    case FREE:
      fprintf (stderr, "FREE");
      break;
    default:
      fprintf (stderr, "(Unknown type %i)", type);
      break;
    }
  fprintf (stderr, " in this cb.\n");

  /* Find the distance to the start/end of the trace buffer */
  cur = ptrace->ptr - 1;
  start_dist = cur - ptrace->buf;
  end_dist = ptrace->end - cur;

  /* Try to start 20 entries above current point and go to 
   * 20 entries below current point.
   */
  start = -20;
  end = 20;

  /* Limit start/end to beginning/end of buffer */
  if (start < (-start_dist))
    start = -start_dist;

  if (end > end_dist)
    end = end_dist;


  fprintf (stderr, "Trace buffer contents (trace count %i):\n",
	   ptrace->trace_count);
  if (start == (-start_dist))
    fprintf (stderr, "(Start of buffer)\n");

  for (i = start; i <= end; i++)
    print_trace_word (i, cur[i]);

  if (end == end_dist)
    fprintf (stderr, "(End of buffer)\n");
  return;
}


static void
P_trace_time_out (int dummy)
{
  fprintf (stderr, "Error: Profiler timed out.\n\n");
  fprintf (stderr, "%s timed out after %i seconds while trying to create "
	   "a trace pipe between\n", program_name, timeout_delay);
  fprintf (stderr, "the profiler and the instrumented program in the "
	   "following trace command:\n");
  fprintf (stderr, "'%s'\n\n", generate_using);
  fprintf (stderr, "Possible causes:\n");
  fprintf (stderr, " 1) The trace command failed.\n");
  fprintf (stderr, " 2) The trace command does not contain an "
	   "instrumented program.\n");
  fprintf (stderr, " 3) The instrumented program takes a long time "
	   "to start.\n");
  fprintf (stderr, "    (If this case, try increasing the 'timeout_delay' "
	   "in the \n");
  fprintf (stderr, "     C source for %s and recompiling.)\n", program_name);
  fprintf (stderr, "\n");
  I_punt ("Terminating profiler due to TIMEOUT.\n");
  return;
}

/*
 * Loop iter statistics
 * ----------------------------------------------------------------------
 */

static Piter *
find_iter (Ploop_info * loop_info, int iterations)
{
  Piter *iter;
  int hash;

  /* ITER_HASH_SIZE must be power of 2 for this hash to work properly */
  hash = iterations & (ITER_HASH_SIZE - 1);

  for (iter = loop_info->iter_hash[hash]; iter != NULL; iter = iter->next)
    {
      /* Iteration found, return it */
      if (iter->iterations == iterations)
	return (iter);
    }

  /* Return NULL for not found */
  return (NULL);
}

static void
update_loop_iter_stats (Ploop_info * loop_info)
{
  Piter *iter;
  int hash;
  int iterations;

  /* Do nothing if first time updating stats */
  iterations = loop_info->cur_iter_count;
  if (iterations < 1)
    return;

  /* Find or create node of the current iteration value */
  iter = find_iter (loop_info, iterations);

  /* If doesn't exist, create it and insert into hash table */
  if (iter == NULL)
    {
      /* Malloc new structure */
      if ((iter = (Piter *) malloc (sizeof (Piter))) == NULL)
	I_punt ("Out of memory in update_loop_iter_stats");

      /* Initialize fields */
      iter->iterations = iterations;
      iter->weight = 1;

      /* Add to hash table */
      hash = iterations & (ITER_HASH_SIZE - 1);
      iter->next = loop_info->iter_hash[hash];
      loop_info->iter_hash[hash] = iter;

      /* Update entry count */
      loop_info->entry_count++;
    }
  /* Otherwise, increment weight */
  else
    {
      iter->weight++;
    }

  if (L_do_buf_info && loop_info->is_inner)
    {
      fprintf (buf_file, "%s %d %d\n",
	       loop_info->func->name, loop_info->cb->id,
	       loop_info->cur_inner_iter_count);
    }

#ifdef DEBUG_LOOP_ITERS
  fprintf (stderr,
	   "  Update func %s cb %i iterated %i times.\n",
	   loop_info->func->name,
	   loop_info->cb->id, loop_info->cur_iter_count);
#endif
}

static void
sort_iter_entries (Ploop_info * loop_info)
{
  int array_size, i, count;
  Piter **sorted_array, *iter_entry;
  Heap *heap;


  /* Do nothing if no entries */
  if (loop_info->entry_count < 1)
    return;

  /* Build array of pointers to the iteration entries */
  array_size = (loop_info->entry_count * sizeof (Piter *));
  if ((sorted_array = (Piter **) malloc (array_size)) == NULL)
    I_punt ("sort_iter_entries: out of memory.");

  /* Create heap for heap sort */
  heap = Heap_Create (HEAP_MIN);

  /* Insert each item onto heap */
  count = 0;
  for (i = 0; i < ITER_HASH_SIZE; i++)
    {
      for (iter_entry = loop_info->iter_hash[i]; iter_entry != NULL;
	   iter_entry = iter_entry->next)
	{
	  Heap_Insert (heap, iter_entry, (double) iter_entry->iterations);
	  count++;
	}
    }

  /* Sanity check, make sure we got the correct number of entries */
  if (count != loop_info->entry_count)
    I_punt ("sort_iter_entries: count (%i) != entry_count (%i)",
	    count, loop_info->entry_count);

  /* Use heap sort and put them into sorted array */
  for (i = 0; i < count; i++)
    sorted_array[i] = Heap_ExtractTop (heap);

  /* Heap should be empty at this point */
  if (Heap_ExtractTop (heap) != NULL)
    I_punt ("sort_iter_entries: expected heap to be empty");

  /* Free heap, should be empty so don't pass element_dispose routine */
  Heap_Dispose (heap, NULL);

  /* Point loop info at sorted array */
  loop_info->sorted_iters = sorted_array;
}

#ifdef DEBUG_LOOP_ITERS

static void
print_iter_info (FILE * out, Ploop_info * loop_info)
{
  Piter *iter;
  int i;

  fprintf (out, "  Loop at %s cb %i has %i entries:\n",
	   loop_info->func->name, loop_info->cb->id, loop_info->entry_count);

  for (i = 0; i < loop_info->entry_count; i++)
    {
      iter = loop_info->sorted_iters[i];
      fprintf (out, "    " ITintmaxformat " iterations " ITintmaxformat 
	       " times.\n", iter->iterations, iter->weight);
    }
}

#endif

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
static void
test_TF_flag (unsigned int *flag_ptr, char *flag_string,
	      unsigned int flag_mask, int cond)
{
  int flag_set;

  /* Make sure flag_mask not 0 */
  if (flag_mask == 0)
    {
      I_punt ("test_TF_flag: flag_mask may not be 0!");
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
	fprintf (stderr,
		 "Lprofile doesn't expect trace flag '%s' to be set!\n",
		 flag_string);
      else if (cond == 1)
	fprintf (stderr,
		 "Lprofile expects trace flag '%s' to be set!\n",
		 flag_string);
      /* Sanity check */
      else
	I_punt ("test_TF_flag: invalid cond '%i'\n", cond);
    }

  /* Clear flag in passed flags */
  *flag_ptr &= ~flag_mask;
}


/* 
 * Run the probed executable and combine trace info with the
 * encoded lcode to generate the trace info.
 */
static Ptrace *
init_trace (char *trace_source)
{
  Ptrace *ptrace;
  int ptrace_size, buf_size;
  int read_cnt;
  int trace_word;
  unsigned int untested_flags;
  char fifo_name[40], link_name[40];
#ifdef WIN32
  char cmd_line[512];
  char *msg_buf;
  char event_name[40];
  HANDLE pipe_event;
  BOOL result;
  STARTUPINFO si;		/* for CreateProcess call */
  PROCESS_INFORMATION pi;	/* for CreateProcess call */
  DWORD ret_val;
#endif

  ptrace_size = sizeof (Ptrace);
  buf_size = sizeof (int) * TRACE_SIZE;

  if (((ptrace = (Ptrace *) malloc (ptrace_size)) == NULL) ||
      ((ptrace->buf = (int *) malloc (buf_size)) == NULL))
    I_punt ("init_trace: Out of memory");

/* NT does not have setsid, fork, SIGALRM or mknod -ART 1/00 */
#ifdef WIN32
  /* Create unique trace pipe name */
  strcpy (link_name, "traceXXXXXX");

  if (_mktemp (link_name) == NULL)
    I_punt ("Error creating unique trace name");

  /* Trace pipes ALWAYS begin with //./pipe/ */
  strcpy (fifo_name, "//./pipe/");
  strcat (fifo_name, link_name);

  /* An event name is needed for process synchronization */
  strcpy (event_name, "impact_event_");
  strcat (event_name, link_name);

  SetEnvironmentVariable ("IMPACT_TEMP_TRACE_PIPE", link_name);

  ptrace->source_fd = CreateNamedPipe (fifo_name,	// pointer to pipe name
				       PIPE_ACCESS_INBOUND,	// pipe open mode
				       PIPE_TYPE_BYTE,	// pipe-specific modes, defaults to blocking mode
				       1,	// maximum number of instances
				       4000,	// output buffer size, in bytes
				       4000,	// input buffer size, in bytes
				       30000,	// time-out time, in milliseconds
				       NULL	// pointer to security attributes
    );

  if (ptrace->source_fd == INVALID_HANDLE_VALUE)
    {
      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
		     NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1, NULL);

      I_punt ("Error creating named pipe %s: %s", fifo_name, msg_buf);
      LocalFree (msg_buf);	// never called
    }

  pipe_event = CreateEvent (NULL,	// pointer to security attributes
			    FALSE,	// flag for manual-reset event
			    FALSE,	// flag for initial state
			    event_name	// pointer to event-object name
    );

  if (pipe_event == NULL)
    {
      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		     FORMAT_MESSAGE_FROM_SYSTEM,
		     // source and processing options
		     NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1, NULL);

      I_punt ("Error creating event %s: %s", event_name, msg_buf);
      LocalFree (msg_buf);	// never called
    }

  /* Initialize process structures to default */
  memset (&si, 0, sizeof (STARTUPINFO));
  si.cb = sizeof (STARTUPINFO);
  memset (&pi, 0, sizeof (PROCESS_INFORMATION));

  /* Create the command line */
  strcpy (cmd_line, command_shell_path);
  strcat (cmd_line, " -c \"");
  strcat (cmd_line, trace_source);
  strcat (cmd_line, "\"");

  result = CreateProcess (NULL,	        /* exe name */
			  cmd_line,	/* command line string */
			  NULL,	        /* process security attributes */
			  NULL,	        /* thread security attributes */
			  0,	        /* handle inheritance flag */
			  0,	        /* creation flags */
			  NULL,	        /* & new environment block */
			  NULL,	        /* & current directory name */
			  &si,	        /* & STARTUPINFO */
			  &pi	        /* & PROCESS_INFORMATION */
    );

  if (!result)
    {
      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		     FORMAT_MESSAGE_FROM_SYSTEM, /* source, proc options */
		     NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1, NULL);

      I_punt ("Error creating process: %s", msg_buf);
      LocalFree (msg_buf);	// never called
    }

  child_pid = pi.dwProcessId;

  ret_val = WaitForSingleObject (pipe_event, timeout_delay * 1000);
  if (ret_val != WAIT_OBJECT_0)
    {
      if (ret_val == WAIT_TIMEOUT)
	P_trace_time_out ();
      if (ret_val == WAIT_FAILED)
	{
	  FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
			 NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1,
			 NULL);

	  I_punt ("init_trace: Error waiting for event: %s", msg_buf);
	  LocalFree (msg_buf);	// never called
	}
      I_punt ("init_trace: Error waiting for event");
    }

  if (!ReadFile (ptrace->source_fd, ptrace->buf, buf_size, &read_cnt, NULL))
    {
      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
		     NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1, NULL);

      I_punt ("init_trace: Could not read trace file: %s", msg_buf);
      LocalFree (msg_buf);	// never called
    }

#else

  /* Create ptrace buffer and struct */

  /* 
   * Make the simulation the process group leader so
   * that the probed program will create a trace pipe using
   * the pid of the simulation
   */
  setsid ();

  /* Create new process. MCM-Save child shell's process id. */
  if ((child_pid = fork ()) == 0)
    {
      /*
       * Make this new process a command shell and execute the
       * trace_source command in it.
       */
      execlp (command_shell_path, command_shell_path, "-c",
	      trace_source, (char *) 0);

      /* Detect failure of execlp, should never get here */
      I_punt ("execlp failed executing: %s -c \"%s\"",
	      command_shell_path, trace_source);
    }

  /* Name of pipe created uses simulation's pid */
  /* Now trace pipe always placed in /tmp/trace.pipe.%i */
  sprintf (fifo_name, "/tmp/trace.pipe.%i", getpid ());
  sprintf (link_name, "trace.pipe.%i", getpid ());

  /* 
   *  In order to prevent hanging forever, set an alarm to wake us
   * up if we don't read something from S_trace_fd soon.
   */
  signal (SIGALRM, P_trace_time_out);

  /*
   * Give the probed program timeout_delay seconds to generate some
   * trace info before punting.
   */
  alarm (timeout_delay);

  /* Make fifo, ok if already exists */
  if ((mknod (fifo_name, S_IFIFO | 0666, 0) < 0) && (errno != EEXIST))
    I_punt ("Cannot create fifo %s, unix error no %i\n", fifo_name, errno);

  /* Create link to the name buf in current directory -JCG 10/18/96
   * Ok if already exists since probed executable is also trying
   * to make link
   */
  if ((symlink (fifo_name, link_name) == -1) && (errno != EEXIST))
    I_punt ("Unable to link '%s' to '%s'\n", link_name, fifo_name);

  /* Open link in the mode requested */
  if ((ptrace->source_fd = open (link_name, 0, 0)) < 0)
    I_punt ("Unable to open fifo %s, flags %i, mode %i\n", link_name, 0, 0);

  /* disable timeout */
  alarm (0);

  /* Remove trace.pipe and link (neither are necessary at this point)
   * ignore errors since probed executable is also trying to clean up.
   */
  unlink (link_name);
  unlink (fifo_name);

  /* read trace into buffer */
  read_cnt = read (ptrace->source_fd, ptrace->buf, buf_size);

  if (read_cnt <= 0)
    I_punt ("init_trace: Could not read trace file");

#endif

  /* Set end of buffer based on number of words read */
  ptrace->ptr = &ptrace->buf[0];
  ptrace->end = &ptrace->buf[read_cnt >> 2];
  ptrace->func = NULL;
  ptrace->trace_count = 0;

  /* Read what should be L_TRACE_START_FORMAT3 */
  trace_word = get_ptrace (ptrace);

  /* If it isn't, determine if byte order needs reversing  */
  if ((trace_word != L_TRACE_START_FORMAT3) &&
      (trace_word != L_TRACE_START_FORMAT2) &&
      (trace_word != L_TRACE_START_FORMAT1))
    {
      if ((trace_word == SWAP_BYTES (L_TRACE_START_FORMAT3)) ||
	  (trace_word == SWAP_BYTES (L_TRACE_START_FORMAT2)) ||
	  (trace_word == SWAP_BYTES (L_TRACE_START_FORMAT1)))
	{
	  /* switch byte order flag */
	  if (ptrace_byte_order_reversed)
	    ptrace_byte_order_reversed = 0;
	  else
	    ptrace_byte_order_reversed = 1;

	  /* Switch byte order of trace already in buffer */
	  Ptrace_reverse_byte_order (ptrace->ptr, ptrace->end);
	  trace_word = SWAP_BYTES (trace_word);

	  /* Tell them we are switching byte order */
	  fprintf (stderr, "\n*** Reversing trace byte order ***\n");
	}
      else
	I_punt ("init_trace: L_TRACE_START_FORMAT3 expected first");
    }

  /* For FORMAT3, get trace flags (otherwise default to 0) */
  if (trace_word == L_TRACE_START_FORMAT3)
    {
      trace_flags = get_ptrace (ptrace);

      /* Detect supported flags & make sure we know what every flag means! */
      untested_flags = trace_flags;

      /* Make sure everything is turned on that Lprofile expects in 
       * the trace.
       */
      TEST_TF_FLAG (&untested_flags, TF_PROBE_FOR_PROFILING, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_CONTROL_FLOW, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_EMPTY_CBS, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_JUMP_RG_SRC1, 1);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_PRED_JUMP_FALL_THRU, 1);

      if (Lprofile_mem_addrs)
	TEST_TF_FLAG (&untested_flags, TF_TRACE_MEM_ADDRS, 1);
      else
	TEST_TF_FLAG (&untested_flags, TF_TRACE_MEM_ADDRS, 0);

      /* Make sure everything else is turned off */
      TEST_TF_FLAG (&untested_flags, TF_PROBE_FOR_SIMULATION, 0);
      TEST_TF_FLAG (&untested_flags, TF_PROBE_FOR_CUSTOM, 0);
      TEST_TF_FLAG (&untested_flags, TF_PREDICATE_PROBE_CODE, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_MASKED_LOAD_FAULTS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_PRED_USES, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_PRED_DEFS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_PROMOTED_PREDS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_EXTRA_HEADERS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_OP_IDS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_ENHANCED_OP_IDS, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_DEST_REG_VALUES, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_SRC_REG_VALUES, 0);
      TEST_TF_FLAG (&untested_flags, TF_TRACE_SRC_LIT_VALUES, 0);

      /* Determine if function ids (Lemulate) or address are in the trace */

      if (trace_flags & TF_FUNC_IDS)
	use_func_ids_not_addrs = 1;
      else
	use_func_ids_not_addrs = 0;

      untested_flags &= ~TF_FUNC_IDS;

      if (untested_flags != 0)
	{
	  fprintf (stderr,
		   "Error: Unknown trace flags (0x%08x) present!\n",
		   untested_flags);
	  fprintf (stderr,
		   "       See impact/include/Lcode/l_trace_interface.h "
		   "for details!\n");
	  TF_flag_errors++;
	}

      /* Punt if have trace flag errors */
      if (TF_flag_errors != 0)
	{
	  I_punt ("Lprofile exiting due to above unexpected trace flag "
		  "settings!");
	}
    }
  /* Otherwise, use default settings for FORMAT2 and earlier */
  else
    {
      use_func_ids_not_addrs = 0;
      trace_flags = 0x00000000;
    }

  if ((trace_word = get_ptrace (ptrace)) != L_TRACE_SAMPLE_START)
    I_punt ("init_trace: L_TRACE_SAMPLE_START expected, not %i", trace_word);

  return (ptrace);
}

/* SER: for memory profiling */
static void
P_update_address_hist (Pcontrol * pint, int addr,
		       int global_objs_table[MAX_GLOB_OBJS][8],
		       int heap_objs_table[MAX_LIVE_HEAP_OBJS][8],
		       int malloc_table[MAX_MALLOCS][12], int st_flag)
{
  int i, j, index, num_objs = pint->mem_addr_struct->num_objs;
  int * obj_ids = pint->mem_addr_struct->id;
  int * count = pint->mem_addr_struct->count;
  int heap_checked = 0, heap_id, malloc_id, malloc_found = 0;
  struct Pop_heap_obj * heap_obj;
  struct Pop_missing_obj * missing_obj;

#if DEBUG_OBJ
  fprintf (stderr, "Checking for address 0%x in table\n", addr);
#endif
  for (i = 0; i < num_objs; i++)
    {
      index = obj_ids[i];
      if (index == 0)
	continue;
      if (index >= glob_index_max || global_objs_table[index][0] != index)
	{
#ifndef TRACK_HEAP_OBJS
	  continue;
#else
	  if (heap_checked++ > 0)  /* only check in stack/heap once */
	    continue;
	  /* Search in stack. */
	  for (j = 0; j <= call_depth; j++)
	    {
	      if (!((addr >= stack_table[j][0]) && (addr < stack_table[j][1])))
		continue;

	      if (st_flag)
		{
		  stack_stores++;
		  if (pint->mem_addr_struct->stack_count++ == 0)
		    stack_st_sites++;
		}
	      else
		{
		  stack_loads++;
		  if (pint->mem_addr_struct->stack_count++ == 0)
		    stack_ld_sites++;
		}
	      return;  /* found in stack */
	    }

          /* Search in heap instead. */
	  for (j = 1; j < heap_index_max; j++)
	    {
	      if (heap_objs_table[j][0] == 0)
		continue;
	      if (!(addr >= heap_objs_table[j][1] &&
		    addr < heap_objs_table[j][2]))
		continue;
	      /* match */
	      heap_id = heap_objs_table[j][0];
#ifdef DEBUG_HEAP_OBJ
	      fprintf (stderr, "Found heap obj %d in table entry %d\n",
		       heap_id, j);
#endif
	      malloc_id = heap_objs_table[j][3];
	      for (heap_obj = pint->mem_addr_struct->heap_obj;
		   heap_obj != NULL; heap_obj = heap_obj->next)
		{
		  if (heap_obj->id == heap_id)
		    {
#ifdef DEBUG_HEAP_OBJ
		      fprintf (stderr, "Found heap obj %d in pint list\n");
#endif
		      malloc_found = 1;
		      break;
		    }
		  else if (heap_obj->malloc_id == malloc_id)
		    malloc_found = 1;
		}
	      if (heap_obj == NULL)
		{
#ifdef DEBUG_HEAP_OBJ
		  fprintf (stderr, "Creating heap obj %d in pint list\n",
			   heap_id);
#endif
		  heap_obj = malloc (sizeof(Pop_heap_obj));
		  heap_obj->id = heap_id;
		  heap_obj->count = 0;
		  heap_obj->malloc_id = malloc_id;
		  heap_obj->next = pint->mem_addr_struct->heap_obj;
		  pint->mem_addr_struct->heap_obj = heap_obj;
		  pint->mem_addr_struct->num_heap_objs++;
		}
	      if (st_flag)
		{
		  heap_objs_table[j][5]++;
		  if ((heap_obj->count)++ == 0)
		    heap_objs_table[j][7]++;
		  malloc_table[malloc_id][6]++;
		  if (!malloc_found)
		    malloc_table[malloc_id][8]++;
		}
	      else
		{
		  heap_objs_table[j][4]++;
		  if ((heap_obj->count)++ == 0)
		    heap_objs_table[j][6]++;
		  malloc_table[malloc_id][5]++;
		  if (!malloc_found)
		    malloc_table[malloc_id][7]++;
		}
	      return;
	    }
	  /* Need to continue if object is not in table. */
	  continue;
#endif
	}
#ifdef DEBUG_OBJ
      fprintf (stderr, "  Checking index %d, start addr 0x%x, end addr 0x%x\n",
	       index, global_objs_table[index][1],
	       global_objs_table[index][2]);
#endif
      if ((addr >= global_objs_table[index][1]) &&
	  (addr < global_objs_table[index][2]))
	{
#ifdef DEBUG_OBJ
	  fprintf (stderr, "  Found object id %d\n", index);
#endif
	  if (st_flag)
	    {
	      global_objs_table[index][5]++;
	      if (0 == count[i]++)
		global_objs_table[index][7]++;
	    }
	  else
	    {
	      global_objs_table[index][4]++;
	      if (0 == count[i]++)
		global_objs_table[index][6]++;
	    }
	  return;
	}
    }

  /* At this point, the access is either a true id0 object access, meaning
   * it is misspeculated or a global not in the table, or it is missing
   * from the op's OBJS info. To facilitate debugging, we try to find missing
   * info here and mark the pint with it, so we can mark the Lcode op
   * in Lget. */

  unmarked_access++;
  /* First check global objs, since it's relatively fast. */
  for (j = 1; j < glob_index_max; j++)
    {
      if (global_objs_table[j][0] == 0)
	continue;
      if (!(addr >= global_objs_table[j][1] &&
	    addr < global_objs_table[j][2]))
	continue;
      /* match */
#ifdef DEBUG_OBJ
      fprintf (stderr, "Unmarked global object found, id %d, st_flag %d.\n", j,
	       st_flag);
#endif
      missing_global++;
      for (missing_obj = pint->mem_addr_struct->missing_obj;
	   missing_obj != NULL; missing_obj = missing_obj->next)
	{
	  if (missing_obj->id == j)
	    break;
	}
      if (missing_obj == NULL)
	{
	  pint->mem_addr_struct->num_missing_objs++;
	  missing_obj = malloc (sizeof(Pop_missing_obj));
	  missing_obj->id = j;
	  missing_obj->type = OBJ_GLOBAL;
	  missing_obj->count = 0;
	  missing_obj->next = pint->mem_addr_struct->missing_obj;
	  pint->mem_addr_struct->missing_obj = missing_obj;
	}
      if (st_flag)
	{
	  global_objs_table[j][4]++;
	  if (missing_obj->count++ == 0)
	    global_objs_table[j][6]++;
	}
      else
	{
	  global_objs_table[j][5]++;
	  if (missing_obj->count++ == 0)
	    global_objs_table[j][7]++;
	}
      return;
    }

  /* If we haven't checked in the heap or stack yet, do so. */
  if (heap_checked == 0)
    {
      /* First, check in stack. */
      for (j = 0; j <= call_depth; j++)
	{
	  if (!((addr >= stack_table[j][0]) && (addr < stack_table[j][1])))
	    continue;
	  /* match */
#ifdef DEBUG_OBJ
	  fprintf (stderr, "Late stack object found\n");
#endif
	  missing_stack++;
	  if (st_flag)
	    {
	      stack_stores++;
	      if (pint->mem_addr_struct->stack_count++ == 0)
		stack_st_sites++;
	    }
	  else
	    {
	      stack_loads++;
	      if (pint->mem_addr_struct->stack_count++ == 0)
		stack_ld_sites++;
	    }

	  for (missing_obj = pint->mem_addr_struct->missing_obj;
	       missing_obj != NULL; missing_obj = missing_obj->next)
	    {
	      if (missing_obj->type == OBJ_STACK)
		{
		  missing_obj->count++;
		  return;
		}
	    }
	  if (missing_obj == NULL)
	    {
	      pint->mem_addr_struct->num_missing_objs++;
	      missing_obj = malloc (sizeof(Pop_missing_obj));
	      missing_obj->id = 0;
	      missing_obj->type = OBJ_STACK;
	      missing_obj->count = 1;
	      missing_obj->next = pint->mem_addr_struct->missing_obj;
	      pint->mem_addr_struct->missing_obj = missing_obj;
	    }
	  return;  /* found in stack */
	}

      /* Next, check in heap. */
      for (j = 0; j < heap_index_max; j++)
	{
	  if (heap_objs_table[j][0] == 0)
	    continue;
	  if (!(addr >= heap_objs_table[j][1] &&
		addr < heap_objs_table[j][2]))
	    continue;
	  /* match */
#ifdef DEBUG_OBJ
	  fprintf (stderr, "Late heap object found\n");
#endif
	  missing_heap++;
	  heap_id = heap_objs_table[j][0];
	  malloc_id = heap_objs_table[j][3];
	  for (heap_obj = pint->mem_addr_struct->heap_obj;
	       heap_obj != NULL; heap_obj = heap_obj->next)
	    {
	      if (heap_obj->id == heap_id)
		{
		  malloc_found = 1;
		  break;
		}
	      else if (heap_obj->malloc_id == malloc_id)
		malloc_found = 1;
	    }
	  if (heap_obj == NULL)
	    {
	      heap_obj = malloc (sizeof(Pop_heap_obj));
	      heap_obj->id = heap_id;
	      heap_obj->count = 0;
	      heap_obj->malloc_id = malloc_id;
	      heap_obj->next = pint->mem_addr_struct->heap_obj;
	      pint->mem_addr_struct->heap_obj = heap_obj;
	      pint->mem_addr_struct->num_heap_objs++;

	      if (!malloc_found)
		{
		  pint->mem_addr_struct->num_missing_objs++;
		  missing_obj = malloc (sizeof(Pop_missing_obj));
		  missing_obj->id = malloc_id + BASE_MALLOC_ID;
		  missing_obj->type = OBJ_GLOBAL;
		  missing_obj->count = 0;
		  missing_obj->next = pint->mem_addr_struct->missing_obj;
		  pint->mem_addr_struct->missing_obj = missing_obj;
		}
	    }
	  if (st_flag)
	    {
	      heap_objs_table[j][5]++;
	      if ((heap_obj->count)++ == 0)
		heap_objs_table[j][7]++;
	      malloc_table[malloc_id][6]++;
	      if (!malloc_found)
		malloc_table[malloc_id][8]++;
	    }
	  else
	    {
	      heap_objs_table[j][4]++;
	      if ((heap_obj->count)++ == 0)
		heap_objs_table[j][6]++;
	      malloc_table[malloc_id][5]++;
	      if (!malloc_found)
		malloc_table[malloc_id][7]++;
	    }
	  return;
	}
    }

  /* If not found, finally increment last entry. */
#ifdef DEBUG_OBJ
  fprintf (stderr, "  Object not found for address 0x%x\n", addr);
  if (i == 0)
    fprintf (stderr, "Op with no objects found\n");
#endif
  if (st_flag)
    {
      global_objs_table[0][5]++;
      if (count[num_objs]++ == 0)
	global_objs_table[0][7]++;
    }
  else
    {
      global_objs_table[0][4]++;
      if (count[num_objs]++ == 0)
	global_objs_table[0][6]++;
    }
}

static void
gather_profile_info (Ptrace * ptrace)
{
  Pfunc *func, *new_func;
  Pcontrol *pint, *prev_pint, *cb_entered;
  Ploop_info *loop_info;
  int addr, pc, jsr_id, size, e_size;
  int trace_word;
  int prev_cb_id, entered_from_preheader;
  unsigned int cb;
  int just_entered_cb_id;
  int global_objs_table[MAX_GLOB_OBJS][8] = {{0}};
  int heap_objs_table[MAX_LIVE_HEAP_OBJS][8] = {{0}};
  int malloc_table[MAX_MALLOCS][12] = {{0}};
  int i, spills = 0, fills = 0, heap_obj_id = 1;
  int local_stack_size, stack_current_size = 0, stack_max_size = 0;

  /* SER 20040510: For HCH MICRO '04 submission */
  if (Lprofile_mem_addrs)
    {
      while ((trace_word = get_ptrace (ptrace)) != L_TRACE_FN)
	{
	  int ignore_var = 0;
	  if (trace_word != L_TRACE_ASYNCH)
	    I_punt ("gather_profile_info: invalid trace value");
	  trace_word = get_ptrace (ptrace);
	  switch (trace_word)
	    {
	    case L_TRACE_OBJ_GLOB:
	      /* 1: id of obj */
	      trace_word = get_ptrace (ptrace);
	      if (trace_word == -10)
		ignore_var = 1;
	      else if (trace_word < 1 || trace_word > MAX_GLOB_OBJS)
		I_punt ("gather_profile_info: global obj not in table bounds.\n");
	      if (trace_word >= glob_index_max)
		glob_index_max = trace_word + 1;
	      /* 2: address of obj */
	      addr = get_ptrace (ptrace);
	      /* 3: size of obj */
	      size = get_ptrace (ptrace);
	      e_size = get_ptrace (ptrace);
	      if (ignore_var)
		{
#if 1
		  fprintf (stderr, "Ignoring id %d, addr 0x%x, size %d\n",
			   trace_word, addr, size);
#endif
		}
	      else
		{
		  global_objs_table[trace_word][0] = trace_word;
		  global_objs_table[trace_word][1] = addr;
		  global_objs_table[trace_word][2] = addr+size;
		  global_objs_table[trace_word][3] = e_size;
		  global_objs_table[trace_word][4] = 0;  /* load weight */
		  global_objs_table[trace_word][5] = 0;  /* store weight */
		  global_objs_table[trace_word][6] = 0;  /* load reference sites */
		  global_objs_table[trace_word][7] = 0;  /* store reference */
#if 1
		  fprintf (stderr, "Adding id %d, addr 0x%x, end addr "
			   "0x%x (size %d), element size %d to table\n",
			   trace_word, addr, global_objs_table[trace_word][1],
			   size, e_size);
#endif
		}
	      break;
	    default:
	      I_punt ("gather_profile_info: invalid trace value");
	    }
	}
    }
  else if ((trace_word = get_ptrace (ptrace)) != L_TRACE_FN)
    I_punt ("gather_profile_info: L_TRACE_FN expected, not %i", trace_word);

  /* Get function that we jumped into */
  addr = get_ptrace (ptrace);
  func = find_func (addr);

  /* SER: for HCH MICRO '04 */
  if (Lprofile_mem_addrs)
    {
      stack_table[0][0] = get_ptrace (ptrace);
      local_stack_size = get_ptrace (ptrace);
      stack_table[0][1] = local_stack_size + stack_table[0][0];
      stack_current_size = local_stack_size;
      stack_max_size = local_stack_size;

#if 0
      fprintf (stderr, "main() stack between 0x%x and 0x%x\n", stack_table[0][0],
	       stack_table[0][1]);
#endif
    }

  if (func == NULL)
    {
      print_debug_info (ptrace, func, 1);
      I_punt ("Unknown main function address %i", addr);
    }
  func->weight++;
  func->control_tab[0].weight++;

  /* Start after cb 1 */
  pc = 1;

  /* Initate the variable for remembering cb just entered */
  just_entered_cb_id = func->control_tab[0].id;

  /* Initially prev_pint = NULL, otherwise points to previous pint.
   * Used by loop info code to determine the cb that the loops
   * was entered from.  
   */
  pint = NULL;
  prev_pint = NULL;

  while (1)
    {
      ITintmax old_wt;
      trace_word = get_ptrace (ptrace);
      pint = &func->control_tab[pc];

      /* Exit if hit TRACE_SAMPLE_END and not a hashing jump, (The hashing
       * jump, load, and store can produce any value for its condition code)
       */

      if ((trace_word == L_TRACE_SAMPLE_END) && (pint->type < HASH))
	{
	  /* Branch predict last instruction (Should always be miss) */
	  if ((pint->type != CB) && profile_BTB)
	    P_model_BTB (&func->BTB_tab[pc], 1, -1);
	  break;
	}
      old_wt = pint->weight;
      pint->weight++;
      if (pint->weight < old_wt)
	I_punt ("Counter rolled (weight)!");

      /* Debug */
      /*  printf ("Trace_word = %i\n", trace_word); */
      switch (pint->type)
	{
	case HASH:
	case CB:
	case JMP:
	case PJMP:
	case BR:

	  /*
	   * Hashing jump, need to keep trace of what condition code
	   * jump was based on.
	   */
	  if (pint->type == HASH)
	    {
	      increment_cond_count (pint, trace_word);
	      if ((trace_word = get_ptrace (ptrace)) == L_TRACE_END)
		{
		  print_debug_info (ptrace, func, pc);
		  I_punt ("Unexpected EOF during hashing jump");
		}
	    }
	  else if (pint->type == CB)
	    {
	      just_entered_cb_id = pint->id;
	    }

	  /* Fell through branch */
	  if (trace_word == L_TRACE_BRTHRU)
	    {
	      if ((pint->type == BR) || (pint->type == PJMP))
		{
		  if (profile_BTB)
		    P_model_BTB (&func->BTB_tab[pc], 0, 0);
		  pc = pc + 1;
		}
	      else
		{
		  print_debug_info (ptrace, func, pc);
		  I_punt ("L_TRACE_BRTHRU unexpected for type %i, id %i",
			  pint->type, pint->id);
		}

	      /* Set prev_pint to the branch just fell thru */
	      prev_pint = pint;
	    }
	  else
	    {
	      cb = -trace_word;

	      if (pint->type == CB)
		{
		  if (cb != pint->id)
		    {
		      print_debug_info (ptrace, func, pc);
		      I_punt ("Cb %i expected, not cb %i", pint->id, cb);
		    }
		}

	      /* Increment branched stats */
	      old_wt = pint->branched;
	      pint->branched++;
	      if (pint->branched < old_wt)
		I_punt ("Counter rolled (branched)!");

	      if (cb > func->max_cb)
		{
		  print_debug_info (ptrace, func, pc);
		  I_punt ("Cb %i out of bounds", cb);
		}

	      if (profile_BTB)
		P_model_BTB (&func->BTB_tab[pc], 1, cb);

	      /* Get new pc */
	      pc = func->cb_tab[cb] + 1;

	      /* Get a pointer to the pint for the cb we just entered */
	      cb_entered = &func->control_tab[pc - 1];

	      /* Increase target cb weight if branched */
	      if (pint != cb_entered)
		{
		  old_wt = cb_entered->weight;
		  cb_entered->weight++;
		  if (cb_entered->weight < old_wt)
		    I_punt ("Counter rolled (cb)!");
		}

	      /* If the cb we just entered is the header of a loop,
	       * collect loop information
	       */
	      if (cb_entered->flags & LOOP_HEADER)
		{
		  /* Get the cb the loop was entered from */
		  if (prev_pint)
		    prev_cb_id = prev_pint->cb_id;
		  else
		    prev_cb_id = just_entered_cb_id;

		  /* Get a pointer to loop_info for ease of use */
		  loop_info = cb_entered->ext.loop_info;

		  /* Determine if loop was just entered from preheader
		   * use loop_preheader array (1 if prev_cb_id is a 
		   * preheader to the loop).
		   */
		  entered_from_preheader =
		    loop_info->loop_preheader[prev_cb_id];

		  if (entered_from_preheader)
		    {
		      /* Update loop iterations stats */
		      update_loop_iter_stats (loop_info);
		      loop_info->cur_iter_count = 1;
		      loop_info->cur_inner_iter_count = 1;
		    }
		  else
		    {
		      loop_info->cur_iter_count++;

		      if (L_do_buf_info && loop_info->is_inner)
			{
			  if(prev_cb_id == cb)
			    {
			      loop_info->cur_inner_iter_count++;
			    }
			  else
			    {
			      if (loop_info->cur_inner_iter_count)
				fprintf (buf_file, "%s %d %d\n",
					 loop_info->func->name, loop_info->cb->id,
					 loop_info->cur_inner_iter_count);
			      loop_info->cur_inner_iter_count=1;
			    }
			}
		    }
#if DEBUG_LOOP_ITERS
		  fprintf (stderr,
			   "Entered loop header cb %i from cb %i entered %i\n",
			   cb_entered->id, prev_cb_id,
			   entered_from_preheader);
#endif
		}

	      /* Set prev_pint to the cb just entered */
	      prev_pint = cb_entered;
	    }
	  break;

	case JSR:
	case RET:
	  /* Called traced function */
	  if (trace_word == L_TRACE_FN)
	    {
	      addr = get_ptrace (ptrace);
	      new_func = find_func (addr);

	      if (!new_func)
		{
		  print_debug_info (ptrace, func, pc);
		  I_punt ("JSR to unknown function (address %i)\n\n"
			  "A possible cause:\n\n"
			  "Many programs, when encountering\n"
			  "an input error, call the function "
			  "error().  This function's\n"
			  "address is incorrectly reported to "
			  "the profiler and can cause\n"
			  "the above error.\n\n"
			  "Make sure all needed files are present "
			  "and the input is correct.\n\n"
			  "Terminating profile due to JSR to unknown "
			  "function with address %i", addr, addr);
		}

	      if (profile_BTB)
		P_model_BTB (&func->BTB_tab[pc], 1, addr);

	      func = new_func;
	      func->weight++;
	      func->control_tab[0].weight++;
	      pc = 1;

	      /* Set prev_pint to the cb just entered */
	      prev_pint = &func->control_tab[pc];

	      /* SER: For HCH MICRO '04 */
	      if (Lprofile_mem_addrs)
		{
		  if (++call_depth >= MAX_CALL_DEPTH)
		    I_punt ("call_depth exceeded MAX_CALL_DEPTH, increase");
		  stack_table[call_depth][0] = get_ptrace(ptrace);
		  local_stack_size = get_ptrace (ptrace);
		  stack_table[call_depth][1] = local_stack_size +
		    stack_table[call_depth][0];
		  stack_current_size += local_stack_size;
		  if (stack_current_size > stack_max_size)
		  stack_max_size = stack_current_size;
#if 0
		  fprintf (stderr, "New stack %d range from 0x%x to 0x%x\n",
			   call_depth, stack_table[call_depth][0],
			   stack_table[call_depth][1]);
#endif
		}
	    }
	  else
	    {
	      if (trace_word > -2048)
		{
		  print_debug_info (ptrace, func, pc);
		  I_punt ("At return, expected (-2048-jsr_id), not %i",
			  trace_word);
		}

	      jsr_id = -2048 - trace_word;

	      addr = get_ptrace (ptrace);
	      new_func = find_func (addr);

	      if (new_func == NULL)
		{
		  print_debug_info (ptrace, func, pc);
		  I_punt ("RET to unknown function (address %i, jsr_id %i)",
			  addr, jsr_id);
		}

	      if (jsr_id > new_func->num_jsrs)
		{
		  print_debug_info (ptrace, func, pc);
		  I_punt ("Ret to function %i, jsr_id (%i) out of bounds",
			  new_func->name, jsr_id);
		}

	      /* 
	       * May be unprofiled JSR or normal RET.  Don't care.
	       * Just treat as branch to address + jsr_id.
	       */
	      if (profile_BTB)
		P_model_BTB (&func->BTB_tab[pc], 1, addr + jsr_id);

	      func = new_func;
	      pc = func->jsr_tab[jsr_id] + 1;

	      /* Set prev_pint to the cb that the jsr was in */
	      prev_pint = &func->control_tab[pc - 1];
	      if (Lprofile_mem_addrs && pint->type == RET)
		{
		  local_stack_size = stack_table[call_depth][1] -
		    stack_table[call_depth][0];
		  stack_current_size -= local_stack_size;
		  if (--call_depth < 0)
		    I_punt ("call_depth below 0!");
#if 0
		  fprintf (stderr, "Decreasing call depth to %d\n", call_depth);
#endif
		}
	    }
	  break;

	case LOAD:
	  /* Check and update address history. */
#ifdef DEBUG_MEM_ADDR
	  fprintf (stderr, "Load op in function %s, cb %d, addr 0x%x\n",
		   func->name, cb, trace_word);
#endif
	  P_update_address_hist (pint, trace_word, global_objs_table, 
				 heap_objs_table, malloc_table, 0);
	  pc++;
	  break;

	case LOAD_ALLOC:
	  fills++;
	  pc++;
	  break;

#if 0
	case LOAD_STACK:
	  stack_loads++;
	  pc++;
	  break;
#endif

	case STORE:
	  /* Check and update address history. */
#ifdef DEBUG_MEM_ADDR
	  fprintf (stderr, "Store op in function %s, cb %d, addr 0x%x\n",
		   func->name, cb, trace_word);
#endif
	  P_update_address_hist (pint, trace_word, global_objs_table,
				 heap_objs_table, malloc_table, 1);
	  pc++;
	  break;

	case STORE_ALLOC:
	  spills++;
	  pc++;
	  break;
#if 0
	case STORE_STACK:
	  stack_stores++;
	  pc++;
	  break;
#endif
	case MALLOC:  /* Create a new heap object. */
	  addr = trace_word;
	  size = get_ptrace(ptrace);
#ifdef TRACK_HEAP_OBJS
#if 0
	  fprintf (stderr, "malloc() new heap obj %d, addr 0x%x, size %d\n",
		   heap_obj_id, addr, size);
#endif
	  for (i = heap_index_full; i < MAX_LIVE_HEAP_OBJS; i++)
	    {
	      int malloc_id;
	      if (heap_objs_table[i][0])
		continue;
	      heap_objs_table[i][0] = heap_obj_id;
	      heap_objs_table[i][1] = addr;
	      heap_objs_table[i][2] = addr+size;
	      heap_objs_table[i][4] = 0;
	      heap_objs_table[i][5] = 0;
	      heap_objs_table[i][6] = 0;
	      heap_objs_table[i][7] = 0;
	      heap_index_full = i + 1;
	      if (i >= heap_index_max)
		heap_index_max = i + 1;

	      /* now track malloc info */
	      malloc_id = (pint->mem_addr_struct->id)[0];
	      heap_objs_table[i][3] = malloc_id;
	      malloc_table[malloc_id][0] = malloc_id;
	      malloc_table[malloc_id][1] += size;
	      if (malloc_table[malloc_id][1] > malloc_table[malloc_id][2])
		malloc_table[malloc_id][2] = malloc_table[malloc_id][1];
	      malloc_table[malloc_id][3] += size;
	      /* Track malloc size. */
	      if (malloc_table[malloc_id][4] == 0)
		malloc_table[malloc_id][4] = size;
	      else if (malloc_table[malloc_id][4] != size)
		malloc_table[malloc_id][4] = -1;
	      /* Track malloc numbers. */
	      malloc_table[malloc_id][9]++;
	      if (malloc_table[malloc_id][9] > malloc_table[malloc_id][10])
		malloc_table[malloc_id][10] = malloc_table[malloc_id][9];
	      malloc_table[malloc_id][11]++;
	      break;
	    }
	  if (i >= MAX_LIVE_HEAP_OBJS)
	    I_punt("Need to enlarge heap object table.");
	  heap_obj_id++;
#endif
	  pc++;
	  break;

	case FREE:
	  addr = trace_word;
#ifdef TRACK_HEAP_OBJS
#if 0
	  fprintf (stderr, "free() heap obj, addr 0x%x\n", addr);
#endif
	  for (i = 0; i < heap_index_max; i++)
	    {
	      if (addr != heap_objs_table[i][1])
		continue;
	      break;
	    }
	  size = heap_objs_table[i][2] - heap_objs_table[i][1];
	  if (i >= heap_index_max)
	    {
	      free_nonvis_malloc++;
	      /* I_punt ("free() did not find valid heap obj in table."); */
	    }
	  /* Insert heap object into global table immediately. */
	  else {
	    int malloc_id;
#if 0
	    int j;
	    for (j = glob_index_full; j < MAX_GLOB_OBJS; j++)
	      {
		if (global_objs_table[j][0])
		  continue;
		global_objs_table[j][0] =
		  heap_objs_table[i][0] + BASE_HEAP_OBJ_ID;
		global_objs_table[j][1] = heap_objs_table[i][1];
		global_objs_table[j][2] = heap_objs_table[i][2];
		global_objs_table[j][3] = size;
		global_objs_table[j][4] = heap_objs_table[i][4];
		global_objs_table[j][5] = heap_objs_table[i][5];
		global_objs_table[j][6] = heap_objs_table[i][6];
		global_objs_table[j][7] = heap_objs_table[i][7];
		glob_index_full = j + 1;
		if (j >= glob_index_max)
		  glob_index_max = j + 1;

		/* now track malloc info */
		break;
	      }
	    if (j >= MAX_GLOB_OBJS)
	      I_punt("Need to enlarge global object table.");
#endif
	    heap_objs_table[i][0] = 0;
	    if (i < heap_index_full)
	      heap_index_full = i;
	    malloc_id = heap_objs_table[i][3];
	    malloc_table[malloc_id][1] -= size;
	    malloc_table[malloc_id][9]--;
	  }
#endif
	  pc++;
	  break;

	default:
	  print_debug_info (ptrace, func, pc);
	  I_punt ("Unknown instr type %i", pint->type);
	}
    }

  /* Expect L_TRACE_END here */
  trace_word = get_ptrace (ptrace);
  if (trace_word != L_TRACE_END)
    {
      print_debug_info (ptrace, func, pc);
      I_punt ("gather_profile_info: L_TRACE_END expected, not (%i)",
	      trace_word);
    }

#ifdef WIN32
  CloseHandle (ptrace->source_fd);
#endif

  /* Scan entire program for any stats updating that need to be done */
  for (func = func_list; func != NULL; func = func->next_func)
    {
      /* pcontrol is indexed starting at 1 */
      for (pc = 1; pc <= func->size; pc++)
	{
	  pint = &func->control_tab[pc];

	  /* Do final stat updating for all loops in program */
	  if (pint->flags & LOOP_HEADER)
	    {
	      update_loop_iter_stats (pint->ext.loop_info);
	      sort_iter_entries (pint->ext.loop_info);
#ifdef DEBUG_LOOP_ITERS
	      print_iter_info (stderr, pint->ext.loop_info);
#endif
	    }
	}
    }

  {
    FILE * out;
    int j, k, temp;
    if (Lprofile_mem_addrs)
      {
	/* SER 20040510: For HCH MICRO '04 */
#if 0
	/* Insert heap objects into global table. */
	i = 0;
	for (j = 1; j < MAX_GLOB_OBJS; j++)
	  {
	    while (heap_objs_table[i][0] == 0 && i < heap_index_max)
	      i++;
	    if (i >= heap_index_max)
	      break;
	    if (global_objs_table[j][0])
	      continue;

	    global_objs_table[j][0] = heap_objs_table[i][0] + BASE_HEAP_OBJ_ID;
	    global_objs_table[j][1] = heap_objs_table[i][1];
	    global_objs_table[j][2] = heap_objs_table[i][2];
	    global_objs_table[j][3] =
	      heap_objs_table[i][2] - heap_objs_table[i][1];
	    global_objs_table[j][4] = heap_objs_table[i][4];
	    global_objs_table[j][5] = heap_objs_table[i][5];
	    global_objs_table[j][6] = heap_objs_table[i][6];
	    global_objs_table[j][7] = heap_objs_table[i][7];
	    i++;
	    if (j >= glob_index_max)
	      glob_index_max = j + 1;
	  }
	if (i < heap_index_max)
	  I_punt ("Couldn't insert all heap objects into object table, "
		  "Enlarge.");
#endif
#if 0
	/* Insert malloc objects into global table. */
	i = 1;
	for (j = 1; j < MAX_MALLOCS; j++)
	  {
	    while (malloc_table[i][3] == 0 && i < MAX_MALLOCS)
	      i++;
	    if (i >= MAX_MALLOCS)
	      break;
	    if (globl_objs_table[j][0])
	      continue;

	    global_objs_table[j][0] = malloc_table[i][1] + BASE_MALLOC_ID;
	    global_objs_table[j][1] = 0;
	    global_objs_table[j][2] = malloc_table[i][3];
	    global_objs_table[j][3] = malloc_table[i][4];
	    global_objs_table[j][4] = malloc_table[i][5];
	    global_objs_table[j][5] = malloc_table[i][6];
	    global_objs_table[j][6] = malloc_table[i][7];
	    global_objs_table[j][7] = malloc_table[i][8];
	  }
#endif

	/* Bubble Sort objects. */
	fprintf (stderr, "Sorting %d objects\n", glob_index_max);
	for (i = 1; i <= glob_index_max; i++)
	  {
	    for (j = 1; j <= glob_index_max-i; j++)
	      {
		if ((global_objs_table[j+1][4] + global_objs_table[j+1][5]) >
		    (global_objs_table[j][4] + global_objs_table[j][5]))
		  { /* swap */
		    for (k = 0; k < 8; k++)
		      {
			temp = global_objs_table[j][k];
			global_objs_table[j][k] = global_objs_table[j+1][k];
			global_objs_table[j+1][k] = temp;
		      }
		  }
	      }
	  }

	/* Print object stats. */
	fprintf (stderr, "Printing object profiling results to obj.out\n");
	out = fopen ("obj.out", "w");

	if (unmarked_access)
	  fprintf (out, "Unmarked object accesses: %d\n", unmarked_access);
	if (missing_global)
	  fprintf (out, "Missing global accesses: %d\n", missing_global);
	if (missing_stack)
	  fprintf (out, "Missing stack accesses: %d\n", missing_stack);
	if (missing_heap)
	  fprintf (out, "Missing heap accesses: %d\n", missing_heap);

	fprintf (out, "%10s %8s %6s %12s %12s %6s %6s %6s %6s\n", "Object_ID",
		 "T_Size", "E_Size", "Load_Wgt", "Store_Wgt", "DL_St", "DS_St",
		 "SL_St", "SS_St");
	fprintf (out, "%10s %8d %6s %12d %12d %6d %6d %6s %6s\n", "stack",
		 stack_max_size, "-", stack_loads, stack_stores,
		 stack_ld_sites, stack_st_sites, "-", "-");
	for (i = 0; i < glob_index_max; i++)
	  if (i == 0 || global_objs_table[i][0])
	    /* print out all valid objects */
	    fprintf (out, "%10d %8d %6d %12d %12d %6d %6d %6d %6d\n",
		     global_objs_table[i][0],
		     global_objs_table[i][2] - global_objs_table[i][1],
		     global_objs_table[i][3],
		     global_objs_table[i][4], global_objs_table[i][5],
		     global_objs_table[i][6], global_objs_table[i][7],
		     static_glob_obj_info[global_objs_table[i][0]][0],
		     static_glob_obj_info[global_objs_table[i][0]][1]);
	fclose (out);

	/* Bubble Sort mallocs. */
	fprintf (stderr, "Sorting mallocs\n");
	for (i = 1; i <= num_malloc; i++)
	  {
	    for (j = 1; j <= num_malloc - i; j++)
	      {
		if ((malloc_table[j+1][5] + malloc_table[j+1][6]) >
		    (malloc_table[j][5] + malloc_table[j][6]))
		  { /* swap */
		    for (k = 0; k < 12; k++)
		      {
			temp = malloc_table[j][k];
			malloc_table[j][k] = malloc_table[j+1][k];
			malloc_table[j+1][k] = temp;
		      }
		  }
	      }
	  }

	fprintf (stderr, "Printing malloc profiling results to malloc.out\n");
	out = fopen ("malloc.out", "w");
	fprintf (out, "%9s %6s %6s %8s %9s %6s %12s %12s %5s %5s\n",
		 "Malloc_ID", "L_Mall", "T_Mall", "L_Size", "T_Size",
		 "P_Size", "Load_Wgt", "Store_Wgt", "L_St", "S_St");
	for (i = 1; i <= num_malloc; i++)
	  if (malloc_table[i][0])
	    fprintf (out, "%9d %6d %6d %8d %9d %6d %12d %12d %5d %5d\n",
		     malloc_table[i][0] + BASE_MALLOC_ID,
		     malloc_table[i][10], malloc_table[i][11],
		     malloc_table[i][2],
		     malloc_table[i][3], malloc_table[i][4],
		     malloc_table[i][5], malloc_table[i][6],
		     malloc_table[i][7], malloc_table[i][8]);
	fclose (out);

	if (free_nonvis_malloc)
	  fprintf (stderr, "Number of free() calls to non-visible malloc() "
		   "locations: %i\n", free_nonvis_malloc);
	/* End of HCH MICRO '04 */
      }
  }
  return;
}

/* 
 * Increments the weight for this cond value for a hashing jump.
 * Allocates a node, if necessary, inserting the nodes in 
 * sorted order based on cond.
 */
static void
increment_cond_count (Pcontrol * pint, int cond)
{
  Cnode *node, *new_node;

  /* 
   * If cond list empty, or this cond less than head of list, make first in
   * list.
   */
  if ((pint->ext.cnode_head == NULL) || (cond < pint->ext.cnode_head->cond))
    {
      if ((new_node = (Cnode *) malloc (sizeof (Cnode))) == NULL)
	I_punt ("increment_cond_count: Out of memory");

      new_node->cond = cond;
      new_node->weight = 1;
      new_node->next = pint->ext.cnode_head;
      if (pint->ext.cnode_head != NULL)
	pint->ext.cnode_head->prev = new_node;
      new_node->prev = NULL;
      pint->ext.cnode_head = new_node;
      return;
    }

  /* Otherwise, search for match or place to insert in list */
  node = pint->ext.cnode_head;
  while (node != NULL)
    {
      if (node->cond == cond)
	{
	  node->weight++;
	  return;
	}

      /*
       * If at the end of the list, or the next node cond is
       * bigger than the one desired, add a new node
       * after node
       */
      else if ((node->next == NULL) || (node->next->cond > cond))
	{
	  /* Allocate new node for condition code */
	  if ((new_node = (Cnode *) malloc (sizeof (Cnode))) == NULL)
	    I_punt ("increment_cond_count: Out of memory");

	  new_node->cond = cond;
	  new_node->weight = 1;
	  new_node->next = node->next;
	  new_node->prev = node;
	  if (node->next != NULL)
	    node->next->prev = new_node;
	  node->next = new_node;
	  return;
	}

      /* Otherwise, go to the next node to test its cond code */
      else
	node = node->next;
    }

  I_punt ("increment_cond_count: Should not reach here");
}
/* Prints the cond and weights for a hashing jump */
static void
print_condition_codes (FILE * out, Pcontrol * pint)
{
  int sum;
  Cnode *node;

  sum = 0;

  /* Print weight for every cond in linked list */
  for (node = pint->ext.cnode_head; node != NULL; node = node->next)
    {
      fprintf (out, "       (%3i " ITintmaxformat ".000000)\n",
	       node->cond, node->weight);
      sum += node->weight;
    }

  /* Make sure they all add up the instr weight */
  if (sum != pint->weight)
    I_punt ("print_condition_codes: instr weight %i and sum %i differ",
	    pint->weight, sum);
}

/* 
 * Strips leading and trailing whitespace and comments.
 * Modifies buffer contents.
 */
static char *
strip (char *buf)
{
  int i;

  /* Strip trailing white space, comments */
  for (i = 0; (buf[i] != 0) && (buf[i] != '#'); i++)
    ;
  buf[i] = 0;
  for (i = i - 1; (i >= 0) && isspace (buf[i]); i--)
    buf[i] = 0;

  /* Strip leading white space */
  for (; (*buf != 0) && isspace (*buf); buf++)
    ;

  return (buf);
}

static void
print_output (char *name)
{
  FILE *out;
  Pfunc *func;
  Pcontrol *instr;
  Ploop_info *loop_info;
  Piter *iter;
  int place_in_func;
  int i, k, entry_count;
  Pop_heap_obj * heap_obj;
  Pop_missing_obj * missing_obj;

  if ((out = fopen (name, "w")) == NULL)
    I_punt ("Could not open output file %s.\n", name);

  fprintf (out, "(count 1)\n");

  for (func = func_list; func != NULL; func = func->next_func)
    {
      fprintf (out, "(begin %s " ITintmaxformat ".000000)\n", func->name,
	       func->weight);

      /* Print out stats for all cbs first */
      for (i = 0; i < func->size; i++)
	{
	  instr = &func->control_tab[i];
	  switch (instr->type)
	    {
	    case CB:
	      if (instr->flags & LOOP_HEADER)
		{
		  loop_info = instr->ext.loop_info;
		  /* Print out lcb cb# cb_weight iter_count input_count */

		  fprintf (out, " (lcb %2i " ITintmaxformat ".000000 %2i 1\n",
			   instr->id, instr->weight, loop_info->entry_count);
		  entry_count = loop_info->entry_count;
		  for (k = 0; k < entry_count; k++)
		    {
		      iter = loop_info->sorted_iters[k];
		      fprintf (out, "       ( " ITintmaxformat " "
			       ITintmaxformat ".000000)\n",
			       iter->iterations, iter->weight);
		    }
		  fprintf (out, " )\n");
		}
	      else
		{
		  fprintf (out, " (cb %2i " ITintmaxformat ".000000)\n",
			   instr->id, instr->weight);
		}
	      break;
	    }
	}
      /* Print out stats for all branches */
      place_in_func = 0;
      for (i = 0; i < func->size; i++)
	{
	  instr = &func->control_tab[i];
	  switch (instr->type)
	    {
	      /* Nothing on this pass */
	    case CB:
	      break;
	    case BR:
	      fprintf (out,
		       " (b  %2i " ITintmaxformat ".000000 " ITintmaxformat
		       ".000000)\n", place_in_func, instr->weight,
		       instr->branched);
	      place_in_func++;
	      break;

	    case JMP:
	      fprintf (out, " (j  %2i " ITintmaxformat ".000000)\n",
		       place_in_func, instr->weight);
	      place_in_func++;
	      break;

	    case PJMP:
	      fprintf (out,
		       " (pj  %2i " ITintmaxformat ".000000 " ITintmaxformat
		       ".000000)\n", place_in_func, instr->weight,
		       instr->branched);
	      place_in_func++;
	      break;

	    case HASH:
	      fprintf (out, " (jrg %2i  " ITintmaxformat ".000000\n",
		       place_in_func, instr->weight);
	      print_condition_codes (out, instr);
	      fprintf (out, " )\n");
	      place_in_func++;
	      break;

	    case JSR:
	      break;

	    case RET:
	      break;

	    case MALLOC:
	      fprintf (out, " (malloc %2i " ITintmaxformat ".000000 %i)\n",
		       place_in_func, instr->weight,
		       (instr->mem_addr_struct->id)[0] + BASE_MALLOC_ID);
	      place_in_func++;
	      break;

	    case FREE:
	      place_in_func++;
	      break;

	      /* SER: for HCH MICRO '04 */
	    case LOAD:
	      fprintf (out, " (ld %2i " ITintmaxformat ".000000 %i %i %i %i\n",
		       place_in_func, instr->weight,
		       instr->mem_addr_struct->num_objs,
		       instr->mem_addr_struct->num_heap_objs,
		       instr->mem_addr_struct->stack_count,
		       instr->mem_addr_struct->num_missing_objs);
	      for (k = 0; k <= instr->mem_addr_struct->num_objs; k++)
		fprintf (out, " (%i %i)\n", (instr->mem_addr_struct->id)[k],
			 (instr->mem_addr_struct->count)[k]);
	      heap_obj = instr->mem_addr_struct->heap_obj;
	      for (k = 0; k < instr->mem_addr_struct->num_heap_objs; k++)
		{
		  fprintf (out, " (%i %i %i)\n",
			   heap_obj->id + BASE_HEAP_OBJ_ID,
			   heap_obj->malloc_id + BASE_MALLOC_ID,
			   heap_obj->count);
		  heap_obj = heap_obj->next;
		}
	      missing_obj = instr->mem_addr_struct->missing_obj;
	      for (k = 0; k < instr->mem_addr_struct->num_missing_objs; k++)
		{
		  fprintf (out, " (%i %i)\n",
			   missing_obj->id, missing_obj->count);
		  missing_obj = missing_obj->next;
		}
	      fprintf (out, ")\n");
	      place_in_func++;
	      break;

	    case LOAD_ALLOC:
	      place_in_func++;
	      break;
#if 0
	    case LOAD_STACK:
	      place_in_func++;
	      break;
#endif
	    case STORE:
	      fprintf (out, " (st %2i " ITintmaxformat ".000000 %i %i %i %i\n",
		       place_in_func, instr->weight,
		       instr->mem_addr_struct->num_objs,
		       instr->mem_addr_struct->num_heap_objs,
		       instr->mem_addr_struct->stack_count,
		       instr->mem_addr_struct->num_missing_objs);
	      for (k = 0; k <= instr->mem_addr_struct->num_objs; k++)
		fprintf (out, " (%i %i)\n", (instr->mem_addr_struct->id)[k],
			 (instr->mem_addr_struct->count)[k]);
	      heap_obj = instr->mem_addr_struct->heap_obj;
	      for (k = 0; k < instr->mem_addr_struct->num_heap_objs; k++)
		{
		  fprintf (out, " (%i %i %i)\n",
			   heap_obj->id + BASE_HEAP_OBJ_ID,
			   heap_obj->malloc_id + BASE_MALLOC_ID,
			   heap_obj->count);
		  heap_obj = heap_obj->next;
		}
	      missing_obj = instr->mem_addr_struct->missing_obj;
	      for (k = 0; k < instr->mem_addr_struct->num_missing_objs; k++)
		{
		  fprintf (out, " (%i %i)\n",
			   missing_obj->id, missing_obj->count);
		  missing_obj = missing_obj->next;
		}
	      fprintf (out, ")\n");
	      place_in_func++;
	      break;

	    case STORE_ALLOC:
	      place_in_func++;
	      break;
#if 0
	    case STORE_STACK:
	      place_in_func++;
	      break;
#endif
	    default:
	      I_punt ("Unknown control type %i\n", instr->type);
	    }
	}

      fprintf (out, "(end %s)\n", func->name);
    }
  return;
}

static void
load_input (char *name)
{
  FILE *in;
  char in_buf[1025], name_buf[1025], asm_name_buf[1025], *buf;
  int size, max_cb, addr, num_jsrs, num_objs;
  int i, k, preheader_cb_id, line, num_parsed, jsr_num, cb_id;
  int hash_size, num_preheader_cbs, preheader_index;
  int is_inner_loop = 0, char_index = 0;
  Pfunc *func;
  Pcontrol *pcontrol;
  Ploop_info *loop_info;

  if ((in = fopen (name, "r")) == NULL)
    I_punt ("Could not open input file %s.\n", name);

  /* Create function hash table */
  hash_size = FUNC_HASH_SIZE * sizeof (Pfunc *);
  if ((func_hash_tab = (Pfunc **) malloc (hash_size)) == NULL)
    I_punt ("load_input: Out of memory");

  for (i = 0; i < FUNC_HASH_SIZE; i++)
    func_hash_tab[i] = NULL;

  line = 0;
  while (fgets (in_buf, 1024, in) != NULL)
    {
      line++;

      /* Strip leading and trailing whitespace and comments */
      buf = strip (in_buf);

      /* Skip comments and blank lines */
      if (*buf == 0)
	continue;

      /* Make sure line starts with 'begin' */
      if ((buf[0] != 'b') || (buf[1] != 'e') ||
	  (buf[2] != 'g') || (buf[3] != 'i') ||
	  (buf[4] != 'n') || (buf[5] != ' '))
	{
	  I_punt ("%s, line %i: 'begin' expected not '%s'", name, line, buf);
	}

      /* Read in function, addr, size and max cb */
      num_parsed = sscanf (buf, "begin %s %s  Addr: 0x%8x  Size: %8d  "
			   "Max_Cb: %8d  JSRs: %8d",
			   name_buf, asm_name_buf, &addr, &size, &max_cb,
			   &num_jsrs);
      if (num_parsed != 6)
	I_punt ("%s line %i: Parsed only %d fields of '%s'", program_name,
		line, num_parsed, buf);

      func = create_func (name_buf, asm_name_buf, addr, size, max_cb,
			  num_jsrs);
      func_count++;

      /* debug */
      /*
       * fprintf (stderr, "---> %s %x %d %d %d\n", name_buf, addr, size,
       *          max_cb, num_jsrs);
       */
      jsr_num = 0;

      cb_id = 0;	    /* The cb id to mark each pcontrol struct with */
      for (i = 0; i < size; i++)
	{
	  if (fgets (in_buf, 1024, in) == NULL)
	    I_punt ("%s line %i: Unexpected end of file", program_name, line);
	  line++;

	  /* Strip leading and trailing whitespace and comments */
	  buf = strip (in_buf);

	  /* Get pointer to control structure for ease of use */
	  pcontrol = &func->control_tab[i];

	  /* Parse based on first character */
	  switch (buf[0])
	    {
	    case 'c':
	      if (buf[2] == ' ')
		{
		  num_parsed = sscanf (buf, "cb %d", &pcontrol->id);
		  if (num_parsed != 1)
		    I_punt ("%s line %i: Error parsing '%s'", program_name,
			    line, buf);
		  pcontrol->type = CB;
		}
	      else
		{
		  if (L_do_buf_info)
		    {
		      num_parsed = sscanf (buf, "cb_loop_header %d %d %d",
					   &pcontrol->id, &num_preheader_cbs,
					   &is_inner_loop);
		      if (num_parsed != 3)
			I_punt ("%s line %i: Error parsing '%s'",
				program_name, line, buf);
		    }
		  else
		    {
		      num_parsed = sscanf (buf, "cb_loop_header %d %d",
					   &pcontrol->id, &num_preheader_cbs);
		      if (num_parsed != 2)
			I_punt ("%s line %i: Error parsing '%s'",
				program_name, line, buf);
		    }
		  pcontrol->type = CB;
		  pcontrol->flags |= LOOP_HEADER;

		  /* Allocate loop info structure */
		  loop_info = (Ploop_info *) malloc (sizeof (Ploop_info));
		  if (loop_info == NULL)
		    I_punt ("%s line %i: Out of memory", program_name, line);

		  /* 
		   * Initialize loop info 
		   */

		  /* Allocate array of (max_cb + 1) chars, and
		   * initialize them all to 0.  
		   * Then read preheader cb id's from encoded file
		   * and mark those id's in array with 1.
		   */
		  /* Allocate array of preheader cb id's and read 
		   * them int from encoded file.
		   */
		  loop_info->loop_preheader =
		    (char *) malloc (sizeof (char) * (max_cb + 1));
		  if (loop_info->loop_preheader == NULL)
		    I_punt ("%s line %i: Out of memory", program_name, line);
		  for (k = 0; k <= max_cb; k++)
		    loop_info->loop_preheader[k] = 0;

		  for (preheader_index = 0;
		       preheader_index < num_preheader_cbs; preheader_index++)
		    {
		      if (fgets (in_buf, 1024, in) == NULL)
			I_punt ("%s line %i: Unexpected end of file",
				program_name, line);
		      line++;

		      /* Strip leading and trailing whitespace and comments */
		      buf = strip (in_buf);

		      num_parsed = sscanf (buf, "_preheader_cb %d",
					   &preheader_cb_id);
		      if (num_parsed != 1)
			I_punt ("%s line %i: Error parsing '%s'",
				program_name, line, buf);
		      loop_info->loop_preheader[preheader_cb_id] = 1;
		    }

		  loop_info->cur_iter_count = 0;
		  loop_info->cur_inner_iter_count = 0;
		  loop_info->cb = pcontrol;
		  loop_info->func = func;
		  loop_info->entry_count = 0;	/* Number of iter entries */
		  if (L_do_buf_info)
		    loop_info->is_inner = is_inner_loop;

		  /* Initialize loop iteration hash table */
		  for (k = 0; k < ITER_HASH_SIZE; k++)
		    loop_info->iter_hash[k] = NULL;
		  /*
		   * Array of sorted iteration entries.
		   * Used only just before print time.
		   */
		  loop_info->sorted_iters = NULL;

		  pcontrol->ext.loop_info = loop_info;
		}

	      if (pcontrol->id > func->max_cb)
		I_punt ("%s line %i: Cb out of bounds '%s'", buf);

	      func->cb_tab[pcontrol->id] = i;
	      cb_id = pcontrol->id;
	      break;

	    case 'b':
	      if (sscanf (buf, "br %i", &pcontrol->bb_size) == 1)
		pcontrol->type = BR;
	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	    case 'f':
	      if (sscanf (buf, "free %i", &pcontrol->bb_size) == 1)
		pcontrol->type = FREE;
	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	    case 'h':
	      if (sscanf (buf, "hash %i", &pcontrol->bb_size) == 1)
		pcontrol->type = HASH;
	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	    case 'j':
	      if (buf[1] == 'm')
		{
		  if (sscanf (buf, "jmp %i", &pcontrol->bb_size) == 1)
		    pcontrol->type = JMP;
		  else
		    I_punt ("%s line %i: Error parsing '%s'", program_name,
			    line, buf);
		}
	      else if (sscanf (buf, "jsr %i", &pcontrol->bb_size) == 1)
		{
		  jsr_num++;
		  pcontrol->type = JSR;
		  func->jsr_tab[jsr_num] = i;
		}

	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	      /* SER: For HCH MICRO '04 */
	    case 'l':
	      if (sscanf (buf, "ld %i %i", &pcontrol->bb_size, &num_objs) == 2)
		{
		  int obj_count = 0;

		  pcontrol->type = LOAD;
		  /* Initialize memory tables & stats. */
		  if ((pcontrol->mem_addr_struct =
		       (Pmem_addr *) malloc (sizeof (Pmem_addr))) == NULL)
		    I_punt ("load_input: Out of memory");
		  pcontrol->mem_addr_struct->num_objs = num_objs;
		  pcontrol->mem_addr_struct->num_heap_objs = 0;
		  pcontrol->mem_addr_struct->heap_obj = NULL;
		  pcontrol->mem_addr_struct->num_missing_objs = 0;
		  pcontrol->mem_addr_struct->missing_obj = NULL;
		  pcontrol->mem_addr_struct->stack_count = 0;

		  pcontrol->mem_addr_struct->id =
		    calloc ((num_objs+1), sizeof (int));
		  pcontrol->mem_addr_struct->count =
		    calloc ((num_objs+1), sizeof (int));

		  if (num_objs == 0)
		    break;

		  /* IDs are in buf, but need to be parsed out. */
		  for (k = 0; k < num_objs; k++)
		    {
		      int obj_id;
		      if ((k == 0) || (obj_count == OBJS_PER_LINE))
			{
			  if (fgets (in_buf, 1024, in) == NULL)
			    I_punt ("%s line %i: Unexpected end of file",
				    program_name, line);
			  line++;
			  buf = strip (in_buf);
			  char_index = obj_count = 0;
			}
		      else if (obj_count != 0)
			while (*(char *)(buf+char_index++) != ' ')
			  { /* Find preceeding space */ }

		      sscanf (buf + char_index, "%i", &obj_id);
		      (pcontrol->mem_addr_struct->id)[k] = obj_id;
#if 0
		      fprintf (stderr, "Load obj id %i into mem loc 0x%x\n",
			       (pcontrol->mem_addr_struct->id)[k],
			       &((pcontrol->mem_addr_struct->id)[k]));
#endif
		      if (obj_id < MAX_GLOB_OBJS)
		        static_glob_obj_info[obj_id][0]++;
		      obj_count++;
		    }
		}
	      else if (sscanf (buf, "ld_alloc %i", &pcontrol->bb_size) == 1)
		{
		  pcontrol->type = LOAD_ALLOC;
		}
#if 0
	      else if (sscanf (buf, "ld_stack %i", &pcontrol->bb_size) == 1)
		{
		  pcontrol->type = LOAD_STACK;
		}
#endif
	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	    case 'm':
	      if (sscanf (buf, "malloc %i", &pcontrol->bb_size) == 1)
		{
		  pcontrol->type = MALLOC;
		  if ((pcontrol->mem_addr_struct =
		       (Pmem_addr *) malloc (sizeof (Pmem_addr))) == NULL)
		    I_punt ("load_input: Out of memory");
		  pcontrol->mem_addr_struct->id = malloc (sizeof(int));
		  (pcontrol->mem_addr_struct->id)[0] = ++num_malloc;
#if 0
		  fprintf (stderr, "Initializing malloc location %d\n",
			   (pcontrol->mem_addr_struct->id)[0]);
#endif
		  if (num_malloc ==  MAX_MALLOCS)
		    I_punt ("Need to increase MAX_MALLOCS");
		}
	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	    case 'p':
	      if (sscanf (buf, "pjmp %i", &pcontrol->bb_size) == 1)
		pcontrol->type = PJMP;

	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;


	    case 'r':
	      if (sscanf (buf, "ret %i", &pcontrol->bb_size) == 1)
		pcontrol->type = RET;

	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	    case 's':
	      if (sscanf (buf, "st %i %i", &pcontrol->bb_size, &num_objs) == 2)
		{
		  int obj_count = 0;

		  pcontrol->type = STORE;
		  /* Initialize memory tables & stats. */
		  if ((pcontrol->mem_addr_struct =
		       (Pmem_addr *) malloc (sizeof (Pmem_addr))) == NULL)
		    I_punt ("load_input: Out of memory");
		  pcontrol->mem_addr_struct->num_objs = num_objs;
		  pcontrol->mem_addr_struct->num_heap_objs = 0;
		  pcontrol->mem_addr_struct->heap_obj = NULL;
		  pcontrol->mem_addr_struct->num_missing_objs = 0;
		  pcontrol->mem_addr_struct->missing_obj = NULL;
		  pcontrol->mem_addr_struct->stack_count = 0;

		  pcontrol->mem_addr_struct->id =
		    calloc ((num_objs+1), sizeof (int));
		  pcontrol->mem_addr_struct->count =
		    calloc ((num_objs+1), sizeof (int));

		  if (num_objs == 0)
		    break;

		  /* IDs are in buf, but need to be parsed out. */
		  for (k = 0; k < num_objs; k++)
		    {
		      int obj_id;
		      if ((k == 0) || (obj_count == OBJS_PER_LINE))
			{
			  if (fgets (in_buf, 1024, in) == NULL)
			    I_punt ("%s line %i: Unexpected end of file",
				    program_name, line);
			  line++;
			  buf = strip (in_buf);
			  char_index = obj_count = 0;
			}
		      else if (obj_count != 0)
			while (*(char *)(buf+char_index++) != ' ')
			  { /* Find preceeding space */ }

		      sscanf (buf + char_index, "%i", &obj_id);
		      (pcontrol->mem_addr_struct->id)[k] = obj_id;
#if 0
		      fprintf (stderr, "Store obj id %i into mem loc 0x%x\n",
			       (pcontrol->mem_addr_struct->id)[k],
			       &((pcontrol->mem_addr_struct->id)[k]));
#endif
		      if (obj_id < MAX_GLOB_OBJS)
		        static_glob_obj_info[obj_id][1]++;
		      obj_count++;
		    }
		}
	      else if (sscanf (buf, "st_alloc %i", &pcontrol->bb_size) == 1)
		{
		  pcontrol->type = STORE_ALLOC;
		}
#if 0
	      else if (sscanf (buf, "st_stack %i", &pcontrol->bb_size) == 1)
		{
		  pcontrol->type = STORE_STACK;
		}
#endif
	      else
		I_punt ("%s line %i: Error parsing '%s'", program_name,
			line, buf);
	      break;

	    default:
	      I_punt ("%s line %i: unknown control instr '%s'",
		      program_name, line, buf);
	    }

	  /* Set the cb_id of each pcontrol structure */
	  pcontrol->cb_id = cb_id;
	}

      if (fgets (in_buf, 1024, in) == NULL)
	I_punt ("%s line %i: Unexpected end of file", program_name, line);
      line++;

      /* Strip leading and trailing whitespace and comments */
      buf = strip (in_buf);

      if ((buf[0] != 'e') || (buf[1] != 'n') || (buf[2] != 'd'))
	{
	  I_punt ("%s line %i: end %s expected, not '%s'\n",
		  program_name, line, func->name, buf);
	}
    }
}

static Pfunc *
create_func (char *name, char *asm_name, int addr,
	     int size, int max_cb, int num_jsrs)
{
  Pfunc *func;
  Pcontrol *pcontrol;
  int name_size, asm_name_size, cb_size, control_size, jsr_size, btb_size;
  int i;

  if ((func = (Pfunc *) malloc (sizeof (Pfunc))) == NULL)
    I_punt ("create_func: out of memory");

  name_size = strlen (name) + 1;
  asm_name_size = strlen (asm_name) + 1;
  jsr_size = sizeof (int) * (1 + num_jsrs);
  cb_size = sizeof (int) * (1 + max_cb);
  control_size = sizeof (Pcontrol) * (1 + size);

  if (((func->name = (char *) malloc (name_size)) == NULL) ||
      ((func->asm_name = (char *) malloc (asm_name_size)) == NULL) ||
      ((func->cb_tab = (int *) malloc (cb_size)) == NULL) ||
      ((func->jsr_tab = (int *) malloc (jsr_size)) == NULL) ||
      ((func->control_tab = (Pcontrol *) malloc (control_size)) == NULL))
    I_punt ("create_func: out of memory");

  strcpy (func->name, name);
  strcpy (func->asm_name, asm_name);
  func->addr = addr;
  func->size = size;
  func->max_cb = max_cb;
  func->num_jsrs = num_jsrs;
  func->weight = 0;

  for (i = 0; i <= max_cb; i++)
    func->cb_tab[i] = -1;

  for (i = 0; i <= num_jsrs; i++)
    func->jsr_tab[i] = -1;

  for (i = 0; i <= size; i++)
    {
      pcontrol = &func->control_tab[i];

      pcontrol->type = 0;
      pcontrol->id = 0;
      pcontrol->weight = 0;
      pcontrol->branched = 0;
      pcontrol->bb_size = 0;
      pcontrol->flags = 0;
      pcontrol->cb_id = 0;

      /* Initialize union, using first pointer */
      pcontrol->ext.cnode_head = NULL;
    }

  /* if profiling a BTB, create BTB tables */
  if (profile_BTB)
    {
      btb_size = sizeof (PBTB) * (1 + size);
      if ((func->BTB_tab = (PBTB *) malloc (btb_size)) == NULL)
	I_punt ("create_func: out of memory");

      for (i = 0; i <= size; i++)
	{
	  func->BTB_tab[i].flags = 0;
	  func->BTB_tab[i].counter = 0;
	  func->BTB_tab[i].target = 0;
	  func->BTB_tab[i].mispred_count = 0;
	}
    }

  /* Otherwise, Set BTB fields to NULL */
  else
    {
      func->BTB_tab = NULL;
    }

  /* Add to linked list of functions */
  func->next_func = func_list;
  func_list = func;

  return (func);
}

/* 
 * Returns next word in the trace (byte-swapped if necessary).
 * Reads buffer in a way that guarentees that there is always
 * at least one valid int in it (so above peek_ptrace will work)
 */
static int
get_ptrace (Ptrace * ptrace)
{
  int trace_word, read_cnt;
#ifdef WIN32
  char *msg_buf;
#endif

  trace_word = *ptrace->ptr;
  ptrace->ptr++;
  ptrace->trace_count += sizeof (int);

  /* Is the buffer now empty ? */
  if (ptrace->ptr >= ptrace->end)
    {
      /* Should never happen (but often does) */
      if (ptrace->ptr > ptrace->end)
	I_punt ("get_ptrace: Read past end of trace file");

#ifdef WIN32
      if (!ReadFile (ptrace->source_fd, ptrace->buf,
		     sizeof (int) * TRACE_SIZE, &read_cnt, NULL))
	{
	  /* Treat broken pipe as end of file reached */
	  if (GetLastError () == ERROR_BROKEN_PIPE)
	    read_cnt = 0;
	  else
	    {
	      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,	// source and processing options
			     NULL, GetLastError (), 0, (LPTSTR) & msg_buf, 1,
			     NULL);

	      I_punt ("get_ptrace: Could not read trace file: %s", msg_buf);
	      LocalFree (msg_buf);	// never called
	    }
	}
#else
      read_cnt = read (ptrace->source_fd, ptrace->buf,
		       sizeof (int) * TRACE_SIZE);
#endif
      /* Set end of buffer based on read_cnt (may be 0) */
      ptrace->ptr = &ptrace->buf[0];
      ptrace->end = &ptrace->buf[read_cnt >> 2];

      /* If need to reverse byte order, do it now */
      if (ptrace_byte_order_reversed)
	Ptrace_reverse_byte_order (ptrace->ptr, ptrace->end);
    }

  return (trace_word);
}


/* Reverses the byte order in an array of ints */
static void
Ptrace_reverse_byte_order (int *ptr, int *end)
{
  while (ptr < end)
    {
      *ptr = SWAP_BYTES (*ptr);
      ptr++;
    }
}

/*
 * BTB Modeling
 * ----------------------------------------------------------------------
 */

/*
 * Profile BTB prediction accurracy.
 */
static void
P_model_BTB (PBTB * btb_entry, int actual_direction, int actual_target)
{
  int BTB_direction, BTB_target;

  if (BTB_model == BTB_MODEL_COUNTER)
    {
      /* Make prediction */
      if (btb_entry->counter >= 2)
	BTB_direction = 1;
      else
	BTB_direction = 0;
      BTB_target = btb_entry->target;

      /* If was not in btb before */
      if (!(btb_entry->flags & IN_BTB))
	{
	  /* Set to hard taken or hard not taken */
	  if (actual_direction)
	    btb_entry->counter = 3;
	  else
	    btb_entry->counter = 0;

	  /* Flag that now in BTB */
	  btb_entry->flags |= IN_BTB;
	}
      /* If was in BTB, update counter */
      else
	{
	  if (actual_direction)
	    {
	      if (btb_entry->counter == 0)
		btb_entry->counter = 1;
	      else
		btb_entry->counter = 3;
	    }
	  else
	    {
	      if (btb_entry->counter == 3)
		btb_entry->counter = 2;
	      else
		btb_entry->counter = 0;
	    }
	}

      /* Update target if branch was taken */
      if (actual_direction)
	btb_entry->target = actual_target;

      /* Update BTB stats,
       * Misprediction if directions not correct, or if taken and
       * target incorrect
       */
      if ((actual_direction != BTB_direction) ||
	  (actual_direction && (actual_target != BTB_target)))
	btb_entry->mispred_count++;
    }
  else if (BTB_model == BTB_MODEL_2_LEVEL)
    {
      I_punt ("P_model_BTB: Two level BTB not implemented yet.  Sorry.");
    }
  else
    {
      I_punt ("P_model_BTB: Unknown BTB_model %i.", BTB_model);
    }
  return;
}

static void
print_BTB_output (char *name)
{
  FILE *out;
  Pfunc *func;
  Pcontrol *instr;
  int branch_count;
  PBTB *btb;
  char code_char = '\0';
  int i;

  if ((out = fopen (name, "w")) == NULL)
    I_punt ("Could not open BTB output file %s.\n", name);

  for (func = func_list; func != NULL; func = func->next_func)
    {
      branch_count = 0;
      for (i = 0; i < func->size; i++)
	{
	  instr = &func->control_tab[i];

	  if (instr->type != CB)
	    branch_count++;
	}
      fprintf (out, "Begin %s %i %s\n", func->name, branch_count,
	       BTB_model_name);

      /* Print mispred_stats out stats for all branches */
      for (i = 0; i < func->size; i++)
	{
	  instr = &func->control_tab[i];
	  btb = &func->BTB_tab[i];

	  switch (instr->type)
	    {
	      /* Nothing on this pass */
	    case CB:
	      break;
	    case BR:
	      code_char = 'b';
	      break;

	    case JMP:
	      code_char = 'j';
	      break;

	    case PJMP:
	      code_char = 'p';
	      break;

	    case HASH:
	      code_char = 'h';
	      break;

	    case JSR:
	      code_char = 'f';
	      break;

	    case RET:
	      code_char = 'r';
	      break;

	    default:
	      I_punt ("Unknown control type %i\n", instr->type);
	    }
	  if (instr->type != CB)
	    fprintf (out, "%c %i\n", code_char, btb->mispred_count);
	}

      fprintf (out, "End %s\n\n", func->name);
    }
}

