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


#ifndef __PLIB_CF_LOOP_H__
#define __PLIB_CF_LOOP_H__
/*-----------------------------------------------------------------------------*/

#include <Pcode/cfg.h>

typedef struct _PC_Loop
{
  int ID;
  int head;
  Set body;			/*!< BB's in loop */
  Set exits;
  struct _PC_Loop *next;

  int num_exit;			/* num of exits */
  int num_back_edge; 		/* num of back-edges */
  int nesting_level;		/* outmost is 1 */
  struct _PC_Loop *child;	/* child loops */
  struct _PC_Loop *sibling;	/* sibling loops */
  struct _PC_Loop *parent;	/* parent loop */

  double entr_wt;
  double body_wt;

  void *ext;			/* extension field for temps */
}
_PC_Loop, *PC_Loop;

#define Get_Loop_id(l)                  ((l)->ID)
#define Get_Loop_head(l)                ((l)->head)
#define Get_Loop_body(l)                ((l)->body)
#define Get_Loop_exits(l)               ((l)->exits)
#define Get_Loop_next(l)		((l)->next)
#define Get_Loop_nesting_level(l)       ((l)->nesting_level)
#define Get_Loop_child(l)               ((l)->child)
#define Get_Loop_sibling(l)             ((l)->sibling)
#define Get_Loop_parent(l)              ((l)->parent)
#define Get_Loop_num_exit(l)		((l)->num_exit)
#define Get_Loop_num_back_edge(l)	((l)->num_back_edge)

#define PC_ExprInnerLoop(G,EXPR) (PC_FindExprBB(G,EXPR->id)->loop)

extern void PC_FindBackEdges (PC_Graph cfg);
extern void PC_FindLoops (PC_Graph cfg);
extern void PC_PrintLoop (FILE * f, PC_Loop lp);

extern PC_Loop PC_NewLoop (int head, Set body, Set exits, int num_back_edge, int num_exit);
extern PC_Loop PC_FreeLoop (PC_Loop lp);
extern int PC_LoopContainsLoop (PC_Loop l1, PC_Loop l2);
extern int PC_LoopContainsExpr (PC_Graph cfg, PC_Loop lp, Expr expr);
extern int PC_BB_In_Child_Loop (int, PC_Loop);
extern int PC_BB_In_Direct_Ancestor_Loop_Scope (PC_Graph, PC_Loop, int);
/*! /brief return the loopnest depth of a loop */
extern int PC_LoopDepth(PC_Loop pcloop);

#endif /* __PLIB_CF_LOOP_H__ */
