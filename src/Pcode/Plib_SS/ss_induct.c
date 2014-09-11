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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <library/dynamic_symbol.h>
#include <Pcode/pcode.h>
#include <Pcode/query.h>
#include <Pcode/struct.h>
#include <Pcode/cast.h>
#include <Pcode/symtab_i.h>
#include <Pcode/gen_ccode.h>
#include "ss_ssa.h"
#include "ss_induct.h"
#include "ss_setup.h"

/**********************************************************************************
 	Globals
***********************************************************************************/

#define DEBUG_SS_VGRAPH 0

PC_Graph curr_CFG_for_vgraph;	/* should hide this in FuncDcl */

Key ss_curr_func_scope;

/**********************************************************************************
        Static/private variables
***********************************************************************************/

#define INDENT_STRING	"\t"

static int visit_id;
static _LptrQ vnode_stack;
static char err_msg[256];
static _PC_Loop dummy_loop;

static int Dump_VnodeFindValue;

static STRING_Symbol_Table *var_id_tbl;

static P_Vgraph curr_vgraph;

static Stmt dummy_stmt;

/**********************************************************************************
        Static/private function header
***********************************************************************************/

#define STRING_Symbol_data(s)		((s)->data)

/* 
 * Vgraph operations 
 */
static P_Vgraph NewVgraph (PC_Graph cfg);

static P_Vnode BuildExprVgraph (P_Vgraph vgraph, Expr expr, PC_Block bb);

/* 
 * Scc operations 
 */
static P_Scc NewScc (int id);
static void FreeSccInVgraph (P_Vgraph vgraph);
static void FreeScc(P_Scc scc);
static int SccVisit (P_Vgraph vgraph, P_Vnode node);
static void DumpScc(FILE *out_file, P_Scc scc, int dump_vnode);
static void RegisterSccInLoops(P_Scc scc_list);
static void FreeSccInLoops(PC_Loop loop_list);
static void FreeSccInLoop(PC_Loop loop);

/* 
 * Vnode operations 
 */
static P_Vnode NewVnode(P_Vgraph vgraph);
static void VnodeAppendNewSuccVnode (P_Vnode vnode, P_Vnode succ_vnode);
static P_Vnode FindAndAllocateVnode (P_Vgraph vgraph, Expr expr, PC_Block bb);
static void DumpVnode (FILE *out_file, P_Vnode node, DumpVnodeFormat dump_format, int level);
static void DumpVnodeExpr (FILE *out_file, Expr expr);
static P_Value VnodeClosedValue (P_Vnode vnode, PC_Loop user_loop);
static P_Value VnodeFindValue (P_Vnode vnode);
static void SetLoopInformation (P_Vnode vnode);

static PC_Loop Vnode_loop(P_Vnode v);

/* 
 * Value operands 
 */
#if 0
static void DumpValue(FILE *out_file, P_Value value);
#endif
static P_Value NewValue();
static P_Value NewUnknownValue();
static P_Value NewIntValue (int n);
static P_Value NewConstValue (Expr expr);
static P_Value NewInductValue (Alpha_var_id vid, P_Value init, Expr incr);
static P_Value ValueClose (P_Value v, PC_Loop user_loop);
static int ValueEqual (P_Value v1, P_Value v2);
static P_Value ValueComputeConstConst (_Opcode opcode, P_Value v1, P_Value v2);
static P_Value ValueComputeInductConst (_Opcode opcode, P_Value v1, P_Value v2);
static P_Value ValueComputeConstInduct (_Opcode opcode, P_Value v1, P_Value v2);
static P_Value ValueComputeInductInduct (_Opcode opcode, P_Value v1, P_Value v2);
static P_Value ValueComputeWithTwoOperands (_Opcode opcode, P_Value v1, P_Value v2);
static P_Value ValueComputeConst (_Opcode opcode, P_Value v);
static P_Value ValueComputeInduct (_Opcode opcode, P_Value v);
static P_Value ValueComputeWithOneOperand (_Opcode opcode, P_Value v);
static P_Value CopyConstValue (P_Value v);
static P_Value CopyInductValue (P_Value v);
static P_Value CopyValue (P_Value v);
static P_Value NormalizedLoopCondition (P_Value value, Expr expr, int reverse_condition);
static P_Value ComputeLoopTripCount (P_Value loop_condition, int plus1);
static void FindValuesInLoop (PC_Loop loop);
static void FindValuesInScc (P_Scc scc);
static void FreeUnknownValue(P_Value value);

/* 
 * Induction variable 
 */

static Alpha_var_id New_Alpha_var_id (Key key, char *name, struct _PC_Loop *loop);
static Alpha_var_id Copy_Alpha_var_id (Alpha_var_id v);
static Alpha_var_id NewInductionVarId(PC_Loop loop);
static void FreeAlphaVarId(void *v);

/* 
 * misc 
 */
static void DumpIndent(FILE *out_file, int level, char *str);

static Expr UncastedExpr (Expr expr);
static int IsCommutative (_Opcode opcode);

static int ConstExprEqual (Expr e1, Expr e2);
static int ConstValueEqual (Expr e1, Expr e2);
static int ConstValueEqualOfDualOp (Expr expr1, Expr expr2);
static int ConstValueEqualOfCommutativeOp (Expr expr1, Expr expr2);
static bool IsExitBlock(PC_Block bb, PC_Loop loop);

/*
 * Expr
 */
static Expr NewIntExpr (ITintmax i);
static Expr NewStringExpr (char *str);
static Expr NewFloatExpr (double f);
static Expr NewDoubleExpr (double f);
static Expr NewVarExpr(char *name, Type type, Key key);
static Expr NewDualOpExpr (_Opcode opcode, Expr op1, Expr op2);
static Expr NewSingleOpExpr (_Opcode opcode, Expr op1);
static Expr NewCastExpr (Type type, Expr op1);
static Expr ReduceExpr (Expr expr);

/*
 * potential cfg stuff
 */
static void Set_CFG_vgraph (PC_Graph cfg, P_Vgraph vg);


/**********************************************************************************
        Export function body
***********************************************************************************/
/*
 * assumption: SSA is built 
 */
P_Vgraph 
P_SS_BuildVgraph(PC_Graph cfg)
{
  _PC_ExprIter ei;
  PC_Block bb;
  P_Vgraph vgraph;
  Expr expr;

  vgraph = Get_CFG_vgraph (cfg);

  if (vgraph != NULL) 
    P_SS_FreeVgraph(vgraph);

  vgraph = NewVgraph(cfg);
  Set_CFG_vgraph(cfg, vgraph);
  ss_curr_func_scope = P_GetFuncDclKey(cfg->func);

  for (bb = cfg->first_bb; bb; bb = bb->next) {
    for (expr = PC_ExprIterFirst (bb, &ei, 1); expr; expr = PC_ExprIterNext (&ei, 1))
      BuildExprVgraph (vgraph, expr, bb);
  }

  #if DEBUG_SS_VGRAPH
  {
    char fname[128];
    FILE *out_file;

    sprintf(fname, "%s.vgraph.P_SS_BuildVgraph", cfg->func->name);
    out_file = fopen(fname, "w");
    assert (out_file != NULL);
    fprintf(out_file, "P_SS_BuildVgraph ......\n\n");
    P_SS_DumpVgraph(out_file, vgraph);
    fclose(out_file);
  }
  #endif

  return vgraph;
}

P_Vgraph
Get_CFG_vgraph (PC_Graph cfg)
{
  return curr_vgraph;
  /* should store this in some ext */
}

void
P_SS_FreeValue(void *value)
{
  Expr expr;
  P_Value v;

  if (!value)
    return;

  v = value;
  if (Value_is_unknown(v)) {
    FreeUnknownValue(v);    
    return;
  }

  if (Value_is_const(v)) {
    expr = Value_const(v);
    P_RemoveExpr(expr);
    Value_const (v) = NULL;
  } else if (Value_is_induct(v)) {
    P_SS_FreeValue(Value_induct_init(v));
    P_RemoveExpr(Value_induct_incr(v));
  } else 
    P_punt ("P_SS_FreeValue: not const nor induct");

  DISPOSE(v);
}

void
P_SS_FreeVgraph (P_Vgraph vgraph)
{
  P_Vnode curr_vnode;
  P_Vnode prev_vnode;

  if (vgraph == NULL)
    return;
  /*
   * Free SCC
   */
  FreeSccInVgraph(vgraph);
  /*
   * Free vnodes 
   */
  curr_vnode = vgraph->first_vnode; 
  while (curr_vnode) {
    prev_vnode = curr_vnode;
    curr_vnode = curr_vnode->next; 
    DISPOSE (prev_vnode);
  }
  DISPOSE (vgraph); 
}

void
P_SS_DumpVgraph (FILE *out_file, P_Vgraph vgraph)
{
  P_Vnode vnode;

  for (vnode = vgraph->first_vnode; vnode; vnode = vnode->next) {
    DumpVnode(out_file, vnode, 0, 0);
    fprintf(out_file, "\n");
  }
}

void 
P_SS_FindSccInVgraph (P_Vgraph vgraph)
{
  P_Vnode node;

  visit_id = 0; 
  for (node = vgraph->first_vnode ; node ; node = node->next)
    Vnode_status(node) = -1;

  vnode_stack.head = NULL;
  vnode_stack.tail = NULL;
  vnode_stack.length = 0;

  FreeSccInVgraph(vgraph);

  for (node = Vgraph_first_vnode(vgraph) ; node ; node = Vnode_next(node))
    if (Vnode_status(node) == -1) 
      SccVisit(vgraph, node); 
}

void
P_SS_DumpScc (FILE *out_file, P_Scc scc_list, int dump_vnode)
{
  P_Scc curr_scc;

  for (curr_scc = scc_list; curr_scc; curr_scc = curr_scc->next) {
    DumpScc(out_file, curr_scc, dump_vnode);
    fprintf(out_file, "\n");
  }
}

void
P_SS_FindValuesInCFG (PC_Graph cfg)
{
  P_Vgraph vgraph;
  PC_Loop loop;

  dummy_stmt = P_GetFuncDclStmt(cfg->func);

  vgraph = Get_CFG_vgraph(cfg);

  RegisterSccInLoops(Vgraph_first_scc(vgraph));

  var_id_tbl = STRING_new_symbol_table ("var_id_tbl", 512);
  /* 
   * If a loop has more than 1 exit node or more than 1 back edge, 
   * just conservatively set the loop trip count to unknown.
   */
  for (loop = cfg->lp; loop; loop = loop->next)
    if ((Get_Loop_num_exit(loop) != 1) ||
        (Get_Loop_num_back_edge(loop) != 1))
      Set_Loop_tripcount(loop, NewUnknownValue());
  /*
   * calculate the value of each node 
   */ 
  for (loop = cfg->lp_tree; loop; loop = Get_Loop_sibling(loop))
    FindValuesInLoop(loop);

  FindValuesInLoop(&dummy_loop);

  FreeSccInLoops(cfg->lp);
  FreeSccInLoop(&dummy_loop);
}

PC_Loop 
P_SS_VnodeOuterMostEnclosingParloop (P_Vnode vnode)
{
  PC_Loop loop;
  PC_Loop parloop;

  parloop = NULL;
  loop = Vnode_loop(vnode);
  while (loop) {
    #if 0 /* GCD */
    if (Loop_type(loop) == LP_PARLOOP)
    #endif
    if (Get_Loop_iv(loop))
      parloop = loop;
    loop = Get_Loop_parent(loop);
  } 
  return parloop;
}

STRING_Symbol *
P_SS_FindStringSymbol(char *name)
{
  return STRING_find_symbol(var_id_tbl, name);
}

void
P_SS_DeleteVarIdTable()
{
   STRING_delete_symbol_table(var_id_tbl, FreeAlphaVarId);
}

/**********************************************************************************
        Static function body
***********************************************************************************/

static void
FreeUnknownValue(P_Value value)
{
  /* do nothing */
}

static void
FreeAlphaVarId(void *v)
{
  Alpha_var_id vid;

  vid = v;
  free(vid->name); 
  DISPOSE(vid);
}

static void
FindValuesInLoop(PC_Loop loop)
{
  PC_Loop child;
  P_Scc scc;
  Lptr scc_ptr;
  
  NewInductionVarId(loop);
  for (child = Get_Loop_child(loop); 
       child;
       child = Get_Loop_sibling(child))
    FindValuesInLoop(child);
  for (scc_ptr = Get_Loop_scc_list(loop);
       scc_ptr;
       scc_ptr = LPTR_next(scc_ptr)) {
    scc = LPTR_ptr(scc_ptr);
    FindValuesInScc(scc);
  }
}

static void
FindValuesInScc(P_Scc scc)
{
  Lptr comp;
  Expr curr_expr;
  P_Vnode curr_vnode = NULL;
  P_Vnode phi_vnode = NULL;
  P_Vnode op1_vnode = NULL;
  P_Vnode op2_vnode = NULL;
  P_Value init_value = NULL;
  P_Value incr_value = NULL;		
  P_Value acc_incr_value = NULL;
  PC_Loop curr_loop = NULL;

  if (Scc_flag(scc) == 1) 
    return;
  assert (Scc_flag(scc) == 0);
  Scc_flag(scc) = 1;
  if (Scc_num_comp(scc) > 1) 
    {
      phi_vnode = NULL;
      init_value = NewUnknownValue();
      acc_incr_value = NewConstValue(NewIntExpr(0));
      /*
       * check whether this scc is corresponding to an induction
       * variable, meanwhile, getting the initial value and the
       * increment value.  
       */
      for (comp = Scc_comp(scc); 
	   !Scc_end_of_comp(comp) && Value_is_known(acc_incr_value); 
	   comp = Scc_next_comp(comp)) 
	{
	  curr_vnode = Scc_comp_vnode(comp);
	  curr_expr = Vnode_expr(curr_vnode); 
	  
	  assert(Get_ExprExtForVgraph_value(curr_expr) == NULL);

	  curr_loop = Vnode_loop(curr_vnode);
	  switch (P_GetExprOpcode(curr_expr)) 
	    {
	    case OP_cast:
	    case OP_var: 
	      continue;
	    case OP_add:
	    case OP_sub: 
	      {
		assert (Vnode_num_succ(curr_vnode) == 2);
		op1_vnode = Vnode_succ1_vnode(curr_vnode);
		assert(op1_vnode != NULL);
		op2_vnode = Vnode_succ2_vnode(curr_vnode);
		assert(op2_vnode != NULL);
		if ((Vnode_scc_id(op1_vnode) == Vnode_scc_id(curr_vnode)) &&
		    (Vnode_scc_id(op2_vnode) != Vnode_scc_id(curr_vnode))) 
		  {
		    /*
		     * x +/- incr
		     */
		    incr_value = VnodeClosedValue(op2_vnode, curr_loop);
		  } 
		else if ((Vnode_scc_id(op1_vnode) != 
			  Vnode_scc_id(curr_vnode)) &&
			 (Vnode_scc_id(op2_vnode) == 
			  Vnode_scc_id(curr_vnode)) &&
			 (P_GetExprOpcode(curr_expr) != OP_sub)) 
		  {
		    /*
		     * incr + x
		     */
		    incr_value = VnodeClosedValue(op1_vnode, curr_loop);
		  } 
		else 
		  {
		    P_SS_FreeValue(incr_value);
		    incr_value = NewUnknownValue();
		    break;
		  }
		/*
		 * for now, the increment must be known constant integer.
		 */
		if (Value_is_known(incr_value) && 
		    Value_is_integer(incr_value)) 
		  {
		    /*
		     * calculate new accumulated increment value
		     */
		    if (P_GetExprOpcode(curr_expr) == OP_add)
		      P_GetExprScalar(Value_const(acc_incr_value)) += 
			Value_integer(incr_value);
		    else
		      P_GetExprScalar(Value_const(acc_incr_value)) -= 
			Value_integer(incr_value);
		  } 
		else 
		  {
		    P_SS_FreeValue(acc_incr_value);
		    acc_incr_value = NewUnknownValue();
		  } 
		break;
	      }
	    case OP_phi: 
	      {
		if (phi_vnode) 
		  { /* there are more than one phi node in the Scc */
		    P_SS_FreeValue(init_value);
		    P_SS_FreeValue(acc_incr_value);
		    init_value = NewUnknownValue();
		    acc_incr_value = NewUnknownValue();
		    break;
		  }
		phi_vnode = curr_vnode;
		/*
		 * two src operands: one points to an expr in the same
		 * loop as OP_phi.  the other points to outside the
		 * loop.  */

		if (Vnode_num_succ(curr_vnode) < 2)
		  {
		    P_warn ("P_SS_induct.c:FindValuesInScc() "
			    "num_succ < 2 (JWS)");
		    P_SS_FreeValue(init_value);
		    P_SS_FreeValue(acc_incr_value);
		    init_value = NewUnknownValue();
		    acc_incr_value = NewUnknownValue();
		    break;
		  }

		op1_vnode = Vnode_succ1_vnode(curr_vnode);
		op2_vnode = Vnode_succ2_vnode(curr_vnode);
		if ((Vnode_scc_id(op1_vnode) != Vnode_scc_id(curr_vnode)) && 
		    (Vnode_scc_id(op2_vnode) == Vnode_scc_id(curr_vnode)))
		  {
		    init_value = VnodeClosedValue(op1_vnode, curr_loop);
		  }
		else if ((Vnode_scc_id(op1_vnode) == Vnode_scc_id(curr_vnode)) && 
			 (Vnode_scc_id(op2_vnode) != Vnode_scc_id(curr_vnode)))
		  {
		    init_value = VnodeClosedValue(op2_vnode, curr_loop);
		  }
		else 
		  {
		    P_SS_FreeValue(init_value);
		    P_SS_FreeValue(acc_incr_value);
		    init_value = NewUnknownValue();
		    acc_incr_value = NewUnknownValue();
		  }
		break;
	      }
	    default: 
	      {
		P_SS_FreeValue(init_value);
		P_SS_FreeValue(acc_incr_value);
		init_value = NewUnknownValue();
		acc_incr_value = NewUnknownValue();
		break;
	      }
	    }
	}
      P_SS_FreeValue(incr_value);
      /*
       * calculate the value of each node in Scc
       */
      if (Value_is_known(init_value) &&        	/* known initial value */ 
	  Value_is_known(acc_incr_value) && 
	  Value_is_integer(acc_incr_value) && 	/* non-zero increment value */
	  (Value_integer(acc_incr_value) != 0) )
	{
	  /*
	   * An induction variable found.
	   * For each induction expression, starting from the phi vnode, 
	   * set the tuples of <init_value, incr_value>,
	   * which will be used later for building Omega test.
	   */
	  Alpha_var_id new_iv;
	  int step;
	  Expr step_expr;
	  P_Value value;

	  /*
	   * create a new indcution variable, if not exist.
	   */ 
	  curr_loop = Vnode_loop(phi_vnode);
	  new_iv = NewInductionVarId(curr_loop); 
	  /*
	   * for each node in the SCC, starting from the phi_node,
	   * calculate their value */
	  step = Value_integer(acc_incr_value);
	  step_expr = NewIntExpr(step);
	  value = NewInductValue(new_iv, init_value, step_expr); 

	  Set_ExprExtForVgraph_value(Vnode_expr(phi_vnode), value);

	  for (curr_vnode = Vnode_scc_prev(phi_vnode); 
	       curr_vnode != phi_vnode; 
	       curr_vnode = Vnode_scc_prev(curr_vnode))
	    VnodeFindValue(curr_vnode);

	  P_SS_FreeValue(acc_incr_value);
	} 
      else 
	{
	  /*
	   * each Vnode in scc is unknown.
	   */
	  for (comp = Scc_comp(scc); !Scc_end_of_comp(comp); 
	       comp = Scc_next_comp(comp)) 
	    {
	      curr_vnode = Scc_comp_vnode(comp);
	      curr_expr = Vnode_expr(curr_vnode);
	      Set_ExprExtForVgraph_value(curr_expr, NewUnknownValue());
	    }
	  P_SS_FreeValue(init_value);
	  P_SS_FreeValue(acc_incr_value);
	} 
    } 
  else 
    { /* Scc_num_comp(scc) == 1 */
      comp = Scc_comp(scc); 
      curr_vnode = LPTR_ptr(comp);  
      VnodeFindValue(curr_vnode);
      SetLoopInformation(curr_vnode);
    }

  return;
}

static void
DumpVnodeExpr (FILE *out_file, Expr expr)
{
   Expr opnd;

   if (expr == NULL) {
     fprintf(out_file, "{ NULL } !!!");
   } else {
      switch (P_GetExprOpcode(expr)) {
      case OP_var:
         fprintf(out_file, "{ %s }", P_GetExprVarName(expr));
         break;
      case OP_enum:
         P_punt("Build_Expr_DAG: OP_enum");
      case OP_error:
         P_punt("Build_Expr_DAG: OP_error");
      case OP_expr_size:
         P_punt("Build_Expr_DAG: OP_expr_size");
      case OP_type_size:
         P_punt("Build_Expr_DAG: OP_type_size");
      case OP_int:
         if (PSI_IsUnsignedTypeExpr(expr))
            fprintf(out_file, "{" ITuintmaxformat "}", P_GetExprUScalar(expr)); 
         else
            fprintf(out_file, "{" ITintmaxformat "}", P_GetExprScalar(expr));
         break;
      case OP_real:
      case OP_float:
      case OP_double:
         fprintf(out_file, "{ %lf }", P_GetExprReal(expr));
         break;
      case OP_char:
         P_punt("DumpVnodeExpr: OP_char");
      case OP_string:
         fprintf(out_file, "{ %s }", P_GetExprString(expr));
         break;
      case OP_dot:    /* P_GetExprOperands(expr)} {"."} {expr->value.string} */
         fprintf(out_file, "{ . }");
         break;
      case OP_arrow:  /* {P_GetExprOperands(expr)} {"->"} {expr->value.string} */
         fprintf(out_file, "{ -> }");
         break;
      case OP_cast:   /* {"("} {expr->type} {")"} {P_GetExprOperands(expr)} */
         fprintf(out_file, "{ (cast) }");
         break;
      case OP_neg:    /* {"-"} {P_GetExprOperands(expr)} */
         fprintf(out_file, "{ - }");
         break;
      case OP_not:    /* {"!"} {P_GetExprOperands(expr)} */
         fprintf(out_file, "{ ! }");
         break;
      case OP_inv:    /* {"~"} {P_GetExprOperands(expr)} */
         fprintf(out_file, "{ ~ }");
         break;
      case OP_indr:   /* {"*"} {P_GetExprOperands(expr)} */
         fprintf(out_file, "{ (*) }");
         break;
      case OP_addr:   /* {"&"} {P_GetExprOperands(expr)} */
         fprintf(out_file, "{ (&) }");
         break;
      case OP_preinc:
         P_punt("Build_Expr_DAG: OP_preinc");
      case OP_predec:
         P_punt("Build_Expr_DAG: OP_predec");
      case OP_postinc:
         P_punt("Build_Expr_DAG: OP_postinc");
      case OP_postdec:
         P_punt("Build_Expr_DAG: OP_postdec");
      case OP_quest:
         P_punt("Build_Expr_DAG: OP_quest");
      case OP_compexpr: /* {"("} {P_GetExprOperands(expr)} (maybe another OP_compexpr)
                           {","} {P_GetExprOperands(expr)->next} (maybe empty) {")"} */
         fprintf(out_file, "{ , }");
         break;
      case OP_index: /* {P_GetExprOperands(expr)} {"["} {P_GetExprSibling(P_GetExprOperands(expr))} {"]"} */
         fprintf(out_file, "{ [] }");
         break;
      case OP_disj:  /* {P_GetExprOperands(expr)} {"||"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ || }");
         break;
      case OP_conj:  /* {P_GetExprOperands(expr)} {"&&"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ && }");
         break;
      case OP_or:    /* {P_GetExprOperands(expr)} {"|"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ | }");
         break;
      case OP_xor:   /* {P_GetExprOperands(expr)} {"^"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ ^ }");
         break;
      case OP_and:   /* {P_GetExprOperands(expr)} {"&"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ & }");
         break;
      case OP_eq:    /* {P_GetExprOperands(expr)} {"=="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ == }");
         break;
      case OP_ne:    /* {P_GetExprOperands(expr)} {"!="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ != }");
         break;
      case OP_lt:    /* {P_GetExprOperands(expr)} {"<"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ < }");
         break;
      case OP_le:    /* {P_GetExprOperands(expr)} {"<="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ <= }");
         break;
      case OP_ge:    /* {P_GetExprOperands(expr)} {">="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ >= }");
         break;
      case OP_gt:    /* {P_GetExprOperands(expr)} {">"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ > }");
         break;
      case OP_rshft: /* {P_GetExprOperands(expr)} {">>"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ >> }");
         break;
      case OP_lshft: /* {P_GetExprOperands(expr)} {"<<"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ << }");
         break;
      case OP_add:   /* {P_GetExprOperands(expr)} {"+"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ + }");
         break;
      case OP_sub:   /* {P_GetExprOperands(expr)} {"-"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ - }");
         break;
      case OP_mul:   /* {P_GetExprOperands(expr)} {"*"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ * }");
         break;
      case OP_div:   /* {P_GetExprOperands(expr)} {"/"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ / }");
         break;
      case OP_mod:   /* {P_GetExprOperands(expr)} {"%"} {P_GetExprSibling(P_GetExprOperands(expr))} */
         fprintf(out_file, "{ %% }");
         break;
      case OP_call:  /* {P_GetExprOperands(expr)} {"("}
                        {P_GetExprSibling(P_GetExprOperands(expr))} {","}
                        {P_GetExprSibling(P_GetExprOperands(expr))->next} {","}
                        {P_GetExprSibling(P_GetExprOperands(expr))->next->next} ... {")"} */
         fprintf(out_file, "{ call }");
         break;
      case OP_assign: /* {P_GetExprOperands(expr)} {"="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_assign");
      case OP_Aadd:   /* {P_GetExprOperands(expr)} {"+="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Aadd");
      case OP_Asub:   /* {P_GetExprOperands(expr)} {"-="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Asub");
      case OP_Amul:   /* {P_GetExprOperands(expr)} {"*="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Amul");
      case OP_Adiv:   /* {P_GetExprOperands(expr)} {"/="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Adiv");
      case OP_Amod:   /* {P_GetExprOperands(expr)} {"%="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Arshft");
      case OP_Alshft: /* {P_GetExprOperands(expr)} {"<<="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Alshft");
      case OP_Aand:   /* {P_GetExprOperands(expr)} {"&="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Aand");
      case OP_Aor:    /* {P_GetExprOperands(expr)} {"|="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Aor");
      case OP_Axor:   /* {P_GetExprOperands(expr)} {"^="} {P_GetExprSibling(P_GetExprOperands(expr))} */
         P_punt("Build_Expr_DAG: OP_Axor");
      case OP_phi: /* expr->oprands PHI(P_GetExprSibling(P_GetExprOperands(expr)), P_GetExprSibling(P_GetExprOperands(expr))->sibling ... ) */
         fprintf(out_file, "{ %s = PHI ( ", P_GetExprVarName(P_GetExprOperands(expr)));
         opnd = P_GetExprSibling(P_GetExprOperands(expr));
         while (opnd) {
           assert (P_GetExprOpcode(opnd) == OP_int);
           fprintf(out_file, "[%d] ", (int) P_GetExprScalar(opnd));
           opnd = P_GetExprSibling(opnd);
         }
         fprintf(out_file, " )");
         break;
      default:
         P_punt("Dump_DAG_Node: unknown opcode");
      }
   }
}

static void
DumpIndent(FILE *out_file, int level, char *str)
{
  int i;

  for (i = 0 ; i < level ; i++)
    fprintf(out_file, "%s", str);
}

static void
DumpVnode (FILE *out_file, P_Vnode node, DumpVnodeFormat dump_format, int level)
{
   Lptr succ_ptr;
   P_Vnode succ_vnode;
   P_Value value; 

   DumpIndent(out_file, level, INDENT_STRING);
   fprintf(out_file, "Vnode <%d> in SCC[%d], BB[%d]:", 
                      Vnode_id(node), 
                      Vnode_scc(node)?Vnode_scc_id(node):-1,
                      Vnode_bb(node)?(Vnode_bb(node)->ID):-1);
   fprintf(out_file, "\n");

   DumpIndent(out_file, level+1, INDENT_STRING);
#if LP64_ARCHITECTURE
   fprintf(out_file, "Expr (%d:0x%lx) = ", Vnode_expr (node) -> id,
	   (long) Vnode_expr(node));
#else
   fprintf(out_file, "Expr (%d:0x%x) = ", Vnode_expr (node) -> id,
	   (int) Vnode_expr(node));
#endif
   DumpVnodeExpr(out_file, Vnode_expr(node));
   fprintf(out_file, "\n");

   DumpIndent(out_file, level+1, INDENT_STRING);
   fprintf(out_file, "Value = ");
   value = Get_ExprExtForVgraph_value(Vnode_expr(node));
   if (value == NULL)
     fprintf(out_file, "NULL");
   else
     P_SS_DumpValue(out_file, value);
   fprintf(out_file, "\n");

   DumpIndent(out_file, level+1, INDENT_STRING);
   fprintf(out_file, "num_succ = %d\n", Vnode_num_succ(node));

   DumpIndent(out_file, level+1, INDENT_STRING);
   fprintf(out_file, "succ ->");
   succ_ptr = Vnode_first_succ(node);
   while (succ_ptr) {
     succ_vnode = LPTR_ptr(succ_ptr);
     if (!succ_vnode)
       P_punt("Dump_DAG_Node: succ_node is null");
     if (dump_format & DUMP_SUCC_VNODE) {
       fprintf(out_file, "\n");
       DumpVnode(out_file, succ_vnode, dump_format & (~DUMP_SUCC_VNODE), level+1);
     } else {
       fprintf(out_file, "<%d> ", Vnode_id(succ_vnode));
     }
     succ_ptr = LPTR_next(succ_ptr);
   }
   fprintf(out_file, "\n");

   DumpIndent(out_file, level+1, INDENT_STRING);
   fprintf(out_file, "scc_prev = ");
   if (Vnode_scc_prev(node))
     fprintf(out_file, "Vnode <%d>\n", Vnode_id(Vnode_scc_prev(node)));
   else
     fprintf(out_file, "-\n");
}

static P_Vgraph 
NewVgraph (PC_Graph cfg)
{
  P_Vgraph vgraph;

  vgraph = ALLOCATE (struct _P_Vgraph);

  Vgraph_cfg(vgraph) = cfg;

  Vgraph_first_vnode(vgraph) = NULL;
  Vgraph_last_vnode(vgraph) = NULL;
  Vgraph_num_vnode(vgraph) = 0;
  Vgraph_first_scc(vgraph) = NULL;
  Vgraph_num_scc(vgraph) = 0;
  return vgraph;
}

static P_Vnode 
NewVnode(P_Vgraph vgraph)
{
  P_Vnode v;

  v = ALLOCATE (struct _P_Vnode);
  Vnode_id(v) = Vgraph_num_vnode(vgraph)++;
  Vnode_vgraph(v) = vgraph;
  Vnode_scc(v) = NULL;
  Vnode_status(v) = -1;
  Vnode_num_succ(v) = 0;
  Vnode_first_succ(v) = NULL;
  Vnode_scc_prev(v) = NULL;
  Vnode_expr(v) = NULL;
  Vnode_bb(v) = NULL;
  Vnode_next(v) = NULL;
  return v;
}

static void
VnodeAppendNewSuccVnode(P_Vnode vnode, P_Vnode succ_vnode)
{
  Vnode_first_succ(vnode) = AppendLptr(Vnode_first_succ(vnode), NewLptr(succ_vnode));
  Vnode_num_succ(vnode)++;
}

/*
 * change this to hash table with expr as key.
 * if there is an vnode for expr, return that vnode.
 * if not, allocate a new vnode and add it to the hash table.
 */
static P_Vnode 
FindAndAllocateVnode(P_Vgraph vgraph, Expr expr, PC_Block bb)
{
  P_Vnode vnode;

  vnode = Get_ExprExtForVgraph_vnode(expr);
  if (vnode) { 
    assert (Vnode_expr(vnode) == expr); 
    if (Vnode_bb(vnode))
      assert(Vnode_bb(vnode) == bb);
    else
      Vnode_bb(vnode) = bb;
  } else {
    vnode = NewVnode(vgraph);
    Set_ExprExtForVgraph_vnode(expr, vnode);
    if (Vgraph_first_vnode(vgraph) == NULL) 
      Vgraph_first_vnode(vgraph) = Vgraph_last_vnode(vgraph) = vnode;
    else 
      Vgraph_last_vnode(vgraph) = Vnode_next(Vgraph_last_vnode(vgraph)) = vnode;
    Vnode_expr(vnode) = expr;
    Vnode_bb(vnode) = bb;
  }
  return vnode;
}

/*
 * for lhs op rhs, the input loop condition is (lhs-rhs) op 0, where
 * (lhs-rhs) is the value of the input vnode.
 * loop condition will be adjusted and normalized to the form of cond >= 0, so 
 * 
 *	cond >= 0	---> 	cond - cond.incr >= 0, 		assert(cond.incr < 0)
 *	cond > 0   	--->	cond - cond.incr - 1 >= 0, 	assert(cond.incr < 0)
 *	cond <= 0	---> 	-cond + cond.incr >= 0, 	assert(cond.incr > 0)
 *	cond < 0	--->	-cond + cond.incr - 1 >= 0,	assert(cond.incr > 0)
 *	cond == 0	--->	cond - cond.incr >= 0,		if cond.incr < 0
 *			--->	-cond + cond.incr >= 0,		if cond.incr > 0	
 *			--->	unknown, otherwise
 *	cond != 0	---> 	eqv. to cond > 0,		if cond.incr < 0
 *			--->	eqv. to cond < 0,		if cond.incr > 0
 *			--->	unknwon, otherwise
 */
static P_Value
NormalizedLoopCondition (P_Value value, Expr expr, int reverse_condition)
{
  P_Value negated_value; /* value_negated */
  P_Value offset_value;	/* value_one */
  P_Value normalized_value = NULL;
  Expr incr_expr;
  int incr;

  switch (P_GetExprOpcode(expr)) {
    case OP_ge: /* cond >= 0 ---> cond - cond.incr >= 0, assert(cond.incr < 0) */
    case_OP_ge:
      if (reverse_condition) {
        reverse_condition = 0;
        goto case_OP_lt;
      }

      incr = P_GetExprScalar(Value_induct_incr(value));
      assert (incr < 0);
      offset_value = NewIntValue(incr);

      normalized_value = ValueComputeWithTwoOperands(OP_sub, value, offset_value);

      P_SS_FreeValue(offset_value);
      break;

    case OP_gt: /* cond > 0 ---> cond - cond.incr - 1 >= 0, assert(cond.incr < 0) */
    case_OP_gt:  
      if (reverse_condition) {
        reverse_condition = 0;
        goto case_OP_le;
      }

      incr = P_GetExprScalar(Value_induct_incr(value));
      assert (incr < 0);
      offset_value = NewIntValue(1+incr);

      normalized_value = ValueComputeWithTwoOperands(OP_sub, value, offset_value);

      P_SS_FreeValue(offset_value);
      break;

    case OP_le: /* cond <= 0 ---> -cond + cond.incr >= 0, assert(cond.incr > 0) */
    case_OP_le:
      if (reverse_condition) {
        reverse_condition = 0;
        goto case_OP_gt;
      }

      incr = P_GetExprScalar(Value_induct_incr(value));
      assert (incr > 0);
      offset_value = NewIntValue(incr);
      negated_value = ValueComputeWithOneOperand(OP_neg, value);

      normalized_value = ValueComputeWithTwoOperands(OP_add, negated_value, offset_value); 

      P_SS_FreeValue(offset_value);
      P_SS_FreeValue(negated_value);
      break;

    case OP_lt: /* cond < 0 ---> -cond + cond.incr - 1 >= 0, assert(cond.incr > 0) */
    case_OP_lt:
      if (reverse_condition) {
        reverse_condition = 0;
        goto case_OP_ge;
      }

      incr = P_GetExprScalar(Value_induct_incr(value));
      assert (incr > 0);
      offset_value = NewIntValue(incr-1);
      negated_value = ValueComputeWithOneOperand(OP_neg, value);

      normalized_value = ValueComputeWithTwoOperands(OP_add, negated_value, offset_value); 

      P_SS_FreeValue(offset_value);
      P_SS_FreeValue(negated_value);
      break;

    case OP_eq:
    case_OP_eq:
      if (reverse_condition) {
        reverse_condition = 0;
        goto case_OP_ne;
      }

      if (Value_is_const(value)) 
        return NewUnknownValue();
      
      incr_expr = Value_induct_incr(value);
      if (P_GetExprOpcode(incr_expr) != OP_int) 
        return NewUnknownValue();

      incr = P_GetExprScalar(incr_expr);
      assert (incr != 0);
      if (incr > 0)
        goto case_OP_le;
      goto case_OP_ge;

    case OP_ne:
    case_OP_ne:
      if (reverse_condition) {
        reverse_condition = 0;
        goto case_OP_eq;
      }
 
      if (Value_is_const(value)) 
        return NewUnknownValue();

      incr_expr = Value_induct_incr(value);
      if (P_GetExprOpcode(incr_expr) != OP_int) 
        return NewUnknownValue();

      incr = P_GetExprScalar(incr_expr);
      assert (incr != 0);
      if (incr > 0) 
        goto case_OP_lt;
      goto case_OP_gt;

    default:
      P_punt ("NormalizedLoopCondition: unexpected opcode");
  }
  return normalized_value;
}

/*
 * trip_count = loop_condition.init/abs(loop_condition.incr) + 1
 */
static P_Value
ComputeLoopTripCount (P_Value loop_condition, int plus1)
{
  P_Value value_one;
  P_Value init_value;
  P_Value final_value;  
  Expr incr_expr;
  int incr;
  int init;

  if (Value_is_unknown(loop_condition))
    return NewUnknownValue();
  if (Value_is_const(loop_condition))
    return NewUnknownValue();
  incr_expr = Value_induct_incr(loop_condition);
  if (P_GetExprOpcode(incr_expr) != OP_int) /* too complicated/lazy */
    return NewUnknownValue();
  incr = P_GetExprScalar(incr_expr);
  if (incr >= 0) {
    fprintf(stderr, "\n WARNING: ComputeLoopTripCount: incr >= 0, maybe infinite loop.\n");
    return NewUnknownValue();
  } 
  init_value = Value_induct_init(loop_condition); 
  if (incr == -1)
    {
      if (plus1) 
        {
          value_one = NewIntValue(1);
          final_value = ValueComputeWithTwoOperands(OP_add, init_value, value_one);
          P_SS_FreeValue(value_one);
          return final_value;
        }
      else
          return init_value;
    }
  if (!Value_is_integer(init_value)) /* too complicated/lazy */
    return NewUnknownValue();
  init = Value_integer(init_value);
  if (init <= 0)
    {
      fprintf(stderr, "\n WARNING: ComputeLoopTripCount: init <= 0, maybe infinite loop.\n");
      return NewUnknownValue();
    }
  if (plus1)
    final_value = NewIntValue(init/(-incr)+1);
  else
    final_value = NewIntValue(init/(-incr));
  return final_value;
}

/*
 * set two things: 
 *   - iteration count for calculating the final values
 *   - loop condition for the upper bound of loop index in Omega test.
 *
 * to set them correctly we need to consider exit blocks and back edges.
 *   - the derivation of loop trip count is more conservative.
 *     if there is more than 1 loop exit block or more than 1 back edge, 
 *     the loop trip count is conservatively to unknown in P_SS_FindValuesInCFG().
 *   - the derivation of the loop condition 
 */
static void 
SetLoopInformation (P_Vnode vnode)
{
  int reverse_condition;
  PC_Flow link;
  PC_Block bb;
  PC_Block succ_bb = NULL;
  Expr expr;
  PC_Loop loop;
  P_Value cond_value;
  P_Value new_condition;
  P_Value old_condition;

  loop = Vnode_loop(vnode);
  expr = Vnode_expr(vnode);
  bb = Vnode_bb(vnode);
  /*
   * check whether curr_vnode is at an exit block of a loop
   */
  if (loop &&
      !Value_is_unknown(Get_Loop_condition(loop)) &&
      (P_GetExprParentExpr(expr) == NULL) &&
      ((P_GetExprOpcode(expr) == OP_ge) ||
       (P_GetExprOpcode(expr) == OP_gt) ||
       (P_GetExprOpcode(expr) == OP_le) ||
       (P_GetExprOpcode(expr) == OP_lt) ||
       (P_GetExprOpcode(expr) == OP_ne) ||
       (P_GetExprOpcode(expr) == OP_eq)) &&
      IsExitBlock(bb, loop))
    { 
      for (link = bb->s_flow; link; link = link->s_next_flow)
	{
	  succ_bb = link->dest_bb;

          assert (link->flow_cond != NULL);
          assert (P_GetExprOpcode(link->flow_cond) == OP_int);
          if (P_GetExprScalar(link->flow_cond) == 1)
            break;
        }
      if (Get_BlockExtForVgraph_loop(succ_bb) != Get_BlockExtForVgraph_loop(bb))
        reverse_condition = 1;
      else
        reverse_condition = 0; 

      cond_value = Get_ExprExtForVgraph_value(expr);

      if (Value_is_unknown(cond_value) || Value_is_const(cond_value)) 
        {
          Set_Loop_condition(loop, NewUnknownValue());
          Set_Loop_tripcount(loop, NewUnknownValue());
	  return;
        }

      new_condition = NormalizedLoopCondition (cond_value, expr, reverse_condition);

      if (Value_is_unknown(new_condition)) 
        { 
          Set_Loop_condition(loop, NewUnknownValue());
          Set_Loop_tripcount(loop, NewUnknownValue());
        } 
      else 
        {
          assert (Value_is_induct(new_condition));
          old_condition = Get_Loop_condition(loop);
          if (old_condition && !ValueEqual(old_condition, new_condition)) 
            {
              P_SS_FreeValue(Get_Loop_condition(loop));
              P_SS_FreeValue(Get_Loop_tripcount(loop));
              Set_Loop_condition(loop, NewUnknownValue());
              Set_Loop_tripcount(loop, NewUnknownValue());
            } 
          else 
            {
              P_Value tripcount;
              /*
               * !old_condition || ValueEqual(old_condition, new_condition)
               */
              if (!old_condition)      
                Set_Loop_condition(loop, new_condition);
            
              if (bb->ID == loop->head)
                 tripcount = ComputeLoopTripCount (new_condition, 0);
              else 
                 tripcount = ComputeLoopTripCount (new_condition, 1);
              Set_Loop_tripcount(loop, tripcount);
            }
        }
    }
}

static Expr
UncastedExpr(Expr expr)
{
  Expr opnd;

  opnd = expr;
  while (opnd && (P_GetExprOpcode(opnd) == OP_cast))
    opnd = P_GetExprOperands(opnd);
  return opnd;
}

static P_Vnode 
BuildExprVgraph(P_Vgraph vgraph, Expr expr, PC_Block bb)
{
  P_Vnode rhs_vnode;
  P_Vnode lhs_vnode;
  P_Vnode op_vnode;
  P_Vnode succ_vnode;
  Expr lhs_expr;
  Expr ssa_expr;
  Expr succ_expr = NULL;
  Expr opnd;

  if (expr == NULL) {
    #if DEBUG_P_SS_BuildVgraph
    fprintf(stderr, "Warning: BuildExprVgraph, null expr, bb[%d]\n", P_SS_GetBasicBlockBBId(bb));
    #endif
    return NULL;
  }
  if (P_GetExprOpcode(expr) == OP_null) {
    #if DEBUG_P_SS_BuildVgraph
    fprintf(stderr, "Warning: BuildExprVgraph, OP_null, bb[%d]\n", P_SS_GetBasicBlockBBId(bb));
    #endif
    return NULL;
  }
  if (P_GetExprOpcode(expr) == OP_assign) {
    /* 
     * Build RHS Vgraph
     */
    #if 0
    if (IsShadowAssign(expr)) 
      {
      char name[1024];
      Key key;
      VarDcl var;
      Expr rhs_expr;

      key = P_GetExprVarKey(P_GetExprOperands(expr));
      var = PSI_GetVarDclEntry(key);
      assert (P_GetVarDclQualifier(var) & VQ_PARAMETER);
      rhs_expr = P_NewExprWithOpcode(OP_var);
      sprintf(name, "@%s", P_GetVarDclName(var));
      P_SetExprVarName(rhs_expr, strdup(name));
      P_SetExprVarKey(rhs_expr, key);
      P_SetExprType(rhs_expr, P_GetVarDclType(var));
      P_SetExprPragma (rhs_expr, P_NewPragmaWithSpecExpr("ParamValue", NULL));
      rhs_vnode = BuildExprVgraph(vgraph, rhs_expr, bb);
      } 
    else 
    #endif
      {
      rhs_vnode = BuildExprVgraph(vgraph, P_GetExprSibling(P_GetExprOperands(expr)), bb);
      }
    lhs_expr = UncastedExpr(P_GetExprOperands(expr));
    if (P_GetExprOpcode(lhs_expr) == OP_var) {
      /*
       * connect lhs to rhs 
       */ 
      lhs_vnode = FindAndAllocateVnode(vgraph, lhs_expr, bb);
      VnodeAppendNewSuccVnode(lhs_vnode, rhs_vnode);
      #if DEBUG_P_SS_BuildVgraph
        fprintf(stderr, "\n>>> BuildExprVgraph add Vnode <<<\n");
        DumpVnode(stderr, lhs_vnode, DUMP_SUCC_VNODE, 0);
      #endif
    } else {
      /* 
       * for now, we don't connect lhs to rhs if lhs is not a scalar.
       */
      BuildExprVgraph(vgraph, P_GetExprOperands(expr), bb);
    }
    /*
     * for assignments of the form x = y = rhs,
     * build vraph of x->rhs, y->rhs, instead of x->y, y->rhs.
     */
    return rhs_vnode;
  }
  /*
   * handle non-assignment expression
   */
  op_vnode = FindAndAllocateVnode(vgraph, expr, bb); 
  switch (P_GetExprOpcode(expr)) {
    case OP_var:
      ssa_expr = P_CF_GetSSA(expr);
      if (ssa_expr) {
        assert((P_GetExprOpcode(ssa_expr) == OP_assign) || (P_GetExprOpcode(ssa_expr) == OP_phi));
        if (P_GetExprOpcode(ssa_expr) == OP_assign)
          succ_expr = P_GetExprOperands(ssa_expr);
        else if (P_GetExprOpcode(ssa_expr) == OP_phi)
          succ_expr = ssa_expr;
        else
          P_punt("BuildExprVgraph: ssa_expr is not OP_assign nor OP_phi.\n");

        succ_vnode = FindAndAllocateVnode(vgraph, succ_expr, Get_ExprExtForVgraph_bb(succ_expr));
        VnodeAppendNewSuccVnode(op_vnode, succ_vnode);
      } else {
        Key key;
        VarDcl var;
        Type type;
        key = P_GetExprVarKey(expr);
        var = PSI_GetVarDclEntry(key);
        if (var) 
          {
            type = P_GetVarDclType(var);
            if (!P_TstVarDclQualifier(var, VQ_GLOBAL) && !PSI_IsArrayType(type) && !strstr(P_GetExprVarName(expr), "@"))
              fprintf(stderr, "!! BuildExprVgraph: Expr (%d) \"%s\" has no SSA\n", P_GetExprID(expr), P_GetExprVarName(expr));
          } 
        #if 0
        else 
          {
            SymbolTable st;
            st = PSI_GetTable(); 
            fprintf(stderr, "!! BuildExprVgraph: Expr (%d) \"%s\" not found in Symbol Table (%s)\n", 
                            P_GetExprID(expr), P_GetExprVarName(expr), P_GetSymbolTableIPTableName(st));
          }
        #endif
      }
      break;
    case OP_enum:
      P_punt("BuildExprVgraph: unexpected opcode OP_enum");
    case OP_error:
      P_punt("BuildExprVgraph: unexpected opcode OP_error");
    case OP_expr_size:
      P_punt("BuildExprVgraph: unexpected opcode OP_expr_size");
    case OP_type_size:
      P_punt("BuildExprVgraph: unexpected opcode OP_type_size");
    case OP_int:
    case OP_real:
    case OP_float:
    case OP_double:
      break;
    case OP_char:
      P_punt("BuildExprVgraph: OP_char");
    case OP_string:
      break;
    case OP_dot:    /* {P_GetExprOperands(expr)} {"."} {expr->value.string} */
    case OP_arrow:  /* {P_GetExprOperands(expr)} {"->"} {expr->value.string} */
    case OP_indr:   /* {"*"} {P_GetExprOperands(expr)} */
      succ_vnode = BuildExprVgraph(vgraph, P_GetExprOperands(expr), bb);
      break;
    case OP_cast:   /* {"("} {expr->type} {")"} {P_GetExprOperands(expr)} */
    case OP_neg:    /* {"-"} {P_GetExprOperands(expr)} */
    case OP_not:    /* {"!"} {P_GetExprOperands(expr)} */
    case OP_inv:    /* {"~"} {P_GetExprOperands(expr)} */
    case OP_addr:   /* {"&"} {P_GetExprOperands(expr)} */
      succ_vnode = BuildExprVgraph(vgraph, P_GetExprOperands(expr), bb);
      VnodeAppendNewSuccVnode(op_vnode, succ_vnode);
      break;
    case OP_preinc:
      P_punt("Build_Expr_DAG: OP_preinc");
    case OP_predec:
      P_punt("Build_Expr_DAG: OP_predec");
    case OP_postinc:
      P_punt("Build_Expr_DAG: OP_postinc");
    case OP_postdec:
      P_punt("Build_Expr_DAG: OP_postdec");
    case OP_quest:
      P_punt("Build_Expr_DAG: OP_quest");
    case OP_compexpr: /* {"("} {P_GetExprOperands(expr)} (maybe another OP_compexpr)
                         {","} {P_GetExprOperands(expr)->next} (maybe empty) {")"} */
      /*
       * build DAG for each operand individually.
       */
      opnd = P_GetExprOperands(expr);
      if (opnd) {
        succ_vnode = BuildExprVgraph(vgraph, opnd, bb);
        VnodeAppendNewSuccVnode(op_vnode, succ_vnode);
        opnd = P_GetExprNext(opnd);
        while (opnd) {
          succ_vnode = BuildExprVgraph(vgraph, opnd, bb);
          VnodeAppendNewSuccVnode(op_vnode, succ_vnode);
          opnd = P_GetExprNext(opnd);
        }
      } 
      break;
    case OP_index: /* {P_GetExprOperands(expr)} {"["} {P_GetExprSibling(P_GetExprOperands(expr))} {"]"} */
    case OP_disj:  /* {P_GetExprOperands(expr)} {"||"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_conj:  /* {P_GetExprOperands(expr)} {"&&"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_or:    /* {P_GetExprOperands(expr)} {"|"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_xor:   /* {P_GetExprOperands(expr)} {"^"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_and:   /* {P_GetExprOperands(expr)} {"&"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_eq:    /* {P_GetExprOperands(expr)} {"=="} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_ne:    /* {P_GetExprOperands(expr)} {"!="} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_lt:    /* {P_GetExprOperands(expr)} {"<"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_le:    /* {P_GetExprOperands(expr)} {"<="} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_ge:    /* {P_GetExprOperands(expr)} {">="} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_gt:    /* {P_GetExprOperands(expr)} {">"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_rshft: /* {P_GetExprOperands(expr)} {">>"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_lshft: /* {P_GetExprOperands(expr)} {"<<"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_add:   /* {P_GetExprOperands(expr)} {"+"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_sub:   /* {P_GetExprOperands(expr)} {"-"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_mul:   /* {P_GetExprOperands(expr)} {"*"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_div:   /* {P_GetExprOperands(expr)} {"/"} {P_GetExprSibling(P_GetExprOperands(expr))} */
    case OP_mod:   /* {P_GetExprOperands(expr)} {"%"} {P_GetExprSibling(P_GetExprOperands(expr))} */
      succ_vnode = BuildExprVgraph(vgraph, P_GetExprOperands(expr), bb);
      VnodeAppendNewSuccVnode(op_vnode, succ_vnode);
      succ_vnode = BuildExprVgraph(vgraph, P_GetExprSibling(P_GetExprOperands(expr)), bb);
      VnodeAppendNewSuccVnode(op_vnode, succ_vnode);
      break;
    case OP_call:  /* {P_GetExprOperands(expr)} {"("}
                      {P_GetExprSibling(P_GetExprOperands(expr))} {","}
                      {P_GetExprSibling(P_GetExprOperands(expr))->next} {","}
                      {P_GetExprSibling(P_GetExprOperands(expr))->next->next} ... {")"} */
      /*
       * build DAG for each operand independently.
       */
      succ_vnode = BuildExprVgraph(vgraph, P_GetExprOperands(expr), bb); /* dummy succ_node */
      opnd = P_GetExprSibling(P_GetExprOperands(expr));
      while(opnd) {
        succ_vnode = BuildExprVgraph(vgraph, opnd, bb); /* dummy succ_node */
        opnd = P_GetExprNext(opnd);
      }
      break;
    case OP_assign: /* {P_GetExprOperands(expr)} {"="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_assign");
    case OP_Aadd:   /* {P_GetExprOperands(expr)} {"+="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Aadd");
    case OP_Asub:   /* {P_GetExprOperands(expr)} {"-="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Asub");
    case OP_Amul:   /* {P_GetExprOperands(expr)} {"*="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Amul");
    case OP_Adiv:   /* {P_GetExprOperands(expr)} {"/="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Adiv");
    case OP_Amod:   /* {P_GetExprOperands(expr)} {"%="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Amod");
    case OP_Arshft: /* {P_GetExprOperands(expr)} {">>="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Arshft");
    case OP_Alshft: /* {P_GetExprOperands(expr)} {"<<="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Alshft");
    case OP_Aand:   /* {P_GetExprOperands(expr)} {"&="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Aand");
    case OP_Aor:    /* {P_GetExprOperands(expr)} {"|="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Aor");
    case OP_Axor:   /* {P_GetExprOperands(expr)} {"^="} {P_GetExprSibling(P_GetExprOperands(expr))} */
      P_punt("Build_Expr_DAG: OP_Axor");
    case OP_phi: /* P_GetExprOperands(expr) = PHI(P_GetExprSibling(P_GetExprOperands(expr)), 
                                              P_GetExprSibling(P_GetExprSibling(P_GetExprOperands(expr))) ... ) */
      opnd = P_GetExprSibling(P_GetExprOperands(expr));
      #if DEBUG_P_SS_BuildVgraph
      if (opnd == NULL)
        fprintf(stderr, "\n!!! WARNING: BuildExprVgraph: OP_phi->operands->sibling is null !!!\n");
      #endif
      while (opnd) {
        ssa_expr = P_CF_GetSSA(opnd);
        assert(ssa_expr);
        assert((P_GetExprOpcode(ssa_expr) == OP_assign) || (P_GetExprOpcode(ssa_expr) == OP_phi));
        if (P_GetExprOpcode(ssa_expr) == OP_assign)
          succ_expr = P_GetExprOperands(ssa_expr);
        else if (P_GetExprOpcode(ssa_expr) == OP_phi)
          succ_expr = ssa_expr;
        else 
          P_punt("BuildExprVgraph: ssa of OP_phi operand is not OP_assign nor OP_phi");

        succ_vnode = FindAndAllocateVnode(vgraph, succ_expr, Get_ExprExtForVgraph_bb(succ_expr));

        VnodeAppendNewSuccVnode(op_vnode, succ_vnode);
        opnd = P_GetExprSibling(opnd);
      }
      break;
    default:
      P_punt("BuildExprVgraph: unknown opcode"); 
  }
  return op_vnode;
}

static P_Scc 
NewScc (int id)
{
  P_Scc s;

  s = ALLOCATE(struct _P_Scc);
  Scc_id(s) = id;
  Scc_flag(s) = 0;
  Scc_num_comp(s) = 0;
  Scc_comp(s) = NULL;
  Scc_next(s) = NULL;
  Scc_prev(s) = NULL;
  return s;
}

static void
FreeScc(P_Scc scc)
{
  Lptr curr_comp;
  Lptr prev_comp;

  curr_comp = Scc_comp(scc);
  while (curr_comp) {
    prev_comp = curr_comp;
    curr_comp = LPTR_next(curr_comp);
    LPTR_ptr(prev_comp) = NULL;
    LPTR_next(prev_comp) = NULL;
    FreeLptr(prev_comp);
  }  
  DISPOSE(scc);
}

static void
FreeSccInVgraph (P_Vgraph vgraph)
{
  P_Scc curr_scc;
  P_Scc prev_scc;

  curr_scc = Vgraph_first_scc(vgraph); 
  while (!Vgraph_end_of_scc(curr_scc)) {
    prev_scc = curr_scc;
    curr_scc = Scc_next(curr_scc);
    FreeScc(prev_scc);
  } 
  Vgraph_first_scc(vgraph) = NULL;
  Vgraph_num_scc(vgraph) = 0;
}

static int
SccVisit (P_Vgraph vgraph, P_Vnode node)
{
  int m, min;
  Lptr succ_ptr;
  Lptr new_scc_comp;
  P_Vnode succ_node;
  P_Vnode pop_node;
  P_Scc new_scc;

  min = Vnode_status(node) = visit_id++;
  En_FILO_LptrQ(&vnode_stack, node);
  for (succ_ptr = Vnode_first_succ(node); 
       !Vnode_end_of_succ(succ_ptr); 
       succ_ptr = Vnode_next_succ(succ_ptr)) {
    succ_node = Vnode_succ_vnode(succ_ptr);
    if (Vnode_status(succ_node) == -1)
      m = SccVisit(vgraph, succ_node);
    else
      m = Vnode_status(succ_node);
    if (m < min)
      min = m;
  }
  if (min == Vnode_status(node)) { /* a new Scc found */
    Vgraph_num_scc(vgraph)++;
    new_scc = NewScc(Vgraph_num_scc(vgraph));
    succ_node = node;
    do {
      pop_node = De_LptrQ(&vnode_stack);
      Vnode_scc(pop_node) = new_scc;
      Vnode_status(pop_node) = Vgraph_num_vnode(vgraph) + 1;
      Vnode_scc_prev(succ_node) = pop_node;
      succ_node = pop_node;
      new_scc_comp = NewSccComp(pop_node);
      Scc_comp(new_scc) = AppendLptr(Scc_comp(new_scc), new_scc_comp);
      Scc_num_comp(new_scc)++;
    } while (pop_node != node);
    Scc_next(new_scc) = Vgraph_first_scc(vgraph);
    if (Vgraph_first_scc(vgraph))
      Scc_prev(Vgraph_first_scc(vgraph)) = new_scc;
    Vgraph_first_scc(vgraph) = new_scc;
  }
  return min;
}

static P_Value 
NewValue()
{
  P_Value v;

  v = ALLOCATE (struct _P_Value);
  Value_vid(v) = NULL;
  Value_const(v) = NULL;
  Value_induct_init(v) = NULL;
  Value_induct_incr(v) = NULL;
  return v;
}

static P_Value
NewUnknownValue()
{
  return UNKNOWN_VALUE;
}

void
P_SS_DumpValue (FILE *out_file, P_Value value)
{
  if (value == NULL) {
    fprintf(out_file, "NULL");
    return;
  }
  if (Value_is_unknown(value)) {
    fprintf(out_file, "UNKNOWN");  
    return;
  }
  if (Value_is_const(value)) {
    Gen_CCODE_Expr(out_file, Value_const(value));
    return;  
  } 
  fprintf(out_file, " < ");
  if (Value_vid(value) && Alpha_var_ids_loop(Value_vid(value)))
    fprintf(out_file, "Loop(%d), ", Get_Loop_id(Alpha_var_ids_loop(Value_vid(value))));
  else if (Value_vid(value) == NULL)
    fprintf(out_file, "vid==NULL, ");
  else if (Alpha_var_ids_loop(Value_vid(value)) == NULL)
    fprintf(out_file, "loop=NULL, ");
  P_SS_DumpValue(out_file, Value_induct_init(value));
  fprintf(out_file, ", ");
  Gen_CCODE_Expr(out_file, Value_induct_incr(value));
  fprintf(out_file, " >");
}

static int
IsCommutative(_Opcode opcode)
{
  return (opcode == OP_add)  ||
         (opcode == OP_mul)  ||
         (opcode == OP_and)  ||
         (opcode == OP_or)   ||
         (opcode == OP_xor)  ||
         (opcode == OP_eq)   ||
         (opcode == OP_ne)   ||
         (opcode == OP_disj) ||
         (opcode == OP_conj);
}

static P_Value 
NewIntValue (int n)
{
  P_Value value;

  value = NewValue();
  Value_const(value) = NewIntExpr(n);
  return value;
}

static P_Value 
NewConstValue (Expr expr)
{
  Alpha_var_id vid;
  P_Value value;

  if (P_FindPragma(P_GetExprPragma(expr), "UnknownValue"))
    return NewUnknownValue();  
  if (expr->opcode == OP_var) {
    STRING_Symbol *string_symbol;

    vid = New_Alpha_var_id(P_GetExprVarKey(expr), P_GetExprVarName(expr), NULL);
    string_symbol = STRING_find_symbol(var_id_tbl, P_GetExprVarName(expr));
    if (!string_symbol) 
      STRING_add_symbol(var_id_tbl, P_GetExprVarName(expr), vid);
    else if (!PSI_IsArrayTypeExpr(expr))
      P_punt("NewConstValue: re-define symbol");
  } else
    vid = NULL;
  value = NewValue();
  Value_vid(value) = vid;
  Value_const(value) = expr;
  return value;
}

static P_Value 
NewInductValue (Alpha_var_id vid, P_Value init, Expr incr)
{
  P_Value value;

  assert (Alpha_var_ids_index_p(vid));
  value = NewValue(); 
  Value_vid (value) = vid;
  Value_induct_init(value) = init;
  Value_induct_incr(value) = incr;
  return value;
}

static P_Value
CopyConstValue (P_Value v)
{
  P_Value new_value;

  new_value = NewValue();
  Value_vid(new_value) = Copy_Alpha_var_id(Value_vid(v));
  Value_const(new_value) = P_CopyExpr(Value_const(v));
  return new_value;
}

static P_Value
CopyInductValue (P_Value v)
{
  P_Value new_init;
  Expr new_incr;

  new_init = CopyValue (Value_induct_init(v));
  assert (P_GetExprOpcode(Value_induct_incr(v)) == OP_int);
  new_incr = NewIntExpr(P_GetExprScalar(Value_induct_incr(v)));
  new_init = NewInductValue(Copy_Alpha_var_id(Value_vid(v)), new_init, new_incr);
  return new_init;
}

static P_Value
CopyValue (P_Value v)
{
  P_Value ret_value;

  if (Value_is_unknown(v))
    return NewUnknownValue();
  if (Value_is_const(v)) {
    ret_value = CopyConstValue(v);
    return ret_value;
  }
  assert (Value_is_induct(v));
  ret_value = CopyInductValue(v);
  return ret_value;
}

/*
 *   closure(<loop, init, incr>)
 * = closure(init + incr*(loop.count-1))
 */
static P_Value 
ValueClose (P_Value v, PC_Loop user_loop)
{
  Alpha_var_id vid;
  PC_Loop loop;
  P_Value tripcount;
  P_Value init_value;
  P_Value incr_value; 
  P_Value mul_value;
  P_Value add_value;
  Expr incr_expr;

  if (Value_is_unknown(v))
    return NewUnknownValue();
  if (Value_is_const(v)) {
    init_value = CopyConstValue(v);
    return init_value;
  }
  assert (Value_is_induct(v)); 
  vid = Value_vid(v);  
  loop = Alpha_var_ids_loop(vid); 
  if ((user_loop != NULL) && ((loop == NULL) || 
      (loop == user_loop) || 
      Loop1EnclosedInLoop2(user_loop, loop))) {
    init_value = CopyInductValue(v);
    return init_value;
  }

  tripcount = Get_Loop_tripcount(loop);
  if (tripcount == NULL) { /* TO DO: see can remove this, be conservative now */
    fprintf(stderr, "\nWARNING: ValueClose: tripcount == NULL\n");
    return NewUnknownValue();
  }

  if (Value_is_unknown(tripcount))
    return NewUnknownValue();

  if (!(Value_is_const(tripcount) && (P_GetExprOpcode(Value_const(tripcount)) == OP_int)))
    return NewUnknownValue();

  assert (P_GetExprOpcode(Value_induct_incr(v)) == OP_int);
  incr_expr = NewIntExpr(P_GetExprScalar(Value_induct_incr(v)));
  incr_value = NewConstValue(incr_expr);

  mul_value = ValueComputeWithTwoOperands(OP_mul, incr_value, tripcount); 
  init_value = ValueClose(Value_induct_init(v), user_loop);
  add_value = ValueComputeWithTwoOperands(OP_add, init_value, mul_value);
 
  P_SS_FreeValue(incr_value);
  P_SS_FreeValue(mul_value);
  P_SS_FreeValue(init_value);

  return add_value;
}

static int
ValueEqual (P_Value v1, P_Value v2)
{
  if (Value_is_unknown(v1) || Value_is_unknown(v2))
    return 0;
  if (v1 == v2)
    return 1;
  if (Value_is_induct(v1) && Value_is_induct(v2))
    return ValueEqual(Value_induct_init(v1), Value_induct_init(v2)) &&
           ConstValueEqual(Value_induct_incr(v1), Value_induct_incr(v2));
  if (Value_is_const(v1) && Value_is_const(v2))
      return ConstValueEqual(Value_const(v1), Value_const(v2));
  return 0;
}

static int
ConstExprEqual (Expr e1, Expr e2)
{
  P_Value value1;
  P_Value value2;

  if (e1 == e2)
    return 1;

  value1 = Get_ExprExtForVgraph_value(e1);
  value2 = Get_ExprExtForVgraph_value(e2);

  if (value1 && value2)
    return ValueEqual(value1, value2);
  if (value1 || value2)
    return 0;
  /* value1 == NULL && value2 == NULL */
  return ConstValueEqual(e1, e2);
}

static int
ConstValueEqualOfDualOp (Expr expr1, Expr expr2)
{
  Expr e1;
  Expr e2;

  e1 = P_GetExprOperands(expr1);
  e2 = P_GetExprOperands(expr2);
  if (!ConstExprEqual(e1, e2))
    return 0;
  e1 = P_GetExprSibling(e1);
  e2 = P_GetExprSibling(e2); 
  return ConstExprEqual(e1, e2);
}

static int
ConstValueEqualOfCommutativeOp (Expr expr1, Expr expr2)
{
  Expr e1;
  Expr e2;

  if (ConstValueEqualOfDualOp(expr1, expr2))
    return 1;
  e1 = P_GetExprOperands(expr1);  
  e2 = P_GetExprSibling(P_GetExprOperands(expr2));
  if (!ConstExprEqual(e1, e2))
    return 0;
  e1 = P_GetExprOperands(expr2);
  e2 = P_GetExprSibling(P_GetExprOperands(expr1));
  return ConstExprEqual(e1, e2);
}

/*
 * Now the comparison is not semantics based. Maybe call m_linearity and find_coeff to normalize it.
 */
static int
ConstValueEqual (Expr e1, Expr e2)
{
  if (e1 == e2)
    return 1;

  if (P_GetExprOpcode(e1) != P_GetExprOpcode(e2))
    return 0;

  switch (P_GetExprOpcode(e1))
    {
      case OP_var:
        return 0;

      case OP_char:
        P_punt("ConstValueEqual: OP_char");
      case OP_enum:
      case OP_string:
        return !strcmp(P_GetExprString(e1), P_GetExprString(e2));

      case OP_int:
        return (P_GetExprScalar(e1) == P_GetExprScalar(e2));

      case OP_float:
      case OP_double:
      case OP_real:
        return (P_GetExprReal(e1) == P_GetExprReal(e2));

      case OP_error:
        P_punt("ConstValueIsEqual: OP_enum");
     
      case OP_dot:
      case OP_arrow: /* TO DO: too conservative? */
          return 0;

      case OP_expr_size:
        P_punt("ConstValueIsEqual: OP_expr_size");

      case OP_type_size:
        P_punt("ConstValueIsEqual: OP_type_size");

      case OP_quest:
        return 0;

      case OP_disj:
        P_punt("ConstValueIsEqual: OP_disj");

      case OP_conj:
        P_punt("ConstValueIsEqual: OP_conj");

      case OP_compexpr:
        P_punt("ConstValueIsEqual: OP_compexpr");

      case OP_assign:
        P_punt("ConstValueIsEqual: OP_assign");

      case OP_or:
      case OP_xor:
      case OP_and:
      case OP_eq:
      case OP_ne:
      case OP_add:
      case OP_mul:
        return ConstValueEqualOfCommutativeOp(e1, e2);

      case OP_lt:
      case OP_le:
      case OP_ge:
      case OP_gt:
      case OP_rshft:
      case OP_lshft:
      case OP_sub:
      case OP_div:
      case OP_mod:
      case OP_index:
        return ConstValueEqualOfDualOp(e1, e2); 

      case OP_neg:
      case OP_not:
      case OP_inv:
      case OP_preinc:
      case OP_predec:
      case OP_postinc:
      case OP_postdec:
      case OP_addr:
      case OP_indr:
        return ConstExprEqual(P_GetExprOperands(e1), P_GetExprOperands(e2));

      case OP_cast:
        return ConstExprEqual(P_GetExprOperands(e1), P_GetExprOperands(e2));

      case OP_Aadd:
      case OP_Asub:
      case OP_Amul:
      case OP_Adiv:
      case OP_Amod:
      case OP_Arshft:
      case OP_Alshft:
      case OP_Aand:
      case OP_Aor:
      case OP_Axor:
        P_punt ("ConstValueEqual: OP_Axxx");

      case OP_call:
      case OP_null:
      case OP_sync:
      case OP_stmt_expr:
      case OP_asm_oprd:
      case OP_phi:
        return 0;

      default:
        P_punt("ConstValueEqual: unknown opcode");
    }
  return 0;
}

static P_Value 
VnodeClosedValue (P_Vnode vnode, PC_Loop user_loop)
{
  P_Value value;

  value = VnodeFindValue(vnode);
  return ValueClose(value, user_loop);
}

static P_Value 
ValueComputeConstConst (_Opcode opcode, P_Value v1, P_Value v2)
{
  P_Value value;
  Expr new_expr;
  Expr op1;
  Expr op2;

  if (Value_is_integer(v1) && Value_is_integer(v2)) {
    op1 = NewIntExpr(Value_integer(v1));
    op2 = NewIntExpr(Value_integer(v2));
    new_expr = NewDualOpExpr(opcode, op1, op2);
    new_expr = ReduceExpr(new_expr); 
    value = NewConstValue(new_expr);
  } else {
    op1 = P_CopyExpr(Value_const(v1));
    op2 = P_CopyExpr(Value_const(v2));
    new_expr = NewDualOpExpr(opcode, op1, op2);
    value = NewConstValue(new_expr);
  }
  return value;
}

static P_Value 
ValueComputeInductConst (_Opcode opcode, P_Value v1, P_Value v2)
{
  Expr new_incr = NULL;
  P_Value new_init;

  if (((opcode != OP_add) && (opcode != OP_sub) && (opcode != OP_mul) && (opcode != OP_lshft)) ||
      (((opcode == OP_mul) || (opcode == OP_lshft)) && !Value_is_integer(v2)))
    return NewUnknownValue();
  if (Value_is_zero(v2) && (opcode == OP_mul))
    return CopyValue(v2);
  if ((Value_is_zero(v2) && (opcode != OP_mul)) ||
      (Value_is_one(v2) && (opcode == OP_mul))) 
    return CopyValue(v1);
  new_init = ValueComputeWithTwoOperands(opcode, Value_induct_init(v1), v2);
  if (Value_is_unknown(new_init))
    return NewUnknownValue();
  assert(P_GetExprOpcode(Value_induct_incr(v1)) == OP_int);
  switch (opcode) {
    case OP_add:
    case OP_sub:
      new_incr = NewIntExpr(P_GetExprScalar(Value_induct_incr(v1)));
      break;
    case OP_mul:
      new_incr = NewIntExpr(P_GetExprScalar(Value_induct_incr(v1)) * Value_integer(v2));
      break;
    case OP_lshft:
      new_incr = NewIntExpr(P_GetExprScalar(Value_induct_incr(v1)) << Value_integer(v2));
      break;
    default:
      P_punt("ValueCoputeInductConst: unexpected opcode");
  }
  new_init = NewInductValue(Copy_Alpha_var_id(Value_vid(v1)), new_init, new_incr);
  return new_init;
}

static P_Value 
ValueComputeConstInduct (_Opcode opcode, P_Value v1, P_Value v2)
{
  Expr new_incr;
  P_Value new_init;

  if (IsCommutative(opcode)) 
    return ValueComputeInductConst(opcode, v2, v1);
  if (opcode != OP_sub)
    return NewUnknownValue();
  /*
   * only handle const - <init, incr> = <const - init, -incr>
   */
  if (Value_is_zero(v1))
    new_init = ValueComputeWithOneOperand (OP_neg, Value_induct_init(v2));
  else
    new_init = ValueComputeWithTwoOperands (OP_sub, v1, Value_induct_init(v2));
  if (Value_is_unknown(new_init))
    return NewUnknownValue();
  new_incr = NewIntExpr(-P_GetExprScalar(Value_induct_incr(v2)));
  new_init = NewInductValue(Value_vid(v2), new_init, new_incr);
  return new_init;
}

static P_Value 
ValueComputeInductInduct (_Opcode opcode, P_Value v1, P_Value v2)
{
  Alpha_var_id vid1;
  Alpha_var_id vid2;
  PC_Loop loop1;
  PC_Loop loop2;
  P_Value new_init;
  int new_incr;
  Expr new_incr_expr;

  /*
   * only handle <init1, incr1> +/- <init2, incr2> = <init1 +/- init2, incr1 +/- incr2>
   */
  if ((opcode != OP_add) && (opcode != OP_sub))
    return NewUnknownValue();
  vid1 = Value_vid(v1);
  assert(vid1);
  vid2 = Value_vid(v2);
  assert(vid2);
  loop1 = Alpha_var_ids_loop(vid1);
  loop2 = Alpha_var_ids_loop(vid2);
  if (loop1 == loop2) 
    {
      /* assert (vid1 == vid2); */
      new_init = ValueComputeWithTwoOperands(opcode, Value_induct_init(v1), Value_induct_init(v2)); 
      if (Value_is_unknown(new_init))
        return NewUnknownValue();
      if (opcode == OP_add)
        new_incr = P_GetExprScalar(Value_induct_incr(v1)) + P_GetExprScalar(Value_induct_incr(v2));
      else /* opcode == OP_sub */
        new_incr = P_GetExprScalar(Value_induct_incr(v1)) - P_GetExprScalar(Value_induct_incr(v2));
      if (new_incr == 0)
        return new_init;
      new_incr_expr = NewIntExpr(new_incr); 
      new_init = NewInductValue(Copy_Alpha_var_id(vid1), new_init, new_incr_expr);
      return new_init;
    }
  else if (Loop1EnclosedInLoop2(loop1, loop2)) 
    {
      /*
       * v1 is inductive in inner loop
       * v2 is inductive in outer loop
       */
      new_init = ValueComputeWithTwoOperands(opcode, Value_induct_init(v1), v2);
      if (Value_is_unknown(new_init))
        return NewUnknownValue();
      new_incr_expr = NewIntExpr(P_GetExprScalar(Value_induct_incr(v1)));
      new_init = NewInductValue(Copy_Alpha_var_id(vid1), new_init, new_incr_expr); 
      return new_init;
    }
  else if (Loop1EnclosedInLoop2(loop2, loop1)) 
    {
      /*
       * v2 is inductive in inner loop
       * v1 is inductive in outer loop
       */
      new_init = ValueComputeWithTwoOperands(opcode, v1, Value_induct_init(v2));
      if (Value_is_unknown(new_init))
        return NewUnknownValue();
      if (opcode == OP_add)
        new_incr_expr = NewIntExpr(P_GetExprScalar(Value_induct_incr(v2)));
      else /* opcode == OP_sub */
        new_incr_expr = NewIntExpr(-P_GetExprScalar(Value_induct_incr(v2)));
      new_init =  NewInductValue(Copy_Alpha_var_id(vid2), new_init, new_incr_expr); 
      return new_init;
    } 
  else 
    P_punt("ValueComputeInductInduct: v1 and v2 have no common loop nest");
  return NULL; /* just for removing compile warning */
}

static P_Value 
ValueComputeWithTwoOperands (_Opcode opcode, P_Value v1, P_Value v2)
{
  assert ((v1 != NULL) && (v2 != NULL));
  if (Value_is_unknown(v1) || Value_is_unknown(v2))
    return NewUnknownValue();

  if (Value_is_const(v1) && Value_is_const(v2)) 
    return ValueComputeConstConst(opcode, v1, v2);

  if (Value_is_const(v1) && Value_is_induct(v2)) 
    return ValueComputeConstInduct(opcode, v1, v2);

  if (Value_is_induct(v1) && Value_is_const(v2))
    return ValueComputeInductConst(opcode, v1, v2);

  assert (Value_is_induct(v1) && Value_is_induct(v2));
  return ValueComputeInductInduct(opcode, v1, v2);
}

static P_Value 
ValueComputeConst (_Opcode opcode, P_Value v)
{
  P_Value value;
  Expr new_expr;
  Expr op1;

  assert (v != NULL);
  if (Value_is_unknown(v))
    return NewUnknownValue();
  if (Value_is_integer(v)) {
    op1 = NewIntExpr(P_GetExprScalar(Value_const(v)));
    new_expr = NewSingleOpExpr(opcode, op1);
    new_expr = ReduceExpr(new_expr);
    value = NewConstValue(new_expr);
  } else {
    op1 = P_CopyExpr(Value_const(v));
    new_expr = NewSingleOpExpr(opcode, op1);
    value = NewConstValue(new_expr);
  }
  return value;
}

static P_Value 
ValueComputeInduct (_Opcode opcode, P_Value v)
{
  P_Value new_init;
  Expr incr_expr;
  Expr new_incr;

  assert (v != NULL);
  if (Value_is_unknown(v))
    return NewUnknownValue();
  switch (opcode) {
    case OP_neg:
    {
      new_init = ValueComputeWithOneOperand(OP_neg, Value_induct_init(v));
      if (Value_is_unknown(new_init))
        return NewUnknownValue();
      incr_expr = Value_induct_incr(v);
      assert (P_GetExprOpcode(incr_expr) == OP_int);
      new_incr = NewIntExpr(-P_GetExprScalar(incr_expr));
      new_init = NewInductValue(Value_vid(v), new_init, new_incr);
      return new_init;
    }
    default:
      return NewUnknownValue();
  }
}

static P_Value 
ValueComputeWithOneOperand (_Opcode opcode, P_Value v)
{
  assert (v != NULL);
  if (Value_is_unknown(v))
    return NewUnknownValue();
  if (Value_is_const(v))
    return ValueComputeConst(opcode, v);
  assert (Value_is_induct(v));
  return ValueComputeInduct(opcode, v);
}

static P_Value
VnodeFindValue (P_Vnode vnode)
{
  _Opcode opcode;
  Expr expr;
  Expr new_expr;
  PC_Loop loop;
  P_Vnode op1_vnode;
  P_Vnode op2_vnode;
  P_Value value1;
  P_Value value2;
  P_Value ret_value = NULL;
  P_Scc scc;

  if (Dump_VnodeFindValue)
    fprintf(stderr, "VnodeFindValue %d\n", vnode->id);
  expr = Vnode_expr(vnode);

  value1 = Get_ExprExtForVgraph_value(expr);

  if (value1 != NULL)
    return value1;

#if 1 
  scc = Vnode_scc(vnode);
  if ((Scc_num_comp(scc) > 1) && (Scc_flag(scc) == 0)) {
    FindValuesInScc(scc);
    expr = Vnode_expr(vnode);
    value1 = Get_ExprExtForVgraph_value(expr);
    assert (value1 != NULL);
    return value1;
  }
#endif

  loop = Vnode_loop(vnode);
  opcode = P_GetExprOpcode(expr);
  switch (opcode) {
    case OP_var: 
    {
      if (Vnode_first_succ(vnode)) {
        assert (Vnode_num_succ(vnode) == 1);
        op1_vnode = Vnode_succ1_vnode(vnode);
        ret_value = VnodeClosedValue(op1_vnode, loop);
      } else if (P_FindPragma(P_GetExprPragma(expr), "ParamValue") || 
		 PSI_IsArrayTypeExpr(expr)) 
	{
	  new_expr = NewVarExpr(strdup(P_GetExprVarName(expr)), 
				PSI_ExprType(expr), P_GetExprVarKey(expr));
	  ret_value = NewConstValue(new_expr);
	} 
      else 
        ret_value = NewUnknownValue();

      Set_ExprExtForVgraph_value(expr, ret_value); 

      break;
    }
    case OP_int: 
    {
      if (P_FindPragma(P_GetExprPragma(expr), "UnknownValue"))
        ret_value = NewUnknownValue();
      else {
        new_expr = NewIntExpr(P_GetExprScalar(expr));
        ret_value = NewConstValue(new_expr);
        Set_ExprExtForVgraph_value(expr, ret_value);
      }
      break;
    }
    case OP_char:
    {
      P_punt("VnodeFindValue: OP_char");
      break;
    }
    case OP_string:
    {
      if (P_FindPragma(P_GetExprPragma(expr), "UnknownValue"))
        ret_value = NewUnknownValue();
      else {
        new_expr = NewStringExpr(strdup(P_GetExprString(expr)));
        ret_value = NewConstValue(new_expr);
        Set_ExprExtForVgraph_value(expr, ret_value);
      }
      break;
    }
    case OP_real:
    {
      P_punt("VnodeFindValue: OP_real");
      break;
    }
    case OP_float:
    {
      if (P_FindPragma(P_GetExprPragma(expr), "UnknownValue"))
        ret_value = NewUnknownValue();
      else {
        new_expr = NewFloatExpr(P_GetExprReal(expr));
        ret_value = NewConstValue(new_expr);
        Set_ExprExtForVgraph_value(expr, ret_value);
      }
      break;
    }
    case OP_double:
    {
      if (P_FindPragma(P_GetExprPragma(expr), "UnknownValue"))
        ret_value = NewUnknownValue();
      else {
        new_expr = NewDoubleExpr(P_GetExprReal(expr));
        ret_value = NewConstValue(new_expr);
        Set_ExprExtForVgraph_value(expr, ret_value);
      }
      break;
    }
    case OP_cast:
    {
      assert (Vnode_num_succ(vnode) == 1);
      op1_vnode = Vnode_succ1_vnode(vnode);
      value1 = VnodeClosedValue(op1_vnode, loop);
      if (!Value_is_unknown(value1)) {
        if (Value_is_const(value1)) {

          new_expr = NewCastExpr(P_GetExprType(expr), 
				 P_CopyExpr(Value_const(value1)));
	  ret_value = NewConstValue(new_expr);
#if 0
          PSI_SetExprType(Value_const(ret_value), PSI_ExprType(expr));
#endif
        } else { 
          fprintf(stderr, "VnodeFindValue: casting induction value for vnode(%d)\n", vnode->id);
          ret_value = value1;
        }
      } else
        ret_value = value1;
      Set_ExprExtForVgraph_value(expr, ret_value);
      break;
    } 
    case OP_compexpr:
    {
      Lptr vnode_ptr;

      vnode_ptr = Vnode_first_succ(vnode);
      if (vnode_ptr) {
        while (LPTR_next(vnode_ptr))
          vnode_ptr = LPTR_next(vnode_ptr);
        op1_vnode = LPTR_ptr(vnode_ptr);
        ret_value = VnodeClosedValue(op1_vnode, loop);
      } else
        ret_value = NewUnknownValue();
      Set_ExprExtForVgraph_value(expr, ret_value);
      break;
    }
    case OP_disj:
    case OP_conj:
    case OP_or:
    case OP_xor:
    case OP_and:
    case OP_add:
    case OP_lshft: 
    case OP_rshft: 
    case OP_mul: 
    case OP_div: 
    case OP_mod: 
    {
      assert (Vnode_num_succ(vnode) == 2);
      op1_vnode = Vnode_succ1_vnode(vnode);
      op2_vnode = Vnode_succ2_vnode(vnode);
      value1 = VnodeClosedValue(op1_vnode, loop);
      value2 = VnodeClosedValue(op2_vnode, loop);
      ret_value = ValueComputeWithTwoOperands(opcode, value1, value2);
      Set_ExprExtForVgraph_value(expr, ret_value);
      P_SS_FreeValue(value1);
      P_SS_FreeValue(value2);
      break;
    }
    case OP_sub:
    case OP_ge:
    case OP_gt:
    case OP_lt:
    case OP_le:
    case OP_ne:
    case OP_eq: 
    {
      assert (Vnode_num_succ(vnode) == 2);
      op1_vnode = Vnode_succ1_vnode(vnode);
      op2_vnode = Vnode_succ2_vnode(vnode);
      value1 = VnodeClosedValue(op1_vnode, loop);
      value2 = VnodeClosedValue(op2_vnode, loop);
      ret_value = ValueComputeWithTwoOperands(OP_sub, value1, value2);
      Set_ExprExtForVgraph_value(expr, ret_value);
      P_SS_FreeValue(value1);
      P_SS_FreeValue(value2);
      break;
    }
    case OP_neg:
    case OP_not:
    case OP_inv:
    {
      assert (Vnode_num_succ(vnode) == 1);
      op1_vnode = Vnode_succ1_vnode(vnode);
      value1 = VnodeClosedValue(op1_vnode, loop); 
      ret_value = ValueComputeWithOneOperand(opcode, value1);
      Set_ExprExtForVgraph_value(expr, ret_value);
      P_SS_FreeValue(value1);
      break;
    }
    case OP_dot:
    case OP_arrow:
    case OP_indr:
    case OP_addr:
    case OP_index:
    {
      ret_value = NewUnknownValue();
      Set_ExprExtForVgraph_value(expr, ret_value);
      break;
    } 
    case OP_call:
    {
      /*
       * it will be interesting to do this inter-procedurally, but not for now.
       */
      ret_value = NewUnknownValue(); 
      Set_ExprExtForVgraph_value(expr, ret_value);
      break;
    }
    case OP_phi: 
    {
      Lptr succ;
      succ = Vnode_first_succ(vnode);
      assert (succ);
      op1_vnode = LPTR_ptr(succ);
      if (op1_vnode == vnode)
        succ = LPTR_next(succ);
      if (!succ)
        value1 = NewUnknownValue();
      else {
	op1_vnode = LPTR_ptr(succ);
        value1 = VnodeClosedValue(op1_vnode, loop);
        value2 = NULL;
        succ = LPTR_next(succ);
        while (!Vnode_end_of_succ(succ)) {
          op2_vnode = LPTR_ptr(succ);
          if (op2_vnode != vnode) {
            P_SS_FreeValue(value2);
            value2 = VnodeClosedValue(op2_vnode, loop);
 
            if (!ValueEqual(value1, value2)) {
              P_SS_FreeValue(value1);
              value1 = NewUnknownValue();
              break;
            }   
          }
          succ = LPTR_next(succ);
        }
        P_SS_FreeValue(value2);
      }
      ret_value = value1;
      Set_ExprExtForVgraph_value(expr, ret_value);
      break;
    }
    case OP_quest:
    case OP_assign:
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
    case OP_Aadd:
    case OP_Asub:
    case OP_Amul:
    case OP_Adiv:
    case OP_Amod:
    case OP_Arshft:
    case OP_Alshft:
    case OP_Aand:
    case OP_Aor:
    case OP_Axor:
    default:
    {
      sprintf(err_msg, "VnodeFindValue: Unexpected opcode %d", opcode);
      P_punt(err_msg);
    }
  } /* end of switch */
  return ret_value;
}

static Alpha_var_id
New_Alpha_var_id (Key key, char *name, struct _PC_Loop *loop)
{
  Alpha_var_id vid;

  vid = ALLOCATE (struct _Alpha_var_id);

  Alpha_var_ids_key(vid) = key;
  Alpha_var_ids_name(vid) = name;
  Alpha_var_ids_loop(vid) = loop;
  Alpha_var_ids_tag(vid) = -1;
  return vid;
}

static Alpha_var_id
Copy_Alpha_var_id (Alpha_var_id v)
{
  return v;
}

static Alpha_var_id 
NewInductionVarId(PC_Loop loop)
{
  Alpha_var_id iv;
  char iv_name[80];
  STRING_Symbol *string_symbol;
  VarDcl var;
  Type type;
  Key key;

  if (Get_Loop_iv(loop))
    return Get_Loop_iv(loop);
  sprintf(iv_name, INDVAR_FORMAT, Get_Loop_id(loop));
  type = PSI_FindBasicType(BT_INT);
  key = PSI_NewLocalVar(ss_curr_func_scope, type, iv_name);
  var = PSI_GetVarDclEntry (key);
  iv = New_Alpha_var_id(key, P_GetVarDclName(var), loop);
  Set_Loop_iv(loop, iv);
  string_symbol = STRING_find_symbol(var_id_tbl, iv_name); 
  if (!string_symbol) 
    STRING_add_symbol(var_id_tbl, iv_name, iv);
  else {
    Alpha_var_id vid;

    vid = STRING_Symbol_data(string_symbol);
    if (!vid)
      P_punt("NewInductionVarId: NULL vid"); 
    if (!Alpha_var_ids_loop(vid))
      P_punt("NewInductionVarId: NULL Alpha_var_ids_loop"); 
    if (Get_Loop_id(Alpha_var_ids_loop(vid)) != Get_Loop_id(loop))
      P_punt("NewInductionVarId: Get_Loop_id(Alpha_var_ids_loop(vid)) != Get_Loop_id(loop)");
  }
  return iv;
}

static void
RegisterSccInLoops(P_Scc scc_list)
{
  Lptr comp;
  P_Scc scc;
  P_Vnode node;
  PC_Block bb;
  PC_Loop loop;
  PC_Loop scc_loop;

  dummy_loop.ext = P_LoopExtForVgraph_alloc();
  Set_Loop_scc_list(&dummy_loop, NULL);
  for (scc = scc_list; scc; scc = Scc_next(scc)) {
    Scc_flag(scc) = 0;
    comp = Scc_comp(scc); 
    node = LPTR_ptr(comp);
    bb = Vnode_bb(node);
    scc_loop = Get_BlockExtForVgraph_loop(bb);
    for (comp = LPTR_next(comp);
         comp;
         comp = LPTR_next(comp)) {
      node = LPTR_ptr(comp);
      bb = Vnode_bb(node);
      loop = Get_BlockExtForVgraph_loop(bb);
      if (loop != scc_loop) {
        #if DEBUG_P_SS_BuildVgraph
        fprintf(stderr, "\n!!! WARNING: RegisterSccInLoops: components in different loops!!!\n");
        #endif
        if (!scc_loop || (loop && Loop1EnclosedInLoop2(loop, scc_loop)))
          scc_loop = loop;
      }
    }
    if (scc_loop)
      Set_Loop_scc_list(scc_loop, AppendLptr(NewLptr(scc), Get_Loop_scc_list(scc_loop)));
    else
      Set_Loop_scc_list(&dummy_loop, AppendLptr(NewLptr(scc), Get_Loop_scc_list(&dummy_loop)));
  }
}

static void
FreeSccInLoop(PC_Loop loop)
{
  FreeLptr(Get_Loop_scc_list(loop));
}

static void
FreeSccInLoops(PC_Loop loop_list)
{
  while (loop_list) {
    FreeSccInLoop(loop_list);
    loop_list = Get_Loop_next(loop_list);
  }
}

static void
DumpScc(FILE *out_file, P_Scc scc, int dump_vnode)
{
  Lptr scc_comp;
  P_Vnode node;

  fprintf(out_file, "Scc[%d] (num_comp = %d):\n", Scc_id(scc), Scc_num_comp(scc));
  if (!dump_vnode) {
    DumpIndent(out_file, 1, INDENT_STRING);
    fprintf(out_file, "Vnode ");
  }
  for (scc_comp = Scc_comp(scc); !Scc_end_of_comp(scc_comp); scc_comp = Scc_next_comp(scc_comp)) {
    node = Scc_comp_vnode(scc_comp);
    if (node == NULL)
      P_punt("P_SS_DumpScc: node is NULL");
    if (node->expr == NULL)
      P_punt("P_SS_DumpScc: node->expr is NULL");
    if (!dump_vnode) {
      fprintf(out_file, "<%d> ", Vnode_id(node));
    } else {
      DumpIndent(out_file, 1, INDENT_STRING);
      fprintf(out_file, "Vnode <%d>\n", Vnode_id(node));
      DumpVnode(out_file, node, 0, 2);
      fprintf(out_file, "\n");
    }
  }
}

static void
Set_CFG_vgraph (PC_Graph cfg, P_Vgraph vg)
{
  curr_vgraph = vg; 
  /* should store this in some ext */
}

static PC_Loop
Vnode_loop(P_Vnode v)
{
  PC_Block bb;

  bb = Vnode_bb(v);
  return Get_BlockExtForVgraph_loop(bb);
}

/*********************************************************************/

void
P_CF_Dump_Control_type(FILE *out_file, _PC_BB_CNT_TYPE t)
{
  switch (t) {
    case CNT_RETURN: 
      fprintf (out_file, "RETURN");
      break;
    case CNT_BREAK:
      fprintf (out_file, "BREAK");
      break;
    case CNT_GOTO:
      fprintf (out_file, "GOTO");
      break;
    case CNT_IF:
      fprintf (out_file, "IF");
      break;
    case CNT_SWITCH:
      fprintf (out_file, "SWITCH");
      break;
    case CNT_ENTRY:
      fprintf (out_file, "ENTRY");
      break;
    case CNT_EXIT:
      fprintf (out_file, "EXIT");
      break;
    default:
      P_punt ("P_CF_Dump_Control_type: illegal type");
  }
}

void
P_CF_Dump_BB (FILE *out_file, PC_Block bb)
{
  Expr expr;
  _PC_ExprIter ei;
  PC_Flow link;

  fprintf(out_file, "BB(%d) cnt_type = ", bb->ID);
  P_CF_Dump_Control_type(out_file, bb->cont_type);
  fprintf(out_file, "\n");

  /*
   * dump previous bb 
   */
  if (bb->prev)
    fprintf(out_file, "\tprev -> BB(%d)\n", bb->prev->ID);
  else
    fprintf(out_file, "\tprev -> null\n");

  /*
   * dump next bb 
   */
  if (bb->next)
    fprintf(out_file, "\tnext -> BB(%d)\n", bb->next->ID);
  else
    fprintf(out_file, "\tnext -> null\n");

  /*
   * dump successor links
   */
  fprintf(out_file, "\tsuccessor -> ");
  for (link = bb->s_flow; link; link = link->s_next_flow)
    {
      if (link->flags & PC_FL_NEVER_TAKEN)
        fprintf(out_file, "{* ");
      fprintf(out_file, "BB(%d)", link->dest_bb->ID);
      if (link->flags & PC_FL_NEVER_TAKEN)
        fprintf(out_file, " *}");
      fprintf(out_file, " ");
    }
  fprintf(out_file, "\n");

  /*
   * dump predecessor links
   */
  fprintf(out_file, "\tpredecessor -> ");
  for (link = bb->p_flow; link; link = link->p_next_flow)
    {
      if (link->flags & PC_FL_NEVER_TAKEN)
        fprintf(out_file, "{* ");
      fprintf(out_file, "BB(%d)", link->src_bb->ID);
      if (link->flags & PC_FL_NEVER_TAKEN)
        fprintf(out_file, " *}");
      fprintf(out_file, " ");
    }
  fprintf(out_file, "\n");

  /*
   * dump enclosing loops
   */
  {  
    PC_Loop loop;
  
    if (P_GetBlockExtForVgraph(bb)) 
      loop = Get_BlockExtForVgraph_loop (bb);
    else
      loop = NULL;
    fprintf(out_file, "\tloop -> %d\n", loop ? loop->ID : -1);
  }

  /*
   * dump lex_predecessor
   */
  {
    Set lex_pred;

    if (P_GetBlockExtForVgraph(bb))
      lex_pred = Get_BlockExtForVgraph_lex_pred (bb);
    else
      lex_pred = NULL;
    Set_print(out_file, "\tlex_predecessor -> ", lex_pred);
  }

 /*
   * dump expressions
   */
  fprintf(out_file, "\texpr ->\n");
  for (expr = PC_ExprIterFirst(bb, &ei, 1); expr; expr = PC_ExprIterNext(&ei, 1))
    {
      if (expr->opcode <= OP_last)
        {
          switch (expr->opcode)
            {
            case OP_phi:
              {
                Expr ssa_expr;
                Expr opnd;

                fprintf(out_file, "\t(%d) %s = PHI ( ", P_GetExprID(expr), P_GetExprVarName(P_GetExprOperands(expr)));
                opnd = expr->operands->sibling;
                while (opnd) {
                   assert (opnd->opcode == OP_int);
                   ssa_expr = P_CF_GetSSA(opnd);
                   fprintf(out_file, "bb[%d]expr(%d) ", (int) P_GetExprScalar(opnd), P_GetExprID(ssa_expr));
                   opnd = opnd->sibling;
                }
                fprintf(out_file, ")\n");
                break;
              }
            default:
               fprintf(out_file, "\t(%d) ", P_GetExprID(expr));
               Gen_CCODE_Expr(out_file, expr);
               fprintf(out_file, "\n");
           }
        }
      else
        {
          P_punt("dump_BB: unknown opcode");
        }
    }
  fprintf(out_file, "\n");
}

void
P_CF_Dump_All_BB(FILE *out_file, PC_Graph cfg, char *title)
{
   PC_Block bb;

   fprintf(out_file, "\n--------------------------------\n%s\n", title);
   /*
    * dump all basic blocks
    */
   for (bb = cfg->first_bb; bb; bb = bb->next)
     P_CF_Dump_BB(out_file, bb);
}

static Expr 
NewIntExpr(ITintmax i)
{
  Expr expr;

  expr = P_NewIntExpr(i);
  PSI_CastExpr(expr);
  P_SetExprParentStmt (expr, dummy_stmt);
  return expr;
}

static Expr 
NewFloatExpr(double f)
{
  Expr expr;

  expr = P_NewFloatExpr(f);
  PSI_CastExpr(expr);
  P_SetExprParentStmt (expr, dummy_stmt);
  return expr;
}

static Expr 
NewDoubleExpr(double f)
{
  Expr expr;

  expr = P_NewDoubleExpr(f);
  PSI_CastExpr(expr);
  P_SetExprParentStmt (expr, dummy_stmt);
  return expr;
}

static Expr 
NewStringExpr(char *str)
{
  Expr expr;

  expr = P_NewStringExpr(str);
  PSI_CastExpr(expr);
  P_SetExprParentStmt (expr, dummy_stmt);
  return expr;
}

static Expr
NewVarExpr(char *name, Type type, Key key)
{
  Expr expr;

  expr = P_NewExprWithOpcode(OP_var);
  P_SetExprVarName(expr, name);
  P_SetExprVarKey(expr, key);
  P_SetExprType(expr, type);
  P_SetExprParentStmt (expr, dummy_stmt);
  return expr;
}

static Expr
NewCastExpr (Type type, Expr op1)
{
  Expr expr;

  expr = P_NewExprWithOpcode(OP_cast);
  expr = P_AppendExprOperands(expr, op1);   
  P_SetExprType(expr, type); 
  P_SetExprParentStmt (expr, dummy_stmt);
  return expr;
}

static Expr
NewDualOpExpr(_Opcode opcode, Expr op1, Expr op2)
{
  Expr expr;

  expr = P_NewExprWithOpcode(opcode);
  expr = P_AppendExprOperands(expr, op1);
  expr = P_AppendExprOperands(expr, op2);
  PSI_CastExpr(expr);
  P_SetExprParentStmt (expr, dummy_stmt); 
  return expr;
}

static Expr
NewSingleOpExpr(_Opcode opcode, Expr op1)
{
  Expr expr;

  expr = P_NewExprWithOpcode(opcode);
  expr = P_AppendExprOperands(expr, op1);
  PSI_CastExpr(expr);
  P_SetExprParentStmt (expr, dummy_stmt); 
  return expr;
}

static Expr
ReduceExpr (Expr expr)
{
  Expr new_expr;

  new_expr = PSI_ReduceExpr(expr);
  P_SetExprParentStmt (new_expr, dummy_stmt); 
  return new_expr;
}


static bool
IsExitBlock(PC_Block bb, PC_Loop loop)
{
  PC_Flow succ_flow;
  PC_Block succ_bb; 

  /*
   * This is inefficient. The reason for doing this is because the 'exits' field of PC_Loop
   * actually records 'out' block.
   */
  for (succ_flow = bb->s_flow; succ_flow; succ_flow = succ_flow->s_next_flow) {
    succ_bb = succ_flow->dest_bb;
    if (Set_in(Get_Loop_exits(loop), succ_bb->ID))
      return 1;
  }
  return 0;
}
