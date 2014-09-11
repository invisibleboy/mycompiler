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
 *      File:   l_pred_flow.h
 *      Author: David August, Wen-mei Hwu
 *      Creation Date:  January 1997
 *
\*****************************************************************************/

#ifndef L_PRED_FLOW_H
#define L_PRED_FLOW_H

/* 10/29/02 REK Adding config.h */
#include <config.h>

#define IS_PREDICATED(a)        (a->pred[0] != NULL)
#define PREDICATE(a)            ((a->pred[0])?a->pred[0]->value.r:0)
#define DF_DEAD_CODE_ATTR "DF_dead_code"

#define PF_VISITED   0x0001
#define PF_DEAD_CODE 0x0002
#define PF_LIVE_CODE 0x0004
#define PF_CRIT_CODE 0x0008

#define PF_NO_OPERANDS          0x0000
#define PF_PRED_OPERANDS        0x0001
#define PF_STD_OPERANDS         0x0002
#define PF_ALL_OPERANDS         0x0003
#define PF_VPRED_OPERANDS       0x0004
#define PF_SUPPRESS_PRED_GRAPH  0x0010

/* PF_OPER flags */

#define PF_UNCOND_SRC           0x0001
#define PF_UNCOND_DEST          0x0002

typedef struct _pf_mem_set
{
  int id;
  Set conflicts;
} PF_MEM_SET;


typedef struct _df_pce_info
{
  Set trans;      /* shared with PDE */
  Set n_comp;     /* shared with PDE: loc_blocked info */
  Set x_comp;     /* shared with PDE */
  Set complement; /* shared with PDE: loc dead info */
  Set nd_safe;
  Set xd_safe;
  Set nu_safe;    /* shared with P3DE */
  Set xu_safe;    /* shared with P3DE */
  Set n_earliest;
  Set x_earliest;
  Set n_delayed;  /* shared with PDE */
  Set x_delayed;  /* shared with PDE */
  Set n_latest;   /* shared with P3DE: holds partial sink transparent info. */
  Set x_latest;   /* shared with P3DE: holds partial sink - blocking info. */
  Set n_isolated; /* shared with PDE: dead info */
  Set x_isolated; /* shared with PDE: dead info */
  Set n_insert;   /* shared with PDE */
  Set x_insert;   /* shared with P3DE: holds partial sink info */
  Set n_replace;  /* shared with P3DE: holds transparency info for up-safety */
  Set x_replace;  /* shared with PDE: holds delete info */

  Set n_spec_us;  /* shared with P3DE: holds partial delay info */
  Set x_spec_us;  /* shared with P3DE: holds partial delay info */
  Set n_spec_ds;  /* shared with PDE: holds partial dead info */
  Set x_spec_ds;  /* shared with PDE: holds partial dead info */
  Set speculate_up;

  /* P3DE-only information */
  Set loc_delayed;
  Set pred_guard;
  Set pred_set;
  Set pred_clear;
} DF_PCE_INFO;


typedef struct _df_inst_info
{
  Set mem_r_in;
  Set mem_r_out;

  Set mem_a_in;
  Set mem_a_out;

} DF_INST_INFO;


typedef struct _df_oper_info
{
  Set dom;
  Set post_dom;

  Set v_in;
  Set v_out;

  Set r_in;
  Set r_out;

  Set a_in;
  Set a_out;

  Set e_in;
  Set e_out;

  Set mem_r_in;
  Set mem_r_out;

  Set mem_a_in;
  Set mem_a_out;

} DF_OPER_INFO;


typedef struct _df_node_info
{
  Set dom;
  Set post_dom;

  Set use_gen;
  Set def_kill;

  Set in;
  Set out;

  List mem_gen;
  List mem_kill;

  List mem_in;
  List mem_out;

  DF_PCE_INFO  * pce_info;

  int cnt;
} DF_NODE_INFO;


typedef struct _df_bb_info
{
  DF_PCE_INFO * pce_info;

  double weight, weight_t, weight_nt;

} DF_BB_INFO;


typedef struct _df_cb_info
{
  Set dom;
  Set post_dom;

  Set v_in;
  Set v_out;

  Set r_in;
  Set r_out;

  Set a_in;
  Set a_out;

  Set e_in;
  Set e_out;

} DF_CB_INFO;


typedef struct _pf_cb
{
  L_Cb *cb;

  List pf_bbs;
  List pf_nodes;
  List pf_nodes_entry;

  List pf_opers;

  /* Dataflow */
  DF_CB_INFO *info;
} PF_CB;


typedef struct _pf_oper
{
  L_Oper *oper;
  List pf_insts;

  List src;
  List dest;
  List mem_src;
  List mem_dest;

  int flags;

  /* Dataflow */
  DF_OPER_INFO *info;
} PF_OPER;


typedef struct _pf_bb
{
  PF_CB * pf_cb;

  L_Oper *first_op;

  List pf_nodes;
  List pf_nodes_entry;
  List pf_nodes_last;

  /* Dataflow */
  DF_BB_INFO *info;
} PF_BB;


typedef struct _pf_node
{
  int id;

  int flags;
  int count;

  PF_CB *pf_cb;
  PF_BB *pf_bb;

  List pf_insts;

  Set pred_def;
  Set pred_true;

  List succ;
  List pred;

  /* Dataflow */
  DF_NODE_INFO *info;
} PF_NODE;


typedef struct _pf_inst
{
  int id;

  int flags;

  PF_OPER *pf_oper;

  PF_NODE *pf_node;

  int pred_true;
  int pred_known;
  List src;
  List dest;
  List mem_src;
  List mem_dest;

  /* Dataflow */
  DF_INST_INFO *info;

} PF_INST;


typedef struct _pf_operand
{
  L_Operand *operand;
  PF_OPER *pf_oper;
  int id;
  int reg;
  int memory:1;
  int transparent:1;
  int unconditional:1;
} PF_OPERAND;


typedef struct _pred_flow
{
  L_Func *fn;
  HashTable hash_cb_pfcb;
  HashTable hash_oper_pfoper;

  HashTable hash_pf_node;
  HashTable hash_pf_operand;
  HashTable hash_pf_mem_operand;
  HashTable hash_pf_inst;

  List list_pf_cb;
  List list_pf_node;

  PF_NODE *pf_node_root;

  /* Dataflow */
  Set pf_node_U;
  Set pf_operand_U;
  Set oper_U;
  Set reg_U;
  Set expression_U;
  Set fragile_U;
  Set mem_U;
  Set store_U;
  Set mem_stack_U;
  Set local_var_U;
  Set unpred_U;
  Set store_specific_U;

  List pf_mem_dest_operand_U;
  List pf_mem_src_operand_U;

  HashTable hash_RD_operand_def;
  HashTable hash_RD_operand_use;
  HashTable hash_mem_oper_conflict;
  HashTable hash_amb_mem_oper_conflict;

  int num_pf_operand;
  int num_pf_inst;

  int poison;
  /*
   * poison is set by PF_invalidate_graph to indicate that dataflow
   * needs to be rerun.
   */
} PRED_FLOW;

#define PF_FOREACH_CB(cb,list) for(List_start ((list)); \
                               (((cb) = (PF_CB *) List_next ((list))));)
#define PF_FOREACH_BB(cb,list) for(List_start ((list)); \
                               (((cb) = (PF_BB *) List_next ((list))));)
#define PF_FOREACH_NODE(node,list) for(List_start ((list)); \
                               (((node) = (PF_NODE *) List_next ((list))));)
#define PF_FOREACH_OPER(oper,list) for(List_start ((list)); \
                               (((oper) = (PF_OPER *) List_next ((list))));)
#define PF_FOREACH_INST(inst,list) for(List_start ((list)); \
                               (((inst) = (PF_INST *) List_next ((list))));)
#define PF_FOREACH_OPERAND(opd,list) for(List_start ((list)); \
                               (((opd) = (PF_OPERAND *) List_next ((list))));)

#define PF_FORHCAE_CB(cb,list) for(List_start ((list)); \
                               (((cb) = (PF_CB *) List_prev ((list))));)
#define PF_FORHCAE_BB(cb,list) for(List_start ((list)); \
                               (((cb) = (PF_BB *) List_prev ((list))));)
#define PF_FORHCAE_NODE(node,list) for(List_start ((list)); \
                               (((node) = (PF_NODE *) List_prev ((list))));)
#define PF_FORHCAE_OPER(oper,list) for(List_start ((list)); \
                               (((oper) = (PF_OPER *) List_prev ((list))));)
#define PF_FORHCAE_INST(inst,list) for(List_start ((list)); \
                               (((inst) = (PF_INST *) List_prev ((list))));)
#define PF_FORHCAE_OPERAND(opd,list) for(List_start ((list)); \
                               (((opd) = (PF_OPERAND *) List_prev ((list))));)

#define PF_POSSIBLE(pf_inst, pf_operand) ((pf_operand)->unconditional ||      \
                                          (pf_inst)->pred_true)

#define PF_ASSURED(pf_inst, pf_operand) (!(pf_operand)->transparent &&        \
                                         ((pf_operand)->unconditional ||      \
                                          ((pf_inst)->pred_true &&            \
                                           (pf_inst)->pred_known)))

#define PF_FIND_OPER(pf, id)   ((PF_OPER *) HashTable_find \
                                ((pf)->hash_oper_pfoper, (id)))

#define PF_FIND_CB(pf, id)   ((PF_CB *) HashTable_find \
                              ((pf)->hash_cb_pfcb, (id)))

#ifdef __cplusplus
extern "C"
{
#endif

  extern void PF_initialize (void);
  extern PRED_FLOW *PF_delete_flow_graph (PRED_FLOW * pred_flow);

  extern void PF_add_src_operand (PRED_FLOW * pred_flow, L_Oper * oper,
                                  int reg, int transparent, int ptype);
  extern void PF_add_dest_operand (PRED_FLOW * pred_flow, L_Oper * oper,
                                   int reg, int transparent, int ptype);

  extern PRED_FLOW *PF_build_bb_graph (L_Func * fn, int do_operands);
  extern PRED_FLOW *PF_build_pred_flow_graph (L_Func * fn,
                                              int do_operands);
  extern PRED_FLOW *PF_build_max_pred_flow_graph (L_Func * fn,
                                                  int do_operands);


  extern void PF_daVinci (FILE * file, PRED_FLOW * pred_flow);
  extern void PF_count_paths (PRED_FLOW * pred_flow);
  extern void PF_clear_partial_dead_code_markings (PRED_FLOW * pred_flow);
  extern void PF_partial_dead_code_removal (PRED_FLOW * pred_flow);

  extern void PF_invalidate_graph (PRED_FLOW * pred_flow);

  extern int PF_is_valid_graph (PRED_FLOW * pred_flow);

  extern PF_MEM_SET *PF_new_mem_set (int id, Set conflicts);
  extern PF_MEM_SET *PF_delete_mem_set (PF_MEM_SET * mem_set);

  extern void PF_demote_branches (L_Func * fn);

  extern DF_PCE_INFO *D_new_df_pce_info (void);

#ifdef __cplusplus
}
#endif

#endif
