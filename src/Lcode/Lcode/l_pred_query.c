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
 *  File:  l_pred_query.c
 *
 *  Description:  Functions to build and use predicate analysis system.  
 *
 *  Creation Date:  August, 1997
 *
 *  Author:  David August
 *
 *  Revisions: 
 *      Condition analysis framework
 *      More general liveness allowances -- John Sias
 *
 *      (C) Copyright 1997-2001, David August, John Sias, Wen-mei Hwu
 *      All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

long int PG_cnt_identity;
long int PG_cnt_disjoint;
long int PG_cnt_multi;

void
PG_cnt_clear (void)
{
  PG_cnt_identity = 0;
  PG_cnt_disjoint = 0;
  PG_cnt_multi = 0;
  return;
}

void
PG_cnt_read (long int *i, long int *d, long int *m)
{
  *i = PG_cnt_identity;
  *d = PG_cnt_disjoint;
  *m = PG_cnt_multi;
  return;
}

#define PSEL(op,ud) ((ud)?((op)->pred[1]):((op)->pred[0]))

#define PNODE(pg,pr) ((pr) ? (((pr)->value.pred.ssa) ? \
                      (pr)->value.pred.ssa->node : \
                      (L_punt("Missing PredSSA"), (DdNode *)NULL)) : (pg)->one)

/*===========================================================================
 *
 *      Query Functions 
 *
 *===========================================================================*/

int
PG_false_predicate_op (L_Oper * oper)
{
  PG_Pred_Graph *pg;
  PG_Pred_SSA *pred_ssa;

  PG_cnt_identity++;

  if (!oper)
    L_punt ("PG_false_predicate_op: NULL oper");

  if (!oper->pred[0])
    return 0;

  if (!(pred_ssa = oper->pred[0]->value.pred.ssa))
    L_punt ("PG_false_predicate_op: Rebuild pred graph");

  pg = PG_pred_graph;

  return (pred_ssa->node == pg->zero);
}

int
PG_true_predicate_op (L_Oper * oper)
{
  PG_Pred_Graph *pg;
  PG_Pred_SSA *pred_ssa;

  PG_cnt_identity++;

  if (!oper)
    L_punt ("PG_true_predicate_op: NULL oper");

  if (!oper->pred[0])
    return 1;

  if (!(pred_ssa = oper->pred[0]->value.pred.ssa))
    L_punt ("PG_true_predicate_op: Rebuild pred graph");

  pg = PG_pred_graph;

  return (pred_ssa->node == pg->one);
}

/*
 * return TRUE if (pred1 == !pred2)
 */
int
PG_complementary_predicates_ops_explicit (L_Oper * oper1, int use_demoted1,
                                          L_Oper * oper2, int use_demoted2)
{
  PG_Pred_Graph *pg;
  L_Operand *pred1, *pred2;
  DdNode *node1, *node2;

  pg = PG_pred_graph;

  PG_cnt_identity++;

  if (!oper1 || !oper2)
    L_punt ("PG_complementary_predicates_ops: NULL oper");

  pred1 = PSEL (oper1, use_demoted1);
  pred2 = PSEL (oper2, use_demoted2);

  /* Shortcut for non-predicated code */

  if (!pred1 && !pred2)
    return FALSE;

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  node1 = PNODE (pg, pred1);
  node2 = PNODE (pg, pred2);

  return (node1 == Cudd_Not (node2));
}

int
PG_complementary_predicates_ops (L_Oper * oper1, L_Oper * oper2)
{
  return PG_complementary_predicates_ops_explicit (oper1, FALSE, oper2,
                                                   FALSE);
}

int
PG_complementary_predicates_ops_demoted (L_Oper * oper1, L_Oper * oper2)
{
  return PG_complementary_predicates_ops_explicit (oper1, TRUE, oper2, TRUE);
}


/*
 * return TRUE if (pred1 == pred2)
 */
int
PG_equivalent_predicates_ops_explicit (L_Oper * oper1, int use_demoted1,
                                       L_Oper * oper2, int use_demoted2)
{
  L_Operand *pred1, *pred2;
  DdNode *node1, *node2;
  PG_Pred_Graph *pg;

  pg = PG_pred_graph;

  PG_cnt_identity++;

  if (!oper1 || !oper2)
    L_punt ("PG_equivalent_predicates_ops: NULL oper");

  pred1 = PSEL (oper1, use_demoted1);
  pred2 = PSEL (oper2, use_demoted2);

  /* Shortcut for non-predicated code */

  if (!pred1 && !pred2)
    return TRUE;

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  node1 = PNODE (pg, pred1);
  node2 = PNODE (pg, pred2);

  return (node2 == node1);
}

int
PG_equivalent_predicates_ops (L_Oper * oper1, L_Oper * oper2)
{
  return PG_equivalent_predicates_ops_explicit (oper1, FALSE, oper2, FALSE);
}

int
PG_equivalent_predicates_ops_demoted (L_Oper * oper1, L_Oper * oper2)
{
  return PG_equivalent_predicates_ops_explicit (oper1, TRUE, oper2, TRUE);
}

/*
 * return TRUE if pred1 and pred2 intersect
 */
int
PG_intersecting_predicates_ops_explicit (L_Oper * oper1, int use_demoted1,
                                         L_Oper * oper2, int use_demoted2)
{
  PG_Pred_Graph *pg;
  L_Operand *pred1, *pred2;
  DdNode *node1, *node2;
  int retval;

  PG_cnt_disjoint++;

  pg = PG_pred_graph;

  if (!oper1 || !oper2)
    L_punt ("PG_intersecting_predicates_ops: NULL oper");

  pred1 = PSEL (oper1, use_demoted1);
  pred2 = PSEL (oper2, use_demoted2);

  /* Shortcut for non-predicated code */
  if (!pred1 || !pred2)
    return TRUE;

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  if (!pred1->value.pred.ssa || !pred2->value.pred.ssa)
    L_punt ("Rerun Pred Graph");

  node1 = pred1->value.pred.ssa->node;
  node2 = pred2->value.pred.ssa->node;

  retval = (Cudd_bddLeq (pg->dd, node2, Cudd_Not (node1)));

  return (!retval);
}


int
PG_intersecting_predicates_ops (L_Oper * oper1, L_Oper * oper2)
{
  return PG_intersecting_predicates_ops_explicit (oper1, FALSE, oper2, FALSE);
}

int
PG_intersecting_predicates_ops_demoted (L_Oper * oper1, L_Oper * oper2)
{
  return PG_intersecting_predicates_ops_explicit (oper1, TRUE, oper2, TRUE);
}

int
PG_disjoint_predicates_ops (L_Oper * oper1, L_Oper * oper2)
{
  return (!PG_intersecting_predicates_ops_explicit
          (oper1, FALSE, oper2, FALSE));
}

int
PG_disjoint_predicates_ops_demoted (L_Oper * oper1, L_Oper * oper2)
{
  return (!PG_intersecting_predicates_ops_explicit
          (oper1, TRUE, oper2, TRUE));
}


/*
 * return TRUE if pred1 <= pred2
 */
int
PG_subset_predicate_ops_explicit (L_Oper * oper1, int use_demoted1,
                                  L_Oper * oper2, int use_demoted2)
{
  PG_Pred_Graph *pg;
  L_Operand *pred1, *pred2;
  DdNode *node1 = NULL, *node2 = NULL;
  int retval;

  PG_cnt_disjoint++;

  pg = PG_pred_graph;

  if (!oper1 || !oper2)
    L_punt ("PG_subset_predicates_ops: NULL oper");

  pred1 = PSEL (oper1, use_demoted1);
  pred2 = PSEL (oper2, use_demoted2);

  if (!pred1 && !pred2)
    return TRUE;

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  if (!pred1)
    node1 = pg->one;
  else if (pred1->value.pred.ssa)
    node1 = pred1->value.pred.ssa->node;
  else
    L_punt ("Rerun Pred Graph");

  if (!pred2)
    node2 = pg->one;
  else if (pred2->value.pred.ssa)
    node2 = pred2->value.pred.ssa->node;
  else
    L_punt ("Rerun Pred Graph");

  retval = (Cudd_bddLeq (pg->dd, node1, node2));

  return (retval);
}

int
PG_subset_predicate_ops (L_Oper * oper1, L_Oper * oper2)
{
  return PG_subset_predicate_ops_explicit (oper1, FALSE, oper2, FALSE);
}

int
PG_subset_predicate_ops_demoted (L_Oper * oper1, L_Oper * oper2)
{
  return PG_subset_predicate_ops_explicit (oper1, TRUE, oper2, TRUE);
}

/* 
 * return TRUE if oper1 is a superset of oper2
 */

int
PG_superset_predicate_ops (L_Oper * oper1, L_Oper * oper2)
{
  return (PG_subset_predicate_ops_explicit (oper2, FALSE, oper1, FALSE));
}

int
PG_superset_predicate_ops_demoted (L_Oper * oper1, L_Oper * oper2)
{
  return (PG_subset_predicate_ops_explicit (oper2, TRUE, oper1, TRUE));
}

/* 
 * return TRUE if the sum of predicates is equal to the third
 */
int
PG_sum_predicates_ops_explicit (L_Oper * oper1, int use_demoted1,
                                L_Oper * oper2, int use_demoted2,
                                L_Oper * oper_sum, int use_demoted_sum)
{
  PG_Pred_Graph *pg;
  L_Operand *pred1, *pred2, *pred_sum;
  DdNode *node1, *node2, *nodePS, *nodeS;

  pg = PG_pred_graph;

  PG_cnt_multi++;

  if (!oper1 || !oper2 || !oper_sum)
    L_punt ("PG_sum_predicates_ops: NULL oper");

  pred1 = PSEL (oper1, use_demoted1);
  pred2 = PSEL (oper2, use_demoted2);
  pred_sum = PSEL (oper_sum, use_demoted_sum);

  if (!pred1 && !pred2 && !pred_sum)
    return TRUE;

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  node1 = PNODE (pg, pred1);
  node2 = PNODE (pg, pred2);
  nodePS = PNODE (pg, pred_sum);

  nodeS = Cudd_bddOr (pg->dd, node1, node2);
  /* This ref count manipulation seems logically unnecessary... however, 
   * deleted / dead counts can be corrupted if it is removed. -- JWS
   */
  Cudd_Ref (nodeS);
  Cudd_RecursiveDeref (pg->dd, nodeS);
  /* pred1 + pred2 == pred_sum */
  return (nodePS == nodeS);
}

int
PG_sum_predicates_ops (L_Oper * oper1, L_Oper * oper2, L_Oper * oper_sum)
{
  return PG_sum_predicates_ops_explicit (oper1, FALSE, oper2, FALSE, oper_sum,
                                         FALSE);
}

int
PG_sum_predicates_ops_demoted (L_Oper * oper1, L_Oper * oper2,
                               L_Oper * oper_sum)
{
  return PG_sum_predicates_ops_explicit (oper1, TRUE, oper2, TRUE, oper_sum,
                                         TRUE);
}

/*
 * return TRUE if (pred1 | pred2 = pred3) && !(pred1 & pred2)
 */
int
PG_rel_complementary_predicates_ops_explicit (L_Oper * oper1, int use_demoted1,
					      L_Oper * oper2, int use_demoted2,
					      L_Oper * oper3, int use_demoted3)
{
  L_Operand *pred1, *pred2, *pred3;
  DdNode *node1 = NULL, *node2 = NULL, *node3 = NULL, *nodeS;
  PG_Pred_Graph *pg;

  pg = PG_pred_graph;

  PG_cnt_identity++;

  if (!oper1 || !oper2 || !oper3)
    L_punt ("PG_complementary_predicates_ops: NULL oper");

  pred1 = PSEL (oper1, use_demoted1);
  pred2 = PSEL (oper2, use_demoted2);
  pred3 = PSEL (oper3, use_demoted3);

  /* Shortcut for non-predicated code */
  if (!pred1 && !pred2)
    return FALSE;

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  if (!pred1)
    node1 = pg->one;
  else if (pred1->value.pred.ssa)
    node1 = pred1->value.pred.ssa->node;
  else
    L_punt ("Rerun Pred Graph");

  if (!pred2)
    node2 = pg->one;
  else if (pred2->value.pred.ssa)
    node2 = pred2->value.pred.ssa->node;
  else
    L_punt ("Rerun Pred Graph");

  if (!pred3)
    node3 = pg->one;
  else if (pred3->value.pred.ssa)
    node3 = pred3->value.pred.ssa->node;
  else
    L_punt ("Rerun Pred Graph");

  /* check that pred1 and pred2 are non-intersecting */

  if (!Cudd_bddLeq (pg->dd, node2, Cudd_Not (node1)))
    return 0;

  nodeS = Cudd_bddOr (pg->dd, node1, node2);
  /* This ref count manipulation seems logically unnecessary... however, 
   * deleted / dead counts can be corrupted if it is removed. -- JWS
   */
  Cudd_Ref (nodeS);
  Cudd_RecursiveDeref (pg->dd, nodeS);
  return (nodeS == node3);
}

int
PG_rel_complementary_predicates_ops (L_Oper * oper1,
				     L_Oper * oper2,
				     L_Oper * oper3)
{
  return PG_rel_complementary_predicates_ops_explicit (oper1, FALSE, oper2,
						       FALSE, oper3, FALSE);
}

/* 
 * return TRUE if the sum of all predicates is 1
 */
int
PG_collectively_exhaustive_predicates_ops_explicit (List opers,
                                                    int use_demoted_val)
{
  PG_Pred_Graph *pg;
  DdNode *cur_node, *old_node;
  DdNode *pred_node;
  L_Operand *pred;
  L_Oper *oper;

  pg = PG_pred_graph;

  PG_cnt_multi++;

  if (!opers)
    return FALSE;

  List_start (opers);
  while ((oper = (L_Oper *) List_next (opers)))
    {
      pred = PSEL (oper, use_demoted_val);

      if (!pred)
        return TRUE;
    }

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  cur_node = pg->zero;
  Cudd_Ref (cur_node);
  List_start (opers);
  while ((oper = (L_Oper *) List_next (opers)) && (cur_node != pg->one))
    {
      pred = PSEL (oper, use_demoted_val);
      pred_node = PNODE (pg, pred);

      cur_node = Cudd_bddOr (pg->dd, pred_node, old_node = cur_node);
      Cudd_Ref (cur_node);
      Cudd_RecursiveDeref (pg->dd, old_node);
    }

  Cudd_RecursiveDeref (pg->dd, cur_node);

  return ((cur_node == pg->one) ? TRUE : FALSE);
}

int
PG_collectively_exhaustive_predicates_ops (List opers)
{
  return PG_collectively_exhaustive_predicates_ops_explicit (opers, FALSE);
}

int
PG_collectively_exhaustive_predicates_ops_demoted (List opers)
{
  return PG_collectively_exhaustive_predicates_ops_explicit (opers, TRUE);
}

/* 
 * return TRUE if the sum of all predicates subsumes predicate
 */
int
PG_collective_subsumption_explicit(List opers,
				   L_Oper *test_op,
				   int use_demoted_val)
{
  PG_Pred_Graph *pg;
  DdNode *cur_node, *old_node, *pred_node, *test_node;
  L_Operand *pred;
  L_Oper *oper;
  int rv;

  pg = PG_pred_graph;

  PG_cnt_multi++;

  if (!opers || !test_op)
    return FALSE;

  List_start (opers);
  while ((oper = (L_Oper *) List_next (opers)))
    if (!(pred = PSEL (oper, use_demoted_val)))
      return TRUE;

  if (!pg)
    L_punt ("Predicate graph not built.  (Check H func flag)");

  if (use_demoted_val)
    test_node = (!test_op->pred[1]) ? pg->one
      : test_op->pred[1]->value.pred.ssa->node;
  else
    test_node = (!test_op->pred[0]) ? pg->one
      : test_op->pred[0]->value.pred.ssa->node;

  cur_node = pg->zero;
  Cudd_Ref (cur_node);
  List_start (opers);
  while ((oper = (L_Oper *) List_next (opers)) && (cur_node != pg->one))
    {
      pred = PSEL (oper, use_demoted_val);
      pred_node = PNODE (pg, pred);

      cur_node = Cudd_bddOr (pg->dd, pred_node, old_node = cur_node);
      Cudd_Ref (cur_node);
      Cudd_RecursiveDeref (pg->dd, old_node);
    }

  rv = Cudd_bddLeq (pg->dd, test_node, cur_node) ? TRUE : FALSE;

  Cudd_RecursiveDeref (pg->dd, cur_node);

  return rv;
}

int
PG_collective_subsumption (List opers, L_Oper *test_op)
{
  return PG_collective_subsumption_explicit (opers, test_op, FALSE);
}

int
PG_collective_subsumption_demoted (List opers, L_Oper *test_op)
{
  return PG_collective_subsumption_explicit (opers, test_op, TRUE);
}



/*
 * Sets contain ssa index numbers - not predicate indexes.
 *      Returns 0 if predicate can only be FALSE.
 *              1 if predicate can only be TRUE.
 *              2 if predicate can be TRUE or FALSE.
 *              -1 if the input predicates combination is not possible
 */
int
PG_possible_values_tf (PG_Pred_SSA * pssa, Set true_preds, Set false_preds)
{
  int *buf_true = NULL, *buf_false = NULL;
  int num_true, num_false, indx, rv;
  DdNode *cur_node, *old_node;
  PG_Pred_SSA *pg_pred_ssa;
  PG_Pred_Graph *pg;

  pg = PG_pred_graph;

  if ((num_true = Set_size (true_preds)))
    {
      buf_true = (int *) alloca (sizeof (int) * num_true);
      Set_2array (true_preds, buf_true);
    }

  if ((num_false = Set_size (false_preds)))
    {
      buf_false = (int *) alloca (sizeof (int) * num_false);
      Set_2array (false_preds, buf_false);
    }

  cur_node = pg->zero;
  Cudd_Ref (cur_node);

  for (indx = 0; indx < num_true; indx++)
    {
      pg_pred_ssa = PG_FIND_SSA(pg, buf_true[indx]);
      cur_node = Cudd_bddOr (pg->dd, Cudd_Not(pg_pred_ssa->node), 
			     old_node = cur_node);
      Cudd_Ref (cur_node);
      Cudd_RecursiveDeref (pg->dd, old_node);
    }

  for (indx = 0; indx < num_false; indx++)
    {
      pg_pred_ssa = PG_FIND_SSA(pg, buf_false[indx]);
      cur_node = Cudd_bddOr (pg->dd, pg_pred_ssa->node, 
			     old_node = cur_node);
      Cudd_Ref (cur_node);
      Cudd_RecursiveDeref (pg->dd, old_node);
    }

  /* cur_node is the prohibited portion of the universe */

  if (pg->one == cur_node)
    rv = -1;     /* combination cannot occur */
  else if (Cudd_bddLeq (pg->dd, Cudd_Not (pssa->node), cur_node))
    rv = 1;      /* predicate is true */
  else if (Cudd_bddLeq (pg->dd, pssa->node, cur_node))
    rv = 0;      /* predicate is false */
  else
    rv = 2;      /* predicate may be either true or false */

  Cudd_RecursiveDeref (pg->dd, cur_node);

  return rv;
}


/*
 * Sets contain ssa index numbers - not predicate indexes.
 *      Returns 0 if predicate can only be FALSE.
 *              1 if predicate can only be TRUE.
 *              2 if predicate can be TRUE or FALSE.
 *              -1 if the input predicates combination is not possible
 */
int
PG_possible_values (PG_Pred_SSA * pred, Set defined_preds, Set true_preds)
{
  Set false_preds;
  int value;

  /* True pred can only be one */
  if (!pred)
    return 1;

  false_preds = Set_subtract (defined_preds, true_preds);

  value = PG_possible_values_tf (pred, true_preds, false_preds);

  Set_dispose (false_preds);

  return value;
}

/*
 * Sets contain ssa index numbers - not predicate indexes.
 *      Return 1 if pred being known 1 or 0 determines a known value for
 *      any of the predicates in the effected_by set
 */
int
PG_effected_by (int pred_ssa_indx, Set effected_preds)
{
  int *buf_effected = NULL;
  int num_effected;
  int indx;
  PG_Pred_SSA *pg_pred_ssa;
  PG_Pred_SSA *pred;
  PG_Pred_Graph *pg;

  pg = PG_pred_graph;

  pred = PG_FIND_SSA(pg, pred_ssa_indx);

  if ((num_effected = Set_size (effected_preds)))
    {
      buf_effected = (int *) alloca (sizeof (int) * num_effected);
      Set_2array (effected_preds, buf_effected);
    }

  for (indx = 0; indx < num_effected; indx++)
    {
      pg_pred_ssa = PG_FIND_SSA(pg, buf_effected[indx]);

      if (Cudd_bddLeq (pg->dd, Cudd_Not (pred->node), pg_pred_ssa->node))
	return 1;

      if (Cudd_bddLeq (pg->dd, pred->node, pg_pred_ssa->node))
	return 1;

      if (Cudd_bddLeq (pg->dd, pred->node, Cudd_Not (pg_pred_ssa->node)))
	return 1;

      if (Cudd_bddLeq (pg->dd, Cudd_Not (pred->node), 
		       Cudd_Not (pg_pred_ssa->node)))
	return 1;
    }

  return 0;
}
