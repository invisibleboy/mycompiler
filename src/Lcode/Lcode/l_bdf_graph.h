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
/* The IMPACT Research Group (www.crhc.uiuc.edu/IMPACT)                      */
/*****************************************************************************\
 *
 * Copyright Notices/Identification of Licensor(s) of
 * Original Software in the File
 *
 * Copyright 1990-1999 The Board of Trustees of the University of Illinois
 * Contact: Research and Technology Management Office,
 * University of Illinois at Urbana-Champaign;
 * FAX: 217-244-3716, or email: rtmo@uiuc.edu
 *
 * All rights reserved by the foregoing, respectively.
 *
 * This is licensed software.  The software license agreement with
 * the University of Illinois specifies the terms and conditions
 * for use and redistribution.
 *
\*****************************************************************************/
/*****************************************************************************\
 *      File:   l_pred_flow.h
 *      Author: David August, Wen-mei Hwu
 *      Creation Date:  January 1997
 *
\*****************************************************************************/

#ifndef L_BDF_GRAPH_H
#define L_BDF_GRAPH_H

#include <config.h>

#define DF_DEAD_CODE_ATTR "DF_dead_code"

#define BDF_VISITED   0x0001
#define BDF_DEAD_CODE 0x0002
#define BDF_LIVE_CODE 0x0004

#define BDF_NO_OPERANDS          0x0000
#define BDF_PRED_OPERANDS        0x0001
#define BDF_STD_OPERANDS         0x0002
#define BDF_ALL_OPERANDS         0x0003
#define BDF_VPRED_OPERANDS       0x0004
#define BDF_SUPPRESS_PRED_GRAPH  0x0010

/* PF_OPER flags */

#define BDF_UNCOND_SRC           0x0001
#define BDF_UNCOND_DEST          0x0002

/*
 * Info containers (dataflow sets)
 * ----------------------------------------------------------------------
 * Always allocated as part of an exit, node, oper, etc.
 */

typedef struct _BDF_ArcInfo {
  Set g;
  Set k;

  Set v_out;
  Set r_out;
  Set a_out;
  Set e_out;

} BDF_ArcInfo;

typedef struct _BDF_NodeInfo {
  Set dom;
  Set pdom;

  Set g;
  Set k;

  Set v_in;
  Set r_in;
  Set a_in;
  Set e_in;

  struct _BDF_Node *idom, *ipdom;

} BDF_NodeInfo;

#define BDF_OP_UNCOND_SRC  0x00000001
#define BDF_OP_UNCOND_DEST 0x00000002

typedef struct _BDF_OperInfo {
  Set dom;
  Set pdom;

  Set v_in;
  Set v_out;

  Set r_in;
  Set r_out;

  Set a_in;
  Set a_out;

  Set e_in;
  Set e_out;

} BDF_OperInfo;

#define BDF_OPD_TRANS  0x000000001
#define BDF_OPD_UNCOND 0x000000002

typedef struct _BDF_Operand {
  struct L_Operand *operand;
  struct _BDF_Oper *oper;
  int id;
  int reg;
  int flags;
} BDF_Operand;

#define BDF_NODE_START 0x00000001
#define BDF_NODE_STOP  0x00000002
#define BDF_NODE_EXIT  0x00000004

typedef struct _BDF_Oper {
  struct L_Oper *oper;

  List dest; /* BDF_Operand * */
  List src;  /* BDF_Operand * */

  int id;
  int flags;
  DdNode *pfunc;

  BDF_OperInfo info;
  struct _BDF_Node *node;
} BDF_Oper;


typedef struct _BDF_Arc {
  struct _BDF_Oper *op;
  struct _BDF_Node *pred;
  struct _BDF_Node *succ;
  struct _BDF_ArcInfo info;
  DdNode *pfunc;
} BDF_Arc;

typedef struct _BDF_Node {
  int id;
  int flags;

  struct L_Cb *cb;

  List oper;
  List pred; /* BDF_Arc * */
  List exit; /* BDF_Arc * */

  struct _BDF_NodeInfo info;

} BDF_Node;

typedef struct _BDF_Graph {
  struct L_Func *fn;
  HashTable hash_cb_dfcb;
  HashTable hash_op_dfop;
  HashTable hash_df_node;
  HashTable hash_df_oper;
  HashTable hash_df_opd;
  HashTable hash_df_opd_def;
  HashTable hash_df_opd_use;

  List cb;

  BDF_Node *start;
  BDF_Node *stop;
  
  int node_cnt;
  int op_cnt;
  int opd_cnt;

  Set node_U;    /* node universe */
  Set oper_U;    /* oper universe */
  Set reg_U;     /* reg indx universe */
  Set df_opd_U;  /* operand universe */

  struct _PG_Pred_Graph *pg;
  struct DdManager *dd;
  struct DdNode *f_0, *f_1;

} BDF_Graph;

#define BDF_FOREACH_CB(cb,list) for(List_start ((list)); \
                               (((cb) = (BDF_Node *) List_next ((list))));)
#define BDF_FOREACH_ARC(arc,list) for(List_start ((list)); \
                               (((arc) = (BDF_Arc *) List_next ((list))));)
#define BDF_FOREACH_OPER(oper,list) for(List_start ((list)); \
                               (((oper) = (BDF_Oper *) List_next ((list))));)
#define BDF_FOREACH_OPERAND(opd,list) for(List_start ((list)); \
                               (((opd) = (BDF_Operand *) List_next ((list))));)

#define BDF_FORHCAE_CB(cb,list) for(List_start ((list)); \
                               (((cb) = (BDF_Node *) List_prev ((list))));)
#define BDF_FORHCAE_OPER(oper,list) for(List_start ((list)); \
                               (((oper) = (BDF_Oper *) List_prev ((list))));)
#define BDF_FORHCAE_OPERAND(opd,list) for(List_start ((list)); \
                               (((opd) = (BDF_Operand *) List_prev ((list))));)

#ifdef __cplusplus
extern "C"
{
#endif

  extern void BDF_initialize ();

  extern BDF_Graph *BDF_delete_graph (BDF_Graph *g);
  extern BDF_Graph *BDF_new_graph (L_Func *fn);


#ifdef __cplusplus
}
#endif

#endif

