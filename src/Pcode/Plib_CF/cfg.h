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
                                                                               #ifndef __PLIB_CF_CFG_H__
#define __PLIB_CF_CFG_H__

/*-----------------------------------------------------------------------------*/

#include <config.h>
#include <library/llist.h>
#include <library/i_list.h>
#include <library/i_hash.h>
#include <Pcode/pcode.h>

/* DO NOT CHECK THIS IN SET TO 1 */
#if 0
# define DEBUG 0
#endif

/* Option flags for PC_Function */

#define PC_SPLIT_CRIT     0x00000001 /* Split critical edges           */
#define PC_ANNOTATE       0x00000002 /* Set up for profile annotation  */
#define PC_NORM_LOOPS     0x00000004 /* Normalize loops                */

/* Option flags for freeing PC_Block & PC_PStmt fields */

#define PC_FREE_EXPR      0x00000001
#define PC_FREE_STMT      0x00000002
#define PC_FREE_PROBE     0x00000004


typedef enum
{
  CNT_RETURN,
  CNT_BREAK,
  CNT_GOTO,
  CNT_IF,
  CNT_SWITCH,
  CNT_ENTRY,
  CNT_EXIT
}
_PC_BB_CNT_TYPE;

typedef enum _PC_LoopType
{
  PC_LT_WHILE = 1,		/*!< while loop */
  PC_LT_FOR = 2,		/*!< for loop */
  PC_LT_DO = 3,			/*!< do...while loop */
  PC_LT_GOTO = 4		/*!< goto -> lab type loop */
}
_PC_LoopType;


typedef enum _PC_PSType
{
  PC_T_NatPcode = 1,  		/*!< normal pcode statement */
  PC_T_Expr = 2, 		/*!< pcode statements that were created
				  specially for CFG (loop intilizers) */
  PC_T_Probe = 3
}
_PC_PSType;

typedef enum _PC_ProbeType
{
  PC_PT_LpHead = 1,
  PC_PT_LpExit = 2
}
_PC_ProbeType;

typedef struct _PC_PStmt
{
  _PC_PSType type;
  union
  {
    struct _Stmt *stmt;
    struct _PC_Probe *probe;
  }
  data;
  struct _PC_PStmt *succ;
  struct _PC_PStmt *pred;
  int f_opts;			/* option flags for freeing PC_PStmt fields. */
}
_PC_PStmt, *PC_PStmt;


typedef struct _PC_Probe
{
  int ID;
  _PC_ProbeType type;
}
_PC_Probe, *PC_Probe;


/* Each PC_Graph node is a basic block of Pcode stmts. */
typedef struct _PC_Block
{
  int ID;
  struct _Expr *cond;
  _PC_BB_CNT_TYPE cont_type;	/* control type */
  struct _PC_Flow *s_flow;	/* links to successor basic blocks */
  struct _PC_Flow *p_flow;
  struct _PC_PStmt *first_ps;	/* the first PC_PStmt of the block */
  struct _PC_PStmt *last_ps;	/* the last PC_PStmt of the block */
  struct _PC_Block *next;	/* lexically next block */
  struct _PC_Block *prev;	/* lexically previous block */
  struct __Pragma *pragma;	/* pcode pragmas to be transferred to lcode. */
  struct _PC_LpPrag *lp_prg;	/* for lp prag info used during annotation. */
  List counters;		/* List of pointers to (double) profile
				 * counters needing update with the weight
				 * of this PC_Block. (used only when
				 * PC_opt_annotate = 1)
				 */
  double weight;		/* profile weight of this block. */
  int color;
  int f_opts;			/* option flags for freeing PC_Block fields. */

  /* Dominator information (dom.h) */

  Set doms;
  struct _PC_Block *idom;
  List dom_tree_children;
  List dom_frontier;

  /* Loop information (loop.h) */

  struct _PC_Loop *loop;

  /* Extension field */

  void *ext;
}
_PC_Block, *PC_Block;


typedef struct _PC_Graph
{
  struct _PC_Block *first_bb;	/* lexically first AND entry BB */
  struct _PC_Block *last_bb;	/* lexically last BB */
  struct _PC_Block *exit_bb;	/* function terminus   */

  struct _FuncDcl *func;	/* function structure assoc. with graph */
  unsigned int num_bbs;		/* # of blocks in the PC_Graph */
  unsigned int num_lps;		/* # of loops in the PC_Graph. */
  unsigned int bb_id_offset;	/* offset of all bb ids from 0 */
  struct _PC_Loop *lp;		/* ptr to list of all lps in the PC_Graph. */
  struct _PC_Loop *lp_tree;     /* ptr to loop structures arranged in tree
  				    to signify loop nestings */
  HashTable hash_bb_id;		/* HASH int id --> PC_Block */
  HashTable hash_expr_bb; 	/* hash table (expr->id, pc_block) */
}
_PC_Graph, *PC_Graph;


typedef struct _PC_Flow
{
  PC_Block src_bb;
  PC_Block dest_bb;
  Expr flow_cond;
  double weight;
  int flags;
  struct _PC_Flow *p_next_flow;	/* next in succ bb's pred list */
  struct _PC_Flow *p_prev_flow;	/* prev in succ bb's pred list */
  struct _PC_Flow *s_next_flow;	/* next in pred bb's succ list */
  struct _PC_Flow *s_prev_flow;	/* prev in pred bb's succ list */
}
_PC_Flow, *PC_Flow;

/* PC_Flow flags */

#define PC_FL_NEVER_TAKEN                     0x00000001

typedef struct _PC_LpPrag
{
  struct __Pragma **ptr;	/* ptr to pragma field of pcode lp stmt. */
  _PC_LoopType lp_typ;		/* lp type (FOR/WHILE/DO-WHILE/GOTO). */
  int lp_ln;			/* line # of lp in C source code. */
}
_PC_LpPrag, *PC_LpPrag;


extern PC_Graph PC_Function (FuncDcl func, int init_bb_id, int options);
extern void PC_ApplyToExprs (void (*fn) (Expr));


extern PC_Graph PC_cfg;

/* Allocation / deallocation fuctions */

extern PC_Graph PC_NewGraph (FuncDcl func);
extern PC_Graph PC_FreeGraph (PC_Graph cfg);

extern PC_Block PC_NewBlock (PC_Graph cfg, double wt);
extern PC_Block PC_FreeBlock (PC_Block bb);

extern PC_PStmt PC_NewPStmt (_PC_PSType p_stmt_type, PC_Block bb, Stmt st);
extern PC_PStmt PC_NewPStmtBefore (_PC_PSType ps_type, PC_Block bb, 
				   PC_PStmt psb, Stmt st);
extern PC_PStmt PC_FreePStmt (PC_PStmt ps);
extern PC_PStmt PC_DeletePStmt (PC_Block bb, PC_PStmt ps);

extern PC_PStmt PC_NewPStmtProbe (_PC_PSType ps_type, PC_Block bb, 
				  PC_Probe pb);
extern PC_PStmt PC_NewPStmtExpr (_PC_PSType ps_type, PC_Block bb, Expr expr);
extern PC_PStmt PC_NewPStmtExprBefore (_PC_PSType ps_type, PC_Block bb, PC_PStmt psb, Expr expr);

extern PC_Probe PC_NewProbe (_PC_ProbeType pb_type, int p_ID);
extern PC_Probe PC_FreeProbe (PC_Probe pb);

extern PC_Flow PC_NewFlow (PC_Block src_bb, PC_Block dest_bb, Expr cond,
			   double weight);
extern PC_Flow PC_FreeFlow (PC_Flow fl);
extern void PC_RemoveFlow (PC_Flow fl);

extern void PC_ConnectPStmts (PC_PStmt p_ps, PC_PStmt s_ps);
extern PC_Probe PC_NewProbe (_PC_ProbeType pb_type, int p_ID);

extern void PC_PrintGraph (PC_Graph cfg, char *filename);
extern Set PC_BuildUnReachableBBSet (PC_Graph cfg);

extern void PC_PrintPcodeGraph (FILE *F, char *name, PC_Graph cfg);

extern int PC_NextLoopID ();

PC_Block PC_FindBlock (PC_Graph g, int id);
PC_Block PC_FindExprBB (PC_Graph g, int id);

#define DEFAULT_VALUE 0x7FFFFFFF
#define TrueExpr() PSI_ScopeNewIntExpr(PSI_GetGlobalScope (), 1)
#define FalseExpr() PSI_ScopeNewIntExpr(PSI_GetGlobalScope (), 0)
#define DefaultExpr() PSI_ScopeNewIntExpr(PSI_GetGlobalScope (), DEFAULT_VALUE)
#define IsDefaultExpr(e) ((e) && ((e)->opcode == OP_int) && \
                          ((e)->value.scalar == DEFAULT_VALUE))

typedef struct _PC_ExprIter {
  PC_Block bb;
  PC_PStmt ps;
} _PC_ExprIter, *PC_ExprIter;

extern Expr PC_ExprIterFirst (PC_Block bb, PC_ExprIter ei, int skip_shadow);
extern Expr PC_ExprIterNext (PC_ExprIter ei, int skip_shadow);

extern Expr PC_ExprIterLast (PC_Block bb, PC_ExprIter ei, int skip_shadow);
extern Expr PC_ExprIterPrev (PC_ExprIter ei, int skip_shadow);

#endif /* __PLIB_CF_CFG_H__ */
