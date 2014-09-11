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
 *  File:  s_load_code.c
 *
 *  Description:  Routines to load in the encoded code file
 *
 *  Creation Date :  July, 1993
 *
 *  Author:  John Gyllenhaal, Roger A. Bringmann
 *
 *  Revisions:
 *
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

#include "s_main.h"
#include <Lcode/l_main.h>

/* Static variables */
static char *encoded_file_name = 0;
static S_Operand *operand_hash[OPERAND_HASH_SIZE];


/* Global variables */
S_Fn	*head_fn;
S_Fn	*tail_fn;
S_Fn 	*fn_hash[FN_HASH_SIZE];	     /* Indexed using hash of fn address */
S_Oper	**oper_tab;		     /* Indexed by pc */
S_Operand **operand_tab;	     /* Indexed operand id.  May be negative */
S_Opc_Info *opc_info_tab;	     /* Indexed by opc */
int	S_max_dest_operand = 0;
int	S_max_src_operand = 0;
int	S_max_pred_operand = 0;
int	S_first_dest;                /* Index of first dest operand */
int     S_last_dest;                 /* Index of last dest operand */
int     S_first_src;                 /* Index of first src operand */
int     S_last_src;                  /* Index of last src operand */
int     S_first_pred;                /* Index of first pred operand */
int     S_last_pred;                 /* Index of last src operand */
int	S_min_const_operand = 0;     /* This will be negative */
int	S_max_register_operand = 0;  /* This will be positive */
int	S_max_pc = 0;
int	S_max_packet_id = 0;
int	S_max_slot = -1;
int	S_max_opc = 0;
int     max_fn_loop = 0;             /* HCH: mem tracing: renumber loop ids */
int     prev_max_loop = 0;

static short 
S_assign_operand_id (int hash_value, int flags, char * string)
{
  S_Operand *soperand;
  int hash_index;
  int len;
  
  hash_index = hash_value & (OPERAND_HASH_SIZE -1);
  
  for (soperand = operand_hash[hash_index]; soperand != NULL;
       soperand = soperand->next_hash)
    {
      /* Search for match operand, use hash value to speed search */
      if ((soperand->hash_value == hash_value) &&
	  (strcmp (soperand->string, string) == 0))
	break;
    }
  
  /* If not found create a new entry */
  if (soperand == NULL)
    {
      soperand = (S_Operand *) L_alloc (S_Operand_pool);
      /* Registers get positive ids, constants get negative */
      if ((flags & REGISTER_OPERAND))
	{
	  S_max_register_operand++;
	  soperand->id = S_max_register_operand;
	  if (soperand->id > MAX_POSITIVE_SHORT)
	    S_punt ("operand id %i does not fit in short", soperand->id);
	}
      else
	{
	  S_min_const_operand--;
	  soperand->id = S_min_const_operand;
	  if (soperand->id < MIN_NEGATIVE_SHORT)
	    S_punt ("operand id %i does not fit in short", soperand->id);
	}
      soperand->hash_value = hash_value;
      soperand->flags = flags;
      len = strlen (string);
      soperand->string = S_alloc_string (len + 1);
      strcpy (soperand->string, string);
      
      /* Add to hash table */
      soperand->next_hash = operand_hash[hash_index];
      operand_hash[hash_index] = soperand;
    }
  
  /* Return id this operand maps to */
  return (soperand->id);
}

void 
S_set_real_latency(S_Opc_Info *info, char virtual_latency,
		   char *real_latency)
{
  int scale_factor, delta_factor;
  
  /* Set the real latency based on the type of operation
   * and the scale and delta for that type of operation
   */
  switch (info->opc_type)
    {
    case LOAD_OPC:
    case STORE_OPC:
    case PREFETCH_OPC:
    case MEM_COPY_OPC:
    case MEM_COPY_BACK_OPC:
    case MEM_COPY_CHECK_OPC:
    case MEM_COPY_RESET_OPC:
    case MEM_COPY_TAG_OPC:
      scale_factor = S_memory_latency_scale_factor;
      delta_factor = S_memory_latency_delta_factor;
      break;
      
    case MOVE_OPC:
      scale_factor = S_move_latency_scale_factor;
      delta_factor = S_move_latency_delta_factor;
      break;
      
    case IALU_OPC:
      scale_factor = S_ialu_latency_scale_factor;
      delta_factor = S_ialu_latency_delta_factor;
      break;
      
    case FALU_OPC:
      scale_factor = S_falu_latency_scale_factor;
      delta_factor = S_falu_latency_delta_factor;
      break;
      
    case CBR_OPC:
    case JMP_OPC:
    case JRG_OPC:
    case JSR_OPC:
    case RTS_OPC:
      /* As of 10/18/96 cannot scale branch latencies.  
       * (Latencies are now dest operand based and cbr's don't have
       *  necessarily have dests.  (Default to latency 1).
       * 
       * Add fetch stages to model larger branch latencies.
       */
      scale_factor = 1;
      delta_factor = 0;
      break;
      
    default:
      scale_factor = S_default_latency_scale_factor;
      delta_factor = S_default_latency_delta_factor;
      break;
    }
  
  /* Scale the virtual latency to get the real latency */
  *real_latency = (virtual_latency * scale_factor) + delta_factor;
  
  /* Make sure nothing has less than 1 cycle real latency */
  if (*real_latency < 1)
    {
      fprintf (stderr,
	       "Error: opc %i ('%s', type '%s') has real latency %i.\n",
	       info->opc, info->name, info->opc_type_name, 
	       *real_latency);
      fprintf (stderr,
	       "Set using (virtual latency (%i) * scale_factor (%i)) ",
	       virtual_latency, scale_factor);
      fprintf (stderr,
	       "+ delta_factor (%i).\n",
	       delta_factor);
      S_punt ("Terminating: cannot handle less than 1 cycle real latency.");
    }
}


S_Loop* 
S_find_loop(S_Fn *fn, int loop_num)
{
  S_Loop *loop = NULL; 

  /* cb is not part of a loop */
  if (loop_num == -1)  {
    return NULL;
  }
  List_start(fn->loops);
  while((loop = (S_Loop*)List_next(fn->loops)))
    {
      if(loop_num == loop->loop_id)
	break;
    }
  if(!loop) {
    loop = malloc(sizeof(S_Loop));
    loop->loop_id = loop_num;
    loop->instance = 0;
    loop->iter = 0;
    fn->loops = List_insert_last(fn->loops, loop);
  }

  return loop;
}

/* Loads the encoded lcode file into memory and builds the hash table
 * of functions by address (in executable).
 */
void 
S_load_code (char *file_name)
{
  S_Fn	*fn, *search_fn;
  S_Cb	*cb, *last_cb;
  S_Oper	*op;
  short	*operand = NULL;
  FILE *in;
  char lbuf[MAX_LINE_SIZE];
  char sbuf[MAX_LINE_SIZE];
  char abuf[MAX_LINE_SIZE];
  char region_name[MAX_LINE_SIZE];
  int function_count;
  int version;
  int num_operands, num_ops, num_cbs, max_cb_id, max_jsr_id;
  int op_size, cb_size, operand_size, operand_map_size, tab_size, jsr_size;
  short *operand_map = NULL;
  int hash_value, operand_flags, i, j, len, raw_id;
  int max_dest_operand, max_src_operand, max_pred_operand, total_operands;
  int sched_info_avail;
  int cb_id, jsr_id, start, last_start;
  int pc, index, dest_index, index2;
  int opc_type;
  S_Operand **temp_operand_tab_ptr;
  S_Operand *operand_ptr;
  static int warned_once = 0;
  int multiple_fn_defs;
  S_Opc_Info *info;
  int lat1, lat2, lat3;
  
  int loop_num;   /* ID loop op is from */
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  L_Attr *attr;
#endif
  int num_preheaders;  /* num preheader cbs to expect in next line*/

  /* for debug messages */
  encoded_file_name = file_name;
  
  /* Create the program stats stucture, need to patch up NULL pnode
   * pointer later.
   */
  S_program_stats = S_create_stats ("Untagged regions");
  
  /* Initialize operand hash */
  for (i=0; i < OPERAND_HASH_SIZE; i++)
    operand_hash[i] = NULL;
  
  /* Start pc at S_program_start_address/4 (assumes 4 byte instruction). 
   * This is so instructions can be aligned in cache.
   * May also want to align functions.
   */
  /*  S_program_start_addr must be >= 4 (address 0 is reserved) */
  if (S_program_start_addr < 4)
    S_punt ("S_load_code: program_start_addr (%i) must be >= 4", 
	    S_program_start_addr);
  
  S_program_start_pc = (S_program_start_addr >> 2);
  head_fn = NULL;
  tail_fn = NULL;
  pc = S_program_start_pc - 1;  /* pc incremented before use */
  S_max_dest_operand = -1;
  S_max_src_operand = -1;
  S_max_pred_operand = -1;
  S_min_const_operand = 0; /* This will be a negative number */
  S_max_register_operand = 0;
  
  /* Create Null operand */
  operand_ptr = (S_Operand *) L_alloc (S_Operand_pool);
  operand_ptr->id = 0;
  operand_ptr->hash_value = 0;
  operand_ptr->string = strdup("");
  operand_ptr->next_hash = operand_hash[0];
  operand_hash[0] = operand_ptr;
  
  function_count = 0;
  
  if ((in = fopen (file_name, "rt")) == NULL)
    S_punt ("Unable to open encoded lcode file '%s'.", file_name);
  
  /* While there are functions in the file, read them in */
  while (fgets (lbuf, MAX_LINE_SIZE, in) != NULL)
    {
      /* Make sure file is in correct format */
      if (strcmp (lbuf, "Lcode encoded for simulation.\n") != 0)
	{
	  if (function_count == 0)
	    S_punt ("'%s' is not an encoded lcode file.", file_name);
	  else
	    S_punt ("'%s' is corrupted.",file_name);
	}
      
      /* Read version number and make sure it is correct */
      if (fscanf (in, "Version %d\n", &version) != 1)
	S_punt ("Error reading version number of '%s'.", file_name);
      
      /* Warn if using old version (warn once) */
      if (version == 2)
	{
	  if (S_max_dest_operand == -1)
	    {
	      fprintf(stderr, 
		      "Converting '%s' (no sched info) to new format.\n", 
		      file_name);
	    }
	}
      /* Quietly convert old versions to new format */
      else if ((version != 3) && (version != 4) &&
	       (version != 5) && (version != 6) &&
	       (version != READER_VERSION) &&
	       (version != OBJTR_VERSION))
	{
	  S_punt ("'%s' in format version %i, expect version %i or %i.",
		  file_name, version, READER_VERSION, OBJTR_VERSION);
	}
      
      /* Get number of operands, ops, and cbs */
      if (version == 2)
	{
	  if (fscanf (in, "%x %x %x %x %x %x %x %x\n", &max_dest_operand,
		      &max_src_operand, &max_pred_operand,
		      &num_operands, &num_ops, &num_cbs, &max_cb_id, 
		      &max_jsr_id) != 8)
	    S_punt ("Parse error loading '%s'.", file_name);
	  sched_info_avail = 0;
	}
      else
	{
	  if (fscanf (in, "%x %x %x %x %x %x %x %x %x\n", &max_dest_operand,
		      &max_src_operand, &max_pred_operand,
		      &num_operands, &num_ops, &num_cbs, &max_cb_id, 
		      &max_jsr_id, &sched_info_avail) != 9)
	    S_punt ("Parse error loading '%s'.", file_name);
	}
      
      /* If first function, set whether sched info avail */
      if (S_max_dest_operand == -1)
	S_sched_info_avail = sched_info_avail;
      
      /* Otherwise, make sure consistant, if not, warn and
       * assume no scheduling info available.
       */
      else if (S_sched_info_avail != sched_info_avail)
	{
	  if (!warned_once)
	    {
	      fprintf (stderr, 
		       "Warning: Mixing scheduled and unscheduled functions in %s.\n",
		       file_name);  
	      warned_once = 1;
	    }
	  S_sched_info_avail = 0;
	}
      
      
      /* If first function, set number of operands */
      if (S_max_dest_operand == -1)
	{
	  S_max_dest_operand = max_dest_operand;
	  S_max_src_operand = max_src_operand;
	  S_max_pred_operand = max_pred_operand;
	  
	  /* Calculate operand indexes */
	  S_first_dest = 0;
	  S_last_dest = S_first_dest + S_max_dest_operand -1;
	  S_first_src = S_last_dest + 1;
	  S_last_src = S_first_src + S_max_src_operand - 1;
	  S_first_pred = S_last_src +1;
	  S_last_pred = S_first_pred + S_max_pred_operand -1;
	  
	  /* Make sure have enough room in latency array -JCG 8/99 */
	  if (S_last_dest >= MAX_LAT)
            {
	      S_punt ("Need to increase value of MAX_LAT in s_main.h:\n"
		      "Set to %i but needs to be at least %i.\n",
		      MAX_LAT, S_last_dest + 1);
            }
	}
      
      /* Otherwise, make sure all functions match */
      else if ((S_max_dest_operand != max_dest_operand) ||
	       (S_max_src_operand != max_src_operand) ||
	       (S_max_pred_operand != max_pred_operand))
	{
	  S_punt ("Number of operands corrupted in '%s'.", 
		  file_name);
	}
      
      
      total_operands = S_max_dest_operand + S_max_src_operand +
	S_max_pred_operand;
      
      /* Allocate memory for function */
      op_size = sizeof (S_Oper) * num_ops;
      cb_size = sizeof (S_Cb) * (max_cb_id + 1);
      jsr_size = sizeof (int) * (max_jsr_id + 1);
      operand_size = sizeof (short) * total_operands * num_ops;
      operand_map_size = sizeof (short) * (num_operands + 1);
      
      if (((fn = (S_Fn *) malloc (sizeof (S_Fn))) == NULL) ||
	  ((fn->op = (S_Oper *) malloc (op_size)) == NULL) ||
	  ((fn->cb = (S_Cb *) malloc (cb_size)) == NULL) ||
	  ((fn->jsr_tab = (int *) malloc (jsr_size)) == NULL) ||
	  ((operand = (short *) malloc (operand_size)) == NULL) ||
	  ((operand_map = (short *) malloc (operand_map_size)) == NULL))
	S_punt ("S_load_code: Out of memory");
      
      /* Get function name */
      if (version <= 3)
	{
	  if (fscanf (in, "Function %s\n\n", sbuf) != 1)
	    S_punt ("Error parsing '%s'.", file_name);
	  /* For old version, lcode name is assembly name without 
	   * leading _ */
	  strcpy (abuf, &sbuf[1]);
	}
      else
	{
	  if (fscanf (in, "Function %s %s\n\n", sbuf, abuf) != 2)
	    S_punt ("Error parsing function name in '%s'.", file_name);
	  
	}
      len = strlen (sbuf);
      fn->name = S_alloc_string (len + 1);
      strcpy (fn->name, sbuf);
      
      len = strlen (abuf);
      fn->asm_name = S_alloc_string (len + 1);
      strcpy (fn->asm_name , abuf);
      fn->addr = 0;
      fn->op_count = num_ops;
      fn->max_cb = max_cb_id;
      fn->max_jsr_id = max_jsr_id;
      fn->guide_table = 0;
      fn->loops = NULL;
      max_fn_loop = 0;

      /* Read in operands and build operand map */
      operand_map[0] = 0;           /* 0 is always NULL */
      
      for (i=1; i <= num_operands; i++)
	{
	  if (fscanf (in, "%x %x\n", &hash_value, &operand_flags) != 2)
	    S_punt ("Error parsing '%s'.", file_name);
	  
	  if (fgets (sbuf, MAX_LINE_SIZE, in) == NULL)
	    S_punt ("Unexpected EOF in '%s'.", file_name);
	  
	  /* Remove \n from end of string */
	  len = strlen (sbuf);
	  sbuf[len-1] = 0;
	  
	  operand_map[i] = S_assign_operand_id (hash_value, operand_flags,
						sbuf);
	}
      fscanf (in, "\n");
      
      
      /* Add this function's operations to the number of ops statistic */
      S_operation_count += num_ops;
      
      /* Intialize jsr_id for jsr_tab.  (entry 0 not used) */
      jsr_id = 0;
      fn->jsr_tab[0] = -1;
      
      /* Read in ops and use operand map to set operand values */
      for (i=0; i < num_ops; i++)
	{
	  op = &fn->op[i];
	  pc++;
	  op->pc = pc;
	  
	  /* No playdoh flags possible until version 6 */
	  op->playdoh_flags = 0;
	  
	  if (version == 2)
	    {
	      if (fscanf (in, "%x %x %x %x %x", &op->lcode_id, &op->opc, 
			  &op->proc_opc, &op->flags, &op->br_target) != 5)
		S_punt ("Error parsing '%s'.", file_name);

	      op->cycle = 0;
	      op->slot = -1;
	      op->dep_id = 0;
	    }
	  else if (version <= 4)
	    {
	      if (fscanf (in, "%x %x %x %x %x %d %x", &op->lcode_id, 
			  &op->opc, &op->proc_opc, &op->flags, 
			  &op->cycle, &op->slot, &op->br_target) != 7)
		S_punt ("Error parsing '%s'.", file_name);
	      
	      op->dep_id = 0;
	    }
	  /* Read in dep id for each operation in version 5 */
	  else if (version == 5)
	    {
	      if (fscanf (in, "%x %x %x %x %x %d %x %x", &op->lcode_id, 
			  &op->opc, &op->proc_opc, &op->flags, 
			  &op->cycle, &op->slot, 
			  &op->dep_id, &op->br_target) != 8)
		S_punt ("Error parsing '%s'.", file_name);
	    }
	  /* Read in playdoh flags for version 6 */
	  else if (version == 6)
	    {
	      if (fscanf (in, "%x %x %x %x %x %x %d %x %x", &op->lcode_id, 
			  &op->opc, &op->proc_opc, &op->flags, 
			  &op->playdoh_flags, 
			  &op->cycle, &op->slot, 
			  &op->dep_id, &op->br_target) != 9)
		S_punt ("Error parsing '%s'.", file_name);
	    }
	  /* Read in latency for version 7 */
	  else if (version == 7)
	    {
	      if (fscanf (in, "%x %x %x %x %x %x %d %x %x %x %x %x", 
			  &op->lcode_id, 
			  &op->opc, &op->proc_opc, &op->flags, 
			  &op->playdoh_flags, 
			  &op->cycle, &op->slot, 
			  &lat1, &lat2, &lat3, 
			  &op->dep_id, &op->br_target) != 12)
		S_punt ("Error parsing '%s'.", file_name);
	      
	      op->virtual_latency[0] = (char) lat1;
	      op->virtual_latency[1] = (char) lat2;
	      op->virtual_latency[2] = (char) lat3;
	    }
	  /* Read in latency for version 8 */
	  else if (version == OBJTR_VERSION)
	    {
	      if (fscanf (in, "%x %x %x %x %x %x %d %x %x %x %x %x %x", 
			  &op->lcode_id, 
			  &op->opc, &op->proc_opc, &op->flags, 
			  &op->playdoh_flags, 
			  &op->cycle, &op->slot, 
			  &lat1, &lat2, &lat3, 
			  &op->dep_id, &op->br_target,
			  &loop_num) != 13)
		S_punt ("Error parsing '%s'.", file_name);

	      op->virtual_latency[0] = (char) lat1;
	      op->virtual_latency[1] = (char) lat2;
	      op->virtual_latency[2] = (char) lat3;
	      /* HCH: renumber loops so that ids are unique across functions */
	      if(loop_num > max_fn_loop)
		max_fn_loop = loop_num;
	      
	      /* Set loop that op originated from */
	      if(loop_num != -1)
		op->loop = S_find_loop(fn,(loop_num+prev_max_loop));
	      else
		op->loop = NULL;
	    }
	  else
	    {
	      S_punt ("Unsupported encoded file version %i\n", version);
	    }
	  
	  /* Make sure the op->opc is a known opcode */
	  if ((op->opc > S_max_opc) || (op->opc < 0) ||
	      (opc_info_tab[op->opc].name == NULL))
	    {
	      S_punt ("%s func %s op %i: opc %i undefined in '%s'.\n",
		      file_name, fn->name, op->lcode_id, op->opc, 
		      S_opc_info_file);
	    }
	  
	  /* TLJ 8/5/96 - if version >= 7 then calculate per-op latencies
	   * using the latency from the encoded source. Otherwise
	   * use those from S_Opc_Info entry. Also get from the opc info
	   * if this op was unscheduled (virtual latency = -1)
	   */
	  info = &opc_info_tab[op->opc];
	  if (version >= 7 && op->virtual_latency[0] != -1)
	    {
	      /* Set the real latency based on the type of operation
	       * and the scale and delta for that type of operation
	       *
	       * Don't scale 0 latency destinations.  The simulation
	       * cannot handle this and it is assume that the 
	       * operation does not have a register there.
	       */
	      if (op->virtual_latency[0] != 0)
		{
		  S_set_real_latency(info,op->virtual_latency[0],
				     &(op->real_latency[0]));
		}
	      else
		{
		  op->virtual_latency[0] = 0;
		}
	      
	      if (op->virtual_latency[1] != 0)
		{
		  S_set_real_latency(info,op->virtual_latency[1],
				     &(op->real_latency[1]));
		}
	      else
		{
		  op->real_latency[1] = 0;
		}
	      
	      if (op->virtual_latency[2] != 0)
		{
		  S_set_real_latency(info,op->virtual_latency[2],
				     &(op->real_latency[2]));
		}
	      else
		{
		  op->real_latency[2] = 0;
		}
	    }
	  else
	    {
	      op->real_latency[0] = info->real_latency;
	      op->real_latency[1] = info->real_latency;
	      op->real_latency[2] = info->real_latency;
	      op->virtual_latency[0] = info->virtual_latency;
	      op->virtual_latency[1] = info->virtual_latency;
	      op->virtual_latency[2] = info->virtual_latency;
	    }
	  op->adjust_real_latency = 0;
	  /* Keep trace of the max slot seen */
	  if (op->slot > S_max_slot)
	    S_max_slot = op->slot;
	  
	  /* For RISC machines, assume 4 byte instructions and that
	   * code starts at byte 512 of addresses space
	   */
	  op->instr_size = 4;	
	  op->instr_addr = (pc << 2);
	  
	  /* 
	   * Only for x86 trace generation do we have a compressed
	   * binary description for the instruction.
	   */
	  op->instr_desc = 0;
	  
	  /* Get array of operands from allocated block of operands */
	  op->operand = operand;
	  operand += total_operands;
	  
	  /* 
	   * Read in operand indexes and translate them with the
	   * operand_map
	   */
	  for (j=0; j < total_operands; j++)
	    {
	      if (fscanf (in, "%x", &raw_id) != 1)
		S_punt ("Error parsing '%s'.", file_name);
	      op->operand[j] = operand_map[raw_id];
	    }
	  
	  /* Mark predicated ops */
	  if ((S_max_pred_operand > 0)  && 
	      (op->operand[S_first_pred] > 0))
	    {
	      op->flags |= PREDICATED;
	    }
	  
	  opc_type = opc_info_tab[op->opc].opc_type;
	  
	  /* If scheduled code, give each op a packet id */
	    if (S_sched_info_avail)
	      {
		/* Detect start of new packets */
		if (op->flags & START_PACKET)
		  S_max_packet_id++;
		
		/* Give the op the current packet id */
		op->packet_id = S_max_packet_id;
	      }
	    
	    /* Otherwise, give each a -1 packet id */
	    else
	      {
		op->packet_id = -1;
	      }
	    
	    /* If JSR, add pc to jsr_tab */
	    if (opc_type == JSR_OPC)
	      {
		jsr_id++;
		fn->jsr_tab[jsr_id] = op->pc;
	      }
	    
	    /* Initialize last address used to 0 (for off path guesses)*/
	    op->last_addr = 0;
	    
	    /*
	     * Initialize skip count to the number of trace words to 
	     * skip for non-branch instructions (-1 for branches).
	     * 
	     * Add 1 for predicated instructions (if not branch)
	     * and for 1st instr in cb (if not branch or 
	     * predicated load/store) (below)
	     */
	    op->trace_words_read = opc_info_tab[op->opc].trace_words_read;
	    
	    if ((op->trace_words_read != -1) && (op->flags & PREDICATED))
	      {
		/* 
		 * Predicated loads/stores may or may not have address
		 * info traced.   Flag that cannot skip quickly
		 */
		if (op->trace_words_read == 0)
		  op->trace_words_read = 1;
		else
		  op->trace_words_read = -1;
	      }
	    
	    /* 
	     * Detect when we are tracing predicates before promotion 
	     * (Trace anything in pred[1] slot if it is a register
	     */
	    if (S_trace_promoted_predicates && (S_max_pred_operand >= 2) &&
		(op->operand[S_first_pred + 1] > 0))
	      {
		/* Flag that we should trace the predicate before promotion*/
		op->flags |= TRACE_PROMOTED_PRED;
		
		/* 
		 * May be skipped easily unless already complexly traced
		 * Read whether or not instruction is predicate squashed.
		 */
		if (op->trace_words_read != -1)
		  {
		    op->trace_words_read++;
		  }
	      }
	    
	    /* 
	     * Add 1 to trace_words read if load is non-trapping and
	     * can be skipped easily (ie. not predicated).
	     */
	    if ((op->trace_words_read != -1) && (opc_type == LOAD_OPC) &&
		(op->flags & NON_TRAPPING))
	      
	      {
		op->trace_words_read++;
	      }
	    
	    /* 
	     * If operation is an implicit memory reference,
	     * increment trace_words_read by 1 unless already needs
	     * special handling (marked by -1 )
	     */
	    if ((op->trace_words_read != -1) && 
		(op->flags & IMPLICIT_MEMORY_OP))
	      {
		op->trace_words_read++;
	      }
	    
	    /* 
	     * If the operation is an predicate define instruction
	     * and we are tracing pred defs,
	     * read in a trace word for each register destination
	     */
	    if (op->flags & PRED_DEF)
	      {
		/* If tracing pred defs, update skip count if necessary */
		if (S_trace_pred_defs)
		  {
		    if ((op->trace_words_read != -1))
		      {
			for (dest_index=S_first_dest; dest_index<=S_last_dest; 
			     dest_index++)
			  {
			    if (op->operand[dest_index] > 0)
			      {
				op->trace_words_read++;
			      }
			  }
		      }
		  }
		/* Otherwise, filter out PRED_DEF flag */
		else
		  {
		    op->flags ^= PRED_DEF;
		  }
	      }
#if 0
	    else {
	      if (S_trace_values) 
		{
		  num_values = 0;
		  if ((op->trace_words_read != -1))
		    {
		      for (dest_index=S_first_dest; dest_index<=S_last_dest; 
			   dest_index++)
			{
			  if (op->operand[dest_index] > 0)
			    {
			      op->trace_words_read++;
			      num_values++;
			    }
			}
		      
		      for (src_index=S_first_src; src_index<=S_last_src; 
			   src_index++)
			{
			  if (op->operand[src_index] > 0)
			    {
			      op->trace_words_read++;
			      num_values++;
			    }
			}
		    }
		}
	    }
#endif
		  
	    /* Detect if program has PREFETCHes */
	    if (opc_type == PREFETCH_OPC ||
		(opc_type == LOAD_OPC && 
		 (op->playdoh_flags & PLAYDOH_PREFETCH)))
	      S_has_prefetches = 1;
	    if (opc_type == CBR_OPC)
	      S_operation_count_cond++;
	    else {
	      if (op->flags & PREDICATED) {
		if (opc_type == JSR_OPC)
		  S_operation_count_pred_call++;
		else if (opc_type == RTS_OPC) {
		  S_operation_count_pred_ret++;
		  S_punt ("No such thing as a predicated RTS");
		}
		else if ((opc_type == JMP_OPC) || (opc_type == JRG_OPC))
		  S_operation_count_pred_uncond++;
	      }
	      else {
		if (opc_type == JSR_OPC)
		  S_operation_count_call++;
		else if (opc_type == RTS_OPC)
		  S_operation_count_ret++;
		else if ((opc_type == JMP_OPC) || (opc_type == JRG_OPC))
		  S_operation_count_uncond++;
	      }
	    }
	    
	    /* Detect if program has MEM_COPY_DIRECTIVEs */
	    if ((opc_type == MEM_COPY_OPC) ||
		(opc_type == MEM_COPY_BACK_OPC) ||
		(opc_type == MEM_COPY_CHECK_OPC) ||
		(opc_type == MEM_COPY_RESET_OPC) ||
		(opc_type == MEM_COPY_TAG_OPC))
	      S_has_mem_copy_directives = 1;
	    
	    /* 
	     * Some versions of mem_copy have MEM_COPY_CHECK and
	     * MEM_COPY_BACK take ZERO_SPACE in the processor.
	     */
	    if ((S_mem_copy_version >= 2) &&
		(opc_type == MEM_COPY_CHECK_OPC))
	      op->flags |= ZERO_SPACE;
	    
	    if ((S_mem_copy_version >= 3) &&
		(opc_type == MEM_COPY_BACK_OPC))
	      op->flags |= ZERO_SPACE;
	    
	    /*
	     * If instruction is has attributes that force simulation
	     * or stops forcing simulation, then mark instruction
	     * as changing simulator state.
	     */
	    if ((op->flags & FORCE_SIM_ON) || (op->flags & FORCE_SIM_OFF))
	      {
		op->flags |= CHANGES_STATE;
		op->trace_words_read = -1;
	      }
	    
	    /* 
	     * If instruction stops the simulator, also mark that it
	     * changes state.
	     */
	    if (op->flags & STOP_SIM) 
	      {
		/* 
		 * If feature enabled, mark that changes state, otherwise
		 * filter out markings.
		 */
		if (S_stop_sim_trip_count > 0)
		  {
		    op->flags |= CHANGES_STATE;
		    op->trace_words_read = -1;
		  }
		else
		  {
		    op->flags ^= STOP_SIM;
		  }
	      }
	    
	    
	    /* 
	     * If we are not taking region stats, then filter out
	     * the STATS_ON and STATS_OFF attribute.
	     */
	    if (!S_region_stats)
	      {
		op->flags &= ~(STATS_ON_ATTR | STATS_OFF_ATTR);
	      }
	    
	    /* 
	     * If instruction starts or stops a region, force it
	     * to go the long skip path (so can put expensive region
	     * code on long path)
	     */
	    if ((op->flags & STATS_ON_ATTR) || (op->flags & STATS_OFF_ATTR))
	      {
		op->trace_words_read = -1;
		
		/* Mark that changing region and simulator state*/
		op->flags |= REGION_BOUNDARY | CHANGES_STATE;
		
		/* 
		 * May not have both stats on and stats off in
		 * same instruction.
		 */
		if ((op->flags & STATS_ON_ATTR) &&
		    (op->flags & STATS_OFF_ATTR))
		  S_punt ("%s op %i: stats_on and stats_off in same operation.",
			  fn->name, op->lcode_id);
		
		/* Create stats structure for each region */
		if (op->flags & STATS_ON_ATTR)
		  {
		    sprintf (region_name, "%s op %i",
			     fn->name, op->lcode_id);
		    op->stats = S_create_stats (region_name);
		  }
		
		/* If stats_off, restore program level stats */
		else
		  {
		    op->stats = S_program_stats;
		  }
	      }
	    
	    /* If not start or stop of stats region, NULL stats */
	    else
	      {
		op->stats = NULL;
	      }
	}
      
      /* Make sure all jsr's read in */
      if (jsr_id != fn->max_jsr_id)
	S_punt ("Error parsing '%s': %i jsrs expected, not %i.",
		file_name, fn->max_jsr_id, jsr_id);
      
      fscanf (in, "\n\n");
      
      /* Initialize cbs first (to cb not present) */
      for (i=0; i <= max_cb_id; i++)
	{
	  cb = &fn->cb[i];
	  cb->fn = fn;
	  cb->lcode_id = -1;
	  cb->start_pc = 0;	
	}
      
      /* Read in cbs */
      last_cb = NULL;
      last_start = 0;
      for (i=0; i < num_cbs; i++)
	{
	  
	  if (version == OBJTR_VERSION)
	    {
	      if (fscanf (in, "%x %x %x\n", 
			  &cb_id, &start, &num_preheaders) != 3)
		S_punt ("'%s' is corrupt.", file_name);
	      
	      
	      if (num_preheaders != 0) {
		
		fn->cb[cb_id].preheaders = malloc ((num_preheaders+1)*sizeof(int));
		
		for (index2 = 0; index2 < num_preheaders; index2++) {
		  if(fscanf (in, "%x ", 
			     &(fn->cb[cb_id].preheaders[index2])) != 1)
		    S_punt ("'%s' preheaders are corrupt.", file_name);
		}
		fscanf (in, "\n");
		fn->cb[cb_id].preheaders[index2] = -1;
	      }
	    }
	  else
	    {
	      if (fscanf (in, "%x %x\n", &cb_id, &start) != 2)
		S_punt ("'%s' is corrupt.", file_name);
	    }
	  
	  fn->cb[cb_id].lcode_id = cb_id;
	  
	  /* Convert op id into pc */
	  fn->cb[cb_id].start_pc = fn->op[start].pc;
	  
	  /* 
	   * Flag that this cb should be traced, unless first
	   * cb in function (i == 0)
	   */
	  if (i != 0)
	    {
	      /* 
	       * Only mark the start of the cb once,
	       * This makes trace_words_read correct.
	       */
	      if (!(fn->op[start].flags & TRACED_CB_ENTRY))
		{
		  fn->op[start].flags |= TRACED_CB_ENTRY;
		  
		  /* 
		   * Need to skip one more trace word (for 
		   * non-branches or predicated loads/stores) 
		   * while skipping instructions.
		   * (see 50 lines above)
		   */
		  if (fn->op[start].trace_words_read != -1)
		    fn->op[start].trace_words_read++;
		}
	    }
	  
	  /* Fill in cb field in oper for last cb read in */
	  for (j=last_start; j < start; j++)
	    fn->op[j].cb = last_cb;
	  
	  /* Save for next pass to can fill in cbs */
	  last_start = start;
	  last_cb = &fn->cb[cb_id];
	}
      
      /* Fill in cb field for last cb */
      for (j=last_start; j < num_ops; j++)
	fn->op[j].cb = last_cb;
      
      if (fscanf (in, "\n%s%s\n\n\n", lbuf, sbuf) != 2)
	S_punt ("'%s' is corrupted.", file_name);
      
      if ((strcmp (lbuf, "end") != 0) ||
	  (strcmp (sbuf, fn->name) != 0))
	S_punt ("'%s' is corrupted. 'end %s' expected.", file_name,
		fn->name); 
      
      /* Add function to linked list */
      if (tail_fn == NULL)
	head_fn = fn;
      else
	tail_fn->next_fn = fn;
      tail_fn = fn;
      fn->next_fn = NULL;
      
      /* Update statistics */
      function_count++;

      /* For global loop renumbering */
      if(S_trace_objects)
	{
	  prev_max_loop = max_fn_loop + prev_max_loop;
	}
    }

  /* 
   * Get function addresses from file or executable's symbol table,
   * or just calculated Lemulate id from .encode file order -ITI (JCG) 1/99
   */
  if (S_use_func_ids_not_addrs)
    {
      S_calc_fn_addresses_from_order (head_fn);
    }
  
  else if (S_use_file_mode || S_read_addr_file)
    {
      if (S_addr_file == NULL)
	S_punt ("Function address list file name not specified");
      S_read_fn_addresses_from_list (head_fn, S_addr_file);
    }
  
  else
    {
      if (S_exec_name == NULL)
	S_punt ("Executable file name not specified");
      S_read_fn_addresses_from_exec (head_fn, S_exec_name);
    }
  
  /* Intialize fn_hash */
  for (i=0; i < FN_HASH_SIZE; i++)
    fn_hash[i] = NULL;
  
  /* Initialize the multiple function def counter */
  multiple_fn_defs = 0;
  
  /* Add function to fn_hash */
  for (fn = head_fn; fn != NULL; fn = fn->next_fn)
    {
      /* Use the lower bits (except byte bits (always 0) as hash index */
      index = (fn->addr >> 2) & (FN_HASH_SIZE - 1);
      
      /* Make sure this function is not already in hash table.
       * This occurs with multiple static function when renaming has
       * not been run!
       */
      for (search_fn = fn_hash[index]; search_fn != NULL; 
	   search_fn = search_fn->next_hash)
	{
	  if (strcmp (fn->name, search_fn->name) == 0)
	    {
	      fprintf (stderr, 
		       "Error static function %s defined multiple times.\n",
		       fn->name);
	      multiple_fn_defs++;
	      break;
	    }
	  
	}
      
      /* Add function to hash table */
      fn->next_hash = fn_hash[index];
      fn_hash[index] = fn;
    }
  if (multiple_fn_defs != 0)
    {
      S_punt ("Error: %i ambiguous function names!\n  Renaming must be done (in Hcode or Pcode) to globalize the function names.\n   Multiple functions with the same name confuses the symbol table parser.", 
	      multiple_fn_defs);
    }
  
  /* Create and initialize oper table */
  S_max_pc = pc;
  tab_size = (S_max_pc + 2) * sizeof (S_Oper *);
  if ((oper_tab = (S_Oper **) malloc (tab_size)) == NULL)
    S_punt ("S_load_code: Out of memory");
  
  /* 
   * The pc == 0 , pc < S_program_start_pc, or pc == (S_max_pc + 1) 
   * are not legal.
   */
  for (i=0; i < S_program_start_pc; i++)
    oper_tab[i] = NULL;
  oper_tab[S_max_pc + 1] = NULL;
  
  /* For all ops, point oper_tab at appropriate op */
  for (fn = head_fn; fn != NULL; fn = fn->next_fn)
    for (i=0; i < fn->op_count; i++)
      oper_tab[fn->op[i].pc] = &fn->op[i];
  
  /* Allocate/Initialize operand_tab */
  tab_size = (S_max_register_operand - S_min_const_operand + 1) * 
    sizeof (S_Operand *);
  if ((temp_operand_tab_ptr = (S_Operand **) malloc (tab_size)) == NULL)
    S_punt ("S_load_code: Out of memory");
  
  /* Allow negative indexing of up to S_min_const_operand */
  operand_tab = &temp_operand_tab_ptr[-S_min_const_operand];
  
  
  /* Scan operand hash table for all operands */
  for (i=0; i < OPERAND_HASH_SIZE; i++)
    {
      for (operand_ptr = operand_hash[i]; operand_ptr != NULL; 
	   operand_ptr = operand_ptr->next_hash)
	{
	  operand_tab[operand_ptr->id] = operand_ptr;
	}
    }
  
  
  /* Save function count for stats */
  S_function_count = function_count;
  
  /* Detect empty files */
  if (function_count == 0)
    S_punt ("Encoded lcode file '%s' is empty.", file_name);
}



void 
S_load_opc_info (char * file_name)
{
  FILE *in;
  int tab_size, opc;
  char name_buf[200];
  char line_buf[1000];
  char type_buf[200];
  S_Opc_Info *info;
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int scale_factor, delta_factor;
#endif
  int virtual_latency, access_size;
  int i;
  char lat;
  
  /* Open file for reading */
  if ((in = fopen (file_name, "r")) == NULL)
    S_punt ("Unable to open opc_info file '%s'.", file_name);
  
  
  /* Get number of opc entries */
  if (fscanf (in, "OPC INFO MAX OPC %i\n", &S_max_opc) != 1)
    S_punt ("Error parsing first line of '%s'.", file_name);
  
  tab_size = (S_max_opc + 1) * sizeof (S_Opc_Info);
  
  if ((opc_info_tab = (S_Opc_Info *) malloc (tab_size)) == NULL)
    S_punt ("Out of memory");
  
  /* 
   * Initialize fields in table, now support gaps in the opcode
   * numbering -JCG 1/22/98
   */
  for (i=0; i <= S_max_opc; i++)
    {
      /* Get the table entry for ease of use */
      info = &opc_info_tab[i];
      
      /* Set to values that should cause core dumps if they are used */
      info->name = NULL;
      info->opc = -10000000;
      info->opc_type_name = NULL;
      info->conflict_mask = -1;
      info->opc_type = -1;
      
      /* Make the machine deadlock if these latencies are used */
      info->virtual_latency = 100000;
      info->real_latency = 100000;
    }
  
  /* Read in opc info until hit EOF */
  while (fgets (line_buf, sizeof(line_buf), in) != NULL)
    {
      
      if (sscanf (line_buf, "%d %s %d %d %s\n", &opc, name_buf, 
		  &virtual_latency, 
		  &access_size,
		  type_buf) != 5)
	S_punt ("Error parsing '%s'.", file_name);
      
      if ((opc < 0) || (opc > S_max_opc))
	{
	  S_punt ("Error parsing '%s'.\n"
		  "Opc %i out of range (min 0 max %i).\n"
		  "Line being parsed: %s",
		  file_name, opc, S_max_opc, line_buf);
	}
      
      /* Get the table entry for ease of use */
      info = &opc_info_tab[opc];
      
      /* Make sure this opcode has not already appeared */
      if (info->name != NULL)
	{
	  S_punt ("Error parsing '%s'.\n"
		  "Already set opc info for opc %i to '%s'.\n"
		  "Line being parsed: %s",
		  file_name, opc, info->name, line_buf);
	  
	}
      
      info->name = S_alloc_string (strlen (name_buf) + 1);
      strcpy (info->name, name_buf);
      
      /* Save opc and opcode type name for debugging */
      info->opc = opc;
      info->opc_type_name = strdup (type_buf);
      
      /* Set virtual latency and access size */
      info->virtual_latency = virtual_latency;
      info->access_size = access_size;
      
      /* 
       * Set conflict mask, used for determining if accesses of 
       * different sizes conflict. (prefetch sizes are handled below)
       */
      if (info->access_size == 0)
	info->conflict_mask = -1;
      else
	info->conflict_mask = ~(info->access_size - 1);
      
      if (strcmp (type_buf, "UNTRACED") == 0)
	info->opc_type = UNTRACED_OPC;
      else if (strcmp (type_buf, "CBR") == 0)
	info->opc_type = CBR_OPC;
      else if (strcmp (type_buf, "JMP") == 0)
	info->opc_type = JMP_OPC;
      else if (strcmp (type_buf, "JRG") == 0)
	info->opc_type = JRG_OPC;
      else if (strcmp (type_buf, "JSR") == 0)
	info->opc_type = JSR_OPC;
      else if (strcmp (type_buf, "RTS") == 0)
	info->opc_type = RTS_OPC;
      else if (strcmp (type_buf, "LOAD") == 0)
	info->opc_type = LOAD_OPC;
      else if (strcmp (type_buf, "STORE") == 0)
	info->opc_type = STORE_OPC;
      
      /* Mem copy directives that need special processing */
      else if (strcmp (type_buf, "MEM_COPY") == 0)
	info->opc_type = MEM_COPY_OPC;
      else if (strcmp (type_buf, "MEM_COPY_TAG") == 0)
	info->opc_type = MEM_COPY_TAG_OPC;
      else if (strcmp (type_buf, "MEM_COPY_BACK") == 0)
	info->opc_type = MEM_COPY_BACK_OPC;
      else if (strcmp (type_buf, "MEM_COPY_CHECK") == 0)
	info->opc_type = MEM_COPY_CHECK_OPC;
      else if (strcmp (type_buf, "MEM_COPY_RESET") == 0)
	info->opc_type = MEM_COPY_RESET_OPC;
      else if (strcmp (type_buf, "MOVE") == 0)
	info->opc_type = MOVE_OPC;
      else if (strcmp (type_buf, "IALU") == 0)
	info->opc_type = IALU_OPC;
      else if (strcmp (type_buf, "FALU") == 0)
	info->opc_type = FALU_OPC;
      else if (strcmp (type_buf, "CHECK") == 0)
	info->opc_type = CHECK_OPC;
      
      /* For prefetches, make access size and mask for entire block */
      else if (strcmp (type_buf, "PREFETCH") == 0)
	{
	  info->opc_type = PREFETCH_OPC;
	  info->access_size = S_dcache_block_size;
	  info->conflict_mask = ~(info->access_size - 1);
	}
      
      else 
	S_punt ("Unknown instruction type '%s' for opc %i (%s).", 
		type_buf, opc, name_buf);
      
      /* 
       * Set the real latency based on the type of operation
       * and the scale and delta for that type of operation
       */
      S_set_real_latency(info,info->virtual_latency,&lat);
      info->real_latency = (int) lat;
    }
  
  /* Intialize opcode specific trace info */
  S_init_opcode_trace ();
}


void 
S_print_opc_info_tab (FILE * out)
{
  int i;
  S_Opc_Info *info;
  
  fprintf (out, "# OPC INFO TABLE\n\n");
  fprintf (out, "# VL -> virtual latency\n");
  fprintf (out, "# RL -> real latency\n");
  fprintf (out, "# AS -> access size\n");
  fprintf (out, "\n");
  fprintf (out,
	   "%4s %-20s %-15s %2s %2s %2s\n", 
	   "OPC", "NAME", "TYPE", "VL", "RL", "AS");
  fprintf (out,
	   "%4s %-20s %-15s %2s %2s %2s\n", 
	   "---", "--------------------", "---------------", "--", 
	   "--", "--");
  
  for (i = 0; i <= S_max_opc; i++)
    {
      info = &opc_info_tab[i];
      
      fprintf (out, 
	       "%4i %-20s %-15s %2i %2i %2i\n",
	       info->opc, info->name, info->opc_type_name, 
	       info->virtual_latency, info->real_latency, 
	       info->access_size);
    }
  
  fprintf (out, "\n");
}




