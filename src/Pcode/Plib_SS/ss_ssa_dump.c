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
#include <Pcode/symtab_i.h>
#include <Pcode/struct.h>
#include <Pcode/ss_ssa.h>

static void
P_CF_Dump_Indent (FILE * out_file, int n)
{
  int i;

  for (i = 0; i < n; i++)
    fprintf (out_file, "\t");
}


static char *optable[] = {
  "OP_0",			/* 0 */
  "var",			/* 1 */
  "enum",			/* 2 */
  "int",			/* 3 */
  "real",			/* 4 */
  "error",			/* 5 */
  "char",			/* 6 */
  "string",			/* 7 */
  "dot",			/* 8 */
  "arrow",			/* 9 */
  "cast",			/* 10 */
  "expr_size",			/* 11 */
  "type_size",			/* 12 */
  "quest",			/* 13 */
  "disj",			/* 14 */
  "conj",			/* 15 */
  "compexpr",			/* 16 */
  "assign",			/* 17 */
  "or",				/* 18 */
  "xor",			/* 19 */
  "and",			/* 20 */
  "eq",				/* 21 */
  "ne",				/* 22 */
  "lt",				/* 23 */
  "le",				/* 24 */
  "ge",				/* 25 */
  "gt",				/* 26 */
  "rshft",			/* 27 */
  "lshft",			/* 28 */
  "add",			/* 29 */
  "sub",			/* 30 */
  "mul",			/* 31 */
  "div",			/* 32 */
  "mod",			/* 33 */
  "neg",			/* 34 */
  "not",			/* 35 */
  "inv",			/* 36 */
  "OP_37",			/* 37 */
  "preinc",			/* 38 */
  "predec",			/* 39 */
  "postinc",			/* 40 */
  "postdec",			/* 41 */
  "Aadd",			/* 42 */
  "Asub",			/* 43 */
  "Amul",			/* 44 */
  "Adiv",			/* 45 */
  "Amod",			/* 46 */
  "Arshft",			/* 47 */
  "Alshft",			/* 48 */
  "Aand",			/* 49 */
  "Aor",			/* 50 */
  "Axor",			/* 51 */
  "indr",			/* 52 */
  "addr",			/* 53 */
  "index",			/* 54 */
  "call",			/* 55 */
  "float",			/* 56 */
  "double",			/* 57 */
  "long",			/* 58 */
  "OP_null",			/* 59 */
  "OP_sync",			/* 60 */
  "OP_stmt_expr",		/* 61 */
  "OP_asm_oprd",		/* 62 */
  "OP_phi"			/* 63 *//* CWL - 10/14/02 for SSA */
};


void
P_CF_Dump_SSA_Expr (FILE * out_file, Expr expr, int n_indent)
{
  Expr def_expr;
  Expr opnd;

  if (!expr)
    return;
  P_CF_Dump_Indent (out_file, n_indent);
  switch (expr->opcode)
    {
    case OP_var:
      fprintf (out_file, "%s", expr->value.var.name);
      break;
    case OP_int:
      if (PSI_IsUnsignedType (expr->type))
	fprintf (out_file, ITuintmaxformat, expr->value.uscalar);
      else
	fprintf (out_file, ITintmaxformat, expr->value.scalar);
      break;
    case OP_real | OP_float | OP_double:
      fprintf (out_file, "%lf", expr->value.real);
      break;
    case OP_char | OP_string:
      fprintf (out_file, "\"%s\"", expr->value.string);
      break;
    default:
      fprintf (out_file, "%s", optable[expr->opcode]);
      break;
    }
#if LP64_ARCHITECTURE
  fprintf (out_file, "(0x%lx)", (long) expr);
#else
  fprintf (out_file, "(0x%x)", (int) expr);
#endif
  fprintf (out_file, " => (");
  def_expr = P_CF_GetSSA (expr);
  if (def_expr == expr)
    fprintf (out_file, "self");
  else if (def_expr)
    {
#if LP64_ARCHITECTURE
      fprintf (out_file, "0x%lx", (long) def_expr);
#else
      fprintf (out_file, "0x%x", (int) def_expr);
#endif
    }
  fprintf (out_file, ")\n");
  opnd = expr->operands;
  while (opnd)
    {
      P_CF_Dump_SSA_Expr (out_file, opnd, n_indent + 1);
      opnd = opnd->sibling;
    }
  if (expr->next)
    P_CF_Dump_SSA_Expr (out_file, expr->next, n_indent);
}

static char *cnt_type_tbl[] = {
  "RETURN",
  "BREAK",
  "GOTO",
  "IF",
  "SWITCH",
  "ENTRY",
  "EXIT"
};

void
P_CF_Dump_SSA_BB (FILE * out_file, PC_Block bb)
{
  Expr expr;
  _PC_ExprIter ei;

  fprintf (out_file, "BB (%d)(cnt_type=%s):\n", bb->ID, cnt_type_tbl[bb->cont_type]);
  for (expr = PC_ExprIterFirst(bb, &ei, 1); expr; expr = PC_ExprIterNext(&ei, 1))
    {
      fprintf (out_file, "Expr(%d): %s ...\n", P_GetExprID(expr), optable[expr->opcode]);
      switch (expr->opcode)
	{
	case OP_assign:
	case OP_phi:
	  fprintf (out_file, "LHS: ");
	  P_CF_Dump_SSA_Expr (out_file, expr->operands, 1);
	  fprintf (out_file, "RHS: ");
	  P_CF_Dump_SSA_Expr (out_file, expr->operands->sibling, 1);
	  break;
	default:
	  P_CF_Dump_SSA_Expr (out_file, expr, 1);
	}
      fprintf (out_file, "\n");
    }
}

void
P_CF_Dump_SSA (FILE * out_file, PC_Graph cfg, char *title)
{
  PC_Block bb;

  fprintf (out_file, "%s", title);
  for (bb = cfg->first_bb; bb; bb = bb->next)
    {
      P_CF_Dump_SSA_BB (out_file, bb);
    }
}

void
Dump_SSALink_List (FILE * out_file, void *list)
{
  P_SSALink ssa_use;

  ssa_use = list;
  while (ssa_use)
    {
      fprintf (out_file, "[d, u] = [ (Block(%d), Expr(%d)), (Block(%d), Expr(%d)) ] ",
	       ssa_use->def_bb->ID,
	       P_GetExprID(ssa_use->def_expr),
	       ssa_use->use_bb->ID,
	       P_GetExprID(ssa_use->use_expr));
      ssa_use = ssa_use->next_use;
    }
}
