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
/*****************************************************************************
 * phase2_func.c                                                             *
 * ------------------------------------------------------------------------- *
 * Scheduling, register allocation, and optimization                         *
 *                                                                           *
 * AUTHORS: D.A. Connors, D.I. August, J. Pierce, J.W. Sias                  *
 *****************************************************************************/
/* 09/12/02 REK Updating file to use the new opcode maps and completers
 *              scheme.
 */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include "ltahoe_main.h"
#include "ltahoe_op_query.h"
#include "ltahoe_completers.h"
#include <Lcode/l_opti.h>
#include <Lcode/mia_opti.h>
#include <Lcode/l_jump_opti.h>
#include <Lcode/l_lcode.h>
#include "phase1_opgen.h"
#include <Lcode/sm.h>
#include <Lcode/l_softpipe.h>
#include "phase1_func.h"
#include "phase2_reg.h"
#include "phase2_br_hint.h"
#include "phase2_icache.h"
#include "phase2_sync.h"
#include "phase2_memstk.h"
#include "phase2_func.h"

#undef DEBUG

/* #define DEBUG_DUMP */
#undef SHOW_CHANGES

/* prototypes */
static void O_mark_speculative (L_Func * fn);
void L_check_for_special_dep_violation (L_Func * fn);
extern void Ltahoe_sp_removal (L_Func *fn);

int
Ltahoe_remove_dead_pred_defs (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *op, *next_op;
  int cnt, dest_indx;

  cnt = 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (op = cb->first_op; op; op = next_op)
      {
	next_op = op->next_op;
	if (!L_general_predicate_define_opcode (op))
	  continue;
	for (dest_indx = 0; dest_indx < 2; dest_indx++)
	  {
	    if (op->dest[dest_indx] &&
		!LT_is_P0_operand (op->dest[dest_indx]))
	      break;
	  }			/* for dest */
	if (dest_indx == 2)
	  {
	    L_nullify_operation (op);
	    op->proc_opc = TAHOEop_NOP_I;
	    op->src[0] = L_new_gen_int_operand (0);
	    cnt++;
	  }			/* if */
      }				/* for op */
  return cnt;
}				/* Ltahoe_remove_dead_pred_defs */

void
Ltahoe_repair_epilogue_defines (L_Cb * cb)
{
  L_Oper *oper, *next_op;
  L_Oper *rts_op = NULL, *epi_op = NULL;

  for (oper = cb->first_op; oper; oper = next_op)
    {
      next_op = oper->next_op;
      if (oper->opc == Lop_EPILOGUE)
	{
	  L_remove_oper (cb, oper);
	  epi_op = oper;
	}			/* if */
      else if (L_subroutine_return_opcode (oper))
	rts_op = oper;
    }				/* for oper */

  if (!rts_op || !epi_op)
    L_punt ("O_process_func: rts missing");

  L_insert_oper_before (cb, rts_op, epi_op);

  for (oper = cb->first_op; oper != epi_op; oper = next_op)
    {
      next_op = oper->next_op;

      if ((oper->opc == Lop_DEFINE) && !oper->dest[0])
	{
	  L_remove_oper (cb, oper);
	  L_insert_oper_before (cb, rts_op, oper);
	}			/* if */
    }				/* for oper */
  return;
}				/* Ltahoe_repair_epilogue_defines */

void
Ltahoe_output_dependence_stall_removal (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *opA, *opB;
  L_Oper *new_op;
  L_Attr *ld_cycle, *consumer_cycle;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
	continue;
      for (opA = cb->first_op; opA; opA = opA->next_op)
	{
	  if (L_int_load_opcode (opA) ||
	      L_int_postincrement_load_opcode (opA))
	    {
	      if (!(ld_cycle = L_find_attr (opA->attr, "cycle")))
		continue;
	      for (opB = opA->next_op; opB; opB = opB->next_op)
		{
		  if (!(opB->dest[0] &&
			(L_same_operand (opB->dest[0], opA->dest[0]))))
		    continue;
		  if (L_postincrement_load_opcode (opB) ||
		      L_postincrement_store_opcode (opB))
		    continue;
		  if ((L_same_operand (opB->dest[0], opB->src[0])) ||
		      (L_same_operand (opB->dest[0], opB->src[1])))
		    {
		      if (!(consumer_cycle =
			    L_find_attr (opB->attr, "cycle")))
			continue;

		      if ((consumer_cycle->field[0]->value.i -
			   ld_cycle->field[0]->value.i) <= 4)
			{
			  new_op = L_create_new_op (Lop_MOV);
			  new_op->proc_opc = TAHOEop_MOV_GR;
			  new_op->src[0] =
			    L_new_register_operand (++L_fn->max_reg_id,
						    L_CTYPE_LLONG,
						    L_PTYPE_NULL);
			  new_op->dest[0] = opB->dest[0];
			  new_op->pred[0] = L_copy_operand (opB->pred[0]);
			  opB->dest[0] = L_copy_operand (new_op->src[0]);
#if 1
			  printf ("Broke output dependence stall in cb %d "
				  "from op %d to op %d by inserting "
				  "mov op %d\n",
				  cb->id, opA->id, opB->id, new_op->id);
#endif
			  L_insert_oper_after (cb, opB, new_op);
			  opB = new_op;
			}	/* if */
		      else
			break;	/* Since we are far enough away from opA */
		    }		/* if */
		}		/* for opB */
	    }			/* if */
	}			/* for opA */
    }				/* for cb */
  L_do_flow_analysis (fn, LIVE_VARIABLE | REACHING_DEFINITION);
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
	L_local_copy_propagation (cb);
    }				/* for cb */
}				/* Ltahoe_output_dependence_stall_removal */

/*****************************************************************************\
 *
 * PHASE II: Software pipeline, prepass, register allocate, postpass
 *
 * This annotation is specific to the Merced processer. Currently annotation
 * generates the function prologue and epilogue, and adjusts stack frame 
 * addresses.
 *
\*****************************************************************************/

static int
Ltahoe_mark_swp_spills (L_Func *fn)
{
  int cnt = 0;
  L_Cb *cb, *next_cb;
  
  for (cb = fn->first_cb; cb; cb = next_cb)
    {
      L_Oper *op, *prev_op, *next_op;
      L_Attr *attr;

      next_cb = cb->next_cb;

      if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE))
	continue;

      for (op = cb->first_op; op ; op = op->next_op)
	{
	  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_SPILL_CODE))
	    break;
	}

      if (!op)
	continue;

      /* Softpipe loop with spill code identified */
      cnt++;

      L_warn ("Spill code forced postpass on %s() loop cb %d.",
	      fn->name, cb->id);

      cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_SOFTPIPE);

      attr = L_new_attr ("SWP_SPILL", 0);
      cb->attr = L_concat_attr (cb->attr, attr);

      for (op = cb->first_op; op; op = next_op)
	{
	  next_op = op->next_op;

	  if (L_is_opcode (Lop_NO_OP, op) ||
	      LT_is_template_op (op))
	    L_delete_oper (cb, op);
	  else if ((attr = L_find_attr (op->attr, "isl")))
	    op->attr = L_delete_attr (op->attr, attr);
	}

      /* Split the defines before, after the kernel into separate CB's
       * to keep the scheduler from munging them.
       */

      op = cb->first_op;
      prev_op = NULL;

      while (op && (op->opc == Lop_DEFINE))
	{
	  prev_op = op;
	  op = op->next_op;
	}

      if (!op)
	continue;

      if (prev_op)
	cb = L_split_cb_after (fn, cb, prev_op);

      while (op && (op->opc != Lop_DEFINE))
	{
	  prev_op = op;
	  op = op->next_op;
	}

      if (!op)
	continue;

      if (prev_op)
	L_split_cb_after (fn, cb, prev_op);
    }
  
  return cnt;
}

void
Ltahoe_estimate_liveness (L_Func *fn, char *str)
{
  L_Cb *cb;
  L_Oper *op;
  L_Attr *attr;
  int liveness, oplive;

  L_do_flow_analysis (fn, LIVE_VARIABLE);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      liveness = Set_size (L_get_cb_IN_set (cb));
      for (op = cb->first_op; op; op = op->next_op)
	{
	  oplive = Set_size (L_get_oper_IN_set (op));

	  if (oplive > liveness)
	    liveness = oplive;
	}
      attr = L_new_attr (str, 1);
      L_set_int_attr_field (attr, 0, liveness);
      cb->attr = L_concat_attr (cb->attr, attr);
    }
 
  return;
}

void
Ltahoe_promote_eff_uncond_br (L_Func *fn)
{
  L_Cb *cb;
  L_Oper *op;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (!(op = cb->last_op))
	continue;
      if (L_has_fallthru_to_next_cb (cb))
	continue;

      while (op->proc_opc == TAHOEop_NOP_B)
	op = op->prev_op;

      if (!L_cond_branch (op))
	continue;

      if (op->next_op)
	{
	  L_remove_oper (cb, op);
	  L_insert_oper_after (cb, cb->last_op, op);
	}

      L_delete_operand (op->pred[0]);
      op->pred[0] = NULL;
      L_warn ("In %s() cb %d Ltahoe_promote_eff_uncond_br "
	      "ate a final predicate\n", fn->name, cb->id);
    }
}


void
O_red_load (L_Func * fn)
{
  L_Cb *cb;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    L_local_redundant_load (cb);
  return;
}				/* O_red_load */


void
O_process_func (L_Func * fn, Parm_Macro_List * command_line_macro_list)
{
  L_Cb *cb;
  int pred_swap_space_size;
  int int_swap_space_size = 0;	/* Size of swap space for integer,
				   computed by R_register_allocation
				   used by
				   O_postpass_adjust_memory_stack */
  int fp_swap_space_size = 0;	/* Size of swap space for floating point */

  L_cleanup_after_mopti (fn);

  LTD ("FUNCTION %s START PHASE 2", fn->name);

  /*
   * Software Pipelining
   * ----------------------------------------------------------------------
   */

  if (L_do_software_pipelining)
    {
      LTD ("Pre-modulo optimizations (SWP)");

      if (L_do_machine_opt && (Ltahoe_machine_opt_mask & 0x8))
	{
	  Mopti_phase2_optimizations (fn);
	  if (Mopti2_redundant_memory_ops)
	    O_red_load (fn);
	  L_cleanup_after_mopti (fn);
	}			/* if */

      /* Perform conservative post increment optimization.  Don't
         perform the optimization if the induction variable has more
         than one use or is live out, to avoid inhibiting the
         speculation of loads.  The advantage of doing this early is
         more accurate prepass scheduling, and the chance to perform
         the optimization for modulo scheduled loops. */

      if (Ltahoe_postinc_ld || Ltahoe_postinc_st)
	{
	  /* Perform some post-increment conversion in softpipe loops */

	  LTD ("Adding conservative post-incr operations");

	  L_do_flow_analysis (fn, LIVE_VARIABLE);

	  Mia_softpipe_post_increment_conversion (fn,
						  Ltahoe_postinc_ld,
						  Ltahoe_postinc_st);
	}			/* if */

      LTD ("Iterative modulo scheduling (SWP)");

      {
	L_Cb *cb;
	L_Oper *op;
	L_Operand *opd;
	int i;

	for (cb = fn->first_cb; cb; cb = cb->next_cb)
	  for (op = cb->first_op; op; op = op->next_op)
	    {
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (!(opd = op->src[i]))
		    continue;
		  if (!L_is_variable (opd))
		    continue;
		  if (!L_is_ctype_float (opd))
		    continue;

		  opd->ctype = L_CTYPE_DOUBLE;
		}		/* for i */
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (!(opd = op->dest[i]))
		    continue;
		  if (!L_is_variable (opd))
		    continue;
		  if (!L_is_ctype_float (opd))
		    continue;

		  opd->ctype = L_CTYPE_DOUBLE;
		}		/* for i */
	    }			/* for op */
      }

      Lpipe_software_pipeline (fn);

      LTD ("Post-modulo optimizations (SWP)");

      L_cleanup_after_mopti (fn);

      if (L_do_machine_opt && (Ltahoe_machine_opt_mask & 0x10))
	{
	  Mopti_phase2_optimizations (fn);
	  L_cleanup_after_mopti (fn);
	}			/* if */
    }				/* if */

  /* clean up empty blocks */
  L_remove_empty_cbs (fn);
  L_check_func (fn);

  /*
   * Pre-pass code scheduling:
   */

  Ltahoe_estimate_liveness (fn, "live1");
  
  if (L_do_prepass_sched)
    {
      LTD ("Prepass instruction scheduling");

      SM_schedule_fn (fn, lmdes, 1);

      {
	L_Cb *cb;
	for (cb = fn->first_cb; cb; cb = cb->next_cb)
	  {
	    if (!L_EXTRACT_BIT_VAL (cb->flags, L_CB_EPILOGUE))
	      continue;

	    Ltahoe_repair_epilogue_defines (cb);
	  }			/* for cb */
      }

#ifdef DEBUG_DUMP
      {
	FILE *dump;
	dump = fopen ("f_phase1.mco", "wt");
	L_print_func (dump, fn);
	fclose (dump);
      }
#endif
    }				/* if */

  Ltahoe_estimate_liveness (fn, "live2");

  if (L_do_machine_opt && (Ltahoe_machine_opt_mask & 0x20))
    {
      LTD ("Jump optimizations");

      Mopti_branch_target_expansion (fn);

      L_jump_combine_labels (fn, L_JUMP_ALLOW_SUPERBLOCKS);
      L_jump_combine_branch_to_uncond_branch (fn, L_JUMP_ALLOW_SUPERBLOCKS);
      L_jump_elim_branch_to_next_block (fn, L_JUMP_ALLOW_SUPERBLOCKS);

      L_remove_empty_cbs (fn);

      LTD ("Classical optimizations");

      Mopti_phase2_optimizations (fn);

      LTD ("Local redundant load elimination");

      if (Mopti2_redundant_memory_ops && !L_do_software_pipelining)
	for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	  L_local_redundant_load (cb);

      L_cleanup_after_mopti (fn);

      if (L_do_software_pipelining)
	{
	  L_Cb *new_cb;
	  /*
	   * If a cb containing a jump was merged into the kernel of a  
	   * modulo scheduled loop, need to push it back out again so it can
	   * be scheduled and given a template.
	   */

	  LTD ("Kernel jump removal (SWP)");

	  for (cb = fn->first_cb; cb; cb = cb->next_cb)
	    {
	      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
		  L_uncond_branch (cb->last_op) &&
		  (cb->last_op->src[0]->value.cb != cb))
		{
		  new_cb = L_create_cb_at_fall_thru_path (cb, 0);
		  new_cb->first_op->proc_opc = TAHOEop_BR_COND;
		}		/* if */
	    }			/* for cb */
	}			/* if */

      /* perform post increment optimization.  This is done after prepass
         scheduling so that it doesn't inhibit speculation of loads.
         If it was done before, both the address computation and the
         load need to be speculated in order to move the load.  This may
         not be possible, but the load itself could go. */

      if (Ltahoe_postinc_ld || Ltahoe_postinc_st)
	{
	  LTD ("Post-increment conversion");

	  L_do_flow_analysis (fn, LIVE_VARIABLE);

	  Mia_post_increment_conversion (fn,
					 Ltahoe_postinc_ld,
					 Ltahoe_postinc_st);
	}			/* if */
    }				/* if */

  Ltahoe_remove_dead_pred_defs (fn);

  /* PART 1/3 of Create Recovery Code For Speculation */
  if (L_do_recovery_code)
    {
      LTD ("Recovery code generation (RC)");
      /* 20031119 SZU */
#if defined(RC_CODE)
      RC_generate_recovery_code (fn);
      LTD ("Classical optimization (RC)");
      Mopti_phase2_optimizations (fn);
      L_cleanup_after_mopti (fn);
#endif
    }				/* if */

  if (L_do_prepass_sched)
    {
      LTD ("Dependence stall padding");
      Ltahoe_output_dependence_stall_removal (fn);
    }				/* if */

  /*
   * Perform register allocation
   *
   * global information available after register allocation
   *
   * spill_space_required
   * number_of_registers
   */

  if (L_do_register_allocation)
    {
      /* Demote predicates to increase register reuse */

      LTD ("Partial dead code removal / predicate demotion");

      L_partial_dead_code_removal (fn);
      L_predicate_demotion (fn);

      LTD ("Register allocation");

      O_register_allocation (fn, command_line_macro_list,
			     &int_swap_space_size, &fp_swap_space_size,
			     &pred_swap_space_size);
    }				/* if */

  /*
   * Reset L_CB_SOFTPIPE flag for software pipelined loops that contain
   * spill code so that postpass scheduling can schedule the spill code
   */

  if (L_do_software_pipelining)
    {
      LTD ("Checking for spill code in SWP loops (SWP)");
      Ltahoe_mark_swp_spills (fn);
    }				/* if */

  /*
   * Perform post "register allocation" code annotation.
   *
   * Certain commands such as prologue and epilogue can not be annotated
   * until all of the register characteristics and memory requirements
   * are known.
   *
   */

  if (L_do_register_allocation)
    {
      LTD ("Postpass annotation");

      O_postpass_adjust_memory_stack (fn,
				      int_swap_space_size,
				      fp_swap_space_size,
				      pred_swap_space_size, callee_flt_array);

      /* Clean up after stack adjustment */

      if (Ltahoe_do_postreg_const_fold && 
	  L_do_machine_opt && (Ltahoe_machine_opt_mask & 0x40))
	{
	  LTD ("Constant folding");
	  for (cb = fn->first_cb; cb; cb = cb->next_cb)
	    M_local_constant_folding (cb);
	}			/* if */

      O_register_cleanup ();
    }				/* if */

  if (Ltahoe_do_sp_removal)
    {
      LTD ("Redundant sp removal");
      Ltahoe_sp_removal (fn);
    }				/* if */

  if (Mopti_do_epilogue_merge)
    {
      LTD ("Epilogue merging");
      L_warn ("Epilogue merging is unsafe");
      Mopti_epilogue_merge (fn);
    }				/* if */

  /* PART 2/3 of Create Recovery Code For Speculation */
#if defined(RC_CODE)
  if (L_do_recovery_code)
    {
      LTD ("Recovery code recombination (RC)");
      RC_recombine_cbs (fn);
    }				/* if */
#endif

  L_cleanup_after_mopti (fn);

  /*
   * Perform post-pass instruction scheduling
   */

  if (L_do_postpass_sched)
    {
      LTD ("Postpass instruction scheduling");

      SM_schedule_fn (fn, lmdes, 0);

      if (M_model == M_IPF_ITANIUM)
	L_check_for_special_dep_violation (fn);
    }				/* if */
  else
    {
#if USE_IMPLICIT_NOSCHED
      LTD ("Marking function for implicit bundling (no postpass schedule");

      fn->attr = L_concat_attr (fn->attr,
				L_new_attr ("TAHOE_IMPLICIT_OUTPUT", 0));

#else
      LTD ("Post-pass bundling (no scheduling)");

      SM_bundle_fn (fn, lmdes);
#endif
    }				/* else */

  Ltahoe_promote_eff_uncond_br (fn);  

  /* PART 3/3 of Create Recovery Code For Speculation */

#if defined(RC_CODE)
  if (L_do_recovery_code)
    {
      LTD ("Repairing RC bundles (RC)");
      RC_fix_recovery_code_bundles (fn);
      L_cleanup_after_mopti (fn);
    }				/* if */
#endif

  O_mark_speculative (fn);

  if ((L_do_postpass_sched && Ltahoe_insert_branch_hints) ||
      (Ltahoe_use_streaming_only))
    {
      LTD ("Counting bundles for branch hinting (BRP)");
      O_count_bundles_before_branch_func (fn);
    }				/* if */

  if (Ltahoe_padding_threshold >= 0)
    {
      LTD ("Aligning CBs for cache performance");
      Ltahoe_cache_align_cbs (fn, Ltahoe_padding_threshold);
    }				/* if */

  if (Mopti_do_epilogue_merge)
    {
      LTD ("Cleaning up after epilogue merge");
      Mopti_epilogue_cleanup (fn);
    }				/* if */

  if (Ltahoe_tag_loads)
    {
      L_tag_load(fn);
    }				/* if */
  
  LTD ("FUNCTION %s END PHASE 2", fn->name);

  return;
}				/* O_process_func */

/* 
 *  Mark speculative operations and update tahoeops
 */

/* 09/12/02 REK With the new completers field, this is no longer needed. */
/*  #define SAMARK(top, base) { if (ctrl && data) top = base ## _SA; \ */
/*                         else if (ctrl) top = base ## _S; \ */
/*                         else if (ctrl) top = base ## _A; \ */
/*                         else top = base; } */

static void
O_mark_speculative (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  int tahoeop = -1;
  int tahoeCompleter = 0;
  int ctrl, data, vol;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  ctrl = 0;
	  vol = L_EXTRACT_BIT_VAL (oper->flags, L_OPER_VOLATILE);

	  if (L_EXTRACT_BIT_VAL (oper->flags, L_OPER_MASK_PE))
	    {
	      if (!L_EXTRACT_BIT_VAL (oper->flags, L_OPER_SPECULATIVE |
				      L_OPER_PROMOTED))
		{
#if 0
		  L_warn ("O_mark_speculative: op %d marked M"
			  " but not S. Fixing up.", oper->id);
#endif
		  oper->flags = L_SET_BIT_FLAG (oper->flags,
						L_OPER_SPECULATIVE);
		}		/* if */
	      ctrl = 1;
	    }			/* if */

	  data = L_EXTRACT_BIT_VAL (oper->flags, L_OPER_DATA_SPECULATIVE);

	  if (L_general_load_opcode (oper))
	    {
	      /* Fill ops have no speculative version */
	      if (LT_is_fill_op (oper))
		continue;

	      /* Set the completers field according to ctrl and data */
	      if (ctrl)
		if (data)
		  TC_SET_LD_TYPE (tahoeCompleter, TC_LD_TYPE_SA);
		else
		  TC_SET_LD_TYPE (tahoeCompleter, TC_LD_TYPE_S);
	      else if (data)
		TC_SET_LD_TYPE (tahoeCompleter, TC_LD_TYPE_A);
	      else
		TC_SET_LD_TYPE (tahoeCompleter, TC_LD_TYPE_NONE);

	      switch (oper->opc)
		{
		case Lop_LD_UC:
		case Lop_LD_POST_UC:
		case Lop_LD_C:
		case Lop_LD_POST_C:
		  tahoeop = !vol ? TAHOEop_LD1 : TAHOEop_LD1_ACQ;
		  break;

		case Lop_LD_UC2:
		case Lop_LD_POST_UC2:
		case Lop_LD_C2:
		case Lop_LD_POST_C2:
		  tahoeop = !vol ? TAHOEop_LD2 : TAHOEop_LD2_ACQ;
		  break;

		case Lop_LD_I:
		case Lop_LD_POST_I:
		case Lop_LD_UI:
		case Lop_LD_POST_UI:
		  tahoeop = !vol ? TAHOEop_LD4 : TAHOEop_LD4_ACQ;
		  break;

		case Lop_LD_Q:
		case Lop_LD_POST_Q:
		  tahoeop = !vol ? TAHOEop_LD8 : TAHOEop_LD8_ACQ;
		  break;

		case Lop_LD_F:
		case Lop_LD_POST_F:
		  if (data)
		    tahoeop = TAHOEop_LDFS_A;
		  else
		    tahoeop = TAHOEop_LDFS;
		  break;

		case Lop_LD_F2:
		case Lop_LD_POST_F2:
		  if (data)
		    tahoeop = TAHOEop_LDFD_A;
		  else
		    tahoeop = TAHOEop_LDFD;
		  break;

		case Lop_LD_UC_CHK:
		case Lop_LD_C_CHK:
		  tahoeop = TAHOEop_LD1_C;
		  break;

		case Lop_LD_UC2_CHK:
		case Lop_LD_C2_CHK:
		  tahoeop = TAHOEop_LD2_C;
		  break;

		case Lop_LD_UI_CHK:
		case Lop_LD_I_CHK:
		  tahoeop = TAHOEop_LD4_C;
		  break;

		case Lop_LD_Q_CHK:
		  tahoeop = TAHOEop_LD8_C;
		  break;

		case Lop_LD_F_CHK:
		  tahoeop = TAHOEop_LDFS_C;
		  break;

		case Lop_LD_F2_CHK:
		  tahoeop = TAHOEop_LDFD_C;
		  break;

		default:
		  L_punt ("O_postpass_alat_annotate: load opc %d for op %d",
			  oper->opc, oper->id);
		  break;
		}		/* switch */
	      oper->proc_opc = tahoeop;
	      oper->completers = tahoeCompleter;
	    }			/* if */
	  else if (oper->opc == Lop_CHECK_ALAT)
	    {
	      if (L_find_attr (oper->attr, "ck_clear"))
		{
		  tahoeop = TAHOEop_CHK_A;
		  tahoeCompleter |= TC_LD_C_CLR;
		}		/* if */
	      else if (L_find_attr (oper->attr, "ck_nclear"))
		{
		  tahoeop = TAHOEop_CHK_A;
		  tahoeCompleter |= TC_LD_C_NC;
		}		/* else if */
	      else
		{
		  tahoeop = TAHOEop_CHK_A;
		  tahoeCompleter |= TC_LD_C_NC;
		}		/* else */

	      oper->proc_opc = tahoeop;
	      oper->completers = tahoeCompleter;
	    }			/* else if */
	  else if (L_flt_arithmetic_opcode (oper) || 
		   L_dbl_arithmetic_opcode (oper))
	    {
	      if (data)
		L_punt("O_mark_speculative: data speculative float");

	      if (ctrl)
		{
		  switch (oper->proc_opc)
		    {
		    case TAHOEop_FMA_D:
		    case TAHOEop_FMA_S:
		    case TAHOEop_FMA:
		    case TAHOEop_FMS_D:
		    case TAHOEop_FMS_S:
		    case TAHOEop_FMS:
		    case TAHOEop_FNMA_D:
		    case TAHOEop_FNMA_S:
		    case TAHOEop_FNMA:
		    case TAHOEop_FAMAX:
		    case TAHOEop_FAMIN:
		    case TAHOEop_FMAX:
		    case TAHOEop_FMIN:
		    case TAHOEop_FRCPA:
		    case TAHOEop_FPRSQRTA:
		    case TAHOEop_FADD_D:
		    case TAHOEop_FADD_S:
		    case TAHOEop_FADD:
		    case TAHOEop_FSUB_D:
		    case TAHOEop_FSUB_S:
		    case TAHOEop_FSUB:
#ifdef DEBUG
		      L_warn ("O_mark_speculative: Marking spec fp op.");
#endif
		      if (Ltahoe_get_fsf (oper) == FSF_S0)
			Ltahoe_set_fsf (oper, FSF_S2);
		      else if (Ltahoe_get_fsf (oper) == FSF_S1)
			Ltahoe_set_fsf (oper, FSF_S3);
		      break;
		    case TAHOEop_FCHKF:
		    case TAHOEop_FCLRF:
		    default:
#ifdef DEBUG
		      L_warn ("O_mark_speculative: NOT marking spec fp op.");
#else
		      ;
#endif
		    }
		}
	    }
	}			/* for oper */
    }				/* for cb */
  return;
}				/* O_mark_speculative */

/*****************************************************************************\
 *
 * Does everything for a function
 *
\*****************************************************************************/

/*
 * Global initializations
 */
void
O_init (Parm_Macro_List * command_line_macro_list)
{
  O_register_init ();

  SM_init (command_line_macro_list);

  if (L_do_software_pipelining)
    Lpipe_init (command_line_macro_list);

  return;
}				/* O_init */

void
O_finalize ()
{
  if (L_do_software_pipelining)
    Lpipe_cleanup ();

  return;
}				/* O_finalize */

void
S_machine_rts (L_Oper * oper)
{
  if (oper->opc != Lop_RTS)
    L_punt ("S_machine_rts: oper not an RTS");

  oper->proc_opc = TAHOEop_BR_RET;
  if (!oper->src[0])
    oper->src[0] = Ltahoe_BMAC (RETADDR);
}				/* S_machine_rts */

void
S_machine_jump (L_Oper * oper)
{
  if (oper->opc != Lop_JUMP)
    L_punt ("S_machine_rts: oper not an RTS");

  oper->proc_opc = TAHOEop_BR_COND;
}				/* S_machine_jump */

int
S_machine_check (L_Oper * oper)
{
  switch (oper->opc)
    {
    case Lop_CHECK:
      if (!L_is_register (oper->src[0]) && !L_is_macro (oper->src[0]))
	{
	  L_print_oper (stderr, oper);
	  L_punt ("S_machine_check: Bad operand format");
	}
      if (L_is_ctype_int_direct (oper->src[0]->ctype))
	return TAHOEop_CHK_S;
      else
	return TAHOEop_CHK_S_F;
    default:
      L_punt ("S_machine_check: check not defined.");
    }				/* switch */
  return -1;
}				/* S_machine_check */

/*
 * PADDING of non-scoreboarded ops (E. Nystrom)
 * ----------------------------------------------------------------------
 */

#undef DEBUG_PADDING

static void
L_insert_padding_bw (L_Cb * from_cb, L_Oper * from_op, L_Cb * to_cb,
		     L_Oper * to_op, int cycles)
{
  L_Oper *define_op, *to_op_define_op, *new_op;
  int num_before, same_bundle = 0;
  int template, stop_bits;

  if (cycles < 1)
    L_punt ("L_insert_padding_bw: cycles to insert is less than 1.");

  /* Find the start of the bundle */
  num_before = 0;
  for (define_op = to_op->prev_op; define_op != NULL;
       define_op = define_op->prev_op)
    {
      if (define_op->proc_opc != TAHOEop_NON_INSTR)
	num_before++;
      if (define_op == from_op)
	same_bundle = 1;
      if ((define_op->opc == Lop_DEFINE) &&
	  (L_is_macro (define_op->dest[0])) &&
	  (define_op->dest[0]->value.mac == TAHOE_MAC_TEMPLATE))
	break;
    }				/* for define_op */
  if ((num_before > 2) || (num_before < 0))
    L_punt ("L_insert_padding_bw: %d instructions"
	    " before op in same bundle.", num_before);
  if (!define_op)
    L_punt ("L_insert_padding_bw: No template found!");

  to_op_define_op = define_op;

  template = (int) define_op->src[0]->value.i;
  stop_bits = (int) define_op->src[1]->value.i;

  if (same_bundle)
    {
      if (num_before == 2)
	{
	  if ((template == MISI) || (template == MSMI))
	    {
	      new_op = L_create_new_op (Lop_NO_OP);
	      new_op->proc_opc = TAHOEop_NOP_I;
	      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
	      L_insert_oper_before (to_cb, to_op, new_op);

	      define_op->src[1]->value.i |= 1;
	      cycles--;

	      new_op = L_create_new_op (Lop_DEFINE);
	      new_op->proc_opc = TAHOEop_NON_INSTR;
	      new_op->dest[0] =
		L_new_macro_operand (TAHOE_MAC_TEMPLATE,
				     L_CTYPE_LLONG, L_PTYPE_NULL);
	      new_op->src[0] = L_new_int_operand (MII, L_CTYPE_LLONG);	/* 0 */
	      new_op->src[1] =
		L_new_int_operand ((stop_bits | 1), L_CTYPE_LLONG);
	      if (cycles)
		{
		  new_op->src[0]->value.i = MISI;
		  new_op->src[1]->value.i |= 2;
		  cycles--;
		}		/* if */
	      L_insert_oper_before (to_cb, to_op, new_op);
	      to_op_define_op = new_op;

	      new_op = L_create_new_op (Lop_NO_OP);
	      new_op->proc_opc = TAHOEop_NOP_M;
	      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
	      L_insert_oper_before (to_cb, to_op, new_op);

	      new_op = L_create_new_op (Lop_NO_OP);
	      new_op->proc_opc = TAHOEop_NOP_I;
	      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
	      L_insert_oper_before (to_cb, to_op, new_op);
	    }			/* if */
	  else
	    {
	      DB_print_cb (to_cb);
	      L_punt ("L_insert_padding_bw: bundle %d appears invalid.",
		      template);
	    }			/* else */
	}			/* if */
      else if (num_before == 1)
	{
	  if (template == MSMI)
	    {
	      new_op = L_create_new_op (Lop_NO_OP);
	      new_op->proc_opc = TAHOEop_NOP_M;
	      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
	      L_insert_oper_before (to_cb, to_op, new_op);

	      new_op = L_create_new_op (Lop_NO_OP);
	      new_op->proc_opc = TAHOEop_NOP_I;
	      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
	      L_insert_oper_before (to_cb, to_op, new_op);

	      define_op->src[1]->value.i |= 1;
	      cycles--;

	      new_op = L_create_new_op (Lop_DEFINE);
	      new_op->proc_opc = TAHOEop_NON_INSTR;
	      new_op->dest[0] =
		L_new_macro_operand (TAHOE_MAC_TEMPLATE,
				     L_CTYPE_LLONG, L_PTYPE_NULL);
	      new_op->src[0] = L_new_int_operand (MMI, L_CTYPE_LLONG);	/* 0 */
	      new_op->src[1] =
		L_new_int_operand ((stop_bits | 1), L_CTYPE_LLONG);

	      if (cycles)
		{
		  new_op->src[0]->value.i = MSMI;
		  new_op->src[1]->value.i |= 4;
		  cycles--;
		}		/* if */
	      L_insert_oper_before (to_cb, to_op, new_op);

	      new_op = L_create_new_op (Lop_NO_OP);
	      new_op->proc_opc = TAHOEop_NOP_M;
	      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
	      L_insert_oper_before (to_cb, to_op, new_op);
	    }			/* if */
	  else
	    L_punt ("L_insert_padding_bw: bundle appears invalid.");
	}			/* else if */
      else
	{
	  L_punt ("L_insert_padding_bw: No instructions before"
		  "me to be the from_op");
	}			/* else */
    }				/* if */

  if (cycles)
    {
      /* First add a cycle (if possible) to the bundle with to_op */
      if (num_before == 2)
	{
	  if ((template == MII) /* 0 */  && !(stop_bits & 2))
	    {
	      define_op->src[0]->value.i = MISI;
	      define_op->src[1]->value.i = (define_op->src[1]->value.i | 2);
	      cycles--;
	    }			/* if */
	}			/* if */
      if (num_before > 1)
	{
	  if ((template == MMI) /* 4 */  && !(stop_bits & 4))
	    {
	      define_op->src[0]->value.i = MSMI;
	      define_op->src[1]->value.i = (define_op->src[1]->value.i | 4);
	      cycles--;
	    }			/* if */
	}			/* if */
    }				/* if */

  /* Next, add a cycle (if possible) to the bundle with from_op */
  if (cycles)
    {
      /* Find the start of the bundle */
      num_before = 0;
      for (define_op = from_op->prev_op; define_op != NULL;
	   define_op = define_op->prev_op)
	{
	  if (define_op->proc_opc != TAHOEop_NON_INSTR)
	    num_before++;
	  if ((define_op->opc == Lop_DEFINE) &&
	      (L_is_macro (define_op->dest[0])) &&
	      (define_op->dest[0]->value.mac == TAHOE_MAC_TEMPLATE))
	    break;
	}			/* for define_op */

      if ((num_before > 2) || (num_before < 0))
	L_punt ("L_insert_padding_bw: %d instructions"
		" before op in same bundle.", num_before);
      if (!define_op)
	L_punt ("L_insert_padding_bw: No template found!");

      template = (int) define_op->src[0]->value.i;
      stop_bits = (int) define_op->src[1]->value.i;

      if (num_before < 2)
	{
	  if ((template == MII) /* 0 */  && !(stop_bits & 2))
	    {
	      define_op->src[0]->value.i = MISI;
	      define_op->src[1]->value.i = (define_op->src[1]->value.i | 2);
	      cycles--;
	    }			/* if */
	}			/* if */
      if (num_before == 0)
	{
	  if ((template == MMI) /* 4 */  && !(stop_bits & 4))
	    {
	      define_op->src[0]->value.i = MSMI;
	      define_op->src[1]->value.i = (define_op->src[1]->value.i | 4);
	      cycles--;
	    }			/* if */
	}			/* if */
      if (cycles && !(stop_bits & 1))
	{
	  define_op->src[1]->value.i = (define_op->src[1]->value.i | 1);
	  cycles--;
	}			/* if */
    }				/* if */

  /* Finally add nop bundles to fill out the cycles */
  while (cycles)
    {
      if (cycles == 1)
	{
	  new_op = L_create_new_op (Lop_DEFINE);
	  new_op->proc_opc = TAHOEop_NON_INSTR;
	  new_op->dest[0] =
	    L_new_macro_operand (TAHOE_MAC_TEMPLATE,
				 L_CTYPE_LLONG, L_PTYPE_NULL);
	  new_op->src[0] = L_new_int_operand (MII, L_CTYPE_LLONG);	/* 0 */
	  /* Put in a stop bit at the end of the bundle */
	  new_op->src[1] = L_new_int_operand (1, L_CTYPE_LLONG);
	  L_insert_oper_before (to_cb, to_op_define_op, new_op);
	  cycles--;
	}			/* if */
      else
	{
	  new_op = L_create_new_op (Lop_DEFINE);
	  new_op->proc_opc = TAHOEop_NON_INSTR;
	  new_op->dest[0] =
	    L_new_macro_operand (TAHOE_MAC_TEMPLATE,
				 L_CTYPE_LLONG, L_PTYPE_NULL);
	  new_op->src[0] = L_new_int_operand (MISI, L_CTYPE_LLONG);	/* 0 */
	  /* Put two stop bits in the bundle */
	  new_op->src[1] = L_new_int_operand (3, L_CTYPE_LLONG);
	  L_insert_oper_before (to_cb, to_op_define_op, new_op);
	  cycles -= 2;
	}			/* else */

      new_op = L_create_new_op (Lop_NO_OP);
      new_op->proc_opc = TAHOEop_NOP_M;
      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
      L_insert_oper_before (to_cb, to_op_define_op, new_op);

      new_op = L_create_new_op (Lop_NO_OP);
      new_op->proc_opc = TAHOEop_NOP_I;
      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
      L_insert_oper_before (to_cb, to_op_define_op, new_op);

      new_op = L_create_new_op (Lop_NO_OP);
      new_op->proc_opc = TAHOEop_NOP_I;
      new_op->src[0] = L_new_int_operand (2, L_CTYPE_LLONG);
      L_insert_oper_before (to_cb, to_op_define_op, new_op);
    }				/* while */

  return;
}				/* L_insert_padding_bw */


static int
L_find_op_delta (L_Cb * from_cb, L_Oper * from_op,
		 L_Cb * to_cb, L_Oper * to_op)
{
  int delta = 0;
  int mask = 0;
  int can_count = 0;
  L_Oper *op = NULL;
  L_Oper *tl = NULL;

  if (from_cb->id == to_cb->id)
    {
      for (tl = from_op; !LT_is_template_op (tl); tl = tl->prev_op);
      can_count = 0;
      mask = 4;
      for (op = tl; op; op = op->next_op)
	{
	  if (op == from_op)
	    can_count = 1;
	  if (op == to_op)
	    break;

	  if (!LT_is_template_op (op))
	    {
	      if (can_count && (mask & tl->src[1]->value.i))
		delta++;
	      mask = mask >> 1;
	    }			/* if */
	  else
	    {
	      mask = 4;
	      tl = op;
	    }			/* else */
	}			/* for op */
      if (!op)
	{
	  mask = 4;
	  for (op = to_cb->first_op; op; op = op->next_op)
	    {
	      if (op == to_op)
		break;

	      if (!LT_is_template_op (op))
		{
		  if (mask & tl->src[1]->value.i)
		    delta++;
		  mask = mask >> 1;
		}		/* if */
	      else
		{
		  mask = 4;
		  tl = op;
		}		/* else */
	    }			/* for op */
	  if (!op)
	    L_punt ("L_find_op_delta: to_op not found\n");
	}			/* if */
    }				/* if */
  else
    {
      /* Conservative: from_op to next branch */
      for (tl = from_op; !LT_is_template_op (tl); tl = tl->prev_op);
      can_count = 0;
      mask = 4;
      for (op = from_op; op; op = op->next_op)
	{
	  if (op == from_op)
	    can_count = 1;
	  if (L_is_control_oper (op))
	    break;

	  if (!LT_is_template_op (op))
	    {
	      if (can_count && (mask & tl->src[1]->value.i))
		delta++;
	      mask = mask >> 1;
	    }			/* if */
	  else
	    {
	      mask = 4;
	      tl = op;
	    }			/* else */
	}			/* for op */

      /* cb start to to_op */
      mask = 4;
      for (op = to_cb->first_op; op; op = op->next_op)
	{
	  if (op == to_op)
	    break;

	  if (!LT_is_template_op (op))
	    {
	      if (mask & tl->src[1]->value.i)
		delta++;
	      mask = mask >> 1;
	    }			/* if */
	  else
	    {
	      mask = 4;
	      tl = op;
	    }			/* else */
	}			/* for op */
    }				/* else */

  return delta;
}				/* L_find_op_delta */

static int
L_check_for_dep_distance_bw (L_Func * fn, L_Oper * dest_op, L_Cb * dest_cb,
			     int (*test_fn) (L_Oper *),
			     int min_latency, char *msg, int detectonly)
{
  L_Oper *def_op = NULL;
  L_Cb *def_cb = NULL;
  Set defs = NULL;
  int *buffer;
  int num_defs, i, d;

  for (i = 0; i < L_max_src_operand; i++)
    {
      if (!dest_op->src[i])
	continue;
      if (!L_is_reg (dest_op->src[i]) && !L_is_macro (dest_op->src[i]))
	continue;

      defs = L_get_oper_RIN_defining_opers (dest_op, dest_op->src[i]);

      num_defs = Set_size (defs);
      if (num_defs <= 0)
	{
#if 0
	  printf ("L_check_for_dep_distance_bw: no defs op%d src%d\n",
		  dest_op->id, i);
#endif
	  continue;
	}			/* if */

      buffer = (int *) Lcode_malloc (sizeof (int) * num_defs);
      Set_2array (defs, buffer);
      for (d = 0; d < num_defs; d++)
	{
	  def_op = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, buffer[d]);
	  def_cb = L_oper_hash_tbl_find_cb (fn->oper_hash_tbl, def_op->id);

	  if (!def_op)
	    L_punt ("L_check_for_dep_distance_bw: oper not found\n");

	  if (test_fn (def_op))
	    {
	      int delta = 0;

	      delta = L_find_op_delta (def_cb, def_op, dest_cb, dest_op);

	      if (delta >= 0 && delta < min_latency)
		{
#ifdef DEBUG_PADDING
		  printf ("%s: cb%4d op%4d -> cb%4d op%4d :: %2d %0.1f\n",
			  msg,
			  def_cb->id, def_op->id,
			  dest_cb->id, dest_op->id, delta, dest_op->weight);
		  fflush (stdout);
#endif
		  if (!detectonly && dest_cb == def_cb)
		    {
		      L_insert_padding_bw (def_cb, def_op,
					   dest_cb, dest_op,
					   (min_latency - delta));

		      delta = L_find_op_delta (def_cb, def_op,
					       dest_cb, dest_op);

#ifdef DEBUG_PADDING
		      /* Make sure not over or under padded */
		      if (delta != min_latency)
			{
			  printf ("padding not correct: cb%4d op%4d -> "
				  "cb%4d op%4d :: %2d != %2d\n",
				  def_cb->id, def_op->id,
				  dest_cb->id, dest_op->id,
				  delta, min_latency);

			  DB_print_cb (dest_cb);
			  L_punt ("L_check_for_dep_distance_bw: error\n");
			}	/* if */
		      else
			{
			  printf ("padding ok\n");
			}	/* else */
#endif
		    }		/* if */
		}		/* if */
	    }			/* if */
	}			/* for d */
      Lcode_free (buffer);
      defs = Set_dispose (defs);
    }				/* for i */

  return 0;
}				/* L_check_for_dep_distance_bw */


static int
IEU_def_test (L_Oper * op)
{
  if (op->proc_opc == TAHOEop_SHL ||
      op->proc_opc == TAHOEop_SHR || op->proc_opc == TAHOEop_SHR_U)
    return 1;

  return 0;
}				/* IEU_def_test */

#if 0
static int
SPEC_def_test (L_Oper * op)
{
  if (L_EXTRACT_BIT_VAL (op->flags, L_OPER_MASK_PE))
    return 1;
  return 0;
}				/* SPEC_def_test */
#endif

int
OUTPUT_DEP_STALL_def_test (L_Oper * op)
{
  return (L_general_load_opcode (op));
}				/* OUTPUT_DEP_STALL_def_test */

void
L_check_for_special_dep_violation (L_Func * fn)
{
  L_Oper *op = NULL;
  L_Cb *cb = NULL;
  Set use_popc_set = NULL;

  PG_setup_pred_graph (fn);
  L_do_flow_analysis (fn, REACHING_DEFINITION);

  /* IEU */
  use_popc_set = Set_add (use_popc_set, TAHOEop_SHLADD);
  use_popc_set = Set_add (use_popc_set, TAHOEop_ADD);
  use_popc_set = Set_add (use_popc_set, TAHOEop_ADDS);
  use_popc_set = Set_add (use_popc_set, TAHOEop_ADDL);
  use_popc_set = Set_add (use_popc_set, TAHOEop_AND);
  use_popc_set = Set_add (use_popc_set, TAHOEop_ANDCM);
  use_popc_set = Set_add (use_popc_set, TAHOEop_OR);
  use_popc_set = Set_add (use_popc_set, TAHOEop_XOR);
  use_popc_set = Set_add (use_popc_set, TAHOEop_DEP);
  use_popc_set = Set_add (use_popc_set, TAHOEop_DEP_Z);
  use_popc_set = Set_add (use_popc_set, TAHOEop_EXTR);
  use_popc_set = Set_add (use_popc_set, TAHOEop_EXTR_U);
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  if (Set_in (use_popc_set, op->proc_opc) ||
	      L_general_store_opcode (op) || L_general_load_opcode (op))
	    {
	      L_check_for_dep_distance_bw (fn, op, cb, IEU_def_test, 4,
					   "IEU ", 0);
	    }			/* if */
	}			/* for op */
    }				/* for cb */
#if 0
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  L_check_for_dep_distance_bw (fn, op, cb, SPEC_def_test, 6, "SPEC ",
				       1);
	}			/* for op */
    }				/* for cb */
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (op = cb->first_op; op; op = op->next_op)
	{
	  if ((op->dest[0] && ((L_same_operand (op->dest[0], op->src[0])) ||
			       (L_same_operand (op->dest[0], op->src[1])))) ||
	      (op->dest[1] && ((L_same_operand (op->dest[1], op->src[0])) ||
			       (L_same_operand (op->dest[1], op->src[1])))))
	    L_check_for_dep_distance_bw (fn, op, cb,
					 OUTPUT_DEP_STALL_def_test, 3,
					 "OUTPUT_DEP_STALL", 1);
	}			/* for op */
    }				/* for cb */
#endif
}				/* L_check_for_special_dep_violation */
