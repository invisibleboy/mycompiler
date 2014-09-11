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
#include <stdlib.h>
#include <stdio.h>
#include <library/i_types.h>
#include <library/i_list.h>
#include <Pcode/struct.h>
#include <Pcode/symtab.h>
#include <Pcode/query_symtab.h>
#include <Pcode/symtab_i.h>
#include <Pcode/dom.h>
#include "ss_hash.h"
#include "ss_ssa.h"

extern void P_CF_Dump_All_BB (FILE *out_file, PC_Graph cfg, char *title);

/****************************************************************************
	Static type/variables
****************************************************************************/

#define DEBUG_P_CF 0

#define MAX_N_VAR 8192
#define MAX_HASH_TBL 2

/* 
 * symbol table entry for each local variable
 */
typedef struct _VarTblEntry
{
  VarDcl vardcl;

  int addr_taken;
  int flag;

  List modifying_expr;		/* list of _Modify's, the expr's which modify
				   a variable */
  List modifying_bb;		/* list of BasicBlock's, the bb's which modify
				   a variable */
  _LptrQ stack;			/* temporary keeping track of SSA target */
}
_VarTblEntry, *VarTblEntry;

typedef struct _Modify
{
  Expr expr;			/* the updating expr */
  PC_Block bb;			/* bb where the updating expr resides */
}
_Modify, *Modify;

typedef struct _SSA_Flag
{
  int HasAlready;
  int Work;
  void *ext;
}
_SSA_Flag, *SSA_Flag;

typedef struct _ExprHolder
{
  PC_Block bb;
  Expr expr;
}
_ExprHolder, *ExprHolder;

static List var_tbl_entries = NULL;

static int IterCount;		/* counter used in SSA */
static PC_Graph current_cfg;
static AddrHashTable ssa_d2u_tbl = NULL;
static AddrHashTable ssa_u2d_tbl = NULL;

/****************************************************************************
	Static function header
****************************************************************************/

static void DeleteVarTblEntry (VarTblEntry entry);
static VarTblEntry NewVarTblEntry (VarDcl v, Modify modifying_expr);
static Modify NewModify (Expr expr, PC_Block bb);
static void FreeModify (Modify mod);
static int AddressTaken (Expr expr);
static void ModifiedVarTableEntry (Expr expr, PC_Block bb);
static void BuildModifiedVarTbl (PC_Graph cfg);
static SSA_Flag NewSSAFlag (int HasAlready, int Work, void *ext);
static void FreeSSAFlag (SSA_Flag flag);
static void Insert_Phi_To_BB (VarDcl v, PC_Block bb);
static long Place_Phi_Var (VarTblEntry ve);
static void Place_Phi_Function (PC_Graph cfg);
static void Remove_Phi_From_BB (PC_Graph cfg, PC_Block bb);
static void Remove_Phi_Function (PC_Graph cfg);
static void Init_Var_Tbl_Entry_Stack (VarTblEntry ve);
static void Connect_SSA_Link (PC_Graph cfg);
static struct _P_SSALink *NewSSALink (PC_Block def_bb, Expr def_expr,
				      PC_Block use_bb, Expr use_expr,
				      P_SSALink next);
static void FreeSSALink (void *value);
static Expr RightMostOperand (Expr expr);

/****************************************************************************
	Export function body
****************************************************************************/

/*
 * Assumption: cfg is built.
 */
void
P_CF_BuildSSA (PC_Graph cfg)
{
  if (var_tbl_entries)
    {
      /* Clean up after previous SSA build */
      
      P_CF_DeleteSSA (NULL);
    }

  PC_BuildDomTree (cfg);
  PC_FindDominatorFrontier (cfg);

#if DEBUG_P_CF_FindDominator
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.dom", cfg->func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    P_CF_Dump_BB_Dominators (out_file, cfg, "P_CF_BuildSSA ...");
    fclose (out_file);
  }
#endif

  BuildModifiedVarTbl (cfg);

  Place_Phi_Function (cfg);

#if 0
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.bb.BuildSSA.0", cfg->func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    P_CF_Dump_All_BB (out_file, cfg, "P_CF_BuildSSA ...\n");
    fclose (out_file);
  }
#endif

  Connect_SSA_Link (cfg);

#if DEBUG_P_CF_FindDominator
  {
    char fname[128];
    FILE *out_file;

    sprintf (fname, "%s.bb.BuildSSA.0", cfg->func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    P_CF_Dump_All_BB (out_file, cfg, "P_CF_BuildSSA ...\n");
    fclose (out_file);

    sprintf (fname, "%s.ssa_d2u", cfg->func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    Dump_AddrHashTable (out_file, ssa_d2u_tbl,
			"\n********** ssa_d2u_tbl **********\n",
			Dump_SSALink_List);
    fclose (out_file);

    sprintf (fname, "%s.ssa_u2d", cfg->func->name);
    out_file = fopen (fname, "w");
    assert (out_file != NULL);
    Dump_AddrHashTable (out_file, ssa_u2d_tbl,
			"\n********** ssa_u2d_tbl **********\n",
			Dump_SSALink_List);
    fclose (out_file);
  }
#endif

}

void
P_CF_DeleteSSA (PC_Graph cfg)
{
  if (ssa_u2d_tbl)
    {
      DeleteAddrHashTable (ssa_u2d_tbl, FreeSSALink);
      ssa_u2d_tbl = NULL;
    }
  if (ssa_d2u_tbl)
    {
      DeleteAddrHashTable (ssa_d2u_tbl, NULL);
      ssa_d2u_tbl = NULL;
    }

  var_tbl_entries =
    List_reset_free_ptrs (var_tbl_entries,
			  (void (*)(void *)) DeleteVarTblEntry);

  if (cfg)
    Remove_Phi_Function (cfg);
}

/*
 * If expr is not a "leaf" node of an expression, return itself.
 * If expr is a "leaf" node, but not a variable, return itself.
 * If expr is a "leaf" node and a variable, but its ssa cannot be determined, return NULL.
 */
Expr
P_CF_GetSSA (Expr expr)
{
  AddrHashTableEntry entry;
  Expr opnd;

  opnd = expr;
  while (opnd->opcode == OP_cast)
    opnd = opnd->operands;
  entry = GetAddrHashTableEntry (ssa_u2d_tbl, expr);
  if (entry)
    return ((P_SSALink) (entry->value))->def_expr;
  if (opnd->opcode != OP_var)
    return opnd;
  return NULL;
}

Expr
P_CF_EnumReverseSSAExprFirst (Expr expr, P_SSALink * enumerator)
{
  AddrHashTableEntry entry;

  entry = GetAddrHashTableEntry (ssa_d2u_tbl, expr);
  if (entry)
    {
      *enumerator = entry->value;
      return ((P_SSALink) (entry->value))->use_expr;
    }
  *enumerator = NULL;
  return NULL;
}

Expr
P_CF_EnumReverseSSAExprNext (P_SSALink * enumerator)
{
  if (*enumerator == NULL)
    return NULL;
  *enumerator = (*enumerator)->next_use;
  if (*enumerator)
    return (*enumerator)->use_expr;
  return NULL;
}

/****************************************************************************
	Static function body
****************************************************************************/

static VarTblEntry
NewVarTblEntry (VarDcl v, Modify modifying_expr)
{
  VarTblEntry new_var_tbl_entry;

  new_var_tbl_entry = ALLOCATE (_VarTblEntry);
  new_var_tbl_entry->vardcl = v;

  if (modifying_expr)
    {
      new_var_tbl_entry->modifying_expr = List_insert_last (NULL, 
							    modifying_expr);
      new_var_tbl_entry->modifying_bb = List_insert_last (NULL, 
							  modifying_expr->bb);
    }
  else
    {
      new_var_tbl_entry->modifying_expr = NULL;
      new_var_tbl_entry->modifying_bb = NULL;
    }

  new_var_tbl_entry->addr_taken = 0;

  var_tbl_entries = List_insert_last (var_tbl_entries,
				      (void *) new_var_tbl_entry);

  return new_var_tbl_entry;
}

static void
DeleteVarTblEntry (VarTblEntry entry)
{
  VarDcl vd;
  Modify modify;

  if (!entry)
    return;

  vd = entry->vardcl;

  List_start (entry->modifying_expr);
  while ((modify = (Modify) List_next (entry->modifying_expr)))
    {
      FreeModify (modify);
    }

  entry->modifying_expr = List_reset (entry->modifying_expr);
  entry->modifying_bb = List_reset (entry->modifying_bb);

  DISPOSE (entry);

  PS_SetVarTblEntry (vd, NULL);

  return;
}

static Modify
NewModify (Expr expr, PC_Block bb)
{
  Modify new_mod;

  new_mod = ALLOCATE (_Modify);
  new_mod->expr = expr;
  new_mod->bb = bb;
  return new_mod;
}

static void
FreeModify (Modify mod)
{
  DISPOSE (mod);
}

static int
AddressTaken (Expr expr)
{
  Expr curr_expr;

  curr_expr = expr->parentexpr;
  while (curr_expr)
    {
      if (curr_expr->opcode == OP_addr)
	return 1;
      curr_expr = curr_expr->parentexpr;
    }
  return 0;
}

static int
IsDummyVariable (char *var_name)
{
  return (var_name[0] == '@');
}

static void
ModifiedVarTableEntry (Expr expr, PC_Block bb)
{
  Expr curr_expr;
  VarTblEntry var_tbl_entry;

  if (expr == NULL)
    return;

  if (expr->operands != NULL)
    ModifiedVarTableEntry (expr->operands, bb);

  if (expr->sibling != NULL)
    ModifiedVarTableEntry (expr->sibling, bb);

  if (expr->next != NULL)
    ModifiedVarTableEntry (expr->next, bb);

  switch (expr->opcode)
    {
    case OP_assign:
      curr_expr = expr->operands;
      /*
       * only handle scalar variable now.
       */
      if (curr_expr->opcode == OP_var)
	{
	  VarDcl v = PSI_GetVarDclEntry (curr_expr->value.var.key);

	  if (!(v->qualifier & VQ_GLOBAL) && !IsDummyVariable (v->name))
	    {
	      if (!(var_tbl_entry = PS_GetVarTblEntry (v)))
		{
		  /*
		   * a modify to a new variable: 1. set the integer
		   * value to 0, indicating address not taken (so
		   * far).  2. set the pointer value to a new
		   * allocated VarTblEntry.  */
		  var_tbl_entry = 
		    NewVarTblEntry (v, NewModify (expr, bb));
		  PS_SetVarTblEntry (v, var_tbl_entry);
		}
	      else
		{
		  List bbl;
		  PC_Block tbb;

		  /*
		   * a modify to existing variable: 1. add the
		   * variable's updating_expr list.  
		   */

		  var_tbl_entry->modifying_expr =
		    List_insert_last (var_tbl_entry->modifying_expr,
				      NewModify (expr, bb));

		  /* 
		   * 2. add to the variable's modifying_bb list if a
		   * new basic block */

		  bbl = var_tbl_entry->modifying_bb;
		  
		  List_start (bbl);
		  while ((tbb = (PC_Block) List_next (bbl)))
		    if (tbb->ID == bb->ID)
		      break;

		  if (!tbb)
		    var_tbl_entry->modifying_bb = List_insert_last (bbl, bb);
		}
	    }
	}
      break;
    case OP_var:
      if (AddressTaken (expr) && !IsDummyVariable (expr->value.var.name))
	{
	  VarDcl v = PSI_GetVarDclEntry (expr->value.var.key);

	  if (!(var_tbl_entry = PS_GetVarTblEntry (v)))
	    PS_SetVarTblEntry (v, var_tbl_entry = NewVarTblEntry (v, NULL));

	  var_tbl_entry->addr_taken = 1;

	  /* set the integer value to 1, indicating address of
	   * this variable is taken.  */
	}
      break;
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
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
      /*
       * Pflatten should have flattened these cases.
       */
      fprintf (stderr, "ModifiedVarTableEntry: OP %d", expr->opcode);
      P_punt ("ModifiedVarTableEntry: OP_Aadd .. OP_postdec");
      break;
    default:
      /* do nothing */
      break;
    }
}

static void
BuildModifiedVarTbl (PC_Graph cfg)
{
  PC_Block bb;
  _PC_ExprIter ei;
  Expr expr;

  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
	   expr = PC_ExprIterNext (&ei, 1))
	ModifiedVarTableEntry (expr, bb);
    }

  return;
}

static SSA_Flag
NewSSAFlag (int HasAlready, int Work, void *ext)
{
  SSA_Flag new_ssa_flag;

  new_ssa_flag = ALLOCATE (_SSA_Flag);
  new_ssa_flag->HasAlready = HasAlready;
  new_ssa_flag->Work = Work;
  new_ssa_flag->ext = ext;
  return new_ssa_flag;
}

static void
FreeSSAFlag (SSA_Flag flag)
{
  flag->HasAlready = 0;
  flag->Work = 0;
  flag->ext = 0;

  DISPOSE (flag);
}

static void
Insert_Phi_To_BB (VarDcl v, PC_Block bb)
{
  Expr lhs, phi;

  lhs = P_NewExprWithOpcode (OP_var);
  P_SetExprVarName (lhs, strdup (v->name));
  P_SetExprVarKey (lhs, v->key);
  P_SetExprType (lhs, v->type);

  phi = P_NewExprWithOpcode (OP_phi);
  P_SetExprType (phi, v->type);

  P_AppendExprOperands (phi, lhs);

  PC_NewPStmtExprBefore (PC_T_Expr, bb, bb->first_ps, phi);

  return;
}

static void
Remove_Phi_From_BB (PC_Graph cfg, PC_Block bb)
{
  PC_PStmt ps, nps;

  for (ps = bb->first_ps; ps; ps = nps)
    {
      nps = ps->succ;

      if (ps->type == PC_T_Expr &&
	  ps->data.stmt->stmtstruct.expr->opcode == OP_phi)
	PC_DeletePStmt (bb, ps);
    }
}

/*
 * increment a global variable IterCount;
 */
static long
Place_Phi_Var (VarTblEntry ve)
{
  List df;
  List W = NULL;
  PC_Block bb, dom_front_bb;
  SSA_Flag ssa_flag;

  if (ve->addr_taken)
    return 1;

  IterCount++;

  List_start (ve->modifying_bb);
  while ((bb = (PC_Block) List_next (ve->modifying_bb)))
    W = List_insert_first (W, bb);

  while ((bb = (PC_Block) List_first (W)))
    {
      W = List_delete_current (W);
      df = bb->dom_frontier;
      List_start (df);
      while ((dom_front_bb = (PC_Block) List_next (df)))
	{
	  ssa_flag = dom_front_bb->ext;
	  if (ssa_flag->HasAlready < IterCount)
	    {
	      Insert_Phi_To_BB (ve->vardcl, dom_front_bb);
	      ssa_flag->HasAlready = IterCount;
	      if (ssa_flag->Work < IterCount)
		{
		  ssa_flag->Work = IterCount;
		  W = List_insert_first (W, dom_front_bb);
		}
	    }
	}
    }
  return 1;
}

static void
Place_Phi_Function (PC_Graph cfg)
{
  PC_Block bb;
  SSA_Flag flag;

  IterCount = 0;
  for (bb = cfg->first_bb; bb; bb = bb->next)
    bb->ext = NewSSAFlag (0, 0, bb->ext);
  current_cfg = cfg;

  {
    VarTblEntry ve;

    List_start (var_tbl_entries);
    while ((ve = (VarTblEntry) List_next (var_tbl_entries)))
      {
	if (ve->addr_taken)
	  continue;

	Place_Phi_Var (ve);
      }
  }

  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      flag = bb->ext;
      bb->ext = flag->ext;
      FreeSSAFlag (flag);
    }
}

static void
Remove_Phi_Function (PC_Graph cfg)
{
  PC_Block bb;

  for (bb = cfg->first_bb; bb; bb = bb->next)
    Remove_Phi_From_BB (cfg, bb);
}

static void
Init_Var_Tbl_Entry_Stack (VarTblEntry ve)
{
  if (!ve->addr_taken)
    {
      ve->stack.length = 0;
      ve->stack.head = NULL;
      ve->stack.tail = NULL;
    }

  return;
}

static ExprHolder
NewExprHolder (Expr expr, PC_Block bb)
{
  ExprHolder new_expr_holder;

  new_expr_holder = ALLOCATE (_ExprHolder);
  new_expr_holder->expr = expr;
  new_expr_holder->bb = bb;
  return new_expr_holder;
}

static void
FreeExprHolder (ExprHolder expr_holder)
{
  DISPOSE (expr_holder);
}

static void
Link_SSA (Expr expr, PC_Block bb)
{
  int value;
  VarTblEntry var_tbl_entry_ptr;
  ExprHolder define_expr_holder;
  P_SSALink ssa_link;
  AddrHashTableEntry ssa_d2u_entry;
  AddrHashTableEntry ssa_u2d_entry;
  Expr opnd;

  if (expr->opcode == OP_phi)
    return;
  if (expr->opcode == OP_var)
    {
      VarDcl v = PSI_GetVarDclEntry (expr->value.var.key);

      if (v && (var_tbl_entry_ptr = PS_GetVarTblEntry (v)))
	{
	  value = var_tbl_entry_ptr->addr_taken;
	  assert ((value == 0) || (value == 1));
	  if ((value == 0) && (var_tbl_entry_ptr->stack.length > 0))
	    {
	      define_expr_holder =
		(ExprHolder) Peek_LptrQ (&(var_tbl_entry_ptr->stack));
	      assert (define_expr_holder != NULL);
	      ssa_d2u_entry =
		FindAndCreateAddrHashTableEntry (ssa_d2u_tbl,
						 define_expr_holder->expr);
	      ssa_d2u_entry->value = ssa_link =
		NewSSALink (define_expr_holder->bb, define_expr_holder->expr,
			    bb, expr, (P_SSALink) (ssa_d2u_entry->value));
	      ssa_u2d_entry =
		FindAndCreateAddrHashTableEntry (ssa_u2d_tbl, expr);
	      ssa_u2d_entry->value = ssa_link;
	    }
	}
    }
  else if (expr->opcode == OP_assign)
    {
      opnd = expr->operands;
      if (opnd->opcode == OP_compexpr)
	{
	  Link_SSA (opnd->operands, bb);
	  opnd = opnd->operands->next;
	}
      if (opnd->opcode != OP_var)
	Link_SSA (expr->operands, bb);
      Link_SSA (expr->operands->sibling, bb);
    }
  else if (expr->opcode == OP_call)
    {
      opnd = expr->operands;
      Link_SSA (opnd, bb);
      opnd = opnd->sibling;
      while (opnd)
	{
	  Link_SSA (opnd, bb);
	  opnd = opnd->next;
	}
    }
  else if (expr->opcode == OP_compexpr)
    {
      assert (expr->operands);
      Link_SSA (expr->operands, bb);
      if (expr->operands->next)
	Link_SSA (expr->operands->next, bb);
    }
  else
    {
      opnd = expr->operands;
      while (opnd)
	{
	  Link_SSA (opnd, bb);
	  opnd = opnd->sibling;
	}
    }
}

static Expr
RightMostOperand (Expr expr)
{
  if (expr->opcode == OP_compexpr)
    return RightMostOperand (expr->operands->next);
  if (expr->opcode == OP_cast)
    return RightMostOperand (expr->operands);
  return expr;
}

static void
Push_SSA (Expr expr, PC_Block bb)
{
  Expr opnd;
  ExprHolder expr_holder;
  int value;
  VarTblEntry var_tbl_entry_ptr;

  switch (expr->opcode)
    {
    case OP_assign:
    case OP_phi:
      if (expr->opcode == OP_assign)
	{
	  /* After P_flatten, it's still possible to have
	   * x = y = ...
	   */
	  Push_SSA (expr->operands->sibling, bb);
	}
      opnd = RightMostOperand (expr->operands);
      if (opnd->opcode == OP_var)
	{			/* handle only scalar now */
	  VarDcl v = PSI_GetVarDclEntry (opnd->value.var.key);

	  if ((var_tbl_entry_ptr = PS_GetVarTblEntry (v)))
	    {
	      value = var_tbl_entry_ptr->addr_taken;
	      assert ((value == 0) || (value == 1));
	      if (value == 0)
		{
		  expr_holder = NewExprHolder (expr, bb);
		  En_FILO_LptrQ (&(var_tbl_entry_ptr->stack),
				 (void *) expr_holder);
		}
	    }
	  else if (expr->opcode == OP_phi)
	    {
	      P_punt ("Push_SSA: C_find fails for OP_phi.");
	    }
	}
      break;
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
      P_punt ("Push_SSA: OP_Aadd ... OP_Axor");
      /*
       * Should have already be Pflattened.
       */
    case OP_preinc:
    case OP_predec:
    case OP_postinc:
    case OP_postdec:
      P_punt ("Push_SSA: OP_preinc ... OP_postdec");
      /*
       * Should have already be Pflattened.
       */
    case OP_call:		/* inter-procedural ? */
      break;
    case OP_compexpr:
      opnd = expr->operands;
      while (opnd)
	{
	  Push_SSA (opnd, bb);
	  opnd = opnd->next;
	}
      break;
    default:
      break;
    }
}

static void
PopVarStack (Expr expr)
{
  VarTblEntry var_tbl_entry_ptr;
  ExprHolder define_expr_holder;
  int value;

  if (expr->opcode == OP_var)	/* handle only scalar for now */
    {
      VarDcl v = PSI_GetVarDclEntry (expr->value.var.key);

      if ((var_tbl_entry_ptr = PS_GetVarTblEntry (v)))
	{
	  value = var_tbl_entry_ptr->addr_taken;
	  assert ((value == 0) || (value == 1));
	  if (value == 0)
	    {
	      define_expr_holder = De_LptrQ (&(var_tbl_entry_ptr->stack));
	      FreeExprHolder (define_expr_holder);
	    }
	}
    }
}

static void
Pop_SSA (Expr expr)
{
  if (expr == NULL)
    return;
  switch (expr->opcode)
  {
    case OP_phi:
      PopVarStack (expr->operands);
      break;
    case OP_assign:
      Pop_SSA(expr->operands->sibling);
      PopVarStack (RightMostOperand (expr->operands));
      break;
    case OP_compexpr:
      expr = expr->operands;
      while (expr) {
        Pop_SSA(expr);
        expr = expr->next;
      }
      break;
    default:
      break;
  }
}

static void
Preorder_Traverse_Dominator_Tree (PC_Block bb)
{
  PC_Block child_bb, succ_bb;
  _PC_ExprIter ei;
  Expr expr;
  Expr new_expr;
  Expr prev_opnd, curr_opnd;
  ExprHolder define_expr_holder;
  AddrHashTableEntry ssa_d2u_entry, ssa_u2d_entry;
  PC_Flow s_flow;
  int value;
  VarTblEntry var_tbl_entry_ptr;
  P_SSALink ssa_link;

  for (expr = PC_ExprIterFirst (bb, &ei, 1); expr;
       expr = PC_ExprIterNext (&ei, 1))
    {
      /*
       * link every variable use to its ssa.
       */
      Link_SSA (expr, bb);
      /*
       * for every variable define, create a ssa and push it to the
       * stack in var_tbl.  */
      Push_SSA (expr, bb);
    }
  /*
   * For each successor, register the SSA links for Phi function.
   */

  for (s_flow = bb->s_flow; s_flow; s_flow = s_flow->s_next_flow)
    {
      succ_bb = s_flow->dest_bb;

      for (expr = PC_ExprIterFirst (succ_bb, &ei, 1); 
	   expr && (expr->opcode == OP_phi);
	   expr = PC_ExprIterNext (&ei, 1))
	{
	  VarDcl v = PSI_GetVarDclEntry (expr->operands->value.var.key);

	  if ((var_tbl_entry_ptr = PS_GetVarTblEntry (v)))
	    {
	      value = var_tbl_entry_ptr->addr_taken;
	      assert ((value == 0) || (value == 1));
	      if ((value == 0) && (var_tbl_entry_ptr->stack.length > 0))
		{
		  define_expr_holder =
		    (ExprHolder) Peek_LptrQ (&(var_tbl_entry_ptr->stack));
		  assert (define_expr_holder != NULL);
		  /*
		   * insert the ssa_link to the RHS of PHI, if it's not there yet.
		   */
		  prev_opnd = NULL;
		  curr_opnd = expr->operands->sibling;
		  while (curr_opnd)
		    {
		      assert (curr_opnd->opcode == OP_int);
		      if (curr_opnd->value.scalar ==
			  define_expr_holder->bb->ID)
			break;
		      prev_opnd = curr_opnd;
		      curr_opnd = curr_opnd->sibling;
		    }
		  if (curr_opnd == NULL)
		    {
		      if (prev_opnd)
			prev_opnd->sibling = new_expr =
			  P_NewIntExpr ((ITintmax) define_expr_holder->bb->ID);
		      else
			expr->operands->sibling = new_expr =
			  P_NewIntExpr ((ITintmax) define_expr_holder->bb->ID);
		      ssa_d2u_entry =
			FindAndCreateAddrHashTableEntry (ssa_d2u_tbl,
							 define_expr_holder->
							 expr);
		      ssa_d2u_entry->value = ssa_link =
			NewSSALink (define_expr_holder->bb,
				    define_expr_holder->expr, succ_bb,
				    new_expr,
				    (P_SSALink) (ssa_d2u_entry->value));
		      ssa_u2d_entry =
			FindAndCreateAddrHashTableEntry (ssa_u2d_tbl,
							 new_expr);
		      assert (ssa_u2d_entry->value == NULL);
		      ssa_u2d_entry->value = ssa_link;
		    }
		  else
		    {		/* 
				 * sanity checking 
				 */
		      ssa_u2d_entry =
			FindAndCreateAddrHashTableEntry (ssa_u2d_tbl,
							 curr_opnd);
		      assert ((((P_SSALink) ssa_u2d_entry->value)->def_bb ==
			       define_expr_holder->bb)
			      && (((P_SSALink) ssa_u2d_entry->value)->
				  def_expr == define_expr_holder->expr));
		    }
		}
	    }
	  else
	    {
	      /* 
	       * the LHS of PHI function is not found in var_tbl 
	       */
	      P_punt
		("Preorder_Traverse_Dominator_Tree: LHS of PHI not found in var_tbl.");
	    }
	}
    }
  /*
   * traverse children in the dominator tree.
   */

  List_start (bb->dom_tree_children);
  while ((child_bb = (PC_Block) List_next (bb->dom_tree_children)))
    Preorder_Traverse_Dominator_Tree (child_bb);

  /*
   * for each assignment in this bb, pop the stack of expr_holder.
   */

  for (expr = PC_ExprIterFirst (bb, &ei, 1); expr; expr = PC_ExprIterNext (&ei, 1))
    Pop_SSA(expr);
}

static void
Connect_SSA_Link (PC_Graph cfg)
{
  VarTblEntry ve;

  List_start (var_tbl_entries);
  while ((ve = (VarTblEntry) List_next (var_tbl_entries)))
    {
      if (ve->addr_taken)
	continue;

      Init_Var_Tbl_Entry_Stack (ve);
    }

  ssa_d2u_tbl = NewAddrHashTable (1024, "ssa_d2u_tbl");
  ssa_u2d_tbl = NewAddrHashTable (2048, "ssa_u2d_tbl");
  Preorder_Traverse_Dominator_Tree (cfg->first_bb);
}

static P_SSALink
NewSSALink (PC_Block def_bb, Expr def_expr, PC_Block use_bb, Expr use_expr,
	    P_SSALink next_use)
{
  P_SSALink link;

  link = ALLOCATE (_P_SSALink);
  link->def_bb = def_bb;
  link->def_expr = def_expr;
  link->use_bb = use_bb;
  link->use_expr = use_expr;
  link->next_use = next_use;
  return link;
}

static void
FreeSSALink (void *value)
{
  P_SSALink ssa_link;

  ssa_link = value;
  ssa_link->def_bb = 0;
  ssa_link->def_expr = 0;
  ssa_link->use_bb = 0;
  ssa_link->use_expr = 0;
  ssa_link->next_use = 0;
  DISPOSE (ssa_link);
}
