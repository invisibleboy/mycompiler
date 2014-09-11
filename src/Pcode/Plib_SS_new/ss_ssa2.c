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
/*===========================================================================
 *
 *      File :          ss_ssa2.c
 *      Description :   Pcode SSA Module
 *      Creation Date : September 27, 2004 
 *      Author :        James Player
 *
 * The functions and structures contained in this file implement an SSA
 * representation in Pcode.  The algorithms for placing the PHI nodes and
 * subscripting the variables are based on a paper entitled "An Efficient
 * Method of Computing Static Single Assignment Form" by Cytron et al.
 *
 * Notes:
 * 
 *===========================================================================*/
#include <string.h>
#include <Pcode/symtab.h>
#include <Pcode/cfg.h>
#include <Pcode/dom.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include "ss_ssa2.h"

typedef struct _SSA_Flags
{
  int Work;
  int DomFronPlus;
  
  void *ext;
}
_SSA_Flags, *SSA_Flags;


static PSS_BaseTbl Collect_Vars (PC_Graph cfg);
static int AddrTaken (Expr expr);

static void Place_Phi_Nodes (PC_Graph cfg, PSS_BaseTbl table);
static void Place_Phi_for_Var (PC_Graph cfg, PSS_Base base);
static void Insert_Phi_To_BB (PC_Graph cfg, VarDcl vdcl, PC_Block bb);

static void Compute_Subscripts (PC_Block bb);
static int Which_Pred (PC_Block bb, PC_Block pred_bb);

static void Prune_Phi_Nodes (PC_Graph cfg);
static void Remove_Unused_Defs (PSS_BaseTbl table);

static SSA_Flags New_SSA_Flags (void *ext);
static void* Free_SSA_Flags (SSA_Flags fl);

static int Is_PhiMu (PSS_Def phi_ssa_def);
static void Convert_Phis_To_Mus (PSS_BaseTbl LocalVars); 


/*! \brief Construct a pruned SSA graph for a CFG.  Convert any PHI nodes
 *   found in loop headers to MU nodes.
 *
 * \param cfg
 *  CFG to process.
 *
 * \return
 *  Table of local variables which were processed.
 */
PSS_BaseTbl
PSS_ComputeSSA (PC_Graph cfg)
{
  PSS_BaseTbl LocalVars = Collect_Vars (cfg);
 
  if (LocalVars && LocalVars->num_valid > 0)
    {
      Place_Phi_Nodes (cfg, LocalVars);
      Compute_Subscripts (cfg->first_bb);
      
      Prune_Phi_Nodes (cfg);
      Remove_Unused_Defs (LocalVars);
      
      Convert_Phis_To_Mus (LocalVars);
    }

  return LocalVars;
}


PSS_BaseTbl
PSS_DeleteSSA (PC_Graph cfg, PSS_BaseTbl baseTbl)
{
  PC_Block bb;
  Expr expr;
  _PC_ExprIter ei;
  
  for (bb = cfg->first_bb; bb; bb = bb->next)
    for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
	 expr = PC_ExprIterNext (&ei, 1))
      {
	List varList = NULL, phiList = NULL;
	Expr phi, var;
	
	/* Remove all PHI nodes from the code. */
	List_start (phiList = PSS_GetSubExprByOpcode_List (expr, OP_phi));
	while ((phi = List_next (phiList)))
	  {

	  }

	List_reset (phiList);

	/* Cleanup SSA fields on any variables. */
	List_start (varList = PSS_GetSubExprByOpcode_List (expr, OP_var));
	while ((var = List_next (varList)))
	  {

	  }

	List_reset (varList);
      }

  /* Delete the base table. */
  return PSS_BaseTbl_Free (baseTbl);
}

static int
Is_Vararg_List (VarDcl vdcl)
{
  int ret = 0;
  Pragma pragma;

  if (!vdcl)
    return 0;

  for (pragma = vdcl->pragma; pragma; pragma = pragma->next)
    if (!strcmp (pragma->specifier, "__builtin_va_list"))
      {
	ret = 1;
	break;
      }
#if 1 
  return ret;
#else
  return 0;
#endif
}

#define IsDummyVariable(name) ((name)[0] == '@')

/*! \brief Create a table of local variables.
 *
 *  Traverses the CFG examining OP_var instances.  If the variable is local,
 *  a PSS_Base is created and entered into a PSS_BaseTbl.  A blank
 *  PSS_Def is also created for each local variable on the LHS of an
 *  OP_assign.
 *
 * \param cfg
 *  Input CFG.
 *
 * \return
 *  A table of PSS_Base structures containing an entry for every local
 *  variable in the CFG.
 */
static PSS_BaseTbl
Collect_Vars (PC_Graph cfg)
{
  PC_Block bb;
  _PC_ExprIter ei;
  Expr expr;
  PSS_BaseTbl table = NULL;

  for (bb = cfg->first_bb; bb; bb = bb->next)
    for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
	 expr = PC_ExprIterNext (&ei, 1))
      {
	List varList = NULL;
	Expr var;
	List_start (varList = PSS_GetSubExprByOpcode_List (expr, OP_var));
	while ((var = List_next (varList)))
	  {
	    PSS_Base base;
	    VarDcl vdcl;

	    /* Don't process the variable if it has no declaration,
	     * is a global, or a dummy variable. */
	    if (!(vdcl = PSI_GetVarDclEntry (var->value.var.key)) ||
		(vdcl->qualifier & VQ_GLOBAL) || IsDummyVariable (vdcl->name) ||
		Is_Vararg_List (vdcl))
	      {
		var->value.var.ssa = NULL;
		continue;
	      }
	      
	    if (!(base = PS_GetSSABaseEntry (vdcl)))
	      {
		base = PSS_Base_New (vdcl, NULL);
		PS_SetSSABaseEntry (vdcl, base);

		table = PSS_BaseTbl_Insert (table, base);

		table->num_valid++;
	      }
	    
	    if (!base->addr_taken && !AddrTaken (var))
	      {
		if (PSS_VarIsUse (var))          /* var is Use */
		  var->value.var.ssa = NULL;
		
		else                             /* var is Def */
		  {
		    PSS_Def new_def = PSS_Def_New (var, bb, NORMAL);
		    var->value.var.ssa = new_def;

		    PSS_AddDefToBase (new_def, base);
		  }
	      }
	    
	    else if (!base->addr_taken)
	      {
		PSS_Def def, next;
		Expr var;
		
		base->addr_taken = 1;
		table->num_valid--;

		/* clean up any defs already linked into the CFG */
		for (def = base->defs; def; def = next)
		  {
		    next = def->next;
		    
		    if ((var = def->var))
		      var->value.var.ssa = NULL;

		    PSS_RemoveDefFromBase (def, base);
		  }
	      }
	    
	  } /* while (var = List_next (varList)) */

	varList = List_reset (varList);
      }
  
  return table;
}


/*! \brief Examines an expression and tells whether its address is taken.
 *
 * \param expr
 *  Expresion to check.
 *
 * \return
 *  1 if the address of expr is taken, 0 otherwise.
 */
static int
AddrTaken (Expr expr)
{
  int ret;
  Expr parent = expr->parentexpr;
  
  if (parent == NULL)
    return 0;

  switch (parent->opcode)
    {
      case OP_addr:
        ret = 1;
        break;

      case OP_cast:
	ret = AddrTaken (parent);
	break;

      case OP_index:
	ret = parent->operands == expr ? AddrTaken (parent) : 0;
	break;
	
      default:
	ret = 0;
	break;
    }

  return ret;
}


/*! \brief Place PHI nodes for all address-not-taken local variables in the
 *   CFG.
 *
 * \param cfg
 *  CFG to process.
 *
 * \param table
 *  Table of local variables.
 */
static void
Place_Phi_Nodes (PC_Graph cfg, PSS_BaseTbl table)
{
  PSS_Base base;
  PC_Block bb;

  if (!table) return;

#if 1
  PC_FindDominatorFrontier (cfg);
#else
  PC_BuildDomFront (cfg);
#endif
  
  for (bb = cfg->first_bb; bb; bb = bb->next)
    bb->ext = New_SSA_Flags (bb->ext);

  for (base = table->first; base; base = base->next)
    if (!base->addr_taken) Place_Phi_for_Var (cfg, base);

  for (bb = cfg->first_bb; bb; bb = bb->next)
    bb->ext = Free_SSA_Flags (bb->ext);
}


#define WORK(a)        (((SSA_Flags)(a)->ext)->Work)
#define DOMFRONPLUS(a) (((SSA_Flags)(a)->ext)->DomFronPlus)

/*! \brief Place PHI nodes for a single variable in BBs that lie on the
 *   dominance of all the BBs in which the variable is assigned to.
 *
 * \param cfg
 *  CFG to place PHI nodes into.
 *
 * \param base
 *  PSS_Base structure of a variable to place PHI nodes for.
 */
static void
Place_Phi_for_Var (PC_Graph cfg, PSS_Base base)
{
  PC_Block bb, df_bb;
  PSS_Def def;
  List W = NULL; /* Worklist of CFG nodes that are being processed. */
  
  /* Initialize DomFronPlus and Work for the current variable */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      DOMFRONPLUS(bb) = 0;
      WORK(bb) = 0;
    }

  /* For each assignment to base, set the work bit for the corresponding
   * bb and add the bb to the work list. */
  for (def = base->defs; def; def = def->next)
    if (!UNINITIALIZED_TYPE (def->type))
      {
	WORK(def->bb) = 1;
	W = List_insert_last (W, def->bb);
      }

  List_start (W);
  while ((bb = List_next (W)))
    {
      List_start (bb->dom_frontier);
      while ((df_bb = List_next (bb->dom_frontier)))
	if (!DOMFRONPLUS(df_bb))
	  {
	    /* Add PHI node for the current variable to df_bb */
	    Insert_Phi_To_BB (cfg, base->vdcl, df_bb);
	    
	    DOMFRONPLUS(df_bb) = 1;
	    if (!WORK(df_bb))
	      {
		W = List_insert_last (W, df_bb);
		WORK(df_bb) = 1;
	      }
	  }
    }
}


/*! \brief Insert a PHI node to the top of a BB.
 * 
 *  Given a variable declaration and a BB, insert a PHI node defining an
 *  instance of the declared variable.  The PHI will have a list of OP_var's
 *  attached to it which correspond to the live definitions from predecessor
 *  BBs.  The PHI node returns a value up to the OP_assign node and stored
 *  into an new instance of the declared variable.  The Pcode structure of
 *  the overall PHI definition looks like this:
 *
 * \verbatim
 *  OP_assign
 *     |
 *     |operands
 *     v
 *   OP_var--------->OP_phi
 *          sibling    |
 *                     |operands
 *                     v
 *                   OP_var------->OP_var----...
 *                           next          next
 * \endverbatim
 *
 * Once this structure is constructed, it is inserted at the beginning of
 * the BB handed into the function.
 *
 * \param cfg
 *  Current CFG
 *
 * \param vdcl
 *  Variable declaration of the variable to be defined by the PHI node.
 *
 * \param bb
 *  BB to insert the PHI node to.
 */
static void
Insert_Phi_To_BB (PC_Graph cfg, VarDcl vdcl, PC_Block bb)
{
  Expr assign, lhs, phi;
  PC_Flow pred;
  PSS_Base base;
  Set bb_set = Set_new ();
  int i;

  assign = P_NewExprWithOpcode (OP_assign);
  assign->id = PST_FuncDclNextExprID(cfg->func);
  P_SetExprType (assign, vdcl->type);
  
  lhs = P_NewExprWithOpcode (OP_var);
  lhs->id = PST_FuncDclNextExprID(cfg->func);
  P_SetExprVarName (lhs, strdup (vdcl->name));
  P_SetExprVarKey (lhs, vdcl->key);
  P_SetExprType (lhs, vdcl->type);

  P_AppendExprOperands (assign, lhs);
  
  phi = P_NewExprWithOpcode (OP_phi);
  phi->id = PST_FuncDclNextExprID(cfg->func);
  P_SetExprType (phi, vdcl->type);

  P_AppendExprOperands (assign, phi);

  /* Count the number of unique BB ids that precede this BB */
  for (pred = bb->p_flow; pred; pred = pred->p_next_flow)
    bb_set = Set_add (bb_set, pred->src_bb->ID);

  /* Create operand variables for the PHI function */
  for (i = 0; i < Set_size (bb_set); i++)
    {
      Expr newVar = P_NewExprWithOpcode (OP_var);
      newVar->id = PST_FuncDclNextExprID(cfg->func);
      P_SetExprVarName (newVar, strdup (vdcl->name));
      P_SetExprVarKey (newVar, vdcl->key);
      P_SetExprType (newVar, vdcl->type);

      newVar->value.var.ssa = NULL;
      newVar->next          = phi->operands;
      newVar->parentexpr    = phi;
      if (phi->operands) phi->operands->previous = newVar;
      phi->operands         = newVar;
    }
  bb_set = Set_dispose (bb_set);

  lhs->value.var.ssa = PSS_Def_New (lhs, bb, PHI);

  base = PS_GetSSABaseEntry(vdcl);

  PSS_AddDefToBase (lhs->value.var.ssa, base);

  PC_NewPStmtExprBefore (PC_T_Expr, bb, bb->first_ps, assign);


  return;
}


/*! \brief Tells whether a variable is being used or defined.
 *
 * \param v
 *  Varable to test.
 *
 * \return
 *  1 if v is a Use, 0 otherwise.
 */
int
PSS_VarIsUse (Expr v)
{
  int ret = 1;
  
  assert (v->opcode == OP_var);

  if (v->parentexpr && v->parentexpr->opcode == OP_assign)
    ret = v->parentexpr->operands == v ? 0 : 1;

  return ret;
}

/*! \brief Compute subscripts for all processed variable Defs and annotates
 *   Uses with Def information.
 *
 *  This function traverses the Dominator Tree in a Depth-First-Search manner.
 *  Along this traversal, the subscripts of all Defs in the current BB are
 *  calculated and Def information is annotated to all normal uses of
 *  variables.  Finally, Def information is annotated onto the operands of any
 *  PHI nodes in successor block.
 * 
 *  \param bb
 *   Current block being processed.
 */
static void
Compute_Subscripts (PC_Block bb)
{
  PC_Block domChild;
  PC_Flow succ;
  List varList = NULL;
  _PC_ExprIter ei;
  Expr expr, variable;
  PSS_Base base;

  /* Collect all the variables from this bb into a List */
  for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
       expr = PC_ExprIterNext (&ei, 1))
    {
      List subExprVarList = PSS_GetSubExprByOpcode_List (expr, OP_var);

      List_start (subExprVarList);
      while ((variable = List_next (subExprVarList)))
	{
	  VarDcl vdcl = PSI_GetVarDclEntry (variable->value.var.key);
	  base = vdcl ? PS_GetSSABaseEntry (vdcl) : NULL;

	  if (!base || base->addr_taken) continue;
	  
	  if (PSS_VarIsUse (variable))  /* variable is a Use */
	    {
	      /* Do not overwrite the ssa field if the
	       * variable is a Use in a PHI node */
	      if (!(variable->parentexpr &&
		    variable->parentexpr->opcode == OP_phi))
		{
		  PSS_Def def = Stack_Top (base->def_var_stk);

		  if (UNUSED_TYPE (def->type))
		    {
		      if (vdcl->qualifier & VQ_PARAMETER)
			SET_DEF_TYPE (def->type, PARAM);
		      else
			SET_DEF_TYPE (def->type, UNDEF);
		    }
		  
		  variable->value.var.ssa = def;
		  PSS_AddUseToDef (variable, bb);
		}
	    }
	  
	  else                            /* variable is a Def */
	    {
	      variable->value.var.ssa->subscr = ++base->assign_count;
	      Push_Top (base->def_var_stk, variable->value.var.ssa);
	    }
	}
      varList = List_append (varList, subExprVarList);
    }

  for (succ = bb->s_flow; succ; succ = succ->s_next_flow)
    {
      _PC_ExprIter ei;
      List phiList = NULL;
      Expr expr, phi;
      int pred_num = Which_Pred (succ->dest_bb, bb);

      assert (pred_num >= 0);
      
      /* Write the operands of all the PHI nodes in succ */
      
      for (expr = PC_ExprIterFirst (succ->dest_bb, &ei, 1); expr;
	   expr = PC_ExprIterNext (&ei, 1))
	{
	  phiList = PSS_GetSubExprByOpcode_List (expr, OP_phi);
	 
	  List_start (phiList);
	  while ((phi = List_next (phiList)))
	    {
	      int i;
	      PSS_Def def;
	      Expr phi_operand = phi->operands;
	      VarDcl vdcl = PSI_GetVarDclEntry (phi->operands->value.var.key);
	      base = PS_GetSSABaseEntry (vdcl);
	      
	      for (i = 0; i < pred_num; i++)
		assert (phi_operand = phi_operand->next);
	     
	      assert (phi_operand->opcode == OP_var);
	      assert (def = Stack_Top (base->def_var_stk));

	      if (UNUSED_TYPE (def->type))
		{
		  VarDcl vdcl =
		    PSI_GetVarDclEntry (phi_operand->value.var.key);

		  if (vdcl->qualifier & VQ_PARAMETER)
		    SET_DEF_TYPE (def->type, PARAM);
		  else
		    SET_DEF_TYPE (def->type, UNDEF);
		}

	      phi_operand->value.var.ssa = def;
	      PSS_AddUseToDef (phi_operand, succ->dest_bb);
	    }
	  phiList = List_reset (phiList);
	}
    }

  /* Process all the successor bb's */
  List_start (bb->dom_tree_children);
  while ((domChild = List_next (bb->dom_tree_children)))
    Compute_Subscripts (domChild);

  /* Pop the SSA Defs for all the simple assignments in this bb */
  List_start (varList);
  while ((variable = List_next (varList)))
    if (!PSS_VarIsUse (variable))
      {
	VarDcl vdcl;

	/* The variable must have a declaration and a valid SSA Base */
	if (!(vdcl = PSI_GetVarDclEntry (variable->value.var.key))) continue;
	if (!(base = PS_GetSSABaseEntry (vdcl))) continue;

	assert (Pop (base->def_var_stk));
      }

  varList = List_reset (varList);

  return;
}

/*! \brief Assigns a unique integer to any predecessor block.
 *
 * \param bb
 *  Destination block of the predecessor edge being queried.
 *
 * \param pred_bb
 *  Source block of the precessor edge being queried.
 */
static int
Which_Pred (PC_Block bb, PC_Block pred_bb)
{
  Set bb_set = Set_new ();
  SetIterator *si;
  PC_Flow pred;
  int pred_bb_id, i = 0;

  for (pred = bb->p_flow; pred; pred = pred->p_next_flow)
    bb_set = Set_add (bb_set, pred->src_bb->ID);

  si = Set_iterator (bb_set, -1);
  while ((pred_bb_id = Set_next (si)) != -1)
    if (pred_bb->ID == pred_bb_id)
      {
	si = Set_end_iterator (si);
	bb_set = Set_dispose (bb_set);
	return i;
      }
    else
      i++;

  si = Set_end_iterator (si);
  bb_set = Set_dispose (bb_set);
  
  return -1;
}


/*! \brief Deletes PHI nodes that define variables which are never used.
 *
 *   This function initializes a worklist of all the BBs in the CFG.  It then
 *   traverses the worklist, removing any PHI nodes which define a variable with
 *   no PSS_Use attached to its PSS_Def.  When it removes a PHI node, it
 *   removes the PSS_Use information from the PSS_Defs of all the PHI
 *   operands and checks each of those PSS_Def structures for an empty Use
 *   list.  If a new empty Use list is uncovered, the Def's BB is added to the
 *   end of the worklist.
 * 
 *  \param cfg
 *   CFG to prune.
 */
static void
Prune_Phi_Nodes (PC_Graph cfg)
{
  PC_Block bb;
  PC_PStmt ps, next_ps;
  List bbList = NULL;
  
  for (bb = cfg->first_bb; bb; bb = bb->next)
    bbList = List_insert_first (bbList, bb);
  
  List_start (bbList);
  while ((bb = List_next (bbList)))
    {
      bbList = List_delete_current (bbList);

      if (bb->p_flow == NULL) continue;

      for (ps = bb->first_ps; ps; ps = next_ps)
	{
	  Expr expr, phi;
	  List phis;
	  
	  next_ps = ps->succ;
	  if (ps->type != PC_T_Expr) continue;
	  
	  expr = ps->data.stmt->stmtstruct.expr;
	  phis = PSS_GetSubExprByOpcode_List (expr, OP_phi);

	  List_start (phis);
	  while ((phi = List_next (phis)))
	    {
	      Expr def_var = phi->parentexpr->operands;
	      assert (def_var->opcode == OP_var);

	      if (def_var->value.var.ssa &&
		  def_var->value.var.ssa->uses == NULL)
		{
		  Expr use_var;
		  VarDcl vdcl = PSI_GetVarDclEntry (def_var->value.var.key);
		  PSS_Base base = PS_GetSSABaseEntry (vdcl);

		  /* Remove the Def and Uses */
		  PSS_RemoveDefFromBase (def_var->value.var.ssa, base);
		  for (use_var = phi->operands; use_var;
		       use_var = use_var->next)
		    {
		      PSS_Def def = use_var->value.var.ssa;

		      if (def)
			{
			  PSS_RemoveUseFromDef (use_var);
			  
			  if (def->uses == NULL && MERGE_TYPE (def->type)
			      && !List_member (bbList, def->bb))
			    bbList = List_insert_last (bbList, def->bb);
			}
		    }

		  /* Remove the stmt */
		  assert (List_size (phis) == 1);
		  PC_DeletePStmt (bb, ps);
		}
	    }
	}
    }
  
  bbList = List_reset (bbList);
}


static void
Remove_Unused_Defs (PSS_BaseTbl table)
{
  PSS_Base base;
  
  for (base = table->first; base; base = base->next)
    {
      PSS_Def def;
      
      for (def = base->defs; def; def = def->next)
	if (UNUSED_TYPE (def->type))
	  PSS_RemoveDefFromBase (def, base);
    }
}


/*! \brief Re-numbers the subscripts so that values start from 'first_subscr'.
 *
 *   This function traverses over the list of local variables.  For each
 *   variable, it identifies the definition (with valid uses) that has the
 *   lowest subscript value.  Then, it adds a correction factor to all
 *   subscripts of that variable to normalize the beginning subscript to
 *   'first_subscr'.
 * 
 *  \param table
 *   Table of variables local to the function being processed.
 *
 *  \first_subscr
 *   Subscript number to normalize all variables to.
 */
void
PSS_NormalizeSubscrs (PSS_BaseTbl table, int first_subscr)
{
  PSS_Base base;

  if (table == NULL) return;

  for (base = table->first; base; base = base->next)
    if (!base->addr_taken)
      {
	PSS_Def def = base->defs;
	int bottom = def->name;
	
	for (def = def->next; def; def = def->next)
	  if (def->name < bottom && !UNUSED_TYPE (def->type))
	    if (!(UNDEF_TYPE (def->type) && def->uses == NULL))
	      bottom = def->name;

	if (bottom != first_subscr)
	  {
	    int correction = first_subscr - bottom;

	    for (def = base->defs; def; def = def->next)
	      def->name += correction;
	  }
      }
}


static SSA_Flags
New_SSA_Flags (void *ext)
{
  SSA_Flags fl = ALLOCATE (_SSA_Flags);

  fl->Work = 0;
  fl->DomFronPlus = 0;

  fl->ext = ext;

  return fl;
}

static void*
Free_SSA_Flags (SSA_Flags fl)
{
  void *ext = fl->ext;
  
  DISPOSE (fl);

  return ext;
}


static List
GetSubExprByOpcode_List_Rec (Expr expr, _Opcode opcode, List lst)
{
  for (; expr; expr = expr->next)
    {
      lst = GetSubExprByOpcode_List_Rec (expr->operands, opcode, lst);
      lst = GetSubExprByOpcode_List_Rec (expr->sibling, opcode, lst);

      if (!opcode || expr->opcode == opcode)
	{
	  lst = List_insert_last(lst, expr);
	}
    } /* for "next" expressions */
  return lst;
}

List
PSS_GetSubExprByOpcode_List (Expr expr, _Opcode opcode)
{
  return GetSubExprByOpcode_List_Rec (expr, opcode, NULL);
}

/*
 * determines if a phi node is a mu node
 */
static int
Is_PhiMu(PSS_Def phi_ssa_def)
{
  Expr phi_operand_expr, assign_to_expr;
  Expr expr;     /* OP_phi */

  if (!PHI_TYPE (phi_ssa_def->type))
    return 0;
  
  /* get the expr of the phi node */
  expr = phi_ssa_def->var->sibling;

  if (expr->opcode != OP_phi)
    return 0;

  if ((assign_to_expr = expr->parentexpr->operands)->opcode != OP_var)
    {
      P_punt("Is_PhiMu: illegal expression tree detected");
    }

  /* mu nodes always have backedges, which are just defs
   * that have higher subdscripts */
  for (phi_operand_expr = expr->operands; phi_operand_expr;
       phi_operand_expr = phi_operand_expr->next)
    {
#if 0
      if ((phi_operand_expr->value.var.ssa) &&
	  (phi_operand_expr->value.var.ssa->subscr > 
	   assign_to_expr->value.var.ssa->subscr))
#else
      PSS_Def operand_ssa_def;

      operand_ssa_def = phi_operand_expr->value.var.ssa;

      if (phi_ssa_def->bb && operand_ssa_def->bb &&
	  (PC_BB1DominatesBB2(phi_ssa_def->bb, operand_ssa_def->bb)))
#endif
	return 1;
    }
  /* should have short circuitted out if we found a backedge */
  return 0;
}

/*
 * scan through the ssa graph, and identify phi nodes that are
 * mu nodes
 */
static void 
Convert_Phis_To_Mus(PSS_BaseTbl varTbl)
{
  PSS_Base base;
  PSS_Def def;

  if (!varTbl)
    return;
  
  for (base = varTbl->first; base; base = base->next)
    for (def = base->defs; def; def = def->next)
      if (Is_PhiMu(def))
	SET_DEF_TYPE (def->var->value.var.ssa->type, MU);
}


/* \brief Scans through operands to see if any of them are loop-variant.
 *
 * This task is done by seeing if an operand gets its def from a mu-function
 * within the loop.  Anything without an SSA def or a memory access is
 * conservatively assumed to be loop-variant.
 */
int
PSS_Is_Loop_Invariant (Expr expr, PC_Loop loop)
{
  Expr operand;
  PSS_Def ssa_def;

  if (loop == NULL)
    P_punt ("PSS_Is_Loop_Invariant:  need loop context.");

  /* Recursive call to operands. */
  switch (expr->opcode)
    {
    case OP_assign:
      operand = expr->operands->sibling;
      return (PSS_Is_Loop_Invariant(operand, loop));
    case OP_var:
      /* Get SSA def, if none found return conservative (0) result.
	 Otherwise check for mu-function def within the loop. */
      ssa_def = expr->value.var.ssa;
      if (ssa_def == NULL)
	return 0;
      switch (ssa_def->type)
	{
	case UNDEF:
	  return 0;
	case PARAM:
	  return 1;
	case MU:
	  /* Moment of truth: if mu is inside the loop, variant. */
	  if (Set_in (Get_Loop_body(loop), ssa_def->bb->ID))
	    return 0;
	  else  /* otherwise, not variant */
	    return 1;
	default:
	  return (PSS_Is_Loop_Invariant (ssa_def->var->parentexpr, loop));
	}
      break;
    case OP_arrow:
    case OP_indr:
    case OP_addr:
    case OP_index:
    case OP_call:
    case OP_sync:
      /* For the above cases, conservatively return 0. */
      return 0;
    default:
      for (operand = expr->operands; operand; operand = operand->sibling)
	{
	  Expr list_operand;
	  if (!(PSS_Is_Loop_Invariant(operand, loop)))
	    return 0;
	  for (list_operand = operand->next; list_operand;
	       list_operand = list_operand->next)
	    if (!(PSS_Is_Loop_Invariant(list_operand, loop)))
	      return 0;
	}
    }
  return 1;
}
