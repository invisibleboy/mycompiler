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
 *  File:  l_pred_graph.h
 *
 *  Description: Header file for predicate analysis system
 *
 *  Creation Date:  August, 1997
 *
 *  Author:  David August, Wen-mei Hwu
 *
 *  Revisions:
 *
 *
\*****************************************************************************/

#ifndef L_PRED_GRAPH_H
#define L_PRED_GRAPH_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <bdd/cudd.h>
#include <library/i_bdd_interface.h>
#include <Lcode/l_int_range.h>
#include <Lcode/l_time.h>

#define PG_NO_RELATION           0
#define PG_COMPLEMENT            1
#define PG_EQUIVALENT            2
#define PG_SUBSET                3
#define PG_SUPERSET              4
#define PG_MUTUALLY_EXCLUSIVE    5

#define PG_POINTER_COMPLEMENT(a)    ((void *)((unsigned long)(a) | 01))
#define PG_POINTER_IS_COMPLEMENT(a) ((long) (a) & 01)
#define PG_POINTER_REGULAR(a)       ((void *)((unsigned long)(a) & ~01))

/*********************************************************************
        TYPEDEFS
*********************************************************************/

typedef struct _PG_Cond {
  int cond_indx;

  DdNode *node;                 /* Cudd DdNode */

  DdNode *range_node;           /* Cudd DdNode in integer range bdd */
  IR_Range *range;              /* Integer range structure that 
                                   describes this cond */

  L_Oper *first_oper;           /* First oper that does this cond */
  L_Cb *first_cb;               /* Cb which has oper in it */
  Set same_cond;                /* Oper ids that match this condition */
  Set compl_cond;               /* Oper ids that compliment this condition */
} PG_Cond;


typedef struct _PG_Pred {
  int pred;
  List pg_pred_ssas;            /* PG_Pred_SSA * */
  struct _PG_Pred_SSA *undef_ssa;
  Set pred_ssa_ids;
  int pred_indx;                /* Compressed Index */
} PG_Pred;


typedef struct _PG_Pred_SSA {
  int ssa_indx;                 /* SSA index */
  int used;                     /* True if SSA is used as pred[0] */
  DdNode *node;                 /* Cudd DdNode */
  int def_type;                 /* (AND, OR, or UNCOND) and (C, or N) 
                                   use ptype */
  L_Oper *oper;                 /* Operation doing definition */
  Set composition;              /* If made of real preds, 
                                   list ssa_indexes here */
  PG_Cond *pg_cond;             /* Condition doing definition */
  PG_Pred *pg_pred;             /* Predicate dest for this def */
  L_Oper *first_use_oper;       /* First use of ssa predicate */
  L_Cb *first_use_cb;           /* First use of ssa predicate */
  struct _PG_Pred_SSA *prev_dest_ssa;
} PG_Pred_SSA;

typedef struct _PG_Pred_Graph {
  L_Func *fn;
  DdManager *dd;
  DdNode *one;                  /* Fast access to 1 */
  DdNode *zero;                 /* Fast access to 0 */

  int max_ssa_indx;
  int max_pred_indx;

  PG_Pred_SSA *true_pred_ssa;
  PG_Pred_SSA *unknown_pred_ssa;
  Set uninit_ssas;              /* Set of uninitialized ssa's
				 * (one for each pred) 
				 */

  List pg_preds;                /* Ordered list of preds */

  List pg_conds;                /* List of conds */

  List pg_pred_ssas;            /* List of SSA's */

  IR_Range_Graph *ir_range_graph;

  HashTable hash_pred_pgPred;
  HashTable hash_pgPredSSA;
  HashTable hash_pgCond;

  int unreachable_code;         /* Unreachable code was encountered
				 * in producing the pred graph.
				 * If pred queries are made in these
				 * unreachable areas, cores will result!
				 */

  int *pred_buf;
} PG_Pred_Graph;

#define PG_FIND_SSA(pg, indx) (PG_Pred_SSA *) HashTable_find \
                              ((pg)->hash_pgPredSSA, (indx))

#define PG_FIND_PRED(pg, indx) (PG_Pred *) HashTable_find_or_null \
                              ((pg)->hash_pred_pgPred, (indx))

#define PG_FIND_COND(pg, indx) (PG_Cond *) HashTable_find_or_null \
                              ((pg)->hash_pgCond, (indx))


extern PG_Pred_Graph *PG_pred_graph;
extern int PG_suppress_rebuilding_pred_graph;

extern L_Time PG_ssa_build_time;
extern L_Time PG_bdd_build_time;

/*********************************************************************
        FUNCTION EXTERNS
*********************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/*
 *      Setup functions
 */
  extern void PG_setup_pred_graph (L_Func *);

  extern void PG_enable_IR_basis_functions (void);
  extern void PG_disable_IR_basis_functions (void);
  extern int PG_using_IR_basis_functions (void);

/*
 *      Clean-up functions
 */
  extern void PG_destroy_pred_graph (void);

/*
 * These functions take operations and returns information how their
 * predicates relate
 */

  extern int PG_false_predicate_op (L_Oper * oper);
  extern int PG_true_predicate_op (L_Oper * oper);
  extern int PG_complementary_predicates_ops_explicit (L_Oper * oper1,
                                                       int use_demoted1,
                                                       L_Oper * oper2,
                                                       int use_demoted2);
  extern int PG_rel_complementary_predicates_ops (L_Oper * oper1,
						  L_Oper * oper2,
						  L_Oper * oper3);
  extern int PG_rel_complementary_predicates_ops_explicit (L_Oper * oper1, 
							   int use_demoted1,
							   L_Oper * oper2, 
							   int use_demoted2,
							   L_Oper * oper3, 
							   int use_demoted3);

  extern int PG_complementary_predicates_ops (L_Oper * oper1, L_Oper * oper2);
  extern int PG_complementary_predicates_ops_demoted (L_Oper * oper1,
                                                      L_Oper * oper2);

  extern int PG_equivalent_predicates_ops_explicit (L_Oper * oper1,
                                                    int use_demoted1,
                                                    L_Oper * oper2,
                                                    int use_demoted2);
  extern int PG_equivalent_predicates_ops (L_Oper * oper1, L_Oper * oper2);
  extern int PG_equivalent_predicates_ops_demoted (L_Oper * oper1,
                                                   L_Oper * oper2);

  extern int PG_intersecting_predicates_ops_explicit (L_Oper * oper1,
                                                      int use_demoted1,
                                                      L_Oper * oper2,
                                                      int use_demoted2);
  extern int PG_intersecting_predicates_ops (L_Oper * oper1, L_Oper * oper2);
  extern int PG_intersecting_predicates_ops_demoted (L_Oper * oper1,
                                                     L_Oper * oper2);
  extern int PG_disjoint_predicates_ops (L_Oper * oper1, L_Oper * oper2);
  extern int PG_disjoint_predicates_ops_demoted (L_Oper * oper1,
                                                 L_Oper * oper2);



  extern int PG_subset_predicate_ops_explicit (L_Oper * oper1,
                                               int use_demoted1,
                                               L_Oper * oper2,
                                               int use_demoted2);
  extern int PG_subset_predicate_ops (L_Oper * oper1, L_Oper * oper2);
  extern int PG_subset_predicate_ops_demoted (L_Oper * oper1, L_Oper * oper2);
  extern int PG_superset_predicate_ops (L_Oper * oper1, L_Oper * oper2);
  extern int PG_superset_predicate_ops_demoted (L_Oper * oper1,
                                                L_Oper * oper2);

  extern int PG_sum_predicates_ops_explicit (L_Oper * oper1, int use_demoted1,
                                             L_Oper * oper2, int use_demoted2,
                                             L_Oper * oper_sum,
                                             int use_demoted_sum);
  extern int PG_sum_predicates_ops (L_Oper * oper1, L_Oper * oper2,
                                    L_Oper * oper_sum);
  extern int PG_sum_predicates_ops_demoted (L_Oper * oper1, L_Oper * oper2,
                                            L_Oper * oper_sum);

  extern int PG_collectively_exhaustive_predicates_ops (List opers);
  extern int PG_collectively_exhaustive_predicates_ops_demoted (List opers);

  extern int PG_collective_subsumption (List opers, L_Oper *test_op);
  extern int PG_collective_subsumption_demoted (List opers, L_Oper *test_op);

  extern int PG_equiv_operand (L_Cb * cb1, L_Oper * op1, L_Operand * src1,
                               L_Cb * cb2, L_Oper * op2, L_Operand * src2);

  extern L_Operand *PG_find_dest_matching_ssa (L_Oper * oper,
                                               PG_Pred_SSA * pred_ssa);

  extern DdNode * PG_find_prev_dest_node (PG_Pred_Graph * pg, 
					  PG_Pred_SSA* pred_ssa);

/* 
 * These functions assume that the predicates are refered to by
 * the new predicate naming system.
 */
  extern int PG_possible_values (PG_Pred_SSA * pred, Set defined_preds,
                                 Set true_preds);
  extern int PG_effected_by (int pred_ssa_indx, Set effected_preds);

  extern void PG_test_partition_graph (L_Func * fn);

  extern int PG_pred_dead_code_removal (L_Func * fn);

  extern int L_delete_unreachable_blocks (L_Func * fn);

#ifdef __cplusplus
}
#endif

#endif
