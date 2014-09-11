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
 *      File:   annotate.c
 *      Author: Ben-Chung Cheng
 *      Copyright (c) 1995 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.
 *      All rights granted to the University of Illinois.
 *      The University of Illinois software License Agreement
 *      specifies the terms and conditions for redistribution.
\*****************************************************************************/

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <library/list.h>
#include <library/dynamic_symbol.h>
#include "pin_inline.h"
#include <Pcode/cast.h>
#include <Pcode/query.h>
#include <Pcode/symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/parloop.h>

static int
Pin_is_vararg (FuncDcl fn)
{
  VarList param_list = fn->param;
  VarDcl param;

  List_start (param_list);
  while ((param = List_next (param_list)))
    {
      /* In ANSI-C, "..." is a vararg parameter */
      if (!strcmp (param->name, "__builtin_impact_ellipsis") ||
	  !strcmp (param->name, "__builtin_va_alist"))
	return 1;
    }
  return 0;
}


static int
Pin_is_empty (FuncDcl fn)
{
  Stmt stmt;

  if (!(stmt = fn->stmt))
    return 1;

  while (stmt->type == ST_COMPOUND)
    if (!(stmt = stmt->stmtstruct.compound->stmt_list) || stmt->lex_next)
      break;

  return (!stmt || ((stmt->type == ST_RETURN) && !stmt->stmtstruct.ret));
}


static int
Pin_get_indir_callee (Expr expr, Pragma *p_pragma, double *p_arcwt,
		      Key *p_key)
{
  double wrat;
  Pragma pragma;

  if (!(expr->operands->opcode == OP_indr))
    return 0;

  if (!(pragma = *p_pragma))
    pragma = expr->pragma;
  else
    pragma = pragma->next;

  for (; pragma; pragma = pragma->next)
    if (!strncmp (pragma->specifier, PIN_PRG_IPC_PFX, PIN_PRG_IPC_PFX_LEN))
      break;

  *p_pragma = pragma;

  if (!pragma)
    return 0;

  if (sscanf (pragma->specifier, PIN_PRG_IPC_PFX "%lf", &wrat) != 1)
    P_punt ("Pin_ann_call_expr: error reading IPC pragma");

  if (p_arcwt)
    *p_arcwt = wrat * expr->profile->count;

  if (p_key)
    *p_key = pragma->expr->value.var.key;

  return 1;
}


/* JWS: Used only in building initial graph */
static PinCG_Arc
Pin_add_arc (PinCG_Node caller_node, int callsite_id,
	     Key callee_key, double weight, 
	     int direct, int noinline, int n_parms)
{
  PinCG_Func callee_cgf, caller_cgf = caller_node->func;
  PinCG_Arc new_arc;
  FuncDcl fd;
  char *func_name;

  /* Make an arc only when we have callee source */  

  if (!P_ValidKey (callee_key))
    return 0;

  fd = PSI_GetFuncDclEntry (callee_key);
  callee_cgf = Pin_fdcl_cgf (fd);
  func_name = fd->name;

  if (!callee_cgf)
    return 0;

  /* DETERMINE IF ARC IS ALLOWED UNDER CURRENT RULES */

  if (is_in_Func_Name_List (prevent_inline_list, func_name))
    noinline = 1; /* Exclude arcs to blacklisted callees                     */
  else if (callee_cgf->is_noninline)
    noinline = 2; /* Exclude arcs to functions marked to prevent inlining    */
  else if ((weight == 0.0) && !callee_cgf->is_always_inline && 
	   !inline_all_functions)
    noinline = 3; /* Exclude zero-weight arcs, unless marked to inline       */
  else if (prevent_cross_file_inlining &&
	   strcmp (caller_cgf->orig_filename, callee_cgf->orig_filename))
    noinline = 4; /* Conditionally, exclude cross-file inlining              */

  /* CREATE AN ARC */

  new_arc = PinCG_create_arc (caller_node, callee_cgf->node,
			      callsite_id, weight);

  new_arc->noinline = noinline;
  new_arc->indirect = !direct;
  new_arc->use_orig = !inline_inlined_body;

  new_arc->num_parms = n_parms;

  /* Record indirect weight.  This accumulated value will be
   * subtracted from the total function weight after the first
   * annotation pass to yield the actual weight not accounted for by
   * arcs.  
   */

  if (weight > 0.0)
    callee_cgf->indir_weight -= weight;

  if (!noinline)
    Heap_Insert (Pin_arc_heap, new_arc, PinCG_compute_arc_key (new_arc));

  return new_arc;
}


static int
Pin_parm_cnt (Expr expr)
{
  int cnt = 0;
  while (expr)
    {
      cnt++;
      expr = expr->next;
    }

  return cnt;
}


/*
 * Add appropriate new PinCG arc(s) for the indicated expression.
 */
static int
Pin_add_arcs (PinCG_Node caller_node, Expr expr, double base_weight)
{
  int direct, cnt = 0, noinline = 0, n_parms;
  double weight, arcwt;
  Expr callee_expr;
  PinCG_Arc arc;
  Key callee_key;

  if (size_only)
    weight = 1.0;
  else
    weight = (expr->profile) ? expr->profile->count : 0.0;

  TotalCallWeight += weight;

  if (P_FindPragma (expr->pragma, "DONTINLINE"))
    noinline = 1;

  callee_expr = expr->operands;
  switch (callee_expr->opcode)
    {
    case OP_var:
      direct = 1;
      break;
    case OP_indr:
      direct = 0;
      break;
    case OP_compexpr:		/* should reduce (foo)() to foo() */
      if (print_inline_trace)
	fprintf (Flog, "Warning: Unflattened function call\n");
      return 0;
    default:
      P_punt ("Pin_add_arcs: Illegal call expr %d", callee_expr->opcode);
      return 0;
    }

  /* Add an arc for each callee */

  n_parms = Pin_parm_cnt (expr->operands->sibling);

  if (direct)
    {
      arcwt = weight;
      callee_key = expr->operands->value.var.key;
      if ((arc = Pin_add_arc (caller_node, expr->id,
			      callee_key, arcwt,
			      direct, noinline, n_parms)))
	cnt++;
    }
  else
    {
      Pragma pragma = NULL;

      if (!inline_function_pointers)
	noinline = 1;

      if (inline_indir_by_profile &&
	  Pin_get_indir_callee (expr, &pragma, &arcwt, &callee_key))
	{
	  do
	    {
	      if (!(arc = Pin_add_arc (caller_node, expr->id,
				       callee_key, arcwt,
				       direct, noinline, n_parms)))
		continue;

	      cnt++;

	      if ((weight < 1.0) || ((arcwt / weight) < indir_thresh))
		arc->noinline = 1;

#if 0
	      if (print_inline_trace && !arc->noinline)
		fprintf (Flog, "\tAdded inlinable arc %d:%d %s() -i-> %s() "
			 "(wt %0.3f/%0.3f)\n", callsite_id, arc->id,
			 caller_node->funcname, 
			 arc->callee->func->funcname, arcwt, weight);
#endif
	    }
	  while (Pin_get_indir_callee (expr, &pragma, &arcwt, &callee_key));
	}
#if 0
      else if (print_inline_trace)
	{
	  fprintf (Flog, "> Undiff fptr call in %s() (wt %0.3f)\n",
		   caller_node->funcname, weight);
	}
#endif
    }

  return cnt;
}


static void
Pin_ann_call_expr (PinCG_Node cg_node, Stmt stmt, double max_parent_weight)
{
  Expr expr = stmt->stmtstruct.expr;

  if (expr->next)
    return;			/* expr list "a(),b()" won't be inlined */

  if (!Pin_find_call (expr, &expr, NULL, NULL))
    return;

  Pin_add_arcs (cg_node, expr, max_parent_weight);

  return;
}


static void
Pin_find_call_exprs (PinCG_Node cg_node, Stmt stmt, double max_parent_weight)
{
  SerLoop serloop;
  ParLoop parloop;

  for (; stmt; stmt = stmt->lex_next)
    {
      double new_parent_weight = max_parent_weight;

      if (stmt->profile)
	{
	  if (stmt->profile->count > max_parent_weight)
	    new_parent_weight = stmt->profile->count;

	  if (cg_node->weight == 0.0)
	    cg_node->func->weight = cg_node->weight = stmt->profile->count;
	}

      switch (stmt->type)
	{
	case ST_NOOP:
	case ST_CONT:
	case ST_BREAK:
	case ST_GOTO:
	case ST_ADVANCE:
	case ST_AWAIT:
	case ST_RETURN:
	case ST_ASM:
	  break;
	case ST_COMPOUND:
	  Pin_find_call_exprs (cg_node, stmt->stmtstruct.compound->stmt_list,
			       new_parent_weight);
	  break;
	case ST_IF:
	  Pin_find_call_exprs (cg_node, stmt->stmtstruct.ifstmt->then_block,
			       new_parent_weight);
	  if (stmt->stmtstruct.ifstmt->else_block)
	    Pin_find_call_exprs (cg_node, stmt->stmtstruct.ifstmt->else_block,
				 new_parent_weight);
	  break;
	case ST_SWITCH:
	  Pin_find_call_exprs (cg_node, 
			       stmt->stmtstruct.switchstmt->switchbody,
			       new_parent_weight);
	  break;
	case ST_EXPR:
	  if ((cg_node->weight == 0.0) && stmt->stmtstruct.expr->profile)
	    cg_node->weight = stmt->stmtstruct.expr->profile->count;
	  Pin_ann_call_expr (cg_node, stmt, new_parent_weight);
	  break;
	case ST_PSTMT:
	  Pin_find_call_exprs (cg_node, stmt->stmtstruct.pstmt->stmt,
			       new_parent_weight);
	  break;
	case ST_MUTEX:
	  Pin_find_call_exprs (cg_node, stmt->stmtstruct.mutex->statement,
			       new_parent_weight);
	  break;
	case ST_COBEGIN:
	  Pin_find_call_exprs (cg_node, stmt->stmtstruct.cobegin->statements,
			       new_parent_weight);
	  break;
	case ST_SERLOOP:
	  serloop = stmt->stmtstruct.serloop;
	  Pin_find_call_exprs (cg_node, serloop->loop_body, new_parent_weight);
	  break;
	case ST_PARLOOP:
	  parloop = stmt->stmtstruct.parloop;
	  Pin_find_call_exprs (cg_node, Parloop_Stmts_Prologue_Stmt (stmt),
			       new_parent_weight);
	  break;
	case ST_BODY:
	  Pin_find_call_exprs (cg_node, stmt->stmtstruct.bodystmt->statement,
			       new_parent_weight);
	  break;
	case ST_EPILOGUE:
	  Pin_find_call_exprs (cg_node, 
			       stmt->stmtstruct.epiloguestmt->statement,
			       new_parent_weight);
	  break;
	default:
	  P_punt ("Pin_find_call_exprs: Invalid statement type", stmt);
	}
    }
}


void
Pin_preprocess_func (PinCG_Func cgf, FuncDcl func, double base_ratio)
{
/*
 * BCC - 3/7/96
 * foo1()
 * {
 *	foo2()
 *	{
 *		first_op;
 *		another call;
 *	}
 * }
 *
 * Purpose : (1) get the weight of the first op of foo2
 *	     (2) mesure the body size and stack size of the function
 */
  Stmt stmt;
  double base_weight;
  PinCG_Node cg_node = cgf->node;

  if (cgf->o_bodysize == 0 && cgf->o_stacksize == 0)
    {
      VarList var_list = func->param;
      VarDcl var_dcl;
      long int sz_stk, sz_body, sz_ebody;

      Pin_function_size (func, &sz_stk, &sz_body, &sz_ebody);

      cgf->o_bodysize = cgf->i_bodysize = sz_body;
      cgf->o_ebodysize = cgf->i_ebodysize = sz_ebody;
      cgf->o_stacksize = cgf->i_stacksize = sz_stk;

      TotalBodySize += sz_body;
      TotalEBodySize += sz_ebody;
      TotalStackSize += sz_stk;

      /* BCC - calculate the number of parms - 4/21/96 */
      cgf->num_parms = 0;
      List_start (var_list);
      while ((var_dcl = (VarDcl) List_next (var_list)))
	cgf->num_parms++;

      cgf->is_vararg = Pin_is_vararg (func);
      cgf->is_empty_func = Pin_is_empty (func);
    }

  if (!size_only)
    {
      stmt = func->stmt->stmtstruct.compound->stmt_list;
      while (stmt && !stmt->profile)
	{
	  switch (stmt->type)
	    {
	    case ST_COMPOUND:
	      stmt = stmt->stmtstruct.compound->stmt_list;
	      break;
	    case ST_NOOP:
	    case ST_EXPR:
	      stmt = stmt->lex_next;
	      break;
	    default:
	      P_punt ("Need to extend pin_annotate.c");
	      break;
	    }
	}

      base_weight = stmt ? base_ratio * stmt->profile->count : 0.0;
    }
  else
    {
      cg_node->weight = 1.0;
      base_weight = base_ratio;
    }

  Pin_find_call_exprs (cg_node, func->stmt, base_weight);

  return;
}
