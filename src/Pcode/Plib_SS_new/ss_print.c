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
 *      File :          ss_print.c
 *      Description :   Pcode SSA Module
 *      Creation Date : October 13, 2004 
 *      Author :        James Player, Ian Steiner
 *
 * 
 * 
 *===========================================================================*/
#include <Pcode/pcode.h>
#include <Pcode/write.h>
#include <Pcode/symtab_i.h>
#include <Pcode/loop.h>
#include "ss_ssa2.h"
#include "ss_induct2.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_INPUT "debug.in"
#define BUFF_SIZE  128

#define TOP_LOOPS_FILE "toploops.stats"

typedef struct _TopLoop
{
  char func_name[255];
  int loop_id;
  int bb_id;
  PSS_TarLoop tloop;
  PSS_Bound_Condition bound_cond;
  double weight;
}
_TopLoop, *TopLoop;

#define TOP_LIST_SIZE 10
typedef struct _TopLoopList
{
  TopLoop loops[TOP_LIST_SIZE];
  int min_index;
}
_TopLoopList, *TopLoopList;

TopLoopList toploops = NULL;

#define MIN_TOP_LOOP (toploops->loops[toploops->min_index])

static void PrintTarLoopSCCStats(FILE *fp, PSS_TarLoop tloop);
static void PrintSCCSummary(FILE *fp, PSS_TarSCC scc);
static void PrintBounds (FILE *fp, TopLoop toploop);

/*
 * simple debug script for creating a key 
 */
Key
P_IMS_Keygen(int file, int sym)
{
  Key key;
  key.file = file;
  key.sym = sym;
  return key;
}

void
PrintSSASubscr(FILE *fp, Expr expr)
{
  if (expr->opcode != OP_var)
    return;

  if (expr->value.var.ssa == NULL)
    fprintf (fp, "???");
  else if (PARAM_TYPE (expr->value.var.ssa->type))
    fprintf(fp, "PARAM");
  else if (UNDEF_TYPE (expr->value.var.ssa->type))
    fprintf(fp, "UNDEF");
  else
    fprintf(fp, "%d", expr->value.var.ssa->subscr);
}

void
PrintLifetimeSubscr(FILE *fp, Expr expr)
{
  if (expr->opcode != OP_var)
    return;

  if (expr->value.var.ssa == NULL)
    fprintf (fp, "???");
  else if (PARAM_TYPE (expr->value.var.ssa->type))
    fprintf(fp, "PARAM_%d", expr->value.var.ssa->name);
  else if (UNDEF_TYPE (expr->value.var.ssa->type))
    fprintf(fp, "UNDEF_%d", expr->value.var.ssa->name);
  else
    fprintf(fp, "%d", expr->value.var.ssa->name);  
}

void
P_PrintPcodeSSAVars (PC_Graph cfg, PSS_BaseTbl varTbl)
{
  FILE *in, *out;
  char prev_tok[BUFF_SIZE];
  char curr_tok[BUFF_SIZE];
  int scan = 0;
  int print_this_func = 0;

  if ((in = fopen (DEBUG_INPUT, "r")))
    {
      scan = fscanf (in, "%s", prev_tok);
      do
	{
	  scan = fscanf (in, "%s", curr_tok);
	  
	  if (curr_tok[0] == ':')
	    {
	      scan = fscanf (in, "%s", curr_tok);
	      print_this_func = (!strcmp (cfg->func->name, prev_tok)) ? 1 : 0;
	    }
	  
	  else
	    {
	      if (print_this_func)
		{
		  PSS_Base base;
		  VarDcl vdcl = NULL;
		  char path[BUFF_SIZE];
		  char *varName = prev_tok;

		  for (base = varTbl->first; base; base = base->next)
		    if (!strcmp (base->vdcl->name, varName))
		      {
			vdcl = base->vdcl;
			break;
		      }

		  if (base) /* print the debugging graphs for this variable */
		    {
		      assert (vdcl != NULL);
		      
		      if (base->addr_taken)
			P_warn ("\n  P_PrintPcodeSSAVars: variable \"%s\" in "
				"function \"%s\" is addr taken.",
				varName, cfg->func->name);

		      else
			{
			  sprintf (path, "%s_%s_SSA.dot", cfg->func->name,
				   vdcl->name);
			  out = fopen (path, "w");
			  P_PrintPcodeSSAGraph (out, cfg, vdcl);
			  fclose (out);

			  P_PrintPcodeLifetimeGraph (cfg, vdcl);
			}
		    }
		  else
		    P_warn ("P_PrintPcodeSSAVars: Specified debug variable "
			    "\"%s\"\n  not found in function \"%s\".",
			    varName, cfg->func->name);
		}
	    }
	  
	  strcpy (prev_tok, curr_tok);
	  
	} while (scan != EOF);

      fclose (in);
    }
}

void
P_PrintPcodeSSAGraph(FILE *fp, PC_Graph cfg, VarDcl vdcl)
{
  PC_Block bb;
  PC_Flow f;
  _PC_ExprIter ei;
  Expr expr;
  List sub_expr_list = NULL;
  List bb_var_list = NULL;
  int printed;
  char path[BUFF_SIZE];
  FILE *def_use_out;

  sprintf (path, "%s_%s_DU", cfg->func->name, vdcl->name);
  def_use_out = fopen (path, "w");

  /* print header information */
  fprintf(fp, "// SSA CFG Graph - Var: %s\n", vdcl->name);
  
  fprintf(fp, "digraph G {\n");
  fprintf(fp, "\tnode [shape=record]\n");

  fprintf (fp, "\tLabelBlock [label=\"{fn = \\\"%s\\\"\\n"
	   "var = \\\"%s\\\"}\"]\n", cfg->func->name, vdcl->name);
  
  /* traverse through the basic blocks */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      fprintf (fp, "\tbb%d [label=\"{", bb->ID);
      
      /* loop through all expressions in bb */
      for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
	   expr = PC_ExprIterNext (&ei, 1))
	{
	  /* get a list of all the var sub expressions */
	  sub_expr_list = PSS_GetSubExprByOpcode_List(expr, OP_var);

	  List_start (sub_expr_list);
	  while ((expr = List_next(sub_expr_list)))
	    if (vdcl == PSI_GetVarDclEntry (expr->value.var.key))
	      bb_var_list = List_insert_last(bb_var_list, expr);

	  /* free up stuff for current top expression */
	  sub_expr_list = List_reset(sub_expr_list);

	} /* for expr in bb */

      printed = 0;
      /* traverse the list of vars, printing them out */
      List_start (bb_var_list);
      while ((expr = List_next(bb_var_list)))
	{
	  if (printed)
	    fprintf(fp, "\\n");
	  
	  if (!PSS_VarIsUse (expr))
	    {
	      if (expr->sibling && expr->sibling->opcode != OP_phi)
		{
		  fprintf(fp, "%d =", expr->value.var.ssa->subscr);
		  P_write_opcode(fp, expr->sibling->opcode, 0);
		  fprintf (stderr, "DEF %s.%i:\n",
			   expr->value.var.name, expr->value.var.ssa->subscr);
		  P_write_expr (stderr, expr->parentexpr, 0, 0);
		  fprintf (stderr, "\n\n");
		}
	      
	      else /* mu or phi */
		{
		  Expr PhiOperand;
		  PSS_Def ssa_def = expr->value.var.ssa;

		  if (MU_TYPE (ssa_def->type))
		    fprintf(fp, "%d = mu(", expr->value.var.ssa->subscr);
		  else
		    fprintf(fp, "%d = phi(", expr->value.var.ssa->subscr);
		  for (PhiOperand = expr->sibling->operands; PhiOperand;
		       PhiOperand = PhiOperand->next)
		    {
		      PrintSSASubscr(fp, PhiOperand);
		      if (PhiOperand->next != NULL)
			fprintf(fp, ",");
		      
		      if (PhiOperand->value.var.ssa)
			{
			  fprintf (def_use_out, "USE %i:\n", 
				  PhiOperand->value.var.ssa->subscr);
			  P_write_expr (def_use_out, PhiOperand->parentexpr,
					2, NULL);
			  fprintf (def_use_out, "\n\n\n");
			}
		    }
		  fprintf(fp, ")");
		}
	      
	      fprintf (def_use_out, "DEF %i:\n", expr->value.var.ssa->subscr);
	      P_write_expr (def_use_out, expr->parentexpr, 2, NULL);
	      fprintf (def_use_out, "\n\n\n");
	      
	      printed = 1;
	    }

	  /* var is use */
	  else if (!expr->parentexpr || expr->parentexpr->opcode != OP_phi)
	    {
	      fprintf(fp, "= ");
	      PrintSSASubscr(fp, expr);
	      P_PrintUseDefChain(cfg, expr);
		{
		  fprintf (def_use_out, "USE %d (bb%d):\n",
			  expr->value.var.ssa->subscr, bb->ID);
		  if (expr->parentexpr)
		    P_write_expr (def_use_out, expr->parentexpr, 2, NULL);
		  else
		    P_write_expr (def_use_out, expr, 2, NULL);
		  fprintf (def_use_out, "\n\n\n");
		}
	      printed = 1;
	    }

	  else printed = 0;

	}


      bb_var_list = List_reset (bb_var_list);
      
      /* close off bb */
      if (printed)
	fprintf(fp, "|{%d}}\", style=filled]\n", bb->ID);
      else
	fprintf(fp, "%d}\"]\n", bb->ID);
      
    } /* for bb */

  /* print the flows for the bbs */
  for (bb = cfg->first_bb; bb; bb = bb->next)
  {
    for (f = bb->s_flow; f; f = f->s_next_flow)
      {
	if (f->flow_cond->opcode != OP_int)
	  P_punt ("PC_PrintGraph: Non-integral flow cond");

	fprintf (fp, "\tbb%d -> bb%d;\n",
		 f->src_bb->ID, f->dest_bb->ID);
      }
  }


  /* close off digraph */
  fprintf(fp, "}\n");

  fclose (def_use_out);
}

void
P_PrintPcodeSSAGraph_stdout(PC_Graph cfg, int file, int sym)
{
  P_PrintPcodeSSAGraph(stdout, cfg, 
		       PSI_GetVarDclEntry(P_IMS_Keygen(file, sym)));
  return;
}


void
P_PrintPcodeSSAGraph_file(PC_Graph cfg, int file, int sym)
{
  FILE *fp;
  fp = fopen("test.dot", "w");
  P_PrintPcodeSSAGraph(fp, cfg, 
		       PSI_GetVarDclEntry(P_IMS_Keygen(file, sym)));
  fclose(fp);
  return;
}

void
P_PrintPcodeLifetimeGraph (PC_Graph cfg, VarDcl vdcl)
{
  PC_Block bb;
  PC_Flow f;
  _PC_ExprIter ei;
  Expr expr;
  List sub_expr_list = NULL;
  List bb_var_list = NULL;
  int printed;
  char path[BUFF_SIZE];
  FILE *fp;

  sprintf (path, "%s_%s_LT.dot", cfg->func->name, vdcl->name);
  fp = fopen (path, "w");

  /* print header information */
  fprintf(fp, "// SSA CFG Graph - Var: %s\n", vdcl->name);
  
  fprintf(fp, "digraph G {\n");
  fprintf(fp, "\tnode [shape=record]\n");

  fprintf (fp, "\tLabelBlock [label=\"{fn = \\\"%s\\\"\\n"
	   "var = \\\"%s\\\"}\"]\n", cfg->func->name, vdcl->name);
  
  /* traverse through the basic blocks */
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      fprintf (fp, "\tbb%d [label=\"{", bb->ID);
      
      /* loop through all expressions in bb */
      for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
	   expr = PC_ExprIterNext (&ei, 1))
	{
	  /* get a list of all the var sub expressions */
	  sub_expr_list = PSS_GetSubExprByOpcode_List(expr, OP_var);

	  List_start (sub_expr_list);
	  while ((expr = List_next(sub_expr_list)))
	    if (vdcl == PSI_GetVarDclEntry (expr->value.var.key))
	      bb_var_list = List_insert_last(bb_var_list, expr);

	  /* free up stuff for current top expression */
	  sub_expr_list = List_reset(sub_expr_list);

	} /* for expr in bb */

      printed = 0;
      /* traverse the list of vars, printing them out */
      List_start (bb_var_list);
      while ((expr = List_next(bb_var_list)))
	{
	  if (printed)
	    fprintf(fp, "\\n");
	  
	  if (!PSS_VarIsUse (expr))
	    {
	      if (expr->sibling && expr->sibling->opcode != OP_phi)
		{
		  PrintLifetimeSubscr (fp, expr);
		  fprintf(fp, " =");
		  P_write_opcode(fp, expr->sibling->opcode, 0);
		}
	      
	      else /* mu or phi */
		{
		  Expr PhiOperand;
		  PSS_Def ssa_def = expr->value.var.ssa;

		  PrintLifetimeSubscr (fp, expr);
		  if (MU_TYPE (ssa_def->type))
		    fprintf(fp, " = mu(");
		  else
		    fprintf(fp, " = phi(");
		  
		  for (PhiOperand = expr->sibling->operands; PhiOperand;
		       PhiOperand = PhiOperand->next)
		    {
		      PrintLifetimeSubscr(fp, PhiOperand);
		      if (PhiOperand->next != NULL)
			fprintf(fp, ",");
		      
		    }
		  fprintf(fp, ")");
		}
	      
	      printed = 1;
	    }

	  /* var is use */
	  else if (!expr->parentexpr || expr->parentexpr->opcode != OP_phi)
	    {
	      fprintf(fp, "= ");
	      PrintLifetimeSubscr(fp, expr);
	      printed = 1;
	    }

	  else printed = 0;

	}


      bb_var_list = List_reset (bb_var_list);
      
      /* close off bb */
      if (printed)
	fprintf(fp, "|{%d}}\", style=filled]\n", bb->ID);
      else
	fprintf(fp, "%d}\"]\n", bb->ID);
      
    } /* for bb */

  /* print the flows for the bbs */
  for (bb = cfg->first_bb; bb; bb = bb->next)
  {
    for (f = bb->s_flow; f; f = f->s_next_flow)
      {
	if (f->flow_cond->opcode != OP_int)
	  P_punt ("PC_PrintGraph: Non-integral flow cond");

	fprintf (fp, "\tbb%d -> bb%d;\n",
		 f->src_bb->ID, f->dest_bb->ID);
      }
  }


  /* close off digraph */
  fprintf(fp, "}\n");
  fclose(fp);
}

/*
 * helper function to create a string with the name and subscr of a
 * variable expression
 */
static char *
SSA_Name(Expr expr)
{
  char *name;
  if (expr->opcode != OP_var)
    {
       P_warn("P_PrintUseDefChain: expr not OP_var\n");
       return NULL;
    }
  name = (char *) malloc (sizeof(char) * (strlen(expr->value.var.name) + 10));
  sprintf (name, "%s_%d", expr->value.var.name, expr->value.var.ssa->subscr);

  return name;
}

/* pass a file pointer and an expr that is a "use" (rhs of an assignment)
 * and it will print out the use -> def tree above that use */
static void
PrintUseDefs(FILE *fp, Expr use_expr, Expr prev_use_expr, Set visitted_set)
{
  PSS_Def ssa_def;
  List var_op_list;
  Expr def_expr, def_use_expr;
  char *name, *prev_name;
  
  /* find the def, and recursively move up from it's operands */
  ssa_def = use_expr->value.var.ssa;

  /* verify that it is a use */
  if (!PSS_VarIsUse(use_expr))
    P_punt("Error: variable is not a use\n");

  if (!ssa_def)
    return;
  
  def_expr = ssa_def->var;

  /* sanity check */
  if (def_expr && PSS_VarIsUse(def_expr))
    P_punt("PrintUseDefs: def_expr should not be a use\n");

  name = SSA_Name(use_expr);

  /* make sure we dont go around cycles - just add the link */
  if (Set_in(visitted_set, ssa_def->subscr))
    {
      prev_name = SSA_Name(prev_use_expr);
      fprintf(fp, "\t%s -> %s;\n", prev_name, name);

      free (name);
      free (prev_name);

      return;
    }

  /* print the def */
  fprintf(fp, "\t%s;\n", name);
  Set_add(visitted_set, ssa_def->subscr);

  /* print the linkage */
  if (prev_use_expr)
    {
      prev_name = SSA_Name(prev_use_expr);
      fprintf(fp, "\t%s -> %s;\n", prev_name, name);
      free (prev_name);
    }

  free (name);
  
  /* get list of operand variables */
  var_op_list = PSS_GetSubExprByOpcode_List(def_expr, OP_var);

  /* generate graphs for each of the operands */
  List_start (var_op_list);
  while ((def_use_expr = List_next(var_op_list)))
    {
      /* sub expresions of a def can also be defs
       * a = b = c;  <= a and b are both defs */
      if (!PSS_VarIsUse(def_use_expr))
	continue;
      PrintUseDefs(fp, def_use_expr, use_expr, visitted_set);
    } /* while use expr exist */
  var_op_list = List_reset (var_op_list);
}

void
P_PrintUseDefChain(PC_Graph cfg, Expr expr)
{
  FILE *fp;
  List var_op_list;
  PSS_Def ssa_def;
  char *name, filename[128];
  Set visitted_set = NULL;

  /* verify that the expr we have is a variable expr */
  if (expr->opcode != OP_var)
    {
       P_warn("P_PrintUseDefChain: expr not OP_var\n");
       return;
    }
  
  ssa_def = expr->value.var.ssa;
  name = SSA_Name(expr);
  
  sprintf (filename, "%s_%s_defuse.dot", cfg->func->name, name);
  
  fp = fopen(filename, "w");
  
  /* print header information */
  fprintf(fp, "// SSA Use -> Def Graph - Var: %s\n", name);
  
  fprintf(fp, "digraph G {\n");
  fprintf(fp, "\tnode [shape=rectangle,style=filled]\n");
  
  visitted_set = Set_new();
  
  /* check if this var is a definition or a use */
  if (ssa_def->var == expr) /* def */
    {
      Expr use_expr;
      /* first lets make sure that our parent is an assign */
      if (!PSS_VarIsUse(expr))
	P_punt("P_PrintUseDefChain: ssa def variable is use variable\n");

      /* print the current node */
      fprintf(fp, "\t%s;\n", name);

      Set_add(visitted_set, ssa_def->subscr);
      
      /* get list of operand variables */
      var_op_list = PSS_GetSubExprByOpcode_List(expr, OP_var);

      /* generate graphs for each of the operands */
      List_start (var_op_list);
      while ((use_expr = List_next(var_op_list)))
	{
	  /* sub expresions of a def can also be defs
	   * a = b = c;  <= a and b are both defs */
	  if (!PSS_VarIsUse(use_expr))
	    continue;
	  PrintUseDefs(fp, use_expr, expr, visitted_set);
	} /* while use expr exist */
      var_op_list = List_reset (var_op_list);
    } /* definition */
  else /* use */
    {
      PrintUseDefs(fp, expr, NULL, visitted_set);
    } /* use */

  /* close off the graph */
  fprintf(fp, "}");
  
  Set_dispose(visitted_set);
  
  fclose(fp);
  
}

/*
 * recursive helper function for TopLoops
 *
 * recursively searches through the loops for loops with higher weights
 */
static void
TopLoops_Rec(PC_Graph cfg, PC_Loop pcloop)
{
  int i;
  PC_Block new_bb;
  
  for (; pcloop; pcloop = pcloop->sibling)
    {
      /* process children */
      TopLoops_Rec(cfg, pcloop->child);
      new_bb = PC_FindBlock(cfg, pcloop->head);

      if (MIN_TOP_LOOP->loop_id == -1 ||
	  MIN_TOP_LOOP->weight < new_bb->weight)
	{
	  /* replace the min_loop */
	  strcpy(MIN_TOP_LOOP->func_name, cfg->func->name);
	  MIN_TOP_LOOP->loop_id = pcloop->ID;
	  MIN_TOP_LOOP->bb_id = new_bb->ID;
	  MIN_TOP_LOOP->weight = new_bb->weight;
	  MIN_TOP_LOOP->tloop = P_LoopTarjanLoop(pcloop);
	  MIN_TOP_LOOP->bound_cond = P_LoopFundInVar(pcloop)->bounds;

	  /* update minloop */
	  for (i = 0; i < TOP_LIST_SIZE; i++)
	    {
	      if (toploops->loops[i] == NULL)
		{
		  toploops->min_index = i;
		  break;
		}
	      else if (MIN_TOP_LOOP->weight > toploops->loops[i]->weight)
		{
		  toploops->min_index = i;
		}
	    }
	}
    }
}

/*
 * returns a list of the top 10 loops (based on header weight)
 * in a given CFG.
 *
 * the *loops takes the current top 10 loops. this allows recursion,
 * and use of the same top 10 list for multiple cfg's
 */
void
P_FindTopLoops(PC_Graph cfg)
{
  int i;

  if (toploops == NULL)
    {
      toploops = (TopLoopList) malloc (sizeof(_TopLoopList));
      toploops->min_index = 0;
      for (i = 0; i < TOP_LIST_SIZE; i++)
	{
	  toploops->loops[i] = (TopLoop) malloc (sizeof(_TopLoop));
	  toploops->loops[i]->loop_id = -1;
	  toploops->loops[i]->bb_id = -1;
	  toploops->loops[i]->tloop = NULL;
	  toploops->loops[i]->weight = -1;
	}
    }

  TopLoops_Rec(cfg, cfg->lp_tree);
}

void
P_PrintTopLoops()
{
  int i;
  FILE *fp;

  fp = fopen(TOP_LOOPS_FILE, "w");

  fprintf(fp, "Top Loop List\n");
  for (i = 0; i < TOP_LIST_SIZE; i++)
    {
      fprintf(fp, "%d - ", i);
      if (toploops->loops[i]->loop_id == -1)
	fprintf(fp, "NULL\n");
      else
	fprintf(fp, "%15s - Loop %d - BB: %d - Weight: %f\n",
	       toploops->loops[i]->func_name,
	       toploops->loops[i]->loop_id,
	       toploops->loops[i]->bb_id,
	       toploops->loops[i]->weight);
      PrintTarLoopSCCStats(fp, toploops->loops[i]->tloop);
      PrintBounds(fp, toploops->loops[i]);
    }
}

static void
PrintBounds (FILE *fp, TopLoop toploop)
{
  PSS_Bound_Condition bound_cond;

  fprintf(fp, "  <BOUNDS>:");
  for (bound_cond = toploop->bound_cond; bound_cond; 
       bound_cond = bound_cond->next)
    {
      PSS_Print_Expr(fp, bound_cond->cond);
      P_PrintContOpcode(fp, bound_cond->cont_opcode);
      fprintf(fp, "\n");
    }

}

static void
PrintTarLoopSCCStats(FILE *fp, PSS_TarLoop tloop)
{
  PSS_TarSCC scc;

  int unknown = 0;
  int linear = 0;
  int linear_monotonic = 0;
  int polynomial = 0;
  int polynomial_monotonic = 0;
  int pointer = 0;
  int pointer_monotonic = 0;

  for (scc = tloop->sccs; scc; scc = scc->next)
    {
      switch (scc->type)
	{
	  case LINEAR:
	    linear++;
	    break;
	  case LINEAR_MONOTONIC:
	    linear_monotonic++;
	    break;
	  case POLYNOMIAL:
	    polynomial++;
	    break;
	  case POLYNOMIAL_MONOTONIC:
	    polynomial_monotonic++;
	    break;
	  case POINTER:
	    pointer++;
	    break;
	  case POINTER_MONOTONIC:
	    pointer_monotonic++;
	    break;
	  case UNKNOWN:
	    unknown++;
	    break;
	  default:
	    break;
	}
      if (scc->type != IGNORE)
	PrintSCCSummary(fp, scc);
    }
  fprintf(fp, " -- TOTALS --\n");
  fprintf(fp, "  > Linear: %d / %d\n", linear, linear_monotonic);
  fprintf(fp, "  > Polynomial: %d / %d\n", polynomial, polynomial_monotonic);
  fprintf(fp, "  > Pointer: %d / %d\n", pointer, pointer_monotonic);
  fprintf(fp, "  > Unknown: %d\n", unknown);
  fprintf(fp, "<CSV>,%d,%d,%d,%d,%d,%d,%d\n",
	 linear,
	 linear_monotonic,
	 polynomial,
	 polynomial_monotonic,
	 pointer,
	 pointer_monotonic,
	 unknown);
}

void
PrintSCCSummary(FILE *fp, PSS_TarSCC scc)
{
  
  fprintf(fp, " >> ");
  PSS_PrintSCCType(fp, scc);
  fprintf(fp, " -- %s", scc->var_name);

  if (scc->type == LINEAR)
    {
      Expr expr;

      fprintf(fp, "  <IE>:");
      expr = ((PSS_TarNode) List_get_first(scc->tnode_list))->expr->operands;
      PSS_Print_Expr(fp, PSS_GetExprIE(expr));
      fprintf (fp, "\n");
    }
  fprintf (fp, "\n");

}

void
P_PrintContOpcode(FILE *fp, _Opcode cont_opcode)
{
  switch (cont_opcode)
    {
    case OP_lt:
      fprintf (fp, "< 0");
      break;
    case OP_le:
      fprintf (fp, "<= 0");
      break;
    case OP_ge:
      fprintf (fp, ">= 0");
      break;
    case OP_gt:
      fprintf (fp, "> 0");
      break;
    case OP_eq:
      fprintf (fp, "== 0");
      break;
    case OP_ne:
    default:
      fprintf (fp, "!= 0");
    }
}
