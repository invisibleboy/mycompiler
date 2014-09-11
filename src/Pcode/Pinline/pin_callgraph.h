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
 *      File:   pin_callgraph.h
 *      Author: Ben-Chung Cheng
 *      Copyright (c) 1995 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#ifndef PIN_CALLGRAPH_H
#define PIN_CALLGRAPH_H

#include <library/i_list.h>

struct _PinCG_Graph;
struct _PinCG_Func;
struct _PinCG_Node;
struct _PinCG_Arc;

typedef struct _PinCG_Func
{
  int id;			/* function id */
  Key key;
  int num_parms;		/* number of parameters */

  /* Body size (AST nodes), original and inlined */
  long int o_bodysize, i_bodysize;
  /* Executed body size (AST nodes), original and inlined */
  long int o_ebodysize, i_ebodysize;
  /* Activation record size (bytes), original and inlined */
  long int o_stacksize, i_stacksize;

  double weight;		/* Total profile weight of the function
				 * (1.0 if size_only)
				 */
  double indir_weight;		/* Function weight not accounted for by
				 * detected arcs is assumed to result from
				 * indirect invocations.
				 */

  char *o_path;			/* the path of the original version */
  char *i_path;			/* the path of the expanded version */
  char *funcname;		/* the function name */
  char *filename;
  char *orig_filename;		/* the file name */
  int is_vararg:1;		/* is vararg */
  int is_empty_func:1;		/* is empty func */
  int is_noninline:1;		/* don't inline this function */
  int is_always_inline:1;
  int self_inlined:1;

  struct _PinCG_Node *node;

  List clones;
} *PinCG_Func;


typedef struct _PinCG_Node
{
  int id;
  double weight, u_weight;
  struct _PinCG_Func *func;	/* Function this node represents */
  struct _PinCG_Func *owner;	/* Function body to which this node belongs */
  List arcs;			/* PinCG_Arc: outgoing arcs */
} *PinCG_Node;


typedef struct _PinCG_Arc
{
  int id;			/* unique ID */
  int callsite_id;              /* ID of the caller's OP_call Expr */
  struct _PinCG_Node *caller;	/* caller node */
  struct _PinCG_Node *callee;	/* callee node */

  double weight;		/* frequency of src node jsr call */
  double r_an;			/* ratio of arc weight to node weight */

  struct _PinCG_Arc *parent;	/* arc of which arc is a copy */

  int num_parms;

  unsigned int natural:1;	/* arc is part of original callgraph */
  unsigned int indirect:1;	/* arc is indirect */
  unsigned int noinline:1;	/* arc is marked for no inlining */
  unsigned int use_orig:1;	/* use original body for inlining */
  unsigned int inlined:1;	/* arc represents an inline expansion */
} *PinCG_Arc;


typedef struct _PinCG_Graph
{
  struct _PinCG_Func *root_func;
  struct _PinCG_Node *root_node;
  List funcs;			/* PinCG_Func: function records */
  List nodes;			/* PinCG_Node: node records */
} *PinCG_Graph;


extern PinCG_Graph Pin_callgraph;

extern PinCG_Func PinCG_create_func (char *, Key);
extern PinCG_Node PinCG_create_node (PinCG_Func, PinCG_Func);
extern void PinCG_create_graph (void);
extern PinCG_Arc PinCG_create_arc (PinCG_Node caller, PinCG_Node callee,
				   int callsite_id, double weight);
extern PinCG_Arc PinCG_derive_arc (PinCG_Arc);
extern double PinCG_compute_arc_key (PinCG_Arc);
extern double PinCG_inline_weight (PinCG_Arc arc);

extern double PinCG_inline (PinCG_Arc inlarc);
extern void PinCG_compute_total_weight (double *, double *);

extern void PinCG_dot_callgraph (char *filename);
extern void PinCG_dot_inlining_graph (PinCG_Func, char *, double);
extern void PinCG_dot_inlining_graphs (void);

extern void Dump_PinCG_Node (FILE * F, PinCG_Node n);
extern void Dump_PinCG_Arc (FILE * F, PinCG_Arc c);

#endif
