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


/*-----------------------------------------------------------------------------*/

#include <config.h>
#include <string.h>
#include <library/set.h>
#include <library/llist.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/parms.h>
#include <Pcode/reduce.h>
#include <Pcode/symtab_i.h>
#include <Pcode/parloop.h>
#include <Pcode/write.h>

#include "cfg.h"
#include "dom.h"
#include "loop.h"

#define UNIQUE_BB_IDs
#define PRINT_DOMS 1
#define GOTO_TABLE_SIZE 400
#define BB_TABLE_SIZE 50
#define PC_BB_HASH_SIZE 128
#define PC_PRINT_GRAPHS 0
#define PC_EXPR_BB_HASH_SIZE 2048


/* Global CFG options parsed from PC_Function parameter */

int PC_opt_split_crit = 0;
int PC_opt_annotate = 0;
int PC_opt_lp_norm = 0;

/* Structures used only in cfg.c */

typedef enum _PC_ProfType
{
  PC_PRF_ST = 0,
  PC_PRF_FN = 1
}
_PC_ProfType;

typedef struct _PC_SwitchContext
{
  PC_Block cond_bb;
  PC_Block exit_bb;
  double weight;
  bool def_present;
  Stmt sw_st;
  ProfST p_ptr;
}
_PC_SwitchContext, *PC_SwitchContext;

typedef struct _PC_GotoEntry
{
  PC_Block goto_bb;
  PC_Block target_bb;
  Label goto_lab;
  Label target_lab;
  struct _PC_GotoEntry *next;
}
_PC_GotoEntry, *PC_GotoEntry;


/* Global variables */

FILE *fout;
FILE *fcfg;
FILE *fbug;
FILE *fapp;
FILE *fprf;

PC_Graph PC_cfg;

/* Global variables used only for debugging */
#if DEBUG
#define LP_HDRS_SIZE 10000
int LP_HDRS[LP_HDRS_SIZE];
#endif

/* Pointer to top switch context used in PC_Graph consruction. */

static PC_SwitchContext PC_sw_ctx;
static List PC_sw_list;

PC_GotoEntry *GotoTable;	/* hash table to store gotos & target labels. */

int total_bbs = 0;		/* global counter for total bbs in the app. */
int total_lps = 0;		/* global counter for total loops in the app. */

int TABLENGTH = 1;
char *TAB = "  ";

typedef Expr BBExprEnum;

int WHITE = 0;
int RED = 1;
int GREEN = 2;
int BLUE = 3;
int CURR_COLOR = 0;
int NUM_COLORS = 4;

static PC_Block PC_Stmt (PC_Graph cfg, Stmt stmt, PC_Block curr_bb);

static PC_Block PC_StmtList (PC_Graph cfg, Stmt stmt_list, PC_Block curr_bb,
			     int parloop_flag);

static void PC_InitSWContext (PC_SwitchContext swc, PC_Block cond_bb,
			      PC_Block exit_bb, Stmt st, ProfST prf);

PC_GotoEntry NewPC_GotoEntry (PC_Block gotoBlock, PC_Block targetBlock,
			      Label gotolabel, Label targetLabel);

bool PC_SetLabBB (PC_Block * curr_bb, Stmt stmt);
void AppendPC_Flow_Succ (PC_Flow pc_flow, PC_Block p_bb);
void AppendPC_Flow_Pred (PC_Flow pc_flow, PC_Block s_bb);

bool PC_ConnToTarget (PC_Block, Label label);
void PC_AddGoto (PC_Block bb, Label label);
bool PC_ConnToGotos (PC_Block bb, Label label);
void PC_AddTarget (PC_Block bb, Label label);

void PC_SetProfPtr (PC_Block bb, ProfST * prof, int prf_typ);

PC_Flow PC_Connect (PC_Block pred, PC_Block succ, Expr cond, double weight);
void PC_RemoveFlow (PC_Flow pc_flow);
void PC_ChangeFlowSrc (PC_Flow pc_flow, PC_Block new_pred);
void PC_ChangeFlowDest (PC_Flow pc_flow, PC_Block new_succ);
bool IsPredBlock (PC_Block pred, PC_Block bb);
bool IsSuccBlock (PC_Block succ, PC_Block bb);

PC_LpPrag PC_NewLpPrag (PC_Block bb, Stmt stmt, Pragma * prg);
PC_LpPrag PC_FreeLpPrag (PC_LpPrag lp_prg);
PC_GotoEntry *PC_FreeGotoTable (PC_GotoEntry * GotoTable);
PC_GotoEntry PC_FreeGotoEntry (PC_GotoEntry g);

void PC_Init ();
void PC_Terminate ();
void ChangeCurrColor ();
void PC_TraverseGraph (PC_Block bb, void (*fPtr) (PC_Block));
void PC_PrintPStmts (FILE * file, PC_Graph cfg);	/*print all P_Stmts in PC_Graph */
void PC_PrintBlock (PC_Block bb);	/* print all P_Stmts in PC_Block */
static void PC_SplitCritOutEdges (PC_Graph cfg, PC_Block bb);
/* split bb into two */
PC_Block PC_RevBlockList (PC_Block h_bb);
void PC_VerifyProfileInfo (PC_Graph cfg);
Set PC_BuildUnReachableBBSet (PC_Graph cfg);

static void PC_AddExprToBBHash(PC_Block bb, Expr expr);


void PC_FlowUnlink_Succ (PC_Flow fl);



/*
 * PC_Stmt_* Routines (called from PC_Stmt() only)
 * ----------------------------------------------------------------------
 * Process all statement types into control flow subgraphs.
 */

static PC_Block
PC_Stmt_Asm (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  PC_PStmt ps;
  ps = PC_NewPStmt (PC_T_NatPcode, bb, stmt);
  return bb;
}


static PC_Block
PC_Stmt_Break (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  if (!PC_sw_ctx)
    P_punt ("PC_sw_ctx is NULL while processing break stmt.");

  bb->cont_type = CNT_BREAK;
  PC_Connect (bb, PC_sw_ctx->exit_bb, TrueExpr (), bb->weight);

  PC_sw_ctx->exit_bb->weight += bb->weight;

  /* A break Stmt has no fall-through. */
  return NULL;
}


static PC_Block
PC_Stmt_Compound (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  bb = PC_StmtList (cfg, stmt->stmtstruct.compound->stmt_list, bb, FALSE);
  return bb;
}


static PC_Block
PC_Stmt_Expr (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  PC_PStmt ps;
  ps = PC_NewPStmt (PC_T_NatPcode, bb, stmt);
  return bb;
}


/*! \brief Processes a goto stmt during CFG construction
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt
 *  pcode stmt (in this case a pcode goto stmt)
 * \param bb
 *  PC_Block which will hold this goto stmt
 *
 * \return
 *  PC_Block to link to next pcode stmt (in this case NULL)
 *
 * Searches the GotoTable for a target label for this goto stmt. If one is found
 * connects the goto and the label, otherwise adds the goto to GotoTable. 
 */
static PC_Block
PC_Stmt_Goto (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  Label goto_lab;

  goto_lab = P_NewLabel ();
  goto_lab->val = strdup (stmt->stmtstruct.label.val);

  bb->cont_type = CNT_GOTO;

  if (PC_ConnToTarget (bb, goto_lab))
    {
#if DEBUG
      fprintf (fbug, "Goto bb %d connected to target block.\n", bb->ID);
#endif
    }
  else
    {
      PC_AddGoto (bb, goto_lab);
    }

  DISPOSE (goto_lab->val);
  goto_lab->val = NULL;
  P_FreeLabel (goto_lab);

  /* A goto Stmt has no fall-through. */
  return NULL;
}


/*! \brief Processes an if stmt during PC_Graph construction
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt
 *  pcode stmt (in this case a pcode if stmt)
 * \param curr_bb
 *  PC_Block which will hold the condition of this if stmt
 *
 * \return
 *  exit PC_Block of this if stmt
 *
 * Creates PC_Blocks for 'then' and 'else' parts, links if cond PC_Block to both 
 * of these, and calls PC_Stmt () on both of them. The PC_Blocks returned from 
 * these two calls of PC_Stmt () are linked to the exit PC_Block of the if stmt.
 * Also, does profile maintenance for the if stmt.
 *
 */
static PC_Block
PC_Stmt_If (PC_Graph cfg, Stmt stmt, PC_Block curr_bb)
{
  double wt_then = 0.0, wt_else = 0.0;
  PC_Block cond_bb = curr_bb, then_bb = NULL, else_bb = NULL, exit_bb = NULL;
  Stmt st_then, st_else;

  cond_bb->cont_type = CNT_IF;
  cond_bb->cond = stmt->stmtstruct.ifstmt->cond_expr;

  PC_AddExprToBBHash(cond_bb, cond_bb->cond);

  if (!cond_bb->cond)
    P_punt ("PC_Stmt_If: malformed if");


  /* Extract profile weights for then, else arcs. */
  if (stmt->profile && stmt->profile->next && stmt->profile->next->next)
    {
      wt_then = stmt->profile->next->count;
      wt_else = stmt->profile->next->next->count;
    }

  st_then = stmt->stmtstruct.ifstmt->then_block;
  st_else = stmt->stmtstruct.ifstmt->else_block;

  /* Create then, else headers */

  then_bb = PC_NewBlock (cfg, wt_then);
  PC_Connect (cond_bb, then_bb, TrueExpr (), wt_then);
  else_bb = PC_NewBlock (cfg, wt_else);
  PC_Connect (cond_bb, else_bb, FalseExpr (), wt_else);

  /* IfStmt profile->next is "then" wt and profile->next->next is "else" wt */

  if (PC_opt_annotate)
    {
      PC_SetProfPtr (then_bb, &(stmt->profile->next), PC_PRF_ST);
      PC_SetProfPtr (else_bb, &(stmt->profile->next->next), PC_PRF_ST);
    }

  /* Process "then" Stmt and "else" Stmt, if it exists. */

  then_bb = PC_Stmt (cfg, st_then, then_bb);

  if (st_else)
    else_bb = PC_Stmt (cfg, st_else, else_bb);

  /* Set up exit BB and connect fall-through arcs if they exist. */

  if (then_bb || else_bb)
    {
      exit_bb = PC_NewBlock (cfg, 0.0);

      if (then_bb)
	{
	  then_bb->cont_type = CNT_GOTO;
	  PC_Connect (then_bb, exit_bb, TrueExpr (), then_bb->weight);
	  exit_bb->weight += then_bb->weight;
	}

      if (else_bb)
	{
	  else_bb->cont_type = CNT_GOTO;
	  PC_Connect (else_bb, exit_bb, TrueExpr (), else_bb->weight);
	  exit_bb->weight += else_bb->weight;
	}
    }
  else
    {
      exit_bb = NULL;
    }

  return exit_bb;
}


static PC_Block
PC_Stmt_Noop (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  PC_PStmt ps;
  ps = PC_NewPStmt (PC_T_NatPcode, bb, stmt);
  return bb;
}


static PC_Block
PC_Stmt_Return (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  PC_PStmt ps;
  ps = PC_NewPStmt (PC_T_NatPcode, bb, stmt);

  bb->cont_type = CNT_RETURN;
  PC_Connect (bb, cfg->exit_bb, TrueExpr (),
	      stmt->profile ? stmt->profile->count : 0.0);

  /* A return Stmt has no fall-through */
  return NULL;
}


/*! \brief Processes a parallel loop stmt during CFG construction
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt
 *  pcode stmt (in this case a parallel loop stmt)
 * \param curr_bb
 *  PC_Block which will hold the init cond of this loop stmt
 *
 * \return
 *  exit PC_Block of this loop stmt
 *
 * Creates prologue and epilogue PC_Blocks to hold extra parallelization stmts, 
 * and creates body PC_Block to hold loop body.  Calls PC_StmtList () to process
 * loop prologue. Calls PC_Stmt () to process loop body and epilogue.  Also, does
 * profile maintenance for the loop stmt. 
 */
static PC_Block
PC_Stmt_Parloop (PC_Graph cfg, Stmt stmt, PC_Block curr_bb)
{
  PC_Block init_bb = curr_bb, pro_bb = NULL, body_bb = NULL, iter_bb = NULL;
  PC_Block epi_bb = NULL, exit_bb = NULL, auxepi_bb = NULL;
  double wt_exit = 0.0;
  PC_PStmt ps = NULL;

  Stmt comp_stmt = NULL, body_stmt = NULL, epilogue_stmt = NULL;
  Expr ex_init, ex_cond, ex_iter;

  ex_init = stmt->stmtstruct.parloop->init_value;
  ex_cond = stmt->stmtstruct.parloop->final_value;
  ex_iter = stmt->stmtstruct.parloop->incr_value;

  if (!ex_cond)
    {
      ex_cond = TrueExpr ();
      if (PC_opt_annotate)
	stmt->stmtstruct.parloop->final_value = ex_cond;
      P_warn ("PC_Stmt_Parloop: ex_cond is NULL.");
    }

  if (stmt->profile && stmt->profile->next)
    wt_exit = stmt->profile->next->count;

  ps = PC_NewPStmtExpr (PC_T_Expr, init_bb, ex_init);

  comp_stmt = Parloop_Stmts_Prologue_Stmt (stmt);
  assert (comp_stmt != NIL && comp_stmt->type == ST_COMPOUND);

  pro_bb = PC_NewBlock (cfg, comp_stmt->profile ?
			comp_stmt->profile->count : 0.0);

  PC_Connect (init_bb, pro_bb, TrueExpr (), init_bb->weight);
  pro_bb = PC_StmtList (cfg, comp_stmt->stmtstruct.compound->stmt_list,
			pro_bb, TRUE);

  body_stmt = Parloop_Stmts_Body_Stmt (stmt);
  assert (body_stmt != NIL && body_stmt->type == ST_BODY);

  /*wt to be added while processing body stmts */
  body_bb = PC_NewBlock (cfg, 0.0);
  PC_Connect (pro_bb, body_bb, TrueExpr (), pro_bb->weight);
  iter_bb =
    PC_NewBlock (cfg, ex_cond->profile ? ex_cond->profile->count : 0.0);

  PC_Connect (iter_bb, body_bb, TrueExpr (), iter_bb->weight - wt_exit);
  body_bb = PC_Stmt (cfg, body_stmt->stmtstruct.bodystmt->statement, body_bb);


  iter_bb->cont_type = CNT_IF;
  iter_bb->cond = ex_cond;

  PC_AddExprToBBHash(iter_bb, iter_bb->cond);

  PC_Connect (body_bb, iter_bb, TrueExpr (), body_bb->weight);

  epilogue_stmt = Parloop_Stmts_First_Epilogue_Stmt (stmt);
  assert (epilogue_stmt != NULL && epilogue_stmt->type == ST_EPILOGUE);
  epi_bb =
    PC_NewBlock (cfg,
		 epilogue_stmt->profile ? epilogue_stmt->profile->
		 count : 0.0);

  ps = PC_NewPStmt (PC_T_NatPcode, epi_bb, epilogue_stmt);

  PC_Connect (iter_bb, epi_bb, FalseExpr (), wt_exit);

  exit_bb = PC_NewBlock (cfg, wt_exit);

  PC_Connect (epi_bb, exit_bb, TrueExpr (), wt_exit);

  /* Process auxiliary epilogues. */
  epilogue_stmt = epilogue_stmt->lex_next;
  while (epilogue_stmt != NULL)
    {
      assert (epilogue_stmt->type == ST_EPILOGUE);
#if 0
      /*wt to be added while processing stmt */
      auxepi_bb = PC_NewBlock (cfg, 0.0);
      ps = PC_NewPStmt (PC_T_NatPcode, auxepi_bb, epilogue_stmt);
#endif
      auxepi_bb =
	PC_Stmt (cfg, epilogue_stmt->stmtstruct.epiloguestmt->statement,
		 auxepi_bb);

      if (auxepi_bb != NULL && auxepi_bb->cont_type != CNT_GOTO)
	P_punt ("PC_Stmt_Parloop: Aux Epilogue must end with goto.");

      epilogue_stmt = epilogue_stmt->lex_next;
    }


  /* create profile fields and save ptrs to them for use during annotation. */
  if (PC_opt_annotate)
    {
      PC_SetProfPtr (exit_bb, &(stmt->profile->next), PC_PRF_ST);
      /* create a PC_LpPrag and copy stmt pragma info into it */
      iter_bb->lp_prg = PC_NewLpPrag (iter_bb, stmt, &(ex_cond->pragma));
    }

  /* save ptrs to pcode lp pragma for transfering to lcode during PtoL. */
  if (ex_cond->pragma)
    iter_bb->pragma = ex_cond->pragma;

  return exit_bb;
}


/*! \brief Processes a serial for/while/do-while stmt during CFG construction
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt
 *  pcode stmt (in this case a serial for/while/do-while stmt)
 * \param curr_bb
 *  PC_Block which will hold the init cond of this loop stmt
 *
 * \return
 *  exit PC_Block of this loop stmt
 *
 * Creates PC_Blocks for the 'test cond' and 'body' of the loop. Calls
 * PC_Stmt () on the 'body' PC_Block, and links the return value to
 * 'exit' PC_Block. Both for/while and do-while loops are processed by
 * the same code, except that init PC_Block links differently
 * depending upon loop type. The test PC_Block serves as the loop
 * header in for/while loops, while in do-while loops the body
 * PC_Block is the loop header. Also, does profile maintenance for the
 * loop stmt.
 */
static PC_Block
PC_Stmt_Serloop (PC_Graph cfg, Stmt stmt, PC_Block curr_bb)
{
  PC_Block init_bb = curr_bb, test_bb, body_bb, body_ft_bb, exit_bb;
  Expr ex_init, ex_cond, ex_iter;
  Stmt st_body;
  double wt_init = 0.0, wt_body, wt_iter = 0.0, wt_cond = 0.0, wt_exit = 0.0;
  _SerLoopType loop_type = stmt->stmtstruct.serloop->loop_type;

  ex_init = stmt->stmtstruct.serloop->init_expr;
  ex_cond = stmt->stmtstruct.serloop->cond_expr;
  ex_iter = stmt->stmtstruct.serloop->iter_expr;
  st_body = stmt->stmtstruct.serloop->loop_body;

  if (loop_type != LT_WHILE && loop_type != LT_FOR && loop_type != LT_DO)
    P_punt ("PC_Stmt_Serloop(): Undefined loop type");

  if ((ex_init || ex_iter) && loop_type != LT_FOR)
    P_punt ("PC_Stmt_Serloop(): Non-for loop with an init/iter expr");

  /* Extract various flow weights from Pcode profile data */

  wt_init = stmt->profile ? stmt->profile->count : 0.0;
  wt_body = (st_body && st_body->profile) ? st_body->profile->count : 0.0;
  wt_cond = (ex_cond && ex_cond->profile) ? ex_cond->profile->count : 0.0;

  if (stmt->profile && stmt->profile->next)
    wt_exit = stmt->profile->next->count;
  if (ex_cond && ex_cond->profile)
    wt_iter = ex_cond->profile->count;

  init_bb->cont_type = CNT_GOTO;

  if (ex_init)
    PC_NewPStmtExpr (PC_T_Expr, init_bb, ex_init);

  body_bb = PC_NewBlock (cfg, wt_body);
  test_bb = PC_NewBlock (cfg, wt_cond);

  if (!ex_cond)
    {
      ex_cond = TrueExpr ();
      if (PC_opt_annotate)
	stmt->stmtstruct.serloop->cond_expr = ex_cond;
      P_warn ("PC_Stmt_Serloop: ex_cond is NULL.");
    }

  test_bb->cont_type = CNT_IF;
  test_bb->cond = ex_cond;

  PC_AddExprToBBHash(test_bb, test_bb->cond);

  exit_bb = PC_NewBlock (cfg, wt_exit);

  PC_Connect (test_bb, body_bb, TrueExpr (), wt_iter - wt_exit);
  PC_Connect (test_bb, exit_bb, FalseExpr (), wt_exit);

  PC_Connect (init_bb, (loop_type == LT_DO) ? body_bb : test_bb,
	      TrueExpr (), wt_init);

  body_ft_bb = PC_Stmt (cfg, st_body, body_bb);

  if (ex_iter)
    {
      if (body_ft_bb)
	PC_NewPStmtExpr (PC_T_Expr, body_ft_bb, ex_iter);
      else
	P_warn ("PC_Stmt_Serloop: apparently unreachable iter expr");
    }

  /* If body has a fall-through, connect to test_bb */
  if (body_ft_bb)
    {
      body_ft_bb->cont_type = CNT_GOTO;
      PC_Connect (body_ft_bb, test_bb, TrueExpr (), body_ft_bb->weight);
    }

  /* create profile fields and save ptrs to them for use during annotation. */
  if (PC_opt_annotate)
    {
      PC_SetProfPtr (exit_bb, &(stmt->profile->next), PC_PRF_ST);
      /* create a PC_LpPrag and copy stmt pragma info into it */
      if (loop_type == LT_FOR || loop_type == LT_WHILE)
	test_bb->lp_prg = PC_NewLpPrag (test_bb, stmt, &(ex_cond->pragma));
      else if (loop_type == LT_DO)
	body_bb->lp_prg = PC_NewLpPrag (body_bb, stmt, &(ex_cond->pragma));
    }

  /* save ptrs to pcode lp pragma for transfering to lcode during PtoL. */
  if (ex_cond->pragma)
    {
      if (loop_type == LT_FOR || loop_type == LT_WHILE)
	test_bb->pragma = ex_cond->pragma;
      else if (loop_type == LT_DO)
	body_bb->pragma = ex_cond->pragma;
    }

  return exit_bb;
}


/*! \brief Processes a serial for/while/do-while stmt during CFG construction,
 *  producing a normalized structure (test enclosing a do loop)
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt
 *  pcode stmt (in this case a serial for/while/do-while stmt)
 * \param curr_bb
 *  PC_Block which will hold the init cond of this loop stmt
 *
 * \return
 *  exit PC_Block of this loop stmt
 *
 * Creates PC_Blocks for the 'test cond' and 'body' of the loop. Calls
 * PC_Stmt () on the 'body' PC_Block, and links the return value to
 * 'exit' PC_Block. Both for/while and do-while loops are processed by
 * the same code, except that init PC_Block links differently
 * depending upon loop type. The test PC_Block serves as the loop
 * header in for/while loops, while in do-while loops the body
 * PC_Block is the loop header. Also, does profile maintenance for the
 * loop stmt.
 */
static PC_Block
PC_Stmt_Serloop_Norm (PC_Graph cfg, Stmt stmt, PC_Block curr_bb)
{
  PC_Block init_bb = curr_bb, test_bb, body_bb, body_ft_bb, exit_bb;
  Expr ex_init, ex_cond, ex_iter;
  Stmt st_body;
  double wt_init = 0.0, wt_body, wt_iter = 0.0, wt_cond = 0.0, wt_exit = 0.0;
  _SerLoopType loop_type = stmt->stmtstruct.serloop->loop_type;

  /* A DO loop is already normalized */
  if (loop_type == LT_DO)
    return PC_Stmt_Serloop (cfg, stmt, curr_bb);

  ex_init = stmt->stmtstruct.serloop->init_expr;
  ex_cond = stmt->stmtstruct.serloop->cond_expr;
  ex_iter = stmt->stmtstruct.serloop->iter_expr;
  st_body = stmt->stmtstruct.serloop->loop_body;

  if (loop_type != LT_WHILE && loop_type != LT_FOR)
    P_punt ("PC_Stmt_Serloop(): Undefined loop type");

  if ((ex_init || ex_iter) && loop_type != LT_FOR)
    P_punt ("PC_Stmt_Serloop(): Non-for loop with an init/iter expr");

  /* Extract various flow weights from Pcode profile data */

  wt_init = stmt->profile ? stmt->profile->count : 0.0;
  wt_body = (st_body && st_body->profile) ? st_body->profile->count : 0.0;
  wt_cond = (ex_cond && ex_cond->profile) ? ex_cond->profile->count : 0.0;

  if (stmt->profile && stmt->profile->next)
    wt_exit = stmt->profile->next->count;
  if (ex_cond && ex_cond->profile)
    wt_iter = ex_cond->profile->count;

  if (ex_init)
    PC_NewPStmtExpr (PC_T_Expr, init_bb, ex_init);

  body_bb = PC_NewBlock (cfg, wt_body);
  test_bb = PC_NewBlock (cfg, wt_cond);

  if (!ex_cond)
    {
      ex_cond = TrueExpr ();
      if (PC_opt_annotate)
	stmt->stmtstruct.serloop->cond_expr = ex_cond;
      P_warn ("PC_Stmt_Serloop: ex_cond is NULL.");
    }

  init_bb->cont_type = CNT_IF;
  init_bb->cond = ex_cond;
  PC_AddExprToBBHash(init_bb, init_bb->cond);

  test_bb->cont_type = CNT_IF;
  test_bb->cond = ex_cond;
  PC_AddExprToBBHash(test_bb, test_bb->cond);

  exit_bb = PC_NewBlock (cfg, wt_exit);

  PC_Connect (test_bb, body_bb, TrueExpr (), wt_iter - wt_exit);
  PC_Connect (test_bb, exit_bb, FalseExpr (), wt_exit);

  /* JWS: Profile approximation! */

  PC_Connect (init_bb, body_bb, TrueExpr (), wt_init);
  PC_Connect (init_bb, exit_bb, FalseExpr (), 0.0);

  body_ft_bb = PC_Stmt (cfg, st_body, body_bb);

  if (ex_iter)
    {
      if (body_ft_bb)
	PC_NewPStmtExpr (PC_T_Expr, body_ft_bb, ex_iter);
      else
	P_warn ("PC_Stmt_Serloop: apparently unreachable iter expr");
    }

  /* If body has a fall-through, connect to test_bb */
  if (body_ft_bb)
    {
      body_ft_bb->cont_type = CNT_GOTO;
      PC_Connect (body_ft_bb, test_bb, TrueExpr (), body_ft_bb->weight);
    }

  /* create profile fields and save ptrs to them for use during annotation. */
  if (PC_opt_annotate)
    {
      PC_SetProfPtr (exit_bb, &(stmt->profile->next), PC_PRF_ST);
      /* create a PC_LpPrag and copy stmt pragma info into it */
      if (loop_type == LT_FOR || loop_type == LT_WHILE)
	test_bb->lp_prg = PC_NewLpPrag (test_bb, stmt, &(ex_cond->pragma));
      else if (loop_type == LT_DO)
	body_bb->lp_prg = PC_NewLpPrag (body_bb, stmt, &(ex_cond->pragma));
    }

  /* save ptrs to pcode lp pragma for transfering to lcode during PtoL. */
  if (ex_cond->pragma)
    body_bb->pragma = ex_cond->pragma;

  /* KVM : Any pragma in source code before a for loop gets attached to
   * the parent statement. I am copying that list to the body_bb here.
   * pl_func now sees the innerloop pragma
   */
  if (stmt->pragma)
    body_bb->pragma = P_AppendPragmaNext(body_bb->pragma, P_CopyPragma(stmt->pragma));

  return exit_bb;
}


/*! \brief Processes a switch stmt during PC_Graph construction
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt
 *  pcode stmt (in this case a pcode switch stmt)
 * \param cond_bb
 *  PC_Block which will hold the condition of this if stmt
 *
 * \return
 *  exit PC_Block of this switch stmt
 *
 * Creates exit PC_Block. PC_Blocks for individual cases are created during 
 * processing of the switch body via PC_Stmt (). Updates switch cond and exit 
 * PC_Blocks in global switch context, saving the older ones for restoration just
 * before func exit. These global switch context fields are used to connect case
 * PC_Blocks as and when they are created during the processing of the switch 
 * body outside of this func. Also, does profile maintenance for the switch stmt.
 *
 * \sa PC_SetLabBB (), PC_SetSwitchCountPtrs ()
 */
static PC_Block
PC_Stmt_Switch (PC_Graph cfg, Stmt stmt, PC_Block cond_bb)
{
  PC_Block exit_bb, body_ft_bb;
  _PC_SwitchContext curr_context;
  PC_SwitchContext orig_context;

  cond_bb->cont_type = CNT_SWITCH;
  cond_bb->cond = stmt->stmtstruct.switchstmt->expression;

  PC_AddExprToBBHash(cond_bb, cond_bb->cond);

  /* If annotating, record this switch stmt for insertion
   * of count ptrs after critical edge splitting.
   */

  if (PC_opt_annotate)
    PC_sw_list = List_insert_last (PC_sw_list, (void *) cond_bb);

  /* exit_bb weight will be increased as incoming arcs are added. */

  exit_bb = PC_NewBlock (cfg, 0.0);

#if DEBUG
  fprintf (fbug, "cond_bb = %d, exit_bb = %d.\n", cond_bb->ID, exit_bb->ID);
#endif

  /* Build a stack of contexts in PC_Stmt* activation records so that
   * case labels, breaks, and profile information can be associated
   * with the nearest enclosing switch statement.
   */

  PC_InitSWContext (&curr_context, cond_bb, exit_bb, stmt, stmt->profile);
  orig_context = PC_sw_ctx;	/* Save enclosing switch context */
  PC_sw_ctx = &curr_context;

  body_ft_bb = PC_Stmt (cfg, stmt->stmtstruct.switchstmt->switchbody, NULL);

  if (body_ft_bb)
    {
      body_ft_bb->cont_type = CNT_GOTO;
      PC_Connect (body_ft_bb, exit_bb, TrueExpr (), body_ft_bb->weight);
      exit_bb->weight += body_ft_bb->weight;
    }

  /* If no explicit default case has been processed, create an
     implicit one. */

  if (!PC_sw_ctx->def_present)
    {
      double wt;
      if (PC_sw_ctx->p_ptr)
	{
	  wt = PC_sw_ctx->p_ptr->count;
	  PC_sw_ctx->p_ptr = PC_sw_ctx->p_ptr->next;
	}
      else
	{
	  wt = 0.0;
	}
      PC_Connect (cond_bb, exit_bb, DefaultExpr (), wt);
      exit_bb->weight += wt;
    }

  /* Ensure default case is last */

  {
    PC_Flow fl;

    for (fl = cond_bb->s_flow; fl; fl = fl->s_next_flow)
      {
	if (fl->flow_cond && IsDefaultExpr (fl->flow_cond) && fl->s_next_flow)
	  {
	    /* Default flow found, not at end of list */

	    PC_FlowUnlink_Succ (fl);
	    AppendPC_Flow_Succ (fl, cond_bb);
	    break;
	  }
      }
  }

  /* "Pop" the context stack */

  PC_sw_ctx = orig_context;
  return exit_bb;
}


/*! \brief Processes a statement for PC_Graph construction
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt
 *  Statement to be processed
 * \param bb
 *  PC_Block to which this statement's sub-PC_Graph should be appended
 * \return
 *  Fall-through PC_Block, if one exists
 *
 * Processes a statement for PC_Graph construction, building nodes
 * and edges as required.
 *
 * \sa PC_Stmt_Asm(), PC_Stmt_Break(), PC_Stmt_Compound(), PC_Stmt_Expr(),
 *     PC_Stmt_Goto(), PC_Stmt_If(), PC_Stmt_Noop(), PC_Stmt_Parloop(),
 *     PC_Stmt_Return(), PC_Stmt_Serloop(), PC_Stmt_Switch()
 */
static PC_Block
PC_Stmt (PC_Graph cfg, Stmt stmt, PC_Block bb)
{
  /* Start a new PC_Block (and set its weight) if this Stmt is labeled. */

  if (bb && bb->cont_type != -1)
    P_punt ("PC_Stmt: extending a closed BB");

  PC_SetLabBB (&bb, stmt);

  /* Create a new block if one is not open */

  if (!bb)
    bb = PC_NewBlock (cfg, stmt->profile ? stmt->profile->count : 0.0);


  /* If annotating Pcode, set up profile field link to this statement's
   * primary profile field.  If the statement has ancillary profile fields
   * (such as the else-field of an ifstmt), these are linked to the
   * appropriate PC_Blocks inside the PC_Stmt_* routines.
   */

  if (PC_opt_annotate)
    PC_SetProfPtr (bb, &(stmt->profile), PC_PRF_ST);

  /* Inelegant way of assimilating more profile data */
  if (stmt->profile && stmt->profile->count > bb->weight)
    bb->weight = stmt->profile->count;

  switch (stmt->type)
    {
    case ST_ASM:
      bb = PC_Stmt_Asm (cfg, stmt, bb);
      break;
    case ST_BREAK:
      bb = PC_Stmt_Break (cfg, stmt, bb);
      break;
    case ST_CONT:
      P_punt ("PC_Stmt: PC_Graph library does not support continue stmts.");
      break;
    case ST_COMPOUND:
      bb = PC_Stmt_Compound (cfg, stmt, bb);
      break;
    case ST_EXPR:
      bb = PC_Stmt_Expr (cfg, stmt, bb);
      break;
    case ST_GOTO:
      bb = PC_Stmt_Goto (cfg, stmt, bb);
      break;
    case ST_IF:
      bb = PC_Stmt_If (cfg, stmt, bb);
      break;
    case ST_NOOP:
      bb = PC_Stmt_Noop (cfg, stmt, bb);
      break;
    case ST_PARLOOP:
      P_punt ("PC_Stmt: Sync arcs aren't supported yet, run with -nosync.");
      bb = PC_Stmt_Parloop (cfg, stmt, bb);
      break;
    case ST_RETURN:
      bb = PC_Stmt_Return (cfg, stmt, bb);
      break;
    case ST_SERLOOP:
      if (!PC_opt_lp_norm)
	bb = PC_Stmt_Serloop (cfg, stmt, bb);
      else
	bb = PC_Stmt_Serloop_Norm (cfg, stmt, bb);
      break;
    case ST_SWITCH:
      bb = PC_Stmt_Switch (cfg, stmt, bb);
      break;
    default:
      P_punt ("PC_Stmt: Unknown stmt type %d", stmt->type);
      break;
    }

  return bb;
}


/*! \brief Processes a list of statements for PC_Graph construction
 *
 * \param cfg
 *  control flow graph (PC_Graph)
 * \param stmt_list
 *  Statement list to be processed
 * \param bb
 *  PC_Block to which this statement's sub-PC_Graph should be appended
 * \param parloop_flag
 *  set when \a stmt_list's parent structure is a parloop
 * \return
 *  Fall-through PC_Block, if one exists
 *
 * Processes a list of statements for PC_Graph construction, ignoring
 * certain ones, as necessary.
 *
 */
static PC_Block
PC_StmtList (PC_Graph cfg, Stmt stmt_list, PC_Block bb, int parloop_flag)
{
  Stmt stmt;

  for (stmt = stmt_list; stmt; stmt = stmt->lex_next)
    {
      if (stmt->artificial || (stmt->foroverlap && stmt->type != ST_COMPOUND))
	continue;

      if (parloop_flag &&
	  (stmt->type == ST_BODY || stmt->type == ST_EPILOGUE))
	break;

      bb = PC_Stmt (cfg, stmt, bb);
    }

  return bb;
}


/*! \brief Sets the prof ptr fields of case and default BBs of a switch stmt.
 *
 * \return void
 *
 */
static void
PC_SetSwitchCountPtrs (void)
{
  PC_Block sw_bb;
  PC_Flow sw_fl;
  ProfST *pp;
  Stmt sw_stmt;

  List_start (PC_sw_list);
  while ((sw_bb = (PC_Block) List_next (PC_sw_list)))
    {
      if (sw_bb->cont_type != CNT_SWITCH)
	P_punt ("PC_SetSwitchCountPtrs: switch list contains non-switch bb");

      if (!(sw_stmt = sw_bb->cond->parentstmt))
	P_punt ("PC_SetSwitchCountPtrs: can't find switch for switch bb");

      /* Insert annotation pointers into target cb's */

      for (sw_fl = sw_bb->s_flow, pp = &(sw_stmt->profile->next); sw_fl;
	   sw_fl = sw_fl->s_next_flow, pp = &((*pp)->next))
	PC_SetProfPtr (sw_fl->dest_bb, pp, PC_PRF_ST);
    }
  return;
}


/*! \brief This is the entry point into the Plib_CF_new library.
 *
 * \param func
 * the function for which the PC_Graph needs to be built.
 * \param init_bb_id
 * the ID of the first BB in the PC_Graph to be built.
 * \param options
 * options to use during PC_Graph construction.
 * \return control flow graph for the function represented by the func param.
 *
 */
PC_Graph
PC_Function (FuncDcl func, int init_bb_id, int options)
{
  PC_Block entry_bb = NULL, exit_bb = NULL, bb = NULL;
  double wt_f = 0.0;

  /* Parse options */

  PC_opt_split_crit = ((options & PC_SPLIT_CRIT) != 0);
  PC_opt_annotate = ((options & PC_ANNOTATE) != 0);
  PC_opt_lp_norm = ((options & PC_NORM_LOOPS) != 0);

  if (PC_cfg)
    PC_FreeGraph (PC_cfg);

  PC_cfg = PC_NewGraph (func);

  PC_sw_ctx = NULL;
  PC_sw_list = NULL;

  PC_Init ();
  PC_cfg->bb_id_offset = init_bb_id;

  /* Read back weight for func entry block. */
  if (func->stmt->profile)
    wt_f = func->stmt->profile->count;

  entry_bb = PC_NewBlock (PC_cfg, wt_f);

  exit_bb = PC_NewBlock (PC_cfg, 0.0);	/* wt is always 0.0 in profile.dat */
  exit_bb->cont_type = CNT_EXIT;
  PC_cfg->exit_bb = exit_bb;

  /* we don't ever read this back, and for now we will do without this. */
#if 0
  /* create profile fields and save ptrs to them for use during annotation. */
  if (PC_opt_annotate)
    PC_SetProfPtr (entry_bb, (ProfST *) & (func->profile), PC_PRF_FN);
#endif

  if ((bb = PC_StmtList (PC_cfg, func->stmt, entry_bb, FALSE)))
    {
      /* If body falls through, connect to exit */

      /* 12/14/04 REK Changing type from CNT_GOTO to CNT_RETURN.  CNT_GOTO
       *          will result in a jump to exit_bb, while CNT_RETURN
       *          is connected to the epilogue cb. */
      bb->cont_type = CNT_RETURN;
      PC_Connect (bb, exit_bb, TrueExpr (), bb->weight);
    }

  if (PC_opt_split_crit)
    {
      /* Split critical edges */
      for (bb = PC_cfg->first_bb; bb; bb = bb->next)
	PC_SplitCritOutEdges (PC_cfg, bb);
    }

  /* After critical edge splitting, if annotating, set
   * switch counter pointers
   */

  if (PC_opt_annotate)
    PC_SetSwitchCountPtrs ();

  /* Find all natural loops in the CFG. */
  PC_FindLoops (PC_cfg);


  /* Run internal consistency checks on profile information. */
  PC_VerifyProfileInfo (PC_cfg);

#if 0
  ChangeCurrColor ();
  PC_TraverseGraph (PC_cfg->first_bb, &PC_PrintBlock);
  /* Print the Split PC_Graph. */
#endif

#if DEBUG
  for (bb = PC_cfg->first_bb; bb; bb = bb->next)
    PC_PrintBlock (PC_FindBlock (PC_cfg, bb->ID));
  fprintf (fcfg, "\n\n");

  {
    PC_Loop lp = NULL;
    int i;
    for (lp = PC_cfg->lp; lp; lp = lp->next)
      PC_PrintLoop (fcfg, lp);

    for (lp = PC_cfg->lp; lp; lp = lp->next)
      if (lp->head < LP_HDRS_SIZE)
	LP_HDRS[lp->head]++;

    for (i = 0; i < LP_HDRS_SIZE; i++)
      if (LP_HDRS[i] > 1)
	fprintf (stdout, "Lp hdr bb #%d is shared by multiple loops.\n", i);
  }
#endif

  if (PC_opt_annotate)
    PC_sw_list = List_reset (PC_sw_list);


  total_lps += PC_cfg->num_lps;	/* Counting total lps in the benchmark/app. */
  total_bbs += PC_cfg->num_bbs;	/* Counting total bbs in the benchmark/app. */
#if DEBUG
  fprintf (fapp, "Total BBs, in %s: %d, in App: %d.\n",
	   func->name, PC_cfg->num_bbs, total_bbs);
  fprintf (fapp, "Total loops, in %s: %d, in App: %d.\n\n",
	   func->name, PC_cfg->num_lps, total_lps);
#endif


#if PC_PRINT_GRAPHS
  {
    char buf[256];
    sprintf (buf, "CFG-%s.dot", func->name);
    PC_PrintGraph (PC_cfg, buf);
  }
#endif

  PC_Terminate ();
  return PC_cfg;
}


/*! \brief Reverses a list of PC_Blocks in place.
 *
 * \param h_bb
 *  ptr to the head of a linked list of PC_Blocks.
 * \return a ptr to the head of the reversed list.
 *
 */
PC_Block
PC_RevBlockList (PC_Block h_bb)
{
  PC_Block c_bb = NULL, cN_bb = NULL, t_bb = NULL;

  if (!h_bb)
    return h_bb;
  c_bb = h_bb->next;
  t_bb = h_bb;

  while (c_bb != NULL)
    {
      cN_bb = c_bb->next;
      t_bb->next = c_bb->next;
      c_bb->next = h_bb;
      h_bb = c_bb;
      c_bb = cN_bb;
    }

  return h_bb;
}


/*! \brief Verifies the internal consistnecy of profile info on the PC_Graph.
 *
 * \param cfg
 *  control flow graph of the function being processed.
 * \return void.
 *
 */
void
PC_VerifyProfileInfo (PC_Graph cfg)
{
#if DEBUG
  PC_Block bb = NULL;
  PC_Flow fl = NULL;

  if (!PC_opt_split_crit)
    fprintf (fprf,
	     "No critical edge splitting, profile data won't match!!!\n");
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      double wt_in = 0.0, wt_out = 0.0;
      /* Compute sum of all inflow weights. */
      for (fl = bb->p_flow; fl; fl = fl->p_next_flow)
	wt_in += fl->weight;

      /* Compute sum of all outflow weights. */
      for (fl = bb->s_flow; fl; fl = fl->s_next_flow)
	wt_out += fl->weight;


      if (bb->ID != cfg->exit_bb->ID)	/* exit_bb wt is always 0.0 */
	{
	  /* Print message if inflow/outflow weights don't match bb weight. */
	  if (bb->weight != wt_in && bb->ID != cfg->first_bb->ID)
	    fprintf (stdout, "\tBB #%d wt = %f, wt_in = %f.\n",
		     bb->ID, bb->weight, wt_in);
	  if (bb->weight != wt_out && bb->ID != cfg->exit_bb->ID)
	    fprintf (stdout, "\tBB #%d wt = %f, wt_out = %f.\n",
		     bb->ID, bb->weight, wt_out);
	}

      fprintf (fprf, "\t%f\n", bb->weight);
    }
#endif
}


/*! \brief Builds the set of unreachable PC_Blocks in the control flow graph.
 *
 * \param cfg
 *  control flow graph of the function being processed.
 * \return set of unreachable PC_Blocks in cfg.
 *
 */
Set
PC_BuildUnReachableBBSet (PC_Graph cfg)
{
  Set uBBs = NULL;
  PC_Block bb;
  PC_Flow fl;

  for (bb = cfg->first_bb->next; bb; bb = bb->next)
    {
      bool unReach = TRUE;

      /* If bb has at least one reachable pred bb, then it is reachable. */
      for (fl = bb->p_flow; fl; fl = fl->p_next_flow)
	if (!Set_in (uBBs, fl->src_bb->ID))
	  unReach = FALSE;

      if (unReach)
	uBBs = Set_add (uBBs, bb->ID);
    }

  return uBBs;
}


/*! \brief Identifies a labelled stmt and creates a new PC_Block to hold it.
 *
 * \param curr_bb
 *  ptr to the PC_Block which holds the stmt processed immediately before this one.
 *
 * \param stmt
 *  the pcode stmt being processed as a potentially labelled stmt.
 *
 * If a label is found, the GotoTable is searched for a corresponding goto(s). 
 * Also, the label is added to the GotoTable. A ptr to the pragma field of stmt 
 * is saved for use duing loop profile annotation.
 *
 * \return TRUE if stmt had a label, FALSE otherwise.
 *
 */
bool
PC_SetLabBB (PC_Block * curr_bb, Stmt stmt)
{
  PC_Block bb = *curr_bb;
  Label label = stmt->labels;
  Label tgt_lab;

  if (!label)
    return FALSE;

  if (!bb || bb->cont_type != -1 || bb->first_ps || bb->p_flow)
    {
      *curr_bb =
	PC_NewBlock (PC_cfg, stmt->profile ? stmt->profile->count : 0.0);

      if (bb)
	{
	  if (bb->cont_type == -1)
	    bb->cont_type = CNT_GOTO;

	  PC_Connect (bb, *curr_bb, TrueExpr (), bb->weight);
	}
    }

  do
    {
      double wt;
#if DEBUG
      fprintf (fbug, "Label value is: %s.\n", label->val);
#endif

      switch (label->type)
	{
	case LB_CASE:
	  if (!PC_sw_ctx)
	    P_punt ("PC_sw_ctx is NULL while processing sw body.");

	  if (PC_sw_ctx->p_ptr)
	    {
	      wt = PC_sw_ctx->p_ptr->count;
	      PC_sw_ctx->p_ptr = PC_sw_ctx->p_ptr->next;
	    }
	  else
	    {
	      wt = 0.0;
	    }

	  PC_Connect (PC_sw_ctx->cond_bb, *curr_bb,
		      PSI_ReduceExpr (label->data.expression),
		      wt);
	  break;

	case LB_DEFAULT:
	  if (!PC_sw_ctx)
	    P_punt ("PC_sw_ctx is NULL while processing sw body.");

	  if (PC_sw_ctx->p_ptr)
	    {
	      wt = PC_sw_ctx->p_ptr->count;
	      PC_sw_ctx->p_ptr = PC_sw_ctx->p_ptr->next;
	    }
	  else
	    {
	      wt = 0.0;
	    }

	  PC_Connect (PC_sw_ctx->cond_bb, *curr_bb, DefaultExpr (), wt);
	  PC_sw_ctx->def_present = TRUE;
	  break;

	case LB_LABEL:
#if DEBUG
	  fprintf (fbug, "\t\tTarget label value is: %s.\n", label->val);
#endif

	  tgt_lab = P_NewLabel ();
	  tgt_lab->val = strdup (label->val);

	  PC_ConnToGotos (*curr_bb, tgt_lab);
	  PC_AddTarget (*curr_bb, tgt_lab);

	  DISPOSE (tgt_lab->val);
	  tgt_lab->val = NULL;
	  P_FreeLabel (tgt_lab);

	  /* create a PC_LpPrag and copy stmt pragma info into it */
	  if (PC_opt_annotate)
	    (*curr_bb)->lp_prg =
	      PC_NewLpPrag ((*curr_bb), stmt, &(stmt->pragma));

	  /*save ptrs to pcode lp pragma for transfering to lcode during PtoL. */
	  if (stmt->pragma)
	    (*curr_bb)->pragma = stmt->pragma;
	  break;
	default:
	  P_punt ("Illegal label type [%d] in PC_SetLabBB().\n", label->type);
	}
    }
  while ((label = label->next));

  return TRUE;
}


/*! \brief Splits critical succ flows and PC_Blocks (needed for profiling).
 *
 * \param cfg
 *  control flow graph of the function currently being processed.
 * \param bb
 *  PC_Block to examine for critical succ flows.
 * \return void.
 *
 */
static void
PC_SplitCritOutEdges (PC_Graph cfg, PC_Block bb)
{
  PC_Flow fl;

  /* Consider only BB's with multiple successors. */

  if (!bb->s_flow || !bb->s_flow->s_next_flow)
    return;

  /*If BB has multiple successors, determine if any one of BB's successors
     has multiple predecessors. */

  for (fl = bb->s_flow; fl; fl = fl->s_next_flow)
    {
      if (fl->p_prev_flow || fl->p_next_flow)
	{
	  /* Found a critical edge */
	  PC_Block new_bb, succ_bb;

	  succ_bb = fl->dest_bb;

	  new_bb = PC_NewBlock (cfg, fl->weight);
	  new_bb->cont_type = CNT_GOTO;
	  PC_ChangeFlowDest (fl, new_bb);
	  PC_Connect (new_bb, succ_bb, TrueExpr (), fl->weight);
#if DEBUG
	  fprintf (fbug, "Split link between BB #%d & BG# %d.\n",
		   bb->ID, succ_bb->ID);
#endif
	}
    }

  return;
}


/* Allocation / deallocation functions
 * ----------------------------------------------------------------------
 */

PC_Graph
PC_NewGraph (FuncDcl func)
{
  PC_Graph cfg = ALLOCATE (_PC_Graph);
  cfg->func = func;
  cfg->first_bb = NULL;
  cfg->last_bb = NULL;
  cfg->num_bbs = 0;
  cfg->num_lps = 0;
  cfg->bb_id_offset = 0;
  cfg->lp = NULL;
  cfg->lp_tree = NULL;
  cfg->hash_bb_id = HashTable_create (PC_BB_HASH_SIZE);
  cfg->hash_expr_bb = HashTable_create (PC_EXPR_BB_HASH_SIZE);

  return cfg;
}


PC_Graph
PC_FreeGraph (PC_Graph cfg)
{
  PC_Block bb, bbn;
  PC_Loop lp, lpn;

  if (!cfg)
    return NULL;

  for (bb = cfg->first_bb; bb; bb = bbn)
    {
      bbn = bb->next;
      PC_FreeBlock (bb);
    }

  for (lp = cfg->lp; lp; lp = lpn)
    {
      lpn = lp->next;
      PC_FreeLoop (lp);
    }

  PC_FreeGotoTable (GotoTable);

  HashTable_free (cfg->hash_bb_id);
  cfg->hash_bb_id = NULL;
  HashTable_free (cfg->hash_expr_bb);
  cfg->hash_expr_bb = NULL;


  if (cfg == PC_cfg)
    PC_cfg = NULL;

  DISPOSE (cfg);

  return NULL;
}


PC_GotoEntry *
PC_FreeGotoTable (PC_GotoEntry * GotoTable)
{
  PC_GotoEntry ge, gen;
  int i;

  if (!GotoTable)
    return NULL;

  for (i = 0; i < GOTO_TABLE_SIZE; i++)
    for (ge = GotoTable[i]; ge; ge = gen)
      {
	gen = ge->next;
	PC_FreeGotoEntry (ge);
      }

  DISPOSE (GotoTable);
  return NULL;
}


PC_Block
PC_NewBlock (PC_Graph cfg, double wt)
{
  PC_Block bb = ALLOCATE (_PC_Block);
  int PREV_COLOR = (CURR_COLOR + NUM_COLORS - 1) % NUM_COLORS;

#ifdef UNIQUE_BB_IDs
  bb->ID = PC_cfg->num_bbs + PC_cfg->bb_id_offset + total_bbs;
#else
  bb->ID = PC_cfg->num_bbs + PC_cfg->bb_id_offset;
#endif
  PC_cfg->num_bbs++;

  bb->cond = NULL;
  bb->cont_type = -1;
  bb->s_flow = NULL;
  bb->p_flow = NULL;
  bb->first_ps = NULL;
  bb->last_ps = NULL;

#if 1
  bb->color = PREV_COLOR;
#else
  bb->color = CURR_COLOR;
#endif

  bb->f_opts = 0;

  bb->doms = NULL;
  bb->idom = NULL;
  bb->dom_tree_children = NULL;
  bb->dom_frontier = NULL;

  bb->loop = NULL;

  bb->ext = NULL;
  bb->pragma = NULL;
  bb->counters = NULL;
  bb->lp_prg = NULL;
  bb->weight = wt;

  bb->next = NULL;
  bb->prev = cfg->last_bb;
  if (cfg->last_bb)
    cfg->last_bb->next = bb;
  cfg->last_bb = bb;
  if (!cfg->first_bb)
    cfg->first_bb = bb;

  HashTable_insert (cfg->hash_bb_id, bb->ID, (void *) bb);

  return bb;
}


PC_Block
PC_FreeBlock (PC_Block bb)
{
  PC_Flow fl, fln;
  PC_PStmt ps, psn;

  if (!bb)
    return NULL;

  bb->counters = List_reset (bb->counters);

  for (ps = bb->first_ps; ps; ps = psn)
    {
      psn = ps->succ;
      PC_FreePStmt (ps);
    }

  /* Every flow is pointed to by a s_flow ptr, so no need to free via p_flow. */
  for (fl = bb->s_flow; fl; fl = fln)
    {
      fln = fl->s_next_flow;
      PC_FreeFlow (fl);
    }

  Set_dispose (bb->doms);
  if (bb->lp_prg)
    PC_FreeLpPrag (bb->lp_prg);

  /* bb->pragma & bb->ext may point to structs needed after PC_Graph is freed. */
  /* bb->cond points to pcode structs to be freed later. */

  DISPOSE (bb);
  return NULL;
}


PC_Flow
PC_NewFlow (PC_Block src_bb, PC_Block dest_bb, Expr cond, double weight)
{
  PC_Flow fl = ALLOCATE (_PC_Flow);

  fl->src_bb = src_bb;
  fl->dest_bb = dest_bb;
  fl->flow_cond = cond;
  fl->weight = weight;
  fl->p_next_flow = NULL;
  fl->p_prev_flow = NULL;
  fl->s_next_flow = NULL;
  fl->s_prev_flow = NULL;
  fl->flags = 0;
  return fl;
}


PC_Flow
PC_FreeFlow (PC_Flow fl)
{
  if (!fl)
    return NULL;

  DISPOSE (fl->flow_cond);
  DISPOSE (fl);
  return NULL;
}



static PC_PStmt
PC_AllocPStmt (_PC_PSType ps_type, Stmt st)
{
  PC_PStmt ps;

  ps = ALLOCATE (_PC_PStmt);
  ps->type = ps_type;
  ps->f_opts = 0;
  switch (ps->type)
    {
    case PC_T_NatPcode:
    case PC_T_Expr:
      ps->data.stmt = st;
      break;
    case PC_T_Probe:
      ps->data.probe = NULL;
      break;
    default:
      P_punt ("PC_NewPStmt: Invalid type: %d", ps->type);
      break;
    }

  ps->pred = NULL;
  ps->succ = NULL;

  return ps;
}

/* IMS 20041116 */
/* \brief add an expression and all its {next,sibling,operands} to the
 * expr -> BB hash
 *
 * \param bb
 *   basic block to link the expression IDs to
 * \param expr
 *   expr to add, and recurse off of
 */
static void
PC_AddExprToBBHash(PC_Block bb, Expr expr)
{
  PC_Block bb2;

  /* sometimes if we do these optimizations, expressions tend to
   * get "replicated" and stuck in multiple BBs which makes our
   * hash an unhappy camper. */
  if (PC_opt_split_crit || PC_opt_annotate || PC_opt_lp_norm)
    return;

  /* recursion base case */
  if (!expr) 
    return;
  if (expr->id == 0)
    {
      P_warn("PC_AddExprToBBHash: Cannot add expression w/ id == 0 to hash"
	     " table\n");
      return;
    }
  
  if (HashTable_member(PC_cfg->hash_expr_bb, expr->id))
    {
      if ((bb2 = HashTable_find(PC_cfg->hash_expr_bb, expr->id)) && 
	  (bb2 != bb))
      {
	P_punt("Expression in multiple BBs is illegal\n");
      }
      /* already in hash - no need to insert again */
    }
  else
    {
      HashTable_insert (PC_cfg->hash_expr_bb, expr->id, (void *) bb);
    }

  /* recursively add the subexpressions */
  PC_AddExprToBBHash(bb, expr->next);
  PC_AddExprToBBHash(bb, expr->sibling);
  PC_AddExprToBBHash(bb, expr->operands);
}

/* IMS 20041116 */
/* \brief add all the Expr in a st to the expr -> BB hash
 *
 * this is just a helper function to call the recursive
 * PC_UpdateExprBBHashTbl function.
 *
 * Note that the hash table will NOT be built if we called the CFG
 * construction using any of the optimizations.  If they are on, 
 * expressions can sometimes exist in multiple BBs and this breaks 
 * our notion of a hash.
 */
static void
PC_AddStmtExprsToBBHash(PC_Block bb, Stmt st)
{
  /* sometimes if we do these optimizations, expressions tend to
   * get "replicated" and stuck in multiple BBs which makes our
   * hash an unhappy camper. */
  if (PC_opt_split_crit || PC_opt_annotate || PC_opt_lp_norm)
    return;

#if 0 /* i don't think that this should be here ... */
  if (st->type == ST_EXPR || st->type == ST_RETURN)
#endif
    PC_AddExprToBBHash(bb, st->stmtstruct.expr);
}


/* IMS 20041116 */
/*! \brief get the BB for a given expr id in a given CFG
 *
 * \param CFG
 *   control flow graph (PC_Graph)
 * \param ID
 *   expression id (Expr->id) (int)
 * \returns
 *   PC_Block that the expr is in
 */
PC_Block 
PC_FindExprBB(PC_Graph g, int id)
{
  PC_Block bb;
  if (id == 0 || !HashTable_member(g->hash_expr_bb, id))
    bb = NULL;
  else
    bb = HashTable_find(g->hash_expr_bb, id);

  return bb;
}

/*! \brief Allocates a new PC_PStmt.
 *
 * \param ps_type
 *  type of the new PC_PStmt.
 * \param bb
 *  PC_Block to which the new PC_Stmt will be appended.
 * \param st
 *  pcode stmt to store in this PC_PStmt (may be NULL)
 * \return A pointer to the new PC_PStmt.
 *
 * \sa PC_NewPStmtProbe, PC_NewPStmtExpr.
 */
PC_PStmt
PC_NewPStmt (_PC_PSType ps_type, PC_Block bb, Stmt st)
{
  PC_PStmt ps = PC_AllocPStmt (ps_type, st);

  PC_AddStmtExprsToBBHash(bb, st);

  if (!bb)
    P_punt ("NULL PC_Block in NewPStmt().");

  if (bb->last_ps == NULL)
    ps->pred = NULL;
  else
    PC_ConnectPStmts (bb->last_ps, ps);

  bb->last_ps = ps;
  if (bb->first_ps == NULL)
    bb->first_ps = ps;

  return ps;
}


PC_PStmt
PC_NewPStmtBefore (_PC_PSType ps_type, PC_Block bb, PC_PStmt psb,
		   Stmt st)
{
  PC_PStmt ps = PC_AllocPStmt (ps_type, st);

  PC_AddStmtExprsToBBHash(bb, st);

  if (!bb)
    P_punt ("NULL PC_Block in NewPStmt().");

  ps->succ = psb;

  if (psb)
    {
      if (psb->pred)
	PC_ConnectPStmts (psb->pred, ps);
      else
	bb->first_ps = ps;

      PC_ConnectPStmts (ps, psb);
    }
  else
    {
      if (bb->last_ps)
	PC_ConnectPStmts (bb->last_ps, ps);
      else
	bb->first_ps = ps;

      bb->last_ps = ps;
    }

  return ps;
}


PC_PStmt
PC_FreePStmt (PC_PStmt ps)
{
  if (!ps)
    return NULL;

  if ((ps->f_opts & PC_FREE_STMT) != 0)
    DISPOSE (ps->data.stmt);

  if ((ps->f_opts & PC_FREE_PROBE) != 0)
    PC_FreeProbe (ps->data.probe);

  DISPOSE (ps);
  return NULL;
}


PC_PStmt
PC_DeletePStmt (PC_Block b, PC_PStmt ps)
{
  if (ps->pred)
    ps->pred->succ = ps->succ;
  else
    b->first_ps = ps->succ;

  if (ps->succ)
    ps->succ->pred = ps->pred;
  else
    b->last_ps = ps->pred;

  PC_FreePStmt (ps);
  return NULL;
}

PC_PStmt
PC_NewPStmtProbe (_PC_PSType ps_type, PC_Block bb, PC_Probe pb)
{
  PC_PStmt ps;

  ps = PC_NewPStmt (ps_type, bb, NULL);
  ps->f_opts |= PC_FREE_PROBE;	/* So that we free up pb later. */
  ps->data.probe = PC_NewProbe (pb->type, pb->ID);
  return ps;
}


PC_PStmt
PC_NewPStmtExpr (_PC_PSType ps_type, PC_Block bb, Expr expr)
{
  PC_PStmt ps;
  Stmt st;

  st = P_NewStmtWithType (ST_EXPR);
  st->stmtstruct.expr = expr;
  ps = PC_NewPStmt (ps_type, bb, st);
  ps->f_opts |= PC_FREE_STMT;	/* So that we free up st later. */
  return ps;
}


PC_PStmt
PC_NewPStmtExprBefore (_PC_PSType ps_type, PC_Block bb, PC_PStmt psb, 
		       Expr expr)
{
  PC_PStmt ps;
  Stmt st;

  st = P_NewStmtWithType (ST_EXPR);
  st->stmtstruct.expr = expr;
  ps = PC_NewPStmtBefore (ps_type, bb, psb, st);
  ps->f_opts |= PC_FREE_STMT;	/* So that we free up st later. */
  return ps;
}


PC_Probe
PC_NewProbe (_PC_ProbeType pb_type, int p_ID)
{
  PC_Probe pb;
  pb = ALLOCATE (_PC_Probe);
  pb->type = pb_type;
  pb->ID = p_ID;
  return pb;
}

PC_Probe
PC_FreeProbe (PC_Probe pb)
{
  if (!pb)
    return NULL;

  DISPOSE (pb);
  return NULL;
}


PC_LpPrag
PC_NewLpPrag (PC_Block bb, Stmt stmt, Pragma * prg)
{
  PC_LpPrag lp_prg = ALLOCATE (_PC_LpPrag);

  lp_prg->ptr = prg;		/* prg could be a Stmt or Expr pragma */
  lp_prg->lp_ln = stmt->lineno;

  if (stmt->type == ST_SERLOOP)
    lp_prg->lp_typ = stmt->stmtstruct.serloop->loop_type;
  else if (stmt->labels)
    lp_prg->lp_typ = PC_LT_GOTO;
  else
    P_punt ("PC_NewLpPrag: Non-loop stmt, type: %d.", stmt->type);

  return lp_prg;
}

PC_LpPrag
PC_FreeLpPrag (PC_LpPrag lp_prg)
{
  if (!lp_prg)
    return NULL;

  /* the pragma struct pointed to by ptr will be freed by pcode. */
  lp_prg->ptr = NULL;

  DISPOSE (lp_prg);
  return NULL;
}

static void
PC_InitSWContext (PC_SwitchContext swc, PC_Block cond_bb,
		  PC_Block exit_bb, Stmt st, ProfST prf)
{
  swc->cond_bb = cond_bb;
  swc->exit_bb = exit_bb;
  swc->sw_st = st;
  if (prf)
    swc->p_ptr = prf->next;
  else
    swc->p_ptr = NULL;
  swc->def_present = FALSE;
}


PC_GotoEntry
PC_NewGotoEntry (PC_Block goto_bb, PC_Block target_bb, Label goto_lab,
		 Label target_lab)
{
  PC_GotoEntry ge = ALLOCATE (_PC_GotoEntry);
  ge->goto_bb = goto_bb;
  ge->target_bb = target_bb;
  ge->goto_lab = ge->target_lab = NULL;

  if (goto_lab)
    ge->goto_lab = P_NewLabel ();
  if (target_lab)
    ge->target_lab = P_NewLabel ();

  if (goto_lab && goto_lab->val)
    ge->goto_lab->val = strdup (goto_lab->val);
  if (target_lab && target_lab->val)
    ge->target_lab->val = strdup (target_lab->val);

  ge->next = NULL;
  return ge;
}

PC_GotoEntry
PC_FreeGotoEntry (PC_GotoEntry ge)
{
  if (!ge)
    return NULL;

  if (ge->goto_lab)
    {
      DISPOSE (ge->goto_lab->val);
      ge->goto_lab->val = NULL;
      P_FreeLabel (ge->goto_lab);
    }

  if (ge->target_lab)
    {
      DISPOSE (ge->target_lab->val);
      ge->target_lab->val = NULL;
      P_FreeLabel (ge->target_lab);
    }
  DISPOSE (ge);
  return NULL;
}


/* Functions to connect/disconnect PC_PStmts, PC_Blocks, PC_Flows
 * ----------------------------------------------------------------------
 */


/*! \brief Connects two PC_PStmts within a PC_Block.
 *
 * \param p_ps
 *  the pred PC_PStmt
 * \param s_ps
 *  the succ PC_PStmt
 * \return void
 *
 */
void
PC_ConnectPStmts (PC_PStmt p_ps, PC_PStmt s_ps)
{
  assert (p_ps != NULL && s_ps != NULL);

  p_ps->succ = s_ps;
  s_ps->pred = p_ps;
}

/*! \brief Connects two PC_Blocks within the PC_Graph.
 *
 * \param p_bb
 *  pred PC_Block
 * \param s_bb
 *  succ PC_Block
 * \param cond
 *  condition expr of the PC_Flow connecting the two PC_Blocks
 * \param weight
 *  weight of the PC_Flow connecting the two PC_Blocks
 * \return 
 *  PC_Flow connecting the two PC_Blocks
 *
 * Note: Only one PC_Flow is created per p_bb <-> s_bb pair, and both have ptrs 
 * to this one PC_Flow.
 */
PC_Flow
PC_Connect (PC_Block p_bb, PC_Block s_bb, Expr cond, double weight)
{
  PC_Flow pc_flow = NULL;

  if (!s_bb || !p_bb)
    P_punt ("%s is NULL in PC_Connect().", s_bb ? "Predecessor" :
	    "Successor");

  if (p_bb->cont_type == -1)
    P_punt ("PC_Connect(): Connecting from a BB of undefined type");

#if DEBUG
  fprintf (fbug, "Connecting #%d & #%d.\n", p_bb->ID, s_bb->ID);
#endif

  /* Create the pc_flow that will connect p_bb and s_bb. */
  pc_flow = PC_NewFlow (p_bb, s_bb, cond, weight);

  /* Then append this same pc_flow to both the p_bb and s_bb lists. */
  AppendPC_Flow_Succ (pc_flow, p_bb);
  AppendPC_Flow_Pred (pc_flow, s_bb);

  return pc_flow;
}


/*! \brief Changes a PC_Flow from, pred <-> succ to, predNew <-> succ.
 *
 * \param pc_flow
 *  PC_Flow linking pred and succ PC_Blocks
 * \param new_pred
 *  new pred PC_Block that will replace old pred in pc_flow
 * \return void
 *
 */
void
PC_ChangeFlowSrc (PC_Flow pc_flow, PC_Block new_pred)
{
  PC_Block old_pred = pc_flow->src_bb;

  /* Unlink in old predecessor */

  if (pc_flow->s_prev_flow)
    pc_flow->s_prev_flow->s_next_flow = pc_flow->s_next_flow;
  if (pc_flow->s_next_flow)
    pc_flow->s_next_flow->s_prev_flow = pc_flow->s_prev_flow;
  if (!pc_flow->s_prev_flow)
    old_pred->s_flow = pc_flow->s_next_flow;

  /* Link into new predecessor */

  pc_flow->src_bb = new_pred;
  pc_flow->s_next_flow = NULL;
  pc_flow->s_prev_flow = NULL;
  AppendPC_Flow_Succ (pc_flow, new_pred);

  return;
}


/*! \brief Changes a PC_Flow from, pred <-> succ to, pred <-> succNew.
 *
 * \param pc_flow
 *  PC_Flow linking pred and succ PC_Blocks
 * \param new_succ
 *  new succ PC_Block that will replace old succ in pc_flow
 * \return void
 *
 */
void
PC_ChangeFlowDest (PC_Flow pc_flow, PC_Block new_succ)
{
  PC_Block old_succ = pc_flow->dest_bb;

  /* Unlink in old successor */

  if (pc_flow->p_prev_flow)
    pc_flow->p_prev_flow->p_next_flow = pc_flow->p_next_flow;
  if (pc_flow->p_next_flow)
    pc_flow->p_next_flow->p_prev_flow = pc_flow->p_prev_flow;
  if (!pc_flow->p_prev_flow)
    old_succ->p_flow = pc_flow->p_next_flow;

  /* Link into new successor */

  pc_flow->dest_bb = new_succ;
  pc_flow->p_next_flow = NULL;
  pc_flow->p_prev_flow = NULL;
  AppendPC_Flow_Pred (pc_flow, new_succ);

  return;
}


/*! \brief Removes a PC_Flow connecting two PC_Blocks.
 *
 * \param pc_flow
 *  PC_Flow to be removed
 * \return void
 *
 */
void
PC_RemoveFlow (PC_Flow pc_flow)
{
  PC_Block pred = pc_flow->src_bb, succ = pc_flow->dest_bb;

  if (!pc_flow)
    P_punt ("pc_flow is NULL in PC_RemoveFlow.");

  /* Unlink in predecessor */

  if (pc_flow->s_prev_flow)
    pc_flow->s_prev_flow->s_next_flow = pc_flow->s_next_flow;
  if (pc_flow->s_next_flow)
    pc_flow->s_next_flow->s_prev_flow = pc_flow->s_prev_flow;
  if (!pc_flow->s_prev_flow)
    pred->s_flow = pc_flow->s_next_flow;

  /* Unlink in successor */

  if (pc_flow->p_prev_flow != NULL)
    pc_flow->p_prev_flow->p_next_flow = pc_flow->p_next_flow;
  if (pc_flow->p_next_flow != NULL)
    pc_flow->p_next_flow->p_prev_flow = pc_flow->p_prev_flow;
  if (!pc_flow->p_prev_flow)
    succ->p_flow = pc_flow->p_next_flow;
}


/*! \brief Appends a PC_Flow to the succ list of a PC_Block.
 *
 * \param pc_flow
 *  PC_Flow to be appended to the succ list
 * \param p_bb
 *  PC_Block to whose succ list pc_flow is to be appended
 * \return void
 *
 */
void
AppendPC_Flow_Succ (PC_Flow pc_flow, PC_Block p_bb)
{
  PC_Flow flow = p_bb->s_flow;

  if (p_bb->s_flow == NULL)
    {
      p_bb->s_flow = pc_flow;
      return;
    }

  /* Find the right place to insert pc_flow. */
  while (flow != NULL && flow->s_next_flow != NULL)
    flow = flow->s_next_flow;

  /* Append pc_flow to the end of the current succ flow list. */
  pc_flow->s_prev_flow = flow;
  flow->s_next_flow = pc_flow;
}


/*! \brief Clears the succ field of a PC_Flow.
 *
 * \param fl
 *  PC_Flow whose succ field is to be cleared
 * \return void
 *
 */
void
PC_FlowUnlink_Succ (PC_Flow fl)
{
  PC_Block bb = fl->src_bb;

  if (fl->s_prev_flow)
    fl->s_prev_flow->s_next_flow = fl->s_next_flow;
  else if (fl == bb->s_flow)
    bb->s_flow = fl->s_next_flow;
  else
    P_punt ("PC_FlowUnlink_Succ: List error");

  if (fl->s_next_flow)
    fl->s_next_flow->s_prev_flow = fl->s_prev_flow;

  fl->s_prev_flow = NULL;
  fl->s_next_flow = NULL;
  return;
}


/*! \brief Appends a PC_Flow to the pred list of a PC_Block.
 *
 * \param pc_flow
 *  PC_Flow to be appended to the pred list
 * \param s_bb
 *  PC_Block to whose pred list pc_flow is to be appended
 * \return void
 *
 */
void
AppendPC_Flow_Pred (PC_Flow pc_flow, PC_Block s_bb)
{
  PC_Flow flow = s_bb->p_flow;

  if (s_bb->p_flow == NULL)
    {
      s_bb->p_flow = pc_flow;
      return;
    }

  /* Find the right place to insert pc_flow. */
  while (flow != NULL && flow->p_next_flow != NULL)
    flow = flow->p_next_flow;

  /* Append pc_flow to the end of the current pred flow list. */
  pc_flow->p_prev_flow = flow;
  flow->p_next_flow = pc_flow;
}


/*! \brief Returns the next sequential loop ID.
 *
 * \return 
 *  next sequential loop ID
 *
 *  Loop IDs are globally unique across all funcs of an application.
 *
 */
int
PC_NextLoopID ()
{
  int l_lps = PC_cfg->num_lps++;
  return (total_lps + l_lps);
}



/* Functions to maintain GotoTable and to connect gotos and their target labels.
 * ----------------------------------------------------------------------------
 */


/*! \brief Connects a given goto to its target, if target is in GotoTable.
 *
 * \param bb
 *  PC_Block containing the goto stmt
 * \param goto_lab
 *  label of the goto stmt
 * \return
 *  TRUE if goto target found, FALSE otherwise
 *
 */
bool
PC_ConnToTarget (PC_Block bb, Label goto_lab)
{
  PC_GotoEntry ge = NULL;
  int index = 0, labLen, i;

  labLen = strlen (goto_lab->val);
  for (i = 0; i < labLen; i++)
    index += (int) goto_lab->val[i];
  index = index % GOTO_TABLE_SIZE;

#if DEBUG
  fprintf (fbug, "\t\tLooking for target label %s.\n", goto_lab->val);
#endif

  ge = GotoTable[index];

  while (ge)
    {
      if (ge->target_lab && !strcmp (ge->target_lab->val, goto_lab->val))
	{
	  PC_Connect (bb, ge->target_bb, TrueExpr (), bb->weight);
	  return TRUE;
	}
      ge = ge->next;
    }

  return FALSE;
}


/*! \brief Adds a goto to the GotoTable.
 *
 * \param bb
 * PC_Block containing the goto stmt
 * \param goto_lab
 * label of the goto stmt
 * \return void
 *
 */
void
PC_AddGoto (PC_Block bb, Label goto_lab)
{
  PC_GotoEntry *ge = NULL;
  int index = 0, labLen, i;

  labLen = strlen (goto_lab->val);
  for (i = 0; i < labLen; i++)
    index += (int) goto_lab->val[i];
  index = index % GOTO_TABLE_SIZE;

  ge = &GotoTable[index];
  while ((*ge) != NULL)		//entry is currently NOT empty
    {
#if DEBUG
      fprintf (fbug, "\t\tThis place at index %d already occupied.\n", index);
#endif
      ge = &(*ge)->next;
    }

  if (*ge == NULL)		//entry is currently empty
    {
      *ge = PC_NewGotoEntry (bb, NULL, goto_lab, NULL);
#if DEBUG
      fprintf (fbug, "\t\tGoto Label %s, added to index: %d.\n",
	       (*ge)->goto_lab->val, index);
#endif
    }
}


/*! \brief Connects a given label to its corresponding goto(s), if goto(s) are 
 *   in GotoTable.
 *
 * \param bb
 *  PC_Block containing the target label
 * \param target_lab
 *  target label
 * \return
 *  TRUE if any goto(s) found, FALSE otherwise
 *
 */
bool
PC_ConnToGotos (PC_Block bb, Label target_lab)
{
  bool GE_FOUND = FALSE;
  PC_GotoEntry ge = NULL;
  int index = 0, labLen, i;

  labLen = strlen (target_lab->val);
  for (i = 0; i < labLen; i++)
    index += (int) target_lab->val[i];
  index = index % GOTO_TABLE_SIZE;

#if DEBUG
  fprintf (fout, "\t\tLooking for Label %s, at index: %d.\n",
	   target_lab->val, index);
#endif

  ge = GotoTable[index];
  while (ge != NULL)
    {
      if (ge->goto_lab && !strcmp (ge->goto_lab->val, target_lab->val))
	{
	  PC_Connect (ge->goto_bb, bb, TrueExpr (), ge->goto_bb->weight);
	  GE_FOUND = TRUE;
	}
      ge = ge->next;
    }

  return GE_FOUND;
}


/*! \brief Adds a target label to the GotoTable.
 *
 * \param bb
 * PC_Block containing the target label
 * \param target_lab
 * target label
 * \return void
 *
 */
void
PC_AddTarget (PC_Block bb, Label target_lab)
{
  PC_GotoEntry *ge = NULL;
  int index = 0, labLen, i;

  labLen = strlen (target_lab->val);
  for (i = 0; i < labLen; i++)
    index += (int) target_lab->val[i];
  index = index % GOTO_TABLE_SIZE;

  ge = &GotoTable[index];
  while ((*ge) != NULL)		//entry is currently NOT empty
    {
#if DEBUG
      fprintf (fout, "\t\tThis place at index %d already occupied.\n", index);
#endif
      ge = &(*ge)->next;
    }

  if (*ge == NULL)		//entry is currently empty
    {
      *ge = PC_NewGotoEntry (NULL, bb, NULL, target_lab);
#if DEBUG
      fprintf (fout, "\t\tTarget Label %s, added at index: %d.\n",
	       (*ge)->target_lab->val, index);
#endif
    }
}


/*! \brief Returns a PC_Block for the given block ID.
 *
 * \param g
 *  the control flow graph
 * \param id
 *  block ID of relevant PC_Block
 * \return
 *  PC_Block corresponding to the block ID of 'id'
 *
 */
PC_Block
PC_FindBlock (PC_Graph g, int id)
{
  PC_Block b = HashTable_find (g->hash_bb_id, id);

  return b;
}


/*! \brief Sets the profile ptr of a PC_Block to the weight field of its 
 *   corresponding profile struct in pcode
 *
 * \param bb
 *  PC_Block whose profile ptr is to be set
 * \param prof
 *  ptr to the profile struct corresponding to bb
 * \param prf_typ
 *  type of profile struct (ProfST, ProfFN, etc.)
 * \return void
 *
 */
void
PC_SetProfPtr (PC_Block bb, ProfST * prof, int prf_typ)
{
  switch (prf_typ)
    {
    case PC_PRF_ST:
      if (!*prof)
	*prof = P_NewProfST ();
      break;
    case PC_PRF_FN:
      if (!*prof)
	*prof = (ProfST) P_NewProfFN ();
      break;
    default:
      P_punt ("PC_SetProfPtr: Invalid prf_typ.");
      break;
    }

  bb->counters = List_insert_last (bb->counters, (void *) &((*prof)->count));
  return;
}


/*! \brief Traverses the PC_Graph in depth-first order, applying any given func 
 *   to each PC_Block.
 *
 * \param bb
 *  starting PC_Block for control flow graph traversal
 * \param fPtr
 *  func ptr for the func to be applied to each PC_Block
 * \return void
 *
 */
void
PC_TraverseGraph (PC_Block bb, void (*fPtr) (PC_Block))
{
  PC_Flow pc_flow = NULL;

  if (bb->color == CURR_COLOR)	/* Already processed. */
    return;
  else
    bb->color = CURR_COLOR;


  fPtr (bb);			/* Process this block. */

  /* Process all of the successors of this block in depth first order. */
  for (pc_flow = bb->s_flow; pc_flow != NULL; pc_flow = pc_flow->s_next_flow)
    {
      TABLENGTH++;
      PC_TraverseGraph (pc_flow->dest_bb, fPtr);
      TABLENGTH--;
    }
}


/*! \brief Applies a given func to all exprs inside PStmts in the PC_Graph.
 *
 * \param fn
 *  func ptr for the func to be applied to each expr
 * \return void
 *
 */
void
PC_ApplyToExprs (void (*fn) (Expr))
{
  PC_Block bb;
  PC_PStmt ps;
  Stmt stmt;

  for (bb = PC_cfg->first_bb; bb; bb = bb->next)
    {
      for (ps = bb->first_ps; ps; ps = ps->succ)
	{
	  if (ps->type != PC_T_Probe && (stmt = ps->data.stmt))
	    {
	      switch (stmt->type)
		{
		case ST_EXPR:
		case ST_RETURN:
		  (*fn) (stmt->stmtstruct.expr);
		  break;
		case ST_NOOP:
		case ST_GOTO:
		case ST_BREAK:
		  break;
		default:
		  P_warn ("PC_ApplyToExprs: stmt type %d", stmt->type);
		  break;
		}
	    }
	}
      if (bb->cond)
	(*fn) (bb->cond);
    }
  return;
}

/*! \brief Changes the current color, which as an abstraction used to 
 *   distinguish between visited and non-visited blocks in a given PC_Graph
 *   traversal
 * 
 * \return void
 *
 * \sa PC_TraverseGraph ()
 */
void
ChangeCurrColor (void)
{
  CURR_COLOR = (CURR_COLOR + 1) % NUM_COLORS;
}



/* Init/Terminate functions dealing mostly with the opening/closing of files.
 * -----------------------------------------------------------------------------
 */
void
PC_Init ()
{
  char *fileName;
  fileName = (char *) malloc (sizeof (char) * 256);
  if (fileName == NULL)
    P_punt ("PC_Init: Out of memory, malloc returned NULL.");
  CURR_COLOR = WHITE;

#if DEBUG
  strcpy (fileName, PC_cfg->func->name);
  strcat (fileName, ".cfg");
  fout = fopen (fileName, "w");

  strcpy (fileName, PC_cfg->func->name);
  strcat (fileName, ".CFG");
  fcfg = fopen (fileName, "w");

  strcpy (fileName, PC_cfg->func->name);
  strcat (fileName, ".debug");
  fbug = fopen (fileName, "w");

  /*Open new file ony if processing 1st func of this app. */
  if (total_bbs == 0)
    {
      int i;
      fapp = fopen ("app.info", "w");
      fprf = fopen ("profile.dat.chk", "w");

      for (i = 0; i < LP_HDRS_SIZE; i++)
	LP_HDRS[i] = 0;
    }

  if (!fout || !fcfg || !fbug)
    P_punt ("PC_Init: fopen() returned NULL.");
#endif

  GotoTable =
    (PC_GotoEntry *) calloc (GOTO_TABLE_SIZE, sizeof (PC_GotoEntry));

  free (fileName);
#if DEBUG
  fprintf (fbug, "\t*********Starting New PC_Graph Routines.**********\n");
#endif
}

void
PC_Terminate ()
{
#if DEBUG
  fprintf (fbug, "\n\t*********Finishing New PC_Graph Routines.**********\n");

  fflush (fout);
  fflush (fcfg);
  fflush (fbug);

  fclose (fout);
  fclose (fcfg);
  fclose (fbug);
#endif
}


/*
 * DEBUG
 * ----------------------------------------------------------------------
 */

void
PC_PrintBlock (PC_Block bb)
{
  int i;
  PC_Flow flow = NULL;


#if !DEBUG
  return;
#endif

  if (fcfg == NULL)
    P_punt ("Can't open main.CFG!!!");

  if (bb == NULL)
    P_punt ("Can't print NULL PC_Block!");


  flow = bb->s_flow;

  fprintf (fcfg, "\n");
  for (i = 0; i < TABLENGTH; i++)
    fprintf (fcfg, "%s", TAB);
  fprintf (fcfg, "=== PC_Block #%d, wt = %f.===\n", bb->ID, bb->weight);
  if (PRINT_DOMS)
    {
      for (i = 0; i < TABLENGTH; i++)
	fprintf (fcfg, "%s", TAB);
      Set_print (fcfg, "Doms:", bb->doms);
    }


  /* This if body prints the PC_Flow info for debugging. */
  if (bb->s_flow != NULL
      && bb->s_flow->src_bb != NULL && bb->s_flow->dest_bb != NULL)
    {
      for (i = 0; i < TABLENGTH; i++)
	fprintf (fcfg, "%s", TAB);
      fprintf (fcfg, "Block #%d is linked to Block #(s) %d",
	       flow->src_bb->ID, flow->dest_bb->ID);

      flow = flow->s_next_flow;
      while (flow != NULL && flow->dest_bb != NULL)
	{
	  fprintf (fcfg, ", %d", flow->dest_bb->ID);
	  flow = flow->s_next_flow;
	}
      fprintf (fcfg, ".\n");
    }

  fflush (fcfg);
}


void
PC_PrintGraph (PC_Graph cfg, char *filename)
{
  FILE *Gout;
  PC_Block b;
  PC_Flow f;

  if (!(Gout = fopen (filename, "w")))
    P_punt ("PC_PrintGraph: unable to open output file %s.", filename);

  fprintf (Gout, "digraph G {\n");

  for (b = cfg->first_bb; b; b = b->next)
    {
      fprintf (Gout, "\tBB%d [label=\"BB%d\\n%0.3g\"];\n", b->ID, b->ID,
	       b->weight);
    }

  for (b = cfg->first_bb; b; b = b->next)
    {
      for (f = b->s_flow; f; f = f->s_next_flow)
	{
	  if (f->flow_cond->opcode != OP_int)
	    P_punt ("PC_PrintGraph: Non-integral flow cond");

	  fprintf (Gout, "\tBB%d -> BB%d [label=\"%d:%0.3g\"];\n",
		   f->src_bb->ID, f->dest_bb->ID,
		   (int) f->flow_cond->value.scalar, f->weight);
	}
    }

  fprintf (Gout, "}\n");

  fclose (Gout);
  return;
}

void
PC_PrintPcodeGraph (FILE *F, char *name, PC_Graph cfg)
{
  PC_Flow fl;
  PC_Block bb;
  PC_PStmt ps;

  fprintf (F, "// PCODE CONTROL FLOW GRAPH DUMP: %s //\n", name);

  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      char *cnt = NULL;
      switch (bb->cont_type)
	{
	case CNT_RETURN:
	  cnt = "return";
	  break;
	case CNT_BREAK:
	  cnt = "break";
	  break;
	case CNT_GOTO:
	  cnt = "goto";
	  break;
	case CNT_IF:
	  cnt = "if";
	  break;
	case CNT_SWITCH:
	  cnt = "switch";
	  break;
	case CNT_ENTRY:
	  cnt = "entry";
	  break;
	case CNT_EXIT:
	  cnt = "exit";
	  break;
	default:
	  P_punt ("PC_PrintPcodeGraph: bad cont type");
	}

      /* Print BB header */

      fprintf (F, "# BB %d wt %0.3f cont %s\n", bb->ID, bb->weight, cnt);

      /* Print PStmts */

      for (ps = bb->first_ps; ps; ps = ps->succ)
	{
	  switch (ps->type)
	    {
	    case PC_T_NatPcode:
	      fprintf (F, "   PST: ");
	      P_write_stmt (F, ps->data.stmt, 3, NULL);
	      fprintf (F, "\n");
	      break;
	    case PC_T_Expr:
	      fprintf (F, "   PEX: ");
	      P_write_stmt (F, ps->data.stmt, 3, NULL);
	      fprintf (F, "\n");
	      break;
	    case PC_T_Probe:
	      fprintf (F, "   PPR:\n");
	      break;
	    default:
	      P_punt ("PC_PrintPcodeGraph: bad ps type");
	    }
	}

      /* Print Cond */

      fprintf (F, "COND: ");
      if (bb->cond)
	P_write_expr (F, bb->cond, 0, NULL);
      else
	fprintf (F, "(null)");

      fprintf (F, "\n");

      /* Print successor flows */

      for (fl = bb->s_flow; fl; fl = fl->s_next_flow)
	{
	  fprintf (F, "#    succ BB %d cond ", fl->dest_bb->ID);
	  if (fl->flow_cond)
	    P_write_expr (F, fl->flow_cond, 0, NULL);
	  else
	    fprintf (F, "(null)");
	  fprintf (F, " weight %0.3f\n", fl->weight);
	}

      fprintf (F, "\n");
    }
  return;
}

/*
 * Iterators
 * ----------------------------------------------------------------------
 */

Expr 
PC_ExprIterFirst (PC_Block bb, PC_ExprIter ei, int skip_shadow)
{
  if (!bb)
    P_punt ("PC_ExprIterFirst: bb is NULL");

  ei->bb = bb;
  ei->ps = bb->first_ps;

  return PC_ExprIterNext (ei, skip_shadow);
}

Expr 
PC_ExprIterNext (PC_ExprIter ei, int skip_shadow)
{
  PC_PStmt ps;
  PC_Block bb;
  Expr e = NULL;

  if (!(bb = ei->bb))
    return NULL;  /* All used up */

  for (ps = ei->ps; ps; ps = ps->succ)
    {
      Stmt st;

      if (ps->type != PC_T_NatPcode &&
	  ps->type != PC_T_Expr)
	continue;

      if (!(st = ps->data.stmt))
	P_punt ("PC_ExprIterNext: PStmt has no payload");

      if (st->shadow && skip_shadow)
        continue;

      switch (st->type)
	{
	case ST_EXPR:
	case ST_RETURN:
	  e = st->stmtstruct.expr;
	  break;
	case ST_NOOP:
	  break;
	default:
	  P_warn ("PC_ExprIterNext: Unanticipated stmt type %d",
		  st->type);
	}

      if (e)
	break;
    }

  if (ps)
    {
      ei->ps = ps->succ;
    }
  else
    {
      /* cond is last expr of block */
      ei->ps = NULL;
      ei->bb = NULL;
      e = bb->cond;
    }

  return e;
}


Expr 
PC_ExprIterLast (PC_Block bb, PC_ExprIter ei, int skip_shadow)
{
  ei->bb = bb;
  ei->ps = NULL;

  return PC_ExprIterPrev (ei, skip_shadow);
}

Expr 
PC_ExprIterPrev (PC_ExprIter ei, int skip_shadow)
{
  PC_PStmt ps;
  PC_Block bb;
  Expr e = NULL;

  if (!(bb = ei->bb))
    return NULL;  /* All used up */

  if (!ei->ps && bb->cond)
    {
      if (bb->last_ps)
	ei->ps = bb->last_ps;
      else
	ei->bb = NULL;

      return bb->cond;
    }

  for (ps = ei->ps; ps; ps = ps->pred)
    {
      Stmt st;

      if (ps->type != PC_T_NatPcode &&
	  ps->type != PC_T_Expr)
	continue;

      if (!(st = ps->data.stmt))
	P_punt ("PC_ExprIterNext: PStmt has no payload");

      if (st->shadow && skip_shadow)
        continue;

      switch (st->type)
	{
	case ST_EXPR:
	case ST_RETURN:
	  e = st->stmtstruct.expr;
	  break;
	case ST_NOOP:
	  break;
	default:
	  P_warn ("PC_ExprIterNext: Unanticipated stmt type %d",
		  st->type);
	}

      if (e)
	break;
    }

  if (ps)
    ei->ps = ps->pred;
  else
    ei->bb = NULL;

  return e;
}



