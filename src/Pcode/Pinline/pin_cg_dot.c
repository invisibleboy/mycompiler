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
/* 07/10/02 REK Including string.h */
#include <string.h>
/* 07/10/02 REK Including sys/param.h to get MAXPATHLEN */
#include <sys/param.h>
#include <library/list.h>
#include <library/l_parms.h>
#include <library/c_symbol.h>
#include <machine/m_spec.h>
#include <Pcode/pcode.h>
#include <Pcode/parms.h>
#include <Pcode/cast.h>
#include <math.h>
#include <library/dynamic_symbol.h>
#include "pin_inline.h"

static double PinCG_scal;

static void
hsbcolor (double wt, double *ph, double *ps, double *pb)
{
  double h, s, b;
  if (wt < 0.0)
    wt = 0.0;
  if (wt < 1.0)
    {
      h = 0.00;
      s = 0.00;
      b = 1.00;
    }
  else
    {
      s = 0.85;
      b = 0.85;
      h = 0.66 - log (wt) * PinCG_scal;

      if (h < 0.0)
	h = 0.0;
      else if (h > 0.66)
	h = 0.66;

      s = 0.8;
    }

  *ph = h;
  *ps = s;
  *pb = b;
  return;
}


static void
PinCG_dot_node (FILE * Fgr, PinCG_Node n, int leaf)
{
  double h, s, b, wt;
  wt = n->weight;
  hsbcolor (wt, &h, &s, &b);

  fprintf (Fgr, "\t\"%d\" [shape=%s,style=filled,"
	   "label=\"%s()\\n%0.3g\\n%de/%dt ops\","
	   "color=\"%0.3f %0.3f %0.3f\"];\n",
	   n->id, leaf ? "ellipse" : "box", n->func->funcname,
	   wt, (int) n->func->o_ebodysize,
	   (int) n->func->o_bodysize, h, s, b);
  return;
}

static void
PinCG_dot_arc (FILE * Fgr, PinCG_Arc a)
{
  char *col, *sty;
  double awt, wt;
  wt = a->weight;
  col = !a->indirect ? "black" : "red";
  sty = (wt == 0.0) ? "dashed" : "solid";
  if (a->inlined)
    {
      if (a->callee->func == a->caller->func)
	sty = "bold,arrowType=box";
      else
	sty = "bold";
    }

  awt = 1000 * log ((wt >= 1.0) ? wt : 1.0);

  fprintf (Fgr, "\t\"%d\" -> \"%d\" "
	   "[label=\"%0.3g\",color=%s,style=%s,weight=%0.0f];\n",
	   a->caller->id, a->callee->id, wt, col, sty, awt);
}

static Set PinCG_nodes_dumped = NULL;

static void
PinCG_dot_inl_subgraph_nodes (FILE * Fgr, PinCG_Node n)
{
  PinCG_Arc a;
  PinCG_Node n2;
  PinCG_nodes_dumped = Set_add (PinCG_nodes_dumped, n->id);
  PinCG_dot_node (Fgr, n, 0);
  List_start (n->arcs);
  while ((a = (PinCG_Arc) List_next (n->arcs)))
    {
      n2 = a->callee;
      if (Set_in (PinCG_nodes_dumped, n2->id))
	continue;
      PinCG_nodes_dumped = Set_add (PinCG_nodes_dumped, n2->id);
      if (a->inlined)
	PinCG_dot_inl_subgraph_nodes (Fgr, n2);
      else
	PinCG_dot_node (Fgr, n2, 1);
    }
  return;
}


static void
PinCG_dot_inl_subgraph_arcs (FILE * Fgr, PinCG_Node n)
{
  PinCG_Arc a;

  List_start (n->arcs);
  while ((a = (PinCG_Arc) List_next (n->arcs)))
    {
      PinCG_dot_arc (Fgr, a);
      if (a->inlined)
	PinCG_dot_inl_subgraph_arcs (Fgr, a->callee);
    }
}


void
PinCG_dot_inlining_graph (PinCG_Func f, char *file, double maxwt)
{
  FILE *Fgr;

  if (!(Fgr = fopen (file, "w")))
    P_punt ("Dump_PinCG_Graph_dot: unable to open output file");

  fprintf (Fgr, "digraph G {\n" "\tsize=\"7.5,10\"\n");

  PinCG_scal = (maxwt > 0.0) ? 0.66 / log (maxwt) : 1.0;

  PinCG_dot_inl_subgraph_nodes (Fgr, f->node);
  PinCG_dot_inl_subgraph_arcs (Fgr, f->node);

  fprintf (Fgr, "}\n");
  fclose (Fgr);
  PinCG_nodes_dumped = Set_dispose (PinCG_nodes_dumped);
  return;
}


void
PinCG_dot_inlining_graphs (void)
{
  char filename[256];
  PinCG_Func f;
  PinCG_Node n;
  double maxwt = 0.0;

  List_start (Pin_callgraph->nodes);
  while ((n = (PinCG_Node) List_next (Pin_callgraph->nodes)))
    {
      if (n->weight > maxwt)
	maxwt = n->weight;
    }

  List_start (Pin_callgraph->funcs);
  while ((f = (PinCG_Func) List_next (Pin_callgraph->funcs)))
    {
      sprintf (filename, "%s.%s.inl.dot", f->orig_filename, f->funcname);
      PinCG_dot_inlining_graph (f, filename, maxwt);
    }
  return;
}


void
PinCG_dot_callgraph (char *filename)
{
  FILE *Fgr;
  PinCG_Node n;
  PinCG_Arc a;
  double maxwt = 0.0, wt, scal;

  if (!(Fgr = fopen (filename, "w")))
    P_punt ("Dump_PinCG_Graph_dot: unable to open output file");

  fprintf (Fgr, "digraph G {\n" "\tsize=\"7.5,10\"\n");

  List_start (Pin_callgraph->nodes);
  while ((n = (PinCG_Node) List_next (Pin_callgraph->nodes)))
    {
      if (n->weight > maxwt)
	maxwt = n->weight;
    }

  scal = (maxwt > 0.0) ? 0.66 / log (maxwt) : 1.0;

  List_start (Pin_callgraph->nodes);
  while ((n = (PinCG_Node) List_next (Pin_callgraph->nodes)))
    {
      double h, s, b;
      wt = n->weight;
      if (wt < 0.0)
	wt = 0.0;
      if (wt < 1.0)
	{
	  h = 0.00;
	  s = 0.00;
	  b = 1.00;
	}
      else
	{
	  s = 0.85;
	  b = 0.85;
	  h = 0.66 - log (wt) * scal;

	  if (h < 0.0)
	    h = 0.0;
	  else if (h > 0.66)
	    h = 0.66;

	  s = 0.8;
	}

      fprintf (Fgr, "\t\"%s\" [shape=box,style=filled,"
	       "label=\"%s()\\n%0.3g\\n%d/%d ops\","
	       "color=\"%0.3f %0.3f %0.3f\"];\n",
	       n->func->funcname, n->func->funcname,
	       wt, (int) n->func->o_ebodysize,
	       (int) n->func->o_bodysize, h, s, b);
    }

  List_start (Pin_callgraph->nodes);
  while ((n = (PinCG_Node) List_next (Pin_callgraph->nodes)))
    {
      List_start (n->arcs);
      while ((a = (PinCG_Arc) List_next (n->arcs)))
	{
	  char *col, *sty;
	  double awt;
	  wt = a->weight;
	  col = !a->indirect ? "black" : "red";
	  sty = (wt == 0.0) ? "dashed" : "solid";

	  awt = 1000 * log ((wt >= 1.0) ? wt : 1.0);

	  fprintf (Fgr, "\t\"%s\" -> \"%s\" "
		   "[label=\"%0.3g\",color=%s,style=%s,weight=%0.0f];\n",
		   n->func->funcname, a->callee->func->funcname, wt,
		   col, sty, awt);
	}
    }

  fprintf (Fgr, "}\n");

  fclose (Fgr);
  return;
}
