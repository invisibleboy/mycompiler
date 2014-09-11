/*****************************************************************************\
 *
 *                    Illinois Open Source License
 *                     University of Illinois/NCSA
 *                         Open Source License
 *
 * Copyright (c) 2004, The University of Illinois at Urbana-Champaign.
 * All rights reserved.
 *
 * Developed by:
 *
 *              IMPACT Research Group
 *
 *              University of Illinois at Urbana-Champaign
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


#ifndef __PLIB_SS_SETUP_H__
#define __PLIB_SS_SETUP_H__

#include <library/list.h>
#include <Pcode/pcode.h>
#include <Pcode/extension.h>
#include <Pcode/cfg.h>
#include <Pcode/loop.h>
#include "ss_induct.h"

/***************************************************************************/

extern void SS_SetUpExtension (void);
extern void SS_Init (PC_Graph cfg);

/***************************************************************************/

typedef struct _P_ExprExtForVgraph {
  PC_Block              bb;
  P_Vnode               vnode;
  P_Value               value;
}
_P_ExprExtForVgraph, *P_ExprExtForVgraph;

extern PC_Block Get_ExprExtForVgraph_bb         (Expr expr);
extern void     Set_ExprExtForVgraph_bb         (Expr expr, PC_Block bb);
extern P_Value  Get_ExprExtForVgraph_value      (Expr expr);
extern void     Set_ExprExtForVgraph_value      (Expr expr, P_Value value);
extern P_Vnode  Get_ExprExtForVgraph_vnode      (Expr expr);
extern void     Set_ExprExtForVgraph_vnode      (Expr expr, P_Vnode vnode);

extern P_ExprExtForVgraph P_GetExprExtForVgraph(Expr expr);

Extension P_ExprExtForVgraph_alloc ();
Extension P_ExprExtForVgraph_free (Extension x);

/***************************************************************************/

typedef struct _P_BlockExtForVgraph {
  Set lex_pred;                 /* lexical predecessor */
  PC_Loop loop;                 /* the enclosing loop */
}
_P_BlockExtForVgraph, *P_BlockExtForVgraph;

extern Set      Get_BlockExtForVgraph_lex_pred  (PC_Block bb);
extern void     Set_BlockExtForVgraph_lex_pred  (PC_Block bb, Set lex_pred);
extern PC_Loop  Get_BlockExtForVgraph_loop      (PC_Block bb);
extern void     Set_BlockExtForVgraph_loop      (PC_Block bb, PC_Loop loop);

extern P_BlockExtForVgraph P_GetBlockExtForVgraph (PC_Block bb);

extern void *P_BlockExtForVgraph_alloc();
extern void P_BlockExtForVgraph_free(void *x);

/***************************************************************************/

typedef struct _P_LoopExtForVgraph {
  #if 0
  int parloop_depth;
  #endif
  Expr cond_expr;
  struct _P_Value *condition;
  struct _P_Value *tripcount;
  struct _Alpha_var_id *iv;
  Lptr scc_list;
}
_P_LoopExtForVgraph, *P_LoopExtForVgraph;

extern Alpha_var_id Get_Loop_iv (PC_Loop loop);
extern void Set_Loop_iv (PC_Loop loop, Alpha_var_id iv);
extern Expr Get_Loop_cond_expr (PC_Loop loop);
extern void Set_Loop_cond_expr (PC_Loop loop, Expr expr);
extern P_Value Get_Loop_condition (PC_Loop loop);
extern void Set_Loop_condition (PC_Loop loop, P_Value value);
extern P_Value Get_Loop_tripcount (PC_Loop loop);
extern void Set_Loop_tripcount (PC_Loop loop, P_Value value);
#if 0
extern int Get_Loop_parloop_depth (PC_Loop loop);
extern void Set_Loop_parloop_depth (PC_Loop loop, int depth);
#endif
extern Lptr Get_Loop_scc_list (PC_Loop loop);
extern void Set_Loop_scc_list (PC_Loop loop, Lptr scc_list);
extern PC_Block Get_Loop_header_bb (PC_Loop loop, PC_Graph cfg);

extern void *P_LoopExtForVgraph_alloc();
extern void P_LoopExtForVgraph_free(void *x);
extern void P_CF_DumpLoopsInCFG (FILE *out_file, PC_Graph cfg);

#define Get_Loop_id(l)                  ((l)->ID)
#define Get_Loop_head(l)                ((l)->head)
#define Get_Loop_body(l)                ((l)->body)
#define Get_Loop_exits(l)               ((l)->exits)
#define Get_Loop_next(l)                ((l)->next)
#define Get_Loop_nesting_level(l)       ((l)->nesting_level)
#define Get_Loop_child(l)               ((l)->child)
#define Get_Loop_sibling(l)             ((l)->sibling)
#define Get_Loop_parent(l)              ((l)->parent)
#define Get_Loop_num_exit(l)            ((l)->num_exit)
#define Get_Loop_num_back_edges(l)      ((l)->num_back_edges)

/***************************************************************************/

extern bool CFGHasLoopDetected (PC_Graph cfg);
extern bool Loop1EnclosedInLoop2 (PC_Loop loop1, PC_Loop loop2);

#endif /* __PLIB_SS_SETUP_H__ */
