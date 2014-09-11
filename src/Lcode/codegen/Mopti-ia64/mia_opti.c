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
 *  File:  m_opti.c
 *
 *  Description:
 *      Performs machine level code optimization:
 *	1) common subexpression elimination
 *	2) limited copy propogation (only R-R, R-M, M-R, M-M)
 *	3) dead code removal (unused operations, src1=dest)
 *
 *	This code is base-lined off the work developed by Scott Mahlke in
 *	l_basic_opti.c
 *
 *  Creation Date :  July 1991
 *
 *  Author:  Roger A. Bringmann
 *
 *  Revisions:
 *	Roger A. Bringmann, February 1993
 *	Modified to support new Lcode format.  Reduces memory requirements
 *	for a code generator.  Also, adds a more friendly interface for
 *	code generation.
 *
 * 	(C) Copyright 1991, Roger A. Bringmann
 * 	All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/
/* 09/17/02 REK Updating to use functions from libtahoeop instead of Tmdes. */

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include  "mia_internal.h"
#include <Lcode/l_disjvreg.h>

#undef TEST_GLOB_COPY_PROP
#undef TEST_GLOB_COMMON_SUB

#undef DEBUG_ADD_REGROUPING
#undef DEBUG_CONSTANT_FOLDING
#undef DEBUG_SXT_ELIM
#undef DEBUG_COPY_PROP1
#undef DEBUG_COPY_PROP2

#define Mopti_imax_num_iterations (4)
#define Mopti_omax_num_iterations (4)

/* prototypes */
static void Mopti_init ();
static void Mopti_deinit ();
static int M_global_code_optimization (L_Func * fn);
static int M_global_common_subexpression (L_Cb * cbA, L_Cb * cbB,
					  int move_flags);
static int M_global_copy_propagation (L_Cb * cbA, L_Cb * cbB);
static int M_local_add_regrouping (L_Cb * cb);
static int Mopti_global_dead_code_optimization (L_Func * fn);
static int M_local_redmov_removal (L_Cb * cb);

static int M_local_cmp_copy_propagation (L_Cb * cb);
static int M_local_cmp_rev_copy_propagation (L_Cb * cb);
static int M_global_cmp_copy_propagation (L_Cb * cbA, L_Cb * cbB);

int M_local_constant_folding (L_Cb * cb);

int M_local_copy_propagation (L_Cb * cb, int use_dem);
int M_local_rev_copy_propagation (L_Cb * cb, int use_dem);
int M_local_constant_propagation (L_Cb * cb);

/* Variable initialization */
static int Mopti_init_performed = 0;

/*
 *	Mopti Parameters
 */
int Mopti_print_stats = 0;

static int Mopti_do_local_opti = 1;

static int Mopti_do_local_common_subs = 1;
static int Mopti_do_local_copy_prop = 1;
static int Mopti_do_local_rev_copy_prop = 1;
static int Mopti_do_local_const_prop = 1;
static int Mopti_do_local_const_fold = 1;
static int Mopti_do_local_add_regrouping = 0;
static int Mopti_do_dead_code = 1;

static int Mopti_do_global_opti = 1;

static int Mopti_do_global_common_subs = 1;
static int Mopti_do_global_copy_prop = 1;

static int Mopti_do_coalescing = 1;

int Mopti_do_compare_height_reduction = 0;
int Mopti_constant_preloading = 0;
int Mopti_shift_add_merge = 0;
int Mopti_do_epilogue_merge = 0;

int Mopti2_redundant_memory_ops = 0;
int Mopti_debug_messages = 0;

void
L_read_parm_mopti (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "print_stats", &Mopti_print_stats);

  L_read_parm_b (ppi, "do_local_opti", &Mopti_do_local_opti);

  L_read_parm_b (ppi, "do_local_common_subs", &Mopti_do_local_common_subs);
  L_read_parm_b (ppi, "do_local_const_prop", &Mopti_do_local_const_prop);
  L_read_parm_b (ppi, "do_local_const_fold", &Mopti_do_local_const_fold);
  L_read_parm_b (ppi, "do_local_copy_prop", &Mopti_do_local_copy_prop);
  L_read_parm_b (ppi, "do_local_rev_copy_prop",
		 &Mopti_do_local_rev_copy_prop);
  L_read_parm_b (ppi, "do_local_add_regrouping",
		 &Mopti_do_local_add_regrouping);
  L_read_parm_b (ppi, "do_dead_code", &Mopti_do_dead_code);

  L_read_parm_b (ppi, "do_global_opti", &Mopti_do_local_opti);

  L_read_parm_b (ppi, "do_global_common_subs", &Mopti_do_global_common_subs);
  L_read_parm_b (ppi, "do_global_copy_prop", &Mopti_do_global_copy_prop);

  L_read_parm_b (ppi, "do_coalescing", &Mopti_do_coalescing);

  L_read_parm_b (ppi, "compare_height_reduction",
		 &Mopti_do_compare_height_reduction);
  L_read_parm_b (ppi, "constant_preloading", &Mopti_constant_preloading);
  L_read_parm_b (ppi, "shift_add_merge", &Mopti_shift_add_merge);
  L_read_parm_b (ppi, "epilogue_merge", &Mopti_do_epilogue_merge);
  L_read_parm_b (ppi, "mopti2_redundant_memory_ops",
		 &Mopti2_redundant_memory_ops);
  L_read_parm_b (ppi, "?debug_messages_mopti", &Mopti_debug_messages);
}


static void
Mopti_init ()
{
  /* This prevents the code from performing initialization again */
  Mopti_init_performed = 1;
  MOD ("Mopti initializing");
  L_alloc_danger_ext = L_create_alloc_pool ("L_Danger_Ext",
					    sizeof (struct L_Danger_Ext), 64);
}


static void
Mopti_deinit ()
{
  Mopti_init_performed = 0;
  MOD ("Mopti deinitializing");
  L_free_alloc_pool (L_alloc_danger_ext);
  L_alloc_danger_ext = NULL;
}

void
Mopti_perform_optimizations_tahoe (L_Func * fn)
{
  int j, k, change;
  int common_subs, copy_prop, rev_copy_prop, dead_code;
  int add_regrouping;
  int c1 = 0, c2 = 0, c4 = 0, c5 = 0;
  L_Cb *cb;

  if (!Mopti_init_performed)
    Mopti_init ();

  MOD ("START Mopti_perform_optimizations_tahoe()");

  if (Mopti_do_coalescing)
    {
      MOD ("Disjoint VREG renaming + coalescing");
      L_rename_coalesce_disjoint_virtual_registers (fn, NULL);
    }

  if (Mopti_do_compare_height_reduction)
    {
      MOD ("DOM analysis");
      L_do_flow_analysis (fn, DOMINATOR);
      L_loop_detection (fn, 0);
      MOD ("Mopti loop compare height reduction");
      Mopti_loop_compare_height_reduction (fn);
    }

  k = 0;
  common_subs = copy_prop = rev_copy_prop = dead_code = 0;
  add_regrouping = 0;

  do
    {
      change = 0;

      MOD ("Optimization iteration %d", k);

      MOD ("Partial dead code removal");

      dead_code += L_partial_dead_code_removal (fn);

      MOD ("Flow analysis: LV");

      L_do_flow_analysis (fn, LIVE_VARIABLE | SUPPRESS_PG);

      if (Mopti_do_local_opti)
	{
	  MOD ("Local optimizations");

	  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	    {
	      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
		  L_find_attr (cb->attr, "kernel"))
		continue;

	      j = 0;
	      do
		{
		  if (Mopti_do_local_common_subs)
		    {
		      /* Handle common sub-expression elimination */
		      c1 = L_local_common_subexpression (cb,
				   L_COMMON_SUB_MOVES_WITH_INT_CONSTANT |
				   L_COMMON_SUB_MOVES_WITH_FLT_CONSTANT |
#if 0
				   L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT |
#endif
				   L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT);
		      common_subs += c1;
		      change += c1;
		    }

		  if (Mopti_do_local_const_prop)
		    {
		      M_local_constant_propagation (cb);
		    }

		  if (Mopti_do_local_add_regrouping)
		    {
		      c4 = M_local_add_regrouping (cb);
		      add_regrouping += c4;
		      change += c4;
		    }

		  if (Mopti_do_local_rev_copy_prop)
		    {
		      c5 = M_local_rev_copy_propagation (cb, 1);
		      rev_copy_prop += c5;
		    }

		  if (Mopti_do_local_copy_prop)
		    {
		      c2 = M_local_copy_propagation (cb, 1);
		      copy_prop += c2;
		      change += c2;
		    }

		  if (Mopti_do_compare_height_reduction)
		    {
		      Mopti_compare_height_reduction (fn, cb, NULL, 0, NULL);
		    }
		  j++;
		}
	      while ((c1 + c2 + c4 != 0) && (j < Mopti_imax_num_iterations));
	    }
	}

      if (Mopti_do_global_opti)
	{
	  MOD ("Flow analysis: LV DOM AD AE");

	  L_do_flow_analysis (fn, LIVE_VARIABLE | DOMINATOR_CB |
			      AVAILABLE_DEFINITION | AVAILABLE_EXPRESSION);

	  MOD ("Global optimizations");

	  L_compute_danger_info (fn);
	  M_global_code_optimization (fn);
	  L_delete_all_danger_ext (fn);
	}

      k++;			/* iteration counter */
    }
  while (change && (k < Mopti_omax_num_iterations));

  if (Mopti_print_stats)
    {
      fprintf (stderr, "Local Optimizations:\n");
      fprintf (stderr, "   Common subexpressions  : %d\n", common_subs);
      fprintf (stderr, "   Copy propagations      : %d\n", copy_prop);
      fprintf (stderr, "   Add regrouping         : %d\n", add_regrouping);
      fprintf (stderr, "   Rev Copy propagations  : %d\n", rev_copy_prop);
      fprintf (stderr, "   Dead code removed      : %d\n", dead_code);
      fprintf (stderr, "  --------------------------------------\n");
    }

  MOD ("Dead block removal");

  L_delete_unreachable_blocks (fn);

  if (Mopti_do_global_opti)
    {
      MOD ("Flow analysis: LV DOM AD AE");

      L_do_flow_analysis (fn, LIVE_VARIABLE | DOMINATOR_CB |
			  AVAILABLE_DEFINITION | AVAILABLE_EXPRESSION);

      MOD ("Global optimizations");

      L_compute_danger_info (fn);
      M_global_code_optimization (fn);
      L_delete_all_danger_ext (fn);

      MOD ("Flow analysis: LV RD");

      L_do_flow_analysis (fn, LIVE_VARIABLE | REACHING_DEFINITION);

      MOD ("Global sxt elim");

      Mopti_global_sxt_elimination (fn);
    }

  if (Mopti_do_local_opti)
    {
      MOD ("Flow analysis: LV");

      L_do_flow_analysis (fn, LIVE_VARIABLE);

      MOD ("Local optimization");

      for (cb = fn->first_cb; cb; cb = cb->next_cb)
	{
	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	      L_find_attr (cb->attr, "kernel"))
	    continue;

	  if (Mopti_do_local_copy_prop)
	    M_local_copy_propagation (cb, 0);
	  L_local_dead_code_removal (cb);
	  M_local_redmov_removal (cb);
	}
    }

  Mopti_deinit ();

  MOD ("END Mopti_perform_optimizations_tahoe");
}


void
Mopti_phase2_optimizations (L_Func * fn)
{
  int cb_iterations, func_iterations, change;
  int common_subs, copy_prop, rev_copy_prop, dead_code;
  L_Cb *cb;

  if (!Mopti_init_performed)
    Mopti_init ();

  MOD ("START Mopti_phase2_optimizations()");

  func_iterations = 0;
  common_subs = copy_prop = rev_copy_prop = dead_code = 0;

  do
    {
      change = 0;

      MOD ("Optimization iteration %d", func_iterations);

      if (Mopti_do_global_opti)
	{
	  MOD ("Flow analysis LV DOM AD AE");

	  L_do_flow_analysis (fn, LIVE_VARIABLE | DOMINATOR_CB |
			      AVAILABLE_DEFINITION | AVAILABLE_EXPRESSION);

	  L_compute_danger_info (fn);

	  MOD ("Global code optimization");

	  M_global_code_optimization (fn);

	  L_delete_all_danger_ext (fn);
	}

      MOD ("Partial dead code removal");

      dead_code += L_partial_dead_code_removal (fn);

      MOD ("Flow analysis LV");

      L_do_flow_analysis (fn, LIVE_VARIABLE | SUPPRESS_PG);

      MOD ("Local optimizations");

      for (cb = fn->first_cb; cb; cb = cb->next_cb)
	{
	  int c1 = 0, c2, c3 = 0, c4;

	  if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	      L_find_attr (cb->attr, "kernel"))
	    continue;

	  cb_iterations = 0;

	  do
	    {
	      if (Mopti_do_local_common_subs)
		{
		  c1 = L_local_common_subexpression (cb,
						     L_COMMON_SUB_MOVES_WITH_INT_CONSTANT
						     |
						     L_COMMON_SUB_MOVES_WITH_FLT_CONSTANT
						     |
						     L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT
						     |
						     L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT);
		  common_subs += c1;
		  change += c1;
		}

	      if (Mopti_do_local_rev_copy_prop)
		{
		  c2 = M_local_rev_copy_propagation (cb, 1);
		  rev_copy_prop += c2;
		}

	      if (Mopti_do_local_copy_prop)
		{
		  c3 = M_local_copy_propagation (cb, 1);
		  copy_prop += c3;
		  change += c3;
		}

	      if (1)
		{
		  c4 = M_local_redmov_removal (cb);
		  change += c4;
		}

	      cb_iterations++;	/* iteration counter */
	    }
	  while ((c1 || c3) && (cb_iterations < Mopti_imax_num_iterations));

	  if (Mopti_do_local_const_fold)
	    change += M_local_constant_folding (cb);
	  if (Mopti_do_local_const_prop)
	    change += M_local_constant_propagation (cb);
	}
      func_iterations++;	/* iteration counter */
    }
  while (change && (func_iterations < Mopti_omax_num_iterations));

  MOD ("Partial dead code removal");

  dead_code += L_partial_dead_code_removal (fn);

  if (Mopti_debug_messages || Mopti_print_stats)
    {
      fprintf (stderr, "Mopti_phase2_optimizations(%s)\n", fn->name);
      fprintf (stderr, "---------------------------------------\n");
      fprintf (stderr, " LOCAL\n");
      fprintf (stderr, "   Common subexpressions    : %6d\n", common_subs);
      fprintf (stderr, "   Copy propagations        : %6d\n", copy_prop);
      fprintf (stderr, "   Rev copy propagations    : %6d\n", rev_copy_prop);
      fprintf (stderr, "   Dead code removed        : %6d\n", dead_code);
      fprintf (stderr, "---------------------------------------\n");
    }

  Mopti_deinit ();
  MOD ("END Mopti_phase2_optimizations()");
  return;
}


static int
M_global_code_optimization (L_Func * fn)
{
  L_Cb *cb1, *cb2;
  int c1, c2, global_common_subs, global_copy_prop;

  global_copy_prop = global_common_subs = 0;

  for (cb1 = fn->first_cb; cb1 != NULL; cb1 = cb1->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb1->flags, L_CB_BOUNDARY))
	continue;

      if (L_EXTRACT_BIT_VAL (cb1->flags, L_CB_SOFTPIPE) &&
	  L_find_attr (cb1->attr, "kernel"))
	continue;

      for (cb2 = fn->first_cb; cb2 != NULL; cb2 = cb2->next_cb)
	{
	  if (cb1 == cb2)
	    continue;

	  if (L_EXTRACT_BIT_VAL (cb2->flags, L_CB_SOFTPIPE) &&
	      L_find_attr (cb2->attr, "kernel"))
	    continue;

	  if (!L_in_cb_DOM_set (cb2, cb1->id))
	    continue;

	  if (Mopti_do_global_common_subs)
	    {
	      c1 = M_global_common_subexpression (cb1, cb2,
						  L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT
						  |
						  L_COMMON_SUB_MOVES_WITH_INT_CONSTANT
						  |
						  L_COMMON_SUB_MOVES_WITH_FLT_CONSTANT);
	      global_common_subs += c1;
	    }

	  if (Mopti_do_global_copy_prop)
	    {
	      c2 = M_global_copy_propagation (cb1, cb2);
	      global_copy_prop += c2;
	    }
	}
    }

  if (Mopti_print_stats)
    {
      fprintf (stderr, "Global Optimizations:\n");
      fprintf (stderr, "   Common subexpressions: %d\n", global_common_subs);
      fprintf (stderr, "   Copy propagations    : %d\n", global_copy_prop);
      fprintf (stderr, "  --------------------------------------\n");
    }
  return 0;
}


static int
M_local_add_regrouping (L_Cb * cb)
{
  int i, index, change, macro_flagA, macro_flagC;
  int match0, match1, match2, made_change;
  int OK, a_b_changed;
  L_Oper *opA, *opB, *opC, *opD;
  L_Oper *temp_oper1, *temp_oper2;

  Set changed_opers = NULL;

  /* Looking for pairs of opers of the form

     add  x1 = a,z1
     add  y1 = x1,b

     add  x2 = a,z2
     add  y2 = x2,b

     replace with 

     add  x1 = a,b
     add  y1 = x1,z1

     add  x2 = a,b
     add  y2 = x2,z2

     so that local_common_sub can remove an instruction */


  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_add_opcode (opA))
	continue;

      if (Set_in (changed_opers, opA->id))
	continue;

      if (!(L_is_register (opA->src[0]) && L_is_register (opA->src[1])))
	continue;

      if (L_has_unsafe_macro_dest_operand (opA))
	continue;

      macro_flagA = L_has_fragile_macro_operand (opA);

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!L_add_opcode (opB))
	    continue;

	  if (Set_in (changed_opers, opB->id))
	    continue;

	  if (!(L_is_register (opB->src[0]) && L_is_register (opB->src[1])))
	    continue;

	  if (!(L_is_src_operand (opA->dest[0], opB)))
	    continue;

	  /* Dest of opA must only be used in opB */
	  if (L_in_cb_OUT_set (cb, opA->dest[0]))
	    continue;

	  if (!(L_no_other_use_in_cb_after (cb, opA->dest[0], opA, opB)))
	    continue;

	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;

	  if (!L_all_dest_operand_no_defs_between (opA, opA, opB))
	    continue;

	  /* Could be more aggressive here - JEP 12/97 */
	  if (!L_all_src_operand_no_defs_between (opB, opA, opB))
	    continue;

	  if (!L_no_danger (macro_flagA, 0 /* load_flag */ ,
			    0 /* store_flag */ , opA, opB))
	    break;

	  /* Create dummy oper holding three add sources */

	  temp_oper1 = L_create_new_op (Lop_NO_OP);
	  temp_oper1->src[0] = L_copy_operand (opA->src[0]);
	  temp_oper1->src[1] = L_copy_operand (opA->src[1]);
	  if (L_same_operand (opA->dest[0], opB->src[0]))
	    temp_oper1->src[2] = L_copy_operand (opB->src[1]);
	  else if (L_same_operand (opA->dest[0], opB->src[1]))
	    temp_oper1->src[2] = L_copy_operand (opB->src[0]);
	  else
	    L_punt ("Something wrong in add_regrouping 1");

	  a_b_changed = 0;
	  made_change = 0;
	  for (opC = opA->next_op; opC != NULL; opC = opC->next_op)
	    {
	      if (!L_add_opcode (opC))
		continue;

	      if (Set_in (changed_opers, opC->id))
		continue;

	      if (opC->id == opB->id)
		continue;

	      if (!
		  (L_is_register (opC->src[0])
		   && L_is_register (opC->src[1])))
		continue;

	      if (L_same_src_operands (opA, opC))
		continue;

	      if (L_has_unsafe_macro_dest_operand (opC))
		continue;

	      macro_flagC = L_has_fragile_macro_operand (opC);

	      for (opD = opC->next_op; opD != NULL; opD = opD->next_op)
		{

		  if (!L_add_opcode (opD))
		    continue;

		  if (!
		      (L_is_register (opD->src[0])
		       && L_is_register (opD->src[1])))
		    continue;

		  if (!(L_is_src_operand (opC->dest[0], opD)))
		    continue;

		  if (Set_in (changed_opers, opD->id))
		    continue;

		  /* Dest of opC must only be used in opD */
		  if (L_in_cb_OUT_set (cb, opA->dest[0]))
		    continue;

		  if (!
		      (L_no_other_use_in_cb_after
		       (cb, opC->dest[0], opC, opD)))
		    continue;

		  if (!PG_superset_predicate_ops (opC, opD))
		    continue;

		  if (!L_all_dest_operand_no_defs_between (opC, opC, opD))
		    continue;

		  /* Could be more aggressive here - JEP 12/97 */
		  if (!L_all_src_operand_no_defs_between (opD, opC, opD))
		    continue;

		  if (!L_no_danger (macro_flagC, 0, 0, opC, opD))
		    break;

		  /* Create dummy oper holding three add sources */

		  temp_oper2 = L_create_new_op (Lop_NO_OP);
		  temp_oper2->src[0] = L_copy_operand (opC->src[0]);
		  temp_oper2->src[1] = L_copy_operand (opC->src[1]);
		  if (L_same_operand (opC->dest[0], opD->src[0]))
		    temp_oper2->src[2] = L_copy_operand (opD->src[1]);
		  else if (L_same_operand (opC->dest[0], opD->src[1]))
		    temp_oper2->src[2] = L_copy_operand (opD->src[0]);
		  else
		    L_punt ("Something wrong in add_regrouping 3");

		  if (a_b_changed)
		    {
		      /* Oper A and B sources have been changed, don't want
		         to change them again */
		      match0 = L_is_src_operand (opA->src[0], temp_oper2);
		      match1 = L_is_src_operand (opA->src[1], temp_oper2);
		      match2 = L_is_src_operand (opB->src[1], temp_oper2);

		      /* for match, C and D sources must share A sources 
		         but cannot share B source */
		      OK = match0 && match1 && !match2;
		    }
		  else
		    {
		      match0 =
			L_is_src_operand (temp_oper1->src[0], temp_oper2);
		      match1 =
			L_is_src_operand (temp_oper1->src[1], temp_oper2);
		      match2 =
			L_is_src_operand (temp_oper1->src[2], temp_oper2);

		      /* any 2 src operand must be shared */
		      OK = (match0 + match1 + match2 == 2);
		    }

		  if (OK)
		    {
		      /* Going to make the change */
#ifdef DEBUG_ADD_REGROUPING

		      Set_print (stderr, "previous changes:", changed_opers);
		      fprintf (stderr, "----- Found matching pair ------\n");
		      L_print_oper (stderr, opA);
		      L_print_oper (stderr, opB);
		      fprintf (stderr, "             -----\n");
		      L_print_oper (stderr, opC);
		      L_print_oper (stderr, opD);
		      fprintf (stderr,
			       "---------  changed to  ------------\n");
#endif

		      if (a_b_changed)
			{
			  for (i = 0; i < 2; i++)
			    {
			      L_delete_operand (opC->src[i]);
			      L_delete_operand (opD->src[i]);
			    }

			  /* use the same operands as oper A */
			  opC->src[0] = L_copy_operand (opA->src[0]);
			  opC->src[1] = L_copy_operand (opA->src[1]);

			  /* opD uses dest of oper C */
			  opD->src[1] = L_copy_operand (opC->dest[0]);

			  /* Find the source which isn't included in A/B */
			  if (!L_is_src_operand
			      (temp_oper2->src[0], temp_oper1))
			    {
			      opD->src[0] =
				L_copy_operand (temp_oper2->src[0]);
			    }
			  else
			    if (!L_is_src_operand
				(temp_oper2->src[1], temp_oper1))
			    {
			      opD->src[0] =
				L_copy_operand (temp_oper2->src[1]);
			    }
			  else
			    if (!L_is_src_operand
				(temp_oper2->src[2], temp_oper1))
			    {
			      opD->src[0] =
				L_copy_operand (temp_oper2->src[2]);
			    }
			  else
			    {
			      L_punt ("Something wrong in add_regrouping 4");
			    }

			}

		      else
			{	/* Haven't yet changed A,B,C, or D */

			  for (i = 0; i < 2; i++)
			    {
			      L_delete_operand (opA->src[i]);
			      L_delete_operand (opB->src[i]);
			      L_delete_operand (opC->src[i]);
			      L_delete_operand (opD->src[i]);
			    }

			  /* opA uses dest of new oper B */
			  opB->src[1] = L_copy_operand (opA->dest[0]);
			  /* opD uses dest of new oper C */
			  opD->src[1] = L_copy_operand (opC->dest[0]);

			  index = 0;
			  if (match0)
			    {
			      opA->src[index] =
				L_copy_operand (temp_oper1->src[0]);
			      opC->src[index] =
				L_copy_operand (temp_oper1->src[0]);
			      index++;
			    }
			  else
			    {
			      opB->src[0] =
				L_copy_operand (temp_oper1->src[0]);
			    }
			  if (match1)
			    {
			      opA->src[index] =
				L_copy_operand (temp_oper1->src[1]);
			      opC->src[index] =
				L_copy_operand (temp_oper1->src[1]);
			      index++;
			    }
			  else
			    {
			      opB->src[0] =
				L_copy_operand (temp_oper1->src[1]);
			    }
			  if (match2)
			    {
			      opA->src[index] =
				L_copy_operand (temp_oper1->src[2]);
			      opC->src[index] =
				L_copy_operand (temp_oper1->src[2]);
			      index++;
			    }
			  else
			    {
			      opB->src[0] =
				L_copy_operand (temp_oper1->src[2]);
			    }

			  if (!L_is_src_operand
			      (temp_oper2->src[0], temp_oper1))
			    {
			      opD->src[0] =
				L_copy_operand (temp_oper2->src[0]);
			    }
			  else
			    if (!L_is_src_operand
				(temp_oper2->src[1], temp_oper1))
			    {
			      opD->src[0] =
				L_copy_operand (temp_oper2->src[1]);
			    }
			  else
			    if (!L_is_src_operand
				(temp_oper2->src[2], temp_oper1))
			    {
			      opD->src[0] =
				L_copy_operand (temp_oper2->src[2]);
			    }
			  else
			    {
			      L_punt ("Something wrong in add_regrouping 3");
			    }

			  changed_opers = Set_add (changed_opers, opA->id);
			  changed_opers = Set_add (changed_opers, opB->id);
			  a_b_changed = 1;

			}

		      changed_opers = Set_add (changed_opers, opC->id);
		      changed_opers = Set_add (changed_opers, opD->id);
		      change++;
		      made_change = 1;

#ifdef DEBUG_ADD_REGROUPING
		      L_print_oper (stderr, opA);
		      L_print_oper (stderr, opB);
		      fprintf (stderr, "             -----\n");
		      L_print_oper (stderr, opC);
		      L_print_oper (stderr, opD);
		      fprintf (stderr,
			       "------------------- ------------\n\n\n");
#endif


		    }

		  /* Delete the temporary oper */
		  L_insert_oper_before (cb, opC, temp_oper2);
		  L_delete_oper (cb, temp_oper2);

		  if (made_change)
		    {
		      break;
		    }
		}		/* End of D loop */
	    }			/* End of opC loop */

	  /* Now look for another opA/opB pair */
	  L_insert_oper_before (cb, opA, temp_oper1);
	  L_delete_oper (cb, temp_oper1);
	  break;
	}
    }

  Set_dispose (changed_opers);
  return (change);
}


static int
M_global_common_subexpression (L_Cb * cbA, L_Cb * cbB, int move_flags)
{
  int i, change, new_opc;
  L_Oper *opA, *opB, *new_op;
  int macro_flag;

  change = 0;

  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!(L_general_arithmetic_opcode (opA) || L_general_move_opcode (opA)))
	continue;
      for (i = 0; i < L_max_dest_operand; i++)
	if (opA->dest[i] && L_is_ctype_predicate (opA->dest[i]))
	  break;
      if (i < L_max_dest_operand)
	continue;
      if (!L_different_src_and_dest_operands (opA))
	continue;

      macro_flag = L_has_fragile_macro_operand (opA);

      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  /*
	   *  match pattern
	   */

	  if (!L_compatible_opcodes (opA, opB))
	    continue;
	  if (!L_same_src_operands (opA, opB))
	    continue;

	  if (!L_different_src_and_dest_operands (opB))
	    continue;

	  /* Don't want the register/macro that will
	     carry the value from opA to opB to be unsafe. MCM */
	  if (L_has_unsafe_macro_dest_operand (opA))
	    continue;

	  if (L_has_unsafe_macro_dest_operand (opB))
	    continue;

	  if (!L_all_src_operand_global_same_def_reachs (opA, cbA, opA,
							 cbB, opB))
	    continue;

	  if (!L_all_dest_operand_global_no_defs_between (opA, cbA, opA,
							  cbB, opB))
	    continue;

	  if (!L_global_no_danger (macro_flag, 0, 0, cbA, opA, cbB, opB))
	    break;

	  /*
	   *  replace pattern
	   */
	  /* if opA and opB identical, just delete opB */
	  if (L_same_dest_operands (opA, opB))
	    {
	      L_nullify_operation (opB);
	      /* make flow analysis conservative, so dont have to redo it!! */

	      /*
	       * Don't need to do this for Exit boundary cb's
	       */
	      if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
		L_remove_from_all_EIN_set (opB);
	    }
	  /* move opcode, only optimize if move_flag set */
	  else if (L_general_move_opcode (opA))
	    {
	      if (move_flags == 0)
		continue;
	      if (L_is_int_constant (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_INT_CONSTANT))
		    continue;
		}
	      else if (L_is_flt_constant (opA->src[0]) ||
		       L_is_dbl_constant (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_FLT_CONSTANT))
		    continue;
		}
	      else if (L_is_label (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_LABEL_CONSTANT))
		    continue;
		}
	      else if (L_is_string (opA->src[0]))
		{
		  if (!L_EXTRACT_BIT_VAL (move_flags,
					  L_COMMON_SUB_MOVES_WITH_STRING_CONSTANT))
		    continue;
		}
	      else
		{
		  continue;
		}

	      L_delete_operand (opB->src[0]);
	      opB->src[0] = L_copy_operand (opA->dest[0]);
	      opB->proc_opc = TAHOEop_MOV_GR;

	      /* make flow analysis conservative, so dont have to redo it!! */
	      /*
	       * Don't need to do this for Exit boundary cb's
	       */
	      if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
		L_remove_from_all_EIN_set (opB);
	    }
	  /* arithmetic op: if only 1 dest, just reuse opB for move op */
	  else if (L_num_dest_operand (opA) == 1)
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (opA->dest[i] != NULL)
		    break;
		}

	      if (i == L_max_dest_operand)
		L_punt ("M_global_common_subexpression: "
			"all opA dest are NULL");
	      L_convert_to_move (opB, L_copy_operand (opB->dest[i]),
				 L_copy_operand (opA->dest[i]));

	      /* make flow analysis conservative, so dont have to redo it!! */
	      /*
	       * Don't need to do this for Exit boundary cb's
	       */
	      if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
		L_remove_from_all_EIN_set (opB);
	    }
	  else
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (!opA->dest[i] && !opB->dest[i])
		    continue;
		  if (!opA->dest[i] || !opB->dest[i])
		    L_punt ("L_common_subexpression; illegal op");
		  new_opc = L_move_from_ctype (opB->dest[i]->ctype);
		  new_op = L_create_new_op (new_opc);
		  L_insert_oper_after (cbB, opB, new_op);
		  new_op->dest[0] = L_copy_operand (opB->dest[i]);
		  new_op->src[0] = L_copy_operand (opA->dest[i]);
		}

	      L_nullify_operation (opB);
	      MOD ("Flow analysis AD AE (due to global cse)");
	      L_do_flow_analysis (L_fn, AVAILABLE_DEFINITION |
				  AVAILABLE_EXPRESSION);
	    }
#ifdef TEST_GLOB_COMMON_SUB
	  fprintf (stderr, "-> Global common sub: "
		   "op%d (cb %d) -> op%d (cb %d) : %lf\n",
		   opA->id, cbA->id, opB->id, cbB->id, cbB->weight);
	  if (cbA->region != cbB->region)
	    Lopti_inter_region_global_common_sub_elim_wgt += cbB->weight;
#endif
	  change += 1;
	}
    }
  return change;
}


static int
M_local_redmov_removal (L_Cb * cb)
{
  int change = 0;
  L_Oper *op, *next_op;

  for (op = cb->first_op; op != NULL; op = next_op)
    {
      next_op = op->next_op;

      if (!L_move_opcode (op))
	continue;
      if (op->proc_opc == TAHOEop_MOVL)
	continue;
      if (!L_is_variable (op->src[0]))
	continue;
      if (L_has_unsafe_macro_operand (op))
	continue;

      if (L_same_operand (op->dest[0], op->src[0]))
	{
	  L_delete_oper (cb, op);
	  change++;
	}
    }

  return change;
}


static int
M_global_copy_propagation (L_Cb * cbA, L_Cb * cbB)
{
  int i, change = 0;
  L_Oper *opA, *opB, *new_oper;

  for (opA = cbA->first_op; opA != NULL; opA = opA->next_op)
    {
      if (!L_move_opcode (opA))
	continue;
      if (opA->proc_opc == TAHOEop_MOVL)
	continue;
      if (!L_is_variable (opA->src[0]))
	continue;
      if (L_has_unsafe_macro_operand (opA))
	continue;

      for (opB = cbB->first_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag;
	  /*
	   *  match pattern
	   */
	  if (!L_is_src_operand (opA->dest[0], opB))
	    continue;
	  if (!L_can_change_src_operand (opB, opA->dest[0]))
	    continue;
	  if (!L_global_no_defs_between (opA->dest[0], cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_same_def_reachs (opA->src[0], cbA, opA, cbB, opB))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));

	  if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
	    {
	      /* Conventional global copy propagation */
	      if (!L_global_no_danger (macro_flag, 0, 0, cbA, opA, cbB, opB))
		break;

	      /*
	       *  replace pattern
	       */
	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (L_same_operand (opA->dest[0], opB->src[i]))
		    {
#ifdef TEST_GLOB_COPY_PROP
		      fprintf (stderr,
			       "-> Global copy prop: "
			       "op%d (cb%d) -> op%d src%d (cb%d) : %lf\n",
			       opA->id, cbA->id, opB->id, i, cbB->id,
			       cbB->weight);
		      if (cbA->region != cbB->region)
			Lopti_inter_region_global_copy_prop_wgt +=
			  cbB->weight;
#endif
		      L_delete_operand (opB->src[i]);
		      opB->src[i] = L_copy_operand (opA->src[0]);
		      change += 1;
		    }
		}
	      /* make flow analysis conservative, so dont have to redo it!! */
	      L_remove_from_all_EIN_set (opB);
	    }
	  else
	    {
	      if (!L_global_no_danger_to_boundary (macro_flag, 0, 0,
						   cbA, opA, cbB, opB))
		break;

	      if (opB->opc == Lop_DEFINE)
		{
		  /* 
		   * The oper in CbB is a liveness boundary oper, so the
		   * operand cannot be replace.  Let's just copy the move
		   * operand instead and it will be pushed outside 
		   * the region. :)
		   */
		  int safe = 1;
		  for (i = 0; i < L_max_dest_operand; i++)
		    {
		      if (opA->dest[i] == NULL)
			continue;
		      if (!L_is_register (opA->dest[i]))
			{
			  safe = 0;
			  break;
			}
		    }
		  if (safe == 1)
		    {
		      new_oper = L_copy_operation (opA);
		      L_insert_oper_before (cbB,
					    (L_Oper *)
					    L_region_boundary_insert_point
					    (cbB), new_oper);
		      change += 1;
#ifdef TEST_GLOB_COPY_PROP
		      fprintf (stderr,
			       "Global copy prop (region): "
			       "op%d (cb%d) -> op%d src%d (cb%d) : %lf\n",
			       opA->id, cbA->id, opB->id, i, cbB->id,
			       cbB->weight);
#endif
		    }
		}
	      else
		{
		  /*
		   *  replace the oper in the boundary cb normally.
		   */
		  for (i = 0; i < L_max_src_operand; i++)
		    {
		      if (L_same_operand (opA->dest[0], opB->src[i]))
			{
#ifdef TEST_GLOB_COPY_PROP
			  fprintf (stderr,
				   "Global copy prop (region): "
				   "op%d (cb%d) -> op%d src%d (cb%d) : %lf\n",
				   opA->id, cbA->id, opB->id, i, cbB->id,
				   cbB->weight);
#endif
			  L_delete_operand (opB->src[i]);
			  opB->src[i] = L_copy_operand (opA->src[0]);
			  change += 1;
			}
		    }
		}
	    }
	}
    }
  return change;
}


/* Modified from Lopti so that it does not operate on
   software pipelined loops. */

#define MAX_NUM_GLOB_ITERATION 10

static int
Mopti_global_dead_code_optimization (L_Func * fn)
{
  int k, change, opti_applied;
  L_Cb *cb;

  if (Lopti_do_global_opti == 0)
    return (0);

  opti_applied = 0;

  if (Lopti_debug_global_opti)
    {
      fprintf (stderr, "\n");
      fprintf (stderr, "> ENTER global dead code removal (fn %s)\n",
	       fn->name);
      fprintf (stderr, "\n");
    }

  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	  L_find_attr (cb->attr, "kernel"))
	continue;

      for (k = 0; k < MAX_NUM_GLOB_ITERATION; k++)
	{
	  int c1 = 0;

	  if (Lopti_do_global_dead_code_rem)
	    {
	      c1 = L_global_dead_code_removal (cb);
	      Lopti_cnt_global_dead_code_rem += c1;
	    }

	  change = c1;
	  Lopti_cnt_global_opti += change;
	  if (change != 0)
	    opti_applied = 1;
	  if (change == 0)
	    break;
	}
    }

  return (opti_applied);
}


#define MAX_RD 16


static int
Mopti_redundant_ext_rev_set (L_Func * fn, L_Oper * oper, Set rd_set)
{
  int i, rd_cnt, *rd_id;
  L_Oper *prod_oper;

  if (!(rd_cnt = Set_size (rd_set)))
    return 0;

  rd_id = alloca (rd_cnt * sizeof (int));

  Set_2array (rd_set, rd_id);

  for (i = 0; i < rd_cnt; i++)
    {
      prod_oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl, rd_id[i]);

      if (!L_redundant_extension_rev (prod_oper, oper))
	break;
    }

  return (i == rd_cnt);
}

int
Mopti_global_sxt_elimination (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *prod_oper, *cons_oper;

  int change = 0;
  int size, cond_size, match;

  int i, rd_cnt, rd_id[MAX_RD];
  Set rd_set;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      for (oper = cb->last_op; oper; oper = oper->prev_op)
	{
	  /* Pattern 0: Using RD, find sign / zero extends of
	   * constants with multiple "extend-safe" reaching defs
	   */

	  if (!L_sign_or_zero_extend_opcode (oper))
	    continue;

	  rd_set = L_get_oper_RIN_defining_opers (oper, oper->src[0]);

	  if (Mopti_redundant_ext_rev_set (fn, oper, rd_set))
	    {
	      L_change_opcode (oper, Lop_MOV);
	      Set_dispose (rd_set);
	      change++;
	      continue;
	    }			/* if */

	  /*
	   *    match pattern 1
	   *     Look for a load of a char or a short
	   *     that undergoes sign extension and is compared only
	   *     to a postive constant that is less than the sxt size.
	   *     Eliminate the sxt.
	   */

	  rd_cnt = (Set_size (rd_set) <= MAX_RD) ?
	    Set_2array (rd_set, rd_id) : 0;

	  Set_dispose (rd_set);

	  if (!rd_cnt)
	    continue;

	  if (oper->proc_opc == TAHOEop_SXT1)
	    size = 1;
	  else if (oper->proc_opc == TAHOEop_SXT2)
	    size = 2;
	  else if (oper->proc_opc == TAHOEop_SXT4)
	    size = 4;
	  else
	    continue;

	  for (i = 0; i < rd_cnt; i++)
	    {
	      prod_oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						     rd_id[i]);

	      switch (prod_oper->proc_opc)
		{
		case TAHOEop_LD1:
		  if (size == 1)
		    continue;
		  break;
		case TAHOEop_LD2:
		  if (size == 2)
		    continue;
		  break;
		case TAHOEop_LD4:
		  if (size == 4)
		    continue;
		  break;
		default:
		  break;
		}		/* switch */
	      break;
	    }			/* for i */

	  if (i < rd_cnt)
	    continue;

	  rd_set = L_get_oper_ROUT_using_opers (oper, oper->dest[0]);

	  rd_cnt = (Set_size (rd_set) <= MAX_RD) ?
	    Set_2array (rd_set, rd_id) : 0;

	  Set_dispose (rd_set);

	  if (!rd_cnt)
	    continue;

	  /* Check to see if the consumers are subword compares of r0
	     or a constant between 0 and 127 or 0 and 2^15 - 1 or 0
	     and 2^31-1. */

	  cond_size = 0;
	  match = 0;
	  for (i = 0; i < rd_cnt; i++)
	    {
	      cons_oper = L_oper_hash_tbl_find_oper (fn->oper_hash_tbl,
						     rd_id[i]);

	      if (!L_int_pred_comparison_opcode (cons_oper))
		break;

	      if (!L_gen_eq_cmp_opcode (cons_oper) &&
		  !L_gen_ne_cmp_opcode (cons_oper))
		break;

	      if (cond_size == 0)
		cond_size = L_get_compare_ctype (cons_oper);
	      else if (cond_size != L_get_compare_ctype (cons_oper))
		break;

	      if (!LT_is_R0_operand (cons_oper->src[0]))
		{
		  if (size == 1)
		    {
		      if (!L_is_int_constant (cons_oper->src[0]) ||
			  cons_oper->src[0]->value.i < 0 ||
			  cons_oper->src[0]->value.i > 0x7F)
			break;
		    }		/* if */
		  else if (size == 2)
		    {
		      if (!L_is_int_constant (cons_oper->src[0]) ||
			  cons_oper->src[0]->value.i < 0 ||
			  cons_oper->src[0]->value.i > 0x7FFF)
			break;
		    }		/* else if */
		  else if (size == 4)
		    {
		      if (!L_is_int_constant (cons_oper->src[0]) ||
			  cons_oper->src[0]->value.i < 0 ||
			  cons_oper->src[0]->value.i > 0x7FFFFFFF)
			break;
		    }		/* else if */
		  else
		    {
		      break;
		    }		/* else */
		}		/* if */
	       match = 1;
	    }			/* for i */

	  if (i != rd_cnt)
	    {
#if 0
	      if (match)
		{
		  /* Not all consumers matched, but at least one did.
		   * Make a partial fix.  This can increase register
		   * pressure.
		   */
		  fprintf (stderr, "GSE missed opportunity\n");
		}
#endif
	    }
	  else
	    {
	      L_change_opcode (oper, Lop_MOV);
	  
#ifdef DEBUG_SXT_ELIM
	  fprintf (stderr, "Mopti_global_sxt_elimination 1: "
		   "applied to op %d [size %d].\n", oper->id, size);
#endif

	      change++;
	    }
	}			/* for oper */
    }				/* for cb */

#ifdef DEBUG_SXT_ELIM
  fprintf (stderr, ">> SXT_ELIM: %d\n", change);
#endif

  return change;
}				/* Mopti_global_sxt_elimination */


static int
M_global_cmp_copy_propagation (L_Cb * cbA, L_Cb * cbB)
{
  int change;
  L_Oper *opA, *opB;

  change = 0;
  for (opA = cbA->first_op; opA; opA = opA->next_op)
    {
      if (!L_int_pred_comparison_opcode (opA) ||
	  (opA->com[1] != Lcmp_COM_EQ) ||
	  !opA->pred[0] ||
	  !L_is_macro (opA->src[0]) ||
	  (opA->src[0]->value.mac != TAHOE_MAC_ZERO) ||
	  !L_is_macro (opA->src[1]) ||
	  (opA->src[1]->value.mac != TAHOE_MAC_ZERO) ||
	  (opA->dest[0]->ptype != L_PTYPE_UNCOND_T))
	continue;

      for (opB = cbB->first_op; opB; opB = opB->next_op)
	{
	  if (!opB->pred[0])
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->pred[0]))
	    continue;

	  if (!L_global_no_defs_between (opA->dest[0], cbA, opA, cbB, opB))
	    continue;
	  if (!L_global_same_def_reachs (opA->pred[0], cbA, opA, cbB, opB))
	    continue;

	  if (!L_EXTRACT_BIT_VAL (cbB->flags, L_CB_EXIT_BOUNDARY))
	    {
	      /*
	       *  replace pattern
	       */
#define TEST_GLOB_COPY_PROP
#ifdef TEST_GLOB_COPY_PROP
	      printf ("Global CMP copy prop: op%d (cb%d) -> op%d (cb%d)\n",
		      opA->id, cbA->id, opB->id, cbB->id);

#endif
	      L_delete_operand (opB->pred[0]);
	      opB->pred[0] = L_copy_operand (opA->pred[0]);
	      change += 1;

	      /* make flow analysis conservative, so dont have to redo it!! */
	      L_remove_from_all_EIN_set (opB);
	    }
	}
    }
  return change;
}


static int
M_local_cmp_copy_propagation (L_Cb * cb)
{
  int change;
  L_Oper *opA, *opB;
  change = 0;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       *        match pattern
       */

      if (!L_int_pred_comparison_opcode (opA) ||
	  (opA->com[1] != Lcmp_COM_EQ) ||
	  !opA->pred[0] ||
	  !L_is_macro (opA->src[0]) ||
	  (opA->src[0]->value.mac != TAHOE_MAC_ZERO) ||
	  !L_is_macro (opA->src[1]) ||
	  (opA->src[1]->value.mac != TAHOE_MAC_ZERO) ||
	  (opA->dest[0]->ptype != L_PTYPE_UNCOND_T))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  /*
	   *    match pattern
	   */
	  if (!opB->pred[0])
	    continue;
	  if (!L_same_operand (opA->dest[0], opB->pred[0]))
	    continue;

	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    break;
	  if (!(L_same_def_reachs (opA->pred[0], opA, opB)))
	    continue;

	  /*
	   *    replace pattern.
	   */
#ifdef DEBUG_COPY_PROP1
	  printf ("Local CMP copy prop: op%d -> op%d, (cb %d)\n",
		  opA->id, opB->id, cb->id);
#endif

	  L_delete_operand (opB->pred[0]);
	  opB->pred[0] = L_copy_operand (opA->pred[0]);
	  change += 1;
	}
    }

  return change;
}


static int
M_local_cmp_rev_copy_propagation (L_Cb * cb)
{
  int i, change, redef, redo_flow, redef_w, redef_z, bad_redef;
  int not_uncond;
  L_Oper *opA, *opB, *ptr, *new_op;
  change = 0;
  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      if (L_is_opcode (Lop_DEFINE, opA))
	continue;
      for (opB = opA->next_op; opB; opB = opB->next_op)
	{
	  /*
	   *    match pattern
	   */

	  if (!L_int_pred_comparison_opcode (opB))
	    continue;
	  if (!L_is_macro (opB->src[0]) || !L_is_macro (opB->src[1]))
	    continue;
	  if (opB->src[0]->value.mac != TAHOE_MAC_ZERO ||
	      opB->src[1]->value.mac != TAHOE_MAC_ZERO)
	    continue;
	  if (opB->com[1] != Lcmp_COM_EQ)
	    continue;
	  if (opB->dest[0]->ptype != L_PTYPE_UNCOND_T)
	    continue;
	  if (!opB->pred[0])
	    continue;

	  if (!(L_is_dest_operand (opB->pred[0], opA)))
	    continue;
	  if (!(L_no_defs_between (opB->pred[0], opA, opB)))
	    break;
	  if (!(L_same_def_reachs (opB->dest[0], opA, opB)))
	    continue;
	  if (!(L_not_live_at_cb_end (cb, opA, opB->pred[0])))
	    continue;

	  if (!(L_no_uses_between (opB->dest[0], opA, opB)))
	    continue;
	  if (!(L_no_defs_between (opB->dest[0], opA, opB)))
	    continue;
	  if (!(L_no_uses_between (opB->pred[0], opA, opB)))
	    continue;
	  if (!(L_can_change_all_later_uses_with (cb, opB->pred[0],
						  opB->dest[0], opB)))
	    continue;
	  if (!(L_no_br_between (opA, opB)))
	    continue;

	  /* Only propagate back into uncond types
	   */
	  not_uncond = 0;
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!opA->dest[i])
		continue;
	      if (!(L_same_operand (opB->pred[0], opA->dest[i])))
		continue;
	      if (opA->dest[i]->ptype != L_PTYPE_UNCOND_T)
		not_uncond = 1;
	    }
	  if (not_uncond)
	    continue;

	  /*
	   * Avoid pattern:
	   *     A: use w = x,y
	   *     B: mov z = w
	   *        ...
	   *        z = ?
	   *          = w
	   */

	  /*  DIA, JWS - This checks for a redef on either operand
	   * which would require placing a move instruction in the
	   * current cb.  In this case the optimization is not
	   * performed.  
	   */
	  redef_w = redef_z = bad_redef = 0;
	  for (ptr = opB->next_op; ptr; ptr = ptr->next_op)
	    {
	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (L_same_operand (opB->pred[0], ptr->dest[i]))
		    {
		      if (ptr->dest[i]->ptype == L_PTYPE_UNCOND_T ||
			  ptr->dest[i]->ptype == L_PTYPE_UNCOND_F)
			{
			  redef_w = 1;	/* w is clobbered */
			  break;
			}
		      else
			bad_redef = 1;	/* Can't handle partial redefine of w */
		    }
		  if (L_same_operand (opB->dest[0], ptr->dest[i]))
		    redef_z = 1;
		  /* Once z is redefined, need to check for further uses of
		   * w which would require a mov w,z in this cb
		   */
		}
	      if (redef_z)
		{
		  if (L_general_branch_opcode (ptr) ||
		      L_check_branch_opcode (ptr))
		    {
		      if (L_in_oper_OUT_set
			  (cb, ptr, opB->pred[0], TAKEN_PATH))
			{
			  bad_redef = 1;
			  break;
			}
		    }
		  if (L_same_operand (opB->pred[0], ptr->pred[0]))
		    {
		      bad_redef = 1;
		      break;
		    }
		}
	      if (bad_redef)
		break;

	      if (redef_w)
		break;
	    }
	  if (bad_redef)
	    continue;

	  /*
	   *    replace pattern.
	   *
	   *    Change all later uses of opB->src[0] to opB->dest[0],
	   *    stop when opB->src[0] is redefined.  Also, at any
	   *    branch oper which opB->src[0] is live, insert
	   *    opB->src[0] <- opB->dest[0] to ensure correct
	   *    execution.  
	   */
#ifdef DEBUG_COPY_PROP2
	  printf ("Rev copy prop op%d -> op%d (cb %d)\n",
		  opA->id, opB->id, cb->id);
#endif
	  redef = 0;
	  redo_flow = 0;
	  for (ptr = opA->next_op; ptr; ptr = ptr->next_op)
	    {
	      if (ptr == opB)
		continue;
	      if (L_general_branch_opcode (ptr) ||
		  L_check_branch_opcode (ptr))
		{
		  if (L_in_oper_OUT_set (cb, ptr, opB->pred[0], TAKEN_PATH))
		    {
		      redo_flow = 1;
		      new_op = L_create_new_op (opB->opc);

		      new_op->dest[0] = L_copy_operand (opB->pred[0]);
		      new_op->pred[0] = L_copy_operand (opB->dest[0]);
		      new_op->src[0] = L_copy_operand (opB->src[0]);
		      new_op->src[1] = L_copy_operand (opB->src[1]);
		      new_op->com[0] = opB->com[0];
		      new_op->com[1] = opB->com[1];

		      L_insert_op_at_dest_of_br (cb, ptr, new_op, 0);
#ifdef DEBUG_COPY_PROP2
		      printf ("Placing cmp op %d at dest of br op %d\n",
			      new_op->id, ptr->id);
#endif
		    }
		}
	      if (L_same_operand (opB->pred[0], ptr->pred[0]))
		{
		  L_delete_operand (ptr->pred[0]);
		  ptr->pred[0] = L_copy_operand (opB->dest[0]);
		}

	      for (i = 0; i < L_max_dest_operand; i++)
		{
		  if (L_same_operand (opB->pred[0], ptr->dest[i]))
		    redef = 1;
		}
	      if (redef)
		break;
	    }
	  /* 8/14/96, Bob McGowan */
	  if (!ptr)
	    {
	      /* If the pointer is NULL, then we know the above loop went
	         through the entire cb.
	         Check the live variables through the fall through path.
	         If the src of the mov is live out, then we must put
	         mov src = dest in the fall though block */

	      /*  if( L_in_oper_OUT_set( cb, cb->last_op, opB->src[0],
	         FALL_THRU_PATH )) */

	      if ((!L_general_branch_opcode (cb->last_op)) &&
		  (!L_check_branch_opcode (cb->last_op)) &&
		  (!L_subroutine_return_opcode (cb->last_op)) &&
		  (L_in_cb_IN_set (cb->next_cb, opB->src[0])))
		{
		  redo_flow = 1;
		  new_op = L_create_new_op (opB->opc);

		  new_op->dest[0] = L_copy_operand (opB->pred[0]);
		  new_op->pred[0] = L_copy_operand (opB->dest[0]);
		  new_op->src[0] = L_copy_operand (opB->src[0]);
		  new_op->src[1] = L_copy_operand (opB->src[1]);
		  new_op->com[0] = opB->com[0];
		  new_op->com[1] = opB->com[1];

		  L_insert_op_at_fallthru_dest (cb, new_op, 0);
#ifdef DEBUG_COPY_PROP2
		  printf ("Placing mov op %d at fall thru\n", new_op->id);
#endif
		}
	    }
	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!L_same_operand (opA->dest[i], opB->pred[0]))
		continue;
	      L_delete_operand (opA->dest[i]);
	      opA->dest[i] = L_copy_operand (opB->dest[0]);
	    }
	  L_nullify_operation (opB);

	  if (redo_flow)
	    {
	      MOD ("Flow analysis LV (due to rev cp pr)");
	      L_do_flow_analysis (L_fn, LIVE_VARIABLE);
	    }
	  change += 1;
	}
    }
  return change;
}


int
M_local_constant_folding (L_Cb * cb)
{
  int change = 0;
  ITint64 sum;
  L_Oper *opA, *opB;
  L_Operand *dest, *src;

  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       * adds   rY = c, rX  --> adds   rY = c, rX
       * adds   rZ = d, rY  --> adds   rZ = c+d, rX
       */

      if (opA->proc_opc != TAHOEop_ADDS)
	continue;

      dest = opA->dest[0];
      src = opA->src[1];

      if (!L_is_int_constant (opA->src[0]))
	continue;

      if (L_same_operand (src, dest))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  if (!PG_intersecting_predicates_ops (opA, opB))
	    continue;

	  if (L_general_subroutine_call_opcode (opB))
	    break;

	  if (L_is_dest_operand (dest, opB))
	    break;

	  if (L_is_dest_operand (src, opB))
	    break;

	  if (opB->proc_opc != TAHOEop_ADDS)
	    continue;
	  if (!L_is_int_constant (opB->src[0]))
	    continue;
	  if (!L_same_operand (dest, opB->src[1]))
	    continue;
	  if (!PG_superset_predicate_ops (opA, opB))
	    continue;
	  sum = opA->src[0]->value.i + opB->src[0]->value.i;

	  if (!SIMM_14 (sum))
	    continue;

	  /* PERFORM */

#ifdef DEBUG_CONSTANT_FOLDING
	  fprintf (stderr, "Constant folding performed:\n");
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif

	  opB->src[0]->value.i = sum;

	  L_delete_operand (opB->src[1]);

	  opB->src[1] = L_copy_operand (opA->src[1]);

	  change++;
	}
    }
  return change;
}


/*
 * pattern:      mov x = y
 *               use z = w,x
 *
 */

int
M_local_copy_propagation (L_Cb * cb, int use_dem)
{
  int i, change;
  int extA;
  L_Oper *opA, *opB;
  int replace_opc;

  change = 0;
  for (opA = cb->first_op; opA != NULL; opA = opA->next_op)
    {
      /*
       *        match pattern
       */

      if (L_move_opcode (opA))
	extA = 0;
      else if (L_sign_or_zero_extend_opcode (opA))
	extA = 1;
      else
	continue;

      if (!(L_is_variable (opA->src[0])) || L_has_unsafe_macro_operand (opA))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *    match pattern
	   */
	  if (!(L_is_src_operand (opA->dest[0], opB)))
	    continue;
	  if (extA && !L_redundant_extension_fwd (opA, opB))
	    {
	      if (L_move_opcode (opB))
		{
		  replace_opc = 1;
		}
	      else if (L_sign_or_zero_extend_opcode (opB))
		{
		  int ctA, ctB;

		  ctA = L_opcode_ctype (opA);
		  ctB = L_opcode_ctype (opB);

		  if (L_extension_compatible_ctype (ctA, ctB))
		    replace_opc = 1;
		  else
		    continue;
		}
	      else
		{
		  continue;
		}
	    }
	  else
	    {
	      replace_opc = 0;
	    }

	  if (L_is_macro (opA->src[0]) && M_subroutine_call (opB->opc))
	    continue;
	  if (!(L_can_change_src_operand (opB, opA->dest[0])))
	    continue;
	  if (use_dem && opB->pred[1])
	    {
	      if (!PG_subset_predicate_ops_explicit (opB, 1, opA, 0))
		continue;
	    }
	  else
	    {
	      if (!PG_superset_predicate_ops (opA, opB))
		continue;
	    }
	  if (!(L_no_defs_between (opA->dest[0], opA, opB)))
	    break;
	  if (!(L_same_def_reachs (opA->src[0], opA, opB)))
	    continue;
	  macro_flag = (L_is_fragile_macro (opA->dest[0]) ||
			L_is_fragile_macro (opA->src[0]));
	  load_flag = 0;
	  store_flag = 0;
	  if (!(L_no_danger (macro_flag, load_flag, store_flag, opA, opB)))
	    break;
	  /*
	   *    replace pattern.
	   */
#ifdef DEBUG_COPY_PROP1
	  fprintf (stderr,
		   "> M_local_copy_propagation: op%d -> op%d, (cb %d)\n",
		   opA->id, opB->id, cb->id);
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif

	  if (replace_opc)
	    {
	      L_change_opcode (opB, opA->opc);
	      opB->proc_opc = opA->proc_opc;
	    }

	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_same_operand (opA->dest[0], opB->src[i]))
		{
		  L_delete_operand (opB->src[i]);
		  opB->src[i] = L_copy_operand (opA->src[0]);
		  change += 1;
		}
	    }

#ifdef DEBUG_COPY_PROP1
	  fprintf (stderr, "  becomes\n");
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif
	}
    }

  return change;
}


/* M_local_rev_copy_propagation (L_Cb *cb, int use_dem)
 * ----------------------------------------------------------------------
 * A: w = 
 * B: z = w
 */

int
M_local_rev_copy_propagation (L_Cb * cb, int use_dem)
{
  int change = 0, extA;
  L_Oper *opA, *opB, *opC;

  for (opB = cb->first_op; opB; opB = opB->next_op)
    {
      L_Operand *opdW, *opdZ;
      int stop = 0;

      if (L_move_opcode (opB))
	extA = 0;
      else if (L_sign_or_zero_extend_opcode (opB))
	extA = 1;
      else
	continue;

      if (L_has_unsafe_macro_operand (opB))
	continue;

      opdW = opB->src[0];
      opdZ = opB->dest[0];

      if (L_is_macro (opdW))
	continue;

      for (opA = opB->prev_op; opA && !stop; opA = opA->prev_op)
	{
	  int macro_flag, i, partial = 0;

	  if (!PG_intersecting_predicates_ops (opA, opB))
	    continue;

	  /* filter */

	  if (L_general_branch_opcode (opA) || L_check_branch_opcode (opA))
	    stop = 1;

	  if (L_is_src_operand (opdZ, opA))
	    stop = 1;

	  if (L_is_dest_operand (opdZ, opA))
	    stop = 1;

	  /* match exactly once */

	  if (!(L_is_dest_operand (opdW, opA)))
	    continue;

	  /* W is redefined by opA.  Further upward search is futile! */

	  if (L_is_opcode (Lop_DEFINE, opA))
	    break;

	  if (extA && !L_redundant_extension_rev (opA, opB))
	    break;

	  if (use_dem && opA->pred[1] &&
	      !PG_equivalent_predicates_ops_explicit (opA, 0, opA, 1))
	    {
	      if (!PG_equivalent_predicates_ops_explicit (opA, 1, opB, 0))
		break;
	      if (!L_same_def_reachs (opB->pred[0], opA, opB))
		break;
	      partial = 1;
	    }
	  else
	    {
	      if (!PG_equivalent_predicates_ops (opA, opB))
		break;
	    }

	  if (!(L_can_change_dest_operand (opA, opdZ)))
	    break;

	  if (L_is_macro (opdZ) && !(L_single_use_of (cb, opdW, opA)))
	    break;
	  if (!(L_not_live_at_cb_end (cb, opA, opdW)))
	    break;
	  if (!(L_same_def_reachs (opdZ, opA, opB)))
	    break;
	  if (partial && !(L_no_uses_between (opA->dest[0], opA, opB)))
	    break;
	  if (!(L_can_change_all_uses_between (cb, opdW, opdZ, opA, opB)))
	    break;
	  if (!(L_can_change_all_later_uses_with (cb, opdW, opdZ, opB)))
	    break;

	  macro_flag = (L_is_fragile_macro (opdZ)
			|| L_is_fragile_macro (opdW));

	  if (!(L_no_danger (macro_flag, 0, 0, opA, opB)))
	    break;

	  {
	    /*
	     * Avoid pattern:
	     *     A: use w =
	     *     B: mov z = w
	     *        ...
	     *        z = ?
	     *          = w
	     */

	    /*  DIA, JWS - This checks for a redef on either operand
	     * which would require placing a move instruction in the
	     * current cb.  In this case the optimization is not
	     * performed.  
	     */
	    int redef_w = 0, redef_z = 0, bad_redef = 0;

	    for (opC = opB->next_op; opC; opC = opC->next_op)
	      {
		if (!PG_intersecting_predicates_ops (opB, opC))
		  continue;

		if (L_is_dest_operand (opdW, opC))
		  {
		    if (PG_superset_predicate_ops (opC, opB))
		      redef_w = 1;
		    else
		      bad_redef = 1;
		  }

		if (!redef_z && L_is_dest_operand (opdZ, opC))
		  redef_z = 1;

		/* Once z is redefined, need to check for further uses of
		 * w which would require a mov w,z in this cb
		 */

		if (redef_z && L_is_src_operand (opdW, opC))
		  bad_redef = 1;

		/* Don't want to put moves in other blocks either */

		if ((L_general_branch_opcode (opC) ||
		     L_check_branch_opcode (opC)) &&
		    L_in_oper_OUT_set (cb, opC, opdW, TAKEN_PATH))
		  bad_redef = 1;

		if (bad_redef || redef_w)
		  break;
	      }

	    if (bad_redef)
	      break;
	  }

	  /*
	   *    replace pattern.
	   *
	   *    Change all later uses of opdW to opdZ,
	   *    stop when opdW is redefined.  Also, at any
	   *    branch oper which opdW is live, insert
	   *    opdW <- opdZ to ensure correct
	   *    execution.  
	   */
#ifdef DEBUG_COPY_PROP2
	  fprintf (stderr,
		   "> M_local_rev_copy_propagation:  op%d -> op%d (cb %d)\n",
		   opA->id, opB->id, cb->id);
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif

	  for (opC = opA->next_op; opC; opC = opC->next_op)
	    {
	      if (!PG_intersecting_predicates_ops (opB, opC))
		continue;
	      if (opC == opB)
		continue;
	      if ((L_general_branch_opcode (opC) ||
		   L_check_branch_opcode (opC)) &&
		  L_in_oper_OUT_set (cb, opC, opdW, TAKEN_PATH))
		L_punt ("M_local_rev_copy_propagation: bad case encountered");

	      for (i = 0; i < L_max_src_operand; i++)
		{
		  if (L_same_operand (opdW, opC->src[i]) &&
		      PG_subset_predicate_ops (opC, opB))
		    {
		      L_delete_operand (opC->src[i]);
		      opC->src[i] = L_copy_operand (opdZ);
		    }
		}

	      if (PG_superset_predicate_ops (opC, opB) &&
		  L_is_dest_operand (opdW, opC))
		break;
	    }

	  for (i = 0; i < L_max_dest_operand; i++)
	    {
	      if (!L_same_operand (opA->dest[i], opdW))
		continue;
	      L_delete_operand (opA->dest[i]);
	      opA->dest[i] = L_copy_operand (opdZ);
	    }

	  if (partial)
	    {
	      L_delete_operand (opA->pred[0]);
	      opA->pred[0] = L_copy_operand (opB->pred[0]);
	    }

	  L_nullify_operation (opB);

	  change++;
#ifdef DEBUG_COPY_PROP2
	  fprintf (stderr, "  becomes\n");
	  L_print_oper (stderr, opA);
	  L_print_oper (stderr, opB);
#endif
	  break;
	}
    }
  return change;
}


int
M_local_constant_propagation (L_Cb * cb)
{
  int i, old_num_oper, new_num_oper, change = 0;
  L_Oper *opA, *opB;
  L_Operand *old_src;

  for (opA = cb->first_op; opA; opA = opA->next_op)
    {
      /*
       *        match pattern
       */
      if (!L_move_opcode (opA) || !L_is_constant (opA->src[0]))
	continue;

      for (opB = opA->next_op; opB != NULL; opB = opB->next_op)
	{
	  int macro_flag, load_flag, store_flag;
	  /*
	   *    match pattern
	   */
	  if (!L_move_opcode (opB) ||
	      !L_is_src_operand (opA->dest[0], opB) ||
	      !L_can_change_src_operand (opB, opA->dest[0]) ||
	      !PG_superset_predicate_ops (opA, opB))
	    continue;

	  if (!L_no_defs_between (opA->dest[0], opA, opB))
	    break;

	  macro_flag = L_is_fragile_macro (opA->dest[0]);
	  load_flag = 0;
	  store_flag = 0;

	  if (!L_no_danger (macro_flag, load_flag, store_flag, opA, opB))
	    break;

	  /*
	   *    replace pattern.
	   */
	  for (i = 0; i < L_max_src_operand; i++)
	    {
	      if (L_same_operand (opA->dest[0], opB->src[i]))
		{
		  old_num_oper = M_num_oper_required_for (opB, L_fn->name);
		  old_src = opB->src[i];
		  opB->src[i] = L_copy_operand (opA->src[0]);
		  new_num_oper = M_num_oper_required_for (opB, L_fn->name);

		  /* if arch cannot handle opB after const prop, undo it */

		  if (new_num_oper > old_num_oper)
		    {
		      L_delete_operand (opB->src[i]);
		      opB->src[i] = old_src;
		      continue;
		    }

		  L_delete_operand (old_src);
#ifdef TEST_CONST_PROP
		  fprintf (ERR, "const prop: op%d -> op%d, src %d (cb %d)\n",
			   opA->id, opB->id, i, cb->id);
#endif
		  change++;
		}
	    }

	  /* Make sure that cleanup fixes the proc_opc */

	  opB->proc_opc = opA->proc_opc;
	}
    }
  return change;
}


void
Mopti_debug (char *fmt, ...)
{
  va_list args;
  time_t now;
  char timebuf[64];

  time (&now);
  strftime (timebuf, 64, "%Y%m%d %H:%M:%S", localtime (&now));
  fprintf (stderr, ">MoD %s> ", timebuf);
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  fflush (stderr);
  return;
}
