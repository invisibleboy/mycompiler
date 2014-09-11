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

#include "ss_ssa.h"
#include "ss_setup.h"
#include "ss_induct.h"

/***********************************************************/
/* static variables */

static int ss_expr_ext_idx = -1;

/***********************************************************/
/* static function headers */

static void SetBlockExtForVgraph (PC_Graph cfg);
static void SetLoopExtForVgraph (PC_Graph cfg);
static void SetExprExtForVgraph (PC_Graph cfg);
static void SetExprExtBB (Expr expr, PC_Block bb);
static void InitBlockExtForVgraph_loop(PC_Graph cfg, PC_Loop loop);
static void DumpLoop (FILE *out_file, PC_Loop loop, int level, PC_Graph cfg);

/***********************************************************/
void
SS_SetUpExtension (void)
{
  PS_def_handlers ();
  assert (ss_expr_ext_idx == -1);
  ss_expr_ext_idx = P_ExtSetupL (ES_EXPR, 
                                 (AllocHandler) P_ExprExtForVgraph_alloc, 
                                 (FreeHandler) P_ExprExtForVgraph_free);
}

void
SS_Init (PC_Graph cfg)
{
  SetBlockExtForVgraph (cfg);
  
  if (cfg->lp_tree)
    InitBlockExtForVgraph_loop(cfg, cfg->lp_tree); 

  SetLoopExtForVgraph (cfg);

  SetExprExtForVgraph (cfg);

}

/***********************************************************/
/* P_ExprExtForVgraph */

P_ExprExtForVgraph
P_GetExprExtForVgraph (Expr expr)
{
  return P_GetExprExtL (expr, ss_expr_ext_idx);
}

PC_Block
Get_ExprExtForVgraph_bb(Expr expr)
{ 
  P_ExprExtForVgraph x;
  
  x = P_GetExprExtForVgraph(expr);
  assert(x != NULL);
  return x->bb;
}

void
Set_ExprExtForVgraph_bb(Expr expr, PC_Block bb)
{
  P_ExprExtForVgraph x;

  x = P_GetExprExtForVgraph(expr);
  assert(x != NULL);
  x->bb = bb;
}

P_Value
Get_ExprExtForVgraph_value(Expr expr)
{
  P_ExprExtForVgraph x;

  x = P_GetExprExtForVgraph(expr);
  assert(x != NULL);
  return x->value;
}

void
Set_ExprExtForVgraph_value(Expr expr, P_Value value)
{
  P_ExprExtForVgraph x;

  x = P_GetExprExtForVgraph(expr);
  assert(x != NULL);
  x->value = value;
}

P_Vnode
Get_ExprExtForVgraph_vnode (Expr expr)
{
  P_ExprExtForVgraph x;

  x = P_GetExprExtForVgraph(expr);
  assert(x != NULL);
  return x->vnode;
}

void
Set_ExprExtForVgraph_vnode (Expr expr, P_Vnode vnode)
{
  P_ExprExtForVgraph x;

  x = P_GetExprExtForVgraph(expr);
  assert(x != NULL);
  x->vnode = vnode;
}

/***********************************************************/
/* P_BlockExtForVgraph */

P_BlockExtForVgraph
P_GetBlockExtForVgraph (PC_Block block)
{
  return block->ext;
} 

Set
Get_BlockExtForVgraph_lex_pred(PC_Block bb)
{
  P_BlockExtForVgraph x;

  x = P_GetBlockExtForVgraph (bb);
  assert (x != NULL);
  return x->lex_pred;
}

void
Set_BlockExtForVgraph_lex_pred(PC_Block bb, Set lex_pred)
{
  P_BlockExtForVgraph x;

  x = P_GetBlockExtForVgraph (bb);
  assert (x != NULL);
  x->lex_pred = lex_pred;
}

PC_Loop
Get_BlockExtForVgraph_loop(PC_Block bb)
{
  P_BlockExtForVgraph x;

  x = P_GetBlockExtForVgraph (bb);
  assert (x != NULL);
  return x->loop;
}

void
Set_BlockExtForVgraph_loop(PC_Block bb, PC_Loop loop)
{
  P_BlockExtForVgraph x;

  x = P_GetBlockExtForVgraph (bb);
  assert (x != NULL);
  x->loop = loop;
}

/***********************************************************/
/* P_LoopExtForVgraph */

Alpha_var_id
Get_Loop_iv (PC_Loop loop)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  return x->iv;
}

void
Set_Loop_iv (PC_Loop loop, Alpha_var_id iv)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  x->iv = iv;
}

Expr
Get_Loop_cond_expr (PC_Loop loop)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  return x->cond_expr;
}

void
Set_Loop_cond_expr (PC_Loop loop, Expr expr)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  x->cond_expr = expr;
}

P_Value
Get_Loop_condition (PC_Loop loop)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  return x->condition;
}

void
Set_Loop_condition (PC_Loop loop, P_Value value)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  x->condition = value;
}

P_Value
Get_Loop_tripcount (PC_Loop loop)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  return x->tripcount;
}

void
Set_Loop_tripcount (PC_Loop loop, P_Value value)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  x->tripcount = value;
}

#if 0
int
Get_Loop_parloop_depth (PC_Loop loop)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  return x->parloop_depth;
}

void
Set_Loop_parloop_depth (PC_Loop loop, int depth)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  x->parloop_depth = depth;
}
#endif

Lptr
Get_Loop_scc_list (PC_Loop loop)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  return x->scc_list;
}

void
Set_Loop_scc_list (PC_Loop loop, Lptr scc_list)
{
  P_LoopExtForVgraph x;

  x = loop->ext;
  assert (x != NULL);
  x->scc_list = scc_list;
}

void *
P_BlockExtForVgraph_alloc ()
{
  P_BlockExtForVgraph x;

  x = ALLOCATE (_P_BlockExtForVgraph);
  x->lex_pred = NULL;
  x->loop = NULL;
  return x;
}

void 
P_BlockExtForVgraph_free (void *x)
{
  DISPOSE (x);
}

void *
P_LoopExtForVgraph_alloc ()
{
  P_LoopExtForVgraph x;

  x = ALLOCATE (_P_LoopExtForVgraph);
  #if 0
  x->parloop_depth = -1;
  #endif
  x->cond_expr = NULL;
  x->condition = NULL;
  x->tripcount = NULL;
  x->iv = NULL;
  x->scc_list = NULL;
  return x;
}

void 
P_LoopExtForVgraph_free (void *x)
{
  DISPOSE(x);
}

/***********************************************************/

PC_Block
Get_Loop_header_bb (PC_Loop loop, PC_Graph cfg)
{
  return PC_FindBlock (cfg, Get_Loop_head(loop));
}

bool
CFGHasLoopDetected (PC_Graph cfg)
{
  return (cfg->lp != NULL);
}

bool
Loop1EnclosedInLoop2 (PC_Loop loop1, PC_Loop loop2)
{
  assert (loop1 && loop2);
  if (loop1 == loop2)
    return 0;
  while (loop1) {
    loop1 = Get_Loop_parent(loop1);
    if (loop1 == loop2)
      return 1;
  }
  return 0;
}

void
P_CF_DumpLoopsInCFG (FILE *out_file, PC_Graph cfg)
{
  if (cfg->lp_tree)
    DumpLoop (out_file, cfg->lp_tree, 0, cfg);
  else
    fprintf(out_file, "\nNO LOOP\n");
}

/***********************************************************/
/* static functions */

Extension
P_ExprExtForVgraph_alloc (void)
{
  P_ExprExtForVgraph x;

  x = ALLOCATE (_P_ExprExtForVgraph);
  x->bb = NULL;
  x->vnode = NULL;
  x->value = NULL;
  return x;
}

Extension
P_ExprExtForVgraph_free (Extension x)
{
  DISPOSE (x);
  return NULL;
}

static void
SetBlockExtForVgraph (PC_Graph cfg)
{
  PC_Block bb;

  for (bb = cfg->first_bb ; bb ; bb = bb->next) {
    /* For now, there should be no use of the ext field of Block */
    assert (bb->ext == NULL);
    bb->ext = P_BlockExtForVgraph_alloc ();
  }
}

static void
SetLoopExtForVgraph (PC_Graph cfg)
{
  PC_Loop loop;

  for (loop = cfg->lp ; loop ; loop = loop->next) {
    /* For now, there should be no use of the ext field of Loop */
    assert (loop->ext == NULL);
    loop->ext = P_LoopExtForVgraph_alloc (); 
  }
}

static void
SetExprExtBB(Expr expr, PC_Block bb)
{
  if (expr) {
    Set_ExprExtForVgraph_bb (expr, bb);
    SetExprExtBB(P_GetExprNext(expr), bb);
    SetExprExtBB(P_GetExprOperands(expr), bb);
    SetExprExtBB(P_GetExprSibling(expr), bb);
  } 
}

static void
SetExprExtForVgraph (PC_Graph cfg)
{
  PC_Block bb;
  _PC_ExprIter ei;
  Expr expr;
  
  for (bb = cfg->first_bb; bb; bb = bb->next) {
    for (expr = PC_ExprIterFirst(bb, &ei, 1); expr; expr = PC_ExprIterNext(&ei, 1)) {
      SetExprExtBB(expr, bb);
    }
  }
}

static void
InitBlockExtForVgraph_loop(PC_Graph cfg, PC_Loop loop)
{
  int i;
  int *bb_id;
  int num_of_bb;
  PC_Block bb;

  bb_id = (int *) calloc(cfg->num_bbs, sizeof(int));
  if (bb_id == NULL)
    P_punt ("InitBlockExtForVgraph_loop");
  num_of_bb = Set_2array(loop->body, bb_id);
  for (i = 0 ; i < num_of_bb ; i++) {
    bb = PC_FindBlock(cfg, bb_id[i]);
    Set_BlockExtForVgraph_loop (bb, loop);
  }
  free(bb_id);
  if (loop->child)
    InitBlockExtForVgraph_loop(cfg, loop->child);
  if (loop->sibling)
    InitBlockExtForVgraph_loop(cfg, loop->sibling);
}

static void 
Dump_Indent (FILE *out_file, int level)
{ 
  int i;
  
  for (i = 0 ; i < level ; i++)
    fprintf(out_file, "    ");
}

static void
Dump_BB_Set (FILE *out_file, Set s, PC_Graph cfg)
{ 
  int i;
  int num_of_bb;
  int *bb_id; 
  
  bb_id = (int *) calloc(cfg->num_bbs, sizeof(int));
  if (bb_id == NULL)
    P_punt ("Dump_BB_Set");
  num_of_bb = Set_2array(s, bb_id);
  for (i = 0 ; i < num_of_bb ; i++)
    fprintf(out_file, "bb(%d) ", bb_id[i]);
  free(bb_id);
}

static void
DumpLoop (FILE *out_file, PC_Loop loop, int level, PC_Graph cfg)
{
  Dump_Indent(out_file, level);
  fprintf(out_file, "LOOP (%d): {\n", Get_Loop_id(loop)); 

  #if 0
  Dump_Indent(out_file, level+1);
  fprintf(out_file, "type = %s\n", LoopTypeString[loop->loop_type]);
  #endif

  Dump_Indent(out_file, level+1); 
  fprintf(out_file, "nesting_level = %d\n", Get_Loop_nesting_level(loop));

  Dump_Indent(out_file, level+1);
  fprintf(out_file, "num_exit = %d\n", Get_Loop_num_exit(loop));

  Dump_Indent(out_file, level+1);
  fprintf(out_file, "num_back_edge = %d\n", Get_Loop_num_back_edge(loop));

  Dump_Indent(out_file, level+1);
  fprintf(out_file, "header = bb(%d)\n", Get_Loop_head(loop));

  Dump_Indent(out_file, level+1);
  fprintf(out_file, "loop_bbs =  ");
  Dump_BB_Set(out_file, Get_Loop_body(loop), cfg);
  fprintf(out_file, "\n");

  #if 0
  Dump_Indent(out_file, level+1);
  fprintf(out_file, "back_edge_bbs = ");
  Dump_BB_Ptr_List(out_file, loop->back_edge_bbs);
  fprintf(out_file, "\n");
  #endif

  Dump_Indent(out_file, level+1);
  fprintf(out_file, "exit_bbs = ");
  Dump_BB_Set(out_file, Get_Loop_exits(loop), cfg);
  fprintf(out_file, "\n");

  #if 0
  Dump_Indent(out_file, level+1);
  fprintf(out_file, "out_bbs = ");
  Dump_BB_Ptr_List(out_file, loop->out_bbs);
  fprintf(out_file, "\n");
  #endif

  Dump_Indent(out_file, level+1);
  if (Get_Loop_parent(loop))
    fprintf(out_file, "parent = LOOP(%d)\n", Get_Loop_id(Get_Loop_parent(loop)));
  else
    fprintf(out_file, "parent = NULL\n");

  Dump_Indent(out_file, level+1);
  fprintf(out_file, "trip_count = ");
  P_SS_DumpValue(out_file, Get_Loop_tripcount(loop));
  fprintf(out_file, "\n");

  #if 0
  Dump_Indent(out_file, level+1);
  if (loop->pcode_hdr) {
    fprintf(out_file, "pcode_hdr = %s Stmt ", StmtTypeString[loop->pcode_hdr->type]);
    if (loop->pcode_hdr->type == ST_SERLOOP)
      fprintf(out_file, "(%s)", SerLoopTypeString[loop->pcode_hdr->stmtstruct.serloop->loop_type]);
  } else
    fprintf(out_file, "pcode_hdr = NULL");
  fprintf(out_file, "\n");
  #endif


  if (Get_Loop_child(loop)) {
    fprintf(out_file, "\n");
    DumpLoop(out_file, Get_Loop_child(loop), level+1, cfg);
  }

  Dump_Indent(out_file, level);
  fprintf(out_file, "}\n");

  if (Get_Loop_sibling(loop)) {
    fprintf(out_file, "\n");
    DumpLoop(out_file, Get_Loop_sibling(loop), level, cfg);
  }
}

