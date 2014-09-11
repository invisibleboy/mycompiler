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
#ifndef _P_INDUCT_H
#define _P_INDUCT_H

#include <Pcode/pcode.h>

#undef  NEWPCODE
#define NEWPCODE	1

#include <Pcode/cfg.h>
#include <library/stack.h>
#include <Pcode/loop.h>
#include <Pcode/struct.h>
#include "ss_ssa2.h"
#include "ss_ind_expr.h"

/* \def DEBUG_FILE
 * \brief the name of the file to dump all debug information to
 */
#define DEBUG_FILE "scc.debug"

/* \def STAT_FILE
 * \brief the name of the file to dump all statistical information to
 */
#define STAT_FILE "scc.stats"

/* typedefs */

typedef enum
{
  NOTYET,
  ONSTACK,
  DONE
} 
PSS_TarNode_Status;


/*! SSA Extension for Tarjan's Loop Cycle Detection Algo
 *
 * This field is intended to be used as an extension to the SSA
 * graph for identifying cycles in the SSA graph (induction vars)
 */
typedef struct _PSS_TarNode
{
  int id; 			/*!< id associated with node */
  
  int lowlink;    		/*!< used with number */
  PSS_TarNode_Status status;   /*!< "color" for tarjan algo */

  struct _PSS_TarLoop *tloop; /*!< link to the control structure of the
				 * current tarjan instance */
  PC_Loop inner_loop; 		/*!< inner loop of expr */
  
  Expr expr; 			/*!< assign expression */

  struct _PSS_TarNode *next;  /*!< list structurs */
  struct _PSS_TarNode *prev;  /*!< list structurs */
}
_PSS_TarNode, *PSS_TarNode;


typedef struct _PSS_TarLoop
{
  Stack *node_stack;  		/*!< stack of tarjan nodes */
  int number; 			/*!< used in tarjan?? */
  int count; 			/*!< count of nodes in loop */
  PC_Graph cfg; 		/*!< link back to cfg */
  PC_Loop pcloop; 		/*!< link back to the loop under 
				 * investigation */
  struct _PSS_TarNode *first; /*!< list of nodes in tloop */
  struct _PSS_TarNode *last;	/*!< list of nodes in tloop */

  struct _PSS_TarSCC *sccs;	/*!< list of PSS_TarSCC's */
  int scc_count;
}
_PSS_TarLoop, *PSS_TarLoop;

typedef enum
{
  UNKNOWN              	= 0x00000001, /*!< Unclassified SCC */
  LINEAR 		= 0x00000002, /*!< Linear SCC */
  LINEAR_MONOTONIC 	= 0x00000004, /*!< Linar SCC w/ PHI node */
  POLYNOMIAL 		= 0x00000008, /*!< Polynomial SCC */
  POLYNOMIAL_MONOTONIC  = 0x00000010, /*!< Polynomial SCC w/ PHI node */
  GEOMETRIC 		= 0x00000020, /*!< Geometric SCC */
  GEOMETRIC_MONOTONIC   = 0x00000040, /*!< Geometric SCC w/ PHI node */
  POINTER 		= 0x00000080, /*!< Geometric SCC */
  POINTER_MONOTONIC     = 0x00000100, /*!< Geometric SCC w/ PHI node */
  IGNORE 		= 0xF0000001, /*!< SCC that can be ignored */
  ANY 			= 0x0FFFFFFF, /*!< ANY SCC except IGNORE 
				       *   (used for searches) */
}
PSS_TarSCC_Type;

typedef struct _PSS_TarSCC
{
  int id; 			/*!< SCC ID */
  PSS_TarSCC_Type type; 	/*!< SCC Type */

  int mu_nodes; 		/*!< count of mu nodes */
  int phi_nodes;		/*!< count of phi nodes.  this will also be -1
				  if there are no phi nodes, but the SCC is
				  dependent on a monotonic IV */

  List tnode_list;		/*!< list of PSS_TarNode's in SCC */

  struct _PSS_TarSCC *next;   /*!< iterator */
  struct _PSS_TarSCC *prev;   /*!< iterator */

  struct _PSS_TarLoop *tloop;	/*!< TarLoop that this SCC is part of */

  char *var_name;		/*!< buffer to hold copy of variable name */
}
_PSS_TarSCC, *PSS_TarSCC;

/* 
 * there are a couple of things that we want to hang off of loops,
 * so to keep it organized we will just hang this off
 */
typedef struct _PSS_LoopExt
{
  struct _PSS_TarLoop *tloop;
  struct _PSS_Fund_Ind_Var *fund_in_var;
}
_PSS_LoopExt, *PSS_LoopExt;
#define P_LoopTarjanLoop(l) (((PSS_LoopExt) l->ext)->tloop)
#define P_LoopFundInVar(l)  (((PSS_LoopExt) l->ext)->fund_in_var)
#define P_LoopSCCs(l) (P_LoopTarjanLoop(l)->sccs)

/* function dcls */
extern void PSS_Find_SCCs (PC_Graph cfg);
extern PSS_TarSCC PSS_Get_SCC (PSS_TarLoop, Expr);

/* print routines */
extern void PSS_Print_TarLoop (PSS_TarLoop tloop);
extern void PSS_PrintLoops (PC_Loop pcloop);
extern void PSS_PrintSCC (PSS_TarSCC scc);
extern void PSS_PrintSCCType(FILE *fp, PSS_TarSCC scc);

#endif
