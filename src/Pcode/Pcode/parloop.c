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


#include <config.h>
#include "pcode.h"
#include "parloop.h"

Stmt
Parloop_Stmts_Prologue_Stmt (Stmt parloop_stmt)
{
  ParLoop parloop;
  Pstmt pstmt;

  assert (parloop_stmt != 0);
  assert (parloop_stmt->type == ST_PARLOOP);

  parloop = parloop_stmt->stmtstruct.parloop;
  assert (parloop != 0);

  pstmt = parloop->pstmt;
  assert (pstmt != 0);

  return pstmt->stmt;
}

Stmt
Parloop_Stmts_Body_Stmt (Stmt parloop_stmt)
{
  Stmt stmt;
  ParLoop parloop;
  Pstmt pstmt;
  Compound compound;

  assert (parloop_stmt != 0);
  assert (parloop_stmt->type == ST_PARLOOP);

  parloop = parloop_stmt->stmtstruct.parloop;
  assert (parloop != 0);

  pstmt = parloop->pstmt;
  assert (pstmt != 0);

  stmt = pstmt->stmt;
  assert (stmt != 0);
  assert (stmt->type == ST_COMPOUND);

  compound = stmt->stmtstruct.compound;
  assert (compound != 0);

  stmt = compound->stmt_list;
  assert (stmt != 0);

  while ((stmt != 0) && (stmt->type != ST_BODY))
    stmt = stmt->lex_next;

  assert (stmt != 0);
  return stmt;
}

Stmt
Parloop_Body_Stmt (ParLoop parloop)
{
  Stmt stmt;
  Pstmt pstmt;
  Compound compound;

  assert (parloop != 0);

  pstmt = parloop->pstmt;
  assert (pstmt != 0);

  stmt = pstmt->stmt;
  assert (stmt != 0);
  assert (stmt->type == ST_COMPOUND);

  compound = stmt->stmtstruct.compound;
  assert (compound != 0);

  stmt = compound->stmt_list;
  assert (stmt != 0);

  while ((stmt != 0) && (stmt->type != ST_BODY))
    stmt = stmt->lex_next;

  assert (stmt != 0);
  return stmt;
}

Stmt
Parloop_Stmts_First_Epilogue_Stmt (Stmt parloop_stmt)
{
  Stmt stmt;
  ParLoop parloop;
  Pstmt pstmt;
  Compound compound;

  assert (parloop_stmt != 0);
  assert (parloop_stmt->type == ST_PARLOOP);

  parloop = parloop_stmt->stmtstruct.parloop;
  assert (parloop != 0);

  pstmt = parloop->pstmt;
  assert (pstmt != 0);

  stmt = pstmt->stmt;
  assert (stmt != 0);
  assert (stmt->type == ST_COMPOUND);

  compound = stmt->stmtstruct.compound;
  assert (compound != 0);

  stmt = compound->stmt_list;
  assert (stmt != 0);

  while ((stmt != 0) && (stmt->type != ST_EPILOGUE))
    stmt = stmt->lex_next;

  assert (stmt != 0);
  return stmt;
}
