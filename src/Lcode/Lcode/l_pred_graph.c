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
 *  File:  l_pred_graph.c
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
 *      (C) Copyright 1997-2002, David August, John Sias, Wen-mei Hwu
 *      All rights granted to University of Illinois Board of Regents.
 *
\*****************************************************************************/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>

/*
 * Must call PG_setup_pred_graph(fn) before using query functions.
 */

#undef PG_PRINT_INACC_WARNINGS
#undef PG_ANALYZE_BR

#define PG_FOREACH_PSSA(p,list) for(List_start ((list)); \
                               (((p) = (PG_Pred_SSA *) List_next ((list))));)
#define PG_FOREACH_PCOND(p,list) for(List_start ((list)); \
                               (((p) = (PG_Cond *) List_next ((list))));)
#define PG_FOREACH_PPRED(p,list) for(List_start ((list)); \
                               (((p) = (PG_Pred *) List_next ((list))));)


/*===========================================================================*/
/*
 *      Global variables
 */
/*===========================================================================*/

PG_Pred_Graph *PG_pred_graph = NULL;
int PG_suppress_rebuilding_pred_graph = FALSE;

L_Time PG_ssa_build_time;
L_Time PG_bdd_build_time;

/*
 * Predicate buffer management
 * ======================================================================
 */

void
PG_alloc_pred_buf (PG_Pred_Graph *pg)
{
  if (!pg)
    return;

  if (pg->pred_buf)
    Lcode_free (pg->pred_buf);

  if (!pg->max_ssa_indx)
    pg->pred_buf = NULL;
  else
    pg->pred_buf = (int *) Lcode_malloc (sizeof (int) * pg->max_ssa_indx);

  return;
}

void
PG_free_pred_buf (PG_Pred_Graph *pg)
{
  if (pg->pred_buf)
    Lcode_free (pg->pred_buf);
  pg->pred_buf = NULL;
  return;
}

/*
 * Integer Range (IR) interface controls
 * ======================================================================
 */

/* Internal parameters.  Set using the exported functions below.
 * These are not modified in general application of the pred graph,
 * but are required for Lpred_opti 
 */

static int PG_IR_basis_functions = TRUE; /* Build cond knowledge into the
					  * pred graph
					  */

static int PG_unrelated_conds = FALSE;   /* Ignore cond relations
					  * entirely
					  */


void
PG_enable_IR_basis_functions (void)
{
  PG_IR_basis_functions = TRUE;
  return;
}


void
PG_disable_IR_basis_functions (void)
{
  PG_IR_basis_functions = FALSE;
  return;
}


int
PG_using_IR_basis_functions (void)
{
  return PG_IR_basis_functions;
}


void
PG_unrelate_conds (void)
{
  PG_unrelated_conds = TRUE;
  return;
}


void
PG_relate_conds (void)
{
  PG_unrelated_conds = FALSE;
  return;
}


int
PG_conds_unrelated (void)
{
  return PG_unrelated_conds;
}


/*
 * BB node info
 * ======================================================================
 */


static L_Alloc_Pool *PG_alloc_binfo = NULL;


typedef struct _PG_Binfo {
  Set r_in, r_out, g, k;
} PG_Binfo;


static PG_Binfo *
PG_new_binfo (void) 
{
  PG_Binfo *bi;

  if (!PG_alloc_binfo)
    PG_alloc_binfo = L_create_alloc_pool ("PG_Binfo", sizeof (PG_Binfo), 256);

  bi = L_alloc (PG_alloc_binfo);

  bi->r_in = NULL;
  bi->r_out = NULL;
  bi->g = NULL;
  bi->k = NULL;
  return bi;
}


static PG_Binfo *
PG_delete_binfo (PG_Binfo *bi)
{
  if (!bi)
    return NULL;

  if (bi->r_in)
    bi->r_in = Set_dispose (bi->r_in);
  if (bi->r_out)
    bi->r_out = Set_dispose (bi->r_out);
  if (bi->g)
    bi->g = Set_dispose (bi->g);
  if (bi->k)
    bi->k = Set_dispose (bi->k);

  L_free (PG_alloc_binfo, bi);

  return NULL;
}


/*
 * PG_Cond
 * ======================================================================
 */

static L_Alloc_Pool *PG_alloc_cond = NULL;


static PG_Cond *
PG_new_cond (PG_Pred_Graph * pred_graph, L_Cb * cb, L_Oper * oper)
{
  PG_Cond *cond;

  if (!PG_alloc_cond)
    PG_alloc_cond = L_create_alloc_pool ("PG_Cond", sizeof (PG_Cond), 256);
  cond = (PG_Cond *) L_alloc (PG_alloc_cond);

  cond->cond_indx = -1;
  cond->node = NULL;
  cond->first_oper = oper;
  cond->first_cb = cb;

  cond->same_cond = Set_add (NULL, oper->id);
  cond->compl_cond = NULL;
  cond->range_node = NULL;
  cond->range = NULL;
  
  pred_graph->pg_conds = List_insert_last (pred_graph->pg_conds, cond);

  return (cond);
}


static PG_Cond *
PG_delete_cond (PG_Pred_Graph *pg, PG_Cond * pcond)
{
  if (Cudd_Regular (pcond->node))
    {
      Cudd_RecursiveDeref (pg->dd, pcond->node);
      pcond->node = NULL;
    }

  if (pcond->compl_cond)
    Set_dispose (pcond->compl_cond);
  if (pcond->same_cond)
    Set_dispose (pcond->same_cond);

  L_free (PG_alloc_cond, pcond);

  return NULL;
}


/*
 * PG_Pred_SSA
 * ======================================================================
 */

static L_Alloc_Pool *PG_alloc_pred_ssa = NULL;


static PG_Pred_SSA *
PG_new_pred_ssa_noinsert (PG_Pred_Graph * pg, PG_Pred *pred)
{
  PG_Pred_SSA *pred_ssa;

  if (!PG_alloc_pred_ssa)
    PG_alloc_pred_ssa =
      L_create_alloc_pool ("PG_Pred_SSA", sizeof (PG_Pred_SSA), 256);
  pred_ssa = (PG_Pred_SSA *) L_alloc (PG_alloc_pred_ssa);

  pred_ssa->first_use_oper = NULL;
  pred_ssa->first_use_cb = NULL;
  pred_ssa->used = FALSE;
  pred_ssa->node = NULL;
  pred_ssa->def_type = 0;
  pred_ssa->oper = NULL;
  pred_ssa->composition = NULL;
  pred_ssa->pg_cond = NULL;
  pred_ssa->pg_pred = pred;
  pred_ssa->ssa_indx = pg->max_ssa_indx++;
  pred_ssa->prev_dest_ssa = NULL;

  HashTable_insert (pg->hash_pgPredSSA, pred_ssa->ssa_indx,
		    pred_ssa);

  if (pred)
    {
      pred->pg_pred_ssas = List_insert_first (pred->pg_pred_ssas, pred_ssa);
      pred->pred_ssa_ids = Set_add (pred->pred_ssa_ids, pred_ssa->ssa_indx);
    }

  return (pred_ssa);
}


static PG_Pred_SSA *
PG_new_pred_ssa (PG_Pred_Graph * pg, PG_Pred *pred)
{
  PG_Pred_SSA *pred_ssa;

  pred_ssa = PG_new_pred_ssa_noinsert (pg, pred);

  pg->pg_pred_ssas = List_insert_last (pg->pg_pred_ssas,
					       pred_ssa);

  return (pred_ssa);
}

static PG_Pred * PG_new_pred (PG_Pred_Graph * pg, int pred_num);

static PG_Pred_SSA *
PG_new_pred_def_ssa (PG_Pred_Graph *pg, PG_Cond *pg_cond,
		     L_Operand *dest, L_Oper *oper)
{
  PG_Pred_SSA *pred_ssa;
  PG_Pred *pg_pred;
  int rmi = L_REG_MAC_INDEX (dest);
		  
  if (!(pg_pred = PG_FIND_PRED (pg, rmi)))
    pg_pred = PG_new_pred (pg, rmi);

  pred_ssa = PG_new_pred_ssa (pg, pg_pred);
  pred_ssa->def_type = dest->ptype;
  pred_ssa->oper = oper;
  pred_ssa->pg_cond = pg_cond;

  dest->value.pred.ssa = pred_ssa;

  return pred_ssa;
}


static PG_Pred_SSA *
PG_delete_pred_ssa (PG_Pred_Graph *pg, PG_Pred_SSA * pssa)
{
  if (Cudd_Regular (pssa->node))
    {
      Cudd_RecursiveDeref (pg->dd, pssa->node);
      pssa->node = NULL;
    }

  if (pssa->composition)
    Set_dispose (pssa->composition);

  L_free (PG_alloc_pred_ssa, pssa);

  return NULL;
}


/*
 * PG_Pred
 * ======================================================================
 */

static L_Alloc_Pool *PG_alloc_pred = NULL;

static PG_Pred *
PG_new_pred (PG_Pred_Graph * pg, int pred_num)
{
  PG_Pred *pred;

  if (!PG_alloc_pred)
    PG_alloc_pred = L_create_alloc_pool ("PG_Pred", sizeof (PG_Pred), 128);

  pred = (PG_Pred *) L_alloc (PG_alloc_pred);
  pred->pred = pred_num;
  pred->pg_pred_ssas = NULL;
  pred->pred_ssa_ids = NULL;
  pred->pred_indx = pg->max_pred_indx++;
  pred->undef_ssa = PG_new_pred_ssa_noinsert (pg, pred);
  HashTable_insert (pg->hash_pred_pgPred, pred_num, pred);

  pg->pg_preds = List_insert_last (pg->pg_preds, pred);

  pg->uninit_ssas = Set_add (pg->uninit_ssas, pred->undef_ssa->ssa_indx);

  pred->pred_ssa_ids = Set_add (pred->pred_ssa_ids, pred->undef_ssa->ssa_indx);

  return (pred);
}


static PG_Pred *
PG_delete_pred (PG_Pred_Graph *pg, PG_Pred * ppred)
{
  PG_Pred_SSA *pssa;

  PG_FOREACH_PSSA (pssa, ppred->pg_pred_ssas)
    PG_delete_pred_ssa (pg, pssa);

  List_reset (ppred->pg_pred_ssas);
  if (ppred->pred_ssa_ids)
    Set_dispose (ppred->pred_ssa_ids);

  L_free (PG_alloc_pred, ppred);

  return NULL;
}


/*
 * PG_Pred_Graph
 * ======================================================================
 */

static L_Alloc_Pool *PG_alloc_pred_graph = NULL;


static PG_Pred_Graph *
PG_new_pred_graph (L_Func * fn, PG_Pred_Graph *pg)
{
  if (!pg)
    {
      if (!PG_alloc_pred_graph)
	PG_alloc_pred_graph =
	  L_create_alloc_pool ("PG_Pred_Graph", sizeof (PG_Pred_Graph), 1);

      pg = (PG_Pred_Graph *) L_alloc (PG_alloc_pred_graph);

      pg->dd = NULL;
    }

  pg->fn = fn;
  
  if (!pg->dd)
    {
      pg->dd = Cudd_Init (0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
      Cudd_AutodynEnable (pg->dd, CUDD_REORDER_SYMM_SIFT);
      pg->one = Cudd_ReadOne (pg->dd);
      pg->zero = Cudd_ReadLogicZero (pg->dd);
    }

  pg->max_ssa_indx = 0;
  pg->max_pred_indx = 0;

  pg->true_pred_ssa = NULL;
  pg->unknown_pred_ssa = NULL;

  pg->pg_preds = NULL;
  pg->pg_conds = NULL;
  pg->pg_pred_ssas = NULL;

  pg->ir_range_graph = NULL;

  pg->hash_pred_pgPred = HashTable_create (128);
  pg->hash_pgPredSSA = HashTable_create (256);
  pg->hash_pgCond = HashTable_create (256);

  pg->unreachable_code = 0;

  pg->pred_buf = NULL;

  pg->uninit_ssas = NULL;

  return (pg);
}


static PG_Pred_Graph *
PG_delete_pred_graph (PG_Pred_Graph * pg, int deallocate)
{
  PG_Cond *pcond;
  PG_Pred *ppred;

  if (!pg)
    return NULL;

  PG_FOREACH_PPRED (ppred, pg->pg_preds)
    PG_delete_pred (pg, ppred);

  pg->pg_preds = List_reset (pg->pg_preds);
  pg->pg_pred_ssas = List_reset (pg->pg_pred_ssas);

  PG_FOREACH_PCOND (pcond, pg->pg_conds)
    PG_delete_cond (pg, pcond);

  pg->pg_conds = List_reset (pg->pg_conds);

  HashTable_free (pg->hash_pred_pgPred);
  HashTable_free (pg->hash_pgPredSSA);
  HashTable_free (pg->hash_pgCond);

  if (pg->pred_buf)
    PG_free_pred_buf (pg);

  /* Unknown predicate does not have a corresponding
   * PG_Pred, so it has to be deleted separately.
   */

  if (pg->unknown_pred_ssa)
    {
      PG_delete_pred_ssa (pg, pg->unknown_pred_ssa);
      pg->unknown_pred_ssa = NULL;
    }

  if (pg->uninit_ssas)
    pg->uninit_ssas = Set_dispose (pg->uninit_ssas);

#if 0
  {
    int nzcnt;
    if ((nzcnt = Cudd_CheckZeroRef (pg->dd)))
      fprintf (stderr, "PG> %d node(s) with nonzero rc\n", nzcnt);
  }
#endif

  if (pg->ir_range_graph)
    IR_delete_range_graph (pg->ir_range_graph, deallocate);
  else if (deallocate)
    Cudd_Quit (pg->dd);

  if (deallocate)
    {
      pg->dd = NULL;
      L_free (PG_alloc_pred_graph, pg);
      pg = NULL;
    }

  return pg;
}


/*===========================================================================*/
/*
 *      Functions to build Predicate Analysis Graph
 */
/*===========================================================================*/


int
PG_no_defs_between (L_Operand * operand, L_Oper * opA, L_Oper * opB)
{
  int indx;
  L_Oper *pA;

  if (!opA || !opB)
    L_punt ("PG_no_defs_between: opA and opB cannot be NULL");

  if (!operand)
    return 1;

  for (pA = opA->next_op; pA != NULL; pA = pA->next_op)
    {
      if (pA == opB)
        return 1;

      for (indx = 0; indx < L_max_dest_operand; indx++)
	if (L_same_operand (operand, pA->dest[indx]))
	  return 0;

      if (L_is_implied_ret_destination_operand (pA, operand))
        return 0;
    }

  L_punt ("PG_no_defs_between: opA does not come before opB");
  return 0;
}


int
PG_equiv_operand (L_Cb * cb1, L_Oper * op1, L_Operand * src1,
                  L_Cb * cb2, L_Oper * op2, L_Operand * src2)
{
  if (cb1 != cb2) /* Conservative assumption */
    return FALSE;

  if (!L_same_operand (src1, src2))
    return FALSE;

  if (!(L_is_reg (src1) || L_is_macro (src1)))
    return TRUE;

  if (PG_no_defs_between (src1, op1, op2))
    return TRUE;

  return FALSE;
}


/*
 * Return condition between Instructions
 */
int
PG_condition_relation (L_Cb * cb1, L_Oper * op_cond1, L_Cb * cb2,
                       L_Oper * op_cond2)
{
  int reverse;
  int cval1, cval2;

  if (!op_cond1 || !op_cond2)
    return PG_NO_RELATION;

  cval1 = IR_cond_const_val (op_cond1);
  cval2 = IR_cond_const_val (op_cond2);

  if ((cval1 != -1) && (cval2 != -1))
    {
      if (cval1 == cval2)
        return PG_EQUIVALENT;
      else
        return PG_COMPLEMENT;
    }

  reverse = 0;
  if ((PG_equiv_operand (cb1, op_cond1, op_cond1->src[0],
                         cb2, op_cond2, op_cond2->src[0])) &&
      (PG_equiv_operand (cb1, op_cond1, op_cond1->src[1],
			 cb2, op_cond2, op_cond2->src[1])))
    reverse = 0;
  else if ((PG_equiv_operand (cb1, op_cond1, op_cond1->src[0],
                              cb2, op_cond2, op_cond2->src[1])) &&
	   (PG_equiv_operand (cb1, op_cond1, op_cond1->src[1],
			      cb2, op_cond2, op_cond2->src[0])))
    reverse = 1;
  else
    return PG_NO_RELATION;

  if (!reverse && L_equivalent_comparison_opcode (op_cond1, op_cond2))
    return PG_EQUIVALENT;
  else if (reverse && L_reversed_comparison_opcode (op_cond1, op_cond2))
    return PG_EQUIVALENT;
  else if (!reverse && L_complement_comparison_opcode (op_cond1, op_cond2))
    return PG_COMPLEMENT;
  else if (reverse && L_rev_complement_comparison_opcode (op_cond1, op_cond2))
    return PG_COMPLEMENT;
  else
    return PG_NO_RELATION;
}


L_Operand *
PG_find_dest_matching_ssa (L_Oper * oper, PG_Pred_SSA * pred_ssa)
{
  int indx;

  if (!oper || !pred_ssa)
    L_punt ("PG_find_dest_matching_ssa: NULL argument");

  if (!L_pred_define_opcode (oper))
    L_punt ("PG_find_dest_matching_ssa: Oper %d not a predicate define.\n",
            oper->id);

  for (indx = 0; indx < L_max_dest_operand; indx++)
    if (oper->dest[indx]->value.pred.ssa == pred_ssa)
      return oper->dest[indx];
  return NULL;
}


/* PG_materialize
 * ----------------------------------------------------------------------
 * Return a reference to a node (ref count 0) representing the logical
 * merge (phi) of the predicate SSAs represented in pssa's composition
 * set.
 */
static DdNode *
PG_materialize (PG_Pred_Graph *pg, PG_Pred_SSA *pssa)
{
  int size;
  DdNode *node;
  Set cset;

  cset = pssa->composition;

  if (!pg->pred_buf)
    PG_alloc_pred_buf (pg);

  if (!(size = Set_2array (cset, pg->pred_buf)))
    {
      /* CASE I: No definition reaches.  Make up an unrelated  function. */
#ifdef PG_PRINT_INACC_WARNINGS
      L_warn ("PAS> PG_materialize: "
	      "Uninitialized reference, PRED RMID %d",
	      pssa->pg_pred->pred);
#endif
      node = Cudd_bddNewVar (pg->dd);
    }
  else
    {
      int i, trivial, undef;
      PG_Pred_SSA *cpssa;
      DdNode *onode;

      /* Check reaching definitions for previous definition, 
       * logical equivalence */

      cpssa = PG_FIND_SSA (pg, pg->pred_buf[0]);
      undef = !(node = onode = cpssa->node);
      trivial = 1;
      for (i = 1; i < size; i++)
	{
	  cpssa = PG_FIND_SSA (pg, pg->pred_buf[i]);
	  undef |= !(node = cpssa->node);
	  trivial &= (node == onode);
	}

      if (undef || (!trivial && (size > 8)))
	{
	  /* CASE II: Either one of the constituents is undefined,
	   * or more than eight constituents are present and the
	   * merge is nontrivial.  Make up an unrelated function. 
	   */
      
#ifdef PG_PRINT_INACC_WARNINGS
	  if (undef)
	    L_warn ("PAS> PG_materialize: "
		    "Undefined constituent(s), PRED RMID %d",
		    pssa->pg_pred->pred);
	  else
	    L_warn ("PAS> PG_materialize: "
		    "Too many (%d) constituents, PRED RMID %d",
		    size, pssa->pg_pred->pred);
#endif

	  node = Cudd_bddNewVar (pg->dd);
	}
      else if (!trivial)
	{
	  /* CASE III: Form a canonical merge. */

	  DdNode **mutex;

	  mutex = alloca (size * sizeof (DdNode *));
	  I_BDD_new_mutex_domain (pg->dd, mutex, size);
      
	  node = pg->zero;
	  Cudd_Ref (node);

	  for (i = 0; i < size; i++)
	    {
	      DdNode *snode;
	      cpssa = PG_FIND_SSA (pg, pg->pred_buf[i]);
	      snode = cpssa->node;
	      node = Cudd_bddIte (pg->dd, mutex[i], snode, onode = node);
	      Cudd_Ref (node);
	      Cudd_RecursiveDeref (pg->dd, mutex[i]);
	      Cudd_RecursiveDeref (pg->dd, onode);
	    }
	  Cudd_Deref (node);
	}
      else
	{
	  /* CASE IV: All nodes are defined and logically equivalent;
	   * "node" references the desired node. */

	  if (!node)
	    L_punt ("PG_materialize: node should not be NULL");
	}
    }
  return node;
}


/* PG_find_matching_srd
 * ----------------------------------------------------------------------
 * Find the def SSA of the referenced predicate (pg_pred_use)
 * corresponding to the set of semantically-reaching definitions
 * (avail_defs).  If one does not exist, create it.
 */
static PG_Pred_SSA *
PG_find_matching_srd (PG_Pred_Graph *pg, PG_Pred *pg_pred_use, Set avail_defs)
{
  PG_Pred_SSA *pssa = NULL;
  int size;
  Set srd_set;

  srd_set = Set_intersect (avail_defs, pg_pred_use->pred_ssa_ids);

  if (!pg->pred_buf)
    PG_alloc_pred_buf (pg);

  if (!(size = Set_2array (srd_set, pg->pred_buf)))
    L_punt ("PG_find_matching_srd: Use of pred RMID %d has "
	    "no semantically reaching definitions", pg_pred_use->pred);

  if (size == 1)
    {
      pssa = PG_FIND_SSA (pg, pg->pred_buf[0]);
    }
  else
    {
      /* Attempt to find and reuse a merge SSA (no pssa->oper) which
       * has the desired srd set.  If none can be found, create 
       * a new one.
       */

      PG_FOREACH_PSSA (pssa, pg_pred_use->pg_pred_ssas)
	if (!pssa->oper &&
	    Set_same (pssa->composition, srd_set))
	  break;

      if (!pssa)
	{
	  /* Make up a new ssa -- could be an uninitialized
	   * predicate, or a merge of multiple predicate definitions */
	  pssa = PG_new_pred_ssa (pg, pg_pred_use);
	  pssa->composition = srd_set;
	  srd_set = NULL;
	}
    }

  if (srd_set)
    Set_dispose (srd_set);

  return pssa;
}

/*
 * Returns a naked previous destination node
 */

DdNode *
PG_find_prev_dest_node (PG_Pred_Graph * pg, PG_Pred_SSA* pssa)
{
  PG_Pred_SSA *prev_dest_ssa;
  DdNode *node;

  if ((prev_dest_ssa = pssa->prev_dest_ssa) &&
      (node = prev_dest_ssa->node))
    {
      /* Previous destination node has already been computed */
      ;
    }
  else
    {
      /* Materialize a node representing the logical combination
       * of all reaching definitions */

      node = PG_materialize (pg, pssa);

      if (prev_dest_ssa)
	{
	  prev_dest_ssa->node = node;
	  Cudd_Ref (node);
	}
      else
	{
#if 0
	  L_warn ("PAS> prev_dest_ssa not annotated on PRED RMID %d (oper %d)",
		  pssa->pg_pred->pred, pssa->oper ? pssa->oper->id : -1);
#else
	  ;
#endif
	}
    }

  return node;
}


static void
PG_instantiate_conds (PG_Pred_Graph *pg)
{
  PG_Cond *pg_cond;

  PG_FOREACH_PCOND (pg_cond, pg->pg_conds)
    {
      int cval = pg_cond->first_oper ? 
	IR_cond_const_val (pg_cond->first_oper) : -1;

      if (cval == 1)
	pg_cond->node = pg->one;
      else if (cval == 0)
	pg_cond->node = pg->zero;
      else
	pg_cond->node = Cudd_bddNewVar (pg->dd);

      Cudd_Ref (pg_cond->node);
      pg_cond->cond_indx = (Cudd_Regular (pg_cond->node))->index;
      HashTable_insert (pg->hash_pgCond, pg_cond->cond_indx, pg_cond);
    }

  return;
}


static int
PG_pred_def_prev_value_reqd (L_Oper *oper, L_Operand *dest)
{
  int ptype = dest->ptype;

  /* some ptypes always use the previous value */

  if (L_is_update_predicate_ptype (ptype))
    return 1;

  if (L_uncond_ptype (dest->ptype))
    return 0;

  return (oper->pred[0] != NULL);
}


static PG_Cond *
PG_attach_cond (PG_Pred_Graph *pg, L_Cb *cb, L_Oper *oper) 
{
  PG_Cond *pg_cond = NULL, *pg_cond_indx;
  int relation;

  if (!PG_unrelated_conds)
    {
      PG_FOREACH_PCOND (pg_cond_indx, pg->pg_conds)
	{
	  relation =
	    PG_condition_relation (pg_cond_indx->first_cb,
				   pg_cond_indx->first_oper,
				   cb, oper);
	  if (relation == PG_EQUIVALENT)
	    {
	      pg_cond = pg_cond_indx;
	      pg_cond_indx->same_cond =
		Set_add (pg_cond_indx->same_cond, oper->id);
	      break;
	    }
	  else if (relation == PG_COMPLEMENT)
	    {
	      pg_cond =
		(PG_Cond *) PG_POINTER_COMPLEMENT (pg_cond_indx);
	      pg_cond_indx->same_cond =
		Set_add (pg_cond_indx->same_cond, oper->id);
	      pg_cond_indx->compl_cond =
		Set_add (pg_cond_indx->compl_cond, oper->id);
	      break;
	    }
	}
    }

  if (!pg_cond)
    pg_cond = PG_new_cond (pg, cb, oper);

  return pg_cond;
}


/*
 * PG_compute_prd
 * ----------------------------------------------------------------------
 * Compute predicate reaching-definitions.  Add block info structures
 * to the BB graph containing this info.  R_IN is used to create Phi
 * functions for global merges of predicate value.  Assumes graph
 * has been sorted topologically.
 */
static void
PG_compute_prd (Graph bb_graph, Set set_uninit)
{
  int change, start;
  GraphNode bb_node;
  L_Oper *oper;
  
  /* Compute gen, kill sets */

  start = 1;

  List_start (bb_graph->topo_list);
  while ((bb_node = (GraphNode) List_next (bb_graph->topo_list)))
    {
      L_BB *bb;
      PG_Binfo *bi;

      bb = (L_BB *) GraphNodeContents (bb_node);
      bi = PG_new_binfo ();
      bb->ptr = (void *)bi;

      if (!bb->first_op)
	continue;

      if (start)
	{
	  bi->g = Set_copy (set_uninit);
	  start = 0;
	}

      for (oper = bb->first_op; oper != bb->last_op->next_op;
	   oper = oper->next_op)
	{
	  int indx;

	  if (!L_pred_define_opcode (oper))
	    continue;
	  
	  for (indx = 0; indx < L_max_dest_operand; indx++)
	    {
	      PG_Pred_SSA *pssa;
	      L_Operand *dest;
	      
	      dest = oper->dest[indx];
	      if (!dest || !L_is_ctype_predicate (dest))
		continue;
		
	      pssa = dest->value.pred.ssa;
	      bi->k = Set_union_acc (bi->k, pssa->pg_pred->pred_ssa_ids);
	      bi->g = Set_subtract_acc (bi->g, pssa->pg_pred->pred_ssa_ids);
	      bi->g = Set_add (bi->g, pssa->ssa_indx);
	    }
	}
      bi->r_out = Set_copy (bi->g);
    }

  /* Find reaching defs */

  do
    {
      change = 0;
      start = 1;
      List_start (bb_graph->topo_list);
      while ((bb_node = (GraphNode) List_next (bb_graph->topo_list)))
	{
	  L_BB *bb, *pbb;
	  PG_Binfo *bi, *pbi;
	  GraphArc pred_arc;
	  GraphNode pred_node;
	  Set tset = NULL;

	  bb = (L_BB *) GraphNodeContents (bb_node);
	  bi = (PG_Binfo *)bb->ptr;

	  List_start (bb_node->pred);
	  while ((pred_arc = (GraphArc) List_next (bb_node->pred)))
	    {
	      pred_node = GraphArcSrc (pred_arc);
	      pbb = (L_BB *) GraphNodeContents (pred_node);
	      pbi = (PG_Binfo *) pbb->ptr;
	      
	      if (pbi) /* some nodes may be predecessors, but unreachable */
		tset = Set_union_acc (tset, pbi->r_out);
	    }

	  if (start)
	    {
	      tset = Set_union_acc (tset, set_uninit);
	      start = 0;
	    }

	  if (Set_subtract_empty (tset, bi->r_in))
	    {
	      Set_dispose (tset);
	    }
	  else
	    {
	      change++;
	      Set_dispose (bi->r_in);
	      bi->r_in = tset;
	      tset = Set_subtract (tset, bi->k);
	      tset = Set_union_acc (tset, bi->g);
	      Set_dispose (bi->r_out);
	      bi->r_out = tset;
	    }
	}
    }
  while (change);
  return;
}


/*
 * PG_build_pred_graph
 * ----------------------------------------------------------------------
 * Main predicate graph builder (internal)
 * Builds the pred graph from scratch.
 * Fundamental assumption: predicates are not live around backedges
 */
static PG_Pred_Graph *
PG_build_pred_graph (L_Func * fn)
{
  PG_Pred_Graph *pg;
  DdManager *dd;

  DdNode *one, *zero, *tnode;

  L_Oper *oper;
  L_Operand *dest;
  PG_Cond *pg_cond;
  PG_Pred *pg_pred;
  PG_Pred_SSA *pred_ssa;
  int indx;
  Graph bb_graph;
  GraphNode bb_node;
  L_BB *bb;

  /*
   * SSA CONSTRUCTION
   * ----------------------------------------------------------------------
   */

  L_start_time (&PG_ssa_build_time);

  /*
   * Create the new pred graph and shortcuts to terminal node
   */
  
  pg = PG_new_pred_graph (fn, PG_pred_graph);

  dd = pg->dd;
  one = pg->one;
  zero = pg->zero;

  /*
   * Create "assumed" predicates
   */

  /* "True" predicate -- must be first predicate created */

  pg_pred = PG_new_pred (pg, 0);

  pred_ssa = PG_new_pred_ssa (pg, pg_pred);
  pred_ssa->def_type = L_PTYPE_NULL;
  pred_ssa->oper = NULL;
  pred_ssa->pg_cond = NULL;
  pred_ssa->node = one;
  Cudd_Ref (pred_ssa->node);

  pg->true_pred_ssa = pred_ssa;

  /* "Unknown" predicate -- be careful as it correlates! */

  pred_ssa = PG_new_pred_ssa (pg, NULL);
  pred_ssa->def_type = L_PTYPE_NULL;
  pred_ssa->oper = NULL;
  pred_ssa->pg_cond = NULL;
  pred_ssa->node = Cudd_bddNewVar (dd);
  Cudd_Ref (pred_ssa->node);

  pg->unknown_pred_ssa = pred_ssa;

  /*
   * Create the basic-block graph and find a topological ordering
   * ----------------------------------------------------------------------
   */

  bb_graph = L_create_bb_graph (fn);
  Graph_dfs_topo_sort (bb_graph);

  /*
   * Extract condition variables and assign SSA's to predicate destinations
   * ----------------------------------------------------------------------
   */

  List_start (bb_graph->topo_list);
  while ((bb_node = (GraphNode) List_next (bb_graph->topo_list)))
    {
      bb = (L_BB *) GraphNodeContents (bb_node);

      if (!bb->first_op)
        continue;

      for (oper = bb->first_op; oper != bb->last_op->next_op;
           oper = oper->next_op)
        {
          if (!L_pred_define_opcode (oper)
#ifdef PG_ANALYZE_BR
	      && !L_cond_branch_opcode (oper)
#endif
	      )
            continue;

	  pg_cond = NULL;

	  /* Create condition variable for any test performed.
	   * For special predicate defines (not traditional
	   * comparisons) make up a "disconnected" condition
	   */

          if (L_general_pred_comparison_opcode (oper)
#ifdef PG_ANALYZE_BR
	      || L_cond_branch_opcode (oper)
#endif
	      )
	    pg_cond = PG_attach_cond (pg, bb->cb, oper);
	  else if (L_special_pred_define_opcode (oper) ||
		   L_pred_load_opcode (oper))
	    pg_cond = PG_new_cond (pg, bb->cb, oper);

#ifdef PG_ANALYZE_BR
	  if (L_cond_branch_opcode (oper) && !oper->dest[0])
	    oper->dest[0] = L_new_macro_operand (L_MAC_CR,
						 L_CTYPE_PREDICATE,
						 L_PTYPE_UNCOND_T);
#endif
	  
	  /* Assign pred ssa's to predicate destinations */

          for (indx = 0; indx < L_max_dest_operand; indx++)
            {
              dest = oper->dest[indx];
              if (!dest || !L_is_ctype_predicate (dest))
                continue;

              if (L_is_macro (dest) && 
		  !M_dataflow_macro (dest->value.mac) &&
		  (dest->value.mac != L_MAC_CR))
		{
		  dest->value.pred.ssa = pg->true_pred_ssa;
		}
	      else if (L_is_rregister (dest))
		{
		  dest->value.pred.ssa = pg->unknown_pred_ssa;
		}
	      else
		{
		  PG_Pred_SSA *pssa;
		  pssa = PG_new_pred_def_ssa (pg, pg_cond, dest, oper);
		}
            }
        }
    }

  /*
   * Compute reaching definitions
   * ----------------------------------------------------------------------
   */

  PG_compute_prd (bb_graph, pg->uninit_ssas);

  /*
   * Ensure that unreachable nodes are cleared out
   * ----------------------------------------------------------------------
   */

  {
    int found = 0;
    List_start (bb_graph->nodes);
    while ((bb_node = (GraphNode) List_next (bb_graph->nodes)))
      {
	if (bb_node->info != -1)
	  continue;
	
	bb = (L_BB *) GraphNodeContents (bb_node);

	if (!bb->first_op)
	  continue;
	
	found = 1;
	
	for (oper = bb->first_op; oper != bb->last_op->next_op;
	     oper = oper->next_op)
	  {
	    int i;

	    for (i = 0; i < L_max_pred_operand; i++)
	      if (oper->pred[i] && L_is_ctype_predicate (oper->pred[i]))
		oper->pred[i]->value.pred.ssa = NULL;
	    for (i = 0; i < L_max_src_operand; i++)
	      if (oper->src[i] && L_is_ctype_predicate (oper->src[i]))
		oper->src[i]->value.pred.ssa = NULL;
	    for (i = 0; i < L_max_dest_operand; i++)
	      if (oper->dest[i] && L_is_ctype_predicate (oper->dest[i]))
		oper->dest[i]->value.pred.ssa = NULL;
	  }
      }

    /* Set to 1 if unreachable code was encountered in producing the
     * pred graph.  If pred queries are made in these unreachable
     * areas, cores will result!  
     */

    pg->unreachable_code = found;

    if (found && L_debug_df_dead_code)
      L_warn ("PG_build_pred_graph found unreachable code");
  }

  /*
   * Assign guard predicate SSA's
   * ----------------------------------------------------------------------
   */

  PG_alloc_pred_buf (pg);

  List_start (bb_graph->topo_list);
  while ((bb_node = (GraphNode) List_next (bb_graph->topo_list)))
    {
      PG_Binfo *bi;
      Set r_def;

      bb = (L_BB *) GraphNodeContents (bb_node);
      bi = (PG_Binfo *)bb->ptr;

      r_def = bi->r_in; /* potentially-reaching definitions */
      bi->r_in = NULL;

      if (bb->first_op)
	{
	  for (oper = bb->first_op;
	       oper != bb->last_op->next_op; oper = oper->next_op)
	    {
	      PG_Pred *pg_pred_use;
	      L_Operand *pred;
	      PG_Pred_SSA *pssa;

	      /* Find predicate SSA for guard predicate */

	      if ((pred = oper->pred[0]))
		{
		  if (L_is_macro (pred) && !M_dataflow_macro (pred->value.mac))
		    {
		      pssa = pg->true_pred_ssa;
		    }
		  else if (L_is_rregister (pred))
		    {
		      pssa = pg->unknown_pred_ssa;
		    }
		  else
		    {
		      if (!(pg_pred_use = 
			    PG_FIND_PRED (pg, L_REG_MAC_INDEX (pred))))
			L_punt ("PG_build_pred_graph: Invalid Lcode.  "
				"Pred use <p %d> bad.", pred->value.pred.reg);

		      pssa = PG_find_matching_srd (pg, pg_pred_use, r_def);

		      if (!L_pred_define_opcode (oper))
			pssa->used = TRUE;

		      if (!pssa->first_use_oper)
			{
			  pssa->first_use_oper = oper;
			  pssa->first_use_cb = bb->cb;
			}
		    }
		  pred->value.pred.ssa = pssa;
		}

	      /* Reset the analysis result on the demoted predicate, since
	       * it does not correspond to the current generation of
	       * analysis. 
	       */

	      if ((pred = oper->pred[1]))
		pred->value.pred.ssa = NULL;

	      /* For predicate defining operations, add the definition to
	       * the bb set */

	      if (!L_pred_define_opcode (oper))
		continue;
	  
	      /* Treat predicate sources on opers that have them */
	      
	      if (L_pred_combine_opcode (oper) || L_pred_store_opcode (oper))
		{
		  for (indx = 0; indx < L_max_src_operand; indx++)
		    {
		      if (!(pred = oper->src[indx]))
			continue;
		      
		      if (!(pg_pred_use = PG_FIND_PRED (pg, 
							L_REG_MAC_INDEX (pred))))
			L_punt ("PG_build_pred_graph: Invalid Lcode.  "
				"Pred use <p %d> bad.", pred->value.pred.reg);

		      pssa = PG_find_matching_srd (pg, pg_pred_use, r_def);
		      
		      if (!pssa->first_use_oper)
			{
			  pssa->first_use_oper = oper;
			  pssa->first_use_cb = bb->cb;
			}
		      pred->value.pred.ssa = pssa;
		    }
		}

	      /* Treat the predicate destinations */

	      for (indx = 0; indx < L_max_dest_operand; indx++)
		{
		  PG_Pred *ppred;

		  if (!(dest = oper->dest[indx]) ||
		      (L_is_macro (dest) &&
		       !M_dataflow_macro (dest->value.mac)) ||
		      L_is_rregister (dest) ||
		      !L_is_ctype_predicate (dest))
		    continue;

		  pssa = dest->value.pred.ssa;
		  ppred = pssa->pg_pred;
		  pssa->composition = Set_intersect (r_def, 
						     ppred->pred_ssa_ids);

		  if (PG_pred_def_prev_value_reqd (oper, dest))
		    pssa->prev_dest_ssa = PG_find_matching_srd (pg, ppred, 
								r_def);

		  /* update local reaching definitions */

		  r_def = Set_subtract_acc (r_def, 
					    pssa->pg_pred->pred_ssa_ids);
		  r_def = Set_add (r_def, pssa->ssa_indx);
		}
	    }
	}

      r_def = Set_dispose (r_def);
      PG_delete_binfo (bi);
      bb->ptr = NULL;
    }

  /* Free bb_graph */

  bb_graph = L_delete_bb_graph (bb_graph);

  L_stop_time (&PG_ssa_build_time);

  /*
   * BDD CONSTRUCTION
   * ----------------------------------------------------------------------
   */

  L_start_time (&PG_bdd_build_time);

  /* 1. Build condition functions, either using integer ranges or as
   * independent BDD variables */

  if (PG_IR_basis_functions)
    IR_form_basis_functions (fn, pg);
  else
    PG_instantiate_conds (pg);

  /* 2. Build predicate functions on top of the condition functions */

  PG_alloc_pred_buf (pg);

  PG_FOREACH_PSSA (pred_ssa, pg->pg_pred_ssas)
    {
      /* Skip SSA for which analysis is already complete */

      if (pred_ssa->node)
        continue;

      /* There are 2 types of SSAs.  Ones with oper defs and ones 
       * with only uses. Treat them independently. */

      if ((oper = pred_ssa->oper))
        {
	  DdNode *cnode, *gnode, *snode, *pdnode;

	  /* Find the node for the guard predicate */

          if (oper->pred[0])
            {
	      PG_Pred_SSA *pg_use_ssa;

              pg_use_ssa = oper->pred[0]->value.pred.ssa;

	      if (!(gnode = pg_use_ssa->node))
		{
		  /* The node referenced has not yet been computed.
		   * This commonly results from multiple reaching
		   * defs, in which case we must form a logical merge,
		   * in other words, an implicit "phi" function.  
		   */

		  gnode = PG_materialize (pg, pg_use_ssa);
		  /* remember this result for later use and garbage collection */
		  pg_use_ssa->node = gnode;
		  Cudd_Ref (gnode);
		}
            }
          else
            {
              gnode = one;
            }

	  Cudd_Ref (gnode);

	  switch (oper->opc)
	    {
	      /*
	       * CONTINUE cases -- handle special cases and skip
	       * conditional define procedure
	       */

	    case Lop_PRED_CLEAR:
	      pred_ssa->def_type = L_PTYPE_COND_T;
              if (gnode != one)
                {
                  pdnode = PG_find_prev_dest_node (pg, pred_ssa);
		  Cudd_Ref (pdnode);
                  pred_ssa->node = Cudd_bddAnd (dd, Cudd_Not (gnode),
                                                pdnode);

		  Cudd_RecursiveDeref (dd, pdnode);
                }
              else
                {
                  pred_ssa->node = zero;
                }
              Cudd_Ref (pred_ssa->node);
	      Cudd_RecursiveDeref (dd, gnode);
              continue;

	    case Lop_PRED_SET:
	      pred_ssa->def_type = L_PTYPE_COND_T;
              if (gnode != one)
                {
                  pdnode = PG_find_prev_dest_node (pg, pred_ssa);
		  Cudd_Ref (pdnode);
                  pred_ssa->node = Cudd_bddOr (dd, gnode,
					       pdnode);

		  Cudd_RecursiveDeref (dd, pdnode);
                }
              else
                {
                  pred_ssa->node = one;
                }
              Cudd_Ref (pred_ssa->node);
	      Cudd_RecursiveDeref (dd, gnode);
              continue;

	    case Lop_PRED_COMPL:
	      gnode = Cudd_Not (gnode);
	    case Lop_PRED_COPY:
	      pred_ssa->node = gnode;
	      pred_ssa->def_type = L_PTYPE_COND_T;
              Cudd_Ref (pred_ssa->node);
	      Cudd_RecursiveDeref (dd, gnode);
              continue;

	      /*
	       * BREAK cases -- continue with normal conditional define
	       * procedure
	       */

	    case Lop_PRED_MASK_AND:
              cnode = one;
	      Cudd_Ref (cnode);
	      pred_ssa->def_type = L_PTYPE_COND_T;
              for (indx = 0; indx < L_max_src_operand; indx++)
                {
                  if (!oper->src[indx])
                    continue;
                  if (!oper->src[indx]->value.pred.ssa)
                    L_punt ("PG_build_pred_graph: missing ssa.");
                  snode = oper->src[indx]->value.pred.ssa->node;
                  cnode = Cudd_bddAnd (dd, snode, tnode = cnode);
		  Cudd_Ref (cnode);
		  Cudd_RecursiveDeref (dd, tnode);
                }
	      Cudd_Deref (cnode);
	      break;

	    case Lop_PRED_MASK_OR:
              cnode = zero;
	      Cudd_Ref (cnode);
	      pred_ssa->def_type = L_PTYPE_COND_T;
              for (indx = 0; indx < L_max_src_operand; indx++)
                {
                  if (!oper->src[indx])
                    continue;
                  if (!oper->src[indx]->value.pred.ssa)
                    L_punt ("PG_build_pred_graph: missing ssa.");
                  snode = oper->src[indx]->value.pred.ssa->node;
                  cnode = Cudd_bddOr (dd, snode, tnode = cnode);
		  Cudd_Ref (cnode);
		  Cudd_RecursiveDeref (dd, tnode);
                }
	      Cudd_Deref (cnode);
	      break;

	    case Lop_PRED_LD:
              pg_cond = (PG_Cond *) PG_POINTER_REGULAR (pred_ssa->pg_cond);

	      pred_ssa->def_type = L_PTYPE_COND_T;

              if (PG_POINTER_IS_COMPLEMENT (pred_ssa->pg_cond))
                cnode = Cudd_Not (pg_cond->node);
              else
                cnode = pg_cond->node;
	      break;

	    default:
              /* 
               * At this point we know that the predicate definition
               * behaves like a traditional conditional define 
               */
              pg_cond = (PG_Cond *) PG_POINTER_REGULAR (pred_ssa->pg_cond);

              if (PG_POINTER_IS_COMPLEMENT (pred_ssa->pg_cond))
                cnode = Cudd_Not (pg_cond->node);
              else
                cnode = pg_cond->node;
	      break;
	    }

          switch (pred_ssa->def_type)
            {
	      /* UNCOND:
	       *  p[dest] = cond & p[src]
	       */
            case L_PTYPE_UNCOND_F:
	      cnode = Cudd_Not (cnode);
            case L_PTYPE_UNCOND_T:
	      pred_ssa->node = Cudd_bddAnd (dd, cnode, gnode);
	      Cudd_Ref (pred_ssa->node);
	      break;

	      /* COND:
	       *  p[dest] = p[src] ? cond : p[prev_dest]
	       */
            case L_PTYPE_COND_F:
	      cnode = Cudd_Not (cnode);
            case L_PTYPE_COND_T:
	      if (gnode != one)
		{
		  pdnode = PG_find_prev_dest_node (pg, pred_ssa);
		  Cudd_Ref (pdnode);
		  pred_ssa->node = Cudd_bddIte (dd, gnode, cnode, pdnode);
		  Cudd_Ref (pred_ssa->node);
		  Cudd_RecursiveDeref (dd, pdnode);
		}
	      else
		{
		  pred_ssa->node = Cudd_bddIte (dd, gnode, cnode, zero);
		  Cudd_Ref (pred_ssa->node);
		}
	      break;

	      /* OR:
	       *  p[dest] = cond ? (p[src] | p[prev_dest]) : p[prev_dest]
	       */
            case L_PTYPE_OR_F:
	      cnode = Cudd_Not (cnode);
            case L_PTYPE_OR_T:
	      pdnode = PG_find_prev_dest_node (pg, pred_ssa);
	      Cudd_Ref (pdnode);
	      tnode = Cudd_bddOr (dd, gnode, pdnode);
	      Cudd_Ref (tnode);
	      pred_ssa->node = Cudd_bddIte (dd, cnode, tnode, pdnode);
	      Cudd_Ref (pred_ssa->node);
	      Cudd_RecursiveDeref (dd, pdnode);
	      Cudd_RecursiveDeref (dd, tnode);
	      break;

	      /* AND:
	       *  p[dest] = p[src] ? (cond & p[prev_dest]) : p[prev_dest]
	       */
	    case L_PTYPE_AND_F:
	      cnode = Cudd_Not (cnode);
            case L_PTYPE_AND_T:
	      pdnode = PG_find_prev_dest_node (pg, pred_ssa);
	      Cudd_Ref (pdnode);
	      tnode = Cudd_bddAnd (dd, cnode, pdnode);
	      Cudd_Ref (tnode);
	      pred_ssa->node = Cudd_bddIte (dd, gnode, tnode, pdnode);
	      Cudd_Ref (pred_ssa->node);
	      Cudd_RecursiveDeref (dd, tnode);
	      Cudd_RecursiveDeref (dd, pdnode);
	      break;

	      /* SAND (conjunctive):
	       *  p[dest] = p[src] & (cond & p[prev_dest])
	       */
            case L_PTYPE_SAND_F:
	      cnode = Cudd_Not (cnode);
            case L_PTYPE_SAND_T:
	      pdnode = PG_find_prev_dest_node (pg, pred_ssa);
	      Cudd_Ref (pdnode);
	      tnode = Cudd_bddAnd (dd, cnode, pdnode);
	      Cudd_Ref (tnode);
	      pred_ssa->node = Cudd_bddAnd (dd, gnode, tnode);
	      Cudd_Ref (pred_ssa->node);
	      Cudd_RecursiveDeref (dd, tnode);
	      Cudd_RecursiveDeref (dd, pdnode);
	      break;

            default:
              L_punt ("L_build_pred_graph: illegal ptype");
            }

	  Cudd_RecursiveDeref (dd, gnode);
        }
      else
        {
	  /* Multiple reaching defs -- form a logical merge,
	   * in other words, an implicit "phi" function.
	   */

#ifdef PG_PRINT_INACC_WARNINGS
	  int pred_cnt;

	  if ((pred_cnt = Set_size (pred_ssa->composition)) != 1)
	    L_warn ("PAS> Merging %d defs to PRED RMID %d",
		    pred_cnt, pred_ssa->pg_pred->pred);

	  if (!Set_intersect_empty (pred_ssa->composition,
				    pg->uninit_ssas))
	    L_warn ("PAS> Pdef use of PRED RMID %d contains uninit val",
		    pred_ssa->pg_pred->pred);
#endif
	  pred_ssa->node = PG_materialize (pg, pred_ssa);
	  /* remember this result for later use and garbage collection */
	  Cudd_Ref (pred_ssa->node);
        }
    }

  PG_FOREACH_PSSA (pred_ssa, pg->pg_pred_ssas)
    {
      if (!pred_ssa->node)
	continue;

      if ((Cudd_Regular (pred_ssa->node))->ref == 0)
	L_punt ("pssa with refcnt 0");
    }

  PG_free_pred_buf (pg);

  if (!PG_IR_basis_functions)
    IR_deref_basis_functions (pg);

  L_stop_time (&PG_bdd_build_time);

  return pg;
}


void
PG_destroy_pred_graph (void)
{
  PG_pred_graph = PG_delete_pred_graph (PG_pred_graph, 1);
  return;
}


void
PG_setup_pred_graph (L_Func * fn)
{
#if 0
  if (!L_EXTRACT_BIT_VAL (fn->flags, L_FUNC_HYPERBLOCK))
    return;
#endif

  if (PG_suppress_rebuilding_pred_graph)
    return;

  if (PG_pred_graph)
    PG_pred_graph = PG_delete_pred_graph (PG_pred_graph, 1);

#if 1
  if (PG_alloc_binfo)
    L_print_alloc_info (stderr, PG_alloc_binfo, 0);
  if (PG_alloc_cond)
    L_print_alloc_info (stderr, PG_alloc_cond, 0);
  if (PG_alloc_pred)
    L_print_alloc_info (stderr, PG_alloc_pred, 0);
  if (PG_alloc_pred_ssa)
    L_print_alloc_info (stderr, PG_alloc_pred_ssa, 0);
  if (PG_alloc_pred_graph)
    L_print_alloc_info (stderr, PG_alloc_pred_graph, 0);
#endif

  PG_pred_graph = PG_build_pred_graph (fn);

  return;
}


int
PG_print_pred_bdd_daVinci (char *filename)
{
  PG_Pred_Graph *pg;
  PG_Pred_SSA *pred_ssa;
  int ssa_count;

  FILE *outfile;

  DdNode **node_array;
  char **name_array;

  if (!filename)
    L_punt ("PG_print_pred_bdd_daVinci: Filename is NULL.");

  if (!(pg = PG_pred_graph))
    L_punt ("PG_print_pred_bdd_daVinci: PG_pred_graph is NULL.");

  outfile = fopen (filename, "w");

  if (!outfile)
    L_punt ("PG_print_pred_bdd_daVinci: Cannot open output file.");

  node_array = (DdNode **) alloca (sizeof (DdNode *) * (pg->max_ssa_indx));

  name_array = (char **) alloca (sizeof (char *) * (pg->max_ssa_indx));

  ssa_count = 0;

  PG_FOREACH_PSSA (pred_ssa, pg->pg_pred_ssas)
    {
      if (pred_ssa == pg->unknown_pred_ssa || pred_ssa == pg->true_pred_ssa)
        continue;

      if (!pred_ssa->used)
        continue;

      node_array[ssa_count] = pred_ssa->node;

      name_array[ssa_count] = (char *) alloca (32);

      sprintf (name_array[ssa_count], "p%d, SSA %d",
               pred_ssa->first_use_oper->pred[0]->value.r,
               pred_ssa->ssa_indx);

      ssa_count++;

    }

  Cudd_DumpDaVinci (pg->dd, ssa_count, node_array, NULL, name_array, outfile);

  fclose (outfile);

  return 0;

}


int
PG_print_pred_bdd_dot (char *filename)
{
  PG_Pred_Graph *pg;
  PG_Pred_SSA *pred_ssa;
  int ssa_count;

  FILE *outfile;

  DdNode **node_array;
  char **name_array;

  if (!filename)
    L_punt ("PG_print_pred_bdd_dot: Filename is NULL.");

  if (!(pg = PG_pred_graph))
    L_punt ("PG_print_pred_bdd_dot: PG_pred_graph is NULL.");

  outfile = fopen (filename, "w");

  if (!outfile)
    L_punt ("PG_print_pred_bdd_daVinci: Cannot open output file.");

  node_array = (DdNode **) alloca (sizeof (DdNode *) * (pg->max_ssa_indx));

  name_array = (char **) alloca (sizeof (char *) * (pg->max_ssa_indx));

  ssa_count = 0;

  PG_FOREACH_PSSA (pred_ssa, pg->pg_pred_ssas)
    {
      if (pred_ssa == pg->unknown_pred_ssa || pred_ssa == pg->true_pred_ssa)
        continue;

      if (!pred_ssa->used)
        continue;

      node_array[ssa_count] = pred_ssa->node;

      name_array[ssa_count] = (char *) alloca (32);

      sprintf (name_array[ssa_count], "p%d, SSA %d",
               pred_ssa->first_use_oper->pred[0]->value.r,
               pred_ssa->ssa_indx);

      ssa_count++;

    }

  Cudd_DumpDot (pg->dd, ssa_count, node_array, NULL, name_array, outfile);

  fclose (outfile);

  return 0;

}


int
PG_print_bdd_expr_dot (char *filename, DdNode *ddnode)
{
  PG_Pred_Graph *pg;
  int ssa_count;

  FILE *outfile;

  DdNode **node_array = &ddnode;
  char *name = "f";
  char **name_array = &name;

  if (!filename)
    L_punt ("PG_print_pred_bdd_dot: Filename is NULL.");

  if (!(pg = PG_pred_graph))
    L_punt ("PG_print_pred_bdd_dot: PG_pred_graph is NULL.");

  outfile = fopen (filename, "w");

  if (!outfile)
    L_punt ("PG_print_pred_bdd_daVinci: Cannot open output file.");

  ssa_count = 0;

  Cudd_DumpDot (pg->dd, 1, node_array, NULL, name_array, outfile);

  fclose (outfile);

  return 0;

}


int
PG_pred_dead_code_removal (L_Func * fn)
{
  L_Cb *cb;
  L_Oper *oper, *nxt_op;
  int cnt;
  int unreachable, disjoint;
  DdNode *cfnode, *newnode, *opnode;
  PG_Pred_Graph *pg;
  cnt = 0;

  pg = PG_pred_graph;
  if (!pg)
    return 0;

  for (cb = fn->first_cb; cb; cb = cb->next_cb)
    {
      if (L_EXTRACT_BIT_VAL (cb->flags, L_CB_SOFTPIPE) &&
	  L_find_attr (cb->attr, "kernel"))
	continue;
      
      unreachable = 0;
      cfnode = pg->one;
      Cudd_Ref (cfnode);

      for (oper = cb->first_op; oper; oper = nxt_op)
        {
          nxt_op = oper->next_op;

	  if (!oper->pred[0])
	    opnode = pg->one;
	  else if (oper->pred[0]->value.pred.ssa)
	    opnode = oper->pred[0]->value.pred.ssa->node;
	  else
	    break; /* if no pred ssa, in unreachable code already */

          disjoint = Cudd_bddLeq (pg->dd, opnode, Cudd_Not (cfnode));

          if (disjoint || unreachable)
            {
              if (!unreachable && L_general_pred_comparison_opcode (oper))
                {
                  L_Operand *dest;
                  int i, uncond_dest = 0;
                  for (i = 0; i < L_max_dest_operand; i++)
                    {
                      if (!(dest = oper->dest[i]))
                        continue;
                      if (L_is_uncond_predicate_ptype (dest->ptype))
                        {
			  uncond_dest = 1;
			  dest->ptype = L_PTYPE_UNCOND_T;
                        }
                      else
                        {
                          L_delete_operand (dest);
                          oper->dest[i] = NULL;
                        }
                    }
                  if (!uncond_dest)
                    {
		      if (L_debug_df_dead_code)
			{
			  fprintf (stderr, "> PG_pred_dead_code_removal: \n"
				   "  elide cmp op %d\n", oper->id);
			  L_print_oper (stderr, oper);
			}
                      L_delete_complete_oper (cb, oper);
                    }
		  else 
		    {
		      if (L_debug_df_dead_code)
			{
			  fprintf (stderr, "> PG_pred_dead_code_removal: \n"
				   "  cmp op %d --> const-zero form\n", oper->id); 
			  L_print_oper (stderr, oper);
			}
		      L_delete_operand (oper->pred[0]);
		      oper->pred[0] = NULL;
		      oper->com[1] = Lcmp_COM_NE; 
		      oper->proc_opc = oper->opc;

		      L_delete_operand (oper->src[0]);
		      oper->src[0] = L_new_gen_int_operand (0);
		      L_delete_operand (oper->src[1]);
		      oper->src[1] = L_new_gen_int_operand (0);
		    }
                }
              else
                {
		  if (L_debug_df_dead_code)
		    {
		      fprintf (stderr, "> PG_pred_dead_code_removal: \n"
			       "  elide op %d\n", oper->id);
		      L_print_oper (stderr, oper);
		    }
                  L_delete_complete_oper (cb, oper);
                }
	      cnt++;
            }
          else if (L_uncond_branch_opcode (oper))
            {
              newnode = Cudd_bddAnd (pg->dd, Cudd_Not (opnode), cfnode);
              Cudd_Ref (newnode);
              Cudd_RecursiveDeref (pg->dd, cfnode);
              cfnode = newnode;
              if (cfnode == pg->zero)
                {
		  L_Flow *last_flow, *match_flow;
		  L_Cb *dst_cb;
                  unreachable = 1;
                  if (oper->pred[0])
                    {
#if 0
		      if (L_debug_df_dead_code)
			{
			  fprintf (stderr, "> PG_pred_dead_code_removal: \n"
				   "  promote uncond br op %d\n", oper->id);
			  L_print_oper (stderr, oper);
			}
                      L_delete_operand (oper->pred[0]);
                      oper->pred[0] = NULL;
#endif
		      last_flow = L_find_last_flow (cb->dest_flow);

		      if (last_flow && (last_flow->cc == 0) &&
			  (last_flow->dst_cb == cb->next_cb))
			{
			  dst_cb = last_flow->dst_cb;
			  match_flow = L_find_matching_flow (dst_cb->src_flow, 
							     last_flow);
			  dst_cb->src_flow = L_delete_flow (dst_cb->src_flow, 
							    match_flow);
			  cb->dest_flow = L_delete_flow (cb->dest_flow, 
							 last_flow);
			}
		      cb->flags = L_SET_BIT_FLAG (cb->flags, 
						  L_CB_HYPERBLOCK_NO_FALLTHRU);
                    }
		}
            }
        }
      Cudd_RecursiveDeref (pg->dd, cfnode);
    }
  return cnt;
}


void
L_bb_print_hookx (FILE * file, GraphNode node)
{
  L_BB *bb;

  bb = (L_BB *) node->ptr;

  if (node->imm_post_dom && node->imm_dom)
    fprintf (file, "id %d: cb %d, ops %d -> %d (%d) imm_dom %d,  imm_pdom %d",
             node->id, bb->cb->id, bb->first_op->id, bb->last_op->id,
             node->info, node->imm_dom->id, node->imm_post_dom->id);
  else if (node->imm_post_dom)
    fprintf (file, "id %d: cb %d, ops %d -> %d (%d) imm_dom X,  imm_pdom %d",
             node->id, bb->cb->id, bb->first_op->id, bb->last_op->id,
             node->info, node->imm_post_dom->id);
  else if (node->imm_dom)
    fprintf (file, "id %d: cb %d, ops %d -> %d (%d) imm_dom %d,  imm_pdom X",
             node->id, bb->cb->id, bb->first_op->id, bb->last_op->id,
             node->info, node->imm_dom->id);
  else if (bb->first_op)
    fprintf (file, "id %d: cb %d, ops %d -> %d (%d) imm_dom X,  imm_pdom X",
             node->id, bb->cb->id, bb->first_op->id, bb->last_op->id,
             node->info);
  else
    fprintf (file, "id %d: cb %d, ops X -> X (%d) imm_dom X,  imm_pdom X",
             node->id, bb->cb->id, node->info);
}

void
DB_print_bb_graph (Graph bb_graph, char *filename)
{
  Graph_daVinci (bb_graph, filename, L_bb_print_hookx);
  return;
}

void
DB_print_topo_list (List topo_list)
{
  ListElement element;
  GraphNode node;
  element = topo_list->first;
  printf ("( DEBUG List : ");
  while (element)
    {
      node = (GraphNode) element->ptr;
      printf ("%d ", node->id);
      element = element->next;
    }
  printf (")\n");
}





