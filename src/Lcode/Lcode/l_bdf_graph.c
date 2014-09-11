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

/* Memory allocation pools */

L_Alloc_Pool *BDF_alloc_operand = NULL;
L_Alloc_Pool *BDF_alloc_oper = NULL;
L_Alloc_Pool *BDF_alloc_arc = NULL;
L_Alloc_Pool *BDF_alloc_node = NULL;
L_Alloc_Pool *BDF_alloc_graph = NULL;

BDF_Graph *BDF_graph = NULL;

#define SETDISPOSE(a) if (a) (a) = Set_dispose (a)

static BDF_Oper *BDF_delete_oper (BDF_Oper *bop);
static BDF_Operand *BDF_delete_operand (BDF_Operand *bopd);

/*
 * BDF_initialize
 * ----------------------------------------------------------------------
 * Set up allocation pools
 */

void
BDF_initialize ()
{
  if (!BDF_alloc_graph)
    BDF_alloc_graph = L_create_alloc_pool ("BDF_Graph",  
					   sizeof (BDF_Graph),
					   2);
  if (!BDF_alloc_node)
    BDF_alloc_node = L_create_alloc_pool ("BDF_Node", 
					  sizeof (BDF_Node),
					  256);
  if (!BDF_alloc_arc)
    BDF_alloc_arc = L_create_alloc_pool ("BDF_Arc", 
					 sizeof (BDF_Arc),
					 256);
  if (!BDF_alloc_oper)
    BDF_alloc_oper = L_create_alloc_pool ("BDF_Oper", 
					  sizeof (BDF_Oper),
					  512);
  if (!BDF_alloc_operand)
    BDF_alloc_operand = L_create_alloc_pool ("BDF_Operand", 
					     sizeof (BDF_Operand),
					     1024);
  return;
}

static BDF_Arc *
BDF_new_arc (BDF_Oper *bop, BDF_Node *pred, BDF_Node *succ, DdNode *pfunc)
{
  BDF_Arc *barc;

  barc = (BDF_Arc *) L_alloc (BDF_alloc_arc);

  barc->op = bop;
  barc->pred = pred;
  barc->succ = succ;
  barc->pfunc = pfunc;
  memset (&barc->info, 0, sizeof (BDF_ArcInfo));

  pred->exit = List_insert_last (pred->exit, barc);
  succ->pred = List_insert_last (succ->pred, pred);

  return barc;
}

static BDF_Arc *
BDF_delete_arc (BDF_Arc *barc)
{
  SETDISPOSE(barc->info.g);
  SETDISPOSE(barc->info.k);

  SETDISPOSE(barc->info.v_out);
  SETDISPOSE(barc->info.r_out);
  SETDISPOSE(barc->info.a_out);
  SETDISPOSE(barc->info.e_out);

#if 0
  Cudd_RecursiveDeref (BDF_graph->dd, barc->pfunc);
#endif

  L_free (BDF_alloc_arc, barc);

  return NULL;
}

static BDF_Node *
BDF_new_node (BDF_Graph *bdfg, L_Cb *cb, int flags)
{
  BDF_Node *bnode;

  bnode = (BDF_Node *) L_alloc (BDF_alloc_node);

  bnode->flags = flags;

  if (cb)
    {
      bnode->id = cb->id;
      HashTable_insert (bdfg->hash_cb_dfcb, cb->id, bnode);
    }
  else if (flags & BDF_NODE_START)
    bnode->id = -1;
  else if (flags & BDF_NODE_STOP)
    bnode->id = -2;
  else
    L_punt ("BDF_new_node: must have CB or be start or stop");

  bnode->cb = cb;
  bnode->oper = NULL;
  bnode->pred = NULL;
  bnode->exit = NULL;
  bdfg->node_cnt++;

  memset (&bnode->info, 0, sizeof (BDF_NodeInfo));

  bdfg->cb = List_insert_last (bdfg->cb, bnode);

  bdfg->node_U = Set_add (bdfg->node_U, bnode->id);
  
  return bnode;
}

static BDF_Node *
BDF_delete_node (BDF_Graph *bdfg, BDF_Node *bnode)
{
  bnode->oper = List_reset (bnode->oper);
  bnode->pred = List_reset (bnode->pred);
  bnode->exit = List_reset (bnode->exit);

  SETDISPOSE (bnode->info.dom);
  SETDISPOSE (bnode->info.pdom);
  SETDISPOSE (bnode->info.v_in);
  SETDISPOSE (bnode->info.r_in);
  SETDISPOSE (bnode->info.a_in);
  SETDISPOSE (bnode->info.e_in);

  if (bnode->id > 0)
    HashTable_remove (bdfg->hash_cb_dfcb, bnode->id);

  bdfg->cb = List_remove (bdfg->cb, bnode);

  bdfg->node_U = Set_delete (bdfg->node_U, bnode->id);

  L_free (BDF_alloc_node, bnode);

  return NULL;
}

static BDF_Oper *
BDF_new_oper (BDF_Graph *bdfg, L_Oper *oper, int flags)
{
  BDF_Oper *bop = (BDF_Oper *) L_alloc (BDF_alloc_oper);

  bop->id = oper->id;

  bdfg->op_cnt++;

  bop->oper = oper;
  bop->dest = NULL;
  bop->src = NULL;

  bop->flags = flags;

  bop->pfunc = oper->pred[0] ? oper->pred[0]->value.pred.ssa->node :
    PG_pred_graph->one;

  HashTable_insert (bdfg->hash_op_dfop, oper->id, bop);
  bdfg->oper_U = Set_add (bdfg->oper_U, oper->id);

  memset (&bop->info, 0, sizeof (BDF_OperInfo));
  return bop;
}

static BDF_Oper *
BDF_delete_oper (BDF_Oper *bop)
{
  SETDISPOSE(bop->info.dom);
  SETDISPOSE(bop->info.pdom);
  SETDISPOSE(bop->info.v_in);
  SETDISPOSE(bop->info.v_out);
  SETDISPOSE(bop->info.r_in);
  SETDISPOSE(bop->info.r_out);
  SETDISPOSE(bop->info.a_in);
  SETDISPOSE(bop->info.a_out);
  SETDISPOSE(bop->info.e_in);
  SETDISPOSE(bop->info.e_out);

  L_free (BDF_alloc_oper, bop);
  return NULL;
}

static void
BDF_append_oper (BDF_Node *n, BDF_Oper *o)
{
  if (o->node)
    L_punt ("op getting added to a second node");

  o->node = n;

  n->oper = List_insert_last (n->oper, o);
  return;
}

static BDF_Operand *
BDF_new_operand (BDF_Graph *bdfg, BDF_Oper *bop,
		 L_Operand *operand, int flags)
{
  BDF_Operand *bopd;

  bopd = (BDF_Operand *) L_alloc (BDF_alloc_operand);
  bopd->oper = bop;
  bopd->operand = operand;
  bopd->id = ++bdfg->opd_cnt;

  switch (L_return_old_type (operand))
    {
    case L_OPERAND_RREGISTER:
      bopd->reg = L_REG_INDEX (operand->value.rr);
      break;
    case L_OPERAND_REGISTER:
      bopd->reg = L_REG_INDEX (operand->value.r);
      break;
    case L_OPERAND_MACRO:
      bopd->reg = L_MAC_INDEX (operand->value.mac);
      break;
    default:
      L_punt ("D_create_df_operand: Operand is not a register or macro.");
    }

  bopd->flags = flags;

  if (L_is_ctype_predicate (operand))
    {
      if (L_is_update_predicate_ptype (operand->ptype))
	bopd->flags |= BDF_OPD_TRANS;
      
      if (L_is_uncond_predicate_ptype (operand->ptype))
	bopd->flags |= BDF_OPD_UNCOND;
    }
  
  HashTable_insert (bdfg->hash_df_opd, bopd->id, bopd);
  
  bdfg->reg_U = Set_add (bdfg->reg_U, bopd->reg);
  bdfg->df_opd_U = Set_add (bdfg->df_opd_U, bopd->id);
  
  return bopd;
}

static void
BDF_hash_ref (HashTable h, int id, int reg)
{
  return;
}

static void
BDF_add_operands (BDF_Graph *g, BDF_Oper *bop, int flags)
{
  BDF_Operand *bopd;
  L_Oper *op;
  L_Operand *lopd;
  L_Attr *attr;
  int indx;

  op = bop->oper;

  if ((lopd = op->pred[0]))
    {
      bopd = BDF_new_operand (g, bop, lopd, BDF_OPD_UNCOND);
      bop->src = List_insert_last (bop->src, bopd);
      BDF_hash_ref (g->hash_df_opd_use, bopd->id, bopd->reg);
      bop->flags |= BDF_OP_UNCOND_SRC;
    }

  for (indx = 0; indx < L_max_src_operand; indx++)
    {
      if (!(lopd = op->src[indx]) ||
	  !(L_is_reg (lopd) || 
	    (L_is_macro (lopd) && M_dataflow_macro (lopd->value.mac))))
        continue;

      bopd = BDF_new_operand (g, bop, lopd, 0);
      bop->src = List_insert_last (bop->src, bopd);

      if (bopd->flags & BDF_OPD_UNCOND)
	bop->flags |= BDF_OP_UNCOND_SRC;

      BDF_hash_ref (g->hash_df_opd_def, bopd->id, bopd->reg);
    }

  for (indx = 0; indx < L_max_dest_operand; indx++)
    {
      if (!(lopd = op->dest[indx]) ||
	  !(L_is_reg (lopd) || 
	    (L_is_macro (lopd) && M_dataflow_macro (lopd->value.mac))))
        continue;

      bopd = BDF_new_operand (g, bop, lopd, 0);
      bop->dest = List_insert_last (bop->dest, bopd);

      if (bopd->flags & BDF_OPD_UNCOND)
	bop->flags |= BDF_OP_UNCOND_DEST;

      BDF_hash_ref (g->hash_df_opd_def, bopd->id, bopd->reg);
    }
  
  if ((attr = L_find_attr (op->attr, "tr")))
    {
      for (indx = 0; indx < attr->max_field; indx++)
        {
	  if (!(lopd = attr->field[indx]) ||
	      !(L_is_reg (lopd) || 
		(L_is_macro (lopd) && M_dataflow_macro (lopd->value.mac))))
	    continue;

	  bopd = BDF_new_operand (g, bop, lopd, 0);
	  bop->src = List_insert_last (bop->src, bopd);

	  if (bopd->flags & BDF_OPD_UNCOND)
	    bop->flags |= BDF_OP_UNCOND_SRC;

	  BDF_hash_ref (g->hash_df_opd_use, bopd->id, bopd->reg);
        }
    }

  if ((attr = L_find_attr (op->attr, "ret")))
    {
      for (indx = 0; indx < attr->max_field; indx++)
        {
	  if (!(lopd = attr->field[indx]) ||
	      !(L_is_reg (lopd) || 
		(L_is_macro (lopd) && M_dataflow_macro (lopd->value.mac))))
	    continue;

	  bopd = BDF_new_operand (g, bop, lopd, 0);
	  bop->dest = List_insert_last (bop->dest, bopd);

	  if (bopd->flags & BDF_OPD_UNCOND)
	    bop->flags |= BDF_OP_UNCOND_DEST;

	  BDF_hash_ref (g->hash_df_opd_def, bopd->id, bopd->reg);
        }
    }
  return;
}

BDF_Graph *
BDF_new_graph  (L_Func *fn)
{
  BDF_Graph *g;
  BDF_Node *n, *nd;
  BDF_Oper *o;
  L_Cb *cb;
  L_Oper *op;
  PG_Pred_Graph *pg = PG_pred_graph;

  if (!pg)
    L_punt ("BDF_new_graph: PG_pred_graph must be pre-existent");

  if (BDF_graph)
    BDF_graph = BDF_delete_graph (BDF_graph);
  else
    BDF_initialize();

  g = (BDF_Graph *) L_alloc (BDF_alloc_graph);

  BDF_graph = g;
  g->pg = PG_pred_graph;
  g->dd = PG_pred_graph->dd;
  g->f_0 = PG_pred_graph->zero;
  g->f_1 = PG_pred_graph->one;

  g->fn = fn;

  g->hash_cb_dfcb = HashTable_create (512);
  g->hash_op_dfop = HashTable_create (1024);
  g->hash_df_node = HashTable_create (512);
  g->hash_df_oper = HashTable_create (1024);
  g->hash_df_opd = HashTable_create (256);
  g->hash_df_opd_def = HashTable_create (256);
  g->hash_df_opd_use = HashTable_create (256);

  g->cb = NULL;

  g->node_U = NULL;
  g->oper_U = NULL;
  g->reg_U = NULL;
  g->df_opd_U = NULL;

  /* Start and stop nodes */

  g->start = BDF_new_node (g, NULL, BDF_NODE_START);

  /* Build the cb's */

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      n = BDF_new_node (g, cb, 0);
    }

  g->stop = BDF_new_node (g, NULL, BDF_NODE_STOP);

  BDF_new_arc (NULL, g->start, List_first (g->cb), pg->one);

  /* Hook up the flows */

  List_start (g->cb);
  while ((n = List_next (g->cb)))
    {
      L_Flow *fl;
      DdNode *f_e, *f_x, *f_t;

      if (!(cb = n->cb))
	continue;

      f_e = pg->one;
      Cudd_Ref (f_e);

      for (op = cb->first_op, fl = cb->dest_flow; op; op = op->next_op)
	{
	  o = BDF_new_oper (g, op, 0);
	  BDF_append_oper (n, o);
	  BDF_add_operands (g, o, BDF_ALL_OPERANDS);

	  if (L_subroutine_return_opcode (op))
	    {
	      /* Add an arc to the stop node */

	      /* compute exit function */
	      f_x = Cudd_bddIte(pg->dd, o->pfunc, f_e, pg->zero);
	      Cudd_Ref(f_x);
	      BDF_new_arc (o, n, g->stop, f_x);

	      /* compute new execution function */
	      f_e = Cudd_bddIte(pg->dd, f_x, pg->zero, f_t = f_e);
	      Cudd_Ref (f_e);
	      Cudd_RecursiveDeref (pg->dd, f_t);
	    }
	  else if (L_uncond_branch_opcode (op) || 
		   L_cond_branch_opcode (op) ||
		   L_check_branch_opcode (op))
	    {
	      nd = HashTable_find (g->hash_cb_dfcb, fl->dst_cb->id);

	      /* compute exit function */
	      f_x = Cudd_bddIte(pg->dd, f_e, o->pfunc, pg->zero);
	      Cudd_Ref (f_x);
	      if (L_cond_branch_opcode (op))
		{
		  DdNode *cfunc;
		  if (!L_is_ctype_predicate (op->dest[0]) ||
		      !L_is_macro (op->dest[0]) ||
		      op->dest[0]->value.mac != L_MAC_CR ||
		      !op->dest[0]->value.pred.ssa ||
		      !op->dest[0]->value.pred.ssa->node)
		    L_punt ("branch doesn't have a cr dest");

		  cfunc = op->dest[0]->value.pred.ssa->node;
		  f_x = Cudd_bddIte (pg->dd, cfunc, f_t = f_x, pg->zero);
		  Cudd_Ref (f_x);
		  Cudd_RecursiveDeref (pg->dd, f_t);
		}
	      BDF_new_arc (o, n, nd, f_x);

	      /* compute new execution function */
	      f_e = Cudd_bddIte(pg->dd, f_x, pg->zero, f_t = f_e);
	      Cudd_Ref (f_e);
	      Cudd_RecursiveDeref (pg->dd, f_t);

	      fl = fl->next_flow;
	    }
	  else if (L_register_branch_opcode (op))
	    {
	      /* compute exit function */
	      f_x = Cudd_bddIte(pg->dd, f_e, o->pfunc, pg->zero);
	      Cudd_Ref (f_x);
	      while (fl)
		{
		  nd = HashTable_find (g->hash_cb_dfcb, fl->dst_cb->id);
		  
		  /* These arcs should have lesser pfuncs... */

		  BDF_new_arc (o, n, nd, f_x);
		  fl = fl->next_flow;
		}
	    }
	}

      if (fl)
	{
	  nd = HashTable_find (g->hash_cb_dfcb, fl->dst_cb->id);
	  BDF_new_arc (NULL, n, nd, f_e);
	}
      else
	{
	  Cudd_RecursiveDeref (pg->dd, f_e);
	}
    }

  return g;
}

BDF_Graph *
BDF_delete_graph (BDF_Graph *g)
{
  BDF_Node *n;
  BDF_Arc *a;

  if (!g)
    return NULL;

  List_start (g->cb);
  while ((n = List_next (g->cb)))
    {
      List_start (n->exit);
      while ((a = List_next (n->exit)))
	BDF_delete_arc (a);

      BDF_delete_node (g, n);
      g->cb = List_delete_current (g->cb);
    }

  HashTable_free (g->hash_cb_dfcb);
  HashTable_free (g->hash_op_dfop);
  HashTable_free (g->hash_df_node);
  HashTable_free (g->hash_df_oper);
  HashTable_free (g->hash_df_opd);
  HashTable_free (g->hash_df_opd_def);
  HashTable_free (g->hash_df_opd_use);
  return NULL;
}

static BDF_Operand *
BDF_delete_operand (BDF_Operand *bopd)
{
  L_free (BDF_alloc_operand, bopd);
  return NULL;
}
