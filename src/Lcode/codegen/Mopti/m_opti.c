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
 *	1) common subexpression ellimination
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
 *	All rights granted to AMD Inc.
 *
\*****************************************************************************/
/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <Lcode/l_opti.h>
#include <Lcode/m_opti.h>

/* Variable initialization */
int Mopti_imax_num_iterations = 10;
int Mopti_omax_num_iterations = 10;
int Mopti_init_performed = 0;

/*
 *	Mopti Parameters
 */
int Mopti_print_stats = 0;
int Mopti_debug_copy_prop = 0;
int Mopti_debug_common_subs = 0;
int Mopti_debug_dead_code = 0;
int Mopti_do_common_subs = 1;
int Mopti_do_copy_prop = 1;
int Mopti_do_dead_code = 1;


void
L_read_parm_mopti (Parm_Parse_Info * ppi)
{
  L_read_parm_b (ppi, "print_stats", &Mopti_print_stats);
  L_read_parm_b (ppi, "debug_copy_prop", &Mopti_debug_copy_prop);
  L_read_parm_b (ppi, "debug_common_subs", &Mopti_debug_common_subs);
  L_read_parm_b (ppi, "debug_dead_code", &Mopti_debug_dead_code);
  L_read_parm_b (ppi, "do_common_subs", &Mopti_do_common_subs);
  L_read_parm_b (ppi, "do_copy_prop", &Mopti_do_copy_prop);
  L_read_parm_b (ppi, "do_dead_code", &Mopti_do_dead_code);

  /* This prevents the code from reperforming initialization again */
  Mopti_init_performed = 1;
}

void
Mopti_init (Parm_Macro_List * command_line_macro_list)
{
  /* Load the parameters specific to Lcode code generation */
  L_load_parameters (L_parm_file, command_line_macro_list,
		     "(Mopti", L_read_parm_mopti);
}


void
Mopti_perform_optimizations (L_Func * fn,
			     Parm_Macro_List * command_line_macro_list)
{
  int j, k, change, common_subs, copy_prop, dead_code, c1 = 0, c2 = 0, c3 = 0;
  L_Cb *cb;

  if (!Mopti_init_performed)
    {
      Mopti_init (command_line_macro_list);
    }

  if (Mopti_print_stats)
    {
      fprintf (stderr, "  Machine Level Code Optimizations\n");
      fprintf (stderr, "  ======================================\n");
    }

  k = 0;
  common_subs = copy_prop = dead_code = 0;

  if (Mopti_do_dead_code)
    L_split_pred_defines (fn);

  do
    {
      change = 0;

      L_do_flow_analysis (fn, LIVE_VARIABLE);

      if (Mopti_do_dead_code)
	{
	  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	    {
	      /* Handle dead code elimination */
	      c3 = L_local_dead_code_removal (cb)
		+ L_global_dead_code_removal (cb);
	      dead_code += c3;
	      change += c3;
	    }
	}

      PG_setup_pred_graph (fn);

      for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
	{
	  j = 0;

	  do
	    {
	      if (Mopti_do_common_subs)
		{
		  /* Handle common sub-expression elimination */
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

	      if (Mopti_do_copy_prop)
		{
		  /* Handle copy propogation */
		  /*
		     c2 = L_local_rev_copy_propagation(cb);
		     copy_prop += c2;
		   */
		  c2 = L_local_copy_propagation (cb);
		  copy_prop += c2;
		  change += c2;
		}

	      j += 1;		/* iteration counter */

	    }
	  while ((c1 + c2 != 0) && (j != Mopti_imax_num_iterations));

	}

      k += 1;			/* iteration counter */

    }
  while ((change != 0) && (k != Mopti_omax_num_iterations));

  if (Mopti_do_dead_code)
    L_combine_pred_defines (fn);

  if (Mopti_print_stats)
    {
      fprintf (stderr, "   Common subexpressions: %d\n", common_subs);
      fprintf (stderr, "   Copy propagations    : %d\n", copy_prop);
      fprintf (stderr, "   Dead code removed    : %d\n", dead_code);
      fprintf (stderr, "  --------------------------------------\n");
    }
}
