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
 *  File:  s_trace.c
 *
 *  Description:  Routines to process the trace and generate sints from it.
 *
 *  Creation Date :  September, 1993
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

#include <unistd.h>
#ifdef WIN32
#include <stdio.h>
#include <io.h>
#include "windows.h"
#endif

#include "s_main.h"
#include "s_object.h"

void T_error(Pnode * pnode, Sint * sint, char * fmt, ...);

#undef DEBUG_TRACE	/* Define to turn on trace messages */

/* Global variables */
int S_serial_no = 1;

/*
 * Allocates and initializes trace manager to no trace information loaded.
 */
Trace_Manager *
S_create_trace_manager (Pnode * pnode)
{
  Trace_Manager *manager;
  char name_buf[100];
  
  manager = (Trace_Manager *) L_alloc (Trace_Manager_pool);
  sprintf (name_buf, "Trace manager %i", pnode->id);
  manager->name = strdup (name_buf);
  manager->pnode = pnode;
  manager->trace_block = NULL;
  manager->trace_ptr = NULL;
  manager->size_left = 0;
  
  return (manager);
}

/*
 * Returns the S_Fn for the given addr.
 * Returns NULL if not found.
 */

S_Fn *
S_get_fn (int fn_addr)
{
  S_Fn *fn;
  int fn_index;
  
  fn_index = (fn_addr >> 2) & (FN_HASH_SIZE -1);
  
  for (fn = fn_hash[fn_index]; fn != NULL; fn = fn->next_hash)
    {
      if (fn->addr == fn_addr)
	break;
    }
  
  return (fn);
}

/* 
 * Trace routines 
 */
void 
S_trace_cbr (Pnode * pnode, Sint * sint)
{
  int peek_val;
  unsigned int cb_id;
  
#ifdef DEBUG_TRACE
  printf ("Tracing cbr\n");
#endif
  peek_val = S_peek_trace_word (pnode);
  
  if (peek_val == L_TRACE_BRTHRU)
    {
      S_get_trace_word (pnode);
    }
  else
    {
      /* 
       * If we are simulating playdoh vliw then the cb id will
       * not come through until the end of the liw.
       */
      if (S_processor_model != PROCESSOR_MODEL_PLAYDOH_VLIW)
	{
	  /* Convert peek val to cb id and make sure valid */
	  cb_id = -peek_val;
	  
	  if (cb_id > (unsigned)sint->fn->max_cb)
	    T_error (pnode, sint, "Cb %i out of bounds.", cb_id);
	  
	  /* Set branch address and flag */
	  sint->trace.target_pc = sint->fn->cb[cb_id].start_pc;
	}
      
      sint->flags |= BRANCHED;
    }
}

void 
S_trace_jmp (Pnode * pnode, Sint * sint)
{
  unsigned int cb_id;
  
#ifdef DEBUG_TRACE
  printf ("Tracing jmp\n");
#endif
  
  /* 
   * TLJ - for playdoh vliw code the jmp will not take until the
   * end of the liw. Only mark as BRANCHED.
   */
  if (S_processor_model == PROCESSOR_MODEL_PLAYDOH_VLIW)
    {
      sint->flags |= BRANCHED;
      return;
    }
  
  /* Peek to see what cb_id jumped to */
  cb_id = -S_peek_trace_word (pnode);
  
  if (cb_id > sint->fn->max_cb)
    T_error (pnode, sint, "Cb %i out of bounds.", cb_id);
  
  /* Set branch address and flag */
  sint->trace.target_pc = sint->fn->cb[cb_id].start_pc;
  sint->flags |= BRANCHED;
}

void 
S_trace_jmp_finish (Pnode * pnode, Sint * sint)
{
  unsigned int cb_id;
  
#ifdef DEBUG_TRACE
  printf ("Tracing jmp\n");
#endif
  
  /* Peek to see what cb_id jumped to */
  cb_id = -S_peek_trace_word (pnode);
  
  if (cb_id > sint->fn->max_cb)
    T_error (pnode, sint, "Cb %i out of bounds.", cb_id);
  
  /* Set branch address and flag */
  sint->trace.target_pc = sint->fn->cb[cb_id].start_pc;
  sint->flags |= BRANCHED;
}

void 
S_trace_exit_point (Pnode * pnode, Sint * sint, int read_end)
{
  S_Oper *op;
  int marker;
  
  /* Get op for ease of use */
  op = sint->oper;
  
  if (!read_end)
    {
      if ((marker = S_get_trace_word(pnode)) != L_TRACE_SAMPLE_END)
	T_error (pnode, sint, 
		 "Expecting L_TRACE_SAMPLE_END not %i.",
		 marker);
    }
  
  if ((marker = S_get_trace_word(pnode)) != L_TRACE_END)
    T_error (pnode, sint, "Expecting L_TRACE_END not %i.",
	     marker);
  
  /* Flag that we are at end of program */
  S_end_of_program = 1;
  
  /* Flag that simulation ending normally */
  S_normal_termination = 1;
  
  /* Exit point better be a branch instruction */
  if (!opc_info_tab[op->opc].is_branch)
    {
      T_error(pnode, sint, 
	      "Expecting branch at exit point, not %i (%s).",
	      op->opc, opc_info_tab[op->opc].name);
    }
  
  /* Make instruction branch to pc 0 */
  sint->trace.target_pc = 0;
  sint->flags |= BRANCHED;
}


void 
S_trace_jsr (Pnode * pnode, Sint * sint)
{
  int jsr_dest;
  int	fn_addr;
  S_Fn *called_fn;
  S_Fn *ret_fn;
  int jsr_id;
  int ret_pc;

#ifdef DEBUG_TRACE
  printf ("Tracing jsr\n");
#endif
  
  /* 
   * TLJ - for playdoh vliw code the jsr will not take until the
   * end of the liw. Only mark as BRANCHED.
   */
  if (S_processor_model == PROCESSOR_MODEL_PLAYDOH_VLIW)
    {
      sint->flags |= BRANCHED;
      return;
    }
  
  /* If function called probed, get function entered */
  if ((jsr_dest = S_get_trace_word (pnode)) == L_TRACE_FN)
    {
      /* Get address of function called */
      fn_addr = S_get_trace_word (pnode);
      
      if ((called_fn = S_get_fn (fn_addr)) == NULL)
	{
	  T_error (pnode, sint,
		   "JSR to unknown address %i (%x).",
		   fn_addr, fn_addr);
	}
      
      latest_fn_name = called_fn->name;

      sint->trace.target_pc = called_fn->op[0].pc;
      sint->flags |= BRANCHED;
    }
  /* 
   * Detect exit at jsr to unprobed function that exits.
   * Happens with partial probing of functions.
   * Print out a warning to let user know what is happening.
   */
  else if (jsr_dest == L_TRACE_SAMPLE_END)
    {
      fprintf (stderr, 
	       "\nWarning: End of program detected in unprobed code");
      fprintf (stderr,
	       " (OK if partially probed).\n");
      S_trace_exit_point (pnode, sint, 1);
      return;
    }
  
  else
    {
      /* Get the jsr_id that we returned to */
      jsr_id = L_TRACE_JSR_OFFSET - jsr_dest;
      
      /* 
       * Otherwise, unprobed function.  Get where we have returned to
       * to see if returned to the same place.
       */
      fn_addr = S_get_trace_word (pnode);
      if ((ret_fn = S_get_fn (fn_addr)) == NULL)
	{
	  T_error (pnode, sint,
		   "Untraced JSR returned to unknown fn address %i (%x).",
		   fn_addr, fn_addr);
	}
      
      /* Get pc of jsr returned to */
      ret_pc = ret_fn->jsr_tab[jsr_id];
      
      /* If not same jsr that called function, we have done a longjmp */
      if (ret_pc != sint->oper->pc)
	{
	  /* Treat as branch to return location */
	  sint->trace.target_pc = ret_pc + 1;
	  sint->flags |= BRANCHED | LONGJMP;
	  
	}
      
      else
	{
	  /* Mark as a untraced jsr */
	  sint->flags |= UNTRACED_JSR;
	}
    }
}

/* 
 * TLJ called for playdoh vliw simulation to get the return
 * info at the end of a liw for a jsr that took during the liw.
 */
void 
S_trace_jsr_finish (Pnode * pnode, Sint * sint)
{
  int jsr_dest;
  int	fn_addr;
  S_Fn *called_fn;
  S_Fn *ret_fn;
  int jsr_id;
  int ret_pc;
  
#ifdef DEBUG_TRACE
  printf ("Tracing jsr\n");
#endif
  
  /* If function called probed, get function entered */
  if ((jsr_dest = S_get_trace_word (pnode)) == L_TRACE_FN)
    {
      /* Get address of function called */
      fn_addr = S_get_trace_word (pnode);
      
      if ((called_fn = S_get_fn (fn_addr)) == NULL)
	{
	  T_error (pnode, sint,
		   "JSR to unknown address %i (%x).",
		   fn_addr, fn_addr);
	}

      latest_fn_name = called_fn->name;      

      sint->trace.target_pc = called_fn->op[0].pc;
      sint->flags |= BRANCHED;
    }
  /* 
   * Detect exit at jsr to unprobed function that exits.
   * Happens with partial probing of functions.
   * Print out a warning to let user know what is happening.
   */
  else if (jsr_dest == L_TRACE_SAMPLE_END)
    {
      fprintf (stderr, 
	       "\nWarning: End of program detected in unprobed code");
      fprintf (stderr, 
	       " (OK if partially probed).\n");
      S_trace_exit_point (pnode, sint, 1);
      return;
    }
  
  else
    {
      /* Get the jsr_id that we returned to */
      jsr_id = L_TRACE_JSR_OFFSET - jsr_dest;
      
      /* 
       * Otherwise, unprobed function.  Get where we have returned to
       * to see if returned to the same place.
       */
      fn_addr = S_get_trace_word (pnode);
      if ((ret_fn = S_get_fn (fn_addr)) == NULL)
	{
	  T_error (pnode, sint,
		   "Untraced JSR returned to unknown fn address %i (%x).",
		   fn_addr, fn_addr);
	}
      
      /* Get pc of jsr returned to */
      ret_pc = ret_fn->jsr_tab[jsr_id];
      
      /* If not same jsr that called function, we have done a longjmp */
      if (ret_pc != sint->oper->pc)
	{
	  /* Treat as branch to return location */
	  sint->trace.target_pc = ret_pc + 1;
	  sint->flags |= BRANCHED | LONGJMP;
	  
	}
      
      else
	{
	  /* Mark as a untraced jsr */
	  sint->flags |= UNTRACED_JSR;
	  
	  /* Remove the BRANCHED flag set earlier */
	  sint->flags &= ~(BRANCHED);
	}
    }
}

void 
S_trace_rts (Pnode * pnode, Sint * sint)
{
  int jsr_id, fn_addr, peek_val;
  S_Fn *calling_fn;
  
#ifdef DEBUG_TRACE
  printf ("Tracing rts\n");
#endif
  
  if (S_trace_objects && S_is_asynch_tr(S_peek_trace_word(pnode)))
    S_read_obj_trace(pnode);
  
  /* 
   * TLJ - for playdoh vliw code the rts will not take until the
   * end of the liw. Only mark as BRANCHED.
   */
  if (S_processor_model == PROCESSOR_MODEL_PLAYDOH_VLIW)
    {
      sint->flags |= BRANCHED;
      return;
    }
  
  /* 
   * In perl, the probed function is called repeatly by qsort.
   * Allow a return to a the start of another function. -JCG 5/17/95
   */
  peek_val = S_peek_trace_word (pnode);
  if (peek_val == L_TRACE_FN)
    {
      /* Trace it like a jsr */
      S_trace_jsr (pnode, sint);
      return;
    }
  
  /* 
   * Detect exit at return other than normal exit point.
   * Happens with partial probing of functions.
   * Print out a warning to let user know what is happening.
   */
  else if (peek_val == L_TRACE_SAMPLE_END)
    {
      fprintf (stderr, 
	       "\nWarning: End of program detected in unprobed code");
      fprintf (stderr,
	       "(OK if partially probed).\n");
      S_trace_exit_point (pnode, sint,0);
      return;
    }
  
  /* Get jsr_id from trace (L_TRACE_JSR_OFFSET - jsr_id) */
  jsr_id = L_TRACE_JSR_OFFSET - S_get_trace_word (pnode);
  
  /* This should be positive */
  if (jsr_id <= 0)
    T_error (pnode, sint, 
	     "RTS to invalid jsr_id %i.", jsr_id);
  
  /* Get function returned to */
  fn_addr = S_get_trace_word (pnode);
  if ((calling_fn = S_get_fn (fn_addr)) == NULL)
    {
      T_error (pnode, sint, 
	       "RTS to unknown fn address %i (%x).",
	       fn_addr, fn_addr);
    }
  
  /* Return to just after jsr */
  sint->trace.target_pc = calling_fn->jsr_tab[jsr_id] + 1;
  sint->flags |= BRANCHED;
}

void 
S_trace_rts_finish (Pnode * pnode, Sint * sint)
{
  int jsr_id, fn_addr, peek_val;
  S_Fn *calling_fn;
  
#ifdef DEBUG_TRACE
  printf ("Tracing rts\n");
#endif
  
  /* 
   * In perl, the probed function is called repeatly by qsort.
   * Allow a return to a the start of another function. -JCG 5/17/95
   */
  peek_val = S_peek_trace_word (pnode);
  if (peek_val == L_TRACE_FN)
    {
      /* Trace it like a jsr */
      S_trace_jsr_finish (pnode, sint);
	return;
    }

  /* 
   * Detect exit at return other than normal exit point.
   * Happens with partial probing of functions.
   * Print out a warning to let user know what is happening.
   */
  else if (peek_val == L_TRACE_SAMPLE_END)
    {
      fprintf (stderr, 
	       "\nWarning: End of program detected in unprobed code");
      fprintf (stderr,
	       "(OK if partially probed).\n");
      S_trace_exit_point (pnode, sint,0);
      return;
    }
  
  /* Get jsr_id from trace (L_TRACE_JSR_OFFSET - jsr_id) */
  jsr_id = L_TRACE_JSR_OFFSET - S_get_trace_word (pnode);
  
  /* This should be positive */
  if (jsr_id <= 0)
    T_error (pnode, sint, 
	     "RTS to invalid jsr_id %i.", jsr_id);
  
  /* Get function returned to */
  fn_addr = S_get_trace_word (pnode);
  if ((calling_fn = S_get_fn (fn_addr)) == NULL)
    {
      T_error (pnode, sint, 
	       "RTS to unknown fn address %i (%x).",
	       fn_addr, fn_addr);
    }
  
  /* 
   * Return to start of next liw just after jsr. To find
   * this I search starting from jsr_pc+1 until I find
   * an op with instr_addr != 0.
   */
  sint->trace.target_pc = calling_fn->jsr_tab[jsr_id] + 1;
  while (oper_tab[sint->trace.target_pc]->instr_addr == 0)
    sint->trace.target_pc++;
  sint->flags |= BRANCHED;
}

/* JWS/HCH 19991026 Speculated load analysis, ISCA2000 */
void 
S_trace_check (Pnode * pnode, Sint * sint)
{
#ifdef DEBUG_TRACE
  printf ("Tracing check for speculative load\n");
#endif
  S_get_trace_word (pnode);  
  
  return;
}

void 
S_trace_load (Pnode * pnode, Sint * sint)
{
  int fault_flag, access_size;
  
#ifdef DEBUG_TRACE
  printf ("Tracing load\n");
#endif
  
  sint->trace.mem_addr = S_get_trace_word (pnode);
  
  /* If non-trapping load, detect seg fault and bus error */
  if (sint->oper->flags & NON_TRAPPING)
    {
      fault_flag = S_get_trace_word (pnode);
      
      /* Detect segmentation fault */
      if (fault_flag == L_TRACE_MASKED_SEG_FAULT)
	sint->flags |= MASKED_SEG_FAULT;
      
      /* Make sure if not seg fault, get no seg fault flag */
      else if (fault_flag != L_TRACE_NO_SEG_FAULT)
	{
	  T_error (pnode, sint, 
		   "Unknown NON-TRAPPING LOAD fault flag %i (%x).",
		   fault_flag, fault_flag);
	}
      
      /* 
       * Detect bus error (assume aligned accesses).
       * May have both a bus error and seg fault.
       */
      access_size = opc_info_tab[sint->oper->opc].access_size;
      if (((access_size -1) & sint->trace.mem_addr) != 0)
	sint->flags |= MASKED_BUS_ERROR;
      
    }

  /* Save address so can guess off path addresses */
  /* Done in S_gen_sint and S_skip now */
}

void 
S_trace_prefetch (Pnode * pnode, Sint * sint)
{
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int addr;
#endif
  int header;
  
#ifdef DEBUG_TRACE
  printf ("Tracing prefetch\n");
#endif
  
  /* Get write header */
  header = S_get_trace_word (pnode);
  if (header != L_TRACE_PREFETCH)
    T_error(pnode, sint, "expecting L_TRACE_PREFETCH not %i.",
	    header);
  
  /* Get prefetch address */
  sint->trace.mem_addr = S_get_trace_word (pnode);
  
  /* Mark as cache directive */
  sint->flags |= CACHE_DIRECTIVE;
  
  /* Save address so can guess off path addresses */
  /* Done in S_gen_sint and S_skip now */
}

void 
S_trace_store (Pnode * pnode, Sint * sint)
{
  int header;
  
#ifdef DEBUG_TRACE
  printf ("Tracing store\n");
#endif
  
  /* Get write header */
  header = S_get_trace_word (pnode);
  if (header != L_TRACE_WRITE)
    T_error(pnode, sint, "expecting L_TRACE_WRITE not %i.",
	    header);
  
  /* Get store address */
  sint->trace.mem_addr = S_get_trace_word (pnode);
  
  /* Save address so can guess off path addresses */
  /* Done in S_gen_sint and S_skip now */
}

/* Traces MEM_COPY and MEM_COPY_TAG */
void 
S_trace_mem_copy (Pnode * pnode, Sint * sint)
{
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int addr;
#endif
  int header;
  
#ifdef DEBUG_TRACE
  printf ("Tracing mem_copy\n");
#endif
  
  /* Get write header */
  header = S_get_trace_word (pnode);
  if (header != L_TRACE_MEM_COPY)
    T_error(pnode, sint, "expecting L_TRACE_MEM_COPY not %i.",
	    header);
  
  /* Get buf addr, array adrr, the setup value, and id */
  sint->trace.mem_copy_array = S_get_trace_word (pnode);
  sint->trace.mem_copy_buf = S_get_trace_word (pnode);
  sint->trace.mem_copy_setup = S_get_trace_word (pnode);
  sint->trace.mem_copy_count = S_get_trace_word (pnode);
  
  /* Mark as a mem_copy directive */
  sint->flags |= MEM_COPY_DIRECTIVE | CACHE_DIRECTIVE;
}

void 
S_trace_mem_copy_back (Pnode * pnode, Sint * sint)
{
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int addr;
#endif
  int header;
  
#ifdef DEBUG_TRACE
  printf ("Tracing mem_copy_back\n");
#endif
  
  /* Get write header */
  header = S_get_trace_word (pnode);
  if (header != L_TRACE_MEM_COPY_BACK)
    T_error(pnode, sint, "expecting L_TRACE_MEM_COPY_BACK not %i.",
	    header);
  
  /* Get buf addr, array adrr, the setup value, and id */
  sint->trace.mem_copy_array = S_get_trace_word (pnode);
  sint->trace.mem_copy_buf = S_get_trace_word (pnode);
  sint->trace.mem_copy_setup = S_get_trace_word (pnode);
  sint->trace.mem_copy_count = S_get_trace_word (pnode);
  
  /* Mark as a mem_copy directive */
  sint->flags |= MEM_COPY_DIRECTIVE | CACHE_DIRECTIVE;
}

void 
S_trace_mem_copy_check (Pnode * pnode, Sint * sint)
{
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  int addr;
#endif
  int header;
  
#ifdef DEBUG_TRACE
  printf ("Tracing mem_copy_check\n");
#endif
  
  /* Get write header */
  header = S_get_trace_word (pnode);
  if (header != L_TRACE_MEM_COPY_CHECK)
    T_error(pnode, sint, "expecting L_TRACE_MEM_COPY_CHECK not %i.",
	    header);
  
  /* Get the buffer to check */
  sint->trace.mem_copy_buf = S_get_trace_word (pnode);
  
  /* Mark as a mem_copy directive.  Not cache directive because returns */
  sint->flags |= MEM_COPY_DIRECTIVE;
}

void 
S_trace_mem_copy_reset (Pnode * pnode, Sint * sint)
{
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int header, addr;
#endif
  
  /* No trace information, just want to mark as a mem_copy directive */
  sint->flags |= MEM_COPY_DIRECTIVE | CACHE_DIRECTIVE;
}

/* 
 * Guess routines
 */
void 
S_guess_load (Pnode * pnode, Sint * sint)
{
  /* Guess memory address using last memory address for this load */
  sint->trace.mem_addr = sint->oper->last_addr;
}

void 
S_guess_prefetch (Pnode * pnode, Sint * sint)
{
  /* Guess memory address using last memory address for this load */
  sint->trace.mem_addr = sint->oper->last_addr;
}

void 
S_guess_store (Pnode * pnode, Sint * sint)
{
  /* Guess memory address using last memory address for this load */
  sint->trace.mem_addr = sint->oper->last_addr;
}

/* 
 * Intialize opc_tab for use by trace tools
 */
void
S_init_opcode_trace()
{
  int i;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  char *type;
#endif
  S_Opc_Info *info;
  
  for (i=0; i <= S_max_opc; i++)
    {
      info = &opc_info_tab[i];
      
      /* Set defaults to be changed when necessary */
      info->is_branch = 0;
      info->trace_info = NULL;
      info->guess_info = NULL;
      info->trace_words_read = 0;
      
      
      /* If opcode not set in info file, goto next opcode -JCG 1/22/98 */
      if (info->opc_type == -1)
	continue;
      
      switch (info->opc_type)
	{
	case UNTRACED_OPC:
	case MOVE_OPC:
	case IALU_OPC:
	case FALU_OPC:
	  break;
	case CBR_OPC:
	  info->is_branch = 1;
	  info->trace_info = S_trace_cbr;
	  info->trace_words_read = -1;
	  break;
	case JMP_OPC:
	case JRG_OPC:
	  info->is_branch = 1;
	  info->trace_info = S_trace_jmp;
	  info->trace_words_read = -1;
	  break;
	case JSR_OPC:
	  info->is_branch = 1;
	  info->trace_info = S_trace_jsr;
	  info->trace_words_read = -1;
	  break;
	case RTS_OPC:
	  info->is_branch = 1;
	  info->trace_info = S_trace_rts;
	  info->trace_words_read = -1;
	  break;
	case LOAD_OPC:
	  info->trace_info = S_trace_load;
	  info->guess_info = S_guess_load;
	  
	  if (S_use_skipped_memory_addresses)
	    info->trace_words_read = -1;
	  else
	    info->trace_words_read = 1;
	  /* 
	   * Setting S_use_skipped_memory_address sets trace_words_read
	   * for memory operations back to -1 so 
	   * that their last memory address would be saved even when
	   * they were being skipped.  This will slow down 
	   * S_skip() some but it is more consistent with what
	   * we did in the past. -John 3/18/94
	   */
	  break;
	  
	case CHECK_OPC:
	  info->trace_info = S_trace_check;
	  info->trace_words_read = 1;
	  break;
	  
	case PREFETCH_OPC:
	  info->trace_info = S_trace_prefetch;
	  info->guess_info = S_guess_prefetch;
	  if (S_use_skipped_memory_addresses)
	    info->trace_words_read = -1;	
	  else
	    info->trace_words_read = 2;	/* has PREFETCH header */
	  break;
	case STORE_OPC:
	  info->trace_info = S_trace_store;
	  info->guess_info = S_guess_store;
	  if (S_use_skipped_memory_addresses)
	    info->trace_words_read = -1;	
	  else
	    info->trace_words_read = 2;
	  break;
	  
	case MEM_COPY_TAG_OPC:
	case MEM_COPY_OPC:
	  info->trace_info = S_trace_mem_copy;
	  info->trace_words_read = 5;
	  break;
	  
	case MEM_COPY_BACK_OPC:
	  info->trace_info = S_trace_mem_copy_back;
	  info->trace_words_read = 5;
	  break;
	  
	case MEM_COPY_CHECK_OPC:
	  info->trace_info = S_trace_mem_copy_check;
	  info->trace_words_read = 2;
	  break;
	  
	/* No trace info for reset currently */
	case MEM_COPY_RESET_OPC:
	  info->trace_info = S_trace_mem_copy_reset;
	  info->trace_words_read = 0;
	  break;
	  
	default:
	  S_punt ("Unknown opc_type %i for opc %i (%s).", info->opc_type,
		  i, info->name);
	}
    }
}

Trace_Block *
S_get_trace_block (Pnode * pnode)
{
  Trace_Block *tblock;
  int bytes_read;
  int i;
#ifdef WIN32
  char *msg_buf;
#endif
  
  tblock = (Trace_Block *) L_alloc (Trace_Block_pool);
  
#ifdef WIN32
  if (!ReadFile (S_trace_fd, tblock->trace_block,
		 sizeof(int) * TRACE_BLOCK_SIZE, &bytes_read, NULL) )
    {
      /* Treat broken pipe as end of file reached */
      if (GetLastError() == ERROR_BROKEN_PIPE) {
	bytes_read = 0;
      }
      else {
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		      FORMAT_MESSAGE_FROM_SYSTEM,
		      NULL, GetLastError(), 0, (LPTSTR) &msg_buf, 1, NULL);
	
	S_punt ("read_trace: Could not read trace file: %s", msg_buf);
	LocalFree( msg_buf );
      }
    }
#else
  /* Read block of trace */
  bytes_read = read (S_trace_fd, tblock->trace_block, 
		     sizeof(int) * TRACE_BLOCK_SIZE);
#endif
  /* Get number of int's read */
  tblock->size = bytes_read >> 2;
  
  /* Update stats on # of words read from trace */
  S_trace_words_read += tblock->size;
  
  /* If we need to reverse byte order, do it now */
  if (S_trace_byte_order_reversed)
    {
      for (i=0; i < tblock->size; i++)
	tblock->trace_block[i] = SWAP_BYTES (tblock->trace_block[i]);
    }
  
  return (tblock);
}

int 
S_get_trace_word (Pnode * pnode)
{
  Trace_Manager *tmanager;
  int trace_word;
  
  tmanager = pnode->trace_manager;
  
  if (tmanager->size_left == 0)
    {
      /* Free old trace block if allocated */
      if (tmanager->trace_block != NULL)
	L_free (Trace_Block_pool, tmanager->trace_block);
      
      /* Get next trace block */
      tmanager->trace_block = S_get_trace_block (pnode);
      tmanager->trace_ptr = tmanager->trace_block->trace_block;
      tmanager->size_left = tmanager->trace_block->size;
      
      /* Make sure not at end of trace */
      if (tmanager->size_left <= 0)
	{
	  fprintf (stderr, "%s: Read past end of trace at word %i.\n", 
		   tmanager->name, S_trace_words_read);
	  fprintf (stderr, 
		   "Either the probed program terminated unexpectedly or\n");
	  fprintf (stderr,
		   "the simulation did not detect end of program.\n");
	  S_close_trace_fd();
	  S_punt ("Trace error: cannot continue.");
	}
    }
  
  trace_word = *tmanager->trace_ptr;
  tmanager->trace_ptr++;
  tmanager->size_left--;
  return (trace_word);
}

int 
S_peek_trace_word (Pnode * pnode)
{
  Trace_Manager *tmanager;
  int trace_word;
  
  tmanager = pnode->trace_manager;
  
  if (tmanager->size_left == 0)
    {
      /* Free old trace block if allocated */
      if (tmanager->trace_block != NULL)
	L_free (Trace_Block_pool, tmanager->trace_block);
      
      /* Get next trace block */
      tmanager->trace_block = S_get_trace_block (pnode);
      tmanager->trace_ptr = tmanager->trace_block->trace_block;
      tmanager->size_left = tmanager->trace_block->size;
      
      /* Make sure not at end of trace */
      if (tmanager->size_left <= 0)
	{
	  fprintf (stderr, "%s: Read past end of trace at word %i.\n", 
		   tmanager->name, S_trace_words_read);
	  fprintf (stderr, 
		   "Either the probed program terminated unexpectedly or\n");
	  fprintf (stderr,
		   "the simulation did not detect end of program.\n");
	  S_close_trace_fd();
	  S_punt ("Trace error: cannot continue.");
	}
    }
  
  /* Just peek at word, don't update pointers */
  trace_word = *tmanager->trace_ptr;
  return (trace_word);
}

void 
S_read_trace_info (Pnode * pnode, Sint * sint)
{
  int cb_id;
  unsigned pred_val, flag_val;
  S_Oper *op;
  int op_flags;
  void (*trace_info)();
  int i, last_dest, first_dest;
  short *operand;
  int pred_squashed;
  
  if (S_trace_objects && S_is_asynch_tr(S_peek_trace_word(pnode)))
    S_read_obj_trace(pnode);
  
  /* Get op and op flags for ease of use */
  op = sint->oper;
  op_flags = op->flags;
  
#if 0
  /* Debug */
  fprintf (debug_out, "%s op %i: tracing %s.\n",
	   op->cb->fn->name, op->lcode_id, opc_info_tab[op->opc].name);
#endif
  
  /* If at first instruction in a traced cb, read cb id */
  if (op_flags & TRACED_CB_ENTRY)
    {
      cb_id = - S_get_trace_word (pnode);
      
#if CHECK_TRACE
      if (cb_id != op->cb->lcode_id)
	T_error (pnode, sint, "Expecting cb %i not %i.",
		 op->cb->lcode_id, cb_id);
#else
      if (cb_id < 0)
	T_error (pnode, sint, "Expecting cb id not %i.",
		 cb_id);
#endif
    }
  
  /* Assume instruction is not predicate squashed */
  pred_squashed = 0;
  
  /* 
   * If instruction predicated, get predicate value.
   */
  if (op_flags & PREDICATED)
    {
      pred_val = S_get_trace_word (pnode);
      
      /* Pred squash if 0 */
      if (pred_val == 0)
	{
	  sint->flags |= PRED_SQUASHED;
	  
	  /* Mark that this instruction is predicate squashed */
	  pred_squashed = 1;
	  
	}
      
      /* Debug, should be 1 now then */
      else if (pred_val != 1)
	T_error (pnode, sint, "Expecting predicate value not %i.",
		 pred_val);
#if CHECK_TRACE
#endif
    }
  
  /* 
   * If instruction is PROMOTED and we are tracing promoted predicates,
   * get the promoted value (even when squashed)
   */
  if (op_flags & TRACE_PROMOTED_PRED)
    {
      pred_val = S_get_trace_word (pnode);
      
      /* Predicate before promotion squashed if 0 */
      if (pred_val == 0)
	{
	  sint->flags |= PROMOTED_PRED_SQUASHED;
	}
      
      /* Debug, should be 1 now then */
      else if (pred_val != 1)
	T_error (pnode, sint, "Expecting promoted predicate value not %i.",
		 pred_val);
    }
  
  /* Get trace info about instruction if not predicate squashed */
  if (!pred_squashed)
    {
      /*
       * If instruction is a implicit memory operation,
       * read in memory address from trace now.
       */
      if (op_flags & IMPLICIT_MEMORY_OP)
	{
	  sint->trace.mem_addr = S_get_trace_word (pnode);
	}
      
      /* If not at exit, read trace info if any */
      if (!(op_flags & SIM_EXIT_POINT))
	{
	  trace_info = opc_info_tab[op->opc].trace_info;
	  
	  /* Call trace routine for op (if exists)  */
	  if (trace_info != NULL) {
	    if (S_trace_objects && S_is_asynch_tr(S_peek_trace_word(pnode)))
	      S_read_obj_trace(pnode);

	    trace_info (pnode, sint);
	    
	    if (sint->flags & PRED_SQUASHED)
	      op->last_addr = op->pc;
	    else 
	      op->last_addr = sint->trace.mem_addr;
	  }
	}
      
      /* 
       * If at exit point, read end of sample and trace markers,
       * Set exit flag and return.
       */
      else
	{
	  S_trace_exit_point (pnode, sint, 0);
	}
    }
  
  /* 
   * If a predicate define instruction (may include pred_lds later),
   * read the values of the predicates defined.
   *
   * Read even if predicate squashed (some pred_defs change the
   * value even when squashed)
   */
  if (op_flags & PRED_DEF)
    {
      first_dest = S_first_dest;
      last_dest = S_last_dest;
      operand = op->operand;
      
      for (i= first_dest; i <= last_dest; i++)
	{
	  /* If have register (predicate) dest, read value */
	  if (operand[i] > 0)
	    {
	      pred_val = S_get_trace_word (pnode);
	      
	      /* Predicate set to 1 if 1 */
	      if (pred_val == 1)
		{
		  /* Sanity check, for now assume max 2 pred dests */
		  if ((i - first_dest) > 1)
		    S_punt ("Oops, assumed only 2 predicate dests...");
		  
		  /* Set the appropriate pred dest flag */
		  flag_val = PRED_DEST0_SET << i;
		  sint->flags |= flag_val;
		}
	      /* Debug, should be 0 now then */
	      else if (pred_val != 0)
		T_error (pnode, sint, 
			 "Expecting pred def dest[%i] value not %i.",
			 i - S_first_dest, pred_val);
	      
#if 0
	      fprintf (debug_out, "%s op %i: pred dest[%i] (%s) traced.\n",
		       op->cb->fn->name, op->lcode_id, i,
		       operand_tab[operand[i]]->string);
#endif
	    }
	}
    }
}


/* 
 * Skips 'skip_count' instructions in the trace.
 * Returns the pc after 'skip_count'.
 *
 * A great deal of time is spent in this routine, so effort was made
 * to make it run quickly (with some loss of clarity).
 *
 * A lot of time was found doing label loads, so invariant addresses
 * are put in local variables (pointers).  Also, some of the code
 * from the trace manager is put in the inner loop.
 */
int 
S_skip (int pc, int skip_count)
{
  int num_skipped, trace_words_read, num_packets_skipped;
  int **trace_ptr_ptr, *size_left_ptr;
  int size_left;
  int *trace_ptr;
  int *end_of_program;
  S_Oper **op_tab;
  S_Oper *oper;
  Sint sint, branch_sint;
  Pnode *pnode;
  int mem_addr;
  int oper_flags = 0;
  int use_skipped_memory_addresses;
  int region_entry_num_skipped, region_entry_num_packets_skipped;
  int op_branched;
  Region_Stats *rstats;
  S_Opc_Info  *info;
  
  /* Get pnode for ease of use */
  pnode = S_pnode;
  
  /* Get trace manager fields for ease of use */
  trace_ptr_ptr = &pnode->trace_manager->trace_ptr;
  size_left_ptr = &pnode->trace_manager->size_left;
  
  /* Get use_skipped_memory_addresses flag */
  use_skipped_memory_addresses = S_use_skipped_memory_addresses;
  
  /* Get pointer to global S_end_of_program */
  end_of_program = &S_end_of_program;
  
  /* If at end of program, return now */
  if (*end_of_program)
    return (pc);
  
  /* Get local pointer to oper_tab */
  op_tab = oper_tab;
  
  /* Initialize number skipped */
  num_skipped = 0;
  num_packets_skipped = 0;
  
  /* Initialize the region entry num skipped to 0 (start of program ) */
  region_entry_num_skipped = 0;
  region_entry_num_packets_skipped = 0;
  
  /* Get region stats structure */
  rstats = pnode->stats->region;
  
  /* Mark sint with serial_no of -1 to mark in skip code */
  sint.serial_no = -1;
  
  /* 
   * Preload trace_ptr and size_left and save and load around
   * "long" part of code below.
   */
  size_left = *size_left_ptr;
  trace_ptr = *trace_ptr_ptr;
  
  /* Init the branched flag (only needed for playdoh code) */
  op_branched = 0;
  
  /* 
   * Loop until skip desired number of instructions or hit end of program 
   * Use outer loop to force skipping of whole vliw long words
   */
  while (1)
    {
      while (1)
	{
	  /* 
	   * If last oper was end of a vliw packet and we are doing
	   * playdoh vliw simulation, then if an op in the liw branched
	   * we need to get the new pc.
	   */
	  if (op_branched && (oper_flags & END_PACKET) &&
			!(branch_sint.oper->flags & SIM_EXIT_POINT))
	    {
	      
	      /* Save size_left and trace_ptr to global memory */
	      *size_left_ptr = size_left;
	      *trace_ptr_ptr = trace_ptr;
	      
	      info = &opc_info_tab[branch_sint.oper->opc];
	      switch (info->opc_type)
		{
		case CBR_OPC:
		case JMP_OPC:
		case JRG_OPC:
		  S_trace_jmp_finish(pnode,&branch_sint);
		  break;
		case JSR_OPC:
		  S_trace_jsr_finish(pnode,&branch_sint);
		  break;
		case RTS_OPC:
		  S_trace_rts_finish(pnode,&branch_sint);
		  break;
		default:
		  S_punt("unknown op type that branched during liw: %d",
			 info->opc_type);
		  break;
		}
	      if (!(branch_sint.flags & UNTRACED_JSR))
		pc = branch_sint.trace.target_pc;
	      
		/* Reset op_branched for next liw */
	      op_branched = 0;
	      
	      /* 
	       * Reload size_left and trace_ptr from 
	       * global memory to registers 
	       */
	      size_left = *size_left_ptr;
	      trace_ptr = *trace_ptr_ptr;
	    }
	  
	  /* 
	   * Get oper for ease of use, moved above branch so branch would
	   * fill load slot (on hp)
	   */
	  oper = op_tab[pc];
	  
	  /* Exit loop if skipped enough instuctions */
	  if (num_skipped >= skip_count)
	    break;
	  
	  /* 
	   * Get number of trace words this instruction reads.
	   * This will be negative for branches and some predicate
	   * instructions that read a variable number of trace words.
	   */
	  trace_words_read = oper->trace_words_read;
	  
	  /* Assume instruction will not branch */
	  pc++;
	  
	  /* The the oper flags (for the START_PACKETS check) */
	  oper_flags = oper->flags;
	  
	  /* Count the number of packets (START_PACKETs) skipped */
	  if (oper_flags & START_PACKET)
	    num_packets_skipped++;
	  
	  /* Count this instruction as an instruction skipped */
	  num_skipped++;
	  
	  /* No trace words read, goto next instruction. The 70% case. */
	  if (trace_words_read == 0)
	    continue;
	  
	  /* 
	   * 1 or more trace words read. The 18% case.
	   * Have inlined part of S_get_trace word to allow
	   * quick skipping of trace information.
	   *
	   * Use unsigned compair to test both for enough words
	   * in buffer to skip and that trace_words_read >= 0.
	   */
	  if (((unsigned) size_left) >= ((unsigned)trace_words_read))
	    {
	      size_left -= trace_words_read;
	      trace_ptr += trace_words_read;
	      continue;
	    }
	  
	  /* 
	   * Long case, handles all possiblities (even if above tests were
	   * not done).  The 12% case.
	   */
	  
	  /* Save size_left and trace_ptr to global memory */
	  *size_left_ptr = size_left;
	  *trace_ptr_ptr = trace_ptr;
	  
	  
	  /* Fix, do long process for only branches */
	  if (trace_words_read == -1)
	    {
	      sint.oper = oper;
	      sint.fn = oper->cb->fn;
	      sint.flags = 0;
	      S_read_trace_info (pnode, &sint);
	      
	      /* 
	       * If we are simulating playdoh vliw then the cb id will
	       * not come through until the end of the liw.
	       */
	      if (S_processor_model != PROCESSOR_MODEL_PLAYDOH_VLIW)
		{
		  /* Assumed fall through above, if branch set to target pc */
		  if (sint.flags & BRANCHED)
		    pc = sint.trace.target_pc;
		}
	      /* Need to save the sint and flag that there was a branch */
	      else
		{
		  if (sint.flags & BRANCHED)
		    {
		      op_branched = 1;
		      branch_sint = sint;
		    }
		}
	      /*
	       * Update memory address if use_skipped_memory_addresses
	       * (Do for all cases, even non-memory operations.)
	       */
	      mem_addr = sint.trace.mem_addr;
	      if (use_skipped_memory_addresses)
		oper->last_addr = mem_addr;
	      
	      /* Detect if this instruction changes simulation state */
	      if (oper->flags & CHANGES_STATE)
		{
		  /* Detect force simulation directives */
		  if (oper->flags & FORCE_SIM_ON)
		    {
		      /* Mark that we are forcing simulation */
		      S_force_sim = 1;
		      
		      /* 
		       * Force skip count to num_skipped, to cause us
		       * to enter simulation.
		       */
		      skip_count = num_skipped;
		      
		      if (S_debug_force_sim_markers)
			{
			  fprintf (debug_out, 
				   "%s op %i: force sim on (S_skip) ",
				   oper->cb->fn->name, oper->lcode_id);
			  fprintf (debug_out,
				   "(skip_count %i)\n", skip_count);
			}
		    }
		  
		  /* 
		   * May get FORCE_SIM_OFF.  S_force_sim better be 0.
		   */
		  if (oper->flags & FORCE_SIM_OFF)
		    {
		      if (S_force_sim != 0)
			S_punt ("S_skip: FORCE_SIM_OFF and S_force_sim = 1");
		      if (S_debug_force_sim_markers)
			{
			  fprintf (debug_out, 
				   "%s op %i: force sim off (S_skip)\n",
				   oper->cb->fn->name, oper->lcode_id);
			}
		    }
		  
		  /* 
		   * Update regions if start or stop region attribute 
		   * and really changing regions.
		   */
		  if ((oper->flags & REGION_BOUNDARY) &&
		      (pnode->stats != oper->stats))
		    {
		      /* Update current region stats before switching regions */
		      rstats->num_skip_on_path += (num_skipped - 
						   region_entry_num_skipped);
		      
		      /* Factor out billions to keep from overflowing */
		      while (rstats->num_skip_on_path > BILLION)
			{
			  rstats->num_skip_on_path -= BILLION;
			  rstats->billions_skipped++;
			}
		      
		      region_entry_num_skipped = num_skipped;
		      
		      /* Update current region stats, before switching regoins */
		      rstats->num_packets_skip_on_path += (num_packets_skipped - 
							   region_entry_num_packets_skipped);
		      
		      /* Factor out billions to keep from overflowing */
		      while (rstats->num_packets_skip_on_path > BILLION)
			{
			  rstats->num_packets_skip_on_path -= BILLION;
			  rstats->billion_packets_skipped++;
			}			
		      
		      region_entry_num_packets_skipped = num_packets_skipped;
		      
		      /* Make sure not nesting regions */
		      if ((oper->flags & STATS_ON_ATTR) &&
			  (pnode->stats != S_program_stats))
			{
			  S_punt ("%s op %i: nested region inside region starting at '%s'.\n",
				  oper->cb->fn->name, 
				  oper->lcode_id,
				  pnode->stats->name);
			}
		      
		      /* Same code for stats on, stats off region switching */
		      rstats->num_exits++;
		      oper->stats->region->num_entries++;
		      pnode->stats = oper->stats;
		      rstats = oper->stats->region;
		    }
		}
	    }
	  else
	    {
	      /*
	       * Skip over trace words using function call
	       * (so that trace buffer will be reloaded)
	       */
	      while(trace_words_read > 0)
		{
		  S_get_trace_word (pnode);
		  trace_words_read--;
		}
	    }
	  
	  /* Reload size_left and trace_ptr from global memory to registers */
	  size_left = *size_left_ptr;
	  trace_ptr = *trace_ptr_ptr;
	  
	  /* Exit if end of program, update count and exit loop */
	  if (*end_of_program)
	    break;
	}
      
      /* Exit if end of program, update count and exit loop */
      if (*end_of_program)
	break;
      
      /* 
       * If scheduling info is available, force skipping to stop on the
       * start of a vliw long word.  This should help superscalar code
       * also since the simulator will start on the first instruction
       * in a scheduled cycle.
       */
      if (num_skipped >= skip_count)
	{
	  /* Stop now if no scheduling info exists */
	  if (!S_sched_info_avail)
	    break;
	  
	  /* 
	   * Stop only if at START_PACKET (start of scheduled cycle) 
	   * (or if we are skipping nothing)
	   */
	  oper = op_tab[pc];
	  
	  if ((oper->flags & START_PACKET) || (skip_count == 0))
	    break;
	  
	  /* Otherwise, increment skip count to skip another instruction */
	  else
	    skip_count ++;
	}
      else
	S_punt ("S_skip: algorithm error.");
    }
  
  /* Save size_left and trace_ptr to global memory */
  *size_left_ptr = size_left;
  *trace_ptr_ptr = trace_ptr;
  
  /* Update stats */
  S_num_skip_on_path += num_skipped;
  
  /*
   * Use another counter to count the "billions" of instructions skipped
   */
  while (S_num_skip_on_path > BILLION)
    {
      S_billions_skipped++;
      S_num_skip_on_path -= BILLION;
    }
  
  S_num_packets_skip_on_path += num_packets_skipped;
  
  /* Factor out billions to keep from overflowing */
  while (S_num_packets_skip_on_path > BILLION)
    {
      S_billion_packets_skipped++;
      S_num_packets_skip_on_path -= BILLION;
    }
  
  /* Update region stats */
  rstats->num_skip_on_path += (num_skipped - region_entry_num_skipped);
  
  /* Factor out billions to keep from overflowing */
  while (rstats->num_skip_on_path > BILLION)
    {
      rstats->num_skip_on_path -= BILLION;
      rstats->billions_skipped++;
    }
  
  rstats->num_packets_skip_on_path += (num_packets_skipped - 
				       region_entry_num_packets_skipped);
  
  /* Factor out billions to keep from overflowing */
  while (rstats->num_packets_skip_on_path > BILLION)
    {
      rstats->num_packets_skip_on_path -= BILLION;
      rstats->billion_packets_skipped++;
    }
  
  /* If doing x86_trace, print out how many instructions were skipped */
  if (S_mode == X86_TRACE_GENERATOR)
    S_write_x86_info_int (X86_INSTRUCTIONS_SKIPPED, num_skipped);
  
  return (pc);
}

void 
S_update_region_cycle_counts (Sint *sint, int sim_cycle)
{
  int opflags;
  
  /* Get operation flags for ease of use */
  opflags = sint->oper->flags;
  
  if (opflags & STATS_ON_ATTR)
    {
      /* Make sure we are leaving the program region */
      if (sint->stats != S_program_stats)
	{
	  S_punt ("%s op %i: nested region inside region starting at '%s'.\n",
		  sint->oper->cb->fn->name, 
		  sint->oper->lcode_id,
		  sint->stats->name);
	}
      /* Calculate cycles of region we are leaving */
      sint->stats->region->num_sim_cycles += 
	(sim_cycle - sint->stats->region->entry_cycle);
      
#if 0
      /* Debug */
      fprintf (debug_out, 
	       "Exiting program region %s at %s op %i: %i to %i, diff %i.\n",
	       sint->stats->name,
	       sint->oper->cb->fn->name,
	       sint->oper->lcode_id,
	       sint->stats->region->entry_cycle,
	       sim_cycle, 	
	       sim_cycle - 
	       sint->stats->region->entry_cycle);
      fprintf (debug_out, 
	       "Entering loop region %s: cycle %i\n",
	       sint->oper->stats->name, sim_cycle);
#endif
      /* Mark time entered new region */
      sint->oper->stats->region->entry_cycle = sim_cycle;
    }
  /* If leaving region, attribute partial cycles towards
   * code inside region.
   */
  else
    {
      /* Calculate cycles of region we are leaving */
      sint->stats->region->num_sim_cycles += 
	(sim_cycle + 1) - 
	sint->stats->region->entry_cycle;
      
#if 0
      /* Debug */
      fprintf (debug_out, 
	       "Exiting loop region %s at %s op %i: %i to %i, diff %i.\n",
	       sint->stats->name,
	       sint->oper->cb->fn->name,
	       sint->oper->lcode_id,
	       sint->stats->region->entry_cycle,
	       sim_cycle, 	
	       (sim_cycle + 1) - 
	       sint->stats->region->entry_cycle);
      
      fprintf (debug_out, 
	       "Entering program region %s: cycle %i\n",
	       sint->oper->stats->name, sim_cycle);
#endif
      /* Mark time entered new region */
      sint->oper->stats->region->entry_cycle = sim_cycle + 1;
      
    }
}

Sint *
S_gen_sint (Pnode *pnode, int pc, int read_trace)
{
  Sint *sint;
  S_Opc_Info *info;
  S_Oper *oper;
  unsigned opflags;
  Region_Stats *rstats;
  
  /* Allocate sint */
  sint = (Sint *) L_alloc (Sint_pool);
  
  /* 
   * These sint fields must be initialized before reading/guessing
   * trace info.
   */
  oper = oper_tab[pc];
  sint->oper = oper;
  sint->fn = oper->cb->fn;
  sint->flags = 0;
  /* TLJ 3/7/97 - copy to sint so I can modify these on a per-sint basis */
  sint->playdoh_flags = oper->playdoh_flags;
  
  /* Initialize other sint fields, if necessary */
  sint->serial_no = S_serial_no;
  S_serial_no++;
  sint->entry_list = NULL;
  
  /* Initialize processor data fields for debugging purposes */
  sint->proc_data_v = NULL;
  sint->superscalar = NULL;
  sint->vliw = NULL;
  
  /* Get opc info for easy access */
  info = &opc_info_tab[GET_OPC(sint)];
  
  /* Set the conflict mask and access size for all sints */
  sint->conflict_mask = info->conflict_mask;
  sint->access_size = info->access_size;
  
  /* Get the stats region this sint falls in */
  sint->stats = pnode->stats;
  rstats = sint->stats->region;
  
  /* If read_trace, get trace info for sint */
  if (read_trace)
    {
      S_read_trace_info (pnode, sint);
      
      /* 
       * Update last_addr for memory operations (do for all so don't
       * need branch)
       */
      oper->last_addr = sint->trace.mem_addr;
      
      /* Get the operation flags for ease of use */
      opflags = oper->flags;
      
      /* 
       * If the operation has the potential to change simulation
       * state, then do more extensive testing of flags.
       */
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
			   "%s op %i: force sim on (S_gen_sint)\n",
			   oper->cb->fn->name, oper->lcode_id);
		}
	    }
	  else if (opflags & FORCE_SIM_OFF)
	    {
	      S_force_sim = 0;
	      
	      if (S_debug_force_sim_markers)
		{
		  fprintf (debug_out, 
			   "%s op %i: force sim off (S_gen_sint)\n",
			   oper->cb->fn->name, oper->lcode_id);
		}
	    }
	  
	  /* 
	   * Single end of program if at stop sim marker and
	   * have encountered stop_sim markes S_stop_sim_trip_count times.
	   */
	  if (opflags & STOP_SIM)
	    {
	      S_stop_sim_markers_encountered ++;
	      
	      if (S_debug_stop_sim_markers)
		{
		  fprintf (debug_out, 
			   "%s op %i: tripped stop sim marker #%i\n",
			   sint->fn->name, oper->lcode_id,
			   S_stop_sim_markers_encountered);
		}
	      
	      if (S_stop_sim_markers_encountered >= S_stop_sim_trip_count)
		{
		  S_end_of_program = 1;
		  if (S_debug_stop_sim_markers)
		    {
		      fprintf (debug_out, 
			       "%s op %i: stop_sim marker terminating program\n",
			       sint->fn->name, oper->lcode_id);
		    }
		}
	    }
	  
	  /* 
	   * Detect change in simulation region - only on path, 
	   * and for real region changes, i.e., not to same region
	   */
	  if ((opflags & REGION_BOUNDARY) &&
	      (oper->stats != pnode->stats))
	    {
	      
	      /* Same code for stats on, stats off region switching */
	      /* Update region exit count */
	      pnode->stats->region->num_exits++;
	      
	      /* Switch regions */
	      pnode->stats = oper->stats;
	      
	      /* 
	       * Do not switch stats pointers, because sint is still
	       * in old region.
	       */
	      /* Update region entry count */
	      pnode->stats->region->num_entries++;
	      
	      /* Mark that this sint changes regions */
	      sint->flags |= CHANGES_REGION;
	    }
	}
      
      /* Keep stats about instruction stream */
      S_num_sim_on_path++;	/* Must also be kept globally */
      rstats->num_sim_on_path++;
      
      if (opflags & START_PACKET)
	{
	  S_num_packets_sim_on_path++; /* Should also be kept globally */
	  rstats->num_packets_sim_on_path++;
	}
      
      if (info->is_branch)
	{
	  rstats->branches++;
	  
	  if (sint->flags & LONGJMP)
	    {
	      rstats->longjmps++;
	    }
	}
      
      if (sint->flags & UNTRACED_JSR)
	{
	  /* Update stats */
	  rstats->untraced_jsrs++;
	}
      
      if (opflags & PREDICATED)
	{
	  rstats->total_preds++;
	  
	  if (opflags & PROMOTED)
	    {
	      rstats->promoted_preds++;
	    }
	  
	  if (sint->flags & PRED_SQUASHED)
	    {
	      rstats->total_preds_squashed++;
	      
	      if (opflags & PROMOTED)
		{
		  rstats->promoted_preds_squashed++;
		}
	    }
	}
      else if (opflags & PROMOTED)
        {
	   rstats->promoted_unpreds++;
	}
      
      if (info->opc_type == LOAD_OPC)
	{
	  if ((sint->flags & MASKED_SEG_FAULT) &&
	      !(sint->flags & MASKED_BUS_ERROR))
	    {
	      S_sim_buffer_page_support (sint);
	    }
	  /* Promoted loads (includes speculative promoted loads) */
	  if (opflags & PROMOTED)
	    {
	      rstats->promoted_loads++;
	      
	      if (opflags & NON_TRAPPING)
		{
		  rstats->promoted_non_trapping_loads++;
		  
		  if (sint->flags & MASKED_SEG_FAULT)
		    {
		      if (sint->flags & MASKED_BUS_ERROR)
			rstats->promoted_masked_both_traps++;
		      else
			rstats->promoted_masked_seg_faults++;
		    }
		  
		  else if (sint->flags & MASKED_BUS_ERROR)
		    {
		      rstats->promoted_masked_bus_errors++;
		    }
		}
	    }
	  
	  /* Speculative loads (speculative promoted loads counted above) */
	  else if (opflags & SPECULATIVE)
	    {
	      rstats->speculative_loads++;
	      
	      if (opflags & NON_TRAPPING)
		{
		  rstats->speculative_non_trapping_loads++;
		  
		  if (sint->flags & MASKED_SEG_FAULT)
		    {
		      if (sint->flags & MASKED_BUS_ERROR)
			rstats->speculative_masked_both_traps++;
		      else
			rstats->speculative_masked_seg_faults++;
		    }
		  
		  else if (sint->flags & MASKED_BUS_ERROR)
		    {
		      rstats->speculative_masked_bus_errors++;
		    }
		}
	    }
	  
	  /* Unmarked (non promoted, non speculative) loads */
	  else
	    {
	      rstats->unmarked_loads++;
	      
	      if (opflags & NON_TRAPPING)
		{
		  rstats->unmarked_non_trapping_loads++;
		  
		  if (sint->flags & MASKED_SEG_FAULT)
		    {
		      if (sint->flags & MASKED_BUS_ERROR)
			rstats->unmarked_masked_both_traps++;
		      else
			rstats->unmarked_masked_seg_faults++;
		    }
		    
		  else if (sint->flags & MASKED_BUS_ERROR)
		    {
		      rstats->unmarked_masked_bus_errors++;
		    }
		}
	    }
	}
      
    }
  
  /* Otherwise, call a 'guess' routine if available. */
  else
    { 
      if (S_trace_objects)
	sint->trace.mem_addr = -1; 
      else if (info->guess_info != NULL)
	info->guess_info (pnode, sint);
    }
  
  
  
  /* Return sint just generated */
  return (sint);
}

/* Prints out a trace word and its likely meaning */
void 
print_trace_word (int index, int word)
{
  S_Fn *fn;
  
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
    case L_TRACE_PREFETCH:
      fprintf (stderr, "(L_TRACE_PREFETCH)");
      break;
    case L_TRACE_MEM_COPY:
      fprintf (stderr, "(L_TRACE_MEM_COPY)");
      break;
    case L_TRACE_MEM_COPY_BACK:
      fprintf (stderr, "(L_TRACE_MEM_COPY_BACK)");
      break;
    case L_TRACE_MASKED_SEG_FAULT:
      fprintf (stderr, "(L_TRACE_MASKED_SEG_FAULT)");
      break;
    case L_TRACE_NO_SEG_FAULT:
      fprintf (stderr, "(L_TRACE_NO_SEG_FAULT)");
      break;
    case L_TRACE_CHECK:
      fprintf (stderr, "(L_TRACE_CHECK)");
      break;
    case L_TRACE_ASYNCH:
      fprintf (stderr, "(L_TRACE_ASYNCH)");
      break;
      
    default:
      /* Print out what it most likely is tracing */
      if ((fn = S_get_fn (word)) != NULL)
        {
	  fprintf (stderr, "(Function %s)", fn->name);
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
 * Prints as much debug info (to stderr) as possible when the
 * there is a trace error.
 */
void 
print_debug_info (Pnode * pnode, Sint * sint)
{
  int i;
  Trace_Manager *manager;
  Trace_Block *block;
  int start, end, start_dist, end_dist;
  int *cur;
  
  /* Make sure pnode is not NULL */
  if (pnode == NULL)
    {
      fprintf (stderr, "print_debug_info: pnode NULL\n");
      return;
    }
  
  /* Make sure sint is not NULL */
  if (sint == NULL)
    {
      fprintf (stderr, "print_debug_info: sint NULL\n");
    }
  
  /* Print which function sint is in */
  fprintf (stderr, "Trace error in function %s cb %i:\n", sint->fn->name,
	   sint->oper->cb->lcode_id);
  
  
  /* Print out pc */
  fprintf (stderr, "At pc %i, which is the following code segment:\n",
	   sint->oper->pc);
  
  /* Print the sint */
  S_print_sint (stderr, sint);
  
  fprintf (stderr, "Sint serial no %i.  S_sim_cycle = %i.\n",
	   sint->serial_no, S_sim_cycle);
  
  fprintf (stderr, "\n");
  
  
  /* Get trace manager and trace block for ease of use */
  manager = pnode->trace_manager;
  block = manager->trace_block;
  
  /* Find the distance to the start/end of the trace buffer */
  cur = manager->trace_ptr -1;
  start_dist = cur - block->trace_block;
  end_dist = manager->size_left;
  
  /* 
   * Try to start 20 entries above current point and go to 
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
	   S_trace_words_read);
  
  if (start == (-start_dist))
    fprintf (stderr, "(Start of buffer)\n");
  
  for (i=start; i <= end; i++)
    {
      
      print_trace_word (i, cur[i]);
    }
  
  if (end == end_dist)
    fprintf (stderr, "(End of buffer)\n");
}

/*
 * Prints error message and debug info about trace.
 * Expects (pnode, sint, printf_format_string, ...);
 * Calls S_punt so can put break points there.
 */
void 
T_error(Pnode *pnode, Sint *sint, char *fmt, ...)
{
    va_list     args;
    
    fprintf(stderr, "Trace error: ");
    va_start (args, fmt);
    vfprintf (stderr, fmt, args);
    va_end(args);
    fprintf (stderr,"\n");
    
    /* Print out trace info */
    print_debug_info (pnode, sint);
    
    S_punt ("Cannot continue: trace error.");
}
