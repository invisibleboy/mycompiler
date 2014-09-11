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
 *      File: l_loop_prep.c
 *      Description: Phase 1 loop preparation for modulo scheduling
 *      Creation Date: January, 1994
 *                     Some routines are rewrites of those in 
 *                     l_opti2.c, created July 1992 by Nancy Warter
 *      Author: Daniel Lavery, Nancy Warter
 *
 *      Revision Date: Summer 1999
 *      Authors: IMPACT Technologies: Matthew Merten and John Gyllenhaal
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "l_softpipe_int.h"
#include "l_loop_prep.h"

/*************************************************************************
                Function Prototypes
*************************************************************************/

static int Lpipe_is_post_increment (L_Oper * ind_op, L_Oper * loop_back_br);
static void Lpipe_fix_max (L_Func * fn, L_Operand * ind_incr,
			   L_Operand * ind_var, L_Oper * loop_back_br,
			   L_Cb * preheader);
static L_Oper *Lpipe_loop_induction_reversal (L_Func * fn, L_Cb * header_cb,
					      L_Cb * preheader_cb,
					      L_Oper * loop_back_br);
static void Lpipe_rem_loop (L_Func * fn, L_Cb * header_cb,
			    L_Cb * preheader_cb, L_Oper * loop_back_br,
			    L_Oper * loop_incr_op);

/* file to which to print stats when checking if loop can be
   software pipelined */
static FILE *prep_statfile;

/*************************************************************************
                (De)Intialization by the Code Generator
*************************************************************************/

/* Called once per invocation of Limpact running phase 1, this
   function reads in the parameter files for Lmarkpipe and Lsoftpipe
   during initialization. */
void
Lpipe_loop_prep_init (Parm_Macro_List * command_line_macro_list)
{
  /* Load the parameters specific to software pipelining */
  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Lmarkpipe", L_read_parm_lmarkpipe);

  /* Renamed 'softpipe' to 'Lsoftpipe' -HCH 11/04/00 */
  L_load_parameters_aliased (L_parm_file, command_line_macro_list,
			     "(Lsoftpipe", "(Softpipe", L_read_parm_lpipe);

  prep_statfile = stdout;

  if (Lpipe_check_loops_in_phase1 && Lpipe_print_statistics)
    {
      prep_statfile = fopen ("mark_ph1.stats", "a");
      if (prep_statfile == NULL)
	L_punt ("Error while processing file %s: cannot append to file "
		"mark_ph1.stats" "\n", L_input_file);
    }
}


void
Lpipe_loop_prep_end ()
{
  if (Lpipe_check_loops_in_phase1 && Lpipe_print_statistics)
    {
      fclose (prep_statfile);
    }
}

/*************************************************************************
                Interface to phase 1 of the code generator
*************************************************************************/

/* Do checks to make sure loop is suitable for software pipelining, 
   and perform induction variable reversal.  Called by code generator
   during phase 1, before annotation. */
void
Lpipe_softpipe_loop_prep (L_Func * fn)
{
  L_Inner_Loop *loop;
  L_Cb *header_cb;		/* loop header - will become the kernel */
  L_Oper *loop_incr_op;		/* loop induction operation */
  L_Attr *attr;
  int counted_loop = 0;

  if (Lpipe_check_loops_in_phase1 && Lpipe_print_statistics)
    {
      fprintf (prep_statfile,
	       "\n..Analyze loops for software pipelining in function (%s)\n",
	       fn->name);
    }

  PG_setup_pred_graph (fn);

  /* Detect inner loops and create preheaders. We modulo schedule 
     only a single cb, so look for marked header cb.  Full loop detection is
     not designed to be used after superblock formation and superscalar
     optimization, so don't use it here. */
  L_reset_loop_headers (fn);
  L_inner_loop_detection (fn, 1);

  /* Prepare loops */
  for (loop = fn->first_inner_loop; loop != NULL;
       loop = loop->next_inner_loop)
    {

      header_cb = loop->cb;

      /* check if loop is marked for software pipelining */
      attr = L_find_attr (header_cb->attr, "L_CB_SOFTPIPE");
      if (L_EXTRACT_BIT_VAL (header_cb->flags, L_CB_SOFTPIPE) || attr)
	{

	  if (L_uncond_branch_opcode (header_cb->last_op) &&
	      L_is_predicated (header_cb->last_op) &&
	      L_EXTRACT_BIT_VAL (header_cb->flags, 
				 L_CB_HYPERBLOCK_NO_FALLTHRU))
	    {
	      L_warn ("Promoting uncond br of nofallthru "
		      "pipelined loop (cb %d)", header_cb->id);
	      L_delete_operand (header_cb->last_op->pred[0]);
	      header_cb->last_op->pred[0] = NULL;
	    }

	  /* Need to know if induction variables are live out, and need to
	     look for recurrences. */
	  L_do_flow_analysis (fn, LIVE_VARIABLE);

	  /* remove sync arcs which are impossible */
	  L_adjust_invalid_sync_arcs_in_cb (header_cb);

	  if (Lpipe_check_loops_in_phase1)
	    {
	      /* check to see if loop is still suitable for pipelining */
	      if (attr != NULL)
		{
		  if (!Lpipe_is_OK_softpipe_loop (loop, prep_statfile,
						  &counted_loop, 0))
		    {
		      if (Lpipe_print_statistics)
			{
			  fprintf (prep_statfile,
				   ":( Loop with header cb %d no "
				   "longer OK for software pipelining\n",
				   header_cb->id);
			}
		      header_cb->attr = L_delete_attr (header_cb->attr, attr);
		    }
		  else if (Lpipe_print_statistics)
		    {
		      fprintf (prep_statfile, ":) Loop with header cb %d "
			       "still OK for software pipelining\n",
			       header_cb->id);
		    }
		  continue;
		}
	      else if (!Lpipe_is_OK_softpipe_loop (loop, prep_statfile,
						   &counted_loop, 0))
		{
		  if (Lpipe_print_statistics)
		    {
		      fprintf (prep_statfile, ":( Loop with header cb %d no "
			       "longer OK for software pipelining\n",
			       header_cb->id);
		    }
		  header_cb->flags = L_CLR_BIT_FLAG (header_cb->flags,
						     L_CB_SOFTPIPE);
		  continue;
		}
	      if (Lpipe_print_statistics)
		fprintf (prep_statfile, ":) Loop with header cb %d still OK "
			 "for software pipelining\n", header_cb->id);
	    }

	  /* print source line number and/or cb id for loop */
	  if (Lpipe_debug >= 1)
	    {
	      attr = L_find_attr (header_cb->attr, "LOOP");
	      if ((attr != NULL) &&
		  (!strcmp (attr->field[0]->value.s, "\"dosuper\"") ||
		   !strcmp (attr->field[0]->value.s, "\"doserial\"") ||
		   !strcmp (attr->field[0]->value.s, "\"while\"") ||
		   !strcmp (attr->field[0]->value.s, "\"for\"") ||
		   !strcmp (attr->field[0]->value.s, "\"do\"")))
		{
		  fprintf (stderr,
			   "Preparing loop (function %s, line %d, cb %d)\n",
			   fn->name, (int) (attr->field[1]->value.i),
			   header_cb->id);
		}
	      else
		{
		  fprintf (stderr, "Preparing loop (function %s, cb %d)\n",
			   fn->name, header_cb->id);
		}
	    }
	  

	  /* Only do this when necessary -- otherwise, breaks
	   * loops ending with uncond jumps to headers, for which
	   * no natural fallthru exists!
	   */

	  if (Lpipe_can_create_fallthru_cb (header_cb))
	    {
	      L_create_cb_at_fall_thru_path (header_cb, 0);
	    }
	  else if (Lpipe_do_induction_reversal || Lpipe_schema == REM_LOOP)
	    {
	      L_punt ("Lpipe_software_loop_prep: "
		      "Cannot create required fallthru cb");
	    }

	  PG_setup_pred_graph (fn);

	  /* perform induction variable reversal */
	  if (Lpipe_do_induction_reversal)
	    {
	      loop_incr_op = Lpipe_loop_induction_reversal (fn, header_cb,
							    loop->preheader,
							    loop->
							    feedback_op);
	    }
	  else
	    {
	      loop_incr_op = L_find_inner_loop_counter (loop);
	    }

	  /* set up remainder loop skeleton */
	  if (Lpipe_schema == REM_LOOP)
	    {
	      Lpipe_rem_loop (fn, header_cb, loop->preheader,
			      loop->feedback_op, loop_incr_op);
	    }
	}
    }
  return;
}

/*************************************************************************
                Helper functions to Lpipe_softpipe_loop_prep
*************************************************************************/

#define UNKNOWN_WEIGHT 0	/* for unknown profile information */
#define DUMMY_VAL 95		/* dummy value for multiply which
				   hopefully will not get strength reduced.
				   Used for remainder loop. */

/*--------------------------------------------------------------------------
               Routines for induction variable reversal
  ------------------------------------------------------------------------*/

/* Check if this induction is a post increment.   There should be no uses of 
   destination of ind_op after ind_op except for loop back branch.   And there 
   should be a use of destination of ind_op before ind_op. */
static int
Lpipe_is_post_increment (L_Oper * ind_op, L_Oper * loop_back_br)
{
  int i;
  L_Oper *oper;

  for (oper = ind_op->next_op; oper != loop_back_br; oper = oper->next_op)
    {
      /* is this induction variable used by a later oper? */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (ind_op->dest[0], oper->src[i]))
	    return 0;		/* not post increment */
	}
    }

  /* in order for ind_op to be post increment, it must have a use
     somewhere earlier in loop body */
  for (oper = ind_op->prev_op; oper != NULL; oper = oper->prev_op)
    {
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (ind_op->dest[0], oper->src[i]))
	    {
	      return 1;
	    }
	}
    }
  return 0;
}


/* Subtract induction increment from loop bound of loop back branch */
static void
Lpipe_fix_max (L_Func * fn, L_Operand * ind_incr, L_Operand * ind_var,
	       L_Oper * loop_back_br, L_Cb * preheader)
{
  L_Oper *new_oper;
  L_Operand *max_src;		/* loop bound */
  int max_pos;			/* position of loop bound operand 
				   in loop back branch oper */

  /* find out which operand is the loop bound */
  if (L_same_operand (ind_var, loop_back_br->src[0]))
    {
      max_src = loop_back_br->src[1];
      max_pos = 1;
    }
  else
    {
      max_src = loop_back_br->src[0];
      max_pos = 0;
    }

  /* if max = const && incr = const then max = max - incr */
  if (L_is_int_constant (max_src) && L_is_int_constant (ind_incr))
    {
      max_src->value.i = max_src->value.i - ind_incr->value.i;
    }

  /*  if max = const && incr = reg then oper new_max_reg = max - incr_reg */
  else if (L_is_int_constant (max_src) && L_is_register (ind_incr))
    {
      new_oper = L_create_new_op (Lop_SUB);
      new_oper->dest[0] =
	L_new_register_operand (++(fn->max_reg_id), L_CTYPE_INT,
				L_PTYPE_NULL);
      new_oper->src[0] = L_copy_operand (max_src);
      new_oper->src[1] = L_copy_operand (ind_incr);
      L_delete_operand (loop_back_br->src[max_pos]);
      loop_back_br->src[max_pos] = L_copy_operand (new_oper->dest[0]);
      L_insert_oper_after (preheader, preheader->last_op, new_oper);
    }

  /* If max = reg && incr = const||reg then oper new_max_reg = max_reg - incr.
     Create new_max_reg because max_reg may be live out. */
  else if (L_is_register (max_src))
    {
      new_oper = L_create_new_op (Lop_SUB);
      new_oper->dest[0] =
	L_new_register_operand (++(fn->max_reg_id), L_CTYPE_INT,
				L_PTYPE_NULL);
      new_oper->src[0] = L_copy_operand (max_src);
      new_oper->src[1] = L_copy_operand (ind_incr);
      max_src->value.r = new_oper->dest[0]->value.r;
      L_insert_oper_after (preheader, preheader->last_op, new_oper);
    }
  else
    {
      L_punt ("Lpipe_fix_max:  invalid operand type combination");
    }
}


/* convert loop induction variables from post-increment to preincrement */
static L_Oper *
Lpipe_loop_induction_reversal (L_Func * fn, L_Cb * header_cb,
			       L_Cb * preheader_cb, L_Oper * loop_back_br)
{
  L_Oper *op, *prev_op;
  L_Operand *ind_incr_src;	/* increment operand for induction op */
  L_Operand *ind_var_src;	/* induction variable */
  int loop_incr_flag = 0;	/* flag to indicate if induction op is 
				   loop counter op */
  L_Oper *new_oper;
  L_Oper *loop_incr_op = 0;	/* loop counter induction op */
  L_Cb *postheader_cb;		/* place to put adjustments for live out
				   induction variables */
  int i;

  postheader_cb = header_cb->next_cb;

  for (op = header_cb->first_op; op != 0; op = op->next_op)
    {

      /* check if loop induction variable */
      if (!L_is_inner_loop_ind_op (header_cb, op))
	continue;

      if (L_same_operand (op->dest[0], op->src[0]))
	{
	  ind_var_src = op->src[0];
	  ind_incr_src = op->src[1];
	}
      else
	{
	  ind_var_src = op->src[1];
	  ind_incr_src = op->src[0];
	}

      /* verify that it is a post-increment variable */
      if (!Lpipe_is_post_increment (op, loop_back_br))
	continue;

      /* check if oper is loop counter induction op */
      for (i = 0; i < L_max_src_operand; i++)
	{
	  if (L_same_operand (op->dest[0], loop_back_br->src[i]))
	    {
	      loop_incr_op = op;
	      loop_incr_flag = 1;
	    }
	}

      /* if variable is post-increment except for a use by the loop back
         branch, subtract increment from max in preheader */
      if (loop_incr_flag)
	{
	  Lpipe_fix_max (fn, ind_incr_src, ind_var_src, loop_back_br,
			 preheader_cb);
	}

      /*  substract increment from initial value in preheader */
      new_oper = L_create_new_op (Lop_SUB);
      new_oper->dest[0] = L_copy_operand (op->dest[0]);
      new_oper->src[0] = L_copy_operand (ind_var_src);
      new_oper->src[1] = L_copy_operand (ind_incr_src);
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_oper);

      /* add increment back in the postheader if value is live out of loop */
      if (L_in_oper_OUT_set (header_cb, loop_back_br, op->dest[0],
			     FALL_THRU_PATH))
	{
	  new_oper = L_create_new_op (Lop_ADD);
	  new_oper->dest[0] = L_copy_operand (op->dest[0]);
	  new_oper->src[0] = L_copy_operand (ind_var_src);
	  new_oper->src[1] = L_copy_operand (ind_incr_src);
	  L_insert_oper_before (postheader_cb, postheader_cb->first_op,
				new_oper);
	}

      /* Move loop induction operation to beginning of loop.  Keep track
         of previous op so that can continue where left off. */
      prev_op = op->prev_op;
      L_move_op_to_start_of_block (header_cb, header_cb, op);
      op = prev_op;
    }

  /* loop_incr_op used in setting up remainder loop skeleton */
  return (loop_incr_op);
}


/*-------------------------------------------------------------------------
                        Remainder loop skeleton
  -------------------------------------------------------------------------*/

/* For remainder loop code schema, set up remainder loop skeleton
   before phase 1 annotation so that the opers added for the code
   schema will be properly annotated. */
static void
Lpipe_rem_loop (L_Func * fn, L_Cb * header_cb, L_Cb * preheader_cb,
		L_Oper * loop_back_br, L_Oper * loop_incr_op)
{
  L_Cb *prologue_cb;
  L_Cb *epilogue_cb;
  L_Cb *remainder_cb;
  L_Flow *flow;
  L_Cb *fall_thru_cb;		/* post header cb */
  L_Operand *ind_var_src;	/* induction variable */
  L_Operand *ind_incr_src;	/* increment operand of induction operation */
  L_Oper *new_oper1;
  L_Oper *new_oper2;
  L_Operand *max_src;
  int max_pos;
  L_Operand *new_max_src;
  L_Oper *oper;
  L_Flow *new_flow;

  /* create prologue, epilogue, and remainder loop cb's */

  /* assume that most of the time, there are enough iterations to use the 
     pipeline.  So prologue weight is approx. equal to preheader weight */
  prologue_cb = L_create_cb (preheader_cb->weight);
  L_insert_cb_before (fn, header_cb, prologue_cb);
  preheader_cb->dest_flow->dst_cb = prologue_cb;
  prologue_cb->src_flow = L_new_flow (0, preheader_cb, prologue_cb,
				      prologue_cb->weight);
  prologue_cb->dest_flow = L_new_flow (0, prologue_cb, header_cb,
				       prologue_cb->weight);
  /* find preheader to header flow */
  flow = header_cb->src_flow;
  if (flow->src_cb == header_cb)
    flow = flow->next_flow;
  flow->src_cb = prologue_cb;

  /* add epilogue cb */
  epilogue_cb = L_create_cb (prologue_cb->weight);
  L_insert_cb_after (fn, header_cb, epilogue_cb);
  flow = header_cb->dest_flow;
  if (flow->dst_cb == header_cb)
    flow = flow->next_flow;
  fall_thru_cb = flow->dst_cb;
  flow->dst_cb = epilogue_cb;
  epilogue_cb->src_flow = L_new_flow (0, header_cb, epilogue_cb,
				      epilogue_cb->weight);

  /* add remainder loop */
  remainder_cb = L_create_cb (UNKNOWN_WEIGHT);
  L_insert_cb_after (fn, epilogue_cb, remainder_cb);
  epilogue_cb->dest_flow = L_new_flow (0, epilogue_cb, remainder_cb,
				       UNKNOWN_WEIGHT);
  remainder_cb->src_flow = L_new_flow (0, epilogue_cb, remainder_cb,
				       UNKNOWN_WEIGHT);
  remainder_cb->dest_flow = L_new_flow (0, remainder_cb, fall_thru_cb,
					UNKNOWN_WEIGHT);

  for (flow = fall_thru_cb->src_flow; flow->src_cb != header_cb;
       flow = flow->next_flow);
  flow->src_cb = remainder_cb;

  /* add instructions to preheader */

  if (L_same_operand (loop_incr_op->dest[0], loop_incr_op->src[0]))
    {
      ind_var_src = loop_incr_op->src[0];
      ind_incr_src = loop_incr_op->src[1];
    }
  else
    {
      ind_var_src = loop_incr_op->src[1];
      ind_incr_src = loop_incr_op->src[0];
    }

  if (L_is_register (ind_incr_src))
    {
      /* multiply loop increment by dummy val representing (minimum #
         of iterations - 1) if pipeline is entered */
      new_oper1 = L_create_new_op (Lop_MUL);
      new_oper1->src[0] = L_new_gen_int_operand (DUMMY_VAL);
      new_oper1->src[1] = L_copy_operand (ind_incr_src);
      new_oper1->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						   L_CTYPE_INT, L_PTYPE_NULL);
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_oper1);
      /* add to induction variable */
      new_oper2 = L_create_new_op (Lop_ADD);
      new_oper2->src[0] = L_copy_operand (new_oper1->dest[0]);
      new_oper2->src[1] = L_copy_operand (ind_var_src);
      new_oper2->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						   L_CTYPE_INT, L_PTYPE_NULL);
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_oper2);
    }
  else
    {
      /* if induction increment is constant, it can be folded with
         (minimum # of iterations - 1) later */
      new_oper2 = L_create_new_op (Lop_ADD);
      new_oper2->src[0] = L_copy_operand (ind_incr_src);
      new_oper2->src[1] = L_copy_operand (ind_var_src);
      new_oper2->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						   L_CTYPE_INT, L_PTYPE_NULL);
      L_insert_oper_after (preheader_cb, preheader_cb->last_op, new_oper2);
    }

  /* add operations to prologue cb */

  /* branch to remainder loop on opposite condition of loop back br */

  new_oper1 = L_create_new_op (loop_back_br->opc);
  L_copy_compare (new_oper1, loop_back_br);
  L_negate_compare (new_oper1);
  if (L_same_operand (loop_back_br->src[0], ind_var_src))
    {
      new_oper1->src[0] = L_new_register_operand (fn->max_reg_id, L_CTYPE_INT,
						  L_PTYPE_NULL);
      max_src = loop_back_br->src[1];
      max_pos = 1;
      new_oper1->src[1] = L_copy_operand (max_src);
    }
  else
    {
      new_oper1->src[1] = L_new_register_operand (fn->max_reg_id, L_CTYPE_INT,
						  L_PTYPE_NULL);
      max_src = loop_back_br->src[0];
      max_pos = 0;
      new_oper1->src[0] = L_copy_operand (max_src);
    }
  new_oper1->src[2] = L_new_cb_operand (remainder_cb);
  L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper1);
  /* add flows */
  /* assume most of the time, this branch away from pipeline is not taken */
  new_flow = L_new_flow (1, prologue_cb, remainder_cb, 0);
  prologue_cb->dest_flow = L_concat_flow (prologue_cb->dest_flow, new_flow);
  new_flow = L_new_flow (1, prologue_cb, remainder_cb, 0);
  remainder_cb->src_flow = L_concat_flow (remainder_cb->src_flow, new_flow);

  if (L_is_register (ind_incr_src))
    {
      /* multiply loop increment by dummy val representing (unroll - 1) */
      new_oper1 = L_create_new_op (Lop_MUL);
      new_oper1->src[0] = L_new_gen_int_operand (DUMMY_VAL);
      new_oper1->src[1] = L_copy_operand (ind_incr_src);
      new_oper1->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						   L_CTYPE_INT, L_PTYPE_NULL);
      L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper1);
      /* subtract from max */
      new_oper2 = L_create_new_op (Lop_SUB);
      new_oper2->src[0] = L_copy_operand (max_src);
      new_oper2->src[1] = L_copy_operand (new_oper1->dest[0]);
      new_oper2->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						   L_CTYPE_INT, L_PTYPE_NULL);
      new_max_src = new_oper2->dest[0];
      L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper2);
    }
  else
    {
      /* if induction increment is constant, it can be folded with
         (unroll - 1) later */
      if (L_is_register (max_src))
	{
	  new_oper2 = L_create_new_op (Lop_ADD);
	  new_oper2->src[0] = L_copy_operand (ind_incr_src);
	  new_oper2->src[1] = L_copy_operand (max_src);
	  new_oper2->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						       L_CTYPE_INT,
						       L_PTYPE_NULL);
	  new_max_src = new_oper2->dest[0];
	  L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper2);
	}
      else
	{
	  /* if induction increment and max are both constants, probably
	     should fold the whole thing together */
	  new_oper1 = L_create_new_op (Lop_MOV);
	  new_oper1->src[0] = L_copy_operand (ind_incr_src);
	  new_oper1->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						       L_CTYPE_INT,
						       L_PTYPE_NULL);
	  L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper1);
	  new_oper2 = L_create_new_op (Lop_SUB);
	  new_oper2->src[0] = L_copy_operand (max_src);
	  new_oper2->src[1] = L_copy_operand (new_oper1->dest[0]);
	  new_oper2->dest[0] = L_new_register_operand (++(fn->max_reg_id),
						       L_CTYPE_INT,
						       L_PTYPE_NULL);
	  new_max_src = new_oper2->dest[0];
	  L_insert_oper_after (prologue_cb, prologue_cb->last_op, new_oper2);
	}
    }

  /* add operations to epilogue cb */

  /* branch to decide if there are remainder iterations, taken if
     no remaining iterations */
  new_oper1 = L_copy_operation (loop_back_br);
  L_copy_compare (new_oper1, loop_back_br);
  L_negate_compare (new_oper1);
  L_delete_operand (new_oper1->src[2]);
  new_oper1->src[2] = L_new_cb_operand (fall_thru_cb);
  L_insert_oper_after (epilogue_cb, epilogue_cb->last_op, new_oper1);
  /* add flow between epilogue and fallthru cbs */
  new_flow = L_new_flow (1, epilogue_cb, fall_thru_cb, UNKNOWN_WEIGHT);
  epilogue_cb->dest_flow = L_concat_flow (epilogue_cb->dest_flow, new_flow);
  new_flow = L_new_flow (1, epilogue_cb, fall_thru_cb, UNKNOWN_WEIGHT);
  fall_thru_cb->src_flow = L_concat_flow (fall_thru_cb->src_flow, new_flow);

  /* copy original loop to remainder cb */
  for (oper = header_cb->first_op; oper != NULL; oper = oper->next_op)
    {
      new_oper1 = L_copy_operation (oper);
      if (oper == loop_back_br)
	{
	  L_change_branch_dest (new_oper1, L_find_branch_dest (new_oper1),
				remainder_cb);
	}
      L_insert_oper_after (remainder_cb, remainder_cb->last_op, new_oper1);
    }
  /* add flow from remainder_cb to itself - it is a loop */
  new_flow = L_new_flow (1, remainder_cb, remainder_cb, UNKNOWN_WEIGHT);
  remainder_cb->dest_flow = L_concat_flow (remainder_cb->dest_flow, new_flow);
  new_flow = L_new_flow (1, remainder_cb, remainder_cb, UNKNOWN_WEIGHT);
  remainder_cb->src_flow = L_concat_flow (remainder_cb->src_flow, new_flow);

  /* change max for loop back branch */
  L_delete_operand (loop_back_br->src[max_pos]);
  loop_back_br->src[max_pos] = L_copy_operand (new_max_src);
}
