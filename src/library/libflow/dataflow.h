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
 *      File:   dataflow.h
 *      Author: Po-hua Chang
 *      Modified By: XXX, date, time, why
 *      Copyright (c) 1991 Po-hua Chang and Wen-mei Hwu. All rights reserved.  
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/
#ifndef IMPACT_DATAFLOW_H
#define IMPACT_DATAFLOW_H

/* 10/29/02 REK Adding config.h */
#include <config.h>
#include <library/set.h>

#define MAX_NODE_ID     (16*1024)       /* instr[] */
#define MAX_REG_ID      (16*1024)       /* vr[] */

struct I_Node
{
  /*
   *      The caller must define the below 3 fields.
   */
  int id;                       /* the instruction ID */
  Set dest_reg;                 /* 0/1 result register */
  Set src_reg;                  /* 0/N source registers */
  /*
   *      The following information are automatically
   *      derived from the above information.
   */
  Set gen;                      /* the expression itself */
  Set kill;                     /* the expressions killed by it */
  Set use;                      /* the source registers */
  Set def;                      /* the destination registers */
  /*
   *      Use-definition analysis.
   *      rd-chain
   */
  Set e_in;
  Set e_out;
  /*
   *      Available expressions.
   */
  Set a_in;
  Set a_out;
  /*
   *      Live-variable life.
   */
  Set v_in;
  Set v_out;
  /*
   *      Dominator information
   */
  Set dom;
  Set post_dom;
  /*
   *      Internal variables (shoule not be used by the user)
   */
  short defined;                /* internal use */
};

struct I_Reg
{
  Set e_def;                    /* the set of nodes that define it */
  Set e_use;                    /* the set of nodes that use it */
  short defined;                /* internal use */
};


#define REACH_DEFINE            1
#define USE_DEF_CHAIN           2
#define LIVE_VAR                4
#define DOMINATOR               8
#define POST_DOMINATOR          16

#ifdef __cplusplus
extern "C"
{
#endif

  extern struct I_Node *find_df_node (int id);
  extern struct I_Reg *find_df_reg (int reg_id);
  extern void define_df_graph (void);
  extern void add_df_node (int id);
  extern void add_df_node_src_reg (int reg_id);
  extern void add_df_node_dest_reg (int reg_id);
  extern void connect_df_node (int src_id, int dest_id);
  extern void end_df_graph (void);
  extern void analyze_df_graph (int mode);
  extern void print_df_node (FILE * F, struct I_Node *node);
  extern void print_df_graph (FILE * F);

#ifdef __cplusplus
}
#endif

#endif
