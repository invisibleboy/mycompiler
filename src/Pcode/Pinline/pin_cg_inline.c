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

#include <config.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pin_inline.h"

#define KEY_DIFFERENCE  1.0
#define DEBUG_DUMP_GRAPHS 0


static long int
Pin_il_size (PinCG_Arc arc)
{
  long int size;
  PinCG_Func fn = arc->callee->func;

  if (Pin_body_size_metric == PIN_BODY_EXECD)
    {
      size = arc->use_orig ? fn->o_ebodysize :
	fn->i_ebodysize;
    }
  else
    {
      size = arc->use_orig ? fn->o_bodysize :
	fn->i_bodysize;
    }

  return size;
}


static int
Pin_inlining_candidate (PinCG_Arc arc)
{
  PinCG_Node callee = arc->callee, caller = arc->caller;
  PinCG_Func callee_fn = callee->func, caller_fn = caller->owner;
  long int callee_size, caller_size;
  int callee_frame;

  if (arc->noinline)
    {
      if (Pin_trace_heap)
	fprintf (Flog, "\tXX NULLIFIED ARC\n");
      return 0;
    }
  else if (!callee)
    {
      if (Pin_trace_heap)
	fprintf (Flog, "\tXX UNKNOWN CALLEE\n");
      return 0;
    }
  else if (callee_fn->is_vararg)
    {
      if (Pin_trace_heap)
	fprintf (Flog, "\tXX NOINLINE (VARARG)\n");
      return 0;
    }
  else if (!inline_all_functions && !callee_fn->is_empty_func)
    {
      if (callee_fn->num_parms != arc->num_parms)
	{
	  if (Pin_trace_heap)
	    fprintf (Flog, "\tXX NOINLINE (PARM INEQUIVALENCE)\n");
	  return 0;
	}
      else if (arc->r_an < Pin_min_arc_ratio)
	{
	  if (Pin_trace_heap)
	    fprintf (Flog, "\tXX NOINLINE (MIN ARC RATIO)\n");
	  return 0;
	}
    }

  callee_size = Pin_il_size (arc);
  caller_size = (Pin_body_size_metric == PIN_BODY_EXECD) ?
    caller_fn->i_ebodysize : caller_fn->i_bodysize;
  callee_frame = arc->use_orig ? callee_fn->o_stacksize : 
    callee_fn->i_stacksize;

  if (!inline_all_functions)
    {
      if ((!exclude_small_from_ratio_limit || 
	   (callee_size > small_function_thresh)) && (callee_size > Pin_budget))
	{
	  if (Pin_trace_heap)
	    fprintf (Flog, "\tXX BUDGET\n");
	  return 0;
	}
      else if ((caller_size + callee_size) > max_function_size)
	{
	  if (Pin_trace_heap)
	    fprintf (Flog, "\tXX NOINLINE (MAX BODY SIZE)\n");
	  return 0;
	}
      else if ((caller_fn->i_stacksize + callee_frame) > max_sf_size_limit)
	{
	  if (Pin_trace_heap)
	    fprintf (Flog, "\tXX NOINLINE (MAX STACK SIZE)\n");
	  return 0;
	}
    } 
  else if ((!inline_self_recursion || size_only) && RECURSIVE (arc))
    {
      if (Pin_trace_heap)
	fprintf (Flog, "\tXX IGNORING RECURSIVE ARC\n");
      return 0;
    }
  return 1;
}


static void
Pin_print_inline_msg (FILE * F, PinCG_Arc arc, int esize)
{
  PinCG_Node en, rn;
  PinCG_Func ef, rf;
  char *efn, *rfn, *efi, *rfi;
  int eop, eeop, erop, rop;
  double cwt, rwt;

  en = arc->callee;
  rn = arc->caller;

  ef = arc->callee->func;
  rf = arc->caller->owner;

  cwt = arc->weight;

  efn = ef->funcname;
  efi = ef->orig_filename;

  if (inline_inlined_body)
    {
      eop = ef->i_bodysize;
      eeop = ef->i_ebodysize;
    }
  else
    {
      eop = ef->o_bodysize;
      eeop = ef->o_ebodysize;
    }

  rfn = rf->funcname;
  rfi = rf->orig_filename;
  rop = rf->i_bodysize;
  erop = rf->i_ebodysize;
  rwt = rn->weight;

  fprintf (F, "Pcode inlining %s() [%s], %d/%d ops\n"
	   "   callsite %s() [%s] wt %0.3g/%0.3g, %d/%d ops RESULT %d ops\n",
	   efn, efi, eeop, eop, rfn, rfi, cwt, rwt, erop, rop, esize);

  return;
}


static int Pin_inline_count = 0;

void
Pin_inline_callgraph (void)
{
  double *dp, key, newkey;
  double orig_body_size = TotalBodySize,
    orig_ebody_size = TotalEBodySize, orig_cs_wt = TotalCallWeight;
#if DEBUG_DUMP_GRAPHS
  PinCG_Node n;
  double maxwt;
#endif

  if (Pin_trace_heap)
    fprintf (Flog, "STARTING INLINING PRIORITY HEAP\n"
	     "---------------------------------------------------------\n");

#if DEBUG_DUMP_GRAPHS
  List_start (Pin_callgraph->nodes);
  while ((n = (PinCG_Node) List_next (Pin_callgraph->nodes)))
    {
      if (n->weight > maxwt)
	maxwt = n->weight;
    }
#endif

  while ((dp = Heap_TopWeight (Pin_arc_heap)))
    {
      long int size, esize;
      PinCG_Arc arc;
      PinCG_Func callee_func;
      key = *dp;
      arc = (PinCG_Arc) Heap_ExtractTop (Pin_arc_heap);

      if (Pin_trace_heap)
	{
	  fprintf (Flog, "[key %0.3f] ", key);
	  Dump_PinCG_Arc (Flog, arc);
	}

      if (arc->inlined)
	{
	  ;
	}
      else if (!Pin_inlining_candidate (arc))
	{
	  arc->noinline = 1;
	}
      else if (((newkey = PinCG_compute_arc_key (arc)) < 0) && !inline_all_functions)
	{
	  arc->noinline = 1;
	  if (Pin_trace_heap)
	    fprintf (Flog, "\tXX WARNING: NEGATIVE KEY\n");
	}
      else if (!inline_all_functions && (newkey < min_expansion_key))
	{
	  arc->noinline = 1;
	  if (Pin_trace_heap)
	    fprintf (Flog, "\tXX NEWKEY %f SMALL ENOUGH TO IGNORE\n", newkey);
	}
      else if ((fabs (key - newkey) > KEY_DIFFERENCE))
	{
	  Heap_Insert (Pin_arc_heap, arc, newkey);
	  if (Pin_trace_heap)
	    {
	      fprintf (Flog, "\t>> KEY ADJUSTED TO %f (ARC SAVED)\n", newkey);
	      if (newkey > key)
		fprintf (Flog, "Warning : newkey is larger\n");
	    }
	}
      else
	{
#if DEBUG_DUMP_GRAPHS
	  char fname[128];
#endif
	  /* INLINE ARC */
	  
	  callee_func = arc->callee->func;

	  if (!arc->use_orig)
	    {
	      size = callee_func->i_bodysize;
	      esize = callee_func->i_ebodysize;
	    }
	  else
	    {
	      size = callee_func->o_bodysize;
	      esize = callee_func->o_ebodysize;
	    }

	  if (print_inline_trace)
	    Pin_print_inline_msg (Flog, arc, 
				  esize + arc->caller->owner->i_ebodysize);

	  Pin_inline_count++;

	  arc->caller->owner->i_bodysize += size;
	  arc->caller->owner->i_ebodysize += esize;
	  TotalBodySize += size;
	  TotalEBodySize += esize;

	  Pin_reduce_budget ((double) 
			     (Pin_body_size_metric == PIN_BODY_EXECD) ? 
			     esize : size);

#if DEBUG_DUMP_GRAPHS
	  sprintf (fname, "inl-%04d.callee-%s-pre", Pin_inline_count,
		   arc->callee->func->funcname);
	  PinCG_dot_inlining_graph (arc->callee->func, fname, maxwt);
	  sprintf (fname, "inl-%04d.caller-%s-pre", Pin_inline_count,
		   arc->caller->owner->funcname);
	  PinCG_dot_inlining_graph (arc->caller->owner, fname, maxwt);
#endif
	  
	  {
	    double wt_n1, wt_n2;
	    double wt_a1, wt_a2;
	    PinCG_compute_total_weight (&wt_n1, &wt_a1);
	    TotalCallWeight -= PinCG_inline (arc);
	    PinCG_compute_total_weight (&wt_n2, &wt_a2);

	    if (fabs (wt_n2 - wt_n1) > 0.1 || fabs (wt_a2 - wt_a1) > 0.1)
	      P_warn ("WEIGHT NOT CONSERVED");
	  }
#if DEBUG_DUMP_GRAPHS
	  sprintf (fname, "inl-%04d.callee-%s-post", Pin_inline_count,
		   arc->callee->func->funcname);
	  PinCG_dot_inlining_graph (arc->callee->func, fname, maxwt);
	  sprintf (fname, "inl-%04d.caller-%s-post", Pin_inline_count,
		   arc->caller->owner->funcname);
	  PinCG_dot_inlining_graph (arc->caller->owner, fname, maxwt);
#endif
	}
    }				/* while */

  if (print_inline_stats)
    {
      double inl_rat, csi_rat, einl_rat;

      inl_rat = (orig_body_size > 0.1) ? TotalBodySize / orig_body_size : 1.0;
      einl_rat = (orig_ebody_size > 0.1) ?
	TotalEBodySize / orig_ebody_size : 1.0;
      csi_rat = 100 * ((orig_cs_wt > 0.1) ?
		       (orig_cs_wt - TotalCallWeight) / orig_cs_wt : 0.0);
      fprintf (Fstat,
	       "--------------------------------------------------"
	       "-----------------------------\n"
	       " INLINING SUMMARY\n"
	       " Original code size %0.3f (%0.3f executed)\n"
	       " Total code expansion %0.3f (%0.3f executed)\n"
	       " %0.3f%% of dynamic callsites inlined\n"
	       "--------------------------------------------------"
	       "-----------------------------\n",
	       orig_body_size, orig_ebody_size, inl_rat, einl_rat, csi_rat);

#if 0
      {
	PinCG_Node n;
	List_start (Pin_callgraph->nodes);
	while ((n = (PinCG_Node) List_next (Pin_callgraph->nodes)))
	  printf ("NODE %s:%s() wt %0.3f bs %ld/%ld->%ld/%ld\n",
		  n->func->orig_filename,
		  n->func->funcname, n->weight,
		  n->func->o_ebodysize, n->func->o_bodysize,
		  n->func->i_ebodysize, n->func->i_bodysize);
      }
#endif
    }
  return;
}				/* Pin_inline_callgraph */
