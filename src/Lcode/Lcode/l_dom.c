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

#include <config.h>
#include <Lcode/l_main.h>
#include "l_ssa.h"
#include "l_dom.h"

#undef DEBUG_DOM

/* ========================================================================= */

LD_Dom *LD_dom_analysis = NULL;

static int cbrec_index;

static void
L_reset_visited_flag (L_Func * fn)
{
  L_Cb *cb;
  for (cb = fn->first_cb; cb != NULL; cb = cb->next_cb)
    cb->flags = L_CLR_BIT_FLAG (cb->flags, L_CB_VISITED);
}

static L_Alloc_Pool *LD_ControlNode_pool = NULL;

static void
LD_assign_dfn (LD_ControlNode *cn, LD_ControlNode *par, int index)
{
  cn->parent = par;
  HashTable_insert (LD_dom_analysis->nhash, index, cn);
  cn->sdom = cn->dfn = index;
}


static LD_ControlNode *
LD_new_control_node (L_Cb *cb, L_Oper *opS, L_Oper *opE, Set live_in, 
		     LD_ControlNode *un)
{
  LD_ControlNode *cn;

  cn = L_alloc (LD_ControlNode_pool);

  bzero (cn, sizeof (LD_ControlNode));

  cn->dfn = -1;

  cn->size = 1;
  cn->label = cn;
  cn->cb = cb;

  cn->opS = opS;
  cn->opE = opE;

  cn->flags = 0;

  if (opS->pred[0])
    {
      if (!un)
	L_punt ("No uncond equivalent in LD_new_control_node");

      cn->uncond = un;
    }
  else
    {
      cn->flags |= LD_NODE_UNCOND;
      cn->uncond = cn;
    }

  if (cn->opS && cn->opS->pred[0])
    cn->guard = L_copy_operand(cn->opS->pred[0]);
  else
    cn->guard = NULL;

  LD_dom_analysis->nodecnt++;

  cn->lsucc = cn->lpred = NULL;

  cn->idom = cn->dtchi = cn->dtsib = NULL;
  cn->ipdom = cn->pdtchi = cn->pdtsib = NULL;

  cn->live_in = live_in;

  return cn;
}


static void
LD_print_control_node_name (FILE *fp, LD_ControlNode *cn)
{
  fprintf (fp, "(%d cb %d %d:%d)", cn->dfn, cn->cb->id,
	   cn->opS->id, cn->opE->id);
}


void
LD_print_control_node (FILE *fp, LD_ControlNode *cn)
{
  LD_ControlNode *sn;

  LD_print_control_node_name (fp, cn);

  List_start (cn->succ);

  fprintf (fp, "PRED:");

  while ((sn = (LD_ControlNode *)List_next (cn->pred)))
    {
      LD_print_control_node_name (fp, sn);
    }

  List_start (cn->succ);

  fprintf (fp, "SUCC:");

  while ((sn = (LD_ControlNode *)List_next (cn->succ)))
    {
      LD_print_control_node_name (fp, sn);
    }

  fprintf (fp, "\n");
}


static LD_ControlNode *
LD_free_control_node (LD_ControlNode *cn)
{
  List_reset (cn->succ);
  List_reset (cn->pred);
  List_reset (cn->bucket);
  Set_dispose (cn->df);
  Set_dispose (cn->ridom);

  if (cn->guard)
    L_delete_operand (cn->guard);

  return NULL;
}


static void 
LD_cleanup_dom_analysis (void)
{
  LD_ControlNode *cn;

  if (!LD_dom_analysis)
    return;

  HashTable_start (LD_dom_analysis->nhash);

  while ((cn = (LD_ControlNode *)HashTable_next (LD_dom_analysis->nhash)))
    {
      LD_free_control_node (cn);
    }

  HashTable_free (LD_dom_analysis->nhash);

  free (LD_dom_analysis);
  LD_dom_analysis = NULL;
  return;
}


static void
LD_connect (LD_ControlNode *pn, LD_ControlNode *cn)
{
  pn->succ = List_insert_last (pn->succ, (void *)cn);
  cn->pred = List_insert_last (cn->pred, (void *)pn);
  return;
}

#define PNODE(p) ((p)?((p)->value.pred.ssa->node):PG_pred_graph->one)

static void
LD_visit (LD_ControlNode *par, LD_ControlNode *cn)
{
  LD_ControlNode *sn;

  LD_assign_dfn (cn, par, ++cbrec_index);

  while ((sn = List_next (cn->succ)))
    if (sn->dfn == -1)
      LD_visit (cn, sn);

  return;
}


static LD_ControlNode *
LD_process_cb (L_Cb *cb)
{
  L_Oper *op, *opn;
  LD_ControlNode *cn = NULL, *sn = NULL, *en = NULL, *un = NULL;
  int vcnt = 0;
  Set live_in;

  op = cb->first_op;

  /* Partition the ops into control nodes */

  if (!op)
    L_punt ("Missing expected oper in LD_process_cb");

  do
    {
      if (!cn)
	{
	  live_in = L_get_oper_IN_set (op);

	  cn = LD_new_control_node (cb, op, NULL, live_in, un);

	  if (!op->pred[0])
	    un = cn;

	  if (!sn)
	    {
	      sn = cn;
	    }
	  else
	    {
	      en->lsucc = cn;
	      cn->lpred = en;
	    }
	  en = cn;
	  vcnt++;
	}

      if (!(opn = op->next_op) || 
	  L_is_control_oper (op) ||
	  !PG_equivalent_predicates_ops (op, opn))
	{
	  cn->opE = op;
	  cn = NULL;
	}

      op = opn;
    }
  while (op);

#ifdef DEBUG_DOM
  if (vcnt != 1)
    fprintf (stderr,"%d nodes in func %s cb %d\n",
	     vcnt, L_fn->name, cb->id);
#endif

  cb->ext = (void *)sn;

  return sn;
}


static void
LD_connect_cb (L_Cb *cb)
{
  LD_ControlNode *an, *bn, *en = NULL, *fn;
  L_Flow *fl, *ffl;
  PG_Pred_Graph *pg = PG_pred_graph;
  DdManager *dd = pg->dd;

  /* Connect the control nodes */

  fl = cb->dest_flow;

  if (L_has_fallthru_to_next_cb (cb) && fl &&
      (ffl = L_find_last_flow (fl)))
    fn = ffl->dst_cb->ext;
  else
    fn = NULL;

  for (an = (LD_ControlNode *) cb->ext; an; an = an->lsucc)
    {
      DdNode *fv, *pv, *sv;
      L_Oper *op;

      en = an;

      fv = PNODE(an->guard);

      Cudd_Ref (fv);

      for (op = an->opS; op; op = op->next_op)
	{
	  if (L_uncond_branch_opcode (op))
	    {
	      LD_connect (an, (LD_ControlNode *)fl->dst_cb->ext);
	      fl = fl->next_flow;
	      pv = fv;
	      fv = Cudd_bddAnd (dd, fv, Cudd_Not (PNODE(op->pred[0])));
	      Cudd_Ref(fv);
	      Cudd_RecursiveDeref (dd, pv);
	    }
	  else if L_cond_branch_opcode (op)
	    {
	      LD_connect (an, (LD_ControlNode *)fl->dst_cb->ext);
	      fl = fl->next_flow;
	    }
	  else if (L_register_branch_opcode (op))
	    {
	      while (fl)
		{
		  LD_connect (an, (LD_ControlNode *)fl->dst_cb->ext);
		  fl = fl->next_flow;
		}
	      pv = fv;
	      fv = Cudd_bddAnd (dd, fv, Cudd_Not (PNODE(op->pred[0])));
	      Cudd_Ref(fv);
	      Cudd_RecursiveDeref (dd, pv);
	    }
	  else if (L_check_branch_opcode (op))
	    {
	      LD_connect (an, (LD_ControlNode *)fl->dst_cb->ext);
	      fl = fl->next_flow;
	    }

	  if (op == an->opE)
	    break;
	}

      for (bn = an->lsucc; bn && (fv != pg->zero); bn = bn->lsucc)
	{
	  op = bn->opS;
	  sv = PNODE(op->pred[0]);

	  if (Cudd_bddLeq (dd, fv, Cudd_Not (sv)))
	    continue;

	  LD_connect (an, bn);

	  pv = fv;
	  fv = Cudd_bddAnd (dd, fv, Cudd_Not (sv));
	  Cudd_Ref(fv);
	  Cudd_RecursiveDeref (dd, pv);
	}

      if ((fv != pg->zero) && fn)
	LD_connect (an, fn);

      Cudd_RecursiveDeref (dd, fv);
    }

  return;
}


static int
LD_setup_cnodes (L_Func * fn)
{
  L_Cb *cb;
#ifdef DEBUG_DOM
  LD_ControlNode *cn;
#endif
  
  if (LD_dom_analysis)
    LD_cleanup_dom_analysis ();

  LD_dom_analysis = malloc (sizeof (LD_Dom));
  LD_dom_analysis->nodecnt = 0;
  LD_dom_analysis->nhash = HashTable_create (1024);

  cbrec_index = 0;

  /*
   *  visit each block.
   */

  L_reset_visited_flag (fn);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    LD_process_cb (cb);

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    LD_connect_cb (cb);

  LD_visit (NULL, fn->first_cb->ext);

#ifdef DEBUG_DOM
  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (cn = cb->ext; cn; cn = cn->lsucc)
      LD_print_control_node (stderr, cn);
#endif

  return LD_dom_analysis->nodecnt;
}


/* ========================================================================= */


static void
LD_compress (LD_ControlNode *v)
{
  LD_ControlNode *a;
  if ((a = v->ancestor) && a->ancestor)
    {
      int sl, sla;
      LD_compress (a);

      sl = v->label ? v->label->sdom : 0;
      sla = a->label ? a->label->sdom : 0;

      if (sla < sl)
	v->label = a->label;

      v->ancestor = v->ancestor ? 
	v->ancestor->ancestor : NULL;
    }
}


static LD_ControlNode *
LD_eval (LD_ControlNode *v)
{
  if (!v->ancestor)
    {
      return v->label;
    }
  else
    {
      LD_ControlNode *a;
      int sl, sla;
      LD_compress (v);

      a = v->ancestor;

      sl = v->label ? v->label->sdom : 0;
      sla = (a && a->label) ? a->label->sdom : 0;

      return (sla >= sl) ? v->label : a->label;
    }
}


static void
LD_print_domtree (LD_ControlNode *n, int nl)
{
  int i;

  if (!n)
    return;

  if (n->dtsib)
    LD_print_domtree (n->dtsib, nl);

  for (i = 0; i < nl; i++)
    printf ("  ");

  printf ("%d\n", n->cb->id);

  if (n->dtchi)
    LD_print_domtree (n->dtchi, nl + 1);

  return;
}


static void
LD_pod_visit (List *pod, LD_ControlNode *node)
{
  if (node->dtchi)
    LD_pod_visit (pod, node->dtchi);

  *pod = List_insert_last (*pod, (void *) node);

  if (node->dtsib)
    LD_pod_visit (pod, node->dtsib);
}


static Set
LD_df_set (Set nodes)
{
  Set df = NULL;
  int i, sz, *buf;
  LD_ControlNode *cr;

  sz = Set_size (nodes);
  buf = alloca (sz * sizeof (int));

  Set_2array(nodes, buf);

  for (i = 0; i < sz; i++)
    {
      cr = LDNODE(buf[i]);
      df = Set_union_acc (df, cr->df);
    }
  
  return df;
}


Set
LD_df_plus (Set nodes)
{
  Set dfp, dfpn, dfpu;
  int change;

  dfp = LD_df_set (nodes);

  do
    {
      dfpu = Set_union (dfp, nodes);
      dfpn = LD_df_set (dfpu);
      Set_dispose (dfpu);
      change = !Set_same (dfp, dfpn);
      Set_dispose (dfp);
      dfp = dfpn;
    }
  while (change);

  return dfp;
}


static int
LD_dom_front (void)
{
  List pod = NULL;
  int *buf, nodecnt = LD_dom_analysis->nodecnt;
  LD_ControlNode *u, *v, *w;

  buf = alloca (nodecnt * sizeof (int));

  LD_pod_visit (&pod, LDNODE(1));

  /* pod contains postorder traversal of dom tree */

  while ((v = (LD_ControlNode *) List_next (pod)))
    {
      /* local */

      List_start (v->succ);
      while ((u = (LD_ControlNode *)List_next (v->succ)))
	if (!Set_in (v->ridom, u->dfn))
	  v->df = Set_add (v->df, u->dfn);

      /* up */

      u = v->dtchi;

      while (u)
	{
	  int cnt, j;
	  cnt = Set_2array (u->df, buf);

	  for (j = 0; j < cnt; j++)
	    {
	      w = LDNODE(buf[j]);

	      if (!Set_in (v->ridom, w->dfn))
		v->df = Set_add (v->df, buf[j]);
	    }

	  u = u->dtsib;
	}

      pod = List_delete_current (pod);
    }

  return 0;
}


static int
LD_remove_unreachable (L_Func *fn)
{
  L_Cb *cb;
  L_Oper *opA, *opN;
  LD_ControlNode *cn;
  int cnt = 0, warned = 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    for (cn = cb->ext; cn; cn = cn->lsucc)
      {
	if (cn->dfn != -1)
	  continue;

	/* Control node is unreachable */

	if (!warned++)
	  L_warn ("Dom analysis found unreachable code in %s()", fn->name);

	for (opA = cn->opS; opA; opA = opN)
	  {
	    opN = opA->next_op;
	    DB_print_oper (opA);
	    L_delete_complete_oper (cb, opA);
	    cnt++;

	    if (opA == cn->opE)
	      break;
	  }
	cn->opS = NULL;
	cn->opE = NULL;
      }

  if (warned)
    {
      cnt += L_delete_unreachable_blocks (fn);
    }

  return cnt;
}


int
LD_setup (L_Func *fn)
{
  int nodecnt;

  if (!LD_ControlNode_pool)
    LD_ControlNode_pool = L_create_alloc_pool ("LD_ControlNode",
					       sizeof (LD_ControlNode),
					       512);
  do
    {
      nodecnt = LD_setup_cnodes (fn);
    }
  while (LD_remove_unreachable (fn));

  return nodecnt;
}


static void
LD_link (LD_ControlNode *v, LD_ControlNode *w)
{
  /* height balancing should occur here */
  w->ancestor = v;
  return;
}


int
LD_dominator (L_Func *fn)
{
  LD_ControlNode *w, *v, *u, *p;
  int nodecnt, i;

  nodecnt = LD_setup (fn);

  /* node[0] is empty; node[1] is root */

  for (i = nodecnt; i > 1; i--)
    {
      int slu;
      
      w = LDNODE(i);

      List_start (w->pred);
      while ((v = (LD_ControlNode *)List_next (w->pred)))
	{
	  u = LD_eval (v);

	  slu = u->sdom;

	  if (slu < w->sdom)
	    w->sdom = slu;
	}

      v = LDNODE(w->sdom);
      v->bucket = List_insert_last (v->bucket, (void *) w);

      LD_link (w->parent, w);

      p = w->parent;

      while ((v = List_first (p->bucket)))
	{
	  p->bucket = List_delete_current (p->bucket);
	  u = LD_eval (v);

	  v->idom = (u->sdom < v->sdom) ? u : p;
	}
    }

  for (i = 2; i <= nodecnt; i++)
    {
      w = LDNODE(i);
      
      if (w->idom != LDNODE(w->sdom))
	w->idom = w->idom->idom;

      w->idom->ridom = Set_add (w->idom->ridom, w->dfn);

      if (!w->idom->dtchi)
	{
	  w->idom->dtchi = w;
	}
      else
	{
	  v = w->idom->dtchi;
	  w->idom->dtchi = w;
	  w->dtsib = v;
	}
    }

  LD_dom_front ();

#ifdef DEBUG_DOM
  printf ("FUNC %s\n", fn->name);

  {
    L_Cb *cb;
    for (cb = fn->first_cb; cb; cb = cb->next_cb)
      {
	LD_ControlNode *n, *idom;
	int id, idd, idn;

	id = cb->id;
	n = (LD_ControlNode *)(cb->ext);
	idom = n->idom;
	idn = n->dfn;
	idd = idom ? idom->dfn : -1;

	printf ("   cb %d(dfn %d) idom dfn %d sdom dfn %d", id, idn, idd, n->sdom);
	Set_print (stdout, "DF", n->df);

      }
  }

  LD_print_domtree (LDNODE(1), 0);
#endif

  return 0;
}

/* ------------------------------------------------------------------------ */

