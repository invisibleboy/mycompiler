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
 *      File:   pin_callgraph.c
 *      Author: Ben-Chung Cheng
 *      Copyright (c) 1995 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <library/list.h>
#include <library/l_parms.h>
#include <library/c_symbol.h>
#include <machine/m_spec.h>
#include <Pcode/parms.h>
#include <Pcode/pcode.h>
#include <Pcode/cast.h>
#include <math.h>
#include <library/dynamic_symbol.h>
#include "pin_inline.h"

PinCG_Graph Pin_callgraph = NULL;

static int Pin_max_func_id = 0;
static int Pin_max_node_id = 0;
static int Pin_max_arc_id = 0;

PinCG_Func
PinCG_create_func (char *funcname, Key key)
{
  PinCG_Func cgf;

  cgf = ALLOCATE (struct _PinCG_Func);
  cgf->id = ++Pin_max_func_id;

  cgf->key = key;
  cgf->num_parms = 0;
  cgf->o_bodysize = cgf->i_bodysize = 0;
  cgf->o_ebodysize = cgf->i_ebodysize = 0;
  cgf->o_stacksize = cgf->i_stacksize = 0;
  cgf->weight = 0.0;
  cgf->indir_weight = 0.0;
  cgf->o_path = cgf->i_path = NULL;
  cgf->funcname = C_findstr (funcname);
  cgf->filename = NULL;
  cgf->orig_filename = NULL;
  cgf->is_vararg = 0;
  cgf->is_empty_func = 0;
  cgf->is_noninline = 0;

  Pin_callgraph->funcs = List_insert_last (Pin_callgraph->funcs, cgf);

  if (!strcmp (funcname, "main"))
    {
      if (Pin_callgraph->root_func)
	P_punt ("PinCG_create_func: main() is redefined");
      Pin_callgraph->root_func = cgf;
    }

  cgf->node = PinCG_create_node (cgf, cgf);
  cgf->clones = NULL;

  return cgf;
}


PinCG_Node
PinCG_create_node (PinCG_Func cgf_fun, PinCG_Func cgf_own)
{
  PinCG_Node cgn;

  cgn = ALLOCATE (struct _PinCG_Node);
  cgn->id = ++Pin_max_node_id;
  cgn->weight = 0.0;
  cgn->func = cgf_fun;
  cgn->owner = cgf_own;
  cgn->arcs = NULL;

  Pin_callgraph->nodes = List_insert_last (Pin_callgraph->nodes, cgn);

  if (cgf_fun == Pin_callgraph->root_func)
    {
      if (Pin_callgraph->root_node)
	P_punt ("PinCG_create_node: main() is redefined");
      Pin_callgraph->root_node = cgn;
    }

  return cgn;
}


PinCG_Arc
PinCG_create_arc (PinCG_Node caller, PinCG_Node callee,
		  int callsite_id, double weight)
{
  PinCG_Arc cga;

  cga = ALLOCATE (struct _PinCG_Arc);
  cga->id = ++Pin_max_arc_id;
  cga->callsite_id = callsite_id;
  cga->caller = caller;
  cga->callee = callee;
  cga->weight = weight;
  cga->r_an = caller->weight >= 0.1 ? weight / caller->weight : 1.0;
  cga->natural = 1;
  cga->indirect = 0;
  cga->noinline = 0;
  cga->use_orig = 0;
  cga->inlined = 0;
  cga->num_parms = 0;
  cga->parent = NULL;

  caller->arcs = List_insert_last (caller->arcs, cga);
  return cga;
}


void
PinCG_adjust_weight (PinCG_Node cgn, double adj)
{
  PinCG_Arc cga;
  double aadj, newwt = cgn->weight + adj;

  if (newwt < 0.0)
    {
      P_warn ("PinCG_adjust_weight: went negative, corrected");
      newwt = 0.0;
      adj = -cgn->weight;
    }

  cgn->weight = newwt;

  List_start (cgn->arcs);
  while ((cga = (PinCG_Arc) List_next (cgn->arcs)))
    {
      aadj = adj * cga->r_an;
      cga->weight += aadj;
      if (cga->weight < 0.0)
	cga->weight = 0.0;
      if (cga->inlined)
	PinCG_adjust_weight (cga->callee, aadj);
    }

  return;
}

Set PinCG_v_fr, PinCG_v_to;

static void
PinCG_shift_weight (PinCG_Node n_fr, PinCG_Node n_to, double adj)
{
  PinCG_Arc a_fr, a_to;
  double aadj;

  if (n_fr == n_to)
    return;

  if (Set_in (PinCG_v_fr, n_fr->id) || Set_in (PinCG_v_to, n_to->id))
    return;

  PinCG_v_fr = Set_add (PinCG_v_fr, n_fr->id);
  PinCG_v_to = Set_add (PinCG_v_to, n_to->id);

  n_fr->weight -= adj;
  n_to->weight += adj;

  List_start (n_fr->arcs);
  List_start (n_to->arcs);
  while ((a_fr = (PinCG_Arc) List_next (n_fr->arcs)))
    {
      a_to = (PinCG_Arc) List_next (n_to->arcs);
      if (a_fr->callee->func != a_to->callee->func ||
	  a_fr->r_an != a_to->r_an)
	P_punt ("GRAPH INCONSISTENCY");

      aadj = adj * a_fr->r_an;
      a_fr->weight -= aadj;
      a_to->weight += aadj;

      if (a_fr->callee != a_to->callee)
	PinCG_shift_weight (a_fr->callee, a_to->callee, aadj);
    }

  PinCG_v_fr = Set_delete (PinCG_v_fr, n_fr->id);
  PinCG_v_to = Set_delete (PinCG_v_to, n_to->id);

  return;
}


PinCG_Arc
PinCG_clone_arc (PinCG_Arc cgp, PinCG_Node cgn_to)
{
  PinCG_Arc cga;

  cga = ALLOCATE (struct _PinCG_Arc);
  cga->id = ++Pin_max_arc_id;
  cga->callsite_id = cgp->callsite_id;
  cga->caller = cgp->caller;
  cga->callee = cgp->callee;
  cga->weight = 0.0;
  cga->r_an = cgp->r_an;
  cga->natural = 0;
  cga->indirect = cgp->indirect;
  cga->noinline = cgp->noinline;
  cga->use_orig = cgp->use_orig;
  cga->num_parms = cgp->num_parms;
  cga->parent = cgp;
  cga->inlined = cgp->inlined;

  cga->caller = cgn_to;
  cgn_to->arcs = List_insert_last (cgn_to->arcs, cga);

  return cga;
}


/* PIN_KAPPA: P_recur(level n) = p * k^n */
#define PIN_KAPPA 1.0


void
PinCG_redirect_flow (PinCG_Arc a, PinCG_Node nn)
{
  PinCG_Node on = a->callee;
  double w_n = on->weight, w_a = a->weight;

  if (w_n < 0.0)
    w_n = 0.0;
  if (w_a < 0.0)
    w_a = 0.0;
  if (w_a > w_n)
    w_a = w_n;

  /* self-recursion */
  if (a->caller == a->callee && (w_a != 0.0))
    {
      w_a = w_n / (PIN_KAPPA + w_n / w_a);

      a->weight = w_a * (1.0 + a->r_an); 
      /* Need to correct arc weight because of recursion;
       * additionally, r_an * w_a will be shifted.
       * If PIN_KAPPA != 1.0, some additional work may be needed.
       */
    }

  PinCG_shift_weight (on, nn, w_a);
  a->callee = nn;
  return;
}


void
PinCG_insert_subgraph (PinCG_Node n)
{
  PinCG_Arc a;

  List_start (n->arcs);
  while ((a = (PinCG_Arc) List_next (n->arcs)))
    {
      if (!a->inlined)
	Heap_Insert (Pin_arc_heap, a, PinCG_compute_arc_key (a));
      else
	PinCG_insert_subgraph (a->callee);
    }
}

PinCG_Node
PinCG_clone_subgraph (PinCG_Arc inlarc)
{
  PinCG_Node n = inlarc->callee;
  PinCG_Func owner = inlarc->caller->owner;
  PinCG_Node nn;
  PinCG_Arc a, na;

  nn = PinCG_create_node (n->func, owner);
  n->func->clones = List_insert_last (n->func->clones, nn);

  List_start (n->arcs);
  while ((a = (PinCG_Arc) List_next (n->arcs)))
    {
      na = PinCG_clone_arc (a, nn);

      if (a->inlined)
	na->callee = PinCG_clone_subgraph (na);
    }

  PinCG_redirect_flow (inlarc, nn);
  inlarc->inlined = 1;

  PinCG_insert_subgraph (nn);

  return nn;
}


PinCG_Node
PinCG_clone_node (PinCG_Arc inlarc)
{
  PinCG_Node n = inlarc->callee;
  PinCG_Func owner = inlarc->caller->owner;
  PinCG_Node nn;
  PinCG_Arc a, na;

  nn = PinCG_create_node (n->func, owner);
  n->func->clones = List_insert_last (n->func->clones, nn);

  List_start (n->arcs);
  while ((a = (PinCG_Arc) List_next (n->arcs)))
    {
      na = PinCG_clone_arc (a, nn);

      /* Undo inlining */

      if (na->inlined)
	{
	  na->callee = na->callee->func->node;
	  na->inlined = 0;
	}
      if (!a->noinline)
	Heap_Insert (Pin_arc_heap, a, PinCG_compute_arc_key (a));
    }


  PinCG_redirect_flow (inlarc, nn);
  inlarc->inlined = 1;

  return nn;
}


double
PinCG_inline (PinCG_Arc inlarc)
{
  if (inlarc->inlined)
    P_punt ("Inlining previously inlined arc");

  /* callee is head of an acyclic graph of single-predecessor nodes
   * connected by inlined arcs */

  if (!inlarc->use_orig)
    PinCG_clone_subgraph (inlarc);
  else
    PinCG_clone_node (inlarc);

  return inlarc->weight;
}


void
PinCG_create_graph (void)
{
  PinCG_Graph gr;

  if (Pin_callgraph)
    P_punt ("NewPinCG_Graph: graph in use");

  gr = ALLOCATE (struct _PinCG_Graph);

  gr->root_func = NULL;
  gr->root_node = NULL;
  gr->funcs = NULL;
  gr->nodes = NULL;

  Pin_callgraph = gr;
  return;
}

#define PIN_HUGE_KEY		1.0E12

double
PinCG_compute_arc_key (PinCG_Arc arc)
{
  PinCG_Func body = arc->callee->func;
  double weight = 0.0, size;

  if (!arc->use_orig)
    {
      if (Pin_body_size_metric == PIN_BODY_EXECD)
	size = body->i_ebodysize;
      else
	size = body->i_bodysize;
    }
  else
    {
      if (Pin_body_size_metric == PIN_BODY_EXECD)
	size = body->o_ebodysize;
      else
	size = body->o_bodysize;
    }

  if (body->is_always_inline)
    {
      weight = PIN_HUGE_KEY;
    }
  else
    {
      if (size_only ||
	  (favor_small_functions && (size <= small_function_thresh) &&
	   (weight > 0.0) && !RECURSIVE (arc) && !arc->indirect))
	weight = PIN_HUGE_KEY;
      else
	weight = arc->weight;

      /* In the case where foo1() calls foo2() and foo1() is
       * preprocessed by Pinline earlier than foo2(), the size of
       * foo2() will be 0 at this moment. If we return 0 instead of
       * the estimated key by assuming the size is 1, later on when
       * the size is resolved, the key will increase which will
       * violate the assumption that the key can only decrease because
       * of the size of the callee can only increase 
       */

      switch (Pin_inline_key_cost)
	{
	case PIN_KEY_SIZE:
	  if (size != 0)
	    weight = weight / size;
	  break;
	case PIN_KEY_MC_SIZE:
	  if ((size != 0) && (weight < (0.99 * body->weight)))
	    weight = weight / size;
	  break;
	case PIN_KEY_SQRT_SIZE:
	default:
	  if (size != 0)
	    weight = weight / sqrt (size);
	  break;
	}
    }

  if (weight < 0.0)
    weight = 0.0;

  return weight;
}


void
Dump_PinCG_Node (FILE * F, PinCG_Node n)
{
  fprintf (F, "id = %d\n", n->id);
  fprintf (F, "num_parms = %d\n", n->func->num_parms);
  fprintf (F, "is_vararg = %d\n", n->func->is_vararg);
  fprintf (F, "i_bodysize = %ld\n", n->func->i_bodysize);
  fprintf (F, "i_stacksize = %ld\n", n->func->i_stacksize);
  fprintf (F, "o_bodysize = %ld\n", n->func->o_bodysize);
  fprintf (F, "o_stacksize = %ld\n", n->func->o_stacksize);
  fprintf (F, "weight = %f\n", n->weight);
  fprintf (F, "indir weight = %f\n", n->func->indir_weight);
  fprintf (F, "path = %s\n", n->func->o_path);
  fprintf (F, "newpath = %s\n", n->func->i_path);
  fprintf (F, "funcname = %s\n", n->func->funcname);
  fprintf (F, "orig_filename = %s\n", n->func->orig_filename);
  return;
}


void
Dump_PinCG_Arc (FILE * F, PinCG_Arc c)
{
  char direct = c->indirect ? 'i' : 'd';
  fprintf (F, "(arc %d (cs %d): %d:%s...%s (%s) -%c-> ", c->id,
	   c->callsite_id, c->caller->id, 
	   c->caller->owner->funcname,
	   c->caller->func->funcname,
	   c->caller->func->orig_filename, direct);

  if (c->callee)
    fprintf (F, "%d:%s (%s)\n",
	     c->callee->id, 
	     c->callee->func->funcname,
	     c->callee->func->orig_filename);
  else
    fprintf (F, "(indirect)\n");

  fprintf (F, "\t(bs %ld/%ld -> i:%ld/%ld o:%ld/%ld)",
	   c->caller->owner->i_ebodysize, c->caller->owner->i_bodysize,
	   c->callee ? c->callee->func->i_ebodysize : 0,
	   c->callee ? c->callee->func->i_bodysize : 0,
	   c->callee ? c->callee->func->o_ebodysize : 0,
	   c->callee ? c->callee->func->o_bodysize : 0);
  fprintf (F, " (sf %ld -> i:%ld o:%ld)",
	   c->caller->func->i_stacksize,
	   c->callee ? c->callee->func->i_stacksize : 0,
	   c->callee ? c->callee->func->o_stacksize : 0);

  if (!size_only)
    {
      fprintf (F, " (wt %0.3f)", c->weight);
      if ((c->weight + 1) > (c->callee->weight))
	fprintf (F, "(SOLE)");
      fprintf (F, "\n");
    }
  return;
}


double
PinCG_inline_weight (PinCG_Arc arc)
{
  double inl_wt;

  if (!RECURSIVE (arc))
    {
      inl_wt = arc->weight;
    }
  else
    {
      double recur_wt, func_wt;

      recur_wt = arc->weight;
      func_wt = arc->callee->weight;

      if (recur_wt <= 0.0)
	inl_wt = 0.0;
      else if (func_wt < 0.0)
	inl_wt = func_wt;
      else
	inl_wt = func_wt / (1.0 + func_wt / recur_wt);
    }

  return inl_wt;
}


void
PinCG_compute_total_weight (double *pwt_n, double *pwt_a)
{
  double wt_n = 0.0, wt_a = 0.0;
  PinCG_Func f;
  PinCG_Node n;
  PinCG_Arc a;

  List_start (Pin_callgraph->funcs);
  while ((f = (PinCG_Func) List_next (Pin_callgraph->funcs)))
    {
      wt_n = f->node->weight;
      List_start (f->clones);
      while ((n = (PinCG_Node) List_next (f->clones)))
	wt_n += n->weight;

      if (fabs (wt_n - f->weight) > 0.1)
	P_warn ("WEIGHT INCONSISTENCY");
    }

  List_start (Pin_callgraph->nodes);
  while ((n = (PinCG_Node) List_next (Pin_callgraph->nodes)))
    {
      wt_n += n->weight;
      List_start (n->arcs);
      while ((a = (PinCG_Arc) List_next (n->arcs)))
	{
	  wt_a += a->weight;

	  if (a->inlined && a->callee->owner != a->caller->owner)
	    P_warn ("OWNER MISMATCH");
	}
    }

  *pwt_n = wt_n;
  *pwt_a = wt_a;

  return;
}
