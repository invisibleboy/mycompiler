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
/*===========================================================================
 *      File :          l_int_range.h
 *      Description :   Integer range analysis utilities
 *      Creation Date : March 10, 1998
 *      Authors :       John W. Sias
 *
 *      (C) Copyright 1998, John W. Sias and Wen-Mei W. Hwu
 *      All rights granted to University of Illinois Board of Regents.
 *==========================================================================*/

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <Lcode/l_main.h>
#include <library/i_hash.h>

#ifndef L_INT_RANGE_H
#define L_INT_RANGE_H

typedef struct _IR_Range_Graph {
  L_Func *fn;
  DdManager *dd;
  DdNode *one;                  /* Fast access to 1 */
  DdNode *zero;                 /* Fast access to 0 */
} IR_Range_Graph;

typedef struct _IR_Range_Table {
  HashTable hash_reg_to_range_list;     /* returns Lists of IR_Range */
  int disj_node_count;
  IR_Range_Graph *rg;
} IR_Range_Table;

typedef struct _IR_Node {
  ITuintmax lower;
  ITuintmax upper;

  int id;
  DdNode *bdd_node;
} IR_Node;

typedef struct _IR_Comp_Inst {
  /*
   * These are pointers to structures in the Lcode -- not copies
   * of the Lcode structures.
   */
  L_Oper *oper;
  L_Cb *cb;
  L_Operand *reg_operand;
} IR_Comp_Inst;

typedef struct _IR_Range {
  L_Operand *reg_operand;
  List instance_list;           /* IR_Comp_Inst */
  List node_list;               /* IR_Node */

  unsigned int bits;
  unsigned int count;
} IR_Range;

typedef struct _IR_Disj_Range {
  IR_Range *range;
  IR_Node *node;
  DdNode *range_node;
} IR_Disj_Range;

#define LIR_UNSIGNED 0x80

#define LIR_EQ      Lcmp_COM_EQ
#define LIR_NE      Lcmp_COM_NE
#define LIR_GT      Lcmp_COM_GT
#define LIR_GE      Lcmp_COM_GE
#define LIR_LT      Lcmp_COM_LT
#define LIR_LE      Lcmp_COM_LE
#define LIR_GT_U    Lcmp_COM_GT | LIR_UNSIGNED
#define LIR_GE_U    Lcmp_COM_GE | LIR_UNSIGNED
#define LIR_LT_U    Lcmp_COM_LT | LIR_UNSIGNED
#define LIR_LE_U    Lcmp_COM_LE | LIR_UNSIGNED

#define LIR_MAX_INT ITUINTMAX
#define LIR_MAX_POS ITINTMAX
#define LIR_MAX_NEG ITINTMIN

#define FALSE 0
#define TRUE 1

extern IR_Range_Table *IR_range_table;

extern IR_Range_Table *IR_form_disjoint_ranges (L_Func * fn);
extern void IR_delete_range_graph (IR_Range_Graph * graph, int deallocate);
extern int IR_possible_values (char *mins, int terms, int min_indx);
extern int IR_minterm_implication (char *minA, char *minB, int terms);

/*
 * Interface to PG construction
 * ----------------------------------------------------------------------------
 */

struct _PG_Pred_Graph;

extern int IR_cond_const_val (L_Oper * oper);
extern void IR_form_basis_functions (L_Func * fn, struct _PG_Pred_Graph *pg);
extern void IR_deref_basis_functions (struct _PG_Pred_Graph *pg);

/*
 * Debugging functions
 * ----------------------------------------------------------------------------
 */

extern void DB_print_minterm (int terms, char *minterm);
extern void DB_print_node_minterms (int terms, DdManager * dd_mgr,
                                    DdNode * dd_node);
extern void DB_print_range (IR_Range * range);
extern void DB_print_range_table (IR_Range_Table * table);
extern void DB_print_table_minterms (IR_Range_Table * table);

#endif
