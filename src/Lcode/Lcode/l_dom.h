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

#ifndef _L_DOM_H
#define _L_DOM_H

#include <config.h>

/* Lengauer and Tarjan Fast Dominators */

typedef struct _LD_ControlNode {
  L_Cb *cb;
  L_Oper *opS, *opE;
  L_Operand *guard;

  List succ;   /* LD_ControlNode * */
  List pred;   /* LD_ControlNode * */

  List bucket; /* LD_ControlNode * */
  Set df;
  Set ridom;
  int dfn, size;

  struct _LD_ControlNode *lpred, *lsucc; /* layout previous, successor */

  /* Lengauer-Tarjan */

  int sdom;
  struct _LD_ControlNode *label, *ancestor, *parent;

  /* Dominator Tree */

  struct _LD_ControlNode *idom, *dtchi, *dtsib;

  /* Post-dominator Tree */

  struct _LD_ControlNode *ipdom, *pdtchi, *pdtsib;

  /* Unconditional "equivalence" node */

  struct _LD_ControlNode *uncond;

  Set live_in;

  int flags;
} LD_ControlNode;

#define LD_NODE_UNCOND          0x00000001

typedef struct _LD_Dom {
  int nodecnt;
  HashTable nhash;
} LD_Dom;

extern LD_Dom *LD_dom_analysis;

#define LDNODE(i) ((LD_ControlNode *) HashTable_find (LD_dom_analysis->nhash, (i)))

extern int LD_setup (L_Func *fn);
extern int LD_dominator (L_Func *fn);
extern Set LD_df_plus (Set nodes);

#endif
