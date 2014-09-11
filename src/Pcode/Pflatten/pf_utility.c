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
 *	File:	flatten.c
 *	Author:	Ben-Chung Cheng and Wen-mei Hwu
 * 	Copyright (c) 1996 Ben-Chung Cheng, Wen-mei Hwu. All rights reserved.  
 * 	All rights granted to the University of Illinois.
 *	The University of Illinois software License Agreement
 * 	specifies the terms and conditions for redistribution.
\*****************************************************************************/
/* 10/11/02 REK Pflatten accepts the following arguments.
 *              -FDEBUG_FLATTENING=(yes|no)
 *              -FDD_SPLIT_COMPOUND_EXPR_STMTS=(yes|no)
 *              -Ffast_mode=(yes|no)
 */

#include <config.h>
#include <library/i_error.h>
#include <Pcode/pcode.h>
#include <Pcode/struct.h>
#include <Pcode/symtab_i.h>
#include <Pcode/query.h>
#include "flatten.h"
#include "data.h"


/*! \brief Copies source line information from one stmt to another.
 *
 * \param src
 *  the stmt from which the line information is copied.
 * \param dst
 *  the stmt to which the line information is copied.
 */
void
PF_CopyLineInfo (Stmt src, Stmt dst)
{
  char *filename;

  if (!src || !dst)
    return;

  P_SetStmtLineno (dst, P_GetStmtLineno (src));
  P_SetStmtColno (dst, P_GetStmtColno (src));

  if ((filename = P_GetStmtFilename (src)))
    P_SetStmtFilename (dst, strdup (filename));

  return;
}


int
PF_HasFallThrough (Stmt stmt)
{
  Stmt stmt1, stmt2;

  if (stmt == 0)
    return 1;
  switch (stmt->type)
    {
    case ST_CONT:
    case ST_BREAK:
    case ST_GOTO:
    case ST_RETURN:
      return 0;
    case ST_COMPOUND:
      stmt1 = stmt->stmtstruct.compound->stmt_list;
      if (stmt1 == NULL)
	return 1;
      while (stmt1->lex_next)
	stmt1 = stmt1->lex_next;
      return PF_HasFallThrough (stmt1);
    case ST_IF:
      stmt1 = stmt->stmtstruct.ifstmt->then_block;
      stmt2 = stmt->stmtstruct.ifstmt->else_block;
      return (PF_HasFallThrough (stmt1) || PF_HasFallThrough (stmt2));
    default:
      return 1;
    }
}


#if 0
/*! \brief Inserts a statement into a statement list, moving label(s).
 *
 * \param s
 *  the stmt to insert the statement before.
 * \param new
 *  the statement ot insert.
 *
 * Inserts statement \a new into \a s's statement list before \a s and
 * move \a s's labels onto \a new.
 *
 * \sa P_StmtInsertStmtBefore(), P_StmtUpdate(),
 * P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertStmtAfter(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() 
 */
void
PF_StmtInsertStmtBefore (Stmt s, Stmt new)
{
  PSI_StmtEncloseInCompound (s);

  if (s->lex_prev)
    s->lex_prev->lex_next = new;
  new->lex_prev = s->lex_prev;
  new->lex_next = s;
  s->lex_prev = new;

  /* Update any parent stmts to refer to new instead of s. */
  P_StmtUpdateParents (s, new);

  P_SetStmtParentStmt (new, P_GetStmtParentStmt (s));
  P_SetStmtParentFunc (new, P_GetStmtParentFunc (s));
  P_SetStmtParentExpr (new, P_GetStmtParentExpr (s));

  P_SetStmtLabels (new, PF_ExtractLabels (s));

  return;
}


/*! \brief Inserts a statement into a statement list.
 *
 * \param s
 *  the stmt to insert the statement after.
 * \param new
 *  the statement to insert.
 *
 * Inserts statement \a new into \a s's statement list after \a s.
 *
 * \sa P_StmtUpdate(), P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertExprBefore(), P_StmtInsertStmtBefore(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() */
void
PF_StmtInsertStmtAfter (Stmt s, Stmt new)
{
  PSI_StmtEncloseInCompound (s);

  if (s->lex_next)
    s->lex_next->lex_prev = new;
  new->lex_next = s->lex_next;
  new->lex_prev = s;
  s->lex_next = new;

  P_SetStmtParentStmt (new, P_GetStmtParentStmt (s));
  P_SetStmtParentFunc (new, P_GetStmtParentFunc (s));
  P_SetStmtParentExpr (new, P_GetStmtParentExpr (s));

  return;
}


/*! \brief Inserts an expression into a statement list and moves label(s) up.
 *
 * \param s
 *  the stmt to insert the expression before.
 * \param e
 *  the expression to insert.
 *
 * Inserts a new expression stmt bearing the label(s) of \a s and
 * containing \a e into \a s's statement list before \a s.
 *
 * \sa P_StmtInsertExprBefore(), P_StmtUpdate(),
 * P_StmtUpdateParents(), P_StmtInsertExprAfter(),
 * P_StmtInsertStmtAfter(), P_StmtInsertStmtBefore(),
 * P_StmtAddLabel(), P_StmtAddLabelAfter(), P_StmtRemoveStmt() 
 */
void
PF_StmtInsertExprBefore (Stmt s, Expr e)
{
  Stmt new = P_NewStmtWithType (ST_EXPR);

  P_SetStmtExpr (new, e);

  PF_StmtInsertStmtBefore (s, new);

  return;
}
#endif
