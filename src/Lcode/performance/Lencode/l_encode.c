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
 *	File :		l_encode.c
 *	Description :	Encodes lcode for simulation or profiling
 *	Creation Date :	April, 1993
 *	Author : 	John Gyllenhaal, Dan Connors
 *
 *	(C) Copyright 1992, John Gyllenhaal Dan Connors, and Wen-mei Hwu
 *	All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/
#include <config.h>
#include <stdio.h>
#include <Lcode/l_main.h>
#include <Lcode/l_code.h>
#include "l_encode.h"

/*
 * Parameter declaration
 */
char *encode_for = "profiling";
int encode_loop_info = 0;
int encode_sim_loop_info = 0;
int encode_for_aout_linux = 0;
int linking_to_f2c_library = 0;
int filter_out_vararg_jsrs = 1;
int L_do_loop_nest_info = 0;
char *loop_nest_file_name = "NULL";
int L_do_buf_info = 0;
char *buf_info_file_name = "NULL";
FILE *buf_info_file = NULL;
/* 20030414 SZU
 * Extra parameters for str reduction and mem addr val profiling
 */
int do_str_red = 0;
int do_mem_addrs = 0;
#define OBJS_PER_LINE 12

/* 
 * Operand encoding pools for simulation and value_profiling
 */
static L_Alloc_Pool *eoperand_pool = NULL;
static L_Alloc_Pool *eop_pool = NULL;
static L_Alloc_Pool *ecb_pool = NULL;
static L_Alloc_Pool *eoperand_array_pool = NULL;

/*
 * Reads probe inserter parameters
 */
void
L_read_parm_Lencode (Parm_Parse_Info * ppi)
{
  L_read_parm_s (ppi, "encode_for", &encode_for);
  L_read_parm_b (ppi, "encode_loop_info", &encode_loop_info);

  L_read_parm_b (ppi, "encode_sim_loop_info", &encode_sim_loop_info);
  L_read_parm_b (ppi, "encode_for_aout_linux", &encode_for_aout_linux);
  L_read_parm_b (ppi, "linking_to_f2c_library", &linking_to_f2c_library);
  L_read_parm_b (ppi, "filter_out_vararg_jsrs", &filter_out_vararg_jsrs);
  
  /* HCH: 3/10/01 - print loop nest info for mem object tracing */
  L_read_parm_b(ppi, "do_loop_nest_info", &L_do_loop_nest_info);
  L_read_parm_s(ppi, "loop_nest_file_name",
		&loop_nest_file_name);
  /* HCH: 5/7/01 - parms for loop buffer experiments */
  L_read_parm_b(ppi, "?do_buf_info", &L_do_buf_info);
  L_read_parm_s(ppi, "?buf_info_file_name",
		&buf_info_file_name);
  /* 20030414 SZU
   * Parms to separate strength reduction and memory address
   * value profiling
   */
  L_read_parm_b (ppi, "do_str_red", &do_str_red);
  L_read_parm_b (ppi, "do_mem_addrs", &do_mem_addrs);
}

/*--------------------------------------------------------------------------*/

int
L_ignore_oper (L_Oper * op)
{
  /* Ignore the following opcodes */
  if ((op->opc == Lop_PROLOGUE) ||
      (op->opc == Lop_EPILOGUE) ||
      (op->opc == Lop_PRED_CLEAR) ||
      (op->opc == Lop_PRED_SET) || (op->opc == Lop_DEFINE))
    return (1);

  return (0);
}

int
L_matches_fn_name (char *fn_name, char *test_name)
{
  int matches;

  /* Assume doesn't match */
  matches = 0;

  /* Has fn_name been prefixed with '_$fn_' (hppa only) */
  if ((fn_name[0] == '_') && (fn_name[1] == '$') &&
      (fn_name[2] == 'f') && (fn_name[3] == 'n') && (fn_name[4] == '_'))
    {
      /* Yes, strip off for comparision */
      if (strcmp (&fn_name[5], test_name) == 0)
	matches = 1;
    }

  /* Has fn_name been prefixed with '_' (normal case) */
  else if (fn_name[0] == '_')
    {
      /* Yes, strip off for comparision */
      if (strcmp (&fn_name[1], test_name) == 0)
	matches = 1;
    }

  else
    L_punt ("L_matches_fn_name: No '_' prefix on '%s'", fn_name);

  return (matches);
}

void
L_print_profile_encoding (FILE * out, L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op;
  L_Loop *loop;
  L_Inner_Loop *lp;
  L_Attr *attr = NULL;
  L_Flow *flow_indx;
  int attr_num;
  int size;
  int num_jsrs;
  int asm_index;
  int bb_size;			/* Basic block size, includes branch */
  int is_branch;
  int i;
  int num_opers;
  int mark_inner;
  /* 
   * If want to encode loop info for iteration profiling, call 
   * the routines to annotate lcode with the loop info.
   */
  if (encode_loop_info)
    {
      /* 
       * Using code segment written by Dave August. -JCG 5/6/95 
       */
      /* Dominator analysis is required for loop detection. */
      L_do_flow_analysis (fn, DOMINATOR);

      /* Detect loops without inserting empty preheaders */
      L_loop_detection (fn, 0);

      for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
	{
	  attr = L_new_attr ("Lencode_LoopHeader", 0);

	  attr_num = 0;
	  for (flow_indx = loop->header->src_flow; flow_indx;
	       flow_indx = flow_indx->next_flow)
	    {
	      if (!L_in_cb_DOM_set (flow_indx->src_cb, loop->header->id))
		{
		  L_set_int_attr_field (attr, attr_num,
					flow_indx->src_cb->id);
		  attr_num++;
		}
	    }

	  loop->header->attr = L_concat_attr (loop->header->attr, attr);
	}
    }

  /* Count the number of lines for this function that will be printed */
  size = 0;
  num_jsrs = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      num_opers = 0;
      size++;
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if(L_do_buf_info)
	    num_opers++;

	  if (L_cond_branch_opcode (op))
	    size++;

	  /* Count if predicated or not */
	  else if (L_uncond_branch_opcode (op))
	    size++;

	  else if (L_register_branch_opcode (op))
	    size++;

	  else if (L_subroutine_return_opcode (op))
	    size++;

	  else if (L_subroutine_call_opcode (op))
	    {

	      /* 
	       * Lhppa replaces __builtin_va_start with an add and 4 stores
	       * during phase 1.  
	       * So don't treat this function as a jsr or Lprofile will
	       * get confused during lcode profiling as to why there is 
	       * not a jsr there.
	       *
	       * Lsparc replaces __builtin_va_arg_incr with and add
	       * and a load.  So again don't treat as a jsr.
	       *
	       * For Lemulate, must treat as jsrs or trace will be
	       * misaligned -ITI/JCG 3/99.
	       */
	      if (!L_is_label (op->src[0]) ||
		  !filter_out_vararg_jsrs ||
		  !(L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_start") ||
		    L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_arg_incr")))
		{
		  num_jsrs++;
		  size++;
		}

	    }
	}
      if(L_do_buf_info)
	fprintf (buf_info_file, "%s %d %d\n", 
		 fn->name, cb->id, num_opers);
      
    }

  /* 
   * Determine how much to strip off fn->name to get asm_name using
   * M_arch.
   */
  if ((M_arch == M_SPARC) || ((M_arch == M_X86) && encode_for_aout_linux))
    asm_index = 0;
  else
    asm_index = 1;

  /* Set basic block size to 0 */
  bb_size = 0;

  fprintf (out, "\n");
  fprintf (out,
	   "begin %s %s  Addr: 0x%08x  Size: %-8d  Max_Cb: %-8d  JSRs: %-8d\n",
	   fn->name, &fn->name[asm_index], 0, size, fn->max_cb_id, num_jsrs);

  if(L_do_buf_info)
    {
      PG_setup_pred_graph(fn);
      L_inner_loop_detection(fn, 0);
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* 
       * If encoding loop info, check to see if this cb is a loop header.
       * If it is, print out loop header cb, and all the entry cbs
       */
      if ((encode_loop_info) &&
	  ((attr = L_find_attr (cb->attr, "Lencode_LoopHeader")) != NULL))
	{
	  if(L_do_buf_info)
	    {
	      mark_inner = 0;
	      for (lp = fn->first_inner_loop; lp!=NULL; lp = lp->next_inner_loop)
		{
		  if (lp->cb->id == cb->id)
		    {
		      mark_inner = 1;
		      break;
		    }
		}
	      fprintf (out, "cb_loop_header  %i %i %i\n", cb->id, 
		       attr->max_field, mark_inner);
	    }
	  else
	    fprintf (out, "cb_loop_header  %i %i\n", cb->id, attr->max_field);

	  /* Print out all the cb's the loop can be entered from.
	   * Entries from all other cb's are considered backedges.
	   */
	  for (i = 0; i < attr->max_field; i++)
	    {
#if LP64_ARCHITECTURE
	      fprintf (out, "  _preheader_cb %li\n",
		       (long)attr->field[i]->value.i);
#else
	      fprintf (out, "  _preheader_cb %i\n", attr->field[i]->value.i);
#endif
	    }
	}
      /* Otherwise, print normal cb marker */
      else
	{
	  fprintf (out, "cb %i\n", cb->id);
	}

      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* If it not an ignored opcode, increase basic block size */
	  if (!L_ignore_oper (op))
	    bb_size++;

	  /* Assume it is a branch */
	  is_branch = 1;

	  if (L_cond_branch_opcode (op))
	    fprintf (out, "  br %i\n", bb_size);

	  else if (L_uncond_branch_opcode (op))
	    {
	      if (!L_is_predicated (op))
		fprintf (out, "  jmp %i\n", bb_size);
	      else
		fprintf (out, "  pjmp %i\n", bb_size);
	    }

	  else if (L_register_branch_opcode (op))
	    fprintf (out, "  hash %i\n", bb_size);

	  else if (L_subroutine_return_opcode (op))
	    fprintf (out, "  ret %i\n", bb_size);

	  else if (L_subroutine_call_opcode (op))
	    {
	      /* 
	       * Lhppa replaces __builtin_va_start with an add and 4 stores
	       * during phase 1.  
	       * So don't treat this function as a jsr or Lprofile will
	       * get confused during lcode profiling as to why there is 
	       * not a jsr there.
	       *
	       * Lsparc replaces __builtin_va_arg_incr with and add
	       * and a load.  So again don't treat as a jsr.
	       *
	       * For Lemulate, must treat as jsrs or trace will be
	       * misaligned -ITI/JCG 3/99.
	       */
	      if (!L_is_label (op->src[0]) ||
		  !filter_out_vararg_jsrs ||
		  !(L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_start") ||
		    L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_arg_incr")))
		{
		  fprintf (out, "  jsr %i\n", bb_size);
		}
	    }

	  /* Mark that it is not a branch */
	  else
	    {
	      is_branch = 0;
	    }

	  /* If it is a branch, reset bb_size */
	  if (is_branch)
	    bb_size = 0;
	}
    }
  fprintf (out, "end %s\n", fn->name);

  return;
}


static void
L_print_mem_objs (FILE * out, L_Oper * op)
{
  L_AccSpec * accspec;
  int line_iter = 0;
  Set visited = NULL;

  fprintf (out, "    ");
  for (accspec = op->acc_info; accspec; accspec = accspec->next)
    {
      /* SER 041013: Need to remove duplicate obj ids. */
      if (Set_in (visited, accspec->id))
	continue;
      else
	visited = Set_add (visited, accspec->id);

      if (line_iter == OBJS_PER_LINE)
	{
          fprintf (out, "\n    ");
	  line_iter = 1;
	}
      else
	line_iter++;

      fprintf (out, " %i", accspec->id);
    }
  fprintf (out, "\n");

  Set_dispose (visited);
}


/* SER */
void
L_print_custom_profile_encoding (FILE * out, L_Func * fn)
{
  L_Cb *cb = NULL;
  L_Oper *op;
  L_Loop *loop;
  L_Inner_Loop *lp;
  L_Attr *attr = NULL;
  L_Flow *flow_indx;
  int attr_num;
  int size;
  int num_jsrs;
  int asm_index;
  int bb_size;			/* Basic block size, includes branch */
  int is_branch;
  int i;
  int num_opers;
  int mark_inner;
  /* 
   * If want to encode loop info for iteration profiling, call 
   * the routines to annotate lcode with the loop info.
   */
  if (encode_loop_info)
    {
      /* 
       * Using code segment written by Dave August. -JCG 5/6/95 
       */
      /* Dominator analysis is required for loop detection. */
      L_do_flow_analysis (fn, DOMINATOR);

      /* Detect loops without inserting empty preheaders */
      L_loop_detection (fn, 0);

      for (loop = fn->first_loop; loop != NULL; loop = loop->next_loop)
	{
	  attr = L_new_attr ("Lencode_LoopHeader", 0);

	  attr_num = 0;
	  for (flow_indx = loop->header->src_flow; flow_indx;
	       flow_indx = flow_indx->next_flow)
	    {
	      if (!L_in_cb_DOM_set (flow_indx->src_cb, loop->header->id))
		{
		  L_set_int_attr_field (attr, attr_num,
					flow_indx->src_cb->id);
		  attr_num++;
		}
	    }

	  loop->header->attr = L_concat_attr (loop->header->attr, attr);
	}
    }

  /* Count the number of lines for this function that will be printed */
  size = 0;
  num_jsrs = 0;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      num_opers = 0;
      size++;
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  if(L_do_buf_info)
	    num_opers++;

	  if (L_cond_branch_opcode (op))
	    size++;

	  /* Count if predicated or not */
	  else if (L_uncond_branch_opcode (op))
	    size++;

	  else if (L_register_branch_opcode (op))
	    size++;

	  else if (L_subroutine_return_opcode (op))
	    size++;

	  else if (L_subroutine_call_opcode (op))
	    {

	      /* 
	       * Lhppa replaces __builtin_va_start with an add and 4 stores
	       * during phase 1.  
	       * So don't treat this function as a jsr or Lprofile will
	       * get confused during lcode profiling as to why there is 
	       * not a jsr there.
	       *
	       * Lsparc replaces __builtin_va_arg_incr with and add
	       * and a load.  So again don't treat as a jsr.
	       *
	       * For Lemulate, must treat as jsrs or trace will be
	       * misaligned -ITI/JCG 3/99.
	       */
	      if (!L_is_label (op->src[0]) ||
		  !filter_out_vararg_jsrs ||
		  !(L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_start") ||
		    L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_arg_incr")))
		{
		  num_jsrs++;
		  size++;
		  if (L_is_label (op->src[0]) &&
		      (L_matches_fn_name (op->src[0]->value.l, "malloc") ||
		       L_matches_fn_name (op->src[0]->value.l, "calloc") ||
		       L_matches_fn_name (op->src[0]->value.l, "free")))
		    size++;
		}
	    }
	  /* Added info for value profiling */
	  /* 20030414 SZU
	   * Condition on parms
	   */
	  else if (do_str_red)
	    {
	      if (L_mul_opcode (op))
		size++;
	      else if (L_div_opcode (op))
		size++;
              else if (L_int_rem_opcode (op))
		size++;
	    }

	  /* 20040507 SER: Mem addr profiling for HCH MICRO04 */
	  else if (do_mem_addrs)
	    {
	      int bad = 0;
	      L_AccSpec * accspec;

	      if (L_load_opcode (op))
		{ /* Don't profile loads which def objs */
		  for (accspec = op->acc_info; accspec;
		       accspec = accspec->next)
		    {
		      if (accspec->is_def)
			{
			  bad = 1;
			  break;
			}
		    }
		  if (bad)
		    continue;
		  size++;
		}
	      else if (L_store_opcode (op))
		{ /* Don't profile stores which use objs */
		  for (accspec = op->acc_info; accspec;
		       accspec = accspec->next)
		    {
		      if (!(accspec->is_def))
			{
			  bad = 1;
			  break;
			}
		    }
		  if (bad)
		    continue;
		  size++;
		}
	    }
	  
	}
      if(L_do_buf_info)
	fprintf (buf_info_file, "%s %d %d\n", 
		 fn->name, cb->id, num_opers);
      
    }

  /* 
   * Determine how much to strip off fn->name to get asm_name using
   * M_arch.
   */
  if ((M_arch == M_SPARC) || ((M_arch == M_X86) && encode_for_aout_linux))
    asm_index = 0;
  else
    asm_index = 1;

  /* Set basic block size to 0 */
  bb_size = 0;

  fprintf (out, "\n");
  fprintf (out,
	   "begin %s %s  Addr: 0x%08x  Size: %-8d  Max_Cb: %-8d  JSRs: %-8d\n",
	   fn->name, &fn->name[asm_index], 0, size, fn->max_cb_id, num_jsrs);

  if(L_do_buf_info)
    {
      PG_setup_pred_graph(fn);
      L_inner_loop_detection(fn, 0);
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* 
       * If encoding loop info, check to see if this cb is a loop header.
       * If it is, print out loop header cb, and all the entry cbs
       */
      if ((encode_loop_info) &&
	  ((attr = L_find_attr (cb->attr, "Lencode_LoopHeader")) != NULL))
	{
	  if(L_do_buf_info)
	    {
	      mark_inner = 0;
	      for (lp = fn->first_inner_loop; lp!=NULL; lp = lp->next_inner_loop)
		{
		  if (lp->cb->id == cb->id)
		    {
		      mark_inner = 1;
		      break;
		    }
		}
	      fprintf (out, "cb_loop_header  %i %i %i\n", cb->id, 
		       attr->max_field, mark_inner);
	    }
	  else
	    fprintf (out, "cb_loop_header  %i %i\n", cb->id, attr->max_field);

	  /* Print out all the cb's the loop can be entered from.
	   * Entries from all other cb's are considered backedges.
	   */
	  for (i = 0; i < attr->max_field; i++)
	    {
#if LP64_ARCHITECTURE
	      fprintf (out, "  _preheader_cb %li\n",
		       (long)attr->field[i]->value.i);
#else
	      fprintf (out, "  _preheader_cb %i\n", attr->field[i]->value.i);
#endif
	    }
	}
      /* Otherwise, print normal cb marker */
      else
	{
	  fprintf (out, "cb %i\n", cb->id);
	}

      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* If it not an ignored opcode, increase basic block size */
	  if (!L_ignore_oper (op))
	    bb_size++;

	  /* Assume it is a branch */
	  is_branch = 1;

	  if (L_cond_branch_opcode (op))
	    fprintf (out, "  br %i\n", bb_size);

	  else if (L_uncond_branch_opcode (op))
	    {
	      if (!L_is_predicated (op))
		fprintf (out, "  jmp %i\n", bb_size);
	      else
		fprintf (out, "  pjmp %i\n", bb_size);
	    }

	  else if (L_register_branch_opcode (op))
	    fprintf (out, "  hash %i\n", bb_size);

	  else if (L_subroutine_return_opcode (op))
	    fprintf (out, "  ret %i\n", bb_size);

	  else if (L_subroutine_call_opcode (op))
	    {
	      /* 
	       * Lhppa replaces __builtin_va_start with an add and 4 stores
	       * during phase 1.  
	       * So don't treat this function as a jsr or Lprofile will
	       * get confused during lcode profiling as to why there is 
	       * not a jsr there.
	       *
	       * Lsparc replaces __builtin_va_arg_incr with and add
	       * and a load.  So again don't treat as a jsr.
	       *
	       * For Lemulate, must treat as jsrs or trace will be
	       * misaligned -ITI/JCG 3/99.
	       */
	      if (!L_is_label (op->src[0]) ||
		  !filter_out_vararg_jsrs ||
		  !(L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_start") ||
		    L_matches_fn_name (op->src[0]->value.l,
				       "__builtin_va_arg_incr")))
		{
		  fprintf (out, "  jsr %i\n", bb_size);
		  if (L_is_label (op->src[0]))
		    {
		      if (L_matches_fn_name (op->src[0]->value.l, "malloc") ||
			  L_matches_fn_name (op->src[0]->value.l, "calloc"))
			fprintf (out, "  malloc %i\n", ++bb_size);
		      else if (L_matches_fn_name (op->src[0]->value.l, "free"))
			fprintf (out, "  free %i\n", ++bb_size);
		    }
		}
	    }

	  /* Mark that it is not a branch */
	  else
	    {
	      /* New value profiling info here. */
	      /* 20030414 SZU
	       * Condition on parms
	       */
	      if (do_str_red)
		{
		  if (L_int_mul_opcode (op))
		    fprintf (out, "  muli %i\n", bb_size);
		  else if (op->opc == Lop_MUL_F)
		    fprintf (out, "  mulf %i\n", bb_size);
		  else if (op->opc == Lop_MUL_F2)
		    fprintf (out, "  muld %i\n", bb_size);
		  else if (L_int_div_opcode (op))
		    fprintf (out, "  divi %i\n", bb_size);
		  else if (op->opc == Lop_DIV_F)
		    fprintf (out, "  divf %i\n", bb_size);
		  else if (op->opc == Lop_DIV_F2)
		    fprintf (out, "  divd %i\n", bb_size);
                  else if (L_int_rem_opcode (op))
		    fprintf (out, "  rem %i\n", bb_size);
		}

	      /* 20040507 SER: Mem addr profiling for HCH MICRO04 */
	      if (do_mem_addrs)
		{
		  int bad = 0;
		  L_AccSpec * accspec;
		  Set visited = NULL;

		  if (L_load_opcode (op))
		    {
		      i = 0;
		      for (accspec = op->acc_info; accspec;
			   accspec = accspec->next)
			{
			  if (accspec->is_def)
			    {
			      bad = 1;
			      break;
			    }
			  /* Skip duplicate objects. */
			  if (Set_in (visited, accspec->id))
			    continue;
			  else
			    visited = Set_add (visited, accspec->id);
			  i++;
			}
		      visited = Set_dispose (visited);
		      if (bad)
			continue;

		      if ((attr = L_find_attr (op->attr, "regalloc1")))
			{
			  fprintf (out, "  ld_alloc %d\n", bb_size);
			}
		      else
			{
			  fprintf (out, "  ld %d %d\n", bb_size, i);
			  if (i > 0)
			    L_print_mem_objs (out, op);
			}
		    }
		  else if (L_store_opcode (op))
		    {
		      i = 0;
		      for (accspec = op->acc_info; accspec;
			   accspec = accspec->next)
			{
			  if (!(accspec->is_def))
			    {
			      bad = 1;
			      break;
			    }
			  /* Skip duplicate objects. */
			  if (Set_in (visited, accspec->id))
			    continue;
			  else
			    visited = Set_add (visited, accspec->id);
			  i++;
			}
		      visited = Set_dispose (visited);
		      if (bad)
			continue;

		      if ((attr = L_find_attr (op->attr, "regalloc1")))
			{
			  fprintf (out, "  st_alloc %d\n", bb_size);
			}
		      else
			{
			  fprintf (out, "  st %d %d\n", bb_size, i);
			  if (i > 0)
			    L_print_mem_objs (out, op);
			}
		    }
		}

	      is_branch = 0;
	    }

	  /* If it is a branch, reset bb_size */
	  if (is_branch)
	    bb_size = 0;
	}
    }
  fprintf (out, "end %s\n", fn->name);

  return;
}


/* Initializes the Sen_Fn structure */
static void
L_init_Efn (Efn * efn, L_Func * fn)
{
  int i;
  int array_size;

  /* Create alloc pools if not already created */
  if (eoperand_pool == NULL)
    {
      /* Create pools of eoperands, eops and ecbs */
      eoperand_pool = L_create_alloc_pool ("Eoperand", sizeof (Eoperand),
					   128);
      eop_pool = L_create_alloc_pool ("Eop", sizeof (Eop), 128);
      ecb_pool = L_create_alloc_pool ("Ecb", sizeof (Ecb), 128);

      /* Create pool of operand id arrays(array of shorts) */
      array_size = sizeof (short) *
	(L_max_dest_operand + L_max_src_operand + L_max_pred_operand);
      eoperand_array_pool = L_create_alloc_pool ("Eoperand array",
						 array_size, 128);

    }

  /* Initialize efn fields */
  efn->name = strdup (fn->name);

  /* 
   * Assembly name depends on architecture.  All but sparc strip
   * off leading underscore.
   */
  if ((M_arch == M_SPARC) || ((M_arch == M_X86) && encode_for_aout_linux))
    efn->asm_name = strdup (fn->name);
  else
    efn->asm_name = strdup (&fn->name[1]);

  efn->head_eop = NULL;
  efn->tail_eop = NULL;
  efn->eop_count = 0;
  for (i = 0; i < OPERAND_HASH_SIZE; i++)
    efn->eoperand_hash[i] = NULL;
  efn->head_eoperand = NULL;
  efn->tail_eoperand = NULL;
  efn->eoperand_count = 0;
  efn->head_ecb = NULL;
  efn->tail_ecb = NULL;
  efn->ecb_count = 0;

  return;
}

static unsigned int
hash_string (char * string)
{
  unsigned int hash_val;

  hash_val = 0;
  for (; *string != 0; string++)
    hash_val = (hash_val << 1) + *string;

  return (hash_val);
}

/*
 * Hash the operand using knowledge of lcode to try to get an even
 * distribution
 */
static int
hash_operand (L_Operand * operand)
{
  int hash_val = 0;
  char buf[100];

  /*
   * Use int value (plus offset) if available, otherwise, make
   * into a string (if not already) and hash that.
   */
  switch (L_operand_case_type (operand))
    {
    case L_OPERAND_REGISTER:
      hash_val = operand->value.r;
      break;
    case L_OPERAND_MACRO:
      hash_val = 64 + operand->value.mac;
      break;
    case L_OPERAND_CB:
      hash_val = 128 + operand->value.cb->id;
      break;
    case L_OPERAND_INT:
      hash_val = operand->value.i;
      break;
    case L_OPERAND_FLOAT:
      sprintf (buf, "%1.6e", operand->value.f);
      hash_val = hash_string (buf);
      break;
    case L_OPERAND_DOUBLE:
      sprintf (buf, "%1.6e", operand->value.f2);
      /* Debug */
      hash_val = hash_string (buf);
      break;
    case L_OPERAND_LABEL:
      hash_val = hash_string (operand->value.s);
      break;
    case L_OPERAND_STRING:
      hash_val = hash_string (operand->value.l);
      break;
    case L_OPERAND_RREGISTER:
      hash_val = operand->value.rr;
      break;
    case L_OPERAND_EVR:
      hash_val = operand->value.evr.num;
      break;

    default:
      L_punt ("hash_operand: Unknown operand type %i",
	      L_return_old_type (operand));
    }
  return (hash_val);
}

static short
get_eoperand_index (Efn * efn, L_Operand * operand)
{
  unsigned int hash_val;
  unsigned int hash_index;
  Eoperand *eoperand;

  /* Null operands get index 0 */
  if (operand == NULL)
    return (0);

  /* Hash operand and look for match in hash table */
  hash_val = hash_operand (operand);
  hash_index = hash_val & (OPERAND_HASH_SIZE - 1);
  for (eoperand = efn->eoperand_hash[hash_index]; eoperand != NULL;
       eoperand = eoperand->next_hash)
    {
      /* Compare hash values first to speed search */
      if ((hash_val == eoperand->hash_val) &&
	  L_same_operand (operand, eoperand->operand))
	break;
    }

  /* If not found, add to hash and eoperand list */
  if (eoperand == NULL)
    {
      eoperand = (Eoperand *) L_alloc (eoperand_pool);
      efn->eoperand_count++;
      /* NULL operand is 0, so start numbering eoperands from 1 */
      eoperand->id = efn->eoperand_count;
      eoperand->flags = 0;

      /* Flag if operand is a  register */
      if (L_is_register (operand) || L_is_macro (operand))
	eoperand->flags |= REGISTER_OPERAND;

      if (L_is_pred_register (operand))
	eoperand->flags |= PREDICATE_OPERAND;

      eoperand->hash_val = hash_val;
      eoperand->operand = operand;

      /* Add to linear linked list */
      if (efn->tail_eoperand == NULL)
	efn->head_eoperand = eoperand;
      else
	efn->tail_eoperand->next_linear = eoperand;
      efn->tail_eoperand = eoperand;
      eoperand->next_linear = NULL;

      /* Add to front of hash table */
      eoperand->next_hash = efn->eoperand_hash[hash_index];
      efn->eoperand_hash[hash_index] = eoperand;
    }
  return (eoperand->id);
}

/* Detect cache directive opcodes (MEM_COPIES) */
int
L_cache_directive_opcode (L_Oper * op)
{
  int opc;

  if (op == NULL)
    return (0);

  opc = op->opc;
  switch (opc)
    {
    case Lop_MEM_COPY:
    case Lop_MEM_COPY_TAG:
    case Lop_MEM_COPY_BACK:
    case Lop_MEM_COPY_CHECK:
    case Lop_MEM_COPY_RESET:
    case Lop_MEM_COPY_SETUP:
      return (1);
    }
  return (0);
}


/* Encode fn and put everything into efn */
static void
L_encode_fn (Efn * efn, L_Func * fn)
{
  L_Cb   *cb;
  L_Loop *loop;  
  /* 10/25/04 REK Commenting out unused variables to quiet compiler warnings.
   */
#if 0
  int    current_loop, id;
  L_Attr *new_attr;
#endif
  int current_cb_loop; 
  L_Oper *op;
  Ecb    *ecb = NULL;
  Eop    *eop, *prev_eop;
  int    i;
  short  *edest, *esrc, *epred;
  L_Attr *attr, *cycle_attr, *dep_attr, *chs_attr;
  int    scheduled_operations, unscheduled_operations;
  double weight, taken_weight;
  L_Flow *flow;
  int    num_blocks = 0;
  int    *buf = NULL; 
  int    attr_num;
  L_Flow *flow_indx;

  /* Initialize linked lists and hash tables */
  L_init_Efn (efn, fn);

  /* Intialize counters */
  scheduled_operations = 0;
  unscheduled_operations = 0;

  if (encode_sim_loop_info)
    {
      /* 
       * Using code segment written by Dave August. -JCG 5/6/95 
       */
      /* Dominator analysis is required for loop detection. */
      L_do_flow_analysis(fn, DOMINATOR);
      
      /* Detect loops without inserting empty preheaders */
      L_loop_detection(fn, 0);

      /* For process of adding loop IDs to cbs */
      for (cb = fn->first_cb ; cb; cb = cb->next_cb)
	cb->flags = L_CLR_BIT_FLAG(cb->flags, L_CB_RESERVED_TEMP8);

      
      for (loop=fn->first_loop; loop!=NULL; loop=loop->next_loop)
	{

	  /* Loop Header info */
	  attr = L_new_attr("Lencode_LoopHeader", 0);
	  
	  attr_num = 0;
	  for(flow_indx = loop->header->src_flow; flow_indx; 
	      flow_indx = flow_indx->next_flow)
	     {
	      if(!L_in_cb_DOM_set(flow_indx->src_cb, loop->header->id))
		{
		  L_set_int_attr_field(attr, attr_num, 
				       flow_indx->src_cb->id);
		  attr_num++;
		}
	    }
	  
	  loop->header->attr = L_concat_attr(loop->header->attr, attr);
	  /* End of Loop Header processing */


	  /* Add loop id's to cb's */

	  num_blocks = Set_size(loop->loop_cb);
	  buf = (int *) Lcode_malloc(sizeof(int) * num_blocks);
	  if (buf == NULL)
	    L_punt("Loop header processing: malloc out of space");
	  
	  Set_2array(loop->loop_cb, buf);
	  
	  /* Step through cb's in the loop */
	  for (i = 0; i < num_blocks; i++)
	    {
	      cb = L_cb_hash_tbl_find(L_fn->cb_hash_tbl, buf[i]);
	      if(!(L_EXTRACT_BIT_VAL(cb->flags, L_CB_RESERVED_TEMP8)))
		{

		  cb->flags = L_SET_BIT_FLAG(cb->flags, L_CB_RESERVED_TEMP8);
		}
	    }
	  
	  Lcode_free(buf);

	}  /* End for over loops in fn */

    }  /* end if encode_sim_loop_info */


  /* Encode every operation in the function */
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      /* Add this cb (and what it's first op is) to the efn's cb list */
      ecb = (Ecb *) L_alloc (ecb_pool);
      ecb->id = cb->id;
      ecb->first_op = efn->eop_count;
      ecb->next = NULL;
      ecb->prehead_cbs = NULL;
      if (efn->tail_ecb == NULL)
	efn->head_ecb = ecb;
      else
	efn->tail_ecb->next = ecb;
      efn->tail_ecb = ecb;
      efn->ecb_count++;
      weight = cb->weight;

      if ((encode_sim_loop_info) && 
	  ((attr = L_find_attr (cb->attr, "Lencode_LoopHeader")) != NULL))
	{
	  ecb->prehead_cbs = L_copy_attr(attr);
	}

      /* No prev_eop at top of cb */
      prev_eop = NULL;
      
      if (encode_sim_loop_info && cb->deepest_loop)
 	current_cb_loop = cb->deepest_loop->id;
      else
 	current_cb_loop = -1;
      
      /* Encode every operation in cb */
      for (op = cb->first_op; op != NULL; op = op->next_op)
	{
	  /* Filter out the following opcodes */
	  if (L_ignore_oper (op))
	    continue;

	  /* Lhppa replaces __builtin_va_start with an add and 4 stores
	   * during phase 1.  
	   * Should only see when encoding lcode and just filtering it
	   * out seems like the best solution.
	   *
	   * For Lemulate, must treat as jsrs or trace will be
	   * misaligned -ITI/JCG 3/99.
	   */
	  if (L_subroutine_call_opcode (op) &&
	      (L_is_label (op->src[0])) &&
	      filter_out_vararg_jsrs &&
	      (L_matches_fn_name (op->src[0]->value.l,
				  "__builtin_va_start") ||
	       L_matches_fn_name (op->src[0]->value.l,
				  "__builtin_va_arg_incr")))
	    {
	      /* Print warning */
	      fprintf (stderr,
		       "Warning %s op %i: jsr to __builtin_va_start in Lcode.\n",
		       fn->name, op->id);
	      fprintf (stderr,
		       "-> Removing it to make Lsim work, but this reduces the simulation's accuracy.\n");
	      fprintf (stderr,
		       "-> Solution (for hppa): replace, by hand, with a lcode sequence \n");
	      fprintf (stderr,
		       "-> or simulate Mcode instead of Lcode (Lhppa/Lsparc phase1 does substitution).\n");
	      continue;
	    }

	  /* Create eop and initialize fields */
	  eop = (Eop *) L_alloc (eop_pool);
	  eop->id = op->id;
	  eop->index = efn->eop_count;
	  eop->opc = op->opc;
	  eop->proc_opc = op->proc_opc;

	  if(encode_sim_loop_info) {
	    eop->loop_id = current_cb_loop;  /* HCH */
	  }

	  /* Determine branch target cb id (when we can determine it) */
	  if (L_cond_branch_opcode (op))
	    {
	      /* Expect target to be src[2] */
	      if (L_is_cb (op->src[2]))
		eop->br_target = op->src[2]->value.cb->id;
	      else
		L_punt ("%s op %i: invalid cond branch dest",
			fn->name, op->id);
	    }

	  else if (L_uncond_branch_opcode (op))
	    {
	      /* Expect target to be src[0] */
	      if (L_is_cb (op->src[0]))
		eop->br_target = op->src[0]->value.cb->id;
	      else
		L_punt ("%s op %i: invalid uncond branch dest",
			fn->name, op->id);
	    }
	  /* Put cb 0 for no branch target (or hashing branch) */
	  else
	    eop->br_target = 0;


	  /* Set operation flags and playdoh flags, default unset */
	  eop->flags = 0;
	  eop->playdoh_flags = 0;

	  /* Flag exit points for simulation */
	  if (L_subroutine_return_opcode (op) &&
	      (L_matches_fn_name (fn->name, "main") ||
	       (linking_to_f2c_library &&
		L_matches_fn_name (fn->name, "MAIN__"))))
	    eop->flags |= SIM_EXIT_POINT;
	  else if (L_subroutine_call_opcode (op) &&
		   (L_is_label (op->src[0])) &&
		   (L_matches_fn_name (op->src[0]->value.l, "exit") ||
		    (linking_to_f2c_library &&
		     L_matches_fn_name (op->src[0]->value.l, "s_stop"))))
	    eop->flags |= SIM_EXIT_POINT;
	  else if (L_subroutine_call_opcode (op) &&
		   L_is_label (op->src[0]) &&
		   L_matches_fn_name (op->src[0]->value.l, "atexit"))
	    L_punt("Lsim does not handle calls to atexit");
		   


	  /* Flag instructions with MCB attribute */
	  if (L_find_attr (op->attr, "MCB") != NULL)
	    eop->flags |= MCB_ATTR;

	  /* Detect non-trapping instructions */
	  if (op->flags & L_OPER_MASK_PE)
	    eop->flags |= NON_TRAPPING;

	  /* Detect predicate promoted instructions. */
	  if (op->flags & L_OPER_PROMOTED)
	    eop->flags |= PROMOTED;

	  /* Detect compile-time speculative instructions */
	  if (op->flags & L_OPER_SPECULATIVE)
	    eop->flags |= SPECULATIVE;

	  /* Flag instructions with stats_on attribute */
	  if (L_find_attr (op->attr, "stats_on") != NULL)
	    eop->flags |= STATS_ON_ATTR;

	  /* Flag instructions with stats_off attribute */
	  if (L_find_attr (op->attr, "stats_off") != NULL)
	    {
	      eop->flags |= STATS_OFF_ATTR;

	      /* May not have both stats_on and stats_off in 
	       * same instruction.
	       */
	      if (eop->flags & STATS_ON_ATTR)
		{
		  L_punt
		    ("%s op %i: Cannot have both stats_on and stats_off in same op.",
		     fn->name, op->id);
		}
	    }

	  /* 
	   * Flag instructions with force_sim attributes with
	   * whether turning on or off force sim (inhibits
	   * sampling in that region of code to cause all of
	   * that code to be simulated).
	   */
	  if ((attr = L_find_attr (op->attr, "force_sim")) != NULL)
	    {
	      switch ((int) attr->field[0]->value.i)
		{
		case 0:
		  eop->flags |= FORCE_SIM_OFF;
		  break;

		case 1:
		  eop->flags |= FORCE_SIM_ON;
		  break;

		default:
		  L_punt ("%s op %i: Unknown force_sim value %i.\n",
			  fn->name, op->id, attr->field[0]->value.i);
		}
	    }

#if 0
	  /* JWS/HCH 19991026 support for speculative load assessment */
	  if ((attr = L_find_attr (op->attr, "SPECID")) != NULL)
	    {
	      id = attr->field[0]->value.i;
	      eop->br_target = id;
	    }
#endif

	  /* Flag a stoping point for a simulation */
	  if (L_find_attr (op->attr, "stop_sim") != NULL)
	    {
	      eop->flags |= STOP_SIM;
	    }

	  /* Flag instructions with static branch prediction */
	  if ((attr = L_find_attr (op->attr, "bpred")) != NULL)
	    {
	      switch ((int) attr->field[0]->value.i)
		{
		case 0:
		  eop->flags |= BRANCH_PREDICTED_FALLTHRU;
		  break;

		case 1:
		  eop->flags |= BRANCH_PREDICTED_TAKEN;
		  break;

		default:
		  L_punt ("%s op %i: Unknown bpred value %i.\n",
			  fn->name, op->id, attr->field[0]->value.i);
		}
	    }
	  else
	    {

	      if ((L_general_branch_opcode (op) ||
		   L_subroutine_call_opcode (op) ||
		   L_subroutine_return_opcode (op)))
		{

		  /* jsr, rts always taken */
		  if (L_subroutine_call_opcode (op) ||
		      L_subroutine_return_opcode (op))
		    {
		      eop->flags |= BRANCH_PREDICTED_TAKEN;
		    }
		  /* cond br or predicated jump */
		  else if (L_cond_branch_opcode (op) ||
			   (L_uncond_branch_opcode (op) &&
			    L_is_predicated (op)))
		    {

		      flow = L_find_flow_for_branch (cb, op);

		      taken_weight = flow->weight;
		      weight -= taken_weight;

		      if (taken_weight > weight)
			eop->flags |= BRANCH_PREDICTED_TAKEN;
		      else
			eop->flags |= BRANCH_PREDICTED_FALLTHRU;
		    }
		  /* unpredicated jump or jump_rg always taken */
		  else
		    {
		      eop->flags |= BRANCH_PREDICTED_TAKEN;
		    }
		}
	    }

	  /* Use new isl attribute if available otherwise use
	   * isc attribute.  (Both have identical field[0] and field[1]).
	   */
	  cycle_attr = L_find_attr (op->attr, "isl");

	  if (cycle_attr == NULL)
	    cycle_attr = L_find_attr (op->attr, "isc");

	  /* Get the cycle and slot each instruction was scheduled for */
	  if ((cycle_attr != NULL) &&
	      (cycle_attr->field[0] != NULL) &&
	      (cycle_attr->field[1] != NULL))
	    {
	      /* Count those instructions with scheduling info */
	      scheduled_operations++;

	      /* Get the cycle and slot that the current instruction is 
	       * scheduled for.
	       */
	      eop->cycle = cycle_attr->field[0]->value.i;
	      eop->slot = cycle_attr->field[1]->value.i;

	      /* Set the latency fields. If there is no 3rd field in
	       * the isl attribute, assume first latency is 1 (i.e. branches).
	       * Up to 3 dests (3rd to 5th fields).
	       */
	      for (i = 2; i < 5; i++)
		{
		  if (cycle_attr->max_field > i)
		    eop->latency[i - 2] = cycle_attr->field[i]->value.i;
		  else if (i == 2)
		    eop->latency[i - 2] = 1;
		  else
		    eop->latency[i - 2] = 0;
		}
	      if (cycle_attr->max_field > 5)
		L_punt ("isl attribute has more than 5 fields!");

	      /* If dealing only with scheduled instructions,
	       * mark the start and stop of each packet.
	       */
	      if (unscheduled_operations == 0)
		{
		  /*
		   * Mark this eop with START_PACKET if:
		   * 1) At beginning of cb or
		   * 2) Cycle of last eop different.
		   */
		  if ((prev_eop == NULL) || (prev_eop->cycle != eop->cycle))
		    eop->flags |= START_PACKET;


		  /*
		   * Mark this eop with END_PACKET if:
		   * 1) At end of cb.
		   */
		  if (op->next_op == NULL)
		    eop->flags |= END_PACKET;

		  /*
		   * Mark PREV eop with END_PACKET if:
		   * 1) Previous eop exists and
		   * 2) Cycle of last eop different.
		   */
		  if ((prev_eop != NULL) && (prev_eop->cycle != eop->cycle))
		    prev_eop->flags |= END_PACKET;

		}

	    }
	  else
	    {
	      /* Count those instructions without scheduling info */
	      unscheduled_operations++;

	      eop->cycle = 0;
	      eop->slot = -1;
	      eop->latency[0] = -1;
	      eop->latency[1] = -1;
	      eop->latency[2] = -1;
	    }

	  /* Get the DEP id for the instruction */
	  if (((dep_attr = L_find_attr (op->attr, "DEP")) != NULL) &&
	      (dep_attr->field[0] != NULL))
	    {
	      eop->dep_id = dep_attr->field[0]->value.i;
	    }
	  else
	    {
	      /* Set to "null" dep id */
	      eop->dep_id = 0;
	    }


	  /*
	   * Detect if this instruction is an implicit memory operation.
	   * Currently only for x86 machines.  Call Mspec call to
	   * determine if it is and flag.
	   */
	  if (M_is_implicit_memory_op (op))
	    {
	      /* Don't flag exit points
	       * (x86 probing cannot handle it)
	       */
	      if (!(eop->flags & SIM_EXIT_POINT))
		eop->flags |= IMPLICIT_MEMORY_OP;
	    }

	  /* 
	   * Detect if this instruction is a predicate definition.
	   * Will read trace info differently if tracing predicate defs.
	   */
	  if (L_general_pred_comparison_opcode (op))
	    {
	      eop->flags |= PRED_DEF;
	    }

	  /*
	   * Get PLAYDOH flags
	   */
	  if ((chs_attr = L_find_attr (op->attr, "chs")) != NULL)
	    {
	      /* Get target specifier if not NULL */
	      if (chs_attr->field[0] != NULL)
		{
		  /* Set TCHS flag based on value */
		  switch ((int) chs_attr->field[0]->value.i)
		    {
		    case 0:
		      eop->playdoh_flags |= PLAYDOH_TCHS_V1;
		      break;

		    case 1:
		      eop->playdoh_flags |= PLAYDOH_TCHS_C1;
		      break;

		    case 2:
		      eop->playdoh_flags |= PLAYDOH_TCHS_C2;
		      break;

		    case 3:
		      eop->playdoh_flags |= PLAYDOH_TCHS_C3;
		      break;

		    default:
		      L_punt
			("L_encode_fn: Function %s op %i: unknown TCHS value %i",
			 efn->name, op->id, chs_attr->field[0]->value.i);
		    }
		}

	      /* Get source specifier if not NULL */
	      if (chs_attr->field[1] != NULL)
		{
		  /* Set TCHS flag based on value */
		  switch ((int) chs_attr->field[1]->value.i)
		    {
		    case 0:
		      eop->playdoh_flags |= PLAYDOH_SCHS_V1;
		      break;

		    case 1:
		      eop->playdoh_flags |= PLAYDOH_SCHS_C1;
		      break;

		    case 2:
		      eop->playdoh_flags |= PLAYDOH_SCHS_C2;
		      break;

		    case 3:
		      eop->playdoh_flags |= PLAYDOH_SCHS_C3;
		      break;

		    default:
		      L_punt
			("L_encode_fn: Function %s op %i: unknown SCHS value %i",
			 efn->name, op->id, chs_attr->field[1]->value.i);
		    }
		}
	    }


	  /* Create array of operand indexes */
	  eop->eoperand = (short *) L_alloc (eoperand_array_pool);

	  /* Get pointers to start of each section for easy access */
	  edest = eop->eoperand;
	  esrc = edest + L_max_dest_operand;
	  epred = esrc + L_max_src_operand;

	  /* 
	   * Get indexes for all operands (NULL operands get 0) 
	   * Filter out destinations for cache directives (MEM_COPYS) 
	   */
	  if (L_cache_directive_opcode (op))
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		edest[i] = get_eoperand_index (efn, NULL);
	    }
	  else
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		edest[i] = get_eoperand_index (efn, op->dest[i]);
	    }
	  for (i = 0; i < L_max_src_operand; i++)
	    esrc[i] = get_eoperand_index (efn, op->src[i]);
	  for (i = 0; i < L_max_pred_operand; i++)
	    epred[i] = get_eoperand_index (efn, op->pred[i]);

	  /* 
	   * TLJ 7/17/96 - for playdoh code a prefetch is
	   * a load to r0. Detect this case and make dest0 NULL
	   * and set the PLAYDOH_PREFETCH flag.
	   */
	  if (M_arch == M_PLAYDOH)
	    {
	      if (L_is_reg (op->dest[0]) &&
		  op->dest[0]->value.mac == 0 && L_load_opcode (op))
		{
		  edest[0] = get_eoperand_index (efn, NULL);
		  eop->playdoh_flags |= PLAYDOH_PREFETCH;
		  fprintf (stderr, "PREFETCH %d\n", op->id);
		}
	    }

	  /* Add to functions list */
	  if (efn->tail_eop == NULL)
	    efn->head_eop = eop;
	  else
	    efn->tail_eop->next = eop;
	  efn->tail_eop = eop;
	  eop->next = NULL;
	  efn->eop_count++;

	  /* Save the eop for the packet detection */
	  prev_eop = eop;
	}
    }

  /* Punt if the last cb in the function is empty */
  if (efn->tail_ecb->first_op == efn->eop_count)
    L_punt ("L_encode_fn: Function %s last cb %i empty", efn->name, ecb->id);

  /* 
   * Punt if the function has some scheduling info but not all.
   * Code must be either unscheduled or have complete scheduling info.
   *
   * For now, punting is more trouble than it is worth. -JCG 3/31/95
   */
#if 0
  if ((unscheduled_operations > 0) && (scheduled_operations > 0))
    L_punt ("%s: inconsistent scheduling. %i ops with attrs, %i without.\n",
	    efn->name, scheduled_operations, unscheduled_operations);
#endif

  /* Mark whether scheduling info is available */
  if (unscheduled_operations > 0)
    efn->sched_info_avail = 0;
  else
    efn->sched_info_avail = 1;

  return;
}

void
L_free_efn (Efn * efn)
{
  Eoperand *eoperand, *next_eoperand;
  Ecb *ecb, *next_ecb;
  Eop *eop, *next_eop;

  /* Free all eoperands */
  for (eoperand = efn->head_eoperand; eoperand != NULL;
       eoperand = next_eoperand)
    {
      next_eoperand = eoperand->next_linear;
      L_free (eoperand_pool, eoperand);
    }

  /* Free all ecbs */
  for (ecb = efn->head_ecb; ecb != NULL; ecb = next_ecb)
    {
      next_ecb = ecb->next;
      L_free_attr(ecb->prehead_cbs);
      L_free (ecb_pool, ecb);
    }

  /* Free all eops */
  for (eop = efn->head_eop; eop != NULL; eop = next_eop)
    {
      next_eop = eop->next;
      L_free (eoperand_array_pool, eop->eoperand);
      L_free (eop_pool, eop);
    }

  return;
}

void
L_print_efn (FILE * out, Efn * efn)
{
  Eoperand  *eoperand;
  Ecb       *ecb;
  Eop       *eop;
  L_Operand *operand;
  L_Attr    *attr = NULL;
  int       i, num_operands, max_cb_id, max_jsr_id;

  num_operands = L_max_dest_operand + L_max_src_operand + L_max_pred_operand;

  /* Find the max cb_id */
  max_cb_id = -1;
  for (ecb = efn->head_ecb; ecb != NULL; ecb = ecb->next)
    {
      if (max_cb_id < ecb->id)
	max_cb_id = ecb->id;
    }
  /* Find the max_jsr_id */
  max_jsr_id = 0;
  for (eop = efn->head_eop; eop != NULL; eop = eop->next)
    {
      if ((eop->opc == Lop_JSR) || (eop->opc == Lop_JSR_FS))
	max_jsr_id++;
    }

  /* Print out encoded function information */
  fprintf (out, "%X %X %X %X %X %X %X %X %X\n", L_max_dest_operand,
	   L_max_src_operand, L_max_pred_operand,
	   efn->eoperand_count, efn->eop_count, efn->ecb_count, max_cb_id,
	   max_jsr_id, efn->sched_info_avail);
  fprintf (out, "Function %s %s\n", efn->name, efn->asm_name);

  /* for readability */
  fprintf (out, "\n");

  /* print out the eoperands */
  for (eoperand = efn->head_eoperand; eoperand != NULL;
       eoperand = eoperand->next_linear)
    {
      /* Print hash_val and flags */
      fprintf (out, "%X %X\n", eoperand->hash_val, eoperand->flags);

      /* Print Lcode string for operand */
      operand = eoperand->operand;
      switch (L_operand_case_type (operand))
	{
	case L_OPERAND_REGISTER:
	  /* All variations of a predicate reg must be printed the same way */
	  fprintf (out, "r %d %s", operand->value.r,
		   L_ctype_name (L_return_old_ctype (operand)));
	  break;
	case L_OPERAND_MACRO:
	  fprintf (out, "mac %s %s", L_macro_name (operand->value.mac),
		   L_ctype_name (L_return_old_ctype (operand)));
	  break;
	case L_OPERAND_CB:
	  fprintf (out, "cb %d", operand->value.cb->id);
	  break;
	case L_OPERAND_INT:
#if LP64_ARCHITECTURE
	  fprintf (out, "i %ld", (long)operand->value.i);
#else
	  fprintf (out, "i %d", operand->value.i);
#endif
	  break;
	case L_OPERAND_FLOAT:
	  fprintf (out, "f %1.15e", operand->value.f);
	  break;
	case L_OPERAND_DOUBLE:
	  fprintf (out, "f2 %1.15e", operand->value.f2);
	  break;
	case L_OPERAND_STRING:
	  fprintf (out, "s %s", operand->value.s);
	  break;
	case L_OPERAND_LABEL:
	  fprintf (out, "l %s", operand->value.l);
	  break;
	case L_OPERAND_RREGISTER:
	  /* All variations of a predicate reg must be printed the same way */
	  fprintf (out, "rr %d %s", operand->value.rr,
		   L_ctype_name (L_return_old_ctype (operand)));
	  break;
	case L_OPERAND_EVR:
	  /* All variations of a predicate reg must be printed the same way */
	  fprintf (out, "evr %d:%d %s", operand->value.evr.num,
		   operand->value.evr.omega,
		   L_ctype_name (L_return_old_ctype (operand)));
	  break;
	default:
	  fprintf (out, "?");
	}

      fprintf (out, "\n");
    }

  /* for readability */
  fprintf (out, "\n");
  
  if(!encode_sim_loop_info)
    {
      for (eop = efn->head_eop; eop != NULL; eop = eop->next)
	{
	  fprintf (out, "%X %X %X %X %X %X %d %X %X %X %X %X", 
		   eop->id,
		   eop->opc, 
		   eop->proc_opc, 
		   eop->flags, 
		   eop->playdoh_flags,
		   eop->cycle, 
		   eop->slot, 
		   eop->latency[0], 
		   eop->latency[1],
		   eop->latency[2],
		   eop->dep_id, 
		   eop->br_target);
	  for (i=0; i < num_operands; i++)
	    fprintf (out, " %X", eop->eoperand[i]);
	  fprintf (out, "\n");
	}
    }
  else
    {
      for (eop = efn->head_eop; eop != NULL; eop = eop->next)
	{
	  fprintf (out, "%X %X %X %X %X %X %d %X %X %X %X %X %X", 
		   eop->id,
		   eop->opc, 
		   eop->proc_opc, 
		   eop->flags, 
		   eop->playdoh_flags,
		   eop->cycle, 
		   eop->slot, 
		   eop->latency[0], 
		   eop->latency[1],
		   eop->latency[2],
		   eop->dep_id, 
		   eop->br_target,
		   eop->loop_id);
	  for (i=0; i < num_operands; i++)
	    fprintf (out, " %X", eop->eoperand[i]);
	  fprintf (out, "\n");
	}
    }
  
  /* for readability */
  fprintf (out, "\n");

  /* Print out ecbs */
  for (ecb = efn->head_ecb; ecb != NULL; ecb = ecb->next)
    {
      if(!encode_sim_loop_info)
	{
	  fprintf (out, "%X %X\n", ecb->id, ecb->first_op);
	}
      else
	{
	  attr = ecb->prehead_cbs;
	  
	  fprintf (out, "%X %X %X\n", 
		   ecb->id, ecb->first_op, 
		   attr ? attr->max_field : 0);
	  if(attr != NULL)
	    {
	      /* 
	       * Print out all the cb's the loop can be entered from.
	       * Entries from all other cb's are considered backedges.
	       */
	      
	      for (i = 0; i < attr->max_field; i++)
		{
#if LP64_ARCHITECTURE
		  fprintf (out, "%lX ", (long)attr->field[i]->value.i);
#else
		  fprintf (out, "%X ", attr->field[i]->value.i);
#endif
		}
	      fprintf (out, "\n");
	    }
	}
    }

  /* for readability */
  fprintf (out, "\n");

  /* Print out purely for readablity */
  fprintf (out, "end %s\n", efn->name);
  fprintf (out, "\n\n");
  
  return;
}

void
L_encode_for_simulation ()
{
  Efn     efn;
  L_Loop  *lp;
  /* 10/25/04 REK Commenting out unused variable to quiet compiler warning. */
#if 0
  L_Loop  *loop;
#endif
  FILE    *loop_nest_file = NULL;

  L_open_input_file (L_input_file);

  if(L_do_loop_nest_info)
    {
      if((loop_nest_file = fopen (loop_nest_file_name, "a")) == NULL)
	{
	  L_punt("Unable to open loop nest file for printing: %s", 
		 loop_nest_file_name);
	}
    }

  /* Process all data and functions within a file */
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
	{
	  L_define_fn_name (L_fn->name);

	  /* Encode function */
	  L_encode_fn (&efn, L_fn);

	  if (L_do_loop_nest_info)
	    {
	      /* 
	       * HCH 3/10/01: Print loop information out to a file for use in
	       * memory object tracing (see s_object.[ch] in Lsim) 
	       */
	      fprintf (loop_nest_file, "%s\n", L_fn->name);
	      
	      for (lp = L_fn->first_loop; lp != NULL; lp = lp->next_loop)
		{
		  fprintf (loop_nest_file, "Loop %i Lvl %i\n", 
			   lp->id, lp->nesting_level);
		  if (lp->nested_loops)
		    {
		      Set_print(loop_nest_file, "nested_loops", lp->nested_loops);
		      fprintf(loop_nest_file, "\n");
		    }
		}
	    }
	  
	  /* Print out version number */
	  if ( !encode_sim_loop_info )
	    {
	      fprintf (L_OUT, "Lcode encoded for simulation.\n");
	      fprintf (L_OUT, "Version 7\n");
	    }
	  else
	    {
	      fprintf (L_OUT, "Lcode encoded for simulation.\n");
	      fprintf (L_OUT, "Version 8\n");
	    }
	  
	  /* Print encoded function */
	  L_print_efn (L_OUT, &efn);

	  /* Free encoded function data */
	  L_free_efn (&efn);

	  /* Make sure everything was freed */
	  L_print_alloc_info (stderr, eoperand_pool, 0);
	  L_print_alloc_info (stderr, ecb_pool, 0);
	  L_print_alloc_info (stderr, eop_pool, 0);
	  L_print_alloc_info (stderr, eoperand_array_pool, 0);

	  /* Do not output lcode */
	  L_delete_func (L_fn);
	}
      else
	{
	  /* Do not output lcode */
	  L_delete_data (L_data);
	}
    }
  L_close_input_file (L_input_file);

  if(L_do_loop_nest_info)
    fclose(loop_nest_file);

  return;
}

void
L_encode_for_value_profiling ()
{
  Efn efn;

  L_open_input_file (L_input_file);

  /* Process all data and functions within a file */
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
	{
	  L_define_fn_name (L_fn->name);

	  /* Encode function */
	  L_encode_fn (&efn, L_fn);

	  /* Print out version number */
	  fprintf (L_OUT, "Lcode encoded for value profiling.\n");
	  fprintf (L_OUT, "Version 1\n");

	  /* Print encoded function */
	  L_print_efn (L_OUT, &efn);

	  /* Free encoded function data */
	  L_free_efn (&efn);

	  /* Make sure everything was freed */
	  L_print_alloc_info (stderr, eoperand_pool, 0);
	  L_print_alloc_info (stderr, ecb_pool, 0);
	  L_print_alloc_info (stderr, eop_pool, 0);
	  L_print_alloc_info (stderr, eoperand_array_pool, 0);

	  /* Do not output lcode */
	  L_delete_func (L_fn);
	}
      else
	{
	  /* Do not output lcode */
	  L_delete_data (L_data);
	}
    }
  L_close_input_file (L_input_file);

  return;
}

void
L_encode_for_profiling ()
{
  L_open_input_file (L_input_file);

  if(L_do_buf_info)
    {
      if((buf_info_file = fopen (buf_info_file_name, "a")) == NULL)
	{
	  L_punt("Unable to open buf info file for printing: %s", 
		 buf_info_file_name);
	}
    }
  
  /* Process all data and functions within a file */
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
	{
	  L_define_fn_name (L_fn->name);

	  fprintf (L_OUT, "# Unprofiled encoded lcode\n");
	  L_print_profile_encoding (L_OUT, L_fn);

	  /* Do not output lcode */
	  L_delete_func (L_fn);
	}
      else
	{
	  /* Do not output lcode */
	  L_delete_data (L_data);
	}
    }

  L_close_input_file (L_input_file);

  if(L_do_buf_info)
    fclose(buf_info_file);

  return;
}

/* 20030410 SZU
 * Modification for CS497CZ
 * Current value_profiling seems to encode for Lvalue_profile,
 * which doesn't work.
 * Writing our own value_profiling in Lencode->Lemulate framework.
 * Currently exact copy of 'profiling'
 * Change to L_encode_for_custom_profiling
 */
void
L_encode_for_custom_profiling ()
{
  L_open_input_file (L_input_file);

  if(L_do_buf_info)
    {
      if((buf_info_file = fopen (buf_info_file_name, "a")) == NULL)
	{
	  L_punt("Unable to open buf info file for printing: %s", 
		 buf_info_file_name);
	}
    }

  /* Process all data and functions within a file */
  while (L_get_input () != L_INPUT_EOF)
    {
      if (L_token_type == L_INPUT_FUNCTION)
	{
	  L_define_fn_name (L_fn->name);

	  fprintf (L_OUT, "# Unprofiled encoded lcode\n");
	  L_print_custom_profile_encoding (L_OUT, L_fn);

	  /* Do not output lcode */
	  L_delete_func (L_fn);
	}
      else
	{
	  /* Do not output lcode */
	  L_delete_data (L_data);
	}
    }

  L_close_input_file (L_input_file);

  if(L_do_buf_info)
    fclose(buf_info_file);

  return;
}

/*
 *      Generate Converted Lcode
 */
void
L_gen_code (Parm_Macro_List * command_line_macro_list)
{
  /* Load the parmaters specific to Lencode */
  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Lencode", L_read_parm_Lencode);

  if (L_pmatch (encode_for, "simulation") ||
      L_pmatch (encode_for, "x86_trace"))
    {
      L_encode_for_simulation ();

    }
  else if (L_pmatch (encode_for, "profiling"))
    {
      L_encode_for_profiling ();
    }
  else if (L_pmatch (encode_for, "value_profiling"))
    {
      L_encode_for_value_profiling ();
    }
  /* 20030410 SZU
   * Added to support custom encode for custom emulate
   */
  else if (L_pmatch (encode_for, "custom"))
    {
      L_encode_for_custom_profiling ();
    }
  else
    L_punt ("Encoding for '%s' not supported", encode_for);

  fclose (L_OUT);

  return;
}
