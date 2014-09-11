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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <library/set.h>
#include <library/dynamic_symbol.h>
#include <Pcode/parms.h>
#include <Pcode/perror.h>
#include <Pcode/gen_ccode.h>
#include <Pcode/ss_induct.h>
#include <Pcode/ss_setup.h>
#include "dd_setup.h"
#include "dd_preprocess.h"
#include "dd_interface.h"


/* from dd_dependence.c */

extern Expr P_DD_qualified_for_omega_test (Expr expr);
extern uint Find_Accesses_Common_Loop_Depth (P_ExprExtForDD oa, 
					     P_ExprExtForDD ia);
extern void P_DD_omega_test (Expr expr1, Expr expr2, 
			     P_DepType oitype, P_DepType iotype,
			     unsigned int nest1, unsigned int nest2, 
			     unsigned int bnest);


static int
PD_IsMemReadExpr (Expr expr)
{
  P_memdep_t md;
  P_memdep_core_t dep;

  md = P_GetMemDep (expr);
  assert (List_size (md->deps) > 0);
  dep = List_get_first (md->deps);
  return (dep->is_def == 0);
}


static int
PD_IsMemWriteExpr (Expr expr)
{
  P_memdep_t md;
  P_memdep_core_t dep;

  md = P_GetMemDep (expr);
  assert (List_size (md->deps) > 0);
  dep = List_get_first (md->deps);
  return (dep->is_def == 1);
}


static int
PD_ExprsMayAlias (Expr expr1, Expr expr2)
{
  P_memdep_t md1, md2;
  List mem_obj_list1, mem_obj_list2;
  P_memdep_core_t mem_obj1, mem_obj2;

  if (!(md1 = P_GetMemDep (expr1)) ||
      !(md2 = P_GetMemDep (expr2)))
    return 0;

  if (md1 == md2)
    return 1;

  mem_obj_list1 = md1->deps;
  mem_obj_list2 = md2->deps;

  List_start (mem_obj_list1);
  while ((mem_obj1 = List_next (mem_obj_list1)))
    {
      List_start (mem_obj_list2);
      while ((mem_obj2 = List_next (mem_obj_list2)))
	{
	  if ((mem_obj1->id != mem_obj2->id) || 
	      (mem_obj1->version != mem_obj2->version))
	    continue;

	  if (mem_obj1->size == -1 || mem_obj2->size == -1)
	    {
	      /* Sizes non-descriptive */
	      return 1;
	    }
	  else
	    {
	      int o1 = mem_obj1->offset, o2 = mem_obj2->offset,
		s1 = mem_obj1->size, s2 = mem_obj2->size;

	      /* Determine if accesses overlap */
	      if ((o1 <= o2 && (o1 + s1) > o2) ||
		  (o2 < o1 && (o2 + s2) > o1))
		return 1;
	    }
	}
    }

  return 0;
}


static void
PD_print_mem_access (FILE * out_file, Expr expr)
{
  P_memdep_t md;
  List mem_obj_list;
  P_memdep_core_t mem_obj;

  md = P_GetMemDep (expr);
  assert (md);
  mem_obj_list = md->deps;
  assert (List_size (mem_obj_list) > 0);
  fprintf (stderr, "%5s {(id, offset, size)} = { ",
	   PD_IsMemWriteExpr (expr) ? "write" : "read");
  List_start (mem_obj_list);
  while ((mem_obj = List_next (mem_obj_list)))
    fprintf (stderr, "(%5d,%5d,%5d) ", mem_obj->id, mem_obj->offset,
	     mem_obj->size);
  fprintf (stderr, "}");

  return;
}



static void
AddExprToList (Expr expr, List * plist)
{
  P_memdep_t md;

  md = P_GetMemDep (expr);
  if ((List_size (md->deps) > 0))
    *plist = List_insert_last (*plist, expr);

  return;
}

/*
 * Return a list of all Expr with memory accesses
 */
static List
FindAllExprWithMemAcc (FuncDcl func)
{
  List expr_list = NULL;
  Stmt stmt = P_GetFuncDclStmt (func);

  P_StmtApply (stmt, NULL, (void (*)(Expr, void *)) AddExprToList,
	       (void *) &expr_list);

  return expr_list;
}


static int
PD_ExprIsEqual (Expr e1, Expr e2)
{
  int ret;

  if (e1 == e2)
    return 1;
  if ((e1 == NULL) || (e2 == NULL))
    return 0;
  if (e1->opcode != e2->opcode)
    return 0;
  switch (e1->opcode)
    {
    case OP_var:
      ret = !strcmp (P_GetExprVarName (e1), P_GetExprVarName (e2));
      break;
    default:
      if (DD_DEBUG_OMEGA)
	fprintf (stderr, "\nWARNING: ExprIsEqual not handled opcode\n");
      ret = 0;
      break;
    }
  return ret;
}


static void
PD_produce_cyclic_sync_arc (Expr expr1, Expr expr2)
{
  P_DepType deptype12, deptype21;
  uint a1nest, a2nest, bnest;
  Expr array1, array2;
  P_ExprExtForDD a1, a2;

  if (DD_DEBUG_OMEGA)
    {				/* CWL - 01/09/03 */
      fprintf (stderr, "\n<<<<<\n");
      fprintf (stderr, "<<<<< DD_produce_cyclic_sync_arc\n");
      fprintf (stderr, "<<<<<\n");
      fprintf (stderr, "Expr (%5d) at line (%5d): ",
	       P_GetExprID (expr1),
	       P_GetStmtLineno (P_GetExprParentStmt (expr1)));
      Gen_CCODE_Expr (stderr, expr1);
      fprintf (stderr, "\n");
      PD_print_mem_access (stderr, expr1);
      fprintf (stderr, "\n");
      fprintf (stderr, "\n");
      fprintf (stderr, "Expr (%5d) at line (%5d): ",
	       P_GetExprID (expr2),
	       P_GetStmtLineno (P_GetExprParentStmt (expr2)));
      Gen_CCODE_Expr (stderr, expr2);
      fprintf (stderr, "\n");
      PD_print_mem_access (stderr, expr2);
      fprintf (stderr, "\n");
    }

  a1 = Get_ExprExtForDD (expr1);
  a2 = Get_ExprExtForDD (expr2);

  if ((array1 = P_DD_qualified_for_omega_test (expr1)) &&
      (array2 = P_DD_qualified_for_omega_test (expr2)) &&
      PD_ExprIsEqual (array1, array2) &&
      (Get_ExprExtForDD_first_context (a1) ==
       Get_ExprExtForDD_first_context (a2)))
    {
      if (PD_IsMemReadExpr (expr1) && 
	  PD_IsMemWriteExpr (expr2))
	{
	  deptype12 = DT_ANTI;
	  deptype21 = DT_FLOW;
	}
      else if (PD_IsMemWriteExpr (expr1) &&
	       PD_IsMemReadExpr (expr2))
	{
	  deptype12 = DT_FLOW;
	  deptype21 = DT_ANTI;
	}
      else if (PD_IsMemWriteExpr (expr1) &&
	       PD_IsMemWriteExpr (expr2))
	{
	  deptype12 = DT_OUTPUT;
	  deptype21 = DT_OUTPUT;
	}
      else
	{
	  P_punt ("DD_produce_cyclic_sync_arc: invalid ddtypes");
	  deptype12 = DT_NONE;
	  deptype21 = DT_NONE;
	}

      bnest = Find_Accesses_Common_Loop_Depth (a1, a2);

      if (Get_ExprExtForDD_first_context (a1))
	a1nest = Get_PO_Context_depth (Get_ExprExtForDD_first_context (a1));
      else
	a1nest = 1;

      if (Get_ExprExtForDD_first_context (a2))
	a2nest = Get_PO_Context_depth (Get_ExprExtForDD_first_context (a2));
      else
	a2nest = 1;

      /* BCC - 10/13/99
       * Changes made to omit outer loop nests:
       * 1) In dd_omega-build.c, added extra cont_i_next(c) in function
       *    bound_indices_and_conditionals(), load_bounds_and_count_steps(),
       *    and load_constants_for_bounds().
       * 2) In dd_data-dep.c, function DD_ParLoop_Stmt(), always assign 1 to
       *    loop depth
       */

      if (deptype12 == DT_ANTI)
	P_DD_omega_test (expr2, expr1, deptype21, deptype12,
			 a2nest, a1nest, bnest);
      else
	P_DD_omega_test (expr1, expr2, deptype12, deptype21,
			 a1nest, a2nest, bnest);
    }

  return;
}


void
DD_DependenceTest (FuncDcl func)
{
  List expr_list;
  Expr expr1, expr2;

  /* 
   * JWS: There is no reason to do this for the whole function at once.
   * Why not do it for one loop nest at a time?
   */

  /* 
   * Find all the expressions which access memory. The resulting list
   * is stored in expr_list.  
   */

  if (!(expr_list = FindAllExprWithMemAcc (func)))
    return;			/* no expressions to examine */

  if (DD_DEBUG_OMEGA)
    fprintf (stderr, "DD_DependenceTest func=%s\n", func->name);

  DD_Preprocess (func);

  initializeOmega ();

  /* do pairwise comparison between expressions which access memory. */

  List_start (expr_list);
  while ((expr1 = List_first (expr_list)))
    {
      expr_list = List_delete_current (expr_list);

      while ((expr2 = List_next (expr_list)))
	{
	  if ((PD_IsMemWriteExpr (expr1) || 
	       PD_IsMemWriteExpr (expr2)) &&
	      PD_ExprsMayAlias (expr1, expr2))
	    PD_produce_cyclic_sync_arc (expr1, expr2);
	}
    }

  expr_list = List_reset (expr_list);

  if (DD_DEBUG_OMEGA)
    DD_PrintFuncDepInfo (stderr, func);

  return;
}
