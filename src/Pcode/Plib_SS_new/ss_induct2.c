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

#include "ss_induct2.h"

#define DEBUG_IND

/*! \def MIN(A,B)
 * \brief returns the minimum of A and B 
 */
#define MIN(A, B) ((A < B) ? A : B )

/* memory functions */
static PSS_TarNode New_TarNode (PSS_TarLoop tloop, Expr expr);
#if 0
static void Free_TarNode(PSS_TarNode tnode);
#endif
static PSS_TarLoop New_TarLoop(PC_Loop pcloop);
#if 0
static void Free_TarLoop(PSS_TarLoop tloop);
#endif
static PSS_TarSCC New_TarSCC(PSS_TarLoop tloop);
#if 0
static void Free_TarSCC(PSS_TarSCC scc);
#endif

/* initilization */
static void Find_SCCs(PC_Loop pcloop);
static PSS_TarLoop Initialize_Tarjan (PC_Loop pcloop);

/* algo functions */
static void Find_Components(PSS_TarLoop tloop);
static void Classify_Trivial(PSS_TarLoop tloop, PSS_TarNode tnode);
static void Visit_Node (PSS_TarLoop tloop, PSS_TarNode tnode);
static int Visit_Descendent (PSS_TarLoop tloop, PSS_TarNode tnode);

/* classification functions */
static int Classify_Ignore(PSS_TarSCC scc);
static int Classify_Linear(PSS_TarSCC scc);
static int Linear_Expr_Test(PSS_TarSCC scc, Expr expr, int *);
static int Classify_Polynomial(PSS_TarSCC scc);
static int Poly_Expr_Test(PSS_TarSCC scc, Expr expr, int *);
static int Classify_Geometric(PSS_TarSCC scc);
static int Geom_Expr_Test(PSS_TarSCC scc, Expr expr, int *);
static int Classify_Pointer(PSS_TarSCC scc);
static int Pointer_Expr_Test(PSS_TarSCC scc, Expr expr, int *);

/* helper functions */
static PSS_TarNode Find_TarNode_Expr (PSS_TarLoop tloop, Expr expr);
#if 0
static PSS_TarNode Find_TarNode_ID (PSS_TarLoop tloop, int id);
#endif
static void Add_Node_To_Scc(PSS_TarNode tnode, PSS_TarSCC scc);
static PSS_TarSCC Find_Expr_SCC(PSS_TarLoop tloop, Expr expr, 
				  PSS_TarSCC_Type type);

/* debug functions */
static void Print_TarNode (PSS_TarNode tnode);
void PSS_Print_TarLoop (PSS_TarLoop tloop);
void PSS_PrintSCC(PSS_TarSCC scc);
static void Counter_Clear_SCC_Types();
static void Counter_Print_SCC_Types();

/* globals */

/* var to keep track of current cfg */
PC_Graph cfg;

FILE *debug_file_id;
FILE *stat_file_id;

/*! \var struct STATS
 * \briefstruct to keep track of numbers of statistics
 */
static struct 
{
  struct
    {
      int linear;
      int linear_monotonic;
      int polynomial;
      int polynomial_monotonic;
      int pointer;
      int pointer_monotonic;
      int unknown;
    } scc_type;
} STATS;

/**********************************************
 * TARJAN ALGO FUNCTIONS
 **********************************************/
/* MEMORY ALLOCATION FUNCTIONS */

static PSS_TarNode
New_TarNode (PSS_TarLoop tloop, Expr expr)
{
  PC_Loop inner_loop;
  PSS_TarNode tnode;

  inner_loop = PC_ExprInnerLoop(tloop->cfg, expr);
#if 1
  if (inner_loop != tloop->pcloop)
    return NULL;
#endif

  if (!(expr->operands->value.var.ssa))
    return NULL;
  
  tnode = ALLOCATE (_PSS_TarNode);
  
  tnode->lowlink = -1;
  tnode->id = tloop->count++;
  tnode->status = NOTYET;
  
  tnode->tloop = tloop;

  if (expr->opcode != OP_assign)
    P_punt ("New_TarNode: nodes must correspond to assign operations");

  tnode->expr = expr;

  /* determine the inner loop for this node */
  tnode->inner_loop = inner_loop;

  /* update tloop and list structure */
  if (tloop->first == NULL)
    {
      tloop->first = tloop->last = tnode;
      tnode->prev = tnode->next = NULL;
    }
  else
    {
      if (tloop->last == NULL)
	P_punt("Bad List, First != NULL && Last == NULL");

      if (tloop->last->next != NULL)
	P_punt("Bad List: Last entry in list has \"next\" value");

      tnode->prev = tloop->last;
      tnode->next = NULL;
      tnode->prev->next = tnode;
      tloop->last = tnode;
    }
  
  return tnode;
}

/*
 * free a tarnode -> note that this does not fix up any
 * of of the linked-lists of tnode's, and should only
 * be used when free'in an entire tarloop
 */
#if 0
static void
Free_TarNode(PSS_TarNode tnode)
{
  DISPOSE (tnode);
}
#endif

static PSS_TarLoop
New_TarLoop(PC_Loop pcloop)
{
  PSS_TarLoop tloop = ALLOCATE (_PSS_TarLoop);

  tloop->cfg = cfg;
  tloop->pcloop = pcloop;
  tloop->number = 0;
  tloop->count = 0;
  tloop->node_stack = New_Stack();
  
  tloop->first = NULL;
  tloop->last  = NULL;
  tloop->sccs  = NULL;
  tloop->scc_count = 0;

  return tloop;
}

#if 0
static void
Free_TarLoop(PSS_TarLoop tloop)
{
  PSS_TarNode tnode;
  
  /* free the node_stack -- we dont have to free the elements
   * cause we will take care of that later using the linked-list */
  Free_Stack(tloop->node_stack);
  
  for (tnode = tloop->first; tnode; tnode = tnode->next)
    Free_TarNode(tnode);
 
  DISPOSE (tloop);
}
#endif

/* 
 * new SCC
 */
static PSS_TarSCC
New_TarSCC(PSS_TarLoop tloop)
{
  PSS_TarSCC scc = ALLOCATE (_PSS_TarSCC);
  
  scc->id = tloop->scc_count++;

  scc->type = UNKNOWN;
  scc->tloop = tloop;
  scc->prev = NULL;

  scc->mu_nodes = 0;
  scc->phi_nodes = 0;

  scc->tnode_list = NULL;

  scc->var_name = NULL;

  /* add scc to tloop scc list */
  if (tloop->sccs) /* if not the first scc */
    {
      tloop->sccs->prev = scc;
      scc->next = tloop->sccs;
    }
  else /* first scc */
    {
      scc->next = NULL;
    }
  tloop->sccs = scc;

  return scc;
}

#if 0
static void
Free_TarSCC(PSS_TarSCC scc)
{
  /* first check to make sure it is not the first scc in the tloop's list */
  if (scc->tloop->sccs == scc)
    {
      /* sanity check */
      if (scc->prev != NULL)
	P_punt("Free_TarSCC: Malformed SCC List");
      
      /* make sure we are not the last element in the list */
      if (scc->next)
	{
	  scc->next->prev = NULL;
	}
      scc->tloop->sccs = scc->next;
    }
  else if (scc->next == NULL) /* last element in the list */
    {
      scc->prev->next = NULL;
    }
  else /* middle element */
    {
      /* sanity check */
      if (!(scc->next) || !(scc->prev))
	P_punt("Free_TarSCC: Malformed SCC List");

      scc->prev->next = scc->next;
      scc->next->prev = scc->prev;
    }

  List_reset(scc->tnode_list);

  DISPOSE(scc);
}
#endif

/********** END MEMORY FUNCTIONS *****************************/

/* TARGAN ALGO FUNCTIONS */
static PSS_TarLoop
Initialize_Tarjan (PC_Loop pcloop)
{
  PSS_TarLoop tloop;
  int *bb_ids;
  int bb_ids_size, i;
  
  /* create the control structure */
  tloop = New_TarLoop(pcloop);

  /* loop through the ops in the loop and generate 
   * tarnodes for each of them */
  bb_ids = (int *) calloc (Set_size (pcloop->body), sizeof (int));
  bb_ids_size = Set_2array (pcloop->body, bb_ids);
  for (i = 0; i < bb_ids_size; i++)
    {
      PC_Block bb;
      Expr expr;
      _PC_ExprIter ei;
      bb = PC_FindBlock (cfg, bb_ids[i]);

      for (expr = PC_ExprIterFirst(bb, &ei, 1); expr;
	   expr = PC_ExprIterNext(&ei, 1))
	{
	  /* we only care about assign ops, cause that is where
	   * the action is */
#if 1
	  if (expr->opcode == OP_assign && expr->operands->opcode == OP_var)
#else
	  /* need to filter out meaningless phi nodes */
	  if (expr->opcode == OP_assign && expr->operands->opcode == OP_var &&
	      (expr->operands->sibling->opcode != OP_phi || 
	       expr->operands->value.var.ssa->uses))
#endif
	    {
	      New_TarNode(tloop, expr);
	    } /* if assign op */
	} /* for expr in bb */
    } /* for bb's in loop */
  return tloop;
}

/*! \brief classify the types of all the SCCs in a loop
 *
 * \param tloop
 *   the loop that we are evaluating
 *
 * This function will go through all the SCC's in a loop and attempt to 
 * determine what the type of each SCC is. It is necessary to first
 * classify all of some types of induction vars before classifying
 * others.  For example, one needs to find all of the linear IV's before
 * trying to identify polynomial IV's.  This can be further complicated
 * by the fact that it may be necessary to identify one SCC before another
 * of the same type.
 *
 * this algo could be somewhat improved by using a worklist, but instead
 * we will take care of it by only processing SCCs that are unknown.
 */
static void
Classify_Components(PSS_TarLoop tloop)
{
  PSS_TarSCC scc;
  int change;

  /* ignore IV's
   *   - these are IV's that are just lame */
  for (scc = tloop->sccs; scc; scc = scc->next)
    {
      Classify_Ignore(scc);
    }
  
  /* linear IV's */
  for (scc = tloop->sccs; scc; scc = scc->next)
    {
      if (scc->type == UNKNOWN)
	Classify_Linear(scc);
    }

  /* Polynomial
   *   this case is slightly more complicated than linear because polynomial
   *   IV's can be made up of other polynomial IVs.  As such, we have to
   *   continue to process through the list as long as things change
   */
  change = 1;
  while (change)
    {
      change = 0;
      for (scc = tloop->sccs; scc; scc = scc->next)
	{
	  if (scc->type == UNKNOWN)
	    {
	      change = change + Classify_Polynomial(scc);
	    }
	}
    } /* while SCCs are being classified */
  
  /* geomtric 
   *   this case is another easy one (at this level) because it cannot
   *   be made up of its own type
   */
  for (scc = tloop->sccs; scc; scc = scc->next)
    {
      if (scc->type == UNKNOWN)
	Classify_Geometric(scc);
    }

  /* pointer SCC */
  for (scc = tloop->sccs; scc; scc = scc->next)
    {
      if (scc->type == UNKNOWN)
	Classify_Pointer(scc);
    }

  /* print out all the SCC's that are still unknown */
  for (scc = tloop->sccs; scc; scc = scc->next)
    if (scc->type == UNKNOWN)
      {
	PSS_PrintSCC(scc);
	STATS.scc_type.unknown++;
      }
}

/*
 * rcursive helper function for PSS_Find_SCCs
 */
static void
Find_SCCs(PC_Loop pcloop)
{
  PSS_TarLoop tloop;

  /* loop through all the loops at this level */
  for (; pcloop; pcloop = pcloop->sibling)
    {
      /* we want to generate summaries for the inner loops first */
      Find_SCCs(pcloop->child);

      /* initialize the structures */
      tloop = Initialize_Tarjan(pcloop);

      if (!pcloop->ext)
	pcloop->ext = (void *) malloc (sizeof(_PSS_LoopExt));
      P_LoopTarjanLoop(pcloop) = tloop;

      /* identify the SCCs */
      Find_Components(tloop);
      
      /* determine what the types of the SCC's are */
      Classify_Components(tloop);

#ifdef DEBUG_IND
	PSS_Print_TarLoop(tloop);
#endif
      /* anotate results onto pcloop */
    }
}


extern void
PSS_Find_SCCs(PC_Graph new_cfg)
{
  cfg = new_cfg;
  
  debug_file_id = fopen(DEBUG_FILE, "a");
  stat_file_id = fopen(STAT_FILE, "a");
  
  Counter_Clear_SCC_Types();
  /* scan through all the natural loops in the cfg */
  Find_SCCs(cfg->lp_tree);

  Counter_Print_SCC_Types();

  fclose(debug_file_id);
  fclose(stat_file_id);
}

/*
 * see "Beyond Induction Variables" paper for details on this
 * implementation of tarjan's algo
 */
static void
Find_Components(PSS_TarLoop tloop)
{
  PSS_TarNode tnode;
  
  tloop->number = 0;

#if 0 /* this does not work cause stacks are lame */
  /* sanity check */
  if (Stack_Top(tloop->node_stack) != NULL)
    P_punt("Find_Components: stack not empty before tarjan calculations");
#endif

  /* all the ops are allocated to NOTYET to start */

  /* process all the ops */
  for (tnode = tloop->first; tnode; tnode = tnode->next)
    {
      if (tnode->status == NOTYET)
	{
	  Visit_Node(tloop, tnode);
	} /* only process if not yet processed */
    } /* for all the ops in the loop */
}

/*! \brief determine if an expression is part of an SCC in a given loop
 *
 * \param tloop
 *   The Loop whose SCCs we want to search through
 *
 * \param expr
 *   The Expression we want to search for.  This can either be of type
 *   OP_assign or OP_var
 *
 * \param type
 *   The type of SCC that you want to search for.  Use type ANY
 *   if any type of SCC will do.  You can also do | of different
 *   types as PSS_TarSCC_Type is a bitfield
 *
 * \return
 *   the linear SCC it is *first* found in or NULL if not found
 *
 * This routine searches through the SCC's in a tarloop and determines
 * if one of them (of type "type") contains the definition of the expression
 * "expr" that is passed to the function.  If one is found, the scc is
 * returned.  The expression can either be of type OP_assign or OP_var.
 * If an OP_assign is passed, it will be search for directly.  If an 
 * OP_var is passed, its SSA_Def will be found and that will be searched
 * for.  If the expr is of a different type, a warning will go off and
 * NULL will be returned.
 */
static PSS_TarSCC
Find_Expr_SCC(PSS_TarLoop tloop, Expr expr, PSS_TarSCC_Type type)
{
  PSS_TarSCC scc;
  PSS_TarNode tnode;
  Expr search_expr;

  if (expr->opcode == OP_assign)
    {
      search_expr = expr;
    }
  else if (expr->opcode == OP_var && expr->value.var.ssa)
    {
      search_expr = expr->value.var.ssa->var->parentexpr;
    }
  else 
    {
      P_warn ("Find_Expr_SCC: searching for expression that is not OP_var"
	      " or OP_assign\n");
      return NULL;
    }

  /* sanity check */
  if (search_expr->opcode != OP_assign)
    P_punt("Find_Expr_SCC: search_expr should be of type OP_assign\n");

  /* loop through the SCCs */
  for (scc = tloop->sccs; scc; scc = scc->next)
    {
      if (scc->type & type)
	{
	  /* loop through the SCC's expressions */
	  List_start(scc->tnode_list);
	  while ((tnode = List_next(scc->tnode_list)))
	    {
	      if (tnode->expr == search_expr)
		{
		  return scc;
		}
	    }
	}
    }
  return NULL;
}

/*
 * this function can be used to determine the expressions
 * for components that are not part of an SCC.  for example,
 * if we have an array index that is calculated based on
 * an induction variable...we could determine the expression
 * for that index here
 */
static void
Classify_Trivial(PSS_TarLoop tloop, PSS_TarNode tnode)
{
  ;
}

/* 
 * classify IGNORE
 *
 * this is to classify dumb SCC's that we find because we
 * are not using a pruned SSA
 *
 * return 1 if we are classifying
 */
static int
Classify_Ignore(PSS_TarSCC scc)
{
  /* you cannot have an SCC with a single node (the mu node)
   * cause this does not do anything */
  if (List_size(scc->tnode_list) == 1)
    {
      scc->type = IGNORE;
      PSS_PrintSCC(scc);
      return 1;
    }

  return 0;
}

/*! \brief Determine if an expression in an SCC could be part of a
 * linear SCC
 *
 * \param scc
 *  SCC the element is in
 *
 * \param expr
 *  Expression to be evaluated
 *
 * \return
 *  1 if an expression (and its operands) are valid
 * 
 * Helper fuction for Classify_Linear that scans down an assign expression
 * to make sure that all the operations are ok.  This is done recursively.
 */
static int
Linear_Expr_Test(PSS_TarSCC scc, Expr expr, int * arith_op)
{
  PSS_TarNode tnode;
  /* scan all the expressions at this level */
  for (; expr; expr = expr->next)
    {
      /* make sure this expr is ok */
      switch (expr->opcode)
	{ 
	  case OP_var:
	      {
		Expr def_expr; 
		
		/* check if var is loop invarient */
		if (!(expr->value.var.ssa) || !(expr->value.var.ssa->var))
		  break;
		def_expr = expr->value.var.ssa->var->parentexpr;
		
		/* check if the expr is in the tar loop */
		if (!(tnode = Find_TarNode_Expr(scc->tloop, def_expr)))
		  {
		    /* make sure that the expr does not come
		     * from an inner loop -- cause then it is still
		     * loop variant */
		    if (PC_LoopContainsExpr(cfg, scc->tloop->pcloop, def_expr))
		      return 0;
		    else
		      break;
		  }
		/* if the expr is part of the scc, don't worry about it */
		else if (List_member(scc->tnode_list, tnode))
		  break;
		/* we should check if it is part of another linear IV */
		else if (0)
		  ;
		else
		  return 0;  /* loop variant that is not Linear IV */
	      } /* OP_var */
	    break;
	    /* the following cases are all boring -- all that matters is their ops */
	  case OP_assign:
	    break;
	  case OP_add:
	  case OP_sub:
	    *arith_op = 1;
	    break;
	  case OP_int:
 	  case OP_cast:
	    break;
	  case OP_phi:
	    if ((scc->phi_nodes == 0) && 
		PHI_TYPE (expr->parentexpr->operands->value.var.ssa->type))
	      {
		P_punt("Phi Node Detected in SCC that has 0 Phi Nodes\n");
	      }
	    /* make sure that the things feeding the phi node are all
	     * from this scc */
	    if (PHI_TYPE (expr->parentexpr->operands->value.var.ssa->type))
	      {
		Expr operand, def_expr;;
		for (operand = expr->operands; operand; operand = operand->next)
		  {
		    def_expr = operand->value.var.ssa->var->parentexpr;
		    tnode = Find_TarNode_Expr(scc->tloop, def_expr);
		    /* if the phi node's assignment operand is not in the loop or
		     * it is in the loop, but not in the scc, then return 0 */
		    if (!tnode || !List_member(scc->tnode_list, tnode))
		      {
			return 0;
		      }

		  }
	      }
#if 0 /* depricated */
	    if ((MU_TYPE (expr->parentexpr->operands->value.var.ssa->type)) &&
		(tnode = Find_TarNode_Expr(scc->tloop, expr)) &&
		(!List_member(scc->tnode_list, tnode)))
	      {
		return 0;
	      }
#endif
	    break;
	  default:
	    return 0;
	    break;
	}

      /* check the operands and siblings */
      if (!Linear_Expr_Test(scc, expr->operands, arith_op) || 
	  !Linear_Expr_Test(scc, expr->sibling, arith_op))
	{
	  return 0;
	}
    }
  return 1;
}

/*! \brief determine if linear IV
 *
 * \param scc
 *   SCC to check
 *
 * \return
 *   1 if classified
 *
 * SCC must be made up of additions and subtractoins of loop invariants
 * and / or other linear induction variables
 * 
 * SCC must contain exactly 1 mu node
 *
 * SCC must contain 0 phi nodes or be classified as monotonic
 */
static int
Classify_Linear(PSS_TarSCC scc)
{
  PSS_TarNode tnode;
  int arith_op, num_arith = 0;

  if (scc->mu_nodes != 1)
    return 0;

  /* loop through all the nodes in the SCC */
  List_start(scc->tnode_list);

  while ((tnode = List_next(scc->tnode_list)))
    {
      arith_op = 0;
      if (!Linear_Expr_Test(scc, tnode->expr, &arith_op))
	return 0;
      num_arith += (arith_op ? 1 : 0);
    }

  /* If there are no arithmetic ops, can't be linear. */
  if (num_arith == 0)
    return 0;

  if (scc->phi_nodes == 0)
    {
      scc->type = LINEAR;
      STATS.scc_type.linear++;
    }
  else
    {
      scc->type = LINEAR_MONOTONIC;
      STATS.scc_type.linear_monotonic++;
    }
  PSS_PrintSCC(scc);
  return 1;
}

/*! \brief Determine if an expression in an SCC could be part of a
 * polynomial SCC
 *
 * \param scc
 *  SCC the element is in
 *
 * \param expr
 *  Expression to be evaluated
 *
 * \return
 *  1 if an expression (and its operands) are valid
 * 
 * Helper fuction for Classify_Polynomial that scans down an assign expression
 * to make sure that all the operations are ok.  This is done recursively.
 */
static int
Poly_Expr_Test(PSS_TarSCC scc, Expr expr, int *arith_op)
{
  PSS_TarNode tnode;
  /* scan all the expressions at this level */
  for (; expr; expr = expr->next)
    {
      /* make sure this expr is ok */
      switch (expr->opcode)
	{ 
	  case OP_var:
	      {
		Expr def_expr; 
		
		/* check if var is loop invarient */
		if (!(expr->value.var.ssa) || !(expr->value.var.ssa->var))
		  break;
		def_expr = expr->value.var.ssa->var->parentexpr;
		
		/* check if the expr is in the loop */
		if (!(tnode = Find_TarNode_Expr(scc->tloop, def_expr)))
		  {
		    /* make sure that the expr does not come
		     * from an inner loop -- cause then it is still
		     * loop variant */
		    if (PC_LoopContainsExpr(cfg, scc->tloop->pcloop, def_expr))
		      return 0;
		    else
		      break;
		  }
		/* if the expr is part of the scc, don't worry about it */
		else if (List_member(scc->tnode_list, tnode))
		  break;
		/* we should check if it is part of another linear or poly IV */
		else if (Find_Expr_SCC(scc->tloop, expr, LINEAR | POLYNOMIAL))
		  break;
		/* if it is dependent on another IV that is monotonic...
		 * this one is monotonic too..even if there is no phi */
		else if (Find_Expr_SCC(scc->tloop, expr, 
				       LINEAR_MONOTONIC | POLYNOMIAL_MONOTONIC))
		  {
		    if (scc->phi_nodes == 0)
		      scc->phi_nodes = -1;
		    break;
		  }
		else
		  return 0;  /* loop variant that is not Linear IV */
	      } /* OP_var */
	    break;
	    /* the following cases are all boring -- all that matters is their ops */
	  case OP_assign:
	    break;
	  case OP_add:
	  case OP_sub:
	    *arith_op = 1;
	    break;
	  case OP_int:
 	  case OP_cast:
	    break;
	  case OP_phi:
	    if ((scc->phi_nodes == 0) && 
		PHI_TYPE (expr->parentexpr->operands->value.var.ssa->type))
	      {
		P_punt("Phi Node Detected in SCC that has 0 Phi Nodes\n");
	      }
	    /* make sure that the things feeding the phi node are all
	     * from this scc */
	    if (PHI_TYPE (expr->parentexpr->operands->value.var.ssa->type))
	      {
		Expr operand, def_expr;;
		for (operand = expr->operands; operand; operand = operand->next)
		  {
		    def_expr = operand->value.var.ssa->var->parentexpr;
		    tnode = Find_TarNode_Expr(scc->tloop, def_expr);
		    /* if the phi node's assignment operand is not in the loop or
		     * it is in the loop, but not in the scc, then return 0 */
		    if (!tnode || !List_member(scc->tnode_list, tnode))
		      {
			return 0;
		      }

		  }
	      }
	    break;
	  default:
	    return 0;
	    break;
	}

      /* check the operands and siblings */
      if (!Poly_Expr_Test(scc, expr->operands, arith_op) || 
	  !Poly_Expr_Test(scc, expr->sibling, arith_op))
	{
	  return 0;
	}
    }
  return 1;
}
/*! \brief determine if polynomial IV
 *
 * \param scc
 *   SCC to check
 *
 * \return
 *   1 if classified
 *
 * SCC must be made up of additions and subtractions of loop invariants,
 * linear induction variables, and/or other polynomial IVs.  Must contain
 * exactly 1 mu node.
 */
static int
Classify_Polynomial(PSS_TarSCC scc)
{
  PSS_TarNode tnode;
  int arith_op, num_arith = 0;

  if (scc->mu_nodes != 1)
    return 0;

  /* loop through all the nodes in the SCC */
  List_start(scc->tnode_list);

  while ((tnode = List_next(scc->tnode_list)))
    {
      arith_op = 0;
      if (!Poly_Expr_Test(scc, tnode->expr, &arith_op))
	return 0;
      num_arith += (arith_op ? 1 : 0);
    }

  if (num_arith == 0)
    return 0;

  if (scc->phi_nodes == 0)
    {
      scc->type = POLYNOMIAL;
      STATS.scc_type.polynomial++;
    }
  else
    {
      scc->type = POLYNOMIAL_MONOTONIC;
      STATS.scc_type.polynomial_monotonic++;
    }
  PSS_PrintSCC(scc);
  return 1;
}

/*! \brief Determine if an expression in an SCC could be part of a
 * geometry SCC
 *
 * \param scc
 *  SCC the element is in
 *
 * \param expr
 *  Expression to be evaluated
 *
 * \return
 *  1 if an expression (and its operands) are valid
 * 
 * Helper fuction for Classify_Geometric that scans down an assign expression
 * to make sure that all the operations are ok.  This is done recursively.
 */
static int
Geom_Expr_Test(PSS_TarSCC scc, Expr expr, int * arith_op)
{
  /* scan all the expressions at this level */
  for (; expr; expr = expr->next)
    {
      /* make sure this expr is ok */
      switch (expr->opcode)
	{ 
	  case OP_var:
	      {
		Expr def_expr; 
		PSS_TarNode tnode;
		
		/* check if var is loop invarient */
		if (!(expr->value.var.ssa) || !(expr->value.var.ssa->var))
		  break;
		def_expr = expr->value.var.ssa->var->parentexpr;
		
		/* check if the expr is in the loop */
		if (!(tnode = Find_TarNode_Expr(scc->tloop, def_expr)))
		  {
		    /* make sure that the expr does not come
		     * from an inner loop -- cause then it is still
		     * loop variant */
		    if (PC_LoopContainsExpr(cfg, scc->tloop->pcloop, def_expr))
		      return 0;
		    else
		      break;
		  }
		/* if the expr is part of the scc, don't worry about it */
		else if (List_member(scc->tnode_list, tnode))
		  break;
		/* we should check if it is part of another linear or poly IV */
		else if (Find_Expr_SCC(scc->tloop, expr, LINEAR | POLYNOMIAL))
		  break;
		/* if it is dependent on another IV that is monotonic...
		 * this one is monotonic too..even if there is no phi */
		else if (Find_Expr_SCC(scc->tloop, expr, 
				       LINEAR_MONOTONIC | POLYNOMIAL_MONOTONIC))
		  {
		    if (scc->phi_nodes == 0)
		      scc->phi_nodes = -1;
		    break;
		  }
		else
		  return 0;  /* loop variant that is not Linear IV */
	      } /* OP_var */
	    break;
	    /* the following cases are all boring -- all that matters is their ops */
	  case OP_assign:
	    break;
	  case OP_add:
	  case OP_sub:
	    *arith_op = 1;
	    break;
	  case OP_int:
 	  case OP_cast:
	    break;
	  case OP_mul:
	    P_punt("Not implemented\n");
	  case OP_phi:
	    if ((scc->phi_nodes == 0) && 
		PHI_TYPE (expr->parentexpr->operands->value.var.ssa->type))
	      {
		P_punt("Phi Node Detected in SCC that has 0 Phi Nodes\n");
	      }
	    break;
	  default:
	    return 0;
	    break;
	}

      /* check the operands and siblings */
      if (!Geom_Expr_Test(scc, expr->operands, arith_op) || 
	  !Geom_Expr_Test(scc, expr->sibling, arith_op))
	{
	  return 0;
	}
    }
  return 1;
}
/*! \brief determine if geometric IV
 *
 * \param scc
 *   SCC to check
 *
 * \return
 *   1 if classified
 *
 * SCC must be made up of additions and subtractions of loop invariants,
 * linear induction variables, and/or other polynomial IVs.  SCC must contain
 * at least 1 multiplication of the sequence variable by a loop invariant.
 * Must contain exactly 1 mu node.
 */
static int
Classify_Geometric(PSS_TarSCC scc)
{
#if 0
  PSS_TarNode tnode;
  int arith_op, num_arith = 0;

  if (scc->mu_nodes != 1)
    return 0;

  /* loop through all the nodes in the SCC */
  List_start(scc->tnode_list);

  while ((tnode = List_next(scc->tnode_list)))
    {
      arith_op = 0;
      if (!Geom_Expr_Test(scc, tnode->expr, &arith_op))
	return 0;
      num_arith += (arith_op ? 1 : 0);
    }

  if (num_arith == 0)
    return 0;

  if (scc->phi_nodes == 0)
    {
      scc->type = GEOMETRIC;
      STATS.scc_type.geometric++;
    }
  else
    {
      scc->type = GEOMETRIC_MONOTONIC;
      STATS.scc_type.geometric_monotonic++;
    }
  PSS_PrintSCC(scc);
  return 1;
#else
  return 0;
#endif
}

/*! \brief Determine if an expression in an SCC could be part of a
 * pointer SCC
 *
 * \param scc
 *  SCC the element is in
 *
 * \param expr
 *  Expression to be evaluated
 *
 * \return
 *  1 if an expression (and its operands) are valid
 * 
 * Helper fuction for Classify_Pointer that scans down an assign expression
 * to make sure that all the operations are ok.  This is done recursively.
 */
static int
Pointer_Expr_Test(PSS_TarSCC scc, Expr expr, int * pointer_op)
{
  /* scan all the expressions at this level */
  for (; expr; expr = expr->next)
    {
      /* make sure this expr is ok */
      switch (expr->opcode)
	{ 
	  case OP_var:
	      {
		Expr def_expr; 
		PSS_TarNode tnode;
		
		/* check if var is loop invarient */
		if (!(expr->value.var.ssa) || !(expr->value.var.ssa->var))
		  break;
		def_expr = expr->value.var.ssa->var->parentexpr;
		
		/* check if the expr is in the loop */
		if (!(tnode = Find_TarNode_Expr(scc->tloop, def_expr)))
		  {
		    /* make sure that the expr does not come
		     * from an inner loop -- cause then it is still
		     * loop variant */
		    if (PC_LoopContainsExpr(cfg, scc->tloop->pcloop, def_expr))
		      return 0;
		    else
		      break;
		  }
		/* if the expr is part of the scc, don't worry about it */
		else if (List_member(scc->tnode_list, tnode))
		  break;
		else
		  return 0;  /* loop variant that is not Linear IV */
	      } /* OP_var */
	    break;
	    /* the following cases are all boring -- all that matters is their ops */
	  case OP_assign:
	  case OP_add:
	  case OP_sub:
	  case OP_int:
 	  case OP_cast:
	    break;
 	  case OP_arrow:
	    *pointer_op = 1;
	    break;
	  case OP_phi:
	    if ((scc->phi_nodes == 0) && 
		PHI_TYPE (expr->parentexpr->operands->value.var.ssa->type))
	      {
		P_punt("Phi Node Detected in SCC that has 0 Phi Nodes\n");
	      }
	    break;
	  default:
	    return 0;
	    break;
	}

      /* check the operands and siblings */
      if (!Pointer_Expr_Test(scc, expr->operands, pointer_op) || 
	  !Pointer_Expr_Test(scc, expr->sibling, pointer_op))
	{
	  return 0;
	}
    }
  return 1;
}
/*! \brief determine if pointer IV
 *
 * \param scc
 *   SCC to check
 *
 * \return
 *   1 if classified
 *
 * SCC must be made up of additions and subtractions of loop invariants,
 * linear induction variables, and an OP_arrow
 */
static int
Classify_Pointer(PSS_TarSCC scc)
{
  PSS_TarNode tnode;
  int pointer_op, num_pointer = 0;

  if (scc->mu_nodes != 1)
    return 0;

  /* loop through all the nodes in the SCC */
  List_start(scc->tnode_list);

  while ((tnode = List_next(scc->tnode_list)))
    {
      pointer_op = 0;
      if (!Pointer_Expr_Test(scc, tnode->expr, &pointer_op))
	return 0;
      num_pointer += (pointer_op ? 1 : 0);
    }

  if (num_pointer == 0)
    return 0;

  if (scc->phi_nodes == 0)
    {
      scc->type = POINTER;
      STATS.scc_type.pointer++;
    }
  else
    {
      scc->type = POINTER_MONOTONIC;
      STATS.scc_type.pointer_monotonic++;
    }
  PSS_PrintSCC(scc);
  return 1;
}

/*
 * finds the tnode for the expr that you pass it in the
 * current tloop
 */
static PSS_TarNode
Find_TarNode_Expr (PSS_TarLoop tloop, Expr expr)
{
  PSS_TarNode tnode;

  /* sanity check */
  if (expr->opcode != OP_assign)
    {
      P_warn("Find_TarNode_Expr: searching for tnode for expr that is not OP_var");
      return NULL;
    }
  
  for (tnode = tloop->first; tnode; tnode = tnode->next)
    {
      if (tnode->expr == expr)
	return tnode;
    }
  /* if not found ... */
  return NULL;
}


PSS_TarSCC
PSS_Get_SCC (PSS_TarLoop tloop, Expr expr)
{
  PSS_TarSCC scc;
  PSS_TarNode tnode;

  if (expr->opcode != OP_assign)
    {
      P_warn("Find_TarNode_Expr: searching for tnode for expr that is not OP_assign");
      return NULL;
    }

  for (scc = tloop->sccs; scc; scc = scc->next)
    {
      if (scc->tnode_list == NULL)  /* List might be empty */
	continue;
      List_start (scc->tnode_list);
      while ((tnode = (PSS_TarNode) List_next(scc->tnode_list)))
	{
	  if (expr == tnode->expr)
	    return scc;
	}
    }
  return NULL;
}

#if 0
static PSS_TarNode
Find_TarNode_ID (PSS_TarLoop tloop, int id)
{
  PSS_TarNode tnode;

  for (tnode = tloop->first; tnode; tnode = tnode->next)
    if (tnode->id == id)
      return tnode;
  
  return NULL;
}
#endif

/*
 * simple function to add a tnode to an scc
 */
static void 
Add_Node_To_Scc(PSS_TarNode tnode, PSS_TarSCC scc)
{
  /* in case we have not initialized an SCC yet... */
  if (scc == NULL)
    {
      scc = New_TarSCC(tnode->tloop);
    }

  scc->tnode_list = List_insert_last(scc->tnode_list, tnode);

  /* make sure we have copied the name of the variable */
  if (!scc->var_name)
    {
      /* Need to malloc strlen + 1 for '\0' char */
      scc->var_name = 
	(char *) malloc((strlen(tnode->expr->operands->value.var.name) + 1) *
			sizeof(char));
      strcpy(scc->var_name, tnode->expr->operands->value.var.name);
    }

  /* determine if we are adding a phi node or mu node */
  if (tnode->expr->operands->sibling->opcode == OP_phi &&
      tnode->expr->operands->value.var.ssa)
    {
      if (MU_TYPE (tnode->expr->operands->value.var.ssa->type))
	scc->mu_nodes++;
      else if (PHI_TYPE (tnode->expr->operands->value.var.ssa->type))
	scc->phi_nodes++;
    }
}

/*
 * see "Beyond Induction Variables" paper for details on this
 * implementation of tarjan's algo
 */
static void
Visit_Node (PSS_TarLoop tloop, PSS_TarNode tnode)
{
  int low, this;
  Expr def_var_expr, operand_expr;
  List operand_list;

  tnode->status = ONSTACK;
  tnode->lowlink = low = this = tloop->number++;

  Push_Top(tloop->node_stack, tnode);

  /* o visit all operand descendents 
   * o visit all ssa flows
   *   - in our implementation, these all happen together
   *     because the ssa links off of phi functions are just
   *     treated like operands to a phi function */
  operand_list = PSS_GetSubExprByOpcode_List (tnode->expr->operands, OP_var);
  List_start(operand_list);
  while ((operand_expr = List_next(operand_list)))
    {
      Expr op_def_expr;
      PSS_TarNode op_def_node;

      /* we can't have parameters or undef's on the SCC */
      if (operand_expr->value.var.ssa && 
	  !UNINITIALIZED_TYPE (operand_expr->value.var.ssa->type))
	{
	  op_def_expr = operand_expr->value.var.ssa->var->parentexpr;

	  if ((op_def_node = Find_TarNode_Expr(tloop, op_def_expr)))
	    {
	      low = MIN(low, Visit_Descendent(tloop, op_def_node));
	    }
	}
    }

  tnode->lowlink = low;

  /* check if classification not possible at this time */
  if (this != low) return;

  def_var_expr = tnode->expr->operands;

  /* sanity check */
  if (tnode->expr->opcode != OP_assign || def_var_expr->opcode != OP_var)
    P_punt("Visit_Node: Illegal TarNode Expression");

  /* SCC's always finish off with the same element
   * on the top of the stack, unless the node we are
   * taking a look at is a mu node */
  if (Stack_Top(tloop->node_stack) == tnode && 
      !MU_TYPE (def_var_expr->value.var.ssa->type))
    {
      Classify_Trivial(tloop, tnode);
      Pop(tloop->node_stack);
      tnode->status = DONE;
    } /* trivial */
  else /* SCC */
    {
      PSS_TarNode stacktop;
      PSS_TarSCC scc;

      scc = New_TarSCC(tloop);
      do
	{
	  stacktop = Pop(tloop->node_stack);
	  stacktop->status = DONE;
	  Add_Node_To_Scc(stacktop, scc);
	  /* === ADD STACKTOP TO COMPONENT === */
	} while(stacktop != tnode);
      /* in the algo presented in the paper, one would classify
       * the sequences here.  however, because different types
       * of induction variables depend on knowledge abou previous
       * types (for example, a polynomial induction var is made 
       * up of linear induction vars), it is necessary to identify
       * all the SCC's (and vars) that are of certain types before
       * identifying other types.  as such, we will classify all
       * the induction vars at one time at the end.
       */
#if 0
      Classify_Sequence(scc);
#endif
    } /* SCC */
}

/*
 * see "Beyond Induction Variables" paper for details on this
 * implementation of tarjan's algo
 */
static int
Visit_Descendent (PSS_TarLoop tloop, PSS_TarNode tnode)
{

  /* first determine if this node is inside of the current loop */
  if (!tnode->inner_loop)
    return (tloop->number);

  if (tnode->status == NOTYET)
    {
      Visit_Node(tloop, tnode);
      return tnode->lowlink;
    }
  else if (tnode->status == ONSTACK)
    {
      return tnode->lowlink;
    }
  /* else status = DONE */
  return tloop->number;
}

/* debug functions */
static void
Print_TarNode (PSS_TarNode tnode)
{
  Expr dest_var;

  dest_var = tnode->expr->operands;

  fprintf(debug_file_id, "ID: %d - Lowlink: %d - Status: %d - Inner Loop %d - Dest: %s",
	 tnode->id, tnode->lowlink, tnode->status, 
	 tnode->inner_loop->ID, dest_var->value.var.name);
  if (dest_var->value.var.ssa)
    fprintf(debug_file_id, "_v%d\n", dest_var->value.var.ssa->subscr);
  else /* no ssa */
    fprintf(debug_file_id, "\n");
}


void
PSS_Print_TarLoop (PSS_TarLoop tloop)
{
  PSS_TarNode tnode;

  fprintf(debug_file_id, "Loop ID: %d\n", tloop->pcloop->ID);

  /* loop through all the tarnodes, showing their status */
  for (tnode = tloop->first; tnode; tnode = tnode->next)
    {
      Print_TarNode(tnode);
    }
}

void
PSS_PrintSCC(PSS_TarSCC scc)
{
  PSS_TarNode tnode;

  fprintf(debug_file_id, "SCC (%d) - loop %d - func %s - type:", 
	  scc->id, scc->tloop->pcloop->ID, scc->tloop->cfg->func->name);
  PSS_PrintSCCType(debug_file_id, scc);
  fprintf(debug_file_id, "\n");

  List_start(scc->tnode_list);
  while ((tnode = List_next(scc->tnode_list)))
    Print_TarNode(tnode);
}

void
PSS_PrintLoops(PC_Loop pcloop)
{
  int *bbs;
  int bb_count, i;
  
  for (; pcloop; pcloop = pcloop->sibling)
    {
      fprintf(debug_file_id, "-");

      PSS_PrintLoops(pcloop->child);

      fprintf(debug_file_id, "Loop %d: ", pcloop->ID);

      bbs = (int *) calloc (Set_size (pcloop->body), sizeof (int));
      bb_count = Set_2array (pcloop->body, bbs);

      for (i = 0; i < bb_count; i++)
	fprintf(debug_file_id, "%d ", bbs[i]);

      fprintf(debug_file_id, "\n");
      
      free(bbs);
    }
}

void 
PSS_PrintSCCType(FILE *fp, PSS_TarSCC scc)
{
  switch (scc->type)
    {
      case IGNORE:
	fprintf(fp, "IGNORE");
	break;
      case UNKNOWN:
	fprintf(fp, "UNKNOWN");
	break;
      case LINEAR:
	fprintf(fp, "LINEAR");
	break;
      case LINEAR_MONOTONIC:
	fprintf(fp, "LINEAR MONOTONIC");
	break;
      case POLYNOMIAL:
	fprintf(fp, "POLYNOMIAL");
	break;
      case POLYNOMIAL_MONOTONIC:
	fprintf(fp, "POLYNOMIAL MONOTONIC");
	break;
      case POINTER:
	fprintf(fp, "POINTER");
	break;
      case POINTER_MONOTONIC:
	fprintf(fp, "POINTER MONOTONIC");
	break;
      default:
	break;
    }

#if 0
  if (scc->type == IGNORE)
    fprintf(debug_file_id, "ignore");
  else if (scc->type == UNKNOWN)
    fprintf(debug_file_id, "unknown");
  else if (scc->type == LINEAR)
    fprintf(debug_file_id, "linear");
  else if (scc->type == LINEAR_MONOTONIC)
    fprintf(debug_file_id, "linear monotonic");
  else if (scc->type == POLYNOMIAL)
    fprintf(debug_file_id, "polynomial");
  else if (scc->type == POLYNOMIAL_MONOTONIC)
    fprintf(debug_file_id, "polynomial monotonic");
  else if (scc->type == POINTER)
    fprintf(debug_file_id, "pointer");
  else if (scc->type == POINTER_MONOTONIC)
    fprintf(debug_file_id, "pointer monotonic");
#endif
}

static void
Counter_Clear_SCC_Types()
{
  STATS.scc_type.linear = 0;
  STATS.scc_type.linear_monotonic = 0;
  STATS.scc_type.polynomial = 0;
  STATS.scc_type.polynomial_monotonic = 0;
  STATS.scc_type.pointer = 0;
  STATS.scc_type.pointer_monotonic = 0;
  STATS.scc_type.unknown = 0;
}

static void
Counter_Print_SCC_Types()
{
  fprintf(stat_file_id, "<SCCTYPES>,%s,%d,%d,%d,%d,%d,%d,%d\n",
	 cfg->func->name,
	 STATS.scc_type.linear,
	 STATS.scc_type.linear_monotonic,
	 STATS.scc_type.polynomial,
	 STATS.scc_type.polynomial_monotonic,
	 STATS.scc_type.pointer,
	 STATS.scc_type.pointer_monotonic,
	 STATS.scc_type.unknown);
}
