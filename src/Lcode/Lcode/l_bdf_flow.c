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
#include "l_bdf_graph.h"
#include "l_bdf_cnt_dep.h"

extern BDF_Graph *BDF_graph;

PRED_FLOW *fg = NULL;

void BDF_flow_live_variable (BDF_Graph *g);
void L_bdf_print_dataflow (L_Func * fn);

void
L_do_new_flow_analysis (L_Func *fn, int mode)
{
  PG_setup_pred_graph (fn);

  printf ("DD nodes: %ld\n", Cudd_ReadNodeCount(PG_pred_graph->dd));

  /* Set up the Boolean Dataflow Graph */

  L_start_time (&L_module_global_dataflow_time);

  BDF_graph = BDF_new_graph (fn);

  printf ("DD nodes: %ld\n", Cudd_ReadNodeCount(PG_pred_graph->dd));

  if (mode &
      (DOMINATOR | POST_DOMINATOR |
       DOMINATOR_CB | POST_DOMINATOR_CB |
       DOMINATOR_INT | POST_DOMINATOR_INT |
       MEM_REACHING_DEFINITION | MEM_AVAILABLE_DEFINITION))
    BDF_flow_dom_pdom (BDF_graph, mode);

  if (mode & LIVE_VARIABLE)
    BDF_flow_live_variable (BDF_graph);

#if 0
  if (mode & REACHING_DEFINITION)
    BDF_flow_reaching_def (BDF_graph);

  if (mode & AVAILABLE_DEFINITION)
    BDF_flow_avail_def (BDF_graph);

  if (mode & AVAILABLE_EXPRESSION)
    BDF_flow_avail_expr (BDF_graph);

  if (mode & CRITICAL_VARIABLE)
    BDF_flow_crit_variable (BDF_graph, mode);
#endif

  L_stop_time (&L_module_global_dataflow_time);

  L_bdf_print_dataflow (BDF_graph->fn);

  return;
}

static void
BDF_dispose_fhash (HashTable h)
{
  PG_Pred_Graph *pg = PG_pred_graph;
  DdNode *n;

  HashTable_start (h);

  while ((n = HashTable_next (h)))
    Cudd_RecursiveDeref (pg->dd, n);

  HashTable_reset (h);
}

#define BDF_ACT_ADD 0x00000001
#define BDF_ACT_SUB 0x00000002

void
BDF_table_update_rmid (HashTable vtbl, Set vvec,
		       DdNode *f_a, BDF_Operand *bo,
		       int action)
{
  DdNode *f_v, *f_n = NULL;
  PG_Pred_Graph *pg = PG_pred_graph;
  DdNode *f_1 = pg->one, *f_0 = pg->zero;
  int i = bo->reg;

  if (!(f_v = (DdNode *) HashTable_find_or_null (vtbl, i)))
    f_v = f_0;
  f_a = (bo->flags & BDF_OPD_UNCOND) ? f_1 : f_a;
  switch (action)
    {
    case BDF_ACT_ADD:
      f_n = Cudd_bddIte (pg->dd, f_a, f_0, f_v);
      if (f_a != f_0)
	vvec = Set_add (vvec, i);
      break;
    case BDF_ACT_SUB:
      f_n = Cudd_bddIte (pg->dd, f_a, f_0, f_v);
      if (f_a != f_0)
	vvec = Set_delete (vvec, i);
      break;
    default:
      L_punt ("BDF_update_table_rmid: bad action");
    }
  Cudd_Ref (f_n);
  Cudd_RecursiveDeref (pg->dd, f_v);
  HashTable_update (vtbl, i, f_n);
}


void
BDF_add_operands_excl (HashTable itbl, HashTable etbl, DdNode *f_p, 
		       List opdlist)
{
  BDF_Operand *opd;
  DdNode *f_1, *f_0, *f_e, *f_i, *f_t, *f_a;
  int idx;
  PG_Pred_Graph *pg = PG_pred_graph;

  f_0 = pg->zero;
  f_1 = pg->one;

  BDF_FOREACH_OPERAND (opd, opdlist)
    {
      idx = opd->reg;

      if (!(f_i = (DdNode *)HashTable_find_or_null (itbl, idx)))
	f_i = f_0;

      if (!(f_e = (DdNode *)HashTable_find_or_null (etbl, idx)))
	f_e = f_0;

      f_a = (opd->flags & BDF_OPD_UNCOND) ? f_1 : f_p;

      /* f_i(k) = (!f_e(k) && f_a) || f_i(k) */
      
      f_a = Cudd_bddIte (pg->dd, f_e, f_0, f_a);
      Cudd_Ref (f_a);
      f_t = Cudd_bddIte (pg->dd, f_i, f_1, f_a);
      Cudd_Ref (f_t);
      Cudd_RecursiveDeref (pg->dd, f_a);
      Cudd_RecursiveDeref (pg->dd, f_i);
      f_i = f_t;
      HashTable_update (itbl, idx, f_i);
    }

  return;
}


/*
 * gen_set_sup
 * ----------------------------------------------------------------------
 * Set an element if h(k) >= f
 */

Set
gen_set_sup (HashTable h, DdNode *f)
{
  DdNode *h_k;
  PG_Pred_Graph *pg = PG_pred_graph;
  Set s = NULL;

  HashTable_start (h);
  while ((h_k = (DdNode *) HashTable_next (h)))
    {
      int k = HashTable_key(h);
      if (Cudd_bddLeq (pg->dd, f, h_k))
	s = Set_add (s, k);
    }
  return s;
}


/*
 * gen_set_isect
 * ----------------------------------------------------------------------
 * Set an element if h(k) & f != 0
 */

Set
gen_set_isect (HashTable h, DdNode *f)
{
  DdNode *h_k, *f_0, *f_1;
  PG_Pred_Graph *pg = PG_pred_graph;
  Set s = NULL;

  f_0 = pg->zero;
  f_1 = pg->one;

  HashTable_start (h);
  while ((h_k = (DdNode *) HashTable_next (h)))
    {
      int k = HashTable_key(h);
      if (!Cudd_bddLeq (pg->dd, h_k, Cudd_Not(f)))
	s = Set_add (s, k);
    }
  return s;
}


/*
 * build_gk_up
 * ----------------------------------------------------------------------
 * Build gen/kill sets for live variable analysis
 */
void
build_gk_up (BDF_Node *n)
{
  HashTable gtbl, ktbl;
  BDF_Oper *bop; 
  BDF_Arc *curr_exit;
  PG_Pred_Graph *pg;

  pg = PG_pred_graph;

  /* Alloc g/k sets */

  gtbl = HashTable_create (256);
  ktbl = HashTable_create (256);

  /* Build upward from each exit */

  List_start (n->exit);
  curr_exit = (BDF_Arc *)List_next (n->exit);

  BDF_FOREACH_OPER (bop, n->oper)
    {
      BDF_add_operands_excl (gtbl, ktbl, bop->pfunc, bop->src);
      BDF_add_operands_excl (ktbl, gtbl, bop->pfunc, bop->dest);
      while (curr_exit && (bop == curr_exit->op))
	{
	  curr_exit->info.k = gen_set_sup (ktbl, curr_exit->pfunc);
	  curr_exit = (BDF_Arc *)List_next (n->exit);
	}
    }

  if (curr_exit) /* fallthrough path */
    curr_exit->info.k = gen_set_sup (ktbl, curr_exit->pfunc);

  n->info.g = gen_set_isect (gtbl, pg->one);

  BDF_dispose_fhash (gtbl);
  HashTable_free (gtbl);
  BDF_dispose_fhash (ktbl);
  HashTable_free (ktbl);
  return;
}


int
BDF_prop_up (BDF_Node *n)
{
  BDF_Arc *exit;
  BDF_Node *succ;
  Set new_in = NULL;

  BDF_FOREACH_ARC (exit, n->exit)
    {
      Set path = NULL;

      succ = exit->succ;

      path = Set_union_acc (path, succ->info.v_in);
      path = Set_subtract_acc (path, exit->info.k);
      new_in = Set_union_acc (new_in, path);
      path = Set_dispose (path);
    }

  new_in = Set_union_acc (new_in, n->info.g);

  if (!Set_subtract_empty (new_in, n->info.v_in))
    {
      Set_dispose (n->info.v_in);
      n->info.v_in = new_in;
      return 1;
    }
  else
    {
      Set_dispose (new_in);
      return 0;
    }  
}


void
BDF_func_or_set (PG_Pred_Graph *pg, HashTable vtbl, DdNode *f, Set s)
{
  DdNode *vfunc, *tfunc;
  int *vbuf;
  int size, i;

  if (!s)
    return;

  size = Set_size (s);
  vbuf = Lcode_malloc (size * sizeof (int));
  size = Set_2array (s, vbuf);

  for (i = 0; i < size; i++)
    {
      if (!(vfunc = (DdNode *)HashTable_find_or_null (vtbl, vbuf[i])))
	vfunc = pg->zero;

      tfunc = Cudd_bddIte (pg->dd, f, pg->one, vfunc);
      Cudd_Ref (tfunc);
      Cudd_RecursiveDeref (pg->dd, vfunc);
      vfunc = tfunc;
      HashTable_update (vtbl, vbuf[i], vfunc);
    }

  Lcode_free (vbuf);
  return;
}


void
BDF_local_live_variable (HashTable vtbl, BDF_Node *n)
{
  BDF_Oper *bop; 
  BDF_Arc *curr_exit;
  DdNode *f_1, *f_0;
  PG_Pred_Graph *pg;
  Set vvec = NULL;
  pg = PG_pred_graph;
  f_0 = pg->zero;
  f_1 = pg->one;

  /* Build upward from each exit */

  List_start (n->exit);
  curr_exit = (BDF_Arc *) List_prev (n->exit);

  /* handle fallthrough flows */
  
  if (curr_exit && !curr_exit->op)
    {
      BDF_func_or_set (pg, vtbl, curr_exit->pfunc, 
		       curr_exit->succ->info.v_in); 
      vvec = Set_copy (curr_exit->succ->info.v_in);
      curr_exit = (BDF_Arc *)List_prev (n->exit);
    }

  BDF_FORHCAE_OPER (bop, n->oper)
    {
      BDF_Operand *bd;

      if (bop->pfunc != f_1)
	bop->info.v_out = gen_set_isect (vtbl, bop->pfunc);
      else
	bop->info.v_out = Set_copy (vvec);

      while (curr_exit && (bop == curr_exit->op))
	{
	  curr_exit->info.v_out = Set_copy (curr_exit->succ->info.v_in);
	  BDF_func_or_set (pg, vtbl, curr_exit->pfunc, curr_exit->info.v_out);
	  vvec = Set_union_acc (vvec, curr_exit->info.v_out);
	  curr_exit = (BDF_Arc *) List_prev (n->exit);
	}

      BDF_FOREACH_OPERAND (bd, bop->dest)
	BDF_table_update_rmid (vtbl, vvec, bop->pfunc, bd, BDF_ACT_SUB);
      BDF_FOREACH_OPERAND (bd, bop->src)
	BDF_table_update_rmid (vtbl, vvec, bop->pfunc, bd, BDF_ACT_ADD);

      if (bop->pfunc != f_1)
	bop->info.v_in = gen_set_isect (vtbl, bop->pfunc);
      else
	bop->info.v_in = Set_copy (vvec);
    }

  BDF_dispose_fhash (vtbl);
  Set_dispose (vvec);

  return;
}


void
BDF_flow_live_variable (BDF_Graph *g)
{
  BDF_Node *cb;
  HashTable vtbl;
  int change;

  /* I: G/K sets */

  BDF_FOREACH_CB (cb, g->cb)
    {
      build_gk_up (cb);
      if (cb->info.v_in)
	cb->info.v_in = Set_dispose (cb->info.v_in);
      cb->info.v_in = Set_copy (cb->info.g);
    }

  /* II: Global propagation */

  do
    {
      change = 0;
      BDF_FORHCAE_CB (cb, g->cb)
	change += BDF_prop_up (cb);
      printf ("Round: %d changes\n", change);
    }
  while (change);

  /* III: Instr dataflow */

  vtbl = HashTable_create (256);

  BDF_FOREACH_CB (cb, g->cb)
    {
      BDF_local_live_variable (vtbl, cb);
    }

  HashTable_free(vtbl);

  printf ("DD nodes: %ld\n", Cudd_ReadNodeCount(PG_pred_graph->dd));

  return;
}


void
L_bdf_print_dataflow (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper;
  Set iset, ofset;
  BDF_Graph *bg;
  BDF_Node *bn;
  BDF_Arc *ba;
  BDF_Oper *bo;
  bg = BDF_graph;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      bn = HashTable_find (bg->hash_cb_dfcb, cb->id);

      List_start (bn->exit);
      ba = List_next (bn->exit);
      printf(">>> (cb %d) :\n", cb->id);
      Set_print (stdout, "GEN", bn->info.g);
      Set_print (stdout, "V_IN", bn->info.v_in);
      for (oper = cb->first_op; oper; oper = oper->next_op)
	{
	  BDF_Operand *bd;
	  bo = HashTable_find (bg->hash_op_dfop, oper->id);

	  iset = bo->info.v_in;
	  Set_print (stdout, "V_IN ", iset);
	  L_print_oper (stdout, oper);
	  printf("(dest");
	  BDF_FOREACH_OPERAND (bd, bo->dest)
	    {
	      printf(" %d", bd->reg);
	      if (bd->flags & BDF_OPD_UNCOND)
		printf("U");
	    }
	  printf(") (src");
	  BDF_FOREACH_OPERAND (bd, bo->src)
	    {
	      printf(" %d", bd->reg);
	      if (bd->flags & BDF_OPD_UNCOND)
		printf("U");
	    }
	  printf(")\n");
	  ofset = bo->info.v_out;
	  Set_print (stdout, "V_OUT (FT) ", ofset);

	  while (ba && (ba->op == bo))
	    {
	      Set_print (stdout, "V_OUT (TK) ", ba->info.v_out);
	      Set_print (stdout, "S-KILL", ba->info.k);
	      ba = List_next (bn->exit);
	    }
	}

      if (ba)
	Set_print (stdout, "F-KILL", ba->info.k);

    }
}
