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
 *      File: l_markpipe.c
 *      Description: Routines for marking loops for modulo scheduling
 *      Creation Date: April, 1997 - extracted from l_pipe_util.c
 *      Author: Daniel Lavery
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_markpipe.h>

/* Return values for L_load_with_same_address_in_cb. */
#define BEFORE_STORE 1
#define AFTER_STORE 2

#define LPIPE_CHECK_PCODE_SWP_INFO 0

/* parameters */

int Lpipe_mark_multi_exit_loops = 0;	/* pipeline multiple exit loops */

double Lpipe_min_header_weight = 100.0;	/* minimum weight of header cb
					   to make software pipelining
					   worthwhile */

double Lpipe_min_ave_iter = 4.0;	/* minimum average iterations
					   per invocation of the loop
					   for software pipelining to
					   be beneficial */

int Lpipe_allow_fp_conversion = 0;	/* Allow F_I, I_F, etc. in software
					   pipelines loops. */

int Lpipe_mark_potential_softpipe_loops = 0;  
                                        /* mark loops that could be
					 * pipelined with an attribute
					 * rather than actually setting
					 * the L_CB_SOFTPIPE flag to
					 * facilitate performance
					 * comparisons. WILL NOT
					 * ACTUALLY PIPELINE!  Used in
					 * Lsuperscalar.  */

int Lpipe_check_sync_arcs = 0;	        /* check sync arcs against
				 	 * static address analysis */

int Lpipe_add_swp_info = 0;	        /* add SWP_INFO pragmas or add
					 * information to attributes
					 * already added by Pcode */

int Lpipe_print_marking_statistics = 0;	/* Print statistics on loop marking
					 * in file mark_phx.stats. */

int Lmarkpipe_min_ii = 0;  /* 0 is no minimum. */
int Lmarkpipe_max_ii = 1000;
int Lmarkpipe_max_stages = 1000;


/* Read parameters from Softpipe section */
/* see l_pipe_util.h for parm descriptions */

void
L_read_parm_lmarkpipe (Parm_Parse_Info *ppi)
{
  L_read_parm_b (ppi, "mark_multi_exit_loops", &Lpipe_mark_multi_exit_loops);

  L_read_parm_lf (ppi, "markpipe_min_header_weight", 
		  &Lpipe_min_header_weight);
  L_read_parm_lf (ppi, "markpipe_min_ave_iter", 
		  &Lpipe_min_ave_iter);
  L_read_parm_b (ppi, "markpipe_allow_fp_conversion", 
		 &Lpipe_allow_fp_conversion);

  L_read_parm_b (ppi, "?mark_potential_softpipe_loops",
		 &Lpipe_mark_potential_softpipe_loops);
  L_read_parm_b (ppi, "check_sync_arcs", &Lpipe_check_sync_arcs);
  L_read_parm_b (ppi, "add_swp_info", &Lpipe_add_swp_info);

  L_read_parm_b (ppi, "print_marking_statistics",
		 &Lpipe_print_marking_statistics);
  L_read_parm_i (ppi, "markpipe_min_ii", &Lmarkpipe_min_ii);
  L_read_parm_i (ppi, "markpipe_max_ii", &Lmarkpipe_max_ii);
  L_read_parm_i (ppi, "markpipe_max_stages", &Lmarkpipe_max_stages);

  return;
}


/* return 1 if there is a load with the same address as store in the cb */
static int
L_load_with_same_address_in_cb (L_Cb * cb, L_Oper * mem_store)
{
  L_Oper *oper;
  int after_store = 0;		/* flag to indicate that the load which depends
				   on store comes after the store in the cb */

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (L_load_opcode (oper))
	{
	  if (L_same_operand (oper->src[0], mem_store->src[0]) &&
	      L_same_operand (oper->src[1], mem_store->src[1]))
	    {
	      if (after_store)
		return AFTER_STORE;
	      else
		return BEFORE_STORE;
	    }
	}

      if (oper == mem_store)
	{
	  after_store = 1;
	}

      /* if reach a another store to same address before reaching a load to
         the same address and before reaching mem_store, the value defined by
         mem_store is dead and the load which depends on mem_store must be
         after mem_store */
      if ((!after_store) && L_store_opcode (oper))
	{
	  if (L_same_operand (oper->src[0], mem_store->src[0]) &&
	      L_same_operand (oper->src[1], mem_store->src[1]))
	    {
	      after_store = 1;
	    }
	}
    }

  return 0;
}


/* return 1 if there is a store with the same address as mem_store in the cb */
static int
L_store_with_same_address_in_cb (L_Cb * cb, L_Oper * mem_store)
{
  L_Oper *oper;
  int after_store = 0;	/* flag to indicate that the store which depends
			   on mem_store comes after the mem_store in the cb */

  for (oper = cb->first_op; oper != NULL; oper = oper->next_op)
    {
      if (oper == mem_store)
	{
	  after_store = 1;
	}
      else
	{
	  if (L_store_opcode (oper))
	    {
	      if (L_same_operand (oper->src[0], mem_store->src[0]) &&
		  L_same_operand (oper->src[1], mem_store->src[1]))
		{
		  if (after_store)
		    return AFTER_STORE;
		  else
		    return BEFORE_STORE;
		}
	    }
	}
    }

  return 0;
}


/* return 1 if loop is suitable for software pipelining, 0 otherwise */
int
Lpipe_is_OK_softpipe_loop (L_Inner_Loop * loop, FILE * statfile,
			   int *counted_loop, int check_ii)
{
  L_Oper *oper;
  int num_opers = 0;		/* number of operations in loop */
  int i, j;
  L_Oper *feedback_op = loop->feedback_op;
  L_Cb *cb = loop->cb;
  L_Attr *attr;
  L_Attr *swp_attr;		/* SWP_INFO attribute for loop */
  int doserial = 0;		/* flag to indicate that current loop was
				   marked as a doserial loop by Pcode */
  int dosuper = 0;		/* flag to indicate that current loop was
				   marked as a dosuper loop by Pcode */
  int parloop = 0;		/* flag to indicate that current loop was
				   converted to a parloop by Pcode */
  int serloop = 0;		/* flag to indicate that current loop was
				   marked as a Serloop by Pcode */
  int doall = 0;		/* flag to indicate that current loop is
				   not marked with a LOOP attribute by Pcode,
				   but is marked with a DOALL attribute */
  int pcode_marked = 0;		/* flag to indicate that SWP_INFO attribute
				   was added by Pcode */
  int load_found;		/* flag to indicate that load with same address
				   as store was found in loop */
  int softpipe_loop = 1;	/* flag to indicate that loop is suitable for
				   software pipelining */

  /* flags to indicate presence of various
     hazards in loop */
  int cond_branch = 0;
  int func_call = 0;
  int macro_register = 0;
  int fp_conversion = 0;
  int scalar_reg_dep = 0;	/* cross-iteration register scalar dependence 
				   which is not a trivial recurrence - i.e.
				   not associated with an induction operation
				   and not a simple accumulation */
  int accumulation = 0;		/* Simple accumulation.  All uses besides the
				   accumulator op, if any, are in same
				   iteration. */
  int unpromoted_scalar = 0;	/* loop has both store and load to scalar
				   variable in memory */
  int scalar_mem_dep = 0;	/* cross-iteration memory scalar dependence due
				   to unpromoted scalar */
  int inner_carried_dep = 0;	/* Cross-iteration dependence carried by 
				   inner loop as indicated by a sync arc. */
  L_Operand *dest;
  L_Sync_Info *sync_info;
  L_Sync *sync;
  L_Oper *dep_oper;		/* oper at sink of dependence (sync arc) */
  L_Cb *dest_cb;		/* cb containing oper at sink of dependence */

  attr = L_find_attr (cb->attr, "LOOP");
  if (Lpipe_print_marking_statistics)
    {
      /* print source line number for loop if available and print cb id */
      if (attr != NULL)
	{
	  fprintf (statfile,
		   "\n..Analyzing loop on line " ITintmaxformat ", cb %d\n",
		   attr->field[1]->value.i, cb->id);
	}
      else
	{
	  fprintf (statfile, "..Analyzing loop with header cb %d\n", cb->id);
	}
    }

  if (attr != NULL)
    {
      if (!strcmp (attr->field[0]->value.s, "\"doserial\""))
	{
	  doserial = 1;
	  parloop = 1;
	  if (Lpipe_print_marking_statistics)
	    {
	      fprintf (statfile, "> Loop is a Doserial loop\n");
	    }
	}
      else if (!strcmp (attr->field[0]->value.s, "\"dosuper\""))
	{
	  dosuper = 1;
	  parloop = 1;
	  if (Lpipe_print_marking_statistics)
	    {
	      fprintf (statfile, "> Loop is a Dosuper loop\n");
	    }
	}
      else
	{
	  serloop = 1;
	  if (Lpipe_print_marking_statistics)
	    {
	      fprintf (statfile, "> Loop is a Serloop\n");
	    }
	}
    }
  else
    {
      attr = L_find_attr (cb->attr, "DOALL");
      if (attr != NULL)
	{
	  doall = 1;
	  if (Lpipe_print_marking_statistics)
	    {
	      fprintf (statfile, "> Loop is marked DOALL\n");
	    }
	}
      else
	{
	  if (Lpipe_print_marking_statistics)
	    {
	      fprintf (statfile,
		       "> Loop is not marked by Pcode "
		       "and is not marked DOALL\n");
	    }
	}
    }

  /* add SWP_INFO attribute if not already added */
  swp_attr = L_find_attr (cb->attr, "SWP_INFO");
  if (swp_attr == NULL)
    {
      if (dosuper && LPIPE_CHECK_PCODE_SWP_INFO)
	{
	  L_punt ("Lpipe_is_OK_softpipe_loop: Dosuper loop missing SWP_INFO "
		  "attribute, cb = %d\n", cb->id);
	}
      if (L_find_string_attr_field (swp_attr, "\"pcode\""))
	{
	  pcode_marked = 1;
	}
      if (Lpipe_add_swp_info)
	{
	  swp_attr = L_new_attr ("SWP_INFO", 0);
	  cb->attr = L_concat_attr (cb->attr, swp_attr);
	}
    }

  /* check loop characteristics */

  if (L_uncond_branch_opcode (feedback_op))
    {
      if (Lpipe_print_marking_statistics)
	fprintf (statfile,
		 "> Loop ends with unconditional jump to header \n");

      if (Lpipe_add_swp_info &&
	  !L_find_string_attr_field (swp_attr, "\"uncond_jump\""))
	L_set_string_attr_field (swp_attr, swp_attr->max_field,
				 "\"uncond_jump\"");
    }

  /* check if loop body executed a minimum number of times */
  if (cb->weight < Lpipe_min_header_weight)
    {
      if (Lpipe_print_marking_statistics)
	fprintf (statfile, "> Header cb weight less than %f\n",
		 Lpipe_min_header_weight);

      if (Lpipe_add_swp_info &&
	  !L_find_string_attr_field (swp_attr, "\"min_weight\""))
	L_set_string_attr_field (swp_attr, swp_attr->max_field,
				 "\"min_weight\"");
      softpipe_loop = 0;
    }

  /* check if loop has minimum number of iterations per invocation */
  if (loop->ave_iteration < Lpipe_min_ave_iter)
    {
      if (Lpipe_print_marking_statistics)
	fprintf (statfile,
		 "> Average iterations is %f which is less than %f\n",
		 loop->ave_iteration, Lpipe_min_ave_iter);

      if (Lpipe_add_swp_info &&
	  !L_find_string_attr_field (swp_attr, "\"min_ave_iter\""))
	L_set_string_attr_field (swp_attr, swp_attr->max_field,
				 "\"min_ave_iter\"");
      softpipe_loop = 0;
    }

  /* check if loop will meet the maximum II requirements */
  if (check_ii)
    softpipe_loop &= Lpipe_is_OK_ii(loop, statfile, swp_attr);

  if (!L_is_counted_inner_loop (loop))
    {
      *counted_loop = 0;
      if (Lpipe_print_marking_statistics)
	fprintf (statfile,
		 "> Loop is not a counted loop with both"
		 " iteration variable and loop bound promoted"
		 " to registers\n");
      if (Lpipe_add_swp_info &&
	  !L_find_string_attr_field (swp_attr, "\"not_counted_loop\""))
	L_set_string_attr_field (swp_attr, swp_attr->max_field,
				 "\"not_counted_loop\"");
    }
  else
    {
      if (!L_find_attr (cb->attr, "COUNTED_LOOP"))
	{
	  attr = L_new_attr ("COUNTED_LOOP", 0);
	  cb->attr = L_concat_attr (cb->attr, attr);
	}
      *counted_loop = 1;
    }

  /* Check characteristics of each operation in loop.  Could break 
     out of this loop after finding the first hazard that prevents
     software pipelining.  Instead, keep going to find all the reasons why
     the loop is not pipelined. */

  for (oper = feedback_op->prev_op; oper != NULL; oper = oper->prev_op)
    {
      num_opers++;

      /* check if single basic block loop with no function calls */

      if (L_cond_branch_opcode (oper))
	{
	  cond_branch = 1;
	  *counted_loop = 0;
	  if (!Lpipe_mark_multi_exit_loops)
	    softpipe_loop = 0;

	  if (pcode_marked && LPIPE_CHECK_PCODE_SWP_INFO)
	    {
	      if (!L_find_string_attr_field (swp_attr, "\"control\""))
		{
		  L_punt
		    ("Lpipe_is_OK_softpipe_loop: "
		     "control flow missed by Pcode Dosuper conversion, "
		     "cb = %d op = %d\n",
		     cb->id, oper->id);
		}
	    }
	}

      if (L_subroutine_call_opcode (oper))
	{
	  func_call = 1;
	  softpipe_loop = 0;
	  if (pcode_marked && LPIPE_CHECK_PCODE_SWP_INFO)
	    {
	      if (!(L_find_string_attr_field (swp_attr, "\"jsr_SE\"") ||
		    L_find_string_attr_field (swp_attr, "\"jsr_NSE\"")))
		{
		  L_warn
		    ("Lpipe_is_OK_softpipe_loop: function call missed "
		     "by Pcode Dosuper conversion, cb = %d op = %d\n",
		     cb->id, oper->id);
		}
	    }
	}

      if (L_initializing_predicate_define_opcode (oper) ||
	  L_general_pred_comparison_opcode (oper))
	{
	  if (Lpipe_add_swp_info &&
	      !L_find_string_attr_field (swp_attr, "\"hyperblock\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"hyperblock\"");
	    }
	}

#if 0
      /* ITI/MCM 09/22/99
         Macro registers cannot be handled by MVE. However, the schedule
         manager understands this and will draw the correct arcs assuming
         that MVE CANNOT rename to allow overlap.  Thus, this check is
         not longer needed.  */

      /* OLD COMMENT: */
      /* don't software pipeline loop if it contains a write to a macro
         register (because can't apply modulo variable expansion to them). */
      for (i = 0; i < L_max_dest_operand; i++)
	{
	  if (L_is_macro (oper->dest[i]))
	    {
	      macro_register = 1;
	      softpipe_loop = 0;
	    }
	}
#endif

      /* Most machines move data between the floating-point and integer
         registers by doing a store to and load from a fixed memory location.
         This causes recurrences which are not supported now. */
      if ((oper->opc == Lop_F_I) ||
	  (oper->opc == Lop_F2_I) ||
	  (oper->opc == Lop_I_F) || (oper->opc == Lop_I_F2))
	{
	  fp_conversion = 1;
	  if (!Lpipe_allow_fp_conversion)
	    softpipe_loop = 0;
	}

      /* Check for cross iteration register dependences that could cause
         recurrences.  The cross-iteration register dependences associated 
         with the induction variables do not result in complex recurrences */
      if (!L_is_inner_loop_ind_op (loop->cb, oper))
	{
	  for (j = 0; j < L_max_dest_operand; j++)
	    {
	      dest = oper->dest[j];
	      /* register that is live out when the loop back branch is taken
	         is a potential cross-iteration scalar dependence */
	      if ((L_is_register (dest) || L_is_macro (dest)) &&
		  (L_in_oper_OUT_set (cb, feedback_op, dest, TAKEN_PATH)))
		{
		  /* if register is used in loop body before an oper which
		     defines it, then there is a cross-iteration scalar 
		     dependence which is not a simple accumulation */
		  if ((oper->prev_op != NULL) &&
		      !L_no_uses_in_range (dest, cb->first_op, oper->prev_op))
		    {
		      scalar_reg_dep = 1;
		      /* Check to see if Pcode also found this scalar
		         dependence.  If looking at doserial loop, 
		         scalar_reg_dep may be the result of array
		         accesses that were global variable migrated 
		         out of loop, so can't compare in this case. */
		      if (dosuper && LPIPE_CHECK_PCODE_SWP_INFO)
			{
			  if (!L_find_string_attr_field (swp_attr,
							 "\"dep_scalar\""))
			    {
			      L_punt
				("Lpipe_is_OK_softpipe_loop: scalar"
				 " dependence missed by Pcode Dosuper"
				 " conversion, cb = %d op = %d\n",
				 cb->id, oper->id);
			    }
			}
		    }
		  else
		    {
		      /* there is a simple accumulation other than the
		         induction ops */
		      accumulation = 1;
		      /* cannot check against pcode here because accumulation
		         could be into an array element and so would not
		         be marked as a scalar dep by Pcode */
		    }
		}
	    }
	}

      /* Check for a scalar variable which is not promoted
         to a register and which is both written and read, or
         written more than once in the loop.   This will always result 
         in a recurrence.  Could also be an array element and the 
         invariant address calculation has been removed from the loop.  
         This analysis is not conservative.  Any definition of the
         address  will cause it assume there is no dependence.  If
         it finds a dependence, though, that dependence does exist.
         So use it only to check for missing sync arcs. */
      if (L_store_opcode (oper))
	{
	  load_found = L_load_with_same_address_in_cb (cb, oper);
	  if (load_found &&
	      L_no_defs_in_range (oper->src[0], cb->first_op, cb->last_op) &&
	      L_no_defs_in_range (oper->src[1], cb->first_op, cb->last_op))
	    {
	      unpromoted_scalar = 1;

	      /* specially mark loops with a cross-iteration scalar
	         flow dependence */
	      if (load_found == BEFORE_STORE)
		{
		  scalar_mem_dep = 1;
		  /* If cross-iteration scalar flow dependence, check to see if
		     Pcode also found it.  If loop is doserial, it is possible
		     that the dependence is for an array element and the
		     invariant address calculation has been removed from the 
		     loop.  So don't compare if doserial.  */
		  if (dosuper && LPIPE_CHECK_PCODE_SWP_INFO)
		    {
		      if (!L_find_string_attr_field (swp_attr,
						     "\"dep_scalar\""))
			{
			  L_punt
			    ("Lpipe_is_OK_softpipe_loop: scalar "
			     "dependence missed by Pcode Dosuper "
			     "conversion, cb = %d op = %d\n",
			     cb->id, oper->id);
			}
		    }
		}
	    }
	  if (L_store_with_same_address_in_cb (cb, oper) &&
	      L_no_defs_in_range (oper->src[0], cb->first_op, cb->last_op) &&
	      L_no_defs_in_range (oper->src[1], cb->first_op, cb->last_op))
	    {
	      unpromoted_scalar = 1;
	      /* Pcode Dosuper conversion ignores scalar output and anti- 
	         dependences, so can't check them here */
	    }
	}

      /* Check for any inner loop carried sync arcs between opers in loop.
         This check should produce the same results as the one above for
         scalar dependences for dosuper loops.  Arcs from an oper to itself 
         can be handled trivially, so don't count that. */
      if ((L_store_opcode (oper) || L_load_opcode (oper)) &&
	  oper->sync_info != NULL)
	{

	  sync_info = oper->sync_info;

	  /* look at the head syncs for each oper */

	  for (i = 0; i < sync_info->num_sync_out; i++)
	    {
	      sync = sync_info->sync_out[i];
	      dep_oper = sync->dep_oper;
	      if (IS_INNER_CARRIED (sync->info) && (dep_oper != oper))
		{
		  dest_cb = L_oper_hash_tbl_find_cb (L_fn->oper_hash_tbl,
						     dep_oper->id);
		  if (dest_cb == cb)
		    {
		      inner_carried_dep = 1;
		      if (!L_use_sync_arcs)
			{
			  softpipe_loop = 0;
			}
		    }
		}
	    }
	}
    }

  if (Lpipe_print_marking_statistics)
    {

      if (cond_branch)
	{
	  if (!Lpipe_mark_multi_exit_loops)
	    {
	      fprintf (statfile,
		       "> Software pipelining prohibited by control flow\n");
	    }
	  else
	    {
	      fprintf (statfile, "> loop contains multiple exits\n");
	    }
	  if (Lpipe_add_swp_info &&
	      !L_find_string_attr_field (swp_attr, "\"control\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"control\"");
	    }
	}

      if (func_call)
	{
	  fprintf (statfile,
		   "> Software pipelining prohibited by function call\n");
	  if (Lpipe_add_swp_info
	      && (!L_find_string_attr_field (swp_attr, "\"jsr_SE\"")
		  || !L_find_string_attr_field (swp_attr, "\"jsr_NSE\"")
		  || !L_find_string_attr_field (swp_attr, "\"jsr\"")))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"jsr\"");
	    }
	}

      if (macro_register)
	{
	  fprintf (statfile,
		   "> Software pipelining prohibited by write "
		   "to macro register\n");
	  if (Lpipe_add_swp_info
	      && !L_find_string_attr_field (swp_attr, "\"macro\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"macro\"");
	    }
	}

      if (fp_conversion)
	{
	  fprintf (statfile,
		   "> loop contains conversion to/from floating point\n");
	  /* JWS -- let's give this a shot */
	  if (!Lpipe_allow_fp_conversion)
	    fprintf (statfile,
		     "> Software pipelining prohibited by conversion "
		     "to/from floating point\n");
	  if (Lpipe_add_swp_info
	      && !L_find_string_attr_field (swp_attr, "\"conversion\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"conversion\"");
	    }
	}

      /* if looking at doserial loop, scalar_reg_dep may be the result
         of array accesses that were global variable migrated out of
         loop */
      if (scalar_reg_dep)
	{
	  fprintf (statfile,
		   "> Loop contains a cross-iteration register dependence\n");
	  if (Lpipe_add_swp_info
	      && !L_find_string_attr_field (swp_attr, "\"dep_scalar_reg\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"dep_scalar_reg\"");
	    }
	}

      if (accumulation)
	{
	  fprintf (statfile, "> Loop contains an accumulation.\n");
	  if (Lpipe_add_swp_info &&
	      !L_find_string_attr_field (swp_attr, "\"accumulation\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"accumulation\"");
	    }
	}

      /* unpromoted_scalar may actually be an array
         dependence if looking at a doserial loop */
      if (unpromoted_scalar)
	{
	  fprintf (statfile, "> Loop contains an unpromoted scalar\n");
	  if (Lpipe_add_swp_info &&
	      !L_find_string_attr_field (swp_attr, "\"unpromoted_scalar\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"unpromoted_scalar\"");
	    }
	}

      /* scalar_mem_dep may actually be an array
         dependence if looking at a doserial loop */
      if (scalar_mem_dep)
	{
	  fprintf (statfile,
		   "> Loop contains a cross-iteration scalar memory "
		   "dependence due to unpromoted scalar\n");
	  if (Lpipe_add_swp_info
	      && !L_find_string_attr_field (swp_attr, "\"dep_scalar_mem\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"dep_scalar_mem\"");
	    }
	}

      /* unpromoted_scalar may actually be an array
         dependence if looking at a doserial loop */
      if (inner_carried_dep)
	{
	  fprintf (statfile,
		   "> Loop contains a non-trivial cross-iteration memory "
		   "dependence\n");
	  if (Lpipe_add_swp_info
	      && !L_find_string_attr_field (swp_attr,
					    "\"inner_carried_sync_arc\""))
	    {
	      L_set_string_attr_field (swp_attr, swp_attr->max_field,
				       "\"inner_carried_sync_arc\"");
	    }
	}
    }

  /* If function has sync arcs, compare results of address analysis and
     sync arc check for dosuper loops.  In general, if find inner carried
     dependence, cannot also check if address analysis found it.  If address
     is loaded from memory, address analysis will assume address is modified
     and that there is no dependence. */
  if (Lpipe_check_sync_arcs && dosuper && L_func_contains_dep_pragmas)
    {
      if (unpromoted_scalar && !inner_carried_dep)
	{
	  fprintf (stderr, "Error in function %s at Cb %i\n", L_fn->name,
		   cb->id);
	  L_punt
	    ("Lpipe_is_OK_softpipe_loop: sync arcs and address analysis "
	     "do not agree\n");
	}
    }

  return (softpipe_loop);
}


/*************************************************************************
                Start/Stop Function Definitions
*************************************************************************/

/* add start and stop pseudo-nodes for the purpose of computing MinDist,
   priority, and earliest and latest issue time */
static void
Lmarkpipe_create_start_stop_nodes (SM_Cb * sm_cb)
{
  L_Oper *start_oper;
  SM_Oper *sm_start_oper;
  L_Oper *stop_oper;
  SM_Oper *sm_stop_oper;
  SM_Oper *sm_oper;
  SM_Reg_Action *action;
  SM_Dep *dep_in;

  /* create start node */
  start_oper = L_create_new_op (Lop_NO_OP);
  sm_start_oper = SM_insert_oper_after (sm_cb, start_oper, NULL);
  /* Make a sync source so that a sync dep can originate from this instr. */
  if (sm_start_oper->ext_dest[SM_SYNC_ACTION_INDEX] == NULL)
    {
      sm_start_oper->ext_dest[SM_SYNC_ACTION_INDEX] =
	SM_add_reg_action (sm_start_oper, MDES_SYNC_OUT, SM_SYNC_ACTION_INDEX,
			   SM_SYNC_ACTION_OPERAND);
    }

  start_oper->flags = L_SET_BIT_FLAG(start_oper->flags, L_OPER_START_NODE);

  /* create stop node */
  stop_oper = L_create_new_op (Lop_NO_OP);
  sm_stop_oper = SM_insert_oper_after (sm_cb, stop_oper,
				       sm_cb->last_serial_op);

  stop_oper->flags = L_SET_BIT_FLAG(stop_oper->flags, L_OPER_STOP_NODE);

  /* Extra dependences (sync) are automatically created between
     the start and stop opers.  They must be ignored. */
  for (action = sm_start_oper->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_in = action->first_dep_in; dep_in != NULL;
	   dep_in = dep_in->next_dep_in)
	{
	  if (dep_in->from_action->sm_op == sm_start_oper ||
	      dep_in->from_action->sm_op == sm_stop_oper)
	    SM_ignore_dep (dep_in, SM_MVE_IGNORE);
	}
    }
  for (action = sm_stop_oper->first_op_action; action != NULL;
       action = action->next_op_action)
    {
      for (dep_in = action->first_dep_in; dep_in != NULL;
	   dep_in = dep_in->next_dep_in)
	{
	  if (dep_in->from_action->sm_op == sm_start_oper ||
	      dep_in->from_action->sm_op == sm_stop_oper)
	    SM_ignore_dep (dep_in, SM_MVE_IGNORE);
	}
    }

  /* add a flow dependence from start node to all other opers and from
     all other opers to stop node */
  for (sm_oper = sm_cb->first_serial_op; sm_oper != NULL;
       sm_oper = sm_oper->next_serial_op)
    {
      if ((sm_oper == sm_start_oper) || (sm_oper == sm_stop_oper))
	continue;

      /* Make a sync source so that a sync dep can 
	 originate from this instr. */
      if (sm_oper->ext_dest[SM_SYNC_ACTION_INDEX] == NULL)
	{
	  sm_oper->ext_dest[SM_SYNC_ACTION_INDEX] =
	    SM_add_reg_action (sm_oper, MDES_SYNC_OUT, SM_SYNC_ACTION_INDEX,
			       SM_SYNC_ACTION_OPERAND);
	}
#ifdef HAMM
      SM_add_dep (sm_start_oper->ext_dest[SM_SYNC_ACTION_INDEX],
		  sm_oper->ext_src[SM_SYNC_ACTION_INDEX],
		  SM_FIXED_DELAY | SM_HARD_DEP |
		  SM_START_STOP_DEP | SM_FLOW_DEP | SM_SYNC_DEP, 0, 0, 0,
		  SM_HARD_DEP | SM_SOFT_DEP);
      SM_add_dep (sm_oper->ext_dest[SM_SYNC_ACTION_INDEX],
		  sm_stop_oper->ext_src[SM_SYNC_ACTION_INDEX],
		  SM_FIXED_DELAY | SM_HARD_DEP |
		  SM_START_STOP_DEP | SM_FLOW_DEP | SM_SYNC_DEP, 0, 0, 0,
		  SM_HARD_DEP | SM_SOFT_DEP);
#else
      SM_add_dep (sm_start_oper->ext_dest[SM_SYNC_ACTION_INDEX],
		  sm_oper->ext_src[SM_SYNC_ACTION_INDEX],
		  SM_FIXED_DELAY | SM_HARD_DEP |
		  SM_START_STOP_DEP | SM_FLOW_DEP | SM_SYNC_DEP, 0, 0, 0,
		  SM_HARD_DEP | SM_SOFT_DEP);
      SM_add_dep (sm_oper->ext_dest[SM_SYNC_ACTION_INDEX],
		  sm_stop_oper->ext_src[SM_SYNC_ACTION_INDEX],
		  SM_FIXED_DELAY | SM_HARD_DEP |
		  SM_START_STOP_DEP | SM_FLOW_DEP | SM_SYNC_DEP, 0, 0, 0,
		  SM_HARD_DEP | SM_SOFT_DEP);
#endif
    }
}


/* delete start and stop pseudo-nodes and their associated dependences  */
static void
Lmarkpipe_delete_start_stop_nodes (SM_Cb * sm_cb)
{
  SM_Oper *sm_oper;

  sm_oper = sm_cb->first_serial_op;

  while (sm_oper)
    {
      if (L_START_STOP_NODE(sm_oper->lcode_op))
	{
	  SM_Oper *next_serial_oper = sm_oper->next_serial_op;
	  L_Cb *cb = sm_oper->sm_cb->lcode_cb;
	  L_Oper *op = sm_oper->lcode_op;

	  /* Delete the sm_op */
	  SM_delete_oper (sm_oper);

	  /* Remove the lcode op from the cb */
	  L_delete_oper (cb, op);

	  sm_oper = next_serial_oper;
	}
      else
	sm_oper = sm_oper->next_serial_op;
    }
}

/*************************************************************************
                II Check Function Definitions
*************************************************************************/

int Lpipe_is_OK_ii(L_Inner_Loop * loop, FILE *statfile, L_Attr *swp_attr)
{
  SM_Cb *sm_cb = NULL;
  L_Cb *header_cb = loop->cb;
  Softpipe_MinII *MinII;
  L_Attr *attr;
  int result = 1;

  /* L_do_flow_analysis (fn, LIVE_VARIABLE | REACHING_DEFINITION); */

  /* Fix infinite lifetimes? */

  /* Rename disjoint vregs in loop? */

  /* L_iterative_partial_deadcode (fn); */

  /* Build SM version of loop cb. */
  sm_cb =
    SM_new_cb (lmdes, header_cb,
	       SM_PREPASS | SM_MODULO | SM_NORMALIZE_ISSUE);

  /* Temporarily insert the new start and stop nodes. */
  Lmarkpipe_create_start_stop_nodes (sm_cb);

#if 0  
  SM_print_cb_dependences (stdout, sm_cb);
#endif  

  MinII = Lpipe_determine_mii (sm_cb, 10000);
  
  /* if MinII fails criteria, then set result to FALSE. */
  /* check if loop will meet the maximum II requirements */
  if ( MinII->MinII < Lmarkpipe_min_ii )
    {
      if (Lpipe_print_marking_statistics)
	{
	  fprintf (statfile,
		   "> MinII %d which is less than %d\n",
		   MinII->MinII, Lmarkpipe_min_ii);
	}
      if (Lpipe_add_swp_info &&
	  !L_find_string_attr_field (swp_attr, "\"min_ii\""))
	{
	  L_set_string_attr_field (swp_attr, swp_attr->max_field,
				   "\"min_ii\"");
	}
      result = 0;
    }
  if ( MinII->MinII > Lmarkpipe_max_ii )
    {
      if (Lpipe_print_marking_statistics)
	{
	  fprintf (statfile,
		   "> MinII %d which is greater than %d\n",
		   MinII->MinII, Lmarkpipe_max_ii);
	}
      if (Lpipe_add_swp_info &&
	  !L_find_string_attr_field (swp_attr, "\"max_ii\""))
	{
	  L_set_string_attr_field (swp_attr, swp_attr->max_field,
				   "\"max_ii\"");
	}
      result = 0;
    }

  /* Delete temporary start and stop nodes. */
  Lmarkpipe_delete_start_stop_nodes (sm_cb);

  SM_delete_cb (sm_cb);
  
  attr = L_new_attr ("markpipe_MinII_res_rec", 2);
  L_set_int_attr_field (attr, 0, MinII->res_MinII);
  L_set_int_attr_field (attr, 1, MinII->rec_MinII);
  header_cb->attr = L_concat_attr (header_cb->attr, attr);

  attr = L_new_attr ("markpipe_loop_dep_height", 1);
  L_set_int_attr_field (attr, 0, Lpipe_get_DepHeight());
  header_cb->attr = L_concat_attr (header_cb->attr, attr);

#if 0
  printf ("Lpipe_is_OK_ii: (cb %d) ResMII = %d, RecMII = %d\n",
	  header_cb->id, MinII->res_MinII, MinII->rec_MinII );
#endif  

  Lcode_free (MinII);
  Lpipe_free_op_array();
  Lpipe_free_MinDist();

  return result;
}
